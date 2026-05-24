#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_DUCKDB_TRANSPILER_TRANSPILER_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_DUCKDB_TRANSPILER_TRANSPILER_H_

// DuckDB transpiler (Phase 5.B).
//
// `Transpiler` walks a GoogleSQL `ResolvedAST` produced by
// `googlesql::AnalyzeStatement` (see
// `backend/engine/reference_impl/reference_impl_engine.cc`) and
// emits a DuckDB-flavored SQL string the DuckDB C++ client can
// execute. This file is the *skeleton* that the engine and the
// follow-up plans (`transpiler-emit-scans_d5a6b7c8`, ...) fill in
// one node kind at a time, mirroring the cloud-spanner-emulator
// shape-tracker approach.
//
// The class inherits from `googlesql::ResolvedASTVisitor` so future
// plans can either override `VisitResolved*` directly (for nodes
// that need to recurse into children with non-default ordering) or
// route through the per-shape `Emit*` helpers declared here. Today
// every `Emit*` returns the empty string — the plan that lands the
// emit logic for each shape flips the disposition in
// `SHAPE_TRACKER.md` from `not_started` to `done`.
//
// `Transpile` is the public entry point: it accepts the root
// `ResolvedQueryStmt` (or any subtree) and returns the lowered
// DuckDB SQL. Today the result is the empty string for everything;
// `DuckDBEngine::ExecuteQuery` (in `duckdb_engine.cc`) treats an
// empty transpilation as "not yet supported" and falls back to the
// reference-impl engine through the disposition table.
//
// Threading: a `Transpiler` instance is *not* thread-safe — it
// carries a per-traversal accumulator. The engine constructs a
// fresh `Transpiler` per `ExecuteQuery` call so cross-query state
// is not an issue.

#include <string>

#include "googlesql/resolved_ast/resolved_ast.h"
#include "googlesql/resolved_ast/resolved_ast_visitor.h"
#include "googlesql/resolved_ast/resolved_node.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace duckdb {
namespace transpiler {

class Transpiler : public ::googlesql::ResolvedASTVisitor {
 public:
  Transpiler();
  ~Transpiler() override;

  Transpiler(const Transpiler&) = delete;
  Transpiler& operator=(const Transpiler&) = delete;

  // Lower the resolved AST rooted at `node` (typically a
  // `ResolvedQueryStmt`) into a DuckDB SQL string.
  //
  // Today this always returns the empty string -- the per-shape
  // `Emit*` methods below are placeholders the follow-up plans
  // fill in. Callers treat an empty result as "transpiler does not
  // yet support this shape" and fall back to the reference-impl
  // engine via the disposition table.
  //
  // Safe to call with a null `node`; returns the empty string in
  // that case.
  std::string Transpile(const ::googlesql::ResolvedNode* node);

 protected:
  // ---------------------------------------------------------------
  // Per-shape Emit hooks.
  //
  // Each hook returns the DuckDB SQL fragment for one ResolvedAST
  // node kind. The default implementations return the empty
  // string; subsequent plans replace each with the real emit
  // logic. Group them roughly by node category so the file diff
  // for "add scan emit" / "add expr emit" stays focused.
  //
  // The list below is *not* exhaustive — it tracks the node
  // kinds the SHAPE_TRACKER classifies as `not_started` and the
  // ones DuckDB has a sensible analog for. Skiplist nodes
  // (graph / ML / pivot / DML / DDL / ...) get their dispositions
  // in the tracker without an `Emit*` here; the visitor's
  // `DefaultVisit` falls through to "not supported" and the
  // engine fallback fires.
  // ---------------------------------------------------------------

  // Statements -----------------------------------------------------
  virtual std::string EmitQueryStmt(
      const ::googlesql::ResolvedQueryStmt* node);

  // Scans ----------------------------------------------------------
  virtual std::string EmitProjectScan(
      const ::googlesql::ResolvedProjectScan* node);
  virtual std::string EmitTableScan(
      const ::googlesql::ResolvedTableScan* node);
  virtual std::string EmitSingleRowScan(
      const ::googlesql::ResolvedSingleRowScan* node);
  virtual std::string EmitFilterScan(
      const ::googlesql::ResolvedFilterScan* node);
  virtual std::string EmitJoinScan(
      const ::googlesql::ResolvedJoinScan* node);
  virtual std::string EmitArrayScan(
      const ::googlesql::ResolvedArrayScan* node);
  virtual std::string EmitAggregateScan(
      const ::googlesql::ResolvedAggregateScan* node);
  virtual std::string EmitSetOperationScan(
      const ::googlesql::ResolvedSetOperationScan* node);
  virtual std::string EmitOrderByScan(
      const ::googlesql::ResolvedOrderByScan* node);
  virtual std::string EmitLimitOffsetScan(
      const ::googlesql::ResolvedLimitOffsetScan* node);
  virtual std::string EmitAnalyticScan(
      const ::googlesql::ResolvedAnalyticScan* node);
  virtual std::string EmitSampleScan(
      const ::googlesql::ResolvedSampleScan* node);
  virtual std::string EmitWithRefScan(
      const ::googlesql::ResolvedWithRefScan* node);

  // Expressions ----------------------------------------------------
  virtual std::string EmitLiteral(
      const ::googlesql::ResolvedLiteral* node);
  virtual std::string EmitParameter(
      const ::googlesql::ResolvedParameter* node);
  virtual std::string EmitColumnRef(
      const ::googlesql::ResolvedColumnRef* node);
  virtual std::string EmitFunctionCall(
      const ::googlesql::ResolvedFunctionCall* node);
  virtual std::string EmitAggregateFunctionCall(
      const ::googlesql::ResolvedAggregateFunctionCall* node);
  virtual std::string EmitAnalyticFunctionCall(
      const ::googlesql::ResolvedAnalyticFunctionCall* node);
  virtual std::string EmitCast(
      const ::googlesql::ResolvedCast* node);
  virtual std::string EmitMakeStruct(
      const ::googlesql::ResolvedMakeStruct* node);
  virtual std::string EmitGetStructField(
      const ::googlesql::ResolvedGetStructField* node);
  virtual std::string EmitSubqueryExpr(
      const ::googlesql::ResolvedSubqueryExpr* node);

  // Column / output shape ------------------------------------------
  virtual std::string EmitOutputColumn(
      const ::googlesql::ResolvedOutputColumn* node);
  virtual std::string EmitComputedColumn(
      const ::googlesql::ResolvedComputedColumn* node);

 private:
  // Dispatch a `ResolvedExpr` to the matching `Emit*` method based on
  // its `node_kind()`. Returns the empty string for any expression
  // kind whose `Emit*` still returns `""` -- the engine fallback
  // policy treats an empty fragment the same way `Transpile()` does
  // and re-runs the query through the reference-impl evaluator.
  std::string EmitExpr(const ::googlesql::ResolvedExpr* expr);

  // Dispatch a `ResolvedScan` to the matching `Emit*` method. Same
  // empty-string-as-fallback contract as `EmitExpr`.
  std::string EmitScan(const ::googlesql::ResolvedScan* scan);
};

}  // namespace transpiler
}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_DUCKDB_TRANSPILER_TRANSPILER_H_
