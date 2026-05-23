# Learn how to use the BigQuery Migration Service MCP server

> [!WARNING]
>
> **Preview**
>
>
> This feature is
>
> subject to the "Pre-GA Offerings Terms" in the General Service Terms section of the
> [Service Specific
> Terms](https://docs.cloud.google.com/terms/service-terms#1), and the
> [Additional Terms for Generative AI
> Preview Products](https://cloud.google.com/trustedtester/aitos).
>
> Pre-GA features are available "as is" and might have limited support.
>
> For more information, see the
> [launch stage descriptions](https://cloud.google.com/products/#product-launch-stages).

<br />

This document shows you how to use the BigQuery remote Model Context Protocol (MCP) server to connect with AI applications including Gemini CLI, ChatGPT, Claude, and custom applications you are developing. You can BigQuery Migration Service Model Context Protocol (MCP) server to perform tasks such as translating SQL queries into GoogleSQL syntax, generating DDL statements from SQL input queries, and getting explanations of SQL translations. The BigQuery remote MCP server is enabled when you enable the BigQuery API.

[Model Context Protocol](https://modelcontextprotocol.io/docs/getting-started/intro)
(MCP) standardizes how large language models (LLMs) and AI applications or
agents connect to outside data sources. MCP servers let you use their tools,
resources, and prompts to take actions and get updated data from their backend
service.

Local MCP servers typically run on your local machine and use the standard input
and output streams (`stdio`) for communication between services on the same
device. MCP servers run on the service's infrastructure and offer an
HTTPS endpoint to AI applications for communication between the AI MCP client
and the MCP server. For more information on MCP architecture, see
[MCP architecture](https://modelcontextprotocol.io/docs/learn/architecture).

Google and Google Cloud MCP servers have the following features and
benefits:

- Simplified, centralized discovery
- Managed global or regional HTTPS endpoints
- Fine-grained authorization
- Optional prompt and response security with Model Armor protection
- Centralized audit logging

For information about other MCP servers and information about security and
governance controls available for Google Cloud MCP servers, see
[Google Cloud MCP servers overview](https://docs.cloud.google.com/mcp/overview).

## Before you begin

1.


   Enable the BigQuery Migration Service API.


   **Roles required to enable APIs**


   To enable APIs, you need the Service Usage Admin IAM
   role (`roles/serviceusage.serviceUsageAdmin`), which
   contains the `serviceusage.services.enable` permission. [Learn how to grant
   roles](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).

   [Enable the API](https://console.cloud.google.com/flows/enableapi?apiid=bigquerymigration.googleapis.com)

### Required roles


To get the permissions that
you need to enable the BigQuery Migration Service MCP server,

ask your administrator to grant you the
following IAM roles on the project where you want to enable the BigQuery Migration Service MCP server:

- Make MCP tool calls: [MCP Tool User](https://docs.cloud.google.com/iam/docs/roles-permissions/mcp#mcp.toolUser) (`roles/mcp.toolUser`)
- Use the BigQuery Migration Service: [Migration Workflow Editor](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquerymigration#bigquerymigration.editor) (`roles/bigquerymigration.editor`)


For more information about granting roles, see [Manage access to projects, folders, and organizations](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).


These predefined roles contain

the permissions required to enable the BigQuery Migration Service MCP server. To see the exact permissions that are
required, expand the **Required permissions** section:


#### Required permissions

The following permissions are required to enable the BigQuery Migration Service MCP server:

- Make MCP tool calls: `mcp.tools.call`
- Use the BigQuery Migration Service:
  - `bigquerymigration.workflows.create`
  - `bigquerymigration.workflows.get`
  - `bigquerymigration.workflows.list`
  - `bigquerymigration.workflows.delete`
  - `bigquerymigration.subtasks.get`
  - `bigquerymigration.subtasks.list`


You might also be able to get
these permissions
with [custom roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles) or
other [predefined roles](https://docs.cloud.google.com/iam/docs/roles-overview#predefined).

Additional BigQuery Migration Service permissions might be required depending
on the task. For information about BigQuery Migration Service roles and
permissions, see [BigQuery Migration Service roles and permissions](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquerymigration).

## Authentication and authorization

BigQuery Migration Service MCP servers use the [OAuth 2.0](https://developers.google.com/identity/protocols/oauth2)
protocol with [Identity and Access Management (IAM)](https://docs.cloud.google.com/iam/docs/overview) for
authentication and authorization. All [Google Cloud identities](https://docs.cloud.google.com/docs/authentication/identity-products)
are supported for authentication to MCP servers.

The BigQuery Migration Service MCP server doesn't accept API keys.

## BigQuery Migration Service MCP OAuth scopes

OAuth 2.0 uses scopes and credentials to determine if an authenticated
principal is authorized to take a specific action on a resource. For more
information about OAuth 2.0 scopes at Google, see
[Using OAuth 2.0 to access Google APIs](https://developers.google.com/identity/protocols/oauth2).

BigQuery Migration Service has the following MCP tool OAuth scopes:

| Scope URI for gcloud CLI | Description |
|---|---|
| `https://www.googleapis.com/auth/bigquerymigration` | View and manage your workflows in BigQuery Migration Service and see the email address for your Google Account. |
| `https://www.googleapis.com/auth/devstorage.read_only` | This scope is required for query translations that read data from Cloud Storage. |

Resources accessed during a tool call might require additional scopes.

## Configure an MCP client to use the BigQuery Migration Service MCP server

Host programs, such as Claude or Gemini CLI, can instantiate MCP
clients that connect to a single MCP server. A host program can have multiple
clients that connect to different MCP servers. To connect to an MCP
server, the MCP client must know at least the URL of the MCP server.

In your host, look for a way to connect to a MCP server. You're prompted
to enter details about the server, such as its name and URL.

For the BigQuery Migration Service MCP server, enter the following as
required:

- **Server name**: BigQuery Migration Service MCP server
- **Server URL** or **Endpoint**: bigquerymigration.googleapis.com/mcp
- **Transport**: HTTP
- **Authentication details**: your Google Cloud credentials, your
  OAuth Client ID, and secret, or an agent identity and credentials

  Which authentication details you choose depend on how you want to
  authenticate. For more information, see
  [Authenticate to MCP servers](https://docs.cloud.google.com/mcp/authenticate-mcp).

For host-specific guidance, see the following:

- [Gemini CLI MCP server setup](https://docs.cloud.google.com/mcp/configure-mcp-ai-application#gemini-cli)
- [Claude support: Getting started with custom connectors using remote MCP](https://support.claude.com/en/articles/11175166-getting-started-with-custom-connectors-using-remote-mcp)

For more general guidance, see [Connect to remote MCP servers](https://modelcontextprotocol.io/docs/develop/connect-remote-servers).

## Available tools

To view details of available MCP tools and their descriptions for the
BigQuery Migration Service MCP server, see the [BigQuery Migration Service MCP
reference](https://docs.cloud.google.com/bigquery/docs/reference/migration/mcp).

### List tools

Use the [MCP inspector](https://modelcontextprotocol.io/docs/tools/inspector) to list tools, or send a
`tools/list` HTTP request directly to the BigQuery Migration Service MCP
server. The `tools/list` method doesn't require authentication.

    POST /mcp HTTP/1.1
    Host: bigquerymigration.googleapis.com
    Content-Type: application/json

    {
      "jsonrpc": "2.0",
      "method": "tools/list",
    }

## Sample use cases

The following are sample use cases for the BigQuery Migration Service MCP
server:

- Using an MCP client with an IDE, translate a query file into GoogleSQL syntax.
- Using an MCP client without an IDE, translate a specified query into GoogleSQL syntax.
- Assess translation quality.
- Get explanations of SQL translations.
- Generate a DDL statement for a specified query.

### Sample prompts

You can use the following sample prompts to create and manage
BigQuery Migration Service resources:

- Translate the <var translate="no">`DIALECT`</var> query in this
  <var translate="no">`FILENAME`</var>. Use <var translate="no">`PROJECT_ID`</var> and
  <var translate="no">`LOCATION`</var>. Persist the output and translation logs
  into separate directories.

  When you use this prompt, the MCP client calls the `translate_query` tool to
  translate the query in the specified file. The MCP client periodically calls
  the `get_translation` tool to get the results. After the translation
  completes, the client writes the output to the output directory and the logs
  to the logs directory.
- Translate this query from <var translate="no">`DIALECT`</var>:
  <var translate="no">`QUERY`</var>. Use <var translate="no">`PROJECT_ID`</var> and
  <var translate="no">`LOCATION`</var>.

  When you use this prompt, the MCP client calls the `translate_query` tool to
  translate the specified query and displays the translation results.
- Assess the translation quality.

  When you use this prompt, the MCP client reads and examines the translation
  logs and displays a summary of the translation issues with suggested next
  steps.
- Explain the translation.

  When you use this prompt, the MCP client calls the `explain_translation`
  tool to get an explanation of the translation. If the translation logs
  contain `RelationNotFound` or `AttributeNotFound` errors, the MCP client
  should suggest that you [create a metadata package](https://docs.cloud.google.com/bigquery/docs/generate-metadata).
  If you can't generate the metadata, you can send a prompt that requests the
  DDL statement.

  A sample response looks like the following:

      The translated code converts Teradata-specific features into their
      BigQuery equivalents. Here's a breakdown of the key changes:
      * `MACRO` to `PROCEDURE`: The `YourMacroName` macro was converted
      into a BigQuery stored procedure because
      BigQuery doesn't support macros.
      * `SELECT INTO` to `SET`:
        * For setting multiple `OUT` parameters in `YourStoredProcedureName`, the
          `SELECT ... INTO` is changed to `SET (...) = (SELECT STRUCT(...))`.
        * For single variable assignment in `YourOtherProcedureName`,
          `SELECT ... INTO` is replaced by `SET variable = (SELECT ...)` which is
          the standard in BigQuery.
      * Atomic Operations to `MERGE: The BEGIN REQUEST ... END REQUEST` blocks in
        the `ProcedureA`, `ProcedureB`, and `ProcedureC` procedures,
        which perform atomic "update or insert" operations, are translated into
        standard SQL `MERGE` statements. This is the correct and modern way to
        handle this logic in BigQuery.

- Generate DDL for this input query.

  The MCP client calls the `generate_ddl_suggestion` tool to start a
  suggestion job. The client gets the suggestion results by calling the
  `fetch_ddl_suggestion` tool. When the suggestion is available, the MCP
  client displays it.

  If the DDL statements are correct, you can send a prompt to prepend the
  generated DDL statements to the query to improve the translation quality.
- Prepend the generated DDL statements to the input query and retranslate.

  When you use this prompt, the MCP client prepends the DDL statements to the
  original input query and calls the `translate_query` tool. The client calls
  the `get_translation` tool to get the translation. The new query translation
  and the logs persist when they're available.

  If the generated DDL statements are correct, any `RelationNotFound` or
  `AttributeNotFound` errors should be resolved which results in improved
  translation quality.

In the prompts, replace the following:

- <var translate="no">`DIALECT`</var>: The dialect of the SQL query you're translating.
- <var translate="no">`QUERY`</var>: The query you're translating.
- <var translate="no">`FILENAME`</var>: The file that contains the query you're translating.
- <var translate="no">`PROJECT_NUMBER`</var>: Your Google Cloud [project number](https://docs.cloud.google.com/resource-manager/docs/creating-managing-projects#before_you_begin).
- <var translate="no">`LOCATION`</var>: The location of the [SQL translator](https://docs.cloud.google.com/bigquery/docs/locations#sql-translator-loc).

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

The BigQuery Migration Service MCP server doesn't have its own quotas. There
is no limit on the number of calls you can make to the MCP server.

You are still subject to the quotas that the APIs called by the MCP server tools
enforce. For more information, see [BigQuery Migration Service API](https://docs.cloud.google.com/bigquery/quotas#migration-api-limits)
on the Quotas and limits page.

## What's next

- Read the [BigQuery Migration Service MCP reference documentation](https://docs.cloud.google.com/bigquery/docs/reference/migration/mcp).
- Learn more about [Google Cloud MCP servers](https://docs.cloud.google.com/mcp/overview).
- See the MCP [supported products](https://docs.cloud.google.com/mcp/supported-products).