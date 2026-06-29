

#include "googlesql/public/type.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace functions {

namespace kll = kll_detail;

absl::StatusOr<Value> KllQuantilesInitInt64Aggregate(
    const ::googlesql::ResolvedAggregateFunctionCall& call,
    const std::vector<std::vector<Value>>& input_column_values) {
  (void)call;
  std::optional<int64_t> precision =
      kll::FirstNonNullInt64(input_column_values, 1);
  const std::vector<Value>* weights =
      input_column_values.size() >= 3 ? &input_column_values[2] : nullptr;
  return KllQuantilesInitInt64Values(
      input_column_values[0], precision, weights);
}

absl::StatusOr<Value> KllQuantilesInitFloat64Aggregate(
    const ::googlesql::ResolvedAggregateFunctionCall& call,
    const std::vector<std::vector<Value>>& input_column_values) {
  (void)call;
  std::optional<int64_t> precision =
      kll::FirstNonNullInt64(input_column_values, 1);
  const std::vector<Value>* weights =
      input_column_values.size() >= 3 ? &input_column_values[2] : nullptr;
  return KllQuantilesInitFloat64Values(
      input_column_values[0], precision, weights);
}

absl::StatusOr<Value> KllQuantilesInitInt64Values(
    const std::vector<Value>& input_values,
    std::optional<int64_t> precision,
    const std::vector<Value>* weights) {
  std::vector<std::vector<Value>> columns = {input_values};
  if (precision.has_value()) {
    columns.push_back({Value::Int64(*precision)});
  }
  if (weights != nullptr) {
    if (!precision.has_value()) {
      columns.push_back({Value::Int64(kll::kDefaultPrecision)});
    }
    columns.push_back(*weights);
  }
  return kll::InitAggregateImpl<kll::Int64Sketch, kll::SketchType::kInt64>(
      columns, ::googlesql::TYPE_INT64);
}

absl::StatusOr<Value> KllQuantilesInitFloat64Values(
    const std::vector<Value>& input_values,
    std::optional<int64_t> precision,
    const std::vector<Value>* weights) {
  std::vector<std::vector<Value>> columns = {input_values};
  if (precision.has_value()) {
    columns.push_back({Value::Int64(*precision)});
  }
  if (weights != nullptr) {
    if (!precision.has_value()) {
      columns.push_back({Value::Int64(kll::kDefaultPrecision)});
    }
    columns.push_back(*weights);
  }
  return kll::InitAggregateImpl<kll::Float64Sketch, kll::SketchType::kFloat64>(
      columns, ::googlesql::TYPE_DOUBLE);
}

absl::StatusOr<Value> KllQuantilesMergePartialAggregate(
    const std::vector<std::vector<Value>>& input_column_values) {
  if (input_column_values.empty() || input_column_values[0].empty()) {
    return Value::NullBytes();
  }
  const Value& first_non_null = [&]() -> const Value& {
    for (const Value& v : input_column_values[0]) {
      if (!v.is_null()) return v;
    }
    return input_column_values[0][0];
  }();
  if (first_non_null.is_null()) return Value::NullBytes();

  auto envelope = kll::ParseSketchEnvelope(first_non_null.bytes_value());
  if (!envelope.ok()) return envelope.status();
  if (envelope->first == kll::SketchType::kInt64) {
    return kll::MergePartialImpl<kll::Int64Sketch, kll::SketchType::kInt64>(
        input_column_values);
  }
  return kll::MergePartialImpl<kll::Float64Sketch, kll::SketchType::kFloat64>(
      input_column_values);
}

absl::StatusOr<Value> KllQuantilesMergeInt64Aggregate(
    const std::vector<std::vector<Value>>& input_column_values,
    const ::googlesql::Type* return_type) {
  return kll::MergeAndExtractImpl<kll::Int64Sketch, kll::SketchType::kInt64>(
      input_column_values, return_type);
}

absl::StatusOr<Value> KllQuantilesMergeFloat64Aggregate(
    const std::vector<std::vector<Value>>& input_column_values,
    const ::googlesql::Type* return_type) {
  return kll::MergeAndExtractImpl<kll::Float64Sketch,
                                  kll::SketchType::kFloat64>(
      input_column_values, return_type);
}

absl::StatusOr<Value> KllQuantilesMergePointInt64Aggregate(
    const std::vector<std::vector<Value>>& input_column_values) {
  return kll::MergeAndExtractPointImpl<kll::Int64Sketch,
                                       kll::SketchType::kInt64>(
      input_column_values);
}

absl::StatusOr<Value> KllQuantilesMergePointFloat64Aggregate(
    const std::vector<std::vector<Value>>& input_column_values) {
  return kll::MergeAndExtractPointImpl<kll::Float64Sketch,
                                       kll::SketchType::kFloat64>(
      input_column_values);
}

absl::StatusOr<Value> KllQuantilesExtractInt64Scalar(
    const std::vector<Value>& args, const ::googlesql::Type* return_type) {
  return kll::ExtractScalarImpl<kll::Int64Sketch, kll::SketchType::kInt64>(
      args, return_type);
}

absl::StatusOr<Value> KllQuantilesExtractFloat64Scalar(
    const std::vector<Value>& args, const ::googlesql::Type* return_type) {
  return kll::ExtractScalarImpl<kll::Float64Sketch, kll::SketchType::kFloat64>(
      args, return_type);
}

absl::StatusOr<Value> KllQuantilesExtractPointInt64Scalar(
    const std::vector<Value>& args) {
  return kll::ExtractPointScalarImpl<kll::Int64Sketch, kll::SketchType::kInt64>(
      args);
}

absl::StatusOr<Value> KllQuantilesExtractPointFloat64Scalar(
    const std::vector<Value>& args) {
  return kll::ExtractPointScalarImpl<kll::Float64Sketch,
                                     kll::SketchType::kFloat64>(args);
}

}  // namespace functions
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
