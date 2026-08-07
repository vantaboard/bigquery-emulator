#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_SCAN_EVAL_MATERIALIZE_INTERNAL_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_SCAN_EVAL_MATERIALIZE_INTERNAL_H_

#include <optional>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "absl/status/statusor.h"
#include "backend/engine/semantic/eval_expr.h"
#include "backend/engine/semantic/value.h"
#include "googlesql/resolved_ast/resolved_ast.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace scan_eval_internal {
namespace materialize_internal {

absl::StatusOr<ColumnBindings> ProjectOneInputRow(
    const ::googlesql::ResolvedProjectScan& project,
    const ColumnBindings& input,
    const EvalContext& ctx,
    const absl::flat_hash_map<int, const ::googlesql::ResolvedExpr*>&
        expr_by_column_id);

// Decomposition of an INNER/LEFT join condition into hashable
// equi-key column pairs plus residual conjuncts. Produced by
// `PlanEquiJoin`, consumed by `MaterializeHashEquiJoinRows`.
struct EquiJoinPlan {
  // Parallel vectors: `left_key_ids[i] = right_key_ids[i]` is one
  // equality conjunct of the join expression (column ids resolved
  // against the materialized bindings of each side).
  std::vector<int> left_key_ids{};
  std::vector<int> right_key_ids{};
  // Conjuncts that are not hashable column equalities; evaluated
  // per candidate pair after the hash probe.
  std::vector<const ::googlesql::ResolvedExpr*> residual{};
};

// Returns a hash-join plan when `join` is an INNER/LEFT join whose
// condition contains at least one `left_col = right_col` conjunct
// over hash-safe types; std::nullopt when the nested-loop path must
// be used instead.
std::optional<EquiJoinPlan> PlanEquiJoin(
    const ::googlesql::ResolvedJoinScan& join);

// Hash-based equi-join with SQL join semantics: NULL keys never
// match, residual conjuncts must evaluate to TRUE, and LEFT joins
// null-extend unmatched left rows. Output row order follows
// left-row order (right matches in right-side order), matching the
// nested-loop path.
absl::StatusOr<std::vector<ColumnBindings>> MaterializeHashEquiJoinRows(
    const ::googlesql::ResolvedJoinScan& join,
    const EquiJoinPlan& plan,
    const std::vector<ColumnBindings>& left_rows,
    const std::vector<ColumnBindings>& right_rows,
    const EvalContext& ctx);

absl::StatusOr<std::vector<ColumnBindings>> MaterializeNestedLoopJoinRows(
    const ::googlesql::ResolvedJoinScan& join,
    const std::vector<ColumnBindings>& left_rows,
    const std::vector<ColumnBindings>& right_rows,
    const EvalContext& ctx);

absl::StatusOr<std::vector<ColumnBindings>> MaterializeLateralJoinRows(
    const ::googlesql::ResolvedJoinScan& join,
    const std::vector<ColumnBindings>& left_rows,
    EvalContext& ctx);

absl::StatusOr<std::vector<ColumnBindings>> MaterializeArrayScanWithJoinExpr(
    const ::googlesql::ResolvedArrayScan& scan, EvalContext& ctx);

absl::StatusOr<std::vector<ColumnBindings>> MaterializeArrayScanFromLeftInput(
    const ::googlesql::ResolvedArrayScan& scan, EvalContext& ctx);

}  // namespace materialize_internal
}  // namespace scan_eval_internal
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_SCAN_EVAL_MATERIALIZE_INTERNAL_H_
