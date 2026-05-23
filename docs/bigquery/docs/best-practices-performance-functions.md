# Best practices for functions

This document describes how to optimize queries that use SQL functions.

## Optimize string comparison

**Best practice:** When possible, use `LIKE` instead of `REGEXP_CONTAINS`.

In BigQuery, you can use the
[`REGEXP_CONTAINS`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#regexp_contains)
function or the [`LIKE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/operators#comparison_operators)
operator to compare strings. `REGEXP_CONTAINS` provides more functionality,
but also has a slower execution time. Using `LIKE` instead of `REGEXP_CONTAINS`
is faster, particularly if you don't need the full power of regular expressions
that `REGEXP_CONTAINS` provides, for example wildcard matching.

Consider the following use of the `REGEXP_CONTAINS` function:

```googlesql
SELECT
  dim1
FROM
  `dataset.table1`
WHERE
  REGEXP_CONTAINS(dim1, '.*test.*');
```

You can optimize this query as follows:

```googlesql
SELECT
  dim1
FROM
  `dataset.table`
WHERE
  dim1 LIKE '%test%';
```

## Optimize aggregation functions

**Best practice:** If your use case supports it, use an approximate aggregation
function.

If the SQL aggregation function you're using has an equivalent approximation
function, the approximation function yields faster query performance. For
example, instead of using
[`COUNT(DISTINCT)`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#count),
use
[`APPROX_COUNT_DISTINCT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/approximate_aggregate_functions#approx_count_distinct).
For more information, see
[approximate aggregation functions](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/approximate_aggregate_functions).

You can also use `HyperLogLog++` functions to do approximations (including custom
approximate aggregations). For more information, see
[HyperLogLog++ functions](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/hll_functions)
in the GoogleSQL reference.

Consider the following use of the `COUNT` function:

```googlesql
SELECT
  dim1,
  COUNT(DISTINCT dim2)
FROM
  `dataset.table`
GROUP BY 1;
```

You can optimize this query as follows:

```googlesql
SELECT
  dim1,
  APPROX_COUNT_DISTINCT(dim2)
FROM
  `dataset.table`
GROUP BY 1;
```

### Optimize quantile functions

**Best practice:** When possible, use `APPROX_QUANTILE` instead of `NTILE`.

Running a query that contains the
[`NTILE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/numbering_functions#ntile)
function can fail with a
[`Resources exceeded`](https://docs.cloud.google.com/bigquery/troubleshooting-errors#resourcesExceeded)
error if there are too many
elements to `ORDER BY` in a single partition, which causes data volume to grow.
The analytic window isn't partitioned, so the `NTILE` computation requires
a global `ORDER BY` for all rows in the table to be processed
by a single worker/slot.

Try using
[`APPROX_QUANTILES`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/approximate_aggregate_functions#approx_quantiles)
instead. This function allows the query to run more efficiently because it
doesn't require a global `ORDER BY` for all rows in the table.

Consider the following use of the `NTILE` function:

```googlesql
SELECT
  individual_id,
  NTILE(nbuckets) OVER (ORDER BY sales desc) AS sales_third
FROM
  `dataset.table`;
```

You can optimize this query as follows:

```googlesql
WITH QuantInfo AS (
  SELECT
    o, qval
  FROM UNNEST((
     SELECT APPROX_QUANTILES(sales, nbuckets)
     FROM `dataset.table`
    )) AS qval
  WITH offset o
  WHERE o > 0
)
SELECT
  individual_id,
  (SELECT
     (nbuckets + 1) - MIN(o)
   FROM QuantInfo
   WHERE sales <= QuantInfo.qval
  ) AS sales_third
FROM `dataset.table`;
```

The optimized version gives similar but not identical results to the original
query, because `APPROX_QUANTILES`:

1. Provides an approximate aggregation.
2. Places the remainder values (the remainder of the number of rows divided by buckets) in a different way.

## Optimize UDFs

**Best practice:** Use SQL UDFs for simple calculations because the query optimizer can apply optimizations to SQL UDF definitions. Use JavaScript UDFs for complex calculations that are not supported by SQL UDFs.

Calling a JavaScript UDF requires the instantiation of a subprocess.
Spinning up this process and running the UDF directly impacts query performance.
If possible, use a
[native (SQL) UDF](https://docs.cloud.google.com/bigquery/docs/user-defined-functions#sql-udf-structure)
instead.

### Persistent UDFs

It is better to create persistent user-defined SQL and JavaScript functions
in a centralized BigQuery dataset that can be invoked across
queries and in logical views, as opposed to creating and calling a UDF in code
each time. Creating org-wide libraries of business logic within shared datasets
helps optimize performance and use fewer resources.

The following example shows how a temporary UDF is invoked in a query:

```googlesql
CREATE TEMP FUNCTION addFourAndDivide(x INT64, y INT64) AS ((x + 4) / y);

WITH numbers AS
  (SELECT 1 as val
  UNION ALL
  SELECT 3 as val
  UNION ALL
  SELECT 4 as val
  UNION ALL
  SELECT 5 as val)
SELECT val, addFourAndDivide(val, 2) AS result
FROM numbers;
```

You can optimize this query by replacing the temporary UDF with a
persistent one:

```googlesql
WITH numbers AS
  (SELECT 1 as val
  UNION ALL
  SELECT 3 as val
  UNION ALL
  SELECT 4 as val
  UNION ALL
  SELECT 5 as val)
SELECT val, `your_project.your_dataset.addFourAndDivide`(val, 2) AS result
FROM numbers;
```

## Optimize AI costs

**Best practice:** If your use case supports it, use the optimized mode when
running `AI.IF` or `AI.CLASSIFY` on large datasets.

If you are processing more than a few thousand rows using managed AI functions,
you can enable the optimized mode to automatically train a lightweight,
distilled model. This significantly reduces LLM token consumption and query
latency. For more information, see [Optimize AI functions for large-scale
data](https://docs.cloud.google.com/bigquery/docs/optimize-ai-functions).