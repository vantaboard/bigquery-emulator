# Generate structured data by using the AI.GENERATE_TABLE function

This document shows you how to generate structured data using a Gemini
model, and then format the model's response using a SQL schema.

You do this by completing the following tasks:

- Creating a BigQuery ML [remote model](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model) over any of the [generally available](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/models#generally_available_models) or [preview](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/models#preview_models) Gemini models.
- Using the model with the [`AI.GENERATE_TABLE` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-generate-table) to generate structured data based on data from [standard tables](https://docs.cloud.google.com/bigquery/docs/tables-intro#standard-tables).

## Required roles

To create a remote model and use the `AI.GENERATE_TABLE` function, you need the
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

### Give the service account access

Grant the connection's service account the Vertex AI User role.

If you plan to specify the endpoint as a URL when you create the remote model---
for example `endpoint = 'https://us-central1-aiplatform.googleapis.com/v1/projects/myproject/locations/us-central1/publishers/google/models/gemini-2.5-flash'`---
grant this role in the same project you specify in the URL.

If you plan to specify the endpoint by using the model name when you create
the remote model, for example `endpoint = 'gemini-2.5-flash'`, grant this role
in the same project where you plan to create the remote model.

Granting the role in a different project results in the error
`bqcx-1234567890-wxyz@gcp-sa-bigquery-condel.iam.gserviceaccount.com does not have the permission to access resource`.

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

## Create a BigQuery ML remote model

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. Using the SQL editor, create a
   [remote model](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model):

   ```sql
   CREATE OR REPLACE MODEL
   `PROJECT_ID.DATASET_ID.MODEL_NAME`
   REMOTE WITH CONNECTION {DEFAULT | `PROJECT_ID.REGION.CONNECTION_ID`}
   OPTIONS (ENDPOINT = 'ENDPOINT');
   ```

   Replace the following:
   - `PROJECT_ID`: your project ID
   - `DATASET_ID`: the ID of the dataset to contain the model. This dataset must be in the same [location](https://docs.cloud.google.com/bigquery/docs/locations) as the connection that you are using
   - `MODEL_NAME`: the name of the model
   - `REGION`: the region used by the connection
   - `CONNECTION_ID`: the ID of your BigQuery connection

     When you [view the connection details](https://docs.cloud.google.com/bigquery/docs/working-with-connections#view-connections)
     in the Google Cloud console, this is the value in the last section of
     the fully qualified connection ID that is shown in
     **Connection ID** , for example
     `projects/myproject/locations/connection_location/connections/*myconnection*`
   - `ENDPOINT`: the name of the Gemini model to use. For supported Gemini models, you can specify the [global endpoint](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model#global-endpoint) to improve availability. For more information, see [`ENDPOINT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model#endpoint).

   <br />

## Generate structured data

Generate structured data by using the
[`AI.GENERATE_TABLE` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-generate-table)
with a remote model, and using prompt data from a
table column:

```sql
SELECT *
FROM AI.GENERATE_TABLE(
  MODEL `PROJECT_ID.DATASET_ID.MODEL_NAME`,
  [TABLE `PROJECT_ID.DATASET_ID.TABLE_NAME` / (PROMPT_QUERY)],
  STRUCT(TOKENS AS max_output_tokens, TEMPERATURE AS temperature,
  TOP_P AS top_p, STOP_SEQUENCES AS stop_sequences,
  SAFETY_SETTINGS AS safety_settings,
  OUTPUT_SCHEMA AS output_schema)
);
```

Replace the following:

- `PROJECT_ID`: your project ID.
- `DATASET_ID`: the ID of the dataset that contains the model.
- `MODEL_NAME`: the name of the model.
- `TABLE_NAME`: the name of the table that contains the prompt. This table must have a column that's named `prompt`, or you can use an alias to use a differently named column.
- `PROMPT_QUERY`: the GoogleSQL query that generates the prompt data. The prompt value itself can be pulled from a column, or you can specify it as a struct value with an arbitrary number of string and column name subfields. For example, `SELECT ('Analyze the sentiment in ', feedback_column, 'using the following categories: positive, negative,
  neutral') AS prompt`.
- `TOKENS`: an `INT64` value that sets the maximum number of tokens that can be generated in the response. This value must be in the range `[1,8192]`. Specify a lower value for shorter responses and a higher value for longer responses. The default is `128`.
- `TEMPERATURE`: a `FLOAT64` value in the range `[0.0,2.0]` that controls the degree of randomness in token selection. The default is `1.0`.

  Lower values for
  `temperature` are good for prompts that require a more
  deterministic and less open-ended or creative response, while higher values
  for `temperature` can lead to more diverse or creative results. A
  value of `0` for `temperature` is deterministic, meaning
  that the highest probability response is always selected.
- `TOP_P`: a `FLOAT64` value in the range `[0.0,1.0]` helps determine the probability of the tokens selected. Specify a lower value for less random responses and a higher value for more random responses. The default is `0.95`.
- `STOP_SEQUENCES`: an `ARRAY<STRING>` value that removes the specified strings if they are included in responses from the model. Strings are matched exactly, including capitalization. The default is an empty array.
- `SAFETY_SETTINGS`: an `ARRAY<STRUCT<STRING AS category, STRING AS
  threshold>>` value that configures content safety thresholds to filter responses. The first element in the struct specifies a harm category, and the second element in the struct specifies a corresponding blocking threshold. The model filters out content that violate these settings. You can only specify each category once. For example, you can't specify both `STRUCT('HARM_CATEGORY_DANGEROUS_CONTENT' AS category,
  'BLOCK_MEDIUM_AND_ABOVE' AS threshold)` and `STRUCT('HARM_CATEGORY_DANGEROUS_CONTENT' AS category, 'BLOCK_ONLY_HIGH' AS
  threshold)`. If there is no safety setting for a given category, the `BLOCK_MEDIUM_AND_ABOVE` safety setting is used.

  <br />

  Supported categories are as follows:

  <br />

  - `HARM_CATEGORY_HATE_SPEECH`
  - `HARM_CATEGORY_DANGEROUS_CONTENT`
  - `HARM_CATEGORY_HARASSMENT`
  - `HARM_CATEGORY_SEXUALLY_EXPLICIT`

  <br />

  Supported thresholds are as follows:

  <br />

  - `BLOCK_NONE` ([Restricted](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/multimodal/configure-safety-filters#how_to_configure_content_filters))
  - `BLOCK_LOW_AND_ABOVE`
  - `BLOCK_MEDIUM_AND_ABOVE` (Default)
  - `BLOCK_ONLY_HIGH`
  - `HARM_BLOCK_THRESHOLD_UNSPECIFIED`

  <br />

  For more information, see [Harm categories](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/multimodal/configure-safety-filters#harm_categories) and [How to configure content filters](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/multimodal/configure-safety-filters#how_to_configure_content_filters).
- `OUTPUT_SCHEMA`: a `STRING` value that specifies the format for the model's response. The `output_schema` value must be a SQL schema definition, similar to that used in the [`CREATE TABLE` statement](https://docs.cloud.google.com/bigquery/docs/tables#create_an_empty_table_with_a_schema_definition). The following data types are supported:
  - `INT64`
  - `FLOAT64`
  - `BOOL`
  - `STRING`
  - `ARRAY`
  - `STRUCT`


  When using the `output_schema` argument to generate structured data
  based on prompts from a table, it is important to understand the
  prompt data in order to specify an appropriate schema.


  For example, say you are analyzing movie review content from a table that
  has the following fields:
  - movie_id
  - review
  - prompt

  Then you might create prompt text by running a query similar to the
  following:

      UPDATE mydataset.movie_review
      SET prompt = CONCAT('Extract the key words and key sentiment from the text below: ', review)
      WHERE review IS NOT NULL;


  And you might specify a `output_schema` value similar to
  `"keywords ARRAY<STRING>, sentiment STRING" AS output_schema`.

<br />

### Examples

The following example shows a request that takes prompt data from a table and
provides a SQL schema to format the model's response:

```googlesql
SELECT
*
FROM
AI.GENERATE_TABLE( MODEL `mydataset.gemini_model`,
  TABLE `mydataset.mytable`,
  STRUCT("keywords ARRAY<STRING>, sentiment STRING" AS output_schema));
```

The following example shows a request that takes prompt data from a query and
provides a SQL schema to format the model's response:

```googlesql
SELECT *
FROM
  AI.GENERATE_TABLE(
    MODEL `mydataset.gemini_model`,
    (
      SELECT
        'John Smith is a 20-year old single man living at 1234 NW 45th St, Kirkland WA, 98033. He has two phone numbers 123-123-1234, and 234-234-2345. He is 200.5 pounds.'
          AS prompt
    ),
    STRUCT("address STRUCT<street_address STRING, city STRING, state STRING, zip_code STRING>, age INT64, is_married BOOL, name STRING, phone_number ARRAY<STRING>, weight_in_pounds FLOAT64"
        AS output_schema, 8192 AS max_output_tokens));
```