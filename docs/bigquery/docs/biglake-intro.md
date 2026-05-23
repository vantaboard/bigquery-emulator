# Introduction to BigLake external tables

This document provides an overview of BigLake and assumes familiarity
with database tables and Identity and Access Management (IAM). To query data stored in the
[supported data stores](https://docs.cloud.google.com/bigquery/docs/biglake-intro#supported-data-stores), you must first create
BigLake tables and then query them using GoogleSQL
syntax:

- [Create Cloud Storage BigLake tables](https://docs.cloud.google.com/bigquery/docs/create-cloud-storage-table-biglake) and then [query](https://docs.cloud.google.com/bigquery/docs/query-cloud-storage-using-biglake).
- [Create Amazon S3 BigLake tables](https://docs.cloud.google.com/bigquery/docs/omni-aws-create-external-table) and then [query](https://docs.cloud.google.com/bigquery/docs/omni-aws-introduction).
- [Create Azure Blob Storage BigLake tables](https://docs.cloud.google.com/bigquery/docs/omni-azure-create-external-table) and then [query](https://docs.cloud.google.com/bigquery/docs/omni-aws-introduction).

You can also upgrade an external table to BigLake. For more
information, see [Upgrade an external table to BigLake](https://docs.cloud.google.com/bigquery/docs/external-data-cloud-storage#upgrade-external-tables-to-biglake-tables).

BigLake tables let you query structured data in
external data stores with access delegation. Access delegation
decouples access to the BigLake table from access to
the underlying data store. An
[external connection](https://docs.cloud.google.com/bigquery/docs/connections-api-intro)
associated with a service account is used to connect to the data store. Because
the service account handles retrieving data from the data store, you only have
to grant users access to the BigLake table. This lets you enforce
fine-grained security at the table level, including
[row-level](https://docs.cloud.google.com/bigquery/docs/row-level-security-intro) and
[column-level](https://docs.cloud.google.com/bigquery/docs/column-level-security-intro) security. For
BigLake tables based on Cloud Storage, you can also use
[dynamic data masking](https://docs.cloud.google.com/bigquery/docs/column-data-masking). To learn more about
multi-cloud analytic solutions using BigLake tables with
Amazon S3 or Blob Storage data, see
[BigQuery Omni](https://docs.cloud.google.com/bigquery/docs/omni-introduction).

## Supported data stores

You can use BigLake tables with the following data stores:

- [Amazon S3](https://docs.cloud.google.com/bigquery/docs/omni-aws-introduction) by using [BigQuery Omni](https://docs.cloud.google.com/bigquery/docs/omni-introduction)
- [Blob Storage](https://docs.cloud.google.com/bigquery/docs/omni-azure-introduction) by using [BigQuery Omni](https://docs.cloud.google.com/bigquery/docs/omni-introduction)
- [Cloud Storage](https://docs.cloud.google.com/bigquery/docs/query-cloud-storage-using-biglake)

## Temporary table support

BigLake tables based on Cloud Storage can be temporary
or permanent. BigLake tables based on Amazon S3 or
Blob Storage must be permanent.

## Multiple source files

You can create a BigLake table based on multiple external data
sources, provided those data sources have the same schema.

## Cross-cloud joins

Cross-cloud joins let you run queries that span both Google Cloud and
BigQuery Omni regions. You can use
[GoogleSQL `JOIN` operations](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#join_types)
to analyze data across many different storage solutions, such as AWS, Azure,
public datasets, and other Google Cloud services. Cross-cloud joins
eliminate the need to copy data across sources before running queries.

You can reference BigLake tables anywhere in a `SELECT` statement
as if they were standard BigQuery tables, including in
[data manipulation language (DML)](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/dml-syntax)
and [data definition language (DDL)](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language)
statements that
use subqueries to retrieve data. You can use multiple BigLake
tables from different clouds and BigQuery tables in the same
query. All BigQuery tables must be from the same region.

### Cross-cloud join required permissions


To get the permissions that
you need to run a cross-cloud join,

ask your administrator to grant you the
following IAM roles on the project where the join is executed:

- [BigQuery Data Viewer](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquery#bigquery.dataViewer) (`roles/bigquery.dataViewer`)
- [BigQuery Job User](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquery#bigquery.jobUser) (`roles/bigquery.jobUser`)


For more information about granting roles, see [Manage access to projects, folders, and organizations](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).


These predefined roles contain

the permissions required to run a cross-cloud join. To see the exact permissions that are
required, expand the **Required permissions** section:


#### Required permissions

The following permissions are required to run a cross-cloud join:

- `bigquery.jobs.create`
- `bigquery.tables.getData`


You might also be able to get
these permissions
with [custom roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles) or
other [predefined roles](https://docs.cloud.google.com/iam/docs/roles-overview#predefined).

### Cross-cloud join costs

When you run a cross-cloud join operation, BigQuery parses the
query into local and remote parts. The local part is treated as a standard query
in the BigQuery region. The remote part is converted into a
`CREATE TABLE AS SELECT` (CTAS) operation on the referenced
BigLake table in the BigQuery Omni region, which
creates a temporary table in your BigQuery region.
BigQuery then uses this temporary table to execute your
cross-cloud join and deletes the table automatically after eight hours.

You incur data transfer costs for data in the referenced BigLake
tables. However, BigQuery helps reduce these costs by only
transferring columns and rows in the BigLake table that are
referenced in the query, rather than the entire table. We recommend specifying a
column filter that is as narrow as possible to further reduce transfer costs.
The CTAS job appears in your job history and displays information such as the
number of transferred bytes. Successful transfers incur costs even if the main
query job fails. For more information, see
[BigQuery Omni pricing](https://cloud.google.com/bigquery/pricing#bqomni).

Consider the following query as an example:

```googlesql
SELECT *
FROM bigquery_dataset.bigquery_table AS clients
WHERE clients.sales_rep IN (
  SELECT id
  FROM aws_dataset.aws_table1 AS employees
  INNER JOIN aws_dataset.aws_table2 AS active_employees
    ON employees.id = active_employees.id
  WHERE employees.level > 3
);
```

This example has two transfers: one from an employees table (with a level
filter) and one from an active employees table. The join is performed in the
BigQuery region after the transfer occurs. If one transfer fails
and the other succeeds, data transfer charges are still applied for the
successful transfer.

### Cross-cloud join limitations

- Cross-cloud joins aren't supported in the BigQuery [free tier](https://cloud.google.com/bigquery/pricing#free-tier) and in the [BigQuery sandbox](https://docs.cloud.google.com/bigquery/docs/sandbox).
- Aggregations might not be pushed down to the BigQuery Omni regions if the query contains `JOIN` statements.
- Each temporary table is only used for a single cross-cloud query and is not reused even if the same query is repeated multiple times.
- The transfer size limit for each transfer is 60 GB. Specifically, if you apply a filter on a BigLake table and load the result, it must be smaller than 60 GB. If needed, you can [request a quota adjustment](https://docs.cloud.google.com/docs/quotas/help/request_increase). There is no limit on scanned bytes.
- Cross-cloud join queries employ an internal quota on the rate of queries. If the rate of queries exceeds the quota, you might receive an `All our servers are busy processing data transferred between regions` error. Retrying the query should work in most cases. Contact support to increase the internal quota to support a higher rate of queries.
- Cross-cloud joins are only supported in [colocated BigQuery regions](https://docs.cloud.google.com/bigquery/docs/omni-introduction#locations) with their corresponding BigQuery Omni regions and in the `US` and `EU` multi-regions. Cross-cloud joins that are run in the `US` or `EU` multi-regions can only access data in US or EU BigQuery Omni regions respectively.
- If a cross-cloud join query references 10 or more datasets from BigQuery Omni regions, it might fail with an error `Not found: Dataset <BigQuery dataset> was not found in
  location <BigQuery Omni region>`. To avoid this issue, we recommend [explicitly specifying a location](https://docs.cloud.google.com/bigquery/docs/locations#specify_locations) when you run a cross-cloud join that references more than 10 datasets. Be aware that if you explicitly specify a BigQuery region and your query only contains BigLake tables, then your query is run as a cross-cloud query and incurs data transfer costs.
- You can't [query the `_FILE_NAME` pseudo-column](https://docs.cloud.google.com/bigquery/docs/query-aws-data#query_the_file_name_pseudo-column) with cross-cloud joins.
- When you reference the columns of a BigLake table in a `WHERE` clause, you can't use `INTERVAL` or `RANGE` literals.
- Cross-cloud join jobs don't report the number of bytes that are processed and transferred from other clouds. This information is available in the child CTAS jobs that are created as part of cross-cloud query execution.
- [Authorized views](https://docs.cloud.google.com/bigquery/docs/authorized-views) and [authorized routines](https://docs.cloud.google.com/bigquery/docs/authorized-routines) referencing BigQuery Omni tables or views are only supported in BigQuery Omni regions.
- If your cross-cloud query references `STRUCT` or `JSON` columns, no pushdowns are applied to any remote subqueries. To optimize performance, consider creating a view in the BigQuery Omni region that filters `STRUCT` and `JSON` columns and returns only the necessary fields as individual columns.
- [Collation](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/collation-concepts) isn't supported by cross-cloud joins.
- Cross-cloud joins don't support joining Omni views using the `ORDER BY` clause.

### Cross-cloud join examples

The following query joins an `orders` table in a BigQuery region
with a `lineitem` table in a BigQuery Omni region:

```googlesql
SELECT
  l_shipmode,
  o_orderpriority,
  count(l_linenumber) AS num_lineitems
FROM bigquery_dataset.orders
JOIN aws_dataset.lineitem
  ON orders.o_orderkey = lineitem.l_orderkey
WHERE
  l_shipmode IN ('AIR', 'REG AIR')
  AND l_commitdate < l_receiptdate
  AND l_shipdate < l_commitdate
  AND l_receiptdate >= DATE '1997-01-01'
  AND l_receiptdate < DATE '1997-02-01'
GROUP BY l_shipmode, o_orderpriority
ORDER BY l_shipmode, o_orderpriority;
```

This query is broken into local and remote parts. The following query is sent to
the BigQuery Omni region to execute first. The result is a
temporary table in the BigQuery region. You can view this child
CTAS job and its metadata in your job history.

```googlesql
CREATE OR REPLACE TABLE temp_table
AS (
  SELECT
    l_shipmode,
    l_linenumber,
    l_orderkey
  FROM aws_dataset.lineitem
  WHERE
    l_shipmode IN ('AIR', 'REG AIR')
    AND l_commitdate < l_receiptdate
    AND l_shipdate < l_commitdate
    AND l_receiptdate >= DATE '1997-01-01'
    AND l_receiptdate < DATE '1997-02-01'
);
```

After the temporary table is created, the `JOIN` operation completes, and the
following query is run:

```googlesql
SELECT
  l_shipmode,
  o_orderpriority,
  count(l_linenumber) AS num_lineitems
FROM bigquery_dataset.orders
JOIN temp_table
  ON orders.o_orderkey = lineitem.l_orderkey
GROUP BY l_shipmode, o_orderpriority
ORDER BY l_shipmode, o_orderpriority;
```

As another example, consider the following cross-cloud join:

```googlesql
SELECT c_mktsegment, c_name
FROM bigquery_dataset.customer
WHERE c_mktsegment = 'BUILDING'
UNION ALL
SELECT c_mktsegment, c_name
FROM aws_dataset.customer
WHERE c_mktsegment = 'FURNITURE'
LIMIT 10;
```

In this query, the `LIMIT` clause is not pushed down to the
BigQuery Omni region. All customers in the `FURNITURE` market
segment are transferred to the BigQuery region first, and then
the limit of 10 is applied.

## Connectors

You can access data in BigLake tables based on
Cloud Storage from other data processing tools by using
BigQuery connectors.
For example, you could access data in BigLake tables from
[Apache Spark](https://github.com/GoogleCloudDataproc/spark-bigquery-connector),
[Apache Hive](https://github.com/GoogleCloudDataproc/hive-bigquery-connector),
[TensorFlow](https://www.tensorflow.org/),
[Trino](https://trino.io/docs/current/connector/bigquery.html), or
[Presto](https://prestodb.io/docs/current/connector/bigquery.html).
The BigQuery Storage API enforces row- and column-level governance policies on all
data access to BigLake tables, including through connectors.

For example, the following diagram demonstrates how
the [BigQuery Storage API](https://docs.cloud.google.com/bigquery/docs/reference/storage)
lets users access authorized data using open source query engines such as Apache
Spark:

![BigLake architecture.](https://docs.cloud.google.com/static/bigquery/images/biglake_arch.png)

For more information about connectors supported by BigQuery, see
[BigQuery connectors](https://docs.cloud.google.com/dataproc/docs/concepts/connectors/bigquery).

## BigLake tables on object stores

For data lake administrators, BigLake lets you set access
controls on tables rather than files, which gives you finer-grained options
when setting user access to data in the data lake.

Because BigLake tables simplifies access control in this way,
we recommend using BigLake tables to build and maintain
connections to external object stores.

You can use [external tables](https://docs.cloud.google.com/bigquery/docs/external-tables) in cases where
governance is not a requirement, or for ad hoc data discovery and manipulation.

## Limitations

- All [limitations for external tables](https://docs.cloud.google.com/bigquery/docs/external-tables#limitations) apply to BigLake tables.
- BigLake tables on object stores are subject to the same limitations as BigQuery tables. For more information, see [Quotas](https://docs.cloud.google.com/bigquery/quotas#external_tables).
- BigLake does not support downscoped credentials from
  [Managed Service for Apache Spark Personal Cluster Authentication](https://docs.cloud.google.com/dataproc/docs/concepts/iam/personal-auth).
  As a workaround, to use clusters with Personal Cluster Authentication, you
  must inject your credentials using an empty
  [Credential Access Boundary](https://cloud.google.com/dataproc/docs/concepts/iam/personal-auth#create_a_cluster_and_enable_an_interactive_session)
  with the `--access-boundary=<(echo -n "{}")` flag. For example, the following
  command enables a credential propagation session in a project named
  `myproject` for the cluster named `mycluster`:

  ```
  gcloud dataproc clusters enable-personal-auth-session \
      --region=us \
      --project=myproject \
      --access-boundary=<(echo -n "{}") \
      mycluster
  ```

  > [!CAUTION]
  > **Caution:** Using an empty credential access boundary removes
  > one layer of protection against attacks through stolen credentials from
  > Managed Service for Apache Spark clusters. Stolen credentials have a larger blast radius
  > without downscoping.
  >
  > As an alternative, you can disable Personal Cluster Authentication
  > and use the [Managed Service for Apache Spark virtual machine (VM) service account](https://docs.cloud.google.com/dataproc/docs/concepts/configuring-clusters/service-accounts) as a proxy for user groups.

  <br />

- BigLake tables are read-only. You cannot modify
  BigLake tables using DML statements or other methods.

- BigLake tables support the following formats:

  - Avro
  - CSV
  - [Delta Lake](https://docs.cloud.google.com/bigquery/docs/create-delta-lake-table)
  - Iceberg
  - JSON
  - ORC
  - Parquet
- You can't use
  [cached metadata](https://docs.cloud.google.com/bigquery/docs/biglake-intro#metadata_caching_for_performance)
  with
  [Apache Iceberg external tables](https://docs.cloud.google.com/bigquery/docs/iceberg-external-tables);
  BigQuery already uses the
  metadata that Iceberg captures in manifest files.

- The [BigQuery Storage API](https://docs.cloud.google.com/bigquery/docs/reference/storage) is not available
  in other cloud environments, such as AWS and Azure.

- If you use [cached metadata](https://docs.cloud.google.com/bigquery/docs/biglake-intro#metadata_caching_for_performance),
  then the following limitations apply:

  - You can only use [cached metadata](https://docs.cloud.google.com/bigquery/docs/biglake-intro#metadata_caching_for_performance) with BigLake tables that use Avro, ORC, Parquet, JSON, and CSV formats.
  - If you create, update, or delete files in Amazon S3, then querying the files does not return the updated data until the next refresh of the metadata cache. This can lead to unexpected results. For example, if you delete a file and write a new file, your query results may exclude both the old and the new files depending on when cached metadata was last updated.
  - Using customer-managed encryption keys (CMEK) with cached metadata is not supported for BigLake tables that reference Amazon S3 or Blob Storage data.

## Security model

The following organizational roles are typically involved in managing and
using BigLake tables:

- **Data lake administrators.** These administrators typically manage Identity and Access Management (IAM) policies on Cloud Storage buckets and objects.
- **Data warehouse administrators.** These administrators typically create, delete, and update tables.
- **Data analysts.** Analysts typically read data and run queries.

Data lake administrators are responsible for creating connections and sharing
them with data warehouse administrators. In turn, data warehouse
administrators create tables, set appropriate access controls, and share the
tables with data analysts.

> [!CAUTION]
> **Caution:** Data analysts should **not** have the following:
>
> - The ability to read objects directly from Cloud Storage (see the [Storage
>   Object Viewer IAM role](https://docs.cloud.google.com/storage/docs/access-control/iam-roles)), which lets data analysts circumvent access controls placed by data warehouse administrators.
> - The ability to bind tables to connections (like the BigQuery Connection
>   Administrator).
>
>   Otherwise, data analysts can create new tables that do
>   not have any access controls, thus circumventing controls placed by data
>   warehouse administrators.

## Metadata caching for performance

You can use cached metadata to improve query performance on some types of
BigLake tables. Metadata caching is especially helpful in cases
where you are working with large numbers of files or if the data is hive
partitioned. The following types of BigLake tables
support metadata caching:

- Amazon S3 BigLake tables
- Cloud Storage BigLake tables

BigQuery uses CMETA as a distributed metadata system to handle large tables efficiently. CMETA provides fine-grained metadata at the column and block level, accessible through system tables. This system helps improve query performance by optimizing data access and processing. To further accelerate query performance on large tables, BigQuery maintains a metadata cache. CMETA refresh jobs keep this cache up-to-date.

<br />

The metadata includes file names, partitioning information, and physical
metadata from files such as row counts. You can choose whether or not to enable
metadata caching on a table. Queries with a large number of files and with
Apache Hive partition filters benefit the most from metadata caching.

If you don't enable metadata caching, queries on the table must read the
external data source to get object metadata. Reading this data increases the
query latency; listing millions of files from the external data source can take
several minutes. If you enable metadata caching, queries can avoid listing files
from the external data source and can partition and prune files more quickly.

Metadata caching also integrates with Cloud Storage object versioning. When the cache is populated or refreshed, it captures metadata based on the live version of the Cloud Storage objects at that time. As a result, metadata caching-enabled queries read data corresponding to the specific cached object version, even if newer versions become live in Cloud Storage. Accessing data from any subsequently updated object versions in Cloud Storage necessitates a metadata cache refresh.

There are two properties that control this feature:

- **Maximum staleness** specifies when queries use cached metadata.
- **Metadata cache mode** specifies how the metadata is collected.

When you have metadata caching enabled, you specify the maximum interval of
metadata staleness that is acceptable for operations against the table. For
example, if you specify an interval of 1 hour, then operations against the table
use cached metadata if it has been refreshed within the past hour. If the cached
metadata is older than that, the operation falls back to retrieving metadata
from

the datastore (Amazon S3 or Cloud Storage) instead.

You can specify a staleness interval between 30 minutes and 7 days.

When you enable metadata caching for BigLake or object tables, BigQuery triggers metadata generation refresh jobs. You can choose to refresh the cache either automatically or manually:

- For automatic refreshes, the cache is refreshed at a system defined interval, usually somewhere between 30 and 60 minutes. Refreshing the cache automatically is a good approach if the files in the datastore are added, deleted, or modified at random intervals. If you need to control the timing of the refresh, for example to trigger the refresh at the end of an extract-transform-load job, use manual refresh.
- For manual refreshes, you run the
  [`BQ.REFRESH_EXTERNAL_METADATA_CACHE` system
  procedure](https://docs.cloud.google.com/bigquery/docs/reference/system-procedures#bqrefresh_external_metadata_cache)
  to refresh the metadata cache on a schedule that meets your requirements.
  For BigLake tables, you can refresh the metadata selectively
  by providing subdirectories of the table data directory. This lets you
  avoid unnecessary metadata
  processing.
  Refreshing the cache manually is a good approach if the files in

  the datastore

  are added, deleted, or modified at known intervals,
  for example as the output of a pipeline.

  If you issue multiple concurrent manual refreshes, only one will succeed.

The metadata cache expires after 7 days if it isn't refreshed.

Both manual and automatic cache refreshes are executed with
[`INTERACTIVE`](https://docs.cloud.google.com/bigquery/docs/running-queries) query priority.

### Use `BACKGROUND` reservations

If you choose to use automatic refreshes, we recommend that you create a
[reservation](https://docs.cloud.google.com/bigquery/docs/reservations-intro), and then create an
[assignment with a `BACKGROUND` job type](https://docs.cloud.google.com/bigquery/docs/reservations-workload-management#assignments)
for the project that runs the metadata cache refresh jobs. With `BACKGROUND` reservations, refresh jobs use a dedicated resource pool which prevents the refresh jobs from competing with user queries, and prevents the jobs from potentially failing if there aren't sufficient resources available for them.

While using a shared slot pool incurs no extra cost, using `BACKGROUND` reservations instead provides more consistent performance by allocating a dedicated resource pool, and improves the reliability of refresh jobs and overall query efficiency in BigQuery.

You should consider how the staleness interval and metadata caching mode
values will interact before you set them. Consider the following examples:

- If you are manually refreshing the metadata cache for a table, and you set the staleness interval to 2 days, you must run the `BQ.REFRESH_EXTERNAL_METADATA_CACHE` system procedure every 2 days or less if you want operations against the table to use cached metadata.
- If you are automatically refreshing the metadata cache for a table, and you set the staleness interval to 30 minutes, it is possible that some of your operations against the table might read from the datastore if the metadata cache refresh takes on the longer side of the usual 30 to 60 minute window.

To find information about metadata refresh jobs, query the
[`INFORMATION_SCHEMA.JOBS` view](https://docs.cloud.google.com/bigquery/docs/information-schema-jobs),
as shown in the following example:

```googlesql
SELECT *
FROM `region-us.INFORMATION_SCHEMA.JOBS_BY_PROJECT`
WHERE job_id LIKE '%metadata_cache_refresh%'
AND creation_time > TIMESTAMP_SUB(CURRENT_TIMESTAMP(), INTERVAL 6 HOUR)
ORDER BY start_time DESC
LIMIT 10;
```

For Cloud Storage BigLake tables that are based on Parquet
files, [table statistics](https://docs.cloud.google.com/bigquery/docs/metadata-caching#table_statistics) are
collected during the metadata cache refresh and used to improve query plans.

To learn more, see [Metadata caching](https://docs.cloud.google.com/bigquery/docs/metadata-caching).

For more information on setting metadata caching options, see
[Create Amazon S3 BigLake tables](https://docs.cloud.google.com/bigquery/docs/omni-aws-create-external-table)
or
[Create Cloud Storage BigLake tables](https://docs.cloud.google.com/bigquery/docs/create-cloud-storage-table-biglake).

### Cache-enabled tables with materialized views

You can use [materialized views over BigLake metadata cache-enabled
tables](https://docs.cloud.google.com/bigquery/docs/materialized-views-intro#biglake) to improve
performance and efficiency when querying structured data stored in
Cloud Storage or Amazon Simple Storage Service (Amazon S3).
These materialized views function like materialized views over
BigQuery-managed storage tables, including the benefits of
[automatic refresh](https://docs.cloud.google.com/bigquery/docs/materialized-views-manage#automatic-refresh)
and [smart tuning](https://docs.cloud.google.com/bigquery/docs/materialized-views-use#smart_tuning).

## Integrations

BigLake tables are accessible from a number of other BigQuery features
and gcloud CLI services, including the following, highlighted services.

### BigQuery sharing (formerly Analytics Hub)

BigLake tables are compatible with Sharing.
Datasets containing BigLake tables can be published as
[Sharing listings](https://docs.cloud.google.com/bigquery/docs/analytics-hub-introduction#listings).
Sharing subscribers can subscribe to these
listings, which provision a read-only dataset, called a [*linked
dataset*](https://docs.cloud.google.com/bigquery/docs/analytics-hub-introduction#linked_datasets), in
their project.
Subscribers can query all tables in the linked dataset, including all
BigLake tables. For more information, see [View and subscribe to
listings](https://docs.cloud.google.com/bigquery/docs/analytics-hub-view-subscribe-listings).

### BigQuery ML

You can use [BigQuery ML](https://docs.cloud.google.com/bigquery/docs/bqml-introduction) to train and
run models on BigLake in Cloud Storage.

### Sensitive Data Protection

[Sensitive Data Protection](https://docs.cloud.google.com/sensitive-data-protection/docs) scans your BigLake tables
to identify and classify sensitive data. If sensitive data is detected,
Sensitive Data Protection de-identification transformations can
[mask, delete, or otherwise obscure](https://docs.cloud.google.com/bigquery/docs/scan-with-dlp)
that data.

## Costs

Costs are associated with the following aspects of BigLake tables:

- Querying the tables.
- [Refreshing the metadata cache](https://docs.cloud.google.com/bigquery/docs/biglake-intro#metadata_caching_for_performance).

If you have [slot
reservations](https://docs.cloud.google.com/bigquery/docs/reservations-workload-management), you are not
charged for querying external tables. Instead, slots are consumed for these
queries.

The following table shows how your pricing model affects how these costs are
applied:

|   | **On-demand pricing** | **Standard, Enterprise, and Enterprise Plus editions** |
|---|---|---|
| Queries | You are [billed for the bytes processed](https://cloud.google.com/bigquery/pricing#on_demand_pricing) by user queries. | [Slots](https://cloud.google.com/bigquery/pricing#capacity_compute_analysis_pricing) in [reservation assignments with a `QUERY` job type](https://docs.cloud.google.com/bigquery/docs/reservations-workload-management#assignments) are consumed during query time. |
| Manually refreshing the metadata cache. | You are [billed for the bytes processed](https://cloud.google.com/bigquery/pricing#on_demand_pricing) to refresh the cache. | [Slots](https://cloud.google.com/bigquery/pricing#capacity_compute_analysis_pricing) in [reservation assignments with a `QUERY` job type](https://docs.cloud.google.com/bigquery/docs/reservations-workload-management#assignments) are consumed during cache refresh. |
| Automatically refreshing the metadata cache. | You are [billed for the bytes processed](https://cloud.google.com/bigquery/pricing#on_demand_pricing) to refresh the cache. | [Slots](https://cloud.google.com/bigquery/pricing#capacity_compute_analysis_pricing) in [reservation assignments with a `BACKGROUND` job type](https://docs.cloud.google.com/bigquery/docs/reservations-workload-management#assignments) are consumed during cache refresh. <br /> If there are no `BACKGROUND` reservations available for refreshing the metadata cache, BigQuery automatically uses slots in `QUERY` reservations instead if you are using the Enterprise or Enterprise Plus edition. |

You are also charged for storage and data access by
[Cloud Storage](https://cloud.google.com/storage/pricing),
[Amazon S3](https://aws.amazon.com/s3/pricing/),
and [Azure Blob Storage](https://azure.microsoft.com/pricing/details/storage/blobs/),
subject to each product's pricing guidelines.

When BigQuery interacts with Cloud Storage, you might
incur the following Cloud Storage costs:

- Data storage costs for the amount of data stored.
- Data retrieval costs for accessing data in [Nearline](https://docs.cloud.google.com/storage/docs/storage-classes#nearline), [Coldline](https://docs.cloud.google.com/storage/docs/storage-classes#coldline), and [Archive](https://docs.cloud.google.com/storage/docs/storage-classes#archive) storage classes. Take caution when querying tables or refreshing the metadata cache against these storage classes, as charges can be significant.
- Network usage costs for data that you read across different regions, such as when your BigQuery dataset and Cloud Storage bucket are in different regions.
- Data processing charges. However, you aren't charged for API calls that are made by BigQuery on your behalf, such as listing or getting resources.

## What's next

- Learn how to [upgrade external tables to BigLake tables](https://docs.cloud.google.com/bigquery/docs/external-data-cloud-storage#upgrade-external-tables-to-biglake-tables).
- Learn how to [create a Cloud Storage BigLake table](https://docs.cloud.google.com/bigquery/docs/create-cloud-storage-table-biglake).
- Learn how to [create an Amazon S3 BigLake table](https://docs.cloud.google.com/bigquery/docs/omni-aws-create-external-table).
- Learn how to [create a Blob Storage BigLake table](https://docs.cloud.google.com/bigquery/docs/omni-azure-create-external-table).
- Learn how to [create data quality checks with Knowledge Catalog](https://docs.cloud.google.com/bigquery/docs/dataplex-shared-introduction).