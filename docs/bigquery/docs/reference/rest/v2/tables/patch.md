# Method: tables.patch

- [HTTP request](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables/patch#body.HTTP_TEMPLATE)
- [Path parameters](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables/patch#body.PATH_PARAMETERS)
- [Query parameters](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables/patch#body.QUERY_PARAMETERS)
- [Request body](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables/patch#body.request_body)
- [Response body](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables/patch#body.response_body)
- [Authorization scopes](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables/patch#body.aspect)
- [Try it!](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables/patch#try-it)

Updates information in an existing table. The update method replaces the entire table resource, whereas the patch method only replaces fields that are provided in the submitted table resource. This method supports RFC5789 patch semantics.

### HTTP request

`PATCH https://bigquery.googleapis.com/bigquery/v2/projects/{projectId}/datasets/{datasetId}/tables/{tableId}`

The URL uses [gRPC Transcoding](https://google.aip.dev/127) syntax.

### Path parameters

| Parameters ||
|---|---|
| `projectId` | `string` Required. Project ID of the table to update |
| `datasetId` | `string` Required. Dataset ID of the table to update |
| `tableId` | `string` Required. Table ID of the table to update |

### Query parameters

| Parameters ||
|---|---|
| `autodetectSchema` | `boolean` Optional. When true will autodetect schema, else will keep original schema. |

### Request body

The request body contains an instance of `https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables#Table`.

### Response body

If successful, the response body contains an instance of `https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables#Table`.

### Authorization scopes

Requires one of the following OAuth scopes:

- `https://www.googleapis.com/auth/bigquery`
- `https://www.googleapis.com/auth/cloud-platform`

For more information, see the [Authentication Overview](https://docs.cloud.google.com/docs/authentication#authorization-gcp).