#include "backend/engine/duckdb/transpiler/transpiler.h"

#include <string>
#include <vector>

#include "absl/log/log.h"
#include "absl/strings/ascii.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"
#include "absl/strings/str_replace.h"
#include "absl/strings/string_view.h"
#include "backend/engine/duckdb/transpiler/functions.h"
#include "googlesql/public/catalog.h"
#include "googlesql/public/function.h"
#include "googlesql/public/options.pb.h"
#include "googlesql/public/type.h"
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

namespace {

// Double-quote escape a DuckDB identifier. DuckDB doubles embedded
// `"` characters; we do the same so column / table names with quotes
// or hyphens round-trip safely through the emitted SQL.
std::string QuoteIdent(absl::string_view name) {
  return absl::StrCat("\"", absl::StrReplaceAll(name, {{"\"", "\"\""}}), "\"");
}

// Single-quote escape a DuckDB string literal. DuckDB doubles embedded
// `'` characters; we do the same so BQ string literals with embedded
// apostrophes round-trip safely. Used for both ResolvedLiteral
// strings and for STRUCT field-name keys in `{'k': v}` literals.
std::string QuoteString(absl::string_view text) {
  return absl::StrCat("'", absl::StrReplaceAll(text, {{"'", "''"}}), "'");
}

// Lower a GoogleSQL `Value` into a DuckDB SQL literal expression.
//
// Scalars route through `Value::GetSQLLiteral(PRODUCT_EXTERNAL)`
// because that path already matches DuckDB syntax for INT / FLOAT /
// BOOL / DATE / NUMERIC / DATETIME etc. Strings, arrays, and structs
// each need a bespoke shape:
//
// * Strings: DuckDB reads double-quoted text as an *identifier*, so we
//   emit the single-quoted form (`'hi'`).
// * Arrays: DuckDB's array literal is `[e1, e2, ...]`, same shape as
//   GoogleSQL's `kSQLLiteral` output, but we recurse so nested
//   STRINGs / STRUCTs get the DuckDB-flavored quoting above instead
//   of GoogleSQL's `"..."` and `(...)` shapes.
// * Structs: DuckDB struct literals are `{'k1': v1, 'k2': v2, ...}`
//   keyed by name. BQ STRUCT field order is positional (the type
//   carries the names), so we walk the StructType for the keys in
//   parallel with the value list. Anonymous fields (empty name) have
//   no DuckDB analog and force the caller back to the reference-impl
//   fallback by returning the empty string.
//
// Returns the empty string when any element / field cannot be lowered;
// callers propagate that up so the engine fallback fires per the
// per-shape disposition in SHAPE_TRACKER.md.
std::string EmitValueLiteral(const ::googlesql::Value& v) {
  if (v.is_null()) return "NULL";
  const ::googlesql::Type* type = v.type();
  if (type == nullptr) return "";
  switch (type->kind()) {
    case ::googlesql::TYPE_STRING:
      return QuoteString(v.string_value());
    case ::googlesql::TYPE_ARRAY: {
      std::vector<std::string> elems;
      elems.reserve(v.num_elements());
      for (int i = 0; i < v.num_elements(); ++i) {
        std::string e = EmitValueLiteral(v.element(i));
        if (e.empty()) return "";
        elems.push_back(std::move(e));
      }
      return absl::StrCat("[", absl::StrJoin(elems, ", "), "]");
    }
    case ::googlesql::TYPE_STRUCT: {
      const ::googlesql::StructType* st = type->AsStruct();
      if (st == nullptr || st->num_fields() != v.num_fields()) return "";
      std::vector<std::string> kvs;
      kvs.reserve(v.num_fields());
      for (int i = 0; i < v.num_fields(); ++i) {
        const ::googlesql::StructField& f = st->field(i);
        if (f.name.empty()) return "";
        std::string fv = EmitValueLiteral(v.field(i));
        if (fv.empty()) return "";
        kvs.push_back(absl::StrCat(QuoteString(f.name), ": ", fv));
      }
      return absl::StrCat("{", absl::StrJoin(kvs, ", "), "}");
    }
    default:
      return v.GetSQLLiteral(::googlesql::PRODUCT_EXTERNAL);
  }
}

}  // namespace

Transpiler::Transpiler() = default;
Transpiler::~Transpiler() = default;

std::string Transpiler::Transpile(const ::googlesql::ResolvedNode* node) {
  // Single-entry dispatch: we walk by `node_kind()` so the per-shape
  // `Emit*` methods can recurse through `EmitExpr` / `EmitScan` /
  // `Transpile` without each one knowing the full node hierarchy.
  // Returning "" is the contract `DuckDBEngine::ExecuteQuery` reads
  // as "not yet supported" -- the engine falls back to the
  // reference-impl evaluator through the disposition policy.
  if (node == nullptr) return "";
  switch (node->node_kind()) {
    case ::googlesql::RESOLVED_QUERY_STMT:
      return EmitQueryStmt(node->GetAs<::googlesql::ResolvedQueryStmt>());
    case ::googlesql::RESOLVED_TABLE_SCAN:
    case ::googlesql::RESOLVED_FILTER_SCAN:
    case ::googlesql::RESOLVED_PROJECT_SCAN:
    case ::googlesql::RESOLVED_SINGLE_ROW_SCAN:
    case ::googlesql::RESOLVED_JOIN_SCAN:
    case ::googlesql::RESOLVED_ARRAY_SCAN:
    case ::googlesql::RESOLVED_AGGREGATE_SCAN:
    case ::googlesql::RESOLVED_ORDER_BY_SCAN:
    case ::googlesql::RESOLVED_LIMIT_OFFSET_SCAN:
    case ::googlesql::RESOLVED_ANALYTIC_SCAN:
      return EmitScan(node->GetAs<::googlesql::ResolvedScan>());
    case ::googlesql::RESOLVED_LITERAL:
    case ::googlesql::RESOLVED_COLUMN_REF:
    case ::googlesql::RESOLVED_FUNCTION_CALL:
    case ::googlesql::RESOLVED_MAKE_STRUCT:
    case ::googlesql::RESOLVED_GET_STRUCT_FIELD:
      return EmitExpr(node->GetAs<::googlesql::ResolvedExpr>());
    default:
      return "";
  }
}

std::string Transpiler::EmitExpr(const ::googlesql::ResolvedExpr* expr) {
  if (expr == nullptr) return "";
  switch (expr->node_kind()) {
    case ::googlesql::RESOLVED_LITERAL:
      return EmitLiteral(expr->GetAs<::googlesql::ResolvedLiteral>());
    case ::googlesql::RESOLVED_COLUMN_REF:
      return EmitColumnRef(expr->GetAs<::googlesql::ResolvedColumnRef>());
    case ::googlesql::RESOLVED_FUNCTION_CALL:
      return EmitFunctionCall(expr->GetAs<::googlesql::ResolvedFunctionCall>());
    case ::googlesql::RESOLVED_MAKE_STRUCT:
      return EmitMakeStruct(expr->GetAs<::googlesql::ResolvedMakeStruct>());
    case ::googlesql::RESOLVED_GET_STRUCT_FIELD:
      return EmitGetStructField(
          expr->GetAs<::googlesql::ResolvedGetStructField>());
    default:
      return "";
  }
}

std::string Transpiler::EmitScan(const ::googlesql::ResolvedScan* scan) {
  if (scan == nullptr) return "";
  switch (scan->node_kind()) {
    case ::googlesql::RESOLVED_TABLE_SCAN:
      return EmitTableScan(scan->GetAs<::googlesql::ResolvedTableScan>());
    case ::googlesql::RESOLVED_FILTER_SCAN:
      return EmitFilterScan(scan->GetAs<::googlesql::ResolvedFilterScan>());
    case ::googlesql::RESOLVED_PROJECT_SCAN:
      return EmitProjectScan(scan->GetAs<::googlesql::ResolvedProjectScan>());
    case ::googlesql::RESOLVED_SINGLE_ROW_SCAN:
      return EmitSingleRowScan(
          scan->GetAs<::googlesql::ResolvedSingleRowScan>());
    case ::googlesql::RESOLVED_JOIN_SCAN:
      return EmitJoinScan(scan->GetAs<::googlesql::ResolvedJoinScan>());
    case ::googlesql::RESOLVED_ARRAY_SCAN:
      return EmitArrayScan(scan->GetAs<::googlesql::ResolvedArrayScan>());
    case ::googlesql::RESOLVED_AGGREGATE_SCAN:
      return EmitAggregateScan(
          scan->GetAs<::googlesql::ResolvedAggregateScan>());
    case ::googlesql::RESOLVED_ORDER_BY_SCAN:
      return EmitOrderByScan(scan->GetAs<::googlesql::ResolvedOrderByScan>());
    case ::googlesql::RESOLVED_LIMIT_OFFSET_SCAN:
      return EmitLimitOffsetScan(
          scan->GetAs<::googlesql::ResolvedLimitOffsetScan>());
    case ::googlesql::RESOLVED_ANALYTIC_SCAN:
      return EmitAnalyticScan(scan->GetAs<::googlesql::ResolvedAnalyticScan>());
    default:
      return "";
  }
}

// ---------------------------------------------------------------------------
// Per-shape Emit hooks.
// ---------------------------------------------------------------------------

// Statements ----------------------------------------------------------------

std::string Transpiler::EmitQueryStmt(
    const ::googlesql::ResolvedQueryStmt* node) {
  // Lower the inner `query()` scan and then apply the
  // `output_column_list()` mapping as the final SELECT list, so the
  // user-visible aliases (`SELECT id AS user_id ...`) land on the
  // outermost projection. The inner scan emit already produces a
  // self-contained SELECT, so we wrap it as a derived table and let
  // each `ResolvedOutputColumn` rewrite the column reference into the
  // user-facing name.
  //
  // Value-table queries (`SELECT AS VALUE ...`) collapse the row to a
  // single anonymous value; DuckDB has no direct analog, so we fall
  // back via the empty-string contract per `SHAPE_TRACKER.md`.
  if (node == nullptr) return "";
  if (node->is_value_table()) return "";
  std::string inner = EmitScan(node->query());
  if (inner.empty()) return "";
  std::vector<std::string> outputs;
  outputs.reserve(node->output_column_list_size());
  for (int i = 0; i < node->output_column_list_size(); ++i) {
    std::string oc = EmitOutputColumn(node->output_column_list(i));
    if (oc.empty()) return "";
    outputs.push_back(std::move(oc));
  }
  if (outputs.empty()) return "";
  return absl::StrCat(
      "SELECT ", absl::StrJoin(outputs, ", "), " FROM (", inner, ")");
}

// Scans ---------------------------------------------------------------------

std::string Transpiler::EmitProjectScan(
    const ::googlesql::ResolvedProjectScan* node) {
  // `SELECT <projections> FROM (<input>)`. The output `column_list`
  // is the schema the upstream scan sees; each column either lives in
  // `expr_list` (a `ResolvedComputedColumn` with the bound expression)
  // or passes through from `input_scan`. Computed columns lower as
  // `<expr> AS "<column-name>"`; pass-through columns reference the
  // input column by name (the inner scan emits each input column with
  // its `ResolvedColumn::name()` already).
  //
  // The empty-string contract: if any sub-emit returns "" (input scan
  // we cannot lower, or a computed expression outside the function /
  // literal whitelist), we propagate "" so the engine takes the
  // reference-impl fallback for the whole query rather than emitting
  // partial SQL.
  if (node == nullptr) return "";
  std::string input = EmitScan(node->input_scan());
  if (input.empty()) return "";

  std::vector<std::string> projections;
  projections.reserve(node->column_list_size());
  for (int i = 0; i < node->column_list_size(); ++i) {
    const ::googlesql::ResolvedColumn& col = node->column_list(i);
    const ::googlesql::ResolvedComputedColumn* match = nullptr;
    for (int j = 0; j < node->expr_list_size(); ++j) {
      const ::googlesql::ResolvedComputedColumn* cc = node->expr_list(j);
      if (cc != nullptr && cc->column().column_id() == col.column_id()) {
        match = cc;
        break;
      }
    }
    if (match != nullptr) {
      std::string emitted = EmitComputedColumn(match);
      if (emitted.empty()) return "";
      projections.push_back(std::move(emitted));
    } else {
      projections.push_back(QuoteIdent(col.name()));
    }
  }

  std::string select_list =
      projections.empty() ? "*" : absl::StrJoin(projections, ", ");
  return absl::StrCat("SELECT ", select_list, " FROM (", input, ")");
}

std::string Transpiler::EmitTableScan(
    const ::googlesql::ResolvedTableScan* node) {
  // Emit a self-contained SELECT so the result composes as a derived
  // table for any outer scan (FilterScan, ProjectScan, JoinScan, ...)
  // that wraps it. The shape mirrors what the reference-impl engine
  // sees on its `EvaluatorTableIterator`: each `column_list` entry
  // pulls one column from the underlying `Table` via the position
  // recorded in `column_index_list`. The DuckDB-side table name is
  // whatever the catalog's `Table::Name()` returned -- the engine is
  // responsible for ATTACHing storage so the bare name resolves at
  // execution time.
  if (node == nullptr || node->table() == nullptr) return "";
  const ::googlesql::Table* table = node->table();
  std::vector<std::string> projections;
  projections.reserve(node->column_list_size());
  for (int i = 0; i < node->column_list_size(); ++i) {
    const ::googlesql::ResolvedColumn& out = node->column_list(i);
    // `column_index_list` is the canonical mapping from
    // `column_list[i]` to a position in `table->GetColumn(idx)`. The
    // header on `ResolvedTableScan` requires it to be set 1:1 with
    // `column_list` for any modern client -- the older
    // name-matching path is documented as a violation of the
    // ResolvedColumn contract, so we don't fall back to it.
    if (i >= node->column_index_list_size()) return "";
    int src_idx = node->column_index_list(i);
    const ::googlesql::Column* src = table->GetColumn(src_idx);
    if (src == nullptr) return "";
    std::string src_name = src->Name();
    std::string out_name = out.name();
    if (src_name == out_name) {
      // Skip the AS alias when both names already match -- keeps the
      // emitted SQL readable for the common case where the analyzer
      // didn't have to disambiguate (a single-table SELECT *).
      projections.push_back(QuoteIdent(src_name));
    } else {
      projections.push_back(
          absl::StrCat(QuoteIdent(src_name), " AS ", QuoteIdent(out_name)));
    }
  }
  std::string select_list =
      projections.empty() ? "*" : absl::StrJoin(projections, ", ");
  return absl::StrCat(
      "SELECT ", select_list, " FROM ", QuoteIdent(table->Name()));
}

std::string Transpiler::EmitSingleRowScan(
    const ::googlesql::ResolvedSingleRowScan* node) {
  // The analyzer represents "no FROM clause" (`SELECT 1`,
  // `SELECT 'hi'`, ...) as a `ResolvedSingleRowScan`: a relation with
  // exactly one row and no columns. DuckDB has no first-class
  // single-row table, but a self-contained `SELECT 1` produces the
  // same shape -- one row, with a synthetic column the surrounding
  // `EmitProjectScan` wrap discards. Emitting it as a derived table
  // keeps the composition contract every other scan emit follows
  // (each scan returns a self-contained `SELECT` so a wrapping scan
  // can splice it into `FROM (<inner>)` without re-emitting).
  if (node == nullptr) return "";
  return "SELECT 1";
}

std::string Transpiler::EmitFilterScan(
    const ::googlesql::ResolvedFilterScan* node) {
  // Wrap the input scan as a derived table so the WHERE clause sees
  // exactly the column names the input emitted. DuckDB supports
  // unparenthesized table references in `FROM`, but the derived-table
  // shape is robust across the scan shapes we already know about (a
  // table scan emits its own `SELECT`, so we can't strip it back to
  // a bare relation here without re-emitting). If either child emits
  // "" (still on the disposition fallback) we propagate the empty
  // string up so the engine knows to take the reference-impl path.
  if (node == nullptr) return "";
  std::string input = EmitScan(node->input_scan());
  if (input.empty()) return "";
  std::string filter = EmitExpr(node->filter_expr());
  if (filter.empty()) return "";
  return absl::StrCat("SELECT * FROM (", input, ") WHERE ", filter);
}

std::string Transpiler::EmitJoinScan(
    const ::googlesql::ResolvedJoinScan* node) {
  // INNER / LEFT / RIGHT / FULL all map directly onto DuckDB join
  // syntax. We compose the two input scans as derived tables for the
  // same reason `EmitFilterScan` does: each scan emits a self-
  // contained SELECT so the join sees the column aliases the child
  // emitted. CROSS JOIN is the natural fallback when the analyzer
  // hands us an INNER join with no `join_expr`.
  //
  // Lateral joins, JOIN USING(...) (`has_using`), and the lateral
  // `parameter_list` need bespoke rewrite passes that the first emit
  // pass doesn't cover; we return "" so the engine takes the
  // reference-impl fallback for those shapes.
  if (node == nullptr) return "";
  if (node->is_lateral() || node->has_using() ||
      node->parameter_list_size() > 0) {
    return "";
  }
  std::string left = EmitScan(node->left_scan());
  if (left.empty()) return "";
  std::string right = EmitScan(node->right_scan());
  if (right.empty()) return "";

  const char* join_kw = nullptr;
  switch (node->join_type()) {
    case ::googlesql::ResolvedJoinScan::INNER:
      join_kw = "INNER JOIN";
      break;
    case ::googlesql::ResolvedJoinScan::LEFT:
      join_kw = "LEFT JOIN";
      break;
    case ::googlesql::ResolvedJoinScan::RIGHT:
      join_kw = "RIGHT JOIN";
      break;
    case ::googlesql::ResolvedJoinScan::FULL:
      join_kw = "FULL JOIN";
      break;
    default:
      return "";
  }

  if (node->join_expr() == nullptr) {
    // INNER + no join_expr is the analyzer's representation of
    // CROSS JOIN. LEFT / RIGHT / FULL without a condition is a
    // grammar error the analyzer rejects upstream; we double-check
    // here so a malformed AST falls back instead of emitting
    // illegal SQL.
    if (node->join_type() != ::googlesql::ResolvedJoinScan::INNER) {
      return "";
    }
    return absl::StrCat("SELECT * FROM (", left, ") CROSS JOIN (", right, ")");
  }
  std::string on = EmitExpr(node->join_expr());
  if (on.empty()) return "";
  return absl::StrCat(
      "SELECT * FROM (", left, ") ", join_kw, " (", right, ") ON ", on);
}

std::string Transpiler::EmitArrayScan(
    const ::googlesql::ResolvedArrayScan* node) {
  // Standalone UNNEST subset. Emit
  // `SELECT unnest(<arr>) AS "<col>"`, which DuckDB lowers to one row
  // per array element with the column carrying `<col>` as its name.
  //
  // The full BigQuery `ResolvedArrayScan` surface is wider than what
  // we lower today; everything outside the standalone case falls
  // back to the reference-impl engine via the disposition policy:
  //
  //   * `FROM t, UNNEST(t.arr)` (`input_scan != nullptr` and the
  //     input is not a `SingleRowScan`) needs DuckDB's lateral
  //     `CROSS JOIN unnest(...)` rewrite; we defer that to a
  //     follow-up plan so the column-aliasing contract for the
  //     cross-join side stays focused.
  //   * `UNNEST(arr) WITH OFFSET pos` (`array_offset_column`) needs
  //     a `generate_subscripts(...)` shape with a positional join,
  //     which BigQuery's ordinal semantics make load-bearing.
  //   * Multi-array `UNNEST(a, b, c)` (`array_zip_mode`) is the BQ
  //     array-zip extension; DuckDB's only analog (`list_zip`) does
  //     not match the PAD / TRUNCATE / STRICT modes exactly.
  //   * LEFT/RIGHT-style outer UNNEST (`is_outer` / `join_expr`)
  //     also needs the lateral rewrite plus a literal `NULL` row
  //     fixup for empty arrays.
  if (node == nullptr) return "";
  if (node->array_expr_list_size() != 1 ||
      node->element_column_list_size() != 1 ||
      node->array_offset_column() != nullptr || node->join_expr() != nullptr ||
      node->is_outer() || node->array_zip_mode() != nullptr) {
    return "";
  }
  // Standalone UNNEST either has no input_scan or a SingleRowScan
  // (the analyzer's stand-in for "no FROM clause"). Anything else is
  // the lateral / cross-join shape we defer above.
  if (node->input_scan() != nullptr &&
      node->input_scan()->node_kind() !=
          ::googlesql::RESOLVED_SINGLE_ROW_SCAN) {
    return "";
  }
  std::string arr = EmitExpr(node->array_expr_list(0));
  if (arr.empty()) return "";
  return absl::StrCat("SELECT unnest(",
                      arr,
                      ") AS ",
                      QuoteIdent(node->element_column_list(0).name()));
}

std::string Transpiler::EmitAggregateScan(
    const ::googlesql::ResolvedAggregateScan* node) {
  // Emit `SELECT <grouping-cols>, <aggregates> FROM (<input>) GROUP BY ...`
  // where each grouping column repeats its underlying expression in
  // GROUP BY (rather than referencing the SELECT-list alias), so the
  // emitted SQL composes safely no matter how DuckDB resolves alias
  // visibility in the future.
  //
  // ROLLUP / CUBE / GROUPING SETS / GROUPING() function calls all set
  // one of the grouping-set / rollup / grouping_call lists on the
  // node; we leave those shapes on the reference-impl fallback until
  // a dedicated lower pass lands.
  if (node == nullptr) return "";
  if (node->grouping_set_list_size() > 0 ||
      node->rollup_column_list_size() > 0 ||
      node->grouping_call_list_size() > 0) {
    return "";
  }
  std::string input = EmitScan(node->input_scan());
  if (input.empty()) return "";

  std::vector<std::string> projections;
  std::vector<std::string> group_by_exprs;
  projections.reserve(node->group_by_list_size() + node->aggregate_list_size());
  group_by_exprs.reserve(node->group_by_list_size());

  for (int i = 0; i < node->group_by_list_size(); ++i) {
    const ::googlesql::ResolvedComputedColumn* gc = node->group_by_list(i);
    if (gc == nullptr) return "";
    std::string expr = EmitExpr(gc->expr());
    if (expr.empty()) return "";
    std::string quoted_out = QuoteIdent(gc->column().name());
    projections.push_back(
        expr == quoted_out ? expr : absl::StrCat(expr, " AS ", quoted_out));
    group_by_exprs.push_back(expr);
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
        absl::StrCat(fn, " AS ", QuoteIdent(ac->column().name())));
  }

  std::string select_list =
      projections.empty() ? "*" : absl::StrJoin(projections, ", ");
  std::string sql = absl::StrCat("SELECT ", select_list, " FROM (", input, ")");
  if (!group_by_exprs.empty()) {
    absl::StrAppend(&sql, " GROUP BY ", absl::StrJoin(group_by_exprs, ", "));
  }
  return sql;
}

std::string Transpiler::EmitSetOperationScan(
    const ::googlesql::ResolvedSetOperationScan* /*node*/) {
  return "";
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
  std::vector<std::string> items;
  items.reserve(node->order_by_item_list_size());
  for (int i = 0; i < node->order_by_item_list_size(); ++i) {
    const ::googlesql::ResolvedOrderByItem* item = node->order_by_item_list(i);
    if (item == nullptr || item->column_ref() == nullptr) return "";
    if (item->collation_name() != nullptr) return "";
    std::string col = EmitColumnRef(item->column_ref());
    if (col.empty()) return "";
    const char* dir = item->is_descending() ? "DESC" : "ASC";
    const char* nulls = "";
    switch (item->null_order()) {
      case ::googlesql::ResolvedOrderByItem::NULLS_FIRST:
        nulls = " NULLS FIRST";
        break;
      case ::googlesql::ResolvedOrderByItem::NULLS_LAST:
        nulls = " NULLS LAST";
        break;
      case ::googlesql::ResolvedOrderByItem::ORDER_UNSPECIFIED:
      default:
        break;
    }
    items.push_back(absl::StrCat(col, " ", dir, nulls));
  }
  if (items.empty()) return "";
  return absl::StrCat(
      "SELECT * FROM (", input, ") ORDER BY ", absl::StrJoin(items, ", "));
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

namespace {

// Suffix for the OVER (...) ORDER BY direction + NULL ordering.
std::string OrderByItemSuffix(const ::googlesql::ResolvedOrderByItem* item) {
  const char* dir = item->is_descending() ? "DESC" : "ASC";
  const char* nulls = "";
  switch (item->null_order()) {
    case ::googlesql::ResolvedOrderByItem::NULLS_FIRST:
      nulls = " NULLS FIRST";
      break;
    case ::googlesql::ResolvedOrderByItem::NULLS_LAST:
      nulls = " NULLS LAST";
      break;
    case ::googlesql::ResolvedOrderByItem::ORDER_UNSPECIFIED:
    default:
      break;
  }
  return absl::StrCat(" ", dir, nulls);
}

}  // namespace

std::string Transpiler::BuildPartitionClause(
    const ::googlesql::ResolvedWindowPartitioning* p) {
  if (p == nullptr) return "";
  // PARTITION BY hints and collations are BQ-specific; bail back to
  // the reference-impl fallback.
  if (!p->collation_list().empty() || p->hint_list_size() > 0) {
    return std::string(kAnalyticBail);
  }
  std::vector<std::string> cols;
  cols.reserve(p->partition_by_list_size());
  for (int i = 0; i < p->partition_by_list_size(); ++i) {
    std::string c = EmitColumnRef(p->partition_by_list(i));
    if (c.empty()) return std::string(kAnalyticBail);
    cols.push_back(std::move(c));
  }
  if (cols.empty()) return "";
  return absl::StrCat("PARTITION BY ", absl::StrJoin(cols, ", "));
}

std::string Transpiler::BuildOrderClause(
    const ::googlesql::ResolvedWindowOrdering* o) {
  if (o == nullptr) return "";
  if (o->hint_list_size() > 0) return std::string(kAnalyticBail);
  std::vector<std::string> items;
  items.reserve(o->order_by_item_list_size());
  for (int i = 0; i < o->order_by_item_list_size(); ++i) {
    const ::googlesql::ResolvedOrderByItem* it = o->order_by_item_list(i);
    if (it == nullptr || it->column_ref() == nullptr)
      return std::string(kAnalyticBail);
    if (it->collation_name() != nullptr) return std::string(kAnalyticBail);
    std::string col = EmitColumnRef(it->column_ref());
    if (col.empty()) return std::string(kAnalyticBail);
    items.push_back(absl::StrCat(col, OrderByItemSuffix(it)));
  }
  if (items.empty()) return "";
  return absl::StrCat("ORDER BY ", absl::StrJoin(items, ", "));
}

std::string Transpiler::BuildFrameClause(
    const ::googlesql::ResolvedWindowFrame* wf) {
  if (wf == nullptr) return "";
  const char* unit = nullptr;
  switch (wf->frame_unit()) {
    case ::googlesql::ResolvedWindowFrame::ROWS:
      unit = "ROWS";
      break;
    case ::googlesql::ResolvedWindowFrame::RANGE:
      unit = "RANGE";
      break;
    default:
      return std::string(kAnalyticBail);
  }
  std::string start = EmitFrameBound(wf->start_expr());
  if (start.empty()) return std::string(kAnalyticBail);
  std::string end = EmitFrameBound(wf->end_expr());
  if (end.empty()) return std::string(kAnalyticBail);
  return absl::StrCat(unit, " BETWEEN ", start, " AND ", end);
}

std::string Transpiler::BuildAnalyticProjection(
    const ::googlesql::ResolvedComputedColumnBase* col,
    absl::string_view partition_clause,
    absl::string_view order_clause) {
  if (col == nullptr || col->expr() == nullptr) return "";
  if (col->expr()->node_kind() !=
      ::googlesql::RESOLVED_ANALYTIC_FUNCTION_CALL) {
    return "";
  }
  const auto* afn =
      col->expr()->GetAs<::googlesql::ResolvedAnalyticFunctionCall>();
  std::string fn_sql = EmitAnalyticFunctionCall(afn);
  if (fn_sql.empty()) return "";

  // Frame clause sits *inside* the OVER (...). DuckDB requires an
  // ORDER BY for ROWS / RANGE frames; we leave that contract to the
  // analyzer (which rejects malformed cases at AnalyzeStatement time)
  // and just propagate the bounds verbatim.
  std::string frame_clause = BuildFrameClause(afn->window_frame());
  if (frame_clause == kAnalyticBail) return "";

  std::vector<absl::string_view> over_parts;
  if (!partition_clause.empty()) over_parts.push_back(partition_clause);
  if (!order_clause.empty()) over_parts.push_back(order_clause);
  if (!frame_clause.empty()) over_parts.push_back(frame_clause);
  std::string over =
      absl::StrCat("OVER (", absl::StrJoin(over_parts, " "), ")");
  return absl::StrCat(
      fn_sql, " ", over, " AS ", QuoteIdent(col->column().name()));
}

std::string Transpiler::EmitAnalyticScan(
    const ::googlesql::ResolvedAnalyticScan* node) {
  // Emit `SELECT *, <fn> OVER (PARTITION BY ... ORDER BY ... [frame])
  // AS "<col>", ... FROM (<input>)` -- one projection per analytic
  // function across every group. The OVER clause lives at the group
  // level (PARTITION / ORDER BY) plus per-function (window_frame), so
  // we walk the function-group list once and delegate the per-clause
  // assembly to `BuildPartitionClause` / `BuildOrderClause` /
  // `BuildAnalyticProjection`.
  //
  // Skiplisted shapes the first emit pass does not cover (each helper
  // returns `kAnalyticBail` for these, which we propagate as the
  // empty-string fallback contract):
  //   * Hint lists on the partition / order spec (PARTITION BY ...
  //     OPTIONS(...) and similar) -- BQ-specific.
  //   * Collation lists on PARTITION BY -- collations land separately.
  //   * The `partition_by` `parameter_list` -- lateral-correlated
  //     analytics; defer to the lateral-rewrite plan.
  if (node == nullptr) return "";
  std::string input = EmitScan(node->input_scan());
  if (input.empty()) return "";

  std::vector<std::string> projections;
  for (int g = 0; g < node->function_group_list_size(); ++g) {
    const ::googlesql::ResolvedAnalyticFunctionGroup* group =
        node->function_group_list(g);
    if (group == nullptr) return "";

    std::string partition_clause = BuildPartitionClause(group->partition_by());
    if (partition_clause == kAnalyticBail) return "";
    std::string order_clause = BuildOrderClause(group->order_by());
    if (order_clause == kAnalyticBail) return "";

    for (int f = 0; f < group->analytic_function_list_size(); ++f) {
      std::string projection = BuildAnalyticProjection(
          group->analytic_function_list(f), partition_clause, order_clause);
      if (projection.empty()) return "";
      projections.push_back(std::move(projection));
    }
  }

  if (projections.empty()) {
    // No analytic functions in any group is a malformed AST; the
    // analyzer would not produce it, but we guard so an unexpected
    // shape falls back rather than emitting illegal SQL.
    return "";
  }
  std::string select_list =
      absl::StrCat("*, ", absl::StrJoin(projections, ", "));
  return absl::StrCat("SELECT ", select_list, " FROM (", input, ")");
}

std::string Transpiler::EmitSampleScan(
    const ::googlesql::ResolvedSampleScan* /*node*/) {
  return "";
}

std::string Transpiler::EmitWithRefScan(
    const ::googlesql::ResolvedWithRefScan* /*node*/) {
  return "";
}

// Expressions ---------------------------------------------------------------

std::string Transpiler::EmitLiteral(const ::googlesql::ResolvedLiteral* node) {
  // Delegates to the file-private `EmitValueLiteral` helper. Scalar
  // kinds keep going through `Value::GetSQLLiteral(PRODUCT_EXTERNAL)`
  // because that path already matches DuckDB syntax for INT / FLOAT /
  // BOOL / DATE / NUMERIC / DATETIME etc. The helper carves out the
  // three cases DuckDB spells differently from GoogleSQL:
  //
  //   * STRING: DuckDB reads double-quoted text as an *identifier*,
  //     so we emit the single-quoted form (`'hi'`).
  //   * ARRAY: GoogleSQL's `[1, 2]` shape happens to match DuckDB's,
  //     but we recurse so nested STRINGs / STRUCTs get the
  //     DuckDB-flavored quoting rather than `"..."` / `(...)`.
  //   * STRUCT: DuckDB struct literals are `{'k': v, ...}` keyed by
  //     field name. Anonymous fields (empty name) have no DuckDB
  //     analog and force the caller back to the reference-impl
  //     fallback via the empty-string contract.
  if (node == nullptr) return "";
  return EmitValueLiteral(node->value());
}

std::string Transpiler::EmitParameter(
    const ::googlesql::ResolvedParameter* /*node*/) {
  return "";
}

std::string Transpiler::EmitColumnRef(
    const ::googlesql::ResolvedColumnRef* node) {
  // We reference the column by the `ResolvedColumn::name()` it
  // carries. `EmitTableScan` aliases each underlying storage column
  // onto exactly this name, so the emitted SQL stays consistent
  // whether the column flows straight from a table scan or through
  // a wrapping FilterScan / ProjectScan.
  if (node == nullptr) return "";
  return QuoteIdent(node->column().name());
}

std::string Transpiler::EmitFunctionCall(
    const ::googlesql::ResolvedFunctionCall* node) {
  // Scalar function dispatch goes through the YAML-backed disposition
  // table in `functions.h` for the well-known BigQuery scalar surface
  // (math / string / conditional / regex / datetime / array). Two
  // narrow special cases stay inline:
  //   * `SAFE.<fn>(...)` (`SAFE_ERROR_MODE`) has no native DuckDB
  //     analog; we short-circuit to "" so the fallback fires
  //     regardless of the underlying function's disposition.
  //   * `$make_array(...)` is the analyzer's representation of a
  //     non-const ARRAY literal (`[a, col, b]`); DuckDB shares the
  //     bracket-literal syntax so we emit it directly rather than
  //     going through a `kMap` entry that would render as
  //     `$MAKE_ARRAY(...)`.
  // Anything outside the table falls back to the reference-impl
  // engine via the empty-string contract; the LOG(INFO) records the
  // miss so debug builds can audit which functions still need a
  // disposition row.
  if (node == nullptr || node->function() == nullptr) return "";
  if (node->error_mode() ==
      ::googlesql::ResolvedFunctionCallBase::SAFE_ERROR_MODE) {
    LOG(INFO) << "duckdb transpiler: SAFE function call falls back to "
                 "reference-impl (function="
              << node->function()->Name() << ")";
    return "";
  }
  const std::string name = absl::AsciiStrToLower(node->function()->Name());
  std::vector<std::string> args;
  args.reserve(node->argument_list_size());
  for (int i = 0; i < node->argument_list_size(); ++i) {
    std::string a = EmitExpr(node->argument_list(i));
    if (a.empty()) return "";
    args.push_back(std::move(a));
  }
  if (name == "$make_array") {
    return absl::StrCat("[", absl::StrJoin(args, ", "), "]");
  }
  const auto* entry = LookupFunction(name);
  if (entry == nullptr) {
    LOG(INFO) << "duckdb transpiler: function '" << name
              << "' has no disposition; falling back to reference-impl";
    return "";
  }
  switch (entry->kind) {
    case FnKind::kSkiplist:
      LOG(INFO) << "duckdb transpiler: function '" << name
                << "' is skiplisted (BigQuery-specific / no DuckDB "
                   "analog); falling back to reference-impl";
      return "";
    case FnKind::kFallback:
      LOG(INFO) << "duckdb transpiler: function '" << name
                << "' lowering deferred; falling back to reference-impl";
      return "";
    case FnKind::kMap:
      return absl::StrCat(
          entry->duckdb_name, "(", absl::StrJoin(args, ", "), ")");
  }
  return "";
}

std::string Transpiler::EmitAggregateFunctionCall(
    const ::googlesql::ResolvedAggregateFunctionCall* node) {
  // Aggregate dispatch lives in the same disposition table the scalar
  // emit consults; we just guard the unsupported modifier set first.
  // `$count_star` is the analyzer's representation of `COUNT(*)` -- we
  // emit it directly rather than going through a kMap entry because
  // the dispatch passes zero argument expressions (not even a single
  // `*` placeholder), so the standard `<NAME>(<args>)` shape wouldn't
  // apply.
  //
  // The richer modifiers (HAVING MAX/MIN, ORDER BY / LIMIT inside the
  // aggregate, IGNORE/RESPECT NULLS, multi-level GROUP BY, aggregate
  // filtering) all need bespoke lower passes; we propagate "" so the
  // engine takes the reference-impl fallback whenever one of them is
  // set. `SAFE.<agg>(...)` (`SAFE_ERROR_MODE`) follows the same
  // fallback contract `EmitFunctionCall` uses.
  if (node == nullptr || node->function() == nullptr) return "";
  if (node->error_mode() ==
      ::googlesql::ResolvedFunctionCallBase::SAFE_ERROR_MODE) {
    LOG(INFO) << "duckdb transpiler: SAFE aggregate falls back to "
                 "reference-impl (function="
              << node->function()->Name() << ")";
    return "";
  }
  if (node->having_modifier() != nullptr ||
      node->order_by_item_list_size() > 0 || node->limit() != nullptr ||
      node->group_by_list_size() > 0 ||
      node->group_by_aggregate_list_size() > 0 ||
      node->where_expr() != nullptr || node->having_expr() != nullptr ||
      node->null_handling_modifier() !=
          ::googlesql::ResolvedNonScalarFunctionCallBase::
              DEFAULT_NULL_HANDLING) {
    LOG(INFO) << "duckdb transpiler: aggregate '" << node->function()->Name()
              << "' uses a modifier (HAVING / ORDER BY / LIMIT / GROUP BY / "
                 "NULL-handling) that has no DuckDB analog yet; falling back";
    return "";
  }
  const std::string name = absl::AsciiStrToLower(node->function()->Name());
  if (name == "$count_star") {
    if (node->argument_list_size() != 0) return "";
    return "COUNT(*)";
  }
  std::vector<std::string> args;
  args.reserve(node->argument_list_size());
  for (int i = 0; i < node->argument_list_size(); ++i) {
    std::string a = EmitExpr(node->argument_list(i));
    if (a.empty()) return "";
    args.push_back(std::move(a));
  }
  const auto* entry = LookupFunction(name);
  if (entry == nullptr) {
    LOG(INFO) << "duckdb transpiler: aggregate '" << name
              << "' has no disposition; falling back to reference-impl";
    return "";
  }
  switch (entry->kind) {
    case FnKind::kSkiplist:
      LOG(INFO) << "duckdb transpiler: aggregate '" << name
                << "' is skiplisted; falling back to reference-impl";
      return "";
    case FnKind::kFallback:
      LOG(INFO) << "duckdb transpiler: aggregate '" << name
                << "' lowering deferred; falling back to reference-impl";
      return "";
    case FnKind::kMap: {
      std::string prefix = node->distinct() ? "DISTINCT " : "";
      return absl::StrCat(
          entry->duckdb_name, "(", prefix, absl::StrJoin(args, ", "), ")");
    }
  }
  return "";
}

std::string Transpiler::EmitAnalyticFunctionCall(
    const ::googlesql::ResolvedAnalyticFunctionCall* node) {
  // Window functions: route the name through the disposition table
  // and emit `<NAME>(<args>)` for the analytic call body. The OVER
  // clause (PARTITION BY, ORDER BY, frame) is stitched on by
  // `EmitAnalyticScan` since those live on the surrounding group.
  //
  // The function table flags ROW_NUMBER / RANK / DENSE_RANK / CUME_DIST
  // / PERCENT_RANK / NTILE / LAG / LEAD / FIRST_VALUE / LAST_VALUE /
  // NTH_VALUE as `kMap` so they fall through here; aggregate-over-
  // window calls (SUM / COUNT / AVG / MIN / MAX OVER (...)) flow
  // through the same map entries the scalar aggregate emit uses.
  //
  // We share the SAFE-mode / modifier-rejection contract with
  // `EmitAggregateFunctionCall` because GoogleSQL hands us the same
  // base class for both.
  if (node == nullptr || node->function() == nullptr) return "";
  if (node->error_mode() ==
      ::googlesql::ResolvedFunctionCallBase::SAFE_ERROR_MODE) {
    LOG(INFO) << "duckdb transpiler: SAFE analytic function falls back to "
                 "reference-impl (function="
              << node->function()->Name() << ")";
    return "";
  }
  if (node->null_handling_modifier() !=
      ::googlesql::ResolvedNonScalarFunctionCallBase::DEFAULT_NULL_HANDLING) {
    // IGNORE / RESPECT NULLS modifies LAG / LEAD / FIRST_VALUE /
    // LAST_VALUE semantics; DuckDB has the same keywords but the
    // rewrite needs more care than this emit pass covers.
    LOG(INFO) << "duckdb transpiler: analytic '" << node->function()->Name()
              << "' uses IGNORE/RESPECT NULLS; falling back";
    return "";
  }
  std::vector<std::string> args;
  args.reserve(node->argument_list_size());
  for (int i = 0; i < node->argument_list_size(); ++i) {
    std::string a = EmitExpr(node->argument_list(i));
    if (a.empty()) return "";
    args.push_back(std::move(a));
  }
  const std::string name = absl::AsciiStrToLower(node->function()->Name());
  if (name == "$count_star") {
    if (!args.empty()) return "";
    return "COUNT(*)";
  }
  const auto* entry = LookupFunction(name);
  if (entry == nullptr) {
    LOG(INFO) << "duckdb transpiler: analytic function '" << name
              << "' has no disposition; falling back to reference-impl";
    return "";
  }
  switch (entry->kind) {
    case FnKind::kSkiplist:
      LOG(INFO) << "duckdb transpiler: analytic function '" << name
                << "' is skiplisted; falling back to reference-impl";
      return "";
    case FnKind::kFallback:
      LOG(INFO) << "duckdb transpiler: analytic function '" << name
                << "' lowering deferred; falling back to reference-impl";
      return "";
    case FnKind::kMap: {
      std::string prefix = node->distinct() ? "DISTINCT " : "";
      return absl::StrCat(
          entry->duckdb_name, "(", prefix, absl::StrJoin(args, ", "), ")");
    }
  }
  return "";
}

std::string Transpiler::EmitFrameBound(
    const ::googlesql::ResolvedWindowFrameExpr* expr) {
  if (expr == nullptr) return "";
  switch (expr->boundary_type()) {
    case ::googlesql::ResolvedWindowFrameExpr::UNBOUNDED_PRECEDING:
      return "UNBOUNDED PRECEDING";
    case ::googlesql::ResolvedWindowFrameExpr::CURRENT_ROW:
      return "CURRENT ROW";
    case ::googlesql::ResolvedWindowFrameExpr::UNBOUNDED_FOLLOWING:
      return "UNBOUNDED FOLLOWING";
    case ::googlesql::ResolvedWindowFrameExpr::OFFSET_PRECEDING: {
      if (expr->expression() == nullptr) return "";
      std::string e = EmitExpr(expr->expression());
      if (e.empty()) return "";
      return absl::StrCat(e, " PRECEDING");
    }
    case ::googlesql::ResolvedWindowFrameExpr::OFFSET_FOLLOWING: {
      if (expr->expression() == nullptr) return "";
      std::string e = EmitExpr(expr->expression());
      if (e.empty()) return "";
      return absl::StrCat(e, " FOLLOWING");
    }
  }
  return "";
}

std::string Transpiler::EmitCast(const ::googlesql::ResolvedCast* /*node*/) {
  return "";
}

std::string Transpiler::EmitMakeStruct(
    const ::googlesql::ResolvedMakeStruct* node) {
  // BigQuery `STRUCT(<expr> [AS <name>], ...)` lowers through the
  // analyzer to a `ResolvedMakeStruct` carrying the resolved
  // `StructType` (the source of truth for field names + order) plus
  // the parallel `field_list` of value expressions. DuckDB's struct
  // literal is `{'<name>': <value>, ...}`, keyed by name -- so the
  // emit walks the two lists in lockstep and stitches them onto the
  // DuckDB key/value syntax.
  //
  // BQ STRUCT semantics quirks we honor here, with the matching
  // SHAPE_TRACKER row documenting the fallback:
  //
  //   * BigQuery STRUCT field order is positional and the analyzer
  //     guarantees `field_list_size() == StructType::num_fields()`;
  //     we double-check defensively because a drift would silently
  //     produce a struct DuckDB rejects.
  //   * DuckDB STRUCTs *require* named fields; BigQuery permits
  //     anonymous ones (`STRUCT(1, 2)`). Anonymous fields fall back
  //     to reference-impl via the empty-string contract; a follow-up
  //     plan could synthesize positional names (`_0`, `_1`, ...) and
  //     teach `EmitGetStructField` to look them up.
  //   * NULL field values propagate through `EmitExpr` /
  //     `EmitValueLiteral` as the literal `NULL`; DuckDB matches
  //     BigQuery's "absent NULL field" semantics for struct literals.
  if (node == nullptr) return "";
  const ::googlesql::Type* t = node->type();
  if (t == nullptr || !t->IsStruct()) return "";
  const ::googlesql::StructType* st = t->AsStruct();
  if (st == nullptr || st->num_fields() != node->field_list_size()) {
    return "";
  }
  std::vector<std::string> kvs;
  kvs.reserve(node->field_list_size());
  for (int i = 0; i < node->field_list_size(); ++i) {
    const ::googlesql::StructField& f = st->field(i);
    if (f.name.empty()) return "";
    std::string v = EmitExpr(node->field_list(i));
    if (v.empty()) return "";
    kvs.push_back(absl::StrCat(QuoteString(f.name), ": ", v));
  }
  return absl::StrCat("{", absl::StrJoin(kvs, ", "), "}");
}

std::string Transpiler::EmitGetStructField(
    const ::googlesql::ResolvedGetStructField* node) {
  // BigQuery `s.a` (or `s[OFFSET(0)]`) lowers to a
  // `ResolvedGetStructField` carrying the parent expression, the
  // 0-indexed `field_idx`, and `field_expr_is_positional` (which is
  // user-intent only; it does not change semantics). DuckDB's struct
  // field access is also `<expr>.<name>` or `<expr>['<name>']`; we
  // pick the dotted form for readability when the source struct's
  // field has a name. Anonymous fields are out of scope (see the
  // `EmitMakeStruct` comment) and fall back here too.
  if (node == nullptr || node->expr() == nullptr) return "";
  const ::googlesql::Type* base_type = node->expr()->type();
  if (base_type == nullptr || !base_type->IsStruct()) return "";
  const ::googlesql::StructType* st = base_type->AsStruct();
  if (st == nullptr) return "";
  int idx = node->field_idx();
  if (idx < 0 || idx >= st->num_fields()) return "";
  const ::googlesql::StructField& f = st->field(idx);
  if (f.name.empty()) return "";
  std::string base = EmitExpr(node->expr());
  if (base.empty()) return "";
  // Mark `field_expr_is_positional` accessed even though it carries
  // no semantic weight on the DuckDB side; the validator inside
  // `ResolvedAST::CheckFieldsAccessed` otherwise tears down deep
  // copies through the conformance harness.
  (void)node->field_expr_is_positional();
  return absl::StrCat(base, ".", QuoteIdent(f.name));
}

std::string Transpiler::EmitSubqueryExpr(
    const ::googlesql::ResolvedSubqueryExpr* /*node*/) {
  return "";
}

// Column / output shape -----------------------------------------------------

std::string Transpiler::EmitOutputColumn(
    const ::googlesql::ResolvedOutputColumn* node) {
  // Final user-visible alias mapping. The inner query exposes each
  // physical column under its `ResolvedColumn::name()`; the output
  // schema renames it to `node->name()`. We collapse the alias when
  // both are equal so the emitted SQL stays readable for the common
  // case (`SELECT id FROM people` -> the output name and the
  // physical column name both resolve to `"id"`).
  if (node == nullptr) return "";
  std::string col = QuoteIdent(node->column().name());
  if (node->name() == node->column().name()) {
    return col;
  }
  return absl::StrCat(col, " AS ", QuoteIdent(node->name()));
}

std::string Transpiler::EmitComputedColumn(
    const ::googlesql::ResolvedComputedColumn* node) {
  // `column := expr` lowers to `<expr> AS "<column-name>"`. The
  // upstream `EmitProjectScan` calls this once per
  // `ResolvedComputedColumn` in `expr_list` and slots the result into
  // the SELECT list. The empty-string fallback contract: any child
  // expression we cannot lower (a function outside the disposition
  // table, a parameter we have not implemented yet, ...) propagates
  // back here and we hand "" to the caller so the engine takes the
  // reference-impl fallback instead of emitting partial SQL.
  if (node == nullptr) return "";
  std::string expr = EmitExpr(node->expr());
  if (expr.empty()) return "";
  return absl::StrCat(expr, " AS ", QuoteIdent(node->column().name()));
}

}  // namespace transpiler
}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
