#include <utility>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "backend/engine/semantic/array_struct/array_scan.h"
#include "backend/engine/semantic/outer_row_eval.h"
#include "backend/engine/semantic/scan_eval_internal.h"
#include "backend/engine/semantic/scan_eval_materialize_internal.h"
#include "backend/engine/semantic/value.h"
#include "googlesql/resolved_ast/resolved_ast.h"
#include "googlesql/resolved_ast/resolved_node_kind.pb.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace scan_eval_internal {

using ::bigquery_emulator::backend::engine::semantic::EvalContext;
using ::bigquery_emulator::backend::engine::semantic::EvalExpr;

namespace materialize_internal {

absl::StatusOr<ColumnBindings> ProjectOneInputRow(
    const ::googlesql::ResolvedProjectScan& project,
    const ColumnBindings& input,
    const EvalContext& ctx,
    const absl::flat_hash_map<int, const ::googlesql::ResolvedExpr*>&
        expr_by_column_id) {
  ColumnBindings merged;
  if (ctx.columns != nullptr) {
    merged = *ctx.columns;
  }
  for (const auto& [col_id, val] : input) {
    merged[col_id] = val;
  }
  ColumnBindings row = merged;
  row.reserve(row.size() + project.column_list_size());
  absl::flat_hash_map<std::string, Value> by_name;
  PopulateColumnNameBindings(project.input_scan(), merged, by_name);
  if (ctx.columns_by_name != nullptr) {
    for (const auto& [name, val] : *ctx.columns_by_name) {
      by_name[name] = val;
    }
  }
  for (int i = 0; i < project.column_list_size(); ++i) {
    const ::googlesql::ResolvedColumn& col = project.column_list(i);
    const int col_id = col.column_id();
    auto eit = expr_by_column_id.find(col_id);
    EvalContext row_ctx = ctx;
    row_ctx.columns = &merged;
    row_ctx.columns_by_name = &by_name;
    Value v;
    if (eit != expr_by_column_id.end()) {
      auto eval_v = EvalExpr(*eit->second, row_ctx);
      if (!eval_v.ok()) return eval_v.status();
      v = *std::move(eval_v);
    } else {
      auto cit = merged.find(col_id);
      if (cit == merged.end()) {
        return absl::InternalError(
            absl::StrCat("semantic: ProjectScan missing binding for column '",
                         col.name(),
                         "'"));
      }
      v = cit->second;
    }
    row.emplace(col_id, std::move(v));
  }
  return row;
}

void AppendNullRightColumns(const ::googlesql::ResolvedScan* rscan,
                            ColumnBindings* merged) {
  if (rscan == nullptr) return;
  for (int i = 0; i < rscan->column_list_size(); ++i) {
    const ::googlesql::ResolvedColumn& col = rscan->column_list(i);
    merged->emplace(col.column_id(), Value::Null(col.type()));
  }
}

absl::StatusOr<bool> ShouldIncludeJoinRow(
    const ::googlesql::ResolvedJoinScan& join,
    bool is_cross,
    const ColumnBindings& merged,
    const EvalContext& ctx) {
  if (is_cross || join.join_expr() == nullptr) return true;
  EvalContext merged_ctx = ctx;
  merged_ctx.columns = &merged;
  return EvalBoolExpr(join.join_expr(), merged_ctx);
}

absl::StatusOr<std::vector<ColumnBindings>> MaterializeNestedLoopJoinRows(
    const ::googlesql::ResolvedJoinScan& join,
    const std::vector<ColumnBindings>& left_rows,
    const std::vector<ColumnBindings>& right_rows,
    const EvalContext& ctx) {
  const bool is_left_outer =
      join.join_type() == ::googlesql::ResolvedJoinScan::LEFT;
  const bool is_cross =
      join.join_expr() == nullptr &&
      join.join_type() == ::googlesql::ResolvedJoinScan::INNER;
  const ::googlesql::ResolvedScan* rscan = StripBarrierScans(join.right_scan());

  std::vector<ColumnBindings> out;
  for (const ColumnBindings& lrow : left_rows) {
    bool any_match = false;
    for (const ColumnBindings& rrow : right_rows) {
      ColumnBindings merged = lrow;
      merged.insert(rrow.begin(), rrow.end());
      auto include_or = ShouldIncludeJoinRow(join, is_cross, merged, ctx);
      if (!include_or.ok()) return include_or.status();
      if (*include_or) {
        any_match = true;
        out.push_back(std::move(merged));
      }
    }
    if (!any_match && is_left_outer) {
      ColumnBindings merged = lrow;
      AppendNullRightColumns(rscan, &merged);
      out.push_back(std::move(merged));
    }
  }
  return out;
}

absl::StatusOr<std::vector<ColumnBindings>> MaterializeLateralJoinRows(
    const ::googlesql::ResolvedJoinScan& join,
    const std::vector<ColumnBindings>& left_rows,
    EvalContext& ctx) {
  const bool is_left_outer =
      join.join_type() == ::googlesql::ResolvedJoinScan::LEFT;
  const bool is_cross =
      join.join_expr() == nullptr &&
      join.join_type() == ::googlesql::ResolvedJoinScan::INNER;
  const ::googlesql::ResolvedScan* rscan = StripBarrierScans(join.right_scan());

  std::vector<ColumnBindings> out;
  for (const ColumnBindings& lrow : left_rows) {
    OuterRowFrame frame = MakeOuterRowFrame(ctx, lrow, join.left_scan());
    BindCorrelatedColumnRefs(join.right_scan(), frame);
    auto right_or = MaterializeScanImpl(join.right_scan(), frame.row_ctx);
    if (!right_or.ok()) return right_or.status();

    bool any_match = false;
    for (const ColumnBindings& rrow : *right_or) {
      ColumnBindings merged = lrow;
      merged.insert(rrow.begin(), rrow.end());
      auto include_or = ShouldIncludeJoinRow(join, is_cross, merged, ctx);
      if (!include_or.ok()) return include_or.status();
      if (*include_or) {
        any_match = true;
        out.push_back(std::move(merged));
      }
    }
    if (!any_match && is_left_outer) {
      ColumnBindings merged = lrow;
      AppendNullRightColumns(rscan, &merged);
      out.push_back(std::move(merged));
    }
  }
  return out;
}

absl::StatusOr<std::vector<ColumnBindings>> MaterializeArrayScanWithJoinExpr(
    const ::googlesql::ResolvedArrayScan& scan, EvalContext& ctx) {
  auto left_or = MaterializeScanImpl(scan.input_scan(), ctx);
  if (!left_or.ok()) return left_or.status();
  std::vector<ColumnBindings> out;
  for (const ColumnBindings& lrow : *left_or) {
    OuterRowFrame frame = MakeOuterRowFrame(ctx, lrow, scan.input_scan());
    auto array_rows = array_struct::EvaluateArrayScan(scan, frame.row_ctx);
    if (!array_rows.ok()) return array_rows.status();
    bool any = false;
    for (const ColumnBindings& arow : *array_rows) {
      ColumnBindings merged = lrow;
      merged.insert(arow.begin(), arow.end());
      EvalContext merged_ctx = ctx;
      merged_ctx.columns = &merged;
      auto ok = EvalBoolExpr(scan.join_expr(), merged_ctx);
      if (!ok.ok()) return ok.status();
      if (*ok) {
        any = true;
        out.push_back(std::move(merged));
      }
    }
    if (!any && scan.is_outer()) {
      ColumnBindings merged = lrow;
      for (int i = 0; i < scan.element_column_list_size(); ++i) {
        merged.emplace(scan.element_column_list(i).column_id(),
                       Value::Null(scan.element_column_list(i).type()));
      }
      if (scan.array_offset_column() != nullptr) {
        merged.emplace(scan.array_offset_column()->column().column_id(),
                       Value::NullInt64());
      }
      out.push_back(std::move(merged));
    }
  }
  return out;
}

absl::StatusOr<std::vector<ColumnBindings>> MaterializeArrayScanFromLeftInput(
    const ::googlesql::ResolvedArrayScan& scan, EvalContext& ctx) {
  auto left_or = MaterializeScanImpl(scan.input_scan(), ctx);
  if (!left_or.ok()) return left_or.status();
  std::vector<ColumnBindings> out;
  for (const ColumnBindings& lrow : *left_or) {
    OuterRowFrame frame = MakeOuterRowFrame(ctx, lrow, scan.input_scan());
    auto array_rows = array_struct::EvaluateArrayScan(scan, frame.row_ctx);
    if (!array_rows.ok()) return array_rows.status();
    for (const ColumnBindings& arow : *array_rows) {
      ColumnBindings merged = lrow;
      merged.insert(arow.begin(), arow.end());
      out.push_back(std::move(merged));
    }
  }
  return out;
}

}  // namespace materialize_internal
}  // namespace scan_eval_internal
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
