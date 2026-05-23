# Method: routines.list

- [HTTP request](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/routines/list#body.HTTP_TEMPLATE)
- [Path parameters](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/routines/list#body.PATH_PARAMETERS)
- [Query parameters](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/routines/list#body.QUERY_PARAMETERS)
- [Request body](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/routines/list#body.request_body)
- [Response body](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/routines/list#body.response_body)
  - [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/routines/list#body.ListRoutinesResponse.SCHEMA_REPRESENTATION)
- [Authorization scopes](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/routines/list#body.aspect)
- [Try it!](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/routines/list#try-it)

Lists all routines in the specified dataset. Requires the READER dataset role.

### HTTP request

`GET https://bigquery.googleapis.com/bigquery/v2/projects/{projectId}/datasets/{datasetId}/routines`

The URL uses [gRPC Transcoding](https://google.aip.dev/127) syntax.

### Path parameters

| Parameters ||
|---|---|
| `projectId` | `string` Required. Project ID of the routines to list |
| `datasetId` | `string` Required. Dataset ID of the routines to list |

### Query parameters

| Parameters ||
|---|---|
| `maxResults` | `integer` The maximum number of results to return in a single response page. Leverage the page tokens to iterate through the entire collection. |
| `pageToken` | `string` Page token, returned by a previous call, to request the next page of results |
| `readMask` | ``string (`https://protobuf.dev/reference/protobuf/google.protobuf#field-mask` format)`` If set, then only the Routine fields in the field mask, as well as projectId, datasetId and routineId, are returned in the response. If unset, then the following Routine fields are returned: etag, projectId, datasetId, routineId, routineType, creationTime, lastModifiedTime, and language. This is a comma-separated list of fully qualified names of fields. Example: `"user.displayName,photo"`. |
| `filter` | `string` If set, then only the Routines matching this filter are returned. The supported format is `routineType:{RoutineType}`, where `{RoutineType}` is a RoutineType enum. For example: `routineType:SCALAR_FUNCTION`. |

### Request body

The request body must be empty.

### Response body

Describes the format of a single result page when listing routines.

If successful, the response body contains data with the following structure:

| JSON representation |
|---|
| ``` { "routines": [ { object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/routines#Routine`) } ], "nextPageToken": string } ``` |

| Fields ||
|---|---|
| `routines[]` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/routines#Routine`)`` Routines in the requested dataset. Unless readMask is set in the request, only the following fields are populated: etag, projectId, datasetId, routineId, routineType, creationTime, lastModifiedTime, language, and remoteFunctionOptions. |
| `nextPageToken` | `string` A token to request the next page of results. |

### Authorization scopes

Requires one of the following OAuth scopes:

- `https://www.googleapis.com/auth/bigquery`
- `https://www.googleapis.com/auth/cloud-platform`
- `https://www.googleapis.com/auth/bigquery.readonly`
- `https://www.googleapis.com/auth/cloud-platform.read-only`

For more information, see the [Authentication Overview](https://docs.cloud.google.com/docs/authentication#authorization-gcp).