# Autonomous embedding generation

> [!WARNING]
>
> **Preview**
>
>
> This feature is
>
> subject to the "Pre-GA Offerings Terms" in the General Service Terms section of the
> [Service Specific
> Terms](https://docs.cloud.google.com/terms/service-terms#1).
>
> Pre-GA features are available "as is" and might have limited support.
>
> For more information, see the
> [launch stage descriptions](https://cloud.google.com/products/#product-launch-stages).

> [!NOTE]
> **Note:** To give feedback or request support for this feature, contact [bq-vector-search@google.com](mailto:bq-vector-search@google.com)

This document describes how to use autonomous embedding generation for your
data, which lets BigQuery maintain a column of embeddings on a table
based on a source column. When you add or modify data in the source column,
BigQuery automatically generates or updates the embedding
column for that data by using a Vertex AI embedding model.
This is helpful if you want to let BigQuery maintain your
embeddings when your source data is updated regularly.

Embeddings are useful for modern generative AI
applications such as Retrieval Augmented Generation (RAG), but they can be
complex to create, manage, and query. You can use
autonomous embedding generation to simplify the process of creating,
maintaining, and querying embeddings for use in similarity searches and other
generative AI applications.

For example, you can use queries similar to the following to
create a table with autonomous embedding generation
enabled, insert data, and then perform semantic search:

    CREATE TABLE mydataset.products (
      name STRING,
      description STRING,
      description_embedding STRUCT<result ARRAY<FLOAT64>, status STRING>
        GENERATED ALWAYS AS (
          AI.EMBED(description, connection_id => 'us.example_connection',
            endpoint => 'text-embedding-005')
          # Alternatively, you can use the syntax for a built-in model.
          # AI.EMBED(description, model => 'embeddinggemma-300m')
        ) STORED OPTIONS( asynchronous = TRUE ));

    # Values in the description_embedding column are automatically generated.
    INSERT INTO mydataset.products (name, description) VALUES
      ('Super slingers', 'An exciting board game for the whole family'), ...;

    SELECT * FROM AI.SEARCH(TABLE mydataset.products, 'description', 'A really fun toy');

## Before you begin

To enable autonomous embedding generation on a table, you must have the
necessary permissions and connection, and enable the [Vertex AI API](https://console.developers.google.com/apis/api/aiplatform.googleapis.com/overview)
for your project.

### Required roles


To get the permissions that
you need to enable autonomous embedding generation,

ask your administrator to grant you the
following IAM roles:

- To use a connection resource: [BigQuery Connections User](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquery#bigquery.connectionUser) (`roles/bigquery.connectionUser`) on the connection
- To create or alter a table: [BigQuery Data Editor](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquery#bigquery.dataEditor) (`roles/bigquery.dataEditor`) on the table
- Grant the connection's service account the following role so that it can access models hosted in Vertex AI endpoints: [Vertex AI User](https://docs.cloud.google.com/iam/docs/roles-permissions/aiplatform#aiplatform.user) (`roles/aiplatform.user`) on the project that has the connection


For more information about granting roles, see [Manage access to projects, folders, and organizations](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).


You might also be able to get
the required permissions through [custom
roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles) or other [predefined
roles](https://docs.cloud.google.com/iam/docs/roles-overview#predefined).

### Create a connection and grant permission to a service account

To enable autonomous embedding generation on a table, you must
[create a Cloud resource connection](https://docs.cloud.google.com/bigquery/docs/permissions-for-ai-functions#create_a_connection).
Then, [grant](https://docs.cloud.google.com/bigquery/docs/permissions-for-ai-functions#grant_access_to_the_service_account)
the [Vertex AI User role](https://docs.cloud.google.com/iam/docs/roles-permissions/aiplatform#aiplatform.user)
(`roles/aiplatform.user`) to the service account that was created when you
created the connection.

## Create an automatically generated embedding column

You can either create an automatically generated embedding column within a new table or add one to an existing table.

### Create a table with an automatically generated embedding column

You can use autonomous embedding generation to generate embeddings by using
the [`AI.EMBED` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-embed)
in a
[`CREATE TABLE` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_table_statement).

```googlesql
CREATE TABLE DATASET_ID.TABLE (
  [COLUMN, ...]
  STRING_COL STRING,
  EMBEDDING_COL_NAME STRUCT<result ARRAY<FLOAT64>, status STRING>
    GENERATED ALWAYS AS (
      AI.EMBED(
        STRING_COL,
        {
          connection_id => CONNECTION_ID,
          endpoint => ENDPOINT |
          model => MODEL
        })
    )
    STORED OPTIONS (asynchronous = TRUE)
);
```

Replace the following:

- `DATASET_ID`: The name of the dataset in which you want to create the table.
- `TABLE`: The name of the table on which to create autonomous embedding generation.
- `COLUMN, ...`: Any columns that your table should contain besides the column that you want to automatically embed.
- `STRING_COL`: The name of the `STRING` column that you want to automatically embed.
- `EMBEDDING_COL_NAME`: The name of the automatically generated embedding column.
- `CONNECTION_ID`: A `STRING` value that contains the name of a connection to use, such as `my_project.us.example_connection`. You must grant the [Vertex AI User](https://docs.cloud.google.com/vertex-ai/docs/general/access-control#aiplatform.user) role to the connection's service account in the project in which you create the table.
- `ENDPOINT`: a `STRING` value that specifies a supported Vertex AI [text embedding model](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/model-reference/text-embeddings-api) endpoint to use for the text embedding model. The endpoint value that you specify must include the model version, for example `text-embedding-005`. If you specify the model name rather than a URL, BigQuery ML automatically identifies the model and uses the model's full endpoint.
- `MODEL` ([Preview](https://cloud.google.com/products#product-launch-stages)):
  a `STRING` value that specifies a built-in text embedding model.
  The only supported value is the
  [`embeddinggemma-300m` model](https://ai.google.dev/gemma/docs/embeddinggemma/model_card).
  If you specify this parameter, you can't specify the `endpoint`
  or `connection_id` parameters.

  When you specify the `MODEL` parameter,
  your data stays in BigQuery and your slots are used to create
  the embeddings; no data is sent to Vertex AI and no charges
  are incurred in Vertex AI.

### Add an automatically generated embedding column to an existing table

You can also add an automatically generated embedding column to an existing table by
using an [`ALTER TABLE ADD COLUMN` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#alter_table_add_column_statement).

```googlesql
ALTER TABLE DATASET_ID.TABLE
  ADD COLUMN EMBEDDING_COL_NAME
    STRUCT<result ARRAY<FLOAT64>, status STRING>
    GENERATED ALWAYS AS (
      AI.EMBED(
        STRING_COL,
        connection_id => CONNECTION_ID,
        endpoint => ENDPOINT)
    )
    STORED OPTIONS (asynchronous = TRUE)
;
```

The background embedding generation job starts shortly after your table is
created or altered, or after you update data in the source column.

To track the progress of the embedding generation, you can use a query
similar to the following:

    SELECT
      COUNT(*) AS total_num_rows,
      COUNTIF(description_embedding IS NOT NULL
              AND description_embedding.status = '') AS total_num_generated_embeddings
    FROM
      PROJECT_ID.DATASET_ID.TABLE;

After you have the table with embeddings, you can
[create a vector index](https://docs.cloud.google.com/bigquery/docs/vector-index#choose-vector-index-type)
on the `STRUCT` column that contains the automatically generated embedding.

## Example

Suppose you are a large retailer that sells many different products. You have a
table of product names and descriptions and you want to help your customers
find the products they're looking for. The following queries show you how to
set up autonomous embedding generation to assist with semantic search of your
product descriptions.

First, create a dataset:

    CREATE SCHEMA mydataset;

Next, create a table with autonomous embedding generation enabled to hold your
product information. The automatically generated column is called
`description_embedding` and it's based on the `description` column.

    # Create a table of products and descriptions with a generated embedding column.
    CREATE TABLE mydataset.products (
      name STRING,
      description STRING,
      description_embedding STRUCT<result ARRAY<FLOAT64>, status STRING>
        GENERATED ALWAYS AS (
          AI.EMBED(description, connection_id => 'us.example_connection',
            endpoint => 'text-embedding-005')
          # Alternatively, you can use the syntax for a built-in model.
          # AI.EMBED(description, model => 'embeddinggemma-300m')
        ) STORED OPTIONS( asynchronous = TRUE )
    );

The following query inserts some product names and descriptions into the table.
You don't specify a value for `description_embedding` because it's generated
automatically.

    # Insert product descriptions into the table.
    # The description_embedding column is automatically updated.
    INSERT INTO mydataset.products (name, description) VALUES
      ("Lounger chair", "A comfortable chair for relaxing in."),
      ("Super slingers", "An exciting board game for the whole family."),
      ("Encyclopedia set", "A collection of informational books.");

You can optionally create a vector index on the table to speed up searching.
A vector index requires more than three rows, so the following query assumes
that you have inserted additional data. Every time you insert data, the
`description_embedding` column is automatically updated.

    CREATE VECTOR INDEX my_index
    ON mydataset.products(description_embedding)
    OPTIONS(index_type = 'IVF');

Finally, you can use the
[`AI.SEARCH` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-search)
to perform semantic search on your products for a fun toy:

    # Search for products that are fun to play with.
    SELECT base.name, base.description, distance
    FROM AI.SEARCH(TABLE mydataset.products, 'description', "A really fun toy");

    /*---+---+---+
     | name             | description                                  | distance             |
     +---+---+---+
     | Super slingers   | An exciting board game for the whole family. | 0.80954913893618929  |
     | Lounger chair    | A comfortable chair for relaxing in.         | 0.938933930620146    |
     | Encyclopedia set | A collection of informational books.         | 1.1119297739353384   |
     +---+---+---*/

## Get information about automatically generated embedding columns

To verify that a column is an automatically generated embedding column, query
the
[`INFORMATION_SCHEMA.COLUMNS` view](https://docs.cloud.google.com/bigquery/docs/information-schema-columns).

The following query shows you information about all of your automatically
generated embedding columns:

    SELECT *
    FROM PROJECT_ID.DATASET_ID.INFORMATION_SCHEMA.COLUMNS
    WHERE is_generated = 'ALWAYS';

The `generation_expression` field shows you the call to the `AI.EMBED` function
that is used to generate the embeddings on the column.

## Troubleshooting

The generated embedding column contains two fields: `result` and `status`.
If an error occurs when BigQuery tries to generate an embedding
for a particular row in your table, then the `result` field is `NULL` and the
`status` field describes the error. For example, if the source column is `NULL`
then the `result` embedding is also `NULL` and the status is
`NULL value is not supported for embedding generation`.

A more severe error can stall embedding generation. In this case, you can
query the
[`INFORMATION_SCHEMA.JOBS` view](https://docs.cloud.google.com/bigquery/docs/information-schema-jobs)
for the background job and look at the information in the `error_result` field.
The job ID of a background embedding job is prefixed with `gc_`. For example,
the following query extracts all background jobs whose error result isn't
`NULL`:

    SELECT * FROM `region-REGION.INFORMATION_SCHEMA.JOBS` j
    WHERE EXISTS (
      SELECT 1
      FROM unnest(j.referenced_tables) t
      WHERE
        j.project_id = 'PROJECT_ID'
        AND t.dataset_id = 'DATASET_ID'
        AND t.table_id = 'TABLE'
    )
    AND starts_with(job_id, 'gc')
    AND error_result IS NOT NULL
    ORDER BY j.creation_time DESC;

## Track costs

Autonomous embedding generation costs fall into the following categories.

### BigQuery background DML costs

Generated embeddings are written to your table using background DML jobs.
By default, BigQuery uses on-demand slots to handle these jobs.
The table's project is billed following the
[DML on-demand billing model](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/dml-syntax).

Alternatively, to ensure predictable and consistent performance, you can
[create a reservation](https://docs.cloud.google.com/bigquery/docs/reservations-tasks)
and set the `job_type` to `BACKGROUND`. When a background reservation is
present, BigQuery uses it to run the background DML jobs. And
the background reservation will be billed for slot time usage from the
background DML jobs.

### Vertex AI costs

Autonomous embedding generation sends requests to
Vertex AI, which can incur costs. To track the Vertex AI
costs incurred by background embedding jobs, follow these steps:

1. [View your billing reports](https://docs.cloud.google.com/billing/docs/how-to/reports) in Cloud Billing.
2. [Use filters](https://docs.cloud.google.com/billing/docs/how-to/reports#filters) to refine your results.

   For services, select **Vertex AI**.
3. To see the charges for a specific job,
   [filter by label](https://docs.cloud.google.com/billing/docs/how-to/reports#filter-by-labels).

   Set the key to `bigquery_ml_job` and the value to the
   [job ID](https://docs.cloud.google.com/bigquery/docs/managing-jobs#view-job) of the embedding job.
   Background embedding jobs all have a prefix of `gc_`.

It can take up to 24 hours for some charges to appear in Cloud Billing.

## Limitations

- Each table supports at most one automatically generated embedding column.
- Concurrent DML operations can cause delays and temporary failures in embedding generation. For better performance and to reduce costs, we recommend injecting data in batches and avoiding frequent DML updates.
- If you are using [BigQuery Storage Write API](https://docs.cloud.google.com/bigquery/docs/write-api) to ingest data, then there might be some delays before the embedding generation starts.
- There is no indication that a column is automatically generated when you view a table's schema using the Google Cloud console, the `bq show` command, or the `ddl` field of the `INFORMATION_SCHEMA.TABLES` view.
- If you create a copy, clone, or snapshot of a table that has a generated embedding column, only the data is copied. The generation configuration doesn't apply to the new table, and updates to the source column of the new table won't result in new embeddings.
- If you restore a table that had autonomous embedding generation enabled from a snapshot, the embedding generation configuration isn't restored.
- You can create generated embedding columns only by using SQL. You can't use the `bq mk` or `bq update` commands to create generated embedding columns.
- The source column of the generated column must be a `STRING` column.
- After you create the generated embedding column, the following
  limitations apply:

  - You can't drop or rename the source column, but you can still drop or rename the generated embedding column. If you drop the embedding column, then you can drop or rename the source column.
  - You can't change the data type of the source column or generated embedding column.
- You can't specify default values for automatically
  generated embedding columns.

- You can't directly write to generated embedding columns by using these
  methods:

  - DML
  - Streaming writes
  - `bq insert`
  - `bq copy -a`
- Tables with generated embedding columns don't support any
  column-level security policies, such as policy tags.

- When you call a search function, such as
  [`VECTOR_SEARCH`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/search_functions#vector_search)
  or
  [`AI.SEARCH`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-search),
  rows with missing embeddings in the base table are skipped during the
  search.

- You can't create a partitioned vector index on a table that has autonomous
  embedding generation enabled.

- If you create a vector index on the automatically generated embedding
  column, then index training starts after at least 80% of the rows
  have generated embeddings. You can use the following query to check
  what percentage of embeddings on your table have been generated:

      SELECT
        COUNTIF(description_embedding IS NOT NULL
        AND description_embedding.status = '') * 100.0 / COUNT(*) AS percent
      FROM PROJECT_ID.DATASET_ID.TABLE;

## What's next

- Learn more about [creating and managing vector indexes](https://docs.cloud.google.com/bigquery/docs/vector-index).
- See the [Introduction to vector search](https://docs.cloud.google.com/bigquery/docs/vector-search-intro).