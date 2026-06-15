#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_FUNCTIONS_KLL_SKETCH_INTERNAL_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_FUNCTIONS_KLL_SKETCH_INTERNAL_H_

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "backend/engine/semantic/error.h"
#include "backend/engine/semantic/value.h"
#include "googlesql/public/type.h"
#include "kll_sketch.hpp"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace functions {
namespace kll_detail {

constexpr unsigned char kKllMagic[] = {0x12, 0xef, 0x81};
constexpr uint8_t kSketchTypeInt64 = 0;
constexpr uint8_t kSketchTypeFloat64 = 1;

constexpr int64_t kDefaultPrecision = 1000;
constexpr int64_t kMinPrecision = 1;
constexpr int64_t kMaxPrecision = 100000;
constexpr int64_t kMaxNumQuantiles = 100000;
constexpr int64_t kMinWeight = 1;
constexpr int64_t kMaxWeight = 100000;
constexpr int64_t kMinK = 8;
constexpr int64_t kMaxK = 4096;

struct Float64GoogleSqlLess {
  bool operator()(double a, double b) const {
    const bool a_nan = std::isnan(a);
    const bool b_nan = std::isnan(b);
    if (a_nan && b_nan) return false;
    if (a_nan) return true;
    if (b_nan) return false;
    return a < b;
  }
};

enum class SketchType { kInt64, kFloat64 };

inline absl::Status InvalidSketch() {
  return MakeSemanticError(
      SemanticErrorReason::kInvalidArgument,
      "KLL_QUANTILES sketch is not in a supported wire format");
}

inline absl::Status TypeMismatch(SketchType expected) {
  return MakeSemanticError(
      SemanticErrorReason::kInvalidArgument,
      expected == SketchType::kInt64
          ? "KLL_QUANTILES sketch underlying type is not INT64"
          : "KLL_QUANTILES sketch underlying type is not FLOAT64");
}

inline uint16_t PrecisionToK(int64_t precision) {
  const int64_t scaled = precision / 5;
  const int64_t clamped =
      std::max<int64_t>(kMinK, std::min<int64_t>(scaled, kMaxK));
  return static_cast<uint16_t>(clamped);
}

inline absl::StatusOr<int64_t> ParseAndValidatePrecision(
    std::optional<int64_t> precision) {
  if (!precision.has_value()) return kDefaultPrecision;
  if (*precision < kMinPrecision || *precision > kMaxPrecision) {
    return MakeSemanticError(
        SemanticErrorReason::kInvalidArgument,
        absl::StrCat("KLL_QUANTILES.INIT precision must be in [",
                     kMinPrecision,
                     ", ",
                     kMaxPrecision,
                     "]"));
  }
  return *precision;
}

inline absl::StatusOr<int64_t> ParseAndValidateNumQuantiles(
    int64_t num_quantiles) {
  if (num_quantiles <= 0 || num_quantiles > kMaxNumQuantiles) {
    return MakeSemanticError(
        SemanticErrorReason::kInvalidArgument,
        absl::StrCat("KLL_QUANTILES num_quantiles must be in (0, ",
                     kMaxNumQuantiles,
                     "]"));
  }
  return num_quantiles;
}

inline absl::StatusOr<double> ParseAndValidatePhi(const Value& phi) {
  if (phi.type_kind() != ::googlesql::TYPE_DOUBLE &&
      phi.type_kind() != ::googlesql::TYPE_FLOAT) {
    return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                             "KLL_QUANTILES phi must be FLOAT64");
  }
  const double value = phi.type_kind() == ::googlesql::TYPE_DOUBLE
                           ? phi.double_value()
                           : static_cast<double>(phi.float_value());
  if (value < 0.0 || value > 1.0) {
    return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                             "KLL_QUANTILES phi must be between 0 and 1");
  }
  return value;
}

inline absl::StatusOr<int64_t> ParseAndValidateWeight(const Value& weight) {
  if (weight.is_null()) return int64_t{1};
  if (weight.type_kind() != ::googlesql::TYPE_INT64) {
    return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                             "KLL_QUANTILES weight must be INT64");
  }
  const int64_t value = weight.int64_value();
  if (value < kMinWeight || value > kMaxWeight) {
    return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                             absl::StrCat("KLL_QUANTILES weight must be in [",
                                          kMinWeight,
                                          ", ",
                                          kMaxWeight,
                                          "]"));
  }
  return value;
}

inline std::optional<int64_t> FirstNonNullInt64(
    const std::vector<std::vector<Value>>& input_column_values,
    size_t column_index) {
  if (column_index >= input_column_values.size()) return std::nullopt;
  for (const Value& v : input_column_values[column_index]) {
    if (v.is_null()) continue;
    if (v.type_kind() == ::googlesql::TYPE_INT64) return v.int64_value();
  }
  return std::nullopt;
}

inline std::string SerializeSketchBytes(SketchType type,
                                        absl::string_view ds_payload) {
  std::string out;
  out.reserve(4 + ds_payload.size());
  out.push_back(static_cast<char>(kKllMagic[0]));
  out.push_back(static_cast<char>(kKllMagic[1]));
  out.push_back(static_cast<char>(kKllMagic[2]));
  out.push_back(static_cast<char>(
      type == SketchType::kInt64 ? kSketchTypeInt64 : kSketchTypeFloat64));
  out.append(ds_payload.data(), ds_payload.size());
  return out;
}

inline absl::StatusOr<std::pair<SketchType, std::string>> ParseSketchEnvelope(
    absl::string_view bytes) {
  if (bytes.size() < 4) return InvalidSketch();
  if (static_cast<unsigned char>(bytes[0]) != kKllMagic[0] ||
      static_cast<unsigned char>(bytes[1]) != kKllMagic[1] ||
      static_cast<unsigned char>(bytes[2]) != kKllMagic[2]) {
    return InvalidSketch();
  }
  const uint8_t type_byte = static_cast<unsigned char>(bytes[3]);
  if (type_byte != kSketchTypeInt64 && type_byte != kSketchTypeFloat64) {
    return InvalidSketch();
  }
  SketchType type =
      type_byte == kSketchTypeInt64 ? SketchType::kInt64 : SketchType::kFloat64;
  return std::make_pair(type, std::string(bytes.substr(4)));
}

template <typename SketchT>
inline std::string SerializeDataSketchesSketch(const SketchT& sketch) {
  const auto bytes = sketch.serialize();
  return std::string(reinterpret_cast<const char*>(bytes.data()), bytes.size());
}

template <typename SketchT>
inline absl::StatusOr<SketchT> DeserializeDataSketchesSketch(
    absl::string_view payload) {
  try {
    return SketchT::deserialize(payload.data(), payload.size());
  } catch (const std::exception& e) {
    return InvalidSketch();
  }
}

template <typename SketchT>
inline absl::StatusOr<std::string> SerializeTypedSketch(const SketchT& sketch,
                                                        SketchType type) {
  return SerializeSketchBytes(type, SerializeDataSketchesSketch(sketch));
}

template <typename SketchT>
inline absl::StatusOr<SketchT> DeserializeTypedSketch(absl::string_view bytes,
                                                      SketchType expected) {
  auto envelope = ParseSketchEnvelope(bytes);
  if (!envelope.ok()) return envelope.status();
  if (envelope->first != expected) return TypeMismatch(expected);
  return DeserializeDataSketchesSketch<SketchT>(envelope->second);
}

using Int64Sketch = datasketches::kll_sketch<int64_t>;
using Float64Sketch = datasketches::kll_sketch<double, Float64GoogleSqlLess>;

template <typename SketchT>
inline void UpdateWithWeight(SketchT* sketch,
                             const typename SketchT::value_type& item,
                             int64_t weight) {
  for (int64_t i = 0; i < weight; ++i) {
    sketch->update(item);
  }
}

template <typename SketchT>
inline absl::StatusOr<Value> BuildQuantileArray(
    const SketchT& sketch,
    int64_t num_quantiles,
    const ::googlesql::Type* return_type) {
  if (sketch.is_empty()) {
    return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                             "KLL_QUANTILES sketch is empty");
  }
  const ::googlesql::ArrayType* arr_type =
      return_type != nullptr && return_type->IsArray() ? return_type->AsArray()
                                                       : nullptr;
  if (arr_type == nullptr) {
    return MakeSemanticError(
        SemanticErrorReason::kInvalidArgument,
        "KLL_QUANTILES extract requires ARRAY return type");
  }

  std::vector<Value> elements;
  elements.reserve(static_cast<size_t>(num_quantiles) + 1);
  if constexpr (std::is_same_v<typename SketchT::value_type, int64_t>) {
    elements.push_back(Value::Int64(sketch.get_min_item()));
    for (int64_t i = 1; i < num_quantiles; ++i) {
      const double rank =
          static_cast<double>(i) / static_cast<double>(num_quantiles);
      elements.push_back(Value::Int64(sketch.get_quantile(rank)));
    }
    elements.push_back(Value::Int64(sketch.get_max_item()));
  } else {
    elements.push_back(Value::Double(sketch.get_min_item()));
    for (int64_t i = 1; i < num_quantiles; ++i) {
      const double rank =
          static_cast<double>(i) / static_cast<double>(num_quantiles);
      elements.push_back(Value::Double(sketch.get_quantile(rank)));
    }
    elements.push_back(Value::Double(sketch.get_max_item()));
  }
  return Value::Array(arr_type, std::move(elements));
}

template <typename SketchT, SketchType TypeTag>
inline absl::StatusOr<Value> InitAggregateImpl(
    const std::vector<std::vector<Value>>& input_column_values,
    ::googlesql::TypeKind input_kind) {
  if (input_column_values.empty()) {
    return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                             "KLL_QUANTILES.INIT expects one argument");
  }
  std::optional<int64_t> precision = FirstNonNullInt64(input_column_values, 1);
  auto checked_precision = ParseAndValidatePrecision(precision);
  if (!checked_precision.ok()) return checked_precision.status();

  const uint16_t k = PrecisionToK(*checked_precision);
  SketchT sketch(k);
  bool saw_non_null = false;

  const bool has_weight_column = input_column_values.size() >= 3;
  const size_t nrows = input_column_values[0].size();
  for (size_t r = 0; r < nrows; ++r) {
    const Value& input = input_column_values[0][r];
    if (input.is_null()) continue;
    if (input.type_kind() != input_kind) {
      return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                               "KLL_QUANTILES.INIT input type mismatch");
    }
    int64_t weight = 1;
    if (has_weight_column) {
      auto parsed_weight = ParseAndValidateWeight(input_column_values[2][r]);
      if (!parsed_weight.ok()) return parsed_weight.status();
      weight = *parsed_weight;
    }
    saw_non_null = true;
    if constexpr (std::is_same_v<typename SketchT::value_type, int64_t>) {
      UpdateWithWeight(&sketch, input.int64_value(), weight);
    } else {
      const double value = input.type_kind() == ::googlesql::TYPE_DOUBLE
                               ? input.double_value()
                               : static_cast<double>(input.float_value());
      UpdateWithWeight(&sketch, value, weight);
    }
  }

  if (!saw_non_null) return Value::NullBytes();
  auto serialized = SerializeTypedSketch(sketch, TypeTag);
  if (!serialized.ok()) return serialized.status();
  return Value::Bytes(*serialized);
}

template <typename SketchT, SketchType TypeTag>
inline absl::StatusOr<SketchT> MergeSketchesImpl(
    const std::vector<std::vector<Value>>& input_column_values) {
  if (input_column_values.empty()) {
    return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                             "KLL_QUANTILES merge expects one argument");
  }

  std::optional<uint16_t> min_k;
  std::vector<SketchT> sketches;
  bool saw_non_null = false;

  for (const Value& v : input_column_values[0]) {
    if (v.is_null()) continue;
    if (v.type_kind() != ::googlesql::TYPE_BYTES) {
      return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                               "KLL_QUANTILES merge inputs must be BYTES");
    }
    auto parsed = DeserializeTypedSketch<SketchT>(v.bytes_value(), TypeTag);
    if (!parsed.ok()) return parsed.status();
    saw_non_null = true;
    const uint16_t sketch_k = parsed->get_k();
    if (!min_k.has_value() || sketch_k < *min_k) {
      min_k = sketch_k;
    }
    sketches.push_back(std::move(*parsed));
  }

  if (!saw_non_null) {
    return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                             "KLL_QUANTILES merge received only NULL sketches");
  }

  SketchT merged(*min_k);
  for (const SketchT& sketch : sketches) {
    merged.merge(sketch);
  }
  return merged;
}

template <typename SketchT, SketchType TypeTag>
inline absl::StatusOr<Value> MergePartialImpl(
    const std::vector<std::vector<Value>>& input_column_values) {
  auto merged = MergeSketchesImpl<SketchT, TypeTag>(input_column_values);
  if (!merged.ok()) {
    if (merged.status().message().find("only NULL") != std::string::npos) {
      return Value::NullBytes();
    }
    return merged.status();
  }
  auto serialized = SerializeTypedSketch(*merged, TypeTag);
  if (!serialized.ok()) return serialized.status();
  return Value::Bytes(*serialized);
}

template <typename SketchT, SketchType TypeTag>
inline absl::StatusOr<Value> ExtractScalarImpl(
    const std::vector<Value>& args, const ::googlesql::Type* return_type) {
  if (args.size() != 2) {
    return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                             "KLL_QUANTILES.EXTRACT expects two arguments");
  }
  if (args[0].is_null()) return Value::Null(return_type);
  if (args[0].type_kind() != ::googlesql::TYPE_BYTES) {
    return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                             "KLL_QUANTILES.EXTRACT expects BYTES sketch");
  }
  if (args[1].is_null()) {
    return MakeSemanticError(
        SemanticErrorReason::kInvalidArgument,
        "KLL_QUANTILES.EXTRACT num_quantiles must not be NULL");
  }
  if (args[1].type_kind() != ::googlesql::TYPE_INT64) {
    return MakeSemanticError(
        SemanticErrorReason::kInvalidArgument,
        "KLL_QUANTILES.EXTRACT num_quantiles must be INT64");
  }
  auto num_quantiles = ParseAndValidateNumQuantiles(args[1].int64_value());
  if (!num_quantiles.ok()) return num_quantiles.status();

  auto sketch = DeserializeTypedSketch<SketchT>(args[0].bytes_value(), TypeTag);
  if (!sketch.ok()) return sketch.status();
  return BuildQuantileArray(*sketch, *num_quantiles, return_type);
}

template <typename SketchT, SketchType TypeTag>
inline absl::StatusOr<Value> ExtractPointScalarImpl(
    const std::vector<Value>& args) {
  if (args.size() != 2) {
    return MakeSemanticError(
        SemanticErrorReason::kInvalidArgument,
        "KLL_QUANTILES.EXTRACT_POINT expects two arguments");
  }
  if (args[0].is_null()) {
    if constexpr (std::is_same_v<typename SketchT::value_type, int64_t>) {
      return Value::NullInt64();
    }
    return Value::NullDouble();
  }
  if (args[0].type_kind() != ::googlesql::TYPE_BYTES) {
    return MakeSemanticError(
        SemanticErrorReason::kInvalidArgument,
        "KLL_QUANTILES.EXTRACT_POINT expects BYTES sketch");
  }
  if (args[1].is_null()) {
    return MakeSemanticError(
        SemanticErrorReason::kInvalidArgument,
        "KLL_QUANTILES.EXTRACT_POINT phi must not be NULL");
  }
  auto phi = ParseAndValidatePhi(args[1]);
  if (!phi.ok()) return phi.status();

  auto sketch = DeserializeTypedSketch<SketchT>(args[0].bytes_value(), TypeTag);
  if (!sketch.ok()) return sketch.status();
  if (sketch->is_empty()) {
    return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                             "KLL_QUANTILES sketch is empty");
  }
  if constexpr (std::is_same_v<typename SketchT::value_type, int64_t>) {
    return Value::Int64(sketch->get_quantile(*phi));
  }
  return Value::Double(sketch->get_quantile(*phi));
}

template <typename SketchT, SketchType TypeTag>
inline absl::StatusOr<Value> MergeAndExtractImpl(
    const std::vector<std::vector<Value>>& input_column_values,
    const ::googlesql::Type* return_type) {
  auto merged = MergeSketchesImpl<SketchT, TypeTag>(input_column_values);
  if (!merged.ok()) {
    if (merged.status().message().find("only NULL") != std::string::npos) {
      return Value::Null(return_type);
    }
    return merged.status();
  }
  auto num_quantiles_opt = FirstNonNullInt64(input_column_values, 1);
  if (!num_quantiles_opt.has_value()) {
    return MakeSemanticError(
        SemanticErrorReason::kInvalidArgument,
        "KLL_QUANTILES.MERGE num_quantiles must not be NULL");
  }
  auto num_quantiles = ParseAndValidateNumQuantiles(*num_quantiles_opt);
  if (!num_quantiles.ok()) return num_quantiles.status();
  return BuildQuantileArray(*merged, *num_quantiles, return_type);
}

template <typename SketchT, SketchType TypeTag>
inline absl::StatusOr<Value> MergeAndExtractPointImpl(
    const std::vector<std::vector<Value>>& input_column_values) {
  auto merged = MergeSketchesImpl<SketchT, TypeTag>(input_column_values);
  if (!merged.ok()) {
    if (merged.status().message().find("only NULL") != std::string::npos) {
      if constexpr (std::is_same_v<typename SketchT::value_type, int64_t>) {
        return Value::NullInt64();
      }
      return Value::NullDouble();
    }
    return merged.status();
  }
  if (input_column_values.size() < 2 || input_column_values[1].empty()) {
    return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                             "KLL_QUANTILES.MERGE_POINT phi must not be NULL");
  }
  const Value& phi_value = input_column_values[1][0];
  if (phi_value.is_null()) {
    return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                             "KLL_QUANTILES.MERGE_POINT phi must not be NULL");
  }
  auto phi = ParseAndValidatePhi(phi_value);
  if (!phi.ok()) return phi.status();
  if (merged->is_empty()) {
    return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                             "KLL_QUANTILES sketch is empty");
  }
  if constexpr (std::is_same_v<typename SketchT::value_type, int64_t>) {
    return Value::Int64(merged->get_quantile(*phi));
  }
  return Value::Double(merged->get_quantile(*phi));
}

}  // namespace kll_detail
}  // namespace functions
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_FUNCTIONS_KLL_SKETCH_INTERNAL_H_
