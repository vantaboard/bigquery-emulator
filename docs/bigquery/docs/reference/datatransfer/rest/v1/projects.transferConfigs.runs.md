# REST Resource: projects.transferConfigs.runs.transferLogs

- [Resource: TransferMessage](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.transferConfigs.runs.transferLogs#TransferMessage)
  - [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.transferConfigs.runs.transferLogs#TransferMessage.SCHEMA_REPRESENTATION)
- [Methods](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.transferConfigs.runs.transferLogs#METHODS_SUMMARY)

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

| ## Methods ||
|---|---|
| ### `https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.transferConfigs.runs.transferLogs/list` | Returns log messages for the transfer run. |