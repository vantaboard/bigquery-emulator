#include <string>

#include "absl/strings/str_cat.h"
#include "backend/engine/duckdb/transpiler/transpiler.h"
#include "backend/engine/duckdb/transpiler/transpiler_emit_pivot_helpers.h"
#include "backend/engine/duckdb/transpiler/transpiler_internal.h"
#include "googlesql/resolved_ast/resolved_ast.h"

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

  const EmitExprFn emit_expr = [this](const ::googlesql::ResolvedExpr* expr) {
    return EmitExpr(expr);
  };
  const EmitAggregateFn emit_agg =
      [this](const ::googlesql::ResolvedAggregateFunctionCall* agg) {
        return EmitAggregateFunctionCall(agg);
      };

  PivotScanParts parts;
  if (!CollectPivotScanParts(node, for_sql, emit_expr, emit_agg, &parts)) {
    return "";
  }

  // Touch column_list for `CheckFieldsAccessed`; the lowering is
  // driven by group_by_list + pivot_column_list rather than by
  // `column_list` directly.
  (void)node->column_list_size();

  return BuildPivotSelectSql(inner, parts);
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

  static constexpr char kRowOrdCol[] = "__bq_unpivot_rn";

  // Stamp each input row with a stable ordinal so the final ORDER BY
  // matches BigQuery's row-major UNPIVOT expansion even when DuckDB
  // returns the inner scan in an arbitrary order.
  inner = absl::StrCat("SELECT *, row_number() OVER () AS ",
                       internal::QuoteIdent(kRowOrdCol),
                       " FROM (",
                       inner,
                       ")");

  const EmitExprFn emit_expr = [this](const ::googlesql::ResolvedExpr* expr) {
    return EmitExpr(expr);
  };
  const EmitLiteralFn emit_literal =
      [this](const ::googlesql::ResolvedLiteral* lit) {
        return EmitLiteral(lit);
      };

  UnpivotScanParts parts;
  if (!CollectUnpivotScanParts(node, emit_expr, emit_literal, &parts)) {
    return "";
  }

  std::string sql = BuildUnpivotSelectSql(inner, node, parts);

  // Touch column_list so `CheckFieldsAccessed` does not flag a miss.
  (void)node->column_list_size();
  return sql;
}

}  // namespace transpiler
}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
