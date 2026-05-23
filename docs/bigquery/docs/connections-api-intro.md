# Introduction to connections

BigQuery lets you create external connections to query data that's
stored outside of BigQuery in Google Cloud services like
Cloud Storage or Spanner, or in third-party sources like
Amazon Web Services (AWS) or Microsoft Azure. These external connections use the
BigQuery Connection API.

For example, suppose that you store details about customer orders in Cloud SQL
and data about sales in BigQuery, and you want to join the two
tables in a single query. You can create a Cloud SQL connection to the
external database by using the BigQuery Connection API. With connections, you never
send database credentials as
[cleartext](https://simple.wikipedia.org/wiki/Cleartext).

A connection is encrypted and securely stored in the BigQuery
connection service. You can [give users access to connections](https://docs.cloud.google.com/bigquery/docs/working-with-connections#share-connections)
by granting them BigQuery connection Identity and Access Management (IAM)
roles.

## Connection types

BigQuery provides different connection types for the following external data sources:

- Amazon Simple Storage Service (Amazon S3)
- Apache Spark
- Azure Blob Storage
- Google Cloud resources such as Vertex AI remote models, remote functions, and BigLake
- Spanner
- Cloud SQL
- AlloyDB for PostgreSQL
- SAP Datasphere

> [!NOTE]
> **Note:** You don't need a connection to query data in [Bigtable](https://docs.cloud.google.com/bigquery/docs/external-data-bigtable) and [Google Drive](https://docs.cloud.google.com/bigquery/docs/external-data-drive).

### Amazon S3 connections

To create an Amazon S3 connection with BigQuery Omni, see
[Connect to Amazon S3](https://docs.cloud.google.com/bigquery/docs/omni-aws-create-connection).

Once you have an existing Amazon S3 connection, you can do the following:

- [Create external tables on Amazon S3](https://docs.cloud.google.com/bigquery/docs/omni-aws-create-external-table)
- [Query the Amazon S3 data](https://docs.cloud.google.com/bigquery/docs/query-aws-data)
- [Export results to Amazon S3](https://docs.cloud.google.com/bigquery/docs/omni-aws-export-results-to-s3)
- [Create datasets based on AWS Glue databases](https://docs.cloud.google.com/bigquery/docs/glue-federated-datasets).

### Spark connections

[Stored procedures for Spark](https://docs.cloud.google.com/bigquery/docs/spark-procedures) let
you run stored procedures written in Python using BigQuery. A
[Spark connection](https://docs.cloud.google.com/bigquery/docs/connect-to-spark) lets you connect to
Managed Service for Apache Spark and run the stored procedures for Spark.

To create this connection, see [Create connections](https://docs.cloud.google.com/bigquery/docs/connect-to-spark#create-spark-connection).

### Blob Storage connections

To create a Blob Storage connection with BigQuery Omni, see
[Connect to Blob Storage](https://docs.cloud.google.com/bigquery/docs/omni-azure-create-connection).

Once you have an existing Blob Storage connection, you can do the following:

- [Create external tables based on Blob Storage](https://docs.cloud.google.com/bigquery/docs/omni-azure-create-external-table)
- [Query the Blob Storage data](https://docs.cloud.google.com/bigquery/docs/query-azure-data)
- [Export results to Blob Storage](https://docs.cloud.google.com/bigquery/docs/omni-azure-export-results-to-azure-storage)

### Google Cloud resource connections

A Google Cloud resource connection is a connection to authorize access to other Google Cloud
resources such as Vertex AI remote models, remote functions, and
BigLake. For details on how to set up a Google Cloud resource connection, see
[Create and set up a Cloud resource connection](https://docs.cloud.google.com/bigquery/docs/create-cloud-resource-connection).

Once you have an existing Google Cloud resource connection, you can create the following
BigQuery objects with it:

- **Remote models** . For more information, see [The CREATE MODEL statement for remote models over LLMs](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model), [The CREATE MODEL statement for remote models over Cloud AI services](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model-service), and [The CREATE MODEL statement for remote models over Vertex AI hosted models](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model-https).
- **Remote functions** . BigQuery [remote functions](https://docs.cloud.google.com/bigquery/docs/remote-functions) let you implement functions with any supported languages in Cloud Run functions or Cloud Run. A remote function connection lets you connect with Cloud Run functions or Cloud Run and run these functions. To create a BigQuery remote function connection, see [Create a connection](https://docs.cloud.google.com/bigquery/docs/remote-functions#create_a_connection).
- **BigLake tables** . BigLake connections connect [BigLake tables](https://docs.cloud.google.com/bigquery/docs/biglake-intro) to external data sources while retaining fine-grained BigQuery access control and security for both structured and unstructured data in Cloud Storage.
- **Object tables** . For more information, see [Introduction to object tables](https://docs.cloud.google.com/bigquery/docs/object-table-introduction).

### Spanner connections

To create a Spanner connection, see [Connect to Spanner](https://docs.cloud.google.com/bigquery/docs/connect-to-spanner).

Once you have an existing Spanner connection, you can run
[federated queries](https://docs.cloud.google.com/bigquery/docs/federated-queries-intro).

### Cloud SQL connections

To create a Cloud SQL connection, see [Connect to Cloud SQL](https://docs.cloud.google.com/bigquery/docs/connect-to-sql).

Once you have an existing Cloud SQL connection, you can run
[federated queries](https://docs.cloud.google.com/bigquery/docs/federated-queries-intro).

### AlloyDB connections

To create an AlloyDB connection, see [Connect to AlloyDB for PostgreSQL](https://docs.cloud.google.com/bigquery/docs/connect-to-alloydb).

Once you have an existing AlloyDB connection, you can run
[federated queries](https://docs.cloud.google.com/bigquery/docs/federated-queries-intro).

### SAP Datasphere connections

> [!WARNING]
>
> **Preview**
>
>
> This product or feature is
>
> subject to the "Pre-GA Offerings Terms" in the General Service Terms section
> of the [Service Specific
> Terms](https://docs.cloud.google.com/terms/service-terms#1).
>
> Pre-GA products and features are available "as is" and might have limited support.
>
> For more information, see the
> [launch stage descriptions](https://cloud.google.com/products/#product-launch-stages).

To create an SAP Datasphere connection, see [Connect to SAP Datasphere](https://docs.cloud.google.com/bigquery/docs/connect-to-sap-datasphere).

Once you have an existing SAP Datasphere connection, you can run
[federated queries](https://docs.cloud.google.com/bigquery/docs/federated-queries-intro).

## Audit logs

BigQuery logs usage and management requests about connections.
For more information, see [BigQuery audit logs overview](https://docs.cloud.google.com/bigquery/docs/reference/auditlogs).

## What's next

- Learn how to [manage connections](https://docs.cloud.google.com/bigquery/docs/working-with-connections).
- Learn more about [default connections](https://docs.cloud.google.com/bigquery/docs/default-connections) for your project.
- Learn how to [analyze object tables by using remote functions](https://docs.cloud.google.com/bigquery/docs/object-table-remote-function).
- Learn how to query stored data:
  - [Query data stored in Amazon S3](https://docs.cloud.google.com/bigquery/docs/omni-aws-create-external-table).
  - [Query data stored in Blob Storage](https://docs.cloud.google.com/bigquery/docs/omni-azure-create-external-table).
  - [Query structured data stored in Cloud Storage](https://docs.cloud.google.com/bigquery/docs/query-cloud-storage-using-biglake#query-biglake-table-bigquery).
  - [Query unstructured data stored in Cloud Storage](https://docs.cloud.google.com/bigquery/docs/object-tables).
  - [Query data stored in Spanner](https://docs.cloud.google.com/bigquery/docs/spanner-federated-queries).
  - [Query data stored in Cloud SQL](https://docs.cloud.google.com/bigquery/docs/cloud-sql-federated-queries).
  - [Query data stored in AlloyDB](https://docs.cloud.google.com/bigquery/docs/alloydb-federated-queries).
  - [Query data using remote functions](https://docs.cloud.google.com/bigquery/docs/remote-functions#create_a_remote_function).
  - [Query unstructured data using remote functions](https://docs.cloud.google.com/bigquery/docs/object-table-remote-function).
  - [Query data using stored procedures for Apache Spark](https://docs.cloud.google.com/bigquery/docs/spark-procedures).
- Learn about [external tables](https://docs.cloud.google.com/bigquery/docs/external-tables).