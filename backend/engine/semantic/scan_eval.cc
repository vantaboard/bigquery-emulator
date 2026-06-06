#include "backend/engine/semantic/scan_eval.h"

#include <string>
#include <utility>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "backend/engine/semantic/error.h"
#include "backend/engine/semantic/eval_expr.h"
#include "backend/engine/semantic/scan_eval_internal.h"
#include "backend/engine/semantic/value.h"
#include "googlesql/resolved_ast/resolved_ast.h"
#include "googlesql/resolved_ast/resolved_ast_visitor.h"
#include "googlesql/resolved_ast/resolved_node_kind.pb.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {

using scan_eval_internal::MaterializeScanImpl;
using scan_eval_internal::StripBarrierScans;

namespace {

void BindCorrelatedSubqueryColumns(
    const ::googlesql::ResolvedSubqueryExpr& node,
    const EvalContext& ctx,
    ColumnBindings& bindings) {
  if (ctx.columns == nullptr || node.parameter_list_size() == 0) {
    return;
  }
  absl::flat_hash_map<std::string, ::googlesql::Value> outer_by_key;
  auto lookup_outer_value =
      [&](const ::googlesql::ResolvedColumn& col) -> const ::googlesql::Value* {
    if (ctx.columns != nullptr) {
      auto it = ctx.columns->find(col.column_id());
      if (it != ctx.columns->end()) return &it->second;
    }
    if (ctx.columns_by_name != nullptr) {
      const std::string qualified =
          absl::StrCat(col.table_name(), ".", col.name());
      auto it = ctx.columns_by_name->find(qualified);
      if (it != ctx.columns_by_name->end()) return &it->second;
      it = ctx.columns_by_name->find(col.name());
      if (it != ctx.columns_by_name->end()) return &it->second;
    }
    return nullptr;
  };
  for (int i = 0; i < node.parameter_list_size(); ++i) {
    const auto* param = node.parameter_list(i);
    if (param == nullptr) continue;
    const ::googlesql::ResolvedColumn& outer_col = param->column();
    const ::googlesql::Value* val = lookup_outer_value(outer_col);
    if (val == nullptr) continue;
    const std::string qualified =
        absl::StrCat(outer_col.table_name(), ".", outer_col.name());
    outer_by_key[qualified] = *val;
    outer_by_key[std::string(outer_col.name())] = *val;
    bindings[outer_col.column_id()] = *val;
  }

  struct CorrelatedBinder : public ::googlesql::ResolvedASTVisitor {
    const absl::flat_hash_map<std::string, ::googlesql::Value>& outer;
    ColumnBindings& bind;

    CorrelatedBinder(
        const absl::flat_hash_map<std::string, ::googlesql::Value>& outer_in,
        ColumnBindings& bind_in)
        : outer(outer_in), bind(bind_in) {}

    absl::Status DefaultVisit(const ::googlesql::ResolvedNode* n) override {
      return ::googlesql::ResolvedASTVisitor::DefaultVisit(n);
    }

    absl::Status VisitResolvedColumnRef(
        const ::googlesql::ResolvedColumnRef* ref) override {
      if (ref == nullptr) {
        return ::googlesql::ResolvedASTVisitor::VisitResolvedColumnRef(ref);
      }
      const int col_id = ref->column().column_id();
      if (bind.contains(col_id)) {
        return ::googlesql::ResolvedASTVisitor::VisitResolvedColumnRef(ref);
      }
      const std::string qualified =
          absl::StrCat(ref->column().table_name(), ".", ref->column().name());
      auto it = outer.find(qualified);
      if (it == outer.end()) {
        it = outer.find(std::string(ref->column().name()));
      }
      if (it != outer.end()) {
        bind.emplace(col_id, it->second);
      }
      return ::googlesql::ResolvedASTVisitor::VisitResolvedColumnRef(ref);
    }
  };

  if (node.subquery() != nullptr) {
    CorrelatedBinder binder{outer_by_key, bindings};
    (void)node.subquery()->Accept(&binder);
  }
}

}  // namespace

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
  BindCorrelatedSubqueryColumns(node, ctx, outer_bind);
  inner_ctx.columns = &outer_bind;
  inner_ctx.columns_by_name = ctx.columns_by_name;

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
