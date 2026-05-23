# Conversational analytics overview

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

> [!NOTE]
> **Note:** To provide feedback or request support for this feature, send an email to [bqca-feedback-external@google.com](mailto:bqca-feedback-external@google.com).

Conversational analytics in BigQuery lets you chat with agents about
your data using natural language. To get answers about your data, you can do
the following:

- Create [data agents](https://docs.cloud.google.com/bigquery/docs/conversational-analytics#data-agents) that automatically define data context and query processing instructions for a set of knowledge sources, such as tables, views, graphs, or user-defined functions (UDFs) that you select.
- If needed, you can create context and instructions for an agent in the form of custom table and field metadata, instructions to the agent for interpreting and querying the data, or by creating verified queries (previously known as *golden queries*) to configure the data agent to effectively answer questions for specific use cases.

Before customizing an agent, it's recommended that you first work with the
context and instructions that the agent creates.

Some examples of context and instructions that you provide to the agent are the
following:

- **Context**. A data agent for sales analysis can be configured to understand that "top performers" refers to sales representatives with the highest revenue, rather than just the most closed deals.
- **Instructions**. You can instruct a data agent to always filter data to the most recent quarter when asked about "trends," or to group results by "product category" by default.

After creating data agents, you can then have
[conversations](https://docs.cloud.google.com/bigquery/docs/conversational-analytics#conversations) with them to ask questions about
BigQuery data by using natural language. You can also create
[direct conversations](https://docs.cloud.google.com/bigquery/docs/create-conversations) with one or more
data sources to answer basic, one-off questions.

Conversational analytics is powered by [Gemini for Google
Cloud](https://docs.cloud.google.com/gemini/docs/overview) and supports some BigQuery ML functions. For
more information, see [BigQuery ML support](https://docs.cloud.google.com/bigquery/docs/conversational-analytics#bigquery-ml-support).

Learn [how and when Gemini
for Google Cloud uses your data](https://docs.cloud.google.com/gemini/docs/discover/data-governance).

> [!CAUTION]
> As an early-stage technology, Gemini for Google Cloud
> products can generate output that seems plausible but is factually incorrect. We recommend that you
> validate all output from Gemini for Google Cloud products before you use it.
> For more information, see
> [Gemini for Google Cloud and responsible AI](https://docs.cloud.google.com/gemini/docs/discover/responsible-ai).

## Data agents

Data agents consist of one or more knowledge sources, and a set of instructions
specific to a use case for processing that data. When you create a data agent,
you can configure it using the following options:

- Use *knowledge sources* such as tables, views, and UDFs with a data agent. You can also connect to Lakehouse tables as sources. For more information, see [Query Lakehouse data with natural language](https://docs.cloud.google.com/biglake/docs/conversational-analytics).
- Provide custom table and field metadata to describe the data in the most appropriate way for the given use case.
- Provide instructions for interpreting and querying the data, such as defining the following:
  - Synonyms and business terms for field names
  - Most important fields and defaults for filtering and grouping
- Create *verified queries* that the data agent can use to shape an agent's response structure and to learn the business logic that your organization uses. Verified queries were previously known as *golden queries* . Verified queries can use [supported BigQuery ML functions](https://docs.cloud.google.com/bigquery/docs/conversational-analytics#bigquery-ml-support) and support [query parameters](https://docs.cloud.google.com/bigquery/docs/create-data-agents#create-param-verified-queries).
- Create BigQuery custom glossary terms for each agent or import business glossary terms from Knowledge Catalog. These terms help an agent interpret user prompts. For advice on when to use each type, see [Create or review glossary terms](https://docs.cloud.google.com/bigquery/docs/create-data-agents#create-review-glossary-terms).

### Manage data agents

You can create, manage, and work with the following types of data agents in the
**Agent Catalog** tab in the Google Cloud console:

- A predefined sample agent for each Google Cloud project.
- A list of your drafted, created, and published agents.
- A list of agents that other people create and share with you.

For more information, see [Create data
agents](https://docs.cloud.google.com/bigquery/docs/create-data-agents).

Other services in the project that support data agents, such as the
[Conversational Analytics API](https://docs.cloud.google.com/gemini/docs/conversational-analytics-api/overview)
and
[Data Studio](https://docs.cloud.google.com/data-studio/conversational-analytics-overview), can
access data agents that you create in BigQuery. You can
also access an agent created in the Google Cloud console by calling it using
the [Conversational Analytics API](https://docs.cloud.google.com/gemini/docs/conversational-analytics-api/overview).

## Conversations

Conversations are persisted chats with a data agent or data source. You can
ask data agents multi-part questions that use common terms like "sales" or "most
popular," without having to specify table field names or define conditions to
filter the data. You can also ask questions about data located in objects such
as PDFs.

The chat response returned to you provides the following features:

- The answer to your question as text, code, or images (multimodal). The answer can include supported BigQuery ML functions.
- Generated charts where appropriate.
- Graph visualizations for GQL query paths.
- The agent's reasoning behind the results.
- Metadata about the conversation, such as the agent and data sources used.

When you create a direct conversation with a data source, the
[Conversational Analytics API](https://docs.cloud.google.com/gemini/docs/conversational-analytics-api/overview)
interprets your question without the context and processing instructions that a
data agent offers. Because of this, direct conversation results can be less
accurate. Use data agents for cases that require greater accuracy.

You can create and manage conversations in BigQuery using the
Google Cloud console. For more information, see [Analyze data with
conversations](https://docs.cloud.google.com/bigquery/docs/create-conversations).

## BigQuery ML support

Conversational analytics supports the following BigQuery ML functions
in response to chats with data agents and data sources, and in verified
SQL queries that you create.

- [`AI.FORECAST`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-forecast)
- [`AI.DETECT_ANOMALIES`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-detect-anomalies)
- [`AI.GENERATE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-generate)

To use the supported `AI.GENERATE` function, you must have [the required
permissions](https://docs.cloud.google.com/bigquery/docs/permissions-for-ai-functions#run_generative_ai_queries_with_end-user_credentials)
to run generative AI queries.

### BigQuery ML use cases

To activate supported BigQuery ML functions, use them in the following ways:

- When you create an agent and add a verified query---for example, if you are a data scientist who prepares a recurring report---you can use supported BigQuery ML functions in a verified query to describe defaults and automate the report.
- When you ask high-level questions about data to an agent, in a conversation, or in a verified query using keywords, the agent generates the BigQuery ML SQL in response to your questions.

The following table shows examples of one-shot prompts that activate the use of
BigQuery ML:

| Use case | Sample usage | [Public dataset](https://docs.cloud.google.com/bigquery/public-data) |
|---|---|---|
| Forecasting | "Predict the number of trips for the next month." | [`bigquery-public-data.san_francisco_bikeshare.bikeshare_trips`](https://console.cloud.google.com/bigquery?p=bigquery-public-data&d=san_francisco_bikeshare&t=bikeshare_trips&page=table) |
| Anomaly detection | "Find outliers in trips per day for 2018 using 2017 as a baseline." | [`bigquery-public-data.san_francisco_bikeshare.bikeshare_trips`](https://console.cloud.google.com/bigquery?p=bigquery-public-data&d=san_francisco_bikeshare&t=bikeshare_trips&page=table) |
| LLM text generation | "For each article in the 'sports' category, summarize the body column in 1-2 sentences." | [`bigquery-public-data.bbc_news.fulltext`](https://console.cloud.google.com/bigquery?p=bigquery-public-data&d=bbc_news.fulltext) |

## Graph support

Conversational analytics supports using a [graph](https://docs.cloud.google.com/bigquery/docs/graph-overview)
as a data source. When you ask questions about your graph, the agent constructs
GQL or SQL queries to answer them. Agents can use
[descriptions and synonyms](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/graph-schema-statements#element_table_property_definition)
that you define on your graph labels and
properties to improve the quality of the results.
Agents can also take advantage of [measures](https://docs.cloud.google.com/bigquery/docs/graph-measures)
defined on your graph to perform multi-level aggregation.
If the response includes graph paths,
graph visualizations are provided.

For example, you can use the
[`Look Graph` agent](https://console.cloud.google.com/bigquery/agents_hub;agentsHubTab=Agents;agentsPath=%2Fbq%2Fagents%2Fagent_59ff15ce-e31b-4ba6-ac72-57788dfe0d48)
to ask questions similar to the following about the
[`bigquery-public-data.thelook_ecommerce.graph`](https://console.cloud.google.com/bigquery?ws=!1m5!1m4!18m3!1sbigquery-public-data!2sthelook_ecommerce!3sgraph) graph:

- `Which product is most popular among 25-year-olds?`
- `Show me all bow tie orders in Chicago from users under 25`

The following limitations apply when you use a graph as a data source:

- You can use at most one graph as a data source per agent or conversation.
- You can't combine tables and graphs as data sources.

## Security

You can manage access to conversational analytics in BigQuery
using [Conversational Analytics API IAM roles and
permissions](https://docs.cloud.google.com/gemini/docs/conversational-analytics-api/access-control). For
information about the roles needed for specific operations, see the [data agent
required roles](https://docs.cloud.google.com/bigquery/docs/create-data-agents#required-roles) and the
[conversation required
roles](https://docs.cloud.google.com/bigquery/docs/create-conversations#required_roles).

## Locations

Conversational analytics operates globally; you can't choose which region to
use.

## Pricing

You are charged at [BigQuery compute
pricing](https://docs.cloud.google.com/bigquery/pricing#analysis_pricing_models) for queries that run when
you create data agents and have conversations with data agents or data
sources. There is no additional charge for creating and using data agents and
conversations during the Preview period.

## Best practices

Review the following guides to learn about best practices for using the
Conversational Analytics API:

- Set project-level, user-level, and query-level spending limits to [manage costs for your agents](https://docs.cloud.google.com/gemini/data-agents/conversational-analytics-api/manage-costs).
- [Ask effective questions](https://docs.cloud.google.com/gemini/data-agents/conversational-analytics-api/ask-effective-questions) in your conversations.
- Understand how [data retention and deletion](https://docs.cloud.google.com/gemini/data-agents/conversational-analytics-api/retention-deletion) works for data agents and conversations.

## Limitations

For more information about limitations on queries, conversations, data, and
visualizations, see
[Conversational Analytics API known limitations](https://docs.cloud.google.com/gemini/data-agents/conversational-analytics-api/known-limitations).

## Dynamic shared quota

Dynamic Shared Quota (DSQ) in Vertex AI manages capacity for the
Gemini model. Unlike conventional quotas, DSQ lets you access a
large shared pool of resources without a fixed per-project limit for model
throughput.

Performance, such as latency, can vary depending on the overall
system load. During times of high demand across the shared pool, you might
occasionally experience temporary `429 Resource Exhausted` errors. These errors
indicate that the shared pool capacity is momentarily constrained, but not that
you have reached a specific quota limit on your project. To check on the
capacity, retry the request after a short delay.

## Identify and analyze agent-generated queries

BigQuery jobs run by a data agent include specific labels. These
labels let you identify, filter, and analyze the agent's jobs.

You can use these labels for the following tasks:

- [Filter your billing report by label](https://docs.cloud.google.com/billing/docs/how-to/reports#filter-by-labels) to understand agent costs.
- Audit agent activity.
- Analyze query performance.

### Identify the data agent labels in the Google Cloud console

BigQuery applies labels to jobs that are run by a data agent. To
get the label key for filtering and other analysis, view the label key in the
Google Cloud console.

To view a data agent's label key, follow these steps:

1. In the Google Cloud console, [view the job details](https://docs.cloud.google.com/bigquery/docs/managing-jobs#view-job).

2. In the **Query job details** pane, locate the **Labels** section and look
   for labels prefixed with `ca`, such as `ca-bq-job: true`.

### Analyze agent-generated jobs

Use the label to analyze your agent-generated jobs. For example, to check how
many jobs were run by a data agent, run the following query against the
[`INFORMATION_SCHEMA.JOBS` view](https://docs.cloud.google.com/bigquery/docs/information-schema-jobs):

    SELECT
      COUNT(*) AS job_count
    FROM
      `PROJECT_ID`.`region-REGION`.INFORMATION_SCHEMA.JOBS
    WHERE
      EXISTS (
        SELECT 1
        FROM UNNEST(labels) AS label
        WHERE label.key = 'ca-bq-job' AND label.value = 'true'
      );

Replace the following:

- `PROJECT_ID`: your Google Cloud project ID.
- `REGION`: the region where your jobs run (for example, `us` or `eu`).

## What's next

- Learn more about the [Conversational Analytics API](https://docs.cloud.google.com/gemini/docs/conversational-analytics-api/overview).
- [Create data agents](https://docs.cloud.google.com/bigquery/docs/create-data-agents).
- [Analyze data with conversations](https://docs.cloud.google.com/bigquery/docs/create-conversations).
- [Use conversational Analytics with Lakehouse](https://docs.cloud.google.com/biglake/docs/conversational-analytics)
- Learn how to [filter resources using labels](https://docs.cloud.google.com/bigquery/docs/filtering-labels).