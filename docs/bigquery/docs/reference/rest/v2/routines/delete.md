# Method: routines.delete

- [HTTP request](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/routines/delete#body.HTTP_TEMPLATE)
- [Path parameters](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/routines/delete#body.PATH_PARAMETERS)
- [Request body](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/routines/delete#body.request_body)
- [Response body](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/routines/delete#body.response_body)
- [Authorization scopes](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/routines/delete#body.aspect)
- [Try it!](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/routines/delete#try-it)

Deletes the routine specified by routineId from the dataset.

### HTTP request

`DELETE https://bigquery.googleapis.com/bigquery/v2/projects/{projectId}/datasets/{datasetId}/routines/{routineId}`

The URL uses [gRPC Transcoding](https://google.aip.dev/127) syntax.

### Path parameters

| Parameters ||
|---|---|
| `projectId` | `string` Required. Project ID of the routine to delete |
| `datasetId` | `string` Required. Dataset ID of the routine to delete |
| `routineId` | `string` Required. Routine ID of the routine to delete |

### Request body

The request body must be empty.

### Response body

If successful, the response body is an empty JSON object.

### Authorization scopes

Requires one of the following OAuth scopes:

- `https://www.googleapis.com/auth/bigquery`
- `https://www.googleapis.com/auth/cloud-platform`

For more information, see the [Authentication Overview](https://docs.cloud.google.com/docs/authentication#authorization-gcp).