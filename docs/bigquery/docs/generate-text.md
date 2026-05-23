# Generate text by using the AI.GENERATE_TEXT function

This document shows you how to create a BigQuery ML
[remote model](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model)
that represents a Vertex AI model, and then use that remote model
with the
[`AI.GENERATE_TEXT` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-generate-text)
to generate text.

The following types of remote models are supported:

- [Remote models](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model) over any of the [generally available](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/models#generally_available_models) or [preview](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/models#preview_models) Gemini models.
- [Remote models](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model) over [Anthropic Claude models](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/partner-models/use-claude).
- [Remote models](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model) over [Llama models](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/partner-models/llama)
- [Remote models](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model) over [Mistral AI models](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/partner-models/mistral)
- [Remote models](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model-open) over [supported open models](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model-open#supported_open_models).

Depending on the Vertex AI model that you choose, you can
generate text based on unstructured data input from
[object tables](https://docs.cloud.google.com/bigquery/docs/object-table-introduction) or text input from
[standard tables](https://docs.cloud.google.com/bigquery/docs/tables-intro#standard-tables).

## Required roles

To create a remote model and generate text, you need the
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


   Enable the BigQuery, BigQuery Connection, and Vertex AI APIs.


   **Roles required to enable APIs**


   To enable APIs, you need the Service Usage Admin IAM
   role (`roles/serviceusage.serviceUsageAdmin`), which
   contains the `serviceusage.services.enable` permission. [Learn how to grant
   roles](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).

   [Enable the APIs](https://console.cloud.google.com/flows/enableapi?apiid=bigquery.googleapis.com,bigqueryconnection.googleapis.com,aiplatform.googleapis.com)

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
for the remote model to use, and get the connection's service account.
Create the connection in the same [location](https://docs.cloud.google.com/bigquery/docs/locations) as the
dataset that you created in the previous step.

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

### Grant a role to the remote model connection's service account

You must grant the Vertex AI User role to the service account of the connection
that the remote model uses.

If you plan to specify the remote model's endpoint as a URL,
for example `endpoint = 'https://us-central1-aiplatform.googleapis.com/v1/projects/myproject/locations/us-central1/publishers/google/models/gemini-2.0-flash'`,
grant this role in the same project you specify in the URL.

If you plan to specify the remote model's endpoint by using the model name,
for example `endpoint = 'gemini-2.0-flash'`, grant this role
in the same project where you plan to create the remote model.

Granting the role in a different project results in the error
`bqcx-1234567890-wxyz@gcp-sa-bigquery-condel.iam.gserviceaccount.com does not have the permission to access resource`.

To grant the Vertex AI User role, follow these steps:

### Console

1. Go to the **IAM \& Admin** page.

   [Go to IAM \& Admin](https://console.cloud.google.com/project/_/iam-admin)
2. Click **Add**.

   The **Add principals** dialog opens.
3. In the **New principals** field, enter the service account ID that you
   copied earlier.

4. In the **Select a role** field, select **Vertex AI** , and then select
   **Vertex AI User**.

5. Click **Save**.

### gcloud

Use the
[`gcloud projects add-iam-policy-binding` command](https://docs.cloud.google.com/sdk/gcloud/reference/projects/add-iam-policy-binding).

```
gcloud projects add-iam-policy-binding 'PROJECT_NUMBER' --member='serviceAccount:MEMBER' --role='roles/aiplatform.user' --condition=None
```

Replace the following:

- `PROJECT_NUMBER`: your project number
- `MEMBER`: the service account ID that you copied earlier

<br />

## Grant a role to the object table connection's service account

If you are using the remote model to generate text from object table data, grant
the object table connection's service account the Vertex AI User role
in the same project where you plan to create the remote model.
Otherwise, you can skip this step.

To find the service account for the object table connection, follow these
steps:

1. Go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)

   If you don't see the left pane, click **Expand left pane** to open the pane.
3. In the **Explorer** pane, click **Datasets**, and then select a dataset that
   contains the object table.

4. Click **Overview \> Tables**, and then select the object table.

5. In the editor pane, click the **Details** tab.

6. Note the connection name in the **Connection ID** field.

7. In the **Explorer** pane, click **Connections**.

8. Select the connection that matches the one from the object table's
   **Connection ID** field.

9. Copy the value in the **Service account ID** field.

To grant the role, follow these steps:

### Console

1. Go to the **IAM \& Admin** page.

   [Go to IAM \& Admin](https://console.cloud.google.com/project/_/iam-admin)
2. Click **Add**.

   The **Add principals** dialog opens.
3. In the **New principals** field, enter the service account ID that you
   copied earlier.

4. In the **Select a role** field, select **Vertex AI** , and then select
   **Vertex AI User**.

5. Click **Save**.

### gcloud

Use the
[`gcloud projects add-iam-policy-binding` command](https://docs.cloud.google.com/sdk/gcloud/reference/projects/add-iam-policy-binding).

```
gcloud projects add-iam-policy-binding 'PROJECT_NUMBER' --member='serviceAccount:MEMBER' --role='roles/aiplatform.user' --condition=None
```

Replace the following:

- `PROJECT_NUMBER`: your project number
- `MEMBER`: the service account ID that you copied earlier

<br />

## Enable a partner model

This step is only required if you want to use Anthropic Claude, Llama, or
Mistral AI models.

1. In the Google Cloud console, go to the Vertex AI **Model Garden**
   page.

   [Go to Model Garden](https://console.cloud.google.com/vertex-ai/model-garden)
2. Search or browse for the partner model that you want to use.

3. Click the model card.

4. On the model page, click **Enable**.

5. Fill out the requested enablement information, and then click **Next**.

6. In the **Terms and conditions** section, select the checkbox.

7. Click **Agree** to agree to the terms and conditions and enable the model.

## Choose an open model deployment method

If you are creating a remote model over a
[supported open model](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model-open#supported_open_models),
you can automatically deploy the open model at the same time that
you create the remote model by specifying the Vertex AI
Model Garden or Hugging Face model ID in the `CREATE MODEL` statement.
Alternatively, you can manually deploy the open model first, and then use that
open model with the remote model by specifying the model
endpoint in the `CREATE MODEL` statement. For more information, see
[Deploy open models](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model-open#deploy_open_models).

## Create a BigQuery ML remote model

Create a remote model:

### Vertex AI models

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. Using the SQL editor, create a
   [remote model](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model):

   ```googlesql
   CREATE OR REPLACE MODEL
   `PROJECT_ID.DATASET_ID.MODEL_NAME`
   REMOTE WITH CONNECTION {DEFAULT | `PROJECT_ID.REGION.CONNECTION_ID`}
   OPTIONS (ENDPOINT = 'ENDPOINT');
   ```

   Replace the following:
   - `PROJECT_ID`: your project ID.
   - `DATASET_ID`: the ID of the dataset to contain the model. This dataset must be in the same [location](https://docs.cloud.google.com/bigquery/docs/locations) as the connection that you are using.
   - `MODEL_NAME`: the name of the model.
   - `REGION`: the region used by the connection.
   - `CONNECTION_ID`: the ID of your BigQuery connection.

     You can get this value by [viewing the connection details](https://docs.cloud.google.com/bigquery/docs/working-with-connections#view-connections) in the Google Cloud console
     and copying the value in the last section of the fully qualified
     connection ID that is shown in **Connection ID** . For example,
     `projects/myproject/locations/connection_location/connections/*myconnection*`.
   - 
   - `ENDPOINT`: the endpoint of the Vertex AI model to use.

     For pre-trained Vertex AI models, Claude models,
     and Mistral AI models, specify the name of the model. For some of
     these models, you can specify a particular
     [version](https://docs.cloud.google.com/vertex-ai/docs/generative-ai/learn/model-versioning)
     of the model as part of the name. For supported Gemini
     models, you can specify the
     [global endpoint](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model#global-endpoint)
     to improve availability.

     For Llama models, specify an
     [OpenAI API](https://platform.openai.com/docs/api-reference/introduction)
     endpoint in the format `openapi/<publisher_name>/<model_name>`.
     For example, `openapi/meta/llama-3.1-405b-instruct-maas`.


     For information about supported model names and versions, see
     [`ENDPOINT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model#endpoint).

     The Vertex AI model that you specify
     must be available in the location where you are creating the remote
     model. For more information, see
     [Locations](https://docs.cloud.google.com/bigquery/docs/locations#locations-for-remote-models).
   - 

   <br />

### New open models

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. Using the SQL editor, create a
   [remote model](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model-open):

   ```googlesql
   CREATE OR REPLACE MODEL
   `PROJECT_ID.DATASET_ID.MODEL_NAME`
   REMOTE WITH CONNECTION {DEFAULT | `PROJECT_ID.REGION.CONNECTION_ID`}
   OPTIONS (
     {HUGGING_FACE_MODEL_ID = 'HUGGING_FACE_MODEL_ID' |
        MODEL_GARDEN_MODEL_NAME = 'MODEL_GARDEN_MODEL_NAME'}
     [, HUGGING_FACE_TOKEN = 'HUGGING_FACE_TOKEN' ]
     [, MACHINE_TYPE = 'MACHINE_TYPE' ]
     [, MIN_REPLICA_COUNT = MIN_REPLICA_COUNT ]
     [, MAX_REPLICA_COUNT = MAX_REPLICA_COUNT ]
     [, RESERVATION_AFFINITY_TYPE = {'NO_RESERVATION' | 'ANY_RESERVATION' | 'SPECIFIC_RESERVATION'} ]
     [, RESERVATION_AFFINITY_KEY = 'compute.googleapis.com/reservation-name' ]
     [, RESERVATION_AFFINITY_VALUES = RESERVATION_AFFINITY_VALUES ]
     [, ENDPOINT_IDLE_TTL = ENDPOINT_IDLE_TTL ]
   );
   ```

   Replace the following:
   - `PROJECT_ID`: your project ID.
   - `DATASET_ID`: the ID of the dataset to contain the model. This dataset must be in the same [location](https://docs.cloud.google.com/bigquery/docs/locations) as the connection that you are using.
   - `MODEL_NAME`: the name of the model.
   - `REGION`: the region used by the connection.
   - `CONNECTION_ID`: the ID of your BigQuery connection.

     You can get this value by [viewing the connection details](https://docs.cloud.google.com/bigquery/docs/working-with-connections#view-connections) in the Google Cloud console
     and copying the value in the last section of the fully qualified
     connection ID that is shown in **Connection ID** . For example,
     `projects/myproject/locations/connection_location/connections/*myconnection*`.
   - 
   - `HUGGING_FACE_MODEL_ID`: a `STRING` value that specifies the model ID for a [supported Hugging Face model](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model-open#hugging-face-models), in the format `provider_name`/`model_name`. For example, `deepseek-ai/DeepSeek-R1`. You can get the model ID by clicking the model name in the Hugging Face Model Hub and then copying the model ID from the top of the model card.
   - `MODEL_GARDEN_MODEL_NAME`: a `STRING` value that specifies the model ID and model version of a [supported Vertex AI Model Garden model](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model-open#model-garden-models), in the format `publishers/publisher`/models/`model_name`@`model_version`. For example, `publishers/openai/models/gpt-oss@gpt-oss-120b`. You can get the model ID by clicking the model card in the Vertex AI Model Garden and then copying the model ID from the **Model ID** field. You can get the default model version by copying it from the **Version** field on the model card. To see other model versions that you can use, click **Deploy model** and then click the **Resource ID** field.
   - `HUGGING_FACE_TOKEN`: a `STRING` value that specifies the Hugging Face [User Access Token](https://huggingface.co/docs/hub/en/security-tokens) to use. You can only specify a value for this option if you also specify a value for the `HUGGING_FACE_MODEL_ID` option.

     The token must have the `read` role at minimum but
     tokens with a broader scope are also acceptable. This option is
     required when the model identified by the
     `HUGGING_FACE_MODEL_ID` value is a Hugging Face
     [gated](https://huggingface.co/docs/hub/en/models-gated) or private
     model.

     Some gated models require explicit agreement to their terms of
     service before access is granted. To agree to these terms, follow
     these steps:
     1. Navigate to the model's page on the Hugging Face website.
     2. Locate and review the model's terms of service. A link to the service agreement is typically found on the model card.
     3. Accept the terms as prompted on the page.
   - `MACHINE_TYPE`: a `STRING` value that specifies the machine type to use when deploying the model to Vertex AI. For information about supported machine types, see [Machine types](https://docs.cloud.google.com/vertex-ai/docs/predictions/configure-compute#machine-types). If you don't specify a value for the `MACHINE_TYPE` option, the Vertex AI Model Garden default machine type for the model is used.
   - `MIN_REPLICA_COUNT`: an `INT64` value that specifies the minimum number of machine replicas used when deploying the model on a Vertex AI endpoint. The service increases or decreases the replica count as required by the inference load on the endpoint. The number of replicas used is never lower than the `MIN_REPLICA_COUNT` value and never higher than the `MAX_REPLICA_COUNT` value. The `MIN_REPLICA_COUNT` value must be in the range `[1, 4096]`. The default value is `1`.
   - `MAX_REPLICA_COUNT`: an `INT64` value that specifies the maximum number of machine replicas used when deploying the model on a Vertex AI endpoint. The service increases or decreases the replica count as required by the inference load on the endpoint. The number of replicas used is never lower than the `MIN_REPLICA_COUNT` value and never higher than the `MAX_REPLICA_COUNT` value. The `MAX_REPLICA_COUNT` value must be in the range `[1, 4096]`. The default value is the `MIN_REPLICA_COUNT` value.
   - `RESERVATION_AFFINITY_TYPE`: determines whether the deployed model uses [Compute Engine reservations](https://docs.cloud.google.com/compute/docs/instances/reservations-overview) to provide assured virtual machine (VM) availability when serving predictions, and specifies whether the model uses VMs from all available reservations or just one specific reservation. For more information, see [Compute Engine reservation affinity](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model-open#reservation-affinity).


     You can only use Compute Engine reservations that are shared
     with Vertex AI. For more information, see
     [Allow a reservation to be consumed](https://docs.cloud.google.com/vertex-ai/docs/predictions/use-reservations#allow-consumption).

     Supported values are as follows:
     - `NO_RESERVATION`: no reservation is consumed when your model is deployed to a Vertex AI endpoint. Specifying `NO_RESERVATION` has the same effect as not specifying a reservation affinity.
     - `ANY_RESERVATION`: the Vertex AI model deployment consumes virtual machines (VMs) from Compute Engine reservations that are in the current project or that are [shared with the project](https://docs.cloud.google.com/compute/docs/instances/reservations-overview#how-shared-reservations-work), and that are [configured for automatic consumption](https://docs.cloud.google.com/compute/docs/instances/reservations-consume#consuming_instances_from_any_matching_reservation). Only VMs that meet the following qualifications are used:
       - They use the machine type specified by the `MACHINE_TYPE` value.
       - If the BigQuery dataset in which you are creating the remote model is a single region, the reservation must be in the same region. If the dataset is in the `US` multiregion, the reservation must be in the `us-central1` region. If the dataset is in the `EU` multiregion, the reservation must be in the `europe-west4` region.

       If there isn't enough
       capacity in the available reservations, or if no suitable reservations
       are found, the system provisions on-demand Compute Engine
       VMs to meet the resource requirements.
     - `SPECIFIC_RESERVATION`: the Vertex AI model deployment consumes VMs only from the reservation that you specify in the `RESERVATION_AFFINITY_VALUES` value. This reservation must be [configured for specifically targeted consumption](https://docs.cloud.google.com/compute/docs/instances/reservations-consume#consuming_instances_from_a_specific_reservation). Deployment fails if the specified reservation doesn't have sufficient capacity.
   - `RESERVATION_AFFINITY_KEY`: the string `compute.googleapis.com/reservation-name`. You must specify this option when the `RESERVATION_AFFINITY_TYPE` value is `SPECIFIC_RESERVATION`.
   - `RESERVATION_AFFINITY_VALUES`: an `ARRAY<STRING>` value that specifies the full resource name of the Compute Engine reservation, in the following format:  

     `projects/myproject/zones/reservation_zone/reservations/reservation_name`


     For example,
     `RESERVATION_AFFINITY_values = ['projects/myProject/zones/us-central1-a/reservations/myReservationName']`.

     You can get the reservation name and zone from the
     **Reservations** page of the Google Cloud console. For more
     information, see
     [View reservations](https://docs.cloud.google.com/compute/docs/instances/reservations-view#view-reservations).

     You must specify this option
     when the `RESERVATION_AFFINITY_TYPE` value is
     `SPECIFIC_RESERVATION`.
   - `ENDPOINT_IDLE_TTL`: an `INTERVAL` value that specifies the duration of inactivity after which the open model is automatically undeployed from the Vertex AI endpoint.

     To enable automatic undeployment, specify an
     [interval literal](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/lexical#interval_literals)
     value between 390 minutes (6.5 hours) and 7 days. For example, specify
     `INTERVAL 8 HOUR` to have the model undeployed after 8 hours of idleness.
     The default value is 390 minutes (6.5 hours).

     Model inactivity is defined as the amount of time that has passed
     since any of the following operations were performed on the model:
     - Running the [`CREATE MODEL` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model-open).
     - Running the [`ALTER MODEL` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-alter-model) with the `DEPLOY_MODEL` argument set to `TRUE`.
     - Sending an inference request to the model endpoint. For example, by running the [`AI.GENERATE_EMBEDDING`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-generate-embedding) or [`AI.GENERATE_TEXT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-generate-text) function.

     Each of these operations resets the inactivity timer to zero. The reset is
     triggered at the start of the BigQuery job that performs the
     operation.


     After the model is undeployed, inference requests sent to the model return
     an error. The BigQuery model object remains unchanged,
     including model metadata. To use the model for inference again, you must
     redeploy it by running the `ALTER MODEL` statement on the model and
     setting the `DEPLOY_MODEL` option to `TRUE`.
   - 

   <br />

### Deployed open models

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. Using the SQL editor, create a
   [remote model](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model-open):

   ```googlesql
   CREATE OR REPLACE MODEL
   `PROJECT_ID.DATASET_ID.MODEL_NAME`
   REMOTE WITH CONNECTION {DEFAULT | `PROJECT_ID.REGION.CONNECTION_ID`}
   OPTIONS (
     ENDPOINT = 'https://ENDPOINT_REGION-aiplatform.googleapis.com/v1/projects/ENDPOINT_PROJECT_ID/locations/ENDPOINT_REGION/endpoints/ENDPOINT_ID'
   );
   ```

   Replace the following:
   - `PROJECT_ID`: your project ID.
   - `DATASET_ID`: the ID of the dataset to contain the model. This dataset must be in the same [location](https://docs.cloud.google.com/bigquery/docs/locations) as the connection that you are using.
   - `MODEL_NAME`: the name of the model.
   - `REGION`: the region used by the connection.
   - `CONNECTION_ID`: the ID of your BigQuery connection.

     You can get this value by [viewing the connection details](https://docs.cloud.google.com/bigquery/docs/working-with-connections#view-connections) in the Google Cloud console
     and copying the value in the last section of the fully qualified
     connection ID that is shown in **Connection ID** . For example,
     `projects/myproject/locations/connection_location/connections/*myconnection*`.
   - 
   - `ENDPOINT_REGION`: the region in which the open model is deployed.
   - `ENDPOINT_PROJECT_ID`: the project in which the open model is deployed.
   - `ENDPOINT_ID`: the ID of the HTTPS endpoint used by the open model. You can get the endpoint ID by locating the open model on the [Online prediction](https://console.cloud.google.com/vertex-ai/online-prediction/endpoints) page and copying the value in the **ID** field.
   - 

   <br />

## Generate text from standard table data

Generate text by using the
[`AI.GENERATE_TEXT` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-generate-text)
with prompt data from a standard table:

### Gemini


```googlesql
SELECT *
FROM AI.GENERATE_TEXT(
  MODEL `PROJECT_ID.DATASET_ID.MODEL_NAME`,
  {TABLE PROJECT_ID.DATASET_ID.TABLE_NAME | (PROMPT_QUERY)},
  STRUCT(
  {
    {
      [MAX_OUTPUT_TOKENS AS max_output_tokens]
      [, TOP_P AS top_p]
      [, TEMPERATURE AS temperature]
      [, STOP_SEQUENCES AS stop_sequences]
      [, GROUND_WITH_GOOGLE_SEARCH AS ground_with_google_search]
      [, SAFETY_SETTINGS AS safety_settings]
    }
    |
    [, MODEL_PARAMS AS model_params]
  }
  [, REQUEST_TYPE AS request_type])
);
```

<br />

Replace the following:

- `PROJECT_ID`: your project ID.
- `DATASET_ID`: the ID of the dataset that contains the model.
- `MODEL_NAME`: the name of the model.
- `TABLE_NAME`: the name of the table that contains the prompt. This table must have a column that's named `prompt`, or you can use an alias to use a differently named column.
- `PROMPT_QUERY`: a query that provides the prompt data. This query must produce a column that's named `prompt`.

  > [!NOTE]
  > **Note:**
  > We recommend against using the [`LIMIT and OFFSET` clause](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#limit_and_offset_clause) in the prompt query. Using this clause causes the query to process all of the input data first and then apply `LIMIT` and `OFFSET`.

- `TOKENS`: an `INT64` value that sets the maximum number of tokens that can be generated in the response. This value must be in the range `[1,8192]`. Specify a lower value for shorter responses and a higher value for longer responses. The default is `128`.
- `TEMPERATURE`: a `FLOAT64` value in the range `[0.0,1.0]` that controls the degree of randomness in token selection. The default is `0`.

  Lower values for `temperature` are good for prompts that
  require a more deterministic and less open-ended or creative response,
  while higher values for `temperature` can lead to more diverse
  or creative results. A value of `0` for
  `temperature` is
  deterministic, meaning that the highest probability response is
  always selected.
- `TOP_P`: a `FLOAT64` value in the range `[0.0,1.0]` helps determine the probability of the tokens selected. Specify a lower value for less random responses and a higher value for more random responses. The default is `0.95`.
- `STOP_SEQUENCES`: an `ARRAY<STRING>` value that removes the specified strings if they are included in responses from the model. Strings are matched exactly, including capitalization. The default is an empty array.
- `GROUND_WITH_GOOGLE_SEARCH`: a `BOOL` value that determines whether the Vertex AI model uses \[Grounding with Google Search\](/vertex-ai/generative-ai/docs/grounding/overview#ground-public) when generating responses. Grounding lets the model use additional information from the internet when generating a response, in order to make model responses more specific and factual. When this field is set to `True`, an additional `grounding_result` column is included in the results, providing the sources that the model used to gather additional information. The default is `FALSE`.
- `SAFETY_SETTINGS`: an `ARRAY<STRUCT<STRING AS category, STRING AS threshold>>` value that configures content safety thresholds to filter responses. The first element in the struct specifies a harm category, and the second element in the struct specifies a corresponding blocking threshold. The model filters out content that violate these settings. You can only specify each category once. For example, you can't specify both `STRUCT('HARM_CATEGORY_DANGEROUS_CONTENT' AS category, 'BLOCK_MEDIUM_AND_ABOVE' AS threshold)` and `STRUCT('HARM_CATEGORY_DANGEROUS_CONTENT' AS category, 'BLOCK_ONLY_HIGH' AS threshold)`. If there is no safety setting for a given category, the `BLOCK_MEDIUM_AND_ABOVE` safety setting is used. Supported categories are as follows:
  - `HARM_CATEGORY_HATE_SPEECH`
  - `HARM_CATEGORY_DANGEROUS_CONTENT`
  - `HARM_CATEGORY_HARASSMENT`
  - `HARM_CATEGORY_SEXUALLY_EXPLICIT`

  Supported thresholds are as follows:
  <!-- -->

  - `BLOCK_NONE` ([Restricted](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/multimodal/configure-safety-attributes#how_to_remove_automated_response_blocking_for_select_safety_attributes))
  - `BLOCK_LOW_AND_ABOVE`
  - `BLOCK_MEDIUM_AND_ABOVE` (Default)
  - `BLOCK_ONLY_HIGH`
  - `HARM_BLOCK_THRESHOLD_UNSPECIFIED`

  For more information, refer to the definition of [safety category](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/multimodal/configure-safety-attributes#safety_attribute_scoring) and [blocking threshold](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/multimodal/configure-safety-attributes#safety-settings).
- `REQUEST_TYPE`: a `STRING` value that specifies the type of inference request to send to the Gemini model. The request type determines what quota the request uses. Valid values are as follows:
  - `DEDICATED`: The `AI.GENERATE_TEXT` function only uses Provisioned Throughput quota. The `AI.GENERATE_TEXT` function returns the error `Provisioned throughput is not purchased or is not
    active` if Provisioned Throughput quota isn't available.
  - `SHARED`: The `AI.GENERATE_TEXT` function only uses [dynamic shared quota (DSQ)](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/dynamic-shared-quota), even if you have purchased Provisioned Throughput quota.
  - `UNSPECIFIED`: The `AI.GENERATE_TEXT` function uses quota as follows:
    - If you haven't purchased Provisioned Throughput quota, the `AI.GENERATE_TEXT` function uses DSQ quota.
    - If you have purchased Provisioned Throughput quota, the `AI.GENERATE_TEXT` function uses the Provisioned Throughput quota first. If requests exceed the Provisioned Throughput quota, the overflow traffic uses DSQ quota.

  The default value is `UNSPECIFIED`.

  For more information, see
  [Use Vertex AI Provisioned Throughput](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-generate-text#provisioned-throughput).
- `MODEL_PARAMS`: a JSON-formatted string literal that provides parameters to the model. The value must conform to the [`generateContent` request body](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/reference/rest/v1/projects.locations.endpoints/generateContent) format. You can provide a value for any field in the request body except for the `contents[]` field. If you set this field, then you can't also specify any model parameters in the top-level struct argument to the `AI.GENERATE_TEXT` function. You must either specify every model parameter in the `MODEL_PARAMS` field, or omit this field and specify each parameter separately.

<br />

**Example 1**

The following example shows a request with these characteristics:

- Prompts for a summary of the text in the `body` column of the `articles` table.

```googlesql
SELECT *
FROM
  AI.GENERATE_TEXT(
    MODEL `mydataset.text_model`,
    (
      SELECT CONCAT('Summarize this text', body) AS prompt
      FROM mydataset.articles
    ));
```

**Example 2**

The following example shows a request with these characteristics:

- Uses a query to create the prompt data by concatenating strings that provide prompt [prefixes](https://docs.cloud.google.com/vertex-ai/docs/generative-ai/text/text-prompts#prompt_structure) with table columns.
- Returns a short response.

```googlesql
SELECT *
FROM
  AI.GENERATE_TEXT(
    MODEL `mydataset.text_model`,
    (
      SELECT CONCAT(question, 'Text:', description, 'Category') AS prompt
      FROM mydataset.input_table
    ),
    STRUCT(
      100 AS max_output_tokens));
```

**Example 3**

The following example shows a request with these characteristics:

- Uses the `prompt` column of the `prompts` table for the prompt.

```googlesql
SELECT *
FROM
  AI.GENERATE_TEXT(
    MODEL `mydataset.text_model`,
    TABLE mydataset.prompts);
```

**Example 4**

The following example shows a request with these characteristics:

- Uses the `prompt` column of the `prompts` table for the prompt.
- Returns a short response.
- Retrieves and returns public web data for response grounding.
- Filters out unsafe responses by using two safety settings.

```googlesql
SELECT *
FROM
  AI.GENERATE_TEXT(
    MODEL `mydataset.text_model`,
    TABLE mydataset.prompts,
    STRUCT(
      100 AS max_output_tokens, 0.5 AS top_p,
      TRUE AS ground_with_google_search,
      [STRUCT('HARM_CATEGORY_HATE_SPEECH' AS category,
        'BLOCK_LOW_AND_ABOVE' AS threshold),
      STRUCT('HARM_CATEGORY_DANGEROUS_CONTENT' AS category,
        'BLOCK_MEDIUM_AND_ABOVE' AS threshold)] AS safety_settings));
```

**Example 5**

The following example shows a request with these characteristics:

- Uses the `prompt` column of the `prompts` table for the prompt.
- Returns a longer response.

```googlesql
SELECT *
FROM
  AI.GENERATE_TEXT(
    MODEL `mydataset.flash_2_model`,
    TABLE mydataset.prompts,
    STRUCT(
      0.4 AS temperature, 8192 AS max_output_tokens));
```

**Example 6**

The following example shows a request with these characteristics:

- Prompts for a summary of the text in the `body` column of the `articles` table.
- Retrieves and returns public web data for response grounding.
- Filters out unsafe responses by using two safety settings.

```googlesql
SELECT *
FROM
  AI.GENERATE_TEXT(
    MODEL `mydataset.text_model`,
    (
      SELECT CONCAT('Summarize this text', body) AS prompt
      FROM mydataset.articles
    ),
    STRUCT(
      .1 AS TEMPERATURE,
      TRUE AS ground_with_google_search,
      [STRUCT('HARM_CATEGORY_HATE_SPEECH' AS category,
        'BLOCK_LOW_AND_ABOVE' AS threshold),
      STRUCT('HARM_CATEGORY_DANGEROUS_CONTENT' AS category,
        'BLOCK_MEDIUM_AND_ABOVE' AS threshold)] AS safety_settings));
```

### Claude


```googlesql
SELECT *
FROM AI.GENERATE_TEXT(
  MODEL `PROJECT_ID.DATASET_ID.MODEL_NAME`,
  {TABLE PROJECT_ID.DATASET_ID.TABLE_NAME | (PROMPT_QUERY)},
  STRUCT(
  {
    {
      [MAX_OUTPUT_TOKENS AS max_output_tokens]
      [, TOP_K AS top_k]
      [, TOP_P AS top_p]
    }
    |
    [, MODEL_PARAMS AS model_params]
  })
);
```

<br />

Replace the following:

- `PROJECT_ID`: your project ID.
- `DATASET_ID`: the ID of the dataset that contains the model.
- `MODEL_NAME`: the name of the model.
- `TABLE_NAME`: the name of the table that contains the prompt. This table must have a column that's named `prompt`, or you can use an alias to use a differently named column.
- `PROMPT_QUERY`: a query that provides the prompt data. This query must produce a column that's named `prompt`.

  > [!NOTE]
  > **Note:**
  > We recommend against using the [`LIMIT and OFFSET` clause](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#limit_and_offset_clause) in the prompt query. Using this clause causes the query to process all of the input data first and then apply `LIMIT` and `OFFSET`.

- `TOKENS`: an `INT64` value that sets the maximum number of tokens that can be generated in the response. This value must be in the range `[1,4096]`. Specify a lower value for shorter responses and a higher value for longer responses. The default is `128`.
- `TOP_K`: an `INT64` value in the range `[1,40]` that determines the initial pool of tokens the model considers for selection. Specify a lower value for less random responses and a higher value for more random responses. If you don't specify a value, the model determines an appropriate value.
- `TOP_P`: a `FLOAT64` value in the range `[0.0,1.0]` helps determine the probability of the tokens selected. Specify a lower value for less random responses and a higher value for more random responses. If you don't specify a value, the model determines an appropriate value.
- `MODEL_PARAMS`: a JSON-formatted string literal that provides parameters to the model. The value must conform to the [`generateContent` request body](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/reference/rest/v1/projects.locations.endpoints/generateContent) format. You can provide a value for any field in the request body except for the `contents[]` field. If you set this field, then you can't also specify any model parameters in the top-level struct argument to the `AI.GENERATE_TEXT` function. You must either specify every model parameter in the `MODEL_PARAMS` field, or omit this field and specify each parameter separately.

<br />

**Example 1**

The following example shows a request with these characteristics:

- Prompts for a summary of the text in the `body` column of the `articles` table.

```googlesql
SELECT *
FROM
  AI.GENERATE_TEXT(
    MODEL `mydataset.text_model`,
    (
      SELECT CONCAT('Summarize this text', body) AS prompt
      FROM mydataset.articles
    ));
```

**Example 2**

The following example shows a request with these characteristics:

- Uses a query to create the prompt data by concatenating strings that provide prompt [prefixes](https://docs.cloud.google.com/vertex-ai/docs/generative-ai/text/text-prompts#prompt_structure) with table columns.
- Returns a short response.

```googlesql
SELECT *
FROM
  AI.GENERATE_TEXT(
    MODEL `mydataset.text_model`,
    (
      SELECT CONCAT(question, 'Text:', description, 'Category') AS prompt
      FROM mydataset.input_table
    ),
    STRUCT(
      100 AS max_output_tokens));
```

**Example 3**

The following example shows a request with these characteristics:

- Uses the `prompt` column of the `prompts` table for the prompt.

```googlesql
SELECT *
FROM
  AI.GENERATE_TEXT(
    MODEL `mydataset.text_model`,
    TABLE mydataset.prompts);
```

### Llama


```googlesql
SELECT *
FROM AI.GENERATE_TEXT(
  MODEL `PROJECT_ID.DATASET_ID.MODEL_NAME`,
  {TABLE PROJECT_ID.DATASET_ID.TABLE_NAME | (PROMPT_QUERY)},
  STRUCT(
  {
    {
      [MAX_OUTPUT_TOKENS AS max_output_tokens]
      [, TOP_P AS top_p]
      [, TEMPERATURE AS temperature]
      [, STOP_SEQUENCES AS stop_sequences]
    |
    }
    [, MODEL_PARAMS AS model_params]
  })
);
```

<br />

Replace the following:

- `PROJECT_ID`: your project ID.
- `DATASET_ID`: the ID of the dataset that contains the model.
- `MODEL_NAME`: the name of the model.
- `TABLE_NAME`: the name of the table that contains the prompt. This table must have a column that's named `prompt`, or you can use an alias to use a differently named column.
- `PROMPT_QUERY`: a query that provides the prompt data. This query must produce a column that's named `prompt`.

  > [!NOTE]
  > **Note:**
  > We recommend against using the [`LIMIT and OFFSET` clause](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#limit_and_offset_clause) in the prompt query. Using this clause causes the query to process all of the input data first and then apply `LIMIT` and `OFFSET`.

- `TOKENS`: an `INT64` value that sets the maximum number of tokens that can be generated in the response. This value must be in the range `[1,4096]`. Specify a lower value for shorter responses and a higher value for longer responses. The default is `128`.
- `TEMPERATURE`: a `FLOAT64` value in the range `[0.0,1.0]` that controls the degree of randomness in token selection. The default is `0`.

  Lower values for `temperature` are good for prompts that
  require a more deterministic and less open-ended or creative response,
  while higher values for `temperature` can lead to more diverse
  or creative results. A value of `0` for
  `temperature` is
  deterministic, meaning that the highest probability response is
  always selected.
- `TOP_P`: a `FLOAT64` value in the range `[0.0,1.0]` helps determine the probability of the tokens selected. Specify a lower value for less random responses and a higher value for more random responses. The default is `0.95`.
- `STOP_SEQUENCES`: an `ARRAY<STRING>` value that removes the specified strings if they are included in responses from the model. Strings are matched exactly, including capitalization. The default is an empty array.
- `MODEL_PARAMS`: a JSON-formatted string literal that provides parameters to the model. The value must conform to the [`generateContent` request body](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/reference/rest/v1/projects.locations.endpoints/generateContent) format. You can provide a value for any field in the request body except for the `contents[]` field. If you set this field, then you can't also specify any model parameters in the top-level struct argument to the `AI.GENERATE_TEXT` function. You must either specify every model parameter in the `MODEL_PARAMS` field, or omit this field and specify each parameter separately.

<br />

**Example 1**

The following example shows a request with these characteristics:

- Prompts for a summary of the text in the `body` column of the `articles` table.

```googlesql
SELECT *
FROM
  AI.GENERATE_TEXT(
    MODEL `mydataset.text_model`,
    (
      SELECT CONCAT('Summarize this text', body) AS prompt
      FROM mydataset.articles
    ));
```

**Example 2**

The following example shows a request with these characteristics:

- Uses a query to create the prompt data by concatenating strings that provide prompt [prefixes](https://docs.cloud.google.com/vertex-ai/docs/generative-ai/text/text-prompts#prompt_structure) with table columns.
- Returns a short response.

```googlesql
SELECT *
FROM
  AI.GENERATE_TEXT(
    MODEL `mydataset.text_model`,
    (
      SELECT CONCAT(question, 'Text:', description, 'Category') AS prompt
      FROM mydataset.input_table
    ),
    STRUCT(
      100 AS max_output_tokens));
```

**Example 3**

The following example shows a request with these characteristics:

- Uses the `prompt` column of the `prompts` table for the prompt.

```googlesql
SELECT *
FROM
  AI.GENERATE_TEXT(
    MODEL `mydataset.text_model`,
    TABLE mydataset.prompts);
```

### Mistral AI


```googlesql
SELECT *
FROM AI.GENERATE_TEXT(
  MODEL `PROJECT_ID.DATASET_ID.MODEL_NAME`,
  {TABLE PROJECT_ID.DATASET_ID.TABLE_NAME | (PROMPT_QUERY)},
  STRUCT(
  {
    {
      [MAX_OUTPUT_TOKENS AS max_output_tokens]
      [, TOP_P AS top_p]
      [, TEMPERATURE AS temperature]
      [, STOP_SEQUENCES AS stop_sequences]
    |
    }
    [, MODEL_PARAMS AS model_params]
  })
);
```

<br />

Replace the following:

- `PROJECT_ID`: your project ID.
- `DATASET_ID`: the ID of the dataset that contains the model.
- `MODEL_NAME`: the name of the model.
- `TABLE_NAME`: the name of the table that contains the prompt. This table must have a column that's named `prompt`, or you can use an alias to use a differently named column.
- `PROMPT_QUERY`: a query that provides the prompt data. This query must produce a column that's named `prompt`.

  > [!NOTE]
  > **Note:**
  > We recommend against using the [`LIMIT and OFFSET` clause](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#limit_and_offset_clause) in the prompt query. Using this clause causes the query to process all of the input data first and then apply `LIMIT` and `OFFSET`.

- `TOKENS`: an `INT64` value that sets the maximum number of tokens that can be generated in the response. This value must be in the range `[1,4096]`. Specify a lower value for shorter responses and a higher value for longer responses. The default is `128`.
- `TEMPERATURE`: a `FLOAT64` value in the range `[0.0,1.0]` that controls the degree of randomness in token selection. The default is `0`.

  Lower values for `temperature` are good for prompts that
  require a more deterministic and less open-ended or creative response,
  while higher values for `temperature` can lead to more diverse
  or creative results. A value of `0` for
  `temperature` is
  deterministic, meaning that the highest probability response is
  always selected.
- `TOP_P`: a `FLOAT64` value in the range `[0.0,1.0]` helps determine the probability of the tokens selected. Specify a lower value for less random responses and a higher value for more random responses. The default is `0.95`.
- `STOP_SEQUENCES`: an `ARRAY<STRING>` value that removes the specified strings if they are included in responses from the model. Strings are matched exactly, including capitalization. The default is an empty array.
- `MODEL_PARAMS`: a JSON-formatted string literal that provides parameters to the model. The value must conform to the [`generateContent` request body](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/reference/rest/v1/projects.locations.endpoints/generateContent) format. You can provide a value for any field in the request body except for the `contents[]` field. If you set this field, then you can't also specify any model parameters in the top-level struct argument to the `AI.GENERATE_TEXT` function. You must either specify every model parameter in the `MODEL_PARAMS` field, or omit this field and specify each parameter separately.

<br />

**Example 1**

The following example shows a request with these characteristics:

- Prompts for a summary of the text in the `body` column of the `articles` table.

```googlesql
SELECT *
FROM
  AI.GENERATE_TEXT(
    MODEL `mydataset.text_model`,
    (
      SELECT CONCAT('Summarize this text', body) AS prompt
      FROM mydataset.articles
    ));
```

**Example 2**

The following example shows a request with these characteristics:

- Uses a query to create the prompt data by concatenating strings that provide prompt [prefixes](https://docs.cloud.google.com/vertex-ai/docs/generative-ai/text/text-prompts#prompt_structure) with table columns.
- Returns a short response.

```googlesql
SELECT *
FROM
  AI.GENERATE_TEXT(
    MODEL `mydataset.text_model`,
    (
      SELECT CONCAT(question, 'Text:', description, 'Category') AS prompt
      FROM mydataset.input_table
    ),
    STRUCT(
      100 AS max_output_tokens));
```

**Example 3**

The following example shows a request with these characteristics:

- Uses the `prompt` column of the `prompts` table for the prompt.

```googlesql
SELECT *
FROM
  AI.GENERATE_TEXT(
    MODEL `mydataset.text_model`,
    TABLE mydataset.prompts);
```

### Open models

> [!NOTE]
> **Note:** You must deploy open models in Vertex AI before you can use them. For more information, see [Deploy open models](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model-open#deploy_open_models).


```sql
SELECT *
FROM AI.GENERATE_TEXT(
  MODEL `PROJECT_ID.DATASET_ID.MODEL_NAME`,
  {TABLE PROJECT_ID.DATASET_ID.TABLE_NAME | (PROMPT_QUERY)},
  STRUCT(
  {
    {
      [MAX_OUTPUT_TOKENS AS max_output_tokens]
      [, TOP_K AS top_k]
      [, TOP_P AS top_p]
      [, TEMPERATURE AS temperature]
    }
    |
    [, MODEL_PARAMS AS model_params]
  })
);
```

<br />

Replace the following:

- `PROJECT_ID`: your project ID.
- `DATASET_ID`: the ID of the dataset that contains the model.
- `MODEL_NAME`: the name of the model.
- `TABLE_NAME`: the name of the table that contains the prompt. This table must have a column that's named `prompt`, or you can use an alias to use a differently named column.
- `PROMPT_QUERY`: a query that provides the prompt data. This query must produce a column that's named `prompt`.

  > [!NOTE]
  > **Note:**
  > We recommend against using the [`LIMIT and OFFSET` clause](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#limit_and_offset_clause) in the prompt query. Using this clause causes the query to process all of the input data first and then apply `LIMIT` and `OFFSET`.

- `TOKENS`: an `INT64` value that sets the maximum number of tokens that can be generated in the response. This value must be in the range `[1,4096]`. Specify a lower value for shorter responses and a higher value for longer responses. If you don't specify a value, the model determines an appropriate value.
- `TEMPERATURE`: a `FLOAT64` value in the range `[0.0,1.0]` that controls the degree of randomness in token selection. If you don't specify a value, the model determines an appropriate value.

  Lower values for `temperature` are good for prompts that
  require a more deterministic and less open-ended or creative response,
  while higher values for `temperature` can lead to more diverse
  or creative results. A value of `0` for
  `temperature` is
  deterministic, meaning that the highest probability response is
  always selected.
- `TOP_K`: an `INT64` value in the range `[1,40]` that determines the initial pool of tokens the model considers for selection. Specify a lower value for less random responses and a higher value for more random responses. If you don't specify a value, the model determines an appropriate value.
- `TOP_P`: a `FLOAT64` value in the range `[0.0,1.0]` helps determine the probability of the tokens selected. Specify a lower value for less random responses and a higher value for more random responses. If you don't specify a value, the model determines an appropriate value.
- `MODEL_PARAMS`: a JSON-formatted string literal that provides parameters to the model. The value must conform to the [`generateContent` request body](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/reference/rest/v1/projects.locations.endpoints/generateContent) format. You can provide a value for any field in the request body except for the `contents[]` field. If you set this field, then you can't also specify any model parameters in the top-level struct argument to the `AI.GENERATE_TEXT` function. You must either specify every model parameter in the `MODEL_PARAMS` field, or omit this field and specify each parameter separately.

<br />

**Example 1**

The following example shows a request with these characteristics:

- Prompts for a summary of the text in the `body` column of the `articles` table.

```googlesql
SELECT *
FROM
  AI.GENERATE_TEXT(
    MODEL `mydataset.text_model`,
    (
      SELECT CONCAT('Summarize this text', body) AS prompt
      FROM mydataset.articles
    ));
```

**Example 2**

The following example shows a request with these characteristics:

- Uses a query to create the prompt data by concatenating strings that provide prompt [prefixes](https://docs.cloud.google.com/vertex-ai/docs/generative-ai/text/text-prompts#prompt_structure) with table columns.
- Returns a short response.

```googlesql
SELECT *
FROM
  AI.GENERATE_TEXT(
    MODEL `mydataset.text_model`,
    (
      SELECT CONCAT(question, 'Text:', description, 'Category') AS prompt
      FROM mydataset.input_table
    ),
    STRUCT(
      100 AS max_output_tokens));
```

**Example 3**

The following example shows a request with these characteristics:

- Uses the `prompt` column of the `prompts` table for the prompt.

```googlesql
SELECT *
FROM
  AI.GENERATE_TEXT(
    MODEL `mydataset.text_model`,
    TABLE mydataset.prompts);
```

## Generate text from object table data

Generate text by using the
[`AI.GENERATE_TEXT` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-generate-text)
with a Gemini model to analyze unstructured data from an object
table. You provide the prompt data in the `prompt` parameter.

```googlesql
SELECT *
FROM AI.GENERATE_TEXT(
MODEL `PROJECT_ID.DATASET.MODEL`,
{ TABLE `PROJECT_ID.DATASET.TABLE` | (QUERY_STATEMENT) },
STRUCT(
  PROMPT AS prompt
  {
    {
      [, MAX_OUTPUT_TOKENS AS max_output_tokens]
      [, TOP_P AS top_p]
      [, TEMPERATURE AS temperature]
      [, STOP_SEQUENCES AS stop_sequences]
      [, SAFETY_SETTINGS AS safety_settings]
    }
    |
    [, MODEL_PARAMS AS model_params]
  })
);
```

Replace the following:

- `PROJECT_ID`: the project that contains the resource.
- `DATASET`: the dataset that contains the resource.
- `MODEL`: the name of the remote model over the Vertex AI model. For more information about how to create this type of remote model, see [The `CREATE MODEL` statement for remote models over LLMs](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model).

  You can confirm which model is used by the remote model by opening the
  Google Cloud console and looking at the **Remote endpoint** field in the
  model details page.

  Note: Using a remote model based on a Gemini 2.5 model incurs
  charges for the
  [thinking process](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/thinking).
- `TABLE`: the name of the [object table](https://docs.cloud.google.com/bigquery/docs/object-table-introduction) that contains the content to analyze. For more information on what types of content you can analyze, see [Input](https://docs.cloud.google.com/bigquery/docs/generate-text#input).

  The Cloud Storage bucket used by the input object table must be in
  the same project where you have created the model and where you are
  calling the `AI.GENERATE_TEXT` function.
- `QUERY_STATEMENT`: the GoogleSQL query that generates the image data. You can only specify `WHERE` and `ORDER BY` clauses in the query.
- `PROMPT`: a `STRING` value that contains the prompt to use to analyze the visual content. The `prompt` value must contain less than 16,000 tokens. A token might be smaller than a word and is approximately four characters. One hundred tokens correspond to approximately 60-80 words.

#### Show additional optional parameters

- `MAX_OUTPUT_TOKENS`: an `INT64` value that sets the maximum number of tokens that can be generated in the response. This value must be in the range `[1,8192]`. Specify a lower value for shorter responses and a higher value for longer responses. The default is `1024`.
- `TOP_P`: a `FLOAT64` value in the range `[0.0,1.0]` that changes how the model selects tokens for output. Specify a lower value for less random responses and a higher value for more random responses. The default is `0.95`.

  Tokens are selected from the most to least probable until the sum of
  their probabilities equals the `TOP_P` value. For example, if
  tokens A, B, and C have a probability of `0.3`,
  `0.2`, and `0.1`, and the `TOP_P` value
  is `0.5`, then the model selects either A or B as the next
  token by using the `TEMPERATURE` value and doesn't consider C.
- `TEMPERATURE`: a `FLOAT64` value in the range `[0.0,1.0]` that controls the degree of randomness in token selection. Lower `TEMPERATURE` values are good for prompts that require a more deterministic and less open-ended or creative response, while higher `TEMPERATURE` values can lead to more diverse or creative results. A `TEMPERATURE` value of `0` is deterministic, meaning that the highest probability response is always selected. The default is `0`.
- `STOP_SEQUENCES`: an `ARRAY` value that removes the specified strings if they are included in responses from the model. Strings are matched exactly, including capitalization. The default is an empty array.
- `SAFETY_SETTINGS`: an `ARRAY>` value that configures content safety thresholds to filter responses. The first element in the struct specifies a harm category, and the second element in the struct specifies a corresponding blocking threshold. The model filters out content that violate these settings. You can only specify each category once. For example, you can't specify both `STRUCT('HARM_CATEGORY_DANGEROUS_CONTENT' AS category,
  'BLOCK_MEDIUM_AND_ABOVE' AS threshold)` and `STRUCT('HARM_CATEGORY_DANGEROUS_CONTENT' AS category,
  'BLOCK_ONLY_HIGH' AS threshold)`. If there is no safety setting for a given category, the `BLOCK_MEDIUM_AND_ABOVE` safety setting is used.

  Supported categories are as follows:
  - `HARM_CATEGORY_HATE_SPEECH`
  - `HARM_CATEGORY_DANGEROUS_CONTENT`
  - `HARM_CATEGORY_HARASSMENT`
  - `HARM_CATEGORY_SEXUALLY_EXPLICIT`

  Supported thresholds are as follows:
  - `BLOCK_NONE` ([Restricted](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/multimodal/configure-safety-attributes#how_to_remove_automated_response_blocking_for_select_safety_attributes))
  - `BLOCK_LOW_AND_ABOVE`
  - `BLOCK_MEDIUM_AND_ABOVE` (Default)
  - `BLOCK_ONLY_HIGH`
  - `HARM_BLOCK_THRESHOLD_UNSPECIFIED`

  For more information, refer to the definition of
  [safety category](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/multimodal/configure-safety-filters#harm_categories) and
  [blocking threshold](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/multimodal/configure-safety-filters#how-to-configure-content-filters).
- `MODEL_PARAMS`: a JSON-formatted string literal that provides additional parameters to the model. The value must conform to the [`generateContent` request body](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/reference/rest/v1/projects.locations.endpoints/generateContent) format. You can provide a value for any field in the request body except for the `contents[]` field. If you set this field, then you can't also specify any model parameters in the top-level struct argument to the `AI.GENERATE_TEXT` function.

**Examples**

This example translates and transcribes audio content from an
object table that's named `feedback`:

```googlesql
SELECT * FROM
  AI.GENERATE_TEXT(
    MODEL `mydataset.audio_model`,
    TABLE `mydataset.feedback`,
      STRUCT('What is the content of this audio clip, translated into Spanish?' AS PROMPT));
```

This example classifies PDF content from an object table
that's named `invoices`:

```googlesql
SELECT * FROM
  AI.GENERATE_TEXT(
    MODEL `mydataset.classify_model`,
    TABLE `mydataset.invoices`,
      STRUCT('Classify this document based on the invoice total, using the following categories: 0 to 100, 101 to 200, greater than 200' AS PROMPT));
```