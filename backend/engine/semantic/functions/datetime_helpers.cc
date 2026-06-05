#include "backend/engine/semantic/functions/datetime_funcs_internal.h"

#include <cstdint>
#include <optional>
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
#include "googlesql/public/type.h"
#include "googlesql/public/type.pb.h"
#include "googlesql/public/value.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace functions {
namespace datetime_internal {

using ::googlesql::Value;

const TimestampScale kMicros = TimestampScale::kMicroseconds;
const FormatDateTimestampOptions kFormatOpts{.expand_Q = true,
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

}  // namespace datetime_internal
}  // namespace functions
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
