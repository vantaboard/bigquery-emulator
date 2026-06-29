#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "absl/status/statusor.h"
#include "absl/strings/numbers.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "backend/engine/semantic/error.h"
#include "backend/engine/semantic/eval_expr_internal.h"
#include "backend/engine/semantic/value.h"
#include "googlesql/public/functions/parse_date_time.h"
#include "googlesql/public/functions/string.h"
#include "googlesql/public/type.h"
#include "googlesql/public/types/struct_type.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace eval_expr_internal {

namespace {

bool StructTypesCompatibleByPosition(const ::googlesql::StructType* source,
                                     const ::googlesql::StructType* target) {
  if (source == nullptr || target == nullptr) return false;
  if (source->num_fields() != target->num_fields()) return false;
  for (int i = 0; i < source->num_fields(); ++i) {
    if (!source->field(i).type->Equals(target->field(i).type)) {
      return false;
    }
  }
  return true;
}

std::optional<absl::StatusOr<Value>> TryCastStringBytesPair(
    Value inner, const ::googlesql::Type* target, bool return_null_on_error) {
  if (target->kind() == ::googlesql::TYPE_BYTES &&
      inner.type_kind() == ::googlesql::TYPE_STRING) {
    return Value::Bytes(std::string(inner.string_value()));
  }
  if (target->kind() == ::googlesql::TYPE_STRING &&
      inner.type_kind() == ::googlesql::TYPE_BYTES) {
    absl::Status error;
    std::string out;
    if (!::googlesql::functions::SafeConvertBytes(
            inner.bytes_value(), &out, &error)) {
      if (return_null_on_error) return NullOfType(target);
      return error;
    }
    return Value::String(std::move(out));
  }
  if (target->kind() == ::googlesql::TYPE_STRING &&
      inner.type_kind() == ::googlesql::TYPE_INT64) {
    return Value::String(absl::StrCat(inner.int64_value()));
  }
  if (target->kind() == ::googlesql::TYPE_DATE &&
      inner.type_kind() == ::googlesql::TYPE_STRING) {
    int32_t date = 0;
    const std::string text(inner.string_value());
    if (auto s = ::googlesql::functions::ParseStringToDate(
            "%Y-%m-%d", text, /*parse_version2=*/true, &date);
        s.ok()) {
      return Value::Date(date);
    }
    if (auto s = ::googlesql::functions::ParseStringToDate(
            "%F", text, /*parse_version2=*/true, &date);
        s.ok()) {
      return Value::Date(date);
    }
    if (return_null_on_error) return NullOfType(target);
    return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                             absl::StrCat("semantic: CAST STRING to DATE "
                                          "failed for '",
                                          text,
                                          "'"));
  }
  return std::nullopt;
}

std::optional<absl::StatusOr<Value>> TryCastArrayElements(
    Value inner,
    const ::googlesql::Type* source,
    const ::googlesql::Type* target,
    bool return_null_on_error);

std::optional<absl::StatusOr<Value>> TryCastStructFields(
    Value inner,
    const ::googlesql::Type* source,
    const ::googlesql::Type* target,
    bool return_null_on_error);

}  // namespace

std::optional<absl::StatusOr<Value>> TryCastValueToType(
    Value inner,
    const ::googlesql::Type* source,
    const ::googlesql::Type* target,
    bool return_null_on_error) {
  if (target == nullptr) {
    return absl::InvalidArgumentError("semantic: cast target type is null");
  }
  if (source != nullptr && source->Equals(target)) {
    return inner;
  }
  if (inner.is_null()) return NullOfType(target);

  if (auto cast = TryCastStringBytesPair(inner, target, return_null_on_error)) {
    return *cast;
  }
  if (auto cast =
          TryCastArrayElements(inner, source, target, return_null_on_error)) {
    return *cast;
  }
  if (auto cast =
          TryCastStructFields(inner, source, target, return_null_on_error)) {
    return *cast;
  }

  return std::nullopt;
}

namespace {

std::optional<absl::StatusOr<Value>> TryCastArrayElements(
    Value inner,
    const ::googlesql::Type* source,
    const ::googlesql::Type* target,
    bool return_null_on_error) {
  if (!target->IsArray() || !inner.type()->IsArray()) return std::nullopt;
  const ::googlesql::ArrayType* target_arr = target->AsArray();
  const ::googlesql::ArrayType* source_arr = inner.type()->AsArray();
  const ::googlesql::Type* target_elem = target_arr->element_type();
  const ::googlesql::Type* source_elem = source_arr->element_type();
  if (target_elem == nullptr || source_elem == nullptr) {
    return absl::InvalidArgumentError(
        "semantic: cast array missing element type");
  }
  if (source_elem->Equals(target_elem)) {
    return inner;
  }
  std::vector<Value> elements;
  elements.reserve(inner.num_elements());
  for (int i = 0; i < inner.num_elements(); ++i) {
    auto cast_elem = TryCastValueToType(
        inner.element(i), source_elem, target_elem, return_null_on_error);
    if (!cast_elem.has_value()) {
      return std::nullopt;
    }
    if (!cast_elem->ok()) return cast_elem->status();
    elements.push_back(*std::move(*cast_elem));
  }
  return Value::Array(target_arr, std::move(elements));
}

std::optional<absl::StatusOr<Value>> TryCastStructFields(
    Value inner,
    const ::googlesql::Type* source,
    const ::googlesql::Type* target,
    bool return_null_on_error) {
  if (!target->IsStruct() || !inner.type()->IsStruct()) return std::nullopt;
  const ::googlesql::StructType* target_st = target->AsStruct();
  const ::googlesql::StructType* source_st = inner.type()->AsStruct();
  if (!StructTypesCompatibleByPosition(source_st, target_st)) {
    if (return_null_on_error) return NullOfType(target);
    return MakeSemanticError(
        SemanticErrorReason::kNotImplemented,
        absl::StrCat("semantic: CAST from ",
                     source != nullptr ? source->DebugString() : "<null>",
                     " to ",
                     target->DebugString(),
                     " is not yet implemented"));
  }
  std::vector<Value> fields;
  fields.reserve(inner.num_fields());
  for (int i = 0; i < inner.num_fields(); ++i) {
    fields.push_back(inner.field(i));
  }
  return Value::Struct(target_st, std::move(fields));
}

}  // namespace

}  // namespace eval_expr_internal
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
