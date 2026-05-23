# BigQuery Emulator Roadmap

This document tracks the implementation roadmap for `bigquery-emulator`, a
locally-runnable emulator of the BigQuery REST API. The architecture is
deliberately modeled on Google's [`cloud-spanner-emulator`][spanner]:

- The **engine is C++** and links GoogleSQL directly. This is the source of
  truth for SQL parsing, name resolution, type inference, analysis, and
  reference-impl execution. We do not re-port any of it to Go.
- The **frontend is a Go HTTP server** that implements BigQuery's public REST
  surface (projects/datasets/tables/jobs/queries/insertAll/...) and forwards
  query work to the C++ engine over an internal gRPC channel.
- The Go gateway spawns the C++ engine binary as a subprocess on startup and
  shuts it down on exit, identical to the Spanner emulator's
  `gateway_main` -> `emulator_main` pattern.

[spanner]: https://github.com/GoogleCloudPlatform/cloud-spanner-emulator

## Pluggable engine and storage

The C++ side is structured around two narrow interfaces so the heavy
machinery can be swapped without touching the gateway, the gRPC contract,
or the conformance harness:

- **`Engine` interface.** Answers `Analyze`, `DryRun`, and `ExecuteQuery`
  against a resolved AST and a catalog. Two implementations ship:
  1. **Reference Impl Engine** — wraps `googlesql::reference_impl::Evaluator`.
     Slow, row-by-row, but the source of truth for semantic parity. Drives
     the conformance harness and covers corners DuckDB can't reach yet.
  2. **DuckDB Engine** — transpiles the ZetaSQL `ResolvedAST` into a
     DuckDB-flavored SQL string via a custom C++ AST visitor/unparser,
     then executes it through DuckDB's C++ client. Massive OLAP speedup
     and Arrow-native results, at the cost of dialect fidelity for cases
     the unparser can't yet handle (see Open Questions).

- **`Storage` interface.** Backs the catalog with rows. Two implementations:
  1. **In-Memory Store** — volatile arena store. CI-friendly, zero-config,
     no on-disk footprint.
  2. **Persistent File Store** — local `.parquet` and Apache Arrow files
     managed by DuckDB. Survives restarts and gives users a real local
     data warehouse.

Engine and storage choices are independent — any engine can sit on any
store. The default profile is `(Reference Impl, In-Memory)`: the slow-but-
correct combo that maximizes conformance. The "production-emulator" profile
is `(DuckDB, Persistent File)`. Tests typically mix-and-match per case.

The point of this roadmap is to keep the work bite-sized, ordered by
dependency, and honest about what is "in" vs. "vendored from upstream."

> Status legend: ✅ done · 🟡 in progress · ⏳ planned · ❌ not yet scoped

---

## Phase 0 - Repo bootstrap

Goal: a buildable skeleton that runs an empty HTTP server and an empty C++
gRPC engine, with the gateway lifecycle wiring them together.

- ✅ Repository scaffold (this commit)
  - `binaries/gateway_main/main.go` (Go entrypoint)
  - `binaries/emulator_main/main.cc` (C++ entrypoint, stub)
  - `gateway/` Go package (HTTP server + subprocess manager)
  - `frontend/server/` C++ gRPC server (stub)
  - `backend/` C++ in-memory storage layer (stub)
  - `proto/emulator.proto` internal contract between Go and C++
  - `Taskfile.yml`, `Makefile`, `CMakeLists.txt`, `BUILD.bazel`, `MODULE.bazel`
  - `.gitignore` (excludes the local `cloud-spanner-emulator/` reference clone)
  - `README.md`, `LICENSE`, `ROADMAP.md`
- ⏳ Dev container / Dockerfile that builds both binaries reproducibly
- ⏳ CI: build matrix (linux/amd64, linux/arm64), `go vet`, `go test`,
  C++ build smoke test

## Phase 1 - Gateway HTTP surface (Go-only, stubs)

Goal: every BigQuery REST endpoint we plan to support has a route registered
and returns a structurally-valid (possibly empty) response or `501 Not
Implemented`. No SQL execution yet. Useful for client-library smoke tests.

- ✅ `GET  /` health check
- ⏳ Project routes
  - `GET    /bigquery/v2/projects`
  - `GET    /bigquery/v2/projects/{projectId}`
- ⏳ Dataset routes (`bigquery.datasets.*`)
  - list / get / insert / patch / update / delete
- ⏳ Table routes (`bigquery.tables.*`)
  - list / get / insert / patch / update / delete
- ⏳ Job routes (`bigquery.jobs.*`)
  - `POST /bigquery/v2/projects/{projectId}/jobs` (insert)
  - `GET  /bigquery/v2/projects/{projectId}/jobs/{jobId}`
  - `GET  /bigquery/v2/projects/{projectId}/jobs` (list)
  - `POST /bigquery/v2/projects/{projectId}/jobs/{jobId}/cancel`
- ⏳ Query routes
  - `POST /bigquery/v2/projects/{projectId}/queries` (`jobs.query`)
  - `GET  /bigquery/v2/projects/{projectId}/queries/{jobId}`
    (`jobs.getQueryResults`)
- ⏳ Tabledata
  - `GET  /bigquery/v2/projects/{projectId}/datasets/{datasetId}/tables/{tableId}/data`
  - `POST /bigquery/v2/projects/{projectId}/datasets/{datasetId}/tables/{tableId}/insertAll`
- ⏳ Discovery doc
  - `GET /discovery/v1/apis/bigquery/v2/rest`
- ⏳ JSON error envelope matches BigQuery shape (`{"error": {"code", "message",
  "errors": [...]}}`)
- ⏳ Authn middleware: parse but ignore bearer tokens (emulator-style)

## Phase 2 - Internal gRPC contract (Go <-> C++)

Goal: nail down the wire format the gateway uses to talk to the engine. This
is what isolates the messy "compile GoogleSQL" question from the "implement
BigQuery REST" question.

- ⏳ `proto/emulator.proto` v0
  - `Catalog` service: `RegisterDataset`, `RegisterTable`, `DropDataset`,
    `DropTable`, `DescribeTable`
  - `Query` service: `ExecuteQuery(QueryRequest) -> stream QueryResultRow`
  - `DryRun(QueryRequest) -> DryRunResult` (for `jobs.query` with
    `dryRun=true`)
- ⏳ Generate Go bindings (`buf generate` or `protoc`)
- ⏳ Generate C++ bindings (Bazel/CMake)
- ⏳ Health + readiness check (`grpc.health.v1`)
- ⏳ Gateway helper that retries until engine is ready (cf. spanner's
  `waitForReady`)

## Phase 3 - Catalog, Storage, and Engine abstractions (C++)

Goal: define the two pluggable interfaces, ship in-memory + DuckDB
implementations of `Storage`, scaffold both `Engine` impls (filled in by
Phase 5), and wire `tabledata.insertAll` / `tabledata.list` end-to-end
against both stores.

### Interfaces

- ⏳ `backend/storage/storage.h`: `Storage` interface
  (`CreateTable`, `DropTable`, `GetSchema`, `AppendRows`, `ScanRows`
  with an iterator/RecordBatch API; no engine-specific types in the
  signatures)
- ⏳ `backend/engine/engine.h`: `Engine` interface
  (`Analyze`, `DryRun`, `ExecuteQuery(resolved_ast, catalog) -> RowSource`)
- ⏳ Engine + Storage selection: `--engine=reference_impl|duckdb` and
  `--storage=memory|duckdb` flags, plus a `--profile=ci|dev` shorthand
  for the two common combos

### Storage implementations

- ⏳ `backend/storage/memory/`: `InMemoryStorage` impl, volatile arena
  store with simple row iterators
- ⏳ Vendor `libduckdb` (Bazel + CMake), pin a stable DuckDB version,
  link the C++ amalgamation, surface its Arrow APIs to our build
- ⏳ `backend/storage/duckdb/`: `DuckDBStorage` impl backed by `.parquet`
  and Apache Arrow files; per-dataset directory layout under a
  `--data_dir` root, with a small JSON sidecar per table for BigQuery-
  level metadata that doesn't fit in Parquet (description, labels, etc.)

### Schema and DDL mapping

- ⏳ `backend/schema/`: dataset / table / column / type metadata,
  conversion to/from `google.cloud.bigquery.v2.TableSchema`, conversion
  to DuckDB schema strings (and reverse, so a developer can drop a
  pre-existing parquet file into `--data_dir` and have it appear as a
  table)
- ⏳ Map BigQuery DDL to DuckDB schemas + Parquet/Arrow backing files:
  `CREATE TABLE` writes an empty Parquet/Arrow file; `DROP TABLE` removes
  it; `ALTER TABLE` rewrites; partitioning hints (date / ingestion-time)
  become per-partition files in a `partitions/` subdirectory

### Catalog wiring

- ⏳ Wire `Catalog` gRPC handlers to both storage impls behind the
  `--storage` flag (default `memory` for CI, `duckdb` for the published
  Docker image)
- ⏳ Implement `tabledata.insertAll` end-to-end (Go REST -> gRPC -> store)
  against both stores
- ⏳ Implement `tabledata.list` end-to-end (paginated reads), using
  Arrow-batched scans on `DuckDBStorage` and naive iterators on memory

### Engine scaffolding

- ⏳ `backend/engine/reference_impl/`: minimal scaffold; real wiring lands
  in Phase 5.A
- ⏳ `backend/engine/duckdb/`: minimal scaffold; transpiler + execution
  land in Phase 5.B

## Phase 4 - Query analysis (C++ via GoogleSQL)

Goal: a syntactically-valid query produces a resolved AST and a plan, without
yet executing.

- ⏳ Plumb `googlesql::Analyzer` through `Query.DryRun`
  - Returns analyzed schema, total bytes processed (estimated), and any
    parse/analysis errors with line+column info
- ⏳ Surface analysis errors via the BigQuery error envelope (Go side maps
  gRPC status codes -> BQ HTTP errors)
- ⏳ `jobs.query?dryRun=true` works end-to-end against an empty catalog
- ⏳ Type/schema reflection (BQ `STANDARD_SQL` types <-> GoogleSQL types)

## Phase 5 - Query execution

Goal: run real queries through both engines. This phase is split into two
parallelizable tracks because the engines have very different work
surfaces. The gateway code is identical between them — the choice is made
at engine-construction time and only the C++ side cares.

### Phase 5.A - Reference Impl execution (semantic source of truth)

Wires GoogleSQL's reference impl directly. Slow but correct; the yardstick
we measure DuckDB's transpiler against in the conformance harness.

- ⏳ Wire `googlesql::reference_impl::Algebrizer` + `Evaluator` to read
  from `Storage` via a `googlesql::Table` adapter that delegates to
  whichever store is active
- ⏳ Execute `SELECT 1` end-to-end via `jobs.query`
- ⏳ Execute single-table `SELECT * FROM ds.t` end-to-end
- ⏳ Joins, GROUP BY, ORDER BY (the reference impl handles these for free
  once the adapter feeds it rows)
- ⏳ Result row marshaling: GoogleSQL `Value` -> BigQuery REST row
  (`f`/`v` shape, structured types, NULLs)
- ⏳ Job lifecycle: `pending` -> `running` -> `done`, statistics
  (`creationTime`, `startTime`, `endTime`, `totalBytesProcessed`)

### Phase 5.B - DuckDB execution (transpiled fast path)

Lowers the ZetaSQL `ResolvedAST` into DuckDB SQL and executes it via the
DuckDB C++ client. Needed for any non-trivial workload to feel remotely
BigQuery-like in latency.

- ⏳ `backend/engine/duckdb/transpiler/`: a `googlesql::ResolvedASTVisitor`
  that emits DuckDB SQL strings. Implemented one node kind at a time,
  shape-tracker style, with a per-shape skiplist
- ⏳ Type lowering: BigQuery / GoogleSQL types -> DuckDB types
  (`INT64`, `FLOAT64`, `STRING`, `BYTES`, `BOOL`, `DATE`, `TIMESTAMP`,
  `NUMERIC`, `BIGNUMERIC`, `JSON`, `GEOGRAPHY`, `ARRAY<T>`, `STRUCT<...>`)
- ⏳ STRUCT handling: BQ struct literals/field access -> DuckDB STRUCTs
  (`{'a': 1}`, `s.a`); rewrites for nested struct field updates where
  DuckDB syntax diverges (`struct_update(...)`-style chains)
- ⏳ UNNEST handling: `UNNEST(arr)` and `UNNEST(arr) WITH OFFSET` ->
  DuckDB `unnest(arr)` and lateral-join shapes; keep ordinal semantics
  faithful
- ⏳ Built-in function mapping table. Start with the 80%
  (`SAFE_CAST`, `IFNULL`/`COALESCE`, `STRUCT()`, `ARRAY_AGG`,
  `STRING_AGG`, `DATE_TRUNC`, `EXTRACT`, regex, common approx
  aggregates). Per-function disposition: `map | polyfill | fallback |
  error`. `fallback` re-runs the query through the Reference Impl
  engine; policy is configurable via `--on_unknown_fn`
- ⏳ DuckDB execution path: open an in-memory or file-backed connection
  per query, attach the storage's Parquet/Arrow files as DuckDB tables,
  run the transpiled SQL, capture errors and translate them back to
  BigQuery's error envelope
- ⏳ Result marshaling: DuckDB Arrow `RecordBatch` -> BigQuery REST row
  JSON (column-major Arrow walked into the row-major `f`/`v` shape;
  NULL bitmap honored; nested STRUCTs/ARRAYs recursed). Phase 7 reuses
  the Arrow batches directly without this conversion
- ⏳ Job lifecycle parity with 5.A so the gateway never branches on
  engine choice
- ⏳ Conformance harness pin: per-test engine override via a session
  label, so we can keep DuckDB-failing shapes on Reference Impl while
  the transpiler catches up

## Phase 6 - DML / DDL

- ⏳ `INSERT`, `UPDATE`, `DELETE`, `MERGE` via reference impl
- ⏳ `CREATE TABLE`, `CREATE TABLE AS SELECT`, `DROP TABLE`, `CREATE VIEW`
- ⏳ `CREATE TEMP FUNCTION`, scripting basics
- ⏳ Job stats: `numDmlAffectedRows`

## Phase 7 - Storage Read API (gRPC)

Goal: support BigQuery client libraries that prefer the Storage Read API
(`google.cloud.bigquery.storage.v1`) for fast reads.

- ⏳ Implement `BigQueryRead` gRPC service (`CreateReadSession`,
  `ReadRows`, `SplitReadStream`)
- ⏳ Avro and Arrow output formats
- ⏳ **Native Arrow fast path when the DuckDB engine is active.** DuckDB
  already produces Arrow `RecordBatch`es as its native result format; on
  `ReadRows` we stream those batches straight onto the gRPC wire
  (`ArrowRecordBatch` / `ArrowSchema` messages) without ever round-tripping
  through the BigQuery REST `f`/`v` row shape. This sidesteps the
  manual row-by-row serialization in Phase 5.B and is the single
  biggest performance win on the read path
- ⏳ When the Reference Impl engine is active, results are row-by-row
  GoogleSQL `Value`s and we serialize them through an explicit Arrow
  encoder; slower, but used in the conformance harness
- ⏳ Single-stream sessions only at first (no parallelism); document the
  limit. DuckDB's parallel scan + Arrow output makes multi-stream a
  tractable Phase 9 add-on

## Phase 8 - Conformance harness

Goal: prove that real BigQuery client libraries work against the emulator.

- ⏳ Smoke test: official `cloud.google.com/go/bigquery` Go client
  - dataset CRUD, table CRUD, `Query.Read`, `Inserter.Put`,
    `Table.Read` (Storage API)
- ⏳ Smoke test: Python `google-cloud-bigquery`
- ⏳ Vendor a subset of the GoogleSQL `.test` corpus and run it
  against the engine via `jobs.query` (catches semantic regressions
  whenever GoogleSQL is upgraded)
- ⏳ Skiplist + drain tracker similar to `go-googlesql`'s

## Phase 9 - Distribution

- ⏳ `Dockerfile` that ships both binaries in one image
  (mirrors `gcr.io/cloud-spanner-emulator/emulator`)
- ⏳ Docker Hub / GHCR publish workflow
- ⏳ Release-tagged static-ish Linux binaries
  (`gateway_main`, `emulator_main`, `THIRD_PARTY_NOTICES.txt`)
- ⏳ Document `gcloud emulators bigquery start`-equivalent usage

---

## Non-goals

- No Go port of GoogleSQL. Engine is C++. If GoogleSQL changes, we rebuild;
  we do not chase semantics in Go.
- Persistence is opt-in, not promised. The default profile is in-memory and
  volatile; the DuckDB-backed Persistent File Store is a first-class option
  for users who want a long-lived local warehouse, but we don't claim
  durability semantics, replication, or backup.
- No production performance promises. The DuckDB engine moves the floor up
  meaningfully versus a row-by-row reference evaluator, but we aren't
  competing with the real BigQuery service. Expect competitive numbers for
  OLAP shapes the transpiler covers; expect the Reference Impl floor for
  shapes that fall back.
- No BigQuery ML / BigQuery Omni / external data sources at first; opt-in
  later if there's demand.

## Open questions / things to investigate

- **Vendoring strategy for GoogleSQL.** Spanner builds it via Bazel from the
  upstream tree. We probably want the same. CMake fallback would need to
  cover Abseil / Protobuf / RE2 / ICU and is likely a worse path; treat it as
  experimental.
- **Subprocess vs. cgo.** Spanner uses subprocess + gRPC. Cleaner ABI, easier
  crash isolation, no cgo build complexity. We start there. Cgo could be
  revisited if subprocess overhead matters in CI runs.
- **Discovery document.** BigQuery clients sometimes hit the discovery
  endpoint to learn the API surface. We can either generate ours from the
  upstream JSON or hand-write the subset we implement.
- **Auth.** Emulator should accept (and ignore) bearer tokens, matching
  Spanner's posture. Document `BIGQUERY_EMULATOR_HOST` env var contract.
- **Streaming inserts vs. Write API.** `tabledata.insertAll` is the legacy
  path; `Storage Write API` (`google.cloud.bigquery.storage.v1`) is the new
  one. Phase 6/7 should make a deliberate call on which to support first.
- **Dialect translation friction (ZetaSQL <-> DuckDB).** The transpiler in
  Phase 5.B is the single riskiest piece of the project; semantic drift
  between BigQuery and DuckDB is real and unevenly distributed. Known
  hazards we'll have to design around:
  - **`MERGE`.** GoogleSQL's `MERGE` permits `INSERT`, `UPDATE`, `DELETE`,
    and conditional branches on `WHEN MATCHED`, `WHEN NOT MATCHED BY
    SOURCE`, and `WHEN NOT MATCHED BY TARGET`. DuckDB's `INSERT ... ON
    CONFLICT` plus separate `UPDATE` / `DELETE` statements doesn't cover
    the full matrix. Likely path: lower complex `MERGE`s into a
    transactional sequence of statements, or refuse them and fall back to
    Reference Impl per the function-disposition policy.
  - **Deep STRUCT mutations.** `UPDATE t SET s.a.b = ...` is well-defined
    in BigQuery but DuckDB's struct field updates are limited; we'll need
    to rewrite as `s = struct_update(s, ...)` and chain. Worse, deeply
    nested updates may need to round-trip through JSON to preserve
    field-existence semantics.
  - **Google-specific built-ins.** `APPROX_QUANTILES`, `HLL_COUNT.*`,
    `ML.*`, `BIT_COUNT`, `NET.*`, `KEYS.NEW_KEYSET`, GIS / GEOGRAPHY
    functions, and date-arithmetic edge cases (`DATE_ADD(d, INTERVAL 1
    MONTH)` semantics on month-end) often have no DuckDB analog. The
    function-disposition table absorbs this; the Reference Impl is the
    default fallback when no mapping is possible.
  - **NULL-equality, ordering, and float corner cases** between the two
    engines are subtly different (e.g., NaN ordering, `IS NULL` in joins,
    integer overflow behavior) and need explicit conformance coverage
    before declaring the DuckDB engine on for a given workload.
  - **JSON.** BigQuery's `JSON` type and `JSON_VALUE` / `JSON_QUERY`
    functions are mostly portable, but precisely-typed JSON (`JSON` vs.
    `STRING`-encoded JSON) and the dot/path navigators have lookalike
    DuckDB functions that aren't quite the same.
  - The pragmatic answer to most of the above is per-shape and per-function
    disposition tables, an honest skiplist for what DuckDB can't yet
    handle, and the Reference Impl as the always-available escape hatch.
