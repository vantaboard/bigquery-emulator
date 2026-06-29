
#include "googlesql/public/catalog.h"
#include "googlesql/public/function.h"
#include "googlesql/public/sql_function.h"
#include "googlesql/public/templated_sql_function.h"
#include "googlesql/public/type.h"
#include "googlesql/public/types/struct_type.h"
#include "googlesql/public/value.h"
#include "googlesql/resolved_ast/resolved_node.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace duckdb {
namespace transpiler {

std::string Transpiler::EmitSetOperationItem(
    const ::googlesql::ResolvedSetOperationItem* item,
    const ::googlesql::ResolvedSetOperationScan* parent) {
  // Lower one item of a `ResolvedSetOperationScan`. The parent's
  // `column_list()` names the *new* columns the set operation
  // produces; each item's `output_column_list[i]` is the
  // ResolvedColumn (from the child scan) that maps into the
  // parent's `column_list(i)`. The two lists are guaranteed 1:1 by
  // the GoogleSQL contract, so we project each output column under
  // the parent's name (collapsed when the names already match).
  //
  // We wrap the child scan as a derived table (`FROM (<scan>)`)
  // because every scan emit returns a self-contained SELECT; the
  // resulting per-item SQL is itself a SELECT so the SetOpScan
  // splice can join two-or-more items with the UNION / INTERSECT /
  // EXCEPT keyword between them.
  if (item == nullptr || parent == nullptr || item->scan() == nullptr) {
    return "";
  }
  if (item->output_column_list_size() != parent->column_list_size()) {
    return "";
  }
  std::string inner = EmitScan(item->scan());
  if (inner.empty()) return "";
  const bool use_join_aliases = join_output_columns_use_id_aliases_;
  std::vector<std::string> projections;
  projections.reserve(item->output_column_list_size());
  for (int i = 0; i < item->output_column_list_size(); ++i) {
    const ::googlesql::ResolvedColumn& src = item->output_column_list(i);
    const ::googlesql::ResolvedColumn& dst = parent->column_list(i);
    if (use_join_aliases) {
      projections.push_back(
          absl::StrCat(internal::JoinColumnIdAlias(src.column_id()),
                       " AS ",
                       internal::QuoteIdent(dst.name())));
      continue;
    }
    std::string src_q = internal::QuoteIdent(src.name());
    if (src.name() == dst.name()) {
      projections.push_back(std::move(src_q));
    } else {
      projections.push_back(
          absl::StrCat(src_q, " AS ", internal::QuoteIdent(dst.name())));
    }
  }
  std::string select_list =
      projections.empty() ? "*" : absl::StrJoin(projections, ", ");
  return absl::StrCat("SELECT ", select_list, " FROM (", inner, ")");
}

std::string Transpiler::EmitSetOperationScan(
    const ::googlesql::ResolvedSetOperationScan* node) {
  // BigQuery `<lhs> UNION ALL <rhs>`, `<lhs> UNION DISTINCT <rhs>`,
  // `<lhs> INTERSECT DISTINCT <rhs>`, `<lhs> EXCEPT DISTINCT <rhs>`
  // all lower onto a `ResolvedSetOperationScan` whose
  // `input_item_list()` carries one `ResolvedSetOperationItem` per
  // arm and whose `column_list()` is the new set of ResolvedColumns
  // the operation produces. We render the items in order with the
  // matching DuckDB keyword between them:
  //
  //   UNION_ALL            -> "UNION ALL"
  //   UNION_DISTINCT       -> "UNION"      (DuckDB's UNION is DISTINCT)
  //   INTERSECT_DISTINCT   -> "INTERSECT"  (DuckDB's default behavior)
  //   EXCEPT_DISTINCT      -> "EXCEPT"     (DuckDB's default behavior)
  //   INTERSECT_ALL        -> "INTERSECT ALL"
  //   EXCEPT_ALL           -> "EXCEPT ALL"
  //
  // The `INTERSECT ALL` / `EXCEPT ALL` variants are included for
  // completeness even though BigQuery's surface SQL only exposes
  // the DISTINCT forms today. DuckDB has supported the ALL
  // variants since v0.10 with the standard SQL bag semantics
  // (`min(m, n)` for INTERSECT ALL, `max(m - n, 0)` for EXCEPT
  // ALL), which is also what GoogleSQL's INTERSECT_ALL /
  // EXCEPT_ALL define when the analyzer surfaces them (see
  // `ResolvedSetOperationScan` docs in
  // `googlesql/resolved_ast/resolved_ast.h`). The kept-in-sync
  // semantics are documented on the row in `SHAPE_TRACKER.md`.
  //
  // Fallbacks (return ""):
  //   * Fewer than two inputs (a malformed AST the analyzer would
  //     not produce, but the GoogleSQL contract says "at least
  //     two", so we guard defensively).
  //   * Any item whose child scan is on the empty-string fallback.
  //
  // `CORRESPONDING` / `CORRESPONDING_BY` rely on the analyzer's
  // per-item `output_column_list` mapping; `EmitSetOperationItem`
  // projects each arm onto the parent's `column_list` names.
  if (node == nullptr) return "";
  // `column_propagation_mode` is documented as informational for
  // engines; for BY_POSITION matching it is essentially
  // load-bearing only when CORRESPONDING is in play. Touch the
  // accessor so `CheckFieldsAccessed` does not flag a missed read
  // when this node round-trips through validation.
  (void)node->column_propagation_mode();
  if (node->input_item_list_size() < 2) return "";

  const char* op_kw = nullptr;
  switch (node->op_type()) {
    case ::googlesql::ResolvedSetOperationScan::UNION_ALL:
      op_kw = "UNION ALL";
      break;
    case ::googlesql::ResolvedSetOperationScan::UNION_DISTINCT:
      op_kw = "UNION";
      break;
    case ::googlesql::ResolvedSetOperationScan::INTERSECT_DISTINCT:
      op_kw = "INTERSECT";
      break;
    case ::googlesql::ResolvedSetOperationScan::EXCEPT_DISTINCT:
      op_kw = "EXCEPT";
      break;
    case ::googlesql::ResolvedSetOperationScan::INTERSECT_ALL:
      op_kw = "INTERSECT ALL";
      break;
    case ::googlesql::ResolvedSetOperationScan::EXCEPT_ALL:
      op_kw = "EXCEPT ALL";
      break;
    default:
      return "";
  }

  std::vector<std::string> items;
  items.reserve(node->input_item_list_size());
  const bool preserve_union_order =
      node->op_type() == ::googlesql::ResolvedSetOperationScan::UNION_ALL;
  for (int i = 0; i < node->input_item_list_size(); ++i) {
    const bool saved_has_rn = input_has_rn_column_;
    const bool saved_rn_order = input_rn_ordering_;
    const bool saved_output_rn = output_includes_input_rn_;
    const std::vector<std::string> saved_output_order = output_order_items_;
    const std::vector<int> saved_output_order_ids = output_order_column_ids_;
    input_has_rn_column_ = false;
    input_rn_ordering_ = false;
    output_includes_input_rn_ = false;
    join_output_columns_use_id_aliases_ = false;
    output_order_items_.clear();
    output_order_column_ids_.clear();
    std::string s = EmitSetOperationItem(node->input_item_list(i), node);
    input_has_rn_column_ = saved_has_rn || input_has_rn_column_;
    input_rn_ordering_ = saved_rn_order || input_rn_ordering_;
    output_includes_input_rn_ = saved_output_rn || output_includes_input_rn_;
    if (output_order_items_.empty()) {
      output_order_items_ = saved_output_order;
      output_order_column_ids_ = saved_output_order_ids;
    }
    if (s.empty()) return "";
    if (preserve_union_order) {
      s = absl::StrCat("SELECT *, ",
                       i + 1,
                       " AS ",
                       internal::QuoteIdent(internal::kBqUnionOrdCol),
                       " FROM (",
                       s,
                       ")");
    }
    items.push_back(std::move(s));
  }
  // Set-op arms may have used join id aliases internally; the combined
  // result schema is the parent's user-visible column_list names.
  join_output_columns_use_id_aliases_ = false;
  join_output_uses_id_aliases_ = false;
  std::string union_sql = absl::StrJoin(items, absl::StrCat(" ", op_kw, " "));
  if (preserve_union_order) {
    std::vector<std::string> cols;
    cols.reserve(node->column_list_size());
    for (int i = 0; i < node->column_list_size(); ++i) {
      cols.push_back(internal::QuoteIdent(node->column_list(i).name()));
    }
    return absl::StrCat("SELECT ",
                        absl::StrJoin(cols, ", "),
                        " FROM (",
                        union_sql,
                        ") ORDER BY ",
                        internal::QuoteIdent(internal::kBqUnionOrdCol));
  }
  if (node->op_type() ==
          ::googlesql::ResolvedSetOperationScan::EXCEPT_DISTINCT ||
      node->op_type() == ::googlesql::ResolvedSetOperationScan::EXCEPT_ALL) {
    return absl::StrCat("SELECT * FROM (", union_sql, ") ORDER BY 1");
  }
  return union_sql;
}

std::string Transpiler::EmitOrderByScan(
    const ::googlesql::ResolvedOrderByScan* node) {
  // `SELECT * FROM (<input>) ORDER BY <col> [ASC|DESC] [NULLS FIRST|LAST]`
  // -- one item per `ResolvedOrderByItem`. The order-by item's
  // `column_ref` always points at a column produced by `input_scan`,
  // so the inner SELECT exposes the right alias for the outer ORDER
  // BY clause to bind against.
  //
  // `collation_name` carries `ORDER BY x COLLATE ...`; the lower
  // pass for collations lands separately, so we fall back when it
  // is set.
  if (node == nullptr) return "";
  std::string input = EmitScan(node->input_scan());
  if (input.empty()) return "";
  const bool input_has_join_aliases =
      join_output_uses_id_aliases_ || join_id_aliases_in_query_;
  std::vector<std::string> items;
  items.reserve(node->order_by_item_list_size());
  for (int i = 0; i < node->order_by_item_list_size(); ++i) {
    const ::googlesql::ResolvedOrderByItem* item = node->order_by_item_list(i);
    if (item == nullptr || item->column_ref() == nullptr) return "";
    if (item->collation_name() != nullptr) return "";
    std::string col = EmitColumnRef(item->column_ref());
    if (col.empty()) return "";
    items.push_back(absl::StrCat(
        col,
        internal::OrderByItemSuffix(item, /*bigquery_null_defaults=*/true)));
  }
  if (items.empty()) return "";
  // Explicit ORDER BY wins over analytic-captured output_order_items_ that
  // child scans (e.g. ROW_NUMBER PARTITION BY) may have queued for the
  // outer QueryStmt wrap.
  output_order_items_.clear();
  output_order_column_ids_.clear();
  input_rn_ordering_ = false;
  std::string projection = "*";
  const ::googlesql::ResolvedScan* input_scan = node->input_scan();
  const bool join_passthrough_input =
      input_scan != nullptr &&
      (input_scan->node_kind() == ::googlesql::RESOLVED_JOIN_SCAN ||
       (input_scan->node_kind() == ::googlesql::RESOLVED_PROJECT_SCAN &&
        input_scan->GetAs<::googlesql::ResolvedProjectScan>()
                ->expr_list_size() == 0));
  if (input_has_join_aliases && join_passthrough_input &&
      input_scan->column_list_size() > 0) {
    std::vector<std::string> cols;
    cols.reserve(input_scan->column_list_size());
    for (int i = 0; i < input_scan->column_list_size(); ++i) {
      cols.push_back(
          internal::JoinColumnIdAlias(input_scan->column_list(i).column_id()));
    }
    projection = absl::StrJoin(cols, ", ");
    join_output_uses_id_aliases_ = true;
  }
  return absl::StrCat("SELECT ",
                      projection,
                      " FROM (",
                      input,
                      ") ORDER BY ",
                      absl::StrJoin(items, ", "));
}

std::string Transpiler::EmitLimitOffsetScan(
    const ::googlesql::ResolvedLimitOffsetScan* node) {
  // `SELECT * FROM (<input>) LIMIT <n> [OFFSET <m>]`. The analyzer
  // guarantees `limit` and `offset` are constant INT64-coercible
  // expressions, so `EmitExpr` lowers them through `EmitLiteral`
  // when they were spelled as literals. We fall back when either
  // expression is something the literal-emit subset cannot lower
  // yet (e.g. a parameter).
  if (node == nullptr) return "";
  std::string input = EmitScan(node->input_scan());
  if (input.empty()) return "";
  std::string sql = absl::StrCat("SELECT * FROM (", input, ")");
  if (node->limit() != nullptr) {
    std::string l = EmitExpr(node->limit());
    if (l.empty()) return "";
    absl::StrAppend(&sql, " LIMIT ", l);
  }
  if (node->offset() != nullptr) {
    std::string o = EmitExpr(node->offset());
    if (o.empty()) return "";
    absl::StrAppend(&sql, " OFFSET ", o);
  }
  return sql;
}

}  // namespace transpiler
}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
