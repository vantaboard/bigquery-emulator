#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_DUCKDB_TRANSPILER_TRANSPILER_EMIT_DATETIME_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_DUCKDB_TRANSPILER_TRANSPILER_EMIT_DATETIME_H_

#include <functional>
#include <optional>
#include <string>

#include "absl/strings/string_view.h"
#include "googlesql/resolved_ast/resolved_ast.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace duckdb {
namespace transpiler {
namespace internal {

using EmitExprFn =
    std::function<std::string(const ::googlesql::ResolvedExpr* expr)>;

std::optional<std::string> TryEmitDateTimeFunctionCall(
    absl::string_view name,
    const ::googlesql::ResolvedFunctionCall* node,
    const EmitExprFn& emit_expr);

// True when the transpiler can lower this TIMESTAMP_ADD /
// TIMESTAMP_SUB call to `bq_timestamp_add` / `bq_timestamp_sub`:
// the interval argument is either a fixed-width INTERVAL literal
// (no month component, no sub-microsecond remainder) or an
// `$interval(<amount>, <literal fixed-width part>)` call. The route
// classifier consults this so non-lowerable forms stay on the
// semantic executor instead of surfacing UNIMPLEMENTED.
bool CanLowerTimestampAddSub(const ::googlesql::ResolvedFunctionCall* node);

}  // namespace internal
}  // namespace transpiler
}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_DUCKDB_TRANSPILER_TRANSPILER_EMIT_DATETIME_H_
