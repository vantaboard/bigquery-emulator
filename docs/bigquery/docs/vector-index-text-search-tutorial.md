# Perform semantic search and retrieval-augmented generation

This tutorial guides you through the end-to-end process of creating
and using
[text embeddings](https://docs.cloud.google.com/bigquery/docs/generative-ai-overview#text_embedding)
for semantic search and
[retrieval-augmented generation (RAG)](https://cloud.google.com/use-cases/retrieval-augmented-generation).

This tutorial covers the following tasks:

- Creating a BigQuery ML [remote model](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model) over a Vertex AI embedding model.
- Using the remote model with the [`AI.GENERATE_EMBEDDING` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-generate-embedding) to generate embeddings from text in a BigQuery table.
- Creating a [vector index](https://docs.cloud.google.com/bigquery/docs/vector-index) to index the embeddings in order to improve search performance.
- Using the [`VECTOR_SEARCH` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/search_functions#vector_search) with the embeddings to search for similar text.
- Perform RAG by generating text with the [`AI.GENERATE_TEXT` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-generate-text), and using vector search results to augment the prompt input and improve results.

This tutorial uses the BigQuery public table
`patents-public-data.google_patents_research.publications`.

## Required roles

To run this tutorial, you need the following Identity and Access Management (IAM)
roles:

- Create and use BigQuery datasets, connections, and models: BigQuery Admin (`roles/bigquery.admin`).
- Grant permissions to the connection's service account: Project IAM Admin (`roles/resourcemanager.projectIamAdmin`).

These predefined roles contain the permissions required to perform the tasks in
this document. To see the exact permissions that are required, expand the
**Required permissions** section:

#### Required permissions

- Create a dataset: `bigquery.datasets.create`
- Create, delegate, and use a connection: `bigquery.connections.*`
- Set the default connection: `bigquery.config.*`
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

## Create the remote model for text embedding generation

Create a remote model that represents a hosted Vertex AI
text embedding generation model:

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, run the following statement:

   ```googlesql
   CREATE OR REPLACE MODEL `bqml_tutorial.embedding_model`
     REMOTE WITH CONNECTION DEFAULT
     OPTIONS (ENDPOINT = 'text-embedding-005');
   ```

   The query takes several seconds to complete, after which the model
   `embedding_model` can be accessed through the **Explorer** pane.
   Because the query uses a `CREATE MODEL` statement to create a model, there
   are no query results.

## Generate text embeddings

Generate text embeddings from patent abstracts using the
[`AI.GENERATE_EMBEDDING` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-generate-embedding),
and then write them to a BigQuery table so that they can be
searched.

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, run the following statement:

   ```googlesql
   CREATE OR REPLACE TABLE `bqml_tutorial.embeddings` AS
   SELECT * FROM AI.GENERATE_EMBEDDING(
     MODEL `bqml_tutorial.embedding_model`,
     (
       SELECT *, abstract AS content
       FROM `patents-public-data.google_patents_research.publications`
       WHERE LENGTH(abstract) > 0 AND LENGTH(title) > 0 AND country = 'Singapore'
     )
   )
   WHERE LENGTH(status) = 0;
   ```

This query takes approximately 5 minutes to complete.

Embedding generation using the
[`AI.GENERATE_EMBEDDING` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-generate-embedding)
might fail due to Vertex AI LLM [quotas](https://docs.cloud.google.com/bigquery/quotas#cloud_ai_service_functions)
or service unavailability. Error details are returned in the
`status` column. An empty `status`
column indicates successful embedding generation.

For alternative text embedding generation methods in BigQuery,
see the
[Embed text with pretrained TensorFlow models tutorial](https://docs.cloud.google.com/bigquery/docs/generate-embedding-with-tensorflow-models).

## Create a vector index

If you create a vector index on an embedding column, a vector search performed
on that column uses the
[Approximate Nearest Neighbor](https://en.wikipedia.org/wiki/Nearest_neighbor_search#Approximation_methods)
search technique. This technique improves vector search performance, with the
trade-off of reducing
[recall](https://developers.google.com/machine-learning/crash-course/classification/precision-and-recall#recallsearch_term_rules)
and so returning more approximate results.

To create a vector index, use the
[`CREATE VECTOR INDEX`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_vector_index_statement)
data definition language (DDL) statement:

1. Go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, run the following SQL statement:

   ```googlesql
   CREATE OR REPLACE VECTOR INDEX my_index
   ON `bqml_tutorial.embeddings`(embedding)
   OPTIONS(index_type = 'IVF',
     distance_type = 'COSINE',
     ivf_options = '{"num_lists":500}')
   ```

Creating a vector index typically takes only a few seconds. It takes another
2 or 3 minutes for the vector index to be populated and ready to use.

### Verify vector index readiness

The vector index is populated asynchronously. You can check whether the index is
ready to be used by querying the
[`INFORMATION_SCHEMA.VECTOR_INDEXES` view](https://docs.cloud.google.com/bigquery/docs/information-schema-vector-indexes)
and verifying that the `coverage_percentage` column value is greater than `0`
and the `last_refresh_time` column value isn't `NULL`.

1. Go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, run the following SQL statement:

   ```googlesql
   SELECT table_name, index_name, index_status,
   coverage_percentage, last_refresh_time, disable_reason
   FROM `PROJECT_ID.bqml_tutorial.INFORMATION_SCHEMA.VECTOR_INDEXES`
   ```

   Replace `PROJECT_ID` with your project ID.

## Perform a text similarity search using the vector index

Use the
[`VECTOR_SEARCH` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/search_functions#vector_search)
to search for relevant patents that match embeddings generated from a
text query.

The `top_k` argument determines the number of matches to return,
in this case five. The `fraction_lists_to_search` option determines the
percentage of vector index lists to search.
[The vector index you created](https://docs.cloud.google.com/bigquery/docs/vector-index-text-search-tutorial#create_a_vector_index) has 500 lists, so
the `fraction_lists_to_search` value of `.01` indicates that this vector search
scans five of those lists. A lower `fraction_lists_to_search` value as shown here
provides lower
[recall](https://developers.google.com/machine-learning/crash-course/classification/accuracy-precision-recall#recall)
and faster performance. For more information about vector index lists, see
the `num_lists`
[vector index option](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#vector_index_option_list).

The model you use to generate the embeddings in this query must be
the same as the one you use to generate the embeddings in the table you are
comparing against, otherwise the search results won't be accurate.

1. Go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, run the following SQL statement:

   ```googlesql
   SELECT query.query, base.publication_number, base.title, base.abstract
   FROM VECTOR_SEARCH(
     TABLE `bqml_tutorial.embeddings`, 'embedding',
     (
     SELECT embedding, content AS query
     FROM AI.GENERATE_EMBEDDING(
     MODEL `bqml_tutorial.embedding_model`,
     (SELECT 'improving password security' AS content))
     ),
     top_k => 5, options => '{"fraction_lists_to_search": 0.01}')
   ```

   The output is similar to the following:

   ```
   +---+---+---+---+
   |            query            | publication_number |                       title                     |                      abstract                   |
   +---+---+---+---+
   | improving password security | SG-120868-A1       | Data storage device security method and a...    | Methods for improving security in data stora... |
   | improving password security | SG-10201610585W-A  | Passsword management system and process...      | PASSSWORD MANAGEMENT SYSTEM AND PROCESS ...     |
   | improving password security | SG-148888-A1       | Improved system and method for...               | IMPROVED SYSTEM AND METHOD FOR RANDOM...        |
   | improving password security | SG-194267-A1       | Method and system for protecting a password...  | A system for providing security for a...        |
   | improving password security | SG-120868-A1       | Data storage device security...                 | Methods for improving security in data...       |
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
     REMOTE WITH CONNECTION DEFAULT
     OPTIONS (ENDPOINT = 'gemini-2.0-flash-001');
   ```

   The query takes several seconds to complete, after which the model
   `text_model` can be accessed through the **Explorer** pane.
   Because the query uses a `CREATE MODEL` statement to create a model, there
   are no query results.

## Generate text augmented by vector search results

Feed the search results as prompts to generate text with the
[`AI.GENERATE_TEXT` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-generate-text)

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, run the following statement:

   ```googlesql
   SELECT result AS generated, prompt
   FROM AI.GENERATE_TEXT(
     MODEL `bqml_tutorial.text_model`,
     (
       SELECT CONCAT(
         'Propose some project ideas to improve user password security using the context below: ',
         STRING_AGG(
           FORMAT("patent title: %s, patent abstract: %s", base.title, base.abstract),
           ',\n')
         ) AS prompt,
       FROM VECTOR_SEARCH(
         TABLE `bqml_tutorial.embeddings`, 'embedding',
         (
           SELECT embedding, content AS query
           FROM AI.GENERATE_EMBEDDING(
             MODEL `bqml_tutorial.embedding_model`,
            (SELECT 'improving password security' AS content)
           )
         ),
       top_k => 5, options => '{"fraction_lists_to_search": 0.01}')
     ),
     STRUCT(600 AS max_output_tokens));
   ```

   The output is similar to the following:

   ```
   +---+---+
   |            generated                           | prompt                                                     |
   +---+---+
   | These patents suggest several project ideas to | Propose some project ideas to improve user password        |
   | improve user password security.  Here are      | security using the context below: patent title: Active     |
   | some, categorized by the patent they build     | new password entry dialog with compact visual indication   |
   | upon:                                          | of adherence to password policy, patent abstract:          |
   |                                                | An active new password entry dialog provides a compact     |
   | **I. Projects based on "Active new password    | visual indication of adherence to password policies. A     |
   | entry dialog with compact visual indication of | visual indication of progress towards meeting all          |
   | adherence to password policy":**               | applicable password policies is included in the display    |
   |                                                | and updated as new password characters are being...        |
   +---+---+
    
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

## What's next

- Try the [Parse PDFs in a retrieval-augmented generation pipeline](https://docs.cloud.google.com/bigquery/docs/rag-pipeline-pdf) tutorial to learn how to create a RAG pipeline based on parsed PDF content.