# Use materialized views

This document provides additional information about materialized views and how
to use them. Before you read this document, familiarize yourself with
[Introduction to materialized views](https://docs.cloud.google.com/bigquery/docs/materialized-views-intro)
and [Create materialized views](https://docs.cloud.google.com/bigquery/docs/materialized-views-create).

## Query materialized views

You can query your materialized views directly, the same way you query a regular
table or standard view. Queries against materialized views are always consistent
with queries against the view's base tables, even if those tables have changed
since the last time the materialized view was refreshed. Querying does not
automatically trigger a materialized refresh.

> [!NOTE]
> **Note:** If you delete a base table without first deleting the materialized view, queries over the materialized view fail. You must recreate the materialized view after you recreate the base table to query it again.

### Required roles


To get the permissions that
you need to query a materialized view,

ask your administrator to grant you the
[BigQuery Data Viewer](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquery#bigquery.dataViewer) (`roles/bigquery.dataViewer`) IAM role on the base table of the materialized view and the materialized view itself.


For more information about granting roles, see [Manage access to projects, folders, and organizations](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).


This predefined role contains

the permissions required to query a materialized view. To see the exact permissions that are
required, expand the **Required permissions** section:


#### Required permissions

The following permissions are required to query a materialized view:

- `bigquery.tables.get`
- `bigquery.tables.getData`


You might also be able to get
these permissions
with [custom roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles) or
other [predefined roles](https://docs.cloud.google.com/iam/docs/roles-overview#predefined).

These permissions are required for queries in order to benefit from
[smart tuning](https://docs.cloud.google.com/bigquery/docs/materialized-views-use#smart_tuning).

For more information about
IAM roles in BigQuery, see
[Introduction to IAM](https://docs.cloud.google.com/bigquery/docs/access-control).

### Incremental updates

Incremental updates occur when BigQuery combines the cached
view's data with new data to provide consistent query results while still using
the materialized view. For single-table materialized views, this is possible if
the base table is unchanged since the last refresh, or if only new data was
added. For `JOIN` views, only tables on the left side of the `JOIN` can have
appended data. If one of the tables on the right side of a `JOIN` has changed,
then the view cannot be incrementally updated.

If the base table had updates or deletions since the last refresh, or if the
materialized view's base tables on the right side of the `JOIN` have changed,
then BigQuery doesn't use incremental updates and instead
automatically reverts to the original query. For more information about joins
and materialized views, see
[Joins](https://docs.cloud.google.com/bigquery/docs/materialized-views-create#joins). The following are
examples of Google Cloud console, bq command-line tool, and API actions that can cause an
update or deletion:

- Data manipulation language (DML) `UPDATE`, `MERGE`, or `DELETE` statements
- Truncation
- Partition expiration

The following metadata operations also prevent a materialized view from being incrementally updated:

- Changing partition expiration
- Updating or dropping a column

If a materialized view can't be incrementally updated, then its cached data is
not used by queries until the view is automatically or manually refreshed. For
details about why a job didn't use materialized view data, see [Understand why
materialized views were
rejected](https://docs.cloud.google.com/bigquery/docs/materialized-views-use#understand-rejected).
Additionally, materialized views cannot be incrementally updated if its base
table has accumulated unprocessed changes for a time period greater than the
table's [time travel
interval](https://docs.cloud.google.com/bigquery/docs/time-travel#configure_the_time_travel_window).

## Partition alignment

If a materialized view is partitioned, BigQuery ensures that its
partitions are aligned with the partitions of the base table's partitioning
column. *Aligned* means that the data from a particular partition of the
base table contributes to the same partition of the materialized view. For
example, a row from partition `20220101` of the base table would contribute only
to partition `20220101` of the materialized view.

When a materialized view is partitioned, the behavior described in
[Incremental updates](https://docs.cloud.google.com/bigquery/docs/materialized-views-use#incremental_updates) occurs for each individual
partition independently. For example, if data is deleted in one partition of
the base table, then BigQuery can still use the materialized
view's other partitions without requiring a full refresh of the entire
materialized view.

Materialized views with inner joins can only be aligned with one of their base
tables. If one of the non-aligned base tables changes, it affects the entire
view.

## Smart tuning

BigQuery automatically rewrites queries to use materialized
views whenever possible. Automatic rewriting improves query performance and
reduces costs without changing query results. Querying does not automatically
trigger a materialized refresh. For a query to be rewritten using smart-tuning,
the materialized view must meet the following conditions:

- Belong to the same project as one of its base tables or the project that the query is running in.
- Use the same set of base tables as the query.
- Include all columns being read.
- Include all rows being read.

Smart tuning isn't supported for the following:

- Materialized views that [reference logical views](https://docs.cloud.google.com/bigquery/docs/materialized-views-create#reference_logical_views).
- Materialized views with [union all or left outer join](https://docs.cloud.google.com/bigquery/docs/materialized-views-create#left-union).
- [Non-incremental materialized views](https://docs.cloud.google.com/bigquery/docs/materialized-views-create#limitations_specific_to_non-incremental_materialized_views).
- Materialized views that reference [change data capture-enabled](https://docs.cloud.google.com/bigquery/docs/change-data-capture) tables.

### Smart tuning examples

Consider the following materialized view query example:

```googlesql
SELECT
  store_id,
  CAST(sold_datetime AS DATE) AS sold_date
  SUM(net_profit) AS sum_profit
FROM dataset.store_sales
WHERE
  CAST(sold_datetime AS DATE) >= '2021-01-01' AND
  promo_id IS NOT NULL
GROUP BY 1, 2
```

The following examples show queries and why those queries are or aren't
automatically rewritten using this view:

| Query | Rewrite? | Reason |
|---|---|---|
| SELECT **SUM(net_paid) AS sum_paid** , SUM(net_profit) AS sum_profit FROM dataset.store_sales WHERE CAST(sold_datetime AS DATE) \>= '2021-01-01' AND promo_id IS NOT NULL | No | The view must include all columns being read. The view does not include 'SUM(net_paid)'. |
| SELECT SUM(net_profit) AS sum_profit FROM dataset.store_sales WHERE CAST(sold_datetime AS DATE) \>= '2021-01-01' AND promo_id IS NOT NULL | Yes |   |
| SELECT SUM(net_profit) AS sum_profit FROM dataset.store_sales WHERE CAST(sold_datetime AS DATE) \>= '2021-01-01' AND promo_id IS NOT NULL AND **customer_id = 12345** | No | The view must include all columns being read. The view does not include 'customer'. |
| SELECT SUM(net_profit) AS sum_profit FROM dataset.store_sales WHERE **sold_datetime** = '2021-01-01' AND promo_id IS NOT NULL | No | The view must include all columns being read. 'sold_datetime' is not an output (but 'CAST(sold_datetime AS DATE)' is). |
| SELECT SUM(net_profit) AS sum_profit FROM dataset.store_sales WHERE CAST(sold_datetime AS DATE) \>= '2021-01-01' AND promo_id IS NOT NULL AND store_id = 12345 | Yes |   |
| SELECT SUM(net_profit) AS sum_profit FROM dataset.store_sales WHERE CAST(sold_datetime AS DATE) \>= '2021-01-01' AND **promo_id = 12345** | No | The view must include all rows being read. 'promo_id' is not an output so the more restrictive filter can't be applied to the view. |
| SELECT SUM(net_profit) AS sum_profit FROM dataset.store_sales WHERE **CAST(sold_datetime AS DATE) \>= '2020-01-01'** | No | The view must include all rows being read. The view filter for dates in 2021 and after, but the query reads dates from 2020. |
| SELECT SUM(net_profit) AS sum_profit FROM dataset.store_sales WHERE CAST(sold_datetime AS DATE) \>= '2022-01-01' AND promo_id IS NOT NULL | Yes |   |

### Understand whether a query was rewritten

To understand if a query was rewritten by smart tuning to use a materialized
view, inspect the [query plan](https://docs.cloud.google.com/bigquery/docs/query-plan-explanation). If the
query was rewritten, then the query plan contains a `READ my_materialized_view`
step, where `my_materialized_view` is the name of the materialized view used. To
understand why a query didn't use a materialized view, see [Understand why
materialized views were rejected](https://docs.cloud.google.com/bigquery/docs/materialized-views-use#understand-rejected).

### Understand why materialized views were rejected

If you have disabled automatic refresh for your materialized view and the table
has unprocessed changes, then the query might be faster for several days but
then start revert to the original query resulting in slower processing speed.
To benefit from materialized views, enable automatic refresh or manually refresh
regularly, and monitor materialized view refresh jobs to confirm they succeed.

The steps to understand why a materialized view was rejected
depend on the type of query that you used:

- Direct query of the materialized view
- Indirect query in which [smart tuning](https://docs.cloud.google.com/bigquery/docs/materialized-views-use#smart_tuning) might choose to use the materialized view

The following sections provide steps to help you understand why a materialized
view was rejected.

#### Direct query of materialized views

Direct queries of materialized views might not use cached data in certain
circumstances. The following steps can help you understand why the materialized
view data was not used:

1. Follow the steps in [Monitor materialized view
   use](https://docs.cloud.google.com/bigquery/docs/materialized-views-monitor#monitor_materialized_view_usage) and find the target materialized view in the [`materialized_view_statistics` field](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/Job#MaterializedViewStatistics) for the query.
2. If `chosen` is present in the statistics and its value is `TRUE`, then the materialized view is used by the query.
3. Review the `rejected_reason` field to find next steps. In most cases, you can [manually refresh](https://docs.cloud.google.com/bigquery/docs/materialized-views-manage#manual-refresh) the materialized view or wait for the next [automatic
   refresh](https://docs.cloud.google.com/bigquery/docs/materialized-views-manage#automatic-refresh).

#### Query with smart tuning

1. Follow the steps in [Monitor materialized view
   use](https://docs.cloud.google.com/bigquery/docs/materialized-views-monitor#monitor_materialized_view_usage) and find the target materialized view in the [`materialized_view_statistics`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/Job#MaterializedViewStatistics) for the query.
2. Review the `rejected_reason` to find next steps. For example, if the `rejected_reason` value is `COST`, then smart tuning identified more efficient data sources for cost and performance.
3. If the materialized view is not present, try a direct query of the materialized view and follow the steps in [Direct query of materialized
   views](https://docs.cloud.google.com/bigquery/docs/materialized-views-use#direct_query_of_materialized_views).
4. If the direct query does not use the materialized view, then the shape of the materialized view does not match the query. For more information about smart tuning and how queries are rewritten using materialized views, see [Smart tuning examples](https://docs.cloud.google.com/bigquery/docs/materialized-views-use#smart_tuning_examples).

## Frequently asked questions

### When should I use scheduled queries versus materialized views?

[Scheduled queries](https://docs.cloud.google.com/bigquery/docs/scheduling-queries) are a convenient way to
run arbitrarily complex calculations periodically. Each time the query runs, it
runs fully, with no benefit from previous results, and you pay the full compute
cost for the query. Scheduled queries are ideal when you don't need the freshest
data and have a high tolerance for data staleness.

Materialized views are best suited for when you need to query the latest data
with minimized latency and cost by reusing the previously computed result.
You can use materialized views as *pseudo-indexes* , accelerating queries to the
base table without updating any existing workflows. The [`--max_staleness`
option](https://docs.cloud.google.com/bigquery/docs/materialized-views-create#max_staleness)
lets you define acceptable staleness for your materialized views, providing
consistently high performance with controlled costs when processing
large, frequently changing datasets.

As a general guideline, whenever possible and if you are not running arbitrarily
complex calculations, use materialized views.

### Some queries over materialized views are slower than same queries over manually materialized tables. Why is that?

In general, a query over a materialized view isn't always as performant as a
query over the equivalent materialized table. The reason is that materialized
views always return fresh results, and must account for
changes to their base tables since the last view refresh.

Consider this scenario:

```googlesql
CREATE MATERIALIZED VIEW my_dataset.my_mv AS
SELECT date, customer_id, region, SUM(net_paid) as total_paid
FROM my_dataset.sales
GROUP BY 1, 2, 3;

CREATE TABLE my_dataset.my_materialized_table AS
SELECT date, customer_id, region, SUM(net_paid) as total_paid
FROM my_dataset.sales
GROUP BY 1, 2, 3;
```

For example, this query:

```googlesql
  SELECT * FROM my_dataset.my_mv LIMIT 10
```
typically runs much more slowly than this query:

```googlesql
  SELECT * FROM my_dataset.my_materialized_table LIMIT 10
```
In order to provide consistently up-to-date results, BigQuery must query new rows in the base table and merge them into the materialized view before applying the 'LIMIT 10' predicate. As a result, slowness remains, even if the materialized view is fully up-to-date.

<br />

On the other hand, aggregations over materialized views
are typically as fast as queries against
the materialized table. For example, the following:

```googlesql
  SELECT SUM(total_paid) FROM my_dataset.my_mv WHERE date > '2020-12-01'
```
Should be as fast as this:

```googlesql
  SELECT SUM(total_paid) FROM my_dataset.my_materialized_table WHERE date > '2020-12-01'
```

<br />