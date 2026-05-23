# Method: projects.locations.workflows.start

- [HTTP request](https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/v2alpha/projects.locations.workflows/start#body.HTTP_TEMPLATE)
- [Path parameters](https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/v2alpha/projects.locations.workflows/start#body.PATH_PARAMETERS)
- [Request body](https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/v2alpha/projects.locations.workflows/start#body.request_body)
- [Response body](https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/v2alpha/projects.locations.workflows/start#body.response_body)
- [Authorization scopes](https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/v2alpha/projects.locations.workflows/start#body.aspect)
- [IAM Permissions](https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/v2alpha/projects.locations.workflows/start#body.aspect_1)
- [Try it!](https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/v2alpha/projects.locations.workflows/start#try-it)

Starts a previously created migration workflow. I.e., the state transitions from DRAFT to RUNNING. This is a no-op if the state is already RUNNING. An error will be signaled if the state is anything other than DRAFT or RUNNING.

### HTTP request

`POST https://bigquerymigration.googleapis.com/v2alpha/{name=projects/*/locations/*/workflows/*}:start`

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

- `bigquerymigration.workflows.update`

For more information, see the [IAM documentation](https://cloud.google.com/iam/docs).