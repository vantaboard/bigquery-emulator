---
name: ""
overview: ""
todos: []
isProject: false
---

# CTE / Subquery Routing

## Goal

Implement CTE and subquery routing. Non-recursive CTEs and
non-correlated subqueries route to the DuckDB fast path; correlated
subqueries and CTEs whose evaluation order requires BigQuery-exact
semantics route to the semantic executor.

## Background

Today:

- `ResolvedWithScan` / `ResolvedWithRefScan` are `(planned)`
  `duckdb_native` — DuckDB supports `WITH ... AS (...)`
  natively, so the lowering is mostly bookkeeping.
- `ResolvedSubqueryExpr` (scalar / IN / EXISTS / ARRAY subqueries)
  is `(planned)` `duckdb_native` for non-correlated forms,
  `(planned)` `semantic_executor` for correlated forms.
- `ResolvedRecursiveScan` / `ResolvedRecursiveRefScan` are
  `(planned)` `duckdb_rewrite` (DuckDB `WITH RECURSIVE`); they live
  under `advanced-relational-routing.plan.md` because their
  semantics are sensitive enough to warrant a dedicated landing.

## Dependencies

- `engine-router-foundation.plan.md` so the classifier can decide
  per-shape (correlation status determines route).
- `semantic-executor-core.plan.md` so correlated forms have a place
  to land.
- `execution-disposition-registry.plan.md` so the disposition rows
  flip from `(planned)` to landed.

## Scope

Shapes this plan covers:

- `ResolvedWithScan` (non-recursive).
- `ResolvedWithRefScan`.
- `ResolvedSubqueryExpr` non-correlated forms (scalar / IN /
  EXISTS / ARRAY).
- `ResolvedSubqueryExpr` correlated forms (when the subquery
  references columns from an outer scan).

## Implementation Plan

1. Teach the route classifier to inspect
   `ResolvedSubqueryExpr::parameter_list()`: non-empty parameter
   list means correlated, which forces the surrounding query to
   `semantic_executor`.
2. Implement `EmitWithScan` and `EmitWithRefScan` in the DuckDB
   transpiler so non-recursive CTEs lower to DuckDB `WITH ... AS
   (...)`. Preserve column-name remapping through the
   `output_column_list()` shape.
3. Implement `EmitSubqueryExpr` for non-correlated scalar / IN /
   EXISTS / ARRAY forms in the DuckDB transpiler.
4. Implement correlated subquery evaluation in the semantic
   executor: outer scan provides rows, each outer row drives a
   per-row evaluation of the inner subquery, the result is collected
   per `ResolvedSubqueryType` (`SCALAR` / `IN` / `EXISTS` /
   `ARRAY`).
5. Update `SHAPE_TRACKER.md` rows as each shape lands.

## Tests

- Unit tests for the classifier's correlation detection.
- Per-shape transpiler unit tests for the DuckDB-route shapes.
- Per-shape semantic-executor unit tests for the correlated forms.
- Engine-level integration tests in `gateway/e2e/`:
  - Non-recursive `WITH a AS (...) SELECT ... FROM a`.
  - Multiple CTE bindings.
  - Scalar subquery `SELECT (SELECT MAX(x) FROM t) AS m`.
  - IN subquery, EXISTS subquery, ARRAY subquery.
  - Correlated scalar subquery
    `SELECT (SELECT COUNT(*) FROM t2 WHERE t2.k = t1.k) FROM t1`.
- Conformance fixtures under `conformance/fixtures/cte_subquery/`
  pinning the route label.

## Done Criteria

- Non-recursive CTE / CTE-ref shapes land on `duckdb_native` and
  no longer surface `UNIMPLEMENTED`.
- Non-correlated subqueries land on `duckdb_native`.
- Correlated subqueries route to `semantic_executor` and produce
  the right BigQuery result per `ResolvedSubqueryType`.
- Conformance fixtures pin the expected route per shape so a future
  classifier change cannot silently demote a correlated subquery
  back to the fast path.

## Status (post-subagent-10)

Landed (subagent 10, commits `1e04a1c` / `9863802` / `e0b5de9`):

- **Family 1 — `ResolvedWithScan` + `ResolvedWithRefScan`** on
  `duckdb_native`. `EmitWithScan` emits the non-recursive CTE
  shape, projecting each CTE entry's columns onto positional
  anchor names (`_cte_<idx>`). `EmitWithRefScan` renames the
  anchors back to the analyzer's per-reference column names so a
  CTE referenced multiple times resolves cleanly. Recursive CTEs
  (`recursive==true`) bail to "" -- owned by plan 11
  (`advanced-relational-routing.plan.md`).
- **Family 2 — non-correlated `ResolvedSubqueryExpr`** on
  `duckdb_native`. `EmitSubqueryExpr` lowers SCALAR / IN / EXISTS
  / ARRAY directly to DuckDB's native subquery surface
  (`(<sub>)`, `(<lhs> IN (<sub>))`, `EXISTS (<sub>)`,
  `ARRAY(<sub>)`). LIKE ANY / LIKE ALL / NOT LIKE ANY / NOT LIKE
  ALL stay on the empty-string fallback (out of plan-10 scope).
- **Family 3 — classifier correlation detection.**
  `ClassifierVisitor::VisitResolvedSubqueryExpr` inspects
  `ResolvedSubqueryExpr::parameter_list_size() > 0` and promotes
  the surrounding statement to `kSemanticExecutor`. The
  non-correlated forms stay on `kDuckdbNative` and lower via
  Family 2.

Deferred (stays planned in the semantic executor; the gateway
surfaces a clean `kNotImplemented` envelope via the executor
stub until the follow-up lands):

- **Family 4 — correlated `ResolvedSubqueryExpr` semantic
  executor.** The classifier promotion in Family 3 already
  routes correlated forms to `kSemanticExecutor`, but the
  executor itself does not yet evaluate a correlated subquery
  (the SCALAR `>1 row` raise, IN membership, EXISTS truth, ARRAY
  collection). **Blocker**: the executor needs an outer-row
  iteration primitive -- a row-by-row driver that re-binds
  `EvalContext::columns` per outer row (the foundation plan 8
  added in commit `9b2bde1` for the per-row column-ref binding
  side, but the driver loop itself does not yet exist) and
  invokes the inner subquery against the re-bound context. This
  same primitive is the missing piece for plan 8's deferred
  correlated `ResolvedJoinScan` lateral (Family 4 of
  `array-struct-semantic-path.plan.md` -- see commit `0ef7569`
  for the deferral note); landing the loop once will unblock
  both. **No silent approximation reason**: DuckDB's correlated-
  subquery decorrelation does not guarantee BigQuery's
  per-outer-row evaluation order or row-count error semantics for
  every shape (the SCALAR ">1 row" raise is on the inner scan's
  per-outer-row materialization, not the decorrelated join). A
  partial implementation that approximated via DuckDB would
  silently return the wrong answer on shapes the local
  interpreter exists to be authoritative on. Until the loop
  lands, the correlated route is a structured UNIMPLEMENTED, not
  a partial answer.
