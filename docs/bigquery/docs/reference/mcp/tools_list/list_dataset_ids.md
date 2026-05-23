## Tool: `list_dataset_ids`

List BigQuery dataset IDs in a Google Cloud project.

The following sample demonstrate how to use `curl` to invoke the `list_dataset_ids` MCP tool.

| Curl Request |
|---|
| ```bash curl --location 'https://bigquery.googleapis.com/mcp' \ --header 'content-type: application/json' \ --header 'accept: application/json, text/event-stream' \ --data '{ "method": "tools/call", "params": { "name": "list_dataset_ids", "arguments": { // provide these details according to the tool's MCP specification } }, "jsonrpc": "2.0", "id": 1 }' ``` |

## Input Schema

Request for a list of datasets in a project.

### ListDatasetsRequest

| JSON representation |
|---|
| ``` { "projectId": string } ``` |

| Fields ||
|---|---|
| `projectId` | `string` Required. Project ID of the dataset request. |

## Output Schema

Response for a list of datasets.

### ListDatasetsResponse

| JSON representation |
|---|
| ``` { "datasets": [ { object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/list_dataset_ids#Output.Schema.ListFormatDataset`) } ] } ``` |

| Fields ||
|---|---|
| `datasets[]` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/list_dataset_ids#Output.Schema.ListFormatDataset`)`` The datasets that matched the request. |

### ListFormatDataset

| JSON representation |
|---|
| ``` { "id": string, "friendlyName": string, "location": string } ``` |

| Fields ||
|---|---|
| `id` | `string` The ID of the dataset. |
| `friendlyName` | `string` An alternate name for the dataset. The friendly name is purely decorative in nature. This can be useful to derive additional information about the dataset. |
| `location` | `string` The geographic location where the dataset resides. |

### StringValue

| JSON representation |
|---|
| ``` { "value": string } ``` |

| Fields ||
|---|---|
| `value` | `string` The string value. |

### Tool Annotations

Destructive Hint: ❌ \| Idempotent Hint: ✅ \| Read Only Hint: ✅ \| Open World Hint: ❌