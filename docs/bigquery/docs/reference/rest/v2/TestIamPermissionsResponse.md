# TestIamPermissionsResponse

- [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/TestIamPermissionsResponse#SCHEMA_REPRESENTATION)

Response message for `tables.testIamPermissions` method.

| JSON representation |
|---|
| ``` { "permissions": [ string ] } ``` |

| Fields ||
|---|---|
| `permissions[]` | `string` A subset of `TestPermissionsRequest.permissions` that the caller is allowed. |