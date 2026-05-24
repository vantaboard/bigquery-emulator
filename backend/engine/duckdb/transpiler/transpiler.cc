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
      return EmitScan(node->GetAs<::googlesql::ResolvedScan>());
    case ::googlesql::RESOLVED_LITERAL:
    case ::googlesql::RESOLVED_COLUMN_REF:
    case ::googlesql::RESOLVED_FUNCTION_CALL:
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
    const ::googlesql::ResolvedJoinScan* /*node*/) {
  return "";
}

std::string Transpiler::EmitArrayScan(
    const ::googlesql::ResolvedArrayScan* /*node*/) {
  return "";
}

std::string Transpiler::EmitAggregateScan(
    const ::googlesql::ResolvedAggregateScan* /*node*/) {
  return "";
}

std::string Transpiler::EmitSetOperationScan(
    const ::googlesql::ResolvedSetOperationScan* /*node*/) {
  return "";
}

std::string Transpiler::EmitOrderByScan(
    const ::googlesql::ResolvedOrderByScan* /*node*/) {
  return "";
}

std::string Transpiler::EmitLimitOffsetScan(
    const ::googlesql::ResolvedLimitOffsetScan* /*node*/) {
  return "";
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
  // For most scalar kinds `Value::GetSQLLiteral` is the
  // closest-to-source SQL form and is also a valid DuckDB literal.
  // The notable exception is `STRING`: GoogleSQL prefers
  // double-quoted string literals (`"hi"`), but DuckDB uses
  // double-quoted text as an *identifier*, so we override the
  // string case onto the single-quoted form (`'hi'`). Container
  // literals (ARRAY / STRUCT) and the DATE / TIMESTAMP / NUMERIC
  // families still fall back to GetSQLLiteral until the per-type
  // emit lands in a later plan -- the conformance harness pin
  // stays on reference-impl for those shapes today.
  if (node == nullptr) return "";
  const ::googlesql::Value& value = node->value();
  if (!value.is_null() && value.type() != nullptr &&
      value.type()->kind() == ::googlesql::TYPE_STRING) {
    return absl::StrCat(
        "'",
        absl::StrReplaceAll(value.string_value(), {{"'", "''"}}),
        "'");
  }
  return value.GetSQLLiteral(::googlesql::PRODUCT_EXTERNAL);
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
  return "";
}

std::string Transpiler::EmitAggregateFunctionCall(
    const ::googlesql::ResolvedAggregateFunctionCall* /*node*/) {
  return "";
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
    const ::googlesql::ResolvedMakeStruct* /*node*/) {
  return "";
}

std::string Transpiler::EmitGetStructField(
    const ::googlesql::ResolvedGetStructField* /*node*/) {
  return "";
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
