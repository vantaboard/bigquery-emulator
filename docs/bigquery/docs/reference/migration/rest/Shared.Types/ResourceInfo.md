# ResourceInfo

- [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/Shared.Types/ResourceInfo#SCHEMA_REPRESENTATION)

Describes the resource that is being accessed.

| JSON representation |
|---|
| ``` { "resourceType": string, "resourceName": string, "owner": string, "description": string } ``` |

| Fields ||
|---|---|
| `resourceType` | `string` A name for the type of resource being accessed, e.g. "sql table", "cloud storage bucket", "file", "Google calendar"; or the type URL of the resource: e.g. "type.googleapis.com/google.pubsub.v1.Topic". |
| `resourceName` | `string` The name of the resource being accessed. For example, a shared calendar name: "example.com[_4fghdhgsrgh@group.calendar.google.com"](mailto:_4fghdhgsrgh@group.calendar.google.com"), if the current error is `google.rpc.Code.PERMISSION_DENIED`. |
| `owner` | `string` The owner of the resource (optional). For example, "user:" or "project:". |
| `description` | `string` Describes what error is encountered when accessing this resource. For example, updating a cloud project may require the `writer` permission on the developer console project. |