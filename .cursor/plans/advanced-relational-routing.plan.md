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
