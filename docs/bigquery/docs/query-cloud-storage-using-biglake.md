# Query Cloud Storage data in BigLake tables

This document describes how to query data stored in a
[Cloud Storage BigLake table](https://docs.cloud.google.com/bigquery/docs/create-cloud-storage-table-biglake).

## Before you begin

Ensure that you have a [Cloud Storage BigLake table](https://docs.cloud.google.com/bigquery/docs/create-cloud-storage-table-biglake).

### Required roles

To query Cloud Storage BigLake tables, ensure
you have the following roles:

- BigQuery Data Viewer (`roles/bigquery.dataViewer`)
- BigQuery User (`roles/bigquery.user`)

Depending on your permissions, you can
grant these roles to yourself or ask your administrator
to grant them to you. For more information about granting roles, see
[Viewing the grantable roles on resources](https://docs.cloud.google.com/iam/docs/viewing-grantable-roles).

To see the exact permissions that are required to query
Cloud Storage BigLake tables, expand the
**Required permissions** section:

#### Required permissions

- `bigquery.jobs.create`
- `bigquery.readsessions.create` (Only required if you are [reading data with the
  BigQuery Storage Read API](https://docs.cloud.google.com/bigquery/docs/reference/storage))
- `bigquery.tables.get`
- `bigquery.tables.getData`

You might also be able to get these permissions with [custom roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles)
or other [predefined roles](https://docs.cloud.google.com/iam/docs/roles-overview#predefined)

## Query BigLake tables

After creating a Cloud Storage BigLake table, you can
[query it using GoogleSQL syntax](https://docs.cloud.google.com/bigquery/docs/running-queries),
the same as if it were a standard BigQuery table. For example,
`SELECT field1, field2 FROM mydataset.my_cloud_storage_table;`.

## Query BigLake tables using external data processing tools

You can use BigQuery connectors with other data processing tools to access BigLake tables on Cloud Storage. For more information, see [Connectors](https://docs.cloud.google.com/bigquery/docs/biglake-intro#connectors).

### Apache Spark

The following example uses
[Managed Service for Apache Spark](https://docs.cloud.google.com/dataproc), but it also works with any Spark deployment that
uses the
[Spark-BigQuery connector](https://docs.cloud.google.com/dataproc/docs/tutorials/bigquery-connector-spark-example).

In this example, you supply the Spark-BigQuery connector as an initialization
action when you [create a cluster](https://docs.cloud.google.com/dataproc/docs/guides/create-cluster).
This action lets you use a Zeppelin notebook and exercise the data analyst user
journey.

Spark-BigQuery connector versions are listed in the GitHub
[GoogleCloudDataproc/spark-bigquery-connector repository](https://github.com/GoogleCloudDataproc/spark-bigquery-connector/releases).

Create a single node cluster using the initialization action for the
Spark-BigQuery connector:

```bash
gcloud dataproc clusters create biglake-demo-cluster \
    --optional-components=ZEPPELIN \
    --region=REGION \
    --enable-component-gateway \
    --single-node \
    --initialization-actions gs://goog-dataproc-initialization-actions-REGION/connectors/connectors.sh \
    --metadata spark-bigquery-connector-url= gs://spark-lib/bigquery/spark-bigquery-with-dependencies_SCALA_VERSION-CONNECTOR_VERSION.jar
```

### Apache Hive

The following example uses
[Managed Service for Apache Spark](https://docs.cloud.google.com/dataproc), but it also works with any Hive deployment that
uses the
[Hive-BigQuery connector](https://github.com/GoogleCloudDataproc/hive-bigquery-connector).

In this example, you supply the Hive-BigQuery connector as an initialization
action when you [create a cluster](https://docs.cloud.google.com/dataproc/docs/guides/create-cluster).

Hive-BigQuery connector versions are listed in the GitHub
[GoogleCloudDataproc/hive-bigquery-connector repository](https://github.com/GoogleCloudDataproc/hive-bigquery-connector/releases).

Create a single node cluster using the initialization action for the
Hive-BigQuery connector:

```bash
gcloud dataproc clusters create biglake-hive-demo-cluster \
    --region=REGION \
    --single-node \
    --initialization-actions gs://goog-dataproc-initialization-actions-REGION/connectors/connectors.sh \
    --metadata hive-bigquery-connector-url=gs://goog-dataproc-artifacts-REGION/hive-bigquery/hive-bigquery-connector-CONNECTOR_VERSION.jar
```

For more information about the Hive-BigQuery connector, see
[Use the Hive-BigQuery Connector](https://docs.cloud.google.com/dataproc/docs/concepts/connectors/hive-bigquery).

### Dataflow

To read BigLake tables from [Dataflow](https://docs.cloud.google.com/dataflow), use the Dataflow
connector in `DIRECT_READ` mode to use the BigQuery Storage API. Reading from a query string
is also supported. See [BigQuery I/O](https://beam.apache.org/documentation/io/built-in/google-bigquery/)
in the Apache Beam documentation.

> [!NOTE]
> **Note:** The default `EXPORT` mode for Dataflow is not supported.

## Query temporary BigLake tables

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

Use the
[`bq query`](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#bq_query) command
with the
[`--external_table_definition` flag](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#bq_query_external_table_definition).

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
    --external_table_definition=sales::sales_def@us.myconnection \
    'SELECT
      Region,
      Total_sales
    FROM
      sales'

To query a temporary table linked to your external data source using an
inline schema definition, enter the following command.

```bash
bq --location=LOCATION query \
--external_table_definition=TABLE::SCHEMA@SOURCE_FORMAT=BUCKET_PATH@projects/PROJECT_ID/locations/REGION/connections/CONNECTION_ID \
'query'
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
- `PROJECT_ID`: the project that contains the
  connection.

- `REGION`: the region that contains the
  connection---for example, `us`.

- `CONNECTION_ID`: the name of the connection---for
  example, `myconnection`.

- `QUERY`: the query you're submitting to the temporary table.

For example, the following command creates and queries a temporary table
named `sales` linked to a CSV file stored in Cloud Storage with the
following schema definition:
`Region:STRING,Quarter:STRING,Total_sales:INTEGER`.

    bq query \
    --external_table_definition=sales::Region:STRING,Quarter:STRING,Total_sales:INTEGER@CSV=gs://mybucket/sales.csv@us.myconnection \
    'SELECT
      Region,
      Total_sales
    FROM
      sales'

To query a temporary table linked to your external data source using a JSON
schema file, enter the following command.

```bash
bq --location=LOCATION query \
--external_table_definition=SCHEMA_FILE@SOURCE_FORMAT=BUCKET_PATH@projects/PROJECT_ID/locations/REGION/connections/CONNECTION_ID \
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
- `PROJECT_ID`: the project that contains the
  connection.

- `REGION`: the region that contains the
  connection---for example, `us`.

- `CONNECTION_ID`: the name of the connection---for
  example, `myconnection`.

- `QUERY`: the query you're submitting to the temporary table.

For example, the following command creates and queries a temporary table
named `sales` linked to a CSV file stored in Cloud Storage using the
`/tmp/sales_schema.json` schema file.

```bash
  bq query \
  --external_table_definition=sales::/tmp/sales_schema.json@CSV=gs://mybucket/sales.csv@us.myconnection \
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
4. Populate the `tableDefinitions` section of the `JobConfigurationQuery` object with an [`ExternalDataConfiguration` object](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables#externaldataconfiguration). Specify the connection to use for connecting to Cloud Storage in the `connectionId` field.
5. Call the [`jobs.insert` method](https://docs.cloud.google.com/bigquery/docs/reference/v2/jobs/insert) to run the query asynchronously or the [`jobs.query` method](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs/query) to run the query synchronously, passing in the `Job` object.

## What's next

- Learn about [using SQL in BigQuery](https://docs.cloud.google.com/bigquery/docs/introduction-sql).
- Learn about [BigLake tables](https://docs.cloud.google.com/bigquery/docs/biglake-intro).
- Learn about [BigQuery quotas](https://docs.cloud.google.com/bigquery/quotas).