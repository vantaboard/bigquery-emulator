# BigQuery Emulator Roadmap

This document tracks the implementation roadmap for `bigquery-emulator`, a
locally-runnable emulator of the BigQuery REST API. The architecture is
deliberately modeled on Google's [`cloud-spanner-emulator`][spanner]:

- The **engine is C++** and links GoogleSQL directly. GoogleSQL is the
  source of truth for SQL parsing, name resolution, type inference, and
  analysis. Execution sits behind a single `Engine` interface and is
  split into local strategies — DuckDB fast path, DuckDB UDFs /
  rewrites, a local semantic executor, and catalog/control handlers —
  chosen per resolved-AST shape by a local execution router. We do not
  re-port any of GoogleSQL to Go.
- The **frontend is a Go HTTP server** that implements BigQuery's public REST
  surface (projects/datasets/tables/jobs/queries/insertAll/...) and forwards
  query work to the C++ engine over an internal gRPC channel.
- The Go gateway spawns the C++ engine binary as a subprocess on startup and
  shuts it down on exit, identical to the Spanner emulator's
  `gateway_main` -> `emulator_main` pattern.
- Everything runs **local-only**. There is no fallback to the real
  BigQuery service; coverage and parity are responsibilities the
  multi-strategy coordinator owns inside this process.

[spanner]: https://github.com/GoogleCloudPlatform/cloud-spanner-emulator

## Work tracking

Active work to green the ported **query port suite** is tracked via:

- [`scripts/query_port_failures.sh`](scripts/query_port_failures.sh) — classify failures from gateway e2e test results
- [`taskfiles/thirdparty.yml`](taskfiles/thirdparty.yml) — third-party Java sample parity tasks
- Inline deferrals in [`docs/ENGINE_POLICY.md`](docs/ENGINE_POLICY.md), this document ([Planned work](#planned-work)), [`backend/engine/duckdb/transpiler/SHAPE_TRACKER.md`](backend/engine/duckdb/transpiler/SHAPE_TRACKER.md), and disposition YAML headers

Route vocabulary, foundation prerequisites, and engine-wide done criteria live in [`docs/ENGINE_POLICY.md`](docs/ENGINE_POLICY.md).

#### History

The earlier DuckDB-only transpiler plan set (one plan per AST family,
all targeting DuckDB SQL lowering) has been retired. Its landed
accomplishments — scans, filters, projections, joins, aggregates,
windows, set operations, struct / array literals, JSON field access,
Arrow output — carry forward as `duckdb_native` / `duckdb_rewrite`
entries in the new tracker. The earlier 46-plan capability-area
scaffold (one plan per gateway / engine / storage / conformance
slice) was retired before that.

## Engine and storage

The C++ side is structured around two narrow interfaces
(`backend/engine/engine.h`, `backend/storage/storage.h`) so the heavy
machinery can be swapped without touching the gateway, the gRPC
contract, or the conformance harness. Today there is exactly one
public implementation behind each interface; the `Engine`
implementation is itself a local **multi-strategy coordinator** that
routes resolved-AST shapes to the right local executor.

- **`Engine` interface — local execution coordinator**
  (`backend/engine/`). Owns a route classifier that consults the
  shape tracker for each resolved-AST node kind and dispatches to one
  of:

  - **DuckDB fast path** (`backend/engine/duckdb/transpiler/`): emits
    DuckDB-flavored SQL via a custom C++ AST visitor/unparser and
    executes through DuckDB's C++ client. Owns the OLAP-shape wins —
    scans, filters, projections, joins, aggregates, windows, set
    operations, struct / array literals, JSON field access, Arrow
    output.
  - **DuckDB rewrites and UDFs**
    (`backend/engine/duckdb/udf/`): DuckDB SQL expressed in terms of
    UDFs / macros / structural rewrites for the BigQuery functions
    whose semantics are close enough to DuckDB's that they can be
    made correct locally without a separate executor.
  - **Local semantic executor** (`backend/engine/semantic/`): a
    row/array/value interpreter that runs against scanned rows when
    the BigQuery semantics differ enough from DuckDB to make a
    rewrite risky (exact NULL behavior, BigQuery-specific date
    arithmetic edge cases, SAFE-mode error surfaces, DML,
    `UNNEST WITH OFFSET`, scripting, SQL UDF/UDAF/TVF evaluation).
  - **Catalog / control ops** (`backend/engine/control/`): DDL,
    metadata, and other "this is really a storage / catalog
    operation" statements bypass query execution entirely and run
    through the storage layer.

  Each strategy returns rows through the same `RowSource`
  abstraction; callers cannot tell which one served a query (the
  conformance harness can, via route labels).

- **`Storage` interface — DuckDB storage** (`backend/storage/duckdb/`).
  A single `catalog.duckdb` file under `--data_dir` (default
  `$HOME/.bigquery-emulator`) holds the catalog and all table rows;
  the engine reads and writes through DuckDB's C++ client. Survives
  restarts and gives users a real local data warehouse.

### History

An earlier iteration carried a second `Engine` implementation
(ReferenceImpl, on top of `googlesql::reference_impl::Evaluator`)
bridged to DuckDB by a `FallbackEngine` wrapper, plus an in-memory
storage backend for hermetic tests. Both were removed once DuckDB
covered the supported surface; the new multi-strategy coordinator
is the deliberate replacement for that pattern, with route
selection happening at AST-classification time rather than at
runtime error catch time. See [`docs/ENGINE_POLICY.md` § History](./docs/ENGINE_POLICY.md#history)
for the deprecation rationale.

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
  tag-triggered release (`release.yml`). Linux/amd64 engine ships today; a
  non-blocking `build-engine-arm64` lane on `ubuntu-24.04-arm` exercises native
  arm64 builds (see [`docs/dev/googlesql-prebuilt/arm64-feasibility.md`](./docs/dev/googlesql-prebuilt/arm64-feasibility.md))

## Gateway HTTP surface (Go)

Goal: every BigQuery REST endpoint a client library expects has a
route registered. The core CRUD + query surface
(`projects` / `datasets` / `tables` / `tabledata` / `jobs` /
`queries`) is implemented end-to-end against the C++ engine; the
peripheral surfaces (`models`, `routines`, `migration`,
`dataTransfer`) are wired as structurally-valid stubs so
client-library startup probes succeed and per-method behavior is
per-row in [`docs/REST_API.md`](./docs/REST_API.md). **Row-access
policies and column-level data masking** are persisted and enforced
at query time for the synthetic principal (see
[`docs/ENGINE_POLICY.md`](./docs/ENGINE_POLICY.md#row-access-and-column-level-security-mvp)).

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
  - `GET /bigquery/v2/projects/{projectId}/jobs` (`jobs.list`) — honors
    `stateFilter`, `minCreationTime`, `maxCreationTime`, `parentJobId`,
    `maxResults`, and `pageToken` against the in-process job registry
  - `POST /bigquery/v2/projects/{projectId}/jobs` (`jobs.insert`, metadata)
  - `POST /upload/bigquery/v2/projects/{projectId}/jobs` (`jobs.insert`,
    media upload — multipart/resumable per the upstream
    [`api-uploads.md`](./docs/bigquery/docs/reference/api-uploads.md);
    LOAD schema add/relax via `schemaUpdateOptions`)
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
  Transfer routes — see per-method status in
  [`docs/REST_API.md`](./docs/REST_API.md). Highlights: `routines.*` and
  `rowAccessPolicies.*` CRUD round-trip through the engine catalog;
  `models.*` list/get/delete serve CREATE MODEL metadata stubs;
  `INFORMATION_SCHEMA.JOBS*` queries rewrite to gateway job snapshots
  (`gateway/query/info_schema_jobs.go`).
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
implementation of `Storage`, ship the local-coordinator `Engine`
impl (today: DuckDB fast path; the route classifier + DuckDB
UDF/polyfill library + semantic executor + control-op executor land
incrementally behind the same interface — see the Execution
strategies section below), and wire `tabledata.insertAll` /
`tabledata.list` end-to-end against `DuckDBStorage`. The interfaces
stay narrow so additional internal strategies (or, eventually, an
alternate storage layout) can land without disturbing the gateway
or the gRPC contract; today there is exactly one public impl
behind each, and the multi-strategy coordinator is the only
`Engine` implementation callers see.

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
  the prebuilt `libduckdb.so` (linux/amd64 and linux/arm64 zips; engine still
  ships amd64-only until the arm64 lane is green) and surfaces its
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
- ✅ BigQuery DDL routed to the control-op executor: `CREATE TABLE`,
  `CREATE TABLE AS SELECT`, `DROP TABLE`, `ALTER TABLE`,
  `CREATE MATERIALIZED VIEW` (full refresh at creation), `EXPORT DATA`
  (local `file://` writers), and `LOAD DATA` (local `file://` readers)
  flow through `backend/engine/control/control_op_executor.{h,cc}` and
  mutate `Storage` directly (no longer through the DuckDB SQL evaluator).
  `CREATE VIEW` registers in the per-project view registry, and
  `CREATE FUNCTION` / `CREATE AGGREGATE FUNCTION` register through
  the UDF registry (see "DML / DDL" below). Partitioning / clustering
  hints are accepted as metadata; direct external-Parquet drop-in
  remains an open question

### Catalog wiring

- ✅ `Catalog` gRPC handlers backed by `DuckDBStorage`
  (`frontend/handlers/catalog.{h,cc}`)
- ✅ `tabledata.insertAll` end-to-end (Go REST ->
  [`gateway/handlers/tabledata.go`](./gateway/handlers/tabledata.go) ->
  engine gRPC -> `DuckDBStorage`); available to the conformance
  harness as a seeding path that bypasses DML entirely (DML
  `INSERT` also works, via the local DML executor)
- ✅ `tabledata.list` end-to-end (paginated reads, Arrow-batched
  scans on `DuckDBStorage`)

### Engine scaffolding

- ✅ [`backend/engine/duckdb/`](./backend/engine/duckdb/): the DuckDB
  fast path. The transpiler + DuckDB execution surface is the only
  routed strategy with substantial production coverage today; per-shape
  dispositions are tracked in
  [`backend/engine/duckdb/transpiler/SHAPE_TRACKER.md`](./backend/engine/duckdb/transpiler/SHAPE_TRACKER.md)
  using the route-aware status vocabulary
  (`duckdb_native`, `duckdb_rewrite`, `duckdb_udf`,
  `semantic_executor`, `control_op`, `local_stub`, `unsupported`)
- ✅ Local execution router behind `backend/engine/engine.h`: dispatches
  each query to the strategy that fits its resolved-AST shape. Policy
  is documented in [`docs/ENGINE_POLICY.md`](docs/ENGINE_POLICY.md);
  shapes with no planned route surface `UNIMPLEMENTED`.
- ✅ Local semantic executor (`backend/engine/semantic/`) — DML,
  scripting, SQL UDF/UDAF/TVF evaluation, array/struct scans,
  function stubs
- ✅ DuckDB UDF / polyfill library (`backend/engine/duckdb/udf/`)
- ✅ Control-op executor for DDL / metadata (`backend/engine/control/`)

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

Goal: run real queries through the local execution coordinator. The
gateway forwards analyzed queries to `emulator_main` over gRPC and
never branches on engine choice. Inside the engine, the route
classifier inspects the GoogleSQL `ResolvedAST` and dispatches to the
strategy that fits: DuckDB fast path (today's primary), DuckDB
rewrite / UDF, local semantic executor, or a catalog/control-op
handler.

- ✅ [`backend/engine/duckdb/transpiler/`](./backend/engine/duckdb/transpiler/):
  the **DuckDB fast path**. A `googlesql::ResolvedASTVisitor` that
  emits DuckDB SQL strings, implemented one node kind at a time;
  per-shape route disposition (`duckdb_native` /
  `duckdb_rewrite` / `duckdb_udf` / `semantic_executor` /
  `control_op` / `local_stub` / `unsupported`) lives in
  [`SHAPE_TRACKER.md`](./backend/engine/duckdb/transpiler/SHAPE_TRACKER.md).
  `(subset)` rows are property-gated fast-path wins, not missing emits.
  Shapes routed to the other strategies are tracked in
  [`docs/ENGINE_POLICY.md`](docs/ENGINE_POLICY.md) and the Work
  tracking section at the top of this document
- ✅ Type lowering: BigQuery / GoogleSQL types -> DuckDB types
  (`INT64`, `FLOAT64`, `STRING`, `BYTES`, `BOOL`, `DATE`, `TIMESTAMP`,
  `NUMERIC`, `BIGNUMERIC`, `JSON`, `INTERVAL`, `UUID`, `ARRAY<T>`,
  `STRUCT<...>`). `GEOGRAPHY` persists as `VARCHAR` in DuckDB storage;
  the GIS MVP (`ST_GEOGPOINT`, `ST_GEOGFROMTEXT`, `ST_ASTEXT`,
  `ST_DISTANCE`, `ST_WITHIN`, `ST_CONTAINS`, `ST_INTERSECTS`) evaluates
  on the semantic executor; remaining `ST_*` stay `unsupported`
- ✅ STRUCT handling: BQ struct literals + field access lower to
  DuckDB STRUCTs (`{'a': 1}`, `s."a"`); anonymous STRUCT fields
  synthesize positional names (`_0`, `_1`, ...) on both the
  construction and access sides so they round-trip; deep STRUCT
  mutations (`UPDATE t SET s.a.b = ...`) land on the semantic DML
  executor (`backend/engine/semantic/dml/dml_mutate.cc`)
- ✅ UNNEST handling: standalone `UNNEST(arr) AS col` lowers to
  DuckDB `SELECT unnest(arr) AS "col"`. Divergent shapes route to
  the semantic executor (`backend/engine/semantic/array_struct/array_scan.cc`
  + `outer_row_eval` + `MaterializeArrayScan`): `WITH OFFSET`,
  multi-array zip (`array_zip_mode`), `LEFT JOIN UNNEST`, and
  cross-joined `FROM t, UNNEST(t.arr)` — pinned by
  `conformance/fixtures/array_struct/`. `JOIN USING` stays on
  `duckdb_native` (`conformance/fixtures/fastpath/join_using_inner.yaml`).
  FULL OUTER JOIN null-pads unmatched sides on `duckdb_native`; the
  transpiler emits per-side `__bq_j_<column_id>` aliases so duplicate
  column names on both inputs bind independently (pinned by
  `conformance/fixtures/fastpath/scan_join_full.yaml` and
  `conformance/fixtures/core_usage/everyday_sql/full_join.yaml`).
  Lateral `ResolvedJoinScan` (`is_lateral`) evaluates per outer row on
  the semantic executor.
- ✅ Built-in function mapping table — sourced from
  [`backend/engine/duckdb/transpiler/functions.yaml`](./backend/engine/duckdb/transpiler/functions.yaml)
  (~140 BigQuery functions across math, string, datetime,
  conditional, array, aggregation, window, and BQ-specific
  categories). Polyfill UDF lane (`duckdb_udf`) covers
  IF/MOD/DIV/LOG, regex wrappers, and unix-epoch helpers;
  interval datetime arithmetic, `FORMAT_*`, and `CONTAINS_SUBSTR`
  route through the semantic executor. `SQRT(NUMERIC)` routes to
  `semantic_executor` via signature-aware dispatch (pinned by
  `conformance/fixtures/scalar/sqrt_numeric.yaml`). Exact-decimal
  shapes that DuckDB widens to `DOUBLE` (or cannot store) also reroute
  to `semantic_executor`: `AVG` over `NUMERIC`/`BIGNUMERIC`
  (`conformance/fixtures/aggregate/aggregate_numeric_avg.yaml`),
  `NUMERIC`/`BIGNUMERIC` division, and `+`/`-`/`*` over `BIGNUMERIC`
  (which DuckDB persists as `VARCHAR`) — pinned by
  `conformance/fixtures/scalar/numeric_division.yaml` and
  `bignumeric_arithmetic.yaml`. `SUM`/`MIN`/`MAX`/`COUNT` over
  `NUMERIC` stay `duckdb_native` and encode as exact decimal strings
  (`aggregate_numeric_sum.yaml`). A Bazel `genrule` materializes the table into
  `functions_table.inc`, which `functions.cc` includes inside an
  `absl::flat_hash_map`. Each entry records one of the seven
  canonical route dispositions (`duckdb_native`, `duckdb_rewrite`,
  `duckdb_udf`, `semantic_executor`, `control_op`, `local_stub`,
  `unsupported`); every in-scope row is landed (zero `status=planned`
  in [`functions.yaml`](./backend/engine/duckdb/transpiler/functions.yaml)).
  `NET.*`, `HLL_COUNT.*`, and the approximate-aggregate family
  (`APPROX_QUANTILES`, `APPROX_COUNT_DISTINCT`, `APPROX_TOP_COUNT`,
  `APPROX_TOP_SUM`) evaluate on the semantic executor (`net_funcs.cc`,
  `hll_funcs.cc`, `aggregate_specialized.cc`). Deferred families that
  today surface `UNIMPLEMENTED` but are tracked as ⏳ planned work
  (see [Planned work](#planned-work) and the ENGINE_POLICY index table
  there) include BigQuery ML inference (**stub**), differential-privacy /
  anonymized aggregates (**stub**), `SESSION_USER` (**stub**), and the
  real implementations of protobuf /
  sequence AST nodes, MEASURE functions, Python UDFs, `ST_GEOGFROMWKB`,
  `KLL_QUANTILES.*`, and cloud-backed external data sources, plus related rows in
  [`node_dispositions.yaml`](./backend/engine/duckdb/transpiler/node_dispositions.yaml).
  Graph / GQL (`ResolvedGraph*Scan`) stays `unsupported` and is **not**
  planned.
  The current posture for each is documented in
  [`docs/ENGINE_POLICY.md`](docs/ENGINE_POLICY.md). The legacy
  `kMap`/`kFallback`/`kSkiplist` vocabulary was retired.
  `SAFE.<fn>(...)` is handled uniformly regardless of disposition
- ✅ DuckDB fast-path execution: file-backed `catalog.duckdb`
  connection, the transpiled SQL is bound and executed via the DuckDB
  C++ client, DuckDB errors are translated back to the BigQuery error
  envelope. Routes that resolve to the semantic executor / control-op
  handlers will reuse the same connection for storage scans but bypass
  DuckDB's SQL engine for the BigQuery-semantics-sensitive evaluation
  step
- ✅ Result marshaling: DuckDB Arrow `RecordBatch` -> BigQuery REST
  row JSON (column-major Arrow walked into the row-major `f`/`v`
  shape; NULL bitmap honored; nested STRUCTs/ARRAYs recursed). The
  Storage Read API path reuses the Arrow batches directly without
  this conversion
- ✅ Job lifecycle: `pending` -> `running` -> `done`, statistics
  (`creationTime`, `startTime`, `endTime`, `totalBytesProcessed`)
  surfaced through [`gateway/jobs/`](./gateway/jobs/)

## Execution strategies

The router maps every `ResolvedAST` shape to one of seven route
dispositions; the same vocabulary lives in
[`backend/engine/duckdb/transpiler/SHAPE_TRACKER.md`](./backend/engine/duckdb/transpiler/SHAPE_TRACKER.md)
on a per-node basis and in
[`docs/ENGINE_POLICY.md`](./docs/ENGINE_POLICY.md) as the
public-facing policy.

| Route                | What it is                                                                                  | Tracking                                          |
|----------------------|---------------------------------------------------------------------------------------------|---------------------------------------------------|
| `duckdb_native`      | Lowers directly to DuckDB SQL with semantics that already match BigQuery's exactly.         | [`SHAPE_TRACKER.md`](./backend/engine/duckdb/transpiler/SHAPE_TRACKER.md) |
| `duckdb_rewrite`     | Lowers to DuckDB SQL with a deliberate rewrite (e.g. struct/array shape rewrites, BigQuery JSON operators -> DuckDB JSON operators). | [`SHAPE_TRACKER.md`](./backend/engine/duckdb/transpiler/SHAPE_TRACKER.md) |
| `duckdb_udf`         | Adds a DuckDB UDF / macro to make the BigQuery function correct locally.                    | [`SHAPE_TRACKER.md`](./backend/engine/duckdb/transpiler/SHAPE_TRACKER.md) |
| `semantic_executor`  | Runs on a local row/value interpreter that owns exact BigQuery semantics; bypasses DuckDB SQL evaluation. | [`SHAPE_TRACKER.md`](./backend/engine/duckdb/transpiler/SHAPE_TRACKER.md) |
| `control_op`         | DDL / metadata / catalog ops routed straight through the storage layer.                     | [`SHAPE_TRACKER.md`](./backend/engine/duckdb/transpiler/SHAPE_TRACKER.md) |
| `local_stub`         | Specialized feature accepted at parse / analyzer but evaluated against a deterministic BigQuery-shaped placeholder (`KEYS.NEW_KEYSET`, `KEYS.KEYSET_LENGTH`, `CREATE MODEL`) so client-library startup probes succeed. | [`docs/ENGINE_POLICY.md`](docs/ENGINE_POLICY.md) |
| `unsupported`        | Deliberately out of scope locally. Surfaces a BigQuery-shaped `UNIMPLEMENTED` error naming the family + linking to `docs/ENGINE_POLICY.md`. | [`docs/ENGINE_POLICY.md`](docs/ENGINE_POLICY.md) |

- ✅ Route classifier behind `Engine::Analyze` /
  `Engine::ExecuteQuery`
- ✅ Per-shape dispositions recorded in the shape tracker
  (seven-route vocabulary + node/function YAML registries are closed
  for in-scope shapes; every `unsupported` / deferred `local_stub`
  family in [`docs/ENGINE_POLICY.md`](docs/ENGINE_POLICY.md) is indexed
  as ⏳ planned in [Planned work](#planned-work))
- ✅ Route labels surfaced on conformance fixture output so
  passing rows can't hide accidental drift between strategies.
  The conformance routing matrix added the
  `emulatorRoute` debug field on `Job.statistics.query`
  (loopback-only, gated by `gateway/middleware/loopback.go`
  so only local-loopback callers see it), extended the
  conformance fixture schema with `expected.route` /
  `route_strict` / `route_allowlist`, backfilled every
  fixture under `conformance/fixtures/` (192 YAML files),
  and added `task conformance:routing-matrix` for a
  reviewable Markdown snapshot of every shape's route. The
  `conformance.yml` CI workflow uploads the matrix as a
  non-blocking artifact.
- ✅ CTE / subquery routing. Non-recursive CTEs
  (`ResolvedWithScan` / `ResolvedWithRefScan`) lower to DuckDB
  `WITH "a" AS (...)` natively. Non-correlated scalar / IN /
  EXISTS / ARRAY `ResolvedSubqueryExpr` forms lower directly to
  DuckDB. Correlated subqueries (`parameter_list()` non-empty)
  evaluate per outer row on the semantic executor via
  `EvalSubqueryExpr` + `outer_row_eval` (pinned by
  `conformance/fixtures/cte_subquery/subquery_expr_correlated_exists.yaml`).
  Recursive CTEs lower through `duckdb_rewrite` (`WITH RECURSIVE`),
  pinned by `conformance/fixtures/advanced_relational/recursive_cte.yaml`;
  `WITH DEPTH` depth pseudo-columns are unit-tested when the analyzer
  supplies `recursion_depth_modifier` (no parse surface in
  `PRODUCT_EXTERNAL` yet).
  LIKE ANY / ALL list forms evaluate on the semantic executor
  (`conformance/fixtures/cte_subquery/subquery_expr_like_any_list.yaml`).
  Window-frame RANGE on numeric ORDER BY keys and TABLESAMPLE
  `REPEATABLE` lower on the DuckDB fast path (`conformance/fixtures/window/`,
  `conformance/fixtures/sample/sample_repeatable_seed.yaml`). Weighted /
  stratified `TABLESAMPLE` and DATE/TIMESTAMP `RANGE` frames with numeric
  offsets evaluate on the semantic executor (`sample_weighted_percent.yaml`,
  `sample_stratified_percent.yaml`, `window_frame_range_date.yaml`).
  `MATCH_RECOGNIZE` evaluates on the semantic executor
  (`conformance/fixtures/advanced_relational/match_recognize_pattern_ab.yaml`)
- ✅ Cast / collation / value-table / set-op edges. `CAST ... FORMAT` /
  `CAST ... AT TIME ZONE` promote to the semantic executor
  (`eval_expr_cast.cc` via googlesql `CastFormat*` / `CastStringTo*`);
  in-scope `extended_cast()` shapes evaluate on `eval_expr_cast_extended.cc`;
  `STRING(n)` / `NUMERIC(p,s)` type modifiers evaluate on the same path
  (pinned by `cast_type_modifiers.yaml` and `cast_extended_geography_string.yaml`).
  `ORDER BY ... COLLATE 'und:ci'` sorts case-insensitively on the
  semantic path (`scan_eval_scan_impl.cc`). `SELECT AS VALUE`
  (`is_value_table()`) evaluates on the semantic executor. Set-op
  `CORRESPONDING` / `CORRESPONDING BY` reuse the analyzer's per-item
  column projection on `duckdb_native` (pinned by
  `conformance/fixtures/scalar/cast_*`, `order_by_collate_und_ci.yaml`,
  `select_as_value_scalar.yaml`, and
  `conformance/fixtures/setops/set_op_corresponding_union_all.yaml`).

## DML / DDL

- ✅ DML routed by shape. All `MERGE` statements — including
  single-branch `WHEN MATCHED` / `WHEN NOT MATCHED BY TARGET` shapes
  and the full matrix (`WHEN NOT MATCHED BY SOURCE`, multi-action
  sequences) — route through the storage-aware local DML executor
  (`backend/engine/semantic/dml/dml_merge.cc`). The DuckDB
  verbatim-SQL MERGE path is retired: DuckDB cannot parse BigQuery MERGE
  surface syntax (`MERGE table AS T`, `WHEN MATCHED AND …`). `INSERT
  VALUES`, `INSERT ... SELECT`, scalar- and deep-STRUCT `SET`
  `UPDATE`, proto `UpdateConstructor` SET, `UPDATE ... FROM`, `DELETE`,
  `TRUNCATE TABLE` (via `RunTruncateTable` in
  `control_op_ddl.cc` with a coordinator `ExecuteDml` bridge that
  returns `dmlStats.deletedRowCount`),
  `THEN RETURN` on INSERT/UPDATE/DELETE (GoogleSQL does not define
  MERGE `THEN RETURN`), and `ASSERT_ROWS_MODIFIED` route through the
  semantic DML executor and populate `numDmlAffectedRows` correctly.
  Nested `(DELETE ... WITH OFFSET ...)` inside `UPDATE SET` lands on
  `ApplyNestedArrayDeleteItem` (pinned by
  `update_delete_array_offset.yaml`). Pipe INSERT (`ResolvedPipeInsertScan`
  via `ExecuteDml` on generalized query statements) is landed on the
  semantic DML executor. Conformance fixtures may seed rows via either
  `tabledata.insertAll` or `INSERT VALUES` `sql:` steps. See
  [`docs/ENGINE_POLICY.md`](./docs/ENGINE_POLICY.md) for the per-shape
  routing decisions.
- ✅ DDL routed to control ops. `CREATE TABLE`, `CREATE TABLE AS
  SELECT`, `DROP TABLE`, `ALTER TABLE`, `CREATE
  MATERIALIZED VIEW` (full refresh at creation), `EXPORT DATA` (local
  `file://`), and `LOAD DATA` (local `file://`) are wired via
  `ControlOpExecutor`, which mutates `Storage` directly and emits a
  BigQuery-shaped envelope (`Job.statistics.query.statementType`
  populated per the BigQuery REST reference). The DuckDB SQL evaluator
  no longer carries any DDL paths — the route classifier dispatches
  every `control_op` statement root to `ControlOpExecutor`.
  `CREATE VIEW` registers in the per-project view registry and replays
  into each query catalog (`sys_calendar` bqutils fixture promoted).
  `CREATE FUNCTION` / `CREATE AGGREGATE FUNCTION` register through
  the UDF registry (next bullet). `FOR SYSTEM_TIME AS OF` table
  decorators and snapshot clone route through control ops
  (`control_op_time_travel.cc`, `duckdb_storage_version_log.*`).
  (`ANALYZE` and `UNDROP TABLE` were removed — neither is a BigQuery
  statement; `UNDROP SCHEMA` restores soft-deleted datasets from
  `.tombstones/__dataset__/` and is wired through `RunUndrop` plus
  REST `datasets.undelete`.) Wildcard tables
  (`dataset.prefix_*`) resolve in the virtual catalog with
  `_TABLE_SUFFIX` filtering (`wildcard_table_suffix_filter.h`).
  Cloud-storage `gs://` URIs for `EXPORT DATA` / `LOAD DATA` remain
  unsupported today; see [External data sources](#external-data-sources).
- ✅ Scripting / UDFs / TVFs routed to a local scripting executor
  — see `docs/ENGINE_POLICY.md`. `ASSERT <expr> [AS '<msg>']`
  lands on the new `backend/engine/semantic/script/` package and
  surfaces BigQuery's documented `Assertion failed` envelope; the
  scripting variable environment has been generalized into the
  shared `semantic::FrameStack` primitive so the UDF / TVF call-
  side reuses the same stack-of-frames type. `ResolvedConstant`
  and `ResolvedArgumentRef` resolve on the semantic executor
  (case-insensitive identifier matching for arg refs). **Landed
  (partial):** `CREATE PROCEDURE` registers in `procedure_registry`;
  `CALL` dispatches via `script_executor.cc` with SQL body execution;
  gateway engine-first scripting (`DECLARE`/`CALL`/`BEGIN` in one
  `ExecuteQuery`) preserves variable scope. Four bqutils README goldens
  are in `passing/stored_procedures/` (2026-06-07). **Landed:** `SET`
  assignment, `IF`/`WHILE`, `EXECUTE IMMEDIATE`, `BEGIN…EXCEPTION` with
  `@@error.message` / `@@error.statement_text`, and `RAISE USING MESSAGE`
  (conformance/fixtures/scripting/, 2026-06-10). **Landed:** `LOOP`/`BREAK`,
  `FOR…IN`, `REPEAT…UNTIL`, and nested `EXCEPTION` inside `WHILE` (pinned by
  `loop_break_accumulation.yaml`, `for_in_sum.yaml`, `repeat_until.yaml`,
  `exception_in_while.yaml`). **Landed:** `CREATE FUNCTION` / `CREATE TABLE FUNCTION` /
  `CREATE PROCEDURE` write through to `DuckDBStorage` (`__bqemu_routines`)
  and rehydrate across engine restarts; `DROP FUNCTION` removes registry +
  storage rows. REST `routines.*` delegates to `Catalog` RPCs backed by
  the same store. `LANGUAGE js` scalar UDFs register through
  `js_udf_registry.cc` and evaluate at call time on the semantic
  executor via embedded Duktape (`js_udf_runtime.cc`; pinned by
  `conformance/fixtures/udf/js_scalar_add.yaml`). Scalar / `ANY TYPE`
  SQL UDFs, SQL UDAFs, and SQL TVFs evaluate on the semantic executor;
  TABLE-typed TVF parameters materialize via `ResolvedRelationArgumentScan`
  (pinned by `conformance/fixtures/udf/tvf_relation_argument.yaml`).
  Conformance fixtures under `conformance/fixtures/udf/` (+
  `gateway/e2e/routine_persistence_test.go` for restart proof).
  REST-created views (`tables.insert` with `view.query`) persist through
  the same engine path and survive restart
  (`gateway/e2e/restart_durability_test.go`,
  `conformance/sessions/restart_view_durability.yaml`). Operators
  migrating from recidiviz/goccy `--database=/path/file.db` should switch
  to `--data-dir=/path` (see [REST API persistence notes](./docs/REST_API.md#persistence-and-data-dir)).
  `LANGUAGE python` scalar UDFs register through `python_udf_registry.cc`
  and evaluate at call time on the semantic executor via a sandboxed
  `python3` subprocess (`python_udf_runtime.cc`; pinned by
  `conformance/fixtures/udf/python_scalar_add.yaml`). See
  [Python UDFs](#python-udfs). Non-scalar JS UDFs remain unsupported;
  non-scalar Python UDF shapes are sharpened rejects (bq dry-run rejects
  `CREATE AGGREGATE FUNCTION ... LANGUAGE python` and
  `CREATE TABLE FUNCTION ... LANGUAGE python`).
- ✅ Job stats: `numDmlAffectedRows` populated for DML shapes the
  local DML executor lands (INSERT, UPDATE, DELETE, TRUNCATE, all MERGE
  shapes, `THEN RETURN`). The
  gateway folds `dmlStats` into the legacy aggregate
  (inserted + updated + deleted) per the BigQuery REST contract.
  Pipe INSERT populates stats via the same DML executor path.

## Storage Read API (gRPC)

Goal: support BigQuery client libraries that prefer the Storage Read API
(`google.cloud.bigquery.storage.v1`) for fast reads.

- ✅ `BigQueryRead` gRPC service implemented
  ([`frontend/handlers/storage_read.{h,cc}`](./frontend/handlers/));
  `CreateReadSession`, `ReadRows`, and `SplitReadStream` are wired;
  multi-stream sessions partition on deterministic `file_row_number`
  ranges snapshotted at session mint time
- ✅ Public `google.cloud.bigquery.storage.v1.BigQueryRead` service
  registered on `:9060` by the gateway shim
  ([`gateway/handlers/bqstorage/`](./gateway/handlers/bqstorage/)),
  adapting the engine's internal `DataRow` stream to the public
  wire formats: Arrow schema + IPC record batches
  (`arrow.go` / `arrow_ipc.go`) and Avro OCF encoding (`avro.go`)
- ✅ `ReadOptions.row_restriction`: restrictions are analyzed with
  GoogleSQL against the table schema and transpiled into a DuckDB
  `WHERE` clause on the `read_parquet(...)` scan (comparisons,
  connectives, `IS NULL`, and other transpiler-supported shapes).
  Unsupported constructs reject at `CreateReadSession` with
  `INVALID_ARGUMENT` (see "Supported `ReadOptions`" in
  [`docs/REST_API.md`](./docs/REST_API.md))
- ✅ `ReadOptions.selected_fields` enforced. CreateReadSession
  validates each name against the table's top-level columns
  (unknowns surface as `INVALID_ARGUMENT` before any streaming RPC
  starts) and the DuckDB backend emits a projected SELECT so the
  `ReadRows` stream yields rows in caller-pinned column order. The
  `ReadSession.schema` reply reflects the projection too
- ✅ Multi-stream sessions — `max_stream_count > 1` mints disjoint
  `file_row_number` partitions (server may return fewer streams than
  requested). `SplitReadStream` subdivides a stream's remaining range

## Storage Write API (gRPC)

Goal: support BigQuery client libraries that prefer the Storage
Write API (`google.cloud.bigquery.storage.v1.BigQueryWrite`) for
append-only writes that ride the same gRPC surface the read path
already uses.

- ✅ `BigQueryWrite` gRPC service implemented
  ([`frontend/handlers/storage_write.{h,cc}`](./frontend/handlers/));
  `CreateWriteStream`, bidi-streaming `AppendRows`, and
  `GetWriteStream` are wired for the `COMMITTED` and reserved
  `_default` stream types (including ManagedWriter DefaultStream
  with the full NUMERIC/BIGNUMERIC proto type matrix). Append
  batches commit immediately through `DuckDBStorage::AppendRows`,
  the same primitive the local DML executor uses
- ✅ Schema-shape mismatches and recoverable storage errors land
  on the `AppendRowsResponse.error_message` envelope so a producer
  can fix the batch and retry without tearing the bidi stream down,
  matching the public Storage Write API's recoverable-error
  contract
- ✅ `BUFFERED` stream type: `CreateWriteStream`, buffered
  `AppendRows`, `FlushRows`, and `FinalizeWriteStream` commit
  through `DuckDBStorage::AppendRows` on flush. The public
  `BigQueryWrite` gRPC shim registers on `:9060`
  ([`gateway/handlers/bqstorage/`](./gateway/handlers/bqstorage/));
  Java `WriteBufferedStreamIT` and `StorageArrowSampleIT` pass
  against the local emulator (see
  [`docs/ENGINE_POLICY.md`](./docs/ENGINE_POLICY.md) Storage gRPC
  section).
- ✅ `PENDING` + `BatchCommitWriteStreams`: `CreateWriteStream`,
  buffered `AppendRows`, `FinalizeWriteStream`, and atomic
  `BatchCommitWriteStreams` commit through `DuckDBStorage::AppendRows`.
  Java `WritePendingStreamIT` and Go `PendingStream` managedwriter
  subtests run against the local emulator when Storage gRPC is set.

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
  `unordered` / `schema_only` matching modes. **190+ YAML fixtures**
  today spanning SELECT shapes, GROUP BY / aggregates, JOINs, CTEs /
  subqueries, DML / DDL round-trips, functions, scripting / UDFs,
  time travel, wildcard tables, GIS, row/column security,
  `MATCH_RECOGNIZE`, `INFORMATION_SCHEMA`, structural errors, and
  schema-only smokes (plus the bigquery-utils suite under
  `conformance/thirdparty-fixtures/`). When a fixture disagrees with
  production BigQuery, validate the SQL with the `bq` CLI before
  assuming an emulator bug (see
  [`.cursor/rules/conformance-bq-validation.mdc`](./.cursor/rules/conformance-bq-validation.mdc)).
  JSON output is consumed by the coverage publisher
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
- ✅ Vendor a subset of the GoogleSQL `.test` corpus and run it
  against the engine via `jobs.query` (catches semantic
  regressions whenever GoogleSQL is upgraded). Vendored subset:
  `logical_functions`, `arithmetic_functions`, `math_functions`,
  `cast_format_validation`, and `regexp_functions` under
  `conformance/googlesql-corpus/corpus/` (85 pinned cases); widen
  further via [`conformance/googlesql-corpus/README.md`](./conformance/googlesql-corpus/README.md).
  CI: `googlesql-corpus` job in
  [`.github/workflows/googlesql-parity.yml`](./.github/workflows/googlesql-parity.yml);
  local: `task conformance:googlesql-corpus`

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
- ✅ MkDocs Material documentation site published via GitHub Pages
  ([`https://vantaboard.github.io/bigquery-emulator/`](https://vantaboard.github.io/bigquery-emulator/);
  source under [`docs/`](./docs/), build config in [`mkdocs.yml`](./mkdocs.yml))
- ✅ Runtime configuration documented in README §Runtime configuration;
  the conformance harness (`conformance/README.md`) and the engine
  policy (`docs/ENGINE_POLICY.md`) cover the local-only execution
  model and the per-route catalog. `emulator_main` accepts
  `--host_port` and `--data_dir` only; `--engine` / `--storage` /
  `--on_unknown_fn` were removed when the ReferenceImpl + in-memory
  backends were deleted, and the multi-strategy coordinator
  intentionally keeps that same single-knob posture (route selection
  happens internally at AST-classification time, not via a runtime
  flag).
- ✅ Install / launch flow documented in README §Quickstart,
  docs/DOCKER.md §Install via Docker, docs/RELEASES.md §Install via release
  archive, and §Pointing client libraries at the emulator (`docker run` +
  `BIGQUERY_EMULATOR_HOST` env-var override). Note: there is **no**
  `gcloud emulators bigquery start` subcommand — gcloud ships
  emulator subgroups only for Firestore and Spanner (plus
  pubsub / datastore / bigtable in alpha/beta), so the "gcloud
  install path" some users expect by analogy with Spanner does
  not exist for BigQuery and is not on the roadmap to add

---

## Planned work

The items below are deliberately deferred today (`unsupported` in
[`functions.yaml`](./backend/engine/duckdb/transpiler/functions.yaml) and
[`node_dispositions.yaml`](./backend/engine/duckdb/transpiler/node_dispositions.yaml);
summarized in [`docs/ENGINE_POLICY.md`](docs/ENGINE_POLICY.md)) but are on
the roadmap to land with **local** implementations. Query execution never
proxies through the real BigQuery service; external data sources (below) may
optionally reach live upstream APIs when configured. Each promotion off
`unsupported` requires the handler, conformance fixture(s), and
disposition-registry update in the same commit (per
[`docs/ENGINE_POLICY.md`](docs/ENGINE_POLICY.md)).

The table below mirrors the `unsupported` / `local_stub` families in
[`docs/ENGINE_POLICY.md`](docs/ENGINE_POLICY.md) §Unsupported families.
Every row is ⏳ planned. **Planned work is one of two kinds:**

- **real** — the feature is useful locally, so land exact BigQuery
  semantics + conformance fixtures.
- **stub** — the feature is not useful in a local emulator, so the only
  goal is that a query referencing it **does not fail**: return a
  schema-correct BigQuery-shaped placeholder via `local_stub`.

| ENGINE_POLICY family | Today | Planned | Plan work |
|---|---|---|---|
| BigQuery ML inference (`ML.PREDICT`, `ML.FORECAST`, `ML.EVALUATE`) | `local_stub` | **stub** (landed) | [BigQuery ML](#bigquery-ml) |
| BigQuery ML `CREATE MODEL` | `local_stub` | stub (stays) | [BigQuery ML](#bigquery-ml) |
| Differential privacy / anonymized aggregation | `local_stub` | **stub** (landed) | [Privacy-preserving aggregates](#privacy-preserving-aggregates) |
| `SESSION_USER` (`session_user`) | `local_stub` | **stub** (landed) | [Deferred built-in functions](#deferred-built-in-functions) |
| `ST_GEOGFROMWKB` (`st_geogfromwkb`) | `local_impl` | **real** (landed) | [Deferred built-in functions](#deferred-built-in-functions) |
| KLL quantile sketches (`KLL_QUANTILES.*`) | `local_impl` | **real** (landed) | [KLL quantile sketches](#kll-quantile-sketches) |
| MEASURE / measure functions | `local_impl` | **real** | [Measure functions](#measure-functions) |
| Sequences (`ResolvedSequence`, `NEXT VALUE FOR`) | `unsupported` | sharpened (not reachable) | [Catalog / sequence helpers](#catalog--sequence-helpers) |
| Expression columns (`ResolvedExpressionColumn`) | `semantic_executor` | **real** (landed) | [Catalog / sequence helpers](#catalog--sequence-helpers) |
| Catalog column refs (`ResolvedCatalogColumnRef`, non-graph) | `unsupported` | sharpened (not reachable) | [Catalog / sequence helpers](#catalog--sequence-helpers) |
| Python UDFs (`CREATE FUNCTION ... LANGUAGE python`) | `local_impl` | **real** | [Python UDFs](#python-udfs) |
| `LOAD DATA <gs://...>` (cloud storage) | `unsupported` | **real** | [External data sources](#external-data-sources) |
| `UNDROP SCHEMA` | `control_op` | **real** (landed) | [DML / DDL](#dml--ddl) (`RunUndrop` + `datasets.undelete`) |
| SQL Tools API (format/parse/complete/analyze) | ✅ landed | **real** (landed) | [SQL Tools API](#sql-tools-api) (M4 query editor) |

> **Graph / GQL (`GRAPH_TABLE`, GQL subqueries, `ResolvedGraph*Scan`) is
> NOT planned.** It is effectively a whole second query language and is
> not worth modeling in a local emulator; it stays `unsupported` (see
> [Non-goals](#non-goals)).

### SQL Tools API

Opt-in gateway routes (`--enable-sql-tools-api`) expose GoogleSQL format,
parse, tokenize, catalog-aware completion, and query analysis for downstream
query-editor UIs. See [`docs/SQL_TOOLS_API.md`](docs/SQL_TOOLS_API.md).

- ✅ Format / parse / tokenize / complete — engine-backed parser and formatter
- ✅ Diagnostic spans + UTF-16 offset contract — CodeMirror integration
- ✅ Routines, views, FQNs, column metadata in completion
- ✅ In-scope column completion via analyzer
- ✅ `POST /analyze` — referenced tables for reference panels
- ✅ `GET /capabilities` — UI feature probe
- Docker default enables `--enable-sql-tools-api` via `docker/gateway_main.sh`

### BigQuery ML

BigQuery ML depends on Vertex AI / real model training + serving that a
local emulator cannot meaningfully provide, so the plan is **stub, not
implement** — the only goal is that ML.* does not fail a query.
`CREATE MODEL` already routes `local_stub` (metadata-only OK); the
inference calls below get the same treatment.

- ✅ `ML.PREDICT` (`ml.predict`) — **stub**: input pass-through + NULL
  predicted columns (`backend/engine/semantic/stubs/ml.cc`)
- ✅ `ML.FORECAST` (`ml.forecast`) — **stub**: single forecast-schema row
  of NULLs
- ✅ `ML.EVALUATE` (`ml.evaluate`) — **stub**: single metrics row of NULLs
- `CREATE MODEL` — stays `local_stub` (no model is trained or stored)

### Deferred built-in functions

Rows in [`functions.yaml`](./backend/engine/duckdb/transpiler/functions.yaml):

- ✅ `ST_GEOGFROMWKB` (`st_geogfromwkb`) — **real**: 2D WKB POINT → `GEOGRAPHY`
  on the semantic GIS path (`geog_funcs.cc`); pinned by
  `conformance/fixtures/specialized/st_geogfromwkb_point.yaml`
- ✅ `SESSION_USER` (`session_user`) — **stub**: returns
  `bigquery-emulator@local`; pinned by
  `conformance/fixtures/specialized/session_user_stub.yaml`

### KLL quantile sketches

`KLL_QUANTILES.*` are real BigQuery approximate-quantile sketch functions
(verified via `bq query --dry_run`), complementing the landed
`HLL_COUNT.*`. They support partial aggregation / re-aggregation across
`INIT`/`MERGE`/`MERGE_PARTIAL`/`MERGE_POINT`/`EXTRACT`/`EXTRACT_POINT` for
both `INT64`- and `FLOAT64`-initialized sketches.

- ✅ `KLL_QUANTILES.*` (`kll_quantiles.*`) — **real** (landed): semantic
  executor via [`kll_funcs.cc`](./backend/engine/semantic/functions/kll_funcs.cc)
  (Apache DataSketches KLL + emulator-local wire format; not byte-compatible
  with cloud BigQuery). Pinned by
  [`conformance/fixtures/specialized/kll_quantiles_round_trip.yaml`](./conformance/fixtures/specialized/kll_quantiles_round_trip.yaml).

### Deferred AST node dispositions

Rows in
[`node_dispositions.yaml`](./backend/engine/duckdb/transpiler/node_dispositions.yaml):

#### Privacy-preserving aggregates

Differential-privacy / anonymized-aggregation scans from
[`docs/ENGINE_POLICY.md`](docs/ENGINE_POLICY.md) (`AnonymizedAggregate*`,
`DifferentialPrivacyAggregate*`, ...). Differential privacy only matters
in production and the emulator cannot honor the guarantee, so these are
**stub** — strip the privacy modifiers and return the plain underlying
aggregate so the query does not fail. This is **not** differential
privacy.

- ✅ `ResolvedAnonymizedAggregateScan` — **stub**: privacy modifiers
  stripped; plain aggregate via `scan_eval_aggregate.cc`
- ✅ `ResolvedDifferentialPrivacyAggregateScan` — **stub**: same
- ✅ `ResolvedAggregationThresholdAggregateScan` — **stub**: same

#### Protobuf field access — removed (not a BigQuery surface)

PROTO construction / field access and `REPLACE_FIELDS` / `FILTER_FIELDS`
are **not** reachable in BigQuery PRODUCT_EXTERNAL (verified via
`bq query --dry_run`: `NEW <proto>` → "Type not found", `REPLACE_FIELDS()`
→ "is not supported", `FILTER_FIELDS` → "Function not found"). These nodes
are now `unsupported` and the handlers were removed.

- ✅ `ResolvedGetRowField` — **kept** (`semantic_executor`): value-table /
  range-variable row field access (`t.f`) IS a real BigQuery shape (bq
  accepts it); evaluated in `eval_expr_proto.cc`.

#### Catalog / sequence helpers

`ResolvedSequence` / `NEXT VALUE FOR` and catalog-internal column refs
from [`docs/ENGINE_POLICY.md`](docs/ENGINE_POLICY.md). **Real**, but land
only the shapes a non-graph BigQuery query can actually reach; otherwise
keep `unsupported` with a sharper envelope.

- ✅ `ResolvedSequence` — no reachable `PRODUCT_EXTERNAL` `CREATE SEQUENCE` /
  `NEXT VALUE FOR` SQL today; stays `unsupported` with a sharper envelope (see
  `docs/ENGINE_POLICY.md`)
- ✅ `ResolvedCatalogColumnRef` — graph-property refs out of scope; catalog DDL
  refs have no reachable `PRODUCT_EXTERNAL` consumer today (stays
  `unsupported`; eval surfaces a sharper envelope if the node appears)
- ✅ `ResolvedExpressionColumn` — `AnalyzeExpression` bindings on the semantic
  executor (`expression_column_bindings.{h,cc}`, `eval_expr.cc`); pinned by
  `expression_column_set_increment.yaml`

### Measure functions

MEASURE types and `AGGREGATE(<measure>)` from
[`docs/ENGINE_POLICY.md`](docs/ENGINE_POLICY.md). Measure columns register
via `measure_catalog` (`bqemu_measure:<expr>:<row_keys>` on table schema
descriptions); `AGG(<measure>)` expands through the analyzer rewrite into
multi-level aggregates on the semantic executor.

- ✅ MEASURE type surface (`measure_catalog.{h,cc}`) and `AGG(<measure>)`
  evaluation — pinned by
  `conformance/fixtures/specialized/measure_agg_group_by.yaml`

### Python UDFs

`CREATE FUNCTION ... LANGUAGE python` from
[`docs/ENGINE_POLICY.md`](docs/ENGINE_POLICY.md). Complements the landed
`LANGUAGE js` scalar path (`js_udf_runtime.cc`); `cw_xml_extract` promoted
from bqutils `known_failing/` to `passing/`.

- ✅ `CREATE FUNCTION ... LANGUAGE python` — register, persist, and
  evaluate scalar Python UDF bodies locally via `python_udf_runtime.cc`
  (sandboxed `python3` subprocess; `BIGQUERY_EMULATOR_PYTHON` override)
- ✅ Scalar Python UDF conformance pinned by
  `conformance/fixtures/udf/python_scalar_add.yaml`
- ✅ Non-scalar Python UDF shapes sharpened to match production BigQuery
  (bq dry-run rejects aggregate + table-valued `LANGUAGE python` forms;
  pinned by `conformance/fixtures/udf/python_udaf_rejected.yaml` and
  `python_tvf_rejected.yaml`)
- ✅ Bounded `packages` support — declared packages round-trip on
  `routines.get` (`pythonOptions.packages`); resolve against
  `BIGQUERY_EMULATOR_PYTHON` → `$data_dir/python-udf-env/` → host `python3`;
  preflight `importlib` miss surfaces a structured error; operator provisioning
  via `task python-udf:provision` (`BIGQUERY_EMULATOR_PYTHON_ALLOW_PIP=1` only).
  See [`docs/guides/python-udfs.md`](docs/guides/python-udfs.md). Pinned by
  `conformance/fixtures/udf/python_packages_lxml.yaml` and
  `python_packages_missing.yaml`.

### External data sources

Today the emulator supports local `file://` and `gs://` (via
`$data_dir/external/gcs-cache/` or `STORAGE_EMULATOR_HOST` / fake-gcs) for
`LOAD DATA` / `EXPORT DATA`, GCS-backed **external tables**, **Google
Sheets** external tables (fixture snapshot + opt-in live fetch), and
ephemeral `tableDefinitions` for Sheets/GCS. See
[`docs/ENGINE_POLICY.md`](docs/ENGINE_POLICY.md) and
[`gateway/external/sourceconfig/`](gateway/external/sourceconfig/).

**Configuration model** (✅)

- **Fixture / local** — per-source `fixture | local | live` resolution via
  `$data_dir/external_sources.yaml` and env vars (`BIGQUERY_EMULATOR_LIVE_SHEETS`,
  `BIGQUERY_EMULATOR_EXTERNAL_GCS_MODE`, …). Defaults: GCS `local`, Sheets
  `fixture`, connections `fixture`.
- **Live** — opt-in (`BIGQUERY_EMULATOR_LIVE_SHEETS=1`) unauthenticated CSV
  export for the public sample sheet; private sheets still need Sheets API
  credentials.

**Surface areas**

- ✅ `gs://` URIs for `LOAD DATA` / `EXPORT DATA` (`control_op` via
  `gcs_uri_resolver.cc`)
- ✅ Google Sheets external tables (`gateway/external/sheets.go`; conformance
  `external/google_sheets_class_data.yaml`)
- ✅ Cloud-resource **connections** and fixture-backed `EXTERNAL_QUERY`
  (`$data_dir/external/connections/`; `docs/guides/external-query.md`;
  conformance `external/external_query_fixture.yaml`). BigLake, object tables,
  and Spanner external datasets return explicit `501 notImplemented` envelopes.
- ✅ Ephemeral `tableDefinitions` for Sheets and GCS sources

## Non-goals

- No Go port of GoogleSQL. Engine is C++. If GoogleSQL changes, we rebuild;
  we do not chase semantics in Go.
- No fallback to the real BigQuery service. Everything runs locally,
  full stop — coverage and parity are responsibilities the
  multi-strategy coordinator owns inside this process. There is no
  cloud-passthrough mode and there is no plan to add one.
- Persistence is best-effort, not promised. The DuckDB-backed
  `--data_dir` is what runs in production; we don't claim durability
  semantics, replication, or backup.
- No production performance promises. We aren't competing with the
  real BigQuery service. Expect competitive numbers for the OLAP
  shapes that route through the DuckDB fast path; expect "fast
  enough for tests, not fast enough for benchmarks" for shapes that
  route through the semantic executor; expect `UNIMPLEMENTED` for
  families not yet landed (see [Planned work](#planned-work) and
  [`docs/ENGINE_POLICY.md`](docs/ENGINE_POLICY.md)).
- No BigQuery Omni (cross-cloud query in AWS / Azure Omni regions).
  Cloud-backed **external data sources** are planned — see
  [External data sources](#external-data-sources) — and will land with
  per-source fixture or live configuration, not by proxying through the
  real BigQuery service.
- No Graph / GQL (`GRAPH_TABLE`, GQL subqueries, the `ResolvedGraph*Scan`
  family). The GQL surface is effectively a whole second query language
  (its own analyzer, data model, and pattern grammar) and is not worth
  modeling in a local emulator; it stays `unsupported` and surfaces
  `UNIMPLEMENTED`. The graph use of `ResolvedCatalogColumnRef` is out of
  scope for the same reason.

## Build systems

The C++ engine is built with Bazel. The
`//binaries/emulator_main:emulator_main` `cc_binary` links
GoogleSQL's analyzer, the local execution coordinator (DuckDB fast
path today; the semantic executor / control-op executor / DuckDB UDF
polyfills link into the same binary as they land), the DuckDB storage
backend, the catalog / query / storage-read / storage-write gRPC
handlers, and grpc++ in one binary that serves `Query.DryRun`,
`Query.ExecuteQuery`, `bigquery_emulator.v1.StorageRead`, and
`bigquery_emulator.v1.StorageWrite` end-to-end.

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

Linux/amd64 engine ships in release archives today. DuckDB prebuilts and CI
infra for `linux/arm64` are wired (non-blocking `build-engine-arm64` job,
arm64 GoogleSQL prebuilt producer); native arm64 `emulator_main` is still
proving out on GitHub arm runners — see
[`docs/dev/googlesql-prebuilt/arm64-feasibility.md`](./docs/dev/googlesql-prebuilt/arm64-feasibility.md).
Non-amd64 hosts should use the published Docker image until arm64 engine
releases land.

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
  REST-side append path; the Storage Write API
  (`google.cloud.bigquery.storage.v1.BigQueryWrite`) lands the
  `_default` + `COMMITTED` stream types on the same gRPC surface
  (`bigquery_emulator.v1.StorageWrite`) and routes appends through
  the same `DuckDBStorage::AppendRows` primitive the local DML
  executor owns. `BUFFERED` (with `FlushRows` /
  `FinalizeWriteStream`) is implemented; `PENDING` +
  `BatchCommitWriteStreams` commit atomically through the same
  `DuckDBStorage::AppendRows` primitive (see
  [`docs/ENGINE_POLICY.md`](./docs/ENGINE_POLICY.md)).
- **Dialect translation friction (GoogleSQL <-> DuckDB).** The DuckDB
  fast path is no longer the project's whole story, but the friction
  it surfaces is exactly why the local coordinator exists: shapes
  where DuckDB SQL diverges from BigQuery semantics get routed to a
  DuckDB rewrite, a DuckDB UDF, or the semantic executor instead of
  silently degrading. Known hazards we design routes around:
  - **`MERGE`.** GoogleSQL's `MERGE` permits `INSERT`, `UPDATE`, `DELETE`,
    and conditional branches on `WHEN MATCHED`, `WHEN NOT MATCHED BY
    SOURCE`, and `WHEN NOT MATCHED BY TARGET`. DuckDB's `INSERT ... ON
    CONFLICT` plus separate `UPDATE` / `DELETE` statements doesn't cover
    the full matrix, and DuckDB cannot parse BigQuery MERGE surface
    syntax (`MERGE table AS T`, `WHEN MATCHED AND …`). All MERGE shapes
    route through the semantic DML executor (`dml_merge.cc`); see
    [`docs/ENGINE_POLICY.md`](docs/ENGINE_POLICY.md).
  - **Deep STRUCT mutations.** `UPDATE t SET s.a.b = ...` is well-defined
    in BigQuery but DuckDB's struct field updates are limited.
    Anything past a single-level rewrite routes through the
    `docs/ENGINE_POLICY.md` semantic executor so deep
    nested updates don't have to round-trip through JSON to fake
    field-existence semantics.
  - **Google-specific built-ins.** `APPROX_QUANTILES`, `ML.*`,
    `BIT_COUNT`, `KEYS.NEW_KEYSET`, GIS / GEOGRAPHY functions, and
    date-arithmetic edge cases (`DATE_ADD(d, INTERVAL 1 MONTH)` semantics
    on month-end) often have no DuckDB analog. `HLL_COUNT.*` and
    `NET.*` are implemented on the semantic executor (local HLL sketch
    wire format; see ENGINE_POLICY). The function-disposition table
    records a routing disposition per entry; close-enough functions
    become `duckdb_udf`, BigQuery-exact ones become `semantic_executor`,
    and entire families (`docs/ENGINE_POLICY.md`) declare a policy of
    "local implementation now," "deterministic stub with BigQuery-shaped
    error," or "unsupported by design."
  - **NULL-equality, ordering, and float corner cases** between the two
    engines are subtly different (e.g., NaN ordering, `IS NULL` in joins,
    integer overflow behavior). Shapes that depend on these route to
    the semantic executor (`docs/ENGINE_POLICY.md`)
    rather than being approximated in DuckDB SQL.
  - **JSON.** BigQuery's `JSON` type and `JSON_VALUE` / `JSON_QUERY`
    functions are mostly portable. The common cases route
    `duckdb_native` / `duckdb_rewrite` today; the precisely-typed
    edges (`JSON` vs. `STRING`-encoded JSON, exact path-navigator
    semantics) become `duckdb_udf` or `semantic_executor` as their
    semantic gaps surface.
  - The pragmatic answer to all of the above is the route catalog,
    not "everything becomes a transpiler shape." Unmapped shapes return
    `UNIMPLEMENTED` rather than silently degrading, but the router has
    five places to put a shape before that, and the disposition
    registries (`functions.yaml` / `node_dispositions.yaml`) record
    which one each remaining shape is going to land in.
