# Loading CSV data from Cloud Storage

When you load CSV data from Cloud Storage, you can load the data into a new
table or partition, or you can append to or overwrite an existing table or
partition. When your data is loaded into BigQuery, it is
converted into columnar format for
[Capacitor](https://cloud.google.com/blog/products/bigquery/inside-capacitor-bigquerys-next-generation-columnar-storage-format)
(BigQuery's storage format).

When you load data from Cloud Storage into a BigQuery table,
the dataset that contains the table must be in the same regional or multi-
regional location as the Cloud Storage bucket.

For information about loading CSV data from a local file, see
[Loading data into BigQuery from a local data source](https://docs.cloud.google.com/bigquery/docs/batch-loading-data#loading_data_from_local_files).

## Limitations

You are subject to the following limitations when you load data into
BigQuery from a Cloud Storage bucket:

- BigQuery does not guarantee data consistency for external data sources. Changes to the underlying data while a query is running can result in unexpected behavior.
- BigQuery doesn't support [Cloud Storage object versioning](https://docs.cloud.google.com/storage/docs/object-versioning). If you include a generation number in the Cloud Storage URI, then the load job fails.

When you load CSV files into BigQuery, note the following:

- CSV files don't support nested or repeated data.
- Remove byte order mark (BOM) characters. They might cause unexpected issues.
- If you use gzip compression, BigQuery cannot read the data in parallel. Loading compressed CSV data into BigQuery is slower than loading uncompressed data. See [Loading compressed and uncompressed data](https://docs.cloud.google.com/bigquery/docs/batch-loading-data#loading_compressed_and_uncompressed_data).
- You cannot include both compressed and uncompressed files in the same load job.
- The maximum size for a gzip file is 4 GB.
- Loading CSV data [using schema autodetection](https://docs.cloud.google.com/bigquery/docs/schema-detect) does not automatically detect headers if all of the columns are string types. In this case, add a numerical column to the input or declare the schema explicitly.
- When you load CSV or JSON data, values in `DATE` columns must use the dash (`-`) separator and the date must be in the following format: `YYYY-MM-DD` (year-month-day).
- When you load JSON or CSV data, values in `TIMESTAMP` columns must use a dash (`-`) or slash (`/`) separator for the date portion of the timestamp, and the date must be in one of the following formats: `YYYY-MM-DD` (year-month-day) or `YYYY/MM/DD` (year/month/day). The `hh:mm:ss` (hour-minute-second) portion of the timestamp must use a colon (`:`) separator.
- Your files must meet the CSV file size limits described in the [load jobs limits](https://docs.cloud.google.com/bigquery/quotas#load_jobs).

## Before you begin

Grant Identity and Access Management (IAM) roles that give users the necessary
permissions to perform each task in this document, and create a dataset
to store your data.

### Required permissions

To load data into BigQuery, you need IAM permissions to run a load job and load data into BigQuery tables and partitions. If you are loading data from Cloud Storage, you also need IAM permissions to access the bucket that contains your data.

#### Permissions to load data into BigQuery

To load data into a new BigQuery table or partition or to append or overwrite an existing table or partition, you need the following IAM permissions:

- `bigquery.tables.create`
- `bigquery.tables.updateData`
- `bigquery.tables.update`
- `bigquery.jobs.create`

Each of the following predefined IAM roles includes the permissions that you need in order to load data into a BigQuery table or partition:

- `roles/bigquery.dataEditor`
- `roles/bigquery.dataOwner`
- `roles/bigquery.admin` (includes the `bigquery.jobs.create` permission)
- `bigquery.user` (includes the `bigquery.jobs.create` permission)
- `bigquery.jobUser` (includes the `bigquery.jobs.create` permission)

Additionally, if you have the `bigquery.datasets.create` permission, you can create and
update tables using a load job in the datasets that you create.

For more information on IAM roles and permissions in
BigQuery, see [Predefined roles and permissions](https://docs.cloud.google.com/bigquery/access-control).

### Permissions to load data from Cloud Storage


To get the permissions that
you need to load data from a Cloud Storage bucket,

ask your administrator to grant you the
[Storage Admin](https://docs.cloud.google.com/iam/docs/roles-permissions/storage#storage.admin) (`roles/storage.admin`) IAM role on the bucket.


For more information about granting roles, see [Manage access to projects, folders, and organizations](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).


This predefined role contains

the permissions required to load data from a Cloud Storage bucket. To see the exact permissions that are
required, expand the **Required permissions** section:


#### Required permissions

The following permissions are required to load data from a Cloud Storage bucket:

- `storage.buckets.get`
- `storage.objects.get`
- `storage.objects.list (required if you are using a URI https://docs.cloud.google.com/bigquery/docs/batch-loading-data#load-wildcards)`


You might also be able to get
these permissions
with [custom roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles) or
other [predefined roles](https://docs.cloud.google.com/iam/docs/roles-overview#predefined).

### Create a dataset

Create a [BigQuery dataset](https://docs.cloud.google.com/bigquery/docs/datasets) to store
your data.

## CSV compression

You can use the `gzip` utility to compress CSV files. Note that `gzip` performs
full file compression, unlike the file content compression performed by
compression codecs for other file formats, such as Avro. Using `gzip` to
compress your CSV files might have a performance impact; for more information
about the trade-offs, see
[Loading compressed and uncompressed data](https://docs.cloud.google.com/bigquery/docs/batch-loading-data#loading_compressed_and_uncompressed_data).

## Loading CSV data into a table

To load CSV data from Cloud Storage into a new BigQuery
table, select one of the following options:

### Console


*** ** * ** ***

To follow step-by-step guidance for this task directly in the
Cloud Shell Editor, click **Guide me**:

[Guide me](https://console.cloud.google.com/?tutorial=bigquery_import_data_from_cloud_storage)

*** ** * ** ***

<br />

1.
   In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Explorer**.
3. In the **Explorer** pane, expand your project, click **Datasets**, and then select a dataset.
4. In the **Dataset info** section, click **Create table**.
5. In the **Create table** pane, specify the following details:
   1. In the **Source** section, select **Google Cloud Storage** in the **Create table from** list. Then, do the following:
      1. Select a file from the Cloud Storage bucket, or enter the [Cloud Storage URI](https://docs.cloud.google.com/bigquery/docs/batch-loading-data#gcs-uri). You cannot include multiple URIs in the Google Cloud console, but [wildcards](https://docs.cloud.google.com/bigquery/docs/batch-loading-data#load-wildcards) are supported. The Cloud Storage bucket must be in the same location as the dataset that contains the table you want to create, append, or overwrite. ![select source file to create a BigQuery table](https://docs.cloud.google.com/static/bigquery/images/create-table-select-file.png)
      2. For **File format** , select **CSV**.
   2. In the **Destination** section, specify the following details:
      1. For **Dataset**, select the dataset in which you want to create the table.
      2. In the **Table** field, enter the name of the table that you want to create.
      3. Verify that the **Table type** field is set to **Native table**.
   3. In the **Schema** section, enter the [schema](https://docs.cloud.google.com/bigquery/docs/schemas) definition. To enable the [auto detection](https://docs.cloud.google.com/bigquery/docs/schema-detect) of a schema, select **Auto detect** . You can enter schema information manually by using one of the following methods:
      - Option 1: Click **Edit as text** and paste the schema in the form of a JSON array. When you use a JSON array, you generate the schema using the same process as [creating a JSON schema file](https://docs.cloud.google.com/bigquery/docs/schemas#specifying_a_json_schema_file). You can view the schema of an existing table in JSON format by entering the following command:

        ```bash
            bq show --format=prettyjson dataset.table
            
        ```
      - Option 2: Click **Add field** and enter the table schema. Specify each field's **Name** , [**Type**](https://docs.cloud.google.com/bigquery/docs/schemas#standard_sql_data_types), and [**Mode**](https://docs.cloud.google.com/bigquery/docs/schemas#modes).
   4. Optional: Specify **Partition and cluster settings** . For more information, see [Creating partitioned tables](https://docs.cloud.google.com/bigquery/docs/creating-partitioned-tables) and [Creating and using clustered tables](https://docs.cloud.google.com/bigquery/docs/creating-clustered-tables).
   5. Click **Advanced options** and do the following:
      - For **Write preference** , leave **Write if empty** selected. This option creates a new table and loads your data into it.
      - For **Number of errors allowed** , accept the default value of `0` or enter the maximum number of rows containing errors that can be ignored. If the number of rows with errors exceeds this value, the job will result in an `invalid` message and fail. This option applies only to CSV and JSON files.
      - For **Time zone** , enter the default time zone that will apply when parsing timestamp values that have no specific time zone. Check [here](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#time_zone_name) for more valid time zone names. If this value is not present, the timestamp values without specific time zone is parsed using default time zone UTC.
      - For **Date Format** , enter the [format elements](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/format-elements#format_string_as_datetime) that define how the DATE values are formatted in the input files. This field expects SQL styles format (for example, `MM/DD/YYYY`). If this value is present, this format is the only compatible DATE format. [Schema autodetection](https://docs.cloud.google.com/bigquery/docs/schema-detect#date_and_time_values) will also decide DATE column type based on this format instead of the existing format. If this value is not present, the DATE field is parsed with the [default formats](https://docs.cloud.google.com/bigquery/docs/loading-data-cloud-storage-csv#data_types).
      - For **Datetime Format** , enter the [format elements](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/format-elements#format_string_as_datetime) that define how the DATETIME values are formatted in the input files. This field expects SQL styles format (for example, `MM/DD/YYYY HH24:MI:SS.FF3`). If this value is present, this format is the only compatible DATETIME format. [Schema autodetection](https://docs.cloud.google.com/bigquery/docs/schema-detect#date_and_time_values) will also decide DATETIME column type based on this format instead of the existing format. If this value is not present, the DATETIME field is parsed with the [default formats](https://docs.cloud.google.com/bigquery/docs/loading-data-cloud-storage-csv#data_types).
      - For **Time Format** , enter the [format elements](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/format-elements#format_string_as_datetime) that define how the TIME values are formatted in the input files. This field expects SQL styles format (for example, `HH24:MI:SS.FF3`). If this value is present, this format is the only compatible TIME format. [Schema autodetection](https://docs.cloud.google.com/bigquery/docs/schema-detect#date_and_time_values) will also decide TIME column type based on this format instead of the existing format. If this value is not present, the TIME field is parsed with the [default formats](https://docs.cloud.google.com/bigquery/docs/loading-data-cloud-storage-csv#data_types).
      - For **Timestamp Format** , enter the [format elements](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/format-elements#format_string_as_datetime) that define how the TIMESTAMP values are formatted in the input files. This field expects SQL styles format (for example, `MM/DD/YYYY HH24:MI:SS.FF3`). If this value is present, this format is the only compatible TIMESTAMP format. [Schema autodetection](https://docs.cloud.google.com/bigquery/docs/schema-detect#date_and_time_values) will also decide TIMESTAMP column type based on this format instead of the existing format. If this value is not present, the TIMESTAMP field is parsed with the [default formats](https://docs.cloud.google.com/bigquery/docs/loading-data-cloud-storage-csv#data_types).
      - If you want to ignore values in a row that are not present in the table's schema, then select **Unknown values**.
      - For **Field delimiter** , choose the character that separates the cells in your CSV file: **Comma** , **Tab** , **Pipe** , or **Custom** . If you choose **Custom** , enter the delimiter in the **Custom field delimiter** box. The default value is **Comma**.
      - For **Source column match**, choose one of the following strategies used to match the loaded columns to the schema.
        - `Default`: Default behavior is chosen based on how the schema is provided. If autodetect is enabled, then the default behavior is to match columns by name. Otherwise, the default is to match columns by position. This is done to keep the behavior backward-compatible.
        - `Position`: Matches columns by position, assuming that the columns are ordered the same way as the schema.
        - `Name`: Matches by name by reading the header row as the column names and reordering columns to match the field names in the schema. Column names are read from the last skipped row based on **Header rows to skip**.
      - For **Header rows to skip** , enter the number of header rows to skip at the top of the CSV file. The default value is `0`.
      - For **Quoted newlines** , check **Allow quoted newlines** to allow quoted data sections that contain newline characters in a CSV file. The default value is `false`.
      - For **Jagged rows** , check **Allow jagged rows** to accept rows in CSV files that are missing trailing optional columns. The missing values are treated as nulls. If unchecked, records with missing trailing columns are treated as bad records, and if there are too many bad records, an invalid error is returned in the job result. The default value is `false`.
      - For **Null markers**, enter a list of custom strings that represents a NULL value in CSV data.
      - For **Encryption** , click **Customer-managed key** to use a [Cloud Key Management Service key](https://docs.cloud.google.com/bigquery/docs/customer-managed-encryption). If you leave the **Google-managed key** setting, BigQuery [encrypts the data at rest](https://docs.cloud.google.com/docs/security/encryption/default-encryption).
   6. Click **Create table**.

> [!NOTE]
> **Note:** When you load data into an empty table by using the Google Cloud console, you cannot add a label, description, table expiration, or partition expiration.  
>
> After the table is created, you can update the table's expiration, description, and labels, but you cannot add a partition expiration after a table is created using the Google Cloud console. For more information, see [Managing tables](https://docs.cloud.google.com/bigquery/docs/managing-tables).

### SQL

Use the
[`LOAD DATA` DDL statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/load-statements).
The following example loads a CSV file into the new table `mytable`:

<br />

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, enter the following statement:

   ```googlesql
   LOAD DATA OVERWRITE mydataset.mytable
   (x INT64,y STRING)
   FROM FILES (
     format = 'CSV',
     uris = ['gs://bucket/path/file.csv']);
   ```

   <br />

3. Click **Run**.

   <br />

For more information about how to run queries, see [Run an interactive query](https://docs.cloud.google.com/bigquery/docs/running-queries#queries).

### bq

Use the `bq load` command, specify `CSV` using the
`--source_format` flag, and include a [Cloud Storage URI](https://docs.cloud.google.com/bigquery/docs/batch-loading-data#gcs-uri).
You can include a single URI, a comma-separated list of URIs, or a URI
containing a [wildcard](https://docs.cloud.google.com/bigquery/docs/batch-loading-data#load-wildcards).
Supply the schema inline, in a schema definition file, or use
[schema auto-detect](https://docs.cloud.google.com/bigquery/docs/schema-detect). If you don't specify a
schema, and `--autodetect` is `false`, and the destination
table exists, then the schema of the destination table is used.

(Optional) Supply the `--location` flag and set the value to your
[location](https://docs.cloud.google.com/bigquery/docs/dataset-locations).

Other optional flags include:

- `--allow_jagged_rows`: When specified, accept rows in CSV files that are missing trailing optional columns. The missing values are treated as nulls. If unchecked, records with missing trailing columns are treated as bad records, and if there are too many bad records, an invalid error is returned in the job result. The default value is `false`.
- `--allow_quoted_newlines`: When specified, allows quoted data sections that contain newline characters in a CSV file. The default value is `false`.
- `--field_delimiter`: The character that indicates the boundary between columns in the data. Both `\t` and `tab` are allowed for tab delimiters. The default value is `,`.
- `--null_marker`: An optional custom string that represents a NULL value in CSV data.
- `--null_markers`: An optional comma-separated list of custom strings that represent NULL values in CSV data. This option cannot be used with `--null_marker` flag.
- `--source_column_match`: Specifies the strategy used to match loaded columns to the schema. You can specify `POSITION` to match loaded columns by position, assuming that the columns are ordered the same way as the schema. You can also specify `NAME` to match by name by reading the header row as the column names and reordering columns to match the field names in the schema. If this value is unspecified, then the default is based on how the schema is provided. If `--autodetect` is enabled, then the default behavior is to match columns by name. Otherwise, the default is to match columns by position.
- `--skip_leading_rows`: Specifies the number of header rows to skip at the top of the CSV file. The default value is `0`.
- `--quote`: The quote character to use to enclose records. The default value is `"`. To indicate no quote character, use an empty string.
- `--max_bad_records`: An integer that specifies the maximum number of bad records allowed before the entire job fails. The default value is `0`. At most, five errors of any type are returned regardless of the `--max_bad_records` value.
- `--ignore_unknown_values`: When specified, allows and ignores extra, unrecognized values in CSV or JSON data.
- `--time_zone`: An optional default time zone that will apply when parsing timestamp values that have no specific time zone in CSV or JSON data.
- `--date_format`: An optional custom string that defines how the DATE values are formatted in CSV or JSON data.
- `--datetime_format`: An optional custom string that defines how the DATETIME values are formatted in CSV or JSON data.
- `--time_format`: An optional custom string that defines how the TIME values are formatted in CSV or JSON data.
- `--timestamp_format`: An optional custom string that defines how the TIMESTAMP values are formatted in CSV or JSON data.
- `--autodetect`: When specified, enable schema auto-detection for CSV and JSON data.
- `--time_partitioning_type`: Enables time-based partitioning on a table and sets the partition type. Possible values are `HOUR`, `DAY`, `MONTH`, and `YEAR`. This flag is optional when you create a table partitioned on a `DATE`, `DATETIME`, or `TIMESTAMP` column. The default partition type for time-based partitioning is `DAY`. You cannot change the partitioning specification on an existing table.
- `--time_partitioning_expiration`: An integer that specifies (in seconds) when a time-based partition should be deleted. The expiration time evaluates to the partition's UTC date plus the integer value.
- `--time_partitioning_field`: The `DATE` or `TIMESTAMP` column used to create a partitioned table. If time-based partitioning is enabled without this value, an ingestion-time partitioned table is created.
- `--require_partition_filter`: When enabled, this option requires users to include a `WHERE` clause that specifies the partitions to query. Requiring a partition filter may reduce cost and improve performance. For more information, see [Querying partitioned tables](https://docs.cloud.google.com/bigquery/docs/querying-partitioned-tables).
- `--clustering_fields`: A comma-separated list of up to four column names used to create a [clustered table](https://docs.cloud.google.com/bigquery/docs/creating-clustered-tables).
- `--destination_kms_key`: The Cloud KMS key for encryption of the table data.
- `--column_name_character_map`: Defines the scope and handling of
  characters in column names, with the option of enabling
  [flexible column names](https://docs.cloud.google.com/bigquery/docs/loading-data-cloud-storage-parquet#flexible-column-names).
  Requires the `--autodetect` option for CSV files.
  For more information, see
  [`load_option_list`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/load-statements#load_option_list).

  For more information on the `bq load` command, see:
  - [Command-line reference](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#bq_load)

  For more information on partitioned tables, see:
  - [Creating partitioned tables](https://docs.cloud.google.com/bigquery/docs/creating-partitioned-tables)

  For more information on clustered tables, see:
  - [Creating and using clustered tables](https://docs.cloud.google.com/bigquery/docs/creating-clustered-tables)

  For more information on table encryption, see:
  - [Protecting data with Cloud KMS keys](https://docs.cloud.google.com/bigquery/docs/customer-managed-encryption)

To load CSV data into BigQuery, enter the following command:

```bash
bq --location=location load \
--source_format=format \
dataset.table \
path_to_source \
schema
```

Where:

- <var translate="no">location</var> is your location. The `--location` flag is optional. For example, if you are using BigQuery in the Tokyo region, you can set the flag's value to `asia-northeast1`. You can set a default value for the location using the [.bigqueryrc file](https://docs.cloud.google.com/bigquery/docs/bq-command-line-tool#setting_default_values_for_command-line_flags).
- <var translate="no">format</var> is `CSV`.
- <var translate="no">dataset</var> is an existing dataset.
- <var translate="no">table</var> is the name of the table into which you're loading data.
- <var translate="no">path_to_source</var> is a fully-qualified [Cloud Storage URI](https://docs.cloud.google.com/bigquery/docs/batch-loading-data#gcs-uri) or a comma-separated list of URIs. [Wildcards](https://docs.cloud.google.com/bigquery/docs/batch-loading-data#load-wildcards) are also supported.
- <var translate="no">schema</var> is a valid schema. The schema can be a local JSON file, or it can be typed inline as part of the command. You can also use the `--autodetect` flag instead of supplying a schema definition.

Examples:

The following command loads data from `gs://mybucket/mydata.csv` into a
table named `mytable` in `mydataset`. The schema is defined in a local
schema file named `myschema.json`.

        bq load \
        --source_format=CSV \
        mydataset.mytable \
        gs://mybucket/mydata.csv \
        ./myschema.json

The following command loads data from `gs://mybucket/mydata.csv` into a
table named `mytable` in `mydataset`. The schema is defined in a local
schema file named `myschema.json`. The CSV file includes two header rows.
If `--skip_leading_rows` is unspecified, the default behavior is to assume
the file does not contain headers.

        bq load \
        --source_format=CSV \
        --skip_leading_rows=2
        mydataset.mytable \
        gs://mybucket/mydata.csv \
        ./myschema.json

The following command loads data from `gs://mybucket/mydata.csv` into an
ingestion-time partitioned table named `mytable` in `mydataset`. The schema
is defined in a local schema file named `myschema.json`.

        bq load \
        --source_format=CSV \
        --time_partitioning_type=DAY \
        mydataset.mytable \
        gs://mybucket/mydata.csv \
        ./myschema.json

The following command loads data from `gs://mybucket/mydata.csv` into a new
partitioned table named `mytable` in `mydataset`. The table is partitioned
on the `mytimestamp` column. The schema is defined in a local schema file
named `myschema.json`.

        bq load \
        --source_format=CSV \
        --time_partitioning_field mytimestamp \
        mydataset.mytable \
        gs://mybucket/mydata.csv \
        ./myschema.json

The following command loads data from `gs://mybucket/mydata.csv` into a
table named `mytable` in `mydataset`. The schema is auto-detected.

        bq load \
        --autodetect \
        --source_format=CSV \
        mydataset.mytable \
        gs://mybucket/mydata.csv

The following command loads data from `gs://mybucket/mydata.csv` into a
table named `mytable` in `mydataset`. The schema is defined inline in the
format `field:data_type,field:data_type`.

        bq load \
        --source_format=CSV \
        mydataset.mytable \
        gs://mybucket/mydata.csv \
        qtr:STRING,sales:FLOAT,year:STRING

> [!NOTE]
> **Note:** When you specify the schema using the bq command-line tool, you cannot include a `RECORD` ([`STRUCT`](https://docs.cloud.google.com/bigquery/docs/loading-data-cloud-storage-csv#struct-type)) type, you cannot include a field description, and you cannot specify the field mode. All field modes default to `NULLABLE`. To include field descriptions, modes, and `RECORD` types, supply a [JSON schema file](https://docs.cloud.google.com/bigquery/docs/schemas#specifying_a_json_schema_file) instead.

The following command loads data from multiple files in `gs://mybucket/`
into a table named `mytable` in `mydataset`. The Cloud Storage URI uses a
wildcard. The schema is auto-detected.

        bq load \
        --autodetect \
        --source_format=CSV \
        mydataset.mytable \
        gs://mybucket/mydata*.csv

The following command loads data from multiple files in `gs://mybucket/`
into a table named `mytable` in `mydataset`. The command includes a comma-separated
list of Cloud Storage URIs with wildcards. The schema is
defined in a local schema file named `myschema.json`.

        bq load \
        --source_format=CSV \
        mydataset.mytable \
        "gs://mybucket/00/*.csv","gs://mybucket/01/*.csv" \
        ./myschema.json

### API

1. Create a `load` job that points to the source data in Cloud Storage.

2. (Optional) Specify your [location](https://docs.cloud.google.com/bigquery/docs/dataset-locations) in
   the `location` property in the `jobReference` section of the [job resource](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs).

3. The `source URIs` property must be fully-qualified, in the format
   `gs://bucket/object`.
   Each URI can contain one '\*' [wildcard character](https://docs.cloud.google.com/bigquery/docs/batch-loading-data#load-wildcards).

4. Specify the CSV data format by setting the `sourceFormat` property to
   `CSV`.

5. To check the job status, call
   [`jobs.get(job_id*)`](https://docs.cloud.google.com/bigquery/docs/reference/v2/jobs/get),
   where <var translate="no">job_id</var> is the ID of the job returned by the initial
   request.

   - If `status.state = DONE`, the job completed successfully.
   - If the `status.errorResult` property is present, the request failed, and that object will include information describing what went wrong. When a request fails, no table is created and no data is loaded.
   - If `status.errorResult` is absent, the job finished successfully, although there might have been some nonfatal errors, such as problems importing a few rows. Nonfatal errors are listed in the returned job object's `status.errors` property.

**API notes:**

- Load jobs are atomic and consistent; if a load job fails, none of the data
  is available, and if a load job succeeds, all of the data is available.

- As a best practice, generate a unique ID and pass it as
  `jobReference.jobId` when calling `jobs.insert` to create a load job. This
  approach is more robust to network failure because the client can poll or
  retry on the known job ID.

- Calling `jobs.insert` on a given job ID is idempotent. You can retry as
  many times as you like on the same job ID, and at most one of those
  operations will succeed.

### C#


Before trying this sample, follow the C# setup instructions in the
[BigQuery quickstart using
client libraries](https://docs.cloud.google.com/bigquery/docs/quickstarts/quickstart-client-libraries).


For more information, see the
[BigQuery C# API
reference documentation](https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest).


To authenticate to BigQuery, set up Application Default Credentials.
For more information, see

[Set up authentication for client libraries](https://docs.cloud.google.com/bigquery/docs/authentication#client-libs).


    using https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.html;
    using System;

    public class BigQueryLoadTableGcsCsv
    {
        public void LoadTableGcsCsv(
            string projectId = "your-project-id",
            string datasetId = "your_dataset_id"
        )
        {
            https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryClient.html client = https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryClient.html.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryClient.html#Google_Cloud_BigQuery_V2_BigQueryClient_Create_System_String_Google_Apis_Auth_OAuth2_GoogleCredential_(projectId);
            var gcsURI = "gs://cloud-samples-data/bigquery/us-states/us-states.csv";
            var dataset = client.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryClient.html#Google_Cloud_BigQuery_V2_BigQueryClient_GetDataset_Google_Apis_Bigquery_v2_Data_DatasetReference_Google_Cloud_BigQuery_V2_GetDatasetOptions_(datasetId);
            var schema = new https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.TableSchemaBuilder.html {
                { "name", https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryDbType.html.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryDbType.html#Google_Cloud_BigQuery_V2_BigQueryDbType_String },
                { "post_abbr", https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryDbType.html.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryDbType.html#Google_Cloud_BigQuery_V2_BigQueryDbType_String }
            }.Build();
            var destinationTableRef = dataset.GetTableReference(
                tableId: "us_states");
            // Create job configuration
            var jobOptions = new https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.CreateLoadJobOptions.html()
            {
                // The source format defaults to CSV; line below is optional.
                SourceFormat = https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.FileFormat.html.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.FileFormat.html#Google_Cloud_BigQuery_V2_FileFormat_Csv,
                SkipLeadingRows = 1
            };
            // Create and run job
            var loadJob = client.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryClient.html#Google_Cloud_BigQuery_V2_BigQueryClient_CreateLoadJob_System_Collections_Generic_IEnumerable_System_String__Google_Apis_Bigquery_v2_Data_TableReference_Google_Apis_Bigquery_v2_Data_TableSchema_Google_Cloud_BigQuery_V2_CreateLoadJobOptions_(
                sourceUri: gcsURI, destination: destinationTableRef,
                schema: schema, options: jobOptions);
            loadJob = loadJob.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryJob.html#Google_Cloud_BigQuery_V2_BigQueryJob_PollUntilCompleted_Google_Cloud_BigQuery_V2_GetJobOptions_Google_Api_Gax_PollSettings_().ThrowOnAnyError();  // Waits for the job to complete.

            // Display the number of rows uploaded
            https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryTable.html table = client.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryClient.html#Google_Cloud_BigQuery_V2_BigQueryClient_GetTable_Google_Apis_Bigquery_v2_Data_TableReference_Google_Cloud_BigQuery_V2_GetTableOptions_(destinationTableRef);
            Console.WriteLine(
                $"Loaded {table.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryTable.html#Google_Cloud_BigQuery_V2_BigQueryTable_Resource.NumRows} rows to {table.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryTable.html#Google_Cloud_BigQuery_V2_BigQueryTable_FullyQualifiedId}");
        }
    }

<br />

### Go


Before trying this sample, follow the Go setup instructions in the
[BigQuery quickstart using
client libraries](https://docs.cloud.google.com/bigquery/docs/quickstarts/quickstart-client-libraries).


For more information, see the
[BigQuery Go API
reference documentation](https://godoc.org/cloud.google.com/go/bigquery).


To authenticate to BigQuery, set up Application Default Credentials.
For more information, see

[Set up authentication for client libraries](https://docs.cloud.google.com/bigquery/docs/authentication#client-libs).

    import (
    	"context"
    	"fmt"

    	"cloud.google.com/go/bigquery"
    )

    // importCSVExplicitSchema demonstrates loading CSV data from Cloud Storage into a BigQuery
    // table and providing an explicit schema for the data.
    func importCSVExplicitSchema(projectID, datasetID, tableID string) error {
    	// projectID := "my-project-id"
    	// datasetID := "mydataset"
    	// tableID := "mytable"
    	ctx := context.Background()
    	client, err := bigquery.NewClient(ctx, projectID)
    	if err != nil {
    		return fmt.Errorf("bigquery.NewClient: %v", err)
    	}
    	defer client.Close()

    	gcsRef := bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_GCSReference_NewGCSReference("gs://cloud-samples-data/bigquery/us-states/us-states.csv")
    	gcsRef.SkipLeadingRows = 1
    	gcsRef.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_Schema = bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_Schema{
    		{Name: "name", Type: bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_StringFieldType_BytesFieldType_IntegerFieldType_FloatFieldType_BooleanFieldType_TimestampFieldType_RecordFieldType_DateFieldType_TimeFieldType_DateTimeFieldType_NumericFieldType_GeographyFieldType_BigNumericFieldType_IntervalFieldType_JSONFieldType_RangeFieldType},
    		{Name: "post_abbr", Type: bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_StringFieldType_BytesFieldType_IntegerFieldType_FloatFieldType_BooleanFieldType_TimestampFieldType_RecordFieldType_DateFieldType_TimeFieldType_DateTimeFieldType_NumericFieldType_GeographyFieldType_BigNumericFieldType_IntervalFieldType_JSONFieldType_RangeFieldType},
    	}
    	loader := client.Dataset(datasetID).Table(tableID).https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_Table_LoaderFrom(gcsRef)
    	loader.WriteDisposition = bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_WriteAppend_WriteTruncate_WriteTruncateData_WriteEmpty

    	job, err := loader.Run(ctx)
    	if err != nil {
    		return err
    	}
    	status, err := job.Wait(ctx)
    	if err != nil {
    		return err
    	}

    	if status.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_JobStatus_Err() != nil {
    		return fmt.Errorf("job completed with error: %v", status.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_JobStatus_Err())
    	}
    	return nil
    }

<br />

### Java


Before trying this sample, follow the Java setup instructions in the
[BigQuery quickstart using
client libraries](https://docs.cloud.google.com/bigquery/docs/quickstarts/quickstart-client-libraries).


For more information, see the
[BigQuery Java API
reference documentation](https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/overview).


To authenticate to BigQuery, set up Application Default Credentials.
For more information, see

[Set up authentication for client libraries](https://docs.cloud.google.com/bigquery/docs/authentication#client-libs).

    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryException.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryOptions.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.CsvOptions.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Field.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Job.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.JobInfo.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.LoadJobConfiguration.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Schema.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.StandardSQLTypeName.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableId.html;

    // Sample to load CSV data from Cloud Storage into a new BigQuery table
    public class LoadCsvFromGcs {

      public static void runLoadCsvFromGcs() throws Exception {
        // TODO(developer): Replace these variables before running the sample.
        String datasetName = "MY_DATASET_NAME";
        String tableName = "MY_TABLE_NAME";
        String sourceUri = "gs://cloud-samples-data/bigquery/us-states/us-states.csv";
        https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Schema.html schema =
            https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Schema.html.of(
                https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Field.html.of("name", https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.StandardSQLTypeName.html.STRING),
                https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Field.html.of("post_abbr", https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.StandardSQLTypeName.html.STRING));
        loadCsvFromGcs(datasetName, tableName, sourceUri, schema);
      }

      public static void loadCsvFromGcs(
          String datasetName, String tableName, String sourceUri, https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Schema.html schema) {
        try {
          // Initialize client that will be used to send requests. This client only needs to be created
          // once, and can be reused for multiple requests.
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html bigquery = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryOptions.html.getDefaultInstance().getService();

          // Skip header row in the file.
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.CsvOptions.html csvOptions = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.CsvOptions.html.newBuilder().setSkipLeadingRows(1).build();

          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableId.html tableId = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableId.html.of(datasetName, tableName);
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.LoadJobConfiguration.html loadConfig =
              https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.LoadJobConfiguration.html.newBuilder(tableId, sourceUri, csvOptions).setSchema(schema).build();

          // Load data from a GCS CSV file into the table
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Job.html job = bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html#com_google_cloud_bigquery_BigQuery_create_com_google_cloud_bigquery_DatasetInfo_com_google_cloud_bigquery_BigQuery_DatasetOption____(JobInfo.of(loadConfig));
          // Blocks until this load table job completes its execution, either failing or succeeding.
          job = job.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Job.html#com_google_cloud_bigquery_Job_waitFor_com_google_cloud_bigquery_BigQueryRetryConfig_com_google_cloud_RetryOption____();
          if (job.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Job.html#com_google_cloud_bigquery_Job_isDone__()) {
            System.out.println("CSV from GCS successfully added during load append job");
          } else {
            System.out.println(
                "BigQuery was unable to load into the table due to an error:"
                    + job.getStatus().getError());
          }
        } catch (https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryException.html | InterruptedException e) {
          System.out.println("Column not added during load append \n" + e.toString());
        }
      }
    }

<br />

### Node.js


Before trying this sample, follow the Node.js setup instructions in the
[BigQuery quickstart using
client libraries](https://docs.cloud.google.com/bigquery/docs/quickstarts/quickstart-client-libraries).


For more information, see the
[BigQuery Node.js API
reference documentation](https://googleapis.dev/nodejs/bigquery/latest/index.html).


To authenticate to BigQuery, set up Application Default Credentials.
For more information, see

[Set up authentication for client libraries](https://docs.cloud.google.com/bigquery/docs/authentication#client-libs).

    // Import the Google Cloud client libraries
    const {BigQuery} = require('https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/overview.html');
    const {Storage} = require('https://docs.cloud.google.com/nodejs/docs/reference/storage/latest/overview.html');

    // Instantiate clients
    const bigquery = new https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/bigquery/bigquery.html();
    const storage = new https://docs.cloud.google.com/nodejs/docs/reference/storage-control/latest/storage-control/protos.google.storage.v2.storage-class.html();

    /**
     * This sample loads the CSV file at
     * https://storage.googleapis.com/cloud-samples-data/bigquery/us-states/us-states.csv
     *
     * TODO(developer): Replace the following lines with the path to your file.
     */
    const bucketName = 'cloud-samples-data';
    const filename = 'bigquery/us-states/us-states.csv';

    async function loadCSVFromGCS() {
      // Imports a GCS file into a table with manually defined schema.

      /**
       * TODO(developer): Uncomment the following lines before running the sample.
       */
      // const datasetId = 'my_dataset';
      // const tableId = 'my_table';

      // Configure the load job. For full list of options, see:
      // https://cloud.google.com/bigquery/docs/reference/rest/v2/Job#JobConfigurationLoad
      const metadata = {
        sourceFormat: 'CSV',
        skipLeadingRows: 1,
        schema: {
          fields: [
            {name: 'name', type: 'STRING'},
            {name: 'post_abbr', type: 'STRING'},
          ],
        },
        location: 'US',
      };

      // Load data from a Google Cloud Storage file into the table
      const [job] = await bigquery
        .dataset(datasetId)
        .table(tableId)
        .https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/bigquery/table.html(storage.bucket(bucketName).file(filename), metadata);

      // load() waits for the job to finish
      console.log(`Job ${https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/bigquery/bigquery.html.id} completed.`);

      // Check the job's status for errors
      const errors = https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/bigquery/bigquery.html.status.errors;
      if (errors && errors.length > 0) {
        throw errors;
      }
    }

<br />

### PHP


Before trying this sample, follow the PHP setup instructions in the
[BigQuery quickstart using
client libraries](https://docs.cloud.google.com/bigquery/docs/quickstarts/quickstart-client-libraries).


For more information, see the
[BigQuery PHP API
reference documentation](https://docs.cloud.google.com/php/docs/reference/cloud-bigquery/latest/BigQueryClient).


To authenticate to BigQuery, set up Application Default Credentials.
For more information, see

[Set up authentication for client libraries](https://docs.cloud.google.com/bigquery/docs/authentication#client-libs).

    use Google\Cloud\BigQuery\BigQueryClient;
    use Google\Cloud\Core\ExponentialBackoff;

    /** Uncomment and populate these variables in your code */
    // $projectId  = 'The Google project ID';
    // $datasetId  = 'The BigQuery dataset ID';

    // instantiate the bigquery table service
    $bigQuery = new BigQueryClient([
        'projectId' => $projectId,
    ]);
    $dataset = $bigQuery->dataset($datasetId);
    $table = $dataset->table('us_states');

    // create the import job
    $gcsUri = 'gs://cloud-samples-data/bigquery/us-states/us-states.csv';
    $schema = [
        'fields' => [
            ['name' => 'name', 'type' => 'string'],
            ['name' => 'post_abbr', 'type' => 'string']
        ]
    ];
    $loadConfig = $table->loadFromStorage($gcsUri)->schema($schema)->skipLeadingRows(1);
    $job = $table->runJob($loadConfig);
    // poll the job until it is complete
    $backoff = new ExponentialBackoff(10);
    $backoff->execute(function () use ($job) {
        print('Waiting for job to complete' . PHP_EOL);
        $job->reload();
        if (!$job->isComplete()) {
            throw new Exception('Job has not yet completed', 500);
        }
    });
    // check if the job has errors
    if (isset($job->info()['status']['errorResult'])) {
        $error = $job->info()['status']['errorResult']['message'];
        printf('Error running job: %s' . PHP_EOL, $error);
    } else {
        print('Data imported successfully' . PHP_EOL);
    }

<br />

### Python


Before trying this sample, follow the Python setup instructions in the
[BigQuery quickstart using
client libraries](https://docs.cloud.google.com/bigquery/docs/quickstarts/quickstart-client-libraries).


For more information, see the
[BigQuery Python API
reference documentation](https://docs.cloud.google.com/python/docs/reference/bigquery/latest).


To authenticate to BigQuery, set up Application Default Credentials.
For more information, see

[Set up authentication for client libraries](https://docs.cloud.google.com/bigquery/docs/authentication#client-libs).

Use the
[Client.load_table_from_uri()](https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client#google_cloud_bigquery_client_Client_load_table_from_uri)
method to load data from a CSV file in Cloud Storage. Supply an explicit
schema definition by setting the
[LoadJobConfig.schema](https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.job.LoadJobConfig#google_cloud_bigquery_job_LoadJobConfig_schema)
property to a list of
[SchemaField](https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.schema.SchemaField#google_cloud_bigquery_schema_SchemaField)
objects.

    from google.cloud import https://docs.cloud.google.com/python/docs/reference/bigquery/latest

    # Construct a BigQuery client object.
    client = https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html()

    # TODO(developer): Set table_id to the ID of the table to create.
    # table_id = "your-project.your_dataset.your_table_name"

    job_config = https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.job.LoadJobConfig.html(
        schema=[
            https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.schema.SchemaField.html("name", "STRING"),
            https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.schema.SchemaField.html("post_abbr", "STRING"),
        ],
        skip_leading_rows=1,
        # The source format defaults to CSV, so the line below is optional.
        source_format=https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.enums.SourceFormat.html.CSV,
    )
    uri = "gs://cloud-samples-data/bigquery/us-states/us-states.csv"

    load_job = client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_load_table_from_uri(
        uri, table_id, job_config=job_config
    )  # Make an API request.

    load_job.result()  # Waits for the job to complete.

    destination_table = client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_get_table(table_id)  # Make an API request.
    print("Loaded {} rows.".format(destination_table.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.table.Table.html#google_cloud_bigquery_table_Table_num_rows))

<br />

### Ruby


Before trying this sample, follow the Ruby setup instructions in the
[BigQuery quickstart using
client libraries](https://docs.cloud.google.com/bigquery/docs/quickstarts/quickstart-client-libraries).


For more information, see the
[BigQuery Ruby API
reference documentation](https://googleapis.dev/ruby/google-cloud-bigquery/latest/Google/Cloud/Bigquery.html).


To authenticate to BigQuery, set up Application Default Credentials.
For more information, see

[Set up authentication for client libraries](https://docs.cloud.google.com/bigquery/docs/authentication#client-libs).

    require "google/cloud/bigquery"

    def load_table_gcs_csv dataset_id = "your_dataset_id"
      bigquery = Google::Cloud::https://docs.cloud.google.com/ruby/docs/reference/google-cloud-bigquery-connection/latest/Google-Cloud-Bigquery.html.https://docs.cloud.google.com/ruby/docs/reference/google-cloud-bigquery/latest/Google-Cloud-Bigquery.html
      dataset  = bigquery.dataset dataset_id
      gcs_uri  = "gs://cloud-samples-data/bigquery/us-states/us-states.csv"
      table_id = "us_states"

      load_job = dataset.load_job table_id, gcs_uri, skip_leading: 1 do |schema|
        schema.string "name"
        schema.string "post_abbr"
      end
      puts "Starting job #{load_job.https://docs.cloud.google.com/ruby/docs/reference/google-cloud-bigquery/latest/Google-Cloud-Bigquery-Job.html}"

      load_job.wait_until_done! # Waits for table load to complete.
      puts "Job finished."

      table = dataset.table table_id
      puts "Loaded #{table.https://docs.cloud.google.com/ruby/docs/reference/google-cloud-bigquery/latest/Google-Cloud-Bigquery-Table.html} rows to table #{table.id}"
    end

<br />

## Loading CSV data into a table that uses column-based time partitioning

To load CSV data from Cloud Storage into a BigQuery table
that uses column-based time partitioning:

### Go


Before trying this sample, follow the Go setup instructions in the
[BigQuery quickstart using
client libraries](https://docs.cloud.google.com/bigquery/docs/quickstarts/quickstart-client-libraries).


For more information, see the
[BigQuery Go API
reference documentation](https://godoc.org/cloud.google.com/go/bigquery).


To authenticate to BigQuery, set up Application Default Credentials.
For more information, see

[Set up authentication for client libraries](https://docs.cloud.google.com/bigquery/docs/authentication#client-libs).


    import (
    	"context"
    	"fmt"
    	"time"

    	"cloud.google.com/go/bigquery"
    )

    // importPartitionedTable demonstrates specifing time partitioning for a BigQuery table when loading
    // CSV data from Cloud Storage.
    func importPartitionedTable(projectID, destDatasetID, destTableID string) error {
    	// projectID := "my-project-id"
    	// datasetID := "mydataset"
    	// tableID := "mytable"
    	ctx := context.Background()
    	client, err := bigquery.NewClient(ctx, projectID)
    	if err != nil {
    		return fmt.Errorf("bigquery.NewClient: %v", err)
    	}
    	defer client.Close()

    	gcsRef := bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_GCSReference_NewGCSReference("gs://cloud-samples-data/bigquery/us-states/us-states-by-date.csv")
    	gcsRef.SkipLeadingRows = 1
    	gcsRef.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_Schema = bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_Schema{
    		{Name: "name", Type: bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_StringFieldType_BytesFieldType_IntegerFieldType_FloatFieldType_BooleanFieldType_TimestampFieldType_RecordFieldType_DateFieldType_TimeFieldType_DateTimeFieldType_NumericFieldType_GeographyFieldType_BigNumericFieldType_IntervalFieldType_JSONFieldType_RangeFieldType},
    		{Name: "post_abbr", Type: bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_StringFieldType_BytesFieldType_IntegerFieldType_FloatFieldType_BooleanFieldType_TimestampFieldType_RecordFieldType_DateFieldType_TimeFieldType_DateTimeFieldType_NumericFieldType_GeographyFieldType_BigNumericFieldType_IntervalFieldType_JSONFieldType_RangeFieldType},
    		{Name: "date", Type: bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_StringFieldType_BytesFieldType_IntegerFieldType_FloatFieldType_BooleanFieldType_TimestampFieldType_RecordFieldType_DateFieldType_TimeFieldType_DateTimeFieldType_NumericFieldType_GeographyFieldType_BigNumericFieldType_IntervalFieldType_JSONFieldType_RangeFieldType},
    	}
    	loader := client.Dataset(destDatasetID).Table(destTableID).https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_Table_LoaderFrom(gcsRef)
    	loader.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_TimePartitioning = &bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_TimePartitioning{
    		Field:      "date",
    		Expiration: 90 * 24 * time.Hour,
    	}
    	loader.WriteDisposition = bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_WriteAppend_WriteTruncate_WriteTruncateData_WriteEmpty

    	job, err := loader.Run(ctx)
    	if err != nil {
    		return err
    	}
    	status, err := job.Wait(ctx)
    	if err != nil {
    		return err
    	}

    	if status.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_JobStatus_Err() != nil {
    		return fmt.Errorf("job completed with error: %v", status.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_JobStatus_Err())
    	}
    	return nil
    }

### Java


Before trying this sample, follow the Java setup instructions in the
[BigQuery quickstart using
client libraries](https://docs.cloud.google.com/bigquery/docs/quickstarts/quickstart-client-libraries).


For more information, see the
[BigQuery Java API
reference documentation](https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/overview).


To authenticate to BigQuery, set up Application Default Credentials.
For more information, see

[Set up authentication for client libraries](https://docs.cloud.google.com/bigquery/docs/authentication#client-libs).

    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryException.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryOptions.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Field.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.FormatOptions.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Job.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.JobId.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.JobInfo.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.LoadJobConfiguration.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Schema.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.StandardSQLTypeName.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableId.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TimePartitioning.html;
    import java.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.QueryParameterValue.html#com_google_cloud_bigquery_QueryParameterValue_time_java_lang_String_.Duration;
    import java.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.QueryParameterValue.html#com_google_cloud_bigquery_QueryParameterValue_time_java_lang_String_.temporal.ChronoUnit;
    import java.util.UUID;

    public class LoadPartitionedTable {

      public static void runLoadPartitionedTable() throws Exception {
        // TODO(developer): Replace these variables before running the sample.
        String datasetName = "MY_DATASET_NAME";
        String tableName = "MY_TABLE_NAME";
        String sourceUri = "/path/to/file.csv";
        loadPartitionedTable(datasetName, tableName, sourceUri);
      }

      public static void loadPartitionedTable(String datasetName, String tableName, String sourceUri)
          throws Exception {
        try {
          // Initialize client that will be used to send requests. This client only needs to be created
          // once, and can be reused for multiple requests.
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html bigquery = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryOptions.html.getDefaultInstance().getService();

          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableId.html tableId = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableId.html.of(datasetName, tableName);

          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Schema.html schema =
              https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Schema.html.of(
                  https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Field.html.of("name", https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.StandardSQLTypeName.html.STRING),
                  https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Field.html.of("post_abbr", https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.StandardSQLTypeName.html.STRING),
                  https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Field.html.of("date", https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.StandardSQLTypeName.html.DATE));

          // Configure time partitioning. For full list of options, see:
          // https://cloud.google.com/bigquery/docs/reference/rest/v2/tables#TimePartitioning
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TimePartitioning.html partitioning =
              https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TimePartitioning.html.newBuilder(https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TimePartitioning.html.Type.DAY)
                  .setField("date")
                  .https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TimePartitioning.Builder.html#com_google_cloud_bigquery_TimePartitioning_Builder_setExpirationMs_java_lang_Long_(Duration.of(90, ChronoUnit.DAYS).toMillis())
                  .build();

          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.LoadJobConfiguration.html loadJobConfig =
              https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.LoadJobConfiguration.html.builder(tableId, sourceUri)
                  .setFormatOptions(https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.FormatOptions.html.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.FormatOptions.html#com_google_cloud_bigquery_FormatOptions_csv__())
                  .setSchema(schema)
                  .setTimePartitioning(partitioning)
                  .build();

          // Create a job ID so that we can safely retry.
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.JobId.html jobId = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.JobId.html.of(UUID.randomUUID().toString());
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Job.html loadJob = bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html#com_google_cloud_bigquery_BigQuery_create_com_google_cloud_bigquery_DatasetInfo_com_google_cloud_bigquery_BigQuery_DatasetOption____(JobInfo.newBuilder(loadJobConfig).setJobId(jobId).build());

          // Load data from a GCS parquet file into the table
          // Blocks until this load table job completes its execution, either failing or succeeding.
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Job.html completedJob = loadJob.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Job.html#com_google_cloud_bigquery_Job_waitFor_com_google_cloud_bigquery_BigQueryRetryConfig_com_google_cloud_RetryOption____();

          // Check for errors
          if (completedJob == null) {
            throw new Exception("Job not executed since it no longer exists.");
          } else if (completedJob.getStatus().getError() != null) {
            // You can also look at queryJob.getStatus().getExecutionErrors() for all
            // errors, not just the latest one.
            throw new Exception(
                "BigQuery was unable to load into the table due to an error: \n"
                    + loadJob.getStatus().getError());
          }
          System.out.println("Data successfully loaded into time partitioned table during load job");
        } catch (https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryException.html | InterruptedException e) {
          System.out.println(
              "Data not loaded into time partitioned table during load job \n" + e.toString());
        }
      }
    }

### Node.js


Before trying this sample, follow the Node.js setup instructions in the
[BigQuery quickstart using
client libraries](https://docs.cloud.google.com/bigquery/docs/quickstarts/quickstart-client-libraries).


For more information, see the
[BigQuery Node.js API
reference documentation](https://googleapis.dev/nodejs/bigquery/latest/index.html).


To authenticate to BigQuery, set up Application Default Credentials.
For more information, see

[Set up authentication for client libraries](https://docs.cloud.google.com/bigquery/docs/authentication#client-libs).

    // Import the Google Cloud client libraries
    const {BigQuery} = require('https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/overview.html');
    const {Storage} = require('https://docs.cloud.google.com/nodejs/docs/reference/storage/latest/overview.html');

    // Instantiate clients
    const bigquery = new https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/bigquery/bigquery.html();
    const storage = new https://docs.cloud.google.com/nodejs/docs/reference/storage-control/latest/storage-control/protos.google.storage.v2.storage-class.html();

    /**
     * This sample loads the CSV file at
     * https://storage.googleapis.com/cloud-samples-data/bigquery/us-states/us-states.csv
     *
     * TODO(developer): Replace the following lines with the path to your file.
     */
    const bucketName = 'cloud-samples-data';
    const filename = 'bigquery/us-states/us-states-by-date.csv';

    async function loadTablePartitioned() {
      // Load data into a table that uses column-based time partitioning.

      /**
       * TODO(developer): Uncomment the following lines before running the sample.
       */
      // const datasetId = 'my_dataset';
      // const tableId = 'my_new_table';

      // Configure the load job. For full list of options, see:
      // https://cloud.google.com/bigquery/docs/reference/rest/v2/Job#JobConfigurationLoad
      const partitionConfig = {
        type: 'DAY',
        expirationMs: '7776000000', // 90 days
        field: 'https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/bigquery/bigquery.html',
      };

      const metadata = {
        sourceFormat: 'CSV',
        skipLeadingRows: 1,
        schema: {
          fields: [
            {name: 'name', type: 'STRING'},
            {name: 'post_abbr', type: 'STRING'},
            {name: 'https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/bigquery/bigquery.html', type: 'DATE'},
          ],
        },
        location: 'US',
        timePartitioning: partitionConfig,
      };

      // Load data from a Google Cloud Storage file into the table
      const [job] = await bigquery
        .dataset(datasetId)
        .table(tableId)
        .https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/bigquery/table.html(storage.bucket(bucketName).file(filename), metadata);

      // load() waits for the job to finish
      console.log(`Job ${https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/bigquery/bigquery.html.id} completed.`);
    }

### Python


Before trying this sample, follow the Python setup instructions in the
[BigQuery quickstart using
client libraries](https://docs.cloud.google.com/bigquery/docs/quickstarts/quickstart-client-libraries).


For more information, see the
[BigQuery Python API
reference documentation](https://docs.cloud.google.com/python/docs/reference/bigquery/latest).


To authenticate to BigQuery, set up Application Default Credentials.
For more information, see

[Set up authentication for client libraries](https://docs.cloud.google.com/bigquery/docs/authentication#client-libs).

    from google.cloud import https://docs.cloud.google.com/python/docs/reference/bigquery/latest

    # Construct a BigQuery client object.
    client = https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html()

    # TODO(developer): Set table_id to the ID of the table to create.
    # table_id = "your-project.your_dataset.your_table_name"

    job_config = https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.job.LoadJobConfig.html(
        schema=[
            https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.schema.SchemaField.html("name", "STRING"),
            https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.schema.SchemaField.html("post_abbr", "STRING"),
            https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.schema.SchemaField.html("date", "DATE"),
        ],
        skip_leading_rows=1,
        time_partitioning=https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.table.TimePartitioning.html(
            type_=https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.table.TimePartitioningType.html.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.table.TimePartitioningType.html#google_cloud_bigquery_table_TimePartitioningType_DAY,
            field="date",  # Name of the column to use for partitioning.
            expiration_ms=7776000000,  # 90 days.
        ),
    )
    uri = "gs://cloud-samples-data/bigquery/us-states/us-states-by-date.csv"

    load_job = client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_load_table_from_uri(
        uri, table_id, job_config=job_config
    )  # Make an API request.

    load_job.result()  # Wait for the job to complete.

    table = client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_get_table(table_id)
    print("Loaded {} rows to table {}".format(table.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.table.Table.html#google_cloud_bigquery_table_Table_num_rows, table_id))

<br />

## Appending to or overwriting a table with CSV data

You can load additional data into a table either from source files or by
appending query results.

In the Google Cloud console, use the **Write preference** option to specify
what action to take when you load data from a source file or from a query
result.

You have the following options when you load additional data into a table:

| Console option | bq tool flag | BigQuery API property | Description |
|---|---|---|---|
| Write if empty | Not supported | `WRITE_EMPTY` | Writes the data only if the table is empty. |
| Append to table | `--noreplace` or `--replace=false`; if `--[no]replace` is unspecified, the default is append | `WRITE_APPEND` | ([Default](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/Job#JobConfigurationLoad.FIELDS.write_disposition)) Appends the data to the end of the table. |
| Overwrite table | `--replace` or `--replace=true` | `WRITE_TRUNCATE` | Erases all existing data in a table before writing the new data. This action also deletes the table schema, row level security, and removes any Cloud KMS key. |

If you load data into an existing table, the load job can append the data or
overwrite the table.

> [!NOTE]
> **Note:** This page does not cover appending or overwriting partitioned tables. For information on appending and overwriting partitioned tables, see: [Appending to and overwriting partitioned table data](https://docs.cloud.google.com/bigquery/docs/managing-partitioned-table-data#append-overwrite).

### Console

1.
   In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Explorer**.
3. In the **Explorer** pane, expand your project, click **Datasets**, and then select a dataset.
4. In the **Dataset info** section, click **Create table**.
5. In the **Create table** pane, specify the following details:
   1. In the **Source** section, select **Google Cloud Storage** in the **Create table from** list. Then, do the following:
      1. Select a file from the Cloud Storage bucket, or enter the [Cloud Storage URI](https://docs.cloud.google.com/bigquery/docs/batch-loading-data#gcs-uri). You cannot include multiple URIs in the Google Cloud console, but [wildcards](https://docs.cloud.google.com/bigquery/docs/batch-loading-data#load-wildcards) are supported. The Cloud Storage bucket must be in the same location as the dataset that contains the table you want to create, append, or overwrite. ![select source file to create a BigQuery table](https://docs.cloud.google.com/static/bigquery/images/create-table-select-file.png)
      2. For **File format** , select **CSV**.

   > [!NOTE]
   > **Note:** It is possible to modify the table's schema when you append or overwrite it. For more information about supported schema changes during a load operation, see [Modifying table schemas](https://docs.cloud.google.com/bigquery/docs/managing-table-schemas).

   2. In the **Destination** section, specify the following details:
      1. For **Dataset**, select the dataset in which you want to create the table.
      2. In the **Table** field, enter the name of the table that you want to create.
      3. Verify that the **Table type** field is set to **Native table**.
   3. In the **Schema** section, enter the [schema](https://docs.cloud.google.com/bigquery/docs/schemas) definition. To enable the [auto detection](https://docs.cloud.google.com/bigquery/docs/schema-detect) of a schema, select **Auto detect** . You can enter schema information manually by using one of the following methods:
      - Option 1: Click **Edit as text** and paste the schema in the form of a JSON array. When you use a JSON array, you generate the schema using the same process as [creating a JSON schema file](https://docs.cloud.google.com/bigquery/docs/schemas#specifying_a_json_schema_file). You can view the schema of an existing table in JSON format by entering the following command:

        ```bash
            bq show --format=prettyjson dataset.table
            
        ```
      - Option 2: Click **Add field** and enter the table schema. Specify each field's **Name** , [**Type**](https://docs.cloud.google.com/bigquery/docs/schemas#standard_sql_data_types), and [**Mode**](https://docs.cloud.google.com/bigquery/docs/schemas#modes).
      **Note:** It is possible to modify the table's schema when you append or overwrite it. For more information about supported schema changes during a load operation, see [Modifying table schemas](https://docs.cloud.google.com/bigquery/docs/managing-table-schemas).
   4. Optional: Specify **Partition and cluster settings** . For more information, see [Creating partitioned tables](https://docs.cloud.google.com/bigquery/docs/creating-partitioned-tables) and [Creating and using clustered tables](https://docs.cloud.google.com/bigquery/docs/creating-clustered-tables). You cannot convert a table to a partitioned or clustered table by appending or overwriting it. The Google Cloud console does not support appending to or overwriting partitioned or clustered tables in a load job.
   5. Click **Advanced options** and do the following:
      - For **Write preference** , choose **Append to table** or **Overwrite
        table**.
      - For **Number of errors allowed** , accept the default value of `0` or enter the maximum number of rows containing errors that can be ignored. If the number of rows with errors exceeds this value, the job will result in an `invalid` message and fail. This option applies only to CSV and JSON files.
      - For **Time zone** , enter the default time zone that will apply when parsing timestamp values that have no specific time zone. Check [here](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#time_zone_name) for more valid time zone names. If this value is not present, the timestamp values without specific time zone is parsed using default time zone UTC.
      - For **Date Format** , enter the [format elements](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/format-elements#format_string_as_datetime) that define how the DATE values are formatted in the input files. This field expects SQL styles format (for example, `MM/DD/YYYY`). If this value is present, this format is the only compatible DATE format. [Schema autodetection](https://docs.cloud.google.com/bigquery/docs/schema-detect#date_and_time_values) will also decide DATE column type based on this format instead of the existing format. If this value is not present, the DATE field is parsed with the [default formats](https://docs.cloud.google.com/bigquery/docs/loading-data-cloud-storage-csv#data_types).
      - For **Datetime Format** , enter the [format elements](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/format-elements#format_string_as_datetime) that define how the DATETIME values are formatted in the input files. This field expects SQL styles format (for example, `MM/DD/YYYY HH24:MI:SS.FF3`). If this value is present, this format is the only compatible DATETIME format. [Schema autodetection](https://docs.cloud.google.com/bigquery/docs/schema-detect#date_and_time_values) will also decide DATETIME column type based on this format instead of the existing format. If this value is not present, the DATETIME field is parsed with the [default formats](https://docs.cloud.google.com/bigquery/docs/loading-data-cloud-storage-csv#data_types).
      - For **Time Format** , enter the [format elements](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/format-elements#format_string_as_datetime) that define how the TIME values are formatted in the input files. This field expects SQL styles format (for example, `HH24:MI:SS.FF3`). If this value is present, this format is the only compatible TIME format. [Schema autodetection](https://docs.cloud.google.com/bigquery/docs/schema-detect#date_and_time_values) will also decide TIME column type based on this format instead of the existing format. If this value is not present, the TIME field is parsed with the [default formats](https://docs.cloud.google.com/bigquery/docs/loading-data-cloud-storage-csv#data_types).
      - For **Timestamp Format** , enter the [format elements](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/format-elements#format_string_as_datetime) that define how the TIMESTAMP values are formatted in the input files. This field expects SQL styles format (for example, `MM/DD/YYYY HH24:MI:SS.FF3`). If this value is present, this format is the only compatible TIMESTAMP format. [Schema autodetection](https://docs.cloud.google.com/bigquery/docs/schema-detect#date_and_time_values) will also decide TIMESTAMP column type based on this format instead of the existing format. If this value is not present, the TIMESTAMP field is parsed with the [default formats](https://docs.cloud.google.com/bigquery/docs/loading-data-cloud-storage-csv#data_types).
      - If you want to ignore values in a row that are not present in the table's schema, then select **Unknown values**.
      - For **Field delimiter** , choose the character that separates the cells in your CSV file: **Comma** , **Tab** , **Pipe** , or **Custom** . If you choose **Custom** , enter the delimiter in the **Custom field delimiter** box. The default value is **Comma**.
      - For **Source column match**, choose one of the following strategies used to match the loaded columns to the schema.
        - `Default`: Default behavior is chosen based on how the schema is provided. If autodetect is enabled, then the default behavior is to match columns by name. Otherwise, the default is to match columns by position. This is done to keep the behavior backward-compatible.
        - `Position`: Matches columns by position, assuming that the columns are ordered the same way as the schema.
        - `Name`: Matches by name by reading the header row as the column names and reordering columns to match the field names in the schema. Column names are read from the last skipped row based on **Header rows to skip**.
      - For **Header rows to skip** , enter the number of header rows to skip at the top of the CSV file. The default value is `0`.
      - For **Quoted newlines** , check **Allow quoted newlines** to allow quoted data sections that contain newline characters in a CSV file. The default value is `false`.
      - For **Jagged rows** , check **Allow jagged rows** to accept rows in CSV files that are missing trailing optional columns. The missing values are treated as nulls. If unchecked, records with missing trailing columns are treated as bad records, and if there are too many bad records, an invalid error is returned in the job result. The default value is `false`.
      - For **Null markers**, enter a list of custom strings that represents a NULL value in CSV data.
      - For **Encryption** , click **Customer-managed key** to use a [Cloud Key Management Service key](https://docs.cloud.google.com/bigquery/docs/customer-managed-encryption). If you leave the **Google-managed key** setting, BigQuery [encrypts the data at rest](https://docs.cloud.google.com/docs/security/encryption/default-encryption).
   6. Click **Create table**.

### SQL

Use the
[`LOAD DATA` DDL statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/load-statements).
The following example appends a CSV file to the table `mytable`:

<br />

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, enter the following statement:

   ```googlesql
   LOAD DATA INTO mydataset.mytable
   FROM FILES (
     format = 'CSV',
     uris = ['gs://bucket/path/file.csv']);
   ```

   <br />

3. Click **Run**.

   <br />

For more information about how to run queries, see [Run an interactive query](https://docs.cloud.google.com/bigquery/docs/running-queries#queries).

### bq

Use the `bq load` command, specify `CSV` using the
`--source_format` flag, and include a [Cloud Storage URI](https://docs.cloud.google.com/bigquery/docs/batch-loading-data#gcs-uri).
You can include a single URI, a comma-separated list of URIs, or a URI
containing a [wildcard](https://docs.cloud.google.com/bigquery/docs/batch-loading-data#load-wildcards).

Supply the schema inline, in a schema definition file, or use
[schema auto-detect](https://docs.cloud.google.com/bigquery/docs/schema-detect). If you don't specify a
schema, and `--autodetect` is `false`, and the destination
table exists, then the schema of the destination table is used.

Specify the `--replace` flag to overwrite the
table. Use the `--noreplace` flag to append data to the table. If no flag is
specified, the default is to append data.

It is possible to modify the table's schema when you append or
overwrite it. For more information on supported schema changes during a load
operation, see [Modifying table schemas](https://docs.cloud.google.com/bigquery/docs/managing-table-schemas).

(Optional) Supply the `--location` flag and set the value to your
[location](https://docs.cloud.google.com/bigquery/docs/dataset-locations).

Other optional flags include:

- `--allow_jagged_rows`: When specified, accept rows in CSV files that are missing trailing optional columns. The missing values are treated as nulls. If unchecked, records with missing trailing columns are treated as bad records, and if there are too many bad records, an invalid error is returned in the job result. The default value is `false`.
- `--allow_quoted_newlines`: When specified, allows quoted data sections that contain newline characters in a CSV file. The default value is `false`.
- `--field_delimiter`: The character that indicates the boundary between columns in the data. Both `\t` and `tab` are allowed for tab delimiters. The default value is `,`.
- `--null_marker`: An optional custom string that represents a NULL value in CSV data.
- `--null_markers`: An optional comma-separated list of custom strings that represent NULL values in CSV data. This option cannot be used with `--null_marker` flag.
- `--source_column_match`: Specifies the strategy used to match loaded columns to the schema. You can specify `POSITION` to match loaded columns by position, assuming that the columns are ordered the same way as the schema. You can also specify `NAME` to match by name by reading the header row as the column names and reordering columns to match the field names in the schema. If this value is unspecified, then the default is based on how the schema is provided. If `--autodetect` is enabled, then the default behavior is to match columns by name. Otherwise, the default is to match columns by position.
- `--skip_leading_rows`: Specifies the number of header rows to skip at the top of the CSV file. The default value is `0`.
- `--quote`: The quote character to use to enclose records. The default value is `"`. To indicate no quote character, use an empty string.
- `--max_bad_records`: An integer that specifies the maximum number of bad records allowed before the entire job fails. The default value is `0`. At most, five errors of any type are returned regardless of the `--max_bad_records` value.
- `--ignore_unknown_values`: When specified, allows and ignores extra, unrecognized values in CSV or JSON data.
- `--time_zone`: An optional default time zone that will apply when parsing timestamp values that have no specific time zone in CSV or JSON data.
- `--date_format`: An optional custom string that defines how the DATE values are formatted in CSV or JSON data.
- `--datetime_format`: An optional custom string that defines how the DATETIME values are formatted in CSV or JSON data.
- `--time_format`: An optional custom string that defines how the TIME values are formatted in CSV or JSON data.
- `--timestamp_format`: An optional custom string that defines how the TIMESTAMP values are formatted in CSV or JSON data.
- `--autodetect`: When specified, enable schema auto-detection for CSV and JSON data.
- `--destination_kms_key`: The Cloud KMS key for encryption of the table data.

```bash
bq --location=location load \
--[no]replace \
--source_format=format \
dataset.table \
path_to_source \
schema
```

where:

- <var translate="no">location</var> is your [location](https://docs.cloud.google.com/bigquery/docs/dataset-locations). The `--location` flag is optional. You can set a default value for the location using the [.bigqueryrc file](https://docs.cloud.google.com/bigquery/docs/bq-command-line-tool#setting_default_values_for_command-line_flags).
- <var translate="no">format</var> is `CSV`.
- <var translate="no">dataset</var> is an existing dataset.
- <var translate="no">table</var> is the name of the table into which you're loading data.
- <var translate="no">path_to_source</var> is a fully-qualified [Cloud Storage URI](https://docs.cloud.google.com/bigquery/docs/batch-loading-data#gcs-uri) or a comma-separated list of URIs. [Wildcards](https://docs.cloud.google.com/bigquery/docs/batch-loading-data#load-wildcards) are also supported.
- <var translate="no">schema</var> is a valid schema. The schema can be a local JSON file, or it can be typed inline as part of the command. You can also use the `--autodetect` flag instead of supplying a schema definition.

Examples:

The following command loads data from `gs://mybucket/mydata.csv` and
overwrites a table named `mytable` in `mydataset`. The schema is defined
using [schema auto-detection](https://docs.cloud.google.com/bigquery/docs/schema-detect).

        bq load \
        --autodetect \
        --replace \
        --source_format=CSV \
        mydataset.mytable \
        gs://mybucket/mydata.csv

The following command loads data from `gs://mybucket/mydata.csv` and
appends data to a table named `mytable` in `mydataset`. The schema is
defined using a JSON schema file --- `myschema.json`.

        bq load \
        --noreplace \
        --source_format=CSV \
        mydataset.mytable \
        gs://mybucket/mydata.csv \
        ./myschema.json

### API

1. Create a `load` job that points to the source data in Cloud Storage.

2. (Optional) Specify your [location](https://docs.cloud.google.com/bigquery/docs/dataset-locations) in
   the `location` property in the `jobReference` section of the [job resource](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs).

3. The `source URIs` property
   must be fully-qualified, in the format
   `gs://bucket/object`. You can
   include multiple URIs as a comma-separated list. Note that
   [wildcards](https://docs.cloud.google.com/bigquery/docs/batch-loading-data#load-wildcards) are
   also supported.

4. Specify the data format by setting the
   `configuration.load.sourceFormat` property to `CSV`.

5. Specify the write preference by setting the
   `configuration.load.writeDisposition` property to `WRITE_TRUNCATE` or
   `WRITE_APPEND`.

### Go


Before trying this sample, follow the Go setup instructions in the
[BigQuery quickstart using
client libraries](https://docs.cloud.google.com/bigquery/docs/quickstarts/quickstart-client-libraries).


For more information, see the
[BigQuery Go API
reference documentation](https://godoc.org/cloud.google.com/go/bigquery).


To authenticate to BigQuery, set up Application Default Credentials.
For more information, see

[Set up authentication for client libraries](https://docs.cloud.google.com/bigquery/docs/authentication#client-libs).

    import (
    	"context"
    	"fmt"

    	"cloud.google.com/go/bigquery"
    )

    // importCSVTruncate demonstrates loading data from CSV data in Cloud Storage and overwriting/truncating
    // data in the existing table.
    func importCSVTruncate(projectID, datasetID, tableID string) error {
    	// projectID := "my-project-id"
    	// datasetID := "mydataset"
    	// tableID := "mytable"
    	ctx := context.Background()
    	client, err := bigquery.NewClient(ctx, projectID)
    	if err != nil {
    		return fmt.Errorf("bigquery.NewClient: %v", err)
    	}
    	defer client.Close()

    	gcsRef := bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_GCSReference_NewGCSReference("gs://cloud-samples-data/bigquery/us-states/us-states.csv")
    	gcsRef.SourceFormat = bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_CSV_Avro_JSON_DatastoreBackup_GoogleSheets_Bigtable_Parquet_ORC_TFSavedModel_XGBoostBooster_Iceberg
    	gcsRef.AutoDetect = true
    	gcsRef.SkipLeadingRows = 1
    	loader := client.Dataset(datasetID).Table(tableID).https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_Table_LoaderFrom(gcsRef)
    	loader.WriteDisposition = bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_WriteAppend_WriteTruncate_WriteTruncateData_WriteEmpty

    	job, err := loader.Run(ctx)
    	if err != nil {
    		return err
    	}
    	status, err := job.Wait(ctx)
    	if err != nil {
    		return err
    	}

    	if status.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_JobStatus_Err() != nil {
    		return fmt.Errorf("job completed with error: %v", status.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_JobStatus_Err())
    	}
    	return nil
    }

<br />

### Java


Before trying this sample, follow the Java setup instructions in the
[BigQuery quickstart using
client libraries](https://docs.cloud.google.com/bigquery/docs/quickstarts/quickstart-client-libraries).


For more information, see the
[BigQuery Java API
reference documentation](https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/overview).


To authenticate to BigQuery, set up Application Default Credentials.
For more information, see

[Set up authentication for client libraries](https://docs.cloud.google.com/bigquery/docs/authentication#client-libs).

    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryException.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryOptions.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.FormatOptions.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Job.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.JobInfo.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.JobInfo.html.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.JobInfo.WriteDisposition.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.LoadJobConfiguration.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableId.html;

    // Sample to overwrite the BigQuery table data by loading a CSV file from GCS
    public class LoadCsvFromGcsTruncate {

      public static void runLoadCsvFromGcsTruncate() throws Exception {
        // TODO(developer): Replace these variables before running the sample.
        String datasetName = "MY_DATASET_NAME";
        String tableName = "MY_TABLE_NAME";
        String sourceUri = "gs://cloud-samples-data/bigquery/us-states/us-states.csv";
        loadCsvFromGcsTruncate(datasetName, tableName, sourceUri);
      }

      public static void loadCsvFromGcsTruncate(String datasetName, String tableName, String sourceUri)
          throws Exception {
        try {
          // Initialize client that will be used to send requests. This client only needs to be created
          // once, and can be reused for multiple requests.
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html bigquery = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryOptions.html.getDefaultInstance().getService();

          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableId.html tableId = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableId.html.of(datasetName, tableName);

          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.LoadJobConfiguration.html configuration =
              https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.LoadJobConfiguration.html.builder(tableId, sourceUri)
                  .setFormatOptions(https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.FormatOptions.html.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.FormatOptions.html#com_google_cloud_bigquery_FormatOptions_csv__())
                  // Set the write disposition to overwrite existing table data
                  .setWriteDisposition(https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.JobInfo.WriteDisposition.html.WRITE_TRUNCATE)
                  .build();

          // For more information on Job see:
          // https://googleapis.dev/java/google-cloud-clients/latest/index.html?com/google/cloud/bigquery/package-summary.html
          // Load the table
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Job.html loadJob = bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html#com_google_cloud_bigquery_BigQuery_create_com_google_cloud_bigquery_DatasetInfo_com_google_cloud_bigquery_BigQuery_DatasetOption____(JobInfo.of(configuration));

          // Load data from a GCS parquet file into the table
          // Blocks until this load table job completes its execution, either failing or succeeding.
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Job.html completedJob = loadJob.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Job.html#com_google_cloud_bigquery_Job_waitFor_com_google_cloud_bigquery_BigQueryRetryConfig_com_google_cloud_RetryOption____();

          // Check for errors
          if (completedJob == null) {
            throw new Exception("Job not executed since it no longer exists.");
          } else if (completedJob.getStatus().getError() != null) {
            // You can also look at queryJob.getStatus().getExecutionErrors() for all
            // errors, not just the latest one.
            throw new Exception(
                "BigQuery was unable to load into the table due to an error: \n"
                    + loadJob.getStatus().getError());
          }
          System.out.println("Table is successfully overwritten by CSV file loaded from GCS");
        } catch (https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryException.html | InterruptedException e) {
          System.out.println("Column not added during load append \n" + e.toString());
        }
      }
    }

<br />

### Node.js


Before trying this sample, follow the Node.js setup instructions in the
[BigQuery quickstart using
client libraries](https://docs.cloud.google.com/bigquery/docs/quickstarts/quickstart-client-libraries).


For more information, see the
[BigQuery Node.js API
reference documentation](https://googleapis.dev/nodejs/bigquery/latest/index.html).


To authenticate to BigQuery, set up Application Default Credentials.
For more information, see

[Set up authentication for client libraries](https://docs.cloud.google.com/bigquery/docs/authentication#client-libs).

To replace the rows in an existing table, set the `writeDisposition`
value in the `metadata` parameter to `'WRITE_TRUNCATE'`.

    // Import the Google Cloud client libraries
    const {BigQuery} = require('https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/overview.html');
    const {Storage} = require('https://docs.cloud.google.com/nodejs/docs/reference/storage/latest/overview.html');

    // Instantiate clients
    const bigquery = new https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/bigquery/bigquery.html();
    const storage = new https://docs.cloud.google.com/nodejs/docs/reference/storage-control/latest/storage-control/protos.google.storage.v2.storage-class.html();

    /**
     * This sample loads the CSV file at
     * https://storage.googleapis.com/cloud-samples-data/bigquery/us-states/us-states.csv
     *
     * TODO(developer): Replace the following lines with the path to your file.
     */
    const bucketName = 'cloud-samples-data';
    const filename = 'bigquery/us-states/us-states.csv';

    async function loadCSVFromGCSTruncate() {
      /**
       * Imports a GCS file into a table and overwrites
       * table data if table already exists.
       */

      /**
       * TODO(developer): Uncomment the following lines before running the sample.
       */
      // const datasetId = 'my_dataset';
      // const tableId = 'my_table';

      // Configure the load job. For full list of options, see:
      // https://cloud.google.com/bigquery/docs/reference/rest/v2/Job#JobConfigurationLoad
      const metadata = {
        sourceFormat: 'CSV',
        skipLeadingRows: 1,
        schema: {
          fields: [
            {name: 'name', type: 'STRING'},
            {name: 'post_abbr', type: 'STRING'},
          ],
        },
        // Set the write disposition to overwrite existing table data.
        writeDisposition: 'WRITE_TRUNCATE',
        location: 'US',
      };

      // Load data from a Google Cloud Storage file into the table
      const [job] = await bigquery
        .dataset(datasetId)
        .table(tableId)
        .https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/bigquery/table.html(storage.bucket(bucketName).file(filename), metadata);
      // load() waits for the job to finish
      console.log(`Job ${https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/bigquery/bigquery.html.id} completed.`);

      // Check the job's status for errors
      const errors = https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/bigquery/bigquery.html.status.errors;
      if (errors && errors.length > 0) {
        throw errors;
      }
    }


Before trying this sample, follow the PHP setup instructions in the
[BigQuery quickstart using
client libraries](https://docs.cloud.google.com/bigquery/docs/quickstarts/quickstart-client-libraries).


For more information, see the
[BigQuery PHP API
reference documentation](https://docs.cloud.google.com/php/docs/reference/cloud-bigquery/latest/BigQueryClient).


To authenticate to BigQuery, set up Application Default Credentials.
For more information, see

[Set up authentication for client libraries](https://docs.cloud.google.com/bigquery/docs/authentication#client-libs).

    use Google\Cloud\BigQuery\BigQueryClient;
    use Google\Cloud\Core\ExponentialBackoff;

    /** Uncomment and populate these variables in your code */
    // $projectId = 'The Google project ID';
    // $datasetId = 'The BigQuery dataset ID';
    // $tableId = 'The BigQuery table ID';

    // instantiate the bigquery table service
    $bigQuery = new BigQueryClient([
        'projectId' => $projectId,
    ]);
    $table = $bigQuery->dataset($datasetId)->table($tableId);

    // create the import job
    $gcsUri = 'gs://cloud-samples-data/bigquery/us-states/us-states.csv';
    $loadConfig = $table->loadFromStorage($gcsUri)->skipLeadingRows(1)->writeDisposition('WRITE_TRUNCATE');
    $job = $table->runJob($loadConfig);

    // poll the job until it is complete
    $backoff = new ExponentialBackoff(10);
    $backoff->execute(function () use ($job) {
        print('Waiting for job to complete' . PHP_EOL);
        $job->reload();
        if (!$job->isComplete()) {
            throw new Exception('Job has not yet completed', 500);
        }
    });

    // check if the job has errors
    if (isset($job->info()['status']['errorResult'])) {
        $error = $job->info()['status']['errorResult']['message'];
        printf('Error running job: %s' . PHP_EOL, $error);
    } else {
        print('Data imported successfully' . PHP_EOL);
    }

<br />

### Python


Before trying this sample, follow the Python setup instructions in the
[BigQuery quickstart using
client libraries](https://docs.cloud.google.com/bigquery/docs/quickstarts/quickstart-client-libraries).


For more information, see the
[BigQuery Python API
reference documentation](https://docs.cloud.google.com/python/docs/reference/bigquery/latest).


To authenticate to BigQuery, set up Application Default Credentials.
For more information, see

[Set up authentication for client libraries](https://docs.cloud.google.com/bigquery/docs/authentication#client-libs).

To replace the rows in an existing table, set the [LoadJobConfig.write_disposition](https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.job.LoadJobConfig#google_cloud_bigquery_job_LoadJobConfig_write_disposition)
property to the [SourceFormat](https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.job.SourceFormat)
constant `WRITE_TRUNCATE`.


    import six

    from google.cloud import https://docs.cloud.google.com/python/docs/reference/bigquery/latest

    # Construct a BigQuery client object.
    client = https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html()

    # TODO(developer): Set table_id to the ID of the table to create.
    # table_id = "your-project.your_dataset.your_table_name

    job_config = https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.job.LoadJobConfig.html(
        schema=[
            https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.schema.SchemaField.html("name", "STRING"),
            https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.schema.SchemaField.html("post_abbr", "STRING"),
        ],
    )

    body = six.BytesIO(b"Washington,WA")
    client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_load_table_from_file(body, table_id, job_config=job_config).result()
    previous_rows = client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_get_table(table_id).https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.table.Table.html#google_cloud_bigquery_table_Table_num_rows
    assert previous_rows > 0

    job_config = https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.job.LoadJobConfig.html(
        write_disposition=https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.enums.WriteDisposition.html.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.enums.WriteDisposition.html#google_cloud_bigquery_enums_WriteDisposition_WRITE_TRUNCATE,
        source_format=https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.enums.SourceFormat.html.CSV,
        skip_leading_rows=1,
    )

    uri = "gs://cloud-samples-data/bigquery/us-states/us-states.csv"
    load_job = client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_load_table_from_uri(
        uri, table_id, job_config=job_config
    )  # Make an API request.

    load_job.result()  # Waits for the job to complete.

    destination_table = client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_get_table(table_id)
    print("Loaded {} rows.".format(destination_table.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.table.Table.html#google_cloud_bigquery_table_Table_num_rows))

<br />

## Loading hive-partitioned CSV data

BigQuery supports loading hive-partitioned CSV data stored on
Cloud Storage and will populate the hive partitioning columns as columns in
the destination BigQuery managed table. For more information, see
[Loading Externally Partitioned Data from Cloud Storage](https://docs.cloud.google.com/bigquery/docs/hive-partitioned-loads-gcs).

## Details of loading CSV data

This section describes how BigQuery handles various CSV formatting
options.

### Encoding

BigQuery expects CSV data to be UTF-8 encoded. If you have
CSV files with other supported encoding types, you should explicitly specify the
encoding so that BigQuery can properly convert the data to UTF-8.

BigQuery supports the following encoding types for CSV files:

- UTF-8
- ISO-8859-1
- UTF-16BE (UTF-16 Big Endian)
- UTF-16LE (UTF-16 Little Endian)
- UTF-32BE (UTF-32 Big Endian)
- UTF-32LE (UTF-32 Little Endian)

If you don't specify an encoding, or if you specify UTF-8 encoding when the CSV
file is not UTF-8 encoded, BigQuery attempts to convert the data
to UTF-8. Generally, if the CSV file is ISO-8859-1 encoded, your data will be
loaded successfully, but it may not exactly match what you expect. If the CSV
file is UTF-16BE, UTF-16LE, UTF-32BE, or UTF-32LE encoded, the load might fail.
To avoid unexpected failures, specify the correct encoding by using the
[`--encoding` flag](https://docs.cloud.google.com/bigquery/docs/loading-data-cloud-storage-csv#csv-options).

> [!NOTE]
> **Note:** If the CSV file is UTF-16BE, UTF-16LE, UTF-32BE, or UTF-32LE encoded, and the [`--allow_quoted_newlines` flag](https://docs.cloud.google.com/bigquery/docs/loading-data-cloud-storage-csv#csv-options) is set as `true`, then the CSV file has a maximum size limit of 1GB.

> [!NOTE]
> **Note:** By default, if the CSV file contains the ASCII `0` (NULL) character, you can't load the data into BigQuery. If you want to allow ASCII `0` and other ASCII control characters, then set `--preserve_ascii_control_characters=true` to your load jobs.

If BigQuery can't convert a character other than the ASCII `0`
character, BigQuery converts the character to the standard
Unicode replacement character: �.

### Field delimiters

Delimiters in CSV files can be any single-byte character. If the source file
uses ISO-8859-1 encoding, any character can be a delimiter. If the source file
uses UTF-8 encoding, any character in the decimal range 1-127 (U+0001-U+007F)
can be used without modification. You can insert an ISO-8859-1 character outside
of this range as a delimiter, and BigQuery will interpret it
correctly. However, if you use a multibyte character as a delimiter, some of the
bytes will be interpreted incorrectly as part of the field value.

Generally, it's a best practice to use a standard delimiter, such as a tab,
pipe, or comma. The default is a comma.

### Data types

**Boolean** . BigQuery can parse any of the following pairs for Boolean data: 1 or 0, true or false, t or f, yes or no, or y or n (all case insensitive). Schema [autodetection](https://docs.cloud.google.com/bigquery/docs/schema-detect) automatically detects any of these except 0 and 1.

<br />

**Bytes**. Columns with BYTES types must be encoded as Base64.

**Date** . Columns with DATE types must be in the format `YYYY-MM-DD`.

**Datetime** . Columns with DATETIME types must be in the format `YYYY-MM-DD
HH:MM:SS[.SSSSSS]`.

**Geography**. Columns with GEOGRAPHY types must contain strings in one of the
following formats:

- Well-known text (WKT)
- Well-known binary (WKB)
- GeoJSON

If you use WKB, the value should be hex encoded.

The following list shows examples of valid data:

- WKT: `POINT(1 2)`
- GeoJSON: `{ "type": "Point", "coordinates": [1, 2] }`
- Hex encoded WKB: `0101000000feffffffffffef3f0000000000000040`

Before loading GEOGRAPHY data, also read
[Loading geospatial data](https://docs.cloud.google.com/bigquery/docs/geospatial-data#loading_geospatial_data).

**Interval** . Columns with `INTERVAL` types must be in the format
`Y-M D H:M:S[.F]`, where:

- Y = Year. Supported range is 0-10,000.
- M = Month. Supported range is 1-12.
- D = Day. Supported range is 1-\[last day of the indicated month\].
- H = Hour.
- M = Minute.
- S = Second.
- \[.F\] = Fractions of a second up to six digits, with microsecond precision.

You can indicate a negative value by prepending a dash (-).

The following list shows examples of valid data:

- `10-6 0 0:0:0`
- `0-0 -5 0:0:0`
- `0-0 0 0:0:1.25`

To load INTERVAL data, you must use the
[`bq load`](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#bq_load) command and use
the `--schema` flag to specify a schema. You can't upload INTERVAL data by using
the console.

**JSON** . Quotes are escaped by using the two character sequence `""`. For more
information, see an example of
[loading JSON data from a CSV file](https://docs.cloud.google.com/bigquery/docs/json-data#load_from_csv_files)

**Time** . Columns with TIME types must be in the format `HH:MM:SS[.SSSSSS]`.

**Timestamp**. BigQuery accepts various timestamp formats.
The timestamp must include a date portion and a time portion.

- The date portion can be formatted as `YYYY-MM-DD` or `YYYY/MM/DD`.

- The timestamp portion must be formatted as `HH:MM[:SS[.SSSSSS]]` (seconds and
  fractions of seconds are optional).

- The date and time must be separated by a space or 'T'.

- Optionally, the date and time can be followed by a UTC offset or the UTC zone
  designator (`Z`). For more information, see
  [Time zones](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#time_zones).

For example, any of the following are valid timestamp values:

- `2018-08-19T12:11`
- `2018-08-19T12:11:35`
- `2018-08-19T12:11:35.22`
- `2018/08/19T12:11`
- `2018-07-05T12:54:00 UTC`
- `2018-08-19T07:11:35.220 -05:00`
- `2018-08-19T12:11:35.220Z`

If you provide a schema, BigQuery also accepts Unix epoch time for
timestamp values. However, schema autodetection doesn't detect this case, and
treats the value as a numeric or string type instead.

Examples of Unix epoch timestamp values:

- 1534680695
- 1.534680695e12

**RANGE** . Represented in CSV files in the format
`[LOWER_BOUND, UPPER_BOUND)`,
where `LOWER_BOUND` and `UPPER_BOUND`
are valid `DATE`, `DATETIME`, or `TIMESTAMP` strings. `NULL` and `UNBOUNDED`
represent unbounded start or end values.

> [!NOTE]
> **Note:** Since the range CSV format contains a comma, if the CSV delimiter is a comma, it must be surrounded by double quotes for the CSV file to be valid.

The following are examples of CSV values for `RANGE<DATE>`:

- `"[2020-01-01, 2021-01-01)"`
- `"[UNBOUNDED, 2021-01-01)"`
- `"[2020-03-01, NULL)"`
- `"[UNBOUNDED, UNBOUNDED)"`

### Schema auto-detection

This section describes the behavior of
[schema auto-detection](https://docs.cloud.google.com/bigquery/docs/schema-detect) when loading CSV files.

#### CSV delimiter

BigQuery detects the following delimiters:

- comma ( , )
- pipe ( \| )
- tab ( \\t )

#### CSV header

BigQuery infers headers by comparing the first row of the file
with other rows in the file. If the first line contains only strings, and the
other lines contain other data types, BigQuery assumes that the
first row is a header row. BigQuery assigns column names based on the field names in the header row. The names might be modified to meet the [naming rules](https://docs.cloud.google.com/bigquery/docs/schemas#column_names) for columns in BigQuery. For example, spaces will be replaced with underscores.

Otherwise, BigQuery assumes the first row is a data row, and
assigns generic column names such as `string_field_1`. Note that after a table
is created, the column names cannot be updated in the schema, although you can
[change the names
manually](https://docs.cloud.google.com/bigquery/docs/manually-changing-schemas#changing_a_columns_name)
after the table is created. Another option is to provide an explicit schema
instead of using autodetect.

You might have a CSV file with a header row, where all of the data fields are
strings. In that case, BigQuery will not automatically detect that
the first row is a header. Use the `--skip_leading_rows` option to skip the
header row. Otherwise, the header will be imported as data. Also consider
providing an explicit schema in this case, so that you can assign column names.

#### CSV quoted new lines

BigQuery detects quoted new line characters within a CSV field
and does not interpret the quoted new line character as a row boundary.

## Troubleshoot parsing errors

If there's a problem parsing your CSV files, then the
load job's [`errors`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/ErrorProto) resource is
populated with the error details.

Generally, these errors identify the start of the problematic line with a byte
offset. For uncompressed files you can use `gcloud storage` with the
`--recursive` argument to access the relevant line.

For example, you run the [`bq load` command](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#bq_load)
and receive an error:

```sh
bq load
    --skip_leading_rows=1 \
    --source_format=CSV \
    mydataset.mytable \
    gs://my-bucket/mytable.csv \
    'Number:INTEGER,Name:STRING,TookOffice:STRING,LeftOffice:STRING,Party:STRING'
```

The error in the output is similar to the following:

```sh
Waiting on bqjob_r5268069f5f49c9bf_0000018632e903d7_1 ... (0s)
Current status: DONE
BigQuery error in load operation: Error processing job
'myproject:bqjob_r5268069f5f49c9bf_0000018632e903d7_1': Error while reading
data, error message: Error detected while parsing row starting at position: 1405.
Error: Data between close quote character (") and field separator.
File: gs://my-bucket/mytable.csv
Failure details:
- *gs://my-bucket/mytable.csv*: Error while reading data,
error message: Error detected while parsing row starting at
position: *1405*. Error: Data between close quote character (") and
field separator. File: gs://my-bucket/mytable.csv
- Error while reading data, error message: CSV processing encountered
too many errors, giving up. Rows: 22; errors: 1; max bad: 0; error
percent: 0
```

Based on the preceding error, there's a format error in the file.
To view the file's content, run the [`gcloud storage cat` command](https://docs.cloud.google.com/sdk/gcloud/reference/storage/cat):

```sh
gcloud storage cat 1405-1505 gs://my-bucket/mytable.csv --recursive
```

The output is similar to the following:

```sh
16,Abraham Lincoln,"March 4, 1861","April 15, "1865,Republican
18,Ulysses S. Grant,"March 4, 1869",
...
```

Based on the output of the file, the problem is a misplaced quote in
`"April 15, "1865`.

### Compressed CSV files

Debugging parsing errors is more challenging for compressed CSV files, since
the reported byte offset refers to the location in the *uncompressed* file.
The following [`gcloud storage cat` command](https://docs.cloud.google.com/sdk/gcloud/reference/storage/cat)
streams the file from Cloud Storage, decompresses the file, identifies
the appropriate byte offset, and prints the line with the format error:

```sh
gcloud storage cat gs://my-bucket/mytable.csv.gz | gunzip - | tail -c +1406 | head -n 1
```

The output is similar to the following:

```sh
16,Abraham Lincoln,"March 4, 1861","April 15, "1865,Republican
```

## Troubleshoot quota errors

Use the information in this section to troubleshoot quota or limit errors
related to loading CSV files into BigQuery.

### Loading CSV files quota errors

If you load a large CSV file using the `bq load` command with the
[`--allow_quoted_newlines` flag](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#flags_and_arguments_9),
you might encounter this error.

**Error message**

    Input CSV files are not splittable and at least one of the files is larger than
    the maximum allowed size. Size is: ...

#### Resolution

To resolve this quota error, do the following:

- Set the `--allow_quoted_newlines` flag to `false`.
- Split the CSV file into smaller chunks that are each less than 4 GB.

For more information about limits that apply when you load data into
BigQuery, see [Load jobs](https://docs.cloud.google.com/bigquery/quotas#load_jobs).

## CSV options

To change how BigQuery parses CSV data, specify additional options
in the Google Cloud console, the bq command-line tool, or the API.

For more information on the CSV format, see
[RFC 4180](https://tools.ietf.org/html/rfc4180).

| CSV option | Console option | bq tool flag | BigQuery API property | Description |
|---|---|---|---|---|
| Field delimiter | Field delimiter: Comma, Tab, Pipe, Custom | `-F` or `--field_delimiter` | `fieldDelimiter` ([Java](https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.CsvOptions.Builder#com_google_cloud_bigquery_CsvOptions_Builder_setFieldDelimiter_java_lang_String_), [Python](https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.job.LoadJobConfig#google_cloud_bigquery_job_LoadJobConfig_field_delimiter)) | (Optional) The separator for fields in a CSV file. The separator can be any ISO-8859-1 single-byte character. BigQuery converts the string to ISO-8859-1 encoding, and uses the first byte of the encoded string to split the data in its raw, binary state. BigQuery also supports the escape sequence "\\t" to specify a tab separator. The default value is a comma (\`,\`). |
| Header rows | Header rows to skip | `--skip_leading_rows` | `skipLeadingRows` ([Java](https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.CsvOptions.Builder#com_google_cloud_bigquery_CsvOptions_Builder_setSkipLeadingRows_long_), [Python](https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.job.LoadJobConfig#google_cloud_bigquery_job_LoadJobConfig_skip_leading_rows)) | (Optional) An integer indicating the number of header rows in the source data. |
| Source column match | Source column match: Default, Position, Name | `--source_column_match` | `sourceColumnMatch` ([Java](https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.LoadJobConfiguration.Builder#com_google_cloud_bigquery_LoadJobConfiguration_Builder_setSourceColumnMatch_com_google_cloud_bigquery_LoadJobConfiguration_SourceColumnMatch_), [Python](https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.job.LoadJobConfig#google_cloud_bigquery_job_LoadJobConfig_source_column_match)) | (Optional) This controls the strategy used to match loaded columns to the schema. Supported values include: - `POSITION`: matches by position. This option assumes that the columns are ordered the same way as the schema. - `NAME`: matches by name. This option reads the header row as column names and reorders columns to match the field names in the schema. Column names are read from the last skipped row based on the `skipLeadingRows` property. If this value is unspecified, then the default is based on how the schema is provided. If autodetect is enabled, then the default behavior is to match columns by name. Otherwise, the default is to match columns by position. This is done to keep the behavior backward-compatible. |
| Number of bad records allowed | Number of errors allowed | `--max_bad_records` | `maxBadRecords` ([Java](https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.LoadJobConfiguration.Builder#com_google_cloud_bigquery_LoadJobConfiguration_Builder_setMaxBadRecords_java_lang_Integer_), [Python](https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.job.LoadJobConfig#google_cloud_bigquery_job_LoadJobConfig_max_bad_records)) | (Optional) The maximum number of bad records that BigQuery can ignore when running the job. If the number of bad records exceeds this value, an invalid error is returned in the job result. The default value is 0, which requires that all records are valid. |
| Newline characters | Allow quoted newlines | `--allow_quoted_newlines` | `allowQuotedNewlines` ([Java](https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.CsvOptions.Builder#com_google_cloud_bigquery_CsvOptions_Builder_setAllowQuotedNewLines_boolean_), [Python](https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.job.LoadJobConfig#google_cloud_bigquery_job_LoadJobConfig_allow_quoted_newlines)) | (Optional) Indicates whether to allow quoted data sections that contain newline characters in a CSV file. The default value is false. |
| Custom null values | None | `--null_marker` | `nullMarker` ([Java](https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.LoadJobConfiguration.Builder#com_google_cloud_bigquery_LoadJobConfiguration_Builder_setNullMarker_java_lang_String_), [Python](https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.job.LoadJobConfig#google_cloud_bigquery_job_LoadJobConfig_null_marker)) | (Optional) Specifies a string that represents a null value in a CSV file. For example, if you specify "\\N", BigQuery interprets "\\N" as a null value when loading a CSV file. The default value is the empty string. If you set this property to a custom value, BigQuery throws an error if an empty string is present for all data types except for STRING and BYTE. For STRING and BYTE columns, BigQuery interprets the empty string as an empty value. |
| Trailing optional columns | Allow jagged rows | `--allow_jagged_rows` | `allowJaggedRows` ([Java](https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.CsvOptions.Builder#com_google_cloud_bigquery_CsvOptions_Builder_setAllowQuotedNewLines_boolean_), [Python](https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.job.LoadJobConfig#google_cloud_bigquery_job_LoadJobConfig_allow_jagged_rows)) | (Optional) Accept rows that are missing trailing optional columns. The missing values are treated as nulls. If false, records with missing trailing columns are treated as bad records, and if there are too many bad records, an invalid error is returned in the job result. The default value is false. Only applicable to CSV, ignored for other formats. |
| Unknown values | Ignore unknown values | `--ignore_unknown_values` | `ignoreUnknownValues` ([Java](https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.LoadJobConfiguration.Builder#com_google_cloud_bigquery_LoadJobConfiguration_Builder_setIgnoreUnknownValues_java_lang_Boolean_), [Python](https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.job.LoadJobConfig#google_cloud_bigquery_job_LoadJobConfig_ignore_unknown_values)) | (Optional) Indicates if BigQuery should allow extra values that are not represented in the table schema. If true, the extra values are ignored. If false, records with extra columns are treated as bad records, and if there are too many bad records, an invalid error is returned in the job result. The default value is false. The `sourceFormat` property determines what BigQuery treats as an extra value: - CSV: Trailing columns - JSON: Named values that don't match any column names |
| Quote | Quote character: Double quote, Single quote, None, Custom | `--quote` | `quote` ([Java](https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.CsvOptions.Builder#com_google_cloud_bigquery_CsvOptions_Builder_setQuote_java_lang_String_), [Python](https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.job.LoadJobConfig#google_cloud_bigquery_job_LoadJobConfig_quote_character)) | (Optional) The value that is used to quote data sections in a CSV file. BigQuery converts the string to ISO-8859-1 encoding, and then uses the first byte of the encoded string to split the data in its raw, binary state. The default value is a double-quote ('"'). If your data does not contain quoted sections, set the property value to an empty string. If your data contains quoted newline characters, you must also set the `allowQuotedNewlines` property to `true`. To include the specific quote character within a quoted value, precede it with an additional matching quote character. For example, if you want to escape the default character ' " ', use ' "" '. |
| Encoding | None | `-E` or `--encoding` | `encoding` ([Java](https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.CsvOptions.Builder#com_google_cloud_bigquery_CsvOptions_Builder_setEncoding_java_nio_charset_Charset_), [Python](https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.job.LoadJobConfig#google_cloud_bigquery_job_LoadJobConfig_encoding)) | (Optional) The character encoding of the data. The supported values are UTF-8, ISO-8859-1, UTF-16BE, UTF-16LE, UTF-32BE, or UTF-32LE. The default value is UTF-8. BigQuery decodes the data after the raw, binary data has been split using the values of the `quote` and `fieldDelimiter` properties. |
| ASCII control character | None | `--preserve_ascii_control_characters` | None | (Optional) If you want to allow ASCII 0 and other ASCII control characters, then set `--preserve_ascii_control_characters` to `true` to your load jobs. |
| Null Markers | Null Markers | `--null_markers` | `nullMarkers` ([Java](https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.LoadJobConfiguration.Builder#com_google_cloud_bigquery_LoadJobConfiguration_Builder_setNullMarkers_java_util_List_java_lang_String__), [Python](https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.job.LoadJobConfig#google_cloud_bigquery_job_LoadJobConfig_null_markers)) | (Optional) A list of custom strings that represents a NULL value in CSV data. This option cannot be used with `--null_marker` option. |
| Time Zone | Time Zone | `--time_zone` | `timeZone` ([Java](https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.LoadJobConfiguration.Builder#com_google_cloud_bigquery_LoadJobConfiguration_Builder_setTimeZone_java_lang_String_), [Python](https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.job.LoadJobConfig#google_cloud_bigquery_job_LoadJobConfig_time_zone)) | (Optional) Default time zone that will apply when parsing timestamp values that have no specific time zone. Check [valid time zone names](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#time_zone_name). If this value is not present, the timestamp values without specific time zone is parsed using default time zone UTC. |
| Date Format | Date Format | `--date_format` | `dateFormat` ([Java](https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.LoadJobConfiguration.Builder#com_google_cloud_bigquery_LoadJobConfiguration_Builder_setDateFormat_java_lang_String_), [Python](https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.job.LoadJobConfig#google_cloud_bigquery_job_LoadJobConfig_date_format)) | (Optional) [Format elements](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/format-elements#format_string_as_datetime) that define how the DATE values are formatted in the input files (for example, `MM/DD/YYYY`). If this value is present, this format is the only compatible DATE format. [Schema autodetection](https://docs.cloud.google.com/bigquery/docs/schema-detect#date_and_time_values) will also decide DATE column type based on this format instead of the existing format. If this value is not present, the DATE field is parsed with the [default formats](https://docs.cloud.google.com/bigquery/docs/loading-data-cloud-storage-csv#data_types). |
| Datetime Format | Datetime Format | `--datetime_format` | `datetimeFormat` ([Java](https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.LoadJobConfiguration.Builder#com_google_cloud_bigquery_LoadJobConfiguration_Builder_setDatetimeFormat_java_lang_String_), [Python](https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.job.LoadJobConfig#google_cloud_bigquery_job_LoadJobConfig_datetime_format)) | (Optional) [Format elements](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/format-elements#format_string_as_datetime) that define how the DATETIME values are formatted in the input files (for example, `MM/DD/YYYY HH24:MI:SS.FF3`). If this value is present, this format is the only compatible DATETIME format. [Schema autodetection](https://docs.cloud.google.com/bigquery/docs/schema-detect#date_and_time_values) will also decide DATETIME column type based on this format instead of the existing format. If this value is not present, the DATETIME field is parsed with the [default formats](https://docs.cloud.google.com/bigquery/docs/loading-data-cloud-storage-csv#data_types). |
| Time Format | Time Format | `--time_format` | `timeFormat` ([Java](https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.LoadJobConfiguration.Builder#com_google_cloud_bigquery_LoadJobConfiguration_Builder_setTimeFormat_java_lang_String_), [Python](https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.job.LoadJobConfig#google_cloud_bigquery_job_LoadJobConfig_time_format)) | (Optional) [Format elements](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/format-elements#format_string_as_datetime) that define how the TIME values are formatted in the input files (for example, `HH24:MI:SS.FF3`). If this value is present, this format is the only compatible TIME format. [Schema autodetection](https://docs.cloud.google.com/bigquery/docs/schema-detect#date_and_time_values) will also decide TIME column type based on this format instead of the existing format. If this value is not present, the TIME field is parsed with the [default formats](https://docs.cloud.google.com/bigquery/docs/loading-data-cloud-storage-csv#data_types). |
| Timestamp Format | Timestamp Format | `--timestamp_format` | `timestampFormat` ([Java](https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.LoadJobConfiguration.Builder#com_google_cloud_bigquery_LoadJobConfiguration_Builder_setTimestampFormat_java_lang_String_), [Python](https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.job.LoadJobConfig#google_cloud_bigquery_job_LoadJobConfig_timestamp_format)) | (Optional) [Format elements](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/format-elements#format_string_as_datetime) that define how the TIMESTAMP values are formatted in the input files (for example, `MM/DD/YYYY HH24:MI:SS.FF3`). If this value is present, this format is the only compatible TIMESTAMP format. [Schema autodetection](https://docs.cloud.google.com/bigquery/docs/schema-detect#date_and_time_values) will also decide TIMESTAMP column type based on this format instead of the existing format. If this value is not present, the TIMESTAMP field is parsed with the [default formats](https://docs.cloud.google.com/bigquery/docs/loading-data-cloud-storage-csv#data_types). |