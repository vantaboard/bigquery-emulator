#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "absl/status/statusor.h"
#include "absl/strings/match.h"
#include "absl/strings/string_view.h"
#include "absl/time/time.h"
#include "backend/engine/semantic/error.h"
#include "backend/engine/semantic/eval_expr_cast_format_internal.h"
#include "backend/engine/semantic/eval_expr_internal.h"
#include "backend/engine/semantic/functions/datetime_funcs_internal.h"
#include "backend/engine/semantic/value.h"
#include "googlesql/public/functions/cast_date_time.h"
#include "googlesql/public/functions/date_time_util.h"
#include "googlesql/public/type.h"
#include "googlesql/resolved_ast/resolved_ast.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace eval_expr_internal {
namespace cast_format_internal {

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

absl::StatusOr<Value> FormatDatetimeToString(const ::googlesql::ResolvedCast& cast,
                                             Value inner,
                                             absl::string_view format_str) {
  std::string out;
  if (absl::Status s = ::googlesql::functions::CastFormatDatetimeToString(
          format_str, inner.datetime_value(), &out);
      !s.ok()) {
    if (cast.return_null_on_error()) return Value::NullString();
    return s;
  }
  return Value::String(std::move(out));
}

absl::StatusOr<Value> FormatDateToString(const ::googlesql::ResolvedCast& cast,
                                         Value inner,
                                         absl::string_view format_str) {
  std::string out;
  if (absl::Status s = ::googlesql::functions::CastFormatDateToString(
          format_str, inner.date_value(), &out);
      !s.ok()) {
    if (cast.return_null_on_error()) return Value::NullString();
    return s;
  }
  return Value::String(std::move(out));
}

absl::StatusOr<Value> FormatTimestampToString(const ::googlesql::ResolvedCast& cast,
                                              Value inner,
                                              absl::string_view format_str) {
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

absl::StatusOr<Value> ParseStringToDateValue(const ::googlesql::ResolvedCast& cast,
                                             absl::string_view format_str,
                                             absl::string_view text) {
  auto current_date_or = CurrentDateValue();
  if (!current_date_or.ok()) return current_date_or.status();
  int32_t date = 0;
  if (absl::Status s = ::googlesql::functions::CastStringToDate(
          format_str, std::string(text), *current_date_or, &date);
      !s.ok()) {
    if (cast.return_null_on_error()) return Value::NullDate();
    return s;
  }
  return Value::Date(date);
}

absl::StatusOr<Value> ParseStringToDatetimeValue(
    const ::googlesql::ResolvedCast& cast,
    absl::string_view format_str,
    absl::string_view text) {
  auto current_date_or = CurrentDateValue();
  if (!current_date_or.ok()) return current_date_or.status();
  ::googlesql::DatetimeValue datetime;
  if (absl::Status s = ::googlesql::functions::CastStringToDatetime(
          format_str, std::string(text), kMicros, *current_date_or, &datetime);
      !s.ok()) {
    if (cast.return_null_on_error()) return Value::NullDatetime();
    return s;
  }
  return Value::Datetime(datetime);
}

absl::StatusOr<Value> ParseStringToTimestampValue(
    const ::googlesql::ResolvedCast& cast,
    absl::string_view format_str,
    absl::string_view text) {
  int64_t micros = 0;
  absl::Status s;
  if (cast.time_zone() != nullptr) {
    auto tz_or = CastTimeZoneLiteral(cast);
    if (!tz_or.ok()) return tz_or.status();
    s = ::googlesql::functions::CastStringToTimestamp(
        format_str, std::string(text), *tz_or, absl::Now(), &micros);
  } else {
    s = ::googlesql::functions::CastStringToTimestamp(
        format_str, std::string(text), DefaultTimeZone(), absl::Now(), &micros);
  }
  if (!s.ok()) {
    if (cast.return_null_on_error()) return Value::NullTimestamp();
    return s;
  }
  return Value::TimestampFromUnixMicros(micros);
}

absl::StatusOr<Value> TimestampToStringInZone(const ::googlesql::ResolvedCast& cast,
                                                Value inner) {
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

absl::StatusOr<Value> TimestampToDatetimeInZone(const ::googlesql::ResolvedCast& cast,
                                                Value inner) {
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

}  // namespace cast_format_internal
}  // namespace eval_expr_internal
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
