#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_SCAN_EVAL_MATERIALIZE_INTERNAL_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_SCAN_EVAL_MATERIALIZE_INTERNAL_H_

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

absl::StatusOr<std::vector<ColumnBindings>> MaterializeNestedLoopJoinRows(
    const ::googlesql::ResolvedJoinScan& join,
    const std::vector<ColumnBindings>& left_rows,
    const std::vector<ColumnBindings>& right_rows,
    EvalContext& ctx);

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
