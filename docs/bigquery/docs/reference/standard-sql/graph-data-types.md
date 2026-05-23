> [!WARNING]
> **Preview**
>
>
> This product or feature is subject to the "Pre-GA Offerings Terms"
> in the General Service Terms section of the
> [Service Specific Terms](https://cloud.google.com/terms/service-terms).
> Pre-GA products and features are available "as is" and might have
> limited support. For more information, see the
> [launch stage descriptions](https://cloud.google.com/products#product-launch-stages).

> [!NOTE]
> **Note:** To provide feedback or request support for this feature, send an email to [bq-graph-preview-support@google.com](mailto:bq-graph-preview-support@google.com).

Graph Query Language (GQL) supports all GoogleSQL [data types](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types),
including the following GQL-specific data type:

## Graph data types list

| Name | Summary |
|---|---|
| [Graph element type](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/graph-data-types#graph_element_type) | An element in a property graph. Can be a `GRAPH_NODE` or `GRAPH_EDGE`. SQL type name: `GRAPH_ELEMENT` |
| [Graph path type](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/graph-data-types#graph_path_type) | A path in a property graph. SQL type name: `GRAPH_PATH` |

## Graph element type

| Name | Description |
|---|---|
| `GRAPH_ELEMENT` | An element in a property graph. |

A variable with a `GRAPH_ELEMENT` type is produced by a graph query.
The generated type has this format:

    GRAPH_ELEMENT<T>

A graph element is either a node or an edge, representing data from a
matching node or edge table based on its label. Each graph element holds a
set of properties that can be accessed with a case-insensitive name,
similar to fields of a struct.

**Example**

In the following example, `n` represents a graph element in the
[`FinGraph`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/graph-schema-statements#fin_graph) property graph:

    GRAPH graph_db.FinGraph
    MATCH (n:Person)
    RETURN n.name

In the following example, the [`TYPEOF`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/utility-functions#typeof) function is used to inspect the
set of properties defined in the graph element type.

    GRAPH graph_db.FinGraph
    MATCH (n:Person)
    RETURN TYPEOF(n) AS t
    LIMIT 1

    /*---+
     | t                                                      |
     +---+
     | GRAPH_NODE(myproject.graph_db.FinGraph)<id INT64, ...> |
     +---*/

## Graph path type

| Name | Description |
|---|---|
| `GRAPH_PATH` | A path in a property graph. |

The graph path data type represents a sequence of nodes interleaved
with edges and has this format:

    GRAPH_PATH<NODE_TYPE, EDGE_TYPE>