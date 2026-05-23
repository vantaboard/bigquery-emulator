# Load Avro data from Cloud Storage

[Avro](https://avro.apache.org) is an open
source data format that bundles serialized data with the data's schema in the
same file.

When you load Avro data from Cloud Storage, you can load the data into a new
table or partition, or you can append to or overwrite an existing table or
partition. When your data is loaded into BigQuery, it is
converted into [columnar format for Capacitor](https://cloud.google.com/blog/products/bigquery/inside-capacitor-bigquerys-next-generation-columnar-storage-format)
(BigQuery's storage format).

When you load data from Cloud Storage into a BigQuery table,
the dataset that contains the table must be in the same regional or multi-regional location as the Cloud Storage bucket.

For information about loading Avro data from a local file, see
[Loading data into BigQuery from a local data source](https://docs.cloud.google.com/bigquery/docs/batch-loading-data#loading_data_from_local_files).

## Limitations

You are subject to the following limitations when you load data into
BigQuery from a Cloud Storage bucket:

- BigQuery does not guarantee data consistency for external data sources. Changes to the underlying data while a query is running can result in unexpected behavior.
- BigQuery doesn't support [Cloud Storage object versioning](https://docs.cloud.google.com/storage/docs/object-versioning). If you include a generation number in the Cloud Storage URI, then the load job fails.

The following limitations also apply when loading Avro files into BigQuery:

- BigQuery doesn't support loading standalone Avro schema (.avsc) files.
- Nested array formatting isn't supported in BigQuery. Avro files that use this format must be converted before importing.
- In an Avro file, names and namespaces for a fullname can only contain alphanumeric characters and the underscore character `_`. The following regular expression shows the allowed characters: `[A-Za-z_][A-Za-z0-9_]*`.

For information about BigQuery load job limits, see
[Load jobs](https://docs.cloud.google.com/bigquery/quotas#load_jobs).

## Input file requirements

To avoid `resourcesExceeded` errors when loading Avro files into
BigQuery, follow these guidelines:

- Keep row sizes to 50 MB or less.
- If the row contains many array fields, or any very long array fields, break the array values into separate fields.

## Before you begin

Grant Identity and Access Management (IAM) roles that give users the necessary
permissions to perform each task in this document, and create a dataset and
table to store your data.

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

### Create a dataset and table

To store your data, you must create a [BigQuery dataset](https://docs.cloud.google.com/bigquery/docs/datasets),
and then create a [BigQuery table](https://docs.cloud.google.com/bigquery/docs/tables)
within that dataset.

## Advantages of Avro

Avro is the preferred format for loading data into BigQuery.
Loading Avro files has the following advantages over CSV and JSON (newline
delimited):

- The Avro binary format:
  - Is faster to load. The data can be read in parallel, even if the data blocks are [compressed](https://docs.cloud.google.com/bigquery/docs/batch-loading-data#loading_compressed_and_uncompressed_data).
  - Doesn't require typing or serialization.
  - Is easier to parse because there are no encoding issues found in other formats such as ASCII.
- When you load Avro files into BigQuery, the table schema is automatically retrieved from the self-describing source data.

## Avro schemas

When you load Avro files into a new BigQuery table, the table
schema is
automatically retrieved using the source data. When BigQuery
retrieves the schema from the source data, the alphabetically last file is used.

For example, you have the following Avro files in Cloud Storage:

```
gs://mybucket/00/
  a.avro
  z.avro
gs://mybucket/01/
  b.avro
```

Running this command in the bq command-line tool loads all of the files (as a
comma-separated list), and the schema is derived from `mybucket/01/b.avro`:

```bash
bq load \
--source_format=AVRO \
dataset.table \
"gs://mybucket/00/*.avro","gs://mybucket/01/*.avro"
```

When importing multiple Avro files with different Avro schemas, all schemas
must be compatible with
[Avro's schema resolution](https://avro.apache.org/docs/1.8.1/spec.html#Schema+Resolution).

When BigQuery detects the schema, some Avro data types are
converted to BigQuery data types to make them compatible with
GoogleSQL syntax. For more information, see [Avro conversions](https://docs.cloud.google.com/bigquery/docs/loading-data-cloud-storage-avro#avro_conversions).
To provide a table schema for creating external tables, set the `referenceFileSchemaUri` property in BigQuery API or   
`--reference_file_schema_uri` parameter in bq command-line tool to the URL of the reference file.

For example, `--reference_file_schema_uri="gs://mybucket/schema.avro"`.

You can also import schema into BigQuery, by [specifying a JSON schema file](https://docs.cloud.google.com/bigquery/docs/schemas#specifying_a_json_schema_file).

## Avro compression

BigQuery supports the following compression codecs for
Avro file contents:

- `Snappy`
- `DEFLATE`
- `ZSTD`

## Loading Avro data into a new table

To load Avro data from Cloud Storage into a new BigQuery table,
select one of the following options:

### Console

1. In the Google Cloud console, open the BigQuery page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)

   If you don't see the left pane, click **Expand left pane** to open the pane.
3. In the **Explorer** pane, expand the project, click **Datasets**, and
   then click the name of your dataset.

4. In the details pane, click **Create table**
   .

5. On the **Create table** page, in the **Source** section:

   - For **Create table from** , select **Google Cloud Storage**.

   - In the source field, browse to or enter the
     [Cloud Storage URI](https://docs.cloud.google.com/bigquery/docs/batch-loading-data#gcs-uri).
     Note that you cannot include multiple URIs in the
     Google Cloud console, but
     [wildcards](https://docs.cloud.google.com/bigquery/docs/batch-loading-data#load-wildcards) are
     supported. The Cloud Storage bucket must be in the same location
     as the dataset that contains the table you're creating.

     ![Select file](https://docs.cloud.google.com/static/bigquery/images/create-table-select-file.png)
   - For **File format** , select **Avro**.

6. On the **Create table** page, in the **Destination** section:

   - For **Dataset name**, choose the appropriate dataset.
   - Verify that **Table type** is set to **Native table**.
   - In the **Table name** field, enter the name of the table you're creating in BigQuery.
7. In the **Schema** section, no action is necessary. The schema is
   self-described in Avro files.

8. (Optional) To partition the table, choose your options in the
   **Partition and cluster settings** . For more information, see
   [Creating partitioned tables](https://docs.cloud.google.com/bigquery/docs/creating-partitioned-tables).

9. (Optional) For **Partitioning filter** , click the **Require partition
   filter** box to require users to include a `WHERE` clause that specifies the
   partitions to query. Requiring a partition filter may reduce cost and
   improve performance. For more information, see
   [Require a partition filter in queries](https://docs.cloud.google.com/bigquery/docs/querying-partitioned-tables#require_a_partition_filter_in_queries).
   This option is unavailable if **No partitioning** is selected.

10. (Optional) To [cluster](https://docs.cloud.google.com/bigquery/docs/creating-clustered-tables) the
    table, in the **Clustering order** box, enter between one and four field
    names.

11. (Optional) Click **Advanced options**.

    - For **Write preference** , leave **Write if empty** selected. This option creates a new table and loads your data into it.
    - For **Unknown values** , leave **Ignore unknown values** cleared. This option applies only to CSV and JSON files.
    - For **Encryption** , click **Customer-managed key** to use a [Cloud Key Management Service key](https://docs.cloud.google.com/bigquery/docs/customer-managed-encryption). If you leave the **Google-managed key** setting, BigQuery [encrypts the data at rest](https://docs.cloud.google.com/docs/security/encryption/default-encryption).
12. Click **Create table**.

> [!NOTE]
> **Note:** When you load data into an empty table by using the Google Cloud console, you cannot add a label, description, table expiration, or partition expiration.  
>
> After the table is created, you can update the table's expiration, description, and labels, but you cannot add a partition expiration after a table is created using the Google Cloud console. For more information, see [Managing tables](https://docs.cloud.google.com/bigquery/docs/managing-tables).

### SQL

Use the
[`LOAD DATA` DDL statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/load-statements).
The following example loads an Avro file into the new table `mytable`:

<br />

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, enter the following statement:

   ```googlesql
   LOAD DATA OVERWRITE mydataset.mytable
   FROM FILES (
     format = 'avro',
     uris = ['gs://bucket/path/file.avro']);
   ```

   <br />

3. Click **Run**.

   <br />

For more information about how to run queries, see [Run an interactive query](https://docs.cloud.google.com/bigquery/docs/running-queries#queries).

### bq

Use the `bq load` command, specify `AVRO` using the `--source_format`
flag, and include a [Cloud Storage URI](https://docs.cloud.google.com/bigquery/docs/batch-loading-data#gcs-uri).
You can include a single URI, a comma-separated list of URIs, or a URI
containing a [wildcard](https://docs.cloud.google.com/bigquery/docs/batch-loading-data#load-wildcards).

(Optional) Supply the `--location` flag and set the value to your
[location](https://docs.cloud.google.com/bigquery/docs/dataset-locations).

Other optional flags include:

- `--time_partitioning_type`: Enables time-based partitioning on a table and sets the partition type. Possible values are `HOUR`, `DAY`, `MONTH`, and `YEAR`. This flag is optional when you create a table partitioned on a `DATE`, `DATETIME`, or `TIMESTAMP` column. The default partition type for time-based partitioning is `DAY`. You cannot change the partitioning specification on an existing table.
- `--time_partitioning_expiration`: An integer that specifies (in seconds) when a time-based partition should be deleted. The expiration time evaluates to the partition's UTC date plus the integer value.
- `--time_partitioning_field`: The `DATE` or `TIMESTAMP` column used to create a partitioned table. If time-based partitioning is enabled without this value, an ingestion-time partitioned table is created.
- `--require_partition_filter`: When enabled, this option requires users to include a `WHERE` clause that specifies the partitions to query. Requiring a partition filter may reduce cost and improve performance. For more information, see [Require a partition filter in queries](https://docs.cloud.google.com/bigquery/docs/querying-partitioned-tables#require_a_partition_filter_in_queries).
- `--clustering_fields`: A comma-separated list of up to four column names used to create a [clustered table](https://docs.cloud.google.com/bigquery/docs/creating-clustered-tables).
- `--destination_kms_key`: The Cloud KMS key for encryption of the
  table data.

  For more information on partitioned tables, see:
  - [Creating partitioned tables](https://docs.cloud.google.com/bigquery/docs/creating-partitioned-tables)

  For more information on clustered tables, see:
  - [Creating and using clustered tables](https://docs.cloud.google.com/bigquery/docs/creating-clustered-tables)

  For more information on table encryption, see:
  - [Protecting data with Cloud KMS keys](https://docs.cloud.google.com/bigquery/docs/customer-managed-encryption)

To load Avro data into BigQuery, enter the following command:

```bash
bq --location=location load \
--source_format=format \
dataset.table \
path_to_source
```

Replace the following:

- <var translate="no">location</var> is your location. The `--location` flag is optional. For example, if you are using BigQuery in the Tokyo region, you can set the flag's value to `asia-northeast1`. You can set a default value for the location using the [.bigqueryrc file](https://docs.cloud.google.com/bigquery/docs/bq-command-line-tool#setting_default_values_for_command-line_flags).
- <var translate="no">format</var> is `AVRO`.
- <var translate="no">dataset</var> is an existing dataset.
- <var translate="no">table</var> is the name of the table into which you're loading data.
- <var translate="no">path_to_source</var> is a fully-qualified [Cloud Storage URI](https://docs.cloud.google.com/bigquery/docs/batch-loading-data#gcs-uri) or a comma-separated list of URIs. [Wildcards](https://docs.cloud.google.com/bigquery/docs/batch-loading-data#load-wildcards) are also supported.

Examples:

The following command loads data from `gs://mybucket/mydata.avro` into a
table named `mytable` in `mydataset`.

        bq load \
        --source_format=AVRO \
        mydataset.mytable \
        gs://mybucket/mydata.avro

The following command loads data from `gs://mybucket/mydata.avro` into an
ingestion-time partitioned table named `mytable` in `mydataset`.

        bq load \
        --source_format=AVRO \
        --time_partitioning_type=DAY \
        mydataset.mytable \
        gs://mybucket/mydata.avro

The following command loads data from `gs://mybucket/mydata.avro` into a new
partitioned table named `mytable` in `mydataset`. The table is partitioned
on the `mytimestamp` column.

        bq load \
        --source_format=AVRO \
        --time_partitioning_field mytimestamp \
        mydataset.mytable \
        gs://mybucket/mydata.avro

The following command loads data from multiple files in `gs://mybucket/`
into a table named `mytable` in `mydataset`. The Cloud Storage URI uses a
wildcard.

        bq load \
        --source_format=AVRO \
        mydataset.mytable \
        gs://mybucket/mydata*.avro

The following command loads data from multiple files in `gs://mybucket/`
into a table named `mytable` in `mydataset`. The command includes a comma-
separated list of Cloud Storage URIs with wildcards.

        bq load \
        --source_format=AVRO \
        mydataset.mytable \
        "gs://mybucket/00/*.avro","gs://mybucket/01/*.avro"

### API

1. Create a `load` job that points to the source data in Cloud Storage.

2. (Optional) Specify your [location](https://docs.cloud.google.com/bigquery/docs/dataset-locations) in
   the `location` property in the `jobReference` section of the [job resource](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs).

3. The `source URIs` property must be fully-qualified, in the format
   `gs://bucket/object`.
   Each URI can contain one '\*' [wildcard character](https://docs.cloud.google.com/bigquery/docs/batch-loading-data#load-wildcards).

4. Specify the Avro data format by setting the `sourceFormat` property to
   `AVRO`.

5. To check the job status, call
   [`jobs.get(job_id)`](https://docs.cloud.google.com/bigquery/docs/reference/v2/jobs/get),
   where <var translate="no">job_id</var> is the ID of the job returned by the initial
   request.

   - If `status.state = DONE`, the job completed successfully.
   - If the `status.errorResult` property is present, the request failed, and that object will include information describing what went wrong. When a request fails, no table is created and no data is loaded.
   - If `status.errorResult` is absent, the job finished successfully, although there might have been some non-fatal errors, such as problems importing a few rows. Non-fatal errors are listed in the returned job object's `status.errors` property.

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

    // importAvro demonstrates loading Apache Avro data from Cloud Storage into a table.
    func importAvro(projectID, datasetID, tableID string) error {
    	// projectID := "my-project-id"
    	// datasetID := "mydataset"
    	// tableID := "mytable"
    	ctx := context.Background()
    	client, err := bigquery.NewClient(ctx, projectID)
    	if err != nil {
    		return fmt.Errorf("bigquery.NewClient: %v", err)
    	}
    	defer client.Close()

    	gcsRef := bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_GCSReference_NewGCSReference("gs://cloud-samples-data/bigquery/us-states/us-states.avro")
    	gcsRef.SourceFormat = bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_CSV_Avro_JSON_DatastoreBackup_GoogleSheets_Bigtable_Parquet_ORC_TFSavedModel_XGBoostBooster_Iceberg
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

    // Sample to load Avro data from Cloud Storage into a new BigQuery table
    public class LoadAvroFromGCS {

      public static void runLoadAvroFromGCS() {
        // TODO(developer): Replace these variables before running the sample.
        String datasetName = "MY_DATASET_NAME";
        String tableName = "MY_TABLE_NAME";
        String sourceUri = "gs://cloud-samples-data/bigquery/us-states/us-states.avro";
        loadAvroFromGCS(datasetName, tableName, sourceUri);
      }

      public static void loadAvroFromGCS(String datasetName, String tableName, String sourceUri) {
        try {
          // Initialize client that will be used to send requests. This client only needs to be created
          // once, and can be reused for multiple requests.
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html bigquery = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryOptions.html.getDefaultInstance().getService();

          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableId.html tableId = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableId.html.of(datasetName, tableName);
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.LoadJobConfiguration.html loadConfig =
              https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.LoadJobConfiguration.html.of(tableId, sourceUri, https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.FormatOptions.html.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.FormatOptions.html#com_google_cloud_bigquery_FormatOptions_avro__());

          // Load data from a GCS Avro file into the table
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Job.html job = bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html#com_google_cloud_bigquery_BigQuery_create_com_google_cloud_bigquery_DatasetInfo_com_google_cloud_bigquery_BigQuery_DatasetOption____(JobInfo.of(loadConfig));
          // Blocks until this load table job completes its execution, either failing or succeeding.
          job = job.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Job.html#com_google_cloud_bigquery_Job_waitFor_com_google_cloud_bigquery_BigQueryRetryConfig_com_google_cloud_RetryOption____();
          if (job.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Job.html#com_google_cloud_bigquery_Job_isDone__()) {
            System.out.println("Avro from GCS successfully loaded in a table");
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
     * This sample loads the Avro file at
     * https://storage.googleapis.com/cloud-samples-data/bigquery/us-states/us-states.avro
     *
     * TODO(developer): Replace the following lines with the path to your file.
     */
    const bucketName = 'cloud-samples-data';
    const filename = 'bigquery/us-states/us-states.avro';

    async function loadTableGCSAvro() {
      // Imports a GCS file into a table with Avro source format.

      /**
       * TODO(developer): Uncomment the following lines before running the sample.
       */
      // const datasetId = 'my_dataset';
      // const tableId = 'us_states';

      // Configure the load job. For full list of options, see:
      // https://cloud.google.com/bigquery/docs/reference/rest/v2/Job#JobConfigurationLoad
      const jobConfigurationLoad = {
        load: {sourceFormat: 'AVRO'},
      };

      // Load data from a Google Cloud Storage file into the table
      const [job] = await bigquery
        .dataset(datasetId)
        .table(tableId)
        .https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/bigquery/table.html(storage.bucket(bucketName).file(filename), jobConfigurationLoad);

      // load() waits for the job to finish
      console.log(`Job ${https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/bigquery/bigquery.html.id} completed.`);

      // Check the job's status for errors
      const errors = https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/bigquery/bigquery.html.status.errors;
      if (errors && errors.length > 0) {
        throw errors;
      }
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
    # table_id = "your-project.your_dataset.your_table_name

    job_config = https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.job.LoadJobConfig.html(source_format=https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.enums.SourceFormat.html.AVRO)
    uri = "gs://cloud-samples-data/bigquery/us-states/us-states.avro"

    load_job = client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_load_table_from_uri(
        uri, table_id, job_config=job_config
    )  # Make an API request.

    load_job.result()  # Waits for the job to complete.

    destination_table = client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_get_table(table_id)
    print("Loaded {} rows.".format(destination_table.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.table.Table.html#google_cloud_bigquery_table_Table_num_rows))

### Extract JSON data from Avro data

There are two ways to ensure that Avro data is loaded into
BigQuery as
[`JSON` data](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#json_type):

1. Annotate your Avro schema with `sqlType` set to `JSON`. For example, if you
   load data with the following Avro schema, then the `json_field` column is
   read as a `JSON` type:

   ```json
   {
       "type": {"type": "string", "sqlType": "JSON"},
       "name": "json_field"
   }
   ```
2. Specify the BigQuery destination table schema explicitly and
   set the column type to `JSON`. For more information, see
   [Specifying a schema](https://docs.cloud.google.com/bigquery/docs/schemas).

If you do not specify JSON as the type in either the Avro schema or the
BigQuery table schema, then the data will be read as a `STRING`.

## Appending to or overwriting a table with Avro data

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

To append or overwrite a table with Avro data:

### Console

1. In the Google Cloud console, open the BigQuery page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)
3. In the **Explorer** pane, expand the project, click **Datasets**, and
   then click the name of your dataset.

4. In the details pane, click **Create table**
   .

5. On the **Create table** page, in the **Source** section:

   - For **Create table from**, select Cloud Storage.
   - In the source field, browse to or
     enter the [Cloud Storage URI](https://docs.cloud.google.com/bigquery/docs/loading-data-cloud-storage-avro#gcs-uri). Note that you cannot
     include multiple URIs in the Google Cloud console, but [wildcards](https://docs.cloud.google.com/bigquery/docs/batch-loading-data#load-wildcards)
     are supported. The Cloud Storage bucket must be in the same location
     as the dataset that contains the table you're appending or overwriting.

     ![Select file](https://docs.cloud.google.com/static/bigquery/images/create-table-select-file.png)
   - For **File format** , select **Avro**.

6. On the **Create table** page, in the **Destination** section:

   - For **Dataset name**, choose the appropriate dataset.

     ![Select dataset](https://docs.cloud.google.com/static/bigquery/images/create-table-select-dataset.png)
   - In the **Table name** field, enter the name of the table you're
     appending or overwriting in BigQuery.

   - Verify that **Table type** is set to **Native table**.

7. In the **Schema** section, no action is necessary. The schema is
   self-described in Avro files.

   > [!NOTE]
   > **Note:** It is possible to modify the table's schema when you append or overwrite it. For more information on supported schema changes during a load operation, see [Modifying table schemas](https://docs.cloud.google.com/bigquery/docs/managing-table-schemas).

8. For **Partition and cluster settings**, leave the default values. You
   cannot convert a table to a partitioned or clustered table by appending or
   overwriting it, and the Google Cloud console does not support
   appending to or overwriting partitioned or clustered tables in a load job.

9. Click **Advanced options**.

   - For **Write preference** , choose **Append to table** or **Overwrite
     table**.
   - For **Unknown values** , leave **Ignore unknown values** cleared. This option applies only to CSV and JSON files.
   - For **Encryption** , click **Customer-managed key** to use a [Cloud Key Management Service key](https://docs.cloud.google.com/bigquery/docs/customer-managed-encryption). If you leave the **Google-owned and managed key** setting, BigQuery [encrypts the data at rest](https://docs.cloud.google.com/docs/security/encryption/default-encryption).
10. Click **Create table**.

### SQL

Use the
[`LOAD DATA` DDL statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/load-statements).
The following example appends an Avro file to the table `mytable`:

<br />

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, enter the following statement:

   ```googlesql
   LOAD DATA INTO mydataset.mytable
   FROM FILES (
     format = 'avro',
     uris = ['gs://bucket/path/file.avro']);
   ```

   <br />

3. Click **Run**.

   <br />

For more information about how to run queries, see [Run an interactive query](https://docs.cloud.google.com/bigquery/docs/running-queries#queries).

### bq

Enter the `bq load` command with the `--replace` flag to overwrite the
table. Use the `--noreplace` flag to append data to the table. If no flag is
specified, the default is to append data. Supply the `--source_format` flag
and set it to `AVRO`. Because Avro schemas are automatically retrieved
from the self-describing source data, you do not need to provide a schema
definition.

> [!NOTE]
> **Note:** It is possible to modify the table's schema when you append or overwrite it. For more information on supported schema changes during a load operation, see [Modifying table schemas](https://docs.cloud.google.com/bigquery/docs/managing-table-schemas).

(Optional) Supply the `--location` flag and set the value to your
[location](https://docs.cloud.google.com/bigquery/docs/dataset-locations).

Other optional flags include:

- `--destination_kms_key`: The Cloud KMS key for encryption of the table data.

```bash
bq --location=location load \
--[no]replace \
--source_format=format \
dataset.table \
path_to_source
```

Replace the following:

- <var translate="no">location</var> is your [location](https://docs.cloud.google.com/bigquery/docs/dataset-locations). The `--location` flag is optional. You can set a default value for the location by using the [.bigqueryrc file](https://docs.cloud.google.com/bigquery/docs/bq-command-line-tool#setting_default_values_for_command-line_flags).
- <var translate="no">format</var> is `AVRO`.
- <var translate="no">dataset</var> is an existing dataset.
- <var translate="no">table</var> is the name of the table into which you're loading data.
- <var translate="no">path_to_source</var> is a fully-qualified [Cloud Storage URI](https://docs.cloud.google.com/bigquery/docs/batch-loading-data#gcs-uri) or a comma-separated list of URIs. [Wildcards](https://docs.cloud.google.com/bigquery/docs/batch-loading-data#load-wildcards) are also supported.

Examples:

The following command loads data from `gs://mybucket/mydata.avro` and
overwrites a table named `mytable` in `mydataset`.

        bq load \
        --replace \
        --source_format=AVRO \
        mydataset.mytable \
        gs://mybucket/mydata.avro

The following command loads data from `gs://mybucket/mydata.avro` and
appends data to a table named `mytable` in `mydataset`.

        bq load \
        --noreplace \
        --source_format=AVRO \
        mydataset.mytable \
        gs://mybucket/mydata.avro

For information on appending and overwriting partitioned tables using the
bq command-line tool, see:
[Appending to and overwriting partitioned table data](https://docs.cloud.google.com/bigquery/docs/managing-partitioned-table-data#append-overwrite).

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
   `configuration.load.sourceFormat` property to `AVRO`.

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

    // importAvroTruncate demonstrates loading Apache Avro data from Cloud Storage into a table
    // and overwriting/truncating existing data in the table.
    func importAvroTruncate(projectID, datasetID, tableID string) error {
    	// projectID := "my-project-id"
    	// datasetID := "mydataset"
    	// tableID := "mytable"
    	ctx := context.Background()
    	client, err := bigquery.NewClient(ctx, projectID)
    	if err != nil {
    		return fmt.Errorf("bigquery.NewClient: %v", err)
    	}
    	defer client.Close()

    	gcsRef := bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_GCSReference_NewGCSReference("gs://cloud-samples-data/bigquery/us-states/us-states.avro")
    	gcsRef.SourceFormat = bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_CSV_Avro_JSON_DatastoreBackup_GoogleSheets_Bigtable_Parquet_ORC_TFSavedModel_XGBoostBooster_Iceberg
    	loader := client.Dataset(datasetID).Table(tableID).https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_Table_LoaderFrom(gcsRef)
    	// Default for import jobs is to append data to a table.  WriteTruncate
    	// specifies that existing data should instead be replaced/overwritten.
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
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.FormatOptions.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Job.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.JobInfo.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.LoadJobConfiguration.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableId.html;

    // Sample to overwrite the BigQuery table data by loading a AVRO file from GCS
    public class LoadAvroFromGCSTruncate {

      public static void runLoadAvroFromGCSTruncate() {
        // TODO(developer): Replace these variables before running the sample.
        String datasetName = "MY_DATASET_NAME";
        String tableName = "MY_TABLE_NAME";
        String sourceUri = "gs://cloud-samples-data/bigquery/us-states/us-states.avro";
        loadAvroFromGCSTruncate(datasetName, tableName, sourceUri);
      }

      public static void loadAvroFromGCSTruncate(
          String datasetName, String tableName, String sourceUri) {
        try {
          // Initialize client that will be used to send requests. This client only needs to be created
          // once, and can be reused for multiple requests.
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html bigquery = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryOptions.html.getDefaultInstance().getService();

          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableId.html tableId = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableId.html.of(datasetName, tableName);
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.LoadJobConfiguration.html loadConfig =
              https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.LoadJobConfiguration.html.newBuilder(tableId, sourceUri)
                  .setFormatOptions(https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.FormatOptions.html.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.FormatOptions.html#com_google_cloud_bigquery_FormatOptions_avro__())
                  // Set the write disposition to overwrite existing table data
                  .setWriteDisposition(https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.JobInfo.html.WriteDisposition.WRITE_TRUNCATE)
                  .build();

          // Load data from a GCS Avro file into the table
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Job.html job = bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html#com_google_cloud_bigquery_BigQuery_create_com_google_cloud_bigquery_DatasetInfo_com_google_cloud_bigquery_BigQuery_DatasetOption____(JobInfo.of(loadConfig));
          // Blocks until this load table job completes its execution, either failing or succeeding.
          job = job.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Job.html#com_google_cloud_bigquery_Job_waitFor_com_google_cloud_bigquery_BigQueryRetryConfig_com_google_cloud_RetryOption____();
          if (job.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Job.html#com_google_cloud_bigquery_Job_isDone__()) {
            System.out.println("Table is successfully overwritten by AVRO file loaded from GCS");
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
     * This sample loads the Avro file at
     * https://storage.googleapis.com/cloud-samples-data/bigquery/us-states/us-states.avro
     *
     * TODO(developer): Replace the following lines with the path to your file.
     */
    const bucketName = 'cloud-samples-data';
    const filename = 'bigquery/us-states/us-states.avro';

    async function loadTableGCSAvroTruncate() {
      /**
       * Imports a GCS file into a table and overwrites
       * table data if table already exists.
       */

      /**
       * TODO(developer): Uncomment the following lines before running the sample.
       */
      // const datasetId = 'my_dataset';
      // const tableId = 'us_states';

      // Configure the load job. For full list of options, see:
      // https://cloud.google.com/bigquery/docs/reference/rest/v2/Job#JobConfigurationLoad
      const jobConfigurationLoad = {
        load: {
          sourceFormat: 'AVRO',
          writeDisposition: 'WRITE_TRUNCATE',
        },
      };

      // Load data from a Google Cloud Storage file into the table
      const [job] = await bigquery
        .dataset(datasetId)
        .table(tableId)
        .https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/bigquery/table.html(storage.bucket(bucketName).file(filename), jobConfigurationLoad);

      // load() waits for the job to finish
      console.log(`Job ${https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/bigquery/bigquery.html.id} completed.`);

      // Check the job's status for errors
      const errors = https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/bigquery/bigquery.html.status.errors;
      if (errors && errors.length > 0) {
        throw errors;
      }
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
        source_format=https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.enums.SourceFormat.html.AVRO,
    )

    uri = "gs://cloud-samples-data/bigquery/us-states/us-states.avro"
    load_job = client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_load_table_from_uri(
        uri, table_id, job_config=job_config
    )  # Make an API request.

    load_job.result()  # Waits for the job to complete.

    destination_table = client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_get_table(table_id)
    print("Loaded {} rows.".format(destination_table.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.table.Table.html#google_cloud_bigquery_table_Table_num_rows))

## Loading hive-partitioned Avro data

BigQuery supports loading hive-partitioned Avro data stored on
Cloud Storage and populates the hive-partitioning columns as columns in
the destination BigQuery managed table. For more information, see
[Loading Externally Partitioned Data from Cloud Storage](https://docs.cloud.google.com/bigquery/docs/hive-partitioned-loads-gcs).

## Avro conversions

BigQuery converts Avro data types to the following
BigQuery data types:

### Primitive types

| Avro data type without [logicalType attribute](https://docs.cloud.google.com/bigquery/docs/loading-data-cloud-storage-avro#logical-types) | BigQuery data type | Notes |
|---|---|---|
| null | BigQuery ignores these values |   |
| boolean | BOOLEAN |   |
| int | INTEGER |   |
| long | INTEGER |   |
| float | FLOAT |   |
| double | FLOAT |   |
| bytes | BYTES |   |
| string | STRING | UTF-8 only |

### Logical types

By default, BigQuery ignores the `logicalType` attribute for most
of the types and uses the underlying Avro type instead. To convert
Avro logical types to their corresponding BigQuery data types,
set the `--use_avro_logical_types` flag to `true` using
the bq command-line tool, or set the `useAvroLogicalTypes` property in the
[job resource](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs)
when you call the
[`jobs.insert`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs/insert)
method to create a load job.

The table below shows the conversion of Avro logical types to
BigQuery data types.

| Avro logical type | BigQuery data type: Logical type disabled | BigQuery data type: Logical type enabled |
|---|---|---|
| date | INTEGER | DATE |
| time-millis | INTEGER | TIME |
| time-micros | INTEGER (converted from LONG) | TIME |
| timestamp-millis | INTEGER (converted from LONG) | TIMESTAMP |
| timestamp-micros | INTEGER (converted from LONG) | TIMESTAMP |
| local-timestamp-millis | INTEGER (converted from LONG) | DATETIME |
| local-timestamp-micros | INTEGER (converted from LONG) | DATETIME |
| duration | BYTES (converted from `fixed` type of size 12) | BYTES (converted from `fixed` type of size 12) |
| decimal | NUMERIC, BIGNUMERIC, or STRING (see [Decimal logical type](https://docs.cloud.google.com/bigquery/docs/loading-data-cloud-storage-avro#decimal_logical_type)) | NUMERIC, BIGNUMERIC, or STRING (see [Decimal logical type](https://docs.cloud.google.com/bigquery/docs/loading-data-cloud-storage-avro#decimal_logical_type)) |

For more information on Avro data types, see the
[Apache Avro™ 1.8.2 Specification](https://avro.apache.org/docs/1.8.2/spec.html).

> [!NOTE]
> **Note:** When exporting to Avro from BigQuery, `DATETIME` is exported as a `STRING` with a custom logical time that is not recognized as a `DATETIME` upon importing back into BigQuery.

#### Date logical type

In any Avro file you intend to load, you must specify date logical types in the
following format:

    {
           "type": {"logicalType": "date", "type": "int"},
           "name": "date_field"
    }

#### Decimal logical type

`Decimal` logical types can be converted to `NUMERIC`, `BIGNUMERIC`
, or `STRING` types. The converted type depends
on the precision and scale parameters of the `decimal` logical type and the
specified decimal target types. Specify the decimal target type as follows:

- For a [load job](https://docs.cloud.google.com/bigquery/docs/batch-loading-data) using the [`jobs.insert`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs/insert) API: use the [`JobConfigurationLoad.decimalTargetTypes`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/Job#JobConfigurationLoad.FIELDS.decimal_target_types) field.
- For a [load job](https://docs.cloud.google.com/bigquery/docs/batch-loading-data) using the [`bq load`](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#bq_load) command in the bq command-line tool: use the [`--decimal_target_types`](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#flags_and_arguments_9) flag.
- For a query against a [table with external sources](https://docs.cloud.google.com/bigquery/external-data-sources): use the [`ExternalDataConfiguration.decimalTargetTypes`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables#ExternalDataConfiguration.FIELDS.decimal_target_types) field.
- For a [persistent external table created with DDL](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language): use the [`decimal_target_types`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#external_table_option_list) option.

For backward compatibility, if the decimal target types are not specified, you can
load an Avro file containing a `bytes` column with
the `decimal` logical type into a `BYTES` column of an existing table. In this
case, the `decimal` logical type on the column in the Avro file is ignored. This
conversion mode is deprecated and might be removed in the future.

For more information on the Avro `decimal` logical type, see the
[Apache Avro™ 1.8.2 Specification](https://avro.apache.org/docs/1.8.2/spec.html#Decimal).

#### Time logical type

In any Avro file you intend to load, you must specify time logical types in one
of the following formats.

For millisecond precision:

    {
           "type": {"logicalType": "time-millis", "type": "int"},
           "name": "time_millis_field"
    }

For microsecond precision:

    {
           "type": {"logicalType": "time-micros", "type": "int"},
           "name": "time_micros_field"
    }

#### Timestamp logical type

In any Avro file you intend to load, you must specify timestamp logical types
in one of the following formats.

For millisecond precision:

    {
           "type": {"logicalType": "timestamp-millis", "type": "long"},
           "name": "timestamp_millis_field"
    }

For microsecond precision:

    {
           "type": {"logicalType": "timestamp-micros", "type": "long"},
           "name": "timestamp_micros_field"
    }

#### Local-Timestamp logical type

In any Avro file you intend to load, you must specify a local-timestamp logical
type in one of the following formats.

For millisecond precision:

    {
           "type": {"logicalType": "local-timestamp-millis", "type": "long"},
           "name": "local_timestamp_millis_field"
    }

For microsecond precision:

    {
           "type": {"logicalType": "local-timestamp-micros", "type": "long"},
           "name": "local_timestamp_micros_field"
    }

### Complex types

| Avro data type | BigQuery data type | Notes |
|---|---|---|
| record | RECORD | - Aliases are ignored - Doc is converted into a [field description](https://docs.cloud.google.com/bigquery/docs/schemas#column_descriptions) - Default values are set at read time - Order is ignored - Recursive fields are dropped --- Only the first level of nesting is maintained for recursive fields |
| enum | STRING | - The string is the symbolic value of the enum - Aliases are ignored - Doc is converted into a [field description](https://docs.cloud.google.com/bigquery/docs/schemas#column_descriptions) |
| array | repeated fields | Arrays of arrays are not supported. Arrays containing only NULL types are ignored. |
| map\<T\> | RECORD | BigQuery converts an Avro map\<T\> field to a repeated RECORD that contains two fields: a key and a value. BigQuery stores the key as a STRING, and converts the value to its corresponding data type in BigQuery. |
| union | - Nullable field - RECORD with a list of nullable fields | - When union only has one non-null type, it converts to a nullable field. - Otherwise it converts to a RECORD with a list of nullable fields. Only one of these fields will be set at read time. |
| fixed | BYTES | - Aliases are ignored - Size is ignored |