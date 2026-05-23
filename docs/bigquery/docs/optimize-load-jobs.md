# Optimize load jobs

The strategies and best practices described in this document help you optimize
batch loading or streaming data into BigQuery to avoid
reaching [the limit for the number of load jobs](https://docs.cloud.google.com/bigquery/quotas#load_jobs)
per table, per day.

Because the limit for load jobs is fixed and can't be increased, you should
optimize your load jobs by structuring your tables through methods such as
table partitions, or by managing your loads through methods such as
batch loading or streaming.

## How table operations quotas work

The BigQuery limit for table modifications per table per day
per project is fixed, regardless of whether modifications append or
update data, or truncate the table. This limit includes the combined total of
all [load jobs](https://docs.cloud.google.com/bigquery/quotas#load_jobs), [copy
jobs](https://docs.cloud.google.com/bigquery/quotas#copy_jobs), and [query jobs](https://docs.cloud.google.com/bigquery/quotas#query_jobs)
that add to or overwrite a destination table.

Load jobs have a *refill rate* . If you exceed the table operation limit or its
refill rate, load jobs fail with a `quotaExceeded` error. The
project-level limit for load jobs per day refills within a rolling 24-hour
period. When load jobs finish, your available quota decreases. The quota then
gradually refills over the next 24 hours. Failed load jobs still count toward
both per-table and per-project quotas. For more information about load job
limits, see [Load jobs](https://docs.cloud.google.com/bigquery/quotas#load_jobs).

For partitioned tables, a separate [limit for partitioned table modifications](https://docs.cloud.google.com/bigquery/quotas#partitioned_tables)
applies, replacing the standard table limit.

To stay within your daily table operation limits, spread operations over a
24-hour period. For example, if you perform 25 updates, each with 60 operations,
you can run about 60 operations every 58 minutes. This approach helps you meet
the daily limit. To monitor table updates, see [BigQuery
`INFORMATION_SCHEMA` views](https://docs.cloud.google.com/bigquery/docs/monitoring#information-schema).

### Table operations excluded from quota

Updating table information (metadata) and using DML statements does not count
toward your daily table modification limit. This exclusion applies to both
standard and partitioned tables.

Your project can run an unlimited number of DML statements. While DML statements
previously counted toward daily table modifications and were not throttled even
at the limit, they no longer do.

*Streaming inserts* also modify tables, but [their own specific
quotas](https://docs.cloud.google.com/bigquery/quotas#streaming_inserts) govern them.

## Load strategies to avoid the table operations limit

To stay within BigQuery's daily table operation limit, consider these best practices:

- Perform fewer, larger writes instead of many small ones.
- Minimize separate write jobs to your final production table each day.

To use these best practices, batch or stream your data into BigQuery. Your choice of load method
depends on whether you need to load high volumes of data in real time, or if
real-time loading is not a concern. The following sections explain batch loading
and data streaming in detail, including the tools and services you can use for each
method.

## Batch loading

To stay within the daily load limit per project for BigQuery,
batch large amounts of data and load it with fewer jobs into
BigQuery. The following sections describe several methods you
can use to batch load your data.

### Load more data for each job

Instead of sending data to BigQuery each time new information
becomes available, collect and load it into
BigQuery using a single large job.

For example, instead of running a separate load job for every few rows of data,
you can wait until you accumulate several thousand rows of data in a file---for
example, in a CSV or JSON file---and then run one load job to append all the data
to a table. This action counts as one table operation, even though the job
contains much more data. You can batch your files by
[using wildcards](https://docs.cloud.google.com/bigquery/docs/batch-loading-data#load-wildcards) with
your load job. Wildcards let you select batches of files in a directory to
load multiple files in a single load job.

The following example shows how to use wildcards with your `bq load` command or
SQL `LOAD DATA` queries.

### bq

The following example shows a [`bq load` command](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#bq_load)
to load CSV data from Cloud Storage into a BigQuery table named
`my_target_table`. To select more than one source filename, use a wildcard
with the command. The `AUTODETECT` flag automatically determines your table
schema from the source data in Cloud Storage, and can support
a wildcard (`*`) to load multiple files that fit a specific naming pattern
into the BigQuery table.

```bash
bq load \
  --source_format=CSV \
  --autodetect \
  --project_id=PROJECT_ID \
  DATASET_NAME.TABLE_NAME \
  "gs://BUCKET_NAME/OBJECT_PATH_WILDCARD"
```

Replace the following:

- `PROJECT_ID`: the ID of your Google Cloud project.
- `DATASET_NAME`: the name of the BigQuery dataset where you want to load the data.
- `TABLE_NAME`: the name of the BigQuery table where you want to load the data.
- `BUCKET_NAME`: the name of your Cloud Storage bucket that contains the source files.
- `OBJECT_PATH_WILDCARD`: the path to your CSV files in the Cloud Storage bucket. Include a wildcard (`*`) to match multiple files. For example, the string `gs://my-bucket/path/to/data/my_prefix_*.csv` uses the wildcard character `*` to load all files in `gs://my-bucket/path/to/data/` that begin with `my_prefix_` and end with `.csv`.

For more information, see the following:

- [Wildcard support for Cloud Storage URIs](https://docs.cloud.google.com/bigquery/docs/batch-loading-data#load-wildcards)
- [bq command-line tool reference](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference)

### SQL

The following example shows how to use the SQL
[`LOAD DATA` query](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/load-statements)
to load CSV data from a Cloud Storage bucket into
BigQuery table. To select more than one source filename, use a
wildcard with the command.

    LOAD DATA INTO
    DATASET_NAME.TABLE_NAME
    FROM FILES (
      format = 'SOURCE_FORMAT',
      uris = ['gs://BUCKET_NAME/OBJECT_PATH_WILDCARD]
      );

Replace the following:

- `DATASET_NAME`: the name of the BigQuery dataset where you want to load the data.
- `TABLE_NAME`: the name of the BigQuery table where you want to load the data.
- The `SOURCE_FORMAT` sets the type of your source files, for example, `CSV` or `JSON`. In this example, use `CSV`.
- `BUCKET_NAME`: the name of your Cloud Storage bucket that contains the source files.
- `OBJECT_PATH_WILDCARD`: the path to your CSV files in the Cloud Storage bucket. Include a wildcard (`*`) to match multiple files. For example, the string `gs://my-bucket/path/to/data/my_prefix_*.csv` uses the wildcard character `*` to load all files in `gs://my-bucket/path/to/data/` that begin with `my_prefix_` and end with `.csv`.

For more information, see [Load statements in GoogleSQL](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/load-statements).

### Batch load using the BigQuery Storage Write API

To load batch data into BigQuery, one option is to use the
Storage Write API directly from your application with the
Google API Client Libraries.

The Storage Write API optimizes data loading to stay within table
limits. For high-volume, real-time streaming, use a `PENDING` stream, rather
than a `COMMITTED` stream. When you use a `PENDING` stream, the API temporarily
stores records until you commit the stream.

For a complete example of batch loading data using the Storage Write API, see
[Batch load data using the Storage Write API](https://docs.cloud.google.com/bigquery/docs/write-api-batch-load).

### Batch load using Dataflow

If you want to stream, transform, and write data into BigQuery
using data pipelines, you can use Dataflow. The data pipelines
that you create read from supported sources like Pub/Sub or
Apache Kafka. You can also create a Dataflow pipeline
using the `BigQueryIO` connector, which uses the Storage Write API
for high-performance data streaming and exactly-once semantics.

For information about using Dataflow to batch load data to
BigQuery, see [Write from Dataflow to
BigQuery](https://docs.cloud.google.com/dataflow/docs/guides/write-to-bigquery).

## Data streaming

To load high volumes of data with frequent updates, we recommend that you stream
your data into BigQuery. With data streaming, new data
continuously writes from your client application into BigQuery,
a strategy that avoids reaching the limit for running too many load jobs. The
following sections describe several methods to stream your data into
BigQuery.

### Stream data using the Storage Write API

Use the [Storage Write API](https://docs.cloud.google.com/bigquery/docs/write-api) to
stream records in real time into BigQuery with minimal
latency. The [Storage Write API](https://docs.cloud.google.com/bigquery/docs/write-api) provides
an efficient streaming protocol that provides advanced functionality like
exactly-once delivery semantics, schema update detection, and streaming
Change Data Capture (CDC) upserts. In addition, you can ingest up to 2 TiB per
month at no cost.

For information about using the Storage Write API, see
[Streaming data using the Storage Write API](https://docs.cloud.google.com/bigquery/docs/write-api-streaming).

### Stream data using Dataflow

Use Dataflow to create data pipelines that read from
supported sources, for example, Pub/Sub or Apache
Kafka. These pipelines then transform and write the data to
BigQuery as a destination. You can create a Dataflow
pipeline using the `BigQueryIO` connector, which uses the
Storage Write API.

For information about using Dataflow to stream data to
BigQuery, see [Write from Dataflow to
BigQuery](https://docs.cloud.google.com/dataflow/docs/guides/write-to-bigquery).

## Best practices to manage your tables for loading

In addition to batch loading or streaming data into BigQuery,
manage your tables in the following ways to optimize them for
data ingestion.

### Use partitioned tables

[Table partitioning](https://docs.cloud.google.com/bigquery/docs/partitioned-tables) is a powerful technique
for managing large tables in BigQuery, especially when you need
to perform frequent data loading operations. You can significantly improve table
performance and cost-effectiveness by dividing a table into smaller, more
manageable segments based on a date, timestamp, or integer.

The primary advantage of partitioning for data loading is that the daily table
operation quotas for BigQuery apply at the partition level rather
than at the table level. For partitioned tables, a separate, higher limit
applies to [partition modifications](https://docs.cloud.google.com/bigquery/quotas#partitioned_tables), which
replaces the [standard table limit](https://docs.cloud.google.com/bigquery/quotas#standard_tables). The limit
for partitioned tables dramatically increases the number of load jobs you can
run per day without reaching quota limits.

A common and highly effective strategy is to batch load your daily data. For
example, you can gather all of the day's data for `2025-09-18` in a temporary
staging table. Then, at the end of the day, you run a single job to load
this data into the specific partition for this day in your main production table.
Because BigQuery interacts only with the data for a single
partition, this approach keeps your data well organized and makes your
loading operations faster and less expensive.

While partitioning is highly recommended for large, growing tables, it's best to
avoid it if your partitions would be consistently smaller than 10 GB. For more
information, see [When to use partitioning](https://docs.cloud.google.com/bigquery/docs/partitioned-tables#when_to_use_partitioning).

To learn more about the different partitioning methods available, such as
time-unit and integer-range partitioning, see
[Types of partitioned tables](https://docs.cloud.google.com/bigquery/docs/partitioned-tables#types_of_partitioning).

### Take advantage of built-in exponential backoff, truncate, and jitter

Built-in [exponential backoff and
retry](https://docs.cloud.google.com/bigquery/docs/reliability-intro#error_handling)
is an error-handling method that helps your application recover smoothly when an
operation fails temporarily. Such failures can include a rate limit error
(`rateLimitExceeded`) or a brief network problem (`unavailable`).

In a reliable system, workers that take tasks from your client-side queue also
use exponential backoff and retry. They do this when calling
BigQuery, which creates two levels of protection.

For example, the official `google-cloud-bigquery-storage` library for Python
includes built-in retry logic with exponential backoff. This logic handles
temporary gRPC errors, for example, `UNAVAILABLE`. In most cases, you don't need
to write this retry code yourself. The `client.append_rows()` call handles these
retries automatically.

This built-in handling is a significant benefit of using the official client
libraries. You only need to deal with errors that cannot be retried, for example,
`INVALID_ARGUMENT`, which means there is a schema mismatch.