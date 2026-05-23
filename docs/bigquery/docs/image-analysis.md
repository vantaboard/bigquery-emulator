# Analyze images

This tutorial shows you how to gain insights from unstructured image data by integrating BigQuery ML with Gemini. In the tutorial, you
create a
[remote model](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model)
based on
[gemini-2.5-flash](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/learn/models#gemini-models)
and use the
[`AI.GENERATE_TEXT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-generate-text)
function to automatically extract metadata, such as titles and release years,
from a collection of movie posters.

## Objectives

- Create a [BigQuery object table](https://docs.cloud.google.com/bigquery/docs/object-table-introduction) over image data in a Cloud Storage bucket.
- Create a BigQuery ML remote model that targets the Vertex AI `gemini-2.5-flash` model.
- Use the remote model with the `AI.GENERATE_TEXT` function to identify the movies associated with a set of movie posters.

## Costs

This tutorial uses the following billable components of Google Cloud:

- [BigQuery ML](https://cloud.google.com/bigquery/pricing)
- [Vertex AI](https://docs.cloud.google.com/vertex-ai/generative-ai/pricing)

To generate a cost estimate based on your projected usage, use the
[pricing calculator](https://cloud.google.com/products/calculator).

When you finish the tasks that are described in this document, you can avoid
continued billing by deleting the resources that you created. For more information, see
[Clean up](https://docs.cloud.google.com/bigquery/docs/image-analysis#clean-up).

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

### Required roles

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
- Create an object table: `bigquery.tables.create` and `bigquery.tables.update`
- Create a model and run inference:
  - `bigquery.jobs.create`
  - `bigquery.models.create`
  - `bigquery.models.getData`
  - `bigquery.models.updateData`
  - `bigquery.models.updateMetadata`

You might also be able to get these permissions with
[custom roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles) or other
[predefined roles](https://docs.cloud.google.com/iam/docs/roles-overview#predefined).

## Prepare the environment

To perform BigQuery ML inference on object tables using
[gemini-2.5-flash](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/learn/models#gemini-models),
you must assign a BigQuery reservation to your
project. If there's already a reservation assigned to your project, you can skip
this step.

### Create a reservation

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the navigation menu, click **Capacity management**.

3. Click **Create Reservation**.

4. On the **Create reservation** page, do the following:

   1. For **Reservation name** , enter `bqml-tutorial-reservation`.
   2. For **Location** , select **us (multiple regions in United States)**.
   3. Leave the remaining default settings as they are, and click **Save**.

### Assign the reservation

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the navigation menu, click **Capacity management**.

3. In the **Slot Reservations** table, find the reservation that you want to assign to your project.

4. Click **View actions \> Create assignment**.

5. In **Create an assignment** , click **Browse** and select your project.

6. For **Job type** , select **QUERY**. This selection ensures that your SQL
   queries use this reservation's slots.

7. Click **Create**.

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

Create an object table over the movie poster images in the public
Cloud Storage [bucket](https://console.cloud.google.com/storage/browser/cloud-samples-data/vertex-ai/dataset-management/datasets/classic-movie-posters;tab=objects?prefix&forceOnObjectsSortingFiltering=false).
The object table lets you analyze the images without moving them
from Cloud Storage.

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, run the following query to create the object table:

   ```googlesql
   CREATE OR REPLACE EXTERNAL TABLE `bqml_tutorial.movie_posters`
     WITH CONNECTION DEFAULT
     OPTIONS (
       object_metadata = 'SIMPLE',
       uris =
         ['gs://cloud-samples-data/vertex-ai/dataset-management/datasets/classic-movie-posters/*']);
   ```

## Create the remote model

Create a remote model that represents a Vertex AI
`gemini-2.5-flash` model:

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, run the following query to create the remote model:

   ```googlesql
   CREATE OR REPLACE MODEL `bqml_tutorial.gemini-vision`
     REMOTE WITH CONNECTION DEFAULT
     OPTIONS (ENDPOINT = 'gemini-2.5-flash');
   ```

   The query might take a few minutes to complete, after which the
   `gemini-vision` model appears in the `bqml_tutorial` dataset in the
   **Explorer** pane.
   Because the query uses a `CREATE MODEL` statement to create a model, there
   are no query results.

## Analyze the movie posters

Use the remote model to analyze the movie posters and determine what movie each
poster represents, and then write this data to a table.

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, run the following query to analyze the movie poster
   images:

   ```googlesql
   CREATE OR REPLACE TABLE
     `bqml_tutorial.movie_posters_results` AS (
     SELECT
       uri,
      result
     FROM
       AI.GENERATE_TEXT( MODEL `bqml_tutorial.gemini-vision`,
         TABLE `bqml_tutorial.movie_posters`,
         STRUCT( 0.2 AS temperature,
           'For the movie represented by this poster, what is the movie title and year of release? Answer in JSON format with two keys: title, year. title should be string, year should be integer.' AS PROMPT)));
       
   ```
3. In the query editor, run the following statement to view the table data:

   ```googlesql
   SELECT * FROM `bqml_tutorial.movie_posters_results`;
   ```

   The output is similar to the following:

       +---+---+
       | uri                                        | result                           |
       +---+---+
       | gs://cloud-samples-data/vertex-ai/dataset- | json                          |
       | management/datasets/classic-movie-         | {                                |
       | posters/little_annie_rooney.jpg            |  "title": "Little Annie Rooney", |
       |                                            |  "year": 1912                    |
       |                                            | }                                |
       |                                            |                              |
       +---+---+
       | gs://cloud-samples-data/vertex-ai/dataset- | json                          |
       | management/datasets/classic-movie-         | {                                |
       | posters/mighty_like_a_mouse.jpg            |  "title": "Mighty Like a Moose", |
       |                                            |  "year": 1926                    |
       |                                            | }                                |
       |                                            |                              |
       +---+---+
       | gs://cloud-samples-data/vertex-ai/dataset- | json                          |
       | management/datasets/classic-movie-         | {                                |
       | posters/brown_of_harvard.jpeg              |  "title": "Brown of Harvard",    |
       |                                            |  "year": 1926                    |
       |                                            | }                                |
       |                                            |                              |
       +---+---+

   <br />

## Format the model output

To make the movie title and year data easier to read, format the data returned
by the model.

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, run the following query to format the data:

   ````googlesql
   CREATE OR REPLACE TABLE
     `bqml_tutorial.movie_posters_results_formatted` AS (
     SELECT
       uri,
       JSON_QUERY(RTRIM(LTRIM(results.result, " ```json"), "```"), "$.title") AS title,
       JSON_QUERY(RTRIM(LTRIM(results.result, " ```json"), "```"), "$.year") AS year
     FROM
       `bqml_tutorial.movie_posters_results` results );
   ````
3. In the query editor, run the following statement to view the table data:

   ```googlesql
   SELECT * FROM `bqml_tutorial.movie_posters_results_formatted`;
   ```

   The output is similar to the following:

   ```
   +---+---+---+
   | uri                                        | title                      | year |
   +---+---+---+
   | gs://cloud-samples-data/vertex-ai/dataset- | "Barque sortant du port"   | 1895 |
   | management/datasets/classic-movie-         |                            |      |
   | posters/barque_sortant_du_port.jpeg        |                            |      |
   +---+---+---+
   | gs://cloud-samples-data/vertex-ai/dataset- | "The Great Train Robbery"  | 1903 |
   | management/datasets/classic-movie-         |                            |      |
   | posters/the_great_train_robbery.jpg        |                            |      |
   +---+---+---+
   | gs://cloud-samples-data/vertex-ai/dataset- | "Little Annie Rooney"      | 1912 |
   | management/datasets/classic-movie-         |                            |      |
   | posters/little_annie_rooney.jpg            |                            |      |
   +---+---+---+
   ```

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

1. In the Google Cloud console, go to the **Manage resources** page.

   [Go to Manage resources](https://console.cloud.google.com/iam-admin/projects)
2. In the project list, select the project that you want to delete, and then click **Delete**.
3. In the dialog, type the project ID, and then click **Shut down** to delete the project.

<br />

### Delete individual resources

If you want to reuse the project, then delete the resources that you created for
the tutorial.

#### Delete the dataset

### Console

Delete the entire `bqml_tutorial` dataset and all its contents by running the
following SQL command:

```googlesql
DROP SCHEMA IF EXISTS `bqml_tutorial` CASCADE;
```

### bq

Delete the entire `bqml_tutorial` dataset and all its contents:

```
bq rm -r bqml_tutorial
```

#### Delete the reservation

### Console

If you created a BigQuery reservation as part of this tutorial,
you should remove it to avoid continued slot charges.

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the navigation menu, click **Capacity management**.

3. In the **Slot Reservations** table, find **`bqml-tutorial-reservation`**.

4. Click **View actions** \> **Delete**.

### bq

If you created a BigQuery reservation named
`bqml-tutorial-reservation` in the `us` location, use the following command to
remove it:

```
bq rm --reservation --location=us bqml-tutorial-reservation
```

#### Delete the connection

### Console

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click
   **Explorer** , locate your project, and then click **Connections**.

3. In the table, find your connection.

4. Click **View actions \> Delete**.

### bq

Delete the connection:

```
bq rm --connection --location=us CONNECTION_ID
```

Replace
<var translate="no">CONNECTION_ID</var> with the actual ID of your connection.

## What's next

- Learn more about [generative AI functions in BigQuery](https://docs.cloud.google.com/bigquery/docs/generative-ai-overview).
- Learn how to [tune a model using your data](https://docs.cloud.google.com/bigquery/docs/generate-text-tuning).
- Explore reference architectures, diagrams, and best practices about Google Cloud. Take a look at our [Cloud Architecture Center](https://docs.cloud.google.com/architecture).