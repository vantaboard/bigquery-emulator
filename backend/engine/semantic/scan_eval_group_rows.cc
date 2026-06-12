#include <utility>
#include <vector>

#include "absl/status/statusor.h"
#include "backend/engine/semantic/error.h"
#include "backend/engine/semantic/eval_expr.h"
#include "backend/engine/semantic/scan_eval_internal.h"
#include "googlesql/resolved_ast/resolved_ast.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace scan_eval_internal {

using ::bigquery_emulator::backend::engine::semantic::EvalContext;
using ::bigquery_emulator::backend::engine::semantic::EvalExpr;

absl::StatusOr<std::vector<ColumnBindings>> MaterializeGroupRowsScan(
    const ::googlesql::ResolvedGroupRowsScan& scan, const EvalContext& ctx) {
  if (ctx.group_rows == nullptr || ctx.group_rows->rows == nullptr) {
    return MakeSemanticError(
        SemanticErrorReason::kNotImplemented,
        "semantic: GROUP_ROWS() is only defined inside WITH GROUP ROWS "
        "aggregate evaluation");
  }
  const std::vector<ColumnBindings>& source = *ctx.group_rows->rows;
  std::vector<ColumnBindings> out;
  out.reserve(source.size());
  for (const ColumnBindings& input_row : source) {
    ColumnBindings merged;
    if (ctx.columns != nullptr) {
      merged = *ctx.columns;
    }
    for (const auto& [col_id, val] : input_row) {
      merged[col_id] = val;
    }
    ColumnBindings row;
    row.reserve(scan.column_list_size());
    for (int i = 0; i < scan.input_column_list_size(); ++i) {
      const ::googlesql::ResolvedComputedColumn* cc = scan.input_column_list(i);
      if (cc == nullptr || cc->expr() == nullptr) {
        return absl::InternalError(
            "semantic: GroupRowsScan input_column_list has null expr");
      }
      EvalContext row_ctx = ctx;
      row_ctx.columns = &merged;
      auto val = EvalExpr(*cc->expr(), row_ctx);
      if (!val.ok()) return val.status();
      row.emplace(cc->column().column_id(), *std::move(val));
    }
    out.push_back(std::move(row));
  }
  return out;
}

}  // namespace scan_eval_internal
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
