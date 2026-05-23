# Apache Iceberg managed tables

*Apache Iceberg managed tables* (formerly BigLake tables for
Apache Iceberg in BigQuery) provide the foundation for
building open-format lakehouses on Google Cloud.
Iceberg managed tables offer the same fully managed experience as
standard BigQuery tables, but store data in customer-owned storage
buckets. Iceberg managed tables support the open
Iceberg table format for better interoperability with
open-source and third-party compute engines on a single copy of data.

Iceberg managed tables support the following features:

- *Table mutations* using GoogleSQL data manipulation language (DML).
- *Unified batch and high throughput streaming* using the [BigQuery Storage Write API](https://docs.cloud.google.com/bigquery/docs/write-api) through connectors like Spark, Dataflow, and other engines.
- *Export of Iceberg V2 snapshot and automatic refresh* on each table mutation for direct query access with open-source and third-party query engines, such as Spark.
- *Schema evolution* , which lets you add, drop, and rename columns to suit your needs. This feature also lets you change an existing column's [data type](https://docs.cloud.google.com/bigquery/docs/managing-table-schemas#change_a_columns_data_type) and [mode](https://docs.cloud.google.com/bigquery/docs/managing-table-schemas#change_a_columns_mode). For more information, see [Conversion rules](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/conversion_rules).
- *Automatic storage optimization*, including adaptive file sizing, automatic clustering, garbage collection, and metadata optimization.
- [*Time travel*](https://docs.cloud.google.com/bigquery/docs/time-travel) for historical data access in BigQuery.
- [*Column-level security*](https://docs.cloud.google.com/bigquery/docs/column-level-security-intro) and [*data masking*](https://docs.cloud.google.com/bigquery/docs/column-data-masking-intro).
- [*Multi-statement transactions*](https://docs.cloud.google.com/bigquery/docs/biglake-iceberg-tables-in-bigquery#use_multi-statement_transactions) (in Preview).
- [*Table partitioning*](https://docs.cloud.google.com/bigquery/docs/biglake-iceberg-tables-in-bigquery#use_partitioning) (in Preview).
- [*Table creation in Dataform workflows*](https://docs.cloud.google.com/dataform/docs/create-tables#create-iceberg-table).

## Architecture

Iceberg managed tables bring the convenience of
BigQuery resource management to tables that reside in your own
cloud buckets. You can use BigQuery and open-source compute
engines on these tables without moving the data out of the buckets that you
control. You must configure a Cloud Storage bucket before you start using
Iceberg managed tables.

Using Iceberg managed tables has the following implications on your
bucket:

- BigQuery creates new data files in the bucket in response to write requests and background storage optimizations, such as DML statements and streaming.
- Automatic compaction and clustering are performed on the data files in the bucket. After the expiration of the [time travel window](https://docs.cloud.google.com/bigquery/docs/time-travel), data files are garbage collected. However, if the table is deleted, the associated data files aren't garbage collected. For more information, see [Storage optimization](https://docs.cloud.google.com/bigquery/docs/biglake-iceberg-tables-in-bigquery#storage_optimization).

Creating an Iceberg managed table is similar to [creating
BigQuery tables](https://docs.cloud.google.com/bigquery/docs/tables). Because it stores data
in open formats on Cloud Storage, you must do the following:

- Specify the [Cloud resource connection](https://docs.cloud.google.com/bigquery/docs/create-cloud-resource-connection) with `WITH CONNECTION` to configure the connection credentials for BigQuery to access Cloud Storage.
- Specify the file format of data storage as `PARQUET` with the `file_format =
  PARQUET` statement.
- Specify the open-source metadata table format as `ICEBERG` with the `table_format = ICEBERG` statement.

## Best practices

> [!WARNING]
> **Warning:** Modifying data files for Iceberg managed tables outside of BigQuery can cause query failure or data loss. Additionally, deleting the Cloud Storage bucket that contains your Iceberg managed tables or making those tables inaccessible to the connection service account can result in data loss. To prevent this, use BigQuery to update or modify Iceberg managed tables.

Directly changing or adding files to the bucket outside of
BigQuery can lead to data loss or unrecoverable errors. The
following table describes possible scenarios:

| **Operation** | **Consequences** | **Prevention** |
|---|---|---|
| Add new files to the bucket outside BigQuery. | **Data loss:** New files or objects added outside of BigQuery are not tracked by BigQuery. Untracked files are deleted by background garbage collection processes. | Add data exclusively through BigQuery. This lets BigQuery track the files and prevent them from being garbage collected. To prevent accidental additions and data loss, we also recommend restricting external tool write permissions on buckets containing Iceberg managed tables. |
| Create a new Iceberg managed table in a non-empty prefix. | **Data loss:** Extant data isn't tracked by BigQuery, so these files are considered untracked, and deleted by background garbage collection processes. | Only create new Iceberg managed tables in empty prefixes. |
| Modify or replace Iceberg managed table data files. | **Data loss:** On external modification or replacement, the table fails a consistency check and becomes unreadable. Queries against the table fail. There is no self-serve way to recover from this point. Contact [support](https://docs.cloud.google.com/bigquery/docs/getting-support) for data recovery assistance. | Modify data exclusively through BigQuery. This lets BigQuery track the files and prevent them from being garbage collected. To prevent accidental additions and data loss, we also recommend restricting external tool write permissions on buckets containing Iceberg managed tables. |
| Create two Iceberg managed tables on the same or overlapping URIs. | **Data loss:** BigQuery doesn't bridge identical URI instances of Iceberg managed tables. Background garbage collection processes for each table will consider the opposite table's files as untracked, and delete them, causing data loss. | Use unique URIs for each Iceberg managed table. |

### Cloud Storage bucket configuration best practices

The configuration of your Cloud Storage bucket and its connection with
BigQuery have a direct impact on the performance, cost, data
integrity, security, and governance of your Iceberg managed tables.
The following are best practices to help with this configuration:

- Select a name that clearly indicates that the bucket is only meant for
  Iceberg managed tables.

- Choose [single-region Cloud Storage buckets](https://docs.cloud.google.com/storage/docs/locations#available-locations)
  that are co-located in the same region as your BigQuery
  dataset. This coordination improves performance and lowers costs by avoiding
  data transfer charges.

- By default, Cloud Storage stores data in the Standard storage class,
  which provides sufficient performance. To optimize data storage costs, you
  can enable [Autoclass](https://docs.cloud.google.com/storage/docs/autoclass) to automatically manage
  [storage class](https://docs.cloud.google.com/storage/docs/storage-classes) transitions. Autoclass starts
  with the Standard storage class and moves objects that aren't accessed to
  progressively colder classes in order to reduce storage costs. When the
  object is read again, it's moved back to the Standard class.

- Enable [uniform bucket-level access](https://docs.cloud.google.com/storage/docs/uniform-bucket-level-access)
  and [public access prevention](https://docs.cloud.google.com/storage/docs/public-access-prevention).

- Verify that the [required roles](https://docs.cloud.google.com/bigquery/docs/biglake-iceberg-tables-in-bigquery#required-roles) are assigned to the
  correct users and service accounts.

- To prevent accidental data deletion or corruption in your Cloud Storage
  bucket, restrict write and delete permissions for most users in your
  organization. You can do this by setting a
  [bucket permission policy](https://docs.cloud.google.com/storage/docs/access-control/using-iam-permissions)
  with conditions that deny `PUT` and `DELETE` requests for all users, except
  those that you specify.

- Apply [google-managed](https://docs.cloud.google.com/storage/docs/encryption/default-keys) or
  [customer-managed](https://docs.cloud.google.com/storage/docs/encryption/customer-managed-keys)
  encryption keys for extra protection of sensitive data.

- Enable [audit logging](https://docs.cloud.google.com/storage/docs/audit-logging#settings) for operational
  transparency, troubleshooting, and monitoring data access.

- Keep the default [soft delete policy](https://docs.cloud.google.com/storage/docs/soft-delete) (7 day
  retention) to protect against accidental deletions. However, if you find
  that data has been deleted, engage with
  [support](https://docs.cloud.google.com/bigquery/docs/getting-support) rather than restoring objects
  manually, as objects that are added or modified outside of
  BigQuery aren't tracked by BigQuery metadata.

- Adaptive file sizing, automatic clustering, and garbage collection are
  enabled automatically and help with optimizing file performance and cost.

- Avoid the following Cloud Storage features, as they are unsupported for
  Iceberg managed tables:

  - [Hierarchical namespaces](https://docs.cloud.google.com/storage/docs/hns-overview)
  - [Object access control lists (ACLs)](https://docs.cloud.google.com/storage/docs/access-control/lists)
  - [Customer-supplied encryption keys](https://docs.cloud.google.com/storage/docs/encryption/customer-supplied-keys)
  - [Object versioning](https://docs.cloud.google.com/storage/docs/object-versioning)
  - [Object lock](https://docs.cloud.google.com/storage/docs/using-object-lock)
  - [Bucket lock](https://docs.cloud.google.com/storage/docs/bucket-lock)
  - Restoring soft-deleted objects with the BigQuery API or bq CLI

You can implement these best practices by creating your bucket with the
following command:

```bash
gcloud storage buckets create gs://BUCKET_NAME \
    --project=PROJECT_ID \
    --location=LOCATION \
    --enable-autoclass \
    --public-access-prevention \
    --uniform-bucket-level-access
```

Replace the following:

- <var translate="no">`BUCKET_NAME`</var>: the name for your new bucket
- <var translate="no">`PROJECT_ID`</var>: the ID of your project
- <var translate="no">`LOCATION`</var>: the [location](https://docs.cloud.google.com/storage/docs/locations) for your new bucket

## Iceberg managed table workflows

The following sections describe how to create, load, manage, and query
Iceberg managed tables.

### Before you begin

Before creating and using Iceberg managed tables, ensure that you have
set up a [Cloud resource
connection](https://docs.cloud.google.com/bigquery/docs/create-cloud-resource-connection) to a storage
bucket. Your connection needs write permissions on the storage bucket, as
specified in the following [Required roles](https://docs.cloud.google.com/bigquery/docs/biglake-iceberg-tables-in-bigquery#required-roles) section. For more
information about required roles and permissions for connections, see [Manage
connections](https://docs.cloud.google.com/bigquery/docs/working-with-connections).

### Required roles


To get the permissions that
you need to let BigQuery manage tables in your project,

ask your administrator to grant you the
following IAM roles:

- To create Iceberg managed tables:
  - [BigQuery Data Owner](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquery#bigquery.dataOwner) (`roles/bigquery.dataOwner`) on your project
  - [BigQuery Connection Admin](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquery#bigquery.connectionAdmin) (`roles/bigquery.connectionAdmin`) on your project
- To query Iceberg managed tables:
  - [BigQuery Data Viewer](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquery#bigquery.dataViewer) (`roles/bigquery.dataViewer`) on your project
  - [BigQuery User](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquery#bigquery.user) (`roles/bigquery.user`) on your project
- Grant the connection service account the following roles so it can read and write data in Cloud Storage:
  - [Storage Object User](https://docs.cloud.google.com/iam/docs/roles-permissions/storage#storage.objectUser) (`roles/storage.objectUser`) on the bucket
  - [Storage Legacy Bucket Reader](https://docs.cloud.google.com/iam/docs/roles-permissions/storage#storage.legacyBucketReader) (`roles/storage.legacyBucketReader`) on the bucket


For more information about granting roles, see [Manage access to projects, folders, and organizations](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).


These predefined roles contain

the permissions required to let BigQuery manage tables in your project. To see the exact permissions that are
required, expand the **Required permissions** section:


#### Required permissions

The following permissions are required to let BigQuery manage tables in your project:

- All:
  - `bigquery.connections.delegate` on your project
  - `bigquery.jobs.create` on your project
  - `bigquery.readsessions.create` on your project
  - `bigquery.tables.create` on your project
  - `bigquery.tables.get` on your project
  - `bigquery.tables.getData` on your project
  - `storage.buckets.get` on your bucket
  - `storage.objects.create` on your bucket
  - `storage.objects.delete` on your bucket
  - `storage.objects.get` on your bucket
  - `storage.objects.list` on your bucket


You might also be able to get
these permissions
with [custom roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles) or
other [predefined roles](https://docs.cloud.google.com/iam/docs/roles-overview#predefined).

### Create Iceberg managed tables

To create an Iceberg managed table, select one of the following
methods:

### SQL

```googlesql
CREATE TABLE [PROJECT_ID.]DATASET_ID.TABLE_NAME (
COLUMN DATA_TYPE[, ...]
)
CLUSTER BY CLUSTER_COLUMN_LIST
WITH CONNECTION {CONNECTION_NAME | DEFAULT}
OPTIONS (
file_format = 'PARQUET',
table_format = 'ICEBERG',
storage_uri = 'STORAGE_URI');
```

Replace the following:

- <var translate="no">PROJECT_ID</var>: the project containing the dataset. If undefined, the command assumes the default project.
- <var translate="no">DATASET_ID</var>: an existing dataset.
- <var translate="no">TABLE_NAME</var>: the name of the table you're creating.
- <var translate="no">DATA_TYPE</var>: the data type of the information that is contained in the column.
- <var translate="no">CLUSTER_COLUMN_LIST</var> (optional): a comma-separated list containing up to four columns. They must be top-level, non-repeated columns.
- <var translate="no">CONNECTION_NAME</var>: the name of the connection. For example, `myproject.us.myconnection`.

To use a [default connection](https://docs.cloud.google.com/bigquery/docs/default-connections), specify
`DEFAULT` instead of the connection string containing
<var translate="no">PROJECT_ID</var>.<var translate="no">REGION</var>.<var translate="no">CONNECTION_ID</var>.

- <var translate="no">STORAGE_URI</var>: a fully qualified [Cloud Storage
  URI](https://docs.cloud.google.com/bigquery/docs/batch-loading-data#gcs-uri). For example, `gs://mybucket/table`.

### bq

```bash
bq --project_id=PROJECT_ID mk \
    --table \
    --file_format=PARQUET \
    --table_format=ICEBERG \
    --connection_id=CONNECTION_NAME \
    --storage_uri=STORAGE_URI \
    --schema=COLUMN_NAME:DATA_TYPE[, ...] \
    --clustering_fields=CLUSTER_COLUMN_LIST \
    DATASET_ID.MANAGED_TABLE_NAME
```

Replace the following:

- <var translate="no">PROJECT_ID</var>: the project containing the dataset. If undefined, the command assumes the default project.
- <var translate="no">CONNECTION_NAME</var>: the name of the connection. For example, `myproject.us.myconnection`.
- <var translate="no">STORAGE_URI</var>: a fully qualified [Cloud Storage
  URI](https://docs.cloud.google.com/bigquery/docs/batch-loading-data#gcs-uri). For example, `gs://mybucket/table`.
- <var translate="no">COLUMN_NAME</var>: the column name.
- <var translate="no">DATA_TYPE</var>: the data type of the information contained in the column.
- <var translate="no">CLUSTER_COLUMN_LIST</var> (optional): a comma-separated list containing up to four columns. They must be top-level, non-repeated columns.
- <var translate="no">DATASET_ID</var>: the ID of an existing dataset.
- <var translate="no">MANAGED_TABLE_NAME</var>: the name of the table you're creating.

### API

Call the [`tables.insert`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables/insert)'
method with a defined [table
resource](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables), similar to the
following:

```json
{
"tableReference": {
  "tableId": "TABLE_NAME"
},
"biglakeConfiguration": {
  "connectionId": "CONNECTION_NAME",
  "fileFormat": "PARQUET",
  "tableFormat": "ICEBERG",
  "storageUri": "STORAGE_URI"
},
"schema": {
  "fields": [
    {
      "name": "COLUMN_NAME",
      "type": "DATA_TYPE"
    }
    [, ...]
  ]
}
}
```

Replace the following:

- <var translate="no">TABLE_NAME</var>: the name of the table that you're creating.
- <var translate="no">CONNECTION_NAME</var>: the name of the connection. For example, `myproject.us.myconnection`.
- <var translate="no">STORAGE_URI</var>: a fully qualified [Cloud Storage
  URI](https://docs.cloud.google.com/bigquery/docs/batch-loading-data#gcs-uri). [Wildcards](https://docs.cloud.google.com/bigquery/docs/batch-loading-data#load-wildcards) are also supported. For example, `gs://mybucket/table`.
- <var translate="no">COLUMN_NAME</var>: the column name.
- <var translate="no">DATA_TYPE</var>: the data type of the information contained in the column.

### Import data into Iceberg managed tables

The following sections describe how to import data from various table formats
into Iceberg managed tables.

#### Standard load data from flat files

Iceberg managed tables use BigQuery load jobs to load
external files into Iceberg managed tables. If you have an existing
Iceberg managed table, follow the [`bq load` CLI
guide](https://docs.cloud.google.com/bigquery/docs/hive-partitioned-loads-gcs#bq) or the [`LOAD` SQL
guide](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/other-statements#load_a_file_that_is_externally_partitioned)
to load external data. After loading the data, new Parquet files are written
into the <var translate="no">STORAGE_URI</var>`/data` folder.

If the prior instructions are used without an existing
Iceberg managed table, a BigQuery table is created
instead.

See the following for tool-specific examples of batch loads into
Iceberg managed tables:

### SQL

```googlesql
LOAD DATA INTO MANAGED_TABLE_NAME
FROM FILES (
uris=['STORAGE_URI'],
format='FILE_FORMAT');
```

Replace the following:

- <var translate="no">MANAGED_TABLE_NAME</var>: the name of an existing Iceberg managed table.
- <var translate="no">STORAGE_URI</var>: a fully qualified [Cloud Storage URI](https://docs.cloud.google.com/bigquery/docs/batch-loading-data#gcs-uri) or a comma-separated list of URIs. [Wildcards](https://docs.cloud.google.com/bigquery/docs/batch-loading-data#load-wildcards) are also supported. For example, `gs://mybucket/table`.
- <var translate="no">FILE_FORMAT</var>: the source table format. For supported formats, see the `format` row of [`load_option_list`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/load-statements#load_option_list).

### bq

```bash
bq load \
  --source_format=FILE_FORMAT \
  MANAGED_TABLE \
  STORAGE_URI
```

Replace the following:

- <var translate="no">FILE_FORMAT</var>: the source table format. For supported formats, see the `format` row of [`load_option_list`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/load-statements#load_option_list).
- <var translate="no">MANAGED_TABLE_NAME</var>: the name of an existing Iceberg managed table.
- <var translate="no">STORAGE_URI</var>: a fully qualified [Cloud Storage URI](https://docs.cloud.google.com/bigquery/docs/batch-loading-data#gcs-uri) or a comma-separated list of URIs. [Wildcards](https://docs.cloud.google.com/bigquery/docs/batch-loading-data#load-wildcards) are also supported. For example, `gs://mybucket/table`.

#### Standard load from Apache Hive-partitioned files

You can load Hive-partitioned files into Iceberg managed tables
using standard BigQuery load jobs. For more information, see
[Loading externally partitioned data](https://docs.cloud.google.com/bigquery/docs/hive-partitioned-loads-gcs).

#### Load streaming data from Pub/Sub

You can load streaming data into Iceberg managed tables by using a [Pub/Sub BigQuery
subscription](https://docs.cloud.google.com/pubsub/docs/subscription-properties#bigquery).

### Export data from Iceberg managed tables

The following sections describe how to export data from
Iceberg managed tables into various table formats.

#### Export data into flat formats

To export an Iceberg managed table into a flat format, use the
[`EXPORT DATA` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/other-statements#export_data_statement)
and select a destination format. For more information, see [Exporting
data](https://docs.cloud.google.com/bigquery/docs/exporting-data#sql).

### Create Iceberg managed table metadata snapshots

To create an Iceberg managed table metadata snapshot, follow these
steps:

1. Export the metadata into the Iceberg V2 format with the
   [`EXPORT TABLE
   METADATA`](https://docs.cloud.google.com/bigquery/docs/exporting-data#export_table_metadata) SQL
   statement.

2. Optional: Schedule Iceberg metadata snapshot refresh.
   To refresh an Iceberg metadata snapshot based on a set
   time interval, use a [scheduled query](https://docs.cloud.google.com/bigquery/docs/scheduling-queries).

3. Optional: Enable metadata auto-refresh for your project to automatically
   update your Iceberg table metadata snapshot on each
   table mutation. To enable metadata auto-refresh, contact
   [bigquery-tables-for-apache-iceberg-help@google.com](mailto:bigquery-tables-for-apache-iceberg-help@google.com).
   [`EXPORT METADATA` costs](https://docs.cloud.google.com/bigquery/docs/biglake-iceberg-tables-in-bigquery#queries_and_jobs) are applied on each refresh
   operation.

The following example creates a scheduled query named `My Scheduled Snapshot
Refresh Query` using the DDL statement `EXPORT TABLE METADATA FROM
mydataset.test`. The DDL statement runs every 24 hours.

```bash
bq query \
    --use_legacy_sql=false \
    --display_name='My Scheduled Snapshot Refresh Query' \
    --schedule='every 24 hours' \
    'EXPORT TABLE METADATA FROM mydataset.test'
```

### View Iceberg managed table metadata snapshot

After you refresh the Iceberg managed table metadata snapshot you
can find the snapshot in the [Cloud Storage
URI](https://docs.cloud.google.com/bigquery/docs/batch-loading-data#gcs-uri) that the
Iceberg managed table was originally created in. The `/data` folder
contains the Parquet file data shards, and the `/metadata` folder contains the
Iceberg managed table metadata snapshot.

```googlesql
SELECT
  table_name,
  REGEXP_EXTRACT(ddl, r"storage_uri\s*=\s*\"([^\"]+)\"") AS storage_uri
FROM
  `mydataset`.INFORMATION_SCHEMA.TABLES;
```

Note that `mydataset` and `table_name` are placeholders for your actual dataset
and table.

### Read Iceberg managed tables with Spark

The following sample sets up your environment to use Spark
SQL with Spark, and then executes a query to fetch data
from a specified Iceberg managed table.

```bash
spark-sql \
  --packages org.apache.iceberg:iceberg-spark-runtime-ICEBERG_VERSION_NUMBER \
  --conf spark.sql.catalog.CATALOG_NAME=org.apache.iceberg.spark.SparkCatalog \
  --conf spark.sql.catalog.CATALOG_NAME.type=hadoop \
  --conf spark.sql.catalog.CATALOG_NAME.warehouse='BUCKET_PATH' \

# Query the table
SELECT * FROM CATALOG_NAME.FOLDER_NAME;
```

Replace the following:

- <var translate="no">ICEBERG_VERSION_NUMBER</var>: the current runtime version. Download the latest version from [Iceberg releases](https://iceberg.apache.org/releases/).
- <var translate="no">CATALOG_NAME</var>: the catalog to reference your Iceberg managed table.
- <var translate="no">BUCKET_PATH</var>: the path to the bucket containing the table files. For example, `gs://mybucket/`.
- <var translate="no">FOLDER_NAME</var>: the folder containing the table files. For example, `myfolder`.

### Modify Iceberg managed tables

To modify an Iceberg managed table, follow the steps shown in
[Modifying table schemas](https://docs.cloud.google.com/bigquery/docs/managing-table-schemas).

### Use multi-statement transactions

> [!WARNING]
>
> **Preview**
>
>
> This product or feature is
>
> subject to the "Pre-GA Offerings Terms" in the General Service Terms section
> of the [Service Specific
> Terms](https://docs.cloud.google.com/terms/service-terms#1).
>
> Pre-GA products and features are available "as is" and might have limited support.
>
> For more information, see the
> [launch stage descriptions](https://cloud.google.com/products/#product-launch-stages).

> [!NOTE]
> **Note:** To provide feedback or ask questions that are related to this Preview feature, contact [biglake-help@google.com](mailto:biglake-help@google.com).

To gain access to [multi-statement transactions](https://docs.cloud.google.com/bigquery/docs/transactions)
for Iceberg managed tables, fill out the [sign-up
form](https://docs.google.com/forms/d/1lQMsrT_jj_bi_aJbcb65dOc8LJTf0Wjb9AZs9EQXkCg).

### Use partitioning

> [!WARNING]
>
> **Preview**
>
>
> This product or feature is
>
> subject to the "Pre-GA Offerings Terms" in the General Service Terms section
> of the [Service Specific
> Terms](https://docs.cloud.google.com/terms/service-terms#1).
>
> Pre-GA products and features are available "as is" and might have limited support.
>
> For more information, see the
> [launch stage descriptions](https://cloud.google.com/products/#product-launch-stages).

> [!NOTE]
> **Note:** To provide feedback or ask questions that are related to this Preview feature, contact [biglake-help@google.com](mailto:biglake-help@google.com).

To gain access to [partitioning](https://docs.cloud.google.com/bigquery/docs/partitioned-tables) for
Iceberg managed tables, fill out the
[sign-up form](https://forms.gle/AJTG3idhjZ6RLLV98).

You partition a table by specifying a partition column, which is used to segment
the table. The following column types are supported for
Iceberg managed tables:

- `DATE`
- `DATETIME`
- `TIMESTAMP`

Partitioning a table on a `DATE`, `DATETIME`, or `TIMESTAMP` column is known as
[time-unit column
partitioning](https://docs.cloud.google.com/bigquery/docs/partitioned-tables#date_timestamp_partitioned_tables).
You choose whether the partitions have [hourly, daily, monthly, or yearly
granularity](https://docs.cloud.google.com/bigquery/docs/partitioned-tables#select_daily_hourly_monthly_or_yearly_partitioning).

Iceberg managed tables also support
[clustering](https://docs.cloud.google.com/bigquery/docs/clustered-tables) and [combining clustered and
partitioned
tables](https://docs.cloud.google.com/bigquery/docs/clustered-tables#combine-clustered-partitioned-tables).

#### Partitioning limitations

- All [BigQuery partitioned table limitations](https://docs.cloud.google.com/bigquery/docs/partitioned-tables#limitations) apply.
- Partitioning column types other than `DATE`, `DATETIME`, or `TIMESTAMP` aren't supported.
- Partition expiration isn't supported.
- [Partition evolution](https://iceberg.apache.org/docs/1.5.1/evolution/#partition-evolution) isn't supported.

#### Create a partitioned Iceberg managed table

To create a partitioned Iceberg managed table, follow the
instructions to
[create a standard Iceberg managed table](https://docs.cloud.google.com/bigquery/docs/biglake-iceberg-tables-in-bigquery#create-iceberg-tables), and
include one of the following, depending on your environment:

- The [`PARTITION BY` clause](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#partition_expression)
- The [`--time_partitioning_field` and `--time_partitioning_type` flags](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#mk-table)
- The [`timePartitioning` property](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables#timepartitioning)

#### Modify and query partitioned Iceberg managed tables

BigQuery data manipulation language (DML) statements and queries
for partitioned Iceberg managed tables are the same as for standard
Iceberg managed tables. BigQuery automatically scopes
the job to the right partitions, similar to
[Iceberg hidden partitioning](https://iceberg.apache.org/docs/latest/partitioning/#icebergs-hidden-partitioning).
Additionally, any new data that you add to the table is automatically
partitioned.

You can also query partitioned Iceberg managed tables with other
engines in the same way as standard Iceberg managed tables. We
recommend [enabling metadata
snapshots](https://docs.cloud.google.com/bigquery/docs/iceberg-tables#create-iceberg-table-snapshots) for the
best experience.

For enhanced security, partitioning information for
Iceberg managed tables is decoupled from the data path and is managed
entirely by the metadata layer.

## Pricing

Iceberg managed table pricing consists of storage, storage
optimization, and queries and jobs.

### Storage

Iceberg managed tables store all data in
[Cloud Storage](https://docs.cloud.google.com/storage). You are charged for all data stored, including
historical table data. Cloud Storage [data
processing](https://cloud.google.com/storage/pricing#process-pricing) and
[transfer charges](https://cloud.google.com/storage/pricing#network-buckets)
might also apply. Some Cloud Storage operation fees might be waived for
operations that are processed through BigQuery or the
BigQuery Storage API. There are no BigQuery-specific storage fees.
For more information, see [Cloud Storage
Pricing](https://cloud.google.com/storage/pricing).

### Storage optimization

Iceberg managed tables perform automatic table management, including
compaction, clustering, garbage collection, and BigQuery metadata
generation/refresh to optimize query performance and reduce storage costs.
Compute resource usage for table management is billed in Data Compute Units
(DCUs) over time, in per second increments. For more details, see
[Iceberg managed table pricing](https://cloud.google.com/products/biglake/pricing).

Data export operations taking place while streaming through the Storage Write
API are included in Storage Write API pricing and are not charged as background
maintenance. For more information, see [Data ingestion
pricing](https://cloud.google.com/bigquery/pricing#data_ingestion_pricing).

To view the logs and compute usage for these background operations, query the
[`INFORMATION_SCHEMA.JOBS`](https://docs.cloud.google.com/bigquery/docs/information-schema-jobs) view. For
example queries, see the following:

- [Storage optimization jobs](https://docs.cloud.google.com/bigquery/docs/information-schema-jobs#get-iceberg-storage-optimization-jobs)
- [`EXPORT TABLE METADATA` jobs](https://docs.cloud.google.com/bigquery/docs/information-schema-jobs#get-iceberg-export-table-metadata-jobs)

### Queries and jobs

Similar to BigQuery tables, you are charged for queries and bytes
read (per TiB) if you are using [BigQuery on-demand
pricing](https://cloud.google.com/bigquery/pricing#on_demand_pricing), or slot
consumption (per slot hour) if you are using [BigQuery capacity
compute
pricing](https://cloud.google.com/bigquery/pricing#capacity_compute_analysis_pricing).

BigQuery pricing also applies to the
[BigQuery Storage Read API](https://cloud.google.com/bigquery/pricing#data_extraction_pricing)
and the [Storage Write
API](https://cloud.google.com/bigquery/pricing#data_ingestion_pricing).

Load and export operations (such as `EXPORT METADATA`) use [Enterprise edition
pay as you go
slots](https://cloud.google.com/bigquery/pricing#enterprise-edition-slots). This
differs from BigQuery tables, which are not charged for these
operations. If `PIPELINE` reservations with Enterprise or Enterprise Plus slots
are available, load and export operations preferentially use these reservation
slots instead.

## Limitations

Iceberg managed tables have the following limitations:

- Iceberg managed tables don't support [renaming operations](https://docs.cloud.google.com/bigquery/docs/managing-tables#renaming-table) or [`ALTER TABLE RENAME TO` statements](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#alter_table_rename_to_statement).
- Iceberg managed tables don't support [table copies](https://docs.cloud.google.com/bigquery/docs/managing-tables#copy-table) or [`CREATE TABLE COPY` statements](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_table_copy).
- Iceberg managed tables don't support [table clones](https://docs.cloud.google.com/bigquery/docs/table-clones-intro) or [`CREATE TABLE CLONE` statements](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_table_clone_statement).
- Iceberg managed tables don't support [table snapshots](https://docs.cloud.google.com/bigquery/docs/table-snapshots-intro) or [`CREATE SNAPSHOT TABLE` statements](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_snapshot_table_statement).
- Iceberg managed tables don't support the following table schema:
  - Empty schema
  - Schema with `BIGNUMERIC`, `INTERVAL`, `JSON`, `RANGE`, or `GEOGRAPHY` data types.
  - Schema with [field collations](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#collatable_data_types).
  - Schema with [default value expressions](https://docs.cloud.google.com/bigquery/docs/default-values).
- Iceberg managed tables don't support the following schema evolution cases:
  - `NUMERIC` to `FLOAT` type coercions
  - `INT` to `FLOAT` type coercions
  - Adding new nested fields to an existing `RECORD` columns using SQL DDL statements
- Iceberg managed tables display a 0-byte storage size when queried by the console or APIs.
- Iceberg managed tables don't support [materialized
  views](https://docs.cloud.google.com/bigquery/docs/materialized-views-intro).
- Iceberg managed tables don't support [authorized
  views](https://docs.cloud.google.com/bigquery/docs/authorized-views), but [column-level access
  control](https://docs.cloud.google.com/bigquery/docs/column-level-security-intro) is supported.
- Iceberg managed tables don't support [change data capture (CDC)](https://docs.cloud.google.com/bigquery/docs/change-data-capture) updates.
- Iceberg managed tables don't support [managed disaster recovery](https://docs.cloud.google.com/bigquery/docs/managed-disaster-recovery)
- Iceberg managed tables don't support [row-level
  security](https://docs.cloud.google.com/bigquery/docs/row-level-security-intro).
- Iceberg managed tables don't support [fail-safe
  windows](https://docs.cloud.google.com/bigquery/docs/time-travel#fail-safe).
- Iceberg managed tables don't support extract jobs.
- The `INFORMATION_SCHEMA.TABLE_STORAGE` view doesn't include Iceberg managed tables.
- Iceberg managed tables aren't supported as query result destinations. You can instead use the [`CREATE
  TABLE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_table_statement) statement with the `AS query_statement` argument to create a table as the query result destination.
- `CREATE OR REPLACE` doesn't support replacing standard tables with Iceberg managed tables, or Iceberg managed tables with standard tables.
- [Batch loading](https://docs.cloud.google.com/bigquery/docs/batch-loading-data) and [`LOAD DATA`
  statements](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/load-statements) only support appending data to existing Iceberg managed tables.
- [Batch loading](https://docs.cloud.google.com/bigquery/docs/batch-loading-data) and [`LOAD
  DATA` statements](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/load-statements) don't support schema updates.
- `TRUNCATE TABLE` doesn't support Iceberg managed tables. There are two alternatives:
  - [`CREATE OR REPLACE
    TABLE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_table_statement), using the same table creation options.
  - `DELETE FROM` table `WHERE` true
- The [`APPENDS` table-valued function (TVF)](https://docs.cloud.google.com/bigquery/docs/change-history) doesn't support Iceberg managed tables.
- Iceberg metadata might not contain data that was streamed to BigQuery by the Storage Write API within the last 90 minutes.
- Record-based paginated access using `tabledata.list` doesn't support Iceberg managed tables.
- Only one concurrent mutating DML statement (`UPDATE`, `DELETE`, and `MERGE`) runs for each Iceberg managed table. Additional mutating DML statements are queued.