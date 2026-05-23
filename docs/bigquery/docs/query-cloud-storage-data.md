# Query Cloud Storage data in external tables

This document describes how to query data stored in a
[Cloud Storage external table](https://docs.cloud.google.com/bigquery/docs/external-data-cloud-storage).

## Before you begin

Ensure that you have a [Cloud Storage external table](https://docs.cloud.google.com/bigquery/docs/external-data-cloud-storage).

### Required roles

To query Cloud Storage external tables, ensure
you have the following roles:

- BigQuery Data Viewer (`roles/bigquery.dataViewer`)
- BigQuery User (`roles/bigquery.user`)
- Storage Object Viewer (`roles/storage.objectViewer`)

Depending on your permissions, you can
grant these roles to yourself or ask your administrator
to grant them to you. For more information about granting roles, see
[Viewing the grantable roles on resources](https://docs.cloud.google.com/iam/docs/viewing-grantable-roles).

To see the exact BigQuery permissions that are required to query
external tables, expand the **Required permissions** section:

#### Required permissions

- `bigquery.jobs.create`
- `bigquery.readsessions.create` (Only required if you are [reading data with the
  BigQuery Storage Read API](https://docs.cloud.google.com/bigquery/docs/reference/storage))
- `bigquery.tables.get`
- `bigquery.tables.getData`

You might also be able to get these permissions with [custom roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles)
or other [predefined roles](https://docs.cloud.google.com/iam/docs/roles-overview#predefined).

## Query permanent external tables

After creating a Cloud Storage external table, you can [query it using
GoogleSQL syntax](https://docs.cloud.google.com/bigquery/docs/running-queries), the same as if
it were a standard BigQuery table. For example, `SELECT field1, field2
FROM mydataset.my_cloud_storage_table;`.

## Query temporary external tables

Querying an external data source using a temporary table is useful
for one-time, ad-hoc queries over external data, or for extract, transform, and load (ETL)
processes.

To query an external data source without creating a permanent table, you provide a table
definition for the temporary table, and then use that table definition in a command or call
to query the temporary table. You can provide the table definition in any of the following
ways:

- A [table definition file](https://docs.cloud.google.com/bigquery/external-table-definition)
- An inline schema definition
- A [JSON schema file](https://docs.cloud.google.com/bigquery/docs/schemas#specifying_a_json_schema_file)

The table definition file or supplied schema is used to create the temporary external table,
and the query runs against the temporary external table.

When you use a temporary external table, you do not create a table in one of your
BigQuery datasets. Because the table is not permanently stored in a dataset, it
cannot be shared with others.

You can create and query a temporary table linked to an external data source
by using the bq command-line tool, the API, or the client libraries.

### bq

You query a temporary table linked to an external data source using the
[`bq query`](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#bq_query) command
with the
[`--external_table_definition` flag](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#bq_query_external_table_definition).
When you use the bq command-line tool to query a temporary table linked to an external
data source, you can identify the table's schema using:

- A [table definition file](https://docs.cloud.google.com/bigquery/docs/external-table-definition) (stored on your local machine)
- An inline schema definition
- A [JSON schema file](https://docs.cloud.google.com/bigquery/docs/schemas#specifying_a_json_schema_file) (stored on your local machine)

(Optional) Supply the `--location` flag and set the value to your
[location](https://docs.cloud.google.com/bigquery/docs/locations).

To query a temporary table linked to your external data source using a table
definition file, enter the following command.

```bash
bq --location=LOCATION query \
--external_table_definition=TABLE::DEFINITION_FILE \
'QUERY'
```

Replace the following:

- `LOCATION`: the name of your [location](https://docs.cloud.google.com/bigquery/docs/locations). The `--location` flag is optional. For example, if you are using BigQuery in the Tokyo region, you can set the flag's value to `asia-northeast1`. You can set a default value for the location using the [.bigqueryrc file](https://docs.cloud.google.com/bigquery/docs/bq-command-line-tool#setting_default_values_for_command-line_flags).
- `TABLE`: the name of the temporary table you're creating.
- `DEFINITION_FILE`: the path to the [table definition file](https://docs.cloud.google.com/bigquery/docs/external-table-definition) on your local machine.
- `QUERY`: the query you're submitting to the temporary table.

For example, the following command creates and queries a temporary table
named `sales` using a table definition file named `sales_def`.

    bq query \
    --external_table_definition=sales::sales_def \
    'SELECT
      Region,
      Total_sales
    FROM
      sales'

To query a temporary table linked to your external data source using an
inline schema definition, enter the following command.

```bash
bq --location=LOCATION query \
--external_table_definition=TABLE::SCHEMA@SOURCE_FORMAT=BUCKET_PATH \
'QUERY'
```

Replace the following:

- `LOCATION`: the name of your [location](https://docs.cloud.google.com/bigquery/docs/locations). The `--location` flag is optional. For example, if you are using BigQuery in the Tokyo region, you can set the flag's value to `asia-northeast1`. You can set a default value for the location using the [.bigqueryrc file](https://docs.cloud.google.com/bigquery/docs/bq-command-line-tool#setting_default_values_for_command-line_flags).
- `TABLE`: the name of the temporary table you're creating.
- `SCHEMA`: the inline schema definition in the format `field:data_type,field:data_type`.
- `SOURCE_FORMAT`: the format of the external data source, for example, `CSV`.
- `BUCKET_PATH`: the path to the
  Cloud Storage bucket that contains the data for the
  table, in the format `gs://bucket_name/[folder_name/]file_pattern`.

  You can select multiple files from the bucket by specifying one asterisk (`*`)
  wildcard character in the `file_pattern`. For example, `gs://mybucket/file00*.parquet`. For more
  information, see
  [Wildcard support for Cloud Storage URIs](https://docs.cloud.google.com/bigquery/docs/external-data-cloud-storage#wildcard-support).

  You can specify multiple buckets for the `uris` option by providing multiple
  paths.

  The following examples show valid `uris` values:
  - `gs://bucket/path1/myfile.csv`
  - `gs://bucket/path1/*.parquet`
  - `gs://bucket/path1/file1*`, `gs://bucket1/path1/*`

  When you specify `uris` values that target multiple files, all of those
  files must share a compatible schema.

  For more information about using Cloud Storage URIs in
  BigQuery, see
  [Cloud Storage resource path](https://docs.cloud.google.com/bigquery/docs/external-data-cloud-storage#google-cloud-storage-uri).
- `QUERY`: the query you're submitting to the temporary table.

For example, the following command creates and queries a temporary table
named `sales` linked to a CSV file stored in Cloud Storage with the
following schema definition:
`Region:STRING,Quarter:STRING,Total_sales:INTEGER`.

    bq query \
    --external_table_definition=sales::Region:STRING,Quarter:STRING,Total_sales:INTEGER@CSV=gs://mybucket/sales.csv \
    'SELECT
      Region,
      Total_sales
    FROM
      sales'

To query a temporary table linked to your external data source using a JSON
schema file, enter the following command.

```bash
bq --location=LOCATION query \
--external_table_definition=SCHEMA_FILE@SOURCE_FORMAT=BUCKET_PATH \
'QUERY'
```

Replace the following:

- `LOCATION`: the name of your [location](https://docs.cloud.google.com/bigquery/docs/locations). The `--location` flag is optional. For example, if you are using BigQuery in the Tokyo region, you can set the flag's value to `asia-northeast1`. You can set a default value for the location using the [.bigqueryrc file](https://docs.cloud.google.com/bigquery/docs/bq-command-line-tool#setting_default_values_for_command-line_flags).
- `SCHEMA_FILE`: the path to the JSON schema file on your local machine.
- `SOURCE_FORMAT`: the format of the external data source, for example, `CSV`.
- `BUCKET_PATH`: the path to the
  Cloud Storage bucket that contains the data for the
  table, in the format `gs://bucket_name/[folder_name/]file_pattern`.

  You can select multiple files from the bucket by specifying one asterisk (`*`)
  wildcard character in the `file_pattern`. For example, `gs://mybucket/file00*.parquet`. For more
  information, see
  [Wildcard support for Cloud Storage URIs](https://docs.cloud.google.com/bigquery/docs/external-data-cloud-storage#wildcard-support).

  You can specify multiple buckets for the `uris` option by providing multiple
  paths.

  The following examples show valid `uris` values:
  - `gs://bucket/path1/myfile.csv`
  - `gs://bucket/path1/*.parquet`
  - `gs://bucket/path1/file1*`, `gs://bucket1/path1/*`

  When you specify `uris` values that target multiple files, all of those
  files must share a compatible schema.

  For more information about using Cloud Storage URIs in
  BigQuery, see
  [Cloud Storage resource path](https://docs.cloud.google.com/bigquery/docs/external-data-cloud-storage#google-cloud-storage-uri).
- `QUERY`: the query you're submitting to the temporary table.

For example, the following command creates and queries a temporary table
named `sales` linked to a CSV file stored in Cloud Storage using the
`/tmp/sales_schema.json` schema file.

```bash
  bq query \
  --external_table_definition=sales::/tmp/sales_schema.json@CSV=gs://mybucket/sales.csv \
  'SELECT
      Region,
      Total_sales
    FROM
      sales'
```

### API

To run a query using the API, follow these steps:

1. Create a [`Job` object](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/Job).
2. Populate the `configuration` section of the `Job` object with a [`JobConfiguration` object](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/Job#JobConfiguration).
3. Populate the `query` section of the `JobConfiguration` object with a [`JobConfigurationQuery` object](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/Job#jobconfigurationquery).
4. Populate the `tableDefinitions` section of the `JobConfigurationQuery` object with an [`ExternalDataConfiguration` object](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables#externaldataconfiguration).
5. Call the [`jobs.insert` method](https://docs.cloud.google.com/bigquery/docs/reference/v2/jobs/insert) to run the query asynchronously or the [`jobs.query` method](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs/query) to run the query synchronously, passing in the `Job` object.

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
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.QueryJobConfiguration.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Schema.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.StandardSQLTypeName.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableResult.html;

    // Sample to queries an external data source using a temporary table
    public class QueryExternalGCSTemp {

      public static void runQueryExternalGCSTemp() {
        // TODO(developer): Replace these variables before running the sample.
        String tableName = "MY_TABLE_NAME";
        String sourceUri = "gs://cloud-samples-data/bigquery/us-states/us-states.csv";
        https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Schema.html schema =
            https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Schema.html.of(
                https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Field.html.of("name", https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.StandardSQLTypeName.html.STRING),
                https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Field.html.of("post_abbr", https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.StandardSQLTypeName.html.STRING));
        String query = String.format("SELECT * FROM %s WHERE name LIKE 'W%%'", tableName);
        queryExternalGCSTemp(tableName, sourceUri, schema, query);
      }

      public static void queryExternalGCSTemp(
          String tableName, String sourceUri, https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Schema.html schema, String query) {
        try {
          // Initialize client that will be used to send requests. This client only needs to be created
          // once, and can be reused for multiple requests.
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html bigquery = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryOptions.html.getDefaultInstance().getService();

          // Skip header row in the file.
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.CsvOptions.html csvOptions = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.CsvOptions.html.newBuilder().setSkipLeadingRows(1).build();

          // Configure the external data source and query job.
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.ExternalTableDefinition.html externalTable =
              https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.ExternalTableDefinition.html.newBuilder(sourceUri, csvOptions).setSchema(schema).build();
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.QueryJobConfiguration.html queryConfig =
              https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.QueryJobConfiguration.html.newBuilder(query)
                  .https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.QueryJobConfiguration.Builder.html#com_google_cloud_bigquery_QueryJobConfiguration_Builder_addTableDefinition_java_lang_String_com_google_cloud_bigquery_ExternalTableDefinition_(tableName, externalTable)
                  .build();

          // Example query to find states starting with 'W'
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableResult.html results = bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html#com_google_cloud_bigquery_BigQuery_query_com_google_cloud_bigquery_QueryJobConfiguration_com_google_cloud_bigquery_BigQuery_JobOption____(queryConfig);

          results
              .https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableResult.html#com_google_cloud_bigquery_TableResult_iterateAll__()
              .forEach(row -> row.forEach(val -> System.out.printf("%s,", val.toString())));

          System.out.println("Query on external temporary table performed successfully.");
        } catch (https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryException.html | InterruptedException e) {
          System.out.println("Query not performed \n" + e.toString());
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

    // Import the Google Cloud client library and create a client
    const {BigQuery} = require('https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/overview.html');
    const bigquery = new https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/bigquery/bigquery.html();

    async function queryExternalGCSTemp() {
      // Queries an external data source using a temporary table.

      const tableId = 'us_states';

      // Configure the external data source
      const externalDataConfig = {
        sourceFormat: 'CSV',
        sourceUris: ['gs://cloud-samples-data/bigquery/us-states/us-states.csv'],
        // Optionally skip header row.
        csvOptions: {skipLeadingRows: 1},
        schema: {fields: schema},
      };

      // Example query to find states starting with 'W'
      const query = `SELECT post_abbr
      FROM \`${tableId}\`
      WHERE name LIKE 'W%'`;

      // For all options, see https://cloud.google.com/bigquery/docs/reference/v2/tables#resource
      const options = {
        query,
        tableDefinitions: {[tableId]: externalDataConfig},
      };

      // Run the query as a job
      const [job] = await bigquery.createQueryJob(options);
      console.log(`Job ${https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/bigquery/bigquery.html.id} started.`);

      // Wait for the query to finish
      const [rows] = await https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/bigquery/bigquery.html.https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/bigquery/job.html();

      // Print the results
      console.log('Rows:');
      console.log(rows);
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

    # Configure the external data source and query job.
    external_config = https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.external_config.ExternalConfig.html("CSV")
    https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.external_config.html.source_uris = [
        "gs://cloud-samples-data/bigquery/us-states/us-states.csv"
    ]
    https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.external_config.html.schema = [
        https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.schema.SchemaField.html("name", "STRING"),
        https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.schema.SchemaField.html("post_abbr", "STRING"),
    ]
    https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.external_config.html.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.external_config.ExternalConfig.html#google_cloud_bigquery_external_config_ExternalConfig_options.skip_leading_rows = 1
    table_id = "us_states"
    job_config = https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.job.QueryJobConfig.html(table_definitions={table_id: external_config})

    # Example query to find states starting with 'W'.
    sql = 'SELECT * FROM `{}` WHERE name LIKE "W%"'.format(table_id)

    query_job = client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html(sql, job_config=job_config)  # Make an API request.

    w_states = list(query_job)  # Wait for the job to complete.
    print("There are {} states with names starting with W.".format(len(w_states)))

## Query the `_FILE_NAME` pseudocolumn


Tables based on external data sources provide a pseudocolumn named `_FILE_NAME`. This
column contains the fully qualified path to the file to which the row belongs. This column is
available only for tables that reference external data stored in
**Cloud Storage** , **Google Drive** ,
**Amazon S3** , and **Azure Blob Storage**.


The `_FILE_NAME` column name is reserved, which means that you cannot create a column
by that name in any of your tables. To select the value of `_FILE_NAME`, you must use
an alias. The following example query demonstrates selecting `_FILE_NAME` by assigning
the alias `fn` to the pseudocolumn.

```bash
  bq query \
  --project_id=PROJECT_ID \
  --use_legacy_sql=false \
  'SELECT
     name,
     _FILE_NAME AS fn
   FROM
     `` `DATASET.TABLE_NAME` ``
   WHERE
     name contains "Alex"' 
```

Replace the following:

- `PROJECT_ID` is a valid project ID (this flag is not required if you use Cloud Shell or if you set a default project in the Google Cloud CLI)
- `DATASET `is the name of the dataset that stores the permanent external table
- `TABLE_NAME` is the name of the permanent external table


When the query has a filter predicate on the `_FILE_NAME` pseudocolumn,
BigQuery attempts to skip reading files that do not satisfy the filter. Similar
recommendations to
[querying ingestion-time partitioned tables using pseudocolumns](https://docs.cloud.google.com/bigquery/docs/querying-partitioned-tables#query_an_ingestion-time_partitioned_table)
apply when constructing query predicates with the `_FILE_NAME` pseudocolumn.

## Optimize external table queries

Consider enabling Rapid Cache when querying Cloud Storage data with
external tables. Rapid Cache provides an SSD-backed zonal read cache for your
Cloud Storage buckets, which can potentially improve query performance
and reduce query costs when querying external tables. For more information, see
[Optimize Cloud Storage external table queries](https://docs.cloud.google.com/bigquery/docs/external-tables#cloud-storage-query-optimization).

## What's next

- Learn about [using SQL in BigQuery](https://docs.cloud.google.com/bigquery/docs/introduction-sql).
- Learn about [external tables](https://docs.cloud.google.com/bigquery/docs/external-tables).
- Learn about [BigQuery quotas](https://docs.cloud.google.com/bigquery/quotas).