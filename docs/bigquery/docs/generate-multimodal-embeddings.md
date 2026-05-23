# Generate and search multimodal embeddings

This tutorial shows how to generate multimodal embeddings for images
and text using BigQuery and Vertex AI, and then use these
embeddings to perform a text-to-image semantic search.

This tutorial covers the following tasks:

- Creating a [BigQuery object table](https://docs.cloud.google.com/bigquery/docs/object-table-introduction) over image data in a Cloud Storage bucket.
- Exploring the image data by using a [Colab Enterprise notebook in BigQuery](https://docs.cloud.google.com/bigquery/docs/notebooks-introduction).
- Creating a BigQuery ML [remote model](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model) that targets the [Vertex AI `multimodalembedding` foundation model](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/learn/models#foundation_model_apis).
- Using the remote model with the [`AI.GENERATE_EMBEDDING` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-generate-embedding) to generate embeddings from the images in the object table.
- Correct any embedding generation errors.
- Optionally, creating a [vector index](https://docs.cloud.google.com/bigquery/docs/vector-index) to index the image embeddings.
- Creating a text embedding for a given search string.
- Using the [`VECTOR_SEARCH` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/search_functions#vector_search) to perform a semantic search for image embeddings that are similar to the text embedding.
- Visualizing the results by using a notebook.

This tutorial uses the public domain art images from
[The Metropolitan Museum of Art](https://www.metmuseum.org/) that are available
in the public Cloud Storage
[`gcs-public-data--met` bucket](https://console.cloud.google.com/storage/browser/gcs-public-data--met;tab=objects?prefix&forceOnObjectsSortingFiltering=false).

## Required roles

To run this tutorial, you need the following Identity and Access Management (IAM)
roles:

- Create and use BigQuery datasets, connections, models, and notebooks: BigQuery Studio Admin (`roles/bigquery.studioAdmin`).
- Grant permissions to the connection's service account: Project IAM Admin (`roles/resourcemanager.projectIamAdmin`).

These predefined roles contain the permissions required to perform the tasks in
this document. To see the exact permissions that are required, expand the
**Required permissions** section:

#### Required permissions

- Create a dataset: `bigquery.datasets.create`
- Create, delegate, and use a connection: `bigquery.connections.*`
- Set the default connection: `bigquery.config.*`
- Set service account permissions: `resourcemanager.projects.getIamPolicy` and `resourcemanager.projects.setIamPolicy`
- Create an object table: `bigquery.tables.create` and `bigquery.tables.update`
- Create a model and run inference:
  - `bigquery.jobs.create`
  - `bigquery.models.create`
  - `bigquery.models.getData`
  - `bigquery.models.updateData`
  - `bigquery.models.updateMetadata`
- Create and use notebooks:
  - `resourcemanager.projects.get`
  - `resourcemanager.projects.list`
  - `bigquery.config.get`
  - `bigquery.jobs.create`
  - `bigquery.readsessions.create`
  - `bigquery.readsessions.getData`
  - `bigquery.readsessions.update`
  - `dataform.locations.get`
  - `dataform.locations.list`
  - `dataform.repositories.create`  

    Users who have the `dataform.repositories.create` permission can execute code using the default Dataform service account and all permissions granted to that service account. For more information, see [Security considerations for Dataform permissions](https://docs.cloud.google.com/dataform/docs/access-control#security-considerations-permissions).
  - `dataform.repositories.list`
  - `dataform.collections.create`
  - `dataform.collections.list`
  - `aiplatform.notebookRuntimeTemplates.apply`
  - `aiplatform.notebookRuntimeTemplates.get`
  - `aiplatform.notebookRuntimeTemplates.list`
  - `aiplatform.notebookRuntimeTemplates.getIamPolicy`
  - `aiplatform.notebookRuntimes.assign`
  - `aiplatform.notebookRuntimes.get`
  - `aiplatform.notebookRuntimes.list`
  - `aiplatform.operations.list`
  - `aiplatform.notebookRuntimeTemplates.apply`

You might also be able to get these permissions with
[custom roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles) or other
[predefined roles](https://docs.cloud.google.com/iam/docs/roles-overview#predefined).

## Costs


In this document, you use the following billable components of Google Cloud:


- **BigQuery ML**: You incur costs for the data that you process in BigQuery.
- **Vertex AI**: You incur costs for calls to the Vertex AI service that's represented by the remote model.


To generate a cost estimate based on your projected usage,
use the [pricing calculator](https://docs.cloud.google.com/products/calculator).
New Google Cloud users might be eligible for a [free trial](https://docs.cloud.google.com/free).

<br />

For more information about BigQuery pricing, see
[BigQuery pricing](https://cloud.google.com/bigquery/pricing) in
the BigQuery documentation.

For more information about Vertex AI pricing, see the
[Vertex AI pricing](https://cloud.google.com/vertex-ai/pricing#generative_ai_models)
page.

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

## Create the object table

Create an object table over the art images in the public Cloud Storage
[`gcs-public-data--met` bucket](https://console.cloud.google.com/storage/browser/gcs-public-data--met;tab=objects?prefix&forceOnObjectsSortingFiltering=false).
The object table makes it possible to analyze the images without moving them
from Cloud Storage.

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, run the following query:

   ```googlesql
   CREATE OR REPLACE EXTERNAL TABLE `bqml_tutorial.met_images`
   WITH CONNECTION DEFAULT
   OPTIONS
     ( object_metadata = 'SIMPLE',
       uris = ['gs://gcs-public-data--met/*']
     );
   ```

## Explore the image data

Create a [Colab Enterprise notebook](https://docs.cloud.google.com/colab/docs/introduction) in
BigQuery to explore the image data.

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. [Create a notebook by using the BigQuery editor](https://docs.cloud.google.com/bigquery/docs/create-notebooks#create-notebook-console).

3. [Connect the notebook to the default runtime](https://docs.cloud.google.com/bigquery/docs/create-notebooks#connect_to_the_default_runtime).

4. Set up the notebook:

   1. Add a code cell to the notebook.
   2. Copy and paste the following code into the code cell:

          #@title Set up credentials

          from google.colab import auth
          auth.authenticate_user()
          print('Authenticated')

          PROJECT_ID='PROJECT_ID'
          from google.cloud import https://docs.cloud.google.com/python/docs/reference/bigquery/latest
          client = https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html(PROJECT_ID)

      Replace `PROJECT_ID` with the name of the project
      that you are using for this tutorial.
   3. Run the code cell.

5. Enable table display:

   1. Add a code cell to the notebook.
   2. Copy and paste the following code into the code cell:

          #@title Enable data table display
          %load_ext google.colab.data_table

   3. Run the code cell.

6. Create a function to display the images:

   1. Add a code cell to the notebook.
   2. Copy and paste the following code into the code cell:

          #@title Util function to display images
          import io
          from PIL import Image
          import matplotlib.pyplot as plt
          import tensorflow as tf

          def printImages(results):
           image_results_list = list(results)
           amt_of_images = len(image_results_list)

           fig, axes = plt.subplots(nrows=amt_of_images, ncols=2, figsize=(20, 20))
           fig.tight_layout()
           fig.subplots_adjust(hspace=0.5)
           for i in range(amt_of_images):
             gcs_uri = image_results_list[i][0]
             text = image_results_list[i][1]
             f = tf.io.gfile.GFile(gcs_uri, 'rb')
             stream = io.BytesIO(f.read())
             img = Image.open(stream)
             axes[i, 0].axis('off')
             axes[i, 0].imshow(img)
             axes[i, 1].axis('off')
             axes[i, 1].text(0, 0, text, fontsize=10)
           plt.show()

   3. Run the code cell.

7. Display the images:

   1. Add a code cell to the notebook.
   2. Copy and paste the following code into the code cell:

          #@title Display Met images

          inspect_obj_table_query = """
          SELECT uri, content_type
          FROM bqml_tutorial.met_images
          WHERE content_type = 'image/jpeg'
          Order by uri
          LIMIT 10;
          """
          printImages(client.query(inspect_obj_table_query))

   3. Run the code cell.

      The results should look similar to the following:

      ![Images showing objects from the Metropolitan Museum of Art.](https://docs.cloud.google.com/static/bigquery/images/met-images.png)
8. Save the notebook as `met-image-analysis`.

## Create the remote model

Create a remote model that represents a hosted Vertex AI
multimodal embedding model:

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, run the following query:

   ```googlesql
   CREATE OR REPLACE MODEL `bqml_tutorial.multimodal_embedding_model`
     REMOTE WITH CONNECTION DEFAULT
     OPTIONS (ENDPOINT = 'gemini-embedding-2-preview');
   ```

   The query takes several seconds to complete, after which you can access the
   `multimodal_embedding_model` model that appears in the `bqml_tutorial`
   dataset. Because the query uses a `CREATE MODEL` statement to create a model, there are no query results.

## Generate image embeddings

Generate embeddings from the images in the object table by using the
[`AI.GENERATE_EMBEDDING` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-generate-embedding),
and then write them to a table for
use in a following step. Embedding generation is an expensive operation, so the
query uses a subquery including the `LIMIT` clause to limit embedding generation to 10,000 images
instead of embedding the full dataset of 601,294 images. This also helps keep
the number of images under the 25,000 limit for the `AI.GENERATE_EMBEDDING`
function. This query takes approximately 40 minutes to run.

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, run the following query:

   ```googlesql
   CREATE OR REPLACE TABLE `bqml_tutorial.met_image_embeddings`
   AS
   SELECT *
   FROM
     AI.GENERATE_EMBEDDING(
       MODEL `bqml_tutorial.multimodal_embedding_model`,
       (SELECT * FROM `bqml_tutorial.met_images` WHERE content_type = 'image/jpeg' LIMIT 10000))
   ```

## Correct any embedding generation errors

Check for and correct any embedding generation errors. Embedding generation
can fail because of
[Generative AI on Vertex AI quotas](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/quotas)
or service unavailability.

The `AI.GENERATE_EMBEDDING` function returns error details in the
`status` column. This column is empty if embedding
generation was successful, or contains an error message if embedding
generation failed.

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, run the following query to see if there were any
   embedding generation failures:

   ```googlesql
   SELECT DISTINCT(status),
     COUNT(uri) AS num_rows
   FROM bqml_tutorial.met_image_embeddings
   GROUP BY 1;
   ```
3. If rows with errors are returned, drop any rows where embedding generation
   failed:

   ```googlesql
   DELETE FROM `bqml_tutorial.met_image_embeddings`
   WHERE status = 'A retryable error occurred: RESOURCE_EXHAUSTED error from remote service/endpoint.';
   ```

## Create a vector index

You can optionally use the
[`CREATE VECTOR INDEX` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_vector_index_statement)
to create the `met_images_index` vector index on the
`embedding` column of the `met_images_embeddings` table.
A vector index lets you perform a vector search more quickly, with the
trade-off of reducing recall and so returning more approximate results.

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, run the following query:

   ```googlesql
   CREATE OR REPLACE
     VECTOR INDEX `met_images_index`
   ON
     bqml_tutorial.met_image_embeddings(embedding)
     OPTIONS (
       index_type = 'IVF',
       distance_type = 'COSINE');
   ```
3. The vector index is created asynchronously. To check if the vector index
   has been created, query the
   [`INFORMATION_SCHEMA.VECTOR_INDEXES` view](https://docs.cloud.google.com/bigquery/docs/information-schema-vector-indexes)
   and confirm that the `coverage_percentage` value is greater than `0`, and the
   `last_refresh_time` value isn't `NULL`:

   ```googlesql
   SELECT table_name, index_name, index_status,
     coverage_percentage, last_refresh_time, disable_reason
   FROM bqml_tutorial.INFORMATION_SCHEMA.VECTOR_INDEXES
   WHERE index_name = 'met_images_index';
   ```

## Generate an embedding for the search text

To search images that correspond to a specified text search string, you must
first create a text embedding for that string. Use the same remote model to
create the text embedding that you used to create the image embeddings,
and then write the text embedding to a table for use in a following step. The
search string is `pictures of white or cream colored dress from victorian era`.

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, run the following query:

   ```googlesql
   CREATE OR REPLACE TABLE `bqml_tutorial.search_embedding`
   AS
   SELECT * FROM AI.GENERATE_EMBEDDING(
     MODEL `bqml_tutorial.multimodal_embedding_model`,
     (
       SELECT 'pictures of white or cream colored dress from victorian era' AS content
     )
   );
   ```

## Perform a text-to-image semantic search

Use the
[`VECTOR_SEARCH` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/search_functions#vector_search)
to perform a semantic search for images that best correspond to the search
string represented by the text embedding.

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, run the following query to perform a semantic
   search and write the results to a table:

   ```googlesql
   CREATE OR REPLACE TABLE `bqml_tutorial.vector_search_results` AS
   SELECT base.uri AS gcs_uri, distance
   FROM
     VECTOR_SEARCH(
       TABLE `bqml_tutorial.met_image_embeddings`,
       'embedding',
       TABLE `bqml_tutorial.search_embedding`,
       'embedding',
       top_k => 3);
   ```

## Visualize the semantic search results

Visualize the semantic search results by using a notebook.

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. Open the `met-image-analysis` notebook that you created earlier.

3. Visualize the vector search results:

   1. Add a code cell to the notebook.
   2. Copy and paste the following code into the code cell:

          query = """
            SELECT * FROM `bqml_tutorial.vector_search_results`
            ORDER BY distance;
          """

          printImages(client.query(query))

   3. Run the code cell.

      The results should look similar to the following:

      ![Returned images from a multimodal vector search query.](https://docs.cloud.google.com/static/bigquery/images/met-image-vector-search.png)

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