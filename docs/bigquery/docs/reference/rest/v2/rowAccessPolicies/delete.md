# Method: rowAccessPolicies.delete

- [HTTP request](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/rowAccessPolicies/delete#body.HTTP_TEMPLATE)
- [Path parameters](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/rowAccessPolicies/delete#body.PATH_PARAMETERS)
- [Query parameters](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/rowAccessPolicies/delete#body.QUERY_PARAMETERS)
- [Request body](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/rowAccessPolicies/delete#body.request_body)
- [Response body](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/rowAccessPolicies/delete#body.response_body)
- [Authorization scopes](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/rowAccessPolicies/delete#body.aspect)
- [Try it!](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/rowAccessPolicies/delete#try-it)

Deletes a row access policy.

### HTTP request

`DELETE https://bigquery.googleapis.com/bigquery/v2/projects/{projectId}/datasets/{datasetId}/tables/{tableId}/rowAccessPolicies/{policyId}`

The URL uses [gRPC Transcoding](https://google.aip.dev/127) syntax.

### Path parameters

| Parameters ||
|---|---|
| `projectId` | `string` Required. Project ID of the table to delete the row access policy. |
| `datasetId` | `string` Required. Dataset ID of the table to delete the row access policy. |
| `tableId` | `string` Required. Table ID of the table to delete the row access policy. |
| `policyId` | `string` Required. Policy ID of the row access policy. |

### Query parameters

| Parameters ||
|---|---|
| `force` | `boolean` If set to true, it deletes the row access policy even if it's the last row access policy on the table and the deletion will widen the access rather narrowing it. |

### Request body

The request body must be empty.

### Response body

If successful, the response body is an empty JSON object.

### Authorization scopes

Requires one of the following OAuth scopes:

- `https://www.googleapis.com/auth/bigquery`
- `https://www.googleapis.com/auth/cloud-platform`

For more information, see the [Authentication Overview](https://docs.cloud.google.com/docs/authentication#authorization-gcp).