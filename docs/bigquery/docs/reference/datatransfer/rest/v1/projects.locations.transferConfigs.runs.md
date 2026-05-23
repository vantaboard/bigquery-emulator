# REST Resource: projects.locations.transferConfigs.runs.transferLogs

- [Resource: TransferMessage](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs.runs.transferLogs#TransferMessage)
  - [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs.runs.transferLogs#TransferMessage.SCHEMA_REPRESENTATION)
  - [MessageSeverity](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs.runs.transferLogs#TransferMessage.MessageSeverity)
- [Methods](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs.runs.transferLogs#METHODS_SUMMARY)

## Resource: TransferMessage

Represents a user facing message for a particular data transfer run.

| JSON representation |
|---|
| ``` { "messageTime": string, "severity": enum (`https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs.runs.transferLogs#TransferMessage.MessageSeverity`), "messageText": string } ``` |

| Fields ||
|---|---|
| `messageTime` | ``string (`https://protobuf.dev/reference/protobuf/google.protobuf#timestamp` format)`` Time when message was logged. |
| `severity` | ``enum (`https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs.runs.transferLogs#TransferMessage.MessageSeverity`)`` Message severity. |
| `messageText` | `string` Message text. |

### MessageSeverity

Represents data transfer user facing message severity.

| Enums ||
|---|---|
| `MESSAGE_SEVERITY_UNSPECIFIED` | No severity specified. |
| `INFO` | Informational message. |
| `WARNING` | Warning message. |
| `ERROR` | Error message. |

| ## Methods ||
|---|---|
| ### `https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs.runs.transferLogs/list` | Returns log messages for the transfer run. |