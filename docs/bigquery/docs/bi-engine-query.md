# What is BI Engine?

BigQuery BI Engine is a fast, in-memory analysis service that accelerates many
SQL queries in BigQuery by intelligently caching the data you use most
frequently. BI Engine can accelerate SQL queries from any source,
including those written by data visualization tools, and can manage cached
tables for ongoing optimization. This lets you improve query performance
without manual tuning or data tiering. You can use [clustering](https://docs.cloud.google.com/bigquery/docs/clustered-tables)
and [partitioning](https://docs.cloud.google.com/bigquery/docs/partitioned-tables) to further optimize the
performance of large tables with BI Engine.

For example, if your dashboard only displays the last quarter's data, then
consider partitioning your tables by time so only the latest partitions are
loaded into memory. You can also combine the benefits of [materialized
views](https://docs.cloud.google.com/bigquery/docs/materialized-views-intro) and BI Engine.
This works particularly well when the materialized views are used to join and
flatten data to optimize their structure for BI Engine.

BI Engine provides the following advantages:

1. **BigQuery API:** BI Engine directly integrates with the BigQuery API. Any BI solution or custom application that works with the BigQuery API through standard mechanisms such as [REST](https://docs.cloud.google.com/bigquery/docs/reference/rest) or [JDBC and ODBC drivers](https://docs.cloud.google.com/bigquery/docs/reference/odbc-jdbc-drivers) can use BI Engine without modification.
2. **Vectorized runtime:** With the BI Engine, BigQuery uses a modern technique called *vectorized processing*. Using vectorized processing in an execution engine makes more efficient use of modern CPU architecture, by operating on batches of data at a time. BI Engine also uses advanced data encodings, specifically, dictionary and run-length encoding, to further compress the data that's stored in the in-memory layer.
3. **Seamless integration:** BI Engine works with BigQuery features and metadata, including authorized views, column and row level security, and data masking.
4. **Reservations:** BI Engine reservations manage memory allocation at the project location level. BI Engine caches specific columns or partitions that are queried, prioritizing those in tables marked as preferred.

### BI Engine architecture

BI Engine integrate with any business intelligence (BI) tool,
including such as Looker, Tableau, Power BI, and custom applications to
accelerate data exploration and analysis.

![BI Engine architecture](https://docs.cloud.google.com/static/bigquery/images/bi-engine-architecture.png)

## BI Engine use cases

BI Engine can significantly accelerate many SQL queries, including
those used for BI dashboards. Acceleration is most effective if you identify
the tables that are essential to your queries and then mark them as [preferred
tables](https://docs.cloud.google.com/bigquery/docs/bi-engine-preferred-tables). To use
BI Engine, create a reservation that defines the storage
capacity dedicated to BI Engine. You can let
BigQuery determine which tables to cache based on the project's
usage patterns or you can mark specific tables to prevent other traffic
from interfering with acceleration.

BI Engine is useful in the following use cases:

- **You use BI tools to analyze your data**: The BI Engine can accelerate BigQuery queries regardless of whether they run in the BigQuery console, client library, or through an API or an ODBC or JDBC connector. This can significantly improve the performance of dashboards connected to BigQuery through a built-in connection (API) or connectors.
- **You have certain tables that are queried most frequently**: BI Engine lets you designate specific preferred tables to accelerate. This is helpful if you have a subset of tables that are queried more frequently or are used for high-visibility dashboards.

BI Engine might not fit your needs in the following cases:

- **You use wildcards in your queries**: Queries referencing wildcard tables are
  not supported by BI Engine and don't benefit from acceleration.

- **You rely heavily on unsupported BigQuery features** : While
  BI Engine supports most [SQL functions and
  operators](https://docs.cloud.google.com/bigquery/docs/bi-engine-optimized-sql) when connecting business
  intelligence (BI) tools to BigQuery, there are [unsupported
  features](https://docs.cloud.google.com/bigquery/docs/bi-engine-optimized-sql#unsupported-features),
  including external tables and non-SQL user-defined functions.

## Considerations for BI Engine

Consider the following when deciding how to configure BI Engine:

### Ensure acceleration for specific queries

You can ensure a particular set of queries always gets accelerated by creating a
separate project with a BI Engine reservation. To do so, you
should ensure that the BI Engine reservation in that project is
large enough to match the size of all tables used in those queries and
designate those tables as [preferred
tables](https://docs.cloud.google.com/bigquery/docs/bi-engine-preferred-tables) for BI Engine.
Only those queries that need to be accelerated should be run in that project.

### Minimize your joins

BI Engine works best with pre-joined or pre-aggregated, and with
data in a small number of joins. This is
particularly true when one side of the join is large and the others are much
smaller such as when you query a large fact table joined with a small dimension
table. You can combine BI Engine with [materialized
views](https://docs.cloud.google.com/bigquery/docs/materialized-views-intro) that which perform joins
to produce a single large, flat table. In this way, the same joins don't have
to be performed on every query.

### Understand the impact of BI Engine

You can better understand how your workloads benefit from
BI Engine by [reviewing the usage statistics in
Cloud Monitoring](https://docs.cloud.google.com/bigquery/docs/bi-engine-intro#limitations)
or querying the [INFORMATION_SCHEMA](https://docs.cloud.google.com/bigquery/docs/bi-engine-monitor#information_schema)
in BigQuery. Be sure to disable the **Use cached results**
option in BigQuery to get the most accurate comparison. For more
information, see [Use cached query results](https://docs.cloud.google.com/bigquery/docs/cached-results).

## Limitations

Queries that contain the
[`VECTOR_SEARCH` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/search_functions#vector_search)
aren't accelerated by [BigQuery BI Engine](https://docs.cloud.google.com/bigquery/docs/bi-engine-intro).

## Quotas and limits

See [BigQuery quotas and limits](https://docs.cloud.google.com/bigquery/quotas#biengine-limits)
for quotas and limits that apply to BI Engine.

## Pricing

For information on BI Engine pricing, see the
[BigQuery Pricing](https://cloud.google.com/bigquery/pricing#bi_engine_pricing) page.

## Query optimization and acceleration

> [!NOTE]
> **Note:** For BI Engine maximum reservation size, see [quotas and limits](https://docs.cloud.google.com/bigquery/quotas#biengine-limits).

BigQuery, and by extension BI Engine, breaks down
the query plan that's produced for a SQL query into subqueries. A subquery
contains a number of operations, such as scanning, filtering, or aggregating
data, and is often the unit of execution on a shard.

While all of BigQuery's supported SQL queries are correctly
executed by the BI Engine, only certain subqueries are optimized. In
particular, BI Engine is most optimized for leaf-level subqueries
that scan the data from storage, and perform operations such as filter, compute,
aggregation, order-by, and certain types of joins. Other subqueries that are not
yet fully accelerated by BI Engine revert back to
BigQuery for execution.

Because of this selective optimization, simpler business intelligence or
dashboard-type queries benefit the most from BI Engine (resulting
in fewer subqueries) because the majority of the execution time is spent on
leaf-level subqueries that process raw data.

## What's next

- Learn about [BI Engine optimized functions](https://docs.cloud.google.com/bigquery/docs/bi-engine-optimized-sql).
- To learn how to create your BI Engine reservation, see [Reserve
  BI Engine capacity](https://docs.cloud.google.com/bigquery/docs/bi-engine-reserve-capacity).
- For information designating preferred tables, see [BI Engine
  preferred tables](https://docs.cloud.google.com/bigquery/docs/bi-engine-preferred-tables).
- To understand your utilization of BI Engine, see [Monitor BI Engine with Cloud Monitoring](https://docs.cloud.google.com/bigquery/docs/bi-engine-monitor).