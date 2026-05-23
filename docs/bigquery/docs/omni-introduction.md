# Introduction to BigQuery Omni

> [!NOTE]
> **Note:** This feature may not be available when using reservations that are created with certain BigQuery editions. For more information about which features are enabled in each edition, see [Introduction to
> BigQuery editions](https://docs.cloud.google.com/bigquery/docs/editions-intro).

With BigQuery Omni, you can run BigQuery analytics on
data stored in Amazon Simple Storage Service (Amazon S3) or Azure Blob Storage using BigLake
tables.

Many organizations store data in multiple public clouds. Often, this data ends
up being siloed, because it's hard to get insights across all of the data. You
want to be able to analyze the data with a multi-cloud data tool that
is inexpensive, fast, and does not create additional overhead of decentralized
data governance. By using BigQuery Omni, we reduce these
frictions with a unified interface.

To run BigQuery analytics on your external data, you
first need to [connect to Amazon S3](https://docs.cloud.google.com/bigquery/docs/omni-aws-create-connection)
or [Blob Storage](https://docs.cloud.google.com/bigquery/docs/omni-azure-create-connection). If you
want to query external data, you would need to create a [BigLake
table](https://docs.cloud.google.com/bigquery/docs/biglake-intro) that references Amazon S3 or
Blob Storage data.

## BigQuery Omni tools

You can use the following BigQuery Omni tools to run BigQuery analytics
on your external data:

- [Cross-cloud joins](https://docs.cloud.google.com/bigquery/docs/biglake-intro#cross-cloud_joins): Run a query directly from a BigQuery region that can join data from a BigQuery Omni region.
- [Cross-cloud materialized
  views](https://docs.cloud.google.com/bigquery/docs/load-data-using-cross-cloud-transfer#materialized_view_replicas): Use [materialized
  view
  replicas](https://docs.cloud.google.com/bigquery/docs/load-data-using-cross-cloud-transfer#materialized_view_replicas) to continuously replicate data from BigQuery Omni regions. Supports data filtering.
- [Cross-cloud transfer using
  `SELECT`](https://docs.cloud.google.com/bigquery/docs/load-data-using-cross-cloud-transfer): Run a query using either the `CREATE TABLE AS SELECT` or `INSERT INTO SELECT` statement in a BigQuery Omni region and move the result to a BigQuery region.
- [Cross-cloud transfer using `LOAD`](https://docs.cloud.google.com/bigquery/docs/load-data-using-cross-cloud-transfer): Use [`LOAD DATA`statements](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/load-statements) to load data directly from Amazon Simple Storage Service (Amazon S3) or Azure Blob Storage into BigQuery

The following table outlines the key features and capabilities of each cross-cloud tool:

|   | Cross-cloud joins | Cross-cloud materialized view | Cross-cloud transfer using `SELECT` | Cross-cloud transfer using `LOAD` |
| Suggested usage | Query external data for one-time use, where you can join with local tables or join data between two different BigQuery Omni regions---for example, between AWS and Azure Blob Storage regions. Use cross-cloud joins if the data isn't large, and if caching is not a key requirement | Set up repeated or scheduled queries to continuously transfer external data incrementally, where caching is a key requirement. For example, to maintain a dashboard | Query external data for one-time use, from a BigQuery Omni region to a BigQuery region, where manual controls like caching and query optimization is a key requirement, and if you're using complex queries that aren't supported by cross-cloud joins or cross-cloud materialized views | Migrate large datasets as-is without the need for filtering, using scheduled queries to move raw data |
| Supports filtering before moving data | Yes. Limits apply on certain query operators. For more information, see [Cross-cloud join limitations](https://docs.cloud.google.com/bigquery/docs/biglake-intro#cross-cloud_join_limitations) | Yes. Limits apply on certain query operators, such as aggregate functions and the `UNION` operator | Yes. No limits on query operators | No |
| Transfer size limitations | [60 GB per transfer](https://docs.cloud.google.com/bigquery/docs/biglake-intro#cross-cloud_join_limitations) (each subquery to a remote region produces one transfer) | No limit | [60 GB per transfer](https://docs.cloud.google.com/bigquery/docs/load-data-using-cross-cloud-transfer#limitations_2) (each subquery to a remote region produces one transfer) | No limit |
| Data transfer compression | Wire compression | Columnar | Wire compression | Wire Compression |
| Caching | Not supported | Supported with [cache-enabled tables with materialized views](https://docs.cloud.google.com/bigquery/docs/omni-introduction#cache-enabled_tables_with_materialized_views) | Not supported | Not supported |
| Egress pricing | AWS egress and inter-continental cost | AWS egress and inter-continental cost | AWS egress and inter-continental cost | AWS egress and inter-continental cost |
| Compute usage for data transfer | Uses slots in the source AWS or Azure Blob Storage region (Reservation or On-demand) | Not used | Uses slots in the source AWS or Azure Blob Storage region (Reservation or On-demand) | Not used |
| Compute usage for filtering | Uses slots in the source AWS or Azure Blob Storage region (Reservation or On-demand) | Uses slots in the source AWS or Azure Blob Storage region (Reservation or On-demand) for computing local materialized views and metadata | Uses slots in the source AWS or Azure Blob Storage region (Reservation or On-demand) | Not used |
| Incremental transfer | Not supported | Supported for non-aggregate materialized views | Not supported | Not supported |
|---|---|---|---|---|

You can also consider the following alternatives to transfer data from Amazon Simple Storage Service (Amazon S3)
or Azure Blob Storage to Google Cloud:

- [Storage Transfer Service](https://docs.cloud.google.com/storage-transfer): Transfer data between object and file storage across Google Cloud and Amazon Simple Storage Service (Amazon S3) or Azure Blob Storage.
- [BigQuery Data Transfer Service](https://docs.cloud.google.com/bigquery/docs/dts-introduction): Set up automated data transfer into BigQuery on a scheduled, managed basis. Supports a [variety of sources](https://docs.cloud.google.com/bigquery/docs/dts-introduction#supported_data_sources) and is suitable for data migration. BigQuery Data Transfer Service doesn't support filtering.

## Architecture

BigQuery's architecture separates compute from storage, allowing
BigQuery to scale out as needed to handle very large workloads.
BigQuery Omni extends this architecture by running the
BigQuery query engine in other clouds. As a result, you don't
have to physically move data into BigQuery storage. Processing
happens where that data already sits.

![BigQuery Omni architecture](https://docs.cloud.google.com/static/bigquery/images/omni-architecture.png)

Query results can be returned to Google Cloud over a secure connection ---
for example, to be displayed in the Google Cloud console. Alternatively, you can
write the results directly to Amazon S3 buckets or Blob Storage.
In that case, there is no cross-cloud movement of the query results.

BigQuery Omni uses standard AWS IAM roles or Azure Active Directory
principals to access the data in your subscription. You delegate read or write
access to BigQuery Omni, and you can revoke access at any time.

> [!NOTE]
> **Note:** Write access is only required if you want to write query results back to your Amazon S3 bucket or Blob Storage container.

### Data flow when querying data

The following image describes how the data moves between Google Cloud and AWS or
Azure for the following queries:

- `SELECT` statement
- `CREATE EXTERNAL TABLE` statement

![Data movement between Google Cloud and AWS or Azure for queries.](https://docs.cloud.google.com/static/bigquery/images/omni-data-movement-query.svg) **Figure 1:** Data movement between Google Cloud and AWS or Azure for queries.

1. BigQuery control plane receive query jobs from you through Google Cloud console, bq command-line tool, an API method, or a client library.
2. BigQuery control plane sends query jobs for processing to BigQuery data plane on AWS or Azure.
3. BigQuery data plane receives the query from the control plane through a VPN connection.
4. BigQuery data plane reads table data from your Amazon S3 bucket or Blob Storage.
5. BigQuery data plane runs the query job on table data. The processing of table data occurs in the specified AWS or Azure region.
6. The query result is transmitted from data plane to the control plane through the VPN connection.
7. The BigQuery control plane receives the query job results for display to you in response to the query job. This data is stored for up to 24 hours.
8. The query result is returned to you.

For more information, see [Query Amazon S3 data](https://docs.cloud.google.com/bigquery/docs/omni-aws-create-external-table)
and [Blob Storage data](https://docs.cloud.google.com/bigquery/docs/omni-azure-create-external-table).

### Data flow when exporting data

The following image describes how data moves between Google Cloud and AWS
or Azure during an `EXPORT DATA` statement.
![Data movement between Google Cloud and AWS or Azure for export queries.](https://docs.cloud.google.com/static/bigquery/images/omni-data-movement-export.svg) **Figure 2:** Data movement between Google Cloud and AWS or Azure for export queries.

1. BigQuery control plane receives export query jobs from you through Google Cloud console, bq command-line tool, an API method, or a client library. The query contains the destination path for the query result in your Amazon S3 bucket or Blob Storage.
2. BigQuery control plane sends export query jobs for processing to BigQuery data plane (on AWS or Azure).
3. BigQuery data plane receives the export query from the control plane through the VPN connection.
4. BigQuery data plane reads table data from your Amazon S3 bucket or Blob Storage.
5. BigQuery data plane runs the query job on table data. Processing of table data occurs in the specified AWS or Azure region.
6. BigQuery writes the query result to the specified destination path in your Amazon S3 bucket or Blob Storage.

For more information, see [Export query results to Amazon S3](https://docs.cloud.google.com/bigquery/docs/omni-aws-export-results-to-s3)
and [Blob Storage](https://docs.cloud.google.com/bigquery/docs/omni-azure-export-results-to-azure-storage).

## Benefits

**Performance.** You can get insights faster, because data is not copied across
clouds, and queries run in the same region where your data resides.

**Cost.** You save on outbound data transfer costs because the data doesn't
move. There are no additional charges to your AWS or Azure account related to
BigQuery Omni analytics, because the queries run on clusters
managed by Google. You are only billed for running the queries, using the
BigQuery pricing model.

**Security and data governance.** You manage the data in your own AWS or Azure
subscription. You don't need to move or copy the raw data out of your public
cloud. All computation happens in the BigQuery multi-tenant
service which runs within the same region as your data.

**Serverless architecture.** Like the rest of BigQuery,
BigQuery Omni is a serverless offering. Google deploys and manages the
clusters that run BigQuery Omni. You don't need to provision any resources or
manage any clusters.

**Ease of management.** BigQuery Omni provides a unified
management interface through Google Cloud. BigQuery Omni can use
your existing Google Cloud account and BigQuery projects. You
can write a GoogleSQL query in the Google Cloud console to query data in
AWS or Azure, and see the results displayed in the Google Cloud console.

**Cross-cloud transfer.** You can load data into standard BigQuery
tables from S3 buckets and Blob Storage. For more information, see
[Transfer Amazon S3 data](https://docs.cloud.google.com/bigquery/docs/omni-aws-cross-cloud-transfer) and
[Blob Storage data to BigQuery](https://docs.cloud.google.com/bigquery/docs/omni-azure-cross-cloud-transfer).

## Metadata caching for performance

You can use cached metadata to improve query performance on
BigLake tables that reference Amazon S3 data. It is
especially helpful in cases where you are working with large numbers of files or
if the data is Apache Hive partitioned.
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

Amazon S3 instead.

You can specify a staleness interval between 30 minutes and 7 days.

When you enable metadata caching for BigLake or object tables, BigQuery triggers metadata generation refresh jobs. You can choose to refresh the cache either automatically or manually:

- For automatic refreshes, the cache is refreshed at a system defined interval, usually somewhere between 30 and 60 minutes. Refreshing the cache automatically is a good approach if the files in Amazon S3 are added, deleted, or modified at random intervals. If you need to control the timing of the refresh, for example to trigger the refresh at the end of an extract-transform-load job, use manual refresh.
- For manual refreshes, you run the
  [`BQ.REFRESH_EXTERNAL_METADATA_CACHE` system
  procedure](https://docs.cloud.google.com/bigquery/docs/reference/system-procedures#bqrefresh_external_metadata_cache)
  to refresh the metadata cache on a schedule that meets your requirements.

  Refreshing the cache manually is a good approach if the files in

  Amazon S3

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
- If you are automatically refreshing the metadata cache for a table, and you set the staleness interval to 30 minutes, it is possible that some of your operations against the table might read from Amazon S3 if the metadata cache refresh takes on the longer side of the usual 30 to 60 minute window.

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

For more information, see [Metadata caching](https://docs.cloud.google.com/bigquery/docs/metadata-caching).

### Cache-enabled tables with materialized views

You can use [materialized views over Amazon Simple Storage Service (Amazon S3) metadata cache-enabled
tables](https://docs.cloud.google.com/bigquery/docs/materialized-views-intro#biglake) to improve
performance and efficiency when querying structured data stored in
Amazon S3. These materialized views function like materialized views
over BigQuery-managed storage tables, including the benefits of
[automatic refresh](https://docs.cloud.google.com/bigquery/docs/materialized-views-manage#automatic-refresh)
and [smart tuning](https://docs.cloud.google.com/bigquery/docs/materialized-views-use#smart_tuning).

To make Amazon S3 data in a materialized view available in a
[supported BigQuery region](https://docs.cloud.google.com/bigquery/docs/locations#omni-loc) for joins,
[create a replica of the materialized view](https://docs.cloud.google.com/bigquery/docs/load-data-using-cross-cloud-transfer#materialized_view_replicas).
You can only create materialized view replicas over
[authorized material views](https://docs.cloud.google.com/bigquery/docs/authorized-views).

## Limitations

In addition to the [limitations for BigLake tables](https://docs.cloud.google.com/bigquery/docs/biglake-intro#limitations), the following limitations apply to
BigQuery Omni, which includes BigLake tables based
on Amazon S3 and Blob Storage data:

- Working with data in any of the [BigQuery Omni
  regions](https://docs.cloud.google.com/bigquery/docs/locations#omni-loc) is not supported by the Standard and Enterprise Plus editions. For more information about editions, see [Introduction
  to
  BigQuery editions](https://docs.cloud.google.com/bigquery/docs/editions-intro).
- The `OBJECT_PRIVILEGES`, `STREAMING_TIMELINE_BY_*`, `TABLE_SNAPSHOTS`, `TABLE_STORAGE`, `TABLE_CONSTRAINTS`, `KEY_COLUMN_USAGE`, `CONSTRAINT_COLUMN_USAGE`, and `PARTITIONS` [`INFORMATION_SCHEMA` views](https://docs.cloud.google.com/bigquery/docs/information-schema-intro) are not available for BigLake tables based on Amazon S3 and Blob Storage data.
- Materialized views are not supported for Blob Storage.
- JavaScript UDFs are not supported.
- The following SQL statements are not supported:

  - [BigQuery ML](https://docs.cloud.google.com/bigquery/docs/bqml-introduction) statements.
  - [Data definition language (DDL) statements](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language) that require data managed in BigQuery. For example, `CREATE EXTERNAL TABLE`, `CREATE SCHEMA`, or `CREATE RESERVATION` are supported, but `CREATE TABLE` is not.
  - [Data manipulation language (DML) statements](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/dml-syntax).
- The following limitations apply on querying and reading destination temporary
  tables:

  - Querying destination temporary tables with the `SELECT` statement is not supported.
- [Scheduled queries](https://docs.cloud.google.com/bigquery/docs/scheduling-queries)
  are only supported through the API or CLI method. The [destination table](https://docs.cloud.google.com/bigquery/docs/scheduling-queries#destination_table)
  option is disabled for queries. Only [`EXPORT DATA`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/export-statements) queries are allowed.

- [BigQuery Storage API](https://docs.cloud.google.com/bigquery/docs/reference/storage/libraries) is not
  available in the [BigQuery Omni regions](https://docs.cloud.google.com/bigquery/docs/locations#omni-loc).

- If your query uses the `ORDER BY` clause and has a result size larger than
  256 MB, then your query fails. To resolve this, either reduce the result
  size or remove the `ORDER BY` clause from the query. For more information
  about BigQuery Omni quotas, see [Quotas and limits](https://docs.cloud.google.com/bigquery/docs/omni-introduction#quotas_and_limits).

- Using customer-managed encryption keys (CMEK) with datasets and external
  tables is not supported.

## Pricing

For information about pricing and limited-time offers in
BigQuery Omni, see [BigQuery Omni pricing](https://cloud.google.com/bigquery/pricing#bqomni).

## Quotas and limits

For information about BigQuery Omni quotas, see
[Quotas and limits](https://docs.cloud.google.com/bigquery/quotas#query_jobs).

If your query result is larger
than 20 GiB, consider exporting the results to [Amazon S3](https://docs.cloud.google.com/bigquery/docs/omni-aws-export-results-to-s3) or [Blob Storage](https://docs.cloud.google.com/bigquery/docs/omni-azure-export-results-to-azure-storage).
To learn about quotas for BigQuery Connection API, see [BigQuery Connection API](https://docs.cloud.google.com/bigquery/quotas#connection_api).

## Locations

BigQuery Omni processes
queries in the same location as the dataset that contains the tables you're
querying. After you create the dataset, the location cannot be changed. Your
data resides within your AWS or Azure account. BigQuery Omni regions
support Enterprise edition reservations and on-demand compute (analysis)
pricing. For more information about editions, see
[Introduction to BigQuery editions](https://docs.cloud.google.com/bigquery/docs/editions-intro).

|   | Region description | Region name | Colocated BigQuery region |
|---|---|---|---|
| **AWS** ||||
|   | AWS - US East (N. Virginia) | `aws-us-east-1` | `us-east4` |
|   | AWS - US West (Oregon) | `aws-us-west-2` | `us-west1` |
|   | AWS - Asia Pacific (Seoul) | `aws-ap-northeast-2` | `asia-northeast3` |
|   | AWS - Asia Pacific (Sydney) | `aws-ap-southeast-2` | `australia-southeast1` |
|   | AWS - Europe (Ireland) | `aws-eu-west-1` | `europe-west1` |
|   | AWS - Europe (Frankfurt) | `aws-eu-central-1` | `europe-west3` |
| **Azure** ||||
|   | Azure - East US 2 | `azure-eastus2` | `us-east4` |

## What's next

- Learn how to [connect to Amazon S3](https://docs.cloud.google.com/bigquery/docs/omni-aws-create-connection) and [Blob Storage](https://docs.cloud.google.com/bigquery/docs/omni-azure-create-connection).
- Learn how to create [Amazon S3](https://docs.cloud.google.com/bigquery/docs/omni-aws-create-external-table) and [Blob Storage](https://docs.cloud.google.com/bigquery/docs/omni-azure-create-external-table) BigLake tables.
- Learn how to query [Amazon S3](https://docs.cloud.google.com/bigquery/docs/query-aws-data) and [Blob Storage](https://docs.cloud.google.com/bigquery/docs/query-azure-data) BigLake tables.
- Learn how to join [Amazon S3](https://docs.cloud.google.com/bigquery/docs/query-aws-data) and [Blob Storage](https://docs.cloud.google.com/bigquery/docs/query-azure-data) BigLake tables with Google Cloud tables using [cross-cloud joins](https://docs.cloud.google.com/bigquery/docs/biglake-intro#cross-cloud_joins).
- Learn how to [export query results to Amazon S3](https://docs.cloud.google.com/bigquery/docs/omni-aws-export-results-to-s3) and [Blob Storage](https://docs.cloud.google.com/bigquery/docs/omni-azure-export-results-to-azure-storage).
- Learn how to [transfer data from Amazon S3](https://docs.cloud.google.com/bigquery/docs/omni-aws-cross-cloud-transfer) and [Blob Storage to BigQuery](https://docs.cloud.google.com/bigquery/docs/omni-azure-cross-cloud-transfer).
- Learn about [setting up VPC Service Controls perimeter](https://docs.cloud.google.com/bigquery/docs/omni-vpc-sc).
- Learn how to [specify your location](https://docs.cloud.google.com/bigquery/docs/locations#specify_locations)