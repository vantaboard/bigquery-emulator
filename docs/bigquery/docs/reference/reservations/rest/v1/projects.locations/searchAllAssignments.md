# Method: projects.locations.searchAllAssignments

- [HTTP request](https://docs.cloud.google.com/bigquery/docs/reference/reservations/rest/v1/projects.locations/searchAllAssignments#body.HTTP_TEMPLATE)
- [Path parameters](https://docs.cloud.google.com/bigquery/docs/reference/reservations/rest/v1/projects.locations/searchAllAssignments#body.PATH_PARAMETERS)
- [Query parameters](https://docs.cloud.google.com/bigquery/docs/reference/reservations/rest/v1/projects.locations/searchAllAssignments#body.QUERY_PARAMETERS)
- [Request body](https://docs.cloud.google.com/bigquery/docs/reference/reservations/rest/v1/projects.locations/searchAllAssignments#body.request_body)
- [Response body](https://docs.cloud.google.com/bigquery/docs/reference/reservations/rest/v1/projects.locations/searchAllAssignments#body.response_body)
  - [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/reservations/rest/v1/projects.locations/searchAllAssignments#body.SearchAllAssignmentsResponse.SCHEMA_REPRESENTATION)
- [Authorization scopes](https://docs.cloud.google.com/bigquery/docs/reference/reservations/rest/v1/projects.locations/searchAllAssignments#body.aspect)
- [Try it!](https://docs.cloud.google.com/bigquery/docs/reference/reservations/rest/v1/projects.locations/searchAllAssignments#try-it)

Looks up assignments for a specified resource for a particular region. If the request is about a project:

1. Assignments created on the project will be returned if they exist.
2. Otherwise assignments created on the closest ancestor will be returned.
3. Assignments for different JobTypes will all be returned.

The same logic applies if the request is about a folder.

If the request is about an organization, then assignments created on the organization will be returned (organization doesn't have ancestors).

Comparing to assignments.list, there are some behavior differences:

1. permission on the assignee will be verified in this API.
2. Hierarchy lookup (project-\>folder-\>organization) happens in this API.
3. Parent here is `projects/*/locations/*`, instead of `projects/*/locations/*reservations/*`.

### HTTP request

`GET https://bigqueryreservation.googleapis.com/v1/{parent=projects/*/locations/*}:searchAllAssignments`

The URL uses [gRPC Transcoding](https://google.aip.dev/127) syntax.

### Path parameters

| Parameters ||
|---|---|
| `parent` | `string` Required. The resource name with location (project name could be the wildcard '-'), e.g.: `projects/-/locations/US`. Authorization requires the following [IAM](https://cloud.google.com/iam/docs/) permission on the specified resource `parent`: - `bigquery.reservationAssignments.search` |

### Query parameters

| Parameters ||
|---|---|
| `query` | `string` Please specify resource name as assignee in the query. Examples: - `assignee=projects/myproject` - `assignee=folders/123` - `assignee=organizations/456` |
| `pageSize` | `integer` The maximum number of items to return per page. |
| `pageToken` | `string` The nextPageToken value returned from a previous List request, if any. |

### Request body

The request body must be empty.

### Response body

The response for `https://docs.cloud.google.com/bigquery/docs/reference/reservations/rest/v1/projects.locations/searchAllAssignments#google.cloud.bigquery.reservation.v1.ReservationService.SearchAllAssignments`.

If successful, the response body contains data with the following structure:

| JSON representation |
|---|
| ``` { "assignments": [ { object (`https://docs.cloud.google.com/bigquery/docs/reference/reservations/rest/v1/projects.locations.reservations.assignments#Assignment`) } ], "nextPageToken": string } ``` |

| Fields ||
|---|---|
| `assignments[]` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/reservations/rest/v1/projects.locations.reservations.assignments#Assignment`)`` List of assignments visible to the user. |
| `nextPageToken` | `string` Token to retrieve the next page of results, or empty if there are no more results in the list. |

### Authorization scopes

Requires one of the following OAuth scopes:

- `https://www.googleapis.com/auth/bigquery`
- `https://www.googleapis.com/auth/cloud-platform`

For more information, see the [Authentication Overview](https://docs.cloud.google.com/docs/authentication#authorization-gcp).