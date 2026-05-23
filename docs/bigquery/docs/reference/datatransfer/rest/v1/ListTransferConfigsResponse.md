# ListTransferConfigsResponse

- [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/ListTransferConfigsResponse#SCHEMA_REPRESENTATION)

The returned list of pipelines in the project.

| JSON representation |
|---|
| ``` { "transferConfigs": [ { object (`https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs#TransferConfig`) } ], "nextPageToken": string } ``` |

| Fields ||
|---|---|
| `transferConfigs[]` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs#TransferConfig`)`` Output only. The stored pipeline transfer configurations. |
| `nextPageToken` | `string` Output only. The next-pagination token. For multiple-page list results, this token can be used as the `ListTransferConfigsRequest.page_token` to request the next page of list results. |