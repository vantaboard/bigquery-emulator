# Using schema auto-detection

## Schema auto-detection

Schema auto-detection enables BigQuery to infer the schema for
CSV, JSON, or Google Sheets data. Schema auto-detection is available when you
[load](https://docs.cloud.google.com/bigquery/docs/loading-data) data into BigQuery and when
you query an [external data source](https://docs.cloud.google.com/bigquery/external-data-sources).

When auto-detection is enabled, BigQuery infers the data type for
each column. BigQuery selects a random file in the data source
and scans up to the first 500 rows of data to use as a representative sample.
BigQuery then examines each field and attempts to assign a data
type to that field based on the values in the sample. If all of the rows in a
column are empty, auto-detection will default to `STRING`
data type for the column.

If you don't enable schema auto-detection for CSV, JSON, or Google Sheets
data, then you must provide the schema manually when creating the table.

You don't need to enable schema auto-detection for Avro, Parquet, ORC, Firestore
export, or Datastore export files. These file formats are self-describing, so
BigQuery automatically infers the table schema from the source
data. For Parquet, Avro, and Orc files, you can optionally provide an explicit
schema to override the inferred schema.

You can see the detected schema for a table in the following ways:

- Use the Google Cloud console.
- Use the bq command-line tool's [`bq show`](https://docs.cloud.google.com/bigquery/bq-command-line-tool#tables) command.

When BigQuery detects schemas, it might, on rare occasions,
change a field name to make it compatible with GoogleSQL
syntax.

For information about data type conversions, see the following:

- [Data type conversion](https://docs.cloud.google.com/bigquery/docs/loading-data-cloud-datastore#data_type_conversion) when loading data from Datastore
- [Data type conversion](https://docs.cloud.google.com/bigquery/docs/loading-data-cloud-firestore#data_type_conversion) when loading data from Firestore
- [Avro conversions](https://docs.cloud.google.com/bigquery/docs/loading-data-cloud-storage-avro#avro_conversions)
- [Parquet conversions](https://docs.cloud.google.com/bigquery/docs/loading-data-cloud-storage-parquet#parquet_conversions)
- [ORC conversions](https://docs.cloud.google.com/bigquery/docs/loading-data-cloud-storage-orc#orc_conversions)

## Loading data using schema auto-detection

To enable schema auto-detection when loading data, use one of these approaches:

- In the Google Cloud console, in the **Schema** section, for **Auto detect** , check the **Schema and input parameters** option.
- In the bq command-line tool, use the `bq load` command with the `--autodetect` parameter.

When schema auto-detection is enabled, BigQuery makes a
best-effort attempt to automatically infer the schema for CSV and JSON files.
The auto-detection logic infers the schema field types by reading up to the
first 500 rows of data. Leading lines are skipped if the `--skip_leading_rows`
flag is present. The field types are based on the rows having the most fields.
Therefore, auto-detection should work as expected as long as there is at least
one row of data that has values in every column/field.

Schema auto-detection is not used with Avro files, Parquet files, ORC files,
Firestore export files, or Datastore export files. When you
load these files into BigQuery, the table schema is automatically
retrieved from the self-describing source data.

To use schema auto-detection when you load JSON or CSV data:

### Console

1. In the Google Cloud console, go to the BigQuery page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)

   If you don't see the left pane, click **Expand left pane** to open the pane.
3. In the **Explorer** pane, expand your project, click **Datasets**, and
   then click your dataset.

4. In the details pane, click **Create table**.

5. On the **Create table** page, in the **Source** section:

   - For **Create table from**, select the source type.
   - In the source field, browse for the File/Cloud Storage bucket, or
     enter the [Cloud Storage URI](https://docs.cloud.google.com/bigquery/docs/schema-detect#gcs-uri). Note that you cannot
     include multiple URIs in the Google Cloud console, but
     [wildcards](https://docs.cloud.google.com/bigquery/docs/batch-loading-data#load-wildcards)
     are supported. The Cloud Storage bucket must be in the same
     location as the dataset that contains the table you're creating.

     ![Select file.](https://docs.cloud.google.com/static/bigquery/images/create-table-select-file.png)
   - For **File format** , select **CSV** or **JSON**.

6. On the **Create table** page, in the **Destination** section:

   - For **Dataset name**, choose the appropriate dataset.

     ![Select dataset.](https://docs.cloud.google.com/static/bigquery/images/create-table-select-dataset.png)
   - In the **Table name** field, enter the name of the table you're
     creating.

   - Verify that **Table type** is set to **Native table**.

7. Click **Create table**.

### bq

Issue the `bq load` command with the `--autodetect` parameter.

(Optional) Supply the `--location` flag and set the value to your
[location](https://docs.cloud.google.com/bigquery/docs/dataset-locations).

The following command loads a file using schema auto-detect:

```bash
bq --location=LOCATION load \
--autodetect \
--source_format=FORMAT \
DATASET.TABLE \
PATH_TO_SOURCE
```

Replace the following:

- `LOCATION`: the name of your location. The `--location` flag is optional. For example, if you are using BigQuery in the Tokyo region, set the flag's value to `asia-northeast1`. You can set a default value for the location by using the [.bigqueryrc file](https://docs.cloud.google.com/bigquery/docs/bq-command-line-tool#setting_default_values_for_command-line_flags).
- `FORMAT`: either `NEWLINE_DELIMITED_JSON` or `CSV`.
- `DATASET`: the dataset that contains the table into which you're loading data.
- `TABLE`: the name of the table into which you're loading data.
- `PATH_TO_SOURCE`: is the location of the CSV or JSON file.

Examples:

Enter the following command to load `myfile.csv` from your local
machine into a table named `mytable` that is stored in a dataset named
`mydataset`.

    bq load --autodetect --source_format=CSV mydataset.mytable ./myfile.csv

Enter the following command to load `myfile.json` from your local
machine into a table named `mytable` that is stored in a dataset named
`mydataset`.

    bq load --autodetect --source_format=NEWLINE_DELIMITED_JSON \
    mydataset.mytable ./myfile.json

### API

1. Create a `load` job that points to the source data. For information about
   creating jobs, see
   [Running BigQuery jobs programmatically](https://docs.cloud.google.com/bigquery/docs/running-jobs).
   Specify your location in the `location` property in the `jobReference`
   section.

2. Specify the data format by setting the `sourceFormat` property. To use
   schema autodetection, this value must be set to `NEWLINE_DELIMITED_JSON`
   or `CSV`.

3. Use the `autodetect` property to set schema autodetection to `true`.

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

    // importJSONAutodetectSchema demonstrates loading data from newline-delimited JSON data in Cloud Storage
    // and using schema autodetection to identify the available columns.
    func importJSONAutodetectSchema(projectID, datasetID, tableID string) error {
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

    // Sample to load JSON data with autodetect schema from Cloud Storage into a new BigQuery table
    public class LoadJsonFromGCSAutodetect {

      public static void runLoadJsonFromGCSAutodetect() {
        // TODO(developer): Replace these variables before running the sample.
        String datasetName = "MY_DATASET_NAME";
        String tableName = "MY_TABLE_NAME";
        String sourceUri = "gs://cloud-samples-data/bigquery/us-states/us-states.json";
        loadJsonFromGCSAutodetect(datasetName, tableName, sourceUri);
      }

      public static void loadJsonFromGCSAutodetect(
          String datasetName, String tableName, String sourceUri) {
        try {
          // Initialize client that will be used to send requests. This client only needs to be created
          // once, and can be reused for multiple requests.
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html bigquery = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryOptions.html.getDefaultInstance().getService();

          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableId.html tableId = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableId.html.of(datasetName, tableName);
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.LoadJobConfiguration.html loadConfig =
              https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.LoadJobConfiguration.html.newBuilder(tableId, sourceUri)
                  .setFormatOptions(https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.FormatOptions.html.json())
                  .setAutodetect(true)
                  .build();

          // Load data from a GCS JSON file into the table
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Job.html job = bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html#com_google_cloud_bigquery_BigQuery_create_com_google_cloud_bigquery_DatasetInfo_com_google_cloud_bigquery_BigQuery_DatasetOption____(JobInfo.of(loadConfig));
          // Blocks until this load table job completes its execution, either failing or succeeding.
          job = job.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Job.html#com_google_cloud_bigquery_Job_waitFor_com_google_cloud_bigquery_BigQueryRetryConfig_com_google_cloud_RetryOption____();
          if (job.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Job.html#com_google_cloud_bigquery_Job_isDone__()) {
            System.out.println("Json Autodetect from GCS successfully loaded in a table");
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

    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryException.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryOptions.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.CsvOptions.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Job.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.JobInfo.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.LoadJobConfiguration.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableId.html;

    // Sample to load CSV data with autodetect schema from Cloud Storage into a new BigQuery table
    public class LoadCsvFromGcsAutodetect {

      public static void main(String[] args) {
        // TODO(developer): Replace these variables before running the sample.
        String datasetName = "MY_DATASET_NAME";
        String tableName = "MY_TABLE_NAME";
        String sourceUri = "gs://cloud-samples-data/bigquery/us-states/us-states.csv";
        loadCsvFromGcsAutodetect(datasetName, tableName, sourceUri);
      }

      public static void loadCsvFromGcsAutodetect(
          String datasetName, String tableName, String sourceUri) {
        try {
          // Initialize client that will be used to send requests. This client only needs to be created
          // once, and can be reused for multiple requests.
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html bigquery = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryOptions.html.getDefaultInstance().getService();

          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableId.html tableId = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableId.html.of(datasetName, tableName);

          // Skip header row in the file.
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.CsvOptions.html csvOptions = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.CsvOptions.html.newBuilder().setSkipLeadingRows(1).build();

          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.LoadJobConfiguration.html loadConfig =
              https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.LoadJobConfiguration.html.newBuilder(tableId, sourceUri)
                  .setFormatOptions(csvOptions)
                  .setAutodetect(true)
                  .build();

          // Load data from a GCS CSV file into the table
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Job.html job = bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html#com_google_cloud_bigquery_BigQuery_create_com_google_cloud_bigquery_DatasetInfo_com_google_cloud_bigquery_BigQuery_DatasetOption____(JobInfo.of(loadConfig));
          // Blocks until this load table job completes its execution, either failing or succeeding.
          job = job.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Job.html#com_google_cloud_bigquery_Job_waitFor_com_google_cloud_bigquery_BigQueryRetryConfig_com_google_cloud_RetryOption____();
          if (job.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Job.html#com_google_cloud_bigquery_Job_isDone__() && job.getStatus().getError() == null) {
            System.out.println("CSV Autodetect from GCS successfully loaded in a table");
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

    /**
     * TODO(developer): Uncomment the following lines before running the sample.
     */
    // const datasetId = "my_dataset";
    // const tableId = "my_table";

    /**
     * This sample loads the JSON file at
     * https://storage.googleapis.com/cloud-samples-data/bigquery/us-states/us-states.json
     *
     * TODO(developer): Replace the following lines with the path to your file.
     */
    const bucketName = 'cloud-samples-data';
    const filename = 'bigquery/us-states/us-states.json';

    async function loadJSONFromGCSAutodetect() {
      // Imports a GCS file into a table with autodetected schema.

      // Instantiate clients
      const bigquery = new https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/bigquery/bigquery.html();
      const storage = new https://docs.cloud.google.com/nodejs/docs/reference/storage-control/latest/storage-control/protos.google.storage.v2.storage-class.html();

      // Configure the load job. For full list of options, see:
      // https://cloud.google.com/bigquery/docs/reference/rest/v2/Job#JobConfigurationLoad
      const metadata = {
        sourceFormat: 'NEWLINE_DELIMITED_JSON',
        autodetect: true,
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
    loadJSONFromGCSAutodetect();

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

    /**
     * Imports data to the given table from json file present in GCS by auto
     * detecting options and schema.
     *
     * @param string $projectId The project Id of your Google Cloud Project.
     * @param string $datasetId The BigQuery dataset ID.
     * @param string $tableId The BigQuery table ID.
     */
    function import_from_storage_json_autodetect(
        string $projectId,
        string $datasetId,
        string $tableId = 'us_states'
    ): void {
        // instantiate the bigquery table service
        $bigQuery = new BigQueryClient([
          'projectId' => $projectId,
        ]);
        $dataset = $bigQuery->dataset($datasetId);
        $table = $dataset->table($tableId);

        // create the import job
        $gcsUri = 'gs://cloud-samples-data/bigquery/us-states/us-states.json';
        $loadConfig = $table->loadFromStorage($gcsUri)->autodetect(true)->sourceFormat('NEWLINE_DELIMITED_JSON');
        $job = $table->runJob($loadConfig);

        // check if the job is complete
        $job->reload();
        if (!$job->isComplete()) {
            throw new \Exception('Job has not yet completed', 500);
        }
        // check if the job has errors
        if (isset($job->info()['status']['errorResult'])) {
            $error = $job->info()['status']['errorResult']['message'];
            printf('Error running job: %s' . PHP_EOL, $error);
        } else {
            print('Data imported successfully' . PHP_EOL);
        }
    }

<br />

### Python

To enable schema auto-detection, set the
[LoadJobConfig.autodetect](https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.job.LoadJobConfig#google_cloud_bigquery_job_LoadJobConfig_autodetect)
property to `True`.


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

    # Set the encryption key to use for the destination.
    # TODO: Replace this key with a key you have created in KMS.
    # kms_key_name = "projects/{}/locations/{}/keyRings/{}/cryptoKeys/{}".format(
    #     "cloud-samples-tests", "us", "test", "test"
    # )
    job_config = https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.job.LoadJobConfig.html(
        autodetect=True, source_format=https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.enums.SourceFormat.html.NEWLINE_DELIMITED_JSON
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

    def load_table_gcs_json_autodetect dataset_id = "your_dataset_id"
      bigquery = Google::Cloud::https://docs.cloud.google.com/ruby/docs/reference/google-cloud-bigquery-connection/latest/Google-Cloud-Bigquery.html.https://docs.cloud.google.com/ruby/docs/reference/google-cloud-bigquery/latest/Google-Cloud-Bigquery.html
      dataset  = bigquery.dataset dataset_id
      gcs_uri  = "gs://cloud-samples-data/bigquery/us-states/us-states.json"
      table_id = "us_states"

      load_job = dataset.load_job table_id,
                                  gcs_uri,
                                  format:     "json",
                                  autodetect: true
      puts "Starting job #{load_job.https://docs.cloud.google.com/ruby/docs/reference/google-cloud-bigquery/latest/Google-Cloud-Bigquery-Job.html}"

      load_job.wait_until_done! # Waits for table load to complete.
      puts "Job finished."

      table = dataset.table table_id
      puts "Loaded #{table.https://docs.cloud.google.com/ruby/docs/reference/google-cloud-bigquery/latest/Google-Cloud-Bigquery-Table.html} rows to table #{table.id}"
    end

<br />

## Schema auto-detection for external data sources

Schema auto-detection can be used with CSV, JSON, and Google Sheets external
data sources. When schema auto-detection is enabled, BigQuery
makes a best-effort attempt to automatically infer the schema from the source
data. If you don't enable schema auto-detection for these sources, then you
must provide an explicit schema.

You don't need to enable schema auto-detection when you query external Avro,
Parquet, ORC, Firestore export, or Datastore export
files. These file formats are self-describing, so BigQuery
automatically infers the table schema from the source data. For Parquet,
Avro, and ORC files, you can optionally provide an explicit schema to
override the inferred schema.

Using the Google Cloud console, you can enable schema auto-detection by
checking the **Schema and input parameters** option for **Auto detect**.

Using the bq command-line tool, you can enable schema auto-detection when you
create a [table definition file](https://docs.cloud.google.com/bigquery/external-table-definition) for CSV,
JSON, or Google Sheets data. When using the bq tool to create a
table definition file, pass the `--autodetect` flag to the `mkdef` command to
enable schema auto-detection, or pass the `--noautodetect` flag to disable
auto-detection.

When you use the `--autodetect` flag, the `autodetect` setting is set to `true`
in the table definition file. When you use the `--noautodetect` flag, the
`autodetect` setting is set to `false`. If you don't provide a schema
definition for the external data source when you create a table definition, and
you don't use the `--noautodetect` or `--autodetect` flag, the `autodetect`
setting defaults to `true`.

When you create a table definition file by using the API, set the value of the
`autodetect` property to `true` or `false`. Setting `autodetect` to `true`
enables auto-detection. Setting `autodetect` to `false` disables autodetect.

## Auto-detection details

In addition to detecting schema details, auto-detection recognizes the
following:

### Compression

BigQuery recognizes gzip-compatible file compression when opening
a file.

### Date and time values

BigQuery detects date and time values based on the formatting of
the source data.

Values in `DATE` columns must be in the following format: `YYYY-MM-DD`.

Values in `TIME` columns must be in the following format: `HH:MM:SS[.SSSSSS]`
(the fractional-second component is optional).

For `TIMESTAMP` columns, BigQuery detects a wide array of
timestamp formats, including, but not limited to:

- `YYYY-MM-DD HH:MM`
- `YYYY-MM-DD HH:MM:SS`
- `YYYY-MM-DD HH:MM:SS.SSSSSS`
- `YYYY/MM/DD HH:MM`

A timestamp can also contain a UTC offset or the UTC zone designator ('Z').

Here are some examples of values that BigQuery will automatically
detect as timestamp values:

- 2018-08-19 12:11
- 2018-08-19 12:11:35.22
- 2018/08/19 12:11
- 2018-08-19 07:11:35.220 -05:00

If auto-detection isn't enabled, and your value is in a format not present in the
preceding examples, then BigQuery can only load the column as a
`STRING` data type. You can enable auto-detection to have BigQuery
recognize these columns as timestamps. For example, BigQuery will
only load `2025-06-16T16:55:22Z` as a timestamp if you enable auto-detection.

Alternatively, you can preprocess the source data
before loading it. For example, if you are exporting CSV data from a
spreadsheet, set the date format to match one of the examples shown here.
You can also transform the data after loading it into
BigQuery.

### Schema auto-detection for CSV data

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

### Schema auto-detection for JSON data

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

For more information, see
[Loading JSON data from Cloud Storage](https://docs.cloud.google.com/bigquery/docs/loading-data-cloud-storage-json).

### Schema auto-detection for Google Sheets

For Sheets, BigQuery auto-detects whether the
first row is a header row, similar to auto-detection for CSV files. If the first
line is identified as a header, BigQuery assigns column names
based on the field names in the header row and skips the row. The names might be
modified to meet the [naming rules](https://docs.cloud.google.com/bigquery/docs/schemas#column_names) for
columns in
BigQuery. For example, spaces will be replaced with underscores.

## Table security

To control access to tables in BigQuery, see
[Control access to resources with IAM](https://docs.cloud.google.com/bigquery/docs/control-access-to-resources-iam).