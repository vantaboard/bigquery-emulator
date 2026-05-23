# Introduction to Cloud Storage transfers

The [BigQuery Data Transfer Service](https://docs.cloud.google.com/bigquery/docs/dts-introduction) for Cloud Storage lets you schedule recurring
data loads from [Cloud Storage buckets](https://docs.cloud.google.com/storage/docs/buckets) to
BigQuery. The path to the data stored in Cloud Storage and
the destination table can both be
[parameterized](https://docs.cloud.google.com/bigquery/docs/gcs-transfer-parameters),
allowing you to load data from Cloud Storage buckets organized by date.

## Supported file formats

The BigQuery Data Transfer Service supports loading data
from Cloud Storage in one of the following formats:

- Comma-separated values (CSV)
- JSON (newline-delimited)
- Avro
- Parquet
- ORC

## Supported compression types

The BigQuery Data Transfer Service for Cloud Storage supports loading compressed data.
The compression types supported by BigQuery Data Transfer Service are the same as the
compression types supported by BigQuery load jobs. For more
information, see
[Loading compressed and uncompressed data](https://docs.cloud.google.com/bigquery/docs/batch-loading-data#loading_compressed_and_uncompressed_data).

## Data ingestion for Cloud Storage transfers

You can specify how data is loaded into BigQuery by selecting a
**Write Preference** in the transfer configuration when you
[set up a Cloud Storage transfer](https://docs.cloud.google.com/bigquery/docs/cloud-storage-transfer#set_up_a_cloud_storage_transfer).
There are two types of write preferences available, [incremental transfers](https://docs.cloud.google.com/bigquery/docs/cloud-storage-transfer-overview#incremental_transfers) and [truncated transfers](https://docs.cloud.google.com/bigquery/docs/cloud-storage-transfer-overview#truncated_transfers).

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

## Cloud Storage resource path

To load data from a Cloud Storage data source, you must provide the path to
the data.

The Cloud Storage resource path contains your bucket name and your
object (filename). For example, if the Cloud Storage bucket is named
`mybucket` and the data file is named `myfile.csv`, the resource path would be
`gs://mybucket/myfile.csv`.

BigQuery does not support Cloud Storage resource paths
that include multiple consecutive slashes after the initial double slash.
Cloud Storage object names can contain multiple consecutive slash ("/")
characters. However, BigQuery converts multiple consecutive
slashes into a single slash. For example, the following resource path, though
valid in Cloud Storage, does not work in BigQuery:
`gs://bucket/my//object//name`.

To retrieve the Cloud Storage resource path:

1. Open the Cloud Storage console.

   [Cloud Storage console](https://console.cloud.google.com/storage/browser)
2. Browse to the location of the object (file) that contains the source data.

3. Click on the name of the object.

   The **Object details** page opens.
4. Copy the value provided in the **gsutil URI** field, which begins with
   `gs://`.

> [!NOTE]
> **Note:** You can also use the [`gcloud storage ls`](https://docs.cloud.google.com/sdk/gcloud/reference/storage/ls) command to list buckets or objects.

### Wildcard support for Cloud Storage resource paths

If your Cloud Storage data is separated into multiple files that share a
common base name, you can use a wildcard in the resource path when you load the
data.

To add a wildcard to the Cloud Storage resource path, you append an
asterisk (\*) to the base name. For example, if you have two files named
`fed-sample000001.csv` and `fed-sample000002.csv`, the resource path would be
`gs://mybucket/fed-sample*`. This wildcard can then be used in the
Google Cloud console or Google Cloud CLI.

You can use multiple wildcards for objects (filenames) within buckets. The
wildcard can appear anywhere inside the object name.

Wildcards don't expand a directory in a `gs://bucket/`. For example,
`gs://bucket/dir/*` finds files in the directory `dir` but doesn't find
files in the subdirectory `gs://bucket/dir/subdir/`.

Neither can you match on prefixes without wildcards. For example,
`gs://bucket/dir` doesn't match on `gs://bucket/dir/file.csv` nor
`gs://bucket/file.csv`

However, you can use multiple wildcards for filenames within buckets.
For example, `gs://bucket/dir/*/*.csv` matches
`gs://bucket/dir/subdir/file.csv`.

For examples of wildcard support in combination with parameterized table names,
see
[Runtime parameters in transfers](https://docs.cloud.google.com/bigquery/docs/gcs-transfer-parameters).

## Quotas and limits

The BigQuery Data Transfer Service uses load jobs to load Cloud Storage data into
BigQuery.

All BigQuery
[quotas and limits](https://docs.cloud.google.com/bigquery/quotas#load_jobs) on load jobs apply to recurring
Cloud Storage load jobs, with the following additional considerations:

| Value | Limit |
|---|---|
| Maximum size per load job transfer run | 15 TB |
| Maximum number of files per transfer run | 10,000 files |

## Pricing

After data is transferred to BigQuery, standard
BigQuery [storage](https://docs.cloud.google.com/bigquery/pricing#storage) and
[query](https://docs.cloud.google.com/bigquery/pricing#queries) pricing applies.

For cross-location transfers from Cloud Storage, pricing is determined by the
location of the Cloud Storage bucket and the location of the destination
BigQuery dataset. For more information, see [Data transfer within Google Cloud](https://docs.cloud.google.com/storage/pricing#network-buckets).

For more information about pricing, see [BigQuery pricing](https://cloud.google.com/bigquery/pricing#data-transfer-service-pricing).

## What's next

- Learn about [setting up a Cloud Storage transfer](https://docs.cloud.google.com/bigquery/docs/cloud-storage-transfer).
- Learn about [runtime parameters in Cloud Storage transfers](https://docs.cloud.google.com/bigquery/docs/gcs-transfer-parameters).
- Learn more about the [BigQuery Data Transfer Service](https://docs.cloud.google.com/bigquery/docs/dts-introduction).