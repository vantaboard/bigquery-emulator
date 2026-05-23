# Write queries with Gemini assistance


This document describes how to use AI-powered assistance in
[Gemini in BigQuery](https://docs.cloud.google.com/bigquery/docs/gemini-overview)
to help you query your data with SQL queries and Python code.
Gemini in BigQuery can generate and explain
queries and code, complete queries and code while you type, and fix code errors.

*** ** * ** ***

To follow step-by-step guidance for this task directly in the
Google Cloud console, click **Guide me**:

[Guide me](https://console.cloud.google.com/bigquery?walkthrough_id=bigquery--write-sql-gemini&start_index=1)

*** ** * ** ***

Gemini for Google Cloud doesn't use your prompts or its
responses as data to train its models without your express permission. For more
information about how Google uses your data, see
[How Gemini for Google Cloud uses your data](https://docs.cloud.google.com/gemini/docs/discover/data-governance).

> [!NOTE]
> **Note:** To opt in to data sharing for Gemini in BigQuery features in [Preview](https://cloud.google.com/products/#product-launch-stages), see [Help improve suggestions](https://docs.cloud.google.com/bigquery/docs/write-sql-gemini#help_improve_suggestions_2).

Only English language prompts are supported for Gemini in
BigQuery.

This document is intended for data analysts, data scientists, and data
developers who work with SQL queries and
[Colab Enterprise notebooks in BigQuery](https://docs.cloud.google.com/bigquery/docs/notebooks-introduction).
It assumes that you know how to query data in the BigQuery Studio
environment or how to work with Python notebooks to analyze
BigQuery data.

## Before you begin

1. Ensure that [Gemini in BigQuery is set up for your Google Cloud project](https://docs.cloud.google.com/bigquery/docs/gemini-set-up). This step is normally done by an administrator. Gemini in BigQuery features might be turned off or unavailable until you complete the remaining steps in this section.
2. To use [Gemini Cloud Assist](https://docs.cloud.google.com/cloud-assist/overview), to write code in the Gemini Cloud Assist pane, you must also follow the steps in [Set up Gemini Cloud Assist](https://docs.cloud.google.com/cloud-assist/set-up-gemini).
3. To use Gemini to explain and fix Python code in your
   Colab Enterprise notebooks in
   BigQuery, you must also follow the steps in
   [Set up Gemini in Colab Enterprise for a project](https://docs.cloud.google.com/colab/docs/gemini-in-colab/set-up-gemini).

4. In the Google Cloud console, on the project selector page,
   select or create a Google Cloud project.

   **Roles required to select or create a project**
   - **Select a project**: Selecting a project doesn't require a specific IAM role---you can select any project that you've been granted a role on.
   - **Create a project** : To create a project, you need the Project Creator role (`roles/resourcemanager.projectCreator`), which contains the `resourcemanager.projects.create` permission. [Learn how to grant
     roles](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).

   > [!NOTE]
   > **Note**: If you don't plan to keep the resources that you create in this procedure, create a project instead of selecting an existing project. After you finish these steps, you can delete the project, removing all resources associated with the project.

   [Go to project selector](https://console.cloud.google.com/projectselector2/home/dashboard)
5. In the Google Cloud console, go to the **BigQuery Studio** page.

   [Go to BigQuery Studio](https://console.cloud.google.com/bigquery)
6. In the BigQuery toolbar, click
   pen_spark**Gemini**.

   ![Gemini button in the BigQuery toolbar.](https://docs.cloud.google.com/static/bigquery/images/duet-ai-assistant-link.png)
7. In the list of features, ensure the following features are selected:

   - **Gemini in SQL query** list:

     - **Auto-completion** (Preview). As you type in the query editor, Gemini can suggest logical next steps that are relevant to your current query's context, or it can help you iterate on a query.
     - **Auto-generation**. You can prompt Gemini in BigQuery using a natural language comment in the BigQuery query editor to generate a SQL query.
     - **SQL generation tool**. You can enter natural language text in a tool to generate a SQL query, with options to refine query results, choose table sources, and compare results.
     - **Explanation**. You can prompt Gemini in BigQuery to explain a SQL query using natural language.
   - **Gemini in Python notebook** list:

     - **Code completion** (Preview). Gemini provides contextually appropriate recommendations that are based on content in the notebook.
     - **Code generation**. You can prompt Gemini using a natural language statement or question to generate Python code.
8. To complete the tasks in this document, ensure that you have the
   [required Identity and Access Management (IAM) permissions](https://docs.cloud.google.com/bigquery/docs/write-sql-gemini#required_permissions).

### Required roles


To get the permissions that
you need to write queries with Gemini assistance,

ask your administrator to grant you the
[Gemini for Google Cloud User](https://docs.cloud.google.com/iam/docs/roles-permissions/cloudaicompanion#cloudaicompanion.user) (`roles/cloudaicompanion.user`) IAM role on the project.


For more information about granting roles, see [Manage access to projects, folders, and organizations](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).


This predefined role contains

the permissions required to write queries with Gemini assistance. To see the exact permissions that are
required, expand the **Required permissions** section:


#### Required permissions

The following permissions are required to write queries with Gemini assistance:

- `cloudaicompanion.entitlements.get`
- `cloudaicompanion.instances.completeTask`
- Explain SQL queries: `cloudaicompanion.companions.generateChat`
- Complete SQL or Python code: `cloudaicompanion.instances.completeCode`
- Generate SQL or Python code: `cloudaicompanion.instances.generateCode`


You might also be able to get
these permissions
with [custom roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles) or
other [predefined roles](https://docs.cloud.google.com/iam/docs/roles-overview#predefined).

For more information about IAM roles and permissions in
BigQuery, see
[Introduction to IAM](https://docs.cloud.google.com/bigquery/docs/access-control).

<br />


## Generate a SQL query

> [!CAUTION]
> As an early-stage technology, Gemini for Google Cloud
> products can generate output that seems plausible but is factually incorrect. We recommend that you
> validate all output from Gemini for Google Cloud products before you use it.
> For more information, see
> [Gemini for Google Cloud and responsible AI](https://docs.cloud.google.com/gemini/docs/discover/responsible-ai).

To generate a SQL query based on your data's schema, you can provide
Gemini in BigQuery with a natural language
statement or question, also known as a *prompt*. You can also
browse prompt recommendations from Gemini.
Even if you're starting with no
code, a limited knowledge of the data schema, or only a basic knowledge of
GoogleSQL syntax, Gemini in BigQuery can
generate SQL that can help you explore your data.

### Use the SQL generation tool

The SQL generation tool lets you use natural language to generate a SQL query
about your recently viewed or queried tables. You can also use the tool to
modify an existing query, and to manually specify the tables for which you want
to generate SQL.

To use the SQL generation tool, follow these steps:

1. In the Google Cloud console, go to the **BigQuery Studio** page.

   [Go to BigQuery Studio](https://console.cloud.google.com/bigquery)
2. Next to the query editor, click
   pen_spark **SQL generation tool**.

   ![SQL generation tool button in the BigQuery query editor.](https://docs.cloud.google.com/static/bigquery/images/help-me-code.png)
3. In the **Generate SQL with Gemini** dialog, you have the following options:

   - Enter a natural language
     prompt about a table that you recently viewed or queried. For example,
     if you recently viewed
     [`bigquery-public-data.austin_bikeshare.bikeshare_trips` table](https://console.cloud.google.com/bigquery?ws=!1m5!1m4!4m3!1sbigquery-public-data!2saustin_bikeshare!3sbikeshare_trips), you might enter the following:

         Show me the duration and subscriber type for the ten longest trips.

   - Click one of the Gemini recommended prompts
     ([Preview](https://cloud.google.com/products/#product-launch-stages)).
     The prompt
     is copied to the **Generate SQL with Gemini** dialog.

4. Click **Generate**.

   The generated SQL query is similar to the following:

       SELECT
           subscriber_type,
           duration_sec
         FROM
             `bigquery-public-data.san_francisco_bikeshare.bikeshare_trips`
       ORDER BY
           duration_sec DESC
       LIMIT 10;

   > [!NOTE]
   > **Note:** Gemini in BigQuery might suggest different syntax each time that you enter the same prompt.

5. Review the generated SQL query and take any of the following actions:

   - To accept the generated SQL query, click **Insert** to insert the statement into the query editor. You can then click **Run** to execute the suggested SQL query.
   - To edit your prompt, click **Edit** and then modify or replace your initial prompt. After you've edited your prompt, click **Update** to generate a new query.
   - To update the table sources that were used as context to generate the suggested SQL query, click **Edit Table Sources** , select the appropriate checkboxes, and then click **Apply**.
   - To view a natural language summary of the generated query, click **Query
     Summary**.
   - To refine the suggested SQL query, enter any refinements in the **Refine** field, and then click **Refine** . For example, enter `limit to 1000` to limit the number of query results. To compare the changes to your query, select the **Show diff** checkbox.
   - To dismiss a suggested query, close the SQL generation tool.

#### Turn off the SQL generation tool

To learn how to turn off the SQL generation tool, see
[Turn off Gemini query assistant features](https://docs.cloud.google.com/bigquery/docs/write-sql-gemini#disable-gemini-features).

### Generate SQL from a comment

You can generate SQL in the query editor by describing the query that you want
in a comment.

1. In the Google Cloud console, go to the **BigQuery Studio** page.


   [Go to BigQuery Studio](https://console.cloud.google.com/bigquery)

   <br />

2. In the query editor, click
   **SQL query**.

3. In the query editor, write a SQL comment about a table you have recently
   viewed or queried. For example, if you recently viewed the
   [`bigquery-public-data.austin_bikeshare.bikeshare_trips` table](https://console.cloud.google.com/bigquery?ws=!1m5!1m4!4m3!1sbigquery-public-data!2saustin_bikeshare!3sbikeshare_trips), then you might write the following
   comment:

       # Show me the duration and subscriber type for the ten longest trips.

4. Press <kbd>Enter</kbd> (<kbd>Return</kbd> on macOS).

   The suggested SQL query is similar to the following:

       # Show me the duration and subscriber type for the ten longest trips

       SELECT
         duration_sec,
         subscriber_type
         AVG(duration_minutes) AS average_trip_length
       FROM
         `bigquery-public-data.austin_bikeshare.bikeshare_trips`
       ORDER BY
         duration_sec
       LIMIT 10;

5. To accept the suggestion, press <kbd>Tab</kbd>.

### Generate SQL with Gemini Cloud Assist

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
> **Note:** To request support or provide feedback for this feature, send email to [gemini-in-bigquery-feedback@google.com](mailto:gemini-in-bigquery-feedback@google.com).

You can generate a SQL query in BigQuery by using the
[**Cloud Assist** panel](https://docs.cloud.google.com/cloud-assist/chat-panel)
in the Google Cloud console.

Before you can use Gemini Cloud Assist chat to generate SQL,
you must enable Gemini Cloud Assist. For more information, see
[Set up Gemini Cloud Assist](https://docs.cloud.google.com/cloud-assist/set-up-gemini).

1. In the Google Cloud console, go to the **BigQuery Studio** page.


   [Go to BigQuery Studio](https://console.cloud.google.com/bigquery)

   <br />

2. In the query editor, click
   **SQL query** to open a new SQL query.

3. In the Google Cloud toolbar, click
   spark **Open or
   close Gemini AI chat** to open Gemini Cloud Assist chat.

   ![Gemini Cloud Assist button in the BigQuery toolbar.](https://docs.cloud.google.com/static/bigquery/images/gemini-spark.png)
4. In the **Enter a prompt** field, enter a prompt to generate a SQL query.
   For example:

       Generate a SQL query to show me the duration and subscriber type for the ten longest trips.

5. Click **Send prompt**. The response includes a SQL query similar to
   the following:

       SELECT
            subscriber_type,
            duration_sec
        FROM
            `bigquery-public-data.san_francisco_bikeshare.bikeshare_trips`
        ORDER BY
            duration_sec DESC
        LIMIT 10;
        ```

6. Review the generated SQL query.

7. To run the generated SQL query, click
   **Copy to clipboard** ,
   paste the generated code in the query editor,
   and then click **Run**.

8. If the query editor is already open, you can select one of the following
   options:

   - To see the difference between your existing query and the generated
     query, click **Preview**.

     A comparison pane opens. After you review the changes, select one of
     the following:
     - **Accept and run**: accept the changes and run the query.
     - **Accept**: accept the changes.
     - **Decline**: close the comparison pane without making changes to your existing query.
   - To replace the contents of the query editor with the generated query
     and run it, click **Apply and run**.

### Tips for SQL generation

The following tips can improve suggestions that Gemini in
BigQuery provides:

- To manually specify which tables to use, you can include the fully qualified table name in backticks (`` ` ``), such as `` `PROJECT.DATASET.TABLE` ``.
- If the column names or their semantic relationships are unclear or complex, then you can provide context in the prompt to guide Gemini towards the answer that you want. For example, to encourage a generated query to reference a column name, describe the column name and its relevance to the answer that you want. To encourage an answer that references complex terms like *lifetime value* or *gross margin*, describe the concept and its relevance to your data to improve SQL generation results.
- When you generate SQL from a comment, you can format your prompt over multiple lines by prefixing each line with the `#` character.
- Column descriptions are considered when you generate SQL queries. To improve accuracy, add column descriptions to your schema. For more information about column descriptions, see [Column descriptions](https://docs.cloud.google.com/bigquery/docs/schemas#column_descriptions) in "Specify a schema."

### Convert comments to SQL

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
> **Note:** To request support or provide feedback for this feature, send email to [gemini-in-bigquery-feedback@google.com](mailto:gemini-in-bigquery-feedback@google.com).

You can use comments as prompts to create SQL queries that help you explore your
data in BigQuery. You can embed comments containing natural
language prompts that describe the information that you want from your data.
Gemini responds with SQL that you can then compare or insert into
your query. Natural language expressions can help you iterate on and transform
your SQL code. Natural language expressions can also help with SQL syntax such
as timestamps and window functions.

To use natural language SQL generation, follow these steps:

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the **BigQuery Studio** query editor, click
   pen_spark and ensure Gemini
   SQL **Auto-generation** is enabled.

   ![SQL assistant link in BigQuery query editor.](https://docs.cloud.google.com/static/gemini/images/gemini-assistant-link-disabled.png)
3. In the BigQuery query editor, enter a SQL query containing a
   natural language prompt enclosed in a comment in the format of `/* natural
   language text */` about a table that you recently viewed or queried.
   Gemini in BigQuery uses the metadata of
   recently queried tables in an effort to find appropriate data, so you can
   help guide responses by querying a table. For best results, your natural
   language prompt should be specific to SQL syntax and your data,
   and not a general expression such as "optimize my query."

   For example, if you recently queried
   [`bigquery-public-data.austin_bikeshare.bikeshare_trips`
   table](https://console.cloud.google.com/bigquery?ws=!1m5!1m4!4m3!1sbigquery-public-data!2saustin_bikeshare!3sbikeshare_trips), you might enter the following:

       SELECT
           subscriber_type,
           /* the name of the day of week of the trip start ordered longest to
            shortest trip with the trip's duration */
       FROM
           `bigquery-public-data`.`austin_bikeshare`.`bikeshare_trips`
       LIMIT 10;

4. Highlight the SQL query, including the natural language expression, that you
   want Gemini to convert. In the previous example, you would
   highlight the entire SQL sample.

   ![Gemini icon highlighted in margin of BigQuery query editor and the full SQL statement selected.](https://docs.cloud.google.com/static/bigquery/images/gemini-query-editor.png)
5. To generate SQL code, in the margin or the query editor you can click
   **Gemini** , and then click
   pen_spark **Convert comments to SQL**.

6. Review the generated SQL. The **Transform SQL with Gemini** output shows
   the difference between the original text and the generated text. The
   generated SQL query should be similar to the following:

       SELECT
         subscriber_type,
         FORMAT_TIMESTAMP('%A', start_time) AS day_of_week,
         duration_minutes
       FROM
         `bigquery-public-data`.`austin_bikeshare`.`bikeshare_trips`
       ORDER BY
         duration_minutes DESC
       LIMIT
         10;

7. To copy the query to the query editor, click **Insert**. Your previous
   statement, including your natural language prompt, appears in comments and
   the generated SQL code is copied to the edit pane where you can run or edit
   it. You can also select one of the following:

   - **Refine**: to prompt Gemini to modify the generated SQL
   - **Edit Table Sources**: to select a different table
   - **Query Summary**: to have Gemini provide a summary of the SQL query.

## Complete a SQL query

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

SQL completion attempts to provide contextually appropriate recommendations that
are based on content in the query editor. As you type, Gemini
can suggest logical next steps that are relevant to your current query's
context, or it can help you iterate on a query.

To try SQL completion with Gemini in BigQuery,
follow these steps:

1. In the Google Cloud console, go to the **BigQuery Studio** page.


   [Go to BigQuery Studio](https://console.cloud.google.com/bigquery)

   <br />

2. In the query editor, copy the following:

       SELECT
         subscriber_type
         , EXTRACT(HOUR FROM start_time) AS hour_of_day
         , AVG(duration_minutes) AS avg_trip_length
       FROM
         `bigquery-public-data.austin_bikeshare.bikeshare_trips`

   An error message states that `subscriber_type` isn't grouped or
   aggregated. It's not uncommon to need some help getting a query just right.
3. At the end of the line for `subscriber_type`, press <kbd>Space</kbd>.

   The suggested refinements to the query might end in text that's similar to
   the following:

       GROUP BY
         subscriber_type, hour_of_day;

   You can also press <kbd>Enter</kbd> (<kbd>Return</kbd> on macOS) to generate
   suggestions.
4. To accept the suggestion, press <kbd>Tab</kbd>, or hold the
   pointer over the suggested text and click through alternate suggestions. To
   dismiss a suggestion, press <kbd>ESC</kbd> or continue typing.

   ![Navigation buttons for SQL suggestions.](https://docs.cloud.google.com/static/bigquery/images/navigate-sql-suggestions.png)

## Explain a SQL query

You can prompt Gemini in BigQuery to explain a
SQL query in natural language. This explanation can help you understand a
query whose syntax, underlying schema, and business context might be difficult
to assess due to the length or complexity of the query.

To get an explanation for a SQL query, follow these steps:

1. In the Google Cloud console, go to the **BigQuery Studio** page.


   [Go to BigQuery Studio](https://console.cloud.google.com/bigquery)

   <br />

2. In the query editor, open or paste a query that you want explained.

3. Highlight the query that you want Gemini in
   BigQuery to explain.

4. Click astrophotography_mode **Gemini** ,
   and then click **Explain this query**.

   ![The Explain this query icon and text highlighted in the
   BigQuery query editor.](https://docs.cloud.google.com/static/bigquery/images/duet-ai-explain.png)

   The SQL explanation appears in the **Cloud** panel.

## Fix and explain SQL errors

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
> **Note:** To request support or provide feedback for this feature, send email to [gemini-in-bigquery-feedback@google.com](mailto:gemini-in-bigquery-feedback@google.com).

You can use Gemini in BigQuery
to fix and explain errors in your SQL queries. To fix an error in the text of
your query before you run it, follow these steps:

1. Highlight the text that contains the error.

2. Click **Refine**
   and then
   **Fix it**.

3. The Gemini Cloud Assist pane opens and shows a suggested
   change to your query to fix the error.

4. Click **Apply \& run** to make the change, or **Preview** to open a comparison
   pane that shows the difference between your query and the suggested one.

To fix an error that appears after you run a query, follow these steps:

1. Next to the error in the **Results** pane, click **Gemini suggested fixes**.

2. The Gemini Cloud Assist pane opens and shows a suggested
   change to your query to fix the error.

3. Click **Apply \& run** to make the change, or **Preview** to open a comparison
   pane that shows the difference between your query and the suggested one.

## Generate Python code

You can ask Gemini in BigQuery to generate
Python code with a natural language statement or question.
Gemini in BigQuery responds with one or more
Python code suggestions, pulling in relevant table names directly from your
BigQuery project, resulting in personalized, executable Python
code.

### Use the Python code generation tool

In the following example, you generate code for a BigQuery
public dataset, `bigquery-public-data.ml_datasets.penguins`.

1. In the Google Cloud console, go to the **BigQuery Studio** page.

   [Go to BigQuery Studio](https://console.cloud.google.com/bigquery)
2. In the tab bar of the query editor, click the

   drop-down arrow next to
   **SQL query** , and then click
   **Notebook**.

   The new notebook opens, containing cells that show example queries against
   the `bigquery-public-data.ml_datasets.penguins` public dataset.
3. To insert a new code cell, in the toolbar, click
   **Code** . The new code cell contains
   the message **Start coding or generate with AI.**

4. In the new code cell, click **generate**.

5. In the **Generate** editor, enter the following natural language prompt:

       Using bigquery magics, query the `bigquery-public-data.ml_datasets.penguins` table

6. Press <kbd>Enter</kbd> (<kbd>Return</kbd> on macOS).

   The suggested Python code is similar to the following:

       %%bigquery
       SELECT *
       FROM `bigquery-public-data.ml_datasets.penguins`
       LIMIT 10

7. To run the code, press
   **Run cell**.

### Generate Python code with Gemini Cloud Assist

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
> **Note:** To request support or provide feedback for this feature, send email to [gemini-in-bigquery-feedback@google.com](mailto:gemini-in-bigquery-feedback@google.com).

You can use [Gemini Cloud Assist](https://docs.cloud.google.com/cloud-assist/overview)
in the Google Cloud console to generate Python code in BigQuery.
Before you can use Gemini Cloud Assist to generate code,
you must enable Gemini Cloud Assist. For more information, see
[Set up
Gemini Cloud Assist](https://docs.cloud.google.com/cloud-assist/set-up-gemini).

1. In the Google Cloud console, go to the **BigQuery Studio** page.


   [Go to BigQuery Studio](https://console.cloud.google.com/bigquery)

   <br />

2. In the tab bar of the query editor, click the

   drop-down arrow next to
   **SQL query** , and then click
   **Notebook**.

3. In the Google Cloud toolbar, click
   spark **Open or
   close Gemini AI chat** to open Gemini Cloud Assist chat.

   ![Gemini button in the BigQuery toolbar.](https://docs.cloud.google.com/static/bigquery/images/gemini-spark.png)
4. In the **Enter a prompt** field, enter a prompt to generate Python code.
   For example:

       Generate python code to query the `bigquery-public-data.ml_datasets.penguins`
       table using bigquery magics

5. Click **Send prompt**.
   Gemini returns Python code similar to the following:

       %%bigquery
       SELECT *
       FROM `bigquery-public-data.ml_datasets.penguins`
       LIMIT 10

6. Review the generated Python code.

7. To run the Python code, click
   **Copy to clipboard** and then paste the generated code in the query editor,
   and then click **Run**.

### Generate BigQuery DataFrames code

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
> **Note:** To request support or provide feedback for this feature, send email to [bq-notebook-python-gen-feedback@google.com](mailto:bq-notebook-python-gen-feedback@google.com).

You can generate [BigQuery DataFrames](https://docs.cloud.google.com/bigquery/docs/dataframes-quickstart)
code with Gemini in BigQuery. To ask
Gemini to use BigQuery DataFrames in the generated code,
express your intent in your prompt. For example, you can start your prompt with
"using bigframes" or "utilizing BigQuery DataFrames".

BigQuery DataFrames provides two libraries:

- bigframes.pandas, which provides a pandas-compatible API for analytics.
- bigframes.ml, which provides a scikit-learn-like API for machine learning (ML).

Gemini code generation is optimized for the bigframes.pandas
library.

To learn more about BigQuery DataFrames and the
permissions that are required to use BigQuery DataFrames, see
[BigQuery DataFrames permissions](https://docs.cloud.google.com/bigquery/docs/use-bigquery-dataframes).
BigQuery DataFrames is an open-source package.
You can run `pip install --upgrade bigframes` to install the latest
version.

In the following example, you generate code for a BigQuery
public dataset, `bigquery-public-data.ml_datasets.penguins`.

1. In the Google Cloud console, go to the BigQuery Studio page.

   [Go to BigQuery Studio](https://console.cloud.google.com/bigquery)
2. In the tab bar of the query editor, click the

   drop-down arrow next to
   **SQL query** , and then click
   **Notebook**.

   A new notebook opens.
3. To insert a new code cell, in the toolbar, click
   **Code**.

4. The new code cell contains the message **Start coding or generate with AI.**
   In the new code cell, click **generate**.

5. In the **Generate** editor, enter the following natural language prompt:

       Read the penguins table from the BigQuery public data using bigframes

6. Press <kbd>Enter</kbd> (<kbd>Return</kbd> on macOS).

   The suggested Python code is similar to the following:

       import bigframes.pandas as bpd

       # Read the penguins table from the BigQuery public data using bigframes
       result = bpd.read_gbd("bigquery-public-data.ml_datasets.penguins")

7. To run the code, press
   **Run cell**.

8. To preview the results, in the toolbar, click **Code** to insert a new code cell.

9. In the new cell, call the `peek()` method---for example,
   `result.peek()`---and press
   **Run cell**. A number of
   rows of data are displayed.

## Complete Python code

Python code completion attempts to provide contextually appropriate
recommendations that are based on content in the query editor. As you type,
Gemini in BigQuery can suggest logical next steps
that are relevant to your current code's context, or it can help you iterate on
your code.

To try Python code completion with Gemini in
BigQuery, follow these steps:

1. In the Google Cloud console, go to the **BigQuery Studio** page.

   [Go to BigQuery Studio](https://console.cloud.google.com/bigquery)
2. In the tab bar of the query editor, click the

   drop-down arrow next to
   **SQL query** , and then click
   **Notebook**.

   A new notebook opens, containing cells that show example queries against
   the `bigquery-public-data.ml_datasets.penguins` public dataset.
3. In the editor, begin typing Python code. For example
   `%%bigquery`. Gemini in BigQuery
   suggests code inline while you type.

4. To accept the suggestion, press <kbd>Tab</kbd>.

## Explain Python code

> [!WARNING]
>
> **Preview**
>
>
> This product or feature is
>
> subject to the "Pre-GA Offerings Terms" in the General Service Terms section
> of the [Service Specific
> Terms](https://docs.cloud.google.com/terms/service-terms#1).
>
> Pre-GA products and features are available "as is" and might have limited support.
>
> For more information, see the
> [launch stage descriptions](https://cloud.google.com/products/#product-launch-stages).

You can use Gemini in BigQuery to explain Python
code in your Colab Enterprise notebooks.

After getting an explanation, you can ask more questions in the prompt dialog
to understand the code better.

To get an explanation for Python code in your notebook, follow these steps:

1. In the Google Cloud console, go to the **BigQuery Studio** page.


   [Go to BigQuery Studio](https://console.cloud.google.com/bigquery)

   <br />

2. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)

   If you don't see the left pane, click **Expand left pane** to open the pane.
3. In the **Explorer** pane, expand your project and click **Notebooks**.

4. Click the notebook that you want to open.

5. Highlight the Python cell that you want to understand.

6. Click spark **Gemini** ,
   and then click **Explain code**.

   The code explanation appears in a panel next to the cell.
7. Optional: To understand your code better, ask questions in the **Enter
   prompt here** field.

## Fix and explain Python errors

> [!WARNING]
>
> **Preview**
>
>
> This product or feature is
>
> subject to the "Pre-GA Offerings Terms" in the General Service Terms section
> of the [Service Specific
> Terms](https://docs.cloud.google.com/terms/service-terms#1).
>
> Pre-GA products and features are available "as is" and might have limited support.
>
> For more information, see the
> [launch stage descriptions](https://cloud.google.com/products/#product-launch-stages).

You can use Gemini in BigQuery to fix and explain
Python code errors in your Colab Enterprise notebooks.

To fix or understand the code errors with Gemini assistance,
follow these steps:

1. In the Google Cloud console, go to the **BigQuery Studio** page.


   [Go to BigQuery Studio](https://console.cloud.google.com/bigquery)

   <br />

2. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)
3. In the **Explorer** pane, expand your project and click **Notebooks**.

4. Click the notebook that you want to open.

5. In a code cell of your notebook, enter code that contains an error, and then
   run the cell. For example, you might enter `print(1`, which is missing a
   closing parenthesis.

   After your code cell runs, the notebook prints an error message below your
   code cell. If you have Gemini in
   Python notebooks turned on and Gemini has a
   suggestion to fix or explain the error, one of the following options
   appears:
   - For Python syntax errors, a **Fix error** option appears.
   - For all other types of errors, an **Explain error** option appears.
6. To fix a syntax error, do the following:

   1. Click **Fix error**.

      Gemini suggests how to fix the error.
   2. Evaluate the suggestion, and then do one of the following:

      - To accept the suggestion, click check **Accept suggestion**.
      - To reject the suggestion, click close **Reject suggestion**.
7. To fix all other types of errors, do the following:

   1. Click **Explain error**.

      A panel opens, explaining the error and suggesting changes.
   2. Optional: To understand the error better, ask questions in the **Enter
      prompt here** field.

   3. To accept a suggested change, click
      library_add
      **Add code cell**.

## Generate PySpark code

> [!WARNING]
>
> **Preview**
>
>
> This product or feature is
>
> subject to the "Pre-GA Offerings Terms" in the General Service Terms section
> of the [Service Specific
> Terms](https://docs.cloud.google.com/terms/service-terms#1).
>
> Pre-GA products and features are available "as is" and might have limited support.
>
> For more information, see the
> [launch stage descriptions](https://cloud.google.com/products/#product-launch-stages).

You can ask Gemini Code Assist to generate PySpark code in your
notebook. Gemini Code Assist fetches and uses relevant BigQuery
and Dataproc Metastore tables and their schemas to generate a code
response. With its schema knowledge, Gemini Code Assist avoids hallucinations, and
suggests join keys and column types.

To generate Gemini Code Assist code in your notebook, do the following:

1. Insert a new code cell by clicking **+ Code** in the toolbar.
   The new code cell displays `Start coding or generate with AI`.
   Click **generate**.

2. In the Generate editor, enter a natural language prompt, and then click
   `enter`. **Make sure to include the keyword `spark` or `pyspark` in your prompt.**.

   Sample prompt:

   ```
   create a spark dataframe from order_items and filter to orders created in 2024
   ```

   <br />

   Sample output:

   ```
   spark.read.format("bigquery").option("table", "sqlgen-testing.pysparkeval_ecommerce.order_items").load().filter("year(created_at) = 2024").createOrReplaceTempView("order_items")
   df = spark.sql("SELECT * FROM order_items")
   ```

### Tips for Gemini Code Assist code generation

- To let Gemini Code Assist fetch relevant tables and schemas,
  turn on [Data Catalog sync](https://docs.cloud.google.com/dataproc-metastore/docs/data-catalog-sync)
  for Dataproc Metastore instances.

- Make sure your user account has access to Data Catalog
  to query tables. To do this, assign the
  [`DataCatalog.Viewer` role](https://docs.cloud.google.com/iam/docs/roles-permissions/datacatalog#datacatalog.viewer).

## Turn off Gemini query assistant features

To turn off specific features in Gemini in
BigQuery, do the following:

1. In the Google Cloud console, go to the **BigQuery Studio** page.

   [Go to BigQuery Studio](https://console.cloud.google.com/bigquery)
2. In the BigQuery toolbar, click
   pen_spark**Gemini**.

   ![Gemini button in the BigQuery toolbar.](https://docs.cloud.google.com/static/bigquery/images/duet-ai-assistant-link.png)
3. In the list, clear the query assistant features that you want to turn off.

To learn how to turn off Gemini in BigQuery,
see
[Turn off Gemini in BigQuery](https://docs.cloud.google.com/bigquery/docs/gemini-set-up#turn-off).

## Turn off Gemini in Colab Enterprise

To turn off Gemini in Colab Enterprise for a Google Cloud project, an
administrator must turn off the Gemini for Google Cloud API. See
[Disabling services](https://docs.cloud.google.com/service-usage/docs/enable-disable#disabling).

To turn off Gemini in Colab Enterprise for a specific user, an administrator
needs to revoke the
[Gemini for Google Cloud User](https://docs.cloud.google.com/iam/docs/roles-permissions/cloudaicompanion#cloudaicompanion.user)
(`roles/cloudaicompanion.user`) role for that user. See
[Revoke a single IAM role](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access#revoke-single-role).

## Provide feedback

1. In the Google Cloud console, go to the **BigQuery Studio** page.

   [Go to BigQuery Studio](https://console.cloud.google.com/bigquery)
2. In the BigQuery toolbar, click
   pen_spark**Gemini**.

   ![Gemini button in the BigQuery toolbar.](https://docs.cloud.google.com/static/bigquery/images/duet-ai-assistant-link.png)
3. Click **Send feedback**.

## Help improve suggestions

You can help improve Gemini suggestions by sharing with Google
the prompt data that you submit to features in [Preview](https://cloud.google.com/products/#product-launch-stages).

To share your prompt data, follow these steps:

1. In the Google Cloud console, go to the **BigQuery Studio** page.

   [Go to BigQuery Studio](https://console.cloud.google.com/bigquery)
2. In the BigQuery toolbar, click
   pen_spark**Gemini**.

   ![Gemini button in the BigQuery toolbar.](https://docs.cloud.google.com/static/bigquery/images/duet-ai-assistant-link.png)
3. Select **Share data to improve Gemini in BigQuery**.

4. In the **Data Use Settings** dialog, update your data use settings.

Data sharing settings apply to the entire project and can only be set by a
project administrator with the `serviceusage.services.enable` and
`serviceusage.services.list` IAM permissions. For more
information about data use in the Trusted Tester Program, see [Gemini
for Google Cloud Trusted Tester Program](https://cloud.google.com/trusted-tester/gemini-for-google-cloud-preview).

## Gemini and BigQuery data

In order to provide accurate results, Gemini in
BigQuery requires access to both your
[Customer Data](https://cloud.google.com/terms/data-processing-addendum) and metadata
in BigQuery for enhanced features. For more information, see
[How Gemini in BigQuery uses your data](https://docs.cloud.google.com/bigquery/docs/gemini-overview#data-usage).

## Locations

You can use Gemini in BigQuery-assisted SQL and
Python data analysis in all [BigQuery
locations](https://docs.cloud.google.com/bigquery/docs/locations). To learn about where Gemini
in BigQuery processes your data, see [Where Gemini
in BigQuery processes your data](https://docs.cloud.google.com/bigquery/docs/gemini-locations).

## Pricing

For details about pricing for this feature, see
[Gemini in BigQuery pricing overview](https://cloud.google.com/products/gemini/pricing#gemini-in-bigquery-pricing).

## What's next

- Read [Gemini for Google Cloud overview](https://docs.cloud.google.com/gemini/docs/overview).
- Learn [how Gemini for Google Cloud uses your data](https://docs.cloud.google.com/gemini/docs/discover/data-governance).
- Learn how to [explore your data by generating data insights](https://docs.cloud.google.com/bigquery/docs/data-insights).

<br />