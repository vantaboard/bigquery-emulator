# Method: rowAccessPolicies.update

- [HTTP request](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/rowAccessPolicies/update#body.HTTP_TEMPLATE)
- [Path parameters](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/rowAccessPolicies/update#body.PATH_PARAMETERS)
- [Request body](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/rowAccessPolicies/update#body.request_body)
- [Response body](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/rowAccessPolicies/update#body.response_body)
- [Authorization scopes](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/rowAccessPolicies/update#body.aspect)
- [Try it!](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/rowAccessPolicies/update#try-it)

Updates a row access policy.

### HTTP request

`PUT https://bigquery.googleapis.com/bigquery/v2/projects/{projectId}/datasets/{datasetId}/tables/{tableId}/rowAccessPolicies/{policyId}`

The URL uses [gRPC Transcoding](https://google.aip.dev/127) syntax.

### Path parameters

| Parameters ||
|---|---|
| `projectId` | `string` Required. Project ID of the table to get the row access policy. |
| `datasetId` | `string` Required. Dataset ID of the table to get the row access policy. |
| `tableId` | `string` Required. Table ID of the table to get the row access policy. |
| `policyId` | `string` Required. Policy ID of the row access policy. |

### Request body

The request body contains an instance of `https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/rowAccessPolicies#RowAccessPolicy`.

### Response body

If successful, the response body contains an instance of `https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/rowAccessPolicies#RowAccessPolicy`.

### Authorization scopes

Requires one of the following OAuth scopes:

- `https://www.googleapis.com/auth/bigquery`
- `https://www.googleapis.com/auth/cloud-platform`

For more information, see the [Authentication Overview](https://docs.cloud.google.com/docs/authentication#authorization-gcp).