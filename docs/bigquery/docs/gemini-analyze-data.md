# Analyze data with Gemini assistance

This tutorial describes how you can use AI-powered assistance in
[Gemini in BigQuery](https://docs.cloud.google.com/bigquery/docs/gemini-overview)
to analyze data.

For the example in this tutorial, consider that you're a data
analyst who needs to analyze and predict product sales from a dataset.

This tutorial assumes that you're familiar with SQL and basic data analytics
tasks. Knowledge of Google Cloud products is not assumed. If you're new to
BigQuery, see the
[BigQuery quickstarts](https://docs.cloud.google.com/bigquery/docs/quickstarts).

## Objectives

- Use Gemini in BigQuery to answer questions about how BigQuery handles specific data analysis tasks.
- Prompt Gemini in BigQuery to find datasets, and to explain and generate SQL queries.
- Build a machine learning (ML) model to forecast future periods.

## Costs

This tutorial uses the following billable Google Cloud products:

- [BigQuery](https://cloud.google.com/bigquery/pricing)
- [BigQuery ML](https://cloud.google.com/bigquery/pricing#bqml)

To estimate your costs based on your projected usage, use the
[pricing calculator](https://cloud.google.com/products/calculator).

## Before you begin

1. In the Google Cloud console, on the project selector page,
   select or create a Google Cloud project.

   **Roles required to select or create a project**
   - **Select a project**: Selecting a project doesn't require a specific IAM role---you can select any project that you've been granted a role on.
   - **Create a project** : To create a project, you need the Project Creator role (`roles/resourcemanager.projectCreator`), which contains the `resourcemanager.projects.create` permission. [Learn how to grant
     roles](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).

   > [!NOTE]
   > **Note**: If you don't plan to keep the resources that you create in this procedure, create a project instead of selecting an existing project. After you finish these steps, you can delete the project, removing all resources associated with the project.

   [Go to project selector](https://console.cloud.google.com/projectselector2/home/dashboard)
2. [Ensure that Gemini in BigQuery is set up for
   your Google Cloud project](https://docs.cloud.google.com/bigquery/docs/gemini-set-up).
3. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
4. [Create a dataset](https://docs.cloud.google.com/bigquery/docs/datasets)
   that's named `bqml_tutorial`. You use the dataset to store database
   objects, including tables and models.

5.
   To turn on the Gemini in BigQuery features
   that you need to complete this tutorial, in the BigQuery
   toolbar, click pen_spark **Gemini**,
   and then select the following options:

   - **Auto-completion**
   - **Auto-generation**
   - **Explanation**

## Learn about BigQuery capabilities

Before you get started, consider that you want to learn more about how
BigQuery handles data querying. To get help, you can send
Gemini in BigQuery a natural language statement
(or *prompt*) like the following:

- "How do I get started with BigQuery?"
- "What are the benefits of using BigQuery for data analysis?"
- "How does BigQuery handle auto-scaling for queries?"

Gemini in BigQuery can also provide information
about how to analyze your data. For that type of help, you might send prompts
such as the following:

- "How do I create a time series forecasting model in BigQuery?"
- "How do I load different types of data into BigQuery?"

## Access and analyze data

> [!CAUTION]
> As an early-stage technology, Gemini for Google Cloud
> products can generate output that seems plausible but is factually incorrect. We recommend that you
> validate all output from Gemini for Google Cloud products before you use it.
> For more information, see
> [Gemini for Google Cloud and responsible AI](https://docs.cloud.google.com/gemini/docs/discover/responsible-ai).

Gemini in BigQuery can help you know *what* data
you can access for analysis, and *how* to analyze that data.

For this example, consider that you need help with the following:

- Finding sales dataset and tables to analyze.
- Knowing how data tables and queries are related in a sales dataset.
- Understanding complex queries and writing queries that use the dataset.

### Find data

Before you can query data, you need to know what data you can access. Every data
product organizes and stores data differently.

To get help, you can send Gemini in BigQuery a
prompt like "How do I learn which datasets and tables are available to me in
BigQuery?"

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the Google Cloud console toolbar, click
   spark **Open or close Gemini Cloud Assist chat**.

3. In the **Cloud Assist** panel, enter the prompt `How do I learn which
   datasets and tables are available to me in BigQuery?`.

4. Click send **Send prompt**.

   Learn [how and when Gemini
   for Google Cloud uses your data](https://docs.cloud.google.com/gemini/docs/discover/data-governance).

   The response includes several ways to list projects, datasets, or tables
   within a dataset.
5. Optional: To reset your chat history, in the **Cloud Assist** panel,
   click delete
   **Clear chat** , and then click **Reset chat**.

   > [!NOTE]
   > **Note:** The chat history state is kept in memory only and doesn't persist when you switch to another workspace or when you close the Google Cloud console.

### Understand and write SQL in BigQuery

For this example, assume that you selected data to analyze and now want to
query that data. Gemini in BigQuery can help you
work with SQL---whether it's to help you understand queries that are complex and
difficult to parse, or to generate new SQL queries.

#### Prompt for Gemini assistance to explain SQL queries

Consider that you want to understand a complex query that someone else wrote.
Gemini in BigQuery can explain the query in plain
language---such as the query syntax, underlying schema, and business context.

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, open or paste the query that you want explained.
   For example:

       SELECT
         u.id AS user_id,
         u.first_name,
         u.last_name,
         avg(oi.sale_price) AS avg_sale_price
       FROM `bigquery-public-data.thelook_ecommerce.users` AS u
       JOIN `bigquery-public-data.thelook_ecommerce.order_items` AS oi
         ON u.id = oi.user_id
       GROUP BY 1, 2, 3
       ORDER BY avg_sale_price DESC
       LIMIT 10

3. Highlight the query, and then click
   auto_awesome
   **Explain this selected query**.

   In the **Cloud Assist** panel, a response is returned that's similar
   to the following:

       The intent of this query is to find the top 10 users by average sale price.
       The query first joins the users and order_items tables on the user_id
       column. It then groups the results by user_id, first_name, and last_name,
       and calculates the average sale price for each group. The results are then
       ordered by average sale price in descending order, and the top 10 results
       are returned.

#### Generate a SQL query that groups sales by day and product

In this example, you want to generate a query that lists your top products for
each day. You then use tables in the `thelook_ecommerce` dataset and prompt
Gemini in BigQuery to generate a query to
calculate sales by order item and by product name.

This type of query is often complex, but using Gemini in
BigQuery, you can automatically create a statement. You can
provide a prompt to generate a SQL query based on your data's schema. Even if
you start with no code, a limited knowledge of the data schema, or only a
basic knowledge of SQL syntax, Gemini assistance can suggest one
or more SQL statements.

To prompt Gemini in BigQuery to generate a query
that lists your top products, follow these steps:

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the navigation menu, click **Studio**.

3. Click **SQL query** .
   The **Explorer** pane automatically loads the selected database.

4. In the query editor, enter the following prompt, and then press
   <kbd>Enter</kbd>:

       # select the sum of sales by date and product casted to day from bigquery-public-data.thelook_ecommerce.order_items joined with bigquery-public-data.thelook_ecommerce.products

   The pound character (`#`) prompts Gemini in
   BigQuery to generate SQL. Gemini in
   BigQuery suggests a SQL query similar to the following:

       SELECT
         sum(sale_price),
         DATE(created_at),
         product_id
       FROM
         `bigquery-public-data.thelook_ecommerce.order_items`
           AS t1
       INNER JOIN `bigquery-public-data.thelook_ecommerce.products` AS t2
         ON t1.product_id = t2.id
       GROUP BY 2, 3

   > [!NOTE]
   > **Note:** Gemini in BigQuery might suggest multiple SQL statements for your prompt.

5. To accept the suggested code, click **Tab** , and then click **Run** to
   run the SQL statement. You can also scroll through the suggested SQL and
   accept specific words suggested in the statement.

6. In the **Query results** pane, view the query results.

## Build a forecasting model and view results

In this example, you use BigQuery ML to do the following:

- Use a trend query to build a forecasting model.
- Use Gemini in BigQuery to explain and help you write a query to view results of the forecasting model.

You use the following example query with actual sales, which are used as an
input to the model. The query is used as a part of creating the ML model.

1. To create a forecasting ML model, in the query editor,
   run the following SQL query:

       CREATE MODEL bqml_tutorial.sales_forecasting_model
         OPTIONS (
           MODEL_TYPE = 'ARIMA_PLUS',
           time_series_timestamp_col = 'date_col',
           time_series_data_col = 'total_sales',
           time_series_id_col = 'product_id')
       AS
       SELECT
         sum(sale_price) AS total_sales,
         DATE(created_at) AS date_col,
         product_id
       FROM
         `bigquery-public-data.thelook_ecommerce.order_items`
           AS t1
       INNER JOIN `bigquery-public-data.thelook_ecommerce.products` AS t2
         ON t1.product_id = t2.id
       GROUP BY 2, 3;

   You can use Gemini in BigQuery to help you
   [understand this query](https://docs.cloud.google.com/bigquery/docs/gemini-analyze-data#prompt-gemini-explain-sql-queries).

   > [!NOTE]
   > **Note:** While the model is running, you can also prompt Gemini in BigQuery in the **Cloud Assist** panel with questions like `What is an ARIMA_PLUS model type?`

   When the model is created, the **Results** tab of the **Query results**
   pane displays a message that's similar to the following:

       Successfully created model named sales_forecasting_model.

2. In the **Cloud Assist** panel, enter a prompt for Gemini in
   BigQuery to help you write a query to get a forecast from the
   model when it's completed---for example, enter
   `How can I get a forecast in SQL from the model?`

   Based on the context of the prompt, the response includes an example of an
   ML model that forecasts sales:

       SELECT
         *
       FROM
         ML.FORECAST(
           MODEL `PROJECT_ID.bqml_tutorial.sales_forecasting_model`,
           STRUCT(
             7 AS horizon,
             0.95 AS confidence_level))

   In this response, `PROJECT_ID` is your
   Google Cloud project.

   > [!NOTE]
   > **Note:** Gemini in BigQuery uses the context of the chat session to help answer questions in the same session.

3. In the **Cloud Assist** panel, copy the SQL query.

4. In the query editor, run the SQL query.

## Clean up

To avoid incurring charges to your Google Cloud account for the resources
used in this tutorial, you can delete the Google Cloud project that you
created for this tutorial. Alternatively, you can delete the individual
resources.

### Delete project

> [!CAUTION]
> **Caution** : Deleting a project has the following effects:
>
> - **Everything in the project is deleted.** If you used an existing project for the tasks in this document, when you delete it, you also delete any other work you've done in the project.
> - **Custom project IDs are lost.** When you created this project, you might have created a custom project ID that you want to use in the future. To preserve the URLs that use the project ID, such as an `appspot.com` URL, delete selected resources inside the project instead of deleting the whole project.
>
>
> If you plan to explore multiple architectures, tutorials, or quickstarts, reusing projects
> can help you avoid exceeding project quota limits.

1. In the Google Cloud console, go to the **Manage resources** page.

   [Go to Manage resources](https://console.cloud.google.com/iam-admin/projects)
2. In the project list, select the project that you want to delete, and then click **Delete**.
3. In the dialog, type the project ID, and then click **Shut down** to delete the project.

### Delete your dataset

Deleting your project removes all datasets and all tables in the project. If you
prefer to reuse the project, then you can delete the dataset that you created in
this tutorial.

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the navigation menu, select the **`bqml_tutorial`** dataset that you
   created.

3. To delete the dataset, the table, and all of the data, click **Delete
   dataset**.

4. To confirm deletion, in the **Delete dataset** dialog, type the name of your
   dataset (`bqml_tutorial`), and then click **Delete**.

## What's next

- Read [Gemini for Google Cloud overview](https://docs.cloud.google.com/gemini/docs/overview).
- Learn about [Gemini for Google Cloud quotas and limits](https://docs.cloud.google.com/gemini/docs/quotas).
- Learn about [locations for Gemini for Google Cloud](https://docs.cloud.google.com/gemini/docs/locations).
- Learn how to [explore your data by generating data insights](https://docs.cloud.google.com/bigquery/docs/data-insights).
- Learn more about how to [write queries with Gemini assistance in BigQuery](https://docs.cloud.google.com/bigquery/docs/write-sql-gemini).