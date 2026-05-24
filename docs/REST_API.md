# BigQuery v2 REST API surface

This is the emulator's canonical mapping from the public BigQuery v2 REST
API to the Go handler that backs each endpoint, derived from the upstream
documentation under [`docs/bigquery/docs/reference/rest/v2/`][refdir].

The goal of this document is operational: when you're staring at a client
library that's failing against the emulator, you want to know exactly
which file to open. Keep this in sync with `gateway/server.go` and
ROADMAP.md Phase 1.

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
| `jobs.insert` (metadata) | `POST /bigquery/v2/projects/{projectId}/jobs` | wired | [`gateway/handlers/jobs.go::JobInsert`][jobs] |
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
own analyzer + reference impl. The
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
`INT64` arrives as a decimal string. Phase 5 marshalers should never
emit numeric types as JSON numbers except for `FLOAT64`.

## Storage Read API

Phase 7 implements `google.cloud.bigquery.storage.v1.BigQueryRead`. The
gRPC service surface and the Avro/Arrow type tables are documented in
[`docs/bigquery/docs/reference/storage.md`][storage] and the per-method
RPC reference under [`docs/bigquery/docs/reference/storage/rpc/`][storagerpc].

[storage]: ./bigquery/docs/reference/storage.md
[storagerpc]: ./bigquery/docs/reference/storage/rpc/

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
