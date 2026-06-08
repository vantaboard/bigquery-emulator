#include "backend/engine/semantic/functions/array_funcs.h"

#include <cstdint>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"
#include "backend/engine/semantic/error.h"
#include "backend/engine/semantic/value.h"
#include "googlesql/public/functions/generate_array.h"
#include "googlesql/public/type.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace functions {

namespace {

using ::googlesql::Value;

bool HasNull(const std::vector<Value>& args) {
  for (const auto& v : args) {
    if (v.is_null()) return true;
  }
  return false;
}

std::optional<Value> NullIfAny(const std::vector<Value>& args,
                               const ::googlesql::Type* return_type) {
  for (const auto& v : args) {
    if (v.is_null()) {
      if (return_type != nullptr) {
        return Value::Null(return_type);
      }
      return std::nullopt;
    }
  }
  return std::nullopt;
}

Value GenerateArrayElement(const ::googlesql::Type* element_type, int64_t v) {
  switch (element_type->kind()) {
    case ::googlesql::TYPE_INT64:
      return Value::Int64(v);
    case ::googlesql::TYPE_DOUBLE:
      return Value::Double(static_cast<double>(v));
    case ::googlesql::TYPE_FLOAT:
      return Value::Float(static_cast<float>(v));
    default:
      break;
  }
  if (element_type->IsFloatingPoint()) {
    return Value::Double(static_cast<double>(v));
  }
  return Value::Int64(v);
}

absl::StatusOr<int64_t> Int64Arg(const Value& v, absl::string_view what) {
  if (v.type_kind() == ::googlesql::TYPE_INT64) {
    return v.int64_value();
  }
  if (v.type_kind() == ::googlesql::TYPE_DOUBLE ||
      v.type_kind() == ::googlesql::TYPE_FLOAT) {
    const double d = v.type_kind() == ::googlesql::TYPE_DOUBLE
                         ? v.double_value()
                         : static_cast<double>(v.float_value());
    return static_cast<int64_t>(d);
  }
  return absl::InvalidArgumentError(
      absl::StrCat("semantic: ",
                   what,
                   " requires INT64-compatible argument; got ",
                   v.type()->DebugString()));
}

absl::StatusOr<const ::googlesql::ArrayType*> ExpectArrayType(
    const Value& v, absl::string_view what) {
  if (!v.type()->IsArray()) {
    return absl::InvalidArgumentError(
        absl::StrCat("semantic: ",
                     what,
                     " requires ARRAY argument; got ",
                     v.type()->DebugString()));
  }
  return v.type()->AsArray();
}

}  // namespace

absl::StatusOr<Value> GenerateArray(const std::vector<Value>& args,
                                    const ::googlesql::Type* return_type) {
  if (args.size() < 2 || args.size() > 3) {
    return absl::InvalidArgumentError(
        "semantic: GENERATE_ARRAY expects 2 or 3 arguments");
  }
  if (args[0].is_null() || args[1].is_null()) {
    if (auto n = NullIfAny(args, return_type)) return *n;
  }
  if (args.size() == 3 && args[2].is_null()) {
    if (auto n = NullIfAny(args, return_type)) return *n;
  }
  auto start = Int64Arg(args[0], "GENERATE_ARRAY start");
  if (!start.ok()) return start.status();
  auto end = Int64Arg(args[1], "GENERATE_ARRAY end");
  if (!end.ok()) return end.status();
  int64_t step = 1;
  if (args.size() == 3) {
    auto step_or = Int64Arg(args[2], "GENERATE_ARRAY step");
    if (!step_or.ok()) return step_or.status();
    step = *step_or;
  }
  std::vector<int64_t> raw;
  if (auto s = ::googlesql::functions::GenerateArray<int64_t, int64_t>(
          *start, *end, step, &raw);
      !s.ok()) {
    return s;
  }
  if (return_type == nullptr || !return_type->IsArray()) {
    return absl::InvalidArgumentError(
        "semantic: GENERATE_ARRAY requires ARRAY return type");
  }
  const ::googlesql::ArrayType* arr_type = return_type->AsArray();
  const ::googlesql::Type* element_type = arr_type->element_type();
  std::vector<Value> elems;
  elems.reserve(raw.size());
  for (int64_t v : raw) {
    elems.push_back(GenerateArrayElement(element_type, v));
  }
  return Value::Array(arr_type, std::move(elems));
}

absl::StatusOr<Value> ArrayConcat(const std::vector<Value>& args,
                                  const ::googlesql::Type* return_type) {
  if (args.empty()) {
    return absl::InvalidArgumentError(
        "semantic: ARRAY_CONCAT expects at least one argument");
  }
  if (HasNull(args)) {
    if (auto n = NullIfAny(args, return_type)) return *n;
  }
  if (return_type == nullptr || !return_type->IsArray()) {
    return absl::InvalidArgumentError(
        "semantic: ARRAY_CONCAT requires ARRAY return type");
  }
  const ::googlesql::ArrayType* out_type = return_type->AsArray();
  std::vector<Value> merged;
  for (const Value& arr : args) {
    auto at = ExpectArrayType(arr, "ARRAY_CONCAT");
    if (!at.ok()) return at.status();
    for (int i = 0; i < arr.num_elements(); ++i) {
      merged.push_back(arr.element(i));
    }
  }
  return Value::Array(out_type, std::move(merged));
}

absl::StatusOr<Value> ArrayLength(const std::vector<Value>& args) {
  if (args.size() != 1) {
    return absl::InvalidArgumentError(
        "semantic: ARRAY_LENGTH expects exactly one argument");
  }
  if (args[0].is_null()) return Value::NullInt64();
  auto at = ExpectArrayType(args[0], "ARRAY_LENGTH");
  if (!at.ok()) return at.status();
  return Value::Int64(args[0].num_elements());
}

absl::StatusOr<Value> ArrayReverse(const std::vector<Value>& args,
                                   const ::googlesql::Type* return_type) {
  if (args.size() != 1) {
    return absl::InvalidArgumentError(
        "semantic: ARRAY_REVERSE expects exactly one argument");
  }
  if (args[0].is_null()) {
    if (return_type != nullptr) return Value::Null(return_type);
    return Value::NullInt64();
  }
  auto at = ExpectArrayType(args[0], "ARRAY_REVERSE");
  if (!at.ok()) return at.status();
  if (return_type == nullptr || !return_type->IsArray()) {
    return absl::InvalidArgumentError(
        "semantic: ARRAY_REVERSE requires ARRAY return type");
  }
  std::vector<Value> elems;
  elems.reserve(args[0].num_elements());
  for (int i = args[0].num_elements() - 1; i >= 0; --i) {
    elems.push_back(args[0].element(i));
  }
  return Value::Array(return_type->AsArray(), std::move(elems));
}

absl::StatusOr<Value> ArrayAtOffset(const std::vector<Value>& args,
                                    const ::googlesql::Type* return_type,
                                    bool safe) {
  if (args.size() != 2) {
    return absl::InvalidArgumentError(
        "semantic: $array_at_offset expects array and offset");
  }
  if (args[0].is_null() || args[1].is_null()) {
    if (return_type != nullptr) return Value::Null(return_type);
    return Value::NullInt64();
  }
  auto at = ExpectArrayType(args[0], "$array_at_offset");
  if (!at.ok()) return at.status();
  if (args[1].type_kind() != ::googlesql::TYPE_INT64) {
    return absl::InvalidArgumentError(
        "semantic: $array_at_offset offset must be INT64");
  }
  const int64_t offset = args[1].int64_value();
  if (offset < 0 || offset >= args[0].num_elements()) {
    if (safe) {
      if (return_type != nullptr) return Value::Null(return_type);
      return Value::NullInt64();
    }
    return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                             "semantic: array offset out of bounds");
  }
  return args[0].element(static_cast<int>(offset));
}

absl::StatusOr<Value> ArrayAtOrdinal(const std::vector<Value>& args,
                                     const ::googlesql::Type* return_type,
                                     bool safe) {
  if (args.size() != 2) {
    return absl::InvalidArgumentError(
        "semantic: array ordinal access expects array and ordinal");
  }
  if (args[0].is_null() || args[1].is_null()) {
    if (return_type != nullptr) return Value::Null(return_type);
    return Value::NullInt64();
  }
  auto at = ExpectArrayType(args[0], "$array_at_ordinal");
  if (!at.ok()) return at.status();
  if (args[1].type_kind() != ::googlesql::TYPE_INT64) {
    return absl::InvalidArgumentError(
        "semantic: array ordinal access index must be INT64");
  }
  const int64_t ordinal = args[1].int64_value();
  const int64_t idx = ordinal - 1;
  if (idx < 0 || idx >= args[0].num_elements()) {
    if (safe) {
      if (return_type != nullptr) return Value::Null(return_type);
      return Value::NullInt64();
    }
    return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                             "semantic: array ordinal out of bounds");
  }
  return args[0].element(static_cast<int>(idx));
}

absl::StatusOr<Value> ArrayToString(const std::vector<Value>& args) {
  if (args.size() < 2 || args.size() > 3) {
    return absl::InvalidArgumentError(
        "semantic: ARRAY_TO_STRING expects two or three arguments");
  }
  if (args[0].is_null() || args[1].is_null()) {
    return Value::NullString();
  }
  auto at = ExpectArrayType(args[0], "ARRAY_TO_STRING");
  if (!at.ok()) return at.status();
  if (args[1].type_kind() != ::googlesql::TYPE_STRING) {
    return absl::InvalidArgumentError(
        "semantic: ARRAY_TO_STRING delimiter must be STRING");
  }
  const absl::string_view delimiter = args[1].string_value();
  const bool replace_nulls = args.size() == 3;
  if (replace_nulls && args[2].is_null()) {
    return Value::NullString();
  }
  const absl::string_view null_replacement =
      replace_nulls ? args[2].string_value() : absl::string_view();
  std::vector<std::string> parts;
  parts.reserve(args[0].num_elements());
  for (int i = 0; i < args[0].num_elements(); ++i) {
    const Value& elem = args[0].element(i);
    if (elem.is_null()) {
      if (replace_nulls) {
        parts.emplace_back(null_replacement);
      }
      continue;
    }
    if (elem.type_kind() != ::googlesql::TYPE_STRING) {
      return absl::InvalidArgumentError(
          "semantic: ARRAY_TO_STRING expects ARRAY<STRING>");
    }
    parts.emplace_back(elem.string_value());
  }
  return Value::String(absl::StrJoin(parts, delimiter));
}

}  // namespace functions
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
