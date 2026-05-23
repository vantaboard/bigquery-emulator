# Introduction to materialized views

Materialized views are precomputed views that periodically store the results of
a SQL query. In some use cases, materialized views reduce the total processing
time and related charges by reducing the amount of data to be scanned for each
query. You can query materialized views as you would other data resources.

The following use cases highlight the value of materialized views:

- **Pre-process data**. Improve query performance by preparing aggregates, filters, joins, and clusters.
- **Dashboard acceleration**. Empower BI tools like Looker that frequently query the same aggregate metrics---for example, daily active users.
- **Real-time analytics on large streams**. Can provide faster responses on tables that receive high-velocity streaming data.
- **Cost management**. Reduce the cost of repetitive, expensive queries over large datasets.

> [!NOTE]
> **Note:** Materialized views aren't available when you use reservations created with certain BigQuery editions. For more information about which features are enabled in each edition, see [Introduction to
> BigQuery editions](https://docs.cloud.google.com/bigquery/docs/editions-intro).

Key characteristics of materialized views include the following:

- **Zero maintenance**. Materialized views are precomputed in the background when the base tables change. Any incremental data changes from the base tables are automatically added to the materialized views, with no user action required.
- **Fresh data**. Materialized views return fresh data. If changes to base tables might invalidate the materialized view, then data is read directly from the base tables. If the changes to the base tables don't invalidate the materialized view, then rest of the data is read from the materialized view and only the changes are read from the base tables.
- **Smart tuning** . If any part of a query against a base table can be resolved by querying the materialized view, then BigQuery reroutes the query to use the materialized view for improved performance and efficiency. For information about how and when smart tuning can improve queries, see [Use materialized views](https://docs.cloud.google.com/bigquery/docs/materialized-views-use#smart_tuning).

### Incremental and non-incremental materialized views

There are two basic kinds of materialized views:

- *Incremental materialized views* support a limited set of features. To learn more about supported SQL syntax for materialized views, see [Create materialized views](https://docs.cloud.google.com/bigquery/docs/materialized-views-create). Only incremental materialized views can take advantage of [smart tuning](https://docs.cloud.google.com/bigquery/docs/materialized-views-use#smart_tuning).
- *Non-incremental functions* support most of the syntaxes that incremental materialized views don't support.

When you create materialized views, by default BigQuery only lets you create
views based upon *incremental* queries. To create a non-incremental view, you can
specify `allow_non_incremental_definition = true` in the materialized view's
definition.

The best type of materialized view to use depends on your situation. The following
table compares the features of incremental and non-incremental materialized
views:

| **Category** | **Incremental** | **Non-incremental** |
|---|---|---|
| Query supported | [Limited](https://docs.cloud.google.com/bigquery/docs/materialized-views-create#aggregate_requirements) | [Most queries](https://docs.cloud.google.com/bigquery/docs/materialized-views-create#create-non-inc) |
| Maintenance cost | Can reduce the cost of frequently used queries. To learn how materialized views are updated, see [incremental updates](https://docs.cloud.google.com/bigquery/docs/materialized-views-use#incremental_updates). | Every refresh runs the full query. |
| Smart tuning support | Supported for most views queries. | No |
| Always fresh results | Supported. Incremental views return fresh query results even when the base tables have changed since the last refresh. | No |

## Authorized materialized views

You can create an authorized materialized view to share a subset of data from a
source dataset to a view in a secondary dataset. You can then share this view to
specific users and groups (principals) who can view the data you share.
Principals can query the data you provide in a view, but they can't access
the source dataset directly.

Authorized views and authorized materialized views are authorized in the same
way. For details, see [Authorized views](https://docs.cloud.google.com/bigquery/docs/authorized-views).

## Interaction with other BigQuery features

The following BigQuery features work transparently with
materialized views:

- **[Query plan explanation](https://docs.cloud.google.com/bigquery/docs/query-plan-explanation):** The query plan
  reflects which materialized views are scanned (if any), and shows how many
  bytes are read from the materialized views and base tables combined.

- **[Query caching](https://docs.cloud.google.com/bigquery/docs/cached-results):** The results of a query
  that BigQuery rewrites using
  a materialized view can be cached subject to the usual limitations (using of
  deterministic functions, no streaming into the base tables, etc.).

- **[Cost restriction](https://docs.cloud.google.com/bigquery/docs/best-practices-costs#restrict-bytes-billed):**
  If you have set a value for maximum bytes billed,
  and a query would read a number of bytes beyond the limit, the query fails
  without incurring a charge, whether the query uses materialized views, the
  base tables, or both.

- **[Cost estimation using dry run](https://docs.cloud.google.com/bigquery/docs/best-practices-costs#perform-dry-run):**
  A dry run repeats query rewrite logic
  using the available materialized views and provides a cost estimate. You can use
  this feature as a way to test whether a specific query uses any
  materialized views.

- **[Cross-region data replication](https://docs.cloud.google.com/bigquery/docs/data-replication):**
  Materialized views can be created on top of BigQuery tables that
  have cross-region replication enabled, but only on the primary region. If the
  secondary region is used, you can encounter the following error message:
  `The dataset replica of the cross region dataset {PROJECT}:{DATASET} in region {REGION} is read-only because it's not the primary replica.`

### Tables with active change data capture

You can create materialized views over tables with active
[change data capture](https://docs.cloud.google.com/bigquery/docs/change-data-capture) (CDC). These materialized
views function like materialized views over BigQuery tables,
including the benefits of automatic refresh. Materialized views can't perform
[runtime merge queries](https://docs.cloud.google.com/bigquery/docs/change-data-capture#query-max-staleness),
so materialized views must be configured with a sufficient `max_staleness` to
avoid runtime merge jobs. For more information, see
[Limitations of materialized views over tables with active change data capture](https://docs.cloud.google.com/bigquery/docs/materialized-views-intro#cdc_limits).

### BigLake metadata cache-enabled tables

Materialized views over [BigLake metadata cache-enabled
tables](https://docs.cloud.google.com/bigquery/docs/biglake-intro#metadata_caching_for_performance) can
reference structured data stored in Cloud Storage and Amazon Simple Storage Service (Amazon S3).
These materialized views function
like materialized views over BigQuery-managed storage tables,
including the benefits of automatic refresh and smart tuning. Other benefits
include the pre-aggregating, pre-filtering, and pre-joining of data stored
outside of BigQuery. Materialized views over
BigLake tables are stored in and have all of the characteristics
of [BigQuery managed storage](https://docs.cloud.google.com/bigquery/docs/storage_overview).

> [!NOTE]
> **Note:** When a materialized view over a BigLake table with cached metadata is refreshed, the materialized view's cached data contains all updates to the external table up to the most recent metadata cache creation.

When you create a materialized view over an Amazon S3
BigLake table, the data in the materialized view isn't
available for joins with BigQuery data. To make Amazon S3
data in a materialized view available for joins, create a
[replica](https://docs.cloud.google.com/bigquery/docs/load-data-using-cross-cloud-transfer#materialized_view_replicas) of the materialized view.
You can only create materialized view replicas over
[authorized materialized views](https://docs.cloud.google.com/bigquery/docs/authorized-views).

## Limitations

- Limits on base table references and other restrictions might apply. For more information about materialized view limits, see [Quotas
  and limits](https://docs.cloud.google.com/bigquery/quotas#materialized_view_limits).
- The data of a materialized view cannot be updated or manipulated directly using operations such as `COPY`, `EXPORT`, `LOAD`, `WRITE`, or data manipulation language (DML) statements.
- The materialized view SQL cannot be updated after the materialized view is created.
- A materialized view must reside in the same organization as its base tables, or in the same project if the project does not belong to an organization.
- Materialized views use a restricted SQL syntax and a limited set of aggregation functions. For more information, see [Supported
  materialized views](https://docs.cloud.google.com/bigquery/docs/materialized-views#supported-mvs).
- Materialized views cannot be nested on other materialized views.
- Materialized views cannot query external or wildcard tables, logical views^1^, or snapshots.
- [System variables](https://docs.cloud.google.com/bigquery/docs/reference/system-variables), including the `@@session_id` system variable, aren't supported with materialized views.
- The value of the `max_staleness` option must be between 30 minutes and 3 days, inclusive.
- Only the GoogleSQL dialect is supported for materialized views.
- You can set descriptions for materialized views, but you cannot set descriptions for the individual columns in the materialized view.
- If you delete a base table without first deleting the materialized view, queries and refreshes of the materialized view fail. If you recreate the base table, you must also recreate the materialized view.
- Only non-incremental materialized view can have [Spanner
  external dataset base tables](https://docs.cloud.google.com/bigquery/docs/spanner-external-datasets). If a non-incremental materialized view's last refresh occurred outside the `max_staleness` interval, then the query reads the base Spanner external dataset tables. To learn more about Spanner external dataset tables, see [Create materialized views over Spanner external datasets](https://docs.cloud.google.com/bigquery/docs/materialized-views-create#spanner).
- Query results are not cached if the query runs against non-incremental materialized views that reference [Spanner external dataset tables](https://docs.cloud.google.com/bigquery/docs/spanner-external-datasets).
- Materialized views can't inherit or explicitly define [parameterized data types](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#parameterized_data_types), such as `STRING(n)`, as parameterized data types are only supported for base table columns and script variables.

^1^Logical view reference support is in
[preview](https://cloud.google.com/products/#product-launch-stages). For more information, see
[Reference logical views](https://docs.cloud.google.com/bigquery/docs/materialized-views-create#reference_logical_views).

### Limitations of materialized views over tables with active CDC

Materialized views with active change data capture base tables have the following
limitations:

- If a materialized view has a base table with active [change data capture](https://docs.cloud.google.com/bigquery/docs/change-data-capture), then that table can't be referenced in a query that also references the materialized view.
- When you create a materialized view over a table with active change data capture, the materialized view can't perform the runtime merge jobs of the underlying CDC table. Set the [`max_staleness` value](https://docs.cloud.google.com/bigquery/docs/materialized-views-create#max_staleness) of the materialized view to at least twice the `max_staleness` value of the base table. Queries against a materialized view fail if the current version of the underlying CDC table is staler than the materialized view `max_staleness`.
- Smart tuning isn't supported for materialized views over tables with active change data capture.

### Limitations of materialized views over BigLake tables

- Partitioning of the materialized view is not supported. The base tables can use hive partitioning but the materialized view storage cannot be partitioned in BigLake tables. This means that any deletion in a base table causes a full refresh of the materialized view. For more details see [Incremental updates](https://docs.cloud.google.com/bigquery/docs/materialized-views-use#incremental_updates).
- The [`--max_staleness`
  option](https://docs.cloud.google.com/bigquery/docs/materialized-views-create#max_staleness) value of the materialized view must be greater than that of the BigLake base table.
- Joins between BigQuery managed tables and BigLake tables are not supported in a single materialized view definition.
- BigQuery BI Engine doesn't support acceleration of materialized views over BigLake tables.

## Materialized views pricing

Costs are associated with the following aspects of materialized views:

- Querying materialized views.
- Maintaining materialized views, such as when materialized views are refreshed. The cost for automatic refresh is billed to the project where the view resides. The cost for manual refresh is billed to the project in which the manual refresh job is run. For more information about controlling maintenance cost, see [Refresh job maintenance](https://docs.cloud.google.com/bigquery/docs/materialized-views-manage#refresh).
- Storing materialized view tables.

| Component | On-demand pricing | Capacity-based pricing |
|---|---|---|
| Querying | Bytes processed by materialized views and any necessary portions of the base tables.^1^ | Slots are consumed during query time. |
| Maintenance | Bytes processed during refresh time. | Slots are consumed during refresh time. |
| Storage | Bytes stored in materialized views. | Bytes stored in materialized views. |

^1^Where possible, BigQuery reads only the changes since
the last time the view was refreshed. For more information, see
[Incremental updates](https://docs.cloud.google.com/bigquery/docs/materialized-views-use#incremental_updates).

### Storage cost details

For `AVG`, `ARRAY_AGG`, and `APPROX_COUNT_DISTINCT` aggregate values in a
materialized view, the final value is not directly stored. Instead,
BigQuery internally stores a materialized view as an intermediate
*sketch*, which is used to produce the final value.

As an example, consider a materialized view that's created with the following
command:

```googlesql
CREATE MATERIALIZED VIEW project-id.my_dataset.my_mv_table AS
SELECT date, AVG(net_paid) AS avg_paid
FROM project-id.my_dataset.my_base_table
GROUP BY date
```

While the `avg_paid` column is rendered as `NUMERIC` or `FLOAT64` to the user,
internally it is stored as `BYTES`, with its content being an intermediate
sketch in proprietary format. For [data size calculation](https://cloud.google.com/bigquery/pricing#data),
the column is treated as `BYTES`.

## What's next

- [Overview of logical and materialized views](https://docs.cloud.google.com/bigquery/docs/logical-materialized-view-overview)
- [Create materialized views](https://docs.cloud.google.com/bigquery/docs/materialized-views-create)
- [Use materialized views](https://docs.cloud.google.com/bigquery/docs/materialized-views-use)
- [Manage materialized views](https://docs.cloud.google.com/bigquery/docs/materialized-views-manage)