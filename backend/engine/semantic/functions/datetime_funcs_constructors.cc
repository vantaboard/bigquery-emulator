#include <cstdint>
#include <string>

#include "absl/status/status.h"
#include "absl/strings/match.h"
#include "absl/strings/string_view.h"
#include "absl/time/time.h"
#include "backend/engine/semantic/functions/datetime_funcs_internal.h"
#include "backend/engine/semantic/value.h"
#include "googlesql/public/functions/date_time_util.h"
#include "googlesql/public/functions/datetime.pb.h"
#include "googlesql/public/functions/parse_date_time.h"
#include "googlesql/public/type.h"
#include "googlesql/public/type.pb.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace functions {

using datetime_internal::DateTimestampPart;
using datetime_internal::DatetimeValue;
using datetime_internal::DefaultTimeZone;
using datetime_internal::HasNull;
using datetime_internal::kFormatOpts;
using datetime_internal::kMicros;
using datetime_internal::TimeValue;

namespace {

absl::StatusOr<Value> TimestampFromDatetimeArgs(
    const std::vector<Value>& args) {
  absl::Time t;
  if (args.size() == 2 && args[1].type_kind() == ::googlesql::TYPE_STRING) {
    if (auto s = ::googlesql::functions::ConvertDatetimeToTimestamp(
            args[0].datetime_value(), args[1].string_value(), &t);
        !s.ok()) {
      return s;
    }
    return Value::Timestamp(t);
  }
  if (args.size() == 1) {
    if (auto s = ::googlesql::functions::ConvertDatetimeToTimestamp(
            args[0].datetime_value(), DefaultTimeZone(), &t);
        !s.ok()) {
      return s;
    }
    return Value::Timestamp(t);
  }
  return absl::InvalidArgumentError(
      "semantic: TIMESTAMP(DATETIME) expects one or two arguments");
}

absl::StatusOr<Value> TimestampFromDateArg(const Value& date_arg) {
  int64_t micros = 0;
  if (auto s = ::googlesql::functions::ConvertDateToTimestamp(
          date_arg.date_value(), kMicros, DefaultTimeZone(), &micros);
      !s.ok()) {
    return s;
  }
  return Value::TimestampFromUnixMicros(micros);
}

absl::StatusOr<Value> TimestampFromStringWithTimezone(
    absl::string_view text, absl::string_view tz_name) {
  absl::TimeZone tz;
  if (auto s = ::googlesql::functions::MakeTimeZone(tz_name, &tz); !s.ok()) {
    return s;
  }
  int64_t micros = 0;
  const std::string normalized =
      NormalizeTimestampOffsetSuffix(std::string(text));
  if (auto s = ::googlesql::functions::ConvertStringToTimestamp(
          normalized,
          tz,
          kMicros,
          /*allow_tz_in_str=*/false,
          &micros);
      !s.ok()) {
    return s;
  }
  return Value::TimestampFromUnixMicros(micros);
}

}  // namespace

absl::StatusOr<Value> DateConstructor(const std::vector<Value>& args) {
  if (HasNull(args)) return Value::NullDate();
  if (args.size() == 1 && args[0].type_kind() == ::googlesql::TYPE_STRING) {
    int32_t date = 0;
    if (auto s = ::googlesql::functions::ParseStringToDate(
            "%Y-%m-%d", args[0].string_value(), /*parse_version2=*/true, &date);
        !s.ok()) {
      return s;
    }
    return Value::Date(date);
  }
  if (args.size() == 3 && args[0].type_kind() == ::googlesql::TYPE_INT64 &&
      args[1].type_kind() == ::googlesql::TYPE_INT64 &&
      args[2].type_kind() == ::googlesql::TYPE_INT64) {
    int32_t date = 0;
    if (auto s = ::googlesql::functions::ConstructDate(
            static_cast<int>(args[0].int64_value()),
            static_cast<int>(args[1].int64_value()),
            static_cast<int>(args[2].int64_value()),
            &date);
        !s.ok()) {
      return s;
    }
    return Value::Date(date);
  }
  if (args.size() == 1 && args[0].type_kind() == ::googlesql::TYPE_DATETIME) {
    int32_t date = 0;
    if (auto s = ::googlesql::functions::ExtractFromDatetime(
            DateTimestampPart::DATE, args[0].datetime_value(), &date);
        !s.ok()) {
      return s;
    }
    return Value::Date(date);
  }
  if (args.size() == 1 && args[0].type_kind() == ::googlesql::TYPE_TIMESTAMP) {
    int32_t date = 0;
    if (auto s = ::googlesql::functions::ExtractFromTimestamp(
            DateTimestampPart::DATE,
            args[0].ToUnixMicros(),
            kMicros,
            DefaultTimeZone(),
            &date);
        !s.ok()) {
      return s;
    }
    return Value::Date(date);
  }
  if (args.size() == 2 && args[0].type_kind() == ::googlesql::TYPE_TIMESTAMP &&
      args[1].type_kind() == ::googlesql::TYPE_STRING) {
    absl::TimeZone tz;
    if (auto s =
            ::googlesql::functions::MakeTimeZone(args[1].string_value(), &tz);
        !s.ok()) {
      return s;
    }
    int32_t date = 0;
    if (auto s = ::googlesql::functions::ExtractFromTimestamp(
            DateTimestampPart::DATE,
            args[0].ToUnixMicros(),
            kMicros,
            tz,
            &date);
        !s.ok()) {
      return s;
    }
    return Value::Date(date);
  }
  return absl::InvalidArgumentError(
      "semantic: DATE constructor signature not supported");
}

absl::StatusOr<Value> DatetimeConstructor(absl::string_view name,
                                          const std::vector<Value>& args) {
  (void)name;
  if (HasNull(args)) return Value::NullDatetime();
  DatetimeValue datetime;
  if (args.size() == 6) {
    if (auto s = ::googlesql::functions::ConstructDatetime(
            static_cast<int>(args[0].int64_value()),
            static_cast<int>(args[1].int64_value()),
            static_cast<int>(args[2].int64_value()),
            static_cast<int>(args[3].int64_value()),
            static_cast<int>(args[4].int64_value()),
            static_cast<int>(args[5].int64_value()),
            &datetime);
        !s.ok()) {
      return s;
    }
    return Value::Datetime(datetime);
  }
  if (args.size() == 2 && args[0].type_kind() == ::googlesql::TYPE_TIMESTAMP &&
      args[1].type_kind() == ::googlesql::TYPE_STRING) {
    absl::TimeZone tz;
    if (auto s =
            ::googlesql::functions::MakeTimeZone(args[1].string_value(), &tz);
        !s.ok()) {
      return s;
    }
    if (auto s = ::googlesql::functions::ConvertTimestampToDatetime(
            args[0].ToTime(), tz, &datetime);
        !s.ok()) {
      return s;
    }
    return Value::Datetime(datetime);
  }
  if (args.size() == 1 && args[0].type_kind() == ::googlesql::TYPE_TIMESTAMP) {
    if (auto s = ::googlesql::functions::ConvertTimestampToDatetime(
            args[0].ToTime(), DefaultTimeZone(), &datetime);
        !s.ok()) {
      return s;
    }
    return Value::Datetime(datetime);
  }
  return absl::InvalidArgumentError(
      "semantic: DATETIME constructor signature not supported");
}

absl::StatusOr<Value> StringFunc(const std::vector<Value>& args) {
  if (args.size() != 2) {
    return absl::InvalidArgumentError(
        "semantic: STRING expects exactly two arguments");
  }
  if (args[0].is_null() || args[1].is_null()) return Value::NullString();
  if (args[0].type_kind() != ::googlesql::TYPE_TIMESTAMP ||
      args[1].type_kind() != ::googlesql::TYPE_STRING) {
    return absl::InvalidArgumentError(
        "semantic: STRING(TIMESTAMP, STRING) signature not supported");
  }
  absl::TimeZone tz;
  if (auto s =
          ::googlesql::functions::MakeTimeZone(args[1].string_value(), &tz);
      !s.ok()) {
    return s;
  }
  std::string out;
  if (auto s = ::googlesql::functions::FormatTimestampToString(
          "%F %T%Ez", args[0].ToUnixMicros(), tz, kFormatOpts, &out);
      !s.ok()) {
    return s;
  }
  if (absl::EndsWith(out, "+00:00")) {
    out.replace(out.size() - 6, 6, "+00");
  }
  return Value::String(std::move(out));
}

absl::StatusOr<Value> TimeConstructor(const std::vector<Value>& args) {
  if (HasNull(args)) return Value::NullTime();
  if (args.size() == 1 && args[0].type_kind() == ::googlesql::TYPE_DATETIME) {
    TimeValue t;
    if (auto s = ::googlesql::functions::ExtractTimeFromDatetime(
            args[0].datetime_value(), &t);
        !s.ok()) {
      return s;
    }
    return Value::Time(t);
  }
  if (args.size() == 3 && args[0].type_kind() == ::googlesql::TYPE_INT64 &&
      args[1].type_kind() == ::googlesql::TYPE_INT64 &&
      args[2].type_kind() == ::googlesql::TYPE_INT64) {
    TimeValue t;
    if (auto s = ::googlesql::functions::ConstructTime(
            static_cast<int>(args[0].int64_value()),
            static_cast<int>(args[1].int64_value()),
            static_cast<int>(args[2].int64_value()),
            &t);
        !s.ok()) {
      return s;
    }
    return Value::Time(t);
  }
  if (args.size() == 2 && args[0].type_kind() == ::googlesql::TYPE_TIMESTAMP &&
      args[1].type_kind() == ::googlesql::TYPE_STRING) {
    absl::TimeZone tz;
    if (auto s =
            ::googlesql::functions::MakeTimeZone(args[1].string_value(), &tz);
        !s.ok()) {
      return s;
    }
    TimeValue t;
    if (auto s = ::googlesql::functions::ConvertTimestampToTime(
            args[0].ToTime(), tz, &t);
        !s.ok()) {
      return s;
    }
    return Value::Time(t);
  }
  return absl::InvalidArgumentError(
      "semantic: TIME constructor signature not supported");
}

absl::StatusOr<Value> TimestampConstructor(absl::string_view name,
                                           const std::vector<Value>& args) {
  (void)name;
  if (HasNull(args)) return Value::NullTimestamp();
  if (args.size() == 1 && args[0].type_kind() == ::googlesql::TYPE_TIMESTAMP) {
    return args[0];
  }
  if (args[0].type_kind() == ::googlesql::TYPE_DATETIME) {
    return TimestampFromDatetimeArgs(args);
  }
  if (args.size() == 1 && args[0].type_kind() == ::googlesql::TYPE_DATE) {
    return TimestampFromDateArg(args[0]);
  }
  if (args.size() == 2 && args[0].type_kind() == ::googlesql::TYPE_DATETIME &&
      args[1].type_kind() == ::googlesql::TYPE_STRING) {
    return TimestampFromDatetimeArgs(args);
  }
  if (args.size() == 2 && args[0].type_kind() == ::googlesql::TYPE_STRING &&
      args[1].type_kind() == ::googlesql::TYPE_STRING) {
    return TimestampFromStringWithTimezone(args[0].string_value(),
                                           args[1].string_value());
  }
  if (args.size() == 1 && args[0].type_kind() == ::googlesql::TYPE_STRING) {
    return ParseTimestampWireString(args[0].string_value());
  }
  return absl::InvalidArgumentError(
      "semantic: TIMESTAMP constructor signature not supported");
}

bool TryParseIsoDateString(const Value& v, Value* out) {
  if (out == nullptr || v.type_kind() != ::googlesql::TYPE_STRING) {
    return false;
  }
  int32_t date = 0;
  if (auto s = ::googlesql::functions::ParseStringToDate(
          "%Y-%m-%d", v.string_value(), /*parse_version2=*/true, &date);
      !s.ok()) {
    return false;
  }
  *out = Value::Date(date);
  return true;
}

}  // namespace functions
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
