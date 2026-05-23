# Generative AI overview

This document describes the generative artificial intelligence (AI) functions
that BigQuery supports. These functions accept natural language
inputs and use pre-trained
[Vertex AI models](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/learn/models) and
built-in BigQuery models.

BigQuery offers a variety of AI functions to help with tasks
such as the following:

- Generate creative content.
- Analyze, detect sentiment, and answer questions about text or unstructured data, such as images.
- Summarize the key ideas or impressions conveyed by the content.
- Extract structured data from text.
- Classify text or unstructured data into user-defined categories.
- Generate embeddings to search for similar text, images, and video.
- Rate inputs in order to rank them by quality, similarity, or other criteria.

AI functions are grouped into the following categories to help you accomplish
these tasks:

- **[General-purpose AI functions](https://docs.cloud.google.com/bigquery/docs/generative-ai-overview#general_purpose_ai):** These functions
  give you full control and transparency on the choice of model, prompt, and
  parameters to use.

  - [Perform LLM inference](https://docs.cloud.google.com/bigquery/docs/generative-ai-overview#inference),
    such as to answer questions about your data

    - `AI.GENERATE` is the most flexible inference function, which lets you analyze any structured or unstructured data.
    - `AI.GENERATE_TEXT` is a table-valued version of `AI.GENERATE` that also supports partner models and open models.
  - [Generate structured output](https://docs.cloud.google.com/bigquery/docs/generative-ai-overview#generate_structured_data), such as
    extracting names, addresses, or object descriptions from text,
    documents, or images.

    - `AI.GENERATE`, when you specify an output schema.
    - `AI.GENERATE_TABLE` is a table-valued version of `AI.GENERATE` that calls a remote model and lets you specify a custom output schema.
    - If your output schema has a single field, you can use one of the specialized functions: `AI.GENERATE_BOOL`, `AI.GENERATE_DOUBLE`, or `AI.GENERATE_INT`.
  - [Generate embeddings](https://docs.cloud.google.com/bigquery/docs/generative-ai-overview#text_embedding) for semantic search and
    clustering

    - `AI.EMBED`: Create an embedding from text or image data.
    - `AI.GENERATE_EMBEDDING`: A table-valued function that adds a column of embedded text, image, audio, video, or document data to your table.
- **[Managed AI functions](https://docs.cloud.google.com/bigquery/docs/generative-ai-overview#managed_ai_functions):** These functions have a
  streamlined syntax and are optimized for cost and quality. With [optimized
  mode](https://docs.cloud.google.com/bigquery/docs/optimize-ai-functions) (Preview), these functions scale
  to millions or billions of rows.

  - Filter your data with natural language conditions

    - `AI.IF`
  - Rate input, such as by quality or sentiment

    - `AI.SCORE`
  - Classify input into user-defined categories

    - `AI.CLASSIFY`
- **Utility functions:** Use the
  [`AI.COUNT_TOKENS` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-count-tokens)
  to estimate the number of tokens in an input prompt before you run a query.

- **[Task-specific functions](https://docs.cloud.google.com/bigquery/docs/generative-ai-overview#task-specific_functions):** These functions
  use Cloud AI APIs to help you perform tasks such as natural language
  processing, machine translation, document processing, audio transcription,
  and computer vision.

## General-purpose AI functions

General-purpose AI functions give you full control and transparency on the
choice of model, prompt, and parameters to use. Their output includes detailed
information about the call to the model, including the status and full model
response, which might include information about the safety rating or citations.

### Perform LLM inference

The [`AI.GENERATE` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-generate)
is a flexible inference function that works by
sending requests to a Vertex AI Gemini model and
returning that model's response. You can use this function to analyze
text, image, audio, video, or PDF data. For
example, you might analyze images of home furnishings to generate text for a
`design_type` column, so that the furnishings SKU has an associated
description, such as `mid-century modern` or `farmhouse`.

You can perform generative AI tasks by using remote models in
BigQuery ML to reference models deployed
to or hosted in Vertex AI with the
[`AI.GENERATE_TEXT` table-valued function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-generate-text).
You can use the following types of
[remote models](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model):

- Remote models over any of the
  [generally available](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/models#generally_available_models)
  or
  [preview](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/models#preview_models)
  Gemini models to analyze text, image, audio, video, or PDF
  content from standard tables or object tables with a prompt that you
  provide as a function argument.

- Remote models over
  [Anthropic Claude](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/partner-models/use-claude),
  [Mistral AI](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/partner-models/mistral), or
  [Llama](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/partner-models/llama) partner models,
  or [supported open models](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model-open#supported_open_models),
  to analyze a prompt that you provide in a query or from a column in a
  [standard table](https://docs.cloud.google.com/bigquery/docs/tables-intro#standard-tables).

Use the following topics to try text generation in BigQuery ML:

- [Generate text by using a Gemini model and the `AI.GENERATE_TEXT` function](https://docs.cloud.google.com/bigquery/docs/generate-text-tutorial-gemini).
- [Generate text by using a Gemma model and the `AI.GENERATE_TEXT` function](https://docs.cloud.google.com/bigquery/docs/generate-text-tutorial-gemma).
- [Analyze images with a Gemini model](https://docs.cloud.google.com/bigquery/docs/image-analysis).
- [Generate text by using the `AI.GENERATE_TEXT` function with your data](https://docs.cloud.google.com/bigquery/docs/generate-text).
- [Tune a model using your data](https://docs.cloud.google.com/bigquery/docs/generate-text-tuning).

For some models, you can optionally choose to configure
[supervised tuning](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model-tuned#supervised_tuning),
which lets you train the model on your own data to make it better suited for
your use case. All inference occurs in Vertex AI.
The results are stored in BigQuery.

### Generate structured data

Structured data generation is very similar to text generation, except that
you can format the response from the model by specifying a SQL schema. For
example, you might generate a table that contains a customer's name, phone
number, address, request, and pricing quote from a transcript of a phone call.

You can generate structured data in the following ways:

- The [`AI.GENERATE` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-generate)
  calls a Vertex AI endpoint and can generate a `STRUCT` value
  with your custom schema.

  To try it out, see how to
  [use structured output](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-generate#use_structured_output)
  when you call the `AI.GENERATE` function.
- The [`AI.GENERATE_TABLE` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-generate-table)
  calls a remote model and is a table-valued
  function that generates a table with your custom schema.

  To try creating structured data, see
  [Generate structured data by using the `AI.GENERATE_TABLE` function](https://docs.cloud.google.com/bigquery/docs/generate-table).
- For a single output field, you can use one of the following specialized
  inference functions:

  - [`AI.GENERATE_BOOL`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-generate-bool)
  - [`AI.GENERATE_DOUBLE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-generate-double)
  - [`AI.GENERATE_INT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-generate-int)

### Generate embeddings

An embedding is a high-dimensional numerical vector that represents a given
entity, like a piece of text or an audio file. Generating embeddings lets you
capture the semantics of your data in a way that makes it easier to reason about
and compare the data.

Some common use cases for embedding generation are as follows:

- Using retrieval-augmented generation (RAG) to augment model responses to user queries by referencing additional data from a trusted source. RAG provides better factual accuracy and response consistency, and also provides access to data that is newer than the model's training data.
- Performing multimodal search. For example, using text input to search images.
- Performing semantic search to find similar items for recommendations, substitution, and record deduplication.
- Creating embeddings to use with a k-means model for clustering.

For more information about how to generate embeddings and use them to perform
these tasks, see the
[Introduction to embeddings and vector search](https://docs.cloud.google.com/bigquery/docs/vector-search-intro).

## Managed AI functions

Managed AI functions simplify routine tasks, such as filtering, classification,
or aggregation. These functions can analyze text, image, audio, video, or PDF
data. These functions use Gemini and don't require
customization. BigQuery uses prompt engineering and can select
the appropriate model and parameters to use for the specific task to optimize
the quality and consistency of your results. Each function returns a scalar
value, such as a `BOOL`, `FLOAT64`, or `STRING`, and doesn't include additional
status information from the model. The following managed AI functions are
available:

- [`AI.IF`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-if): Filter text or multi-modal data, such as in a `WHERE` or `JOIN` clause, based on a prompt. For example, you could filter product descriptions by those that describe an item that would make a good gift.
- [`AI.SCORE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-score): Rate inputs based on a prompt in order to rank rows by quality, similarity, or other criteria. You can use this function in an `ORDER BY` clause to extract the top K items according to score. For example, you could find the top 10 most positive or negative user reviews for a product.
- [`AI.CLASSIFY`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-classify): Classify text into user-defined categories. You can use this function in a `GROUP BY` clause to group inputs according to the categories that you define. For example, you could classify support tickets by whether they relate to billing, shipping, product quality, or something else.

> [!NOTE]
> **Note:** To process large datasets more efficiently, you can use *optimized mode* (Preview) for `AI.IF` and `AI.CLASSIFY` functions. This mode works by training a lightweight, distilled model on a sample of your data, then using that model for inference on the majority of rows. This approach avoids calling an LLM for every row, which [reduces token consumption and latency](https://docs.cloud.google.com/bigquery/docs/optimize-ai-functions) when working with thousands or even billions of rows.

For a tutorial that shows examples of how to use these functions, see
[Perform semantic analysis with managed AI functions](https://docs.cloud.google.com/bigquery/docs/semantic-analysis).

For a notebook tutorial that shows how to use managed and general-purpose AI
functions, see
[Semantic analysis with AI functions](https://github.com/GoogleCloudPlatform/generative-ai/blob/main/gemini/use-cases/applying-llms-to-data/bigquery_ai_operators.ipynb).

## Task-specific functions

In addition to the more general functions described in the previous sections,
you can develop task-specific solutions in BigQuery ML
by using Cloud AI APIs. Supported tasks include the following:

- [Natural language processing](https://docs.cloud.google.com/bigquery/docs/ai-application-overview#natural_language_processing)
- [Machine translation](https://docs.cloud.google.com/bigquery/docs/ai-application-overview#machine_translation)
- [Document processing](https://docs.cloud.google.com/bigquery/docs/ai-application-overview#document_processing)
- [Audio transcription](https://docs.cloud.google.com/bigquery/docs/ai-application-overview#audio_transcription)
- [Computer vision](https://docs.cloud.google.com/bigquery/docs/ai-application-overview#computer_vision)

For more information, see
[Task-specific solutions overview](https://docs.cloud.google.com/bigquery/docs/ai-application-overview).

## Locations

Supported locations for text generation and embedding models vary based on the
model type and version that you use. For more information, see
[Locations](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model#locations).

## Pricing

You are charged for the compute resources that you use to run queries against
models. Remote models make calls to Vertex AI
models, so queries against remote models also incur charges from
Vertex AI.

For more information, see
[BigQuery ML pricing](https://cloud.google.com/bigquery/pricing#bigquery-ml-pricing).

## Track token usage

When you call a generative AI function that uses a Gemini model
other than an embedding model, you can view
the total number of each type of token processed by the query. In the
**Query results** pane, click **Job information**. The following counts appear,
broken down by modality if applicable:

- **Input token count:** The total number of input tokens to all generative AI functions called in the query.
- **Output token count.** The total number of tokens across all candidate responses generated by the query.
- **Thought token count.** The total number of tokens that were part of the model's generated thoughts, if applicable.
- **Cache token count.** The total number of input tokens that were [implicitly cached](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/context-cache/context-cache-overview#implicit-caching) by the query.

## Track costs

The generative AI functions in BigQuery work by sending requests to
Vertex AI, which can incur costs. To estimate your input token count
before you run a query, use the
[`AI.COUNT_TOKENS` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-count-tokens).
To track the Vertex AI
costs incurred by a job that you run in BigQuery, follow these
steps:

1. [View your billing reports](https://docs.cloud.google.com/billing/docs/how-to/reports) in Cloud Billing.
2. [Use filters](https://docs.cloud.google.com/billing/docs/how-to/reports#filters) to refine your results.

   For services, select **Vertex AI**.
3. To see the charges for a specific job,
   [filter by label](https://docs.cloud.google.com/billing/docs/how-to/reports#filter-by-labels).

   Set the key to `bigquery_job_id_prefix` and the value to the
   [job ID](https://docs.cloud.google.com/bigquery/docs/managing-jobs#view-job) of your job. If your job ID
   is longer than 63 characters, only use the first 63 characters. If your
   job ID contains any uppercase characters, change them to lowercase.
   Alternatively, you can
   [associate jobs with a custom label](https://docs.cloud.google.com/bigquery/docs/adding-labels#adding-label-to-session)
   to help you look them up later.

It can take up to 24 hours for some charges to appear in Cloud Billing.

## Monitoring

To better understand the behavior of AI functions that you call in
BigQuery, you can enable request and response logging. To
log the entire request and response sent to and received from
Vertex AI, follow these steps:

1. [Enable request-response logs](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/multimodal/request-response-logging)
   in Vertex AI. The logs are stored in BigQuery.
   You must separately enable logging for each different foundation model and
   region. To log queries that run in the `us` region, specify the `us-central1`
   region in your request. To log queries that run in the `eu` region, specify
   the `europe-west4` region in your request.

2. Run a query using an AI function that makes a call to
   Vertex AI using the model that you enabled logging for
   in the previous step.

3. To view the full Vertex AI request and response, query
   your logging table for rows where the
   `labels.bigquery_job_id_prefix` field of the `full_request` column
   matches the first 63 characters of your
   [job ID](https://docs.cloud.google.com/bigquery/docs/managing-jobs#view-job). Optionally, you can
   [use a custom query label](https://docs.cloud.google.com/bigquery/docs/adding-labels#adding-label-to-session)
   to help you look up the query in the logs.

   For example, you can use a query similar to the following:

       SELECT *
       FROM `my_project.my_dataset.request_response_logging`
       WHERE JSON_VALUE(full_request, '$.labels.bigquery_job_id_prefix') = 'bquxjob_123456...';

## Error management

Row-level errors, such as `RESOURCE_EXHAUSTED`, can occur if an AI function
exceeds the quota or limits of the remote service. When a row-level error
occurs, the function returns `NULL` for that row, which can result in incomplete
query results.

All AI functions can encounter these errors. However, the managed AI functions
(`AI.IF`, `AI.CLASSIFY`, and `AI.SCORE`) support the `max_error_ratio` argument
to help you manage them. Use this argument to set a failure threshold that
allows the query to succeed despite row-level failures.

The default value for `max_error_ratio` is `1.0`. To lower your error tolerance,
set it to a smaller value (for example, `0.2`) so that the query fails instead
of succeeding with partial failures. For syntax details, see the reference
documentation for
[`AI.IF`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-if#arguments),
[`AI.CLASSIFY`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-classify#arguments),
or
[`AI.SCORE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-score#arguments).

If the query succeeds with partial failures, BigQuery returns a
warning. For more information about the function errors, check the **Gen AI
function errors** field in the **Job information** tab of the query results in
the Google Cloud console.

> [!NOTE]
> **Note:** The error ratio is calculated as the number of failed rows divided by the total number of rows processed by the model.

If your query includes a `LIMIT` clause, the limit is applied *after* the model
processes a batch of rows. Consequently, the proportion of `NULL` values in your
final result set might appear higher than the specified `max_error_ratio`.

For example, suppose your query has a `LIMIT 10` clause and a `max_error_ratio`
of `0.2`. The model might process 20 rows before the limit is applied. If 3 of
those 20 rows fail, the error ratio is `0.15` (15%), which is within the 20%
threshold. However, if the subset of rows selected by the `LIMIT` clause happens
to include all 3 failed rows, your visible output will contain 30% `NULL`
values.

## What's next

- For an introduction to AI and ML in BigQuery, see [Introduction to AI and ML in BigQuery](https://docs.cloud.google.com/bigquery/docs/bqml-introduction).
- For more information about performing inference over machine learning models, see [Model inference overview](https://docs.cloud.google.com/bigquery/docs/inference-overview).
- For more information about supported SQL statements and functions for generative AI models, see [End-to-end user journeys for generative AI models](https://docs.cloud.google.com/bigquery/docs/e2e-journey-genai).