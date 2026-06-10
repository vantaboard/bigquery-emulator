#include "backend/engine/semantic/functions/datetime_funcs.h"

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

using datetime_internal::BuildDateArray;
using datetime_internal::BuildTimestampArray;
using datetime_internal::CurrentDatetimeValue;
using datetime_internal::CurrentDateValue;
using datetime_internal::CurrentTimeValue;
using datetime_internal::CurrentUnixMicros;
using datetime_internal::DateTimestampPart;
using datetime_internal::DatetimeValue;
using datetime_internal::DefaultTimeZone;
using datetime_internal::HasNull;
using datetime_internal::kFormatOpts;
using datetime_internal::kMicros;
using datetime_internal::LocalTimeZone;
using datetime_internal::NullIfAny;
using datetime_internal::PartFromArg;
using datetime_internal::TimeValue;
using ::googlesql::IntervalValue;
using ::googlesql::Value;
using ::googlesql::functions::DateIncrement;
using ::googlesql::functions::TimestampIncrement;

absl::StatusOr<Value> MakeInterval(const std::vector<Value>& args) {
  if (args.size() != 6) {
    return absl::InvalidArgumentError(
        "semantic: MAKE_INTERVAL expects six arguments");
  }
  if (HasNull(args)) return Value::NullInterval();
  auto iv = IntervalValue::FromYMDHMS(args[0].int64_value(),
                                      args[1].int64_value(),
                                      args[2].int64_value(),
                                      args[3].int64_value(),
                                      args[4].int64_value(),
                                      args[5].int64_value());
  if (!iv.ok()) return iv.status();
  return Value::Interval(*iv);
}

absl::StatusOr<Value> JustifyInterval(const std::vector<Value>& args) {
  if (args.size() != 1) {
    return absl::InvalidArgumentError(
        "semantic: JUSTIFY_INTERVAL expects one argument");
  }
  if (args[0].is_null()) return Value::NullInterval();
  auto iv = ::googlesql::JustifyInterval(args[0].interval_value());
  if (!iv.ok()) return iv.status();
  return Value::Interval(*iv);
}

absl::StatusOr<Value> Extract(const std::vector<Value>& args,
                              const ::googlesql::Type* return_type) {
  (void)return_type;
  if (args.size() < 2 || args.size() > 3) {
    return absl::InvalidArgumentError(
        "semantic: EXTRACT expects 2 or 3 arguments");
  }
  if (args[0].is_null() || args[1].is_null() ||
      (args.size() == 3 && args[2].is_null())) {
    return Value::NullInt64();
  }
  auto part = PartFromArg(args[1]);
  if (!part.ok()) return part.status();
  const Value& v = args[0];
  if (v.type_kind() == ::googlesql::TYPE_INTERVAL) {
    auto r = v.interval_value().Extract(*part);
    if (!r.ok()) return r.status();
    return Value::Int64(*r);
  }
  int32_t value32 = 0;
  int64_t value64 = 0;
  if (v.type_kind() == ::googlesql::TYPE_DATE) {
    if (auto s = ::googlesql::functions::ExtractFromDate(
            *part, v.date_value(), &value32);
        !s.ok()) {
      return s;
    }
    return Value::Int64(value32);
  }
  if (v.type_kind() == ::googlesql::TYPE_DATETIME) {
    if (auto s = ::googlesql::functions::ExtractFromDatetime(
            *part, v.datetime_value(), &value32);
        !s.ok()) {
      return s;
    }
    return Value::Int64(value32);
  }
  if (v.type_kind() == ::googlesql::TYPE_TIME) {
    if (auto s = ::googlesql::functions::ExtractFromTime(
            *part, v.time_value(), &value64);
        !s.ok()) {
      return s;
    }
    return Value::Int64(value64);
  }
  if (v.type_kind() == ::googlesql::TYPE_TIMESTAMP) {
    absl::TimeZone tz = DefaultTimeZone();
    if (args.size() == 3) {
      if (auto s =
              ::googlesql::functions::MakeTimeZone(args[2].string_value(), &tz);
          !s.ok()) {
        return s;
      }
    }
    value64 = 0;
    if (auto s = ::googlesql::functions::ExtractFromTimestamp(
            *part, v.ToTime(), tz, &value64);
        !s.ok()) {
      return s;
    }
    return Value::Int64(value64);
  }
  return absl::InvalidArgumentError(absl::StrCat(
      "semantic: EXTRACT unsupported type ", v.type()->DebugString()));
}

absl::StatusOr<Value> GenerateDateArray(const std::vector<Value>& args,
                                        const ::googlesql::Type* return_type) {
  if (args.size() < 2 || args.size() > 4) {
    return absl::InvalidArgumentError(
        "semantic: GENERATE_DATE_ARRAY expects 2 to 4 arguments");
  }
  if (args[0].is_null() || args[1].is_null()) {
    if (auto n = NullIfAny(args, return_type)) return *n;
  }
  int64_t step = 1;
  DateTimestampPart step_unit = DateTimestampPart::DAY;
  const bool has_step = args.size() >= 3 && !args[2].is_null();
  if (has_step) {
    step = args[2].int64_value();
    if (args.size() == 4) {
      auto p = PartFromArg(args[3]);
      if (!p.ok()) return p.status();
      step_unit = *p;
    }
  }
  std::vector<int64_t> raw;
  DateIncrement inc{.unit = step_unit, .value = step};
  if (auto s = ::googlesql::functions::GenerateArray<int64_t, DateIncrement>(
          args[0].date_value(), args[1].date_value(), inc, &raw);
      !s.ok()) {
    return s;
  }
  return BuildDateArray(raw, return_type);
}

absl::StatusOr<Value> GenerateTimestampArray(
    const std::vector<Value>& args, const ::googlesql::Type* return_type) {
  if (args.size() != 4) {
    return absl::InvalidArgumentError(
        "semantic: GENERATE_TIMESTAMP_ARRAY expects four arguments");
  }
  if (HasNull(args)) {
    if (auto n = NullIfAny(args, return_type)) return *n;
  }
  const int64_t step = args[2].int64_value();
  auto step_unit = PartFromArg(args[3]);
  if (!step_unit.ok()) return step_unit.status();
  const TimestampIncrement inc{.unit = *step_unit, .value = step};
  std::vector<absl::Time> times;
  if (auto s =
          ::googlesql::functions::GenerateArray<absl::Time, TimestampIncrement>(
              args[0].ToTime(), args[1].ToTime(), inc, &times);
      !s.ok()) {
    return s;
  }
  std::vector<int64_t> raw;
  raw.reserve(times.size());
  for (const absl::Time& t : times) {
    raw.push_back(absl::ToUnixMicros(t));
  }
  return BuildTimestampArray(raw, return_type);
}

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
    absl::Time t;
    if (args.size() == 2 && args[1].type_kind() == ::googlesql::TYPE_STRING) {
      if (auto s = ::googlesql::functions::ConvertDatetimeToTimestamp(
              args[0].datetime_value(), args[1].string_value(), &t);
          !s.ok()) {
        return s;
      }
    } else if (args.size() == 1) {
      if (auto s = ::googlesql::functions::ConvertDatetimeToTimestamp(
              args[0].datetime_value(), DefaultTimeZone(), &t);
          !s.ok()) {
        return s;
      }
    } else {
      return absl::InvalidArgumentError(
          "semantic: TIMESTAMP(DATETIME) expects one or two arguments");
    }
    return Value::Timestamp(t);
  }
  if (args.size() == 1 && args[0].type_kind() == ::googlesql::TYPE_DATE) {
    int64_t micros = 0;
    if (auto s = ::googlesql::functions::ConvertDateToTimestamp(
            args[0].date_value(), kMicros, DefaultTimeZone(), &micros);
        !s.ok()) {
      return s;
    }
    return Value::TimestampFromUnixMicros(micros);
  }
  if (args.size() == 2 && args[0].type_kind() == ::googlesql::TYPE_DATETIME &&
      args[1].type_kind() == ::googlesql::TYPE_STRING) {
    absl::Time t;
    if (auto s = ::googlesql::functions::ConvertDatetimeToTimestamp(
            args[0].datetime_value(), args[1].string_value(), &t);
        !s.ok()) {
      return s;
    }
    return Value::Timestamp(t);
  }
  if (args.size() == 2 && args[0].type_kind() == ::googlesql::TYPE_STRING &&
      args[1].type_kind() == ::googlesql::TYPE_STRING) {
    absl::TimeZone tz;
    if (auto s =
            ::googlesql::functions::MakeTimeZone(args[1].string_value(), &tz);
        !s.ok()) {
      return s;
    }
    int64_t micros = 0;
    if (auto s = ::googlesql::functions::ConvertStringToTimestamp(
            args[0].string_value(),
            tz,
            kMicros,
            /*allow_tz_in_str=*/false,
            &micros);
        !s.ok()) {
      return s;
    }
    return Value::TimestampFromUnixMicros(micros);
  }
  if (args.size() == 1 && args[0].type_kind() == ::googlesql::TYPE_STRING) {
    int64_t micros = 0;
    if (auto s = ::googlesql::functions::ConvertStringToTimestamp(
            args[0].string_value(),
            DefaultTimeZone(),
            kMicros,
            /*allow_tz_in_str=*/true,
            &micros);
        !s.ok()) {
      return s;
    }
    return Value::TimestampFromUnixMicros(micros);
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
