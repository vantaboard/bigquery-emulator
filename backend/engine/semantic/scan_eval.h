#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_SCAN_EVAL_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_SCAN_EVAL_H_

// Row-at-a-time scan materialization for the semantic executor.
//
// Walks `ResolvedScan` trees (ProjectScan, WithScan, OrderByScan,
// FilterScan, JoinScan, ArrayScan, SetOperationScan, ...) and
// produces `ColumnBindings` rows the expression evaluator consumes.
// Owned by `local-exec-07-semantic-core-expr.plan.md`.

#include <string>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "absl/status/statusor.h"
#include "backend/engine/semantic/eval_expr.h"
#include "googlesql/resolved_ast/resolved_ast.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {

// Materialized rows for one CTE binding inside a `ResolvedWithScan`.
struct CteTable {
  // Output column ids in analyzer order (from the CTE subquery scan).
  std::vector<int> column_ids;
  std::vector<ColumnBindings> rows;
};

// Evaluate `scan` and return one binding map per output row.
absl::StatusOr<std::vector<ColumnBindings>> MaterializeScan(
    const ::googlesql::ResolvedScan* scan, EvalContext& ctx);

// Evaluate a `ResolvedSubqueryExpr` (SCALAR / EXISTS / IN / ARRAY).
absl::StatusOr<Value> EvalSubqueryExpr(
    const ::googlesql::ResolvedSubqueryExpr& node, const EvalContext& ctx);

// Find the outermost `ResolvedProjectScan` that carries the SELECT
// projection list for a query scan tree (peels OrderByScan / WithScan
// wrappers that only reorder or scope rows). Returns nullptr when the
// tree ends at an ArrayScan / JoinScan leaf without a projection node
// (UNNEST-only shapes bind output columns from materialized rows).
absl::StatusOr<const ::googlesql::ResolvedProjectScan*> FindOutputProjectScan(
    const ::googlesql::ResolvedScan* scan);

}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_SCAN_EVAL_H_
