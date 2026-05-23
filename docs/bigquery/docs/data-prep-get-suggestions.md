# Prepare data with Gemini

This document explains how to clean and transform your data within
BigQuery data preparations using SQL code suggestions from
Gemini.

For more information, see [BigQuery data preparation overview](https://docs.cloud.google.com/bigquery/docs/data-prep-introduction).

## Before you begin

- [Set up Gemini in BigQuery](https://docs.cloud.google.com/bigquery/docs/gemini-set-up).

- Give the [required Identity and Access Management (IAM) roles and permissions](https://docs.cloud.google.com/bigquery/docs/manage-data-preparations#required-roles).

## Start a data preparation session

Open the BigQuery data preparation editor by creating a new data
preparation, starting one from an existing table or a Cloud Storage or Google
Drive file, or opening an existing data preparation. For more information about
what happens when you create a data preparation, see
[Data preparation entry points](https://docs.cloud.google.com/bigquery/docs/data-prep-introduction#entry-points).

On the **BigQuery** page, you can go to the data preparation editor in
the following ways:

### Create new

To create a new data preparation in BigQuery, follow
these steps:

1. In the Google Cloud console, go to the **BigQuery** page.  
   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. Go to the **Create new** list and click **Data preparation**. The data preparation editor is displayed in a new untitled data preparation tab.
3. In the editor's search bar, enter your table name or keywords and select a table. The data preparation editor for the table opens, showing a preview of your data on the **Data** tab, and an initial set of data preparation suggestions from Gemini.
4. Optional: To simplify your view, turn on full screen mode by clicking fullscreen **Full screen**.
5. Optional: To view data preparation details, version history, add new
   comments, or reply to existing comments, use the toolbar.

   ![Explore the data preparations toolbar.](https://docs.cloud.google.com/static/bigquery/images/code-assets-toolbar.png)

   The **Comments** toolbar feature is in
   [Preview](https://cloud.google.com/products#product-launch-stages). To
   provide feedback or request support for this feature, send an email to
   [bqui-workspace-pod@google.com](mailto:bqui-workspace-pod@google.com).

### Create from a table

To create a new data preparation from an existing table, follow these
steps:

1. In the Google Cloud console, go to the **BigQuery** page.  
   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Explorer**.
3. In the **Explorer** pane, expand your project, click **Datasets**, and then select a dataset.
4. For your table name, click more_vert **Actions \> Open in \> Data preparation** . The data preparation editor for the table opens, showing a preview of your data on the **Data** tab, and an initial set of data preparation suggestions from Gemini.
5. Optional: To simplify your view, turn on full screen mode by clicking fullscreen **Full screen**.
6. Optional: To view data preparation details, version history, add new
   comments, or reply to existing comments, use the toolbar.

   ![Explore the data preparations toolbar.](https://docs.cloud.google.com/static/bigquery/images/code-assets-toolbar.png)

   The **Comments** toolbar feature is in
   [Preview](https://cloud.google.com/products#product-launch-stages). To
   provide feedback or request support for this feature, send an email to
   [bqui-workspace-pod@google.com](mailto:bqui-workspace-pod@google.com).

### Create from a file

To create a new data preparation from a file in
Cloud Storage or Google Drive, follow these steps:

#### Load the file

1. In the Google Cloud console, go to the **BigQuery** page.  
   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the **Create new** list, click **Data
   preparation**. The data preparation editor is displayed in a new untitled data preparation tab.
3. In the list of data sources, click **Google Cloud Storage** or **Google Drive** . The **Prepare data** dialog opens.
4. In the **Source** section, select your file:
   - **Cloud Storage** : Select the file from a Cloud Storage bucket, or enter the path of your source. For example, enter a path to your CSV file: `STORAGE_BUCKET_NAME/FILE_NAME.csv`. Wildcard searches, such as `*.csv`, are supported.
   - **Google Drive**: Select the file from Google Drive by entering its URI. To load a subset of that data, you can enter a specific sheet name and a range.

   > [!NOTE]
   > **Note:** If you use Google Drive as a data source, you must use a service account to [run or
   > schedule the data preparation](https://docs.cloud.google.com/bigquery/docs/orchestrate-data-preparations). End-user credentials are not supported for this operation. You must also share the Google Drive file with the service account.

   The file format is automatically detected. Supported formats are
   Avro, CSV, JSONL, ORC, and Parquet. Other compatible file types,
   such as DAT, TSV, and TXT, are read as the CSV format. The Google Drive
   option also supports the Google Sheets format.
5. Define the external staging table where you'll upload files. In the **Staging table** section, enter the project, dataset, and table names for the new table.
6. In the **Schema** section, review the schema. Gemini checks your file for column names. If it doesn't find any, it provides suggestions.  

   By default, your data preparation file loads data as strings. You can define more specific data types when you [prepare the file data](https://docs.cloud.google.com/bigquery/docs/data-prep-get-suggestions#prepare-file-data).
7. Optional: In **Advanced options**, you can add more information, such as the number of errors allowed before the job fails. Gemini provides additional options based on your file's content.
8. Optional: To preview the new staging table in the data preparation editor, select **Generate preview**.
9. Click **Create** . The data preparation editor for the file opens, showing a preview of your data on the **Data** tab, and an initial set of data preparation suggestions from Gemini.
10. Optional: To simplify your view, turn on full screen mode by clicking fullscreen **Full screen**.
11. Optional: To view data preparation details, version history, add
    new comments, or reply to existing comments, use the toolbar.

    ![Explore the data preparations toolbar.](https://docs.cloud.google.com/static/bigquery/images/code-assets-toolbar.png)

    The **Comments** toolbar feature is in
    [Preview](https://cloud.google.com/products#product-launch-stages).
    To provide feedback or request support for this feature, send an email
    to [bqui-workspace-pod@google.com](mailto:bqui-workspace-pod@google.com).

#### Prepare the file

In the data view, prepare the staged data that
you loaded by following these steps:

1. Optional: Define stronger data types for relevant columns by browsing the suggestion list for transformation suggestions or selecting a column and generating suggestions for it.
2. Optional: Define validation rules. For more information, see [Configure the error table and add a
   validation rule](https://docs.cloud.google.com/bigquery/docs/data-prep-get-suggestions#configure-validation).
3. [Add a destination table](https://docs.cloud.google.com/bigquery/docs/data-prep-get-suggestions#add-or-change-destination).
4. To load the data into the destination table, [run the data preparation](https://docs.cloud.google.com/bigquery/docs/data-prep-get-suggestions#run-data-prep).
5. Optional: [Schedule the data preparation run](https://docs.cloud.google.com/bigquery/docs/orchestrate-data-preparations).
6. Optional: [Optimize data preparation by incrementally processing data](https://docs.cloud.google.com/bigquery/docs/manage-data-preparations#optimize).

### Open existing

To open the editor for an existing data preparation, follow these
steps:

1. In the Google Cloud console, go to the **BigQuery** page.  
   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Explorer**.
3. In the **Explorer** pane, click your project name, and then click **Data preparations**.
4. Select the existing data preparation. The graph view of the data preparation pipeline is displayed.
5. Select one of the nodes in the graph. The data preparation editor for the table opens, showing a preview of your data on the **Data** tab, and an initial set of data preparation suggestions from Gemini.
6. Optional: To simplify your view, turn on full screen mode by clicking fullscreen **Full screen**.
7. Optional: To view data preparation details, version history, add new
   comments, or reply to existing comments, use the toolbar.

   ![Explore the data preparations toolbar.](https://docs.cloud.google.com/static/bigquery/images/code-assets-toolbar.png)

   The **Comments** toolbar feature is in
   [Preview](https://cloud.google.com/products#product-launch-stages). To
   provide feedback or request support for this feature, send an email to
   [bqui-workspace-pod@google.com](mailto:bqui-workspace-pod@google.com).

## Add data preparation steps

You prepare data in steps. You can preview or apply steps suggested by
Gemini. You can also improve the suggestions, or apply your own
steps.

## Apply and improve suggestions by Gemini

When you open the data preparation editor for your table, Gemini
inspects the data and schema from the table you loaded and generates filter and
transformation suggestions. The suggestions appear on cards in the **Steps**
list.

The following image shows where you can apply and improve steps suggested by
Gemini:

![Data view in the data preparation editor showing options to preview, edit, or apply suggestions from Gemini.](https://docs.cloud.google.com/static/bigquery/images/data-prep-data-view.png)

To apply a suggestion by Gemini as a data preparation step, do
the following:

1. In the data view, click a column name or a particular cell. Gemini generates suggestions for filtering and transforming the data.
2. Optional: To improve the suggestions, edit the values of one to three cells
   in the table to demonstrate what the values in a column should look like.
   For example, enter a date the way you want to format all dates.
   Gemini generates new suggestions based on your changes.

   > [!NOTE]
   > **Note:** Your example change to the data isn't saved.

   The following image shows how you can edit values to improve the steps
   suggested by Gemini:

   ![Improve suggestions by editing values in the cells to demonstrate what the values in the column should look like.](https://docs.cloud.google.com/static/bigquery/images/data-prep-improve-suggestion.png)
3. Select a suggestion card.

   1. Optional: To preview the result of the suggestion card, click **Preview**.
   2. Optional: To modify the suggestion card using natural language, click **Edit**.
4. Click **Apply**.

## Add steps with natural language or SQL expressions

If existing suggestions don't meet your needs, add a step. Choose columns or a
step type, then describe what you want using natural language.

### Add a transformation

1. In the data or schema view, choose the **Transform** option. You can also choose columns or add examples to help Gemini understand your data transformation.
2. In the **Description** field, enter a prompt, such as `Convert the state
   column to uppercase`.
3. Click send
   **Send**.

   Gemini generates a SQL expression and a new description based
   on your prompt.
4. In the **Target column** list, select or enter a column name.

5. Optional: To update the SQL expression, revise the prompt and click
   send
   **Send**, or manually enter a SQL expression.

6. Optional: Click **Preview** and review the step.

7. Click **Apply**.

### Flatten JSON columns

To make key-value pairs easier to access and analyze, flatten JSON columns. For
example, if you have a JSON column named `user_properties` that contains the
keys `country` and `device_type`, flattening this column extracts `country` and
`device_type` into their own top-level columns so you can use them directly in
your analysis.

Gemini for BigQuery suggests operations that
extract fields only from the top level of the JSON. If these extracted fields
contain more JSON objects, you can flatten them in additional steps to access
their contents.

1. In the data view for a JSON source table, choose a column or cells.
2. Click **Flatten** to generate suggestions.
3. Optional: To update the SQL expression, you can manually enter a SQL expression.
4. Optional: Click **Preview** and review the step.
5. Click **Apply**.

Flattening has the following behaviors:

- The **Flatten** option appears in the data view after you select cells or columns containing JSON. It doesn't appear by default when you click **Add
  step**.
- If a JSON key isn't present in the selected rows, the generated suggestion doesn't contain that key. This issue might cause some columns to be left out when data is flattened.
- If column names collide during flattening, the repeated column names end in this format: `_<i>`. For example, if there's already a column named `address`, the new flattened column name is `address_1`.
- Flattened column names follow the BigQuery [column naming conventions](https://docs.cloud.google.com/bigquery/docs/schemas#column_names).
- If you leave the JSON key field empty, the default column name format is `f<i>_`.

### Flatten `RECORD` or `STRUCT` columns

To make nested fields easier to access and analyze, flatten columns with the
`RECORD` or `STRUCT` data type. For example, if you have an `event_log` record
that contains the fields `timestamp` and `action`, flattening this record
extracts `timestamp` and `action` into their own top-level columns so you can
transform them directly.

This process extracts all nested columns from the record, up to 10 levels deep,
and creates a new column for each. The new column names are created by combining
the parent column's name with the nested field name, separated by an underscore
(for example,
`PARENT-COLUMN-NAME_FIELD-NAME`). The original
column is dropped. To keep the original column, you can
[delete](https://docs.cloud.google.com/bigquery/docs/data-prep-get-suggestions#delete-applied-step) the **Drop column** step from the **Applied
steps** list.

To flatten records, follow these steps:

1. In the data view for a source table, choose a record column.
2. Click **Flatten** to generate suggestions.
3. Optional: To update the SQL expression, you can manually enter a SQL expression.
4. Optional: Click **Preview** and review the step.
5. Click **Apply**.

> [!NOTE]
> **Note:** Flattening a record doesn't expand nested JSON objects or repeated fields (arrays) within the record. To access their contents, you must flatten these fields in separate steps.

### Unnest arrays

Unnesting expands each element in an array into its own row, duplicating the
other original column values into each new row. This action is useful for
analyzing columns that contain arrays with a variable number of elements, such
as lists of API responses.

You can unnest the following column types:

- **`ARRAY` data type:** Unnests into elements of the array's base type. For example, an `ARRAY<STRUCT<...>>` unnesting results in elements of type `STRUCT`.
- **`JSON` columns:** Unnests JSON arrays within the column into elements of type `JSON`.

When you unnest an array, a new column is created that contains the unnested
elements. By default, the original array column is dropped. To keep the original
column, [delete](https://docs.cloud.google.com/bigquery/docs/data-prep-get-suggestions#delete-applied-step) the **Drop column** step from the
**Applied steps** list.

To unnest arrays, follow these steps:

1. In the data view for a source table, choose an `ARRAY` column.
2. Click **Unnest** to generate suggestions.
3. Optional: To update the SQL expression, you can manually enter a SQL expression.
4. Optional: Click **Preview** and review the step.
5. Click **Apply**.

> [!NOTE]
> **Note:** When unnesting an array column, you might encounter an error if a column in your original table has the same name as a column in the unnested array. To work around this, rename the conflicting column in your original table before performing the unnest operation.

### Filter rows

To add a filter that removes rows, follow these steps:

1. In the data or schema view, choose the **Filter** option. You can also choose columns to help Gemini understand your data filter.
2. In the **Description** field, enter a prompt, such as `Column ID should not
   be NULL`.
3. Click **Generate**. Gemini generates a SQL expression and a new description based on your prompt.
4. Optional: To update the SQL expression, revise the prompt and click send **Send**, or enter a SQL expression manually.
5. Optional: Click **Preview** and review the step.
6. Click **Apply**.

#### Filter expression format

SQL expressions for filters retain rows that match the specified condition. This
is equivalent to a `SELECT ... WHERE SQL_EXPRESSION`
statement.

For example, to retain records where the column `year` is greater than or equal
to `2000`, the condition is `year >= 2000`.

Expressions must follow the BigQuery SQL syntax for the
[`WHERE` clause](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#where_clause).

### Deduplicate data

To remove duplicate rows from your data, follow these steps:

1. In the data or schema view, choose the **Deduplicate** option. Gemini provides an initial deduplication suggestion.
2. Optional: To refine the suggestion, enter a new description and click send **Send**.
3. Optional: To manually configure the deduplication step, use the following options:
   - In the **Record choosing** list, select one of the following strategies:
     - **First** : For each group of rows with the same deduplication key values, this strategy chooses the first row based on the `ORDER BY` expression and removes the rest.
     - **Last** : For each group of rows with the same deduplication key values, this strategy chooses the last row based on the `ORDER BY` expression and removes the rest.
     - **Any**: For each group of rows with the same deduplication key values, this strategy chooses any row from that group and removes the rest.
     - **Distinct**: Removes all duplicate rows across all columns in the table.
   - In the **Deduplication keys** field, choose one or more columns or expressions to identify duplicate rows. This field is applicable when the record choosing strategy is **First** , **Last** , or **Any**.
   - In the **Order by expression** field, enter an expression that defines the row order. For example, to choose the most recent row, enter `datetime DESC`. To choose the first row alphabetically by name, enter a column name like `last_name`. The expression follows the same rules as the standard [`ORDER BY` clause](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#order_by_clause) in BigQuery. This field is only applicable when the record choosing strategy is **First** or **Last**.
4. Optional: Click **Preview** and review the step.
5. Click **Apply**.

### Delete a column

To delete one or more columns from a data preparation, follow these steps:

1. In the data or schema view, select the columns you want to drop.
2. Click **Drop**. A new applied step is added for the deleted columns.

### Add a join operation with Gemini

To add a join operation step between two sources in your data preparation,
follow these steps:

1. In the data view for a node in your data preparation, go to the **Suggestions** list, and click the **Join** option.
2. In the **Add join** dialog, click **Browse**, and then select the other table involved in the join operation (referred to as the right side of the join).
3. Optional: Select the type of [join operation](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#join_types) that you want to perform, such as **Inner join**.
4. Review the Gemini-generated join key information in the
   following fields:

   - **Join description** : The natural language description of the SQL expression for the join operation. When you edit this description and click send **Send**, Gemini suggests new SQL join conditions.
   - **Join conditions** : The SQL expressions within the `ON` clause for the
     join operation. You can use the `L` and `R` qualifiers to refer to the
     left and right source tables, respectively. For example, to join the
     `customer_id` column from the left table to the `customer_id` column from
     the right table, enter `L.customerId = R.customerId`. These qualifiers
     aren't case-sensitive.

     > [!NOTE]
     > **Note:** If the **Join conditions** field is empty, the join operation type is automatically set to **Cross join**, even if you selected a different join type in the previous step.

5. Optional: To refine the suggestions from Gemini, edit the
   **Join description** field, and then click
   send
   **Send**.

6. Optional: To preview the join operation settings of your data preparation,
   click **Preview**.

7. Click **Apply**.

   The join operation step is created. The source table that you selected (the
   right side of the join) and the join operation are reflected in the list of
   applied steps and in the nodes in the graph view of your data preparation.

### Aggregate data

1. In the data or schema view, choose the **Aggregate** option.
2. In the **Description** field, enter a prompt, such as `Find the total
   revenue for a region`.
3. Click **Send**.

   Gemini generates grouping keys and aggregation expressions
   based on your prompt.
4. Optional: Edit the generated grouping keys or aggregation expressions,
   if needed.

5. Optional: You can manually add grouping keys and aggregation expressions.

   - In the **Grouping keys** field, enter a column name or expression. If you leave it blank, the resulting table has one row. If you enter an expression, it must have an alias (an `AS` clause)---for example `EXTRACT(YEAR FROM order_date) AS order_year`. No duplicates are allowed.
   - In the **Aggregation expressions** field, enter an aggregation expression that has an alias (an `AS` clause)---for example `SUM(quantity) AS total_quantity`. You can enter multiple, comma-separated expressions. No duplicates are allowed. For a list of the supported aggregation expressions, see [Aggregate functions](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions).
6. Optional: Click **Preview** and review the step.

7. Click **Apply**.

### Configure the error table and add a validation rule

You can add a filter that creates a validation rule, which sends errors to an
error table or fails the data preparation run.

#### Configure the error table

To configure your error table, follow these steps:

1. In the data preparation editor, go to the toolbar and click **More
   \> Error table**.
2. Click **Enable error table**.
3. Define the table location.
4. Optional: Define a maximum duration for keeping errors.
5. Click **Save**.

#### Add a validation rule

To add a validation rule, follow these steps:

1. In the data or schema view, click the **Filter** option. You can also choose columns to help Gemini understand your data filter.
2. Enter a description for the step.
3. Enter a SQL expression, in the form of a `WHERE` clause.
4. Optional: If you want the SQL expression to act as a validation rule, select the **Failed validation rows go to error table** checkbox. You can also change a filter to a validation in the data preparation toolbar by clicking **More \> Error table**.
5. Optional: Click **Preview** and review the step.
6. Click **Apply**.

### Add or change a destination table

A destination table is required to run or schedule your data preparation. To
add or change a destination table for the output of your data preparation,
follow these steps:

1. In the data or schema view, click **Destination** in the **Suggestions** list.
2. Select the project where the destination table is stored.
3. Select one of the datasets, or load a new dataset.
4. Enter a destination table. If the table doesn't exist, the data preparation creates a new table on the first run. For more information, see [Write mode](https://docs.cloud.google.com/bigquery/docs/data-prep-introduction#write-mode).
5. Select your dataset as the destination dataset.
6. Click **Save**.

## View the data sample and schema for an applied step

To view sample and schema details at a particular step in the data preparation,
do the following:

1. In the data preparation editor, go to the **Steps** list and click **Applied
   steps**.
2. Select a step. The **Data** and **Schema** tabs appear, displaying the data sample and schema as of this particular step.

## Edit an applied step

To edit an applied step, do the following:

1. In the data preparation editor, go to the **Steps** list and click **Applied
   steps**.
2. Select a step.
3. Next to the step, click more_vert **Menu \> Edit**.
4. In the **Edit Applied Step** dialog, you can do the following:
   - Edit the description of the step.
   - Get suggestions from Gemini by editing the description and clicking send **Send**.
   - Edit the SQL expression.
5. In the **Target column** field, select a column.
6. Optional: Click **Preview** and review the step.
7. Click **Apply**.

## Delete an applied step

To delete an applied step, do the following:

1. In the data preparation editor, go to the **Steps** list and click **Applied
   steps**.
2. Select a step.
3. Click more_vert **Menu \> Delete**.

## Run the data preparation

After you've added your data preparation steps, [configured the destination](https://docs.cloud.google.com/bigquery/docs/data-prep-get-suggestions#add-or-change-destination),
and fixed any validation errors, you can perform test runs on a sample of the
data, or deploy the steps and schedule data preparation runs. For more
information, see [Schedule data preparations](https://docs.cloud.google.com/bigquery/docs/orchestrate-data-preparations).

## Refresh data preparation samples

Data in the sample isn't automatically refreshed. If data in the source tables
for the data preparation has changed, but the changes aren't reflected in the
data sample of the preparation, click **More \> Refresh sample**.

## What's next

- Learn how to [schedule data preparations](https://docs.cloud.google.com/bigquery/docs/orchestrate-data-preparations).
- Learn about [managing data preparations](https://docs.cloud.google.com/bigquery/docs/manage-data-preparations).
- Review [Gemini in BigQuery pricing](https://cloud.google.com/products/gemini/pricing#gemini-in-bigquery-pricing).