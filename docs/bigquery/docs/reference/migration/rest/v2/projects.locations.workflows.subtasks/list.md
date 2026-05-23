# Method: projects.locations.workflows.subtasks.list

- [HTTP request](https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/v2/projects.locations.workflows.subtasks/list#body.HTTP_TEMPLATE)
- [Path parameters](https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/v2/projects.locations.workflows.subtasks/list#body.PATH_PARAMETERS)
- [Query parameters](https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/v2/projects.locations.workflows.subtasks/list#body.QUERY_PARAMETERS)
- [Request body](https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/v2/projects.locations.workflows.subtasks/list#body.request_body)
- [Response body](https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/v2/projects.locations.workflows.subtasks/list#body.response_body)
  - [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/v2/projects.locations.workflows.subtasks/list#body.ListMigrationSubtasksResponse.SCHEMA_REPRESENTATION)
- [Authorization scopes](https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/v2/projects.locations.workflows.subtasks/list#body.aspect)
- [IAM Permissions](https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/v2/projects.locations.workflows.subtasks/list#body.aspect_1)
- [Try it!](https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/v2/projects.locations.workflows.subtasks/list#try-it)

Lists previously created migration subtasks.

### HTTP request

`GET https://bigquerymigration.googleapis.com/v2/{parent=projects/*/locations/*/workflows/*}/subtasks`

The URL uses [gRPC Transcoding](https://google.aip.dev/127) syntax.

### Path parameters

| Parameters ||
|---|---|
| `parent` | `string` Required. The migration task of the subtasks to list. Example: `projects/123/locations/us/workflows/1234` |

### Query parameters

| Parameters ||
|---|---|
| `readMask` | ``string (`https://protobuf.dev/reference/protobuf/google.protobuf#field-mask` format)`` Optional. The list of fields to be retrieved. |
| `pageSize` | `integer` Optional. The maximum number of migration tasks to return. The service may return fewer than this number. |
| `pageToken` | `string` Optional. A page token, received from previous `subtasks.list` call. Provide this to retrieve the subsequent page. When paginating, all other parameters provided to `subtasks.list` must match the call that provided the page token. |
| `filter` | `string` Optional. The filter to apply. This can be used to get the subtasks of a specific tasks in a workflow, e.g. `migrationTask = "ab012"` where `"ab012"` is the task ID (not the name in the named map). |

### Request body

The request body must be empty.

### Response body

Response object for a `subtasks.list` call.

If successful, the response body contains data with the following structure:

| JSON representation |
|---|
| ``` { "migrationSubtasks": [ { object (`https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/v2/projects.locations.workflows.subtasks#MigrationSubtask`) } ], "nextPageToken": string } ``` |

| Fields ||
|---|---|
| `migrationSubtasks[]` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/v2/projects.locations.workflows.subtasks#MigrationSubtask`)`` The migration subtasks for the specified task. |
| `nextPageToken` | `string` A token, which can be sent as `pageToken` to retrieve the next page. If this field is omitted, there are no subsequent pages. |

### Authorization scopes

Requires the following OAuth scope:

- `https://www.googleapis.com/auth/cloud-platform`

For more information, see the [Authentication Overview](https://docs.cloud.google.com/docs/authentication#authorization-gcp).

### IAM Permissions

Requires the following [IAM](https://cloud.google.com/iam/docs) permission on the `parent` resource:

- `bigquerymigration.subtasks.list`

For more information, see the [IAM documentation](https://cloud.google.com/iam/docs).