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

The following subqueries are supported in GQL query statements:

## Subquery list

| Name | Summary |
|---|---|
| [`ARRAY` subquery](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/graph-subqueries#array_subquery) | Subquery expression that produces an array. |
| [`EXISTS` subquery](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/graph-subqueries#exists_subquery) | Checks if a subquery produces at least one row. |
| [`IN` subquery](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/graph-subqueries#in_subquery) | Checks if a subquery produces a specified value. |
| [`VALUE` subquery](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/graph-subqueries#array_subquery) | Subquery expression that produces a scalar value. |

## `ARRAY` subquery

```
ARRAY { GRAPH graph_name gql_query_expr }
```

#### Description

Subquery expression that produces an array. If the subquery produces zero rows,
an empty array is produced. Never produces a `NULL` array. This can be used
wherever a query expression is supported in a GQL query statement.

#### Definitions

- `graph_name`: The name of the property graph.
- `gql_query_expr`: A GQL query expression.

#### Return type

`ARRAY<T>`

#### Examples

> [!NOTE]
> **Note:** The examples in this section reference a property graph called [`FinGraph`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/graph-schema-statements#fin_graph).

In the following query, an array of transfer amounts is produced for each
`Account` owned by each `Person` node:

    GRAPH graph_db.FinGraph
    MATCH (p:Person)-[:Owns]->(account:Account)
    RETURN
     p.name, account.id AS account_id,
     ARRAY {
       GRAPH graph_db.FinGraph
       MATCH (a:Account)-[transfer:Transfers]->(:Account)
       WHERE a = account
       RETURN transfer.amount AS transfers
     } AS transfers;

    /*---+
     | name | account_id | transfers |
     +---+---+
     | Alex | 7          | [300,100] |
     | Dana | 20         | [500,200] |
     | Lee  | 16         | [300]     |
     +---*/

## `EXISTS` subquery

```
EXISTS { GRAPH graph_name gql_query_expr }
```

```
EXISTS { match_statement }
```

```
EXISTS { graph_pattern }
```

#### Description

Checks if the subquery produces at least one row. Returns `TRUE` if
at least one row is produced, otherwise returns `FALSE`. Never produces `NULL`. You can't use an `EXISTS` subquery in the `WHERE`
clause of a path pattern. Instead, use a `FILTER` statement.

#### Definitions

- `graph_name`: The name of the property graph.
- `gql_query_expr`: A GQL query expression.
- `match_statement`: A pattern matching operation to perform on a graph. For more information, see [`MATCH` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/graph-query-statements#gql_match).
- `graph_pattern`: A pattern to match in a graph. For more information, see [graph pattern definition](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/graph-patterns#graph_pattern_definition).

#### Return type

`BOOL`

#### Examples

> [!NOTE]
> **Note:** The examples in this section reference a property graph called [`FinGraph`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/graph-schema-statements#fin_graph).

The following query checks whether any person named `"Lee"` owns an
account. The subquery contains a graph query expression.

    GRAPH graph_db.FinGraph
    RETURN EXISTS {
      GRAPH graph_db.FinGraph
      MATCH (p:Person {Name: "Lee"})-[o:Owns]->(a:Account)
      RETURN p.Name
      LIMIT 1
    } AS results;

    /*---+
     | results |
     +---+
     | true    |
     +---*/

You can include a `MATCH` statement or a graph pattern in an `EXISTS`
subquery. The following examples include two ways to construct the subquery
and produce similar results:

    GRAPH graph_db.FinGraph
    RETURN EXISTS {
      MATCH (p:Person {Name: "Lee"})-[o:Owns]->(a:Account)
    } AS results;

    /*---+
     | results |
     +---+
     | true    |
     +---*/

    GRAPH graph_db.FinGraph
    RETURN EXISTS {
      (p:Person {Name: "Lee"})-[o:Owns]->(a:Account)
    } AS results;

    /*---+
     | results |
     +---+
     | true    |
     +---*/

## `IN` subquery

```
value [ NOT ] IN { GRAPH graph_name gql_query_expr }
```

#### Description

Checks if `value` is present in the subquery result. Returns `TRUE` if the
result contains the `value`, otherwise returns `FALSE`.

#### Definitions

- `graph_name`: The name of the property graph.
- `value`: The value look for in the subquery result.
- `IN`: `TRUE` if the value is in the subquery result, otherwise `FALSE`.
- `NOT IN`: `FALSE` if the value is in the subquery result, otherwise `TRUE`.
- `gql_query_expr`: A GQL query expression.

#### Details

The subquery result must have a single column and that column type must
be comparable to the `value` type. If not, an error is
returned. You can't use an `IN` subquery in the `WHERE`
clause of a path pattern. Instead, use a `FILTER` statement.

#### Return type

`BOOL`

#### Examples

> [!NOTE]
> **Note:** The examples in this section reference a property graph called [`FinGraph`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/graph-schema-statements#fin_graph).

The following query checks if `'Dana'` is a name of a person who owns an
account.

    GRAPH graph_db.FinGraph
    RETURN 'Dana' IN {
      GRAPH graph_db.FinGraph
      MATCH (p:Person)-[o:Owns]->(a:Account)
      RETURN p.name
    } AS results;

    /*---+
     | results |
     +---+
     | true    |
     +---*/

## `VALUE` subquery

```
VALUE { GRAPH graph_name gql_query_expr }
```

#### Description

A subquery expression that produces a scalar value.

#### Definitions

- `graph_name`: The name of the property graph.
- `gql_query_expr`: A GQL query expression.

#### Details

The result of the subquery must have a single column. If the subquery returns
more than one column, the query fails with an analysis error. The result type of
the subquery expression is the produced column type. If the subquery produces
exactly one row, that single value is the subquery expression result. If the
subquery returns zero rows, the subquery expression result is `NULL`. If the
subquery returns more than one row, the query fails with a runtime error.

#### Return type

The same as the column type in the subquery result.

#### Examples

> [!NOTE]
> **Note:** The examples in this section reference a property graph called [`FinGraph`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/graph-schema-statements#fin_graph).

The following query returns the name of any person whose `country` property
is `"Australia"`:

    GRAPH graph_db.FinGraph
    RETURN VALUE {
      GRAPH graph_db.FinGraph
      MATCH (p:Person {country: "Australia"})
      RETURN p.name
      LIMIT 1
    } AS results;

    /*---+
     | results |
     +---+
     | Alex    |
     +---*/