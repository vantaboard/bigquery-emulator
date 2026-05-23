# Method: projects.locations.dataPolicies.list

- [HTTP request](https://docs.cloud.google.com/bigquery/docs/reference/bigquerydatapolicy/rest/v1beta1/projects.locations.dataPolicies/list#body.HTTP_TEMPLATE)
- [Path parameters](https://docs.cloud.google.com/bigquery/docs/reference/bigquerydatapolicy/rest/v1beta1/projects.locations.dataPolicies/list#body.PATH_PARAMETERS)
- [Query parameters](https://docs.cloud.google.com/bigquery/docs/reference/bigquerydatapolicy/rest/v1beta1/projects.locations.dataPolicies/list#body.QUERY_PARAMETERS)
- [Request body](https://docs.cloud.google.com/bigquery/docs/reference/bigquerydatapolicy/rest/v1beta1/projects.locations.dataPolicies/list#body.request_body)
- [Response body](https://docs.cloud.google.com/bigquery/docs/reference/bigquerydatapolicy/rest/v1beta1/projects.locations.dataPolicies/list#body.response_body)
  - [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/bigquerydatapolicy/rest/v1beta1/projects.locations.dataPolicies/list#body.ListDataPoliciesResponse.SCHEMA_REPRESENTATION)
- [Authorization scopes](https://docs.cloud.google.com/bigquery/docs/reference/bigquerydatapolicy/rest/v1beta1/projects.locations.dataPolicies/list#body.aspect)
- [IAM Permissions](https://docs.cloud.google.com/bigquery/docs/reference/bigquerydatapolicy/rest/v1beta1/projects.locations.dataPolicies/list#body.aspect_1)
- [Try it!](https://docs.cloud.google.com/bigquery/docs/reference/bigquerydatapolicy/rest/v1beta1/projects.locations.dataPolicies/list#try-it)

List all of the data policies in the specified parent project.

### HTTP request

`GET https://bigquerydatapolicy.googleapis.com/v1beta1/{parent=projects/*/locations/*}/dataPolicies`

The URL uses [gRPC Transcoding](https://google.aip.dev/127) syntax.

### Path parameters

| Parameters ||
|---|---|
| `parent` | `string` Required. Resource name of the project for which to list data policies. Format is `projects/{projectNumber}/locations/{locationId}`. |

### Query parameters

| Parameters ||
|---|---|
| `pageSize` | `integer` The maximum number of data policies to return. Must be a value between 1 and 1000. If not set, defaults to 50. |
| `pageToken` | `string` The `nextPageToken` value returned from a previous list request, if any. If not set, defaults to an empty string. |

### Request body

The request body must be empty.

### Response body

Response message for the dataPolicies.list method.

If successful, the response body contains data with the following structure:

| JSON representation |
|---|
| ``` { "dataPolicies": [ { object (`https://docs.cloud.google.com/bigquery/docs/reference/bigquerydatapolicy/rest/v1beta1/projects.locations.dataPolicies#DataPolicy`) } ], "nextPageToken": string } ``` |

| Fields ||
|---|---|
| `dataPolicies[]` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/bigquerydatapolicy/rest/v1beta1/projects.locations.dataPolicies#DataPolicy`)`` Data policies that belong to the requested project. |
| `nextPageToken` | `string` Token used to retrieve the next page of results, or empty if there are no more results. |

### Authorization scopes

Requires one of the following OAuth scopes:

- `https://www.googleapis.com/auth/bigquery`
- `https://www.googleapis.com/auth/cloud-platform`

For more information, see the [Authentication Overview](https://docs.cloud.google.com/docs/authentication#authorization-gcp).

### IAM Permissions

Requires the following [IAM](https://cloud.google.com/iam/docs) permission on the `parent` resource:

- `bigquery.dataPolicies.list`

For more information, see the [IAM documentation](https://cloud.google.com/iam/docs).