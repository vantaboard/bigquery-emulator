# Parse PDFs in a retrieval-augmented generation pipeline

This tutorial guides you through the process of creating
a retrieval-augmented generation (RAG) pipeline based on parsed PDF content.

PDF files, such as financial documents, can be challenging to use in RAG
pipelines because of their complex structure and mix of text, figures, and
tables. This tutorial shows you how to use BigQuery ML capabilities in
combination with Document AI's Layout Parser to build a RAG pipeline
based on key information extracted from a PDF file.

You can alternatively perform this tutorial by using a
[Colab Enterprise notebook](https://github.com/GoogleCloudPlatform/generative-ai/blob/main/gemini/use-cases/retrieval-augmented-generation/rag_with_bigquery.ipynb).

## Objectives

This tutorial covers the following tasks:

- Creating a Cloud Storage bucket and uploading a sample PDF file.
- Creating a [Cloud resource connection](https://docs.cloud.google.com/bigquery/docs/create-cloud-resource-connection) so that you can connect to Cloud Storage and Vertex AI from BigQuery.
- Creating an [object table](https://docs.cloud.google.com/bigquery/docs/object-table-introduction) over the PDF file to make the PDF file available in BigQuery.
- [Creating a Document AI processor](https://docs.cloud.google.com/document-ai/docs/create-processor#create-processor) that you can use to parse the PDF file.
- Creating a [remote model](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model-service) that lets you use the Document AI API to access the document processor from BigQuery.
- Using the remote model with the [`ML.PROCESS_DOCUMENT` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-process-document) to parse the PDF contents into chunks and then write that content to a BigQuery table.
- Extracting PDF content from the JSON data returned by the `ML.PROCESS_DOCUMENT` function, and then writing that content to a BigQuery table.
- Creating a [remote model](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model) that lets you use the Vertex AI `text-embedding-004` embedding generation model from BigQuery.
- Using the remote model with the [`AI.GENERATE_EMBEDDING` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-generate-embedding) to generate embeddings from the parsed PDF content, and then writing those embeddings to a BigQuery table. Embeddings are numerical representations of the PDF content that enable you to perform semantic search and retrieval on the PDF content.
- Using the [`VECTOR_SEARCH` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/search_functions#vector_search) on the embeddings to identify semantically similar PDF content.
- Creating a [remote model](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model) that lets you use a Gemini text generation model from BigQuery.
- Perform retrieval-augmented generation (RAG) by using the remote model with the [`AI.GENERATE_TEXT` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-generate-text) to generate text, using vector search results to augment the prompt input and improve results.

## Costs


In this document, you use the following billable components of Google Cloud:


- **BigQuery**: You incur costs for the data that you process in BigQuery.
- **Vertex AI**: You incur costs for calls to Vertex AI models.
- **Document AI**: You incur costs for calls to the Document AI API.
- **Cloud Storage**: You incur costs for object storage in Cloud Storage.


To generate a cost estimate based on your projected usage,
use the [pricing calculator](https://docs.cloud.google.com/products/calculator).
New Google Cloud users might be eligible for a [free trial](https://docs.cloud.google.com/free).

<br />

For more information, see the following pricing pages:

- [BigQuery pricing](https://cloud.google.com/bigquery/pricing)
- [Vertex AI pricing](https://cloud.google.com/vertex-ai/pricing#generative_ai_models)
- [Document AI pricing](https://cloud.google.com/document-ai/pricing)
- [Cloud Storage pricing](https://cloud.google.com/storage/pricing)

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


   Enable the BigQuery, BigQuery Connection, Vertex AI, Document AI, and Cloud Storage APIs.


   **Roles required to enable APIs**


   To enable APIs, you need the Service Usage Admin IAM
   role (`roles/serviceusage.serviceUsageAdmin`), which
   contains the `serviceusage.services.enable` permission. [Learn how to grant
   roles](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).

   [Enable the APIs](https://console.cloud.google.com/flows/enableapi?apiid=bigquery.googleapis.com,bigqueryconnection.googleapis.com,aiplatform.googleapis.com,documentai.googleapis.com,storage.googleapis.com)

## Required roles

To run this tutorial, you need the following Identity and Access Management (IAM)
roles:

- Create Cloud Storage buckets and objects: Storage Admin (`roles/storage.storageAdmin`)
- Create a document processor: Document AI Editor (`roles/documentai.editor`)
- Create and use BigQuery datasets, connections, and models: BigQuery Admin (`roles/bigquery.admin`)
- Grant permissions to the connection's service account: Project IAM Admin (`roles/resourcemanager.projectIamAdmin`)

These predefined roles contain the permissions required to perform the tasks in
this document. To see the exact permissions that are required, expand the
**Required permissions** section:

#### Required permissions

- Create a dataset: `bigquery.datasets.create`
- Create, delegate, and use a connection: `bigquery.connections.*`
- Set the default connection: `bigquery.config.*`
- Set service account permissions: `resourcemanager.projects.getIamPolicy` and `resourcemanager.projects.setIamPolicy`
- Create an object table: `bigquery.tables.create` and `bigquery.tables.update`
- Create Cloud Storage buckets and objects: `storage.buckets.*` and `storage.objects.*`
- Create a model and run inference:
  - `bigquery.jobs.create`
  - `bigquery.models.create`
  - `bigquery.models.getData`
  - `bigquery.models.updateData`
  - `bigquery.models.updateMetadata`
- Create a document processor:
  - `documentai.processors.create`
  - `documentai.processors.update`
  - `documentai.processors.delete`

You might also be able to get these permissions with
[custom roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles) or other
[predefined roles](https://docs.cloud.google.com/iam/docs/roles-permissions).

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
[Cloud resource connection](https://docs.cloud.google.com/bigquery/docs/create-cloud-resource-connection)
and get the connection's service account. Create the connection in
the same [location](https://docs.cloud.google.com/bigquery/docs/locations).

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

## Grant access to the service account

Select one of the following options:

### Console

<br />

1. Go to the **IAM \& Admin** page.

   [Go to IAM \& Admin](https://console.cloud.google.com/project/_/iam-admin)
2. Click **Grant Access**.

   The **Add principals** dialog opens.
3. In the **New principals** field, enter the service account ID that you
   copied earlier.

4. In the **Select a role** field, select **Document AI** , and then
   select **Document AI Viewer**.

5. Click **Add another role**.

6. In the **Select a role** field, select **Cloud Storage** , and then
   select **Storage Object Viewer**.

7. Click **Add another role**.

8. In the **Select a role** field, select **Vertex AI** , and then
   select **Vertex AI User**.

9. Click **Save**.

### gcloud

Use the
[`gcloud projects add-iam-policy-binding` command](https://docs.cloud.google.com/sdk/gcloud/reference/projects/add-iam-policy-binding):

```
gcloud projects add-iam-policy-binding 'PROJECT_NUMBER' --member='serviceAccount:MEMBER' --role='roles/documentai.viewer' --condition=None
gcloud projects add-iam-policy-binding 'PROJECT_NUMBER' --member='serviceAccount:MEMBER' --role='roles/storage.objectViewer' --condition=None
gcloud projects add-iam-policy-binding 'PROJECT_NUMBER' --member='serviceAccount:MEMBER' --role='roles/aiplatform.user' --condition=None
 
```

Replace the following:

- `PROJECT_NUMBER`: your project number.
- `MEMBER`: the service account ID that you copied earlier.

<br />

## Upload the sample PDF to Cloud Storage

To upload the sample PDF to Cloud Storage, follow these steps:

1. Download the `scf23.pdf` sample PDF by going to <https://www.federalreserve.gov/publications/files/scf23.pdf> and clicking download .
2. [Create a Cloud Storage bucket](https://docs.cloud.google.com/storage/docs/creating-buckets).
3. [Upload](https://docs.cloud.google.com/storage/docs/uploading-objects) the `scf23.pdf` file to the bucket.

## Create an object table

Create an object table over the PDF file in Cloud Storage:

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, run the following statement:

   ```googlesql
   CREATE OR REPLACE EXTERNAL TABLE `bqml_tutorial.pdf`
   WITH CONNECTION `LOCATION.CONNECTION_ID`
   OPTIONS(
     object_metadata = 'SIMPLE',
     uris = ['gs://BUCKET/scf23.pdf']);
   ```

   Replace the following:
   - `LOCATION`: the connection location.
   - `CONNECTION_ID`: the ID of your BigQuery connection.

     When you [view the connection details](https://docs.cloud.google.com/bigquery/docs/working-with-connections#view-connections)
     in the Google Cloud console, the `CONNECTION_ID` is the value in the last section of
     the fully qualified connection ID that is shown in
     **Connection ID** , for example
     `projects/myproject/locations/connection_location/connections/*myconnection*`.
   - `BUCKET`: the Cloud Storage bucket containing the `scf23.pdf` file. The full `uri` option value should look similar to `['gs://mybucket/scf23.pdf']`.

## Create a document processor

[Create a document processor](https://docs.cloud.google.com/document-ai/docs/create-processor#create-processor)
based on the [Layout Parser processor](https://docs.cloud.google.com/document-ai/docs/layout-parse-chunk)
in the `us` multi-region.

## Create the remote model for the document processor

Create a remote model to access the Document AI processor:

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, run the following statement:

   ```googlesql
   CREATE OR REPLACE MODEL `bqml_tutorial.parser_model`
   REMOTE WITH CONNECTION `LOCATION.CONNECTION_ID`
     OPTIONS(remote_service_type = 'CLOUD_AI_DOCUMENT_V1', document_processor = 'PROCESSOR_ID');
   ```

   Replace the following:
   - `LOCATION`: the connection location.
   - `CONNECTION_ID`: the ID of your BigQuery connection.

     When you [view the connection details](https://docs.cloud.google.com/bigquery/docs/working-with-connections#view-connections)
     in the Google Cloud console, the `CONNECTION_ID` is the value in the last section of
     the fully qualified connection ID that is shown in
     **Connection ID** , for example
     `projects/myproject/locations/connection_location/connections/*myconnection*`.
   - `PROCESSOR_ID`: the document processor ID. To find this value, [view the processor details](https://docs.cloud.google.com/document-ai/docs/create-processor#get-processor), and then look at the **ID** row in the **Basic Information** section.

## Parse the PDF file into chunks

Use the document processor with the `ML.PROCESS_DOCUMENT` function to parse the
PDF file into chunks, and then write that content to a table. The
`ML.PROCESS_DOCUMENT` function returns the PDF chunks in JSON format.

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, run the following statement:

       CREATE or REPLACE TABLE bqml_tutorial.chunked_pdf AS (
         SELECT * FROM ML.PROCESS_DOCUMENT(
         MODEL bqml_tutorial.parser_model,
         TABLE bqml_tutorial.pdf,
         PROCESS_OPTIONS => (JSON '{"layout_config": {"chunking_config": {"chunk_size": 250}}}')
         )
       );

   <br />

## Parse the PDF chunk data into separate columns

Extract the PDF content and metadata information from the JSON data returned
by the `ML.PROCESS_DOCUMENT` function, and then write that content to a
table:

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, run the following statement to parse the PDF content:

       CREATE OR REPLACE TABLE bqml_tutorial.parsed_pdf AS (
       SELECT
         uri,
         JSON_EXTRACT_SCALAR(json , '$.chunkId') AS id,
         JSON_EXTRACT_SCALAR(json , '$.content') AS content,
         JSON_EXTRACT_SCALAR(json , '$.pageFooters[0].text') AS page_footers_text,
         JSON_EXTRACT_SCALAR(json , '$.pageSpan.pageStart') AS page_span_start,
         JSON_EXTRACT_SCALAR(json , '$.pageSpan.pageEnd') AS page_span_end
       FROM bqml_tutorial.chunked_pdf, UNNEST(JSON_EXTRACT_ARRAY(ml_process_document_result.chunkedDocument.chunks, '$')) json
       );

   <br />

3. In the query editor, run the following statement to view a subset of the
   parsed PDF content:

   ```googlesql
   SELECT *
   FROM `bqml_tutorial.parsed_pdf`
   ORDER BY id
   LIMIT 5;
   ```

   The output is similar to the following:

   ```
   +---+---+---+---+---+---+
   |                uri                |  id  |                                                 content                                              | page_footers_text | page_span_start | page_span_end |
   +---+---+---+---+---+---+
   | gs://mybucket/scf23.pdf           | c1   | •BOARD OF OF FEDERAL GOVERN NOR RESERVE SYSTEM RESEARCH & ANALYSIS                                   | NULL              | 1               | 1             |
   | gs://mybucket/scf23.pdf           | c10  | • In 2022, 20 percent of all families, 14 percent of families in the bottom half of the usual ...    | NULL              | 8               | 9             |
   | gs://mybucket/scf23.pdf           | c100 | The SCF asks multiple questions intended to capture whether families are credit constrained, ...     | NULL              | 48              | 48            |
   | gs://mybucket/scf23.pdf           | c101 | Bankruptcy behavior over the past five years is based on a series of retrospective questions ...     | NULL              | 48              | 48            |
   | gs://mybucket/scf23.pdf           | c102 | # Percentiles of the Distributions of Income and Net Worth                                           | NULL              | 48              | 49            |
   +---+---+---+---+---+---+
    
   ```

## Create the remote model for embedding generation

Create a remote model that represents a hosted Vertex AI
text embedding generation model:

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, run the following statement:

   ```googlesql
   CREATE OR REPLACE MODEL `bqml_tutorial.embedding_model`
     REMOTE WITH CONNECTION `LOCATION.CONNECTION_ID`
     OPTIONS (ENDPOINT = 'text-embedding-005');
   ```

   Replace the following:
   - `LOCATION`: the connection location.
   - `CONNECTION_ID`: the ID of your BigQuery connection.

     When you [view the connection details](https://docs.cloud.google.com/bigquery/docs/working-with-connections#view-connections)
     in the Google Cloud console, the `CONNECTION_ID` is the value in the last section of
     the fully qualified connection ID that is shown in
     **Connection ID** , for example
     `projects/myproject/locations/connection_location/connections/*myconnection*`.

## Generate embeddings

Generate embeddings for the parsed PDF content and then write them to a table:

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, run the following statement:

   ```googlesql
   CREATE OR REPLACE TABLE `bqml_tutorial.embeddings` AS
   SELECT * FROM AI.GENERATE_EMBEDDING(
     MODEL `bqml_tutorial.embedding_model`,
     TABLE `bqml_tutorial.parsed_pdf`
   );
   ```

## Run a vector search

Run a vector search against the parsed PDF content.

The following query takes text input, creates an embedding for that input
using the `AI.GENERATE_EMBEDDING` function, and then uses the `VECTOR_SEARCH`
function to match the input embedding with the most similar PDF content
embeddings. The results are the top ten PDF chunks that are most semantically
similar to the input.

1. Go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, run the following SQL statement:

   ```googlesql
   SELECT query.query, base.id AS pdf_chunk_id, base.content, distance
   FROM
     VECTOR_SEARCH( TABLE `bqml_tutorial.embeddings`,
       'embedding',
       (
       SELECT
         embedding,
         content AS query
       FROM
         AI.GENERATE_EMBEDDING( MODEL `bqml_tutorial.embedding_model`,
           ( SELECT 'Did the typical family net worth increase? If so, by how much?' AS content)
         )
       ),
       top_k => 10,
       OPTIONS => '{"fraction_lists_to_search": 0.01}')
   ORDER BY distance DESC;
   ```

   The output is similar to the following:

   ```
   +---+---+---+---+
   |                query                            | pdf_chunk_id |                                                 content                                              | distance            |
   +---+---+---+---+
   | Did the typical family net worth increase? ,... | c9           | ## Assets                                                                                            | 0.31113668174119469 |
   |                                                 |              |                                                                                                      |                     |
   |                                                 |              | The homeownership rate increased slightly between 2019 and 2022, to 66.1 percent. For ...            |                     |
   +---+---+---+---+
   | Did the typical family net worth increase? ,... | c50          | # Box 3. Net Housing Wealth and Housing Affordability                                                | 0.30973592073929113 |
   |                                                 |              |                                                                                                      |                     |
   |                                                 |              | For families that own their primary residence ...                                                    |                     |
   +---+---+---+---+
   | Did the typical family net worth increase? ,... | c50          | 3 In the 2019 SCF, a small portion of the data collection overlapped with early months of            | 0.29270064592817646 |
   |                                                 |              | the COVID- ...                                                                                       |                     |
   +---+---+---+---+
    
   ```

## Create the remote model for text generation

Create a remote model that represents a hosted Vertex AI
text generation model:

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, run the following statement:

   ```googlesql
   CREATE OR REPLACE MODEL `bqml_tutorial.text_model`
     REMOTE WITH CONNECTION `LOCATION.CONNECTION_ID`
     OPTIONS (ENDPOINT = 'gemini-2.0-flash-001');
   ```

   Replace the following:
   - `LOCATION`: the connection location.
   - `CONNECTION_ID`: the ID of your BigQuery connection.

     When you [view the connection details](https://docs.cloud.google.com/bigquery/docs/working-with-connections#view-connections)
     in the Google Cloud console, the `CONNECTION_ID` is the value in the last section of
     the fully qualified connection ID that is shown in
     **Connection ID** , for example
     `projects/myproject/locations/connection_location/connections/*myconnection*`.

## Generate text augmented by vector search results

Perform a vector search on the embeddings to identify semantically similar
PDF content, and then use the `AI.GENERATE_TEXT` function with the vector
search results to augment the prompt input and improve the text generation
results. In this case, the query uses information from the PDF chunks to answer
a question about the change in family net worth over the past decade.

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, run the following statement:

   ```googlesql
   SELECT
     result AS generated
     FROM
     AI.GENERATE_TEXT( MODEL `bqml_tutorial.text_model`,
       (
       SELECT
       CONCAT( 'Did the typical family net worth change? How does this compare the SCF survey a decade earlier? Be concise and use the following context:',
       STRING_AGG(FORMAT("context: %s and reference: %s", base.content, base.uri), ',\n')) AS prompt,
       FROM
         VECTOR_SEARCH( TABLE
           `bqml_tutorial.embeddings`,
           'embedding',
           (
           SELECT
             embedding,
             content AS query
           FROM
             AI.GENERATE_EMBEDDING( MODEL `bqml_tutorial.embedding_model`,
               (
               SELECT
                 'Did the typical family net worth change? How does this compare the SCF survey a decade earlier?' AS content
               )
             )
           ),
           top_k => 10,
           OPTIONS => '{"fraction_lists_to_search": 0.01}')
         ),
         STRUCT(512 AS max_output_tokens)
     );
   ```

   The output is similar to the following:

   ```
   +---+
   |               generated                                                       |
   +---+
   | Between the 2019 and 2022 Survey of Consumer Finances (SCF), real median      |
   | family net worth surged 37 percent to $192,900, and real mean net worth       |
   | increased 23 percent to $1,063,700.  This represents the largest three-year   |
   | increase in median net worth in the history of the modern SCF, exceeding the  |
   | next largest by more than double.  In contrast, between 2010 and 2013, real   |
   | median net worth decreased 2 percent, and real mean net worth remained        |
   | unchanged.                                                                    |
   +---+
    
   ```

## Clean up

> [!CAUTION]
> **Caution** : Deleting a project has the following effects:
>
> - **Everything in the project is deleted.** If you used an existing project for the tasks in this document, when you delete it, you also delete any other work you've done in the project.
> - **Custom project IDs are lost.** When you created this project, you might have created a custom project ID that you want to use in the future. To preserve the URLs that use the project ID, such as an `appspot.com` URL, delete selected resources inside the project instead of deleting the whole project.
>
>
> If you plan to explore multiple architectures, tutorials, or quickstarts, reusing projects
> can help you avoid exceeding project quota limits.

1. In the Google Cloud console, go to the **Manage resources** page.

   [Go to Manage resources](https://console.cloud.google.com/iam-admin/projects)
2. In the project list, select the project that you want to delete, and then click **Delete**.
3. In the dialog, type the project ID, and then click **Shut down** to delete the project.

<br />