# Introduction to AI in BigQuery

> [!NOTE]
> **Note:** This feature may not be available when using reservations that are created with certain BigQuery editions. For more information about which features are enabled in each edition, see [Introduction to
> BigQuery editions](https://docs.cloud.google.com/bigquery/docs/editions-intro).

BigQuery offers various AI capabilities that let you do the following:

- Do predictive machine learning (ML).
- Run inference against large language models (LLMs) such as Gemini.
- Build applications using embeddings and vector search.
- Use built-in agents to assist with coding.
- Create data pipelines.
- Access BigQuery functionality with agent tools.

## Machine learning

With BigQuery ML, you can train, evaluate, and run inference on
models for tasks such as time series forecasting, anomaly detection,
classification, regression, clustering, dimensionality reduction, and
recommendations.

You can work with BigQuery ML capabilities through the
Google Cloud console, the bq command-line tool, the REST API, or in
[Colab Enterprise notebooks](https://docs.cloud.google.com/bigquery/docs/notebooks-introduction).
Because BigQuery ML lets SQL practitioners use
existing SQL tools and skills to build and evaluate models, it democratizes ML
and speeds up model development by bringing ML to the data instead of requiring
data movement. You can use BigQuery ML to help you with the
following types of ML tasks:

- [Create and run ML models](https://docs.cloud.google.com/bigquery/docs/model-overview) by using GoogleSQL queries.
- [Create Colab Enterprise notebooks](https://docs.cloud.google.com/bigquery/docs/create-notebooks) to perform ML workflows. Notebooks let you use SQL and Python interchangeably, and use any AI or ML Python libraries for your development.
- Understand the results of your predictive ML models with [explainable AI](https://docs.cloud.google.com/bigquery/docs/xai-overview).
- Use the `TimesFM`, `ARIMA_PLUS`, and `ARIMA_PLUS_XREG` models to perform [forecasting](https://docs.cloud.google.com/bigquery/docs/forecasting-overview) and [anomaly detection](https://docs.cloud.google.com/bigquery/docs/anomaly-detection-overview) on time series data.
- Generate insights about changes to key metrics in your multi-dimensional data with [contribution analysis](https://docs.cloud.google.com/bigquery/docs/contribution-analysis).

To learn more, see the
[Introduction to ML in BigQuery](https://docs.cloud.google.com/bigquery/docs/bqml-introduction).

## AI functions

BigQuery offers various SQL functions that you can use for
AI tasks such as text generation, text or unstructured data analysis, and
translation. These functions access Gemini and
partner LLM models available from Vertex AI, Cloud AI APIs, or
built-in BigQuery models to perform these tasks.

There are several categories of AI functions:

- **Generative AI functions.** These functions help you perform tasks such as
  content generation, analysis, summarization, structured data extraction,
  classification, embedding generation, and data enrichment. There are two
  types of generative AI functions:

  - [General-purpose AI functions](https://docs.cloud.google.com/bigquery/docs/generative-ai-overview#general_purpose_ai) give you full control and transparency on the choice of model, prompt, and parameters to use.
  - [Managed AI functions](https://docs.cloud.google.com/bigquery/docs/generative-ai-overview#managed_ai_functions) offer a streamlined syntax for routine tasks such as filtering, rating, and classification. BigQuery can choose a model for you to optimize for cost and quality.
- **Task-specific functions.** These functions help you use Cloud AI APIs for
  tasks such as the following:

  - [Natural language processing](https://docs.cloud.google.com/bigquery/docs/ai-application-overview#natural_language_processing)
  - [Machine translation](https://docs.cloud.google.com/bigquery/docs/ai-application-overview#machine_translation)
  - [Document processing](https://docs.cloud.google.com/bigquery/docs/ai-application-overview#document_processing)
  - [Audio transcription](https://docs.cloud.google.com/bigquery/docs/ai-application-overview#audio_transcription)
  - [Computer vision](https://docs.cloud.google.com/bigquery/docs/ai-application-overview#computer_vision)

For more information, see
[Task-specific solutions overview](https://docs.cloud.google.com/bigquery/docs/ai-application-overview).

## Search

BigQuery offers a variety of search functions and features to
help you efficiently find specific data or discover similarities between data
including multimodal data.

- **Text search.** You can use the
  [`SEARCH` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/search_functions#search)
  to perform tokenized search on unstructured text or semi-structured `JSON`
  data. You can improve search performance by creating a
  [search index](https://docs.cloud.google.com/bigquery/docs/search-index), which lets
  BigQuery optimize queries that use the `SEARCH` function, as
  well as other functions and operators.
  For more information, see [Search indexed data](https://docs.cloud.google.com/bigquery/docs/search).

- **Embedding generation.** Embeddings are high-dimensional numerical vectors
  that represent entities such as text or images, and are often generated by ML
  models. You can
  [generate multimodal embeddings](https://docs.cloud.google.com/bigquery/docs/vector-search-intro) by using
  models provided by or hosted on Vertex AI, or by using models
  imported and run in BigQuery.

  You can also have BigQuery automatically maintain a column of
  embeddings by enabling
  [autonomous embedding generation](https://docs.cloud.google.com/bigquery/docs/autonomous-embedding-generation)
  ([Preview](https://cloud.google.com/products#product-launch-stages)).
- **Vector search.** You can use the
  [`VECTOR_SEARCH` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/search_functions#vector_search)
  to search embeddings to find semantically similar items. You can use the
  [`AI.SEARCH` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-search)
  ([Preview](https://cloud.google.com/products#product-launch-stages)) to search
  on tables that have autonomous embedding generation enabled. You can improve
  vector search performance by creating a
  [vector index](https://docs.cloud.google.com/bigquery/docs/vector-index), which uses Approximate Nearest
  Neighbor search techniques to provide faster, more approximate results.

  Common use cases for vector search include semantic search, recommendation,
  and retrieval-augmented generation (RAG). For more information, see
  [Introduction to vector search](https://docs.cloud.google.com/bigquery/docs/vector-search-intro).

## Assistive AI features

AI-powered assistance features in BigQuery, collectively
referred to as
[Gemini in BigQuery](https://docs.cloud.google.com/bigquery/docs/gemini-overview),
help you discover, prepare, query, and visualize your data.

- [**Data insights.**](https://docs.cloud.google.com/bigquery/docs/data-insights) Generate natural language questions about your data, along with the SQL queries to answer those questions.
- [**Data preparation.**](https://docs.cloud.google.com/bigquery/docs/data-prep-introduction) Generate context aware recommendations to clean, transform, and enrich your data.
- [**SQL code assist.**](https://docs.cloud.google.com/bigquery/docs/write-sql-gemini#generate_a_sql_query) Generate, complete, and explain SQL queries.
- [**Python code assist.**](https://docs.cloud.google.com/bigquery/docs/write-sql-gemini#generate_python_code) Generate, complete, and explain Python code, including PySpark and BigQuery DataFrames.
- [**Data canvas.**](https://docs.cloud.google.com/bigquery/docs/data-canvas) Query your data using natural language, visualize results with charts, and ask follow-up questions.
- [**SQL translator.**](https://docs.cloud.google.com/bigquery/docs/interactive-sql-translator) Create Gemini-enhanced SQL translation rules to help you migrate queries written in a different dialect to GoogleSQL.

## Agents

Agents are software tools that can use AI to complete tasks on your behalf.
You can use built-in agents or create your own agents to help you process,
manage, analyze, and visualize your data:

- Use the [Data Science Agent](https://docs.cloud.google.com/bigquery/docs/colab-data-science-agent) to
  automate exploratory data analysis, data processing, ML tasks,
  and visualization insights within a Colab Enterprise notebook.

- Use the
  [Data Engineering Agent](https://docs.cloud.google.com/bigquery/docs/data-engineering-agent-pipelines)
  to build, modify, and manage data pipelines to load and process data in
  BigQuery. You can use natural language prompts to generate data
  pipelines from various data sources or adapt existing data pipelines to suit
  your data engineering needs.

- Use the
  [Conversational Analytics Agent](https://docs.cloud.google.com/bigquery/docs/conversational-analytics)
  to chat with your data using conversational language. This agent consists of
  one or more data sources and a set of use case-specific instructions for
  processing that data. Conversation analytics supports the use of [some
  BigQuery ML functions](https://docs.cloud.google.com/bigquery/docs/conversational-analytics#bigquery-ml-support).

- Use the [Gemini CLI](https://docs.cloud.google.com/bigquery/docs/develop-with-gemini-cli) to
  interact with BigQuery data in your terminal by using
  natural language prompts.

- Build using the [Open source MCP toolbox](https://docs.cloud.google.com/bigquery/docs/pre-built-tools-with-mcp-toolbox)
  or [ADK tools](https://google.github.io/adk-docs/integrations/bigquery/) for
  quick, iterative agent development.

## What's next

- For more information about ML, see [Introduction to ML in BigQuery](https://docs.cloud.google.com/bigquery/docs/bqml-introduction).
- For more information about generative AI functions in SQL, see [Generative AI overview](https://docs.cloud.google.com/bigquery/docs/generative-ai-overview).
- For more information about searching your data, see [Search indexed data](https://docs.cloud.google.com/bigquery/docs/search) and [Introduction to vector search](https://docs.cloud.google.com/bigquery/docs/vector-search-intro).
- For more information about assistive AI features, see [Gemini in BigQuery](https://docs.cloud.google.com/bigquery/docs/gemini-overview).
- For more information about using agents with BigQuery, see:
  - [Use the BigQuery MCP Server](https://docs.cloud.google.com/bigquery/docs/use-bigquery-mcp).
  - [Connect LLMs to BigQuery with MCP](https://docs.cloud.google.com/bigquery/docs/pre-built-tools-with-mcp-toolbox) using IDEs such as Antigravity.
  - [Analyze data with the Gemini CLI](https://docs.cloud.google.com/bigquery/docs/develop-with-gemini-cli).