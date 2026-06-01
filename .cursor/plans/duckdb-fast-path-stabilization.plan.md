---
name: ""
overview: ""
todos: []
isProject: false
---

# DuckDB Fast Path Stabilization

## Goal

Preserve every DuckDB-fast-path win that landed during the
DuckDB-only era, and tighten the conformance bar on the shapes that
already route `duckdb_native` / `duckdb_rewrite`. This plan does not
expand the fast path — new BigQuery-only work goes to the polyfill
library or the semantic executor — it makes sure the fast path stays
correct as the surrounding routes land.

## Background

The DuckDB fast path covers a substantial OLAP surface today:

- Statement: `ResolvedQueryStmt`.
- Scans: `ResolvedTableScan`, `ResolvedSingleRowScan`,
  `ResolvedProjectScan`, `ResolvedFilterScan`,
  `ResolvedJoinScan` (INNER / LEFT / RIGHT / FULL / CROSS),
  `ResolvedAggregateScan` (baseline),
  `ResolvedOrderByScan`, `ResolvedLimitOffsetScan`,
  `ResolvedSetOperationScan`,
  `ResolvedAnalyticScan` (subset),
  `ResolvedSampleScan` (subset),
  `ResolvedArrayScan` (standalone UNNEST).
- Expressions: `ResolvedLiteral`, `ResolvedParameter`,
  `ResolvedColumnRef`, `ResolvedFunctionCall` (YAML-mapped subset),
  `ResolvedAggregateFunctionCall` (baseline),
  `ResolvedAnalyticFunctionCall` (window-function subset),
  `ResolvedCast` (whitelisted type matrix),
  `ResolvedMakeStruct` / `ResolvedGetStructField`,
  `ResolvedGetJsonField`, `ResolvedWithExpr`.
- Per-row helpers: `ResolvedOutputColumn`, `ResolvedComputedColumn`,
  `ResolvedOrderByItem`, `ResolvedWindowFrame*` /
  `ResolvedAnalyticFunctionGroup`, `ResolvedSetOperationItem`,
  `ResolvedFunctionArgument` (expr-only subset).
- Result marshaling: DuckDB Arrow `RecordBatch` → BigQuery REST
  `f`/`v` JSON; native Arrow on the Storage Read API.

## Scope

- Conformance coverage for every fast-path shape: at least one
  fixture per supported `ResolvedAST` node kind that asserts the
  route label is `duckdb_native` (after
  `conformance-routing-matrix.plan.md` lands).
- Stricter typing on the seams where the fast path silently degrades
  today (empty-string `""` fallback inside `Emit*` methods).
- Migration of every `kFallback` function row whose semantics
  actually match DuckDB into `duckdb_native` / `duckdb_rewrite` so
  the polyfill library inherits a clean baseline.

Out of scope (other plans):

- New BigQuery functions that need a polyfill
  (`duckdb-polyfill-udf-library.plan.md`).
- Any shape that currently surfaces `UNIMPLEMENTED`
  (`semantic-executor-core.plan.md`,
  `array-struct-semantic-path.plan.md`,
  `cte-subquery-routing.plan.md`,
  `advanced-relational-routing.plan.md`).

## Dependencies

- `execution-disposition-registry.plan.md` (so the YAML rows can be
  edited in their new vocabulary).
- `engine-router-foundation.plan.md` (so the fast path is reached
  through the classifier, not the implicit fallthrough).

## Implementation Plan

1. Audit every `Emit*` method in
   `backend/engine/duckdb/transpiler/transpiler.cc` and record:
   - Which child shape failures cause the method to return `""`.
   - Whether the returned `""` would actually route the surrounding
     query to `unsupported` once the classifier is in place.
2. For each `""` failure that should be classified upstream, move
   the disposition into `node_dispositions.yaml` instead of leaving
   it as a runtime gate. (Example: `ResolvedJoinScan` with
   `is_lateral=true` should route `semantic_executor` at
   classification time, not return `""` at emit time.)
3. Audit `functions.yaml`; move every `kFallback` whose DuckDB
   target function already exists with matching semantics into
   `duckdb_native` (or `duckdb_rewrite` when the rewrite is purely
   structural). Document each migration in the function row's `notes`.
4. Add per-shape conformance fixtures under `conformance/fixtures/`:
   - One fixture per supported `ResolvedAST` node kind, asserting
     the route label is `duckdb_native` (once
     `conformance-routing-matrix.plan.md` lands).
   - One fixture per non-trivial `duckdb_rewrite` shape (struct
     literal anonymous-field rewrite; BigQuery JSON operator → DuckDB
     JSON operator; `MERGE` straightforward branches; ...).
5. Add a focused regression suite for the fast path's known sharp
   edges: NaN ordering, NULL-equality in joins, integer overflow,
   nested STRUCT literal ordering, anonymous-field STRUCT
   round-tripping.

## Tests

- The conformance fixtures above run in CI under
  `task conformance:run`.
- A new `task conformance:fastpath` runs only the
  `duckdb_native` / `duckdb_rewrite` fixtures (~2 minutes).
- Existing third-party client samples
  (`task thirdparty:*`) continue to pass against the coordinator's
  fast-path route.

## Done Criteria

- Every fast-path row in `SHAPE_TRACKER.md` has at least one
  conformance fixture pinning it on the `duckdb_native` /
  `duckdb_rewrite` route.
- No `Emit*` method returns `""` for a child failure that
  classification could have caught earlier.
- `functions.yaml` has no `kFallback` row whose DuckDB target already
  matches BigQuery semantics; every remaining non-`duckdb_*` row
  carries a planned route from one of the new-route plans.
- `task conformance:fastpath` runs in CI on every PR.
