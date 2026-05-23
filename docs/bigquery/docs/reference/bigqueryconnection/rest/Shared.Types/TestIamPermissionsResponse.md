# TestIamPermissionsResponse

- [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/bigqueryconnection/rest/Shared.Types/TestIamPermissionsResponse#SCHEMA_REPRESENTATION)

Response message for `TestIamPermissions` method.

| JSON representation |
|---|
| ``` { "permissions": [ string ] } ``` |

| Fields ||
|---|---|
| `permissions[]` | `string` A subset of `TestPermissionsRequest.permissions` that the caller is allowed. |