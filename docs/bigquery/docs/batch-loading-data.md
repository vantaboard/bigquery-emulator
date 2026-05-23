# Batch loading data

You can load data into BigQuery from Cloud Storage or from a local
file as a batch operation. The source data can be in any of the following
formats:

- Avro
- Comma-separated values (CSV)
- JSON (newline-delimited)
- ORC
- Parquet
- [Datastore](https://docs.cloud.google.com/datastore) exports stored in Cloud Storage
- [Firestore](https://docs.cloud.google.com/firestore) exports stored in Cloud Storage

You can also use [BigQuery Data Transfer Service](https://docs.cloud.google.com/bigquery/docs/cloud-storage-transfer) to set
up recurring loads from Cloud Storage into BigQuery.

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

## Loading data from Cloud Storage

BigQuery supports loading data from any of the following
Cloud Storage [storage classes](https://docs.cloud.google.com/storage/docs/storage-classes):

- Standard
- Nearline
- Coldline
- Archive

To learn how to load data into BigQuery, see the page for your
data format:

- [CSV](https://docs.cloud.google.com/bigquery/docs/loading-data-cloud-storage-csv)
- [JSON](https://docs.cloud.google.com/bigquery/docs/loading-data-cloud-storage-json)
- [Avro](https://docs.cloud.google.com/bigquery/docs/loading-data-cloud-storage-avro)
- [Parquet](https://docs.cloud.google.com/bigquery/docs/loading-data-cloud-storage-parquet)
- [ORC](https://docs.cloud.google.com/bigquery/docs/loading-data-cloud-storage-orc)
- [Datastore exports](https://docs.cloud.google.com/bigquery/docs/loading-data-cloud-datastore)
- [Firestore exports](https://docs.cloud.google.com/bigquery/docs/loading-data-cloud-firestore)

To learn how to configure a recurring load from Cloud Storage into
BigQuery, see
[Cloud Storage transfers](https://docs.cloud.google.com/bigquery/docs/cloud-storage-transfer).

### Location considerations

You cannot change the location of a dataset after it is created, but you can
make a copy of the dataset or manually move it. For more information, see:

- [Copying datasets](https://docs.cloud.google.com/bigquery/docs/copying-datasets)
- [Moving a dataset](https://docs.cloud.google.com/bigquery/docs/managing-datasets#recreate-dataset)

### Retrieving the Cloud Storage URI

To load data from a Cloud Storage data source, you must provide the
Cloud Storage URI.

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

For Google Datastore exports, only one URI can be specified, and it
must end with `.backup_info` or `.export_metadata`.

### Wildcard support for Cloud Storage URIs

If your data is separated into multiple files, you can use an asterisk (\*)
wildcard to select multiple files. Use of the asterisk wildcard must follow
these rules:

- The asterisk can appear inside the object name or at the end of the object name.
- Using multiple asterisks is unsupported. For example, the path `gs://mybucket/fed-*/temp/*.csv` is invalid.
- Using an asterisk with the bucket name is unsupported.

Examples:

- The following example shows how to select all of the files in all the
  folders which start with the prefix `gs://mybucket/fed-samples/fed-sample`:

      gs://mybucket/fed-samples/fed-sample*

- The following example shows how to select only files with a `.csv` extension
  in the folder named `fed-samples` and any subfolders of `fed-samples`:

      gs://mybucket/fed-samples/*.csv

- The following example shows how to select files with a naming pattern of
  `fed-sample*.csv` in the folder named `fed-samples`. This example doesn't
  select files in subfolders of `fed-samples`.

      gs://mybucket/fed-samples/fed-sample*.csv

When using the bq command-line tool, you might need to escape the asterisk on some
platforms.

You can't use an asterisk wildcard when you load Datastore or
Firestore export data from Cloud Storage.

### Limitations

You are subject to the following limitations when you load data into
BigQuery from a Cloud Storage bucket:

- BigQuery does not guarantee data consistency for external data sources. Changes to the underlying data while a query is running can result in unexpected behavior.
- BigQuery doesn't support [Cloud Storage object versioning](https://docs.cloud.google.com/storage/docs/object-versioning). If you include a generation number in the Cloud Storage URI, then the load job fails.

Depending on the format of your Cloud Storage source data, there may be
additional limitations. For more information, see:

- [CSV limitations](https://docs.cloud.google.com/bigquery/docs/loading-data-cloud-storage-csv#limitations)
- [JSON limitations](https://docs.cloud.google.com/bigquery/docs/loading-data-cloud-storage-json#limitations)
- [Datastore export limitations](https://docs.cloud.google.com/bigquery/docs/loading-data-cloud-datastore#limitations)
- [Firestore export limitations](https://docs.cloud.google.com/bigquery/docs/loading-data-cloud-firestore#limitations)
- [Limitations on nested and repeated data](https://docs.cloud.google.com/bigquery/docs/nested-repeated#limitations)

## Loading data from local files

You can load data from a readable data source (such as your local machine) by
using one of the following:

- The Google Cloud console
- The bq command-line tool's `bq load` command
- The API
- The client libraries

When you load data using the Google Cloud console or the bq command-line tool, a load
job is automatically created.

To load data from a local data source:

### Console

1. Open the BigQuery page in the Google Cloud console.

   [Go to the BigQuery page](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)

   If you don't see the left pane, click **Expand left pane** to open the pane.
3. In the **Explorer** pane, expand your project, click **Datasets**, and
   then select a dataset.

4. In the details pane, click **Create table**.

5. On the **Create table** page, in the **Source** section:

   - For **Create table from** , select **Upload**.
   - For **Select file** , click **Browse**.
   - Browse to the file, and click **Open**. Note that wildcards and comma-separated lists are not supported for local files.
   - For **File format** , select **CSV** , **JSON (newline delimited)** , **Avro** , **Parquet** , or **ORC**.
6. On the **Create table** page, in the **Destination** section:

   - For **Project**, choose the appropriate project.
   - For **Dataset**, choose the appropriate dataset.
   - In the **Table** field, enter the name of the table you're creating in BigQuery.
   - Verify that **Table type** is set to **Native table**.
7. In the **Schema** section, enter the [schema](https://docs.cloud.google.com/bigquery/docs/schemas)
   definition.

   - For CSV and JSON files, you can check the **Auto-detect** option to
     enable schema [auto-detect](https://docs.cloud.google.com/bigquery/docs/schema-detect). Schema
     information is self-described in the source data for other supported
     file types.

   - You can also enter schema information manually by:

     - Clicking **Edit as text** and entering the table schema as a JSON
       array:

       > [!NOTE]
       > **Note:** You can view the schema of an existing table in JSON format by entering the following command: `bq show --format=prettyjson dataset.table`.

     - Using **Add Field** to manually input the schema.

8. Select applicable items in the **Advanced options** section. For information on the available options, see
   [CSV options](https://docs.cloud.google.com/bigquery/docs/loading-data-cloud-storage-csv#csv-options)
   and [JSON options](https://docs.cloud.google.com/bigquery/docs/loading-data-cloud-storage-json#json-options).

9. Optional: In the **Advanced options** choose the write disposition:

   - **Write if empty**: Write the data only if the table is empty.
   - **Append to table**: Append the data to the end of the table. This setting is the default.
   - **Overwrite table**: Erase all existing data in the table before writing the new data.
10. Click **Create Table**.

### bq

Use the `bq load` command, specify the `source_format`, and include the path
to the local file.

(Optional) Supply the `--location` flag and set the value to your
[location](https://docs.cloud.google.com/bigquery/docs/locations).

If you are loading data in a project other than your default project, add
the project ID to the dataset in the following format:
`PROJECT_ID:DATASET`.

```bash
bq --location=LOCATION load \
--source_format=FORMAT \
PROJECT_ID:DATASET.TABLE \
PATH_TO_SOURCE \
SCHEMA
```

Replace the following:

- `LOCATION`: your location. The `--location` flag is optional. For example, if you are using BigQuery in the Tokyo region, set the flag's value to `asia-northeast1`. You can set a default value for the location by using the [.bigqueryrc file](https://docs.cloud.google.com/bigquery/docs/bq-command-line-tool#setting_default_values_for_command-line_flags).
- `FORMAT`: `CSV`, `AVRO`, `PARQUET`, `ORC`, or `NEWLINE_DELIMITED_JSON`.
- `project_id`: your project ID.
- `dataset`: an existing dataset.
- `table`: the name of the table into which you're loading data.
- `path_to_source`: the path to the local file.
- `schema`: a valid schema. The schema can be a local JSON file, or it can be typed inline as part of the command. You can also use the `--autodetect` flag instead of supplying a schema definition.

In addition, you can add flags for options that let you control how
BigQuery parses your data. For example, you can use the
`--skip_leading_rows` flag to ignore header rows in a CSV file. For more
information, see [CSV options](https://docs.cloud.google.com/bigquery/docs/loading-data-cloud-storage-csv#csv-options)
and [JSON options](https://docs.cloud.google.com/bigquery/docs/loading-data-cloud-storage-json#json-options).

Examples:

The following command loads a local newline-delimited JSON file
(`mydata.json`) into a table named `mytable` in `mydataset` in your default
project. The schema is defined in a local schema file named `myschema.json`.

        bq load \
        --source_format=NEWLINE_DELIMITED_JSON \
        mydataset.mytable \
        ./mydata.json \
        ./myschema.json

The following command loads a local CSV file (`mydata.csv`) into a table
named `mytable` in `mydataset` in `myotherproject`. The schema is defined
inline in the format
`FIELD:DATA_TYPE, FIELD:DATA_TYPE`.

        bq load \
        --source_format=CSV \
        myotherproject:mydataset.mytable \
        ./mydata.csv \
        qtr:STRING,sales:FLOAT,year:STRING

> [!NOTE]
> **Note:** When you specify the schema on the command line, you cannot include a `RECORD` ([`STRUCT`](https://docs.cloud.google.com/bigquery/docs/batch-loading-data#struct-type)) type, you cannot include a field description, and you cannot specify the field mode. All field modes default to `NULLABLE`. To include field descriptions, modes, and `RECORD` types, supply a [JSON schema file](https://docs.cloud.google.com/bigquery/docs/batch-loading-data#specifying_a_schema_file) instead.

The following command loads a local CSV file (`mydata.csv`) into a table
named `mytable` in `mydataset` in your default project. The schema is
defined using [schema auto-detection](https://docs.cloud.google.com/bigquery/docs/schema-detect).

        bq load \
        --autodetect \
        --source_format=CSV \
        mydataset.mytable \
        ./mydata.csv

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

The following code demonstrates how to load a local CSV file to a new BigQuery table. To load a local file of another format, use the update options class for the appropriate format from the [JobCreationOptions](https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.JobCreationOptions) base class instead of `UploadCsvOptions`.

<br />



    using https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.html;
    using System;
    using System.IO;

    public class BigQueryLoadFromFile
    {
        public void LoadFromFile(
            string projectId = "your-project-id",
            string datasetId = "your_dataset_id",
            string tableId = "your_table_id",
            string filePath = "path/to/file.csv"
        )
        {
            https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryClient.html client = https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryClient.html.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryClient.html#Google_Cloud_BigQuery_V2_BigQueryClient_Create_System_String_Google_Apis_Auth_OAuth2_GoogleCredential_(projectId);
            // Create job configuration
            var uploadCsvOptions = new https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.UploadCsvOptions.html()
            {
                SkipLeadingRows = 1,  // Skips the file headers
                Autodetect = true
            };
            using (FileStream stream = File.Open(filePath, FileMode.Open))
            {
                // Create and run job
                // Note that there are methods available for formats other than CSV
                https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryJob.html job = client.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryClient.html#Google_Cloud_BigQuery_V2_BigQueryClient_UploadCsv_Google_Apis_Bigquery_v2_Data_TableReference_Google_Apis_Bigquery_v2_Data_TableSchema_System_IO_Stream_Google_Cloud_BigQuery_V2_UploadCsvOptions_(
                    datasetId, tableId, null, stream, uploadCsvOptions);
                job = job.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryJob.html#Google_Cloud_BigQuery_V2_BigQueryJob_PollUntilCompleted_Google_Cloud_BigQuery_V2_GetJobOptions_Google_Api_Gax_PollSettings_().ThrowOnAnyError();  // Waits for the job to complete.

                // Display the number of rows uploaded
                https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryTable.html table = client.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryClient.html#Google_Cloud_BigQuery_V2_BigQueryClient_GetTable_Google_Apis_Bigquery_v2_Data_TableReference_Google_Cloud_BigQuery_V2_GetTableOptions_(datasetId, tableId);
                Console.WriteLine(
                    $"Loaded {table.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryTable.html#Google_Cloud_BigQuery_V2_BigQueryTable_Resource.NumRows} rows to {table.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryTable.html#Google_Cloud_BigQuery_V2_BigQueryTable_FullyQualifiedId}");
            }
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

The following code demonstrates how to load a local CSV file to a new BigQuery table. To load a local file of another format, set the [DataFormat](https://godoc.org/cloud.google.com/go/bigquery#DataFormat) property of the `NewReaderSource` to the appropriate format.

<br />


    import (
    	"context"
    	"fmt"
    	"os"

    	"cloud.google.com/go/bigquery"
    )

    // importCSVFromFile demonstrates loading data into a BigQuery table using a file on the local filesystem.
    func importCSVFromFile(projectID, datasetID, tableID, filename string) error {
    	// projectID := "my-project-id"
    	// datasetID := "mydataset"
    	// tableID := "mytable"
    	ctx := context.Background()
    	client, err := bigquery.NewClient(ctx, projectID)
    	if err != nil {
    		return fmt.Errorf("bigquery.NewClient: %v", err)
    	}
    	defer client.Close()

    	f, err := os.Open(filename)
    	if err != nil {
    		return err
    	}
    	source := bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_ReaderSource_NewReaderSource(f)
    	source.AutoDetect = true   // Allow BigQuery to determine schema.
    	source.SkipLeadingRows = 1 // CSV has a single header line.

    	loader := client.Dataset(datasetID).Table(tableID).https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_Table_LoaderFrom(source)

    	job, err := loader.Run(ctx)
    	if err != nil {
    		return err
    	}
    	status, err := job.Wait(ctx)
    	if err != nil {
    		return err
    	}
    	if err := status.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_JobStatus_Err(); err != nil {
    		return err
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

The following code demonstrates how to load a local CSV file to a new BigQuery table. To load a local file of another format, set the [FormatOptions](https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.FormatOptions) to the appropriate format.

<br />


    TableId tableId = TableId.of(datasetName, tableName);
    WriteChannelConfiguration writeChannelConfiguration =
        WriteChannelConfiguration.newBuilder(tableId).setFormatOptions(FormatOptions.csv()).build();
    // The location must be specified; other fields can be auto-detected.
    JobId jobId = JobId.newBuilder().setLocation(location).build();
    TableDataWriteChannel writer = bigquery.writer(jobId, writeChannelConfiguration);
    // Write data to writer
    try (OutputStream stream = Channels.newOutputStream(writer)) {
      Files.copy(csvPath, stream);
    }
    // Get load job
    Job job = writer.getJob();
    job = job.waitFor();
    LoadStatistics stats = job.getStatistics();
    return stats.getOutputRows();

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

The following code demonstrates how to load a local CSV file to a new BigQuery table. To load a local file of another format, set the `metadata` parameter of the [load](https://googleapis.dev/nodejs/bigquery/latest/Table.html#load) function to the appropriate format.

<br />


    // Imports the Google Cloud client library
    const {BigQuery} = require('https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/overview.html');
    const bigquery = new https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/bigquery/bigquery.html();

    async function loadLocalFile() {
      // Imports a local file into a table.

      /**
       * TODO(developer): Uncomment the following lines before running the sample.
       */
      // const filename = '/path/to/file.csv';
      // const datasetId = 'my_dataset';
      // const tableId = 'my_table';

      // Load data from a local file into the table
      const [job] = await bigquery
        .dataset(datasetId)
        .table(tableId)
        .https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/bigquery/table.html(filename);

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

The following code demonstrates how to load a local CSV file to a new BigQuery table. To load a local file of another format, set the [sourceFormat](https://docs.cloud.google.com/php/docs/reference/cloud-bigquery/latest/LoadJobConfiguration#_Google_Cloud_BigQuery_LoadJobConfiguration__sourceFormat__) to the appropriate format.

<br />


    use Google\Cloud\BigQuery\BigQueryClient;
    use Google\Cloud\Core\ExponentialBackoff;

    /** Uncomment and populate these variables in your code */
    // $projectId  = 'The Google project ID';
    // $datasetId  = 'The BigQuery dataset ID';
    // $tableId    = 'The BigQuery table ID';
    // $source     = 'The path to the CSV source file to import';

    // instantiate the bigquery table service
    $bigQuery = new BigQueryClient([
        'projectId' => $projectId,
    ]);
    $dataset = $bigQuery->dataset($datasetId);
    $table = $dataset->table($tableId);
    // create the import job
    $loadConfig = $table->load(fopen($source, 'r'))->sourceFormat('CSV');

    $job = $table->runJob($loadConfig);
    // poll the job until it is complete
    $backoff = new ExponentialBackoff(10);
    $backoff->execute(function () use ($job) {
        printf('Waiting for job to complete' . PHP_EOL);
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

The following code demonstrates how to load a local CSV file to a new BigQuery table. To load a local file of another format, set the [LoadJobConfig.source_format
property](https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.job.LoadJobConfig#google_cloud_bigquery_job_LoadJobConfig_source_format) to the appropriate format.

<br />


    from google.cloud import https://docs.cloud.google.com/python/docs/reference/bigquery/latest

    # Construct a BigQuery client object.
    client = https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html()

    # TODO(developer): Set table_id to the ID of the table to create.
    # table_id = "your-project.your_dataset.your_table_name"

    job_config = https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.job.LoadJobConfig.html(
        source_format=https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.enums.SourceFormat.html.CSV, skip_leading_rows=1, autodetect=True,
    )

    with open(file_path, "rb") as source_file:
        job = client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_load_table_from_file(source_file, table_id, job_config=job_config)

    https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.job.html.result()  # Waits for the job to complete.

    table = client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_get_table(table_id)  # Make an API request.
    print(
        "Loaded {} rows and {} columns to {}".format(
            table.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.table.Table.html#google_cloud_bigquery_table_Table_num_rows, len(table.schema), table_id
        )
    )

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

The following code demonstrates how to load a local CSV file to a new BigQuery table. To load a local file of another format, set the `format` parameter of the [Table#load_job](https://googleapis.dev/ruby/google-cloud-bigquery/latest/Google/Cloud/Bigquery/Table.html#load_job-instance_method) method to the appropriate format.

<br />


    require "google/cloud/bigquery"

    def load_from_file dataset_id = "your_dataset_id",
                       file_path  = "path/to/file.csv"
      bigquery = Google::Cloud::https://docs.cloud.google.com/ruby/docs/reference/google-cloud-bigquery-connection/latest/Google-Cloud-Bigquery.html.https://docs.cloud.google.com/ruby/docs/reference/google-cloud-bigquery/latest/Google-Cloud-Bigquery.html
      dataset  = bigquery.dataset dataset_id
      table_id = "new_table_id"

      # Infer the config.location based on the location of the referenced dataset.
      load_job = dataset.load_job table_id, file_path do |config|
        config.skip_leading = 1
        config.https://docs.cloud.google.com/ruby/docs/reference/google-cloud-bigquery/latest/Google-Cloud-Bigquery-External-DataSource.html   = true
      end
      load_job.wait_until_done! # Waits for table load to complete.

      table = dataset.table table_id
      puts "Loaded #{table.https://docs.cloud.google.com/ruby/docs/reference/google-cloud-bigquery/latest/Google-Cloud-Bigquery-Table.html} rows into #{table.id}"
    end

<br />

### Limitations

Loading data from a local data source is subject to the following limitations:

- Wildcards and comma-separated lists are not supported when you load files from a local data source. Files must be loaded individually.
- When using the Google Cloud console, files loaded from a local data source cannot exceed 100 MB. For larger files, load the file from Cloud Storage.

## Load job capacity

Similar to the on-demand mode for queries, by default, load jobs use a shared
pool of slots. If a load job uses the shared pool, the reservation
`default-pipeline` is displayed in the job details. BigQuery
doesn't guarantee the available capacity of this shared pool or load job
throughput.

To increase throughput or predictably control the capacity of your load jobs,
you can create a [slot reservation](https://docs.cloud.google.com/bigquery/docs/reservations-workload-management)
and assign dedicated `PIPELINE` slots to run load jobs. For more information, see
[Reservation assignments](https://docs.cloud.google.com/bigquery/docs/reservations-workload-management#assignments).

## Loading compressed and uncompressed data

For Avro, Parquet, and ORC formats, BigQuery supports
loading files where the file data has been compressed using a
supported codec. However, BigQuery doesn't support loading files
in these formats that have themselves been compressed, for example by using
the `gzip` utility.

The Avro binary format is the preferred format for loading both compressed and
uncompressed data. Avro data is faster to load because the data can be read in
parallel, even when the data blocks are compressed.
For a list of supported compression codecs, see
[Avro compression](https://docs.cloud.google.com/bigquery/docs/loading-data-cloud-storage-avro#avro_compression).

Parquet binary format is also a good choice because Parquet's efficient,
per-column encoding typically results in a better compression ratio and smaller
files. Parquet files also leverage compression techniques that allow files to be
loaded in parallel. For a list of supported compression codecs, see
[Parquet compression](https://docs.cloud.google.com/bigquery/docs/loading-data-cloud-storage-parquet#parquet_compression).

The ORC binary format offers benefits similar to the benefits of the Parquet
format. Data in ORC files is fast to load because data stripes can be read in
parallel. The rows in each data stripe are loaded sequentially. To optimize load
time, use a data stripe size of approximately 256 MB or less.
For a list of supported compression codecs, see
[ORC compression](https://docs.cloud.google.com/bigquery/docs/loading-data-cloud-storage-orc#orc_compression).

For other data formats such as CSV and JSON, BigQuery can load
uncompressed files significantly faster than compressed files because
uncompressed files can be read in parallel. Because uncompressed files are
larger, using them can lead to bandwidth limitations and higher Cloud Storage
costs for data staged in Cloud Storage prior to being loaded into
BigQuery. Keep in mind that line ordering isn't
guaranteed for compressed or uncompressed files. It's important to weigh these
tradeoffs depending on your use case.

In general, if bandwidth is limited, compress your CSV and JSON files by using
`gzip` before uploading them to Cloud Storage. `gzip` is the only
supported file compression type for CSV and JSON files when loading data into
BigQuery. If loading speed is important to your app and you have
a lot of bandwidth to load your data, leave your files uncompressed.

## Appending to or overwriting a table

You can load additional data into a table either from source files or by
appending query results. If the
schema of the data does not match the schema of the destination table or
partition, you can update the schema when you append to it or overwrite it.

If you update the schema when appending data, BigQuery allows
you to:

- Add new fields
- Relax `REQUIRED` fields to `NULLABLE`

If you are overwriting a table, the schema is always overwritten. Schema updates
are not restricted when you overwrite a table.

In the Google Cloud console, use the **Write preference** option to specify
what action to take when you load data from a source file or from a query
result. The bq command-line tool and the API include the following options:

| Console option | bq tool flag | BigQuery API property | Description |
|---|---|---|---|
| Write if empty | None | WRITE_EMPTY | Writes the data only if the table is empty. |
| Append to table | `--noreplace` or `--replace=false`; if `--replace` is unspecified, the default is append | WRITE_APPEND | (Default) Appends the data to the end of the table. |
| Overwrite table | `--replace` or `--replace=true` | WRITE_TRUNCATE | Erases all existing data in a table before writing the new data. |

## Quota policy

For information about the quota policy for batch loading data, see
[Load jobs](https://docs.cloud.google.com/bigquery/quotas#load_jobs) on the Quotas and limits page.

### View current quota usage

You can view your current usage of query, load, extract, or copy jobs by running
an `INFORMATION_SCHEMA` query to view metadata about the jobs ran over a
specified time period. You can compare your current usage against the [quota
limit](https://docs.cloud.google.com/bigquery/quotas#copy_jobs) to determine your quota usage for a
particular type of job. The following example query uses the
`INFORMATION_SCHEMA.JOBS` view to list the number of query, load, extract, and
copy jobs by project:

```googlesql
SELECT
  sum(case  when job_type="QUERY" then 1 else 0 end) as QRY_CNT,
  sum(case  when job_type="LOAD" then 1 else 0 end) as LOAD_CNT,
  sum(case  when job_type="EXTRACT" then 1 else 0 end) as EXT_CNT,
  sum(case  when job_type="COPY" then 1 else 0 end) as CPY_CNT
FROM `region-REGION_NAME`.INFORMATION_SCHEMA.JOBS_BY_PROJECT
WHERE date(creation_time)= CURRENT_DATE()
```

## Pricing

There is no charge for batch loading data into
BigQuery using the shared slot pool. For more information, see
[BigQuery data ingestion pricing](https://cloud.google.com/bigquery/pricing#data_ingestion_pricing).

If you attempt to load data from a Cloud Storage bucket that's in a
different location than the destination BigQuery dataset,
[data transfer charges](https://cloud.google.com/storage/pricing#network-buckets)
apply.

## Example use case

Suppose there is a nightly batch processing pipeline that needs to be
completed by a fixed deadline. Data needs to be available by this deadline for
further processing by another batch process to generate reports to be sent to a
regulator. This use case is common in regulated industries such as finance.

[Batch loading of data with load jobs](https://docs.cloud.google.com/bigquery/docs/batch-loading-data)
is the right approach for this use case because latency is not a concern
provided the deadline can be met. Ensure your Cloud Storage buckets
[meet the location requirements](https://docs.cloud.google.com/bigquery/docs/batch-loading-data#data-locations)
for loading data into the BigQuery dataset.

The result of a BigQuery load job is atomic; either all records
get inserted or none do. As a best practice, when inserting all data in a single
load job, create a new table by using the `WRITE_TRUNCATE` disposition of
the [`JobConfigurationLoad`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/Job#jobconfigurationquery)
resource.
This is important when retrying a failed load job, as the client might
not be able to distinguish between jobs that have failed and the failure
caused by for example in communicating the success state back to the client.

Assuming data to be ingested has been successfully copied to
Cloud Storage already, retrying with exponential backoff is sufficient
to address ingestion failures.

It's recommended that a nightly batch job doesn't hit the
[default quota](https://docs.cloud.google.com/bigquery/quotas#load_jobs)
of 1,500 loads per table per day even with retries. When loading data
incrementally, the default quota is sufficient for running a load job every 5
minutes and have unconsumed quota for at least 1 retry per job on average.