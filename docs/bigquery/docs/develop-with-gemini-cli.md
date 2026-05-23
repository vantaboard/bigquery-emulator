# Analyze data with the Gemini CLI

This page explains how to interact with BigQuery data in your terminal
by using natural-language prompts with the
[Gemini CLI](https://docs.cloud.google.com/gemini/docs/codeassist/gemini-cli). Using the
dedicated BigQuery
[extensions](https://docs.cloud.google.com/bigquery/docs/develop-with-gemini-cli#about-bigquery-extensions) with the Gemini CLI
enables AI-driven data analysis and insights directly within your command line.
You can analyze data from your BigQuery tables, or from tables in
the BigQuery public datasets.

These first-party extensions provide some common tools for data analysis and
insights. To build your own tools, see
[Use BigQuery with MCP, Gemini CLI, and other agents](https://docs.cloud.google.com/bigquery/docs/pre-built-tools-with-mcp-toolbox).

## About Gemini CLI and extensions

The Gemini CLI is an open-source conversational AI agent from
Google that accelerates development workflows and assists with coding,
debugging, data exploration, and content creation. It offers an agent-driven
experience to interact with Data Cloud services, such as
BigQuery, and other popular Google Cloud databases and
open-source databases.

For more information about the Gemini CLI, see the
[Gemini CLI documentation](https://github.com/google-gemini/gemini-cli/blob/main/README.md).

### How extensions work

Extensions expand the capabilities of the Gemini CLI, letting it
connect to and control specific Google Cloud services and other tools. The
extensions provide Gemini with context and API understanding,
enabling conversational interaction. You can load the extensions from GitHub
URLs, local directories, or registries. These extensions offer tools, slash
commands, and prompts. These are separate from IDE extensions, such as
Gemini Code Assist, which integrate using the MCP Toolbox.

## About the BigQuery extensions

BigQuery offers the following extensions to work with
BigQuery data through the Gemini CLI:

BigQuery Data Analytics extension
:   Use this extension to
    discover data, learn more about BigQuery tables and datasets,
    and ask natural language questions about your datasets. You can also
    generate forecasts or run a contribution analysis using built-in advanced
    tools. The tools in this extension help the Gemini CLI write
    SQL queries based on your natural language questions, and then execute the
    queries to provide answers for your questions.

BigQuery Conversational Analytics extension
:   Use this extension
    to get advanced insights from your BigQuery data using a
    pre-hosted, server-side analytics agent. The tools in this extension help
    the Gemini CLI send your natural language question to a
    built-in, hosted agent and respond with deeper insights from your data.

### BigQuery Data Analytics tools

The [BigQuery Data Analytics extension](https://github.com/gemini-cli-extensions/bigquery-data-analytics) provides the following
tools:

| Category | Tool | Example natural language prompt |
|---|---|---|
| Dataset and table information | `get_dataset_info` | Get information about the dataset bigquery-public-data.pypi |
| Dataset and table information | `get_table_info` | Get information about the table bigquery-public-data.pypi.file_downloads |
| Dataset and table information | `list_dataset_ids` | List all ML datasets in the project bigquery-public-data |
| Dataset and table information | `list_table_ids` | List all tables in the dataset bigquery-public-data.pypi |
| Search | `search_catalog` | Find tables related to PyPi downloads |
| SQL execution | `execute_sql` | Using bigquery-public-data.pypi.file_downloads, show me the top 10 downloaded pypi packages this month |
| Time-series forecasting | `forecast` | Using bigquery-public-data.pypi.file_downloads, forecast the downloads for the last four months of 2025 for package urllib3 |
| Contribution analysis | `analyze_contribution` | Analyze the contribution of country to revenue in the bigquery-public-data.google_analytics_sample.ga_sessions_20170801 table |

### Conversational Analytics tools

The [Conversational Analytics extension](https://github.com/gemini-cli-extensions/bigquery-conversational-analytics)
provides the following tools:

| Category | Tool | Example natural language prompt |
|---|---|---|
| Search | `search_catalog` | Find tables related to PyPi downloads |
| Conversational analytics | `ask_data_insights` | Using the tool ask_data_insights and the tables under bigquery-public-data.google_analytics_sample, tell me the channels I should focus on and why |

## Before you begin

Before you begin, you need the following:

- Access to at least one Google Cloud project.
- For the Data Analytics extension, enable the following APIs:

  - BigQuery
  - To use the `search_catalog` tool: Knowledge Catalog
- For the Conversational Analytics extension, enable the following APIs:

  - BigQuery
  - Knowledge Catalog
  - To use the `ask_data_insights` tool: Gemini for Google Cloud API, Gemini, and Data Analytics API with Gemini

### Required roles


To get the permissions that
you need to use the BigQuery extensions with Gemini CLI,

ask your administrator to grant you the
following IAM roles on Resource Manager resources (projects, folders, and organizations) or lower-level resources like datasets:

- Run BigQuery queries in a Google Cloud project: [BigQuery User](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquery#bigquery.user) (`roles/bigquery.user`)
- Use the foundational metadata tools to get BigQuery dataset and table metadata: [BigQuery Metadata Viewer](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquery#bigquery.metadataViewer) (`roles/bigquery.metadataViewer`)
- Use the Conversational Analytics extension: [Gemini for Google Cloud User](https://docs.cloud.google.com/iam/docs/roles-permissions/cloudaicompanion#cloudaicompanion.user) (`roles/cloudaicompanion.user`)


For more information about granting roles, see [Manage access to projects, folders, and organizations](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).


You might also be able to get
the required permissions through [custom
roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles) or other [predefined
roles](https://docs.cloud.google.com/iam/docs/roles-overview#predefined).

## Install the BigQuery extensions

Install the BigQuery extensions and then use them to explore and
analyze your data.

1. [Install the Gemini CLI](https://github.com/google-gemini/gemini-cli?tab=readme-ov-file#quickstart).

2. Set the environment variables to connect to your BigQuery
   project:

       export BIGQUERY_PROJECT="PROJECT_ID"

   Replace `PROJECT_ID` with your Google Cloud
   project ID.

   You can also set these optional environment variables:
   - `BIGQUERY_LOCATION`: the dataset location---for example: `export
     BIGQUERY_LOCATION="us"`
   - `BIGQUERY_USE_CLIENT_OAUTH`: whether to use client-side OAuth---for example: `export BIGQUERY_USE_CLIENT_OAUTH=true`
3. Install the BigQuery Data Analytics or Conversational
   Analytics extension using the `gemini extension install` command:

   - Optional: install the BigQuery Data Analytics extension:

         gemini extensions install https://github.com/gemini-cli-extensions/bigquery-data-analytics

   - Optional: install the BigQuery Conversational Analytics
     extension:

         gemini extensions install https://github.com/gemini-cli-extensions/bigquery-conversational-analytics

4. Start the Gemini CLI in interactive mode to explore and
   analyze your data:

       gemini

   The CLI automatically loads the BigQuery extension and its
   tools, which you can use to interact with your data.

> [!NOTE]
> **Note:** if you install both of the BigQuery extensions, the Gemini CLI might use tools from either extension to answer your questions. This could have a billing impact, as there is additional billing impact when using Conversational Analytics tools. For more information, see [Pricing](https://docs.cloud.google.com/bigquery/docs/develop-with-gemini-cli#pricing).

## Example prompts to explore and analyze your data

Enter prompts in the Gemini CLI. You can use the following
examples to get started.

### Explore and analyze tables

The following example prompts let you explore and analyze your tables.

- **Example prompt** to find tables related to PyPi downloads:

      Find tables related to PyPi downloads

  **Example response**

  The Gemini CLI returns a list of related tables:

      ✦ I found the following tables related to PyPi downloads:

          *   file_downloads: projects/bigquery-public-data/datasets/pypi/tables/file_downloads
          *   distribution_metadata: projects/bigquery-public-data/datasets/pypi/tables/distribution_metadata

- **Example prompt** to show the top ten downloaded PyPI packages this month:

      Using bigquery-public-data.pypi.file_downloads, show me the top 10
      downloaded pypi packages this month

  **Example response**

  The Gemini CLI returns the top 10 packages:

      ✦ Here are the top 10 most downloaded PyPI packages this month:

          1.  boto3: 685,007,866 downloads
          2.  botocore: 531,034,851 downloads
          3.  urllib3: 512,611,825 downloads
          4.  requests: 464,595,806 downloads
          5.  typing-extensions: 459,505,780 downloads
          6.  certifi: 451,929,759 downloads
          7.  charset-normalizer: 428,716,731 downloads
          8.  idna: 409,262,986 downloads
          9.  grpcio-status: 402,535,938 downloads
          10. aiobotocore: 399,650,559 downloads

### Run deeper insights using the `ask_data_insights` tool

The `ask_data_insights` tool triggers an agent on the BigQuery
Conversational Analytics API to answer your questions. The server-side agent
gathers additional context about your data and offers more insights.

- **Example prompt** to get insights on channels to focus on:

      Using ask_data_insights and using the tables under
      bigquery-public-data.google_analytics_sample, tell me the channels I should
      focus on and why

  **Example response**

  The Gemini CLI returns recommendations based on the analysis:

      Based on the analysis of the Google Analytics sample data, here are the
      channels you should focus on and why:

      |     Channel      | Total Visits | Total Pageviews | Total Transactions | Total Revenue |
      | --- | --- | --- | --- | --- |
      | Referral         |      104,838 |         707,597 |              5,543 |   $651,429.91 |
      | Direct           |      143,026 |         584,840 |              2,219 |   $434,840.55 |
      | Organic Search   |      381,561 |       1,569,218 |              3,581 |   $326,380.51 |
      | Display          |        6,262 |          34,202 |                152 |    $78,337.46 |
      | Paid Search      |       25,326 |         136,060 |                479 |    $43,558.90 |
      | Social           |      226,117 |         398,690 |                131 |     $4,916.54 |
      | Affiliates       |       16,403 |          47,417 |                  9 |       $597.38 |
      | (Other)          |          120 |             442 |                  1 |         $9.99 |

        Recommendations:

          1.  Prioritize referral traffic:
              *   Why: This is a highly valuable channel. It brings in the most
                  revenue and has a high number of transactions, despite not
                  having the highest number of visits. This indicates that
                  traffic from referrals is highly qualified and converts
                  well.
              *   Action: Identify your top referring websites and build
                  stronger relationships with them. Look for similar websites
                  to partner with.

          2.  Nurture your direct traffic:
              *   Why: Direct traffic is your second-highest revenue source.
                  These are likely returning customers or people who know your
                  brand well.

### Pricing

There is no additional cost for using metadata tools (such as
`get_dataset_info`, `get_table_info`, `list_dataset_ids`, and `list_table_ids`)
or the `search_catalog` tool.

Tools that run queries, including `execute_sql`, `forecast`, and
`analyze_contribution`, are billed according to
[BigQuery on-demand pricing](https://cloud.google.com/bigquery/pricing#on_demand_pricing)
or [BigQuery capacity pricing](https://cloud.google.com/bigquery/pricing#capacity_compute_analysis_pricing),
depending on your project configuration.

During beta (pre-v1.0), the `ask_data_insights` tool is offered at no additional
cost; however, any BigQuery queries that this tool triggers are
billed according to standard BigQuery pricing.

## What's next

- Learn more about [using BigQuery with MCP, Gemini CLI, and other agents](https://docs.cloud.google.com/bigquery/docs/pre-built-tools-with-mcp-toolbox).
- Learn more about [Gemini in BigQuery](https://docs.cloud.google.com/bigquery/docs/gemini-overview).
- See the [Data Analytics extension](https://github.com/gemini-cli-extensions/bigquery-data-analytics) on GitHub.
- See the [Conversational Analytics extension](https://github.com/gemini-cli-extensions/bigquery-conversational-analytics) on GitHub.