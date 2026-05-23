# External tables for Cloud Storage

This document describes how to create a Cloud Storage BigLake table.
A [BigLake table](https://docs.cloud.google.com/bigquery/docs/biglake-intro) lets you use
access delegation to query structured data in Cloud Storage. Access
delegation decouples access to the BigLake table from access
to the underlying datastore.

## Before you begin

1. In the Google Cloud console, on the project selector page,
   select or create a Google Cloud project.

   **Roles required to select or create a project**
   - **Select a project**: Selecting a project doesn't require a specific IAM role---you can select any project that you've been granted a role on.
   - **Create a project** : To create a project, you need the Project Creator role (`roles/resourcemanager.projectCreator`), which contains the `resourcemanager.projects.create` permission. [Learn how to grant
     roles](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).

   > [!NOTE]
   > **Note**: If you don't plan to keep the resources that you create in this procedure, create a project instead of selecting an existing project. After you finish these steps, you can delete the project, removing all resources associated with the project.

   [Go to project selector](https://console.cloud.google.com/projectselector2/home/dashboard)
2.
   [Verify that billing is enabled for your Google Cloud project](https://docs.cloud.google.com/billing/docs/how-to/verify-billing-enabled#confirm_billing_is_enabled_on_a_project).

3.


   Enable the BigQuery Connection API.


   **Roles required to enable APIs**


   To enable APIs, you need the Service Usage Admin IAM
   role (`roles/serviceusage.serviceUsageAdmin`), which
   contains the `serviceusage.services.enable` permission. [Learn how to grant
   roles](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).

   [Enable the API](https://console.cloud.google.com/flows/enableapi?apiid=bigqueryconnection.googleapis.com)

   If you want to read BigLake tables from open source engines
   such as Apache Spark, then you need to enable the
   [BigQuery Storage Read API](https://console.cloud.google.com/apis/library/bigquerystorage.googleapis.com).
4. In the Google Cloud console, activate Cloud Shell.

   [Activate Cloud Shell](https://console.cloud.google.com/?cloudshell=true)
5. Ensure that you have a BigQuery [dataset](https://docs.cloud.google.com/bigquery/docs/datasets).

6. Ensure that your version of the Google Cloud SDK is 366.0.0 or later:

       gcloud version

   If needed,
   [update the Google Cloud SDK](https://docs.cloud.google.com/sdk/docs/quickstart).
   1. Optional: For Terraform, `terraform-provider-google` version 4.25.0 or later is required. `terraform-provider-google` releases are listed on [GitHub](https://github.com/hashicorp/terraform-provider-google/releases). You can download the latest version of Terraform from [HashiCorp Terraform downloads](https://www.terraform.io/downloads).
7. Create a Cloud resource connection or set up a default connection to
   your external data source. Connections require additional roles and
   permissions. For more information, see [Create a Cloud resource
   connection](https://docs.cloud.google.com/bigquery/docs/create-cloud-resource-connection) and the
   [Default connection overview](https://docs.cloud.google.com/bigquery/docs/default-connections).

### Required roles

To create a BigLake table, you need the following
BigQuery Identity and Access Management (IAM) permissions:

- `bigquery.tables.create`
- `bigquery.connections.delegate`

The BigQuery Admin (`roles/bigquery.admin`) predefined
Identity and Access Management role includes these permissions.

If you are not a principal in this role, ask your administrator
to grant you access or to create the BigLake table for you.

For more information on Identity and Access Management roles and permissions in
BigQuery, see [Predefined roles and
permissions](https://docs.cloud.google.com/bigquery/docs/access-control).

### Location consideration

When you use Cloud Storage to store data files, you can improve
performance by using
Cloud Storage
[single-region](https://docs.cloud.google.com/storage/docs/locations#available-locations) or
[dual-region](https://docs.cloud.google.com/storage/docs/locations#location-dr) buckets instead of
multi-region buckets.

## Create external tables on unpartitioned data

If you're familiar with creating tables in BigQuery, the process
of creating an external table is similar.
Your table can use any file format that external tables support. For
more information, see
[Limitations](https://docs.cloud.google.com/bigquery/docs/biglake-intro#limitations).

Before you create a BigLake table, you need to have a
[dataset](https://docs.cloud.google.com/bigquery/docs/datasets) and a [Cloud resource
connection](https://docs.cloud.google.com/bigquery/docs/create-cloud-resource-connection#create-cloud-resource-connection)
that can [access Cloud Storage](https://docs.cloud.google.com/bigquery/docs/create-cloud-resource-connection#access-storage).

To create a BigLake table, select one of the following options:

### Console

1. Go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)

   If you don't see the left pane, click **Expand left pane** to open the pane.
3. In the **Explorer** pane, expand your project, click **Datasets**, and
   then select a dataset.

4. Expand the

   **Actions** option and click **Create table**.

5. In the **Source** section, specify the following details:

   1. For **Create table from** , select **Google Cloud Storage**

   2. For **Select file from GCS bucket or use a URI pattern** , browse to
      select a bucket and file to use, or type the path in the format
      `gs://bucket_name/[folder_name/]file_name`.

      You can't specify multiple URIs in the Google Cloud console, but
      you can select multiple files by specifying one asterisk (`*`)
      wildcard character. For example, `gs://mybucket/file_name*`. For more
      information, see
      [Wildcard support for Cloud Storage URIs](https://docs.cloud.google.com/bigquery/docs/external-data-cloud-storage#wildcard-support).

      The Cloud Storage bucket
      must be in the same location as the dataset that contains the table
      you're creating.
   3. For **File format**, select the format that matches your file.

6. In the **Destination** section, specify the following details:

   1. For **Project**, choose the project in which to create the table.

   2. For **Dataset**, choose the dataset in which to create the table.

   3. For **Table**, enter the name of the table you are creating.

   4. For **Table type** , select **External table**.

   5. Select **Create a BigLake table using a Cloud Resource connection**.

   6. For **Connection ID**, select the connection that you created earlier.

7. In the **Schema** section, you can either enable
   [schema auto-detection](https://docs.cloud.google.com/bigquery/docs/schema-detect) or manually specify
   a schema if you have a source file. If you don't have a source file, you
   must manually specify a schema.

   - To enable schema auto-detection, select the **Auto-detect** option.

   - To manually specify a schema, leave the **Auto-detect** option
     unchecked. Enable **Edit as text** and enter the table schema as a
     [JSON array](https://docs.cloud.google.com/bigquery/docs/schemas#specifying_a_json_schema_file).

8. To ignore rows with extra column values that do not match the schema,
   expand the **Advanced options** section and select **Unknown values**.

9. Click **Create table**.

After the permanent table is created, you can run a query against the table
as if it were a native BigQuery table. After your query
completes, you can [export the results](https://docs.cloud.google.com/bigquery/docs/writing-results)
as CSV or JSON files, save the results
as a table, or save the results to Google Sheets.

### SQL

Use the
[`CREATE EXTERNAL TABLE` DDL statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_external_table_statement).
You can specify the schema explicitly, or use
[schema auto-detection](https://docs.cloud.google.com/bigquery/docs/schema-detect) to infer the schema
from the external data.

<br />

<br />

<br />

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, enter the following statement:

   ```googlesql
   CREATE EXTERNAL TABLE `PROJECT_ID.DATASET.EXTERNAL_TABLE_NAME`
     WITH CONNECTION {`PROJECT_ID.REGION.CONNECTION_ID` | DEFAULT}
     OPTIONS (
       format ="TABLE_FORMAT",
       uris = ['BUCKET_PATH'[,...]],
       max_staleness = STALENESS_INTERVAL,
       metadata_cache_mode = 'CACHE_MODE'
       );
   ```


   Replace the following:
   - `PROJECT_ID`: the name of your project in which you want to create the table---for example, `myproject`
   - `DATASET`: the name of the BigQuery dataset that you want to create the table in---for example, `mydataset`
   - `EXTERNAL_TABLE_NAME`: the name of the table that you want to create---for example, `mytable`
   - `REGION`: the region that contains the connection---for example, `us`
   - `CONNECTION_ID`: the connection ID---for example, `myconnection`

     When you [view the connection details](https://docs.cloud.google.com/bigquery/docs/working-with-connections#view-connections) in the Google Cloud console, the connection
     ID is the value in the last section of the fully qualified connection ID
     that's shown in **Connection ID** ---for example
     `projects/myproject/locations/connection_location/connections/*myconnection*`.

     To use a [default connection](https://docs.cloud.google.com/bigquery/docs/default-connections), specify `DEFAULT` instead of the
     connection string containing
     <var translate="no">PROJECT_ID</var>.<var translate="no">REGION</var>.<var translate="no">CONNECTION_ID</var>.
   - `TABLE_FORMAT`: the format of the table that you want to create---for example, `PARQUET`

     For more information about supported formats, see
     [Limitations](https://docs.cloud.google.com/bigquery/docs/biglake-intro#limitations).
   - `BUCKET_PATH`: the path to the Cloud Storage bucket that contains the data for the external table, in the format `['gs://bucket_name/[folder_name/]file_name']`.

     You can select multiple files from the bucket by specifying one asterisk (`*`)
     wildcard character in the path. For example, `['gs://mybucket/file_name*']`. For more
     information, see
     [Wildcard support for Cloud Storage URIs](https://docs.cloud.google.com/bigquery/docs/external-data-cloud-storage#wildcard-support).

     You can specify multiple buckets for the `uris` option by providing multiple
     paths.

     The following examples show valid `uris` values:
     - `['gs://bucket/path1/myfile.csv']`
     - `['gs://bucket/path1/*.csv']`
     - `['gs://bucket/path1/*', 'gs://bucket/path2/file00*']`

     When you specify `uris` values that target multiple files, all of those
     files must share a compatible schema.

     For more information about using Cloud Storage URIs in
     BigQuery, see
     [Cloud Storage resource path](https://docs.cloud.google.com/bigquery/docs/external-data-cloud-storage#google-cloud-storage-uri).
   - `STALENESS_INTERVAL`: specifies whether cached metadata is used by operations against the BigLake table, and how fresh the cached metadata must be in order for the operation to use it. For more information about metadata caching considerations, see [Metadata caching for performance](https://docs.cloud.google.com/bigquery/docs/biglake-intro#metadata_caching_for_performance).

     To disable metadata caching, specify 0. This is the default.

     To enable metadata caching, specify an
     [interval literal](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/lexical#interval_literals)
     value between 30 minutes and 7 days. For example, specify
     `INTERVAL 4 HOUR` for a 4 hour staleness interval.
     With this value, operations against the table use cached metadata if
     it has been refreshed within the past 4 hours. If the cached metadata
     is older than that, the operation retrieves metadata from
     Cloud Storage instead.
   - `CACHE_MODE`: specifies whether the metadata cache is refreshed automatically or manually. For more information on metadata caching considerations, see [Metadata caching for performance](https://docs.cloud.google.com/bigquery/docs/biglake-intro#metadata_caching_for_performance).

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

### bq

**Option 1: Table definition file**

Use the [`bq mkdef` command](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#bq_mkdef)
to create a table definition file, and then pass the path to
the [`bq mk` command](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#bq_mk) as follows:

<br />

```bash
bq mkdef \
    --connection_id=CONNECTION_ID \
    --source_format=SOURCE_FORMAT \
  BUCKET_PATH > DEFINITION_FILE

bq mk --table \
    --external_table_definition=DEFINITION_FILE \
    --max_staleness=STALENESS_INTERVAL \
    PROJECT_ID:DATASET.EXTERNAL_TABLE_NAME \
    SCHEMA
```

Replace the following:

- `CONNECTION_ID`: the connection ID---for
  example, `myconnection`

  When you [view the connection details](https://docs.cloud.google.com/bigquery/docs/working-with-connections#view-connections)
  in the Google Cloud console, the connection ID is the value in the last section of the
  fully qualified connection ID that is shown in **Connection ID** ---for example
  `projects/myproject/locations/connection_location/connections/*myconnection*`.

  To use a [default connection](https://docs.cloud.google.com/bigquery/docs/default-connections),
  specify `DEFAULT` instead of the connection string containing
  <var translate="no">PROJECT_ID</var>.<var translate="no">REGION</var>.<var translate="no">CONNECTION_ID</var>.
- `SOURCE_FORMAT`: the format of the external data source.
  For example, `PARQUET`.

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
- `DEFINITION_FILE`: the path to the
  [table definition file](https://docs.cloud.google.com/bigquery/docs/external-table-definition)
  on your local machine.

- `STALENESS_INTERVAL`: specifies whether
  cached metadata is used by operations against the
  BigLake table, and
  how fresh the cached metadata must be in order for the operation to
  use it. For more information about metadata caching considerations, see
  [Metadata caching for performance](https://docs.cloud.google.com/bigquery/docs/biglake-intro#metadata_caching_for_performance).

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
- `DATASET`: the name of the
  BigQuery dataset that you want to create a table
  in---for example, `mydataset`

- `EXTERNAL_TABLE_NAME`: the name of the table
  that you want to create---for example, `mytable`

- `SCHEMA`: the schema for the
  BigLake table

Example:

```bash
bq mkdef
    --connection_id=myconnection
    --metadata_cache_mode=CACHE_MODE
    --source_format=CSV 'gs://mybucket/*.csv' > mytable_def

bq mk
    --table
    --external_table_definition=mytable_def='gs://mybucket/*.csv'
    --max_staleness=0-0 0 4:0:0
    myproject:mydataset.mybiglaketable
    Region:STRING,Quarter:STRING,Total_sales:INTEGER
```

To use schema auto-detection, set the `--autodetect=true` flag in the
`mkdef` command and omit the schema:

```bash
bq mkdef \
    --connection_id=myconnection \
    --metadata_cache_mode=CACHE_MODE \
    --source_format=CSV --autodetect=true \
    gs://mybucket/*.csv > mytable_def

bq mk \
    --table \
    --external_table_definition=mytable_def=gs://mybucket/*.csv \
    --max_staleness=0-0 0 4:0:0 \
    myproject:mydataset.myexternaltable
```

**Option 2: Inline table definition**

Instead of creating a table definition file, you can pass the table
definition directly to the [`bq mk` command](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#bq_mk).
Use the `@connection` decorator to specify the connection to use at the end
of the
[`--external_table_definition`](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#external_table_definition_flag) flag.

```bash
bq mk --table \
  --external_table_definition=@SOURCE_FORMAT=BUCKET_PATH@projects/PROJECT_ID/locations/REGION/connections/CONNECTION_ID \
  DATASET_NAME.TABLE_NAME \
  SCHEMA
```

Replace the following:

- `SOURCE_FORMAT`: the format of the external data source

  For example, `CSV`.
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
- `PROJECT_ID`: the name of your
  project in which you want to create the table---for example, `myproject`

- `REGION`: the region that contains the
  connection, `us`

- `CONNECTION_ID`: the connection ID---for
  example, `myconnection`

  When you [view the connection details](https://docs.cloud.google.com/bigquery/docs/working-with-connections#view-connections)
  in the Google Cloud console, the connection ID is the value in the last section of the
  fully qualified connection ID that is shown in **Connection ID** ---for example
  `projects/myproject/locations/connection_location/connections/*myconnection*`.

  To use a [default connection](https://docs.cloud.google.com/bigquery/docs/default-connections),
  specify `DEFAULT` instead of the connection string containing
  <var translate="no">PROJECT_ID</var>.<var translate="no">REGION</var>.<var translate="no">CONNECTION_ID</var>.
- `DATASET_NAME`: the name of the dataset where you
  want to create the BigLake table

- `TABLE_NAME`: the name of the BigLake table

- `SCHEMA`: the schema for the
  BigLake table

Example:

```
bq mk --table \
    --external_table_definition=@CSV=gs://mybucket/*.parquet@projects/myproject/locations/us/connections/myconnection \
    --max_staleness=0-0 0 4:0:0 \
    myproject:mydataset.myexternaltable \
    Region:STRING,Quarter:STRING,Total_sales:INTEGER
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
for connecting to Cloud Storage.

### Terraform


This example creates a BigLake Table on unpartitioned data.

To authenticate to BigQuery, set up Application Default
Credentials. For more information, see
[Set up authentication for client libraries](https://docs.cloud.google.com/bigquery/docs/authentication#client-libs).


    # This creates a bucket in the US region named "my-bucket" with a pseudorandom suffix.
    resource "random_id" "default" {
      byte_length = 8
    }
    resource "google_storage_bucket" "default" {
      name                        = "my-bucket-${random_id.default.hex}"
      location                    = "US"
      force_destroy               = true
      uniform_bucket_level_access = true
    }

    # This queries the provider for project information.
    data "google_project" "project" {}

    # This creates a connection in the US region named "my-connection".
    # This connection is used to access the bucket.
    resource "google_bigquery_connection" "default" {
      connection_id = "my-connection"
      location      = "US"
      cloud_resource {}
    }

    # This grants the previous connection IAM role access to the bucket.
    resource "google_project_iam_member" "default" {
      role    = "roles/storage.objectViewer"
      project = data.google_project.project.id
      member  = "serviceAccount:${google_bigquery_connection.default.cloud_resource[0].service_account_id}"
    }

    # This makes the script wait for seven minutes before proceeding.
    # This lets IAM permissions propagate.
    resource "time_sleep" "default" {
      create_duration = "7m"

      depends_on = [google_project_iam_member.default]
    }

    # This defines a Google BigQuery dataset with
    # default expiration times for partitions and tables, a
    # description, a location, and a maximum time travel.
    resource "google_bigquery_dataset" "default" {
      dataset_id                      = "my_dataset"
      default_partition_expiration_ms = 2592000000  # 30 days
      default_table_expiration_ms     = 31536000000 # 365 days
      description                     = "My dataset description"
      location                        = "US"
      max_time_travel_hours           = 96 # 4 days

      # This defines a map of labels for the bucket resource,
      # including the labels "billing_group" and "pii".
      labels = {
        billing_group = "accounting",
        pii           = "sensitive"
      }
    }


    # This creates a BigQuery Table with automatic metadata caching.
    resource "google_bigquery_table" "default" {
      dataset_id = google_bigquery_dataset.default.dataset_id
      table_id   = "my_table"
      schema = jsonencode([
        { "name" : "country", "type" : "STRING" },
        { "name" : "product", "type" : "STRING" },
        { "name" : "price", "type" : "INT64" }
      ])
      external_data_configuration {
        # This defines an external data configuration for the BigQuery table
        # that reads Parquet data from the publish directory of the default
        # Google Cloud Storage bucket.
        autodetect    = false
        source_format = "PARQUET"
        connection_id = google_bigquery_connection.default.name
        source_uris   = ["gs://${google_storage_bucket.default.name}/data/*"]
        # This enables automatic metadata refresh.
        metadata_cache_mode = "AUTOMATIC"
      }

      # This sets the maximum staleness of the metadata cache to 10 hours.
      max_staleness = "0-0 0 10:0:0"

      depends_on = [time_sleep.default]
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

BigLake supports schema autodetection. However, if you did not
provide a schema and the service account was not granted access in the previous
steps, these steps fail with an access denied message if you try to autodetect
the schema.

## Create BigLake tables on Apache Hive partitioned data

You can create a BigLake table for Hive partitioned data in
Cloud Storage. After you create an externally partitioned table, you
can't change the partition key. You need to recreate the table to change the
partition key.

To create a BigLake table based on Hive partitioned data in
Cloud Storage, select one of the following options:

### Console

1. Go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)

   If you don't see the left pane, click **Expand left pane** to open the pane.
3. In the **Explorer** pane, expand your project, click **Datasets**, and
   then click a dataset.

4. Click **Create table** . The **Create table** pane opens.

5. In the **Source** section, specify the following details:

   1. For **Create table from** , select **Google Cloud Storage**.

   2. Provide the path to the folder, using
      [wildcards](https://docs.cloud.google.com/bigquery/docs/batch-loading-data#load-wildcards).
      For example, `my_bucket/my_files*`.
      The folder
      must be in the same location as the dataset that contains the
      table you want to create, append, or overwrite.

   3. From the **File format** list, select the file type.

   4. Select the **Source data partitioning** checkbox, and then specify
      the following details:

      1. For **Select Source URI Prefix** , enter the URI prefix. For example, `gs://my_bucket/my_files`.
      2. Optional: To require a partition filter on all queries for this table, select the **Require partition filter** checkbox. Requiring a partition filter can reduce cost and improve performance. For more information, see [Requiring predicate filters on partition keys in queries](https://docs.cloud.google.com/bigquery/docs/hive-partitioned-queries-gcs#requiring_predicate_filters_on_partition_keys_in_queries).
      3. In the **Partition inference mode** section, select one of the
         following options:

         - **Automatically infer types** : set the partition schema detection mode to `AUTO`.
         - **All columns are strings** : set the partition schema detection mode to `STRINGS`.
         - **Provide my own** : set the partition schema detection mode to `CUSTOM` and manually enter the schema information for the partition keys. For more information, see [Provide a custom partition key schema](https://docs.cloud.google.com/bigquery/docs/hive-partitioned-loads-gcs#custom_partition_key_schema).
6. In the **Destination** section, specify the following details:

   1. For **Project**, select the project in which you want to create the table.
   2. For **Dataset**, select the dataset in which you want to create the table.
   3. For **Table**, enter the name of the table that you want to create.
   4. For **Table type** , select **External table**.
   5. Select the **Create a BigLake table using a Cloud Resource
      connection** checkbox.
   6. For **Connection ID**, select the connection that you created earlier.
7. In the **Schema** section, enable
   [schema auto-detection](https://docs.cloud.google.com/bigquery/docs/schema-detect) by selecting the
   **Auto detect** option.

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
   WITH CONNECTION {`PROJECT_ID.REGION.CONNECTION_ID` | DEFAULT}
   OPTIONS (
     hive_partition_uri_prefix = "HIVE_PARTITION_URI_PREFIX",
     uris=['FILE_PATH'],
     max_staleness = STALENESS_INTERVAL,
     metadata_cache_mode = 'CACHE_MODE',
     format ="TABLE_FORMAT"
   );
   ```


   Replace the following:
   - `PROJECT_ID`: the name of your project in which you want to create the table---for example, `myproject`
   - `DATASET`: the name of the BigQuery dataset that you want to create the table in---for example, `mydataset`
   - `EXTERNAL_TABLE_NAME`: the name of the table that you want to create---for example, `mytable`
   - `PARTITION_COLUMN`: the name of the partitioning column
   - `PARTITION_COLUMN_TYPE`: the type of the partitioning column
   - `REGION`: the region that contains the connection---for example, `us`
   - `CONNECTION_ID`: the connection ID---for example, `myconnection`

     When you [view the connection details](https://docs.cloud.google.com/bigquery/docs/working-with-connections#view-connections)
     in the Google Cloud console, the connection ID is the value in the last section of the
     fully qualified connection ID that is shown in **Connection ID** ---for example
     `projects/myproject/locations/connection_location/connections/*myconnection*`.

     To use a [default connection](https://docs.cloud.google.com/bigquery/docs/default-connections), specify `DEFAULT` instead of the
     connection string containing
     <var translate="no">PROJECT_ID</var>.<var translate="no">REGION</var>.<var translate="no">CONNECTION_ID</var>.
   - `HIVE_PARTITION_URI_PREFIX`: hive partitioning uri prefix--for example, `gs://mybucket/`
   - `FILE_PATH`: path to the data source for the external table that you want to create---for example, `gs://mybucket/*.parquet`
   - `STALENESS_INTERVAL`: specifies whether cached metadata is used by operations against the BigLake table, and how fresh the cached metadata must be in order for the operation to use it. For more information about metadata caching considerations, see [Metadata caching for performance](https://docs.cloud.google.com/bigquery/docs/biglake-intro#metadata_caching_for_performance).

     To disable metadata caching, specify 0. This is the default.

     To enable metadata caching, specify an
     [interval literal](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/lexical#interval_literals)
     value between 30 minutes and 7 days. For example, specify
     `INTERVAL 4 HOUR` for a 4 hour staleness interval.
     With this value, operations against the table use cached metadata if
     it has been refreshed within the past 4 hours. If the cached metadata
     is older than that, the operation retrieves metadata from
     Cloud Storage instead.
   - `CACHE_MODE`: specifies whether the metadata cache is refreshed automatically or manually. For more information about metadata caching considerations, see [Metadata caching for performance](https://docs.cloud.google.com/bigquery/docs/biglake-intro#metadata_caching_for_performance).

     Set to `AUTOMATIC` for the metadata cache to be
     refreshed at a system-defined interval, usually somewhere between 30 and
     60 minutes.

     Set to `MANUAL` if you want to refresh
     the metadata cache on a schedule you determine. In this case, you can call
     the [`BQ.REFRESH_EXTERNAL_METADATA_CACHE` system procedure](https://docs.cloud.google.com/bigquery/docs/reference/system-procedures#bqrefresh_external_metadata_cache) to refresh the cache.

     You must set `CACHE_MODE` if
     `STALENESS_INTERVAL` is set to a value greater
     than 0.
   - `TABLE_FORMAT`: the format of the table that you want to create---for example, `PARQUET`

   <br />

3. Click **Run**.

   <br />

For more information about how to run queries, see [Run an interactive query](https://docs.cloud.google.com/bigquery/docs/running-queries#queries).

**Examples**

The following example creates a BigLake table over
partitioned data where:

- The schema is autodetected.
- The metadata cache staleness interval for the table is 1 day.
- The metadata cache refreshes automatically.

```googlesql
CREATE EXTERNAL TABLE `my_dataset.my_table`
WITH PARTITION COLUMNS
(
  sku STRING,
)
WITH CONNECTION `us.my-connection`
OPTIONS(
  hive_partition_uri_prefix = "gs://mybucket/products",
  uris = ['gs://mybucket/products/*'],
  max_staleness = INTERVAL 1 DAY,
  metadata_cache_mode = 'AUTOMATIC'
);
```

The following example creates a BigLake table over
partitioned data where:

- The schema is specified.
- The metadata cache staleness interval for the table is 8 hours.
- The metadata cache must be manually refreshed.

```googlesql
CREATE EXTERNAL TABLE `my_dataset.my_table`
(
  ProductId INTEGER,
  ProductName STRING,
  ProductType STRING
)
WITH PARTITION COLUMNS
(
  sku STRING,
)
WITH CONNECTION `us.my-connection`
OPTIONS(
  hive_partition_uri_prefix = "gs://mybucket/products",
  uris = ['gs://mybucket/products/*'],
  max_staleness = INTERVAL 8 HOUR,
  metadata_cache_mode = 'MANUAL'
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
--hive_partitioning_source_uri_prefix=GCS_URI_SHARED_PREFIX \
--require_hive_partition_filter=BOOLEAN \
--metadata_cache_mode=CACHE_MODE \
 GCS_URIS > DEFINITION_FILE
```

Replace the following:

- `SOURCE_FORMAT`: the format of the external data source. For example, `CSV`.
- `REGION`: the region that contains the connection---for example, `us`.
- `CONNECTION_ID`: the connection ID---for
  example, `myconnection`.

  When you [view the connection details](https://docs.cloud.google.com/bigquery/docs/working-with-connections#view-connections)
  in the Google Cloud console, the connection ID is the value in the last section of the
  fully qualified connection ID that is shown in **Connection ID** ---for example
  `projects/myproject/locations/connection_location/connections/*myconnection*`.

  To use a [default connection](https://docs.cloud.google.com/bigquery/docs/default-connections),
  specify `DEFAULT` instead of the connection string containing
  <var translate="no">PROJECT_ID</var>.<var translate="no">REGION</var>.<var translate="no">CONNECTION_ID</var>.
- `PARTITIONING_MODE`: the Hive partitioning mode. Use one of the
  following values:

  - `AUTO`: Automatically detect the key names and types.
  - `STRINGS`: Automatically convert the key names to strings.
  - `CUSTOM`: Encode the key schema in the source URI prefix.
- `GCS_URI_SHARED_PREFIX`: the source URI prefix.

- `BOOLEAN`: specifies whether to require a predicate filter at query
  time. This flag is optional. The default value is `false`.

- `CACHE_MODE`: specifies whether the metadata
  cache is refreshed automatically or manually. You only need to include this
  flag if you also plan to use the `--max_staleness` flag in the
  subsequent `bq mk` command to enable metadata caching. For more
  information on metadata caching considerations, see
  [Metadata caching for performance](https://docs.cloud.google.com/bigquery/docs/biglake-intro#metadata_caching_for_performance).

  Set to `AUTOMATIC` for the metadata cache to be
  refreshed at a system-defined interval, usually somewhere between 30 and
  60 minutes.

  Set to `MANUAL` if you want to refresh
  the metadata cache on a schedule you determine. In this case, you can call
  the
  [`BQ.REFRESH_EXTERNAL_METADATA_CACHE` system procedure](https://docs.cloud.google.com/bigquery/docs/reference/system-procedures#bqrefresh_external_metadata_cache) to refresh the cache.

  You must set `CACHE_MODE` if
  `STALENESS_INTERVAL` is set to a value greater
  than 0.
- `GCS_URIS`: the path to the Cloud Storage folder, using
  wildcard format.

- `DEFINITION_FILE`: the path to the
  [table definition file](https://docs.cloud.google.com/bigquery/external-table-definition) on your local
  machine.

If `PARTITIONING_MODE` is `CUSTOM`, include the partition key schema
in the source URI prefix, using the following format:

```bash
--hive_partitioning_source_uri_prefix=GCS_URI_SHARED_PREFIX/{KEY1:TYPE1}/{KEY2:TYPE2}/...
```

After you create the table definition file, use the
[`bq mk`](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#mk-table) command to
create the BigLake table:

```bash
bq mk --external_table_definition=DEFINITION_FILE \
--max_staleness=STALENESS_INTERVAL \
DATASET_NAME.TABLE_NAME \
SCHEMA
```

Replace the following:

- `DEFINITION_FILE`: the path to the table definition file.
- `STALENESS_INTERVAL`: specifies whether
  cached metadata is used by operations against the
  BigLake table, and
  how fresh the cached metadata must be in order for the operation to
  use it. If you include this flag, you must have also specified a value
  for the `--metadata_cache_mode` flag in the preceding
  `bq mkdef` command. For more information on metadata
  caching considerations, see
  [Metadata caching for performance](https://docs.cloud.google.com/bigquery/docs/biglake-intro#metadata_caching_for_performance).

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
- `DATASET_NAME`: the name of the dataset that contains the
  table.

- `TABLE_NAME`: the name of the table you're creating.

- `SCHEMA`: specifies a path to a
  [JSON schema file](https://docs.cloud.google.com/bigquery/docs/schemas#specifying_a_json_schema_file),
  or specifies the schema in the form
  `field:data_type,field:data_type,...`. To use schema
  auto-detection, omit this argument.

**Examples**

The following example uses `AUTO` Hive partitioning mode, and also
sets the metadata cache to have a 12 hour staleness interval and to get
refreshed automatically:

    bq mkdef --source_format=CSV \
      --connection_id=us.my-connection \
      --hive_partitioning_mode=AUTO \
      --hive_partitioning_source_uri_prefix=gs://myBucket/myTable \
      --metadata_cache_mode=AUTOMATIC \
      gs://myBucket/myTable/* > mytable_def

    bq mk --external_table_definition=mytable_def \
      --max_staleness=0-0 0 12:0:0 \
      mydataset.mytable \
      Region:STRING,Quarter:STRING,Total_sales:INTEGER

The following example uses `STRING` Hive partitioning mode:

    bq mkdef --source_format=CSV \
      --connection_id=us.my-connection \
      --hive_partitioning_mode=STRING \
      --hive_partitioning_source_uri_prefix=gs://myBucket/myTable \
      gs://myBucket/myTable/* > mytable_def

    bq mk --external_table_definition=mytable_def \
      mydataset.mytable \
      Region:STRING,Quarter:STRING,Total_sales:INTEGER

The following example uses `CUSTOM` Hive partitioning mode:

    bq mkdef --source_format=CSV \
      --connection_id=us.my-connection \
      --hive_partitioning_mode=CUSTOM \
      --hive_partitioning_source_uri_prefix=gs://myBucket/myTable/{dt:DATE}/{val:STRING} \
      gs://myBucket/myTable/* > mytable_def

    bq mk --external_table_definition=mytable_def \
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
`gs://BUCKET/PATH_TO_TABLE/{KEY1:TYPE1}/{KEY2:TYPE2}/...`

To enforce the use of a predicate filter at query time, set the
`hivePartitioningOptions.requirePartitionFilter` field to `true`.

### Terraform

This example creates a BigLake Table on partitioned data.

To authenticate to BigQuery, set up Application Default
Credentials. For more information, see
[Set up authentication for client libraries](https://docs.cloud.google.com/bigquery/docs/authentication#client-libs).



    # This creates a bucket in the US region named "my-bucket" with a pseudorandom
    # suffix.
    resource "random_id" "default" {
      byte_length = 8
    }
    resource "google_storage_bucket" "default" {
      name                        = "my-bucket-${random_id.default.hex}"
      location                    = "US"
      force_destroy               = true
      uniform_bucket_level_access = true
    }

    resource "google_storage_bucket_object" "default" {
      # This creates a fake message to create partition locations on the table.
      # Otherwise, the table deployment fails.
      name    = "publish/dt=2000-01-01/hr=00/min=00/fake_message.json"
      content = "{\"column1\": \"XXX\"}"
      bucket  = google_storage_bucket.default.name
    }

    # This queries the provider for project information.
    data "google_project" "default" {}

    # This creates a connection in the US region named "my-connection".
    # This connection is used to access the bucket.
    resource "google_bigquery_connection" "default" {
      connection_id = "my-connection"
      location      = "US"
      cloud_resource {}
    }

    # This grants the previous connection IAM role access to the bucket.
    resource "google_project_iam_member" "default" {
      role    = "roles/storage.objectViewer"
      project = data.google_project.default.id
      member  = "serviceAccount:${google_bigquery_connection.default.cloud_resource[0].service_account_id}"
    }

    # This makes the script wait for seven minutes before proceeding. This lets IAM
    # permissions propagate.
    resource "time_sleep" "default" {
      create_duration = "7m"

      depends_on = [google_project_iam_member.default]
    }

    # This defines a Google BigQuery dataset with default expiration times for
    # partitions and tables, a description, a location, and a maximum time travel.
    resource "google_bigquery_dataset" "default" {
      dataset_id                      = "my_dataset"
      default_partition_expiration_ms = 2592000000  # 30 days
      default_table_expiration_ms     = 31536000000 # 365 days
      description                     = "My dataset description"
      location                        = "US"
      max_time_travel_hours           = 96 # 4 days

      # This defines a map of labels for the bucket resource,
      # including the labels "billing_group" and "pii".
      labels = {
        billing_group = "accounting",
        pii           = "sensitive"
      }
    }

    # This creates a BigQuery table with partitioning and automatic metadata
    # caching.
    resource "google_bigquery_table" "default" {
      dataset_id = google_bigquery_dataset.default.dataset_id
      table_id   = "my_table"
      schema     = jsonencode([{ "name" : "column1", "type" : "STRING", "mode" : "NULLABLE" }])
      external_data_configuration {
        # This defines an external data configuration for the BigQuery table
        # that reads Parquet data from the publish directory of the default
        # Google Cloud Storage bucket.
        autodetect    = false
        source_format = "PARQUET"
        connection_id = google_bigquery_connection.default.name
        source_uris   = ["gs://${google_storage_bucket.default.name}/publish/*"]
        # This configures Hive partitioning for the BigQuery table,
        # partitioning the data by date and time.
        hive_partitioning_options {
          mode                     = "CUSTOM"
          source_uri_prefix        = "gs://${google_storage_bucket.default.name}/publish/{dt:STRING}/{hr:STRING}/{min:STRING}"
          require_partition_filter = false
        }
        # This enables automatic metadata refresh.
        metadata_cache_mode = "AUTOMATIC"
      }


      # This sets the maximum staleness of the metadata cache to 10 hours.
      max_staleness = "0-0 0 10:0:0"

      depends_on = [
        time_sleep.default,
        google_storage_bucket_object.default
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

## Set up access control policies

You can use several methods to control access to BigLake tables:

- For directions on setting up column-level security, see the
  [column-level security guide](https://docs.cloud.google.com/bigquery/docs/column-level-security).

- For directions on setting up data masking, see the
  [data masking guide](https://docs.cloud.google.com/bigquery/docs/column-data-masking).

- For directions on setting up row-level security, see the
  [row-level security guide](https://docs.cloud.google.com/bigquery/docs/managing-row-level-security).

For example, let's say you want to limit row access for the table `mytable`
in the dataset `mydataset`:

```
+---+---+---+
| country | product | price |
+---+---+---+
| US      | phone   |   100 |
| JP      | tablet  |   300 |
| UK      | laptop  |   200 |
+---+---+---+
```

You can create a row-level filter for Kim (`kim@example.com`) that restricts
their access to rows where `country` is equal to `US`.

```googlesql
CREATE ROW ACCESS POLICY only_us_filter
ON mydataset.mytable
GRANT TO ('user:kim@example.com')
FILTER USING (country = 'US');
```

Then, Kim runs the following query:

```googlesql
SELECT * FROM projectid.mydataset.mytable;
```

The output shows only the rows where `country` is equal to `US`:

```
+---+---+---+
| country | product | price |
+---+---+---+
| US      | phone   |   100 |
+---+---+---+
```

## Query BigLake tables

For more information, see
[Query Cloud Storage data in BigLake tables](https://docs.cloud.google.com/bigquery/docs/query-cloud-storage-using-biglake).

## Update BigLake tables

You can update BigLake tables if necessary, for example to
change their
[metadata caching](https://docs.cloud.google.com/bigquery/docs/biglake-intro#metadata_caching_for_performance).
To get table details such as source format and source URI, see
[Get table information](https://docs.cloud.google.com/bigquery/docs/tables#get_table_information).

You can also use this same procedure to upgrade Cloud Storage-based
external tables to BigLake tables by associating the external
table to a connection. For more information, see
[Upgrade external tables to BigLake tables](https://docs.cloud.google.com/bigquery/docs/external-data-cloud-storage#upgrade-external-tables-to-biglake-tables).

To update a BigLake table, select one of the
following options:

### SQL

Use the
[`CREATE OR REPLACE EXTERNAL TABLE` DDL statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_external_table_statement)
to update a table:

<br />

<br />

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, enter the following statement:

   ```googlesql
   CREATE OR REPLACE EXTERNAL TABLE
     `PROJECT_ID.DATASET.EXTERNAL_TABLE_NAME`
     WITH CONNECTION {`REGION.CONNECTION_ID` | DEFAULT}
     OPTIONS(
       format ="TABLE_FORMAT",
       uris = ['BUCKET_PATH'],
       max_staleness = STALENESS_INTERVAL,
       metadata_cache_mode = 'CACHE_MODE'
       );
   ```


   Replace the following:
   - `PROJECT_ID`: the name of the project that contains the table
   - `DATASET`: the name of the dataset that contains the table
   - `EXTERNAL_TABLE_NAME`: the name of the table
   - `REGION`: the region that contains the connection
   - `CONNECTION_ID`: the name of the connection to use

     To use a [default connection](https://docs.cloud.google.com/bigquery/docs/default-connections), specify `DEFAULT` instead of the
     connection string containing
     `REGION.CONNECTION_ID`.
   - `TABLE_FORMAT`: the format used by the table

     <br />

     You can't change this when updating the table.
   - `BUCKET_PATH`: the path to the Cloud Storage bucket that contains the data for the external table, in the format `['gs://bucket_name/[folder_name/]file_name']`.

     You can select multiple files from the bucket by specifying one asterisk (`*`)
     wildcard character in the path. For example, `['gs://mybucket/file_name*']`. For more
     information, see
     [Wildcard support for Cloud Storage URIs](https://docs.cloud.google.com/bigquery/docs/external-data-cloud-storage#wildcard-support).

     You can specify multiple buckets for the `uris` option by providing multiple
     paths.

     The following examples show valid `uris` values:
     - `['gs://bucket/path1/myfile.csv']`
     - `['gs://bucket/path1/*.csv']`
     - `['gs://bucket/path1/*', 'gs://bucket/path2/file00*']`

     When you specify `uris` values that target multiple files, all of those
     files must share a compatible schema.

     For more information about using Cloud Storage URIs in
     BigQuery, see
     [Cloud Storage resource path](https://docs.cloud.google.com/bigquery/docs/external-data-cloud-storage#google-cloud-storage-uri).
   - `STALENESS_INTERVAL`: specifies whether cached metadata is used by operations against the table, and how fresh the cached metadata must be in order for the operation to use it

     <br />

     For more information about metadata caching considerations, see
     [Metadata caching for performance](https://docs.cloud.google.com/bigquery/docs/biglake-intro#metadata_caching_for_performance).

     To disable metadata caching, specify 0. This is the default.

     To enable metadata caching, specify an
     [interval literal](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/lexical#interval_literals)
     value between 30 minutes and 7 days. For example, specify
     `INTERVAL 4 HOUR` for a 4 hour staleness interval.
     With this value, operations against the table use cached metadata if
     it has been refreshed within the past 4 hours. If the cached metadata
     is older than that, the operation retrieves metadata from
     Cloud Storage instead.
   - `CACHE_MODE`: specifies whether the metadata cache is refreshed automatically or manually

     <br />

     For more information
     on metadata caching considerations, see
     [Metadata caching for performance](https://docs.cloud.google.com/bigquery/docs/biglake-intro#metadata_caching_for_performance).

     Set to `AUTOMATIC` for the metadata cache to be
     refreshed at a system-defined interval, usually somewhere between 30 and
     60 minutes.

     Set to `MANUAL` if you want to refresh
     the metadata cache on a schedule you determine. In this case, you can call
     the [`BQ.REFRESH_EXTERNAL_METADATA_CACHE` system procedure](https://docs.cloud.google.com/bigquery/docs/reference/system-procedures#bqrefresh_external_metadata_cache) to refresh
     the cache.

     You must set `CACHE_MODE` if
     `STALENESS_INTERVAL` is set to a value greater
     than 0.

   <br />

3. Click **Run**.

   <br />

For more information about how to run queries, see [Run an interactive query](https://docs.cloud.google.com/bigquery/docs/running-queries#queries).

### bq

Use the [`bq mkdef`](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#bq_mkdef) and
[`bq update`](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#bq_update) commands
to update a table:

1. Generate an
   [external table definition](https://docs.cloud.google.com/bigquery/external-table-definition#table-definition),
   that describes the aspects of the table to change:

   ```bash
   bq mkdef --connection_id=PROJECT_ID.REGION.CONNECTION_ID \
   --source_format=TABLE_FORMAT \
   --metadata_cache_mode=CACHE_MODE \
   "BUCKET_PATH" > /tmp/DEFINITION_FILE
   ```

   Replace the following:
   - `PROJECT_ID`: the name of the project that contains the connection
   - `REGION`: the region that contains the connection
   - `CONNECTION_ID`: the name of the connection to use
   - `TABLE_FORMAT`: the format used by the table. You can't change this when updating the table.
   - `CACHE_MODE`: specifies whether the metadata
     cache is refreshed automatically or manually. For more information
     on metadata caching considerations, see
     [Metadata caching for performance](https://docs.cloud.google.com/bigquery/docs/biglake-intro#metadata_caching_for_performance).

     Set to `AUTOMATIC` for the metadata cache to be refreshed at a
     system-defined interval, usually somewhere between 30 and
     60 minutes.

     Set to `MANUAL` if you want to refresh the metadata cache on a
     schedule you determine. In this case, you can call the
     [`BQ.REFRESH_EXTERNAL_METADATA_CACHE` system procedure](https://docs.cloud.google.com/bigquery/docs/reference/system-procedures#bqrefresh_external_metadata_cache) to refresh
     the cache.

     You must set `CACHE_MODE` if
     `STALENESS_INTERVAL` is set to a value
     greater than 0.
   - `BUCKET_PATH`: the path to the
     Cloud Storage bucket that contains the data for the
     external table, in the format
     `gs://bucket_name/[folder_name/]file_name`.

     You can limit the files selected from the bucket by specifying one asterisk (`*`)
     wildcard character in the path. For example, `gs://mybucket/file_name*`. For more
     information, see
     [Wildcard support for Cloud Storage URIs](https://docs.cloud.google.com/bigquery/docs/external-data-cloud-storage#wildcard-support).

     You can specify multiple buckets for the `uris` option by providing multiple
     paths.

     The following examples show valid `uris` values:
     - `gs://bucket/path1/myfile.csv`
     - `gs://bucket/path1/*.csv`
     - `gs://bucket/path1/*,gs://bucket/path2/file00*`

     When you specify `uris` values that target multiple files, all of those
     files must share a compatible schema.

     For more information about using Cloud Storage URIs in
     BigQuery, see
     [Cloud Storage resource path](https://docs.cloud.google.com/bigquery/docs/external-data-cloud-storage#google-cloud-storage-uri).
   - `DEFINITION_FILE`: the name of the table
     definition file that you are creating.

2. Update the table using the new external table definition:

   ```bash
   bq update --max_staleness=STALENESS_INTERVAL \
   --external_table_definition=/tmp/DEFINITION_FILE \
   PROJECT_ID:DATASET.EXTERNAL_TABLE_NAME
   ```

   Replace the following:
   - `STALENESS_INTERVAL`: specifies whether
     cached metadata is used by operations against the
     table, and how fresh the cached metadata must be in order for
     the operation to use it. For more information about metadata
     caching considerations, see
     [Metadata caching for performance](https://docs.cloud.google.com/bigquery/docs/biglake-intro#metadata_caching_for_performance).

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
   - `DEFINITION_FILE`: the name of the table
     definition file that you created or updated.

   - `PROJECT_ID`: the name of the project
     that contains the table

   - `DATASET`: the name of the dataset that
     contains the table

   - `EXTERNAL_TABLE_NAME`: the name of the table

**Example**

The following example updates `mytable` to use cached metadata as long as
it has been refreshed in the last 4.5 hours, and also to refresh cached
metadata automatically:

    bq update --project_id=myproject --max_staleness='0-0 0 4:30:0' \
    --external_table_definition=enable_metadata.json mydataset.mytable

Where `enable_metadata.json` has the following contents:
`json
{
"metadataCacheMode": "AUTOMATIC"
}`

## Audit logging

For information about logging in BigQuery, see [Introduction to
BigQuery monitoring](https://docs.cloud.google.com/bigquery/docs/monitoring). To learn more
about logging in Google Cloud, see [Cloud Logging](https://docs.cloud.google.com/logging/docs).

## What's next

- Learn more about [BigLake](https://docs.cloud.google.com/bigquery/docs/biglake-intro).
- Learn about [Cloud Storage](https://docs.cloud.google.com/storage/docs/introduction).
- Learn about [querying Amazon Web Services (AWS) data](https://docs.cloud.google.com/bigquery/docs/omni-aws-introduction).
- Learn about [querying Microsoft Azure data](https://docs.cloud.google.com/bigquery/docs/omni-azure-introduction).