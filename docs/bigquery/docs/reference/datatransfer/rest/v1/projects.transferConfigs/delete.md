- [HTTP request](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.transferConfigs/delete#body.HTTP_TEMPLATE)
- [Path parameters](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.transferConfigs/delete#body.PATH_PARAMETERS)
- [Request body](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.transferConfigs/delete#body.request_body)
- [Response body](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.transferConfigs/delete#body.response_body)
- [Authorization scopes](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.transferConfigs/delete#body.aspect)
- [Try it!](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.transferConfigs/delete#try-it)

**Full name**: projects.transferConfigs.delete

Deletes a data transfer configuration, including any associated transfer runs and logs.

### HTTP request

Choose a location:
<button value="global" default="">global</button> <button value="asia-south1">asia-south1</button> <button value="asia-south2">asia-south2</button> <button value="europe-west1">europe-west1</button> <button value="europe-west2">europe-west2</button> <button value="europe-west3">europe-west3</button> <button value="europe-west4">europe-west4</button> <button value="europe-west6">europe-west6</button> <button value="europe-west8">europe-west8</button> <button value="europe-west9">europe-west9</button> <button value="me-central2">me-central2</button> <button value="northamerica-northeast1">northamerica-northeast1</button> <button value="northamerica-northeast2">northamerica-northeast2</button> <button value="us-central1">us-central1</button> <button value="us-central2">us-central2</button> <button value="us-east1">us-east1</button> <button value="us-east4">us-east4</button> <button value="us-east5">us-east5</button> <button value="us-east7">us-east7</button> <button value="us-south1">us-south1</button> <button value="us-west1">us-west1</button> <button value="us-west2">us-west2</button> <button value="us-west3">us-west3</button> <button value="us-west4">us-west4</button> <button value="us-west8">us-west8</button>   
`DELETE https://bigquerydatatransfer.googleapis.com/v1/{name=projects/*/transferConfigs/*}`

<br />

The URLs use [gRPC Transcoding](https://google.aip.dev/127) syntax.

### Path parameters

| Parameters ||
|---|---|
| `name` | `string` Required. The name of the resource to delete. If you are using the regionless method, the location must be `US` and the name should be in the following form: - `projects/{projectId}/transferConfigs/{configId}` If you are using the regionalized method, the name should be in the following form: - `projects/{projectId}/locations/{locationId}/transferConfigs/{configId}` Authorization requires the following [IAM](https://cloud.google.com/iam/docs/) permission on the specified resource `name`: - `bigquery.transfers.update` |

### Request body

The request body must be empty.

### Response body

If successful, the response body is an empty JSON object.

### Authorization scopes

Requires the following OAuth scope:

- `https://www.googleapis.com/auth/cloud-platform`

For more information, see the [Authentication Overview](https://docs.cloud.google.com/docs/authentication#authorization-gcp).