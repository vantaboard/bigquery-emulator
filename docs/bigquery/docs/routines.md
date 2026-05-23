# Manage routines

In BigQuery, *routines* are a resource type that includes the
following:

- [Stored procedures](https://docs.cloud.google.com/bigquery/docs/procedures#writing_a_procedure).
- [User-defined functions](https://docs.cloud.google.com/bigquery/docs/user-defined-functions) (UDFs), including [remote functions](https://docs.cloud.google.com/bigquery/docs/remote-functions) and [user-defined aggregate functions](https://docs.cloud.google.com/bigquery/docs/user-defined-aggregates).
- [Table functions](https://docs.cloud.google.com/bigquery/docs/table-functions).

This document describes tasks that are common to all routine types in
BigQuery.

## Permissions

To reference a routine in a SQL query, you must have the `bigquery.routines.get`
permission. To grant access to routines you can grant an IAM role
with the `bigquery.routines.get` permission on the dataset or on the individual
routine. Granting access at the dataset level gives the principal access to all
routines in the dataset. For more information, see
[Control access to resources with IAM](https://docs.cloud.google.com/bigquery/docs/control-access-to-resources-iam).

By default, you also need permission to access any resources that the routine
references, such as tables or views. For UDFs and table functions, you can
*authorize* the function to access those resources on the caller's behalf. For
more information, see
[Authorized functions](https://docs.cloud.google.com/bigquery/docs/authorized-functions).

## Create a routine

To create a routine, you must have the `bigquery.routines.create` permission.

### SQL

Depending on the routine type, run one of the following DDL statements:

- [Stored procedure: `CREATE PROCEDURE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_procedure)
- [User-defined function: `CREATE FUNCTION`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_function_statement)
- [Table function: `CREATE TABLE FUNCTION`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_table_function_statement)
- [User-defined aggregate function: `CREATE AGGREGATE FUNCTION`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#sql-create-udaf-function)

### API

Call the [`routines.insert` method](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/routines/insert)
with a defined
[`Routine` resource](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/routines#Routine).

## List routines

To list the routines in a dataset, you must have the `bigquery.routines.get` and
`bigquery.routines.list` permissions.

### Console

1. In the Google Cloud console, open the BigQuery page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)

   If you don't see the left pane, click **Expand left pane** to open the pane.
3. In the **Explorer** pane, expand your project, click **Datasets**, and
   then select a dataset.

4. Click the **Routines** tab.

### SQL

Query the [`INFORMATION_SCHEMA.ROUTINES` view](https://docs.cloud.google.com/bigquery/docs/information-schema-routines):

<br />

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, enter the following statement:

   ```googlesql
   SELECT
     COLUMN_LIST
   FROM
      { DATASET | REGION }.INFORMATION_SCHEMA.ROUTINES;
   ```

   <br />

3. Click **Run**.

   <br />

For more information about how to run queries, see [Run an interactive query](https://docs.cloud.google.com/bigquery/docs/running-queries#queries).

Replace the following:

- <var translate="no">COLUMN_LIST</var>: a comma-separated list of columns from the [`INFORMATION_SCHEMA.ROUTINES` view](https://docs.cloud.google.com/bigquery/docs/information-schema-routines).
- <var translate="no">DATASET</var>: the name of a dataset in your project.
- <var translate="no">REGION</var>: a [region qualifier](https://docs.cloud.google.com/bigquery/docs/information-schema-intro#region_qualifier).

Example:

```googlesql
SELECT
  routine_name, routine_type, routine_body
FROM
  mydataset.INFORMATION_SCHEMA.ROUTINES;
```

    +---+---+---+
    |   routine_name   |  routine_type  | routine_body |
    +---+---+---+
    | AddFourAndDivide | FUNCTION       | SQL          |
    | create_customer  | PROCEDURE      | SQL          |
    | names_by_year    | TABLE FUNCTION | SQL          |
    +---+---+---+

### bq

Use the [`bq ls` command](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#bq_ls)
with the `--routines` flag:

```bash
bq ls --routines DATASET
```

Replace the following:

- <var translate="no">DATASET</var>: the name of a dataset in your project.

Example:

```bash
bq ls --routines mydataset
```

             Id              Routine Type        Language    Creation Time    Last Modified Time
    --- --- --- --- ---
     AddFourAndDivide   SCALAR_FUNCTION         SQL        05 May 01:12:03   05 May 01:12:03
     create_customer    PROCEDURE               SQL        21 Apr 19:55:51   21 Apr 19:55:51
     names_by_year      TABLE_VALUED_FUNCTION   SQL        01 Sep 22:59:17   01 Sep 22:59:17

### API

Call the [`routines.list` method](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/routines/list)
with the dataset ID.

## View the body of a routine

To view the body of a routine, you must have the `bigquery.routines.get` permission.

### Console

1. In the Google Cloud console, open the BigQuery page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)
3. In the **Explorer** pane, expand your project, click **Datasets**, and
   then select a dataset.

4. Click the **Routines** tab.

5. Select the routine. The body of the routine is listed under **Routine
   query**.

### SQL

Select the `routine_definition` column of the
[`INFORMATION_SCHEMA.ROUTINES` view](https://docs.cloud.google.com/bigquery/docs/information-schema-routines):

<br />

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, enter the following statement:

   ```googlesql
   SELECT
     routine_definition
   FROM
     { DATASET | REGION }.INFORMATION_SCHEMA.ROUTINES
   WHERE
     routine_name = ROUTINE_NAME;
   ```

   <br />

3. Click **Run**.

   <br />

For more information about how to run queries, see [Run an interactive query](https://docs.cloud.google.com/bigquery/docs/running-queries#queries).

Replace the following:

- <var translate="no">DATASET</var>: the name of a dataset in your project.
- <var translate="no">REGION</var>: a [region qualifier](https://docs.cloud.google.com/bigquery/docs/information-schema-intro#region_qualifier).
- <var translate="no">ROUTINE_NAME</var>: the name of the routine.

Example:

```googlesql
SELECT
  routine_definition
FROM
  mydataset.INFORMATION_SCHEMA.ROUTINES
WHERE
  routine_name = 'AddFourAndDivide';
```

    +---+
    | routine_definition |
    +---+
    | (x + 4) / y        |
    +---+

### bq

Use the [`bq show` command](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#bq_show)
with the `--routine` flag:

```bash
bq show --routine DATASET.ROUTINE_NAME
```

Replace the following:

- <var translate="no">DATASET</var>: the name of a dataset in your project.
- <var translate="no">ROUTINE_NAME</var>: the name of the routine.

Example:

```bash
bq show --routine mydataset.AddFourAndDivide
```

             Id           Routine Type     Language             Signature             Definition     Creation Time    Last Modified Time
     --- --- --- --- --- --- ---
      AddFourAndDivide   SCALAR_FUNCTION   SQL        (x INT64, y INT64) -> FLOAT64   (x + 4) / y   05 May 01:12:03   05 May 01:12:03

### API

Call the [`routines.get` method](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/routines/get)
with the dataset ID and the name of the routine. The body of the
routine is returned in the
[`Routine` object](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/routines#Routine).

## Delete a routine

To delete a routine, you must have the `bigquery.routines.delete` permission.

### Console

1. In the Google Cloud console, open the BigQuery page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)
3. In the **Explorer** pane, expand your project, click **Datasets**, and
   then select a dataset.

4. Click the **Routines** tab.

5. Select the routine.

6. In the details pane, click **Delete**.

7. Type `"delete"` in the dialog, then click **Delete** to confirm.

### SQL

Depending on the routine type, run one of the following DDL statements:

- [Stored procedure: `DROP PROCEDURE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#drop_procedure_statement)
- [User-defined function: `DROP FUNCTION`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#drop_function_statement)
- [Table function: `DROP TABLE FUNCTION`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#drop_table_function)

Example:

    DROP FUNCTION IF EXISTS mydataset.AddFourAndDivide

### bq

Use the [`bq rm` command](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#bq_rm)
with the `--routine` flag:

```bash
bq rm --routine DATASET.ROUTINE_NAME
```

Replace the following:

- <var translate="no">DATASET</var>: the name of a dataset in your project.
- <var translate="no">ROUTINE_NAME</var>: the name of the routine.

Example:

    bq rm --routine mydataset.AddFourAndDivide

### API

Call the [`routines.delete` method](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/routines/delete)
with the dataset ID and the name of the routine.