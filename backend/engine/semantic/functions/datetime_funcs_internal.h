#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_FUNCTIONS_DATETIME_FUNCS_INTERNAL_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_FUNCTIONS_DATETIME_FUNCS_INTERNAL_H_

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "absl/status/statusor.h"
#include "absl/time/time.h"
#include "backend/engine/semantic/value.h"
#include "googlesql/public/functions/date_time_util.h"
#include "googlesql/public/functions/datetime.pb.h"
#include "googlesql/public/type.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace functions {
namespace datetime_internal {

using ::googlesql::DatetimeValue;
using ::googlesql::TimeValue;
using ::googlesql::functions::DateTimestampPart;
using ::googlesql::functions::FormatDateTimestampOptions;
using ::googlesql::functions::TimestampScale;

extern const TimestampScale kMicros;
extern const FormatDateTimestampOptions kFormatOpts;

absl::TimeZone DefaultTimeZone();
absl::TimeZone LocalTimeZone();
std::optional<Value> NullIfAny(const std::vector<Value>& args,
                               const ::googlesql::Type* return_type);
bool HasNull(const std::vector<Value>& args);
absl::StatusOr<DateTimestampPart> PartFromArg(const Value& v);
absl::StatusOr<int64_t> CurrentUnixMicros();
absl::StatusOr<int32_t> CurrentDateValue();
absl::StatusOr<DatetimeValue> CurrentDatetimeValue();
absl::StatusOr<TimeValue> CurrentTimeValue();
absl::StatusOr<Value> BuildDateArray(const std::vector<int64_t>& raw,
                                     const ::googlesql::Type* return_type);
absl::StatusOr<Value> BuildTimestampArray(const std::vector<int64_t>& micros,
                                          const ::googlesql::Type* return_type);

}  // namespace datetime_internal
}  // namespace functions
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_FUNCTIONS_DATETIME_FUNCS_INTERNAL_H_
