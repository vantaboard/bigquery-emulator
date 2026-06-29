

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {

using scan_eval_internal::MaterializeScanImpl;
using scan_eval_internal::StripBarrierScans;

absl::StatusOr<const ::googlesql::ResolvedProjectScan*> FindOutputProjectScan(
    const ::googlesql::ResolvedScan* scan);

namespace {

void BindCorrelatedSubqueryColumns(
    const ::googlesql::ResolvedSubqueryExpr& node,
    const EvalContext& ctx,
    ColumnBindings& bindings) {
  if (ctx.columns == nullptr) {
    return;
  }
  OuterRowFrame frame = MakeOuterRowFrame(ctx, *ctx.columns, nullptr);
  for (int i = 0; i < node.parameter_list_size(); ++i) {
    const auto* param = node.parameter_list(i);
    if (param == nullptr) continue;
    const ::googlesql::ResolvedColumn& outer_col = param->column();
    auto it = ctx.columns->find(outer_col.column_id());
    if (it == ctx.columns->end()) {
      if (ctx.columns_by_name != nullptr) {
        const std::string qualified =
            absl::StrCat(outer_col.table_name(), ".", outer_col.name());
        auto nit = ctx.columns_by_name->find(qualified);
        if (nit == ctx.columns_by_name->end()) {
          nit = ctx.columns_by_name->find(std::string(outer_col.name()));
        }
        if (nit != ctx.columns_by_name->end()) {
          frame.merged.emplace(outer_col.column_id(), nit->second);
          frame.by_name[qualified] = nit->second;
          frame.by_name[std::string(outer_col.name())] = nit->second;
        }
      }
      continue;
    }
    frame.merged.emplace(outer_col.column_id(), it->second);
    const std::string qualified =
        absl::StrCat(outer_col.table_name(), ".", outer_col.name());
    frame.by_name[qualified] = it->second;
    frame.by_name[std::string(outer_col.name())] = it->second;
  }
  for (const auto& [col_id, val] : *ctx.columns) {
    frame.merged.emplace(col_id, val);
  }
  if (node.subquery() != nullptr) {
    BindCorrelatedColumnRefs(node.subquery(), frame);
  }
  bindings = frame.merged;
}

int ResolveSubqueryValueColumnId(const ::googlesql::ResolvedScan* subquery) {
  auto project_or = FindOutputProjectScan(subquery);
  if (project_or.ok() && *project_or != nullptr) {
    const ::googlesql::ResolvedProjectScan* project = *project_or;
    if (project->expr_list_size() > 0 && project->expr_list(0) != nullptr) {
      return project->expr_list(0)->column().column_id();
    }
    if (project->column_list_size() > 0) {
      return project->column_list(0).column_id();
    }
  }
  const ::googlesql::ResolvedScan* sub = StripBarrierScans(subquery);
  if (sub != nullptr && sub->column_list_size() > 0) {
    return sub->column_list(0).column_id();
  }
  return -1;
}

absl::StatusOr<Value> SubqueryValueFromRow(int value_col_id,
                                           const ColumnBindings& row) {
  if (value_col_id >= 0) {
    auto it = row.find(value_col_id);
    if (it != row.end()) return it->second;
  }
  if (!row.empty()) return row.begin()->second;
  return absl::InternalError("semantic: subquery row missing projected column");
}

absl::StatusOr<Value> EvalExistsSubquery(
    const std::vector<ColumnBindings>& rows) {
  return Value::Bool(!rows.empty());
}

absl::StatusOr<Value> EvalScalarSubquery(
    const std::vector<ColumnBindings>& rows,
    int value_col_id,
    const ::googlesql::Type* type) {
  if (rows.empty()) {
    if (type != nullptr && type->IsArray()) {
      return Value::Array(type->AsArray(), {});
    }
    return Value::Null(type);
  }
  if (rows.size() > 1) {
    return MakeSemanticError(
        SemanticErrorReason::kInvalidArgument,
        "semantic: scalar subquery returned more than one row");
  }
  return SubqueryValueFromRow(value_col_id, rows[0]);
}

absl::StatusOr<Value> EvalArraySubquery(const std::vector<ColumnBindings>& rows,
                                        int value_col_id,
                                        const ::googlesql::Type* type) {
  if (type == nullptr || !type->IsArray()) {
    return absl::InternalError("semantic: ARRAY subquery missing ARRAY type");
  }
  std::vector<Value> elements;
  elements.reserve(rows.size());
  for (const ColumnBindings& row : rows) {
    auto v = SubqueryValueFromRow(value_col_id, row);
    if (!v.ok()) return v.status();
    elements.push_back(*std::move(v));
  }
  return Value::Array(type->AsArray(), std::move(elements));
}

absl::StatusOr<Value> EvalInSubquery(
    const ::googlesql::ResolvedSubqueryExpr& node,
    const std::vector<ColumnBindings>& rows,
    int value_col_id,
    const EvalContext& ctx) {
  if (node.in_expr() == nullptr) {
    return absl::InvalidArgumentError("semantic: IN subquery missing in_expr");
  }
  auto lhs = EvalExpr(*node.in_expr(), ctx);
  if (!lhs.ok()) return lhs.status();
  if (lhs->is_null()) return Value::NullBool();
  for (const ColumnBindings& row : rows) {
    auto rv = SubqueryValueFromRow(value_col_id, row);
    if (!rv.ok()) return rv.status();
    if (lhs->is_null() || rv->is_null()) continue;
    if (lhs->Equals(*rv)) return Value::Bool(true);
  }
  return Value::Bool(false);
}

absl::StatusOr<Value> EvalLikeAnyAllSubquery(
    const ::googlesql::ResolvedSubqueryExpr& node,
    const std::vector<ColumnBindings>& rows,
    int value_col_id,
    const EvalContext& ctx) {
  if (node.in_expr() == nullptr) {
    return absl::InvalidArgumentError(
        "semantic: LIKE ANY/ALL subquery missing in_expr");
  }
  auto lhs = EvalExpr(*node.in_expr(), ctx);
  if (!lhs.ok()) return lhs.status();
  if (lhs->is_null()) return Value::NullBool();
  if (rows.empty()) {
    switch (node.subquery_type()) {
      case ::googlesql::ResolvedSubqueryExpr::LIKE_ANY:
      case ::googlesql::ResolvedSubqueryExpr::NOT_LIKE_ALL:
        return Value::Bool(false);
      case ::googlesql::ResolvedSubqueryExpr::LIKE_ALL:
      case ::googlesql::ResolvedSubqueryExpr::NOT_LIKE_ANY:
        return Value::Bool(true);
      default:
        break;
    }
  }
  bool any_match = false;
  bool all_match = true;
  for (const ColumnBindings& row : rows) {
    auto pattern = SubqueryValueFromRow(value_col_id, row);
    if (!pattern.ok()) return pattern.status();
    if (pattern->is_null()) {
      all_match = false;
      continue;
    }
    auto matched = functions::DispatchLike("$like", {*lhs, *pattern});
    if (!matched.ok()) return matched.status();
    if (matched->is_null()) {
      all_match = false;
      continue;
    }
    if (matched->bool_value()) {
      any_match = true;
    } else {
      all_match = false;
    }
  }
  switch (node.subquery_type()) {
    case ::googlesql::ResolvedSubqueryExpr::LIKE_ANY:
      return Value::Bool(any_match);
    case ::googlesql::ResolvedSubqueryExpr::LIKE_ALL:
      return Value::Bool(all_match);
    case ::googlesql::ResolvedSubqueryExpr::NOT_LIKE_ANY:
      return Value::Bool(!any_match);
    case ::googlesql::ResolvedSubqueryExpr::NOT_LIKE_ALL:
      return Value::Bool(!all_match);
    default:
      break;
  }
  return MakeSemanticError(SemanticErrorReason::kNotImplemented,
                           "semantic: LIKE ANY/ALL subquery type");
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
  const int value_col_id = ResolveSubqueryValueColumnId(node.subquery());

  switch (node.subquery_type()) {
    case ::googlesql::ResolvedSubqueryExpr::EXISTS:
      return EvalExistsSubquery(rows);
    case ::googlesql::ResolvedSubqueryExpr::SCALAR:
      return EvalScalarSubquery(rows, value_col_id, node.type());
    case ::googlesql::ResolvedSubqueryExpr::ARRAY:
      return EvalArraySubquery(rows, value_col_id, node.type());
    case ::googlesql::ResolvedSubqueryExpr::IN:
      return EvalInSubquery(node, rows, value_col_id, ctx);
    case ::googlesql::ResolvedSubqueryExpr::LIKE_ANY:
    case ::googlesql::ResolvedSubqueryExpr::LIKE_ALL:
    case ::googlesql::ResolvedSubqueryExpr::NOT_LIKE_ANY:
    case ::googlesql::ResolvedSubqueryExpr::NOT_LIKE_ALL:
      return EvalLikeAnyAllSubquery(node, rows, value_col_id, ctx);
    default:
      return MakeSemanticError(SemanticErrorReason::kNotImplemented,
                               "semantic: subquery type not yet implemented");
  }
}

}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
