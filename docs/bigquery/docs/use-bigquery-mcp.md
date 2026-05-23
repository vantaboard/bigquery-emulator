# Use the BigQuery MCP server

<br />

This document shows you how to use the BigQuery remote Model Context Protocol (MCP) server to connect with AI applications including Gemini CLI, ChatGPT, Claude, and custom applications you are developing. You can use the BigQuery remote MCP server to perform tasks such as running queries, getting metadata, and listing resources. The BigQuery remote MCP server is enabled when you enable the BigQuery API.

[Model Context Protocol](https://modelcontextprotocol.io/docs/getting-started/intro)
(MCP) standardizes how large language models (LLMs) and AI applications or
agents connect to external data sources. MCP servers let you use their tools,
resources, and prompts to take actions and get updated data from their backend
service.

## What's the difference between local and remote MCP servers?

Local MCP servers
:   Typically run on your local machine and use the standard input
    and output streams (stdio) for communication between services on the same
    device.

Remote MCP servers
:   Run on the service's infrastructure and offer an HTTP
    endpoint to AI applications for communication between the AI MCP client and
    the MCP server. For more information about MCP architecture, see
    [MCP architecture](https://modelcontextprotocol.io/docs/learn/architecture).

You might use the BigQuery [local MCP server](https://docs.cloud.google.com/bigquery/docs/pre-built-tools-with-mcp-toolbox)
for the following reasons:

- You need to build a custom tool over a parameterized SQL query.
- You don't have permissions to enable or use the MCP server in your project.

For more information about how to use our local MCP server, see
[Connect LLMs to BigQuery with MCP](https://docs.cloud.google.com/bigquery/docs/pre-built-tools-with-mcp-toolbox). The
following sections apply only to the BigQuery MCP server.

## Google and Google Cloud remote MCP servers

Google and Google Cloud remote MCP servers have the following features and benefits:

<br />

- Simplified, centralized discovery
- Managed global or regional HTTP endpoints
- Fine-grained authorization
- Optional prompt and response security with Model Armor protection
- Centralized audit logging

For information about other MCP servers and information about security
and governance controls available for Google Cloud MCP servers,
see [Google Cloud MCP servers overview](https://docs.cloud.google.com/mcp/overview).

## Before you begin

1.


   Enable the BigQuery API.


   **Roles required to enable APIs**


   To enable APIs, you need the Service Usage Admin IAM
   role (`roles/serviceusage.serviceUsageAdmin`), which
   contains the `serviceusage.services.enable` permission. [Learn how to grant
   roles](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).

   [Enable the API](https://console.cloud.google.com/flows/enableapi?apiid=bigquery.googleapis.com)

   For new projects, the BigQuery API is
   automatically enabled.
2. Optional: [Enable
   billing](https://docs.cloud.google.com/billing/docs/how-to/modify-project) for the project. If you don't want to enable billing or provide a credit card, the steps in this document still work. BigQuery provides you a sandbox to perform the steps. For more information, see [Enable the BigQuery sandbox](https://docs.cloud.google.com/bigquery/docs/sandbox#setup).

   > [!NOTE]
   > **Note:** If your project has a billing account and you want to use the BigQuery sandbox, then [disable billing for your project](https://docs.cloud.google.com/billing/docs/how-to/modify-project#disable_billing_for_a_project).

### Required roles


To get the permissions that
you need to use the BigQuery MCP server,

ask your administrator to grant you the
following IAM roles on the project where you want to use the BigQuery MCP server:

- Make MCP tool calls: [MCP Tool User](https://docs.cloud.google.com/iam/docs/roles-permissions/mcp#mcp.toolUser) (`roles/mcp.toolUser`)
- Run BigQuery jobs: [BigQuery Job User](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquery#bigquery.jobUser) (`roles/bigquery.jobUser`)
- Query BigQuery data: [BigQuery Data Viewer](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquery#bigquery.dataViewer) (`roles/bigquery.dataViewer`)


For more information about granting roles, see [Manage access to projects, folders, and organizations](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).


These predefined roles contain

the permissions required to use the BigQuery MCP server. To see the exact permissions that are
required, expand the **Required permissions** section:


#### Required permissions

The following permissions are required to use the BigQuery MCP server:

- Make MCP tool calls: `mcp.tools.call`
- Run BigQuery jobs: `bigquery.jobs.create`
- Query BigQuery data: `bigquery.tables.getData`


You might also be able to get
these permissions
with [custom roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles) or
other [predefined roles](https://docs.cloud.google.com/iam/docs/roles-overview#predefined).

Additional BigQuery permissions might be required depending
on the task. For information about BigQuery permissions, see
[BigQuery IAM roles and permissions](https://docs.cloud.google.com/bigquery/docs/access-control).

## Authentication and authorization

BigQuery MCP servers use the [OAuth
2.0](https://developers.google.com/identity/protocols/oauth2) protocol with
[IAM](https://docs.cloud.google.com/iam/docs/overview) for authentication and authorization.
All [Google Cloud identities](https://docs.cloud.google.com/docs/authentication/identity-products) are
supported for authentication to MCP servers.

The BigQuery MCP server doesn't accept API keys.

## BigQuery MCP OAuth scopes

OAuth 2.0 uses scopes and credentials to determine if an authenticated
principal is authorized to take a specific action on a resource. For more
information about OAuth 2.0 scopes at Google, see
[Using OAuth 2.0 to access Google APIs](https://developers.google.com/identity/protocols/oauth2).

BigQuery has the following MCP tool OAuth scopes:

| Scope URI for gcloud CLI | Description |
|---|---|
| `https://www.googleapis.com/auth/bigquery` | View and manage your data in BigQuery and see the email address for your Google Account. |

Additional scopes might be required on the resources accessed during a tool
call. To view a list of scopes required for BigQuery, see
[OAuth 2.0 scopes for BigQuery API v2](https://developers.google.com/identity/protocols/oauth2/scopes#bigquery).

## Configure an MCP client to use the BigQuery MCP server

Host programs, such as Claude or Gemini CLI, can instantiate MCP
clients that connect to a single MCP server. A host program can have multiple
clients that connect to different MCP servers. To connect to an MCP
server, the MCP client must know at a minimum the URL of the MCP server.

In your host, look for a way to connect to an MCP server. You're prompted
to enter details about the server, such as its name and URL.

For the BigQuery MCP server, enter the following as
required:

- **Server name**: BigQuery MCP server
- **Server URL** or **Endpoint**: https://bigquery.googleapis.com/mcp
- **Transport**: HTTP
- **Authentication details**: your Google Cloud credentials, your
  OAuth Client ID and secret, or an agent identity and credentials

  Which authentication details you choose depend on how you want to
  authenticate. For more information, see
  [Authenticate to MCP servers](https://docs.cloud.google.com/mcp/authenticate-mcp).

For host-specific guidance, see the following:

- [Gemini CLI MCP server setup](https://docs.cloud.google.com/mcp/configure-mcp-ai-application#gemini-cli)
- [Claude support: Getting started with custom connectors using remote MCP](https://support.claude.com/en/articles/11175166-getting-started-with-custom-connectors-using-remote-mcp)

For more general guidance, see [Connect to remote MCP servers](https://modelcontextprotocol.io/docs/develop/connect-remote-servers).

## Available tools

To view details of available MCP tools and their descriptions for the
BigQuery MCP server, see the [BigQuery MCP
reference](https://docs.cloud.google.com/bigquery/docs/reference/mcp).

### Limitations

The BigQuery MCP tools are subject to the following limitations:

- The [`execute_sql`](https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/execute_sql) and [`execute_sql_readonly`](https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/execute_sql_readonly) tools don't support querying Google Drive external tables.
- By default, the `execute_sql` and `execute_sql_readonly` tools limit query processing time to three minutes. Queries that run longer than three minutes are automatically canceled.
- Query results are limited to a maximum of 3,000 rows.
- The `execute_sql_readonly` tool allows only read-only operations on the data. Mutations such as DML statements, DDL statements, and Python UDFs aren't supported.

### List tools

Use the [MCP inspector](https://modelcontextprotocol.io/docs/tools/inspector) to list tools, or send a
`tools/list` HTTP request directly to the BigQuery MCP server.
The `tools/list` method doesn't require authentication.

    POST /mcp HTTP/1.1
    Host: bigquery.googleapis.com
    Content-Type: application/json

    {
      "jsonrpc": "2.0",
      "method": "tools/list",
    }

### Deny access to tools

The only MCP tool that isn't read-only is `execute_sql`. You can restrict access
to the `execute_sql` tool by creating a [deny policy that restricts read-write
MCP tool use](https://docs.cloud.google.com/mcp/control-mcp-use-iam#deny_read-write_mcp_tool_use).

## Sample use cases

The following are sample use cases for the BigQuery MCP server:

- Build workflows that use insights from BigQuery data to
  trigger certain actions such as creating issues and composing emails.

- Use BigQuery's advanced capabilities like forecasting
  for higher-order insights.

- Build a conversational experience for your users with custom agent
  instructions.

### Sample prompts

You can use the following sample prompts to get information about
BigQuery resources, gain insights, and analyze
BigQuery data:

- "List the datasets in project <var translate="no">`PROJECT_ID`</var>."
- "Find all the queries that I ran in project <var translate="no">`PROJECT_ID`</var> using the MCP server in the <var translate="no">`REGION`</var> region. Use the tag `goog-mcp-server:true` to identify the query jobs that ran through the MCP server."
- "Find the top orders by volume from <var translate="no">`DATASET_ID`</var> in project <var translate="no">`PROJECT_ID`</var>. Identify the appropriate tables, find the correct schema, and show the results."
- "Create a forecast on the table <var translate="no">`PROJECT_ID`</var>.<var translate="no">`DATASET_ID`</var>.<var translate="no">`TABLE_ID`</var> for future years. Use <var translate="no">`COLUMN_NAME`</var> as the data column and <var translate="no">`COLUMN_NAME`</var> as the timestamp column. Show the top 10 forecasts."

In the prompts, replace the following:

- <var translate="no">`PROJECT_ID`</var>: the Google Cloud project ID
- <var translate="no">`REGION`</var>: the name of the region
- <var translate="no">`DATASET_ID`</var>: the name of the dataset
- <var translate="no">`TABLE_ID`</var>: the name of the table
- <var translate="no">`COLUMN_NAME`</var>: the name of the column

## Optional security and safety configurations

MCP introduces new security risks and considerations due to the wide variety of
actions that you can do with the MCP tools. To minimize and manage these risks,
Google Cloud offers default settings and customizable
policies to control the use of MCP tools in your Google Cloud
organization or project.

For more information about MCP security and governance, see
[AI security and safety](https://docs.cloud.google.com/mcp/ai-security-safety).

### Use Model Armor

[Model Armor](https://docs.cloud.google.com/model-armor/overview) is a
Google Cloud service designed to enhance the security and
safety of your AI applications. It works by proactively screening LLM prompts
and responses, protecting against various risks and supporting responsible AI
practices. Whether you are deploying AI in your cloud environment, or on
external cloud providers, Model Armor can help
you prevent malicious input, verify content safety, protect sensitive data,
maintain compliance, and enforce your AI safety and security policies
consistently across your diverse AI landscape.

When Model Armor is enabled with
[logging enabled](https://docs.cloud.google.com/model-armor/configure-logging), Model Armor logs the entire
payload. This might expose sensitive information in your logs.

> [!CAUTION]
> **Caution:** Model Armor is available in [certain regions](https://docs.cloud.google.com/model-armor/locations). When Model Armor is enabled and you use an MCP server in a jurisdiction that Model Armor doesn't support, the routing behavior of the call might be different for different MCP servers. For more information about the behavior of individual MCP servers, see [Model Armor supported products](https://docs.cloud.google.com/mcp/model-armor-supported-products).

#### Enable Model Armor

You must enable Model Armor APIs before you can use Model Armor.

### Console

1.


   Enable the Model Armor API.


   **Roles required to enable APIs**


   To enable APIs, you need the Service Usage Admin IAM
   role (`roles/serviceusage.serviceUsageAdmin`), which
   contains the `serviceusage.services.enable` permission. [Learn how to grant
   roles](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).

   [Enable the API](https://console.cloud.google.com/flows/enableapi?apiid=modelarmor.googleapis.com)

   <br />

2. Select the project where you want to activate Model Armor.

### gcloud

Before you begin, follow these steps using the Google Cloud CLI with the
Model Armor API:

1.


   In the Google Cloud console, activate Cloud Shell.

   [Activate Cloud Shell](https://console.cloud.google.com/?cloudshell=true)


   At the bottom of the Google Cloud console, a
   [Cloud Shell](https://docs.cloud.google.com/shell/docs/how-cloud-shell-works)
   session starts and displays a command-line prompt. Cloud Shell is a shell environment
   with the Google Cloud CLI
   already installed and with values already set for
   your current project. It can take a few seconds for the session to initialize.

   <br />

2.

   Run the following command to set the API endpoint for the
   Model Armor service.

   ```bash
   gcloud config set api_endpoint_overrides/modelarmor "https://modelarmor.LOCATION.rep.googleapis.com/"
   ```

   Replace `LOCATION` with the region where you want to use
   Model Armor.

   <br />

#### Configure protection for Google and Google Cloud remote MCP servers

To help protect your MCP tool calls and responses you can use
Model Armor floor settings. A floor setting defines the minimum
security filters that apply across the project. This configuration applies a
consistent set of filters to all MCP tool calls and responses within
the project.

> [!TIP]
> **Tip:** Don't enable the prompt injection and jailbreak filter unless your MCP traffic carries natural language data.

Set up a Model Armor floor setting with MCP sanitization
enabled. For more information, see [Configure Model Armor floor
settings](https://docs.cloud.google.com/model-armor/configure-floor-settings).

> [!NOTE]
> **Note:** If the agent and the MCP server are in different projects, you can create floor settings in both projects (the client project and the resource project). In this case, Model Armor is invoked twice, once for each project.

See the following example command:

```bash
gcloud model-armor floorsettings update \
--full-uri='projects/PROJECT_ID/locations/global/floorSetting' \
--enable-floor-setting-enforcement=TRUE \
--add-integrated-services=GOOGLE_MCP_SERVER \
--google-mcp-server-enforcement-type=INSPECT_AND_BLOCK \
--enable-google-mcp-server-cloud-logging \
--malicious-uri-filter-settings-enforcement=ENABLED \
--add-rai-settings-filters='[{"confidenceLevel": "MEDIUM_AND_ABOVE", "filterType": "DANGEROUS"}]'
```

Replace `PROJECT_ID` with your Google Cloud project ID.

Note the following settings:

- <var translate="no">`INSPECT_AND_BLOCK`</var>: The enforcement type that inspects content for the Google MCP server and blocks prompts and responses that match the filters.
- <var translate="no">`ENABLED`</var>: The setting that enables a filter or enforcement.
- <var translate="no">`MEDIUM_AND_ABOVE`</var>: The confidence level for the Responsible AI - Dangerous filter settings. You can modify this setting, though lower values might result in more false positives. For more information, see [Model Armor confidence levels](https://docs.cloud.google.com/model-armor/overview#ma-confidence-levels).

#### Disable scanning MCP traffic with Model Armor

To stop Model Armor from automatically scanning traffic to and
from Google MCP servers based on the project's floor settings, run the following
command:

    gcloud model-armor floorsettings update \
      --full-uri='projects/PROJECT_ID/locations/global/floorSetting' \
      --remove-integrated-services=GOOGLE_MCP_SERVER

Replace `PROJECT_ID` with the Google Cloud project
ID. Model Armor doesn't automatically apply the rules defined in
this project's floor settings to any Google MCP server traffic.

Model Armor floor settings and general configuration can impact
more than just MCP. Because Model Armor integrates with services
like Vertex AI, any changes you make to floor settings can affect
traffic scanning and safety behaviors across all integrated services, not just
MCP.

### Control MCP use with IAM deny policies

[Identity and Access Management (IAM) deny policies](https://docs.cloud.google.com/iam/docs/deny-overview)
help you secure Google Cloud remote MCP servers. Configure these policies to
block unwanted MCP tool access.

For example, you can deny or allow access based on:

- The principal
- Tool properties like read-only
- The application's OAuth client ID

For more information, see [Control MCP use with Identity and Access Management](https://docs.cloud.google.com/mcp/control-mcp-use-iam).

## Quotas and limits

The BigQuery MCP server doesn't have its own quotas. There
is no limit on the number of calls that can be made to the MCP server.

You are still subject to the quotas enforced by the APIs called by the MCP
server tools. The following API methods are called by the MCP server tools:

| Tool | API method | Quotas |
|---|---|---|
| `list_dataset_ids` | [`datasets.list`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets/list) | [Dataset quotas and limits](https://docs.cloud.google.com/bigquery/quotas#dataset_limits) |
| `list_table_ids` | [`tables.list`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables/list) | [Table quotas and limits](https://docs.cloud.google.com/bigquery/quotas#table_limits) |
| `get_dataset_info` | [`datasets.get`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets/get) | [Dataset quotas and limits](https://docs.cloud.google.com/bigquery/quotas#dataset_limits) |
| `get_table_info` | [`tables.get`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables/get) | [Table quotas and limits](https://docs.cloud.google.com/bigquery/quotas#table_limits) |
| `execute_sql` | [`jobs.Query`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs/query) | [Query job quotas and limits](https://docs.cloud.google.com/bigquery/quotas#query_jobs) |
| `execute_sql_readonly` | [`jobs.Query`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs/query) | [Query job quotas and limits](https://docs.cloud.google.com/bigquery/quotas#query_jobs) |

For more information on BigQuery quotas, see
[Quotas and limits](https://docs.cloud.google.com/bigquery/quotas).

## What's next

- See the [BigQuery MCP reference documentation](https://docs.cloud.google.com/bigquery/docs/reference/mcp).
- Learn more about [Google Cloud MCP servers](https://docs.cloud.google.com/mcp/overview).
- See the MCP [supported products](https://docs.cloud.google.com/mcp/supported-products).