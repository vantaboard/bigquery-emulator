# Use BigQuery Graph and Spanner Graph

This document helps you determine which graph solution to use by comparing
[BigQuery Graph](https://docs.cloud.google.com/bigquery/docs/graph-overview) and
[Spanner Graph](https://docs.cloud.google.com/spanner/docs/graph/overview).

This comparison serves as a recommendation, and isn't intended to be absolute.
There is significant feature overlap between BigQuery Graph and
Spanner Graph, and users should identify their own workload needs
to determine which feature to use.

## When to use BigQuery Graph and Spanner Graph

BigQuery Graph and Spanner Graph serve different
types of graph workloads. BigQuery Graph is for deep analysis at
scale, while Spanner Graph is for real-time operations.

### BigQuery Graph

BigQuery Graph is optimized for
running complex queries on large graphs. You can analyze global
patterns, identify historical trends, and uncover hidden relationships in
massive datasets.

Its technical characteristics include optimized performance at scale,
parallel processing, complex queries over a significant part of the graph,
intensive compute, and complex aggregations.

Common use cases include the following:

- **Offline fraud detection:** Find other suspicious users that are connected to known fraudsters across your entire network in a customer graph, within a few degrees of connections sharing the same email, phone, or address. For a tutorial about fraud detection, see [Spanner \&
  BigQuery: Real-Time Fraud Defense
  Shield](https://codelabs.developers.google.com/spanner-bigquery-graph).
- **Supply chain optimization:** Build a graph of the bill of materials that represents relationships between final products and their components for inventory planning. Calculate product delivery dates and understand material availability by analyzing top-level products down to their root components across all product lines.
- **Customer 360 segmentation:** Build a customer 360 graph to understand customer journeys for product subscriptions, conversions, and churn. Use this to identify why customers churn, identify usage patterns, and use the graph for customer segmentation and audience targeting across the entire audience.

### Spanner Graph

Spanner Graph is for real-time operations. It is optimized for
applications that need to perform k-hop queries touching a small set of graph
elements instantly, detect fraud in milliseconds, perform lineage tracing for
identity and dependency verification, and serve live recommendations.

Its technical characteristics include predictable latency with minimal jitter,
queries per second (QPS) that scales linearly with the number of
Spanner nodes, and virtually unlimited scale. It also features built-in
graph storage with node and edge table interleaving, always-on availability,
global consistency, and point lookup and multi-hop queries starting from a
single graph node or a set of graph nodes.

Common use cases include the following:

- **Real-time fraud detection:** Check a credit card swipe against a graph of known fraudulent devices and accounts within milliseconds.
- **Autonomous network operations:** Build a digital twin of your network for real-time performance monitoring and optimization.
- **Entity resolution:** Build clusters of linked identities as a source of truth from different PIIs (email, phone, vemo id). Use the canonical profiles for identity lookup before serving ads, perform real-time fraud detection, and train feature stores.

## How BigQuery Graph and Spanner Graph work together

BigQuery Graph and Spanner Graph work together to
provide a comprehensive solution. For example, in a customer 360 use case:

1. **Real-time insights:** A customer service agent uses Spanner Graph to handle user complaints about shipping the wrong product based on real-time purchase and delivery statistics.
2. **Replicate or query:** You can replicate data from Spanner to BigQuery by using [Spanner Change Streams](https://docs.cloud.google.com/spanner/docs/change-streams) without complex extract, transform, and load (ETL), or query Spanner data directly from BigQuery by using [BigQuery federated queries](https://docs.cloud.google.com/bigquery/docs/federated-queries-intro).
3. **Analyze patterns:** A data scientist uses BigQuery Graph to identify "churn hotspot" on that same data to label the customer as "at-risk".
4. **Feedback loop:** The "at-risk" labels are pushed to Spanner Graph with reverse extract, transform, and load (ETL) support to generate a coupon code for this customer to prevent churn.

### Move data between BigQuery Graph and Spanner Graph

You can move data between Spanner Graph and
BigQuery Graph to suit your workload requirements:

- **Forward ETL:** To move data from Spanner to
  BigQuery for analytical queries, use a [Dataflow
  template](https://docs.cloud.google.com/dataflow/docs/guides/templates/provided/cloud-spanner-to-bigquery).

- **Reverse ETL:** While you can query Spanner data
  directly from BigQuery Graph, there might be scenarios
  where you need to bring BigQuery data into
  Spanner Graph. Use the [`EXPORT
  DATA`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/export-statements#export_data_statement)
  SQL statement. For more details, see [Understand the reverse ETL
  pipeline](https://docs.cloud.google.com/spanner/docs/graph/reverse-etl#reverse-etl-pipeline).

## Feature comparison

The following table provides a detailed breakdown of features by the product
they are most optimized for:

| Feature | BigQuery Graph (offline/batch) | Spanner Graph (online/real-time) |
|---|---|---|
| **Graph Model, Query, and Visualization** | Unified graph modeling and graph query language, all powered by GoogleSQL, part of ISO SQL standard. Same interface for graph visualization. | Unified graph modeling and graph query language, all powered by GoogleSQL, part of ISO SQL standard. Same interface for graph visualization. |
| **Primary Workload** | Offline (batch): aggregations over massive datasets. | Online (real-time): high volume of low latency reads/writes. |
| **Query Latency** | Seconds to hours. Optimized for scanning terabytes/petabytes. | Milliseconds to seconds. Critical for user-facing apps. |
| **Query pattern** | Global or whole graph: "Who has a 'Network Reach' of more than 5,000 people?" | Local or neighborhood: "Who are friends with my friends, but aren't in my immediate circle?" |
| **Scale** | Petabyte-scale, virtually limitless, optimized for large-scale historical data. | Horizontally scalable, virtually unlimited scale, optimized for hot data. |
| **Data freshness** | Near real-time or batch. Can access data from different sources (for example, Spanner Graph data using [Data Boost for Spanner](https://docs.cloud.google.com/spanner/docs/databoost/databoost-overview), [Bigtable](https://docs.cloud.google.com/bigtable/docs/overview), [Cloud Storage](https://docs.cloud.google.com/bigquery/docs/external-data-cloud-storage), or [Amazon S3](https://docs.cloud.google.com/bigquery/docs/omni-aws-create-connection)). | Real-time, strong consistency. |
| **Data Input** | Data lakes, historical logs, archived transaction data. | Live application streams, user interactions. |
| **Data Movement** | Spanner to BigQuery (forward extract, transform, and load (ETL)) with prebuilt templates (batch, streaming). BigQuery to Spanner ([reverse ETL](https://docs.cloud.google.com/bigquery/docs/export-to-spanner)) with [`EXPORT_DATA`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/export-statements#export_data_statement). [Query federation](https://docs.cloud.google.com/bigquery/docs/federated-queries-intro) (zero-ETL) with external schema. | Spanner to BigQuery (forward extract, transform, and load (ETL)) with prebuilt templates (batch, streaming). BigQuery to Spanner ([reverse ETL](https://docs.cloud.google.com/bigquery/docs/export-to-spanner)) with [`EXPORT_DATA`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/export-statements#export_data_statement). [Query federation](https://docs.cloud.google.com/bigquery/docs/federated-queries-intro) (zero-ETL) with external schema. |

## What's next

- Learn more about [BigQuery Graph](https://docs.cloud.google.com/bigquery/docs/graph-overview).
- Learn more about [Spanner Graph](https://docs.cloud.google.com/spanner/docs/graph/overview).