# Create object tables

This document describes how to make unstructured data in Cloud Storage accessible
in BigQuery by creating an
[object table](https://docs.cloud.google.com/bigquery/docs/object-table-introduction).

To create an object table, you must complete the following tasks:

1. Create a [dataset](https://docs.cloud.google.com/bigquery/docs/datasets-intro) to contain the object table.
2. Create a [connection](https://docs.cloud.google.com/bigquery/docs/connections-api-intro) to read object information from Cloud Storage.
3. Grant the Storage Object Viewer (`roles/storage.objectViewer`) role to the service account associated with the connection.
4. Create the object table and associate it with the connection by using the [`CREATE EXTERNAL TABLE` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_external_table_statement).

## Before you begin

<br />

## Required roles

To create object tables, you must have the following roles on the project:

- To create datasets and tables, you must have the BigQuery Data Editor (`roles/bigquery.dataEditor`) role.
- To create a connection, you must have the BigQuery Connection Admin (`roles/bigquery.connectionAdmin`) role.
- To grant a role to the connection's service account, you must have the Project IAM Admin (`roles/resourcemanager.projectIamAdmin`).

To [query object tables](https://docs.cloud.google.com/bigquery/docs/object-tables#query-object-tables), you must have the following
roles on the project:

- BigQuery Data Viewer (`roles/bigquery.dataViewer`) role
- BigQuery Connection User (`roles/bigquery.connectionUser`) role

To see the exact permissions that are required, expand the
**Required permissions** section:

<br />

#### Required permissions

- `bigquery.datasets.create`
- `bigquery.tables.create`
- `bigquery.tables.update`
- `bigquery.connections.create`
- `bigquery.connections.get`
- `bigquery.connections.list`
- `bigquery.connections.update`
- `bigquery.connections.use`
- `bigquery.connections.delete`
- `bigquery.connections.delegate`
- `storage.bucket.*`
- `storage.object.*`
- `bigquery.jobs.create`
- `bigquery.tables.get`
- `bigquery.tables.getData`
- `bigquery.readsessions.create`

<br />

You might also be able to get these permissions with [custom roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles)
or other [predefined roles](https://docs.cloud.google.com/iam/docs/roles-overview#predefined).

> [!CAUTION]
> **Caution:** Users that are meant to only query the data should **not** have the following:
>
> - The ability to read objects directly from Cloud Storage, granted by the Storage Object Viewer role.
> - The ability to bind tables to connections, granted by the BigQuery Connection Administrator role.
>
> Otherwise, users can create new object tables that don't have any access
> controls, thus circumventing controls placed by data warehouse administrators.

## Create a dataset

Create a BigQuery dataset to contain the object table:

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)

   If you don't see the left pane, click **Expand left pane** to open the pane.
3. In the **Explorer** pane, click your project name.

4. Click **View actions \> Create dataset**.

5. On the **Create dataset** page, do the following:

   1. For **Dataset ID**, type a name for the dataset.

   2. For **Location type** , select **Region** or **Multi-region**.

      - If you selected **Region** , then select a location from the **Region** list.
      - If you selected **Multi-region** , then select **US** or **Europe** from the **Multi-region** list.
   3. Click **Create dataset**.

## Create a connection

You can skip this step if you either have a default connection configured, or
you have the BigQuery Admin role.

Create a
[Cloud resource connection](https://docs.cloud.google.com/bigquery/docs/create-cloud-resource-connection)
for the object table to use, and get the connection's service account.

1. Go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)
3. In the **Explorer** pane, click **Add data**.

   The **Add data** dialog opens.
4. In the **Filter By** pane, in the **Data Source Type** section, select **Business Applications**.

   Alternatively, in the **Search for data sources** field, you can enter
   `Vertex AI`.
5. In the **Featured data sources** section, click **Vertex AI**.

6. Click the **Vertex AI Models: BigQuery Federation** solution card.

7. In the **Connection type** list, select
   **Vertex AI remote models, remote functions, BigLake and Spanner (Cloud Resource)**.

8. In the **Connection ID** field, type a name for the connection.

9. For **Location type** , select **Region** or **Multi-region**.

   - If you selected **Region** , then select a location from the **Region** list.
   - If you selected **Multi-region** , then select **US** or **Europe** from the **Multi-region** list.
10. Click **Create connection**.

11. Click **Go to connection**.

12. In the **Connection info** pane, copy the service account ID for use in a
    following step.

### Give the service account access

Grant the connection's service account the Storage Object Viewer role:

### Console

1. Go to the **IAM \& Admin** page.

   [Go to IAM \& Admin](https://console.cloud.google.com/project/_/iam-admin)
2. Click **Add**.

   The **Add principals** dialog opens.
3. In the **New principals** field, enter the service account ID that you
   copied earlier.

4. In the **Select a role** field, choose **Cloud Storage** , and then
   select **Storage Object Viewer**.

5. Click **Save**.

### gcloud

Use the
[`gcloud projects add-iam-policy-binding` command](https://docs.cloud.google.com/sdk/gcloud/reference/projects/add-iam-policy-binding).

```
gcloud projects add-iam-policy-binding 'PROJECT_NUMBER' --member='serviceAccount:MEMBER' --role='roles/storage.objectViewer' --condition=None
```

Replace the following:

- `PROJECT_NUMBER`: the project number of the project in which to grant the role.
- `MEMBER`: the service account ID that you copied earlier.

<br />

## Create an object table

To create an object table:

### SQL

Use the
[`CREATE EXTERNAL TABLE` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_external_table_statement).

<br />

<br />

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, enter the following statement:

   ```googlesql
   CREATE EXTERNAL TABLE `PROJECT_ID.DATASET_ID.TABLE_NAME`
   WITH CONNECTION {`PROJECT_ID.REGION.CONNECTION_ID`| DEFAULT}
   OPTIONS(
     object_metadata = 'SIMPLE',
     uris = ['BUCKET_PATH'[,...]],
     max_staleness = STALENESS_INTERVAL,
     metadata_cache_mode = 'CACHE_MODE');
   ```


   Replace the following:
   - `PROJECT_ID`: your project ID.
   - `DATASET_ID`: the ID of the dataset to contain the object table.
   - `TABLE_NAME`: the name of the object table.
   - `REGION`: the [region or multi-region](https://docs.cloud.google.com/bigquery/docs/locations#supported_locations) that contains the connection.
   - `CONNECTION_ID`: the ID of the [cloud resource connection](https://docs.cloud.google.com/bigquery/docs/connections-api-intro) to use with this object table. The connection determines which service account is used to read data from Cloud Storage.

     When you [view the connection details](https://docs.cloud.google.com/bigquery/docs/working-with-connections#view-connections)
     in the Google Cloud console, the connection ID is the value in the last
     section of the fully qualified connection ID that is shown in
     **Connection ID** ---for example
     `projects/myproject/locations/connection_location/connections/*myconnection*`.

     To use a [default connection](https://docs.cloud.google.com/bigquery/docs/default-connections), specify `DEFAULT` instead of the
     connection string containing
     <var translate="no">PROJECT_ID</var>.<var translate="no">REGION</var>.<var translate="no">CONNECTION_ID</var>.
   - `BUCKET_PATH`: the path to the Cloud Storage bucket that contains the objects represented by the object table, in the format `['gs://bucket_name/[folder_name/]*']`.

     You can use one asterisk (`*`) wildcard character
     in each path to limit the objects included in the object table.
     For example, if the bucket contains several types of unstructured data,
     you could create the object table over only PDF objects by specifying
     `['gs://bucket_name/*.pdf']`. For more information, see
     [Wildcard support for Cloud Storage URIs](https://docs.cloud.google.com/bigquery/docs/external-data-cloud-storage#wildcard-support).

     You can specify
     multiple buckets for the `uris` option by providing multiple
     paths, for example
     `['gs://mybucket1/*', 'gs://mybucket2/folder5/*']`.

     For more information about using Cloud Storage URIs in
     BigQuery, see
     [Cloud Storage resource path](https://docs.cloud.google.com/bigquery/docs/external-data-cloud-storage#google-cloud-storage-uri).
   - `STALENESS_INTERVAL`: specifies whether cached metadata is used by operations against the object table, and how fresh the cached metadata must be in order for the operation to use it. For more information on metadata caching considerations, see [Metadata caching for performance](https://docs.cloud.google.com/bigquery/docs/object-table-introduction#metadata_caching_for_performance).

     To disable metadata caching, specify 0. This is the default.

     To enable metadata caching, specify an
     [interval literal](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/lexical#interval_literals)
     value between 30 minutes and 7 days. For example, specify
     `INTERVAL 4 HOUR` for a 4 hour staleness interval.
     With this value, operations against the table use cached metadata if
     it has been refreshed within the past 4 hours. If the cached metadata
     is older than that, the operation retrieves metadata from
     Cloud Storage instead.
   - `CACHE_MODE`: specifies whether the metadata cache is refreshed automatically or manually. For more information on metadata caching considerations, see [Metadata caching for performance](https://docs.cloud.google.com/bigquery/docs/object-table-introduction#metadata_caching_for_performance).

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

The following example creates an object table with a metadata cache
staleness interval of 1 day:

```googlesql
CREATE EXTERNAL TABLE `my_dataset.object_table`
WITH CONNECTION `us.my-connection`
OPTIONS(
  object_metadata = 'SIMPLE',
  uris = ['gs://mybucket/*'],
  max_staleness = INTERVAL 1 DAY,
  metadata_cache_mode = 'AUTOMATIC'
);
```

The following example creates an object table over the objects in three
Cloud Storage buckets:

```googlesql
CREATE EXTERNAL TABLE `my_dataset.object_table`
WITH CONNECTION `us.my-connection`
OPTIONS(
  object_metadata = 'SIMPLE',
  uris = ['gs://bucket1/*','gs://bucket2/folder1/*','gs://bucket3/*']
);
```

The following example creates an object table over just the PDF objects
in a Cloud Storage bucket:

```googlesql
CREATE EXTERNAL TABLE `my_dataset.object_table`
WITH CONNECTION `us.my-connection`
OPTIONS(
  object_metadata = 'SIMPLE',
  uris = ['gs://bucket1/*.pdf']
);
```

### bq

Use the
[`bq mk` command](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#mk-table).

```bash
bq mk --table \
--external_table_definition=BUCKET_PATH@REGION.CONNECTION_ID \
--object_metadata=SIMPLE \
--max_staleness=STALENESS_INTERVAL \
--metadata_cache_mode=CACHE_MODE \
PROJECT_ID:DATASET_ID.TABLE_NAME
```

Replace the following:

- `PROJECT_ID`: your project ID.
- `DATASET_ID`: the ID of the dataset to contain the object table.
- `TABLE_NAME`: the name of the object table.
- `REGION`: the [region or multi-region](https://docs.cloud.google.com/bigquery/docs/locations#supported_locations) that contains the connection.
- `CONNECTION_ID`: the ID of the [cloud resource connection](https://docs.cloud.google.com/bigquery/docs/working-with-connections) to use with this external table. The connection determines which service account is used to read data from Cloud Storage.

  When you [view the connection details](https://docs.cloud.google.com/bigquery/docs/working-with-connections#view-connections)
  in the Google Cloud console, the connection ID is the value in the last section of the
  fully qualified connection ID that is shown in **Connection ID** ---for example
  `projects/myproject/locations/connection_location/connections/*myconnection*`.
- `BUCKET_PATH`: the path to the Cloud Storage bucket that contains the objects represented by the object table, in the format `gs://bucket_name/[folder_name/]*`.

  You can use one asterisk (`*`) wildcard character
  in each path to limit the objects included in the object table.
  For example, if the bucket contains several types of unstructured data,
  you could create the object table over only PDF objects by specifying
  `gs://bucket_name/*.pdf`. For more information, see
  [Wildcard support for Cloud Storage URIs](https://docs.cloud.google.com/bigquery/docs/external-data-cloud-storage#wildcard-support).

  You can specify
  multiple buckets for the `uris` option by providing multiple
  paths, for example
  `gs://mybucket1/*,gs://mybucket2/folder5/*`.

  For more information about using Cloud Storage URIs in
  BigQuery, see
  [Cloud Storage resource path](https://docs.cloud.google.com/bigquery/docs/external-data-cloud-storage#google-cloud-storage-uri).
- `STALENESS_INTERVAL`: specifies whether cached metadata is used by operations against the object table, and how fresh the cached metadata must be in order for the operation to use it. For more information on metadata caching considerations, see [Metadata caching for performance](https://docs.cloud.google.com/bigquery/docs/object-table-introduction#metadata_caching_for_performance).

  To disable metadata caching, specify 0. This is the default.

  To enable metadata caching, specify an interval value between 30
  minutes and 7 days, using the
  `Y-M D H:M:S` format described in the
  [`INTERVAL` data type](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#interval_type)
  documentation. For example, specify `0-0 0 4:0:0` for a 4
  hour staleness interval.
  With this value, operations against the table use cached metadata if
  it has been refreshed within the past 4 hours. If the cached metadata
  is older than that, the operation retrieves metadata from
  Cloud Storage instead.
- `CACHE_MODE`: specifies whether the metadata cache is refreshed automatically or manually. For more information on metadata caching considerations, see [Metadata caching for performance](https://docs.cloud.google.com/bigquery/docs/object-table-introduction#metadata_caching_for_performance).

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

**Examples**

The following example creates an object table with a metadata cache
staleness interval of 1 day:

```bash
bq mk --table \
--external_table_definition=gs://mybucket/*@us.my-connection \
--object_metadata=SIMPLE \
--max_staleness=0-0 1 0:0:0 \
--metadata_cache_mode=AUTOMATIC \
my_dataset.object_table
```

The following example creates an object table over the objects in three
Cloud Storage buckets:

```bash
bq mk --table \
--external_table_definition=gs://bucket1/*,gs://bucket2/folder1/*,gs://bucket3/*@us.my-connection \
--object_metadata=SIMPLE \
my_dataset.object_table
```

The following example creates an object table over just the PDF objects
in a Cloud Storage bucket:

```bash
bq mk --table \
--external_table_definition=gs://bucket1/*.pdf@us.my-connection \
--object_metadata=SIMPLE \
my_dataset.object_table
```

### API

Call the
[`tables.insert` method](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables/insert).
Include an
[`ExternalDataConfiguration` object](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables#ExternalDataConfiguration)
with the `objectMetadata` field set to `SIMPLE` in the
[`Table` resource](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables#resource:-table)
that you pass in.

The following example shows how to call this method by using `curl`:

    ACCESS_TOKEN=$(gcloud auth print-access-token) curl \
    -H "Authorization: Bearer ${ACCESS_TOKEN}" \
    -H "Content-Type: application/json" \
    -X POST \
    -d '{"tableReference": {"projectId": "my_project", "datasetId": "my_dataset", "tableId": "object_table_name"}, "externalDataConfiguration": {"objectMetadata": "SIMPLE", "sourceUris": ["gs://mybucket/*"]}}' \
    https://www.googleapis.com/bigquery/v2/projects/my_project/datasets/my_dataset/tables

### Terraform


This example creates an object table with metadata caching enabled with
manual refresh.

To authenticate to BigQuery, set up Application Default
Credentials. For more information, see
[Set up authentication for client libraries](https://docs.cloud.google.com/bigquery/docs/authentication#client-libs).

The key fields to specify for an object table are
[`google_bigquery_table.external_data_configuration.object_metadata`](https://registry.terraform.io/providers/hashicorp/google/latest/docs/resources/bigquery_table.html#object_metadata),
[`google_bigquery_table.external_data_configuration.metadata_cache_mode`](https://registry.terraform.io/providers/hashicorp/google/latest/docs/resources/bigquery_table.html#metadata_cache_mode),
and [`google_bigquery_table.max_staleness`](https://registry.terraform.io/providers/hashicorp/google/latest/docs/resources/bigquery_table.html#max_staleness). For more information on each resource, see the [Terraform BigQuery documentation](https://registry.terraform.io/providers/hashicorp/google/latest/docs/resources/bigquery_table.html).



    # This queries the provider for project information.
    data "google_project" "default" {}

    # This creates a connection in the US region named "my-connection-id".
    # This connection is used to access the bucket.
    resource "google_bigquery_connection" "default" {
      connection_id = "my-connection-id"
      location      = "US"
      cloud_resource {}
    }

    # This grants the previous connection IAM role access to the bucket.
    resource "google_project_iam_member" "default" {
      role    = "roles/storage.objectViewer"
      project = data.google_project.default.project_id
      member  = "serviceAccount:${google_bigquery_connection.default.cloud_resource[0].service_account_id}"
    }

    # This defines a Google BigQuery dataset.
    resource "google_bigquery_dataset" "default" {
      dataset_id = "my_dataset_id"
    }

    # This creates a bucket in the US region named "my-bucket" with a pseudorandom suffix.
    resource "random_id" "bucket_name_suffix" {
      byte_length = 8
    }
    resource "google_storage_bucket" "default" {
      name                        = "my-bucket-${random_id.bucket_name_suffix.hex}"
      location                    = "US"
      force_destroy               = true
      uniform_bucket_level_access = true
    }

    # This defines a BigQuery object table with manual metadata caching.
    resource "google_bigquery_table" "default" {
      table_id   = "my-table-id"
      dataset_id = google_bigquery_dataset.default.dataset_id
      external_data_configuration {
        connection_id = google_bigquery_connection.default.name
        autodetect    = false
        # `object_metadata is` required for object tables. For more information, see
        # https://registry.terraform.io/providers/hashicorp/google/latest/docs/resources/bigquery_table#object_metadata
        object_metadata = "SIMPLE"
        # This defines the source for the prior object table.
        source_uris = [
          "gs://${google_storage_bucket.default.name}/*",
        ]

        metadata_cache_mode = "MANUAL"
      }

      # This ensures that the connection can access the bucket
      # before Terraform creates a table.
      depends_on = [
        google_project_iam_member.default
      ]
    }

To apply your Terraform configuration in a Google Cloud project, complete the steps in the
following sections.

## Prepare Cloud Shell

1. Launch [Cloud Shell](https://shell.cloud.google.com/).
2. Set the default Google Cloud project
   where you want to apply your Terraform configurations.

   You only need to run this command once per project, and you can run it in any directory.

   ```
   export GOOGLE_CLOUD_PROJECT=PROJECT_ID
   ```

   Environment variables are overridden if you set explicit values in the Terraform
   configuration file.

## Prepare the directory

Each Terraform configuration file must have its own directory (also
called a *root module*).

1. In [Cloud Shell](https://shell.cloud.google.com/), create a directory and a new file within that directory. The filename must have the `.tf` extension---for example `main.tf`. In this tutorial, the file is referred to as `main.tf`.

   ```
   mkdir DIRECTORY && cd DIRECTORY && touch main.tf
   ```
2. If you are following a tutorial, you can copy the sample code in each section or step.

   Copy the sample code into the newly created `main.tf`.

   Optionally, copy the code from GitHub. This is recommended
   when the Terraform snippet is part of an end-to-end solution.
3. Review and modify the sample parameters to apply to your environment.
4. Save your changes.
5. Initialize Terraform. You only need to do this once per directory.

   ```
   terraform init
   ```

   Optionally, to use the latest Google provider version, include the `-upgrade`
   option:

   ```
   terraform init -upgrade
   ```

## Apply the changes

1. Review the configuration and verify that the resources that Terraform is going to create or update match your expectations:

   ```
   terraform plan
   ```

   Make corrections to the configuration as necessary.
2. Apply the Terraform configuration by running the following command and entering `yes` at the prompt:

   ```
   terraform apply
   ```

   Wait until Terraform displays the "Apply complete!" message.
3. [Open your Google Cloud project](https://console.cloud.google.com/) to view the results. In the Google Cloud console, navigate to your resources in the UI to make sure that Terraform has created or updated them.

> [!NOTE]
> **Note:** Terraform samples typically assume that the required APIs are enabled in your Google Cloud project.

<br />

## Query object tables

You can query an object table like any other BigQuery, for
example:

```bash
SELECT *
FROM mydataset.myobjecttable;
```

Querying an object table returns metadata for the underlying objects. For more
information, see
[Object table schema](https://docs.cloud.google.com/bigquery/docs/object-table-introduction#object_table_schema).

## What's next

- Learn how to [run inference on image object tables](https://docs.cloud.google.com/bigquery/docs/object-table-inference).
- Learn how to [analyze object tables by using remote functions](https://docs.cloud.google.com/bigquery/docs/object-table-remote-function).