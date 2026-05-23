# Method: tabledata.insertAll

- [HTTP request](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tabledata/insertAll#body.HTTP_TEMPLATE)
- [Path parameters](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tabledata/insertAll#body.PATH_PARAMETERS)
- [Request body](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tabledata/insertAll#body.request_body)
  - [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tabledata/insertAll#body.request_body.SCHEMA_REPRESENTATION)
- [Response body](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tabledata/insertAll#body.response_body)
  - [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tabledata/insertAll#body.TableDataInsertAllResponse.SCHEMA_REPRESENTATION)
- [Authorization scopes](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tabledata/insertAll#body.aspect)
- [Try it!](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tabledata/insertAll#try-it)

Streams data into BigQuery one record at a time without needing to run a load job.

### HTTP request

`POST https://bigquery.googleapis.com/bigquery/v2/projects/{projectId}/datasets/{datasetId}/tables/{tableId}/insertAll`

The URL uses [gRPC Transcoding](https://google.aip.dev/127) syntax.

### Path parameters

| Parameters ||
|---|---|
| `projectId` | `string` Required. Project ID of the destination. |
| `datasetId` | `string` Required. Dataset ID of the destination. |
| `tableId` | `string` Required. Table ID of the destination. |

### Request body

The request body contains data with the following structure:

| JSON representation |
|---|
| ``` { "kind": string, "skipInvalidRows": boolean, "ignoreUnknownValues": boolean, "templateSuffix": string, "rows": [ { "insertId": string, "json": { object } } ], "traceId": string } ``` |

| Fields ||
|---|---|
| `kind` | `string` Optional. The resource type of the response. The value is not checked at the backend. Historically, it has been set to "bigquery#tableDataInsertAllRequest" but you are not required to set it. |
| `skipInvalidRows` | `boolean` Optional. Insert all valid rows of a request, even if invalid rows exist. The default value is false, which causes the entire request to fail if any invalid rows exist. |
| `ignoreUnknownValues` | `boolean` Optional. Accept rows that contain values that do not match the schema. The unknown values are ignored. Default is false, which treats unknown values as errors. |
| `templateSuffix` | `string` Optional. If specified, treats the destination table as a base template, and inserts the rows into an instance table named "{destination}{templateSuffix}". BigQuery will manage creation of the instance table, using the schema of the base template table. See <https://cloud.google.com/bigquery/streaming-data-into-bigquery#template-tables> for considerations when working with templates tables. |
| `rows[]` | `object` |
| `rows[].insertId` | `string` Insertion ID for best-effort deduplication. This feature is not recommended, and users seeking stronger insertion semantics are encouraged to use other mechanisms such as the BigQuery Write API. |
| `rows[].json` | ``object (`https://protobuf.dev/reference/protobuf/google.protobuf#struct` format)`` Data for a single row. |
| `traceId` | `string` Optional. Unique request trace id. Used for debugging purposes only. It is case-sensitive, limited to up to 36 ASCII characters. A UUID is recommended. |

### Response body

Describes the format of a streaming insert response.

If successful, the response body contains data with the following structure:

| JSON representation |
|---|
| ``` { "kind": string, "insertErrors": [ { "index": integer, "errors": [ { object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables#ErrorProto`) } ] } ] } ``` |

| Fields ||
|---|---|
| `kind` | `string` Returns "bigquery#tableDataInsertAllResponse". |
| `insertErrors[]` | `object` Describes specific errors encountered while processing the request. |
| `insertErrors[].index` | `integer (https://developers.google.com/discovery/v1/type-format format)` The index of the row that error applies to. |
| `insertErrors[].errors[]` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables#ErrorProto`)`` Error information for the row indicated by the index property. |

### Authorization scopes

Requires one of the following OAuth scopes:

- `https://www.googleapis.com/auth/bigquery`
- `https://www.googleapis.com/auth/cloud-platform`
- `https://www.googleapis.com/auth/bigquery.insertdata`

For more information, see the [Authentication Overview](https://docs.cloud.google.com/docs/authentication#authorization-gcp).