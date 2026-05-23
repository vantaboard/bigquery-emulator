# Method: models.get

- [HTTP request](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/models/get#body.HTTP_TEMPLATE)
- [Path parameters](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/models/get#body.PATH_PARAMETERS)
- [Request body](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/models/get#body.request_body)
- [Response body](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/models/get#body.response_body)
- [Authorization scopes](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/models/get#body.aspect)
- [Try it!](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/models/get#try-it)

Gets the specified model resource by model ID.

### HTTP request

`GET https://bigquery.googleapis.com/bigquery/v2/projects/{projectId}/datasets/{datasetId}/models/{modelId}`

The URL uses [gRPC Transcoding](https://google.aip.dev/127) syntax.

### Path parameters

| Parameters ||
|---|---|
| `projectId` | `string` Required. Project ID of the requested model. |
| `datasetId` | `string` Required. Dataset ID of the requested model. |
| `modelId` | `string` Required. Model ID of the requested model. |

### Request body

The request body must be empty.

### Response body

If successful, the response body contains an instance of `https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/models#Model`.

### Authorization scopes

Requires one of the following OAuth scopes:

- `https://www.googleapis.com/auth/bigquery`
- `https://www.googleapis.com/auth/cloud-platform`
- `https://www.googleapis.com/auth/bigquery.readonly`
- `https://www.googleapis.com/auth/cloud-platform.read-only`

For more information, see the [Authentication Overview](https://docs.cloud.google.com/docs/authentication#authorization-gcp).