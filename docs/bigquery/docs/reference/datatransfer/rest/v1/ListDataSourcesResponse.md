# ListDataSourcesResponse

- [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/ListDataSourcesResponse#SCHEMA_REPRESENTATION)

Returns list of supported data sources and their metadata.

| JSON representation |
|---|
| ``` { "dataSources": [ { object (`https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.dataSources#DataSource`) } ], "nextPageToken": string } ``` |

| Fields ||
|---|---|
| `dataSources[]` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.dataSources#DataSource`)`` List of supported data sources and their transfer settings. |
| `nextPageToken` | `string` Output only. The next-pagination token. For multiple-page list results, this token can be used as the `ListDataSourcesRequest.page_token` to request the next page of list results. |