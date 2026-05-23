# Status

- [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/auditlogs/rest/Shared.Types/Status#SCHEMA_REPRESENTATION)

The `Status` type defines a logical error model that is suitable for different programming environments, including REST APIs and RPC APIs. It is used by [gRPC](https://github.com/grpc). Each `Status` message contains three pieces of data: error code, error message, and error details.

You can find out more about this error model and how to work with it in the [API Design Guide](https://cloud.google.com/apis/design/errors).

| JSON representation |
|---|
| ``` { "code": integer, "message": string, "details": [ { "@type": string, field1: ..., ... } ] } ``` |

| Fields ||
|---|---|
| `code` | `integer` The status code, which should be an enum value of `google.rpc.Code`. |
| `message` | `string` A developer-facing error message, which should be in English. Any user-facing error message should be localized and sent in the `https://docs.cloud.google.com/bigquery/docs/reference/auditlogs/rest/Shared.Types/Status#FIELDS.details` field, or localized by the client. |
| `details[]` | `object` A list of messages that carry the error details. There is a common set of message types for APIs to use. An object containing fields of an arbitrary type. An additional field `"@type"` contains a URI identifying the type. Example: `{ "id": 1234, "@type": "types.example.com/standard/id" }`. |