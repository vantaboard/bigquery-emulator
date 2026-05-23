# Default connection overview

To simplify your workflow, you can configure a default
[Cloud resource connection](https://docs.cloud.google.com/bigquery/docs/create-cloud-resource-connection)
in BigQuery for creating external tables and BigQuery ML
remote models. An administrator configures the default connection, and then
users can reference it during resource creation instead of having to specify
connection details.

BigQuery supports default connections in the following resources:

- [Cloud Storage BigLake tables](https://docs.cloud.google.com/bigquery/docs/create-cloud-storage-table-biglake)
- [Object tables](https://docs.cloud.google.com/bigquery/docs/object-tables)
- [Apache Iceberg managed tables](https://docs.cloud.google.com/bigquery/docs/iceberg-tables#create-iceberg-tables)
- [Remote models](https://docs.cloud.google.com/bigquery/docs/bqml-introduction#remote_models)

To use the default connection, specify the `DEFAULT` keyword in the following
SQL clauses:

- The `WITH CONNECTION` clause of a [`CREATE EXTERNAL TABLE` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_external_table_statement)
- The `REMOTE WITH CONNECTION` clause of a [`CREATE MODEL` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model) for a remote model

## Before you begin


Enable the BigQuery Connection API.


**Roles required to enable APIs**


To enable APIs, you need the Service Usage Admin IAM
role (`roles/serviceusage.serviceUsageAdmin`), which
contains the `serviceusage.services.enable` permission. [Learn how to grant
roles](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).

[Enable the API](https://console.cloud.google.com/flows/enableapi?apiid=bigqueryconnection.googleapis.com)

### Required roles and permissions

To work with default connections, use the following Identity and Access Management (IAM)
roles:

- Use the default connection: BigQuery Connection User (`roles/bigquery.connectionUser`) on your project
- Set the default connection: BigQuery Admin (`roles/bigquery.admin`) on your project
- If it is necessary to grant permissions to the service account of a default
  connection:

  - If the default connection is used to create external tables: Storage Admin (`roles/storage.admin`) on any Cloud Storage buckets used by the external tables.
  - If the default connection is used to create remote models: Project IAM
    Admin (`roles/resourcemanager.projectIamAdmin`) on the project that contains
    the Vertex AI endpoint. For the following types of remote models,
    this is the current project:

    - Remote models over Cloud AI services.
    - Remote models over Google or partner models that you created by specifying the model name as an endpoint.

    For all other remote models, this is the project that contains the
    Vertex AI endpoint to which the target model is deployed.

    If you use the remote model to analyze unstructured data from an object
    table, and the Cloud Storage bucket that you use in the object
    table is in a different project than your Vertex AI
    endpoint, you must also have Storage Admin (`roles/storage.admin`) on the
    Cloud Storage bucket used by the object table.

  You only need these roles if you are an administrator configuring a
  connection for use as the default connection, or a user who is using a
  default connection that has not yet had the appropriate role granted to its
  service account. For more information, see [Configure the default
  connection](https://docs.cloud.google.com/bigquery/docs/default-connections#configure_the_default_connection).

These predefined roles contain the permissions required to perform the tasks in
this document. To see the exact permissions that are required, expand the
**Required permissions** section:

#### Required permissions

- Use the default connection: `bigquery.connections.use`
- Create a connection: `bigquery.connections.*`
- Set the default connection: `bigquery.config.*`
- Set service account permissions for a default connection that is used to create external tables: `storage.buckets.getIamPolicy` and `storage.buckets.setIamPolicy`
- Set service account permissions for a default connection that is used to create remote models:
  - `resourcemanager.projects.getIamPolicy` and `resourcemanager.projects.setIamPolicy`
  - If the default connection is used with a remote model that processes unstructured data from an object table, `storage.buckets.getIamPolicy` and `storage.buckets.setIamPolicy`

You might also be able to get these permissions with [custom roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles)
or other [predefined roles](https://docs.cloud.google.com/iam/docs/roles-overview#predefined).

## Configure the default connection

To configure the default connection for the first time, use one of the
following methods:

- Create a connection, grant appropriate roles to the connection's service
  account, and then set the connection as the default connection.

  The user creating and configuring the default connection needs the BigQuery
  Admin role and the Storage Admin or Project IAM Admin role, as
  appropriate. The default connection user needs the BigQuery Connection User
  role.
- Create a connection and then set it as the default connection. The service
  [grants appropriate roles](https://docs.cloud.google.com/bigquery/docs/default-connections#permissions-provisioning) to the default
  connection's service account when the default connection is used.

  The user creating and setting the default connection needs the BigQuery Admin
  role. The default connection user needs the BigQuery Connection User role
  and the Storage Admin or Project IAM Admin role, as appropriate.
- Specify the `DEFAULT` keyword in a supported statement. The service
  creates a connection, grants appropriate roles to the connection's service
  account, and then sets the connection as the default connection.

  The default connection user needs the BigQuery Admin role and the Storage
  Admin or Project IAM Admin role, as appropriate.
- When the default connection is missing, BigQuery creates a
  new connection with the following properties:

  - **Region:** The same region as the dataset.
  - **Name:** `__default_cloudresource_connection__`
  - **Type:** `CLOUD_RESOURCE`

> [!IMPORTANT]
> **Important:** Use of a default connection can extend additional privileges to users. For example, if an administrator uses the default connection to create an object table, the default connection's service account is granted the Storage Legacy Bucket Reader and Storage Legacy Object Reader roles on the appropriate Cloud Storage bucket. Any user that has been granted access to use the connection can then also access that Cloud Storage bucket with the permissions granted to these roles.

## Set the default connection for a project

Set the default Cloud resource connection for the project by using the
[`ALTER PROJECT SET OPTIONS` DDL statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#alter_project_set_options_statement).

The following example sets the default connection for the project:

<br />

```googlesql
  ALTER PROJECT PROJECT_ID
  SET OPTIONS (
    `region-REGION.default_cloud_resource_connection_id` = CONNECTION_ID);
  
```

<br />

Replace the following:

- `PROJECT_ID`: the ID of the project where you're setting the default connection.
- `REGION`: the region of the connection.
- `CONNECTION_ID`: the ID or name of the connection to use as the default for tables and models. Only specify the connection ID or name, and exclude the project ID and region prefixes attached to the name or ID.

For more information about configuring a default connection for a project, see
[Manage default configurations](https://docs.cloud.google.com/bigquery/docs/default-configuration).

## Permissions provisioning for the default connection

When you use the default connection to create an external table or remote model,
Google Cloud grants the default connection's service account the
appropriate roles if the service account doesn't already have them. This action
fails if you don't have administrative privileges on the Cloud Storage or
Vertex AI resource used by the external table or remote model.

The following roles are granted to the default connection's service account:

| Type of table or model | Remote resource | Roles assigned to the connection's service account |
|---|---|---|
| [Cloud Storage BigLake table](https://docs.cloud.google.com/bigquery/docs/create-cloud-storage-table-biglake) | Cloud Storage | `roles/storage.legacyBucketReader` `roles/storage.legacyObjectReader` |
| [Object Table](https://docs.cloud.google.com/bigquery/docs/object-tables) | Cloud Storage | `roles/storage.legacyBucketReader` `roles/storage.legacyObjectReader` |
| [Iceberg managed tables](https://docs.cloud.google.com/bigquery/docs/iceberg-tables#create-iceberg-tables) | Cloud Storage | `roles/storage.legacyBucketWriter` `roles/storage.legacyObjectOwner` |
| [BigQuery ML remote models over Vertex AI models](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model-https) | Google owned models | `roles/aiplatform.user` |
| [BigQuery ML remote models over Vertex AI models](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model-https) | Deployable to an endpoint from Model Garden | `roles/aiplatform.user` |
| [BigQuery ML remote models over Vertex AI models](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model-https) | User models | `roles/aiplatform.user` |
| [BigQuery ML remote models over Vertex AI models](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model-https) | Fine tuned models | `roles/aiplatform.serviceAgent` |
| [BigQuery ML remote models over Cloud AI services](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model-service) | Document processor | `roles/documentai.apiUser` |
| [BigQuery ML remote models over Cloud AI services](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model-service) | Speech recognizer | `roles/speech.serviceAgent` |
| [BigQuery ML remote models over Cloud AI services](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model-service) | Cloud NLP | `roles/serviceusage.serviceUsageConsumer` |
| [BigQuery ML remote models over Cloud AI services](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model-service) | Cloud Vision | `roles/serviceusage.serviceUsageConsumer` |
| [BigQuery ML remote models over Cloud AI services](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model-service) | Cloud Translation | `roles/cloudtranslate.user` |

## Create external tables using `CONNECTION DEFAULT`

The following examples show how to create external tables by specifying
`WITH CONNECTION DEFAULT` in BigQuery.

### Example: Create a Cloud Storage BigLake table

The following SQL expression creates a
[Cloud Storage BigLake table](https://docs.cloud.google.com/bigquery/docs/create-cloud-storage-table-biglake)
with a default connection:

    CREATE EXTERNAL TABLE PROJECT_ID.DATASET.EXTERNAL_TABLE_NAME
    WITH CONNECTION DEFAULT
    OPTIONS (
      format = 'TABLE_FORMAT',
      uris = ['BUCKET_PATH']);

### Example: Create an object table with a default connection

The following SQL expression creates an
[object table](https://docs.cloud.google.com/bigquery/docs/object-tables) with a default
connection:

    CREATE EXTERNAL TABLE PROJECT_ID.DATASET.EXTERNAL_TABLE_NAME
    WITH CONNECTION DEFAULT
    OPTIONS (
      object_metadata = 'SIMPLE'
      uris = ['BUCKET_PATH']);

### Example: Create a Iceberg managed tables with a default connection

The following SQL expression creates a
[Iceberg managed tables](https://docs.cloud.google.com/bigquery/docs/iceberg-tables#create-iceberg-tables)
with a default connection:

    CREATE TABLE `myproject.tpch_clustered.nation` (
      n_nationkey integer,
      n_name string,
      n_regionkey integer,
      n_comment string)
    CLUSTER BY n_nationkey
    WITH CONNECTION DEFAULT
    OPTIONS (
      file_format = 'PARQUET',
      table_format = 'ICEBERG',
      storage_uri = 'gs://mybucket/warehouse/nation');

## Create remote models using `REMOTE WITH CONNECTION DEFAULT`

The following examples show how to create remote models by specifying
`REMOTE WITH CONNECTION DEFAULT` in BigQuery.

### Example: Create a remote model over a Vertex AI model

The following SQL expression creates a [remote model](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model)
with a default connection:

    CREATE OR REPLACE MODEL `mydataset.flash_model`
      REMOTE WITH CONNECTION DEFAULT
      OPTIONS(ENDPOINT = 'gemini-2.0-flash');

### Example: Create a remote model over a Cloud AI service

The following SQL expression creates a [remote model over a Cloud AI
service](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model-service)
with a default connection:

    CREATE MODEL `project_id.mydataset.mymodel`
    REMOTE WITH CONNECTION DEFAULT
     OPTIONS(REMOTE_SERVICE_TYPE = 'CLOUD_AI_VISION_V1')

### Example: Create a remote model with an HTTPS endpoint

The following SQL expression creates a [remote model with an HTTPS endpoint](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model-https)
and a default connection:

    CREATE MODEL `project_id.mydataset.mymodel`
     INPUT(f1 INT64, f2 FLOAT64, f3 STRING, f4 ARRAY)
     OUTPUT(out1 INT64, out2 INT64)
     REMOTE WITH CONNECTION DEFAULT
     OPTIONS(ENDPOINT = 'https://us-central1-aiplatform.googleapis.com/v1/projects/myproject/locations/us-central1/endpoints/1234')

## What's next

- Learn about [default configuration](https://docs.cloud.google.com/bigquery/docs/default-configuration) in BigQuery.