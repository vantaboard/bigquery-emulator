# Method: projects.locations.workflows.create

- [HTTP request](https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/v2/projects.locations.workflows/create#body.HTTP_TEMPLATE)
- [Path parameters](https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/v2/projects.locations.workflows/create#body.PATH_PARAMETERS)
- [Request body](https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/v2/projects.locations.workflows/create#body.request_body)
- [Response body](https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/v2/projects.locations.workflows/create#body.response_body)
- [Authorization scopes](https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/v2/projects.locations.workflows/create#body.aspect)
- [IAM Permissions](https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/v2/projects.locations.workflows/create#body.aspect_1)
- [Try it!](https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/v2/projects.locations.workflows/create#try-it)

Creates a migration workflow.

### HTTP request

`POST https://bigquerymigration.googleapis.com/v2/{parent=projects/*/locations/*}/workflows`

The URL uses [gRPC Transcoding](https://google.aip.dev/127) syntax.

### Path parameters

| Parameters ||
|---|---|
| `parent` | `string` Required. The name of the project to which this migration workflow belongs. Example: `projects/foo/locations/bar` |

### Request body

The request body contains an instance of `https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/v2/projects.locations.workflows#MigrationWorkflow`.

### Response body

If successful, the response body contains a newly created instance of `https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/v2/projects.locations.workflows#MigrationWorkflow`.

### Authorization scopes

Requires the following OAuth scope:

- `https://www.googleapis.com/auth/cloud-platform`

For more information, see the [Authentication Overview](https://docs.cloud.google.com/docs/authentication#authorization-gcp).

### IAM Permissions

Requires the following [IAM](https://cloud.google.com/iam/docs) permission on the `parent` resource:

- `bigquerymigration.workflows.create`

For more information, see the [IAM documentation](https://cloud.google.com/iam/docs).