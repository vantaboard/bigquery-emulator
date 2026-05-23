# ListTransferRunsResponse

- [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/ListTransferRunsResponse#SCHEMA_REPRESENTATION)

The returned list of pipelines in the project.

| JSON representation |
|---|
| ``` { "transferRuns": [ { object (`https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs.runs#TransferRun`) } ], "nextPageToken": string } ``` |

| Fields ||
|---|---|
| `transferRuns[]` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs.runs#TransferRun`)`` Output only. The stored pipeline transfer runs. |
| `nextPageToken` | `string` Output only. The next-pagination token. For multiple-page list results, this token can be used as the `ListTransferRunsRequest.page_token` to request the next page of list results. |