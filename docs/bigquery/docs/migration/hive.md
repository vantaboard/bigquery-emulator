# Migrate schema and data from Apache Hive

This document describes how to migrate your data, security settings, and
pipelines from Apache Hive to BigQuery.

You can also use
[batch SQL translation](https://docs.cloud.google.com/bigquery/docs/batch-sql-translator)
to migrate your SQL scripts in bulk, or
[interactive SQL translation](https://docs.cloud.google.com/bigquery/docs/interactive-sql-translator)
to translate ad hoc queries. Apache HiveQL is fully supported by both
SQL translation services.

## Prepare for migration

The following sections describe how to collect information about table
statistics, metadata, and security settings to help you
migrate your data warehouse from Apache Hive to BigQuery.

### Collect source table information

Gather information about source Hive tables such as their number of rows,
number of columns, column data types, size, input format of the
data, and location. This information is useful in the
migration process and also to validate the data migration. If you have a Hive
table named `employees` in a database named `corp`, use the following commands
to collect table information:

```bash
# Find the number of rows in the table
hive> SELECT COUNT(*) FROM corp.employees;

# Output all the columns and their data types
hive> DESCRIBE corp.employees;

# Output the input format and location of the table
hive> SHOW CREATE TABLE corp.employees;
Output:
…
STORED AS INPUTFORMAT
  'org.apache.hadoop.hive.ql.io.avro.AvroContainerInputFormat'
OUTPUTFORMAT
  'org.apache.hadoop.hive.ql.io.avro.AvroContainerOutputFormat'
LOCATION
  'hdfs://demo_cluster/user/hive/warehouse/corp/employees'
TBLPROPERTIES (
…

# Get the total size of the table data in bytes
shell> hdfs dfs -du -s TABLE_LOCATION
```

### Source table format conversion

Some of the formats that Hive supports cannot be ingested into
BigQuery directly.

Hive supports storing data in the following formats:

- Text file
- RC file
- Sequence file
- Avro file
- ORC file
- Parquet file

BigQuery supports loading data from Cloud Storage in
any of the following file formats:

- CSV
- JSON (Newline delimited)
- Avro
- ORC
- Parquet

BigQuery can load data files in Avro, ORC, and Parquet formats
directly without the need of schema files. For text files that are not formatted
as CSV or JSON (Newline delimited), you can either copy the data to a Hive table
in Avro format, or you can convert the table schema to a BigQuery
[JSON schema](https://docs.cloud.google.com/bigquery/docs/schemas)
to provide when ingesting.

### Collect Hive access control settings

Hive and BigQuery have different access control mechanisms.
Collect all the Hive access control settings such as roles, groups, members, and
privileges granted to them. Map out a security model in BigQuery
on a per-dataset level and implement a fine-grained ACL. For example, a Hive
user can be mapped to a
[Google account](https://docs.cloud.google.com/iam/docs/principals-overview#google-account)
and an HDFS group can be mapped to a
[Google group](https://docs.cloud.google.com/iam/docs/overview#google_group). Access can be set on the
dataset level. Use the following commands to collect access control settings in
Hive:

```bash
# List all the users
> hdfs dfs -ls /user/ | cut -d/ -f3

# Show all the groups that a specific user belongs to
> hdfs groups user_name

# List all the roles
hive> SHOW ROLES;

# Show all the roles assigned to a specific group
hive> SHOW ROLE GRANT GROUP group_name

# Show all the grants for a specific role
hive> SHOW GRANT ROLE role_name;

# Show all the grants for a specific role on a specific object
hive> SHOW GRANT ROLE role_name on object_type object_name;
```

In Hive, you may access the HDFS files behind the tables directly if you have
the required permissions. In standard BigQuery tables, after the
data is loaded into the table, the data gets stored in the
BigQuery storage. You can read data by using the
BigQuery Storage Read API but all IAM, row-, and column-level
security is still enforced. If you are using BigQuery
external tables to query the data in Cloud Storage, access to
Cloud Storage is also controlled by IAM.

You can
create a [BigLake table](https://docs.cloud.google.com/bigquery/docs/biglake-quickstart) that
lets you use
[connectors](https://docs.cloud.google.com/bigquery/docs/biglake-quickstart#query-biglake-table-using-connectors)
to query the data with Apache Spark, Trino, or Hive. The
BigQuery Storage API enforces row- and column-level governance policies for all
BigLake tables in Cloud Storage or
BigQuery.

## Data migration

Migrating Hive data from your on-premises or other cloud-based source cluster to
BigQuery has two steps:

1. Copying data from a source cluster to Cloud Storage
2. Loading data from Cloud Storage into BigQuery

The following sections cover migrating Hive data, validating migrated data, and
handling migration of continuously ingested data. The examples are written for
non-ACID tables.

### Partition column data

In Hive, data in partitioned tables is stored in a directory structure.
Each partition of the table is associated with a particular value of
partition column. The data files themselves don't contain any data of the
partition columns. Use the `SHOW PARTITIONS` command to list the different
partitions in a partitioned table.

The following example shows that the source Hive table is partitioned on the columns
`joining_date` and `department`. The data files under this table don't
contain any data related to these two columns.

```bash
hive> SHOW PARTITIONS corp.employees_partitioned
joining_date="2018-10-01"/department="HR"
joining_date="2018-10-01"/department="Analyst"
joining_date="2018-11-01"/department="HR"
```

One way to copy these columns is to convert the partitioned table
into a non-partitioned table before loading into BigQuery:

1. Create a non-partitioned table with schema similar to the partitioned table.
2. Load data into the non-partitioned table from the source partitioned table.
3. Copy these data files under the staged non-partitioned table to Cloud Storage.
4. Load the data into BigQuery with the `bq load` command and provide the name of the `TIMESTAMP` or `DATE` type partition column, if any, as the `time_partitioning_field` argument.

### Copy data to Cloud Storage

The first step in data migration is to copy the data to Cloud Storage.
Use
[Hadoop DistCp](https://hadoop.apache.org/docs/current/hadoop-distcp/DistCp.html)
to copy data from your on-premises or other-cloud cluster to
Cloud Storage. Store your data in a bucket in the same region or
multi-region as the dataset where you want to store the data in
BigQuery.
For example, if you want to use an existing BigQuery dataset as
the destination which is in the Tokyo region, you must choose a
Cloud Storage regional bucket in Tokyo to hold the data.

After selecting the Cloud Storage bucket location, you can use the
following command to list out all the data files present at the `employees` Hive
table location:

```bash
> hdfs dfs -ls hdfs://demo_cluster/user/hive/warehouse/corp/employees
hdfs://demo_cluster/user/hive/warehouse/corp/employees/000000_0
hdfs://demo_cluster/user/hive/warehouse/corp/employees/000001_0
hdfs://demo_cluster/user/hive/warehouse/corp/employees/000002_0
```

Copy all the preceding files to Cloud Storage:

```bash
> hadoop distcp
hdfs://demo_cluster/user/hive/warehouse/corp/employees
gs://hive_data/corp/employees
```

Note that you are charged for storing the data in Cloud Storage
according to the
[Data storage pricing](https://cloud.google.com/storage/pricing#storage-pricing).

There might be staging directories that hold intermediate files created for
query jobs. You must ensure that you delete any such directories before running
the `bq load` command.

### Loading data

BigQuery supports
[batch loading data](https://docs.cloud.google.com/bigquery/docs/batch-loading-data) in many formats from
Cloud Storage. Ensure that the BigQuery
[dataset](https://docs.cloud.google.com/bigquery/docs/datasets) you want to load your data into exists
prior to creating a load job.

The following command shows the data copied from Hive for a non-ACID table:

```bash
> gcloud storage ls gs://hive_data/corp/employees/
gs://hive-migration/corp/employees/
gs://hive-migration/corp/employees/000000_0
gs://hive-migration/corp/employees/000001_0
gs://hive-migration/corp/employees/000002_0
```

To load your Hive data into BigQuery, use the
[`bq load` command](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#bq_load).
You can use a wildcard character \* in the URL to load data from multiple
files that share a common object prefix. For example, use the following command
to load all the files sharing the prefix `gs://hive_data/corp/employees/`:

```bash
bq load --source_format=AVRO corp.employees gs://hive_data/corp/employees/*
```

Because jobs can take a long time to complete, you can execute them
asynchronously by setting the `--sync` flag to `False`. Running the `bq load`
command outputs the job ID of the created load job, so you can use this command
to poll the job status.
This data includes details such as the job type, the job state, and
the user who ran the job.

Poll each load job status using its respective job ID and check for any job
that has failed with errors. In case of failure, BigQuery uses an
"All or None" approach while loading data into a table. You can try resolving
the errors and safely re-create another load job. For more information, see
[troubleshooting errors](https://docs.cloud.google.com/bigquery/troubleshooting-errors).

Ensure you have enough load job
[quota](https://docs.cloud.google.com/bigquery/quotas#load_jobs)
per table and project. If you exceed your quota, then the load job fails with a
`quotaExceeded` error.

Note that you are not charged for a load operation to load data into
BigQuery from Cloud Storage. Once the data is loaded
into BigQuery, it is subject to BigQuery's
[storage pricing](https://cloud.google.com/bigquery/pricing#storage).
When the load jobs are finished successfully, you can
delete any remaining files in Cloud Storage to avoid incurring charges
for storing redundant data.

### Validation

After loading data successfully, you can validate your migrated data by
comparing the [number of rows in the Hive](https://docs.cloud.google.com/bigquery/docs/migration/hive#collect_source_table_information)
and BigQuery tables. View the
[table information](https://docs.cloud.google.com/bigquery/docs/tables#get_information_about_tables)
to get details about BigQuery tables such as the number of rows,
number of columns, partitioning fields, or clustering fields. For additional
validation, consider trying the
[Data validation tool](https://github.com/GoogleCloudPlatform/professional-services-data-validator).

### Continuous ingestion

If you continuously ingest data into a Hive table, perform an initial
migration and then migrate only the incremental data changes to
BigQuery. It is common to create scripts that run repeatedly
to find and load new data. There are many ways to do this, and the following
sections describe one possible approach.

You can keep track of the migration progress in a
[Cloud SQL](https://docs.cloud.google.com/sql/docs/mysql)
database table, which is referred to as a tracking table in the following
sections. During the first run of migration, store the progress in the
tracking table.
For the subsequent runs of migration, use the tracking table information to
detect if any additional data has been ingested and can be migrated to
BigQuery.

Select an `INT64`, `TIMESTAMP`, or `DATE` type identifier column to distinguish
the incremental data. This is referred to as an incremental column.

The following table is an example of a table with no partitioning that uses a
`TIMESTAMP` type for its incremental column:

```
+---+---+---+---+---+
| timestamp_identifier        | column_2  | column_3  | column_4  | column_5  |
+---+---+---+---+---+
| 2018-10-10 21\:56\:41       |           |           |           |           |
| 2018-10-11 03\:13\:25       |           |           |           |           |
| 2018-10-11 08\:25\:32       |           |           |           |           |
| 2018-10-12 05\:02\:16       |           |           |           |           |
| 2018-10-12 15\:21\:45       |           |           |           |           |
+---+---+---+---+---+
```

The following table is an example of a table partitioned on a `DATE` type column
`partition_column`. It has an integer type incremental column `int_identifier`
in each partition.

```
+---+---+---+---+---+
| partition_column    | int_identifier      | column_3 | column_4 | column_5  |
+---+---+---+---+---+
| 2018-10-01          | 1                   |          |          |           |
| 2018-10-01          | 2                   |          |          |           |
| ...                 | ...                 |          |          |           |
| 2018-10-01          | 1000                |          |          |           |
| 2018-11-01          | 1                   |          |          |           |
| 2018-11-01          | 2                   |          |          |           |
| ...                 | ...                 |          |          |           |
| 2018-11-01          | 2000                |          |          |           |
+---+---+---+---+---+
```

The following sections describe migrating Hive data based on whether or not it
is partitioned and whether or not it has incremental columns.

#### Non-partitioned table without incremental columns

Assuming there are no file compactions in Hive, Hive creates new data files when
ingesting new data. During the first run, store the list of files in the
tracking table and complete the initial migration of the Hive table by copying
these files to Cloud Storage and loading them into
BigQuery.

```bash
> hdfs dfs -ls hdfs://demo_cluster/user/hive/warehouse/corp/employees
Found 3 items
hdfs://demo_cluster/user/hive/warehouse/corp/employees/000000_0
hdfs://demo_cluster/user/hive/warehouse/corp/employees/000001_0
hdfs://demo_cluster/user/hive/warehouse/corp/employees/000002_0
```

After the initial migration, some data is ingested in Hive. You only need to
migrate this incremental data to BigQuery. In the subsequent
migration runs, list out the data files again and compare them with the
information from the tracking table to detect new data files that haven't been
migrated.

```bash
> hdfs dfs -ls hdfs://demo_cluster/user/hive/warehouse/corp/employees
Found 5 items
hdfs://demo_cluster/user/hive/warehouse/corp/employees/000000_0
hdfs://demo_cluster/user/hive/warehouse/corp/employees/000001_0
hdfs://demo_cluster/user/hive/warehouse/corp/employees/000002_0
hdfs://demo_cluster/user/hive/warehouse/corp/employees/000003_0
hdfs://demo_cluster/user/hive/warehouse/corp/employees/000004_0
```

In this example, two new files are present at the table
location. Migrate the data by copying these new data files to
Cloud Storage and loading them into the existing
BigQuery table.

#### Non-partitioned table with incremental columns

In this case, you can use the maximum value of incremental columns to determine
if any new data was added. While performing the initial migration, query the Hive table to fetch the maximum value of the incremental column and store it in
the tracking table:

```bash
hive> SELECT MAX(timestamp_identifier) FROM corp.employees;
2018-12-31 22:15:04
```

In the subsequent runs of migration, repeat the same query again to fetch the
present maximum value of the incremental column and compare it with the previous
maximum value from the tracking table to check if incremental data exists:

```bash
hive> SELECT MAX(timestamp_identifier) FROM corp.employees;
2019-01-04 07:21:16
```

If the present maximum value is greater than the previous maximum value, it
indicates that incremental data has been ingested into the Hive table as in the
example. To migrate the incremental data, create a staging table and load only
the incremental data into it.

```bash
hive> CREATE TABLE stage_employees LIKE corp.employees;
hive> INSERT INTO TABLE stage_employees SELECT * FROM corp.employees WHERE timestamp_identifier>"2018-12-31 22:15:04" and timestamp_identifier<="2019-01-04 07:21:16"
```

Migrate the staging table by listing out the HDFS data files, copying them to
Cloud Storage, and loading them into the existing
BigQuery table.

#### Partitioned table without incremental columns

Ingestion of data into a partitioned table might create new partitions, append
incremental data to existing partitions, or do both. In this scenario, you
can identify those updated partitions but cannot easily identify what data
has been added to these existing partitions since there is no incremental column
to distinguish. Another option is to take and maintain HDFS snapshots, but
snapshotting creates performance concerns for Hive so it is generally
disabled.

While migrating the table for the first time, run the `SHOW PARTITIONS` command
and store the information about the different partitions in the tracking
table.

```bash
hive> SHOW PARTITIONS corp.employees
partition_column=2018-10-01
partition_column=2018-11-01
```

The following output shows that the table `employees` has two partitions. A
simplified version of the tracking table is provided in the following table to show how this
information can be stored.

| **partition_information** | **file_path** | **gcs_copy_status** | **gcs_file_path** | **bq_job_id** | **...** |
|---|---|---|---|---|---|
| partition_column =2018-10-01 |   |   |   |   |   |
| partition_column =2018-11-01 |   |   |   |   |   |

In the subsequent migration runs, run the `SHOW PARTITIONS` command again to
list all the partitions and compare these with the partition information from
the tracking table to check if any new partitions are present which haven't been
migrated.

```bash
hive> SHOW PARTITIONS corp.employees
partition_column=2018-10-01
partition_column=2018-11-01
partition_column=2018-12-01
partition_column=2019-01-01
```

If any new partitions are identified as in the example, create a staging table
and load only the new partitions into it from the source table. Migrate the
staging table by copying the files to Cloud Storage and loading them
into the existing BigQuery table.

#### Partitioned table with incremental columns

In this scenario, the Hive table is partitioned and an incremental column is
present in every partition. Continuously ingested data increments upon this
column value. Here you have the ability to migrate the new partitions as described
in the previous section and you can also migrate incremental data that has been ingested into the
existing partitions.

When migrating the table for the first time, store the minimum and maximum
values of the incremental column in each partition along with the information
about the table partitions in the tracking table.

```bash
hive> SHOW PARTITIONS corp.employees
partition_column=2018-10-01
partition_column=2018-11-01

hive> SELECT MIN(int_identifier),MAX(int_identifier) FROM corp.employees WHERE partition_column="2018-10-01";
1 1000

hive> SELECT MIN(int_identifier),MAX(int_identifier) FROM corp.employees WHERE partition_column="2018-11-01";
1 2000
```

The following output shows that the table employees has two partitions and the
minimum and maximum values of the incremental column in each partition. A
simplified version of the tracking table is provided in the following table to show how this
information can be stored.

| **partition_information** | **inc_col_min** | **inc_col_max** | **file_path** | **gcs_copy_status** | **...** |
|---|---|---|---|---|---|
| partition_column =2018-10-01 | 1 | 1000 |   |   |   |
| partition_column =2018-11-01 | 1 | 2000 |   |   |   |

In the subsequent runs, run the same queries to fetch the present maximum value
in each partition and compare it with the previous maximum value from the
tracking table.

```bash
hive> SHOW PARTITIONS corp.employees
partition_column=2018-10-01
partition_column=2018-11-01
partition_column=2018-12-01
partition_column=2019-01-01

hive> SELECT MIN(int_identifier),MAX(int_identifier) FROM corp.employees WHERE partition_column="2018-10-01";
```

In the example, two new partitions have been identified and some incremental
data has been ingested in the existing partition `partition_column=2018-10-01`.
If there is any incremental data, create a staging table, load only the
incremental data into the staging table, copy the data to
Cloud Storage, and load the data into the existing
BigQuery table.

## Security settings

BigQuery uses IAM to manage access to resources.
BigQuery
[predefined roles](https://docs.cloud.google.com/iam/docs/roles-overview#predefined)
provide granular access for a specific service and are meant to support common
use cases and access control patterns. You can use
[custom roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles)
to provide even more fine-grained access by customizing a set of permissions.

Access controls on [tables](https://docs.cloud.google.com/bigquery/docs/table-access-controls) and
datasets specify the operations that users, groups, and
service accounts are allowed to perform on tables, views, and datasets.
[Authorized views](https://docs.cloud.google.com/bigquery/docs/authorized-views) lets you share query results
with particular users and groups without giving them access to the underlying
source data. With
[row-level security](https://docs.cloud.google.com/bigquery/docs/managing-row-level-security)
and [column-level security](https://docs.cloud.google.com/bigquery/docs/column-level-security-intro),
you can restrict who can access which rows or columns within a table.
[Data masking](https://docs.cloud.google.com/bigquery/docs/column-data-masking-intro) lets you selectively
obscure column data for groups of users, while still allowing access to the
column.

When you apply access controls, you can grant access to the
following users and groups:

- User by email: gives an individual Google Account access to the dataset
- Group by email: gives all members of a Google group access to the dataset
- Domain: gives all users and groups in a [Google domain](https://support.google.com/a/answer/53295) access to the dataset
- All Authenticated Users: gives all Google Account holders access to the dataset (makes the dataset public)
- Project Owners: gives all project owners access to the dataset
- Project Viewers: gives all project viewers access to the dataset
- Project Editors: gives all project editors access to the dataset
- Authorized View: gives a view access to the dataset

## Data pipeline changes

The following sections discuss how to change your data pipelines when you
migrate from Hive to BigQuery.

### Sqoop

If your existing pipeline uses Sqoop to import data into HDFS or Hive for
processing, modify the job to import data into Cloud Storage.

If you are importing data into HDFS, choose one of the following:

- Copy the Sqoop output files to Cloud Storage using [Hadoop DistCp](https://hadoop.apache.org/docs/current/hadoop-distcp/DistCp.html).
- Output the files to Cloud Storage directly using the [Cloud Storage connector](https://docs.cloud.google.com/dataproc/docs/concepts/connectors/cloud-storage). The [Cloud Storage](https://docs.cloud.google.com/storage) connector is an [open source Java library](https://github.com/GoogleCloudPlatform/bigdata-interop/tree/master/gcs) that lets you run [Apache Hadoop](https://hadoop.apache.org/) or [Apache Spark](https://spark.apache.org/) jobs directly on data in Cloud Storage. For more information, see [Installing the Cloud Storage connector](https://docs.cloud.google.com/dataproc/docs/concepts/connectors/install-storage-connector).

If you want Sqoop to import data into Hive running on
Google Cloud, point it to the Hive table directly and use
Cloud Storage as the Hive warehouse instead of HDFS. To do this, set
the property `hive.metastore.warehouse.dir` to a Cloud Storage bucket.

You can run your Sqoop job without managing a Hadoop cluster by
using Managed Service for Apache Spark to submit Sqoop jobs to import data into
BigQuery.

### Apache Spark SQL and HiveQL

The [batch SQL translator](https://docs.cloud.google.com/bigquery/docs/batch-sql-translator) or
[interactive SQL translator](https://docs.cloud.google.com/bigquery/docs/interactive-sql-translator) can
automatically translate your Apache Spark SQL or HiveQL to GoogleSQL.

If you don't want to migrate your Apache Spark SQL or HiveQL to
BigQuery, you can use Managed Service for Apache Spark or the
[BigQuery connector with Apache Spark](https://docs.cloud.google.com/dataproc/docs/tutorials/bigquery-connector-spark-example).

### Hive ETL

If there are any existing ETL jobs in Hive, you can modify them in the
following ways to migrate them from Hive:

- Convert the Hive ETL job to a BigQuery job by using the [batch SQL translator](https://docs.cloud.google.com/bigquery/docs/batch-sql-translator).
- Use Apache Spark to read from and write to BigQuery by using the [BigQuery connector](https://docs.cloud.google.com/dataproc/docs/concepts/connectors/bigquery#dataproc_name_clusters). You can use Managed Service for Apache Spark to run your Apache Spark jobs in a cost-efficient way with the help of ephemeral clusters.
- Rewrite your pipelines using the [Apache Beam](https://beam.apache.org/) SDK and run them on Dataflow.
- Use [Apache Beam SQL](https://beam.apache.org/documentation/dsls/sql/overview/) to rewrite your pipelines.

To manage your ETL pipeline, you can use
[Managed Service for Apache Airflow](https://docs.cloud.google.com/composer/docs)
(Apache Airflow) and
[Managed Service for Apache Spark Workflow Templates](https://docs.cloud.google.com/dataproc/docs/concepts/workflows/overview).
Managed Service for Apache Airflow provides a
[tool](https://github.com/GoogleCloudPlatform/oozie-to-airflow/)
for converting Oozie workflows to Managed Service for Apache Airflow workflows.

### Dataflow

If you want to move your Hive ETL pipeline to fully managed cloud services,
consider writing your data pipelines using the Apache Beam SDK and running
them on Dataflow.

[Dataflow](https://docs.cloud.google.com/dataflow/docs)
is a managed service for executing data processing pipelines. It executes
programs written using the open source framework
[Apache Beam](https://beam.apache.org/).
Apache Beam is a unified programming model that lets you develop both
batch and streaming pipelines.

If your data pipelines are standard data movement, you can use
Dataflow templates to quickly create Dataflow pipelines
without writing code. You can refer to this
[Google-provided template](https://docs.cloud.google.com/dataflow/docs/guides/templates/provided-templates#cloud-storage-text-to-bigquery)
which lets you read text files from Cloud Storage, apply
transformations, and write the results to a BigQuery table.

To further simplify data processing, you can also try
[Beam SQL](https://beam.apache.org/documentation/dsls/sql/walkthrough/)
which lets you process data using SQL-like statements.