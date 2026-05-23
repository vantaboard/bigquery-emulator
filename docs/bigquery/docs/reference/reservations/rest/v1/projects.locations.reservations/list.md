# Method: projects.locations.reservations.list

- [HTTP request](https://docs.cloud.google.com/bigquery/docs/reference/reservations/rest/v1/projects.locations.reservations/list#body.HTTP_TEMPLATE)
- [Path parameters](https://docs.cloud.google.com/bigquery/docs/reference/reservations/rest/v1/projects.locations.reservations/list#body.PATH_PARAMETERS)
- [Query parameters](https://docs.cloud.google.com/bigquery/docs/reference/reservations/rest/v1/projects.locations.reservations/list#body.QUERY_PARAMETERS)
- [Request body](https://docs.cloud.google.com/bigquery/docs/reference/reservations/rest/v1/projects.locations.reservations/list#body.request_body)
- [Response body](https://docs.cloud.google.com/bigquery/docs/reference/reservations/rest/v1/projects.locations.reservations/list#body.response_body)
  - [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/reservations/rest/v1/projects.locations.reservations/list#body.ListReservationsResponse.SCHEMA_REPRESENTATION)
- [Authorization scopes](https://docs.cloud.google.com/bigquery/docs/reference/reservations/rest/v1/projects.locations.reservations/list#body.aspect)
- [Try it!](https://docs.cloud.google.com/bigquery/docs/reference/reservations/rest/v1/projects.locations.reservations/list#try-it)

Lists all the reservations for the project in the specified location.

### HTTP request

`GET https://bigqueryreservation.googleapis.com/v1/{parent=projects/*/locations/*}/reservations`

The URL uses [gRPC Transcoding](https://google.aip.dev/127) syntax.

### Path parameters

| Parameters ||
|---|---|
| `parent` | `string` Required. The parent resource name containing project and location, e.g.: `projects/myproject/locations/US` Authorization requires the following [IAM](https://cloud.google.com/iam/docs/) permission on the specified resource `parent`: - `bigquery.reservations.list` |

### Query parameters

| Parameters ||
|---|---|
| `pageSize` | `integer` The maximum number of items to return per page. |
| `pageToken` | `string` The nextPageToken value returned from a previous List request, if any. |

### Request body

The request body must be empty.

### Response body

The response for `https://docs.cloud.google.com/bigquery/docs/reference/reservations/rest/v1/projects.locations.reservations/list#google.cloud.bigquery.reservation.v1.ReservationService.ListReservations`.

If successful, the response body contains data with the following structure:

| JSON representation |
|---|
| ``` { "reservations": [ { object (`https://docs.cloud.google.com/bigquery/docs/reference/reservations/rest/v1/projects.locations.reservations#Reservation`) } ], "nextPageToken": string } ``` |

| Fields ||
|---|---|
| `reservations[]` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/reservations/rest/v1/projects.locations.reservations#Reservation`)`` List of reservations visible to the user. |
| `nextPageToken` | `string` Token to retrieve the next page of results, or empty if there are no more results in the list. |

### Authorization scopes

Requires one of the following OAuth scopes:

- `https://www.googleapis.com/auth/bigquery`
- `https://www.googleapis.com/auth/cloud-platform`

For more information, see the [Authentication Overview](https://docs.cloud.google.com/docs/authentication#authorization-gcp).