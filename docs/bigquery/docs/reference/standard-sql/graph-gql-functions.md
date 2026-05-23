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

All GoogleSQL [functions](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/functions-all) are supported,
including the following GQL-specific functions:

## Function list

| Name | Summary |
|---|---|
| [`DESTINATION_NODE_ID`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/graph-gql-functions#destination_node_id) | Gets a unique identifier of a graph edge's destination node. |
| [`EDGES`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/graph-gql-functions#edges) | Gets the edges in a graph path. The resulting array retains the original order in the graph path. |
| [`ELEMENT_ID`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/graph-gql-functions#element_id) | Gets a graph element's unique identifier. |
| [`LABELS`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/graph-gql-functions#labels) | Gets the labels associated with a graph element. |
| [`NODES`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/graph-gql-functions#nodes) | Gets the nodes in a graph path. The resulting array retains the original order in the graph path. |
| [`PATH_FIRST`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/graph-gql-functions#path_first) | Gets the first node in a graph path. |
| [`PATH_LAST`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/graph-gql-functions#path_last) | Gets the last node in a graph path. |
| [`PATH_LENGTH`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/graph-gql-functions#path_length) | Gets the number of edges in a graph path. |
| [`SOURCE_NODE_ID`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/graph-gql-functions#source_node_id) | Gets a unique identifier of a graph edge's source node. |

## `DESTINATION_NODE_ID`

    DESTINATION_NODE_ID(edge_element)

**Description**

Gets a unique identifier of a graph edge's destination node. The unique identifier is only valid for the scope of the query where it's obtained.

**Definitions**

- `edge_element`: A `GRAPH_ELEMENT` value that represents an edge.

**Details**

Returns `NULL` if `edge_element` is `NULL`.

**Return type**

`STRING`

**Examples**

    GRAPH graph_db.FinGraph
    MATCH (:Person)-[o:Owns]->(a:Account)
    RETURN a.id AS account_id, DESTINATION_NODE_ID(o) AS destination_node_id

    /*---+
     |account_id | destination_node_id          |
     +---|---+
     | 7         | mUZpbkdyYXBoLkFjY291bnQAeJEO |
     | 16        | mUZpbkdyYXBoLkFjY291bnQAeJEg |
     | 20        | mUZpbkdyYXBoLkFjY291bnQAeJEo |
     +---*/

Note that the actual identifiers obtained may be different from what's shown above.

## `EDGES`

    EDGES(graph_path)

**Description**

Gets the edges in a graph path. The resulting array retains the
original order in the graph path.

**Definitions**

- `graph_path`: A `GRAPH_PATH` value that represents a graph path.

**Details**

If `graph_path` is `NULL`, returns `NULL`.

**Return type**

`ARRAY<GRAPH_ELEMENT>`

**Examples**

    GRAPH graph_db.FinGraph
    MATCH p=(src:Account)-[t1:Transfers]->(mid:Account)-[t2:Transfers]->(dst:Account)
    LET es = EDGES(p)
    RETURN TO_JSON(es) AS edges

## `ELEMENT_ID`

    ELEMENT_ID(element)

**Description**

Gets a graph element's unique identifier. The unique identifier is only valid for the scope of the query where it's obtained.

**Definitions**

- `element`: A `GRAPH_ELEMENT` value.

**Details**

Returns `NULL` if `element` is `NULL`.

**Return type**

`STRING`

**Examples**

    GRAPH graph_db.FinGraph
    MATCH (p:Person)-[o:Owns]->(:Account)
    RETURN p.name AS name, ELEMENT_ID(p) AS node_element_id, ELEMENT_ID(o) AS edge_element_id

    /*---+
     | name | node_element_id              | edge_element_id                                                                                      |
     +---|---|---+
     | Alex | mUZpbkdyYXBoLlBlcnNvbgB4kQI= | mUZpbkdyYXBoLlBlcnNvbk93bkFjY291bnQAeJECkQ6ZRmluR3JhcGguUGVyc29uAHiRAplGaW5HcmFwaC5BY2NvdW50AHiRDg== |
     | Dana | mUZpbkdyYXBoLlBlcnNvbgB4kQQ= | mUZpbkdyYXBoLlBlcnNvbk93bkFjY291bnQAeJEGkSCZRmluR3JhcGguUGVyc29uAHiRBplGaW5HcmFwaC5BY2NvdW50AHiRIA== |
     | Lee  | mUZpbkdyYXBoLlBlcnNvbgB4kQY= | mUZpbkdyYXBoLlBlcnNvbk93bkFjY291bnQAeJEEkSiZRmluR3JhcGguUGVyc29uAHiRBJlGaW5HcmFwaC5BY2NvdW50AHiRKA== |
     +---*/

Note that the actual identifiers obtained may be different from what's shown above.

## `LABELS`

    LABELS(element)

**Description**

Gets the labels associated with a graph element and preserves the original case
of each label.

**Definitions**

- `element`: A `GRAPH_ELEMENT` value that represents the graph element to extract labels from.

**Details**

Returns `NULL` if `element` is `NULL`.

**Return type**

`ARRAY<STRING>`

**Examples**

    GRAPH graph_db.FinGraph
    MATCH (n:Person|Account)
    RETURN LABELS(n) AS label, n.id

    /*---+
     | label     | id |
     +---+
     | [Account] | 7  |
     | [Account] | 16 |
     | [Account] | 20 |
     | [Person]  | 1  |
     | [Person]  | 2  |
     | [Person]  | 3  |
     +---*/

## `NODES`

    NODES(graph_path)

**Description**

Gets the nodes in a graph path. The resulting array retains the
original order in the graph path.

**Definitions**

- `graph_path`: A `GRAPH_PATH` value that represents a graph path.

**Details**

Returns `NULL` if `graph_path` is `NULL`.

**Return type**

`ARRAY<GRAPH_ELEMENT>`

**Examples**

    GRAPH graph_db.FinGraph
    MATCH p=(src:Account)-[t1:Transfers]->(mid:Account)-[t2:Transfers]->(dst:Account)
    LET ns = NODES(p)
    RETURN
      JSON_QUERY(TO_JSON(ns)[0], '$.labels') AS labels,
      JSON_QUERY(TO_JSON(ns)[0], '$.properties.nick_name') AS nick_name;

    /*---+
     | labels      | nick_name        |
     +---+
     | ["Account"] | "Vacation Fund"  |
     | ["Account"] | "Rainy Day Fund" |
     | ["Account"] | "Rainy Day Fund" |
     | ["Account"] | "Rainy Day Fund" |
     | ["Account"] | "Vacation Fund"  |
     | ["Account"] | "Vacation Fund"  |
     | ["Account"] | "Vacation Fund"  |
     | ["Account"] | "Rainy Day Fund" |
     +---*/

## `PATH_FIRST`

    PATH_FIRST(graph_path)

**Description**

Gets the first node in a graph path.

**Definitions**

- `graph_path`: A `GRAPH_PATH` value that represents the graph path to extract the first node from.

**Details**

Returns `NULL` if `graph_path` is `NULL`.

**Return type**

`GRAPH_ELEMENT`

**Examples**

    GRAPH graph_db.FinGraph
    MATCH p=(src:Account)-[t1:Transfers]->(mid:Account)-[t2:Transfers]->(dst:Account)
    LET f = PATH_FIRST(p)
    RETURN
      LABELS(f) AS labels,
      f.nick_name AS nick_name;

    /*---+
     | labels  | nick_name      |
     +---+
     | Account | Vacation Fund  |
     | Account | Rainy Day Fund |
     | Account | Rainy Day Fund |
     | Account | Vacation Fund  |
     | Account | Vacation Fund  |
     | Account | Vacation Fund  |
     | Account | Rainy Day Fund |
     +---*/

## `PATH_LAST`

    PATH_LAST(graph_path)

**Description**

Gets the last node in a graph path.

**Definitions**

- `graph_path`: A `GRAPH_PATH` value that represents the graph path to extract the last node from.

**Details**

Returns `NULL` if `graph_path` is `NULL`.

**Return type**

`GRAPH_ELEMENT`

**Examples**

    GRAPH graph_db.FinGraph
    MATCH p=(src:Account)-[t1:Transfers]->(mid:Account)-[t2:Transfers]->(dst:Account)
    LET f = PATH_LAST(p)
    RETURN
      LABELS(f) AS labels,
      f.nick_name AS nick_name;

    /*---+
     | labels  | nick_name      |
     +---+
     | Account | Vacation Fund  |
     | Account | Vacation Fund  |
     | Account | Vacation Fund  |
     | Account | Vacation Fund  |
     | Account | Rainy Day Fund |
     | Account | Rainy Day Fund |
     | Account | Rainy Day Fund |
     +---*/

## `PATH_LENGTH`

    PATH_LENGTH(graph_path)

**Description**

Gets the number of edges in a graph path.

**Definitions**

- `graph_path`: A `GRAPH_PATH` value that represents the graph path with the edges to count.

**Details**

Returns `NULL` if `graph_path` is `NULL`.

**Return type**

`INT64`

**Examples**

    GRAPH graph_db.FinGraph
    MATCH p=(src:Account)-[t1:Transfers]->(mid:Account)-[t2:Transfers]->(dst:Account)
    RETURN PATH_LENGTH(p) AS results

    /*---+
     | results |
     +---+
     | 2       |
     | 2       |
     | 2       |
     | 2       |
     | 2       |
     | 2       |
     | 2       |
     +---*/

## `SOURCE_NODE_ID`

    SOURCE_NODE_ID(edge_element)

**Description**

Gets a unique identifier of a graph edge's source node. The unique identifier is only valid for the scope of the query where it's obtained.

**Definitions**

- `edge_element`: A `GRAPH_ELEMENT` value that represents an edge.

**Details**

Returns `NULL` if `edge_element` is `NULL`.

**Return type**

`STRING`

**Examples**

    GRAPH graph_db.FinGraph
    MATCH (p:Person)-[o:Owns]->(:Account)
    RETURN p.name AS name, SOURCE_NODE_ID(o) AS source_node_id

    /*---+
     | name | source_node_id               |
     +---|---+
     | Alex | mUZpbkdyYXBoLlBlcnNvbgB4kQI= |
     | Dana | mUZpbkdyYXBoLlBlcnNvbgB4kQQ= |
     | Lee  | mUZpbkdyYXBoLlBlcnNvbgB4kQY= |
     +---*/

Note that the actual identifiers obtained may be different from what's shown above.

## Supplemental materials

### Horizontal aggregate function calls in GQL

In GQL, a horizontal aggregate function is an aggregate function that summarizes
the contents of exactly one array-typed value. Because a horizontal aggregate
function doesn't need to aggregate vertically across rows like a traditional
aggregate function, you can use it like a normal function expression.
Horizontal aggregates are only allowed in certain syntactic contexts: `LET`,
`FILTER` statements or `WHERE` clauses.

Horizontal aggregation is especially useful when paired with a
[group variable](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/graph-patterns#quantified_paths). You can create a group variable inside a
quantified path pattern in a linear graph query.

Some aggregates use an `ORDER BY` clause, such as the `ARRAY_AGG`,
`STRING_AGG`, and `ARRAY_CONCAT_AGG` functions. For these aggregates the
system orders inputs by their position in the array if you don't provide an
`ORDER BY` clause.

#### Syntactic restrictions

- The argument to the aggregate function must reference exactly one array-typed value.
- Can be used in `LET`, `FILTER` statements, or `WHERE` clauses only.
- Nesting horizontal aggregates isn't allowed.

#### Examples

In the following query, the `SUM` function horizontally aggregates over an
array (`arr`), and then produces the sum of the values in `arr`:

    GRAPH graph_db.FinGraph
    LET arr = [1, 2, 3]
    LET total = SUM(arr)
    RETURN total

    /*---+
     | total |
     +---+
     | 6     |
     +---*/

In the following query, the `SUM` function horizontally aggregates over an
array of structs (`arr`), and then produces the sum of the `x` fields in the
array:

    GRAPH graph_db.FinGraph
    LET arr = [STRUCT(1 as x, 10 as y), STRUCT(2, 9), STRUCT(3, 8)]
    LET total = SUM(arr.x)
    RETURN total

    /*---+
     | total |
     +---+
     | 6     |
     +---*/

In the following query, the `AVG` function horizontally aggregates over an
array of structs (`arr`), and then produces the average of the `x` and `y`
fields in the array:

    GRAPH graph_db.FinGraph
    LET arr = [STRUCT(1 as x, 10 as y), STRUCT(2, 9), STRUCT(3, 8)]
    LET avg_sum = AVG(arr.x + arr.y)
    RETURN avg_sum

    /*---+
     | avg_sum |
     +---+
     | 11      |
     +---*/

The `ARRAY_AGG` function can be used as a projection when horizontally
aggregating. The resulting array is in the same order as the array that's
horizontally aggregated over.

    GRAPH graph_db.FinGraph
    LET arr = [STRUCT(1 as x, 9 as y), STRUCT(2, 9), STRUCT(4, 8)]
    LET result = ARRAY_AGG(arr.x + arr.y)
    RETURN result

    /*---+
     | result       |
     +---+
     | [10, 11, 12] |
     +---*/

The following query produces an error because two arrays were passed into
the `AVG` aggregate function:

    -- ERROR: Horizontal aggregation on more than one array-typed variable
    -- isn't allowed
    GRAPH graph_db.FinGraph
    LET arr1 = [1, 2, 3]
    LET arr2 = [5, 4, 3]
    LET avg_val = AVG(arr1 + arr2)
    RETURN avg_val

The following query demonstrates a common pitfall. All instances of the array
that is horizontally aggregated over are treated as a single element from that
array in the aggregate.

To resolve this error, move expressions that use the entire array
outside of the horizontal aggregation.

    -- ERROR: No matching signature for function ARRAY_LENGTH for argument types: INT64
    GRAPH graph_db.FinGraph
    LET arr1 = [1, 2, 3]
    LET bad_avg_val = SUM(arr1 / ARRAY_LENGTH(arr1))
    RETURN bad_avg_val

The fix:

    GRAPH graph_db.FinGraph
    LET arr1 = [1, 2, 3]
    LET len = ARRAY_LENGTH(arr1)
    LET avg_val = SUM(arr1 / len)
    RETURN avg_val

In the following query, the `COUNT` function counts the unique amount
transfers with one to three hops between a source account (`src`) and a
destination account (`dst`):

    GRAPH graph_db.FinGraph
    MATCH (src:Account)-[e:Transfers]->{1, 3}(dst:Account)
    WHERE src != dst
    LET num_transfers = COUNT(e)
    LET unique_amount_transfers = COUNT(DISTINCT e.amount)
    FILTER unique_amount_transfers != num_transfers
    RETURN src.id as src_id, num_transfers, unique_amount_transfers, dst.id AS destination_account_id

    /*---+
     | src_id | num_transfers | unique_transfers_amount | destination_account_id |
     +---+
     | 7      | 3             | 2                       | 16                     |
     | 20     | 3             | 2                       | 16                     |
     | 7      | 2             | 1                       | 20                     |
     | 16     | 3             | 2                       | 20                     |
     +---*/

In the following query, the `SUM` function takes a group variable called `e`
that represents an array of transfers, and then sums the amount for each
transfer. Horizontal aggregation isn't allowed in the `RETURN`
statement. `ARRAY_AGG` is a vertical aggregate over the result set, which is
grouped implicitly by the non-aggregated columns
(`source_account_id`, `destination_account_id`). `ARRAY_AGG` produces one row
for each distinct destination account.

    GRAPH graph_db.FinGraph
    MATCH (src:Account {id: 7})-[e:Transfers]->{1,2}(dst:Account)
    LET total_amount = SUM(e.amount)
    RETURN
      src.id AS source_account_id, dst.id AS destination_account_id,
      ARRAY_AGG(total_amount) as total_amounts_per_path

    /*---+
     | source_account_id | destination_account_id | total_amounts_per_path |
     +---+
     | 7                 | 16                     | [300, 100]             |
     | 7                 | 20                     | [600, 400]             |
     +---*/