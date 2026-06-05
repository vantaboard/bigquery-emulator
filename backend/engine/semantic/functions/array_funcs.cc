#include "backend/engine/semantic/functions/array_funcs.h"

#include <cstdint>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
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
  const int64_t start = args[0].int64_value();
  const int64_t end = args[1].int64_value();
  int64_t step = 1;
  if (args.size() == 3) {
    step = args[2].int64_value();
  }
  std::vector<int64_t> raw;
  if (auto s = ::googlesql::functions::GenerateArray<int64_t, int64_t>(
          start, end, step, &raw);
      !s.ok()) {
    return s;
  }
  if (return_type == nullptr || !return_type->IsArray()) {
    return absl::InvalidArgumentError(
        "semantic: GENERATE_ARRAY requires ARRAY return type");
  }
  const ::googlesql::ArrayType* arr_type = return_type->AsArray();
  std::vector<Value> elems;
  elems.reserve(raw.size());
  for (int64_t v : raw) {
    elems.push_back(Value::Int64(v));
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

}  // namespace functions
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
