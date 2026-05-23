---
name: duckdb-transpiler-advanced
overview: "Phase 5.B part 2: expand transpiler to STRUCT/UNNEST/ARRAY, function disposition table with skiplist, Arrow result marshaling to REST rows, and DuckDB engine parity for common analytics queries."
todos:
  - id: struct-unnest
    content: "Implement Emit for ResolvedArrayScan (UNNEST), ResolvedStructFieldAccess, ResolvedCreateArray, ResolvedCreateStruct; document DuckDB LIST/STRUCT mapping quirks in SHAPE_TRACKER.md"
    status: pending
  - id: function-table
    content: "Add backend/engine/duckdb/transpiler/functions.yaml: BQ function → DuckDB equivalent | skiplist | fallback; loader used by ResolvedFunctionCall emitter"
    status: pending
  - id: emit-window
    content: "Implement Emit for ResolvedAnalyticScan (ROW_NUMBER, RANK, SUM OVER PARTITION BY) for common window shapes"
    status: pending
  - id: arrow-results
    content: "ExecuteQuery on DuckDB engine: run transpiled SQL, fetch Arrow RecordBatch stream, convert to proto QueryResultRow batches"
    status: pending
  - id: arrow-to-wire
    content: "Add backend/schema/arrow_to_bq.{h,cc} or reuse gateway/bqtypes/wire.go path via proto — map Arrow types to StandardSqlDataType wire cells"
    status: pending
  - id: parity-tests
    content: "Add tests comparing reference_impl vs duckdb engine results for same query on identical seed data (allow numeric tolerance for FLOAT)"
    status: pending
  - id: skiplist-fallback
    content: "Wire skiplist functions to trigger ReferenceImplEngine fallback transparently; log fallback reason at debug level"
    status: pending
  - id: e2e-analytics
    content: "E2E: UNNEST array column, STRUCT field access, window function query on DuckDB engine"
    status: pending
isProject: false
---

# Phase 5.B (part 2): DuckDB transpiler advanced

## Prerequisites

- [duckdb-transpiler-core_3a4b5c6d.plan.md](duckdb-transpiler-core_3a4b5c6d.plan.md)
- [reference-impl-execution_9c0d1e2f.plan.md](reference-impl-execution_9c0d1e2f.plan.md) — wire marshaling + fallback target

## Scope

**In:** STRUCT/UNNEST/ARRAY, function disposition, window basics, Arrow pipeline, engine parity tests.

**Out:** MERGE/upsert (Plan 10). Full BigQuery function surface. Storage Read API (Plan 11).

## Key files

- `backend/engine/duckdb/transpiler/` — extend visitor
- `backend/engine/duckdb/transpiler/functions.yaml`
- `backend/schema/arrow_to_bq.{h,cc}`
- `backend/engine/duckdb/duckdb_engine.{h,cc}`

## Verification

```bash
go test -tags=integration ./gateway/e2e/... -run DuckDBParity
# manual UNNEST query against seeded table with REPEATED field
```

## Done criteria

- functions.yaml covers ≥50 common BQ functions with disposition.
- UNNEST + STRUCT queries work or fall back cleanly.
- DuckDB engine returns same REST row shape as reference impl for parity test suite.
- SHAPE_TRACKER.md updated with advanced nodes.

## Next plans

- [dml-ddl-statements_b1c2d3e4.plan.md](dml-ddl-statements_b1c2d3e4.plan.md)
- [storage-read-api_f5a6b7c8.plan.md](storage-read-api_f5a6b7c8.plan.md)
