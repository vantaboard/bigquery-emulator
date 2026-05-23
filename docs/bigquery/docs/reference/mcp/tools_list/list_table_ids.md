## Tool: `list_table_ids`

List table ids in a BigQuery dataset.

The following sample demonstrate how to use `curl` to invoke the `list_table_ids` MCP tool.

| Curl Request |
|---|
| ```bash curl --location 'https://bigquery.googleapis.com/mcp' \ --header 'content-type: application/json' \ --header 'accept: application/json, text/event-stream' \ --data '{ "method": "tools/call", "params": { "name": "list_table_ids", "arguments": { // provide these details according to the tool's MCP specification } }, "jsonrpc": "2.0", "id": 1 }' ``` |

## Input Schema

Request for a list of tables in a dataset.

### ListTablesRequest

| JSON representation |
|---|
| ``` { "projectId": string, "datasetId": string } ``` |

| Fields ||
|---|---|
| `projectId` | `string` Required. Project ID of the table request. |
| `datasetId` | `string` Required. Dataset ID of the table request. |

## Output Schema

Response for a list of tables.

### ListTablesResponse

| JSON representation |
|---|
| ``` { "tables": [ { object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/list_table_ids#Output.Schema.ListFormatTable`) } ] } ``` |

| Fields ||
|---|---|
| `tables[]` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/list_table_ids#Output.Schema.ListFormatTable`)`` The tables that matched the request. |

### ListFormatTable

| JSON representation |
|---|
| ``` { "id": string } ``` |

| Fields ||
|---|---|
| `id` | `string` The ID of the table. |

### Tool Annotations

Destructive Hint: ❌ \| Idempotent Hint: ✅ \| Read Only Hint: ✅ \| Open World Hint: ❌