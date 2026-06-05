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

namespace {

using ::googlesql::DatetimeValue;
using ::googlesql::IntervalValue;
using ::googlesql::TimeValue;
using ::googlesql::Value;
using ::googlesql::functions::DateIncrement;
using ::googlesql::functions::DateTimestampPart;
using ::googlesql::functions::FormatDateTimestampOptions;
using ::googlesql::functions::TimestampIncrement;
using ::googlesql::functions::TimestampScale;

constexpr TimestampScale kMicros = TimestampScale::kMicroseconds;
constexpr FormatDateTimestampOptions kFormatOpts{.expand_Q = true,
                                                 .expand_J = true};

absl::TimeZone DefaultTimeZone() {
  return absl::UTCTimeZone();
}

absl::TimeZone LocalTimeZone() {
  return absl::LocalTimeZone();
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

bool HasNull(const std::vector<Value>& args) {
  for (const auto& v : args) {
    if (v.is_null()) return true;
  }
  return false;
}

absl::StatusOr<DateTimestampPart> PartFromArg(const Value& v) {
  if (v.type_kind() == ::googlesql::TYPE_INT64) {
    const int part_int = static_cast<int>(v.int64_value());
    if (::googlesql::functions::DateTimestampPart_IsValid(part_int)) {
      return static_cast<DateTimestampPart>(part_int);
    }
    return absl::InvalidArgumentError(absl::StrCat(
        "semantic: invalid DateTimestampPart enum value ", v.int64_value()));
  }
  if (v.type_kind() == ::googlesql::TYPE_STRING) {
    DateTimestampPart part = DateTimestampPart::YEAR;
    if (!::googlesql::functions::DateTimestampPart_Parse(v.string_value(),
                                                         &part)) {
      return absl::InvalidArgumentError(absl::StrCat(
          "semantic: unknown interval part '", v.string_value(), "'"));
    }
    return part;
  }
  if (v.type_kind() == ::googlesql::TYPE_ENUM) {
    const int part_int = v.enum_value();
    if (!::googlesql::functions::DateTimestampPart_IsValid(part_int)) {
      return absl::InvalidArgumentError(absl::StrCat(
          "semantic: invalid DateTimestampPart enum value ", part_int));
    }
    return static_cast<DateTimestampPart>(part_int);
  }
  return absl::InvalidArgumentError(absl::StrCat(
      "semantic: interval part must be INT64 enum or STRING name; got ",
      v.type()->DebugString()));
}

absl::StatusOr<int64_t> CurrentUnixMicros() {
  return absl::ToUnixMicros(absl::Now());
}

absl::StatusOr<int32_t> CurrentDateValue() {
  int32_t date = 0;
  const int64_t now_micros = absl::ToUnixMicros(absl::Now());
  if (!::googlesql::functions::ExtractFromTimestamp(DateTimestampPart::DATE,
                                                    now_micros,
                                                    kMicros,
                                                    DefaultTimeZone(),
                                                    &date)
           .ok()) {
    return absl::InternalError("semantic: failed to extract current DATE");
  }
  return date;
}

absl::StatusOr<DatetimeValue> CurrentDatetimeValue() {
  DatetimeValue dt;
  if (!::googlesql::functions::ConvertTimestampToDatetime(
           absl::Now(), LocalTimeZone(), &dt)
           .ok()) {
    return absl::InternalError("semantic: failed to build current DATETIME");
  }
  return dt;
}

absl::StatusOr<TimeValue> CurrentTimeValue() {
  TimeValue t;
  if (!::googlesql::functions::ConvertTimestampToTime(
           absl::Now(), LocalTimeZone(), &t)
           .ok()) {
    return absl::InternalError("semantic: failed to build current TIME");
  }
  return t;
}

absl::StatusOr<Value> BuildDateArray(const std::vector<int64_t>& raw,
                                     const ::googlesql::Type* return_type) {
  std::vector<Value> elems;
  elems.reserve(raw.size());
  for (int64_t d : raw) {
    elems.push_back(Value::Date(static_cast<int32_t>(d)));
  }
  if (return_type == nullptr || !return_type->IsArray()) {
    return absl::InvalidArgumentError(
        "semantic: GENERATE_DATE_ARRAY requires ARRAY return type");
  }
  return Value::Array(return_type->AsArray(), elems);
}

absl::StatusOr<Value> BuildTimestampArray(
    const std::vector<int64_t>& micros, const ::googlesql::Type* return_type) {
  std::vector<Value> elems;
  elems.reserve(micros.size());
  for (int64_t t : micros) {
    elems.push_back(Value::TimestampFromUnixMicros(t));
  }
  if (return_type == nullptr || !return_type->IsArray()) {
    return absl::InvalidArgumentError(
        "semantic: GENERATE_TIMESTAMP_ARRAY requires ARRAY return type");
  }
  return Value::Array(return_type->AsArray(), elems);
}

}  // namespace

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

absl::StatusOr<Value> DateAddSubDiffTrunc(absl::string_view name,
                                          const std::vector<Value>& args) {
  if (args.size() < 2) {
    return absl::InvalidArgumentError(
        absl::StrCat("semantic: ", name, " expects at least two arguments"));
  }
  if (args[0].is_null() || args[1].is_null()) {
    return Value::Null(args[0].type());
  }
  if (name == "date_diff") {
    if (args.size() != 3) {
      return absl::InvalidArgumentError(
          "semantic: DATE_DIFF expects three arguments");
    }
    if (args[2].is_null()) return Value::NullInt64();
    auto part = PartFromArg(args[2]);
    if (!part.ok()) return part.status();
    int32_t out = 0;
    if (auto s = ::googlesql::functions::DiffDates(
            args[0].date_value(), args[1].date_value(), *part, &out);
        !s.ok()) {
      return s;
    }
    return Value::Int64(out);
  }
  if (name == "date_trunc") {
    if (args.size() < 2 || args.size() > 3) {
      return absl::InvalidArgumentError(
          "semantic: DATE_TRUNC expects 2 or 3 arguments");
    }
    if ((args.size() == 3 && args[2].is_null()) || args[1].is_null()) {
      return Value::NullDate();
    }
    auto part = PartFromArg(args[1]);
    if (!part.ok()) return part.status();
    int32_t out = 0;
    if (auto s = ::googlesql::functions::TruncateDate(
            args[0].date_value(), *part, &out);
        !s.ok()) {
      return s;
    }
    return Value::Date(out);
  }
  if (args.size() != 3) {
    return absl::InvalidArgumentError(
        absl::StrCat("semantic: ", name, " expects three arguments"));
  }
  if (args[2].is_null()) return Value::NullDate();
  auto part = PartFromArg(args[2]);
  if (!part.ok()) return part.status();
  const int64_t interval = args[1].int64_value();
  int32_t out = 0;
  if (name == "date_add") {
    if (auto s = ::googlesql::functions::AddDate(
            args[0].date_value(), *part, interval, &out);
        !s.ok()) {
      return s;
    }
  } else if (name == "date_sub") {
    if (auto s = ::googlesql::functions::SubDate(
            args[0].date_value(), *part, interval, &out);
        !s.ok()) {
      return s;
    }
  } else {
    return absl::InvalidArgumentError(
        absl::StrCat("semantic: unknown date function ", name));
  }
  return Value::Date(out);
}

absl::StatusOr<Value> DatetimeAddSubDiffTrunc(absl::string_view name,
                                              const std::vector<Value>& args) {
  if (args.size() < 2) {
    return absl::InvalidArgumentError(
        absl::StrCat("semantic: ", name, " expects at least two arguments"));
  }
  if (args[0].is_null() || args[1].is_null()) {
    return Value::Null(args[0].type());
  }
  if (name == "datetime_diff") {
    if (args.size() != 3) {
      return absl::InvalidArgumentError(
          "semantic: DATETIME_DIFF expects three arguments");
    }
    if (args[2].is_null()) return Value::NullInt64();
    auto part = PartFromArg(args[2]);
    if (!part.ok()) return part.status();
    int64_t out = 0;
    if (auto s = ::googlesql::functions::DiffDatetimes(
            args[0].datetime_value(), args[1].datetime_value(), *part, &out);
        !s.ok()) {
      return s;
    }
    return Value::Int64(out);
  }
  if (name == "datetime_trunc") {
    if (args.size() != 2) {
      return absl::InvalidArgumentError(
          "semantic: DATETIME_TRUNC expects two arguments");
    }
    if (args[1].is_null()) return Value::NullDatetime();
    auto part = PartFromArg(args[1]);
    if (!part.ok()) return part.status();
    DatetimeValue out;
    if (auto s = ::googlesql::functions::TruncateDatetime(
            args[0].datetime_value(), *part, &out);
        !s.ok()) {
      return s;
    }
    return Value::Datetime(out);
  }
  if (args.size() != 3) {
    return absl::InvalidArgumentError(
        absl::StrCat("semantic: ", name, " expects three arguments"));
  }
  if (args[2].is_null()) return Value::NullDatetime();
  auto part = PartFromArg(args[2]);
  if (!part.ok()) return part.status();
  const int64_t interval = args[1].int64_value();
  DatetimeValue out;
  if (name == "datetime_add") {
    if (auto s = ::googlesql::functions::AddDatetime(
            args[0].datetime_value(), *part, interval, &out);
        !s.ok()) {
      return s;
    }
  } else if (name == "datetime_sub") {
    if (auto s = ::googlesql::functions::SubDatetime(
            args[0].datetime_value(), *part, interval, &out);
        !s.ok()) {
      return s;
    }
  } else {
    return absl::InvalidArgumentError(
        absl::StrCat("semantic: unknown datetime function ", name));
  }
  return Value::Datetime(out);
}

absl::StatusOr<Value> TimestampAddSubDiffTrunc(absl::string_view name,
                                               const std::vector<Value>& args) {
  if (args.size() < 2) {
    return absl::InvalidArgumentError(
        absl::StrCat("semantic: ", name, " expects at least two arguments"));
  }
  if (args[0].is_null() || args[1].is_null()) {
    return Value::NullTimestamp();
  }
  if (name == "timestamp_diff") {
    if (args.size() != 3) {
      return absl::InvalidArgumentError(
          "semantic: TIMESTAMP_DIFF expects three arguments");
    }
    if (args[2].is_null()) return Value::NullInt64();
    auto part = PartFromArg(args[2]);
    if (!part.ok()) return part.status();
    int64_t out = 0;
    const Value& lhs = args[0];
    const Value& rhs = args[1];
    auto to_micros = [](const Value& v) -> absl::StatusOr<int64_t> {
      if (v.type_kind() == ::googlesql::TYPE_TIMESTAMP) {
        return v.ToUnixMicros();
      }
      absl::Time t;
      if (auto s = ::googlesql::functions::ConvertDatetimeToTimestamp(
              v.datetime_value(), DefaultTimeZone(), &t);
          !s.ok()) {
        return s;
      }
      return absl::ToUnixMicros(t);
    };
    if (lhs.type_kind() == ::googlesql::TYPE_DATETIME &&
        rhs.type_kind() == ::googlesql::TYPE_DATETIME) {
      if (auto s = ::googlesql::functions::DiffDatetimes(
              lhs.datetime_value(), rhs.datetime_value(), *part, &out);
          !s.ok()) {
        return s;
      }
      return Value::Int64(out);
    }
    auto t0 = to_micros(lhs);
    auto t1 = to_micros(rhs);
    if (!t0.ok()) return t0.status();
    if (!t1.ok()) return t1.status();
    if (auto s = ::googlesql::functions::TimestampDiff(
            *t0, *t1, kMicros, *part, &out);
        !s.ok()) {
      return s;
    }
    return Value::Int64(out);
  }
  if (name == "timestamp_trunc") {
    if (args.size() < 2 || args.size() > 3) {
      return absl::InvalidArgumentError(
          "semantic: TIMESTAMP_TRUNC expects 2 or 3 arguments");
    }
    if (args[1].is_null() || (args.size() == 3 && args[2].is_null())) {
      return Value::NullTimestamp();
    }
    auto part = PartFromArg(args[1]);
    if (!part.ok()) return part.status();
    int64_t out = 0;
    if (args.size() == 2) {
      if (auto s = ::googlesql::functions::TimestampTrunc(
              args[0].ToUnixMicros(), DefaultTimeZone(), *part, &out);
          !s.ok()) {
        return s;
      }
    } else {
      if (auto s = ::googlesql::functions::TimestampTrunc(
              args[0].ToUnixMicros(), args[2].string_value(), *part, &out);
          !s.ok()) {
        return s;
      }
    }
    return Value::TimestampFromUnixMicros(out);
  }
  if (args.size() != 3) {
    return absl::InvalidArgumentError(
        absl::StrCat("semantic: ", name, " expects three arguments"));
  }
  if (args[2].is_null()) return Value::NullTimestamp();
  auto part = PartFromArg(args[2]);
  if (!part.ok()) return part.status();
  const int64_t interval = args[1].int64_value();
  int64_t out = 0;
  if (name == "timestamp_add") {
    if (auto s = ::googlesql::functions::AddTimestamp(args[0].ToUnixMicros(),
                                                      kMicros,
                                                      DefaultTimeZone(),
                                                      *part,
                                                      interval,
                                                      &out);
        !s.ok()) {
      return s;
    }
  } else if (name == "timestamp_sub") {
    if (auto s = ::googlesql::functions::SubTimestamp(args[0].ToUnixMicros(),
                                                      kMicros,
                                                      DefaultTimeZone(),
                                                      *part,
                                                      interval,
                                                      &out);
        !s.ok()) {
      return s;
    }
  } else {
    return absl::InvalidArgumentError(
        absl::StrCat("semantic: unknown timestamp function ", name));
  }
  return Value::TimestampFromUnixMicros(out);
}

absl::StatusOr<Value> TimeAddSubDiffTrunc(absl::string_view name,
                                          const std::vector<Value>& args) {
  if (args.size() < 2) {
    return absl::InvalidArgumentError(
        absl::StrCat("semantic: ", name, " expects at least two arguments"));
  }
  if (args[0].is_null() || args[1].is_null()) {
    return Value::NullTime();
  }
  if (name == "time_diff") {
    if (args.size() != 3) {
      return absl::InvalidArgumentError(
          "semantic: TIME_DIFF expects three arguments");
    }
    if (args[2].is_null()) return Value::NullInt64();
    auto part = PartFromArg(args[2]);
    if (!part.ok()) return part.status();
    int64_t out = 0;
    if (auto s = ::googlesql::functions::DiffTimes(
            args[0].time_value(), args[1].time_value(), *part, &out);
        !s.ok()) {
      return s;
    }
    return Value::Int64(out);
  }
  if (name == "time_trunc") {
    if (args.size() != 2) {
      return absl::InvalidArgumentError(
          "semantic: TIME_TRUNC expects two arguments");
    }
    if (args[1].is_null()) return Value::NullTime();
    auto part = PartFromArg(args[1]);
    if (!part.ok()) return part.status();
    TimeValue out;
    if (auto s = ::googlesql::functions::TruncateTime(
            args[0].time_value(), *part, &out);
        !s.ok()) {
      return s;
    }
    return Value::Time(out);
  }
  if (args.size() != 3) {
    return absl::InvalidArgumentError(
        absl::StrCat("semantic: ", name, " expects three arguments"));
  }
  if (args[2].is_null()) return Value::NullTime();
  auto part = PartFromArg(args[2]);
  if (!part.ok()) return part.status();
  const int64_t interval = args[1].int64_value();
  TimeValue out;
  if (name == "time_add") {
    if (auto s = ::googlesql::functions::AddTime(
            args[0].time_value(), *part, interval, &out);
        !s.ok()) {
      return s;
    }
  } else if (name == "time_sub") {
    if (auto s = ::googlesql::functions::SubTime(
            args[0].time_value(), *part, interval, &out);
        !s.ok()) {
      return s;
    }
  } else {
    return absl::InvalidArgumentError(
        absl::StrCat("semantic: unknown time function ", name));
  }
  return Value::Time(out);
}

absl::StatusOr<Value> CurrentDate(const std::vector<Value>& args) {
  (void)args;
  auto d = CurrentDateValue();
  if (!d.ok()) return d.status();
  return Value::Date(*d);
}

absl::StatusOr<Value> CurrentDatetime(const std::vector<Value>& args) {
  (void)args;
  auto dt = CurrentDatetimeValue();
  if (!dt.ok()) return dt.status();
  return Value::Datetime(*dt);
}

absl::StatusOr<Value> CurrentTime(const std::vector<Value>& args) {
  (void)args;
  auto t = CurrentTimeValue();
  if (!t.ok()) return t.status();
  return Value::Time(*t);
}

absl::StatusOr<Value> CurrentTimestamp(const std::vector<Value>& args) {
  (void)args;
  return Value::TimestampFromUnixMicros(absl::ToUnixMicros(absl::Now()));
}

absl::StatusOr<Value> UnixSeconds(const std::vector<Value>& args) {
  if (args.size() != 1) {
    return absl::InvalidArgumentError(
        "semantic: UNIX_SECONDS expects one argument");
  }
  if (args[0].is_null()) return Value::NullInt64();
  const int64_t micros = args[0].ToUnixMicros();
  int64_t sec = micros / 1000000;
  if (micros < 0 && (micros % 1000000) != 0) {
    --sec;
  }
  return Value::Int64(sec);
}

absl::StatusOr<Value> UnixMillis(const std::vector<Value>& args) {
  if (args.size() != 1) {
    return absl::InvalidArgumentError(
        "semantic: UNIX_MILLIS expects one argument");
  }
  if (args[0].is_null()) return Value::NullInt64();
  const int64_t micros = args[0].ToUnixMicros();
  int64_t ms = micros / 1000;
  if (micros < 0 && (micros % 1000) != 0) {
    --ms;
  }
  return Value::Int64(ms);
}

absl::StatusOr<Value> UnixMicros(const std::vector<Value>& args) {
  if (args.size() != 1) {
    return absl::InvalidArgumentError(
        "semantic: UNIX_MICROS expects one argument");
  }
  if (args[0].is_null()) return Value::NullInt64();
  return Value::Int64(args[0].ToUnixMicros());
}

absl::StatusOr<Value> UnixDate(const std::vector<Value>& args) {
  if (args.size() != 1) {
    return absl::InvalidArgumentError(
        "semantic: UNIX_DATE expects one argument");
  }
  if (args[0].is_null()) return Value::NullInt64();
  return Value::Int64(args[0].date_value());
}

absl::StatusOr<Value> TimestampSeconds(const std::vector<Value>& args) {
  if (args.size() != 1) {
    return absl::InvalidArgumentError(
        "semantic: TIMESTAMP_SECONDS expects one argument");
  }
  if (args[0].is_null()) return Value::NullTimestamp();
  const int64_t v = args[0].int64_value();
  const int64_t micros = v * 1000000;
  if (!::googlesql::functions::IsValidTimestamp(micros, kMicros)) {
    return MakeSemanticError(
        SemanticErrorReason::kInvalidArgument,
        absl::StrCat("Input value ", v, " cannot be converted into TIMESTAMP"));
  }
  return Value::TimestampFromUnixMicros(micros);
}

absl::StatusOr<Value> TimestampMillis(const std::vector<Value>& args) {
  if (args.size() != 1) {
    return absl::InvalidArgumentError(
        "semantic: TIMESTAMP_MILLIS expects one argument");
  }
  if (args[0].is_null()) return Value::NullTimestamp();
  const int64_t v = args[0].int64_value();
  const int64_t micros = v * 1000;
  if (!::googlesql::functions::IsValidTimestamp(micros, kMicros)) {
    return MakeSemanticError(
        SemanticErrorReason::kInvalidArgument,
        absl::StrCat("Input value ", v, " cannot be converted into TIMESTAMP"));
  }
  return Value::TimestampFromUnixMicros(micros);
}

absl::StatusOr<Value> TimestampMicros(const std::vector<Value>& args) {
  if (args.size() != 1) {
    return absl::InvalidArgumentError(
        "semantic: TIMESTAMP_MICROS expects one argument");
  }
  if (args[0].is_null()) return Value::NullTimestamp();
  const int64_t v = args[0].int64_value();
  if (!::googlesql::functions::IsValidTimestamp(v, kMicros)) {
    return MakeSemanticError(
        SemanticErrorReason::kInvalidArgument,
        absl::StrCat("Input value ", v, " cannot be converted into TIMESTAMP"));
  }
  return Value::TimestampFromUnixMicros(v);
}

absl::StatusOr<Value> DateFromUnixDate(const std::vector<Value>& args) {
  if (args.size() != 1) {
    return absl::InvalidArgumentError(
        "semantic: DATE_FROM_UNIX_DATE expects one argument");
  }
  if (args[0].is_null()) return Value::NullDate();
  const int64_t days = args[0].int64_value();
  const int32_t date = static_cast<int32_t>(days);
  if (!::googlesql::functions::IsValidDate(date)) {
    return MakeSemanticError(
        SemanticErrorReason::kInvalidArgument,
        absl::StrCat("DATE_FROM_UNIX_DATE range is -719162 to 2932896 but saw ",
                     days));
  }
  return Value::Date(date);
}

absl::StatusOr<Value> LastDay(const std::vector<Value>& args) {
  if (args.size() < 1 || args.size() > 2) {
    return absl::InvalidArgumentError(
        "semantic: LAST_DAY expects one or two arguments");
  }
  if (args[0].is_null()) return Value::NullDate();
  DateTimestampPart part = DateTimestampPart::MONTH;
  if (args.size() == 2) {
    if (args[1].is_null()) return Value::NullDate();
    auto p = PartFromArg(args[1]);
    if (!p.ok()) return p.status();
    part = *p;
  }
  int32_t date = 0;
  if (args[0].type_kind() == ::googlesql::TYPE_DATE) {
    if (auto s = ::googlesql::functions::LastDayOfDate(
            args[0].date_value(), part, &date);
        !s.ok()) {
      return s;
    }
  } else if (args[0].type_kind() == ::googlesql::TYPE_DATETIME) {
    if (auto s = ::googlesql::functions::LastDayOfDatetime(
            args[0].datetime_value(), part, &date);
        !s.ok()) {
      return s;
    }
  } else {
    return absl::InvalidArgumentError(
        "semantic: LAST_DAY requires DATE or DATETIME");
  }
  return Value::Date(date);
}

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
    int64_t value64 = 0;
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

}  // namespace functions
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
