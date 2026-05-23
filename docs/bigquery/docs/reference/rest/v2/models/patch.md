# Method: models.patch

- [HTTP request](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/models/patch#body.HTTP_TEMPLATE)
- [Path parameters](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/models/patch#body.PATH_PARAMETERS)
- [Request body](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/models/patch#body.request_body)
- [Response body](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/models/patch#body.response_body)
- [Authorization scopes](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/models/patch#body.aspect)
- [Try it!](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/models/patch#try-it)

Patch specific fields in the specified model.

### HTTP request

`PATCH https://bigquery.googleapis.com/bigquery/v2/projects/{projectId}/datasets/{datasetId}/models/{modelId}`

The URL uses [gRPC Transcoding](https://google.aip.dev/127) syntax.

### Path parameters

| Parameters ||
|---|---|
| `projectId` | `string` Required. Project ID of the model to patch. |
| `datasetId` | `string` Required. Dataset ID of the model to patch. |
| `modelId` | `string` Required. Model ID of the model to patch. |

### Request body

The request body contains an instance of `https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/models#Model`.

### Response body

If successful, the response body contains an instance of `https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/models#Model`.

### Authorization scopes

Requires one of the following OAuth scopes:

- `https://www.googleapis.com/auth/bigquery`
- `https://www.googleapis.com/auth/cloud-platform`

For more information, see the [Authentication Overview](https://docs.cloud.google.com/docs/authentication#authorization-gcp).