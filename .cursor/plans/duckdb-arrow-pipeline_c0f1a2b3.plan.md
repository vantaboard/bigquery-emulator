---
name: duckdb-arrow-pipeline
overview: "Phase 5l: DuckDB Arrow result fetch and conversion to proto/wire cells."
todos:
  - id: arrow-results
    content: "DuckDB ExecuteQuery: fetch Arrow RecordBatch stream, convert to QueryResultRow batches"
    status: pending
  - id: arrow-to-wire
    content: "Add arrow_to_bq.{h,cc} or proto path mapping Arrow types to StandardSqlDataType wire cells"
    status: pending
isProject: false
---

# Phase 5l: Duckdb Arrow Pipeline

## Prerequisites

- [transpiler-functions-window_b9e0f1a2.plan.md](transpiler-functions-window_b9e0f1a2.plan.md)
- [wire-marshal-go_z1c2d3e4.plan.md](wire-marshal-go_z1c2d3e4.plan.md)

## Verification

```bash
go test -tags=integration ./gateway/e2e/... -run DuckDBArrow
```

## Done criteria

- DuckDB engine returns REST row shape via Arrow path

## Next plan(s)

- [duckdb-parity-e2e_d1a2b3c4.plan.md](duckdb-parity-e2e_d1a2b3c4.plan.md)
