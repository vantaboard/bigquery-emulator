- [HTTP request](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs.transferResources/list#body.HTTP_TEMPLATE)
- [Path parameters](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs.transferResources/list#body.PATH_PARAMETERS)
- [Query parameters](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs.transferResources/list#body.QUERY_PARAMETERS)
- [Request body](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs.transferResources/list#body.request_body)
- [Response body](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs.transferResources/list#body.response_body)
- [Authorization scopes](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs.transferResources/list#body.aspect)
- [Try it!](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs.transferResources/list#try-it)

**Full name**: projects.locations.transferConfigs.transferResources.list

Returns information about transfer resources.

### HTTP request

Choose a location:
<button value="global" default="">global</button> <button value="asia-south1">asia-south1</button> <button value="asia-south2">asia-south2</button> <button value="europe-west1">europe-west1</button> <button value="europe-west2">europe-west2</button> <button value="europe-west3">europe-west3</button> <button value="europe-west4">europe-west4</button> <button value="europe-west6">europe-west6</button> <button value="europe-west8">europe-west8</button> <button value="europe-west9">europe-west9</button> <button value="me-central2">me-central2</button> <button value="northamerica-northeast1">northamerica-northeast1</button> <button value="northamerica-northeast2">northamerica-northeast2</button> <button value="us-central1">us-central1</button> <button value="us-central2">us-central2</button> <button value="us-east1">us-east1</button> <button value="us-east4">us-east4</button> <button value="us-east5">us-east5</button> <button value="us-east7">us-east7</button> <button value="us-south1">us-south1</button> <button value="us-west1">us-west1</button> <button value="us-west2">us-west2</button> <button value="us-west3">us-west3</button> <button value="us-west4">us-west4</button> <button value="us-west8">us-west8</button>   
`GET https://bigquerydatatransfer.googleapis.com/v1/{parent=projects/*/locations/*/transferConfigs/*}/transferResources`

<br />

The URLs use [gRPC Transcoding](https://google.aip.dev/127) syntax.

### Path parameters

| Parameters ||
|---|---|
| `parent` | `string` Required. Name of transfer configuration for which transfer resources should be retrieved. The name should be in one of the following forms: - `projects/{project}/transferConfigs/{transferConfig}` - `projects/{project}/locations/{locationId}/transferConfigs/{transferConfig}` Authorization requires the following [IAM](https://cloud.google.com/iam/docs/) permission on the specified resource `parent`: - `bigquery.transfers.get` |

### Query parameters

| Parameters ||
|---|---|
| `pageSize` | `integer` Optional. The maximum number of transfer resources to return. The maximum value is 1000; values above 1000 will be coerced to 1000. The default page size is the maximum value of 1000 results. |
| `pageToken` | `string` Optional. A page token, received from a previous `transferResources.list` call. Provide this to retrieve the subsequent page. When paginating, all other parameters provided to `transferResources.list` must match the call that provided the page token. |
| `filter` | `string` Optional. Filter for the transfer resources. Currently supported filters include: - Resource name: `name` - Wildcard supported - Resource type: `type` - Resource destination: `destination` - Latest resource state: `latest_status_detail.state` - Last update time: `update_time` - RFC-3339 format - Parent table name: `hierarchy_detail.partition_detail.table` Multiple filters can be applied using the `AND/OR` operator. Examples: - `name="*123" AND (type="TABLE" OR latest_status_detail.state="SUCCEEDED")` - `update_time >= "2012-04-21T11:30:00-04:00"` - `hierarchy_detail.partition_detail.table = "table1"` |

### Request body

The request body must be empty.

### Response body

If successful, the response body contains an instance of `https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/ListTransferResourcesResponse`.

### Authorization scopes

Requires the following OAuth scope:

- `https://www.googleapis.com/auth/cloud-platform`

For more information, see the [Authentication Overview](https://docs.cloud.google.com/docs/authentication#authorization-gcp).