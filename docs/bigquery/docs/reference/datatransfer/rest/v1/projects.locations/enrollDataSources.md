- [HTTP request](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations/enrollDataSources#body.HTTP_TEMPLATE)
- [Path parameters](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations/enrollDataSources#body.PATH_PARAMETERS)
- [Request body](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations/enrollDataSources#body.request_body)
  - [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations/enrollDataSources#body.request_body.SCHEMA_REPRESENTATION)
- [Response body](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations/enrollDataSources#body.response_body)
- [Authorization scopes](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations/enrollDataSources#body.aspect)
- [Try it!](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations/enrollDataSources#try-it)

**Full name**: projects.locations.enrollDataSources

Enroll data sources in a user project. This allows users to create transfer configurations for these data sources. They will also appear in the ListDataSources RPC and as such, will appear in the [BigQuery UI](https://console.cloud.google.com/bigquery), and the documents can be found in the public guide for [BigQuery Web UI](https://cloud.google.com/bigquery/bigquery-web-ui) and [Data Transfer Service](https://cloud.google.com/bigquery/docs/working-with-transfers).

### HTTP request

Choose a location:
<button value="global" default="">global</button> <button value="asia-south1">asia-south1</button> <button value="asia-south2">asia-south2</button> <button value="europe-west1">europe-west1</button> <button value="europe-west2">europe-west2</button> <button value="europe-west3">europe-west3</button> <button value="europe-west4">europe-west4</button> <button value="europe-west6">europe-west6</button> <button value="europe-west8">europe-west8</button> <button value="europe-west9">europe-west9</button> <button value="me-central2">me-central2</button> <button value="northamerica-northeast1">northamerica-northeast1</button> <button value="northamerica-northeast2">northamerica-northeast2</button> <button value="us-central1">us-central1</button> <button value="us-central2">us-central2</button> <button value="us-east1">us-east1</button> <button value="us-east4">us-east4</button> <button value="us-east5">us-east5</button> <button value="us-east7">us-east7</button> <button value="us-south1">us-south1</button> <button value="us-west1">us-west1</button> <button value="us-west2">us-west2</button> <button value="us-west3">us-west3</button> <button value="us-west4">us-west4</button> <button value="us-west8">us-west8</button>   
`POST https://bigquerydatatransfer.googleapis.com/v1/{name=projects/*/locations/*}:enrollDataSources`

<br />

The URLs use [gRPC Transcoding](https://google.aip.dev/127) syntax.

### Path parameters

| Parameters ||
|---|---|
| `name` | `string` Required. The name of the project resource in the form: `projects/{projectId}` Authorization requires the following [IAM](https://cloud.google.com/iam/docs/) permission on the specified resource `name`: - `resourcemanager.projects.update` |

### Request body

The request body contains data with the following structure:

| JSON representation |
|---|
| ``` { "dataSourceIds": [ string ] } ``` |

| Fields ||
|---|---|
| `dataSourceIds[]` | `string` Data sources that are enrolled. It is required to provide at least one data source id. |

### Response body

If successful, the response body is an empty JSON object.

### Authorization scopes

Requires the following OAuth scope:

- `https://www.googleapis.com/auth/cloud-platform`

For more information, see the [Authentication Overview](https://docs.cloud.google.com/docs/authentication#authorization-gcp).