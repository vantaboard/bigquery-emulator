# Method: projects.locations.reservations.assignments.delete

- [HTTP request](https://docs.cloud.google.com/bigquery/docs/reference/reservations/rest/v1/projects.locations.reservations.assignments/delete#body.HTTP_TEMPLATE)
- [Path parameters](https://docs.cloud.google.com/bigquery/docs/reference/reservations/rest/v1/projects.locations.reservations.assignments/delete#body.PATH_PARAMETERS)
- [Request body](https://docs.cloud.google.com/bigquery/docs/reference/reservations/rest/v1/projects.locations.reservations.assignments/delete#body.request_body)
- [Response body](https://docs.cloud.google.com/bigquery/docs/reference/reservations/rest/v1/projects.locations.reservations.assignments/delete#body.response_body)
- [Authorization scopes](https://docs.cloud.google.com/bigquery/docs/reference/reservations/rest/v1/projects.locations.reservations.assignments/delete#body.aspect)
- [Try it!](https://docs.cloud.google.com/bigquery/docs/reference/reservations/rest/v1/projects.locations.reservations.assignments/delete#try-it)

Deletes a assignment. No expansion will happen.

Example:

- Organization `organizationA` contains two projects, `project1` and `project2`.
- Reservation `res1` exists and was created previously.
- assignments.create was used previously to define the following associations between entities and reservations: `<organizationA, res1>` and `<project1, res1>`

In this example, deletion of the `<organizationA, res1>` assignment won't affect the other assignment `<project1, res1>`. After said deletion, queries from `project1` will still use `res1` while queries from `project2` will switch to use on-demand mode.

### HTTP request

`DELETE https://bigqueryreservation.googleapis.com/v1/{name=projects/*/locations/*/reservations/*/assignments/*}`

The URL uses [gRPC Transcoding](https://google.aip.dev/127) syntax.

### Path parameters

| Parameters ||
|---|---|
| `name` | `string` Required. Name of the resource, e.g. `projects/myproject/locations/US/reservations/team1-prod/assignments/123` Authorization requires the following [IAM](https://cloud.google.com/iam/docs/) permission on the specified resource `name`: - `bigquery.reservationAssignments.delete` |

### Request body

The request body must be empty.

### Response body

If successful, the response body is an empty JSON object.

### Authorization scopes

Requires one of the following OAuth scopes:

- `https://www.googleapis.com/auth/bigquery`
- `https://www.googleapis.com/auth/cloud-platform`

For more information, see the [Authentication Overview](https://docs.cloud.google.com/docs/authentication#authorization-gcp).