
#include "googlesql/public/functions/cast_date_time.h"
#include "googlesql/public/functions/date_time_util.h"
#include "googlesql/public/type.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace eval_expr_internal {

namespace {

using functions::datetime_internal::CurrentDateValue;
using functions::datetime_internal::DefaultTimeZone;
using functions::datetime_internal::kFormatOpts;
using functions::datetime_internal::kMicros;

absl::StatusOr<absl::string_view> CastFormatLiteral(
    const ::googlesql::ResolvedCast& cast) {
  if (cast.format() == nullptr) {
    return absl::InvalidArgumentError("semantic: ResolvedCast has null format");
  }
  if (cast.format()->node_kind() != ::googlesql::RESOLVED_LITERAL ||
      cast.format()->type()->kind() != ::googlesql::TYPE_STRING) {
    return MakeSemanticError(
        SemanticErrorReason::kNotImplemented,
        "semantic: CAST ... FORMAT requires a string literal format");
  }
  return cast.format()
      ->GetAs<::googlesql::ResolvedLiteral>()
      ->value()
      .string_value();
}

absl::StatusOr<absl::string_view> CastTimeZoneLiteral(
    const ::googlesql::ResolvedCast& cast) {
  if (cast.time_zone() == nullptr) {
    return absl::InvalidArgumentError(
        "semantic: ResolvedCast has null time_zone");
  }
  if (cast.time_zone()->node_kind() != ::googlesql::RESOLVED_LITERAL ||
      cast.time_zone()->type()->kind() != ::googlesql::TYPE_STRING) {
    return MakeSemanticError(
        SemanticErrorReason::kNotImplemented,
        "semantic: CAST ... AT TIME ZONE requires a string literal zone");
  }
  return cast.time_zone()
      ->GetAs<::googlesql::ResolvedLiteral>()
      ->value()
      .string_value();
}

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
    std::string out;
    if (absl::Status s = ::googlesql::functions::CastFormatDatetimeToString(
            format_str, inner.datetime_value(), &out);
        !s.ok()) {
      if (cast.return_null_on_error()) return Value::NullString();
      return s;
    }
    return Value::String(std::move(out));
  }
  if (inner.type_kind() == ::googlesql::TYPE_DATE) {
    std::string out;
    if (absl::Status s = ::googlesql::functions::CastFormatDateToString(
            format_str, inner.date_value(), &out);
        !s.ok()) {
      if (cast.return_null_on_error()) return Value::NullString();
      return s;
    }
    return Value::String(std::move(out));
  }
  if (inner.type_kind() == ::googlesql::TYPE_TIMESTAMP) {
    std::string out;
    absl::Status s;
    if (cast.time_zone() != nullptr) {
      auto tz_or = CastTimeZoneLiteral(cast);
      if (!tz_or.ok()) return tz_or.status();
      s = ::googlesql::functions::CastFormatTimestampToString(
          format_str, inner.ToUnixMicros(), *tz_or, &out);
    } else {
      s = ::googlesql::functions::CastFormatTimestampToString(
          format_str, inner.ToUnixMicros(), DefaultTimeZone(), &out);
    }
    if (!s.ok()) {
      if (cast.return_null_on_error()) return Value::NullString();
      return s;
    }
    return Value::String(std::move(out));
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
  const std::string text(inner.string_value());
  if (target->kind() == ::googlesql::TYPE_DATE) {
    auto current_date_or = CurrentDateValue();
    if (!current_date_or.ok()) return current_date_or.status();
    int32_t date = 0;
    if (absl::Status s = ::googlesql::functions::CastStringToDate(
            format_str, text, *current_date_or, &date);
        !s.ok()) {
      if (cast.return_null_on_error()) return Value::NullDate();
      return s;
    }
    return Value::Date(date);
  }
  if (target->kind() == ::googlesql::TYPE_DATETIME) {
    auto current_date_or = CurrentDateValue();
    if (!current_date_or.ok()) return current_date_or.status();
    ::googlesql::DatetimeValue datetime;
    if (absl::Status s = ::googlesql::functions::CastStringToDatetime(
            format_str, text, kMicros, *current_date_or, &datetime);
        !s.ok()) {
      if (cast.return_null_on_error()) return Value::NullDatetime();
      return s;
    }
    return Value::Datetime(datetime);
  }
  if (target->kind() == ::googlesql::TYPE_TIMESTAMP) {
    int64_t micros = 0;
    absl::Status s;
    if (cast.time_zone() != nullptr) {
      auto tz_or = CastTimeZoneLiteral(cast);
      if (!tz_or.ok()) return tz_or.status();
      s = ::googlesql::functions::CastStringToTimestamp(
          format_str, text, *tz_or, absl::Now(), &micros);
    } else {
      s = ::googlesql::functions::CastStringToTimestamp(
          format_str, text, DefaultTimeZone(), absl::Now(), &micros);
    }
    if (!s.ok()) {
      if (cast.return_null_on_error()) return Value::NullTimestamp();
      return s;
    }
    return Value::TimestampFromUnixMicros(micros);
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
    auto tz_or = CastTimeZoneLiteral(cast);
    if (!tz_or.ok()) return tz_or.status();
    absl::TimeZone tz;
    if (absl::Status s = ::googlesql::functions::MakeTimeZone(*tz_or, &tz);
        !s.ok()) {
      if (cast.return_null_on_error()) return Value::NullString();
      return s;
    }
    std::string out;
    if (absl::Status s = ::googlesql::functions::FormatTimestampToString(
            "%F %T%Ez", inner.ToUnixMicros(), tz, kFormatOpts, &out);
        !s.ok()) {
      if (cast.return_null_on_error()) return Value::NullString();
      return s;
    }
    if (absl::EndsWith(out, "+00:00")) {
      out.replace(out.size() - 6, 6, "+00");
    }
    return Value::String(std::move(out));
  }
  if (target->kind() == ::googlesql::TYPE_DATETIME) {
    if (inner.is_null()) return Value::NullDatetime();
    auto tz_or = CastTimeZoneLiteral(cast);
    if (!tz_or.ok()) return tz_or.status();
    absl::TimeZone tz;
    if (absl::Status s = ::googlesql::functions::MakeTimeZone(*tz_or, &tz);
        !s.ok()) {
      if (cast.return_null_on_error()) return Value::NullDatetime();
      return s;
    }
    ::googlesql::DatetimeValue datetime;
    if (absl::Status s = ::googlesql::functions::ConvertTimestampToDatetime(
            inner.ToTime(), tz, &datetime);
        !s.ok()) {
      if (cast.return_null_on_error()) return Value::NullDatetime();
      return s;
    }
    return Value::Datetime(datetime);
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
