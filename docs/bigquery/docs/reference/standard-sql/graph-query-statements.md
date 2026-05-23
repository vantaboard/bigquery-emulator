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

Graph Query Language (GQL) lets you execute multiple linear
graph queries in one query. Each linear graph query generates results
(the working table) and then passes those results to the next.

GQL supports the following building blocks, which can be composed into a
GQL query based on the [syntax rules](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/graph-query-statements#gql_syntax).

## Language list

| Name | Summary |
|---|---|
| [GQL syntax](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/graph-query-statements#gql_syntax) | Creates a graph query with the GQL syntax. |
| [`GRAPH` clause](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/graph-query-statements#graph_query) | Specifies a property graph to query. |
| [`FILTER` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/graph-query-statements#gql_filter) | Filters out rows in the query results that don't satisfy a specified condition. |
| [`FOR` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/graph-query-statements#gql_for) | Unnests an `ARRAY`-typed expression. |
| [`LET` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/graph-query-statements#gql_let) | Defines variables and assigns values for later use in the current linear query statement. |
| [`LIMIT` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/graph-query-statements#gql_limit) | Limits the number of query results. |
| [`MATCH` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/graph-query-statements#gql_match) | Matches data described by a graph pattern. |
| [`NEXT` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/graph-query-statements#gql_next) | Chains multiple linear query statements together. |
| [`OFFSET` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/graph-query-statements#gql_offset) | Skips a specified number of rows in the query results. |
| [`ORDER BY` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/graph-query-statements#gql_order_by) | Orders the query results. |
| [`RETURN` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/graph-query-statements#gql_return) | Marks the end of a linear query statement and returns the results. |
| [`SKIP` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/graph-query-statements#gql_skip) | Synonym for the `OFFSET` statement. |
| [`WITH` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/graph-query-statements#gql_with) | Passes on the specified columns, optionally filtering, renaming, and transforming those results. |
| [Set operation](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/graph-query-statements#gql_set) | Combines a sequence of linear query statements with a set operation. |

## GQL syntax

```
graph_query:
  GRAPH clause
  multi_linear_query_statement

multi_linear_query_statement:
  linear_query_statement
  [
    NEXT
    linear_query_statement
  ]
  [...]

linear_query_statement:
  {
    simple_linear_query_statement
    | composite_linear_query_statement
  }

composite_linear_query_statement:
  simple_linear_query_statement
  set_operator simple_linear_query_statement
  [...]

simple_linear_query_statement:
  primitive_query_statement
  [...]
```

#### Description

Creates a graph query with the GQL syntax. The syntax rules define how
to composite the building blocks of GQL into a query.

#### Definitions

- `primitive_query_statement`: A statement in [Query statements](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/graph-query-statements#language_list) except for the `NEXT` statement.
- `simple_linear_query_statement`: A list of `primitive_query_statement`s that ends with a [`RETURN` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/graph-query-statements#gql_return).
- `composite_linear_query_statement`: A list of `simple_linear_query_statement`s composited with the [set operators](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#set_operators).
- `linear_query_statement`: A statement that's either a `simple_linear_query_statement` or a `composite_linear_query_statement`.
- `multi_linear_query_statement`: A list of `linear_query_statement`s chained together with the [`NEXT` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/graph-query-statements#gql_next).
- `graph_query`: A GQL query that starts with a [`GRAPH` clause](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/graph-query-statements#graph_query), then follows with a `multi_linear_query_statement`.

## `GRAPH` clause

```
GRAPH property_graph_name
multi_linear_query_statement
```

#### Description

Specifies a property graph to query. This clause must be added before the first
linear query statement in a graph query.

#### Definitions

- `property_graph_name`: The name of the property graph to query.
- `multi_linear_query_statement`: A multi linear query statement. For more information, see `multi_linear_query_statement` in [GQL syntax](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/graph-query-statements#gql_syntax).

#### Examples

The following example queries the [`FinGraph`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/graph-schema-statements#fin_graph) property graph to find
accounts with incoming transfers and looks up their owners:

    GRAPH graph_db.FinGraph
    MATCH (:Account)-[:Transfers]->(account:Account)
    RETURN account, COUNT(*) AS num_incoming_transfers
    GROUP BY account

    NEXT

    MATCH (account:Account)<-[:Owns]-(owner:Person)
    RETURN
      account.id AS account_id, owner.name AS owner_name,
      num_incoming_transfers

    /*---+
     | account_id | owner_name | num_incoming_transfers |
     +---+
     | 7          | Alex       | 1                      |
     | 20         | Dana       | 1                      |
     | 6          | Lee        | 3                      |
     +---*/

## `FILTER` statement

```
FILTER [ WHERE ] bool_expression
```

#### Description

Filters out rows in the query results that don't satisfy a specified condition.

#### Definitions

- `bool_expression`: A boolean expression. Only rows whose `bool_expression` evaluates to `TRUE` are included. Rows whose `bool_expression` evaluates to `NULL` or `FALSE` are discarded.

#### Details

The `FILTER` statement can reference columns in the working table.

The syntax for the `FILTER` statement is similar to the syntax for the
[graph pattern `WHERE` clause](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/graph-patterns#graph_pattern_definition), but they are evaluated
differently. The `FILTER` statement is evaluated after the previous statement.
The `WHERE` clause is evaluated as part of the containing statement.

#### Examples

> [!NOTE]
> **Note:** The examples in this section reference a property graph called [`FinGraph`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/graph-schema-statements#fin_graph).

In the following query, people with `Id = 1` are excluded from the
results table:

    GRAPH graph_db.FinGraph
    MATCH (p:Person)-[o:Owns]->(a:Account)
    FILTER p.Id <> 1
    RETURN p.name, a.Id AS account_id

    /*---+---+
     | name   | account_id |
     +---+---+
     | "Dana" | 20         |
     | "Lee"  | 16         |
     +---+---*/

`WHERE` is an optional keyword that you can include in a `FILTER` statement.
The following query is semantically identical to the previous query:

    GRAPH graph_db.FinGraph
    MATCH (p:Person)-[o:Owns]->(a:Account)
    FILTER WHERE p.Id <> 1
    RETURN p.name, a.Id AS account_id

    /*---+---+
     | name   | account_id |
     +---+---+
     | "Dana" |         20 |
     | "Lee"  |         16 |
     +---+---*/

In the following example, `FILTER` follows an aggregation step with
grouping. Semantically, it's similar to the `HAVING` clause in SQL:

    GRAPH graph_db.FinGraph
    MATCH (source:Account)-[e:Transfers]->(dest:Account)
    RETURN source, COUNT(e) AS num_transfers
    GROUP BY source

    NEXT

    FILTER WHERE num_transfers > 1
    RETURN source.id AS source_id, num_transfers

    /*---+
     | source_id | num_transfers |
     +---+
     | 7         | 2             |
     | 20        | 2             |
     +---*/

In the following example, an error is produced because `FILTER` references
`m`, which isn't in the working table:

    -- Error: m doesn't exist
    GRAPH graph_db.FinGraph
    MATCH (p:Person)-[o:Owns]->(a:Account)
    FILTER WHERE m.Id <> 1
    RETURN p.name

In the following example, an error is produced because even though `p` is in the
working table, `p` doesn't have a property called `date_of_birth`:

    -- ERROR: date_of_birth isn't a property of p
    GRAPH graph_db.FinGraph
    MATCH (p:Person)-[o:Owns]->(a:Account)
    FILTER WHERE p.date_of_birth < '1990-01-10'
    RETURN p.name

## `FOR` statement

```
FOR element_name IN array_expression
  [ with_offset_clause ]

with_offset_clause:
  WITH OFFSET [ AS offset_name ]
```

#### Description

Unnests an `ARRAY`-typed expression and joins the result with the current working table.

#### Definitions

- `array_expression`: An `ARRAY`-typed expression.
- `element_name`: The name of the element column. The name can't be the name of a column that already exists in the current linear query statement.
- `offset_name`: The name of the offset column. The name can't be the name of a column that already exists in the current linear query statement. If not specified, the default is `offset`.

#### Details

The `FOR` statement expands the working table by defining a new column for the
elements of `array_expression`, with an optional offset column. The cardinality
of the working table might change as a result.

- The `FOR` statement can reference columns in the working table.
- The `FOR` statement evaluation is similar to the [`UNNEST`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#unnest_operator) operator.
- The `FOR` statement doesn't preserve order.
- An empty or `NULL` `array_expression` produces zero rows.

The keyword `WITH` following the `FOR` statement is always interpreted as the
beginning of `with_offset_clause`. If you want to use the `WITH` statement
following the `FOR` statement, you should fully qualify the `FOR` statement with
`with_offset_clause`, or use the `RETURN` statement instead of the `WITH`
statement.

#### Examples

> [!NOTE]
> **Note:** The examples in this section reference a property graph called [`FinGraph`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/graph-schema-statements#fin_graph).

In the following query, there are three rows in the working table prior to the
`FOR` statement. After the `FOR` statement, each row is expanded into two rows,
one per `element` value from the array.

    GRAPH graph_db.FinGraph
    MATCH (p:Person)-[o:Owns]->(a:Account)
    FOR element in ["all","some"] WITH OFFSET
    RETURN p.Id, element as alert_type, offset
    ORDER BY p.Id, element, offset

    /*---+
     | Id   | alert_type | offset |
     +---+
     | 1    | all        | 0      |
     | 1    | some       | 1      |
     | 2    | all        | 0      |
     | 2    | some       | 1      |
     | 3    | all        | 0      |
     | 3    | some       | 1      |
     +---*/

In the following query, there are two rows in the working table prior to the
`FOR` statement. After the `FOR` statement, each row is expanded into a
different number of rows, based on the value of `array_expression` for that row.

    GRAPH graph_db.FinGraph
    MATCH (p:Person)-[o:Owns]->(a:Account)
    FILTER WHERE p.Id <> 1
    FOR element in GENERATE_ARRAY(1, p.Id)
    RETURN p.Id, element
    ORDER BY p.Id, element

    /*---+
     | Id   | element |
     +---+
     | 2    | 1       |
     | 2    | 2       |
     | 3    | 1       |
     | 3    | 2       |
     | 3    | 3       |
     +---*/

In the following query, there are three rows in the working table prior to the
`FOR` statement. After the `FOR` statement, no row is produced because
`array_expression` is an empty array.

    -- No rows produced
    GRAPH graph_db.FinGraph
    MATCH (p:Person)
    FOR element in [] WITH OFFSET AS off
    RETURN p.name, element, off

In the following query, there are three rows in the working table prior to the
`FOR` statement. After the `FOR` statement, no row is produced because
`array_expression` is a `NULL` array.

    -- No rows produced
    GRAPH graph_db.FinGraph
    MATCH (p:Person)
    FOR element in CAST(NULL AS ARRAY<STRING>) WITH OFFSET
    RETURN p.name, element, offset

In the following example, an error is produced because `WITH` is used directly
After the `FOR` statement. The query can be fixed by adding `WITH OFFSET` after
the `FOR` statement, or by using `RETURN` directly instead of `WITH`.

    -- Error: Expected keyword OFFSET but got identifier "element"
    GRAPH graph_db.FinGraph
    FOR element in [1,2,3]
    WITH element as col
    RETURN col
    ORDER BY col

    GRAPH graph_db.FinGraph
    FOR element in [1,2,3] WITH OFFSET
    WITH element as col
    RETURN col
    ORDER BY col

    /*---+
     | col |
     +---+
     | 1   |
     | 2   |
     | 3   |
     +---*/

## `LET` statement

```
LET linear_graph_variable[, ...]

linear_graph_variable:
  variable_name = value
```

#### Description

Defines variables and assigns values to them for later use in the current
linear query statement.

#### Definitions

- `linear_graph_variable`: The variable to define.
- `variable_name`: The name of the variable.
- `value`: A scalar expression that represents the value of the variable. The names referenced by this expression must be in the incoming working table.

#### Details

`LET` doesn't change the cardinality of the working table nor modify its
existing columns.

- The variable can only be used in the current linear query statement. To use it in a following linear query statement, you must include it in the `RETURN` statement as a column.
- You can't define and reference a variable within the same `LET` statement.
- You can't redefine a variable with the same name.
- You can use horizontal aggregate functions in this statement. To learn more, see [Horizontal aggregate function calls in GQL](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/graph-gql-functions#gql-horiz-agg-func-calls).

#### Examples

> [!NOTE]
> **Note:** The examples in this section reference a property graph called [`FinGraph`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/graph-schema-statements#fin_graph).

In the following graph query, the variable `a` is defined and then referenced
later:

    GRAPH graph_db.FinGraph
    MATCH (source:Account)-[e:Transfers]->(destination:Account)
    LET a = source
    RETURN a.id AS a_id

    /*---+
     | a_id |
     +---+
     | 20   |
     | 7    |
     | 7    |
     | 20   |
     | 16   |
     +---*/

The following `LET` statement in the second linear query statement is valid
because `a` is defined and returned from the first linear query statement:

    GRAPH graph_db.FinGraph
    MATCH (source:Account)-[e:Transfers]->(destination:Account)
    LET a = source
    RETURN a

    NEXT

    LET b = a -- Valid: 'a' is defined and returned from the linear query statement above.
    RETURN b.id AS b_id

    /*---+
     | b_id |
     +---+
     | 20   |
     | 7    |
     | 7    |
     | 20   |
     | 16   |
     +---*/

The following `LET` statement in the second linear query statement is invalid
because `a` isn't returned from the first linear query statement.

    GRAPH graph_db.FinGraph
    MATCH (source:Account)-[e:Transfers]->(destination:Account)
    LET a = source
    RETURN source.id

    NEXT

    LET b = a  -- ERROR: 'a' doesn't exist.
    RETURN b.id AS b_id

The following `LET` statement is invalid because `a` is defined and then
referenced in the same `LET` statement:

    GRAPH graph_db.FinGraph
    MATCH (source:Account)-[e:Transfers]->(destination:Account)
    LET a = source, b = a -- ERROR: Can't define and reference 'a' in the same operation.
    RETURN a

The following `LET` statement is valid because `a` is defined first and then
referenced afterwards:

    GRAPH graph_db.FinGraph
    MATCH (source:Account)-[e:Transfers]->(destination:Account)
    LET a = source
    LET b = a
    RETURN b.id AS b_id

    /*---+
     | b_id |
     +---+
     | 20   |
     | 7    |
     | 7    |
     | 20   |
     | 16   |
     +---*/

In the following examples, the `LET` statements are invalid because `a` is
redefined:

    GRAPH graph_db.FinGraph
    MATCH (source:Account)-[e:Transfers]->(destination:Account)
    LET a = source, a = destination -- ERROR: 'a' has already been defined.
    RETURN a.id AS a_id

    GRAPH graph_db.FinGraph
    MATCH (source:Account)-[e:Transfers]->(destination:Account)
    LET a = source
    LET a = destination -- ERROR: 'a' has already been defined.
    RETURN a.id AS a_id

In the following examples, the `LET` statements are invalid because `b` is
redefined:

    GRAPH graph_db.FinGraph
    MATCH (source:Account)-[e:Transfers]->(destination:Account)
    LET a = source
    LET b = destination
    RETURN a, b

    NEXT

    MATCH (a)
    LET b = a -- ERROR: 'b' has already been defined.
    RETURN b.id

The following `LET` statement is valid because although `b` is defined in the
first linear query statement, it's not passed to the second linear query
statement:

    GRAPH graph_db.FinGraph
    MATCH (source:Account)-[e:Transfers]->(destination:Account)
    LET a = source
    LET b = destination
    RETURN a

    NEXT

    MATCH (a)
    LET b = a
    RETURN b.id

    /*---+
     | b_id |
     +---+
     | 20   |
     | 7    |
     | 7    |
     | 20   |
     | 16   |
     +---*/

## `LIMIT` statement

```
LIMIT count
```

#### Description

Limits the number of query results.

#### Definitions

- `count`: A non-negative `INT64` value that represents the number of results to produce. For more information, see the [`LIMIT` and `OFFSET` clauses](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#limit_and_offset_clause).

#### Details

The `LIMIT` statement can appear before the `RETURN` statement. You can also use
it as a qualifying clause in the [`RETURN` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/graph-query-statements#gql_return).

#### Examples

> [!NOTE]
> **Note:** The examples in this section reference a property graph called [`FinGraph`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/graph-schema-statements#fin_graph).

The following example uses the `LIMIT` statement to limit the query results to
three rows:

    GRAPH graph_db.FinGraph
    MATCH (source:Account)-[e:Transfers]->(destination:Account)
    ORDER BY source.Id
    LIMIT 3
    RETURN source.Id, source.nick_name

    /*---+---+
     | Id      | nick_name     |
     +---+---+
     | 7       | Vacation fund |
     | 7       | Vacation fund |
     | 16      | Vacation fund |
     +---*/

The following query finds the account and its owner with the most outgoing
transfers to a blocked account:

    GRAPH graph_db.FinGraph
    MATCH (src_account:Account)-[transfer:Transfers]->(dst_account:Account {is_blocked:true})
    RETURN src_account, COUNT(transfer) as total_transfers
    ORDER BY total_transfers
    LIMIT 1
    NEXT
    MATCH (src_account:Account)<-[owns:Owns]-(owner:Person)
    RETURN src_account.id AS account_id, owner.name AS owner_name

    /*---+
     | account_id | owner_name |
     +---+
     | 20         | Dana       |
     +---*/

## `MATCH` statement

```
[ OPTIONAL ] MATCH graph_pattern
```

#### Description

Matches data described by a graph pattern. You can have zero or more `MATCH`
statements in a linear query statement.

#### Definitions

- `MATCH graph_pattern`: The graph pattern to match. For more information, see [`MATCH` graph pattern definition](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/graph-patterns#graph_pattern_definition).
- `OPTIONAL MATCH graph_pattern`: The graph pattern to optionally match. If there are missing parts in the pattern, the missing parts are represented by `NULL` values. For more information, see [`OPTIONAL MATCH` graph pattern definition](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/graph-patterns#graph_pattern_definition).

#### Details

The `MATCH` statement joins the incoming working table with the matched
result with either `INNER JOIN` or `CROSS JOIN` semantics.

The `INNER JOIN` semantics is used when the working table and matched result
have variables in common. In the following example, the `INNER JOIN`
semantics is used because `account` is produced by both `MATCH` statements:

    MATCH (person:Person)-[:Owns]->(account:Account)
    MATCH (account)-[:Transfers]->(otherAcct:Account)

The `CROSS JOIN` semantics is used when the incoming working table and matched
result have no variables in common. In the following example, the `CROSS JOIN`
semantics is used because `person1` and `account` exist in the result of the
first `MATCH` statement, but not the second one:

    MATCH (person1:Person)-[:Owns]->(account:Account)
    MATCH (person2:Person)-[:Owns]->(otherAcct:Account)

#### Examples

> [!NOTE]
> **Note:** The examples in this section reference a property graph called [`FinGraph`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/graph-schema-statements#fin_graph).

The following query matches all `Person` nodes and returns the name and ID of
each person:

    GRAPH graph_db.FinGraph
    MATCH (p:Person)
    RETURN p.name, p.id

    /*---+
     | name | id |
     +---+
     | Alex | 1  |
     | Dana | 2  |
     | Lee  | 3  |
     +---*/

The following query matches all `Person` and `Account` nodes and returns their
labels and ID:

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

The following query matches all `Account` nodes that aren't blocked:

    GRAPH graph_db.FinGraph
    MATCH (a:Account {is_blocked: false})
    RETURN a.id

    /*---+
     | id |
     +---+
     | 7  |
     | 20 |
     +---*/

The following query matches all `Person` nodes that have a `birthday` less than
`1990-01-10`:

    GRAPH graph_db.FinGraph
    MATCH (p:Person WHERE p.birthday < '1990-01-10')
    RETURN p.name

    /*---+
     | name |
     +---+
     | Dana |
     | Lee  |
     +---*/

The following query matches all `Owns` edges:

    GRAPH graph_db.FinGraph
    MATCH -[e:Owns]->
    RETURN e.id

    /*---+
     | id |
     +---+
     | 1  |
     | 3  |
     | 2  |
     +---*/

The following query matches all `Owns` edges created within a specific period of
time:

    GRAPH graph_db.FinGraph
    MATCH -[e:Owns WHERE e.create_time > '2020-01-14' AND e.create_time < '2020-05-14']->
    RETURN e.id

    /*---+
     | id |
     +---+
     | 2  |
     | 3  |
     +---*/

The following query matches all `Transfers` edges where a blocked account is
involved in any direction:

    GRAPH graph_db.FinGraph
    MATCH (account:Account)-[transfer:Transfers]-(:Account {is_blocked:true})
    RETURN transfer.order_number, transfer.amount

    /*---+
     | order_number    | amount |
     +---+
     | 304330008004315 | 300    |
     | 304120005529714 | 100    |
     | 103650009791820 | 300    |
     | 302290001255747 | 200    |
     +---*/

The following query matches all `Transfers` initiated from an `Account` owned by
`Person` with `id` equal to `2`:

    GRAPH graph_db.FinGraph
    MATCH
      (p:Person {id: 2})-[:Owns]->(account:Account)-[t:Transfers]->
      (to_account:Account)
    RETURN p.id AS sender_id, to_account.id AS to_id

    /*---+
     | sender_id | to_id |
     +---+
     | 2         | 7     |
     | 2         | 16    |
     +---*/

The following query matches all the destination `Accounts` one to three
transfers away from a source `Account` with `id` equal to `7`, other than the
source itself:

    GRAPH graph_db.FinGraph
    MATCH (src:Account {id: 7})-[e:Transfers]->{1, 3}(dst:Account)
    WHERE src != dst
    RETURN ARRAY_LENGTH(e) AS hops, dst.id AS destination_account_id

    /*---+
     | hops | destination_account_id |
     +---+
     | 1    | 16                     |
     | 3    | 16                     |
     | 3    | 16                     |
     | 1    | 16                     |
     | 2    | 20                     |
     | 2    | 20                     |
     +---*/

The following query matches paths between `Account` nodes with one to two
`Transfers` edges through intermediate accounts that are blocked:

    GRAPH graph_db.FinGraph
    MATCH
      (src:Account)
      ((a:Account)-[:Transfers]->(b:Account {is_blocked:true}) WHERE a != b ){1,2}
      -[:Transfers]->(dst:Account)
    RETURN src.id AS source_account_id, dst.id AS destination_account_id

    /*---+
     | source_account_id | destination_account_id |
     +---+
     | 7                 | 20                     |
     | 7                 | 20                     |
     | 20                | 20                     |
     +---*/

The following query finds unique reachable accounts which are one or two
transfers away from a given `Account` node:

    GRAPH graph_db.FinGraph
    MATCH ANY (src:Account {id: 7})-[e:Transfers]->{1,2}(dst:Account)
    LET ids_in_path = ARRAY_CONCAT(ARRAY_AGG(e.Id), [dst.Id])
    RETURN src.id AS source_account_id, dst.id AS destination_account_id, ids_in_path

    /*---+
     | source_account_id | destination_account_id | ids_in_path |
     +---+
     | 7                 | 16                     | [7, 16]     |
     | 7                 | 20                     | [7, 16, 20] |
     +---*/

The following query matches all `Person` nodes and optionally matches the
blocked `Account` owned by the `Person`. The missing blocked `Account` is
represented as `NULL`:

    GRAPH graph_db.FinGraph
    MATCH (n:Person)
    OPTIONAL MATCH (n:Person)-[:Owns]->(a:Account {is_blocked: TRUE})
    RETURN n.name, a.id AS blocked_account_id

    /*---+
     | name  | id   |
     +---+
     | Lee   | 16   |
     | Alex  | NULL |
     | Dana  | NULL |
     +---*/

## `NEXT` statement

```
NEXT
```

#### Description

Chains multiple linear query statements together.

#### Examples

> [!NOTE]
> **Note:** The examples in this section reference a property graph called [`FinGraph`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/graph-schema-statements#fin_graph).

The following linear query statements are chained by the `NEXT` statement:

    GRAPH graph_db.FinGraph
    MATCH (:Account)-[:Transfers]->(account:Account)
    RETURN account, COUNT(*) AS num_incoming_transfers
    GROUP BY account

    NEXT

    MATCH (account:Account)<-[:Owns]-(owner:Person)
    RETURN
      account.id AS account_id, owner.name AS owner_name,
      num_incoming_transfers

    NEXT

    FILTER num_incoming_transfers < 2
    RETURN account_id, owner_name

    /*---+
     | account_id | owner_name |
     +---+
     | 7          | Alex       |
     | 20         | Dana       |
     +---*/

## `OFFSET` statement

```
OFFSET count
```

#### Description

Skips a specified number of rows in the query results.

#### Definitions

- `count`: A non-negative `INT64` value that represents the number of rows to skip. For more information, see the [`LIMIT` and `OFFSET` clauses](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#limit_and_offset_clause).

#### Details

The `OFFSET` statement can appear anywhere in a linear query statement before
the `RETURN` statement.

#### Examples

> [!NOTE]
> **Note:** The examples in this section reference a property graph called [`FinGraph`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/graph-schema-statements#fin_graph).

In the following example, the first two rows aren't included in the results:

    GRAPH graph_db.FinGraph
    MATCH (p:Person)
    OFFSET 2
    RETURN p.name, p.id

    /*---+
     | name | id |
     +---+
     | Lee  | 3  |
     +---*/

## `ORDER BY` statement

```
ORDER BY order_by_specification[, ...]

order_by_specification:
  expression
  [ { ASC | ASCENDING | DESC | DESCENDING } ]
  [ { NULLS FIRST | NULLS LAST } ]
```

#### Description

Orders the query results.

#### Definitions

- `expression`: The sort criterion for the result set. For more information, see the [`ORDER BY` clause](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#order_by_clause).
- `COLLATE collation_specification`: The collation specification for `expression`. For more information, see the [`ORDER BY` clause](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#order_by_clause).
- `ASC | ASCENDING | DESC | DESCENDING`: The sort order, which can be either
  ascending or descending. The following options are synonymous:

  - `ASC` and `ASCENDING`

  - `DESC` and `DESCENDING`

  For more information about sort order, see the
  [`ORDER BY` clause](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#order_by_clause).
- `NULLS FIRST | NULLS LAST`: Determines how `NULL` values are sorted for
  `expression`. For more information, see the
  [`ORDER BY` clause](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#order_by_clause).

#### Details

Ordinals aren't supported in the `ORDER BY` statement.

The `ORDER BY` statement is ignored unless it's immediately followed by the
`LIMIT` or `OFFSET` statement.

If you would like to apply `ORDER BY` to what is in `RETURN` statement, use the
`ORDER BY` clause in `RETURN` statement. For more information, see
[`RETURN` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/graph-query-statements#gql_return).

#### Examples

> [!NOTE]
> **Note:** The examples in this section reference a property graph called [`FinGraph`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/graph-schema-statements#fin_graph).

The following query sorts the results by the `transfer.amount`
values in descending order:

    GRAPH graph_db.FinGraph
    MATCH (src_account:Account)-[transfer:Transfers]->(dst_account:Account)
    ORDER BY transfer.amount DESC
    LIMIT 3
    RETURN src_account.id AS account_id, transfer.amount AS transfer_amount

    /*---+
     | account_id | transfer_amount |
     +---+
     | 20         | 500             |
     | 7          | 300             |
     | 16         | 300             |
     +---*/

    GRAPH graph_db.FinGraph
    MATCH (src_account:Account)-[transfer:Transfers]->(dst_account:Account)
    ORDER BY transfer.amount DESC
    OFFSET 1
    RETURN src_account.id AS account_id, transfer.amount AS transfer_amount

    /*---+
     | account_id | transfer_amount |
     +---+
     | 7          | 300             |
     | 16         | 300             |
     | 20         | 200             |
     | 7          | 100             |
     +---*/

If you don't include the `LIMIT` or `OFFSET` statement right after the
`ORDER BY` statement, the effect of `ORDER BY` is discarded and the result is
unordered.

    -- Warning: The transfer.amount values aren't sorted because the
    -- LIMIT statement is missing.
    GRAPH graph_db.FinGraph
    MATCH (src_account:Account)-[transfer:Transfers]->(dst_account:Account)
    ORDER BY transfer.amount DESC
    RETURN src_account.id AS account_id, transfer.amount AS transfer_amount

    /*---+
     | account_id | transfer_amount |
     +---+
     | 7          | 300             |
     | 7          | 100             |
     | 16         | 300             |
     | 20         | 500             |
     | 20         | 200             |
     +---*/

    -- Warning: Using the LIMIT clause in the RETURN statement, but not immediately
    -- after the ORDER BY statement, also returns the unordered transfer.amount
    -- values.
    GRAPH graph_db.FinGraph
    MATCH (src_account:Account)-[transfer:Transfers]->(dst_account:Account)
    ORDER BY transfer.amount DESC
    RETURN src_account.id AS account_id, transfer.amount AS transfer_amount
    LIMIT 10

    /*---+
     | account_id | transfer_amount |
     +---+
     | 7          | 300             |
     | 7          | 100             |
     | 16         | 300             |
     | 20         | 500             |
     | 20         | 200             |
     +---*/

## `RETURN` statement

```
RETURN *
```

```
RETURN
  [ { ALL | DISTINCT } ]
  return_item[, ... ]
  [ group_by_clause ]
  [ order_by_clause ]
  [ limit_and_offset_clauses ]

return_item:
expression [ AS alias ]

limit_and_offset_clauses:
  {
    limit_clause
    | offset_clause
    | offset_clause limit_clause
  }
```

#### Description

Marks the end of a linear query statement and returns the results. Only one `RETURN`
statement is allowed in a linear query statement.

#### Definitions

- `*`: Returns all columns in the current working table.
- `return_item`: A column to include in the results.
- `ALL`: Returns all rows. This is equivalent to not using any prefix.
- `DISTINCT`: Duplicate rows are discarded and only the remaining distinct rows are returned. This deduplication takes place after any aggregation is performed.
- `expression`: An expression that represents a column to produce. Aggregation is supported.
- `alias`: An alias for `expression`.
- `group_by_clause`: Groups the current rows of the working table, using the [`GROUP BY` clause](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#group_by_clause). If `GROUP BY ALL` is applied, the groupable items from the `return_item` list are used to group the rows.
- `order_by_clause`: Orders the current rows in a linear query statement, using the [`ORDER BY` clause](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#order_by_clause).
- `limit_clause`: Limits the number of current rows in a linear query statement, using the [`LIMIT` clause](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#limit_and_offset_clause).
- `offset_clause`: Skips a specified number of rows in a linear query statement, using the [`OFFSET` clause](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#limit_and_offset_clause).

#### Details

You can return graph elements to make them visible from one linear query
statement to the next, but the final `RETURN` statement in a graph query can't
contain a graph element on its own. Instead, you can return a property of a
graph element.

Ordinals aren't supported in the `ORDER BY` and `GROUP BY` clauses.

#### Examples

> [!NOTE]
> **Note:** The examples in this section reference a property graph called [`FinGraph`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/graph-schema-statements#fin_graph).

The following query returns `p.name` and `p.id`:

    GRAPH graph_db.FinGraph
    MATCH (p:Person)
    RETURN p.name, p.id

    /*---+
     | name | id |
     +---+
     | Alex | 1  |
     | Dana | 2  |
     | Lee  | 3  |
     +---*/

The following query fails because you can't return a graph element from a
query:

    -- Error: You can't return a graph element
    GRAPH graph_db.FinGraph
    MATCH (p:Person)
    RETURN p

In the following example, the first linear query statement returns all columns
including `p`, `a`, `b`, and `c`. The second linear query statement returns the
specified `p.name` and `d` columns:

    GRAPH graph_db.FinGraph
    MATCH (p:Person)
    LET a = 1, b = 2, c = 3
    RETURN *

    NEXT

    RETURN p.name, (a + b + c) AS d

    /*---+
     | name | d |
     +---+
     | Alex | 6 |
     | Dana | 6 |
     | Lee  | 6 |
     +---*/

The following query returns distinct rows:

    GRAPH graph_db.FinGraph
    MATCH (src:Account {id: 7})-[e:Transfers]->{1, 3}(dst:Account)
    RETURN DISTINCT ARRAY_LENGTH(e) AS hops, dst.id AS destination_account_id

    /*---+
     | hops | destination_account_id |
     +---+
     | 3    | 7                      |
     | 1    | 16                     |
     | 3    | 16                     |
     | 2    | 20                     |
     +---*/

In the following example, the first linear query statement returns `account` and
aggregated `num_incoming_transfers` per account. The second statement returns
sorted result.

    GRAPH graph_db.FinGraph
    MATCH (:Account)-[:Transfers]->(account:Account)
    RETURN account, COUNT(*) AS num_incoming_transfers
    GROUP BY account

    NEXT

    MATCH (account:Account)<-[:Owns]-(owner:Person)
    RETURN owner.name AS owner_name, num_incoming_transfers
    ORDER BY num_incoming_transfers DESC

    /*---+
     | owner_name | num_incoming_transfers |
     +---+
     | Lee        | 3                      |
     | Alex       | 1                      |
     | Dana       | 1                      |
     +---*/

In the following example, the `GROUP BY ALL` clause groups rows by inferring
grouping keys from the return items in the `RETURN` statement.

    GRAPH graph_db.FinGraph
    MATCH (:Account)-[:Transfers]->(account:Account)
    RETURN account, COUNT(*) AS num_incoming_transfers
    GROUP BY ALL
    ORDER BY num_incoming_transfers DESC

    NEXT

    MATCH (account:Account)<-[:Owns]-(owner:Person)
    RETURN owner.name AS owner_name, num_incoming_transfers

    /*---+
     | owner_name | num_incoming_transfers |
     +---+
     | Alex       | 1                      |
     | Dana       | 1                      |
     | Lee        | 3                      |
     +---*/

In the following example, the `LIMIT` clause in the `RETURN` statement
reduces the results to one row:

    GRAPH graph_db.FinGraph
    MATCH (p:Person)
    RETURN p.name, p.id
    LIMIT 1

    /*---+
     | name | id |
     +---+
     | Alex | 1  |
     +---*/

In the following example, the `OFFSET` clause in the `RETURN` statement
skips the first row:

    GRAPH graph_db.FinGraph
    MATCH (p:Person)
    RETURN p.name, p.id
    OFFSET 1

    /*---+
     | name | id |
     +---+
     | Dana | 2  |
     | Lee  | 3  |
     +---*/

In the following example, the `OFFSET` clause in the `RETURN` statement
skips the first row, then the `LIMIT` clause reduces the
results to one row:

    GRAPH graph_db.FinGraph
    MATCH (p:Person)
    RETURN p.name, p.id
    OFFSET 1
    LIMIT 1

    /*---+
     | name | id |
     +---+
     | Dana | 2  |
     +---*/

In the following example, an error is produced because the `OFFSET` clause must
come before the `LIMIT` clause when they are both used in the
`RETURN` statement:

    -- Error: The LIMIT clause must come after the OFFSET clause in a
    -- RETURN operation.
    GRAPH graph_db.FinGraph
    MATCH (p:Person)
    RETURN p.name, p.id
    LIMIT 1
    OFFSET 1

In the following example, the `ORDER BY` clause in the `RETURN` statement sorts
the results by `hops` and then `destination_account_id`:

    GRAPH graph_db.FinGraph
    MATCH (src:Account {id: 7})-[e:Transfers]->{1, 3}(dst:Account)
    RETURN DISTINCT ARRAY_LENGTH(e) AS hops, dst.id AS destination_account_id
    ORDER BY hops, destination_account_id

    /*---+
     | hops | destination_account_id |
     +---+
     | 1    | 16                     |
     | 2    | 20                     |
     | 3    | 7                      |
     | 3    | 16                     |
     +---*/

## `SKIP` statement

```
SKIP count
```

#### Description

Synonym for the [`OFFSET` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/graph-query-statements#gql_offset).

#### Examples

> [!NOTE]
> **Note:** The examples in this section reference a property graph called [`FinGraph`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/graph-schema-statements#fin_graph).

`SKIP` is a synonym for `OFFSET`. Therefore, these queries are equivalent:

    GRAPH graph_db.FinGraph
    MATCH (p:Person)
    SKIP 2
    RETURN p.name, p.id

    /*---+
     | name | id |
     +---+
     | Lee  | 3  |
     +---*/

    GRAPH graph_db.FinGraph
    MATCH (p:Person)
    OFFSET 2
    RETURN p.name, p.id

    /*---+
     | name | id |
     +---+
     | Lee  | 3  |
     +---*/

## `WITH` statement

```
WITH
  [ { ALL | DISTINCT } ]
  return_item[, ... ]
  [ group_by_clause ]

return_item:
expression [ AS alias ]

```

#### Description

Passes on the specified columns, optionally filtering, renaming, and
transforming those results.

#### Definitions

- `*`: Returns all columns in the current working table.
- `ALL`: Returns all rows. This is equivalent to not using any prefix.
- `DISTINCT`: Returns distinct rows. Deduplication takes place after aggregations are performed.
- `return_item`: A column to include in the results.
- `expression`: An expression that represents a column to produce. Aggregation is supported.
- `alias`: An alias for `expression`.
- `group_by_clause`: Groups the current rows of the working table, using the [`GROUP BY` clause](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#group_by_clause). If `GROUP BY ALL` is applied, the groupable items from the `return_item` list are used to group the rows.

#### Details

If any expression performs aggregation, and no `GROUP BY` clause is
specified, all groupable items from the return list are implicitly used as
grouping keys (This is equivalent to `GROUP BY ALL`).

Window functions aren't supported in `expression`.

Ordinals aren't supported in the `GROUP BY` clause.

#### Examples

> [!NOTE]
> **Note:** The examples in this section reference a property graph called [`FinGraph`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/graph-schema-statements#fin_graph).

The following query returns all distinct destination account IDs:

    GRAPH graph_db.FinGraph
    MATCH (src:Account)-[transfer:Transfers]->(dst:Account)
    WITH DISTINCT dst
    RETURN dst.id AS destination_id

    /*---+
     | destination_id |
     +---+
     | 7              |
     | 16             |
     | 20             |
     +---*/

The following query uses `*` to carry over the existing columns of
the working table in addition to defining a new one for the destination
account id.

    GRAPH graph_db.FinGraph
    MATCH (src:Account)-[transfer:Transfers]->(dst:Account)
    WITH *, dst.id
    RETURN dst.id AS destination_id

    /*---+
     | destination_id |
     +---+
     | 7              |
     | 16             |
     | 16             |
     | 16             |
     | 20             |
     +---*/

In the following example, aggregation is performed implicitly because the
`WITH` statement has an aggregate expression but doesn't specify a `GROUP BY`
clause. All groupable items from the return item list are used as grouping keys
(This is equivalent to `GROUP BY ALL`).
In this case, the grouping keys inferred are `src.id` and `dst.id`.
Therefore, this query returns the number of transfers for each
distinct combination of `src.id` and `dst.id`.

    GRAPH graph_db.FinGraph
    MATCH (src:Account)-[transfer:Transfers]->(dst:Account)
    WITH COUNT(*) AS transfer_total, src.id AS source_id, dst.id AS destination_id
    RETURN transfer_total, destination_id, source_id

    /*---+
     | transfer_total | destination_id | source_id |
     +---+
     | 2              | 16             | 7         |
     | 1              | 20             | 16        |
     | 1              | 7              | 20        |
     | 1              | 16             | 20        |
     +---*/

In the following example, an error is produced because the `WITH` statement only
contains `dst`. `src` isn't visible after the `WITH` statement in the `RETURN`
statement.

    -- Error: src doesn't exist
    GRAPH graph_db.FinGraph
    MATCH (src:Account)-[transfer:Transfers]->(dst:Account)
    WITH dst
    RETURN src.id AS source_id

## Set operation

```
linear_query_statement
set_operator
linear_query_statement
[
  set_operator
  linear_graph_query
][...]

set_operator:
  { 
    UNION ALL
    | UNION DISTINCT
    | INTERSECT DISTINCT
    | EXCEPT DISTINCT
  }

```

#### Description

Combines a sequence of linear query statements with a set operation.
Only one type of set operation is allowed per set operation.

#### Definitions

- `linear_query_statement`: A [linear query statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/graph-query-statements#gql_syntax) to include in the set operation.

#### Details

Each linear query statement in the same set operation shares the same working table.

Most of the rules for GQL set operators are the same as those for
SQL [set operators](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#set_operators), but there are some differences:

- A GQL set operator doesn't support the `CORRESPONDING` keyword. Since each set operation input (a linear query statement) only produces columns with names, the default behavior of GQL set operations requires all inputs to have the same set of column names and all paired columns to share the same [supertype](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/conversion_rules#supertypes).
- GQL doesn't allow chaining different kinds of set operations in the same set operation.
- GQL doesn't allow using parentheses to separate different set operations.
- The results produced by the linear query statements are combined in a left associative order.
- Only the `UNION ALL` set operator supports input columns with type `GRAPH_ELEMENT`.

#### Examples

A set operation between two linear query statements with the same set of
output column names and types but with different column orders is supported.
For example:

    GRAPH graph_db.FinGraph
    MATCH (p:Person)
    RETURN p.name, 1 AS group_id
    UNION ALL
    MATCH (p:Person)
    RETURN 2 AS group_id, p.name

    /*---+---+
     | name | group_id |
     +---+---+
     | Alex |    1     |
     | Dana |    1     |
     | Lee  |    1     |
     | Alex |    2     |
     | Dana |    2     |
     | Lee  |    2     |
     +---+---*/

In a set operation, chaining the same kind of set operation is supported, but
chaining different kinds of set operations isn't.
For example:

    GRAPH graph_db.FinGraph
    MATCH (p:Person)
    RETURN p.name, 1 AS group_id
    UNION ALL
    MATCH (p:Person)
    RETURN 2 AS group_id, p.name
    UNION ALL
    MATCH (p:Person)
    RETURN 3 AS group_id, p.name

    /*---+---+
     | name | group_id |
     +---+---+
     | Alex |    1     |
     | Dana |    1     |
     | Lee  |    1     |
     | Alex |    2     |
     | Dana |    2     |
     | Lee  |    2     |
     | Alex |    3     |
     | Dana |    3     |
     | Lee  |    3     |
     +---+---*/

    -- ERROR: GQL doesn't allow chaining EXCEPT DISTINCT with UNION ALL
    GRAPH graph_db.FinGraph
    MATCH (p:Person)
    RETURN p.name, 1 AS group_id
    UNION ALL
    MATCH (p:Person)
    RETURN 2 AS group_id, p.name
    EXCEPT DISTINCT
    MATCH (p:Person)
    RETURN 3 AS group_id, p.name