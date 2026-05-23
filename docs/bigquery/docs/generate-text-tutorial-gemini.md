This tutorial shows you how to create a [remote model](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model) that's based on the [`gemini-2.5-flash` model](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/learn/models#gemini-models), and how to use that model with the [`AI.GENERATE_TEXT` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-generate-text) to extract keywords and perform sentiment analysis.

## Costs


In this document, you use the following billable components of Google Cloud:


- [BigQuery ML](https://cloud.google.com/bigquery/pricing#bigquery-ml-pricing). You incur costs for the data that you process in BigQuery.
- [Vertex AI](https://cloud.google.com/vertex-ai/pricing#generative_ai_models). You incur costs for calls to the Vertex AI service that's represented by the remote model.


To generate a cost estimate based on your projected usage,
use the [pricing calculator](https://docs.cloud.google.com/products/calculator).
New Google Cloud users might be eligible for a [free trial](https://docs.cloud.google.com/free).

<br />

When you finish the tasks that are described in this document, you can avoid
continued billing by deleting the resources that you created. For more information, see
[Clean up](https://docs.cloud.google.com/bigquery/docs/generate-text-tutorial-gemini#clean-up).

## Before you begin

### Console


1.

   Make sure that you have the following role or roles on the project:


   **BigQuery Admin** ,
   **Project IAM Admin**

   #### Check for the roles

   1.
      In the Google Cloud console, go to the **IAM** page.

      [Go to IAM](https://console.cloud.google.com/projectselector/iam-admin/iam?supportedpurview=project)
   2. Select the project.
   3.
      In the **Principal** column, find all rows that identify you or a group that
      you're included in. To learn which groups you're included in, contact your
      administrator.

   4. For all rows that specify or include you, check the **Role** column to see whether the list of roles includes the required roles.

   #### Grant the roles

   1.
      In the Google Cloud console, go to the **IAM** page.

      [Go to IAM](https://console.cloud.google.com/projectselector/iam-admin/iam?supportedpurview=project)
   2. Select the project.
   3. Click **Grant access**.
   4.
      In the **New principals** field, enter your user identifier.

      This is typically the email address for a Google Account.

   5. Click **Select a role**, then search for the role.
   6. To grant additional roles, click **Add
      another role** and add each additional role.
   7. Click **Save**.

<br />

### gcloud


1.
   Grant roles to your user account. Run the following command once for each of the following
   IAM roles:
   `
   roles/bigquery.admin,
   roles/resourcemanager.projectIamAdmin
   `

   ```bash
   gcloud projects add-iam-policy-binding PROJECT_ID --member="user:USER_IDENTIFIER" --role=ROLE
   ```

   Replace the following:
   - `PROJECT_ID`: Your project ID.
   - `USER_IDENTIFIER`: The identifier for your user account. For example, `myemail@example.com`.
   - `ROLE`: The IAM role that you grant to your user account.

<br />

## Create a dataset

Create a BigQuery dataset to store your ML model.

### Console

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to the BigQuery page](https://console.cloud.google.com/bigquery)
2. In the **Explorer** pane, click your project name.

3. Click **View actions \> Create dataset**

4. On the **Create dataset** page, do the following:

   - For **Dataset ID** , enter `bqml_tutorial`.

   - For **Location type** , select **Multi-region** , and then select
     **US**.

   - Leave the remaining default settings as they are, and click
     **Create dataset**.

### bq

To create a new dataset, use the
[`bq mk --dataset` command](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#mk-dataset).

1. Create a dataset named `bqml_tutorial` with the data location set to `US`.

   ```
   bq mk --dataset \
     --location=US \
     --description "BigQuery ML tutorial dataset." \
     bqml_tutorial
   ```
2. Confirm that the dataset was created:

   ```bash
   bq ls
   ```

### API

Call the [`datasets.insert`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets/insert)
method with a defined [dataset resource](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets).

<br />

```json
{
  "datasetReference": {
     "datasetId": "bqml_tutorial"
  }
}
```

## Create a connection

Create a
[Cloud resource connection](https://docs.cloud.google.com/bigquery/docs/create-cloud-resource-connection) in
the `US` multiregion, where you created the dataset. Then get the connection's
service account.
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

## Grant permissions to the connection's service account

Grant the connection's service account the Vertex AI User role. You must grant this role in the same project you created or selected in the
[Before you begin](https://docs.cloud.google.com/bigquery/docs/generate-text-tutorial-gemini#before_you_begin) section. Granting the role in a different project results in the error `bqcx-1234567890-xxxx@gcp-sa-bigquery-condel.iam.gserviceaccount.com does not have the permission to access resource`.

To grant the role, follow these steps:

1. Go to the **IAM \& Admin** page.

   [Go to IAM \& Admin](https://console.cloud.google.com/project/_/iam-admin)
2. Click **Grant Access**.

3. In the **New principals** field, enter the service account ID that you
   copied earlier.

4. In the **Select a role** field, choose **Vertex AI** , and then
   select **Vertex AI User role**.

5. Click **Save**.

## Create the remote model

Use the
[`CREATE MODEL`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model)
statement to create a remote model that represents a hosted
Vertex AI model:

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, run the following statement:

```googlesql
CREATE OR REPLACE MODEL `bqml_tutorial.gemini_model`
  REMOTE WITH CONNECTION `LOCATION.CONNECTION_ID`
  OPTIONS (ENDPOINT = 'gemini-2.5-flash');
```

Replace the following:

- `LOCATION`: the connection location
- `CONNECTION_ID`: the ID of your BigQuery connection

  When you [view the connection details](https://docs.cloud.google.com/bigquery/docs/working-with-connections#view-connections)
  in the Google Cloud console, this is the value in the last section of
  the fully qualified connection ID that is shown in
  **Connection ID** , for example
  `projects/myproject/locations/connection_location/connections/*myconnection*`

The query takes several seconds to complete, after which the model
`gemini_model` appears in the `bqml_tutorial` dataset. There are no query results.

## Perform keyword extraction

Perform keyword extraction on [IMDB](https://www.imdb.com/) movie reviews by
using the remote model and the `AI.GENERATE_TEXT` function:

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, enter the following statement to perform keyword
   extraction on five movie reviews:

   ```googlesql
   SELECT
     title, result, review
   FROM
     AI.GENERATE_TEXT(
       MODEL `bqml_tutorial.gemini_model`,
       (
         SELECT
           CONCAT(
             """Extract a list of only 3 key words from this review.
               List only the key words, nothing else. Review: """,
               review) AS prompt,
           *
         FROM
           `bigquery-public-data.imdb.reviews`
         LIMIT 5
       ),
       STRUCT(
         0.2 AS temperature,
         100 AS max_output_tokens));
   ```

   The output is similar to the following:

   ```
   +---+---+---+
   | title        | result           | review                                 |
   +---+---+---+
   | The Guardian | * Costner        | Once again Mr. Costner has dragged out |
   |              | * Kutcher        | a movie for far longer than necessary. |
   |              | * Rescue         | Aside from the terrific sea rescue...  |
   |              |                  |                                        |
   | Trespass     | * Generic        | This is an example of why the majority |
   |              | * Waste          | of action films are the same. Generic  |
   |              | * Cinematography | and boring, there's really nothing...  |
   | ...          | ...              | ...                                    |
   +---+---+---+
   ```

## Perform sentiment analysis

Perform sentiment analysis on [IMDB](https://www.imdb.com/) movie reviews by
using the remote model and the `AI.GENERATE_TEXT` function:

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, run the following statement to perform sentiment
   analysis on movie reviews:

   ```googlesql
   SELECT
     title, result, review
   FROM
     AI.GENERATE_TEXT(
       MODEL `bqml_tutorial.gemini_model`,
       (
         SELECT
           CONCAT(
             """Perform sentiment analysis on the following text and
                return one the following categories: positive, negative: """,
             review) AS prompt,
           *
         FROM
           `bigquery-public-data.imdb.reviews`
         LIMIT 5
       ),
       STRUCT(
         0.2 AS temperature,
         100 AS max_output_tokens));
   ```

   The output is similar to the following:

   ```
   +---+---+---+
   | title    | result   | review                                         |
   +---+---+---+
   | Quitting | Positive | This movie is amazing because the fact that... |
   | Trespass | Negative | This is an example of why the majority of ...  |
   | ...      | ...      | ...                                            |
   +---+---+---+
   ```

## Clean up


To avoid incurring charges to your Google Cloud account for the resources used in this
tutorial, either delete the project that contains the resources, or keep the project and
delete the individual resources.

### Delete the project

> [!CAUTION]
> **Caution** : Deleting a project has the following effects:
>
> - **Everything in the project is deleted.** If you used an existing project for the tasks in this document, when you delete it, you also delete any other work you've done in the project.
> - **Custom project IDs are lost.** When you created this project, you might have created a custom project ID that you want to use in the future. To preserve the URLs that use the project ID, such as an `appspot.com` URL, delete selected resources inside the project instead of deleting the whole project.
>
>
> If you plan to explore multiple architectures, tutorials, or quickstarts, reusing projects
> can help you avoid exceeding project quota limits.

1. Delete a Google Cloud project:

```
gcloud projects delete PROJECT_ID
```

<br />

### Delete individual resources

If you want to reuse the project, then delete the resources that you created
for the tutorial.

### Console

1. Go to the BigQuery page.


   [Go to BigQuery](https://console.cloud.google.com/bigquery)

   <br />

2. Delete the `bqml_tutorial` dataset. Deleting the dataset also deletes
   the remote model.

   1. In the **Explorer** pane, expand your project and click **Datasets**

   2. In the **Datasets** list, click the dataset.

   3. In the details pane, click
      **Delete**.

   4. In the **Delete dataset** dialog, click **Delete**.

3. Delete the connection.

   1. In the **Explorer** pane, expand your project and click **Connections**.

   2. In the **Datasets** list, click the connection.

   3. In the details pane, click
      **Delete**.

   4. In the **Delete connection** dialog, enter `delete` to confirm deletion.

   5. Click **Delete**.

### gcloud

1. Delete the `bqml_tutorial` dataset and the remote model.

       bq rm --dataset --recursive bqml_tutorial

2. Delete the connection.

       bq rm --connection PROJECT_ID.REGION.CONNECTION_ID

   Replace the following:
   - <var translate="no">PROJECT_ID</var>: your Google Cloud project ID
   - <var translate="no">REGION</var>: the connection region
   - <var translate="no">CONNECTION_ID</var>: the connection ID

## What's next

- [Choose a text generation function](https://docs.cloud.google.com/bigquery/docs/choose-text-generation-function)
- [Tune a model using your data](https://docs.cloud.google.com/bigquery/docs/generate-text-tuning)