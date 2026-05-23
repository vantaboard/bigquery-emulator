# Search embeddings with vector search

This tutorial shows you how to perform a
[similarity search](https://wikipedia.org/wiki/Similarity_search) on
embeddings stored in BigQuery tables by using the
[`VECTOR_SEARCH` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/search_functions#vector_search)
and optionally a [vector index](https://docs.cloud.google.com/bigquery/docs/vector-index).

When you use `VECTOR_SEARCH` with a vector index, `VECTOR_SEARCH` uses the
[Approximate Nearest Neighbor](https://en.wikipedia.org/wiki/Nearest_neighbor_search#Approximation_methods)
method to improve vector search performance, with the trade-off of reducing
[recall](https://developers.google.com/machine-learning/crash-course/classification/precision-and-recall#recallsearch_term_rules)
and so returning more approximate results. Without a vector index,
`VECTOR_SEARCH` uses
[brute force search](https://en.wikipedia.org/wiki/Brute-force_search)
to measure distance for every record.

## Required permissions

To run this tutorial, you need the following Identity and Access Management (IAM)
permissions:

- To create a dataset, you need the `bigquery.datasets.create` permission.
- To create a table, you need the following permissions:

  - `bigquery.tables.create`
  - `bigquery.tables.updateData`
  - `bigquery.jobs.create`
- To create a vector index, you need the `bigquery.tables.createIndex`
  permission on the table where you're creating the index.

- To drop a vector index, you need the `bigquery.tables.deleteIndex` permission
  on the table where you're dropping the index.

Each of the following predefined IAM roles includes the
permissions that you need to work with vector indexes:

- BigQuery Data Owner (`roles/bigquery.dataOwner`)
- BigQuery Data Editor (`roles/bigquery.dataEditor`)

## Costs

The `VECTOR_SEARCH` function uses
[BigQuery compute pricing](https://cloud.google.com/bigquery/pricing#analysis_pricing_models).
You are charged for similarity search, using on-demand or editions pricing.

- On-demand: You are charged for the amount of bytes scanned in the base table, the index, and the search query.
- Editions pricing: You are charged for the slots required to complete
  the job within your reservation edition. Larger, more complex
  similarity calculations incur more charges.

  > [!NOTE]
  > **Note:** Using an index isn't supported in [Standard editions](https://docs.cloud.google.com/bigquery/docs/editions-intro).

For more information, see
[BigQuery pricing](https://cloud.google.com/bigquery/pricing).

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


   Enable the BigQuery API.


   **Roles required to enable APIs**


   To enable APIs, you need the Service Usage Admin IAM
   role (`roles/serviceusage.serviceUsageAdmin`), which
   contains the `serviceusage.services.enable` permission. [Learn how to grant
   roles](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).

   [Enable the API](https://console.cloud.google.com/flows/enableapi?apiid=bigquery.googleapis.com)

## Create a dataset

Create a BigQuery dataset:

1. In the Google Cloud console, go to the BigQuery page.

   [Go to the BigQuery page](https://console.cloud.google.com/bigquery)
2. In the **Explorer** pane, click your project name.

3. Click **View actions \> Create dataset**.

   ![Create a dataset to contain the objects used in the tutorial.](https://docs.cloud.google.com/static/bigquery/images/create-dataset.png)
4. On the **Create dataset** page, do the following:

   - For **Dataset ID** , enter `vector_search`.

   - For **Location type** , select **Multi-region** , and then select
     **US (multiple regions in United States)**.

     The public datasets are stored in the `US`
     [multi-region](https://docs.cloud.google.com/bigquery/docs/locations#multi-regions). For simplicity,
     store your dataset in the same location.
   - Leave the remaining default settings as they are, and click
     **Create dataset**.

## Create test tables

1. Create the `patents` table that contains patents embeddings, based on a
   subset of the
   [Google Patents](https://console.cloud.google.com/marketplace/product/google_patents_public_datasets/google-patents-public-data)
   public dataset:

   ```googlesql
   CREATE TABLE vector_search.patents AS
   SELECT * FROM `patents-public-data.google_patents_research.publications`
   WHERE ARRAY_LENGTH(embedding_v1) > 0
    AND publication_number NOT IN ('KR-20180122872-A')
   LIMIT 1000000;
   ```
2. Create the `patents2` table that contains a patent embedding to find
   nearest neighbors for:

   ```googlesql
   CREATE TABLE vector_search.patents2 AS
   SELECT * FROM `patents-public-data.google_patents_research.publications`
   WHERE publication_number = 'KR-20180122872-A';
   ```

## Create a vector index

1. Create the `my_index` vector index on the `embedding_v1` column of the
   `patents` table:

   ```googlesql
   CREATE OR REPLACE VECTOR INDEX my_index ON vector_search.patents(embedding_v1)
   STORING(publication_number, title)
   OPTIONS(distance_type='COSINE', index_type='IVF');
   ```
2. Wait several minutes for the vector index to be created, then run the
   following query and confirm that the `coverage_percentage` value is `100`:

   ```googlesql
   SELECT * FROM vector_search.INFORMATION_SCHEMA.VECTOR_INDEXES;
   ```

## Use the `VECTOR_SEARCH` function with an index

After the vector index is created and populated, use the `VECTOR_SEARCH`
function to find the nearest neighbor for the embedding in the `embedding_v1`
column in the `patents2` table. This query uses the vector index in the search,
so `VECTOR_SEARCH` uses an
[Approximate Nearest Neighbor](https://en.wikipedia.org/wiki/Nearest_neighbor_search#Approximation_methods)
method to find the embedding's nearest neighbor.

> [!NOTE]
> **Note:** Vector indexes are more effective on large datasets. If you want to see this in action, [recreate the `vector_search.patents` table](https://docs.cloud.google.com/bigquery/docs/vector-search#create_test_tables) without the `LIMIT 1000000` clause, [recreate the vector index](https://docs.cloud.google.com/bigquery/docs/vector-search#create_a_vector_index), and then run the following query.

Use the `VECTOR_SEARCH` function with an index:

```googlesql
SELECT query.publication_number AS query_publication_number,
  query.title AS query_title,
  base.publication_number AS base_publication_number,
  base.title AS base_title,
  distance
FROM
  VECTOR_SEARCH(
    TABLE vector_search.patents,
    'embedding_v1',
    TABLE vector_search.patents2,
    top_k => 5,
    distance_type => 'COSINE',
    options => '{"fraction_lists_to_search": 0.005}');
```

The results look similar to the following:

```
+---+---+---+---+---+
| query_publication_number |                         query_title                         | base_publication_number |                                                        base_title                                                        |      distance       |
+---+---+---+---+---+
| KR-20180122872-A         | Rainwater management system based on rainwater keeping unit | CN-106599080-B          | A kind of rapid generation for keeping away big vast transfer figure based on GIS                                        | 0.14471956347590609 |
| KR-20180122872-A         | Rainwater management system based on rainwater keeping unit | CN-114118544-A          | Urban waterlogging detection method and device                                                                           | 0.17472108931171348 |
| KR-20180122872-A         | Rainwater management system based on rainwater keeping unit | KR-20200048143-A        | Method and system for mornitoring dry stream using unmanned aerial vehicle                                               | 0.17561990745619782 |
| KR-20180122872-A         | Rainwater management system based on rainwater keeping unit | KR-101721695-B1         | Urban Climate Impact Assessment method of Reflecting Urban Planning Scenarios and Analysis System using the same         | 0.17696129365559843 |
| KR-20180122872-A         | Rainwater management system based on rainwater keeping unit | CN-109000731-B          | The experimental rig and method that research inlet for stom water chocking-up degree influences water discharged amount | 0.17902723269642917 |
+---+---+---+---+---+
```

## Use the `VECTOR_SEARCH` function with brute force

Use the `VECTOR_SEARCH`
function to find the nearest neighbor for the embedding in the `embedding_v1`
column in the `patents2` table. This query doesn't use the vector index in the
search, so `VECTOR_SEARCH` finds the embedding's exact nearest neighbor.

```googlesql
SELECT query.publication_number AS query_publication_number,
  query.title AS query_title,
  base.publication_number AS base_publication_number,
  base.title AS base_title,
  distance
FROM
  VECTOR_SEARCH(
    TABLE vector_search.patents,
    'embedding_v1',
    TABLE vector_search.patents2,
    top_k => 5,
    distance_type => 'COSINE',
    options => '{"use_brute_force":true}');
```

The results look similar to the following:

```
+---+---+---+---+---+
| query_publication_number |                         query_title                         | base_publication_number |                                                        base_title                                                        |      distance       |
+---+---+---+---+---+
| KR-20180122872-A         | Rainwater management system based on rainwater keeping unit | CN-106599080-B          | A kind of rapid generation for keeping away big vast transfer figure based on GIS                                        |  0.1447195634759062 |
| KR-20180122872-A         | Rainwater management system based on rainwater keeping unit | CN-114118544-A          | Urban waterlogging detection method and device                                                                           |  0.1747210893117136 |
| KR-20180122872-A         | Rainwater management system based on rainwater keeping unit | KR-20200048143-A        | Method and system for mornitoring dry stream using unmanned aerial vehicle                                               | 0.17561990745619782 |
| KR-20180122872-A         | Rainwater management system based on rainwater keeping unit | KR-101721695-B1         | Urban Climate Impact Assessment method of Reflecting Urban Planning Scenarios and Analysis System using the same         | 0.17696129365559843 |
| KR-20180122872-A         | Rainwater management system based on rainwater keeping unit | CN-109000731-B          | The experimental rig and method that research inlet for stom water chocking-up degree influences water discharged amount | 0.17902723269642928 |
+---+---+---+---+---+
```

## Evaluate recall

When you perform a vector search with an index, it returns approximate results,
with the trade-off of reducing
[recall](https://developers.google.com/machine-learning/crash-course/classification/precision-and-recall#recallsearch_term_rules). You can compute recall by
comparing the results returned by vector search with an index and by vector
search with brute force. In this dataset, the `publication_number` value
uniquely identifies a patent, so it is used for comparison.

```googlesql
WITH approx_results AS (
  SELECT query.publication_number AS query_publication_number,
    base.publication_number AS base_publication_number
  FROM
    VECTOR_SEARCH(
      TABLE vector_search.patents,
      'embedding_v1',
      TABLE vector_search.patents2,
      top_k => 5,
      distance_type => 'COSINE',
      options => '{"fraction_lists_to_search": 0.005}')
),
  exact_results AS (
  SELECT query.publication_number AS query_publication_number,
    base.publication_number AS base_publication_number
  FROM
    VECTOR_SEARCH(
      TABLE vector_search.patents,
      'embedding_v1',
      TABLE vector_search.patents2,
      top_k => 5,
      distance_type => 'COSINE',
      options => '{"use_brute_force":true}')
)

SELECT
  a.query_publication_number,
  SUM(CASE WHEN a.base_publication_number = e.base_publication_number THEN 1 ELSE 0 END) / 5 AS recall
FROM exact_results e LEFT JOIN approx_results a
  ON e.query_publication_number = a.query_publication_number
GROUP BY a.query_publication_number
```

If the recall is lower than you would like, you can increase the
`fraction_lists_to_search` value, with the downside of potentially higher
latency and resource usage. To tune your vector search, you can try multiple
runs of `VECTOR_SEARCH` with different argument values, save the results to
tables, and then compare the results.

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