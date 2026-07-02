# third_party/duckdb

Vendoring shim for [DuckDB](https://duckdb.org). The pin and SHA-256
hashes live in [`VERSION`](./VERSION); the Bazel plumbing that fetches
the upstream `libduckdb-linux-amd64.zip` archive lives in
[`MODULE.bazel`](../../MODULE.bazel) (the `http_archive` for
`@duckdb_linux_amd64`) plus the two BUILD files in this directory:

- [`BUILD.bazel`](./BUILD.bazel) exposes the public
  `//third_party/duckdb:duckdb` target with a `@platforms//cpu` `select()`
  across the amd64 and arm64 prebuilt zips.
- [`duckdb.BUILD.bazel`](./duckdb.BUILD.bazel) is injected into each
  extracted tarball as its top-level BUILD file and exposes
  `@duckdb_linux_{amd64,arm64}//:duckdb` (an `hdrs` + `cc_import(libduckdb.so)`
  pair).

DuckDB powers two parts of the engine:

- **DuckDB-backed `Storage`** — Parquet / Arrow files on disk,
  attached as DuckDB tables at query time.
- **DuckDB fast path inside the local execution coordinator** —
  transpiled fast path for OLAP workloads, one of several routes
  the coordinator dispatches resolved-AST shapes to (see
  [`docs/ENGINE_POLICY.md`](../../docs/ENGINE_POLICY.md)).
  Shares the same Arrow result format with the Storage Read API.
