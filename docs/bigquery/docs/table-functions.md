# Table functions

A table function, also called a table-valued function (TVF), is a user-defined
function that returns a table. You can use a table function anywhere that you
can use a table. Table functions behave similarly to views, but a table function
can take parameters.

## Create table functions

To create a table function, use the
[`CREATE TABLE FUNCTION`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_table_function_statement)
statement. A table function contains a query that produces a table. The function
returns the query result. The following table function takes an `INT64`
parameter and uses this value inside a `WHERE` clause in a query over a
[public dataset](https://docs.cloud.google.com/bigquery/public-data) called
`bigquery-public-data.usa_names.usa_1910_current`:

```googlesql
CREATE OR REPLACE TABLE FUNCTION mydataset.names_by_year(y INT64)
AS (
  SELECT year, name, SUM(number) AS total
  FROM `bigquery-public-data.usa_names.usa_1910_current`
  WHERE year = y
  GROUP BY year, name
);
```

To filter in other ways, you can pass multiple parameters to a table function.
The following table function filters the data by year and name prefix:

```googlesql
CREATE OR REPLACE TABLE FUNCTION mydataset.names_by_year_and_prefix(
  y INT64, z STRING)
AS (
  SELECT year, name, SUM(number) AS total
  FROM `bigquery-public-data.usa_names.usa_1910_current`
  WHERE
    year = y
    AND STARTS_WITH(name, z)
  GROUP BY year, name
);
```

### Table parameters

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

Note: To request support or provide feedback for this feature, email [bq-dcr-eng@google.com](mailto:bq-dcr-eng@google.com).

<br />

You can set TVF parameters to be tables. Following the table parameter
name, you must specify the required table schema explicitly, the same way that
you specify the fields of a struct. The table argument that you pass to the TVF
can contain additional columns besides those specified in the parameter schema,
and the columns can appear in any order.

The following table function returns a table that contains
total sales for `item_name` from the `orders` table:

```googlesql
CREATE TABLE FUNCTION mydataset.compute_sales (
  orders TABLE<sales INT64, item STRING>, item_name STRING)
AS (
  SELECT SUM(sales) AS total_sales, item
  FROM orders
  WHERE item = item_name
  GROUP BY item
);
```

### Parameter names

If a table function parameter matches the name of a table column, it can create
an ambiguous reference. In that case, BigQuery interprets the
name as a reference to the table column, not the parameter. The recommended
practice is to use parameter names that are distinct from the names of any
referenced table columns.

## Use table functions

You can call a table function in any context where a table is valid. The following
example calls the `mydataset.names_by_year` function in the `FROM` clause of
a `SELECT` statement:

    SELECT * FROM mydataset.names_by_year(1950)
      ORDER BY total DESC
      LIMIT 5

The results look like the following:

    +---+---+---+
    | year |  name  | total |
    +---+---+---+
    | 1950 | James  | 86447 |
    | 1950 | Robert | 83717 |
    | 1950 | Linda  | 80498 |
    | 1950 | John   | 79561 |
    | 1950 | Mary   | 65546 |
    +---+---+---+

You can join the output from a table function with another table:

    SELECT *
      FROM `bigquery-public-data.samples.shakespeare` AS s
      JOIN mydataset.names_by_year(1950) AS n
      ON n.name = s.word

You can also use a table function in a
[subquery](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/subqueries#array_subquery_concepts):

    SELECT ARRAY(
      SELECT name FROM mydataset.names_by_year(1950)
      ORDER BY total DESC
      LIMIT 5)

When you call a table function that has a table parameter, you must use the
`TABLE` keyword before the name of the table argument. The table argument can
have columns not listed in the table parameter schema:

```googlesql
CREATE TABLE FUNCTION mydataset.compute_sales (
  orders TABLE<sales INT64, item STRING>, item_name STRING)
AS (
  SELECT SUM(sales) AS total_sales, item
  FROM orders
  WHERE item = item_name
  GROUP BY item
);

WITH my_orders AS (
    SELECT 1 AS sales, "apple" AS item, 0.99 AS price
    UNION ALL
    SELECT 2, "banana", 0.49
    UNION ALL
    SELECT 5, "apple", 0.99)
SELECT *
FROM mydataset.compute_sales(TABLE my_orders, "apple");

/*---+---+
 | total_sales | item  |
 +---+---+
 | 6           | apple |
 +---+---*/
```

### Use system variables with TVFs

The `@@session_id` and `@@location`
[system variables](https://docs.cloud.google.com/bigquery/docs/reference/system-variables) are supported with
TVFs. You can include these system variables anywhere in your function creation
statement to return the session ID or location of the current query. All other
system variables aren't supported.

## List table functions

Table functions are a type of routine. To list all of the routines in a dataset,
see [List routines](https://docs.cloud.google.com/bigquery/docs/routines#list_routines).

## Delete table functions

To delete a table function, use the
[`DROP TABLE FUNCTION`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#drop_table_function)
statement:

    DROP TABLE FUNCTION mydataset.names_by_year

## Authorize routines

You can authorize table functions as *routines* .
Authorized routines let you share query results with specific users or groups
without giving them access to the underlying tables that generated the results.
For example, an authorized routine can compute an aggregation
over data or look up a table value and use that value in a computation.
For more information, see [Authorized routines](https://docs.cloud.google.com/bigquery/docs/authorized-routines).

## Limitations

- The query body must be a `SELECT` statement and cannot modify anything. For
  example, data definition language (DDL) and data manipulation language (DML)
  statements are not allowed in table functions. If you need side-effects,
  consider writing a
  [procedure](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_procedure)
  instead.

- Table functions must be stored in the same location as the tables they
  reference.

## Quotas

For more information about table function quotas and limits, see
[Quotas and limits](https://docs.cloud.google.com/bigquery/quotas#table_function_limits).