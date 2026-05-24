#include "backend/engine/duckdb/transpiler/transpiler.h"

#include <string>

#include "googlesql/resolved_ast/resolved_ast.h"
#include "googlesql/resolved_ast/resolved_ast_visitor.h"
#include "googlesql/resolved_ast/resolved_node.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace duckdb {
namespace transpiler {

Transpiler::Transpiler() = default;
Transpiler::~Transpiler() = default;

std::string Transpiler::Transpile(const ::googlesql::ResolvedNode* node) {
  // Skeleton: every shape currently lowers to the empty string. The
  // follow-up plans (`transpiler-emit-scans_d5a6b7c8`, ...) wire the
  // per-node `Emit*` helpers below into a real dispatch. Returning
  // "" here is the contract `DuckDBEngine::ExecuteQuery` reads as
  // "not yet supported" -- the engine falls back to the
  // reference-impl engine through the disposition policy.
  (void)node;
  return "";
}

// ---------------------------------------------------------------------------
// Per-shape Emit hooks.
//
// TODO(phase-5b): replace each `return ""` with the real DuckDB SQL
// fragment for the corresponding ResolvedAST node kind. Flip the
// matching row in `SHAPE_TRACKER.md` from `not_started` to `done` in
// the same commit so the tracker stays the source of truth.
// ---------------------------------------------------------------------------

// Statements ----------------------------------------------------------------

std::string Transpiler::EmitQueryStmt(
    const ::googlesql::ResolvedQueryStmt* /*node*/) {
  return "";
}

// Scans ---------------------------------------------------------------------

std::string Transpiler::EmitProjectScan(
    const ::googlesql::ResolvedProjectScan* /*node*/) {
  return "";
}

std::string Transpiler::EmitTableScan(
    const ::googlesql::ResolvedTableScan* /*node*/) {
  return "";
}

std::string Transpiler::EmitSingleRowScan(
    const ::googlesql::ResolvedSingleRowScan* /*node*/) {
  return "";
}

std::string Transpiler::EmitFilterScan(
    const ::googlesql::ResolvedFilterScan* /*node*/) {
  return "";
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
    const ::googlesql::ResolvedLiteral* /*node*/) {
  return "";
}

std::string Transpiler::EmitParameter(
    const ::googlesql::ResolvedParameter* /*node*/) {
  return "";
}

std::string Transpiler::EmitColumnRef(
    const ::googlesql::ResolvedColumnRef* /*node*/) {
  return "";
}

std::string Transpiler::EmitFunctionCall(
    const ::googlesql::ResolvedFunctionCall* /*node*/) {
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
