---
name: duckdb-storage-core
overview: "Phase 3e: DuckDBStorage core — connection, layout, metadata sidecar."
todos:
  - id: duckdb-storage-impl
    content: "Implement duckdb_storage.{h,cc}: open connection per --data_dir; datasets as schemas/dirs; tables as Parquet"
    status: completed
  - id: metadata-sidecar
    content: "Per-table table.meta.json for description, labels, friendlyName, etag"
    status: completed
  - id: storage-factory
    content: "Wire --storage=duckdb factory; default --data_dir=$HOME/.bigquery-emulator"
    status: completed
isProject: false
---

# Phase 3e: Duckdb Storage Core

## Prerequisites

- [vendor-duckdb_n9c0d1e2.plan.md](vendor-duckdb_n9c0d1e2.plan.md)
- [engine-cli-scaffold_m8b9c0d1.plan.md](engine-cli-scaffold_m8b9c0d1.plan.md)

## Verification

```bash
./build-out/emulator_main --storage=duckdb --data_dir=/tmp/bqemu --host_port localhost:9060
```

## Done criteria

- --storage=duckdb selects DuckDBStorage

## Next plan(s)

- [duckdb-storage-ddl_p1e2f3a4.plan.md](duckdb-storage-ddl_p1e2f3a4.plan.md)
