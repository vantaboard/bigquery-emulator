#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_OUTER_ROW_EVAL_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_OUTER_ROW_EVAL_H_

// Per-outer-row evaluation frame for correlated scans and lateral
// joins. Generalizes the UDF invocation binding pattern
// (`FrameStack` + `EvalContext`) to `ColumnBindings` keyed by
// analyzer `column_id`, which is what `EvalExpr` / `MaterializeScan`
// resolve against.
//
// Callers stream outer rows (from `MaterializeScanImpl` on an input
// scan), push each row's bindings into a fresh `OuterRowFrame`, and
// evaluate an inner scan or expression against `frame.row_ctx`.

#include "absl/container/flat_hash_map.h"
#include "backend/engine/semantic/eval_context.h"
#include "backend/engine/semantic/value.h"
#include "googlesql/resolved_ast/resolved_ast.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {

struct OuterRowFrame {
  ColumnBindings merged;
  absl::flat_hash_map<std::string, ::googlesql::Value> by_name;
  EvalContext row_ctx;
};

// Bind `outer_row` into a child evaluation context. When
// `scan_for_names` is non-null, populate `columns_by_name` from the
// scan's column list so unqualified / qualified name lookups match
// BigQuery's resolution rules.
OuterRowFrame MakeOuterRowFrame(
    const EvalContext& parent_ctx,
    const ColumnBindings& outer_row,
    const ::googlesql::ResolvedScan* scan_for_names);

// Walk `correlated_scan` and bind any `ResolvedColumnRef` nodes that
// reference columns already present in `frame` into `frame.merged`.
void BindCorrelatedColumnRefs(const ::googlesql::ResolvedScan* correlated_scan,
                              OuterRowFrame& frame);

}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_OUTER_ROW_EVAL_H_
