# Method: projects.list

- [HTTP request](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/projects/list#body.HTTP_TEMPLATE)
- [Query parameters](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/projects/list#body.QUERY_PARAMETERS)
- [Request body](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/projects/list#body.request_body)
- [Response body](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/projects/list#body.response_body)
  - [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/projects/list#body.ProjectList.SCHEMA_REPRESENTATION)
- [Authorization scopes](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/projects/list#body.aspect)
- [Try it!](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/projects/list#try-it)

RPC to list projects to which the user has been granted any project role.

Users of this method are encouraged to consider the [Resource Manager](https://cloud.google.com/resource-manager/docs/) API, which provides the underlying data for this method and has more capabilities.

### HTTP request

`GET https://bigquery.googleapis.com/bigquery/v2/projects`

The URL uses [gRPC Transcoding](https://google.aip.dev/127) syntax.

### Query parameters

| Parameters ||
|---|---|
| `maxResults` | `integer` `maxResults` unset returns all results, up to 50 per page. Additionally, the number of projects in a page may be fewer than `maxResults` because projects are retrieved and then filtered to only projects with the BigQuery API enabled. |
| `pageToken` | `string` Page token, returned by a previous call, to request the next page of results. If not present, no further pages are present. |

### Request body

The request body must be empty.

### Response body

Response object of projects.list

If successful, the response body contains data with the following structure:

| JSON representation |
|---|
| ``` { "kind": string, "etag": string, "nextPageToken": string, "projects": [ { "kind": string, "id": string, "numericId": string, "projectReference": { object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/ProjectReference`) }, "friendlyName": string } ], "totalItems": integer } ``` |

| Fields ||
|---|---|
| `kind` | `string` The resource type of the response. |
| `etag` | `string` A hash of the page of results. |
| `nextPageToken` | `string` Use this token to request the next page of results. |
| `projects[]` | `object` Projects to which the user has at least READ access. This field can be omitted if `totalItems` is 0. |
| `projects[].kind` | `string` The resource type. |
| `projects[].id` | `string` An opaque ID of this project. |
| `projects[].numericId` | `string (https://developers.google.com/discovery/v1/type-format format)` The numeric ID of this project. |
| `projects[].projectReference` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/ProjectReference`)`` A unique reference to this project. |
| `projects[].friendlyName` | `string` A descriptive name for this project. A wrapper is used here because friendlyName can be set to the empty string. |
| `totalItems` | `integer` The total number of projects in the page. A wrapper is used here because the field should still be in the response when the value is 0. |

### Authorization scopes

Requires one of the following OAuth scopes:

- `https://www.googleapis.com/auth/bigquery`
- `https://www.googleapis.com/auth/cloud-platform`
- `https://www.googleapis.com/auth/bigquery.readonly`
- `https://www.googleapis.com/auth/cloud-platform.read-only`

For more information, see the [Authentication Overview](https://docs.cloud.google.com/docs/authentication#authorization-gcp).