# Introduction to embeddings and vector search

This document provides an overview of embeddings and vector search in
BigQuery. Vector search is a technique to compare similar objects using embeddings, and it
is used to power Google products, including Google Search,
YouTube, and Google Play. You can use vector search to perform
searches at scale. When you use [vector indexes](https://docs.cloud.google.com/bigquery/docs/vector-index)
with vector search, you can take advantage of foundational technologies like
inverted file indexing (IVF) and the
[ScaNN algorithm](https://research.google/blog/announcing-scann-efficient-vector-similarity-search/).

Vector search is built on embeddings. Embeddings are high-dimensional numerical
vectors that represent a given entity, like a piece of text or an audio file.
Machine learning (ML) models use embeddings to encode semantics about such
entities to make it easier to reason about and compare them. For example, a
common operation in clustering, classification, and recommendation models is to
measure the distance between vectors in an
[embedding space](https://en.wikipedia.org/wiki/Latent_space) to find items
that are most semantically similar.

This concept of semantic similarity and distance in an embedding space is
visually demonstrated when you consider how different items might be plotted.
For example, terms like *cat* , *dog* , and *lion* , which all represent types of
animals, are grouped close together in this space due to their shared semantic
characteristics. Similarly, terms like *car* , *truck* , and the more generic term
*vehicle* would form another cluster. This is shown in the following image:

![Semantically similar concepts, like _cat_, _dog_, and _lion_, or _car_, _truck_, and _vehicle_, are close
together in the embedding space.](https://docs.cloud.google.com/static/bigquery/images/embedding-space.png)

You can see that the animal and vehicle clusters are positioned far apart
from each other. The separation between the groups illustrates the principle
that the closer objects are in the embedding space, the more semantically
similar they are, and greater distances indicate greater semantic dissimilarity.

## Use cases

The combination of embedding generation and vector search enables many
interesting use cases. Some possible use cases are as follows:

- **[Retrieval-augmented generation (RAG)](https://cloud.google.com/use-cases/retrieval-augmented-generation):** Parse documents, perform vector search on content, and generate summarized answers to natural language questions using Gemini models, all within BigQuery. For a notebook that illustrates this scenario, see [Build a Vector Search application using BigQuery DataFrames](https://github.com/googleapis/python-bigquery-dataframes/blob/main/notebooks/generative_ai/bq_dataframes_llm_vector_search.ipynb).
- **Recommending product substitutes or matching products:** Enhance ecommerce applications by suggesting product alternatives based on customer behavior and product similarity.
- **Log analytics:** Help teams proactively triage anomalies in logs and accelerate investigations. You can also use this capability to enrich context for LLMs, in order to improve threat detection, forensics, and troubleshooting workflows. For a notebook that illustrates this scenario, see [Log Anomaly Detection \& Investigation with Text Embeddings + BigQuery Vector Search](https://github.com/GoogleCloudPlatform/generative-ai/blob/main/embeddings/use-cases/outlier-detection/bq-vector-search-outlier-detection-audit-logs.ipynb).
- **Clustering and targeting:** Segment audiences with precision. For example, a hospital chain could cluster patients using natural language notes and structured data, or a marketer could target ads based on query intent. For a notebook that illustrates this scenario, see [Create-Campaign-Customer-Segmentation](https://github.com/GoogleCloudPlatform/chocolate-ai/blob/main/colab-enterprise/Create-Campaign-Customer-Segmentation.ipynb).
- **Entity resolution and deduplication:** Cleanse and consolidate data. For example, an advertising company could deduplicate personally identifiable information (PII) records, or a real estate company could identify matching mailing addresses.

## Generate embeddings

The following sections describe the functions that BigQuery
offers to help you generate or work with embeddings.

### Generate single embeddings

You can use the
[`AI.EMBED` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-embed)
with Vertex AI embedding models to generate a single embedding
of your input.

The `AI.EMBED` function supports the following types of input:

- Text data.
- Image data represented by [`ObjectRef`](https://docs.cloud.google.com/bigquery/docs/work-with-objectref) values.
- Image data represented by [`ObjectRefRuntime`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/objectref_functions#objectrefruntime) values.

### Generate a table of embeddings

You can use the
[`AI.GENERATE_EMBEDDING`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-generate-embedding)
to create a table that has embeddings for all of the data in a column of your
input table. For all types of supported models, `AI.GENERATE_EMBEDDING`
works with structured data in
[standard tables](https://docs.cloud.google.com/bigquery/docs/tables-intro#standard-tables). For multimodal
embedding models, `AI.GENERATE_EMBEDDING` also works with visual content
from either standard table
[columns that contain `ObjectRef` values](https://docs.cloud.google.com/bigquery/docs/objectref-columns),
or from [object tables](https://docs.cloud.google.com/bigquery/docs/object-table-introduction).

For remote models, all inference occurs in Vertex AI. For other
model types, all inference occurs in BigQuery. The results are
stored in BigQuery.

Use the following topics to try embedding generation in
BigQuery ML:

- Generate [text](https://docs.cloud.google.com/bigquery/docs/generate-text-embedding), [images](https://docs.cloud.google.com/bigquery/docs/generate-visual-content-embedding), or [video](https://docs.cloud.google.com/bigquery/docs/generate-video-embedding) by using the `AI.GENERATE_EMBEDDING` function.
- [Generate and search multimodal embeddings](https://docs.cloud.google.com/bigquery/docs/generate-multimodal-embeddings)
- [Perform semantic search and retrieval-augmented generation](https://docs.cloud.google.com/bigquery/docs/vector-index-text-search-tutorial)

### Autonomous embedding generation

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

You can use
[autonomous embedding generation](https://docs.cloud.google.com/bigquery/docs/autonomous-embedding-generation)
to simplify the process of creating, maintaining, and querying embeddings.
BigQuery maintains a column of embeddings on your table
based on a source column. When you add or modify data in the source column,
BigQuery automatically generates or updates the embedding
column for that data by using a Vertex AI embedding model.
This is helpful if you want to let BigQuery maintain your
embeddings when your source data is updated regularly.

## Search

The following search functions are available:

- [`VECTOR_SEARCH`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/search_functions#vector_search):
  Perform a vector search by using SQL.

- [`AI.SEARCH`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-search)
  ([Preview](https://cloud.google.com/products#product-launch-stages)):
  Search for results that are close to a string that you provide. You can use
  this function if your table has
  [autonomous embedding generation](https://docs.cloud.google.com/bigquery/docs/vector-search-intro#autonomous_embedding_generation) enabled.

- [`AI.SIMILARITY`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-similarity):
  Compare two inputs by computing the
  [cosine similarity](https://wikipedia.org/wiki/Cosine_similarity) between
  their embeddings. This function works well if you want to perform a small
  number of comparisons and you haven't precomputed any embeddings. You should
  use `VECTOR_SEARCH` when performance is critical and you're working with a
  large number of embeddings.
  [Compare their functionality](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-similarity#related_functions)
  to choose the best function for your use case.

Optionally, you can create a [vector index](https://docs.cloud.google.com/bigquery/docs/vector-index) by
using the
[`CREATE VECTOR INDEX` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_vector_index_statement).
When a vector index is used, the `VECTOR_SEARCH` and `AI.SEARCH` functions use
the [Approximate Nearest Neighbor](https://en.wikipedia.org/wiki/Nearest_neighbor_search#Approximation_methods)
search technique to improve vector search performance, with the trade-off of
reducing
[recall](https://developers.google.com/machine-learning/crash-course/classification/precision-and-recall#recallsearch_term_rules)
and so returning more approximate results. Without a vector index, these
functions use
[brute force search](https://en.wikipedia.org/wiki/Brute-force_search) to
measure distance for every record. You can also choose to use brute force to get
exact results even when a vector index is available.

## Pricing

The `VECTOR_SEARCH` and `AI.SEARCH` functions and the `CREATE VECTOR INDEX`
statement use
[BigQuery compute pricing](https://cloud.google.com/bigquery/pricing#analysis_pricing_models).

- `VECTOR_SEARCH` and `AI.SEARCH` functions: You are charged for similarity
  search, using on-demand or editions pricing.

  - On-demand: You are charged for the amount of bytes scanned in the base table, the index, and the search query.
  - Editions pricing: You are charged for the slots required to complete
    the job within your reservation edition. Larger, more complex
    similarity calculations incur more charges.

    > [!NOTE]
    > **Note:** Using an index isn't supported in [Standard editions](https://docs.cloud.google.com/bigquery/docs/editions-intro).

- `CREATE VECTOR INDEX` statement: There is no charge for the processing
  required to build and refresh your vector indexes as long as the total
  size of the indexed table data is below your per-organization
  [limit](https://docs.cloud.google.com/bigquery/quotas#vector_index_maximum_table_size). To
  support indexing beyond this limit, you must
  [provide your own reservation](https://docs.cloud.google.com/bigquery/docs/vector-index#use_your_own_reservation)
  for handling the index management jobs.

Storage is also a consideration for embeddings and indexes. The amount of bytes
stored as embeddings and indexes are subject to
[active storage costs](https://cloud.google.com/bigquery/pricing#storage).

- Vector indexes incur storage costs when they are active.
- You can find the index storage size by using the [`INFORMATION_SCHEMA.VECTOR_INDEXES` view](https://docs.cloud.google.com/bigquery/docs/information-schema-vector-indexes). If the vector index is not yet at 100% coverage, you are still charged for whatever has been indexed. You can check index coverage by using the `INFORMATION_SCHEMA.VECTOR_INDEXES` view.

## Quotas and limits

For more information, see
[Vector index limits](https://docs.cloud.google.com/bigquery/quotas#vector_index_limits) and
[generative AI function limits](https://docs.cloud.google.com/bigquery/quotas#generative_ai_functions).

## Limitations

Queries that contain the `VECTOR_SEARCH` or `AI.SEARCH` function aren't
accelerated by
[BigQuery BI Engine](https://docs.cloud.google.com/bigquery/docs/bi-engine-intro).

## What's next

- Learn more about [creating a vector index](https://docs.cloud.google.com/bigquery/docs/vector-index).
- Learn how to perform a vector search using the [`VECTOR_SEARCH`
  function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/search_functions#vector_search).
- Learn how to perform semantic search using the [`AI.SEARCH` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-search).
- Learn more about [autonomous embedding generation](https://docs.cloud.google.com/bigquery/docs/autonomous-embedding-generation).
- Try the [Search embeddings with vector search](https://docs.cloud.google.com/bigquery/docs/vector-search) tutorial to learn how to create a vector index, and then do a vector search for embeddings both with and without the index.
- Try the [Perform semantic search and retrieval-augmented generation](https://docs.cloud.google.com/bigquery/docs/vector-index-text-search-tutorial)
  tutorial to learn how to do the following tasks:

  - Generate text embeddings.
  - Create a vector index on the embeddings.
  - Perform a vector search with the embeddings to search for similar text.
  - Perform retrieval-augmented generation (RAG) by using vector search results to augment the prompt input and improve results.
- Try the [Parse PDFs in a retrieval-augmented generation pipeline](https://docs.cloud.google.com/bigquery/docs/rag-pipeline-pdf)
  tutorial to learn how to create a RAG pipeline based on parsed PDF content.

- You can also perform vector searches by using BigQuery DataFrames in Python.
  For a notebook that illustrates this approach, see
  [Build a Vector Search application using BigQuery DataFrames](https://github.com/googleapis/python-bigquery-dataframes/blob/main/notebooks/generative_ai/bq_dataframes_llm_vector_search.ipynb).