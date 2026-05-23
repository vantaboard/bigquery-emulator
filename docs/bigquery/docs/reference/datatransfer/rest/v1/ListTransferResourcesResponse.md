# ListTransferResourcesResponse

- [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/ListTransferResourcesResponse#SCHEMA_REPRESENTATION)

Response for the `transferResources.list` RPC.

| JSON representation |
|---|
| ``` { "transferResources": [ { object (`https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs.transferResources#TransferResource`) } ], "nextPageToken": string } ``` |

| Fields ||
|---|---|
| `transferResources[]` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs.transferResources#TransferResource`)`` Output only. The transfer resources. |
| `nextPageToken` | `string` Output only. A token, which can be sent as `pageToken` to retrieve the next page. If this field is omitted, there are no subsequent pages. |