# Overview of BigQuery analytics

This document describes how BigQuery processes queries, and it provides
an overview of several features that are useful for understanding and
analyzing your data.

BigQuery is optimized to run analytic queries on large datasets,
including terabytes of data in seconds and petabytes in minutes. Understanding
its capabilities and how it processes queries can help you maximize your data
analysis investments.

## Analytic workflows

BigQuery supports several data analysis workflows:

- **Ad hoc analysis.** BigQuery uses
  [GoogleSQL](https://docs.cloud.google.com/bigquery/docs/introduction-sql),
  the SQL dialect in BigQuery, to support ad hoc
  analysis. You can run queries in the Google Cloud console or through
  [third-party tools](https://docs.cloud.google.com/bigquery/docs/query-overview#third-party_tool_integration)
  that integrate with BigQuery.

- **Geospatial analysis.** BigQuery uses geography data types and
  GoogleSQL geography functions to let you analyze and visualize
  geospatial data. For information about these data types and functions, see
  [Introduction to geospatial analytics](https://docs.cloud.google.com/bigquery/docs/geospatial-intro).

- **Graph analysis.** [BigQuery Graph](https://docs.cloud.google.com/bigquery/docs/graph-overview)
  lets you model your data as
  a graph with nodes and edges. You can use Graph Query Language (GQL) to find
  complex, hidden relationships between data points that would be challenging to
  find using SQL.

- **Search for data.** You can [index your data](https://docs.cloud.google.com/bigquery/docs/search-index) to
  perform flexible, optimized [searches](https://docs.cloud.google.com/bigquery/docs/search) on unstructured
  text or semi-structured JSON data.

- **Search for Google Cloud resources.** Use
  [natural language search](https://docs.cloud.google.com/bigquery/docs/search-resources) ([Preview](https://cloud.google.com/products#product-launch-stages))
  to discover Google Cloud resources from within BigQuery.

- **Machine learning.** [BigQuery ML](https://docs.cloud.google.com/bigquery/docs/bqml-introduction)
  uses GoogleSQL queries to let you create and execute machine
  learning (ML) models in BigQuery.

- **Business intelligence.** [BigQuery BI Engine](https://docs.cloud.google.com/bigquery/docs/bi-engine-intro)
  is a fast, in-memory analysis service that lets you
  build rich, interactive dashboards and reports without compromising
  performance, scalability, security, or data freshness.

- **AI assistance.** You can use [Gemini in
  BigQuery](https://docs.cloud.google.com/bigquery/docs/gemini-overview) to prepare
  and explore your data, generate SQL queries and Python code, and visualize
  your results.

## Data exploration

BigQuery can help you understand
your data before you start writing SQL queries. Use the following features
if you want to find data, are unfamiliar with your data, don't know which
questions to ask, or need help writing SQL:

- [**Knowledge Catalog.**](https://docs.cloud.google.com/bigquery/docs/search-resources) Find
  Google Cloud resources from within BigQuery, such as
  datasets and tables.

- [**Table explorer.**](https://docs.cloud.google.com/bigquery/docs/table-explorer) Visually explore the
  range and frequency of values in your table and
  interactively build queries.

- [**Data insights.**](https://docs.cloud.google.com/bigquery/docs/data-insights) Generate
  natural language questions about your data, along with the SQL
  queries to answer those questions.

- [**Data profile scan.**](https://docs.cloud.google.com/bigquery/docs/data-profile-scan) See
  statistical characteristics of your data, including average, unique, maximum,
  and minimum values.

- [**Data canvas.**](https://docs.cloud.google.com/bigquery/docs/data-canvas) Query your data using natural
  language, visualize results with charts, and ask follow-up questions.

## Queries

The primary way to analyze data in BigQuery is to
[run a SQL query](https://docs.cloud.google.com/bigquery/docs/running-queries). The
[GoogleSQL dialect](https://docs.cloud.google.com/bigquery/docs/introduction-sql)
supports [SQL:2011](https://www.iso.org/standard/53681.html)
and includes extensions that support geospatial analysis and ML.

### Data sources

BigQuery lets you query the following types of data sources:

- **Data stored in BigQuery.** You can
  [load data into BigQuery](https://docs.cloud.google.com/bigquery/docs/loading-data),
  modify existing data by using [data manipulation language
  (DML)
  statements](https://docs.cloud.google.com/bigquery/docs/data-manipulation-language),
  or [write query results](https://docs.cloud.google.com/bigquery/docs/writing-results) to a table. You can
  [query historical data](https://docs.cloud.google.com/bigquery/docs/access-historical-data) from a point in
  time within your time travel window.

  You can query data stored in single-region or multi-region locations.
  A query that accesses data stored in more than one location can be treated as
  a [global query](https://docs.cloud.google.com/bigquery/docs/global-queries) [(Preview)](https://docs.cloud.google.com/products#product-launch-stages).
  Queries that reference data in multiple locations are always treated as
  global queries, even if one region is a single-region location and the other
  is a multi-region location that contains the single-region location.
- **External data.** You can query various external data sources such as
  Cloud Storage, or database services such as Spanner or
  Cloud SQL. For information about how to
  set up connections to external sources, see
  [Introduction to external data sources](https://docs.cloud.google.com/bigquery/docs/external-data-sources)

- **Multi-cloud data.** You can query data that's stored in other public clouds
  such as AWS or Azure. For information on how to set up connections to
  Amazon Simple Storage Service (Amazon S3) or Azure Blob Storage, see
  [Introduction to BigQuery Omni](https://docs.cloud.google.com/bigquery/docs/omni-introduction).

- **Public datasets.** You can analyze any of
  the datasets that are available in the
  [public dataset marketplace](https://docs.cloud.google.com/bigquery/public-data).

- **BigQuery sharing (formerly Analytics Hub).** You can publish and
  subscribe to BigQuery datasets and Pub/Sub topics to
  share data across organizational boundaries. For more information, see
  [Introduction to BigQuery sharing](https://docs.cloud.google.com/bigquery/docs/analytics-hub-introduction).

### Types of queries

You can [query BigQuery data](https://docs.cloud.google.com/bigquery/docs/running-queries)
by using one of the following query job types:

- **[Interactive query jobs](https://docs.cloud.google.com/bigquery/docs/running-queries#queries)**. By
  default, BigQuery runs queries as interactive query jobs, which
  are intended to start executing as quickly as possible.

- **[Batch query jobs](https://docs.cloud.google.com/bigquery/docs/running-queries#batch)** . Batch queries
  have lower priority than interactive queries. When a project or reservation
  is using all of its available compute resources, batch queries are more
  likely to be queued and remain in the queue. After a batch query starts
  running, the batch query runs the same as an interactive query. For more
  information, see [query queues](https://docs.cloud.google.com/bigquery/docs/query-queues).

- **[Continuous query jobs](https://docs.cloud.google.com/bigquery/docs/continuous-queries-introduction)**.
  With these jobs, the query runs continuously, letting you analyze
  incoming data in BigQuery in real time and then write the
  results to a BigQuery table, or export the results to
  Bigtable or Pub/Sub. You can use this capability to
  perform time sensitive tasks, such as creating and immediately acting on
  insights, applying real time machine learning (ML) inference, and
  building event-driven data pipelines.

You can run query jobs by using the following methods:

- Compose and run a query in the [Google Cloud console](https://docs.cloud.google.com/bigquery/bigquery-web-ui#overview).
- Run the `bq query` command in the [bq command-line tool](https://docs.cloud.google.com/bigquery/bq-command-line-tool).
- Programmatically call the [`jobs.query`](https://docs.cloud.google.com/bigquery/docs/reference/v2/jobs/query) or [`jobs.insert`](https://docs.cloud.google.com/bigquery/docs/reference/v2/jobs/insert) method in the BigQuery [REST API](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2).
- Use the BigQuery [client libraries](https://docs.cloud.google.com/bigquery/docs/reference/libraries).

### Multi-statement queries

You can run multiple statements in a sequence, with shared state, by using
[multi-statement queries](https://docs.cloud.google.com/bigquery/docs/multi-statement-queries).
Multi-statement queries are often used in
[stored procedures](https://docs.cloud.google.com/bigquery/docs/procedures) and support
[procedural language statements](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/procedural-language),
which let you define variables and implement control flow.

### Saved and shared queries

BigQuery lets you
[save queries](https://docs.cloud.google.com/bigquery/docs/work-with-saved-queries#create_saved_queries)
and
[share queries](https://docs.cloud.google.com/bigquery/docs/work-with-saved-queries#share-saved-query)
with others.

When you save a query, it can be private (visible only to you), shared at the
project level (visible to specific principals), or public (anyone can view it).
For more information, see
[Work with saved queries](https://docs.cloud.google.com/bigquery/docs/work-with-saved-queries).

### How BigQuery processes queries

Several processes occur when BigQuery runs a query:

- **Execution tree.** When you run a query, BigQuery
  generates an *execution tree* that breaks the query into stages. These stages
  contain steps that can run in parallel.

- **Shuffle tier.** Stages communicate with one another by using a fast,
  distributed *shuffle tier* that stores intermediate data produced by the
  workers of a stage. When possible, the shuffle tier leverages technologies
  such as a petabit network and RAM to quickly move data to worker nodes.

- **Query plan.** When BigQuery has all the information that it
  needs to run a query, it generates a *query plan* . You can
  [view the query plan](https://docs.cloud.google.com/bigquery/docs/query-plan-explanation) in
  the Google Cloud console and use it to troubleshoot or
  [optimize query performance](https://docs.cloud.google.com/bigquery/docs/best-practices-performance-compute).

- **Query execution graph.** You can review the query plan information in
  graphical format for any query, whether running or completed, and see
  [performance insights](https://docs.cloud.google.com/bigquery/docs/query-insights) to help you optimize
  your queries.

- **Query monitoring and dynamic planning.** Besides the workers that perform
  the work of the query plan itself, additional workers monitor and direct the
  overall progress of work throughout the system. As the query progresses,
  BigQuery might dynamically adjust the query plan to adapt to
  the results of the various stages.

- **Query results.** When a query is complete, BigQuery writes
  the results to persistent storage and returns them to the user. This design
  lets BigQuery serve
  [cached results](https://docs.cloud.google.com/bigquery/docs/cached-results) the next time that query is
  run.

### Query concurrency and performance

The performance of queries that are run repeatedly on the same data can vary
because of the
shared nature of the BigQuery environment, use of
cached query results, or because
BigQuery dynamically adjusts the query plan while the query runs.
For a typical busy system where many queries run concurrently,
BigQuery uses several processes to smooth out variances in query
performance:

- BigQuery runs many queries in parallel and can
  [queue queries](https://docs.cloud.google.com/bigquery/docs/query-queues) to run when resources are
  available.

- As queries start and finish, BigQuery redistributes
  resources fairly between new and running queries. This process ensures that
  query performance doesn't depend on the order in which queries are submitted
  but rather on the number of queries run at a given time.

### Query optimization

When you run a query, you can
[view the query plan](https://docs.cloud.google.com/bigquery/docs/query-insights)
in the Google Cloud console. You can also request execution details by using
the
[`INFORMATION_SCHEMA.JOBS*` views](https://docs.cloud.google.com/bigquery/docs/information-schema-jobs)
or the
[`jobs.get` REST API method](https://docs.cloud.google.com/bigquery/docs/query-plan-explanation#api_sample_representation).

The query plan includes details about query stages and steps. These details can
help you identify ways to improve query performance. For example, if you notice
a stage that writes a lot more output than other stages, it might mean that you
need to filter earlier in the query.

For more information about the query plan and query optimization, see the
following resources:

- To learn more about the query plan and see examples of how the plan information can help you to improve query performance, see [Query plan and timeline](https://docs.cloud.google.com/bigquery/docs/query-plan-explanation).
- For more information about query optimization in general, see [Introduction to optimizing query performance](https://docs.cloud.google.com/bigquery/docs/best-practices-performance-overview).

### Query monitoring

Monitoring and logging are crucial for running reliable applications in the
cloud. BigQuery workloads are no exception, especially if your
workload has high volumes or is mission critical. BigQuery
provides various metrics, logs, and metadata views to help you monitor your
BigQuery usage.

For more information, see the following resources:

- To learn about monitoring options in BigQuery, see [Introduction to BigQuery monitoring](https://docs.cloud.google.com/bigquery/docs/monitoring).
- To learn about audit logs and how to analyze query behavior, see [BigQuery audit logs](https://docs.cloud.google.com/bigquery/docs/reference/auditlogs).

### Query pricing

BigQuery offers two pricing models for analytics:

- **[On-demand pricing](https://cloud.google.com/bigquery/pricing#on_demand_pricing).** You pay for the data scanned by your queries. You have a fixed, [query-processing capacity](https://docs.cloud.google.com/bigquery/quotas#max_concurrent_slots_on-demand) for each project, and your cost is based on the number of bytes processed.
- **[Capacity-based pricing](https://cloud.google.com/bigquery/pricing#capacity_compute_analysis_pricing).** You purchase dedicated query-processing capacity.

For information about the two pricing models and to learn more about making reservations
for capacity-based pricing, see [Introduction to reservations](https://docs.cloud.google.com/bigquery/docs/reservations-intro).

### Quotas and query cost controls

BigQuery enforces project-level quotas on running queries. For
information on query quotas, see [Quotas and limits](https://docs.cloud.google.com/bigquery/quotas).

To control query costs, BigQuery provides several options,
including custom quotas and billing alerts. For more information, see
[Creating custom cost controls](https://docs.cloud.google.com/bigquery/docs/custom-quotas).

## Data analytics features

BigQuery supports both descriptive and predictive analytics and
helps you explore your data with AI powered tools, SQL, machine learning,
notebooks, and other third-party integrations.

### BigQuery Studio

BigQuery Studio helps you discover, analyze, and run
inference on data in BigQuery with the following features:

- A robust [SQL editor](https://docs.cloud.google.com/bigquery/docs/running-queries) that provides code completion and generation, query validation, and estimation of bytes processed.
- Embedded [Python notebooks](https://docs.cloud.google.com/bigquery/docs/notebooks-introduction) built using [Colab Enterprise](https://docs.cloud.google.com/colab/docs/introduction). Notebooks provide one-click Python development runtimes, and built-in support for [BigQuery DataFrames](https://docs.cloud.google.com/python/docs/reference/bigframes/latest).
- A [PySpark editor](https://docs.cloud.google.com/bigquery/docs/spark-procedures#use-python-pyspark-editor) that lets you create stored Python procedures for Apache Spark.
- Asset management and version history for code assets such as notebooks and [saved queries](https://docs.cloud.google.com/bigquery/docs/saved-queries-introduction), built on top of [Dataform](https://docs.cloud.google.com/dataform).
- Assistive code development in the SQL editor and in notebooks, built on top of [Gemini generative AI](https://docs.cloud.google.com/bigquery/docs/write-sql-gemini) ([Preview](https://cloud.google.com/products/#product-launch-stages)).
- [Knowledge Catalog](https://docs.cloud.google.com/dataplex) features for [data discovery](https://docs.cloud.google.com/bigquery/docs/bigquery-web-ui#search-page), and [data profiling](https://docs.cloud.google.com/bigquery/docs/data-profile-scan) and [data quality](https://docs.cloud.google.com/bigquery/docs/data-quality-scan) scans.
- The ability to view [job history](https://docs.cloud.google.com/bigquery/docs/bigquery-web-ui#studio-overview) on a per-user or per-project basis.
- The ability to analyze saved query results by connecting to other tools such as Looker and Google Sheets, and to export saved query results for use in other applications.

> [!NOTE]
> **Note:** BigQuery Studio requires the following APIs which are enabled by default in projects and automated scripts created after March 24, 2024:
>
> - [Compute Engine API](https://docs.cloud.google.com/compute/docs/reference/rest/v1)
> - [Analytics Hub API](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest)
> - [Dataform API](https://docs.cloud.google.com/dataform/reference/rest)
> - [Vertex AI API](https://docs.cloud.google.com/vertex-ai/docs/reference/rest)
> - [BigQuery Connection API](https://docs.cloud.google.com/bigquery/docs/reference/bigqueryconnection/rest)
> - [BigQuery Data Policy API](https://docs.cloud.google.com/bigquery/docs/reference/bigquerydatapolicy/rest)
> - [BigQuery Reservation API](https://docs.cloud.google.com/bigquery/docs/reference/reservations/rest)
> - [Dataplex API](https://docs.cloud.google.com/dataplex/docs/reference/rest)

### BigQuery ML

BigQuery ML lets you use SQL in BigQuery to perform
machine learning (ML) and predictive analytics. For more information,
see [Introduction to BigQuery ML](https://docs.cloud.google.com/bigquery/docs/bqml-introduction).

The [Conversational Analytics Agent](https://docs.cloud.google.com/bigquery/docs/conversational-analytics)
lets you chat with your data using conversational language. This agent consists
of one or more data sources and a set of use case-specific instructions for
processing that data. Conversation analytics supports the use of
[some BigQuery ML functions](https://docs.cloud.google.com/bigquery/docs/conversational-analytics#bigquery-ml-support).

### Analytics tools integration

In addition to running queries in BigQuery, you can analyze your
data with various analytics and business intelligence tools that integrate with
BigQuery, such as the following:

- **Looker.** Looker is an enterprise platform for
  business intelligence, data applications, and embedded analytics. The
  Looker platform works with many datastores including
  BigQuery. For information on how to connect
  Looker to BigQuery, see
  [Using Looker](https://docs.cloud.google.com/bigquery/docs/looker).

- **Data Studio.** After you run a query, you can launch
  Data Studio directly from BigQuery in the
  Google Cloud console. Then, in Data Studio you can create
  visualizations and explore the data that's returned from the query. For
  information about Data Studio, see
  [Data Studio overview](https://lookerstudio.google.com/overview).

- **Connected Sheets.** You can also launch
  Connected Sheets directly from BigQuery in the
  console. Connected Sheets runs
  BigQuery queries on your behalf either upon your request or on
  a defined schedule. Results of those queries are saved in your spreadsheet for
  analysis and sharing. For information about Connected Sheets,
  see
  [Using connected sheets](https://docs.cloud.google.com/bigquery/docs/connected-sheets).

- **Tableau.** You can
  [connect to a dataset from Tableau](https://docs.cloud.google.com/bigquery/docs/analyze-data-tableau). Use
  BigQuery to power your charts, dashboards, and other data
  visualizations.

### Third-party tool integration

Several third-party analytics tools work with BigQuery.
For example, you can connect
[Tableau](https://docs.cloud.google.com/bigquery/docs/analyze-data-tableau)
to BigQuery data and use its visualization tools to analyze and
share your analysis. For more information on considerations when using
third-party tools, see
[Third-party tool integration](https://docs.cloud.google.com/bigquery/docs/third-party-integration).

ODBC and JDBC drivers are available and can be used to integrate your
application with BigQuery. The intent of these drivers is to help
users leverage the power of BigQuery with existing tooling and
infrastructure. For information on latest release and known issues, see
[ODBC and JDBC drivers for BigQuery](https://docs.cloud.google.com/bigquery/docs/reference/odbc-jdbc-drivers).

The pandas libraries like `pandas-gbq` let you interact with
BigQuery data in Jupyter notebooks. For information about this
library and how it compares with using the BigQuery
[Python client library](https://docs.cloud.google.com/bigquery/docs/reference/libraries),
see
[Comparison with `pandas-gbq`](https://docs.cloud.google.com/bigquery/docs/pandas-gbq-migration).

You can also use BigQuery with other notebooks and analysis
tools. For more information, see
[Programmatic analysis tools](https://docs.cloud.google.com/bigquery/docs/programmatic-analysis).

For a full list of BigQuery analytics and broader technology
partners, see the
[Partners](https://docs.cloud.google.com/bigquery#section-12)
list on the BigQuery product page.

## What's next

- For an introduction and overview of supported SQL statements, see [Introduction to SQL in BigQuery](https://docs.cloud.google.com/bigquery/docs/introduction-sql).
- To learn about the GoogleSQL syntax used for querying data in BigQuery, see [Query syntax in GoogleSQL](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax).
- Learn how to [run a query](https://docs.cloud.google.com/bigquery/docs/running-queries) in BigQuery.
- Learn more about [optimizing query performance](https://docs.cloud.google.com/bigquery/docs/best-practices-performance-overview).
- Learn about getting started with [notebooks](https://docs.cloud.google.com/bigquery/docs/programmatic-analysis).
- Learn how to [schedule a recurring query](https://docs.cloud.google.com/bigquery/docs/scheduling-queries).