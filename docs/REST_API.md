# BigQuery v2 REST API surface

This is the emulator's canonical mapping from the public BigQuery v2 REST
API to the Go handler that backs each endpoint, derived from the upstream
documentation under [BigQuery REST v2 reference](https://cloud.google.com/bigquery/docs/reference/rest/v2).

The goal of this document is operational: when you're staring at a client
library that's failing against the emulator, you want to know exactly
which file to open. Keep this in sync with `gateway/server.go` and the
gateway-HTTP-surface section of `ROADMAP.md`.

[refdir]: https://cloud.google.com/bigquery/docs/reference/rest/v2/

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
| `tables.delete` | `DELETE /bigquery/v2/projects/{projectId}/datasets/{datasetId}/tables/{tableId}` | done (captures snapshot for undelete) | [`gateway/handlers/tables.go::TableDelete`][tables] |
| `tables.getIamPolicy` | `POST /bigquery/v2/projects/{projectId}/datasets/{datasetId}/tables/{tableId}:getIamPolicy` | wired | [`gateway/handlers/tables.go::TableGetIamPolicy`][tables] |
| `tables.setIamPolicy` | `POST /bigquery/v2/projects/{projectId}/datasets/{datasetId}/tables/{tableId}:setIamPolicy` | wired | [`gateway/handlers/tables.go::TableSetIamPolicy`][tables] |
| `tables.testIamPermissions` | `POST /bigquery/v2/projects/{projectId}/datasets/{datasetId}/tables/{tableId}:testIamPermissions` | wired | [`gateway/handlers/tables.go::TableTestIamPermissions`][tables] |

**External tables:** `tables.insert` accepts
`externalDataConfiguration` (`sourceUris`, `sourceFormat`, `schema`,
`csvOptions`, …). Supported GCS formats (`CSV`, `NEWLINE_DELIMITED_JSON`,
`PARQUET`, …) are fetched via fake-gcs (`STORAGE_EMULATOR_HOST` /
`FAKE_GCS_PORT`, same as LOAD jobs) and materialized into the engine
catalog at insert time; `externalDataConfiguration` round-trips through
the gateway [`MetadataStore`][metadata-store] on GET/PATCH/UPDATE.
`type` defaults to `EXTERNAL`. Google Sheets (`GOOGLE_SHEETS` /
`googleSheetsOptions`) returns **501** with a clear message.

**Logical views:** `CREATE [OR REPLACE] VIEW` via `jobs.query` registers
the view in the engine's in-memory catalog and persists the DDL in
`DuckDBStorage` (`__bqemu_views`). `tables.list` / `tables.get`
return `type=VIEW` and `view.query` (with `view.useLegacySql=false` for
GoogleSQL views). After an engine restart the gateway rehydrates views
from storage so `tables.get` still resolves through
`Catalog.DescribeTable` even though the gateway's in-memory metadata
overlay is empty. `CREATE MATERIALIZED VIEW` DDL is also surfaced on
`tables.list` / `tables.get` as `type=MATERIALIZED_VIEW` with
`materializedView.query` (the engine materializes rows into a physical
table).

Query-time ephemeral external tables use `tableDefinitions` on
`jobs.query` and `configuration.query` (jobs.insert). When the query
omits `defaultDataset`, definitions are registered under internal dataset
`_bq_external_temp` and that dataset is forwarded as
`default_dataset_id` so unqualified table ids in SQL resolve.

[metadata-store]: https://github.com/vantaboard/bigquery-emulator/blob/main/gateway/handlers/metadata_store.go

### Tabledata (`bigquery.tabledata.*`)

| Method | Path | Status | Handler |
|---|---|---|---|
| `tabledata.list` | `GET /bigquery/v2/projects/{projectId}/datasets/{datasetId}/tables/{tableId}/data` | done | [`gateway/handlers/tabledata.go::TableDataList`][tabledata] |
| `tabledata.insertAll` | `POST /bigquery/v2/projects/{projectId}/datasets/{datasetId}/tables/{tableId}/insertAll` | done | [`gateway/handlers/tabledata.go::TableDataInsertAll`][tabledata] |

**`tabledata.list` notes:** `maxResults` defaults to **10000** (cap **100000**).
`maxResults=0` returns `totalRows`/`etag` with zero rows and no `pageToken`
(same semantics as `jobs.getQueryResults`). `pageToken` is a decimal row index.
`selectedFields` projects top-level columns (comma-separated; dotted paths
select the top-level STRUCT). `formatOptions.useInt64Timestamp=true` emits
TIMESTAMP cells as JSON int64 microseconds. Logical **views** have no Parquet
backing — use `jobs.query` for preview; native tables paginate via
`pageToken`.

### Jobs (`bigquery.jobs.*`)

| Method | Path | Status | Handler |
|---|---|---|---|
| `jobs.list` | `GET /bigquery/v2/projects/{projectId}/jobs` | done | [`gateway/handlers/jobs.go::JobList`][jobs] |
| `jobs.insert` (metadata) | `POST /bigquery/v2/projects/{projectId}/jobs` | done (query, LOAD, COPY, EXTRACT) | [`gateway/handlers/jobs.go::JobInsert`][jobs] |
| `jobs.insert` (media upload) | `POST /upload/bigquery/v2/projects/{projectId}/jobs` | done (multipart + resumable LOAD) | [`gateway/handlers/jobs.go::JobInsertUpload`][jobs] |
| `jobs.insert` (resumable chunk) | `PUT /upload/bigquery/v2/projects/{projectId}/jobs` | done (resumable LOAD upload) | [`gateway/handlers/jobs.go::JobInsertUpload`][jobs] |
| `jobs.get` | `GET /bigquery/v2/projects/{projectId}/jobs/{jobId}` | done | [`gateway/handlers/jobs.go::JobGet`][jobs] |
| `jobs.cancel` | `POST /bigquery/v2/projects/{projectId}/jobs/{jobId}/cancel` | done | [`gateway/handlers/jobs.go::JobCancel`][jobs] |
| `jobs.delete` | `DELETE /bigquery/v2/projects/{projectId}/jobs/{jobId}/delete` | done | [`gateway/handlers/jobs.go::JobDelete`][jobs] |

The literal `/delete` segment after `{jobId}` is not a typo — that is
the upstream URL template, see
[`jobs.delete` reference](https://cloud.google.com/bigquery/docs/reference/rest/v2/jobs/delete).

**`jobs.list` filters:** `stateFilter` (repeatable `pending`/`running`/`done`),
`minCreationTime` / `maxCreationTime` (epoch ms), `parentJobId`,
`maxResults` (default 50), and opaque `pageToken` pagination are honored
against the gateway job registry. `allUsers=true` returns 501 (no auth
context).

**`INFORMATION_SCHEMA.JOBS*`:** queries against `` `region-*`.INFORMATION_SCHEMA.JOBS(_BY_PROJECT) ``
are rewritten to `` `{project}`.`_bqemu_jobs`.`JOBS` `` with rows
materialized from the job registry before the engine executes the SQL
([`gateway/query/info_schema_jobs.go`](https://github.com/vantaboard/bigquery-emulator/blob/main/gateway/query/info_schema_jobs.go)).

**COPY / EXTRACT / undelete:** `configuration.copy` copies rows
from `sourceTable` / `sourceTables` into `destinationTable`, honoring
`writeDisposition` (`WRITE_EMPTY`, `WRITE_TRUNCATE`, `WRITE_APPEND`).
Live sources prefer engine SQL (`CREATE TABLE AS SELECT` / `UNION ALL`);
snapshot sources (`tableId@epoch`) and SQL fallbacks use catalog row copy.
`configuration.extract` serializes a source table to `destinationUris`
(`CSV`, `NEWLINE_DELIMITED_JSON`, optional `GZIP`) via fake-gcs HTTP upload.
Table undelete (python `test_undelete_table`, node `undeleteTable`) is a
COPY job from a snapshot decorator after `tables.delete`; there is no
separate `tables.undelete` RPC.

[delete-md]: https://cloud.google.com/bigquery/docs/reference/rest/v2/jobs/delete

### Queries (synchronous query API)

| Method | Path | Status | Handler |
|---|---|---|---|
| `jobs.query` | `POST /bigquery/v2/projects/{projectId}/queries` | done (incl. `tableDefinitions`) | [`gateway/handlers/queries.go::QueryRun`][queries] |
| `jobs.getQueryResults` | `GET /bigquery/v2/projects/{projectId}/queries/{jobId}` | wired | [`gateway/handlers/queries.go::QueryGetResults`][queries] |

### Models (`bigquery.models.*`)

BQML has no trained-model store (inference stays `UNIMPLEMENTED`), but
`CREATE MODEL` DDL registers metadata-only model resources the REST
surface can list/get/delete for client-library round-trips.

| Method | Path | Status | Handler |
|---|---|---|---|
| `models.list` | `GET /bigquery/v2/projects/{projectId}/datasets/{datasetId}/models` | done | [`gateway/handlers/models.go::ModelList`][models] |
| `models.get` | `GET /bigquery/v2/projects/{projectId}/datasets/{datasetId}/models/{modelId}` | done | [`gateway/handlers/models.go::ModelGet`][models] |
| `models.patch` | `PATCH /bigquery/v2/projects/{projectId}/datasets/{datasetId}/models/{modelId}` | wired | [`gateway/handlers/models.go::ModelPatch`][models] |
| `models.delete` | `DELETE /bigquery/v2/projects/{projectId}/datasets/{datasetId}/models/{modelId}` | done | [`gateway/handlers/models.go::ModelDelete`][models] |

### Routines (`bigquery.routines.*`)

Routines (UDFs / TVFs / stored procedures) persist in the engine's
`DuckDBStorage` catalog (`__bqemu_routines` in `catalog.duckdb`) and
surface through the `Catalog.ListRoutines` / `GetRoutine` /
`UpsertRoutine` / `DeleteRoutine` gRPC RPCs. REST
insert/get/list/update/delete delegates to those RPCs when the gateway
is wired to `emulator_main`; the in-memory [`gateway/routines/`][routines-pkg]
store mirrors responses for the synchronous query path.
`CREATE FUNCTION` / `CREATE PROCEDURE` DDL via `jobs.query` also
registers routines and surfaces `ddlTargetRoutine` on the job
statistics envelope.

| Method | Path | Status | Handler |
|---|---|---|---|
| `routines.list` | `GET /bigquery/v2/projects/{projectId}/datasets/{datasetId}/routines` | done | [`gateway/handlers/routines.go::RoutineList`][routines] |
| `routines.insert` | `POST /bigquery/v2/projects/{projectId}/datasets/{datasetId}/routines` | done | [`gateway/handlers/routines.go::RoutineInsert`][routines] |
| `routines.get` | `GET /bigquery/v2/projects/{projectId}/datasets/{datasetId}/routines/{routineId}` | done | [`gateway/handlers/routines.go::RoutineGet`][routines] |
| `routines.update` | `PUT /bigquery/v2/projects/{projectId}/datasets/{datasetId}/routines/{routineId}` | done | [`gateway/handlers/routines.go::RoutineUpdate`][routines] |
| `routines.delete` | `DELETE /bigquery/v2/projects/{projectId}/datasets/{datasetId}/routines/{routineId}` | done | [`gateway/handlers/routines.go::RoutineDelete`][routines] |

[routines-pkg]: https://github.com/vantaboard/bigquery-emulator/tree/main/gateway/routines/

### Row-access policies (`bigquery.rowAccessPolicies.*`)

Row-level access policies persist in the engine catalog
(`__bqemu_row_access_policies`) and round-trip through REST insert/get/list/update/delete.

| Method | Path | Status | Handler |
|---|---|---|---|
| `rowAccessPolicies.list` | `GET /bigquery/v2/projects/{projectId}/datasets/{datasetId}/tables/{tableId}/rowAccessPolicies` | done | [`gateway/handlers/row_access_policies.go::RowAccessPolicyList`][rowaccess] |
| `rowAccessPolicies.insert` | `POST /bigquery/v2/projects/{projectId}/datasets/{datasetId}/tables/{tableId}/rowAccessPolicies` | done | [`gateway/handlers/row_access_policies.go::RowAccessPolicyInsert`][rowaccess] |
| `rowAccessPolicies.get` | `GET /bigquery/v2/projects/{projectId}/datasets/{datasetId}/tables/{tableId}/rowAccessPolicies/{policyId}` | done | [`gateway/handlers/row_access_policies.go::RowAccessPolicyGet`][rowaccess] |
| `rowAccessPolicies.update` | `PUT /bigquery/v2/projects/{projectId}/datasets/{datasetId}/tables/{tableId}/rowAccessPolicies/{policyId}` | done | [`gateway/handlers/row_access_policies.go::RowAccessPolicyUpdate`][rowaccess] |
| `rowAccessPolicies.delete` | `DELETE /bigquery/v2/projects/{projectId}/datasets/{datasetId}/tables/{tableId}/rowAccessPolicies/{policyId}` | done | [`gateway/handlers/row_access_policies.go::RowAccessPolicyDelete`][rowaccess] |

### Migration (`bigquerymigration.v2alpha`)

Served from the **same** HTTP listener as the BigQuery v2 surface. The
official client libraries
(`cloud.google.com/go/bigquery/migration/apiv2alpha`,
`google-cloud-bigquery-migration` for Python/Node/Java) read
`BIGQUERY_MIGRATION_EMULATOR_HOST` and fall back to
`BIGQUERY_EMULATOR_HOST`. Routes are registered under both `v2alpha`
and `v2` (alias parity for client compatibility). Workflow metadata is
held in an in-process store (no AST translator or LRO execution).

| Method | Path | Status | Handler |
|---|---|---|---|
| `migration.workflows.list` | `GET /v2alpha/projects/{projectId}/locations/{location}/workflows` (also `v2`) | done | [`gateway/handlers/migration.go::MigrationWorkflowList`][migration] |
| `migration.workflows.create` | `POST /v2alpha/projects/{projectId}/locations/{location}/workflows` (also `v2`) | done | [`gateway/handlers/migration.go::MigrationWorkflowCreate`][migration] |
| `migration.workflows.get` | `GET /v2alpha/projects/{projectId}/locations/{location}/workflows/{workflowId}` (also `v2`) | done | [`gateway/handlers/migration.go::MigrationWorkflowGet`][migration] |
| `migration.workflows.delete` | `DELETE /v2alpha/projects/{projectId}/locations/{location}/workflows/{workflowId}` (also `v2`) | done | [`gateway/handlers/migration.go::MigrationWorkflowDelete`][migration] |
| `migration.workflows.start` | `POST /v2alpha/projects/{projectId}/locations/{location}/workflows/{workflowId}:start` (also `v2`) | done | [`gateway/handlers/migration.go::MigrationWorkflowCustomMethodPOST`][migration] |

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

[projects]: https://github.com/vantaboard/bigquery-emulator/blob/main/gateway/handlers/projects.go
[datasets]: https://github.com/vantaboard/bigquery-emulator/blob/main/gateway/handlers/datasets.go
[tables]: https://github.com/vantaboard/bigquery-emulator/blob/main/gateway/handlers/tables.go
[tabledata]: https://github.com/vantaboard/bigquery-emulator/blob/main/gateway/handlers/tabledata.go
[jobs]: https://github.com/vantaboard/bigquery-emulator/blob/main/gateway/handlers/jobs.go
[queries]: https://github.com/vantaboard/bigquery-emulator/blob/main/gateway/handlers/queries.go
[models]: https://github.com/vantaboard/bigquery-emulator/blob/main/gateway/handlers/models.go
[routines]: https://github.com/vantaboard/bigquery-emulator/blob/main/gateway/handlers/routines.go
[rowaccess]: https://github.com/vantaboard/bigquery-emulator/blob/main/gateway/handlers/row_access_policies.go
[migration]: https://github.com/vantaboard/bigquery-emulator/blob/main/gateway/handlers/migration.go
[datatransfer]: https://github.com/vantaboard/bigquery-emulator/blob/main/gateway/handlers/data_transfer.go
[handlers]: https://github.com/vantaboard/bigquery-emulator/blob/main/gateway/handlers/handlers.go
[discovery]: https://github.com/vantaboard/bigquery-emulator/blob/main/gateway/handlers/discovery.go

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
[error messages doc][errors]):

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

[errors]: https://cloud.google.com/bigquery/docs/error-messages

## SQL dialect

BigQuery's wire field `useLegacySql` defaults to `true` (legacy SQL).
The emulator executes GoogleSQL via the engine (GoogleSQL's analyzer
feeding the local execution coordinator). When `useLegacySql=true`,
the gateway transpiles a **narrow** subset of legacy SQL used by
thirdparty samples—bracket table references such as
`[project:dataset.table]` and `[dataset.table]`—into GoogleSQL
backtick form before forwarding; `use_legacy_sql` is always cleared on
the engine RPC. Full legacy SQL dialect (functions, `#legacy`, JOIN
variants, etc.) is not supported; unsupported constructs return HTTP
400 with `reason: invalidQuery`.

- Treats `useLegacySql` unset or `false` as GoogleSQL (the common
  case).
- Translates bracket table refs when `useLegacySql=true`, then runs
  GoogleSQL on the engine.

Clients that default to legacy via older library versions may still set
`useLegacySql=true` for bracket-style samples; for new queries prefer
`useLegacySql=false` (GoogleSQL).

## Type wire encoding

For result marshaling, types follow
[`StandardSqlDataType.TypeKind`][sqltype]:

[sqltype]: https://cloud.google.com/bigquery/docs/reference/rest/v2/StandardSqlDataType

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
[Storage Read API][storage] and the per-method
RPC reference under [Storage Read RPC][storagerpc].

[storage]: https://cloud.google.com/bigquery/docs/reference/storage
[storagerpc]: https://cloud.google.com/bigquery/docs/reference/storage/rpc

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
[Client libraries](./CLIENTS.md)
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
[BigQuery authentication][auth] and is intentionally
**not** modeled by the emulator.

[authmw]: https://github.com/vantaboard/bigquery-emulator/blob/main/gateway/middleware/auth.go

[auth]: https://cloud.google.com/bigquery/docs/authentication
