#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_DUCKDB_DUCKDB_EXECUTOR_TIME_TRAVEL_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_DUCKDB_DUCKDB_EXECUTOR_TIME_TRAVEL_H_

#include <cstdint>

#include "absl/status/statusor.h"
#include "googlesql/resolved_ast/resolved_ast.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace duckdb {
namespace internal {

// Evaluates a FOR SYSTEM_TIME AS OF timestamp expression to Unix epoch
// milliseconds. Supports analyzer-resolved literals and CURRENT_TIMESTAMP().
absl::StatusOr<std::int64_t> EvaluateForSystemTimeAsOfMs(
    const ::googlesql::ResolvedExpr& expr);

}  // namespace internal
}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_DUCKDB_DUCKDB_EXECUTOR_TIME_TRAVEL_H_
