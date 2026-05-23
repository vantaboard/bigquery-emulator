# Tune a model using your data

This document shows you how to create a BigQuery ML
[remote model](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model-tuned)
that references a Vertex AI model, and then configure the model to
perform supervised tuning. The Vertex AI model must be one of
the following:

- `gemini-2.5-pro`
- `gemini-2.5-flash-lite`
- `gemini-2.0-flash-001`
- `gemini-2.0-flash-lite-001`

After you create the remote model, you use the
[`ML.EVALUATE` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-evaluate)
to evaluate the model and confirm that the model's performance suits your use
case. You can then use the model in conjunction with the
[`AI.GENERATE_TEXT` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-generate-text)
to analyze text in a BigQuery table.

For more information, see
[Vertex AI Gemini API model supervised tuning](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/models/tune-gemini-overview).

## Required roles

To create and evaluate a tuned model, you need the following
Identity and Access Management (IAM) roles:

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


   Enable the BigQuery, BigQuery Connection,Vertex AI, and Compute Engine APIs.


   **Roles required to enable APIs**


   To enable APIs, you need the Service Usage Admin IAM
   role (`roles/serviceusage.serviceUsageAdmin`), which
   contains the `serviceusage.services.enable` permission. [Learn how to grant
   roles](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).

   [Enable the APIs](https://console.cloud.google.com/flows/enableapi?apiid=bigquery.googleapis.com,bigqueryconnection.googleapis.com,aiplatform.googleapis.com,compute.googleapis.com)

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

### Give the connection's service account access

Grant the connection's service account the Vertex AI Service Agent role.

If you plan to specify the endpoint as a URL when you create the remote model,
for example `endpoint = 'https://us-central1-aiplatform.googleapis.com/v1/projects/myproject/locations/us-central1/publishers/google/models/gemini-2.0-flash'`,
grant this role in the same project you specify in the URL.

If you plan to specify the endpoint by using the model name when you create
the remote model, for example `endpoint = 'gemini-2.0-flash'`, grant this role
in the same project where you plan to create the remote model.

Granting the role in a different project results in the error
`bqcx-1234567890-wxyz@gcp-sa-bigquery-condel.iam.gserviceaccount.com does not have the permission to access resource`.

To grant the role, follow these steps:

### Console

1. Go to the **IAM \& Admin** page.

   [Go to IAM \& Admin](https://console.cloud.google.com/project/_/iam-admin)
2. Click **Grant Access**.

3. For **New principals**, enter the service account ID that you copied
   earlier.

4. Click **Select a role**.

5. In **Filter** , type `Vertex AI Service Agent` and then select that role.

6. Click **Save**.

### gcloud

Use the
[`gcloud projects add-iam-policy-binding` command](https://docs.cloud.google.com/sdk/gcloud/reference/projects/add-iam-policy-binding):

```
gcloud projects add-iam-policy-binding 'PROJECT_NUMBER' --member='serviceAccount:MEMBER' --role='roles/aiplatform.serviceAgent' --condition=None
```

Replace the following:

- `PROJECT_NUMBER`: your project number.
- `MEMBER`: the service account ID that you copied earlier.

<br />

The service account associated with your connection is an instance of the
[BigQuery Connection Delegation Service Agent](https://docs.cloud.google.com/iam/docs/service-agents#bigquery-connection-delegation-service-agent),
so it is OK to assign a service agent role to it.

## Create a model with supervised tuning

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, run the following query to create a
   [remote model](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model-tuned):

   ```googlesql
   CREATE OR REPLACE MODEL
   `PROJECT_ID.DATASET_ID.MODEL_NAME`
   REMOTE WITH CONNECTION {DEFAULT | `PROJECT_ID.REGION.CONNECTION_ID`}
   OPTIONS (
     ENDPOINT = 'ENDPOINT',
     MAX_ITERATIONS = MAX_ITERATIONS,
     LEARNING_RATE_MULTIPLIER = LEARNING_RATE_MULTIPLIER,
     DATA_SPLIT_METHOD = 'DATA_SPLIT_METHOD',
     DATA_SPLIT_EVAL_FRACTION = DATA_SPLIT_EVAL_FRACTION,
     DATA_SPLIT_COL = 'DATA_SPLIT_COL',
     EVALUATION_TASK = 'EVALUATION_TASK',
     PROMPT_COL = 'INPUT_PROMPT_COL',
     INPUT_LABEL_COLS = INPUT_LABEL_COLS)
   AS SELECT PROMPT_COLUMN, LABEL_COLUMN
   FROM `TABLE_PROJECT_ID.TABLE_DATASET.TABLE_NAME`;
   ```

   Replace the following:
   - `PROJECT_ID`: the project ID of the project in which to create the model.
   - `DATASET_ID`: the ID of the dataset to contain the model. This dataset must be in a [supported
     Vertex AI region](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/learn/locations).
   - `MODEL_NAME`: the name of the model.
   - `REGION`: the region used by the connection.
   - `CONNECTION_ID`: the ID of your BigQuery connection. This connection must be in the same [location](https://docs.cloud.google.com/bigquery/docs/locations) as the dataset that you are using.

     When you [view the connection details](https://docs.cloud.google.com/bigquery/docs/working-with-connections#view-connections)
     in the Google Cloud console, this is the value in the last section of
     the fully qualified connection ID that is shown in
     **Connection ID** ---for example,
     `projects/myproject/locations/connection_location/connections/*myconnection*`.
   - `ENDPOINT`: a `STRING` value that specifies the name of the model to use.
   - `MAX_ITERATIONS`: an `INT64` value that specifies the number of steps to run for supervised tuning. The `MAX_ITERATIONS` value must be between `1` and `∞`.

     Gemini models train using epochs
     rather than steps, so BigQuery ML converts the
     `MAX_ITERATIONS` value to epochs.
     The default value for `MAX_ITERATIONS` is the
     number of rows in the input data, which is equivalent to one epoch. To
     use multiple epochs, specify a multiple of the number of rows in your
     training data. For example, if you have 100 rows of input data and you
     want to use two epochs, specify `200` for the argument value.
     If you provide a value that isn't a multiple of the number of rows in
     the input data, BigQuery ML rounds up to the nearest epoch.
     For example, if you have 100 rows of input data and you specify `101` for
     the `MAX_ITERATIONS` value, training is performed
     with two epochs.

     For more information about the parameters used to tune
     Gemini models, see
     [Create a tuning job](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/models/gemini-use-supervised-tuning#create_a_text_model_supervised_tuning_job).
   - `DATA_SPLIT_METHOD`: a `STRING` value that specifies the method used to split input data into training and evaluation sets. The valid options are the following:
     - `AUTO_SPLIT`: BigQuery ML automatically splits the data. The way in which the data is split varies depending on the number of rows in the input table. This is the default value.
     - `RANDOM`: data is randomized before being split into sets. To customize the data split, you can use this option with the `DATA_SPLIT_EVAL_FRACTION` option.
     - `CUSTOM`: data is split using the column provided in the `DATA_SPLIT_COL` option. The `DATA_SPLIT_COL` value must be the name of a column of type `BOOL`. Rows with a value of `TRUE` or `NULL` are used as evaluation data, and rows with a value of `FALSE` are used as training data.
     - `SEQ`: split data using the column provided in the `DATA_SPLIT_COL` option. The `DATA_SPLIT_COL` value must be the name of a column of one of the following types:
       - `NUMERIC`
       - `BIGNUMERIC`
       - `STRING`
       - `TIMESTAMP`

       The data is sorted smallest to largest based on the specified column.

       The first *n* rows are used as evaluation data, where *n*
       is the value specified for
       `DATA_SPLIT_EVAL_FRACTION`. The remaining rows
       are used as training data.
     - `NO_SPLIT`: no data split; all input data is used as training data.

     For more information about these data split options, see
     [`DATA_SPLIT_METHOD`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model-tuned#data_split_method).
   - `DATA_SPLIT_EVAL_FRACTION`: a `FLOAT64` value that specifies the fraction of the data to use as evaluation data when performing supervised tuning. Must be a value in the range `[0, 1.0]`. The default value is `0.2`.

     Use this option
     when you specify `RANDOM` or `SEQ` as the
     value for the `DATA_SPLIT_METHOD` option. To
     customize the data split, you can use the
     `DATA_SPLIT_METHOD` option with
     the `DATA_SPLIT_EVAL_FRACTION`
     option.
   - `DATA_SPLIT_COL`: a `STRING` value that specifies the name of the column to use to sort input data into the training or evaluation set. Use when you are specifying `CUSTOM` or `SEQ` as the value for the `DATA_SPLIT_METHOD` option.
   - `EVALUATION_TASK`: a `STRING` value that specifies the type of task that you want to tune the model to perform. The valid options are:
     - `TEXT_GENERATION`
     - `CLASSIFICATION`
     - `SUMMARIZATION`
     - `QUESTION_ANSWERING`
     - `UNSPECIFIED`

     The default value is
     `UNSPECIFIED`.
   - `INPUT_PROMPT_COL`: a `STRING` value that contains the name of the prompt column in the training data table to use when performing supervised tuning. The default value is `prompt`.
   - `INPUT_LABEL_COLS`: an `ARRAY<<STRING>` value that contains the name of the label column in the training data table to use in supervised tuning. You can only specify one element in the array. The default value is an empty array. This causes `label` to be the default value of the `LABEL_COLUMN` argument.
   - `PROMPT_COLUMN`: the column in the training data table that contains the prompt for evaluating the content in the `LABEL_COLUMN` column. This column must be of `STRING` type or be cast to `STRING`. If you specify a value for the `INPUT_PROMPT_COL` option, you must specify the same value for `PROMPT_COLUMN`. Otherwise this value must be `prompt`. If your table does not have a `prompt` column, use an alias to specify an existing table column. For example, `AS SELECT hint AS prompt, label FROM mydataset.mytable`.
   - `LABEL_COLUMN`: the column in the training data table that contains the examples to train the model with. This column must be of `STRING` type or be cast to `STRING`. If you specify a value for the `INPUT_LABEL_COLS` option, you must specify the same value for `LABEL_COLUMN`. Otherwise this value must be `label`. If your table does not have a `label` column, use an alias to specify an existing table column. For example, `AS SELECT prompt, feature AS label FROM mydataset.mytable`.
   - `TABLE_PROJECT_ID`: the project ID of the project that contains the training data table.
   - `TABLE_DATASET`: the name of the dataset that contains the training data table.
   - `TABLE_NAME`: the name of the table that contains the data to use to train the model.

   <br />

## Evaluate the tuned model

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, run the following query to evaluate the tuned model:

   ```googlesql
   SELECT
   *
   FROM
   ML.EVALUATE(
     MODEL `PROJECT_ID.DATASET_ID.MODEL_NAME`,
     TABLE `TABLE_PROJECT_ID.TABLE_DATASET.TABLE_NAME`,
     STRUCT('TASK_TYPE' AS task_type, TOKENS AS max_output_tokens,
       TEMPERATURE AS temperature, TOP_K AS top_k,
       TOP_P AS top_p));
   ```

   Replace the following:
   - `PROJECT_ID`: the project ID of the project that contains the model.
   - `DATASET_ID`: the ID of the dataset that contains the model.
   - `MODEL_NAME`: the name of the model.
   - `TABLE_PROJECT_ID`: the project ID of the project that contains the evaluation data table.
   - `TABLE_DATASET`: the name of the dataset that contains the evaluation data table.
   - `TABLE_NAME`: the name of the table that contains the evaluation data.

     The table must have a column whose name
     matches the prompt column name that is provided during model training. You
     can provide this value by using the `prompt_col` option during
     model training. If `prompt_col` is unspecified, the column
     named `prompt` in the training data is used. An error is
     returned if there is no column named `prompt`.

     The table
     must have a column whose name matches the label column name that is
     provided during model training. You can provide this value by using the
     `input_label_cols` option during model training. If
     `input_label_cols` is unspecified, the column named
     `label` in the training data is used. An error is returned if
     there is no column named `label`.
   - `TASK_TYPE`: a `STRING` value that specifies the type of task that you want to evaluate the model for. The valid options are:
     - `TEXT_GENERATION`
     - `CLASSIFICATION`
     - `SUMMARIZATION`
     - `QUESTION_ANSWERING`
     - `UNSPECIFIED`
   - `TOKENS`: an `INT64` value that sets the maximum number of tokens that can be generated in the response. This value must be in the range `[1,1024]`. Specify a lower value for shorter responses and a higher value for longer responses. The default is `128`.
   - `TEMPERATURE`: a `FLOAT64` value in the range `[0.0,1.0]` that controls the degree of randomness in token selection. The default is `0`.

     Lower values for `temperature` are good for prompts that
     require a more deterministic and less open-ended or creative response,
     while higher values for `temperature` can lead to more diverse
     or creative results. A value of `0` for
     `temperature` is
     deterministic, meaning that the highest probability response is
     always selected.
   - `TOP_K`: an `INT64` value in the range `[1,40]` that determines the initial pool of tokens the model considers for selection. Specify a lower value for less random responses and a higher value for more random responses. The default is `40`.
   - `TOP_P`: a `FLOAT64` value in the range `[0.0,1.0]` helps determine which tokens from the pool determined by `TOP_K` are selected. Specify a lower value for less random responses and a higher value for more random responses. The default is `0.95`.

   <br />

## Generate text

Generate text with the
[`AI.GENERATE_TEXT` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-generate-text):

### Prompt column

Generate text by using a table column to provide the prompt.

```googlesql
SELECT *
FROM AI.GENERATE_TEXT(
  MODEL `PROJECT_ID.DATASET_ID.MODEL_NAME`,
  TABLE PROJECT_ID.DATASET_ID.TABLE_NAME,
  STRUCT(TOKENS AS max_output_tokens, TEMPERATURE AS temperature,
  TOP_P AS top_p,
  STOP_SEQUENCES AS stop_sequences)
);
```

Replace the following:

- `PROJECT_ID`: your project ID.
- `DATASET_ID`: the ID of the dataset that contains the model.
- `MODEL_NAME`: the name of the model.
- `TABLE_NAME`: the name of the table that contains the prompt. This table must have a column whose name matches the name of the feature column in the tuned model. The feature column name in the model can be set by using the `PROMPT_COL` option when creating the model. Otherwise, the feature column name in the model is `prompt` by default, or you can use an alias to use a differently named column.
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
- `GROUND_WITH_GOOGLE_SEARCH`: a `BOOL` value that determines whether the Vertex AI model uses [Grounding with Google Search](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/grounding/overview#ground-public) when generating responses. Grounding lets the model use additional information from the internet when generating a response, in order to make model responses more specific and factual. When this field is set to `True`, an additional `grounding_result` column is included in the results, providing the sources that the model used to gather additional information. The default is `FALSE`.
- `SAFETY_SETTINGS`: an `ARRAY<STRUCT<STRING AS category, STRING AS threshold>>` value that configures content safety thresholds to filter responses. The first element in the struct specifies a harm category, and the second element in the struct specifies a corresponding blocking threshold. The model filters out content that violate these settings. You can only specify each category once. For example, you can't specify both `STRUCT('HARM_CATEGORY_DANGEROUS_CONTENT' AS category, 'BLOCK_MEDIUM_AND_ABOVE' AS threshold)` and `STRUCT('HARM_CATEGORY_DANGEROUS_CONTENT' AS category, 'BLOCK_ONLY_HIGH' AS threshold)`. If there is no safety setting for a given category, the `BLOCK_MEDIUM_AND_ABOVE` safety setting is used.

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

  - `BLOCK_NONE` ([Restricted](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/multimodal/configure-safety-attributes#how_to_remove_automated_response_blocking_for_select_safety_attributes))
  - `BLOCK_LOW_AND_ABOVE`
  - `BLOCK_MEDIUM_AND_ABOVE` (Default)
  - `BLOCK_ONLY_HIGH`
  - `HARM_BLOCK_THRESHOLD_UNSPECIFIED`

  <br />

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

The following example shows a request with these characteristics:

- Uses the `prompt` column of the `prompts` table for the prompt.
- Returns a short and moderately probable response.
- Returns the generated text and the safety attributes in separate columns.

```googlesql
SELECT *
FROM
  AI.GENERATE_TEXT(
    MODEL `mydataset.mymodel`,
    TABLE mydataset.prompts,
    STRUCT(
      0.4 AS temperature, 100 AS max_output_tokens, 0.5 AS top_p));
```

### Prompt query

Generate text by using a query to provide the prompt.

```googlesql
SELECT *
FROM AI.GENERATE_TEXT(
  MODEL `PROJECT_ID.DATASET_ID.MODEL_NAME`,
  (PROMPT_QUERY),
  STRUCT(TOKENS AS max_output_tokens, TEMPERATURE AS temperature,
  TOP_P AS top_p,
  STOP_SEQUENCES AS stop_sequences)
);
```

Replace the following:

- `PROJECT_ID`: your project ID.
- `DATASET_ID`: the ID of the dataset that contains the model.
- `MODEL_NAME`: the name of the model.
- `PROMPT_QUERY`: a query that provides the prompt data.
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
- `GROUND_WITH_GOOGLE_SEARCH`: a `BOOL` value that determines whether the Vertex AI model uses [Grounding with Google Search](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/grounding/overview#ground-public) when generating responses. Grounding lets the model use additional information from the internet when generating a response, in order to make model responses more specific and factual. When this field is set to `True`, an additional `grounding_result` column is included in the results, providing the sources that the model used to gather additional information. The default is `FALSE`.
- `SAFETY_SETTINGS`: an `ARRAY<STRUCT<STRING AS category, STRING AS threshold>>` value that configures content safety thresholds to filter responses. The first element in the struct specifies a harm category, and the second element in the struct specifies a corresponding blocking threshold. The model filters out content that violate these settings. You can only specify each category once. For example, you can't specify both `STRUCT('HARM_CATEGORY_DANGEROUS_CONTENT' AS category, 'BLOCK_MEDIUM_AND_ABOVE' AS threshold)` and `STRUCT('HARM_CATEGORY_DANGEROUS_CONTENT' AS category, 'BLOCK_ONLY_HIGH' AS threshold)`. If there is no safety setting for a given category, the `BLOCK_MEDIUM_AND_ABOVE` safety setting is used.

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

  - `BLOCK_NONE` ([Restricted](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/multimodal/configure-safety-attributes#how_to_remove_automated_response_blocking_for_select_safety_attributes))
  - `BLOCK_LOW_AND_ABOVE`
  - `BLOCK_MEDIUM_AND_ABOVE` (Default)
  - `BLOCK_ONLY_HIGH`
  - `HARM_BLOCK_THRESHOLD_UNSPECIFIED`

  <br />

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
- Returns a moderately long and more probable response.
- Returns the generated text and the safety attributes in separate columns.

```googlesql
SELECT *
FROM
  AI.GENERATE_TEXT(
    MODEL `mydataset.mymodel`,
    (
      SELECT CONCAT('Summarize this text', body) AS prompt
      FROM mydataset.articles
    ),
    STRUCT(
      0.2 AS temperature, 650 AS max_output_tokens, 0.2 AS top_p));
```

**Example 2**

The following example shows a request with these characteristics:

- Uses a query to create the prompt data by concatenating strings that provide prompt [prefixes](https://docs.cloud.google.com/vertex-ai/docs/generative-ai/text/text-prompts#prompt_structure) with table columns.
- Returns a short and moderately probable response.
- Doesn't return the generated text and the safety attributes in separate columns.

```googlesql
SELECT *
FROM
  AI.GENERATE_TEXT(
    MODEL `mydataset.mytuned_model`,
    (
      SELECT CONCAT(question, 'Text:', description, 'Category') AS prompt
      FROM mydataset.input_table
    ),
    STRUCT(
      0.4 AS temperature, 100 AS max_output_tokens, 0.5 AS top_p));
```