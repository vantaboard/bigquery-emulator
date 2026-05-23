# Method: models.delete

- [HTTP request](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/models/delete#body.HTTP_TEMPLATE)
- [Path parameters](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/models/delete#body.PATH_PARAMETERS)
- [Request body](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/models/delete#body.request_body)
- [Response body](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/models/delete#body.response_body)
- [Authorization scopes](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/models/delete#body.aspect)
- [Try it!](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/models/delete#try-it)

Deletes the model specified by modelId from the dataset.

### HTTP request

`DELETE https://bigquery.googleapis.com/bigquery/v2/projects/{projectId}/datasets/{datasetId}/models/{modelId}`

The URL uses [gRPC Transcoding](https://google.aip.dev/127) syntax.

### Path parameters

| Parameters ||
|---|---|
| `projectId` | `string` Required. Project ID of the model to delete. |
| `datasetId` | `string` Required. Dataset ID of the model to delete. |
| `modelId` | `string` Required. Model ID of the model to delete. |

### Request body

The request body must be empty.

### Response body

If successful, the response body is an empty JSON object.

### Authorization scopes

Requires one of the following OAuth scopes:

- `https://www.googleapis.com/auth/bigquery`
- `https://www.googleapis.com/auth/cloud-platform`

For more information, see the [Authentication Overview](https://docs.cloud.google.com/docs/authentication#authorization-gcp).