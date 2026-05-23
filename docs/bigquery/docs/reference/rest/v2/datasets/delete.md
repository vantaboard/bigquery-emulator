# Method: datasets.delete

- [HTTP request](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets/delete#body.HTTP_TEMPLATE)
- [Path parameters](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets/delete#body.PATH_PARAMETERS)
- [Query parameters](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets/delete#body.QUERY_PARAMETERS)
- [Request body](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets/delete#body.request_body)
- [Response body](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets/delete#body.response_body)
- [Authorization scopes](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets/delete#body.aspect)
- [Try it!](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets/delete#try-it)

Deletes the dataset specified by the datasetId value. Before you can delete a dataset, you must delete all its tables, either manually or by specifying deleteContents. Immediately after deletion, you can create another dataset with the same name.

### HTTP request

`DELETE https://bigquery.googleapis.com/bigquery/v2/projects/{projectId}/datasets/{datasetId}`

The URL uses [gRPC Transcoding](https://google.aip.dev/127) syntax.

### Path parameters

| Parameters ||
|---|---|
| `projectId` | `string` Required. Project ID of the dataset being deleted |
| `datasetId` | `string` Required. Dataset ID of dataset being deleted |

### Query parameters

| Parameters ||
|---|---|
| `deleteContents` | `boolean` If True, delete all the tables in the dataset. If False and the dataset contains tables, the request will fail. Default is False |

### Request body

The request body must be empty.

### Response body

If successful, the response body is an empty JSON object.

### Authorization scopes

Requires one of the following OAuth scopes:

- `https://www.googleapis.com/auth/bigquery`
- `https://www.googleapis.com/auth/cloud-platform`

For more information, see the [Authentication Overview](https://docs.cloud.google.com/docs/authentication#authorization-gcp).