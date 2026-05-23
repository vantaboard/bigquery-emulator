[Video](https://www.youtube.com/watch?v=mW2CLYr6w4M)

Pipe query syntax is an extension to GoogleSQL that supports a linear
query structure designed to make your queries easier to read, write, and
maintain. You can use pipe syntax anywhere you write GoogleSQL.

Pipe syntax supports the same operations as existing [GoogleSQL query
syntax](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax), or *standard syntax*---for instance, selection,
aggregation and grouping, joining, and filtering---but the operations can be
applied in any order, any number of times. The linear structure of pipe syntax
lets you write queries so that the order of the query syntax matches the order
of logical steps taken to build the result table.

Queries that use pipe syntax are priced, executed, and optimized the same way
as their equivalent standard syntax queries. When you write queries with pipe
syntax, follow the guidelines to
[estimate costs](https://docs.cloud.google.com/bigquery/docs/best-practices-costs) and
[optimize query computation](https://docs.cloud.google.com/bigquery/docs/best-practices-performance-compute).

Standard syntax suffers from issues that can make it
difficult to read, write, and maintain. The following table shows how pipe
syntax addresses these issues:

| Standard syntax | Pipe syntax |
|---|---|
| Clauses must appear in a particular order. | Pipe operators can be applied in any order. |
| More complex queries, such as queries with multi-level aggregation, usually require CTEs or nested subqueries. | More complex queries are usually expressed by adding pipe operators to the end of the query. |
| During aggregation, columns are repeated in the `SELECT`, `GROUP BY`, and `ORDER BY` clauses. | Columns can be listed only once per aggregation. |

To build up a complex query step by step in pipe syntax, see
[Analyze data using pipe syntax](https://docs.cloud.google.com/bigquery/docs/analyze-data-pipe-syntax).
For full syntax details, see the [Pipe query syntax](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/pipe-syntax) reference
documentation.

## Basic syntax

In pipe syntax, queries start with a standard SQL query or a `FROM` clause. For
example, a standalone `FROM` clause, such as `FROM MyTable`, is valid
pipe syntax. The result of the standard SQL query or the table from the `FROM`
clause can then be passed as input to a pipe symbol, `|>`, followed by a pipe
operator name and any arguments to that operator. The pipe operator transforms
the table in some way, and the result of that transformation can be passed to
another pipe operator.

You can use any number of pipe operators in your query to do things such as
select, order, filter, join, or aggregate columns. The names of pipe operators
match their standard syntax counterparts and generally have the same behavior.
The main difference between standard syntax and pipe syntax is the way you
structure your query. As the logic expressed by your query becomes more complex,
the query can still be expressed as a linear sequence of pipe operators, without
using deeply nested subqueries, making it easier to read and understand.

Pipe syntax has the following key characteristics:

- Each pipe operator in pipe syntax consists of the pipe symbol, `|>`, an operator name, and any arguments:   
  `|> operator_name argument_list`
- Pipe operators can be added to the end of any valid query.
- Pipe operators can be applied in any order, any number of times.
- Pipe syntax works anywhere standard syntax is supported: in queries, views, table-valued functions, and other contexts.
- Pipe syntax can be mixed with standard syntax in the same query. For example, subqueries can use different syntax from the parent query.
- A pipe operator can see every alias that exists in the table preceding the pipe.
- A query can [start with a `FROM` clause](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/pipe-syntax#from_queries), and pipe operators can optionally be added after the `FROM` clause.

Consider the following table:

    CREATE OR REPLACE TABLE mydataset.Produce AS (
      SELECT 'apples' AS item, 2 AS sales, 'fruit' AS category
      UNION ALL
      SELECT 'apples' AS item, 7 AS sales, 'fruit' AS category
      UNION ALL
      SELECT 'carrots' AS item, 0 AS sales, 'vegetable' AS category
      UNION ALL
      SELECT 'bananas' AS item, 15 AS sales, 'fruit' AS category);

The following queries each contain valid pipe syntax that shows how you can
build a query sequentially.

Queries can
[start with a `FROM` clause](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/pipe-syntax#from_queries)
and don't need to contain a pipe symbol:

    -- View the table.
    FROM mydataset.Produce;

    /*---+---+---+
     | item    | sales | category  |
     +---+---+---+
     | apples  | 7     | fruit     |
     | apples  | 2     | fruit     |
     | carrots | 0     | vegetable |
     | bananas | 15    | fruit     |
     +---+---+---*/

You can filter with a [`WHERE` pipe operator](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/pipe-syntax#where_pipe_operator):

    -- Filter items with no sales.
    FROM mydataset.Produce
    |> WHERE sales > 0;

    /*---+---+---+
     | item    | sales | category  |
     +---+---+---+
     | apples  | 7     | fruit     |
     | apples  | 2     | fruit     |
     | bananas | 15    | fruit     |
     +---+---+---*/

To perform aggregation, use the [`AGGREGATE` pipe
operator](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/pipe-syntax#aggregate_pipe_operator), followed by any number of aggregate
functions, followed by a `GROUP BY` clause. The `GROUP BY` clause is part of the
`AGGREGATE` pipe operator and isn't separated by a pipe symbol (`|>`).

    -- Compute total sales by item.
    FROM mydataset.Produce
    |> WHERE sales > 0
    |> AGGREGATE SUM(sales) AS total_sales, COUNT(*) AS num_sales
       GROUP BY item;

    /*---+---+---+
     | item    | total_sales | num_sales |
     +---+---+---+
     | apples  | 9           | 2         |
     | bananas | 15          | 1         |
     +---+---+---*/

Now suppose you have the following table that contains an ID for each item:

    CREATE OR REPLACE TABLE mydataset.ItemData AS (
      SELECT 'apples' AS item, '123' AS id
      UNION ALL
      SELECT 'bananas' AS item, '456' AS id
      UNION ALL
      SELECT 'carrots' AS item, '789' AS id
    );

You can use the [`JOIN` pipe operator](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/pipe-syntax#join_pipe_operator) to join the results
of the previous query with this table to include each item's ID:

    FROM mydataset.Produce
    |> WHERE sales > 0
    |> AGGREGATE SUM(sales) AS total_sales, COUNT(*) AS num_sales
       GROUP BY item
    |> JOIN mydataset.ItemData USING(item);

    /*---+---+---+---+
     | item    | total_sales | num_sales | id  |
     +---+---+---+---+
     | apples  | 9           | 2         | 123 |
     | bananas | 15          | 1         | 456 |
     +---+---+---+---*/

## Key differences from standard syntax

Pipe syntax differs from standard syntax in the following ways:

- Queries can [start with a `FROM` clause](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/pipe-syntax#from_queries).
- The `SELECT` pipe operator doesn't perform aggregation. You must use the [`AGGREGATE` pipe operator](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/pipe-syntax#aggregate_pipe_operator) instead.
- Filtering is always done with the [`WHERE` pipe
  operator](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/pipe-syntax#where_pipe_operator), which can be applied anywhere. The `WHERE` pipe operator, which replaces `HAVING` and `QUALIFY`, can filter the results of aggregation or window functions.

For more details, see the complete list of [pipe operators](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/pipe-syntax#pipe_operators).

## Use cases

Common use cases for pipe syntax include the following:

- **Ad-hoc analysis and incremental query building** : The logical order of operations makes it easier to write and debug queries. The prefix of any query up to a pipe symbol `|>` is a valid query, which helps you view intermediate results in a long query. The productivity gains can speed up the development process across your organization.
- **Log analytics** : There exist other types of pipe-like syntax that are popular among log analytics users. Pipe syntax provides a familiar structure that simplifies onboarding for those users to [Observability Analytics](https://docs.cloud.google.com/logging/docs/log-analytics#analytics) and BigQuery.

## Additional features in pipe syntax

With few exceptions, pipe syntax supports all operators that standard syntax
does with the same syntax. In addition, pipe syntax introduces additional pipe
operators and uses a modified syntax for aggregations and joins. The following
sections explain some of these operators. For all supported operators, see the
complete list of [pipe operators](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/pipe-syntax#pipe_operators).

### `EXTEND` pipe operator

The [`EXTEND` pipe operator](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/pipe-syntax#extend_pipe_operator) lets you append computed
columns to the current table. The `EXTEND` pipe operator is similar to the
`SELECT *, new_column` statement, but it gives you more flexibility in
referencing column aliases.

Consider the following table that contains two test scores for each person:

    CREATE OR REPLACE TABLE mydataset.Scores AS (
      SELECT 'Alex' AS student, 9 AS score1, 10 AS score2, 10 AS points_possible
      UNION ALL
      SELECT 'Dana' AS student, 5 AS score1, 7 AS score2, 10 AS points_possible);

    /*---+---+---+---+
     | student | score1 | score2 | points_possible |
     +---+---+---+---+
     | Alex    | 9      | 10     | 10              |
     | Dana    | 5      | 7      | 10              |
     +---+---+---+---*/

Suppose you want to compute the average raw score and average percentage score
that each student received on the test. In standard syntax, later columns in
a `SELECT` statement don't have visibility to earlier aliases. To avoid a
subquery, you have to repeat the expression for the average:

    SELECT student,
      (score1 + score2) / 2 AS average_score,
      (score1 + score2) / 2 / points_possible AS average_percent
    FROM mydataset.Scores;

The `EXTEND` pipe operator can reference previously used aliases, making the
query easier to read and less error prone:

    FROM mydataset.Scores
    |> EXTEND (score1 + score2) / 2 AS average_score
    |> EXTEND average_score / points_possible AS average_percent
    |> SELECT student, average_score, average_percent;

    /*---+---+---+
     | student | average_score | average_percent |
     +---+---+---+
     | Alex    | 9.5           | .95             |
     | Dana    | 6.0           | 0.6             |
     +---+---+---*/

### `SET` pipe operator

The [`SET` pipe operator](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/pipe-syntax#set_pipe_operator) lets you replace the value of
columns in the current table. The `SET` pipe operator is similar to the `SELECT
* REPLACE (expression AS column)` statement. You can reference the original
value by qualifying the column name with a table alias.

    FROM (SELECT 3 AS x, 5 AS y)
    |> SET x = 2 * x;

    /*---+---+
     | x | y |
     +---+---+
     | 6 | 5 |
     +---+---*/

### `DROP` pipe operator

The [`DROP` pipe operator](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/pipe-syntax#drop_pipe_operator) lets you remove columns from the
current table. The `DROP` pipe operator is similar to the `SELECT *
EXCEPT(column)` statement. After a column is dropped you can still reference the
original value by qualifying the column name with a table alias.

    FROM (SELECT 1 AS x, 2 AS y) AS t
    |> DROP x;

    /*---+
     | y |
     +---+
     | 2 |
     +---*/

### `RENAME` pipe operator

The [`RENAME` pipe operator](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/pipe-syntax#rename_pipe_operator) lets you rename columns from
the current table. The `RENAME` pipe operator is similar to the `SELECT *
EXCEPT(old_column), old_column AS new_column` statement.

    FROM (SELECT 1 AS x, 2 AS y, 3 AS z) AS t
    |> RENAME y AS w;

    /*---+---+---+
     | x | w | z |
     +---+---+---+
     | 1 | 2 | 3 |
     +---+---+---*/

### `AGGREGATE` pipe operator

To perform aggregation in pipe syntax, use the [`AGGREGATE` pipe
operator](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/pipe-syntax#aggregate_pipe_operator), followed by any number of aggregate
functions, followed by a `GROUP BY` clause. You don't need to repeat columns in
a `SELECT` clause.

The examples in this section use the `Produce` table:

    CREATE OR REPLACE TABLE mydataset.Produce AS (
      SELECT 'apples' AS item, 2 AS sales, 'fruit' AS category
      UNION ALL
      SELECT 'apples' AS item, 7 AS sales, 'fruit' AS category
      UNION ALL
      SELECT 'carrots' AS item, 0 AS sales, 'vegetable' AS category
      UNION ALL
      SELECT 'bananas' AS item, 15 AS sales, 'fruit' AS category);

    /*---+---+---+
     | item    | sales | category  |
     +---+---+---+
     | apples  | 7     | fruit     |
     | apples  | 2     | fruit     |
     | carrots | 0     | vegetable |
     | bananas | 15    | fruit     |
     +---+---+---*/

    FROM mydataset.Produce
    |> AGGREGATE SUM(sales) AS total, COUNT(*) AS num_records
       GROUP BY item, category;

    /*---+---+---+---+
     | item    | category  | total | num_records |
     +---+---+---+---+
     | apples  | fruit     | 9     | 2           |
     | carrots | vegetable | 0     | 1           |
     | bananas | fruit     | 15    | 1           |
     +---+---+---+---*/

If you are ready to order your results immediately following aggregation, you
can mark the columns in the `GROUP BY` clause that you want to order with
`ASC` or `DESC`. Unmarked columns aren't ordered.

If you want to order all columns, then you can replace the `GROUP BY` clause
with a [`GROUP AND ORDER BY` clause](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/pipe-syntax#shorthand_order_pipe_syntax), which orders
every column in ascending order by default. You can specify `DESC` following the
columns that you want to order in descending order. For example, the following
three queries are equivalent:

    -- Use a separate ORDER BY clause.
    FROM mydataset.Produce
    |> AGGREGATE SUM(sales) AS total, COUNT(*) AS num_records
       GROUP BY category, item
    |> ORDER BY category DESC, item;

    -- Explicitly mark how to order columns in the GROUP BY clause.
    FROM mydataset.Produce
    |> AGGREGATE SUM(sales) AS total, COUNT(*) AS num_records
       GROUP BY category DESC, item ASC;

    -- Only mark descending columns in the GROUP AND ORDER BY clause.
    FROM mydataset.Produce
    |> AGGREGATE SUM(sales) AS total, COUNT(*) AS num_records
       GROUP AND ORDER BY category DESC, item;

The advantage of using a `GROUP AND ORDER BY` clause is that you don't have to
repeat column names in two places.

To perform full table aggregation, use `GROUP BY()` or omit the `GROUP BY`
clause entirely:

    FROM mydataset.Produce
    |> AGGREGATE SUM(sales) AS total, COUNT(*) AS num_records;

    /*---+---+
     | total | num_records |
     +---+---+
     | 24    | 4           |
     +---+---*/

### `JOIN` pipe operator

The [`JOIN` pipe operator](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/pipe-syntax#join_pipe_operator) lets you join the current table
with another table and supports the standard [join operations](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#join_types),
including `CROSS`, `INNER`, `LEFT`, `RIGHT`, and `FULL`.

The following examples reference the `Produce` and `ItemData` tables:

    CREATE OR REPLACE TABLE mydataset.Produce AS (
      SELECT 'apples' AS item, 2 AS sales, 'fruit' AS category
      UNION ALL
      SELECT 'apples' AS item, 7 AS sales, 'fruit' AS category
      UNION ALL
      SELECT 'carrots' AS item, 0 AS sales, 'vegetable' AS category
      UNION ALL
      SELECT 'bananas' AS item, 15 AS sales, 'fruit' AS category);

    CREATE OR REPLACE TABLE mydataset.ItemData AS (
      SELECT 'apples' AS item, '123' AS id
      UNION ALL
      SELECT 'bananas' AS item, '456' AS id
      UNION ALL
      SELECT 'carrots' AS item, '789' AS id
    );

The following example uses a `USING` clause and avoids column ambiguity:

    FROM mydataset.Produce
    |> JOIN mydataset.ItemData USING(item)
    |> WHERE item = 'apples';

    /*---+---+---+---+
     | item   | sales | category | id  |
     +---+---+---+---+
     | apples | 2     | fruit    | 123 |
     | apples | 7     | fruit    | 123 |
     +---+---+---+---*/

To reference columns in the current table, such as to disambiguate columns in an
`ON` clause, you need to alias the current table by using the [`AS` pipe
operator](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/pipe-syntax#as_pipe_operator). You can optionally alias the joined table. You can
reference both aliases following subsequent pipe operators:

    FROM mydataset.Produce
    |> AS produce_table
    |> JOIN mydataset.ItemData AS item_table
       ON produce_table.item = item_table.item
    |> WHERE produce_table.item = 'bananas'
    |> SELECT item_table.item, sales, id;

    /*---+---+---+
     | item    | sales | id  |
     +---+---+---+
     | bananas | 15    | 123 |
     +---+---+---*/

The right-hand side of the join doesn't have visibility to the left-hand side
of the join, which means you can't join the current table with itself. For
example, the following query fails:

    -- This query doesn't work.
    FROM mydataset.Produce
    |> AS produce_table
    |> JOIN produce_table AS produce_table_2 USING(item);

To perform a self-join with a modified table, you can use a common table
expression (CTE) inside of a `WITH` clause.

    WITH cte_table AS (
      FROM mydataset.Produce
      |> WHERE item = 'carrots'
    )
    FROM cte_table
    |> JOIN cte_table AS cte_table_2 USING(item);

## Example

Consider the following table with information about customer orders:

    CREATE OR REPLACE TABLE mydataset.CustomerOrders AS (
      SELECT 1 AS customer_id, 100 AS order_id, 'WA' AS state, 5 AS cost, 'clothing' AS item_type
      UNION ALL
      SELECT 1 AS customer_id, 101 AS order_id, 'WA' AS state, 20 AS cost, 'clothing' AS item_type
      UNION ALL
      SELECT 1 AS customer_id, 102 AS order_id, 'WA' AS state, 3 AS cost, 'food' AS item_type
      UNION ALL
      SELECT 2 AS customer_id, 103 AS order_id, 'NY' AS state, 16 AS cost, 'clothing' AS item_type
      UNION ALL
      SELECT 2 AS customer_id, 104 AS order_id, 'NY' AS state, 22 AS cost, 'housewares' AS item_type
      UNION ALL
      SELECT 2 AS customer_id, 104 AS order_id, 'WA' AS state, 45 AS cost, 'clothing' AS item_type
      UNION ALL
      SELECT 3 AS customer_id, 105 AS order_id, 'MI' AS state, 29 AS cost, 'clothing' AS item_type);

Suppose you want to know, for each state and item type, the average amount spent
by repeat customers. You could write the query in the following way:

    SELECT state, item_type, AVG(total_cost) AS average
    FROM
      (
        SELECT
          SUM(cost) AS total_cost,
          customer_id,
          state,
          item_type,
          COUNT(*) OVER (PARTITION BY customer_id) AS num_orders
        FROM mydataset.CustomerOrders
        GROUP BY customer_id, state, item_type
        QUALIFY num_orders > 1
      )
    GROUP BY state, item_type
    ORDER BY state DESC, item_type ASC;

If you read the query from top to bottom, you encounter the column `total_cost`
before it has been defined. Even within the subquery, you read the names of
columns before you see which table they come from.

To make sense of this query, it needs to be
read from the inside out. The columns `state` and `item_type` are repeated
numerous times in the `SELECT` and `GROUP BY` clauses, then again
in the `ORDER BY` clause.

The following equivalent query is written using
pipe syntax:

    FROM mydataset.CustomerOrders
    |> AGGREGATE SUM(cost) AS total_cost, GROUP BY customer_id, state, item_type
    |> EXTEND COUNT(*) OVER (PARTITION BY customer_id) AS num_orders
    |> WHERE num_orders > 1
    |> AGGREGATE AVG(total_cost) AS average GROUP BY state DESC, item_type ASC;

    /*---+---+---+
     | state | item_type  | average |
     +---+---+---+
     | WA    | clothing   | 35.0    |
     | WA    | food       | 3.0     |
     | NY    | clothing   | 16.0    |
     | NY    | housewares | 22.0    |
     +---+---+---*/

With pipe syntax, you can write the query to follow the logical steps you might
think through to solve the original problem. The lines of syntax in the query
correspond to the following logical steps:

- Start with the table of customer orders.
- Find out how much each customer spent on each type of item by state.
- Count the number of orders for each customer.
- Restrict the results to repeat customers.
- Find the average amount that repeat customers spend for each state and item type.

## Limitations

- You can't include a [differential privacy clause](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#dp_clause) in a `SELECT` statement following a pipe operator. Instead, use a differential privacy clause in standard syntax and apply pipe operators following the query.

## What's next

- [Analyze data using pipe syntax](https://docs.cloud.google.com/bigquery/docs/analyze-data-pipe-syntax)
- [Pipe query syntax reference](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/pipe-syntax)
- [Standard query syntax reference](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax)
- [VLDB 2024](https://research.google/pubs/sql-has-problems-we-can-fix-them-pipe-syntax-in-sql/) conference paper on pipe syntax