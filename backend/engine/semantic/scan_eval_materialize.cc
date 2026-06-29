
#include "googlesql/public/simple_catalog.h"
#include "googlesql/public/type.h"
#include "googlesql/public/types/struct_type.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace scan_eval_internal {

using ::bigquery_emulator::backend::engine::semantic::EvalContext;
using ::bigquery_emulator::backend::engine::semantic::EvalExpr;

namespace {

void AppendColumnNameBindings(
    const ::googlesql::ResolvedScan* scan,
    const ColumnBindings& row,
    absl::flat_hash_map<std::string, ::googlesql::Value>& out) {
  scan = StripBarrierScans(scan);
  if (scan == nullptr) return;
  if (scan->node_kind() == ::googlesql::RESOLVED_ARRAY_SCAN) {
    const auto* array_scan = scan->GetAs<::googlesql::ResolvedArrayScan>();
    for (int i = 0; i < array_scan->element_column_list_size(); ++i) {
      const ::googlesql::ResolvedColumn& col =
          array_scan->element_column_list(i);
      auto it = row.find(col.column_id());
      if (it == row.end()) continue;
      out[std::string(col.name())] = it->second;
      out[absl::StrCat(col.table_name(), ".", col.name())] = it->second;
    }
    if (array_scan->array_offset_column() != nullptr) {
      const ::googlesql::ResolvedColumn& col =
          array_scan->array_offset_column()->column();
      auto it = row.find(col.column_id());
      if (it != row.end()) {
        out[std::string(col.name())] = it->second;
        out[absl::StrCat(col.table_name(), ".", col.name())] = it->second;
      }
    }
  }
  for (int i = 0; i < scan->column_list_size(); ++i) {
    const ::googlesql::ResolvedColumn& col = scan->column_list(i);
    auto it = row.find(col.column_id());
    if (it == row.end()) continue;
    out[std::string(col.name())] = it->second;
    out[absl::StrCat(col.table_name(), ".", col.name())] = it->second;
  }
  if (scan->node_kind() == ::googlesql::RESOLVED_PROJECT_SCAN) {
    const auto* project = scan->GetAs<::googlesql::ResolvedProjectScan>();
    if (project->input_scan() != nullptr) {
      AppendColumnNameBindings(project->input_scan(), row, out);
    }
  }
}

}  // namespace

void PopulateColumnNameBindingsDeep(
    const ::googlesql::ResolvedScan* scan,
    const ColumnBindings& row,
    absl::flat_hash_map<std::string, ::googlesql::Value>& out) {
  out.clear();
  scan = StripBarrierScans(scan);
  while (scan != nullptr) {
    AppendColumnNameBindings(scan, row, out);
    switch (scan->node_kind()) {
      case ::googlesql::RESOLVED_PROJECT_SCAN:
        scan = StripBarrierScans(
            scan->GetAs<::googlesql::ResolvedProjectScan>()->input_scan());
        break;
      case ::googlesql::RESOLVED_FILTER_SCAN:
        scan = StripBarrierScans(
            scan->GetAs<::googlesql::ResolvedFilterScan>()->input_scan());
        break;
      case ::googlesql::RESOLVED_AGGREGATE_SCAN:
      case ::googlesql::RESOLVED_ANONYMIZED_AGGREGATE_SCAN:
      case ::googlesql::RESOLVED_DIFFERENTIAL_PRIVACY_AGGREGATE_SCAN:
      case ::googlesql::RESOLVED_AGGREGATION_THRESHOLD_AGGREGATE_SCAN: {
        const auto* aggregate =
            dynamic_cast<const ::googlesql::ResolvedAggregateScanBase*>(scan);
        if (aggregate == nullptr) {
          scan = nullptr;
          break;
        }
        scan = StripBarrierScans(aggregate->input_scan());
        break;
      }
      case ::googlesql::RESOLVED_ARRAY_SCAN:
        scan = StripBarrierScans(
            scan->GetAs<::googlesql::ResolvedArrayScan>()->input_scan());
        break;
      default:
        scan = nullptr;
        break;
    }
  }
}

void PopulateColumnNameBindings(
    const ::googlesql::ResolvedScan* scan,
    const ColumnBindings& row,
    absl::flat_hash_map<std::string, ::googlesql::Value>& out) {
  out.clear();
  AppendColumnNameBindings(scan, row, out);
}

absl::StatusOr<std::vector<ColumnBindings>> ProjectRows(
    const ::googlesql::ResolvedProjectScan& project,
    const std::vector<ColumnBindings>& input_rows,
    const EvalContext& ctx) {
  absl::flat_hash_map<int, const ::googlesql::ResolvedExpr*> expr_by_column_id;
  for (int i = 0; i < project.expr_list_size(); ++i) {
    const ::googlesql::ResolvedComputedColumn* cc = project.expr_list(i);
    if (cc == nullptr || cc->expr() == nullptr) {
      return absl::InternalError(
          "semantic: ResolvedComputedColumn has null expr");
    }
    expr_by_column_id[cc->column().column_id()] = cc->expr();
  }

  if (input_rows.empty()) {
    return std::vector<ColumnBindings>{};
  }
  std::vector<ColumnBindings> out;
  out.reserve(input_rows.size());
  for (const ColumnBindings& input : input_rows) {
    // Keep inner scan bindings (e.g. `$agg1`) so the executor's
    // `ProjectOneRow` pass can still resolve column refs in the
    // output projection expressions.
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
    out.push_back(std::move(row));
  }
  return out;
}

absl::StatusOr<std::vector<ColumnBindings>> MaterializeTableScan(
    const ::googlesql::ResolvedTableScan& scan) {
  if (scan.table() == nullptr) {
    return absl::InternalError("semantic: TableScan has null table");
  }
  const auto* simple_table =
      dynamic_cast<const ::googlesql::SimpleTable*>(scan.table());
  if (simple_table == nullptr) {
    return MakeSemanticError(SemanticErrorReason::kNotImplemented,
                             absl::StrCat("semantic: table '",
                                          scan.table()->FullName(),
                                          "' is not iterable via SimpleTable"));
  }
  if (scan.column_list_size() != scan.column_index_list_size()) {
    return absl::InternalError(
        "semantic: TableScan column_list / column_index_list size mismatch");
  }
  std::vector<int> column_idxs;
  column_idxs.reserve(scan.column_list_size());
  for (int i = 0; i < scan.column_list_size(); ++i) {
    column_idxs.push_back(scan.column_index_list(i));
  }
  absl::StatusOr<std::unique_ptr<::googlesql::EvaluatorTableIterator>> iter_or =
      simple_table->CreateEvaluatorTableIterator(column_idxs);
  if (!iter_or.ok()) return iter_or.status();
  std::unique_ptr<::googlesql::EvaluatorTableIterator> iter =
      std::move(iter_or).value();
  std::vector<ColumnBindings> out;
  while (iter->NextRow()) {
    absl::Status st = iter->Status();
    if (!st.ok()) return st;
    ColumnBindings row;
    row.reserve(scan.column_list_size());
    for (int i = 0; i < scan.column_list_size(); ++i) {
      const ::googlesql::ResolvedColumn& col = scan.column_list(i);
      row.emplace(col.column_id(), iter->GetValue(i));
    }
    out.push_back(std::move(row));
  }
  absl::Status st = iter->Status();
  if (!st.ok()) return st;
  return out;
}

absl::StatusOr<std::vector<ColumnBindings>> MaterializeSingleRowScan(
    const ::googlesql::ResolvedSingleRowScan& scan) {
  ColumnBindings row;
  for (int i = 0; i < scan.column_list_size(); ++i) {
    const ::googlesql::ResolvedColumn& col = scan.column_list(i);
    row.emplace(col.column_id(), Value::Null(col.type()));
  }
  return std::vector<ColumnBindings>{std::move(row)};
}

absl::StatusOr<std::vector<ColumnBindings>> MaterializeWithRefScan(
    const ::googlesql::ResolvedWithRefScan& ref, EvalContext& ctx) {
  if (ctx.with_tables == nullptr) {
    return absl::InternalError(
        "semantic: WithRefScan without active WithScan bindings");
  }
  auto it = ctx.with_tables->find(std::string(ref.with_query_name()));
  if (it == ctx.with_tables->end()) {
    return MakeSemanticError(
        SemanticErrorReason::kInvalidArgument,
        absl::StrCat("semantic: unknown CTE '", ref.with_query_name(), "'"));
  }
  const CteTable& cte = it->second;
  if (static_cast<int>(cte.column_ids.size()) != ref.column_list_size()) {
    return absl::InternalError(
        "semantic: WithRefScan column count does not match CTE");
  }
  std::vector<ColumnBindings> out;
  out.reserve(cte.rows.size());
  for (const ColumnBindings& cte_row : cte.rows) {
    ColumnBindings row;
    row.reserve(ref.column_list_size());
    for (int i = 0; i < ref.column_list_size(); ++i) {
      const ::googlesql::ResolvedColumn& dst = ref.column_list(i);
      const int src_id = cte.column_ids[i];
      auto cit = cte_row.find(src_id);
      if (cit == cte_row.end()) {
        return absl::InternalError(
            absl::StrCat("semantic: CTE row missing column_id=", src_id));
      }
      row.emplace(dst.column_id(), cit->second);
    }
    out.push_back(std::move(row));
  }
  return out;
}

absl::StatusOr<std::vector<ColumnBindings>> MaterializeJoinScan(
    const ::googlesql::ResolvedJoinScan& join, EvalContext& ctx) {
  if (join.is_lateral()) {
    auto left_or = MaterializeScanImpl(join.left_scan(), ctx);
    if (!left_or.ok()) return left_or.status();

    const bool is_left_outer =
        join.join_type() == ::googlesql::ResolvedJoinScan::LEFT;
    const bool is_cross =
        join.join_expr() == nullptr &&
        join.join_type() == ::googlesql::ResolvedJoinScan::INNER;
    const ::googlesql::ResolvedScan* rscan =
        StripBarrierScans(join.right_scan());

    std::vector<ColumnBindings> out;
    for (const ColumnBindings& lrow : *left_or) {
      OuterRowFrame frame = MakeOuterRowFrame(ctx, lrow, join.left_scan());
      BindCorrelatedColumnRefs(join.right_scan(), frame);
      auto right_or = MaterializeScanImpl(join.right_scan(), frame.row_ctx);
      if (!right_or.ok()) return right_or.status();

      bool any_match = false;
      for (const ColumnBindings& rrow : *right_or) {
        ColumnBindings merged = lrow;
        merged.insert(rrow.begin(), rrow.end());
        EvalContext merged_ctx = ctx;
        merged_ctx.columns = &merged;
        bool include = is_cross || join.join_expr() == nullptr;
        if (!include) {
          auto ok = EvalBoolExpr(join.join_expr(), merged_ctx);
          if (!ok.ok()) return ok.status();
          include = *ok;
        }
        if (include) {
          any_match = true;
          out.push_back(std::move(merged));
        }
      }
      if (!any_match && is_left_outer) {
        ColumnBindings merged = lrow;
        if (rscan != nullptr) {
          for (int i = 0; i < rscan->column_list_size(); ++i) {
            const ::googlesql::ResolvedColumn& col = rscan->column_list(i);
            merged.emplace(col.column_id(), Value::Null(col.type()));
          }
        }
        out.push_back(std::move(merged));
      }
    }
    return out;
  }
  if (join.join_type() == ::googlesql::ResolvedJoinScan::RIGHT ||
      join.join_type() == ::googlesql::ResolvedJoinScan::FULL) {
    return MakeSemanticError(SemanticErrorReason::kNotImplemented,
                             "semantic: RIGHT/FULL JOIN not yet implemented");
  }
  auto left_or = MaterializeScanImpl(join.left_scan(), ctx);
  if (!left_or.ok()) return left_or.status();
  auto right_or = MaterializeScanImpl(join.right_scan(), ctx);
  if (!right_or.ok()) return right_or.status();

  const bool is_left_outer =
      join.join_type() == ::googlesql::ResolvedJoinScan::LEFT;
  const bool is_cross =
      join.join_expr() == nullptr &&
      join.join_type() == ::googlesql::ResolvedJoinScan::INNER;

  const ::googlesql::ResolvedScan* rscan = StripBarrierScans(join.right_scan());

  std::vector<ColumnBindings> out;
  for (const ColumnBindings& lrow : *left_or) {
    bool any_match = false;
    for (const ColumnBindings& rrow : *right_or) {
      ColumnBindings merged = lrow;
      merged.insert(rrow.begin(), rrow.end());
      EvalContext merged_ctx = ctx;
      merged_ctx.columns = &merged;
      bool include = is_cross || join.join_expr() == nullptr;
      if (!include) {
        auto ok = EvalBoolExpr(join.join_expr(), merged_ctx);
        if (!ok.ok()) return ok.status();
        include = *ok;
      }
      if (include) {
        any_match = true;
        out.push_back(std::move(merged));
      }
    }
    if (!any_match && is_left_outer) {
      ColumnBindings merged = lrow;
      if (rscan != nullptr) {
        for (int i = 0; i < rscan->column_list_size(); ++i) {
          const ::googlesql::ResolvedColumn& col = rscan->column_list(i);
          merged.emplace(col.column_id(), Value::Null(col.type()));
        }
      }
      out.push_back(std::move(merged));
    }
  }
  return out;
}

absl::StatusOr<std::vector<ColumnBindings>> MaterializeArrayScan(
    const ::googlesql::ResolvedArrayScan& scan, EvalContext& ctx) {
  if (scan.join_expr() != nullptr && scan.input_scan() != nullptr) {
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

  if (scan.input_scan() != nullptr &&
      scan.input_scan()->node_kind() != ::googlesql::RESOLVED_SINGLE_ROW_SCAN) {
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

  auto rows_or = array_struct::EvaluateArrayScan(scan, ctx);
  if (!rows_or.ok()) return rows_or.status();
  const int n_arrays = scan.array_expr_list_size();
  for (ColumnBindings& row : *rows_or) {
    array_struct::AliasUnnestPublicColumnIds(scan, n_arrays, row);
    array_struct::InjectArrayScanInternalColumns(scan, row);
  }
  return rows_or;
}

}  // namespace scan_eval_internal
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
