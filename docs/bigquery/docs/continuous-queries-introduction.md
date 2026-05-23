# Introduction to continuous queries

This document describes BigQuery continuous queries.

BigQuery continuous queries are SQL statements that run
continuously. Continuous queries let you analyze incoming data in
BigQuery in real time. You can insert the output rows produced
by a continuous query into a BigQuery table or export them to
Pub/Sub, Bigtable, or Spanner. Continuous
queries can process data that has been written to
[standard BigQuery tables](https://docs.cloud.google.com/bigquery/docs/tables-intro#standard-tables)
by using one of the following methods:

- The [BigQuery Storage Write API](https://docs.cloud.google.com/bigquery/docs/write-api)
- The [`tabledata.insertAll` method](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tabledata/insertAll)
- [Batch load](https://docs.cloud.google.com/bigquery/docs/batch-loading-data)
- The [`INSERT` DML statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/dml-syntax#insert_statement)
- Mutating [data manipulation language (DML) statements](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/dml-syntax) such as `DELETE`, `UPDATE`, and `MERGE` when [exporting data to Pub/Sub](https://docs.cloud.google.com/bigquery/docs/export-to-pubsub).
- Writes from the [results of a batch query to a permanent table](https://docs.cloud.google.com/bigquery/docs/writing-results#permanent-table)
- Writes from the [results of a BigQuery continuous query to a permanent table](https://docs.cloud.google.com/bigquery/docs/continuous-queries#write-bigquery)
- A [Pub/Sub BigQuery subscription](https://docs.cloud.google.com/pubsub/docs/bigquery)
- Writes from [Dataflow to BigQuery](https://docs.cloud.google.com/dataflow/docs/guides/write-to-bigquery)
- Writes from Datastream to BigQuery using [append-only write mode](https://docs.cloud.google.com/datastream/docs/destination-bigquery#append-only_write_mode)

You can use continuous queries to perform time-sensitive tasks, such as
creating and immediately acting on insights, applying real-time machine
learning (ML) inference, and replicating data into other platforms. This
lets you use BigQuery as an event-driven data processing
engine for your application's decision logic.

The following diagram shows common continuous query workflows:

![Diagram illustrating common BigQuery continuous query workflows, including data ingestion, processing, and export to destinations like Bigtable and Pub/Sub.](https://docs.cloud.google.com/static/bigquery/images/continuous-queries.png)

## Use cases

Common use cases where you might want to use continuous queries are as follows:

- **Personalized customer interaction services**: use generative AI to create tailored messages customized for each customer interaction.
- **Anomaly detection**: build solutions that let you perform anomaly and threat detection on complex data in real time, so that you can react to issues more quickly.
- **Customizable event-driven pipelines**: use continuous query integration with Pub/Sub to trigger downstream applications based on incoming data.
- **Data enrichment and entity extraction**: use continuous queries to perform real-time data enrichment and transformation by using SQL functions and ML models.
- **Reverse extract-transform-load (ETL)**: perform real-time reverse ETL into other storage systems more suited for low latency application serving. For example, analyzing or enhancing event data that is written to BigQuery, and then streaming it to Bigtable or Spanner for application serving.
- **Autonomous agent triggering** : trigger agentic data pipelines in real-time based on complex events detected in live data streams. For an example, refer to the [Build an Event-Driven Data Agent with BigQuery and Agent Development Kit (ADK) codelab](https://codelabs.developers.google.com/bigquery-adk-event-driven-agents).
- **Autonomous agent monitoring** : develop real-time automated monitoring and alerting for real-time agentic interactions using the [BigQuery agent analytics plugin](https://adk.dev/integrations/bigquery-agent-analytics/), which streams all agent trace data, tool usage, and operational logs directly into BigQuery for deep observability into your AI workforce.

## Supported functionality

The following operations are supported in continuous queries:

- Running [`INSERT` statements](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/dml-syntax#insert_statement) to write data from a continuous query into a BigQuery table.
- Running
  [`EXPORT DATA` statements](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/export-statements)
  to [publish](https://docs.cloud.google.com/pubsub/docs/publish-message-overview)
  continuous query output to Pub/Sub topics. For
  more information, see
  [Export data to Pub/Sub](https://docs.cloud.google.com/bigquery/docs/export-to-pubsub).

  From a Pub/Sub topic, you can use the data with other
  services, such as performing streaming analytics by using Dataflow,
  or using the data in an application integration workflow.
- Running `EXPORT DATA` statements to export data from BigQuery
  to [Bigtable tables](https://docs.cloud.google.com/bigtable/docs/managing-tables).
  For more information, see
  [Export data to Bigtable](https://docs.cloud.google.com/bigquery/docs/export-to-bigtable).

- Running `EXPORT DATA` statements to export data from BigQuery
  to Spanner tables. For more information, see
  [Export data to Spanner (reverse ETL)](https://docs.cloud.google.com/bigquery/docs/export-to-spanner).

- Calling the following generative AI functions:

  - [`AI.GENERATE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-generate)
  - [`AI.GENERATE_TEXT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-generate-text)

    - This function requires you to have a [BigQuery ML remote model](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model) over a [Vertex AI model](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/learn/models).
- Calling the following AI functions:

  - [`ML.UNDERSTAND_TEXT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-understand-text)
  - [`ML.TRANSLATE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-translate)

  These functions require you to have a
  [BigQuery ML remote model](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model-service)
  over a Cloud AI API.
- Normalizing numerical data by using the
  [`ML.NORMALIZER` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-normalizer).

- Analyzing and processing `JSON` data, including support for
  [JSON functions](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions) and
  [JSON unnesting](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#unnest_operator).

- Using stateless GoogleSQL functions---for example,
  [conversion functions](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/conversion_functions).
  In stateless functions, each row is processed independently from other
  rows in the table.

- Using [stateful
  operations](https://docs.cloud.google.com/bigquery/docs/continuous-queries-introduction#supported_stateful_operations)---for
  example [`JOIN`s, aggregations, and window
  aggregations](https://docs.cloud.google.com/bigquery/docs/continuous-queries#join-agg-window-example). In
  stateful operations, the state of ingested data is retained across multiple
  rows or time intervals in order to compute an accurate result.

- Using the
  [`APPENDS`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/time-series-functions#appends)
  change history function to process appended data from a
  specific point in time.

- Using the
  [`CHANGES`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/time-series-functions#changes)
  change history function to process changed data, including both appends and
  mutations, from a specific point in time when
  [exporting data to Pub/Sub](https://docs.cloud.google.com/bigquery/docs/export-to-pubsub).
  However, `CHANGES` is not supported when using a
  [stateful operation](https://docs.cloud.google.com/bigquery/docs/continuous-queries-introduction#supported_stateful_operations).

## Supported stateful operations

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

To request support or provide feedback for this feature, send an email to
[bq-continuous-queries-feedback@google.com](mailto:bq-continuous-queries-feedback@google.com).

Stateful operations let continuous queries perform complex analysis that
requires retaining information across multiple rows or time intervals. While
stateless functions process each row independently, stateful operations maintain
the state of ingested data to support functions like [`JOIN`s, aggregations, and
window aggregations](https://docs.cloud.google.com/bigquery/docs/continuous-queries#join-agg-window-example). This
capability lets you correlate events from different streams or calculate metrics
over time---such as a 30-minute average---by storing necessary data in memory while
the query runs.

Continuous queries support the following stateful operations:

- [JOINs](https://docs.cloud.google.com/bigquery/docs/continuous-query-joins)
- [Aggregations and windowing](https://docs.cloud.google.com/bigquery/docs/window-aggregations)

## Authorization

The
[Google Cloud access tokens](https://docs.cloud.google.com/docs/authentication/token-types#access-tokens)
that are used when running continuous query jobs have a time to live (TTL) of
two days when they are generated by a user account. Therefore, such jobs stop
running after two days. The access tokens that are generated by service
accounts can run longer, but must still adhere to the maximum query runtime.
For more information, see
[Run a continuous query by using a service account](https://docs.cloud.google.com/bigquery/docs/continuous-queries#run_a_continuous_query_by_using_a_service_account).

## Locations

For a list of supported regions, see
[BigQuery continuous query locations](https://docs.cloud.google.com/bigquery/docs/locations#continuous-query-loc).

## Limitations

Continuous queries are subject to the following limitations:

- The state of ingested data is only maintained for the specific [stateful operations in Preview](https://docs.cloud.google.com/bigquery/docs/continuous-queries-introduction#supported_stateful_operations). While continuous queries now support some types of `JOIN`s, aggregations, and window aggregations, these are restricted to specific stateful operations. Not all types of stateful operations are supported.
- You can't use the following SQL capabilities in a continuous query, unless
  they are listed as a [supported stateful operation](https://docs.cloud.google.com/bigquery/docs/continuous-queries-introduction#supported_stateful_operations):

  - The following
    [query](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax) operators:

    - [`PIVOT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#pivot_operator)
    - [`UNPIVOT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#unpivot_operator)
    - [`TABLESAMPLE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#tablesample_operator)
  - Query [set operators](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#set_operators)

  - The [`SELECT DISTINCT` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#select_distinct)

  - [`EXISTS` or `NOT EXISTS` subqueries](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/subqueries#exists_subquery_concepts)

  - [Recursive CTEs](https://docs.cloud.google.com/bigquery/docs/recursive-ctes)

  - [User-defined functions](https://docs.cloud.google.com/bigquery/docs/user-defined-functions)

  - [Window function calls](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/window-function-calls)

  - BigQuery ML functions other than those listed in
    [Supported functionality](https://docs.cloud.google.com/bigquery/docs/continuous-queries-introduction#supported_functionality)

  - [Data definition language (DDL) statements](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language)

  - [Data manipulation language (DML) statements](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/dml-syntax)
    except for `INSERT`.

  - [Data control language (DCL) statements](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-control-language)

  - `EXPORT DATA` statements that don't target Bigtable,
    Pub/Sub, or Spanner.

  - [Procedural language](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/procedural-language)

  - [Debugging statements](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/debugging-statements)

- Continuous queries don't support the following data sources:

  - [External tables](https://docs.cloud.google.com/bigquery/docs/external-data-sources).
  - [Information schema views](https://docs.cloud.google.com/bigquery/docs/information-schema-intro).
  - [Apache Iceberg managed tables](https://docs.cloud.google.com/bigquery/docs/iceberg-tables).
  - [Wildcard tables](https://docs.cloud.google.com/bigquery/docs/querying-wildcard-tables).
  - [Change Data Capture (CDC) upsert](https://docs.cloud.google.com/bigquery/docs/change-data-capture) data.
  - [Materialized views](https://docs.cloud.google.com/bigquery/docs/materialized-views-intro).
  - [Views](https://docs.cloud.google.com/bigquery/docs/views) that are defined by other continuous query limitations, such as `JOIN` operations, aggregate functions, user-defined functions or change data capture-enabled tables.
- Continuous queries don't support the [column-](https://docs.cloud.google.com/bigquery/docs/column-level-security-intro)
  and [row-level](https://docs.cloud.google.com/bigquery/docs/row-level-security-intro) security features.

- The output of a continuous query is subject to the inherent quotas and limits
  of the destination service the output is being exported to.

- When exporting data to Bigtable, Spanner, or
  [Pub/Sub locational
  endpoints](https://docs.cloud.google.com/pubsub/docs/reference/service_apis_overview#pubsub_endpoints)
  you can only target Bigtable, Spanner, or
  Pub/Sub resources that fall within the same Google Cloud
  regional boundary as the BigQuery dataset that contains the
  table you are querying. This restriction doesn't apply when exporting data
  to Pub/Sub global endpoints.
  For more information about
  exporting to a [Bigtable app profile](https://docs.cloud.google.com/bigtable/docs/app-profiles)
  routing policy, see [Location considerations](https://docs.cloud.google.com/bigquery/docs/export-to-bigtable#data-locations).

- You can't run a continuous query from a
  [data canvas](https://docs.cloud.google.com/bigquery/docs/data-canvas).

- You can't modify the SQL used in a continuous query while the continuous
  query job is running. For more information, see
  [Modify the SQL of a continuous query](https://docs.cloud.google.com/bigquery/docs/continuous-queries#modify_the_sql_of_a_continuous_query).

- If a continuous query job falls behind in processing incoming data and has
  an [output watermark lag](https://docs.cloud.google.com/bigquery/docs/monitoring-dashboard#metrics) of
  more than 48 hours, then it fails. You can run the query again
  and use the
  [`APPENDS`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/time-series-functions#appends) or
  [`CHANGES`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/time-series-functions#changes)
  change history function to resume processing from the point in time at which
  you stopped the previous continuous query job. For more information, see
  [Start a continuous query from a particular point in time](https://docs.cloud.google.com/bigquery/docs/continuous-queries#start_a_continuous_query_from_a_particular_point_in_time).

- A continuous query configured with a user account can run for up to two
  days. A continuous query configured with a service account can run for
  up to 150 days. When the maximum query runtime is reached, the query fails
  and stops processing incoming data.

- Although continuous queries are built using [BigQuery
  reliability features](https://docs.cloud.google.com/bigquery/docs/reliability-intro), occasional
  temporary issues can occur. Issues might lead to some amount of automatic
  reprocessing of your continuous query, which could result in duplicate
  data in the continuous query output. Design your downstream systems to
  handle such scenarios.

### Reservation limitations

- You must create Enterprise edition or Enterprise Plus edition [reservations](https://docs.cloud.google.com/bigquery/docs/reservations-intro) in order to run continuous queries. Continuous queries don't support the on-demand compute billing model.
- When you create a `CONTINUOUS` [reservation assignment](https://docs.cloud.google.com/bigquery/docs/reservations-assignments), the associated reservation is limited to at most 500 slots. You can request an increase to this limit by contacting [bq-continuous-queries-feedback@google.com](mailto:bq-continuous-queries-feedback@google.com).
- You can't create a reservation assignment that uses a different [job type](https://docs.cloud.google.com/bigquery/docs/reservations-workload-management#assignments) in the same reservation as a continuous query reservation assignment.
- You can't configure continuous query concurrency. BigQuery automatically determines the number of continuous queries that can run concurrently, based on available reservation assignments that use the `CONTINUOUS` job type.
- When running multiple continuous queries using the same reservation, individual jobs might not split available resources fairly, as defined by [BigQuery fairness](https://docs.cloud.google.com/bigquery/docs/slots#fair_scheduling_in_bigquery).

## Slots autoscaling

Continuous queries can use
[slot autoscaling](https://docs.cloud.google.com/bigquery/docs/slots-autoscaling-intro) to dynamically scale
allocated capacity to accommodate your workload. As your continuous queries
workload increases or decreases, BigQuery dynamically adjusts
your slots.

After a continuous query starts running, it actively *listens* for incoming data,
which consumes slot resources. While a reservation with a running continuous
query does not scale down to zero slots, an idle continuous query that is
primarily listening for incoming data is expected to consume a minimal amount of
slots, typically around 1 slot.

## Idle slot sharing

Continuous queries can use [idle slot sharing](https://docs.cloud.google.com/bigquery/docs/slots#idle_slots)
to share unused slot resources with other reservations and
[job types](https://docs.cloud.google.com/bigquery/docs/reservations-workload-management#assignments).

- A `CONTINUOUS` [reservation assignment](https://docs.cloud.google.com/bigquery/docs/reservations-assignments) is still required to run a continuous query and can't solely rely on idle slots from other reservations. Thus a `CONTINUOUS` reservation assignment requires either a non-zero slot baseline or a non-zero slot autoscaling configuration.
- Only idle baseline slots or committed slots from a `CONTINUOUS` reservation assignment are shareable. [Autoscaled slots](https://docs.cloud.google.com/bigquery/docs/slots-autoscaling-intro) aren't shareable as idle slots for other reservations.

## Pricing

Continuous queries use
[BigQuery capacity compute pricing](https://cloud.google.com/bigquery/pricing#capacity_compute_analysis_pricing),
which is measured in [slots](https://docs.cloud.google.com/bigquery/docs/slots).
To run continuous queries, you must have a
[reservation](https://docs.cloud.google.com/bigquery/docs/reservations-workload-management) that uses the
[Enterprise or Enterprise Plus edition](https://docs.cloud.google.com/bigquery/docs/editions-intro),
and a [reservation assignment](https://docs.cloud.google.com/bigquery/docs/reservations-workload-management#assignments)
that uses the `CONTINUOUS` job type.

Usage of other BigQuery resources, such as data ingestion and
storage, are charged at the rates shown in
[BigQuery pricing](https://cloud.google.com/bigquery/pricing).

Usage of other services that receive continuous query results or that are called
during continuous query processing are charged at the rates published for those
services. For the pricing of other Google Cloud services used by continuous
queries, see the following topics:

- [Bigtable pricing](https://cloud.google.com/bigtable/pricing)
- [Pub/Sub pricing](https://cloud.google.com/pubsub/pricing)
- [Spanner pricing](https://cloud.google.com/spanner/pricing)
- [Vertex AI pricing](https://cloud.google.com/vertex-ai/pricing)

## What's next

Try [creating a continuous query](https://docs.cloud.google.com/bigquery/docs/continuous-queries).