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

## Phase 3 - Catalog and storage backend (C++)

Goal: the engine has somewhere to put rows. In-memory only; no persistence.

- ⏳ `backend/storage/`: in-memory column-oriented or row-oriented store
- ⏳ `backend/schema/`: dataset / table / column / type metadata, plus
  conversion to/from `google.cloud.bigquery.v2.TableSchema`
- ⏳ Wire `Catalog` gRPC handlers to the storage backend
- ⏳ Implement `tabledata.insertAll` end-to-end (Go REST -> gRPC -> store)
- ⏳ Implement `tabledata.list` end-to-end (paginated reads)

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

## Phase 5 - Query execution (C++ via GoogleSQL reference impl)

Goal: run real queries against the in-memory tables.

- ⏳ Wire `googlesql::reference_impl::Algebrizer` + `Evaluator` to read from
  the in-memory backend via a `Catalog`/`Table` adapter
- ⏳ Execute `SELECT 1` end-to-end via `jobs.query`
- ⏳ Execute single-table `SELECT * FROM ds.t` end-to-end
- ⏳ Execute joins, GROUP BY, ORDER BY (whatever the reference impl already
  does, no extra effort needed in our code)
- ⏳ Result row marshaling: GoogleSQL `Value` -> BigQuery REST result row
  (`f` array of `v` strings, structured types, NULLs)
- ⏳ Job lifecycle: `pending` -> `running` -> `done`, statistics
  (`creationTime`, `startTime`, `endTime`, `totalBytesProcessed`)

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
- ⏳ Single-stream sessions only (no parallelism); document the limit

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
- No persistence. Emulator is for local dev / CI; data lives only as long as
  the process.
- No production performance. Spanner emulator explicitly carves this out and
  we do the same.
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
