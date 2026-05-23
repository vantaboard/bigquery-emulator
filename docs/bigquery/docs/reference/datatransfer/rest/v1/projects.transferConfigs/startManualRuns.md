- [HTTP request](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.transferConfigs/startManualRuns#body.HTTP_TEMPLATE)
- [Path parameters](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.transferConfigs/startManualRuns#body.PATH_PARAMETERS)
- [Request body](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.transferConfigs/startManualRuns#body.request_body)
  - [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.transferConfigs/startManualRuns#body.request_body.SCHEMA_REPRESENTATION)
- [Response body](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.transferConfigs/startManualRuns#body.response_body)
- [Authorization scopes](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.transferConfigs/startManualRuns#body.aspect)
- [Try it!](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.transferConfigs/startManualRuns#try-it)

**Full name**: projects.transferConfigs.startManualRuns

Manually initiates transfer runs. You can schedule these runs in two ways:

1. For a specific point in time using the 'requestedRunTime' parameter.
2. For a period between 'startTime' (inclusive) and 'endTime' (exclusive).

If scheduling a single run, it is set to execute immediately (scheduleTime equals the current time). When scheduling multiple runs within a time range, the first run starts now, and subsequent runs are delayed by 15 seconds each.

### HTTP request

Choose a location:
<button value="global" default="">global</button> <button value="asia-south1">asia-south1</button> <button value="asia-south2">asia-south2</button> <button value="europe-west1">europe-west1</button> <button value="europe-west2">europe-west2</button> <button value="europe-west3">europe-west3</button> <button value="europe-west4">europe-west4</button> <button value="europe-west6">europe-west6</button> <button value="europe-west8">europe-west8</button> <button value="europe-west9">europe-west9</button> <button value="me-central2">me-central2</button> <button value="northamerica-northeast1">northamerica-northeast1</button> <button value="northamerica-northeast2">northamerica-northeast2</button> <button value="us-central1">us-central1</button> <button value="us-central2">us-central2</button> <button value="us-east1">us-east1</button> <button value="us-east4">us-east4</button> <button value="us-east5">us-east5</button> <button value="us-east7">us-east7</button> <button value="us-south1">us-south1</button> <button value="us-west1">us-west1</button> <button value="us-west2">us-west2</button> <button value="us-west3">us-west3</button> <button value="us-west4">us-west4</button> <button value="us-west8">us-west8</button>   
`POST https://bigquerydatatransfer.googleapis.com/v1/{parent=projects/*/transferConfigs/*}:startManualRuns`

<br />

The URLs use [gRPC Transcoding](https://google.aip.dev/127) syntax.

### Path parameters

| Parameters ||
|---|---|
| `parent` | `string` Required. Transfer configuration name. If you are using the regionless method, the location must be `US` and the name should be in the following form: - `projects/{projectId}/transferConfigs/{configId}` If you are using the regionalized method, the name should be in the following form: - `projects/{projectId}/locations/{locationId}/transferConfigs/{configId}` Authorization requires the following [IAM](https://cloud.google.com/iam/docs/) permission on the specified resource `parent`: - `bigquery.transfers.update` |

### Request body

The request body contains data with the following structure:

| JSON representation |
|---|
| ``` { // Union field `time` can be only one of the following: "requestedTimeRange": { object (`https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/TimeRange`) }, "requestedRunTime": string // End of list of possible types for union field `time`. } ``` |

| Fields ||
|---|---|
| Union field `time`. The requested time specification - this can be a time range or a specific run_time. `time` can be only one of the following: ||
| `requestedTimeRange` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/TimeRange`)`` A time_range start and end timestamp for historical data files or reports that are scheduled to be transferred by the scheduled transfer run. requestedTimeRange must be a past time and cannot include future time values. |
| `requestedRunTime` | ``string (`https://protobuf.dev/reference/protobuf/google.protobuf#timestamp` format)`` A runTime timestamp for historical data files or reports that are scheduled to be transferred by the scheduled transfer run. requestedRunTime must be a past time and cannot include future time values. |

### Response body

If successful, the response body contains an instance of `https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/StartManualTransferRunsResponse`.

### Authorization scopes

Requires the following OAuth scope:

- `https://www.googleapis.com/auth/cloud-platform`

For more information, see the [Authentication Overview](https://docs.cloud.google.com/docs/authentication#authorization-gcp).