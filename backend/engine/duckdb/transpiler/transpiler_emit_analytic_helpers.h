#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_DUCKDB_TRANSPILER_TRANSPILER_EMIT_ANALYTIC_HELPERS_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_DUCKDB_TRANSPILER_TRANSPILER_EMIT_ANALYTIC_HELPERS_H_

#include <functional>
#include <optional>
#include <string>
#include <vector>

#include "absl/strings/string_view.h"
#include "absl/types/span.h"
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

// When the analytic input still exposes `__bq_j_<id>` aliases, wrap the
// analytic SELECT so join columns are renamed to user names while analytic
// AS aliases stay by name. Rewrites captured ORDER BY keys to those names.
std::string MaybeNormalizeAnalyticJoinAliases(
    const ::googlesql::ResolvedAnalyticScan* node,
    std::string sql,
    bool input_used_join_aliases,
    bool input_has_rn_column,
    std::vector<std::string>* order_items,
    absl::Span<const int> order_column_ids);

}  // namespace transpiler
}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_DUCKDB_TRANSPILER_TRANSPILER_EMIT_ANALYTIC_HELPERS_H_
