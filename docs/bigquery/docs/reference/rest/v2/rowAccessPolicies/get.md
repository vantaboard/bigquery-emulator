# Method: rowAccessPolicies.get

- [HTTP request](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/rowAccessPolicies/get#body.HTTP_TEMPLATE)
- [Path parameters](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/rowAccessPolicies/get#body.PATH_PARAMETERS)
- [Request body](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/rowAccessPolicies/get#body.request_body)
- [Response body](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/rowAccessPolicies/get#body.response_body)
- [Authorization scopes](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/rowAccessPolicies/get#body.aspect)
- [Try it!](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/rowAccessPolicies/get#try-it)

Gets the specified row access policy by policy ID.

### HTTP request

`GET https://bigquery.googleapis.com/bigquery/v2/projects/{projectId}/datasets/{datasetId}/tables/{tableId}/rowAccessPolicies/{policyId}`

The URL uses [gRPC Transcoding](https://google.aip.dev/127) syntax.

### Path parameters

| Parameters ||
|---|---|
| `projectId` | `string` Required. Project ID of the table to get the row access policy. |
| `datasetId` | `string` Required. Dataset ID of the table to get the row access policy. |
| `tableId` | `string` Required. Table ID of the table to get the row access policy. |
| `policyId` | `string` Required. Policy ID of the row access policy. |

### Request body

The request body must be empty.

### Response body

If successful, the response body contains an instance of `https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/rowAccessPolicies#RowAccessPolicy`.

### Authorization scopes

Requires one of the following OAuth scopes:

- `https://www.googleapis.com/auth/bigquery`
- `https://www.googleapis.com/auth/cloud-platform`
- `https://www.googleapis.com/auth/bigquery.readonly`
- `https://www.googleapis.com/auth/cloud-platform.read-only`

For more information, see the [Authentication Overview](https://docs.cloud.google.com/docs/authentication#authorization-gcp).