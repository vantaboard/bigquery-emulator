# Graph query best practices

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

This document describes best practices for optimizing your
BigQuery Graph queries.

## Start path traversal from low-cardinality nodes

To keep intermediate result sets small and speed up query execution, write your
graph queries so that path traversal starts from lower cardinality nodes,
regardless of the direction of path traversal. The following
`MATCH` statements use property filters to reduce the number of possible
starting nodes instead of computing all matches and then filtering:

    MATCH (p:Person {id: 10})-[own:Owns]->(a:Account)

    MATCH (a:Account WHERE balance > 10)<-[own:Owns]-(p:Person)

This is especially important for quantified path queries:

    MATCH (p:Person {id: 10})-[own:Owns]->{1,3}(a:Account)

## Use `ANY` or `ANY SHORTEST` syntax for connectivity checks

Quantified path queries can return duplicate paths between source nodes and
destination nodes. If your goal is to check for connectivity and you don't
require all possible paths, use `ANY` or `ANY SHORTEST` to reduce redundant
computations and improve path lookup efficiency. For example, the following
`MATCH` statement uses `ANY SHORTEST` to keep only one path between each pair
of nodes:

    MATCH ANY SHORTEST (a1:Account)-[t:Transfers]->{1,3}(a2:Account)

## Use directional path traversal

BigQuery Graph schemas are directional, which means that each
edge has a source node and a destination node. Although graph query
syntax allows path traversal in any direction (for example, `-[edge]-`), we
recommend using directional path traversal (for example, `-[edge]->` or
`<-[edge]-`) for better performance. Any direction path traversal might cause
performance loss.

The following `MATCH` statement uses any direction path traversal:

    -- Avoid.
    MATCH (a1:Account {id: 7})-[t:Transfers]-(a2:Account)

Instead, combine two directional traversals with `UNION ALL`:

    MATCH (a1:Account {id: 7})-[t:Transfers]->(a2:Account)
    ...
    UNION ALL
    ...
    MATCH (a1:Account  {id: 7})<-[t:Transfers]-(a2:Account)

## Specify labels explicitly

If node or edge labels are omitted in a query, BigQuery Graph
enumerates all qualifying node and edge labels. This enumeration might cause
more labels to be scanned than necessary. To avoid this, specify labels for
all nodes and edges in your query whenever possible.

For example, the following query specifies the `Account` and `Transfers` labels:

    GRAPH graph_db.FinGraph
    MATCH (a1:Account)-[t:Transfers]->(a2:Account)
    RETURN COUNT(*) AS num_transfers;

Avoid omitting labels, because it might scan other unneeded relationships
between nodes. In the following query, `a1` can represent an account or a
person, and `t` can represent a transfer or an account ownership.

    GRAPH graph_db.FinGraph
    MATCH (a1)-[t]->(a2)
    RETURN COUNT(*) AS num_transfers;

## Prefer a single `MATCH` statement

BigQuery Graph lets you include multiple `MATCH` statements in
a single graph query. These statements are connected by multi-declared variables
that represent the same node or edge. However, using multiple `MATCH`
statements can diminish cardinality benefits across statements. When
possible, use a single `MATCH` statement for better performance.

For example, the following queries are equivalent, but the first one performs
better because it uses a single `MATCH` statement:

    -- Preferred syntax.
    GRAPH graph_db.FinGraph
    MATCH
      (p:Person {id: 1})-[o:Owns]->
      (a:Account)-[t:Transfers]->(a2:Account)
    RETURN o.account_id, t.amount;

    -- Avoid this syntax.
    GRAPH graph_db.FinGraph
    MATCH (p:Person {id: 1})-[o:Owns]->(a:Account)
    MATCH (a:Account)-[t:Transfers]->(a2:Account)
    RETURN o.account_id, t.amount;

## Limit traversed edges from high-cardinality nodes

When you query graphs, some nodes can have a significantly larger number of
incoming or outgoing edges compared to other nodes. These high-cardinality
nodes are sometimes called *super nodes* or *hub nodes*. Super nodes can cause
performance issues because traversals through them might involve processing
large amounts of data, which leads to data skew and long execution times.

To optimize a query of a graph with super nodes, use the
[`ROW_NUMBER()` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/numbering_functions#row_number)
within a `FILTER` clause or `WHERE` clause in `MATCH` to limit the
number of edges that the query traverses from or to a node. This technique is
particularly useful when you don't need a complete enumeration of all
connections from or to a super node.

For example, if some accounts in `FinGraph` have a large number of
transactions, you can use `ROW_NUMBER()` to limit the number of `Transfers`
edges to consider for each `Account` and avoid an inefficient query:

    GRAPH graph_db.FinGraph
    MATCH (a1:Account)-[e1:Transfers WHERE e1 IN {
      GRAPH graph_db.FinGraph
      -- Sample 5 edges per source node
      MATCH -[selected_e:Transfers]->
        FILTER ROW_NUMBER() OVER (
          PARTITION BY SOURCE_NODE_ID(selected_e)) < 5
        RETURN selected_e
    }]->{1,3}(a2:Account)
    RETURN COUNT(*) AS cnt;

## Sample intermediate nodes or edges in multi-hop queries

You can also improve query efficiency by using `ROW_NUMBER()` to sample
intermediate nodes in multi-hop queries. This technique improves efficiency by
limiting the number of paths that the query considers for each intermediate
node. To do this, break a multi-hop query into multiple `MATCH` statements
separated by `NEXT`, and apply `ROW_NUMBER()` at the midpoint where you need to
sample:

    GRAPH graph_db.FinGraph
    MATCH (a1:Account)-[e1:Transfers]->(a2:Account)
    -- Sample 5 destination nodes per a1 source node
    FILTER ROW_NUMBER() OVER (PARTITION BY ELEMENT_ID(a1)) < 5
    RETURN a1, a2
    NEXT
    MATCH (a2)-[e2:Transfers]->(a3:Account)
    RETURN a1.id AS src_id, a2.id AS mid_id, a3.id AS dst_id;

## What's next

- To learn more about writing graph queries, see the [query overview](https://docs.cloud.google.com/bigquery/docs/graph-query-overview).
- Learn more about [schema best practices](https://docs.cloud.google.com/bigquery/docs/graph-schema-best-practices).