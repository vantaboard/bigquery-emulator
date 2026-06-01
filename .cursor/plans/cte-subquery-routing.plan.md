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
