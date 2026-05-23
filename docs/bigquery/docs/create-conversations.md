# Analyze data with conversations

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

This document describes how to create, edit, and delete conversations in
BigQuery. Conversations are persisted chats with a [data
agent](https://docs.cloud.google.com/bigquery/docs/create-data-agents) or data sources, such as tables, views
or graphs, that you select.

You can ask data agents multi-part questions that use common terms---for example,
"sales" or "most popular"---without specifying table field names, or defining
conditions to filter the data. An agent can determine which data sources to
query and take advantage of optimizations, such as table partitions, when it
constructs a response. The chat response contains the answer to your
question as text and code, and it includes the reasoning behind the results.
The response can also include images and charts when appropriate.

You can create a conversation with a data agent, or a direct conversation with
one or more data sources. When you create a direct conversation, the
[Conversational Analytics API](https://docs.cloud.google.com/gemini/docs/conversational-analytics-api/overview)
interprets your question without the context and processing instructions offered
by a data agent.

## Before you begin

1.
   [Verify that billing is enabled for your Google Cloud project](https://docs.cloud.google.com/billing/docs/how-to/verify-billing-enabled#confirm_billing_is_enabled_on_a_project).

2.


   Enable the BigQuery, Gemini Data Analytics, and Gemini for Google Cloud APIs.


   **Roles required to enable APIs**


   To enable APIs, you need the Service Usage Admin IAM
   role (`roles/serviceusage.serviceUsageAdmin`), which
   contains the `serviceusage.services.enable` permission. [Learn how to grant
   roles](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).

   [Enable the APIs](https://console.cloud.google.com/flows/enableapi?apiid=bigquery.googleapis.com,geminidataanalytics.googleapis.com,cloudaicompanion.googleapis.com)

### Required roles

To create conversations, you must have one of the following
[Conversational Analytics API IAM roles](https://docs.cloud.google.com/gemini/docs/conversational-analytics-api/access-control):

- To view and create conversations with any data agent that has been shared with you, you must have the Gemini Data Analytics Data Agent User (`roles/geminidataanalytics.dataAgentUser`) role and the Gemini for Google Cloud User (`roles/cloudaicompanion.user`) role at the project level.
- To create a direct conversation, you must have the Gemini Data Analytics Stateless Chat User (`roles/geminidataanalytics.dataAgentStatelessUser`) role.

Additionally, in the following situations, you must have the following roles:

- If a data agent uses a data table as a knowledge source, you must have the BigQuery Data Viewer (`roles/bigquery.dataViewer`) role on that table.
- If a data table uses [column-level access
  control](https://docs.cloud.google.com/bigquery/docs/column-level-security-intro), you need the Fine-Grained Reader (`roles/datacatalog.categoryFineGrainedReader`) role on the appropriate policy tag. For more information, see [Roles used with column-level access
  control](https://docs.cloud.google.com/bigquery/docs/column-level-security-intro#roles).
- If a data table uses [row-level access
  control](https://docs.cloud.google.com/bigquery/docs/row-level-security-intro), you must have the role-level access policy on that table. For more information, see [Create or
  update row-level access
  policies](https://docs.cloud.google.com/bigquery/docs/managing-row-level-security#create-policy).
- If a data table uses [data
  masking](https://docs.cloud.google.com/bigquery/docs/column-data-masking-intro), you need the Masked Reader (`roles/bigquerydatapolicy.maskedReader`) role on the appropriate data policy. For more information, see [Roles for querying masked
  data](https://docs.cloud.google.com/bigquery/docs/column-data-masking-intro#roles_for_querying_masked_data).

If you don't have appropriate roles on the source data tables used by the data
agent, the system returns the following error when you chat with the data agent:

    Schema_Resolution: Access Denied

## Best practices

Conversational analytics automatically runs queries on your behalf to answer
your questions. Consider the following factors that might increase query cost:

- Large table sizes
- Use of data joins in queries
- Frequent calls to AI functions within queries

## Create conversations

You can create a conversation with an existing customized data agent.
Alternatively, for quick, one-off questions, you can create a conversation
directly with a single data source.

### Create a conversation with a data agent

To create a conversation with a data agent, you first
[create a data agent](https://docs.cloud.google.com/bigquery/docs/create-data-agents) and publish it. You
can also initiate a conversation with agents that others share with you.

To create a conversation with an existing data agent in the Google Cloud console,
follow these steps:

1. Go to the BigQuery **Agents** page.

   [Go to Agents](https://console.cloud.google.com/bigquery/agents_hub)
2. Select the **Agent Catalog** tab.

3. From either the **My agents** or **Shared by others in your organization**
   section, click the agent card of the agent that you want to chat with.

4. Click **Start a Conversation**. A new chat panel opens.

### Create a direct conversation with a data source

To create a conversation with a data source in the Google Cloud console, select one
of the following options:

### Agents page

To create a direct conversation with a data source from the **Agents**
page, follow these steps:

1. Go to the BigQuery **Agents** page.

   [Go to Agents](https://console.cloud.google.com/bigquery/agents_hub)
2. On the **Conversations** tab, on the **Chat with your data** pane, click
   **Data sources**.

3. Select one or more data sources and click **Create conversation**.

### BigQuery Editor

To create a direct conversation with a data source from the **BigQuery**
page, follow these steps:

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click
   **Explorer**.

3. In the **Explorer** pane, expand your project, click **Datasets**, and
   then select a dataset. The dataset overview opens.

4. Select a data source, such as a table, view, or graph. The resource
   opens.

5. In the menu bar, click **Create conversation**.

6. Optional: To create a new conversation with your query results when you
   [run a query](https://docs.cloud.google.com/bigquery/docs/running-queries#query-settings),
   click **Create conversation** in the **Query results** pane.
   The data source is the temporary table of
   [cached results](https://docs.cloud.google.com/bigquery/docs/cached-results) that typically persists
   for 24 hours. After the cached results expire, you can't ask
   questions about the data.

### Create a data agent from a conversation

1. From within a conversation's **Data** pane, in the **Quick Actions** section, click **Create Agent**.
2. Follow the steps to [create an
   agent](https://docs.cloud.google.com/bigquery/docs/create-data-agents#create-a-data-agent).

## Have a conversation

In the **Ask a question** field, enter a question for the data agent. You
can also click one of the Gemini-suggested questions to get
started.

The Conversational Analytics API processes your question and returns the
results. To see each step the data agent took to provide the answer
to your question, click **Show reasoning**.

![How to open the **Show reasoning** results](https://docs.cloud.google.com/static/bigquery/images/ca-show-reasoning.png)

To see information about how the results were calculated, click

**How was this calculated?**

The **Summary** section includes the generated
query followed by the query result. You can optionally open the query in
the query editor.

![The API's calculation details, including the generated query and the
query result.](https://docs.cloud.google.com/static/bigquery/images/ca-how-calculated.png)

When appropriate for the data, the response provides images, charts, tables,
and other visualizations.

## Manage conversations

You can open, rename, or delete a conversation on the **Agents** page, and
manage conversations in BigQuery Studio Explorer.

### Open an existing conversation

1. In the Google Cloud console, go to the BigQuery **Agents** page.

   [Go to Agents](https://console.cloud.google.com/bigquery/agents_hub)
2. On the **Conversations** tab, in the conversations list, click the
   conversation you want to open.

### Rename a conversation

1. In the Google Cloud console, go to the BigQuery **Agents** page.

   [Go to Agents](https://console.cloud.google.com/bigquery/agents_hub)
2. On the **Conversations** tab, in the conversations list, click the
   conversation you want to rename.

3. Click
   **View actions** \> **Rename**.

4. In the **Rename conversation** dialog, enter a new name for the conversation
   in the **Conversation name** field.

5. Click **Rename**.

### Delete a conversation

Results from questions in a conversation persist even if the underlying data
sources are deleted. To delete a conversation and all the results that it
contains, follow these steps:

1. In the Google Cloud console, go to the BigQuery **Agents** page.

   [Go to Agents](https://console.cloud.google.com/bigquery/agents_hub)
2. On the **Conversations** tab, in the conversations list, click the
   conversation you want to delete.

3. Click
   **View actions** \> **Delete**.

4. In the **Delete conversation?** dialog, click **Delete**.

If you don't update a conversation for 180 days, then BigQuery
deletes it automatically.

### Manage conversations using BigQuery Studio Explorer

Manage conversations using BigQuery Studio Explorer. This conversation
list provides a central place to search for, open, or create conversations. You
can also copy the conversation ID or refresh the conversations list.

To manage your conversations, follow these steps:

1. Go to the BigQuery Studio Explorer page.

   [Go to Explorer](https://console.cloud.google.com/bigquery/explorer)
2. In the **Explorer** pane, expand a project name.

3. Click **Conversations**.

   1. To filter the conversation list, enter a property name or value in the filter field.
   2. To open a conversation, click **View actions** \> **Open**.
   3. To copy a conversation ID, click **View actions** \> **Copy ID**.
   4. To create a conversation, in the menu bar, click **Create conversation**.
   5. To refresh the list, in the menu bar, click **Refresh**.

## Locations

Conversational analytics operates globally; you can't choose which region to
use. Your conversations might not be stored in the same region as their
data sources.

## What's next

- Learn about [Conversational analytics in
  BigQuery](https://docs.cloud.google.com/bigquery/docs/conversational-analytics).
- Learn about the [Conversational Analytics API](https://docs.cloud.google.com/gemini/docs/conversational-analytics-api/overview).
- [Create data agents](https://docs.cloud.google.com/bigquery/docs/create-data-agents).