# Loading JSON data from Cloud Storage

You can load newline-delimited JSON (ndJSON) data from Cloud Storage into a new table
or partition, or append to or overwrite an existing table or partition. When
your data is loaded into BigQuery, it is converted into columnar format
for [Capacitor](https://cloud.google.com/blog/products/bigquery/inside-capacitor-bigquerys-next-generation-columnar-storage-format)
(BigQuery's storage format).

When you load data from Cloud Storage into a BigQuery table,
the dataset that contains the table must be in the same regional or multi-
regional location as the Cloud Storage bucket.

The ndJSON format is the same format as the
[JSON Lines](http://jsonlines.org/) format.

## Limitations

You are subject to the following limitations when you load data into
BigQuery from a Cloud Storage bucket:

- BigQuery does not guarantee data consistency for external data sources. Changes to the underlying data while a query is running can result in unexpected behavior.
- BigQuery doesn't support [Cloud Storage object versioning](https://docs.cloud.google.com/storage/docs/object-versioning). If you include a generation number in the Cloud Storage URI, then the load job fails.

When you load JSON files into BigQuery, note the following:

- JSON data must be newline-delimited, or ndJSON. Each JSON object must be on a separate line in the file.
- If you use gzip [compression](https://docs.cloud.google.com/bigquery/docs/batch-loading-data#loading_compressed_and_uncompressed_data), BigQuery cannot read the data in parallel. Loading compressed JSON data into BigQuery is slower than loading uncompressed data.
- You cannot include both compressed and uncompressed files in the same load job.
- The maximum size for a gzip file is 4 GB.
- BigQuery supports the `JSON` type even if schema information is
  not known at the time of ingestion. A field that is declared as `JSON` type
  is loaded with the raw JSON values.

- If you use the BigQuery API to load an integer outside the range
  of \[-2^53^+1, 2^53^-1\] (usually this means
  larger than 9,007,199,254,740,991), into an integer (INT64)
  column, pass it as a string to avoid data corruption. This issue is
  caused by a limitation on integer size in JSON or ECMAScript. For more
  information, see
  [the Numbers section of RFC 7159](https://www.rfc-editor.org/rfc/rfc7159.html#section-6).

- When you load CSV or JSON data, values in `DATE` columns must use the dash (`-`) separator and the date must be in the following format: `YYYY-MM-DD` (year-month-day).
- When you load JSON or CSV data, values in `TIMESTAMP` columns must use a dash (`-`) or slash (`/`) separator for the date portion of the timestamp, and the date must be in one of the following formats: `YYYY-MM-DD` (year-month-day) or `YYYY/MM/DD` (year/month/day). The `hh:mm:ss` (hour-minute-second) portion of the timestamp must use a colon (`:`) separator.
- Your files must meet the JSON file size limits described in the
  [load jobs limits](https://docs.cloud.google.com/bigquery/quotas#load_jobs).

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

## JSON compression

You can use the `gzip` utility to compress JSON files. Note that `gzip` performs
full file compression, unlike the file content compression performed by
compression codecs for other file formats, such as Avro. Using `gzip` to
compress your JSON files might have a performance impact; for more information
about the trade-offs, see
[Loading compressed and uncompressed data](https://docs.cloud.google.com/bigquery/docs/batch-loading-data#loading_compressed_and_uncompressed_data).

## Loading JSON data into a new table

To load JSON data from Cloud Storage into a new BigQuery
table:

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
      2. For **File format** , select **JSONL (Newline delimited JSON)**.
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
      - For **Encryption** , click **Customer-managed key** to use a [Cloud Key Management Service key](https://docs.cloud.google.com/bigquery/docs/customer-managed-encryption). If you leave the **Google-managed key** setting, BigQuery [encrypts the data at rest](https://docs.cloud.google.com/docs/security/encryption/default-encryption).
   6. Click **Create table**.

> [!NOTE]
> **Note:** When you load data into an empty table by using the Google Cloud console, you cannot add a label, description, table expiration, or partition expiration.  
>
> After the table is created, you can update the table's expiration, description, and labels, but you cannot add a partition expiration after a table is created using the Google Cloud console. For more information, see [Managing tables](https://docs.cloud.google.com/bigquery/docs/managing-tables).

### SQL

Use the
[`LOAD DATA` DDL statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/load-statements).
The following example loads a JSON file into the new table `mytable`:

<br />

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, enter the following statement:

   ```googlesql
   LOAD DATA OVERWRITE mydataset.mytable
   (x INT64,y STRING)
   FROM FILES (
     format = 'JSON',
     uris = ['gs://bucket/path/file.json']);
   ```

   <br />

3. Click **Run**.

   <br />

For more information about how to run queries, see [Run an interactive query](https://docs.cloud.google.com/bigquery/docs/running-queries#queries).

### bq

Use the `bq load` command, specify `NEWLINE_DELIMITED_JSON` using the
`--source_format` flag, and include a [Cloud Storage URI](https://docs.cloud.google.com/bigquery/docs/batch-loading-data#gcs-uri).
You can include a single URI, a comma-separated list of URIs, or a URI
containing a [wildcard](https://docs.cloud.google.com/bigquery/docs/batch-loading-data#load-wildcards).
Supply the schema inline, in a schema definition file, or use
[schema auto-detect](https://docs.cloud.google.com/bigquery/docs/schema-detect).

(Optional) Supply the `--location` flag and set the value to your
[location](https://docs.cloud.google.com/bigquery/docs/dataset-locations).

Other optional flags include:

- `--max_bad_records`: An integer that specifies the maximum number of bad records allowed before the entire job fails. The default value is `0`. At most, five errors of any type are returned regardless of the `--max_bad_records` value.
- `--ignore_unknown_values`: When specified, allows and ignores extra, unrecognized values in CSV or JSON data.
- `--time_zone`: ([Preview](https://cloud.google.com/products/#product-launch-stages)) An optional default time zone that will apply when parsing timestamp values that have no specific time zone in CSV or JSON data.
- `--date_format`: ([Preview](https://cloud.google.com/products/#product-launch-stages)) An optional custom string that defines how the DATE values are formatted in CSV or JSON data.
- `--datetime_format`: ([Preview](https://cloud.google.com/products/#product-launch-stages)) An optional custom string that defines how the DATETIME values are formatted in CSV or JSON data.
- `--time_format`: ([Preview](https://cloud.google.com/products/#product-launch-stages)) An optional custom string that defines how the TIME values are formatted in CSV or JSON data.
- `--timestamp_format`: ([Preview](https://cloud.google.com/products/#product-launch-stages)) An optional custom string that defines how the TIMESTAMP values are formatted in CSV or JSON data.
- `--autodetect`: When specified, enable schema auto-detection for CSV and JSON data.
- `--time_partitioning_type`: Enables time-based partitioning on a table and sets the partition type. Possible values are `HOUR`, `DAY`, `MONTH`, and `YEAR`. This flag is optional when you create a table partitioned on a `DATE`, `DATETIME`, or `TIMESTAMP` column. The default partition type for time-based partitioning is `DAY`. You cannot change the partitioning specification on an existing table.
- `--time_partitioning_expiration`: An integer that specifies (in seconds) when a time-based partition should be deleted. The expiration time evaluates to the partition's UTC date plus the integer value.
- `--time_partitioning_field`: The `DATE` or `TIMESTAMP` column used to create a partitioned table. If time-based partitioning is enabled without this value, an ingestion-time partitioned table is created.
- `--require_partition_filter`: When enabled, this option requires users to include a `WHERE` clause that specifies the partitions to query. Requiring a partition filter can reduce cost and improve performance. For more information, see [Require a partition filter in queries](https://docs.cloud.google.com/bigquery/docs/querying-partitioned-tables).
- `--clustering_fields`: A comma-separated list of up to four column names used to create a [clustered table](https://docs.cloud.google.com/bigquery/docs/creating-clustered-tables).
- `--destination_kms_key`: The Cloud KMS key for encryption of the
  table data.

  For more information on partitioned tables, see:
  - [Creating partitioned tables](https://docs.cloud.google.com/bigquery/docs/creating-partitioned-tables)

  For more information on clustered tables, see:
  - [Creating and using clustered tables](https://docs.cloud.google.com/bigquery/docs/creating-clustered-tables)

  For more information on table encryption, see:
  - [Protecting data with Cloud KMS keys](https://docs.cloud.google.com/bigquery/docs/customer-managed-encryption)

To load JSON data into BigQuery, enter the following command:

```bash
bq --location=LOCATION load \
--source_format=FORMAT \
DATASET.TABLE \
PATH_TO_SOURCE \
SCHEMA
```

Replace the following:

- `LOCATION`: your location. The `--location` flag is optional. For example, if you are using BigQuery in the Tokyo region, you can set the flag's value to `asia-northeast1`. You can set a default value for the location using the [.bigqueryrc file](https://docs.cloud.google.com/bigquery/docs/bq-command-line-tool#setting_default_values_for_command-line_flags).
- `FORMAT`: `NEWLINE_DELIMITED_JSON`.
- `DATASET`: an existing dataset.
- `TABLE`: the name of the table into which you're loading data.
- `PATH_TO_SOURCE`: a fully qualified [Cloud Storage URI](https://docs.cloud.google.com/bigquery/docs/batch-loading-data#gcs-uri) or a comma-separated list of URIs. [Wildcards](https://docs.cloud.google.com/bigquery/docs/batch-loading-data#load-wildcards) are also supported.
- `SCHEMA`: a valid schema. The schema can be a local JSON file, or it can be typed inline as part of the command. If you use a schema file, do not give it an extension. You can also use the `--autodetect` flag instead of supplying a schema definition.

Examples:

The following command loads data from `gs://mybucket/mydata.json` into a
table named `mytable` in `mydataset`. The schema is defined in a local
schema file named `myschema`.

        bq load \
        --source_format=NEWLINE_DELIMITED_JSON \
        mydataset.mytable \
        gs://mybucket/mydata.json \
        ./myschema

The following command loads data from `gs://mybucket/mydata.json` into a new
ingestion-time partitioned table named `mytable` in `mydataset`. The schema
is defined in a local schema file named `myschema`.

        bq load \
        --source_format=NEWLINE_DELIMITED_JSON \
        --time_partitioning_type=DAY \
        mydataset.mytable \
        gs://mybucket/mydata.json \
        ./myschema

The following command loads data from `gs://mybucket/mydata.json` into a
partitioned table named `mytable` in `mydataset`. The table is partitioned
on the `mytimestamp` column. The schema is defined in a local schema file
named `myschema`.

        bq load \
        --source_format=NEWLINE_DELIMITED_JSON \
        --time_partitioning_field mytimestamp \
        mydataset.mytable \
        gs://mybucket/mydata.json \
        ./myschema

The following command loads data from `gs://mybucket/mydata.json` into a
table named `mytable` in `mydataset`. The schema is auto-detected.

        bq load \
        --autodetect \
        --source_format=NEWLINE_DELIMITED_JSON \
        mydataset.mytable \
        gs://mybucket/mydata.json

The following command loads data from `gs://mybucket/mydata.json` into a
table named `mytable` in `mydataset`. The schema is defined inline in the
format `FIELD:DATA_TYPE, FIELD:DATA_TYPE`.

        bq load \
        --source_format=NEWLINE_DELIMITED_JSON \
        mydataset.mytable \
        gs://mybucket/mydata.json \
        qtr:STRING,sales:FLOAT,year:STRING

> [!NOTE]
> **Note:** When you specify the schema using the bq tool, you cannot include a `RECORD` ([`STRUCT`](https://docs.cloud.google.com/bigquery/docs/loading-data-cloud-storage-json#struct-type)) type, you cannot include a field description, and you cannot specify the field mode. All field modes default to `NULLABLE`. To include field descriptions, modes, and `RECORD` types, supply a [JSON schema file](https://docs.cloud.google.com/bigquery/docs/loading-data-cloud-storage-json#specifying_a_schema_file) instead.

The following command loads data from multiple files in `gs://mybucket/`
into a table named `mytable` in `mydataset`. The Cloud Storage URI uses a
wildcard. The schema is auto-detected.

        bq load \
        --autodetect \
        --source_format=NEWLINE_DELIMITED_JSON \
        mydataset.mytable \
        gs://mybucket/mydata*.json

The following command loads data from multiple files in `gs://mybucket/`
into a table named `mytable` in `mydataset`. The command includes a comma-
separated list of Cloud Storage URIs with wildcards. The schema is
defined in a local schema file named `myschema`.

        bq load \
        --source_format=NEWLINE_DELIMITED_JSON \
        mydataset.mytable \
        "gs://mybucket/00/*.json","gs://mybucket/01/*.json" \
        ./myschema

### API

1. Create a `load` job that points to the source data in Cloud Storage.

2. (Optional) Specify your [location](https://docs.cloud.google.com/bigquery/docs/dataset-locations) in
   the `location` property in the `jobReference` section of the
   [job resource](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs).

3. The `source URIs` property must be fully qualified, in the format
   `gs://BUCKET/OBJECT`.
   Each URI can contain one '\*'
   [wildcard character](https://docs.cloud.google.com/bigquery/docs/batch-loading-data#load-wildcards).

4. Specify the `JSON` data format by setting the `sourceFormat` property to
   `NEWLINE_DELIMITED_JSON`.

5. To check the job status, call
   [`jobs.get(JOB_ID*)`](https://docs.cloud.google.com/bigquery/docs/reference/v2/jobs/get),
   replacing `JOB_ID` with the ID of the job returned by
   the initial request.

   - If `status.state = DONE`, the job completed successfully.
   - If the `status.errorResult` property is present, the request failed, and that object includes information describing what went wrong. When a request fails, no table is created and no data is loaded.
   - If `status.errorResult` is absent, the job finished successfully; although, there might have been some nonfatal errors, such as problems importing a few rows. Nonfatal errors are listed in the returned job object's `status.errors` property.

**API notes:**

- Load jobs are atomic and consistent; if a load job fails, none of the data
  is available, and if a load job succeeds, all of the data is available.

- As a best practice, generate a unique ID and pass it as
  `jobReference.jobId` when calling `jobs.insert` to create a load job. This
  approach is more robust to network failure because the client can poll or
  retry on the known job ID.

- Calling `jobs.insert` on a given job ID is idempotent. You can retry as
  many times as you like on the same job ID, and at most, one of those
  operations succeed.

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

Use the [`BigQueryClient.CreateLoadJob()`](https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryClient#Google_Cloud_BigQuery_V2_BigQueryClient_CreateLoadJob_System_Collections_Generic_IEnumerable_System_String__Google_Apis_Bigquery_v2_Data_TableReference_Google_Apis_Bigquery_v2_Data_TableSchema_Google_Cloud_BigQuery_V2_CreateLoadJobOptions_) method to start a load job from Cloud Storage. To use JSONL, create a [`CreateLoadJobOptions`](https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.CreateLoadJobOptions) object and set its [`SourceFormat`](https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.CreateLoadJobOptions#Google_Cloud_BigQuery_V2_CreateLoadJobOptions_SourceFormat) property to [`FileFormat.NewlineDelimitedJson`](https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.FileFormat).

<br />



    using Google.Apis.Bigquery.v2.Data;
    using https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.html;
    using System;

    public class BigQueryLoadTableGcsJson
    {
        public void LoadTableGcsJson(
            string projectId = "your-project-id",
            string datasetId = "your_dataset_id"
        )
        {
            https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryClient.html client = https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryClient.html.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryClient.html#Google_Cloud_BigQuery_V2_BigQueryClient_Create_System_String_Google_Apis_Auth_OAuth2_GoogleCredential_(projectId);
            var gcsURI = "gs://cloud-samples-data/bigquery/us-states/us-states.json";
            var dataset = client.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryClient.html#Google_Cloud_BigQuery_V2_BigQueryClient_GetDataset_Google_Apis_Bigquery_v2_Data_DatasetReference_Google_Cloud_BigQuery_V2_GetDatasetOptions_(datasetId);
            var schema = new https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.TableSchemaBuilder.html {
                { "name", https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryDbType.html.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryDbType.html#Google_Cloud_BigQuery_V2_BigQueryDbType_String },
                { "post_abbr", https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryDbType.html.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryDbType.html#Google_Cloud_BigQuery_V2_BigQueryDbType_String }
            }.Build();
            TableReference destinationTableRef = dataset.GetTableReference(
                tableId: "us_states");
            // Create job configuration
            var jobOptions = new https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.CreateLoadJobOptions.html()
            {
                SourceFormat = https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.FileFormat.html.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.FileFormat.html#Google_Cloud_BigQuery_V2_FileFormat_NewlineDelimitedJson
            };
            // Create and run job
            https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryJob.html loadJob = client.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryClient.html#Google_Cloud_BigQuery_V2_BigQueryClient_CreateLoadJob_System_Collections_Generic_IEnumerable_System_String__Google_Apis_Bigquery_v2_Data_TableReference_Google_Apis_Bigquery_v2_Data_TableSchema_Google_Cloud_BigQuery_V2_CreateLoadJobOptions_(
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

    // importJSONExplicitSchema demonstrates loading newline-delimited JSON data from Cloud Storage
    // into a BigQuery table and providing an explicit schema for the data.
    func importJSONExplicitSchema(projectID, datasetID, tableID string) error {
    	// projectID := "my-project-id"
    	// datasetID := "mydataset"
    	// tableID := "mytable"
    	ctx := context.Background()
    	client, err := bigquery.NewClient(ctx, projectID)
    	if err != nil {
    		return fmt.Errorf("bigquery.NewClient: %v", err)
    	}
    	defer client.Close()

    	gcsRef := bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_GCSReference_NewGCSReference("gs://cloud-samples-data/bigquery/us-states/us-states.json")
    	gcsRef.SourceFormat = bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_CSV_Avro_JSON_DatastoreBackup_GoogleSheets_Bigtable_Parquet_ORC_TFSavedModel_XGBoostBooster_Iceberg
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

Use the [LoadJobConfiguration.builder(tableId, sourceUri)](https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.LoadJobConfiguration#com_google_cloud_bigquery_LoadJobConfiguration_builder_com_google_cloud_bigquery_TableId_java_lang_String_) method to start a load job from Cloud Storage. To use newline-delimited JSON, use the [LoadJobConfiguration.setFormatOptions(FormatOptions.json())](https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.LoadConfiguration.Builder#com_google_cloud_bigquery_LoadConfiguration_Builder_setFormatOptions_com_google_cloud_bigquery_FormatOptions_).

<br />


    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryException.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryOptions.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Field.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.FormatOptions.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Job.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.JobInfo.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.LoadJobConfiguration.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Schema.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.StandardSQLTypeName.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableId.html;

    // Sample to load JSON data from Cloud Storage into a new BigQuery table
    public class LoadJsonFromGCS {

      public static void runLoadJsonFromGCS() {
        // TODO(developer): Replace these variables before running the sample.
        String datasetName = "MY_DATASET_NAME";
        String tableName = "MY_TABLE_NAME";
        String sourceUri = "gs://cloud-samples-data/bigquery/us-states/us-states.json";
        https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Schema.html schema =
            https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Schema.html.of(
                https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Field.html.of("name", https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.StandardSQLTypeName.html.STRING),
                https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Field.html.of("post_abbr", https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.StandardSQLTypeName.html.STRING));
        loadJsonFromGCS(datasetName, tableName, sourceUri, schema);
      }

      public static void loadJsonFromGCS(
          String datasetName, String tableName, String sourceUri, https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Schema.html schema) {
        try {
          // Initialize client that will be used to send requests. This client only needs to be created
          // once, and can be reused for multiple requests.
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html bigquery = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryOptions.html.getDefaultInstance().getService();

          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableId.html tableId = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableId.html.of(datasetName, tableName);
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.LoadJobConfiguration.html loadConfig =
              https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.LoadJobConfiguration.html.newBuilder(tableId, sourceUri)
                  .setFormatOptions(https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.FormatOptions.html.json())
                  .setSchema(schema)
                  .build();

          // Load data from a GCS JSON file into the table
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Job.html job = bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html#com_google_cloud_bigquery_BigQuery_create_com_google_cloud_bigquery_DatasetInfo_com_google_cloud_bigquery_BigQuery_DatasetOption____(JobInfo.of(loadConfig));
          // Blocks until this load table job completes its execution, either failing or succeeding.
          job = job.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Job.html#com_google_cloud_bigquery_Job_waitFor_com_google_cloud_bigquery_BigQueryRetryConfig_com_google_cloud_RetryOption____();
          if (job.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Job.html#com_google_cloud_bigquery_Job_isDone__()) {
            System.out.println("Json from GCS successfully loaded in a table");
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
     * This sample loads the json file at
     * https://storage.googleapis.com/cloud-samples-data/bigquery/us-states/us-states.json
     *
     * TODO(developer): Replace the following lines with the path to your file.
     */
    const bucketName = 'cloud-samples-data';
    const filename = 'bigquery/us-states/us-states.json';

    async function loadJSONFromGCS() {
      // Imports a GCS file into a table with manually defined schema.

      /**
       * TODO(developer): Uncomment the following lines before running the sample.
       */
      // const datasetId = "my_dataset";
      // const tableId = "my_table";

      // Configure the load job. For full list of options, see:
      // https://cloud.google.com/bigquery/docs/reference/rest/v2/Job#JobConfigurationLoad
      const metadata = {
        sourceFormat: 'NEWLINE_DELIMITED_JSON',
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
    $gcsUri = 'gs://cloud-samples-data/bigquery/us-states/us-states.json';
    $schema = [
        'fields' => [
            ['name' => 'name', 'type' => 'string'],
            ['name' => 'post_abbr', 'type' => 'string']
        ]
    ];
    $loadConfig = $table->loadFromStorage($gcsUri)->schema($schema)->sourceFormat('NEWLINE_DELIMITED_JSON');
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

Use the [Client.load_table_from_uri()](https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client#google_cloud_bigquery_client_Client_load_table_from_uri) method to start a load job from Cloud Storage. To use JSONL, set the [LoadJobConfig.source_format
property](https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.job.LoadJobConfig#google_cloud_bigquery_job_LoadJobConfig_source_format) to the string `NEWLINE_DELIMITED_JSON` and pass the job config as the `job_config` argument to the `load_table_from_uri()` method.

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
        source_format=https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.enums.SourceFormat.html.NEWLINE_DELIMITED_JSON,
    )
    uri = "gs://cloud-samples-data/bigquery/us-states/us-states.json"

    load_job = client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_load_table_from_uri(
        uri,
        table_id,
        location="US",  # Must match the destination dataset location.
        job_config=job_config,
    )  # Make an API request.

    load_job.result()  # Waits for the job to complete.

    destination_table = client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_get_table(table_id)
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

Use the [Dataset.load_job()](https://googleapis.dev/ruby/google-cloud-bigquery/latest/Google/Cloud/Bigquery/Dataset.html?method=load_job-instance) method to start a load job from Cloud Storage. To use JSONL, set the `format` parameter to `"json"`.

<br />


    require "google/cloud/bigquery"

    def load_table_gcs_json dataset_id = "your_dataset_id"
      bigquery = Google::Cloud::https://docs.cloud.google.com/ruby/docs/reference/google-cloud-bigquery-storage/latest/Google-Cloud-Bigquery.html.https://docs.cloud.google.com/ruby/docs/reference/google-cloud-bigquery/latest/Google-Cloud-Bigquery.html
      dataset  = bigquery.dataset dataset_id
      gcs_uri  = "gs://cloud-samples-data/bigquery/us-states/us-states.json"
      table_id = "us_states"

      load_job = dataset.load_job table_id, gcs_uri, format: "json" do |schema|
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

## Loading nested and repeated JSON data

BigQuery supports loading nested and
repeated data from source formats that support object-based schemas, such as
JSON, Avro, ORC, Parquet, Firestore, and Datastore.

One [JSON](http://www.json.org) object,
including any nested or repeated fields, must appear on each line.

The following example shows sample nested or repeated data. This table contains
information about people. It consists of the following fields:

- `id`
- `first_name`
- `last_name`
- `dob` (date of birth)
- `addresses` (a nested and repeated field)
  - `addresses.status` (current or previous)
  - `addresses.address`
  - `addresses.city`
  - `addresses.state`
  - `addresses.zip`
  - `addresses.numberOfYears` (years at the address)

The JSON data file would look like the following. Notice that the address field
contains an array of values (indicated by `[ ]`).

```
{"id":"1","first_name":"John","last_name":"Doe","dob":"1968-01-22","addresses":[{"status":"current","address":"123 First Avenue","city":"Seattle","state":"WA","zip":"11111","numberOfYears":"1"},{"status":"previous","address":"456 Main Street","city":"Portland","state":"OR","zip":"22222","numberOfYears":"5"}]}
{"id":"2","first_name":"Jane","last_name":"Doe","dob":"1980-10-16","addresses":[{"status":"current","address":"789 Any Avenue","city":"New York","state":"NY","zip":"33333","numberOfYears":"2"},{"status":"previous","address":"321 Main Street","city":"Hoboken","state":"NJ","zip":"44444","numberOfYears":"3"}]}
```

<br />

The schema for this table would look like the following:

```json
[
    {
        "name": "id",
        "type": "STRING",
        "mode": "NULLABLE"
    },
    {
        "name": "first_name",
        "type": "STRING",
        "mode": "NULLABLE"
    },
    {
        "name": "last_name",
        "type": "STRING",
        "mode": "NULLABLE"
    },
    {
        "name": "dob",
        "type": "DATE",
        "mode": "NULLABLE"
    },
    {
        "name": "addresses",
        "type": "RECORD",
        "mode": "REPEATED",
        "fields": [
            {
                "name": "status",
                "type": "STRING",
                "mode": "NULLABLE"
            },
            {
                "name": "address",
                "type": "STRING",
                "mode": "NULLABLE"
            },
            {
                "name": "city",
                "type": "STRING",
                "mode": "NULLABLE"
            },
            {
                "name": "state",
                "type": "STRING",
                "mode": "NULLABLE"
            },
            {
                "name": "zip",
                "type": "STRING",
                "mode": "NULLABLE"
            },
            {
                "name": "numberOfYears",
                "type": "STRING",
                "mode": "NULLABLE"
            }
        ]
    }
]
```

<br />

For information on specifying a nested and repeated schema, see
[Specifying nested and repeated fields](https://docs.cloud.google.com/bigquery/docs/nested-repeated).

## Loading semi-structured JSON data

BigQuery supports loading semi-structured data, in which a field
can take values of different types. The following example shows data similar to
the preceding
[nested and repeated JSON data](https://docs.cloud.google.com/bigquery/docs/loading-data-cloud-storage-json#loading_nested_and_repeated_json_data)
example, except that the `address` field can be a `STRING`, a `STRUCT`, or
an `ARRAY`:

```
{"id":"1","first_name":"John","last_name":"Doe","dob":"1968-01-22","address":"123 First Avenue, Seattle WA 11111"}

{"id":"2","first_name":"Jane","last_name":"Doe","dob":"1980-10-16","address":{"status":"current","address":"789 Any Avenue","city":"New York","state":"NY","zip":"33333","numberOfYears":"2"}}

{"id":"3","first_name":"Bob","last_name":"Doe","dob":"1982-01-10","address":[{"status":"current","address":"789 Any Avenue","city":"New York","state":"NY","zip":"33333","numberOfYears":"2"}, "321 Main Street Hoboken NJ 44444"]}
```

You can load this data into BigQuery by using the following
schema:

```json
[
    {
        "name": "id",
        "type": "STRING",
        "mode": "NULLABLE"
    },
    {
        "name": "first_name",
        "type": "STRING",
        "mode": "NULLABLE"
    },
    {
        "name": "last_name",
        "type": "STRING",
        "mode": "NULLABLE"
    },
    {
        "name": "dob",
        "type": "DATE",
        "mode": "NULLABLE"
    },
    {
        "name": "address",
        "type": "JSON",
        "mode": "NULLABLE"
    }
]
```

The `address` field is loaded into a column with type
[`JSON`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#json_type) that allows
it to hold
the mixed types in the example. You can ingest data as `JSON` whether it
contains mixed types or not. For example, you could specify `JSON` instead of
`STRING` as the type for the `first_name` field. For more information, see
[Working with JSON data in GoogleSQL](https://docs.cloud.google.com/bigquery/docs/json-data).

## Appending to or overwriting a table with JSON data

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

You can append or overwrite a table by using one of the following:

- The Google Cloud console
- The bq command-line tool's `bq load` command
- The `jobs.insert` API method and configuring a `load` job
- The client libraries

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
      2. For **File format** , select **JSONL (Newline delimited JSON)**.

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
      - For **Encryption** , click **Customer-managed key** to use a [Cloud Key Management Service key](https://docs.cloud.google.com/bigquery/docs/customer-managed-encryption). If you leave the **Google-managed key** setting, BigQuery [encrypts the data at rest](https://docs.cloud.google.com/docs/security/encryption/default-encryption).
   6. Click **Create table**.

### SQL

Use the
[`LOAD DATA` DDL statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/load-statements).
The following example appends a JSON file to the table `mytable`:

<br />

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, enter the following statement:

   ```googlesql
   LOAD DATA INTO mydataset.mytable
   FROM FILES (
     format = 'JSON',
     uris = ['gs://bucket/path/file.json']);
   ```

   <br />

3. Click **Run**.

   <br />

For more information about how to run queries, see [Run an interactive query](https://docs.cloud.google.com/bigquery/docs/running-queries#queries).

### bq

Use the `bq load` command, specify `NEWLINE_DELIMITED_JSON` using the
`--source_format` flag, and include a [Cloud Storage URI](https://docs.cloud.google.com/bigquery/docs/batch-loading-data#gcs-uri).
You can include a single URI, a comma-separated list of URIs, or a URI
containing a [wildcard](https://docs.cloud.google.com/bigquery/docs/batch-loading-data#load-wildcards).

Supply the schema inline, in a schema definition file, or use
[schema auto-detect](https://docs.cloud.google.com/bigquery/docs/schema-detect).

Specify the `--replace` flag to overwrite the
table. Use the `--noreplace` flag to append data to the table. If no flag is
specified, the default is to append data.

It is possible to modify the table's schema when you append or
overwrite it. For more information on supported schema changes during a load
operation, see [Modifying table schemas](https://docs.cloud.google.com/bigquery/docs/managing-table-schemas).

(Optional) Supply the `--location` flag and set the value to your
[location](https://docs.cloud.google.com/bigquery/docs/dataset-locations).

Other optional flags include:

- `--max_bad_records`: An integer that specifies the maximum number of bad records allowed before the entire job fails. The default value is `0`. At most, five errors of any type are returned regardless of the `--max_bad_records` value.
- `--ignore_unknown_values`: When specified, allows and ignores extra, unrecognized values in CSV or JSON data.
- `--time_zone`: ([Preview](https://cloud.google.com/products/#product-launch-stages)) An optional default time zone that will apply when parsing timestamp values that have no specific time zone in CSV or JSON data.
- `--date_format`: ([Preview](https://cloud.google.com/products/#product-launch-stages)) An optional custom string that defines how the DATE values are formatted in CSV or JSON data.
- `--datetime_format`: ([Preview](https://cloud.google.com/products/#product-launch-stages)) An optional custom string that defines how the DATETIME values are formatted in CSV or JSON data.
- `--time_format`: ([Preview](https://cloud.google.com/products/#product-launch-stages)) An optional custom string that defines how the TIME values are formatted in CSV or JSON data.
- `--timestamp_format`: ([Preview](https://cloud.google.com/products/#product-launch-stages)) An optional custom string that defines how the TIMESTAMP values are formatted in CSV or JSON data.
- `--autodetect`: When specified, enable schema auto-detection for CSV and JSON data.
- `--destination_kms_key`: The Cloud KMS key for encryption of the table data.

```bash
bq --location=LOCATION load \
--[no]replace \
--source_format=FORMAT \
DATASET.TABLE \
PATH_TO_SOURCE \
SCHEMA
```

Replace the following:

- `LOCATION`: your [location](https://docs.cloud.google.com/bigquery/docs/dataset-locations). The `--location` flag is optional. You can set a default value for the location using the [.bigqueryrc file](https://docs.cloud.google.com/bigquery/docs/bq-command-line-tool#setting_default_values_for_command-line_flags).
- `FORMAT`: `NEWLINE_DELIMITED_JSON`.
- `DATASET`: an existing dataset.
- `TABLE`: the name of the table into which you're loading data.
- `PATH_TO_SOURCE`: a fully qualified [Cloud Storage URI](https://docs.cloud.google.com/bigquery/docs/batch-loading-data#gcs-uri) or a comma-separated list of URIs. [Wildcards](https://docs.cloud.google.com/bigquery/docs/batch-loading-data#load-wildcards) are also supported.
- `SCHEMA`: a valid schema. The schema can be a local JSON file, or it can be typed inline as part of the command. You can also use the `--autodetect` flag instead of supplying a schema definition.

Examples:

The following command loads data from `gs://mybucket/mydata.json` and
overwrites a table named `mytable` in `mydataset`. The schema is defined
using [schema auto-detection](https://docs.cloud.google.com/bigquery/docs/schema-detect).

        bq load \
        --autodetect \
        --replace \
        --source_format=NEWLINE_DELIMITED_JSON \
        mydataset.mytable \
        gs://mybucket/mydata.json

The following command loads data from `gs://mybucket/mydata.json` and
appends data to a table named `mytable` in `mydataset`. The schema is
defined using a JSON schema file --- `myschema`.

        bq load \
        --noreplace \
        --source_format=NEWLINE_DELIMITED_JSON \
        mydataset.mytable \
        gs://mybucket/mydata.json \
        ./myschema

### API

1. Create a `load` job that points to the source data in Cloud Storage.

2. (Optional) Specify your [location](https://docs.cloud.google.com/bigquery/docs/dataset-locations) in
   the `location` property in the `jobReference` section of the [job resource](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs).

3. The `source URIs` property
   must be fully qualified, in the format
   `gs://BUCKET/OBJECT`. You can
   include multiple URIs as a comma-separated list. The
   [wildcards](https://docs.cloud.google.com/bigquery/docs/batch-loading-data#load-wildcards) are
   also supported.

4. Specify the data format by setting the
   `configuration.load.sourceFormat` property to `NEWLINE_DELIMITED_JSON`.

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

    // importJSONTruncate demonstrates loading data from newline-delimeted JSON data in Cloud Storage
    // and overwriting/truncating data in the existing table.
    func importJSONTruncate(projectID, datasetID, tableID string) error {
    	// projectID := "my-project-id"
    	// datasetID := "mydataset"
    	// tableID := "mytable"
    	ctx := context.Background()
    	client, err := bigquery.NewClient(ctx, projectID)
    	if err != nil {
    		return fmt.Errorf("bigquery.NewClient: %v", err)
    	}
    	defer client.Close()

    	gcsRef := bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_GCSReference_NewGCSReference("gs://cloud-samples-data/bigquery/us-states/us-states.json")
    	gcsRef.SourceFormat = bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_CSV_Avro_JSON_DatastoreBackup_GoogleSheets_Bigtable_Parquet_ORC_TFSavedModel_XGBoostBooster_Iceberg
    	gcsRef.AutoDetect = true
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


    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryException.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryOptions.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Field.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.FormatOptions.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Job.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.JobInfo.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.LoadJobConfiguration.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Schema.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.StandardSQLTypeName.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableId.html;

    // Sample to overwrite the BigQuery table data by loading a JSON file from GCS
    public class LoadJsonFromGCSTruncate {

      public static void runLoadJsonFromGCSTruncate() {
        // TODO(developer): Replace these variables before running the sample.
        String datasetName = "MY_DATASET_NAME";
        String tableName = "MY_TABLE_NAME";
        String sourceUri = "gs://cloud-samples-data/bigquery/us-states/us-states.json";
        https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Schema.html schema =
            https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Schema.html.of(
                https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Field.html.of("name", https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.StandardSQLTypeName.html.STRING),
                https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Field.html.of("post_abbr", https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.StandardSQLTypeName.html.STRING));
        loadJsonFromGCSTruncate(datasetName, tableName, sourceUri, schema);
      }

      public static void loadJsonFromGCSTruncate(
          String datasetName, String tableName, String sourceUri, https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Schema.html schema) {
        try {
          // Initialize client that will be used to send requests. This client only needs to be created
          // once, and can be reused for multiple requests.
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html bigquery = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryOptions.html.getDefaultInstance().getService();

          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableId.html tableId = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableId.html.of(datasetName, tableName);
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.LoadJobConfiguration.html loadConfig =
              https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.LoadJobConfiguration.html.newBuilder(tableId, sourceUri)
                  .setFormatOptions(https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.FormatOptions.html.json())
                  // Set the write disposition to overwrite existing table data
                  .setWriteDisposition(https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.JobInfo.html.WriteDisposition.WRITE_TRUNCATE)
                  .setSchema(schema)
                  .build();

          // Load data from a GCS JSON file into the table
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Job.html job = bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html#com_google_cloud_bigquery_BigQuery_create_com_google_cloud_bigquery_DatasetInfo_com_google_cloud_bigquery_BigQuery_DatasetOption____(JobInfo.of(loadConfig));
          // Blocks until this load table job completes its execution, either failing or succeeding.
          job = job.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Job.html#com_google_cloud_bigquery_Job_waitFor_com_google_cloud_bigquery_BigQueryRetryConfig_com_google_cloud_RetryOption____();
          if (job.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Job.html#com_google_cloud_bigquery_Job_isDone__()) {
            System.out.println("Table is successfully overwritten by JSON file loaded from GCS");
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
     * This sample loads the JSON file at
     * https://storage.googleapis.com/cloud-samples-data/bigquery/us-states/us-states.json
     *
     * TODO(developer): Replace the following lines with the path to your file.
     */
    const bucketName = 'cloud-samples-data';
    const filename = 'bigquery/us-states/us-states.json';

    async function loadJSONFromGCSTruncate() {
      /**
       * Imports a GCS file into a table and overwrites
       * table data if table already exists.
       */

      /**
       * TODO(developer): Uncomment the following lines before running the sample.
       */
      // const datasetId = "my_dataset";
      // const tableId = "my_table";

      // Configure the load job. For full list of options, see:
      // https://cloud.google.com/bigquery/docs/reference/rest/v2/Job#JobConfigurationLoad
      const metadata = {
        sourceFormat: 'NEWLINE_DELIMITED_JSON',
        schema: {
          fields: [
            {name: 'name', type: 'STRING'},
            {name: 'post_abbr', type: 'STRING'},
          ],
        },
        // Set the write disposition to overwrite existing table data.
        writeDisposition: 'WRITE_TRUNCATE',
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
    // $projectId = 'The Google project ID';
    // $datasetId = 'The BigQuery dataset ID';
    // $tableID = 'The BigQuery table ID';

    // instantiate the bigquery table service
    $bigQuery = new BigQueryClient([
        'projectId' => $projectId,
    ]);
    $table = $bigQuery->dataset($datasetId)->table($tableId);

    // create the import job
    $gcsUri = 'gs://cloud-samples-data/bigquery/us-states/us-states.json';
    $loadConfig = $table->loadFromStorage($gcsUri)->sourceFormat('NEWLINE_DELIMITED_JSON')->writeDisposition('WRITE_TRUNCATE');
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

To replace the rows in an existing table, set the
[LoadJobConfig.write_disposition property](https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.job.LoadJobConfig#google_cloud_bigquery_job_LoadJobConfig_write_disposition)
to the string `WRITE_TRUNCATE`.


Before trying this sample, follow the Python setup instructions in the
[BigQuery quickstart using
client libraries](https://docs.cloud.google.com/bigquery/docs/quickstarts/quickstart-client-libraries).


For more information, see the
[BigQuery Python API
reference documentation](https://docs.cloud.google.com/python/docs/reference/bigquery/latest).


To authenticate to BigQuery, set up Application Default Credentials.
For more information, see

[Set up authentication for client libraries](https://docs.cloud.google.com/bigquery/docs/authentication#client-libs).

    import io

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

    body = io.BytesIO(b"Washington,WA")
    client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_load_table_from_file(body, table_id, job_config=job_config).result()
    previous_rows = client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_get_table(table_id).https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.table.Table.html#google_cloud_bigquery_table_Table_num_rows
    assert previous_rows > 0

    job_config = https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.job.LoadJobConfig.html(
        write_disposition=https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.enums.WriteDisposition.html.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.enums.WriteDisposition.html#google_cloud_bigquery_enums_WriteDisposition_WRITE_TRUNCATE,
        source_format=https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.enums.SourceFormat.html.NEWLINE_DELIMITED_JSON,
    )

    uri = "gs://cloud-samples-data/bigquery/us-states/us-states.json"
    load_job = client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_load_table_from_uri(
        uri, table_id, job_config=job_config
    )  # Make an API request.

    load_job.result()  # Waits for the job to complete.

    destination_table = client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_get_table(table_id)
    print("Loaded {} rows.".format(destination_table.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.table.Table.html#google_cloud_bigquery_table_Table_num_rows))

<br />

### Ruby

To replace the rows in an existing table, set the `write` parameter of
[Table.load_job()](https://googleapis.dev/ruby/google-cloud-bigquery/latest/Google/Cloud/Bigquery/Table.html?method=load_job-instance)
to `"WRITE_TRUNCATE"`.


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

    def load_table_gcs_json_truncate dataset_id = "your_dataset_id",
                                     table_id   = "your_table_id"
      bigquery = Google::Cloud::https://docs.cloud.google.com/ruby/docs/reference/google-cloud-bigquery-storage/latest/Google-Cloud-Bigquery.html.https://docs.cloud.google.com/ruby/docs/reference/google-cloud-bigquery/latest/Google-Cloud-Bigquery.html
      dataset  = bigquery.dataset dataset_id
      gcs_uri  = "gs://cloud-samples-data/bigquery/us-states/us-states.json"

      load_job = dataset.load_job table_id,
                                  gcs_uri,
                                  format: "json",
                                  write:  "truncate"
      puts "Starting job #{load_job.https://docs.cloud.google.com/ruby/docs/reference/google-cloud-bigquery/latest/Google-Cloud-Bigquery-Job.html}"

      load_job.wait_until_done! # Waits for table load to complete.
      puts "Job finished."

      table = dataset.table table_id
      puts "Loaded #{table.https://docs.cloud.google.com/ruby/docs/reference/google-cloud-bigquery/latest/Google-Cloud-Bigquery-Table.html} rows to table #{table.id}"
    end

<br />

## Loading hive-partitioned JSON data

BigQuery supports loading hive-partitioned JSON data stored on
Cloud Storage and populates the hive-partitioning columns as columns in
the destination BigQuery managed table. For more information, see
[Loading externally partitioned data](https://docs.cloud.google.com/bigquery/docs/hive-partitioned-loads-gcs).

## Details of loading JSON data

This section describes how BigQuery parses various data types when
loading JSON data.

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

**Interval** . Columns with INTERVAL types must be in
[ISO 8601](https://www.iso.org/iso-8601-date-and-time-format.html) format
`PYMDTHMS`, where:

- P = Designator that indicates that the value represents a duration. You must always include this.
- Y = Year
- M = Month
- D = Day
- T = Designator that denotes the time portion of the duration. You must always include this.
- H = Hour
- M = Minute
- S = Second. Seconds can be denoted as a whole value or as a fractional value of up to six digits, at microsecond precision.

You can indicate a negative value by prepending a dash (-).

The following list shows examples of valid data:

- `P-10000Y0M-3660000DT-87840000H0M0S`
- `P0Y0M0DT0H0M0.000001S`
- `P10000Y0M3660000DT87840000H0M0S`

To load INTERVAL data, you must use the `bq load` command and use the `--schema`
flag to specify a schema. You can't upload INTERVAL data by using the console.

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

**Array** (repeated field). The value must be a JSON array or `null`. JSON
`null` is converted to SQL `NULL`. The array itself cannot contain `null`
values.

### Schema auto-detection

This section describes the behavior of
[schema auto-detection](https://docs.cloud.google.com/bigquery/docs/schema-detect) when loading JSON files.

#### JSON nested and repeated fields

BigQuery infers nested and repeated fields in JSON files. If a
field value is a JSON object, then BigQuery loads the column as a
`RECORD` type. If a field value is an array, then BigQuery loads
the column as a repeated column. For an example of JSON data with nested and
repeated data, see
[Loading nested and repeated JSON data](https://docs.cloud.google.com/bigquery/docs/loading-data-cloud-storage-json#loading_nested_and_repeated_json_data).

#### String conversion

If you enable schema auto-detection, then BigQuery converts
strings into Boolean, numeric, or date/time types when possible. For example,
using the following JSON data, schema auto-detection converts the `id` field
to an `INTEGER` column:

    { "name":"Alice","id":"12"}
    { "name":"Bob","id":"34"}
    { "name":"Charles","id":"45"}

### Encoding types

BigQuery expects JSON data to be UTF-8 encoded. If you have
JSON files with other supported encoding types, you should explicitly specify
the encoding by using the [`--encoding` flag](https://docs.cloud.google.com/bigquery/docs/loading-data-cloud-storage-json#json-options) so that
BigQuery converts the data to UTF-8.

BigQuery supports the following encoding types for JSON files:

- UTF-8
- ISO-8859-1
- UTF-16BE (UTF-16 Big Endian)
- UTF-16LE (UTF-16 Little Endian)
- UTF-32BE (UTF-32 Big Endian)
- UTF-32LE (UTF-32 Little Endian)

## JSON options

To change how BigQuery parses JSON data, specify additional
options in the Google Cloud console, the bq command-line tool, the API, or the client
libraries.

| JSON option | Console option | bq tool flag | BigQuery API property | Description |
|---|---|---|---|---|
| Number of bad records allowed | Number of errors allowed | `--max_bad_records` | `maxBadRecords` ([Java](https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.LoadJobConfiguration.Builder#com_google_cloud_bigquery_LoadJobConfiguration_Builder_setMaxBadRecords_java_lang_Integer_), [Python](https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.job.LoadJobConfig#google_cloud_bigquery_job_LoadJobConfig_max_bad_records)) | (Optional) The maximum number of bad records that BigQuery can ignore when running the job. If the number of bad records exceeds this value, an invalid error is returned in the job result. The default value is \`0\`, which requires that all records are valid. |
| Unknown values | Ignore unknown values | `--ignore_unknown_values` | `ignoreUnknownValues` ([Java](https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.LoadJobConfiguration.Builder#com_google_cloud_bigquery_LoadJobConfiguration_Builder_setIgnoreUnknownValues_java_lang_Boolean_), [Python](https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.job.LoadJobConfig#google_cloud_bigquery_job_LoadJobConfig_ignore_unknown_values)) | (Optional) Indicates whether BigQuery should allow extra values that are not represented in the table schema. If true, the extra values are ignored. If false, records with extra columns are treated as bad records, and if there are too many bad records, an invalid error is returned in the job result. The default value is false. The \`sourceFormat\` property determines what BigQuery treats as an extra value: CSV: trailing columns, JSON: named values that don't match any column names. |
| Encoding | None | `-E` or `--encoding` | `encoding` ([Python](https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.job.LoadJobConfig#google_cloud_bigquery_job_LoadJobConfig_encoding)) | (Optional) The character encoding of the data. The supported values are UTF-8, ISO-8859-1, UTF-16BE, UTF-16LE, UTF-32BE, or UTF-32LE. The default value is UTF-8. |
| Time Zone | Time Zone | `--time_zone` | `timeZone` ([Java](https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.LoadJobConfiguration.Builder#com_google_cloud_bigquery_LoadJobConfiguration_Builder_setTimeZone_java_lang_String_), [Python](https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.job.LoadJobConfig#google_cloud_bigquery_job_LoadJobConfig_time_zone)) | (Optional) Default time zone that is applied when parsing timestamp values that have no specific time zone. Check [valid time zone names](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#time_zone_name). If this value is not present, the timestamp values without specific time zone is parsed using default time zone UTC. |
| Date Format | Date Format | `--date_format` | `dateFormat` ([Java](https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.LoadJobConfiguration.Builder#com_google_cloud_bigquery_LoadJobConfiguration_Builder_setDateFormat_java_lang_String_), [Python](https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.job.LoadJobConfig#google_cloud_bigquery_job_LoadJobConfig_date_format)) | (Optional) [Format elements](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/format-elements#format_string_as_datetime) that define how the DATE values are formatted in the input files (for example, `MM/DD/YYYY`). If this value is present, this format is the only compatible DATE format. [Schema autodetection](https://docs.cloud.google.com/bigquery/docs/schema-detect#date_and_time_values) will also decide DATE column type based on this format instead of the existing format. If this value is not present, the DATE field is parsed with the [default formats](https://docs.cloud.google.com/bigquery/docs/loading-data-cloud-storage-csv#data_types). |
| Datetime Format | Datetime Format | `--datetime_format` | `datetimeFormat` ([Java](https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.LoadJobConfiguration.Builder#com_google_cloud_bigquery_LoadJobConfiguration_Builder_setDatetimeFormat_java_lang_String_), [Python](https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.job.LoadJobConfig#google_cloud_bigquery_job_LoadJobConfig_datetime_format)) | (Optional) [Format elements](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/format-elements#format_string_as_datetime) that define how the DATETIME values are formatted in the input files (for example, `MM/DD/YYYY HH24:MI:SS.FF3`). If this value is present, this format is the only compatible DATETIME format. [Schema autodetection](https://docs.cloud.google.com/bigquery/docs/schema-detect#date_and_time_values) will also decide DATETIME column type based on this format instead of the existing format. If this value is not present, the DATETIME field is parsed with the [default formats](https://docs.cloud.google.com/bigquery/docs/loading-data-cloud-storage-csv#data_types). |
| Time Format | Time Format | `--time_format` | `timeFormat` ([Java](https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.LoadJobConfiguration.Builder#com_google_cloud_bigquery_LoadJobConfiguration_Builder_setTimeFormat_java_lang_String_), [Python](https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.job.LoadJobConfig#google_cloud_bigquery_job_LoadJobConfig_time_format)) | (Optional) [Format elements](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/format-elements#format_string_as_datetime) that define how the TIME values are formatted in the input files (for example, `HH24:MI:SS.FF3`). If this value is present, this format is the only compatible TIME format. [Schema autodetection](https://docs.cloud.google.com/bigquery/docs/schema-detect#date_and_time_values) will also decide TIME column type based on this format instead of the existing format. If this value is not present, the TIME field is parsed with the [default formats](https://docs.cloud.google.com/bigquery/docs/loading-data-cloud-storage-csv#data_types). |
| Timestamp Format | Timestamp Format | `--timestamp_format` | `timestampFormat` ([Java](https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.LoadJobConfiguration.Builder#com_google_cloud_bigquery_LoadJobConfiguration_Builder_setTimestampFormat_java_lang_String_), [Python](https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.job.LoadJobConfig#google_cloud_bigquery_job_LoadJobConfig_timestamp_format)) | (Optional) [Format elements](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/format-elements#format_string_as_datetime) that define how the TIMESTAMP values are formatted in the input files (for example, `MM/DD/YYYY HH24:MI:SS.FF3`). If this value is present, this format is the only compatible TIMESTAMP format. [Schema autodetection](https://docs.cloud.google.com/bigquery/docs/schema-detect#date_and_time_values) will also decide TIMESTAMP column type based on this format instead of the existing format. If this value is not present, the TIMESTAMP field is parsed with the [default formats](https://docs.cloud.google.com/bigquery/docs/loading-data-cloud-storage-csv#data_types). |

## What's next

- For information about loading JSON data from a local file, see [Loading data from local files](https://docs.cloud.google.com/bigquery/docs/batch-loading-data#loading_data_from_local_files).
- For more information about creating, ingesting, and querying JSON data, see [Working with JSON data in GoogleSQL](https://docs.cloud.google.com/bigquery/docs/json-data).