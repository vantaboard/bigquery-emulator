# Query open table formats with manifests

This document describes how to use manifest files to query data stored in open
table formats such as [Apache Hudi](https://github.com/apache/hudi) and [Delta Lake](https://github.com/delta-io).

Some open table formats such as Hudi and Delta Lake
export their current state as one or more manifest files. A manifest file contains
a list of data files that make tables. With the manifest support in
BigQuery, you can query and load data stored in open table formats.

## Before you begin

-


  Enable the BigQuery Connection,
  BigQuery Reservation, and BigLake APIs.


  **Roles required to enable APIs**


  To enable APIs, you need the Service Usage Admin IAM
  role (`roles/serviceusage.serviceUsageAdmin`), which
  contains the `serviceusage.services.enable` permission. [Learn how to grant
  roles](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).

  [Enable the APIs](https://console.cloud.google.com/flows/enableapi?apiid=bigqueryconnection.googleapis.com,biglake.googleapis.com,%0Abigqueryreservation.googleapis.com&redirect=https://console.cloud.google.com)

  <br />

- To create BigLake tables, you can
  run the Spark commands by using one of the following methods:

  - [Create a Managed Service for Apache Spark cluster](https://docs.cloud.google.com/dataproc/docs/guides/create-cluster).
    For querying Hudi tables, set the `--optional-components`
    field to `HUDI`. For querying Delta tables, set
    `--optional-components` to `Presto`.

  - Use a stored procedure for Spark in
    BigQuery. To do so, follow these steps:

    1. [Create a Spark connection](https://docs.cloud.google.com/bigquery/docs/connect-to-spark#create-spark-connection).
    2. [Set up access control for that connection](https://docs.cloud.google.com/bigquery/docs/connect-to-spark#grant-access).
- To store the manifest file in Cloud Storage,
  [create a Cloud Storage bucket](https://docs.cloud.google.com/storage/docs/creating-buckets). You
  need to connect to your Cloud Storage bucket to access the manifest file.
  To do so, follow these steps:

  1. [Create a Cloud resource connection](https://docs.cloud.google.com/bigquery/docs/create-cloud-resource-connection#create-cloud-resource-connection).
  2. [Set up access for that connection](https://docs.cloud.google.com/bigquery/docs/create-cloud-resource-connection#access-storage).

### Required roles

To query BigLake tables based on Hudi and
Delta Lake data, ensure you have the following roles:

- BigQuery Connection User (`roles/bigquery.connectionUser`)
- BigQuery Data Viewer (`roles/bigquery.dataViewer`)
- BigQuery User (`roles/bigquery.user`)

You can also query Hudi external tables. However, we
recommend you to [upgrade the external table to BigLake](https://docs.cloud.google.com/bigquery/docs/query-open-table-format-using-manifest-files#upgrade-external-to-biglake).
To query Hudi external tables, ensure you have the
following roles:

- BigQuery Data Viewer (`roles/bigquery.dataViewer`)
- BigQuery User (`roles/bigquery.user`)
- Storage Object Viewer (`roles/storage.objectViewer`)

Depending on your permissions, you can
grant these roles to yourself or ask your administrator
to grant them to you. For more information about granting roles, see
[Viewing the grantable roles on resources](https://docs.cloud.google.com/iam/docs/viewing-grantable-roles).

To see the exact permissions that are required to query
BigLake tables, expand the **Required permissions** section:

#### Required permissions

- `bigquery.connections.use`
- `bigquery.jobs.create`
- `bigquery.readsessions.create` (Only required if you are [reading data with the
  BigQuery Storage Read API](https://docs.cloud.google.com/bigquery/docs/reference/storage))
- `bigquery.tables.get`
- `bigquery.tables.getData`

You might also be able to get these permissions with [custom roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles)
or other [predefined roles](https://docs.cloud.google.com/iam/docs/roles-overview#predefined).

## Query Hudi workloads

> [!IMPORTANT]
> **Important:** To perform the actions described in the following sections, you must use either the [Hudi-BigQuery
> connector](https://hudi.apache.org/docs/gcp_bigquery/) version 0.14.0 or greater, or the Hudi component in Managed Service for Apache Spark 2.1, which has the appropriate version of the connector backported. In the previous versions, the connector created views on the manifest files which wasn't optimal for query performance. If you are using the previous version of the connector, then you must drop the existing view that represents the Hudi table in BigQuery to avoid schema mismatch error.

To [query Hudi data](https://docs.cloud.google.com/bigquery/docs/query-open-table-format-using-manifest-files#query-biglake-external-table),
follow these steps:

1. [Create an external table](https://docs.cloud.google.com/bigquery/docs/query-open-table-format-using-manifest-files#create-hudi-external-tables) based on Hudi data.
2. [Upgrade the external table to BigLake](https://docs.cloud.google.com/bigquery/docs/query-open-table-format-using-manifest-files#upgrade-biglake-tables).

### Create Hudi external tables

When you sync tables by using the sync tool for Hudi and
BigQuery, enable the `use-bq-manifest-file` flag to transition
to the manifest file approach. This flag also exports a manifest file in a
format supported by BigQuery and uses it to create an external
table with the name specified in the `--table` parameter.

To create a Hudi external table, follow these steps:

1. To create a Hudi external table,
   [submit a job](https://docs.cloud.google.com/dataproc/docs/guides/submit-job) to an existing
   Managed Service for Apache Spark cluster. When you build the
   Hudi-BigQuery connector, enable the `use-bq-manifest-file` flag
   to transition to the manifest file approach. This flag exports a
   manifest file in a format supported by BigQuery and uses it to
   create an external table with the name specified in the `--table` parameter.

   ```
   spark-submit \
      --master yarn \
      --packages com.google.cloud:google-cloud-bigquery:2.10.4 \
      --class org.apache.hudi.gcp.bigquery.BigQuerySyncTool  \
      JAR \
      --project-id PROJECT_ID \
      --dataset-name DATASET \
      --dataset-location LOCATION \
      --table TABLE \
      --source-uri URI  \
      --source-uri-prefix URI_PREFIX \
      --base-path BASE_PATH  \
      --partitioned-by PARTITION_BY \
      --use-bq-manifest-file
   ```

   Replace the following:
   - `JAR`: If you are using the Hudi-BigQuery connector, specify `hudi-gcp-bundle-0.14.0.jar`. If you are using
     the Hudi component in Managed Service for Apache Spark 2.1, specify `/usr/lib/hudi/tools/bq-sync-tool/hudi-gcp-bundle-0.12.3.1.jar`

   - `PROJECT_ID`: the project ID in which you want to
     create the Hudi BigLake table

   - `DATASET`: the dataset in which you want to create
     the Hudi BigLake table

   - `LOCATION`: the location in which you want to create
     the Hudi BigLake table

   - `TABLE`: the name of the table that you want to create

     If you are transitioning from the earlier version of the
     Hudi-BigQuery
     connector (0.13.0 and earlier) that created views on
     the manifest files, ensure that you use the same table name as it lets you
     keep the existing downstream pipeline code.
   - `URI`: the Cloud Storage URI that you created
     to store the Hudi manifest file

     This URI points to the first level partition; make sure to include the
     partition key. For example, `gs://mybucket/hudi/mydataset/EventDate=*`
   - `URI_PREFIX`: the prefix for the Cloud Storage
     URI path, usually it's the path to Hudi tables

   - `BASE_PATH`: the base path for Hudi
     tables

     For example, `gs://mybucket/hudi/mydataset/`
   - `PARTITION_BY`: the partition value

     For example, `EventDate`

   For more information about the connector's configuration, see
   [Hudi-BigQuery connector](https://hudi.apache.org/docs/gcp_bigquery/).
2. To set appropriate fine-grained controls or to accelerate the performance
   by enabling metadata caching, see [Upgrade BigLake tables](https://docs.cloud.google.com/bigquery/docs/query-open-table-format-using-manifest-files#upgrade-biglake-tables).

## Query Delta workloads

Delta tables are now [natively supported](https://docs.cloud.google.com/bigquery/docs/create-delta-lake-table). We recommend creating Delta BigLake tables for Delta workloads. Delta Lake BigLake tables support more advanced [Delta Lake tables](https://github.com/delta-io), including tables with column remapping and deletion vectors. Additionally, Delta BigLake tables directly read the latest snapshot, so updates are instantly available.

To [query Delta workloads](https://docs.cloud.google.com/bigquery/docs/query-open-table-format-using-manifest-files#query-biglake-external-table),
follow these steps:

1. [Generate a manifest file](https://docs.cloud.google.com/bigquery/docs/query-open-table-format-using-manifest-files#generate-manifest-file).
2. [Create a BigLake table](https://docs.cloud.google.com/bigquery/docs/query-open-table-format-using-manifest-files#create-delta-lake-biglake-tables) based on the manifest file.
3. Set appropriate fine-grained controls or accelerate the performance by enabling metadata caching. To do this, see [Upgrade BigLake tables](https://docs.cloud.google.com/bigquery/docs/query-open-table-format-using-manifest-files#upgrade-biglake-tables).

### Generate a manifest file

BigQuery supports the manifest file in a
[`SymLinkTextInputFormat`](https://github.com/apache/hive/blob/master/ql/src/java/org/apache/hadoop/hive/ql/io/SymlinkTextInputFormat.java) format, which is a
newline-delimited list of URIs. For more information about generating a manifest
file, see [Set up Presto to Delta Lake integration and query Delta tables](https://docs.delta.io/latest/presto-integration.html#set-up-the-presto-trino-or-athena-to-delta-lake-integration-and-query-delta-tables).

To generate a manifest file, [submit a job](https://docs.cloud.google.com/dataproc/docs/guides/submit-job)
to an existing Managed Service for Apache Spark cluster:

### SQL

Using Spark, run the following command on a Delta table at location `path-to-delta-table`:

```
GENERATE symlink_format_manifest FOR TABLE delta.`<path-to-delta-table>`
```

### Scala

Using Spark, run the following command on a Delta table at location `path-to-delta-table`:

```
val deltaTable = DeltaTable.forPath(<path-to-delta-table>)
deltaTable.generate("symlink_format_manifest")
```

### Java

Using Spark, run the following command on a Delta table at location `path-to-delta-table`:

```
DeltaTable deltaTable = DeltaTable.forPath(<path-to-delta-table>);
deltaTable.generate("symlink_format_manifest");
```

### Python

Using Spark, run the following command on a Delta table at location `path-to-delta-table`:

```
deltaTable = DeltaTable.forPath(<path-to-delta-table>)
deltaTable.generate("symlink_format_manifest")
```

### Create Delta BigLake tables

To create a Delta BigLake table, use the
[`CREATE EXTERNAL TABLE` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_external_table_statement) with the
`file_set_spec_type` field set to `NEW_LINE_DELIMITED_MANIFEST`:

1. Go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, run the `CREATE EXTERNAL TABLE` statement:

   ```sh
   CREATE EXTERNAL TABLE PROJECT_ID.DATASET_NAME.TABLE_NAME
   WITH PARTITION COLUMNS(
   `PARTITION_COLUMN PARTITION_COLUMN_TYPE`,)
   WITH CONNECTION `PROJECT_IDREGION.CONNECTION_NAME`
   OPTIONS (
      format = "DATA_FORMAT",
      uris = ["URI"],
      file_set_spec_type = 'NEW_LINE_DELIMITED_MANIFEST',
      hive_partition_uri_prefix = "PATH_TO_DELTA_TABLE"
      max_staleness = STALENESS_INTERVAL,
      metadata_cache_mode = 'CACHE_MODE');
   ```

   Replace the following:
   - `DATASET_NAME`: the name of the dataset you created
   - `TABLE_NAME`: the name you want to give to this table
   - `REGION`: the location where the connection is located (for example, `us-east1`)
   - `CONNECTION_NAME`: the name of the connection you created
   - `DATA_FORMAT`: any of the supported [formats](https://docs.cloud.google.com/bigquery/docs/biglake-intro) (such as `PARQUET`)
   - `URI`: the path to the manifest file (for example, `gs://mybucket/path`)
   - `PATH_TO_DELTA_TABLE`: a common prefix for all source URIs before the partition key encoding begins
   - `STALENESS_INTERVAL`: specifies whether cached metadata is used by operations against the BigLake table, and how fresh the cached metadata must be in order for the operation to use it. For more information about metadata caching considerations, see [Metadata caching for performance](https://docs.cloud.google.com/bigquery/docs/omni-introduction#metadata_caching_for_performance).

     To disable metadata caching, specify 0. This is the default.

     To enable metadata caching, specify an
     [interval literal](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/lexical#interval_literals)
     value between 30 minutes and 7 days. For example, specify
     `INTERVAL 4 HOUR` for a 4 hour staleness interval.
     With this value, operations against the table use cached metadata if
     it has been refreshed within the past 4 hours. If the cached metadata
     is older than that, the operation retrieves metadata from
     Delta Lake instead.
   - `CACHE_MODE`: specifies whether the metadata cache is refreshed automatically or manually. For more information about metadata caching considerations, see [Metadata caching for performance](https://docs.cloud.google.com/bigquery/docs/omni-introduction#metadata_caching_for_performance).

     Set to `AUTOMATIC` for the metadata cache to be
     refreshed at a system-defined interval, usually somewhere between 30 and
     60 minutes.

     Set to `MANUAL` if you want to refresh
     the metadata cache on a schedule you determine. In this case, you can call
     the [`BQ.REFRESH_EXTERNAL_METADATA_CACHE` system procedure](https://docs.cloud.google.com/bigquery/docs/reference/system-procedures#bqrefresh_external_metadata_cache) to refresh the cache.

     You must set `CACHE_MODE` if
     `STALENESS_INTERVAL` is set to a value greater
     than 0.

   Example:

   ```googlesql
   CREATE EXTERNAL TABLE mydataset.mytable
   WITH CONNECTION `us-east1.myconnection`
   OPTIONS (
       format="PARQUET",
       uris=["gs://mybucket/path/partitionpath=*"],
       file_set_spec_type = 'NEW_LINE_DELIMITED_MANIFEST'
       hive_partition_uri_prefix = "gs://mybucket/path/"
       max_staleness = INTERVAL 1 DAY,
       metadata_cache_mode = 'AUTOMATIC'
   );
   ```

## Upgrade BigLake tables

You can also [accelerate the performance](https://cloud.google.com/blog/products/data-analytics/deep-dive-on-how-biglake-accelerates-query-performance)
of your workloads by taking advantage of
[metadata caching](https://docs.cloud.google.com/bigquery/docs/biglake-intro#metadata_caching_for_performance)
and
[materialized views](https://docs.cloud.google.com/bigquery/docs/materialized-views-intro#biglake). If you want to use
metadata caching, you can specify settings for this at
the same time. To get table details such as source format and source URI, see
[Get table information](https://docs.cloud.google.com/bigquery/docs/tables#get_table_information).

To update an external table to a BigLake table or update an
existing BigLake, select one of the following options:

### SQL

Use the
[`CREATE OR REPLACE EXTERNAL TABLE` DDL statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_external_table_statement)
to update a table:

<br />

<br />

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, enter the following statement:

   ```googlesql
   CREATE OR REPLACE EXTERNAL TABLE
     `PROJECT_ID.DATASET.EXTERNAL_TABLE_NAME`
     WITH CONNECTION {`REGION.CONNECTION_ID` | DEFAULT}
     OPTIONS(
       format ="TABLE_FORMAT",
       uris = ['BUCKET_PATH'],
       max_staleness = STALENESS_INTERVAL,
       metadata_cache_mode = 'CACHE_MODE'
       );
   ```


   Replace the following:
   - `PROJECT_ID`: the name of the project that contains the table
   - `DATASET`: the name of the dataset that contains the table
   - `EXTERNAL_TABLE_NAME`: the name of the table
   - `REGION`: the region that contains the connection
   - `CONNECTION_ID`: the name of the connection to use

     To use a [default connection](https://docs.cloud.google.com/bigquery/docs/default-connections), specify `DEFAULT` instead of the
     connection string containing
     `REGION.CONNECTION_ID`.
   - `TABLE_FORMAT`: the format used by the table

     <br />

     You can't change this when updating the table.
   - `BUCKET_PATH`: the path to the Cloud Storage bucket that contains the data for the external table, in the format `['gs://bucket_name/[folder_name/]file_name']`.

     You can select multiple files from the bucket by specifying one asterisk (`*`)
     wildcard character in the path. For example, `['gs://mybucket/file_name*']`. For more
     information, see
     [Wildcard support for Cloud Storage URIs](https://docs.cloud.google.com/bigquery/docs/external-data-cloud-storage#wildcard-support).

     You can specify multiple buckets for the `uris` option by providing multiple
     paths.

     The following examples show valid `uris` values:
     - `['gs://bucket/path1/myfile.csv']`
     - `['gs://bucket/path1/*.csv']`
     - `['gs://bucket/path1/*', 'gs://bucket/path2/file00*']`

     When you specify `uris` values that target multiple files, all of those
     files must share a compatible schema.

     For more information about using Cloud Storage URIs in
     BigQuery, see
     [Cloud Storage resource path](https://docs.cloud.google.com/bigquery/docs/external-data-cloud-storage#google-cloud-storage-uri).
   - `STALENESS_INTERVAL`: specifies whether cached metadata is used by operations against the table, and how fresh the cached metadata must be in order for the operation to use it

     <br />

     For more information about metadata caching considerations, see
     [Metadata caching for performance](https://docs.cloud.google.com/bigquery/docs/biglake-intro#metadata_caching_for_performance).

     To disable metadata caching, specify 0. This is the default.

     To enable metadata caching, specify an
     [interval literal](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/lexical#interval_literals)
     value between 30 minutes and 7 days. For example, specify
     `INTERVAL 4 HOUR` for a 4 hour staleness interval.
     With this value, operations against the table use cached metadata if
     it has been refreshed within the past 4 hours. If the cached metadata
     is older than that, the operation retrieves metadata from
     Cloud Storage instead.
   - `CACHE_MODE`: specifies whether the metadata cache is refreshed automatically or manually

     <br />

     For more information
     on metadata caching considerations, see
     [Metadata caching for performance](https://docs.cloud.google.com/bigquery/docs/biglake-intro#metadata_caching_for_performance).

     Set to `AUTOMATIC` for the metadata cache to be
     refreshed at a system-defined interval, usually somewhere between 30 and
     60 minutes.

     Set to `MANUAL` if you want to refresh
     the metadata cache on a schedule you determine. In this case, you can call
     the [`BQ.REFRESH_EXTERNAL_METADATA_CACHE` system procedure](https://docs.cloud.google.com/bigquery/docs/reference/system-procedures#bqrefresh_external_metadata_cache) to refresh
     the cache.

     You must set `CACHE_MODE` if
     `STALENESS_INTERVAL` is set to a value greater
     than 0.

   <br />

3. Click **Run**.

   <br />

For more information about how to run queries, see [Run an interactive query](https://docs.cloud.google.com/bigquery/docs/running-queries#queries).

### bq

Use the [`bq mkdef`](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#bq_mkdef) and
[`bq update`](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#bq_update) commands
to update a table:

1. Generate an
   [external table definition](https://docs.cloud.google.com/bigquery/external-table-definition#table-definition),
   that describes the aspects of the table to change:

   ```bash
   bq mkdef --connection_id=PROJECT_ID.REGION.CONNECTION_ID \
   --source_format=TABLE_FORMAT \
   --metadata_cache_mode=CACHE_MODE \
   "BUCKET_PATH" > /tmp/DEFINITION_FILE
   ```

   Replace the following:
   - `PROJECT_ID`: the name of the project that contains the connection
   - `REGION`: the region that contains the connection
   - `CONNECTION_ID`: the name of the connection to use
   - `TABLE_FORMAT`: the format used by the table. You can't change this when updating the table.
   - `CACHE_MODE`: specifies whether the metadata
     cache is refreshed automatically or manually. For more information
     on metadata caching considerations, see
     [Metadata caching for performance](https://docs.cloud.google.com/bigquery/docs/biglake-intro#metadata_caching_for_performance).

     Set to `AUTOMATIC` for the metadata cache to be refreshed at a
     system-defined interval, usually somewhere between 30 and
     60 minutes.

     Set to `MANUAL` if you want to refresh the metadata cache on a
     schedule you determine. In this case, you can call the
     [`BQ.REFRESH_EXTERNAL_METADATA_CACHE` system procedure](https://docs.cloud.google.com/bigquery/docs/reference/system-procedures#bqrefresh_external_metadata_cache) to refresh
     the cache.

     You must set `CACHE_MODE` if
     `STALENESS_INTERVAL` is set to a value
     greater than 0.
   - `BUCKET_PATH`: the path to the
     Cloud Storage bucket that contains the data for the
     external table, in the format
     `gs://bucket_name/[folder_name/]file_name`.

     You can limit the files selected from the bucket by specifying one asterisk (`*`)
     wildcard character in the path. For example, `gs://mybucket/file_name*`. For more
     information, see
     [Wildcard support for Cloud Storage URIs](https://docs.cloud.google.com/bigquery/docs/external-data-cloud-storage#wildcard-support).

     You can specify multiple buckets for the `uris` option by providing multiple
     paths.

     The following examples show valid `uris` values:
     - `gs://bucket/path1/myfile.csv`
     - `gs://bucket/path1/*.csv`
     - `gs://bucket/path1/*,gs://bucket/path2/file00*`

     When you specify `uris` values that target multiple files, all of those
     files must share a compatible schema.

     For more information about using Cloud Storage URIs in
     BigQuery, see
     [Cloud Storage resource path](https://docs.cloud.google.com/bigquery/docs/external-data-cloud-storage#google-cloud-storage-uri).
   - `DEFINITION_FILE`: the name of the table
     definition file that you are creating.

2. Update the table using the new external table definition:

   ```bash
   bq update --max_staleness=STALENESS_INTERVAL \
   --external_table_definition=/tmp/DEFINITION_FILE \
   PROJECT_ID:DATASET.EXTERNAL_TABLE_NAME
   ```

   Replace the following:
   - `STALENESS_INTERVAL`: specifies whether
     cached metadata is used by operations against the
     table, and how fresh the cached metadata must be in order for
     the operation to use it. For more information about metadata
     caching considerations, see
     [Metadata caching for performance](https://docs.cloud.google.com/bigquery/docs/biglake-intro#metadata_caching_for_performance).

     To disable metadata caching, specify 0. This is the default.

     To enable metadata caching, specify an interval value between 30
     minutes and 7 days, using the
     `Y-M D H:M:S` format described in the
     [`INTERVAL` data type](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#interval_type)
     documentation. For example, specify `0-0 0 4:0:0` for a 4
     hour staleness interval.
     With this value, operations against the table use cached metadata if
     it has been refreshed within the past 4 hours. If the cached metadata
     is older than that, the operation retrieves metadata from
     Cloud Storage instead.
   - `DEFINITION_FILE`: the name of the table
     definition file that you created or updated.

   - `PROJECT_ID`: the name of the project
     that contains the table

   - `DATASET`: the name of the dataset that
     contains the table

   - `EXTERNAL_TABLE_NAME`: the name of the table

## Query BigLake and external tables

After creating a BigLake table, you can
[query it using GoogleSQL syntax](https://docs.cloud.google.com/bigquery/docs/running-queries), the
same as if it were a standard BigQuery table.
For example, `SELECT field1, field2 FROM mydataset.my_cloud_storage_table;`.

## Limitations

- BigQuery only supports querying [Delta Lake reader](https://github.com/delta-io/delta/blob/master/PROTOCOL.md#protocol-evolution) v1
  tables.

- Hudi and BigQuery integration only
  works for hive-style partitioned `copy-on-write` tables.

## What's next

- Learn about [using SQL in BigQuery](https://docs.cloud.google.com/bigquery/docs/introduction-sql).
- Learn about [BigLake tables](https://docs.cloud.google.com/bigquery/docs/biglake-intro).
- Learn about [BigQuery quotas](https://docs.cloud.google.com/bigquery/quotas).