# Operation

- [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/Operation#SCHEMA_REPRESENTATION)
- [Status](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/Operation#Status)
  - [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/Operation#Status.SCHEMA_REPRESENTATION)

This resource represents a long-running operation that is the result of a network API call.

| JSON representation |
|---|
| ``` { "name": string, "metadata": { "@type": string, field1: ..., ... }, "done": boolean, // Union field `result` can be only one of the following: "error": { object (`https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/Operation#Status`) }, "response": { "@type": string, field1: ..., ... } // End of list of possible types for union field `result`. } ``` |

| Fields ||
|---|---|
| `name` | `string` The server-assigned name, which is only unique within the same service that originally returns it. If you use the default HTTP mapping, the `name` should be a resource name ending with `operations/{uniqueId}`. |
| `metadata` | `object` Service-specific metadata associated with the operation. It typically contains progress information and common metadata such as create time. Some services might not provide such metadata. Any method that returns a long-running operation should document the metadata type, if any. An object containing fields of an arbitrary type. An additional field `"@type"` contains a URI identifying the type. Example: `{ "id": 1234, "@type": "types.example.com/standard/id" }`. |
| `done` | `boolean` If the value is `false`, it means the operation is still in progress. If `true`, the operation is completed, and either `error` or `response` is available. |
| Union field `result`. The operation result, which can be either an `error` or a valid `response`. If `done` == `false`, neither `error` nor `response` is set. If `done` == `true`, exactly one of `error` or `response` can be set. Some services might not provide the result. `result` can be only one of the following: ||
| `error` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/Operation#Status`)`` The error result of the operation in case of failure or cancellation. |
| `response` | `object` The normal, successful response of the operation. If the original method returns no data on success, such as `Delete`, the response is `google.protobuf.Empty`. If the original method is standard `Get`/`Create`/`Update`, the response should be the resource. For other methods, the response should have the type `XxxResponse`, where `Xxx` is the original method name. For example, if the original method name is `TakeSnapshot()`, the inferred response type is `TakeSnapshotResponse`. An object containing fields of an arbitrary type. An additional field `"@type"` contains a URI identifying the type. Example: `{ "id": 1234, "@type": "types.example.com/standard/id" }`. |

## Status

The `Status` type defines a logical error model that is suitable for different programming environments, including REST APIs and RPC APIs. It is used by [gRPC](https://github.com/grpc). Each `Status` message contains three pieces of data: error code, error message, and error details.

You can find out more about this error model and how to work with it in the [API Design Guide](https://cloud.google.com/apis/design/errors).

| JSON representation |
|---|
| ``` { "code": integer, "message": string, "details": [ { "@type": string, field1: ..., ... } ] } ``` |

| Fields ||
|---|---|
| `code` | `integer` The status code, which should be an enum value of `google.rpc.Code`. |
| `message` | `string` A developer-facing error message, which should be in English. Any user-facing error message should be localized and sent in the `https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/Operation#Status.FIELDS.details` field, or localized by the client. |
| `details[]` | `object` A list of messages that carry the error details. There is a common set of message types for APIs to use. An object containing fields of an arbitrary type. An additional field `"@type"` contains a URI identifying the type. Example: `{ "id": 1234, "@type": "types.example.com/standard/id" }`. |