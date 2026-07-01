#include <functional>
#include <string>
#include <utility>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "absl/container/flat_hash_set.h"
#include "absl/strings/ascii.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"
#include "absl/strings/string_view.h"
#include "backend/engine/duckdb/transpiler/transpiler.h"
#include "backend/engine/duckdb/transpiler/transpiler_internal.h"
#include "googlesql/public/catalog.h"
#include "googlesql/public/function.h"
#include "googlesql/public/sql_function.h"
#include "googlesql/public/templated_sql_function.h"
#include "googlesql/resolved_ast/resolved_ast.h"
#include "googlesql/resolved_ast/resolved_node.h"
#include "googlesql/resolved_ast/resolved_node_kind.pb.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace duckdb {
namespace transpiler {

namespace {

using EmitExprFn = std::function<std::string(const ::googlesql::ResolvedExpr*)>;
using EmitAggregateFn = std::function<std::string(
    const ::googlesql::ResolvedAggregateFunctionCall*)>;

std::string EmitGroupingSetMultiColumn(
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

std::string EmitGroupingSetEntry(
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

const ::googlesql::ResolvedAggregateFunctionCall* AsAggregateFunctionCall(
    const ::googlesql::ResolvedComputedColumnBase* ac) {
  if (ac == nullptr || ac->expr() == nullptr ||
      ac->expr()->node_kind() !=
          ::googlesql::RESOLVED_AGGREGATE_FUNCTION_CALL) {
    return nullptr;
  }
  return ac->expr()->GetAs<::googlesql::ResolvedAggregateFunctionCall>();
}

std::string TryEmitArrayAggDistinctUnnestScan(
    const ::googlesql::ResolvedAggregateScan* node,
    const EmitExprFn& emit_expr) {
  if (node == nullptr || node->group_by_list_size() != 0 ||
      node->aggregate_list_size() != 1 || node->grouping_set_list_size() != 0 ||
      node->grouping_call_list_size() != 0) {
    return "";
  }
  const auto* agg = AsAggregateFunctionCall(node->aggregate_list(0));
  const ::googlesql::ResolvedScan* input = node->input_scan();
  const auto* arr_scan =
      input != nullptr && input->node_kind() == ::googlesql::RESOLVED_ARRAY_SCAN
          ? input->GetAs<::googlesql::ResolvedArrayScan>()
          : nullptr;
  if (agg == nullptr || agg->function() == nullptr || !agg->distinct() ||
      agg->argument_list_size() != 1 ||
      absl::AsciiStrToLower(agg->function()->Name()) != "array_agg" ||
      arr_scan == nullptr || arr_scan->array_expr_list_size() != 1 ||
      arr_scan->element_column_list_size() != 1 ||
      arr_scan->array_offset_column() != nullptr ||
      arr_scan->join_expr() != nullptr || arr_scan->is_outer() ||
      arr_scan->array_zip_mode() != nullptr ||
      (arr_scan->input_scan() != nullptr &&
       arr_scan->input_scan()->node_kind() !=
           ::googlesql::RESOLVED_SINGLE_ROW_SCAN)) {
    return "";
  }
  std::string arr = emit_expr(arr_scan->array_expr_list(0));
  if (arr.empty()) return "";
  const std::string col = arr_scan->element_column_list(0).name();
  const std::string quoted_col = internal::QuoteIdent(col);
  return absl::StrCat(
      "SELECT (SELECT list(",
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
      internal::QuoteIdent(node->aggregate_list(0)->column().name()),
      " FROM (SELECT 1)");
}

std::string TryEmitStringAggDistinctDedupeScan(
    const ::googlesql::ResolvedAggregateScan* node,
    absl::string_view input,
    const EmitExprFn& emit_expr) {
  if (node->group_by_list_size() != 0 || node->aggregate_list_size() != 1 ||
      node->grouping_call_list_size() != 0) {
    return "";
  }
  const auto* agg = AsAggregateFunctionCall(node->aggregate_list(0));
  if (agg == nullptr || agg->function() == nullptr ||
      internal::ResolveFunctionName(agg->function()) != "string_agg" ||
      !agg->distinct() || agg->order_by_item_list_size() != 0 ||
      agg->limit() == nullptr || agg->argument_list_size() < 1) {
    return "";
  }
  std::string val = emit_expr(agg->argument_list(0));
  if (val.empty()) return "";
  std::string delim = "','";
  if (agg->argument_list_size() >= 2) {
    delim = emit_expr(agg->argument_list(1));
    if (delim.empty()) return "";
  }
  return absl::StrCat(
      "SELECT array_to_string(list(",
      val,
      " ORDER BY ",
      internal::QuoteIdent(internal::kBqInputRnCol),
      " ASC), ",
      delim,
      ") AS ",
      internal::QuoteIdent(node->aggregate_list(0)->column().name()),
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

struct AggregateScanParts {
  std::vector<std::string> projections{};
  std::vector<std::string> group_by_exprs{};
  absl::flat_hash_map<int, std::string> group_by_id_to_name{};
};

bool BuildAggregateScanParts(const ::googlesql::ResolvedAggregateScan* node,
                             const EmitExprFn& emit_expr,
                             const EmitAggregateFn& emit_agg,
                             AggregateScanParts* out) {
  out->projections.reserve(node->group_by_list_size() +
                           node->aggregate_list_size() +
                           node->grouping_call_list_size());
  out->group_by_exprs.reserve(node->group_by_list_size());
  out->group_by_id_to_name.reserve(node->group_by_list_size());

  for (int i = 0; i < node->group_by_list_size(); ++i) {
    const ::googlesql::ResolvedComputedColumn* gc = node->group_by_list(i);
    if (gc == nullptr) return false;
    std::string expr = emit_expr(gc->expr());
    if (expr.empty()) return false;
    std::string col_name(gc->column().name());
    std::string quoted_out = internal::QuoteIdent(col_name);
    out->projections.push_back(
        expr == quoted_out ? expr : absl::StrCat(expr, " AS ", quoted_out));
    out->group_by_exprs.push_back(expr);
    out->group_by_id_to_name[gc->column().column_id()] = std::move(col_name);
  }

  for (int i = 0; i < node->aggregate_list_size(); ++i) {
    const ::googlesql::ResolvedComputedColumnBase* ac = node->aggregate_list(i);
    if (ac == nullptr) return false;
    const auto* agg_call = AsAggregateFunctionCall(ac);
    if (agg_call == nullptr) return false;
    std::string fn = emit_agg(agg_call);
    if (fn.empty()) return false;
    out->projections.push_back(
        absl::StrCat(fn, " AS ", internal::QuoteIdent(ac->column().name())));
  }

  for (int i = 0; i < node->grouping_call_list_size(); ++i) {
    const ::googlesql::ResolvedGroupingCall* gc = node->grouping_call_list(i);
    if (gc == nullptr || gc->group_by_column() == nullptr) return false;
    auto it = out->group_by_id_to_name.find(
        gc->group_by_column()->column().column_id());
    if (it == out->group_by_id_to_name.end()) return false;
    out->projections.push_back(
        absl::StrCat("GROUPING(",
                     internal::QuoteIdent(it->second),
                     ") AS ",
                     internal::QuoteIdent(gc->output_column().name())));
  }
  return true;
}

bool AppendAggregateGroupByClause(
    const ::googlesql::ResolvedAggregateScan* node,
    const AggregateScanParts& parts,
    std::string* sql) {
  if (node->grouping_set_list_size() == 0) {
    if (!parts.group_by_exprs.empty()) {
      absl::StrAppend(
          sql, " GROUP BY ", absl::StrJoin(parts.group_by_exprs, ", "));
    }
    return true;
  }
  (void)node->rollup_column_list_size();
  std::vector<std::string> entries;
  entries.reserve(node->grouping_set_list_size());
  for (int i = 0; i < node->grouping_set_list_size(); ++i) {
    std::string entry = EmitGroupingSetEntry(node->grouping_set_list(i),
                                             parts.group_by_id_to_name);
    if (entry.empty()) return false;
    entries.push_back(std::move(entry));
  }
  absl::StrAppend(
      sql, " GROUP BY GROUPING SETS (", absl::StrJoin(entries, ", "), ")");
  if (!parts.group_by_exprs.empty()) {
    std::vector<std::string> rollup_order;
    rollup_order.reserve(parts.group_by_exprs.size() * 2);
    for (const std::string& expr : parts.group_by_exprs) {
      rollup_order.push_back(absl::StrCat("GROUPING(", expr, ") DESC"));
      rollup_order.push_back(absl::StrCat(expr, " NULLS FIRST"));
    }
    absl::StrAppend(sql, " ORDER BY ", absl::StrJoin(rollup_order, ", "));
  }
  return true;
}

void FinalizeAggregateScanState(const ::googlesql::ResolvedAggregateScan* node,
                                const std::vector<std::string>& projections,
                                std::vector<std::string>* output_order_items,
                                std::vector<int>* output_order_column_ids,
                                bool* input_rn_ordering,
                                bool* output_includes_input_rn,
                                bool* join_output_columns_use_id_aliases) {
  if (projections.empty()) return;
  absl::flat_hash_set<std::string> projected;
  projected.reserve(projections.size());
  for (int i = 0; i < node->group_by_list_size(); ++i) {
    const ::googlesql::ResolvedComputedColumn* gc = node->group_by_list(i);
    if (gc == nullptr) continue;
    projected.insert(internal::QuoteIdent(gc->column().name()));
  }
  for (int i = 0; i < node->aggregate_list_size(); ++i) {
    const ::googlesql::ResolvedComputedColumnBase* ac = node->aggregate_list(i);
    if (ac == nullptr) continue;
    projected.insert(internal::QuoteIdent(ac->column().name()));
  }
  for (int i = 0; i < node->grouping_call_list_size(); ++i) {
    const ::googlesql::ResolvedGroupingCall* gc = node->grouping_call_list(i);
    if (gc == nullptr) continue;
    projected.insert(internal::QuoteIdent(gc->output_column().name()));
  }
  internal::FilterOutputOrderItemsByProjectedColumns(
      output_order_items, output_order_column_ids, projected);
  if (!projected.contains(internal::QuoteIdent(internal::kBqInputRnCol))) {
    *input_rn_ordering = false;
    *output_includes_input_rn = false;
  }
  *join_output_columns_use_id_aliases = false;
}

}  // namespace

std::string Transpiler::EmitAggregateScan(
    const ::googlesql::ResolvedAggregateScan* node) {
  const EmitExprFn emit_expr = [this](const ::googlesql::ResolvedExpr* expr) {
    return EmitExpr(expr);
  };
  const EmitAggregateFn emit_agg =
      [this](const ::googlesql::ResolvedAggregateFunctionCall* agg) {
        return EmitAggregateFunctionCall(agg);
      };

  if (std::string special = TryEmitArrayAggDistinctUnnestScan(node, emit_expr);
      !special.empty()) {
    join_output_uses_id_aliases_ = false;
    return special;
  }
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

  if (std::string special =
          TryEmitStringAggDistinctDedupeScan(node, input, emit_expr);
      !special.empty()) {
    join_output_uses_id_aliases_ = false;
    return special;
  }

  AggregateScanParts parts;
  if (!BuildAggregateScanParts(node, emit_expr, emit_agg, &parts)) return "";

  std::string select_list =
      parts.projections.empty() ? "*" : absl::StrJoin(parts.projections, ", ");
  std::string sql = absl::StrCat("SELECT ", select_list, " FROM (", input, ")");

  if (!AppendAggregateGroupByClause(node, parts, &sql)) return "";

  FinalizeAggregateScanState(node,
                             parts.projections,
                             &output_order_items_,
                             &output_order_column_ids_,
                             &input_rn_ordering_,
                             &output_includes_input_rn_,
                             &join_output_columns_use_id_aliases_);
  join_output_uses_id_aliases_ = false;
  return sql;
}

}  // namespace transpiler
}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
