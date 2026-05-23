# third_party/duckdb

Vendoring shim for [DuckDB](https://duckdb.org). The pin and SHA-256
hashes live in [`VERSION`](./VERSION); the CMake plumbing that fetches
the upstream `libduckdb-linux-{amd64,arm64}.zip` archive lives in
[`CMakeLists.txt`](./CMakeLists.txt). The top-level `CMakeLists.txt`
includes this directory whenever `BIGQUERY_EMULATOR_ENABLE_DUCKDB=ON`
(the default) and exposes the resulting library as the
[`duckdb::duckdb`](./CMakeLists.txt) imported target.

DuckDB powers two later phases of the roadmap:

- **DuckDB-backed `Storage`** (Phase 3e–3f) — Parquet / Arrow files
  on disk, attached as DuckDB tables at query time.
- **DuckDB engine** (Phase 5.B) — transpiled fast path for OLAP
  workloads, sharing the same Arrow result format with the Storage
  Read API in Phase 7.

This plan (`vendor-duckdb`) only wires the dependency. The next plan
([`duckdb-storage-core`](../../.cursor/plans/duckdb-storage-core_o0d1e2f3.plan.md))
is the first consumer.

## Disabling DuckDB

If you are working offline, cross-compiling, or otherwise want to skip
the DuckDB fetch, configure with:

```bash
cmake -S . -B build-out -DBIGQUERY_EMULATOR_ENABLE_DUCKDB=OFF
```

With DuckDB disabled, `--engine=duckdb` and `--storage=duckdb` both
return `UNIMPLEMENTED` at runtime (the scaffolds stay compiled in but
never reference DuckDB symbols), and the rest of the build is
unaffected.
