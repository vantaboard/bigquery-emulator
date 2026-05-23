# ScheduleTransferRunsResponse

- [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/ScheduleTransferRunsResponse#SCHEMA_REPRESENTATION)

A response to schedule transfer runs for a time range.

| JSON representation |
|---|
| ``` { "runs": [ { object (`https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs.runs#TransferRun`) } ] } ``` |

| Fields ||
|---|---|
| `runs[]` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs.runs#TransferRun`)`` The transfer runs that were scheduled. |