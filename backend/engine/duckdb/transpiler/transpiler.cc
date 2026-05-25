#include "backend/engine/duckdb/transpiler/transpiler.h"

#include <string>
#include <vector>

#include "absl/strings/ascii.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"
#include "absl/strings/str_replace.h"
#include "absl/strings/string_view.h"
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
      return EmitFunctionCall(
          expr->GetAs<::googlesql::ResolvedFunctionCall>());
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
      return EmitOrderByScan(
          scan->GetAs<::googlesql::ResolvedOrderByScan>());
    case ::googlesql::RESOLVED_LIMIT_OFFSET_SCAN:
      return EmitLimitOffsetScan(
          scan->GetAs<::googlesql::ResolvedLimitOffsetScan>());
    default:
      return "";
  }
}

// ---------------------------------------------------------------------------
// Per-shape Emit hooks.
// ---------------------------------------------------------------------------

// Statements ----------------------------------------------------------------

std::string Transpiler::EmitQueryStmt(
    const ::googlesql::ResolvedQueryStmt* /*node*/) {
  // ResolvedQueryStmt wiring lands with the project/output-column
  // emit plan (`transpiler-emit-join-agg_e6b7c8d9`). Today the
  // dispatcher returns "" so the engine falls back to reference-impl.
  return "";
}

// Scans ---------------------------------------------------------------------

std::string Transpiler::EmitProjectScan(
    const ::googlesql::ResolvedProjectScan* /*node*/) {
  return "";
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
  return absl::StrCat("SELECT ", select_list, " FROM ",
                      QuoteIdent(table->Name()));
}

std::string Transpiler::EmitSingleRowScan(
    const ::googlesql::ResolvedSingleRowScan* /*node*/) {
  return "";
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
    return absl::StrCat("SELECT * FROM (", left, ") CROSS JOIN (", right,
                        ")");
  }
  std::string on = EmitExpr(node->join_expr());
  if (on.empty()) return "";
  return absl::StrCat("SELECT * FROM (", left, ") ", join_kw, " (", right,
                      ") ON ", on);
}

std::string Transpiler::EmitArrayScan(
    const ::googlesql::ResolvedArrayScan* node) {
  // Phase 5j subset: standalone UNNEST. Emit
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
      node->array_offset_column() != nullptr ||
      node->join_expr() != nullptr || node->is_outer() ||
      node->array_zip_mode() != nullptr) {
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
  return absl::StrCat("SELECT unnest(", arr, ") AS ",
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
  projections.reserve(node->group_by_list_size() +
                      node->aggregate_list_size());
  group_by_exprs.reserve(node->group_by_list_size());

  for (int i = 0; i < node->group_by_list_size(); ++i) {
    const ::googlesql::ResolvedComputedColumn* gc = node->group_by_list(i);
    if (gc == nullptr) return "";
    std::string expr = EmitExpr(gc->expr());
    if (expr.empty()) return "";
    std::string quoted_out = QuoteIdent(gc->column().name());
    projections.push_back(
        expr == quoted_out ? expr
                           : absl::StrCat(expr, " AS ", quoted_out));
    group_by_exprs.push_back(expr);
  }

  for (int i = 0; i < node->aggregate_list_size(); ++i) {
    const ::googlesql::ResolvedComputedColumnBase* ac =
        node->aggregate_list(i);
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
  std::string sql =
      absl::StrCat("SELECT ", select_list, " FROM (", input, ")");
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
    const ::googlesql::ResolvedOrderByItem* item =
        node->order_by_item_list(i);
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
  return absl::StrCat("SELECT * FROM (", input, ") ORDER BY ",
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

std::string Transpiler::EmitAnalyticScan(
    const ::googlesql::ResolvedAnalyticScan* /*node*/) {
  return "";
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

std::string Transpiler::EmitLiteral(
    const ::googlesql::ResolvedLiteral* node) {
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
  // Phase 5g subset: only the handful of scalar functions the plan
  // pulled forward (`COALESCE` and `IFNULL`) lower to DuckDB SQL
  // here. `SAFE.<fn>(...)` style call-sites set the SAFE error mode
  // which has no native DuckDB analog yet, so we leave them on the
  // reference-impl fallback. Everything else (including SAFE_CAST,
  // which the analyzer routes through `ResolvedCast` rather than
  // `ResolvedFunctionCall`) follows the same fallback path.
  if (node == nullptr || node->function() == nullptr) return "";
  if (node->error_mode() == ::googlesql::ResolvedFunctionCallBase::SAFE_ERROR_MODE) {
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
  if (name == "coalesce") {
    return absl::StrCat("COALESCE(", absl::StrJoin(args, ", "), ")");
  }
  if (name == "ifnull") {
    if (args.size() != 2) return "";
    return absl::StrCat("IFNULL(", args[0], ", ", args[1], ")");
  }
  if (name == "$make_array") {
    // `[a, b, c]` (or `ARRAY[a, b, c]`) lowers through the analyzer
    // to a `$make_array(...)` function call when the element
    // expressions aren't all constants. DuckDB's array constructor
    // shares BigQuery's bracket syntax, so the lowering is a
    // direct join. The all-const case ships as a `ResolvedLiteral`
    // (handled by `EmitValueLiteral` above), not here.
    return absl::StrCat("[", absl::StrJoin(args, ", "), "]");
  }
  return "";
}

std::string Transpiler::EmitAggregateFunctionCall(
    const ::googlesql::ResolvedAggregateFunctionCall* node) {
  // First-wave aggregate subset: COUNT(*), SUM, COUNT, AVG, MIN, MAX
  // with optional DISTINCT. The richer modifiers (ORDER BY inside
  // an aggregate, HAVING MAX/MIN, IGNORE/RESPECT NULLS, multi-level
  // GROUP BY, aggregate filtering, LIMIT) all need bespoke lower
  // passes -- we propagate "" so the engine takes the reference-impl
  // fallback whenever one of them is set. SAFE.<agg>(...) follows
  // the same fallback contract `EmitFunctionCall` uses.
  if (node == nullptr || node->function() == nullptr) return "";
  if (node->error_mode() ==
      ::googlesql::ResolvedFunctionCallBase::SAFE_ERROR_MODE) {
    return "";
  }
  if (node->having_modifier() != nullptr ||
      node->order_by_item_list_size() > 0 || node->limit() != nullptr ||
      node->group_by_list_size() > 0 ||
      node->group_by_aggregate_list_size() > 0 ||
      node->where_expr() != nullptr ||
      node->having_expr() != nullptr ||
      node->null_handling_modifier() !=
          ::googlesql::ResolvedNonScalarFunctionCallBase::
              DEFAULT_NULL_HANDLING) {
    return "";
  }
  const std::string name = absl::AsciiStrToLower(node->function()->Name());
  if (name == "$count_star") {
    if (node->argument_list_size() != 0) return "";
    return "COUNT(*)";
  }
  if (name != "sum" && name != "count" && name != "avg" && name != "min" &&
      name != "max") {
    return "";
  }
  std::vector<std::string> args;
  args.reserve(node->argument_list_size());
  for (int i = 0; i < node->argument_list_size(); ++i) {
    std::string a = EmitExpr(node->argument_list(i));
    if (a.empty()) return "";
    args.push_back(std::move(a));
  }
  std::string args_str = absl::StrJoin(args, ", ");
  std::string prefix = node->distinct() ? "DISTINCT " : "";
  return absl::StrCat(absl::AsciiStrToUpper(name), "(", prefix, args_str,
                      ")");
}

std::string Transpiler::EmitAnalyticFunctionCall(
    const ::googlesql::ResolvedAnalyticFunctionCall* /*node*/) {
  return "";
}

std::string Transpiler::EmitCast(
    const ::googlesql::ResolvedCast* /*node*/) {
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
  if (st == nullptr ||
      st->num_fields() != node->field_list_size()) {
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
    const ::googlesql::ResolvedOutputColumn* /*node*/) {
  return "";
}

std::string Transpiler::EmitComputedColumn(
    const ::googlesql::ResolvedComputedColumn* /*node*/) {
  return "";
}

}  // namespace transpiler
}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
