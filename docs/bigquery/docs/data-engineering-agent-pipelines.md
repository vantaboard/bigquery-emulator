# Use the Data Engineering Agent to build and modify data pipelines

The Data Engineering Agent lets you to build, modify, and troubleshoot data
pipelines in BigQuery using natural language prompts. The Data
Engineering Agent offers the following capabilities to streamline your data
engineering workflows to ingest data into BigQuery:

- **Dataform Integration**: The agent generates and organizes data pipeline code directly within Dataform repositories and workspaces
- **Plan Generation**: The agent can summarize its thinking and generate a plan that lets you review and verify the agent's plan before proceeding
- **Code Validation**: The agent automatically validates and fixes compilation errors of any generated code to ensure that the data pipeline is functional
- **Automatic Data Wrangling**: The agent performs data wrangling and transforms raw data into structured tables without manual intervention.
- **Custom Instructions**: The agent supports custom agent instructions that lets you define specific rules and reusable guidelines in natural language
- **External Context**: The agent is integrated with Knowledge Catalog for additional context
- **Pipeline Control**: You can review and customize generated agent plans before any actions are executed.
- **Optimization**: The agent can optimize performance in your data pipeline
- **Troubleshoot and Repair**: The agent can troubleshoot pipeline failures and fix its code.

For more examples of prompts you can use with the Data Engineering Agent, see
[Sample prompts](https://docs.cloud.google.com/bigquery/docs/data-engineering-agent-pipelines#sample_prompts).

You can also use the [Data Engineering Agent
API](https://docs.cloud.google.com/gemini/data-agents/data-engineering-agent/api-overview), which uses the
A2A protocol, to interact with the agent.

## Limitations

The Data Engineering Agent has the following limitations:

- The Data Engineering Agent doesn't support natural-language commands for the following file types:
  - Notebooks
  - Data preparation
  - Javascript within any SQLX
- The Data Engineering Agent cannot execute pipelines. You must review and run or schedule pipelines.
- The Data Engineering Agent cannot validate SQL that depends on nonexistent intermediary resources without full pipeline invocation (user-triggered).
- The Data Engineering Agent cannot search any web links or URLs provided through instructions or direct prompts.
- When importing files in an [agent instruction
  file](https://docs.cloud.google.com/bigquery/docs/data-engineering-agent-pipelines#create_agent_instructions), the `@` import syntax supports only paths that begin with `./`, `/`, or a letter.
- The [data preview](https://docs.cloud.google.com/bigquery/docs/data-engineering-agent-pipelines#review_a_data_pipeline) feature is supported only for tables, declarations, or queries with the `hasOutput` flag set to `true`.
- The Data Engineering Agent is subject to the [general limitations of AI
  technology](https://docs.cloud.google.com/gemini/docs/discover/responsible-ai).

## How the Data Engineering Agent uses your data

To produce higher-quality agent responses, the Data Engineering Agent can
retrieve additional data and metadata from BigQuery and
Knowledge Catalog, including sample rows from
BigQuery tables and data scan profiles generated in
Knowledge Catalog. The agent does not use this data for training; it uses the data only as additional
context during agent conversations to inform its responses.

## Where the Data Engineering Agent processes your data

For more information about the locations where the Data Engineering Agent
processes your data, see [Where Gemini in BigQuery
processes your data](https://docs.cloud.google.com/bigquery/docs/gemini-locations).

## Before you begin

Before you use the Data Engineering Agent, perform the steps in this section.

### Enable Gemini in BigQuery

Make sure that Gemini in BigQuery is enabled for your
Google Cloud project. For more information, see [Set up Gemini in
BigQuery](https://docs.cloud.google.com/bigquery/docs/gemini-set-up).

### Enable the required APIs

> [!NOTE]
> **Note:** If you don't have permission to enable the APIs, ask your project administrator to enable the APIs for you from the [APIs \& Services
> dashboard](https://console.cloud.google.com/apis/dashboard). Alternatively, your project administrator can grant you the [Service Usage Admin
> (`roles/serviceusage.serviceUsageAdmin`)](https://docs.cloud.google.com/service-usage/docs/access-control#serviceusage.serviceUsageAdmin) role in the Google Cloud console, which lets you enable and disable APIs for the current project.

### console

Enable the following APIs in the Google Cloud console for the Google Cloud
project you use with the Conversational Analytics API.

[Enable the Gemini Data Analytics API](https://console.cloud.google.com/flows/enableapi?apiid=geminidataanalytics.googleapis.com)

[Enable the Gemini for Google Cloud API](https://console.cloud.google.com/flows/enableapi?apiid=cloudaicompanion.googleapis.com)

[Enable the BigQuery API](https://console.cloud.google.com/flows/enableapi?apiid=bigquery.googleapis.com)

> [!TIP]
> **Tip:** After you enable an API, refresh the Google Cloud console page to confirm that it's enabled.

### gcloud

To enable the Gemini Data Analytics API, the Gemini for Google Cloud
API, and the BigQuery API, use the [Google Cloud CLI](https://docs.cloud.google.com/sdk/docs/install-sdk)
and run the following [`gcloud
services enable`](https://docs.cloud.google.com/sdk/gcloud/reference/services/enable) commands:

```
gcloud services enable geminidataanalytics.googleapis.com --project=PROJECT_ID
gcloud services enable cloudaicompanion.googleapis.com --project=PROJECT_ID
gcloud services enable bigquery.googleapis.com --project=PROJECT_ID
```

Replace `PROJECT_ID` with your Google Cloud project ID.

### Required roles


To get the permission that
you need to use the Data Engineering Agent,

ask your administrator to grant you the
following IAM roles on the project:

- [Dataform Code Editor](https://docs.cloud.google.com/iam/docs/roles-permissions/dataform#dataform.codeEditor) (`roles/dataform.codeEditor`)
- [BigQuery Job User](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquery#bigquery.jobuser) (`roles/bigquery.jobuser`)
- [Gemini Data Analytics Stateless Chat User](https://docs.cloud.google.com/iam/docs/roles-permissions/geminidataanalytics#geminidataanalytics.dataAgentStatelessUser) (`roles/geminidataanalytics.dataAgentStatelessUser`)


For more information about granting roles, see [Manage access to projects, folders, and organizations](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).


This predefined role contains the
`geminidataanalytics.locations.useDataEngineeringAgent`
permission,
which is required to
use the Data Engineering Agent.


You might also be able to get
this permission
with [custom roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles) or
other [predefined roles](https://docs.cloud.google.com/iam/docs/roles-overview#predefined).

### Knowledge Catalog integration prerequisites


To get the permission that
you need to integrate the Data Engineering Agent with Knowledge Catalog,

ask your administrator to grant you the
[Dataplex Catalog Editor](https://docs.cloud.google.com/iam/docs/roles-permissions/dataplex#dataplex.catalogEditor) (`roles/dataplex.catalogEditor`) IAM role on the project.


For more information about granting roles, see [Manage access to projects, folders, and organizations](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).


This predefined role contains the
`geminidataanalytics.locations.useDataEngineeringAgent`
permission,
which is required to
integrate the Data Engineering Agent with Knowledge Catalog.


You might also be able to get
this permission
with [custom roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles) or
other [predefined roles](https://docs.cloud.google.com/iam/docs/roles-overview#predefined).

You must also enable the [Knowledge Catalog API](https://console.cloud.google.com/flows/enableapi?apiid=dataplex.googleapis.com).

### Encrypt data with Cloud Key Management Service keys

You can encrypt data at the dataset or project level with the default
customer-managed Cloud Key Management Service keys in BigQuery. For more
information, see [Set a dataset default key](https://docs.cloud.google.com/bigquery/docs/customer-managed-encryption#dataset_default_key)
and [Set a project default key](https://docs.cloud.google.com/bigquery/docs/customer-managed-encryption#project_default_key).

You can encrypt pipeline code at the project level by [setting a default Dataform Cloud Key Management Service key](https://docs.cloud.google.com/dataform/docs/cmek#set-default-key).

### Configure VPC Service Controls perimeters

If you use VPC Service Controls, you must configure the perimeter to protect
Dataform, BigQuery, and Conversational Analytics API. For
more information, see
[Dataform](https://docs.cloud.google.com/vpc-service-controls/docs/supported-products#dataform),
[BigQuery](https://docs.cloud.google.com/vpc-service-controls/docs/supported-products#table_bigquery),
and [Conversational Analytics API](https://docs.cloud.google.com/vpc-service-controls/docs/supported-products#table_conversational_analytics).

## Generate a data pipeline with the Data Engineering Agent

To use the Data Engineering Agent in BigQuery, select one of the
following options:

### BigQuery pipelines

You can use the Data Engineering Agent in the BigQuery pipelines
interface by doing the following:

1. Go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, click arrow_drop_down **Create new** \>
   **Pipeline**.

3. Select an option for execution credentials, and then click **Get started**. These
   credentials aren't used by the agent, but are required to execute
   the generated data pipeline.

4. Click **Try out the agent experience for data pipeline**.

5. In the **Ask agent** field, enter a natural language prompt to generate a
   data pipeline---for example:

         Create dimension tables for a taxi trips star schema from
         new_york_taxi_trips.tlc_green_trips_2022. Generate surrogate keys and all
         the descriptive attributes.

   After you enter a prompt, click **Send**.
6. The Data Engineering Agent generates a data pipeline based on your prompt.

The Data Engineering Agent generates a proposed draft of
a data pipeline. You can click a pipeline node to review the generated SQLX query.
To apply the agent-suggested data pipeline, click **Apply**.

![Data pipeline with an 'Apply' button highlighted, indicating changes suggested by the Data Engineering Agent.](https://docs.cloud.google.com/static/bigquery/images/dea-apply.png)

### Dataform

You can use the Data Engineering Agent in Dataform
by doing the following:

1. Go to **Dataform**.

   [Go to Dataform](https://console.cloud.google.com/bigquery/dataform)
2. Select a repository.

3. Select or create a development workspace.

4. In the workspace, click **Ask Agent**.

5. In the **Ask agent** prompt that appears, enter a natural language prompt
   to generate a data pipeline---for example:

         Create dimension tables for a taxi trips star schema from
         new_york_taxi_trips.tlc_green_trips_2022. Generate surrogate keys and all
         the descriptive attributes.

   After you enter a prompt, click **Send**.

After your prompt is sent, the Data Engineering Agent generates a data pipeline and
modifies Dataform SQLX files based on your prompt. The agent
applies these changes directly to your workspace files.

### Edit a data pipeline

To edit your data pipeline, click
**Ask agent**, and then enter a prompt that suggests a change to the data pipeline.

![Data pipeline interface with the 'Ask agent' button highlighted.](https://docs.cloud.google.com/static/bigquery/images/dea-ask-agent.png)

Review the changes proposed by the Data Engineering Agent, and then click **Apply**
to apply the changes.

You can also edit a SQLX query manually by selecting a pipeline node and then
clicking **Open**.

## Review a data pipeline

You can click a pipeline node in a data pipeline generated by the Data Engineering Agent to review it.

- The **Configuration** tab shows the generated SQLX query associated with the node.
- The **Data preview** tab shows the input and output table of the file. You can preview your data transformation through this node by clicking **Run task** to run the task with or without dependencies.

## Troubleshoot data pipeline errors

If you encounter any errors during data pipeline generation, verify that you
have completed all prerequisites to run the Data Engineering Agent. For more
information, see [Before you begin](https://docs.cloud.google.com/bigquery/docs/data-engineering-agent-pipelines#before_you_begin).

### Run a Gemini Cloud Assist investigation

> [!WARNING]
>
> **Preview**
>
>
> This feature is
>
> subject to the "Pre-GA Offerings Terms" in the General Service Terms section of the
> [Service Specific
> Terms](https://docs.cloud.google.com/terms/service-terms#1).
>
> Pre-GA features are available "as is" and might have limited support.
>
> For more information, see the
> [launch stage descriptions](https://cloud.google.com/products/#product-launch-stages).

For further pipeline troubleshooting, you can use the Data Engineering Agent to
run a root cause analysis and to suggest troubleshooting recommendations.

This feature uses Gemini Cloud Assist investigations
([Preview](https://cloud.google.com/products#product-launch-stages)) and is only
available to users with a [Premium Support
contract](https://cloud.google.com/support/premium). For more information about
enabling Gemini Cloud Assist investigations, see [Troubleshoot issues
with Gemini Cloud Assist Investigations](https://docs.cloud.google.com/cloud-assist/investigations).

You can use the Data Engineering Agent to troubleshoot data pipeline errors with
the following steps:

1. In your pipeline or your development workspace, click the **Executions** tab.
2. From the executions list, find the failed data pipeline run. You can identify
   failed runs in the **Status** column.

   ![A list of pipeline executions, with a failed run highlighted in the 'Status' column.](https://docs.cloud.google.com/static/bigquery/images/dea-failed-runs.png)
3. Hover over the icon, and then click **Investigate**. The Data Engineering Agent
   runs a root cause analysis (RCA) on your data pipeline execution for errors.

   ![Data pipeline interface showing an icon to 'Investigate' a failed run, prompting the Data Engineering Agent to diagnose errors.](https://docs.cloud.google.com/static/bigquery/images/dea-investigate.png)
4. After the analysis is complete, the Data Engineering Agent generates a
   report in the **Observations and Hypothesis** section. The report includes the following:

   - Observations and data points extracted from the data pipeline execution logs.
   - Probable causes for the failure.
   - A set of actionable steps or recommendations to resolve the identified issue.

With the troubleshooting report from the Data Engineering Agent, you can implement
the recommendations manually. You can also instruct the Data Engineering Agent
to apply the fix for you by doing the following steps:

1. Copy the suggestions in the troubleshooting report.
2. Return to the Data Engineering Agent:
   1. If you are using BigQuery pipelines, go to your pipelines page, and then click **Ask agent**.
   2. If you are using Dataform, click **Ask agent**.
3. Paste the suggestions into the prompt, and then instruct the Data Engineering Agent to make the fixes directly to your data pipeline.
4. Click **Send**.

## Additional agent features and customizations

The following sections describe additional agent capabilities and other methods to customize the Data Engineering Agent.

### Create agent instructions

Agent instructions are natural-language instructions for the Data Engineering Agent that let you store persistent instructions so the agent follows a set
of custom, predefined rules. Use agent instructions if you want the agent's results to be consistent across your organization---for example, with naming
conventions or to enforce a style guide.

You can create a [`GEMINI.MD` context
file](https://docs.cloud.google.com/gemini/docs/codeassist/use-agentic-chat-pair-programmer#create-context-file)
as an agent instruction file for the Data Engineering Agent. You can create agent
instruction files to use in your local workspace, or you can use the same
instruction files across multiple data pipelines with an external repository.

To create agent instructions, do the following:

1. Under **Ask agent** , click **Pipeline instructions**.
2. In the **Instructions for pipeline** pane, click **Create instructions file**.
3. In the `GEMINI.MD` file that appears, enter your instructions in natural
   language.

   The following example shows an agent instruction file with several rules:

         1. All event-specific tables MUST be prefixed with `cs_event_`.
         2. The primary key for any player activity table is a composite key of `player_id` and `event_timestamp_micros`.
         3. Filter out any player actions where `mana_spent` is greater than `max_mana_pool`. This is considered a data anomaly.

4. Click **Save**.

For information on how best to structure your agent instruction files, see [Best
practices with agent instruction files](https://docs.cloud.google.com/bigquery/docs/data-engineering-agent-pipelines#best_practices_with_agent_instruction_files).

#### Load agent instructions from an external repository

To reuse a set of agent instructions across multiple
data pipelines, link an external repository:

1. Under **Ask agent** , click **Pipeline instructions**.
2. Under **External repository** , select **Use instructions from external
   repository**.
3. In the fields provided, specify a repository that contains agent instructions you want to use with your data pipeline.
4. Click **Save**.

#### Import additional local files as agent instructions

You can also import other instruction files for the Data Engineering Agent into
the `GEMINI.md` file with `@file.md` syntax. For more information, see [Memory Import Processor](https://geminicli.com/docs/reference/memport/).

### Automatic data wrangling

You can use the Data Engineering Agent to transform raw, unprocessed data into
structured tables suitable for data analysis. When requested, the agent first samples up to 1,000,000 records
from each standard or external table. The agent then performs deep data
analysis by running profiling queries on this sample. After generating data
transformations, the agent repeats this sampling and profiling process to
assess the quality of the transformations. These data wrangling transformations might include fixing data inconsistencies, outliers, or type mismatches.
The Data Engineering Agent then creates a plan that outlines the proposed
wrangling steps for you to review and refine before any action occurs.

The Data Engineering Agent also initiates the data wrangling analysis whenever
you add a raw table, such as a CSV-based external table. You can review the data
wrangling plan and adjust it with conversational commands.

Data sampling and profiling uses
BigQuery resources and are subject to [BigQuery
pricing](https://cloud.google.com/bigquery/pricing).

The Data Engineering Agent supports the following data wrangling
transformations:

- Data cleaning. The agent can analyze raw data and suggest cleanup opportunities, such as removing outliers, filling missing or inconsistent values (data imputation), fixing duplicate data, or standardizing data formats---for example, phone numbers or addresses
- Structural transformations. When a target schema is provided, the agent can unnest or extract values from `JSON`, `ARRAY`, or `STRUCT` types; merge multiple columns into one; or split one column into multiple columns
- Data type detection and conversion. The agent can analyze the data to determine the appropriate field types. The agent can then perform secure type casting to resolve any formatting inconsistencies within the date, time, datetime, or timestamp fields.
- Unit conversions. The agent can automatically convert various units within a field into one consistent unit to standardize your data.

To ensure accuracy, the agent uses representative samples of your data to detect
issues and validate its transformation logic.

### Generate and review agent plans

The Data Engineering Agent can generate agent plans that provide a summary and
overview of the objectives and steps that it takes to complete a request. When
you prompt the agent with complex requests that require many changes, we
recommend asking the agent to provide you an agent plan so you can review the
agent's intentions before it takes any actions. A Data Engineering Agent plan
generally consists of the following:

- The agent's objective for a particular request
- A high-level overview of the steps the agent plans to take
- Any assumptions the agent makes
- Files the agent plans to modify
- Any optimization or cleaning steps it plans to perform
- A phased execution plan

In your prompt, you can include the need to review and approve the plan
so that the agent doesn't take any action without your explicit approval. For
example:

```
Create a plan for a pipeline that finds the
top N pick up and drop off locations in NYC. I want to review the plan and
approve it before you create the pipeline.
```

The agent might also generate an agent plan automatically and request your
approval. This result can occur when a prompt is too ambiguous, or if the agent
needs more clarity to fulfill your request.

For best practices about using agent plans, see [Best practices](https://docs.cloud.google.com/bigquery/docs/data-engineering-agent-pipelines#best_practices).

### Add context from Knowledge Catalog

The Data Engineering Agent uses Knowledge Catalog by attaching
glossary terms to BigQuery tables and columns and generating data
profile scans. Glossary terms can tag columns that require additional context,
such as columns containing personally identifiable information (PII) that
require special-handling instructions, or to identify matching columns with
different naming across tables.

Knowledge Catalog also utilizes [data
profiling](https://docs.cloud.google.com/dataplex/docs/data-profiling-overview), which provides the agent
with a better understanding of data distribution within table columns and helps
the agent create more specified data quality assertions

### Add data quality checks to an existing table

When you prompt the agent to add quality checks, the agent infers reasonable checks for the table based
on the schema and samples. You can also add opinionated assertions as
part of the prompt. For example:

      Add data quality checks for bigquery-public-data.thelook_ecommerce.users.

### Optimize data pipelines

You can prompt the agent to optimize your data pipelines. When generating
DDL for new tables, the Data Engineering Agent recommends partitioning and
clustering based on the analyzed data usage patterns. Additionally, the
agent can automatically apply other pipeline optimizations. Examples of
possible optimizations include the following:

- Column pruning to reduce data read from storage to act as a primary cost and performance driver.
- Predicate pushdowns to filter data early in the execution plan to significantly reduce the volume processed by subsequent operations.
- Elimination of common subexpressions to improve efficiency by identifying and computing shared transformation logic only once, preventing inefficient practices like scanning and joining large tables multiple times.
- Incremental models to process only new or changed data since the last run instead of rebuilding entire tables with each run.

## Sample prompts

The following sections provide sample prompts you can use with the
Data Engineering Agent to develop your data pipeline.

### Aggregate existing data into a new table

With this prompt, the Data Engineering Agent uses the schema and samples to
infer data grouping by key. The agent typically sets up a new table
configuration with table and column descriptions.

      Create a daily sales report from the
      bigquery-public-data.thelook_ecommerce.order_items table into a
      reporting.daily_sales_aggregation table.

### Create a new derived column and add data quality checks to the new table

This prompt shows how to add a table and a column, and specify
quality checks to the table at the same time:

      Create a new table named staging.products from
      bigquery-public-data.thelook_ecommerce.products and add a calculated column
      named gross_profit, which is the retail_price minus the cost.


      Also, add the following assertions: ID must not be null and must be unique.
      The retail_price must be greater than or equal to the cost. The department
      column can only contain 'Men' or 'Women'.

### Create UDFs as part of the model definition

The Data Engineering Agent can also set up the DDL to create user-defined
functions (UDFs). While the agent won't actually create the UDF, you can create
the UDF by running the data pipeline. These UDFs can be used in model definitions in
your data pipeline.

      Create a user-defined function (UDF) named get_age_group that takes an integer
      age as input and returns a string representing the age group ('Gen Z',
      'Millennial', 'Gen X', 'Baby Boomer').


      Use this UDF on the age column from the
      bigquery-public-data.thelook_ecommerce.users table to create a new view called
      reporting.user_age_demographics that includes user_id, age, and the calculated
      age_group.

## Best practices

To improve results when working with the Data Engineering Agent and
Dataform, we recommend that you do the following:

**Use agent instructions for common requests.** If you commonly apply certain
techniques, or if you frequently make the same corrections to the agent, use
agent instructions as a centralized location to store common instructions and
requests.

**Utilize agent plans.** Agent plans can be helpful to break down complex
pipeline tasks. Agent plans can also show you agent assumptions and intentions,
so we recommend reviewing those plans to make sure the agent is provided the
correct context.

After reviewing a plan, you can edit the plan by prompting the Data Engineering
Agent with feedback and changes. For example:

    In the plan, ensure that all of the intermediate tables are views.

In some cases, it can be helpful to ask the agent to generate a plan that
doesn't need your explicit approval. The act of making the agent plan forces the
Data Engineering Agent to break down its actions, which often leads to better
outcomes. You can force the agent to generate a plan and execute it
automatically. For example:

    Create a plan for a pipeline that finds the
    top N pick up and drop off locations in NYC. You have my explicit pre-approval
    to go ahead and execute this plan.

**Write clearly.** State your request clearly and avoid being vague. Where
possible, provide source and destination data sources when prompting, as shown in
the following example:

      Extract data from the sales.customers table in the us_west_1 region, and load
      it into the reporting.dim_customers table in BigQuery. Match the schema of the
      destination table.

**Provide direct and scoped requests.** Ask one question at a time, and keep
prompts concise. For prompts with more than one question, itemize each distinct
part of the question to improve clarity, as shown in the following example:

      1. Create a new table named staging.events_cleaned. Use raw.events as the
         source. This new table should filter out any records where the user_agent
         matches the pattern '%bot%'. All original columns should be included.

      2. Next, create a table named analytics.user_sessions. Use
         staging.events_cleaned as the source. This table should calculate the
         duration for each session by grouping by session_id and finding the
         difference between the MAX(event_timestamp) and MIN(event_timestamp).

**Provide explicit instructions and emphasize key terms.** You can add emphasis to
key terms or concepts in your prompts and label certain requirements as
important, as shown in the following example:

      When creating the staging.customers table, it is *VERY IMPORTANT* that you
      transform the email column from the source table bronze.raw_customers.
      Coalesce any NULL values in the email column to an empty string ''.

**Specify the order of operations.** For ordered tasks, structure your prompt in
lists, where listed items are divided into small, focused steps, as shown in the
following example:

      Create a pipeline with the following steps:
      1. Extract data from the ecomm.orders table.
      2. Join the extracted data with the marts.customers table on customer_id.
      3. Load the final result into the reporting.customer_orders table.

**Refine and iterate.** Keep trying different phrases and approaches to see what
yields the best results. If the agent generates invalid SQL or other mistakes,
guide the agent with examples or public documentation.

      The previous query was incorrect because it removed the timestamp. Please
      correct the SQL. Use the TIMESTAMP_TRUNC function to truncate the
      event_timestamp to the nearest hour, instead of casting it as a DATE. For
      example: TIMESTAMP_TRUNC(event_timestamp, HOUR).

### Best practices with agent instruction files

[Create agent instruction files](https://docs.cloud.google.com/bigquery/docs/data-engineering-agent-pipelines#create_agent_instructions) to customize the Data Engineering Agent
to suit your needs. When you use agent instructions, we recommend the following:

- All file paths in Dataform are relative to the root of the repository. Use relative paths for any `@file.md` syntax to properly import instructions to `GEMINI.md`.
- Files imported in `GEMINI.md` can themselves contain imports, which can create a nested structure. To prevent infinite recursion, `GEMINI.md` has a maximum import depth of five levels.
- To share instructions across data pipelines, store instructions in a central Dataform repository and link them to the working Dataform repository. You can use local instructions to override central rules for pipeline-specific behavior.
- To ensure consistency in your project, you can link to naming convention files or style guides and instruct the agent to follow these guidelines when working with your data pipelines.
- You can suggest data layers in the instruction file to group different types of data together.
- Using headings and lists in the agent instruction file can help organize and clarify instructions for the Data Engineering Agent.
- Provide meaningful filenames and group similar instructions together in a file. Organize rules logically by category, feature, or functionality with Markdown headings.
- To avoid conflicting instructions, clearly define the specific conditions under which each instruction applies.
- Iterate and refine your prompts and workflow. Agent behavior changes over time with agent rollouts and model upgrades, so we recommend iterating on your rules with different prompts to identify areas that might need improvement. Keep your rules file in sync with any changes to your data pipeline.

The following example shows an agent instruction file named `GEMINI.md` that
utilizes our best practices for effective use of the Data Engineering Agent:

      ### Naming Conventions

      * Datasets: [business_domain]_[use_case] (e.g., ecommerce_sales)

      * Tables:
          - Raw/External: raw_[source_name]
          - Staging: stg_[business_entity]
          - Dimension: dim_[dimension_name]
          - Fact: fct_[fact_name]

      * Dataform Folders:
          - sources
          - staging
          - marts
          - dataProducts

      * Views: vw_[view_name]

      * Columns: snake_case (e.g., order_id, customer_name)

      ## Cloud Storage data load
      * When ingesting data from Cloud Storage, create external tables.

      ## Null handling
      * Filter out null id values

      ## String normalization
      * Standardize string columns by converting to lower case

      ## Data Cleaning Guidelines
      @./generic_cleaning.md