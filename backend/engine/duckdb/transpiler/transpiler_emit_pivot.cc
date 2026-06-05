#include <algorithm>
#include <optional>
#include <string>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "absl/container/flat_hash_set.h"
#include "absl/log/log.h"
#include "absl/strings/ascii.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"
#include "absl/strings/str_replace.h"
#include "absl/strings/string_view.h"
#include "absl/time/time.h"
#include "backend/engine/disposition.h"
#include "backend/engine/duckdb/transpiler/functions.h"
#include "backend/engine/duckdb/transpiler/transpiler.h"
#include "backend/engine/duckdb/transpiler/transpiler_internal.h"
#include "backend/engine/duckdb/transpiler/types.h"
#include "googlesql/public/catalog.h"
#include "googlesql/public/function.h"
#include "googlesql/public/options.pb.h"
#include "googlesql/public/sql_function.h"
#include "googlesql/public/templated_sql_function.h"
#include "googlesql/public/type.h"
#include "googlesql/public/type.pb.h"
#include "googlesql/public/types/struct_type.h"
#include "googlesql/public/value.h"
#include "googlesql/resolved_ast/resolved_ast.h"
#include "googlesql/resolved_ast/resolved_ast_visitor.h"
#include "googlesql/resolved_ast/resolved_node.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace duckdb {
namespace transpiler {

std::string Transpiler::EmitPivotScan(
    const ::googlesql::ResolvedPivotScan* node) {
  // BigQuery PIVOT lowers to DuckDB conditional aggregation. Each
  // `ResolvedPivotColumn` pins one (pivot_expr_index, pivot_value_index)
  // pair plus the output column name the analyzer chose; the lowering
  // emits `<agg> FILTER (WHERE <for_expr> = <pivot_value>)` for that
  // pair and aliases the result to the analyzer-chosen column name.
  // `group_by_list` columns pass through and become the GROUP BY of
  // the wrapping SELECT.
  //
  // Why FILTER rather than DuckDB's native PIVOT keyword: the native
  // syntax produces DuckDB-named output columns derived from the
  // pivot value(s) (`<val>` or `<val>_<agg>`), which forces an
  // additional column-rename pass on top to land on the
  // analyzer-chosen names in `pivot_column_list`. The FILTER form
  // emits the analyzer's exact output names in one pass, which keeps
  // the downstream `EmitQueryStmt` alias step honest, and matches
  // BigQuery's documented semantics ("the pivot expression is
  // evaluated over the subset of input rows where FOR matches the
  // pivot value") line-for-line.
  //
  // BigQuery PIVOT forbids NULL pivot values; the analyzer rejects
  // them upstream. Using `=` (rather than `IS NOT DISTINCT FROM`)
  // therefore matches the BigQuery semantic that NULL `for_expr`
  // rows match no pivot column.
  //
  // Pivot expressions must be aggregate function calls. Anything
  // else (a scalar, a sub-query, a malformed AST) trips the
  // empty-string fallback so the engine surfaces UNIMPLEMENTED.
  if (node == nullptr) return "";
  std::string inner = EmitScan(node->input_scan());
  if (inner.empty()) return "";

  std::string for_sql = EmitExpr(node->for_expr());
  if (for_sql.empty()) return "";

  std::vector<std::string> pivot_values_sql;
  pivot_values_sql.reserve(node->pivot_value_list_size());
  for (int i = 0; i < node->pivot_value_list_size(); ++i) {
    std::string v = EmitExpr(node->pivot_value_list(i));
    if (v.empty()) return "";
    pivot_values_sql.push_back(std::move(v));
  }

  std::vector<std::string> pivot_exprs_sql;
  pivot_exprs_sql.reserve(node->pivot_expr_list_size());
  for (int i = 0; i < node->pivot_expr_list_size(); ++i) {
    const ::googlesql::ResolvedExpr* expr = node->pivot_expr_list(i);
    if (expr == nullptr ||
        expr->node_kind() != ::googlesql::RESOLVED_AGGREGATE_FUNCTION_CALL) {
      return "";
    }
    std::string e = EmitAggregateFunctionCall(
        expr->GetAs<::googlesql::ResolvedAggregateFunctionCall>());
    if (e.empty()) return "";
    pivot_exprs_sql.push_back(std::move(e));
  }

  std::vector<std::string> projections;
  projections.reserve(node->group_by_list_size() +
                      node->pivot_column_list_size());
  std::vector<std::string> group_by_sql;
  group_by_sql.reserve(node->group_by_list_size());
  for (int i = 0; i < node->group_by_list_size(); ++i) {
    const ::googlesql::ResolvedComputedColumn* gc = node->group_by_list(i);
    if (gc == nullptr) return "";
    std::string e = EmitExpr(gc->expr());
    if (e.empty()) return "";
    std::string quoted_out = internal::QuoteIdent(gc->column().name());
    projections.push_back(
        e == quoted_out ? e : absl::StrCat(e, " AS ", quoted_out));
    group_by_sql.push_back(e);
  }

  for (int i = 0; i < node->pivot_column_list_size(); ++i) {
    const ::googlesql::ResolvedPivotColumn* pc = node->pivot_column_list(i);
    if (pc == nullptr) return "";
    int ei = pc->pivot_expr_index();
    int vi = pc->pivot_value_index();
    if (ei < 0 || ei >= node->pivot_expr_list_size()) return "";
    if (vi < 0 || vi >= node->pivot_value_list_size()) return "";
    std::string filtered = absl::StrCat(pivot_exprs_sql[ei],
                                        " FILTER (WHERE ",
                                        for_sql,
                                        " = ",
                                        pivot_values_sql[vi],
                                        ")");
    projections.push_back(absl::StrCat(
        filtered, " AS ", internal::QuoteIdent(pc->column().name())));
  }

  // Touch column_list for `CheckFieldsAccessed`; the lowering is
  // driven by group_by_list + pivot_column_list rather than by
  // `column_list` directly.
  (void)node->column_list_size();

  std::string select_list =
      projections.empty() ? "*" : absl::StrJoin(projections, ", ");
  std::string sql = absl::StrCat("SELECT ", select_list, " FROM (", inner, ")");
  if (!group_by_sql.empty()) {
    absl::StrAppend(&sql, " GROUP BY ", absl::StrJoin(group_by_sql, ", "));
    absl::StrAppend(&sql, " ORDER BY ", absl::StrJoin(group_by_sql, ", "));
  }
  return sql;
}

std::string Transpiler::EmitUnpivotScan(
    const ::googlesql::ResolvedUnpivotScan* node) {
  // BigQuery UNPIVOT expands each input row into one output row per
  // `unpivot_arg_list` entry, preserving input row order and the IN-
  // list order within each row. We lower that to:
  //
  //   SELECT <projected_input>, u.<value>, ..., u.<label>
  //   FROM (<input>) AS __bq_unpivot_src
  //   CROSS JOIN LATERAL (
  //     VALUES
  //       (__bq_unpivot_src.<col>, ..., <label_0>),
  //       (__bq_unpivot_src.<col>, ..., <label_1>),
  //       ...
  //   ) AS u(<value names>, <label name>)
  //   [WHERE NOT (u.<value_0> IS NULL AND ... AND u.<value_N> IS NULL)]
  //
  // LATERAL VALUES keeps BigQuery's row-major expansion (all unpivot
  // columns for row 1, then row 2, ...) instead of the column-major
  // order a per-arg UNION ALL produces. The WHERE-NOT-all-NULL filter
  // fires when `include_nulls()` is false (UNPIVOT EXCLUDE NULLS).
  //
  // We avoid DuckDB's native UNPIVOT for the same reason
  // `EmitPivotScan` avoids native PIVOT: the analyzer assigns specific
  // output column names through `value_column_list` and
  // `label_column`, and this pattern emits those names in one pass.
  if (node == nullptr) return "";
  if (node->unpivot_arg_list_size() == 0) return "";
  if (node->unpivot_arg_list_size() != node->label_list_size()) return "";

  std::string inner = EmitScan(node->input_scan());
  if (inner.empty()) return "";

  static constexpr char kSrcAlias[] = "__bq_unpivot_src";
  static constexpr char kUnpivotAlias[] = "u";
  static constexpr char kRowOrdCol[] = "__bq_unpivot_rn";
  static constexpr char kArgOrdCol[] = "__bq_unpivot_arg_ord";

  // Stamp each input row with a stable ordinal so the final ORDER BY
  // matches BigQuery's row-major UNPIVOT expansion even when DuckDB
  // returns the inner scan in an arbitrary order.
  inner = absl::StrCat("SELECT *, row_number() OVER () AS ",
                       internal::QuoteIdent(kRowOrdCol),
                       " FROM (",
                       inner,
                       ")");

  // Projected input columns (not unpivoted) come from the source alias.
  std::vector<std::string> projected_input_sql;
  projected_input_sql.reserve(node->projected_input_column_list_size());
  for (int i = 0; i < node->projected_input_column_list_size(); ++i) {
    const ::googlesql::ResolvedComputedColumn* cc =
        node->projected_input_column_list(i);
    if (cc == nullptr) return "";
    const ::googlesql::ResolvedExpr* expr = cc->expr();
    if (expr == nullptr ||
        expr->node_kind() != ::googlesql::RESOLVED_COLUMN_REF) {
      return "";
    }
    const std::string& col_name =
        expr->GetAs<::googlesql::ResolvedColumnRef>()->column().name();
    std::string src =
        absl::StrCat(kSrcAlias, ".", internal::QuoteIdent(col_name));
    std::string quoted_out = internal::QuoteIdent(cc->column().name());
    projected_input_sql.push_back(
        src == quoted_out ? src : absl::StrCat(src, " AS ", quoted_out));
  }

  // Lateral subquery column names: arg ordinal, value columns, label.
  std::vector<std::string> lateral_col_names;
  lateral_col_names.reserve(node->value_column_list_size() + 2);
  lateral_col_names.push_back(internal::QuoteIdent(kArgOrdCol));
  for (int j = 0; j < node->value_column_list_size(); ++j) {
    lateral_col_names.push_back(
        internal::QuoteIdent(node->value_column_list(j).name()));
  }
  const std::string label_col_name =
      internal::QuoteIdent(node->label_column().name());
  lateral_col_names.push_back(label_col_name);

  // One VALUES tuple per unpivot arg, in IN-list order.
  std::vector<std::string> value_tuples;
  value_tuples.reserve(node->unpivot_arg_list_size());
  for (int i = 0; i < node->unpivot_arg_list_size(); ++i) {
    const ::googlesql::ResolvedUnpivotArg* arg = node->unpivot_arg_list(i);
    if (arg == nullptr) return "";
    if (arg->column_list_size() != node->value_column_list_size()) return "";

    std::vector<std::string> tuple_elems;
    tuple_elems.reserve(arg->column_list_size() + 2);
    tuple_elems.push_back(std::to_string(i));
    for (int j = 0; j < arg->column_list_size(); ++j) {
      const ::googlesql::ResolvedColumnRef* ref = arg->column_list(j);
      if (ref == nullptr) return "";
      tuple_elems.push_back(absl::StrCat(
          kSrcAlias, ".", internal::QuoteIdent(ref->column().name())));
    }
    std::string label_sql = EmitLiteral(node->label_list(i));
    if (label_sql.empty()) return "";
    tuple_elems.push_back(std::move(label_sql));
    value_tuples.push_back(
        absl::StrCat("(", absl::StrJoin(tuple_elems, ", "), ")"));
  }

  // Outer SELECT: projected input + lateral value/label columns.
  std::vector<std::string> outer_projections = projected_input_sql;
  outer_projections.reserve(outer_projections.size() +
                            node->value_column_list_size() + 1);
  for (int j = 0; j < node->value_column_list_size(); ++j) {
    outer_projections.push_back(
        absl::StrCat(kUnpivotAlias,
                     ".",
                     internal::QuoteIdent(node->value_column_list(j).name())));
  }
  outer_projections.push_back(absl::StrCat(kUnpivotAlias, ".", label_col_name));

  std::vector<std::string> lateral_def_cols;
  lateral_def_cols.reserve(lateral_col_names.size());
  for (const std::string& col : lateral_col_names) {
    lateral_def_cols.push_back(col);
  }

  std::string sql = absl::StrCat("SELECT ",
                                 absl::StrJoin(outer_projections, ", "),
                                 " FROM (",
                                 inner,
                                 ") AS ",
                                 kSrcAlias,
                                 " CROSS JOIN LATERAL (VALUES ",
                                 absl::StrJoin(value_tuples, ", "),
                                 ") AS ",
                                 kUnpivotAlias,
                                 "(",
                                 absl::StrJoin(lateral_def_cols, ", "),
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

  // Touch column_list so `CheckFieldsAccessed` does not flag a miss.
  (void)node->column_list_size();
  return sql;
}

}  // namespace transpiler
}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
