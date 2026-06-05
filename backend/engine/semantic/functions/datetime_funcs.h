#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_FUNCTIONS_DATETIME_FUNCS_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_FUNCTIONS_DATETIME_FUNCS_H_

#include <vector>

#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"
#include "backend/engine/semantic/value.h"
#include "googlesql/public/type.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace functions {

absl::StatusOr<Value> ParseDate(const std::vector<Value>& args);
absl::StatusOr<Value> ParseDatetime(const std::vector<Value>& args);
absl::StatusOr<Value> ParseTimestamp(const std::vector<Value>& args);
absl::StatusOr<Value> ParseTime(const std::vector<Value>& args);

absl::StatusOr<Value> FormatDate(const std::vector<Value>& args);
absl::StatusOr<Value> FormatDatetime(const std::vector<Value>& args);
absl::StatusOr<Value> FormatTimestamp(const std::vector<Value>& args);
absl::StatusOr<Value> FormatTime(const std::vector<Value>& args);

absl::StatusOr<Value> DateAddSubDiffTrunc(absl::string_view name,
                                          const std::vector<Value>& args);
absl::StatusOr<Value> DatetimeAddSubDiffTrunc(absl::string_view name,
                                              const std::vector<Value>& args);
absl::StatusOr<Value> TimestampAddSubDiffTrunc(absl::string_view name,
                                               const std::vector<Value>& args);
absl::StatusOr<Value> TimeAddSubDiffTrunc(absl::string_view name,
                                          const std::vector<Value>& args);

absl::StatusOr<Value> CurrentDate(const std::vector<Value>& args);
absl::StatusOr<Value> CurrentDatetime(const std::vector<Value>& args);
absl::StatusOr<Value> CurrentTime(const std::vector<Value>& args);
absl::StatusOr<Value> CurrentTimestamp(const std::vector<Value>& args);

absl::StatusOr<Value> UnixSeconds(const std::vector<Value>& args);
absl::StatusOr<Value> UnixMillis(const std::vector<Value>& args);
absl::StatusOr<Value> UnixMicros(const std::vector<Value>& args);
absl::StatusOr<Value> UnixDate(const std::vector<Value>& args);

absl::StatusOr<Value> TimestampSeconds(const std::vector<Value>& args);
absl::StatusOr<Value> TimestampMillis(const std::vector<Value>& args);
absl::StatusOr<Value> TimestampMicros(const std::vector<Value>& args);

absl::StatusOr<Value> DateFromUnixDate(const std::vector<Value>& args);
absl::StatusOr<Value> LastDay(const std::vector<Value>& args);
absl::StatusOr<Value> MakeInterval(const std::vector<Value>& args);
absl::StatusOr<Value> JustifyInterval(const std::vector<Value>& args);
absl::StatusOr<Value> Extract(const std::vector<Value>& args,
                              const ::googlesql::Type* return_type);

absl::StatusOr<Value> GenerateDateArray(const std::vector<Value>& args,
                                        const ::googlesql::Type* return_type);
absl::StatusOr<Value> GenerateTimestampArray(
    const std::vector<Value>& args, const ::googlesql::Type* return_type);

absl::StatusOr<Value> TimeConstructor(const std::vector<Value>& args);
absl::StatusOr<Value> DatetimeConstructor(absl::string_view name,
                                          const std::vector<Value>& args);
absl::StatusOr<Value> TimestampConstructor(absl::string_view name,
                                           const std::vector<Value>& args);
absl::StatusOr<Value> StringFunc(const std::vector<Value>& args);

}  // namespace functions
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_FUNCTIONS_DATETIME_FUNCS_H_
