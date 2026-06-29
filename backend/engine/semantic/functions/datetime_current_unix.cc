#include <cstdint>

#include "absl/status/status.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "backend/engine/semantic/error.h"
#include "backend/engine/semantic/functions/datetime_funcs_internal.h"
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

using datetime_internal::CurrentDatetimeValue;
using datetime_internal::CurrentDateValue;
using datetime_internal::CurrentTimeValue;
using datetime_internal::CurrentUnixMicros;
using datetime_internal::DateTimestampPart;
using datetime_internal::DefaultTimeZone;
using datetime_internal::HasNull;
using datetime_internal::kMicros;
using datetime_internal::LocalTimeZone;
using datetime_internal::PartFromArg;
using ::googlesql::Value;

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

}  // namespace functions
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
