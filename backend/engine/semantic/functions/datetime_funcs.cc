#include "backend/engine/semantic/functions/datetime_funcs.h"

#include <cstdint>
#include <string>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
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
using datetime_internal::DateTimestampPart;
using datetime_internal::DefaultTimeZone;
using datetime_internal::HasNull;
using datetime_internal::kMicros;
using datetime_internal::NullIfAny;
using datetime_internal::PartFromArg;
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
    if (args[2].type_kind() == ::googlesql::TYPE_INTERVAL) {
      const IntervalValue& interval = args[2].interval_value();
      static constexpr DateTimestampPart kParts[] = {
          DateTimestampPart::DAY,
          DateTimestampPart::WEEK,
          DateTimestampPart::MONTH,
          DateTimestampPart::QUARTER,
          DateTimestampPart::YEAR,
      };
      bool found = false;
      for (DateTimestampPart part : kParts) {
        auto extracted = interval.Extract(part);
        if (!extracted.ok() || *extracted == 0) continue;
        step_unit = part;
        step = *extracted;
        found = true;
        break;
      }
      if (!found) {
        return absl::InvalidArgumentError(
            "semantic: GENERATE_DATE_ARRAY step INTERVAL has no "
            "supported date part");
      }
    } else {
      step = args[2].int64_value();
      if (args.size() == 4) {
        auto p = PartFromArg(args[3]);
        if (!p.ok()) return p.status();
        step_unit = *p;
      }
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

}  // namespace functions
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
