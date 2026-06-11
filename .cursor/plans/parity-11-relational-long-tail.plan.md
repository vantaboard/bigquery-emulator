---
name: Parity 11 — Relational long tail
overview: Clear the low-frequency deferred relational edges - window frames with non-literal bounds and RANGE over non-numeric ORDER BY, TABLESAMPLE repeatable seeds / stratified / weights, recursive CTE WITH DEPTH, and the LIKE ANY / ALL subquery family.
depends_on: [parity-01-unnest-lateral-correlated]
est_effort: ~1 week
isProject: true
todos:
  - id: window-frames
    content: "ResolvedWindowFrame non-literal bound offsets + RANGE over non-numeric ORDER BY (DATE/TIMESTAMP ranges): lower natively where DuckDB's RANGE with INTERVAL offsets is exact; otherwise semantic executor; also clear analytic hint/collation-list bails where they are no-ops."
    status: completed
  - id: tablesample
    content: "ResolvedSampleScan deferred matrix: REPEATABLE(seed) via DuckDB's REPEATABLE, plus the method/unit combos currently bailing; stratified partition lists + weight columns route duckdb_rewrite or stay documented-deferred if DuckDB has no exact analog - decide per shape and record it."
    status: completed
  - id: with-depth
    content: "Recursive CTE WITH DEPTH (recursion_depth_modifier): emit the depth pseudo-column through the recursive union arms (duckdb_rewrite: thread a depth counter column through EmitRecursiveScan)."
    status: completed
  - id: like-any-all
    content: "LIKE ANY / LIKE ALL subquery + list forms: currently on the empty-string fallback; lower to conjunction/disjunction of LIKEs for list form, semantic-executor per-outer-row eval for the subquery form (reuses parity-01 primitive)."
    status: completed
  - id: fixtures-trackers
    content: Fixtures under window/ sample/ cte_subquery/; flip SHAPE_TRACKER rows (ResolvedWindowFrame*, ResolvedSampleScan, ResolvedRecursiveScan WITH DEPTH note, ResolvedSubqueryExpr LIKE ANY/ALL note); update node_dispositions.yaml.
    status: completed
---

# Parity 11 — Relational long tail

## Why

Each of these is explicitly deferred in SHAPE_TRACKER with a named
target route, but none blocks common workloads — hence late ordering:

- `ResolvedAnalyticScan` / `ResolvedWindowFrame*`: *"Frame bounds with
  non-literal offsets ... reroute to semantic_executor (planned)"*;
  *"RANGE over non-numeric ORDER BY"* likewise.
- `ResolvedSampleScan`: *"Repeatable seeds, weight columns, stratified
  partition lists, and unsupported method/unit combos reroute to
  duckdb_rewrite (planned)"*.
- `ResolvedRecursiveScan`: *"WITH DEPTH (recursion_depth_modifier)
  still surfaces UNIMPLEMENTED"*.
- `ResolvedSubqueryExpr`: *"LIKE ANY/ALL stays on the empty-string
  fallback (deferred)"*.

## Key files

- [`backend/engine/duckdb/transpiler/`](../../backend/engine/duckdb/transpiler/) — `EmitFrameBound`, `EmitRecursiveScan`, sample emit
- [`backend/engine/coordinator/route_classifier_visitor.cc`](../../backend/engine/coordinator/route_classifier_visitor.cc)
- [`backend/engine/semantic/`](../../backend/engine/semantic/) — window/subquery eval where DuckDB is not exact
- `conformance/fixtures/window/`, `conformance/fixtures/sample/`, `conformance/fixtures/cte_subquery/`

## Steps

1. Probe DuckDB exactness per shape before choosing routes (same
   discipline as plan 03): DuckDB supports `RANGE BETWEEN INTERVAL ...`
   and `USING SAMPLE ... REPEATABLE (seed)` — several of these may be
   native lowerings, which is much cheaper than semantic work.
2. Land native lowerings first; route the rest to the semantic
   executor only where BigQuery exactness demands it.
3. `WITH DEPTH` is contained transpiler work on the existing
   `recursive_cte_stack_` machinery.
4. LIKE ANY/ALL: list form is a transpiler expansion; subquery form
   reuses the parity-01 outer-row primitive.
5. Where a shape has no exact DuckDB analog and semantic cost is
   unjustified (e.g. stratified sampling weights), keep it deferred
   but convert the silent empty-string bail into a structured
   UNIMPLEMENTED naming the family, and say so in the tracker row.

## Verify

```bash
task emulator:build-engine:bazel
task conformance:run
task lint:dispositions
task bazel:shutdown && task bazel:status
```

## Out of scope

- `ResolvedMatchRecognizeScan` — plan 12
- DP aggregate scans — `unsupported` by design
- `EXPLAIN` — `unsupported` posture stands (BigQuery semantics differ from DuckDB)
