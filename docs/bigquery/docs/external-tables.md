# Introduction to external tables

This document describes how to work with data stored outside of BigQuery in external tables. To work with external data sources, you can also use [External datasets](https://docs.cloud.google.com/bigquery/docs/datasets-intro#external_datasets).

Non-BigLake external tables let you query structured data
in external data stores. To query a non-BigLake external
table, you must have permissions to both the external table and the
external data source. For example, to query a non-BigLake
external table that uses a data source in Cloud Storage,
you must have the following permissions:

- `bigquery.tables.getData`
- `bigquery.jobs.create`
- `storage.buckets.get`
- `storage.objects.get`

## Supported data stores

You can use non-BigLake external tables with the
following data stores:

- [Cloud Storage](https://docs.cloud.google.com/bigquery/docs/external-data-cloud-storage)
- [Bigtable](https://docs.cloud.google.com/bigquery/docs/external-data-bigtable)
- [Google Drive](https://docs.cloud.google.com/bigquery/docs/external-data-drive)

## Temporary table support

You can query an external data source in BigQuery by using a permanent table or a
temporary table. A permanent table is a table that is created in a dataset and is linked to your
external data source. Because the table is permanent, you can use
[access controls](https://docs.cloud.google.com/bigquery/docs/access-control) to share the
table with others who also have access to the underlying external data source, and you can query the
table at any time.

When you query an external data source using a temporary table, you submit a command that
includes a query and creates a non-permanent table linked to the external data source. When you use
a temporary table, you do not create a table in one of your BigQuery datasets.
Because the table is not permanently stored in a dataset, it cannot be shared with others. Querying
an external data source using a temporary table is useful for one-time, ad-hoc queries over external
data, or for extract, transform, and load (ETL) processes.

## Multiple source files

If you create a non-BigLake external table based on
Cloud Storage, then you can use multiple external data sources,
provided those data sources have the same schema. This isn't supported
for non-BigLake external table based on Bigtable
or Google Drive.

## Limitations

The following limitations apply to external tables:

- BigQuery does not guarantee data consistency for external data tables. Changes to the underlying data while a query is running can result in unexpected behavior.
- Query performance for external tables might be slow compared to querying data in a standard BigQuery table. If query speed is a priority, [load the data into BigQuery](https://docs.cloud.google.com/bigquery/loading-data) instead of setting up an external data source. The performance of a query that includes an external table depends on the external storage type. For example, querying data stored in Cloud Storage is faster than querying data stored in Google Drive. In general, the query performance for an external table should be equivalent to reading the data directly from the data source.
- You cannot modify external data tables using DML or other methods. External tables are read-only for BigQuery.
- You cannot use the `TableDataList` JSON API method to retrieve data from external tables. For more information, see [`tabledata.list`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tabledata/list). To work around this limitation, you can save query results in a destination table. You can then use the `TableDataList` method on the results table.
- You cannot run a BigQuery job that exports data from an external table. To work around this limitation, you can save query results in a destination table. Then, run an extract job against the results table.
- You cannot copy an external table.
- You cannot reference an external table in a [wildcard table](https://docs.cloud.google.com/bigquery/docs/querying-wildcard-tables) query.
- External tables don't support clustering. They support partitioning in limited ways. For details, see [Querying externally partitioned data](https://docs.cloud.google.com/bigquery/docs/hive-partitioned-queries-gcs).
- When you query an external data source other than Cloud Storage, the results are not [cached](https://docs.cloud.google.com/bigquery/docs/cached-results). (GoogleSQL queries on Cloud Storage are supported.) You are charged for each query against an external table even if you issue the same query multiple times. If you need to repeatedly issue a query against an external table that does not change frequently, consider [writing the query results to a permanent table](https://docs.cloud.google.com/bigquery/docs/writing-results#permanent-table) and run the queries against the permanent table instead.
- You are limited to 16 concurrent queries against a Bigtable external data source.
- A dry run of a federated query that uses an external table might report a lower bound of 0 bytes of data, even if rows are returned. This is because the amount of data processed from the external table can't be determined until the actual query completes. Running the federated query incurs a cost for processing this data.
- You can't use `_object_metadata` as a column name in external tables. It is reserved for internal use.
- BigQuery doesn't support the display of table storage statistics for external tables.
- External tables don't support [flexible column names](https://docs.cloud.google.com/bigquery/docs/schemas#flexible-column-names).
- BI Engine doesn't support queries to external tables.
- BigQuery doesn't support [Data Boost for Spanner](https://docs.cloud.google.com/bigquery/docs/spanner-federated-queries#data_boost) for [reading Bigtable data from BigQuery](https://docs.cloud.google.com/bigquery/docs/create-bigtable-external-table).
- BigQuery doesn't support [time travel or fail-safe data
  retention windows](https://docs.cloud.google.com/bigquery/docs/time-travel) for external tables. However, for Apache Iceberg external tables, you can use the [`FOR SYSTEM_TIME AS OF` clause](https://docs.cloud.google.com/bigquery/docs/access-historical-data#query_data_at_a_point_in_time) to access snapshots that are retained in your Iceberg metadata.
- All format specific limitations apply:
  - [CSV limitations](https://docs.cloud.google.com/bigquery/docs/loading-data-cloud-storage-csv#limitations)
  - [JSON limitations](https://docs.cloud.google.com/bigquery/docs/loading-data-cloud-storage-json#limitations)
  - [Parquet limitations](https://docs.cloud.google.com/bigquery/docs/loading-data-cloud-storage-parquet#limitations)
  - [ORC limitations](https://docs.cloud.google.com/bigquery/docs/loading-data-cloud-storage-orc#limitations)
  - [Avro limitations](https://docs.cloud.google.com/bigquery/docs/loading-data-cloud-storage-avro#limitations)
  - [Iceberg limitations](https://docs.cloud.google.com/bigquery/docs/iceberg-tables#limitations)
  - [Delta Lake limitations](https://docs.cloud.google.com/bigquery/docs/omni-azure-create-external-table#limitations)

## Location considerations

When you choose a location for your external table, you need to take into
consideration both the location of the BigQuery dataset and
the external data source.

### Cloud Storage

When you query data in Cloud Storage by using a
[BigLake](https://docs.cloud.google.com/bigquery/docs/query-cloud-storage-using-biglake) or a
[non-BigLake external table](https://docs.cloud.google.com/bigquery/docs/query-cloud-storage-data),
the bucket must be colocated with your BigQuery dataset
that contains the external table definition. For example:

- [Single region buckets](https://docs.cloud.google.com/storage/docs/locations#location-r)

  If your Cloud Storage bucket is in the `us-central1` (Iowa)
  region, your BigQuery dataset must be in the `us-central1` (Iowa) region or
  the `US` multi-region.

  If your Cloud Storage bucket is in the `europe-west4` (Netherlands)
  region, your BigQuery dataset must in the `europe-west4` (Netherlands)
  or the `EU` multi-region.

  If your Cloud Storage bucket is in the `europe-west1` (Belgium)
  region, the corresponding BigQuery
  dataset must also be in the `europe-west1` (Belgium) or the `EU`
  multi-region.
- [Dual-region buckets](https://docs.cloud.google.com/storage/docs/locations#location-dr)

  If your Cloud Storage bucket is in the `NAM4` predefined
  dual-region or any configurable dual-region that includes the
  `us-central1` (Iowa) region, the corresponding BigQuery
  dataset must be in the `us-central1` (Iowa) region *or* the `US` multi-region.

  If your Cloud Storage bucket is in the `EUR4` predefined
  dual-region or any configurable dual-region that includes the
  `europe-west4` (Netherlands) region, the corresponding BigQuery
  dataset must be in the `europe-west4` (Netherlands) region *or* the `EU` multi-region.

  If your Cloud Storage bucket is in the `ASIA1` predefined
  dual-region, the corresponding BigQuery
  dataset must be in the `asia-northeast1` (Tokyo) *or* the
  `asia-northeast2` (Osaka) region.

  If your Cloud Storage bucket uses a configurable dual-region that
  includes the `australia-southeast1` (Sydney) and the `australia-southeast2` (Melbourne)
  region, the corresponding BigQuery
  bucket must be in either the `australia-southeast1` (Sydney) *or* the
  `australia-southeast2` (Melbourne) region.
- [Multi-region buckets](https://docs.cloud.google.com/storage/docs/locations#location-mr)

  Using multi-region dataset locations with multi-region Cloud Storage
  buckets is *not* recommended for external tables, because external query
  performance depends on minimal latency and optimal network bandwidth.

  If your BigQuery dataset is in the `US` multi-region, the
  corresponding Cloud Storage bucket must be in the `US` multi-region,
  in the single region `us-central1` (Iowa), or in a dual-region that includes
  `us-central1` (Iowa), like the `NAM4` dual-region, or in a configurable
  dual-region that includes `us-central1`.

  If your BigQuery dataset is in the `EU` multi-region, the
  corresponding Cloud Storage bucket must be in the `EU` multi-region,
  in the single region `europe-west1` (Belgium) or `europe-west4` (Netherlands),
  or a dual-region that includes `europe-west1` (Belgium) or `europe-west4` (Netherlands),
  like the `EUR4` dual-region, or in a configurable dual-region that includes
  `europe-west1` or `europe-west4`.

For more information about supported Cloud Storage locations, see
[Bucket locations](https://docs.cloud.google.com/storage/docs/bucket-locations) in the
Cloud Storage documentation.

### Bigtable

When you [query data in Bigtable](https://docs.cloud.google.com/bigquery/docs/external-data-bigtable)
through a BigQuery external table,
your Bigtable instance must be in the same location as your
BigQuery dataset:

- Single region: If your BigQuery dataset is in the Belgium (`europe-west1`) regional location, the corresponding Bigtable instance must be in the Belgium region.
- Multi-region: Because external query performance depends on minimal latency and optimal network bandwidth, using multi-region dataset locations is *not* recommended for external tables on Bigtable.

For more information about supported Bigtable locations, see
[Bigtable locations](https://docs.cloud.google.com/bigquery/docs/create-bigtable-external-table#supported_regions_and_zones).

### Google Drive

Location considerations don't apply to [Google Drive](https://docs.cloud.google.com/bigquery/external-data-drive)
external data sources.

## Moving data between locations

To manually move a dataset from one location to another, follow these steps:

1. [Export the data](https://docs.cloud.google.com/bigquery/docs/exporting-data) from your BigQuery
   tables to a Cloud Storage bucket.

   There are no charges for exporting data from BigQuery, but you do incur charges
   for [storing the exported data](https://docs.cloud.google.com/storage/pricing#storage-pricing) in
   Cloud Storage. BigQuery exports are subject to the limits on
   [extract jobs](https://docs.cloud.google.com/bigquery/quotas#export_jobs).
2. Copy or move the data from your export Cloud Storage bucket to a new bucket you created
   in the destination location. For example, if you are moving your data from the `US`
   multi-region to the `asia-northeast1` Tokyo region, you would transfer the data to a bucket
   that you created in Tokyo. For information about transferring Cloud Storage objects, see
   [Copy, rename, and move objects](https://docs.cloud.google.com/storage/docs/copying-renaming-moving-objects)
   in the Cloud Storage documentation.

   Transferring data between regions incurs
   [network egress charges](https://docs.cloud.google.com/storage/pricing#network-pricing) in Cloud Storage.
3. Create a new BigQuery
   dataset in the new location, and then load your data from the Cloud Storage bucket
   into the new dataset.

   You are not charged for loading the data into BigQuery, but you will incur
   charges for storing the data in Cloud Storage until you delete the data or the bucket. You
   are also charged for storing the data in BigQuery after it is loaded. Loading
   data into BigQuery is subject to the
   [load jobs](https://docs.cloud.google.com/bigquery/quotas#load_jobs) limits.

You can also use [Managed Service for Apache Airflow](https://cloud.google.com/blog/products/data-analytics/how-to-transfer-bigquery-tables-between-locations-with-cloud-composer) to move and copy large datasets programmatically.

For more information about using Cloud Storage to store and move large datasets, see
[Use Cloud Storage with big data](https://docs.cloud.google.com/storage/docs/working-with-big-data).

## Optimize Cloud Storage external table queries

To optimize performance and potentially reduce costs when querying data in
Cloud Storage with external tables, consider enabling
[Rapid Cache](https://docs.cloud.google.com/storage/docs/rapid/rapid-cache).

Rapid Cache provides an SSD-backed zonal read cache for your
Cloud Storage buckets. When enabled, BigQuery leverages
Rapid Cache to serve object read requests, providing you with:

- **Accelerated query performance**: Faster data reads from Cloud Storage for your BigQuery workloads.
- **Reduced network data transfer costs**: Reduced data transfer fees for BigQuery workloads backed by multi-region buckets. Data that is read from a cache incurs lower networking costs than data that is read directly from a multi-region bucket.

BigQuery is a regional service, but its underlying compute
resources can shift between zones for load balancing, so we recommend enabling
Rapid Cache in all zones within the region where your BigQuery
workload is run. This ensures that a cache instance is available regardless of
which zone BigQuery compute uses. For more information,
see [Rapid Cache pricing](https://cloud.google.com/storage/pricing#rapid-cache).

To determine if Rapid Cache is right for you, we recommend using the
[Rapid Cache recommender](https://docs.cloud.google.com/storage/docs/rapid/rapid-cache-recommender).
The Rapid Cache recommender provides recommendations and insights
for creating caches in bucket-zone pairs by analyzing your data usage and
storage.

## Pricing

When querying an external table from BigQuery, you are
charged for running the query and the applicable bytes read if using
[BigQuery on-demand](https://cloud.google.com/bigquery/pricing#on_demand_pricing) (per
TiB) pricing or slot consumption if using [BigQuery capacity](https://cloud.google.com/bigquery/pricing#capacity_compute_analysis_pricing) (per slot-hour) pricing.

If your data is stored in ORC or Parquet on Cloud Storage, see
[Data size calculation](https://docs.cloud.google.com/bigquery/docs/best-practices-costs#estimate-query-costs).

You are also charged for storing the data and any resources used by the source
application, subject to the application's pricing guidelines:

- For information on Cloud Storage pricing, see
  [Cloud Storage pricing](https://cloud.google.com/storage/pricing).
  Cloud Storage charges might include the following:

  - Data storage costs.
  - Data retrieval costs for accessing data in [Nearline](https://docs.cloud.google.com/storage/docs/storage-classes#nearline), [Coldline](https://docs.cloud.google.com/storage/docs/storage-classes#coldline), and [Archive](https://docs.cloud.google.com/storage/docs/storage-classes#archive) storage classes.
  - Network usage costs for data that you read across different regions.
  - Data processing charges. However, you aren't charged for API calls that are made by BigQuery on your behalf.
- For information on Bigtable pricing, see [Pricing](https://cloud.google.com/bigtable/pricing).

- For information on Drive pricing, see [Pricing](https://gsuite.google.com/pricing.html).

## What's next

- Learn how to [create a Bigtable external table](https://docs.cloud.google.com/bigquery/docs/create-bigtable-external-table).
- Learn how to [create a Cloud Storage external table](https://docs.cloud.google.com/bigquery/docs/external-data-cloud-storage).
- Learn how to [create a Drive external table](https://docs.cloud.google.com/bigquery/docs/external-data-drive).
- Learn how to [schedule and run data quality checks with Knowledge Catalog](https://docs.cloud.google.com/bigquery/docs/dataplex-shared-introduction).