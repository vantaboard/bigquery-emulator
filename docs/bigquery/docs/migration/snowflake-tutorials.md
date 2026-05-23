# Snowflake to BigQuery migration tutorials

This document provides end-to-end examples and tutorials for the different ways you
can set up a Snowflake to BigQuery migration pipeline.

## Snowflake migration pipeline examples

You can migrate your data from Snowflake
to BigQuery using three different processes: ELT,
ETL, or using partner tools.

### Extract, load, and transform

You can set up an extract, load, and transform (ELT) process with two methods:

- Use a pipeline to extract data from Snowflake and load the data to BigQuery
- Extract data from Snowflake using other Google Cloud products.

#### Use a pipeline to extract data from Snowflake

To [extract data from Snowflake](https://docs.snowflake.com/en/user-guide/data-unload-gcs.html)
and load directly into Cloud Storage, use the [snowflake2bq](https://github.com/GoogleCloudPlatform/professional-services/tree/main/tools/snowflake2bq)
tool.

You can then load your data from Cloud Storage to BigQuery
using one of the following tools:

- [The BigQuery Data Transfer Service for Cloud Storage connector](https://docs.cloud.google.com/bigquery/docs/cloud-storage-transfer-overview)
- The [`LOAD` command](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#bq_load) using the bq command-line tool
- BigQuery API client libraries

#### Other tools to extract data from Snowflake

You can also use the following tools to extract data from Snowflake:

- Dataflow
  - [JDBC to BigQuery template](https://docs.cloud.google.com/dataflow/docs/guides/templates/provided/jdbc-to-bigquery)
  - [SnowflakeIO connector](https://beam.apache.org/documentation/io/built-in/snowflake/)
- [Cloud Data Fusion](https://docs.cloud.google.com/data-fusion/docs)
  - [JDBC drivers](https://docs.cloud.google.com/data-fusion/docs/how-to/using-jdbc-drivers)
- Managed Service for Apache Spark
  - [Apache Spark BigQuery connector](https://docs.cloud.google.com/dataproc/docs/tutorials/bigquery-connector-spark-example)
  - [Snowflake connector for Apache Spark](https://docs.snowflake.com/en/user-guide/spark-connector.html)
  - [Hadoop BigQuery connector](https://docs.cloud.google.com/dataproc/docs/concepts/connectors/bigquery)
  - The JDBC driver from Snowflake and Sqoop to extract data from Snowflake into Cloud Storage:
    - [Moving data with Apache Sqoop in Managed Service for Apache Spark](https://medium.com/google-cloud/moving-data-with-apache-sqoop-in-google-cloud-dataproc-4056b8fa2600)

#### Other tools to load data to BigQuery

You can also use the following tools to load data to BigQuery:

- Dataflow
  - [Read from Cloud Storage](https://beam.apache.org/documentation/programming-guide/#pipeline-io-reading-data)
  - [Write to BigQuery](https://beam.apache.org/documentation/io/built-in/google-bigquery/#writing-to-bigquery)
  - [Cloud Storage Text to BigQuery template](https://docs.cloud.google.com/dataflow/docs/guides/templates/provided/cloud-storage-to-bigquery)
- Cloud Data Fusion
  - [Create a target campaign pipeline](https://docs.cloud.google.com/data-fusion/docs/tutorials/targeting-campaign-pipeline)
- Managed Service for Apache Spark
  - [Cloud Storage connector with Spark](https://docs.cloud.google.com/dataproc/docs/tutorials/gcs-connector-spark-tutorial)
  - [Spark BigQuery connector](https://docs.cloud.google.com/dataproc/docs/tutorials/bigquery-connector-spark-example)
  - [Hadoop Cloud Storage connector](https://docs.cloud.google.com/dataproc/docs/concepts/connectors/cloud-storage)
  - [Hadoop BigQuery connector](https://docs.cloud.google.com/dataproc/docs/concepts/connectors/bigquery)
- [Dataprep by Trifacta](https://docs.trifacta.com/Dataprep/en/product-overview.html)
  - [Read from Cloud Storage](https://docs.trifacta.com/Dataprep/en/platform/connections/connection-types/google-cloud-storage-access.html##)
  - [Write to BigQuery](https://docs.trifacta.com/Dataprep/en/platform/connections/connection-types/bigquery-connections.html##)

### Extract, transform, and load

If you want to transform your data before loading it into
BigQuery, consider the following tools:

- Dataflow
  - Clone the [JDBC to BigQuery
    template](https://docs.cloud.google.com/dataflow/docs/guides/templates/provided/jdbc-to-bigquery) code and modify the template to add [Apache Beam
    transforms](https://beam.apache.org/documentation/programming-guide/#transforms).
- Cloud Data Fusion
  - Create a reusable pipeline and transform your data using [CDAP plugins](https://cdap.io/resources/plugins/).
- Managed Service for Apache Spark
  - Transform your data using [Spark SQL](https://spark.apache.org/docs/latest/sql-programming-guide.html) or custom code in any of the supported Spark languages, such as Scala, Java, Python, or R.

### Partner tools for migration

There are multiple vendors that specialize in the EDW migration space. For a
list of key partners and their provided solutions, see
[BigQuery partners](https://docs.cloud.google.com/bigquery/docs/bigquery-ready-partners).

## Snowflake export tutorial

The following tutorial show a sample data export from Snowflake to
BigQuery that uses the `COPY INTO <location>` Snowflake command.
For a detailed, step-by step process that includes code samples, see
the [Google Cloud professional services Snowflake to BigQuery tool](https://github.com/GoogleCloudPlatform/professional-services/tree/master/tools/snowflake2bq)

### Prepare for the export

You can prepare your Snowflake data for an export by extracting
your Snowflake data into a Cloud Storage or an Amazon Simple Storage Service (Amazon S3)
bucket with the following steps:

### Cloud Storage

This tutorial prepares the file in `PARQUET` format.

1. Use Snowflake SQL statements to create a [named file format specification](https://docs.snowflake.com/en/user-guide/data-unload-prepare.html#creating-a-named-file-format).

   ```bash
   create or replace file format NAMED_FILE_FORMAT
       type = 'PARQUET'
   ```

   Replace <var translate="no">`NAMED_FILE_FORMAT`</var> with a name for the file format. For example, `my_parquet_unload_format`.
2. Create an integration with the [`CREATE STORAGE INTEGRATION`](https://docs.snowflake.com/en/sql-reference/sql/create-storage-integration)
   command.

   ```bash
   create storage integration INTEGRATION_NAME
       type = external_stage
       storage_provider = gcs
       enabled = true
       storage_allowed_locations = ('BUCKET_NAME')
   ```

   Replace the following:
   - <var translate="no">`INTEGRATION_NAME`</var>: a name for the storage integration. For example, `gcs_int`
   - <var translate="no">`BUCKET_NAME`</var>: the path to the Cloud Storage bucket. For example, `gcs://mybucket/extract/`
3. [Retrieve the Cloud Storage service account for
   Snowflake](https://docs.snowflake.com/en/user-guide/data-load-gcs-config.html#step-2-retrieve-the-cloud-storage-service-account-for-your-snowflake-account)
   with the [`DESCRIBE INTEGRATION`](https://docs.snowflake.com/en/sql-reference/sql/desc-integration.html)
   command.

   ```bash
   desc storage integration INTEGRATION_NAME;
   ```

   The output is similar to the following:

   ```bash
   +---+---+---+---+
   | property                    | property_type | property_value                                                              | property_default |
   +---+---+---+---|
   | ENABLED                     | Boolean       | true                                                                        | false            |
   | STORAGE_ALLOWED_LOCATIONS   | List          | gcs://mybucket1/path1/,gcs://mybucket2/path2/                               | []               |
   | STORAGE_BLOCKED_LOCATIONS   | List          | gcs://mybucket1/path1/sensitivedata/,gcs://mybucket2/path2/sensitivedata/   | []               |
   | STORAGE_GCP_SERVICE_ACCOUNT | String        | service-account-id@iam.gserviceaccount.com                 |                  |
   +---+---+---+---+
   ```
4. Grant the service account listed as `STORAGE_GCP_SERVICE_ACCOUNT` read
   and write access to the bucket specified in the storage integration
   command. In this example, grant the `service-account-id@` service account
   read and write access to the `<var>UNLOAD_BUCKET</var>` bucket.

5. Create an external Cloud Storage stage that references the integration that
   you created previously.

   ```bash
   create or replace stage STAGE_NAME
       url='UNLOAD_BUCKET'
       storage_integration = INTEGRATION_NAME
       file_format = NAMED_FILE_FORMAT;
   ```

   Replace the following:
   - <var translate="no">`STAGE_NAME`</var>: a name for the Cloud Storage stage object. For example, `my_ext_unload_stage`

### Amazon S3

The following example shows how to
[move data from a Snowflake table to an Amazon S3 bucket](https://docs.snowflake.com/en/user-guide/data-unload-s3.html):

1. In Snowflake,
   [configure a storage integration object](https://docs.snowflake.com/en/user-guide/data-load-s3-config.html#option-1-configuring-a-snowflake-storage-integration)
   to allow Snowflake to write to an Amazon S3 bucket referenced in an
   external Cloud Storage stage.

   This step involves
   [configuring access permissions](https://docs.snowflake.com/en/user-guide/data-load-s3-config.html#step-1-configure-access-permissions-for-the-s3-bucket)
   to the Amazon S3 bucket,
   [creating the Amazon Web Services (AWS) IAM role](https://docs.snowflake.com/en/user-guide/data-load-s3-config.html#step-2-create-the-iam-role-in-aws),
   and creating a storage integration in Snowflake with the `CREATE STORAGE INTEGRATION` command:

   ```bash
   create storage integration INTEGRATION_NAME
   type = external_stage
   storage_provider = s3
   enabled = true
   storage_aws_role_arn = 'arn:aws:iam::001234567890:role/myrole'
   storage_allowed_locations = ('BUCKET_NAME')
   ```

   Replace the following:
   - <var translate="no">`INTEGRATION_NAME`</var>: a name for the storage integration. For example, `s3_int`
   - <var translate="no">`BUCKET_NAME`</var>: the path to the Amazon S3 bucket to load files to. For example, `s3://unload/files/`
2. [Retrieve the AWS IAM user](https://docs.snowflake.com/en/user-guide/data-load-s3-config-storage-integration.html#step-4-retrieve-the-aws-iam-user-for-your-snowflake-account)
   with the [`DESCRIBE INTEGRATION`](https://docs.snowflake.com/en/sql-reference/sql/desc-integration.html)
   command.

   ```bash
   desc integration INTEGRATION_NAME;
   ```

   The output is similar to the following:

   ```bash
   +---+---+================================================================================+---+
   | property                  | property_type | property_value                                                                 | property_default |
   +---+---+================================================================================+---|
   | ENABLED                   | Boolean       | true                                                                           | false            |
   | STORAGE_ALLOWED_LOCATIONS | List          | s3://mybucket1/mypath1/,s3://mybucket2/mypath2/                                | []               |
   | STORAGE_BLOCKED_LOCATIONS | List          | s3://mybucket1/mypath1/sensitivedata/,s3://mybucket2/mypath2/sensitivedata/    | []               |
   | STORAGE_AWS_IAM_USER_ARN  | String        | arn:aws:iam::123456789001:user/abc1-b-self1234                                 |                  |
   | STORAGE_AWS_ROLE_ARN      | String        | arn:aws:iam::001234567890:role/myrole                                          |                  |
   | STORAGE_AWS_EXTERNAL_ID   | String        | MYACCOUNT_SFCRole=                                                   |                  |
   +---+---+================================================================================+---+
   ```
3. Create a role that has the `CREATE STAGE` privilege for the schema, and the `USAGE` privilege for the storage integration:

   ```bash
       CREATE role ROLE_NAME;  
       GRANT CREATE STAGE ON SCHEMA public TO ROLE ROLE_NAME;
       GRANT USAGE ON INTEGRATION s3_int TO ROLE ROLE_NAME;
   ```

   Replace <var translate="no">`ROLE_NAME`</var> with a name for the role. For example, `myrole`.
4. Grant the AWS IAM user permissions to access the Amazon S3 bucket, and [create an external stage](https://docs.snowflake.com/en/user-guide/data-load-s3-create-stage)
   with the `CREATE STAGE` command:

   ```bash
       USE SCHEMA mydb.public;

       create or replace stage STAGE_NAME
           url='BUCKET_NAME'
           storage_integration = INTEGRATION_NAMEt
           file_format = NAMED_FILE_FORMAT;
   ```

   Replace the following:
   - <var translate="no">`STAGE_NAME`</var>: a name for the Cloud Storage stage object. For example, `my_ext_unload_stage`

### Export Snowflake data

After you have prepared your data, you can move your data to Google Cloud.
Use the `COPY INTO` command to copy data from the Snowflake database table into a Cloud Storage or Amazon S3 bucket by specifying the external stage object, <var translate="no">`STAGE_NAME`</var>.

```bash
    copy into @STAGE_NAME/d1
    from TABLE_NAME;
```

Replace <var translate="no">`TABLE_NAME`</var> with the name of your Snowflake database table.

As a result of this command, the table data is copied to the stage object, which is linked to the Cloud Storage or Amazon S3 bucket. The file includes the `d1` prefix.

### Other export methods

To use Azure Blob Storage for your data exports, follow the steps detailed in
[Unloading into Microsoft Azure](https://docs.snowflake.com/en/user-guide/data-unload-azure.html).
Then, transfer the exported files into Cloud Storage using
[Storage Transfer Service](https://docs.cloud.google.com/storage-transfer/docs/create-manage-transfer-program).