# Method: tabledata.list

- [HTTP request](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tabledata/list#body.HTTP_TEMPLATE)
- [Path parameters](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tabledata/list#body.PATH_PARAMETERS)
- [Query parameters](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tabledata/list#body.QUERY_PARAMETERS)
- [Request body](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tabledata/list#body.request_body)
- [Response body](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tabledata/list#body.response_body)
  - [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tabledata/list#body.TableDataList.SCHEMA_REPRESENTATION)
- [Authorization scopes](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tabledata/list#body.aspect)
- [Try it!](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tabledata/list#try-it)

tabledata.list the content of a table in rows.

### HTTP request

`GET https://bigquery.googleapis.com/bigquery/v2/projects/{projectId}/datasets/{datasetId}/tables/{tableId}/data`

The URL uses [gRPC Transcoding](https://google.aip.dev/127) syntax.

### Path parameters

| Parameters ||
|---|---|
| `projectId` | `string` Required. Project id of the table to list. |
| `datasetId` | `string` Required. Dataset id of the table to list. |
| `tableId` | `string` Required. Table id of the table to list. |

### Query parameters

| Parameters ||
|---|---|
| `startIndex` | `string` Start row index of the table. |
| `maxResults` | `integer (https://developers.google.com/discovery/v1/type-format format)` Row limit of the table. |
| `pageToken` | `string` To retrieve the next page of table data, set this field to the string provided in the pageToken field of the response body from your previous call to tabledata.list. |
| `selectedFields` | `string` Subset of fields to return, supports select into sub fields. Example: selectedFields = "a,e.d.f"; |
| `formatOptions` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/DataFormatOptions`)`` Output timestamp field value in usec int64 instead of double. Output format adjustments. |

### Request body

The request body must be empty.

### Response body

The response of a tabledata.list request.

If successful, the response body contains data with the following structure:

| JSON representation |
|---|
| ``` { "kind": string, "etag": string, "totalRows": string, "pageToken": string, "rows": [ { object } ] } ``` |

| Fields ||
|---|---|
| `kind` | `string` Will be set to "bigquery#tableDataList". |
| `etag` | `string` Etag to the response. |
| `totalRows` | `string` Total rows of the entire table. In order to show default value "0", we have to present it as string. |
| `pageToken` | `string` When this field is non-empty, it indicates that additional results are available. To request the next page of data, set the pageToken field of your next tabledata.list call to the string returned in this field. |
| `rows[]` | ``object (`https://protobuf.dev/reference/protobuf/google.protobuf#struct` format)`` Repeated rows as result. The REST-based representation of this data leverages a series of JSON f,v objects for indicating fields and values. |

### Authorization scopes

Requires one of the following OAuth scopes:

- `https://www.googleapis.com/auth/bigquery`
- `https://www.googleapis.com/auth/cloud-platform`
- `https://www.googleapis.com/auth/bigquery.readonly`
- `https://www.googleapis.com/auth/cloud-platform.read-only`

For more information, see the [Authentication Overview](https://docs.cloud.google.com/docs/authentication#authorization-gcp).