# Loading Parquet data from Cloud Storage

This page provides an overview of loading Parquet data from Cloud Storage
into BigQuery.

[Parquet](http://parquet.apache.org) is an
open source column-oriented data format that is widely used in the
Apache Hadoop ecosystem.

When you load Parquet data from Cloud Storage, you can load the data into a
new table or partition, or you can append to or overwrite an existing table or
partition. When your data is loaded into BigQuery, it is
converted into columnar format for
[Capacitor](https://cloud.google.com/blog/products/gcp/inside-capacitor-bigquerys-next-generation-columnar-storage-format)
(BigQuery's storage format).

When you load data from Cloud Storage into a BigQuery table,
the dataset that contains the table must be in the same regional or multi-
regional location as the Cloud Storage bucket.

For information about loading Parquet data from a local file, see
[Loading data from local files](https://docs.cloud.google.com/bigquery/docs/batch-loading-data).

## Limitations

You are subject to the following limitations when you load data into
BigQuery from a Cloud Storage bucket:

- BigQuery does not guarantee data consistency for external data sources. Changes to the underlying data while a query is running can result in unexpected behavior.
- BigQuery doesn't support
  [Cloud Storage object versioning](https://docs.cloud.google.com/storage/docs/object-versioning). If you
  include a generation number in the Cloud Storage URI, then the load job
  fails.

- You can't use a wildcard in the Cloud Storage URI if any of the
  files to be loaded have different schemas. Any difference in the position of
  columns qualifies as a different schema.

## Input file requirements

To avoid `resourcesExceeded` errors when loading Parquet files into
BigQuery, follow these guidelines:

- Keep row sizes to 50 MB or less.
- If your input data contains more than 100 columns, consider reducing the page size to be smaller than the default page size (1 \* 1024 \* 1024 bytes). This is especially helpful if you are using significant compression.
- For optimal performance, aim for row group sizes of at least 16 MiB. Smaller row group sizes increase I/O and slow down loads and queries.

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

## Parquet schemas

When you load Parquet files into BigQuery, the table schema is
automatically retrieved from the self-describing source data. When
BigQuery retrieves the schema from the source data, the
alphabetically last file is used.

For example, you have the following Parquet files in Cloud Storage:

```
gs://mybucket/00/
  a.parquet
  z.parquet
gs://mybucket/01/
  b.parquet
```

Running this command in the bq command-line tool loads all of the files (as a
comma-separated list), and the schema is derived from `mybucket/01/b.parquet`:

```bash
bq load \
--source_format=PARQUET \
dataset.table \
"gs://mybucket/00/*.parquet","gs://mybucket/01/*.parquet"
```

When you load multiple Parquet files that have different schemas, identical
columns specified in multiple schemas must have the same [mode](https://docs.cloud.google.com/bigquery/docs/schemas#modes)
in each schema definition.

When BigQuery detects the schema, some Parquet data types are
converted to BigQuery data types to make them compatible with
GoogleSQL syntax. For more information, see
[Parquet conversions](https://docs.cloud.google.com/bigquery/docs/loading-data-cloud-storage-parquet#parquet_conversions).
To provide a table schema for creating external tables, set the `referenceFileSchemaUri` property in BigQuery API or   
`--reference_file_schema_uri` parameter in bq command-line tool to the URL of the reference file.

For example, `--reference_file_schema_uri="gs://mybucket/schema.parquet"`.

## Parquet compression

BigQuery supports the following compression codecs for
Parquet file contents:

- `GZip`
- `LZO_1C`
- `LZO_1X`
- `LZ4_RAW`
- `Snappy`
- `ZSTD`

## Loading Parquet data into a new table

You can load Parquet data into a new table by using one of the following:

- The Google Cloud console
- The bq command-line tool's `bq load` command
- The `jobs.insert` API method and configuring a `load` job
- The client libraries

To load Parquet data from Cloud Storage into a new BigQuery
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
      2. For **File format** , select **Parquet**.
   2. In the **Destination** section, specify the following details:
      1. For **Dataset**, select the dataset in which you want to create the table.
      2. In the **Table** field, enter the name of the table that you want to create.
      3. Verify that the **Table type** field is set to **Native table**.
   3. In the **Schema** section, no action is necessary. The schema is self-described in Parquet files.
   4. Optional: Specify **Partition and cluster settings** . For more information, see [Creating partitioned tables](https://docs.cloud.google.com/bigquery/docs/creating-partitioned-tables) and [Creating and using clustered tables](https://docs.cloud.google.com/bigquery/docs/creating-clustered-tables).
   5. Click **Advanced options** and do the following:
      - For **Write preference** , leave **Write if empty** selected. This option creates a new table and loads your data into it.
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
The following example loads a Parquet file into the new table `mytable`:

<br />

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, enter the following statement:

   ```googlesql
   LOAD DATA OVERWRITE mydataset.mytable
   FROM FILES (
     format = 'PARQUET',
     uris = ['gs://bucket/path/file.parquet']);
   ```

   <br />

3. Click **Run**.

   <br />

For more information about how to run queries, see [Run an interactive query](https://docs.cloud.google.com/bigquery/docs/running-queries#queries).

### bq

Use the `bq load` command, specify `PARQUET` using the `--source_format`
flag, and include a [Cloud Storage URI](https://docs.cloud.google.com/bigquery/docs/batch-loading-data#gcs-uri).
You can include a single URI, a comma-separated list of URIs, or a URI
containing a [wildcard](https://docs.cloud.google.com/bigquery/docs/batch-loading-data#load-wildcards).

(Optional) Supply the `--location` flag and set the value to your
[location](https://docs.cloud.google.com/bigquery/docs/dataset-locations).

Other optional flags include:

- `--time_partitioning_type`: Enables time-based partitioning on a table and sets the partition type. Possible values are `HOUR`, `DAY`, `MONTH`, and `YEAR`. This flag is optional when you create a table partitioned on a `DATE`, `DATETIME`, or `TIMESTAMP` column. The default partition type for time-based partitioning is `DAY`. You cannot change the partitioning specification on an existing table.
- `--time_partitioning_expiration`: An integer that specifies (in seconds) when a time-based partition should be deleted. The expiration time evaluates to the partition's UTC date plus the integer value.
- `--time_partitioning_field`: The `DATE` or `TIMESTAMP` column used to create a partitioned table. If time-based partitioning is enabled without this value, an ingestion-time partitioned table is created.
- `--require_partition_filter`: When enabled, this option requires users to include a `WHERE` clause that specifies the partitions to query. Requiring a partition filter can reduce cost and improve performance. For more information, see [Require a partition filter in queries](https://docs.cloud.google.com/bigquery/docs/querying-partitioned-tables).
- `--clustering_fields`: A comma-separated list of up to four column names used to create a [clustered table](https://docs.cloud.google.com/bigquery/docs/creating-clustered-tables).
- `--destination_kms_key`: The Cloud KMS key for encryption of the table data.
- `--column_name_character_map`: Defines the scope and handling of
  characters in column names, with the option of enabling
  [flexible column names](https://docs.cloud.google.com/bigquery/docs/loading-data-cloud-storage-parquet#flexible-column-names).
  For more information, see
  [`load_option_list`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/load-statements#load_option_list).
  For more information on supported and unsupported characters, see
  [flexible column names](https://docs.cloud.google.com/bigquery/docs/loading-data-cloud-storage-parquet#flexible-column-names).

  For more information on partitioned tables, see:
  - [Creating partitioned tables](https://docs.cloud.google.com/bigquery/docs/creating-partitioned-tables)

  For more information on clustered tables, see:
  - [Creating and using clustered tables](https://docs.cloud.google.com/bigquery/docs/creating-clustered-tables)

  For more information on table encryption, see:
  - [Protecting data with Cloud KMS keys](https://docs.cloud.google.com/bigquery/docs/customer-managed-encryption)

To load Parquet data into BigQuery, enter the following command:

```bash
bq --location=LOCATION load \
--source_format=FORMAT \
DATASET.TABLE \
PATH_TO_SOURCE
```

Replace the following:

- `LOCATION`: your location. The `--location` flag is optional. For example, if you are using BigQuery in the Tokyo region, you can set the flag's value to `asia-northeast1`. You can set a default value for the location using the [.bigqueryrc file](https://docs.cloud.google.com/bigquery/docs/bq-command-line-tool#setting_default_values_for_command-line_flags).
- `FORMAT`: `PARQUET`.
- `DATASET`: an existing dataset.
- `TABLE`: the name of the table into which you're loading data.
- `PATH_TO_SOURCE`: a fully qualified [Cloud Storage URI](https://docs.cloud.google.com/bigquery/docs/batch-loading-data#gcs-uri) or a comma-separated list of URIs. [Wildcards](https://docs.cloud.google.com/bigquery/docs/batch-loading-data#load-wildcards) are also supported.

Examples:

The following command loads data from `gs://mybucket/mydata.parquet` into a
table named `mytable` in `mydataset`.

        bq load \
        --source_format=PARQUET \
        mydataset.mytable \
        gs://mybucket/mydata.parquet

The following command loads data from `gs://mybucket/mydata.parquet` into a
new ingestion-time partitioned table named `mytable` in `mydataset`.

        bq load \
        --source_format=PARQUET \
        --time_partitioning_type=DAY \
        mydataset.mytable \
        gs://mybucket/mydata.parquet

The following command loads data from `gs://mybucket/mydata.parquet` into a
partitioned table named `mytable` in `mydataset`. The table is partitioned
on the `mytimestamp` column.

        bq load \
        --source_format=PARQUET \
        --time_partitioning_field mytimestamp \
        mydataset.mytable \
        gs://mybucket/mydata.parquet

The following command loads data from multiple files in `gs://mybucket/`
into a table named `mytable` in `mydataset`. The Cloud Storage URI uses a
wildcard.

        bq load \
        --source_format=PARQUET \
        mydataset.mytable \
        gs://mybucket/mydata*.parquet

The following command loads data from multiple files in `gs://mybucket/`
into a table named `mytable` in `mydataset`. The command includes a comma-
separated list of Cloud Storage URIs with wildcards.

        bq load \
        --source_format=PARQUET \
        mydataset.mytable \
        "gs://mybucket/00/*.parquet","gs://mybucket/01/*.parquet"

### API

1. Create a `load` job that points to the source data in Cloud Storage.

2. (Optional) Specify your [location](https://docs.cloud.google.com/bigquery/docs/dataset-locations) in
   the `location` property in the `jobReference` section of the [job resource](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs).

3. The `source URIs` property must be fully qualified, in the format
   `gs://BUCKET/OBJECT`.
   Each URI can contain one '\*'
   [wildcard character](https://docs.cloud.google.com/bigquery/docs/batch-loading-data#load-wildcards).

4. Specify the Parquet data format by setting the `sourceFormat` property to
   `PARQUET`.

5. To check the job status, call
   [`jobs.get(JOB_ID*)`](https://docs.cloud.google.com/bigquery/docs/reference/v2/jobs/get),
   replacing <var translate="no">JOB_ID</var> with the ID of the job returned by the initial
   request.

   - If `status.state = DONE`, the job completed successfully.
   - If the `status.errorResult` property is present, the request failed, and that object includes information describing what went wrong. When a request fails, no table is created and no data is loaded.
   - If `status.errorResult` is absent, the job finished successfully; although, there might have been some nonfatal errors, such as problems importing a few rows. Nonfatal errors are listed in the returned job object's `status.errors` property.

**API notes:**

- Load jobs are atomic and consistent: if a load job fails, none of the data
  is available, and if a load job succeeds, all of the data is available.

- As a best practice, generate a unique ID and pass it as
  `jobReference.jobId` when calling `jobs.insert` to create a load job. This
  approach is more robust to network failure because the client can poll or
  retry on the known job ID.

- Calling `jobs.insert` on a given job ID is idempotent. You can retry as
  many times as you like on the same job ID, and at most one of those
  operations will succeed.

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

    // importParquet demonstrates loading Apache Parquet data from Cloud Storage into a table.
    func importParquet(projectID, datasetID, tableID string) error {
    	// projectID := "my-project-id"
    	// datasetID := "mydataset"
    	// tableID := "mytable"
    	ctx := context.Background()
    	client, err := bigquery.NewClient(ctx, projectID)
    	if err != nil {
    		return fmt.Errorf("bigquery.NewClient: %v", err)
    	}
    	defer client.Close()

    	gcsRef := bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_GCSReference_NewGCSReference("gs://cloud-samples-data/bigquery/us-states/us-states.parquet")
    	gcsRef.SourceFormat = bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_CSV_Avro_JSON_DatastoreBackup_GoogleSheets_Bigtable_Parquet_ORC_TFSavedModel_XGBoostBooster_Iceberg
    	gcsRef.AutoDetect = true
    	loader := client.Dataset(datasetID).Table(tableID).https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_Table_LoaderFrom(gcsRef)

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
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.LoadJobConfiguration.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableId.html;
    import java.math.BigInteger;

    public class LoadParquet {

      public static void runLoadParquet() {
        // TODO(developer): Replace these variables before running the sample.
        String datasetName = "MY_DATASET_NAME";
        loadParquet(datasetName);
      }

      public static void loadParquet(String datasetName) {
        try {
          // Initialize client that will be used to send requests. This client only needs to be created
          // once, and can be reused for multiple requests.
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html bigquery = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryOptions.html.getDefaultInstance().getService();

          String sourceUri = "gs://cloud-samples-data/bigquery/us-states/us-states.parquet";
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableId.html tableId = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableId.html.of(datasetName, "us_states");

          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.LoadJobConfiguration.html configuration =
              https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.LoadJobConfiguration.html.builder(tableId, sourceUri)
                  .setFormatOptions(https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.FormatOptions.html.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.FormatOptions.html#com_google_cloud_bigquery_FormatOptions_parquet__())
                  .build();

          // For more information on Job see:
          // https://googleapis.dev/java/google-cloud-clients/latest/index.html?com/google/cloud/bigquery/package-summary.html
          // Load the table
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Job.html job = bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html#com_google_cloud_bigquery_BigQuery_create_com_google_cloud_bigquery_DatasetInfo_com_google_cloud_bigquery_BigQuery_DatasetOption____(JobInfo.of(configuration));

          // Blocks until this load table job completes its execution, either failing or succeeding.
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Job.html completedJob = job.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Job.html#com_google_cloud_bigquery_Job_waitFor_com_google_cloud_bigquery_BigQueryRetryConfig_com_google_cloud_RetryOption____();
          if (completedJob == null) {
            System.out.println("Job not executed since it no longer exists.");
            return;
          } else if (completedJob.getStatus().getError() != null) {
            System.out.println(
                "BigQuery was unable to load the table due to an error: \n"
                    + job.getStatus().getError());
            return;
          }

          // Check number of rows loaded into the table
          BigInteger numRows = bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html#com_google_cloud_bigquery_BigQuery_getTable_com_google_cloud_bigquery_TableId_com_google_cloud_bigquery_BigQuery_TableOption____(tableId).getNumRows();
          System.out.printf("Loaded %d rows. \n", numRows);

          System.out.println("GCS parquet loaded successfully.");
        } catch (https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryException.html | InterruptedException e) {
          System.out.println("GCS Parquet was not loaded. \n" + e.toString());
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
     * This sample loads the Parquet file at
     * https://storage.googleapis.com/cloud-samples-data/bigquery/us-states/us-states.parquet
     *
     * TODO(developer): Replace the following lines with the path to your file.
     */
    const bucketName = 'cloud-samples-data';
    const filename = 'bigquery/us-states/us-states.parquet';

    async function loadTableGCSParquet() {
      // Imports a GCS file into a table with Parquet source format.

      /**
       * TODO(developer): Uncomment the following lines before running the sample.
       */
      // const datasetId = 'my_dataset';
      // const tableId = 'my_table';

      // Configure the load job. For full list of options, see:
      // https://cloud.google.com/bigquery/docs/reference/rest/v2/Job#JobConfigurationLoad
      const metadata = {
        sourceFormat: 'PARQUET',
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
    $gcsUri = 'gs://cloud-samples-data/bigquery/us-states/us-states.parquet';
    $loadConfig = $table->loadFromStorage($gcsUri)->sourceFormat('PARQUET');
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

Use the [Client.load_table_from_uri()](https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client#google_cloud_bigquery_client_Client_load_table_from_uri) method to start a load job from Cloud Storage. To use Parquet, set the [LoadJobConfig.source_format
property](https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.job.LoadJobConfig#google_cloud_bigquery_job_LoadJobConfig_source_format) to the string `PARQUET` and pass the job config as the `job_config` argument to the `load_table_from_uri()` method.

    from google.cloud import https://docs.cloud.google.com/python/docs/reference/bigquery/latest

    # Construct a BigQuery client object.
    client = https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html()

    # TODO(developer): Set table_id to the ID of the table to create.
    # table_id = "your-project.your_dataset.your_table_name"

    job_config = https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.job.LoadJobConfig.html(
        source_format=https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.enums.SourceFormat.html.PARQUET,
    )
    uri = "gs://cloud-samples-data/bigquery/us-states/us-states.parquet"

    load_job = client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_load_table_from_uri(
        uri, table_id, job_config=job_config
    )  # Make an API request.

    load_job.result()  # Waits for the job to complete.

    destination_table = client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_get_table(table_id)
    print("Loaded {} rows.".format(destination_table.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.table.Table.html#google_cloud_bigquery_table_Table_num_rows))

<br />

## Appending to or overwriting a table with Parquet data

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

To append or overwrite a table with Parquet data:

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
      2. For **File format** , select **Parquet**.

   > [!NOTE]
   > **Note:** It is possible to modify the table's schema when you append or overwrite it. For more information about supported schema changes during a load operation, see [Modifying table schemas](https://docs.cloud.google.com/bigquery/docs/managing-table-schemas).

   2. In the **Destination** section, specify the following details:
      1. For **Dataset**, select the dataset in which you want to create the table.
      2. In the **Table** field, enter the name of the table that you want to create.
      3. Verify that the **Table type** field is set to **Native table**.
   3. In the **Schema** section, no action is necessary. The schema is self-described in Parquet files.
   **Note:** It is possible to modify the table's schema when you append or overwrite it. For more information about supported schema changes during a load operation, see [Modifying table schemas](https://docs.cloud.google.com/bigquery/docs/managing-table-schemas).
   4. Optional: Specify **Partition and cluster settings** . For more information, see [Creating partitioned tables](https://docs.cloud.google.com/bigquery/docs/creating-partitioned-tables) and [Creating and using clustered tables](https://docs.cloud.google.com/bigquery/docs/creating-clustered-tables). You cannot convert a table to a partitioned or clustered table by appending or overwriting it. The Google Cloud console does not support appending to or overwriting partitioned or clustered tables in a load job.
   5. Click **Advanced options** and do the following:
      - For **Write preference** , choose **Append to table** or **Overwrite
        table**.
      - If you want to ignore values in a row that are not present in the table's schema, then select **Unknown values**.
      - For **Encryption** , click **Customer-managed key** to use a [Cloud Key Management Service key](https://docs.cloud.google.com/bigquery/docs/customer-managed-encryption). If you leave the **Google-managed key** setting, BigQuery [encrypts the data at rest](https://docs.cloud.google.com/docs/security/encryption/default-encryption).
   6. Click **Create table**.

### SQL

Use the
[`LOAD DATA` DDL statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/load-statements).
The following example appends a Parquet file to the table `mytable`:

<br />

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, enter the following statement:

   ```googlesql
   LOAD DATA INTO mydataset.mytable
   FROM FILES (
     format = 'PARQUET',
     uris = ['gs://bucket/path/file.parquet']);
   ```

   <br />

3. Click **Run**.

   <br />

For more information about how to run queries, see [Run an interactive query](https://docs.cloud.google.com/bigquery/docs/running-queries#queries).

### bq

Enter the `bq load` command with the `--replace` flag to overwrite the
table. Use the `--noreplace` flag to append data to the table. If no flag is
specified, the default is to append data. Supply the `--source_format` flag
and set it to `PARQUET`. Because Parquet schemas are automatically retrieved
from the self-describing source data, you don't need to provide a schema
definition.

> [!NOTE]
> **Note:** It is possible to modify the table's schema when you append or overwrite it. For more information on supported schema changes during a load operation, see [Modifying table schemas](https://docs.cloud.google.com/bigquery/docs/managing-table-schemas).

(Optional) Supply the `--location` flag and set the value to your
[location](https://docs.cloud.google.com/bigquery/docs/dataset-locations).

Other optional flags include:

- `--destination_kms_key`: The Cloud KMS key for encryption of the table data.

```bash
bq --location=LOCATION load \
--[no]replace \
--source_format=FORMAT \
DATASET.TABLE \
PATH_TO_SOURCE
```

Replace the following:

- `location`: your [location](https://docs.cloud.google.com/bigquery/docs/dataset-locations). The `--location` flag is optional. You can set a default value for the location by using the [.bigqueryrc file](https://docs.cloud.google.com/bigquery/docs/bq-command-line-tool#setting_default_values_for_command-line_flags).
- `format`: `PARQUET`.
- `dataset`: an existing dataset.
- `table`: the name of the table into which you're loading data.
- `path_to_source`: a fully qualified [Cloud Storage URI](https://docs.cloud.google.com/bigquery/docs/batch-loading-data#gcs-uri) or a comma-separated list of URIs. [Wildcards](https://docs.cloud.google.com/bigquery/docs/batch-loading-data#load-wildcards) are also supported.

Examples:

The following command loads data from `gs://mybucket/mydata.parquet` and
overwrites a table named `mytable` in `mydataset`.

        bq load \
        --replace \
        --source_format=PARQUET \
        mydataset.mytable \
        gs://mybucket/mydata.parquet

The following command loads data from `gs://mybucket/mydata.parquet` and
appends data to a table named `mytable` in `mydataset`.

        bq load \
        --noreplace \
        --source_format=PARQUET \
        mydataset.mytable \
        gs://mybucket/mydata.parquet

For information on appending and overwriting partitioned tables using the
bq command-line tool, see
[Appending to and overwriting partitioned table data](https://docs.cloud.google.com/bigquery/docs/managing-partitioned-table-data#append-overwrite).

### API

1. Create a `load` job that points to the source data in Cloud Storage.

2. (Optional) Specify your [location](https://docs.cloud.google.com/bigquery/docs/dataset-locations) in
   the `location` property in the `jobReference` section of the [job resource](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs).

3. The `source URIs` property
   must be fully qualified, in the format
   `gs://BUCKET/OBJECT`. You can
   include multiple URIs as a comma-separated list. Note that
   [wildcards](https://docs.cloud.google.com/bigquery/docs/batch-loading-data#load-wildcards) are
   also supported.

4. Specify the data format by setting the
   `configuration.load.sourceFormat` property to `PARQUET`.

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

    // importParquetTruncate demonstrates loading Apache Parquet data from Cloud Storage into a table
    // and overwriting/truncating existing data in the table.
    func importParquetTruncate(projectID, datasetID, tableID string) error {
    	// projectID := "my-project-id"
    	// datasetID := "mydataset"
    	// tableID := "mytable"
    	ctx := context.Background()
    	client, err := bigquery.NewClient(ctx, projectID)
    	if err != nil {
    		return fmt.Errorf("bigquery.NewClient: %v", err)
    	}
    	defer client.Close()

    	gcsRef := bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_GCSReference_NewGCSReference("gs://cloud-samples-data/bigquery/us-states/us-states.parquet")
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
    import java.math.BigInteger;

    public class LoadParquetReplaceTable {

      public static void runLoadParquetReplaceTable() {
        // TODO(developer): Replace these variables before running the sample.
        String datasetName = "MY_DATASET_NAME";
        loadParquetReplaceTable(datasetName);
      }

      public static void loadParquetReplaceTable(String datasetName) {
        try {
          // Initialize client that will be used to send requests. This client only needs to be created
          // once, and can be reused for multiple requests.
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html bigquery = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryOptions.html.getDefaultInstance().getService();

          // Imports a GCS file into a table and overwrites table data if table already exists.
          // This sample loads CSV file at:
          // https://storage.googleapis.com/cloud-samples-data/bigquery/us-states/us-states.csv
          String sourceUri = "gs://cloud-samples-data/bigquery/us-states/us-states.parquet";
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableId.html tableId = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableId.html.of(datasetName, "us_states");

          // For more information on LoadJobConfiguration see:
          // https://googleapis.dev/java/google-cloud-clients/latest/com/google/cloud/bigquery/LoadJobConfiguration.Builder.html
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.LoadJobConfiguration.html configuration =
              https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.LoadJobConfiguration.html.builder(tableId, sourceUri)
                  .setFormatOptions(https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.FormatOptions.html.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.FormatOptions.html#com_google_cloud_bigquery_FormatOptions_parquet__())
                  // Set the write disposition to overwrite existing table data.
                  .setWriteDisposition(https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.JobInfo.WriteDisposition.html.WRITE_TRUNCATE)
                  .build();

          // For more information on Job see:
          // https://googleapis.dev/java/google-cloud-clients/latest/index.html?com/google/cloud/bigquery/package-summary.html
          // Load the table
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Job.html job = bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html#com_google_cloud_bigquery_BigQuery_create_com_google_cloud_bigquery_DatasetInfo_com_google_cloud_bigquery_BigQuery_DatasetOption____(JobInfo.of(configuration));

          // Load data from a GCS parquet file into the table
          // Blocks until this load table job completes its execution, either failing or succeeding.
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Job.html completedJob = job.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Job.html#com_google_cloud_bigquery_Job_waitFor_com_google_cloud_bigquery_BigQueryRetryConfig_com_google_cloud_RetryOption____();
          if (completedJob == null) {
            System.out.println("Job not executed since it no longer exists.");
            return;
          } else if (completedJob.getStatus().getError() != null) {
            System.out.println(
                "BigQuery was unable to load into the table due to an error: \n"
                    + job.getStatus().getError());
            return;
          }

          // Check number of rows loaded into the table
          BigInteger numRows = bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html#com_google_cloud_bigquery_BigQuery_getTable_com_google_cloud_bigquery_TableId_com_google_cloud_bigquery_BigQuery_TableOption____(tableId).getNumRows();
          System.out.printf("Loaded %d rows. \n", numRows);

          System.out.println("GCS parquet overwrote existing table successfully.");
        } catch (https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryException.html | InterruptedException e) {
          System.out.println("Table extraction job was interrupted. \n" + e.toString());
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
    const filename = 'bigquery/us-states/us-states.parquet';

    async function loadParquetFromGCSTruncate() {
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
        sourceFormat: 'PARQUET',
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
    $gcsUri = 'gs://cloud-samples-data/bigquery/us-states/us-states.parquet';
    $loadConfig = $table->loadFromStorage($gcsUri)->sourceFormat('PARQUET')->writeDisposition('WRITE_TRUNCATE');
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

To append the rows to an existing table, set the [`LoadJobConfig.write_disposition` property](https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.job.LoadJobConfig#google_cloud_bigquery_job_LoadJobConfig_write_disposition) to [`WRITE_APPEND`](https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.enums.WriteDisposition#google.cloud.bigquery.enums.WriteDisposition.WRITE_APPEND).

<br />

To replace the rows in an existing table, set the
[`LoadJobConfig.write_disposition` property](https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.job.LoadJobConfig#google_cloud_bigquery_job_LoadJobConfig_write_disposition)
to
[`WRITE_TRUNCATE`](https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.enums.WriteDisposition#google.cloud.bigquery.enums.WriteDisposition.WRITE_TRUNCATE).


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
        source_format=https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.enums.SourceFormat.html.PARQUET,
    )

    uri = "gs://cloud-samples-data/bigquery/us-states/us-states.parquet"
    load_job = client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_load_table_from_uri(
        uri, table_id, job_config=job_config
    )  # Make an API request.

    load_job.result()  # Waits for the job to complete.

    destination_table = client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_get_table(table_id)
    print("Loaded {} rows.".format(destination_table.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.table.Table.html#google_cloud_bigquery_table_Table_num_rows))

<br />

## Loading hive-partitioned Parquet data

BigQuery supports loading hive partitioned Parquet data stored on
Cloud Storage and populates the hive partitioning columns as columns in
the destination BigQuery managed table. For more information, see
[Loading externally partitioned data](https://docs.cloud.google.com/bigquery/docs/hive-partitioned-loads-gcs).

## Parquet conversions

This section describes how BigQuery parses various data types when loading Parquet data.

Some Parquet data types (such as `INT32`, `INT64`, `BYTE_ARRAY`, and `FIXED_LEN_BYTE_ARRAY`) can be converted into multiple BigQuery data types. To ensure BigQuery converts the Parquet data types correctly, specify the appropriate data type in the Parquet file.

For example, to convert the Parquet `INT32` data type to the BigQuery `DATE` data type, specify the following:

```
optional int32 date_col (DATE);
```

BigQuery converts Parquet data types to the
BigQuery data types that are described in the following sections.

### Type conversions

| Parquet type | Parquet logical type(s) | BigQuery data type |
|---|---|---|
| `BOOLEAN` | None | BOOLEAN |
| INT32 | None, `INTEGER` (`UINT_8`, `UINT_16`, `UINT_32`, `INT_8`, `INT_16`, `INT_32`) | INT64 |
| INT32 | [DECIMAL](https://docs.cloud.google.com/bigquery/docs/loading-data-cloud-storage-parquet#decimal_logical_type) | NUMERIC, BIGNUMERIC, or STRING |
| `INT32` | `DATE` | DATE |
| `INT64` | None, `INTEGER` (`UINT_64`, `INT_64`) | INT64 |
| INT64 | [DECIMAL](https://docs.cloud.google.com/bigquery/docs/loading-data-cloud-storage-parquet#decimal_logical_type) | NUMERIC, BIGNUMERIC, or STRING |
| `INT64` | `TIMESTAMP`, `precision=MILLIS` (`TIMESTAMP_MILLIS`) | TIMESTAMP |
| `INT64` | `TIMESTAMP`, `precision=MICROS` (`TIMESTAMP_MICROS`) | TIMESTAMP |
| `INT96` | None | TIMESTAMP |
| `FLOAT` | None | FLOAT64 |
| `DOUBLE` | None | FLOAT64 |
| `BYTE_ARRAY` | None | BYTES |
| `BYTE_ARRAY` | `STRING` (`UTF8`) | STRING |
| FIXED_LEN_BYTE_ARRAY | [DECIMAL](https://docs.cloud.google.com/bigquery/docs/loading-data-cloud-storage-parquet#decimal_logical_type) | NUMERIC, BIGNUMERIC, or STRING |
| `FIXED_LEN_BYTE_ARRAY` | None | BYTES |

Nested groups are converted into
[`STRUCT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#struct_type) types.
Other combinations of Parquet types and converted types are not supported.

### Unsigned logical types

The Parquet `UINT_8`, `UINT_16`, `UINT_32`, and `UINT_64` types are unsigned.
BigQuery will treat values with these types as unsigned when loading into a
BigQuery signed `INTEGER` column. In the case of `UINT_64`, an error will be returned
if the unsigned value exceeds the maximum `INTEGER` value of
9,223,372,036,854,775,807.

### Decimal logical type

`Decimal` logical types can be converted to `NUMERIC`, `BIGNUMERIC`
, or `STRING` types. The converted type depends
on the precision and scale parameters of the `decimal` logical type and the
specified decimal target types. Specify the decimal target type as follows:

- For a [load job](https://docs.cloud.google.com/bigquery/docs/batch-loading-data) using the [`jobs.insert`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs/insert) API: use the [`JobConfigurationLoad.decimalTargetTypes`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/Job#JobConfigurationLoad.FIELDS.decimal_target_types) field.
- For a [load job](https://docs.cloud.google.com/bigquery/docs/batch-loading-data) using the [`bq load`](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#bq_load) command in the bq command-line tool: use the [`--decimal_target_types`](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#flags_and_arguments_9) flag.
- For a query against a [table with external sources](https://docs.cloud.google.com/bigquery/external-data-sources): use the [`ExternalDataConfiguration.decimalTargetTypes`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables#ExternalDataConfiguration.FIELDS.decimal_target_types) field.
- For a [persistent external table created with DDL](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language): use the [`decimal_target_types`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#external_table_option_list) option.

### Enum logical type

`Enum` logical types can be converted to `STRING` or `BYTES`. Specify the converted target type as follows:

- For a [load job](https://docs.cloud.google.com/bigquery/docs/batch-loading-data) using the [`jobs.insert`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs/insert) API: use the [`JobConfigurationLoad.parquetOptions`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/Job#JobConfigurationLoad.FIELDS.parquet_options) field.
- For a [load job](https://docs.cloud.google.com/bigquery/docs/batch-loading-data) using the [`bq load`](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#bq_load) command in the bq command-line tool: use the [`--parquet_enum_as_string`](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#parquet_enum_as_string_flag) flag.
- For a persistent external table created with [`bq mk`](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#bq_mk): use the [`--parquet_enum_as_string`](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#mk-table) flag.

### List logical type

You can enable schema inference for Parquet `LIST` logical types. BigQuery
checks whether the `LIST` node is in the
[standard form](https://github.com/apache/parquet-format/blob/master/LogicalTypes.md#lists) or in one of the forms described by the [backward-compatibility rules](https://github.com/apache/parquet-format/blob/master/LogicalTypes.md#backward-compatibility-rules):

    // standard form
    <optional | required> group <name> (LIST) {
      repeated group list {
        <optional | required> <element-type> element;
      }
    }

If yes, the corresponding field for the `LIST` node in the converted schema is treated
as if the node has the following schema:

    repeated <element-type> <name>

The nodes "list" and "element" are omitted.

- For a [load job](https://docs.cloud.google.com/bigquery/docs/batch-loading-data) using the [`jobs.insert`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs/insert) API, use the [`JobConfigurationLoad.parquetOptions` field](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/Job#JobConfigurationLoad.FIELDS.parquet_options).
- For a [load job](https://docs.cloud.google.com/bigquery/docs/batch-loading-data) using the [`bq load`](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#bq_load) command in the bq command-line tool, use the [`--parquet_enable_list_inference` flag](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#parquet_enable_list_inference_flag).
- For a persistent external table created with [`bq mk`](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#bq_mk), use the [`--parquet_enable_list_inference` flag](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#mk-table).
- For a persistent external table created with the [`CREATE EXTERNAL TABLE` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_external_table_statement), use the [`enable_list_inference` option](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#external_table_option_list).

### Geospatial data

You can load Parquet files that contain [WKT](https://en.wikipedia.org/wiki/Well-known_text_representation_of_geometry), hex-encoded [WKB](https://en.wikipedia.org/wiki/Well-known_text_representation_of_geometry), or [GeoJSON](https://geojson.org/) in a
`STRING` column, or [WKB](https://en.wikipedia.org/wiki/Well-known_text_representation_of_geometry) in a `BYTE_ARRAY` column by specifying a
BigQuery schema with the type `GEOGRAPHY`. For more information,
see [Loading geospatial data](https://docs.cloud.google.com/bigquery/docs/geospatial-data#loading_geospatial_data).

You can also load [GeoParquet](https://geoparquet.org) files. In this case, the
columns described by the GeoParquet metadata are interpreted as type `GEOGRAPHY`
by default. You can also load the raw WKB data into a `BYTES` column by
providing an explicit schema. For more information, see [Loading GeoParquet
files](https://docs.cloud.google.com/bigquery/docs/geospatial-data#loading_geoparquet_files).

### Column name conversions

A column name can contain letters (a-z, A-Z), numbers (0-9), or underscores
(_), and it must start with a letter or underscore. If you use flexible column
names, BigQuery supports starting a column name with a number.
Exercise caution when starting columns with a number, since using flexible
column names with the BigQuery Storage Read API or
BigQuery Storage Write API requires special handling. For more information about
flexible column name support, see
[flexible column names](https://docs.cloud.google.com/bigquery/docs/loading-data-cloud-storage-parquet#flexible-column-names).

Column names have a maximum length of 300 characters. Column names can't use any
of the following prefixes:

- `_TABLE_`
- `_FILE_`
- `_PARTITION`
- `_ROW_TIMESTAMP`
- `__ROOT__`
- `_COLIDENTIFIER`
- `_CHANGE_SEQUENCE_NUMBER`
- `_CHANGE_TYPE`
- `_CHANGE_TIMESTAMP`

Duplicate column names are not allowed even if the case differs. For example, a
column named `Column1` is considered identical to a column named `column1`. To
learn more about column naming rules, see [Column
names](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/lexical#column_names) in the
GoogleSQL reference.

If a table name (for example, `test`) is the same as one of its column names
(for example, `test`), the `SELECT` expression interprets the `test` column as
a `STRUCT` containing all other table columns. To avoid this collision, use
one of the following methods:

- Avoid using the same name for a table and its columns.

- Avoid using `_field_` as a column name prefix. System-reserved prefixes
  cause automatic renaming during queries. For example, the
  `SELECT _field_ FROM project1.dataset.test` query returns a column named
  `_field_1`. If you must query a column with this name, use an alias to
  control the output.

- Assign the table a different alias. For example, the following query assigns
  a table alias `t` to the table `project1.dataset.test`:

      SELECT test FROM project1.dataset.test AS t;

- Include the table name when referencing a column. For example:

      SELECT test.test FROM project1.dataset.test;

### Flexible column names

You have more flexibility in what you name columns, including expanded access
to characters in languages other than English as well as additional symbols.
Make sure to use backtick (`` ` ``) characters to enclose flexible column names if they are
[Quoted Identifiers](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/lexical#quoted_identifiers).

Flexible column names support the following characters:

- Any letter in any language, as represented by the Unicode regular expression [`\p{L}`](https://www.unicode.org/reports/tr44/#General_Category_Values).
- Any numeric character in any language as represented by the Unicode regular expression [`\p{N}`](https://www.unicode.org/reports/tr44/#General_Category_Values).
- Any connector punctuation character, including underscores, as represented by the Unicode regular expression [`\p{Pc}`](https://www.unicode.org/reports/tr44/#General_Category_Values).
- A hyphen or dash as represented by the Unicode regular expression [`\p{Pd}`](https://www.unicode.org/reports/tr44/#General_Category_Values).
- Any mark intended to accompany another character as represented by the Unicode regular expression [`\p{M}`](https://www.unicode.org/reports/tr44/#General_Category_Values). For example, accents, umlauts, or enclosing boxes.
- The following special characters:
  - An ampersand (`&`) as represented by the Unicode regular expression `\u0026`.
  - A percent sign (`%`) as represented by the Unicode regular expression `\u0025`.
  - An equals sign (`=`) as represented by the Unicode regular expression `\u003D`.
  - A plus sign (`+`) as represented by the Unicode regular expression `\u002B`.
  - A colon (`:`) as represented by the Unicode regular expression `\u003A`.
  - An apostrophe (`'`) as represented by the Unicode regular expression `\u0027`.
  - A less-than sign (`<`) as represented by the Unicode regular expression `\u003C`.
  - A greater-than sign (`>`) as represented by the Unicode regular expression `\u003E`.
  - A number sign (`#`) as represented by the Unicode regular expression `\u0023`.
  - A vertical line (`|`) as represented by the Unicode regular expression `\u007c`.
  - Whitespace.

Flexible column names don't support the following special characters:

- An exclamation mark (`!`) as represented by the Unicode regular expression `\u0021`.
- A quotation mark (`"`) as represented by the Unicode regular expression `\u0022`.
- A dollar sign (`$`) as represented by the Unicode regular expression `\u0024`.
- A left parenthesis (`(`) as represented by the Unicode regular expression `\u0028`.
- A right parenthesis (`)`) as represented by the Unicode regular expression `\u0029`.
- An asterisk (`*`) as represented by the Unicode regular expression `\u002A`.
- A comma (`,`) as represented by the Unicode regular expression `\u002C`.
- A period (`.`) as represented by the Unicode regular expression `\u002E`. Periods are *not* replaced by underscores in Parquet file column names when a column name character map is used. For more information, see [flexible column limitations](https://docs.cloud.google.com/bigquery/docs/loading-data-cloud-storage-parquet#limitations_2).
- A slash (`/`) as represented by the Unicode regular expression `\u002F`.
- A semicolon (`;`) as represented by the Unicode regular expression `\u003B`.
- A question mark (`?`) as represented by the Unicode regular expression `\u003F`.
- An at sign (`@`) as represented by the Unicode regular expression `\u0040`.
- A left square bracket (`[`) as represented by the Unicode regular expression `\u005B`.
- A backslash (`\`) as represented by the Unicode regular expression `\u005C`.
- A right square bracket (`]`) as represented by the Unicode regular expression `\u005D`.
- A circumflex accent (`^`) as represented by the Unicode regular expression `\u005E`.
- A grave accent (`` ` ``) as represented by the Unicode regular expression `\u0060`.
- A left curly bracket {`{`) as represented by the Unicode regular expression `\u007B`.
- A right curly bracket (`}`) as represented by the Unicode regular expression `\u007D`.
- A tilde (`~`) as represented by the Unicode regular expression `\u007E`.

For additional guidelines, see
[Column names](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/lexical#column_names).

The expanded column characters are supported by both the BigQuery Storage Read API
and the BigQuery Storage Write API. To use the expanded list of Unicode characters
with the BigQuery Storage Read API, you must set a flag. You can use the
`displayName` attribute to retrieve the column name. The following example
shows how to set a flag with the Python client:

    from google.cloud.bigquery_storage import types
    requested_session = types.ReadSession()

    #set avro serialization options for flexible column.
    options = types.AvroSerializationOptions()
    options.enable_display_name_attribute = True
    requested_session.read_options.avro_serialization_options = options

To use the expanded list of Unicode characters with the BigQuery Storage Write API,
you must provide the schema with `column_name` notation, unless you are using
the `JsonStreamWriter` writer object. The following example shows how to
provide the schema:

    syntax = "proto2";
    package mypackage;
    // Source protos located in github.com/googleapis/googleapis
    import "google/cloud/bigquery/storage/v1/annotations.proto";

    message FlexibleSchema {
      optional string item_name_column = 1
      [(.google.cloud.bigquery.storage.v1.column_name) = "name-列"];
      optional string item_description_column = 2
      [(.google.cloud.bigquery.storage.v1.column_name) = "description-列"];
    }

In this example, `item_name_column` and `item_description_column` are
placeholder names which need to be compliant with the
[protocol buffer](https://protobuf.dev/) naming
convention. Note that `column_name` annotations always take precedence over
placeholder names.

#### Limitations

- Flexible column names are not supported with
  [external tables](https://docs.cloud.google.com/bigquery/docs/external-tables).

- You cannot load Parquet files containing columns that have a period
  (.) in the column name.

- Column names from Parquet files are treated as case-insensitive when loaded
  into BigQuery. Identical case-insensitive names will cause
  collisions. To avoid this, either append an underscore to one of the duplicate
  column names or rename the columns before loading.

### Debugging your Parquet file

If your load jobs fail with data errors, you can use
[PyArrow](https://arrow.apache.org/docs/python/index.html) to verify if your
Parquet data files are corrupted. If PyArrow fails to read the files, the files
are likely to be rejected by the BigQuery load job. The following
example shows how to read the contents of a Parquet file by using PyArrow:

    from pyarrow import parquet as pq

    # Read the entire file
    pq.read_table('your_sample_file.parquet')
    # Read specific columns
    pq.read_table('your_sample_file.parquet',columns=['some_column', 'another_column'])
    # Read the metadata of specific columns
    file_metadata=pq.read_metadata('your_sample_file.parquet')
    for col in file_metadata.row_group(0).to_dict()['columns']:
        print col['column_path_in_schema']
        print col['num_values']

For more information, see the [PyArrow docs](https://arrow.apache.org/docs/python/index.html).