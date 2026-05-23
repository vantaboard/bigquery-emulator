# Method: rowAccessPolicies.list

- [HTTP request](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/rowAccessPolicies/list#body.HTTP_TEMPLATE)
- [Path parameters](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/rowAccessPolicies/list#body.PATH_PARAMETERS)
- [Query parameters](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/rowAccessPolicies/list#body.QUERY_PARAMETERS)
- [Request body](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/rowAccessPolicies/list#body.request_body)
- [Response body](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/rowAccessPolicies/list#body.response_body)
  - [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/rowAccessPolicies/list#body.ListRowAccessPoliciesResponse.SCHEMA_REPRESENTATION)
- [Authorization scopes](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/rowAccessPolicies/list#body.aspect)
- [Try it!](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/rowAccessPolicies/list#try-it)

Lists all row access policies on the specified table.

### HTTP request

`GET https://bigquery.googleapis.com/bigquery/v2/projects/{projectId}/datasets/{datasetId}/tables/{tableId}/rowAccessPolicies`

The URL uses [gRPC Transcoding](https://google.aip.dev/127) syntax.

### Path parameters

| Parameters ||
|---|---|
| `projectId` | `string` Required. Project ID of the row access policies to list. |
| `datasetId` | `string` Required. Dataset ID of row access policies to list. |
| `tableId` | `string` Required. Table ID of the table to list row access policies. |

### Query parameters

| Parameters ||
|---|---|
| `pageToken` | `string` Page token, returned by a previous call, to request the next page of results. |
| `pageSize` | `integer` The maximum number of results to return in a single response page. Leverage the page tokens to iterate through the entire collection. |

### Request body

The request body must be empty.

### Response body

Response message for the rowAccessPolicies.list method.

If successful, the response body contains data with the following structure:

| JSON representation |
|---|
| ``` { "rowAccessPolicies": [ { object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/rowAccessPolicies#RowAccessPolicy`) } ], "nextPageToken": string } ``` |

| Fields ||
|---|---|
| `rowAccessPolicies[]` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/rowAccessPolicies#RowAccessPolicy`)`` Row access policies on the requested table. |
| `nextPageToken` | `string` A token to request the next page of results. |

### Authorization scopes

Requires one of the following OAuth scopes:

- `https://www.googleapis.com/auth/bigquery`
- `https://www.googleapis.com/auth/cloud-platform`
- `https://www.googleapis.com/auth/bigquery.readonly`
- `https://www.googleapis.com/auth/cloud-platform.read-only`

For more information, see the [Authentication Overview](https://docs.cloud.google.com/docs/authentication#authorization-gcp).