---
name: ""
overview: ""
todos: []
isProject: false
---

# Advanced Relational Routing

## Goal

Classify the advanced relational scans by route and land each
classification: recursive CTEs, PIVOT / UNPIVOT, MATCH_RECOGNIZE,
pipe operators, graph query scans, optimizer barriers, and deferred
computed columns. This replaces the retired
`duckdb-transpiler-advanced-relational.plan.md` with a
route-aware version.

## Background

The DuckDB-only roadmap planned to lower every advanced relational
scan through the DuckDB SQL fast path. That underestimated the
BigQuery semantics in this family. The current per-shape posture:

| Shape                              | Planned route          |
|------------------------------------|------------------------|
| `ResolvedRecursiveScan`            | `duckdb_rewrite`       |
| `ResolvedRecursiveRefScan`         | `duckdb_rewrite`       |
| `ResolvedPivotScan`                | `duckdb_rewrite`       |
| `ResolvedUnpivotScan`              | `duckdb_rewrite`       |
| `ResolvedMatchRecognizeScan`       | `semantic_executor`    |
| `ResolvedGroupRowsScan`            | `semantic_executor`    |
| `ResolvedBarrierScan`              | `semantic_executor`    |
| `ResolvedPipeIfScan`               | `semantic_executor`    |
| `ResolvedPipeForkScan`             | `semantic_executor`    |
| `ResolvedPipeTeeScan`              | `semantic_executor`    |
| `ResolvedPipeExportDataScan`       | `control_op`           |
| `ResolvedPipeCreateTableScan`      | `control_op`           |
| `ResolvedPipeInsertScan`           | `semantic_executor`    |
| `ResolvedGraph*Scan`               | `unsupported`          |
| `ResolvedDeferredComputedColumn`   | `semantic_executor`    |
| GROUPING SETS / ROLLUP / CUBE      | `duckdb_rewrite`       |

## Dependencies

- `engine-router-foundation.plan.md`.
- `semantic-executor-core.plan.md`.
- `control-op-executor.plan.md` (for pipe forms that route to
  control ops).
- `dml-local-executor.plan.md` (for `ResolvedPipeInsertScan`).

## Scope

This plan implements:

- Recursive CTE lowering through DuckDB `WITH RECURSIVE`
  (`duckdb_rewrite`).
- PIVOT / UNPIVOT lowering through DuckDB `PIVOT` / `UNPIVOT`
  (`duckdb_rewrite`).
- GROUPING SETS / ROLLUP / CUBE / `GROUPING()` lowering
  (`duckdb_rewrite`).
- MATCH_RECOGNIZE in the semantic executor (BigQuery-only pattern
  matching).
- Pipe operator scans: `PipeIf` / `PipeFork` / `PipeTee` (semantic
  executor), `PipeExportData` / `PipeCreateTable` (control op),
  `PipeInsert` (DML local executor).
- Optimizer barrier semantics in the semantic executor.
- Deferred computed column evaluation in the semantic executor.
- Graph query scans stay `unsupported`;
  `specialized-feature-policy.plan.md` documents the posture.

## Implementation Plan

1. Implement the `duckdb_rewrite` shapes first (recursive CTE,
   PIVOT / UNPIVOT, GROUPING SETS). They land in
   `backend/engine/duckdb/transpiler/` and reuse the DuckDB fast
   path's execution surface.
2. Implement the pipe-operator-to-control-op handlers
   (`PipeExportData`, `PipeCreateTable`) in
   `backend/engine/control/` (cross-link from
   `control-op-executor.plan.md`).
3. Implement `PipeInsert` in `backend/engine/semantic/dml/`
   (cross-link from `dml-local-executor.plan.md`).
4. Implement the semantic-executor pipe forms (`PipeIf`,
   `PipeFork`, `PipeTee`, `BarrierScan`).
5. Implement `MATCH_RECOGNIZE` in the semantic executor as a small
   row-state machine.
6. Implement `ResolvedDeferredComputedColumn` evaluation in the
   semantic executor.
7. Update `SHAPE_TRACKER.md` rows as each shape lands.
8. Confirm `ResolvedGraph*Scan` rows are documented as
   `unsupported` by `specialized-feature-policy.plan.md` with a
   BigQuery-shaped error message.

## Tests

- Per-shape unit tests in the matching package
  (transpiler / semantic / control).
- Engine-level integration tests under `gateway/e2e/`:
  - Recursive CTE (hierarchical traversal).
  - PIVOT and UNPIVOT.
  - GROUPING SETS / ROLLUP / CUBE with `GROUPING()`.
  - Pipe operators: `|> WHERE`, `|> SELECT`, `|> AGGREGATE`,
    `|> IF`, `|> FORK`.
  - `MATCH_RECOGNIZE` example from the BigQuery docs.
- Conformance fixtures under
  `conformance/fixtures/advanced_relational/` asserting the
  per-shape route label.

## Done Criteria

- Every shape in this plan's table has a non-`(planned)`
  disposition.
- `ResolvedGraph*Scan` rows continue to return a
  BigQuery-shaped `UNIMPLEMENTED` with a message that names the
  unsupported feature.
- Route-aware conformance fixtures pin each shape's route.

## Status (2026-06-01)

Landed family-by-family in the eleventh subagent of the
local-execution-roadmap (precedent: plans 4, 5, 7, 8, 9, 10).
Routing is the contract of each landing; full executor /
transpiler surface is implicitly deferred where noted.

| Family | Shapes | Disposition | Commit |
|--------|--------|-------------|--------|
| 1 | `ResolvedAggregateScan::grouping_set_list` / `rollup_column_list` / `grouping_call_list` (GROUPING SETS / ROLLUP / CUBE / `GROUPING()`) | `duckdb_rewrite` -- transpiler `EmitAggregateScan` lowers to DuckDB `GROUP BY GROUPING SETS` + `GROUPING()` calls. | (see git log; landed first) |
| 2 | `ResolvedBarrierScan` | `semantic_executor` -- `StripBarrierScans` transparently unwraps the optimizer barrier in `backend/engine/semantic/executor.cc`. | (see git log) |
| 3 | `ResolvedPivotScan` / `ResolvedUnpivotScan` | `duckdb_rewrite` -- analyzer's `REWRITE_PIVOT` / `REWRITE_UNPIVOT` are disabled so the raw scans reach the transpiler. `EmitPivotScan` lowers to conditional aggregation (`<agg> FILTER (WHERE <for_expr> = <pivot_value>)`); `EmitUnpivotScan` lowers to per-arg UNION ALL with `EXCLUDE NULLS` semantics. | (see git log) |
| 4 | `ResolvedRecursiveScan` / `ResolvedRecursiveRefScan` | `duckdb_rewrite` -- `EmitWithScan` pushes a `RecursiveCteContext` onto the transpiler's stack; `EmitRecursiveScan` lowers anchor + recursive arm via DuckDB `WITH RECURSIVE`; `EmitRecursiveRefScan` resolves the CTE name from the stack. Bails on `recursion_depth_modifier` (deferred). | (see git log) |
| 5 | `ResolvedDeferredComputedColumn` | `semantic_executor` -- disposition row promoted (no `status=planned`). Real deferred-evaluation semantics are deferred to a downstream subagent of this plan; the disposition gets the classifier routing to the right executor. | `b1adcdc` |
| 6 | `ResolvedPipeExportDataScan` / `ResolvedPipeCreateTableScan` | `control_op` -- coordinator disables `REWRITE_GENERALIZED_QUERY_STMT` and allows `RESOLVED_GENERALIZED_QUERY_STMT` so the pipe scans survive into the resolved tree. New translation units `backend/engine/control/pipe_{export_data,create_table}.{h,cc}` host the handlers (separate from the `control_op_executor.cc` lint-cap carve-out). Both handlers surface `UNIMPLEMENTED` naming the deferred follow-up (EXPORT DATA writer family / pipe-input transpiler surface). | `cf05e19` |
| 7 + 8 | `ResolvedGroupRowsScan` (`WITH GROUP ROWS`) + `ResolvedPipeIfScan` / `ResolvedPipeForkScan` / `ResolvedPipeTeeScan` | `semantic_executor` -- disposition rows promoted (no `status=planned`). The classifier routes any query containing one of these scans to the semantic executor, which surfaces the existing structured `kNotImplemented` until the per-group row-set evaluator + subpipeline executor land. | `c2d3fb9` |

Deferred to focused follow-up subagents of this plan:

- **`ResolvedMatchRecognizeScan` (Family 9).** The row-pattern
  state machine MATCH_RECOGNIZE describes is the most complex
  shape in this plan; landing the routing alone is not useful
  (the executor has no row-state machine to hand the routed
  shape to). The disposition row stays
  `semantic_executor plan=advanced-relational-routing.plan.md
  status=planned` so the classifier still rejects unsupported
  queries with the structured "not yet implemented" envelope
  and a future subagent owns the executor work without
  re-deriving the route. No silent approximation: the row
  surfaces UNIMPLEMENTED today through the classifier's
  `status=planned` short-circuit.
- **`ResolvedPipeExportDataScan` executor semantics (Family 6
  follow-up).** Needs Arrow / Parquet / CSV / JSON writers
  plus the fake-gcs / local-filesystem URI scheme dispatch
  the statement-form `EXPORT DATA` also needs. Tracked by
  `control-op-executor.plan.md` follow-up.
- **`ResolvedPipeCreateTableScan` executor semantics (Family 6
  follow-up).** Needs the transpiler to lower an arbitrary
  pipe-input scan into a syntactically-valid SELECT so the
  inner `ResolvedCreateTableAsSelectStmt` (whose `query`
  field is intentionally null in the pipe form) can be
  re-issued through the existing CTAS handler.
- **`ResolvedDeferredComputedColumn` evaluator (Family 5
  follow-up).** Routing is in place; the side-effect-aware
  evaluator (capturing errors into the companion
  `side_effect_column`) is a downstream subagent of this plan.

Audited, no change required:

- **`ResolvedPipeInsertScan` (Family 10).** The disposition
  row already points at `dml-local-executor.plan.md
  status=planned`; Family 8 of plan 9's deferrals owns the
  implementation. No re-implementation here.
- **`ResolvedGraph*Scan` family (Family 11).** Every row already
  points at `specialized-feature-policy.plan.md unsupported`;
  the `UnsupportedExecutor` (`backend/engine/coordinator/
  stub_executors.cc`) returns a BigQuery-shaped UNIMPLEMENTED
  naming the plan owner. No change needed.
