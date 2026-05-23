This page documents production updates to BigQuery. We recommend
that BigQuery developers periodically check this list for any
new announcements. BigQuery automatically updates to the latest
release and cannot be downgraded to a previous version.

For older release notes, see the
[Release notes archive](https://docs.cloud.google.com/bigquery/docs/release-notes-archive).


You can see the latest product updates for all of Google Cloud on the
[Google Cloud](https://docs.cloud.google.com/release-notes) page, browse and filter all release notes in the
[Google Cloud console](https://console.cloud.google.com/release-notes),
or programmatically access release notes in
[BigQuery](https://console.cloud.google.com/bigquery?p=bigquery-public-data&d=google_cloud_release_notes&t=release_notes&page=table).

To get the latest product updates delivered to you, add the URL of this page to your
[feed
reader](https://wikipedia.org/wiki/Comparison_of_feed_aggregators), or add the
[feed URL](https://docs.cloud.google.com/feeds/bigquery-release-notes.xml) directly.

## May 12, 2026

Feature You can now use the
[`AI.COUNT_TOKENS` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-count-tokens)
to estimate the token count of text input that you provide. For some generative
AI functions, you can [view](https://docs.cloud.google.com/bigquery/docs/generative-ai-overview#token_usage)
the total number of input, output, thought, and cache tokens for each modality
processed by the query. These features are in
[Preview](https://cloud.google.com/products#product-launch-stages).

## May 08, 2026

Announcement Starting August 11, 2026, the billing label for the BigQuery Data Transfer
Service SKU will be updated from `goog-bq-feature-type: DATA_TRANSFER_SERVICE`
(uppercase) to `goog-bq-feature-type: data_transfer_service` (lowercase) to
provide a more unified and complete view of your costs. This update expands the
scope of the label to cover all costs associated with the BigQuery Data Transfer
Service, including data transfer orchestration, data load operations, and data
merge operations.

To ensure uninterrupted cost visibility, update your billing exports,
dashboards, and reporting queries to include both these labels.

## May 06, 2026

Feature You can configure BigQuery sharing listings for multiple regions, which
allows you to share datasets and linked replicas across global geographies
simultaneously. For more information, see
[Create a listing](https://docs.cloud.google.com/bigquery/docs/analytics-hub-manage-listings#create_a_listing).
This feature is
[generally available](https://cloud.google.com/products#product-launch-stages)
(GA).
Breaking Starting June 1, 2026, due to changes in Google Ads data retention policies,
the BigQuery Data Transfer Service connectors for [Google
Ads](https://docs.cloud.google.com/bigquery/docs/transfer-changes#June01-google-ads), [Search Ads
360](https://docs.cloud.google.com/bigquery/docs/transfer-changes#June01-search-ads), and [Google Analytics
4](https://docs.cloud.google.com/bigquery/docs/transfer-changes#June01-ga4) will stop populating
data for backfill runs with dates earlier than 37 months from the current date.

For more information about the changes to the Google Ads data retention
policies, see [New Data Retention Policy for Google Ads starting June 1,
2026](https://ads-developers.googleblog.com/2026/05/new-data-retention-policy-for-google.html).

## April 30, 2026

Breaking Starting May 7, 2026, new transfer configurations that transfer data from Google Ads using the
BigQuery Data Transfer Service will require [Multi-factor authentication (MFA)
for individual user
authentication](https://ads-developers.googleblog.com/2026/04/multi-factor-authentication-requirement.html).
For more information, see [May 7,
2026](https://docs.cloud.google.com/bigquery/docs/transfer-changes#May7-google-ads).

## April 29, 2026

Breaking [Strict act-as mode](https://docs.cloud.google.com/dataform/docs/strict-act-as-mode)
is enforced globally for all Dataform repositories, requiring the use of a
custom service account or user credentials for running Dataform workflows,
BigQuery pipelines, notebooks, and data preparations.
Feature You can now use the
[`VECTOR_INDEX.STATISTICS` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/vectorindex_functions#vector_indexstatistics) to calculate how much an indexed table's data has drifted between when a
vector index was created and the present. If table data has changed enough
to require a [vector index rebuild](https://docs.cloud.google.com/bigquery/docs/vector-index#rebuild_a_vector_index), you can use the
[`ALTER VECTOR INDEX REBUILD` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#alter_vector_index_rebuild_statement)
to rebuild the vector index without downtime. These features are
[generally available](https://cloud.google.com/products/#product-launch-stages)
(GA).
Feature You can now use the `PARTITION BY` clause of the
[`CREATE VECTOR INDEX` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_vector_index_statement)
to [partition TreeAH vector indexes](https://docs.cloud.google.com/bigquery/docs/vector-index#partitions).
Partitioning enables partition pruning and can decrease I/O costs. This feature
is [Generally Available](https://cloud.google.com/products/#product-launch-stages).

## April 28, 2026

Feature You can now
[create materialized views over active change data capture (CDC) enabled tables](https://docs.cloud.google.com/bigquery/docs/materialized-views-intro#cdc).
This feature is [generally available](https://cloud.google.com/products#product-launch-stages)
(GA).

## April 23, 2026

Change An updated version of the
[Simba JDBC driver for BigQuery](https://docs.cloud.google.com/bigquery/docs/reference/odbc-jdbc-drivers#current_jdbc_driver)
is now available.

## April 22, 2026

Feature You can now use the [visual graph modeler](https://docs.cloud.google.com/bigquery/docs/graph-modeler) in
BigQuery Studio to define BigQuery graph nodes and edges from your
BigQuery tables and edit graph schema. This
feature is available in [Preview](https://docs.cloud.google.com/products#product-launch-stages).
Announcement Dataproc is now called [Managed Service for Apache Spark](https://docs.cloud.google.com/dataproc/docs/concepts/overview). The names for associated API, client
library, CLI, and Identity and Access Management (IAM) resources remain unchanged.
Announcement BigLake is now called [Google Cloud Lakehouse](https://docs.cloud.google.com/biglake/docs/introduction).
BigLake metastore is now called the [Lakehouse runtime
catalog](https://docs.cloud.google.com/biglake/docs/about-blms). The names for associated APIs, client
libraries, CLI commands, and Identity and Access Management (IAM) remain
unchanged and still reference BigLake.
Announcement Dataplex Universal Catalog is now called [Knowledge
Catalog](https://docs.cloud.google.com/dataplex/docs/introduction). The API, client library, CLI, and
Identity and Access Management (IAM) names remain unchanged. For more
information, see [Knowledge Catalog overview](https://docs.cloud.google.com/dataplex/docs/introduction).
Announcement Looker Studio is now called [Data Studio](https://docs.cloud.google.com/data-studio).
The website and endpoint change from `lookerstudio.google.com` to
`datastudio.google.com`. You do not need to update your reports for this change,
as Data Studio automatically redirects to the new domain. However,
if your company uses proxies to restrict access to external sites, your IT
administrator needs to add the new domain to your access control list (ACL).
The names for associated API, client library, CLI, and Identity and Access
Management (IAM) resources remain unchanged. For more information, see [Data Studio returns as new home for Data Cloud
assets](https://cloud.google.com/blog/products/data-analytics/looker-studio-is-data-studio).
Feature [BigQuery graphs](https://docs.cloud.google.com/bigquery/docs/graph-overview) now support the following
features:

- You can [query graphs](https://docs.cloud.google.com/bigquery/docs/conversational-analytics#graphs) using natural language in Conversational Analytics.
- You can add [descriptions and synonyms](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/graph-schema-statements#element_table_property_definition) to the labels and properties in your graphs.
- For some types of graphs you can [define measures](https://docs.cloud.google.com/bigquery/docs/graph-measures), which lock an aggregation to a key to help you perform complex aggregations without overcounting. To query measures, you transform your graph into a flattened table by using the [`GRAPH_EXPAND` TVF](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/graph-sql-queries#graph_expand), and then query measures in that table with the [`AGG` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#agg).

These features are in [Preview](https://cloud.google.com/products#product-launch-stages).
Feature You can now [use the Data Engineering Agent](https://docs.cloud.google.com/bigquery/docs/data-engineering-agent-pipelines)
to build, modify, and troubleshoot data pipelines in BigQuery. This feature is
[generally available](https://cloud.google.com/products/#product-launch-stages) (GA).
Feature You can now use the `gemini-embedding-2-preview` model in the
[`AI.EMBED`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-embed),
[`AI.SIMILARITY`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-similarity),
and
[`AI.GENERATE_EMBEDDING`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-generate-embedding)
functions to generate a single embedding from a combination of input types,
including text, image, audio, video, and PDF files.
This feature is in [Preview](https://cloud.google.com/products#product-launch-stages).

## April 21, 2026

Feature You can now [visualize BigQuery graph query results and graph
schemas](https://docs.cloud.google.com/bigquery/docs/graph-visualization#visualization-results) directly in
BigQuery Studio, without the need of a notebook environment. This feature is in
[Preview](https://cloud.google.com/products/#product-launch-stages).

## April 20, 2026

Change Starting July 25, 2026, the [BigQuery Data Transfer Service for Facebook Ads
connector](https://docs.cloud.google.com/bigquery/docs/facebook-ads-transfer) will update the data type
mapping for the `ActionValue` field in the `AdInsightsActions` report from `INT`
to `FLOAT`.
Feature The following features have been added to [Python UDFs](https://docs.cloud.google.com/bigquery/docs/user-defined-functions-python)
during [Preview](https://cloud.google.com/products/#product-launch-stages):

- Vectorized UDFs with Apache Arrow. You can now create [vectorized Python
  UDFs](https://docs.cloud.google.com/bigquery/docs/user-defined-functions-python#create-vector-udf-apache) using the Apache Arrow `RecordBatch` interface for improved performance.
- Cloud Monitoring integration. Python UDFs now export [metrics](https://docs.cloud.google.com/bigquery/docs/user-defined-functions-python#view_python_udf_metrics) to Cloud Monitoring, including CPU utilization, memory utilization, and maximum concurrent requests per instance.
- Container request concurrency. A new option, `container_request_concurrency`, is available for the `CREATE FUNCTION` statement. This option controls the maximum number of concurrent requests per Python UDF container instance.
- New quotas. Python UDFs are now subject to [new quotas](https://docs.cloud.google.com/bigquery/quotas#udf_limits) on image storage bytes (10 GiB per project per region) and mutation rate (30 per minute per project per region).
- Cost visibility. Python UDF costs can be seen in the `external_service_costs` column in the `INFORMATION_SCHEMA.JOBS` view and in the `ExternalServiceCosts` field in the [Job API](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/Job#externalservicecost).
Feature You can now [migrate metadata from external data catalogs to BigLake tables for
Apache
Iceberg](https://docs.cloud.google.com/bigquery/docs/migration/external-metastore-lakehouse-migration). This
feature supports external data catalogs such as such as Apache Hive Metastore
and Apache Iceberg REST Catalog. This feature is in
[Preview](https://cloud.google.com/products#product-launch-stages).
Feature You can use the [BigQuery MCP server](https://docs.cloud.google.com/bigquery/docs/use-bigquery-mcp)
to perform a range of data-related tasks with your AI applications including:

- Examining BigQuery resources.
- Generating accurate and efficient SQL queries.
- Securely executing queries.
- Interpreting query results.

This feature is [Generally Available](https://cloud.google.com/products#product-launch-stages)
(GA).
Feature You can now publish a [BigQuery Conversational Analytics agent in Gemini
Enterprise](https://docs.cloud.google.com/bigquery/docs/create-data-agents#publish-agent-gemini-enterprise).
This feature is in
[Preview](https://cloud.google.com/products/#product-launch-stages).
Feature You can now use the [notebook gallery](https://docs.cloud.google.com/bigquery/docs/notebooks-introduction#notebook_gallery)
in the BigQuery web UI as your central hub for discovering and using prebuilt notebook
templates. This feature is [generally available](https://cloud.google.com/products/#product-launch-stages)
(GA).

## April 17, 2026

Feature Using
[folders](https://docs.cloud.google.com/bigquery/docs/code-asset-folders)
to organize and control access to single file code assets is
[generally available](https://cloud.google.com/products#product-launch-stages)
(GA). In addition, you can perform bulk move and delete operations, refresh
folder contents, and view full breadcrumb paths based on resource permissions.
For more information, see
[Create and manage folders](https://docs.cloud.google.com/bigquery/docs/create-manage-folders).

## April 16, 2026

Feature [Conversational analytics](https://docs.cloud.google.com/bigquery/docs/conversational-analytics) now supports
querying Lakehouse tables that connect to the Apache Iceberg REST catalog or are
federated to an external catalog. For more information, see [Query BigLake data
with natural language](https://docs.cloud.google.com/biglake/docs/conversational-analytics).

This feature is in [Preview](https://cloud.google.com/products#product-launch-stages).
Feature You can now use [Colab Data Apps](https://docs.cloud.google.com/bigquery/docs/colab-data-apps)
to transform your data analyses from Colab notebooks into
polished, interactive applications.

This feature is in [Preview](https://cloud.google.com/products#product-launch-stages).
Feature You can now use the
[`AI.KEY_DRIVERS` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-key-drivers)
to identify segments of data that cause statistically significant changes to a
summable metric.

This feature is in
[Preview](https://cloud.google.com/products/#product-launch-stages).

## April 15, 2026

Feature BigQuery Apache Iceberg external tables now support
[Iceberg version 3](https://iceberg.apache.org/spec/#version-3-extended-types-and-capabilities),
including binary deletion vectors. For more information, see
[Apache Iceberg external tables](https://docs.cloud.google.com/bigquery/docs/iceberg-external-tables).
This feature is in
[Preview](https://cloud.google.com/products/#product-launch-stages).
Feature BigQuery agent analytics is now [generally available](https://cloud.google.com/products#product-launch-stages) (GA) in the Google Agent Developer Kit. [BigQuery agent analytics](https://docs.cloud.google.com/bigquery/docs/bigquery-agent-analytics)
is an open source solution that lets you capture, analyze, and visualize
multimodal agent interaction data at scale.
Announcement A known issue has been resolved where a materialized view refresh could expose could expose masked or filtered data from fine grained access control policies in error messages. No further action is needed.
Feature You can now use [`EXPORT DATA`
statements](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/export-statements#export_to_alloydb) to [reverse
ETL BigQuery data to AlloyDB](https://docs.cloud.google.com/bigquery/docs/export-to-alloydb). This feature is
in [Preview](https://cloud.google.com/products/#product-launch-stages).

## April 13, 2026

Feature Support for the `AI.AGG` function [preview](https://cloud.google.com/products/#product-launch-stages)
has been temporarily disabled. We are working to restore this feature as soon as
possible.
Feature To reduce LLM token consumption and query latency when processing large
datasets, enable [optimized mode](https://docs.cloud.google.com/bigquery/docs/optimize-ai-functions)
using the following [managed AI functions](https://docs.cloud.google.com/bigquery/docs/generative-ai-overview#managed_ai_functions):

- [`AI.IF`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-if)
- [`AI.CLASSIFY`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-classify)

This feature is in
[Preview](https://cloud.google.com/products/#product-launch-stages).
Feature The following [managed AI functions](https://docs.cloud.google.com/bigquery/docs/generative-ai-overview#managed_ai_functions)
use Gemini to help you filter, join, rank, and classify your data:

- [`AI.IF`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-if): Filter and join text and unstructured data (such as images, PDFs, audio, or video) based on a condition described in natural language.
- [`AI.SCORE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-score): Rate text and unstructured data (such as images, PDFs, audio, or video) to rank your data by quality, similarity, or other criteria.
- [`AI.CLASSIFY`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-classify): Classify text and unstructured data (such as images, PDFs, audio, or video) into user-defined categories.

These functions are [generally available](https://cloud.google.com/products#product-launch-stages)
(GA).
Feature You can use [visualization cells](https://docs.cloud.google.com/bigquery/docs/create-notebooks#cells) to
automatically [generate a visualization](https://docs.cloud.google.com/bigquery/docs/visualize-data-colab)
of any DataFrame in your notebook. You can customize the columns, chart type,
aggregations, colors, labels, and title.

This feature is [generally available](https://cloud.google.com/products#product-launch-stages)
(GA).

## April 10, 2026

Feature [SQL cells](https://docs.cloud.google.com/colab/docs/sql-cells) in BigQuery notebooks are now
[generally available](https://cloud.google.com/products/#product-launch-stages)
(GA).

## April 09, 2026

Feature The BigQuery Data Transfer Service can now
[transfer data from Snowflake to BigQuery](https://docs.cloud.google.com/bigquery/docs/migration/snowflake-transfer).
This feature is [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).
Feature You can now use stateful operations in [continuous
queries](https://docs.cloud.google.com/bigquery/docs/continuous-queries-introduction#supported_stateful_operations),
which let you perform complex analysis by retaining information across multiple
rows or time intervals using `JOIN`s and windowing aggregations. This feature is
in [Preview](https://cloud.google.com/products/#product-launch-stages).
Feature You can now use [BigQuery Graph](https://docs.cloud.google.com/bigquery/docs/graph-overview) to model your
data as a graph and perform analysis on a large scale.

- [Create a graph](https://docs.cloud.google.com/bigquery/docs/graph-create) directly from tables that store
  entities and relationships between entities. You don't need to modify your
  existing workflows or replicate your data to use it in graph queries.

- Use
  [Graph Query Language (GQL)](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/graph-intro)
  to find complex, hidden relationships between data points that would be
  challenging to find using SQL.

- [Visualize](https://docs.cloud.google.com/bigquery/docs/graph-visualization) your graph schema and graph
  query results in a notebook.

This feature is in
[Preview](https://cloud.google.com/products/#product-launch-stages).

## April 08, 2026

Feature The BigQuery Data Transfer Service now supports [incremental data transfers](https://docs.cloud.google.com/bigquery/docs/sqlserver-transfer#full_or_incremental_transfers)
when transferring data from Microsoft SQL Server to BigQuery. This feature is supported in
[Preview](https://cloud.google.com/products/#product-launch-stages).
Feature You can now use the
[`@@session_id` system variable](https://docs.cloud.google.com/bigquery/docs/reference/system-variables) with
SQL user-defined functions, table functions, and logical views. This feature is
[generally available](https://cloud.google.com/products#product-launch-stages)
(GA).

## April 07, 2026

Feature The BigQuery Data Transfer Service now supports incremental data transfers for
the following data source connectors:

- [MySQL](https://docs.cloud.google.com/bigquery/docs/mysql-transfer)
- [Oracle](https://docs.cloud.google.com/bigquery/docs/oracle-transfer)
- [PostgreSQL](https://docs.cloud.google.com/bigquery/docs/postgresql-transfer)
- [ServiceNow](https://docs.cloud.google.com/bigquery/docs/servicenow-transfer)

These features are supported in
[Preview](https://cloud.google.com/products/#product-launch-stages).
Feature You can now use the built-in text embedding model `embeddinggemma-300m` in the
[`AI.EMBED`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-embed)
and
[`AI.SIMILARITY`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-similarity)
functions. This model uses your BigQuery slots to generate embeddings at scale.
This feature is in
[Preview](https://cloud.google.com/products/#product-launch-stages).

## April 06, 2026

Feature You can now use the
[`AI.AGG` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-agg)
to semantically aggregate unstructured input data based on natural language
instructions. This feature is in
[Preview](https://cloud.google.com/products#product-launch-stages).
Feature You can now use a [custom organization policy](https://docs.cloud.google.com/bigquery/docs/custom-constraints)
to allow or deny specific operations on these BigQuery resources:
tables, data policies, and row access policies. This feature is in [preview](https://cloud.google.com/products/#product-launch-stages).

## April 02, 2026

Feature You can now use the
[`CREATE CONNECTION`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_connection_statement),
[`ALTER CONNECTION SET OPTIONS`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#alter_connection_set_options_statement),
and [`DROP CONNECTION`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#drop_connection_statement)
data definition language (DDL) statements to manage Cloud resource connections
with GoogleSQL. Additionally, you can now use the
[`connection` user type](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-control-language#user_list)
and [`PROJECT` resource type](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-control-language#arguments)
with `GRANT` and `REVOKE` data control language (DCL) statements to manage
connection and project access. These features are
[generally available](https://cloud.google.com/products#product-launch-stages)
(GA).
Feature The [BigQuery Migration Service supports SQL translations from Snowflake
SQL to GoogleSQL](https://docs.cloud.google.com/bigquery/docs/migration/snowflake-migration-intro).
This feature is now [generally available](https://cloud.google.com/products#product-launch-stages) (GA).

With this change, the translation service supports a wider variety of
Snowflake SQL and has improved support for several data types.
Among other changes, the translation service maps Snowflake
`INTEGER` and zero-scale `NUMERIC` types up to precision 38 to `INT64` type in
GoogleSQL for improved performance by default.
Feature You can set the
[column granularity](https://docs.cloud.google.com/bigquery/docs/search-index#column-granularity) when you
[create a search index](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_search_index_statement),
which stores additional column information in your search index to further
optimize your search query performance. This feature is
[generally available](https://cloud.google.com/products#product-launch-stages)
(GA).

## March 31, 2026

Feature BigQuery [`ObjectRef` values](https://docs.cloud.google.com/bigquery/docs/work-with-objectref)
now support the following:

- You can run [`ObjectRef` functions](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/objectref_functions) with either [direct access or delegated access](https://docs.cloud.google.com/bigquery/docs/work-with-objectref#authorizer_and_permissions).
- The [`OBJ.MAKE_REF` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/objectref_functions#objmake_ref) automatically fetches the latest Cloud Storage metadata and populates this in the `ref.details` field.
- The [`OBJ.GET_READ_URL` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/objectref_functions#objget_read_url) returns a `STRUCT` value with a read URL and status columns and renders image results in the Cloud console. Use this function when you don't require a write URL.

These features are
[generally available](https://cloud.google.com/products#product-launch-stages)
(GA).

## March 30, 2026

Feature The following forecasting and anomaly detection functions and updates are
[generally available](https://cloud.google.com/products#product-launch-stages)
(GA):

- The
  [`AI.DETECT_ANOMALIES` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-detect-anomalies)
  supports providing a custom context window that determines how many of the
  most recent data points should be used by the model.

- The
  [`AI.FORECAST` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-forecast)
  supports specifying the latest timestamp value for forecasting.

- The
  [`AI.EVALUATE` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-evaluate)
  supports the following:

  - You can provide a custom context window that determines how many of the most
    recent data points should be used by the model.

  - The function outputs the
    [mean absolute scaled error](https://en.wikipedia.org/wiki/Mean_absolute_scaled_error)
    for the time series.

Feature You can now create BigQuery [non-incremental materialized views over Spanner data](https://docs.cloud.google.com/bigquery/docs/materialized-views-create#spanner)
to improve query performance by periodically caching results. This feature is
[generally available](https://cloud.google.com/products/#product-launch-stages) (GA).

## March 26, 2026

Feature You can now use
[Cloud resource connections with `EXPORT DATA` statements](https://docs.cloud.google.com/bigquery/docs/export-to-spanner#export_using_a_cloud_resource_connection)
to reverse ETL BigQuery data to Spanner. This
feature is
[generally available](https://cloud.google.com/products/#product-launch-stages) (GA).

## March 25, 2026

Announcement The [Gemini for Google Cloud API](https://docs.cloud.google.com/gemini/docs/overview)
(cloudaicompanion.googleapis.com) is now enabled for existing
BigQuery projects in the European jurisdiction.
Feature You can now use the [BigQuery Migration Service MCP server](https://docs.cloud.google.com/bigquery/docs/use-bigquery-migration-mcp)
to perform SQL translation tasks, including translating SQL queries into
GoogleSQL syntax, generating DDL statements from SQL input queries, and getting
explanations of SQL translations.

This feature is in
[preview](https://cloud.google.com/products/#product-launch-stages).
Feature In BigQuery Data Transfer Service, you can
[monitor resource-level status reporting for Hive managed tables](https://docs.cloud.google.com/bigquery/docs/hdfs-data-lake-transfer#monitor-transfer-status)
to track progress and view granular error details for individual tables.
This feature is in
[preview](https://cloud.google.com/products#product-launch-stages).
Feature You can use the [BigQuery migration assessment for
Snowflake](https://docs.cloud.google.com/bigquery/docs/migration-assessment) to assess the complexity of
migrating from Snowflake to BigQuery. This feature is
[generally available](https://cloud.google.com/products#product-launch-stages)
(GA).

## March 24, 2026

Feature You can now use the [BigQuery Data Transfer Service remote MCP
server](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/mcp) to enable AI agents to
create, manage, and run data transfers. This feature is in
[Preview](https://cloud.google.com/products/#product-launch-stages).

## March 23, 2026

Feature The following functions are now
[generally available](https://cloud.google.com/products#product-launch-stages)
(GA):

- [`AI.EMBED`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-embed): create embeddings from text or image data.
- [`AI.SIMILARITY`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-similarity): compute the semantic similarity between pairs of text, pairs of images, or across text and images.
Feature You can clean, transform, and enrich data from files in Cloud Storage and Google
Drive in your BigQuery data preparations. For more information, see
[Prepare data with Gemini](https://docs.cloud.google.com/bigquery/docs/data-prep-get-suggestions#open-data-prep-editor).
This feature is [generally available](https://cloud.google.com/products#product-launch-stages)
(GA).

## March 19, 2026

Feature You can now use a [custom organization policy](https://docs.cloud.google.com/bigquery/docs/custom-constraints)
to allow or deny specific operations on routines. This feature is in
[preview](https://cloud.google.com/products/#product-launch-stages).

## March 17, 2026

Feature In BigQuery ML, you can now
[automatically deploy](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model-open#automatically_deployed_models)
open models to Vertex AI endpoints. Automatically deployed models offer the
following benefits:

- [Automatic Vertex AI resource management](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model-open#managed-resources)
- Reserve open model resources by [using Compute Engine reservations](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model-open#reservation-affinity)
- [Automatic or immediate open model undeployment](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model-open#managed-model-undeployment) to save costs

This feature is [generally available](https://cloud.google.com/products/#product-launch-stages)
(GA).

## March 16, 2026

Feature BigQuery now lets you configure a [global default location](https://docs.cloud.google.com/bigquery/docs/default-configuration#global-settings).
This setting is used if the location isn't set or can't be inferred from the
request. You can set the default location at the organization or project level.

This feature is [generally available](https://cloud.google.com/products/#product-launch-stages)
(GA).

## March 12, 2026

Change [BigQuery advanced runtime](https://docs.cloud.google.com/bigquery/docs/advanced-runtime) is now enabled as
the default runtime for all projects.

## March 11, 2026

Feature You can now understand and debug BigQuery query performance with
a
[visual mapping of your SQL query in the query execution graph](https://docs.cloud.google.com/bigquery/docs/query-plan-explanation#query_text_heatmap).
A heatmap highlights the steps that consume more slot-time. This feature is
[generally available](https://cloud.google.com/products#product-launch-stages)
(GA).

## March 09, 2026

Feature Updates to [conversational analytics](https://docs.cloud.google.com/bigquery/docs/conversational-analytics) include the following improvements:

- ObjectRef support: BigQuery conversational analytics now integrates with Google Cloud Storage through [ObjectRef functions](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/objectref_functions). This lets you reference and interact with unstructured data such as images and PDFs in Cloud Storage buckets in your conversational analysis.
- BQML support: BigQuery conversational analytics now supports [a set of BigQuery ML functions](https://docs.cloud.google.com/bigquery/docs/conversational-analytics#bigquery-ml-support), including AI.FORECAST, AI.DETECT_ANOMALIES, and AI.GENERATE. These functions let you perform advanced analytics tasks with simple conversational prompts.
- Chat with BigQuery results: You can now start conversations and chat with query results in BigQuery Studio (SQL editor).
- Enhanced support for partitioned tables: BigQuery conversational analytics can now use BigQuery table partitioning. The agent can optimize SQL queries by using partitioned columns such as date ranges on a date-partitioned table. This can improve query performance and reduce costs.
- Labels for agent-generated queries: BigQuery jobs initiated by the conversational analytics agent are now labeled in [BigQuery Job History](https://docs.cloud.google.com/bigquery/docs/managing-jobs) in the Google Cloud Console. You can identify, filter, and analyze the jobs run by the conversational analytics agent by referencing labels similar to `{'ca-bq-job': 'true'}`. These labels can help with the following tasks:
  - Monitor and attribute cost.
  - Audit agent activity.
  - Analyze agent-generated query performance.
- Suggest next questions (clickable): When working with BigQuery conversational analytics, the agent now suggests questions that are directly clickable in the Google Cloud console.

This feature is available in [Preview](https://cloud.google.com/products/#product-launch-stages).

## March 06, 2026

Feature You can create a [remote model](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model-embedding-maas)
based on the Vertex AI `gemini-embedding-001` model, or a
[remote model](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model-open)
based on an open embedding model from Vertex Model Garden or Hugging Face that
is deployed to Vertex AI.

You can then use the
[`AI.GENERATE_EMBEDDING` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-generate-embedding)
with these remote models to generate embeddings. You can also use the
[`AI.EMBED` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-embed)
directly with the `gemini-embedding-001` model endpoint.

These features are
[generally available](https://cloud.google.com/products/#product-launch-stages)
(GA).
Feature You can now use the [Pipelines \& Connections page](https://docs.cloud.google.com/bigquery/docs/pipeline-connection-page)
to streamline your data integration tasks by using guided,
BigQuery-specific configuration workflows for services like
BigQuery Data Transfer Service, Datastream, and Pub/Sub.

This feature is in [Preview](https://cloud.google.com/products/#product-launch-stages).

## March 05, 2026

Change An updated version of the
[Simba ODBC driver for BigQuery](https://docs.cloud.google.com/bigquery/docs/reference/odbc-jdbc-drivers#current_odbc_driver)
is now available.
Feature You can now use an alternate syntax when you call the
[`VECTOR_SEARCH` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/search_functions#vector_search)
to improve query performance when you search for a single vector. This feature
is in [Preview](https://cloud.google.com/products/#product-launch-stages).

## March 04, 2026

Feature Monitor dataset replication latency and network egress bytes in Cloud Monitoring
for BigQuery [cross-region replication](https://docs.cloud.google.com/bigquery/docs/data-replication#monitor-replication)
and [managed disaster recovery](https://docs.cloud.google.com/bigquery/docs/managed-disaster-recovery#monitor-replication).
These metrics are [generally available](https://cloud.google.com/products/#product-launch-stages)
(GA).
Feature You can now use [continuous queries to stream BigQuery data to Spanner in real
time](https://docs.cloud.google.com/bigquery/docs/continuous-queries#spanner-example). This feature is
[generally available](https://cloud.google.com/products/#product-launch-stages)
(GA).

## February 25, 2026

Change Effective *June 1, 2026* , BigQuery will limit legacy SQL use. This depends on
whether your organization or project uses it from November 1, 2025, to June 1,
2026. If you don't use legacy SQL during this time, you won't be able to use it
after June 1, 2026. If you do use it, your existing workloads
will keep running, but new ones might not. For more information, see
[Legacy SQL feature availability](https://docs.cloud.google.com/bigquery/docs/legacy-sql-feature-availability).

## February 24, 2026

Feature You can now [create and review](https://docs.cloud.google.com/bigquery/docs/create-data-agents#create-review-glossary-terms)
custom glossary terms in BigQuery for a conversational
analytics agent and you can review business glossary terms imported from
Dataplex Universal Catalog for an agent. These terms help an agent interpret your
prompts.

This feature is now in [Preview](https://cloud.google.com/products/#product-launch-stages).

## February 23, 2026

Feature You can now [undelete a dataset](https://docs.cloud.google.com/bigquery/docs/restore-deleted-datasets) that
is within your time travel window to recover it to the state that it was in when
it was deleted. This feature is [generally
available](https://cloud.google.com/products/#product-launch-stages) (GA).

## February 17, 2026

Feature You can now run [global queries](https://docs.cloud.google.com/bigquery/docs/global-queries), which let you
reference data stored in more than one region in a single query. This feature is
in [Preview](https://cloud.google.com/products#product-launch-stages).
Change After March 17, 2026, when you enable BigQuery, the BigQuery MCP server is
automatically enabled.
Deprecated Control of MCP use with organization policies is deprecated. After
March 17, 2026, organization policies that use the
`gcp.managed.allowedMCPServices constraint` won't work, and you can control
MCP use with IAM deny policies. For more information about controlling MCP use,
see [Control MCP use with IAM deny policies](https://docs.cloud.google.com/mcp/control-mcp-use-iam).

## February 12, 2026

Feature The
[`AI.CLASSIFY` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-classify)
now supports classifying your input into multiple categories. This feature is in
[Preview](https://cloud.google.com/products/#product-launch-stages).
Feature You can now provide descriptions for the fields in your custom output schema
when you use the
[`AI.GENERATE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-generate)
and
[`AI.GENERATE_TABLE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-generate-table)
functions.
This feature is [generally available](https://cloud.google.com/products/#product-launch-stages)
(GA).
Feature You can now use [dataset insights](https://docs.cloud.google.com/bigquery/docs/generate-dataset-insights)
to understand relationships between tables in a dataset by generating
relationship graphs and cross-table queries. You can automatically generate
dataset summaries, infer relationships across tables, and receive suggestions
for analytical questions. This feature is in
[Preview](https://cloud.google.com/products/#product-launch-stages).

## February 11, 2026

Feature You can now run pipelines with three distinct execution methods: running all
tasks, running selected tasks, and running tasks with selected tags. For more
information, see
[Run a pipeline](https://docs.cloud.google.com/bigquery/docs/create-pipelines#run-pipeline).
This feature is [generally available](https://cloud.google.com/products/#product-launch-stages)
(GA).

## February 09, 2026

Feature You can now customize the scope of data documentation scans for BigQuery tables
to generate specific insights. You can choose to generate only SQL queries,
only table and column descriptions, or all insights.

You can also create one-time data scans that execute immediately upon creation,
removing the need for a separate `run` command. These scans support a
Time to Live (TTL) setting to automatically delete the scan resource after
completion.

For more information, see
[Generate insights for a BigQuery table](https://docs.cloud.google.com/bigquery/docs/generate-table-insights#insights-bigquery-table).

## February 04, 2026

Change Data transfers from the [YouTube Channel](https://docs.cloud.google.com/bigquery/docs/youtube-channel-transfer)
and [YouTube Content Owner](https://docs.cloud.google.com/bigquery/docs/youtube-content-owner-transfer)
data sources now support reach reports. For more information, see
[YouTube Channel report transformation](https://docs.cloud.google.com/bigquery/docs/youtube-channel-transformation)
and [YouTube Content Owner report transformation](https://docs.cloud.google.com/bigquery/docs/youtube-content-owner-transformation).
Feature You can now associate [data policies directly on
columns](https://docs.cloud.google.com/bigquery/docs/column-data-masking#data-policies-on-column). This
feature enables direct database administration for controlling access and
applying masking and transformation rules at the column level. This feature is
now [generally
available](https://cloud.google.com/products/#product-launch-stages) (GA).

## February 03, 2026

Announcement Gemini in BigQuery now processes data in the same jurisdiction (`US` or `EU`) as
your BigQuery datasets, or based upon user-specified location settings. For more
information, see [Where Gemini BigQuery processes your
data](https://docs.cloud.google.com/bigquery/docs/gemini-locations).

## February 02, 2026

Feature You can now pass [parameterized queries](https://cloud.google.com/bigquery/docs/parameterized-queries)
from the BigQuery query editor in the Google Cloud console.

This feature is [generally available](https://cloud.google.com/products/#product-launch-stages)
(GA).

## January 29, 2026

Feature BigQuery now supports a `RANDOM_HASH` predefined masking rule. This rule returns
a hash of the column's value using a salted hash algorithm, and it provides
stronger security than the standard `Hash (SHA-256)` rule.

For more information, see [Data masking rules](https://docs.cloud.google.com/bigquery/docs/column-data-masking-intro#masking_options).
Feature BigQuery now offers [conversational analytics](https://docs.cloud.google.com/bigquery/docs/conversational-analytics),
which accelerates data analysis by enabling insights through natural language.
Users can view a predefined sample agent, chat with their BigQuery data or
custom agents, and access those agents even outside of BigQuery. They can also
use [supported BigQuery ML functions](https://docs.cloud.google.com/bigquery/docs/conversational-analytics#bigquery-ml-support)
in verified queries and in chat. This feature is in
[Preview](https://cloud.google.com/products/#product-launch-stages).
Feature You can now create [BigQuery ML models by using the
Google Cloud console](https://cloud.google.com/bigquery/docs/create-machine-learning-model-console).

This feature is [generally available](https://cloud.google.com/products/#product-launch-stages)
(GA).

## January 28, 2026

Change The BigQuery change data capture feature has been renamed to
[BigQuery change data capture ingestion](https://docs.cloud.google.com/bigquery/docs/change-data-capture).
Feature The BigQuery Data Transfer Service can now
[transfer data from Shopify to BigQuery](https://docs.cloud.google.com/bigquery/docs/shopify-transfer).
This feature is in [Preview](https://cloud.google.com/products/#product-launch-stages).

## January 27, 2026

Change An updated version of the
[Simba JDBC driver for BigQuery](https://docs.cloud.google.com/bigquery/docs/reference/odbc-jdbc-drivers#current_jdbc_driver)
is now available.
Feature The BigQuery Data Transfer Service can now
[transfer data from Mailchimp to BigQuery](https://docs.cloud.google.com/bigquery/docs/mailchimp-transfer).
This feature is in [Preview](https://cloud.google.com/products/#product-launch-stages).

## January 26, 2026

Feature You can now use Gemini Cloud Assist to
[discover resources](https://docs.cloud.google.com/bigquery/docs/use-cloud-assist#discover_resources)
across your projects. For example, you can ask about a specific table's schema,
or which tables contain demographic information about new users. This feature is
in [Preview](https://cloud.google.com/products/#product-launch-stages).

## January 23, 2026

Change You can now optionally specify which model to use by passing an endpoint
argument to the
[`AI.IF`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-if),
[`AI.SCORE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-score),
and
[`AI.CLASSIFY`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-classify)
functions.

## January 22, 2026

Fixed Support for
[table parameters in table-valued functions](https://docs.cloud.google.com/bigquery/docs/table-functions#table_parameters)
is restored.
Change You can now run queries that use the
[`AI.IF`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-if),
[`AI.SCORE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-score),
and
[`AI.CLASSIFY`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-classify)
functions by using your
[end-user credentials](https://docs.cloud.google.com/bigquery/docs/permissions-for-ai-functions) instead of a
BigQuery connection.

## January 21, 2026

Change BigQuery is now available in the [Bangkok (`asia-southeast3`) region](https://docs.cloud.google.com/bigquery/docs/locations#regions).
Feature You can now use Gemini Cloud Assist to
[get information about your job history](https://docs.cloud.google.com/bigquery/docs/use-cloud-assist#analyze_jobs),
such as why a particular query was slow or which queries were the most
resource-intensive in the past day. This feature is in
[Preview](https://cloud.google.com/products/#product-launch-stages).

## January 19, 2026

Breaking [Dataform workflows](https://docs.cloud.google.com/dataform/docs/sql-workflows),
[BigQuery notebooks](https://docs.cloud.google.com/bigquery/docs/orchestrate-notebooks),
[pipelines](https://docs.cloud.google.com/bigquery/docs/schedule-pipelines),
and
[data preparations](https://docs.cloud.google.com/bigquery/docs/orchestrate-data-preparations)
are enforcing strict act-as mode at the project level. To avoid failures and
maintain automatic releases, you must use custom service accounts instead of the
default Dataform service agent across all repositories. You must also grant the
Service Account User role (`roles/iam.serviceAccountUser`) to the default
Dataform service agent and relevant principals. For more information and to
verify act-as permissions, see
[Use strict act-as mode](https://docs.cloud.google.com/dataform/docs/strict-act-as-mode).

## January 07, 2026

Feature You can now use the Google-developed, open source
[Java Database Connectivity (JDBC) driver for BigQuery](https://docs.cloud.google.com/bigquery/docs/jdbc-for-bigquery)
to connect your Java applications to BigQuery. This feature is in
[Preview](https://cloud.google.com/products/#product-launch-stages).

## January 06, 2026

Feature The [`CREATE EXTERNAL TABLE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_external_table_statement)
and [`LOAD DATA`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/load-statements)
statements now support the following options:

- `time_zone`: specify a time zone to use when loading data
- `date_format`, `datetime_format`, `time_format`, and `timestamp_format`: define how date and time values are formatted in your source files
- `null_markers`: define the strings that represent `NULL` values in CSV files.
- `source_column_match`: specify how loaded columns are matched to the schema. You can match columns by position or by name.

These features are [generally
available](https://cloud.google.com/products/#product-launch-stages) (GA).

## December 22, 2025

Feature The BigQuery Data Transfer Service can now [transfer data from PostgreSQL to
BigQuery](https://docs.cloud.google.com/bigquery/docs/postgresql-transfer). This feature is [generally
available](https://cloud.google.com/products/#product-launch-stages) (GA).
Libraries

### Java

#### [2.58.0-rc1](https://github.com/googleapis/java-bigquery/compare/v2.57.1...v2.58.0-rc1) (2025-12-17)

##### Features

- Add ability to specify RetryOptions and BigQueryRetryConfig when create job and waitFor ([#3398](https://github.com/googleapis/java-bigquery/issues/3398)) ([1f91ae7](https://github.com/googleapis/java-bigquery/commit/1f91ae7fa2100a05f969a7429cb619a2b8b42dee))
- add additional parameters to CsvOptions and ParquetOptions ([#3370](https://github.com/googleapis/java-bigquery/issues/3370)) ([34f16fb](https://github.com/googleapis/java-bigquery/commit/34f16fbaad236f5a6db26d693efde2025913d540))
- add columnNameCharacterMap to LoadJobConfiguration ([#3356](https://github.com/googleapis/java-bigquery/issues/3356)) ([2f3cbe3](https://github.com/googleapis/java-bigquery/commit/2f3cbe39619bcc93cb7d504417accd84b418dd41))
- add max staleness to ExternalTableDefinition ([#3499](https://github.com/googleapis/java-bigquery/issues/3499)) ([f1ebd5b](https://github.com/googleapis/java-bigquery/commit/f1ebd5be5877a68f76efafc30e3b5b0763f343c5))
- add MetadataCacheMode to ExternalTableDefinition ([#3351](https://github.com/googleapis/java-bigquery/issues/3351)) ([2814dc4](https://github.com/googleapis/java-bigquery/commit/2814dc49dfdd5671257b6a9933a5dd381d889dd1))
- add remaining Statement Types ([#3381](https://github.com/googleapis/java-bigquery/issues/3381)) ([5f39b19](https://github.com/googleapis/java-bigquery/commit/5f39b19e8839f06d956addb8d95cf05e4b60a3f1))
- add WRITE_TRUNCATE_DATA as an enum value for write disposition ([#3752](https://github.com/googleapis/java-bigquery/issues/3752)) ([acea61c](https://github.com/googleapis/java-bigquery/commit/acea61c20b69b44c8612ca22745458ad04bc6be4))
- **bigquery:** Add custom ExceptionHandler to BigQueryOptions ([#3937](https://github.com/googleapis/java-bigquery/issues/3937)) ([de0914d](https://github.com/googleapis/java-bigquery/commit/de0914ddbccf988294d50faf56a515e58ab3505d))
- **bigquery:** Add OpenTelemetry Samples ([#3899](https://github.com/googleapis/java-bigquery/issues/3899)) ([e3d9ed9](https://github.com/googleapis/java-bigquery/commit/e3d9ed92ca5d9b58b5747960d74f895ed8733ebf))
- **bigquery:** Add OpenTelemetry support to BQ rpcs ([#3860](https://github.com/googleapis/java-bigquery/issues/3860)) ([e2d23c1](https://github.com/googleapis/java-bigquery/commit/e2d23c1b15f2c48a4113f82b920f5c29c4b5dfea))
- **bigquery:** Add otel metrics to request headers ([#3900](https://github.com/googleapis/java-bigquery/issues/3900)) ([4071e4c](https://github.com/googleapis/java-bigquery/commit/4071e4cb2547b236183fd4fbb92c73f074cf2fa0))
- **bigquery:** Add support for custom timezones and timestamps ([#3859](https://github.com/googleapis/java-bigquery/issues/3859)) ([e5467c9](https://github.com/googleapis/java-bigquery/commit/e5467c917c63ac066edcbcd902cc2093a39971a3))
- **bigquery:** Add support for reservation field in jobs. ([#3768](https://github.com/googleapis/java-bigquery/issues/3768)) ([3e97f7c](https://github.com/googleapis/java-bigquery/commit/3e97f7c0c4676fcdda0862929a69bbabc69926f2))
- **bigquery:** Implement getArray in BigQueryResultImpl ([#3693](https://github.com/googleapis/java-bigquery/issues/3693)) ([e2a3f2c](https://github.com/googleapis/java-bigquery/commit/e2a3f2c1a1406bf7bc9a035dce3acfde78f0eaa4))
- **bigquery:** Integrate Otel in client lib ([#3747](https://github.com/googleapis/java-bigquery/issues/3747)) ([6e3e07a](https://github.com/googleapis/java-bigquery/commit/6e3e07a22b8397e1e9d5b567589e44abc55961f2))
- **bigquery:** Integrate Otel into retries, jobs, and more ([#3842](https://github.com/googleapis/java-bigquery/issues/3842)) ([4b28c47](https://github.com/googleapis/java-bigquery/commit/4b28c479c1bc22326c8d2501354fb86ec2ce1744))
- **bigquery:** job creation mode GA ([#3804](https://github.com/googleapis/java-bigquery/issues/3804)) ([a21cde8](https://github.com/googleapis/java-bigquery/commit/a21cde8994e93337326cc4a2deb4bafd1596b77f))
- **bigquery:** Support Fine Grained ACLs for Datasets ([#3803](https://github.com/googleapis/java-bigquery/issues/3803)) ([bebf1c6](https://github.com/googleapis/java-bigquery/commit/bebf1c610e6d050c49fc05f30d3fa0247b7dfdcb))
- **bigquery:** support IAM conditions in datasets in Java client. ([#3602](https://github.com/googleapis/java-bigquery/issues/3602)) ([6696a9c](https://github.com/googleapis/java-bigquery/commit/6696a9c7d42970e3c24bda4da713a855dbe40ce5))
- **bigquery:** Support resource tags for datasets in java client ([#3647](https://github.com/googleapis/java-bigquery/issues/3647)) ([01e0b74](https://github.com/googleapis/java-bigquery/commit/01e0b742b9ffeafaa89b080a39d8a66c12c1fd3b))
- configure rc releases to be on prerelease mode ([93700c8](https://github.com/googleapis/java-bigquery/commit/93700c83559ca7ff4b9facc34a11c6dace3b5982))
- Enable Lossless Timestamps in BQ java client lib ([#3589](https://github.com/googleapis/java-bigquery/issues/3589)) ([c0b874a](https://github.com/googleapis/java-bigquery/commit/c0b874aa0150e63908450b13d019864b8cbfbfe3))
- Enable maxTimeTravelHours in BigQuery java client library ([#3555](https://github.com/googleapis/java-bigquery/issues/3555)) ([bd24fd8](https://github.com/googleapis/java-bigquery/commit/bd24fd8c550bfbd1207b194ed5c863a4a9924d48))
- implement wasNull for BigQueryResultSet ([#3650](https://github.com/googleapis/java-bigquery/issues/3650)) ([c7ef94b](https://github.com/googleapis/java-bigquery/commit/c7ef94be115cd572df589385f9be801033d72d6d))
- introduce `java.time` methods and variables ([#3586](https://github.com/googleapis/java-bigquery/issues/3586)) ([31fb15f](https://github.com/googleapis/java-bigquery/commit/31fb15fb963c18e4c29391e9fe56dfde31577511))
- new queryWithTimeout method for customer-side wait ([#3995](https://github.com/googleapis/java-bigquery/issues/3995)) ([9c0df54](https://github.com/googleapis/java-bigquery/commit/9c0df5422c05696f7ce4bedf914a58306150dc21))
- next release from main branch is 2.49.0 ([#3706](https://github.com/googleapis/java-bigquery/issues/3706)) ([b46a6cc](https://github.com/googleapis/java-bigquery/commit/b46a6ccc959f8defb145279ea18ff2e4f1bac58f))
- next release from main branch is 2.53.0 ([#3879](https://github.com/googleapis/java-bigquery/issues/3879)) ([c47a062](https://github.com/googleapis/java-bigquery/commit/c47a062136fea4de91190cafb1f11bac6abbbe3a))
- Relax client-side validation for BigQuery entity IDs ([#4000](https://github.com/googleapis/java-bigquery/issues/4000)) ([c3548a2](https://github.com/googleapis/java-bigquery/commit/c3548a2f521b19761c844c0b24fc8caab541aba7))
- update with latest from main ([#4034](https://github.com/googleapis/java-bigquery/issues/4034)) ([ec447b5](https://github.com/googleapis/java-bigquery/commit/ec447b57c250f6f0de504dd53b1dd31d76da1f03))

##### Bug Fixes

- adapt graalvm config to arrow update ([#3928](https://github.com/googleapis/java-bigquery/issues/3928)) ([ecfabc4](https://github.com/googleapis/java-bigquery/commit/ecfabc4b70922d0e697699ec5508a7328cadacf8))
- add clustering value to ListTables result ([#3359](https://github.com/googleapis/java-bigquery/issues/3359)) ([5d52bc9](https://github.com/googleapis/java-bigquery/commit/5d52bc9f4ef93f84200335685901c6ac0256b769))
- Add labels to converter for listTables method ([#3735](https://github.com/googleapis/java-bigquery/issues/3735)) ([#3736](https://github.com/googleapis/java-bigquery/issues/3736)) ([8634822](https://github.com/googleapis/java-bigquery/commit/8634822e1836c5ccc0f8d0263ac57ac561578360))
- **bigquery:** Add MY_VIEW_DATASET_NAME*TEST* to resource clean up sample ([#3838](https://github.com/googleapis/java-bigquery/issues/3838)) ([b1962a7](https://github.com/googleapis/java-bigquery/commit/b1962a7f0084ee4c3e248266b50406cf575cd657))
- **bigquery:** Remove ReadAPI bypass in executeSelect() ([#3624](https://github.com/googleapis/java-bigquery/issues/3624)) ([fadd992](https://github.com/googleapis/java-bigquery/commit/fadd992a63fd1bc87c99cc689ed103f05de49a99))
- Close bq read client ([#3644](https://github.com/googleapis/java-bigquery/issues/3644)) ([8833c97](https://github.com/googleapis/java-bigquery/commit/8833c97d73e3ba8e6a2061bbc55a6254b9e6668e))
- executeSelect now use provided credentials instead of GOOGLE_APP... ([#3465](https://github.com/googleapis/java-bigquery/issues/3465)) ([cd82235](https://github.com/googleapis/java-bigquery/commit/cd82235475310cacf1f607a412418be97c83559f))
- load jobs preserve ascii control characters configuration ([#3876](https://github.com/googleapis/java-bigquery/issues/3876)) ([5cfdf85](https://github.com/googleapis/java-bigquery/commit/5cfdf855fa0cf206660fd89743cbaabf3afa75a3))
- next release candidate ([d01971e](https://github.com/googleapis/java-bigquery/commit/d01971e74c7fc4b233c3504e2a56410a037bd501))
- NPE for executeSelect nonFast path with empty result ([#3445](https://github.com/googleapis/java-bigquery/issues/3445)) ([d0d758a](https://github.com/googleapis/java-bigquery/commit/d0d758a6e5e90502491eefa64e3a7409bdcea6a9))
- NPE when reading BigQueryResultSet from empty tables ([#3627](https://github.com/googleapis/java-bigquery/issues/3627)) ([9a0b05a](https://github.com/googleapis/java-bigquery/commit/9a0b05a3b57797b7cdd8ca9739699fc018dbd868))
- null field mode inconsistency ([#2863](https://github.com/googleapis/java-bigquery/issues/2863)) ([b9e96e3](https://github.com/googleapis/java-bigquery/commit/b9e96e3aa738a1813ad452cf6141f792f437e8de))
- retry ExceptionHandler not retrying on IOException ([#3668](https://github.com/googleapis/java-bigquery/issues/3668)) ([83245b9](https://github.com/googleapis/java-bigquery/commit/83245b961950ca9a993694082e533834ee364417))
- **test:** Force usage of ReadAPI ([#3625](https://github.com/googleapis/java-bigquery/issues/3625)) ([5ca7d4a](https://github.com/googleapis/java-bigquery/commit/5ca7d4acbbc40d6ef337732464b3bbd130c86430))
- **test:** Update schema for broken ConnImplBenchmark test ([#3574](https://github.com/googleapis/java-bigquery/issues/3574)) ([8cf4387](https://github.com/googleapis/java-bigquery/commit/8cf4387fae22c81d40635b470b216fa4c126d681))
- Update experimental methods documentation to [@internalapi](https://github.com/internalapi) ([#3552](https://github.com/googleapis/java-bigquery/issues/3552)) ([20826f1](https://github.com/googleapis/java-bigquery/commit/20826f1b08a3cc5bdcce5637b7ea21d467b2bce2))

##### Dependencies

- exclude io.netty:netty-common from org.apache.arrow:arrow-memor... ([#3715](https://github.com/googleapis/java-bigquery/issues/3715)) ([11b5809](https://github.com/googleapis/java-bigquery/commit/11b580949b910b38732c1c8d64704c54c260214e))
- fix update dependency com.google.cloud:google-cloud-bigquerystorage-bom to v3.17.2 ([b25095d](https://github.com/googleapis/java-bigquery/commit/b25095d23279dab178975c33f4de84612612e175))
- remove version declaration of open-telemetry-bom ([#3855](https://github.com/googleapis/java-bigquery/issues/3855)) ([6f9f77d](https://github.com/googleapis/java-bigquery/commit/6f9f77d47596b00b7317c8a0d4a10c3d849ad57b))
- rollback netty.version to v4.1.119.Final ([#3827](https://github.com/googleapis/java-bigquery/issues/3827)) ([94c71a0](https://github.com/googleapis/java-bigquery/commit/94c71a090eab745c81dd9530bcdd3c8c1e734788))
- update actions/checkout action to v4.1.6 ([#3309](https://github.com/googleapis/java-bigquery/issues/3309)) ([c7d6362](https://github.com/googleapis/java-bigquery/commit/c7d6362d47cb985abf3c08f5c4e89f651480c4c8))
- update actions/checkout action to v4.1.7 ([#3349](https://github.com/googleapis/java-bigquery/issues/3349)) ([0857234](https://github.com/googleapis/java-bigquery/commit/085723491e4aca58d670c313bc18b0c044cfdca8))
- update actions/checkout action to v4.2.0 ([#3495](https://github.com/googleapis/java-bigquery/issues/3495)) ([b57fefb](https://github.com/googleapis/java-bigquery/commit/b57fefbdfee7b8dacdb12502d1df72af21323b51))
- update actions/checkout action to v4.2.1 ([#3520](https://github.com/googleapis/java-bigquery/issues/3520)) ([ad8175a](https://github.com/googleapis/java-bigquery/commit/ad8175af06d5308a9366f8109055d61c115a4852))
- update actions/checkout action to v4.2.2 ([#3541](https://github.com/googleapis/java-bigquery/issues/3541)) ([c36c123](https://github.com/googleapis/java-bigquery/commit/c36c123f5cd298b1481c9073ac9f5e634b0e1e68))
- update actions/upload-artifact action to v4.3.4 ([#3382](https://github.com/googleapis/java-bigquery/issues/3382)) ([efa1aef](https://github.com/googleapis/java-bigquery/commit/efa1aef0a579baa379adbfbd2ee12f4ee5f3d987))
- update actions/upload-artifact action to v4.3.5 ([#3420](https://github.com/googleapis/java-bigquery/issues/3420)) ([d5ec87d](https://github.com/googleapis/java-bigquery/commit/d5ec87d16f64c231c8bfd87635952cb1a04f5e25))
- update actions/upload-artifact action to v4.3.5 ([#3422](https://github.com/googleapis/java-bigquery/issues/3422)) ([c7d07b3](https://github.com/googleapis/java-bigquery/commit/c7d07b3f1d6fa2c2259fa7315b284bcaf48ee5f2))
- update actions/upload-artifact action to v4.3.5 ([#3424](https://github.com/googleapis/java-bigquery/issues/3424)) ([a9d6869](https://github.com/googleapis/java-bigquery/commit/a9d6869251fa3df80d639c6998b62992468d6625))
- update actions/upload-artifact action to v4.3.5 ([#3427](https://github.com/googleapis/java-bigquery/issues/3427)) ([022eb57](https://github.com/googleapis/java-bigquery/commit/022eb578ae0b6f02e943662c8d4e453590f7c209))
- update actions/upload-artifact action to v4.3.5 ([#3430](https://github.com/googleapis/java-bigquery/issues/3430)) ([c7aacba](https://github.com/googleapis/java-bigquery/commit/c7aacbaeddc4809e283c6dfcdedd9610eac7730f))
- update actions/upload-artifact action to v4.3.5 ([#3432](https://github.com/googleapis/java-bigquery/issues/3432)) ([b7e8244](https://github.com/googleapis/java-bigquery/commit/b7e8244cffdef926465e2d2700766b98ad687247))
- update actions/upload-artifact action to v4.3.5 ([#3436](https://github.com/googleapis/java-bigquery/issues/3436)) ([ccefd6e](https://github.com/googleapis/java-bigquery/commit/ccefd6e755042b1e4c2aaec10228abb05779ed87))
- update actions/upload-artifact action to v4.3.5 ([#3440](https://github.com/googleapis/java-bigquery/issues/3440)) ([916fe9a](https://github.com/googleapis/java-bigquery/commit/916fe9ad67e5162a9f24852a96e40a2051ebffbd))
- update actions/upload-artifact action to v4.3.5 ([#3443](https://github.com/googleapis/java-bigquery/issues/3443)) ([187f099](https://github.com/googleapis/java-bigquery/commit/187f099edbf785e3ef50ae28fce6ae194d44dfb3))
- update actions/upload-artifact action to v4.3.5 ([#3444](https://github.com/googleapis/java-bigquery/issues/3444)) ([04aea5e](https://github.com/googleapis/java-bigquery/commit/04aea5e1d0eeab02f8ea92ff3467c64507dc05c9))
- update actions/upload-artifact action to v4.3.5 ([#3449](https://github.com/googleapis/java-bigquery/issues/3449)) ([c6e93cd](https://github.com/googleapis/java-bigquery/commit/c6e93cd1996f2feca3c79bf5ec4a079bd821c0f6))
- update actions/upload-artifact action to v4.3.5 ([#3455](https://github.com/googleapis/java-bigquery/issues/3455)) ([fbfc106](https://github.com/googleapis/java-bigquery/commit/fbfc1064688ba594a0d232c413e6f8b54558590f))
- update actions/upload-artifact action to v4.3.5 ([#3456](https://github.com/googleapis/java-bigquery/issues/3456)) ([f00977c](https://github.com/googleapis/java-bigquery/commit/f00977ccf60227bf1415795da5b6e0a208f21b2c))
- update actions/upload-artifact action to v4.3.5 ([#3462](https://github.com/googleapis/java-bigquery/issues/3462)) ([e1c6e92](https://github.com/googleapis/java-bigquery/commit/e1c6e92813c739fcd861e0622413b74c638cb547))
- update actions/upload-artifact action to v4.3.6 ([#3463](https://github.com/googleapis/java-bigquery/issues/3463)) ([ba91227](https://github.com/googleapis/java-bigquery/commit/ba91227b972acb1d0796d5a9470ba790dfb8d5b0))
- update actions/upload-artifact action to v4.4.0 ([#3467](https://github.com/googleapis/java-bigquery/issues/3467)) ([08b28c5](https://github.com/googleapis/java-bigquery/commit/08b28c510a2280119a03da3caa385ec31e0c944c))
- update actions/upload-artifact action to v4.4.1 ([#3521](https://github.com/googleapis/java-bigquery/issues/3521)) ([dc21975](https://github.com/googleapis/java-bigquery/commit/dc21975cc6f3597d8f789f12a58feaa5b9b94da0))
- update actions/upload-artifact action to v4.4.2 ([#3524](https://github.com/googleapis/java-bigquery/issues/3524)) ([776a554](https://github.com/googleapis/java-bigquery/commit/776a5541cc94e8ffb1f5e5c6969ae06585571b45))
- update actions/upload-artifact action to v4.4.3 ([#3530](https://github.com/googleapis/java-bigquery/issues/3530)) ([2f87fd9](https://github.com/googleapis/java-bigquery/commit/2f87fd9d777175cb5a8e5b0dc55f07546351e504))
- update actions/upload-artifact action to v4.5.0 ([#3620](https://github.com/googleapis/java-bigquery/issues/3620)) ([cc25099](https://github.com/googleapis/java-bigquery/commit/cc25099f81cbf94e9e2ee9db03a7d9ecd913c176))
- update actions/upload-artifact action to v4.6.0 ([#3633](https://github.com/googleapis/java-bigquery/issues/3633)) ([ca20aa4](https://github.com/googleapis/java-bigquery/commit/ca20aa47ea7826594975ab6aeb8498e2377f8553))
- update actions/upload-artifact action to v4.6.1 ([#3691](https://github.com/googleapis/java-bigquery/issues/3691)) ([9c0edea](https://github.com/googleapis/java-bigquery/commit/9c0edea7c00b3ffbe6b6a404e4161f768acb34f2))
- update actions/upload-artifact action to v4.6.2 ([#3724](https://github.com/googleapis/java-bigquery/issues/3724)) ([426a59b](https://github.com/googleapis/java-bigquery/commit/426a59b9b999e836804f84c5cbe11d497128f0a8))
- update actions/upload-artifact action to v4.6.2 ([#3724](https://github.com/googleapis/java-bigquery/issues/3724)) ([483f930](https://github.com/googleapis/java-bigquery/commit/483f9305023988b3884329733d0e5fbcb6599eb1))
- update bigquerystorage-bom to 3.20.0-rc1 ([#4035](https://github.com/googleapis/java-bigquery/issues/4035)) ([cb44b5f](https://github.com/googleapis/java-bigquery/commit/cb44b5f0d7ae817335f034ef5cd686246323df95))
- update dependency com.google.api.grpc:proto-google-cloud-bigqueryconnection-v1 to v2.46.0 ([#3328](https://github.com/googleapis/java-bigquery/issues/3328)) ([a6661ad](https://github.com/googleapis/java-bigquery/commit/a6661ade5e297102ff54d314fa55caac9201ac67))
- update dependency com.google.api.grpc:proto-google-cloud-bigqueryconnection-v1 to v2.47.0 ([#3342](https://github.com/googleapis/java-bigquery/issues/3342)) ([79e34c2](https://github.com/googleapis/java-bigquery/commit/79e34c256ddf99a43d546788535a9e8fa0e97e6d))
- update dependency com.google.api.grpc:proto-google-cloud-bigqueryconnection-v1 to v2.48.0 ([#3374](https://github.com/googleapis/java-bigquery/issues/3374)) ([45b7f20](https://github.com/googleapis/java-bigquery/commit/45b7f20e1b324d9b77183c0f8bb5ae14724d6aef))
- update dependency com.google.api.grpc:proto-google-cloud-bigqueryconnection-v1 to v2.49.0 ([#3417](https://github.com/googleapis/java-bigquery/issues/3417)) ([66336a8](https://github.com/googleapis/java-bigquery/commit/66336a8989681a7c5c3d901c11c7fc6cef0b9fef))
- update dependency com.google.api.grpc:proto-google-cloud-bigqueryconnection-v1 to v2.50.0 ([#3448](https://github.com/googleapis/java-bigquery/issues/3448)) ([2c12839](https://github.com/googleapis/java-bigquery/commit/2c128398b04c28ccd0844d028e2f8c467f8723f0))
- update dependency com.google.api.grpc:proto-google-cloud-bigqueryconnection-v1 to v2.51.0 ([#3480](https://github.com/googleapis/java-bigquery/issues/3480)) ([986b036](https://github.com/googleapis/java-bigquery/commit/986b036a022c8f68db59dd9d5944f3b724777533))
- update dependency com.google.api.grpc:proto-google-cloud-bigqueryconnection-v1 to v2.53.0 ([#3504](https://github.com/googleapis/java-bigquery/issues/3504)) ([57ce901](https://github.com/googleapis/java-bigquery/commit/57ce9018448ebf4f09d3ecf9760054ebd117bc36))
- update dependency com.google.api.grpc:proto-google-cloud-bigqueryconnection-v1 to v2.54.0 ([#3532](https://github.com/googleapis/java-bigquery/issues/3532)) ([25be311](https://github.com/googleapis/java-bigquery/commit/25be311c1477db0993a5825a2b839a295170790f))
- update dependency com.google.api.grpc:proto-google-cloud-bigqueryconnection-v1 to v2.55.0 ([#3559](https://github.com/googleapis/java-bigquery/issues/3559)) ([950ad0c](https://github.com/googleapis/java-bigquery/commit/950ad0cce6370e332a568d3b2e9ef3911503d206))
- update dependency com.google.api.grpc:proto-google-cloud-bigqueryconnection-v1 to v2.56.0 ([#3582](https://github.com/googleapis/java-bigquery/issues/3582)) ([616ee2a](https://github.com/googleapis/java-bigquery/commit/616ee2aa8ccf3d2975274b256252f2f249775960))
- update dependency com.google.api.grpc:proto-google-cloud-bigqueryconnection-v1 to v2.57.0 ([#3617](https://github.com/googleapis/java-bigquery/issues/3617)) ([51370a9](https://github.com/googleapis/java-bigquery/commit/51370a92e7ab29dfce91199666f23576d2d1b64a))
- update dependency com.google.api.grpc:proto-google-cloud-bigqueryconnection-v1 to v2.58.0 ([#3631](https://github.com/googleapis/java-bigquery/issues/3631)) ([b0ea0d5](https://github.com/googleapis/java-bigquery/commit/b0ea0d5bc4ac730b0e2eaf47e8a7441dc113686b))
- update dependency com.google.api.grpc:proto-google-cloud-bigqueryconnection-v1 to v2.59.0 ([#3660](https://github.com/googleapis/java-bigquery/issues/3660)) ([3a6228b](https://github.com/googleapis/java-bigquery/commit/3a6228b4adc638759d3b2725c612e97e1a3b9cec))
- update dependency com.google.api.grpc:proto-google-cloud-bigqueryconnection-v1 to v2.60.0 ([#3680](https://github.com/googleapis/java-bigquery/issues/3680)) ([6d9a40d](https://github.com/googleapis/java-bigquery/commit/6d9a40d55a6bbcbff7df39723d33f0af2b24f66e))
- update dependency com.google.api.grpc:proto-google-cloud-bigqueryconnection-v1 to v2.61.0 ([#3703](https://github.com/googleapis/java-bigquery/issues/3703)) ([53b07b0](https://github.com/googleapis/java-bigquery/commit/53b07b0e77f6ef57c8518df2b106edace679f79a))
- update dependency com.google.api.grpc:proto-google-cloud-bigqueryconnection-v1 to v2.62.0 ([#3726](https://github.com/googleapis/java-bigquery/issues/3726)) ([38e004b](https://github.com/googleapis/java-bigquery/commit/38e004b58134caf4f7b0d96257456930beb0e599))
- update dependency com.google.api.grpc:proto-google-cloud-bigqueryconnection-v1 to v2.63.0 ([#3770](https://github.com/googleapis/java-bigquery/issues/3770)) ([934389e](https://github.com/googleapis/java-bigquery/commit/934389eb114d8fbb10c9c125d21ec26d503dca65))
- update dependency com.google.api.grpc:proto-google-cloud-bigqueryconnection-v1 to v2.65.0 ([#3787](https://github.com/googleapis/java-bigquery/issues/3787)) ([0574ecc](https://github.com/googleapis/java-bigquery/commit/0574eccec2975738804be7d0ccb4c973459c82c9))
- update dependency com.google.api.grpc:proto-google-cloud-bigqueryconnection-v1 to v2.66.0 ([#3835](https://github.com/googleapis/java-bigquery/issues/3835)) ([69be5e7](https://github.com/googleapis/java-bigquery/commit/69be5e7345fb8ca69d633d9dc99cf6c15fa5227b))
- update dependency com.google.api.grpc:proto-google-cloud-bigqueryconnection-v1 to v2.68.0 ([#3858](https://github.com/googleapis/java-bigquery/issues/3858)) ([d4ca353](https://github.com/googleapis/java-bigquery/commit/d4ca3535f54f3282aec133337103bbfa2c9a3653))
- update dependency com.google.api.grpc:proto-google-cloud-bigqueryconnection-v1 to v2.69.0 ([#3870](https://github.com/googleapis/java-bigquery/issues/3870)) ([a7f1007](https://github.com/googleapis/java-bigquery/commit/a7f1007b5242da2c0adebbb309a908d7d4db5974))
- update dependency com.google.api.grpc:proto-google-cloud-bigqueryconnection-v1 to v2.70.0 ([#3890](https://github.com/googleapis/java-bigquery/issues/3890)) ([84207e2](https://github.com/googleapis/java-bigquery/commit/84207e297eec75bcb4f1cc1b64423d7c2ddd6c30))
- update dependency com.google.apis:google-api-services-bigquery to v2-rev20240602-2.0.0 ([#3273](https://github.com/googleapis/java-bigquery/issues/3273)) ([7b7e52b](https://github.com/googleapis/java-bigquery/commit/7b7e52b339f57af752c573a222df68196f1808f5))
- update dependency com.google.apis:google-api-services-bigquery to v2-rev20240616-2.0.0 ([#3368](https://github.com/googleapis/java-bigquery/issues/3368)) ([ceb270c](https://github.com/googleapis/java-bigquery/commit/ceb270c5cc2af4d69948ac89af1d72990fe1a7ee))
- update dependency com.google.apis:google-api-services-bigquery to v2-rev20240623-2.0.0 ([#3384](https://github.com/googleapis/java-bigquery/issues/3384)) ([e1de34f](https://github.com/googleapis/java-bigquery/commit/e1de34f0c4c67d75bcf15f35fe86c411b61d04ac))
- update dependency com.google.apis:google-api-services-bigquery to v2-rev20240629-2.0.0 ([#3392](https://github.com/googleapis/java-bigquery/issues/3392)) ([352562d](https://github.com/googleapis/java-bigquery/commit/352562da445e35a8207bcf77442130867f32e52d))
- update dependency com.google.apis:google-api-services-bigquery to v2-rev20240714-2.0.0 ([#3412](https://github.com/googleapis/java-bigquery/issues/3412)) ([8a48fd1](https://github.com/googleapis/java-bigquery/commit/8a48fd1eb6762e42bbdc49d1aa4ebab36c3e8e26))
- update dependency com.google.apis:google-api-services-bigquery to v2-rev20240727-2.0.0 ([#3421](https://github.com/googleapis/java-bigquery/issues/3421)) ([91d780b](https://github.com/googleapis/java-bigquery/commit/91d780b0db2b9b05923b60621cf80251293be184))
- update dependency com.google.apis:google-api-services-bigquery to v2-rev20240727-2.0.0 ([#3423](https://github.com/googleapis/java-bigquery/issues/3423)) ([16f350c](https://github.com/googleapis/java-bigquery/commit/16f350c28ec60dc4011b77cbda6416c9de45d431))
- update dependency com.google.apis:google-api-services-bigquery to v2-rev20240727-2.0.0 ([#3428](https://github.com/googleapis/java-bigquery/issues/3428)) ([9ae6eca](https://github.com/googleapis/java-bigquery/commit/9ae6ecac3337eb19bced14b9fcd7ce74580d7326))
- update dependency com.google.apis:google-api-services-bigquery to v2-rev20240803-2.0.0 ([#3435](https://github.com/googleapis/java-bigquery/issues/3435)) ([b4e20db](https://github.com/googleapis/java-bigquery/commit/b4e20db60b30dac9039407d724b8f7c816301e5c))
- update dependency com.google.apis:google-api-services-bigquery to v2-rev20240815-2.0.0 ([#3454](https://github.com/googleapis/java-bigquery/issues/3454)) ([8796aee](https://github.com/googleapis/java-bigquery/commit/8796aee5f669414169dc8baf88f9121697f4cd04))
- update dependency com.google.apis:google-api-services-bigquery to v2-rev20240905-2.0.0 ([#3483](https://github.com/googleapis/java-bigquery/issues/3483)) ([a6508a2](https://github.com/googleapis/java-bigquery/commit/a6508a29f81b6729e41e827096e90f1d1bf07f4d))
- update dependency com.google.apis:google-api-services-bigquery to v2-rev20240919-2.0.0 ([#3514](https://github.com/googleapis/java-bigquery/issues/3514)) ([9fe3829](https://github.com/googleapis/java-bigquery/commit/9fe382927ff4718252e22ac20c4e012f490e6b0e))
- update dependency com.google.apis:google-api-services-bigquery to v2-rev20241013-2.0.0 ([#3544](https://github.com/googleapis/java-bigquery/issues/3544)) ([0c42092](https://github.com/googleapis/java-bigquery/commit/0c42092e34912d21a4d13f041577056faadf914a))
- update dependency com.google.apis:google-api-services-bigquery to v2-rev20241027-2.0.0 ([#3568](https://github.com/googleapis/java-bigquery/issues/3568)) ([b5ccfcc](https://github.com/googleapis/java-bigquery/commit/b5ccfccb552e731ccb09be923715849a4282d44d))
- update dependency com.google.apis:google-api-services-bigquery to v2-rev20241111-2.0.0 ([#3591](https://github.com/googleapis/java-bigquery/issues/3591)) ([3eef3a9](https://github.com/googleapis/java-bigquery/commit/3eef3a9959bcfdb76c26fdf9069d9acf89f93a7a))
- update dependency com.google.apis:google-api-services-bigquery to v2-rev20241115-2.0.0 ([#3601](https://github.com/googleapis/java-bigquery/issues/3601)) ([41f9adb](https://github.com/googleapis/java-bigquery/commit/41f9adbe4235329fa2bbfd0930f4113e63f72e05))
- update dependency com.google.apis:google-api-services-bigquery to v2-rev20241222-2.0.0 ([#3623](https://github.com/googleapis/java-bigquery/issues/3623)) ([4061922](https://github.com/googleapis/java-bigquery/commit/4061922e46135d673bfa48c00bbf284efa46e065))
- update dependency com.google.apis:google-api-services-bigquery to v2-rev20250112-2.0.0 ([#3651](https://github.com/googleapis/java-bigquery/issues/3651)) ([fd06100](https://github.com/googleapis/java-bigquery/commit/fd06100c4c18b0416d384ec1f6bdfc796b70ad9f))
- update dependency com.google.apis:google-api-services-bigquery to v2-rev20250128-2.0.0 ([#3667](https://github.com/googleapis/java-bigquery/issues/3667)) ([0b92af6](https://github.com/googleapis/java-bigquery/commit/0b92af6eba4a633bb514089c24b7dd19cf286789))
- update dependency com.google.apis:google-api-services-bigquery to v2-rev20250216-2.0.0 ([#3688](https://github.com/googleapis/java-bigquery/issues/3688)) ([e3beb6f](https://github.com/googleapis/java-bigquery/commit/e3beb6ffe433db8ad4087d0f27a8f0d23e7c9322))
- update dependency com.google.apis:google-api-services-bigquery to v2-rev20250302-2.0.0 ([#3720](https://github.com/googleapis/java-bigquery/issues/3720)) ([c0b3902](https://github.com/googleapis/java-bigquery/commit/c0b39029302c51e65ea31495d837598eefbe94e8))
- update dependency com.google.apis:google-api-services-bigquery to v2-rev20250313-2.0.0 ([#3723](https://github.com/googleapis/java-bigquery/issues/3723)) ([b8875a8](https://github.com/googleapis/java-bigquery/commit/b8875a895d6d5e267086e24f97d0ed5fec36b9fe))
- update dependency com.google.apis:google-api-services-bigquery to v2-rev20250404-2.0.0 ([#3754](https://github.com/googleapis/java-bigquery/issues/3754)) ([1381c8f](https://github.com/googleapis/java-bigquery/commit/1381c8fe6c2552eec4519304c71697302733d6c7))
- update dependency com.google.apis:google-api-services-bigquery to v2-rev20250427-2.0.0 ([#3773](https://github.com/googleapis/java-bigquery/issues/3773)) ([c0795fe](https://github.com/googleapis/java-bigquery/commit/c0795fe948e0ca231dbe8fc47c470603cb48ecc8))
- update dependency com.google.apis:google-api-services-bigquery to v2-rev20250511-2.0.0 ([#3794](https://github.com/googleapis/java-bigquery/issues/3794)) ([d3bf724](https://github.com/googleapis/java-bigquery/commit/d3bf724feef91469b44e1e5068738604d2b3cead))
- update dependency com.google.apis:google-api-services-bigquery to v2-rev20250615-2.0.0 ([#3872](https://github.com/googleapis/java-bigquery/issues/3872)) ([f081589](https://github.com/googleapis/java-bigquery/commit/f08158955b7fec3c2ced6332b6e4d76cc13f2e90))
- update dependency com.google.apis:google-api-services-bigquery to v2-rev20250706-2.0.0 ([#3910](https://github.com/googleapis/java-bigquery/issues/3910)) ([ae5c971](https://github.com/googleapis/java-bigquery/commit/ae5c97146c7076e90c000fd98b797ec8e08a9cd8))
- update dependency com.google.apis:google-api-services-bigquery to v2-rev20251012-2.0.0 ([#3923](https://github.com/googleapis/java-bigquery/issues/3923)) ([1d8977d](https://github.com/googleapis/java-bigquery/commit/1d8977df3b1451378e5471cce9fd8b067f80fc9a))
- update dependency com.google.cloud:google-cloud-bigquerystorage-bom to v3.10.0 ([0bd3c86](https://github.com/googleapis/java-bigquery/commit/0bd3c862636271c5a851fcd229b4cf6878a8c5d4))
- update dependency com.google.cloud:google-cloud-bigquerystorage-bom to v3.10.1 ([c03a63a](https://github.com/googleapis/java-bigquery/commit/c03a63a0da4f4915e9761dc1ca7429c46748688c))
- update dependency com.google.cloud:google-cloud-bigquerystorage-bom to v3.10.2 ([19fc184](https://github.com/googleapis/java-bigquery/commit/19fc1843f7db8ab6fb361bf7f8119014033bc1c6))
- update dependency com.google.cloud:google-cloud-bigquerystorage-bom to v3.17.0 ([#3954](https://github.com/googleapis/java-bigquery/issues/3954)) ([e73deed](https://github.com/googleapis/java-bigquery/commit/e73deed9c68a45023d02b40144c304329d6b5829))
- update dependency com.google.cloud:google-cloud-bigquerystorage-bom to v3.9.0 ([c4afbef](https://github.com/googleapis/java-bigquery/commit/c4afbef9d4df03c798241d56d8988adb5724d008))
- update dependency com.google.cloud:google-cloud-datacatalog-bom to v1.50.0 ([#3330](https://github.com/googleapis/java-bigquery/issues/3330)) ([cabb0ab](https://github.com/googleapis/java-bigquery/commit/cabb0ab1bc09ba10c43a2cf109f1390268441693))
- update dependency com.google.cloud:google-cloud-datacatalog-bom to v1.51.0 ([#3343](https://github.com/googleapis/java-bigquery/issues/3343)) ([e3b934f](https://github.com/googleapis/java-bigquery/commit/e3b934fa133679a2d61baeea6f4de15eed287f7f))
- update dependency com.google.cloud:google-cloud-datacatalog-bom to v1.52.0 ([#3375](https://github.com/googleapis/java-bigquery/issues/3375)) ([2115c04](https://github.com/googleapis/java-bigquery/commit/2115c0448b242ddd887f2bac3d68c45847273c3d))
- update dependency com.google.cloud:google-cloud-datacatalog-bom to v1.53.0 ([#3418](https://github.com/googleapis/java-bigquery/issues/3418)) ([6cff7f0](https://github.com/googleapis/java-bigquery/commit/6cff7f0c2241223c529321e2b613f15c84ecbdcc))
- update dependency com.google.cloud:google-cloud-datacatalog-bom to v1.54.0 ([#3450](https://github.com/googleapis/java-bigquery/issues/3450)) ([cc9da95](https://github.com/googleapis/java-bigquery/commit/cc9da9576fa276afe069caff075c50e41e412ce1))
- update dependency com.google.cloud:google-cloud-datacatalog-bom to v1.55.0 ([#3481](https://github.com/googleapis/java-bigquery/issues/3481)) ([8908cfd](https://github.com/googleapis/java-bigquery/commit/8908cfd82332d09997a5538113fbe8e382f52c4a))
- update dependency com.google.cloud:google-cloud-datacatalog-bom to v1.57.0 ([#3505](https://github.com/googleapis/java-bigquery/issues/3505)) ([6e78f56](https://github.com/googleapis/java-bigquery/commit/6e78f56d17bb0d30b361220c86b1c66f21e9bd48))
- update dependency com.google.cloud:google-cloud-datacatalog-bom to v1.58.0 ([#3533](https://github.com/googleapis/java-bigquery/issues/3533)) ([cad2643](https://github.com/googleapis/java-bigquery/commit/cad26430f21a37eec2b87ea417f0cf67dcf9c97a))
- update dependency com.google.cloud:google-cloud-datacatalog-bom to v1.59.0 ([#3561](https://github.com/googleapis/java-bigquery/issues/3561)) ([1bd24a1](https://github.com/googleapis/java-bigquery/commit/1bd24a1ad28d168587b7cba95ec348cb1308a803))
- update dependency com.google.cloud:google-cloud-datacatalog-bom to v1.60.0 ([#3583](https://github.com/googleapis/java-bigquery/issues/3583)) ([34dd8bc](https://github.com/googleapis/java-bigquery/commit/34dd8bc22c8188f2b61dc9939b24a8d820548e2b))
- update dependency com.google.cloud:google-cloud-datacatalog-bom to v1.61.0 ([#3618](https://github.com/googleapis/java-bigquery/issues/3618)) ([6cba626](https://github.com/googleapis/java-bigquery/commit/6cba626ff14cebbc04fa4f6058b273de0c5dd96e))
- update dependency com.google.cloud:google-cloud-datacatalog-bom to v1.62.0 ([#3632](https://github.com/googleapis/java-bigquery/issues/3632)) ([e9ff265](https://github.com/googleapis/java-bigquery/commit/e9ff265041f6771a71c8c378ed3ff5fdec6e837b))
- update dependency com.google.cloud:google-cloud-datacatalog-bom to v1.63.0 ([#3661](https://github.com/googleapis/java-bigquery/issues/3661)) ([9bc8c01](https://github.com/googleapis/java-bigquery/commit/9bc8c0115dc16fb950567cd85cc7dfaa9df50d7d))
- update dependency com.google.cloud:google-cloud-datacatalog-bom to v1.64.0 ([#3681](https://github.com/googleapis/java-bigquery/issues/3681)) ([9e4e261](https://github.com/googleapis/java-bigquery/commit/9e4e26116226d17cc42ae030eed284bd6674b74b))
- update dependency com.google.cloud:google-cloud-datacatalog-bom to v1.65.0 ([#3704](https://github.com/googleapis/java-bigquery/issues/3704)) ([53b68b1](https://github.com/googleapis/java-bigquery/commit/53b68b13a505aa5d38e56032eaeb8c95bf3e9078))
- update dependency com.google.cloud:google-cloud-datacatalog-bom to v1.66.0 ([#3727](https://github.com/googleapis/java-bigquery/issues/3727)) ([7339f94](https://github.com/googleapis/java-bigquery/commit/7339f94cfa53d1c988f8ef051ddd5a2d7668d430))
- update dependency com.google.cloud:sdk-platform-java-config to v3.31.0 ([#3335](https://github.com/googleapis/java-bigquery/issues/3335)) ([0623455](https://github.com/googleapis/java-bigquery/commit/062345501c392c2a186c3cd82dee8d20ceda2a0a))
- update dependency com.google.cloud:sdk-platform-java-config to v3.32.0 ([#3360](https://github.com/googleapis/java-bigquery/issues/3360)) ([4420996](https://github.com/googleapis/java-bigquery/commit/4420996e89fef49270771bb4f01ffa4e871e7885))
- update dependency com.google.cloud:sdk-platform-java-config to v3.33.0 ([#3405](https://github.com/googleapis/java-bigquery/issues/3405)) ([a4a9999](https://github.com/googleapis/java-bigquery/commit/a4a9999def9805b8fecbc1820cc9f6f6c1997991))
- update dependency com.google.cloud:sdk-platform-java-config to v3.34.0 ([#3433](https://github.com/googleapis/java-bigquery/issues/3433)) ([801f441](https://github.com/googleapis/java-bigquery/commit/801f44172f7be43e0649a116fb0bb556507fc572))
- update dependency com.google.cloud:sdk-platform-java-config to v3.35.0 ([#3472](https://github.com/googleapis/java-bigquery/issues/3472)) ([fa9ac5d](https://github.com/googleapis/java-bigquery/commit/fa9ac5d73ec4f21ab7d12949e413b4ee9d11aa6d))
- update dependency com.google.cloud:sdk-platform-java-config to v3.36.0 ([#3490](https://github.com/googleapis/java-bigquery/issues/3490)) ([a72c582](https://github.com/googleapis/java-bigquery/commit/a72c5825c93f359d295fb78e0e541752f535876b))
- update dependency com.google.cloud:sdk-platform-java-config to v3.36.1 ([#3496](https://github.com/googleapis/java-bigquery/issues/3496)) ([8f2e5c5](https://github.com/googleapis/java-bigquery/commit/8f2e5c542760ecd7c217c36c80cb3b5aebee6a73))
- update dependency com.google.cloud:sdk-platform-java-config to v3.37.0 ([bf4d37a](https://github.com/googleapis/java-bigquery/commit/bf4d37a15f13ada3cf0045b2d45355193d2c2f34))
- update dependency com.google.cloud:sdk-platform-java-config to v3.38.0 ([#3542](https://github.com/googleapis/java-bigquery/issues/3542)) ([16448ee](https://github.com/googleapis/java-bigquery/commit/16448eec7c7f00a113c923a0fcde463c8ac91f9b))
- update dependency com.google.cloud:sdk-platform-java-config to v3.39.0 ([#3548](https://github.com/googleapis/java-bigquery/issues/3548)) ([616b2f6](https://github.com/googleapis/java-bigquery/commit/616b2f611f313994bf0ec2889daea3b569a84baf))
- update dependency com.google.cloud:sdk-platform-java-config to v3.40.0 ([#3576](https://github.com/googleapis/java-bigquery/issues/3576)) ([d5fa951](https://github.com/googleapis/java-bigquery/commit/d5fa951b8255ec1bcbcdf9bb3c29f247e38a0c7e))
- update dependency com.google.cloud:sdk-platform-java-config to v3.41.0 ([#3607](https://github.com/googleapis/java-bigquery/issues/3607)) ([11499d1](https://github.com/googleapis/java-bigquery/commit/11499d16727934fd3dfa5c18226e6f20471a11ac))
- update dependency com.google.cloud:sdk-platform-java-config to v3.41.1 ([#3628](https://github.com/googleapis/java-bigquery/issues/3628)) ([442d217](https://github.com/googleapis/java-bigquery/commit/442d217606b7d93d26887344a7a4a01303b18b8c))
- update dependency com.google.cloud:sdk-platform-java-config to v3.42.0 ([#3653](https://github.com/googleapis/java-bigquery/issues/3653)) ([1a14342](https://github.com/googleapis/java-bigquery/commit/1a143428c7f584db3dd6e827c2ee8fe980afe18c))
- update dependency com.google.cloud:sdk-platform-java-config to v3.43.0 ([#3669](https://github.com/googleapis/java-bigquery/issues/3669)) ([4d9e0ff](https://github.com/googleapis/java-bigquery/commit/4d9e0ff30269127f47484910e71fa7a21a735492))
- update dependency com.google.cloud:sdk-platform-java-config to v3.44.0 ([#3694](https://github.com/googleapis/java-bigquery/issues/3694)) ([f69fbd3](https://github.com/googleapis/java-bigquery/commit/f69fbd371f18da6ddc43d4f32f532e684026fe16))
- update dependency com.google.cloud:sdk-platform-java-config to v3.45.1 ([#3714](https://github.com/googleapis/java-bigquery/issues/3714)) ([e4512aa](https://github.com/googleapis/java-bigquery/commit/e4512aa5966e7b935fa55a062d940d9db0c834b3))
- update dependency com.google.cloud:sdk-platform-java-config to v3.46.0 ([#3753](https://github.com/googleapis/java-bigquery/issues/3753)) ([a335927](https://github.com/googleapis/java-bigquery/commit/a335927e16d0907d62e584f08fa8393daae40354))
- update dependency com.google.cloud:sdk-platform-java-config to v3.46.2 ([#3756](https://github.com/googleapis/java-bigquery/issues/3756)) ([907e39f](https://github.com/googleapis/java-bigquery/commit/907e39fd467f972863deeb86356fc3bfb989a76d))
- update dependency com.google.cloud:sdk-platform-java-config to v3.46.3 ([#3772](https://github.com/googleapis/java-bigquery/issues/3772)) ([ab166b6](https://github.com/googleapis/java-bigquery/commit/ab166b6c33c574b4494368709db0443e055b4863))
- update dependency com.google.cloud:sdk-platform-java-config to v3.47.0 ([#3779](https://github.com/googleapis/java-bigquery/issues/3779)) ([b27434b](https://github.com/googleapis/java-bigquery/commit/b27434b8a75e74184458e920142f5575fed9ba52))
- update dependency com.google.cloud:sdk-platform-java-config to v3.48.0 ([#3790](https://github.com/googleapis/java-bigquery/issues/3790)) ([206f06d](https://github.com/googleapis/java-bigquery/commit/206f06de115ead53b26f09a5f4781efd279b5a73))
- update dependency com.google.cloud:sdk-platform-java-config to v3.49.0 ([#3811](https://github.com/googleapis/java-bigquery/issues/3811)) ([2c5ede4](https://github.com/googleapis/java-bigquery/commit/2c5ede4b115cf7cdd078d54d29ce93636c1cedf5))
- update dependency com.google.cloud:sdk-platform-java-config to v3.49.2 ([#3853](https://github.com/googleapis/java-bigquery/issues/3853)) ([cf864df](https://github.com/googleapis/java-bigquery/commit/cf864df739bbb820e99999b7c1592a3635fea4ec))
- update dependency com.google.cloud:sdk-platform-java-config to v3.50.0 ([#3861](https://github.com/googleapis/java-bigquery/issues/3861)) ([eb26dee](https://github.com/googleapis/java-bigquery/commit/eb26deee37119389aee3962eea5ad67d63f26c70))
- update dependency com.google.cloud:sdk-platform-java-config to v3.50.1 ([#3878](https://github.com/googleapis/java-bigquery/issues/3878)) ([0e971b8](https://github.com/googleapis/java-bigquery/commit/0e971b8ace013caa31b8a02a21038e94bebae2a5))
- update dependency com.google.cloud:sdk-platform-java-config to v3.50.2 ([#3901](https://github.com/googleapis/java-bigquery/issues/3901)) ([8205623](https://github.com/googleapis/java-bigquery/commit/82056237f194a6c99ec4fb3a4315023efdedff1b))
- update dependency com.google.cloud:sdk-platform-java-config to v3.51.0 ([#3924](https://github.com/googleapis/java-bigquery/issues/3924)) ([cb66be5](https://github.com/googleapis/java-bigquery/commit/cb66be596d1bfd0a5aed75f5a0e36d80269c7f6a))
- update dependency com.google.cloud:sdk-platform-java-config to v3.52.0 ([#3939](https://github.com/googleapis/java-bigquery/issues/3939)) ([794bf83](https://github.com/googleapis/java-bigquery/commit/794bf83e84efc0712638bebde5158777b9c89397))
- update dependency com.google.cloud:sdk-platform-java-config to v3.52.1 ([#3952](https://github.com/googleapis/java-bigquery/issues/3952)) ([79b7557](https://github.com/googleapis/java-bigquery/commit/79b7557501d318fd92b90a681036fe6a1aa1bac4))
- update dependency com.google.cloud:sdk-platform-java-config to v3.52.2 ([#3964](https://github.com/googleapis/java-bigquery/issues/3964)) ([6775fce](https://github.com/googleapis/java-bigquery/commit/6775fce537df9c5f4d0b1488ce28591f6aed195f))
- update dependency com.google.cloud:sdk-platform-java-config to v3.52.3 ([#3971](https://github.com/googleapis/java-bigquery/issues/3971)) ([f8cf508](https://github.com/googleapis/java-bigquery/commit/f8cf50833772412c4f15922bffcdf5100792948d))
- update dependency com.google.cloud:sdk-platform-java-config to v3.53.0 ([#3980](https://github.com/googleapis/java-bigquery/issues/3980)) ([a961247](https://github.com/googleapis/java-bigquery/commit/a961247e9546a9fce8da1609afd18975142c2379))
- update dependency com.google.cloud:sdk-platform-java-config to v3.54.1 ([#3994](https://github.com/googleapis/java-bigquery/issues/3994)) ([4e09f6b](https://github.com/googleapis/java-bigquery/commit/4e09f6bc7a25904ad8f61141a0837535d39dbb4e))
- update dependency com.google.oauth-client:google-oauth-client-java6 to v1.36.0 ([#3305](https://github.com/googleapis/java-bigquery/issues/3305)) ([d05e554](https://github.com/googleapis/java-bigquery/commit/d05e5547e97f52ccfdcec1d6fe167e6587dd00c6))
- update dependency com.google.oauth-client:google-oauth-client-java6 to v1.37.0 ([#3614](https://github.com/googleapis/java-bigquery/issues/3614)) ([f5faa69](https://github.com/googleapis/java-bigquery/commit/f5faa69bc5b6fdae137724df5693f8aecf27d609))
- update dependency com.google.oauth-client:google-oauth-client-java6 to v1.38.0 ([#3685](https://github.com/googleapis/java-bigquery/issues/3685)) ([53bd7af](https://github.com/googleapis/java-bigquery/commit/53bd7af47783674a3accbadb1172edbcf628ab2b))
- update dependency com.google.oauth-client:google-oauth-client-java6 to v1.39.0 ([#3710](https://github.com/googleapis/java-bigquery/issues/3710)) ([c0c6352](https://github.com/googleapis/java-bigquery/commit/c0c6352b8d02145fe9513e3e23d316e045360d2d))
- update dependency com.google.oauth-client:google-oauth-client-jetty to v1.36.0 ([#3306](https://github.com/googleapis/java-bigquery/issues/3306)) ([0eeed66](https://github.com/googleapis/java-bigquery/commit/0eeed668b5f88f9c59ef6c1b309e7a81f5c1f0e9))
- update dependency com.google.oauth-client:google-oauth-client-jetty to v1.37.0 ([#3615](https://github.com/googleapis/java-bigquery/issues/3615)) ([a6c7944](https://github.com/googleapis/java-bigquery/commit/a6c79443a5e675a01ecb91e362e261a6f6ecc055))
- update dependency com.google.oauth-client:google-oauth-client-jetty to v1.38.0 ([#3686](https://github.com/googleapis/java-bigquery/issues/3686)) ([d71b2a3](https://github.com/googleapis/java-bigquery/commit/d71b2a34a728fb6ee1c88cdc895b87959e230b7a))
- update dependency com.google.oauth-client:google-oauth-client-jetty to v1.39.0 ([#3711](https://github.com/googleapis/java-bigquery/issues/3711)) ([43b86e9](https://github.com/googleapis/java-bigquery/commit/43b86e91a664dd9d3edaea7b31b46ac635fb22b0))
- update dependency io.opentelemetry:opentelemetry-api to v1.52.0 ([#3902](https://github.com/googleapis/java-bigquery/issues/3902)) ([772407b](https://github.com/googleapis/java-bigquery/commit/772407b12f4da005f79eafc944d4c53f0eec5c27))
- update dependency io.opentelemetry:opentelemetry-bom to v1.51.0 ([#3840](https://github.com/googleapis/java-bigquery/issues/3840)) ([51321c2](https://github.com/googleapis/java-bigquery/commit/51321c22778fd41134cc0cdfc70bdc47f05883f1))
- update dependency io.opentelemetry:opentelemetry-bom to v1.52.0 ([#3903](https://github.com/googleapis/java-bigquery/issues/3903)) ([509a6fc](https://github.com/googleapis/java-bigquery/commit/509a6fc0bb7e7a101bf0d4334a3ff1adde2cab09))
- update dependency io.opentelemetry:opentelemetry-context to v1.52.0 ([#3904](https://github.com/googleapis/java-bigquery/issues/3904)) ([96c1bae](https://github.com/googleapis/java-bigquery/commit/96c1bae0fcdfdfc2dbb25dcae5007c5d02111a8c))
- update dependency io.opentelemetry:opentelemetry-exporter-logging to v1.52.0 ([#3905](https://github.com/googleapis/java-bigquery/issues/3905)) ([28ee4c9](https://github.com/googleapis/java-bigquery/commit/28ee4c941b99b1fe3803aefbe7a8ae57100d76cb))
- update dependency node to v22 ([#3713](https://github.com/googleapis/java-bigquery/issues/3713)) ([251def5](https://github.com/googleapis/java-bigquery/commit/251def5659d2648dff0833ba967a65435e11b643))
- update dependency org.graalvm.buildtools:junit-platform-native to v0.10.2 ([#3311](https://github.com/googleapis/java-bigquery/issues/3311)) ([3912a92](https://github.com/googleapis/java-bigquery/commit/3912a9232788e09c10fc4e91ef6d65514fc106e4))
- update dependency org.graalvm.buildtools:native-maven-plugin to v0.10.2 ([#3312](https://github.com/googleapis/java-bigquery/issues/3312)) ([9737a5d](https://github.com/googleapis/java-bigquery/commit/9737a5d63d545ed197879bbd9dbfd3f1dbc15d93))
- update dependency org.junit.vintage:junit-vintage-engine to v5.10.3 ([#3371](https://github.com/googleapis/java-bigquery/issues/3371)) ([2e804c5](https://github.com/googleapis/java-bigquery/commit/2e804c56eeef5009cc46c7544fe9b04bfdd65d7a))
- update dependency ubuntu to v24 ([#3498](https://github.com/googleapis/java-bigquery/issues/3498)) ([4f87ade](https://github.com/googleapis/java-bigquery/commit/4f87adec6c010b572675f98b651f88d14323e2e2))
- update github/codeql-action action to v2.25.10 ([#3348](https://github.com/googleapis/java-bigquery/issues/3348)) ([8b6feff](https://github.com/googleapis/java-bigquery/commit/8b6feffa0e8add73a7587ce1762989713c2af38b))
- update github/codeql-action action to v2.25.11 ([#3376](https://github.com/googleapis/java-bigquery/issues/3376)) ([f1e0014](https://github.com/googleapis/java-bigquery/commit/f1e0014dca5ca04522796b44ff313696d2b41176))
- update github/codeql-action action to v2.25.12 ([#3387](https://github.com/googleapis/java-bigquery/issues/3387)) ([af60b30](https://github.com/googleapis/java-bigquery/commit/af60b30cd774992c5d82063106471926dc6aaa6e))
- update github/codeql-action action to v2.25.13 ([#3395](https://github.com/googleapis/java-bigquery/issues/3395)) ([95c8d6f](https://github.com/googleapis/java-bigquery/commit/95c8d6f65c5c5355fc52a0a2b54002d8f9cdb1ef))
- update github/codeql-action action to v2.25.15 ([#3402](https://github.com/googleapis/java-bigquery/issues/3402)) ([a61ce7d](https://github.com/googleapis/java-bigquery/commit/a61ce7d710e2e8b000ee25ec9d295abbc2b63dd1))
- update github/codeql-action action to v2.25.6 ([#3307](https://github.com/googleapis/java-bigquery/issues/3307)) ([8999d33](https://github.com/googleapis/java-bigquery/commit/8999d337b92d7030825c5a36686ddd082cadc816))
- update github/codeql-action action to v2.25.7 ([#3334](https://github.com/googleapis/java-bigquery/issues/3334)) ([768342d](https://github.com/googleapis/java-bigquery/commit/768342da168921251c34163b51ffc3cddfefc0ce))
- update github/codeql-action action to v2.25.8 ([#3338](https://github.com/googleapis/java-bigquery/issues/3338)) ([8673fe5](https://github.com/googleapis/java-bigquery/commit/8673fe55e6d33e50c32a520a848cddc25eb6088e))
- update github/codeql-action action to v2.26.10 ([#3506](https://github.com/googleapis/java-bigquery/issues/3506)) ([ca71294](https://github.com/googleapis/java-bigquery/commit/ca712948b1adfb26bb1f9ef2250be10fe45d3424))
- update github/codeql-action action to v2.26.11 ([#3517](https://github.com/googleapis/java-bigquery/issues/3517)) ([ac736bb](https://github.com/googleapis/java-bigquery/commit/ac736bb50bf4b2e629dcbfe7de90b846e07038e4))
- update github/codeql-action action to v2.26.12 ([#3522](https://github.com/googleapis/java-bigquery/issues/3522)) ([fdf8dc4](https://github.com/googleapis/java-bigquery/commit/fdf8dc4b7cb4e26939da10002e47c810d71bad6c))
- update github/codeql-action action to v2.26.13 ([#3536](https://github.com/googleapis/java-bigquery/issues/3536)) ([844744f](https://github.com/googleapis/java-bigquery/commit/844744f3dea804a31abc806592f557a26cffbab4))
- update github/codeql-action action to v2.26.2 ([#3426](https://github.com/googleapis/java-bigquery/issues/3426)) ([0a6574f](https://github.com/googleapis/java-bigquery/commit/0a6574fa11aa83b5c899f1dcd3b1132aa4f46ebd))
- update github/codeql-action action to v2.26.3 ([#3438](https://github.com/googleapis/java-bigquery/issues/3438)) ([390e182](https://github.com/googleapis/java-bigquery/commit/390e1824bffef17e85d0ec142b4fcca6dff80a9c))
- update github/codeql-action action to v2.26.5 ([#3446](https://github.com/googleapis/java-bigquery/issues/3446)) ([58aacc5](https://github.com/googleapis/java-bigquery/commit/58aacc5a92e18b790a03c0b9b4a75062928768c2))
- update github/codeql-action action to v2.26.6 ([#3464](https://github.com/googleapis/java-bigquery/issues/3464)) ([2aeb44d](https://github.com/googleapis/java-bigquery/commit/2aeb44d8b2ff5fa264cb14a8fc31e9494d77cb6b))
- update github/codeql-action action to v2.26.7 ([#3482](https://github.com/googleapis/java-bigquery/issues/3482)) ([e2c94b6](https://github.com/googleapis/java-bigquery/commit/e2c94b601781ebe236c25cd3f40059e7543ba387))
- update github/codeql-action action to v2.26.8 ([#3488](https://github.com/googleapis/java-bigquery/issues/3488)) ([a6d75de](https://github.com/googleapis/java-bigquery/commit/a6d75de60b822dcc5433afab55b5d392e6a6caf5))
- update github/codeql-action action to v2.26.9 ([#3494](https://github.com/googleapis/java-bigquery/issues/3494)) ([8154043](https://github.com/googleapis/java-bigquery/commit/815404319a43a8a14d1d8aaa8ab22dd924b48175))
- update github/codeql-action action to v2.27.0 ([#3540](https://github.com/googleapis/java-bigquery/issues/3540)) ([1616a0f](https://github.com/googleapis/java-bigquery/commit/1616a0f6057916e21f3b4a6d418d1431d8d1fa16))
- update github/codeql-action action to v2.27.1 ([#3567](https://github.com/googleapis/java-bigquery/issues/3567)) ([e154ee3](https://github.com/googleapis/java-bigquery/commit/e154ee300485dc9d900343a8b5ceb7f6633bc3ff))
- update github/codeql-action action to v2.27.3 ([#3569](https://github.com/googleapis/java-bigquery/issues/3569)) ([3707a40](https://github.com/googleapis/java-bigquery/commit/3707a402039365c49e1976a388593f621231dc02))
- update github/codeql-action action to v2.27.4 ([#3572](https://github.com/googleapis/java-bigquery/issues/3572)) ([2c7b4f7](https://github.com/googleapis/java-bigquery/commit/2c7b4f750f4c8bf03c0ba74402d745341382a209))
- update github/codeql-action action to v2.27.5 ([#3588](https://github.com/googleapis/java-bigquery/issues/3588)) ([3f94075](https://github.com/googleapis/java-bigquery/commit/3f9407570fea5317aaf212b058ca1da05985eda9))
- update github/codeql-action action to v2.27.6 ([#3597](https://github.com/googleapis/java-bigquery/issues/3597)) ([bc1f3b9](https://github.com/googleapis/java-bigquery/commit/bc1f3b97a0c8ccc6e93a07b2f0ebcf8e05da9b48))
- update github/codeql-action action to v2.27.7 ([#3603](https://github.com/googleapis/java-bigquery/issues/3603)) ([528426b](https://github.com/googleapis/java-bigquery/commit/528426bf9b7801b1b9b45758b560f14a4c9bbc57))
- update github/codeql-action action to v2.27.9 ([#3608](https://github.com/googleapis/java-bigquery/issues/3608)) ([567ce01](https://github.com/googleapis/java-bigquery/commit/567ce01ed77d44760ddcd872a0d61abdd6a09832))
- update github/codeql-action action to v2.28.0 ([#3621](https://github.com/googleapis/java-bigquery/issues/3621)) ([e0e09ec](https://github.com/googleapis/java-bigquery/commit/e0e09ec4954f5b5e2f094e4c67600f38353f453c))
- update github/codeql-action action to v2.28.1 ([#3637](https://github.com/googleapis/java-bigquery/issues/3637)) ([858e517](https://github.com/googleapis/java-bigquery/commit/858e51792d98276f10fd780ef6edd0bb4a1b4f54))
- update netty.version to v4.1.119.final ([#3717](https://github.com/googleapis/java-bigquery/issues/3717)) ([08a290a](https://github.com/googleapis/java-bigquery/commit/08a290adcfa7551ee27a58da0eaf5ac00a759b90))
- update netty.version to v4.2.0.final ([#3745](https://github.com/googleapis/java-bigquery/issues/3745)) ([bb811c0](https://github.com/googleapis/java-bigquery/commit/bb811c068b3efabf04fbe67dbb2979d562c604d9))
- update netty.version to v4.2.1.final ([#3780](https://github.com/googleapis/java-bigquery/issues/3780)) ([6dcd858](https://github.com/googleapis/java-bigquery/commit/6dcd858eca788a8cb571368e12b4925993e380c4))
- update ossf/scorecard-action action to v2.4.0 ([#3408](https://github.com/googleapis/java-bigquery/issues/3408)) ([66777a2](https://github.com/googleapis/java-bigquery/commit/66777a2c3c7b0462330bd1c820e2f04ad4727465))
- update ossf/scorecard-action action to v2.4.1 ([#3690](https://github.com/googleapis/java-bigquery/issues/3690)) ([cdb61fe](https://github.com/googleapis/java-bigquery/commit/cdb61febcb1a64f6ddd3c0e3c29fa7995f1d3fa5))
- update ossf/scorecard-action action to v2.4.2 ([#3810](https://github.com/googleapis/java-bigquery/issues/3810)) ([414f61d](https://github.com/googleapis/java-bigquery/commit/414f61d7efcfa568c1446bd41945d7a8e2450649))
- update sdk-platform-java-config to 3.55.0-rc1 ([#4033](https://github.com/googleapis/java-bigquery/issues/4033)) ([580427d](https://github.com/googleapis/java-bigquery/commit/580427d7adfba3f11fb9c310c8723d3d66a72a69))

##### Documentation

- add short mode query sample ([#3397](https://github.com/googleapis/java-bigquery/issues/3397)) ([6dca6ff](https://github.com/googleapis/java-bigquery/commit/6dca6fffe96937db87713e45f0501d64fd5b544f))
- add simple query connection read api sample ([#3394](https://github.com/googleapis/java-bigquery/issues/3394)) ([d407baa](https://github.com/googleapis/java-bigquery/commit/d407baa3e95ad894d4028aa46def7ca8efe930c3))
- **bigquery:** Add javadoc description of timestamp() parameter. ([#3604](https://github.com/googleapis/java-bigquery/issues/3604)) ([6ee0c10](https://github.com/googleapis/java-bigquery/commit/6ee0c103771ef678f66cc7a584bdce27e21f29c4))
- **bigquery:** Update TableResult.getTotalRows() docstring ([#3785](https://github.com/googleapis/java-bigquery/issues/3785)) ([6483588](https://github.com/googleapis/java-bigquery/commit/6483588a3c5785b95ea841f21aa38f50ecf4226d))
- fix BigQuery documentation formatting ([#3565](https://github.com/googleapis/java-bigquery/issues/3565)) ([552f491](https://github.com/googleapis/java-bigquery/commit/552f49132af370f66aa1ccdde86e6280f638da22))
- reformat javadoc ([#3545](https://github.com/googleapis/java-bigquery/issues/3545)) ([4763f73](https://github.com/googleapis/java-bigquery/commit/4763f73ad854ca4bfdddbbdc0bb43fe639238665))
- update CONTRIBUTING.md for users without branch permissions ([#3670](https://github.com/googleapis/java-bigquery/issues/3670)) ([009b9a2](https://github.com/googleapis/java-bigquery/commit/009b9a2b3940ab66220e68ddd565710b8552cc45))
- update error handling comment to be more precise in samples ([#3712](https://github.com/googleapis/java-bigquery/issues/3712)) ([9eb555f](https://github.com/googleapis/java-bigquery/commit/9eb555ff61bef42a3bdfe197da8423b7bf14f493))
- update iam policy sample user to be consistent with other languages ([#3429](https://github.com/googleapis/java-bigquery/issues/3429)) ([2fc15b3](https://github.com/googleapis/java-bigquery/commit/2fc15b3e9f89289f0a047bb0a6ae7fb5bb71d253))
- update maven format command ([#3877](https://github.com/googleapis/java-bigquery/issues/3877)) ([d2918da](https://github.com/googleapis/java-bigquery/commit/d2918da844cd20ca1602c6fcf9fa1df685f261fc))
- Update SimpleApp to explicitly set project id ([#3534](https://github.com/googleapis/java-bigquery/issues/3534)) ([903a0f7](https://github.com/googleapis/java-bigquery/commit/903a0f7db0926f3d166eebada1710413056fb4a2))

## December 19, 2025

Feature The BigQuery Data Transfer Service can now [transfer data from MySQL to
BigQuery](https://docs.cloud.google.com/bigquery/docs/mysql-transfer). This feature is [generally
available](https://cloud.google.com/products/#product-launch-stages) (GA).
Feature The BigQuery Data Transfer Service can now [transfer data from Microsoft SQL
Server to BigQuery](https://docs.cloud.google.com/bigquery/docs/sqlserver-transfer). This feature is in
[Preview](https://cloud.google.com/products/#product-launch-stages).

## December 18, 2025

Feature The BigQuery Data Transfer Service can now transfer data from the following data
sources to BigQuery:

- [Klaviyo](https://docs.cloud.google.com/bigquery/docs/klaviyo-transfer)
- [HubSpot](https://docs.cloud.google.com/bigquery/docs/hubspot-transfer)

These features are in [Preview](https://cloud.google.com/products/#product-launch-stages).
Feature You can now use the BigQuery Data Transfer Service [to transfer data from blob
storage sources](https://docs.cloud.google.com/bigquery/docs/iceberg-ingestion), such as Amazon Simple
Storage Service (Amazon S3), Azure Blob Storage, and Cloud Storage, into BigLake
Iceberg tables in BigQuery. This feature is in
[Preview](https://cloud.google.com/products/#product-launch-stages).

## December 16, 2025

Feature The BigQuery Data Transfer Service can now [transfer data from Oracle to
BigQuery](https://docs.cloud.google.com/bigquery/docs/oracle-transfer). This feature is [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).

## December 15, 2025

Libraries

### Java

#### [2.57.0](https://github.com/googleapis/java-bigquery/compare/v2.56.0...v2.57.0) (2025-12-11)

##### Features

- Add timestamp_precision to Field ([#4014](https://github.com/googleapis/java-bigquery/issues/4014)) ([57ffe1d](https://github.com/googleapis/java-bigquery/commit/57ffe1d2ba8af3b950438c926d66ac23ca8a3093))
- Introduce DataFormatOptions to configure the output of BigQuery data types ([#4010](https://github.com/googleapis/java-bigquery/issues/4010)) ([6dcc900](https://github.com/googleapis/java-bigquery/commit/6dcc90053353422ae766e531413b3ecc65b8b155))
- Relax client-side validation for BigQuery entity IDs ([#4000](https://github.com/googleapis/java-bigquery/issues/4000)) ([c3548a2](https://github.com/googleapis/java-bigquery/commit/c3548a2f521b19761c844c0b24fc8caab541aba7))

##### Dependencies

- Update dependency com.google.cloud:sdk-platform-java-config to v3.54.2 ([#4022](https://github.com/googleapis/java-bigquery/issues/4022)) ([d2f2057](https://github.com/googleapis/java-bigquery/commit/d2f20579fd60efc36fa4239619e0d679a914cd6d))

### Java

#### [2.57.1](https://github.com/googleapis/java-bigquery/compare/v2.57.0...v2.57.1) (2025-12-12)

##### Dependencies

- Update actions/upload-artifact action to v6 ([#4027](https://github.com/googleapis/java-bigquery/issues/4027)) ([5d389cf](https://github.com/googleapis/java-bigquery/commit/5d389cf45b41a0edceb3c5ed98dd2421ba6f2234))

## December 10, 2025

Feature You can now use the [BigQuery remote MCP server](https://docs.cloud.google.com/bigquery/docs/use-bigquery-mcp)
to enable LLM agents to perform a range of data-related tasks.

This feature is in
[Preview](https://cloud.google.com/products/#product-launch-stages).

## December 02, 2025

Change An updated version of the
[ODBC driver for BigQuery](https://docs.cloud.google.com/bigquery/docs/reference/odbc-jdbc-drivers#current_odbc_driver)
is now available.
Feature You can now enable
[autonomous embedding generation](https://docs.cloud.google.com/bigquery/docs/autonomous-embedding-generation)
on tables that you make with the
[`CREATE TABLE` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_table_statement).
When you do this, BigQuery maintains a column of embeddings on
the table based on a source column. When you add or modify data in the source
column, BigQuery automatically generates or updates the embedding
column for that data.

You can also use the
[`AI.SEARCH`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-search)
function, enabling semantic search on tables that have autonomous embedding
generation enabled.

These features are in
[Preview](https://cloud.google.com/products/#product-launch-stages).

## December 01, 2025

Feature Search results in the [Explorer
pane](https://docs.cloud.google.com/bigquery/docs/bigquery-web-ui#explorer_pane) in BigQuery Studio now show
results in the current organization. You can use a drop-down menu to switch
between organizations. This feature is [generally
available](https://cloud.google.com/products/#product-launch-stages) (GA).

## November 26, 2025

Feature The BigQuery Data Transfer Service now supports [incremental data transfers](https://docs.cloud.google.com/bigquery/docs/salesforce-transfer#full_or_incremental_transfers)
when transferring data from Salesforce to BigQuery. This feature is supported in
[Preview](https://cloud.google.com/products/#product-launch-stages).

## November 25, 2025

Change An updated version of the
[JDBC driver for BigQuery](https://docs.cloud.google.com/bigquery/docs/reference/odbc-jdbc-drivers#current_jdbc_driver)
is now available.

## November 24, 2025

Feature You can
[set the default project and dataset for your pipeline](https://docs.cloud.google.com/bigquery/docs/create-pipelines#sqlx-options)
in the **SQLX options** section, which simplifies task configuration by using
these defaults for all tasks. This feature is
[generally available](https://cloud.google.com/products/#product-launch-stages)
(GA).

## November 20, 2025

Feature You can now use the BigQuery Agent Analytics plugin within the Agent Development
Kit to export agent interaction data directly into BigQuery. This plugin
captures comprehensive logs of your agent's prompts, tool usage, and responses,
enabling you to analyze and visualize agent performance metrics. The plugin
leverages the BigQuery Storage Write API for efficient high-throughput
streaming. For more information on how to leverage this plugin in your agent,
see the
[Announcing BigQuery Agent Analytics for the Google ADK](https://cloud.google.com/blog/products/data-analytics/introducing-bigquery-agent-analytics/?e=48754805).

## November 19, 2025

Feature You can use the
[`JSON_FLATTEN` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#json_flatten)
to extract all non-array values that are either directly in the input `JSON`
value or children of one or more consecutively nested arrays in the input
`JSON` value. This function is available in
[Preview](https://cloud.google.com/products/#product-launch-stages).
Feature You can now use Gemini in BigQuery to
[fix and explain errors](https://docs.cloud.google.com/bigquery/docs/write-sql-gemini#fix_and_explain_sql_errors)
in your SQL queries. This feature is in
[Preview](https://cloud.google.com/products/#product-launch-stages).

## November 18, 2025

Feature You can now use [Gemini 3.0](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/models/gemini/3-pro)
when you call generative AI functions in BigQuery,
such as [`AI.GENERATE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-generate).
You must use the full global endpoint argument:

`https://aiplatform.googleapis.com/v1/projects/PROJECT_ID/locations/global/publishers/google/models/gemini-3-pro-preview`.
Feature Dataform now lets you automate the creation of
[BigLake tables for Apache Iceberg in BigQuery](https://docs.cloud.google.com/dataform/docs/create-tables#create-iceberg-table).
This feature is
[generally available](https://cloud.google.com/products#product-launch-stages)
(GA).
Feature BigQuery ML now supports the following
[generative AI functions](https://docs.cloud.google.com/bigquery/docs/generative-ai-overview):

- [`AI.GENERATE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-generate): generate free text to accomplish a wide range of tasks, such as translation, summarization, and classification, on any unstructured data, including images, audio, video, and documents. It can also perform entity extraction and generate structured output. This function is [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).
- [`AI.EMBED`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-embed): turn text, image, audio, video, or documents into embeddings. This function is in [Preview](https://cloud.google.com/products/#product-launch-stages).
- [`AI.SIMILARITY`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-similarity): compute the semantic similarity between pairs of text, pairs of images, or across text and images. This function is in [Preview](https://cloud.google.com/products/#product-launch-stages).
- You can use the [`AI.GENERATE_BOOL`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-generate-bool), [`AI.GENERATE_DOUBLE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-generate-double), and [`AI.GENERATE_INT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-generate-int) functions to generate scalar values, which are convenient for filtering, scoring, and counting purposes.
- Each of these functions supports [authentication with end-user credentials (EUC)](https://docs.cloud.google.com/bigquery/docs/permissions-for-ai-functions#run_generative_ai_queries_with_end-user_credentials) to set up the necessary Vertex AI permissions.

BigQuery ML now supports the following table-valued generative AI functions:

- [`AI.GENERATE_TABLE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-generate-table): generate a table of structured output from unstructured data including text, images, audio, and video.
- [`AI.GENERATE_TEXT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-generate-text) is the new, preferred version of `ML.GENERATE_TEXT`, which has the same functionality but with simplified column output names.
- [`AI.GENERATE_EMBEDDING`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-generate-embedding) is the new, preferred version of `ML.GENERATE_EMBEDDING`, which has the same functionality but with simplified column output names.
- These functions are all [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).
Feature You can now [publish data insights](https://docs.cloud.google.com/bigquery/docs/data-insights#modes),
including query recommendations and
auto-generated table and column descriptions, to the Dataplex Universal Catalog.
This feature is in
[Preview](https://cloud.google.com/products/#product-launch-stages).

## November 17, 2025

Feature You can use [folders](https://docs.cloud.google.com/bigquery/docs/code-asset-folders) to organize and control
access to single file code assets, such as notebooks, saved queries, data
canvases, and data preparation files. This feature is in
[Preview](https://cloud.google.com/products/#product-launch-stages).
Feature In the query execution graph, you can now use the [query text
heatmap](https://docs.cloud.google.com/bigquery/docs/query-plan-explanation#query_text_heatmap) to identify
which query text contributes to stages that consume more slot time, and to see
query plan details for those stages. This feature is in
[Preview](https://cloud.google.com/products/#product-launch-stages).
Feature You can now
[share SQL stored procedures in BigQuery sharing listings](https://docs.cloud.google.com/bigquery/docs/analytics-hub-manage-listings#share-stored-procedure-in-listing)
and enable
[role-based authorization for stored procedures](https://docs.cloud.google.com/bigquery/docs/authorized-routines#authorize_routines).
These features are in [preview](https://cloud.google.com/products#product-launch-stages).
Libraries

### Java

#### [2.56.0](https://github.com/googleapis/java-bigquery/compare/v2.55.3...v2.56.0) (2025-11-15)

##### Features

- New queryWithTimeout method for customer-side wait ([#3995](https://github.com/googleapis/java-bigquery/issues/3995)) ([9c0df54](https://github.com/googleapis/java-bigquery/commit/9c0df5422c05696f7ce4bedf914a58306150dc21))

##### Dependencies

- Update dependency com.google.apis:google-api-services-bigquery to v2-rev20251012-2.0.0 ([#3923](https://github.com/googleapis/java-bigquery/issues/3923)) ([1d8977d](https://github.com/googleapis/java-bigquery/commit/1d8977df3b1451378e5471cce9fd8b067f80fc9a))
- Update dependency com.google.cloud:sdk-platform-java-config to v3.54.1 ([#3994](https://github.com/googleapis/java-bigquery/issues/3994)) ([4e09f6b](https://github.com/googleapis/java-bigquery/commit/4e09f6bc7a25904ad8f61141a0837535d39dbb4e))

## November 11, 2025

Feature The BigQuery [Overview page](https://docs.cloud.google.com/bigquery/docs/bigquery-web-ui#open-overview)
is now your hub for discovering tutorials, features, and resources to help you
get the most out of BigQuery. It provides guided paths for users
of all skill levels. This feature is in
[Preview](https://cloud.google.com/products/#product-launch-stages).
Feature You can now use the
[interactive SQL translator](https://docs.cloud.google.com/bigquery/docs/interactive-sql-translator), the
[translation API](https://docs.cloud.google.com/bigquery/docs/api-sql-translator), and the
[batch SQL translator](https://docs.cloud.google.com/bigquery/docs/batch-sql-translator) to translate the
following SQL dialects into GoogleSQL:

- Apache Impala SQL
- GoogleSQL (BigQuery)

Impala SQL translation can be used to migrate Cloudera and Apache Hadoop SQL
workloads that use Impala as a query engine.

GoogleSQL (BigQuery) translation can be used to verify and iteratively customize
your translated SQL queries after an initial translation from an external
dialect. For example, you can apply systematic query rewrites using [YAML
configurations](https://docs.cloud.google.com/bigquery/docs/config-yaml-translation#optimize_and_improve_the_performance_of_translated_sql)
to customize and optimize your GoogleSQL queries before deploying it.

These features are in [Preview](https://cloud.google.com/products/#product-launch-stages).
Feature You can now use [custom constraints](https://docs.cloud.google.com/bigquery/docs/custom-constraints) with an
Organization Policy to provide more granular control over specific fields for
BigQuery dataset resources. This feature is [generally available](https://docs.cloud.google.com/products#product-launch-stages) (GA).

## November 10, 2025

Feature You can [aggregate](https://docs.cloud.google.com/bigquery/docs/data-prep-get-suggestions#aggregate)
and [deduplicate](https://docs.cloud.google.com/bigquery/docs/data-prep-get-suggestions#deduplicate)
table data with Gemini assistance in your
[BigQuery data preparations](https://docs.cloud.google.com/bigquery/docs/data-prep-introduction).
These features are [generally available](https://docs.cloud.google.com/products#product-launch-stages) (GA).
Feature Partitioning is now available for
[BigLake tables for Apache Iceberg in BigQuery](https://docs.cloud.google.com/bigquery/docs/iceberg-tables#use_partitioning).
This feature is in
[Preview](https://cloud.google.com/products/#product-launch-stages).
Feature BigQuery ML now supports the `TimesFM 2.5`
[time series foundational model](https://docs.cloud.google.com/bigquery/docs/timesfm-model).
You can use the `TimesFM 2.5` model in the
[`AI.FORECAST`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-forecast),
[`AI.EVALUATE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-evaluate),
and
[`AI.DETECT_ANOMALIES`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-detect-anomalies)
functions to achieve better forecasting accuracy and lower latency.
Feature BigQuery ML now offers the
[`AI.DETECT_ANOMALIES` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-detect-anomalies).
Use the `AI.DETECT_ANOMALIES` function with a TimesFM model to
[detect anomalies](https://docs.cloud.google.com/bigquery/docs/anomaly-detection-overview)
in time series data, using historical data as a baseline.
This feature is in
[Preview](https://cloud.google.com/products/#product-launch-stages).

## November 06, 2025

Announcement The research paper [ARIMA_PLUS: Large-scale, Accurate, Automatic and
Interpretable In-Database Time Series Forecasting and Anomaly Detection in
Google BigQuery](https://arxiv.org/abs/2510.24452) is now publicly available.
This paper describes the algorithms behind the
[`ARIMA_PLUS`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-time-series)
and
[`ARIMA_PLUS_XREG`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-multivariate-time-series)
models for time series forecasting and anomaly detection, and demonstrates the
high performance, scalability, explainability, and customizability of the
models.

## November 05, 2025

Feature You can use the
[`MATCH_RECOGNIZE` clause](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#match_recognize_clause)
in your SQL queries to filter and aggregate matches across rows in a table.
This feature is
[generally available](https://cloud.google.com/products/#product-launch-stages)
(GA).
Announcement The [BigQuery Data Transfer Service for Google Ads](https://docs.cloud.google.com/bigquery/docs/google-ads-transfer) now supports [Google Ads API v21](https://developers.google.com/google-ads/api/fields/v21/overview).
Feature You can now
[generate table and column descriptions](https://docs.cloud.google.com/bigquery/docs/data-insights#generate-column-table-descriptions)
in all supported Gemini languages when you generate data insights.
This feature is
[generally available](https://cloud.google.com/products/#product-launch-stages)
(GA).
Feature You can now generate [data insights](https://docs.cloud.google.com/bigquery/docs/data-insights#rest) when you
create a
[`DataScan`](https://docs.cloud.google.com/dataplex/docs/reference/rest/v1/projects.locations.dataScans)
using the Dataplex API. This feature is
[generally available](https://cloud.google.com/products/#product-launch-stages)
(GA).

## November 04, 2025

Feature You can now use [custom organization policies with the BigQuery migration
service](https://docs.cloud.google.com/bigquery/docs/migration-custom-org-policies) to allow or deny specific
operations during a BigQuery migration to meet your organization's compliance
and security requirements. This includes an option to disable AI suggestions
during a migration. This feature is in
[Preview](https://cloud.google.com/products/#product-launch-stages).

## November 03, 2025

Libraries

### Go

#### [1.72.0](https://github.com/googleapis/google-cloud-go/compare/bigquery/v1.71.0...bigquery/v1.72.0) (2025-10-28)

##### Features

- **bigquery/reservation:** Add new `BACKGROUND_CHANGE_DATA_CAPTURE`, `BACKGROUND_COLUMN_METADATA_INDEX`, and `BACKGROUND_SEARCH_INDEX_REFRESH` reservation assignment types ([182df61](https://github.com/googleapis/google-cloud-go/commit/182df616184794be315edc7299aff021052c4f46))
- **bigquery/reservation:** Add new reservation IAM policy get/set/test methods ([182df61](https://github.com/googleapis/google-cloud-go/commit/182df616184794be315edc7299aff021052c4f46))
- **bigquery/reservation:** Add support for creation and modification of new reservation groups ([182df61](https://github.com/googleapis/google-cloud-go/commit/182df616184794be315edc7299aff021052c4f46))
- **bigquery:** Expose continuous query in config ([#13130](https://github.com/googleapis/google-cloud-go/issues/13130)) ([2f0942b](https://github.com/googleapis/google-cloud-go/commit/2f0942b65854dcabbf49c1605e26fc5a6543c734))

##### Bug Fixes

- **bigquery/v2:** Upgrade gRPC service registration func ([8fffca2](https://github.com/googleapis/google-cloud-go/commit/8fffca2819fa3dc858c213aa0c503e0df331b084))
- **bigquery:** Upgrade gRPC service registration func ([8fffca2](https://github.com/googleapis/google-cloud-go/commit/8fffca2819fa3dc858c213aa0c503e0df331b084))

## October 31, 2025

Feature We have [increased the row capacity](https://workspaceupdates.googleblog.com/2025/10/powerful-pivot-tables-connected-sheets.html)
for pivot tables backed by BigQuery in [Connected Sheets](https://docs.cloud.google.com/bigquery/docs/connected-sheets)
from 100,000 to 200,000 rows.

## October 30, 2025

Feature The
[Apache Iceberg REST catalog in BigLake metastore](https://docs.cloud.google.com/biglake/docs/blms-rest-catalog)
is now
[generally available](https://cloud.google.com/products#product-launch-stages)
(GA) with several new features, including BigQuery catalog federation,
credential vending, and catalog management in the Google Cloud console.

## October 29, 2025

Feature You can now [group
reservations](https://docs.cloud.google.com/bigquery/docs/reservations-tasks#prioritize_idle_slots_with_reservation_groups)
together to prioritize idle slot sharing within the group. Reservations within a
reservation group share idle slots with each other before making them available
to other reservations in the project, giving you more control over slot
allocation for high-priority workloads. This feature is in
[Preview](https://cloud.google.com/products/#product-launch-stages).

## October 28, 2025

Feature The BigQuery Data Transfer Service can now transfer data from the
following data sources:

- [Facebook Ads](https://docs.cloud.google.com/bigquery/docs/facebook-ads-transfer)
- [Salesforce](https://docs.cloud.google.com/bigquery/docs/salesforce-transfer)
- [Salesforce Marketing Cloud](https://docs.cloud.google.com/bigquery/docs/sfmc-transfer)
- [ServiceNow](https://docs.cloud.google.com/bigquery/docs/servicenow-transfer)

Transfers from these data sources are now [generally available](https://cloud.google.com/products#product-launch-stages)
(GA).
Feature Subscriber email logging lets you log the principal identifiers of users
who execute jobs and queries against linked datasets. You can enable
logging at the
[listing level](https://cloud.google.com/bigquery/docs/analytics-hub-manage-listings#create_a_listing)
and the
[data exchange level](https://cloud.google.com/bigquery/docs/analytics-hub-manage-exchanges#create-exchange).
The logged data is available in the `job_principal_subject` field of the
[`INFORMATION_SCHEMA.SHARED_DATASET_USAGE` view](https://docs.cloud.google.com/bigquery/docs/information-schema-shared-dataset-usage).
This feature is
[generally available](https://cloud.google.com/products#product-launch-stages).

## October 27, 2025

Feature The administrative jobs explorer now includes a [job details
page](https://docs.cloud.google.com/bigquery/docs/admin-jobs-explorer#get-job-details) to help you diagnose
and troubleshoot queries. The **Performance** tab compiles query information
including the execution graph, SQL text, execution history, performance
variance, and system load during execution. You can also [compare two
jobs](https://docs.cloud.google.com/bigquery/docs/admin-jobs-explorer#compare-jobs) to identify discrepancies
and potential areas to improve query performance.

This feature is in [Preview](https://cloud.google.com/products/#product-launch-stages).
Feature BigQuery now offers the following
[managed AI functions](https://docs.cloud.google.com/bigquery/docs/generative-ai-overview#managed_ai_functions)
that use Gemini to help you filter, join, rank, and classify your data:

- [`AI.IF`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-if): Filter and join text or multimodal data based on a condition described in natural language.
- [`AI.SCORE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-score): Rate text or multimodal input to rank your data by quality, similarity, or other criteria.
- [`AI.CLASSIFY`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-classify): Classify text into user-defined categories.

These functions are in [Preview](https://cloud.google.com/products/#product-launch-stages).
Feature You can now [use the Data Engineering Agent](https://docs.cloud.google.com/bigquery/docs/data-engineering-agent-pipelines)
to use Gemini in BigQuery to build and modify data
pipelines to ingest data into BigQuery. This feature is in
[preview](https://cloud.google.com/products/#product-launch-stages).
Feature You can now use the Apache Arrow format to
[stream data to BigQuery with the Storage Write API](https://docs.cloud.google.com/bigquery/docs/write-api#arrow-handling).
This feature is
[generally available](https://cloud.google.com/products/#product-launch-stages)
(GA).
Libraries

### Java

#### [2.55.3](https://github.com/googleapis/java-bigquery/compare/v2.55.2...v2.55.3) (2025-10-21)

##### Dependencies

- Update dependency com.google.cloud:sdk-platform-java-config to v3.53.0 ([#3980](https://github.com/googleapis/java-bigquery/issues/3980)) ([a961247](https://github.com/googleapis/java-bigquery/commit/a961247e9546a9fce8da1609afd18975142c2379))

## October 23, 2025

Feature BigQuery is now offering
[early access](https://cloud.google.com/products/#product-launch-stages)
to conversational analytics. Conversational analytics accelerates data analysis
by enabling quick insights through natural language. Users can chat with their
BigQuery data, create custom agents, and access those agents even outside of
BigQuery. To enroll in conversational analytics early access, fill out the
[request form](https://docs.google.com/forms/d/e/1FAIpQLSe5KhRr81uUce6mKj8YrV1iFezGIydTxOcx8wUTqcBJP3e7vg/viewform).

## October 22, 2025

Issue Support for
[table parameters in table-value functions (TVFs)](https://docs.cloud.google.com/bigquery/docs/table-functions#table_parameters)
has been temporarily disabled. We are working to restore this feature as soon
as possible.
Feature You can now use custom constraints with Organization Policy to provide more
granular control over specific fields for some BigQuery sharing
resources. For more information, see
[Manage Sharing data exchanges and listings using custom constraints](https://docs.cloud.google.com/bigquery/docs/analytics-hub-custom-constraints).
This feature is in
[preview](https://cloud.google.com/products#product-launch-stages).
Feature BigQuery ML now offers a built-in
[TimesFM univariate time series forecasting model](https://docs.cloud.google.com/bigquery/docs/timesfm-model)
that implements Google Research's open source TimesFM model. You can use
BigQuery ML's built-in TimesFM model with the following functions:

- Use [`AI.FORECAST`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-forecast) to perform forecasting. This function now supports a larger context window.
- Use [`AI.EVALUATE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-evaluate) to evaluate forecasted data against a reference time series based on historical data.

To try using a TimesFM model with the `AI.FORECAST` function, see
[Forecast a time series with a TimesFM univariate model](https://docs.cloud.google.com/bigquery/docs/timesfm-time-series-forecasting-tutorial).

This feature is
[generally available](https://cloud.google.com/products/#product-launch-stages)
(GA).

## October 21, 2025

Feature BigQuery now supports TransUnion for
[entity resolution](https://docs.cloud.google.com/bigquery/docs/entity-resolution-setup).
This feature is
[generally available](https://cloud.google.com/products#product-launch-stages)
(GA).

## October 20, 2025

Feature In BigQuery ML, you can now fully manage open models as Vertex AI endpoints.
BigQuery-managed open models offer the following benefits:

- [Manage Vertex AI resource by using SQL queries](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model-open#managed-resources)
- [Automatic or immediate open model undeployment](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model-open#managed-model-undeployment) to save costs
- [Customize model deployment machine types](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model-open#machine-type) or reserve open model resources by [using Compute Engine reservations](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model-open#reservation-affinity)

This feature is in [Preview](https://cloud.google.com/products/#product-launch-stages).
Feature You can now use [visualization cells](https://docs.cloud.google.com/bigquery/docs/create-notebooks#cells) to
automatically
[generate a visualization](https://docs.cloud.google.com/bigquery/docs/visualize-data-colab)
of any DataFrame in your notebook.
You can customize the columns, chart type, aggregations, colors , labels, and
title.

This feature is in [Preview](https://cloud.google.com/products/#product-launch-stages).

## October 16, 2025

Feature You can now access repositories by clicking Repositories in the Explorer pane.
A new tab opens that displays a list of repositories. The Explorer pane no
longer has a bottom pane for repositories. When you open a workspace in a
repository, it opens in the Git repository pane in the left pane. These
features are available in BigQuery Studio in
[preview](https://cloud.google.com/products/#product-launch-stages).
Feature The following features are now [generally available](https://cloud.google.com/products/#product-launch-stages) (GA) in BigQuery Studio:

- To streamline resource discovery and access, the [left Explorer
  pane](https://docs.cloud.google.com/bigquery/docs/bigquery-web-ui#explorer_panel) has been reorganized
  into three sections: Explorer, Classic Explorer, and Git repository. You can
  still use the Classic Explorer, which provides the complete resources tree.

- In the Explorer pane, you can use the [search
  feature](https://docs.cloud.google.com/bigquery/docs/bigquery-web-ui#explorer_pane) to find BigQuery
  resources in your organization. The results appear in a new tab in the
  details pane. You can use filters to narrow your search.

- You can access job histories by clicking [Job
  history](https://docs.cloud.google.com/bigquery/docs/managing-jobs) in the Explorer pane. A new tab opens
  that displays a list of job histories. BigQuery Studio no longer has a
  bottom pane for job history.

- To reduce tab proliferation, clicking a resource opens it within the same
  tab. To open the resource in a separate tab, press <kbd>Ctrl</kbd> (or
  <kbd>Command</kbd> on macOS) and click the resource. To prevent the current
  tab from getting its content replaced, double-click the tab. The name
  changes from italicized to regular font. If you still
  lose your resource, you can click tab_recent Recent tabs in the
  details pane to find the resource.

- You can use breadcrumbs to navigate through different tabs and
  resources in the details pane.

- In the Home tab, the [What's new section](https://docs.cloud.google.com/bigquery/docs/bigquery-web-ui#welcome_tab)
  contains a list of new capabilities and changes to the BigQuery Studio.

- The notebook action bar is consolidated by default to give you more screen
  space for writing code.

## October 15, 2025

Feature You can
[visualize your geospatial query results](https://docs.cloud.google.com/bigquery/docs/geospatial-visualize)
on an interactive map in BigQuery Studio. This feature is
[generally available](https://cloud.google.com/products#product-launch-stages)
(GA).
Feature You can use the `dbt-bigquery` adapter to run Python code that's defined in
BigQuery DataFrames. For more information, see
[Use BigQuery DataFrames in dbt](https://docs.cloud.google.com/bigquery/docs/dataframes-dbt).
This feature is
[generally available](https://cloud.google.com/products#product-launch-stages)
(GA).

## October 14, 2025

Feature You can now use [SQL cells](https://docs.cloud.google.com/bigquery/docs/create-notebooks#cells) to write,
edit, and run SQL queries on your BigQuery data directly from your
notebooks. This feature is in
[Preview](https://cloud.google.com/products#product-launch-stages).
Announcement The BigQuery Data Transfer API (bigquerydatatransfer.googleapis.com) is now
enabled by default for every new Google Cloud project. This feature is
[generally available](https://cloud.google.com/products#product-launch-stages)
(GA).
Feature You can now [embed natural language as comments in existing SQL to refine and
transform your code](https://docs.cloud.google.com/bigquery/docs/write-sql-gemini#generate_sql_from_a_comment).
This feature is [preview](https://cloud.google.com/products#product-launch-stages).

## October 13, 2025

Libraries

### Java

#### [2.55.2](https://github.com/googleapis/java-bigquery/compare/v2.55.1...v2.55.2) (2025-10-08)

##### Dependencies

- Fix update dependency com.google.cloud:google-cloud-bigquerystorage-bom to v3.17.2 ([b25095d](https://github.com/googleapis/java-bigquery/commit/b25095d23279dab178975c33f4de84612612e175))
- Update dependency com.google.cloud:sdk-platform-java-config to v3.52.3 ([#3971](https://github.com/googleapis/java-bigquery/issues/3971)) ([f8cf508](https://github.com/googleapis/java-bigquery/commit/f8cf50833772412c4f15922bffcdf5100792948d))

## October 09, 2025

Change An updated version of the [ODBC driver for BigQuery](https://docs.cloud.google.com/bigquery/docs/reference/odbc-jdbc-drivers#odbc_release_3151022) is now available.
Feature You can set a [maximum slot
limit](https://docs.cloud.google.com/bigquery/docs/reservations-workload-management#predictable) for a
reservation. You can configure the maximum reservation size when creating or
updating a reservation. This feature is now [generally
available](https://cloud.google.com/products#product-launch-stages) (GA).
Feature You can [allocate idle slots fairly](https://docs.cloud.google.com/bigquery/docs/slots#fairness) across
reservations within a single admin project. This ensures each reservation
receives an approximately equal share of available capacity. This feature is now
[generally available](https://cloud.google.com/products#product-launch-stages)
(GA).
Announcement [Security, privacy, and compliance for Gemini in
BigQuery](https://docs.cloud.google.com/gemini/docs/bigquery/security-privacy-compliance) details how
customer data is protected and processed by Gemini in BigQuery.

## October 08, 2025

Breaking The [default limit of `QueryUsagePerDay`](https://docs.cloud.google.com/bigquery/quotas#query_jobs) for
on-demand pricing has changed. The default limit of all new projects is now 200
TiB. For existing projects, the default limit has been set based on your
project's usage over the last 30 days. Projects that have [custom cost
controls](https://docs.cloud.google.com/bigquery/docs/custom-quotas) configured or that use
[reservations](https://docs.cloud.google.com/bigquery/docs/reservations-workload-management) aren't affected.
If the new limit might affect your workload, create a [custom cost
control](https://docs.cloud.google.com/bigquery/docs/custom-quotas) based on your workload needs.
Feature You can [specify which reservation a query uses at
runtime](https://docs.cloud.google.com/bigquery/docs/reservations-workload-management#flexible), and [set IAM
policies directly on
reservations](https://docs.cloud.google.com/bigquery/docs/reservations-workload-management#securable). This
provides more flexibility and fine-grained control over resource management.
This feature is [generally
available](https://cloud.google.com/products#product-launch-stages) (GA).
Feature You can set [labels](https://docs.cloud.google.com/bigquery/docs/labels-intro) on reservations. These labels
can be used to organize your reservations and for billing analysis. This feature
is [generally
available](https://cloud.google.com/products#product-launch-stages) (GA).

## October 07, 2025

Announcement As of February 25, 2025, enhancements to the [workload management
autoscaler](https://docs.cloud.google.com/bigquery/docs/slots-autoscaling-intro) that were announced on [July
31, 2024](https://docs.cloud.google.com/bigquery/docs/release-notes#July_31_2024) have rolled out to all users.
These enhancements are [generally
available](https://cloud.google.com/products#product-launch-stages) (GA).

## October 06, 2025

Feature The [`INFORMATION_SCHEMA.SHARED_DATASET_USAGE` view](https://docs.cloud.google.com/bigquery/docs/information-schema-shared-dataset-usage#schema)
now includes the following schema fields to support usage metrics for
external tables and routines:

- `shared_resource_id`: the ID of the queried resource
- `shared_resource_type`: the type of the queried resource
- `referenced_tables`: Contains `project_id`, `dataset_id`, `table_id`, and `processed_bytes` fields of the base table.

These fields are
[generally available](https://cloud.google.com/products#product-launch-stages) (GA).
Feature You can now set the priority of BigQuery jobs initiated by
Dataform workflows to run queries as interactive jobs that start
running as quickly as possible or as batch jobs with lower priority. For more
information, see
[Create a pipeline schedule](https://docs.cloud.google.com/bigquery/docs/schedule-pipelines#create-schedule)
and
[InvocationConfig](https://docs.cloud.google.com/dataform/reference/rest/v1/InvocationConfig).
This feature is
[generally available](https://cloud.google.com/products#product-launch-stages)
(GA).
Feature The BigQuery Data Transfer Service can now [transfer reporting data from Google Analytics 4](https://docs.cloud.google.com/bigquery/docs/google-analytics-4-transfer)
into BigQuery. You can also include custom reports from
Google Analytics 4 in your data transfer. This feature is
[generally available](https://cloud.google.com/products#product-launch-stages) (GA).
Feature The BigQuery Data Transfer Service can now transfer data from the
following data sources:

- [PayPal](https://docs.cloud.google.com/bigquery/docs/paypal-transfer)
- [Stripe](https://docs.cloud.google.com/bigquery/docs/stripe-transfer)

Transfers from these data sources are supported in [preview](https://cloud.google.com/products/#product-launch-stages).
Libraries

### Go

#### [1.71.0](https://github.com/googleapis/google-cloud-go/compare/bigquery/v1.70.0...bigquery/v1.71.0) (2025-09-30)

##### Features

- **bigquery/analyticshub:** You can now configure listings for multiple regions for shared datasets and linked dataset replicas in BigQuery sharing ([10e67ef](https://github.com/googleapis/google-cloud-go/commit/10e67efccf048adea11d3ecba8d0c625455e545f))
- **bigquery/reservation:** Add a new field `failover_mode` to `.google.cloud.bigquery.reservation.v1.FailoverReservationRequest` that allows users to choose between the HARD or SOFT failover modes when they initiate a failover operation on a reservation ([10e67ef](https://github.com/googleapis/google-cloud-go/commit/10e67efccf048adea11d3ecba8d0c625455e545f))
- **bigquery/reservation:** Add a new field `soft_failover_start_time` in the existing `replication_status` in `.google.cloud.bigquery.reservation.v1.Reservation` to provide visibility into the state of ongoing soft failover operations on the reservation ([10e67ef](https://github.com/googleapis/google-cloud-go/commit/10e67efccf048adea11d3ecba8d0c625455e545f))
- **bigquery:** Add support for MaxSlots ([#12958](https://github.com/googleapis/google-cloud-go/issues/12958)) ([a3c0aca](https://github.com/googleapis/google-cloud-go/commit/a3c0aca6edb873132360b46c7bb1a2aaab6d3fce))
Announcement Starting March 17, 2026, the BigQuery Data Transfer Service will require the
`bigquery.datasets.setIamPolicy` and the `bigquery.datasets.getIamPolicy`
permissions on the target dataset to create or update a transfer configuration.
For more information, see [Changes to dataset-level access controls](https://docs.cloud.google.com/bigquery/docs/dataset-access-control).

## October 02, 2025

Feature You can now use the [notebook gallery](https://docs.cloud.google.com/bigquery/docs/notebooks-introduction#notebook_gallery)
in the BigQuery web UI as your central hub for discovering and using prebuilt notebook
templates. This feature is in [preview](https://cloud.google.com/products/#product-launch-stages).

## October 01, 2025

Feature You can now apply
[SQL query generated in the Gemini Cloud Assist chat](https://docs.cloud.google.com/bigquery/docs/write-sql-gemini#chat)
to the query open in your editor. This feature is in
[Preview](https://cloud.google.com/products/#product-launch-stages).

## September 29, 2025

Feature To simplify access management for your Iceberg tables, you
can use [credential vending mode with the Apache Iceberg REST catalog](https://docs.cloud.google.com/bigquery/docs/blms-rest-catalog) in Lakehouse runtime catalog. Credential vending removes
the need for catalog users to have direct access to Cloud Storage buckets. This
feature is in [Preview](https://cloud.google.com/products/#product-launch-stages).
Feature You can now create BigQuery
[non-incremental materialized views over Spanner data](https://docs.cloud.google.com/bigquery/docs/materialized-views-create#spanner)
to improve query performance by periodically caching results.
This feature is in [Preview](https://cloud.google.com/products/#product-launch-stages).
Feature BigQuery data preparation supports unnesting arrays, which expands each
array element into its own row for easier analysis. For more information, see
[Unnest arrays](https://docs.cloud.google.com/bigquery/docs/data-prep-get-suggestions#unnest-arrays).
This feature is [generally
available](https://cloud.google.com/products#product-launch-stages) (GA).
Announcement [History-based query optimizations](https://docs.cloud.google.com/bigquery/docs/history-based-optimizations)
are now enabled by default. If history-based optimizations have been previously
disabled, you can [re-enable history-based optimizations](https://docs.cloud.google.com/bigquery/docs/history-based-optimizations#enable-history-based-optimization)
for your project or organization.

## September 25, 2025

Feature The
[`ARRAY_FIRST`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/array_functions#array_first),
[`ARRAY_LAST`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/array_functions#array_last),
and
[`ARRAY_SLICE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/array_functions#array_slice)
GoogleSQL functions are now
[generally available](https://cloud.google.com/products#product-launch-stages) (GA).
Feature BigQuery [data
canvas](https://docs.cloud.google.com/bigquery/docs/data-canvas#destination_node) now
supports destination table nodes. Destination table nodes let you persist query
results to a new or existing table. This feature is [generally
available](https://cloud.google.com/products#product-launch-stages) (GA).

## September 24, 2025

Feature BigQuery ML now supports
[visualization of model monitoring metrics](https://docs.cloud.google.com/bigquery/docs/model-monitoring-overview#monitoring_visualization).
This feature lets you use charts and graphs to
[analyze model monitoring function output](https://docs.cloud.google.com/vertex-ai/docs/model-monitoring/run-monitoring-job#analyze_monitoring_job_results).
You can use metric visualization with the
[`ML.VALIDATE_DATA_SKEW`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-validate-data-skew)
and
[`ML.VALIDATE_DATA_DRIFT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-validate-data-drift)
functions. This feature is
[generally available](https://cloud.google.com/products/#product-launch-stages)
(GA).
Feature For command-line users, BigQuery is now integrated with the Gemini CLI to provide an agentic CLI experience. Using the dedicated [Gemini CLI extensions for BigQuery](https://cloud.google.com/bigquery/docs/develop-with-gemini-cli), you can search, explore, analyze, and gain insights from your data by asking natural language questions, generating forecasts, and running contribution analysis directly from the command line. This feature is available in beta.

## September 22, 2025

Libraries

### Python

#### [3.38.0](https://github.com/googleapis/python-bigquery/compare/v3.37.0...v3.38.0) (2025-09-15)

##### Features

- Add additional query stats ([#2270](https://github.com/googleapis/python-bigquery/issues/2270)) ([7b1b718](https://github.com/googleapis/python-bigquery/commit/7b1b718123afd80c0f68212946e4179bcd6db67f))
Feature You can now run federated queries against [PostgreSQL dialect databases in Spanner](https://docs.cloud.google.com/spanner/docs/reference/postgresql/overview) using [BigQuery external datasets](https://docs.cloud.google.com/bigquery/docs/spanner-external-datasets) with GoogleSQL; this includes [cross-region federated queries](https://cloud.google.com/bigquery/docs/spanner-federated-queries#cross_region_queries). This feature is [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).

## September 16, 2025

Feature You can now [access snapshots of Apache Iceberg external tables](https://docs.cloud.google.com/bigquery/docs/iceberg-external-tables#query_historical_data) that are retained in your Iceberg metadata by using the `FOR SYSTEM_TIME AS OF` clause. This feature is [generally available](https://cloud.google.com/products#product-launch-stages) (GA).
Feature You can use the [`JSON_KEYS` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#json_keys) to extract unique JSON keys from a JSON expression, and you can specify a [mode](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#JSONPath_mode) for some JSON functions that take a JSONPath to allow more flexibility in how the path matches the JSON structure. These features are [generally available](https://cloud.google.com/products#product-launch-stages) (GA).
Feature [SQL code completion](https://docs.cloud.google.com/bigquery/docs/write-sql-gemini) is now available for all BigQuery projects. To learn how to enable and activate Gemini in BigQuery features, see [Set up Gemini in BigQuery](https://docs.cloud.google.com/gemini/docs/bigquery/set-up-gemini). This feature is available in [preview](https://cloud.google.com/products#product-launch-stages).

## September 15, 2025

Libraries

### Python

#### [3.37.0](https://github.com/googleapis/python-bigquery/compare/v3.36.0...v3.37.0) (2025-09-08)

##### Features

- Updates to fastpath query execution ([#2268](https://github.com/googleapis/python-bigquery/issues/2268)) ([ef2740a](https://github.com/googleapis/python-bigquery/commit/ef2740a158199633b5543a7b6eb19587580792cd))

##### Bug Fixes

- Remove deepcopy while setting properties for _QueryResults ([#2280](https://github.com/googleapis/python-bigquery/issues/2280)) ([33ea296](https://github.com/googleapis/python-bigquery/commit/33ea29616c06a2e2a106a785d216e784737ae386))

##### Documentation

- Clarify that the presence of `XyzJob.errors` doesn't necessarily mean that the job has not completed or was unsuccessful ([#2278](https://github.com/googleapis/python-bigquery/issues/2278)) ([6e88d7d](https://github.com/googleapis/python-bigquery/commit/6e88d7dbe42ebfc35986da665d656b49ac481db4))
- Clarify the api_method arg for client.query() ([#2277](https://github.com/googleapis/python-bigquery/issues/2277)) ([8a13c12](https://github.com/googleapis/python-bigquery/commit/8a13c12905ffcb3dbb6086a61df37556f0c2cd31))
Libraries

### Java

#### [2.55.0](https://github.com/googleapis/java-bigquery/compare/v2.54.2...v2.55.0) (2025-09-12)

##### Features

- **bigquery:** Add custom ExceptionHandler to BigQueryOptions ([#3937](https://github.com/googleapis/java-bigquery/issues/3937)) ([de0914d](https://github.com/googleapis/java-bigquery/commit/de0914ddbccf988294d50faf56a515e58ab3505d))

##### Dependencies

- Update dependency com.google.cloud:google-cloud-bigquerystorage-bom to v3.17.0 ([#3954](https://github.com/googleapis/java-bigquery/issues/3954)) ([e73deed](https://github.com/googleapis/java-bigquery/commit/e73deed9c68a45023d02b40144c304329d6b5829))
- Update dependency com.google.cloud:sdk-platform-java-config to v3.52.1 ([#3952](https://github.com/googleapis/java-bigquery/issues/3952)) ([79b7557](https://github.com/googleapis/java-bigquery/commit/79b7557501d318fd92b90a681036fe6a1aa1bac4))
Feature In the BigQuery Studio, in the Explorer pane, you can now [open saved queries in Connected Sheets](https://docs.cloud.google.com/bigquery/docs/manage-saved-queries#open-saved-queries-in-sheets). This feature is [generally available](https://cloud.google.com/products#product-launch-stages) (GA).
Feature You can now enable the [BigQuery advanced runtime](https://docs.cloud.google.com/bigquery/docs/advanced-runtime) to improve query execution time and slot usage. This feature is [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).
Between September 15, 2025 and early 2026, the BigQuery advanced runtime will become the default runtime for all projects.

## September 11, 2025

Feature Gemini now recommends natural language prompts for you in the [SQL generation tool](https://docs.cloud.google.com/bigquery/docs/write-sql-gemini#generate_a_sql_query). This feature is in [Preview](https://cloud.google.com/products#product-launch-stages).
Feature When you use the [Data Science Agent](https://docs.cloud.google.com/bigquery/docs/colab-data-science-agent) in BigQuery, you can now use the Apache Spark or PySpark keywords in your prompt. The Data Science Agent is in [Preview](https://cloud.google.com/products#product-launch-stages).
Feature Use the [BigQuery migration assessment for Informatica](https://docs.cloud.google.com/bigquery/docs/migration-assessment) to assess the complexity of migrating data from your Informatica platform to BigQuery. This feature is in [Preview](https://cloud.google.com/products/#product-launch-stages).

## September 10, 2025

Feature You can [load files from Cloud Storage in BigQuery data preparations](https://docs.cloud.google.com/bigquery/docs/data-prep-get-suggestions#create-new-from-gcs). This feature is in [Preview](https://cloud.google.com/products#product-launch-stages).

## September 09, 2025

Feature You can configure reusable, default Cloud resource connections in a project. [Default connections](https://docs.cloud.google.com/bigquery/docs/default-connections) are [generally available](https://cloud.google.com/products#product-launch-stages) (GA).
Feature The [batch](https://docs.cloud.google.com/bigquery/docs/batch-sql-translator) and [interactive translators](https://docs.cloud.google.com/bigquery/docs/interactive-sql-translator) now caches your metadata, which can improve latency when you run a SQL translation. This feature is [generally available](https://cloud.google.com/products#product-launch-stages) (GA).
Change You can now perform [supervised tuning](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model#supervised_tuning) on a BigQuery ML [remote model](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model) based on a Vertex AI `gemini-2.5-pro` or `gemini-2.5-flash-lite` model.

## September 08, 2025

Feature When you use the [Data Science Agent](https://docs.cloud.google.com/bigquery/docs/colab-data-science-agent) in BigQuery, you can now use the `@` symbol to search for BigQuery tables in your project, and you can use the `+` symbol to search for files to upload. The Data Science Agent is in [Preview](https://cloud.google.com/products#product-launch-stages).
Feature You can now add tables and views as tasks to BigQuery pipelines. For more information, see [Add a pipeline task](https://docs.cloud.google.com/bigquery/docs/create-pipelines#add_a_pipeline_task). This feature is in [Preview](https://cloud.google.com/products#product-launch-stages).
Feature You can now include [table parameters](https://docs.cloud.google.com/bigquery/docs/table-functions#table_parameters) when you create a table-valued function (TVF). This feature is in [Preview](https://cloud.google.com/products#product-launch-stages).

## September 03, 2025

Feature The `INFORMATION_SCHEMA.RESERVATIONS_TIMELINE` view now includes the [`per_second_details` schema field](https://docs.cloud.google.com/bigquery/docs/information-schema-reservation-timeline#schema). This new field provides information regarding reservation capacity and usage on a per-second basis, and also includes details on autoscale utilization. This feature is [generally available](https://cloud.google.com/products#product-launch-stages) (GA).
Feature BigQuery now supports [soft failover](https://docs.cloud.google.com/bigquery/docs/managed-disaster-recovery) with managed disaster recovery. This feature is [generally available](https://cloud.google.com/products#product-launch-stages) (GA).
Feature You can [flatten records](https://docs.cloud.google.com/bigquery/docs/data-prep-get-suggestions#flatten-records) in [BigQuery data preparation](https://docs.cloud.google.com/bigquery/docs/data-prep-introduction) with a single operation. This feature is [generally available](https://cloud.google.com/products#product-launch-stages) (GA).

## September 02, 2025

Feature You can now configure listings for multiple regions for shared datasets and linked dataset replicas in BigQuery sharing. For more information, see [Create a listing](https://docs.cloud.google.com/bigquery/docs/analytics-hub-manage-listings#create_a_listing). This feature is in [preview](https://cloud.google.com/products#product-launch-stages).
Feature You can now reference BigQuery ML and DataFrames in your [prompts](https://docs.cloud.google.com/bigquery/docs/colab-data-science-agent#sample-prompts) when you use the [Data Science Agent](https://docs.cloud.google.com/bigquery/docs/colab-data-science-agent) in a BigQuery notebook. The Data Science Agent is in [Preview](https://cloud.google.com/products/#product-launch-stages).
Feature You can now create a [remote model](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model) based on the Vertex AI `gemini-embedding-001` model. You can then use the [`ML.GENERATE_EMBEDDING` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-generate-embedding) with this remote model to generate embeddings. This feature is in [Preview](https://cloud.google.com/products/#product-launch-stages).
Feature You can now enable the automatic selection of a processing location in your pipeline configurations. For more information, see [Create pipelines](https://docs.cloud.google.com/bigquery/docs/create-pipelines). This feature is [generally available](https://cloud.google.com/products#product-launch-stages) (GA).
Feature You can now create a [remote model based on an open embedding model from Vertex Model Garden or Hugging Face that is deployed to Vertex AI](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model-open). Options include E5 Embedding and other leading open embedding generation models. You can then use the [`ML.GENERATE_EMBEDDING` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-generate-embedding) with this remote model to generate embeddings.

Try this feature with the [Generate text embeddings by using an open model and the `ML.GENERATE_EMBEDDING` function](https://docs.cloud.google.com/bigquery/docs/generate-text-embedding-tutorial-open-models) tutorial.

This feature is in [Preview](https://cloud.google.com/products/#product-launch-stages).

## September 01, 2025

Libraries

### Java

#### [2.54.2](https://github.com/googleapis/java-bigquery/compare/v2.54.1...v2.54.2) (2025-08-26)

##### Dependencies

- Update dependency com.google.cloud:sdk-platform-java-config to v3.52.0 ([#3939](https://github.com/googleapis/java-bigquery/issues/3939)) ([794bf83](https://github.com/googleapis/java-bigquery/commit/794bf83e84efc0712638bebde5158777b9c89397))
Libraries

### Go

#### [1.70.0](https://github.com/googleapis/google-cloud-go/compare/bigquery/v1.69.0...bigquery/v1.70.0) (2025-08-28)

##### Features

- **bigquery/reservation:** Add Reservation.max_slots field to Reservation proto, indicating the total max number of slots this reservation can use up to ([f1de706](https://github.com/googleapis/google-cloud-go/commit/f1de7062db662aa6dfbf1e8cd2f0ac5df678e76d))
- **bigquery/reservation:** Add Reservation.scaling_mode field and its corresponding enum message ScalingMode. This field should be used together with Reservation.max_slots ([f1de706](https://github.com/googleapis/google-cloud-go/commit/f1de7062db662aa6dfbf1e8cd2f0ac5df678e76d))
- **bigquery/storage/managedwriter:** Allow overriding proto conversion mapping ([#12579](https://github.com/googleapis/google-cloud-go/issues/12579)) ([ce9d29b](https://github.com/googleapis/google-cloud-go/commit/ce9d29bf2ca22877c64c9eea5b5c6489de141cc5)), refs [#12578](https://github.com/googleapis/google-cloud-go/issues/12578)
- **bigquery:** Add load/extract job completion ratio ([#12471](https://github.com/googleapis/google-cloud-go/issues/12471)) ([3dab483](https://github.com/googleapis/google-cloud-go/commit/3dab483ad579c65ce520d6d9a2f8ad738ad68c9c))
- **bigquery:** Load job and external table opts for custom time format, null markers and source column match ([#12470](https://github.com/googleapis/google-cloud-go/issues/12470)) ([67b0320](https://github.com/googleapis/google-cloud-go/commit/67b0320a54be1ba7bc64eeee47a9afff14faac5f))

## August 28, 2025

Feature For additional layers of security and control, you can now use query templates to predefine and limit the queries that can be run in data clean rooms. For more information, see [Use query templates](https://docs.cloud.google.com/bigquery/docs/query-templates). This feature is in [preview](https://cloud.google.com/products#product-launch-stages).

## August 26, 2025

Feature You can [deduplicate table data](https://docs.cloud.google.com/bigquery/docs/data-prep-get-suggestions#deduplicate) with Gemini assistance in your [BigQuery data preparations](https://docs.cloud.google.com/bigquery/docs/data-prep-introduction). Deduplication is in [Preview](https://cloud.google.com/products#product-launch-stages).

## August 25, 2025

Libraries

### Python

#### [3.36.0](https://github.com/googleapis/python-bigquery/compare/v3.35.1...v3.36.0) (2025-08-20)

##### Features

- Add created/started/ended properties to RowIterator. ([#2260](https://github.com/googleapis/python-bigquery/issues/2260)) ([0a95b24](https://github.com/googleapis/python-bigquery/commit/0a95b24192395cc3ccf801aa9bc318999873a2bf))
- Retry query jobs if `jobBackendError` or `jobInternalError` are encountered ([#2256](https://github.com/googleapis/python-bigquery/issues/2256)) ([3deff1d](https://github.com/googleapis/python-bigquery/commit/3deff1d963980800e8b79fa3aaf5b712d4fd5062))

##### Documentation

- Add a TROUBLESHOOTING.md file with tips for logging ([#2262](https://github.com/googleapis/python-bigquery/issues/2262)) ([b684832](https://github.com/googleapis/python-bigquery/commit/b68483227693ea68f6b12eacca2be1803cffb1d1))
- Update README to break infinite redirect loop ([#2254](https://github.com/googleapis/python-bigquery/issues/2254)) ([8f03166](https://github.com/googleapis/python-bigquery/commit/8f031666114a826da2ad965f8ecd4727466cb480))
Feature You can use the [`ST_REGIONSTATS` geography function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_regionstats) to combine raster data using Earth Engine with your vector data stored in BigQuery. For more information, see [Work with raster data](https://docs.cloud.google.com/bigquery/docs/raster-data) and try the tutorial that shows you how to [use raster data to analyze global temperature](https://docs.cloud.google.com/bigquery/docs/raster-tutorial-weather). This feature is [generally available](https://cloud.google.com/products#product-launch-stages).
Feature You can now use data insights to have Gemini [generate table and column descriptions from table metadata](https://docs.cloud.google.com/bigquery/docs/data-insights). This feature is [generally available](https://cloud.google.com/products#product-launch-stages) (GA).

## August 22, 2025

Feature Multi-statement transactions are now available for [BigLake Iceberg tables in BigQuery](https://docs.cloud.google.com/bigquery/docs/iceberg-tables#use_multi-statement_transactions). This feature is in [Preview](https://cloud.google.com/products#product-launch-stages).

## August 21, 2025

Announcement Starting September 25, 2025, the BigQuery Data Transfer Service for third-party SAAS and database connectors will update to a consumption-based pricing model. With this new pricing model, you will be charged based on the compute resources consumed by your data transfers, measured in slot-hours. For more information, see [Data Transfer Service pricing](https://cloud.google.com/bigquery/pricing#section-5). This pricing update applies to the following third-party connectors when they are [generally available (GA)](https://cloud.google.com/products#product-launch-stages):

- [Facebook Ads](https://docs.cloud.google.com/bigquery/docs/facebook-ads-transfer)
- [MySQL](https://docs.cloud.google.com/bigquery/docs/mysql-transfer)
- [Oracle](https://docs.cloud.google.com/bigquery/docs/oracle-transfer)
- [PostgreSQL](https://docs.cloud.google.com/bigquery/docs/postgresql-transfer)
- [Salesforce](https://docs.cloud.google.com/bigquery/docs/salesforce-transfer)
- [Salesforce Marketing Cloud](https://docs.cloud.google.com/bigquery/docs/sfmc-transfer)
- [ServiceNow](https://docs.cloud.google.com/bigquery/docs/servicenow-transfer)
- Other third-party connectors planned for future releases

## August 18, 2025

Libraries

### Java

#### [2.54.1](https://github.com/googleapis/java-bigquery/compare/v2.54.0...v2.54.1) (2025-08-13)

##### Bug Fixes

- Adapt graalvm config to arrow update ([#3928](https://github.com/googleapis/java-bigquery/issues/3928)) ([ecfabc4](https://github.com/googleapis/java-bigquery/commit/ecfabc4b70922d0e697699ec5508a7328cadacf8))

##### Dependencies

- Update dependency com.google.cloud:sdk-platform-java-config to v3.51.0 ([#3924](https://github.com/googleapis/java-bigquery/issues/3924)) ([cb66be5](https://github.com/googleapis/java-bigquery/commit/cb66be596d1bfd0a5aed75f5a0e36d80269c7f6a))
Feature In the BigQuery console, you can now use the **Reference** panel to do the following:

- In the query editor, you can use the [Reference panel](https://docs.cloud.google.com/bigquery/docs/running-queries#use-reference-panel) to preview the schema details of tables, snapshots, views, and materialized views, or open these resources in a new tab. You can also use the panel to construct new queries or edit existing queries by inserting query snippets or field names.

- In the notebook editor, you can use the [Reference panel](https://docs.cloud.google.com/bigquery/docs/create-notebooks#create-notebook-console) to preview the schema details of tables, snapshots, views, or materialized views, or open these resources in a new tab.

This feature is [generally available](https://cloud.google.com/products#product-launch-stages) (GA).
Feature When you use the Data Science Agent in BigQuery, you can now use the [table selector](https://docs.cloud.google.com/bigquery/docs/colab-data-science-agent#analyze-table) to choose one or more BigQuery tables to analyze. The Data Science Agent is in [Preview](https://cloud.google.com/products#product-launch-stages).

## August 14, 2025

Feature You can use [cross region federated queries](https://cloud.google.com/bigquery/docs/spanner-federated-queries#cross_region_queries) to query Spanner tables from regions other than the source BigQuery region. These cross region queries incur additional [Spanner network egress charges](https://docs.cloud.google.com/spanner/pricing#network). This feature is [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).
Feature You can now [visualize your geospatial query results](https://docs.cloud.google.com/bigquery/docs/geospatial-visualize) on an interactive map in BigQuery studio. This feature is in [preview](https://cloud.google.com/products#product-launch-stages).

## August 13, 2025

Feature You can aggregate table data with Gemini assistance in your [BigQuery data preparations](https://docs.cloud.google.com/bigquery/docs/data-prep-get-suggestions). Aggregations in data preparations are in [Preview](https://cloud.google.com/products#product-launch-stages).

## August 12, 2025

Feature You can now [save query results to Cloud Storage](https://docs.cloud.google.com/bigquery/docs/export-file#saving-query-results-to-cloud-storage). This feature is [generally available](https://cloud.google.com/products#product-launch-stages) (GA).

## August 11, 2025

Feature BigQuery [resource utilization charts](https://docs.cloud.google.com/bigquery/docs/admin-resource-charts#view-resource-utilization) are [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).
Feature You can now use [chained function call syntax](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/functions-reference#chained-function-calls) in GoogleSQL to make deeply nested function calls easier to read. This feature is [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).
Feature You can now use [`WITH` expressions](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/operators#with_expression) in your GoogleSQL queries to create temporary variables. This feature is [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).
Change BigQuery data preparations are now represented in the SQLX format and in the pipe query syntax to simplify the CI/CD code review process. For more information, see [Manage data preparations](https://docs.cloud.google.com/bigquery/docs/manage-data-preparations).

## August 06, 2025

Feature Enabling the advanced runtime now includes [short query optimizations](https://docs.cloud.google.com/bigquery/docs/advanced-runtime#short_query_optimizations). This feature is in [preview](https://cloud.google.com/products#product-launch-stages).

## August 04, 2025

Libraries

### Java

#### [2.54.0](https://github.com/googleapis/java-bigquery/compare/v2.53.0...v2.54.0) (2025-07-31)

##### Features

- **bigquery:** Add OpenTelemetry Samples ([#3899](https://github.com/googleapis/java-bigquery/issues/3899)) ([e3d9ed9](https://github.com/googleapis/java-bigquery/commit/e3d9ed92ca5d9b58b5747960d74f895ed8733ebf))
- **bigquery:** Add otel metrics to request headers ([#3900](https://github.com/googleapis/java-bigquery/issues/3900)) ([4071e4c](https://github.com/googleapis/java-bigquery/commit/4071e4cb2547b236183fd4fbb92c73f074cf2fa0))

##### Dependencies

- update dependency com.google.cloud:google-cloud-bigquerystorage-bom to v3.16.1 (#3912) (https://github.com/googleapis/java-bigquery/commit/bb6f6dcb90b1ddf72e630c4dc64737cf2c2ebd2e)
- Update dependency com.google.api.grpc:proto-google-cloud-bigqueryconnection-v1 to v2.70.0 ([#3890](https://github.com/googleapis/java-bigquery/issues/3890)) ([84207e2](https://github.com/googleapis/java-bigquery/commit/84207e297eec75bcb4f1cc1b64423d7c2ddd6c30))
- Update dependency com.google.apis:google-api-services-bigquery to v2-rev20250706-2.0.0 ([#3910](https://github.com/googleapis/java-bigquery/issues/3910)) ([ae5c971](https://github.com/googleapis/java-bigquery/commit/ae5c97146c7076e90c000fd98b797ec8e08a9cd8))
- Update dependency com.google.cloud:sdk-platform-java-config to v3.50.2 ([#3901](https://github.com/googleapis/java-bigquery/issues/3901)) ([8205623](https://github.com/googleapis/java-bigquery/commit/82056237f194a6c99ec4fb3a4315023efdedff1b))
- Update dependency io.opentelemetry:opentelemetry-api to v1.52.0 ([#3902](https://github.com/googleapis/java-bigquery/issues/3902)) ([772407b](https://github.com/googleapis/java-bigquery/commit/772407b12f4da005f79eafc944d4c53f0eec5c27))
- Update dependency io.opentelemetry:opentelemetry-bom to v1.52.0 ([#3903](https://github.com/googleapis/java-bigquery/issues/3903)) ([509a6fc](https://github.com/googleapis/java-bigquery/commit/509a6fc0bb7e7a101bf0d4334a3ff1adde2cab09))
- Update dependency io.opentelemetry:opentelemetry-context to v1.52.0 ([#3904](https://github.com/googleapis/java-bigquery/issues/3904)) ([96c1bae](https://github.com/googleapis/java-bigquery/commit/96c1bae0fcdfdfc2dbb25dcae5007c5d02111a8c))
- Update dependency io.opentelemetry:opentelemetry-exporter-logging to v1.52.0 ([#3905](https://github.com/googleapis/java-bigquery/issues/3905)) ([28ee4c9](https://github.com/googleapis/java-bigquery/commit/28ee4c941b99b1fe3803aefbe7a8ae57100d76cb))
Feature You can now use the new [Data Science Agent (DSA)](https://docs.cloud.google.com/bigquery/docs/colab-data-science-agent) for Colab Enterprise and BigQuery to automate exploratory data analysis, perform
machine learning tasks, and deliver insights all within a
Colab Enterprise notebook. This feature is in [preview](https://cloud.google.com/products#product-launch-stages).

## July 31, 2025

Feature You can manage data profile scans and data quality scans across your project by using the **Metadata curation** page in the Google Cloud console. For more information, see [Profile your data](https://docs.cloud.google.com/bigquery/docs/data-profile-scan) and [Scan for data quality issues](https://docs.cloud.google.com/bigquery/docs/data-quality-scan). This feature is [generally available](https://cloud.google.com/products#product-launch-stages) (GA).
Change BigQuery ML now can automatically detect model quota increases in Vertex AI, and automatically adjusts the quota for any BigQuery ML functions that use those models. You no longer need to email the BigQuery ML team to increase model quota.
Change BigQuery ML has improved throughput by more than 100x for the following
generative AI functions:

- [`ML.GENERATE_TEXT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-generate-text)
- [`AI.GENERATE_TABLE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-generate-table)
- [`AI.GENERATE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-generate)
- [`AI.GENERATE_BOOL`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-generate-bool)
- [`AI.GENERATE_DOUBLE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-generate-double)
- [`AI.GENERATE_INT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-generate-int)

Actual performance varies based on the number of input and output tokens in the
request, but a typical 6-hour job can now process millions of rows. For more
information, see
[Generative AI functions](https://docs.cloud.google.com/bigquery/quotas#generative_ai_functions).
Feature You can now use [continuous queries](https://docs.cloud.google.com/bigquery/docs/continuous-queries-introduction) to [export BigQuery data to Spanner in real time](https://docs.cloud.google.com/bigquery/docs/export-to-spanner). This feature is in [Preview](https://cloud.google.com/products#product-launch-stages).

## July 30, 2025

Announcement The Gemini for Google Cloud API (cloudaicompanion.googleapis.com) is now enabled by default for most BigQuery projects. Exceptions include projects where customers have opted out, and those linked to accounts based in EMEA regions including [BigQuery Europe, Middle East, and Africa regions](https://docs.cloud.google.com/bigquery/docs/locations#supported_locations).

## July 28, 2025

Libraries

### Python

#### [3.35.1](https://github.com/googleapis/python-bigquery/compare/v3.35.0...v3.35.1) (2025-07-21)

##### Documentation

- Specify the inherited-members directive for job classes ([#2244](https://github.com/googleapis/python-bigquery/issues/2244)) ([d207f65](https://github.com/googleapis/python-bigquery/commit/d207f6539b7a4c248a5de5719d7f384abbe20abe))
Libraries

### Node.js

#### [8.1.1](https://github.com/googleapis/nodejs-bigquery/compare/v8.1.0...v8.1.1) (2025-07-23)

##### Bug Fixes

- Remove `is` package as dependency ([#1500](https://github.com/googleapis/nodejs-bigquery/issues/1500)) ([926c9f8](https://github.com/googleapis/nodejs-bigquery/commit/926c9f879521f0c06ab4f96b0b86e426aff3543c))
Feature You can now associate [data policies directly on columns](https://docs.cloud.google.com/bigquery/docs/column-data-masking#data-policies-on-column). This feature enables direct database administration for controlling access and applying masking and transformation rules at the column level. This feature is in [Preview](https://cloud.google.com/products/#product-launch-stages).

## July 22, 2025

Feature You can now use the [`MATCH_RECOGNIZE` clause](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#match_recognize_clause) in your SQL queries to filter and aggregate matches across rows in a table. This feature is in [Preview](https://cloud.google.com/products/#product-launch-stages).
Feature The [`CREATE EXTERNAL TABLE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_external_table_statement) and [`LOAD DATA`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/load-statements) statements now support the following options in [Preview](https://cloud.google.com/products#product-launch-stages):

- `null_markers`: define the strings that represent `NULL` values in CSV files.
- `source_column_match`: specify how loaded columns are matched to the schema. You can match columns by position or by name.
Feature [Access Transparency](https://docs.cloud.google.com/assured-workloads/access-transparency/docs/supported-services) supports [BigQuery data preparation](https://docs.cloud.google.com/bigquery/docs/data-prep-introduction) in the [GA](https://cloud.google.com/products#product-launch-stages) stage.
Feature You can now use the
[`VECTOR_INDEX.STATISTICS` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/vectorindex_functions#vector_indexstatistics) to calculate how much an indexed table's data has drifted between when a
vector index was created and the present. If table data has changed enough
to require a [vector index rebuild](https://docs.cloud.google.com/bigquery/docs/vector-index#rebuild_a_vector_index), you can use the
[`ALTER VECTOR INDEX REBUILD` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#alter_vector_index_rebuild_statement)
to rebuild the vector index. This feature is in [Preview](https://cloud.google.com/products/#product-launch-stages).

## July 21, 2025

Libraries

### Java

#### [2.53.0](https://github.com/googleapis/java-bigquery/compare/v2.52.0...v2.53.0) (2025-07-14)

##### Features

- **bigquery:** Add OpenTelemetry support to BQ rpcs ([#3860](https://github.com/googleapis/java-bigquery/issues/3860)) ([e2d23c1](https://github.com/googleapis/java-bigquery/commit/e2d23c1b15f2c48a4113f82b920f5c29c4b5dfea))
- **bigquery:** Add support for custom timezones and timestamps ([#3859](https://github.com/googleapis/java-bigquery/issues/3859)) ([e5467c9](https://github.com/googleapis/java-bigquery/commit/e5467c917c63ac066edcbcd902cc2093a39971a3))
- Next release from main branch is 2.53.0 ([#3879](https://github.com/googleapis/java-bigquery/issues/3879)) ([c47a062](https://github.com/googleapis/java-bigquery/commit/c47a062136fea4de91190cafb1f11bac6abbbe3a))

##### Bug Fixes

- Load jobs preserve ascii control characters configuration ([#3876](https://github.com/googleapis/java-bigquery/issues/3876)) ([5cfdf85](https://github.com/googleapis/java-bigquery/commit/5cfdf855fa0cf206660fd89743cbaabf3afa75a3))

##### Dependencies

- Update dependency com.google.api.grpc:proto-google-cloud-bigqueryconnection-v1 to v2.69.0 ([#3870](https://github.com/googleapis/java-bigquery/issues/3870)) ([a7f1007](https://github.com/googleapis/java-bigquery/commit/a7f1007b5242da2c0adebbb309a908d7d4db5974))
- Update dependency com.google.apis:google-api-services-bigquery to v2-rev20250615-2.0.0 ([#3872](https://github.com/googleapis/java-bigquery/issues/3872)) ([f081589](https://github.com/googleapis/java-bigquery/commit/f08158955b7fec3c2ced6332b6e4d76cc13f2e90))
- Update dependency com.google.cloud:sdk-platform-java-config to v3.50.1 ([#3878](https://github.com/googleapis/java-bigquery/issues/3878)) ([0e971b8](https://github.com/googleapis/java-bigquery/commit/0e971b8ace013caa31b8a02a21038e94bebae2a5))

##### Documentation

- Update maven format command ([#3877](https://github.com/googleapis/java-bigquery/issues/3877)) ([d2918da](https://github.com/googleapis/java-bigquery/commit/d2918da844cd20ca1602c6fcf9fa1df685f261fc))

### Java

#### [2.53.0](https://github.com/googleapis/java-bigquery/compare/v2.52.0...v2.53.0) (2025-07-14)

##### Features

- **bigquery:** Add OpenTelemetry support to BQ rpcs ([#3860](https://github.com/googleapis/java-bigquery/issues/3860)) ([e2d23c1](https://github.com/googleapis/java-bigquery/commit/e2d23c1b15f2c48a4113f82b920f5c29c4b5dfea))
- **bigquery:** Add support for custom timezones and timestamps ([#3859](https://github.com/googleapis/java-bigquery/issues/3859)) ([e5467c9](https://github.com/googleapis/java-bigquery/commit/e5467c917c63ac066edcbcd902cc2093a39971a3))
- Next release from main branch is 2.53.0 ([#3879](https://github.com/googleapis/java-bigquery/issues/3879)) ([c47a062](https://github.com/googleapis/java-bigquery/commit/c47a062136fea4de91190cafb1f11bac6abbbe3a))

##### Bug Fixes

- Load jobs preserve ascii control characters configuration ([#3876](https://github.com/googleapis/java-bigquery/issues/3876)) ([5cfdf85](https://github.com/googleapis/java-bigquery/commit/5cfdf855fa0cf206660fd89743cbaabf3afa75a3))

##### Dependencies

- Update dependency com.google.api.grpc:proto-google-cloud-bigqueryconnection-v1 to v2.69.0 ([#3870](https://github.com/googleapis/java-bigquery/issues/3870)) ([a7f1007](https://github.com/googleapis/java-bigquery/commit/a7f1007b5242da2c0adebbb309a908d7d4db5974))
- Update dependency com.google.apis:google-api-services-bigquery to v2-rev20250615-2.0.0 ([#3872](https://github.com/googleapis/java-bigquery/issues/3872)) ([f081589](https://github.com/googleapis/java-bigquery/commit/f08158955b7fec3c2ced6332b6e4d76cc13f2e90))
- Update dependency com.google.cloud:sdk-platform-java-config to v3.50.1 ([#3878](https://github.com/googleapis/java-bigquery/issues/3878)) ([0e971b8](https://github.com/googleapis/java-bigquery/commit/0e971b8ace013caa31b8a02a21038e94bebae2a5))

##### Documentation

- Update maven format command ([#3877](https://github.com/googleapis/java-bigquery/issues/3877)) ([d2918da](https://github.com/googleapis/java-bigquery/commit/d2918da844cd20ca1602c6fcf9fa1df685f261fc))
Libraries

### Python

#### [3.35.0](https://github.com/googleapis/python-bigquery/compare/v3.34.0...v3.35.0) (2025-07-15)

##### Features

- Add null_markers property to LoadJobConfig and CSVOptions ([#2239](https://github.com/googleapis/python-bigquery/issues/2239)) ([289446d](https://github.com/googleapis/python-bigquery/commit/289446dd8c356d11a0b63b8e6275629b1ae5dc08))
- Add total slot ms to RowIterator ([#2233](https://github.com/googleapis/python-bigquery/issues/2233)) ([d44bf02](https://github.com/googleapis/python-bigquery/commit/d44bf0231e6e96369e4e03667a3f96618fb664e2))
- Add UpdateMode to update_dataset ([#2204](https://github.com/googleapis/python-bigquery/issues/2204)) ([eb9c2af](https://github.com/googleapis/python-bigquery/commit/eb9c2aff242c5107f968bbd8b6a9d30cecc877f6))
- Adds dataset_view parameter to get_dataset method ([#2198](https://github.com/googleapis/python-bigquery/issues/2198)) ([28a5750](https://github.com/googleapis/python-bigquery/commit/28a5750d455f0381548df6f9b1f7661823837d81))
- Adds date_format to load job and external config ([#2231](https://github.com/googleapis/python-bigquery/issues/2231)) ([7d31828](https://github.com/googleapis/python-bigquery/commit/7d3182802deccfceb0646b87fc8d12275d0a569b))
- Adds datetime_format as an option ([#2236](https://github.com/googleapis/python-bigquery/issues/2236)) ([54d3dc6](https://github.com/googleapis/python-bigquery/commit/54d3dc66244d50a031e3c80d43d372d2743ecbc3))
- Adds source_column_match and associated tests ([#2227](https://github.com/googleapis/python-bigquery/issues/2227)) ([6d5d236](https://github.com/googleapis/python-bigquery/commit/6d5d23685cd457d85955356705c1101e9ec3cdcd))
- Adds time_format and timestamp_format and associated tests ([#2238](https://github.com/googleapis/python-bigquery/issues/2238)) ([371ad29](https://github.com/googleapis/python-bigquery/commit/371ad292df537278767dba71d81822ed57dd8e7d))
- Adds time_zone to external config and load job ([#2229](https://github.com/googleapis/python-bigquery/issues/2229)) ([b2300d0](https://github.com/googleapis/python-bigquery/commit/b2300d032843512b7e4a5703377632fe60ef3f8d))

##### Bug Fixes

- Adds magics.context.project to eliminate issues with unit tests ... ([#2228](https://github.com/googleapis/python-bigquery/issues/2228)) ([27ff3a8](https://github.com/googleapis/python-bigquery/commit/27ff3a89a5f97305fa3ff673aa9183baa7df200f))
- Fix rows returned when both start_index and page_size are provided ([#2181](https://github.com/googleapis/python-bigquery/issues/2181)) ([45643a2](https://github.com/googleapis/python-bigquery/commit/45643a2e20ce5d503118522dd195aeca00dec3bc))
- Make AccessEntry equality consistent with from_api_repr ([#2218](https://github.com/googleapis/python-bigquery/issues/2218)) ([4941de4](https://github.com/googleapis/python-bigquery/commit/4941de441cb32cabeb55ec0320f305fb62551155))
- Update type hints for various BigQuery files ([#2206](https://github.com/googleapis/python-bigquery/issues/2206)) ([b863291](https://github.com/googleapis/python-bigquery/commit/b86329188ba35e61871db82ae1d95d2a576eed1b))

##### Documentation

- Improve clarity of "Output Only" fields in Dataset class ([#2201](https://github.com/googleapis/python-bigquery/issues/2201)) ([bd5aba8](https://github.com/googleapis/python-bigquery/commit/bd5aba8ba40c2f35fb672a68eed11d6baedb304f))
Feature You can now use the [`DISTINCT` pipe operator](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/pipe-syntax#distinct_pipe_operator) to select distinct rows from a table in your pipe syntax queries. This feature is [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).

## July 17, 2025

Feature You can now use the [`WITH` pipe operator](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/pipe-syntax#with_pipe_operator) to define common table expressions in your pipe syntax queries. This feature is [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).
Feature You can now use [named windows](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/pipe-syntax#select_pipe_operator) in your pipe syntax queries. This feature is [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).

## July 16, 2025

Feature You can now create [BigQuery ML models by using the
Google Cloud console user interface](https://cloud.google.com/bigquery/docs/create-machine-learning-model-console). This feature is in [Preview](https://cloud.google.com/products/#product-launch-stages).
Feature You can now add comments to [notebooks](https://docs.cloud.google.com/bigquery/docs/create-notebooks#create-notebook-console), [data canvases](https://docs.cloud.google.com/bigquery/docs/data-canvas#work-with-data-canvas), [data preparation files](https://docs.cloud.google.com/bigquery/docs/data-prep-get-suggestions#open-data-prep-editor), or [saved queries](https://docs.cloud.google.com/bigquery/docs/work-with-saved-queries#create_saved_queries). You can also reply to existing comments or get a link to them. This feature is in [Preview](https://cloud.google.com/products#product-launch-stages).

## July 15, 2025

Feature You can flatten [JSON columns](https://docs.cloud.google.com/bigquery/docs/data-prep-get-suggestions#flatten-json) in BigQuery data preparation with a single operation. This feature is [generally available](https://cloud.google.com/products#product-launch-stages) (GA).
Feature You can now [commercialize your BigQuery sharing listings on Google Cloud Marketplace](https://docs.cloud.google.com/bigquery/docs/analytics-hub-cloud-marketplace). This feature is [generally available](https://cloud.google.com/products#product-launch-stages) (GA).

## July 08, 2025

Announcement Starting August 1, 2025, GoogleSQL will become the default dialect for queries run from the command line interface (CLI) or API. To use LegacySQL, you will need to explicitly specify it in your requests or [set the configuration setting](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#alter_project_set_options_statement) `default_sql_dialect_option` to `'default_legacy_sql'` at the project or organization level.

## July 07, 2025

Feature You can now use your Google Account user credentials to authorize the execution of a data preparation in development. For more information, see
[Manually run a data preparation in development](https://docs.cloud.google.com/bigquery/docs/orchestrate-data-preparations#run-undeployed-manually). This feature is in [preview](https://cloud.google.com/products#product-launch-stages).

## July 01, 2025

Feature You can now [update a Cloud KMS encryption key](https://docs.cloud.google.com/bigquery/docs/customer-managed-encryption#key_rotation) by updating the table with the same key. This feature is [generally available](https://cloud.google.com/products#product-launch-stages) (GA).
Feature You can use the [`@@location` system variable](https://docs.cloud.google.com/bigquery/docs/reference/system-variables) to set the location in which to run a query. This feature is [generally available](https://cloud.google.com/products#product-launch-stages) (GA).
Feature BigQuery now supports the following Apache Hadoop migration features in [Preview](https://cloud.google.com/products#product-launch-stages):

- [Use the `dwh-migration-dumper` tool to migrate the metadata](https://docs.cloud.google.com/bigquery/docs/hadoop-metadata) necessary for a Hadoop permissions and data migration.
- [Migrate permissions from Apache Hadoop, Apache Hive, and Ranger HDFS](https://docs.cloud.google.com/bigquery/docs/hadoop-permissions-migration) to BigQuery.
- [Migrate tables from a HDFS data lake](https://docs.cloud.google.com/bigquery/docs/hdfs-data-lake-transfer) to Google Cloud.

## June 30, 2025

Libraries

### Java

#### [2.52.0](https://github.com/googleapis/java-bigquery/compare/v2.51.0...v2.52.0) (2025-06-25)

##### Features

- **bigquery:** Integrate Otel in client lib ([#3747](https://github.com/googleapis/java-bigquery/issues/3747)) ([6e3e07a](https://github.com/googleapis/java-bigquery/commit/6e3e07a22b8397e1e9d5b567589e44abc55961f2))
- **bigquery:** Integrate Otel into retries, jobs, and more ([#3842](https://github.com/googleapis/java-bigquery/issues/3842)) ([4b28c47](https://github.com/googleapis/java-bigquery/commit/4b28c479c1bc22326c8d2501354fb86ec2ce1744))

##### Bug Fixes

- **bigquery:** Add MY_VIEW_DATASET_NAME*TEST* to resource clean up sample ([#3838](https://github.com/googleapis/java-bigquery/issues/3838)) ([b1962a7](https://github.com/googleapis/java-bigquery/commit/b1962a7f0084ee4c3e248266b50406cf575cd657))

##### Dependencies

- Remove version declaration of open-telemetry-bom ([#3855](https://github.com/googleapis/java-bigquery/issues/3855)) ([6f9f77d](https://github.com/googleapis/java-bigquery/commit/6f9f77d47596b00b7317c8a0d4a10c3d849ad57b))
- Update dependency com.google.api.grpc:proto-google-cloud-bigqueryconnection-v1 to v2.66.0 ([#3835](https://github.com/googleapis/java-bigquery/issues/3835)) ([69be5e7](https://github.com/googleapis/java-bigquery/commit/69be5e7345fb8ca69d633d9dc99cf6c15fa5227b))
- Update dependency com.google.api.grpc:proto-google-cloud-bigqueryconnection-v1 to v2.68.0 ([#3858](https://github.com/googleapis/java-bigquery/issues/3858)) ([d4ca353](https://github.com/googleapis/java-bigquery/commit/d4ca3535f54f3282aec133337103bbfa2c9a3653))
- Update dependency com.google.cloud:sdk-platform-java-config to v3.49.2 ([#3853](https://github.com/googleapis/java-bigquery/issues/3853)) ([cf864df](https://github.com/googleapis/java-bigquery/commit/cf864df739bbb820e99999b7c1592a3635fea4ec))
- Update dependency com.google.cloud:sdk-platform-java-config to v3.50.0 ([#3861](https://github.com/googleapis/java-bigquery/issues/3861)) ([eb26dee](https://github.com/googleapis/java-bigquery/commit/eb26deee37119389aee3962eea5ad67d63f26c70))
- Update dependency io.opentelemetry:opentelemetry-bom to v1.51.0 ([#3840](https://github.com/googleapis/java-bigquery/issues/3840)) ([51321c2](https://github.com/googleapis/java-bigquery/commit/51321c22778fd41134cc0cdfc70bdc47f05883f1))
- Update ossf/scorecard-action action to v2.4.2 ([#3810](https://github.com/googleapis/java-bigquery/issues/3810)) ([414f61d](https://github.com/googleapis/java-bigquery/commit/414f61d7efcfa568c1446bd41945d7a8e2450649))
Feature You can now [create and manage scheduled notebooks](https://docs.cloud.google.com/bigquery/docs/orchestrate-notebooks) using the **Schedule details** pane in BigQuery Studio. This feature is [generally available](https://cloud.google.com/products#product-launch-stages) (GA).

## June 26, 2025

Feature [BigQuery search indexes](https://docs.cloud.google.com/bigquery/docs/search-intro) provide free index management until your organization reaches the [limit](https://docs.cloud.google.com/bigquery/quotas#index_limits) in a given region. You can now use the [`INFORMATION_SCHEMA.SEARCH_INDEXES_BY_ORGANIZATION` view](https://docs.cloud.google.com/bigquery/docs/information-schema-indexes-by-organization) to understand your current consumption towards that limit, broken down by projects and tables. This feature is [generally available](https://cloud.google.com/products#product-launch-stages) (GA).
Feature You can now use the
use the `PARTITION BY` clause of the
[`CREATE VECTOR INDEX` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_vector_index_statement) to [partition TreeAH vector indexes](https://docs.cloud.google.com/bigquery/docs/vector-index#partitions). Partitioning enables partition pruning and can decrease I/O costs. This feature is in [preview](https://cloud.google.com/products/#product-launch-stages).

## June 23, 2025

Feature You can now use the [Apache Iceberg REST catalog in BigLake metastore](https://docs.cloud.google.com/bigquery/docs/blms-rest-catalog) to create interoperability between your query engines by allowing your open source engines to access Iceberg data in Cloud Storage. This feature is in [Preview](https://cloud.google.com/products#product-launch-stages).
Feature Colab Enterprise notebooks in BigQuery let you do the following in [Preview](https://cloud.google.com/products/#product-launch-stages):

- [Explain code with Gemini assistance](https://docs.cloud.google.com/bigquery/docs/write-sql-gemini#explain_python_code)
- [Fix and explain errors with Gemini assistance](https://docs.cloud.google.com/bigquery/docs/write-sql-gemini#fix_and_explain_python_errors)

## June 18, 2025

Feature You can now [publish the results of a data quality scan as Dataplex Universal Catalog metadata](https://docs.cloud.google.com/bigquery/docs/data-quality-scan). Previously, data quality scan results were published only to the Google Cloud console. The latest results are saved to the entry that represents the source table. You can view the results in the Google Cloud console. If you want to enable catalog publishing for an existing data quality scan, you must edit the scan and re-enable the publishing option. This feature is [generally available](https://cloud.google.com/products#product-launch-stages) (GA).
Feature You can now use data insights to have Gemini [generate table and column descriptions from table metadata](https://docs.cloud.google.com/bigquery/docs/data-insights). This feature is in [Preview](https://cloud.google.com/products#product-launch-stages).

## June 16, 2025

Feature The BigQuery migration assessment is now available for workflows that use [Cloudera and Apache Hadoop](https://docs.cloud.google.com/bigquery/docs/migration-assessment#hadoop-cloudera). This feature is in [Preview](https://cloud.google.com/products#product-launch-stages).
Feature You can now manage [IAM tags](https://docs.cloud.google.com/bigquery/docs/tags) on BigQuery datasets and tables using SQL. This feature is [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).
Feature The [Merchant Center best sellers report](https://docs.cloud.google.com/bigquery/docs/merchant-center-best-sellers-schema) supports multi-client accounts (MCAs). If you have an MCA, you can use the `aggregator_id` to query the tables. The `BestSellersEntityProductMapping` table maps the best-selling entities to the products in the sub-accounts' inventory. This provides a consolidated view of best-selling products, which you can then join with product data for more detailed insights. This feature is [generally available](https://cloud.google.com/products#product-launch-stages) (GA).
Feature In BigQuery ML, you can now forecast multiple time series at once by using the [`TIME_SERIES_ID_COL` option](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-multivariate-time-series#time_series_id_col) that is available in `ARIMA_PLUS_XREG` multivariate time series models. Try this feature with the [Forecast multiple time series with a multivariate model](https://docs.cloud.google.com/bigquery/docs/arima-plus-xreg-multiple-time-series-forecasting-tutorial) tutorial. This feature is [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).
Feature BigQuery now offers the following Gemini-enhanced SQL translation features:

- Create [Gemini-based configuration YAML files](https://docs.cloud.google.com/bigquery/docs/config-yaml-translation#ai_yaml_guidelines) to generate AI suggestions for batch or interactive SQL translations. This feature is now [generally available](https://cloud.google.com/products#product-launch-stages) (GA).
- After making a batch SQL translation, review your translation output, including Gemini-based suggestions, using the [code tab](https://docs.cloud.google.com/bigquery/docs/batch-sql-translator#code-tab) and [configuration tab](https://docs.cloud.google.com/bigquery/docs/batch-sql-translator#configuration_tab). This feature is now [generally available](https://cloud.google.com/products#product-launch-stages) (GA).
- When making an interactive SQL translation, [create and apply Gemini-enhanced translation rules](https://docs.cloud.google.com/bigquery/docs/interactive-sql-translator#create-apply-rules) to customize your SQL inputs. This feature is in [Preview](https://cloud.google.com/products#product-launch-stages).

## June 12, 2025

Feature Dark theme is now available for BigQuery in [Preview](https://cloud.google.com/products#product-launch-stages). To enable the dark theme, in the Google Cloud console, click **Settings and utilities \> Preferences** . In the navigation menu, click **Appearance** , and then select your color theme and click **Save**.

## June 11, 2025

Feature The following GoogleSQL functions are now available in [preview](https://cloud.google.com/products#product-launch-stages):

- The [`ARRAY_FIRST` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/array_functions#array_first) returns the first element of the input array.
- The [`ARRAY_LAST` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/array_functions#array_last) returns the last element of the input array.
- The [`ARRAY_SLICE` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/array_functions#array_slice) returns an array that contains consecutive elements from the input array.

## June 10, 2025

Change An updated version of the [ODBC driver for BigQuery](https://docs.cloud.google.com/bigquery/docs/reference/odbc-jdbc-drivers#odbc_release_3121009) is now available.
Feature For [supported Gemini models](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/provisioned-throughput/supported-models), you can now use [Vertex AI Provisioned Throughput](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/provisioned-throughput/overview) with the [`ML.GENERATE_TEXT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-generate-text#provisioned-throughput)and [`AI.GENERATE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-generate#provisioned-throughput) functions to provide consistent high throughput for requests.

This feature is [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).

## June 09, 2025

Libraries

### Java

#### [2.51.0](https://github.com/googleapis/java-bigquery/compare/v2.50.1...v2.51.0) (2025-06-06)

##### Features

- **bigquery:** Job creation mode GA ([#3804](https://github.com/googleapis/java-bigquery/issues/3804)) ([a21cde8](https://github.com/googleapis/java-bigquery/commit/a21cde8994e93337326cc4a2deb4bafd1596b77f))
- **bigquery:** Support Fine Grained ACLs for Datasets ([#3803](https://github.com/googleapis/java-bigquery/issues/3803)) ([bebf1c6](https://github.com/googleapis/java-bigquery/commit/bebf1c610e6d050c49fc05f30d3fa0247b7dfdcb))

##### Dependencies

- Rollback netty.version to v4.1.119.Final ([#3827](https://github.com/googleapis/java-bigquery/issues/3827)) ([94c71a0](https://github.com/googleapis/java-bigquery/commit/94c71a090eab745c81dd9530bcdd3c8c1e734788))
- Update dependency com.google.api.grpc:proto-google-cloud-bigqueryconnection-v1 to v2.65.0 ([#3787](https://github.com/googleapis/java-bigquery/issues/3787)) ([0574ecc](https://github.com/googleapis/java-bigquery/commit/0574eccec2975738804be7d0ccb4c973459c82c9))
- Update dependency com.google.apis:google-api-services-bigquery to v2-rev20250511-2.0.0 ([#3794](https://github.com/googleapis/java-bigquery/issues/3794)) ([d3bf724](https://github.com/googleapis/java-bigquery/commit/d3bf724feef91469b44e1e5068738604d2b3cead))
- Update dependency com.google.cloud:sdk-platform-java-config to v3.49.0 ([#3811](https://github.com/googleapis/java-bigquery/issues/3811)) ([2c5ede4](https://github.com/googleapis/java-bigquery/commit/2c5ede4b115cf7cdd078d54d29ce93636c1cedf5))
Feature You can reference [Iceberg external tables in materialized views](https://docs.cloud.google.com/bigquery/docs/materialized-views-create#iceberg) instead of migrating that data to BigQuery-managed storage. This feature is [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).

## June 04, 2025

Change The organization-level [configuration settings](https://docs.cloud.google.com/bigquery/docs/default-configuration) for `default_sql_dialect_option` and `query_runtime` are unsupported.

## June 03, 2025

Feature BigQuery metastore has been renamed [BigLake metastore](https://docs.cloud.google.com/bigquery/docs/about-blms) and is now [generally available](https://cloud.google.com/products#product-launch-stages) (GA). The feature formerly known as BigLake metastore has been renamed BigLake metastore (classic).
Feature You can now use the [BigQuery advanced runtime](https://docs.cloud.google.com/bigquery/docs/advanced-runtime) to improve query execution time and slot usage. This feature is in [Preview](https://cloud.google.com/products#product-launch-stages).
Feature BigQuery tables for Apache Iceberg have been renamed [BigLake tables for Apache Iceberg in BigQuery](https://docs.cloud.google.com/bigquery/docs/iceberg-tables). This feature is now [generally available](https://cloud.google.com/products#product-launch-stages) (GA).

## June 02, 2025

Libraries

### Python

#### [3.34.0](https://github.com/googleapis/python-bigquery/compare/v3.33.0...v3.34.0) (2025-05-27)

##### Features

- Job creation mode GA ([#2190](https://github.com/googleapis/python-bigquery/issues/2190)) ([64cd39f](https://github.com/googleapis/python-bigquery/commit/64cd39fb395c4a03ef6d2ec8261e1709477b2186))

##### Bug Fixes

- **deps:** Update all dependencies ([#2184](https://github.com/googleapis/python-bigquery/issues/2184)) ([12490f2](https://github.com/googleapis/python-bigquery/commit/12490f2f03681516465fc34217dcdf57000f6fdd))

##### Documentation

- Update query.py ([#2192](https://github.com/googleapis/python-bigquery/issues/2192)) ([9b5ee78](https://github.com/googleapis/python-bigquery/commit/9b5ee78f046d9ca3f758eeca6244b8485fe35875))
- Use query_and_wait in the array parameters sample ([#2202](https://github.com/googleapis/python-bigquery/issues/2202)) ([28a9994](https://github.com/googleapis/python-bigquery/commit/28a9994792ec90a6a4d16835faf2137c09c0fb02))
Libraries

### Go

#### [1.69.0](https://github.com/googleapis/google-cloud-go/compare/bigquery/v1.68.0...bigquery/v1.69.0) (2025-05-27)

##### Features

- **bigquery/analyticshub:** Add support for Analytics Hub \& Marketplace Integration ([2aaada3](https://github.com/googleapis/google-cloud-go/commit/2aaada3fb7a9d3eaacec3351019e225c4038646b))
- **bigquery/analyticshub:** Adding allow_only_metadata_sharing to Listing resource ([2aaada3](https://github.com/googleapis/google-cloud-go/commit/2aaada3fb7a9d3eaacec3351019e225c4038646b))
- **bigquery/analyticshub:** Adding CommercialInfo message to the Listing and Subscription resources ([2aaada3](https://github.com/googleapis/google-cloud-go/commit/2aaada3fb7a9d3eaacec3351019e225c4038646b))
- **bigquery/analyticshub:** Adding delete_commercial and revoke_commercial to DeleteListingRequest and RevokeSubscriptionRequest ([2aaada3](https://github.com/googleapis/google-cloud-go/commit/2aaada3fb7a9d3eaacec3351019e225c4038646b))
- **bigquery/analyticshub:** Adding DestinationDataset to the Subscription resource ([2aaada3](https://github.com/googleapis/google-cloud-go/commit/2aaada3fb7a9d3eaacec3351019e225c4038646b))
- **bigquery/analyticshub:** Adding routine field to the SharedResource message ([2aaada3](https://github.com/googleapis/google-cloud-go/commit/2aaada3fb7a9d3eaacec3351019e225c4038646b))
- **bigquery:** Add support for dataset view and update modes ([#12290](https://github.com/googleapis/google-cloud-go/issues/12290)) ([7c1f961](https://github.com/googleapis/google-cloud-go/commit/7c1f9616b7ea95436582eb3c40c94e6bd9b48610))
- **bigquery:** Job creation mode GA ([#12225](https://github.com/googleapis/google-cloud-go/issues/12225)) ([1d8990d](https://github.com/googleapis/google-cloud-go/commit/1d8990dbf2563a5fbc96769ac9c6ea4ed06b239e))
Libraries

### Node.js

#### [8.1.0](https://github.com/googleapis/nodejs-bigquery/compare/v8.0.0...v8.1.0) (2025-05-29)

##### Features

- Job creation mode GA ([#1480](https://github.com/googleapis/nodejs-bigquery/issues/1480)) ([b51359a](https://github.com/googleapis/nodejs-bigquery/commit/b51359a61d93a5d9cff729221f457a50a5c7a52f))
- Support per-job reservation assignment ([#1477](https://github.com/googleapis/nodejs-bigquery/issues/1477)) ([8151e72](https://github.com/googleapis/nodejs-bigquery/commit/8151e72bb1e149f6f36f7acdba25629d208b1074))
Feature BigQuery now supports using [Spanner external datasets](https://docs.cloud.google.com/bigquery/docs/spanner-external-datasets) with [authorized views](https://docs.cloud.google.com/bigquery/docs/authorized-views), [authorized routines](https://docs.cloud.google.com/bigquery/docs/authorized-routines), and [Cloud resource connections](https://docs.cloud.google.com/bigquery/docs/create-cloud-resource-connection). This feature is [generally available](https://cloud.google.com/products?e=48754805&hl=en#product-launch-stages) (GA).
Feature The [`CREATE EXTERNAL TABLE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_external_table_statement) and [`LOAD DATA`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/load-statements) statements now support the following options in [preview](https://cloud.google.com/products/#product-launch-stages):

- `time_zone`: specify a time zone to use when loading data
- `date_format`, `datetime_format`, `time_format`, and `timestamp_format`: define how date and time values are formatted in your source files
Feature In the navigation menu, you can now go to **Settings** and select **Configuration settings** to [customize the BigQuery Studio experience](https://docs.cloud.google.com/bigquery/docs/bigquery-web-ui#navigation_menu) for users within the selected project or organization. This is achieved by showing or hiding user interface elements. This feature is in [preview](https://cloud.google.com/products#product-launch-stages).
Feature In the BigQuery console, in the **Welcome** tab, you can now try the [Apache Spark demo notebook](https://docs.cloud.google.com/bigquery/docs/bigquery-web-ui#run_spark_notebook_demo_guide) that walks you through the basics of Spark notebook and showcases [serverless Spark in BigQuery](https://docs.cloud.google.com/bigquery/docs/use-spark). This feature is [generally available](https://cloud.google.com/products#product-launch-stages) (GA).

## May 29, 2025

Feature You can now use the `dbt-bigquery` adapter to run Python code that's defined in BigQuery DataFrames. For more information, see [Use BigQuery DataFrames in dbt](https://docs.cloud.google.com/bigquery/docs/dataframes-dbt). This feature is in [preview](https://cloud.google.com/products#product-launch-stages).
Feature You can now use your Google Account user credentials to authorize the creation, scheduling, and running of pipelines as well as the scheduling of notebooks and data preparations. For more information, see [Create a pipeline schedule](https://docs.cloud.google.com/bigquery/docs/schedule-pipelines#create-schedule). This feature is in [preview](https://cloud.google.com/products#product-launch-stages).
Feature You can now create [event-driven transfers](https://docs.cloud.google.com/bigquery/docs/event-driven-transfer) when transferring data from Cloud Storage to BigQuery. Event-driven transfers can automatically trigger transfer runs when data in your Cloud Storage bucket has been modified or added. This feature is [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).

## May 28, 2025

Feature You can now create a [serverless Spark session and run PySpark code in a BigQuery notebook](https://docs.cloud.google.com/bigquery/docs/use-spark). This feature is [generally available](https://cloud.google.com/products#product-launch-stages) (GA).
Feature Column metadata indexing is now available for both [BigQuery tables](https://docs.cloud.google.com/bigquery/docs/metadata-indexing-managed-tables) and [external tables](https://docs.cloud.google.com/bigquery/docs/metadata-caching-external-tables). This feature is [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).

## May 27, 2025

Feature BigQuery offers [optional job creation mode](https://docs.cloud.google.com/bigquery/docs/running-queries#optional-job-creation) to speed up small queries that you use in your dashboards, data exploration, and other workflows. This mode automatically optimizes eligible queries and uses a cache to improve latency. This feature is [generally available](https://cloud.google.com/products#product-launch-stages) (GA).
Feature You can now [share Pub/Sub streaming data through BigQuery sharing](https://docs.cloud.google.com/bigquery/docs/analytics-hub-stream-sharing) with additional [client libraries](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub) support and provider usage metrics. This feature is [generally available](https://cloud.google.com/products#product-launch-stages) (GA).

## May 26, 2025

Libraries

### Python

#### [3.33.0](https://github.com/googleapis/python-bigquery/compare/v3.32.0...v3.33.0) (2025-05-19)

##### Features

- Add ability to set autodetect_schema query param in update_table ([#2171](https://github.com/googleapis/python-bigquery/issues/2171)) ([57f940d](https://github.com/googleapis/python-bigquery/commit/57f940d957613b4d80fb81ea40a1177b73856189))
- Add dtype parameters to to_geodataframe functions ([#2176](https://github.com/googleapis/python-bigquery/issues/2176)) ([ebfd0a8](https://github.com/googleapis/python-bigquery/commit/ebfd0a83d43bcb96f65f5669437220aa6138b766))
- Support job reservation ([#2186](https://github.com/googleapis/python-bigquery/issues/2186)) ([cb646ce](https://github.com/googleapis/python-bigquery/commit/cb646ceea172bf199f366ae0592546dff2d3bcb2))

##### Bug Fixes

- Ensure AccessEntry equality and repr uses the correct `entity_type` ([#2182](https://github.com/googleapis/python-bigquery/issues/2182)) ([0217637](https://github.com/googleapis/python-bigquery/commit/02176377d5e2fc25b5cd4f46aa6ebfb1b6a960a6))
- Ensure SchemaField.field_dtype returns a string ([#2188](https://github.com/googleapis/python-bigquery/issues/2188)) ([7ec2848](https://github.com/googleapis/python-bigquery/commit/7ec2848379d5743bbcb36700a1153540c451e0e0))
Libraries

### Java

#### [2.50.1](https://github.com/googleapis/java-bigquery/compare/v2.50.0...v2.50.1) (2025-05-16)

##### Dependencies

- Update dependency com.google.cloud:sdk-platform-java-config to v3.48.0 ([#3790](https://github.com/googleapis/java-bigquery/issues/3790)) ([206f06d](https://github.com/googleapis/java-bigquery/commit/206f06de115ead53b26f09a5f4781efd279b5a73))
- Update netty.version to v4.2.1.final ([#3780](https://github.com/googleapis/java-bigquery/issues/3780)) ([6dcd858](https://github.com/googleapis/java-bigquery/commit/6dcd858eca788a8cb571368e12b4925993e380c4))

##### Documentation

- **bigquery:** Update TableResult.getTotalRows() docstring ([#3785](https://github.com/googleapis/java-bigquery/issues/3785)) ([6483588](https://github.com/googleapis/java-bigquery/commit/6483588a3c5785b95ea841f21aa38f50ecf4226d))

## May 22, 2025

Change Starting March 17 2026, the `bigquery.datasets.getIamPolicy`
IAM permission is required to view a dataset's access controls and to query the
[`INFORMATION_SCHEMA.OBJECT_PRIVILEGES`](https://docs.cloud.google.com/bigquery/docs/information-schema-object-privileges)
view. The `bigquery.datasets.setIamPolicy` permission is required to update a
dataset's access controls or to [create a dataset with access controls using the
API](https://cloud.google.com/bigquery/docs/dataset-access-control#changes_to_api_methods). For more information on this change and how to opt into early enforcement, see [Changes to dataset-level access controls](https://docs.cloud.google.com/bigquery/docs/dataset-access-control).
Feature When you migrate Teradata data to BigQuery using the BigQuery Data Transfer Service, you can now [specify the outputs of the BigQuery translation engine to use as schema mapping](https://docs.cloud.google.com/bigquery/docs/migration/teradata-overview#using_translation_engine_output_for_schema). This feature is in [preview](https://cloud.google.com/products/#product-launch-stages).
Feature You can use [custom constraints](https://docs.cloud.google.com/bigquery/docs/custom-constraints) with Organization Policy to provide more granular control over specific fields for some BigQuery resources. This feature is available in [Preview](https://cloud.google.com/products#product-launch-stages).
Feature When you [Set up Gemini in BigQuery](https://docs.cloud.google.com/gemini/docs/bigquery/set-up-gemini) you are now prompted to grant the BigQuery Studio User and BigQuery Studio Admin roles. These roles now include permission to use Gemini in BigQuery features. This feature is [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).
Feature You can select multiple columns and perform data preparation tasks on them, including dropping columns. For more information, see [Prepare data with Gemini](https://docs.cloud.google.com/bigquery/docs/data-prep-get-suggestions). This feature is [generally available](https://cloud.google.com/products#product-launch-stages) (GA).

## May 21, 2025

Change You can now perform [supervised tuning](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model#supervised_tuning) on a BigQuery ML [remote model](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model) based on a Vertex AI `gemini-2.0-flash-001` or `gemini-2.0-flash-lite-001` model.
Feature You are now able to [set access controls on routines](https://docs.cloud.google.com/bigquery/docs/control-access-to-resources-iam#grant_access_to_a_routine). This feature is in [Preview](https://cloud.google.com/products#product-launch-stages).

## May 19, 2025

Libraries

### Go

#### [1.68.0](https://github.com/googleapis/google-cloud-go/compare/bigquery/v1.67.0...bigquery/v1.68.0) (2025-05-12)

##### Features

- **bigquery/analyticshub:** Support new feature Sharing Cloud Pubsub Streams via AH (GA) and Subscriber Email logging feature ([#11908](https://github.com/googleapis/google-cloud-go/issues/11908)) ([a21d596](https://github.com/googleapis/google-cloud-go/commit/a21d5965fa3f4322da9563425350ba1079279d5a))
- **bigquery/storage:** Increased the number of partitions can be written in a single request ([43bc515](https://github.com/googleapis/google-cloud-go/commit/43bc51591e4ffe7efc76449bb00e3747cda2c944))
- **bigquery:** Add performance insights ([#12101](https://github.com/googleapis/google-cloud-go/issues/12101)) ([aef68ab](https://github.com/googleapis/google-cloud-go/commit/aef68abaa336e0ecd1f488ef6cb3d6b0e8930835))
- **bigquery:** Add some missing fields to BQ stats ([#12212](https://github.com/googleapis/google-cloud-go/issues/12212)) ([77b08e8](https://github.com/googleapis/google-cloud-go/commit/77b08e8e72ece0d56ff8f86dcbfe44b944ab083f))
- **bigquery:** Add WriteTruncateData write disposition ([#12013](https://github.com/googleapis/google-cloud-go/issues/12013)) ([b1126a3](https://github.com/googleapis/google-cloud-go/commit/b1126a3580a0c81c1d7df7cf138d17c748adefbc))
- **bigquery:** New client(s) ([#12228](https://github.com/googleapis/google-cloud-go/issues/12228)) ([f229bd9](https://github.com/googleapis/google-cloud-go/commit/f229bd9b90830d96781d3f9059b64dbfece1690b))
- **bigquery:** Support managed iceberg tables ([#11931](https://github.com/googleapis/google-cloud-go/issues/11931)) ([35e0774](https://github.com/googleapis/google-cloud-go/commit/35e0774bf17166dbaa88eba286f40ad91d9aa68a))
- **bigquery:** Support per-job reservation assignment ([#12078](https://github.com/googleapis/google-cloud-go/issues/12078)) ([c9cebcc](https://github.com/googleapis/google-cloud-go/commit/c9cebcceebc5fb5eecacf99e18652e0c2a53cc6c))

##### Bug Fixes

- **bigquery:** Cache total rows count ([#12230](https://github.com/googleapis/google-cloud-go/issues/12230)) ([202dce0](https://github.com/googleapis/google-cloud-go/commit/202dce02888c5d1d2821732145d5780e5c07ba05)), refs [#11874](https://github.com/googleapis/google-cloud-go/issues/11874) [#11873](https://github.com/googleapis/google-cloud-go/issues/11873)
- **bigquery:** Parse timestamps with timezone info ([#11950](https://github.com/googleapis/google-cloud-go/issues/11950)) ([530d522](https://github.com/googleapis/google-cloud-go/commit/530d522a1f8622e51310680cce31ff1dae007f81))
- **bigquery:** Update google.golang.org/api to 0.229.0 ([3319672](https://github.com/googleapis/google-cloud-go/commit/3319672f3dba84a7150772ccb5433e02dab7e201))
- **bigquery:** Upgrade gRPC service registration func ([7c01015](https://github.com/googleapis/google-cloud-go/commit/7c01015f2aafb5eeb0237accced76b059bc7635d))

##### Documentation

- **bigquery/storage:** Updated the number of partitions (from 100 to 900) can be inserted, updated and deleted in a single request ([43bc515](https://github.com/googleapis/google-cloud-go/commit/43bc51591e4ffe7efc76449bb00e3747cda2c944))
Libraries

### Python

#### [3.32.0](https://github.com/googleapis/python-bigquery/compare/v3.31.0...v3.32.0) (2025-05-12) - YANKED

#### Reason this release was yanked:

PR #2154 caused a performance regression.

##### Features

- Add dataset access policy version attribute ([#2169](https://github.com/googleapis/python-bigquery/issues/2169)) ([b7656b9](https://github.com/googleapis/python-bigquery/commit/b7656b97c1bd6c204d0508b1851d114719686655))
- Add preview support for incremental results ([#2145](https://github.com/googleapis/python-bigquery/issues/2145)) ([22b80bb](https://github.com/googleapis/python-bigquery/commit/22b80bba9d0bed319fd3102e567906c9b458dd02))
- Add WRITE_TRUNCATE_DATA enum ([#2166](https://github.com/googleapis/python-bigquery/issues/2166)) ([4692747](https://github.com/googleapis/python-bigquery/commit/46927479085f13fd326e3f2388f60dfdd37f7f69))
- Adds condition class and assoc. unit tests ([#2159](https://github.com/googleapis/python-bigquery/issues/2159)) ([a69d6b7](https://github.com/googleapis/python-bigquery/commit/a69d6b796d2edb6ba453980c9553bc9b206c5a6e))
- Support BigLakeConfiguration (managed Iceberg tables) ([#2162](https://github.com/googleapis/python-bigquery/issues/2162)) ([a1c8e9a](https://github.com/googleapis/python-bigquery/commit/a1c8e9aaf60986924868d54a0ab0334e77002a39))
- Update the AccessEntry class with a new condition attribute and unit tests ([#2163](https://github.com/googleapis/python-bigquery/issues/2163)) ([7301667](https://github.com/googleapis/python-bigquery/commit/7301667272dfbdd04b1a831418a9ad2d037171fb))

##### Bug Fixes

- `query()` now warns when `job_id` is set and the default `job_retry` is ignored ([#2167](https://github.com/googleapis/python-bigquery/issues/2167)) ([ca1798a](https://github.com/googleapis/python-bigquery/commit/ca1798aaee2d5905fe688d3097f8ee5c989da333))
- Empty record dtypes ([#2147](https://github.com/googleapis/python-bigquery/issues/2147)) ([77d7173](https://github.com/googleapis/python-bigquery/commit/77d71736fcc006d3ab8f8ba17955ad5f06e21876))
- Table iterator should not use bqstorage when page_size is not None ([#2154](https://github.com/googleapis/python-bigquery/issues/2154)) ([e89a707](https://github.com/googleapis/python-bigquery/commit/e89a707b162182ededbf94cc9a0f7594bc2be475))
Feature [Continuous queries](https://docs.cloud.google.com/bigquery/docs/continuous-queries-introduction) let you build long-lived, continuously processing SQL statements that can analyze, process, and perform machine learning (ML) inference on incoming data in BigQuery in real time.

- To monitor your continuous queries, you can use a [custom job ID prefix](https://docs.cloud.google.com/bigquery/docs/continuous-queries#custom-job-id) to simplify filtering or view [metrics specific to continuous queries](https://docs.cloud.google.com/bigquery/docs/monitoring-dashboard#metrics) in Cloud Monitoring.
- Continuous queries can use [slot autoscaling](https://docs.cloud.google.com/bigquery/docs/continuous-queries-introduction#slots_autoscaling) to dynamically scale allocated capacity to accommodate your workload.

This feature is [generally available](https://cloud.google.com/products#product-launch-stages) (GA).
Feature Spanner now supports [cross regional federated queries](https://cloud.google.com/bigquery/docs/spanner-federated-queries#cross_region_queries) from BigQuery which allow BigQuery users to query Spanner tables from regions other than their BigQuery region. Users don't incur [Spanner network egress charges](https://docs.cloud.google.com/spanner/pricing#network) during the preview period. This feature is in [preview](https://cloud.google.com/products#product-launch-stages).

## May 14, 2025

Feature You can now [schedule automated data transfers from Snowflake to BigQuery](https://docs.cloud.google.com/bigquery/docs/migration/snowflake-transfer) using the BigQuery Data Transfer Service. This feature is in [preview](https://cloud.google.com/products#product-launch-stages).
Feature Vector indexes support the [TreeAH index type](https://docs.cloud.google.com/bigquery/docs/vector-index#tree-ah-index), which uses Google's ScaNN algorithm. The TreeAH index is optimized for efficient batch processing, capable of handling anywhere from a few thousand to hundreds of thousands of embeddings at once. This feature is [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).
Feature BigQuery now supports cross-region transfers for [batch loading](https://docs.cloud.google.com/bigquery/docs/batch-loading-data) and [exporting](https://docs.cloud.google.com/bigquery/docs/exporting-data#export-data-in-bigquery) data. You can load or export your data from any region or multi-region to any other region or multi-region using a single [`bq load`](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#bq_load), [`LOAD DATA`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/load-statements#load_data_statement), [`bq extract`](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#bq_extract), or [`EXPORT DATA`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/export-statements#export_data_statement) statement. This feature is [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).

## May 13, 2025

Feature The following SQL features are now [generally available](https://cloud.google.com/products/#product-launch-stages) (GA) in BigQuery:

- [`GROUP BY STRUCT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#group_with_structs) and the `SELECT DISTINCT` clause.
- [`GROUP BY ARRAY`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#group_with_arrays) and the `SELECT DISTINCT` clause.
- [`GROUP BY ALL`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#group_by_all) clause.

## May 12, 2025

Libraries

### Java

#### [2.50.0](https://github.com/googleapis/java-bigquery/compare/v2.49.2...v2.50.0) (2025-05-06)

##### Features

- Add WRITE_TRUNCATE_DATA as an enum value for write disposition ([#3752](https://github.com/googleapis/java-bigquery/issues/3752)) ([acea61c](https://github.com/googleapis/java-bigquery/commit/acea61c20b69b44c8612ca22745458ad04bc6be4))
- **bigquery:** Add support for reservation field in jobs. ([#3768](https://github.com/googleapis/java-bigquery/issues/3768)) ([3e97f7c](https://github.com/googleapis/java-bigquery/commit/3e97f7c0c4676fcdda0862929a69bbabc69926f2))

##### Dependencies

- Update dependency com.google.api.grpc:proto-google-cloud-bigqueryconnection-v1 to v2.63.0 ([#3770](https://github.com/googleapis/java-bigquery/issues/3770)) ([934389e](https://github.com/googleapis/java-bigquery/commit/934389eb114d8fbb10c9c125d21ec26d503dca65))
- Update dependency com.google.apis:google-api-services-bigquery to v2-rev20250404-2.0.0 ([#3754](https://github.com/googleapis/java-bigquery/issues/3754)) ([1381c8f](https://github.com/googleapis/java-bigquery/commit/1381c8fe6c2552eec4519304c71697302733d6c7))
- Update dependency com.google.apis:google-api-services-bigquery to v2-rev20250427-2.0.0 ([#3773](https://github.com/googleapis/java-bigquery/issues/3773)) ([c0795fe](https://github.com/googleapis/java-bigquery/commit/c0795fe948e0ca231dbe8fc47c470603cb48ecc8))
- Update dependency com.google.cloud:sdk-platform-java-config to v3.46.3 ([#3772](https://github.com/googleapis/java-bigquery/issues/3772)) ([ab166b6](https://github.com/googleapis/java-bigquery/commit/ab166b6c33c574b4494368709db0443e055b4863))
- Update dependency com.google.cloud:sdk-platform-java-config to v3.47.0 ([#3779](https://github.com/googleapis/java-bigquery/issues/3779)) ([b27434b](https://github.com/googleapis/java-bigquery/commit/b27434b8a75e74184458e920142f5575fed9ba52))
Feature You can now view the [Query text section](https://docs.cloud.google.com/bigquery/docs/query-plan-explanation#understand_steps_with_query_text) in a BigQuery execution graph to understand how the stage steps are related to the query text. This feature is in [preview](https://cloud.google.com/products#product-launch-stages).
Feature [BigQuery resource utilization charts](https://docs.cloud.google.com/bigquery/docs/admin-resource-charts#view-resource-utilization) have the following changes:

- The default timeline shown in the event timeline chart has changed from one to six hours.
- Several improvements have been made to the views, including a new reservation slot usage view. This view helps monitor idle, baseline, and autoscaled slot usage.

This feature is in [Preview](https://cloud.google.com/products/#product-launch-stages).
Feature You can now use BigQuery and BigQuery DataFrames to enable multimodal analysis, transformation, and data engineering (ELT) workflows in both SQL and Python. Use multimodal data features to do the following:

- Integrate unstructured data into standard tables by using [`ObjectRef`](https://docs.cloud.google.com/bigquery/docs/analyze-multimodal-data#objectref_values) values, and then work with this data in analysis and transformation workflows by using [`ObjectRefRuntime`](https://docs.cloud.google.com/bigquery/docs/analyze-multimodal-data#objectrefruntime_values) values.

- Use generative AI to analyze multimodal data and generate embeddings by using
  [BigQuery ML SQL functions](https://docs.cloud.google.com/bigquery/docs/analyze-multimodal-data#generative_ai_functions) or [BigQuery DataFrames methods](https://docs.cloud.google.com/bigquery/docs/analyze-multimodal-data#generative_ai_methods)
  with Gemini and multimodal embedding models.

- [Create multimodal DataFrames](https://docs.cloud.google.com/bigquery/docs/analyze-multimodal-data#multimodal_dataframes) in BigQuery DataFrames, and then use [object transformation methods](https://docs.cloud.google.com/bigquery/docs/analyze-multimodal-data#object_transformation_methods) to transform images and chunk PDF files.

- [Use Python user-defined functions (UDFs)](https://docs.cloud.google.com/bigquery/docs/multimodal-data-sql-tutorial) to transform images and chunk PDF files.

This feature is in [Preview](https://cloud.google.com/products/#product-launch-stages).

## May 06, 2025

Change In the Google Cloud console, Analytics Hub has been renamed [BigQuery sharing (Analytics Hub)](https://docs.cloud.google.com/bigquery/docs/analytics-hub-introduction).

## May 05, 2025

Libraries

### Node.js

#### [8.0.0](https://github.com/googleapis/nodejs-bigquery/compare/v7.9.4...v8.0.0) (2025-04-23)

##### ⚠ BREAKING CHANGES

- migrate to node 18 ([#1458](https://github.com/googleapis/nodejs-bigquery/issues/1458))

##### Miscellaneous Chores

- Migrate to node 18 ([#1458](https://github.com/googleapis/nodejs-bigquery/issues/1458)) ([6cd706b](https://github.com/googleapis/nodejs-bigquery/commit/6cd706b6e96ac54a9289211e7e3d2cc1f4e934e2))
Feature Changes that you make to your saved queries are now [automatically saved](https://docs.cloud.google.com/bigquery/docs/work-with-saved-queries#update_saved_queries). This feature is in [preview](https://cloud.google.com/products#product-launch-stages).

## April 28, 2025

Libraries

### Java

#### [2.49.2](https://github.com/googleapis/java-bigquery/compare/v2.49.1...v2.49.2) (2025-04-26)

##### Dependencies

- Update dependency com.google.cloud:sdk-platform-java-config to v3.46.2 ([#3756](https://github.com/googleapis/java-bigquery/issues/3756)) ([907e39f](https://github.com/googleapis/java-bigquery/commit/907e39fd467f972863deeb86356fc3bfb989a76d))

### Java

#### [2.49.1](https://github.com/googleapis/java-bigquery/compare/v2.49.0...v2.49.1) (2025-04-24)

##### Bug Fixes

- Add labels to converter for listTables method ([#3735](https://github.com/googleapis/java-bigquery/issues/3735)) ([#3736](https://github.com/googleapis/java-bigquery/issues/3736)) ([8634822](https://github.com/googleapis/java-bigquery/commit/8634822e1836c5ccc0f8d0263ac57ac561578360))

##### Dependencies

- Update dependency com.google.cloud:sdk-platform-java-config to v3.46.0 ([#3753](https://github.com/googleapis/java-bigquery/issues/3753)) ([a335927](https://github.com/googleapis/java-bigquery/commit/a335927e16d0907d62e584f08fa8393daae40354))
- Update netty.version to v4.2.0.final ([#3745](https://github.com/googleapis/java-bigquery/issues/3745)) ([bb811c0](https://github.com/googleapis/java-bigquery/commit/bb811c068b3efabf04fbe67dbb2979d562c604d9))
Libraries

### Java

#### [2.49.1](https://github.com/googleapis/java-bigquery/compare/v2.49.0...v2.49.1) (2025-04-24)

##### Bug Fixes

- Add labels to converter for listTables method ([#3735](https://github.com/googleapis/java-bigquery/issues/3735)) ([#3736](https://github.com/googleapis/java-bigquery/issues/3736)) ([8634822](https://github.com/googleapis/java-bigquery/commit/8634822e1836c5ccc0f8d0263ac57ac561578360))

##### Dependencies

- Update dependency com.google.cloud:sdk-platform-java-config to v3.46.0 ([#3753](https://github.com/googleapis/java-bigquery/issues/3753)) ([a335927](https://github.com/googleapis/java-bigquery/commit/a335927e16d0907d62e584f08fa8393daae40354))
- Update netty.version to v4.2.0.final ([#3745](https://github.com/googleapis/java-bigquery/issues/3745)) ([bb811c0](https://github.com/googleapis/java-bigquery/commit/bb811c068b3efabf04fbe67dbb2979d562c604d9))
Feature [Dataplex automatic discovery](https://docs.cloud.google.com/bigquery/docs/automatic-discovery) in BigQuery scans your data in Cloud Storage buckets to extract and catalog metadata, creating BigLake, external, or object tables for analytics and AI for insights, security, and governance. This feature is [generally available](https://cloud.google.com/products#product-launch-stages) (GA).
Feature When you translate SQL queries from your source database, you can use configuration YAML files to [optimize and improve the performance of your translated SQL](https://docs.cloud.google.com/bigquery/docs/config-yaml-translation#optimize_and_improve_the_performance_of_translated_sql). This feature is [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).

## April 24, 2025

Feature You can now work with a [Gemini powered assistant](https://docs.cloud.google.com/bigquery/docs/data-canvas#assistant) in a BigQuery data canvas. The data canvas assistant is an agent-like tool, capable of constructing and modifying a data canvas to answer data analytics questions from user prompting. This feature is now in [Preview](https://cloud.google.com/products#product-launch-stages).

## April 23, 2025

Feature You can now [specify which reservation a query uses at runtime](https://docs.cloud.google.com/bigquery/docs/reservations-workload-management#flexible), and [set IAM policies directly on reservations](https://docs.cloud.google.com/bigquery/docs/reservations-workload-management#securable). This provides more flexibility and fine-grained control over resource management. This feature is in [public preview](https://cloud.google.com/products#product-launch-stages).
Feature You can now set a [maximum slot limit](https://docs.cloud.google.com/bigquery/docs/reservations-workload-management#predictable) for a reservation. You can configure the maximum reservation size when creating or updating a reservation. This feature is in [public preview](https://cloud.google.com/products#product-launch-stages).
Feature You can now [allocate idle slots fairly](https://docs.cloud.google.com/bigquery/docs/slots#fairness) across reservations within a single admin project. This ensures each reservation receives an approximately equal share of available capacity. This feature is in [public preview](https://cloud.google.com/products#product-launch-stages).

## April 21, 2025

Libraries

### Node.js

#### [7.9.4](https://github.com/googleapis/nodejs-bigquery/compare/v7.9.3...v7.9.4) (2025-04-02)

##### Bug Fixes

- MergeSchemaWithRows can be called with empty schema if result set is empty ([#1455](https://github.com/googleapis/nodejs-bigquery/issues/1455)) ([e608601](https://github.com/googleapis/nodejs-bigquery/commit/e608601157a95430a63ce0047194ba40190b2e42))
Announcement BigQuery now provides spend-based [committed use discounts](https://docs.cloud.google.com/bigquery/docs/bigquery-cud) (CUDs). Spend-based committed use discounts provide a discount in exchange for your commitment to spend a minimum amount per hour on PAYG compute resources [listed here](https://docs.cloud.google.com/bigquery/docs/bigquery-cud#usage). You can purchase CUDs with a one or three year commitment period.
Feature You can now enable [fine-grained access control on BigQuery metastore Iceberg tables](https://docs.cloud.google.com/bigquery/docs/bqms-features#set_access_control_policies). This feature is [generally available](https://cloud.google.com/products#product-launch-stages) (GA).
Change You can get the required permissions to use BigQuery data preparation through the [BigQuery Studio User](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquery#bigquery.studioUser) (`roles/bigquery.studioUser`) and [Gemini for Google Cloud User](https://docs.cloud.google.com/iam/docs/roles-permissions/cloudaicompanion#cloudaicompanion.user) (`roles/cloudaicompanion.user`) roles, and permission to access the data you're preparing.

BigQuery data preparation no longer requires that you have the permissions granted by the following IAM roles:

- [BigQuery Data Editor](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquery#bigquery.dataEditor)(`roles/bigquery.dataEditor`)
- [Service Usage Consumer](https://docs.cloud.google.com/iam/docs/roles-permissions/serviceusage#serviceusage.serviceUsageConsumer) (`roles/serviceusage.serviceUsageConsumer`)

For more information about the required roles, see [Manage data preparations](https://docs.cloud.google.com/bigquery/docs/manage-data-preparations).

## April 17, 2025

Feature You can use [partial ordering mode](https://docs.cloud.google.com/bigquery/docs/use-bigquery-dataframes#partial-ordering-mode) in BigQuery DataFrames to generate efficient queries. This feature is [generally available](https://cloud.google.com/products#product-launch-stages) (GA).
Feature You can now use [BigQuery DataFrames version 2.0](https://docs.cloud.google.com/bigquery/docs/use-bigquery-dataframes#version-2), which makes security and performance improvements to the BigQuery DataFrames API, adds new features, and introduces breaking changes.

## April 09, 2025

Announcement *Dataplex Catalog* has been renamed *BigQuery universal catalog* . You'll see this new name in the product page of the Google Cloud console, the documentation set, and the marketing collateral. Universal catalog brings together the [data catalog capabilities of Dataplex Catalog](https://docs.cloud.google.com/dataplex/docs/catalog-overview) and the [runtime metastore capabilities of BigQuery metastore](https://docs.cloud.google.com/bigquery/docs/about-bqms). For more information, see [Introduction to data governance in BigQuery](https://docs.cloud.google.com/bigquery/docs/data-governance).
Change Updated pricing, packaging, and setup guidance is now available for [Gemini in BigQuery](https://docs.cloud.google.com/gemini/docs/bigquery/set-up-gemini).
Feature You can now use the Apache Arrow format to [stream data to BigQuery with the Storage Write API](https://docs.cloud.google.com/bigquery/docs/supported-data-types#supported-apache-arrow-data-types). This feature is available in [preview](https://cloud.google.com/products/#product-launch-stages).
Change *Analytics Hub* has been renamed [*BigQuery sharing*](https://docs.cloud.google.com/bigquery/docs/analytics-hub-introduction). You'll see this new name in the documentation set and the marketing collateral. The product functionality and endpoints remain the same. For more information, see [Introduction to data governance in BigQuery](https://docs.cloud.google.com/bigquery/docs/data-governance).
Feature You can now combine raster and vector data with the [`ST_REGIONSTATS` geography function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_regionstats) to perform geospatial analysis in BigQuery. For more information, see [Work with raster data](https://docs.cloud.google.com/bigquery/docs/raster-data) and try the tutorial that shows you how to [use raster data to analyze global temperature by country](https://docs.cloud.google.com/bigquery/docs/raster-tutorial-weather). This feature is in [preview](https://cloud.google.com/products#product-launch-stages).

## April 08, 2025

Feature You can now [create, view, modify, and delete Apache Iceberg resources in BigQuery metastore](https://docs.cloud.google.com/bigquery/docs/bqms-manage-resources). This feature is [generally available](https://cloud.google.com/products#product-launch-stages) (GA).
Feature You can now [connect BigQuery metastore to Apache Flink](https://docs.cloud.google.com/bigquery/docs/bqms-use-dataproc#connect-bigquery-flink). This feature is [generally available](https://cloud.google.com/products#product-launch-stages) (GA).
Feature BigQuery ML now offers a built-in [TimesFM univariate time series forecasting model](https://docs.cloud.google.com/bigquery/docs/timesfm-model) that implements Google Research's open source TimesFM model. You can use BigQuery ML's built-in TimesFM model with the [`AI.FORECAST` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-forecast) to perform forecasting without having to create and train your own model. This lets you avoid the need for model management.

To try using a TimesFM model with the `AI.FORECAST` function, see [Forecast a time series with a TimesFM univariate model](https://docs.cloud.google.com/bigquery/docs/timesfm-time-series-forecasting-tutorial).

This feature is in [preview](https://cloud.google.com/products/#product-launch-stages).

## April 07, 2025

Feature You can now create [remote models](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model) in BigQuery ML based on [Llama](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/partner-models/llama) and [Mistral AI](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/partner-models/mistral) models in Vertex AI.

Use the [`ML.GENERATE_TEXT` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-generate-text) with these remote models to perform generative natural language tasks for text stored in BigQuery tables. Try this feature with the [Generate text by using the `ML.GENERATE_TEXT` function](https://docs.cloud.google.com/bigquery/docs/generate-text) tutorial.

This feature is [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).
Change An updated version of [JDBC driver for BigQuery](https://docs.cloud.google.com/bigquery/docs/reference/odbc-jdbc-drivers#current_jdbc_driver) is now available.
Feature [BigQuery data preparation](https://docs.cloud.google.com/bigquery/docs/data-prep-introduction) is [generally available](https://cloud.google.com/products#product-launch-stages) (GA). It offers AI-powered suggestions from Gemini for data cleansing, transformation, and enrichment. BigQuery supports visual data preparation pipelines and pipeline scheduling with Dataform.
Feature [Smart-tuning](https://docs.cloud.google.com/bigquery/docs/materialized-views-use#smart_tuning) is now supported for [materialized views](https://docs.cloud.google.com/bigquery/docs/materialized-views-intro) when they are in the same project as one of their base tables, or when they are in the project running the query. This feature is [generally available](https://cloud.google.com/products#product-launch-stages) (GA).
Change BigQuery ML now uses dynamic token-based batching for [embedding generation](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-generate-embedding) requests. Dynamic token-based batching puts as many rows as possible into one request. This change boosts per-request utilization and improves scalability for any [queries per minute (QPM) quota](https://docs.cloud.google.com/bigquery/quotas#cloud_ai_service_functions). Actual performance varies based on the embedding content length, with an average 10x improvement.

## April 04, 2025

Feature BigQuery ML now supports the following [generative AI functions](https://docs.cloud.google.com/bigquery/docs/generative-ai-overview#generative_ai_functions), which let you analyze text using a Vertex AI Gemini model. The function output includes a response that matches the type in the function name:

- [`AI.GENERATE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-generate)
- [`AI.GENERATE_BOOL`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-generate-bool)
- [`AI.GENERATE_INT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-generate-int)
- [`AI.GENERATE_DOUBLE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-generate-double)

This feature is in [preview](https://cloud.google.com/products#product-launch-stages).

## April 03, 2025

Feature [BigQuery migration assessment](https://cloud.google.com/bigquery/docs/migration-assessment) now includes support for Amazon Redshift Serverless. This feature is in [preview](https://cloud.google.com/products#product-launch-stages).
Feature You can now generate structured data by using BigQuery ML's [`AI.GENERATE_TABLE` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-generate-table) with Gemini 1.5 Pro, Gemini 1.5 Flash, and Gemini 2.0 Flash models. You can use the `AI.GENERATE_TABLE` function's `output_schema` argument to more easily format the model's response. The `output_schema` argument lets you specify a SQL schema for formatting, similar to the schema used in the `CREATE TABLE` statement. By creating structured output, you can more easily convert the function output into a BigQuery table.

Try this feature with the [Generate structured data by using the `AI.GENERATE_TABLE` function](https://docs.cloud.google.com/bigquery/docs/generate-table) tutorial.

This feature is in [preview](https://cloud.google.com/products/#product-launch-stages).

## April 02, 2025

Feature You can now create and use [Python user-defined functions](https://cloud.google.com/bigquery/docs/user-defined-functions-python) (UDFs) in BigQuery. Python UDFs support the use of additional libraries and external APIs. This feature is in [preview](https://cloud.google.com/products#product-launch-stages).
Change The Python code that you generate using [Gemini in BigQuery Notebooks](https://docs.cloud.google.com/bigquery/docs/write-sql-gemini) is now much more likely to leverage your data. With this change, BigQuery Notebooks can intelligently pull relevant table names directly from your BigQuery project, resulting in personalized, executable Python code.
Feature You can now [generate Dataframes code in BigQuery Notebooks](https://docs.cloud.google.com/bigquery/docs/write-sql-gemini#dataframe) that use BigFrames libraries. In your code generation prompt, include the word `BigFrames` to generate code that uses [BigQuery DataFrames](https://docs.cloud.google.com/bigquery/docs/use-bigquery-dataframes). This feature is in [preview](https://cloud.google.com/products#product-launch-stages).

## April 01, 2025

Feature You can use a [`CREATE MODEL` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-contribution-analysis) to create a [contribution analysis](https://docs.cloud.google.com/bigquery/docs/contribution-analysis) model in BigQuery ML. The [`top_k_insights_by_apriori_support`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-contribution-analysis#top_k_insights_by_apriori_support) and [`pruning_method model` options](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-contribution-analysis#pruning_method) are now supported. You can use a contribution analysis model with the [`ML.GET_INSIGHTS` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-get-insights) to generate insights about changes to key metrics in your multi-dimensional data. The following metric types are supported:

- [Summable metric](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-contribution-analysis#use_a_summable_metric)
- [Summable ratio metric](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-contribution-analysis#use_a_summable_ratio_metric)
- [Summable by category metric](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-contribution-analysis#use_a_summable_by_category_metric)

This feature is [generally available](https://products#product-launch-stages) (GA).
Feature [Pipe syntax](https://docs.cloud.google.com/bigquery/docs/pipe-syntax-guide) supports a linear query structure designed to make your queries easier to read, write, and maintain. This feature is [generally available](https://cloud.google.com/products#product-launch-stages) (GA).

## March 31, 2025

Feature [Iceberg external tables](https://docs.cloud.google.com/bigquery/docs/iceberg-external-tables) now support merge-on-read. You can query Iceberg tables with position deletes and equality deletes. This feature is [generally available](https://cloud.google.com/products#product-launch-stages) (GA).
Libraries

### Python

#### [3.31.0](https://github.com/googleapis/python-bigquery/compare/v3.30.0...v3.31.0) (2025-03-20)

##### Features

- Add query text and total bytes processed to RowIterator ([#2140](https://github.com/googleapis/python-bigquery/issues/2140)) ([2d5f932](https://github.com/googleapis/python-bigquery/commit/2d5f9320d7103bc64c7ba496ba54bb0ef52b5605))
- Add support for Python 3.13 ([0842aa1](https://github.com/googleapis/python-bigquery/commit/0842aa10967b1d8395cfb43e52c8ea091b381870))

##### Bug Fixes

- Adding property setter for table constraints, [#1990](https://github.com/googleapis/python-bigquery/issues/1990) ([#2092](https://github.com/googleapis/python-bigquery/issues/2092)) ([f8572dd](https://github.com/googleapis/python-bigquery/commit/f8572dd86595361bae82c3232b2c0d159690a7b7))
- Allow protobuf 6.x ([0842aa1](https://github.com/googleapis/python-bigquery/commit/0842aa10967b1d8395cfb43e52c8ea091b381870))
- Avoid "Unable to determine type" warning with JSON columns in `to_dataframe` ([#1876](https://github.com/googleapis/python-bigquery/issues/1876)) ([968020d](https://github.com/googleapis/python-bigquery/commit/968020d5be9d2a30b90d046eaf52f91bb2c70911))
- Remove setup.cfg configuration for creating universal wheels ([#2146](https://github.com/googleapis/python-bigquery/issues/2146)) ([d7f7685](https://github.com/googleapis/python-bigquery/commit/d7f76853d598c354bfd2e65f5dde28dae97da0ec))

##### Dependencies

- Remove Python 3.7 and 3.8 as supported runtimes ([#2133](https://github.com/googleapis/python-bigquery/issues/2133)) ([fb7de39](https://github.com/googleapis/python-bigquery/commit/fb7de398cb2ad000b80a8a702d1f6539dc03d8e0))
Feature BigQuery now supports [subqueries](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/subqueries) in [row level access policies](https://docs.cloud.google.com/bigquery/docs/managing-row-level-security#create_or_update_a_row-level_access_policy). It also includes support for [BigLake managed tables](https://docs.cloud.google.com/bigquery/docs/biglake-intro) and the [BigQuery Storage Read API.](https://docs.cloud.google.com/bigquery/docs/reference/storage) This feature is now [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).
Feature You can now configure the repeat frequency of [BigQuery Data Transfer Service for Google Ad Manager](https://docs.cloud.google.com/bigquery/docs/doubleclick-publisher-transfer). This option has a default of every 8 hours and a minimum of every 4 hours. This feature is [generally available](https://cloud.google.com/products#product-launch-stages) (GA).
Feature You can [build BigQuery pipelines](https://docs.cloud.google.com/bigquery/docs/workflows-introduction) (formerly workflows), composed of SQL queries or notebooks, in BigQuery Studio. You can then run these pipelines on a schedule. You can also configure notebook runtimes for a pipeline, share a pipeline, or share a pipeline link. This feature is [generally available](https://cloud.google.com/products#product-launch-stages) (GA).
Feature You can now skip loading match tables for [BigQuery Data Transfer Service for Google Ad Manager](https://docs.cloud.google.com/bigquery/docs/doubleclick-publisher-transfer). If match tables are not needed, you can set parameter `load_match_tables` to `FALSE`. This feature is [generally available](https://cloud.google.com/products#product-launch-stages) (GA).
Feature You can now use [BigQuery Data Transfer Service for Search Ads](https://docs.cloud.google.com/bigquery/docs/search-ads-transfer#pmax-support) to view [Performance Max (PMax) campaign data](https://support.google.com/google-ads/answer/10724817) for the following tables:

- CartDataSalesStats
- ProductAdvertised
- ProductAdvertisedDeviceStats
- ProductAdvertisedConversionActionAndDeviceStats

This feature is [generally available](https://cloud.google.com/products#product-launch-stages) (GA).
Feature On the Scheduling page, you can now view existing schedules, create new schedules, and perform other actions for data preparations, notebooks, BigQuery pipelines, and scheduled queries. For more information, see [Create a pipeline schedule](https://docs.cloud.google.com/bigquery/docs/orchestrate-workflows#create-schedule). This feature is [generally available](https://cloud.google.com/products#product-launch-stages) (GA).
Feature You can now define a [`_CHANGE_SEQUENCE_NUMBER`](https://docs.cloud.google.com/bigquery/docs/change-data-capture#manage_custom_ordering) for BigQuery change data capture (CDC) to manage streaming `UPSERT` ordering for BigQuery. This feature is [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).
Feature You can include data preparation tasks in [BigQuery pipelines](https://docs.cloud.google.com/bigquery/docs/workflows-introduction) that execute your code assets in sequence at a scheduled time. This feature is in [Preview](https://cloud.google.com/products#product-launch-stages).

## March 27, 2025

Feature You can now [enable metadata caching for SQL translation](https://docs.cloud.google.com/bigquery/docs/interactive-sql-translator#translate-with-additional-configs), which can significantly reduce latency for subsequent translation requests. This feature is in [preview](https://cloud.google.com/products#product-launch-stages).

## March 26, 2025

Feature You can now set the [column granularity](https://docs.cloud.google.com/bigquery/docs/search-index#column-granularity) when you [create a search index](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_search_index_statement), which stores additional column information in your search index to further optimize your search query performance. This feature is in [preview](https://cloud.google.com/products/#product-launch-stages).

## March 25, 2025

Feature BigQuery ML now supports [visualization of model monitoring metrics](https://docs.cloud.google.com/bigquery/docs/model-monitoring-overview#monitoring_visualization). This feature lets you use charts and graphs to [analyze model monitoring function output](https://docs.cloud.google.com/vertex-ai/docs/model-monitoring/run-monitoring-job#analyze_monitoring_job_results). The following functions support metric visualization:

- [`ML.VALIDATE_DATA_SKEW`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-validate-data-skew): compute the statistics for a set of serving data, and then compare them to the statistics for the data used to train a BigQuery ML model in order to identify anomalous differences between the two data sets.
- [`ML.VALIDATE_DATA_DRIFT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-validate-data-drift): compute and compare the statistics for two sets of serving data in order to identify anomalous differences between the two data sets.

This feature is in [preview](https://cloud.google.com/products/#product-launch-stages).

## March 24, 2025

Libraries

### Java

#### [2.49.0](https://github.com/googleapis/java-bigquery/compare/v2.48.1...v2.49.0) (2025-03-20)

##### Features

- **bigquery:** Implement getArray in BigQueryResultImpl ([#3693](https://github.com/googleapis/java-bigquery/issues/3693)) ([e2a3f2c](https://github.com/googleapis/java-bigquery/commit/e2a3f2c1a1406bf7bc9a035dce3acfde78f0eaa4))
- Next release from main branch is 2.49.0 ([#3706](https://github.com/googleapis/java-bigquery/issues/3706)) ([b46a6cc](https://github.com/googleapis/java-bigquery/commit/b46a6ccc959f8defb145279ea18ff2e4f1bac58f))

##### Bug Fixes

- Retry ExceptionHandler not retrying on IOException ([#3668](https://github.com/googleapis/java-bigquery/issues/3668)) ([83245b9](https://github.com/googleapis/java-bigquery/commit/83245b961950ca9a993694082e533834ee364417))

##### Dependencies

- Exclude io.netty:netty-common from org.apache.arrow:arrow-memor... ([#3715](https://github.com/googleapis/java-bigquery/issues/3715)) ([11b5809](https://github.com/googleapis/java-bigquery/commit/11b580949b910b38732c1c8d64704c54c260214e))
- Update actions/upload-artifact action to v4.6.2 ([#3724](https://github.com/googleapis/java-bigquery/issues/3724)) ([426a59b](https://github.com/googleapis/java-bigquery/commit/426a59b9b999e836804f84c5cbe11d497128f0a8))
- Update actions/upload-artifact action to v4.6.2 ([#3724](https://github.com/googleapis/java-bigquery/issues/3724)) ([483f930](https://github.com/googleapis/java-bigquery/commit/483f9305023988b3884329733d0e5fbcb6599eb1))
- Update dependency com.google.api.grpc:proto-google-cloud-bigqueryconnection-v1 to v2.61.0 ([#3703](https://github.com/googleapis/java-bigquery/issues/3703)) ([53b07b0](https://github.com/googleapis/java-bigquery/commit/53b07b0e77f6ef57c8518df2b106edace679f79a))
- Update dependency com.google.api.grpc:proto-google-cloud-bigqueryconnection-v1 to v2.62.0 ([#3726](https://github.com/googleapis/java-bigquery/issues/3726)) ([38e004b](https://github.com/googleapis/java-bigquery/commit/38e004b58134caf4f7b0d96257456930beb0e599))
- Update dependency com.google.apis:google-api-services-bigquery to v2-rev20250302-2.0.0 ([#3720](https://github.com/googleapis/java-bigquery/issues/3720)) ([c0b3902](https://github.com/googleapis/java-bigquery/commit/c0b39029302c51e65ea31495d837598eefbe94e8))
- Update dependency com.google.apis:google-api-services-bigquery to v2-rev20250313-2.0.0 ([#3723](https://github.com/googleapis/java-bigquery/issues/3723)) ([b8875a8](https://github.com/googleapis/java-bigquery/commit/b8875a895d6d5e267086e24f97d0ed5fec36b9fe))
- Update dependency com.google.cloud:google-cloud-datacatalog-bom to v1.65.0 ([#3704](https://github.com/googleapis/java-bigquery/issues/3704)) ([53b68b1](https://github.com/googleapis/java-bigquery/commit/53b68b13a505aa5d38e56032eaeb8c95bf3e9078))
- Update dependency com.google.cloud:google-cloud-datacatalog-bom to v1.66.0 ([#3727](https://github.com/googleapis/java-bigquery/issues/3727)) ([7339f94](https://github.com/googleapis/java-bigquery/commit/7339f94cfa53d1c988f8ef051ddd5a2d7668d430))
- Update dependency com.google.cloud:sdk-platform-java-config to v3.45.1 ([#3714](https://github.com/googleapis/java-bigquery/issues/3714)) ([e4512aa](https://github.com/googleapis/java-bigquery/commit/e4512aa5966e7b935fa55a062d940d9db0c834b3))
- Update dependency com.google.oauth-client:google-oauth-client-java6 to v1.39.0 ([#3710](https://github.com/googleapis/java-bigquery/issues/3710)) ([c0c6352](https://github.com/googleapis/java-bigquery/commit/c0c6352b8d02145fe9513e3e23d316e045360d2d))
- Update dependency com.google.oauth-client:google-oauth-client-jetty to v1.39.0 ([#3711](https://github.com/googleapis/java-bigquery/issues/3711)) ([43b86e9](https://github.com/googleapis/java-bigquery/commit/43b86e91a664dd9d3edaea7b31b46ac635fb22b0))
- Update dependency node to v22 ([#3713](https://github.com/googleapis/java-bigquery/issues/3713)) ([251def5](https://github.com/googleapis/java-bigquery/commit/251def5659d2648dff0833ba967a65435e11b643))
- Update netty.version to v4.1.119.final ([#3717](https://github.com/googleapis/java-bigquery/issues/3717)) ([08a290a](https://github.com/googleapis/java-bigquery/commit/08a290adcfa7551ee27a58da0eaf5ac00a759b90))

##### Documentation

- Update error handling comment to be more precise in samples ([#3712](https://github.com/googleapis/java-bigquery/issues/3712)) ([9eb555f](https://github.com/googleapis/java-bigquery/commit/9eb555ff61bef42a3bdfe197da8423b7bf14f493))
Libraries

### Go

#### [1.67.0](https://github.com/googleapis/google-cloud-go/compare/bigquery/v1.66.2...bigquery/v1.67.0) (2025-03-14)

##### Features

- **bigquery/reservation:** Add a new field `enable_gemini_in_bigquery` to `.google.cloud.bigquery.reservation.v1.Assignment` that indicates if "Gemini in Bigquery"(https ([601e742](https://github.com/googleapis/google-cloud-go/commit/601e74202ca6bf28506f06f27abc1d99018f9dc5))
- **bigquery/reservation:** Add a new field `replication_status` to `.google.cloud.bigquery.reservation.v1.Reservation` to provide visibility into errors that could arise during Disaster Recovery(DR) replication ([#11666](https://github.com/googleapis/google-cloud-go/issues/11666)) ([601e742](https://github.com/googleapis/google-cloud-go/commit/601e74202ca6bf28506f06f27abc1d99018f9dc5))
- **bigquery/reservation:** Add the CONTINUOUS Job type to `.google.cloud.bigquery.reservation.v1.Assignment.JobType` for continuous SQL jobs ([601e742](https://github.com/googleapis/google-cloud-go/commit/601e74202ca6bf28506f06f27abc1d99018f9dc5))
- **bigquery:** Support MetadataCacheMode for ExternalDataConfig ([#11803](https://github.com/googleapis/google-cloud-go/issues/11803)) ([af5174d](https://github.com/googleapis/google-cloud-go/commit/af5174daa535bb1ceee4bf5eee894eedeca66498)), refs [#11802](https://github.com/googleapis/google-cloud-go/issues/11802)

##### Bug Fixes

- **bigquery:** Increase timeout for storage api test and remove usage of deprecated pkg ([#11810](https://github.com/googleapis/google-cloud-go/issues/11810)) ([f47e038](https://github.com/googleapis/google-cloud-go/commit/f47e038e360375558da50c185f16002f1b1f73f4)), refs [#11801](https://github.com/googleapis/google-cloud-go/issues/11801)
- **bigquery:** Update golang.org/x/net to 0.37.0 ([1144978](https://github.com/googleapis/google-cloud-go/commit/11449782c7fb4896bf8b8b9cde8e7441c84fb2fd))

##### Documentation

- **bigquery/reservation:** Remove the section about `EDITION_UNSPECIFIED` in the comment for `slot_capacity` in `.google.cloud.bigquery.reservation.v1.Reservation` to clarify that ([601e742](https://github.com/googleapis/google-cloud-go/commit/601e74202ca6bf28506f06f27abc1d99018f9dc5))
- **bigquery/reservation:** Update the `google.api.field_behavior` for the `.google.cloud.bigquery.reservation.v1.Reservation.primary_location` and `.google.cloud.bigquery.reservation.v1.Reservation.original_primary_location` fields to clarify that they are `OUTPUT_ONLY` ([601e742](https://github.com/googleapis/google-cloud-go/commit/601e74202ca6bf28506f06f27abc1d99018f9dc5))
Libraries

### Node.js

#### [7.9.3](https://github.com/googleapis/nodejs-bigquery/compare/v7.9.2...v7.9.3) (2025-03-17)

##### Bug Fixes

- Make sure to pass selectedFields to tabledata.list method ([#1449](https://github.com/googleapis/nodejs-bigquery/issues/1449)) ([206aff9](https://github.com/googleapis/nodejs-bigquery/commit/206aff93d3d3520199388fc31314fa7ec221cee8))
Feature We have redesigned the [Add Data](https://docs.cloud.google.com/bigquery/docs/bigquery-web-ui#Add_data) dialog to guide you through loading data into BigQuery with a source-first experience and enhanced search and filtering capabilities. This feature is [generally available](https://cloud.google.com/products#product-launch-stages) (GA).
Feature You can now set [labels](https://docs.cloud.google.com/bigquery/docs/labels-intro) on reservations. These labels can be used to organize your reservations and for billing analysis. This feature is in [preview](https://cloud.google.com/products#product-launch-stages).
Feature The BigQuery Data Transfer Service can now [transfer reporting and configuration data from Google Analytics 4](https://docs.cloud.google.com/bigquery/docs/google-analytics-4-transfer) into BigQuery. This feature is in [preview](https://cloud.google.com/products/#product-launch-stages).
Feature You can now use [KLL quantile functions](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/kll_functions) to efficiently compute approximate quantiles. This feature is in [preview](https://cloud.google.com/products#product-launch-stages).

## March 20, 2025

Feature You can now create [remote models](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model) in BigQuery ML based on the [Anthropic Claude model](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/partner-models/use-claude) in Vertex AI.

Use the [`ML.GENERATE_TEXT` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-generate-text) with these remote models to perform generative natural language tasks for text stored in BigQuery tables. Try this feature with the [Generate text by using the `ML.GENERATE_TEXT` function](https://docs.cloud.google.com/bigquery/docs/generate-text) tutorial.

You can also evaluate Claude models by using the [`ML.EVALUATE` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-evaluate).

This feature is [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).
Feature You can now use [repositories](https://docs.cloud.google.com/bigquery/docs/repository-intro) and [workspaces](https://docs.cloud.google.com/bigquery/docs/workspaces-intro) in BigQuery to perform version control.

Repositories perform version control on files by using Git to record changes and manage file versions. You can use workspaces within repositories to edit the code stored in the repository.

You can have a repository use Git directly on BigQuery, or you can [connect a repository to a third-party Git provider](https://docs.cloud.google.com/bigquery/docs/repositories#connect-third-party).

This feature is in [preview](https://cloud.google.com/products/#product-launch-stages).
Announcement BigQuery workflows have been renamed to BigQuery pipelines in the Google Cloud console. For more information, see [Introduction to BigQuery pipelines](https://docs.cloud.google.com/bigquery/docs/workflows-introduction).

## March 17, 2025

Feature You can now use [`EXPORT DATA` statements](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/other-statements) to [reverse ETL BigQuery data to Spanner](https://docs.cloud.google.com/bigquery/docs/export-to-spanner). This feature is [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).
Feature You can now use the [`TYPEOF` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/utility-functions#typeof) to determine the data type of an expression. This feature is [generally available](https://cloud.google.com/products#product-launch-stages) (GA).
Feature You can now create an [external dataset](https://docs.cloud.google.com/bigquery/docs/spanner-external-datasets) in BigQuery that links to an existing database in [Spanner](https://docs.cloud.google.com/spanner/docs). This feature is [generally available](https://cloud.google.com/products/#product-launch-stages) (GA).

## March 13, 2025

Feature [Dataform](https://docs.cloud.google.com/dataform/docs/cmek#org-policy) now supports the [CMEK organization policy](https://kms/docs/cmek-org-policy).
Feature You can now use Gemini Cloud Assist chat to generate [SQL queries](https://docs.cloud.google.com/bigquery/docs/write-sql-gemini#chat) and [Python code](https://docs.cloud.google.com/bigquery/docs/write-sql-gemini#chat-python). This feature is in [preview](https://cloud.google.com/products#product-launch-stages).

## March 12, 2025

Feature You can configure reusable, default Cloud resource connections in a project. [Default connections](https://docs.cloud.google.com/bigquery/docs/default-connections) are available in [Preview](https://cloud.google.com/products#product-launch-stages).
Change An updated version of [ODBC driver for BigQuery](https://docs.cloud.google.com/bigquery/docs/reference/odbc-jdbc-drivers#odbc_release_3121004) is now available.

## March 10, 2025

Announcement Analytics Hub [egress controls](https://docs.cloud.google.com/bigquery/docs/analytics-hub-introduction#data_egress) and [data clean room](https://docs.cloud.google.com/bigquery/docs/data-clean-rooms#subscriber_workflows) subscriptions are now available in all BigQuery [editions](https://docs.cloud.google.com/bigquery/docs/editions-intro#analysis_features) and on-demand pricing.

## March 06, 2025

Feature BigQuery Data Transfer Service now supports [custom reports for Google Ads](https://docs.cloud.google.com/bigquery/docs/google-ads-transfer#custom_reports). You can use Google Ads Query Language (GAQL) queries in your transfer configuration to ingest custom Google Ads reports and fields beyond those available in the [standard reports and fields](https://docs.cloud.google.com/bigquery/docs/google-ads-transformation). This feature is now [generally available](https://cloud.google.com/products#product-launch-stages) (GA).

## March 04, 2025

Change BigQuery is now available in the [Stockholm (europe-north2) region](https://docs.cloud.google.com/bigquery/docs/locations#regions).

## March 03, 2025

Libraries

### Python

#### [3.30.0](https://github.com/googleapis/python-bigquery/compare/v3.29.0...v3.30.0) (2025-02-26)

##### Features

- Add roundingmode enum, wiring, and tests ([#2121](https://github.com/googleapis/python-bigquery/issues/2121)) ([3a48948](https://github.com/googleapis/python-bigquery/commit/3a4894827f6e73a4a88cb22933c2004697dabcc7))
- Adds foreign_type_info attribute to table class and adds unit tests. ([#2126](https://github.com/googleapis/python-bigquery/issues/2126)) ([2c19681](https://github.com/googleapis/python-bigquery/commit/2c1968115bef8e1dc84e0125615f551b9b011a4b))
- Support resource_tags for table ([#2093](https://github.com/googleapis/python-bigquery/issues/2093)) ([d4070ca](https://github.com/googleapis/python-bigquery/commit/d4070ca21b5797e900a9e87b966837ee1c278217))

##### Bug Fixes

- Avoid blocking in download thread when using BQ Storage API ([#2034](https://github.com/googleapis/python-bigquery/issues/2034)) ([54c8d07](https://github.com/googleapis/python-bigquery/commit/54c8d07f06a8ae460c9e0fb1614e1fbc21efb5df))
- Retry 404 errors in `Client.query(...)` ([#2135](https://github.com/googleapis/python-bigquery/issues/2135)) ([c6d5f8a](https://github.com/googleapis/python-bigquery/commit/c6d5f8aaec21ab8f17436407aded4bc2316323fd))

##### Dependencies

- Updates required checks list in github ([#2136](https://github.com/googleapis/python-bigquery/issues/2136)) ([fea49ff](https://github.com/googleapis/python-bigquery/commit/fea49ffbf8aa1d53451864ceb7fd73189b6661cb))
- Use pandas-gbq to determine schema in `load_table_from_dataframe` ([#2095](https://github.com/googleapis/python-bigquery/issues/2095)) ([7603bd7](https://github.com/googleapis/python-bigquery/commit/7603bd71d60592ef2a551d9eea09987b218edc73))

##### Documentation

- Update magics.rst ([#2125](https://github.com/googleapis/python-bigquery/issues/2125)) ([b5bcfb3](https://github.com/googleapis/python-bigquery/commit/b5bcfb303d27015b747a3b0747ecd7f7ed0ed557))
Libraries

### Java

#### [2.48.1](https://github.com/googleapis/java-bigquery/compare/v2.48.0...v2.48.1) (2025-02-26)

##### Dependencies

- Update actions/upload-artifact action to v4.6.1 ([#3691](https://github.com/googleapis/java-bigquery/issues/3691)) ([9c0edea](https://github.com/googleapis/java-bigquery/commit/9c0edea7c00b3ffbe6b6a404e4161f768acb34f2))
- Update dependency com.google.api.grpc:proto-google-cloud-bigqueryconnection-v1 to v2.60.0 ([#3680](https://github.com/googleapis/java-bigquery/issues/3680)) ([6d9a40d](https://github.com/googleapis/java-bigquery/commit/6d9a40d55a6bbcbff7df39723d33f0af2b24f66e))
- Update dependency com.google.apis:google-api-services-bigquery to v2-rev20250216-2.0.0 ([#3688](https://github.com/googleapis/java-bigquery/issues/3688)) ([e3beb6f](https://github.com/googleapis/java-bigquery/commit/e3beb6ffe433db8ad4087d0f27a8f0d23e7c9322))
- Update dependency com.google.cloud:google-cloud-datacatalog-bom to v1.64.0 ([#3681](https://github.com/googleapis/java-bigquery/issues/3681)) ([9e4e261](https://github.com/googleapis/java-bigquery/commit/9e4e26116226d17cc42ae030eed284bd6674b74b))
- Update dependency com.google.cloud:sdk-platform-java-config to v3.44.0 ([#3694](https://github.com/googleapis/java-bigquery/issues/3694)) ([f69fbd3](https://github.com/googleapis/java-bigquery/commit/f69fbd371f18da6ddc43d4f32f532e684026fe16))
- Update dependency com.google.oauth-client:google-oauth-client-java6 to v1.38.0 ([#3685](https://github.com/googleapis/java-bigquery/issues/3685)) ([53bd7af](https://github.com/googleapis/java-bigquery/commit/53bd7af47783674a3accbadb1172edbcf628ab2b))
- Update dependency com.google.oauth-client:google-oauth-client-jetty to v1.38.0 ([#3686](https://github.com/googleapis/java-bigquery/issues/3686)) ([d71b2a3](https://github.com/googleapis/java-bigquery/commit/d71b2a34a728fb6ee1c88cdc895b87959e230b7a))
- Update ossf/scorecard-action action to v2.4.1 ([#3690](https://github.com/googleapis/java-bigquery/issues/3690)) ([cdb61fe](https://github.com/googleapis/java-bigquery/commit/cdb61febcb1a64f6ddd3c0e3c29fa7995f1d3fa5))
Feature Gemini in BigQuery can help you [complete Python code](https://docs.cloud.google.com/bigquery/docs/write-sql-gemini#complete_python_code) with contextually appropriate recommendations that are based on content in the query editor. This feature is now [generally available](https://cloud.google.com/products#product-launch-stages) (GA).
Feature You can create a [SQL user-defined aggregate function](https://docs.cloud.google.com/bigquery/docs/user-defined-aggregates#create-sql-udaf) by using the [`CREATE AGGREGATE FUNCTION` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#sql-create-udaf-function). This feature is [generally available](https://cloud.google.com/products#product-launch-stages) (GA).

## February 25, 2025

Feature BigQuery [resource utilization charts](https://docs.cloud.google.com/bigquery/docs/admin-resource-charts#view-resource-utilization) provide metrics views and more chart configuration options in [Preview](https://cloud.google.com/products/#product-launch-stages).
Feature You can use the [best sellers](https://docs.cloud.google.com/bigquery/docs/merchant-center-best-sellers-migration) and [price competitiveness](https://docs.cloud.google.com/bigquery/docs/merchant-center-price-competitiveness-migration) migration guides to transition to the newer version of the reports. This feature is in [preview](https://cloud.google.com/products#product-launch-stages).
Announcement You can now see a [list of BigQuery API and service dependencies](https://docs.cloud.google.com/bigquery/docs/service-dependencies). You can also review the effects of disabling an API or service.

## February 24, 2025

Feature You can now use the [`@@location` system variable](https://docs.cloud.google.com/bigquery/docs/reference/system-variables) to set the location in which to run a query. This feature is in [preview](https://cloud.google.com/products#product-launch-stages).

## February 17, 2025

Libraries

### Java

#### [2.48.0](https://github.com/googleapis/java-bigquery/compare/v2.47.0...v2.48.0) (2025-02-13)

##### Features

- Implement wasNull for BigQueryResultSet ([#3650](https://github.com/googleapis/java-bigquery/issues/3650)) ([c7ef94b](https://github.com/googleapis/java-bigquery/commit/c7ef94be115cd572df589385f9be801033d72d6d))

##### Dependencies

- Update dependency com.google.api.grpc:proto-google-cloud-bigqueryconnection-v1 to v2.59.0 ([#3660](https://github.com/googleapis/java-bigquery/issues/3660)) ([3a6228b](https://github.com/googleapis/java-bigquery/commit/3a6228b4adc638759d3b2725c612e97e1a3b9cec))
- Update dependency com.google.apis:google-api-services-bigquery to v2-rev20250128-2.0.0 ([#3667](https://github.com/googleapis/java-bigquery/issues/3667)) ([0b92af6](https://github.com/googleapis/java-bigquery/commit/0b92af6eba4a633bb514089c24b7dd19cf286789))
- Update dependency com.google.cloud:google-cloud-datacatalog-bom to v1.63.0 ([#3661](https://github.com/googleapis/java-bigquery/issues/3661)) ([9bc8c01](https://github.com/googleapis/java-bigquery/commit/9bc8c0115dc16fb950567cd85cc7dfaa9df50d7d))
- Update dependency com.google.cloud:sdk-platform-java-config to v3.43.0 ([#3669](https://github.com/googleapis/java-bigquery/issues/3669)) ([4d9e0ff](https://github.com/googleapis/java-bigquery/commit/4d9e0ff30269127f47484910e71fa7a21a735492))

##### Documentation

- Update CONTRIBUTING.md for users without branch permissions ([#3670](https://github.com/googleapis/java-bigquery/issues/3670)) ([009b9a2](https://github.com/googleapis/java-bigquery/commit/009b9a2b3940ab66220e68ddd565710b8552cc45))
Libraries

### Node.js

#### [7.9.2](https://github.com/googleapis/nodejs-bigquery/compare/v7.9.1...v7.9.2) (2025-02-12)

##### Bug Fixes

- Avoid schema field mutation when passing selectedFields opt ([#1437](https://github.com/googleapis/nodejs-bigquery/issues/1437)) ([27044d5](https://github.com/googleapis/nodejs-bigquery/commit/27044d52e6bb6b4b6dbc746a0cfb02951817d7f1))

### Java

#### [2.48.0](https://github.com/googleapis/java-bigquery/compare/v2.47.0...v2.48.0) (2025-02-13)

##### Features

- Implement wasNull for BigQueryResultSet ([#3650](https://github.com/googleapis/java-bigquery/issues/3650)) ([c7ef94b](https://github.com/googleapis/java-bigquery/commit/c7ef94be115cd572df589385f9be801033d72d6d))

##### Dependencies

- Update dependency com.google.api.grpc:proto-google-cloud-bigqueryconnection-v1 to v2.59.0 ([#3660](https://github.com/googleapis/java-bigquery/issues/3660)) ([3a6228b](https://github.com/googleapis/java-bigquery/commit/3a6228b4adc638759d3b2725c612e97e1a3b9cec))
- Update dependency com.google.apis:google-api-services-bigquery to v2-rev20250128-2.0.0 ([#3667](https://github.com/googleapis/java-bigquery/issues/3667)) ([0b92af6](https://github.com/googleapis/java-bigquery/commit/0b92af6eba4a633bb514089c24b7dd19cf286789))
- Update dependency com.google.cloud:google-cloud-datacatalog-bom to v1.63.0 ([#3661](https://github.com/googleapis/java-bigquery/issues/3661)) ([9bc8c01](https://github.com/googleapis/java-bigquery/commit/9bc8c0115dc16fb950567cd85cc7dfaa9df50d7d))
- Update dependency com.google.cloud:sdk-platform-java-config to v3.43.0 ([#3669](https://github.com/googleapis/java-bigquery/issues/3669)) ([4d9e0ff](https://github.com/googleapis/java-bigquery/commit/4d9e0ff30269127f47484910e71fa7a21a735492))

##### Documentation

- Update CONTRIBUTING.md for users without branch permissions ([#3670](https://github.com/googleapis/java-bigquery/issues/3670)) ([009b9a2](https://github.com/googleapis/java-bigquery/commit/009b9a2b3940ab66220e68ddd565710b8552cc45))
Feature Subscriber email logging lets you log the principal identifiers of users who execute jobs and queries against linked datasets. You can enable logging at the [listing level](https://docs.cloud.google.com/bigquery/docs/analytics-hub-manage-listings#create_a_listing) and the [data exchange level](https://docs.cloud.google.com/bigquery/docs/analytics-hub-manage-exchanges#create-exchange) (for all the listings in the data exchange). Once you enable and save subscriber email logging, this setting cannot be edited. This feature is in [preview](https://cloud.google.com/products#product-launch-stages).

## February 10, 2025

Libraries

### Go

#### [1.66.2](https://github.com/googleapis/google-cloud-go/compare/bigquery/v1.66.1...bigquery/v1.66.2) (2025-02-04)

##### Bug Fixes

- **bigquery:** Broken github.com/envoyproxy/go-control-plane/envoy dep ([#11556](https://github.com/googleapis/google-cloud-go/issues/11556)) ([e70d63b](https://github.com/googleapis/google-cloud-go/commit/e70d63bbc267c3b166bf264670b8b282a3651cc5)), refs [#11542](https://github.com/googleapis/google-cloud-go/issues/11542)
Libraries

### Go

#### [1.66.1](https://github.com/googleapis/google-cloud-go/compare/bigquery/v1.66.0...bigquery/v1.66.1) (2025-02-03)

##### Bug Fixes

- **bigquery:** Move MaxStaleness field to table level ([#10066](https://github.com/googleapis/google-cloud-go/issues/10066)) ([164492d](https://github.com/googleapis/google-cloud-go/commit/164492d749ef0eeaf03a93d94b4a2c6c407eb4d6))
Feature BigQuery data preparation provides [context-aware join operation recommendations from Gemini](https://docs.cloud.google.com/bigquery/docs/data-prep-get-suggestions#add-join). Data preparation is available in [Preview](https://cloud.google.com/products#product-launch-stages).

## February 06, 2025

Feature You can create a [JavaScript user-defined aggregate function](https://docs.cloud.google.com/bigquery/docs/user-defined-aggregates#create-javascript-udaf) by using the [`CREATE AGGREGATE FUNCTION` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#javascript-create-udaf-function). This feature is [generally available](https://cloud.google.com/products#product-launch-stages) (GA).

## February 03, 2025

Libraries

### Java

#### [2.47.0](https://github.com/googleapis/java-bigquery/compare/v2.46.0...v2.47.0) (2025-01-29)

##### Features

- **bigquery:** Support resource tags for datasets in java client ([#3647](https://github.com/googleapis/java-bigquery/issues/3647)) ([01e0b74](https://github.com/googleapis/java-bigquery/commit/01e0b742b9ffeafaa89b080a39d8a66c12c1fd3b))

##### Bug Fixes

- **bigquery:** Remove ReadAPI bypass in executeSelect() ([#3624](https://github.com/googleapis/java-bigquery/issues/3624)) ([fadd992](https://github.com/googleapis/java-bigquery/commit/fadd992a63fd1bc87c99cc689ed103f05de49a99))
- Close bq read client ([#3644](https://github.com/googleapis/java-bigquery/issues/3644)) ([8833c97](https://github.com/googleapis/java-bigquery/commit/8833c97d73e3ba8e6a2061bbc55a6254b9e6668e))

##### Dependencies

- Update dependency com.google.apis:google-api-services-bigquery to v2-rev20250112-2.0.0 ([#3651](https://github.com/googleapis/java-bigquery/issues/3651)) ([fd06100](https://github.com/googleapis/java-bigquery/commit/fd06100c4c18b0416d384ec1f6bdfc796b70ad9f))
- Update dependency com.google.cloud:sdk-platform-java-config to v3.42.0 ([#3653](https://github.com/googleapis/java-bigquery/issues/3653)) ([1a14342](https://github.com/googleapis/java-bigquery/commit/1a143428c7f584db3dd6e827c2ee8fe980afe18c))
- Update github/codeql-action action to v2.28.1 ([#3637](https://github.com/googleapis/java-bigquery/issues/3637)) ([858e517](https://github.com/googleapis/java-bigquery/commit/858e51792d98276f10fd780ef6edd0bb4a1b4f54))
Feature You can now use the [`BY NAME` and `CORRESPONDING` modifiers](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#by_name_or_corresponding) with set operations to match columns by name instead of by position. This feature is [generally available](https://cloud.google.com/products#product-launch-stages) (GA).
Change The BigQuery ML [`ML.BUCKETIZE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-bucketize) and [`ML.QUANTILE_BUCKETIZE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-quantile-bucketize) functions now support formatting of the function output. You can use the `output_format` argument to format the function output as one of the following:

- A string in the format `bin_<bucket_index>`
- A string in [interval notation](https://en.wikipedia.org/wiki/Interval_(mathematics))
- A JSON-formatted string

## January 28, 2025

Feature You can now view [stored column usage](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/Job#StoredColumnsUsage) information for a query job that performs vector search using stored columns. This feature is [generally available](https://cloud.google.com/products#product-launch-stages) (GA).

## January 27, 2025

Libraries

### Go

#### [1.66.0](https://github.com/googleapis/google-cloud-go/compare/bigquery/v1.65.0...bigquery/v1.66.0) (2025-01-20)

##### Features

- **bigquery/storage/managedwriter:** Graceful connection drains ([#11463](https://github.com/googleapis/google-cloud-go/issues/11463)) ([b29912f](https://github.com/googleapis/google-cloud-go/commit/b29912faab73a2e708127eeb2f729ae581e7a24e))

##### Bug Fixes

- **bigquery:** Update golang.org/x/net to v0.33.0 ([e9b0b69](https://github.com/googleapis/google-cloud-go/commit/e9b0b69644ea5b276cacff0a707e8a5e87efafc9))
Libraries

### Python

#### [3.29.0](https://github.com/googleapis/python-bigquery/compare/v3.28.0...v3.29.0) (2025-01-21)

##### Features

- Add ExternalCatalogTableOptions class and tests ([#2116](https://github.com/googleapis/python-bigquery/issues/2116)) ([cdc1a6e](https://github.com/googleapis/python-bigquery/commit/cdc1a6e1623b8305c6a6a1a481b3365e866a073d))

##### Bug Fixes

- Add default value in SchemaField.from_api_repr() ([#2115](https://github.com/googleapis/python-bigquery/issues/2115)) ([7de6822](https://github.com/googleapis/python-bigquery/commit/7de6822e1c556a68cb8d50e90664c094697cca1d))
Feature You can now set [conditional IAM access on BigQuery datasets](https://docs.cloud.google.com/bigquery/docs/conditions) with access control lists (ACLs). This feature is [generally available](https://cloud.google.com/products#product-launch-stages) (GA).
Feature The following BigQuery ML generative AI features are now available:

- Creating a [remote model](https://bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model-open) based on an [open model from Vertex Model Garden or Hugging Face that is deployed to Vertex AI](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model-open). Options include Llama, Gemma, and other leading open text generation models.
- Using the [`ML.GENERATE_TEXT` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-generate-text) with this remote model to perform a broad range of generative AI tasks.
- Using the [`ML.EVALUATE` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-evaluate) to evaluate the remote model.

Try these features with the
[Generate text by using the `ML.GENERATE_TEXT` function](https://docs.cloud.google.com/bigquery/docs/generate-text)
how-to topic and the  

[Generate text by using a Gemma open model and the `ML.GENERATE_TEXT` function](https://docs.cloud.google.com/bigquery/docs/generate-text-tutorial-gemma)
tutorial.

These features are
[generally available](https://cloud.google.com/products/#product-launch-stages)
(GA).
Announcement We previously communicated that after January 27, 2025, a purchase would be required to use [Gemini in BigQuery features](https://cloud.google.com/gemini/docs/bigquery/overview). We are temporarily delaying enforcement of these procurement methods, and no purchase is required at this time. For more information, see [Gemini for Google Cloud pricing](https://cloud.google.com/products/gemini/pricing).

## January 22, 2025

Feature [BigQuery metastore](https://docs.cloud.google.com/bigquery/docs/about-bqms) lets you access and manage metadata from a variety of processing engines, including BigQuery and Apache Spark. BigQuery metastore supports BigQuery tables and open formats such as Apache Iceberg. This feature is in [preview](https://cloud.google.com/products#product-launch-stages).

## January 21, 2025

Feature You can use natural language to [prepare data with Gemini in BigQuery](https://docs.cloud.google.com/bigquery/docs/data-prep-get-suggestions).
Feature In BigQuery ML, you can now evaluate Anthropic Claude models by using the
[`ML.EVALUATE` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-evaluate).
[The quotas](https://docs.cloud.google.com/bigquery/quotas#cloud_ai_service_functions)
for use of Anthropic Claude models in BigQuery ML have also been brought into
parity with Vertex AI quotas.

This feature is in
[preview](https://cloud.google.com/products/#product-launch-stages).
Feature Data preparation in BigQuery lets you test data preparations you're developing before you deploy and schedule runs in production. For more information, see [Develop a data preparation](https://docs.cloud.google.com/bigquery/docs/orchestrate-data-preparations#develop).

## January 20, 2025

Libraries

### Java

#### [2.46.0](https://github.com/googleapis/java-bigquery/compare/v2.45.0...v2.46.0) (2025-01-11)

##### Features

- **bigquery:** Support IAM conditions in datasets in Java client. ([#3602](https://github.com/googleapis/java-bigquery/issues/3602)) ([6696a9c](https://github.com/googleapis/java-bigquery/commit/6696a9c7d42970e3c24bda4da713a855dbe40ce5))

##### Bug Fixes

- NPE when reading BigQueryResultSet from empty tables ([#3627](https://github.com/googleapis/java-bigquery/issues/3627)) ([9a0b05a](https://github.com/googleapis/java-bigquery/commit/9a0b05a3b57797b7cdd8ca9739699fc018dbd868))
- **test:** Force usage of ReadAPI ([#3625](https://github.com/googleapis/java-bigquery/issues/3625)) ([5ca7d4a](https://github.com/googleapis/java-bigquery/commit/5ca7d4acbbc40d6ef337732464b3bbd130c86430))

##### Dependencies

- Update actions/upload-artifact action to v4.5.0 ([#3620](https://github.com/googleapis/java-bigquery/issues/3620)) ([cc25099](https://github.com/googleapis/java-bigquery/commit/cc25099f81cbf94e9e2ee9db03a7d9ecd913c176))
- Update actions/upload-artifact action to v4.6.0 ([#3633](https://github.com/googleapis/java-bigquery/issues/3633)) ([ca20aa4](https://github.com/googleapis/java-bigquery/commit/ca20aa47ea7826594975ab6aeb8498e2377f8553))
- Update dependency com.google.api.grpc:proto-google-cloud-bigqueryconnection-v1 to v2.57.0 ([#3617](https://github.com/googleapis/java-bigquery/issues/3617)) ([51370a9](https://github.com/googleapis/java-bigquery/commit/51370a92e7ab29dfce91199666f23576d2d1b64a))
- Update dependency com.google.api.grpc:proto-google-cloud-bigqueryconnection-v1 to v2.58.0 ([#3631](https://github.com/googleapis/java-bigquery/issues/3631)) ([b0ea0d5](https://github.com/googleapis/java-bigquery/commit/b0ea0d5bc4ac730b0e2eaf47e8a7441dc113686b))
- Update dependency com.google.apis:google-api-services-bigquery to v2-rev20241222-2.0.0 ([#3623](https://github.com/googleapis/java-bigquery/issues/3623)) ([4061922](https://github.com/googleapis/java-bigquery/commit/4061922e46135d673bfa48c00bbf284efa46e065))
- Update dependency com.google.cloud:google-cloud-datacatalog-bom to v1.61.0 ([#3618](https://github.com/googleapis/java-bigquery/issues/3618)) ([6cba626](https://github.com/googleapis/java-bigquery/commit/6cba626ff14cebbc04fa4f6058b273de0c5dd96e))
- Update dependency com.google.cloud:google-cloud-datacatalog-bom to v1.62.0 ([#3632](https://github.com/googleapis/java-bigquery/issues/3632)) ([e9ff265](https://github.com/googleapis/java-bigquery/commit/e9ff265041f6771a71c8c378ed3ff5fdec6e837b))
- Update dependency com.google.cloud:sdk-platform-java-config to v3.41.1 ([#3628](https://github.com/googleapis/java-bigquery/issues/3628)) ([442d217](https://github.com/googleapis/java-bigquery/commit/442d217606b7d93d26887344a7a4a01303b18b8c))
- Update dependency com.google.oauth-client:google-oauth-client-java6 to v1.37.0 ([#3614](https://github.com/googleapis/java-bigquery/issues/3614)) ([f5faa69](https://github.com/googleapis/java-bigquery/commit/f5faa69bc5b6fdae137724df5693f8aecf27d609))
- Update dependency com.google.oauth-client:google-oauth-client-jetty to v1.37.0 ([#3615](https://github.com/googleapis/java-bigquery/issues/3615)) ([a6c7944](https://github.com/googleapis/java-bigquery/commit/a6c79443a5e675a01ecb91e362e261a6f6ecc055))
- Update github/codeql-action action to v2.27.9 ([#3608](https://github.com/googleapis/java-bigquery/issues/3608)) ([567ce01](https://github.com/googleapis/java-bigquery/commit/567ce01ed77d44760ddcd872a0d61abdd6a09832))
- Update github/codeql-action action to v2.28.0 ([#3621](https://github.com/googleapis/java-bigquery/issues/3621)) ([e0e09ec](https://github.com/googleapis/java-bigquery/commit/e0e09ec4954f5b5e2f094e4c67600f38353f453c))
Libraries

### Python

#### [3.28.0](https://github.com/googleapis/python-bigquery/compare/v3.27.0...v3.28.0) (2025-01-15) - YANKED

#### Reason this release was yanked:

This turned out to be incompatible with [pandas-gbq](https://github.com/googleapis/python-bigquery-pandas). For more details, see [issue](https://github.com/googleapis/python-bigquery-pandas/issues/854).

##### Features

- Add property for `allowNonIncrementalDefinition` for materialized view ([#2084](https://github.com/googleapis/python-bigquery/issues/2084)) ([3359ef3](https://github.com/googleapis/python-bigquery/commit/3359ef37b90243bea2d9e68bb996fe5d736f304c))
- Add property for maxStaleness in table definitions ([#2087](https://github.com/googleapis/python-bigquery/issues/2087)) ([729322c](https://github.com/googleapis/python-bigquery/commit/729322c2288a30464f2f135ba18b9c4aa7d2f0da))
- Add type hints to Client ([#2044](https://github.com/googleapis/python-bigquery/issues/2044)) ([40529de](https://github.com/googleapis/python-bigquery/commit/40529de923e25c41c6728c121b9c82a042967ada))
- Adds ExternalCatalogDatasetOptions and tests ([#2111](https://github.com/googleapis/python-bigquery/issues/2111)) ([b929a90](https://github.com/googleapis/python-bigquery/commit/b929a900d49e2c15897134209ed9de5fc7f238cd))
- Adds ForeignTypeInfo class and tests ([#2110](https://github.com/googleapis/python-bigquery/issues/2110)) ([55ca63c](https://github.com/googleapis/python-bigquery/commit/55ca63c23fcb56573e2de67e4f7899939628c4a1))
- Adds new input validation function similar to isinstance. ([#2107](https://github.com/googleapis/python-bigquery/issues/2107)) ([a2bebb9](https://github.com/googleapis/python-bigquery/commit/a2bebb95c5ef32ac7c7cbe19c3e7a9412cbee60d))
- Adds StorageDescriptor and tests ([#2109](https://github.com/googleapis/python-bigquery/issues/2109)) ([6be0272](https://github.com/googleapis/python-bigquery/commit/6be0272ff25dac97a38ae4ee5aa02016dc82a0d8))
- Adds the SerDeInfo class and tests ([#2108](https://github.com/googleapis/python-bigquery/issues/2108)) ([62960f2](https://github.com/googleapis/python-bigquery/commit/62960f255d05b15940a8d2cdc595592175fada11))
- Migrate to pyproject.toml ([#2041](https://github.com/googleapis/python-bigquery/issues/2041)) ([1061611](https://github.com/googleapis/python-bigquery/commit/106161180ead01aca1ead909cf06ca559f68666d))
- Preserve unknown fields from the REST API representation in `SchemaField` ([#2097](https://github.com/googleapis/python-bigquery/issues/2097)) ([aaf1eb8](https://github.com/googleapis/python-bigquery/commit/aaf1eb85ada95ab866be0199812ea7f5c7f50766))
- Resource tags in dataset ([#2090](https://github.com/googleapis/python-bigquery/issues/2090)) ([3e13016](https://github.com/googleapis/python-bigquery/commit/3e130166f43dcc06704fe90edf9068dfd44842a6))
- Support setting max_stream_count when fetching query result ([#2051](https://github.com/googleapis/python-bigquery/issues/2051)) ([d461297](https://github.com/googleapis/python-bigquery/commit/d4612979b812d2a835e47200f27a87a66bcb856a))

##### Bug Fixes

- Allow geopandas 1.x ([#2065](https://github.com/googleapis/python-bigquery/issues/2065)) ([f2ab8cb](https://github.com/googleapis/python-bigquery/commit/f2ab8cbfe00d442ad3b40683ecfec320e53b4688))

##### Documentation

- Render fields correctly for update calls ([#2055](https://github.com/googleapis/python-bigquery/issues/2055)) ([a4d9534](https://github.com/googleapis/python-bigquery/commit/a4d9534a900f13ae7355904cda05097d781f27e3))

## January 17, 2025

Feature In the [navigation menu](https://docs.cloud.google.com/bigquery/docs/bigquery-web-ui#navigation_menu), you can now go to the **Settings** page to set default settings that are applied when you start a session in BigQuery Studio. This feature is in [preview](https://cloud.google.com/products#product-launch-stages).
Feature The BigQuery Data Transfer Service can now transfer data from the following data sources:

- [MySQL](https://docs.cloud.google.com/bigquery/docs/mysql-transfer)
- [PostgreSQL](https://docs.cloud.google.com/bigquery/docs/postgresql-transfer)

Transfers from these data sources are supported in [Preview](https://cloud.google.com/products#product-launch-stages).

## January 16, 2025

Feature The [BigQuery migration assessment for Oracle](https://docs.cloud.google.com/bigquery/docs/migration-assessment) now includes a total cost of ownership (TCO) calculator that provides an estimation of compute and storage costs for migrating your Oracle data warehouse to BigQuery. This feature is in [preview](https://cloud.google.com/products#product-launch-stages).
Feature We have rearranged the [navigation menu](https://docs.cloud.google.com/bigquery/docs/bigquery-web-ui#navigation_menu) into new categories. This feature is [generally available](https://cloud.google.com/products#product-launch-stages) (GA).

## January 13, 2025

Feature You can now use BigQuery Omni Virtual Private Cloud (VPC) allowlists to restrict access to [AWS S3 buckets](https://docs.cloud.google.com/bigquery/docs/omni-aws-create-external-table#allow-vpc) and [Azure Blob Storage](https://docs.cloud.google.com/bigquery/docs/omni-azure-create-external-table#allow-vpc) from specific BigQuery Omni VPCs. This feature is [generally available](https://cloud.google.com/products#product-launch-stages) (GA).
Feature In BigQuery ML, you can now forecast multiple time series at once by using the
new
[`TIME_SERIES_ID_COL` option](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-multivariate-time-series#time_series_id_col)
that is available in `ARIMA_PLUS_XREG` multivariate time series models. Try this
feature with the
[Forecast multiple time series with a multivariate model](https://docs.cloud.google.com/bigquery/docs/arima-plus-xreg-multiple-time-series-forecasting-tutorial)
tutorial.

This feature is in
[preview](https://cloud.google.com/products/#product-launch-stages).

## January 02, 2025

Change An updated version of [JDBC driver for BigQuery](https://docs.cloud.google.com/bigquery/docs/reference/odbc-jdbc-drivers#current_jdbc_driver) is now available.