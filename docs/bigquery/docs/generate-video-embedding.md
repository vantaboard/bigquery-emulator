# Generate video embeddings by using the AI.GENERATE_EMBEDDING function

This document shows you how to create a BigQuery ML
[remote model](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model)
that references a Vertex AI embedding
[foundation model](https://docs.cloud.google.com/vertex-ai/docs/generative-ai/learn/models#foundation_models).
You then use that model with the
[`AI.GENERATE_EMBEDDING` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-generate-embedding)
to create video embeddings by using data from a
BigQuery
[object table](https://docs.cloud.google.com/bigquery/docs/object-table-introduction).

## Required roles

To create a remote model and generate embeddings, you need the
following Identity and Access Management (IAM) roles:

- Create and use BigQuery datasets, tables, and models: BigQuery Data Editor (`roles/bigquery.dataEditor`) on your project.
- Create, delegate, and use BigQuery connections:
  BigQuery Connections Admin (`roles/bigquery.connectionsAdmin`) on your
  project.

  If you don't have a [default connection](https://docs.cloud.google.com/bigquery/docs/default-connections)
  configured, you can create and set one as part of running the
  `CREATE MODEL` statement. To do so, you must have BigQuery Admin
  (`roles/bigquery.admin`) on your project. For more information, see
  [Configure the default connection](https://docs.cloud.google.com/bigquery/docs/default-connections#configure_the_default_connection).
- Grant permissions to the connection's service account: Project IAM Admin
  (`roles/resourcemanager.projectIamAdmin`) on the project that contains the
  Vertex AI endpoint. This is the current project for remote models
  that you create by specifying the model name as an endpoint. This is the
  project identified in the URL for remote models that you create by
  specifying a URL as an endpoint.

  If you use the remote model to analyze unstructured data from an object
  table, and the Cloud Storage bucket that you use in the object table is
  in a different project than your Vertex AI endpoint, you must
  also have Storage Admin (`roles/storage.admin`) on the
  Cloud Storage bucket used by the object table.
- Create BigQuery jobs: BigQuery Job User
  (`roles/bigquery.jobUser`) on your project.

These predefined roles contain the permissions required to perform the tasks in
this document. To see the exact permissions that are required, expand the
**Required permissions** section:

#### Required permissions

- Create a dataset: `bigquery.datasets.create`
- Create, delegate, and use a connection: `bigquery.connections.*`
- Set service account permissions: `resourcemanager.projects.getIamPolicy` and `resourcemanager.projects.setIamPolicy`
- Create an object table: `bigquery.tables.create` and `bigquery.tables.update`
- Create a model and run inference:
  - `bigquery.jobs.create`
  - `bigquery.models.create`
  - `bigquery.models.getData`
  - `bigquery.models.updateData`
  - `bigquery.models.updateMetadata`

You might also be able to get these permissions with
[custom roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles) or other
[predefined roles](https://docs.cloud.google.com/iam/docs/roles-overview#predefined).

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


   Enable the BigQuery, BigQuery Connection, Cloud Storage, and Vertex AI APIs.


   **Roles required to enable APIs**


   To enable APIs, you need the Service Usage Admin IAM
   role (`roles/serviceusage.serviceUsageAdmin`), which
   contains the `serviceusage.services.enable` permission. [Learn how to grant
   roles](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).

   [Enable the APIs](https://console.cloud.google.com/flows/enableapi?apiid=bigquery.googleapis.com,bigqueryconnection.googleapis.com,storage.googleapis.com,aiplatform.googleapis.com)

## Create a dataset

Create a BigQuery dataset to contain your resources:

### Console

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

### bq

1. To create a new dataset, use the
   [`bq mk`](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#mk-dataset) command
   with the `--location` flag:

   ```
   bq --location=LOCATION mk -d DATASET_ID
   ```

   Replace the following:
   - `LOCATION`: the dataset's [location](https://docs.cloud.google.com/bigquery/docs/locations).
   - `DATASET_ID` is the ID of the dataset that you're creating.
2. Confirm that the dataset was created:

   ```bash
   bq ls
   ```

## Create a connection

Create a
[Cloud resource connection](https://docs.cloud.google.com/bigquery/docs/create-cloud-resource-connection)
and get the connection's service account. Create the connection in
the same [location](https://docs.cloud.google.com/bigquery/docs/locations) as the dataset you created in the
previous step.

You can skip this step if you either have a default connection configured, or
you have the BigQuery Admin role.
Select one of the following options:

<br />

### Console

1. Go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click
   **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)

   If you don't see the left pane, click
   **Expand left pane** to open the pane.
3. In the **Explorer** pane, expand your project name, and then click
   **Connections**.

4. On the **Connections** page, click **Create connection**.

5. For **Connection type** , choose **Vertex AI remote models, remote
   functions, BigLake and Spanner (Cloud Resource)**.

6. In the **Connection ID** field, enter a name for your connection.

7. For **Location type**, select a location for your connection. The
   connection should be colocated with your other resources such as
   datasets.

8. Click **Create connection**.

9. Click **Go to connection**.

10. In the **Connection info** pane, copy the service account ID for use in
    a later step.

### SQL

Use the [`CREATE CONNECTION` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_connection_statement):


1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, enter the following statement:

   ```googlesql
   CREATE CONNECTION [IF NOT EXISTS] `CONNECTION_NAME`
   OPTIONS (
     connection_type = "CLOUD_RESOURCE",
     friendly_name = "FRIENDLY_NAME",
     description = "DESCRIPTION"
     );
   ```


   Replace the following:
   - `CONNECTION_NAME`: the name of the connection in either the `PROJECT_ID.LOCATION.CONNECTION_ID`, `LOCATION.CONNECTION_ID`, or `CONNECTION_ID` format. If the project or location are omitted, then they are inferred from the project and location where the statement is run.
   - `FRIENDLY_NAME` (optional): a descriptive name for the connection.
   - `DESCRIPTION` (optional): a description of the connection.

   <br />

3. Click **Run**.

   <br />

For more information about how to run queries, see [Run an interactive query](https://docs.cloud.google.com/bigquery/docs/running-queries#queries).

### bq

1. In a command-line environment, create a connection:

   ```bash
   bq mk --connection --location=REGION --project_id=PROJECT_ID \
       --connection_type=CLOUD_RESOURCE CONNECTION_ID
   ```

   The `--project_id` parameter overrides the default project.

   Replace the following:
   - `REGION`: your [connection region](https://docs.cloud.google.com/bigquery/docs/locations#supported_locations)
   - `PROJECT_ID`: your Google Cloud project ID
   - `CONNECTION_ID`: an ID for your connection

   When you create a connection resource, BigQuery creates a
   unique system service account and associates it with the connection.

   **Troubleshooting** : If you get the following connection error,
   [update the Google Cloud SDK](https://docs.cloud.google.com/sdk/docs/quickstart):

   ```
   Flags parsing error: flag --connection_type=CLOUD_RESOURCE: value should be one of...
   ```
2. Retrieve and copy the service account ID for use in a later
   step:

   ```bash
   bq show --connection PROJECT_ID.REGION.CONNECTION_ID
   ```

   The output is similar to the following:

   ```
   name                          properties
   1234.REGION.CONNECTION_ID     {"serviceAccountId": "connection-1234-9u56h9@gcp-sa-bigquery-condel.iam.gserviceaccount.com"}
   ```

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

    import google.api_core.exceptions
    from google.cloud import https://docs.cloud.google.com/python/docs/reference/bigqueryconnection/latest

    client = https://docs.cloud.google.com/python/docs/reference/bigqueryconnection/latest.https://docs.cloud.google.com/python/docs/reference/bigqueryconnection/latest/google.cloud.bigquery_connection_v1.services.connection_service.ConnectionServiceClient.html()


    def create_connection(
        project_id: str,
        location: str,
        connection_id: str,
    ):
        """Creates a BigQuery connection to a Cloud Resource.

        Cloud Resource connection creates a service account which can then be
        granted access to other Google Cloud resources for federated queries.

        Args:
            project_id: The Google Cloud project ID.
            location: The location of the connection (for example, "us-central1").
            connection_id: The ID of the connection to create.
        """

        parent = client.https://docs.cloud.google.com/python/docs/reference/bigqueryconnection/latest/google.cloud.bigquery_connection_v1.services.connection_service.ConnectionServiceClient.html#google_cloud_bigquery_connection_v1_services_connection_service_ConnectionServiceClient_common_location_path(project_id, location)

        connection = https://docs.cloud.google.com/python/docs/reference/bigqueryconnection/latest.https://docs.cloud.google.com/python/docs/reference/bigqueryconnection/latest/google.cloud.bigquery_connection_v1.types.Connection.html(
            friendly_name="Example Connection",
            description="A sample connection for a Cloud Resource.",
            cloud_resource=https://docs.cloud.google.com/python/docs/reference/bigqueryconnection/latest.https://docs.cloud.google.com/python/docs/reference/bigqueryconnection/latest/google.cloud.bigquery_connection_v1.types.CloudResourceProperties.html(),
        )

        try:
            created_connection = client.https://docs.cloud.google.com/python/docs/reference/bigqueryconnection/latest/google.cloud.bigquery_connection_v1.services.connection_service.ConnectionServiceClient.html#google_cloud_bigquery_connection_v1_services_connection_service_ConnectionServiceClient_create_connection(
                parent=parent, connection_id=connection_id, connection=connection
            )
            print(f"Successfully created connection: {created_connection.name}")
            print(f"Friendly name: {created_connection.friendly_name}")
            print(
                f"Service Account: {created_connection.cloud_resource.service_account_id}"
            )

        except google.api_core.exceptions.AlreadyExists:
            print(f"Connection with ID '{connection_id}' already exists.")
            print("Please use a different connection ID.")
        except Exception as e:
            print(f"An unexpected error occurred while creating the connection: {e}")

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

    const {ConnectionServiceClient} =
      require('https://docs.cloud.google.com/nodejs/docs/reference/bigquery-connection/latest/overview.html').v1;
    const {status} = require('@grpc/grpc-js');

    const client = new https://docs.cloud.google.com/nodejs/docs/reference/bigquery-connection/latest/overview.html();

    /**
     * Creates a new BigQuery connection to a Cloud Resource.
     *
     * A Cloud Resource connection creates a service account that can be granted access
     * to other Google Cloud resources.
     *
     * @param {string} projectId The Google Cloud project ID. for example, 'example-project-id'
     * @param {string} location The location of the project to create the connection in. for example, 'us-central1'
     * @param {string} connectionId The ID of the connection to create. for example, 'example-connection-id'
     */
    async function createConnection(projectId, location, connectionId) {
      const parent = client.locationPath(projectId, location);

      const connection = {
        friendlyName: 'Example Connection',
        description: 'A sample connection for a Cloud Resource',
        // The service account for this cloudResource will be created by the API.
        // Its ID will be available in the response.
        cloudResource: {},
      };

      const request = {
        parent,
        connectionId,
        connection,
      };

      try {
        const [response] = await client.createConnection(request);

        console.log(`Successfully created connection: ${response.name}`);
        console.log(`Friendly name: ${response.friendlyName}`);

        console.log(`Service Account: ${response.cloudResource.serviceAccountId}`);
      } catch (err) {
        if (err.code === status.ALREADY_EXISTS) {
          console.log(`Connection '${connectionId}' already exists.`);
        } else {
          console.error(`Error creating connection: ${err.message}`);
        }
      }
    }

### Terraform

Use the
[`google_bigquery_connection`](https://registry.terraform.io/providers/hashicorp/google/latest/docs/resources/bigquery_connection)
resource.

> [!NOTE]
> **Note:** To create BigQuery objects using Terraform, you must enable the [Cloud Resource Manager API](https://docs.cloud.google.com/resource-manager/reference/rest).

To authenticate to BigQuery, set up Application Default
Credentials. For more information, see
[Set up authentication for client libraries](https://docs.cloud.google.com/bigquery/docs/authentication#client-libs).

The following example creates a Cloud resource connection named
`my_cloud_resource_connection` in the `US` region:


    # This queries the provider for project information.
    data "google_project" "default" {}

    # This creates a cloud resource connection in the US region named my_cloud_resource_connection.
    # Note: The cloud resource nested object has only one output field - serviceAccountId.
    resource "google_bigquery_connection" "default" {
      connection_id = "my_cloud_resource_connection"
      project       = data.google_project.default.project_id
      location      = "US"
      cloud_resource {}
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

### Give the service account access

Grant the connection's service account the Vertex AI User and
Storage Object Viewer roles.

If you plan to specify the endpoint as a URL when you create the remote model,
for example `endpoint = 'https://us-central1-aiplatform.googleapis.com/v1/projects/myproject/locations/us-central1/publishers/google/models/gemini-2.0-flash'`,
grant this role in the same project you specify in the URL.

If you plan to specify the endpoint by using the model name when you create
the remote model, for example `endpoint = 'gemini-2.0-flash'`, grant this role
in the same project where you plan to create the remote model.

Granting the role in a different project results in the error
`bqcx-1234567890-wxyz@gcp-sa-bigquery-condel.iam.gserviceaccount.com does not have the permission to access resource`.

To grant these roles, follow these steps:

### Console

1. Go to the **IAM \& Admin** page.

   [Go to IAM \& Admin](https://console.cloud.google.com/project/_/iam-admin)
2. Click **Add**.

   The **Add principals** dialog opens.
3. In the **New principals** field, enter the service account ID that you
   copied earlier.

4. In the **Select a role** field, select **Vertex AI** , and then select
   **Vertex AI User**.

5. Click **Add another role**.

6. In the **Select a role** field, choose **Cloud Storage** , and then
   select **Storage Object Viewer**.

7. Click **Save**.

### gcloud

Use the
[`gcloud projects add-iam-policy-binding` command](https://docs.cloud.google.com/sdk/gcloud/reference/projects/add-iam-policy-binding).

```
gcloud projects add-iam-policy-binding 'PROJECT_NUMBER' --member='serviceAccount:MEMBER' --role='roles/aiplatform.user' --condition=None
gcloud projects add-iam-policy-binding 'PROJECT_NUMBER' --member='serviceAccount:MEMBER' --role='roles/storage.objectViewer' --condition=None
```

Replace the following:

- `PROJECT_NUMBER`: the project number of the project in which to grant the role.
- `MEMBER`: the service account ID that you copied earlier.

<br />

## Create an object table

To analyze videos without moving them from Cloud Storage, create an
object table.

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
   - `DATASET_ID`: the ID of the [dataset that you created](https://docs.cloud.google.com/bigquery/docs/generate-video-embedding#create_a_dataset).
   - `TABLE_NAME`: the name of the object table.
   - `REGION`: the [region or multi-region](https://docs.cloud.google.com/bigquery/docs/locations#supported_locations) that contains the connection.
   - `CONNECTION_ID`: the ID of the [connection that you created](https://docs.cloud.google.com/bigquery/docs/generate-video-embedding#create_a_connection).

     When you [view the connection details](https://docs.cloud.google.com/bigquery/docs/working-with-connections#view-connections)
     in the Google Cloud console, this is the value in the last section of
     the fully qualified connection ID that is shown in
     **Connection ID** , for example
     `projects/myproject/locations/connection_location/connections/*myconnection*`.

     To use a [default connection](https://docs.cloud.google.com/bigquery/docs/default-connections), specify `DEFAULT` instead of the
     connection string containing
     `PROJECT_ID.REGION.CONNECTION_ID`.
   - `BUCKET_PATH`: the path to the Cloud Storage bucket that contains the videos, in the format `['gs://bucket_name/[folder_name/]*']`.

     The Cloud Storage bucket that you use should be in the
     same project where you plan to create the model and call the
     `AI.GENERATE_EMBEDDING` function. If you want to call the
     `AI.GENERATE_EMBEDDING` function in a different project than
     the one that contains the Cloud Storage bucket used by the
     object table, you must
     [grant the Storage Admin role at the bucket level](https://docs.cloud.google.com/storage/docs/access-control/using-iam-permissions#bucket-add)
     to the
     `service-A@gcp-sa-aiplatform.iam.gserviceaccount.com`
     service account.
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

- `BUCKET_PATH`: the path to the Cloud Storage bucket that contains the videos, in the format `['gs://bucket_name/[folder_name/]*']`.

  The Cloud Storage bucket that you use should be in the
  same project where you plan to create the model and call the
  `AI.GENERATE_EMBEDDING` function. If you want to call the
  `AI.GENERATE_EMBEDDING` function in a different project than
  the one that contains the Cloud Storage bucket used by the
  object table, you must
  [grant the Storage Admin role at the bucket level](https://docs.cloud.google.com/storage/docs/access-control/using-iam-permissions#bucket-add)
  to the
  `service-A@gcp-sa-aiplatform.iam.gserviceaccount.com`
  service account.
- `REGION`: the [region or multi-region](https://docs.cloud.google.com/bigquery/docs/locations#supported_locations) that contains the connection.
- `CONNECTION_ID`: the ID of the [connection that you created](https://docs.cloud.google.com/bigquery/docs/generate-video-embedding#create_a_connection).

  When you [view the connection details](https://docs.cloud.google.com/bigquery/docs/working-with-connections#view-connections)
  in the Google Cloud console, this is the value in the last section of
  the fully qualified connection ID that is shown in
  **Connection ID** , for example
  `projects/myproject/locations/connection_location/connections/*myconnection*`.
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
- `PROJECT_ID`: your project ID.
- `DATASET_ID`: the ID of the [dataset that you created](https://docs.cloud.google.com/bigquery/docs/generate-video-embedding#create_a_dataset).
- `TABLE_NAME`: the name of the object table.

<br />

## Create a model

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. Using the SQL editor, create a
   [remote model](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model):

   ```googlesql
   CREATE OR REPLACE MODEL `PROJECT_ID.DATASET_ID.MODEL_NAME`
   REMOTE WITH CONNECTION {DEFAULT | `PROJECT_ID.REGION.CONNECTION_ID`}
   OPTIONS (ENDPOINT = 'ENDPOINT');
   ```

   Replace the following:
   - `PROJECT_ID`: your project ID.
   - `DATASET_ID`: the ID of the [dataset that you created](https://docs.cloud.google.com/bigquery/docs/generate-video-embedding#create_a_dataset).
   - `MODEL_NAME`: the name of the model.
   - `REGION`: the [region or multi-region](https://docs.cloud.google.com/bigquery/docs/locations#supported_locations) that contains the connection.
   - `CONNECTION_ID`: the ID of the [connection that you created](https://docs.cloud.google.com/bigquery/docs/generate-video-embedding#create_a_connection).

     When you [view the connection details](https://docs.cloud.google.com/bigquery/docs/working-with-connections#view-connections)
     in the Google Cloud console, this is the value in the last section of
     the fully qualified connection ID that is shown in
     **Connection ID** , for example
     `projects/myproject/locations/connection_location/connections/*myconnection*`.
   - `ENDPOINT`: the [embedding model](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model#endpoint) to use, in this case `multimodalembedding@001`.

     If you specify a URL as the endpoint when you create the remote model,
     for example
     `endpoint = 'https://us-central1-aiplatform.googleapis.com/v1/projects/myproject/locations/us-central1/publishers/google/models/multimodalembedding@001'`,
     make sure that the project that you specify in the URL is the project in
     which you have granted the Vertex AI user role to the connection's service account.

     The `multimodalembedding@001` model
     must be available in the location where you are creating the remote
     model. For more information, see
     [Locations](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model#locations).

   <br />

## Generate video embeddings

Generate video embeddings with the
[`AI.GENERATE_EMBEDDING` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-generate-embedding)
by using video data from an object table:

```googlesql
SELECT *
FROM AI.GENERATE_EMBEDDING(
  MODEL `PROJECT_ID.DATASET_ID.MODEL_NAME`,
  TABLE PROJECT_ID.DATASET_ID.TABLE_NAME,
  STRUCT(
    START_SECOND AS start_second,
    END_SECOND AS end_second,
    INTERVAL_SECONDS AS interval_seconds)
);
```

Replace the following:

- `PROJECT_ID`: your project ID.
- `DATASET_ID`: the ID of the dataset that contains the model.
- `MODEL_NAME`: the name of the remote model over a `multimodalembedding@001` model.
- `TABLE_NAME`: the name of the object table that contains the videos to embed.
- `START_SECOND`: a `FLOAT64` value that specifies the second in the video at which to start the embedding. The default value is `0`. This value must be positive and less than the `end_second` value.
- `END_SECOND`: a `FLOAT64` value that specifies the second in the video at which to end the embedding. The default value is `120`. This value must be positive and greater than the `start_second` value.
- `INTERVAL_SECONDS`: a `FLOAT64` value that specifies the interval to use when creating embeddings. For example, if you set `start_second = 0`, `end_second = 120`, and `interval_seconds = 10`, then the video is split into twelve 10 second segments (`[0, 10), [10, 20), [20, 30)...`) and embeddings are generated for each segment. This value must be greater than `4` and less than `120`. The default value is `16`.

<br />

## Example

The following example shows how to create embeddings for the videos in
the `videos` object table. Embeddings are created for each 5 second interval
between the 10 second and 40 second marks in each video.

```googlesql
SELECT *
FROM
  AI.GENERATE_EMBEDDING(
    MODEL `mydataset.embedding_model`,
    TABLE `mydataset.videos`,
    STRUCT(
    10 AS start_second,
    40 AS end_second,
    5 AS interval_seconds)
  );
```

## What's next

- Learn how to [use text and image embeddings to perform a text-to-image semantic search](https://docs.cloud.google.com/bigquery/docs/generate-multimodal-embeddings).
- Learn how to [use text embeddings for semantic search and retrieval-augmented generation (RAG)](https://docs.cloud.google.com/bigquery/docs/vector-index-text-search-tutorial).