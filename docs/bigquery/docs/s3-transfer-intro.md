# Introduction to Amazon S3 transfers

The [BigQuery Data Transfer Service](https://docs.cloud.google.com/bigquery/docs/dts-introduction) for
Amazon S3 lets you automatically schedule and manage recurring load jobs
from Amazon S3 into BigQuery.

## Supported file formats

The BigQuery Data Transfer Service supports loading data from Amazon S3 in one of the
following formats:

- Comma-separated values (CSV)
- JSON (newline-delimited)
- Avro
- Parquet
- ORC

## Supported compression types

The BigQuery Data Transfer Service for Amazon S3 supports loading compressed data. The
compression types supported by BigQuery Data Transfer Service are the same as the
compression types supported by BigQuery load jobs. For more
information, see
[Loading compressed and uncompressed data](https://docs.cloud.google.com/bigquery/docs/batch-loading-data#loading_compressed_and_uncompressed_data).

## Amazon S3 prerequisites

To load data from an Amazon S3 data source, you must:

- Provide the Amazon S3 URI for your source data
- Have your access key ID
- Have your secret access key
- Set, at a minimum, the AWS managed policy [`AmazonS3ReadOnlyAccess`](https://docs.aws.amazon.com/IAM/latest/UserGuide/access_policies_manage.html) on your Amazon S3 source data

## Amazon S3 URIs

When you supply the Amazon S3 URI, the path must be in the following format
`s3://bucket/folder1/folder2/...` Only the top-level bucket name is required.
Folder names are optional. If you specify a URI that includes only the bucket
name, all files in the bucket are transferred and loaded into
BigQuery.

## Amazon S3 transfer runtime parameterization

The Amazon S3 URI and the destination table can both be [parameterized](https://docs.cloud.google.com/bigquery/docs/s3-transfer-parameters),
allowing you to load data from Amazon S3 buckets organized by date. Note that
the bucket portion of the URI cannot be parameterized. The parameters
used by Amazon S3 transfers are the same as those used by Cloud Storage
transfers.

For details, see [Runtime parameters in transfers](https://docs.cloud.google.com/bigquery/docs/s3-transfer-parameters).

## Data ingestion for Amazon S3 transfers

You can specify how data is loaded into BigQuery by selecting a
**Write Preference** in the transfer configuration when you
[set up an Amazon S3 transfer](https://docs.cloud.google.com/bigquery/docs/s3-transfer#set_up_an_amazon_s3_data_transfer).
There are two types of write preferences available, [incremental transfers](https://docs.cloud.google.com/bigquery/docs/s3-transfer-intro#incremental_transfers) and [truncated transfers](https://docs.cloud.google.com/bigquery/docs/s3-transfer-intro#truncated_transfers).

<br />

### Incremental transfers

A transfer configuration with an **`APPEND`** or **`WRITE_APPEND`** write
preference, also called an incremental transfer, incrementally appends new data
since the previous successful transfer to a BigQuery destination
table. When a transfer configuration runs with an **`APPEND`** write preference,
the
BigQuery Data Transfer Service filters for files which have been modified since the
previous successful transfer run. To determine when a file is modified,
BigQuery Data Transfer Service looks at the file metadata for a "last modified time"
property. For example, the BigQuery Data Transfer Service looks at the [`updated` timestamp property](https://docs.cloud.google.com/storage/docs/metadata#timestamps)
in a Cloud Storage file. If the
BigQuery Data Transfer Service finds any files with a "last modified time" that have
occurred after the timestamp of the last successful transfer, the
BigQuery Data Transfer Service transfers those files in an incremental transfer.

To demonstrate how incremental transfers work, consider the following
Cloud Storage transfer example. A user creates a file in a
Cloud Storage bucket at time 2023-07-01T00:00Z named `file_1`. The
[`updated` timestamp](https://docs.cloud.google.com/storage/docs/metadata#timestamps) for `file_1` is
the time that the file was created. The user then
creates an incremental transfer from the Cloud Storage bucket,
scheduled to run once daily at time 03:00Z, starting from 2023-07-01T03:00Z.

- At 2023-07-01T03:00Z, the first transfer run starts. As this is the first transfer run for this configuration, BigQuery Data Transfer Service attempts to load all files matching the source URI into the destination BigQuery table. The transfer run succeeds and BigQuery Data Transfer Service successfully loads `file_1` into the destination BigQuery table.
- The next transfer run, at 2023-07-02T03:00Z, detects no files where the `updated` timestamp property is greater than the last successful transfer run (2023-07-01T03:00Z). The transfer run succeeds without loading any additional data into the destination BigQuery table.

The preceding example shows how the BigQuery Data Transfer Service looks at the
`updated` timestamp property of the source file to determine if any changes were
made to the source files, and to transfer those changes if any were detected.

Following the same example, suppose that the user then creates another file in
the Cloud Storage bucket at time 2023-07-03T00:00Z, named `file_2`. The
[`updated` timestamp](https://docs.cloud.google.com/storage/docs/metadata#timestamps) for `file_2` is
the time that the file was created.

- The next transfer run, at 2023-07-03T03:00Z, detects that `file_2` has an `updated` timestamp greater than the last successful transfer run (2023-07-01T03:00Z). Suppose that when the transfer run starts it fails due to a transient error. In this scenario, `file_2` is not loaded into the destination BigQuery table. The last successful transfer run timestamp remains at 2023-07-01T03:00Z.
- The next transfer run, at 2023-07-04T03:00Z, detects that `file_2` has an `updated` timestamp greater than the last successful transfer run (2023-07-01T03:00Z). This time, the transfer run completes without issue, so it successfully loads `file_2` into the destination BigQuery table.
- The next transfer run, at 2023-07-05T03:00Z, detects no files where the `updated` timestamp is greater than the last successful transfer run (2023-07-04T03:00Z). The transfer run succeeds without loading any additional data into the destination BigQuery table.

The preceding example shows that when a transfer fails, no files are
transferred to the BigQuery destination table. Any file changes
are transferred at the next successful transfer run. Any subsequent
successful transfers following a failed transfer does not cause duplicate
data. In the case of a failed transfer, you can also choose to [manually trigger a transfer](https://docs.cloud.google.com/bigquery/docs/working-with-transfers#manually_trigger_a_transfer)
outside its regularly scheduled time.

> [!WARNING]
> **Warning:** BigQuery Data Transfer Service relies on the "last modified time" property in each source file to determine which files to transfer, as seen in the incremental transfer examples. Modifying these properties can cause the transfer to skip certain files, or load the same file multiple times. This property can have different names in each storage system supported by BigQuery Data Transfer Service. For example, Cloud Storage objects call this property [`updated`](https://docs.cloud.google.com/storage/docs/metadata#timestamps).

### Truncated transfers

A transfer configuration with a **`MIRROR`** or **`WRITE_TRUNCATE`** write
preference, also called a truncated transfer, overwrites data in the
BigQuery destination table during each transfer run with data
from all files matching the source URI. **`MIRROR`** overwrites a fresh copy of
data in the destination table. If the destination table is using a partition
decorator, the transfer run only overwrites data in the specified partition. A
destination table with a partition decorator has the format
`my_table${run_date}`---for example, `my_table$20230809`.

Repeating the same incremental or truncated transfers in a day does not cause
duplicate data. However, if you run multiple different transfer
configurations that affect the same BigQuery destination table,
this can cause the BigQuery Data Transfer Service to duplicate data.

## Wildcard support for Amazon S3 URIs

If your source data is separated into multiple files that share a common base
name, you can use a wildcard in the URI when you load the data. A wildcard
consists of an asterisk (\*), and can be used anywhere in the Amazon S3 URI
except for the bucket name.

While more than one wildcard can be used in the Amazon S3 URI, some optimization
is possible when the Amazon S3 URI specifies only a single wildcard:

- There is a [higher limit](https://docs.cloud.google.com/bigquery/docs/s3-transfer-intro#quotas_and_limits) on the maximum number of files
  per transfer run.

- The wildcard will span directory boundaries. For example, the Amazon S3 URI
  `s3://my-bucket/*.csv` will match the file
  `s3://my-bucket/my-folder/my-subfolder/my-file.csv`.

## Amazon S3 URI examples

### Example 1

To load a single file from Amazon S3 into BigQuery, specify the
Amazon S3 URI of the file.

    s3://my-bucket/my-folder/my-file.csv

### Example 2

To load all files from an Amazon S3 bucket into BigQuery, specify
only the bucket name, with or without a wildcard.

    s3://my-bucket/

or

    s3://my-bucket/*

Note that `s3://my-bucket*` is not a permitted Amazon S3 URI, as a wildcard
can't be used in the bucket name.

### Example 3

To load all files from Amazon S3 that share a common prefix, specify the common
prefix followed by a wildcard.

    s3://my-bucket/my-folder/*

Note that in contrast to loading all files from a top level Amazon S3 bucket,
the wildcard must be specified at the end of the Amazon S3 URI for any files to
be loaded.

### Example 4

To load all files from Amazon S3 with a similar path, specify the common prefix
followed by a wildcard.

    s3://my-bucket/my-folder/*.csv

### Example 5

Note the wildcards span directories, so any `csv` files in `my-folder`, as well
as in subfolders of `my-folder` will be loaded into BigQuery.

If you have these source files under a `logs` folder:

    s3://my-bucket/logs/logs.csv
    s3://my-bucket/logs/system/logs.csv
    s3://my-bucket/logs/some-application/system_logs.log
    s3://my-bucket/logs/logs_2019_12_12.csv

then the following identifies them:

    s3://my-bucket/logs/*

### Example 6

If you have these source files, but want to transfer only those that have
`logs.csv` as the filename:

    s3://my-bucket/logs.csv
    s3://my-bucket/metadata.csv
    s3://my-bucket/system/logs.csv
    s3://my-bucket/system/users.csv
    s3://my-bucket/some-application/logs.csv
    s3://my-bucket/some-application/output.csv

then the following identifies the files with `logs.csv` in the name:

    s3://my-bucket/*logs.csv

### Example 7

By using multiple wildcards, more control can be achieved over which files are
transferred, at the cost of [lower limits](https://docs.cloud.google.com/bigquery/docs/s3-transfer-intro#quotas_and_limits). Using multiple
wildcards means that each wildcard will only match up to the end of a path
within a subdirectory. For example, for the following source files in Amazon S3:

    s3://my-bucket/my-folder1/my-file1.csv
    s3://my-bucket/my-other-folder2/my-file2.csv
    s3://my-bucket/my-folder1/my-subfolder/my-file3.csv
    s3://my-bucket/my-other-folder2/my-subfolder/my-file4.csv

If the intention is to only transfer `my-file1.csv` and `my-file2.csv`, use the
following as the value for the Amazon S3 URI:

    s3://my-bucket/*/*.csv

As neither wildcard spans directories, this URI would limit the transfer to only
the CSV files that are in `my-folder1` and `my-other-folder2`. Subfolders would
not be included in the transfer.

## AWS access keys

The access key ID and secret access key are used to access the Amazon S3 data on
your behalf. As a best practice, create a unique access key ID and secret access
key specifically for Amazon S3 transfers to give minimal access to the
BigQuery Data Transfer Service. For information on managing your access keys, see the
[AWS general reference documentation](https://docs.aws.amazon.com/general/latest/gr/managing-aws-access-keys.html).

## IP restrictions

If you use IP restrictions for access to Amazon S3, you must add the IP
ranges used by BigQuery Data Transfer Service workers to your list of allowed IPs.

To add IP ranges as allowed public IP addresses to Amazon S3, see
[IP restrictions](https://docs.cloud.google.com/storage-transfer/docs/source-amazon-s3#ip_restrictions).

## Consistency considerations

When you transfer data from Amazon S3, it is possible that some of your data
won't be transferred to BigQuery, particularly if the files
were added to the bucket very recently. It should take approximately 5 minutes
for a file to become available to the BigQuery Data Transfer Service after it is added to
the bucket.

> [!IMPORTANT]
> **Important:** To reduce the possibility of missing data, schedule your Amazon S3 transfers to occur at least 5 minutes after your files are added to the bucket.

## Outbound data transfer costs best practice

Transfers from Amazon S3 could fail if the destination table has not been
configured properly. Reasons that could result in an improper configuration
include:

- The destination table does not exist.
- The table schema is not defined.
- The table schema is not compatible with the data being transferred.

To avoid Amazon S3 outbound data transfer costs, you should first test a transfer
with a small but representative subset of the files. Small means the test should
have a small data size, and a small file count.

## Pricing

For information on BigQuery Data Transfer Service pricing, see the
[Pricing](https://cloud.google.com/bigquery/pricing#data-transfer-service-pricing)
page.

Note that costs can be incurred outside of Google by using this service. Review the [Amazon S3 pricing page](https://aws.amazon.com/s3/pricing/)
for details.

## Quotas and limits

The BigQuery Data Transfer Service uses load jobs to load Amazon S3 data into
BigQuery. All BigQuery [Quotas and
limits](https://docs.cloud.google.com/bigquery/quotas#load_jobs) on load jobs apply to recurring
Amazon S3 transfers, with the following additional considerations:

| Value | Limit |
|---|---|
| Maximum size per load job transfer run | 15 TB |
| Maximum number of files per transfer run when the Amazon S3 URI includes 0 or 1 wildcards | 10,000,000 files |
| Maximum number of files per transfer run when the Amazon S3 URI includes more than 1 wildcard | 10,000 files |

## What's next

- Learn about [setting up an Amazon S3 transfer](https://docs.cloud.google.com/bigquery/docs/s3-transfer).
- Learn about [runtime parameters in S3 transfers](https://docs.cloud.google.com/bigquery/docs/s3-transfer-parameters).
- Learn more about the [BigQuery Data Transfer Service](https://docs.cloud.google.com/bigquery/docs/dts-introduction).