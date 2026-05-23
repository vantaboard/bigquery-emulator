---
name: cpp-interfaces-memory-storage
overview: "Phase 3 part 1: define pluggable Storage and Engine C++ interfaces, implement InMemoryStorage with schema conversion, add --engine/--storage/--profile flags to emulator_main, and scaffold empty Reference Impl / DuckDB engine shells."
todos:
  - id: storage-interface
    content: "Replace backend/storage/storage.h placeholder with Storage abstract class: CreateDataset, DropDataset, CreateTable, DropTable, GetSchema, AppendRows, ScanRows returning row iterator or Arrow RecordBatch stream"
    status: pending
  - id: engine-interface
    content: "Add backend/engine/engine.h: Analyze, DryRun, ExecuteQuery taking resolved AST handle + Storage catalog; return RowSource or error"
    status: pending
  - id: schema-types
    content: "Add backend/schema/schema.h + conversions between proto FieldSchema, BigQuery TableSchema JSON shape, and internal ColumnType enum"
    status: pending
  - id: memory-storage
    content: "Implement backend/storage/memory/in_memory_storage.{h,cc}: thread-safe map of dataset→table→rows; AppendRows accepts JSON-like row maps from proto"
    status: pending
  - id: memory-storage-test
    content: "Add backend/storage/memory/in_memory_storage_test.cc: create table, append 3 rows, scan back, verify schema round-trip"
    status: pending
  - id: engine-scaffolds
    content: "Add backend/engine/reference_impl/reference_impl_engine.{h,cc} and backend/engine/duckdb/duckdb_engine.{h,cc} returning UNIMPLEMENTED for all Engine methods"
    status: pending
  - id: cli-flags
    content: "Add emulator_main flags: --engine=reference_impl|duckdb, --storage=memory|duckdb, --profile=ci|dev (ci=reference_impl+memory, dev=duckdb+duckdb); wire factory in frontend/server"
    status: pending
  - id: cmake-targets
    content: "Update CMakeLists.txt with backend/storage/memory, backend/schema, backend/engine libraries linked into emulator_main"
    status: pending
isProject: false
---

# Phase 3 (part 1): C++ interfaces + in-memory storage

## Prerequisites

- [grpc-contract-go-cpp_8a9b0c1d.plan.md](grpc-contract-go-cpp_8a9b0c1d.plan.md) — proto FieldSchema types exist.

## Scope

**In:** Storage/Engine interfaces, InMemoryStorage, schema conversion, CLI flags, engine scaffolds.

**Out:** DuckDB storage (Plan 04). Catalog gRPC handler wiring (Plan 05). GoogleSQL Analyzer (Plan 06).

## Key files

- `backend/storage/storage.h`
- `backend/storage/memory/in_memory_storage.{h,cc}`
- `backend/engine/engine.h`
- `backend/engine/reference_impl/reference_impl_engine.{h,cc}`
- `backend/engine/duckdb/duckdb_engine.{h,cc}`
- `backend/schema/schema.{h,cc}`
- `binaries/emulator_main/main.cc` — flag parsing
- `CMakeLists.txt`

## Design constraints (from ROADMAP)

- Storage signatures must **not** leak engine-specific types (no googlesql::Value in Storage API).
- Default profile: `(Reference Impl, In-Memory)` via `--profile=ci`.
- Engine and storage choices are independent.

## Verification

```bash
cmake --build build-out --target emulator_main
./build-out/emulator_main --host_port localhost:9060 --profile=ci &
# C++ unit tests if using gtest:
ctest --test-dir build-out -R in_memory_storage
```

## Done criteria

- InMemoryStorage unit test passes.
- `emulator_main --help` documents engine/storage/profile flags.
- Engine scaffolds compile and link.
- Storage can hold a table with STRING+INT64 columns and return rows via ScanRows.

## Next plan

[catalog-grpc-tabledata-e2e_1a2b3c4d.plan.md](catalog-grpc-tabledata-e2e_1a2b3c4d.plan.md) after Plan 04 optional (memory-only path works without DuckDB).

Parallel: [duckdb-persistent-storage_6c7d8e9f.plan.md](duckdb-persistent-storage_6c7d8e9f.plan.md)
