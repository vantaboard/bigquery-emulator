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

// Emit one `ResolvedGroupingSetMultiColumn` (a single grouping set
// item used INSIDE a ROLLUP / CUBE list). The single-column case
// emits the bare column name (DuckDB accepts both `ROLLUP(a, b)` and
// `ROLLUP((a), (b))`); the multi-column case wraps in parentheses
// so DuckDB sees one tuple per ROLLUP step. The lookup walks
// `group_by_id_to_name` because the `ResolvedColumnRef`s inside a
// grouping set point back to the `group_by_list` columns by id, not
// by name -- the analyzer rewrites the column ids when GROUPING SETS
// is in play, so reading `ref->column().name()` directly produces
// stale names DuckDB cannot resolve against the SELECT-list aliases.
static std::string EmitGroupingSetMultiColumn(
    const ::googlesql::ResolvedGroupingSetMultiColumn* mc,
    const absl::flat_hash_map<int, std::string>& group_by_id_to_name) {
  if (mc == nullptr) return "";
  std::vector<std::string> cols;
  cols.reserve(mc->column_list_size());
  for (int i = 0; i < mc->column_list_size(); ++i) {
    const ::googlesql::ResolvedColumnRef* ref = mc->column_list(i);
    if (ref == nullptr) return "";
    auto it = group_by_id_to_name.find(ref->column().column_id());
    if (it == group_by_id_to_name.end()) return "";
    cols.push_back(internal::QuoteIdent(it->second));
  }
  if (cols.empty()) return "()";
  if (cols.size() == 1) return cols[0];
  return absl::StrCat("(", absl::StrJoin(cols, ", "), ")");
}

// Render one `grouping_set_list` entry. The list is heterogeneous:
// each entry is a `ResolvedGroupingSet` (an explicit tuple),
// `ResolvedRollup` (a list of multi-columns), or `ResolvedCube`
// (same shape as Rollup). DuckDB's `GROUP BY GROUPING SETS (...)`
// accepts the three forms verbatim, so we lower each entry directly
// rather than expanding ROLLUP / CUBE on our side: keeping the
// keyword preserves DuckDB's faster code path for the symmetric
// rollup/cube hash tables.
static std::string EmitGroupingSetEntry(
    const ::googlesql::ResolvedGroupingSetBase* entry,
    const absl::flat_hash_map<int, std::string>& group_by_id_to_name) {
  if (entry == nullptr) return "";
  switch (entry->node_kind()) {
    case ::googlesql::RESOLVED_GROUPING_SET: {
      const auto* gs = entry->GetAs<::googlesql::ResolvedGroupingSet>();
      std::vector<std::string> cols;
      cols.reserve(gs->group_by_column_list_size());
      for (int i = 0; i < gs->group_by_column_list_size(); ++i) {
        const ::googlesql::ResolvedColumnRef* ref = gs->group_by_column_list(i);
        if (ref == nullptr) return "";
        auto it = group_by_id_to_name.find(ref->column().column_id());
        if (it == group_by_id_to_name.end()) return "";
        cols.push_back(internal::QuoteIdent(it->second));
      }
      return absl::StrCat("(", absl::StrJoin(cols, ", "), ")");
    }
    case ::googlesql::RESOLVED_ROLLUP: {
      const auto* r = entry->GetAs<::googlesql::ResolvedRollup>();
      std::vector<std::string> items;
      items.reserve(r->rollup_column_list_size());
      for (int i = 0; i < r->rollup_column_list_size(); ++i) {
        std::string item = EmitGroupingSetMultiColumn(r->rollup_column_list(i),
                                                      group_by_id_to_name);
        if (item.empty()) return "";
        items.push_back(std::move(item));
      }
      return absl::StrCat("ROLLUP (", absl::StrJoin(items, ", "), ")");
    }
    case ::googlesql::RESOLVED_CUBE: {
      const auto* c = entry->GetAs<::googlesql::ResolvedCube>();
      std::vector<std::string> items;
      items.reserve(c->cube_column_list_size());
      for (int i = 0; i < c->cube_column_list_size(); ++i) {
        std::string item = EmitGroupingSetMultiColumn(c->cube_column_list(i),
                                                      group_by_id_to_name);
        if (item.empty()) return "";
        items.push_back(std::move(item));
      }
      return absl::StrCat("CUBE (", absl::StrJoin(items, ", "), ")");
    }
    default:
      return "";
  }
}

std::string Transpiler::EmitAggregateScan(
    const ::googlesql::ResolvedAggregateScan* node) {
  // BigQuery `ARRAY_AGG(DISTINCT x)` preserves first-seen order from the
  // input sequence. DuckDB's `list(DISTINCT x)` / `array_agg(DISTINCT x)`
  // do not, so for `SELECT ARRAY_AGG(DISTINCT x) FROM UNNEST(<literal>)`
  // we lower via WITH ORDINALITY and a grouped subquery.
  if (node != nullptr && node->group_by_list_size() == 0 &&
      node->aggregate_list_size() == 1 && node->grouping_set_list_size() == 0 &&
      node->grouping_call_list_size() == 0) {
    const ::googlesql::ResolvedComputedColumnBase* ac = node->aggregate_list(0);
    const auto* agg =
        ac != nullptr && ac->expr() != nullptr &&
                ac->expr()->node_kind() ==
                    ::googlesql::RESOLVED_AGGREGATE_FUNCTION_CALL
            ? ac->expr()->GetAs<::googlesql::ResolvedAggregateFunctionCall>()
            : nullptr;
    const ::googlesql::ResolvedScan* input = node->input_scan();
    const auto* arr_scan =
        input != nullptr &&
                input->node_kind() == ::googlesql::RESOLVED_ARRAY_SCAN
            ? input->GetAs<::googlesql::ResolvedArrayScan>()
            : nullptr;
    if (agg != nullptr && agg->function() != nullptr && agg->distinct() &&
        agg->argument_list_size() == 1 &&
        absl::AsciiStrToLower(agg->function()->Name()) == "array_agg" &&
        arr_scan != nullptr && arr_scan->array_expr_list_size() == 1 &&
        arr_scan->element_column_list_size() == 1 &&
        arr_scan->array_offset_column() == nullptr &&
        arr_scan->join_expr() == nullptr && !arr_scan->is_outer() &&
        arr_scan->array_zip_mode() == nullptr &&
        (arr_scan->input_scan() == nullptr ||
         arr_scan->input_scan()->node_kind() ==
             ::googlesql::RESOLVED_SINGLE_ROW_SCAN)) {
      std::string arr = EmitExpr(arr_scan->array_expr_list(0));
      if (!arr.empty()) {
        const absl::string_view col = arr_scan->element_column_list(0).name();
        const std::string quoted_col = internal::QuoteIdent(col);
        join_output_uses_id_aliases_ = false;
        return absl::StrCat("SELECT (SELECT list(",
                            quoted_col,
                            " ORDER BY min_ord) FROM (SELECT ",
                            quoted_col,
                            ", min(ord) AS min_ord FROM (SELECT ",
                            quoted_col,
                            ", ord FROM unnest(",
                            arr,
                            ") WITH ORDINALITY AS __bq_unnest__(",
                            quoted_col,
                            ", ord)) GROUP BY ",
                            quoted_col,
                            ")) AS ",
                            internal::QuoteIdent(ac->column().name()),
                            " FROM (SELECT 1)");
      }
    }
  }
  // Emit `SELECT <grouping-cols>, <aggregates> FROM (<input>) GROUP BY ...`
  // where each grouping column repeats its underlying expression in
  // GROUP BY (rather than referencing the SELECT-list alias), so the
  // emitted SQL composes safely no matter how DuckDB resolves alias
  // visibility in the future.
  //
  // GROUPING SETS / ROLLUP / CUBE: when `grouping_set_list` is
  // non-empty, the GROUP BY clause becomes
  // `GROUP BY GROUPING SETS (<entry>, ...)`. Each entry is one of
  //   * a bare tuple `(a, b)` (`ResolvedGroupingSet`)
  //   * `ROLLUP (a, b)` (`ResolvedRollup`)
  //   * `CUBE (a, b)` (`ResolvedCube`)
  // The grouping-set tuples reference the GROUP BY columns by NAME
  // (DuckDB resolves the SELECT-list alias inside GROUP BY), so we
  // collect a `column_id -> name` map from `group_by_list` first
  // and pass it through to the per-entry emitter.
  //
  // `rollup_column_list` is the analyzer's legacy mirror of the
  // ROLLUP shape. We rely on `grouping_set_list` exclusively for the
  // emit (the modern form), but still touch the accessor below so
  // `CheckFieldsAccessed` does not flag the legacy field.
  //
  // GROUPING(<col>) function calls land in `grouping_call_list`
  // separately from the aggregate list. Each
  // `ResolvedGroupingCall` carries the source group-by column ref
  // plus the output column the GROUPING bit lands on. We project
  // each as `GROUPING("<col>") AS "<output>"` so the SELECT list
  // exposes the bit at the analyzer-chosen output name.
  if (node == nullptr) return "";
  std::string input = EmitScan(node->input_scan());
  if (input.empty()) return "";
  if (internal::AggregateScanNeedsInputRn(node)) {
    if (!input_has_rn_column_) {
      input = absl::StrCat("SELECT *, row_number() OVER () AS ",
                           internal::QuoteIdent(internal::kBqInputRnCol),
                           " FROM (",
                           input,
                           ")");
      input_has_rn_column_ = true;
    }
  }

  // STRING_AGG(DISTINCT x) without ORDER BY: DuckDB cannot ORDER BY
  // __bq_input_rn inside list(DISTINCT ...). Dedupe to first-seen rows,
  // then list-aggregate in input order.
  if (node->group_by_list_size() == 0 && node->aggregate_list_size() == 1 &&
      node->grouping_call_list_size() == 0) {
    const ::googlesql::ResolvedComputedColumnBase* ac = node->aggregate_list(0);
    const auto* agg =
        ac != nullptr && ac->expr() != nullptr &&
                ac->expr()->node_kind() ==
                    ::googlesql::RESOLVED_AGGREGATE_FUNCTION_CALL
            ? ac->expr()->GetAs<::googlesql::ResolvedAggregateFunctionCall>()
            : nullptr;
    if (agg != nullptr && agg->function() != nullptr &&
        internal::ResolveFunctionName(agg->function()) == "string_agg" &&
        agg->distinct() && agg->order_by_item_list_size() == 0 &&
        agg->limit() == nullptr && agg->argument_list_size() >= 1) {
      std::string val = EmitExpr(agg->argument_list(0));
      if (val.empty()) return "";
      std::string delim = "','";
      if (agg->argument_list_size() >= 2) {
        delim = EmitExpr(agg->argument_list(1));
        if (delim.empty()) return "";
      }
      join_output_uses_id_aliases_ = false;
      return absl::StrCat("SELECT array_to_string(list(",
                          val,
                          " ORDER BY ",
                          internal::QuoteIdent(internal::kBqInputRnCol),
                          " ASC), ",
                          delim,
                          ") AS ",
                          internal::QuoteIdent(ac->column().name()),
                          " FROM (SELECT ",
                          val,
                          ", ",
                          internal::QuoteIdent(internal::kBqInputRnCol),
                          " FROM (",
                          input,
                          ") QUALIFY row_number() OVER (PARTITION BY ",
                          val,
                          " ORDER BY ",
                          internal::QuoteIdent(internal::kBqInputRnCol),
                          ") = 1)");
    }
  }

  std::vector<std::string> projections;
  std::vector<std::string> group_by_exprs;
  projections.reserve(node->group_by_list_size() + node->aggregate_list_size() +
                      node->grouping_call_list_size());
  group_by_exprs.reserve(node->group_by_list_size());
  absl::flat_hash_map<int, std::string> group_by_id_to_name;
  group_by_id_to_name.reserve(node->group_by_list_size());

  for (int i = 0; i < node->group_by_list_size(); ++i) {
    const ::googlesql::ResolvedComputedColumn* gc = node->group_by_list(i);
    if (gc == nullptr) return "";
    std::string expr = EmitExpr(gc->expr());
    if (expr.empty()) return "";
    std::string col_name(gc->column().name());
    std::string quoted_out = internal::QuoteIdent(col_name);
    projections.push_back(
        expr == quoted_out ? expr : absl::StrCat(expr, " AS ", quoted_out));
    group_by_exprs.push_back(expr);
    group_by_id_to_name[gc->column().column_id()] = std::move(col_name);
  }

  for (int i = 0; i < node->aggregate_list_size(); ++i) {
    const ::googlesql::ResolvedComputedColumnBase* ac = node->aggregate_list(i);
    if (ac == nullptr) return "";
    const ::googlesql::ResolvedExpr* expr_node = ac->expr();
    if (expr_node == nullptr ||
        expr_node->node_kind() !=
            ::googlesql::RESOLVED_AGGREGATE_FUNCTION_CALL) {
      return "";
    }
    std::string fn = EmitAggregateFunctionCall(
        expr_node->GetAs<::googlesql::ResolvedAggregateFunctionCall>());
    if (fn.empty()) return "";
    projections.push_back(
        absl::StrCat(fn, " AS ", internal::QuoteIdent(ac->column().name())));
  }

  // GROUPING() calls. Each entry pins one bit-mask column.
  for (int i = 0; i < node->grouping_call_list_size(); ++i) {
    const ::googlesql::ResolvedGroupingCall* gc = node->grouping_call_list(i);
    if (gc == nullptr || gc->group_by_column() == nullptr) return "";
    auto it =
        group_by_id_to_name.find(gc->group_by_column()->column().column_id());
    if (it == group_by_id_to_name.end()) return "";
    projections.push_back(
        absl::StrCat("GROUPING(",
                     internal::QuoteIdent(it->second),
                     ") AS ",
                     internal::QuoteIdent(gc->output_column().name())));
  }

  std::string select_list =
      projections.empty() ? "*" : absl::StrJoin(projections, ", ");
  std::string sql = absl::StrCat("SELECT ", select_list, " FROM (", input, ")");

  // GROUP BY clause: GROUPING SETS form when grouping_set_list is
  // non-empty, otherwise the bare `GROUP BY <expr>, ...` list.
  if (node->grouping_set_list_size() > 0) {
    // Touch the legacy `rollup_column_list` accessor so the analyzer's
    // CheckFieldsAccessed does not flag a missed read when ROLLUP is
    // used and the analyzer still populates both fields.
    (void)node->rollup_column_list_size();
    std::vector<std::string> entries;
    entries.reserve(node->grouping_set_list_size());
    for (int i = 0; i < node->grouping_set_list_size(); ++i) {
      std::string entry =
          EmitGroupingSetEntry(node->grouping_set_list(i), group_by_id_to_name);
      if (entry.empty()) return "";
      entries.push_back(std::move(entry));
    }
    absl::StrAppend(
        &sql, " GROUP BY GROUPING SETS (", absl::StrJoin(entries, ", "), ")");
    if (!group_by_exprs.empty()) {
      std::vector<std::string> rollup_order;
      rollup_order.reserve(group_by_exprs.size() * 2);
      for (const std::string& expr : group_by_exprs) {
        rollup_order.push_back(absl::StrCat("GROUPING(", expr, ") DESC"));
        rollup_order.push_back(absl::StrCat(expr, " NULLS FIRST"));
      }
      absl::StrAppend(&sql, " ORDER BY ", absl::StrJoin(rollup_order, ", "));
    }
  } else if (!group_by_exprs.empty()) {
    absl::StrAppend(&sql, " GROUP BY ", absl::StrJoin(group_by_exprs, ", "));
  }
  if (!projections.empty()) {
    absl::flat_hash_set<std::string> projected;
    projected.reserve(projections.size());
    for (int i = 0; i < node->group_by_list_size(); ++i) {
      const ::googlesql::ResolvedComputedColumn* gc = node->group_by_list(i);
      if (gc == nullptr) continue;
      projected.insert(internal::QuoteIdent(gc->column().name()));
    }
    for (int i = 0; i < node->aggregate_list_size(); ++i) {
      const ::googlesql::ResolvedComputedColumnBase* ac =
          node->aggregate_list(i);
      if (ac == nullptr) continue;
      projected.insert(internal::QuoteIdent(ac->column().name()));
    }
    for (int i = 0; i < node->grouping_call_list_size(); ++i) {
      const ::googlesql::ResolvedGroupingCall* gc = node->grouping_call_list(i);
      if (gc == nullptr) continue;
      projected.insert(internal::QuoteIdent(gc->output_column().name()));
    }
    internal::FilterOutputOrderItemsByProjectedColumns(
        &output_order_items_, &output_order_column_ids_, projected);
    if (!projected.contains(internal::QuoteIdent(internal::kBqInputRnCol))) {
      input_rn_ordering_ = false;
      output_includes_input_rn_ = false;
    }
  }
  join_output_uses_id_aliases_ = false;
  return sql;
}

}  // namespace transpiler
}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
