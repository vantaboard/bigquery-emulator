# Generate text embeddings by using an open model and the AI.GENERATE_EMBEDDING function

This tutorial shows you how to create a
[remote model](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model-open)
that's based on the
open-source text embedding model [Qwen3-Embedding-0.6B](https://huggingface.co/Qwen/Qwen3-Embedding-0.6B),
and then how to use that model with the
[`AI.GENERATE_EMBEDDING` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-generate-embedding)
to embed movie reviews from the `bigquery-public-data.imdb.reviews` public table.

## Required permissions

To run this tutorial, you need the following Identity and Access Management (IAM)
roles:

- Create and use BigQuery datasets, connections, and models: BigQuery Admin (`roles/bigquery.admin`).
- Grant permissions to the connection's service account: Project IAM Admin (`roles/resourcemanager.projectIamAdmin`).
- Deploy and undeploy models in Vertex AI: Vertex AI Administrator (`roles/aiplatform.admin`).

These predefined roles contain the permissions required to perform the tasks in
this document. To see the exact permissions that are required, expand the
**Required permissions** section:

#### Required permissions

- Create a dataset: `bigquery.datasets.create`
- Create, delegate, and use a connection: `bigquery.connections.*`
- Set the default connection: `bigquery.config.*`
- Set service account permissions: `resourcemanager.projects.getIamPolicy` and `resourcemanager.projects.setIamPolicy`
- Deploy and undeploy a Vertex AI model:
  - `aiplatform.endpoints.deploy`
  - `aiplatform.endpoints.undeploy`
- Create a model and run inference:
  - `bigquery.jobs.create`
  - `bigquery.models.create`
  - `bigquery.models.getData`
  - `bigquery.models.updateData`
  - `bigquery.models.updateMetadata`

You might also be able to get these permissions with
[custom roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles) or other
[predefined roles](https://docs.cloud.google.com/iam/docs/roles-overview#predefined).

## Costs


In this document, you use the following billable components of Google Cloud:


- **BigQuery ML**: You incur costs for the data that you process in BigQuery.
- **Vertex AI**: You incur costs for calls to the Vertex AI model that's represented by the remote model.


To generate a cost estimate based on your projected usage,
use the [pricing calculator](https://docs.cloud.google.com/products/calculator).
New Google Cloud users might be eligible for a [free trial](https://docs.cloud.google.com/free).

<br />

For more information about BigQuery pricing, see
[BigQuery pricing](https://cloud.google.com/bigquery/pricing) in
the BigQuery documentation.

Open models that you deploy to Vertex AI are charged per
machine-hour. This means billing starts as soon as the endpoint is fully set
up, and continues until you undeploy it.
For more information about Vertex AI pricing, see the
[Vertex AI pricing](https://cloud.google.com/vertex-ai/pricing#prediction-prices)
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

## Create the remote model

Create a remote model that represents a hosted Vertex AI
model:

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, run the following statement:

```sql
CREATE OR REPLACE MODEL `bqml_tutorial.qwen3_embedding_model`
  REMOTE WITH CONNECTION DEFAULT
  OPTIONS (
    HUGGING_FACE_MODEL_ID = 'Qwen/Qwen3-Embedding-0.6B'
);
```

The query takes up to 20 minutes to complete, after which the
`qwen3_embedding_model` model appears in the `bqml_tutorial` dataset in the
**Explorer** pane. Because the query uses a `CREATE MODEL` statement to create a
model, there are no query results.

## Perform text embedding

Perform text embedding on [IMDB](https://www.imdb.com/) movie reviews by
using the remote model and the `AI.GENERATE_EMBEDDING` function:

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, enter the following statement to perform text embedding on five movie reviews:

   ```sql
   SELECT
     *
   FROM
     AI.GENERATE_EMBEDDING(
       MODEL `bqml_tutorial.qwen3_embedding_model`,
       (
         SELECT
           review AS content,
           *
         FROM
           `bigquery-public-data.imdb.reviews`
         LIMIT 5
       )
     );
   ```

   The results include the following columns:
   - `embedding`: an array of double to represent the generated embeddings.
   - `status`: the API response status for the corresponding row. If the operation was successful, this value is empty.
   - `content`: the input text from which to extract embeddings.
   - All of the columns from the `bigquery-public-data.imdb.reviews` table.

## Undeploy model

If you choose not to [delete your project as recommended](https://docs.cloud.google.com/bigquery/docs/generate-text-embedding-tutorial-open-models#clean_up), you must
undeploy the Qwen3 embedding model in Vertex AI to avoid
continued billing for it. BigQuery automatically undeploys the
model after a specified period of idleness (6.5 hours by default).
Alternatively, you can immediately undeploy the model by using the
[`ALTER MODEL` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-alter-model),
as shown in the following example:

```sql
ALTER MODEL `bqml_tutorial.qwen3_embedding_model`
SET OPTIONS (deploy_model = false);
```

For more information, see
[Automatic or immediate open model undeployment](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model-open#managed-model-undeployment).

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

## What's next

- Learn how to [use text embeddings for semantic search and retrieval-augmented generation (RAG)](https://docs.cloud.google.com/bigquery/docs/vector-index-text-search-tutorial).