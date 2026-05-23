# StartManualTransferRunsResponse

- [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/StartManualTransferRunsResponse#SCHEMA_REPRESENTATION)

A response to start manual transfer runs.

| JSON representation |
|---|
| ``` { "runs": [ { object (`https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs.runs#TransferRun`) } ] } ``` |

| Fields ||
|---|---|
| `runs[]` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs.runs#TransferRun`)`` The transfer runs that were created. |