---
name: engine-cli-scaffold
overview: "Phase 3c: engine scaffolds, CLI flags, CMake targets."
todos:
  - id: engine-scaffolds
    content: "Add reference_impl_engine and duckdb_engine returning UNIMPLEMENTED for all Engine methods"
    status: completed
  - id: cli-flags
    content: "Add --engine, --storage, --profile=ci|dev flags to emulator_main; wire factory"
    status: completed
  - id: cmake-targets
    content: "Update CMakeLists.txt linking backend/storage/memory, schema, engine into emulator_main"
    status: completed
isProject: false
---

# Phase 3c: Engine Cli Scaffold

## Prerequisites

- [memory-storage_l7a8b9c0.plan.md](memory-storage_l7a8b9c0.plan.md)

## Verification

```bash
./build-out/emulator_main --help
```

## Done criteria

- --help documents engine/storage/profile flags
- Engine scaffolds compile

## Next plan(s)

- [duckdb-storage-core_o0d1e2f3.plan.md](duckdb-storage-core_o0d1e2f3.plan.md)
- [catalog-grpc-cpp_q2f3a4b5.plan.md](catalog-grpc-cpp_q2f3a4b5.plan.md)
