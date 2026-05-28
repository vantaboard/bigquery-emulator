# BigQuery Emulator Roadmap

This document tracks the implementation roadmap for `bigquery-emulator`, a
locally-runnable emulator of the BigQuery REST API. The architecture is
deliberately modeled on Google's [`cloud-spanner-emulator`][spanner]:

- The **engine is C++** and links GoogleSQL directly. GoogleSQL is the
  source of truth for SQL parsing, name resolution, type inference, and
  analysis; the DuckDB engine lowers the resolved AST to DuckDB SQL for
  execution. We do not re-port any of it to Go.
- The **frontend is a Go HTTP server** that implements BigQuery's public REST
  surface (projects/datasets/tables/jobs/queries/insertAll/...) and forwards
  query work to the C++ engine over an internal gRPC channel.
- The Go gateway spawns the C++ engine binary as a subprocess on startup and
  shuts it down on exit, identical to the Spanner emulator's
  `gateway_main` -> `emulator_main` pattern.

[spanner]: https://github.com/GoogleCloudPlatform/cloud-spanner-emulator

## Execution plans

For subagent-driven implementation, the roadmap is split into **46 small
plans** under [`.cursor/plans/`](.cursor/plans/) (~3 todos each, one session
per plan). Start with the index:

- [`.cursor/plans/bigquery-emulator-roadmap-index_a1b2c3d4.plan.md`](.cursor/plans/bigquery-emulator-roadmap-index_a1b2c3d4.plan.md) — full catalog, dependency graph, subagent template

Each plan maps to a slice of one of the capability areas below and includes
filled-in todos, verification commands, and done criteria.

## Engine and storage

The C++ side is structured around two narrow interfaces
(`backend/engine/engine.h`, `backend/storage/storage.h`) so the heavy
machinery can be swapped without touching the gateway, the gRPC
contract, or the conformance harness. Today there is exactly one
implementation behind each interface:

- **`Engine` interface — DuckDB engine** (`backend/engine/duckdb/`).
  Transpiles the GoogleSQL `ResolvedAST` into a DuckDB-flavored SQL
  string via a custom C++ AST visitor/unparser, then executes it
  through DuckDB's C++ client. Massive OLAP speedup and Arrow-native
  results, at the cost of dialect fidelity for cases the unparser
  can't yet handle (see Open Questions).

- **`Storage` interface — DuckDB storage** (`backend/storage/duckdb/`).
  Local `.parquet` and Apache Arrow files managed by DuckDB. Survives
  restarts and gives users a real local data warehouse.

An earlier iteration carried a second `Engine` implementation
(ReferenceImpl, on top of `googlesql::reference_impl::Evaluator`)
bridged to DuckDB by a `FallbackEngine` wrapper, plus an in-memory
storage backend for hermetic tests. Both were removed once DuckDB
covered the supported surface; see `docs/ENGINE_POLICY.md` for the
deprecation rationale.

The point of this roadmap is to keep the work bite-sized, ordered by
dependency, and honest about what is "in" vs. "vendored from upstream."

> Status legend: ✅ done · 🟡 in progress · ⏳ planned · ❌ not yet scoped

---

## Repo bootstrap

Goal: a buildable skeleton that runs an empty HTTP server and an empty C++
gRPC engine, with the gateway lifecycle wiring them together.

- ✅ Repository scaffold (this commit)
  - `binaries/gateway_main/main.go` (Go entrypoint)
  - `binaries/emulator_main/main.cc` (C++ entrypoint, stub)
  - `gateway/` Go package (HTTP server + subprocess manager)
  - `frontend/server/` C++ gRPC server (stub)
  - `backend/` C++ storage + engine layer (stub)
  - `proto/emulator.proto` internal contract between Go and C++
  - `Taskfile.yml`, `Makefile`, `BUILD.bazel`, `MODULE.bazel`
  - `.gitignore` (excludes the local `cloud-spanner-emulator/` reference clone)
  - `README.md`, `LICENSE`, `ROADMAP.md`
- ⏳ Dev container / Dockerfile that builds both binaries reproducibly
- ⏳ CI: build matrix (linux/amd64, linux/arm64), `go vet`, `go test`,
  C++ build smoke test

## Gateway HTTP surface (Go-only, stubs)

Goal: every BigQuery REST endpoint we plan to support has a route registered
and returns a structurally-valid (possibly empty) response or `501 Not
Implemented`. No SQL execution yet. Useful for client-library smoke tests.

The canonical, always-up-to-date mapping from BigQuery v2 REST endpoints
to the handlers in this repo lives in [`docs/REST_API.md`](./docs/REST_API.md);
that document is cross-referenced against the upstream documentation under
[`docs/bigquery/docs/reference/rest/v2/`](./docs/bigquery/docs/reference/rest/v2/).

- ✅ Health checks (`GET /`, `GET /healthz`) — emulator-only, not part of the
  public BigQuery API
- ✅ Project routes
  - `GET /bigquery/v2/projects` (`projects.list`)
  - `GET /bigquery/v2/projects/{projectId}/serviceAccount`
    (`projects.getServiceAccount`) — note: there is **no**
    `GET /bigquery/v2/projects/{projectId}` endpoint in the public API
- ✅ Dataset routes (`bigquery.datasets.*`)
  - list / get / insert / patch / update / delete / undelete
  - `datasets.undelete` is `POST .../datasets/{datasetId}:undelete` (AIP-136
    custom method); dispatched on the trailing `:undelete` because Go's
    mux can't match a literal segment after a wildcard
- ✅ Table routes (`bigquery.tables.*`)
  - list / get / insert / patch / update / delete
  - IAM: `getIamPolicy`, `setIamPolicy`, `testIamPermissions` — all three
    are `POST .../tables/{tableId}:operation` custom methods, dispatched
    the same way as `datasets.undelete`
- ✅ Tabledata routes (`bigquery.tabledata.*`)
  - `GET .../tables/{tableId}/data` (`tabledata.list`)
  - `POST .../tables/{tableId}/insertAll` (`tabledata.insertAll`)
- ✅ Job routes (`bigquery.jobs.*`)
  - `GET /bigquery/v2/projects/{projectId}/jobs` (`jobs.list`)
  - `POST /bigquery/v2/projects/{projectId}/jobs` (`jobs.insert`, metadata)
  - `POST /upload/bigquery/v2/projects/{projectId}/jobs` (`jobs.insert`,
    media upload — multipart/resumable per the upstream
    [`api-uploads.md`](./docs/bigquery/docs/reference/api-uploads.md))
  - `GET /bigquery/v2/projects/{projectId}/jobs/{jobId}` (`jobs.get`)
  - `POST /bigquery/v2/projects/{projectId}/jobs/{jobId}/cancel`
    (`jobs.cancel`)
  - `DELETE /bigquery/v2/projects/{projectId}/jobs/{jobId}/delete`
    (`jobs.delete`) — the literal `/delete` suffix is the upstream URL
    template, not a typo
- ✅ Query routes
  - `POST /bigquery/v2/projects/{projectId}/queries` (`jobs.query`)
  - `GET /bigquery/v2/projects/{projectId}/queries/{jobId}`
    (`jobs.getQueryResults`)
- ⏳ Discovery doc
  - `GET /discovery/v1/apis/bigquery/v2/rest`
- ✅ JSON error envelope matches the documented BigQuery shape
  (`{"error": {"code", "message", "status", "errors": [...]}}`); see
  [`docs/bigquery/docs/error-messages.md`](./docs/bigquery/docs/error-messages.md)
  for the canonical sample response and reason codes
- ⏳ Authn middleware: parse but ignore bearer tokens (emulator-style),
  honor `BIGQUERY_EMULATOR_HOST` env-var conventions
- ⏳ SQL dialect posture: BigQuery's `useLegacySql` field defaults to
  `true` on the wire, but the emulator only supports GoogleSQL. Reject
  `useLegacySql=true` with HTTP 400 + `reason: invalidQuery`; treat
  unset and `false` as GoogleSQL
- ⏳ Result wire encoding follows
  [`StandardSqlDataType.TypeKind`](./docs/bigquery/docs/reference/rest/v2/StandardSqlDataType.md)
  exactly: `INT64`/`NUMERIC`/`BIGNUMERIC` are decimal strings, `FLOAT64`
  is JSON number with `"NaN"`/`"Infinity"` sentinels, `BYTES` is base64,
  `TIMESTAMP`/`DATE`/`TIME`/`DATETIME` are RFC 3339, `STRUCT` is a
  positional list, etc.

## Internal gRPC contract (Go <-> C++)

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
- ⏳ Generate C++ bindings (Bazel)
- ⏳ Health + readiness check (`grpc.health.v1`)
- ⏳ Gateway helper that retries until engine is ready (cf. spanner's
  `waitForReady`)

## Catalog, Storage, and Engine abstractions (C++)

Goal: define the two pluggable interfaces, ship the DuckDB
implementation of `Storage`, scaffold the DuckDB `Engine` impl (filled
in by the query-execution work below), and wire `tabledata.insertAll`
/ `tabledata.list` end-to-end against `DuckDBStorage`. The interfaces
stay narrow so a future second implementation (e.g. a remote engine,
an alternate storage layout) can land without disturbing the gateway
or the gRPC contract; today there is exactly one impl behind each.

### Interfaces

- ⏳ `backend/storage/storage.h`: `Storage` interface
  (`CreateTable`, `DropTable`, `GetSchema`, `AppendRows`, `ScanRows`
  with an iterator/RecordBatch API; no engine-specific types in the
  signatures)
- ⏳ `backend/engine/engine.h`: `Engine` interface
  (`Analyze`, `DryRun`, `ExecuteQuery(resolved_ast, catalog) -> RowSource`)

### Storage implementation

- ⏳ Vendor `libduckdb` (Bazel), pin a stable DuckDB version,
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

- ⏳ Wire `Catalog` gRPC handlers to `DuckDBStorage` (the only storage
  impl today)
- ⏳ Implement `tabledata.insertAll` end-to-end (Go REST -> gRPC ->
  `DuckDBStorage`)
- ⏳ Implement `tabledata.list` end-to-end (paginated reads), using
  Arrow-batched scans on `DuckDBStorage`

### Engine scaffolding

- ⏳ `backend/engine/duckdb/`: minimal scaffold; transpiler + execution
  land in the DuckDB execution work below

## Query analysis (C++ via GoogleSQL)

Goal: a syntactically-valid query produces a resolved AST and a plan, without
yet executing.

- ⏳ Plumb `googlesql::Analyzer` through `Query.DryRun`
  - Returns analyzed schema, total bytes processed (estimated), and any
    parse/analysis errors with line+column info
- ⏳ Surface analysis errors via the BigQuery error envelope (Go side maps
  gRPC status codes -> BQ HTTP errors)
- ⏳ `jobs.query?dryRun=true` works end-to-end against an empty catalog
- ⏳ Type/schema reflection (BQ `STANDARD_SQL` types <-> GoogleSQL types)

## Query execution

Goal: run real queries through the DuckDB engine. The gateway forwards
analyzed queries to `emulator_main` over gRPC and never branches on
engine choice. The engine lowers the ZetaSQL `ResolvedAST` into DuckDB
SQL and executes it via the DuckDB C++ client.

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
  aggregates). Per-function disposition: `map | polyfill | error`.
  Functions without a mapping return `UNIMPLEMENTED` so the
  conformance harness pins the gap rather than silently misbehaving
- ⏳ DuckDB execution path: open an in-memory or file-backed connection
  per query, attach the storage's Parquet/Arrow files as DuckDB tables,
  run the transpiled SQL, capture errors and translate them back to
  BigQuery's error envelope
- ⏳ Result marshaling: DuckDB Arrow `RecordBatch` -> BigQuery REST row
  JSON (column-major Arrow walked into the row-major `f`/`v` shape;
  NULL bitmap honored; nested STRUCTs/ARRAYs recursed). The Storage
  Read API surface reuses the Arrow batches directly without this
  conversion
- ⏳ Job lifecycle: `pending` -> `running` -> `done`, statistics
  (`creationTime`, `startTime`, `endTime`, `totalBytesProcessed`)

## DML / DDL

- ⏳ `INSERT`, `UPDATE`, `DELETE`, `MERGE` lowered through the DuckDB
  transpiler. `MERGE`, `CREATE TABLE`, `CREATE TABLE AS SELECT`, and
  `DROP TABLE` are implemented today; `INSERT VALUES` /
  `INSERT ... SELECT`, `UPDATE`, and `DELETE` still return
  `UNIMPLEMENTED` (see `docs/ENGINE_POLICY.md`)
- ⏳ `CREATE TABLE`, `CREATE TABLE AS SELECT`, `DROP TABLE`, `CREATE VIEW`
- ⏳ `CREATE TEMP FUNCTION`, scripting basics
- ⏳ Job stats: `numDmlAffectedRows`

## Storage Read API (gRPC)

Goal: support BigQuery client libraries that prefer the Storage Read API
(`google.cloud.bigquery.storage.v1`) for fast reads.

- ⏳ Implement `BigQueryRead` gRPC service (`CreateReadSession`,
  `ReadRows`, `SplitReadStream`)
- ⏳ Avro and Arrow output formats
- ⏳ **Native Arrow fast path.** DuckDB already produces Arrow
  `RecordBatch`es as its native result format; on `ReadRows` we stream
  those batches straight onto the gRPC wire (`ArrowRecordBatch` /
  `ArrowSchema` messages) without ever round-tripping through the
  BigQuery REST `f`/`v` row shape. This sidesteps the manual
  row-by-row serialization that the DuckDB execution path uses for
  REST results and is the single biggest performance win on the read
  path
- ⏳ Single-stream sessions only at first (no parallelism); document the
  limit. DuckDB's parallel scan + Arrow output makes multi-stream a
  tractable post-distribution add-on

## Conformance harness

Goal: prove that real BigQuery client libraries work against the emulator.

- ⏳ Smoke test: official `cloud.google.com/go/bigquery` Go client
  - dataset CRUD, table CRUD, `Query.Read`, `Inserter.Put`,
    `Table.Read` (Storage API)
- ⏳ Smoke test: Python `google-cloud-bigquery`
- ⏳ Vendor a subset of the GoogleSQL `.test` corpus and run it
  against the engine via `jobs.query` (catches semantic regressions
  whenever GoogleSQL is upgraded)
- ⏳ Skiplist + drain tracker similar to `go-googlesql`'s

## Distribution

- ✅ `Dockerfile` that ships both binaries in one image
  (mirrors `gcr.io/cloud-spanner-emulator/emulator`)
- ✅ Docker Hub / GHCR publish workflow (`.github/workflows/release.yml`,
  pushes to `ghcr.io/vantaboard/bigquery-emulator`)
- ✅ Release-tagged static-ish Linux binaries via goreleaser
  (`bigquery-emulator-gateway`, bundled `bin/emulator_main` +
  `bin/libduckdb.so`; linux/amd64 engine only — see README §Releases)
- ✅ `--version` on both binaries reporting consistent semver +
  git commit + build date; gateway via `-X main.<sym>=…` ldflags
  (see `.goreleaser.yml`), engine via the `:version_cc` genrule under
  `binaries/emulator_main/BUILD.bazel`
- ✅ Runtime configuration documented in README §Runtime configuration;
  the conformance harness (`conformance/README.md`) and the engine
  policy (`docs/ENGINE_POLICY.md`) cover the DuckDB-only runtime.
  `emulator_main` accepts `--host_port` and `--data_dir` only;
  `--engine` / `--storage` / `--on_unknown_fn` were removed when
  the ReferenceImpl + in-memory backends were deleted.
- ⏳ Document `gcloud emulators bigquery start`-equivalent usage

---

## Non-goals

- No Go port of GoogleSQL. Engine is C++. If GoogleSQL changes, we rebuild;
  we do not chase semantics in Go.
- Persistence is best-effort, not promised. The DuckDB-backed
  `--data_dir` is what runs in production; we don't claim durability
  semantics, replication, or backup.
- No production performance promises. The DuckDB engine is the only
  execution backend; we aren't competing with the real BigQuery
  service. Expect competitive numbers for OLAP shapes the transpiler
  covers; expect `UNIMPLEMENTED` for shapes it does not lower yet
  (INSERT / UPDATE / DELETE / scalar-only SELECT today).
- No BigQuery ML / BigQuery Omni / external data sources at first; opt-in
  later if there's demand.

## Build systems

The C++ engine is built with Bazel. The
`//binaries/emulator_main:emulator_main` `cc_binary` links
GoogleSQL's analyzer, the DuckDB engine + DuckDB storage backend,
the catalog/query gRPC handlers, and grpc++ in one binary that can
serve `Query.DryRun` and `Query.ExecuteQuery` end-to-end. GoogleSQL
is vendored via a sibling
`../googlesql/` checkout (`local_path_override` in
[`MODULE.bazel`](./MODULE.bazel)); DuckDB v1.5.3 is pulled in as a
prebuilt tarball through `http_archive`
(see [`third_party/duckdb/`](./third_party/duckdb/)). Build with
`task emulator:build-engine-bazel` or directly
`bazel build //binaries/emulator_main:emulator_main`.

Linux/amd64 only today — the GoogleSQL hermetic LLVM toolchain ships
amd64 binaries and does not cross-build cleanly to linux/arm64 yet,
so the engine binary is not produced for arm64. Non-amd64 hosts
should use the published Docker image.

The integration tests under [`gateway/e2e/`](./gateway/e2e/)
discover the engine binary via `./bin/emulator_main`, and
`task emulator:build-engine-bazel` stages the Bazel-built binary +
sibling `libduckdb.so` there with an `rpath` of `$ORIGIN`.

## Open questions / things to investigate

- **Vendoring strategy for GoogleSQL.** Spanner builds it via Bazel from the
  upstream tree. We do the same (see "Build systems" above).
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
  one. The DML/DDL and Storage Read API tracks should make a deliberate
  call on which to support first.
- **Dialect translation friction (ZetaSQL <-> DuckDB).** The DuckDB
  transpiler is the single riskiest piece of the project; semantic drift
  between BigQuery and DuckDB is real and unevenly distributed. Known
  hazards we'll have to design around:
  - **`MERGE`.** GoogleSQL's `MERGE` permits `INSERT`, `UPDATE`, `DELETE`,
    and conditional branches on `WHEN MATCHED`, `WHEN NOT MATCHED BY
    SOURCE`, and `WHEN NOT MATCHED BY TARGET`. DuckDB's `INSERT ... ON
    CONFLICT` plus separate `UPDATE` / `DELETE` statements doesn't cover
    the full matrix. The current path lowers `MERGE` into a
    transactional sequence of statements where possible; shapes the
    transpiler can't yet model return `UNIMPLEMENTED` so the gap is
    pinned by the conformance harness.
  - **Deep STRUCT mutations.** `UPDATE t SET s.a.b = ...` is well-defined
    in BigQuery but DuckDB's struct field updates are limited; we'll need
    to rewrite as `s = struct_update(s, ...)` and chain. Worse, deeply
    nested updates may need to round-trip through JSON to preserve
    field-existence semantics.
  - **Google-specific built-ins.** `APPROX_QUANTILES`, `HLL_COUNT.*`,
    `ML.*`, `BIT_COUNT`, `NET.*`, `KEYS.NEW_KEYSET`, GIS / GEOGRAPHY
    functions, and date-arithmetic edge cases (`DATE_ADD(d, INTERVAL 1
    MONTH)` semantics on month-end) often have no DuckDB analog. The
    function-disposition table absorbs the ones we can map or
    polyfill; the rest return `UNIMPLEMENTED` so the conformance
    harness pins each gap.
  - **NULL-equality, ordering, and float corner cases** between the two
    engines are subtly different (e.g., NaN ordering, `IS NULL` in joins,
    integer overflow behavior) and need explicit conformance coverage
    before declaring the DuckDB engine on for a given workload.
  - **JSON.** BigQuery's `JSON` type and `JSON_VALUE` / `JSON_QUERY`
    functions are mostly portable, but precisely-typed JSON (`JSON` vs.
    `STRING`-encoded JSON) and the dot/path navigators have lookalike
    DuckDB functions that aren't quite the same.
  - The pragmatic answer to most of the above is per-shape and
    per-function disposition tables and an honest skiplist for what
    DuckDB can't yet handle; unmapped shapes return `UNIMPLEMENTED`
    rather than silently degrading.
