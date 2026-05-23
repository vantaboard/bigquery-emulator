---
name: duckdb-persistent-storage
overview: "Phase 3 part 2: vendor libduckdb, implement DuckDBStorage backed by Parquet/Arrow files under --data_dir, and map BigQuery DDL operations to DuckDB schemas + on-disk layout."
todos:
  - id: vendor-duckdb
    content: "Add third_party/duckdb/ via pinned release tarball or git submodule; wire CMake FetchContent or Bazel external; document version pin in third_party/duckdb/VERSION"
    status: pending
  - id: duckdb-storage-impl
    content: "Implement backend/storage/duckdb/duckdb_storage.{h,cc}: open DuckDB connection per --data_dir; datasets as schemas or directory prefixes; tables as Parquet files"
    status: pending
  - id: metadata-sidecar
    content: "Per-table JSON sidecar (table.meta.json) for BigQuery metadata not in Parquet: description, labels, friendlyName, etag"
    status: pending
  - id: ddl-mapping
    content: "CreateTable writes empty Parquet with schema; DropTable removes Parquet + sidecar; AppendRows appends via DuckDB COPY or INSERT; ScanRows uses DuckDB read_parquet"
    status: pending
  - id: schema-duckdb-convert
    content: "Extend backend/schema/ with ToDuckDBType and FromDuckDBType for INT64, STRING, BOOL, FLOAT64, TIMESTAMP, DATE, NUMERIC, STRUCT, ARRAY"
    status: pending
  - id: partition-layout
    content: "Stub partitions/ subdirectory layout for date-partitioned tables (create dirs, document format; full partition pruning deferred)"
    status: pending
  - id: storage-factory
    content: "Wire --storage=duckdb in emulator_main factory to DuckDBStorage; default --data_dir=$HOME/.bigquery-emulator or ./data"
    status: pending
  - id: duckdb-storage-test
    content: "Integration test: create table, insert 100 rows, restart storage (new connection), scan 100 rows back"
    status: pending
isProject: false
---

# Phase 3 (part 2): DuckDB persistent storage

## Prerequisites

- [cpp-interfaces-memory-storage_2e3f4a5b.plan.md](cpp-interfaces-memory-storage_2e3f4a5b.plan.md) — Storage interface defined.

## Scope

**In:** DuckDB vendoring, DuckDBStorage, Parquet backing, DDL mapping, sidecar metadata.

**Out:** DuckDB query engine / transpiler (Plans 08–09). Partition pruning optimization.

## Key files

- `third_party/duckdb/` or CMake FetchContent
- `backend/storage/duckdb/duckdb_storage.{h,cc}`
- `backend/schema/schema.{h,cc}` — DuckDB type conversion
- `CMakeLists.txt`, `MODULE.bazel` (stub external dep)

## Open questions (from ROADMAP)

- Bazel vs CMake for DuckDB: start with CMake FetchContent; Bazel `http_archive` can follow.
- Pre-existing Parquet drop-in: ScanRows should discover unknown `.parquet` in `--data_dir` and expose as tables (stretch — document if deferred).

## Verification

```bash
cmake --build build-out --target emulator_main
rm -rf /tmp/bqemu-data
./build-out/emulator_main --storage=duckdb --data_dir=/tmp/bqemu-data --host_port localhost:9060 &
# run duckdb_storage integration test via ctest
```

## Done criteria

- `--storage=duckdb` selects DuckDBStorage; tables survive process restart.
- CreateTable + AppendRows + ScanRows round-trip for STRING/INT64 table.
- DuckDB version pinned and documented.

## Next plan

[catalog-grpc-tabledata-e2e_1a2b3c4d.plan.md](catalog-grpc-tabledata-e2e_1a2b3c4d.plan.md)
