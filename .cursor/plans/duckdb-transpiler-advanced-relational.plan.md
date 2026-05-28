# DuckDB Transpiler Advanced Relational Support

## Goal

Promote advanced relational scan families that are intentionally outside the
current SELECT baseline.

## Tracker Rows

- `ResolvedRecursiveScan` and `ResolvedRecursiveRefScan` from `skiplist` to
  `done`.
- `ResolvedPivotScan` and `ResolvedUnpivotScan` from `skiplist` to `done`.
- `ResolvedMatchRecognizeScan` from `skiplist` to `done`.
- `ResolvedGroupRowsScan` from `skiplist` to `done`.
- `ResolvedDeferredComputedColumn` from `skiplist` to `done`.
- `ResolvedBarrierScan` from `skiplist` to `done`.
- `ResolvedPipeIfScan`, `ResolvedPipeForkScan`, `ResolvedPipeTeeScan`,
  `ResolvedPipeExportDataScan`, `ResolvedPipeCreateTableScan`, and
  `ResolvedPipeInsertScan` from `skiplist` to `done`.
- `ResolvedGraph*Scan` from `skiplist` to `done`.

## Implementation Plan

1. Implement recursive CTE support after non-recursive CTE scope handling is
   stable. Use DuckDB `WITH RECURSIVE` only when termination and column typing
   match BigQuery.
2. Implement `PIVOT` and `UNPIVOT` through DuckDB native syntax or explicit
   aggregate/projection rewrites.
3. Evaluate `MATCH_RECOGNIZE` against DuckDB support. If no native equivalent
   exists, build a staged relational rewrite only for well-scoped subsets.
4. Implement pipe-operator scans as syntax sugar over existing relational nodes
   once each target operation is available.
5. Treat graph query scans as a separate feature area with catalog metadata,
   graph schema DDL, and graph pattern execution tests.
6. Preserve optimizer barrier semantics only if DuckDB planning can be nudged
   without changing query results; otherwise keep it as a no-op with tests that
   prove semantic equivalence.

## Tests

- Recursive CTE base/recursive terms and cycle-sensitive cases.
- Pivot/unpivot result shape and null behavior.
- Pipe operator forms mapped onto SELECT, EXPORT, CREATE TABLE, and INSERT.
- Graph query smoke tests after graph schema support exists.

## Done Criteria

- Advanced relational rows are `done` or split into narrower subfamily rows with
  explicit tracker notes.
- Each promoted family has at least one conformance test pinned to DuckDB.
- BigQuery-only syntax without DuckDB parity is rewritten deliberately, not
  approximated.
