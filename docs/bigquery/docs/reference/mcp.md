A [Model Context Protocol (MCP) server](https://modelcontextprotocol.io/docs/learn/server-concepts) acts as a proxy between an external service that provides context, data, or capabilities to a Large Language Model (LLM) or AI application. MCP servers connect AI applications to external systems such as databases and web services, translating their responses into a format that the AI application can understand.

### Server Setup


You must [enable MCP servers](https://docs.cloud.google.com/mcp/enable-disable-mcp-servers) and [set up authentication](https://docs.cloud.google.com/mcp/authenticate-mcp) before use. For more information about using Google and Google Cloud remote MCP servers, see [Google Cloud MCP servers overview](https://docs.cloud.google.com/mcp/overview).

BigQuery MCP server provides tools to interact with BigQuery

### Server Endpoints

An MCP service endpoint is the network address and communication interface (usually a URL) of the MCP server that an AI application (the Host for the MCP client) uses to establish a secure, standardized connection. It is the point of contact for the LLM to request context, call a tool, or access a resource. Google MCP endpoints can be global or regional.

The bigquery.googleapis.com MCP server has the following MCP endpoint:

- https://bigquery.googleapis.com/mcp

## MCP Tools

An [MCP tool](https://modelcontextprotocol.io/legacy/concepts/tools) is a function or executable capability that an MCP server exposes to a LLM or AI application to perform an action in the real world.

The bigquery.googleapis.com MCP server has the following tools:

| MCP Tools ||
|---|---|
| [list_dataset_ids](https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/list_dataset_ids) | List BigQuery dataset IDs in a Google Cloud project. |
| [get_dataset_info](https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_dataset_info) | Get metadata information about a BigQuery dataset. |
| [list_table_ids](https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/list_table_ids) | List table ids in a BigQuery dataset. |
| [get_table_info](https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_table_info) | Get metadata information about a BigQuery table. |
| [execute_sql_readonly](https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/execute_sql_readonly) | Run a read-only SQL query in the project and return the result. Prefer this tool over `execute_sql` if possible. This tool is restricted to only `SELECT` statements. `INSERT`, `UPDATE`, and `DELETE` statements and stored procedures aren't allowed. If the query doesn't include a `SELECT` statement, an error is returned. For information on creating queries, see the [GoogleSQL documentation](https://cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax). Queries executed using the `execute_sql_readonly` tool will have the job label `goog-mcp-server: true` automatically set. Queries are charged to the project specified in the `project_id` field. |
| [execute_sql](https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/execute_sql) | Run a SQL query in the project and return the result. Prefer the `execute_sql_readonly` tool if possible. This tool can execute any query that bigquery supports including: \* SQL Queries (SELECT, INSERT, UPDATE, DELETE, CREATE, etc.) \* AI/ML functions like AI.FORECAST, ML.EVALUATE, ML.PREDICT \* Any other query that bigquery supports. Queries executed using the `execute_sql` tool will have the job label `goog-mcp-server: true` automatically set. Queries are charged to the project specified in the `project_id` field. |

### Get MCP tool specifications


To get the MCP tool specifications for all tools in an MCP server, use the `tools/list` method. The following example demonstrates how to use `curl` to list all tools and their specifications currently available within the MCP server.

| Curl Request |
|---|
| ```bash curl --location 'https://bigquery.googleapis.com/mcp' \ --header 'content-type: application/json' \ --header 'accept: application/json, text/event-stream' \ --data '{ "method": "tools/list", "jsonrpc": "2.0", "id": 1 }' ``` |