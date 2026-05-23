# Method: projects.locations.workflows.get

- [HTTP request](https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/v2alpha/projects.locations.workflows/get#body.HTTP_TEMPLATE)
- [Path parameters](https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/v2alpha/projects.locations.workflows/get#body.PATH_PARAMETERS)
- [Query parameters](https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/v2alpha/projects.locations.workflows/get#body.QUERY_PARAMETERS)
- [Request body](https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/v2alpha/projects.locations.workflows/get#body.request_body)
- [Response body](https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/v2alpha/projects.locations.workflows/get#body.response_body)
- [Authorization scopes](https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/v2alpha/projects.locations.workflows/get#body.aspect)
- [IAM Permissions](https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/v2alpha/projects.locations.workflows/get#body.aspect_1)
- [Try it!](https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/v2alpha/projects.locations.workflows/get#try-it)

Gets a previously created migration workflow.

### HTTP request

`GET https://bigquerymigration.googleapis.com/v2alpha/{name=projects/*/locations/*/workflows/*}`

The URL uses [gRPC Transcoding](https://google.aip.dev/127) syntax.

### Path parameters

| Parameters ||
|---|---|
| `name` | `string` Required. The unique identifier for the migration workflow. Example: `projects/123/locations/us/workflows/1234` |

### Query parameters

| Parameters ||
|---|---|
| `readMask` | ``string (`https://protobuf.dev/reference/protobuf/google.protobuf#field-mask` format)`` The list of fields to be retrieved. |

### Request body

The request body must be empty.

### Response body

If successful, the response body contains an instance of `https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/v2alpha/projects.locations.workflows#MigrationWorkflow`.

### Authorization scopes

Requires the following OAuth scope:

- `https://www.googleapis.com/auth/cloud-platform`

For more information, see the [Authentication Overview](https://docs.cloud.google.com/docs/authentication#authorization-gcp).

### IAM Permissions

Requires the following [IAM](https://cloud.google.com/iam/docs) permission on the `name` resource:

- `bigquerymigration.workflows.get`

For more information, see the [IAM documentation](https://cloud.google.com/iam/docs).