# Metadata caching for external tables

This document describes how to use metadata caching (also known as
*column metadata indexing*) to improve query performance on object tables and
some types of BigLake tables.

Object tables and some types of BigLake tables can cache metadata
information about files in external datastores---for example,
Cloud Storage. The following types of BigLake tables
support metadata caching:

- Amazon S3 BigLake tables
- Cloud Storage BigLake tables

The metadata includes file names, partitioning information, and metadata for
files such as row counts. You can choose whether to enable metadata
caching on a table. Queries with a large number of files and with
Hive partition filters benefit the most from metadata
caching.

If you don't enable metadata caching, queries on the table must read the
external data source to get object metadata. Reading this data increases the
query latency; listing millions of files from the external data source can take
several minutes. If you enable metadata caching, queries can avoid listing files
from the external data source and can partition and prune files more quickly.

You can enable metadata caching on a BigLake or object table
when you create the table. For more information about creating object tables,
see
[Create object tables](https://docs.cloud.google.com/bigquery/docs/object-tables). For more information about
creating BigLake tables, see one of the following topics:

- [Create Amazon S3 BigLake external tables](https://docs.cloud.google.com/bigquery/docs/omni-aws-create-external-table)
- [Create BigLake external tables for Cloud Storage](https://docs.cloud.google.com/bigquery/docs/create-cloud-storage-table-biglake)

## Metadata caching settings

There are two properties that control the behavior of this feature:

- **Maximum staleness** specifies when queries use cached metadata.
- **Metadata cache mode** specifies how the metadata is collected.

When you have metadata caching enabled, you specify the maximum interval of
metadata staleness that is acceptable for operations against the table. For
example, if you specify an interval of 1 hour, then operations against the table
use cached metadata if it has been refreshed within the past hour. If the cached
metadata is older than that, the operation falls back to retrieving metadata
from the datastore (Amazon S3 or Cloud Storage) instead. You
can specify a staleness interval between 30 minutes and 7 days.

You can choose to refresh the cache either automatically or manually:

- For automatic refreshes, the cache is refreshed at a system-defined interval, usually somewhere between 30 and 60 minutes. Refreshing the cache automatically is a good approach if the files in the datastore are added, deleted, or modified at random intervals. If you need to control the timing of the refresh, for example to trigger the refresh at the end of an extract-transform-load job, use manual refresh.
- For manual refreshes, you run the [`BQ.REFRESH_EXTERNAL_METADATA_CACHE` system procedure](https://docs.cloud.google.com/bigquery/docs/reference/system-procedures#bqrefresh_external_metadata_cache) to refresh the metadata cache on whatever schedule you determine. For BigLake tables, you can refresh the metadata selectively by providing subdirectories of the table data directory. This approach lets you avoid unnecessary metadata processing. Refreshing the cache manually is a good approach if the files in the datastore are added, deleted, or modified at known intervals---for example, as the output of a pipeline.

Both manual and automatic cache refreshes are executed with
[`INTERACTIVE`](https://docs.cloud.google.com/bigquery/docs/running-queries) query priority.

If you choose to use automatic refreshes, we recommend that you create a
[reservation](https://docs.cloud.google.com/bigquery/docs/reservations-intro), and then create an
[assignment with a `BACKGROUND` job type](https://docs.cloud.google.com/bigquery/docs/reservations-workload-management#assignments)
for the project that runs the metadata cache refresh jobs. This prevents the
refresh jobs from competing with user queries for resources, and
potentially failing if there aren't sufficient resources available for them.

You should consider how the staleness interval and metadata caching mode
values will interact before you set them. Consider the following examples:

- If a table's metadata cache is set to require manual refreshes, and the staleness interval is set to 2 days, you must run the `BQ.REFRESH_EXTERNAL_METADATA_CACHE` system procedure every 2 days or less if you want operations against the table to use cached metadata.
- If a table's metadata cache is set to refresh automatically, and the staleness interval is set to 30 minutes, some of the operations against the table might read from the datastore if the metadata cache refresh takes on the longer side of the usual 30- to 60-minute window.

For more information on setting metadata caching options for
BigLake tables, see
[Create Amazon S3 BigLake external tables](https://docs.cloud.google.com/bigquery/docs/omni-aws-create-external-table)
or
[Create BigLake external tables for Cloud Storage](https://docs.cloud.google.com/bigquery/docs/create-cloud-storage-table-biglake).

For more information on setting metadata caching options for object tables, see
[Create object tables](https://docs.cloud.google.com/bigquery/docs/object-tables#create-object-table).

## Get information on metadata cache refresh jobs

To find information about metadata cache refresh jobs, query the
[`INFORMATION_SCHEMA.JOBS` view](https://docs.cloud.google.com/bigquery/docs/information-schema-jobs),
as shown in the following example:

```sql
SELECT *
FROM `region-us.INFORMATION_SCHEMA.JOBS`
WHERE job_id LIKE '%metadata_cache_refresh%'
AND creation_time > TIMESTAMP_SUB(CURRENT_TIMESTAMP(), INTERVAL 6 HOUR)
ORDER BY start_time DESC
LIMIT 10;
```

## Use customer-managed encryption keys with cached metadata

Cached metadata is protected by the
[customer-managed encryption key (CMEK)](https://docs.cloud.google.com/bigquery/docs/customer-managed-encryption)
used for the table that the cached metadata is
associated with. This might be a CMEK applied directly to the table, or
a CMEK that the table inherits from the dataset or project.

If a default CMEK is set for the project or dataset, or if the existing CMEK for
the project or dataset is changed, this doesn't affect existing tables or
their cached metadata. You must
[change the key for the table](https://docs.cloud.google.com/bigquery/docs/customer-managed-encryption#change_key)
to apply the new key to both the table and its cached metadata.

CMEKs created in BigQuery don't apply to the
Cloud Storage files that are used by BigLake and
object tables. To obtain end-to-end CMEK encryption,
[configure CMEKs in Cloud Storage](https://docs.cloud.google.com/storage/docs/encryption/customer-managed-keys)
for those files.

## Get information on metadata cache usage by query jobs

To get information about metadata cache usage for a query job, call the
[`jobs.get` method](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs/get) for that job
and look at the
[`MetadataCacheStatistics` field](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/Job#metadatacachestatistics)
in the
[`JobStatistics2` section](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/Job#jobstatistics2)
of the `Job` resource. This field provides information on which metadata
cache-enabled tables were used by the query, whether the metadata cache was
used by the query, and if not, the reason why not.

## Table statistics

For BigLake tables that are based on Parquet files, table
statistics are collected when the metadata cache is refreshed. Table statistic
collection happens during both automatic and manual refreshes, and the
statistics are kept for the same period as the metadata cache.

The table statistics collected include file information like row counts,
physical and uncompressed file sizes, and cardinality of columns. When you run
a query on a Parquet-based BigLake table, these statistics
are supplied to the query optimizer to enable better query planning and
potentially improve query performance for some types of queries. For example,
a common query optimization is dynamic constraint propagation, where the
query optimizer dynamically infers predicates on the larger fact tables in a
join from the smaller dimension tables. While this optimization can speed up
queries by using normalized table schemas, it requires accurate table
statistics. The table statistics collected by metadata caching enable greater
optimization of query plans in both BigQuery and
Apache Spark.

## Limitations

The following limitations apply to the metadata cache:

- If you issue multiple concurrent manual refreshes, only one will succeed.
- The metadata cache expires after 7 days if it isn't refreshed.
- If you update the source URI for a table, the metadata cache is not automatically refreshed, and subsequent queries return data from the outdated cache. To avoid this, refresh the metadata cache manually. If the table's metadata cache is set to refresh automatically, you must change the table's refresh mode to manual, perform the manual refresh, then set the table's refresh mode back to automatic again.
- If you are manually refreshing the metadata cache, and your target dataset
  and Cloud Storage bucket are in a
  [regional](https://docs.cloud.google.com/bigquery/docs/locations#regions) location, you must explicitly
  specify this location when you run the
  [`BQ.REFRESH_EXTERNAL_METADATA_CACHE`](https://docs.cloud.google.com/bigquery/docs/reference/system-procedures#bqrefresh_external_metadata_cache)
  procedure call. You can do this one of the following ways:

  ### Console

  1. Go to the **BigQuery** page.

     [Go to BigQuery](https://console.cloud.google.com/bigquery)
  2. Select a tab in the Editor.

  3. Click
     **More** , and then
     click **Query settings**.

  4. In the **Location** section, unselect the
     **Automatic location selection** checkbox, and then specify the
     target region.

  5. Click **Save**.

  6. Run the query containing the
     `BQ.REFRESH_EXTERNAL_METADATA_CACHE` procedure call in that Editor tab.

  ### bq

  If you run the query containing the `BQ.REFRESH_EXTERNAL_METADATA_CACHE`
  procedure call by using
  [`bq query`](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#bq_query), be sure
  to specify the
  [`--location` flag](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#global_flags).

## What's next

- Learn more about [creating Cloud Storage BigLake tables with metadata caching](https://docs.cloud.google.com/bigquery/docs/create-cloud-storage-table-biglake).
- Learn more about [creating Amazon S3 BigLake tables with metadata caching](https://docs.cloud.google.com/bigquery/docs/omni-aws-create-external-table).
- Learn more about [creating object tables with metadata caching](https://docs.cloud.google.com/bigquery/docs/object-tables).
- Learn about [using materialized views over BigLake metadata cache-enabled tables](https://docs.cloud.google.com/bigquery/docs/materialized-views-intro#biglake).