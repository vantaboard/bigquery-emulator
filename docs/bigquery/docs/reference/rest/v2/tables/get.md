# Method: tables.get

- [HTTP request](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables/get#body.HTTP_TEMPLATE)
- [Path parameters](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables/get#body.PATH_PARAMETERS)
- [Query parameters](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables/get#body.QUERY_PARAMETERS)
- [Request body](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables/get#body.request_body)
- [Response body](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables/get#body.response_body)
- [Authorization scopes](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables/get#body.aspect)
- [TableMetadataView](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables/get#TableMetadataView)
- [Try it!](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables/get#try-it)

Gets the specified table resource by table ID. This method does not return the data in the table, it only returns the table resource, which describes the structure of this table.

### HTTP request

`GET https://bigquery.googleapis.com/bigquery/v2/projects/{projectId}/datasets/{datasetId}/tables/{tableId}`

The URL uses [gRPC Transcoding](https://google.aip.dev/127) syntax.

### Path parameters

| Parameters ||
|---|---|
| `projectId` | `string` Required. Project ID of the requested table |
| `datasetId` | `string` Required. Dataset ID of the requested table |
| `tableId` | `string` Required. Table ID of the requested table |

### Query parameters

| Parameters ||
|---|---|
| `selectedFields` | `string` tabledata.list of table schema fields to return (comma-separated). If unspecified, all fields are returned. A fieldMask cannot be used here because the fields will automatically be converted from camelCase to snake_case and the conversion will fail if there are underscores. Since these are fields in BigQuery table schemas, underscores are allowed. |
| `view` | ``enum (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables/get#TableMetadataView`)`` Optional. Specifies the view that determines which table information is returned. By default, basic table information and storage statistics (STORAGE_STATS) are returned. |

### Request body

The request body must be empty.

### Response body

If successful, the response body contains an instance of `https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables#Table`.

### Authorization scopes

Requires one of the following OAuth scopes:

- `https://www.googleapis.com/auth/bigquery`
- `https://www.googleapis.com/auth/cloud-platform`
- `https://www.googleapis.com/auth/bigquery.readonly`
- `https://www.googleapis.com/auth/cloud-platform.read-only`

For more information, see the [Authentication Overview](https://docs.cloud.google.com/docs/authentication#authorization-gcp).

## TableMetadataView

TableMetadataView specifies which table information is returned.

| Enums ||
|---|---|
| `TABLE_METADATA_VIEW_UNSPECIFIED` | The default value. Default to the STORAGE_STATS view. |
| `BASIC` | Includes basic table information including schema and partitioning specification. This view does not include storage statistics such as numRows or numBytes. This view is significantly more efficient and should be used to support high query rates. |
| `STORAGE_STATS` | Includes all information in the BASIC view as well as storage statistics (numBytes, numLongTermBytes, numRows and lastModifiedTime). |
| `FULL` | Includes all table information, including storage statistics. It returns same information as STORAGE_STATS view, but may contain additional information in the future. |