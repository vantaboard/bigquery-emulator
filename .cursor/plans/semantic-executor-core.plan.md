---
name: ""
overview: ""
todos: []
isProject: false
---

# Semantic Executor Core

## Goal

Build the foundational local row/array/value interpreter that the
`semantic_executor` route dispatches to. This is the in-process
evaluator that owns BigQuery-exact semantics for the shapes the
DuckDB fast path cannot represent correctly: strict NULL behavior,
exact error surfaces, deep STRUCT mutation, complex array
evaluation order, and SAFE-mode behavior.

This plan ships the **core** evaluator — the engine the other
semantic-route plans (`semantic-functions-compliance.plan.md`,
`array-struct-semantic-path.plan.md`, `dml-local-executor.plan.md`,
`cte-subquery-routing.plan.md`) layer on top of.

## Background

DuckDB is a column-oriented analytical SQL engine. Its semantics are
mostly compatible with BigQuery, but for a long-tail of shapes
("compatible" is not "identical") the only honest local
implementation is a small row-at-a-time interpreter that reads
inputs as Arrow batches from DuckDB and evaluates BigQuery
expressions itself. The semantic executor is that interpreter.

Design constraints:

- The semantic executor must **not** re-implement SQL planning. The
  GoogleSQL analyzer already provides a `ResolvedAST` that is
  evaluation-ready.
- It must **reuse** `DuckDbExecutor` as a row source for everything
  it cannot evaluate itself (table scans, joins, aggregations the
  fast path covers).
- It must produce the **same Arrow output** shape the DuckDB fast
  path produces, so the Storage Read API path and the REST `f`/`v`
  marshaler do not need to branch on route.

## Dependencies

- `engine-router-foundation.plan.md` so the coordinator can route
  queries here.
- `execution-disposition-registry.plan.md` so the routing decision
  is registry-driven.

## Scope

This plan covers:

- Scalar-only `SELECT` (no `FROM` clause). Today this surfaces
  `UNIMPLEMENTED` on the DuckDB fast path; it becomes the smallest
  end-to-end semantic-executor case.
- Expression evaluation for the BigQuery type system: BOOL, INT64,
  FLOAT64, NUMERIC / BIGNUMERIC, STRING, BYTES, DATE, TIME, DATETIME,
  TIMESTAMP, JSON, INTERVAL, UUID, ARRAY, STRUCT.
- Parameter binding identical to the DuckDB fast path's bind order
  (named + positional GoogleSQL parameters).
- `CASE` / `IF` / coalescing operators with BigQuery NULL semantics.
- Strict NULL semantics for arithmetic (overflow → error in non-
  SAFE mode, NULL in SAFE mode).
- Exact error surfaces: BigQuery error codes / messages on each
  evaluation failure, propagated to the gateway as
  `INVALID_ARGUMENT` with the right `reason` token.
- A `RowSource` adapter that wraps a `DuckDbExecutor` result so
  composable shapes can feed semantic-executor evaluations.

Out of scope (other plans):

- BigQuery-specific function bodies whose precise behavior justifies
  semantic routing (`semantic-functions-compliance.plan.md`).
- Array / struct shapes that need lateral evaluation
  (`array-struct-semantic-path.plan.md`).
- DML evaluation (`dml-local-executor.plan.md`).
- Correlated subquery evaluation (`cte-subquery-routing.plan.md`).
- Procedure / scripting (`procedural-scripting-executor.plan.md`).

## Implementation Plan

1. Add `backend/engine/semantic/` with `executor.{h,cc}`,
   `value.{h,cc}`, `eval_expr.{h,cc}`. `Value` carries the BigQuery
   type tag plus the value union; `EvalExpr` is a switch on
   `ResolvedExpr::node_kind()`.
2. Implement scalar-only SELECT end-to-end: classifier routes
   `SELECT <expr-only>` to the semantic executor; the executor
   evaluates each output column and emits a one-row Arrow batch.
3. Implement the BigQuery type system in `Value`. Use Arrow's
   in-memory layout for arrays / structs so the result-marshaling
   path is unchanged.
4. Implement parameter binding using the same parameter-ordering
   contract as the DuckDB fast path.
5. Implement `CASE`, `IF`, `COALESCE`, `IFNULL`, `NULLIF` with the
   BigQuery NULL evaluation order.
6. Implement strict arithmetic on each numeric type, including the
   `SAFE.*` switch for swap-error-with-NULL behavior.
7. Implement the BigQuery error surface: each evaluation failure
   carries a structured error with the right `reason` token, and
   the coordinator maps it onto the BigQuery REST error envelope.
8. Add a `RowSource` adapter that wraps a `DuckDbExecutor` result
   so composable shapes (a semantic-executor query whose `FROM` is a
   DuckDB-fast-path scan) can stream rows through.

## Tests

- Per-node unit tests for `EvalExpr`.
- Per-type unit tests for `Value` arithmetic and NULL behavior.
- Engine-level integration tests under `gateway/e2e/` covering:
  - `SELECT 1`, `SELECT 1 + 2`, `SELECT NULL + 1`.
  - `SELECT @p`, `SELECT @p + 1` (parameter binding).
  - `SELECT CASE WHEN ... THEN ... END`.
  - `SELECT SAFE_ADD(1, 9223372036854775807)` (overflow → NULL).
  - `SELECT 1 / 0` (error surface).
- Conformance fixtures under `conformance/fixtures/scalar/` pinning
  the route label to `semantic_executor`.

## Done Criteria

- Scalar-only `SELECT` no longer returns `UNIMPLEMENTED`.
- `Value` covers every BigQuery primitive type plus ARRAY and
  STRUCT.
- Strict NULL / overflow behavior matches BigQuery on the tested
  edge cases.
- The `RowSource` adapter lets a semantic-executor query consume
  a DuckDB fast-path scan as a `FROM` clause.
- `semantic-functions-compliance.plan.md` and
  `array-struct-semantic-path.plan.md` have a working executor to
  build on.
