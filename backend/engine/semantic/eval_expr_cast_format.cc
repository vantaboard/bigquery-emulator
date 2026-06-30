#include <string>
#include <utility>

#include "absl/status/statusor.h"
#include "backend/engine/semantic/eval_expr_cast_format_internal.h"
#include "backend/engine/semantic/eval_expr_internal.h"
#include "backend/engine/semantic/value.h"
#include "googlesql/public/type.h"
#include "googlesql/resolved_ast/resolved_ast.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace eval_expr_internal {

namespace {

using cast_format_internal::CastFormatLiteral;
using cast_format_internal::FormatDatetimeToString;
using cast_format_internal::FormatDateToString;
using cast_format_internal::FormatTimestampToString;
using cast_format_internal::ParseStringToDatetimeValue;
using cast_format_internal::ParseStringToDateValue;
using cast_format_internal::ParseStringToTimestampValue;
using cast_format_internal::TimestampToDatetimeInZone;
using cast_format_internal::TimestampToStringInZone;

std::optional<absl::StatusOr<Value>> TryEvalCastFormatToString(
    const ::googlesql::ResolvedCast& cast, Value inner) {
  if (cast.format() == nullptr ||
      cast.type()->kind() != ::googlesql::TYPE_STRING) {
    return std::nullopt;
  }
  if (inner.is_null()) return Value::NullString();
  auto format_or = CastFormatLiteral(cast);
  if (!format_or.ok()) return format_or.status();
  const absl::string_view format_str = *format_or;
  if (inner.type_kind() == ::googlesql::TYPE_DATETIME) {
    return FormatDatetimeToString(cast, std::move(inner), format_str);
  }
  if (inner.type_kind() == ::googlesql::TYPE_DATE) {
    return FormatDateToString(cast, std::move(inner), format_str);
  }
  if (inner.type_kind() == ::googlesql::TYPE_TIMESTAMP) {
    return FormatTimestampToString(cast, std::move(inner), format_str);
  }
  return std::nullopt;
}

std::optional<absl::StatusOr<Value>> TryEvalCastFormatFromString(
    const ::googlesql::ResolvedCast& cast,
    Value inner,
    const ::googlesql::Type* target) {
  if (cast.format() == nullptr ||
      inner.type_kind() != ::googlesql::TYPE_STRING) {
    return std::nullopt;
  }
  if (inner.is_null()) return NullOfType(target);
  auto format_or = CastFormatLiteral(cast);
  if (!format_or.ok()) return format_or.status();
  const absl::string_view format_str = *format_or;
  const absl::string_view text(inner.string_value());
  if (target->kind() == ::googlesql::TYPE_DATE) {
    return ParseStringToDateValue(cast, format_str, text);
  }
  if (target->kind() == ::googlesql::TYPE_DATETIME) {
    return ParseStringToDatetimeValue(cast, format_str, text);
  }
  if (target->kind() == ::googlesql::TYPE_TIMESTAMP) {
    return ParseStringToTimestampValue(cast, format_str, text);
  }
  return std::nullopt;
}

std::optional<absl::StatusOr<Value>> TryEvalCastTimestampTimezone(
    const ::googlesql::ResolvedCast& cast,
    Value inner,
    const ::googlesql::Type* target) {
  if (cast.format() != nullptr || cast.time_zone() == nullptr ||
      inner.type_kind() != ::googlesql::TYPE_TIMESTAMP) {
    return std::nullopt;
  }
  if (target->kind() == ::googlesql::TYPE_STRING) {
    if (inner.is_null()) return Value::NullString();
    return TimestampToStringInZone(cast, std::move(inner));
  }
  if (target->kind() == ::googlesql::TYPE_DATETIME) {
    if (inner.is_null()) return Value::NullDatetime();
    return TimestampToDatetimeInZone(cast, std::move(inner));
  }
  return std::nullopt;
}

}  // namespace

std::optional<absl::StatusOr<Value>> TryEvalCastFormatAndTimezone(
    const ::googlesql::ResolvedCast& cast,
    Value inner,
    const ::googlesql::Type* target) {
  if (target == nullptr) {
    return absl::InvalidArgumentError("semantic: ResolvedCast has null type");
  }
  if (auto to_string = TryEvalCastFormatToString(cast, inner)) {
    return *to_string;
  }
  if (auto from_string = TryEvalCastFormatFromString(cast, inner, target)) {
    return *from_string;
  }
  if (auto tz_cast = TryEvalCastTimestampTimezone(cast, inner, target)) {
    return *tz_cast;
  }
  return std::nullopt;
}

}  // namespace eval_expr_internal
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
