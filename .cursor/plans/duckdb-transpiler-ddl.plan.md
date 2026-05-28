# DuckDB Transpiler DDL Support

## Goal

Promote DDL-shaped resolved statements from `skiplist` to `done` by defining
which operations lower to DuckDB SQL and which route through storage/catalog
commands with equivalent behavior.

## Tracker Rows

- `ResolvedCreateTableStmt` from `skiplist` to `done`.
- `ResolvedCreateTableAsSelectStmt` from `skiplist` to `done`.
- `ResolvedCreateViewStmt` from `skiplist` to `done`.
- `ResolvedCreateMaterializedViewStmt` from `skiplist` to `done`.
- `ResolvedCreateFunctionStmt` from `skiplist` to `done` if implemented as DDL
  rather than the UDF/TVF plan.
- `ResolvedCreateTableFunctionStmt` from `skiplist` to `done` if implemented as
  DDL rather than the UDF/TVF plan.
- `ResolvedDropStmt` and `ResolvedDropFunctionStmt` from `skiplist` to `done`.
- `ResolvedExplainStmt` from `skiplist` to `done`.
- `ResolvedAnalyzeStmt` from `skiplist` to `done`.
- `ResolvedAuxLoadDataStmt` from `skiplist` to `done`.
- `ResolvedExportDataStmt` from `skiplist` to `done`.
- `ResolvedCatalogColumnRef` from `skiplist` to `done` where DDL expressions
  require it.
- `ResolvedSequence` from `skiplist` to `done` if sequence objects become part
  of the supported catalog.

## Implementation Plan

1. Decide the execution boundary: pure query DDL can emit DuckDB SQL, while
   catalog/storage mutations may need an engine command path instead of a
   string-only transpiler result.
2. Add statement dispatch in `Transpile` for the promoted DDL node kinds.
3. Lower create/drop table and CTAS through storage-aware DuckDB operations so
   the emulator catalog and on-disk data stay consistent.
4. Lower views/materialized views only after deciding persistence and refresh
   semantics in the storage layer.
5. Implement `EXPLAIN` by running the child query through the transpiler and
   returning DuckDB plan output only where the public BigQuery shape can be
   reproduced.
6. Implement LOAD DATA and EXPORT DATA through gateway/storage integration if
   they cannot be represented as direct DuckDB SQL.
7. Add dry-run and error-message parity tests for unsupported DDL options.

## Tests

- Create/drop table, CTAS, create/drop view, and analyze smoke tests.
- Catalog consistency tests after each mutating statement.
- LOAD and EXPORT tests with local files or fake GCS samples.
- Fallback/error tests for unsupported BigQuery DDL options.

## Done Criteria

- DDL rows are `done` with notes that identify the engine/storage boundary.
- Mutating DDL updates both DuckDB storage and BigQuery emulator metadata.
- Unsupported options produce deliberate errors, not silent partial DDL.
