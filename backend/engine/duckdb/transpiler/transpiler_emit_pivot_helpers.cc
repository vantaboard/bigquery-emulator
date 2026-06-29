#include "backend/engine/duckdb/transpiler/transpiler_emit_pivot_helpers.h"

#include <string>
#include <utility>
#include <vector>

#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"
#include "absl/strings/string_view.h"
#include "backend/engine/duckdb/transpiler/transpiler_internal.h"
#include "googlesql/resolved_ast/resolved_node.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace duckdb {
namespace transpiler {

namespace {

bool CollectPivotValues(const ::googlesql::ResolvedPivotScan* node,
                        const EmitExprFn& emit_expr,
                        std::vector<std::string>* pivot_values_sql) {
  pivot_values_sql->reserve(node->pivot_value_list_size());
  for (int i = 0; i < node->pivot_value_list_size(); ++i) {
    std::string v = emit_expr(node->pivot_value_list(i));
    if (v.empty()) return false;
    pivot_values_sql->push_back(std::move(v));
  }
  return true;
}

bool CollectPivotExprs(const ::googlesql::ResolvedPivotScan* node,
                       const EmitAggregateFn& emit_agg,
                       std::vector<std::string>* pivot_exprs_sql) {
  pivot_exprs_sql->reserve(node->pivot_expr_list_size());
  for (int i = 0; i < node->pivot_expr_list_size(); ++i) {
    const ::googlesql::ResolvedExpr* expr = node->pivot_expr_list(i);
    if (expr == nullptr ||
        expr->node_kind() != ::googlesql::RESOLVED_AGGREGATE_FUNCTION_CALL) {
      return false;
    }
    std::string e =
        emit_agg(expr->GetAs<::googlesql::ResolvedAggregateFunctionCall>());
    if (e.empty()) return false;
    pivot_exprs_sql->push_back(std::move(e));
  }
  return true;
}

bool CollectPivotGroupBy(const ::googlesql::ResolvedPivotScan* node,
                         const EmitExprFn& emit_expr,
                         std::vector<std::string>* projections,
                         std::vector<std::string>* group_by_sql) {
  projections->reserve(projections->size() + node->group_by_list_size());
  group_by_sql->reserve(node->group_by_list_size());
  for (int i = 0; i < node->group_by_list_size(); ++i) {
    const ::googlesql::ResolvedComputedColumn* gc = node->group_by_list(i);
    if (gc == nullptr) return false;
    std::string e = emit_expr(gc->expr());
    if (e.empty()) return false;
    std::string quoted_out = internal::QuoteIdent(gc->column().name());
    projections->push_back(
        e == quoted_out ? e : absl::StrCat(e, " AS ", quoted_out));
    group_by_sql->push_back(e);
  }
  return true;
}

bool AppendPivotColumnProjections(
    const ::googlesql::ResolvedPivotScan* node,
    absl::string_view for_sql,
    const std::vector<std::string>& pivot_exprs_sql,
    const std::vector<std::string>& pivot_values_sql,
    std::vector<std::string>* projections) {
  for (int i = 0; i < node->pivot_column_list_size(); ++i) {
    const ::googlesql::ResolvedPivotColumn* pc = node->pivot_column_list(i);
    if (pc == nullptr) return false;
    const int ei = pc->pivot_expr_index();
    const int vi = pc->pivot_value_index();
    if (ei < 0 || ei >= node->pivot_expr_list_size()) return false;
    if (vi < 0 || vi >= node->pivot_value_list_size()) return false;
    std::string filtered = absl::StrCat(pivot_exprs_sql[ei],
                                        " FILTER (WHERE ",
                                        for_sql,
                                        " = ",
                                        pivot_values_sql[vi],
                                        ")");
    projections->push_back(absl::StrCat(
        filtered, " AS ", internal::QuoteIdent(pc->column().name())));
  }
  return true;
}

}  // namespace

bool CollectPivotScanParts(const ::googlesql::ResolvedPivotScan* node,
                           absl::string_view for_sql,
                           const EmitExprFn& emit_expr,
                           const EmitAggregateFn& emit_agg,
                           PivotScanParts* out) {
  if (!CollectPivotValues(node, emit_expr, &out->pivot_values_sql)) {
    return false;
  }
  if (!CollectPivotExprs(node, emit_agg, &out->pivot_exprs_sql)) {
    return false;
  }
  out->projections.clear();
  if (!CollectPivotGroupBy(
          node, emit_expr, &out->projections, &out->group_by_sql)) {
    return false;
  }
  return AppendPivotColumnProjections(node,
                                      for_sql,
                                      out->pivot_exprs_sql,
                                      out->pivot_values_sql,
                                      &out->projections);
}

std::string BuildPivotSelectSql(absl::string_view inner,
                                const PivotScanParts& parts) {
  std::string select_list =
      parts.projections.empty() ? "*" : absl::StrJoin(parts.projections, ", ");
  std::string sql = absl::StrCat("SELECT ", select_list, " FROM (", inner, ")");
  if (!parts.group_by_sql.empty()) {
    absl::StrAppend(
        &sql, " GROUP BY ", absl::StrJoin(parts.group_by_sql, ", "));
    absl::StrAppend(
        &sql, " ORDER BY ", absl::StrJoin(parts.group_by_sql, ", "));
  }
  return sql;
}

bool CollectUnpivotScanParts(const ::googlesql::ResolvedUnpivotScan* node,
                             const EmitExprFn& emit_expr,
                             const EmitLiteralFn& emit_literal,
                             UnpivotScanParts* out) {
  static constexpr char kSrcAlias[] = "__bq_unpivot_src";

  out->projected_input_sql.reserve(node->projected_input_column_list_size());
  for (int i = 0; i < node->projected_input_column_list_size(); ++i) {
    const ::googlesql::ResolvedComputedColumn* cc =
        node->projected_input_column_list(i);
    if (cc == nullptr) return false;
    const ::googlesql::ResolvedExpr* expr = cc->expr();
    if (expr == nullptr ||
        expr->node_kind() != ::googlesql::RESOLVED_COLUMN_REF) {
      return false;
    }
    const std::string& col_name =
        expr->GetAs<::googlesql::ResolvedColumnRef>()->column().name();
    std::string src =
        absl::StrCat(kSrcAlias, ".", internal::QuoteIdent(col_name));
    std::string quoted_out = internal::QuoteIdent(cc->column().name());
    out->projected_input_sql.push_back(
        src == quoted_out ? src : absl::StrCat(src, " AS ", quoted_out));
  }

  static constexpr char kArgOrdCol[] = "__bq_unpivot_arg_ord";
  out->lateral_col_names.reserve(node->value_column_list_size() + 2);
  out->lateral_col_names.push_back(internal::QuoteIdent(kArgOrdCol));
  for (int j = 0; j < node->value_column_list_size(); ++j) {
    out->lateral_col_names.push_back(
        internal::QuoteIdent(node->value_column_list(j).name()));
  }
  out->label_col_name = internal::QuoteIdent(node->label_column().name());
  out->lateral_col_names.push_back(out->label_col_name);

  out->value_tuples.reserve(node->unpivot_arg_list_size());
  for (int i = 0; i < node->unpivot_arg_list_size(); ++i) {
    const ::googlesql::ResolvedUnpivotArg* arg = node->unpivot_arg_list(i);
    if (arg == nullptr) return false;
    if (arg->column_list_size() != node->value_column_list_size()) {
      return false;
    }

    std::vector<std::string> tuple_elems;
    tuple_elems.reserve(arg->column_list_size() + 2);
    tuple_elems.push_back(std::to_string(i));
    for (int j = 0; j < arg->column_list_size(); ++j) {
      const ::googlesql::ResolvedColumnRef* ref = arg->column_list(j);
      if (ref == nullptr) return false;
      tuple_elems.push_back(absl::StrCat(
          kSrcAlias, ".", internal::QuoteIdent(ref->column().name())));
    }
    std::string label_sql = emit_literal(node->label_list(i));
    if (label_sql.empty()) return false;
    tuple_elems.push_back(std::move(label_sql));
    out->value_tuples.push_back(
        absl::StrCat("(", absl::StrJoin(tuple_elems, ", "), ")"));
  }

  out->outer_projections = out->projected_input_sql;
  out->outer_projections.reserve(out->outer_projections.size() +
                                 node->value_column_list_size() + 1);
  static constexpr char kUnpivotAlias[] = "u";
  for (int j = 0; j < node->value_column_list_size(); ++j) {
    out->outer_projections.push_back(
        absl::StrCat(kUnpivotAlias,
                     ".",
                     internal::QuoteIdent(node->value_column_list(j).name())));
  }
  out->outer_projections.push_back(
      absl::StrCat(kUnpivotAlias, ".", out->label_col_name));
  return true;
}

std::string BuildUnpivotSelectSql(absl::string_view inner,
                                  const ::googlesql::ResolvedUnpivotScan* node,
                                  const UnpivotScanParts& parts) {
  static constexpr char kSrcAlias[] = "__bq_unpivot_src";
  static constexpr char kUnpivotAlias[] = "u";
  static constexpr char kRowOrdCol[] = "__bq_unpivot_rn";
  static constexpr char kArgOrdCol[] = "__bq_unpivot_arg_ord";

  std::string sql = absl::StrCat("SELECT ",
                                 absl::StrJoin(parts.outer_projections, ", "),
                                 " FROM (",
                                 inner,
                                 ") AS ",
                                 kSrcAlias,
                                 " CROSS JOIN LATERAL (VALUES ",
                                 absl::StrJoin(parts.value_tuples, ", "),
                                 ") AS ",
                                 kUnpivotAlias,
                                 "(",
                                 absl::StrJoin(parts.lateral_col_names, ", "),
                                 ")");

  if (!node->include_nulls() && node->value_column_list_size() > 0) {
    std::vector<std::string> null_checks;
    null_checks.reserve(node->value_column_list_size());
    for (int j = 0; j < node->value_column_list_size(); ++j) {
      null_checks.push_back(
          absl::StrCat(kUnpivotAlias,
                       ".",
                       internal::QuoteIdent(node->value_column_list(j).name()),
                       " IS NULL"));
    }
    absl::StrAppend(
        &sql, " WHERE NOT (", absl::StrJoin(null_checks, " AND "), ")");
  }

  absl::StrAppend(&sql,
                  " ORDER BY ",
                  kSrcAlias,
                  ".",
                  internal::QuoteIdent(kRowOrdCol),
                  ", ",
                  kUnpivotAlias,
                  ".",
                  internal::QuoteIdent(kArgOrdCol));
  return sql;
}

}  // namespace transpiler
}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
