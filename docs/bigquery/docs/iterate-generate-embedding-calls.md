# Handle quota errors by calling ML.GENERATE_EMBEDDING iteratively

This tutorial shows you how to use the BigQuery
`bqutil.procedure.bqml_generate_embeddings` public stored procedure to iterate
through calls to the
[`ML.GENERATE_EMBEDDING` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-generate-embedding).
Calling the function iteratively lets you address any retryable errors that
occur due to exceeding the
[quotas and limits](https://docs.cloud.google.com/bigquery/quotas#cloud_ai_service_functions) that apply to
the function.

To review the source code for the `bqutil.procedure.bqml_generate_embeddings`
stored procedure in GitHub, see
[`bqml_generate_embeddings.sqlx`](https://github.com/GoogleCloudPlatform/bigquery-utils/blob/master/stored_procedures/definitions/bqml_generate_embeddings.sqlx).
For more information about the stored procedure parameters and usage, see the
[README file](https://github.com/GoogleCloudPlatform/bigquery-utils/blob/master/stored_procedures/README.md#bqml_generate_embeddings-source_table-string-target_table-string-ml_model-string-content_column-string-key_columns-array-options_string-string).

This tutorial guides you through the following tasks:

- Creating a [remote model over a `text-embedding-005` model](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model).
- Iterating through calls to the `ML.GENERATE_EMBEDDING` function, using the remote model and the `bigquery-public-data.bbc_news.fulltext` public data table with the `bqutil.procedure.bqml_generate_embeddings` stored procedure.

## Required permissions

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
- **Vertex AI**: You incur costs for calls to the Vertex AI model.


To generate a cost estimate based on your projected usage,
use the [pricing calculator](https://docs.cloud.google.com/products/calculator).
New Google Cloud users might be eligible for a [free trial](https://docs.cloud.google.com/free).

<br />

For more information about BigQuery pricing, see
[BigQuery pricing](https://cloud.google.com/bigquery/pricing).

For more information about Vertex AI pricing, see
[Vertex AI pricing](https://docs.cloud.google.com/vertex-ai/generative-ai/pricing).

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

Create a BigQuery dataset to store your models and sample data:

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to the **BigQuery** page](https://console.cloud.google.com/bigquery)
2. In the **Explorer** pane, click your project name.

3. Click **View actions \> Create dataset**.

4. On the **Create dataset** page, do the following:

   1. For **Dataset ID** , enter `target_dataset`.

   2. For **Location type** , select **Multi-region** , and then select
      **US (multiple regions in United States)**.

   3. Leave the remaining default settings as they are, and click
      **Create dataset**.

## Create the text embedding generation model

Create a remote model that represents a hosted Vertex AI
`text-embedding-005` model:

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, run the following statement:

   ```googlesql
   CREATE OR REPLACE MODEL `target_dataset.embedding_model`
     REMOTE WITH CONNECTION DEFAULT
     OPTIONS (ENDPOINT = 'text-embedding-005');
   ```

   The query takes several seconds to complete, after which the
   `embedding` model appears in the `sample` dataset in the **Explorer** pane.
   Because the query uses a `CREATE MODEL` statement to create a model, there
   are no query results.

## Run the stored procedure

Run the `bqutil.procedure.bqml_generate_embeddings` stored procedure, which
iterates through calls to the `ML.GENERATE_EMBEDDING` function
using the `target_dataset.embedding_model` model and the
`bigquery-public-data.bbc_news.fulltext` public data table:

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, run the following statement:

   ```googlesql
   CALL `bqutil.procedure.bqml_generate_embeddings`(
       "bigquery-public-data.bbc_news.fulltext",            -- source table
       "PROJECT_ID.target_dataset.news_body_embeddings",  -- destination table
       "PROJECT_ID.target_dataset.embedding_model",       -- model
       "body",                                              -- content column
       ["filename"],                                        -- key columns
       '{}'                                                 -- optional arguments encoded as a JSON string
   );
   ```

   Replace `PROJECT_ID` with the project ID of the
   project you are using for this tutorial.

   The stored procedure creates a `target_dataset.news_body_embeddings` table
   to contain the output of the `ML.GENERATE_EMBEDDING` function.
3. When the query is finished running, confirm that there are no rows
   in the `target_dataset.news_body_embeddings` table that contain a retryable
   error. In the query editor, run the following statement:

   ```googlesql
   SELECT *
   FROM `target_dataset.news_body_embeddings`
   WHERE ml_generate_embedding_status LIKE '%A retryable error occurred%';
   ```

   The query returns the message `No data to display`.

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