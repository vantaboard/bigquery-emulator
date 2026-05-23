# Generate text embeddings by using the AI.GENERATE_EMBEDDING function

This document shows you how to create a BigQuery ML
[remote model](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model)
that references an embedding model. You then use that model with the
[`AI.GENERATE_EMBEDDING` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-generate-embedding)
to create text embeddings by using data from a BigQuery
[standard table](https://docs.cloud.google.com/bigquery/docs/tables-intro#standard-tables).

The following types of remote models are supported:

- [Remote models](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model) over [Vertex AI embedding models](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/models#embeddings-models).
- [Remote models](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model-open) over [supported open models](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model-open#supported_open_models).

## Required roles

To create a remote model and use the `AI.GENERATE_EMBEDDING` function, you
need the following Identity and Access Management (IAM) roles:

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
- Query table data: `bigquery.tables.getData`

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

### Grant a role to the remote model connection's service account

You must grant the connection's service account the Vertex AI User role.

If you plan to specify the endpoint as a URL when you create the remote model,
for example
`endpoint = 'https://us-central1-aiplatform.googleapis.com/v1/projects/myproject/locations/us-central1/publishers/google/models/text-embedding-005'`,
grant this role in the same project you specify in the URL.

If you plan to specify the endpoint by using the model name when you create
the remote model, for example `endpoint = 'text-embedding-005'`, grant this
role in the same project where you plan to create the remote model.

Granting the role in a different project results in the error
`bqcx-1234567890-wxyz@gcp-sa-bigquery-condel.iam.gserviceaccount.com does not have the permission to access resource`.

To grant the role, follow these steps:

### Console

1. Go to the **IAM \& Admin** page.

   [Go to IAM \& Admin](https://console.cloud.google.com/project/_/iam-admin)
2. Click **Grant access**.

   The **Add principals** dialog opens.
3. In the **New principals** field, enter the service account ID that you
   copied earlier.

4. In the **Select a role** field, select **Vertex AI** , and then select
   **Vertex AI User**.

5. Click **Save**.

### gcloud

Use the
[`gcloud projects add-iam-policy-binding` command](https://docs.cloud.google.com/sdk/gcloud/reference/projects/add-iam-policy-binding):

```
gcloud projects add-iam-policy-binding 'PROJECT_NUMBER' --member='serviceAccount:MEMBER' --role='roles/aiplatform.user' --condition=None
```

Replace the following:

- `PROJECT_NUMBER`: your project number
- `MEMBER`: the service account ID that you copied earlier

<br />

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
   - `ENDPOINT`: the name of an embedding model to use. For more information, see [`ENDPOINT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model#endpoint).

     The Vertex AI model that you specify
     must be available in the location where you are creating the remote
     model. For more information, see
     [Locations](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model#locations).

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

## Generate text embeddings

Generate text embeddings with the
[`AI.GENERATE_EMBEDDING` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-generate-embedding)
by using text data from a table column or a query.

Typically, you would use a text embedding model for text-only use cases, and a
multimodal embedding model for cross-modal search use cases, where embeddings
for text and visual content are generated in the same semantic space.

### Vertex AI text

Generate text embeddings by using a remote model over a
Vertex AI text embedding model:

```googlesql
SELECT *
FROM AI.GENERATE_EMBEDDING(
  MODEL `PROJECT_ID.DATASET_ID.MODEL_NAME`,
  {TABLE PROJECT_ID.DATASET_ID.TABLE_NAME | (CONTENT_QUERY)},
  STRUCT(TASK_TYPE AS task_type,
    OUTPUT_DIMENSIONALITY AS output_dimensionality)
);
```

Replace the following:

- `PROJECT_ID`: your project ID.
- `DATASET_ID`: the ID of the dataset that contains the model.
- `MODEL_NAME`: the name of the remote model over an embedding model.
- `TABLE_NAME`: the name of the table that contains the text to embed. This table must have a column that's named `content`, or you can use an alias to use a differently named column.
- `CONTENT_QUERY`: a query whose result contains a `STRING` column called `content`.
- `TASK_TYPE`: a `STRING` literal that specifies the intended downstream application to help the model produce better quality embeddings. `TASK_TYPE` accepts the following values:
  - `RETRIEVAL_QUERY`: specifies that the given text is a query in a search or retrieval setting.
  - `RETRIEVAL_DOCUMENT`: specifies that the given text is a document in a search or retrieval setting.


    When using this task type, it is helpful to include the document title
    in the query statement in order to improve embedding quality.
    The document title must be in a column either named
    `title` or aliased as `title`, for example:

              SELECT *
              FROM
                AI.GENERATE_EMBEDDING(
                  MODEL mydataset.embedding_model,
                  (SELECT abstract as content, header as title, publication_number
                  FROM mydataset.publications),
                  STRUCT('RETRIEVAL_DOCUMENT' as task_type)
              );
              

    Specifying the title column in the input query populates the
    [`title` field](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/model-reference/text-embeddings-api#request_body)
    of the request body sent to the model. If you specify a
    `title` value when using any other task type, that input
    is ignored and has no effect on the embedding results.
  - `SEMANTIC_SIMILARITY`: specifies that the given text will be used for Semantic Textual Similarity (STS).
  - `CLASSIFICATION`: specifies that the embeddings will be used for classification.
  - `CLUSTERING`: specifies that the embeddings will be used for clustering.
  - `QUESTION_ANSWERING`: specifies that the embeddings will be used for question answering.
  - `FACT_VERIFICATION`: specifies that the embeddings will be used for fact verification.
  - `CODE_RETRIEVAL_QUERY`: specifies that the embeddings will be used for code retrieval.
- `OUTPUT_DIMENSIONALITY`: an `INT64` value that specifies the number of dimensions to use when generating embeddings. For example, if you specify `256 AS
  output_dimensionality`, then the `embedding` output column contains a 256 dimensional embedding for each input value.

  For remote models over `gemini-embedding-2-preview` or `gemini-embedding-001` models,
  the `OUTPUT_DIMENSIONALITY` value must be in the
  range `[1, 3072]`. The default value is `3072`. For remote models over
  `text-embedding` models, the
  `OUTPUT_DIMENSIONALITY` value
  must be in the range `[1, 768]`. The default value is `768`.

<br />

**Example: Embed text in a table**

The following example shows a request to embed the `content` column
of the `text_data` table:

```googlesql
SELECT *
FROM
  AI.GENERATE_EMBEDDING(
    MODEL `mydataset.embedding_model`,
    TABLE mydataset.text_data,
    STRUCT('CLASSIFICATION' AS task_type)
  );
```

### Open text


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

Note: To give feedback or request support for this feature, contact [bqml-feedback@google.com](mailto:bqml-feedback@google.com).

<br />

Generate text embeddings by using a remote model over an open
embedding model:

```googlesql
SELECT *
FROM AI.GENERATE_EMBEDDING(
  MODEL `PROJECT_ID.DATASET_ID.MODEL_NAME`,
  {TABLE PROJECT_ID.DATASET_ID.TABLE_NAME | (CONTENT_QUERY)},
);
```

Replace the following:

- `PROJECT_ID`: your project ID.
- `DATASET_ID`: the ID of the dataset that contains the model.
- `MODEL_NAME`: the name of the remote model over an embedding model.
- `TABLE_NAME`: the name of the table that contains the text to embed. This table must have a column that's named `content`, or you can use an alias to use a differently named column.
- `CONTENT_QUERY`: a query whose result contains a `STRING` column called `content`.

<br />

### Vertex AI multimodal

Generate text embeddings by using a remote model over a
Vertex AI multimodal embedding model:

```googlesql
SELECT *
FROM AI.GENERATE_EMBEDDING(
  MODEL `PROJECT_ID.DATASET_ID.MODEL_NAME`,
  {TABLE PROJECT_ID.DATASET_ID.TABLE_NAME | (CONTENT_QUERY)},
  STRUCT(OUTPUT_DIMENSIONALITY AS output_dimensionality)
);
```

Replace the following:

- `PROJECT_ID`: your project ID.
- `DATASET_ID`: the ID of the dataset that contains the model.
- `MODEL_NAME`: the name of the remote model over a `multimodalembedding@001` model.
- `TABLE_NAME`: the name of the table that contains the text to embed. This table must have a column that's named `content`, or you can use an alias to use a differently named column.
- `CONTENT_QUERY`: a query whose result contains a `STRING` column called `content`.
- `OUTPUT_DIMENSIONALITY`: an `INT64` value that specifies the number of dimensions to use when generating embeddings. Valid values are `128`, `256`, `512`, and `1408`. The default value is `1408`. For example, if you specify `256 AS output_dimensionality`, then the `embedding` output column contains a 256-dimensional embedding for each input value.

<br />

**Example: Use embeddings to rank semantic similarity**

The following example embeds a collection of movie reviews and orders them by
cosine distance to the review "This movie was average" using the [`VECTOR_SEARCH` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/search_functions#vector_search).
A smaller distance indicates more semantic similarity.

For more information about vector search and vector index, see [Introduction to vector search](https://docs.cloud.google.com/bigquery/docs/vector-search-intro).

```googlesql
CREATE TEMPORARY TABLE movie_review_embeddings AS (
  SELECT *
  FROM
    AI.GENERATE_EMBEDDING(
      MODEL `bqml_tutorial.embedding_model`,
      (
        SELECT "This movie was fantastic" AS content
        UNION ALL
        SELECT "This was the best movie I've ever seen!!" AS content
        UNION ALL
        SELECT "This movie was just okay..." AS content
        UNION ALL
        SELECT "This movie was terrible." AS content
      )
    )
);

WITH average_review_embedding AS (
  SELECT embedding
  FROM
    AI.GENERATE_EMBEDDING(
      MODEL `bqml_tutorial.embedding_model`,
      (SELECT "This movie was average" AS content)
    )
)
SELECT
  base.content AS content,
  distance AS distance_to_average_review
FROM
  VECTOR_SEARCH(
    TABLE movie_review_embeddings,
    "embedding",
    (SELECT embedding FROM average_review_embedding),
    distance_type=>"COSINE",
    top_k=>-1
  )
ORDER BY distance_to_average_review;
```

The result is similar to the following:

```
+---+---+
| content                                  | distance_to_average_review |
+---+---+
| This movie was just okay...              | 0.062789813467745592       |
| This movie was fantastic                 |  0.18579561313064263       |
| This movie was terrible.                 |  0.35707466240930985       |
| This was the best movie I've ever seen!! |  0.41844932504542975       |
+---+---+
```

## What's next

- Learn how to [use text and image embeddings to perform a text-to-image semantic search](https://docs.cloud.google.com/bigquery/docs/generate-multimodal-embeddings).
- Learn how to [use text embeddings for semantic search and retrieval-augmented generation (RAG)](https://docs.cloud.google.com/bigquery/docs/vector-index-text-search-tutorial).