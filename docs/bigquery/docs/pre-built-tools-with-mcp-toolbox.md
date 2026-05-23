# Connect LLMs to BigQuery with MCP

This guide shows you how to use the [MCP Toolbox for Databases](https://github.com/googleapis/mcp-toolbox) to
connect your BigQuery project to a variety of Integrated Development
Environments (IDEs) and developer tools. It uses the [Model Context Protocol
(MCP)](https://modelcontextprotocol.io/introduction), an open protocol for
connecting large language models (LLMs) to data sources like
BigQuery, allowing you to run SQL queries and interact with your
project directly from your existing tools.

If you use the Gemini CLI, you can use BigQuery
extensions. To learn how, see [Develop with Gemini
CLI](https://docs.cloud.google.com/bigquery/docs/develop-with-gemini-cli). If you plan to build custom tools
for the Gemini CLI, continue reading.

This guide demonstrates the connection process for the following IDEs:

- Cursor
- Windsurf (formerly Codeium)
- Visual Studio Code (Copilot)
- Cline (VS Code extension)
- Claude desktop
- Claude code
- Antigravity

## Before you begin

1. In the Google Cloud console, on the [project selector page](https://console.cloud.google.com/projectselector2/home/dashboard), select or create a Google Cloud project.

2. [Make sure that billing is enabled for your Google Cloud project](https://docs.cloud.google.com/billing/docs/how-to/verify-billing-enabled#confirm_billing_is_enabled_on_a_project).

3. [Enable the BigQuery API in the Google Cloud project](https://console.cloud.google.com/flows/enableapi?apiid=bigquery.googleapis.com&redirect=https://console.cloud.google.com/).

4. Configure the required roles and permissions to complete this task. You will need the [BigQuery User](https://docs.cloud.google.com/bigquery/docs/access-control) role (`roles/bigquery.user`), the BigQuery Data Viewer role (`roles/bigquery.dataViewer`), or equivalent IAM permissions to connect to the project.

5. Configure [Application Default Credentials (ADC)](https://docs.cloud.google.com/docs/authentication/set-up-adc-local-dev-environment) for your environment.

## Connect with Antigravity

You can connect BigQuery to Antigravity in the following ways:

- Using the MCP Store
- Using a custom configuration

**Note:** You don't need to download the MCP Toolbox binary to use these methods.

### MCP Store

The most straightforward way to connect to BigQuery in Antigravity is by using the built-in MCP Store.

1. Open [Antigravity](https://antigravity.google/docs/mcp) and open the **editor's agent panel**.
2. Click the **"..."** icon at the top of the panel and select **MCP Servers**.
3. Locate **BigQuery** in the list of available servers and click **Install**.
4. Follow the on-screen prompts to securely link your accounts where applicable.

After you install BigQuery in the MCP Store, resources and tools from the server are automatically available to the editor.

### Custom config

To connect to a custom MCP server, follow these steps:

1. Open [Antigravity](https://antigravity.google/docs/mcp) and navigate to the MCP store using the **"..."** drop-down at the top of the editor's agent panel.
2. To open the **mcp_config.json** file, click **MCP Servers** and then click **Manage MCP Servers \> View raw config**.
3. Add the following configuration, replace the environment variable with your values, and save.

```
{
  "mcpServers": {
    "bigquery": {
      "command": "npx",
      "args": ["-y","@toolbox-sdk/server","--prebuilt","bigquery","--stdio"],
      "env": {
          "BIGQUERY_PROJECT": "PROJECT_ID"
      }
    }
  }
}
```

## Install the MCP Toolbox

You don't need to install MCP Toolbox if you only plan to use the
BigQuery Gemini CLI extensions, as they bundle the
required server capabilities. For other IDEs and tools, follow the steps in this
section to install MCP Toolbox.

The toolbox acts as an open-source
[Model Context Protocol (MCP)](https://modelcontextprotocol.io/introduction)
server that sits between your IDE and BigQuery, providing a
secure and efficient control plane for your AI tools.

1. Download the latest version of the MCP Toolbox as a binary. Select the
   [binary](https://github.com/googleapis/mcp-toolbox/releases) corresponding
   to your operating system (OS) and CPU architecture. You must use MCP Toolbox
   version V0.7.0 or later:

   ### linux/amd64

   ```
   curl -O https://storage.googleapis.com/mcp-toolbox-for-databases/VERSION/linux/amd64/toolbox
   ```

   Replace `VERSION` with the MCP Toolbox
   version---for example `v0.7.0`.

   ### macOS darwin/arm64

   ```
   curl -O https://storage.googleapis.com/mcp-toolbox-for-databases/VERSION/darwin/arm64/toolbox
   ```

   Replace `VERSION` with the MCP Toolbox
   version---for example `v0.7.0`.

   ### macOS darwin/amd64

   ```
   curl -O https://storage.googleapis.com/mcp-toolbox-for-databases/VERSION/darwin/amd64/toolbox
   ```

   Replace `VERSION` with the MCP Toolbox
   version---for example `v0.7.0`.

   ### windows/amd64

   ```
   curl -O https://storage.googleapis.com/mcp-toolbox-for-databases/VERSION/windows/amd64/toolbox
   ```

   Replace `VERSION` with the MCP Toolbox
   version---for example `v0.7.0`.
2. Make the binary executable:

       chmod +x toolbox

3. Verify the installation:

       ./toolbox --version

## Set up clients and connections

This section explains how to connect BigQuery to your tools.

If you are using the standalone Gemini CLI, you don't need to
install or configure MCP Toolbox, as the extensions bundle the required server
capabilities.

For other MCP-compatible tools and IDEs, you must first [install MCP
Toolbox](https://docs.cloud.google.com/bigquery/docs/pre-built-tools-with-mcp-toolbox#install).

### Claude code

1. Install [Claude Code](https://docs.anthropic.com/en/docs/agents-and-tools/claude-code/overview).
2. Create a `.mcp.json` file in your project root, if it doesn't exist.
3. Add the configuration, replace the environment variables with your values, and save:

   ```
           {
             "mcpServers": {
               "bigquery": {
                 "command": "./PATH/TO/toolbox",
                 "args": ["--prebuilt","bigquery","--stdio"],
                 "env": {
                   "BIGQUERY_PROJECT": "PROJECT_ID"
                 }
               }
             }
           }
           
   ```
4. Restart Claude Code to load the new settings. When it reopens, the tool provides an indication that the configured MCP server has been detected.

### Claude desktop

1. Open [Claude Desktop](https://claude.ai/download) and navigate to **Settings**.
2. In the **Developer** tab, click **Edit Config** to open the configuration file.
3. Add the configuration, replace the environment variables with your values, and save:

   ```
           {
             "mcpServers": {
               "bigquery": {
                 "command": "./PATH/TO/toolbox",
                 "args": ["--prebuilt","bigquery","--stdio"],
                 "env": {
                   "BIGQUERY_PROJECT": "PROJECT_ID"
                 }
               }
             }
           }
           
   ```
4. Restart Claude Desktop.
5. The new chat screen displays a hammer (MCP) icon with the new MCP server.

### Cline

1. Open the [Cline](https://github.com/cline/cline) extension in VS Code and tap the **MCP Servers** icon.
2. Tap **Configure MCP Servers** to open the configuration file.
3. Add the following configuration, replace the environment variables with your values, and save:

   ```
           {
             "mcpServers": {
               "bigquery": {
                 "command": "./PATH/TO/toolbox",
                 "args": ["--prebuilt","bigquery","--stdio"],
                 "env": {
                   "BIGQUERY_PROJECT": "PROJECT_ID"
                 }
               }
             }
           }
           
   ```


A green active status appears after the server connects successfully.

### Cursor

1. Create the `.cursor` directory in your project root if it doesn't exist.
2. Create the `.cursor/mcp.json` file if it doesn't exist and open it.
3. Add the following configuration, replace the environment variables with your values, and save:

   ```
           {
             "mcpServers": {
               "bigquery": {
                 "command": "./PATH/TO/toolbox",
                 "args": ["--prebuilt","bigquery","--stdio"],
                 "env": {
                   "BIGQUERY_PROJECT": "PROJECT_ID"
                 }
               }
             }
           }
           
   ```
4. Open [Cursor](https://www.cursor.com/) and navigate to **Settings \> Cursor Settings \> MCP**. A green active status appears when the server connects.

### Visual Studio Code (Copilot)

1. Open [VS Code](https://code.visualstudio.com/docs/copilot/overview) and create a `.vscode` directory in your project root if it does not exist.
2. Create the `.vscode/mcp.json` file if it doesn't exist, and open it.
3. Add the following configuration, replace the environment variables with your values, and save:

   ```
           {
             "servers": {
               "bigquery": {
                 "command": "./PATH/TO/toolbox",
                 "args": ["--prebuilt","bigquery","--stdio"],
                 "env": {
                   "BIGQUERY_PROJECT": "PROJECT_ID"
                 }
               }
             }
           }
           
   ```
4. Reload the VS Code window. The MCP-compatible extension automatically detects the configuration and starts the server.

### Windsurf

1. Open [Windsurf](https://docs.codeium.com/windsurf) and navigate to the Cascade assistant.
2. Click the MCP icon, then click **Configure** to open the configuration file.
3. Add the following configuration, replace the environment variables with your values, and save:

   ```
           {
             "mcpServers": {
               "bigquery": {
                 "command": "./PATH/TO/toolbox",
                 "args": ["--prebuilt","bigquery","--stdio"],
                 "env": {
                   "BIGQUERY_PROJECT": "PROJECT_ID"
                 }
               }
             }
           }
           
   ```
> **Note:** The `BIGQUERY_PROJECT` environment variable specifies the default Google Cloud Project ID for the MCP Toolbox to use. All BigQuery operations, such as executing queries, are run within this project.

## Use the tools

Your AI tool is now connected to BigQuery using MCP. Try asking your AI assistant to list tables, create a table, or define and execute other SQL statements.

The following tools are available to the LLM:

- **analyze_contribution**: perform contribution analysis, also called key driver analysis.
- **ask_data_insights**: perform data analysis, get insights, or answer complex questions about the contents of BigQuery tables.
- **execute_sql**: execute SQL statement.
- **forecast**: forecast time series data.
- **get_dataset_info**: get dataset metadata.
- **get_table_info**: get table metadata.
- **list_dataset_ids**: list datasets.
- **list_table_ids**: list tables.
- **search_catalog**: search for a table using natural language.