# Introduction to BI Engine

BigQuery BI Engine is a fast, in-memory analysis service that accelerates many
SQL queries in BigQuery by intelligently caching the data you use most
frequently. BI Engine can accelerate SQL queries from any source,
including those written by data visualization tools, and can manage cached
tables for ongoing optimization. This lets you improve query performance
without manual tuning or data tiering. You can [cluster](https://docs.cloud.google.com/bigquery/docs/clustered-tables)
and [partition](https://docs.cloud.google.com/bigquery/docs/partitioned-tables) tables to further optimize
BI Engine performance for large tables.

For example, if your dashboard only displays the last quarter's data, then
you could partition your tables by time so only the latest partitions are
loaded into memory. You can also combine the benefits of [materialized
views](https://docs.cloud.google.com/bigquery/docs/materialized-views-intro) and BI Engine.
This works particularly well when the materialized views are used to join and
flatten data to optimize their structure for BI Engine.

BI Engine provides the following advantages:

- **BigQuery API compatibility:** BI Engine directly integrates with the BigQuery API. Any BI solution or custom application that works with the BigQuery API through standard mechanisms such as [REST](https://docs.cloud.google.com/bigquery/docs/reference/rest) or [JDBC and ODBC drivers](https://docs.cloud.google.com/bigquery/docs/reference/odbc-jdbc-drivers) can use BI Engine without modification.
- **Vectorized runtime:** Using vectorized processing in an execution engine makes more efficient use of modern CPU architecture, by operating on batches of data at a time. BI Engine also uses advanced data encodings, specifically dictionary run-length encoding, to further compress the data that's stored in the in-memory layer.
- **Seamless integration:** BI Engine works with BigQuery features and metadata, including authorized views, column-level security, and data masking.
- **Reservation allocations:** BI Engine reservations separately manage memory allocation for each project and region. BI Engine only caches the queried, required parts of columns and partitions. You can specify which tables use BI Engine acceleration with [preferred tables](https://docs.cloud.google.com/bigquery/docs/bi-engine-preferred-tables).

In most organizations, BI Engine is enabled by a billing
administrator who must reserve capacity for BI Engine
acceleration with an appropriate [edition](https://docs.cloud.google.com/bigquery/docs/editions-intro#administration_features).
To learn more, see
[Reserve BI Engine capacity](https://docs.cloud.google.com/bigquery/docs/bi-engine-reserve-capacity).

## BI Engine use cases

BI Engine can significantly accelerate many SQL queries, including
those used for BI dashboards. Acceleration is most effective if you identify the
tables that are essential to your queries, and then mark them as [preferred
tables](https://docs.cloud.google.com/bigquery/docs/bi-engine-preferred-tables). To use
BI Engine, you create a reservation in a region and specify its
size. You can let BigQuery determine which tables to cache based
on the project's usage patterns or you can specify tables to prevent other
traffic from interfering with their acceleration.

BI Engine is useful in the following use cases:

- **You use BI tools to analyze your data**: BI Engine accelerate BigQuery queries whether they run in the BigQuery console, a BI tool such as Data Studio or Tableau, or a client library, API, or an ODBC or JDBC connector. This can significantly improve the performance of dashboards connected to BigQuery through a built-in connection (API) or connectors.
- **You have frequently queried tables**: BI Engine lets you designate preferred tables to accelerate. This is helpful if you have a subset of tables that are queried more frequently or are used for high-visibility dashboards.

BI Engine might not fit your needs in the following cases:

- **You use wildcards in your queries**: Queries referencing wildcard tables are not supported by BI Engine and don't benefit from acceleration.
- **You require BigQuery features unsupported by BI Engine** : While BI Engine supports most SQL functions and operators, [BI Engine unsupported features](https://docs.cloud.google.com/bigquery/docs/bi-engine-optimized-sql#unsupported-features) include external tables, row-level security, and non-SQL user-defined functions.

## Considerations for BI Engine

Consider the following when deciding how to configure BI Engine:

### Ensure acceleration for specific queries

To ensure a set of queries are accelerated, create a separate project with a
dedicated BI Engine reservation. First, [estimate the compute
capacity required for your queries](https://docs.cloud.google.com/bigquery/docs/bi-engine-reserve-capacity#estimate_and_measure_capacity),
then designate those tables as [preferred tables](https://docs.cloud.google.com/bigquery/docs/bi-engine-preferred-tables)
for BI Engine.

### Minimize your joins

BI Engine works best for pre-joined or pre-aggregated data, and
for queries with a small number of joins. This is particularly true when one
side of the join is large and the others are much smaller, such as when you
query a large fact table joined with smaller dimension tables. You can combine
BI Engine with [materialized views](https://docs.cloud.google.com/bigquery/docs/materialized-views-intro),
which perform joins to produce a single large, flat table. In this way, the same
joins aren't performed for each query. Stale materialized views are recommended
for optimal query performance.

### Understand the impact of BI Engine

To understand your use of BI Engine, see
[Monitor BI Engine with Cloud Monitoring](https://docs.cloud.google.com/bigquery/docs/bi-engine-monitor),
or query the
[`INFORMATION_SCHEMA.BI_CAPACITIES`](https://docs.cloud.google.com/bigquery/docs/information-schema-bi-capacities)
and
[`INFORMATION_SCHEMA.BI_CAPACITY_CHANGES`](https://docs.cloud.google.com/bigquery/docs/information-schema-bi-capacity-changes)
views. Be sure to disable the **Use cached results** option in
BigQuery to get the most accurate comparison. For more
information, see [Use cached query results](https://docs.cloud.google.com/bigquery/docs/cached-results).

### Preferred tables

BI Engine preferred tables let you limit BI Engine
acceleration to a specified set of tables. Queries to all other tables use
regular BigQuery slots. For example, with preferred tables you
can accelerate only the tables and dashboards that you identify as important to
your business.

If there is not enough RAM in the project to hold all of the preferred tables,
BI Engine offloads partitions and columns that haven't been
accessed recently. This process frees memory for new queries that need
acceleration.

#### Preferred tables limitations

BI Engine preferred tables have the following limitations:

- You cannot add views to the preferred tables reservation list. BI Engine preferred tables only support tables.
- Queries to materialized views are only accelerated if both the materialized views and their base tables are in the preferred tables list.
- Specifying partitions or columns for acceleration is not supported.
- `JSON` type columns are unsupported and are not accelerated by BI Engine.
- Queries that access multiple tables are only accelerated if all tables are preferred tables. For example, all tables in a query with a `JOIN` must be in the preferred tables list to be accelerated. If even one table is not in the preferred list, then the query cannot use BI Engine.
- Public datasets are not supported in the Google Cloud console. To add a public table as a preferred table, use the API or the DDL.

## Limitations

To use BI Engine, your organization must purchase
BI Engine capacity by creating a BI Engine
reservation with a supported edition. For more information, see
[Understand BigQuery editions](https://docs.cloud.google.com/bigquery/docs/editions-intro#administration_features).

In addition, BigQuery BI Engine has the following limitations.

### Joins

BI Engine accelerates certain types of join queries. Acceleration
happens on leaf-level subqueries with `INNER` and `LEFT OUTER JOINS`,
where a large fact table is joined with up to four smaller, "dimension" tables.
Small dimension tables have the following restrictions:

- Less than 5 million rows
- Size limit:
  - Unpartitioned tables: 5 GiB or less
  - Partitioned tables: Referenced partitions 1 GB or less

### Window functions

[Window functions](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/window-function-calls),
also known as analytical functions, have the following limitations when
accelerated by BigQuery BI Engine:

- The input stages are accelerated by BigQuery BI Engine if they don't have window functions. In this case `INFORMATION_SCHEMA.JOBS` view reports `bi_engine_statistics`.`acceleration_mode` as `FULL_INPUT`.
- The input stages of queries with window functions in their input stages are accelerated by BI Engine, but don't have the limitations described in [BI Engine Window functions limitations](https://docs.cloud.google.com/bigquery/docs/bi-engine-intro#window_function_limitations). In that case, the input stages or the full query is executed in BI Engine. In this case `INFORMATION_SCHEMA.JOBS` view reports `bi_engine_statistics`.`acceleration_mode` as `FULL_INPUT` or `FULL_QUERY`.

For more information about the `BiEngineStatistics` field, see the
[Job reference](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/Job#bienginestatistics).

#### BI Engine window functions limitations

Queries with window functions only run in BI Engine if all of the
following conditions are true:

- The query scans exactly one table.
  - The table is not partitioned.
  - The table has less than 5 million rows.
- The query has no `JOIN` operators.
- The scanned table size times the number of window function operators does not exceed 300 MiB.

Two window functions with identical `OVER` clauses and the
same direct inputs can share the same window function operator. For example:

- `SELECT ROW_NUMBER() OVER (ORDER BY x), SUM(x) OVER (ORDER BY x)
  FROM my_table` has only one window function operator.
- `SELECT ROW_NUMBER() OVER (ORDER BY x), SUM(x) OVER (PARTITION BY
  y ORDER BY x) FROM my_table` has two window function operators because the two functions have different `OVER` clauses.
- `SELECT ROW_NUMBER() OVER (ORDER BY x) FROM (SELECT SUM(x) OVER
  (ORDER BY x) AS x FROM my_table)` has two window function operators because the two functions have different direct inputs although their `OVER` clauses appear the same.

#### Supported window functions

The following referenced window functions are supported:

- `ANY_VALUE`
- `AVG`
- `BIT_AND`
- `BIT_OR`
- `BIT_XOR`
- `CORR`
- `COUNT`
- `COUNTIF`
- `COVAR_POP`
- `COVAR_SAMP`
- `CUME_DIST`
- `DENSE_RANK`
- `FIRST_VALUE`
- `LAG`
- `LAST_VALUE`
- `LEAD`
- `LOGICAL_AND`
- `LOGICAL_OR`
- `MAX`
- `MIN`
- `NTH_VALUE`
- `NTILE`
- `PERCENT_RANK`
- `PERCENTILE_CONT`
- `PERCENTILE_DISC`
- `RANK`
- `ROW_NUMBER`
- `ST_CLUSTERDBSCAN`
- `STDDEV_POP`
- `STDDEV_SAMP`
- `STDDEV`
- `STRING_AGG`
- `SUM`
- `VAR_POP`
- `VAR_SAMP`
- `VARIANCE`

If window functions aren't supported, then you might see the following error:

**Analytic function is incompatible with other operators or its inputs are too
large**

### Other BI Engine limitations

BI Engine acceleration is not available for the following features:

- JavaScript UDFs
- External tables, including BigLake tables
- Querying JSON data - Error message: **JSON native type is not supported**
- Writing results to a permanent BigQuery table
- Tables containing upserts that use [BigQuery change data capture ingestion](https://docs.cloud.google.com/bigquery/docs/change-data-capture)
- [Transactions](https://docs.cloud.google.com/bigquery/docs/transactions)
- Queries that return more than 1 GiB of data. For latency-sensitive applications, a response size of less than 1 MiB is recommended.
- Row-level security
- Queries that use the [`SEARCH` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/search_functions#search) or are optimized by [search indexes](https://docs.cloud.google.com/bigquery/docs/search-intro)

### Work-around for unsupported features

While some SQL features are not supported in BigQuery BI Engine,
there is an available workaround:

1. Write a query in BigQuery.
2. Save the results of the query to a table.
3. [Schedule your query](https://docs.cloud.google.com/bigquery/docs/scheduling-queries) to update the table on a regular basis. An hourly or daily refresh rate works best. Refreshing every minute might invalidate the cache too frequently.
4. Reference this table in your performance-critical queries.

## Quotas and limits

See [BigQuery quotas and limits](https://docs.cloud.google.com/bigquery/quotas#biengine-limits)
for quotas and limits that apply to BI Engine.

## Pricing

You incur costs for the reservation that you create for BI Engine
capacity. For information on BI Engine pricing, see the
[BigQuery Pricing](https://cloud.google.com/bigquery/pricing#bi_engine_pricing) page.

## What's next

- To learn how to create your BI Engine reservation, see [Reserve
  BI Engine capacity](https://docs.cloud.google.com/bigquery/docs/bi-engine-reserve-capacity).
- For information designating preferred tables, see [BI Engine
  preferred tables](https://docs.cloud.google.com/bigquery/docs/bi-engine-preferred-tables).
- To understand your utilization of BI Engine, see [Monitor
  BI Engine with Cloud Monitoring](https://docs.cloud.google.com/bigquery/docs/bi-engine-monitor).
- Learn about [BI Engine optimized
  functions](https://docs.cloud.google.com/bigquery/docs/bi-engine-optimized-sql)
- Learn how to use BI Engine with the following:
  - [Data Studio](https://docs.cloud.google.com/bigquery/docs/visualize-looker-studio)
  - [Tableau](https://docs.cloud.google.com/bigquery/docs/analyze-data-tableau)