#include "backend/engine/semantic/scan_eval.h"

#include <string>
#include <utility>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "backend/engine/semantic/error.h"
#include "backend/engine/semantic/eval_expr.h"
#include "backend/engine/semantic/scan_eval_internal.h"
#include "backend/engine/semantic/value.h"
#include "googlesql/resolved_ast/resolved_ast.h"
#include "googlesql/resolved_ast/resolved_node_kind.pb.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {

using scan_eval_internal::MaterializeScanImpl;
using scan_eval_internal::StripBarrierScans;

absl::StatusOr<const ::googlesql::ResolvedProjectScan*> FindOutputProjectScan(
    const ::googlesql::ResolvedScan* scan) {
  scan = StripBarrierScans(scan);
  if (scan == nullptr) {
    return nullptr;
  }
  switch (scan->node_kind()) {
    case ::googlesql::RESOLVED_PROJECT_SCAN:
      return scan->GetAs<::googlesql::ResolvedProjectScan>();
    case ::googlesql::RESOLVED_ORDER_BY_SCAN:
      return FindOutputProjectScan(
          scan->GetAs<::googlesql::ResolvedOrderByScan>()->input_scan());
    case ::googlesql::RESOLVED_WITH_SCAN:
      return FindOutputProjectScan(
          scan->GetAs<::googlesql::ResolvedWithScan>()->query());
    case ::googlesql::RESOLVED_FILTER_SCAN:
      return FindOutputProjectScan(
          scan->GetAs<::googlesql::ResolvedFilterScan>()->input_scan());
    case ::googlesql::RESOLVED_LIMIT_OFFSET_SCAN:
      return FindOutputProjectScan(
          scan->GetAs<::googlesql::ResolvedLimitOffsetScan>()->input_scan());
    case ::googlesql::RESOLVED_ARRAY_SCAN: {
      const auto* array = scan->GetAs<::googlesql::ResolvedArrayScan>();
      if (array->input_scan() != nullptr) {
        return FindOutputProjectScan(array->input_scan());
      }
      return nullptr;
    }
    case ::googlesql::RESOLVED_JOIN_SCAN: {
      const auto* join = scan->GetAs<::googlesql::ResolvedJoinScan>();
      auto left = FindOutputProjectScan(join->left_scan());
      if (!left.ok()) return left.status();
      if (*left != nullptr) return left;
      return FindOutputProjectScan(join->right_scan());
    }
    case ::googlesql::RESOLVED_SET_OPERATION_SCAN: {
      const auto* set_op = scan->GetAs<::googlesql::ResolvedSetOperationScan>();
      for (int i = 0; i < set_op->input_item_list_size(); ++i) {
        const ::googlesql::ResolvedSetOperationItem* item =
            set_op->input_item_list(i);
        if (item == nullptr || item->scan() == nullptr) continue;
        auto found = FindOutputProjectScan(item->scan());
        if (!found.ok()) return found.status();
        if (*found != nullptr) return found;
      }
      return nullptr;
    }
    default:
      return nullptr;
  }
}

absl::StatusOr<std::vector<ColumnBindings>> MaterializeScan(
    const ::googlesql::ResolvedScan* scan, EvalContext& ctx) {
  return MaterializeScanImpl(scan, ctx);
}

absl::StatusOr<Value> EvalSubqueryExpr(
    const ::googlesql::ResolvedSubqueryExpr& node, const EvalContext& ctx) {
  if (node.subquery() == nullptr) {
    return absl::InvalidArgumentError(
        "semantic: ResolvedSubqueryExpr has null subquery");
  }
  EvalContext inner_ctx = ctx;
  ColumnBindings outer_bind;
  if (ctx.columns != nullptr) {
    outer_bind = *ctx.columns;
  }
  inner_ctx.columns = &outer_bind;

  auto rows_or = MaterializeScanImpl(node.subquery(), inner_ctx);
  if (!rows_or.ok()) return rows_or.status();
  const std::vector<ColumnBindings>& rows = *rows_or;
  const ::googlesql::ResolvedScan* sub = StripBarrierScans(node.subquery());
  int value_col_id = -1;
  if (sub != nullptr && sub->column_list_size() > 0) {
    value_col_id = sub->column_list(0).column_id();
  }

  auto value_from_row =
      [&](const ColumnBindings& row) -> absl::StatusOr<Value> {
    if (value_col_id >= 0) {
      auto it = row.find(value_col_id);
      if (it != row.end()) return it->second;
    }
    if (!row.empty()) return row.begin()->second;
    return absl::InternalError(
        "semantic: subquery row missing projected column");
  };

  switch (node.subquery_type()) {
    case ::googlesql::ResolvedSubqueryExpr::EXISTS:
      return Value::Bool(!rows.empty());
    case ::googlesql::ResolvedSubqueryExpr::SCALAR: {
      if (rows.empty()) return Value::Null(node.type());
      if (rows.size() > 1) {
        return MakeSemanticError(
            SemanticErrorReason::kInvalidArgument,
            "semantic: scalar subquery returned more than one row");
      }
      return value_from_row(rows[0]);
    }
    case ::googlesql::ResolvedSubqueryExpr::ARRAY: {
      if (node.type() == nullptr || !node.type()->IsArray()) {
        return absl::InternalError(
            "semantic: ARRAY subquery missing ARRAY type");
      }
      std::vector<Value> elements;
      elements.reserve(rows.size());
      for (const ColumnBindings& row : rows) {
        auto v = value_from_row(row);
        if (!v.ok()) return v.status();
        elements.push_back(*std::move(v));
      }
      return Value::Array(node.type()->AsArray(), std::move(elements));
    }
    case ::googlesql::ResolvedSubqueryExpr::IN: {
      if (node.in_expr() == nullptr) {
        return absl::InvalidArgumentError(
            "semantic: IN subquery missing in_expr");
      }
      auto lhs = EvalExpr(*node.in_expr(), ctx);
      if (!lhs.ok()) return lhs.status();
      if (lhs->is_null()) return Value::NullBool();
      for (const ColumnBindings& row : rows) {
        auto rv = value_from_row(row);
        if (!rv.ok()) return rv.status();
        if (lhs->is_null() || rv->is_null()) continue;
        if (lhs->Equals(*rv)) return Value::Bool(true);
      }
      return Value::Bool(false);
    }
    default:
      return MakeSemanticError(SemanticErrorReason::kNotImplemented,
                               "semantic: subquery type not yet implemented");
  }
}

}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
