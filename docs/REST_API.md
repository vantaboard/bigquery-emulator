# BigQuery v2 REST API surface

This is the emulator's canonical mapping from the public BigQuery v2 REST
API to the Go handler that backs each endpoint, derived from the upstream
documentation under [`docs/bigquery/docs/reference/rest/v2/`][refdir].

The goal of this document is operational: when you're staring at a client
library that's failing against the emulator, you want to know exactly
which file to open. Keep this in sync with `gateway/server.go` and the
gateway-HTTP-surface section of `ROADMAP.md`.

[refdir]: ./bigquery/docs/reference/rest/v2/

> Status legend: `done` = end-to-end implemented · `wired` = route
> registered, returns a structurally-valid stub or 501 · `todo` = not yet
> in the route table

## Method summary

The emulator targets the BigQuery REST surface served at
`https://bigquery.googleapis.com/bigquery/v2/...`. Path templates here
omit the host and use `{x}` for path variables.

### Projects (`bigquery.projects.*`)

| Method | Path | Status | Handler |
|---|---|---|---|
| `projects.list` | `GET /bigquery/v2/projects` | wired | [`gateway/handlers/projects.go::ProjectList`][projects] |
| `projects.getServiceAccount` | `GET /bigquery/v2/projects/{projectId}/serviceAccount` | wired | [`gateway/handlers/projects.go::ProjectGetServiceAccount`][projects] |

> [!NOTE]
> There is **no** `GET /bigquery/v2/projects/{projectId}` endpoint in the
> public API; an early scaffold registered one and was removed. Use
> `projects.list` to enumerate projects and resource manager APIs for
> per-project metadata.

### Datasets (`bigquery.datasets.*`)

| Method | Path | Status | Handler |
|---|---|---|---|
| `datasets.list` | `GET /bigquery/v2/projects/{projectId}/datasets` | done | [`gateway/handlers/datasets.go::DatasetList`][datasets] |
| `datasets.insert` | `POST /bigquery/v2/projects/{projectId}/datasets` | done | [`gateway/handlers/datasets.go::DatasetInsert`][datasets] |
| `datasets.get` | `GET /bigquery/v2/projects/{projectId}/datasets/{datasetId}` | done | [`gateway/handlers/datasets.go::DatasetGet`][datasets] |
| `datasets.update` | `PUT /bigquery/v2/projects/{projectId}/datasets/{datasetId}` | done | [`gateway/handlers/datasets.go::DatasetUpdate`][datasets] |
| `datasets.patch` | `PATCH /bigquery/v2/projects/{projectId}/datasets/{datasetId}` | done | [`gateway/handlers/datasets.go::DatasetPatch`][datasets] |
| `datasets.delete` | `DELETE /bigquery/v2/projects/{projectId}/datasets/{datasetId}` | done | [`gateway/handlers/datasets.go::DatasetDelete`][datasets] |
| `datasets.undelete` | `POST /bigquery/v2/projects/{projectId}/datasets/{datasetId}:undelete` | wired | [`gateway/handlers/datasets.go::DatasetUndelete`][datasets] |

### Tables (`bigquery.tables.*`)

| Method | Path | Status | Handler |
|---|---|---|---|
| `tables.list` | `GET /bigquery/v2/projects/{projectId}/datasets/{datasetId}/tables` | done | [`gateway/handlers/tables.go::TableList`][tables] |
| `tables.insert` | `POST /bigquery/v2/projects/{projectId}/datasets/{datasetId}/tables` | done | [`gateway/handlers/tables.go::TableInsert`][tables] |
| `tables.get` | `GET /bigquery/v2/projects/{projectId}/datasets/{datasetId}/tables/{tableId}` | done | [`gateway/handlers/tables.go::TableGet`][tables] |
| `tables.update` | `PUT /bigquery/v2/projects/{projectId}/datasets/{datasetId}/tables/{tableId}` | done | [`gateway/handlers/tables.go::TableUpdate`][tables] |
| `tables.patch` | `PATCH /bigquery/v2/projects/{projectId}/datasets/{datasetId}/tables/{tableId}` | done | [`gateway/handlers/tables.go::TablePatch`][tables] |
| `tables.delete` | `DELETE /bigquery/v2/projects/{projectId}/datasets/{datasetId}/tables/{tableId}` | done | [`gateway/handlers/tables.go::TableDelete`][tables] |
| `tables.getIamPolicy` | `POST /bigquery/v2/projects/{projectId}/datasets/{datasetId}/tables/{tableId}:getIamPolicy` | wired | [`gateway/handlers/tables.go::TableGetIamPolicy`][tables] |
| `tables.setIamPolicy` | `POST /bigquery/v2/projects/{projectId}/datasets/{datasetId}/tables/{tableId}:setIamPolicy` | wired | [`gateway/handlers/tables.go::TableSetIamPolicy`][tables] |
| `tables.testIamPermissions` | `POST /bigquery/v2/projects/{projectId}/datasets/{datasetId}/tables/{tableId}:testIamPermissions` | wired | [`gateway/handlers/tables.go::TableTestIamPermissions`][tables] |

### Tabledata (`bigquery.tabledata.*`)

| Method | Path | Status | Handler |
|---|---|---|---|
| `tabledata.list` | `GET /bigquery/v2/projects/{projectId}/datasets/{datasetId}/tables/{tableId}/data` | done | [`gateway/handlers/tabledata.go::TableDataList`][tabledata] |
| `tabledata.insertAll` | `POST /bigquery/v2/projects/{projectId}/datasets/{datasetId}/tables/{tableId}/insertAll` | done | [`gateway/handlers/tabledata.go::TableDataInsertAll`][tabledata] |

### Jobs (`bigquery.jobs.*`)

| Method | Path | Status | Handler |
|---|---|---|---|
| `jobs.list` | `GET /bigquery/v2/projects/{projectId}/jobs` | wired | [`gateway/handlers/jobs.go::JobList`][jobs] |
| `jobs.insert` (metadata) | `POST /bigquery/v2/projects/{projectId}/jobs` | wired (query done; load/copy/extract partial) | [`gateway/handlers/jobs.go::JobInsert`][jobs] |
| `jobs.insert` (media upload) | `POST /upload/bigquery/v2/projects/{projectId}/jobs` | wired | [`gateway/handlers/jobs.go::JobInsertUpload`][jobs] |
| `jobs.get` | `GET /bigquery/v2/projects/{projectId}/jobs/{jobId}` | wired | [`gateway/handlers/jobs.go::JobGet`][jobs] |
| `jobs.cancel` | `POST /bigquery/v2/projects/{projectId}/jobs/{jobId}/cancel` | wired | [`gateway/handlers/jobs.go::JobCancel`][jobs] |
| `jobs.delete` | `DELETE /bigquery/v2/projects/{projectId}/jobs/{jobId}/delete` | wired | [`gateway/handlers/jobs.go::JobDelete`][jobs] |

The literal `/delete` segment after `{jobId}` is not a typo — that is
the upstream URL template, see
[`docs/bigquery/docs/reference/rest/v2/jobs/delete.md`][delete-md].

[delete-md]: ./bigquery/docs/reference/rest/v2/jobs/delete.md

### Queries (synchronous query API)

| Method | Path | Status | Handler |
|---|---|---|---|
| `jobs.query` | `POST /bigquery/v2/projects/{projectId}/queries` | wired | [`gateway/handlers/queries.go::QueryRun`][queries] |
| `jobs.getQueryResults` | `GET /bigquery/v2/projects/{projectId}/queries/{jobId}` | wired | [`gateway/handlers/queries.go::QueryGetResults`][queries] |

### Models (`bigquery.models.*`)

BQML has no engine backing yet (the emulator has no trained-model
store). List returns an empty BigQuery-shaped page so client probes
succeed; specific-resource methods return 404 (absent) or 501
(mutating) so list-get-delete sample loops behave predictably.

| Method | Path | Status | Handler |
|---|---|---|---|
| `models.list` | `GET /bigquery/v2/projects/{projectId}/datasets/{datasetId}/models` | wired | [`gateway/handlers/models.go::ModelList`][models] |
| `models.get` | `GET /bigquery/v2/projects/{projectId}/datasets/{datasetId}/models/{modelId}` | wired | [`gateway/handlers/models.go::ModelGet`][models] |
| `models.patch` | `PATCH /bigquery/v2/projects/{projectId}/datasets/{datasetId}/models/{modelId}` | wired | [`gateway/handlers/models.go::ModelPatch`][models] |
| `models.delete` | `DELETE /bigquery/v2/projects/{projectId}/datasets/{datasetId}/models/{modelId}` | wired | [`gateway/handlers/models.go::ModelDelete`][models] |

### Routines (`bigquery.routines.*`)

Routines (UDFs / TVFs / stored procedures) are not yet registered in
the engine catalog. Same wired-stub posture as `models.*`.

| Method | Path | Status | Handler |
|---|---|---|---|
| `routines.list` | `GET /bigquery/v2/projects/{projectId}/datasets/{datasetId}/routines` | wired | [`gateway/handlers/routines.go::RoutineList`][routines] |
| `routines.insert` | `POST /bigquery/v2/projects/{projectId}/datasets/{datasetId}/routines` | wired | [`gateway/handlers/routines.go::RoutineInsert`][routines] |
| `routines.get` | `GET /bigquery/v2/projects/{projectId}/datasets/{datasetId}/routines/{routineId}` | wired | [`gateway/handlers/routines.go::RoutineGet`][routines] |
| `routines.update` | `PUT /bigquery/v2/projects/{projectId}/datasets/{datasetId}/routines/{routineId}` | wired | [`gateway/handlers/routines.go::RoutineUpdate`][routines] |
| `routines.delete` | `DELETE /bigquery/v2/projects/{projectId}/datasets/{datasetId}/routines/{routineId}` | wired | [`gateway/handlers/routines.go::RoutineDelete`][routines] |

### Row-access policies (`bigquery.rowAccessPolicies.*`)

Row-level access policies have no engine backing yet (they would need a
policy store + per-query analysis rewrite). Only `list` is wired so
client libraries that probe at startup get an empty page; the AIP-136
IAM custom methods return 501.

| Method | Path | Status | Handler |
|---|---|---|---|
| `rowAccessPolicies.list` | `GET /bigquery/v2/projects/{projectId}/datasets/{datasetId}/tables/{tableId}/rowAccessPolicies` | wired | [`gateway/handlers/row_access_policies.go::RowAccessPolicyList`][rowaccess] |

### Migration (`bigquerymigration.v2alpha`)

Served from the **same** HTTP listener as the BigQuery v2 surface. The
official client libraries
(`cloud.google.com/go/bigquery/migration/apiv2alpha`,
`google-cloud-bigquery-migration` for Python/Node/Java) read
`BIGQUERY_MIGRATION_EMULATOR_HOST` and fall back to
`BIGQUERY_EMULATOR_HOST`. Routes are registered under both `v2alpha`
and `v2` (alias parity with go-googlesql). Engine has no workflow
state, AST translator, or LRO store yet — list returns the documented
empty page, get/delete return 404, create / `:start` return 501.

| Method | Path | Status | Handler |
|---|---|---|---|
| `migration.workflows.list` | `GET /v2alpha/projects/{projectId}/locations/{location}/workflows` (also `v2`) | wired | [`gateway/handlers/migration.go::MigrationWorkflowList`][migration] |
| `migration.workflows.create` | `POST /v2alpha/projects/{projectId}/locations/{location}/workflows` (also `v2`) | wired | [`gateway/handlers/migration.go::MigrationWorkflowCreate`][migration] |
| `migration.workflows.get` | `GET /v2alpha/projects/{projectId}/locations/{location}/workflows/{workflowId}` (also `v2`) | wired | [`gateway/handlers/migration.go::MigrationWorkflowGet`][migration] |
| `migration.workflows.delete` | `DELETE /v2alpha/projects/{projectId}/locations/{location}/workflows/{workflowId}` (also `v2`) | wired | [`gateway/handlers/migration.go::MigrationWorkflowDelete`][migration] |
| `migration.workflows.start` | `POST /v2alpha/projects/{projectId}/locations/{location}/workflows/{workflowId}:start` (also `v2`) | wired | [`gateway/handlers/migration.go::MigrationWorkflowCustomMethodPOST`][migration] |

### Data Transfer Service (`bigquerydatatransfer.v1`)

Served from the same listener via `BIGQUERY_EMULATOR_HOST`. No data
source catalog or transfer config store exists yet, so the standard
list endpoints return the documented empty page, specific-resource
gets return 404, and `transferConfigs.create` returns 501. Both
project-scoped and location-scoped variants are wired (client
libraries pick whichever the user's API region demands).

| Method | Path | Status | Handler |
|---|---|---|---|
| `dataSources.list` | `GET /v1/projects/{projectId}/dataSources` | wired | [`gateway/handlers/data_transfer.go::DataTransferDataSourceList`][datatransfer] |
| `dataSources.list` (regional) | `GET /v1/projects/{projectId}/locations/{location}/dataSources` | wired | [`gateway/handlers/data_transfer.go::DataTransferDataSourceList`][datatransfer] |
| `dataSources.get` | `GET /v1/projects/{projectId}/dataSources/{dataSourceId}` | wired | [`gateway/handlers/data_transfer.go::DataTransferDataSourceGet`][datatransfer] |
| `dataSources.get` (regional) | `GET /v1/projects/{projectId}/locations/{location}/dataSources/{dataSourceId}` | wired | [`gateway/handlers/data_transfer.go::DataTransferDataSourceGet`][datatransfer] |
| `transferConfigs.list` | `GET /v1/projects/{projectId}/transferConfigs` | wired | [`gateway/handlers/data_transfer.go::DataTransferConfigList`][datatransfer] |
| `transferConfigs.list` (regional) | `GET /v1/projects/{projectId}/locations/{location}/transferConfigs` | wired | [`gateway/handlers/data_transfer.go::DataTransferConfigList`][datatransfer] |
| `transferConfigs.get` | `GET /v1/projects/{projectId}/transferConfigs/{configId}` | wired | [`gateway/handlers/data_transfer.go::DataTransferConfigGet`][datatransfer] |
| `transferConfigs.get` (regional) | `GET /v1/projects/{projectId}/locations/{location}/transferConfigs/{configId}` | wired | [`gateway/handlers/data_transfer.go::DataTransferConfigGet`][datatransfer] |
| `transferConfigs.create` | `POST /v1/projects/{projectId}/transferConfigs` | wired | [`gateway/handlers/data_transfer.go::DataTransferConfigCreate`][datatransfer] |
| `transferConfigs.create` (regional) | `POST /v1/projects/{projectId}/locations/{location}/transferConfigs` | wired | [`gateway/handlers/data_transfer.go::DataTransferConfigCreate`][datatransfer] |

### Discovery and health

| Method | Path | Status | Handler |
|---|---|---|---|
| Discovery doc | `GET /discovery/v1/apis/bigquery/v2/rest` | wired | [`gateway/handlers/discovery.go::Discovery`][discovery] |
| Health (emulator-only) | `GET /` and `GET /healthz` | done | [`gateway/handlers/handlers.go::Health`][handlers] |

[projects]: ../gateway/handlers/projects.go
[datasets]: ../gateway/handlers/datasets.go
[tables]: ../gateway/handlers/tables.go
[tabledata]: ../gateway/handlers/tabledata.go
[jobs]: ../gateway/handlers/jobs.go
[queries]: ../gateway/handlers/queries.go
[models]: ../gateway/handlers/models.go
[routines]: ../gateway/handlers/routines.go
[rowaccess]: ../gateway/handlers/row_access_policies.go
[migration]: ../gateway/handlers/migration.go
[datatransfer]: ../gateway/handlers/data_transfer.go
[handlers]: ../gateway/handlers/handlers.go
[discovery]: ../gateway/handlers/discovery.go

## Routing notes (Go specifics)

Go's `net/http` ServeMux requires every wildcard path segment to end in
`}`. Several BigQuery endpoints use the [AIP-136][aip136] custom-method
shape with a `:operation` suffix on the resource, e.g.
`/datasets/{datasetId}:undelete`. The mux can't match `:undelete` as a
literal after a wildcard, so we register the parent route
(`POST /bigquery/v2/projects/{projectId}/datasets/{datasetId}`) and use
a tiny handler-level dispatcher in
[`gateway/handlers/handlers.go::dispatchColonOp`][handlers] that reads
the trailing `:op` from the captured wildcard. The same trick is used
for the three `tables.*IamPolicy*`/`tables.testIamPermissions`
endpoints.

[aip136]: https://google.aip.dev/136

## Error envelope

All non-2xx responses use BigQuery's documented JSON shape (see
[`docs/bigquery/docs/error-messages.md`][errors]):

```json
{
  "error": {
    "code": 404,
    "message": "Not Found: Dataset myproject:foo",
    "status": "notFound",
    "errors": [
      {
        "domain": "global",
        "reason": "notFound",
        "message": "Not Found: Dataset myproject:foo"
      }
    ]
  }
}
```

`reason` values used by the emulator and recognized by BigQuery clients
include `notFound`, `notImplemented`, `invalid`, `invalidQuery`,
`duplicate`, `quotaExceeded`, `accessDenied`, `stopped`. These are a
subset of the table in the upstream
[error messages doc][errors].

[errors]: ./bigquery/docs/error-messages.md

## SQL dialect

BigQuery's wire field `useLegacySql` defaults to `true` (legacy SQL).
The emulator only supports GoogleSQL because the engine is GoogleSQL's
own analyzer feeding the local execution coordinator. The
[`jobs.query`](../gateway/handlers/queries.go) handler:

- Treats `useLegacySql` unset or `false` as GoogleSQL (the supported
  case).
- Rejects `useLegacySql=true` with HTTP 400 and `reason: invalidQuery`
  before any engine work, returning the standard
  [error envelope](#error-envelope).

Document this clearly to clients that default to legacy via older Go
client library versions: pass `option.WithEndpoint(...)` together with
explicitly setting `Query.UseLegacySQL = false`.

## Type wire encoding

For result marshaling, types follow
[`StandardSqlDataType.TypeKind`][sqltype]:

[sqltype]: ./bigquery/docs/reference/rest/v2/StandardSqlDataType.md

| TypeKind | Wire encoding |
|---|---|
| `INT64` | decimal string |
| `BOOL` | JSON boolean |
| `FLOAT64` | JSON number, or string `"NaN"`/`"Infinity"`/`"-Infinity"` |
| `STRING` | JSON string |
| `BYTES` | base64 string (RFC 4648 §4) |
| `TIMESTAMP` | RFC 3339 with mandatory `Z` (e.g. `1985-04-12T23:20:50.52Z`) |
| `DATE` | RFC 3339 full-date (`1985-04-12`) |
| `TIME` | RFC 3339 partial-time (`23:20:50.52`) |
| `DATETIME` | RFC 3339 full-date `T` partial-time (`1985-04-12T23:20:50.52`) |
| `GEOGRAPHY` | WKT |
| `NUMERIC` / `BIGNUMERIC` | decimal string |
| `JSON` | string-encoded JSON |
| `ARRAY` | list with element type per `arrayElementType` |
| `STRUCT` | list (positional) with element types per `structType.fields[i]` |
| `RANGE<T>` | pair `[start, end)` formatted with the inner type |

`f`/`v` rows are always strings/objects/arrays at the wire level — even
`INT64` arrives as a decimal string. The query-execution row marshalers
should never emit numeric types as JSON numbers except for `FLOAT64`.

## Storage Read API

The Storage Read API surface implements
`google.cloud.bigquery.storage.v1.BigQueryRead`. The
gRPC service surface and the Avro/Arrow type tables are documented in
[`docs/bigquery/docs/reference/storage.md`][storage] and the per-method
RPC reference under [`docs/bigquery/docs/reference/storage/rpc/`][storagerpc].

[storage]: ./bigquery/docs/reference/storage.md
[storagerpc]: ./bigquery/docs/reference/storage/rpc/

### Transport: gRPC-only, served by the engine

The public BigQuery Storage Read API is **gRPC-only** in production —
the REST surface BigQuery exposes does not proxy it, and the
`google-cloud-bigquery-storage` client libraries (Go's
`cloud.google.com/go/bigquery/storage`, Python's
`google-cloud-bigquery-storage`, Java's
`google-cloud-bigquerystorage`) all open a separate gRPC channel to
`bigquerystorage.googleapis.com:443`. The emulator follows the same
shape:

* The **REST gateway** (`gateway_main`, default `:9050`) does **not**
  expose any `bigquery_emulator.v1.StorageRead` surface. Plan 39
  intentionally keeps the gateway focused on the REST-only halves of
  the public API (`projects`, `datasets`, `tables`, `jobs`,
  `tabledata.list`, `jobs.query`, `tabledata.insertAll`).
* The **C++ engine** (`emulator_main`, default `:9060` via
  `--grpc_port` on `gateway_main` / `--host_port` on `emulator_main`)
  serves `bigquery_emulator.v1.StorageRead` directly on its gRPC port.
  `task emulator:run-full` exposes both ports, so point a Storage Read
  client at the engine port (`localhost:9060`) rather than the gateway.

For programmatic tests, the
`gateway/e2e/storage_read_test.go` harness gates a BigQuery Storage
client off the gateway's `engine.Client` channel (the same connection
the gateway uses internally for `Catalog` and `Query`). See
`gateway/e2e/catalog_test.go::startEmulatorWithFlags` for the full
plumbing.

### Supported `ReadOptions`

* `row_restriction`: a single `<column> = <literal>` equality clause.
  Literals support INT64 (`id = 42`), BOOL (`active = true`,
  case-insensitive), and STRING (`name = 'ada'`, with the SQL `''`
  escape for embedded apostrophes). Backtick-quoted column names
  (`` `id` = 42``) round-trip; bare identifiers are limited to
  `[A-Za-z_][A-Za-z0-9_]*`. Anything more complex (range / inequality
  ops, connectives, IN, NULL, ARRAY / STRUCT columns,
  FLOAT64 / DATE / NUMERIC literals) is rejected at
  `CreateReadSession` time with `INVALID_ARGUMENT` — the gateway
  surfaces that as the public Storage Read 400 envelope.

  Pushdown shape:
  * **Memory backend**: the predicate filters the row vector in C++
    before `offset` / `row_limit` slicing.
  * **DuckDB backend**: the predicate becomes a `WHERE` clause on the
    `read_parquet(...)` scan, so DuckDB filters before materializing
    rows.

* `selected_fields`: accepted and echoed on the `ReadSession` reply,
  but **not enforced** — every column is returned regardless. Pushing
  projection into the storage layer is deferred to a future plan.

## Authentication posture

The emulator follows `cloud-spanner-emulator`'s posture: it parses but
ignores bearer tokens, and the `BIGQUERY_EMULATOR_HOST` environment
variable is the canonical client-library override (mirroring
`STORAGE_EMULATOR_HOST` and `SPANNER_EMULATOR_HOST`). Concretely, code
that targets BigQuery normally:

```go
client, err := bigquery.NewClient(ctx, "test-project")
```

is redirected at the emulator with either:

```go
client, err := bigquery.NewClient(ctx, "test-project",
    option.WithEndpoint("http://localhost:9050"),
    option.WithoutAuthentication(),
)
```

or by setting `BIGQUERY_EMULATOR_HOST=localhost:9050` in the
environment. The README's
[Quickstart](../README.md#pointing-client-libraries-at-the-emulator)
documents both forms for end users; this file documents the server-side
posture.

Every request passes through
[`gateway/middleware/auth.go::WithAuth`][authmw], which:

- Parses the `Authorization` header when present (RFC 6750 `Bearer`
  tokens have the scheme stripped; other schemes are stored verbatim).
- Attaches a synthetic [`Principal`][authmw] to the request context
  with `Email = "emulator@bigquery.local"`, regardless of what the
  client sent.
- Never short-circuits the response — well-formed, malformed, and
  absent `Authorization` headers are all served identically. The
  emulator never returns 401.

Handlers that need to know whether the client tried to authenticate
read the principal via `middleware.PrincipalFromContext` and inspect
the `Anonymous` and `Bearer` fields.

The full upstream auth model (ADC, service-account keys, IAM scopes) is
documented under
[`docs/bigquery/docs/authentication.md`][auth] and is intentionally
**not** modeled by the emulator.

[authmw]: ../gateway/middleware/auth.go

[auth]: ./bigquery/docs/authentication.md
