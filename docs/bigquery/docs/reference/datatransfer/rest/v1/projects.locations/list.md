- [HTTP request](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations/list#body.HTTP_TEMPLATE)
- [Path parameters](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations/list#body.PATH_PARAMETERS)
- [Query parameters](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations/list#body.QUERY_PARAMETERS)
- [Request body](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations/list#body.request_body)
- [Response body](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations/list#body.response_body)
  - [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations/list#body.ListLocationsResponse.SCHEMA_REPRESENTATION)
- [Authorization scopes](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations/list#body.aspect)
- [Try it!](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations/list#try-it)

**Full name**: projects.locations.list

Lists information about the supported locations for this service.

This method lists locations based on the resource scope provided in the `https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations/list#body.PATH_PARAMETERS.name` field:

- **Global locations** : If `name` is empty, the method lists the public locations available to all projects.
- **Project-specific locations** : If `name` follows the format `projects/{project}`, the method lists locations visible to that specific project. This includes public, private, or other project-specific locations enabled for the project.

For gRPC and client library implementations, the resource name is passed as the `name` field. For direct service calls, the resource name is incorporated into the request path based on the specific service implementation and version.

### HTTP request

Choose a location:
<button value="global" default="">global</button> <button value="asia-south1">asia-south1</button> <button value="asia-south2">asia-south2</button> <button value="europe-west1">europe-west1</button> <button value="europe-west2">europe-west2</button> <button value="europe-west3">europe-west3</button> <button value="europe-west4">europe-west4</button> <button value="europe-west6">europe-west6</button> <button value="europe-west8">europe-west8</button> <button value="europe-west9">europe-west9</button> <button value="me-central2">me-central2</button> <button value="northamerica-northeast1">northamerica-northeast1</button> <button value="northamerica-northeast2">northamerica-northeast2</button> <button value="us-central1">us-central1</button> <button value="us-central2">us-central2</button> <button value="us-east1">us-east1</button> <button value="us-east4">us-east4</button> <button value="us-east5">us-east5</button> <button value="us-east7">us-east7</button> <button value="us-south1">us-south1</button> <button value="us-west1">us-west1</button> <button value="us-west2">us-west2</button> <button value="us-west3">us-west3</button> <button value="us-west4">us-west4</button> <button value="us-west8">us-west8</button>   
`GET https://bigquerydatatransfer.googleapis.com/v1/{name=projects/*}/locations`

<br />

The URLs use [gRPC Transcoding](https://google.aip.dev/127) syntax.

### Path parameters

| Parameters ||
|---|---|
| `name` | `string` The resource that owns the locations collection, if applicable. |

### Query parameters

| Parameters ||
|---|---|
| `filter` | `string` A filter to narrow down results to a preferred subset. The filtering language accepts strings like `"displayName=tokyo"`, and is documented in more detail in [AIP-160](https://google.aip.dev/160). |
| `pageSize` | `integer` The maximum number of results to return. If not set, the service selects a default. |
| `pageToken` | `string` A page token received from the `nextPageToken` field in the response. Send that page token to receive the subsequent page. |
| `extraLocationTypes[]` | `string` Optional. Do not use this field unless explicitly documented otherwise. This is primarily for internal usage. |

### Request body

The request body must be empty.

### Response body

The response message for `https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations/list#google.cloud.location.Locations.ListLocations`.

If successful, the response body contains data with the following structure:

| JSON representation |
|---|
| ``` { "locations": [ { object (`https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations#Location`) } ], "nextPageToken": string } ``` |

| Fields ||
|---|---|
| `locations[]` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations#Location`)`` A list of locations that matches the specified filter in the request. |
| `nextPageToken` | `string` The standard List next-page token. |

### Authorization scopes

Requires the following OAuth scope:

- `https://www.googleapis.com/auth/cloud-platform`

For more information, see the [Authentication Overview](https://docs.cloud.google.com/docs/authentication#authorization-gcp).