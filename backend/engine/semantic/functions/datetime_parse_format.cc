#include <cstdint>
#include <string>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/match.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "absl/time/time.h"
#include "backend/engine/semantic/error.h"
#include "backend/engine/semantic/functions/datetime_funcs.h"
#include "backend/engine/semantic/functions/datetime_funcs_internal.h"
#include "backend/engine/semantic/value.h"
#include "googlesql/public/functions/date_time_util.h"
#include "googlesql/public/functions/datetime.pb.h"
#include "googlesql/public/functions/generate_array.h"
#include "googlesql/public/functions/parse_date_time.h"
#include "googlesql/public/interval_value.h"
#include "googlesql/public/type.h"
#include "googlesql/public/type.pb.h"
#include "googlesql/public/value.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace functions {

using datetime_internal::DatetimeValue;
using datetime_internal::DefaultTimeZone;
using datetime_internal::HasNull;
using datetime_internal::kFormatOpts;
using datetime_internal::kMicros;
using datetime_internal::TimeValue;
using ::googlesql::Value;

absl::StatusOr<Value> ParseDate(const std::vector<Value>& args) {
  if (args.size() != 2) {
    return absl::InvalidArgumentError(
        "semantic: PARSE_DATE expects exactly two arguments");
  }
  if (HasNull(args)) return Value::NullDate();
  int32_t date = 0;
  if (auto s =
          ::googlesql::functions::ParseStringToDate(args[0].string_value(),
                                                    args[1].string_value(),
                                                    /*parse_version2=*/true,
                                                    &date);
      !s.ok()) {
    return s;
  }
  return Value::Date(date);
}

absl::StatusOr<Value> ParseDatetime(const std::vector<Value>& args) {
  if (args.size() != 2) {
    return absl::InvalidArgumentError(
        "semantic: PARSE_DATETIME expects exactly two arguments");
  }
  if (HasNull(args)) return Value::NullDatetime();
  DatetimeValue datetime;
  if (auto s =
          ::googlesql::functions::ParseStringToDatetime(args[0].string_value(),
                                                        args[1].string_value(),
                                                        kMicros,
                                                        /*parse_version2=*/true,
                                                        &datetime);
      !s.ok()) {
    return s;
  }
  return Value::Datetime(datetime);
}

absl::StatusOr<Value> ParseTimestamp(const std::vector<Value>& args) {
  if (args.size() < 2 || args.size() > 4) {
    return absl::InvalidArgumentError(
        "semantic: PARSE_TIMESTAMP expects 2 to 4 arguments");
  }
  if (HasNull(args)) return Value::NullTimestamp();
  int64_t precision = 6;
  std::optional<absl::string_view> timezone;
  if (args.size() >= 3) {
    if (args[2].type_kind() == ::googlesql::TYPE_INT64) {
      precision = args[2].int64_value();
    }
    if (args.back().type_kind() == ::googlesql::TYPE_STRING &&
        args.size() >= 3) {
      timezone = args.back().string_value();
    }
  }
  int64_t timestamp_micros = 0;
  if (timezone.has_value()) {
    if (auto s = ::googlesql::functions::ParseStringToTimestamp(
            args[0].string_value(),
            args[1].string_value(),
            *timezone,
            /*parse_version2=*/true,
            &timestamp_micros);
        !s.ok()) {
      return s;
    }
  } else {
    if (auto s = ::googlesql::functions::ParseStringToTimestamp(
            args[0].string_value(),
            args[1].string_value(),
            DefaultTimeZone(),
            /*parse_version2=*/true,
            &timestamp_micros);
        !s.ok()) {
      return s;
    }
  }
  (void)precision;
  return Value::TimestampFromUnixMicros(timestamp_micros);
}

absl::StatusOr<Value> ParseTime(const std::vector<Value>& args) {
  if (args.size() != 2) {
    return absl::InvalidArgumentError(
        "semantic: PARSE_TIME expects exactly two arguments");
  }
  if (HasNull(args)) return Value::NullTime();
  TimeValue time;
  if (auto s = ::googlesql::functions::ParseStringToTime(
          args[0].string_value(), args[1].string_value(), kMicros, &time);
      !s.ok()) {
    return s;
  }
  return Value::Time(time);
}

absl::StatusOr<Value> FormatDate(const std::vector<Value>& args) {
  if (args.size() != 2) {
    return absl::InvalidArgumentError(
        "semantic: FORMAT_DATE expects exactly two arguments");
  }
  if (HasNull(args)) return Value::NullString();
  std::string out;
  if (auto s = ::googlesql::functions::FormatDateToString(
          args[0].string_value(), args[1].date_value(), kFormatOpts, &out);
      !s.ok()) {
    return s;
  }
  return Value::String(out);
}

absl::StatusOr<Value> FormatDatetime(const std::vector<Value>& args) {
  if (args.size() != 2) {
    return absl::InvalidArgumentError(
        "semantic: FORMAT_DATETIME expects exactly two arguments");
  }
  if (HasNull(args)) return Value::NullString();
  std::string out;
  if (auto s = ::googlesql::functions::FormatDatetimeToStringWithOptions(
          args[0].string_value(), args[1].datetime_value(), kFormatOpts, &out);
      !s.ok()) {
    return s;
  }
  return Value::String(out);
}

absl::StatusOr<Value> FormatTimestamp(const std::vector<Value>& args) {
  if (args.size() < 2 || args.size() > 3) {
    return absl::InvalidArgumentError(
        "semantic: FORMAT_TIMESTAMP expects 2 or 3 arguments");
  }
  if (HasNull(args)) return Value::NullString();
  std::string out;
  absl::TimeZone tz = DefaultTimeZone();
  if (args.size() == 3) {
    if (auto s =
            ::googlesql::functions::MakeTimeZone(args[2].string_value(), &tz);
        !s.ok()) {
      return s;
    }
  }
  if (auto s = ::googlesql::functions::FormatTimestampToString(
          args[0].string_value(),
          args[1].ToUnixMicros(),
          tz,
          kFormatOpts,
          &out);
      !s.ok()) {
    return s;
  }
  return Value::String(out);
}

absl::StatusOr<Value> FormatTime(const std::vector<Value>& args) {
  if (args.size() != 2) {
    return absl::InvalidArgumentError(
        "semantic: FORMAT_TIME expects exactly two arguments");
  }
  if (HasNull(args)) return Value::NullString();
  std::string out;
  if (auto s = ::googlesql::functions::FormatTimeToString(
          args[0].string_value(), args[1].time_value(), &out);
      !s.ok()) {
    return s;
  }
  return Value::String(out);
}

}  // namespace functions
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
