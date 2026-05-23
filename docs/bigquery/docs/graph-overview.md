# Introduction to BigQuery Graph

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
> **Note:** To request support or provide feedback for this feature, send email to [bq-graph-preview-support@google.com](mailto:bq-graph-preview-support@google.com).

> [!NOTE]
> **Note:** This feature may not be available when using reservations that are created with certain BigQuery editions. For more information about which features are enabled in each edition, see [Introduction to
> BigQuery editions](https://docs.cloud.google.com/bigquery/docs/editions-intro).

[Video](https://www.youtube.com/watch?v=_9YTyst9xWg)

BigQuery Graph lets you use the
analytical power of BigQuery to perform graph
analysis on a large scale. When you model your data as a graph with nodes and
edges, you can use
[Graph Query Language (GQL)](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/graph-intro)
to find complex, hidden
relationships between data points that would be challenging to find using SQL.

You can create node and edge tables directly from tables or views that store
entities and relationships between entities. You don't need to modify your
existing workflows or replicate your data to use it in graph queries.

BigQuery Graph supports a graph query interface compatible with
the [ISO GQL standard](https://www.iso.org/standard/76120.html) and the
[ISO Property Graph Queries (SQL/PGQ) standard](https://www.iso.org/standard/79473.html).
This provides you with interoperability between relational and graph models by
combining well-established SQL capabilities with the expressiveness of graph
pattern matching.

## Benefits of BigQuery Graph

Graphs are a natural way to represent relationships in data. Graph databases
are used for fraud detection, recommendations, community detection,
knowledge graphs, customer profiles, data cataloging, and lineage tracking.

When your graph data is represented as tables, you must perform self joins or
recursive joins to traverse your data. Expressing graph traversal logic in SQL
leads to complex queries that are difficult to write, maintain, and debug.
BigQuery Graph lets you navigate relationships and identify
patterns in your graph data in a more intuitive way.

## Key capabilities

- **Built-in graph experience**. The ISO GQL interface offers a familiar,
  purpose-built graph experience that's based on open standards.

- **Unified relational and graph**. Full interoperability between graph
  queries and SQL breaks down data silos and lets you choose the optimal tool
  for each use case, without any operational overhead to extract, transform,
  and load (ETL).

- **Built-in search capabilities**. Rich vector and full-text search
  capabilities integrate with graph, letting you use semantic meaning and
  keywords in graph analysis.

- **Graph visualization**. Graph query results are displayed in a
  visually appealing graph format that makes data exploration, investigation,
  and explanation much easier.

- **Performance and scalability**. Graph workloads are powered by
  BigQuery's scalable, cost-effective and distributed analytics
  engine.

- **Integration with Spanner Graph**. BigQuery Graph
  and Spanner Graph share the same graph schema and query language.
  You can execute operational graph workloads in Spanner and
  run complex graph analytics in BigQuery without needing to
  remodel your data or translate your queries.

- **Query using natural language** . Ask questions about your graph using
  [conversational analytics](https://docs.cloud.google.com/bigquery/docs/conversational-analytics#graphs).
  Agents can write SQL and GQL queries and provide visualizations of your
  output. Agents can also use descriptions, synonyms, and
  [measures](https://docs.cloud.google.com/bigquery/docs/graph-measures) defined on your graph to improve
  the quality of the results. To try querying an agent about a graph, chat with
  the
  [`Look Graph` agent](https://console.cloud.google.com/bigquery/agents_hub;agentsHubTab=Agents;agentsPath=%2Fbq%2Fagents%2Fagent_59ff15ce-e31b-4ba6-ac72-57788dfe0d48)
  about the
  [`bigquery-public-data.thelook_ecommerce.graph`](https://console.cloud.google.com/bigquery?ws=!1m5!1m4!18m3!1sbigquery-public-data!2sthelook_ecommerce!3sgraph) graph.

### Use cases

You can use BigQuery Graph to build many types of analytic
graph workloads, including the following:

- **Financial fraud detection** . Analyze complex relationships among
  users, accounts, and transactions to identify suspicious patterns and
  anomalies, such as money laundering and irregular connections between
  entities, which can be difficult to detect using relational databases.
  For a tutorial about fraud detection, see
  [Fraud detection with BigQuery Graph](https://codelabs.developers.google.com/codelabs/fraud-bigquery-graph).

- **Customer profiles** . Track customer relationships, preferences, and
  purchase histories. Gain a holistic understanding of each customer to enable
  personalized recommendations, targeted marketing campaigns, and improved
  customer service experiences. For a tutorial about customer profiles, see
  [Build customer 360 recommendations with BigQuery Graph](https://codelabs.developers.google.com/codelabs/c360-bigquery-graph).

- **Social networks**. Capture user activities and interactions and use
  graph pattern matching for friend recommendations and content discovery.

- **Manufacturing and supply chain management** . Use graph patterns for
  efficient impact analysis, cost rollups, and compliance checks by modeling
  parts, suppliers, orders, availability, and defects in the graph. For a
  tutorial about supply chains, see
  [Supply chain traceability with BigQuery Graph](https://codelabs.developers.google.com/codelabs/supplychaingraph).

- **Health care**. Capture patient relationships, conditions, diagnosis,
  and treatments to facilitate patient similarity analysis and treatment
  planning.

- **Transportation**. Model places, connections, distances, and costs in
  the graph, and then use graph queries to find the optimal route.

## Pricing

BigQuery Graph uses the standard BigQuery
capacity-based [pricing model](https://cloud.google.com/bigquery/pricing)
to ensure that you only pay for what you use across compute and storage.

### Compute

To use BigQuery Graph, you must have a
[reservation](https://docs.cloud.google.com/bigquery/docs/reservations-workload-management) that uses the
[Enterprise or Enterprise Plus edition](https://docs.cloud.google.com/bigquery/docs/editions-intro).
Graph queries use
[BigQuery capacity compute pricing](https://cloud.google.com/bigquery/pricing#capacity-compute-pricing)
measured in slots.

### Storage

You are charged only once for the storage of the underlying tables used to
define your graphs. Storage costs follow the standard
[BigQuery storage pricing](https://cloud.google.com/bigquery/pricing#storage-pricing)
(active or long-term storage), regardless of how many graph
models are built on top of those tables.

## What's next

- Learn how to [create and query a property graph](https://docs.cloud.google.com/bigquery/docs/graph-create).
- Learn about [graph schemas](https://docs.cloud.google.com/bigquery/docs/graph-schema-overview).
- Learn how to [write graph queries](https://docs.cloud.google.com/bigquery/docs/graph-query-overview).
- Learn how to [visualize graphs](https://docs.cloud.google.com/bigquery/docs/graph-visualization).
- Learn about the [differences between BigQuery Graph and Spanner Graph](https://docs.cloud.google.com/bigquery/docs/graph-compare).
- Learn about the [Graph Query Language (GQL)](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/graph-intro).