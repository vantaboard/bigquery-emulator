# MCP Reference: bigquerymigration.googleapis.com

A [Model Context Protocol (MCP) server](https://modelcontextprotocol.io/docs/learn/server-concepts) acts as a proxy between an external service that provides context, data, or capabilities to a Large Language Model (LLM) or AI application. MCP servers connect AI applications to external systems such as databases and web services, translating their responses into a format that the AI application can understand.

### Server Setup


You must [enable MCP servers](https://docs.cloud.google.com/mcp/enable-disable-mcp-servers) and [set up authentication](https://docs.cloud.google.com/mcp/authenticate-mcp) before use. For more information about using Google and Google Cloud remote MCP servers, see [Google Cloud MCP servers overview](https://docs.cloud.google.com/mcp/overview).

BigQuery Migration MCP server provides tools to work with BigQuery Migration Services, such as [SQL translation](https://docs.cloud.google.com/bigquery/docs/api-sql-translator).

### Server Endpoints

An MCP service endpoint is the network address and communication interface (usually a URL) of the MCP server that an AI application (the Host for the MCP client) uses to establish a secure, standardized connection. It is the point of contact for the LLM to request context, call a tool, or access a resource. Google MCP endpoints can be global or regional.

The bigquerymigration.googleapis.com MCP server has the following MCP endpoint:

- https://bigquerymigration.googleapis.com/mcp

## MCP Tools

An [MCP tool](https://modelcontextprotocol.io/legacy/concepts/tools) is a function or executable capability that an MCP server exposes to a LLM or AI application to perform an action in the real world.

The bigquerymigration.googleapis.com MCP server has the following tools:

| MCP Tools ||
|---|---|
| [translate_query](https://docs.cloud.google.com/bigquery/docs/reference/migration/mcp/tools_list/translate_query) | Translates a single query into BigQuery SQL syntax. |
| [get_translation](https://docs.cloud.google.com/bigquery/docs/reference/migration/mcp/tools_list/get_translation) | Gets the SQL translation for a given translation ID. |
| [explain_translation](https://docs.cloud.google.com/bigquery/docs/reference/migration/mcp/tools_list/explain_translation) | Explains the SQL translation for a given translation ID. |
| [generate_ddl_suggestion](https://docs.cloud.google.com/bigquery/docs/reference/migration/mcp/tools_list/generate_ddl_suggestion) | Generates DDL statement suggestions for a given input query. |
| [fetch_ddl_suggestion](https://docs.cloud.google.com/bigquery/docs/reference/migration/mcp/tools_list/fetch_ddl_suggestion) | Fetches DDL suggestion for a given suggestion ID. |

### Get MCP tool specifications


To get the MCP tool specifications for all tools in an MCP server, use the `tools/list` method. The following example demonstrates how to use `curl` to list all tools and their specifications currently available within the MCP server.

| Curl Request |
|---|
| ```bash curl --location 'https://bigquerymigration.googleapis.com/mcp' \ --header 'content-type: application/json' \ --header 'accept: application/json, text/event-stream' \ --data '{ "method": "tools/list", "jsonrpc": "2.0", "id": 1 }' ``` |