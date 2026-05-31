#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_DUCKDB_TRANSPILER_TRANSPILER_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_DUCKDB_TRANSPILER_TRANSPILER_H_

// DuckDB transpiler.
//
// `Transpiler` walks a GoogleSQL `ResolvedAST` produced by
// `googlesql::AnalyzeStatement` (see
// `backend/engine/duckdb/duckdb_engine.cc`) and
// emits a DuckDB-flavored SQL string the DuckDB C++ client can
// execute. The visitor's emit set grows one shape at a time per the
// `SHAPE_TRACKER.md` file in this directory, mirroring the
// cloud-spanner-emulator shape-tracker approach.
//
// The class inherits from `googlesql::ResolvedASTVisitor` so future
// plans can either override `VisitResolved*` directly (for nodes
// that need to recurse into children with non-default ordering) or
// route through the per-shape `Emit*` helpers declared here. The
// SHAPE_TRACKER row marked `done` for a given node corresponds to
// an `Emit*` that returns DuckDB-flavored SQL; rows marked
// `not_started` still bottom out at the empty string and the engine
// reads that as "not yet supported" and surfaces UNIMPLEMENTED to
// the gateway.
//
// Implemented baseline (top-level SELECT, the `transpiler-select-core`
// plan):
//
// * `EmitQueryStmt` lowers the root statement: it walks `query()` for
//   the relational tree, then renames each physical column through
//   the `output_column_list()` mapping so the user-visible alias
//   lands on the outermost SELECT.
// * `EmitProjectScan` wraps `input_scan()` and projects the
//   `column_list()` schema, picking each column out of the matching
//   `expr_list()` entry (computed) or referencing it by name
//   (pass-through). When `expr_list()` is empty and `column_list()`
//   is a permutation of `input_scan()->column_list()` by column id,
//   the wrap is elided so the redundant
//   `SELECT * FROM (SELECT * FROM ...)` layer the analyzer always
//   inserts above a TableScan does not stack onto every scan emit.
// * `EmitSingleRowScan` emits `SELECT 1` so a `ProjectScan` over
//   `SingleRowScan` (the analyzer's "no FROM clause" shape) composes
//   as a derived table.
// * `EmitComputedColumn` lowers `column := expr` to
//   `<expr> AS "<resolved-column-name>"`.
// * `EmitOutputColumn` renders the final alias mapping
//   (`<column-name> AS <output-name>`, collapsed when the names
//   match).
//
// Earlier plans (`transpiler-emit-scans` etc.) cover the other
// `done` rows in `SHAPE_TRACKER.md` (TableScan, FilterScan, JoinScan,
// AggregateScan, OrderByScan, LimitOffsetScan, AnalyticScan, ...).
//
// Set operations + sampling (`transpiler-setops-sample`) layer two
// more scan kinds onto the dispatcher:
//
// * `EmitSetOperationScan` lowers `UNION ALL`, `UNION DISTINCT`,
//   `INTERSECT DISTINCT`, `EXCEPT DISTINCT` (and the DuckDB-native
//   `INTERSECT ALL` / `EXCEPT ALL` extensions that match standard
//   SQL bag semantics) by routing each `ResolvedSetOperationItem`
//   through a private `EmitSetOperationItem` helper. The helper
//   honors BigQuery's column-name propagation by projecting each
//   item's `output_column_list(i)` onto the parent's
//   `column_list(i)` name.
// * `EmitSampleScan` lowers BigQuery `TABLESAMPLE` to DuckDB's
//   `USING SAMPLE` for the `SYSTEM` / `BERNOULLI` (PERCENT) and
//   `RESERVOIR` (ROWS) shapes whose semantics match. Weight columns,
//   stratify (`PARTITION BY`) lists, and repeatable seeds stay on
//   the empty-string fallback for now.
//
// Expression-core (`transpiler-expression-core`) layers four scalar
// shapes on top of the baseline:
//
// * `EmitParameter` lowers `@<name>` and positional `?` parameters
//   to DuckDB `$N` placeholders and accumulates a `ParameterRef`
//   per unique slot in `parameter_order()` so the engine can copy
//   values across in bind order.
// * `EmitCast` lowers `CAST(<expr> AS <type>)` (and `SAFE_CAST(...)`
//   -> `TRY_CAST(...)`) for every type kind that `types.h` exposes a
//   first-class DuckDB analog for. Format / time-zone / extended /
//   type-modifier shapes stay on the fallback.
// * `EmitFunctionArgument` routes a `ResolvedFunctionArgument`
//   through the `expr()` slot it wraps; non-expression slots
//   (scan / model / lambda / ...) stay on the fallback.
// * `EmitWithExpr` lowers `WITH(<a> AS <expr>, ...) <body>` to a
//   DuckDB scalar subquery that exposes each binding as a column on
//   an inner SELECT and then projects the body off it, preserving
//   GoogleSQL's once-per-row evaluation semantics.
//
// `Transpile` is the public entry point: it accepts the root
// `ResolvedQueryStmt` (or any subtree) and returns the lowered
// DuckDB SQL. `DuckDBEngine::ExecuteQuery` (in `duckdb_engine.cc`)
// treats an empty transpilation as "not yet supported" and surfaces
// UNIMPLEMENTED to the gateway through the engine's empty-string
// contract.
//
// Threading: a `Transpiler` instance is *not* thread-safe — it
// carries a per-traversal accumulator. The engine constructs a
// fresh `Transpiler` per `ExecuteQuery` call so cross-query state
// is not an issue.

#include <string>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "absl/strings/string_view.h"
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
  // Bind metadata for one DuckDB `$N` placeholder emitted by the
  // transpiler. The accumulator is exposed through
  // `parameter_order()` so the engine can copy GoogleSQL
  // `Value`s into DuckDB's bind buffer in placeholder-numerical
  // order.
  //
  // Exactly one of {`name`, `position`} carries the GoogleSQL
  // identity:
  //   * Named parameter: `name` is the analyzer-lowercased
  //     identifier (e.g. "@CustomerId" -> "customerid"); `position`
  //     is 0.
  //   * Positional parameter: `name` is empty; `position` is the
  //     1-based GoogleSQL position the analyzer assigned.
  // Either way the entry's index in `parameter_order()` is the
  // 0-based DuckDB placeholder slot (slot `i` is `$<i+1>`), so
  // multiple references to the same named parameter share one slot.
  struct ParameterRef {
    std::string name;
    int position = 0;
  };

  Transpiler();
  ~Transpiler() override;

  Transpiler(const Transpiler&) = delete;
  Transpiler& operator=(const Transpiler&) = delete;

  // Lower the resolved AST rooted at `node` (typically a
  // `ResolvedQueryStmt`) into a DuckDB SQL string.
  //
  // Returns the DuckDB SQL for any shape covered by the per-shape
  // `Emit*` methods below; an empty string means "transpiler cannot
  // lower this shape yet" and callers treat that as a signal for
  // the engine to surface UNIMPLEMENTED.
  //
  // Safe to call with a null `node`; returns the empty string in
  // that case.
  std::string Transpile(const ::googlesql::ResolvedNode* node);

  // Bind-order accumulator for the GoogleSQL parameters encountered
  // during the most recent traversal. Slot `i` (0-based) is the
  // parameter the transpiler emitted as `$<i+1>`. The engine reads
  // this back to copy values into DuckDB's bind buffer; named
  // parameters that appear multiple times in the SQL share a single
  // slot (see `EmitParameter`), so the vector contains one entry per
  // unique placeholder, not one per textual reference.
  const std::vector<ParameterRef>& parameter_order() const {
    return parameter_order_;
  }

 protected:
  // ---------------------------------------------------------------
  // Per-shape Emit hooks.
  //
  // Each hook returns the DuckDB SQL fragment for one ResolvedAST
  // node kind. Hooks for `done` rows in `SHAPE_TRACKER.md` emit
  // DuckDB SQL; hooks for `not_started` rows still return the empty
  // string and trip the engine fallback. Group them roughly by node
  // category so the file diff for "add scan emit" / "add expr emit"
  // stays focused.
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
  virtual std::string EmitQueryStmt(const ::googlesql::ResolvedQueryStmt* node);

  // Scans ----------------------------------------------------------
  virtual std::string EmitProjectScan(
      const ::googlesql::ResolvedProjectScan* node);
  virtual std::string EmitTableScan(const ::googlesql::ResolvedTableScan* node);
  virtual std::string EmitSingleRowScan(
      const ::googlesql::ResolvedSingleRowScan* node);
  virtual std::string EmitFilterScan(
      const ::googlesql::ResolvedFilterScan* node);
  virtual std::string EmitJoinScan(const ::googlesql::ResolvedJoinScan* node);
  virtual std::string EmitArrayScan(const ::googlesql::ResolvedArrayScan* node);
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
  virtual std::string EmitLiteral(const ::googlesql::ResolvedLiteral* node);
  virtual std::string EmitParameter(const ::googlesql::ResolvedParameter* node);
  virtual std::string EmitColumnRef(const ::googlesql::ResolvedColumnRef* node);
  virtual std::string EmitFunctionCall(
      const ::googlesql::ResolvedFunctionCall* node);
  virtual std::string EmitAggregateFunctionCall(
      const ::googlesql::ResolvedAggregateFunctionCall* node);
  virtual std::string EmitAnalyticFunctionCall(
      const ::googlesql::ResolvedAnalyticFunctionCall* node);
  virtual std::string EmitCast(const ::googlesql::ResolvedCast* node);
  virtual std::string EmitMakeStruct(
      const ::googlesql::ResolvedMakeStruct* node);
  virtual std::string EmitGetStructField(
      const ::googlesql::ResolvedGetStructField* node);
  // BigQuery `<json>.<field>` lowers through DuckDB's `->` operator
  // when the result type is JSON (the common case for chained
  // `<json>.a.b` access) and through `->>` when the analyzer
  // resolves the return as a scalar (rare today; reserved for
  // analyzer-driven coercion). See the `EmitGetJsonField` body for
  // the deliberate choice between `->` / `->>` and
  // `json_extract` / `json_extract_string`.
  virtual std::string EmitGetJsonField(
      const ::googlesql::ResolvedGetJsonField* node);
  virtual std::string EmitSubqueryExpr(
      const ::googlesql::ResolvedSubqueryExpr* node);
  virtual std::string EmitWithExpr(const ::googlesql::ResolvedWithExpr* node);

  // Function-argument wrapper --------------------------------------
  // `ResolvedFunctionArgument` carries one of {expr, scan, model,
  // connection, descriptor, lambda, sequence, graph}. Today we
  // route through the `expr()` slot so callers that walk
  // `generic_argument_list` can treat a wrapped scalar argument as a
  // first-class expression. Every other slot stays on the
  // empty-string fallback (no caller proves the corresponding
  // function-argument syntax lowers cleanly to DuckDB yet).
  virtual std::string EmitFunctionArgument(
      const ::googlesql::ResolvedFunctionArgument* node);

  // Column / output shape ------------------------------------------
  virtual std::string EmitOutputColumn(
      const ::googlesql::ResolvedOutputColumn* node);
  virtual std::string EmitComputedColumn(
      const ::googlesql::ResolvedComputedColumn* node);

 private:
  // Dispatch a `ResolvedExpr` to the matching `Emit*` method based on
  // its `node_kind()`. Returns the empty string for any expression
  // kind whose `Emit*` still returns `""` -- the engine treats an
  // empty fragment the same way `Transpile()` does and surfaces
  // UNIMPLEMENTED to the gateway.
  std::string EmitExpr(const ::googlesql::ResolvedExpr* expr);

  // Dispatch a `ResolvedScan` to the matching `Emit*` method. Same
  // empty-string-as-fallback contract as `EmitExpr`.
  std::string EmitScan(const ::googlesql::ResolvedScan* scan);

  // Lower one `ResolvedWindowFrameExpr` (PRECEDING / CURRENT ROW /
  // FOLLOWING). Used by `EmitAnalyticScan` for the inner ROWS / RANGE
  // BETWEEN ... AND ... clause; returns the empty string when the
  // bound is malformed or when an offset bound carries an expression
  // we cannot lower yet, so the caller can propagate the empty-string
  // contract.
  std::string EmitFrameBound(const ::googlesql::ResolvedWindowFrameExpr* expr);

  // Per-clause helpers for `EmitAnalyticScan`. Each returns:
  //   * `""` when the clause is legally absent (e.g. no PARTITION BY)
  //   * `kAnalyticBail` when the shape is not yet lowerable and the
  //     caller must abandon the analytic emit and let the engine
  //     surface UNIMPLEMENTED.
  //   * the SQL fragment otherwise.
  // The bail sentinel keeps the helpers' empty-string-as-fallback
  // contract from colliding with the legitimate "clause omitted"
  // value.
  static constexpr absl::string_view kAnalyticBail =
      "\x01"
      "bail";
  std::string BuildPartitionClause(
      const ::googlesql::ResolvedWindowPartitioning* p);
  std::string BuildOrderClause(const ::googlesql::ResolvedWindowOrdering* o);
  std::string BuildFrameClause(const ::googlesql::ResolvedWindowFrame* wf);
  std::string BuildAnalyticProjection(
      const ::googlesql::ResolvedComputedColumnBase* col,
      absl::string_view partition_clause,
      absl::string_view order_clause);

  // Lower one `ResolvedSetOperationItem` into a derived-table SELECT
  // that projects `parent->column_list()` in order. Each
  // `output_column_list[i]` names the column the item's child scan
  // exposes for the parent's `column_list(i)` slot, so we wrap the
  // child scan as `FROM (<scan>)` and project `"<output_i>" AS
  // "<parent_i>"` (collapsed when the names already match). Returns
  // the empty string when the child scan cannot lower or the
  // output/parent column counts disagree, so callers propagate the
  // empty-string fallback contract.
  std::string EmitSetOperationItem(
      const ::googlesql::ResolvedSetOperationItem* item,
      const ::googlesql::ResolvedSetOperationScan* parent);

  // Look up (or assign) the DuckDB `$N` slot for a named GoogleSQL
  // parameter. The first reference appends a new `ParameterRef` to
  // `parameter_order_`; subsequent references reuse the same slot
  // so a query like `WHERE @x AND @x` carries only one bind value.
  // The returned slot is 1-based to match DuckDB's `$1` notation.
  int LookupOrAssignNamedParameter(absl::string_view name);

  // Append a new positional `ParameterRef` and return its 1-based
  // slot. Positional GoogleSQL parameters are referentially distinct
  // every time they appear, so we never dedupe them.
  int AssignPositionalParameter(int analyzer_position);

  // Bind-order accumulator. Each entry is a `ParameterRef` whose
  // index `i` is the 0-based DuckDB placeholder slot (slot `i` is
  // `$<i+1>`). Named parameter lookups go through
  // `name_to_slot_` for dedup; positional ones always append.
  std::vector<ParameterRef> parameter_order_;
  absl::flat_hash_map<std::string, int> name_to_slot_;
};

}  // namespace transpiler
}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_DUCKDB_TRANSPILER_TRANSPILER_H_
