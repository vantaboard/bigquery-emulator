---
name: vendor-duckdb
overview: "Phase 3d: pin and vendor libduckdb for CMake build."
todos:
  - id: vendor-duckdb
    content: "Add third_party/duckdb via pinned tarball or submodule; CMake FetchContent; document VERSION pin"
    status: completed
  - id: cmake-duckdb-link
    content: "Wire DuckDB into CMakeLists.txt as optional target; CI builds with duckdb enabled"
    status: completed
isProject: false
---

# Phase 3d: Vendor Duckdb

## Prerequisites

- [cpp-interfaces_k6f7a8b9.plan.md](cpp-interfaces_k6f7a8b9.plan.md)

## Verification

```bash
cmake --build build-out --target duckdb_storage 2>/dev/null || cmake --build build-out
```

## Done criteria

- DuckDB version pinned and documented
- Build links libduckdb

## Next plan(s)

- [duckdb-storage-core_o0d1e2f3.plan.md](duckdb-storage-core_o0d1e2f3.plan.md)
