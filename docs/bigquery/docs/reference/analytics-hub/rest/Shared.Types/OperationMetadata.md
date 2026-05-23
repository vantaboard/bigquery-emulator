# OperationMetadata

- [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/Shared.Types/OperationMetadata#SCHEMA_REPRESENTATION)

Represents the metadata of a long-running operation in Analytics Hub.

| JSON representation |
|---|
| ``` { "createTime": string, "endTime": string, "target": string, "verb": string, "statusMessage": string, "requestedCancellation": boolean, "apiVersion": string } ``` |

| Fields ||
|---|---|
| `createTime` | ``string (`https://protobuf.dev/reference/protobuf/google.protobuf#timestamp` format)`` Output only. The time the operation was created. Uses RFC 3339, where generated output will always be Z-normalized and use 0, 3, 6 or 9 fractional digits. Offsets other than "Z" are also accepted. Examples: `"2014-10-02T15:01:23Z"`, `"2014-10-02T15:01:23.045123456Z"` or `"2014-10-02T15:01:23+05:30"`. |
| `endTime` | ``string (`https://protobuf.dev/reference/protobuf/google.protobuf#timestamp` format)`` Output only. The time the operation finished running. Uses RFC 3339, where generated output will always be Z-normalized and use 0, 3, 6 or 9 fractional digits. Offsets other than "Z" are also accepted. Examples: `"2014-10-02T15:01:23Z"`, `"2014-10-02T15:01:23.045123456Z"` or `"2014-10-02T15:01:23+05:30"`. |
| `target` | `string` Output only. Server-defined resource path for the target of the operation. |
| `verb` | `string` Output only. Name of the verb executed by the operation. |
| `statusMessage` | `string` Output only. Human-readable status of the operation, if any. |
| `requestedCancellation` | `boolean` Output only. Identifies whether the user has requested cancellation of the operation. Operations that have successfully been cancelled have \[Operation.error\]\[\] value with a `https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/Operation#Status.FIELDS.code` of 1, corresponding to `Code.CANCELLED`. |
| `apiVersion` | `string` Output only. API version used to start the operation. |