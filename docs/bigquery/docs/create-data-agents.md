# Create data agents

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

This document describes how to create, edit, manage, and delete data agents in
BigQuery.

In BigQuery, you can have
[conversations](https://docs.cloud.google.com/bigquery/docs/ca/create-conversations)
with data agents to ask questions about BigQuery data using
natural language. Data agents contain table metadata and use-case-specific query
processing instructions that define the best way to answer user questions about
a set of knowledge sources, such as tables, views, or user-defined functions
(UDFs) that you select.

## Before you begin

1.
   [Verify that billing is enabled for your Google Cloud project](https://docs.cloud.google.com/billing/docs/how-to/verify-billing-enabled#confirm_billing_is_enabled_on_a_project).

2.


   Enable the BigQuery, Gemini Data Analytics,
   Gemini for Google Cloud, and Knowledge Catalog APIs.


   **Roles required to enable APIs**


   To enable APIs, you need the Service Usage Admin IAM
   role (`roles/serviceusage.serviceUsageAdmin`), which
   contains the `serviceusage.services.enable` permission. [Learn how to grant
   roles](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).

   [Enable the APIs](https://console.cloud.google.com/flows/enableapi?apiid=bigquery.googleapis.com,geminidataanalytics.googleapis.com,cloudaicompanion.googleapis.com,dataplex.googleapis.com)

### Required roles

To work with data agents, you must have Identity and Access Management permissions that match
your use case. The following sections list required roles based on whether you
are creating and publishing agents, provisioning agents in
Gemini Enterprise, or discovering and using agents. For more
information, see [Conversational Analytics API Identity and Access Management
roles](https://docs.cloud.google.com/gemini/docs/conversational-analytics-api/access-control).

- **Create, edit, publish, share, and delete agents:**
  - To create data agents in a project: Gemini Data Analytics Data Agent Creator (`roles/geminidataanalytics.dataAgentCreator`) on the project. This role automatically grants you the Gemini Data Analytics Data Agent Owner role on the data agents that you create.
  - To edit, share, or delete a data agent: Gemini Data Analytics Data Agent Owner (`roles/geminidataanalytics.dataAgentOwner`) on the agent or project.
  - To edit a data agent in a project: Gemini Data Analytics Data Agent Editor (`roles/geminidataanalytics.dataAgentEditor`) on the project.
  - To view data agents in a project: Gemini Data Analytics Data Agent Viewer (`roles/geminidataanalytics.dataAgentViewer`) on the project.
- **Provision agents in Gemini Enterprise:**
  - To make a published agent available to users in Gemini Enterprise, you need permissions to [register and
    manage A2A
    agents](https://docs.cloud.google.com/gemini/enterprise/docs/register-and-manage-an-a2a-agent) in the Gemini Enterprise administration console.
- **Discover and use agents:**
  - To chat with data agents: Gemini Data Analytics Data Agent User (`roles/geminidataanalytics.dataAgentUser`).
  - To view all data agents in the project: Gemini Data Analytics Data Agent Viewer (`roles/geminidataanalytics.dataAgentViewer`).
- **Add knowledge sources to an agent:** Data Catalog Viewer (`roles/datacatalog.viewer`) on the project.

To have conversations,
see the [required roles for conversations](https://docs.cloud.google.com/bigquery/docs/create-conversations#required_roles).

To work with BigQuery resources, such as viewing tables or
running queries, see [BigQuery
roles](https://docs.cloud.google.com/bigquery/docs/access-control#bigquery-roles).

## Best practices

Conversational analytics automatically runs queries on your behalf to answer
your questions. Consider the following factors that might increase query cost:

- Large table sizes
- Use of data joins in queries
- Frequent calls to AI functions within queries

### Generate insights

You can optionally [generate data insights](https://docs.cloud.google.com/dataplex/docs/data-insights) in
Knowledge Catalog for any table that you want to use as a knowledge
source.

Generated insights provide table metadata that the data agent can use to
help generate responses to your questions.

If you don't generate insights beforehand, the system automatically generates
them when you select a table as a knowledge source while creating a data agent.

## Work with the sample data agent

If you're unfamiliar with configuring agents for conversational analytics, you
can optionally view the predefined sample agent generated for every
Google Cloud project. You can chat with it and view its parameters to see how it
was created, but you can't modify it.

To view the sample agent, do the following:

1. In the Google Cloud console, go to the BigQuery **Agents** page.

   [Go to Agents](https://console.cloud.google.com/bigquery/agents_hub)
2. Select the **Agent catalog** tab.

3. Under the section **Sample agents by Google**, click the sample agent card.

## Create a data agent

The following sections describe how to create a data agent.

After you create an agent, you can [edit its settings](https://docs.cloud.google.com/bigquery/docs/create-data-agents#edit-agent).

> [!NOTE]
> **Note:** If you're in a conversation with a data source, you can also [create
> an agent](https://docs.cloud.google.com/bigquery/docs/create-conversations#create-agent-from-conversation) from that conversation.

### Initial steps

1. In the Google Cloud console, go to the BigQuery **Agents** page.

   [Go to Agents](https://console.cloud.google.com/bigquery/agents_hub)
2. Select the **Agent catalog** tab.

3. Click **New agent** . The **New agent** page opens.

4. In the **Editor** section, in the **Agent name** field, type a descriptive
   name for the data agent---for example, `Q4 sales data` or `User activity
   logs`.

5. In the **Agent description** field, type a description of the data agent. A
   good description explains what the agent does, what data it uses, and helps
   you know when this is the right data agent to chat with---for example,
   `Ask questions about customer orders and revenue`.

6. In the **Knowledge sources** section, click **Add source** . The **Add
   knowledge source** page opens.

7. In the **Recents** section, select any tables, views, graphs, or UDFs that
   you want to use as knowledge sources. UDFs are prefixed with an 'fx'
   indicator in the Google Cloud console.

8. To view additional knowledge sources, select
   **Show more**.

9. Optional: Add a knowledge source that isn't listed in the **Recents**
   section:

   1. In the **Search** section, type the source name into the
      **Search for tables** field, and then press **Enter**. The source name
      doesn't need to be exact.

   2. In the **Search results** section, select one or more sources.

10. Click **Add**. The new agent page reopens.

### Customize table and field descriptions

To improve data agent accuracy, you can optionally provide additional table
metadata. Only the data agent uses this metadata, and it doesn't affect the
source table.

Follow these best practices when you create a table and field descriptions:

- Use these descriptions as a guide to understand how the data agent
  understands the schema. If the descriptions suggested by the agent are
  correct, you can accept them.

- If the data agent doesn't show an understanding of the schema after you
  configure these descriptions, then manually adjust the descriptions to
  provide the correct information.

Follow these steps to configure table and field descriptions:

1. In the **Knowledge sources** section, click the **Customize** link for a
   table.

2. Create a table description. You can type a description in the
   **Table Description** field or accept the suggestion from Gemini.

3. In the **Fields** section, review the Gemini-suggested field
   descriptions.

4. Select any field descriptions that you want to accept and click
   **Accept suggestions** . Select any descriptions that you want to reject and
   click **Reject suggestions**.

5. Manually edit any field description by clicking **Edit** next to the
   field. The **Edit field** pane opens.

   1. In the **Description** field, type a field description.
   2. To save the field description, click **Update**.
6. To save the description and field updates, click **Update**. The new agent
   page reopens.

7. Repeat these steps for each table that needs customization.

### Create agent instructions

The agent should understand context for user questions without needing any
custom instructions. Create custom instructions for the agent only if you need
to change the agent's behavior or improve the context in ways that aren't
already supported by other context features---for example, custom table and field
metadata, or verified queries.

In the **Instructions** section, type instructions for the data agent in the
**Agent instructions** field. Because the data agent uses these instructions to
understand the context for user questions and to provide answers, make the
instructions as clear as possible.

If you don't get a satisfactory answer from the agent, then add structured
context such as descriptions, examples or glossary terms. If you still don't
get a satisfactory answer, add custom instructions like the examples in the
following table.

For even more examples of instructions, click **Show examples**.

| Information type | Description | Examples |
|---|---|---|
| Key fields | The most important fields for analysis. | "The most important fields in this table are: Customer ID, Product ID, Order Date." |
| Filtering and grouping | Fields that the agent should use to filter and group data. | "When a question is about a timeline or 'over time,' always use the order_created_date column." "When someone says 'by product,' group by the product_category column." |
| Default filtering | Fields to filter on by default. | "Unless stated otherwise, always filter the data on order_status = 'Complete'." |
| Synonyms and business terms | Alternative terms for key fields. | "If someone asks about 'Revenue' or 'Sales', use the total_sale_amount column." "We consider 'loyal' customers to be those with purchase_count \> 5." |
| Excluded fields | Fields that the data agent should avoid using. | "Never use these fields: Transaction Date Derived, City Derived." |
| Join relationships | How two or more tables are related to each other, and which columns are used to join them. The agent must use standard SQL JOINs on column pairs to combine data. See the example column. | **Customer Activity** - `order_items.user_id` = `users.id` (to link sales to customers) - `events.user_id` = `users.id` (to link website activity to logged-in customers) |

### Create verified queries

An agent uses verified queries in two ways:

- If an agent can use a verified query to answer a question that you ask it, to ensure a trustworthy answer, the agent invokes the query exactly as written.
- If the agent can't use the verified query to answer a question, it still uses the query as a reference to understand the data and the best practices for querying it.

You can select verified queries from a list generated by the system, or create
your own.

> [!NOTE]
> **Note:** To create a parameterized verified query, see [Create parameterized verified queries](https://docs.cloud.google.com/bigquery/docs/create-data-agents#create-param-verified-queries).

To create a verified query for the data agent, formerly known as a
*golden query*, do the following:

1. Select one or more Gemini-suggested verified queries:

   1. In the **Verified Queries** section, click **Review suggestions** . The **Review suggested verified queries** page opens.
   2. Review the suggested verified queries. Select any that apply to your use case.
   3. Click **Add**. The new agent page reopens.
2. To create your own verified query, click **Add query** . The **Add verified
   query** page opens.

   1. In the **Question** field, type the user question that the verified query answers.
   2. Click **Generate SQL** to have Gemini generate a verified query that corresponds to the user question that you specified.
   3. Modify the verified query if you choose.
   4. Click **Run** and verify that the query returns the results that you expect.
   5. Click **Add**. The new agent page reopens.
3. Repeat these steps as needed to create additional verified queries.

### Create parameterized verified queries

Parameterized verified queries extract values from a user's question for the
conversational analytics agent and provide tailored results.

Analysts and builders can create reusable SQL templates that contain
placeholders for these values. The templates dynamically substitute parameters
at runtime to answer a wider range of user questions than do regular verified
queries.

When a user asks a question that matches the template's pattern, the
conversational analytics agent extracts parameter values from the question---
for example, product name, region, and date. It then injects these values into
the parameterized SQL query. Matching responses from the query template appear
as **verified**.

Parameterized verified queries significantly enhance the power and flexibility
of verified queries. They ensure consistent, trusted answers across various
inputs and reduce the number of individual queries to maintain. For more
information, see the following:

- To create a parameterized verified query, see [Create parameterized verified queries](https://docs.cloud.google.com/bigquery/docs/create-data-agents#create-param-verified-queries).
- To learn about parameterized queries in general, see [Run parameterized queries](https://docs.cloud.google.com/bigquery/docs/parameterized-queries).

#### How it works

An expert, such as a data analyst, defines a verified query by using a template
question---for example, "What were the sales for `@product` in `@region`?" Then,
the expert creates or modifies the verified query using SQL parameters, as
the following example shows:

    SELECT * FROM sales WHERE region = @region AND product = @product

After the verified query is saved, a user can ask the conversational analytics
agent a question in natural language; for example, "What were the sales for
laptops in North America?"

To answer the user's question, the conversational agent performs the following
steps:

1. Matches the question to the pattern associated with the parameterized verified query. The agent uses [natural language understanding
   (NLU)](https://wikipedia.org/wiki/Natural_language_understanding) to identify and extract the values for `@region` (North America) and `@product` (laptops) from the user's question.
2. Substitutes the extracted values into the `@region` and `@product` placeholders in the SQL template.
3. Runs the complete SQL query; for example, `SELECT * FROM sales WHERE
   region = 'North America' AND product = 'Laptops'`.
4. Returns the results to the user. Matches are always marked as **verified**.

#### Tips for creating effective parameterized queries

- **Use clear parameter names** . Use descriptive names for parameters---for example, `@start_date` instead of `@d1`.
- **Create detailed parameter descriptions** . The large language model (LLM) for conversational analytics uses parameter descriptions to identify the parameters and their values from user questions. For example, `num_enrollments` is an effective parameter name, but `number of student
  enrollments from ages 5-14` is a parameter description that gives more context about the query.
- **Ensure consistent data typing**. Ensure that the data types expected by the SQL query match the data types likely to be extracted from the user's question.
- **Provide a well-defined scope**. Create templates for common and important question patterns where the query construction is complex, or the logic is unintuitive. Doing so helps the LLM return optimal results.
- **Test thoroughly**. Test with various natural language phrasings to ensure that parameters are extracted correctly.

#### Create a parameterized verified query

You can select verified queries from a list generated by the system, or you can
create your own.

Before you create or modify a query, draft the query, considering the natural
language pattern and your question. For example, if you ask "Do we know the
total stock for organic bananas in the US-EAST warehouse?," you can rewrite the
question as the parameterized verified query as "What is the total stock for
@product in the @region warehouse?" The conversational analytics agent turns
this question into a SQL query that you update with default values.

To create a parameterized verified query for a data agent, you can either
create a new query when you create a new agent, or you can edit an existing
verified query for a new or existing agent.

The following instructions use a sample verified query to configure with
parameters.

##### Select an existing Gemini-suggested verified query

1. In the **Verified Queries** section of a new or existing agent, click **Review suggestions** . The **Review suggested verified queries** page opens.
2. Select the checkbox next to a suggested verified query.
3. In the query window, click **Show more** to expand the query description.
4. To open the existing query, click **Edit**.
5. To finish configuring the query, see [Configure the parameters for the
   verified query](https://docs.cloud.google.com/bigquery/docs/create-data-agents#configure-query-parameters).

##### Create an agent and then create a verified query

1. See [Initial steps](https://docs.cloud.google.com/bigquery/docs/create-data-agents#initial-steps) and continue through the rest of the configuration steps to **Verified queries.**
2. In the Google Cloud console, in the **Verified queries** section of the new agent, click **Add query** . The **Add verified query** page opens.
3. To finish configuring the query, see [Configure the parameters for the
   verified query](https://docs.cloud.google.com/bigquery/docs/create-data-agents#configure-query-parameters).

##### Configure the parameters for the verified query

1. In the **Question** field, type the user question answered by the verified query.
2. To specify parameters, use the `@` symbol followed by a parameter name. This syntax identifies a placeholder that ingests a value from the user question. Use a natural language question that shows how the parameters will be used in user questions. For example: "What is the total stock for @product in the @region warehouse?"
3. Click **Generate SQL**. The SQL looks like the following example:

          SELECT
              SUM(stock) AS total_stock
          FROM
              inventory
          WHERE
              product_id = @product
              AND region = @region;

4. To add default values to the placeholders in the query, click
   **Manage query parameters** , and then click **Add query parameter**.

5. For the first parameter, four fields appear for **Name** , **Type,** **Value** , and **Description**.

   - For **Name** , copy `@product` from your question and paste it into this field.
   - For **Type** , select **STRING**.
   - For **Value** , enter `organic bananas`.
   - For **Description**, enter as specific a description as possible. For example, a product located in a regional warehouse.
6. For the second parameter, click **Add query parameter**.

   - For **Name** , copy `@region` from your question and paste it into this field.
   - For **Type** , select **STRING**.
   - For **Value** , enter `US-EAST`.
   - For **Description** , enter as specific a description as possible---for example, `a regional warehouse where products are located.`
7. When you have filled out fields for both parameters, click **Save**.

##### Test the parameterized verified query

1. Click **Run** and verify that the query returns the results that you expect.
2. To test the query for users in a later screen, copy the entire question field.
3. Click **Save** to exit the **Add query** screen and return to the agent **Edit** page.
4. On the agent **Edit** page, paste the question field, that you copied previously, into the **Preview** window.
   1. Replace the`@product` variable with `organic bananas`.
   2. Replace the `@region` variable with `US-EAST`.
5. Press enter. Check the result. In this case, a valid answer is the total stock number of the bananas in the US-EAST region---for example, 1,000.
6. To create or edit additional verified queries, repeat these steps as needed.

Now that you have saved the query, a user can ask the question
"Do we know the total stock for organic bananas in the US-EAST warehouse?"
Conversational analytics then does the following:

1. Matches this question to the pattern.
2. Extracts the `@product` parameter as `@product` = "organic bananas" and the `@region` parameter = "US-EAST" from the question.
3. Executes the query: `SELECT SUM(stock) AS total_stock FROM inventory WHERE product_id = 'organic bananas' AND region = 'US-EAST';`
4. Returns the calculated `total_stock`.

#### Create or review glossary terms

You can create BigQuery custom glossary terms local to an agent,
or review business glossary terms imported from Knowledge Catalog that
apply to the knowledge sources that you selected for an agent.

- Because business glossary terms from Knowledge Catalog apply globally to BigQuery resources, if you use Knowledge Catalog, [create and manage](https://docs.cloud.google.com/dataplex/docs/manage-glossaries) business glossary terms in Knowledge Catalog instead of for individual agents.
- If you need to modify business glossary terms imported from Knowledge Catalog, you must edit them in Knowledge Catalog and return to conversational analytics to see them.
- BigQuery custom glossary terms stay in BigQuery. They don't appear in Knowledge Catalog.
- If you're not using Knowledge Catalog, you can create BigQuery custom glossary terms for terms that you need to define for a specific agent.

Follow these steps to create custom glossary terms for an agent:

1. In the **Glossary** section of the agent **Editor** page, click **Add term**.
2. In the **Custom terms** section, you can edit or delete any existing custom terms.
3. To create one or more new terms, click **Create term** .
   1. Enter a **Term** , a **Definition** , and one or more **Synonyms** separated by a comma.
   2. To create the term, click **Add**.
   3. If you want to delete the new term, click **Delete**.
4. To create more custom terms, repeat these steps.

Follow these steps to view business glossary terms imported from Knowledge Catalog:

1. In the **Glossary** section of the agent **Editor** page, click **Add term**.
2. Navigate to the page section called **Imported from Dataplex**.
3. To modify imported terms in Knowledge Catalog, you must click the link "Go to Dataplex glossaries."
4. After you've modified the terms in Knowledge Catalog, you can return to the agent **Editor** page to view the modified terms.

#### Configure settings

In the **Settings** section, you can configure the following optional settings:

1. Create [labels](https://docs.cloud.google.com/bigquery/docs/labels-intro) to help you organize your
   Google Cloud resources. Labels are key-value pairs that let you group
   related objects together or with other Google Cloud resources.

   1. In the **Settings** section, click **Manage labels**.
   2. Click **Add label**.
   3. In the **key** and **value** fields, enter your key-value pair for the label.
   4. If you want to add more labels, click **Add label** again.
   5. To delete a label, click **Delete**.
   6. When you're finished, click **Add**. The new agent page reopens.
2. Optional: Set a size limit for the queries processed by the data agent. In
   the **Settings** section, type a value in the **Maximum bytes billed**
   field. You must set this limit to `10485760` or higher, otherwise you
   receive the following error message:

    Value error. In BigQuery on-demand pricing charges are
    rounded up to the nearest MB, with a minimum of 10 MB of data processed
    per query. So, max bytes billed must be set to greater or equal to
    10485760.

If you don't specify a value, `maximum bytes billed` defaults to the
project's [query usage per day quota](https://docs.cloud.google.com/bigquery/quotas#query_jobs). The
usage per day quota is unlimited unless you have specified a [custom
quota](https://docs.cloud.google.com/bigquery/docs/custom-quotas).

Continue to the next section to place the agent in draft mode or
publish the agent.

### Preview and publish the agent

1. In the **Preview** section, type an example user question in the
   **Ask a question** field, and then press **Enter** . To verify that the
   data agent returns the data that you expect, review the agent's
   response. If the response is not what you expect, change the settings in
   the **Editor** section to refine the data agent configuration until you get
   satisfactory responses. You can continue to test and modify your agent to
   refine the agent's results.

2. Click **Save**.

3. To place the data agent in draft mode, which you can re-edit later, click

   **Go back** to return to the **Agent Catalog** page. Because your agent is
   now in draft mode, it appears in the **My draft agents** section on the
   **Agent Catalog** tab.

   To publish your agent, remain on the agent creation page and proceed to the
   next step.
4. Click **Publish** to publish the data agent and make it available for use in
   the project. You can create conversations with the data agent by using
   BigQuery Studio or [Data Studio](https://docs.cloud.google.com/data-studio/conversational-analytics-data-agents).
   You can also build your own interface to chat with the data agent by using
   the Conversational Analytics API. For information on publishing to
   Gemini Enterprise, see [Publish a data agent in
   Gemini Enterprise](https://docs.cloud.google.com/bigquery/docs/create-data-agents#publish-agent-gemini-enterprise).

5. Optional: In the **Your agent has been published** dialog, click
   **Share** to share the data agent with other users.

   1. In the **Share permissions** pane, click **Add principal**.

   2. In the **New principals** field, enter one or more principals.

   3. Click the **Select a role** list.

   4. In the **Role** list, select one of the following roles:

      - Gemini Data Analytics Data Agent User (`roles/geminidataanalytics.dataAgentUser`): grants permission to chat with the data agent.
      - Gemini Data Analytics Data Agent Editor (`roles/geminidataanalytics.dataAgentEditor`): grants permission to edit the data agent.
      - Gemini Data Analytics Data Agent Viewer (`roles/geminidataanalytics.dataAgentViewer`): grants permission to view the data agent.
6. Click **Save**.

7. To return to the new agent page, click **Close** . Immediately after saving
   or publishing your agent, you can see it in the **Agent Catalog**.

## Manage data agents

You can find existing agents in the **Agent Catalog** tab, which consists of
three sections:

- **My agents**: a list of all agents that you create and publish. You can modify and share published agents with others.
- **My draft agents**: agents that you haven't published yet. You can't share draft agents.
- **Shared by others in your organization**: Agents that others create and share with you. If others grant you permissions, you can edit these shared agents.

### Edit a data agent

Follow these steps to edit a data agent:

1. Go to the BigQuery **Agents** page.

   [Go to Agents](https://console.cloud.google.com/bigquery/agents_hub)
2. Select the **Agent Catalog** tab.

3. Locate the agent card of the data agent that you want to modify.

4. To open the data agent in the agent editor, click

   **Open actions** \> click **Edit** on the agent card.

5. Edit the data agent's configuration as needed.

6. To save your changes without publishing, click **Save**.

7. To publish your changes, click **Publish** . In the **Share** dialog, you
   can either [share](https://docs.cloud.google.com/bigquery/docs/create-data-agents#share-a-data-agent) the agent with others, or click
   **Cancel**.

8. To return to the **Agents** pane, click **Go back**.

   ![Go back icon to return to the Agents page from the agent editing
   page.](https://docs.cloud.google.com/static/bigquery/images/ca-go-back.png)

### Share a data agent

Follow these steps to share a published data agent. You can't share draft
agents.

1. Go to the BigQuery **Agents** page.

   [Go to Agents](https://console.cloud.google.com/bigquery/agents_hub)
2. Select the **Agent Catalog** tab.

3. Locate the agent card of the data agent that you want to modify.

4. To open the data agent in the agent editor, click

   **Open actions** \> click **Edit** on the agent card.

5. To share the data agent with other users, click **Share**.

6. In the **Share permissions** pane, click **Add principal**.

7. In the **New principals** field, enter one or more principals.

8. Click the **Select a role** list.

9. In the **Role** list, select one of the following roles:

   - Gemini Data Analytics Data Agent User (`roles/geminidataanalytics.dataAgentUser`): gives permission to chat with the data agent.
   - Gemini Data Analytics Data Agent Editor (`roles/geminidataanalytics.dataAgentEditor`): gives permission to edit the data agent.
   - Gemini Data Analytics Data Agent Viewer (`roles/geminidataanalytics.dataAgentViewer`): gives permission to view the data agent.
10. Click **Save**.

11. To return to the agent editing page, click **Close**.

12. To return to the **Agents** pane, click
    **Go back**.

    ![Go back icon to return to the Agents page from the agent editing
    page.](https://docs.cloud.google.com/static/bigquery/images/ca-go-back.png)

#### Share a data agent in Data Studio

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

When you [publish](https://docs.cloud.google.com/bigquery/docs/create-data-agents#preview-publish) your agent and [share](https://docs.cloud.google.com/bigquery/docs/create-data-agents#share-a-data-agent) it with Data Studio users, the agent automatically appears for those users on the **Chat with your data** page in Data Studio.

You can also copy a direct link to the agent, which opens [Data Studio directly into a session with that specific BigQuery data agent](https://docs.cloud.google.com/data-studio/conversational-analytics-data-agents). You can copy the agent's dedicated URL in these ways:

- From the agent catalog: Select **Open actions** ; next, select **Copy link** , and then select **Data Studio**.
- From the agent details view: Select **Copy agent link** , and then select **Data Studio**.
- From the **Share** overflow menu: Select **Copy link to agent in Data Studio**.

### Delete a data agent

1. Go to the BigQuery **Agents** page.

   [Go to Agents](https://console.cloud.google.com/bigquery/agents_hub)
2. Select the **Agent Catalog** tab.

3. In either the **My agents** or **My draft agents** section of the **Agent Catalog**
   tab, locate the agent card of the data agent that you want to delete.

4. Click
   **Open actions** \> **Delete**.

5. In the **Delete agent?** dialog, click **Delete**.

## Publish a data agent in Gemini Enterprise

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
> **Note:** To use this feature, you must be added to the allowlist. To request access, fill out the [application
> form](https://docs.google.com/forms/d/1HIFPcPJk1g0SCPDNyVqoLWszBeXVa9qyU4fbBdW_91Y/edit#responses). To provide feedback or request support for this feature, send an email to [bqca-feedback-external@google.com](mailto:bqca-feedback-external@google.com).

The following sections describe how to publish a data agent in
Gemini Enterprise. This process typically involves collaboration
between data analysts, Gemini Enterprise administrators, and
business users. For more information about the roles required for these users,
see [Required roles](https://docs.cloud.google.com/bigquery/docs/create-data-agents#required-roles).

To publish a data agent in Gemini Enterprise, complete the
following steps:

1. Create and publish the data agent.
2. Provision the agent.
3. Discover and use the agent.

### Create and publish an agent

As a data analyst, you create, edit, and publish an agent to
Gemini Enterprise by completing the following steps:

1. [Create](https://docs.cloud.google.com/bigquery/docs/create-data-agents#create-a-data-agent) or [edit](https://docs.cloud.google.com/bigquery/docs/create-data-agents#edit-agent) your data agent in BigQuery.
2. [Publish](https://docs.cloud.google.com/bigquery/docs/create-data-agents#preview-publish) the agent. When you publish the agent, select **Gemini Enterprise** as a publishing option.
3. Copy the A2A endpoint JSON.
4. Share the A2A endpoint JSON and the list of users with your Gemini Enterprise administrator.
5. [Share](https://docs.cloud.google.com/bigquery/docs/create-data-agents#share-a-data-agent) the agent with the users and groups who need access.

### Provision an agent

As the Gemini Enterprise administrator, you can make a published
agent available to users in the Google Cloud console.

1. In the Google Cloud console, go to **Gemini Enterprise**.

   [Gemini Enterprise](https://console.cloud.google.com/gemini-enterprise/)
2. Click the name of the app where you want to register the agent.

3. Click **Agents** \>
   **Add Agents**.

4. In the **Choose an agent type** section, click **Add** for **Custom agent
   via A2A**.

5. In the **Agent card JSON** field, enter the agent card details that you
   previously received from the data analyst in JSON format. For a complete
   list of available fields, see the [Agent2Agent (A2A) Protocol
   Specification](https://a2a-protocol.org/latest/specification/). The example
   uses only the required fields.

6. Click **Preview agent details \> Next**.

7. To enable the agent to access Google Cloud resources on your behalf,
   complete the following steps:

8. Enter the **Client ID** ,
   **Client secret** , **Authorization URI** , and **Token URI** that you
   generated in the [Configure authorization
   details](https://docs.cloud.google.com/gemini/enterprise/docs/register-and-manage-an-a2a-agent#authorize-your-agent)
   section.

9. Enter the **Scopes**.

10. Click **Finish**.

11. [Share the agent](https://docs.cloud.google.com/gemini/enterprise/docs/share-custom-agents#share_an_agent)
    with the users or groups that the data analyst provides.

For more information about working with A2A agents in
Gemini Enterprise, see [Register and manage A2A
agents](https://docs.cloud.google.com/gemini/enterprise/docs/register-and-manage-an-a2a-agent).

### Discover and use an agent

You can discover and use a data agent in Gemini Enterprise using
any of these methods:

- **Manual discovery** : Locate a data agent in the [Agent Gallery](https://docs.cloud.google.com/gemini/enterprise/docs/agent-gallery) and use it with any of these methods:
  - **Browse the gallery**: Select the agent and start a dedicated chat.
  - **Direct link**: Use the agent's dedicated URL to open Gemini Enterprise directly into a session with that specific BigQuery data agent.
  - **Directed intent** : Invoke the agent by `@mention` (for example, `@sales_pipeline_agent`) in the Gemini Enterprise core chat.
  - **Seamless orchestration**: Ask a general analytical question (for example, "How is our sales pipeline trending in the last 3 months?") and Gemini Enterprise automatically routes the query to the relevant data agent.

After you've discovered the agent, you can interact with it by performing the
following steps:

- **Authenticate**: Complete the one-time OAuth sign-in to securely authenticate to BigQuery.
- **Chat**: Ask natural language questions to the agent. The requests are processed by the BigQuery conversational analytics agent, and the response is streamed back to Gemini Enterprise as text, Markdown, charts, or tables.
- **View conversation history**: Conversations are automatically saved in the history pane.

## Locations

Conversational analytics operates globally; you can't choose which region to
use.

## What's next

- Learn more about [conversational analytics in
  BigQuery](https://docs.cloud.google.com/bigquery/docs/ca/conversational-analytics).
- Learn more about the [Conversational Analytics API](https://docs.cloud.google.com/gemini/docs/conversational-analytics-api/overview).
- [Analyze data with conversations](https://docs.cloud.google.com/bigquery/docs/ca/create-conversations).
- Learn more about how the [Gemini Data Analytics Data Agent Viewer
  (`roles/geminidataanalytics.dataAgentViewer`)](https://docs.cloud.google.com/gemini/docs/conversational-analytics-api/access-control#predefined-roles) role gives permission to view the data agent.