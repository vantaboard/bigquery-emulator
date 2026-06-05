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

using datetime_internal::DateTimestampPart;
using datetime_internal::DatetimeValue;
using datetime_internal::DefaultTimeZone;
using datetime_internal::kMicros;
using datetime_internal::PartFromArg;
using datetime_internal::TimeValue;
using ::googlesql::Value;

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

}  // namespace functions
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
