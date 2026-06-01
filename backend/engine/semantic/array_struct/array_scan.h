#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_ARRAY_STRUCT_ARRAY_SCAN_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_ARRAY_STRUCT_ARRAY_SCAN_H_

// Local evaluator for the `ResolvedArrayScan` shapes the DuckDB
// fast path cannot lower cleanly.
//
// `.cursor/plans/array-struct-semantic-path.plan.md` lists the
// divergent subset routed here by `RouteClassifier`'s
// `VisitResolvedArrayScan` property-based promotion:
//
//   * `array_offset_column != nullptr` (UNNEST WITH OFFSET).
//   * `is_outer == true` (outer UNNEST emits one all-NULL row on
//     empty arrays; DuckDB drops the row).
//   * `array_expr_list_size() > 1` (multi-array zip, PAD / STRICT
//     / TRUNCATE).
//
// Out of scope for this header (and deferred per the plan's
// pragmatic posture):
//
//   * Correlated `input_scan != nullptr` (the
//     `FROM t, UNNEST(t.arr)` shape) -- Family 4.
//   * `join_expr != nullptr` (LEFT JOIN UNNEST ... ON ...) -- the
//     ON-clause re-evaluation hooks into Family 4's correlated
//     evaluation.
//   * `ResolvedFlatten` / `ResolvedFlattenedArg` -- Family 5.
//
// Calling `EvaluateArrayScan` on a shape this implementation does
// not yet cover returns a structured `SemanticErrorReason::kNotImplemented`
// so the gateway envelope stays the same as for any other
// "planned but not landed" route.

#include <vector>

#include "absl/status/statusor.h"
#include "backend/engine/semantic/eval_expr.h"
#include "googlesql/resolved_ast/resolved_ast.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace array_struct {

// Walk `scan` and materialize one `ColumnBindings` entry per
// emitted row. The output preserves the analyzer-declared
// `element_column_list` ordering (and, when present, the
// `array_offset_column` binding) so the caller's `ProjectScan` can
// resolve any `ResolvedColumnRef` against the returned bindings.
//
// `parent_ctx` carries the query-level state (parameters, columns
// from a surrounding correlated scan). Today the function only
// reads `parent_ctx.parameters` and `parent_ctx.columns` (the
// latter being null on the standalone-UNNEST happy path).
//
// Materializes eagerly. Array literals at this layer are bounded
// by the analyzer's parser limits (the resolved AST holds them
// in-process), so a vector is acceptable; streaming row sources
// hook into Family 4 when correlated scans land.
absl::StatusOr<std::vector<ColumnBindings>> EvaluateArrayScan(
    const ::googlesql::ResolvedArrayScan& scan, const EvalContext& parent_ctx);

}  // namespace array_struct
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_ARRAY_STRUCT_ARRAY_SCAN_H_
