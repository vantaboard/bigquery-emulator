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

The remaining unscoped work is the DuckDB transpiler shape coverage —
flipping every `skiplist` / `not_started` row in
[`backend/engine/duckdb/transpiler/SHAPE_TRACKER.md`](./backend/engine/duckdb/transpiler/SHAPE_TRACKER.md)
to `done`. The execution order, dependencies, and per-shape plans
live under [`.cursor/plans/`](.cursor/plans/); start with the index:

- [`.cursor/plans/duckdb-transpiler-shape-roadmap-index.plan.md`](.cursor/plans/duckdb-transpiler-shape-roadmap-index.plan.md) — execution order across the 16 transpiler plans, with shared rules and per-shape done criteria

The older 46-plan capability-area scaffold (one plan per gateway /
engine / storage / conformance slice) has been retired now that its
slices have either landed or collapsed into the shape-tracker work
below.

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
  A single `catalog.duckdb` file under `--data_dir` (default
  `$HOME/.bigquery-emulator`) holds the catalog and all table rows;
  the engine reads and writes through DuckDB's C++ client. Survives
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

- ✅ Repository scaffold
  - `binaries/gateway_main/main.go` (Go entrypoint)
  - `binaries/emulator_main/main.cc` (C++ entrypoint)
  - `gateway/` Go package (HTTP server + subprocess manager)
  - `frontend/server/` C++ gRPC server
  - `backend/` C++ catalog + storage + engine layer
  - `proto/emulator.proto` + `proto/storage_read.proto` (internal
    contract between Go and C++)
  - `Taskfile.yml` + `taskfiles/<namespace>.yml`, `Makefile`,
    `BUILD.bazel`, `MODULE.bazel`
  - `.gitignore` (excludes the local `cloud-spanner-emulator/` reference clone)
  - `README.md`, `LICENSE`, `ROADMAP.md`
- ✅ Dockerfile that builds both binaries reproducibly
  (multi-stage; the canonical Bazel `engine-builder` stage links
  GoogleSQL + DuckDB; the runtime stage ships
  `bigquery-emulator-gateway` + `bin/emulator_main` +
  `bin/libduckdb.so` and mirrors `gcr.io/cloud-spanner-emulator/emulator`)
- ✅ CI: GitHub Actions workflows for build/test/lint (`ci.yml`),
  Docker smoke (`docker-smoke.yml`), conformance fixtures
  (`conformance.yml`), third-party client samples
  (`thirdparty-samples.yml`), GoogleSQL parity + prebuilt artifact
  (`googlesql-parity.yml`, `googlesql-prebuilt.yml`), coverage
  publish (`coverage-bazel.yml`, `coverage-publish.yml`), and
  tag-triggered release (`release.yml`). Linux/amd64 only — the
  GoogleSQL hermetic LLVM toolchain does not cross-build cleanly
  to linux/arm64 yet (see "Build systems" below)

## Gateway HTTP surface (Go)

Goal: every BigQuery REST endpoint a client library expects has a
route registered. The core CRUD + query surface
(`projects` / `datasets` / `tables` / `tabledata` / `jobs` /
`queries`) is implemented end-to-end against the C++ engine; the
peripheral surfaces (`models`, `routines`, `rowAccessPolicies`,
`migration`, `dataTransfer`) are wired as structurally-valid stubs
so client-library startup probes succeed and per-method behavior
is per-row in [`docs/REST_API.md`](./docs/REST_API.md).

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
- ✅ Discovery doc
  - `GET /discovery/v1/apis/bigquery/v2/rest`
- ✅ Models / Routines / RowAccessPolicies / Migration / Data
  Transfer routes registered as wired stubs so client-library
  startup probes (list / get / delete sample loops) succeed; the
  per-method status lives in [`docs/REST_API.md`](./docs/REST_API.md)
- ✅ JSON error envelope matches the documented BigQuery shape
  (`{"error": {"code", "message", "status", "errors": [...]}}`); see
  [`docs/bigquery/docs/error-messages.md`](./docs/bigquery/docs/error-messages.md)
  for the canonical sample response and reason codes
- ✅ Authn middleware: parses but ignores bearer tokens
  (emulator-style), attaches a synthetic
  `emulator@bigquery.local` principal to every request, never
  returns 401 ([`gateway/middleware/auth.go`](./gateway/middleware/auth.go));
  honors `BIGQUERY_EMULATOR_HOST` env-var conventions on the client
  side
- ✅ SQL dialect posture: `useLegacySql=true` rejected with HTTP
  400 + `reason: invalidQuery` before any engine work; unset and
  `false` treated as GoogleSQL
  ([`gateway/handlers/queries.go`](./gateway/handlers/queries.go))
- ✅ Result wire encoding follows
  [`StandardSqlDataType.TypeKind`](./docs/bigquery/docs/reference/rest/v2/StandardSqlDataType.md)
  exactly: `INT64`/`NUMERIC`/`BIGNUMERIC` are decimal strings, `FLOAT64`
  is JSON number with `"NaN"`/`"Infinity"` sentinels, `BYTES` is base64,
  `TIMESTAMP`/`DATE`/`TIME`/`DATETIME` are RFC 3339, `STRUCT` is a
  positional list, etc. (see the type table in `docs/REST_API.md`)

## Internal gRPC contract (Go <-> C++)

Goal: nail down the wire format the gateway uses to talk to the engine. This
is what isolates the messy "compile GoogleSQL" question from the "implement
BigQuery REST" question.

- ✅ [`proto/emulator.proto`](./proto/emulator.proto) v0
  - `Catalog` service: `RegisterDataset`, `RegisterTable`, `DropDataset`,
    `DropTable`, `DescribeTable`
  - `Query` service: `ExecuteQuery(QueryRequest) -> stream QueryResultRow`
    + `DryRun(QueryRequest) -> DryRunResult` (for `jobs.query` with
    `dryRun=true`)
- ✅ [`proto/storage_read.proto`](./proto/storage_read.proto) —
  internal Storage Read API contract served by the engine on its
  gRPC port (`bigquery_emulator.v1.StorageRead`)
- ✅ Generate Go bindings (`buf generate`) — output under
  [`gateway/enginepb/`](./gateway/enginepb/)
- ✅ Generate C++ bindings (Bazel `proto_library` + `cc_grpc_library`
  rules in [`proto/BUILD.bazel`](./proto/BUILD.bazel))
- ✅ Health + readiness check (`grpc.health.v1`), served by
  `emulator_main`
- ✅ Gateway helper that retries until engine reports SERVING
  before the gateway opens its public listener
  ([`gateway/gateway.go`](./gateway/gateway.go))

## Catalog, Storage, and Engine abstractions (C++)

Goal: define the two pluggable interfaces, ship the DuckDB
implementation of `Storage`, ship the DuckDB `Engine` impl (the
transpiler + execution surface tracked in the Query execution
section below), and wire `tabledata.insertAll` / `tabledata.list`
end-to-end against `DuckDBStorage`. The interfaces stay narrow so
a future second implementation (e.g. a remote engine, an alternate
storage layout) can land without disturbing the gateway or the gRPC
contract; today there is exactly one impl behind each.

### Interfaces

- ✅ [`backend/storage/storage.h`](./backend/storage/storage.h):
  `Storage` interface (`CreateTable`, `DropTable`, `GetSchema`,
  `AppendRows`, `ScanRows`, plus row-restriction pushdown for the
  Storage Read API path)
- ✅ [`backend/engine/engine.h`](./backend/engine/engine.h):
  `Engine` interface (`Analyze`, `DryRun`,
  `ExecuteQuery(resolved_ast, catalog) -> RowSource`)

### Storage implementation

- ✅ Vendor `libduckdb` v1.5.3 via Bazel `http_archive`
  ([`third_party/duckdb/`](./third_party/duckdb/)); the engine links
  the prebuilt `libduckdb.so` (linux/amd64 zip) and surfaces its
  Arrow APIs to the build
- ✅ [`backend/storage/duckdb/`](./backend/storage/duckdb/):
  `DuckDBStorage` impl backed by a single `catalog.duckdb` file
  under `--data_dir` (default `$HOME/.bigquery-emulator`). Catalog
  + table rows persist across restarts; the per-dataset directory
  layout / external Parquet drop-in is on the open-questions list
  rather than the active surface

### Schema and DDL mapping

- ✅ [`backend/schema/`](./backend/schema/): dataset / table / column /
  type metadata, conversion to/from
  `google.cloud.bigquery.v2.TableSchema` and to DuckDB schema strings
  (`backend/schema/googlesql_to_bq.{h,cc}`)
- 🟡 BigQuery DDL through the DuckDB transpiler: `CREATE TABLE`,
  `CREATE TABLE AS SELECT`, and `DROP TABLE` lower today;
  `ALTER TABLE`, `CREATE VIEW`, partitioning / clustering hints,
  and direct external-Parquet drop-in still pending

### Catalog wiring

- ✅ `Catalog` gRPC handlers backed by `DuckDBStorage`
  (`frontend/handlers/catalog.{h,cc}`)
- ✅ `tabledata.insertAll` end-to-end (Go REST ->
  [`gateway/handlers/tabledata.go`](./gateway/handlers/tabledata.go) ->
  engine gRPC -> `DuckDBStorage`); used by the conformance harness
  as the canonical seeding path while DML INSERT is still
  UNIMPLEMENTED on the DuckDB engine
- ✅ `tabledata.list` end-to-end (paginated reads, Arrow-batched
  scans on `DuckDBStorage`)

### Engine scaffolding

- ✅ [`backend/engine/duckdb/`](./backend/engine/duckdb/): the
  transpiler + DuckDB execution path are the sole engine implementation.
  Per-shape coverage is tracked in
  [`backend/engine/duckdb/transpiler/SHAPE_TRACKER.md`](./backend/engine/duckdb/transpiler/SHAPE_TRACKER.md)
  and worked through the plans linked from the index above

## Query analysis (C++ via GoogleSQL)

Goal: a syntactically-valid query produces a resolved AST and a plan, without
yet executing.

- ✅ `googlesql::Analyzer` plumbed through `Query.DryRun` and
  `Query.ExecuteQuery` (the DuckDB engine reuses the resolved AST
  from analysis as the input to the transpiler)
- ✅ Analysis errors surface via the BigQuery error envelope; the
  Go side maps engine gRPC status codes to BigQuery HTTP error
  shapes ([`gateway/handlers/queries.go`](./gateway/handlers/queries.go),
  [`gateway/handlers/errors.go`](./gateway/handlers/errors.go))
- ✅ `jobs.query?dryRun=true` works end-to-end (returns
  `totalBytesProcessed`, the analyzed schema, and any error with
  line + column info from the analyzer)
- ✅ Type/schema reflection: BigQuery `STANDARD_SQL` types <->
  GoogleSQL types via
  [`backend/schema/googlesql_to_bq.{h,cc}`](./backend/schema/)

## Query execution

Goal: run real queries through the DuckDB engine. The gateway forwards
analyzed queries to `emulator_main` over gRPC and never branches on
engine choice. The engine lowers the GoogleSQL `ResolvedAST` into
DuckDB SQL and executes it via the DuckDB C++ client.

- 🟡 [`backend/engine/duckdb/transpiler/`](./backend/engine/duckdb/transpiler/):
  a `googlesql::ResolvedASTVisitor` that emits DuckDB SQL strings.
  Implemented one node kind at a time, shape-tracker style; per-row
  status lives in
  [`SHAPE_TRACKER.md`](./backend/engine/duckdb/transpiler/SHAPE_TRACKER.md)
  and the remaining `skiplist` / `not_started` rows are scheduled by
  the index plan linked at the top of this document
- ✅ Type lowering: BigQuery / GoogleSQL types -> DuckDB types
  (`INT64`, `FLOAT64`, `STRING`, `BYTES`, `BOOL`, `DATE`, `TIMESTAMP`,
  `NUMERIC`, `BIGNUMERIC`, `JSON`, `INTERVAL`, `UUID`, `ARRAY<T>`,
  `STRUCT<...>`). `GEOGRAPHY` is `kSkiplist` until a GIS pass lands
- ✅ STRUCT handling: BQ struct literals + field access lower to
  DuckDB STRUCTs (`{'a': 1}`, `s."a"`); anonymous STRUCT fields
  synthesize positional names (`_0`, `_1`, ...) on both the
  construction and access sides so they round-trip; deep STRUCT
  mutations (`UPDATE t SET s.a.b = ...`) still fall back via the
  empty-string contract (see Open Questions)
- 🟡 UNNEST handling: standalone `UNNEST(arr) AS col` lowers to
  DuckDB `SELECT unnest(arr) AS "col"`. `WITH OFFSET`, multi-array
  zip, outer `UNNEST`, and lateral / cross-join shapes still surface
  `UNIMPLEMENTED` pending the lateral-join rewrites tracked in
  `duckdb-transpiler-array-lateral.plan.md`
- 🟡 Built-in function mapping table — sourced from
  [`backend/engine/duckdb/transpiler/functions.yaml`](./backend/engine/duckdb/transpiler/functions.yaml)
  (~140 BigQuery functions across math, string, datetime,
  conditional, array, aggregation, window, and BQ-specific
  categories). A Bazel `genrule` materializes it into
  `functions_table.inc`, which `functions.cc` includes inside an
  `absl::flat_hash_map`. Per-function disposition is `kMap` (emit a
  DuckDB function call), `kFallback` (deferred lowering, surfaces
  UNIMPLEMENTED), or `kSkiplist` (out of scope today —
  `APPROX_QUANTILES`, `ML.*`, `NET.*`, `KEYS.*`, `ST_*`, ...).
  `SAFE.<fn>(...)` is handled uniformly regardless of disposition
- ✅ DuckDB execution path: file-backed `catalog.duckdb` connection,
  the transpiled SQL is bound and executed via the DuckDB C++ client,
  DuckDB errors are translated back to the BigQuery error envelope
- ✅ Result marshaling: DuckDB Arrow `RecordBatch` -> BigQuery REST
  row JSON (column-major Arrow walked into the row-major `f`/`v`
  shape; NULL bitmap honored; nested STRUCTs/ARRAYs recursed). The
  Storage Read API path reuses the Arrow batches directly without
  this conversion
- ✅ Job lifecycle: `pending` -> `running` -> `done`, statistics
  (`creationTime`, `startTime`, `endTime`, `totalBytesProcessed`)
  surfaced through [`gateway/jobs/`](./gateway/jobs/)

## DML / DDL

- 🟡 DML through the DuckDB transpiler. `MERGE` is supported;
  `INSERT VALUES` / `INSERT ... SELECT`, `UPDATE`, and `DELETE`
  still return `UNIMPLEMENTED` (see
  [`docs/ENGINE_POLICY.md`](./docs/ENGINE_POLICY.md) and the
  `duckdb-transpiler-dml.plan.md` plan). Until INSERT lands,
  conformance fixtures and tests seed rows with
  `tabledata.insertAll` instead
- 🟡 DDL: `CREATE TABLE`, `CREATE TABLE AS SELECT`, and
  `DROP TABLE` lower today; `CREATE VIEW`, `CREATE MATERIALIZED
  VIEW`, and `ALTER TABLE` still return `UNIMPLEMENTED` (see
  `duckdb-transpiler-ddl.plan.md`)
- ⏳ `CREATE TEMP FUNCTION`, scripting basics (UDFs / TVFs /
  procedures) — `duckdb-transpiler-udf-tvf-module.plan.md` and
  `duckdb-transpiler-procedural.plan.md`
- ⏳ Job stats: `numDmlAffectedRows`

## Storage Read API (gRPC)

Goal: support BigQuery client libraries that prefer the Storage Read API
(`google.cloud.bigquery.storage.v1`) for fast reads.

- 🟡 `BigQueryRead` gRPC service implemented
  ([`frontend/handlers/storage_read.{h,cc}`](./frontend/handlers/));
  `CreateReadSession` and `ReadRows` are wired, `SplitReadStream`
  is still pending (single-stream sessions only — see below)
- 🟡 Arrow output format implemented; Avro output is not wired yet
- ✅ **Native Arrow fast path.** DuckDB produces Arrow
  `RecordBatch`es as its native result format; `ReadRows` streams
  those batches straight onto the gRPC wire (`ArrowRecordBatch` /
  `ArrowSchema` messages) without round-tripping through the
  BigQuery REST `f`/`v` row shape, sidestepping the row-by-row
  serialization the REST path uses
- 🟡 `ReadOptions.row_restriction`: a single `<column> = <literal>`
  equality clause is pushed down into the DuckDB `read_parquet(...)`
  scan as a `WHERE` clause. Range / inequality ops, connectives,
  `IN`, `NULL`, and array/struct columns reject at `CreateReadSession`
  with `INVALID_ARGUMENT` (see "Supported `ReadOptions`" in
  [`docs/REST_API.md`](./docs/REST_API.md))
- ⏳ `ReadOptions.selected_fields` accepted + echoed on the
  `ReadSession` reply but **not enforced** — every column is
  returned regardless. Pushing projection into the storage layer
  is deferred
- ⏳ Single-stream sessions only — `max_stream_count > 1` is rejected.
  DuckDB's parallel scan + Arrow output makes multi-stream a
  tractable follow-up

## Conformance harness

Goal: prove that real BigQuery client libraries work against the emulator.

The repo runs two independent conformance lanes; both are wired into
CI ([`.github/workflows/conformance.yml`](./.github/workflows/conformance.yml)
and
[`.github/workflows/thirdparty-samples.yml`](./.github/workflows/thirdparty-samples.yml)):

- ✅ **Fixture lane** — purpose-built YAML runner under
  [`conformance/`](./conformance/) (`task conformance:*`,
  `go run ./conformance/cmd/runner`). Seeds catalog state via REST,
  diffs `expected.rows` against the gateway's wire response with
  typed cell comparison (INT64 as `*big.Rat`, FLOAT64 with epsilon,
  RFC3339 / SQL-form timestamps, ...), supports `ordered` /
  `unordered` / `schema_only` matching modes. 18 fixtures today
  spanning SELECT shapes, GROUP BY / aggregates, JOINs, DDL,
  structural errors, and schema-only smokes. JSON output is
  consumed by the coverage publisher
- ✅ **Third-party client lane** — the five official BigQuery
  client-library sample suites are vendored under
  [`third_party/`](./third_party/) and driven by `task thirdparty:*`:
  Go (`cloud.google.com/go/bigquery`, including the Storage Read
  API), Node.js, Python (`google-cloud-bigquery`), Python
  (`bigframes`), and the Java `bigquery-samples` ITs (curated 15-IT
  Failsafe subset). Each suite reads
  `BIGQUERY_EMULATOR_HOST`/`BIGQUERY_STORAGE_GRPC_ENDPOINT` and
  optionally points at `fake-gcs-server` for GCS-backed subtests
- ✅ Per-suite skip rules pin which subtests today land on
  engine-side `UNIMPLEMENTED`s, so a CI regression is "a test that
  used to pass now fails" rather than "a test that never passed is
  still failing" — see `third_party/README.md` for the per-language
  skip matrices
- ⏳ Vendor a subset of the GoogleSQL `.test` corpus and run it
  against the engine via `jobs.query` (catches semantic
  regressions whenever GoogleSQL is upgraded). The GoogleSQL
  parity workflow ([`.github/workflows/googlesql-parity.yml`](./.github/workflows/googlesql-parity.yml))
  is the placeholder for this lane today

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
the catalog/query/storage-read gRPC handlers, and grpc++ in one
binary that serves `Query.DryRun`, `Query.ExecuteQuery`, and
`bigquery_emulator.v1.StorageRead` end-to-end.

GoogleSQL is consumed in one of two modes selected by
`GOOGLESQL_SOURCE` (defaults to `prebuilt`):

- `prebuilt` (default): the `@googlesql//...` Bazel external repo is
  hydrated from a published artifact under `.cache/googlesql-prebuilt/`
  so cold builds finish in a few minutes instead of the multi-hour
  link from source. Populate the cache with
  `task googlesql:fetch-prebuilt URL=<asset> SHA256=<hex>` or
  `task googlesql:stage-bazel`. There is no silent fallback if the
  cache is empty
- `local`: explicit source rebuild from a sibling `../googlesql/`
  checkout (`local_path_override` in
  [`MODULE.bazel`](./MODULE.bazel)). Used when iterating on a
  GoogleSQL upgrade or producer change

DuckDB v1.5.3 is pulled in as a prebuilt `libduckdb` tarball through
`http_archive` (see [`third_party/duckdb/`](./third_party/duckdb/)).
Build with `task emulator:build-engine:bazel` (alias
`task emulator:build-engine`) or directly
`bazel build //binaries/emulator_main:emulator_main`.

Linux/amd64 only today — the GoogleSQL hermetic LLVM toolchain ships
amd64 binaries and does not cross-build cleanly to linux/arm64 yet,
so the engine binary is not produced for arm64. Non-amd64 hosts
should use the published Docker image.

The integration tests under [`gateway/e2e/`](./gateway/e2e/)
discover the engine binary via `./bin/emulator_main`, and
`task emulator:build-engine:bazel` stages the Bazel-built binary +
sibling `libduckdb.so` there with an `rpath` of `$ORIGIN`.

## Open questions / things to investigate

- **Vendoring strategy for GoogleSQL.** Resolved — the engine consumes
  GoogleSQL from a published prebuilt artifact by default
  (`GOOGLESQL_SOURCE=prebuilt`) and falls back to a sibling source
  checkout when explicitly opted in. See "Build systems" above and
  [`docs/dev/googlesql-prebuilt/`](./docs/dev/googlesql-prebuilt/).
- **Subprocess vs. cgo.** Spanner uses subprocess + gRPC. Cleaner ABI, easier
  crash isolation, no cgo build complexity. We start there. Cgo could be
  revisited if subprocess overhead matters in CI runs.
- **Streaming inserts vs. Write API.** `tabledata.insertAll` is the
  implemented path today; `Storage Write API`
  (`google.cloud.bigquery.storage.v1.BigQueryWrite`) is not yet
  wired. The Storage Read API surface lands first; Write API is
  scheduled behind the DML INSERT plan.
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
