---
name: duckdb-storage-ddl
overview: "Phase 3f: DuckDB DDL mapping, type conversion, persistence test."
todos:
  - id: ddl-mapping
    content: "CreateTable writes empty Parquet; DropTable removes files; AppendRows via DuckDB INSERT; ScanRows via read_parquet"
    status: pending
  - id: schema-duckdb-convert
    content: "Add ToDuckDBType/FromDuckDBType for INT64, STRING, BOOL, FLOAT64, TIMESTAMP, DATE, NUMERIC, STRUCT, ARRAY"
    status: pending
  - id: duckdb-storage-test
    content: "Integration test: create table, insert 100 rows, restart, scan 100 rows back"
    status: pending
isProject: false
---

# Phase 3f: Duckdb Storage Ddl

## Prerequisites

- [duckdb-storage-core_o0d1e2f3.plan.md](duckdb-storage-core_o0d1e2f3.plan.md)

## Verification

```bash
ctest --test-dir build-out -R duckdb_storage
```

## Done criteria

- Tables survive process restart
- CreateTable+AppendRows+ScanRows round-trip

## Next plan(s)

- [catalog-grpc-cpp_q2f3a4b5.plan.md](catalog-grpc-cpp_q2f3a4b5.plan.md)
