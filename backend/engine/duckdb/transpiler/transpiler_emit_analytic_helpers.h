#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_DUCKDB_TRANSPILER_TRANSPILER_EMIT_ANALYTIC_HELPERS_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_DUCKDB_TRANSPILER_TRANSPILER_EMIT_ANALYTIC_HELPERS_H_

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

using AnalyticEmitExprFn =
    std::function<std::string(const ::googlesql::ResolvedExpr*)>;

std::optional<std::string> FindPercentileDiscSortKey(
    const ::googlesql::ResolvedAnalyticScan* node,
    const AnalyticEmitExprFn& emit_expr);

std::string WrapInputWithPctCoalesce(absl::string_view input,
                                     absl::string_view sort_key);

std::string TryEmitPercentileDiscOnlyScan(
    const ::googlesql::ResolvedAnalyticScan* node,
    absl::string_view input,
    const AnalyticEmitExprFn& emit_expr);

}  // namespace transpiler
}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_DUCKDB_TRANSPILER_TRANSPILER_EMIT_ANALYTIC_HELPERS_H_
