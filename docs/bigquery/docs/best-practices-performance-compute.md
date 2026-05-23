# Optimize query computation

This document provides the best practices for optimizing your query performance.

When you run a query, you can
[view the query plan](https://docs.cloud.google.com/bigquery/docs/query-insights)
in the Google Cloud console. You can also request execution details by using
the
[`INFORMATION_SCHEMA.JOBS*` views](https://docs.cloud.google.com/bigquery/docs/information-schema-jobs)
or the
[`jobs.get` REST API method](https://docs.cloud.google.com/bigquery/docs/query-plan-explanation#api_sample_representation).

The query plan includes details about query stages and steps. These details can
help you identify ways to improve query performance. For example, if you notice
a stage that writes a lot more output than other stages, it might mean that you
need to filter earlier in the query.

To learn more about the query plan and see examples of how the query plan
information can help you to improve query performance, see
[Get query performance insights](https://docs.cloud.google.com/bigquery/docs/query-insights).
After addressing the query performance insights, you can further optimize your
query by performing the following tasks:

- [Reduce data that is to be processed](https://docs.cloud.google.com/bigquery/docs/best-practices-performance-compute#reduce-data-processed)
- [Optimize query operations](https://docs.cloud.google.com/bigquery/docs/best-practices-performance-compute#optimize-query-operations)
- [Reduce the output of your query](https://docs.cloud.google.com/bigquery/docs/best-practices-performance-compute#reduce-query-output)
- [Avoid anti-SQL patterns](https://docs.cloud.google.com/bigquery/docs/best-practices-performance-compute#avoid-anti-sql-patterns)

## Reduce data processed

You can reduce data that needs to be processed by using the options described in
the following sections.

### Avoid `SELECT *`

**Best practice:** Control projection by querying only the columns that you
need.

Projection refers to the number of columns that are read by your query.
Projecting excess columns incurs additional (wasted) I/O and materialization
(writing results).

- **Use the data preview options.** If you are experimenting with data or exploring data, use one of the [data preview options](https://docs.cloud.google.com/bigquery/docs/best-practices-costs#preview-data) instead of `SELECT *`.
- **Query specific columns.** Applying a `LIMIT` clause to a `SELECT *` query does not affect the amount of data read. You are billed for reading all bytes in the entire table, and the query counts against your free tier quota. Instead, query only the columns you need. For example, use `SELECT * EXCEPT` to exclude one or more columns from the results.
- **Use partitioned tables.** If you do require queries against every column in a table, but only against a subset of data, consider:

  <br />

  - Materializing results in a destination table and querying that table instead.
  - [Partitioning your tables](https://docs.cloud.google.com/bigquery/docs/creating-partitioned-tables) and [querying the relevant partition](https://docs.cloud.google.com/bigquery/docs/querying-partitioned-tables). For example, use `WHERE _PARTITIONDATE="2017-01-01"` to query only the January 1, 2017 partition.

<!-- -->

- **Use `SELECT * EXCEPT`** . Querying a subset of data or using `SELECT * EXCEPT`
  can greatly reduce the amount of data that is read by a query. In addition to
  the cost savings, performance is improved by reducing the amount of data I/O
  and the amount of materialization that is required for the query results.

  ```googlesql
  SELECT * EXCEPT (col1, col2, col5)
  FROM mydataset.newtable
  ```

### Avoid excessive wildcard tables

**Best practice:** When querying [wildcard tables](https://docs.cloud.google.com/bigquery/docs/querying-wildcard-tables),
you must use the most granular prefix.

Use wildcards to query multiple tables by using concise SQL statements. Wildcard
tables are a union of tables that match the wildcard expression. Wildcard tables
are useful if your dataset contains the following resources:

- Multiple, similarly named tables with compatible schemas
- Sharded tables

> [!NOTE]
> **Note:** If your data allows it, use time-partitioned tables instead of sharded tables. For more information, see [Avoid oversharding tables](https://docs.cloud.google.com/bigquery/docs/best-practices-performance-compute#avoid-oversharding-tables).

When you query a wildcard table, specify a wildcard (`*`) after the common
table prefix. For example, `` FROM `bigquery-public-data.noaa_gsod.gsod194*` ``
queries all tables from the 1940s.

More granular prefixes perform better than shorter prefixes. For example,
`` FROM `bigquery-public-data.noaa_gsod.gsod194*` `` performs better than
`` FROM `bigquery-public-data.noaa_gsod.*` `` because fewer tables match
the wildcard.

### Avoid tables sharded by date

**Best practice:** Don't use tables sharded by date (also called date-named
tables) in place of time-partitioned tables.

[Partitioned tables](https://docs.cloud.google.com/bigquery/docs/partitioned-tables) perform better than
date-named tables. When you create tables sharded by date,
BigQuery must maintain a copy of the schema and metadata for each
date-named table. Also, when date-named tables are used, BigQuery
might be required to verify permissions for each queried table. This practice
also adds to query overhead and impacts query performance.

### Avoid oversharding tables

**Best practice:** Avoid creating too many table shards. If you are sharding
tables by date, use time-partitioned tables instead.

Table sharding refers to dividing large datasets into separate tables and adding
a suffix to each table name. If you are sharding tables by date, use
[time-partitioned tables](https://docs.cloud.google.com/bigquery/docs/creating-partitioned-tables) instead.

Because of the low cost of BigQuery storage, you don't need to
optimize your tables for cost as you would in a relational database system.
Creating a large number of table shards has performance impacts that outweigh
any cost benefits.

Sharded tables require BigQuery to maintain schema, metadata,
and permissions for each shard. Because of the added overhead required to
maintain information on each shard, oversharding tables can impact query
performance.

The amount and source of data read by a query can impact query
performance and cost.

### Prune partitioned queries

**Best practice:** When querying a [partitioned table](https://docs.cloud.google.com/bigquery/docs/querying-partitioned-tables),
to filter with partitions on partitioned tables, use the following columns:

- For ingestion-time partitioned tables, use the pseudocolumn `_PARTITIONTIME`
- For partitioned tables such as the time-unit column-based and integer-range, use the *partitioning column*.

For time-unit partitioned tables, filtering the data with `_PARTITIONTIME` or
*partitioning column* lets you specify a date or range of dates. For example,
the following `WHERE` clause uses the `_PARTITIONTIME` pseudocolumn to specify
partitions between January 1, 2016 and January 31, 2016:

    WHERE _PARTITIONTIME
    BETWEEN TIMESTAMP("20160101")
    AND TIMESTAMP("20160131")

The query processes data only in the partitions that are indicated by the date
range. Filtering your partitions improves query performance and reduces costs.

### Reduce data before using a `JOIN`

**Best practice:** Reduce the amount of data that is processed before a `JOIN`
clause by performing aggregations.

Using a [`GROUP BY` clause](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#group_by_clause)
with [aggregate functions](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions)
is computationally intensive, because these types of queries use
[shuffle](https://cloud.google.com/blog/products/bigquery/in-memory-query-execution-in-google-bigquery).
As these queries are computationally intensive, you must use a `GROUP BY`
clause only when necessary.

For queries with `GROUP BY` and `JOIN`, perform aggregation earlier in the query
to reduce the amount of data processed.
For example, the following query performs a `JOIN` on two large tables without
any filtering beforehand:

```googlesql
WITH
  users_posts AS (
  SELECT *
  FROM
    `bigquery-public-data`.stackoverflow.comments AS c
  JOIN
    `bigquery-public-data`.stackoverflow.users AS u
  ON
    c.user_id = u.id
  )
SELECT
  user_id,
  ANY_VALUE(display_name) AS display_name,
  ANY_VALUE(reputation) AS reputation,
  COUNT(text) AS comments_count
FROM users_posts
GROUP BY user_id
ORDER BY comments_count DESC
LIMIT 20;
```

This query pre-aggregates the comment counts which reduces the amount of data
read for the `JOIN`:

```googlesql
WITH
  comments AS (
  SELECT
    user_id,
    COUNT(text) AS comments_count
  FROM
    `bigquery-public-data`.stackoverflow.comments
  WHERE
    user_id IS NOT NULL
  GROUP BY user_id
  ORDER BY comments_count DESC
  LIMIT 20
  )
SELECT
  user_id,
  display_name,
  reputation,
  comments_count
FROM comments
JOIN
  `bigquery-public-data`.stackoverflow.users AS u
ON
  user_id = u.id
ORDER BY comments_count DESC;
```

> [!NOTE]
> **Note:** `WITH` clauses with common table expressions (CTEs) are used for query readability, not performance. There is no guarantee that adding a `WITH` clause causes BigQuery to materialize temporary intermediate tables and reuse the temporary result for multiple references. The `WITH` clause might be evaluated multiple times within a query, depending on query optimizer decisions.

### Use the `WHERE` clause

**Best practice:** Use a
[`WHERE` clause](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#where_clause)
to limit the amount of data a query returns. When possible, use `BOOL`,
`INT64`, `FLOAT64`, or `DATE` columns in the `WHERE` clause.

Operations on `BOOL`, `INT64`, `FLOAT64`, and `DATE` columns are typically faster
than operations on `STRING` or `BYTE` columns. When possible, use a column that
uses one of these data types in the `WHERE` clause to reduce the amount of
data returned by the query.

### Use materialized views

**Best practice:** Use materialized views to precompute the results of a query
for increased performance and efficiency.

[Materialized views](https://docs.cloud.google.com/bigquery/docs/materialized-views-intro) are precomputed views that
periodically cache the results of a query for increased performance and
efficiency. BigQuery leverages precomputed results from
materialized views and [whenever
possible](https://docs.cloud.google.com/bigquery/docs/materialized-views-use#incremental_updates) reads only
changes from the base tables to compute up-to-date results. Materialized
views can be queried directly or can be used by the
BigQuery optimizer to process queries to the base tables.

### Use BI Engine

**Best practice:** Use BigQuery BI Engine to accelerate queries by caching the
data that you use most frequently.

Consider adding a [BI Engine](https://docs.cloud.google.com/bigquery/docs/bi-engine-query)
reservation to the project where the queries are being computed.
BigQuery BI Engine uses a vectorized query engine to accelerate the `SELECT`
query performance.

### Use search indexes

**Best practice:** Use search indexes for efficient row lookups when you need
to find individual rows of data in large tables.

A [search index](https://docs.cloud.google.com/bigquery/docs/search-intro) is a data structure designed
to enable very efficient search with the [`SEARCH` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/search_functions#search)
but can also accelerate queries using [other operators and functions](https://docs.cloud.google.com/bigquery/docs/search#operator_and_function_optimization),
such as the equal (`=`), `IN`, or `LIKE` operators and certain string and JSON
functions.

## Optimize query operations

You can optimize your query operations by using the options described in the
following sections.

### Avoid repeatedly transforming data

**Best practice:** If you are using SQL to perform ETL operations, then avoid
situations where you are repeatedly transforming the same data.

For example, if you are using SQL to trim strings or extract data by using
regular expressions, it is more performant to materialize the transformed
results in a destination table. Functions like regular expressions require
additional computation. Querying the destination table without the added
transformation overhead is much more efficient.

### Avoid multiple evaluations of the same CTEs

**Best practice** : Use [procedural language](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/procedural-language),
variables, [temporary tables](https://docs.cloud.google.com/bigquery/docs/multi-statement-queries#temporary_tables),
and automatically expiring tables to persist calculations and use them later in
the query.

When your query contains [common table expressions (CTEs)](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#with_clause)
that are used in multiple places in the query, they might end up being evaluated
each time they are referenced. The query optimizer attempts to detect parts of
the query that could be executed only once, but this might not always be
possible. As a result, using a CTE might not help reduce internal query
complexity and resource consumption.

You can store the result of a CTE in a scalar variable or a temporary table
depending on the data that the CTE returns.

### Avoid repeated joins and subqueries

**Best practice:** Avoid repeatedly joining the same tables and using the same
subqueries.

Instead of repeatedly joining the data, it might be more performant for you to
use nested repeated data to represent the relationships. Nested repeated data
saves you the performance impact of the communication bandwidth that a join
requires. It also saves you the I/O costs that you incur by repeatedly
reading and writing the same data. For more information, see
[use nested and repeated fields](https://docs.cloud.google.com/bigquery/docs/best-practices-performance-nested).

Similarly, repeating the same subqueries affects performance through repetitive
query processing. If you are using the same subqueries in multiple queries,
consider materializing the subquery results in a table. Then consume the
materialized data in your queries.

Materializing your subquery results improves performance and reduces the overall
amount of data that BigQuery reads and writes. The small
cost of storing the materialized data outweighs the performance impact of
repeated I/O and query processing.

### Optimize your join patterns

**Best practice:** For queries that join data from multiple tables, optimize
your join patterns by starting with the largest table.

When you create a query by using a `JOIN` clause, consider the order in which you are
merging the data. The GoogleSQL query optimizer determines which table
should be on which side of the join. As a best practice, place the
table with the largest number of rows first, followed by the table with the
fewest rows, and then place the remaining tables by decreasing size.

When you have a large table as the left side of the `JOIN` and a small one on
the right side of the `JOIN`, a broadcast join is created. A broadcast join
sends all the data in the smaller table to each slot that processes the larger
table. It is advisable to perform the broadcast join first.

To view the size of the tables in your `JOIN`, see
[Get information about tables](https://docs.cloud.google.com/bigquery/docs/tables#get_information_about_tables).

### Specify primary key and foreign key constraints

**Best practice:** Specify key constraints in the table schema when table data
satisfies the data integrity requirements of
[primary key or foreign key constraints](https://docs.cloud.google.com/bigquery/docs/primary-foreign-keys).
The query engine can use the key constraints to optimize query plans.

BigQuery doesn't automatically check for data integrity, so you
must ensure that your data meets the constraints specified in the table schema.
If you don't maintain data integrity
in tables with specified constraints, your query results might be inaccurate.

### Optimize the `ORDER BY` clause

**Best practice:** When you use the `ORDER BY` clause, ensure that you follow
the best practices:

- **Use `ORDER BY` in the outermost query or within
  [window clauses](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/window-function-calls).**
  Push complex operations to the end of the query.
  Placing an `ORDER BY` clause in the middle of a query greatly impacts
  performance unless it is being used in a window function.

  Another technique for ordering your query is to push complex operations, such as
  regular expressions and mathematical functions, to the end of the query.
  This technique reduces the data to be processed before the complex operations
  are performed.
- **Use a `LIMIT` clause.** If you are ordering a very large number of values
  but don't need to have all of them returned, use a `LIMIT` clause.
  For example, the following query orders a very large result set and throws a
  `Resources exceeded` error. The query sorts by the `title` column in `mytable`.
  The `title` column contains millions of values.

  ```googlesql
  SELECT
  title
  FROM
  `my-project.mydataset.mytable`
  ORDER BY
  title;
  ```

  To remove the error, use a query like the following:

  ```googlesql
  SELECT
  title
  FROM
  `my-project.mydataset.mytable`
  ORDER BY
  title DESC
  LIMIT
  1000;
  ```
- **Use a window function.** If you are ordering a very large number of values,
  use a window function, and limit data before calling the window function.
  For example, the following query lists the ten oldest
  Stack Overflow users and their ranking, with the oldest account being ranked
  lowest:

  ```googlesql
  SELECT
  id,
  reputation,
  creation_date,
  DENSE_RANK() OVER (ORDER BY creation_date) AS user_rank
  FROM bigquery-public-data.stackoverflow.users
  ORDER BY user_rank ASC
  LIMIT 10;
  ```

  This query takes approximately 15 seconds to run. This query uses `LIMIT` at
  the end of the query, but not in the `DENSE_RANK() OVER` window function.
  Because of this, the query requires all of the data to be sorted on a
  single worker node.

  Instead, you should limit the dataset before computing the window function
  in order to improve performance:

  ```googlesql
  WITH users AS (
  SELECT
  id,
  reputation,
  creation_date,
  FROM bigquery-public-data.stackoverflow.users
  ORDER BY creation_date ASC
  LIMIT 10)
  SELECT
  id,
  reputation,
  creation_date,
  DENSE_RANK() OVER (ORDER BY creation_date) AS user_rank
  FROM users
  ORDER BY user_rank;
  ```

  This query takes approximately 2 seconds to run, while returning the
  same results as the previous query.

  One caveat is that the `DENSE_RANK()` function ranks the data within years, so
  for ranking data that spans across multiple years, these queries don't give
  identical results.

### Split complex queries into smaller ones

**Best practice** : Leverage [multi-statement query](https://docs.cloud.google.com/bigquery/docs/multi-statement-queries)
capabilities and [stored procedures](https://docs.cloud.google.com/bigquery/docs/procedures)
to perform the computations that were designed as one complex query as multiple
smaller and simpler queries instead.

Complex queries, `REGEX` functions, and layered subqueries or joins can be slow
and resource intensive to run. Trying to fit all computations in one huge
`SELECT` statement, for example to make it a view, is sometimes an antipattern,
and it can result in a slow, resource-intensive query. In extreme cases, the
internal query plan becomes so complex that BigQuery is unable to
execute it.

Splitting up a complex query allows for materializing intermediate results in
variables or
[temporary tables](https://docs.cloud.google.com/bigquery/docs/multi-statement-queries#temporary_tables).
You can then use these intermediate results in other parts of the query. It is
increasingly useful when these results are needed in more than one place of the
query.

Often it lets you better express the true intent of parts of
the query with temporary tables being the data materialization points.

### Use nested and repeated fields

For information about how to denormalize data storage using nested and repeated
fields, see [Use nested and repeated fields](https://docs.cloud.google.com/bigquery/docs/best-practices-performance-nested).

### Use `INT64` data types in joins

**Best practice:** Use `INT64` data types in joins instead of `STRING` data
types to reduce cost and improve comparison performance.

BigQuery doesn't index primary keys like traditional databases,
so the wider the join column is, the longer the comparison takes. Therefore,
using `INT64` data types in joins is cheaper and more efficient than using
`STRING` data types.

## Reduce query outputs

You can reduce the query outputs by using the options described in the following
the sections.

### Materialize large result sets

**Best practice:** Consider [materializing large result sets](https://docs.cloud.google.com/bigquery/docs/writing-results#large-results)
to a destination table. Writing large result sets has performance and cost
impacts.

BigQuery limits cached results to approximately 10 GB
compressed. Queries that return larger results overtake this limit and
frequently result in the following error: [`Response too large`](https://docs.cloud.google.com/bigquery/troubleshooting-errors#responseTooLarge).

This error often occurs when you select a large number of fields from a table
with a considerable amount of data. Issues writing cached results can also occur
in ETL-style queries that normalize data without reduction or aggregation.

You can overcome the limitation on cached result size by using the following
options:

- Use filters to limit the result set
- Use a `LIMIT` clause to reduce the result set, especially if you are using an `ORDER BY` clause
- Write the output data to a destination table

You can page through the results using the BigQuery REST API. For more
information, see [Paging through table data](https://docs.cloud.google.com/bigquery/docs/paging-results).

> [!NOTE]
> **Note:** Writing very large result sets to destination tables impacts query performance (I/O). In addition, you incur a small cost for storing the destination table. You can automatically delete a large destination table by using the dataset's [default table expiration](https://docs.cloud.google.com/bigquery/docs/datasets#create-dataset). For more information, see [Use the expiration settings](https://docs.cloud.google.com/bigquery/docs/best-practices-storage#use-expiration-settings) in the storage best practices.

## Avoid anti-SQL patterns

The following best practices provide guidance on avoiding query anti-patterns
that impact performance in BigQuery.

### Avoid self joins

**Best practice:** Instead of using self-joins, use a
[window (analytic) function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/analytic-function-concepts)
or the
[`PIVOT` operator](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#pivot_operator).

Typically, self-joins are used to compute row-dependent relationships. The
result of using a self-join is that it potentially squares the number of output
rows. This increase in output data can cause poor performance.

### Avoid cross joins

**Best practice:** Avoid joins that generate more outputs than inputs. When a
`CROSS JOIN` is required, pre-aggregate your data.

Cross joins are queries where each row from the first table is joined to every
row in the second table, with non-unique keys on both sides. The worst
case output is the number of rows in the left table multiplied by the number of
rows in the right table. In extreme cases, the query might not finish.

If the query job completes, the query plan explanation shows output rows
versus input rows. You can confirm a [Cartesian product](https://en.wikipedia.org/wiki/Relational_algebra#Selection_and_cross_product)
by modifying the query to print the number of rows on each side of the `JOIN`
clause, grouped by the join key. You can also check the performance insights
in the query execution graph for a
[high cardinality join](https://docs.cloud.google.com/bigquery/docs/query-insights#high_cardinality_join).

To avoid performance issues associated with joins that generate more outputs
than inputs:

- Use a `GROUP BY` clause to [pre-aggregate the data](https://docs.cloud.google.com/bigquery/docs/best-practices-performance-compute#reduce_data_before_using_a_join).
- Use a window function. Window functions are often more efficient than using a cross join. For more information, see [window functions](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/window-function-calls).

### Avoid DML statements that update or insert single rows

**Best practice:** Avoid [DML](https://docs.cloud.google.com/bigquery/docs/data-manipulation-language)
statements that update or insert single rows. Batch your updates and
inserts.

Using point-specific DML statements is an attempt to treat
BigQuery like an Online Transaction Processing (OLTP) system.
BigQuery focuses on Online Analytical Processing (OLAP) by using
table scans and not point lookups. If you need OLTP-like behavior (single-row
updates or inserts), consider a database designed to support OLTP use cases such
as [Cloud SQL](https://docs.cloud.google.com/sql/docs).

BigQuery DML statements are intended for bulk updates. `UPDATE`
and `DELETE` DML statements in BigQuery are oriented towards
periodic rewrites of your data, not single row mutations. The `INSERT` DML
statement is intended to be used sparingly. Inserts consume the same
modification [quotas](https://docs.cloud.google.com/bigquery/quotas#data-manipulation-language-statements)
as load jobs. If your use case involves frequent single row inserts, consider
[streaming](https://docs.cloud.google.com/bigquery/docs/streaming-data-into-bigquery) your data instead.

If batching your `UPDATE` statements yields many tuples in very long queries,
you might approach the query length limit of 256 KB. To work around the query
length limit, consider whether your updates can be handled based on a logical
criteria instead of a series of direct tuple replacements.

For example, you could load your set of replacement records into another table,
then write the DML statement to update all values in the original table if the
non-updated columns match. For example, if the original data is in table `t` and
the updates are staged in table `u`, the query would look like the following:

```googlesql
UPDATE
  dataset.t t
SET
  my_column = u.my_column
FROM
  dataset.u u
WHERE
  t.my_key = u.my_key
```

### Use alias names for similarly named columns

**Best Practice:** Use column and table aliases when you work with similarly
named columns across queries, including subqueries.

Aliases help to identify which columns and tables are referenced in addition to
your initial reference of the column. Using aliases can help you understand
and address problems in your SQL query, including finding the columns that are used
in subqueries.

## What's next

- Learn how to [optimize cost](https://docs.cloud.google.com/bigquery/docs/best-practices-costs).
- Learn how to [optimize storage](https://docs.cloud.google.com/bigquery/docs/best-practices-storage).
- Learn how to [optimize functions](https://docs.cloud.google.com/bigquery/docs/best-practices-performance-functions).