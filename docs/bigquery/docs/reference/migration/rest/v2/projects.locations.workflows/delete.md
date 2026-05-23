# Method: projects.locations.workflows.delete

- [HTTP request](https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/v2/projects.locations.workflows/delete#body.HTTP_TEMPLATE)
- [Path parameters](https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/v2/projects.locations.workflows/delete#body.PATH_PARAMETERS)
- [Request body](https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/v2/projects.locations.workflows/delete#body.request_body)
- [Response body](https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/v2/projects.locations.workflows/delete#body.response_body)
- [Authorization scopes](https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/v2/projects.locations.workflows/delete#body.aspect)
- [IAM Permissions](https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/v2/projects.locations.workflows/delete#body.aspect_1)
- [Try it!](https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/v2/projects.locations.workflows/delete#try-it)

Deletes a migration workflow by name.

### HTTP request

`DELETE https://bigquerymigration.googleapis.com/v2/{name=projects/*/locations/*/workflows/*}`

The URL uses [gRPC Transcoding](https://google.aip.dev/127) syntax.

### Path parameters

| Parameters ||
|---|---|
| `name` | `string` Required. The unique identifier for the migration workflow. Example: `projects/123/locations/us/workflows/1234` |

### Request body

The request body must be empty.

### Response body

If successful, the response body is an empty JSON object.

### Authorization scopes

Requires the following OAuth scope:

- `https://www.googleapis.com/auth/cloud-platform`

For more information, see the [Authentication Overview](https://docs.cloud.google.com/docs/authentication#authorization-gcp).

### IAM Permissions

Requires the following [IAM](https://cloud.google.com/iam/docs) permission on the `name` resource:

- `bigquerymigration.workflows.delete`

For more information, see the [IAM documentation](https://cloud.google.com/iam/docs).