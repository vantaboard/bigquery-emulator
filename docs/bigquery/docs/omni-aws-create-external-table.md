# External tables for S3

This document describes how to create an Amazon Simple Storage Service (Amazon S3) BigLake table. A
[BigLake table](https://docs.cloud.google.com/bigquery/docs/biglake-intro) lets you use
access delegation to query data in Amazon S3. Access delegation
decouples access to the BigLake table from access to the
underlying datastore.

For information about how data flows between BigQuery and
Amazon S3, see [Data flow when querying data](https://docs.cloud.google.com/bigquery/docs/omni-introduction#query-data).

## Before you begin

Ensure that you have a [connection to access Amazon S3 data](https://docs.cloud.google.com/bigquery/docs/omni-aws-create-connection).

### Required roles


To get the permissions that
you need to create an external table,

ask your administrator to grant you the
[BigQuery Admin](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquery#bigquery.admin) (`roles/bigquery.admin`) IAM role on your dataset.


For more information about granting roles, see [Manage access to projects, folders, and organizations](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).


This predefined role contains

the permissions required to create an external table. To see the exact permissions that are
required, expand the **Required permissions** section:


#### Required permissions

The following permissions are required to create an external table:

- `bigquery.tables.create`
- `bigquery.connections.delegate`


You might also be able to get
these permissions
with [custom roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles) or
other [predefined roles](https://docs.cloud.google.com/iam/docs/roles-overview#predefined).

## Create a dataset

Before you create an external table, you need to create a dataset in the [supported region](https://docs.cloud.google.com/bigquery/docs/omni-introduction#locations). Select one of the following options:

<br />

### Console

1.
   Go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Explorer**.
3. In the **Explorer** pane, select the project where you want to create the dataset.
4. Click **View actions** , and then click **Create dataset**.
5. On the **Create dataset** page, specify the following details:
   1. For **Dataset ID** enter a unique dataset [name](https://docs.cloud.google.com/bigquery/docs/datasets#dataset-naming).
   2. For **Data location** choose a [supported region](https://docs.cloud.google.com/bigquery/docs/omni-introduction#locations).
   3. Optional: To delete tables automatically, select the **Enable table expiration** checkbox and set the **Default maximum table age** in days. Data in Amazon S3 is not deleted when the table expires.
   4. If you want to use [default collation](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/collation-concepts), expand the **Advanced options** section and then select the **Enable default collation** option.
   5. Click **Create dataset**.

### SQL

Use the [`CREATE SCHEMA` DDL statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_schema_statement).
The following example create a dataset in the `aws-us-east-1` region:


1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, enter the following statement:

   ```googlesql
   CREATE SCHEMA mydataset
   OPTIONS (
     location = 'aws-us-east-1');
   ```

   <br />

3. Click **Run**.

   <br />

For more information about how to run queries, see [Run an interactive query](https://docs.cloud.google.com/bigquery/docs/running-queries#queries).

### bq

In a command-line environment, create a dataset using the [`bq mk`
command](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#mk-dataset):

```bash
bq --location=LOCATION mk \
    --dataset \
PROJECT_ID:DATASET_NAME
```

The `--project_id` parameter overrides the default project.

Replace the following:

- `LOCATION`: the location of your dataset

  For information about supported regions, see
  [Locations](https://docs.cloud.google.com/bigquery/docs/omni-introduction#locations).
  After you
  create a dataset, you can't change its location. You can set a default
  value for the location by using the
  [`.bigqueryrc` file](https://docs.cloud.google.com/bigquery/docs/bq-command-line-tool#setting_default_values_for_command-line_flags).
- `PROJECT_ID`: your project ID

- `DATASET_NAME`: the name of the dataset that
  you want to create

  To create a dataset in a project other than your default project, add the
  project ID to the dataset name in the following format:
  `PROJECT_ID:DATASET_NAME`.

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
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Dataset.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.DatasetInfo.html;

    // Sample to create a aws dataset
    public class CreateDatasetAws {

      public static void main(String[] args) {
        // TODO(developer): Replace these variables before running the sample.
        String projectId = "MY_PROJECT_ID";
        String datasetName = "MY_DATASET_NAME";
        // Note: As of now location only supports aws-us-east-1
        String location = "aws-us-east-1";
        createDatasetAws(projectId, datasetName, location);
      }

      public static void createDatasetAws(String projectId, String datasetName, String location) {
        try {
          // Initialize client that will be used to send requests. This client only needs to be created
          // once, and can be reused for multiple requests.
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html bigquery = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryOptions.html.getDefaultInstance().getService();

          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.DatasetInfo.html datasetInfo =
              https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.DatasetInfo.html.newBuilder(projectId, datasetName).setLocation(location).build();

          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Dataset.html dataset = bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html#com_google_cloud_bigquery_BigQuery_create_com_google_cloud_bigquery_DatasetInfo_com_google_cloud_bigquery_BigQuery_DatasetOption____(datasetInfo);
          System.out.println(
              "Aws dataset created successfully :" + dataset.getDatasetId().getDataset());
        } catch (https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryException.html e) {
          System.out.println("Aws dataset was not created. \n" + e.toString());
        }
      }
    }

## Create BigLake tables on unpartitioned data

Select one of the following options:

### Console

1. Go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)

   If you don't see the left pane, click **Expand left pane** to open the pane.
3. In the **Explorer** pane, expand your project, click **Datasets**, and then select a dataset.

4. In the **Dataset info** section, click **Create table**.

5. On the **Create table** page, in the **Source** section, do the following:

   1. For **Create table from** , select **Amazon S3**.
   2. For **Select S3 path** , enter a URI pointing to the Amazon S3 data in the format `s3://BUCKET_NAME/PATH`. Replace `BUCKET_NAME` with the name of the Amazon S3 bucket; the bucket's region should be the same as the dataset's region. Replace `PATH` with the path that you would like to write the exported file to; it can contain one wildcard `*`.
   3. For **File format** , select the data format in Amazon S3. Supported formats are **AVRO** , **CSV** , **DELTA_LAKE** , **ICEBERG** , **JSONL** , **ORC** , and **PARQUET**.
6. In the **Destination** section, specify the
   following details:

   1. For **Dataset**, choose the appropriate dataset.
   2. In the **Table** field, enter the name of the table.
   3. Verify that **Table type** is set to **External table**.
   4. For **Connection ID** , choose the appropriate connection ID from the drop-down. For information about connections, see [Connect to Amazon S3](https://docs.cloud.google.com/bigquery/docs/omni-aws-create-connection).
7. In the **Schema** section, you can either enable
   [schema auto-detection](https://docs.cloud.google.com/bigquery/docs/schema-detect) or manually specify
   a schema if you have a source file. If you don't have a source file, you
   must manually specify a schema.

   - To enable schema auto-detection, select the **Auto-detect** option.

   - To manually specify a schema, leave the **Auto-detect** option
     unchecked. Enable **Edit as text** and enter the table schema as a
     [JSON array](https://docs.cloud.google.com/bigquery/docs/schemas#specifying_a_json_schema_file).

8. Click **Create table**.

### SQL

To create a BigLake table, use the
[`CREATE EXTERNAL TABLE` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_external_table_statement) with the
`WITH CONNECTION` clause:

<br />


1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, enter the following statement:

   ```googlesql
   CREATE EXTERNAL TABLE DATASET_NAME.TABLE_NAME
     WITH CONNECTION `AWS_LOCATION.CONNECTION_NAME`
     OPTIONS (
       format = "DATA_FORMAT",
       uris = ["S3_URI"],
       max_staleness = STALENESS_INTERVAL,
       metadata_cache_mode = 'CACHE_MODE');
   ```


   Replace the following:
   - `DATASET_NAME`: the name of the dataset you created
   - `TABLE_NAME`: the name you want to give to this table
   - `AWS_LOCATION`: an AWS location in Google Cloud (for example, \`aws-us-east-1\`)
   - `CONNECTION_NAME`: the name of the connection you created
   - `DATA_FORMAT`: any of the supported [BigQuery federated formats](https://docs.cloud.google.com/bigquery/external-data-sources) (such as `AVRO`, `CSV`, `DELTA_LAKE`, `ICEBERG`, or `PARQUET` ([preview](https://cloud.google.com/products/#product-launch-stages)))
   - `S3_URI`: a URI pointing to the Amazon S3 data (for example, `s3://bucket/path`)
   - `STALENESS_INTERVAL`: specifies whether cached metadata is used by operations against the BigLake table, and how fresh the cached metadata must be in order for the operation to use it. For more information about metadata caching considerations, see [Metadata caching for performance](https://docs.cloud.google.com/bigquery/docs/omni-introduction#metadata_caching_for_performance).

     To disable metadata caching, specify 0. This is the default.

     To enable metadata caching, specify an
     [interval literal](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/lexical#interval_literals)
     value between 30 minutes and 7 days. For example, specify
     `INTERVAL 4 HOUR` for a 4 hour staleness interval.
     With this value, operations against the table use cached metadata if
     it has been refreshed within the past 4 hours. If the cached metadata
     is older than that, the operation retrieves metadata from
     Amazon S3 instead.
   - `CACHE_MODE`: specifies whether the metadata cache is refreshed automatically or manually. For more information about metadata caching considerations, see [Metadata caching for performance](https://docs.cloud.google.com/bigquery/docs/omni-introduction#metadata_caching_for_performance).

     Set to `AUTOMATIC` for the metadata cache to be
     refreshed at a system-defined interval, usually somewhere between 30 and
     60 minutes.

     Set to `MANUAL` if you want to refresh
     the metadata cache on a schedule you determine. In this case, you can call
     the [`BQ.REFRESH_EXTERNAL_METADATA_CACHE` system procedure](https://docs.cloud.google.com/bigquery/docs/reference/system-procedures#bqrefresh_external_metadata_cache) to refresh the cache.

     You must set `CACHE_MODE` if
     `STALENESS_INTERVAL` is set to a value greater
     than 0.

   <br />

3. Click **Run**.

   <br />

For more information about how to run queries, see [Run an interactive query](https://docs.cloud.google.com/bigquery/docs/running-queries#queries).

Example:

```googlesql
CREATE EXTERNAL TABLE awsdataset.awstable
  WITH CONNECTION `aws-us-east-1.s3-read-connection`
  OPTIONS (
    format="CSV",
    uris=["s3://s3-bucket/path/file.csv"],
    max_staleness = INTERVAL 1 DAY,
    metadata_cache_mode = 'AUTOMATIC'
);
```

### bq

Create a [table definition file](https://docs.cloud.google.com/bigquery/external-table-definition):

```bash
bq mkdef  \
--source_format=DATA_FORMAT \
--connection_id=AWS_LOCATION.CONNECTION_NAME \
--metadata_cache_mode=CACHE_MODE \
S3_URI > table_def
```

Replace the following:

- `DATA_FORMAT`: any of the supported [BigQuery federated formats](https://docs.cloud.google.com/bigquery/external-data-sources) (such as `AVRO`, `CSV`, `DELTA_LAKE`, `ICEBERG`, or `PARQUET`).
- `S3_URI`: a URI pointing to the Amazon S3 data (for example, `s3://bucket/path`).
- `AWS_LOCATION`: an AWS location in Google Cloud (for example, `aws-us-east-1`).
- `CONNECTION_NAME`: the name of the connection you
  created.

- `CACHE_MODE`: specifies whether the metadata
  cache is refreshed automatically or manually. You only need to include
  this flag if you also plan to use the `--max_staleness` flag
  in the subsequent `bq mk` command to enable metadata caching.
  For more information about metadata caching considerations, see
  [Metadata caching for performance](https://docs.cloud.google.com/bigquery/docs/omni-introduction#metadata_caching_for_performance).

  Set to `AUTOMATIC` for the metadata cache to be
  refreshed at a system-defined interval, usually somewhere between 30 and
  60 minutes.

  Set to `MANUAL` if you want to refresh
  the metadata cache on a schedule you determine. In this case, you can call
  the [`BQ.REFRESH_EXTERNAL_METADATA_CACHE` system procedure](https://docs.cloud.google.com/bigquery/docs/reference/system-procedures#bqrefresh_external_metadata_cache) to refresh the cache.
  You must set `CACHE_MODE` if
  `STALENESS_INTERVAL` is set to a value greater
  than 0.

> [!NOTE]
> **Note:** To override the default project, use the `--project_id=PROJECT_ID` parameter. Replace `PROJECT_ID` with the ID of your Google Cloud project.

Next, create the BigLake table:

```bash
bq mk --max_staleness=STALENESS_INTERVAL --external_table_definition=table_def DATASET_NAME.TABLE_NAME
```

Replace the following:

- `STALENESS_INTERVAL`: specifies whether
  cached metadata is used by operations against the BigLake
  table, and how fresh the cached metadata must be in order for the
  operation to use it. For more information about metadata caching
  considerations, see
  [Metadata caching for performance](https://docs.cloud.google.com/bigquery/docs/omni-introduction#metadata_caching_for_performance).

  To disable metadata caching, specify 0. This is the default.

  To enable metadata caching, specify an
  [interval literal](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/lexical#interval_literals)
  value between 30 minutes and 7 days. For example, specify
  `INTERVAL 4 HOUR` for a 4 hour staleness interval.
  With this value, operations against the table use cached metadata if
  it has been refreshed within the past 4 hours. If the cached metadata
  is older than that, the operation retrieves metadata from
  Amazon S3 instead.
- `DATASET_NAME`: the name of the dataset you created.

- `TABLE_NAME`: the name you want to give to this table.

For example, the following command creates a new BigLake table,
`awsdataset.awstable`, which can query your Amazon S3 data that's stored
at the path `s3://s3-bucket/path/file.csv` and has a read connection in the
location `aws-us-east-1`:

```bash
bq mkdef  \
--autodetect \
--source_format=CSV \
--connection_id=aws-us-east-1.s3-read-connection \
--metadata_cache_mode=AUTOMATIC \
s3://s3-bucket/path/file.csv > table_def

bq mk --max_staleness=INTERVAL "1" HOUR \
--external_table_definition=table_def awsdataset.awstable
```

### API

Call the [`tables.insert` method](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables/insert)
API method, and create an
[`ExternalDataConfiguration`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables#externaldataconfiguration)
in the [`Table` resource](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables#Table)
that you pass in.

Specify the `schema` property or set the
`autodetect` property to `true` to enable schema auto detection for
supported data sources.

Specify the `connectionId` property to identify the connection to use
for connecting to Amazon S3.

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
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.ExternalTableDefinition.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Field.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Schema.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.StandardSQLTypeName.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableId.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableInfo.html;

    // Sample to create an external aws table
    public class CreateExternalTableAws {

      public static void main(String[] args) {
        // TODO(developer): Replace these variables before running the sample.
        String projectId = "MY_PROJECT_ID";
        String datasetName = "MY_DATASET_NAME";
        String tableName = "MY_TABLE_NAME";
        String connectionId = "MY_CONNECTION_ID";
        String sourceUri = "s3://your-bucket-name/";
        https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.CsvOptions.html options = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.CsvOptions.html.newBuilder().setSkipLeadingRows(1).build();
        https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Schema.html schema =
            https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Schema.html.of(
                https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Field.html.of("name", https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.StandardSQLTypeName.html.STRING),
                https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Field.html.of("post_abbr", https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.StandardSQLTypeName.html.STRING));
        https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.ExternalTableDefinition.html externalTableDefinition =
            https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.ExternalTableDefinition.html.newBuilder(sourceUri, options)
                .setConnectionId(connectionId)
                .setSchema(schema)
                .build();
        createExternalTableAws(projectId, datasetName, tableName, externalTableDefinition);
      }

      public static void createExternalTableAws(
          String projectId,
          String datasetName,
          String tableName,
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.ExternalTableDefinition.html externalTableDefinition) {
        try {
          // Initialize client that will be used to send requests. This client only needs to be created
          // once, and can be reused for multiple requests.
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html bigquery = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryOptions.html.getDefaultInstance().getService();

          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableId.html tableId = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableId.html.of(projectId, datasetName, tableName);
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableInfo.html tableInfo = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableInfo.html.newBuilder(tableId, externalTableDefinition).build();

          bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html#com_google_cloud_bigquery_BigQuery_create_com_google_cloud_bigquery_DatasetInfo_com_google_cloud_bigquery_BigQuery_DatasetOption____(tableInfo);
          System.out.println("Aws external table created successfully");

          // Clean up
          bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html#com_google_cloud_bigquery_BigQuery_delete_com_google_cloud_bigquery_DatasetId_com_google_cloud_bigquery_BigQuery_DatasetDeleteOption____(TableId.of(projectId, datasetName, tableName));
        } catch (https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryException.html e) {
          System.out.println("Aws external was not created." + e.toString());
        }
      }
    }

## Create BigLake tables on partitioned data

You can create a BigLake table for Hive partitioned data in
Amazon S3. After you create an externally partitioned table, you can't
change the partition key. You need to recreate the table to change the
partition key.

To create a BigLake table based on Hive partitioned data,
select one of the following options:

### Console

1. Go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)
3. In the **Explorer** pane, expand your project, click **Datasets**, and
   select a dataset.

4. Click **Create table** . This opens the **Create table** pane.

5. In the **Source** section, specify the following details:

   1. For **Create table from** , select **Amazon S3**.

   2. Provide the path to the folder, using
      [wildcards](https://docs.cloud.google.com/bigquery/docs/batch-loading-data#load-wildcards).
      For example, `s3://mybucket/*`.

      The folder
      must be in the same location as the dataset that contains the
      table you want to create, append, or overwrite.
   3. From the **File format** list, select the file type.

   4. Select the **Source data partitioning** checkbox, and then specify
      the following details:

      1. For **Select Source URI Prefix** , enter the URI prefix. For example, `s3://mybucket/my_files`.
      2. Optional: To require a partition filter on all queries for this table, select the **Require partition filter** checkbox. Requiring a partition filter can reduce cost and improve performance. For more information, see [Requiring predicate filters on partition keys in queries](https://docs.cloud.google.com/bigquery/docs/hive-partitioned-queries-gcs#requiring_predicate_filters_on_partition_keys_in_queries).
      3. In the **Partition inference mode** section, select one of the
         following options:

         - **Automatically infer types** : set the partition schema detection mode to `AUTO`.
         - **All columns are strings** : set the partition schema detection mode to `STRINGS`.
         - **Provide my own** : set the partition schema detection mode to `CUSTOM` and manually enter the schema information for the partition keys. For more information, see [Custom partition key schema](https://docs.cloud.google.com/bigquery/docs/hive-partitioned-loads-gcs#custom_partition_key_schema).
6. In the **Destination** section, specify the following details:

   1. For **Project**, select the project in which you want to create the table.
   2. For **Dataset**, select the dataset in which you want to create the table.
   3. For **Table**, enter the name of the table that you want to create.
   4. For **Table type** , verify that **External table** is selected.
   5. For **Connection ID**, select the connection that you created earlier.
7. In the **Schema** section, you can either enable
   [schema auto-detection](https://docs.cloud.google.com/bigquery/docs/schema-detect) or manually specify
   a schema if you have a source file. If you don't have a source file, you
   must manually specify a schema.

   - To enable schema auto-detection, select the **Auto-detect** option.

   - To manually specify a schema, leave the **Auto-detect** option
     unchecked. Enable **Edit as text** and enter the table schema as a
     [JSON array](https://docs.cloud.google.com/bigquery/docs/schemas#specifying_a_json_schema_file).

8. To ignore rows with extra column values that don't match the schema,
   expand the **Advanced options** section and select **Unknown values**.

9. Click **Create table**.

### SQL

Use the
[`CREATE EXTERNAL TABLE` DDL statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_external_table_statement):

<br />

<br />

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, enter the following statement:

   ```googlesql
   CREATE EXTERNAL TABLE `PROJECT_ID.DATASET.EXTERNAL_TABLE_NAME`
   WITH PARTITION COLUMNS
   (
     PARTITION_COLUMN PARTITION_COLUMN_TYPE,
   )
   WITH CONNECTION `PROJECT_ID.REGION.CONNECTION_ID`
   OPTIONS (
     hive_partition_uri_prefix = "HIVE_PARTITION_URI_PREFIX",
     uris=['FILE_PATH'],
     format ="TABLE_FORMAT"
     max_staleness = STALENESS_INTERVAL,
     metadata_cache_mode = 'CACHE_MODE'
   );
   ```


   Replace the following:
   - `PROJECT_ID`: the name of your project in which you want to create the table---for example, `myproject`
   - `DATASET`: the name of the BigQuery dataset that you want to create the table in---for example, `mydataset`
   - `EXTERNAL_TABLE_NAME`: the name of the table that you want to create---for example, `mytable`
   - `PARTITION_COLUMN`: the name of the partitioning column
   - `PARTITION_COLUMN_TYPE`: the type of the partitioning column
   - `REGION`: the region that contains the connection---for example, `us`
   - `CONNECTION_ID`: the name of the connection---for example, `myconnection`
   - `HIVE_PARTITION_URI_PREFIX`: hive partitioning uri prefix--for example: `s3://mybucket/`
   - `FILE_PATH`: path to the data source for the external table that you want to create---for example: `s3://mybucket/*.parquet`
   - `TABLE_FORMAT`: the format of the table that you want to create---for example, `PARQUET`
   - `STALENESS_INTERVAL`: specifies whether cached metadata is used by operations against the BigLake table, and how fresh the cached metadata must be in order for the operation to use it. For more information about metadata caching considerations, see [Metadata caching for performance](https://docs.cloud.google.com/bigquery/docs/omni-introduction#metadata_caching_for_performance).

     To disable metadata caching, specify 0. This is the default.

     To enable metadata caching, specify an
     [interval literal](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/lexical#interval_literals)
     value between 30 minutes and 7 days. For example, specify
     `INTERVAL 4 HOUR` for a 4 hour staleness interval.
     With this value, operations against the table use cached metadata if
     it has been refreshed within the past 4 hours. If the cached metadata
     is older than that, the operation retrieves metadata from
     Amazon S3 instead.
   - `CACHE_MODE`: specifies whether the metadata cache is refreshed automatically or manually. For more information about metadata caching considerations, see [Metadata caching for performance](https://docs.cloud.google.com/bigquery/docs/omni-introduction#metadata_caching_for_performance).

     Set to `AUTOMATIC` for the metadata cache to be
     refreshed at a system-defined interval, usually somewhere between 30 and
     60 minutes.

     Set to `MANUAL` if you want to refresh
     the metadata cache on a schedule you determine. In this case, you can call
     the [`BQ.REFRESH_EXTERNAL_METADATA_CACHE` system procedure](https://docs.cloud.google.com/bigquery/docs/reference/system-procedures#bqrefresh_external_metadata_cache) to refresh the cache.

     You must set `CACHE_MODE` if
     `STALENESS_INTERVAL` is set to a value greater
     than 0.

   <br />

3. Click **Run**.

   <br />

For more information about how to run queries, see [Run an interactive query](https://docs.cloud.google.com/bigquery/docs/running-queries#queries).

**Examples**

The following example creates a BigLake table over
partitioned data in Amazon S3. The schema is autodetected.

```googlesql
CREATE EXTERNAL TABLE `my_dataset.my_table`
WITH PARTITION COLUMNS
(
  sku STRING,
)
WITH CONNECTION `us.my-connection`
OPTIONS(
  hive_partition_uri_prefix = "s3://mybucket/products",
  uris = ['s3://mybucket/products/*']
  max_staleness = INTERVAL 1 DAY,
  metadata_cache_mode = 'AUTOMATIC'
);
```

### bq

First, use the
[`bq mkdef`](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#bq_mkdef) command to
create a table definition file:

```bash
bq mkdef \
--source_format=SOURCE_FORMAT \
--connection_id=REGION.CONNECTION_ID \
--hive_partitioning_mode=PARTITIONING_MODE \
--hive_partitioning_source_uri_prefix=URI_SHARED_PREFIX \
--require_hive_partition_filter=BOOLEAN \
--metadata_cache_mode=CACHE_MODE \
 URIS > DEFINITION_FILE
```

Replace the following:

- `SOURCE_FORMAT`: the format of the external data source. For example, `CSV`.
- `REGION`: the region that contains the connection---for example, `us`.
- `CONNECTION_ID`: the name of the connection---for example, `myconnection`.
- `PARTITIONING_MODE`: the Hive partitioning mode. Use one of the following values:
  - `AUTO`: Automatically detect the key names and types.
  - `STRINGS`: Automatically convert the key names to strings.
  - `CUSTOM`: Encode the key schema in the source URI prefix.
- `URI_SHARED_PREFIX`: the source URI prefix.
- `BOOLEAN`: specifies whether to require a predicate filter at query
  time. This flag is optional. The default value is `false`.

- `CACHE_MODE`: specifies whether the metadata
  cache is refreshed automatically or manually. You only need to include
  this flag if you also plan to use the `--max_staleness` flag
  in the subsequent `bq mk` command to enable metadata caching.
  For more information about metadata caching considerations, see
  [Metadata caching for performance](https://docs.cloud.google.com/bigquery/docs/omni-introduction#metadata_caching_for_performance).

  Set to `AUTOMATIC` for the metadata cache to be
  refreshed at a system-defined interval, usually somewhere between 30 and
  60 minutes.

  Set to `MANUAL` if you want to refresh
  the metadata cache on a schedule you determine. In this case, you can call
  the [`BQ.REFRESH_EXTERNAL_METADATA_CACHE` system procedure](https://docs.cloud.google.com/bigquery/docs/reference/system-procedures#bqrefresh_external_metadata_cache) to refresh the cache.
  You must set `CACHE_MODE` if
  `STALENESS_INTERVAL` is set to a value greater
  than 0.
- `URIS`: the path to the Amazon S3 folder, using
  wildcard format.

- `DEFINITION_FILE`: the path to the
  [table definition file](https://docs.cloud.google.com/bigquery/external-table-definition) on your local
  machine.

If `PARTITIONING_MODE` is `CUSTOM`, include the partition key schema
in the source URI prefix, using the following format:

```bash
--hive_partitioning_source_uri_prefix=URI_SHARED_PREFIX/{KEY1:TYPE1}/{KEY2:TYPE2}/...
```

After you create the table definition file, use the
[`bq mk`](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#mk-table) command to
create the BigLake table:

```bash
bq mk --max_staleness=STALENESS_INTERVAL \
--external_table_definition=DEFINITION_FILE \
DATASET_NAME.TABLE_NAME \
SCHEMA
```

Replace the following:

- `STALENESS_INTERVAL`: specifies whether
  cached metadata is used by operations against the BigLake
  table, and how fresh the cached metadata must be in order for the
  operation to use it. For more information about metadata caching
  considerations, see
  [Metadata caching for performance](https://docs.cloud.google.com/bigquery/docs/omni-introduction#metadata_caching_for_performance).

  To disable metadata caching, specify 0. This is the default.

  To enable metadata caching, specify an
  [interval literal](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/lexical#interval_literals)
  value between 30 minutes and 7 days. For example, specify
  `INTERVAL 4 HOUR` for a 4 hour staleness interval.
  With this value, operations against the table use cached metadata if
  it has been refreshed within the past 4 hours. If the cached metadata
  is older than that, the operation retrieves metadata from
  Amazon S3 instead.
- `DEFINITION_FILE`: the path to the table definition file.

- `DATASET_NAME`: the name of the dataset that contains the
  table.

- `TABLE_NAME`: the name of the table you're creating.

- `SCHEMA`: specifies a path to a
  [JSON schema file](https://docs.cloud.google.com/bigquery/docs/schemas#specifying_a_json_schema_file),
  or specifies the schema in the form
  `field:data_type,field:data_type,...`. To use schema
  auto-detection, omit this argument.

**Examples**

The following example uses `AUTO` Hive partitioning mode for Amazon S3
data:

    bq mkdef --source_format=CSV \
      --connection_id=us.my-connection \
      --hive_partitioning_mode=AUTO \
      --hive_partitioning_source_uri_prefix=s3://mybucket/myTable \
      --metadata_cache_mode=AUTOMATIC \
      s3://mybucket/* > mytable_def

    bq mk --max_staleness=INTERVAL "1" HOUR \
      --external_table_definition=mytable_def \
      mydataset.mytable \
      Region:STRING,Quarter:STRING,Total_sales:INTEGER

The following example uses `STRING` Hive partitioning mode for Amazon S3
data:

    bq mkdef --source_format=CSV \
      --connection_id=us.my-connection \
      --hive_partitioning_mode=STRING \
      --hive_partitioning_source_uri_prefix=s3://mybucket/myTable \
      --metadata_cache_mode=AUTOMATIC \
      s3://mybucket/myTable/* > mytable_def

    bq mk --max_staleness=INTERVAL "1" HOUR \
      --external_table_definition=mytable_def \
      mydataset.mytable \
      Region:STRING,Quarter:STRING,Total_sales:INTEGER

### API

To set Hive partitioning using the BigQuery API, include the
[`hivePartitioningOptions`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables#hivepartitioningoptions)
object in the [`ExternalDataConfiguration`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables#externaldataconfiguration)
object when you create the [table definition file](https://docs.cloud.google.com/bigquery/external-table-definition).
To create a BigLake table, you must also specify a value
for the `connectionId` field.

If you set the `hivePartitioningOptions.mode` field to `CUSTOM`, you must
encode the partition key schema in the
`hivePartitioningOptions.sourceUriPrefix` field as follows:
`s3://BUCKET/PATH_TO_TABLE/{KEY1:TYPE1}/{KEY2:TYPE2}/...`

To enforce the use of a predicate filter at query time, set the
`hivePartitioningOptions.requirePartitionFilter` field to `true`.

## Delta Lake tables

Delta Lake is an open source table format that supports petabyte scale data
tables. Delta Lake tables can be queried as both temporary and permanent tables,
and is supported as a [BigLake
table](https://docs.cloud.google.com/bigquery/docs/biglake-intro).

### Schema synchronization

Delta Lake maintains a canonical schema as part of its metadata. You
can't update a schema using a JSON metadata file. To update the schema:

1. Use the [`bq update` command](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#bq_update)
   with the `--autodetect_schema` flag:

   ```sh
   bq update --autodetect_schema
   PROJECT_ID:DATASET.TABLE
   ```

   Replace the following:
   - `PROJECT_ID`: the project ID containing the table that you
     want to update

   - `DATASET`: the dataset containing the table that you
     want to update

   - `TABLE`: the table that you want to update

### Type conversion

BigQuery converts Delta Lake data types to the following
BigQuery data types:

| **Delta Lake Type** | **BigQuery Type** |
|---|---|
| `boolean` | `BOOL` |
| `byte` | `INT64` |
| `int` | `INT64` |
| `long` | `INT64` |
| `float` | `FLOAT64` |
| `double` | `FLOAT64` |
| `Decimal(P/S)` | `NUMERIC` or `BIG_NUMERIC` depending on precision |
| `date` | `DATE` |
| `time` | `TIME` |
| `timestamp (not partition column)` | `TIMESTAMP` |
| `timestamp (partition column)` | `DATETIME` |
| `string` | `STRING` |
| `binary` | `BYTES` |
| `array<Type>` | `ARRAY<Type>` |
| `struct` | `STRUCT` |
| `map<KeyType, ValueType>` | `ARRAY<Struct<key KeyType, value ValueType>>` |

### Limitations

The following limitations apply to Delta Lake tables:

- [External table limitations](https://docs.cloud.google.com/bigquery/docs/external-tables#limitations) apply
  to Delta Lake tables.

- Delta Lake tables are only supported on
  BigQuery Omni and have the associated
  [limitations](https://docs.cloud.google.com/bigquery/docs/omni-introduction#limitations).

- You can't update a table with a new JSON metadata file. You must use an auto
  detect schema table update operation. See [Schema
  synchronization](https://docs.cloud.google.com/bigquery/docs/omni-aws-create-external-table#schema_synchronization) for more information.

- BigLake security features only protect Delta Lake
  tables when accessed through BigQuery services.

### Create a Delta Lake table

The following example creates an external table by using the [`CREATE EXTERNAL
TABLE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_external_table_statement)
statement with the
Delta Lake format:

```googlesql
CREATE [OR REPLACE] EXTERNAL TABLE table_name
WITH CONNECTION connection_name
OPTIONS (
         format = 'DELTA_LAKE',
         uris = ["parent_directory"]
       );
```

Replace the following:

- <var translate="no">table_name</var>: The name of the table.

- <var translate="no">connection_name</var>: The name of the connection. The connection must
  identify either an
  [Amazon S3](https://docs.cloud.google.com/bigquery/docs/omni-aws-create-connection) or a
  [Blob Storage](https://docs.cloud.google.com/bigquery/docs/omni-azure-create-connection) source.

- <var translate="no">parent_directory</var>: The URI of the parent directory.

### Cross-cloud transfer with Delta Lake

The following example uses the [`LOAD DATA`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/other-statements#load_data_statement)
statement to load data to the appropriate table:

```googlesql
LOAD DATA [INTO | OVERWRITE] table_name
FROM FILES (
        format = 'DELTA_LAKE',
        uris = ["parent_directory"]
)
WITH CONNECTION connection_name;
```

For more examples of cross-cloud data transfers, see [Load data with cross cloud
operations](https://docs.cloud.google.com/bigquery/docs/load-data-using-cross-cloud-transfer#load-data).

## Query BigLake tables

For more information, see
[Query Amazon S3 data](https://docs.cloud.google.com/bigquery/docs/query-aws-data).

## View resource metadata

> [!WARNING]
>
> **Preview**
>
>
> This feature is
>
> subject to the "Pre-GA Offerings Terms" in the General Service Terms section of the
> [Service Specific
> Terms](https://docs.cloud.google.com/terms/service-terms#1).
>
> Pre-GA features are available "as is" and might have limited support.
>
> For more information, see the
> [launch stage descriptions](https://cloud.google.com/products/#product-launch-stages).

You can view the resource metadata with [`INFORMATION_SCHEMA`](https://docs.cloud.google.com/bigquery/docs/information-schema-intro) views. When you query the [`JOBS_BY_*`](https://docs.cloud.google.com/bigquery/docs/information-schema-jobs), [`JOBS_TIMELINE_BY_*`](https://docs.cloud.google.com/bigquery/docs/information-schema-jobs-timeline), and [`RESERVATION*`](https://docs.cloud.google.com/bigquery/docs/information-schema-reservations) views, you must [specify the query's processing location](https://docs.cloud.google.com/bigquery/docs/locations#specify_locations) that is collocated with the table's region. For information about BigQuery Omni locations, see [Locations](https://docs.cloud.google.com/bigquery/docs/locations#omni-loc). For all other system tables, specifying the query job location is *optional* .

<br />

For information about the system tables that BigQuery Omni supports, see
[Limitations](https://docs.cloud.google.com/bigquery/docs/omni-introduction#limitations).

To query `JOBS_*` and `RESERVATION*` system tables, select one of the following
methods to specify the processing location:

### Console

1. Go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. If the **Editor** tab isn't visible, then click
   **Compose new query**.

3. Click **More** \> **Query settings** . The **Query settings**
   dialog opens.

4. In the **Query settings** dialog, for **Additional settings** \>
   **Data location** , select the [BigQuery region](https://docs.cloud.google.com/bigquery/docs/locations#omni-loc)
   that is collocated with the BigQuery Omni region.
   For example, if your BigQuery Omni region is `aws-us-east-1`,
   specify `us-east4`.

5. Select the remaining fields and click **Save**.

### bq

Use the `--location` flag to set the job's processing location to the
[BigQuery region](https://docs.cloud.google.com/bigquery/docs/locations#omni-loc) that is
collocated with the BigQuery Omni region.
For example, if your BigQuery Omni region is `aws-us-east-1`,
specify `us-east4`.

**Example**

    bq query --use_legacy_sql=false --location=us-east4 \
    "SELECT * FROM region-aws-us-east-1.INFORMATION_SCHEMA.JOBS limit 10;"

    bq query --use_legacy_sql=false --location=asia-northeast3 \
    "SELECT * FROM region-aws-ap-northeast-2.INFORMATION_SCHEMA.JOBS limit 10;"

### API

If you are [running jobs programmatically](https://docs.cloud.google.com/bigquery/docs/running-jobs),
set the location argument to the [BigQuery region](https://docs.cloud.google.com/bigquery/docs/locations#omni-loc)
that is collocated with the BigQuery Omni region.
For example, if your BigQuery Omni region is `aws-us-east-1`,
specify `us-east4`.

The following example lists the metadata refresh jobs in last six hours:

```googlesql
SELECT
 *
FROM
 `region-REGION_NAME`.INFORMATION_SCHEMA.JOBS_BY_PROJECT
WHERE
 job_id LIKE '%metadata_cache_refresh%'
 AND creation_time > TIMESTAMP_SUB(CURRENT_TIMESTAMP(), INTERVAL 6 HOUR)
ORDER BY start_time desc
LIMIT 10;
```

Replace <var translate="no">REGION_NAME</var> with your region.

## VPC Service Controls

You can use VPC Service Controls perimeters to restrict access from
BigQuery Omni to an external cloud service as an extra layer of
defense. For example, VPC Service Controls perimeters can limit exports from
your BigQuery Omni tables to a specific Amazon S3 bucket
or Blob Storage container.

To learn more about VPC Service Controls, see
[Overview of VPC Service Controls](https://docs.cloud.google.com/vpc-service-controls/docs/overview).

### Required permission

Ensure that you have the required permissions to configure service perimeters.
To view a list of IAM roles required to
configure VPC Service Controls, see [Access control with
IAM](https://docs.cloud.google.com/vpc-service-controls/docs/access-control) in the
VPC Service Controls documentation.

### Set up VPC Service Controls using the Google Cloud console

1. In the Google Cloud console navigation menu, click **Security** , and then
   click **VPC Service Controls**.

   [Go to VPC Service Controls](https://console.cloud.google.com/security/service-perimeter)
2. To set up VPC Service Controls for BigQuery Omni,
   follow the steps in the [Create a service
   perimeter](https://docs.cloud.google.com/vpc-service-controls/docs/create-service-perimeters#create_a_service_perimeter)
   guide, and when you are in the **Egress rules** pane, follow these steps:

   1. In the **Egress rules** panel, click **Add rule**.

   2. In the **From attributes of the API client** section, select an option
      from the **Identity** list.

   3. Select **To attributes of external resources**.

   4. To add an external resource, click **Add external resources**.

   5. In the **Add external resource** dialog, for **External resource name**,
      enter a valid resource name. For example:

      - For Amazon Simple Storage Service (Amazon S3): `s3://BUCKET_NAME`

        Replace <var translate="no">BUCKET_NAME</var> with the name of your Amazon S3 bucket.
      - For Azure Blob Storage: `azure://myaccount.blob.core.windows.net/CONTAINER_NAME`

        Replace <var translate="no">CONTAINER NAME</var> with the name of your Blob Storage
        container.

      For a list of egress rule attributes, see [Egress rules reference](https://docs.cloud.google.com/vpc-service-controls/docs/ingress-egress-rules#egress_rules_reference).
   6. Select the methods that you want to allow on your external resources:

      1. If you want to allow all methods, select **All methods** in the **Methods** list.
      2. If you want to allow specific methods, select **Selected method** , click **Select methods**, and then select the methods that you want to allow on your external resources.
   7. Click **Create perimeter**.

### Set up VPC Service Controls using the gcloud CLI

To set up VPC Service Controls using the gcloud CLI, follow these
steps:

1. [Set the default access policy](https://docs.cloud.google.com/bigquery/docs/omni-aws-create-external-table#set-default-policy).
2. [Create the egress policy input file](https://docs.cloud.google.com/bigquery/docs/omni-aws-create-external-table#create-egress-file).
3. [Add the egress policy](https://docs.cloud.google.com/bigquery/docs/omni-aws-create-external-table#add-egress-policy).

#### Set the default access policy

An access policy is an organization-wide container
for access levels and service perimeters. For information about setting a
default access policy or getting an access policy name, see [Managing an access
policy](https://docs.cloud.google.com/access-context-manager/docs/manage-access-policy#set-default).

#### Create the egress policy input file

An egress rule block defines the allowed access from within a perimeter to resources
outside of that perimeter. For external resources, the `externalResources` property
defines the external resource paths allowed access from within your
VPC Service Controls perimeter.

Egress rules can be configured using
a JSON file, or a YAML file. The following sample uses the `.yaml` format:

```yaml
- egressTo:
    operations:
    - serviceName: bigquery.googleapis.com
      methodSelectors:
      - method: "*"
      *OR*
      - permission: "externalResource.read"
    externalResources:
      - EXTERNAL_RESOURCE_PATH
  egressFrom:
    identityType: IDENTITY_TYPE
    *OR*
    identities:
    - serviceAccount:SERVICE_ACCOUNT
```

- `egressTo`: lists allowed service operations on Google Cloud resources
  in specified projects outside the perimeter.

- `operations`: list accessible services and actions or methods that a
  client satisfying the `from` block conditions is allowed to access.

- `serviceName`: set `bigquery.googleapis.com` for BigQuery Omni.

- `methodSelectors`: list methods that a client satisfying the `from` conditions
  can access. For restrictable methods and permissions for services, see
  [Supported service method restrictions](https://docs.cloud.google.com/vpc-service-controls/docs/supported-method-restrictions).

- `method` : a valid service method, or `\"*\"` to allow all `serviceName` methods.

- `permission`: a valid service permission, such as `\"*\"`,
  `externalResource.read`, or `externalResource.write`. Access to resources
  outside the perimeter is allowed for operations that require this permission.

- `externalResources`: lists external resources that clients inside a perimeter
  can access. Replace <var translate="no">EXTERNAL_RESOURCE_PATH</var> with either a valid
  Amazon S3 bucket, such as `s3://bucket_name`, or a
  Blob Storage container path, such as
  `azure://myaccount.blob.core.windows.net/container_name`.

- `egressFrom`: lists allowed service operations on Google Cloud
  resources in specified projects within the perimeter.

- `identityType` or `identities`: defines the identity types that can access the
  specified resources outside the perimeter. Replace <var translate="no">IDENTITY_TYPE</var>
  with one of the following valid values:

  - `ANY_IDENTITY`: to allow all identities.
  - `ANY_USER_ACCOUNT`: to allow all users.
  - `ANY_SERVICE_ACCOUNT`: to allow all service accounts
- `identities`: lists service accounts that can access the specified resources
  outside the perimeter.

- `serviceAccount` (optional): replace <var translate="no">SERVICE_ACCOUNT</var> with the
  service account that can access the specified resources outside the
  perimeter.

#### Examples

The following example is a policy that allows egress operations from inside the
perimeter to the `s3://mybucket` Amazon S3 location in AWS.

```yaml
- egressTo:
    operations:
    - serviceName: bigquery.googleapis.com
      methodSelectors:
      - method: "*"
    externalResources:
      - s3://mybucket
      - s3://mybucket2
  egressFrom:
    identityType: ANY_IDENTITY
```

The following example allows egress operations to a Blob Storage container:

```yaml
- egressTo:
    operations:
    - serviceName: bigquery.googleapis.com
      methodSelectors:
      - method: "*"
    externalResources:
      - azure://myaccount.blob.core.windows.net/mycontainer
  egressFrom:
    identityType: ANY_IDENTITY
```

For more information about egress policies, see the
[Egress rules reference](https://docs.cloud.google.com/vpc-service-controls/docs/ingress-egress-rules#egress-rules-reference).

#### Add the egress policy

To add the egress policy when you create a new service perimeter, use the
[`gcloud access-context-manager perimeters create` command](https://docs.cloud.google.com/sdk/gcloud/reference/access-context-manager/perimeters/create).
For example, the following command creates a new
perimeter named `omniPerimeter` that includes the project with project number
`12345`, restricts the BigQuery API, and adds an egress policy
defined in the `egress.yaml` file:

```bash
gcloud access-context-manager perimeters create omniPerimeter \
    --title="Omni Perimeter" \
    --resources=projects/12345 \
    --restricted-services=bigquery.googleapis.com \
    --egress-policies=egress.yaml
```

To add the egress policy to an existing service perimeter, use the
[`gcloud access-context-manager perimeters update` command](https://docs.cloud.google.com/sdk/gcloud/reference/access-context-manager/perimeters/update).
For example, the following command adds an egress policy defined in the
`egress.yaml` file to an existing service perimeter named `omniPerimeter`:

```bash
gcloud access-context-manager perimeters update omniPerimeter
    --set-egress-policies=egress.yaml
```

### Verify your perimeter

To verify the perimeter, use the
[`gcloud access-context-manager perimeters describe` command](https://docs.cloud.google.com/sdk/gcloud/reference/access-context-manager/perimeters/describe):

```bash
gcloud access-context-manager perimeters describe PERIMETER_NAME
```

Replace <var translate="no">PERIMETER_NAME</var> with the name of the perimeter.

For example, the following command describes the perimeter `omniPerimeter`:

```bash
gcloud access-context-manager perimeters describe omniPerimeter
```

For more information, see
[Managing service perimeters](https://docs.cloud.google.com/vpc-service-controls/docs/manage-service-perimeters#list-and-describe).

## Allow BigQuery Omni VPC access to Amazon S3

As a BigQuery administrator, you can create an S3 bucket policy to
grant BigQuery Omni access to your Amazon S3 resources.
This ensures that only authorized BigQuery Omni VPCs can interact with
your Amazon S3, enhancing the security of your data.

### Apply an S3 bucket policy for BigQuery Omni VPC

To apply an S3 bucket policy, use the AWS CLI or Terraform:

### AWS CLI


Run the following command to apply an S3 bucket policy that includes a
condition using the `aws:SourceVpc` attribute:

```bash
  aws s3api put-bucket-policy \
    --bucket=BUCKET_NAME \
    --policy "{
      \"Version\": \"2012-10-17\",
      \"Id\": \"RestrictBucketReads\",
      \"Statement\": [
          {
              \"Sid\": \"AccessOnlyToOmniVPC\",
              \"Principal\": \"*\",
              \"Action\": [\"s3:ListBucket\", \"s3:GetObject\"],
              \"Effect\": \"Allow\",
              \"Resource\": [\"arn:aws:s3:::BUCKET_NAME\",
                             \"arn:aws:s3:::BUCKET_NAME/*\"],
              \"Condition\": {
                  \"StringEquals\": {
                    \"aws:SourceVpc\": \"VPC_ID\"
                  }
              }
          }
      ]
    }"
```

Replace the following:

- `BUCKET_NAME`: the Amazon S3 bucket that you want BigQuery to access.
- `VPC_ID`: the BigQuery Omni VPC ID of the BigQuery Omni region collocated with the Amazon S3 bucket. You can find this information in the table on this page.

### Terraform


Add the following to your Terraform configuration file:

```terraform
  resource "aws_s3_bucket" "example" {
    bucket = "BUCKET_NAME"
  }

  resource "aws_s3_bucket_policy" "example" {
    bucket = aws_s3_bucket.example.id
    policy = jsonencode({
      Version = "2012-10-17"
      Id      = "RestrictBucketReads"
      Statement = [
          {
              Sid       = "AccessOnlyToOmniVPC"
              Effect    = "Allow"
              Principal = "*"
              Action    = ["s3:GetObject", "s3:ListBucket"]
              Resource  = [
                  aws_s3_bucket.example.arn,
                  "${aws_s3_bucket.example.arn}/*"
                  ]
              Condition = {
                  StringEquals = {
                      "aws:SourceVpc": "VPC_ID"
                  }
              }
          },
      ]
    })
  }
```

Replace the following:

- `BUCKET_NAME`: the Amazon S3 bucket that you want BigQuery to access.
- `VPC_ID`: the BigQuery Omni VPC ID of the BigQuery Omni region collocated with the Amazon S3 bucket.

### BigQuery Omni VPC Resource IDs

| **Region** | **VPC ID** |
|---|---|
| aws-ap-northeast-2 | vpc-0b488548024288af2 |
| aws-ap-southeast-2 | vpc-0726e08afef3667ca |
| aws-eu-central-1 | vpc-05c7bba12ad45558f |
| aws-eu-west-1 | vpc-0e5c646979bbe73a0 |
| aws-us-east-1 | vpc-0bf63a2e71287dace |
| aws-us-west-2 | vpc-0cc24e567b9d2c1cb |

## Limitations

For a full list of limitations that apply to BigLake tables
based on Amazon S3 and Blob Storage, see [Limitations](https://docs.cloud.google.com/bigquery/docs/omni-introduction#limitations).

## What's next

- Learn about [BigQuery Omni](https://docs.cloud.google.com/bigquery/docs/omni-introduction).
- Use the [BigQuery Omni with AWS lab](https://www.cloudskillsboost.google/catalog_lab/5345).
- Learn about [BigLake tables](https://docs.cloud.google.com/bigquery/docs/biglake-intro).
- Learn how to [export query results to Amazon S3](https://docs.cloud.google.com/bigquery/docs/omni-aws-export-results-to-s3).
- Learn how to create [materialized views over Amazon Simple Storage Service (Amazon S3) metadata cache-enabled
  tables](https://docs.cloud.google.com/bigquery/docs/materialized-views-intro#biglake).
- Learn how to make Amazon S3 data in a materialized view available locally for joins by [creating a replica of the materialized view](https://docs.cloud.google.com/bigquery/docs/load-data-using-cross-cloud-transfer#materialized_view_replicas).