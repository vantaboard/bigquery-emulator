# Method: projects.locations.workflows.list

- [HTTP request](https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/v2alpha/projects.locations.workflows/list#body.HTTP_TEMPLATE)
- [Path parameters](https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/v2alpha/projects.locations.workflows/list#body.PATH_PARAMETERS)
- [Query parameters](https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/v2alpha/projects.locations.workflows/list#body.QUERY_PARAMETERS)
- [Request body](https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/v2alpha/projects.locations.workflows/list#body.request_body)
- [Response body](https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/v2alpha/projects.locations.workflows/list#body.response_body)
  - [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/v2alpha/projects.locations.workflows/list#body.ListMigrationWorkflowsResponse.SCHEMA_REPRESENTATION)
- [Authorization scopes](https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/v2alpha/projects.locations.workflows/list#body.aspect)
- [IAM Permissions](https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/v2alpha/projects.locations.workflows/list#body.aspect_1)
- [Try it!](https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/v2alpha/projects.locations.workflows/list#try-it)

Lists previously created migration workflow.

### HTTP request

`GET https://bigquerymigration.googleapis.com/v2alpha/{parent=projects/*/locations/*}/workflows`

The URL uses [gRPC Transcoding](https://google.aip.dev/127) syntax.

### Path parameters

| Parameters ||
|---|---|
| `parent` | `string` Required. The project and location of the migration workflows to list. Example: `projects/123/locations/us` |

### Query parameters

| Parameters ||
|---|---|
| `readMask` | ``string (`https://protobuf.dev/reference/protobuf/google.protobuf#field-mask` format)`` The list of fields to be retrieved. |
| `pageSize` | `integer` The maximum number of migration workflows to return. The service may return fewer than this number. |
| `pageToken` | `string` A page token, received from previous `workflows.list` call. Provide this to retrieve the subsequent page. When paginating, all other parameters provided to `workflows.list` must match the call that provided the page token. |
| `filter` | `string` Optional. An optional AIP-160 filter to apply. The following attributes are supported: `displayName`, `state`, `task.name`, and `task.type`. |
| `orderBy` | `string` Optional. An optional AIP-132 order by field. The following attributes are supported: `displayName`, `state`, `task.name`, and `task.type`. |

### Request body

The request body must be empty.

### Response body

Response object for a `workflows.list` call.

If successful, the response body contains data with the following structure:

| JSON representation |
|---|
| ``` { "migrationWorkflows": [ { object (`https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/v2alpha/projects.locations.workflows#MigrationWorkflow`) } ], "nextPageToken": string } ``` |

| Fields ||
|---|---|
| `migrationWorkflows[]` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/v2alpha/projects.locations.workflows#MigrationWorkflow`)`` The migration workflows for the specified project / location. |
| `nextPageToken` | `string` A token, which can be sent as `pageToken` to retrieve the next page. If this field is omitted, there are no subsequent pages. |

### Authorization scopes

Requires the following OAuth scope:

- `https://www.googleapis.com/auth/cloud-platform`

For more information, see the [Authentication Overview](https://docs.cloud.google.com/docs/authentication#authorization-gcp).

### IAM Permissions

Requires the following [IAM](https://cloud.google.com/iam/docs) permission on the `parent` resource:

- `bigquerymigration.workflows.list`

For more information, see the [IAM documentation](https://cloud.google.com/iam/docs).