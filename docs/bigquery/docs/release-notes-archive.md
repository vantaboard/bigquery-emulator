This page documents production updates to BigQuery. We recommend
that BigQuery developers periodically check this list for any
new announcements. BigQuery automatically updates to the latest
release and cannot be downgraded to a previous version.

This page contains a historical archive of all release notes for
BigQuery. To view more recent release notes, see the
[Release notes](https://docs.cloud.google.com/bigquery/docs/release-notes).


You can see the latest product updates for all of Google Cloud on the
[Google Cloud](https://docs.cloud.google.com/release-notes) page, browse and filter all release notes in the
[Google Cloud console](https://console.cloud.google.com/release-notes),
or programmatically access release notes in
[BigQuery](https://console.cloud.google.com/bigquery?p=bigquery-public-data&d=google_cloud_release_notes&t=release_notes&page=table).

To get the latest product updates delivered to you, add the URL of this page to your
[feed
reader](https://wikipedia.org/wiki/Comparison_of_feed_aggregators), or add the
[feed URL](https://docs.cloud.google.com/feeds/bigquery-release-notes.xml) directly.

## December 23, 2024

Change BigQuery is available in the [Mexico (northamerica-south1)](https://docs.cloud.google.com/bigquery/docs/locations#regions) region.

## December 19, 2024

Feature The [Sovereign Controls for EU](https://docs.cloud.google.com/assured-workloads/docs/eu-sovereign-controls-restrictions-limitations) control package now supports BigQuery Data Transfer Service. For more information, see [Supported products by control package](https://docs.cloud.google.com/assured-workloads/docs/supported-products). This feature is [generally available](https://cloud.google.com/products#product-launch-stages) (GA).
Feature You can now manage [data canvases](https://docs.cloud.google.com/bigquery/docs/data-canvas), [data preparations](https://docs.cloud.google.com/bigquery/docs/manage-data-preparations), [notebooks](https://docs.cloud.google.com/bigquery/docs/manage-notebooks), [saved queries](https://docs.cloud.google.com/bigquery/docs/manage-saved-queries), and [workflows](https://docs.cloud.google.com/bigquery/docs/manage-workflows) in Dataplex. Metadata of data canvases, data preparations, notebooks, saved queries, and workflows is automatically available in Dataplex, without additional configuration. This feature is [generally available](https://cloud.google.com/products#product-launch-stages) (GA).
Feature You can now [search for](https://docs.cloud.google.com/dataplex/docs/search-assets) and view the metadata of data canvases, data preparations, notebooks, saved queries, and workflows in the Dataplex console. This feature is in [preview](https://cloud.google.com/products#product-launch-stages).

## December 16, 2024

Feature You can now use the [Google Cloud Code extension for VS Code](https://docs.cloud.google.com/bigquery/docs/vs-code-extension) to work with BigQuery datasets and notebooks in your VS Code environment. This feature is in [preview](https://cloud.google.com/products#product-launch-stages).

## December 12, 2024

Feature Regional endpoints, which help you run your workloads in compliance with [data residency](https://docs.cloud.google.com/assured-workloads/docs/data-residency) and data sovereignty requirements, are now [generally available](https://cloud.google.com/products#product-launch-stages) (GA). With regional endpoints, your request traffic is routed directly to the region specified in the endpoint. For more information, see [BigQuery regional endpoints](https://docs.cloud.google.com/bigquery/docs/regional-endpoints).
Feature You can now discover, procure, and [commercialize your Analytics Hub listings on Google Cloud Marketplace](https://docs.cloud.google.com/bigquery/docs/analytics-hub-cloud-marketplace) to share data offerings at scale. This feature is in [preview](https://cloud.google.com/products#product-launch-stages).

## December 11, 2024

Feature You can now create
[remote models](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model)
in BigQuery ML based on the
[`gemini-2.0-flash-exp`](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/gemini-v2)
model in Vertex AI. To create remote models, you can use either SQL or BigQuery
DataFrames.

You can use the
[`ML.GENERATE_TEXT` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-generate-text)
with these remote models to perform generative natural language tasks for text
stored in BigQuery tables. You can also
use the `ML.GENERATE_TEXT` function with these remote models to perform
generative AI tasks, for example audio transcription or document classification,
using image, video, audio, PDF, or text content stored in BigQuery
[object tables](https://docs.cloud.google.com/bigquery/docs/object-table-introduction).

Try this feature by using either the
[Generate text by using the `ML.GENERATE_TEXT` function](https://docs.cloud.google.com/bigquery/docs/generate-text)
how-to topic, or the
[BigFrames Gemini 2.0 Text Generation Simple Example](https://github.com/googleapis/python-bigquery-dataframes/blob/main/notebooks/generative_ai/bq_dataframes_llm_gemini_2.ipynb)
notebook.

This feature is in
[preview](https://cloud.google.com/products/#product-launch-stages).
Feature You can now replicate a dataset from the source region to one or more other regions with [cross-region dataset replication](https://docs.cloud.google.com/bigquery/docs/data-replication). This feature is now [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).
Feature [BigQuery Managed Disaster Recovery](https://docs.cloud.google.com/bigquery/docs/managed-disaster-recovery) provides managed failover and redundant compute capacity for business-critical workloads. It is intended for use in the case of a total region outage and is supported with the [BigQuery Enterprise Plus edition](https://docs.cloud.google.com/bigquery/docs/editions-intro) only. This feature is now [generally available](https://cloud.google.com/products#product-launch-stages) (GA).

## November 19, 2024

Feature You can [create a search index](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_search_index_statement) on columns containing `INT64` or `TIMESTAMP` data and BigQuery can [optimize predicates](https://docs.cloud.google.com/bigquery/docs/search#numeric-predicates-seo) that use those columns. This feature is [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).

## November 14, 2024

Feature The following BigQuery ML features are now available:

- Creating [remote models](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model) based on the [Vertex AI gemini-1.5-flash and gemini-1.5-pro models](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/learn/models#gemini-models).
- Using the [`ML.GENERATE_TEXT` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-generate-text) with these remote models to perform generative natural language tasks for text stored in BigQuery tables.
- Using the [`ML.GENERATE_TEXT` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-generate-text) with these remote models to perform generative AI tasks, for example audio transcription or document classification, using image, video, audio, PDF, or text content stored in BigQuery [object tables](https://docs.cloud.google.com/bigquery/docs/object-table-introduction).

Try these features with the
[Generate text by using the `ML.GENERATE_TEXT` function](https://docs.cloud.google.com/bigquery/docs/generate-text)
how-to topic.

These features are now
[generally available](https://cloud.google.com/products/#product-launch-stages)
(GA).
Announcement You can try Gemini in BigQuery at no charge until January 27, 2025. After that date, to continue to use Gemini in BigQuery you must do one of the following:

- Purchase and assign BigQuery Enterprise Plus edition reservations to projects that use Gemini in BigQuery.
- Purchase Gemini Code Assist Enterprise.

To learn more, see [Purchase Gemini in BigQuery](https://docs.cloud.google.com/gemini/docs/bigquery/set-up-gemini#purchase). These purchase options are now [generally available](https://cloud.google.com/products#product-launch-stages) (GA).

<br />

## November 11, 2024

Feature The following BigQuery ML features are now available:

- You can perform [supervised tuning](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model#supervised_tuning) on a remote model based on a Vertex AI [Gemini 1.5 flash or Gemini 1.5 pro model](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/learn/models#gemini-models).
- You can evaluate a Vertex AI LLM using the [`ML.EVALUATE` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-evaluate). Pre-trained PaLM and Gemini models and tuned Gemini models are supported for evaluation.

Try tuning and evaluating an LLM with the
[Customize an LLM by using supervised fine tuning](https://docs.cloud.google.com/bigquery/docs/generate-text-tuning)
how-to topic or the
[Use tuning and evaluation to improve model performance](https://docs.cloud.google.com/bigquery/docs/tune-evaluate)
tutorial.

These BigQuery ML features are
[generally available](https://cloud.google.com/products/#product-launch-stages)
(GA).

## November 06, 2024

Feature BigQuery now offers the following Gemini-enhanced SQL translation features:

- In interactive translation mode, you can use [Gemini-enhanced SQL translations](https://docs.cloud.google.com/bigquery/docs/interactive-sql-translator#customize) to customize translated GoogleSQL queries. This feature is [generally available](https://cloud.google.com/products#product-launch-stages) (GA).

- You can [generate AI suggestions for batch translations](https://docs.cloud.google.com/bigquery/docs/batch-sql-translator#submit_a_translation_job) using the Gemini model. The suggestions are based on a [Gemini-based configuration YAML file](https://docs.cloud.google.com/bigquery/docs/config-yaml-translation#ai_yaml_guidelines). This feature is in [Preview](https://cloud.google.com/products#product-launch-stages).

- After running an [interactive SQL translation](https://docs.cloud.google.com/bigquery/docs/interactive-sql-translator), you can request a [Gemini-generated text explanation](https://docs.cloud.google.com/bigquery/docs/interactive-sql-translator#explain_a_translation) that includes a summary of the translated SQL query. This feature is in [Preview](https://cloud.google.com/products#product-launch-stages).

## November 05, 2024

Announcement The [BigQuery Data Transfer Service data source change log](https://docs.cloud.google.com/bigquery/docs/transfer-changes) provides details about upcoming changes to data source schemas and schema mappings.
Feature [Dataplex automatic discovery](https://docs.cloud.google.com/bigquery/docs/automatic-discovery) lets you scan data in Cloud Storage buckets to extract and catalog metadata. Automatic discovery creates BigLake or external tables and object tables you can use for analytics and AI, and catalogs that data in Dataplex Catalog. This feature is available in [public preview](https://cloud.google.com/products#product-launch-stages).

## October 24, 2024

Feature BigQuery provides context-aware transformation recommendations from Gemini for cleansing data for analysis. [Data preparation](https://docs.cloud.google.com/bigquery/docs/data-prep-introduction) is available in [Preview](https://cloud.google.com/products#product-launch-stages).

## October 21, 2024

Feature You can now [view, trigger, and pause Airflow DAGs](https://docs.cloud.google.com/bigquery/docs/orchestrate-dags) in BigQuery. This feature is in [Preview](https://cloud.google.com/products#product-launch-stages).
Feature You can now [manage notebook schedules](https://docs.cloud.google.com/bigquery/docs/orchestrate-notebooks) on the Orchestration page. Notebook scheduling is in [Preview](https://cloud.google.com/products#product-launch-stages).
Feature [Custom organization policies](https://docs.cloud.google.com/bigquery/docs/transfer-custom-constraints) let you allow or deny specific operations on BigQuery Data Transfer Service transfer configurations to meet your organization's compliance and security requirements. This feature is [generally available](https://cloud.google.com/products#product-launch-stages) (GA).

## October 14, 2024

Feature You can now use [fine-grained DML](https://docs.cloud.google.com/bigquery/docs/data-manipulation-language#fine-grained_dml) to optimize the execution of `UPDATE`, `DELETE`, and `MERGE` statements on tables. This feature is in [Preview](https://cloud.google.com/products#product-launch-stages).

## October 11, 2024

Feature Use the [BigQuery migration assessment for Oracle](https://docs.cloud.google.com/bigquery/docs/migration-assessment) to assess the complexity of migrating data from your Oracle data warehouse to BigQuery. This feature is in [preview](https://cloud.google.com/products/#product-launch-stages).

## October 10, 2024

Feature You can now export and load [Parquet files](https://docs.cloud.google.com/bigquery/docs/loading-data-cloud-storage-parquet#geospatial_data) that include [GeoParquet](https://docs.cloud.google.com/bigquery/docs/geospatial-data#loading_geoparquet_files) metadata. This feature is [generally available](https://cloud.google.com/products#product-launch-stages) (GA).
Feature [BigQuery tables for Apache Iceberg](https://docs.cloud.google.com/bigquery/docs/iceberg-tables) bring the convenience of BigQuery storage optimization to Apache Iceberg tables that reside in your own cloud buckets. BigQuery tables for Apache Iceberg let you use BigQuery without moving data out of buckets that you control. This feature is now in [preview](https://cloud.google.com/products#product-launch-stages).

## October 08, 2024

Feature You can now use [pipe syntax](https://docs.cloud.google.com/bigquery/docs/pipe-syntax) anywhere you write GoogleSQL. Pipe syntax supports a linear query structure designed to make your queries easier to read, write, and maintain. This feature is in [Preview](https://cloud.google.com/products#product-launch-stages).

## October 03, 2024

Feature You can now create an [external dataset](https://docs.cloud.google.com/bigquery/docs/spanner-external-datasets) in BigQuery that links to an existing database in [Spanner](https://docs.cloud.google.com/spanner/docs). This feature is in [preview](https://cloud.google.com/products/#product-launch-stages).
Feature [ODBC driver update, release 3.0.7 1016](https://docs.cloud.google.com/bigquery/docs/reference/odbc-jdbc-drivers#current_odbc_driver)

- \[New\] Connector authentication on Google Cloud VMs: The connector now supports authentication through Application Default Credentials using the Google internal metadata server, eliminating the need for a keyfile. This feature works only on Google Cloud Compute Engine VMs.
- \[Resolved\] The output for PrimaryKeys previously denoted the Key Sequence as a 0-indexed value. This has been corrected to a 1-indexed value, indicating the sequential order of the primary key's column within the primary key itself.

## September 30, 2024

Feature You can now [enable, disable, and analyze history-based optimizations for queries](https://docs.cloud.google.com/bigquery/docs/history-based-optimizations). This feature is [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).
Feature You can now use the [operational health dashboard](https://docs.cloud.google.com/bigquery/docs/admin-resource-charts#monitor-operational-health) to get a single-pane view of key metrics such as slot usage, shuffle usage, errors, and total storage in real time. This feature is [generally available](https://cloud.google.com/products#product-launch-stages) (GA).
Feature You can now use [flexible column names](https://docs.cloud.google.com/bigquery/docs/schemas#flexible-column-names) with BigQuery tables and views for extracting, loading, streaming, and querying data. This feature is [generally available](https://cloud.google.com/products#product-launch-stages) (GA).
Feature You can now [create a materialized view replica](https://docs.cloud.google.com/bigquery/docs/materialized-view-replicas-create#create) directly from the Google Cloud console. This feature is [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).

## September 26, 2024

Feature **Cloud console updates** : You can now use [keyboard shortcuts](https://docs.cloud.google.com/bigquery/docs/bigquery-web-ui#keyboard_shortcuts) to control tab navigation in the details pane. This feature is [generally available](https://cloud.google.com/products#product-launch-stages) (GA).

## September 24, 2024

Feature You can now use [Cloud KMS Autokey](https://docs.cloud.google.com/kms/docs/autokey-overview) to automate the creation and use of [customer-managed encryption keys (CMEKs)](https://docs.cloud.google.com/bigquery/docs/customer-managed-encryption), including the [Cloud HSM](https://docs.cloud.google.com/kms/docs/hsm) service. This feature is [generally available (GA)](https://cloud.google.com/products/#product-launch-stages).
Feature BigQuery ML now offers the following AI features:

- You can process documents from BigQuery [object tables](https://docs.cloud.google.com/bigquery/docs/object-tables) by doing the following:

  1. Creating a [remote model](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model-service#remote_service_type) based on the [Document AI](https://docs.cloud.google.com/document-ai) API, including [specifying a document processor](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model-service#document_processor) to use.
  2. Using the [`ML.PROCESS_DOCUMENT` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-process-document) with a Document AI-based remote model to process the documents.

  Try this feature with the [Process documents with the `ML.PROCESS_DOCUMENT` function](https://docs.cloud.google.com/bigquery/docs/process-document) how-to.
- You can transcribe audio files from BigQuery [object tables](https://docs.cloud.google.com/bigquery/docs/object-tables) by doing the following:

  1. Creating a [remote model](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model-service#remote_service_type) based on the [Speech-to-Text](https://docs.cloud.google.com/speech-to-text) API, including[specifying a speech recognizer](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model-service#speech_recognizer) to use.
  2. Using the [`ML.TRANSCRIBE` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-transcribe) with a Speech-to-Text-based remote model to transcribe the audio files.

  Try this feature with the [Transcribe audio files with the `ML.TRANSCRIBE` function](https://docs.cloud.google.com/bigquery/docs/transcribe) how-to.

These BigQuery ML feature are [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).
Feature BigQuery ML now offers the following expanded embedding support features:

- Using the [`ML.GENERATE_EMBEDDING` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-generate-embedding) with a [remote model](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model) based on a [Vertex AI `multimodalembedding` large language model (LLM](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/learn/models#models)) to create multimodal embeddings, which embed text, image, and video into the same semantic space.
- Using the `ML.GENERATE_EMBEDDING` function with a [principal component analysis (PCA)](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-pca) model or [autoencoder](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-autoencoder) model to create embeddings for structured [independent and identically distributed random variables (IID)](https://en.wikipedia.org/wiki/Independent_and_identically_distributed_random_variables) data.
- Using the `ML.GENERATE_EMBEDDING` function with a [matrix factorization](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-matrix-factorization) model to create embeddings for user or item data.

Try these capabilities with the following tutorials:

- [Generate image embeddings by using the `ML.GENERATE_EMBEDDING` function](https://docs.cloud.google.com/bigquery/docs/generate-visual-content-embedding)
- [Generate video embeddings by using the `ML.GENERATE_EMBEDDING` function](https://docs.cloud.google.com/bigquery/docs/generate-video-embedding)
- [Generate text embeddings by using the `ML.GENERATE_EMBEDDING` function](https://docs.cloud.google.com/bigquery/docs/generate-text-embedding)
- [Generate and search multimodal embeddings](https://docs.cloud.google.com/bigquery/docs/generate-multimodal-embeddings)

These features are
[generally available](https://cloud.google.com/products/#product-launch-stages)
(GA).

## September 23, 2024

Feature You can now create [workflows](https://docs.cloud.google.com/bigquery/docs/workflows-introduction) to execute code assets in sequence at a scheduled time. This feature is in [Preview](https://cloud.google.com/products#product-launch-stages).

## September 19, 2024

Feature You can perform
[model monitoring](https://docs.cloud.google.com/bigquery/docs/model-monitoring-overview)
in BigQuery ML. The following model monitoring functions are now
[generally available](https://cloud.google.com/products/#product-launch-stages)
(GA):

- [`ML.DESCRIBE_DATA`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-describe-data): compute descriptive statistics for a set of training or serving data.
- [`ML.VALIDATE_DATA_SKEW`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-validate-data-skew): compute the statistics for a set of serving data, and then compare them to the statistics for the data used to train a BigQuery ML model in order to identify anomalous differences between the two data sets.
- [`ML.VALIDATE_DATA_DRIFT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-validate-data-drift): compute and compare the statistics for two sets of serving data in order to identify anomalous differences between the two data sets.
- [`ML.TFDV_DESCRIBE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-tfdv-describe): compute fine-grained descriptive statistics for a set of training or serving data. This function provides the same behavior as the [TensorFlow `tfdv.generate_statistics_from_csv` API](https://www.tensorflow.org/tfx/data_validation/api_docs/python/tfdv/generate_statistics_from_csv).
- [`ML.TFDV_VALIDATE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-tfdv-validate): compute and compare the statistics for training and serving data, or two sets of serving data, in order to identify anomalous differences between the two data sets. This function provides the same behavior as the [TensorFlow `tfdv.validate_statistics` API](https://www.tensorflow.org/tfx/data_validation/api_docs/python/tfdv/validate_statistics).

## September 16, 2024

Feature You can now [batch migrate classic saved queries to saved queries](https://docs.cloud.google.com/bigquery/docs/manage-saved-queries#migrate_classic_saved_queries). This feature is in [Preview](https://cloud.google.com/products#product-launch-stages) for projects that have fewer than 2500 classic saved queries.
Feature You can now use a
[`CREATE MODEL` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-contribution-analysis)
to create a
[contribution analysis](https://docs.cloud.google.com/bigquery/docs/contribution-analysis)
model in BigQuery ML. You can use a contribution analysis model with the
[`ML.GET_INSIGHTS` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-get-insights)
to generate insights about changes to key metrics in your multi-dimensional
data.

Try this feature with the
[Get data insights from a contribution analysis model](https://docs.cloud.google.com/bigquery/docs/get-contribution-analysis-insights)
tutorial.

This feature is in
[preview](https://products/#product-launch-stages).
Feature You can [store columns](https://docs.cloud.google.com/bigquery/docs/vector-index#stored-columns) in your vector indexes and pre-filter data in your [vector searches](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/search_functions#vector_search) to improve query efficiency. This feature is [Generally Available](https://cloud.google.com/products#product-launch-stages).

## September 12, 2024

Feature You can now use the [partial ordering mode in BigQuery DataFrames](https://docs.cloud.google.com/bigquery/docs/use-bigquery-dataframes#partial-ordering-mode) to generate more efficient queries. This feature is in [Preview](https://cloud.google.com/products#product-launch-stages).

## September 11, 2024

Feature You can now use Terraform to [manage IAM tags on datasets and tables](https://docs.cloud.google.com/bigquery/docs/tags). This feature is [generally available](https://cloud.google.com/products#product-launch-stages) (GA).

## September 09, 2024

Feature The BigQuery Data Transfer Service can now [transfer campaign reporting and configuration data from Display \& Video 360](https://docs.cloud.google.com/bigquery/docs/display-video-transfer) into BigQuery, including `Creative`, `Partner`, and `Advertiser` tables. This feature is [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).

## September 04, 2024

Feature You can now use vector search and vector index features in BigQuery.

You can use the
[`VECTOR_SEARCH` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/search_functions#vector_search)
to search embeddings in order to identify semantically similar entities.

You can use
[vector indexes](https://docs.cloud.google.com/bigquery/docs/vector-index)
to make `VECTOR_SEARCH` more efficient, with the trade-off of returning more
approximate results.

You can try the vector search and vector index capabilities by using the
[Search embeddings with vector search](https://docs.cloud.google.com/bigquery/docs/vector-search)
tutorial.

The BigQuery vector search and vector index features are
[generally available](https://cloud.google.com/products/#product-launch-stages)
(GA).

## August 29, 2024

Feature The BigQuery Data Transfer Service now supports [incremental transfers](https://docs.cloud.google.com/bigquery/docs/migration/teradata-overview#incremental) when you migrate your data from your Teradata data warehouses to BigQuery. This feature is [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).
Feature [Delta Lake BigLake tables](https://docs.cloud.google.com/bigquery/docs/create-delta-lake-table) are now [generally available](https://cloud.google.com/products/#product-launch-stages) (GA). Delta Lake is an open source, tabular data storage format that supports petabyte scale data tables.

## August 28, 2024

Feature Phrase support for the [`SEARCH` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/search_functions#search) is now [generally available](https://cloud.google.com/products#product-launch-stages) (GA).
Feature The following [Gemini in BigQuery](https://docs.cloud.google.com/gemini/docs/bigquery/overview) features are now [generally available](https://cloud.google.com/products#product-launch-stages) (GA):

- [Data insights](https://docs.cloud.google.com/bigquery/docs/data-insights)
- [Data canvas](https://docs.cloud.google.com/bigquery/docs/data-canvas)
- SQL and Python code assistance features:
  - [Use the SQL generation tool](https://docs.cloud.google.com/bigquery/docs/write-sql-gemini#use_the_sql_generation_tool)
  - [Prompt to generate SQL queries](https://docs.cloud.google.com/bigquery/docs/write-sql-gemini#prompt_to_generate_sql_queries)
  - [Explain a SQL query](https://docs.cloud.google.com/bigquery/docs/write-sql-gemini#explain_a_sql_query)
  - [Generate Python code](https://docs.cloud.google.com/bigquery/docs/write-sql-gemini#generate_python_code)
- [Partitioning and clustering recommendations](https://docs.cloud.google.com/bigquery/docs/manage-partition-cluster-recommendations)

To learn how to enable and activate Gemini in BigQuery features, see [Set up Gemini in BigQuery](https://docs.cloud.google.com/gemini/docs/bigquery/set-up-gemini).
Feature You can now use the `GROUP BY` clause and the `SELECT DISTINCT` clause with the `ARRAY` and `STRUCT` data types. This feature is in [Preview](https://cloud.google.com/products#product-launch-stages).
Feature You can now [query data in AlloyDB using a federated query](https://docs.cloud.google.com/bigquery/docs/alloydb-federated-queries). This feature is now [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).

## August 26, 2024

Feature You can now use `EXPORT DATA` statements to [directly export BigQuery data to Bigtable (reverse ETL)](https://docs.cloud.google.com/bigquery/docs/export-to-bigtable). This feature is [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).
Feature You can now create
[remote models](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model)
in BigQuery ML based on the
[Anthropic Claude](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/partner-models/use-claude)
model in Vertex AI.

Use the
[`ML.GENERATE_TEXT` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-generate-text) with these remote models to perform generative natural language tasks for text
stored in BigQuery tables. Try this feature with the
[Generate text by using the `ML.GENERATE_TEXT` function](https://docs.cloud.google.com/bigquery/docs/generate-text)
how-to topic.

This feature is in
[preview](https://cloud.google.com/products/#product-launch-stages).

## August 21, 2024

Feature [Python code completion](https://docs.cloud.google.com/bigquery/docs/write-sql-gemini#python_code_completion) is now available for all BigQuery projects. This feature is available in [preview](https://cloud.google.com/products#product-launch-stages). To learn how to enable and activate Gemini in BigQuery features, see [Set up Gemini in BigQuery](https://docs.cloud.google.com/gemini/docs/bigquery/set-up-gemini).

## August 20, 2024

Feature You can now perform
[anomaly detection](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-detect-anomalies)
with BigQuery ML
[multivariate time series (`ARIMA_PLUS_XREG`) models](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-multivariate-time-series).
This feature lets you detect anomalies in historical time series data or in new data with multiple feature columns. You can try this feature by using the
[Perform anomaly detection with a multivariate time-series forecasting model](https://docs.cloud.google.com/bigquery/docs/time-series-anomaly-detection-tutorial)
tutorial. This feature is
[generally available](https://cloud.google.com/products/#product-launch-stages)
(GA).

## August 19, 2024

Feature You can now view your BigQuery insights and recommendations using the [Recommendations page](https://docs.cloud.google.com/bigquery/docs/recommendations-intro#view_recommendations) in the Google Cloud console. You can also view your BigQuery insights and recommendations using the following `INFORMATION_SCHEMA` views:

- [`INSIGHTS`](https://docs.cloud.google.com/bigquery/docs/information-schema-insights)
- [`RECOMMENDATIONS`](https://docs.cloud.google.com/bigquery/docs/information-schema-recommendations)
- [`RECOMMENDATIONS_BY_ORGANIZATION`](https://docs.cloud.google.com/bigquery/docs/information-schema-recommendations-by-org)

These features are now in [preview](https://cloud.google.com/products#product-launch-stages).

## August 14, 2024

Feature You can now get lower latency for small queries with the new [short query optimized mode](https://docs.cloud.google.com/bigquery/docs/running-queries#optional-job-creation). BigQuery automatically determines which queries may be accelerated while other queries continue to run like before. This feature is now in [preview](https://cloud.google.com/products#product-launch-stages).

## August 12, 2024

Feature You can now use [time series](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/time-series-functions) and [range functions](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/range-functions) to support [time series analysis](https://docs.cloud.google.com/bigquery/docs/working-with-time-series). This feature is now [generally available](https://cloud.google.com/products#product-launch-stages) (GA).

## August 08, 2024

Feature The [`JSON_KEYS` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#json_keys), which extracts unique JSON keys from a JSON expression, is in [Preview](https://cloud.google.com/products#product-launch-stages).
Feature Some JSON functions that take a JSONPath let you specify a [mode](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#JSONPath_mode) that allows flexibility in how the JSONPath matches the JSON data structure. This feature is in [Preview](https://cloud.google.com/products#product-launch-stages).

## August 07, 2024

Feature You can now create a [materialized view over Apache Iceberg table that is partition aligned with the base table](https://docs.cloud.google.com/bigquery/docs/materialized-views-create#iceberg). The materialized view only supports time-based partition transformation, for example, `YEAR`, `MONTH`, `DAY`, and `HOUR`. This feature is in [preview](https://cloud.google.com/products#product-launch-stages).
Change An updated version of [JDBC driver for BigQuery](https://docs.cloud.google.com/bigquery/docs/reference/odbc-jdbc-drivers#current_jdbc_driver) is now available.

## July 31, 2024

Feature When you translate SQL queries from your source database, you can use configuration YAML files to [optimize and improve the performance of your translated SQL](https://docs.cloud.google.com/bigquery/docs/config-yaml-translation#optimize_and_improve_the_performance_of_translated_sql). This feature is in [preview](https://cloud.google.com/products/#product-launch-stages).
Feature [Workload management](https://docs.cloud.google.com/bigquery/docs/slots-autoscaling-intro) now provides the following benefits:

- The autoscaler now scales up immediately.
- The autoscaler now scales more precisely.
- The autoscaler scales to the nearest multiple of 50 slots, instead of 100.
- You can now purchase capacity commitments, set baseline slots, and set autoscale max slots in incremental steps of 50 slots.
- If one minute or more has passed since the most recent increase in capacity, you can now reduce capacity without resetting the one minute minimum. This allows for multiple consecutive decreases without a one minute delay between them.

These features are now [generally available](https://cloud.google.com/products#product-launch-stages) (GA).

## July 30, 2024

Feature You can now use the `output_dimensionality` argument of the
[`ML.GENERATE_EMBEDDING` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-generate-embedding#multimodalembedding)
when you use the function with a
[remote model](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model)
based on a
[Vertex AI `multimodalembedding` model](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/learn/models). The `output_dimensionality` argument lets you specify the number of dimensions
to use when generating embeddings. This feature is in [Preview](https://cloud.google.com/products/#product-launch-stages).

## July 29, 2024

Feature The `RANGE` data type is now a supported [JSON encoding](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#json_encodings). This feature is [Generally Available](https://cloud.google.com/products#product-launch-stages) (GA).
Feature You can now use the [administrative jobs explorer](https://docs.cloud.google.com/bigquery/docs/admin-jobs-explorer) to help you quickly monitor jobs activity across your organization. This feature is [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).
Feature Vector indexes support the [TreeAH index type](https://docs.cloud.google.com/bigquery/docs/vector-index#tree-ah-index), which uses Google's ScaNN algorithm. The TreeAH index is optimized for batch queries that process hundreds or more query vectors. This feature is in [Preview](https://cloud.google.com/products#product-launch-stages).

## July 25, 2024

Feature [IAM deny policies](https://docs.cloud.google.com/bigquery/docs/control-access-to-resources-iam#deny_access_to_a_resource) now support [additional permissions](https://docs.cloud.google.com/iam/docs/deny-permissions-support), including `bigquery.tables.getData` which can deny permission to read tables. Consider [special cases](https://docs.cloud.google.com/bigquery/docs/control-access-to-resources-iam#special_cases) when you create deny policies for `bigquery.tables.getData` and other BigQuery permissions. This feature is in [preview](https://cloud.google.com/products#product-launch-stages).
Feature You can now use [table explorer](https://docs.cloud.google.com/bigquery/docs/table-explorer) to examine table data and create data exploration queries. This feature is in [preview](https://cloud.google.com/products#product-launch-stages).

## July 23, 2024

Feature [Manifest files](https://docs.cloud.google.com/bigquery/docs/query-open-table-format-using-manifest-files) are now supported for Amazon S3 and Azure Blob Storage. This feature is [generally available](https://cloud.google.com/products) (GA).
Announcement Starting September 17, 2024, the `bigquery.datasets.update` permission check when creating or updating authorized datasets will be removed. For more information, see [Required permissions and roles for authorized datasets](https://docs.cloud.google.com/bigquery/docs/authorized-datasets#permissions_datasets).
Feature You can now [configure SAP Datasphere connections with network attachments](https://docs.cloud.google.com/bigquery/docs/connections-with-network-attachment) to help secure connections. SAP Datasphere connections are in [preview](https://cloud.google.com/products#product-launch-stages).

## July 22, 2024

Feature The BigQuery [continuous queries](https://docs.cloud.google.com/bigquery/docs/continuous-queries-introduction) feature is now in [preview](https://cloud.google.com/products/#product-launch-stages).

Continuous queries let you build long-lived, continuously processing SQL statements that can analyze, process, and perform machine learning (ML) inference on incoming data in BigQuery in real time. You can configure continuous queries to replicate query results to a Pub/Sub topic, Bigtable instance, or another BigQuery table, a process also known as Reverse ETL.

You can use continuous queries to perform the following tasks, using the accessible language of SQL:

- Transform incoming data and act immediately on insights.
- Use Vertex AI to apply real time ML insights.
- Build automated event-driven data pipelines.
- Replicate real-time events to downstream operational systems like Bigtable.

To try BigQuery continuous queries, see [Create continuous queries](https://docs.cloud.google.com/bigquery/docs/continuous-queries).
Feature You can now use BigQuery Omni Virtual Private Cloud (VPC) allowlists to restrict access to [AWS S3 buckets](https://docs.cloud.google.com/bigquery/docs/omni-aws-create-external-table#allow-vpc) and [Azure Blob Storage](https://docs.cloud.google.com/bigquery/docs/omni-azure-create-external-table#allow-vpc) from specific BigQuery Omni VPCs. This feature is in [preview](https://cloud.google.com/products#product-launch-stages).
Feature You can use data manipulation language (DML) to [modify rows that have been recently written to a BigQuery table](https://docs.cloud.google.com/bigquery/docs/write-api#use_data_manipulation_language_dml_with_recently_streamed_data) by the Storage Write API. This is now [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).
Feature The [`CHANGES` change history function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/table-functions-built-in#changes) is now in [preview](https://cloud.google.com/products/#product-launch-stages). This table-valued function provides a history of table changes over a window of time and captures the following operations:

- [`CREATE TABLE` DDL statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_table_statement)
- [`INSERT` DML statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/dml-syntax#insert_statement)
- [Data appended or changed as part of a `MERGE` DML statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/dml-syntax#merge_statement)
- [`UPDATE` DML statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/dml-syntax#update_statement)
- [`DELETE` DML statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/dml-syntax#delete_statement)
- [Loading data](https://docs.cloud.google.com/bigquery/docs/loading-data) into BigQuery
- [Streaming ingestion](https://docs.cloud.google.com/bigquery/docs/write-api#use_data_manipulation_language_dml_with_recently_streamed_data)
- [`TRUNCATE TABLE` DML statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/dml-syntax#truncate_table_statement)
- [Jobs](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/Job) configured with a `writeDisposition` of `WRITE_TRUNCATE`
- Individual [table partition deletions](https://docs.cloud.google.com/bigquery/docs/managing-partitioned-tables#delete_a_partition)

## July 18, 2024

Feature The following [BigQuery migration assessment](https://docs.cloud.google.com/bigquery/docs/migration-assessment) features are now [generally available](https://cloud.google.com/products/#product-launch-stages) (GA):

- When you [run a migration assessment](https://docs.cloud.google.com/bigquery/docs/migration-assessment#run_the_migration_assessment), the migration assessment now automatically creates a BigQuery dataset to store the assessment results. You can also choose to store assessment results in an existing empty dataset or manually create a dataset with a custom name.
- While a migration assessment is running, you can view the assessment report with partial data. You can also view its progress and estimated completion time in the status icon tooltip.
- You can view more information and errors about a migration assessment in the [assessment details](https://docs.cloud.google.com/bigquery/docs/migration-assessment#assessment_details) page.

## July 17, 2024

Feature You can now configure the [default storage billing model](https://docs.cloud.google.com/bigquery/docs/default-configuration) for new datasets. This feature is [generally available](https://cloud.google.com/products#product-launch-stages) (GA).

## July 16, 2024

Feature When you run a migration assessment for Amazon Redshift, Teradata, or Snowflake, the [service also creates a dataset containing only highly aggregated assessment results](https://docs.cloud.google.com/bigquery/docs/migration-assessment#shareable_aggregated_assessment_result). This aggregated dataset doesn't contain any query logs; therefore, no personally identifiable information (PII) or business-sensitive information is visible. You can [share this dataset](https://docs.cloud.google.com/bigquery/docs/migration-assessment#share_your_dataset_with_users_in_other_projects) with users that are not in your project. This feature is in [preview](https://cloud.google.com/products#product-launch-stages).

## July 11, 2024

Feature You can now use [EXPORT DATA](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/other-statements) statements to [reverse ETL BigQuery data to Spanner](https://docs.cloud.google.com/bigquery/docs/export-to-spanner). This feature is in [preview](https://cloud.google.com/products/#product-launch-stages).

## July 01, 2024

Feature The following [Analytics Hub features](https://docs.cloud.google.com/bigquery/docs/analytics-hub-manage-listings) are now [generally available](https://cloud.google.com/products#product-launch-stages):

- Making exchanges and listings publicly discoverable.
- Highlighting listings in the Featured section of the Analytics Hub catalog.
- Generating unauthenticated URLs for public listings.
Feature **Cloud console updates** : You can now [drag a tab](https://docs.cloud.google.com/bigquery/docs/bigquery-web-ui#details_panel) in the details pane to open a new column and compare tabs. You can also drag the tab to a new position in the current or an adjacent column. This feature is in [preview](https://cloud.google.com/products#product-launch-stages).
Feature Data publishers can now [share Pub/Sub topics and manage subscriptions in Analytics Hub](https://docs.cloud.google.com/bigquery/docs/analytics-hub-stream-sharing). This feature is in [preview](https://cloud.google.com/products/#product-launch-stages).

## June 27, 2024

Feature You can now use [tags](https://docs.cloud.google.com/bigquery/docs/tags) on BigQuery tables to conditionally grant or deny access with Identity and Access Management (IAM) policies. This feature is [generally available](https://cloud.google.com/products#product-launch-stages) (GA). You can also attach tags to BigQuery datasets during dataset creation to conditionally grant or deny access with IAM policies.

## June 25, 2024

Feature You can now use the [BigQuery JupyterLab plugin](https://docs.cloud.google.com/bigquery/docs/jupyterlab-plugin) to explore your data, use BigQuery DataFrames in a Jupyter notebook, and deploy a BigQuery DataFrames notebook to Cloud Composer. This feature is in [preview](https://cloud.google.com/products#product-launch-stages).

## June 21, 2024

Feature The [BigQuery migration assessment](https://docs.cloud.google.com/bigquery/docs/migration-assessment) for Amazon Redshift is now [generally available](https://cloud.google.com/products/#product-launch-stages) (GA). You can use this feature to assess the complexity of migrating from your Amazon Redshift data warehouse to BigQuery.

## June 18, 2024

Feature Additional [collation support](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/collation-concepts) for the [`NULLIF` conditional expression](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/conditional_expressions#nullif) has been added. The `NULLIF` conditional expression is now affected by collation and can be used in collation-supported comparisons with the `STRUCT` data type. This feature is [generally available](https://cloud.google.com/products#product-launch-stages) (GA).

## June 17, 2024

Announcement [Global rate limits on BigQuery Omni connection creation and use](https://docs.cloud.google.com/bigquery/quotas#connection_api) have replaced the regional limits on AWS and Azure connections.
Feature You can now perform
[supervised tuning](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model#supervised_tuning)
on a BigQuery ML remote model based on a
[`gemini-1.0-pro-002` model](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/model-reference/gemini).
This feature is in
[preview](https://cloud.google.com/products/#product-launch-stages).
To try this feature, see
[Tune a model using your data](https://docs.cloud.google.com/bigquery/docs/generate-text-tuning).

You can also perform supervised tuning by using the
[BigQuery DataFrames Python API](https://docs.cloud.google.com/python/docs/reference/bigframes/latest).
Use the `fit()` and `score()` methods in the
[`bigframes.ml.llm.GeminiTextGenerator` model class](https://docs.cloud.google.com/python/docs/reference/bigframes/latest/bigframes.ml.llm.GeminiTextGenerator)
to perform supervised tuning.

## June 13, 2024

Feature You can now [schedule notebooks](https://docs.cloud.google.com/bigquery/docs/manage-notebooks#schedule_notebooks). This feature is available in [preview](https://cloud.google.com/products#product-launch-stages).

## June 05, 2024

Feature The [slot recommender](https://docs.cloud.google.com/bigquery/docs/slot-recommender) for editions analyzes historical usage data to recommend optimal capacity purchasing for edition and on-demand workloads. This feature is [generally available](https://cloud.google.com/products#product-launch-stages) (GA).
Feature [Analytics Hub data egress](https://docs.cloud.google.com/bigquery/docs/analytics-hub-introduction#data_egress) controls are now [generally available](https://cloud.google.com/products#product-launch-stages) (GA). Publishers can now enforce egress restrictions on Analytics Hub listings to prevent subscribers from copying or exporting the shared data.
Change The BigQuery ML
[`ML.GENERATE_EMBEDDING` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-generate-embedding)
now supports the `output_dimensionality` argument for `text-embedding` and
`text-multilingual-embedding` models. The `output_dimensionality` argument lets
you specify the number of dimensions to use when generating embeddings.

## May 31, 2024

Feature You can now use [IAM conditions](https://docs.cloud.google.com/bigquery/docs/conditions) to control access to BigQuery resources. This feature is [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).

## May 30, 2024

Feature You can now define a [_CHANGE_SEQUENCE_NUMBER](https://docs.cloud.google.com/bigquery/docs/change-data-capture#manage_custom_ordering) for BigQuery change data capture (CDC) to manage streaming UPSERT ordering for BigQuery. This feature is in [preview](https://cloud.google.com/products#product-launch-stages).

## May 29, 2024

Change The [maximum number of partitions per partitioned table](https://docs.cloud.google.com/bigquery/quotas#partitioned_tables) limit has changed from 4,000 to 10,000.

## May 28, 2024

Feature The following Generative AI features are now in
[preview](https://cloud.google.com/products/#product-launch-stages):

- Creating [remote models](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model) based on the [Vertex AI gemini-1.5-flash foundation model](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/learn/models#gemini-models).
- Using the [`ML.GENERATE_TEXT` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-generate-text) with these remote models to perform generative natural language tasks for text stored in BigQuery tables.
- Using the [`ML.GENERATE_TEXT` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-generate-text) with these remote models to perform generative AI tasks, for example audio transcription or document classification, using image, video, audio, PDF, or text content stored in BigQuery [object tables](https://docs.cloud.google.com/bigquery/docs/object-table-introduction).

Try these features with the
[Generate text by using the `ML.GENERATE_TEXT` function](https://docs.cloud.google.com/bigquery/docs/generate-text)
how-to topic.

## May 23, 2024

Change In BigQuery ML
[univariate time series models](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-time-series),
the
[`FORECAST_LIMIT_LOWER_BOUND`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-time-series#forecast_limit_lower_bound)
and
[`FORECAST_LIMIT_UPPER_BOUND`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-time-series#forecast_limit_upper_bound)
parameters now work with the
[`TIME_SERIES_ID_COL`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-time-series#time_series_id_col)
parameter. The `FORECAST_LIMIT_LOWER_BOUND` and `FORECAST_LIMIT_UPPER_BOUND`
arguments let you set the lower and upper bounds of the forecasted values
returned by the model. Try this feature with the
[Limit forecasted values for a time series model](https://docs.cloud.google.com/bigquery/docs/arima-time-series-forecasting-with-limits-tutorial)
tutorial.
Feature BigQuery ML now offers the following Generative AI features:

- [Grounding](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/grounding/overview#ground-public)
  and
  [safety attributes](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/multimodal/configure-safety-attributes)
  when you use Vertex AI Gemini models with the
  [`ML.GENERATE_TEXT` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-generate-text):

  - Use the `ground_with_google_search` argument to perform grounding. Grounding lets the Gemini model use additional information from the internet when generating a response, in order to make model responses more specific and factual.
  - Use the `safety_settings` argument to configure safety attributes.The Gemini model filters the responses it returns based on the attributes you specify.
- Video embedding
  ([Preview](https://cloud.google.com/products/#product-launch-stages)).
  You can use the
  [`ML.GENERATE_EMBEDDING` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-generate-embedding)
  with a
  [remote model](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model)
  based on a
  [Vertex AI `multimodalembedding` model](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/learn/models#models)
  to create multimodal embeddings that include video embeddings.

  To try the new video embedding functionality, see
  [Generate video embeddings by using the `ML.GENERATE_EMBEDDING` function](https://docs.cloud.google.com/bigquery/docs/generate-video-embedding).

## May 22, 2024

Feature You can now [query data in AlloyDB using a federated query](https://docs.cloud.google.com/bigquery/docs/alloydb-federated-queries). This feature is in [preview](https://cloud.google.com/products/#product-launch-stages).
Feature The [interactive SQL translator](https://docs.cloud.google.com/bigquery/docs/interactive-sql-translator), the [translation API](https://docs.cloud.google.com/bigquery/docs/api-sql-translator), and the [batch SQL translator](https://docs.cloud.google.com/bigquery/docs/batch-sql-translator) features let you translate the following SQL dialects into GoogleSQL:

- IBM DB2 SQL
- Greenplum SQL
- SQLite

These features are in [preview](https://cloud.google.com/products/#product-launch-stages).

## May 21, 2024

Feature The following Generative AI features are now in
[preview](https://cloud.google.com/products/#product-launch-stages):

- Creating [remote models](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model) based on the [Vertex AI `gemini-1.5-pro` foundation model](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/learn/models#gemini-models).
- Using the [`ML.GENERATE_TEXT` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-generate-text) with these remote models to perform generative natural language tasks for text stored in BigQuery tables.
- Using the [`ML.GENERATE_TEXT` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-generate-text) with these remote models to perform generative AI tasks, for example audio transcription or document classification, using image, video, audio, PDF, or text content stored in BigQuery [object tables](https://docs.cloud.google.com/bigquery/docs/object-table-introduction).

Try these features with the
[Generate text by using the `ML.GENERATE_TEXT` function](https://docs.cloud.google.com/bigquery/docs/generate-text) how-to topic.

## May 20, 2024

Feature You can now use a [search index](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_search_index_statement) to optimize lookups on the `INT64` and `TIMESTAMP` data types. The feature is in [preview](https://cloud.google.com/products/#product-launch-stages).
Feature You can use [DLP functions](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/dlp_functions) to support encryption and decryption between [BigQuery](https://docs.cloud.google.com/bigquery/docs) and [Sensitive Data Protection](https://docs.cloud.google.com/sensitive-data-protection/docs), using AES-SIV. This feature is now [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).

## May 14, 2024

Feature You can now [create Gemini-enhanced translation rules](https://docs.cloud.google.com/bigquery/docs/interactive-sql-translator#create_a_translation_rule) to use with the [interactive SQL translator](https://docs.cloud.google.com/bigquery/docs/interactive-sql-translator). Translation rules let you customize and adjust the results of the interactive translator according to your SQL migration needs. This feature is in [preview](https://cloud.google.com/products/#product-launch-stages).

## May 13, 2024

Feature Phrase support for the [`SEARCH` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/search_functions#search) is in [preview](https://cloud.google.com/products#product-launch-stages).

## May 07, 2024

Feature [JavaScript user-defined aggregate functions (UDAFs)](https://docs.cloud.google.com/bigquery/docs/user-defined-aggregates) are in [preview](https://cloud.google.com/products#product-launch-stages). You can create a JavaScript UDAF with the [CREATE AGGREGATE FUNCTION](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#javascript-create-udaf-function) statement.
Feature You can now [store columns](https://docs.cloud.google.com/bigquery/docs/vector-index#stored-columns) in your vector indexes and pre-filter data in your [vector searches](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/search_functions#vector_search) to improve query efficiency. This feature is in [preview](https://cloud.google.com/products#product-launch-stages).

## May 06, 2024

Feature [BigQuery Managed Disaster Recovery](https://docs.cloud.google.com/bigquery/docs/managed-disaster-recovery) provides managed failover and redundant compute capacity for business critical workloads. It is intended for use in the case of a total region outage and is supported with the [BigQuery Enterprise Plus edition](https://docs.cloud.google.com/bigquery/docs/editions-intro) only. This feature is now available in [preview](https://cloud.google.com/products#product-launch-stages).
Feature You can now create [AWS Glue federated datasets](https://docs.cloud.google.com/bigquery/docs/glue-federated-datasets) using the the Google Cloud console. This feature is [generally available](https://cloud.google.com/products#product-launch-stages) (GA).

## May 02, 2024

Feature [Analytics Hub Subscription Management](https://docs.cloud.google.com/bigquery/docs/analytics-hub-manage-subscriptions) is [generally available](https://cloud.google.com/products#product-launch-stages) (GA). Data Publishers can now manage their subscriptions, view information about their subscribers, and revoke access to their data at any time.
Feature [Analytics Hub Provider Usage Metrics](https://docs.cloud.google.com/bigquery/docs/analytics-hub-monitor-listings) is now [generally available](https://cloud.google.com/products#product-launch-stages) (GA). The usage metrics include the following:

- Jobs that run against your shared data.
- The consumption details of your shared data by subscribers' projects and organizations.
- The number of rows and bytes processed by the job.

## April 30, 2024

Feature [AWS Glue federated datasets](https://docs.cloud.google.com/bigquery/docs/glue-federated-datasets) are now [generally available (GA)](https://cloud.google.com/products/#product-launch-stages).

An AWS Glue federated dataset is a connection at the dataset level between BigQuery and an existing database in AWS Glue.
Feature You can now specify translation configurations in the [BigQuery interactive SQL translator](https://docs.cloud.google.com/bigquery/docs/interactive-sql-translator#translate_a_query_with_additional_configurations) and use it to [debug batch SQL translator jobs](https://docs.cloud.google.com/bigquery/docs/batch-sql-translator#debug-interactive-translator). This feature is [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).
Feature The following BigQuery ML data preprocessing features are now
[generally available](https://cloud.google.com/products/#product-launch-stages)
(GA):

- The [`ML.TRANSFORM` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-transform), which you can use to preprocess feature data. This function processes input data by applying the data transformations captured in the [`TRANSFORM` clause](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create#transform) of an existing model.
- [Transform-only models](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-transform), which you can use to apply [preprocessing functions](https://docs.cloud.google.com/bigquery/docs/manual-preprocessing) to input data and return the preprocessed data. Transform-only models decouple data preprocessing from model training, making it easier for you to capture and reuse a set of data preprocessing rules.
Feature You can now reference [Iceberg tables in materialized views](https://docs.cloud.google.com/bigquery/docs/materialized-views-create#iceberg) instead of migrating that data to BigQuery-managed storage. This feature is in [preview](https://cloud.google.com/products#product-launch-stages).

## April 29, 2024

Feature You can now let users that are in Microsoft Entra groups [access BigQuery data in Power BI](https://docs.cloud.google.com/iam/docs/workforce-sign-in-power-bi) by using Workforce Identity Federation. This feature is [generally available](https://cloud.google.com/products#product-launch-stages).

## April 26, 2024

Feature [SQL code generation](https://docs.cloud.google.com/bigquery/docs/write-sql-gemini#generate_a_sql_query) is now available for all BigQuery projects. This feature is available in [preview](https://cloud.google.com/products#product-launch-stages). To learn how to enable and activate Gemini in BigQuery features, see [Set up Gemini in BigQuery](https://docs.cloud.google.com/gemini/docs/bigquery/set-up-gemini).

## April 25, 2024

Announcement BigQuery Studio is now available in the following regions:

- Johannesburg (africa-south1)
- Hong Kong (asia-east2)
- Seoul (asia-northeast3)
- Jakarta (asia-southeast2)
- Sydney (australia-southeast1)
- Madrid (europe-southwest1)
- Turin (europe-west12)
- Doha (me-central1)
- Dammam (me-central2)
- Montréal (northamerica-northeast1)
- N. Virginia (us-east4)
- Columbus (us-east5)
- Dallas (us-south1)
- Los Angeles (us-west2)
- Las Vegas (us-west4)

For more information, see [BigQuery Studio locations](https://docs.cloud.google.com/bigquery/docs/locations#bqstudio-loc).
Feature The BigQuery Data Transfer Service for Google Merchant Center supports the [Product Targeting report](https://docs.cloud.google.com/bigquery/docs/merchant-center-transfer#product-targeting).

## April 24, 2024

Feature [User-defined aggregate functions (UDAFs)](https://docs.cloud.google.com/bigquery/docs/user-defined-aggregates) that support SQL expressions are in [preview](https://cloud.google.com/products#product-launch-stages). You can create a UDAF with the [CREATE AGGREGATE FUNCTION](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#sql-create-udaf-function) statement.

## April 18, 2024

Feature The [quantified `LIKE` operator](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/operators#like_operator_quantified) is [generally available (GA)](https://cloud.google.com/products#product-launch-stages). With this operator, you can check a search value for matches against a list of patterns or an array of patterns, using one of these conditions:

- `LIKE ANY`: Checks if at least one pattern matches.
- `LIKE SOME`: Synonym for `LIKE ANY`.
- `LIKE ALL`: Checks if every pattern matches.

## April 17, 2024

Feature [More permissions](https://docs.cloud.google.com/iam/docs/deny-permissions-support) are now supported by [deny policies](https://docs.cloud.google.com/bigquery/docs/control-access-to-resources-iam#deny_access_to_a_resource). This feature is in [preview](https://cloud.google.com/products#product-launch-stages).

## April 16, 2024

Feature BigQuery now supports [subqueries](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/subqueries) in [row level access policies](https://docs.cloud.google.com/bigquery/docs/managing-row-level-security#create_or_update_a_row-level_access_policy). This feature is now in public [preview](https://cloud.google.com/products/#product-launch-stages).

## April 09, 2024

Feature You can now create a [data canvas](https://docs.cloud.google.com/bigquery/docs/data-canvas) in BigQuery Studio. A data canvas lets you discover, transform, query, and visualize data using natural language. It provides a graphic interface for your analysis that lets you work with data sources, queries, and visualizations in a directed acyclic graph (DAG), giving you a graphical view of your analysis workflow that maps to your mental model. You can iterate on query results and work with multiple branches of inquiry in a single place. This feature is in [preview](https://cloud.google.com/products/#product-launch-stages) and access can be requested [here](http://goo.gle/bqdc-request-access).
Feature BigQuery ML now offers the following expanded embedding support features in
[preview](https://cloud.google.com/products/#product-launch-stages):

- Using the [`ML.GENERATE_EMBEDDING` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-generate-embedding) with a [remote model](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model) based on a [Vertex AI `multimodalembedding` large language model (LLM)](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/learn/models#imagen-models) to create multimodal embeddings, which embed text and images into the same semantic space.
- Using the `ML.GENERATE_EMBEDDING` function with a [principal component analysis (PCA)](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-pca) model or [autoencoder](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-autoencoder) model to create embeddings for structured [independent and identically distributed random variables (IID)](https://en.wikipedia.org/wiki/Independent_and_identically_distributed_random_variables) data.
- Using the `ML.GENERATE_EMBEDDING` function with a [matrix factorization](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-matrix-factorization) model to create embeddings for user or item data.

Try the new multimodal embedding functionality:

- [Generate image embeddings by using the `ML.GENERATE_EMBEDDING` function](https://docs.cloud.google.com/bigquery/docs/generate-visual-content-embedding)
- [Generate text embeddings by using the `ML.GENERATE_EMBEDDING` function](https://docs.cloud.google.com/bigquery/docs/generate-text-embedding)
- [Generate and search multimodal embeddings](https://docs.cloud.google.com/bigquery/docs/generate-multimodal-embeddings)
Feature The following [Gemini in BigQuery](https://docs.cloud.google.com/gemini/docs/bigquery/overview) features are now available in [Public Preview](https://cloud.google.com/products#product-launch-stages):

- [Data insights](https://docs.cloud.google.com/bigquery/docs/data-insights): an automated and intuitive way to explore and understand your data.
- [Data canvas](https://docs.cloud.google.com/bigquery/docs/data-canvas): a graphic interface that lets you discover, transform, query, and visualize data using natural language.
- [SQL and Python code assistance](https://docs.cloud.google.com/bigquery/docs/write-sql-gemini): Gemini-assisted code generation, completion, and explanation.
- [Materialized views](https://docs.cloud.google.com/bigquery/docs/manage-materialized-recommendations), [partitioning, and clustering](https://docs.cloud.google.com/bigquery/docs/view-partition-cluster-recommendations) recommendations: recommendations to reduce cost and improve performance.
- [Autotune](https://docs.cloud.google.com/dataproc-serverless/docs/concepts/autotuning) and [troubleshoot](https://docs.cloud.google.com/dataproc-serverless/docs/guides/monitor-troubleshoot-batches#advanced-troubleshooting) serverless Spark: optimize and explain Spark workloads.

To learn how to enable and activate Gemini in BigQuery features, see [Set up Gemini in BigQuery](https://docs.cloud.google.com/gemini/docs/bigquery/set-up-gemini).

## April 08, 2024

Feature [BigQuery Studio](https://docs.cloud.google.com/bigquery/docs/query-overview#bigquery-studio) is [generally available (GA)](https://cloud.google.com/products#product-launch-stages).

BigQuery Studio lets you save, share, and manage versions of code assets such as [notebooks](https://docs.cloud.google.com/bigquery/docs/notebooks-introduction) and [saved queries](https://docs.cloud.google.com/bigquery/docs/saved-queries-introduction).
Feature [BigQuery DataFrames](https://docs.cloud.google.com/bigquery/docs/bigquery-dataframes-introduction) is [generally available (GA)](https://cloud.google.com/products#product-launch-stages).

BigQuery DataFrames is a set of open source Python libraries that implements the `pandas` and `scikit-learn` APIs with server-side processing. To get started, you can [try BigQuery DataFrames](https://docs.cloud.google.com/bigquery/docs/dataframes-quickstart).
Feature The [BigQuery materialized view recommender](https://docs.cloud.google.com/bigquery/docs/manage-materialized-recommendations) analyzes your past query jobs to identify opportunities to apply materialized views to your queries for potential cost savings. You can view all available materialized view recommendations through the BigQuery UI or Recommender API. This feature is in [preview](https://cloud.google.com/products/#product-launch-stages).

## April 05, 2024

Feature [Differential privacy](https://docs.cloud.google.com/bigquery/docs/differential-privacy) is now [generally available (GA)](https://cloud.google.com/products/#product-launch-stages).
Feature You can now use BigLake to access Delta Lake tables. For more information, see [Create Delta Lake BigLake tables](https://docs.cloud.google.com/bigquery/docs/create-delta-lake-table). This feature is available in [preview](https://cloud.google.com/products/#product-launch-stages).

## April 04, 2024

Feature [Join restrictions](https://docs.cloud.google.com/bigquery/docs/analysis-rules#join_restriction_rules), [list overlap](https://docs.cloud.google.com/bigquery/docs/analysis-rules#list_overlap_rules), [differential privacy with privacy budgeting](https://docs.cloud.google.com/bigquery/docs/analysis-rules#dp_analysis_rules), and [aggregation thresholding](https://docs.cloud.google.com/bigquery/docs/analysis-rules#agg_analysis_rules) are now enforceable in BigQuery data clean rooms using analysis rules.
Feature [BigQuery data clean rooms](https://docs.cloud.google.com/bigquery/docs/data-clean-rooms) with analysis rules and enhanced usage metrics are now [generally available (GA)](https://cloud.google.com/products/#product-launch-stages). Data clean rooms provide a security-enhanced and privacy-preserving environment for multiple parties to share and augment data without moving or revealing the underlying data.
Feature You can now perform
[model monitoring](https://docs.cloud.google.com/bigquery/docs/model-monitoring-overview) in BigQuery ML. The following model monitoring functions are now in
[preview](https://cloud.google.com/products/#product-launch-stages):

- [`ML.DESCRIBE_DATA`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-describe-data): compute descriptive statistics for a set of training or serving data.
- [`ML.VALIDATE_DATA_SKEW`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-validate-data-skew): compute the statistics for a set of serving data, and then compare them to the statistics for the data used to train a BigQuery ML model in order to identify anomalous differences between the two data sets.
- [`ML.VALIDATE_DATA_DRIFT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-validate-data-drift): compute and compare the statistics for two sets of serving data in order to identify anomalous differences between the two data sets.
- [`ML.TFDV_DESCRIBE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-tfdv-describe): compute fine-grained descriptive statistics for a set of training or serving data. This function provides the same behavior as the [TensorFlow `tfdv.generate_statistics_from_csv` API](https://www.tensorflow.org/tfx/data_validation/api_docs/python/tfdv/generate_statistics_from_csv).
- [`ML.TFDV_VALIDATE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-tfdv-validate): compute and compare the statistics for training and serving data, or two sets of serving data, in order to identify anomalous differences between the two data sets. This function provides the same behavior as the [TensorFlow `validate_statistics` API](https://www.tensorflow.org/tfx/data_validation/api_docs/python/tfdv/validate_statistics).
Feature The [`allow_non_incremental_definition` option](https://docs.cloud.google.com/bigquery/docs/materialized-views-create#non-incremental) and [`max_staleness` option](https://docs.cloud.google.com/bigquery/docs/materialized-views-create#max_staleness) for materialized views are now [generally available (GA)](https://cloud.google.com/products/#product-launch-stages). The `allow_non_incremental_definition` option supports an expanded range of SQL queries to create materialized views, and the `max_staleness` option provides consistently high performance with controlled costs when processing large, frequently changing datasets.

## April 03, 2024

Feature You can now configure materialized views with tables enabled for [change data capture (CDC)](https://docs.cloud.google.com/bigquery/docs/change-data-capture) streaming update and delete operations.
Feature [Collation](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/collation-concepts) now supports the following [generally available](https://cloud.google.com/products/#product-launch-stages) (GA) features:

- The underscore in the [`LIKE` operator](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/operators#like_operator).
- Comparison support for the [`STRUCT` data type](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/collation-concepts#collate_data_types) with the following operators and conditional expressions: `=`, `!=`, `IN` and `CASE`.

## April 02, 2024

Feature The following BigQuery ML features are now in
[preview](https://cloud.google.com/products/#product-launch-stages):

- Performing [supervised tuning](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model#supervised_tuning) on a remote model based on a [Vertex AI `text-bison` large language model (LLM)](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/learn/models#palm-models).
- Evaluate a Vertex AI LLM using the [`ML.EVALUATE` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-evaluate). Pre-trained [`text-bison`, `text unicorn`, or `gemini-pro`](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/learn/models) models and tuned `text-bison` models are supported for evaluation.

Try tuning and evaluating an LLM with the
[Customize an LLM by using supervised fine tuning](https://docs.cloud.google.com/bigquery/docs/generate-text-tuning)
how-to topic.

## April 01, 2024

Feature You can now enable, disable, and analyze
[history-based optimizations for queries](https://docs.cloud.google.com/bigquery/docs/history-based-optimizations). This feature is in [preview](https://cloud.google.com/products/#product-launch-stages).
Feature The [BigQuery Data Transfer Service for Search Ads 360](https://docs.cloud.google.com/bigquery/docs/search-ads-transfer) now supports the new Search Ads 360 Reporting API. You can use the Search Ads 360 connector to specify custom Floodlight variables and custom columns when transferring Search Ads 360 data to BigQuery. This feature is now [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).
Announcement BigQuery Studio is now available in the [South Carolina (us-east1) region](https://docs.cloud.google.com/bigquery/docs/locations#bqstudio-loc) to manage versions of code assets such as notebooks and saved queries.

## March 28, 2024

Feature [Query optimization using search indexes](https://docs.cloud.google.com/bigquery/docs/search#operator_and_function_optimization) is now applied to comparisons of string literals and indexed data, including the equal (`=`), `IN`, and `LIKE` operators and the `STARTS_WITH` function. This feature is [generally available](https://cloud.google.com/products#product-launch-stages) (GA).
Feature You can now [query data in SAP Datasphere using a federated query](https://docs.cloud.google.com/bigquery/docs/sap-datasphere-federated-queries). This feature is in [preview](https://cloud.google.com/products/#product-launch-stages).

## March 27, 2024

Change An updated version of [JDBC driver for BigQuery](https://docs.cloud.google.com/bigquery/docs/reference/odbc-jdbc-drivers#current_jdbc_driver) is now available.

## March 26, 2024

Feature The [Help me code tool](https://docs.cloud.google.com/bigquery/docs/write-sql-duet-ai#use_the_help_me_code_tool) lets you use natural language to generate a SQL query that can then be run in BigQuery. This feature is now in [preview](https://cloud.google.com/products/#product-launch-stages).
Feature The following Generative AI features are now in
[preview](https://cloud.google.com/products/#product-launch-stages):

- Creating a [remote model](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model) based on a [Vertex AI gemini-pro-vision large vision model (VLM)](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/learn/models#gemini-models).
- Using the [`ML.GENERATE_TEXT` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-generate-text) with this remote model to perform Vision Generative AI tasks, such as image or video captioning and visual Q\&A, for visual content stored in BigQuery [object tables](https://docs.cloud.google.com/bigquery/docs/object-table-introduction).

Try these features with the
[Generate text that describes visual content](https://docs.cloud.google.com/bigquery/docs/generate-text#generate_text_that_describes_visual_content)
how-to topic.
Announcement Duet AI in BigQuery is now Gemini for BigQuery. See our [blog post](https://blog.google/technology/ai/google-gemini-update-sundar-pichai-2024/) for more information.

## March 22, 2024

Change The [March 20, 2024 release notes](https://docs.cloud.google.com/bigquery/docs/release-notes#March_20_2024) announced the preview for user-defined aggregate functions, but user-defined aggregate functions are not yet supported.

## March 21, 2024

Feature You can now add [Salesforce Data Cloud](https://docs.cloud.google.com/bigquery/docs/salesforce-quickstart) data to BigQuery. This feature is [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).
Feature Incremental materialized views now support [`LEFT OUTER JOIN` and `UNION ALL`](https://docs.cloud.google.com/bigquery/docs/materialized-views-create#left-union). This feature is in [preview](https://cloud.google.com/products/#product-launch-stages).

## March 20, 2024

Feature The [maximum notebook size](https://docs.cloud.google.com/bigquery/quotas#notebook_limits) has been increased from 10 MB to 20 MB. Notebooks are available in [preview](https://cloud.google.com/products/#product-launch-stages).
Feature You can now view lists of [all saved queries](https://docs.cloud.google.com/bigquery/docs/manage-saved-queries#view_all_saved_queries) and [all notebooks](https://docs.cloud.google.com/bigquery/docs/manage-notebooks#view_all_notebooks) in your project. These features are available in [preview](https://cloud.google.com/products/#product-launch-stages).

## March 19, 2024

Feature You can now create and run [Spark stored procedures](https://docs.cloud.google.com/bigquery/docs/spark-procedures) that are written in Python, Java, and Scala. You can also use the [PySpark editor in BigQuery](https://docs.cloud.google.com/bigquery/docs/spark-procedures#use-python-pyspark-editor) to create stored Python procedures for Apache Spark. This feature is now [generally available](https://cloud.google.com/products#product-launch-stages) (GA).
Feature The minimum duration between [scheduled queries](https://docs.cloud.google.com/bigquery/docs/scheduling-queries#set_up_scheduled_queries) has been reduced from 15 minutes to 5 minutes. This feature is [generally available](https://cloud.google.com/products#product-launch-stages).

## March 18, 2024

Feature You can now perform hierarchical forecasts in BigQuery ML
[time series models](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-time-series#hierarchical_time_series_cols),
which let you aggregate and roll up values for all time series in the model.
This feature is
[generally available](https://cloud.google.com/products/#product-launch-stages)
(GA).
Feature You can now [undelete a dataset](https://docs.cloud.google.com/bigquery/docs/managing-datasets#undelete_datasets) that is within your time travel window to recover it to the state that it was in when it was deleted. This feature is in [preview](https://cloud.google.com/products/#product-launch-stages).
Feature These BigQuery features are now
[generally available](https://cloud.google.com/products/#product-launch-stages)
(GA):

[Text analysis configuration options](https://docs.cloud.google.com/bigquery/docs/text-analysis-search)
for the following:

- [`CREATE SEARCH INDEX` DDL](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_search_index_statement)
- Existing [`LOG_ANALYZER`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/text-analysis#log_analyzer) and new [`PATTERN_ANALYZER`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/text-analysis#pattern_analyzer) analyzers, which are used in various functions, including [`SEARCH`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/search_functions#search)
- The [`TEXT_ANALYZE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/text-analysis-functions#text_analyze) function

The following advanced processing functions:

- [`ML.BAG_OF_WORDS`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-bag-of-words)
- [`ML.TF_IDF`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-tf-idf)
- [`BAG_OF_WORDS`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/text-analysis-functions#bag_of_words)
- [`TF_IDF`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/text-analysis-functions#tf_idf)
- [`COSINE_DISTANCE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#cosine_distance)
- [`EUCLIDEAN_DISTANCE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#euclidean_distance)
- [`EDIT_DISTANCE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#edit_distance)

## March 06, 2024

Feature [Duet AI in BigQuery](https://docs.cloud.google.com/bigquery/docs/write-sql-duet-ai#generate_python_code) can now assist with Python code generation and code completion. This feature is in [preview](https://cloud.google.com/products/#product-launch-stages).
Feature The [INFORMATION_SCHEMA.WRITE_API_TIMELINE\*](https://docs.cloud.google.com/bigquery/docs/information-schema-write-api) views, containing per minute aggregated BigQuery Storage Write API ingestion statistics, are now [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).

## March 04, 2024

Feature You can now selectively refresh the metadata cache for BigLake tables by using the
[`BQ.REFRESH_EXTERNAL_METADATA_CACHE` system procedure](https://docs.cloud.google.com/bigquery/docs/reference/system-procedures#bqrefresh_external_metadata_cache).
This feature is
[generally available](https://cloud.google.com/products/#product-launch-stages)
(GA).

## February 29, 2024

Feature The following BigQuery cross-cloud features are now
[generally available](https://cloud.google.com/products/#product-launch-stages)
(GA):

- You can take advantage of the benefits of [materialized views over Amazon S3 metadata cache-enabled BigLake tables](https://docs.cloud.google.com/bigquery/docs/materialized-views-intro#biglake).
- You can create [materialized view replicas](https://docs.cloud.google.com/bigquery/docs/materialized-views-intro#materialized_view_replicas) of materialized views over Amazon S3 metadata cache-enabled Biglake tables. Materialized view replicas let you use the materialized view data in queries while avoiding data egress costs and improving query performance.
- You can [get information about materialized view replicas](https://docs.cloud.google.com/bigquery/docs/materialized-view-replicas-manage#get-info) by using SQL, the bq command-line tool, or the BigQuery API.
- You can use [cross-cloud joins](https://docs.cloud.google.com/bigquery/docs/biglake-intro#cross-cloud_joins) to run queries that span both Google Cloud and BigQuery Omni regions.
Feature The [SQL translation API](https://docs.cloud.google.com/bigquery/docs/api-sql-translator) combines the interactive and batch translator into a single workflow, improving the efficiency and stability of your translation jobs created using the API. This feature is available in [preview](https://cloud.google.com/products#product-launch-stages).

## February 28, 2024

Feature The following statements are now [generally available](https://cloud.google.com/products/#product-launch-stages) (GA) with billing enabled:

- `CREATE TABLE AS SELECT`
- `CREATE TABLE IF NOT EXISTS AS SELECT`
- `CREATE OR REPLACE TABLE AS SELECT`
- `INSERT INTO SELECT`

These statements let you [filter data from files in Amazon S3 and Azure Blob Storage](https://docs.cloud.google.com/bigquery/docs/load-data-using-cross-cloud-transfer#filter-data) before transferring results into BigQuery tables.
Feature Materialized views can now [reference logical views](https://docs.cloud.google.com/bigquery/docs/materialized-views-create#reference_logical_views). This feature is in [preview](https://cloud.google.com/products#product-launch-stages).
Feature The ability to perform
[anomaly detection](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-detect-anomalies)
with BigQuery ML
[multivariate time series (`ARIMA_PLUS_XREG`) models](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-multivariate-time-series)
is now in
[preview](https://cloud.google.com/products/#product-launch-stages).
This feature enables you to detect anomalies in historical time series data or
in new data with multiple feature columns. Try this new feature by using the
[Perform anomaly detection with a multivariate time-series forecasting model](https://docs.cloud.google.com/bigquery/docs/time-series-anomaly-detection-tutorial)
tutorial.

## February 27, 2024

Feature You can now [use data manipulation language (DML) statements to efficiently delete entire partitions](https://cloud.google.com/bigquery/docs/using-dml-with-partitioned-tables#using_dml_delete_to_delete_partitions). If a `DELETE` statement targets all rows in a partition, then the entire partition is deleted without scanning bytes or consuming slots. This feature is now[generally available (GA)](https://cloud.google.com/products/#product-launch-stages).
Feature You can now use [time series](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/time-series-functions) and [range](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/range-functions) functions to support [time series](https://docs.cloud.google.com/bigquery/docs/working-with-time-series) analysis. This feature is in [preview](https://cloud.google.com/products#product-launch-stages).

## February 26, 2024

Feature The [`GROUP BY ALL` clause](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#group_by_all), which groups rows by inferring grouping keys from the `SELECT` items, is now in [preview](https://cloud.google.com/products#product-launch-stages).
Feature The following SQL features are now [generally available](https://cloud.google.com/products#product-launch-stages) (GA):

- [`GROUP BY GROUPING SETS` clause](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#group_by_grouping_sets): Produces aggregated data for one or more grouping sets.
- [`GROUP BY CUBE` clause](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#group_by_cube): Produces aggregated data for all grouping set permutations.
- [`GROUPING` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#grouping): Checks if a groupable value in the `GROUP BY` clause is aggregated.
Feature The BigQuery Data Transfer Service can now transfer data from the following data sources:

- [Facebook Ads](https://docs.cloud.google.com/bigquery/docs/facebook-ads-transfer)
- [Oracle](https://docs.cloud.google.com/bigquery/docs/oracle-transfer)
- [Salesforce](https://docs.cloud.google.com/bigquery/docs/salesforce-transfer)
- [Salesforce Marketing Cloud](https://docs.cloud.google.com/bigquery/docs/sfmc-transfer)
- [ServiceNow](https://docs.cloud.google.com/bigquery/docs/servicenow-transfer)

Transfers from these data sources are supported in [preview](https://cloud.google.com/products#product-launch-stages).

## February 22, 2024

Feature The following BigQuery text embedding features are now
[generally available](https://cloud.google.com/products/#product-launch-stages)
(GA):

- Creating a BigQuery ML remote model that references a Vertex AI `textembedding-gecko*` text embedding model.
- Using the [`ML.GENERATE_EMBEDDING` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-generate-embedding) with the remote model to [embed text stored in BigQuery](https://docs.cloud.google.com/bigquery/docs/generate-text-embedding).
- Generating text embeddings with the [NNLM, SWIVEL, and BERT TensorFlow models](https://docs.cloud.google.com/bigquery/docs/generate-embedding-with-tensorflow-models).

## February 15, 2024

Feature After you run a query in the query editor, in the **Chart** tab, you can now see [a visualization of your query results](https://docs.cloud.google.com/bigquery/docs/running-queries#queries). This feature is [generally available](https://cloud.google.com/products#product-launch-stages) (GA).
Feature The following Generative AI features are now
[generally available](https://cloud.google.com/products/#product-launch-stages)
(GA):

- Creating a [remote model](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model#remote_service_type) based on the [`gemini-pro`](https://docs.cloud.google.com/vertex-ai/docs/generative-ai/learn/models#gemini-models) Vertex AI large language model (LLM).
- Using the [`ML.GENERATE_TEXT` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-generate-text) with a remote model based upon `gemini-pro` to perform generative natural language tasks on text stored in BigQuery tables.
- Use the BigQuery DataFrames [`GeminiTextGenerator` class](https://docs.cloud.google.com/python/docs/reference/bigframes/latest/bigframes.ml.llm.GeminiTextGenerator) in the [`bigframes.ml.llm` module](https://docs.cloud.google.com/python/docs/reference/bigframes/latest/bigframes.ml.llm) to create estimator-like Gemini text generator models.

## February 08, 2024

Feature BigQuery now offers [entity resolution](https://docs.cloud.google.com/bigquery/docs/entity-resolution-intro). This feature lets users match records across datasets even when a common identifier is missing. It utilizes an identity provider for this process; BigQuery supports LiveRamp and provides a framework for other identity providers to offer similar services. This feature is [generally available](https://cloud.google.com/products#product-launch-stages) (GA).
Feature [Custom data masking](https://docs.cloud.google.com/bigquery/docs/user-defined-functions#custom-mask) is now [generally available](https://cloud.google.com/products#product-launch-stages) (GA). You can define custom masking routines for custom masking capabilities such as salt based hash. The feature is available on the [Enterprise Plus edition](https://docs.cloud.google.com/bigquery/docs/editions-intro).

## February 07, 2024

Feature You can now view query plans to see [details of SQL pushdowns in federated queries](https://docs.cloud.google.com/bigquery/docs/query-plan-explanation#explanation_for_federated_queries). This feature is now [generally available](https://cloud.google.com/products/#product-launch-stages).

## February 06, 2024

Announcement [Billing for Spark stored procedures](https://docs.cloud.google.com/bigquery/docs/spark-procedures#pricing) begins on March 12, 2024. Until that date, Spark stored procedures are offered at no extra cost.

## January 31, 2024

Feature BigQuery now supports vector search and vector indexes. These features are in
[preview](https://cloud.google.com/products#product-launch-stages).

You can use the
[`VECTOR_SEARCH` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/search_functions#vector_search)
to search embeddings in order to identify semantically similar entities.

You can use
[vector indexes](https://docs.cloud.google.com/bigquery/docs/vector-index)
to make `VECTOR_SEARCH` more efficient, with the trade-off of returning more
approximate results.

Try the new vector search and vector index capabilities with the
[Search embeddings with vector search](https://docs.cloud.google.com/bigquery/docs/vector-search)
tutorial.
Feature The following information schema views display the history of configuration changes to the options of your organization and projects:

- [`ORGANIZATION_OPTIONS_CHANGES view`](https://docs.cloud.google.com/bigquery/docs/information-schema-organization-options-changes) displays the configuration changes to an organization, including all organization and project-level changes.
- [`PROJECT_OPTIONS_CHANGES view`](https://docs.cloud.google.com/bigquery/docs/information-schema-project-options-changes) displays the configuration changes to a project.

This feature is now in [preview](https://cloud.google.com/products/#product-launch-stages).

## January 29, 2024

Feature You can now use [tags](https://docs.cloud.google.com/bigquery/docs/tags) on BigQuery tables to conditionally grant or deny access with Identity and Access Management (IAM) policies. This feature is in [preview](https://cloud.google.com/products/#product-launch-stages).
Feature **Cloud console updates** : You can now sort query results by column. Click **Open sort menu** next to the column name and select a sort order. This feature is [generally available](https://cloud.google.com/products#product-launch-stages) (GA).

## January 24, 2024

Feature BigQuery now natively supports the Delta Lake format for [Amazon S3](https://docs.cloud.google.com/bigquery/docs/omni-aws-create-external-table#delta-lake-tables) and [Azure](https://docs.cloud.google.com/bigquery/docs/omni-azure-create-external-table#delta-lake-tables) tables. This feature is now in [preview](https://cloud.google.com/products/#product-launch-stages).
Change To improve BigQuery ML training performance, the
[`APPROX_GLOBAL_FEATURE_CONTRIB` argument](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create#approx_global_feature_contrib)
now defaults to `TRUE` when you set the `ENABLE_GLOBAL_EXPLAIN`
argument to `TRUE`, and you set the `NUM_PARALLEL_TREE` argument to greater than
10 for
[boosted tree models](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-boosted-tree)
or greater than 50 for
[random forest models](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-random-forest).
Feature BigQuery ML has added a new `residual` column to the output of the
[`ML.EXPLAIN_FORECAST` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-explain-forecast) for
[`ARIMA_PLUS`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-time-series)
and
[`ARIMA_PLUS_XREG`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-multivariate-time-series) models. The `residual` column contains the difference between the actual time
series and the fitted time series for the historical data. This lets you compare
the modeled historical data that is returned in the other output columns of
`ML.EXPLAIN_FORECAST` with the actual historical data.
Feature BigQuery now supports the [`ST_LINEINTERPOLATEPOINT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_lineinterpolatepoint) geography function, which gets a point at a specific fraction in a linestring. This feature is [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).

## January 22, 2024

Change BigQuery is now available in the [Berlin (europe-west10) region](https://docs.cloud.google.com/bigquery/docs/locations).

## January 16, 2024

Feature You can now use [cross-cloud joins](https://docs.cloud.google.com/bigquery/docs/biglake-intro#cross-cloud_joins) to run queries that span both Google Cloud and BigQuery Omni regions. This feature is in [preview](https://cloud.google.com/products/#product-launch-stages).

## January 02, 2024

Feature [Analytics Hub listings](https://docs.cloud.google.com/bigquery/docs/analytics-hub-manage-listings) can now include data encrypted with customer-managed encryption keys (CMEK). This feature is [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).

## December 18, 2023

Feature [Operational Health administrative resource charts](https://docs.cloud.google.com/bigquery/docs/admin-resource-charts#monitor-operational-health) are now in [preview](https://cloud.google.com/products/#product-launch-stages). You can use charts to view slot and shuffle usage, job concurrency, errors, and other metrics.

## December 14, 2023

Feature The BigQuery Data Transfer Service now supports [federated workforce identities](https://docs.cloud.google.com/iam/docs/workforce-identity-federation) when creating a data transfer from most data sources. This feature is [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).
Feature The [Apache Hive connector](https://docs.cloud.google.com/bigquery/docs/programmatic-analysis#apache_hadoop_apache_spark_and_apache_hive) is now [generally available](https://cloud.google.com/products/#product-launch-stages) (GA) for data analytics pipeline migration.

## December 12, 2023

Feature The following BigQuery cross-cloud features are now in
[preview](https://cloud.google.com/products/#product-launch-stages):

- You can now take advantage of the benefits of [materialized views over Amazon S3 metadata cache-enabled BigLake tables](https://docs.cloud.google.com/bigquery/docs/materialized-views-intro#biglake).
- You can create [materialized view replicas](https://docs.cloud.google.com/bigquery/docs/materialized-views-intro#materialized_view_replicas) of materialized views over Amazon S3 metadata cache-enabled BigLake tables. Materialized view replicas let you use the materialized view data in queries while avoiding data egress costs and improving query performance.

## December 07, 2023

Feature The following BigQuery ML data preprocessing features are now in
[preview](https://cloud.google.com/products/#product-launch-stages):

- The [`ML.TRANSFORM` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-transform), which you can use to preprocess feature data. This function processes input data by applying the data transformations captured in the [`TRANSFORM` clause](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create#transform) of an existing model.
- [Transform-only models](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-transform) which you can use to apply [preprocessing functions](https://docs.cloud.google.com/bigquery/docs/manual-preprocessing) to input data and return the preprocessed data. Transform-only models decouple data preprocessing from model training, making it easier for you to capture and reuse a set of data preprocessing rules.

## November 30, 2023

Feature The [slot estimator](https://docs.cloud.google.com/bigquery/docs/slot-estimator) now supports project level cost-optimal commitment and autoscale [recommendations](https://docs.cloud.google.com/bigquery/docs/slot-recommender) for on-demand workloads. This feature is now in [preview](https://cloud.google.com/products/#product-launch-stages).
Feature You can use [configuration YAML files to transform SQL code](https://docs.cloud.google.com/bigquery/docs/config-yaml-translation) when you translate SQL queries from your source database. Configuration YAML files can be used with the batch SQL translator, the interactive SQL translator, and the batch translation Python client. This feature is now [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).

## November 16, 2023

Feature The following BigQuery ML features for Vertex AI large language models (LLMs)
are now [generally available](https://cloud.google.com/products/#product-launch-stages) (GA):

- The SQL syntax for
  [remote models](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model)
  has been updated to provide access to all text generation and text
  embedding LLMs (for example, `text-bison-32k` and
  `textembedding-gecko-multilingual`) and also to provide support for different
  LLM
  [versions](https://docs.cloud.google.com/vertex-ai/docs/generative-ai/learn/model-versioning).

- [Region support](https://docs.cloud.google.com/bigquery/docs/locations#locations-for-remote-models)
  for `text-bison*` LLM models has been expanded to include the following
  locations in addition to `us` and `us-central1`:

  - `asia-northeast3`
  - `asia-southeast1`
  - `eu`
  - `europe-west1`
  - `europe-west2`
  - `europe-west3`
  - `europe-west4`
  - `europe-west9`
  - `us-west4`

## November 14, 2023

Feature You can now see query performance insights about [partition skew](https://docs.cloud.google.com/bigquery/docs/query-insights#partition_skew). This feature is in [preview](https://cloud.google.com/products/#product-launch-stages).

## November 13, 2023

Feature The following BigQuery ML point-in-time lookup functions are now
[generally available](https://cloud.google.com/products/#product-launch-stages) (GA). These functions let you specify a point-in-time cutoff when retrieving
features for training a model or running inference, in order to avoid
[data leakage](https://www.kaggle.com/code/dansbecker/data-leakage/notebook).

- Use the [`ML.FEATURES_AT_TIME` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-feature-time) to use the same point-in-time cutoff for all entities when retrieving features.
- Use the [`ML.ENTITY_FEATURES_AT_TIME` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-entity-feature-time) to retrieve features from multiple points in time for multiple entities.
Feature The following AI features in BigQuery are now in
[preview](https://cloud.google.com/products/#product-launch-stages):

- The ability to process documents from BigQuery
  [object tables](https://docs.cloud.google.com/bigquery/docs/object-tables)
  by doing the following:

  - Creating a [remote model](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model#remote_service_type) based on the [Document AI](https://docs.cloud.google.com/document-ai) API, including [specifying a document processor](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model#document_processor) to use.
  - Using the [`ML.PROCESS_DOCUMENT` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-process-document) with a Document AI-based remote model to process the documents.   

    Try this feature with the [Process documents with the `ML.PROCESS_DOCUMENT` function](https://docs.cloud.google.com/bigquery/docs/process-document) how-to.   

- The ability to transcribe audio files from BigQuery
  [object tables](https://docs.cloud.google.com/bigquery/docs/object-tables)
  by doing the following:

  - Creating a [remote model](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model#remote_service_type) based on the [Speech-to-Text](https://docs.cloud.google.com/speech-to-text) API, including[specifying a speech recognizer](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model#speech_recognizer) to use.
  - Using the [`ML.TRANSCRIBE` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-transcribe) with a Speech-to-Text-based remote model to transcribe the audio files.   

    Try this feature with the [Transcribe audio files with the `ML.TRANSCRIBE` function](https://docs.cloud.google.com/bigquery/docs/transcribe) how-to.

## November 07, 2023

Feature The batch SQL translator has added enhancements when viewing SQL translation reports. You can now see a [log summary of all issues during a translation job](https://docs.cloud.google.com/bigquery/docs/batch-sql-translator#console-output), as well as a [code tab](https://docs.cloud.google.com/bigquery/docs/batch-sql-translator#code-tab) that displays a side-by-side comparison of your input and output files from a translation. This feature is in [preview](https://cloud.google.com/products/#product-launch-stages).

## November 06, 2023

Feature The following BigQuery ML features for time series forecasting are now
[generally available](https://cloud.google.com/products/#product-launch-stages) (GA):

- Ensure forecasted values fall within specified limits. The
  [`FORECAST_LIMIT_LOWER_BOUND`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-time-series#forecast_limit_lower_bound)
  and [`FORECAST_LIMIT_UPPER_BOUND`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-time-series#forecast_limit_upper_bound)
  options of the
  [`CREATE MODEL`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-time-series)
  statement let you set the lower and upper bounds of the forecasted values
  returned by the model.

  Try this feature with the
  [Limit forecasted values for a time series model](https://docs.cloud.google.com/bigquery/docs/arima-time-series-forecasting-with-limits-tutorial) tutorial.
- Custom holiday modeling:

  - [`CREATE MODEL` syntax](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-time-series#as) lets you specify [custom holiday modeling](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-time-series#custom_holidays) for time series models.
  - The [`ML.HOLIDAY_INFO` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-holiday-info) returns the list of holidays being modeled by an [ARIMA_PLUS](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-time-series) or [ARIMA_PLUS_XREG](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-time-series) time series forecasting model.
  - The updated [`ML.EXPLAIN_FORECAST` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-explain-forecast) includes an explanation of the holiday effect for each holiday included in the model.

  Try this feature with the
  [Use custom holidays in a time-series forecasting model](https://docs.cloud.google.com/bigquery/docs/time-series-forecasting-holidays-tutorial)
  tutorial.
Feature The BigQuery Data Transfer Service can now [transfer campaign reporting and configuration data from Display \& Video 360](https://docs.cloud.google.com/bigquery/docs/display-video-transfer) into BigQuery. This feature is in [preview](https://cloud.google.com/products#product-launch-stages).

## November 02, 2023

Feature BigQuery now supports [text analysis configuration options](https://docs.cloud.google.com/bigquery/docs/text-analysis-search) for the following:

- [`CREATE SEARCH INDEX DDL`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_search_index_statement)
- Existing [`LOG_ANALYZER`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/text-analysis#log_analyzer) and new [`PATTERN_ANALYZER`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/text-analysis#pattern_analyzer) analyzers, which are used in various functions, including [`SEARCH`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/search_functions#search)
- New [`TEXT_ANALYZE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/text-analysis-functions#text_analyze) function

BigQuery now also provides support for the following advanced processing functions:

- [`ML.BAG_OF_WORDS`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-bag-of-words)
- [`ML.TF_IDF`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-tf-idf)
- [`BAG_OF_WORDS`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/text-analysis-functions#bag_of_words)
- [`TF_IDF`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/text-analysis-functions#tf_idf)
- [`COSINE_DISTANCE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#cosine_distance)
- [`EUCLIDEAN_DISTANCE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#euclidean_distance)
- [`EDIT_DISTANCE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#edit_distance)

These features are now in [preview](https://cloud.google.com/products/#product-launch-stages).

## November 01, 2023

Feature You can now use [cached results](https://docs.cloud.google.com/bigquery/docs/cached-results) from the same query issued by other users in the same project when you use Enterprise or Enterprise Plus edition. This feature is [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).
Feature The following `INFORMATION_SCHEMA` views that show metadata for table storage usage are now in
[preview](https://cloud.google.com/products/#product-launch-stages):

- Use the [`TABLE_STORAGE_USAGE_TIMELINE` view](https://docs.cloud.google.com/bigquery/docs/information-schema-table-storage-usage) to get total billable bytes per table per day at the project level.
- Use the [`TABLE_STORAGE_USAGE_TIMELINE_BY_ORGANIZATION` view](https://docs.cloud.google.com/bigquery/docs/information-schema-table-storage-usage-by-organization) to get total billable bytes per table per day at the organization level.

## October 31, 2023

Feature BigQuery support for [change data capture (CDC)](https://docs.cloud.google.com/bigquery/docs/change-data-capture) by processing and applying streamed changes in real-time to existing data using the BigQuery Storage Write API is now [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).
Feature You can now use data manipulation language (DML) to [modify rows that have been recently written](https://docs.cloud.google.com/bigquery/docs/write-api#use_data_manipulation_language_dml_with_recently_streamed_data) by the Storage Write API. This feature is in [preview](https://cloud.google.com/products/#product-launch-stages).

## October 30, 2023

Feature The BigQuery Data Transfer Service can now [transfer data from Azure Blob Storage](https://docs.cloud.google.com/bigquery/docs/blob-storage-transfer-intro) into BigQuery. This feature is now [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).
Feature The [administrative resource charts](https://docs.cloud.google.com/bigquery/docs/admin-resource-charts) now supports the following features in [preview](https://cloud.google.com/products/#product-launch-stages):

- View your [resource utilization chart at the project level](https://docs.cloud.google.com/bigquery/docs/admin-resource-charts#view_project_level_administrative_charts_data).
- [Filter your resource utilization data](https://docs.cloud.google.com/bigquery/docs/admin-resource-charts#view-admin-resource-charts) based on different billing models.

## October 23, 2023

Feature [Custom data masking](https://docs.cloud.google.com/bigquery/docs/user-defined-functions#custom-mask) now supports an expanded list of functions, including SHA hash functions with salt. This feature is in [preview](https://cloud.google.com/products#product-launch-stages).

## October 19, 2023

Feature [Stored procedures for Apache Spark](https://docs.cloud.google.com/bigquery/docs/spark-procedures) are now available without enrollment. This feature is in [preview](https://cloud.google.com/products/#product-launch-stages).

## October 18, 2023

Feature The [BigQuery migration assessment](https://docs.cloud.google.com/bigquery/docs/migration-assessment) is now available for Apache Hive in [preview](https://cloud.google.com/products/#product-launch-stages). You can use this feature to assess the complexity of migrating data from your Apache Hive data warehouse to BigQuery.

## October 16, 2023

Feature You can now use [DLP functions](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/dlp_functions) to support encryption and decryption between BigQuery and DLP, using AES-SIV. This feature is in [preview](https://cloud.google.com/products/#product-launch-stages).

## October 12, 2023

Feature The following geography functions are now [generally available](https://cloud.google.com/products/#product-launch-stages) (GA):

- [`ST_LINESUBSTRING`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_linesubstring): Gets a segment of a single linestring at a specific starting and ending fraction.
- [`ST_HAUSDORFFDISTANCE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_hausdorffdistance): Gets the discrete Hausdorff distance between two geometries.

## October 09, 2023

Feature Adding descriptions to the columns of a view is now [generally available](https://cloud.google.com/products/#product-launch-stages) (GA). Use the [`CREATE VIEW`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_view_statement) or [`ALTER COLUMN`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#alter_column_set_options_statement) DDL statements to add descriptions.
Feature Queries now support additional ways to work with grouping sets, which include:

- [`GROUP BY GROUPING SETS` clause](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#group_by_grouping_sets) (new): Produce aggregated data for one or more grouping sets.
- [`GROUP BY CUBE` clause](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#group_by_cube) (new): Produce aggregated data for all grouping set permutations.
- [`GROUP BY ROLLUP` clause](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#group_by_rollup) (update): You can now include groupable items sets in this clause.
- [`GROUPING` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#grouping) (new): Check if a groupable value in the `GROUP BY` clause is aggregated.

This feature is in [preview](https://cloud.google.com/products/#product-launch-stages).
Change BigQuery is now available in the [Dammam (me-central2)](https://docs.cloud.google.com/bigquery/docs/locations#regional-locations) region.
Change BigQuery ML is now available in the [Dammam (me-central2)](https://docs.cloud.google.com/bigquery-ml/docs/locations#regional-locations) region.
Change BigQuery Data Transfer Service is now available in the [Dammam (me-central2)](https://docs.cloud.google.com/bigquery-transfer/docs/locations#regional-locations) region.

## October 05, 2023

Feature The [BigQuery migration assessment](https://docs.cloud.google.com/bigquery/docs/migration-assessment) is now available for Snowflake in [preview](https://cloud.google.com/products/#product-launch-stages). You can use this feature to assess the complexity of migrating data from your Snowflake data warehouse to BigQuery.

## October 04, 2023

Feature You can now [copy tables across regions](https://docs.cloud.google.com/bigquery/docs/managing-tables#copy_tables_across_regions). This feature is now in [preview](https://cloud.google.com/products/#product-launch-stages).

## October 03, 2023

Feature The following Google Cloud Blockchain Analytics datasets are now available in [Preview](https://cloud.google.com/products/#product-launch-stages) and available through the [Public Datasets Program](https://docs.cloud.google.com/bigquery/public-data) and [Analytics Hub](https://docs.cloud.google.com/bigquery/docs/analytics-hub-view-subscribe-listings#view_listings):

- [Google Cloud's Tron Mainnet data](https://console.cloud.google.com/bigquery/analytics-hub/exchanges;cameo=analyticshub;pageName=listing-detail;pageResource=938420344946.us.preview_google_cloud_blockchain_analytics_189b1d89e86.public_preview_blockchain_analytics_tron_mainnet_18a66110122)

- [Google Cloud's Optimism Mainnet data](https://console.cloud.google.com/bigquery/analytics-hub/exchanges;cameo=analyticshub;pageName=listing-detail;pageResource=938420344946.us.preview_google_cloud_blockchain_analytics_189b1d89e86.public_preview_blockchain_analytics_optimism_mainnet_18a660ef3ca)

- [Google Cloud's Avalanche Contract Chain data](https://console.cloud.google.com/bigquery/analytics-hub/exchanges;cameo=analyticshub;pageName=listing-detail;pageResource=938420344946.us.preview_google_cloud_blockchain_analytics_189b1d89e86.public_preview_blockchain_analytics_avalanche_contract_chain_18a660b19c8)

- [Google Cloud's Fantom Opera data](https://console.cloud.google.com/bigquery/analytics-hub/exchanges;cameo=analyticshub;pageName=listing-detail;pageResource=938420344946.us.preview_google_cloud_blockchain_analytics_189b1d89e86.public_preview_blockchain_analytics_fantom_opera_us_189e08de2bb)

- [Google Cloud's Ethereum Mainnet data](https://console.cloud.google.com/bigquery/analytics-hub/exchanges;cameo=analyticshub;pageName=listing-detail;pageResource=938420344946.us.preview_google_cloud_blockchain_analytics_189b1d89e86.public_preview_blockchain_analytics_ethereum_mainnet_us_189c135b141)

- [Google Cloud's Arbitrum One Chain data](https://console.cloud.google.com/bigquery/analytics-hub/exchanges;cameo=analyticshub;pageName=listing-detail;pageResource=938420344946.us.preview_google_cloud_blockchain_analytics_189b1d89e86.public_preview_blockchain_analytics_arbitrum_one_chain_18add05adee)

- [Google Cloud's Cronos Mainnet Chain data](https://console.cloud.google.com/bigquery/analytics-hub/exchanges;cameo=analyticshub;pageName=listing-detail;pageResource=938420344946.us.preview_google_cloud_blockchain_analytics_189b1d89e86.public_preview_blockchain_analytics_cronos_mainnet_chain_18add08d212)

## October 02, 2023

Feature [BigQuery native integration in Looker Studio](https://docs.cloud.google.com/bigquery/docs/visualize-looker-studio#looker-studio-integration) enables monitoring features for Looker Studio queries, improves query performance, and supports many BigQuery features. This feature is in [preview](https://cloud.google.com/products/#product-launch-stages).

## September 29, 2023

Feature As a BigQuery administrator, to monitor your organization's slots utilization and BigQuery jobs' performance over time, use can now use [administrative query inspector](https://docs.cloud.google.com/bigquery/docs/admin-resource-charts#query-inspector-admin-jobs). This feature is now [generally available](https://cloud.google.com/products/#product-launch-stages).

## September 28, 2023

Feature The following BigQuery ML point-in-time lookup functions are now in
[preview](https://cloud.google.com/products/#product-launch-stages).
These functions let you specify a point-in-time cutoff when retrieving features
for training a model or running inference, in order to avoid
[data leakage](https://en.wikipedia.org/wiki/Leakage_(machine_learning)).

- Use the [`ML.FEATURES_AT_TIME` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-feature-time) to use the same point-in-time cutoff for all entities when retrieving features.
- Use the [`ML.ENTITY_FEATURES_AT_TIME` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-entity-feature-time) to retrieve features from multiple points in time for multiple entities.
Feature You can now use [IAM conditions](https://docs.cloud.google.com/bigquery/docs/conditions) to control access to BigQuery resources. This feature is in [preview](https://cloud.google.com/products/#product-launch-stages).

## September 25, 2023

Feature [Materialized views over BigLake metadata cache-enabled tables](https://docs.cloud.google.com/bigquery/docs/materialized-views-intro#biglake) can reference structured data stored in Cloud Storage. These materialized views function like materialized views over BigQuery-managed storage tables, including the benefits of automatic refresh and smart tuning. This feature is now [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).
Feature [Authorized stored procedures](https://docs.cloud.google.com/bigquery/docs/procedures#authorize_routines) are now [generally available](https://cloud.google.com/products/#product-launch-stages) (GA). This feature lets you share stored procedures with users or groups without giving them direct access to the underlying tables.
Deprecated Support for Google AdWords is now deprecated by the BigQuery Data Transfer Service. For information about transfers from Google Ads, see [Google Ads transfers](https://docs.cloud.google.com/bigquery/docs/google-ads-transfer).

## September 20, 2023

Change The maximum number of rows for results returned in [Connected Sheets](https://workspaceupdates.googleblog.com/2023/09/increased-row-limits-in-connected-sheets.html) has increased as follows:

- Pivot tables increased from 30,000 to 50,000 rows
- Data extracts increased from 25,000 to 50,000 rows

## September 18, 2023

Feature The BigQuery Data Transfer Service now supports [transfers from Search Ads 360](https://docs.cloud.google.com/bigquery/docs/search-ads-transfer) using the [new Search Ads 360 reporting API](https://developers.google.com/search-ads/reporting/api/reference/fields/v0/overview). This feature is in [preview](https://cloud.google.com/products/#product-launch-stages). Customers with existing Search Ads 360 transfers should [migrate their workflows](https://docs.cloud.google.com/bigquery/docs/search-ads-migration-guide) to be compatible with the new Search Ads 360. The BigQuery Data Transfer Service will stop its support for the old Search Ads 360 reporting API on May 31st, 2024.

## September 13, 2023

Feature You can now [create a federated dataset in BigQuery](https://docs.cloud.google.com/bigquery/docs/glue-federated-datasets) that federates to an existing database in AWS Glue. This feature is in [preview](https://cloud.google.com/products/#product-launch-stages).

## September 12, 2023

Feature The [array subscript operator](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/operators#array_subscript_operator) now returns a value in an array directly by index. Previously, only offset and ordinal were available. This feature is [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).
Feature The [struct subscript operator](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/operators#struct_subscript_operator) has been added. With this operator, you can access a `STRUCT` field by index, offset, or ordinal. This feature is [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).

## August 30, 2023

Feature You can now use [`EXPORT DATA`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/other-statements) statements to [directly export BigQuery data to Bigtable](https://docs.cloud.google.com/bigquery/docs/export-to-bigtable). This feature is in [preview](https://cloud.google.com/products/#product-launch-stages).

## August 29, 2023

Feature The following Generative AI features are now [generally available](https://cloud.google.com/products/#product-launch-stages) (GA) in BigQuery ML:

- Creating a [remote model](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model#remote_service_type) based on the [Vertex AI large language model (LLM) text-bison](https://docs.cloud.google.com/vertex-ai/docs/generative-ai/learn/models#foundation_models).
- Using the [`ML.GENERATE_TEXT` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-generate-text) with an LLM-based remote model to perform generative natural language tasks on text stored in BigQuery tables.

Try these features with the [Generate text by using a remote model and the ML.GENERATE_TEXT function](https://docs.cloud.google.com/bigquery/docs/generate-text-tutorial) tutorial.
Feature [BigQuery Studio](https://docs.cloud.google.com/bigquery/docs/query-overview#bigquery-studio) is now in
[preview](https://cloud.google.com/products/#product-launch-stages). BigQuery Studio offers features to make it easier for you to discover, explore, analyze, and run inference on data in BigQuery, including:

- Python notebooks, powered by [Colab Enterprise](https://docs.cloud.google.com/colab/docs/introduction). Notebooks provide one-click Python development runtimes, and built-in support for [BigQuery DataFrames](https://docs.cloud.google.com/python/docs/reference/bigframes/latest).
- Asset management and version history for notebooks and saved queries, powered by [Dataform](https://docs.cloud.google.com/dataform).
Feature [Data clean rooms](https://docs.cloud.google.com/bigquery/docs/data-clean-rooms) is now in
[preview](https://cloud.google.com/products/#product-launch-stages). Data clean rooms provide a secure environment in which multiple parties can share, join, and analyze their data assets without moving or revealing the underlying data. To learn more, see the following topics:

- [Use data clean rooms](https://docs.cloud.google.com/bigquery/docs/data-clean-rooms)
- [Aggregation threshold for queries and views](https://docs.cloud.google.com/bigquery/docs/privacy-policies)
- [Aggregation threshold clause](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#agg_threshold_clause)
Feature [Duet AI in BigQuery](https://docs.cloud.google.com/bigquery/docs/write-sql-duet-ai), an AI-powered collaborator in Google Cloud, can help you complete, generate, and explain SQL queries. This feature is in [preview](https://cloud.google.com/products/#product-launch-stages).
Feature [BigQuery DataFrames](https://docs.cloud.google.com/python/docs/reference/bigframes/latest) is now in [preview](https://cloud.google.com/products/#product-launch-stages). BigQuery DataFrames is a Python API that you can use to analyze data and perform machine learning tasks in BigQuery.
BigQuery DataFrames consists of the following parts:

- `bigframes.pandas` implements a DataFrame API (with partial Pandas compatibility) on top of BigQuery.
- `bigframes.ml` implements a Python API for BigQuery ML (with partial scikit-learn compatibility).

Get started with BigQuery DataFrames by using the [BigQuery DataFrames quickstart](https://docs.cloud.google.com/bigquery/docs/dataframes-quickstart).

## August 24, 2023

Feature The following BigQuery ML inference features are now
[generally available](https://cloud.google.com/products/#product-launch-stages) (GA):

- Importing [ONNX](https://docs.cloud.google.com/bigquery-ml/docs/reference/standard-sql/bigqueryml-syntax-create-onnx), [XGBoost](https://docs.cloud.google.com/bigquery-ml/docs/reference/standard-sql/bigqueryml-syntax-create-xgboost), and [TensorFlow Lite](https://docs.cloud.google.com/bigquery-ml/docs/reference/standard-sql/bigqueryml-syntax-create-tflite) models so that you can run them within the BigQuery ML inference engine.
- Hosting [models remotely on Vertex AI Prediction](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model#with_endpoint) so you can do inference with BigQuery ML, removing the need to build data pipelines manually.
- Using BigQuery ML functions to perform inference on Vertex AI pretrained models so that you can accomplish [natural language processing](https://docs.cloud.google.com/bigquery/docs/inference-overview#natural_language_processing), [translation](https://docs.cloud.google.com/bigquery/docs/inference-overview#machine_translation), and [computer vision](https://docs.cloud.google.com/bigquery/docs/inference-overview#computer_vision) tasks in BigQuery. These functions work with the Cloud Vision, Cloud Natural Language, and Cloud Translation APIs.
Feature The following text embedding features are now available in [preview](https://cloud.google.com/products/#product-launch-stages):

- Creating a BigQuery ML remote model that references the Vertex AI PaLM APIs for embeddings ([`textembedding-gecko`](https://docs.cloud.google.com/vertex-ai/docs/generative-ai/embeddings/get-text-embeddings)).
- Using the [`ML.GENERATE_TEXT_EMBEDDING` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-generate-text-embedding) with the remote embedding model to [embed text stored in BigQuery](https://docs.cloud.google.com/bigquery/docs/generate-text-embedding).
- Using the `ARRAY<NUMERIC> type` as an [input feature type](https://docs.cloud.google.com/bigquery/docs/input-feature-types) to other models.
- Generating text embeddings with the [NNLM, SWIVEL, and BERT TensorFlow models](https://docs.cloud.google.com/bigquery/docs/generate-embedding-with-tensorflow-models).

For more information, see the tutorial for performing [basic semantic search with text embeddings](https://docs.cloud.google.com/bigquery/docs/text-embedding-semantic-search).

## August 22, 2023

Feature BigQuery now allows you to [create your own masking routines](https://docs.cloud.google.com/bigquery/docs/user-defined-functions#custom-mask) for your data. You can use the `REGEX_REPLACE` scalar function to create custom masking rules to obfuscate your sensitive data. This feature is currently in [preview](https://cloud.google.com/products/#product-launch-stages).

## August 21, 2023

Feature You can now scan tables to [create data profiles](https://docs.cloud.google.com/bigquery/docs/data-profile-scan) and [monitor data quality](https://docs.cloud.google.com/bigquery/docs/data-quality-scan). These features help you better understand your data and ensure it is accurate and reliable. These features are [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).
Feature Analytics Hub now lets you [manage subscriptions](https://docs.cloud.google.com/bigquery/docs/analytics-hub-manage-subscriptions). The subscription resource stores relevant information about the subscriber and represents the connection between the shared resource and linked dataset. This feature is now in [preview](https://cloud.google.com/products/#product-launch-stages).

## August 17, 2023

Feature You can now replicate a dataset from the source region to one or more other regions with [cross-region dataset replication](https://docs.cloud.google.com/bigquery/docs/data-replication). This feature is now in [preview](https://cloud.google.com/products/#product-launch-stages).

## August 14, 2023

Change Starting September 15, 2023, prices will apply for network egress from a BigQuery Google Cloud region to another Google Cloud region on the same continent and between different continents. For more information, see [BigQuery Network Egress Traffic Pricing Charges Announcement](https://cloud.google.com/bigquery/pricing-announce).

## August 10, 2023

Change The [September 14, 2022 release notes](https://docs.cloud.google.com/bigquery/docs/release-notes#September_14_2022) announced that you could configure the connector to authenticate the connection using an external account with workload identity federation for [ODBC driver update release 2.5.0 1001](https://docs.cloud.google.com/bigquery/docs/reference/odbc-jdbc-drivers#odbc_release_2501001), but workload identity federation is not supported. Workforce identity federation is still supported.
Feature You can now use user-defined functions to [export BigQuery data as Protocol Buffer (Protobuf) columns](https://docs.cloud.google.com/bigquery/docs/protobuf-export). This feature is [generally-available](https://cloud.google.com/products/#product-launch-stages).
Feature You can now see query performance insights about [high cardinality joins](https://docs.cloud.google.com/bigquery/docs/query-insights#high_cardinality_join). This feature is [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).

## August 08, 2023

Feature The following features are now [generally available](https://cloud.google.com/products/#product-launch-stages) (GA) in queries and [materialized views](https://docs.cloud.google.com/bigquery/docs/materialized-views-create#aggregate_requirements):

- `HAVING MAX` and `HAVING MIN` clauses for the [`ANY_VALUE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#any_value) function.

- [`MAX_BY`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#max_by) function, which is a synonym for `ANY_VALUE(x HAVING MAX y)`.

- [`MIN_BY`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#min_by) function, which is a synonym for `ANY_VALUE(x HAVING MIN y)`.

## August 07, 2023

Feature The following JSON functions are now [generally available](https://cloud.google.com/products#product-launch-stages) (GA).

- [`JSON_ARRAY`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#json_array): Creates a JSON array.
- [`JSON_ARRAY_APPEND`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#json_array_append): Appends JSON data to the end of a JSON array.
- [`JSON_ARRAY_INSERT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#json_array_insert): Inserts JSON data into a JSON array.
- [`JSON_OBJECT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#json_object): Creates a JSON object.
- [`JSON_REMOVE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#json_remove): Produces JSON with the specified JSON data removed.
- [`JSON_SET`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#json_set): Inserts or replaces JSON data.
- [`JSON_STRIP_NULLS`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#json_strip_nulls): Removes JSON nulls.
- [`LAX_BOOL`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#lax_bool): Attempts to convert a JSON value to a SQL `BOOL` value.
- [`LAX_FLOAT64`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#lax_double): Attempts to convert a JSON value to a SQL `FLOAT64` value.
- [`LAX_INT64`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#lax_int64): Attempts to convert a JSON value to a SQL `INT64` value.
- [`LAX_STRING`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#lax_string): Attempts to convert a JSON value to a SQL `STRING` value.
Feature [Analytics Hub](https://docs.cloud.google.com/bigquery/docs/analytics-hub-introduction) now supports the use of routines in linked datasets. This feature is now in [preview](https://cloud.google.com/products/#product-launch-stages).
Feature The [quantitive `LIKE` operator](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/operators#like_operator_quantified) is now in
[preview](https://cloud.google.com/products/#product-launch-stages). With this operator, you can check a search value for matches against several patterns, using one of these conditions:

- `LIKE ANY`: Checks if at least one pattern matches.
- `LIKE SOME`: Synonym for `LIKE ANY`.
- `LIKE ALL`: Checks if every pattern matches.
Feature BigQuery now supports the ability to deny access to principals via [deny policies](https://docs.cloud.google.com/iam/docs/deny-access) for the following IAM permissions :

- **Managing reservations and capacity commitments:** `bigquery.googleapis.com/capacityCommitments.*, bigquery.googleapis.com/bireservations.*, bigquery.googleapis.com/reservationAssignments.*, bigquery.googleapis.com/reservations.*`
- **Resource Deletion:** `bigquery.googleapis.com/[datasets, tables, models, routines, jobs, connections].delete`
- **Dataset tag bindings:** `bigquery.googleapis.com/datasets.[createTagBinding, listTagBinding]`
- **Row Access Policies:** `bigquery.rowAccessPolicies.[create, delete, update, setIamPolicy]`

## August 04, 2023

Feature BigQuery now supports [using manifest files for external tables](https://docs.cloud.google.com/bigquery/docs/query-open-table-format-using-manifest-files). This feature is now [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).

## August 03, 2023

Feature With Analytics Hub, you can now [track usage metrics of your shared datasets](https://docs.cloud.google.com/bigquery/docs/analytics-hub-manage-listings#get-usage-metrics-shared-data). This feature is [generally available](https://cloud.google.com/products#product-launch-stages) (GA). The usage metrics include the following:

- Jobs that run against your shared dataset.
- The consumption details of your shared dataset by subscribers' projects and organizations.
- The number of rows and bytes processed by the job.
Feature You can now `https://docs.cloud.google.com/bigquery/docs/control-access-to-resources-iam#grant_access_to_a_table_or_view` or `https://docs.cloud.google.com/bigquery/docs/control-access-to-resources-iam#revoke_access_to_a_table_or_view` access to materialized views with a SQL statement. This feature is [generally available](https://cloud.google.com/products#product-launch-stages) (GA).
Feature **Cloud console updates** : The following features are now available in [preview](https://cloud.google.com/products/#product-launch-stages):

- On the **Welcome** page, in the **Recently accessed** section, you can view your 10 most [recently accessed resources](https://docs.cloud.google.com/bigquery/docs/bigquery-web-ui#view_recently_accessed_resources).
- After you run a query in the query editor, in the **Chart** tab, you can see the [visualization of your query results](https://docs.cloud.google.com/bigquery/docs/running-queries#queries).

## July 31, 2023

Change BigQuery Omni is now available in the [AWS - US West (Oregon) (aws-us-west-2)](https://docs.cloud.google.com/bigquery/docs/locations#omni-loc) and the [AWS - Europe (Ireland) (aws-eu-west-1)](https://docs.cloud.google.com/bigquery/docs/locations#omni-loc) regions.
Feature [BigQuery Storage Write API multiplexing](https://docs.cloud.google.com/bigquery/docs/write-api-best-practices#connection_pool_management) is now [generally available](https://cloud.google.com/products#product-launch-stages) (GA). You can use multiplexing in the default stream to write to multiple destination tables with shared connections.

## July 28, 2023

Feature [Query queues](https://docs.cloud.google.com/bigquery/docs/query-queues) are now [generally available](https://cloud.google.com/products#product-launch-stages) (GA). With query queues, BigQuery automatically determines your query concurrency based on available slots rather than a fixed limit. Once the maximum concurrency is reached, additional queries are queued until processing resources are available. Query queues are enabled by default and have been rolled out over the last several weeks; no user action is required and you shouldn't see any degradation in your query performance. You can optionally [set the maximum concurrency target](https://docs.cloud.google.com/bigquery/docs/query-queues#set_the_maximum_concurrency_target) for a reservation. You can also control the interactive and batch query queue timeout by using [default configurations](https://docs.cloud.google.com/bigquery/docs/default-configuration).

## July 20, 2023

Feature Multivariate time series forecasting with the
[`ARIMA_PLUS_XREG` model](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-multivariate-time-series) in BigQuery ML is now [generally available](https://cloud.google.com/products#product-launch-stages)
(GA). This feature lets you perform time series forecasting with extra feature columns. For more information, see the `ARIMA_PLUS_XREG` information in the [end-to-end user journey](https://docs.cloud.google.com/bigquery/docs/e2e-journey) topic, and try the
[multivariate time-series forecasting from Seattle air quality data tutorial](https://docs.cloud.google.com/bigquery/docs/arima-plus-xreg-single-time-series-forecasting-tutorial).
Feature BigQuery ML has introduced new [Explainable AI](https://docs.cloud.google.com/bigquery/docs/xai-overview)
capabilities for better model explainability:

- You can now use the [`ML.EXPLAIN_FORECAST` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-explain-forecast) with [`ARIMA_PLUS_XREG` models](https://docs.cloud.google.com/bigquery-ml/docs/reference/standard-sql/bigqueryml-syntax-create-multivariate-time-series).
- You can use the updated [`ML.EXPLAIN_FORECAST` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-explain-forecast) to get explanations of the holiday effect for holidays in time series forecasting models (both `ARIMA_PLUS` and `ARIMA_PLUS_XREG`).
- You can now use the [`ML.GLOBAL_EXPLAIN` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-global-explain) with [AutoML Tables models](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-automl) for global model explainability.
- For [Boosted Tree](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-boosted-tree) and [Random Forest](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-random-forest) models, you can now use the [`approx_global_feature_contrib`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-boosted-tree#approx_global_feature_contrib) training option to use fast approximation for global feature contribution computation in model training, and the [`approx_feature_contrib`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-explain-predict#approx_feature_contrib) option in the [`ML.EXPLAIN_PREDICT` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-explain-predict) to use the fast approximation for local feature contribution computation in model inference.

Now you can also use [Vertex Explainable AI](https://docs.cloud.google.com/vertex-ai/docs/explainable-ai/overview) on BigQuery ML models that you've registered to the Vertex AI Model Registry. To learn more, see [Explainable AI for BigQuery ML models](https://docs.cloud.google.com/bigquery/docs/vertex-xai).

## July 19, 2023

Feature BigQuery can now use search indexes to [optimize](https://docs.cloud.google.com/bigquery/docs/search#operator_and_function_optimization) some queries that contain the equal operator (`=`), `IN` operator, `LIKE` operator, or `STARTS_WITH` function to compare string literals with indexed data. This feature is in [preview](https://cloud.google.com/products/#product-launch-stages).

## July 17, 2023

Feature Primary and foreign key table constraints are now [generally available](https://cloud.google.com/products/#product-launch-stages). In addition to the [features available in preview](https://docs.cloud.google.com/bigquery/docs/release-notes#February_22_2023), you can now also [manage constraints through the BigQuery API](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables#tableconstraints) and [view constraints in the BigQuery console](https://cloud.google.com/blog/products/data-analytics/join-optimizations-with-bigquery-primary-and-foreign-keys/).
Deprecated The [google.cloud.bigquery.storage.v1beta2 API package](https://docs.cloud.google.com/bigquery/docs/reference/storage/rpc/google.cloud.bigquery.storage.v1beta2#bigquerywrite) for BigQueryWrite operations is deprecated and will be removed on July 17, 2024. After that date, requests to that package version for use with the BigQuery Storage Write API will fail. Data written to BigQuery using the [BigQuery Storage Write API](https://docs.cloud.google.com/bigquery/docs/write-api) is accessible by using the [google.cloud.bigquery.storage.v1 package](https://docs.cloud.google.com/bigquery/docs/reference/storage/rpc/google.cloud.bigquery.storage.v1).

**Next steps** : If you call the API directly, switch to [google.cloud.bigquery.storage.v1](https://docs.cloud.google.com/bigquery/docs/reference/storage/rpc/google.cloud.bigquery.storage.v1), the [generally available](https://cloud.google.com/products/#product-launch-stages) (GA) version of the API, to prevent any impact on your workflow.

## July 12, 2023

Feature Custom holiday modeling for time series forecasting is now in
[preview](https://cloud.google.com/products/#product-launch-stages).
This release offers the following features to improve the transparency,
flexibility, and explainability of time series forecasting in BigQuery ML:

- New [CREATE MODEL syntax](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-time-series#as) to specify [custom holiday modeling](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-time-series#custom_holidays) for time series models.
- The new [ML.HOLIDAY_INFO function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-holiday-info), which returns the list of holidays being modeled by an [ARIMA_PLUS](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-time-series) or [ARIMA_PLUS_XREG](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-multivariate-time-series) time series forecasting model.
- An updated [ML.EXPLAIN_FORECAST function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-explain-forecast), which includes an explanation of the holiday effect for each holiday included in the model.

A new public table, `bigquery-public-data.ml_datasets.holidays_and_events_for_forecasting`, has also been added to provide easy look-up of the built-in holidays used in time series forecasting models.

Try these features with the [Use custom holidays in a time-series forecasting model](https://docs.cloud.google.com/bigquery/docs/time-series-forecasting-holidays-tutorial) tutorial.
Feature The following BigQuery ML feature preprocessing functionality is now
[generally available](https://cloud.google.com/products/#product-launch-stages) (GA).

You can
[export](https://docs.cloud.google.com/bigquery/docs/exporting-models#export_model_trained_with_transform)
models that use the
[TRANSFORM](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create#transform)
clause for feature preprocessing
to[the TensorFlow SavedModel format](https://docs.cloud.google.com/bigquery/docs/exporting-models#export_model_trained_with_transform).
There are 13
[data types](https://docs.cloud.google.com/bigquery/docs/exporting-models#export-transform-types)
supported for TRANSFORM clause input, and 127
[SQL functions](https://docs.cloud.google.com/bigquery/docs/exporting-models#export-transform-functions)
supported for use within the TRANSFORM clause.

You can also now deploy a model trained with the TRANSFORM clause to Vertex AI
and locally.

Use the following functions to perform feature preprocessing:

- [ML.IMPUTER](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-imputer)
- [ML.LABEL_ENCODER](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-label-encoder)
- [ML.MAX_ABS_SCALER](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-max-abs-scaler)
- [ML.MULTI_HOT_ENCODER](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-multi-hot-encoder)
- [ML.NORMALIZER](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-normalizer)
- [ML.ONE_HOT_ENCODER](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-one-hot-encoder)
- [ML.ROBUST_SCALER](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-robust-scaler)

## July 06, 2023

Feature Spanner [Data Boost](https://docs.cloud.google.com/bigquery/docs/cloud-spanner-federated-queries#data_boost) lets you execute analytics queries and data exports with near-zero impact to existing workloads on your provisioned [Spanner](https://docs.cloud.google.com/spanner/docs) instance. This feature is now [generally available](https://cloud.google.com/products/#product-launch-stages) (GA) in the following regions:

- asia-northeast1 (Tokyo)
- us-central1 (Iowa)
- southamerica-east1 (São Paulo)
- europe-west1 (Belgium)
- europe-west2 (London)
- europe-west3 (Frankfurt)

## July 05, 2023

Feature You can use the [`LOAD DATA` SQL statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/other-statements#load_data_statement) to load data from Avro, CSV, newline delimited JSON, JSON, ORC, or Parquet files into a table. This feature is [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).
Change BigQuery is now available in the [Turin (europe-west12) and Doha (me-central1)](https://docs.cloud.google.com/bigquery/docs/locations#regions) regions.
Feature The ability to use physical bytes for storage billing is now [generally available](https://cloud.google.com/products/#product-launch-stages) (GA). When you set your dataset's storage billing model to use physical bytes, the total [active storage](https://cloud.google.com/bigquery/pricing#storage/bigquery/pricing#storage) costs you are billed for includes the bytes used for [time travel and fail-safe](https://docs.cloud.google.com/bigquery/docs/time-travel#time_travel) storage. For more information, see [Dataset storage billing models](https://docs.cloud.google.com/bigquery/docs/datasets-intro#dataset_storage_billing_models).
Announcement BigQuery capacity commitments have changed as follows:

- Annual commitments are now only available in Enterprise or Enterprise Plus edition. Flat-rate annual commitments are no longer available. For more information about pricing, see [Capacity compute (analysis) pricing](https://cloud.google.com/bigquery/pricing#capacity_compute_analysis_pricing).
- Monthly and flex commitments are no longer available. For more information about commitment options, see [Capacity commitment plans](https://docs.cloud.google.com/bigquery/docs/reservations-details).
Feature The ability to [configure the time travel window](https://docs.cloud.google.com/bigquery/docs/time-travel#configure_the_time_travel_window) is now [generally available](https://cloud.google.com/products/#product-launch-stages) (GA). You can specify the duration of the time travel window from a minimum of two days to a maximum of seven days.
Feature The [slot estimator](https://docs.cloud.google.com/bigquery/docs/slot-estimator) now provides cost-optimal commitment and autoscale [recommendations](https://docs.cloud.google.com/bigquery/docs/slot-recommender) based on editions pricing and historical performance metrics. This feature is in [preview](https://cloud.google.com/products/#product-launch-stages).
Feature The [fail-safe](https://docs.cloud.google.com/bigquery/docs/time-travel#fail-safe) period is now [generally available](https://cloud.google.com/products/#product-launch-stages) (GA). The fail-safe period offers an additional seven days of data storage after the time travel window, so that the data is available for emergency recovery. Billed costs won't include the bytes used for [fail-safe storage](https://docs.cloud.google.com/bigquery/docs/time-travel#fail-safe)
until July 17th, 2023.
Feature You can now restrict [data egress](https://docs.cloud.google.com/bigquery/docs/analytics-hub-introduction#data_egress) on Analytics Hub listings. This feature is now in [preview](https://cloud.google.com/products/#product-launch-stages).

## June 30, 2023

Feature [Metadata caching](https://docs.cloud.google.com/bigquery/docs/omni-introduction#metadata_caching_for_performance) is now available for BigLake tables that reference Amazon S3 data. This feature is in [preview](https://cloud.google.com/products/#product-launch-stages). Using cached metadata might improve query performance for BigLake tables.

## June 29, 2023

Feature Support for the following compliance programs is now [generally available](https://cloud.google.com/products/#product-launch-stages) (GA):

- [EU Regions and Support with Sovereignty Controls](https://docs.cloud.google.com/assured-workloads/docs/compliance-programs#eu-sovereignty-controls)
- [Sovereign Controls by Partners](https://docs.cloud.google.com/sovereign-controls-by-partners)
- [International Traffic in Arms Regulation (ITAR)](https://docs.cloud.google.com/assured-workloads/docs/compliance-programs#itar)

## June 26, 2023

Feature You can now create [stored procedures for Apache Spark](https://docs.cloud.google.com/bigquery/docs/spark-procedures) using Java or Scala.
You can also use the [Google Cloud console PySpark editor](https://docs.cloud.google.com/bigquery/docs/spark-procedures#use-python-pyspark-editor) to add options for stored Python procedures for Apache Spark. This feature is in [Preview](https://cloud.google.com/products/#product-launch-stages).

## June 21, 2023

Feature `TRUNCATE TABLE` is now supported for [multi-statement transactions](https://docs.cloud.google.com/bigquery/docs/transactions#statements_supported_in_transactions). This feature is now [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).

## June 20, 2023

Feature [Metadata caching](https://docs.cloud.google.com/bigquery/docs/metadata-caching) is now [generally available](https://cloud.google.com/products/#product-launch-stages) (GA). Using cached metadata might improve query performance for [BigLake tables](https://docs.cloud.google.com/bigquery/docs/biglake-intro) and [object tables](https://docs.cloud.google.com/bigquery/docs/object-table-introduction) that reference large numbers of objects, by allowing the query to avoid listing objects from Cloud Storage.

This release includes support for the following new features:

- [Protecting metadata cache data with customer-managed encryption keys](https://docs.cloud.google.com/bigquery/docs/metadata-caching#use_customer-managed_encryption_keys_with_cached_metadata).
- [Statistics on metadata cache usage](https://docs.cloud.google.com/bigquery/docs/metadata-caching#get_information_on_metadata_cache_usage_by_query_jobs).
- [Table statistics](https://docs.cloud.google.com/bigquery/docs/metadata-caching#table_statistics) for better query plan performance.

Metadata cache usage is billed going forward. For more information, see [Costs](https://docs.cloud.google.com/bigquery/docs/biglake-intro#costs).
Feature BigQuery now supports [querying Apache Iceberg tables](https://docs.cloud.google.com/bigquery/docs/iceberg-tables) that are created by open source engines. This feature is now [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).

## June 15, 2023

Feature The following Generative AI features are now in [preview](https://cloud.google.com/products/#product-launch-stages) with allowlist:

- Creating a [remote model](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model#remote_service_type) based on the [Vertex AI large language model (LLM) `text-bison`](https://docs.cloud.google.com/vertex-ai/docs/generative-ai/learn/models#foundation_models).
- Using the [`ML.GENERATE_TEXT` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-generate-text) with an LLM-based remote model to perform generative natural language tasks on text stored in BigQuery tables.

Try these features with the [Generate text by using a remote model and the ML.GENERATE_TEXT function](https://docs.cloud.google.com/bigquery/docs/generate-text-tutorial) tutorial.

## June 14, 2023

Feature BigQuery now provides information about the [fail-safe period](https://docs.cloud.google.com/bigquery/docs/time-travel#fail-safe). The fail-safe period offers an additional seven days of data storage after the time travel window, so that the data is available for emergency recovery. This feature is in [preview](https://cloud.google.com/products/#product-launch-stages).
Feature [BigLake Metastore](https://docs.cloud.google.com/bigquery/docs/manage-open-source-metadata) is now [generally available](https://cloud.google.com/products/#product-launch-stages) (GA). You can use BigLake Metastore to access and manage Iceberg table metadata from multiple sources.
Feature The `INFORMATION_SCHEMA` views that show table storage metadata are now [generally available](https://cloud.google.com/products/#product-launch-stages) (GA):

- Use the [`TABLE_STORAGE` view](https://docs.cloud.google.com/bigquery/docs/information-schema-table-storage) to get a snapshot of current storage usage for tables at the project level.
- Use the [`TABLE_STORAGE_BY_ORGANIZATION`](https://docs.cloud.google.com/bigquery/docs/information-schema-table-storage-by-organization) view to get a snapshot of current storage usage for tables at the organization level.

## June 12, 2023

Feature The [query execution graph](https://docs.cloud.google.com/bigquery/docs/query-insights) is now [generally available](https://cloud.google.com/products/#product-launch-stages) (GA). You can use the query execution graph to diagnose query performance issues, and to receive query performance insights.

## May 25, 2023

Feature The BigQuery partitioning and clustering recommender is now in [preview](https://cloud.google.com/products/#product-launch-stages). The recommender analyzes your BigQuery tables to identify partitioning or clustering opportunities for potential cost savings. You can [view partition or cluster recommendations](https://docs.cloud.google.com/bigquery/docs/view-partition-cluster-recommendations) through the BigQuery UI or recommender API. You can also [apply recommendations](https://docs.cloud.google.com/bigquery/docs/apply-partition-cluster-recommendations) directly to your BigQuery tables.

## May 23, 2023

Change [DML statements](https://docs.cloud.google.com/bigquery/quotas#data-manipulation-language-statements) no longer count toward the number of [table](https://docs.cloud.google.com/bigquery/quotas#standard_tables) or [partitioned tables](https://docs.cloud.google.com/bigquery/quotas#partitioned_tables) modifications per day. The limit of table and partitioned table modifications has not changed.

## May 19, 2023

Feature [`EXTERNAL_QUERY` SQL pushdown](https://docs.cloud.google.com/bigquery/docs/cloud-sql-federated-queries) optimizes data retrieval from external sources like Cloud SQL or Cloud Spanner databases. Transferring less data reduces execution time and cost. SQL pushdown encompasses both column pruning (`SELECT` clauses) and filter pushdowns (`WHERE` clauses). SQL pushdown applies to `SELECT * FROM T` queries, a significant percentage of all federated queries. Not all data types are supported for filter pushdowns. This feature is [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).

## May 18, 2023

Feature You can now sort your query results by using the sort menu next to a column name. This feature is in [preview](https://cloud.google.com/products/#product-launch-stages).

## May 16, 2023

Change The VPC Service Controls perimeter that protects the BigQuery API now also protects the BigQuery Reservation API. Customers who have already configured VPC Service Controls for the BigQuery API or the BigQuery Reservation API should update their configurations to reflect this change. For more information, see [BigQuery Reservation API](https://docs.cloud.google.com/vpc-service-controls/docs/supported-products#table_bigquery_reservation_api).

## May 15, 2023

Change BigQuery Omni is now available in the [AWS - Asia Pacific (Seoul) (aws-ap-northeast-2)](https://docs.cloud.google.com/bigquery/docs/locations#omni-loc) region.

## May 11, 2023

Feature [Object tables](https://docs.cloud.google.com/bigquery/docs/object-table-introduction) are now [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).

Object tables are read-only tables containing metadata for unstructured data stored in Cloud Storage. They enable you to [analyze](https://docs.cloud.google.com/bigquery/docs/object-table-remote-function) and [perform inference](https://docs.cloud.google.com/bigquery/docs/object-table-inference) on images, audio files, documents and other file types by using BigQuery ML and BigQuery remote functions. Object tables extend the data security and governance best practices currently applied to structured data to unstructured data as well.

The GA release includes the following new and updated functions:

- [`ML.DECODE_IMAGE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-decode-image): Decodes image data so that it can be interpreted by the [`ML.PREDICT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-predict) function.
- [`ML.CONVERT_COLOR_SPACE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-convert-color-space): Converts images with an RGB color space to a different color space.
- [`ML.CONVERT_IMAGE_TYPE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-convert-image-type): Converts the data type of the pixel values in an image.
- [`ML.RESIZE_IMAGE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-resize-image): Resizes images.
- [`ML.DISTANCE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-distance): Computes the distance between two vectors.
- [`ML.LP_NORM`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-lp-norm): Computes the Lᵖ norm for a vector, where ᵖ is the degree.

## May 10, 2023

Change BigQuery is now available in the [Dallas (us-south1)](https://docs.cloud.google.com/bigquery/docs/locations#regions) region.

## May 09, 2023

Feature [EXTERNAL_QUERY SQL pushdown](https://docs.cloud.google.com/bigquery/docs/cloud-sql-federated-queries) optimizes data retrieval from external sources like Cloud SQL or Cloud Spanner databases. Transferring less data reduces execution time and cost. SQL pushdown encompasses both column pruning (`SELECT` clauses) and filter pushdowns (`WHERE` clauses). SQL pushdown applies to `SELECT * FROM T` queries, a significant percentage of all federated queries. Pushdowns have limitations, for example not all data types are supported for filter pushdowns. This feature is [generally available (GA)](https://cloud.google.com/products/#product-launch-stages).
Feature You can now view [BI Engine Top Tables Cached Bytes](https://docs.cloud.google.com/bigquery/docs/bi-engine-monitor#metrics), [BI Engine Query Fallback Count](https://docs.cloud.google.com/bigquery/docs/monitoring-dashboard#metrics), and [Query Execution Count](https://docs.cloud.google.com/bigquery/docs/monitoring-dashboard#metrics) as dashboard metrics for BigQuery. This feature is now [generally available (GA)](https://cloud.google.com/products/#product-launch-stages).

## May 08, 2023

Feature [Differential privacy](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/differential-privacy) is now in [preview](https://cloud.google.com/products/#product-launch-stages) and includes four differential privacy aggregate functions that can be used to anonymize data: `AVG`, `COUNT`, `SUM`, and `PERCENTILE_CONT`. To learn more, see the following topics:

- [Use differential privacy](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/differential-privacy)
- [Differential privacy clause](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#dp_clause)
- [Differentially private aggregate functions](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate-dp-functions)
- [Extending differential privacy](https://docs.cloud.google.com/bigquery/docs/extend-differential-privacy)
Feature [INFORMATION_SCHEMA.MATERIALIZED_VIEW view](https://docs.cloud.google.com/bigquery/docs/information-schema-materialized-views) and enhanced job statistics now let you [monitor materialized view usage and refresh jobs](https://docs.cloud.google.com/bigquery/docs/materialized-views-monitor). This feature is in [preview](https://cloud.google.com/products/#product-launch-stages).

## May 05, 2023

Feature The [`INSERT INTO SELECT` statement](https://docs.cloud.google.com/bigquery/docs/load-data-using-cross-cloud-transfer) now lets you filter data from files in Amazon S3 and Azure Blob Storage and append it into BigQuery tables. This feature is in [preview](https://cloud.google.com/products/#product-launch-stages).

## May 04, 2023

Feature You can now [use configuration YAML files to transform SQL code](https://docs.cloud.google.com/bigquery/docs/config-yaml-translation) when you translate SQL queries from your source database. Configuration YAML files can be used with the batch SQL translator, the interactive SQL translator, and the batch translation Python client. This feature is now in [preview](https://cloud.google.com/products/#product-launch-stages).

## May 03, 2023

Feature The [table clones](https://docs.cloud.google.com/bigquery/docs/table-clones-intro) feature of BigQuery is now [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).

## May 01, 2023

Feature If you use query queues, then you can set the interactive and batch queue timeouts in your [default configuration](https://docs.cloud.google.com/bigquery/docs/default-configuration). This feature is in [preview](https://cloud.google.com/products#product-launch-stages).
Feature You can now add descriptions to the columns of a view. To do this, use the [`CREATE VIEW`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_view_statement) or [`ALTER COLUMN`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#alter_column_set_options_statement) DDL statements. This feature is in [preview](https://cloud.google.com/products/#product-launch-stages).

## April 27, 2023

Feature [`JSON` data type mapping](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/federated_query_functions#spanner-mapping) is now available for Cloud Spanner federated queries. This feature is [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).

## April 26, 2023

Feature [BigLake and non-BigLake external tables](https://docs.cloud.google.com/bigquery/docs/locations#query-storage-data-location) now support [Cloud Storage custom dual-regions](https://docs.cloud.google.com/storage/docs/use-dual-regions#create-dr-bucket). This feature is [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).

## April 25, 2023

Feature [Dynamic data masking](https://docs.cloud.google.com/bigquery/docs/column-data-masking-intro) has been updated to allow masking on `RECORD` columns that have been set to `REPEATED` mode. Previously, querying such columns when data masking had been applied would return internal errors. This feature is [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).

## April 24, 2023

Feature The [BigQuery Data Transfer Service for Google Ads](https://docs.cloud.google.com/bigquery/docs/google-ads-transfer) supports the new [Google Ads API](https://developers.google.com/google-ads/api/docs/start). The Google Ads connector supports [PMax](https://support.google.com/google-ads/answer/10724817) and Discovery campaigns, a limit of 8000 leaf accounts per transfer, the [`--table_filter`](https://docs.cloud.google.com/bigquery/docs/google-ads-transfer#setup-data-transfer) flag, and [backwards compatibility](https://docs.cloud.google.com/bigquery/docs/google-ads-transfer#backwards_compatibility). This feature is now [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).

## April 19, 2023

Feature [Updates to preferred tables for existing BI engine reservations](https://docs.cloud.google.com/bigquery/docs/bi-engine-preferred-tables#specify_preferred_tables_for_existing_reservations) now take up to ten seconds to propagate, down from five minutes. This feature is [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).

## April 13, 2023

Feature BigQuery supports setting the [rounding mode](https://docs.cloud.google.com/bigquery/docs/schemas#rounding_mode) to `ROUND_HALF_EVEN` or `ROUND_HALF_AWAY_FROM_ZERO` for parameterized `NUMERIC` or `BIGNUMERIC` columns at the column level. You can specify a default rounding mode at the table or dataset level that is automatically attached to any columns added within those entities. The [ROUND() function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#round) also accepts the rounding mode as an optional argument. This feature is [generally available](https://cloud.google.com/products/#product-launch-stages) GA.

## April 10, 2023

Change The results for queries against [table snapshots](https://docs.cloud.google.com/bigquery/docs/table-snapshots-intro) can now be [returned from cache](https://docs.cloud.google.com/bigquery/docs/cached-results).
Feature The limit for maximum result size (20 GiB logical bytes) when querying [Azure](https://docs.cloud.google.com/bigquery/docs/query-azure-data) or [Amazon Simple Storage service (S3)](https://docs.cloud.google.com/bigquery/docs/query-aws-data) data is now [generally available](https://cloud.google.com/products/#product-launch-stages) (GA). Querying Azure and Amazon S3 data are now subject to the following quotas and limitations:

- The maximum row size is 10 MiB. For more information, see [Quotas for query jobs](https://docs.cloud.google.com/bigquery/quotas#query_jobs).

- If your query uses the `ORDER BY` clause and has a result size larger than 256 MB, then your query fails. Previously, this limit was 2 MB. For more information, see [Limitations](https://docs.cloud.google.com/bigquery/docs/omni-introduction#limitations).

## April 06, 2023

Feature The [add data demo guide](https://docs.cloud.google.com/bigquery/docs/bigquery-web-ui#run_add_data_demo_guide) walks you through the process of adding data to BigQuery through popular sources and is now in [preview](https://cloud.google.com/products/#product-launch-stages).

## April 05, 2023

Feature [Non-incremental materialized views](https://docs.cloud.google.com/bigquery/docs/materialized-views-create#non-incremental) support most SQL queries, including `OUTER
JOIN`, `UNION`, and `HAVING` clauses, as well as analytic functions. This feature is in [preview](https://cloud.google.com/products/#product-launch-stages).

## April 04, 2023

Change BigQuery is now available in the [Israel (me-west1)](https://docs.cloud.google.com/bigquery/docs/locations) region.

## March 30, 2023

Feature [BigQuery Partner Center](https://docs.cloud.google.com/bigquery/docs/bigquery-ready-overview#partner_center), which can be used to discover and try validated partner applications, is now [generally available](https://cloud.google.com/products/#product-launch-stages) (GA). In addition, the [Google Cloud Ready - BigQuery](https://docs.cloud.google.com/bigquery/docs/bigquery-ready-partners) initiative has added 14 new partners.
Announcement [BigQuery ML documentation](https://docs.cloud.google.com/bigquery/docs/bqml-introduction) is now integrated with BigQuery documentation to unify resources for data analysis and machine learning tasks such as inference. BigQuery ML documentation resources include:

- [Get started with BigQuery ML](https://docs.cloud.google.com/bigquery/docs/create-machine-learning-model)
- [End-to-end user journey for each model](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-e2e-journey)
- [BigQuery ML SQL reference](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-glm)
- [BigQuery ML pricing](https://cloud.google.com/bigquery/pricing#bqml)

## March 29, 2023

Feature Compute (analysis) is now [generally available](https://cloud.google.com/products/#product-launch-stages) (GA) in three new [BigQuery editions](http://cloud.google.com/bigquery/docs/editions-intro): Standard, Enterprise, and Enterprise Plus. These editions support the slots autoscaling model to meet your organizations' needs and budgets.
Feature [Autoscaling slots](http://cloud.google.com/bigquery/docs/slots-autoscaling-intro) are now [generally available](https://cloud.google.com/products/#product-launch-stages) (GA). Autoscaling slot reservations and commitments created during the feature's preview have been set to [BigQuery Enterprise edition](http://cloud.google.com/bigquery/docs/editions-intro).

## March 28, 2023

Feature You can now import model artifacts saved in [ONNX](https://docs.cloud.google.com/bigquery-ml/docs/reference/standard-sql/bigqueryml-syntax-create-onnx), [XGBoost](https://docs.cloud.google.com/bigquery-ml/docs/reference/standard-sql/bigqueryml-syntax-create-xgboost), and [TensorFlow Lite](https://docs.cloud.google.com/bigquery-ml/docs/reference/standard-sql/bigqueryml-syntax-create-tflite) formats into BigQuery for inference, allowing you to leverage models built in popular frameworks directly within the BigQuery ML inference engine.

You can also host [models remotely on Vertex AI Prediction](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model#with_endpoint) and do inference with BigQuery ML, removing the need to build data pipelines manually.

You can do inference with Google Cloud's state of the art pretrained models using [Cloud AI service table-valued functions (TVFs)](https://docs.cloud.google.com/bigquery-ml/docs/reference/standard-sql/bigqueryml-syntax-cloud-ai-service-tvfs-overview) to get insights from your data. The TVFs work with Cloud Vision API, Cloud Natural Language API and Cloud Translation API.

These features are [in preview](https://cloud.google.com/products/#product-launch-stages). To enroll to use this feature, complete the [enrollment form](https://forms.gle/q97oMuz8Muigp3cT7).
Feature You can now use the [`tf_version`](https://docs.cloud.google.com/bigquery-ml/docs/reference/standard-sql/bigqueryml-syntax-create#tf_version) training option to specify the Tensorflow (TF) version during model training. By default, `tf_version` is set as '1.15'. If you want to use TF2 with Keras API, you can add `tf_version` = '2.8.0' when creating the model.

You can now use the [`xgboost_version`](https://docs.cloud.google.com/bigquery-ml/docs/reference/standard-sql/bigqueryml-syntax-create#xgboost_version) training option to specify the XGBoost version during model training. By default, `xgboost_version` is set as '0.9'. You can choose XGBoost version 1.1 by specifying `xgboost_version` = '1.1'.

You can now use the [`instance_weight_col`](https://docs.cloud.google.com/bigquery-ml/docs/reference/standard-sql/bigqueryml-syntax-create#instance_weight_col) training option to identify the column containing weights for each data point in the training dataset. Currently the `instance_weight_col` option is only available for boosted tree and random forest models with non-array feature types.

## March 27, 2023

Feature BigQuery now supports [change data capture (CDC)](https://docs.cloud.google.com/bigquery/docs/change-data-capture) by processing and applying streamed changes in real-time to existing data using the BigQuery Storage Write API. This feature is in [preview](https://cloud.google.com/products/#product-launch-stages).

## March 22, 2023

Feature BigQuery now supports [Unicode column naming](http://cloud.google.com/bigquery/docs/schemas#flexible-column-names) using international character sets, alphanumeric and special characters. Existing columns can use these new capabilities using the `RENAME` command. This feature is now in [preview](https://cloud.google.com/products/#product-launch-stages).

## March 20, 2023

Feature The following [AutoML Tables model](https://docs.cloud.google.com/bigquery-ml/docs/reference/standard-sql/bigqueryml-syntax-create-automl) features are now [generally available](https://cloud.google.com/products/#product-launch-stages):

- Availability in [additional regions](https://docs.cloud.google.com/bigquery-ml/docs/locations#regional-locations).
- [CMEK](https://docs.cloud.google.com/bigquery-ml/docs/customer-managed-encryption-key) support in available regions except multi-regions US and EU.
- [OPTIMIZATION_OBJECTIVE](https://docs.cloud.google.com/bigquery-ml/docs/reference/standard-sql/bigqueryml-syntax-create-automl#optimization_objective) now accepts two additional options:
  - MAXIMIZE_PRECISION_AT_RECALL
  - MAXIMIZE_RECALL_AT_PRECISION

## March 14, 2023

Feature The [Lineage tab](https://docs.cloud.google.com/bigquery/docs/data-catalog#data_lineage) in the table properties page lets you track how your data moves and transforms through BigQuery. This feature is now [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).

## March 13, 2023

Feature You can now specify translation configurations in the [BigQuery Interactive SQL Translator](https://docs.cloud.google.com/bigquery/docs/interactive-sql-translator) and use it to [debug Batch SQL translator jobs](https://docs.cloud.google.com/bigquery/docs/batch-sql-translator). This feature is now in [preview](https://cloud.google.com/products/#product-launch-stages).

## March 10, 2023

Feature The [`CREATE TABLE AS SELECT` statement](https://docs.cloud.google.com/bigquery/docs/load-data-using-cross-cloud-transfer#filter-data) now lets you filter data from files in Amazon S3 and Azure Blob Storage before transferring results into BigQuery tables This feature is in [preview](https://cloud.google.com/products/#product-launch-stages).

## March 07, 2023

Feature [Case-insensitive collation](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/collation-concepts) support is now [generally available](https://cloud.google.com/products/#product-launch-stages) (GA). In addition to features available in the preview, the GA release includes:

- [MIN](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#min), [MAX](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#max), [COUNT with DISTINCT](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#count), and [PERCENTILE_DISC](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/navigation_functions#percentile_disc) windows functions
- [ORDER BY and PARTITION BY in the WINDOWS clause](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/window-function-calls)
- [LIKE operator](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/operators#like_operator) with [limitations](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/operators#like_operator)
- [Views](https://docs.cloud.google.com/bigquery/docs/views-intro)
- [Materialized views](https://docs.cloud.google.com/bigquery/docs/materialized-views-intro) with [limitations](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/collation-concepts#limitations)
- [Table functions](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/table-functions) with [limitations](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/collation-concepts#limitations)
- [BigQuery BI engine](https://docs.cloud.google.com/bigquery/docs/bi-engine-intro)

## March 02, 2023

Feature The [`WITH RECURSIVE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#with_clause) clause is now [generally available](https://cloud.google.com/products/#product-launch-stages) (GA). This clause lets you include one or more [recursive common table expressions (CTEs)](https://docs.cloud.google.com/bigquery/docs/recursive-ctes) in a query.

## February 27, 2023

Change The multivariate time-series forecasting model [`ARIMA_PLUS_XREG`](https://docs.cloud.google.com/bigquery-ml/docs/reference/standard-sql/bigqueryml-syntax-create-multivariate-time-series) is now available to on-demand users.
Feature You can set [default values](https://docs.cloud.google.com/bigquery/docs/default-values) on columns in your BigQuery tables. This feature is now [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).

## February 23, 2023

Feature [Authorized stored procedures](https://docs.cloud.google.com/bigquery/docs/procedures#authorize_routines) are now in [preview](https://cloud.google.com/products/#product-launch-stages). This feature lets you share stored procedures with users or groups without giving them direct access to the underlying tables.

## February 22, 2023

Feature [Primary and foreign key table constraints](https://docs.cloud.google.com/bigquery/docs/information-schema-table-constraints) are now available in [preview](https://cloud.google.com/products/#product-launch-stages). You can define table constraints using the [`CREATE TABLE` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_table_statement), the [`ALTER TABLE ADD PRIMARY KEY` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#alter_table_add_primary_key_statement), or the [`ALTER TABLE ADD CONSTRAINT` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#alter_table_add_constraint_statement).
Fixed Fixed linked datasets querying shared dataset that has data ingested through streaming inserts or the BigQuery Storage Write API.

## February 21, 2023

Feature The [ALTER TABLE RENAME COLUMN statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#alter_table_rename_column_statement) and the [ALTER TABLE DROP COLUMN statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#alter_table_drop_column_statement) are now [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).

## February 15, 2023

Feature You can now [run `bq` commands using service account impersonation](https://docs.cloud.google.com/bigquery/docs/bq-command-line-tool#bq_service_account). This feature is [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).
Feature In the **Explorer** pane, you can now see all the [resources in the searched resource's level](https://docs.cloud.google.com/bigquery/docs/bigquery-web-ui#explorer_panel) by clicking **Show more** . This feature is [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).
Feature You can now make a dataset and the tables in that dataset [case-insensitive](https://docs.cloud.google.com/bigquery/docs/datasets#dataset-naming) when you [create a dataset](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_schema_statement) or [alter a dataset](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#alter_schema_set_options_statement). This feature is [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).
Feature In the **Explorer** pane, the [resource corresponding to the focused tab](https://docs.cloud.google.com/bigquery/docs/bigquery-web-ui#details_panel) is now selected. This feature is [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).

## February 13, 2023

Feature You can now create [materialized views over BigLake metadata cache-enabled tables](https://docs.cloud.google.com/bigquery/docs/materialized-views-intro#biglake) to reference structured data stored in Cloud Storage. This feature is in [preview](https://cloud.google.com/products/#product-launch-stages).

## February 09, 2023

Feature You can now apply four new types of [dynamic data masking](https://docs.cloud.google.com/bigquery/docs/column-data-masking-intro#masking_options) to table columns in BigQuery. These new data masking types include **date year** , **email** , **first four characters** , and **last four characters** masks. This feature is [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).
Change Autoscaling slot reservations are now available in [preview](https://cloud.google.com/products/#product-launch-stages). You can create autoscaling reservations and associated commitments using [slots autoscaling](http://cloud.google.com/bigquery/docs/slots-autoscaling-intro).
Feature **Cloud console updates** : In the **Explorer** pane, you can now refresh the contents of a resource (project or dataset). To refresh the contents of a resource, click **View actions** , and then click **Refresh contents**.

## February 06, 2023

Feature You can now view information related to query processing to monitor and optimize queries with the `query_info` column in `https://cloud.google.com/bigquery/docs/information-schema-jobs`, `https://cloud.google.com/bigquery/docs/information-schema-jobs-by-folder` and `https://cloud.google.com/bigquery/docs/information-schema-jobs-by-organization` views. This feature is [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).
Feature The [`HAVING MAX` and `HAVING MIN` clause](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate-function-calls) for the [`ANY_VALUE` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#any_value) is now in [preview](https://cloud.google.com/products#product-launch-stages).

## February 01, 2023

Feature The BigQuery Data Transfer Service can now [transfer data from Azure Blob Storage](https://docs.cloud.google.com/bigquery/docs/blob-storage-transfer-intro) into BigQuery. This feature is now in [preview](https://cloud.google.com/products#product-launch-stages).

## January 31, 2023

Change **Cloud console updates** : When you create datasets, select locations to run specific queries, or create exchanges in [Analytics Hub](https://cloud.google.com/bigquery/docs/analytics-hub-introduction), you now see separate options for multi-region and specific regions. Based on your selection, you see a list with more options.
Feature [Azure workload identity federation](https://docs.cloud.google.com/bigquery/docs/omni-azure-create-connection) is now [generally available](https://cloud.google.com/products/#product-launch-stages) (GA) for BigQuery Omni connections. You can now [create a connection for federated identity](https://docs.cloud.google.com/bigquery/docs/omni-azure-create-connection#creating-azure-connection) using Google Cloud console.

## January 30, 2023

Feature You can search for BigQuery partners in the [BigQuery Partner Center](https://console.cloud.google.com/bigquery/partner-center). This feature is in [Preview](https://cloud.google.com/products#product-launch-stages).

## January 19, 2023

Feature The following functions have been added for BigQuery ML:

- [ML.ROBUST_SCALER](https://docs.cloud.google.com/bigquery/docs/manual-preprocessing#mlrobust_scaler)
- [ML.NORMALIZER](https://docs.cloud.google.com/bigquery-ml/docs/reference/standard-sql/bigqueryml-preprocessing-functions#mlnormalizer)
- [ML.ONE_HOT_ENCODER](https://docs.cloud.google.com/bigquery-ml/docs/reference/standard-sql/bigqueryml-preprocessing-functions#mlone_hot_encoder)
- [ML.IMPUTER](https://docs.cloud.google.com/bigquery-ml/docs/reference/standard-sql/bigqueryml-preprocessing-functions#mlimputer)
- [ML.MAX_ABS_SCALER](https://docs.cloud.google.com/bigquery-ml/docs/reference/standard-sql/bigqueryml-preprocessing-functions#mlmax_abs_scaler)
- [ML.LABEL_ENCODER](https://docs.cloud.google.com/bigquery-ml/docs/reference/standard-sql/bigqueryml-preprocessing-functions#mllabel_encoder)

These features are now available in [preview](https://cloud.google.com/products#product-launch-stages).
Feature You can now attach [Resource Manager tags](https://docs.cloud.google.com/bigquery/docs/tags) to datasets, which let you conditionally apply Identity and Access Management (IAM) policies to your resources. This feature is [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).
Feature You can now use the [`TRANSFORM` clause](https://docs.cloud.google.com/bigquery-ml/docs/exporting-models#export_model_trained_with_transform) to train models which you can then export in the Tensorflow SavedModel format. This feature is now available in [preview](https://cloud.google.com/products#product-launch-stages).
Feature [More than 20 BigQuery ML components](https://google-cloud-pipeline-components.readthedocs.io/en/google-cloud-pipeline-components-1.0.14/google_cloud_pipeline_components.v1.bigquery.html#module-google_cloud_pipeline_components.v1.bigquery) for [Vertex AI Managed Pipelines](https://docs.cloud.google.com/vertex-ai/docs/pipelines) are now [generally available](https://cloud.google.com/products#product-launch-stages). These components benefit AI/ML users for the following:

- [Building pipelines](https://docs.cloud.google.com/vertex-ai/docs/pipelines/build-pipeline) using the KFP SDK and TFX SDK
- [Linking and tracking metadata automatically](https://docs.cloud.google.com/vertex-ai/docs/pipelines/lineage)
- Seamless integration with [Vertex AI](https://docs.cloud.google.com/vertex-ai) for [online prediction](https://docs.cloud.google.com/vertex-ai/docs/predictions/get-predictions)

Major Google Cloud pipeline components available in Vertex AI are.

- [BigqueryQueryJobOp](https://google-cloud-pipeline-components.readthedocs.io/en/google-cloud-pipeline-components-1.0.14/google_cloud_pipeline_components.v1.bigquery.html#google_cloud_pipeline_components.v1.bigquery.BigqueryQueryJobOp)
- [BigqueryCreateModelJobOp](https://google-cloud-pipeline-components.readthedocs.io/en/google-cloud-pipeline-components-1.0.14/google_cloud_pipeline_components.v1.bigquery.html#google_cloud_pipeline_components.v1.bigquery.BigqueryCreateModelJobOp)
- [BigqueryExportModelJobOp](https://google-cloud-pipeline-components.readthedocs.io/en/google-cloud-pipeline-components-1.0.14/google_cloud_pipeline_components.v1.bigquery.html#google_cloud_pipeline_components.v1.bigquery.BigqueryExportModelJobOp)
- [BigqueryPredictModelJobOp](https://google-cloud-pipeline-components.readthedocs.io/en/google-cloud-pipeline-components-1.0.14/google_cloud_pipeline_components.v1.bigquery.html#google_cloud_pipeline_components.v1.bigquery.BigqueryPredictModelJobOp)
- [BigqueryEvaluateModelJobOp](https://google-cloud-pipeline-components.readthedocs.io/en/google-cloud-pipeline-components-1.0.14/google_cloud_pipeline_components.v1.bigquery.html#google_cloud_pipeline_components.v1.bigquery.BigqueryPredictModelJobOp)
- [BigqueryDropModelJobOp](https://google-cloud-pipeline-components.readthedocs.io/en/google-cloud-pipeline-components-1.0.14/google_cloud_pipeline_components.v1.bigquery.html#google_cloud_pipeline_components.v1.bigquery.BigqueryPredictModelJobOp)
- [BigqueryEvaluateModelJobOp](https://google-cloud-pipeline-components.readthedocs.io/en/google-cloud-pipeline-components-1.0.14/google_cloud_pipeline_components.v1.bigquery.html#google_cloud_pipeline_components.v1.bigquery.BigqueryEvaluateModelJobOp)
- [BigqueryExplainForecastModelJobOp](https://google-cloud-pipeline-components.readthedocs.io/en/google-cloud-pipeline-components-1.0.14/google_cloud_pipeline_components.v1.bigquery.html#google_cloud_pipeline_components.v1.bigquery.BigqueryExplainForecastModelJobOp)
- [BigqueryExplainPredictModelJobOp](https://google-cloud-pipeline-components.readthedocs.io/en/google-cloud-pipeline-components-1.0.14/google_cloud_pipeline_components.v1.bigquery.html#google_cloud_pipeline_components.v1.bigquery.BigqueryExplainPredictModelJobOp)
- [BigqueryForecastModelJobOp](https://google-cloud-pipeline-components.readthedocs.io/en/google-cloud-pipeline-components-1.0.14/google_cloud_pipeline_components.v1.bigquery.html#google_cloud_pipeline_components.v1.bigquery.BigqueryPredictModelJobOp)

## January 17, 2023

Feature [Sparse input](https://docs.cloud.google.com/bigquery-ml/docs/reference/standard-sql/bigqueryml-input-feature-types#split-inputs) support in BigQuery ML model training is now [generally available](https://cloud.google.com/products#product-launch-stages) (GA). This feature improves model training for data whose values are mostly zero or empty. For additional examples, see the [sparse features support in BigQuery blog](https://cloud.google.com/blog/topics/developers-practitioners/sparse-features-support-in-bigquery).
Feature BigQuery ML support for multivariate time-series forecasting with the [`ARIMA_PLUS_XREG` model](https://docs.cloud.google.com/bigquery-ml/docs/reference/standard-sql/bigqueryml-syntax-create-multivariate-time-series) is now available in [preview](https://cloud.google.com/products/#product-launch-stages). This feature lets you perform time-series forecasting with extra feature columns. For more information, see the `ARIMA_PLUS_XREG` sections in the [end-to-end user journey](https://docs.cloud.google.com/bigquery-ml/docs/reference/standard-sql/bigqueryml-syntax-e2e-journey) and the [multivariate time-series forecasting from Seattle air quality data tutorial](https://docs.cloud.google.com/bigquery-ml/docs/arima-plus-xreg-single-time-series-forecasting-tutorial).

## January 10, 2023

Feature The [`ALTER CAPACITY SET OPTIONS` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#alter_capacity_set_options_statement) and [`ALTER RESERVATION SET OPTIONS` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#alter_reservation_set_options_statement) are now [generally available](https://cloud.google.com/products/#product-launch-stages) (GA). Additionally, the [`CREATE CAPACITY`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_capacity_statement), [`CREATE RESERVATION`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_reservation_statement), and [`CREATE ASSIGNMENT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_assignment_statement) statements now support the `OPTIONS` clause.

## January 09, 2023

Feature The following [generally available](https://cloud.google.com/products/#product-launch-stages) (GA) features have been added for [sessions](https://docs.cloud.google.com/bigquery/docs/sessions-write-queries):

- In a session, [temporary functions](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_function_statement) are now maintained until the session ends.

- In a session, statements that include the `TEMP` keyword can also include the `OR REPLACE` and `IF NOT EXISTS` keywords.

## January 04, 2023

Feature BigQuery ML support for image analytics with vision models is available in [preview](https://cloud.google.com/products/#product-launch-stages).
Customers can import vision models to perform inference modeling with images to detect objects, perform optical character recognition (OCR), and more. To request access to these features, complete the [BigQuery ML interest sign up form](https://bit.ly/bqml-interest-form).

This new capability uses BigQuery object tables to access image data stored in Cloud Storage and predict results from machine learning models. You can now generate insights from structured and unstructured data with the following steps:

1. Create an object table to access images stored in Cloud Storage.
2. Import vision models in TensorFlow vision models such as [ImageNet](https://tfhub.dev/google/imagenet/mobilenet_v3_small_075_224/feature_vector/5) or [ResNet 50](https://tfhub.dev/tensorflow/resnet_50/classification/1), or import your own models to detect objects from images, to annotate photos, and to perform OCR.
3. Unify image data with structured data such as user activities or sales orders to train machine learning models. You can then use prediction results to extract insights from your data.

## January 03, 2023

Feature [BigQuery ML integration with Vertex AI Model Registry](https://docs.cloud.google.com/bigquery-ml/docs/managing-models-vertex) is now [generally available](https://cloud.google.com/products#product-launch-stages). With this integration, you can now use the following capabilities:

- Register and monitor BigQuery ML models with Vertex AI Model Registry
- Deploy BigQuery ML models directly from Vertex AI Model Registry to Vertex Deployment endpoints
- Use Vertex AI to compare and track evaluation metrics.
- Explainable AI for BigQuery ML models, including built-in XAI, inside Vertex AI
- The seamless integration between BigQuery ML and Vertex AI lets you use Vertex AI for MLOps.

Key features include:

- Model versioning for models registered with Vertex AI Model Registry
- Revision alias for different model versions, and User specified model ID
- List the models by type (custom model, BigQuery ML, AutoML)
- BigQuery ML models can be registered with Vertex AI Model Registry to help you explore, manage, and govern your BigQuery ML models
- Ability to deploy BigQuery ML models to Vertex AI end points
- BigQuery ML models deployed on Vertex AI endpoints can use MLOps features such as model monitoring
Change Customers can use BigQuery ML to train and run models on BigLake in Cloud Storage.
See [Data Cloud Blog](https://cloud.google.com/blog/products/data-analytics/building-most-open-data-cloud-all-data-all-source-any-platform) and [End to end unstructured data use cases demo](https://www.youtube.com/watch?v=u7XS59COjDY).

## December 22, 2022

Change BigQuery now blocks [saving query results to Google Drive](https://docs.cloud.google.com/bigquery/docs/writing-results#saving-query-results-to-drive) from projects inside a [VPC Service Controls protected perimeter](https://docs.cloud.google.com/vpc-service-controls/docs/supported-products#table_bigquery).
Feature The [Lineage tab](https://docs.cloud.google.com/bigquery/docs/data-catalog#data_lineage) in the table properties page lets you track how your data moves and transforms through BigQuery. This feature is now in [preview](https://cloud.google.com/products#product-launch-stages).

## December 15, 2022

Feature You can now access and query Cloud SQL data over a [private connection](https://docs.cloud.google.com/bigquery/docs/connect-to-sql#before_you_begin). This feature is [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).

## December 08, 2022

Feature The [demo query guide](https://cloud.google.com/bigquery/docs/quickstarts/query-public-dataset-console#run_demo_query_guide) helps you query a public dataset from Google Trends and is now in [preview](https://cloud.google.com/products/#product-launch-stages).

## December 01, 2022

Feature BigQuery now supports [querying Apache Iceberg tables](https://docs.cloud.google.com/bigquery/docs/iceberg-tables) that are created by open source engines. This feature is in [preview](https://cloud.google.com/products/#product-launch-stages).

## November 28, 2022

Feature BigQuery now supports the following features when you load data:

- [ASCII control characters](https://docs.cloud.google.com/bigquery/docs/loading-data-cloud-storage-csv#csv-options) for CSV files.
- [Reference file with the expected table schema](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#reference-file-schema-uri-load) for creating external tables with Avro, ORC, and Parquet files.

These features are [generally available](https://cloud.google.com/products#product-launch-stages) (GA).

## November 17, 2022

Feature [Object tables](https://docs.cloud.google.com/bigquery/docs/object-table-introduction) are now in
[preview](https://cloud.google.com/products/#product-launch-stages). Object tables are read-only tables containing metadata for unstructured data
stored in Cloud Storage. These tables enable you to
[analyze](https://docs.cloud.google.com/bigquery/docs/object-table-remote-function)
and
[perform inference](https://docs.cloud.google.com/bigquery/docs/object-table-inference)
on images, audio files, documents, and other file types by using BigQuery ML and
BigQuery remote functions. Object tables extend structured data features such as data security and governance best practices to unstructured data.
Feature [Metadata caching](https://docs.cloud.google.com/bigquery/docs/biglake-intro#metadata_caching_for_performance) is now in
[preview](https://cloud.google.com/products/#product-launch-stages). Using cached metadata might improve query performance for
[BigLake tables](https://docs.cloud.google.com/bigquery/docs/biglake-intro)
and
[object tables](https://docs.cloud.google.com/bigquery/docs/object-table-introduction)
that reference large numbers of objects, by allowing the query to avoid listing
objects from Cloud Storage.

## November 14, 2022

Feature The [slot estimator](https://docs.cloud.google.com/bigquery/docs/slot-estimator) helps you manage slot capacity based on historical performance metrics. This feature is now [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).

## November 09, 2022

Feature You can now transfer data from [Amazon S3](https://docs.cloud.google.com/bigquery/docs/omni-aws-cross-cloud-transfer) and [Azure Blob Storage](https://docs.cloud.google.com/bigquery/docs/omni-azure-cross-cloud-transfer) to BigQuery using the `LOAD DATA` statement. This feature is [generally available (GA)](https://cloud.google.com/products/#product-launch-stages) and includes support for the following features:

- Transfer files that are hive partitioned.
- Load semi-structured JSON source data into BigQuery without providing a schema by using [JSON columns](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json-data) in the destination table.
- Encrypt destination tables using customer managed encryption keys.
- Transfer data to `US` multi-region and `US-EAST-4` regions.

## November 07, 2022

Change In the **Explorer** pane, you can now [star](https://docs.cloud.google.com/bigquery/docs/bigquery-web-ui#star_resources) your projects, datasets, and tables. This feature replaces the pin feature, which formerly allowed you to pin projects to the **Explorer** pane. This feature is now [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).
Feature In the Cloud console, the **Add data** feature lets you access popular ways to search for and ingest data sources that work with BigQuery. For an example, see [viewing listings in Analytics Hub](https://docs.cloud.google.com/bigquery/docs/analytics-hub-view-subscribe-listings#view_listings). This feature is now [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).

## November 03, 2022

Feature SQL functions for managing [wrapped keysets](https://docs.cloud.google.com/bigquery/docs/aead-encryption-concepts#wrapped_keysets) are [generally available (GA)](https://cloud.google.com/products/#product-launch-stages). You can now perform the following actions natively in BigQuery with fewer risks and steps:

- [Create a wrapped keyset](https://docs.cloud.google.com/bigquery/docs/column-key-encrypt#wrap-keyset)
- [Rotate a wrapped keyset](https://docs.cloud.google.com/bigquery/docs/column-key-encrypt#rotate-wrapped-keyset)
- [Rewrap a wrapped keyset](https://docs.cloud.google.com/bigquery/docs/column-key-encrypt#rewrap-keyset)
- [Encrypt and decrypt a column with a wrapped keyset](https://docs.cloud.google.com/bigquery/docs/column-key-encrypt#encryption_and_decryption)

Included with this release are the following new key management functions:

- [`KEYS.NEW_WRAPPED_KEYSET`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aead_encryption_functions#keysnew_wrapped_keyset)
- [`KEYS.ROTATE_WRAPPED_KEYSET`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aead_encryption_functions#keysrotate_wrapped_keyset)
- [`KEYS.REWRAP_KEYSET`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aead_encryption_functions#keysrewrap_keyset)

## November 02, 2022

Feature The [query execution graph](https://docs.cloud.google.com/bigquery/docs/query-insights) is now in [preview](https://cloud.google.com/products/#product-launch-stages). You can use the query execution graph to diagnose query performance issues, and to receive query performance insights.

## November 01, 2022

Feature The [BigQuery migration assessment](https://docs.cloud.google.com/bigquery/docs/migration-assessment) is now available for Amazon Redshift in [preview](https://cloud.google.com/products/#product-launch-stages). You can use this feature to assess the complexity of migrating from your Amazon Redshift data warehouse to BigQuery.

## October 31, 2022

Feature [Column-level data masking](https://docs.cloud.google.com/bigquery/docs/column-data-masking-intro) is now [generally available](https://cloud.google.com/products/#product-launch-stages) (GA). You can use data masking to selectively obscure column data for groups of users, while still allowing access to the column.
Feature The [`max_staleness` materialized view](https://docs.cloud.google.com/bigquery/docs/materialized-views-create#max_staleness) option helps you achieve consistently high performance with controlled costs when processing large, frequently changing datasets. This feature is now in [preview](https://cloud.google.com/products/#product-launch-stages).

## October 27, 2022

Feature [Search indexes](https://docs.cloud.google.com/bigquery/docs/search-intro) and the [SEARCH() function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/search_functions#search) are now [generally available](https://cloud.google.com/products/#product-launch-stages) (GA). These enable you to use Google Standard SQL to efficiently pinpoint specific data elements in unstructured text and semi-structured data.

## October 26, 2022

Feature The following geography functions are now [generally available](https://cloud.google.com/products/#product-launch-stages) (GA):

- [`ST_ISCLOSED`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_isclosed): Returns `TRUE` for a non-empty geography, where each element in the geography has an empty boundary.
- [`ST_ISRING`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_isring): Checks if a geography is a linestring and if the linestring is both closed and simple.

## October 24, 2022

Feature You can now view [BI Engine Top Tables Cached Bytes](https://docs.cloud.google.com/bigquery/docs/bi-engine-monitor#metrics), [BI Engine Query Fallback Count](https://docs.cloud.google.com/bigquery/docs/monitoring-dashboard#metrics), and [Query Execution Count](https://docs.cloud.google.com/bigquery/docs/monitoring-dashboard#metrics) as dashboard metrics for BigQuery. This feature is now in [preview](https://cloud.google.com/products/#product-launch-stages).

## October 18, 2022

Feature [Remote functions](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/remote-functions), which let you invoke functions from Cloud Functions or Cloud Run in your Google Standard SQL queries, are now [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).

## October 12, 2022

Change The reporting process for the [`tabledata.list bytes per minute` quota](https://docs.cloud.google.com/bigquery/quotas#api_request_quotas) has been updated to more accurately reflect the enforced limit. The limit has not changed.

## October 11, 2022

Feature [Analytics Hub](https://docs.cloud.google.com/bigquery/docs/analytics-hub-introduction) is now [generally available](https://cloud.google.com/products/#product-launch-stages). As an Analytics Hub publisher, you can now [view all subscriptions to your listing](https://docs.cloud.google.com/bigquery/docs/analytics-hub-manage-listings#view_all_subscriptions) and [remove a subscription from your listing](https://docs.cloud.google.com/bigquery/docs/analytics-hub-manage-listings#remove_a_subscription).
Feature You can now use [stored procedures for Apache Spark](https://docs.cloud.google.com/bigquery/docs/spark-procedures). This feature is in [preview](https://cloud.google.com/products/#product-launch-stages).

## October 10, 2022

Feature The ability to use physical bytes for storage billing is now in
[Preview](https://cloud.google.com/products/#product-launch-stages).
For more information, see
[Dataset storage billing models](https://docs.cloud.google.com/bigquery/docs/datasets-intro#dataset_storage_billing_models).
Feature [Multi-statement transactions](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/transactions) are now [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).

## October 05, 2022

Change [Concurrent connections quotas](https://docs.cloud.google.com/bigquery/quotas#write-api-limits) are now based on the project that initiates the Storage Write API request, not the project containing the BigQuery dataset resource.
Feature You can now [explore query results in Colab](https://docs.cloud.google.com/bigquery/docs/explore-data-colab) using Python libraries. This feature is now in [preview](https://cloud.google.com/products/#product-launch-stages).

## October 03, 2022

Feature BigQuery supports JSON as a [native column type](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#json_type). This feature is now [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).

## September 29, 2022

Feature In addition to standard rounding, BigQuery now supports the [rounding mode](https://docs.cloud.google.com/bigquery/docs/schemas#rounding_mode) `ROUND_HALF_EVEN` for parameterized `NUMERIC` or `BIGNUMERIC` columns. The [`ROUND()` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#round) also accepts the rounding mode as an optional argument. This feature is now in [preview](https://cloud.google.com/products/#product-launch-stages).

## September 28, 2022

Feature With [Datastream for BigQuery](https://docs.cloud.google.com/datastream-for-bigquery), you can now replicate data and schema updates from operational databases directly into BigQuery. This feature is now in [preview](https://cloud.google.com/products/#product-launch-stages).

## September 26, 2022

Change The `totalItems` field returned by the [`projects.list`](https://cloud.google.com/bigquery/docs/reference/rest/v2/projects/list) API method now returns the number of items per page, rather than an approximate total number of projects across all pages.
Feature In the **Explorer** pane, you can now [open tables in Connected Sheets](https://docs.cloud.google.com/bigquery/docs/connected-sheets#open-tables-sheets). This feature is now [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).

## September 21, 2022

Feature [BigQuery Omni](https://cloud.google.com/bigquery/docs/omni-introduction) has introduced support for on-demand pricing model [(GA)](https://cloud.google.com/products#product-launch-stages) for a limited duration. For more information, see [BigQuery Omni Pricing](https://cloud.google.com/bigquery/pricing#bqomni).
Feature You can now view shuffle usage ratios in the [admin resource charts](https://docs.cloud.google.com/bigquery/docs/admin-resource-charts#main_chart_view). This feature is now in [preview](https://cloud.google.com/products/#product-launch-stages).

## September 16, 2022

Feature [BigQuery Omni](https://docs.cloud.google.com/bigquery/docs/omni-introduction) now supports the following quota and limit:

- The quota for total query result sizes for a project is now 1 TB per day. For more information, see [Query jobs](https://docs.cloud.google.com/bigquery/quotas#query_jobs).
- The limit for maximum result size for a query has been increased from 2 MB to 10 GB ([preview](https://cloud.google.com/products/#product-launch-stages)).

For more information, see [Limitations](https://docs.cloud.google.com/bigquery/docs/omni-introduction#limitations).

## September 15, 2022

Change BigQuery is now available in the [Madrid (europe-southwest1)](https://docs.cloud.google.com/bigquery/docs/locations#regions), [Milan (europe-west8)](https://docs.cloud.google.com/bigquery/docs/locations#regions), and [Paris (europe-southwest1)](https://docs.cloud.google.com/bigquery/docs/locations#regions) regions. The Madrid and Paris regions have the [lowest carbon impact](https://docs.cloud.google.com/sustainability/region-carbon).
Feature The [BigQuery Data Transfer Service for Google Ads](https://docs.cloud.google.com/bigquery/docs/google-ads-transfer) now supports the new [Google Ads API](https://developers.google.com/google-ads/api/docs/start). This feature is now in [preview](https://cloud.google.com/products/#product-launch-stages).
Change BigQuery ML is now available in the [Madrid (europe-southwest1)](https://docs.cloud.google.com/bigquery/docs/locations#regional-locations), [Milan (europe-west8)](https://docs.cloud.google.com/bigquery/docs/locations#regional-locations), and \[Paris (europe-southwest1)\]https://cloud.google.com/bigquery/docs/locations#regional-locations) regions. The Madrid and Paris regions have the [lowest carbon impact](https://docs.cloud.google.com/sustainability/region-carbon).

## September 14, 2022

Change [The Merge](https://ethereum.org/en/upgrades/merge/) is coming! You may experience disruptions in the Ethereum public datasets in BigQuery.
Feature [ODBC driver update, release 2.5.0 1001](https://docs.cloud.google.com/bigquery/docs/reference/odbc-jdbc-drivers#odbc_release_2501001)

- You can now configure the connector to authenticate the connection using an external account (workforce or workload identity federation), with limited support, using Azure AD and Okta identity providers.
- You can now configure the connector to use Private Service Connect URLs.
- The connector now supports ODBC transaction APIs. BigQuery supports multi-statement transactions inside a single query, or across multiple queries, when using sessions.
- The connector is now verified to use a default project for datasets. To do this, set the `dataset_project_id` property in `QueryProperties` of the connection string to the desired project.
- `MATERIALIZED_VIEW` has been added to the list of table types. To retrieve these table types, configure `SQLTables` to `TABLE_TYPES_ONLY`.
- The connector now supports the JSON data type.
Feature [JDBC driver update, release 1.3.0 1001](https://docs.cloud.google.com/bigquery/docs/reference/odbc-jdbc-drivers#jdbc_release_130_1001)

- You can now configure the connector to authenticate the connection using an external account (workforce or workload identity federation).
- You can now configure the connector to use Private Service Connect URLs.
- The connector now supports JDBC transaction APIs. BigQuery supports multi-statement transactions inside a single query, or across multiple queries, when using sessions.
- The connector is now verified to use a default project for datasets. To do this, set the `dataset_project_id` property in `QueryProperties` of the connection string to the desired project.
- `MATERIALIZED_VIEW` has been added to the list of table types when using the `getTableTypes` function.
- The connector now supports the JSON data type.
Feature The [`is_case_insensitive` schema option](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#schema_option_list), which allows you to make a dataset and its table names case-insensitive, is now in [preview](https://cloud.google.com/products/#product-launch-stages).

## September 13, 2022

Feature In [Cloud Monitoring](https://docs.cloud.google.com/bigquery/docs/monitoring-dashboard#view_quota_usage_and_limits), you can view metrics for quota usage and limits of the Storage Write API's [concurrent connections and throughput quotas](https://docs.cloud.google.com/bigquery/quotas#write-api-limits). This feature is now [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).

## September 09, 2022

Feature The following features are now [generally available](https://cloud.google.com/products#product-launch-stages) for [`ARIMA_PLUS`](https://docs.cloud.google.com/bigquery-ml/docs/reference/standard-sql/bigqueryml-syntax-create-time-series) models:

- The [`HOLIDAY_REGION`](https://docs.cloud.google.com/bigquery-ml/docs/reference/standard-sql/bigqueryml-syntax-create-time-series#holiday_region) option can now take more than one region string as input. If you include more than one region string, the union of the holidays in all of the provided regions will be taken into the modeling.
- You can use the new [`TREND_SMOOTHING_WINDOW_SIZE`](https://docs.cloud.google.com/bigquery-ml/docs/reference/standard-sql/bigqueryml-syntax-create-time-series#trend_smoothing_window_size) option to smooth the trend component of the time series by applying a center moving average.

## September 06, 2022

Feature **Cloud console updates**: Improvements that are related to query execution include the following:

- For long-running queries, the **Execution details** tab is automatically displayed with the timing details of each stage of the query.

- In the query editor, you can now see the query validation message when your query is completed or canceled.

## September 01, 2022

Feature The [Random Forest model](https://docs.cloud.google.com/bigquery-ml/docs/reference/standard-sql/bigqueryml-syntax-create-random-forest) is now [generally available](https://cloud.google.com/products#product-launch-stages) (GA). For more information, see the random forest sections in the [end-to-end user journey page](https://docs.cloud.google.com/bigquery-ml/docs/reference/standard-sql/bigqueryml-syntax-e2e-journey).
Feature [Customer-managed encryption keys](https://docs.cloud.google.com/bigquery/docs/customer-managed-encryption) are now integrated with [CMEK organization policies](https://docs.cloud.google.com/kms/docs/cmek-org-policy). This feature is [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).
Feature The [slot recommender](https://docs.cloud.google.com/bigquery/docs/slot-recommender) creates recommendations for customers using on-demand billing and is now [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).

## August 29, 2022

Feature [Community contributed UDFs](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/user-defined-functions#community-contributed_functions) are now [generally available](https://cloud.google.com/products/#product-launch-stages) in the `bigquery-utils` GitHub repository and the `bigquery-public-data.persistent_udfs` public dataset.
Feature **Cloud console updates** : In the query editor, when you select a function signature from the autocomplete list, you can remove the parameter names quickly by pressing the <kbd>Backspace</kbd> or <kbd>Delete</kbd> key.

## August 19, 2022

Feature The [`ALTER TABLE RENAME COLUMN` DDL statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#alter_table_rename_column_statement), which allows you to rename the columns of a table, is now in [preview](https://cloud.google.com/products/#product-launch-stages).

## August 17, 2022

Feature **Cloud console updates**: You can now copy BigQuery metadata to your clipboard by using the following options:

- In the **Schema** view, to copy a table's schema, select any fields, and then click **Copy**.

- In the **Explorer** pane, to copy the ID of a resource, click **View actions** , and then click **Copy ID**.

Feature You can now set [default values](https://docs.cloud.google.com/bigquery/docs/default-values) on columns in your BigQuery tables. This feature is now in [preview](https://cloud.google.com/products/#product-launch-stages).
Feature **Cloud console updates**: Improvements include the following:

- Query results are now displayed in resizable columns.

- Tab titles now expand when space is available for longer names.

- Tooltips no longer display text immediately when you hold the pointer over them, avoiding unnecessary distraction.

- In the **Explorer** pane, you can now access saved queries by expanding your project. The **Saved Queries** pane is no longer at the bottom of the console.

- In the **Explorer** pane, you can now find a table by searching for `mydataset.mytable`.

- In the query editor, you can now press the <kbd>F1</kbd> shortcut key to view more editor shortcuts.

## August 16, 2022

Feature [Workforce identity federation](https://docs.cloud.google.com/iam/docs/workforce-identity-federation) lets you authenticate and authorize users from external identity providers to access supported Google Cloud products, including [BigQuery resources](https://docs.cloud.google.com/iam/docs/federated-identity-supported-services#products_and_limitations). This feature is now in [preview](https://cloud.google.com/products/#product-launch-stages).

## August 15, 2022

Change Previously, you could commit up to 100 GB in streaming bytes for every Storage Write API pending mode commit that you triggered in regions other than the US and EU multi-regions. This limit is now 1 TB. For more information, see [Storage Write API quotas](https://docs.cloud.google.com/bigquery/quotas#write-api-limits).
Feature [BigQuery Omni](https://docs.cloud.google.com/bigquery/docs/omni-introduction) now supports [reservation DDL](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_reservation_statement) and [access control DCL](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-control-language). This feature is now [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).
Issue An issue was identified in the `max_staleness` materialized view option. This feature is not available.

## August 10, 2022

Feature You can now [manage query execution priority for Cloud Spanner federated queries](https://docs.cloud.google.com/bigquery/docs/cloud-spanner-federated-queries#manage_query_execution_priority). This feature is now [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).
Feature You can now set [default configurations](https://docs.cloud.google.com/bigquery/docs/default-configuration) at a project or organization level. This feature is now [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).

## August 09, 2022

Feature [Querying Google Cloud Bigtable external data sources](https://docs.cloud.google.com/bigquery/docs/external-data-bigtable) is now [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).

## August 03, 2022

Feature The [`max_staleness` materialized view option](https://docs.cloud.google.com/bigquery/docs/materialized-views-create#max_staleness) helps you achieve consistently high performance with controlled costs when processing large, frequently changing datasets. This feature is now in [preview](https://cloud.google.com/products/#product-launch-stages).

## August 01, 2022

Feature The trigonometric SQL function [CBRT](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#cbrt) is now [generally available](https://cloud.google.com/products/#product-launch-stages) (GA). With this function, you can compute the cube root of a value.
Feature The [`LOAD DATA`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/other-statements#load_data_statement) statement
is now available for [Preview](https://cloud.google.com/products/#product-launch-stages) in Google Standard SQL for BigQuery.
You can use the `LOAD DATA` statement to load data from one or more files into a table.

## July 28, 2022

Feature You can now create [BigQuery subscriptions](https://docs.cloud.google.com/pubsub/docs/bigquery) in [Pub/Sub](https://docs.cloud.google.com/pubsub/docs/overview) to write messages directly to an existing BigQuery table.

## July 27, 2022

Feature Inverse trigonometric SQL functions are now [generally available](https://cloud.google.com/products/#product-launch-stages) (GA). These functions include:

- [COT](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#cot): Compute the cotangent for an angle.
- [COTH](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#coth): Compute the hyperbolic cotangent for an angle.
- [CSC](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#csc): Compute the cosecant for an angle.
- [CSCH](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#csch): Compute the hyperbolic cosecant for an angle.
- [SEC](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#sec): Compute the secant for an angle.
- [SECH](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#sech): Compute the hyperbolic secant for an angle.

## July 25, 2022

Announcement The new **Migrate** section in the BigQuery documentation helps you migrate to BigQuery. This includes high-level guidance with a [migration overview](https://cloud.google.com/bigquery/docs/migration/migration-overview), an [introduction to free-to-use tools](https://cloud.google.com/bigquery/docs/migration-intro) that help you with each phase of migration, and platform-specific migration guides.
Feature [BigLake](https://docs.cloud.google.com/bigquery/docs/biglake-intro) is now [generally available](https://cloud.google.com/products/#product-launch-stages) (GA). You can now create [BigQuery ML](https://docs.cloud.google.com/bigquery-ml/docs/introduction) models using data in Cloud Storage by using BigLake and publish BigLake tables as [Analytics Hub listings](https://docs.cloud.google.com/bigquery/docs/analytics-hub-introduction).

## July 20, 2022

Change Analytics Hub is now available in additional regions across the Americas, Asia Pacific, and Europe. For more information, see [Analytics Hub supported regions](https://docs.cloud.google.com/bigquery/docs/analytics-hub-introduction#supported-regions).

## July 14, 2022

Change Previously, the Storage Write API had a maximum concurrent connection limit of 100 connections for non-multi-regions such as Montreal (northamerica-northeast1). This limit has now been increased to 1,000 connections across all non-multi-regions. For more information, see [Storage Write API quotas and limits](https://docs.cloud.google.com/bigquery/quotas#write-api-limits).

## July 12, 2022

Feature You can now [select a job type](https://cloud.google.com/bigquery/docs/reservations-assignments#console) when assigning a folder, organization, or project to a reservation in the Cloud console. This feature is now [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).

## July 11, 2022

Deprecated The google.cloud.bigquery.reservation.v1beta1.api package is deprecated and will be removed on September 27, 2022. After that date, requests to that package will fail. Data created by using google.cloud.bigquery.reservation.v1beta1.api are accessible by using the [google.cloud.bigquery.reservation.v1.api package](https://docs.cloud.google.com/bigquery/docs/reference/reservations/rpc/google.cloud.bigquery.reservation.v1).

**Next steps:**

- If you use the API directly, you should switch to [google.cloud.bigquery.reservation.v1.api](https://docs.cloud.google.com/bigquery/docs/reference/reservations/rpc/google.cloud.bigquery.reservation.v1), the GA version of the API, to prevent any impact on your workflow.
- If you only use the Cloud console to manage BigQuery reservations, no action is needed.
- If you use the [bq command-line tool](https://docs.cloud.google.com/bigquery/docs/bq-command-line-tool) to manage BigQuery reservations, [upgrade the tool to the latest version](https://docs.cloud.google.com/bigquery/docs/bq-command-line-tool#keep-sdk-up-to-date).

## July 07, 2022

Feature [Azure workload identity federation](https://docs.cloud.google.com/bigquery/docs/omni-azure-create-connection#federated-identity) is now available in [preview](https://cloud.google.com/products/#product-launch-stages) for BigQuery Omni connections. This feature helps you secure data by allowing you to grant Google access to an application you manage in your Azure tenant so that neither you nor Google must manage application client secrets.

## July 06, 2022

Feature The [`APPENDS` change history TVF](https://docs.cloud.google.com/bigquery/docs/change-history) is now in [preview](https://cloud.google.com/products/#product-launch-stages). This table-valued function provides a history of table appends over a window of time.

## July 01, 2022

Change An updated version of [JDBC driver for BigQuery](https://docs.cloud.google.com/bigquery/docs/reference/odbc-jdbc-drivers#current_jdbc_driver) is now available. This version includes a fix for an issue with connector returning stack overflow in some cases when executing complex long queries.

## June 29, 2022

Change Previously, all BigQuery BI Engine projects had a maximum reservation size per project per location limit of 100 GB. This limit is now 250 GB. For more information, see [BI Engine quotas and limits](https://docs.cloud.google.com/bigquery/quotas#biengine-limits).
Feature You can now set the [`view` field](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables/get#tablemetadataview) in the `tables.get()` API method to indicate which table information is returned. Setting the value to `BASIC` reduces latency by omitting some storage statistics.

## June 23, 2022

Feature The [BI Engine preferred tables](https://docs.cloud.google.com/bigquery/docs/bi-engine-preferred-tables) feature lets you limit BI Engine acceleration to a specified set of tables. This feature is now in [preview](https://cloud.google.com/products/#product-launch-stages).

## June 21, 2022

Feature [Query queues](https://docs.cloud.google.com/bigquery/docs/query-queues) are now available in [preview](https://cloud.google.com/products/#product-launch-stages) for on-demand and flat-rate customers. When query queues are enabled, BigQuery automatically determines the query concurrency rather than setting a fixed limit. Flat-rate customers can override this setting with a custom concurrency target. Additional queries beyond the concurrency target are queued until processing resources become available.

## June 15, 2022

Feature [Deterministic encryption SQL functions](https://docs.cloud.google.com/bigquery/docs/column-key-encrypt) are now [generally available](https://cloud.google.com/products/#product-launch-stages) (GA). New AEAD encryption functions include [`DETERMINISTIC_ENCRYPT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aead_encryption_functions#deterministic_encrypt), [`DETERMINISTIC_DECRYPT_BYTES`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aead_encryption_functions#deterministic_decrypt_bytes), and
[`DETERMINISTIC_DECRYPT_STRING`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aead_encryption_functions#deterministic_decrypt_string). These functions allow column-level encryption and decryption of data while supporting aggregation and table joins.

## June 14, 2022

Feature You can now [explore data in Data Studio](https://docs.cloud.google.com/bigquery/docs/visualize-data-studio) by using links from your BigQuery query results in the Cloud console. This feature is now [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).
Feature You can now use the Cloud console to [set up VPC service control perimeters](https://docs.cloud.google.com/bigquery/docs/omni-vpc-sc) to restrict access from [BigQuery Omni](https://docs.cloud.google.com/bigquery/docs/omni-introduction) to external clouds. You can also specify whether you want to grant read or write permission on your external resource. This feature is now [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).

## June 13, 2022

Feature A new system variable, [`@@dataset_project_id`](https://docs.cloud.google.com/bigquery/docs/reference/system-variables), is now [generally available](https://cloud.google.com/products/#product-launch-stages). `@@dataset_project_id` allows you to set a default project where one is not specified for a dataset in your query. This variable is also available as a [Connection Property](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/ConnectionProperty).

## June 10, 2022

Change [Quotas for multi-statement queries](https://docs.cloud.google.com/bigquery/quotas#multi_statement_query_limits) have changed. The cumulative time limit for a multi-statement query has increased from 6 hours to 24 hours.

## June 08, 2022

Feature [Batch](https://docs.cloud.google.com/bigquery/docs/batch-sql-translator) and
[interactive](https://docs.cloud.google.com/bigquery/docs/interactive-sql-translator) translation services are now
[generally available](https://cloud.google.com/products/#product-launch-stages) (GA), and include support for most major SQL dialects. This release also includes
[preview](https://cloud.google.com/products/#product-launch-stages) availability of
[SQL object name mapping](https://docs.cloud.google.com/bigquery/docs/output-name-mapping) and
[metadata extraction](https://docs.cloud.google.com/bigquery/docs/generate-metadata) tools that you can use to increase the accuracy of your batch translation jobs.

## June 06, 2022

Feature You can now attach [Resource Manager tags](https://docs.cloud.google.com/bigquery/docs/tags) to datasets. This feature is supported in [Preview](https://cloud.google.com/products/#product-launch-stages). Tags let you conditionally apply Identity and Access Management (IAM) policies to resources.
Change The following [Storage Read API](https://docs.cloud.google.com/bigquery/docs/reference/storage) quotas and limits have changed:

- There is now a limit of 2,000 concurrent `ReadRows` calls per project in the `US` and `EU` multi-regions and 400 concurrent `ReadRows` calls in other regions.
- The number of data plane requests per user per project per minute has increased from 5,000 to 25,000.

For more information, see [Storage Read API quotas and limits](https://docs.cloud.google.com/bigquery/quotas#storage-limits).

## June 03, 2022

Feature [BigQuery Omni](https://docs.cloud.google.com/bigquery/docs/omni-introduction) now supports [Reservation](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_reservation_statement) and [Access Control DCL](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-control-language). This feature is in [Preview](https://cloud.google.com/products/#product-launch-stages).

## May 31, 2022

Feature [Column-level data masking](https://docs.cloud.google.com/bigquery/docs/column-data-masking-intro)
is now available in
[preview](https://cloud.google.com/products/#product-launch-stages).
You can use data masking to selectively obscure column data for groups of users,
while still allowing access to the column. When you use data masking in combination with
[column-level access control](https://docs.cloud.google.com/bigquery/docs/column-level-security-intro),
you can configure a range of access to column data, from full access to no
access, based on the requirements of different groups of users.

## May 24, 2022

Feature You can now [load data into BigQuery using Informatica Data Loader](https://docs.cloud.google.com/bigquery/docs/load-data-third-party). This feature is [generally available](https://cloud.google.com/products/#product-launch-stages). [Informatica](https://docs.cloud.google.com/bigquery/docs/bigquery-ready-partners#informatica) provides connectors that can ingest data into BigQuery.

## May 23, 2022

Change Metrics for **query/statement_scanned_bytes** and **query/statement_scanned_bytes_billed** are no longer delayed for 6 hours in order to smooth reporting over the duration of the job. Values are now reported every 180 seconds without smoothing. For more information about metrics, see [Google Cloud metrics](https://docs.cloud.google.com/monitoring/api/metrics_gcp#gcp-bigquery).

## May 18, 2022

Change Updated versions of [ODBC and JDBC drivers for BigQuery](https://docs.cloud.google.com/bigquery/docs/reference/odbc-jdbc-drivers) are now available that include enhancements.

## May 05, 2022

Feature The new format element [`%J`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/format-elements) is [generally available](https://cloud.google.com/products/#product-launch-stages) (GA) for [`DATE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions), [`TIME`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/time_functions), [`DATETIME`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/datetime_functions), and [`TIMESTAMP`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions) functions. This format element lets you use the [ISO 8601](https://en.wikipedia.org/wiki/ISO_8601) 1-based day of the year.
Change [`PARSE_DATE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#parse_date), [`PARSE_TIME`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/time_functions#parse_time), [`PARSE_DATETIME`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/datetime_functions#parse_datetime), and [`PARSE_TIMESTAMP`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#parse_timestamp) now support the following [date and time format elements](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/format-elements): `%a`, `%A`, `%g`, `%G`, `%j`, `%u`, `%U`, `%V`, `%w`, and `%W`.

## May 03, 2022

Feature The following new features are now [generally available](https://cloud.google.com/products/#product-launch-stages) (GA) for [`ARIMA_PLUS` models](https://docs.cloud.google.com/bigquery-ml/docs/reference/standard-sql/bigqueryml-syntax-create-time-series):

- You can use [ML.EVALUATE](https://docs.cloud.google.com/bigquery-ml/docs/reference/standard-sql/bigqueryml-syntax-evaluate) to calculate new forecasting accuracy metrics such as [MAPE](https://en.wikipedia.org/wiki/Mean_absolute_percentage_error), [SMAPE](https://en.wikipedia.org/wiki/Symmetric_mean_absolute_percentage_error), and [MSE](https://en.wikipedia.org/wiki/Mean_squared_error).
- You can perform fast model training with little or no loss of forecasting accuracy by using the [`TIME_SERIES_LENGTH_FRACTION`](https://docs.cloud.google.com/bigquery-ml/docs/reference/standard-sql/bigqueryml-syntax-create-time-series#time_series_length_fraction), [`MIN_TIME_SERIES_LENGTH`](https://docs.cloud.google.com/bigquery-ml/docs/reference/standard-sql/bigqueryml-syntax-create-time-series#min_time_series_length) and [`MAX_TIME_SERIES_LENGTH`](https://docs.cloud.google.com/bigquery-ml/docs/reference/standard-sql/bigqueryml-syntax-create-time-series#max_time_series_length) options.

To learn how to achieve one hundred times higher scalability with the `ARIMA_PLUS` model while using the new forecasting accuracy metrics, see the [Accelerate `ARIMA_PLUS` to forecast 1 million time series within hours](https://docs.cloud.google.com/bigquery-ml/docs/arima-speed-up-tutorial). You can also read [`ARIMA_PLUS` best practices](https://docs.cloud.google.com/bigquery-ml/docs/reference/standard-sql/bigqueryml-syntax-create-time-series#large-scale-time-series-forecasting-best-practices).

## May 02, 2022

Feature [Case-insensitive collation](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/collation-concepts) support for BigQuery is now available for [Preview](https://cloud.google.com/products/#product-launch-stages). Collation determines how strings are sorted and compared in collation-supported operations. If case-insensitive collation is used, case is ignored in comparison and sorting operations.

These [operations](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/collation-concepts#collate_operations) support collation:

- [Several comparison operations](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/collation-concepts#operators_2)
- [Join operations](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#join_types)
- [`ORDER BY` operations](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#order_by_clause)
- [`GROUP BY` operations](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#group_by_clause)
- [Several scalar and aggregate function operations](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/collation-concepts#functions_2)
- [Set operations](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#set_operators)
Feature The [`COLLATE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/collation-concepts#collate_ddl) clause is now available for [Preview](https://cloud.google.com/products/#product-launch-stages). With this clause, a collation specification is applied to a specific column in a table. You can use the `COLLATE` clause in the following DDL statements:

- [`ALTER TABLE ADD COLUMN`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#alter_table_add_column_statement)
- [`ALTER COLUMN SET DATA TYPE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#alter_column_set_data_type_statement)
Feature The [`COLLATE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#collate) function is now available for [Preview](https://cloud.google.com/products/#product-launch-stages) in Google Standard SQL for BigQuery. With the `COLLATE` function, you can pass in a `STRING` and return a `STRING` with a collation specification.
Feature The [`DEFAULT COLLATE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/collation-concepts#collate_ddl) clause is now available for [Preview](https://cloud.google.com/products/#product-launch-stages). With this clause, the default collation specification is applied to all column data types supporting collation. You can use the `DEFAULT COLLATE` clause in the following DDL statements:

- [`CREATE SCHEMA`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_schema_statement) and [`ALTER SCHEMA`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#alter_schema_collate_statement)
- [`CREATE TABLE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_table_statement) and [`ALTER TABLE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#alter_table_collate_statement)

## April 25, 2022

Feature [BigQuery Admin Resource Charts](https://docs.cloud.google.com/bigquery/docs/admin-resource-charts) are now [generally available](https://cloud.google.com/products/#product-launch-stages) (GA) for on-demand users, enabling administrators to monitor key metrics and troubleshoot issues across the entire organization. Previously, it was only available for reservation users. A new permission, [bigquery.jobs.listExecutionMetadata](https://docs.cloud.google.com/bigquery/docs/access-control), has been added to make it easier to gain access to the full UI.
Feature Use the
[`TABLE_STORAGE` view](https://docs.cloud.google.com/bigquery/docs/information-schema-table-storage) to get a snapshot of current storage usage for tables and materialized views. This feature is now in
[Preview](https://cloud.google.com/products/#product-launch-stages).
Feature The [ability to
configure the time travel window](https://cloud.google.com/bigquery/docs/time-travel#configure_the_time_travel_window) is now in [Preview](https://cloud.google.com/products/#product-launch-stages). You can specify the
duration of the time travel window, from a minimum of two days to a
maximum of seven days.

## April 11, 2022

Change Starting in July 2022, the [`projects.list`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/projects/list) API method will return results in unsorted order. Currently, the API returns the results in sorted order, although this is not a documented behavior of the API.

## April 07, 2022

Feature [BigLake](https://docs.cloud.google.com/biglake) is now available in [Preview](https://cloud.google.com/products/#product-launch-stages). BigLake is a storage engine that allows you to query and unify cross-cloud data lakes and warehouses. Additionally, it provides fine-grained access controls to your tables, allowing you to set access policies on a column or row basis.
Feature BigQuery now supports the creation of [search indexes](https://docs.cloud.google.com/bigquery/docs/search-intro) and a [`SEARCH`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/search_functions) function. This feature is in [Preview](https://cloud.google.com/products/#product-launch-stages). This enables you to use Google Standard SQL to efficiently find data elements in unstructured text and semi-structured data.

## April 06, 2022

Feature [Analytics Hub](https://docs.cloud.google.com/bigquery/docs/analytics-hub-introduction) is now available in [Preview](https://cloud.google.com/products/#product-launch-stages). Analytics Hub is a new service in BigQuery that lets you create secure data exchanges and share analytics assets within and across organizations. This platform allows data providers to publish listings that reference shared datasets. Analytics Hub subscribers can then view and subscribe to these listings.

## April 05, 2022

Feature [BigQuery Omni](https://docs.cloud.google.com/bigquery-omni/docs/introduction) now supports cross-cloud transfer. This feature is in [Preview](https://cloud.google.com/products/#product-launch-stages). For more information, see [Cross-cloud transfer (AWS)](https://docs.cloud.google.com/bigquery-omni/docs/aws/omni-load-data) and [Cross-cloud transfer (Azure)](https://docs.cloud.google.com/bigquery-omni/docs/azure/omni-load-data).

## April 01, 2022

Feature [BigQuery ML and Vertex AI Model Registry](http://cloud.google.com/bigquery-ml/docs/managing-models-vertex) integration is available in [preview](https://cloud.google.com/products/#product-launch-stages). With this integration, BigQuery ML models can be sent to the [Vertex AI Model Registry](http://cloud.google.com/vertex-ai/docs/model-registry/introduction) where you can manage the lifecycle of all your ML models. From the Vertex AI Model Registry, you can organize your BigQuery ML models and deploy directly to endpoints.

## March 31, 2022

Feature The international public dataset for Data Signals for Google Search Trends is now available in [Preview](https://cloud.google.com/products/#product-launch-stages) and available in the [Google Cloud Marketplace](https://console.cloud.google.com/marketplace/product/bigquery-public-datasets/google-search-trends) and [Analytics Hub](https://console.cloud.google.com/bigquery(analyticshub:projects/1057666841514/locations/us/dataExchanges/google_cloud_public_datasets_17e74966199/listings/17561ab059154c988f72c7ae52d6a3c4)?project=subscriber-project-316517).

## March 28, 2022

Feature The [Wide-and-Deep model](https://docs.cloud.google.com/bigquery-ml/docs/reference/standard-sql/bigqueryml-syntax-create-wnd-models) is now [generally available](https://cloud.google.com/products#product-launch-stages) (GA). For more information, see the Wide-and-Deep sections in the [end-to-end user journey page](https://docs.cloud.google.com/bigquery-ml/docs/reference/standard-sql/bigqueryml-syntax-e2e-journey).

## March 17, 2022

Feature The BigQuery [slot recommender](https://docs.cloud.google.com/bigquery/docs/slot-recommender) is now available in [Preview](https://cloud.google.com/products/#product-launch-stages). The slot recommender creates recommendations for customers using on-demand billing. These recommendations help you to understand the cost and performance tradeoffs of purchasing different amounts of slot capacity.

## March 16, 2022

Feature You can now explicitly specify a schema for BigQuery [external tables](https://docs.cloud.google.com/bigquery/external-data-cloud-storage) created over Parquet, ORC, and Avro file formats. Previously, the schema was always auto-detected using the last lexicographic file.

## March 04, 2022

Feature [Session support for BigQuery](https://docs.cloud.google.com/bigquery/docs/sessions-intro) is now [generally available](https://cloud.google.com/products/#product-launch-stages) (GA). In addition to the [features available in the preview](https://docs.cloud.google.com/bigquery/docs/release-notes#September_08_2021), you can:

- [Terminate a session automatically or manually](https://docs.cloud.google.com/bigquery/docs/sessions-terminating).
- [Set a label for all queries in a session](https://docs.cloud.google.com/bigquery/docs/sessions-write-queries#session_labels).
- Get sessions metadata in [`INFORMATION_SCHEMA.SESSIONS_BY_PROJECT`](https://docs.cloud.google.com/bigquery/docs/information-schema-sessions-by-project) and [`INFORMATION_SCHEMA.SESSIONS_BY_USER`](https://docs.cloud.google.com/bigquery/docs/information-schema-sessions-by-user) views.

## February 16, 2022

Feature [Remote functions](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/remote-functions) are now available for [preview](https://cloud.google.com/products/#product-launch-stages). Remote functions allow you to implement your function in other languages than SQL and Javascript, or with libraries or services which are not allowed in BigQuery user-defined functions.

## February 15, 2022

Feature The [table clones feature](https://docs.cloud.google.com/bigquery/docs/table-clones-intro) in BigQuery is now in [Preview](https://cloud.google.com/products/#product-launch-stages). A table clone is a lightweight, writable copy of a table. You are only charged for storing the data in a table clone that differs from its base table.

## February 14, 2022

Announcement [BigQuery reliability guide](https://docs.cloud.google.com/bigquery/docs/reliability-intro) is now available. This guide describes how to build solutions with BigQuery that meet your application's needs for availability, durability, consistency, and data recovery. Topics include the following:

- [Import reliability](https://docs.cloud.google.com/bigquery/docs/reliability-import) - Managed storage, methods, load jobs, and the Storage Write API
- [Query reliability](https://docs.cloud.google.com/bigquery/docs/reliability-query) - Slots, reservations, and job optimization.
- [Read reliability](https://docs.cloud.google.com/bigquery/docs/reliability-read) - Read methods, consistency concerns including quotas and limits, and the Storage Read API.
- [Disaster planning](https://docs.cloud.google.com/bigquery/docs/reliability-disaster) - Disaster considerations and their mitigation.
Change BigQuery ML time series [ARIMA_PLUS](https://docs.cloud.google.com/bigquery-ml/docs/reference/standard-sql/bigqueryml-syntax-create-time-series) now trains models 5 times faster than previous training.
Feature The [`QUALIFY` clause](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#qualify_clause), which lets you filter the results of analytic functions in Google Standard SQL, is now [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).
Feature The [`INFORMATION_SCHEMA.STREAMING_TIMELINE_*`](https://docs.cloud.google.com/bigquery/docs/information-schema-streaming) views are now [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).

## February 10, 2022

Feature [BigQuery Omni](https://docs.cloud.google.com/bigquery-omni/docs/introduction) now supports `INFORMATION_SCHEMA.JOBS_*` and `INFORMATION_SCHEMA.RESERVATION*` views. This feature is in [Preview](https://cloud.google.com/products/#product-launch-stages). For more information, see [View resource metadata (AWS)](https://docs.cloud.google.com/bigquery-omni/docs/aws/create-external-table#view_resource_metadata_with_information_schema) and [View resource metadata (Azure)](https://docs.cloud.google.com/bigquery-omni/docs/azure/create-external-table#view_resource_metadata_with_information_schema).

## February 03, 2022

Feature The [BigQuery migration assessment](https://docs.cloud.google.com/bigquery/docs/migration-assessment) is now available in [Preview](https://cloud.google.com/products/#product-launch-stages). Use this feature to assess the complexity of migrating from your current data warehouse to BigQuery.
Feature BigQuery ML Hyperparameter tuning is now [generally available](https://cloud.google.com/products#product-launch-stages) (GA). You can use this feature to improve model performance by searching for the optimal hyperparameters when training ML models using `CREATE MODEL` statements.

To learn more, check out the following topics:

- [BigQuery ML Hyperparameter Tuning Overview](https://docs.cloud.google.com/bigquery-ml/docs/reference/standard-sql/bigqueryml-syntax-hp-tuning-overview)
- [Using the BigQuery ML Hyperparameter Tuning to improve model performance](https://docs.cloud.google.com/bigquery-ml/docs/hyperparameter-tuning-tutorial)
- [End-to-end user journey for each model](https://docs.cloud.google.com/bigquery-ml/docs/reference/standard-sql/bigqueryml-syntax-e2e-journey)

## February 02, 2022

Feature The [`WITH RECURSIVE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#with_clause) feature has been added to Google Standard SQL for BigQuery and is now in [Preview](https://cloud.google.com/products#product-launch-stages). This feature allows a query in a `WITH` clause to refer to either itself or to queries defined later in the `WITH` clause.

## January 31, 2022

Feature BigQuery now supports [materialized views without aggregation](https://docs.cloud.google.com/bigquery/docs/materialized-views#without_aggr) and [materialized views with inner join](https://docs.cloud.google.com/bigquery/docs/materialized-views#inner_joins). This feature is [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).

## January 25, 2022

Feature Explainable AI in BigQuery ML is now [generally available](https://cloud.google.com/products#product-launch-stages) (GA). This feature helps you understand BigQuery ML prediction or forecasting results at scale. For additional information about explainable AI, see the following:

- [Explainable AI documentation](https://docs.cloud.google.com/bigquery-ml/docs/reference/standard-sql/bigqueryml-syntax-xai-overview)
- Blog post: [BigQuery Explainable AI helps you interpret your ML models](https://cloud.google.com/blog/topics/developers-practitioners/bigquery-explainable-ai-now-ga-help-you-interpret-your-machine-learning-models)
- Tutorials for [regression](https://docs.cloud.google.com/bigquery-ml/docs/linear-regression-tutorial#step_six_explain_prediction_results_with_explainable_ai_methods), [classification](https://docs.cloud.google.com/bigquery-ml/docs/logistic-regression-prediction#step_seven_explain_prediction_results_with_explainable_ai_methods), and [forecasting](https://docs.cloud.google.com/bigquery-ml/docs/arima-single-time-series-forecasting-tutorial#step_seven_explain_and_visualize_the_forecasting_results) tasks

## January 06, 2022

Feature BigQuery standard SQL now supports the [`JSON`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#json_type) data type for storing JSON data. The `JSON` data type is in [Preview](https://cloud.google.com/products/#product-launch-stages). For more information, see [Working with JSON data in Standard SQL](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json-data).

## December 23, 2021

Announcement Documentation now includes a series of introductory topics to orient you to BigQuery including:

- [What is BigQuery?](https://docs.cloud.google.com/bigquery/docs/introduction) - Product overview, available tools, and learning resources
- [Storage](https://docs.cloud.google.com/bigquery/docs/storage_overview) - Infrastructure, ingestion, and optimization
- [Analytics](https://docs.cloud.google.com/bigquery/docs/query-overview) - Strategies, SQL queries, and BI tools
- [Administration](https://docs.cloud.google.com/bigquery/docs/admin-intro) - Resources, workload management, security, and monitoring

In addition, the table of contents is updated to guide you through your staged BigQuery deployment with stages including: [Discovery](https://docs.cloud.google.com/bigquery/docs/introduction), [Get started](https://docs.cloud.google.com/bigquery/docs/quickstarts/quickstart-cloud-console), [Design](https://docs.cloud.google.com/bigquery/docs/resource-hierarchy), [Ingest](https://docs.cloud.google.com/bigquery/docs/loading-data), [Analyze](https://docs.cloud.google.com/bigquery/docs/query-overview), [Administer](https://docs.cloud.google.com/bigquery/docs/admin-intro), [Secure](https://docs.cloud.google.com/bigquery/docs/data-governance), and [Develop](https://docs.cloud.google.com/bigquery/docs/reference/libraries-overview).

## December 16, 2021

Change The [row-level security](https://docs.cloud.google.com/bigquery/docs/row-level-security-intro) feature now supports administrator access to [historical data](https://docs.cloud.google.com/bigquery/docs/time-travel#time_travel_and_row-level_access) for tables with row-level access policies.

## December 14, 2021

Feature BigQuery [BI Engine SQL interface](https://docs.cloud.google.com/bi-engine/docs/sql-interface-overview) is now [generally available](https://cloud.google.com/products#product-launch-stages).

## December 06, 2021

Feature [Anomaly detection](https://docs.cloud.google.com/bigquery-ml/docs/reference/standard-sql/bigqueryml-syntax-inference-overview#anomaly_detection) in BigQuery ML is now [generally available](https://cloud.google.com/products#product-launch-stages)(GA). You can use the [ML.DETECT_ANOMALIES](https://docs.cloud.google.com/bigquery-ml/docs/reference/standard-sql/bigqueryml-syntax-detect-anomalies) function with the [ARIMA_PLUS](https://docs.cloud.google.com/bigquery-ml/docs/reference/standard-sql/bigqueryml-syntax-create-time-series) model to detect anomalies in time-series data. You can also use this function with the [K-means](https://docs.cloud.google.com/bigquery-ml/docs/reference/standard-sql/bigqueryml-syntax-create-kmeans), [Autoencoder](https://docs.cloud.google.com/bigquery-ml/docs/reference/standard-sql/bigqueryml-syntax-create-autoencoder), or [PCA](https://docs.cloud.google.com/bigquery-ml/docs/reference/standard-sql/bigqueryml-syntax-create-pca) models to detect anomalies in independent and identically distributed (IID) data.

## December 03, 2021

Feature The [principal component analysis (PCA) model](https://docs.cloud.google.com/bigquery-ml/docs/reference/standard-sql/bigqueryml-syntax-create-pca) and the [autoencoder model](https://docs.cloud.google.com/bigquery-ml/docs/reference/standard-sql/bigqueryml-syntax-create-autoencoder) are now [generally available](https://cloud.google.com/products#product-launch-stages) (GA). You can use these models for common machine learning tasks such as dimensionality reduction, feature embedding, and unsupervised anomaly detection.

For more information, see the PCA and autoencoder sections in the [end-to-end user journey page](https://docs.cloud.google.com/bigquery-ml/docs/reference/standard-sql/bigqueryml-syntax-e2e-journey).

## December 01, 2021

Feature BigQuery Data Transfer Service now supports [Audit Logging](https://docs.cloud.google.com/bigquery-transfer/docs/audit-logging), [Cloud Logging](https://docs.cloud.google.com/bigquery-transfer/docs/cloud-logging), and [Cloud Monitoring](https://docs.cloud.google.com/bigquery-transfer/docs/cloud-monitoring). These features are in [preview status](https://cloud.google.com/products/#product-launch-stages).

## November 19, 2021

Change Updated versions of [ODBC and JDBC drivers for BigQuery](https://docs.cloud.google.com/bigquery/docs/reference/odbc-jdbc-drivers) are now available that include enhancements.

## November 16, 2021

Change BigQuery Data Transfer Service is now available in the [Santiago (southamerica-west1)](https://docs.cloud.google.com/bigquery-transfer/docs/locations#regional-locations) region.
Change BigQuery is now available in the [Santiago (southamerica-west1)](https://docs.cloud.google.com/bigquery/docs/locations#regional-locations) region.
Change BigQuery BI Engine is now available in the [Santiago (southamerica-west1)](https://docs.cloud.google.com/bi-engine/docs/locations#regional-locations) region.
Change BigQuery ML is now available in the [Santiago (southamerica-west1)](https://docs.cloud.google.com/bigquery-ml/docs/locations#regional-locations) region.

## November 12, 2021

Feature BigQuery now supports [authorized datasets](https://docs.cloud.google.com/bigquery/docs/authorized-datasets) ([General Availability)](https://cloud.google.com/products#product-launch-stages).

## November 09, 2021

Feature The following scripting statements have been added to Google Standard SQL for BigQuery.

- [CASE](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/scripting#case): Executes the first list of SQL statements where a boolean expression is `TRUE`.
- [CASE search_expression](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/scripting#case_search_expression): Executes the first list of SQL statements where the search expression matches a `WHEN` expression.
- [LABELS](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/scripting#labels): Provides an unconditional jump to the end of the block or loop associated with a label.
- [REPEAT](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/scripting#repeat): Repeatedly executes a list of SQL statements until the boolean condition at the end of the list is `TRUE`.
- [FOR...IN](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/scripting#for-in): Loops over every row in a table expression.

These features are [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).

## November 08, 2021

Feature The following INFORMATION_SCHEMA views now support a `DDL` column. The value of the column is the DDL statement that can be used to create the resource.

- [`ROUTINES`](https://docs.cloud.google.com/bigquery/docs/information-schema-routines#routines_view)
- [`SCHEMATA`](https://docs.cloud.google.com/bigquery/docs/information-schema-datasets#schemata_view)
- [`TABLES`](https://docs.cloud.google.com/bigquery/docs/information-schema-tables#tables_view)

This feature is [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).

## November 02, 2021

Feature BigQuery now supports [parameterized types](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#parameterized_data_types). The following parameterized types are supported:

- [STRING(L)](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#parameterized_string_type)
- [BYTES(L)](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#parameterized_bytes_type)
- [NUMERIC(P) / NUMERIC(P, S)](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#parameterized_decimal_type)
- [BIGNUMERIC(P) / BIGNUMERIC(P, S)](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#parameterized_decimal_type)

This feature is generally available [GA](https://cloud.google.com/products/#product-launch-stages).

## October 28, 2021

Feature The [table snapshots](https://docs.cloud.google.com/bigquery/docs/table-snapshots-intro) feature is now [generally available](https://cloud.google.com/products/#product-launch-stages) (GA). It includes the Cloud console interface and support for creating a table snapshot in a different project from its base table.

## October 27, 2021

Feature [SQL column-level encryption](https://docs.cloud.google.com/bigquery/docs/column-key-encrypt) using Cloud Key Management Service (KMS) is now [generally available](https://cloud.google.com/products/#product-launch-stages) (GA), letting you encrypt keysets within [AEAD encryption functions](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aead_encryption_functions).

## October 25, 2021

Feature [BigQuery Omni](https://docs.cloud.google.com/bigquery-omni/docs), a multi-cloud analytics solution, is now [generally available](https://cloud.google.com/products/#product-launch-stages).

## October 12, 2021

Feature The BigQuery [Storage Write API](https://docs.cloud.google.com/bigquery/docs/write-api) is now [generally available](https://cloud.google.com/products/#product-launch-stages) (GA). The Storage Write API combines the functionality of high-throughput streaming ingestion and batch loading into a single API.

## October 04, 2021

Feature [BigQuery Migration Service](https://docs.cloud.google.com/bigquery/docs/migration-intro) is now in [Preview](https://cloud.google.com/products/#product-launch-stages).
It includes the following features:

- [Interactive SQL Translator](https://docs.cloud.google.com/bigquery/docs/interactive-sql-translator)
- [Batch SQL Translator](https://docs.cloud.google.com/bigquery/docs/batch-sql-translator)

## October 01, 2021

Feature BigQuery now supports the following geospatial data functions:

- [ST_BUFFER](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_buffer): Returns a `GEOGRAPHY` that represents the buffer around the input `GEOGRAPHY`. You specify the number of segments to determine how much the resulting geography can deviate from the ideal buffer radius.

- [ST_BUFFERWITHTOLERANCE](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_bufferwithtolerance): Returns a `GEOGRAPHY` that represents the buffer around the input `GEOGRAPHY`. You specify the tolerance to determine how much the resulting geography can deviate from the ideal buffer radius.

These functions are available as a [preview](https://cloud.google.com/products/#product-launch-stages).
Announcement BigQuery pricing has changed as follows:

1. [BigQuery Storage Read API](https://cloud.google.com/bigquery/docs/reference/storage) has moved from a single regional SKU to a set of regional SKUs for bytes scanned. All BigQuery Storage Read API users can now read up to 300 TB of data per month at no charge. For more information, see [BigQuery data extraction pricing](https://cloud.google.com/bigquery/pricing#data_extraction_pricing).

2. BigQuery now charges BigQuery Storage Read API users for network egress. For more information, see [BigQuery Storage Read API Network Egress Within Google Cloud](https://cloud.google.com/bigquery/pricing#bigquery_storage_read_api_network_egress_within_google_cloud).

## September 28, 2021

Feature [Table functions](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/table-functions) are now [generally available](https://cloud.google.com/products/?hl=EN#product-launch-stages) (GA). With the GA release, [authorized table functions](https://docs.cloud.google.com/bigquery/docs/authorized-functions) are now supported.

## September 27, 2021

Feature BigQuery now supports the following geospatial data functions:

- [ST_BOUNDINGBOX](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_boundingbox): Returns a `STRUCT` that represents the bounding box for a geography.

- [ST_EXTENT](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_extent): Returns a `STRUCT` that represents the bounding box for a set of geographies.

- [S2_COVERINGCELLIDS](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#s2_coveringcellids): Returns an array of S2 cell IDs that cover a geography.

- [S2_CELLIDFROMPOINT](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#s2_cellidfrompoint): Returns the S2 cell ID covering a point geography.

These functions are [generally available](https://cloud.google.com/products/?hl=EN#product-launch-stages) (GA).

## September 21, 2021

Change When [saving query results](https://docs.cloud.google.com/bigquery/docs/writing-results#downloading-saving-results-console) from the Cloud console to a CSV file, the available download size is now 10 MB. Previously the limit was 16,000 rows. Also, you can now download tables with nested and repeated data to CSV files.

## September 17, 2021

Feature BigQuery now supports the following geospatial data functions:

- [ST_EXTERIORRING](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_exteriorring): Returns a linestring geography that corresponds to the outermost ring of a polygon geography.

- [ST_INTERIORRINGS](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_interiorrings): Returns an array of linestring geographies that corresponds to the interior rings of a polygon geography.

- [ST_ANGLE](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_angle): Returns the angle between two intersecting lines.

- [ST_AZIMUTH](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_azimuth): Returns the azimuth of a line segment formed by two points.

- [ST_NUMGEOMETRIES](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_numgeometries): Returns the number of geometries in a geography.

- [ST_GEOMETRYTYPE](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_geometrytype): Returns the Open Geospatial Consortium (OGC) geometry type that describes a geography as a string.

These functions are [generally available](https://cloud.google.com/products/?hl=EN#product-launch-stages) (GA).

## September 16, 2021

Feature BigQuery ML documentation has been updated with the following improvements:

- The [end-to-end user journey](https://docs.cloud.google.com/bigquery-ml/docs/reference/standard-sql/bigqueryml-syntax-e2e-journey) now includes an overview of the machine-learning workflow for each available model.
- Each machine learning module now provides an overview document that describes the BigQuery ML behavior and links to additional guidance. New documentation includes the following:
  - [Model creation overview](https://docs.cloud.google.com/bigquery-ml/docs/reference/standard-sql/bigqueryml-syntax-create)
  - [Preprocessing overview](https://docs.cloud.google.com/bigquery-ml/docs/reference/standard-sql/bigqueryml-syntax-preprocess-overview)
  - [Hyperparameter tuning overview](https://docs.cloud.google.com/bigquery-ml/docs/reference/standard-sql/bigqueryml-syntax-hp-tuning-overview)
  - [Model evaluation overview](https://docs.cloud.google.com/bigquery-ml/docs/reference/standard-sql/bigqueryml-syntax-evaluate-overview)
  - [Model inference overview](https://docs.cloud.google.com/bigquery-ml/docs/reference/standard-sql/bigqueryml-syntax-inference-overview)
  - [Explainable AI overview](https://docs.cloud.google.com/bigquery-ml/docs/reference/standard-sql/bigqueryml-syntax-xai-overview)
  - [Model weights overview](https://docs.cloud.google.com/bigquery-ml/docs/reference/standard-sql/bigqueryml-syntax-weights-overview)
- Improvements to documentation organization and content, as well as the addition of new [landing pages](https://docs.cloud.google.com/bigquery-ml/docs).

## September 08, 2021

Feature [Session support for BigQuery](https://docs.cloud.google.com/bigquery/docs/sessions-intro) is now in [Preview](https://cloud.google.com/products/#product-launch-stages). With sessions:

- You can associate your SQL activities in a session across scripts and multi-statement transactions in BigQuery with a unique session identifier.
- You can use session variables (for example, default timezone or dataset) and temporary tables throughout the life of the session and also across scripts and transactions
- When you enable sessions, all actions performed across multiple sessions can be viewed using the `SESSION_ID` column now available in jobs `INFORMATION_SCHEMA` views.
Feature [Deleting the metadata for a specific job](https://docs.cloud.google.com/bigquery/docs/managing-jobs#delete_job_metadata) using the `bq` command-line tool is now [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).

## August 31, 2021

Change An updated version of [ODBC driver for BigQuery](https://docs.cloud.google.com/bigquery/docs/reference/odbc-jdbc-drivers#current_odbc_driver) is now available that includes enhancements.

## August 30, 2021

Feature Exporting table data in [Parquet format](https://docs.cloud.google.com/bigquery/docs/exporting-data#parquet_export_details) is now [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).

## August 23, 2021

Feature [BigQuery Admin Resource Charts](https://docs.cloud.google.com/bigquery/docs/admin-resource-charts) are now [generally available](https://cloud.google.com/products/#product-launch-stages) (GA) for reservation users, enabling administrators to more easily monitor and troubleshoot their BigQuery environment. They provide visibility into key metrics such as slot consumption, job concurrency, job execution time, job errors, and bytes processed across the entire organization.
Feature BigQuery [Slot Estimator](https://docs.cloud.google.com/bigquery/docs/slot-estimator) is now in [Preview](https://cloud.google.com/products/#product-launch-stages) for reservation users. This tool analyzes slot utilization data to help administrators estimate the right number of slots to purchase, and provides insights on how job performance might be impacted by adding or reducing slot capacity for the entire organization or specific reservations.

## August 19, 2021

Feature [Cloud Spanner federated queries](https://cloud.google.com/bigquery/docs/cloud-spanner-federated-queries) are now generally available [(GA)](https://docs.cloud.google.com/terms/launch-stages).

## August 06, 2021

Feature The principal component analysis (PCA) model is now available for [preview](https://cloud.google.com/products/#product-launch-stages). For more information, see [CREATE MODEL statement for PCA models](https://docs.cloud.google.com/bigquery-ml/docs/reference/standard-sql/bigqueryml-syntax-create-pca) and the PCA details in the [end-to-end user journey](https://docs.cloud.google.com/bigquery-ml/docs/reference/standard-sql/bigqueryml-syntax-e2e-journey).

## August 03, 2021

Change BigQuery BI Engine is now available in the [Toronto (northamerica-northeast2)](https://docs.cloud.google.com/bi-engine/docs/locations#regional-locations) region.
Change BigQuery Data Transfer Service is now available in the [Toronto (northamerica-northeast2)](https://docs.cloud.google.com/bigquery-transfer/docs/locations#regional-locations) region.
Change BigQuery is now available in the [Toronto (northamerica-northeast2)](https://docs.cloud.google.com/bigquery/docs/locations#regional-locations) region.
Change BigQuery ML is now available in the [Toronto (northamerica-northeast2)](https://docs.cloud.google.com/bigquery-ml/docs/locations#regional-locations) region.
Feature BigQuery now supports the [ALTER COLUMN SET DATA TYPE](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#alter_column_set_data_type_statement) data definition language (DDL) statement. This feature is [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).
Feature BigQuery now supports the following data definition language (DDL) statement:

- [CREATE TABLE LIKE](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_table_like)
- [CREATE TABLE COPY](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_table_copy)

This feature is [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).

## August 02, 2021

Change An updated version of [JDBC driver for BigQuery](https://docs.cloud.google.com/bigquery/docs/reference/odbc-jdbc-drivers#current_jdbc_driver) is now available that includes Enhancements \& New Features.

## July 28, 2021

Feature The [Wide-and-Deep model](https://docs.cloud.google.com/bigquery-ml/docs/reference/standard-sql/bigqueryml-syntax-create-wnd-models) is now available for [preview](https://cloud.google.com/products/#product-launch-stages). `'DNN_LINEAR_COMBINED_CLASSIFIER'` and `'DNN_LINEAR_COMBINED_REGRESSOR'` create Wide-and-Deep Classifier and Regressor models, respectively.

## July 27, 2021

Feature [Explainable artificial intelligence](https://docs.cloud.google.com/bigquery-ml/docs/reference/standard-sql/bigqueryml-syntax-xai-overview) (XAI) helps you understand the results that your predictive
machine-learning model generates for classification and regression tasks by
defining how each feature in a row of data contributed to the predicted result.
This feature is now available for [preview](https://cloud.google.com/products/#product-launch-stages).
Feature BigQuery now supports the [`INTERVAL`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#interval_type) type, which represents a duration or an amount of time. This type is in [Preview](https://cloud.google.com/products/?hl=EN#product-launch-stages).

## July 26, 2021

Feature Time series models now support [holiday effects](https://docs.cloud.google.com/bigquery-ml/docs/reference/standard-sql/bigqueryml-syntax-create-time-series#holiday_region) for weekly time series, in addition to the daily time series that was previously supported. This feature is now [generally available](https://cloud.google.com/products#product-launch-stages) (GA).
Feature DML query jobs now return statistics about the number of rows that were inserted, deleted, or updated. For more information, see [`DmlStats`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/DmlStats) in the [`Job`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/Job) resource type. In addition, DML statistics are now available in the [`INFORMATION_SCHEMA.JOBS_BY_*`](https://docs.cloud.google.com/bigquery/docs/information-schema-jobs#schema) views. This feature is [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).

## July 19, 2021

Feature BigQuery now supports the following SQL query operators:

- [PIVOT operator](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#pivot_operator)
- [UNPIVOT operator](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#unpivot_operator)

This feature is [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).
Feature BigQuery now supports workload management data control language (DCL) statements:

- [CREATE CAPACITY](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-control-language#create_capacity_statement)
- [CREATE RESERVATION](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-control-language#create_reservation_statement)
- [CREATE ASSIGNMENT](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-control-language#create_assignment_statement)
- [DROP CAPACITY](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-control-language#drop_capacity_statement)
- [DROP RESERVATION](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-control-language#drop_reservation_statement)
- [DROP ASSIGNMENT](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-control-language#drop_assignment_statement)

This feature is generally available [GA](https://cloud.google.com/products/#product-launch-stages).
Feature The [end-to-end user journey](https://docs.cloud.google.com/bigquery-ml/docs/reference/standard-sql/bigqueryml-syntax-e2e-journey) for BigQuery ML documents an overview of the complete machine-learning flow for each available model including feature preprocessing, model creation, hyperparameter tuning, inference, evaluation, model export, etc.
Feature BigQuery standard SQL now supports the [`CONTAINS_SUBSTR`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#contains_substr) function. This feature is [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).

## July 07, 2021

Feature BigQuery now supports [materialized views without aggregation](https://docs.cloud.google.com/bigquery/docs/materialized-views#without_aggr) and [materialized views with inner join](https://docs.cloud.google.com/bigquery/docs/materialized-views#inner_joins). This feature is in [Preview](https://cloud.google.com/products/#product-launch-stages).

## July 01, 2021

Change An updated version of [JDBC driver for BigQuery](https://docs.cloud.google.com/bigquery/docs/reference/odbc-jdbc-drivers#current_jdbc_driver) is now available that includes bug fixes, parameterized data type support, and job retry improvements.
Change An updated version of [ODBC driver for BigQuery](https://docs.cloud.google.com/bigquery/docs/reference/odbc-jdbc-drivers#current_odbc_driver) is now available that includes bug fixes, parameterized data type support, and metadata retrieval performance improvements.

## June 29, 2021

Change BigQuery ML is now available in the [Delhi (asia-south2) region](https://docs.cloud.google.com/bigquery-ml/docs/locations#regional-locations).
Change BigQuery is now available in the [Delhi (asia-south2) region](https://docs.cloud.google.com/bigquery/docs/locations#regional-locations).
Change BigQuery BI Engine is now available in the [Delhi (asia-south2) region](https://docs.cloud.google.com/bi-engine/docs/locations#regional-locations).
Feature BigQuery now supports [multi-statement transactions](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/transactions). These allow you to perform mutating operations, such as inserting or deleting rows, on one or more tables, and either commit or roll back the changes atomically. This feature is in [Preview](https://cloud.google.com/products/#product-launch-stages).
Change BigQuery Data Transfer Service is now available in the [Delhi (asia-south2) region](https://docs.cloud.google.com/bigquery-transfer/docs/locations#regional-locations).

## June 28, 2021

Feature [Table functions](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/table-functions) are now available in [Preview](https://cloud.google.com/products/#product-launch-stages). These user-defined functions, commonly known as table-valued functions (TVFs), return a table value.
Feature [Audit logging](https://docs.cloud.google.com/bigquery-transfer/docs/audit-logging), [Cloud Logging](https://docs.cloud.google.com/bigquery-transfer/docs/cloud-logging), and [Cloud Monitoring](https://docs.cloud.google.com/bigquery-transfer/docs/cloud-monitoring) for the BigQuery Data Transfer Service are now [generally available (GA)](https://cloud.google.com/products/#product-launch-stages).
Feature The Google Trends dataset is now in [Preview](https://cloud.google.com/products/#product-launch-stages) and available in the [Google Cloud Marketplace](https://console.cloud.google.com/marketplace/product/bigquery-public-datasets/google-search-trends).
Feature The Google Trends dataset is now available in [Preview](https://cloud.google.com/products/#product-launch-stages) and available in the [Google Cloud Marketplace](https://console.cloud.google.com/marketplace/product/bigquery-public-datasets/google-search-trends).
Feature BigQuery now supports the [ALTER COLUMN SET OPTIONS](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#alter_column_set_options_statement) data definition language (DDL) statement. This feature is [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).
Feature BigQuery now supports the following casting features:

- [PARSE_BIGNUMERIC](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/functions-and-operators#parse_bignumeric)
- [PARSE_NUMERIC](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/functions-and-operators#parse_numeric)
- [Format clause for CAST](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/conversion_functions#formatting_syntax) available for the following data types:
  - String type
  - Date type
  - Datetime type
  - Time type
  - Timestamp type
  - Numeric types
  - Bytes type
- [Numeric type INT64 aliases](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#numeric_types) (INT, SMALLINT, INTEGER, BIGINT, TINYINT, BYTEINT)
- [ST_GEOGFROM](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_geogfrom)

These features are [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).
Feature BigQuery now supports access management data control language (DCL) statements and corresponding views:

- [GRANT](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-control-language#grant_statement)
- [REVOKE](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-control-language#revoke_statement)
- [INFORMATION_SCHEMA.OBJECT_PRIVILEGES view](https://docs.cloud.google.com/bigquery/docs/information-schema-object-privileges)

GRANT and REVOKE statements are [generally available](https://cloud.google.com/products/#product-launch-stages) (GA). OBJECT_PRIVILEGES table is available in [Preview](https://cloud.google.com/products/#product-launch-stages).

## June 25, 2021

Feature BigQuery table snapshots are now in [Preview](https://cloud.google.com/products/#product-launch-stages). A table snapshot is a low-cost, read-only copy of a table's data as it was at a particular time. For more information, see [Introduction to table snapshots](https://docs.cloud.google.com/bigquery/docs/table-snapshots-intro).

## June 22, 2021

Feature BigQuery ML is releasing the following features for [preview](https://cloud.google.com/products/#product-launch-stages):

- The [`ML.DETECT_ANOMALIES` function](https://docs.cloud.google.com/bigquery-ml/docs/reference/standard-sql/bigqueryml-syntax-detect-anomalies) is now available. This function provides anomaly detection for BigQuery ML. The function runs against time-series data using `ARIMA_PLUS` models. The function runs against [independent and identically distributed (IID)](https://en.wikipedia.org/wiki/Independent_and_identically_distributed_random_variables) random variables data using [`AUTOENCODER`](https://docs.cloud.google.com/bigquery-ml/docs/reference/standard-sql/bigqueryml-syntax-create-autoencoder) and [`KMEANS`](https://docs.cloud.google.com/bigquery-ml/docs/reference/standard-sql/bigqueryml-syntax-create-kmeans) models.
- The [`AUTOENCODER` model type](https://docs.cloud.google.com/bigquery-ml/docs/reference/standard-sql/bigqueryml-syntax-create-autoencoder) is now available for CREATE MODEL statements. This is a TensorFlow-based, deep-learning model that supports sparse data representations, and is commonly used in ML tasks such as feature embedding, unsupervised anomaly detection, and non-linear dimensionality reduction. The [ML.PREDICT function](https://docs.cloud.google.com/bigquery-ml/docs/reference/standard-sql/bigqueryml-syntax-predict#predicting_an_outcome_with_a_model_trained_with_the_transform_clause) can use previously built AUTOENCODER models to reduce the dimensionality of query results.
- [Hyperparameter tuning](https://docs.cloud.google.com/bigquery-ml/docs/reference/standard-sql/bigqueryml-hyperparameter-tuning) is now available and can be used to improve model performance by searching for the optimal hyperparameters when training ML models using CREATE MODEL statements. View the [BigQuery ML Hypertuning tutorial](https://docs.cloud.google.com/bigquery-ml/docs/hyperparameter-tuning-tutorial) to learn how to improve model performance by 40%.
Feature BigQuery Data Transfer Service now supports [Google Merchant Center data transfers for local inventories](https://docs.cloud.google.com/bigquery-transfer/docs/merchant-center-local-inventories-schema) and [regional inventories](https://docs.cloud.google.com/bigquery-transfer/docs/merchant-center-regional-inventories-schema).

## June 21, 2021

Feature [Row-level security](https://docs.cloud.google.com/bigquery/docs/row-level-security-intro) on table data is now generally available in BigQuery.
Change BigQuery ML is now available in the [Melbourne (australia-southeast2) region](https://docs.cloud.google.com/bigquery-ml/docs/locations#regional-locations).
Change BigQuery is now available in the [Melbourne (australia-southeast2) region](https://docs.cloud.google.com/bigquery/docs/locations#regional-locations).
Change BigQuery Data Transfer Service is now available in the [Melbourne (australia-southeast2) region](https://docs.cloud.google.com/bigquery-transfer/docs/locations#regional-locations).
Change BigQuery BI Engine is now available in the [Melbourne (australia-southeast2) region](https://docs.cloud.google.com/bi-engine/docs/locations#regional-locations).

## June 07, 2021

Feature BigQuery now supports [parameterized types](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#parameterized_data_types). The following parameterized types are supported:

- [STRING(L)](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#parameterized_string_type)
- [BYTES(L)](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#parameterized_bytes_type)
- [NUMERIC(P) / NUMERIC(P, S)](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#parameterized_decimal_type)
- [BIGNUMERIC(P) / BIGNUMERIC(P, S)](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#parameterized_decimal_types)

This feature is in [Preview](https://cloud.google.com/products/#product-launch-stages).

## May 25, 2021

Announcement The free trial period for BigQuery BI Engine's [SQL interface](https://docs.cloud.google.com/bi-engine/docs/sql-interface-overview) has been extended to July 15th, 2021. You must [enroll](https://docs.cloud.google.com/bi-engine/docs/sql-interface-overview#requesting_access_to_the_preview) to participate in the [preview](https://cloud.google.com/products#product-launch-stages). With this feature, BI Engine now interacts with popular BI tools such as Looker, Tableau, and more, by means of an interactive SQL interface.

## May 20, 2021

Feature BigQuery GIS now supports the following functions. These functions are [generally available](https://cloud.google.com/products/?hl=EN#product-launch-stages) (GA).

- [`ST_STARTPOINT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_startpoint)
- [`ST_ENDPOINT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_endpoint)
- [`ST_POINTN`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_pointn)

These functions return a point of a linestring geography as a point geography.
Feature BigQuery GIS now supports loading geography data from newline-delimited GeoJSON files. This feature is [generally available](https://cloud.google.com/products/#product-launch-stages) (GA). For more information, see [Loading GeoJSON data](https://docs.cloud.google.com/bigquery/docs/gis-data#loading_geojson_data).

## May 19, 2021

Feature BigQuery now supports the ability to rename tables using SQL. See [ALTER TABLE RENAME TO](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#alter_table_rename_to_statement). This feature is [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).

## May 18, 2021

Feature The `CREATE MODEL` statement for training AutoML Tables models is now [generally available](https://cloud.google.com/products/#product-launch-stages) (GA). AutoML Tables enable you to automatically build state-of-the-art machine learning models on structured data at massively increased speed and scale. For more information, see [`CREATE MODEL` statement for training AutoML Tables models](https://docs.cloud.google.com/bigquery-ml/docs/reference/standard-sql/bigqueryml-syntax-create-automl).

## May 11, 2021

Change Updated version of [JDBC driver for BigQuery](https://docs.cloud.google.com/bigquery/docs/reference/odbc-jdbc-drivers) includes bug fixes, service account keyfile support, connection property enhancements, and BigQuery client library updates.
Change Updated version of [ODBC driver for BigQuery](https://docs.cloud.google.com/bigquery/docs/reference/odbc-jdbc-drivers) includes bug fixes and install guide improvements.

## May 10, 2021

Feature BigQuery now supports the following SQL query clauses and operators:

- [PIVOT operator](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#pivot_operator)
- [UNPIVOT operator](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#unpivot_operator)
- [QUALIFY clause](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#qualify_clause)

This feature is in [Preview](https://cloud.google.com/products/#product-launch-stages).

## April 30, 2021

Feature BigQuery now supports the following data definition language (DDL) statements:

- [CREATE VIEW with column name list](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#view_column_name_list)
- [ALTER COLUMN DROP NOT NULL constraint](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#alter_column_drop_not_null_statement)

This feature is in [GA](https://cloud.google.com/products/#product-launch-stages).

## April 21, 2021

Feature BigQuery supports changing an existing non-clustered table to a clustered table and vice versa. You can also update the set of clustered columns of a clustered table. This feature was first documented in October 2020 but was not included in a release note. For more information, see [Modifying clustering specification](https://docs.cloud.google.com/bigquery/docs/creating-clustered-tables#modifying-cluster-spec).

## April 19, 2021

Feature BigQuery ML is introducing new ARIMA_PLUS models and deprecating the ARIMA model type. While the underlying modeling technique has not changed, the following improvements are now available in ARIMA_PLUS:

- Explainable forecasting via [`ML.EXPLAIN_FORECAST`](https://cloud.google.com/bigquery-ml/docs/reference/standard-sql/bigqueryml-syntax-explain-forecast) ([tutorial](https://docs.cloud.google.com/bigquery-ml/docs/arima-multiple-time-series-forecasting-tutorial#step_four_forecast_the_time_series_and_visualize_the_results)).

- More comprehensive evaluation via [`ML.ARIMA_EVALUATE`](https://cloud.google.com/bigquery-ml/docs/reference/standard-sql/bigqueryml-syntax-arima-evaluate) ([tutorial](https://docs.cloud.google.com/bigquery-ml/docs/arima-multiple-time-series-forecasting-tutorial#step_six_inspect_the_evaluation_metrics_of_the_set_of_time_series_models)).

- Multiple ID columns are specifiable via [`time_series_id_col`](https://docs.cloud.google.com/bigquery-ml/docs/reference/standard-sql/bigqueryml-syntax-create-time-series#time_series_id_col).

- Additional time series (500,000) for simultaneous forecasting.

- Two new training options: [`clean_spikes_and_dips`](https://docs.cloud.google.com/bigquery-ml/docs/reference/standard-sql/bigqueryml-syntax-create-time-series#clean_spikes_and_dips) and [`adjust_step_changes`](https://docs.cloud.google.com/bigquery-ml/docs/reference/standard-sql/bigqueryml-syntax-create-time-series#adjust_step_changes).

- Finer data frequency: [`per_minute`](https://docs.cloud.google.com/bigquery-ml/docs/reference/standard-sql/bigqueryml-syntax-create-time-series#data_frequency).

## April 12, 2021

Feature The [BigQuery Admin Resource Charts](https://docs.cloud.google.com/bigquery/docs/admin-resource-charts) [Preview](https://cloud.google.com/products/#product-launch-stages) is now available for [Reservation](https://docs.cloud.google.com/bigquery/docs/reservations-intro) users, enabling administrators to more easily monitor and troubleshoot their BigQuery environment. It provides visibility into key metrics such as slot consumption, job concurrency, and job execution time across the entire organization.

## April 09, 2021

Feature BigQuery now has better support for loading `ENUM` and `LIST` types in Parquet files.

- `ENUM` logical types can be converted to `STRING` or `BYTES`.
- Schema inference is supported for `LIST` logical types.

For more information, see [Loading Parquet data from Cloud Storage](https://docs.cloud.google.com/bigquery/docs/loading-data-cloud-storage-parquet#enum_logical_type).

## April 07, 2021

Announcement Beginning in early Q3 2021, [BigQuery Storage Read API](https://cloud.google.com/bigquery/docs/reference/storage) will start charging for [network egress](https://docs.cloud.google.com/storage/pricing#network-pricing). In addition, BigQuery Storage Read API will become available in all locations, with appropriate pricing. Another release note will be issued when these changes take effect.

## April 06, 2021

Feature The BigQuery Storage Write API is now in [Preview](https://cloud.google.com/products/#product-launch-stages). The Storage Write API is a stream-based API for ingesting data into BigQuery at low cost and high throughput. It provides exactly-once delivery semantics with real-time latency. For more information, see [Using the BigQuery Storage Write API](https://docs.cloud.google.com/bigquery/docs/write-api).

## April 02, 2021

Feature BigQuery standard SQL now supports the [ALTER TABLE DROP COLUMN](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#alter_table_drop_column_statement). This feature is in [Preview](https://cloud.google.com/products/#product-launch-stages).
Change The maximum length has been increased from 128 characters to 300 characters for the following BigQuery fields: table column names, column alias names, and user-defined function names.

## March 31, 2021

Feature The [`INFORMATION_SCHEMA.TABLES`](https://docs.cloud.google.com/bigquery/docs/information-schema-tables) view now includes a `DDL` column that can be used to recreate the table. This feature is in [Preview](https://cloud.google.com/products/#product-launch-stages).
Feature `INFORMATION_SCHEMA` views for [table partitions](https://docs.cloud.google.com/bigquery/docs/information-schema-tables#partitions_view) are now available. This feature is in [Preview](https://cloud.google.com/products/#product-launch-stages).
Feature BigQuery standard SQL now supports the following JSON functions:

- [`JSON_EXTRACT_STRING_ARRAY`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#json_extract_string_array)
- [`JSON_QUERY_ARRAY`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#json_query_array)
- [`JSON_VALUE_ARRAY`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#json_value_array)

These statements are [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).
Feature BigQuery standard SQL now supports the following statements for creating, configuring, and deleting datasets:

- [`CREATE SCHEMA`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_schema_statement)
- [`ALTER SCHEMA`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#alter_schema_set_options_statement)
- [`DROP SCHEMA`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#drop_schema_statement)

These statements are [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).
Feature BigQuery standard SQL now supports the `TABLESAMPLE` operator, which lets you query random subsets of data from large BigQuery tables. For more information, see [Table sampling](https://docs.cloud.google.com/bigquery/docs/table-sampling). This feature is in [Preview](https://cloud.google.com/products/#product-launch-stages).
Feature Support for the [BigNumeric](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#bignumeric_type) type in BigQuery standard SQL is now [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).

## March 24, 2021

Change BigQuery is now available in the [Warsaw (europe-central2) region](https://docs.cloud.google.com/bigquery/docs/locations#regional-locations).
Change BigQuery BI Engine is now available in the [Warsaw (europe-central2) region](https://docs.cloud.google.com/bi-engine/docs/locations#regional-locations).
Change BigQuery ML is now available in the [Warsaw (europe-central2) region](https://docs.cloud.google.com/bigquery-ml/docs/locations#regional-locations).
Change BigQuery Data Transfer Service is now available in the [Warsaw (europe-central2) region](https://docs.cloud.google.com/bigquery-transfer/docs/locations#regional-locations).

## March 11, 2021

Change BigQuery ML now supports training for DNN/Boosted Tree models in the [Iowa (us-central1) region](https://docs.cloud.google.com/bigquery-ml/docs/locations#regional-locations).

## March 02, 2021

Change Updated version of [Magnitude Simba ODBC](https://docs.cloud.google.com/bigquery/providers/simba-drivers) driver includes bug fixes, performance improvements, and enhancements such as support for dynamic SQL and additional DDL and DML keywords.
Change Updated version of [Magnitude Simba JDBC](https://docs.cloud.google.com/bigquery/providers/simba-drivers) driver includes bug fixes and performance improvements.

## February 25, 2021

Feature BigQuery materialized views are now [generally available](https://cloud.google.com/products/#product-launch-stages) (GA). BigQuery materialized views are now generally available (GA). Materialized views are precomputed views that periodically cache the results of a query, enhancing performance and efficiency, and reducing costs, particularly for aggregated queries. For more information, see [Introduction to materialized views](https://docs.cloud.google.com/bigquery/docs/materialized-views-intro).
Feature BigQuery BI Engine now interacts with popular BI tools such as Looker, Tableau, and more, by means of an [SQL interface](https://docs.cloud.google.com/bi-engine/docs/sql-interface-overview). You must [enroll](https://docs.cloud.google.com/bi-engine/docs/sql-interface-overview#requesting_access_to_the_preview) to participate in the [preview](https://cloud.google.com/products#product-launch-stages).

## February 24, 2021

Change The BigQuery Data Transfer Service's 1-hour minimum file age requirement for transfers from Cloud Storage has been [eliminated](https://docs.cloud.google.com/bigquery-transfer/docs/cloud-storage-transfer#minimum_intervals).

## February 23, 2021

Change The BigQuery Data Transfer Service's minimum interval time between recurring transfers from Cloud Storage has been reduced from one hour to [15 minutes](https://docs.cloud.google.com/bigquery-transfer/docs/cloud-storage-transfer#minimum_intervals).

## February 16, 2021

Feature BigQuery now supports exporting table data in Parquet format. This feature is in [Preview](https://cloud.google.com/products/#product-launch-stages). For more information, see [Parquet export details](https://docs.cloud.google.com/bigquery/docs/exporting-data#parquet_export_details).

## February 12, 2021

Change BigQuery standard SQL queries on Google Cloud Storage data are now supported by [cached query results](https://docs.cloud.google.com/bigquery/docs/cached-results#cache-exceptions).

## February 02, 2021

Change Updated version of [Magnitude Simba JDBC driver](https://docs.cloud.google.com/bigquery/providers/simba-drivers) includes bug and security fixes and enhancements for [additional DDL keywords](https://cloud.google.com/blog/topics/developers-practitioners/smile-new-user-friendly-sql-capabilities-bigquery) and [dynamic SQL](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/scripting#execute_immediate).

## January 29, 2021

Feature Clustered tables now support the `DATETIME` type for clustering columns. For more information, see [Creating and using clustered tables](https://docs.cloud.google.com/bigquery/docs/creating-clustered-tables). This feature is generally available.

## January 19, 2021

Change BigQuery BI Engine is now available in the [Iowa (us-central1) region](https://docs.cloud.google.com/bi-engine/docs/locations#regional-locations).
Change BigQuery ML is now available in the [Iowa (us-central1) region](https://docs.cloud.google.com/bigquery-ml/docs/locations#regional-locations).
Change BigQuery is now available in the [Iowa (us-central1) region](https://docs.cloud.google.com/bigquery/docs/locations#regional-locations).
Change The BigQuery Data Transfer Service is now available in the [Iowa (us-central1) region](https://docs.cloud.google.com/bigquery-transfer/docs/locations#regional-locations).

## January 14, 2021

Change BigQuery's Cloud console UI has been updated with many usability improvements for analyzing data, including multi-tab navigation, a new resource panel, and a new SQL editor. These updates are in [Preview](https://cloud.google.com/products/#product-launch-stages). For more information, see [Using the Cloud console](https://docs.cloud.google.com/bigquery/docs/bigquery-web-ui).

## December 21, 2020

Feature BigQuery standard SQL now supports the [BigNumeric](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#bignumeric_type) data type for high-precision computations. The BigNumeric data type is in [Preview](https://cloud.google.com/products/#product-launch-stages).

## December 16, 2020

Feature BigQuery Data Transfer Service is now fully integrated with VPC Service Controls, and can be protected using a service perimeter. Please refer to [VPC-SC supported products page](https://docs.cloud.google.com/vpc-service-controls/docs/supported-products#table_bigquery_data_transfer) for more info.

## December 10, 2020

Change Updated version of [Magnitude Simba JDBC](https://docs.cloud.google.com/bigquery/providers/simba-drivers) driver includes bug and security fixes and enhancements such as support for Java 11, SSL trust store, BigNumeric data, and version-agnostic fully-qualified class names.
Change Updated version of [Magnitude Simba ODBC](https://docs.cloud.google.com/bigquery/providers/simba-drivers) driver includes bug fixes and enhancements such as support for BigNumeric data and improved driver logic.

## November 23, 2020

Feature BigQuery ML integration with [AI Platform](https://docs.cloud.google.com/ai-platform) for Deep Neural Network (DNN) models is now [generally available](https://cloud.google.com/products/#product-launch-stages) (GA). For more information, see [`CREATE MODEL` statement for Deep Neural Network (DNN) models](https://docs.cloud.google.com/bigquery-ml/docs/reference/standard-sql/bigqueryml-syntax-create-dnn-models).
Feature Exporting BigQuery ML models to Cloud Storage and using them for online prediction is now [generally available](https://cloud.google.com/products#product-launch-stages) (GA). For more information, see [Exporting models](https://docs.cloud.google.com/bigquery-ml/docs/exporting-models) and the [`EXPORT MODEL` statement](https://docs.cloud.google.com/bigquery-ml/docs/reference/standard-sql/bigqueryml-syntax-export-model).
Feature BigQuery ML integration with [AI Platform](https://docs.cloud.google.com/ai-platform) for Boosted Tree models is now [generally available](https://cloud.google.com/products/#product-launch-stages) (GA). For more information, see the following documentation:

- [`CREATE MODEL` statement for Boosted Tree models using XGBoost](https://docs.cloud.google.com/bigquery-ml/docs/reference/standard-sql/bigqueryml-syntax-create-boosted-tree)

- [`ML.FEATURE_IMPORTANCE` function](https://docs.cloud.google.com/bigquery-ml/docs/reference/standard-sql/bigqueryml-syntax-importance)

## November 17, 2020

Feature BigQuery column-level security is now [generally available](https://cloud.google.com/products#product-launch-stages). Policy tags can be replicated across locations. For more information, see [Introduction to BigQuery column-level security](https://docs.cloud.google.com/bigquery/docs/column-level-security-intro).

## October 26, 2020

Feature BigQuery standard SQL now supports the [SUBSTRING](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#substring) function. This function is [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).

## October 22, 2020

Feature The [`ST_GEOGFROMGEOJSON`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_geogfromgeojson) and [`ST_GEOGFROMTEXT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_geogfromtext) geographic functions support a new `make_valid` parameter. If set to `TRUE`, the function attempts to correct polygon issues when importing geography data.

The [`ST_GEOGFROMTEXT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_geogfromtext_signature2) function also supports a new `planar` parameter. If set to `TRUE`, the function treats imported WKT geometries as having planar edges.

These new function parameters are in [Beta](https://cloud.google.com/products/#product-launch-stages).
Change Updated version of [Magnitude Simba ODBC](https://docs.cloud.google.com/bigquery/providers/simba-drivers) driver includes improvements to performance, logging, OpenSSL support, and bug fixes.

## October 19, 2020

Change BigQuery Audit Logs stopped using the following checks for redacting resource names for cross-project access and caller identities: The `bigquery.jobs.create` permission check and the internal setting for a project domain. Please review the documentation at [Caller identities and resource names](https://docs.cloud.google.com/bigquery/docs/reference/auditlogs#ids).

## October 14, 2020

Feature Dynamic SQL is now [generally available](https://cloud.google.com/products/#product-launch-stages) (GA). Dynamic SQL lets you generate and execute SQL statements dynamically at runtime. For more information, see [EXECUTE IMMEDIATE](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/scripting#execute_immediate).
Feature BigQuery standard SQL now supports the following new functions. These functions are [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).

- [ASCII](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#ascii)
- [CHR](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#chr)
- [INITCAP](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#initcap)
- [INSTR](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#instr)
- [LAST_DAY](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#last_day)
- [LEFT](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#left)
- [OCTET_LENGTH](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#octet_length)
- [REGEXP_EXTRACT](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#regexp_extract) with 2 additional parameters (position and occurrence)
- [REGEXP_INSTR](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#regexp_instr)
- [REGEXP_SUBSTR](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#regexp_substr)
- [RIGHT](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#right)
- [SOUNDEX](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#soundex)
- [TRANSLATE](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#translate)
- [UNICODE](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#unicode)
Feature BigQuery now supports the following new statements. These statements are [generally available](https://cloud.google.com/products/?hl=EN#product-launch-stages) (GA).

- [`CREATE EXTERNAL TABLE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_external_table_statement)
- [`DROP EXTERNAL TABLE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#drop_external_table_statement)
- [`ALTER TABLE ADD COLUMN`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#alter_table_add_column_statement)
- [`EXPORT DATA`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/other-statements#export_data_statement)
- [`TRUNCATE TABLE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/dml-syntax#truncate_table_statement)
Feature BigQuery standard SQL now supports [DATE arithmetics operators](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/operators#date_arithmetics_operators).
Feature BigQuery now supports Unicode table names. For more information, see [Table naming](https://docs.cloud.google.com/bigquery/docs/tables#table_naming).
Feature Queries can now have [duplicate column names](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#duplicate_aliases).
Feature The following [`INFORMATION_SCHEMA`](https://docs.cloud.google.com/bigquery/docs/information-schema-intro) views are now [generally available](https://cloud.google.com/products/?hl=EN#product-launch-stages) (GA).

- [TABLES](https://docs.cloud.google.com/bigquery/docs/information-schema-tables#tables_view)
- [TABLE_OPTIONS](https://docs.cloud.google.com/bigquery/docs/information-schema-tables#table_options_view)
- [COLUMNS](https://docs.cloud.google.com/bigquery/docs/information-schema-tables#columns_view)
- [COLUMN_FIELD_PATHS](https://docs.cloud.google.com/bigquery/docs/information-schema-tables#column_field_paths_view)
- [VIEWS](https://docs.cloud.google.com/bigquery/docs/information-schema-views#the_views_view)
- [ROUTINES](https://docs.cloud.google.com/bigquery/docs/information-schema-routines#routines_view)
- [ROUTINE_OPTIONS](https://docs.cloud.google.com/bigquery/docs/information-schema-routines#routine_options_view)
- [PARAMETERS](https://docs.cloud.google.com/bigquery/docs/information-schema-routines#parameters_view)
- [SCHEMATA](https://docs.cloud.google.com/bigquery/docs/information-schema-datasets#schemata_view)
- [SCHEMATA_OPTIONS](https://docs.cloud.google.com/bigquery/docs/information-schema-datasets#schemata_options_view)

## October 06, 2020

Feature Authorized user-defined functions (UDFs) are now [generally available](https://cloud.google.com/products/?hl=EN#product-launch-stages) (GA). Authorized UDFs let you share query results without giving access to the underlying tables. For more information, see [Authorized UDFs](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/user-defined-functions#authorized_udfs).
Feature The Cloud console now lets you [opt in to search and autocomplete](https://bigquery/docs/bigquery-web-ui#overview) powered by Data Catalog. This feature is in [beta](https://products#product-launch-stages).

## October 01, 2020

Feature BigQuery table-level access control is now [generally available](https://cloud.google.com/products#product-launch-stages). For more information, see [Introduction to table access controls](https://docs.cloud.google.com/bigquery/docs/table-access-controls-intro) .

## September 29, 2020

Feature Time series models now let you change `DATA_FREQUENCY` from the default value (`AUTO_FREQUENCY`) when forecasting multiple time series using `TIME_SERIES_ID_COL`.

## September 21, 2020

Feature The following [time-unit partitioning](https://docs.cloud.google.com/bigquery/docs/creating-column-partitions#daily_partitioning_vs_hourly_partitioning) features are now [Generally Available](https://cloud.google.com/products/?hl=EN#product-launch-stages) (GA):

- Creating partitions using hourly, monthly, and yearly time-unit granularities.
- Using a `DATETIME` column as the partitioning column.

## September 18, 2020

Change Beginning in late 2020, all [BigQuery predefined roles with permission to create jobs](https://docs.cloud.google.com/bigquery/docs/access-control#bigquery), including queries, will also be able to get information about their own jobs and cancel their own jobs. This change does not include permission to see jobs created by other users.

## September 17, 2020

Change The BigQuery Data Transfer Service is now available in the following regions: [Los Angeles (us-west2), São Paulo (southamerica-east1), South Carolina (us-east1), Hong Kong (asia-east1) and Osaka (asia-northeast2)](https://docs.cloud.google.com/bigquery-transfer/docs/locations#regional-locations).

## September 11, 2020

Feature You can now use the `BQ.JOBS.CANCEL` system procedure to cancel a job. For more information, see [Canceling jobs](https://docs.cloud.google.com/bigquery/docs/managing-jobs#canceling_a_job).

## August 31, 2020

Change Updated version of [Magnitude Simba JDBC](https://docs.cloud.google.com/bigquery/providers/simba-drivers) driver includes bug fixes and enhancements such as automatically turning on the BigQuery Storage API for anonymous table reads (no charge for temporary table reads).

## August 27, 2020

Feature Time series model support is now [Generally Available](https://cloud.google.com/products/#product-launch-stages) (GA). This release includes a new training option: [AUTO_ARIMA_MAX_ORDER](https://docs.cloud.google.com/bigquery-ml/docs/reference/standard-sql/bigqueryml-syntax-create-time-series#auto_arima_max_order).

For more information about time series model support, see the following documentation:

- [`CREATE MODEL` statement for time series models](https://docs.cloud.google.com/bigquery-ml/docs/reference/standard-sql/bigqueryml-syntax-create-time-series)

- [`ML.FORECAST` function](https://docs.cloud.google.com/bigquery-ml/docs/reference/standard-sql/bigqueryml-syntax-forecast)

- [`ML.ARIMA_COEFFICIENTS` function](https://docs.cloud.google.com/bigquery-ml/docs/reference/standard-sql/bigqueryml-syntax-arima-coefficients)

- [Single time-series forecasting from Google Analytics data](https://docs.cloud.google.com/bigquery-ml/docs/arima-single-time-series-forecasting-tutorial)

- [Multiple time-series forecasting with a single query](https://docs.cloud.google.com/bigquery-ml/docs/arima-multiple-time-series-forecasting-tutorial)

## August 26, 2020

Feature `INFORMATION_SCHEMA` views for [BigQuery reservations](https://docs.cloud.google.com/bigquery/docs/information-schema-reservations) are now [Generally Available](https://cloud.google.com/products/?hl=EN#product-launch-stages) (GA).

## August 25, 2020

Feature `INFORMATION_SCHEMA` views for [jobs metadata by timeslice](https://docs.cloud.google.com/bigquery/docs/information-schema-jobs-timeline) are now available.

## August 24, 2020

Feature BigQuery support for using service account credentials with scheduled queries is [Generally Available](https://cloud.google.com/products/#product-launch-stages) (GA).

## August 19, 2020

Feature When using consecutive `ON`/`USING` clauses, parentheses are now optional and can be omitted. For example, you can use either of the following statements:

- `FROM A JOIN (B JOIN C ON B.x = C.y) USING (z)`
- `FROM A JOIN B JOIN C ON B.x = C.y USING (z)`

## August 18, 2020

Feature You can now explicitly assign projects to use on-demand pricing with Reservations. See [Assign a project to None](https://docs.cloud.google.com/bigquery/docs/reservations-tasks#assign-project-to-none).

## August 17, 2020

Feature Matrix Factorization model support is now [Generally Available](https://cloud.google.com/products/#product-launch-stages) (GA). For more information, see the following documentation:

- [`CREATE MODEL` statement for Matrix Factorization](https://docs.cloud.google.com/bigquery-ml/docs/reference/standard-sql/bigqueryml-syntax-create-matrix-factorization)

- [`ML.RECOMMEND` function](https://docs.cloud.google.com/bigquery-ml/docs/reference/standard-sql/bigqueryml-syntax-recommend)

- [Using BigQuery ML to predict movie recommendations](https://docs.cloud.google.com/bigquery-ml/docs/bigqueryml-mf-explicit-tutorial)

- [Using BigQuery ML to predict website content for visitors](https://docs.cloud.google.com/bigquery-ml/docs/bigqueryml-mf-implicit-tutorial)

## August 13, 2020

Change The [exports per day](https://docs.cloud.google.com/bigquery/quotas#export_jobs) (Extract Bytes) default quota has been raised from 10 TB to 50 TB per day.

## August 11, 2020

Change For [flat-rate pricing](https://cloud.google.com/bigquery/pricing#flat_rate_pricing), the minimum slot purchase is now 100 slots. Slots can be purchased in 100-slot increments.

## August 06, 2020

Change BigQuery ML is now available following regions: [Oregon (us-west1), Belgium (europe-west1), and Netherlands (europe-west4)](https://docs.cloud.google.com/bigquery-ml/docs/locations#regional-locations).
Change BigQuery BI Engine is now available following regions: [Oregon (us-west1), Belgium (europe-west1), and Netherlands (europe-west4)](https://docs.cloud.google.com/bi-engine/docs/locations#regional-locations).
Change BigQuery Data Transfer Service is now available following regions: [Oregon (us-west1), Belgium (europe-west1), and Netherlands (europe-west4)](https://docs.cloud.google.com/bigquery-transfer/docs/locations#regional-locations).
Change BigQuery is now available in the following regions: [Oregon (us-west1), Belgium (europe-west1), and Netherlands (europe-west4)](https://docs.cloud.google.com/bigquery/docs/locations#regional-locations).

## July 31, 2020

Change Updated version of [Magnitude Simba ODBC](https://docs.cloud.google.com/bigquery/providers/simba-drivers) driver includes performance improvements and bug fixes.

## July 27, 2020

Feature `INFORMATION_SCHEMA` views for [streaming metadata](https://docs.cloud.google.com/bigquery/docs/information-schema-streaming) are now in [alpha](https://cloud.google.com/products/?hl=EN#product-launch-stages). You can use these views to retrieve historical
and real-time information about streaming data into BigQuery.

## July 24, 2020

Change BigQuery Data Transfer Service is now available in the following regions: [Montréal (northamerica-northeast1), Frankfurt (europe-west3), Mumbai (asia-south1), and Seoul (asia-northeast3)](https://docs.cloud.google.com/bigquery-transfer/docs/locations#regional-locations).

## July 16, 2020

Feature BigQuery GIS now supports two new functions, `ST_CONVEXHULL` and `ST_DUMP`:

- `ST_CONVEXHULL` returns the smallest convex `GEOGRAPHY` that covers the input.
- `ST_DUMP` returns an `ARRAY` of simple `GEOGRAPHY`s where each element is a component of the input `GEOGRAPHY`.

For more information, see the [`ST_CONVEXHULL`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_convexhull) and [`ST_DUMP`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_dump) reference pages.

## July 15, 2020

Feature Data split and validation options are now available for [AutoML Table model training](https://docs.cloud.google.com/bigquery-ml/docs/reference/standard-sql/bigqueryml-syntax-create-automl).

## July 13, 2020

Feature The Standard SQL statement [`ASSERT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/debugging-statements) is now supported. You can use [`ASSERT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/debugging-statements) to validate that data matches specified expectations.

## July 06, 2020

Change Updated version of [Magnitude Simba ODBC](https://docs.cloud.google.com/bigquery/providers/simba-drivers) driver. This version includes some performance improvements and bug fixes, and it catches up with the JDBC driver by adding support for user defined functions and variable time zones using the connection string.

## July 01, 2020

Feature BigQuery ML now supports time series models as a [beta](https://cloud.google.com/products#product-launch-stages) release. For more information, see [CREATE MODEL statement for time series models](https://docs.cloud.google.com/bigquery-ml/docs/reference/standard-sql/bigqueryml-syntax-create-time-series).

## June 29, 2020

Feature [Flex slots](https://docs.cloud.google.com/bigquery/docs/reservations-concepts#commitment_plans) are now [generally available (GA)](https://cloud.google.com/products/?hl=EN#product-launch-stages).
Change The [BigQuery SLA](https://cloud.google.com/bigquery/sla) has been updated to \>= 99.99% Monthly Uptime Percentage for all users.

## June 26, 2020

Feature [Region qualified `INFORMATION_SCHEMA` views](https://docs.cloud.google.com/bigquery/docs/information-schema-intro#syntax) are now in [beta](https://cloud.google.com/products/#product-launch-stages).
Change Starting in mid-August, unqualified `INFORMATION_SCHEMA` queries for `SCHEMATA` and `SCHEMATA_OPTIONS` views will default to returning metadata from the `US` multi-region. For information about how to specify a region, see [region qualifier syntax](https://docs.cloud.google.com/bigquery/docs/information-schema-intro#region_qualifier).

## June 16, 2020

Feature The [Top Brands report](https://docs.cloud.google.com/bigquery-transfer/docs/merchant-center-top-brands-schema) for Google Merchant Center Best Sellers exports is now in [beta](https://cloud.google.com/products/#product-launch-stages).
Feature [`INFORMATION_SCHEMA` views for jobs](https://docs.cloud.google.com/bigquery/docs/information-schema-jobs) are now [generally available (GA)](https://cloud.google.com/products/?hl=EN#product-launch-stages).
Feature BigQuery ML now supports [preview](https://cloud.google.com/products#product-launch-stages) integration with [AI Platform](https://docs.cloud.google.com/ai-platform). The following models are supported in [preview](https://cloud.google.com/products#product-launch-stages):

- AutoML Tables models. For more information, see [CREATE MODEL statement for AutoML Tables models](https://docs.cloud.google.com/bigquery-ml/docs/reference/standard-sql/bigqueryml-syntax-create-automl).

- Boosted Tree models using XGBoost. For more information, see [CREATE MODEL statement for Boosted Tree models](https://docs.cloud.google.com/bigquery-ml/docs/reference/standard-sql/bigqueryml-syntax-create-boosted-tree).

- Deep Neural Network (DNN) models. For more information, see [CREATE MODEL statement for DNN models](https://docs.cloud.google.com/bigquery-ml/docs/reference/standard-sql/bigqueryml-syntax-create-dnn-models).

## June 09, 2020

Feature Clustering for non-partitioned tables is now supported. For more information about clustered tables, see [Introduction to clustered tables](https://docs.cloud.google.com/bigquery/docs/clustered-tables).

## June 08, 2020

Change BigQuery Data Transfer Service is now available in the [Jakarta (asia-southeast2) region](https://docs.cloud.google.com/bigquery-transfer/docs/locations#supported_regions).
Change BigQuery ML is now available in the [Jakarta (asia-southeast2) region](https://docs.cloud.google.com/bigquery-ml/docs/locations#supported_regions).
Change BigQuery is now available in the [Jakarta (asia-southeast2) region](https://docs.cloud.google.com/bigquery/docs/locations#supported_regions).
Change BigQuery BI Engine is now available in the [Jakarta (asia-southeast2) region](https://docs.cloud.google.com/bi-engine/docs/locations#supported_regions).

## June 04, 2020

Feature BigQuery Table ACL is now available as a [beta](https://cloud.google.com/products#product-launch-stages) release. For more information, see [Introduction to table access controls](https://docs.cloud.google.com/bigquery/docs/table-access-controls-intro) .

## June 02, 2020

Feature You can now [purchase BigQuery slots](https://cloud.google.com/bigquery/docs/reservations-workload-management#getting_started_with_reservations) using the `bq` command-line tool. BigQuery Reservations allows you to purchase slots to take advantage of BigQuery [flat-rate pricing](https://cloud.google.com/bigquery/pricing#flat_rate_pricing) and allocate slots for workload management.
Feature A new GIS function, [`ST_Simplify`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_simplify), is available. [`ST_Simplify`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_simplify) returns a simplified version of the input `GEOGRAPHY` by replacing sections with straight lines.
Change Standard SQL [user-defined function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/user-defined-functions) definition bodies can now contain references to tables and views.
Change Standard SQL view definition bodies can now contain references without project qualifiers, as long as the view is created by the [`tables.insert`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables/insert) REST API or is in the same project used to run the [`CREATE VIEW`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_view_statement) DDL query.

## May 21, 2020

Change The BigQuery Storage API now supports reading small anonymous (cached) tables without any limitations.

## May 20, 2020

Feature Happy 10th birthday, BigQuery!
Feature [Cloud SQL federated queries](https://cloud.google.com/bigquery/docs/cloud-sql-federated-queries) are now generally available [(GA)](https://docs.cloud.google.com/terms/launch-stages).
Feature [BigQuery Trial slots](https://cloud.google.com/bigquery/pricing#flat_rate_pricing) are now available in US and EU multi-regions. Trial slots are a limited promotion for qualified customers.
Feature [Hourly partitioned tables](https://docs.cloud.google.com/bigquery/docs/creating-column-partitions) are now in [beta](https://cloud.google.com/products/#product-launch-stages).
Feature Dynamic SQL is now available as a [beta](https://cloud.google.com/products/#product-launch-stages) release in all BigQuery regions. Dynamic SQL lets you generate and execute SQL statements dynamically at runtime. For more information, see [EXECUTE IMMEDIATE](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/scripting#execute_immediate).

## May 13, 2020

Change Updated versions of [Magnitude Simba JDBC](https://docs.cloud.google.com/bigquery/providers/simba-drivers) drivers have been released.

## May 12, 2020

Change Updated versions of [Magnitude Simba ODBC](https://docs.cloud.google.com/bigquery/providers/simba-drivers) drivers have been released.

## May 08, 2020

Feature [Next generation BigQuery streaming](https://docs.cloud.google.com/bigquery/quotas#streaming_inserts) is now [Generally Available (GA)](https://cloud.google.com/products/?hl=EN#product-launch-stages).

## May 01, 2020

Feature A new function, `JSON_EXTRACT_ARRAY`, has been added to the list of JSON functions. This function allows you to extract the contents of a JSON document as a string array. For more information, see the [`JSON_EXTRACT_ARRAY` reference section](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#json_extract_array).
Feature The `ORDER BY` clause now supports the `NULLS FIRST` and `NULLS LAST` clauses. These clauses allow you to specify the sort order of null and non-null values. For more information, see the [`ORDER BY` reference section](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#order_by_clause).

## April 30, 2020

Change The BigQuery Data Transfer Service is now available in the [Taiwan (asia-east1) region](https://docs.cloud.google.com/bigquery-transfer/docs/locations#supported_regions).
Change BigQuery Data Transfer Service is now available in the [Taiwan (asia-east1) region](https://docs.cloud.google.com/bigquery-transfer/docs/locations#supported_regions).

## April 27, 2020

Change BigQuery is now available in the [Las Vegas (us-west4) region](https://docs.cloud.google.com/bigquery/docs/locations#supported_regions).
Change BigQuery ML is now available in the [Las Vegas (us-west4) region](https://docs.cloud.google.com/bigquery-ml/docs/locations#supported_regions).
Change BigQuery Data Transfer Service is now available in the [Las Vegas (us-west4) region](https://docs.cloud.google.com/bigquery-transfer/docs/locations#supported_regions).
Change BigQuery BI Engine is now available in the [Las Vegas (us-west4) region](https://docs.cloud.google.com/bi-engine/docs/locations#supported_regions).

## April 22, 2020

Feature BigQuery ML now supports exporting BigQuery ML models to Cloud Storage and using them for online prediction. This feature is in [beta](https://cloud.google.com/products#product-launch-stages). For more information, see [Exporting models](https://docs.cloud.google.com/bigquery-ml/docs/exporting-models).

## April 17, 2020

Feature BigQuery ML now supports Matrix Factorization models for recommendations, as a [beta](https://cloud.google.com/products#product-launch-stages) release. For more information, see [The CREATE MODEL statement for Matrix Factorization](https://docs.cloud.google.com/bigquery-ml/docs/reference/standard-sql/bigqueryml-syntax-create-matrix-factorization).

## April 16, 2020

Change Around the end of April 2020, [`INFORMATION_SCHEMA` (Beta) views for dataset metadata](https://docs.cloud.google.com/bigquery/docs/information-schema-datasets) will
return metadata about all datasets in a region. Currently,
these views return metadata about all datasets in the project across all regions. This
upcoming change will also provide support for querying a specific
region's metadata (for example, `region-us.INFORMATION_SCHEMA.SCHEMATA` instead of `INFORMATION_SCHEMA.SCHEMATA`).

You can replicate this future behavior now by filtering
on the `SCHEMATA` view's `LOCATION` column (for example, `LOCATION = 'US'`).
Feature [BigQuery Reservations](https://cloud.google.com/bigquery/docs/reservations-intro) is now [Generally Available (GA)](https://cloud.google.com/products/?hl=EN#product-launch-stages). BigQuery Reservations allows you to purchase BigQuery [slots](https://docs.cloud.google.com/bigquery/docs/slots) to take advantage of BigQuery [flat-rate pricing](https://cloud.google.com/bigquery/pricing#flat_rate_pricing) and allocate slots for workload management.

## April 14, 2020

Feature BigQuery Data Transfer Service now supports [Google Merchant Center data transfers for best sellers data](https://docs.cloud.google.com/bigquery/docs/merchant-center-transfer#supported_reports).

## April 09, 2020

Feature Scheduling queries no longer requires the `bigquery.transfers.update` permission. The `bigquery.jobs.create` permission can now be used to schedule queries. See [Scheduling queries](https://docs.cloud.google.com/bigquery/docs/scheduling-queries#required_permissions) for details.

## April 08, 2020

Feature BigQuery materialized views are now available as a [beta](https://cloud.google.com/products#product-launch-stages) release. For more information, see [Introduction to materialized views](https://docs.cloud.google.com/bigquery/docs/materialized-views-intro) .

## April 02, 2020

Feature [BigQuery Reservations](https://docs.cloud.google.com/bigquery/docs/reservations-intro) is now available in all [BigQuery regions](https://docs.cloud.google.com/bigquery/docs/locations).

## March 31, 2020

Feature `INFORMATION_SCHEMA` views for [BigQuery reservations](https://docs.cloud.google.com/bigquery/docs/information-schema-reservations) are now in public [alpha](https://cloud.google.com/products/?hl=EN#product-launch-stages).

## March 30, 2020

Feature [Scripting](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/scripting) and [stored procedures](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_procedure) are now [Generally Available](https://cloud.google.com/products/#product-launch-stages).

## March 27, 2020

Feature BigQuery Column-level security is now available as a [beta](https://cloud.google.com/products#product-launch-stages) release. For more information, see [Introduction to BigQuery Column-level security](https://docs.cloud.google.com/bigquery/docs/column-level-security-intro).

## March 24, 2020

Change BigQuery Data Transfer Service is now available in the [Northern Virginia (us-east4) region and the Salt Lake City (us-west3) region](https://docs.cloud.google.com/bigquery-transfer/docs/locations#locations).

## March 23, 2020

Feature BigQuery Data Transfer Service now supports [Google Merchant Center data transfers for pricing competitiveness](https://docs.cloud.google.com/bigquery/docs/merchant-center-transfer#supported_reports).

## March 13, 2020

Change BigQuery Data Transfer Service now supports the [Finland region](https://docs.cloud.google.com/bigquery-transfer/docs/locations#locations).

## March 11, 2020

Change BigQuery Data Transfer Service now supports the [Zürich region](https://docs.cloud.google.com/bigquery-transfer/docs/locations#locations).

## March 02, 2020

Feature The BigQuery Storage API is now [generally available](https://cloud.google.com/terms/launch-stages). For more information, see [the Storage API overview](https://docs.cloud.google.com/bigquery/docs/reference/storage).
Feature Support for [loading](https://cloud.google.com/bigquery/docs/hive-partitioned-loads-gcs) and [querying](https://docs.cloud.google.com/bigquery/docs/hive-partitioned-queries-gcs) externally partitioned data that is stored on Cloud Storage using a default hive partitioning layout is now [Generally Available](https://docs.cloud.google.com/terms/launch-stages). The data must be in Avro, Parquet, ORC, CSV, or JSON format.
Feature Support for [querying Cloud Storage data](https://cloud.google.com/bigquery/external-data-cloud-storage) in ORC and Parquet format is now [Generally Available](https://docs.cloud.google.com/terms/launch-stages).
Change Personally identifiable information (PII) in audit log records for successful reads of resources with restricted access is no longer redacted. For more information, see [Caller identities and resource names](https://docs.cloud.google.com/bigquery/docs/reference/auditlogs#ids).

## February 27, 2020

Feature [Cloud Storage transfers to BigQuery](https://docs.cloud.google.com/bigquery-transfer/docs/cloud-storage-transfer) now have the option of appending data or refreshing (overwriting) data.

## February 26, 2020

Change BigQuery DML statements no longer have quota limits.

> [!NOTE]
> **Note:** DML statements are counted toward the maximum number of table operations per day and partition modifications per day. However, DML statements will not fail due to these limits. (Updated April 6, 2021)

Feature BigQuery [Flex slots](https://cloud.google.com/bigquery/pricing#flex-slots-pricing) are now available in beta in all BigQuery [Reservations](https://docs.cloud.google.com/bigquery/docs/reservations-intro) regions. See [Workload management](https://docs.cloud.google.com/bigquery/docs/reservations-workload-management#choosing_between_on-demand_and_flat-rate_billing_models) and [Commitment plans](https://docs.cloud.google.com/bigquery/docs/reservations-concepts#commitment_plans) for more information.

## February 25, 2020

Feature Support for data transfers from [Amazon S3](https://cloud.google.com/bigquery/docs/s3-transfer-intro) to BigQuery is now [generally available](https://docs.cloud.google.com/terms/launch-stages).

## February 24, 2020

Change BigQuery ML is now available in the [Salt Lake City (us-west3) region](https://docs.cloud.google.com/bigquery/docs/locations#regional-locations).
Change BI Engine is now available in the [Salt Lake City (us-west3) region](https://docs.cloud.google.com/bi-engine/docs/overview#regional_locations).
Change BigQuery is now available in the [Salt Lake City (us-west3) region](https://docs.cloud.google.com/bigquery/docs/locations#regional_locations).

## February 20, 2020

Feature Support for [Amazon Redshift data warehouse migrations](https://docs.cloud.google.com/bigquery-transfer/docs/redshift-migration) to BigQuery is now [generally available](https://cloud.google.com/products/#product-launch-stages).

Support for [migrating Amazon Redshift private instances](https://docs.cloud.google.com/bigquery-transfer/docs/redshift-vpc-migration) to BigQuery with VPC is also now [generally available](https://cloud.google.com/products/#product-launch-stages).
Change [BigQuery BI Engine](https://cloud.google.com/bi-engine/docs) is now [GA](https://docs.cloud.google.com/terms/launch-stages).
Change Increased reservation size to 100 GB.
Feature Added support for custom queries and views from Data Studio.

## February 18, 2020

Feature Support for tables partitioned on an integer column is now [GA](https://cloud.google.com/products/#product-launch-stages). For more information, see [Creating and using integer range partitioned tables](https://docs.cloud.google.com/bigquery/docs/creating-integer-range-partitions).

## February 14, 2020

Feature `INFORMATION_SCHEMA` views for [live and historical jobs metadata](https://docs.cloud.google.com/bigquery/docs/information-schema-jobs) are now in [beta](https://cloud.google.com/products/?hl=EN#product-launch-stages).

## February 13, 2020

Change The [BigQuery sandbox](https://cloud.google.com/bigquery/docs/sandbox) is now [GA](https://docs.cloud.google.com/terms/launch-stages).

## February 11, 2020

Change Updated versions of [Magnitude Simba ODBC](https://docs.cloud.google.com/bigquery/providers/simba-drivers) drivers are now available.

## January 24, 2020

Change BigQuery ML is now available in the [Seoul (asia-northeast3) region](https://docs.cloud.google.com/bigquery/docs/locations#regional-locations).
Change BigQuery is now available in the [Seoul (asia-northeast3) region](https://docs.cloud.google.com/bigquery/docs/locations#regional_locations).

## January 22, 2020

Change The BigQuery Storage API is now available in all BigQuery regional
and multi-regional locations. For more information, see
[Dataset locations](https://docs.cloud.google.com/bigquery/docs/locations).

## January 16, 2020

Change Updated versions of [Magnitude Simba JDBC and ODBC](https://docs.cloud.google.com/bigquery/providers/simba-drivers) drivers have been released.

## January 14, 2020

Feature [BigQuery Reservations](https://docs.cloud.google.com/bigquery/docs/reservations-intro) is now available in the London (europe-west2) region.
Change There is no longer a charge to use Google's ads management connectors and YouTube connectors for the [BigQuery Data Transfer Service](https://cloud.google.com/bigquery-transfer/pricing). Normal [BigQuery analysis and storage pricing](https://cloud.google.com/bigquery/pricing) will continue to apply after any transfers.

## December 20, 2019

Feature The `ALTER TABLE` and `ALTER VIEW` Data Definition Language statements are now [Generally Available](https://cloud.google.com/products/#product-launch-stages) (GA). These statements let you set options on [tables](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#alter_table_set_options_statement) and [views](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#alter_view_set_options_statement).
Feature [BigQuery Reservations](https://docs.cloud.google.com/bigquery/docs/reservations-intro) is now available in the Tokyo (`asia-northeast1`) region.

## December 19, 2019

Feature Support for transferring [Google Play reports](https://cloud.google.com/bigquery/docs/play-transfer) is now generally available [GA](https://docs.cloud.google.com/terms/launch-stages).
Feature [Third party transfers](https://cloud.google.com/bigquery/docs/third-party-transfer) for loading data from external data sources are now generally available [GA](https://docs.cloud.google.com/terms/launch-stages).
Feature BigQuery ML data preprocessing is now [Generally Available](https://cloud.google.com/products/#product-launch-stages) (GA). Read about the [preprocessing functions](https://docs.cloud.google.com/bigquery-ml/docs/reference/standard-sql/bigqueryml-preprocessing-functions) and walk through [how to use the TRANSFORM clause for feature engineering](https://docs.cloud.google.com/bigquery-ml/docs/bigqueryml-transform).

## December 13, 2019

Feature [BigQuery Reservations](https://cloud.google.com/bigquery/docs/reservations-intro) is now in beta in US and EU regions. BigQuery Reservations allows you to purchase BigQuery [slots](https://docs.cloud.google.com/bigquery/docs/slots) to take advantage of BigQuery [flat-rate pricing](https://cloud.google.com/bigquery/pricing#flat_rate_pricing) and allocate slots for workload management.

## December 11, 2019

Feature Support for tables partitioned on an integer column is now in [Beta](https://cloud.google.com/products/#product-launch-stages). For more information, see [Creating and using integer range partitioned tables](https://docs.cloud.google.com/bigquery/docs/creating-integer-range-partitions).

## December 10, 2019

Change For users billed on the [on-demand](https://cloud.google.com/bigquery/pricing#on_demand_pricing) pricing model, BigQuery can now burst beyond the default slot capacity to accelerate queries. There is no extra charge associated with query acceleration.

## December 09, 2019

Feature Support for [Cloud Storage Transfers](https://cloud.google.com/bigquery-transfer/docs/cloud-storage-transfer-overview) is now generally available [GA](https://docs.cloud.google.com/terms/launch-stages).

## December 04, 2019

Feature You can now [use KMEANS++](https://docs.cloud.google.com/bigquery-ml/docs/reference/standard-sql/bigqueryml-syntax-create#kmeans_init_method) to initialize the clusters of a k-means model. KMEANS++ trains a better model than random cluster initialization.

## November 26, 2019

Change [`INFORMATION_SCHEMA`](https://docs.cloud.google.com/bigquery/docs/information-schema-intro) now has updated permissions checks. `INFORMATION_SCHEMA` is currently in [beta](https://cloud.google.com/products/?hl=EN#product-launch-stages).

Previously, queries against `INFORMATION_SCHEMA` views filtered the results based on the IAM permissions needed to get metadata about the resource. Now, queries return accessDenied (403) errors if users have insufficient IAM permissions to get metadata.

## November 22, 2019

Feature Support for [persistent standard SQL UDFs](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/user-defined-functions) is now [Generally Available](https://cloud.google.com/products/#product-launch-stages) (GA).
Feature You can now use `INFORMATION_SCHEMA` views to [get routine metadata](https://docs.cloud.google.com/bigquery/docs/information-schema-routines) in BigQuery. `INFORMATION_SCHEMA` is currently in [beta](https://cloud.google.com/products/?hl=EN#product-launch-stages).

## November 21, 2019

Feature BigQuery ML [data preprocessing](https://docs.cloud.google.com/bigquery-ml/docs/reference/standard-sql/bigqueryml-preprocessing-functions) is now in beta.
Feature BigQuery now supports using service account credentials with scheduled queries. This feature is in [Beta](https://cloud.google.com/products/#product-launch-stages).
Feature BigQuery ML now supports customer-managed encryption keys (CMEK). You can [use your own Cloud KMS keys](https://docs.cloud.google.com/bigquery-ml/docs/customer-managed-encryption-key) to encrypt ML models.

## November 20, 2019

Change BigQuery ML is now available in the South Carolina (us-east1) [region](https://cloud.google.com/bigquery-ml/pricing#bqml).
Change BigQuery is now available in the South Carolina (us-east1) [region](https://docs.cloud.google.com/bigquery/docs/locations#regional_locations).

## November 12, 2019

Feature Support for omitting column names in [INSERT](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/dml-syntax#insert_statement) and [MERGE](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/dml-syntax#merge_statement) statements is now [Generally Available](https://cloud.google.com/products/#product-launch-stages) (GA).

## October 31, 2019

Feature Support for [querying Cloud Storage data](https://cloud.google.com/bigquery/external-data-cloud-storage) in ORC and Parquet format is now in [beta](https://docs.cloud.google.com/terms/launch-stages).
Feature Support for [loading](https://cloud.google.com/bigquery/docs/hive-partitioned-loads-gcs) and [querying](https://docs.cloud.google.com/bigquery/docs/hive-partitioned-queries-gcs) externally partitioned data that is stored on Cloud Storage using a default hive partitioning layout is now in [beta](https://docs.cloud.google.com/terms/launch-stages). The data must be in Avro, Parquet, ORC, CSV, or JSON format.

## October 23, 2019

Change The BigQuery service is being renamed from:

`bigquery-json.googleapis.com`

to

`bigquery.googleapis.com`.

`bigquery-json.googleapis.com` will become an alias of `bigquery.googleapis.com going` forward.

This change is largely transparent to users and does not affect runtime traffic or management operations. Risks and mitigation are discussed below.

Calls to the BigQuery API will not be affected.

### Timing

The change is a two phase process that we plan to release during November and December 2019.

### Impact when using Service Management and Service Usage APIs

Responses from the [Service Management API](https://docs.cloud.google.com/service-infrastructure/docs/overview) and the [Service Usage API](https://docs.cloud.google.com/service-usage/docs/overview) for information about the BigQuery service will contain information about `bigquery.googleapis.com`, even when you request data for `bigquery-json.googleapis.com`. Other than the name, the responses from the Service Management API and the Service Usage API will be the same. For example, listing enabled services will now return `bigquery.googleapis.com`, instead of `bigquery-json.googleapis.com`.

### Risks and Mitigation

- During the week of the second phase, alerts on [Stackdriver Transparent Service Level Indicators](https://docs.cloud.google.com/transparent-sli) (SLIs) for the `bigquery-json.googleapis.com` will break. Users should add a duplicate alert for `bigquery.googleapis.com`. The following are the [serviceruntime](https://docs.cloud.google.com/monitoring/api/metrics_gcp#gcp-serviceruntime) SLIs where you now need to use `bigquery.googleapis.com`:

  - `serviceruntime.googleapis.com/api/request_count`
  - `serviceruntime.googleapis.com/api/request_latencies`
  - `serviceruntime.googleapis.com/api/request_latencies_backend`
  - `serviceruntime.googleapis.com/api/request_latencies_overhead`
  - `serviceruntime.googleapis.com/api/request_sizes`
  - `serviceruntime.googleapis.com/api/response_sizes`
- The Google provider for Terraform has been updated to use the new name, `bigquery.googleapis.com`. Terraform users should upgrade to a Google provider with version \>= 2.17.0 before November 4, 2019.

## October 09, 2019

Feature The [`RANGE_BUCKET`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#range_bucket) mathematical function has been added. This function scans through a sorted array and returns the 0-based position of a given point's upper bound. This function can be useful if you need to group your data to build partitions, histograms, business-defined rules, and more.

## October 04, 2019

Change Updated versions of [Magnitude Simba JDBC and ODBC](https://docs.cloud.google.com/bigquery/providers/simba-drivers) drivers have been released.

## October 03, 2019

Feature BigQuery Data Transfer Service now supports [Search Ads 360 data transfers](https://docs.cloud.google.com/bigquery-transfer/docs/sa360-transfer). Search Ad 360 transfers are in [beta](https://cloud.google.com/products/#product-launch-stages).
Feature The ability to use [scripting](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/scripting), [stored procedures](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_procedure), and [temporary tables](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#temporary_tables) is now in [Beta](https://cloud.google.com/products/#product-launch-stages).

## September 30, 2019

Feature [Importing TensorFlow](https://cloud.google.com/bigquery-ml/docs/reference/standard-sql/bigqueryml-syntax-create-tensorflow) models is now [GA](https://docs.cloud.google.com/terms/launch-stages).

## September 27, 2019

Change
- The maximum size of a BI Engine reservation is now 50 GB per project per location. For more information, see [Capacity limits](https://docs.cloud.google.com/bigquery/quotas#capacity_limits).
- We added more options for BI Engine capacity that is bundled with flat-rate pricing. For more information, see [Flat-rate pricing](https://cloud.google.com/bi-engine/pricing#flat_rate_pricing).

## September 26, 2019

Change BigQuery Data Transfer Service support for data \& schema migration from a Teradata data warehouse to BigQuery has been upgraded. On-demand, one-time transfers are now generally available ([GA](https://cloud.google.com/products/#product-launch-stages)). Incremental, recurring transfers are in [Beta](https://cloud.google.com/products/#product-launch-stages).

## September 23, 2019

Feature Support for k-means clustering models is now [GA](https://cloud.google.com/terms/launch-stages). For more information, see [Creating a k-means clustering model](https://docs.cloud.google.com/bigquery-ml/docs/kmeans-tutorial).

## September 17, 2019

Feature BigQuery BI Engine is now available in the Frankfurt (europe-west3) [region](https://docs.cloud.google.com/bigquery/docs/locations#regional_locations).
Feature BigQuery is now available in the Frankfurt (europe-west3) [region](https://docs.cloud.google.com/bigquery/docs/locations#regional_locations).
Feature BigQuery ML is now available in the Frankfurt (europe-west3) [region](https://docs.cloud.google.com/bigquery/docs/locations#regional_locations).

## September 11, 2019

Change Billing is now enabled for BigQuery BI Engine. You are charged according to the information on the [Pricing](https://cloud.google.com/bi-engine/pricing) page.

## August 30, 2019

Feature Support for [copying datasets](https://docs.cloud.google.com/bigquery/docs/copying-datasets), including across regions, is now in [Beta](https://cloud.google.com/products/#product-launch-stages).

## August 27, 2019

Feature When you create an [external table](https://docs.cloud.google.com/bigquery/external-data-drive#creating_and_querying_a_permanent_external_table) that is linked to a Google Sheets file stored in Google Drive, you can now specify an individual sheet or a cell range. You are no longer limited to the first sheet only.

## August 22, 2019

Feature You no longer have to manually re-cluster data in clustered tables. BigQuery now performs [automatic re-clustering](https://docs.cloud.google.com/bigquery/docs/clustered-tables#automatic_re-clustering) on all clustered tables.
Change Updated version of [Magnitude Simba ODBC driver](https://docs.cloud.google.com/bigquery/providers/simba-drivers).

## August 19, 2019

Change If you stream data into BigQuery without populating the `insertId` field, you get the following higher quotas in the `US` multi-region location. These higher quotas are currently in [beta](https://cloud.google.com/terms/launch-stages).

- Maximum rows per second: 1,000,000
- Maximum bytes per second: 1 GB

For more information, see the [Quotas and limits](https://docs.cloud.google.com/bigquery/quotas#streaming_inserts) page.

## August 07, 2019

Feature You can now query data in BigQuery and Cloud SQL with a federated query. [Cloud SQL federated queries](https://cloud.google.com/bigquery/docs/cloud-sql-federated-queries) are now in [beta](https://docs.cloud.google.com/terms/launch-stages).

## August 05, 2019

Feature The BigQuery Storage API now supports reading data from BigQuery Storage in [ARROW](https://cloud.google.com/bigquery/docs/reference/storage/#arrow_schema_details) format. The [BigQuery Storage API](https://docs.cloud.google.com/bigquery/docs/reference/storage) is currently in [beta](https://docs.cloud.google.com/terms/launch-stages).

## July 25, 2019

Change Updated version of [Magnitude Simba ODBC driver](https://docs.cloud.google.com/bigquery/providers/simba-drivers). This version adds support for the BigQuery Storage API. The structure of the Linux ODBC tar package has also been changed to contain both 32-bit and 64-bit driver versions.

## July 19, 2019

Feature The [scheduled query](https://docs.cloud.google.com/bigquery/docs/scheduling-queries) feature is now [Generally Available](https://cloud.google.com/products/#product-launch-stages).

## July 02, 2019

Feature [Importing TensorFlow](https://cloud.google.com/bigquery-ml/docs/reference/standard-sql/bigqueryml-syntax-create-tensorflow) models is now [Beta](https://docs.cloud.google.com/terms/launch-stages).

## July 01, 2019

Feature Support for [persistent user-defined functions](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_function_statement) (UDFs) is now in [Beta](https://cloud.google.com/products/#product-launch-stages).
Feature BigQuery Data Transfer Service now supports [Google Merchant Center](https://docs.cloud.google.com/bigquery/docs/merchant-center-transfer) data transfers. Merchant Center transfers are in the [beta](https://cloud.google.com/products/#product-launch-stages) stage.

## June 17, 2019

Feature Support for [Amazon Redshift data warehouse migrations](https://docs.cloud.google.com/bigquery/docs/redshift-migration) to BigQuery is now in [Beta](https://cloud.google.com/products/#product-launch-stages).

## June 13, 2019

Change Updated version of [Magnitude Simba JDBC driver](https://docs.cloud.google.com/bigquery/providers/simba-drivers).

## June 10, 2019

Change BigQuery is moving to the [Cloud console](https://docs.cloud.google.com/bigquery/docs/bigquery-web-ui). The classic web UI remains available through June 2020. For information on migrating to the Cloud console, see [Moving to the Cloud console](https://docs.cloud.google.com/bigquery/docs/bigquery-classic-ui#migration).

## June 03, 2019

Feature Support for omitting column names in [`INSERT` and `MERGE` statements](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/dml-syntax) is now in [Beta](https://cloud.google.com/products/#product-launch-stages).

## May 29, 2019

Feature [BigQuery ML](https://cloud.google.com/bigquery/docs/bqml-introduction) is now [Generally Available](https://docs.cloud.google.com/terms/launch-stages) (GA).

## May 28, 2019

Change The [maximum response size](https://docs.cloud.google.com/bigquery/quotas#query_jobs) for query results has increased from 128 MB to 10 GB.

## May 15, 2019

Feature Support for clustering a table by using a `GEOGRAPHY` column is now [Generally Available](https://cloud.google.com/products/#product-launch-stages) (GA).

## May 14, 2019

Feature BigQuery ML now supports the [DROP MODEL](https://docs.cloud.google.com/bigquery-ml/docs/reference/standard-sql/bigqueryml-syntax-drop-model) DDL statement for deleting models.

## May 13, 2019

Change The concurrent rate limit for on-demand, interactive queries has increased from 50 to 100 concurrent queries.

## May 06, 2019

Feature [BigQuery ML IAM permissions](https://docs.cloud.google.com/bigquery-ml/docs/reference/access-control) are now available. These permissions take effect on June 6, 2019. Customers with custom roles should migrate to these permissions no later than June 6. Pre-defined IAM roles and basic roles are not impacted by this change.

## April 29, 2019

Feature BigQuery BI Engine is now available in several additional regions:

- Los Angeles
- Montréal
- Northern Virginia
- São Paulo
- Zürich
- Hong Kong
- Mumbai
- Taiwan
- Singapore
- Sydney

For a complete list of BigQuery BI Engine locations, see [Supported regions](https://docs.cloud.google.com/bi-engine/docs/overview#supported_regions)

## April 22, 2019

Change The following quotas have changed:

- The number of [copy jobs](https://docs.cloud.google.com/bigquery/quotas#copy_jobs) per project per day has increased from 10,000 to 100,000.
- The number of [load jobs](https://docs.cloud.google.com/bigquery/quotas#load_jobs) per project per day has increased from 50,000 to 100,000.
- The number of [export jobs](https://docs.cloud.google.com/bigquery/quotas#export_jobs) per project per day has increased from 50,000 to 100,000.

## April 18, 2019

Feature BigQuery is now available in the Osaka (asia-northeast2) [region](https://docs.cloud.google.com/bigquery/docs/locations#regional_locations). For upcoming regional launches, see the [Google Cloud blog](https://cloud.google.com/blog/products/data-analytics/bigquery-arrives-in-the-london-region-with-more-regions-to-come).

## April 10, 2019

Feature BigQuery standard SQL now supports [AEAD encryption functions](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aead_encryption_functions).
Feature BigQuery ML now supports the [k-means](https://docs.cloud.google.com/bigquery-ml/docs/reference/standard-sql/bigqueryml-syntax-create#model_option_list) model type for clustering and customer segmentation.
Feature The [BigQuery web UI in the Cloud console](https://docs.cloud.google.com/bigquery/docs/bigquery-web-ui) is now [generally available (GA)](https://cloud.google.com/products/#product-launch-stages).
Feature The BigQuery Data Transfer Service now supports transferring data into BigQuery from [Amazon S3](https://cloud.google.com/bigquery/docs/s3-transfer-intro). Amazon S3 transfers are now in [beta](https://docs.cloud.google.com/terms/launch-stages).
Feature [Third party transfers](https://cloud.google.com/bigquery/docs/third-party-transfer) for loading data from external data sources are now in [beta](https://docs.cloud.google.com/terms/launch-stages).
Feature [BigQuery BI Engine](https://docs.cloud.google.com/bi-engine/docs) is now in [beta](https://cloud.google.com/products/#product-launch-stages).
Feature Support for [clustered tables](https://docs.cloud.google.com/bigquery/docs/clustered-tables) is now [generally available (GA)](https://cloud.google.com/products/#product-launch-stages).

## April 05, 2019

Feature The [BigQuery Data Transfer Service](https://docs.cloud.google.com/bigquery/docs/dts) is now available in [beta](https://cloud.google.com/products/#product-launch-stages) in the [Cloud console](https://console.cloud.google.com/bigquery/transfers).
Change BigQuery now allows you to purchase as few as 500 slots when you enroll in a [flat-rate pricing plan](https://cloud.google.com/bigquery/pricing#flat_rate_pricing). You are also now able to choose either a monthly or a yearly flat-rate pricing plan. The new flat-rate pricing plans are currently in [alpha](https://cloud.google.com/products/#product-launch-stages).

## April 04, 2019

Change The BigQuery Data Transfer Service for Google Ads has been updated to support [v201809](https://docs.cloud.google.com/bigquery/docs/adwords-transfer) of Google Ads reports.
Feature [BigQuery Geographic Information Systems](https://cloud.google.com/bigquery/docs/gis-intro) is now [generally available (GA)](https://docs.cloud.google.com/terms/launch-stages).
Feature During the beta period, Table permissions were automatically applied to models for custom IAM roles. BigQuery ML will begin enforcing several new IAM [permissions](https://docs.cloud.google.com/bigquery/docs/access-control) on June 6, 2019. Customers who used custom IAM roles during the beta period must reconfigure these roles to use the new BigQuery ML permissions. This change will enable you to manage Models permissions separately from BigQuery ML Table permissions. You can begin redefining your custom roles by the end of April, 2019 when the permissions are released. Pre-defined IAM roles and basic roles are not impacted by this change.

## March 27, 2019

Feature Combined with a special migration agent, the BigQuery Data Transfer Service now provides a [database migration service](https://docs.cloud.google.com/bigquery/docs/data-warehouse-migration-overview) from an on-premises data warehouse system to BigQuery. Support for [Teradata](https://docs.cloud.google.com/bigquery/docs/teradata-migration) migrations is now in [beta](https://cloud.google.com/products/#product-launch-stages).

## March 18, 2019

Change The [limit on the number of `CREATE MODEL` queries](https://docs.cloud.google.com/bigquery-ml/quotas) has increased from 100 to 1,000.
Change BigQuery ML now supports [ML.ROC_CURVE](https://docs.cloud.google.com/bigquery-ml/docs/reference/standard-sql/bigqueryml-syntax-roc) and [ML.CONFUSION_MATRIX](https://docs.cloud.google.com/bigquery-ml/docs/reference/standard-sql/bigqueryml-syntax-confusion) without input data.

## March 12, 2019

Change Updated version of [Simba ODBC and JDBC driver](https://docs.cloud.google.com/bigquery/providers/simba-drivers). The Simba JDBC driver now includes support for the [BigQuery Storage API](https://docs.cloud.google.com/bigquery/docs/reference/storage).

## March 11, 2019

Feature BigQuery is now available in the Zürich (europe-west6) [region](https://docs.cloud.google.com/bigquery/docs/locations#regional_locations). For upcoming regional launches, see the [Google Cloud blog](https://cloud.google.com/blog/products/data-analytics/bigquery-arrives-in-the-london-region-with-more-regions-to-come).

## March 01, 2019

Feature The [BigQuery web UI](https://docs.cloud.google.com/bigquery/docs/bigquery-web-ui) in the Cloud console now supports assigning access controls to BigQuery datasets.
Change When you create a [scheduled query](https://docs.cloud.google.com/bigquery/docs/scheduling-queries), the minimum time interval for custom schedules has changed from three hours to 15 minutes.

## February 25, 2019

Feature You can now [save query results to Google Drive](https://cloud.google.com/bigquery/docs/writing-results#saving_query_results_to_google_drive) by using the BigQuery web UI in the Cloud console. Saving query results to Google Drive is currently in [beta](https://docs.cloud.google.com/terms/launch-stages).
Feature Support for DML [MERGE statements](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/dml-syntax#merge_statement) is now generally available (GA).

## February 22, 2019

Feature The [BigQuery Storage API](https://cloud.google.com/bigquery/docs/reference/storage/) is now in [beta](https://docs.cloud.google.com/terms/launch-stages).

## February 15, 2019

Feature BigQuery is now available in the São Paulo (southamerica-east1) [region](https://docs.cloud.google.com/bigquery/docs/locations#regional_locations). For upcoming regional launches, see the [Google Cloud blog](https://cloud.google.com/blog/products/data-analytics/bigquery-arrives-in-the-london-region-with-more-regions-to-come).

## February 13, 2019

Change The following [load job quota](https://docs.cloud.google.com/bigquery/quotas#load_jobs) has changed:

- The 16 MB maximum block size limit for Avro files has been removed.

## February 11, 2019

Change The following DML quotas have changed:

- The maximum number of combined UPDATE, DELETE, and MERGE statements per day per table has increased from 200 to 1,000.
- The limit of 10,000 combined UPDATE, DELETE, and MERGE statements per day per project has been removed. There is no longer a project-level limit on DML statements.

## February 04, 2019

Feature The Data Definition Language now supports `ALTER TABLE` statements to set options on [tables](https://cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#alter_table_set_options_statement) and [views](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#alter_view_set_options_statement). `ALTER TABLE` statements are currently in [beta](https://docs.cloud.google.com/terms/launch-stages).
Feature You can now use [`INFORMATION_SCHEMA`](https://cloud.google.com/bigquery/docs/information-schema-intro) views to retrieve resource metadata in BigQuery. `INFORMATION_SCHEMA` is currently in [beta](https://docs.cloud.google.com/terms/launch-stages).

## January 31, 2019

Change The [daily export quota](https://docs.cloud.google.com/bigquery/quotas#export_jobs) has been changed to measure the actual export file size in bytes, when counted toward your daily export quota. Usage is no longer measured by rounding up to the nearest GB.
Feature You can now try BigQuery without providing a credit card or attaching a billing account to your project by using the BigQuery [sandbox](https://cloud.google.com/bigquery/docs/sandbox). The sandbox is now in [beta](https://docs.cloud.google.com/terms/launch-stages) and is available to all Google Cloud customers.
Change The AudienceExplorer table has been deprecated by Google Ad Manager, and has been removed from [Google Ad Manager report transformations](https://docs.cloud.google.com/bigquery/docs/doubleclick-publisher-transformation).

## January 29, 2019

Feature BigQuery ML now supports automatic, batch gradient descent, and normal equation [optimization strategies](https://docs.cloud.google.com/bigquery-ml/docs/reference/standard-sql/bigqueryml-syntax-create#model_option_list) for linear regression models.

## January 28, 2019

Feature BigQuery is now available in the Los Angeles (us-west2) and Hong Kong (asia-east2) [regions](https://docs.cloud.google.com/bigquery/docs/locations#regional_locations). For upcoming regional launches, see the [Google Cloud blog](https://cloud.google.com/blog/products/data-analytics/bigquery-arrives-in-the-london-region-with-more-regions-to-come).

## January 22, 2019

Feature BigQuery [audit logging](https://docs.cloud.google.com/bigquery/docs/reference/auditlogs) now writes enhanced messages using the `BigQueryAuditMetadata` format.

## December 18, 2018

Feature BigQuery is now available in the Mumbai (asia-south1), and Montréal (northamerica-northeast1) [regions](https://docs.cloud.google.com/bigquery/docs/locations#regional_locations). For upcoming regional launches, see the [Google Cloud blog](https://cloud.google.com/blog/products/data-analytics/bigquery-arrives-in-the-london-region-with-more-regions-to-come).

## December 13, 2018

Feature The BigQuery ML [`ML.PREDICT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-predict) function now supports thresholds for binary logistic regression models.
Feature The BigQuery ML [`ML.WEIGHTS`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-weights) function now supports standardization.

## December 12, 2018

Change Raised the [maximum length of standard SQL queries to 1 MB](https://docs.cloud.google.com/bigquery/quotas).

## December 07, 2018

Change BigQuery is now available in the Taiwan (asia-east1), Finland (europe-north1), and Northern Virginia (us-east4) [regions](https://docs.cloud.google.com/bigquery/docs/locations#regional_locations). For upcoming regional launches, see the [Google Cloud blog](https://cloud.google.com/blog/products/data-analytics/bigquery-arrives-in-the-london-region-with-more-regions-to-come).

## December 05, 2018

Feature BigQuery now supports [converting Avro logical types](https://docs.cloud.google.com/bigquery/docs/loading-data-cloud-storage-avro#logical_types) when you load Avro data.
Feature You can now set or update a [default partition expiration](https://docs.cloud.google.com/bigquery/docs/updating-datasets#partition-expiration) time on a dataset.

## December 04, 2018

Change Updated versions of the [Simba ODBC and JDBC drivers](https://docs.cloud.google.com/bigquery/providers/simba-drivers).

## November 26, 2018

Change Updated version of the [Simba ODBC drivers](https://docs.cloud.google.com/bigquery/providers/simba-drivers).

## November 13, 2018

Feature BigQuery is now available in the Singapore (asia-southeast1) [region](https://docs.cloud.google.com/bigquery/docs/locations). For upcoming regional launches, see the [Google Cloud blog](https://cloud.google.com/blog/products/gcp).

## November 08, 2018

Change BigQuery ML [pricing](https://cloud.google.com/bigquery-ml/pricing) is now available.

## October 29, 2018

Feature BigQuery now supports the [`bigquery.metadataViewer`](https://docs.cloud.google.com/bigquery/docs/access-control#roles) predefined IAM role.

## October 22, 2018

Feature BigQuery is now available in the Sydney (australia-southeast1) [region](https://docs.cloud.google.com/bigquery/docs/locations). For upcoming regional launches, see the [Google Cloud blog](https://cloud.google.com/blog/products/gcp).

## October 19, 2018

Change The BigQuery ML [`CREATE MODEL`](https://docs.cloud.google.com/bigquery-ml/docs/reference/standard-sql/bigqueryml-syntax-create) statement has increased support for unique values in labels from 10 to 50. Multiclass logistic regression models now support up to 50 unique values for labels.

## October 17, 2018

Feature You can now try BigQuery without providing a credit card or attaching a billing account to your project by using the BigQuery [sandbox](https://cloud.google.com/bigquery/docs/sandbox) for Firebase. The sandbox is now in [beta](https://docs.cloud.google.com/terms/launch-stages).

## October 11, 2018

Change When you create a model using the `random` [data split method](https://docs.cloud.google.com/bigquery-ml/docs/reference/standard-sql/bigqueryml-syntax-create#model_option_list), the split is now deterministic. Subsequent training runs will produce the same split so long as the underlying input data hasn't changed.

Providing input data to the [`ML.EVALUATE`](https://docs.cloud.google.com/bigquery-ml/docs/reference/standard-sql/bigqueryml-syntax-evaluate) function is now optional.

## October 10, 2018

Feature Support for [using DML statements against partitioned tables](https://cloud.google.com/bigquery/docs/using-dml-with-partitioned-tables) is also [generally available (GA)](https://docs.cloud.google.com/terms/launch-stages).
Feature BigQuery is now available in the London (europe-west2) [region](https://docs.cloud.google.com/bigquery/docs/locations). For upcoming regional launches, see the [Google Cloud blog](https://cloud.google.com/blog/products/gcp).
Feature Support for tables [partitioned on a `DATE` or `TIMESTAMP` column](https://cloud.google.com/bigquery/docs/creating-column-partitions) is now [generally available (GA)](https://docs.cloud.google.com/terms/launch-stages).

## October 09, 2018

Change The [location](https://docs.cloud.google.com/bigquery/docs/locations#specifying_your_location) is no longer required when executing jobs that reference resources outside the `US` and `EU` regions.

## October 03, 2018

Feature Support for [Cloud Storage Transfers](https://cloud.google.com/bigquery/docs/cloud-storage-transfer) is now in [Beta](https://docs.cloud.google.com/terms/launch-stages).

## September 25, 2018

Change BigQuery standard SQL now supports the [`GENERATE_UUID` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/uuid_functions#generate_uuid).

## September 20, 2018

Change Updated versions of [Simba ODBC and JDBC drivers](https://docs.cloud.google.com/bigquery/providers/simba-drivers) to support regional resources.

## September 19, 2018

Feature BigQuery ML is now available in the Tokyo (asia-northeast1) [region](https://docs.cloud.google.com/about/locations).

## September 13, 2018

Feature BigQuery ML now supports [multiclass logistic regression models](https://docs.cloud.google.com/bigquery-ml/docs/reference/standard-sql/bigqueryml-syntax-create#models_in_bigqueryml_name) and the [`ML.CONFUSION_MATRIX` evaluation function](https://docs.cloud.google.com/bigquery-ml/docs/reference/standard-sql/bigqueryml-syntax-confusion).

## September 11, 2018

Feature [Scheduling queries](https://cloud.google.com/bigquery/docs/scheduling-queries) is now in [Beta](https://docs.cloud.google.com/terms/launch-stages).

## September 10, 2018

Feature [Loading data in ORC format](https://cloud.google.com/bigquery/docs/loading-data-cloud-storage-orc) is now [GA (Generally Available)](https://docs.cloud.google.com/terms/launch-stages).
Feature [BigQuery Geographic Information Systems](https://cloud.google.com/bigquery/docs/gis-intro) is now [Beta](https://docs.cloud.google.com/terms/launch-stages).

## August 27, 2018

Change Updated version of the [Simba JDBC driver](https://docs.cloud.google.com/bigquery/providers/simba-drivers).

## August 20, 2018

Feature BigQuery now supports [loading Cloud Firestore export data](https://docs.cloud.google.com/bigquery/docs/loading-data-cloud-firestore) and [querying Cloud Firestore data](https://docs.cloud.google.com/bigquery/external-data-cloud-storage) stored in Cloud Storage.
Feature The [`NUMERIC` data type](https://cloud.google.com/bigquery/docs/reference/standard-sql/data-types#numeric-type) is now [generally available](https://docs.cloud.google.com/terms/launch-stages).

## July 31, 2018

Change The [BigQuery Data Transfer Service](https://docs.cloud.google.com/bigquery/docs/transfer-service-overview) no longer requires an enrollment step for each data source when you enable the service.

## July 30, 2018

Feature BigQuery support for [clustered tables](https://cloud.google.com/bigquery/docs/clustered-tables) is now [Beta](https://docs.cloud.google.com/terms/launch-stages).

## July 25, 2018

Feature [Data Definition Language (DDL)](https://cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language) support in BigQuery is now [Generally Available (GA)](https://docs.cloud.google.com/terms/launch-stages).
Feature [BigQuery ML](https://cloud.google.com/bigquery/docs/bqml-introduction) is now [Beta](https://docs.cloud.google.com/terms/launch-stages).
Change BigQuery has achieved approximately 5x improvement in [TPC-DS](http://www.tpc.org/tpcds/) 10T power run geomean over the past 18 months.

## July 19, 2018

Feature BigQuery support for [clustered tables](https://cloud.google.com/bigquery/docs/clustered-tables) is now [Alpha](https://docs.cloud.google.com/terms/launch-stages).
Feature The [BigQuery web UI](https://cloud.google.com/bigquery/docs/bigquery-web-ui) in the Cloud console is now [Beta](https://docs.cloud.google.com/terms/launch-stages).

## July 18, 2018

Feature Support for [Google Play reports](https://cloud.google.com/bigquery/docs/play-transfer) is now in [Beta](https://docs.cloud.google.com/terms/launch-stages).

## July 11, 2018

Change Google Ads transfers now support API version [v201806](https://developers.google.com/adwords/api/docs/appendix/reports/all-reports).
Feature BigQuery Data Transfer Service for [Google Ads](https://cloud.google.com/bigquery/docs/adwords-transfer) is now [GA](https://docs.cloud.google.com/terms/launch-stages) (generally available).

## June 28, 2018

Feature BigQuery standard SQL now supports [templated SQL UDFs](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/user-defined-functions#templated-sql-udf-parameters).
Feature BigQuery standard SQL now supports the function [`GENERATE_TIMESTAMP_ARRAY`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/functions-and-operators#generate_timestamp_array).
Feature BigQuery standard SQL now supports the functions [`FROM_BASE32`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/functions-and-operators#from_base32) and [`TO_BASE32`.](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/functions-and-operators#to_base32)

## June 27, 2018

Feature BigQuery Data Transfer Service support for [YouTube Channel](https://cloud.google.com/bigquery/docs/youtube-channel-transfer) reports and [YouTube Content Owner](https://docs.cloud.google.com/bigquery/docs/youtube-content-owner-transfer) reports is now [GA](https://docs.cloud.google.com/terms/launch-stages) (generally available).

## June 18, 2018

Feature [Loading data in Parquet format](https://cloud.google.com/bigquery/docs/loading-data-cloud-storage-parquet) is now [GA](https://docs.cloud.google.com/terms/launch-stages) (generally available).
Feature [Loading data in ORC format](https://cloud.google.com/bigquery/docs/loading-data-cloud-storage-orc) is now [Beta](https://docs.cloud.google.com/terms/launch-stages).

## June 06, 2018

Change Updated version of the [Simba ODBC drivers](https://docs.cloud.google.com/bigquery/providers/simba-drivers).

## May 25, 2018

Change The following quotas have changed for load jobs:

- The [row and cell size limit](https://docs.cloud.google.com/bigquery/quotas#load_jobs) for CSV files has changed from 10 MB to 100 MB.
- The [row size limit](https://docs.cloud.google.com/bigquery/quotas#load_jobs) for JSON files has changed from 10 MB to 100 MB.

## May 15, 2018

Feature Support for the [`NUMERIC` data type](https://cloud.google.com/bigquery/docs/reference/standard-sql/data-types#numeric-type) is now in [Beta](https://docs.cloud.google.com/terms/launch-stages).

## May 04, 2018

Change The following quotas have changed:

- The [Maximum number of partitions per partitioned table](https://docs.cloud.google.com/bigquery/quotas#partitioned_tables) has changed from 2,500 to 4,000.
- The [Maximum number of combined UPDATE, DELETE, and MERGE DML statements per day per table](https://docs.cloud.google.com/bigquery/quotas#data_manipulation_language_statements) has changed from 96 to 200.

## May 01, 2018

Change When you [create a Google Ads transfer](https://docs.cloud.google.com/bigquery/docs/adwords-transfer), the maximum value for the refresh window is now 7 days. Existing transfers are unaffected by this change.
Change Updated version of the [Simba JDBC driver](https://docs.cloud.google.com/bigquery/providers/simba-drivers).

## April 17, 2018

Feature BigQuery now includes [`bigquery.jobs.listAll`permissions](https://docs.cloud.google.com/bigquery/docs/access-control#bq-permissions). `bigquery.jobs.list` still allows users to list all jobs in a project, but details and metadata are redacted for jobs submitted by other users.
Feature BigQuery is now available in the Tokyo (asia-northeast1) [region](https://docs.cloud.google.com/bigquery/docs/locations).

## April 13, 2018

Change By default, the number of [export jobs](https://docs.cloud.google.com/bigquery/quotas#export_jobs) per project per day has increased from 1,000 to 50,000.

## April 10, 2018

Feature Support for DML [`MERGE` statements](https://cloud.google.com/bigquery/docs/reference/standard-sql/dml-syntax#merge_statement) is now in [Beta](https://docs.cloud.google.com/terms/launch-stages).

## April 06, 2018

Feature Google Ad Manager transfers now support API version [v201802](https://developers.google.com/doubleclick-publishers/docs/rel_notes#v201802).

## March 30, 2018

Feature BigQuery now allows you to monitor [slot usage by job type](https://docs.cloud.google.com/bigquery/docs/monitoring#metrics) in Stackdriver.
Feature BigQuery standard SQL now supports: the [ERROR function to return errors](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/functions-and-operators#error) and the [SAFE prefix](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/functions-and-operators#safe-prefix) to suppress errors.

## March 05, 2018

Feature BigQuery support for [using DML statements with partitioned tables](https://cloud.google.com/bigquery/docs/using-dml-with-partitioned-tables) is now in [Beta](https://docs.cloud.google.com/terms/launch-stages).
Feature BigQuery [data definition language](https://docs.cloud.google.com/bigquery/docs/data-definition-language) now supports creating a table from the result of a query.

## March 01, 2018

Feature Support for [loading Parquet files](https://cloud.google.com/bigquery/docs/loading-data-cloud-storage-parquet) is now in [Beta](https://docs.cloud.google.com/terms/launch-stages).
Change Updated version of [Simba JDBC driver](https://docs.cloud.google.com/bigquery/providers/simba-drivers).

## February 16, 2018

Feature Google Ads transfers now support [manager accounts (MCCs)](https://docs.cloud.google.com/bigquery/docs/adwords-transfer#support_for_google_ads_manager_accounts).
Change Google Ads transfers now support [API version 201710](https://docs.cloud.google.com/bigquery/docs/adwords-transformation).

## February 15, 2018

Feature BigQuery Data Transfer Service [run notifications](https://cloud.google.com/bigquery/docs/transfer-run-notifications) are now in [Alpha](https://docs.cloud.google.com/terms/launch-stages).

## February 08, 2018

Change Several API methods will begin including a location field in their responses: `jobs.insert`, `jobs.get`, `jobs.cancel`, `jobs.query`, `jobs.getQueryResults`, `datasets.list`, and `jobs.list`. The location field is currently not required in requests.

For example, the `id` property will include a location in the following format: `"id": "[PROJECT_ID]:[LOCATION].[JOB_ID]"`. The `"id"` property should continue to be treated as opaque. Also, the `"jobReference"` property will begin including a location field in the following format:

    "jobReference": {
      "jobId": "[JOB_ID]",
      "location": "[LOCATION]",
      ...

Added documentation on [manually changing table schemas](https://docs.cloud.google.com/bigquery/docs/manually-changing-schemas).
Feature BigQuery support for [tables partitioned using a `TIMESTAMP` or `DATE` column](https://cloud.google.com/bigquery/docs/creating-column-partitions) is now in [Beta](https://docs.cloud.google.com/terms/launch-stages).

## January 17, 2018

Feature BigQuery [data definition language](https://cloud.google.com/bigquery/docs/data-definition-language) support is now in [Beta](https://docs.cloud.google.com/terms/launch-stages).

## January 08, 2018

Feature Google Ads transfers now support the following reports:

- [Age Range Performance Report](https://developers.google.com/adwords/api/docs/appendix/reports/v201705/age-range-performance-report)
- [Criteria Performance Report](https://developers.google.com/adwords/api/docs/appendix/reports/v201705/criteria-performance-report)
- [Gender Performance Report](https://developers.google.com/adwords/api/docs/appendix/reports/v201705/gender-performance-report)
- [Bid Goal Performance Report](https://developers.google.com/adwords/api/docs/appendix/reports/v201705/bid-goal-performance-report)
- [Budget Performance Report](https://developers.google.com/adwords/api/docs/appendix/reports/v201705/budget-performance-report)
- [Parental Status Performance Report](https://developers.google.com/adwords/api/docs/appendix/reports/v201705/parental-status-performance-report)

For more information, see [Google Ads Report Transformation](https://docs.cloud.google.com/bigquery/docs/adwords-transformation).

## January 05, 2018

Feature `ISOWEEK` and `ISOYEAR` are now supported by the following functions: [`DATE_TRUNC`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/functions-and-operators#date_trunc), [`DATETIME_TRUNC`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/functions-and-operators#datetime_trunc), [`TIMESTAMP_TRUNC`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/functions-and-operators#timestamp_trunc), [`DATE_DIFF`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/functions-and-operators#date_diff), and [`DATETIME_DIFF`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/functions-and-operators#datetime_diff).

## December 11, 2017

Feature You can now check the **Configure Jobs** option to allow the BigQuery Data Transfer Service to manage YouTube [Channel](https://docs.cloud.google.com/bigquery/docs/youtube-channel-transfer#setting_up_a_youtube_channel_transfer) and [Content Owner](https://docs.cloud.google.com/bigquery/docs/youtube-content-owner-transfer#setting_up_a_youtube_content_owner_transfer) reporting jobs for you.
Feature You can now exclude removed or deleted items when [setting up a Google Ads transfer](https://docs.cloud.google.com/bigquery/docs/adwords-transfer#setting_up_an_adwords_data_transfer).
Change [Backfill requests](https://docs.cloud.google.com/bigquery/docs/working-with-transfers#set_up_a_backfill) for Google Ads do not overwrite Match Tables.

## December 08, 2017

Change Updated versions of [Simba ODBC and JDBC releases](https://docs.cloud.google.com/bigquery/providers/simba-drivers).

## December 04, 2017

Feature The BigQuery web UI supports additional dimensions for filtering your [query history](https://docs.cloud.google.com/bigquery/bigquery-web-ui#viewing_query_history).

## November 30, 2017

Feature IAM [custom roles](https://docs.cloud.google.com/bigquery/docs/access-control#custom_roles) are now available for BigQuery.
Feature [`FARM_FINGERPRINT`](https://docs.cloud.google.com/bigquery/docs/reference/legacy-sql#farm_fingerprint) is now a supported function in legacy SQL.
Change There is now a quota of 300 concurrent [API requests](https://docs.cloud.google.com/bigquery/quotas#api_requests) per user.
Feature The `_PARTITIONTIME` pseudo column is now supported when [querying data using a wildcard table](https://docs.cloud.google.com/bigquery/docs/querying-wildcard-tables#scanning_a_range_of_ingestion-time_partitioned_tables_using_partitiontime) over partitioned tables.
Feature [`WEEK(<WEEKDAY>)`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/functions-and-operators#extract) is now a valid date part for the EXTRACT function.
Feature [`INTERSECT DISTINCT` and `EXCEPT DISTINCT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#set-operators) are now supported set operators in standard SQL.

## November 16, 2017

Feature [BigQuery Data Transfer Service](https://docs.cloud.google.com/bigquery/docs/transfer-service-overview) is now generally available.

## November 14, 2017

Change High compute queries are being phased out so you are no longer required to opt-in. All high compute queries under tier 100 are billed as tier 1. Running a high compute query does not require setting a threshold flag.

All queries above tier 100 will fail with a `RESOURCES_EXCEEDED_PER_BYTE` error unless the query is running in a reserved instance. Existing users who have run a high compute query higher than tier 100 in the last 30 days retain the ability to run queries at the higher billing tier until October 31, 2018.

## November 02, 2017

Feature The `IGNORE NULLS` and `RESPECT NULLS` modifiers are now supported in the `FIRST_VALUE`, `LAST_VALUE`, and `NTH_VALUE` [`analytic functions`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/functions-and-operators#analytic-functions).
Feature The `FOR SYSTEM_TIME AS OF` expression is now available in the [`FROM clause`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#from-clause).
Change Updated version of the [BigQuery client library for Python v0.28](https://docs.cloud.google.com/bigquery/docs/python-client-migration).

## October 16, 2017

Change Updated versions of [Simba ODBC and JDBC releases](https://docs.cloud.google.com/bigquery/providers/simba-drivers).

## September 29, 2017

Feature [`GROUP BY ROLLUP`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#group-by-clause) is now supported in standard SQL.

## September 28, 2017

Feature Added support for [creating ingestion-time partitioned tables](https://docs.cloud.google.com/bigquery/docs/creating-partitioned-tables#creating_ingestion-time_partitioned_tables) by specifying partitioning options when you write query results or load data into a new destination table.
Feature The following string functions are now supported in standard SQL:

- [`NORMALIZE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/functions-and-operators#normalize)
- [`NORMALIZE_AND_CASEFOLD`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/functions-and-operators#normalize_and_casefold)

## September 01, 2017

Feature The following analytic functions are now supported in standard SQL:

- [`PERCENTILE_CONT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/functions-and-operators#percentile_cont)
- [`PERCENTILE_DISC`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/functions-and-operators#percentile_disc)

## August 18, 2017

Feature Added best practices documentation for BigQuery:

- [BigQuery Best Practices](https://docs.cloud.google.com/bigquery/docs/best-practices)

## August 17, 2017

Change Updated versions of [Simba ODBC and JDBC releases](https://docs.cloud.google.com/bigquery/providers/simba-drivers).

## August 11, 2017

Feature The following standard SQL statistical aggregate functions now support the `OVER` clause --- `COVAR_SAMP`, `COVAR_POP`, and `CORR`:

- [Statistical-aggregate-functions](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/functions-and-operators#statistical-aggregate-functions)

## August 02, 2017

Feature Announced support for `WEEK`, `ISOWEEK`, and `ISOYEAR` with the standard SQL `EXTRACT` function:

- [`EXTRACT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/functions-and-operators#extract)

## July 27, 2017

Feature Announced availability of snappy codec for Avro compressed data blocks:

- [Avro format](https://docs.cloud.google.com/bigquery/data-formats#avro_format)

## July 26, 2017

Feature Announced availability of standard SQL `TO_HEX` and `FROM_HEX` functions:

- [`TO_HEX`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/functions-and-operators#to_hex)
- [`FROM_HEX`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/functions-and-operators#from_hex)

## July 13, 2017

Feature Announced support for using DML statements to modify tables with required fields:

- [DML](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-manipulation-language)
- You can also update a table with required fields by using a SELECT statement and writing to a destination table.

## June 19, 2017

Feature Announced availability of new public datasets:

- [World Bank: Education Data](https://docs.cloud.google.com/bigquery/public-data)
- [World Bank: Global Health, Nutrition, and Population Data](https://docs.cloud.google.com/bigquery/public-data)
- [World Bank: International Debt Data](https://docs.cloud.google.com/bigquery/public-data)

## June 07, 2017

Feature Announced availability of new public dataset:

- [NHTSA Traffic Fatality Data](https://docs.cloud.google.com/bigquery/public-data)

## June 06, 2017

Feature Announced availability of new public dataset:

- [OpenAQ: Real-time Air Quality Data](https://docs.cloud.google.com/bigquery/public-data)

## June 05, 2017

Feature The following standard SQL statistical aggregate functions now support the `OVER` clause --- `STDDEV`, `STDDEV_POP`, `STDDEV_SAMP`, `VAR_POP`, `VAR_SAMP`, `VARIANCE`:

- [Statistical-aggregate-functions](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/functions-and-operators#statistical-aggregate-functions)

## June 02, 2017

Feature Standard SQL now supports the [`TO_JSON_STRING`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/functions-and-operators#to_json_string) function.

## May 23, 2017

Feature Added troubleshooting documentation for the BigQuery Data Transfer Service:

- [Troubleshooting BigQuery Data Transfer Service Transfers](https://docs.cloud.google.com/bigquery/docs/transfer-troubleshooting)
Feature Added troubleshooting documentation for the BigQuery Data Transfer Service:

- [Troubleshooting BigQuery Data Transfer Service](https://docs.cloud.google.com/bigquery/docs/transfer-troubleshooting)

## May 22, 2017

Feature Announced availability of new public dataset:

- [Healthcare Common Procedure Coding System (HCPCS) Level II](https://docs.cloud.google.com/bigquery/public-data)

## May 17, 2017

Feature Announced [free tier pricing for storage](https://cloud.google.com/bigquery/pricing#storage).

## May 12, 2017

Feature Announced availability of new public datasets:

- [RxNorm](https://docs.cloud.google.com/bigquery/public-data)
- [United States Census Bureau International Data](https://docs.cloud.google.com/bigquery/public-data)
- [NOAA ICOADS Data](https://docs.cloud.google.com/bigquery/public-data)

## May 10, 2017

Feature Announced availability of new IAM role:

- [`bigquery.jobUser`](https://docs.cloud.google.com/bigquery/docs/access-control#bigquery.jobUser)

## May 05, 2017

Feature Announced availability of new public dataset:

- [Chicago Taxi Trips Data](https://docs.cloud.google.com/bigquery/public-data)

## May 02, 2017

Feature Announced availability of new public datasets:

- [Chicago Crime Data](https://docs.cloud.google.com/bigquery/public-data)
- [EPA Historical Air Quality Data](https://docs.cloud.google.com/bigquery/public-data)

## April 20, 2017

Feature Announced availability of new public dataset:

- [United States Census Data](https://docs.cloud.google.com/bigquery/public-data)

## April 05, 2017

Change Updated versions of [JDBC releases](https://docs.cloud.google.com/bigquery/providers/simba-drivers).

## March 31, 2017

Feature Announced availability of new public dataset:

- [1000 Cannabis Genomes Project](https://docs.cloud.google.com/bigquery/public-data)

## March 16, 2017

Feature Standard SQL now supports [REVERSE](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/functions-and-operators#reverse) and [ARRAY_REVERSE](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/functions-and-operators#array_reverse) functions for reversing STRING, BYTES, and ARRAY types.
Change Updated [limits](https://docs.cloud.google.com/bigquery/quota-policy#streaminginserts) for maximum rows per second when streaming data into BigQuery.

## March 15, 2017

Change Updated [limits](https://docs.cloud.google.com/bigquery/quota-policy#import) for Avro headers and maximum file size for load jobs.
Feature Standard SQL now supports several [functions that use HyperLogLog++](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/functions-and-operators#hyperloglog-functions) to estimate the number of unique values in a large dataset.
Feature Standard SQL now supports window, or analytic, mode for [ARRAY_AGG](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/functions-and-operators#array_agg) and [STRING_AGG](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/functions-and-operators#string_agg) using the OVER clause.

## March 14, 2017

Change Updated versions of [JDBC and ODBC releases](https://docs.cloud.google.com/bigquery/providers/simba-drivers).

## March 09, 2017

Feature Announced beta availability of [BigQuery Data Transfer Service](https://docs.cloud.google.com/bigquery/docs/transfer-service-overview).

## March 08, 2017

Feature Announced beta availability of [Querying Google Cloud Bigtable External Data Sources](https://docs.cloud.google.com/bigquery/external-data-bigtable).
Feature Announced availability of new public datasets:

- [Bay Area Bike Share Trips Data](https://docs.cloud.google.com/bigquery/public-data)
- [San Francisco Fire Department Service Calls Data](https://docs.cloud.google.com/bigquery/public-data)
- [San Francisco 311 Service Requests Data](https://docs.cloud.google.com/bigquery/public-data)
- [San Francisco Street Trees Data](https://docs.cloud.google.com/bigquery/public-data)
- [San Francisco Police Reports Data](https://docs.cloud.google.com/bigquery/public-data)
Feature Announced general availability of IGNORE NULLS and RESPECT NULLS modifiers for [ARRAY_AGG](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/functions-and-operators#array_agg) and [APPROX_QUANTILES](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/functions-and-operators#approx_quantiles) in standard SQL.
Change Increased [daily limits](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-manipulation-language#quotas) for [update and delete statements](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-manipulation-language).
Change Announced [daily limits](https://docs.cloud.google.com/bigquery/quotas#copy_jobs) for copying a table in BigQuery.
Feature Announced general availability of [Data Manipulation Language](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-manipulation-language) for nonpartitioned tables in standard SQL.

## February 01, 2017

Feature Announced availability of a new public dataset: [US Bureau of Labor Statistics data](https://docs.cloud.google.com/bigquery/public-data).
Feature Standard SQL now supports:

- [Bitwise operators for BYTES](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/functions-and-operators#bitwise-operators).
- The [BIT_COUNT](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/functions-and-operators#bit_count) function for BYTES.
- The string functions [LPAD](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/functions-and-operators#lpad), [RPAD](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/functions-and-operators#rpad), and [REPEAT](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/functions-and-operators#repeat).
- The [FARM_FINGERPRINT](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/functions-and-operators#farm_fingerprint) fingerprinting function.
- [Net functions](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/functions-and-operators#net-functions) for working with IP addresses: [NET.IP_FROM_STRING](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/functions-and-operators#netip_from_string), [NET.SAFE_IP_FROM_STRING](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/functions-and-operators#netsafe_ip_from_string), [NET.IP_TO_STRING](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/functions-and-operators#netip_to_string), [NET.IP_NET_MASK](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/functions-and-operators#netip_net_mask), [NET.IP_TRUNC](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/functions-and-operators#netip_trunc), [NET.IPV4_FROM_INT64](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/functions-and-operators#netipv4_from_int64), and [NET.IPV4_TO_INT64](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/functions-and-operators#netipv4_to_int64).
Feature Announced support for copying non-partitioned tables to partitioned tables. For more information, see [Copying to partitioned tables](https://docs.cloud.google.com/bigquery/docs/modifying-partitioned-tables#copying_to_partitioned_tables).
Feature Announced availability of a new public dataset: [New York City Tree Census data](https://docs.cloud.google.com/bigquery/public-data).

## December 22, 2016

Feature Announced availability of a new public dataset: [New York City Citi Bike Trips data](https://docs.cloud.google.com/bigquery/public-data).
Feature Announced availability of a new public dataset: [NYPD Motor Vehicle Collisions data](https://docs.cloud.google.com/bigquery/public-data).
Feature [Hash functions](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/functions-and-operators#hash-functions) in standard SQL.
Feature Standard SQL now supports the following code point functions: [TO_CODE_POINTS](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/functions-and-operators#to_code_points), [CODE_POINTS_TO_BYTES](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/functions-and-operators#code_points_to_bytes), [CODE_POINTS_TO_STRING](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/functions-and-operators#code_points_to_string).
Feature Announced availability of [custom null markers for CSV load jobs](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs#configuration.load.nullMarker).
Feature [URL functions](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/functions-and-operators#net-functions) in standard SQL.
Issue If you use [OMIT...IF](https://docs.cloud.google.com/bigquery/docs/reference/legacy-sql#omit) to exclude a portion of a record in a repeating field, and the query also selects other independently repeating fields, BigQuery now returns an error. To avoid this error, [migrate your query to standard SQL](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/migrating-from-legacy-sql#filtering_rows_with_omit_record_if).
Feature Announced availability of a new public dataset: [NYC 311 Service Requests data](https://docs.cloud.google.com/bigquery/public-data).
Feature Standard SQL now supports [parameterized queries](https://docs.cloud.google.com/bigquery/querying-data#running_parameterized_queries), which help prevent SQL injection when queries are constructed using user input.
Feature Standard SQL now supports ORDER BY and LIMIT clauses within [ARRAY_AGG](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/functions-and-operators#array_agg), [ARRAY_CONCAT_AGG](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/functions-and-operators#array_concat_agg), and [STRING_AGG](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/functions-and-operators#string_agg).

## November 30, 2016

Feature Announced availability of [JSON functions in standard SQL](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/functions-and-operators#json-functions).
Feature Announced availability of a new public dataset: [IRS Form 990 Data](https://docs.cloud.google.com/bigquery/public-data).
Feature Announced publication of a new tutorial:

- [Visualizing BigQuery Data Using Google Data Studio 360](https://docs.cloud.google.com/bigquery/docs/visualize-data-studio)

## November 07, 2016

Feature Announced availability of a new public dataset: [Medicare Data](https://docs.cloud.google.com/bigquery/public-data).

## November 02, 2016

Feature Announced availability of a new public dataset: [Open Images Data](https://docs.cloud.google.com/bigquery/public-data).

## November 01, 2016

Feature Announced [JDBC drivers](https://docs.cloud.google.com/bigquery/providers/simba-drivers#current_jdbc_driver_releases).
Feature Announced availability of a new public dataset: [Major League Baseball Data](https://docs.cloud.google.com/bigquery/public-data).
Change Additional documentation is available for [access control](https://docs.cloud.google.com/bigquery/docs/access-control) using Identity and Access Management (IAM):

- A table that describes the permissions available in BigQuery.
- A table that describes the permissions required for methods.
- A table that lists all BigQuery IAM roles and the permissions included in each role.
- A section that describes the methods to use for programmatic manipulation of roles.
Feature You can set a SQL variant, either legacy SQL or standard SQL, by [setting a query prefix](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/enabling-standard-sql#sql-prefix).
Feature Announced availability of [standard SQL safe casting functions](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/functions-and-operators#safe-casting), which return a null value in cases where normal casting generates an error.
Change The BigQuery [Client Libraries page](https://docs.cloud.google.com/bigquery/docs/reference/libraries) now links exclusively to one set of libraries: the [Google Cloud Client Libraries](https://docs.cloud.google.com/apis/docs/cloud-client-libraries). Work is underway to update all sample code throughout the BigQuery documentation to use the Google Cloud Client Libraries.
Change Bracket notation is now supported for the legacy SQL [JSON functions](https://docs.cloud.google.com/bigquery/docs/reference/legacy-sql#syntax-json).

## September 29, 2016

Feature Announced [BigQuery Stackdriver Monitoring](https://docs.cloud.google.com/bigquery/docs/monitoring) functionality, which enables you to monitor your BigQuery resources, including creating charts and alerts based on BigQuery metrics.
Feature [Flat-Rate Pricing](https://cloud.google.com/bigquery/pricing#flat_rate_pricing) gives you unlimited querying for a fixed monthly price.
Feature You can now [save and share queries](https://docs.cloud.google.com/bigquery/docs/saving-sharing-queries).
Feature Announced general availability of [standard SQL](https://docs.cloud.google.com/bigquery/sql-reference), which is based on the SQL 2011 standard, with extensions for querying nested and repeated data.

New documentation includes:

- [Migrating from legacy SQL](https://docs.cloud.google.com/bigquery/sql-reference/migrating-from-legacy-sql)
- [User-Defined Functions](https://docs.cloud.google.com/bigquery/sql-reference/user-defined-functions)
Feature Announced [ODBC drivers](https://docs.cloud.google.com/bigquery/providers/simba-drivers).
Feature Announced [Data Manipulation Language (DML)](https://docs.cloud.google.com/bigquery/sql-reference/data-manipulation-language) functionality for standard SQL, which enables you to insert, update, and delete rows and columns in your BigQuery datasets.

For syntax details and examples, see [DML Syntax](https://docs.cloud.google.com/bigquery/sql-reference/dml-syntax).

## August 26, 2016

Feature The updated beta IAM [predefined roles](https://docs.cloud.google.com/bigquery/docs/access-control) for BigQuery are now accessible to all.

## August 01, 2016

Change When [loading Avro files](https://docs.cloud.google.com/bigquery/docs/loading-data-cloud-storage-avro), BigQuery no longer creates an unnamed root-level column named "root." To ease the transition to this new schema, projects with the following contents are exempted from this change through a project override:

- Projects that contained federated tables based on Avro files as of August 1, 2016
- Projects that have queried or loaded Avro files in the 30 days prior to August 1, 2016

All affected project owners will be notified and provided the option to remove the project override.

## July 27, 2016

Feature Announced support for [labeling datasets](https://docs.cloud.google.com/bigquery/docs/labeling-datasets) using the BigQuery API or command-line tool.

## June 02, 2016

Feature Announced support for [time-partitioned tables](https://docs.cloud.google.com/bigquery/docs/partitioned-tables).
Feature Announced a Beta preview of Product [Identity and Access Management (IAM) Roles](https://docs.cloud.google.com/bigquery/docs/iam).
Feature Announced Beta availability of [standard SQL](https://docs.cloud.google.com/bigquery/sql-reference), which is based on the SQL 2011 standard, with extensions for querying nested and repeated data.

## May 06, 2016

Feature Announced support for Google Drive:

- You can [configure a Google Drive document as a federated data source](https://docs.cloud.google.com/bigquery/external-data-drive).
- You can [query a Google Drive data source using a temporary table](https://docs.cloud.google.com/bigquery/external-data-drive#temporary-tables).
- You can [save query results to Google Sheets](https://docs.cloud.google.com/bigquery/bigquery-web-ui#exportdata).
Change [Query reference](https://docs.cloud.google.com/bigquery/query-reference) updates:

- Updated [PARSE_PACKED_IP](https://docs.cloud.google.com/bigquery/query-reference#parse-packed-ip), which now returns BYTES.
- Added [FROM_BASE64](https://docs.cloud.google.com/bigquery/query-reference#from-base64).
- Added [SHA1](https://docs.cloud.google.com/bigquery/query-reference#sha1).
- Added [TO_BASE64](https://docs.cloud.google.com/bigquery/query-reference#to-base64).

## March 23, 2016

Change Updated the interface for [creating tables](https://docs.cloud.google.com/bigquery/docs/tables) in the BigQuery web UI.
Feature Added support for [CSV and JSON schema auto-detection](https://docs.cloud.google.com/bigquery/docs/schema-detect) for federated data sources in the BigQuery API or command-line tool.
Feature Announced availability of [BigQuery Public Datasets](https://docs.cloud.google.com/bigquery/public-data).
Feature Announced [Long Term Storage pricing](https://cloud.google.com/bigquery/pricing.html#long-term-storage).
Feature Added support for [Avro](https://docs.cloud.google.com/bigquery/docs/loading-data-cloud-storage-avro) source format for load operations and as an federated data source in the BigQuery API or command-line tool.
Feature Added support for the [BYTES data type](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types).

## December 15, 2015

Feature Added [Custom quotas](https://docs.cloud.google.com/bigquery/cost-controls), which allow you to set project-wide billing alerts that help prevent runaway costs.
Feature Added improvements to the [Streaming API](https://docs.cloud.google.com/bigquery/streaming-data-into-bigquery):

- You can use the Streaming API in [EU locations](https://docs.cloud.google.com/bigquery/streaming-data-into-bigquery#streaming_data_across_data_locations).
- You can use [Template tables](https://docs.cloud.google.com/bigquery/streaming-data-into-bigquery#template-tables) to manage related tables used for streaming.
- No more warm-up delay. You can now query your data within a few seconds of the first streaming insertion.
Feature Added support for [analyzing Google Cloud Audit logs](https://docs.cloud.google.com/bigquery/audit-logs) that enable you to track usage and access.
Feature Added [Query explain](https://docs.cloud.google.com/bigquery/query-plan-explanation), which shows, stage by stage, how BigQuery executes your queries.

## August 25, 2015

Feature Added streaming buffer statistics for tables that are being actively modified by [streaming inserts](https://docs.cloud.google.com/bigquery/streaming-data-into-bigquery). These statistics offer better information about table size and availability for copy/extract, and they can also help with debugging.
Change Pricing updates:

- Introduced [BigQuery Slots](https://docs.cloud.google.com/bigquery/docs/slots), which lets you guarantee resources, regardless of demand on the overall multi-tenant pool.
- Introduced query pricing tiers.
Feature BigQuery web UI improvements:

- Tables named with a date suffix (for example, when using the [`TABLE_DATE_RANGE`](https://docs.cloud.google.com/bigquery/query-reference#tablewildcardfunctions) function) display as a collapsed item in the navigation panel.
- [User-defined function (UDF)](https://docs.cloud.google.com/bigquery/user-defined-functions#webui) support.
- A new "Format Query" button.
Feature Added support for [user-defined functions (UDFs)](https://docs.cloud.google.com/bigquery/user-defined-functions).
Change Quota updates:
\* Increased the [concurrent rate limit](https://docs.cloud.google.com/bigquery/quota-policy#queries) to 50 queries.
\* Increased the [daily query limit](https://docs.cloud.google.com/bigquery/quota-policy#queries) to 100,000 queries.
Feature Added the ability to query CSV and JSON data directly from [Google Cloud Storage](https://docs.cloud.google.com/storage). For more information, see [federated data sources](https://docs.cloud.google.com/bigquery/external-data-sources).
Feature Added ability to [cancel a job](https://docs.cloud.google.com/bigquery/docs/reference/v2/jobs/cancel)

## April 16, 2015

Change Increased the [API requests per second, per user quota](https://docs.cloud.google.com/bigquery/quota-policy#apirequests) from 10 to 100 requests per second.
Feature Added ability to [undelete a table](https://docs.cloud.google.com/bigquery/docs/managing-tables#undeletetable) using table decorators.
Feature Added ability to [load data from Google Cloud Datastore](https://docs.cloud.google.com/bigquery/docs/loading-data-cloud-datastore).
Change [Query reference](https://docs.cloud.google.com/bigquery/query-reference) updates:

- Added [COALESCE](https://docs.cloud.google.com/bigquery/query-reference#coalesce).
- Added [GROUP_CONCAT_UNQUOTED](https://docs.cloud.google.com/bigquery/query-reference#group_concat_unquoted).
- Added support for `RIGHT OUTER JOIN` and `FULL OUTER JOIN`.
- Added `ROLLUP` and `GROUPING` modifiers.

## March 25, 2014

Feature Added [table wildcard functions](https://docs.cloud.google.com/bigquery/query-reference#tablewildcardfunctions), for a cost-effective way to query data from a specific set of tables.
Feature Improved `JOIN` support by adding `CROSS JOIN` and multiple `JOIN` functionality. For more information, see [joins](https://docs.cloud.google.com/bigquery/query-reference#joins).
Change Updated the concurrent rate limit and export request quota in the [quota policy](https://docs.cloud.google.com/bigquery/quota-policy).
Feature Added [views](https://docs.cloud.google.com/bigquery/querying-data#views). A view is a virtual table defined by a SQL query.
Feature Updated batch query functionality. Batch queries no longer have a 30 minute execution delay. Instead, batch jobs now begin as soon as idle resources are available. If the job hasn't started within 3 hours, the job priority changes to interactive.
Change Increased quota limits for [streaming data into BigQuery](https://docs.cloud.google.com/bigquery/streaming-data-into-bigquery).
Change Added a `description` property for [datasets](https://docs.cloud.google.com/bigquery/docs/reference/v2/datasets), [tables](https://docs.cloud.google.com/bigquery/docs/reference/v2/tables) and fields.
Feature Added [JSON functions](https://docs.cloud.google.com/bigquery/query-reference#jsonfunctions).
Change Changed the [pricing model](https://cloud.google.com/bigquery/pricing), lowering rates and adding reserved capacity pricing.
Feature Added [parallel export](https://docs.cloud.google.com/bigquery/exporting-data-from-bigquery#exportingmultiple) capability.

## September 18, 2013

Feature Added [table decorators](https://docs.cloud.google.com/bigquery/table-decorators) for time-based table snapshots.
Feature Added support for [streaming inserts](https://docs.cloud.google.com/bigquery/streaming-data-into-bigquery).
Feature Added new [aggregate functions](https://docs.cloud.google.com/bigquery/query-reference#aggfunctions).

## June 11, 2013

Change Added support for package-based pricing.
Feature Added support for [returning large query results](https://docs.cloud.google.com/bigquery/docs/writing-results#large-results).
Feature Added [query caching](https://docs.cloud.google.com/bigquery/docs/queries#querycaching).
Change Updated [quota policies](https://docs.cloud.google.com/bigquery/docs/quota-policy).
Feature Added [window functions](https://docs.cloud.google.com/bigquery/docs/query-reference#windowfunctions).

## March 14, 2013

Feature Added [GROUP EACH BY](https://docs.cloud.google.com/bigquery/docs/query-reference#groupby) functionality for aggregations of more distributed values.
Feature Added [JOIN EACH](https://docs.cloud.google.com/bigquery/docs/query-reference#joins) functionality for joining tables of any size.
Feature Added ability to add columns to table schema after the data is already loaded into BigQuery. To add columns to a table schema, pass the new schema to the `schema` parameter of [Tables.update()](https://docs.cloud.google.com/bigquery/docs/reference/v2/tables/update).
Feature Added [TIMESTAMP](https://docs.cloud.google.com/bigquery/docs/timestamp) data type functionality.

## October 01, 2012

Feature API: New support for [importing JSON files](https://docs.cloud.google.com/bigquery/docs/import) and [nested and repeated](https://docs.cloud.google.com/bigquery/docs/data#nested) fields
Change API: New [quota limits](https://docs.cloud.google.com/bigquery/docs/quota-policy)

## August 28, 2012

Feature **New tool release:** [BigQuery Connector for Excel](https://docs.cloud.google.com/bigquery/docs/bigquery_connector_for_excel) now allows you to make queries to Google BigQuery from Microsft Excel!
Feature API: New [batch queries](https://cloud.google.com/bigquery/docs/developers_guide#batchqueries) feature allows for queries to be executed between 30 minutes to 3 hours later, but are charged at a lower [cost](https://docs.cloud.google.com/bigquery/docs/pricing) than interactive queries which execute immediately

## August 17, 2012

Change API: Improved validation of project IDs
Feature API: CSV chunking - when importing, you can now specify the `allowQuotedNewlines` flag to be `FALSE` in the bq command-line tool, or in the API, and the import will be chunked, or processed in parallel. If you have quoted new lines in your data, be sure to set this flag to be `TRUE`.
Fixed API: Deletion of large tables has been fixed

## July 16, 2012

Feature **BigQuery Browser Tool**: Added syntax highlighting
Feature **BigQuery Browser Tool**: Add a drag handle to the separator below the query editor
Change API: Project owners can now call `bigquery.jobs.get` for jobs created by other users
Feature **BigQuery Browser Tool**: New favicon
Fixed **BigQuery Browser Tool**: Fixed some keyboard shortcuts in the query editor

## May 30, 2012

Fixed **API**: Improved list operations for tables and datasets
Change **API**: Introduced new quota enforcement schema. User can now have one (1) large query and one (1) pipeline job (import, export, streaming, etc.) concurrently.
Change **API**: Added support for leading underscores in field name
Fixed **API**: Improved error handling
Feature **API**: Added token based pagination support for list operations. Clients can use token based pagination to page through a single table snapshot. Subsequent changes to the table are not reflected when paging through list results.

## May 11, 2012

Change **API**: Custom delimiters added
Change **API**: Table expiration time is now exposed
Fixed **API**: Better errors for field/rows being too long

## April 20, 2012

Change **BigQuery Browser Tool**: New welcome page and more helpful messages when there aren't any queries or jobs to display
Change **API**

- Clearer quota error messages
- Copy jobs are now atomic

## April 13, 2012

Change **API**: Remove bigquery.jobs.delete
Change **BigQuery Browser Tool**

- Add retries for connection errors
- Table view: Field names in schemas are left-aligned
- Table view: Removed "Actions" menu (use the nav panel instead)
- Table view: Added "Query Table" menu, which prefills query builder with a table query
- Query builder has simpler prefill logic and a better help bubble

## April 05, 2012

Change **API**

- API Discovery docs have typed configuration
- Bug fix limiting the amount of data returned in a `TableData.List`
- [Atomic](https://en.wikipedia.org/wiki/Atomicity_(database_systems)) updates of tables for all import jobs
- Join behavior tightened (no outer join, only left most table can be large)
Change **BigQuery Browser Tool**

- 10MB limit to uploads done through the BigQuery browser tool
- Bug fix for zero row results

## March 09, 2012

Change **BigQuery Browser Tool**

- Pressing the tab key in the query box now adds a tab instead of tabbing to the next UI element
- Pressing the enter key in the query box now preserves leading whitespace to the next lines
- Pressing the enter key no longer starts a query but will always add a newline to the box, and CTRL+ENTER runs the query
- Navigation panel no longer forgets scroll location and submenu state when jobs are running
- BigQuery browser tool now has a new URL at <http://bigquery.cloud.google.com>; it is still possible to access the browser tool using the old URL

## March 01, 2012

Change **BigQuery Browser Tool**

- Faster BigQuery browser tool load time
- "Max invalid records" support in the BigQuery browser tool
Change **API**

- Enable support for `VARIANCE`, `STDDEV`, and `AVG` aggregate functions
- Add `job.kind` to `Jobs:list` API response
- Better error messages for quota-related errors

## February 01, 2012

Feature **BigQuery Browser Tool**

- `Show previous query results` option in the query panel
- The current project view is now highlighted
- "Refresh" option in the projects drop-down menu
Feature **API**

- Support for loading gzip compressed files from Google Cloud Storage
- Error handling
  - Faster detection of unreadable files
  - Improvements in handline multibyte UTF-8 characters

## November 01, 2011

Change **API**

- The default `configuration.load.createDisposition` for load jobs is now `CREATE_IF_NEEDED`
- "load" jobs now allow users to specify a number of rows to skip
  - This is useful for skipping header rows or other metadata that is not parseable when you import CSV files.
  - This value applies to all files in the source list.
- BigQuery can now load data from files that contain delimiters other than the comma
  - Delimiters may be any character expressable as a single Latin-1 byte other than `"`, , and .
  - Example (using the [bq command line tool](https://docs.cloud.google.com/bigquery/bq-command-line-tool-quickstart)): `bq load --field_delimiter=: newtable gs://bucket/input.txt`
Feature **BigQuery Browser Tool**

The Web UI now allows JSON schema format to be used in the **Create Table** dialog.

This is useful in situations where developers may want to use the same schema format for API calls, the `bq` command-line client, and the Web UI.

## October 01, 2011

Feature **BigQuery V2 Release**

- A new [REST API](https://docs.cloud.google.com/bigquery/docs/reference/v2)
- A new [web user interface](https://docs.cloud.google.com/bigquery/docs/browser_tool)
- Support for [JOIN](https://docs.cloud.google.com/bigquery/docs/query-reference#joins) statements
- Ability to export a table or query result to a CSV file in Google Cloud Storage
- Support for ACLs on groups of tables ([*datasets*](https://docs.cloud.google.com/bigquery/docs/reference/v2/datasets))
- A new object architecture describing tables, groups of tables, and queries