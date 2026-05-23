> [!WARNING]
> GoogleSQL is the new name for Google Standard SQL! New name, same great SQL dialect.

Query statements scan one or more tables or expressions and return the computed
result rows. This topic describes the syntax for SQL queries in
GoogleSQL for BigQuery.

## SQL syntax notation rules

The following table lists and describes the syntax notation rules that GoogleSQL
documentation commonly uses.

| Notation | Example | Description |
|---|---|---|
| Square brackets | `[ ]` | Optional clauses |
| Parentheses | `( )` | Literal parentheses |
| Vertical bar | `|` | Logical `XOR` (exclusive `OR`) |
| Curly braces | `{ }` | A set of options, such as `{ a | b | c }`. Select one option. |
| Ellipsis | `...` | The preceding item can repeat. |
| Comma | `,` | Literal comma |
| Comma followed by an ellipsis | `, ...` | The preceding item can repeat in a comma-separated list. |
| Item list | `item [, ...]` | One or more items |
| Item list | `[item, ...]` | Zero or more items |
| Double quotes | `""` | The enclosed syntax characters (for example, `"{"..."}"`) are literal and required. |
| Angle brackets | `<>` | Literal angle brackets |

## SQL syntax

```
query_statement:
  query_expr

query_expr:
  [ WITH [ RECURSIVE ] { non_recursive_cte | recursive_cte }[, ...] ]
  { select | ( query_expr ) | set_operation }
  [ ORDER BY expression [{ ASC | DESC }] [, ...] ]
  [ LIMIT count [ OFFSET skip_rows ] ]

select:
  SELECT
    [ WITH differential_privacy_clause ]
    [ { ALL | DISTINCT } ]
    [ AS { STRUCT | VALUE } ]
    select_list
  [ FROM from_clause[, ...] ]
  [ WHERE bool_expression ]
  [ GROUP BY group_by_specification ]
  [ HAVING bool_expression ]
  [ QUALIFY bool_expression ]
  [ WINDOW window_clause ]
```

## `SELECT` statement

```
SELECT
  [ WITH differential_privacy_clause ]
  [ { ALL | DISTINCT } ]
  [ AS { STRUCT | VALUE } ]
  select_list

select_list:
  { select_all | select_expression } [, ...]

select_all:
  [ expression. ]*
  [ EXCEPT ( column_name [, ...] ) ]
  [ REPLACE ( expression AS column_name [, ...] ) ]

select_expression:
  expression [ [ AS ] alias ]
```

The `SELECT` list defines the columns that the query will return. Expressions in
the `SELECT` list can refer to columns in any of the `from_item`s in its
corresponding `FROM` clause.

Each item in the `SELECT` list is one of:

- `*`
- `expression`
- `expression.*`

### `SELECT *`

`SELECT *`, often referred to as *select star*, produces one output column for
each column that's visible after executing the full query.

    SELECT * FROM (SELECT "apple" AS fruit, "carrot" AS vegetable);

    /*---+---+
     | fruit | vegetable |
     +---+---+
     | apple | carrot    |
     +---+---*/

### `SELECT expression`

Items in a `SELECT` list can be expressions. These expressions evaluate to a
single value and produce one output column, with an optional explicit `alias`.

If the expression doesn't have an explicit alias, it receives an implicit alias
according to the rules for [implicit aliases](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#implicit_aliases), if possible.
Otherwise, the column is anonymous and you can't refer to it by name elsewhere
in the query.

### `SELECT expression.*`

An item in a `SELECT` list can also take the form of `expression.*`. This
produces one output column for each column or top-level field of `expression`.
The expression must either be a table alias or evaluate to a single value of a
data type with fields, such as a STRUCT.

> [!NOTE]
> **Note:** The `*` or `.*` wildcard preserves the order of the fields in the data structure on which they're operating.

The following query produces one output column for each column in the table
`groceries`, aliased as `g`.

    WITH groceries AS
      (SELECT "milk" AS dairy,
       "eggs" AS protein,
       "bread" AS grain)
    SELECT g.*
    FROM groceries AS g;

    /*---+---+---+
     | dairy | protein | grain |
     +---+---+---+
     | milk  | eggs    | bread |
     +---+---+---*/

More examples:

    WITH locations AS
      (SELECT STRUCT("Seattle" AS city, "Washington" AS state) AS location
      UNION ALL
      SELECT STRUCT("Phoenix" AS city, "Arizona" AS state) AS location)
    SELECT l.location.*
    FROM locations l;

    /*---+---+
     | city    | state      |
     +---+---+
     | Seattle | Washington |
     | Phoenix | Arizona    |
     +---+---*/

    WITH locations AS
      (SELECT ARRAY<STRUCT<city STRING, state STRING>>[("Seattle", "Washington"),
        ("Phoenix", "Arizona")] AS location)
    SELECT l.LOCATION[offset(0)].*
    FROM locations l;

    /*---+---+
     | city    | state      |
     +---+---+
     | Seattle | Washington |
     +---+---*/

### `SELECT * EXCEPT`

A `SELECT * EXCEPT` statement specifies the names of one or more columns to
exclude from the result. All matching column names are omitted from the output.

    WITH orders AS
      (SELECT 5 as order_id,
      "sprocket" as item_name,
      200 as quantity)
    SELECT * EXCEPT (order_id)
    FROM orders;

    /*---+---+
     | item_name | quantity |
     +---+---+
     | sprocket  | 200      |
     +---+---*/

> [!NOTE]
> **Note:** `SELECT * EXCEPT` doesn't exclude columns that don't have names.

### `SELECT * REPLACE`

A `SELECT * REPLACE` statement specifies one or more
`expression AS identifier` clauses. Each identifier must match a column name
from the `SELECT *` statement. In the output column list, the column that
matches the identifier in a `REPLACE` clause is replaced by the expression in
that `REPLACE` clause.

A `SELECT * REPLACE` statement doesn't change the names or order of columns.
However, it can change the value and the value type.

    WITH orders AS
      (SELECT 5 as order_id,
      "sprocket" as item_name,
      200 as quantity)
    SELECT * REPLACE ("widget" AS item_name)
    FROM orders;

    /*---+---+---+
     | order_id | item_name | quantity |
     +---+---+---+
     | 5        | widget    | 200      |
     +---+---+---*/

    WITH orders AS
      (SELECT 5 as order_id,
      "sprocket" as item_name,
      200 as quantity)
    SELECT * REPLACE (quantity/2 AS quantity)
    FROM orders;

    /*---+---+---+
     | order_id | item_name | quantity |
     +---+---+---+
     | 5        | sprocket  | 100      |
     +---+---+---*/

> [!NOTE]
> **Note:** `SELECT * REPLACE` doesn't replace columns that don't have names.

### `SELECT DISTINCT`

A `SELECT DISTINCT` statement discards duplicate rows and returns only the
remaining rows. `SELECT DISTINCT` can't return columns of the following types:

- `GRAPH_ELEMENT`
- `GRAPH_PATH`

In the following example, `SELECT DISTINCT` is used to produce distinct arrays:

    WITH PlayerStats AS (
      SELECT ['Coolidge', 'Adams'] as Name, 3 as PointsScored UNION ALL
      SELECT ['Adams', 'Buchanan'], 0 UNION ALL
      SELECT ['Coolidge', 'Adams'], 1 UNION ALL
      SELECT ['Kiran', 'Noam'], 1)
    SELECT DISTINCT Name
    FROM PlayerStats;

    /*---+
     | Name             |
     +---+
     | [Coolidge,Adams] |
     | [Adams,Buchanan] |
     | [Kiran,Noam]     |
     +---*/

In the following example, `SELECT DISTINCT` is used to produce distinct structs:

    WITH
      PlayerStats AS (
        SELECT
          STRUCT<last_name STRING, first_name STRING, age INT64>(
            'Adams', 'Noam', 20) AS Player,
          3 AS PointsScored UNION ALL
        SELECT ('Buchanan', 'Jie', 19), 0 UNION ALL
        SELECT ('Adams', 'Noam', 20), 4 UNION ALL
        SELECT ('Buchanan', 'Jie', 19), 13
      )
    SELECT DISTINCT Player
    FROM PlayerStats;

    /*---+
     | player                   |
     +---+
     | {                        |
     |   last_name: "Adams",    |
     |   first_name: "Noam",    |
     |   age: 20                |
     |  }                       |
     +---+
     | {                        |
     |   last_name: "Buchanan", |
     |   first_name: "Jie",     |
     |   age: 19                |
     |  }                       |
     +---*/

### `SELECT ALL`

A `SELECT ALL` statement returns all rows, including duplicate rows.
`SELECT ALL` is the default behavior of `SELECT`.

### `SELECT AS STRUCT`

    SELECT AS STRUCT expr [[AS] struct_field_name1] [,...]

This produces a [value table](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#value_tables) with a
STRUCT row type, where the
STRUCT field names and types match the column names
and types produced in the `SELECT` list.

Example:

    SELECT ARRAY(SELECT AS STRUCT 1 a, 2 b)

`SELECT AS STRUCT` can be used in a scalar or array subquery to produce a single
STRUCT type grouping multiple values together. Scalar
and array subqueries (see [Subqueries](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/subqueries)) are normally not
allowed to return multiple columns, but can return a single column with
STRUCT type.

### `SELECT AS VALUE`

`SELECT AS VALUE` produces a [value table](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#value_tables) from any
`SELECT` list that produces exactly one column. Instead of producing an
output table with one column, possibly with a name, the output will be a
value table where the row type is just the value type that was produced in the
one `SELECT` column. Any alias the column had will be discarded in the
value table.

Example:

    SELECT AS VALUE STRUCT(1 AS a, 2 AS b) xyz

The query above produces a table with row type `STRUCT<a int64, b int64>`.

## `FROM` clause

```
FROM from_clause[, ...]

from_clause:
  from_item
  [ { pivot_operator | unpivot_operator | match_recognize_clause } ]
  [ tablesample_operator ]

from_item:
  {
    table_name [ as_alias ] [ FOR SYSTEM_TIME AS OF timestamp_expression ] 
    | { join_operation | ( join_operation ) }
    | ( query_expr ) [ as_alias ]
    | field_path
    | unnest_operator
    | https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#cte_name [ as_alias ]
    | graph_table_operator [ as_alias ]
  }

as_alias:
  [ AS ] alias
```

The `FROM` clause indicates the table or tables from which to retrieve rows,
and specifies how to join those rows together to produce a single stream of
rows for processing in the rest of the query.

#### `pivot_operator`

See [PIVOT operator](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#pivot_operator).

#### `unpivot_operator`

See [UNPIVOT operator](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#unpivot_operator).

#### `tablesample_operator`

See [TABLESAMPLE operator](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#tablesample_operator).

#### `match_recognize_clause`

See [MATCH_RECOGNIZE clause](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#match_recognize_clause).

#### `graph_table_operator`

See [GRAPH_TABLE operator](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/graph-sql-queries#graph_table_operator).

#### `table_name`

The name (optionally qualified) of an existing table.

```
SELECT * FROM Roster;
SELECT * FROM dataset.Roster;
SELECT * FROM project.dataset.Roster;
```

#### `FOR SYSTEM_TIME AS OF`

`FOR SYSTEM_TIME AS OF` references the historical versions of the table
definition and rows that were current at `timestamp_expression`.

Limitations:

The source table in the `FROM` clause containing `FOR SYSTEM_TIME AS OF` must
not be any of the following:

- An array scan, including a [flattened array](https://docs.cloud.google.com/bigquery/docs/arrays#flattening_arrays) or the output of the `UNNEST` operator.
- A common table expression defined by a `WITH` clause.
- The source table in a `CREATE TABLE FUNCTION` statement creating a new table-valued function

`timestamp_expression` must be a constant expression. It can't
contain the following:

- Subqueries.
- Correlated references (references to columns of a table that appear at a higher level of the query statement, such as in the `SELECT` list).
- User-defined functions (UDFs).

The value of `timestamp_expression` can't fall into the following ranges:

- After the current timestamp (in the future).
- More than seven (7) days before the current timestamp.

A single query statement can't reference a single table at more than one point
in time, including the current time. That is, a query can reference a table
multiple times at the same timestamp, but not the current version and a
historical version, or two different historical versions.

> [!NOTE]
> **Note:** DML statements always operate on the current version of the destination table, so if the destination table is used multiple times in the query, all of them must use the current version.

The default time zone for `timestamp_expression` in a
`FOR SYSTEM_TIME AS OF` expression is `America/Los_Angeles`, even though the
default time zone for timestamp literals is `UTC`.

Examples:

The following query returns a historical version of the table from one hour ago.

    SELECT *
    FROM t
      FOR SYSTEM_TIME AS OF TIMESTAMP_SUB(CURRENT_TIMESTAMP(), INTERVAL 1 HOUR);

The following query returns a historical version of the table at an absolute
point in time.

    SELECT *
    FROM t
      FOR SYSTEM_TIME AS OF '2017-01-01 10:00:00-07:00';

The following query returns an error because the `timestamp_expression` contains
a correlated reference to a column in the containing query.

    SELECT *
    FROM t1
    WHERE t1.a IN (SELECT t2.a
                   FROM t2 FOR SYSTEM_TIME AS OF t1.timestamp_column);

The following operations show accessing a historical version of the table before
table is replaced.

    DECLARE before_replace_timestamp TIMESTAMP;

    -- Create table books.
    CREATE TABLE books AS
    SELECT 'Hamlet' title, 'William Shakespeare' author;

    -- Get current timestamp before table replacement.
    SET before_replace_timestamp = CURRENT_TIMESTAMP();

    -- Replace table with different schema(title and release_date).
    CREATE OR REPLACE TABLE books AS
    SELECT 'Hamlet' title, DATE '1603-01-01' release_date;

    -- This query returns Hamlet, William Shakespeare as result.
    SELECT * FROM books FOR SYSTEM_TIME AS OF before_replace_timestamp;

The following operations show accessing a historical version of the table
before a DML job.

    DECLARE JOB_START_TIMESTAMP TIMESTAMP;

    -- Create table books.
    CREATE OR REPLACE TABLE books AS
    SELECT 'Hamlet' title, 'William Shakespeare' author;

    -- Insert two rows into the books.
    INSERT books (title, author)
    VALUES('The Great Gatsby', 'F. Scott Fizgerald'),
          ('War and Peace', 'Leo Tolstoy');

    SELECT * FROM books;

    SET JOB_START_TIMESTAMP = (
      SELECT start_time
      FROM `region-us`.INFORMATION_SCHEMA.JOBS_BY_USER
      WHERE job_type="QUERY"
        AND statement_type="INSERT"
      ORDER BY start_time DESC
      LIMIT 1
     );

    -- This query only returns Hamlet, William Shakespeare as result.
    SELECT * FROM books FOR SYSTEM_TIME AS OF JOB_START_TIMESTAMP;

The following query returns an error because the DML operates on the current
version of the table, and a historical version of the table from one day ago.

    INSERT INTO t1
    SELECT * FROM t1
      FOR SYSTEM_TIME AS OF TIMESTAMP_SUB(CURRENT_TIMESTAMP(), INTERVAL 1 DAY);

#### `join_operation`

See [Join operation](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#join_types).

#### `query_expr`

`( query_expr ) [ [ AS ] alias ]` is a [table subquery](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/subqueries#table_subquery_concepts).

#### `field_path`

In the `FROM` clause, `field_path` is any path that
resolves to a field within a data type. `field_path` can go
arbitrarily deep into a nested data structure.

Some examples of valid `field_path` values include:

    SELECT * FROM T1 t1, t1.array_column;

    SELECT * FROM T1 t1, t1.struct_column.array_field;

    SELECT (SELECT ARRAY_AGG(c) FROM t1.array_column c) FROM T1 t1;

    SELECT a.struct_field1 FROM T1 t1, t1.array_of_structs a;

    SELECT (SELECT STRING_AGG(a.struct_field1) FROM t1.array_of_structs a) FROM T1 t1;

Field paths in the `FROM` clause must end in an
array field. In
addition, field paths can't contain arrays before the end of the path. For example, the path
`array_column.some_array.some_array_field` is invalid because it
contains an array before the end of the path.

> [!NOTE]
> **Note:** If a path has only one name, it's interpreted as a table. To work around this, wrap the path using `UNNEST`, or use the fully-qualified path.

> [!NOTE]
> **Note:** If a path has more than one name, and it matches a field name, it's interpreted as a field name. To force the path to be interpreted as a table name, wrap the path using `` ` ``.

#### `unnest_operator`

See [UNNEST operator](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#unnest_operator).

#### `cte_name`

Common table expressions (CTEs) in a [`WITH` Clause](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#with_clause) act like
temporary tables that you can reference anywhere in the `FROM` clause.
In the example below, `subQ1` and `subQ2` are CTEs.

Example:

    WITH
      subQ1 AS (SELECT * FROM Roster WHERE SchoolID = 52),
      subQ2 AS (SELECT SchoolID FROM subQ1)
    SELECT DISTINCT * FROM subQ2;

The `WITH` clause hides any permanent tables with the same name
for the duration of the query, unless you qualify the table name, for example:

`dataset.Roster` or `project.dataset.Roster`.

## `UNNEST` operator

```
unnest_operator:
  {
    UNNEST( array ) [ as_alias ]
    | array_path [ as_alias ]
  }
  [ WITH OFFSET [ as_alias ] ]

array:
  { array_expression | array_path }

as_alias:
  [AS] alias
```

The `UNNEST` operator takes an array and returns a table with one row for each
element in the array. The output of `UNNEST` is one [value table](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#value_tables) column.
For these `ARRAY` element types, `SELECT *` against the value table column
returns multiple columns:

- `STRUCT`

Input values:

- `array_expression`: An expression that produces an array and that's not an array path.
- `array_path`: The
  path to an `ARRAY` type.

  - In an implicit `UNNEST` operation, the path must start with a [range variable](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#range_variables) name.
  - In an explicit `UNNEST` operation, the path can optionally start with a [range variable](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#range_variables) name.

  The `UNNEST` operation with any [correlated](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#correlated_join) `array_path` must
  be on the right side of a `CROSS JOIN`, `LEFT JOIN`, or
  `INNER JOIN` operation.
- `as_alias`: If specified, defines the explicit name of the value table
  column containing the array element values. It can be used to refer to
  the column elsewhere in the query.

- `WITH OFFSET`: `UNNEST` destroys the order of elements in the input
  array. Use this optional clause to return an additional column with
  the array element indexes, or *offsets* . Offset counting starts at zero for
  each row produced by the `UNNEST` operation. This column has an
  optional alias; If the optional alias isn't used, the default column name is
  `offset`.

  Example:

      SELECT * FROM UNNEST ([10,20,30]) as numbers WITH OFFSET;

      /*---+---+
       | numbers | offset |
       +---+---+
       | 10      | 0      |
       | 20      | 1      |
       | 30      | 2      |
       +---+---*/

You can also use `UNNEST` outside of the `FROM` clause with the
[`IN` operator](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/operators#in_operators).

For several ways to use `UNNEST`, including construction, flattening, and
filtering, see [Work with arrays](https://docs.cloud.google.com/bigquery/docs/arrays).

To learn more about the ways you can use `UNNEST` explicitly and implicitly,
see [Explicit and implicit `UNNEST`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#explicit_implicit_unnest).

### `UNNEST` and structs

For an input array of structs, `UNNEST`
returns a row for each struct, with a separate column for each field in the
struct. The alias for each column is the name of the corresponding struct
field.

Example:

    SELECT *
    FROM UNNEST(
      ARRAY<
        STRUCT<
          x INT64,
          y STRING,
          z STRUCT<a INT64, b INT64>>>[
            (1, 'foo', (10, 11)),
            (3, 'bar', (20, 21))]);

    /*---+---+---+
     | x | y   | z        |
     +---+---+---+
     | 1 | foo | {10, 11} |
     | 3 | bar | {20, 21} |
     +---+---+---*/

Because the `UNNEST` operator returns a
[value table](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#value_tables),
you can alias `UNNEST` to define a range variable that you can reference
elsewhere in the query. If you reference the range variable in the `SELECT`
list, the query returns a struct containing all of the fields of the original
struct in the input table.

Example:

    SELECT *, struct_value
    FROM UNNEST(
      ARRAY<
        STRUCT<
        x INT64,
        y STRING>>[
          (1, 'foo'),
          (3, 'bar')]) AS struct_value;

    /*---+---+---+
     | x | y   | struct_value |
     +---+---+---+
     | 3 | bar | {3, bar}     |
     | 1 | foo | {1, foo}     |
     +---+---+---*/

### Explicit and implicit `UNNEST`

Array unnesting can be either explicit or implicit. To learn more, see the
following sections.

#### Explicit unnesting

The `UNNEST` keyword is required in explicit unnesting. For example:

    WITH Coordinates AS (SELECT [1,2] AS position)
    SELECT results FROM Coordinates, UNNEST(Coordinates.position) AS results;

This example and the following examples use the `array_path` called
`Coordinates.position` to illustrate unnesting.

#### Implicit unnesting

The `UNNEST` keyword isn't used in implicit unnesting.

For example:

    WITH Coordinates AS (SELECT [1,2] AS position)
    SELECT results FROM Coordinates, Coordinates.position AS results;

##### Tables and implicit unnesting

When you use `array_path` with implicit `UNNEST`, `array_path` must be prepended
with the table. For example:

    WITH Coordinates AS (SELECT [1,2] AS position)
    SELECT results FROM Coordinates, Coordinates.position AS results;

### `UNNEST` and `NULL` values

`UNNEST` treats `NULL` values as follows:

- `NULL` and empty arrays produce zero rows.
- An array containing `NULL` values produces rows containing `NULL` values.

## `PIVOT` operator

```
FROM from_item[, ...] pivot_operator

pivot_operator:
  PIVOT(
    aggregate_function_call [as_alias][, ...]
    FOR input_column
    IN ( pivot_column [as_alias][, ...] )
  ) [AS alias]

as_alias:
  [AS] alias
```

The `PIVOT` operator rotates rows into columns, using aggregation.
`PIVOT` is part of the `FROM` clause.

- `PIVOT` can be used to modify any table expression.
- Combining `PIVOT` with `FOR SYSTEM_TIME AS OF` isn't allowed, although users may use `PIVOT` against a subquery input which itself uses `FOR SYSTEM_TIME AS OF`.
- A `WITH OFFSET` clause immediately preceding the `PIVOT` operator isn't allowed.

Conceptual example:

    -- Before PIVOT is used to rotate sales and quarter into Q1, Q2, Q3, Q4 columns:
    /*---+---+---+---+
     | product | sales | quarter | year |
     +---+---+---+---|
     | Kale    | 51    | Q1      | 2020 |
     | Kale    | 23    | Q2      | 2020 |
     | Kale    | 45    | Q3      | 2020 |
     | Kale    | 3     | Q4      | 2020 |
     | Kale    | 70    | Q1      | 2021 |
     | Kale    | 85    | Q2      | 2021 |
     | Apple   | 77    | Q1      | 2020 |
     | Apple   | 0     | Q2      | 2020 |
     | Apple   | 1     | Q1      | 2021 |
     +---+---+---+---*/

    -- After PIVOT is used to rotate sales and quarter into Q1, Q2, Q3, Q4 columns:
    /*---+---+---+---+---+---+
     | product | year | Q1 | Q2   | Q3   | Q4   |
     +---+---+---+---+---+---+
     | Apple   | 2020 | 77 | 0    | NULL | NULL |
     | Apple   | 2021 | 1  | NULL | NULL | NULL |
     | Kale    | 2020 | 51 | 23   | 45   | 3    |
     | Kale    | 2021 | 70 | 85   | NULL | NULL |
     +---+---+---+---+---+---*/

**Definitions**

Top-level definitions:

- `from_item`: The table, subquery, or table-valued function (TVF) on which to perform a pivot operation. The `from_item` must [follow these rules](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#rules_for_pivot_from_item).
- `pivot_operator`: The pivot operation to perform on a `from_item`.
- `alias`: An alias to use for an item in the query.

`pivot_operator` definitions:

- `aggregate_function_call`: An aggregate function call that aggregates all input rows such that `input_column` matches a particular value in `pivot_column`. Each aggregation corresponding to a different `pivot_column` value produces a different column in the output. [Follow these rules](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#rules_for_pivot_agg_function) when creating an aggregate function call.
- `input_column`: Takes a column and retrieves the row values for the column, [following these rules](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#rules_input_column).
- `pivot_column`: A pivot column to create for each aggregate function call. If an alias isn't provided, a default alias is created. A pivot column value type must match the value type in `input_column` so that the values can be compared. It's possible to have a value in `pivot_column` that doesn't match a value in `input_column`. Must be a constant and [follow these rules](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#rules_pivot_column).

**Rules**


Rules for a `from_item` passed to `PIVOT`:

- The `from_item` may consist of any table, subquery, or table-valued function (TVF) result.
- The `from_item` may not produce a value table.
- The `from_item` may not be a subquery using `SELECT AS STRUCT`.


Rules for `aggregate_function_call`:

- Must be an aggregate function. For example, `SUM`.
- You may reference columns in a table passed to `PIVOT`, as well as correlated columns, but may not access columns defined by the `PIVOT` clause itself.
- A table passed to `PIVOT` may be accessed through its alias if one is provided.
- You can only use an aggregate function that takes one argument.
- Except for `COUNT`, you can only use aggregate functions that ignore `NULL` inputs.
- If you are using `COUNT`, you can use `*` as an argument.


Rules for `input_column`:

- May access columns from the input table, as well as correlated columns, not columns defined by the `PIVOT` clause, itself.
- Evaluated against each row in the input table; aggregate and window function calls are prohibited.
- Non-determinism is okay.
- The type must be groupable.
- The input table may be accessed through its alias if one is provided.


Rules for `pivot_column`:

- A `pivot_column` must be a constant.
- Named constants, such as variables, aren't supported.
- Query parameters aren't supported.
- If a name is desired for a named constant or query parameter, specify it explicitly with an alias.
- Corner cases exist where a distinct `pivot_column`s can end up with the same default column names. For example, an input column might contain both a `NULL` value and the string literal `"NULL"`. When this happens, multiple pivot columns are created with the same name. To avoid this situation, use aliases for pivot column names.
- If a `pivot_column` doesn't specify an alias, a column name is constructed as follows:

| From | To | Example |
|---|---|---|
| NULL | NULL | Input: NULL Output: "NULL" |
| `INT64` `NUMERIC` `BIGNUMERIC` | The number in string format with the following rules: - Positive numbers are preceded with `_`. - Negative numbers are preceded with `minus_`. - A decimal point is replaced with `_point_`. | Input: 1 Output: _1 *** ** * ** *** Input: -1 Output: minus_1 *** ** * ** *** Input: 1.0 Output: _1_point_0 |
| BOOL | `TRUE` or `FALSE`. | Input: TRUE Output: TRUE *** ** * ** *** Input: FALSE Output: FALSE |
| STRING | The string value. | Input: "PlayerName" Output: PlayerName |
| DATE | The date in `_YYYY_MM_DD` format. | Input: DATE '2013-11-25' Output: _2013_11_25 |
| ENUM | The name of the enumeration constant. | Input: COLOR.RED Output: RED |
| STRUCT | A string formed by computing the `pivot_column` name for each field and joining the results together with an underscore. The following rules apply: - If the field is named: `<field_name>_<pivot_column_name_for_field_name>`. - If the field is unnamed: `<pivot_column_name_for_field_name>`. `<pivot_column_name_for_field_name>` is determined by applying the rules in this table, recursively. If no rule is available for any `STRUCT` field, the entire pivot column is unnamed. Due to implicit type coercion from the `IN` list values to the type of `<value-expression>`, field names must be present in `input_column` to have an effect on the names of the pivot columns. | Input: STRUCT("one", "two") Output: one_two *** ** * ** *** Input: STRUCT("one" AS a, "two" AS b) Output: one_a_two_b |
| All other data types | Not supported. You must provide an alias. |   |

**Examples**

The following examples reference a table called `Produce` that looks like this:

    WITH Produce AS (
      SELECT 'Kale' as product, 51 as sales, 'Q1' as quarter, 2020 as year UNION ALL
      SELECT 'Kale', 23, 'Q2', 2020 UNION ALL
      SELECT 'Kale', 45, 'Q3', 2020 UNION ALL
      SELECT 'Kale', 3, 'Q4', 2020 UNION ALL
      SELECT 'Kale', 70, 'Q1', 2021 UNION ALL
      SELECT 'Kale', 85, 'Q2', 2021 UNION ALL
      SELECT 'Apple', 77, 'Q1', 2020 UNION ALL
      SELECT 'Apple', 0, 'Q2', 2020 UNION ALL
      SELECT 'Apple', 1, 'Q1', 2021)
    SELECT * FROM Produce

    /*---+---+---+---+
     | product | sales | quarter | year |
     +---+---+---+---|
     | Kale    | 51    | Q1      | 2020 |
     | Kale    | 23    | Q2      | 2020 |
     | Kale    | 45    | Q3      | 2020 |
     | Kale    | 3     | Q4      | 2020 |
     | Kale    | 70    | Q1      | 2021 |
     | Kale    | 85    | Q2      | 2021 |
     | Apple   | 77    | Q1      | 2020 |
     | Apple   | 0     | Q2      | 2020 |
     | Apple   | 1     | Q1      | 2021 |
     +---+---+---+---*/

With the `PIVOT` operator, the rows in the `quarter` column are rotated into
these new columns: `Q1`, `Q2`, `Q3`, `Q4`. The aggregate function `SUM` is
implicitly grouped by all unaggregated columns other than the `pivot_column`:
`product` and `year`.

    SELECT * FROM
      Produce
      PIVOT(SUM(sales) FOR quarter IN ('Q1', 'Q2', 'Q3', 'Q4'))

    /*---+---+---+---+---+---+
     | product | year | Q1 | Q2   | Q3   | Q4   |
     +---+---+---+---+---+---+
     | Apple   | 2020 | 77 | 0    | NULL | NULL |
     | Apple   | 2021 | 1  | NULL | NULL | NULL |
     | Kale    | 2020 | 51 | 23   | 45   | 3    |
     | Kale    | 2021 | 70 | 85   | NULL | NULL |
     +---+---+---+---+---+---*/

If you don't include `year`, then `SUM` is grouped only by `product`.

    SELECT * FROM
      (SELECT product, sales, quarter FROM Produce)
      PIVOT(SUM(sales) FOR quarter IN ('Q1', 'Q2', 'Q3', 'Q4'))

    /*---+---+---+---+---+
     | product | Q1  | Q2  | Q3   | Q4   |
     +---+---+---+---+---+
     | Apple   | 78  | 0   | NULL | NULL |
     | Kale    | 121 | 108 | 45   | 3    |
     +---+---+---+---+---*/

You can select a subset of values in the `pivot_column`:

    SELECT * FROM
      (SELECT product, sales, quarter FROM Produce)
      PIVOT(SUM(sales) FOR quarter IN ('Q1', 'Q2', 'Q3'))

    /*---+---+---+---+
     | product | Q1  | Q2  | Q3   |
     +---+---+---+---+
     | Apple   | 78  | 0   | NULL |
     | Kale    | 121 | 108 | 45   |
     +---+---+---+---*/

    SELECT * FROM
      (SELECT sales, quarter FROM Produce)
      PIVOT(SUM(sales) FOR quarter IN ('Q1', 'Q2', 'Q3'))

    /*---+---+---+
     | Q1  | Q2  | Q3 |
     +---+---+---+
     | 199 | 108 | 45 |
     +---+---+---*/

You can include multiple aggregation functions in the `PIVOT`. In this case, you
must specify an alias for each aggregation. These aliases are used to construct
the column names in the resulting table.

    SELECT * FROM
      (SELECT product, sales, quarter FROM Produce)
      PIVOT(SUM(sales) AS total_sales, COUNT(*) AS num_records FOR quarter IN ('Q1', 'Q2'))

    /*---+---+---+---+---+
     |product | total_sales_Q1 | num_records_Q1 | total_sales_Q2 | num_records_Q2 |
     +---+---+---+---+---+
     | Kale   | 121            | 2              | 108            | 2              |
     | Apple  | 78             | 2              | 0              | 1              |
     +---+---+---+---+---*/

## `UNPIVOT` operator

```
FROM from_item[, ...] unpivot_operator

unpivot_operator:
  UNPIVOT [ { INCLUDE NULLS | EXCLUDE NULLS } ] (
    { single_column_unpivot | multi_column_unpivot }
  ) [unpivot_alias]

single_column_unpivot:
  values_column
  FOR name_column
  IN (columns_to_unpivot)

multi_column_unpivot:
  values_column_set
  FOR name_column
  IN (column_sets_to_unpivot)

values_column_set:
  (values_column[, ...])

columns_to_unpivot:
  unpivot_column [row_value_alias][, ...]

column_sets_to_unpivot:
  (unpivot_column [row_value_alias][, ...])

unpivot_alias and row_value_alias:
  [AS] alias
```

The `UNPIVOT` operator rotates columns into rows. `UNPIVOT` is part of the
`FROM` clause.

- `UNPIVOT` can be used to modify any table expression.
- Combining `UNPIVOT` with `FOR SYSTEM_TIME AS OF` isn't allowed, although users may use `UNPIVOT` against a subquery input which itself uses `FOR SYSTEM_TIME AS OF`.
- A `WITH OFFSET` clause immediately preceding the `UNPIVOT` operator isn't allowed.
- `PIVOT` aggregations can't be reversed with `UNPIVOT`.

Conceptual example:

    -- Before UNPIVOT is used to rotate Q1, Q2, Q3, Q4 into sales and quarter columns:
    /*---+---+---+---+---+
     | product | Q1 | Q2 | Q3 | Q4 |
     +---+---+---+---+---+
     | Kale    | 51 | 23 | 45 | 3  |
     | Apple   | 77 | 0  | 25 | 2  |
     +---+---+---+---+---*/

    -- After UNPIVOT is used to rotate Q1, Q2, Q3, Q4 into sales and quarter columns:
    /*---+---+---+
     | product | sales | quarter |
     +---+---+---+
     | Kale    | 51    | Q1      |
     | Kale    | 23    | Q2      |
     | Kale    | 45    | Q3      |
     | Kale    | 3     | Q4      |
     | Apple   | 77    | Q1      |
     | Apple   | 0     | Q2      |
     | Apple   | 25    | Q3      |
     | Apple   | 2     | Q4      |
     +---+---+---*/

**Definitions**

Top-level definitions:

- `from_item`: The table, subquery, or table-valued function (TVF) on which to perform a pivot operation. The `from_item` must [follow these rules](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#rules_for_unpivot_from_item).
- `unpivot_operator`: The pivot operation to perform on a `from_item`.

`unpivot_operator` definitions:

- `INCLUDE NULLS`: Add rows with `NULL` values to the result.
- `EXCLUDE NULLS`: don't add rows with `NULL` values to the result. By default, `UNPIVOT` excludes rows with `NULL` values.
- `single_column_unpivot`: Rotates columns into one `values_column` and one `name_column`.
- `multi_column_unpivot`: Rotates columns into multiple `values_column`s and one `name_column`.
- `unpivot_alias`: An alias for the results of the `UNPIVOT` operation. This alias can be referenced elsewhere in the query.

`single_column_unpivot` definitions:

- `values_column`: A column to contain the row values from `columns_to_unpivot`. [Follow these rules](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#rules_for_values_column) when creating a values column.
- `name_column`: A column to contain the column names from `columns_to_unpivot`. [Follow these rules](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#rules_for_name_column) when creating a name column.
- `columns_to_unpivot`: The columns from the `from_item` to populate `values_column` and `name_column`. [Follow these rules](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#rules_for_unpivot_column) when creating an unpivot column.
  - `row_value_alias`: An optional alias for a column that's displayed for the column in `name_column`. If not specified, the string value of the column name is used. [Follow these rules](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#rules_for_row_value_alias) when creating a row value alias.

`multi_column_unpivot` definitions:

- `values_column_set`: A set of columns to contain the row values from `columns_to_unpivot`. [Follow these rules](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#rules_for_values_column) when creating a values column.
- `name_column`: A set of columns to contain the column names from `columns_to_unpivot`. [Follow these rules](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#rules_for_name_column) when creating a name column.
- `column_sets_to_unpivot`: The columns from the `from_item` to unpivot. [Follow these rules](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#rules_for_unpivot_column) when creating an unpivot column.
  - `row_value_alias`: An optional alias for a column set that's displayed for the column set in `name_column`. If not specified, a string value for the column set is used and each column in the string is separated with an underscore (`_`). For example, `(col1, col2)` outputs `col1_col2`. [Follow these rules](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#rules_for_row_value_alias) when creating a row value alias.

**Rules**


Rules for a `from_item` passed to `UNPIVOT`:

- The `from_item` may consist of any table, subquery, or table-valued function (TVF) result.
- The `from_item` may not produce a value table.
- Duplicate columns in a `from_item` can't be referenced in the `UNPIVOT` clause.


Rules for `unpivot_operator`:

- Expressions aren't permitted.
- Qualified names aren't permitted. For example, `mytable.mycolumn` isn't allowed.
- In the case where the `UNPIVOT` result has duplicate column names:
  - `SELECT *` is allowed.
  - `SELECT values_column` causes ambiguity.


Rules for `values_column`:

- It can't be a name used for a `name_column` or an `unpivot_column`.
- It can be the same name as a column from the `from_item`.


Rules for `name_column`:

- It can't be a name used for a `values_column` or an `unpivot_column`.
- It can be the same name as a column from the `from_item`.


Rules for `unpivot_column`:

- Must be a column name from the `from_item`.
- It can't reference duplicate `from_item` column names.
- All columns in a column set must have equivalent data types.
  - Data types can't be coerced to a common supertype.
  - If the data types are exact matches (for example, a struct with different field names), the data type of the first input is the data type of the output.
- You can't have the same name in the same column set. For example, `(emp1, emp1)` results in an error.
- You can have a the same name in different column sets. For example, `(emp1, emp2), (emp1, emp3)` is valid.


Rules for `row_value_alias`:

- This can be a string or an `INT64` literal.
- The data type for all `row_value_alias` clauses must be the same.
- If the value is an `INT64`, the `row_value_alias` for each `unpivot_column` must be specified.

**Examples**

The following examples reference a table called `Produce` that looks like this:

    WITH Produce AS (
      SELECT 'Kale' as product, 51 as Q1, 23 as Q2, 45 as Q3, 3 as Q4 UNION ALL
      SELECT 'Apple', 77, 0, 25, 2)
    SELECT * FROM Produce

    /*---+---+---+---+---+
     | product | Q1 | Q2 | Q3 | Q4 |
     +---+---+---+---+---+
     | Kale    | 51 | 23 | 45 | 3  |
     | Apple   | 77 | 0  | 25 | 2  |
     +---+---+---+---+---*/

With the `UNPIVOT` operator, the columns `Q1`, `Q2`, `Q3`, and `Q4` are
rotated. The values of these columns now populate a new column called `Sales`
and the names of these columns now populate a new column called `Quarter`.
This is a single-column unpivot operation.

    SELECT * FROM Produce
    UNPIVOT(sales FOR quarter IN (Q1, Q2, Q3, Q4))

    /*---+---+---+
     | product | sales | quarter |
     +---+---+---+
     | Kale    | 51    | Q1      |
     | Kale    | 23    | Q2      |
     | Kale    | 45    | Q3      |
     | Kale    | 3     | Q4      |
     | Apple   | 77    | Q1      |
     | Apple   | 0     | Q2      |
     | Apple   | 25    | Q3      |
     | Apple   | 2     | Q4      |
     +---+---+---*/

In this example, we `UNPIVOT` four quarters into two semesters.
This is a multi-column unpivot operation.

    SELECT * FROM Produce
    UNPIVOT(
      (first_half_sales, second_half_sales)
      FOR semesters
      IN ((Q1, Q2) AS 'semester_1', (Q3, Q4) AS 'semester_2'))

    /*---+---+---+---+
     | product | first_half_sales | second_half_sales | semesters  |
     +---+---+---+---+
     | Kale    | 51               | 23                | semester_1 |
     | Kale    | 45               | 3                 | semester_2 |
     | Apple   | 77               | 0                 | semester_1 |
     | Apple   | 25               | 2                 | semester_2 |
     +---+---+---+---*/

## `TABLESAMPLE` operator

```
TABLESAMPLE SYSTEM ( percent PERCENT )
```

**Description**

You can use the `TABLESAMPLE` operator to select a random sample of a dataset.
This operator is useful when you're working with tables that have large
amounts of data and you don't need precise answers.

Sampling returns a variety of records while avoiding the costs associated with
scanning and processing an entire table. Each execution of the query might
return different results because each execution processes an independently
computed sample. GoogleSQL doesn't cache the results of queries that
include a `TABLESAMPLE` clause.

Replace `percent` with the percentage of the dataset that you want to include in
the results. The value must be between `0` and `100`. The value can be a literal
value or a query parameter. It can't be a variable.

For more information, see [Table sampling](https://docs.cloud.google.com/bigquery/docs/table-sampling).

**Example**

The following query selects approximately 10% of a table's data:

    SELECT * FROM dataset.my_table TABLESAMPLE SYSTEM (10 PERCENT)

## `MATCH_RECOGNIZE` clause

```
FROM from_item
MATCH_RECOGNIZE (
  [ PARTITION BY partition_expr [, ... ] ]
  ORDER BY order_expr [{ ASC | DESC }] [{ NULLS FIRST | NULLS LAST }] [, ...]
  MEASURES { measures_expr [AS] alias } [, ... ]
  [ AFTER MATCH SKIP { PAST LAST ROW | TO NEXT ROW } ]
  PATTERN (pattern)
  DEFINE symbol AS boolean_expr [, ... ]
  [ OPTIONS ( [ use_longest_match = { TRUE | FALSE } ] ) ]
)
```

**Description**

The `MATCH_RECOGNIZE` clause is an optional sub-clause of the `FROM` clause,
used to filter and aggregate based on matches. A *match* is an ordered sequence
of rows that match a pattern that you specify.
Matching rows works similarly to matching with regular expressions, but
instead of matching characters in a string, the `MATCH_RECOGNIZE` clause finds
matches across rows in a table.

**Definitions**

- `from_item`: The data over which the `MATCH_RECOGNIZE` clause operates.
- `partition_expr`: An expression for how to partition the input. The expression can't contain floating point types, non-groupable types, constants, or window functions.
- `order_expr`: An orderable column or expression used to order the rows before matching.
- `measures_expr`: An aggregate expression that is evaluated per partition and match. The expression can reference columns and symbols.
- `alias`: The output column name for `measures_expr`.
- `PAST LAST ROW`: Don't allow overlapping matches. The next match must start from one or more rows past the last row in the previous match. The default value for `AFTER MATCH SKIP` is `PAST LAST ROW`.
- `TO NEXT ROW`: Allow overlapping matches. The next match can begin one row after the start of the previous match. Multiple matches that start at the same row aren't allowed.
- `pattern`: A sequence of symbols and [pattern elements](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#match_recognize_pattern) that defines a match.
- `symbol`: A name given to a boolean expression used to construct the pattern.
- `boolean_expr`: A boolean expression for a symbol that determines whether a row matches that symbol.
- `use_longest_match`: If true, then the chosen match starting from any given row is the match that includes the most rows. The default value is `FALSE`. For more information, see [Match disambiguation rules](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#match_disambiguation).

**Output**

When you use the `MATCH_RECOGNIZE` clause, input data is transformed
in the following way:

If the `PARTITION BY` clause is present, then the output is the following:

- There is one row for each match in each partition. Partitions with no matches don't generate any rows.
- There is one column for each expression in the `PARTITION BY` clause, and one column for each expression in the `MEASURES` clause.

If the `PARTITION BY` clause is omitted, then the output is the following:

- There is one row for each match.
- There is one column for each expression in the `MEASURES` clause.

### `PARTITION BY` clause

The `PARTITION BY` clause specifies a list of expressions that are used
to partition the input rows for pattern
matching. Each partition is sorted according to the `ORDER BY` clause and
matches must be entirely contained within a partition.
If the `PARTITION BY` clause is omitted,
then the entire input table belongs to a single partition.

You can't introduce an alias for a partition expression. If the expression is a
single column name, then that name is used as the column name in the output.
Otherwise, the output column for that partition is anonymous.

### `ORDER BY` clause

The `ORDER BY` clause within a `MATCH_RECOGNIZE` clause orders the
rows of the input for pattern matching and
follows the same rules as the standard [`ORDER BY` clause](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#order_by_clause).
The data type of each
expression must be [orderable](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#orderable_data_types).
If all expressions in the `ORDER BY`
clause are tied, then the rows may be ordered arbitrarily.

### `MEASURES` clause

The `MEASURES` clause lists which aggregate expressions to compute for each
match, subject to the following rules:

- You must provide an alias for each aggregate expression.
- Aggregate expressions must aggregate any columns that they reference, except for columns in the `PARTITION BY` clause, which are guaranteed to have the same value for every row in a match.
- To perform aggregation only on rows that matched a particular symbol, use the syntax `symbol_name.column_name`. You can't use a symbol without a column reference. You can't reference multiple symbols within the same aggregation function.
- A single row can match at most one symbol within a pattern. If a row satisfies the boolean expressions of multiple symbols in the `DEFINE` clause, only the symbol that that takes precedence can contribute to an aggregation expression. For more information about how to determine which symbol counts towards a match, see [match disambiguation](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#match_disambiguation).

#### Order-sensitive aggregate functions

If an aggregate expression contains a function that depends on the order of
input, such as `ARRAY_AGG` or `STRING_AGG`, the inputs are ordered according
to the `ORDER BY` clause of the `MATCH_RECOGNIZE` clause, unless they include
the keyword `DISTINCT` or an explicit ordering.
If you explicitly specify an ordering within an
aggregate function call, such as `ARRAY_AGG(x ORDER BY x)`, then the function
uses the order that you specified.

#### `MEASURES` clause functions

In addition to the standard aggregate functions, you can use the following
functions in the `MEASURES` clause:

- `FIRST(x)`: Returns the value of `x` in the first row of the match, or `NULL` if the match is empty.
- `LAST(x)`: Returns the value of `x` in the last row of the match, or `NULL` if the match is empty.
- `MATCH_NUMBER()`: Returns the integer rank of the match, relative to other matches in the same partition, starting at 1. Matches are ranked according to the order of their starting rows using the `ORDER BY` clause. This function can be used with or without aggregation, because it returns the same value for every row in a match.
- `MATCH_ROW_NUMBER()`: Returns an integer specifying the row number of the current row, within the current match, starting at 1. The results of this function must be aggregated.
- `CLASSIFIER()`: Returns a `STRING` value for each row in the match that indicates the name of the symbol matched by the row. The results of this function must be aggregated.

The following SQL snippet references a table with columns called `sales`,
`sale_date`, and `customer`, and
shows examples of what is and isn't allowed in the `MEASURES` clause:

    PARTITION BY customer
    ORDER BY sale_date
    MEASURES
      # Error. No alias.
      SUM(sales),

      # Correct. You don't need to aggregate a partition column.
      customer AS customer_name,

      # Error. You must aggregate all non-partition columns.
      sales AS customer_sales,

      # Correct. You can aggregate table columns.
      SUM(sales) AS total_sales,

      # Correct. Uses the symbol.column syntax.
      SUM(low_sales.sales) AS total_low_sales,

      # Correct. Multiple symbols appear in separate aggregations.
      SUM(low_sales.sales) + SUM(high_sales.sales) AS combined_low_high,

      # Error. Multiple symbols appear in a single aggregation.
      SUM(low_sales.sales + high_sales.sales) AS bad_combined_sales,

      # Error. A single aggregation can't combine symbol-qualified and
      # non-symbol-qualified terms.
      SUM(low_sales.sales + sales)
    PATTERN (low_sales+ high_sales*)
    DEFINE
      low_sales AS sales < 100,
      high_sales AS sales > 200

### `PATTERN` clause

The `PATTERN` clause specifies a pattern to match. A pattern is a sequence of
symbols and operators. Adjacent symbols within a pattern must be separated by a
space or grouped separately using parentheses. Pattern matches are computed
based on the order specified in the `ORDER BY` clause. Each pattern match
must appear entirely within a partition. Patterns support the
following elements, listed in order of precedence:

| Element | Description |
|---|---|
| `(<pattern>)` | Matches `<pattern>`. Parentheses indicate grouping. |
| `<symbol>` | Matches a single row such that the associated expression in the `DEFINE` clause evaluates to `TRUE` for that row. See the [`DEFINE` clause](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#match_recognize_define) for details. |
| `()` | Matches an empty row sequence unconditionally. |
| `^` | Matches an empty row sequence, only before the first row of input. |
| `$` | Matches an empty row sequence, only after the last row of input. |
| `<pattern>?` | Matches `<pattern>` zero or one times; prefer once. |
| `<pattern>??` | Matches `<pattern>` zero or one times; prefer zero. |
| `<pattern>*` | Matches `<pattern>` zero or more times; prefer more. |
| `<pattern>*?` | Matches `<pattern>` zero or more times; prefer fewer. |
| `<pattern>+` | Matches `<pattern>` one or more times; prefer more. |
| `<pattern>+?` | Matches `<pattern>` one or more times; prefer fewer. |
| `<pattern>{n}` | Matches `<pattern>` exactly n times. |
| `<pattern>{m,}` | Matches `<pattern>` at least `m` times; prefer more. |
| `<pattern>{m,}?` | Matches `<pattern>` at least `m` times; prefer fewer. |
| `<pattern>{,n}` | Matches `<pattern>` at most `n` times; prefer more. |
| `<pattern>{,n}?` | Matches `<pattern>` at most `n` times; prefer fewer. |
| `<pattern>{m,n}` | Matches `<pattern>` between `m` and `n` times, inclusive; prefer more. |
| `<pattern>{m,n}?` | Matches `<pattern>` between `m` and `n` times, inclusive; prefer fewer. |
| `<pattern1> <pattern2>` | Matches `<pattern1>`, followed by `<pattern2>`. |
| `<pattern1> | <pattern2>` | Matches either `<pattern1>` or `<pattern2>`; prefer `<pattern1>`. |
| `<pattern> | ` | Matches either `<pattern>` or an empty row sequence; prefer `<pattern>`. This is equivalent to `<pattern>?`. |
| `| <pattern>` | Matches either `<pattern>` or an empty row sequence; prefer the empty row sequence. This is equivalent to `<pattern>??`. |

The values of `m` and `n` must be non-null integer
literals or query parameters between
0 and 10,000. If a quantifier contains an upper and lower bound, such as
`{m,n}`, then the upper bound can't be less than the lower bound.

### `DEFINE` clause

The `DEFINE` clause lists all symbols used in the pattern. Each symbol is
defined by a boolean expression. The symbol can
match a row if the expression evaluates to `TRUE` for that row.
Every symbol must appear at least once in the
`PATTERN` clause.

A single row can
match at most one symbol within a match, even if it satisfies the boolean
expressions of multiple symbols in the `DEFINE` clause. Multiple matches
can't start at the same row. For more information, see
[match disambiguation](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#match_disambiguation).

For example, you can use the following pattern and symbols in
a `MATCH_RECOGNIZE` clause to
match pairs of adjacent rows in which the value of `sales` is less than 100 in
the first row and greater than 200 in the following row:

    PATTERN (low_sales high_sales)
    DEFINE
      low_sales AS sales < 100,
      high_sales AS sales > 200

The following example matches one or more rows with sales less than 100,
followed by at most one row with sales greater than 200, followed
by any number of rows with sales less than 100:

    PATTERN (low_sales+ high_sales? low_sales*)
    DEFINE
      low_sales AS sales < 100,
      high_sales AS sales > 200

There are two functions unique to the `DEFINE` clause, `PREV()` and `NEXT()`,
that you can use to define a symbol in relation to rows around the
current row. Each function accepts a column name and, optionally, a nonnegative
integer indicating how many rows away to look:

- `PREV(column_name [, num_rows])`: Returns the value of `column_name` that occurs `num_rows` before the current row, or `NULL` if that row doesn't exist. The default value of `num_rows` is 1.
- `NEXT(column_name [, num_rows])`: Returns the value of `column_name` that occurs `num_rows` after the current row, or `NULL` if that row doesn't exist. The default value of `num_rows` is 1.

You can't use navigation functions, such as `LEAD` or `LAG`, to reference other
row values.

The following example matches sequences of three rows in which the value of `x`
increases and then decreases:

    PATTERN (incline peak decline)
    DEFINE
      incline AS x < NEXT(x),
      peak AS PREV(x) < x AND x > NEXT(x),
      decline AS PREV(x) > x

### Match disambiguation

When multiple matches exist with the same starting row, only one is selected
according to the following rules:

1. Longest match mode.

   - If the `OPTIONS` clause is present and `use_longest_match` is `TRUE`, then the chosen match is the one that includes the most rows. In case of a tie, the chosen match is decided by operator preference.
   - If the `OPTIONS` clause is not present or `use_longest_match` is `FALSE`, then the match is chosen based on operator preference.
2. Operator preference.

   - The `|` operator gives preference to the left operand over the right operand.
   - Greedy quantifiers, which include `?`, `*`, `+`, `{m,}`, `{,n}`, and `{m,n}`, give preference to matches that repeat the operand more times over matches that repeat it fewer times.
   - Reluctant quantifiers, which include `??`, `*?`, `{m,}?`, `{,n}?`, and `{m,n}?`, give preference to matches that repeat the operand fewer times over matches that repeat it more times.

When operator preferences in different parts of the pattern conflict with each
other, preferences that occur earlier in the match take priority over those that
occur later in the match. For example, the pattern `A* B+` first prioritizes
increasing the number of rows that match the `A` symbol, even if that
results in fewer rows that match the `B` symbol. In other words, `AAB` is
chosen over `ABB`.

The pattern `(A|B B)+` prioritizes starting the match with `AB` rather than
`BB`, even if that results in fewer repetitions overall.

**Examples**

In the following example, pattern matches are single rows. The pattern matches
the `low` symbol if `x` is less than or equal to 2. The pattern matches the
`high` symbol if `x` is greater than or equal to 2. There are three matches,
one for each row:

- The first row of input only satisfies the `low` symbol expression, so `x = 1` contributes to the `low_agg` aggregation but not the `high_agg` aggregation.
- In the second row of input, `x` satisfies both symbol expressions, but the `low` symbol takes precedence because in the `|` operator, preference is given to the symbol on the left. Therefore, `x = 2` contributes only to the `low_agg` aggregation.
- The third row of input only satisfies the `high` symbol expression, so `x = 3` contributes to the `high_agg` aggregation but not the `low_agg` aggregation.

    SELECT * FROM (
      SELECT 1 as x
      UNION ALL
      SELECT 2
      UNION ALL
      SELECT 3
    )
    MATCH_RECOGNIZE(
      ORDER BY x
      MEASURES
        ARRAY_AGG(high.x) AS high_agg,
        ARRAY_AGG(low.x) AS low_agg
      AFTER MATCH SKIP TO NEXT ROW
      PATTERN (low | high)
      DEFINE
        low AS x <= 2,
        high AS x >= 2
    );

    /*---+---+
     | high_agg | low_agg |
     +---+---+
     | NULL     | [1]     |
     | NULL     | [2]     |
     | [3]      | NULL    |
     +---+---*/

The following example is similar to the preceding example, except that the
pattern is changed to `low | high+` and the `use_longest_match` option is set to
`TRUE`. There are three potential pattern matches that start at the second row
of input:

1. Match the second row using the `low` symbol.
2. Match the second row using the `high` symbol.
3. Match the second row using the `high` symbol and the third row using the `high` symbol.

The third option is chosen because it's strictly longest. Since there is no
tie for length, operator preference isn't considered.

    SELECT * FROM (
      SELECT 1 as x
      UNION ALL
      SELECT 2
      UNION ALL
      SELECT 3
    )
    MATCH_RECOGNIZE(
      ORDER BY x
      MEASURES
        ARRAY_AGG(high.x) AS high_agg,
        ARRAY_AGG(low.x) AS low_agg
      AFTER MATCH SKIP TO NEXT ROW
      PATTERN (low | high+)
      DEFINE
        low AS x <= 2,
        high AS x >= 2
      OPTIONS (use_longest_match = TRUE)
    );

    /*---+---+
     | high_agg | low_agg |
     +---+---+
     | NULL     | [1]     |
     | [2,3]    | NULL    |
     | [3]      | NULL    |
     +---+---*/

The following examples reference a table called `Sales`:

    WITH Sales AS (
      SELECT 'Daisy' AS customer, DATE '2024-01-03' AS sale_date, 'Electronics' AS product_category, 500 AS amount UNION ALL
      SELECT 'Daisy', DATE '2024-01-04', 'Software', 30 UNION ALL
      SELECT 'Ian', DATE '2024-02-01', 'Books', 20 UNION ALL
      SELECT 'Ian', DATE '2024-02-08', 'Clothing', 30 UNION ALL
      SELECT 'Ian', DATE '2024-02-10', 'Clothing', 90 UNION ALL
      SELECT 'Daisy', DATE '2024-03-15', 'Software', 40 UNION ALL
      SELECT 'Ian', DATE '2024-03-15', 'Electronics', 300 UNION ALL
      SELECT 'Ian', DATE '2024-03-15', 'Electronics', 400 UNION ALL
      SELECT 'Ian', DATE '2024-03-21', 'Software', 30 UNION ALL
      SELECT 'Ian', DATE '2024-04-07', 'Books', 50 UNION ALL
      SELECT 'Daisy', DATE '2024-06-28', 'Electronics', 400 UNION ALL
      SELECT 'Daisy', DATE '2024-06-29', 'Clothing', 100 UNION ALL
      SELECT 'Daisy', DATE '2024-06-30', 'Software', 30 UNION ALL
      SELECT 'Ian', DATE '2024-07-07', 'Clothing', 110
    )
    SELECT * FROM Sales;

    /*---+---+---+---+
     | customer | sale_date  | product_category | amount |
     +---+---+---+---+
     | Daisy    | 2024-01-03 | Electronics      | 500    |
     | Daisy    | 2024-01-04 | Software         | 30     |
     | Ian      | 2024-02-01 | Books            | 20     |
     | Ian      | 2024-02-08 | Clothing         | 30     |
     | Ian      | 2024-02-10 | Clothing         | 90     |
     | Daisy    | 2024-03-15 | Software         | 40     |
     | Ian      | 2024-03-15 | Electronics      | 300    |
     | Ian      | 2024-03-15 | Electronics      | 400    |
     | Ian      | 2024-03-21 | Software         | 30     |
     | Ian      | 2024-04-07 | Books            | 50     |
     | Daisy    | 2024-06-28 | Electronics      | 400    |
     | Daisy    | 2024-06-29 | Clothing         | 100    |
     | Daisy    | 2024-06-30 | Software         | 30     |
     | Ian      | 2024-07-07 | Clothing         | 110    |
     +---+---+---+---*/

The following example finds electronics purchases, followed by any number of
other purchases of other types, followed by software purchases. The `MEASURES`
clause aggregates
the data in each match and computes total sales and software sales:

    SELECT *
    FROM
      Sales
    MATCH_RECOGNIZE (
      PARTITION BY customer
      ORDER BY sale_date
      MEASURES
        ARRAY_AGG(STRUCT(sale_date, product_category, amount)) AS sales,
        SUM(amount) AS total_sale_amount,
        SUM(software.amount) AS software_sale_amount
      PATTERN (electronics+ any_category*? software+)
      DEFINE
        electronics AS product_category = 'Electronics',
        software AS product_category = 'Software',
        any_category AS TRUE
    );

    /*---+---+---+---+---+---+
     | customer | sales.sale_date | sales.product_category | sales.amount | total_sale_amount | software_sale_amount |
     +---+---+---+---+---+---+
     | Daisy    | 2024-01-03      | Electronics            | 500          | 570               | 70                   |
     |          | 2024-01-04      | Software               | 30           |                   |                      |
     |          | 2024-03-15      | Software               | 40           |                   |                      |
     | Daisy    | 2024-06-28      | Electronics            | 400          | 530               | 30                   |
     |          | 2024-06-29      | Clothing               | 100          |                   |                      |
     |          | 2024-06-30      | Software               | 30           |                   |                      |
     | Ian      | 2024-03-15      | Electronics            | 300          | 730               | 30                   |
     |          | 2024-03-15      | Electronics            | 400          |                   |                      |
     |          | 2024-03-21      | Software               | 30           |                   |                      |
     +---+---+---+---+---+---*/

The following example, like the previous example, matches electronics
purchases that were eventually followed by software purchases. The query
uses the `LAST` and `FIRST` functions that are unique
to the `MEASURES` clause to compute the number of days between the final
electronics purchase in a match and the first following software purchase:

    SELECT *
    FROM
      Sales
    MATCH_RECOGNIZE (
      PARTITION BY customer
      ORDER BY sale_date
      MEASURES
        LAST(electronics.sale_date) AS last_electronics_sale_date,
        FIRST(software.sale_date) AS first_software_sale_date,
        DATE_DIFF(FIRST(software.sale_date), LAST(electronics.sale_date), day) AS date_gap
      PATTERN (electronics+ any_category*? software+)
      DEFINE
        electronics AS product_category = 'Electronics',
        software AS product_category = 'Software',
        any_category AS TRUE
    );

    /*---+---+---+---+
     | customer | last_electronics_sale_date | first_software_sale_date | date_gap |
     +---+---+---+---+
     | Daisy    | 2024-01-03                 | 2024-01-04               | 1        |
     | Daisy    | 2024-06-28                 | 2024-06-30               | 2        |
     | Ian      | 2024-03-15                 | 2024-03-21               | 6        |
     +---+---+---+---*/

The following example matches a sequence of rows where `amount` is less than
50, followed by rows where `amount` is between 50 and 100, followed by rows
where `amount` is greater than 100. The query uses the
`MATCH_NUMBER`, `MATCH_ROW_NUMBER`, and `CLASSIFIER` functions in the `MEASURES`
clause to identify matches and their symbols in the results.

    SELECT *
    FROM
      Sales
    MATCH_RECOGNIZE (
      PARTITION BY customer
      ORDER BY sale_date
      MEASURES
        MATCH_NUMBER() AS match_number,
        ARRAY_AGG(STRUCT(MATCH_ROW_NUMBER() AS row,
                         CLASSIFIER() AS symbol,
                         sale_date,
                         product_category)) AS sales
      PATTERN (low+ mid+ high+)
      DEFINE
        low AS amount < 50,
        mid AS amount >= 50 AND amount <= 100,
        high AS amount > 100
    );

    /*---+---+---+---+---+---+
     | customer | match_number | sales.row | sales.symbol | sales.sale_date | sales.product_category |
     +---+---+---+---+---+---+
     | Ian      | 1            | 1         | low          | 2024-02-01      | Books                  |
     |          |              | 2         | low          | 2024-02-08      | Clothing               |
     |          |              | 3         | mid          | 2024-02-10      | Clothing               |
     |          |              | 4         | high         | 2024-03-15      | Electronics            |
     |          |              | 5         | high         | 2024-03-15      | Electronics            |
     | Ian      | 2            | 1         | low          | 2024-03-21      | Software               |
     |          |              | 2         | mid          | 2024-04-07      | Books                  |
     |          |              | 3         | high         | 2024-07-07      | Clothing               |
     +---+---+---+---+---+---*/

The following example is similar to the previous one, except it allows
overlapping matches:

    SELECT *
    FROM
      Sales
    MATCH_RECOGNIZE (
      PARTITION BY customer
      ORDER BY sale_date
      MEASURES
        MATCH_NUMBER() AS match_number,
        ARRAY_AGG(STRUCT(MATCH_ROW_NUMBER() AS row,
                         CLASSIFIER() AS symbol,
                         sale_date,
                         product_category)) AS sales
      AFTER MATCH SKIP TO NEXT ROW
      PATTERN (low+ mid+ high+)
      DEFINE
        low AS amount < 50,
        mid AS amount >= 50 AND amount <= 100,
        high AS amount > 100
    );

    /*---+---+---+---+---+---+
     | customer | match_number | sales.row | sales.symbol | sales.sale_date | sales.product_category |
     +---+---+---+---+---+---+
     | Ian      | 1            | 1         | low          | 2024-02-01      | Books                  |
     |          |              | 2         | low          | 2024-02-08      | Clothing               |
     |          |              | 3         | mid          | 2024-02-10      | Clothing               |
     |          |              | 4         | high         | 2024-03-15      | Electronics            |
     |          |              | 5         | high         | 2024-03-15      | Electronics            |
     | Ian      | 2            | 1         | low          | 2024-02-08      | Clothing               |
     |          |              | 2         | mid          | 2024-02-10      | Clothing               |
     |          |              | 3         | high         | 2024-03-15      | Electronics            |
     |          |              | 4         | high         | 2024-03-15      | Electronics            |
     | Ian      | 3            | 1         | low          | 2024-03-21      | Software               |
     |          |              | 2         | mid          | 2024-04-07      | Books                  |
     |          |              | 3         | high         | 2024-07-07      | Clothing               |
     +---+---+---+---+---+---*/

### Best practices

To scale the performance of queries that contain the `MATCH_RECOGNIZE` clause,
use the following best practices:

- Use the `PARTITION BY` clause.
  - Within each partition, rows are processed one at a time. Separate partitions can be processed in parallel, subject to slot availability.
  - The `MATCH_RECOGNIZE` clause generally runs faster when you use a larger number of smaller partitions, rather than fewer large partitions.
  - Individual partitions which are too large, typically exceeding 1 million rows, might cause an [`Out of memory` error](https://docs.cloud.google.com/bigquery/docs/troubleshoot-queries#query_not_executed).
- Use simple patterns.
  - Avoid patterns with a large number of operators.
  - Avoid bounded quantifiers with large bound values. For example, matching the patterns `A+` or `A*` is faster than matching `A{,100}`.
- Use the `ORDER BY` clause to guarantee the order in which the input rows are processed. Otherwise the `MATCH_RECOGNIZE` clause can produce non-deterministic results.

For more example queries, see the [`MATCH_RECOGNIZE` notebook tutorial](https://github.com/GoogleCloudPlatform/bigquery-utils/blob/master/notebooks/bigquery_match_recognize_demo.ipynb).

## `GRAPH_TABLE` operator

To learn more about this operator, see
[`GRAPH_TABLE` operator](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/graph-sql-queries#graph_table_operator) in the
Graph Query Language (GQL) reference guide.

## Join operation

```
join_operation:
  { cross_join_operation | condition_join_operation }

cross_join_operation:
  https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#from_clause cross_join_operator https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#from_clause

condition_join_operation:
  https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#from_clause condition_join_operator https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#from_clause join_condition

cross_join_operator:
  { CROSS JOIN | , }

condition_join_operator:
  {
    [INNER] JOIN
    | FULL [OUTER] JOIN
    | LEFT [OUTER] JOIN
    | RIGHT [OUTER] JOIN
  }

join_condition:
  { on_clause | using_clause }

on_clause:
  ON bool_expression

using_clause:
  USING ( column_list )
```

The `JOIN` operation merges two `from_item`s so that the `SELECT` clause can
query them as one source. The join operator and join condition specify how to
combine and discard rows from the two `from_item`s to form a single source.

### `[INNER] JOIN`

An `INNER JOIN`, or simply `JOIN`, effectively calculates the Cartesian product
of the two `from_item`s and discards all rows that don't meet the join
condition. *Effectively* means that it's possible to implement an `INNER JOIN`
without actually calculating the Cartesian product.

    FROM A INNER JOIN B ON A.w = B.y

    /*
    Table A       Table B       Result
    +---+     +---+     +---+
    | w | x |  *  | y | z |  =  | w | x | y | z |
    +---+     +---+     +---+
    | 1 | a |     | 2 | k |     | 2 | b | 2 | k |
    | 2 | b |     | 3 | m |     | 3 | c | 3 | m |
    | 3 | c |     | 3 | n |     | 3 | c | 3 | n |
    | 3 | d |     | 4 | p |     | 3 | d | 3 | m |
    +---+     +---+     | 3 | d | 3 | n |
                                +---+
    */

    FROM A INNER JOIN B USING (x)

    /*
    Table A       Table B       Result
    +---+     +---+     +---+
    | x | y |  *  | x | z |  =  | x | y | z |
    +---+     +---+     +---+
    | 1 | a |     | 2 | k |     | 2 | b | k |
    | 2 | b |     | 3 | m |     | 3 | c | m |
    | 3 | c |     | 3 | n |     | 3 | c | n |
    | 3 | d |     | 4 | p |     | 3 | d | m |
    +---+     +---+     | 3 | d | n |
                                +---+
    */

**Example**

This query performs an `INNER JOIN` on the [`Roster`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#roster_table)
and [`TeamMascot`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#teammascot_table) tables.

    SELECT Roster.LastName, TeamMascot.Mascot
    FROM Roster JOIN TeamMascot ON Roster.SchoolID = TeamMascot.SchoolID;

    /*---+
     | LastName   | Mascot       |
     +---+
     | Adams      | Jaguars      |
     | Buchanan   | Lakers       |
     | Coolidge   | Lakers       |
     | Davis      | Knights      |
     +---*/

You can use a [correlated](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#correlated_join) `INNER JOIN` to flatten an array
into a set of rows. To learn more, see
[Convert elements in an array to rows in a table](https://docs.cloud.google.com/bigquery/docs/arrays#flattening_arrays).

### `CROSS JOIN`

`CROSS JOIN` returns the Cartesian product of the two `from_item`s. In other
words, it combines each row from the first `from_item` with each row from the
second `from_item`.

If the rows of the two `from_item`s are independent, then the result has
*M \* N* rows, given *M* rows in one `from_item` and *N* in the other. Note that
this still holds for the case when either `from_item` has zero rows.

In a `FROM` clause, a `CROSS JOIN` can be written like this:

    FROM A CROSS JOIN B

    /*
    Table A       Table B       Result
    +---+     +---+     +---+
    | w | x |  *  | y | z |  =  | w | x | y | z |
    +---+     +---+     +---+
    | 1 | a |     | 2 | c |     | 1 | a | 2 | c |
    | 2 | b |     | 3 | d |     | 1 | a | 3 | d |
    +---+     +---+     | 2 | b | 2 | c |
                                | 2 | b | 3 | d |
                                +---+
    */

You can use a [correlated](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#correlated_join) cross join to convert or
flatten an array into a set of rows, though the (equivalent) `INNER JOIN` is
preferred over `CROSS JOIN` for this case. To learn more, see
[Convert elements in an array to rows in a table](https://docs.cloud.google.com/bigquery/docs/arrays#flattening_arrays).

**Examples**

This query performs an `CROSS JOIN` on the [`Roster`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#roster_table)
and [`TeamMascot`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#teammascot_table) tables.

    SELECT Roster.LastName, TeamMascot.Mascot
    FROM Roster CROSS JOIN TeamMascot;

    /*---+
     | LastName   | Mascot       |
     +---+
     | Adams      | Jaguars      |
     | Adams      | Knights      |
     | Adams      | Lakers       |
     | Adams      | Mustangs     |
     | Buchanan   | Jaguars      |
     | Buchanan   | Knights      |
     | Buchanan   | Lakers       |
     | Buchanan   | Mustangs     |
     | ...                       |
     +---*/

### Comma cross join (,)

[`CROSS JOIN`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#cross_join)s can be written implicitly with a comma. This is
called a comma cross join.

A comma cross join looks like this in a `FROM` clause:

    FROM A, B

    /*
    Table A       Table B       Result
    +---+     +---+     +---+
    | w | x |  *  | y | z |  =  | w | x | y | z |
    +---+     +---+     +---+
    | 1 | a |     | 2 | c |     | 1 | a | 2 | c |
    | 2 | b |     | 3 | d |     | 1 | a | 3 | d |
    +---+     +---+     | 2 | b | 2 | c |
                                | 2 | b | 3 | d |
                                +---+
    */

You can't write comma cross joins inside parentheses. To learn more, see
[Join operations in a sequence](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#sequences_of_joins).

    FROM (A, B)  // INVALID

You can use a [correlated](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#correlated_join) comma cross join to convert or
flatten an array into a set of rows. To learn more, see
[Convert elements in an array to rows in a table](https://docs.cloud.google.com/bigquery/docs/arrays#flattening_arrays).

**Examples**

This query performs a comma cross join on the [`Roster`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#roster_table)
and [`TeamMascot`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#teammascot_table) tables.

    SELECT Roster.LastName, TeamMascot.Mascot
    FROM Roster, TeamMascot;

    /*---+
     | LastName   | Mascot       |
     +---+
     | Adams      | Jaguars      |
     | Adams      | Knights      |
     | Adams      | Lakers       |
     | Adams      | Mustangs     |
     | Buchanan   | Jaguars      |
     | Buchanan   | Knights      |
     | Buchanan   | Lakers       |
     | Buchanan   | Mustangs     |
     | ...                       |
     +---*/

### `FULL [OUTER] JOIN`

A `FULL OUTER JOIN` (or simply `FULL JOIN`) returns all fields for all matching
rows in both `from_items` that meet the join condition. If a given row from one
`from_item` doesn't join to any row in the other `from_item`, the row returns
with `NULL` values for all columns from the other `from_item`.

    FROM A FULL OUTER JOIN B ON A.w = B.y

    /*
    Table A       Table B       Result
    +---+     +---+     +---+
    | w | x |  *  | y | z |  =  | w    | x    | y    | z    |
    +---+     +---+     +---+
    | 1 | a |     | 2 | k |     | 1    | a    | NULL | NULL |
    | 2 | b |     | 3 | m |     | 2    | b    | 2    | k    |
    | 3 | c |     | 3 | n |     | 3    | c    | 3    | m    |
    | 3 | d |     | 4 | p |     | 3    | c    | 3    | n    |
    +---+     +---+     | 3    | d    | 3    | m    |
                                | 3    | d    | 3    | n    |
                                | NULL | NULL | 4    | p    |
                                +---+
    */

    FROM A FULL OUTER JOIN B USING (x)

    /*
    Table A       Table B       Result
    +---+     +---+     +---+
    | x | y |  *  | x | z |  =  | x    | y    | z    |
    +---+     +---+     +---+
    | 1 | a |     | 2 | k |     | 1    | a    | NULL |
    | 2 | b |     | 3 | m |     | 2    | b    | k    |
    | 3 | c |     | 3 | n |     | 3    | c    | m    |
    | 3 | d |     | 4 | p |     | 3    | c    | n    |
    +---+     +---+     | 3    | d    | m    |
                                | 3    | d    | n    |
                                | 4    | NULL | p    |
                                +---+
    */

**Example**

This query performs a `FULL JOIN` on the [`Roster`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#roster_table)
and [`TeamMascot`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#teammascot_table) tables.

    SELECT Roster.LastName, TeamMascot.Mascot
    FROM Roster FULL JOIN TeamMascot ON Roster.SchoolID = TeamMascot.SchoolID;

    /*---+
     | LastName   | Mascot       |
     +---+
     | Adams      | Jaguars      |
     | Buchanan   | Lakers       |
     | Coolidge   | Lakers       |
     | Davis      | Knights      |
     | Eisenhower | NULL         |
     | NULL       | Mustangs     |
     +---*/

### `LEFT [OUTER] JOIN`

The result of a `LEFT OUTER JOIN` (or simply `LEFT JOIN`) for two
`from_item`s always retains all rows of the left `from_item` in the
`JOIN` operation, even if no rows in the right `from_item` satisfy the join
predicate.

All rows from the *left* `from_item` are retained;
if a given row from the left `from_item` doesn't join to any row
in the *right* `from_item`, the row will return with `NULL` values for all
columns exclusively from the right `from_item`. Rows from the right
`from_item` that don't join to any row in the left `from_item` are discarded.

    FROM A LEFT OUTER JOIN B ON A.w = B.y

    /*
    Table A       Table B       Result
    +---+     +---+     +---+
    | w | x |  *  | y | z |  =  | w    | x    | y    | z    |
    +---+     +---+     +---+
    | 1 | a |     | 2 | k |     | 1    | a    | NULL | NULL |
    | 2 | b |     | 3 | m |     | 2    | b    | 2    | k    |
    | 3 | c |     | 3 | n |     | 3    | c    | 3    | m    |
    | 3 | d |     | 4 | p |     | 3    | c    | 3    | n    |
    +---+     +---+     | 3    | d    | 3    | m    |
                                | 3    | d    | 3    | n    |
                                +---+
    */

    FROM A LEFT OUTER JOIN B USING (x)

    /*
    Table A       Table B       Result
    +---+     +---+     +---+
    | x | y |  *  | x | z |  =  | x    | y    | z    |
    +---+     +---+     +---+
    | 1 | a |     | 2 | k |     | 1    | a    | NULL |
    | 2 | b |     | 3 | m |     | 2    | b    | k    |
    | 3 | c |     | 3 | n |     | 3    | c    | m    |
    | 3 | d |     | 4 | p |     | 3    | c    | n    |
    +---+     +---+     | 3    | d    | m    |
                                | 3    | d    | n    |
                                +---+
    */

**Example**

This query performs a `LEFT JOIN` on the [`Roster`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#roster_table)
and [`TeamMascot`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#teammascot_table) tables.

    SELECT Roster.LastName, TeamMascot.Mascot
    FROM Roster LEFT JOIN TeamMascot ON Roster.SchoolID = TeamMascot.SchoolID;

    /*---+
     | LastName   | Mascot       |
     +---+
     | Adams      | Jaguars      |
     | Buchanan   | Lakers       |
     | Coolidge   | Lakers       |
     | Davis      | Knights      |
     | Eisenhower | NULL         |
     +---*/

### `RIGHT [OUTER] JOIN`

The result of a `RIGHT OUTER JOIN` (or simply `RIGHT JOIN`) for two
`from_item`s always retains all rows of the right `from_item` in the
`JOIN` operation, even if no rows in the left `from_item` satisfy the join
predicate.

All rows from the *right* `from_item` are returned;
if a given row from the right `from_item` doesn't join to any row
in the *left* `from_item`, the row will return with `NULL` values for all
columns exclusively from the left `from_item`. Rows from the left `from_item`
that don't join to any row in the right `from_item` are discarded.

    FROM A RIGHT OUTER JOIN B ON A.w = B.y

    /*
    Table A       Table B       Result
    +---+     +---+     +---+
    | w | x |  *  | y | z |  =  | w    | x    | y    | z    |
    +---+     +---+     +---+
    | 1 | a |     | 2 | k |     | 2    | b    | 2    | k    |
    | 2 | b |     | 3 | m |     | 3    | c    | 3    | m    |
    | 3 | c |     | 3 | n |     | 3    | c    | 3    | n    |
    | 3 | d |     | 4 | p |     | 3    | d    | 3    | m    |
    +---+     +---+     | 3    | d    | 3    | n    |
                                | NULL | NULL | 4    | p    |
                                +---+
    */

    FROM A RIGHT OUTER JOIN B USING (x)

    /*
    Table A       Table B       Result
    +---+     +---+     +---+
    | x | y |  *  | x | z |  =  | x    | y    | z    |
    +---+     +---+     +---+
    | 1 | a |     | 2 | k |     | 2    | b    | k    |
    | 2 | b |     | 3 | m |     | 3    | c    | m    |
    | 3 | c |     | 3 | n |     | 3    | c    | n    |
    | 3 | d |     | 4 | p |     | 3    | d    | m    |
    +---+     +---+     | 3    | d    | n    |
                                | 4    | NULL | p    |
                                +---+
    */

**Example**

This query performs a `RIGHT JOIN` on the [`Roster`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#roster_table)
and [`TeamMascot`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#teammascot_table) tables.

    SELECT Roster.LastName, TeamMascot.Mascot
    FROM Roster RIGHT JOIN TeamMascot ON Roster.SchoolID = TeamMascot.SchoolID;

    /*---+
     | LastName   | Mascot       |
     +---+
     | Adams      | Jaguars      |
     | Buchanan   | Lakers       |
     | Coolidge   | Lakers       |
     | Davis      | Knights      |
     | NULL       | Mustangs     |
     +---*/

### Join conditions

In a [join operation](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#join_types), a join condition helps specify how to
combine rows in two `from_items` to form a single source.

The two types of join conditions are the [`ON` clause](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#on_clause) and
[`USING` clause](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#using_clause). You must use a join condition when you perform a
conditional join operation. You can't use a join condition when you perform a
cross join operation.

#### `ON` clause

    ON bool_expression

**Description**

Given a row from each table, if the `ON` clause evaluates to `TRUE`, the query
generates a consolidated row with the result of combining the given rows.

Definitions:

- `bool_expression`: The boolean expression that specifies the condition for the join. This is frequently a [comparison operation](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/operators#comparison_operators) or logical combination of comparison operators.

Details:

Similarly to `CROSS JOIN`, `ON` produces a column once for each column in each
input table.

A `NULL` join condition evaluation is equivalent to a `FALSE` evaluation.

If a column-order sensitive operation such as `UNION` or `SELECT *` is used with
the `ON` join condition, the resulting table contains all of the columns from
the left input in order, and then all of the columns from the right input in
order.

**Examples**

The following examples show how to use the `ON` clause:

    WITH
      A AS ( SELECT 1 as x UNION ALL SELECT 2 UNION ALL SELECT 3),
      B AS ( SELECT 2 as x UNION ALL SELECT 3 UNION ALL SELECT 4)
    SELECT * FROM A INNER JOIN B ON A.x = B.x;

    WITH
      A AS ( SELECT 1 as x UNION ALL SELECT 2 UNION ALL SELECT 3),
      B AS ( SELECT 2 as x UNION ALL SELECT 3 UNION ALL SELECT 4)
    SELECT A.x, B.x FROM A INNER JOIN B ON A.x = B.x;

    /*
    Table A   Table B   Result (A.x, B.x)
    +---+     +---+     +---+
    | x |  *  | x |  =  | x | x |
    +---+     +---+     +---+
    | 1 |     | 2 |     | 2 | 2 |
    | 2 |     | 3 |     | 3 | 3 |
    | 3 |     | 4 |     +---+
    +---+     +---+
    */

    WITH
      A AS ( SELECT 1 as x UNION ALL SELECT 2 UNION ALL SELECT 3 UNION ALL SELECT NULL),
      B AS ( SELECT 2 as x UNION ALL SELECT 3 UNION ALL SELECT 4 UNION ALL SELECT 5)
    SELECT * FROM A LEFT OUTER JOIN B ON A.x = B.x;

    WITH
      A AS ( SELECT 1 as x UNION ALL SELECT 2 UNION ALL SELECT 3 UNION ALL SELECT NULL),
      B AS ( SELECT 2 as x UNION ALL SELECT 3 UNION ALL SELECT 4 UNION ALL SELECT 5)
    SELECT A.x, B.x FROM A LEFT OUTER JOIN B ON A.x = B.x;

    /*
    Table A    Table B   Result
    +---+   +---+     +---+
    | x    | * | x |  =  | x    | x    |
    +---+   +---+     +---+
    | 1    |   | 2 |     | 1    | NULL |
    | 2    |   | 3 |     | 2    | 2    |
    | 3    |   | 4 |     | 3    | 3    |
    | NULL |   | 5 |     | NULL | NULL |
    +---+   +---+     +---+
    */

    WITH
      A AS ( SELECT 1 as x UNION ALL SELECT 2 UNION ALL SELECT 3 UNION ALL SELECT NULL),
      B AS ( SELECT 2 as x UNION ALL SELECT 3 UNION ALL SELECT 4 UNION ALL SELECT 5)
    SELECT * FROM A FULL OUTER JOIN B ON A.x = B.x;

    WITH
      A AS ( SELECT 1 as x UNION ALL SELECT 2 UNION ALL SELECT 3 UNION ALL SELECT NULL),
      B AS ( SELECT 2 as x UNION ALL SELECT 3 UNION ALL SELECT 4 UNION ALL SELECT 5)
    SELECT A.x, B.x FROM A FULL OUTER JOIN B ON A.x = B.x;

    /*
    Table A    Table B   Result
    +---+   +---+     +---+
    | x    | * | x |  =  | x    | x    |
    +---+   +---+     +---+
    | 1    |   | 2 |     | 1    | NULL |
    | 2    |   | 3 |     | 2    | 2    |
    | 3    |   | 4 |     | 3    | 3    |
    | NULL |   | 5 |     | NULL | NULL |
    +---+   +---+     | NULL | 4    |
                         | NULL | 5    |
                         +---+
    */

#### `USING` clause

    USING ( column_name_list )

    column_name_list:
        column_name[, ...]

**Description**

When you are joining two tables, `USING` performs an
[equality comparison operation](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/operators#comparison_operators) on the columns named in
`column_name_list`. Each column name in `column_name_list` must appear in both
input tables. For each pair of rows from the input tables, if the
equality comparisons all evaluate to `TRUE`, one row is added to the resulting
column.

Definitions:

- `column_name_list`: A list of columns to include in the join condition.
- `column_name`: The column that exists in both of the tables that you are joining.

Details:

A `NULL` join condition evaluation is equivalent to a `FALSE` evaluation.

If a column-order sensitive operation such as `UNION` or `SELECT *` is used
with the `USING` join condition, the resulting table contains columns in this
order:

- The columns from `column_name_list` in the order they appear in the `USING` clause.
- All other columns of the left input in the order they appear in the input.
- All other columns of the right input in the order they appear in the input.

A column name in the `USING` clause must not be qualified by a
table name.

If the join is an `INNER JOIN` or a `LEFT OUTER JOIN`, the output
columns are populated from the values in the first table. If the
join is a `RIGHT OUTER JOIN`, the output columns are populated from the values
in the second table. If the join is a `FULL OUTER JOIN`, the output columns
are populated by [coalescing](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/conditional_expressions#coalesce) the values from the left and right
tables in that order.

**Examples**

The following example shows how to use the `USING` clause with one
column name in the column name list:

    WITH
      A AS ( SELECT 1 as x UNION ALL SELECT 2 UNION ALL SELECT 9 UNION ALL SELECT NULL),
      B AS ( SELECT 2 as x UNION ALL SELECT 9 UNION ALL SELECT 9 UNION ALL SELECT 5)
    SELECT * FROM A INNER JOIN B USING (x);

    /*
    Table A    Table B   Result
    +---+   +---+     +---+
    | x    | * | x |  =  | x |
    +---+   +---+     +---+
    | 1    |   | 2 |     | 2 |
    | 2    |   | 9 |     | 9 |
    | 9    |   | 9 |     | 9 |
    | NULL |   | 5 |     +---+
    +---+   +---+
    */

The following example shows how to use the `USING` clause with
multiple column names in the column name list:

    WITH
      A AS (
        SELECT 1 as x, 15 as y UNION ALL
        SELECT 2, 10 UNION ALL
        SELECT 9, 16 UNION ALL
        SELECT NULL, 12),
      B AS (
        SELECT 2 as x, 10 as y UNION ALL
        SELECT 9, 17 UNION ALL
        SELECT 9, 16 UNION ALL
        SELECT 5, 15)
    SELECT * FROM A INNER JOIN B USING (x, y);

    /*
    Table A         Table B        Result
    +---+   +---+     +---+
    | x    | y  | * | x  | y  |  =  | x  | y  |
    +---+   +---+     +---+
    | 1    | 15 |   | 2  | 10 |     | 2  | 10 |
    | 2    | 10 |   | 9  | 17 |     | 9  | 16 |
    | 9    | 16 |   | 9  | 16 |     +---+
    | NULL | 12 |   | 5  | 15 |
    +---+   +---+
    */

The following examples show additional ways in which to use the `USING` clause
with one column name in the column name list:

    WITH
      A AS ( SELECT 1 as x UNION ALL SELECT 2 UNION ALL SELECT 9 UNION ALL SELECT NULL),
      B AS ( SELECT 2 as x UNION ALL SELECT 9 UNION ALL SELECT 9 UNION ALL SELECT 5)
    SELECT x, A.x, B.x FROM A INNER JOIN B USING (x)

    /*
    Table A    Table B   Result
    +---+   +---+     +---+
    | x    | * | x |  =  | x    | A.x  | B.x  |
    +---+   +---+     +---+
    | 1    |   | 2 |     | 2    | 2    | 2    |
    | 2    |   | 9 |     | 9    | 9    | 9    |
    | 9    |   | 9 |     | 9    | 9    | 9    |
    | NULL |   | 5 |     +---+
    +---+   +---+
    */

    WITH
      A AS ( SELECT 1 as x UNION ALL SELECT 2 UNION ALL SELECT 9 UNION ALL SELECT NULL),
      B AS ( SELECT 2 as x UNION ALL SELECT 9 UNION ALL SELECT 9 UNION ALL SELECT 5)
    SELECT x, A.x, B.x FROM A LEFT OUTER JOIN B USING (x)

    /*
    Table A    Table B   Result
    +---+   +---+     +---+
    | x    | * | x |  =  | x    | A.x  | B.x  |
    +---+   +---+     +---+
    | 1    |   | 2 |     | 1    | 1    | NULL |
    | 2    |   | 9 |     | 2    | 2    | 2    |
    | 9    |   | 9 |     | 9    | 9    | 9    |
    | NULL |   | 5 |     | 9    | 9    | 9    |
    +---+   +---+     | NULL | NULL | NULL |
                         +---+
    */

    WITH
      A AS ( SELECT 1 as x UNION ALL SELECT 2 UNION ALL SELECT 2 UNION ALL SELECT NULL),
      B AS ( SELECT 2 as x UNION ALL SELECT 9 UNION ALL SELECT 9 UNION ALL SELECT 5)
    SELECT x, A.x, B.x FROM A RIGHT OUTER JOIN B USING (x)

    /*
    Table A    Table B   Result
    +---+   +---+     +---+
    | x    | * | x |  =  | x    | A.x  | B.x  |
    +---+   +---+     +---+
    | 1    |   | 2 |     | 2    | 2    | 2    |
    | 2    |   | 9 |     | 2    | 2    | 2    |
    | 2    |   | 9 |     | 9    | NULL | 9    |
    | NULL |   | 5 |     | 9    | NULL | 9    |
    +---+   +---+     | 5    | NULL | 5    |
                         +---+
    */

    WITH
      A AS ( SELECT 1 as x UNION ALL SELECT 2 UNION ALL SELECT 2 UNION ALL SELECT NULL),
      B AS ( SELECT 2 as x UNION ALL SELECT 9 UNION ALL SELECT 9 UNION ALL SELECT 5)
    SELECT x, A.x, B.x FROM A FULL OUTER JOIN B USING (x);

    /*
    Table A    Table B   Result
    +---+   +---+     +---+
    | x    | * | x |  =  | x    | A.x  | B.x  |
    +---+   +---+     +---+
    | 1    |   | 2 |     | 1    | 1    | NULL |
    | 2    |   | 9 |     | 2    | 2    | 2    |
    | 2    |   | 9 |     | 2    | 2    | 2    |
    | NULL |   | 5 |     | NULL | NULL | NULL |
    +---+   +---+     | 9    | NULL | 9    |
                         | 9    | NULL | 9    |
                         | 5    | NULL | 5    |
                         +---+
    */

The following example shows how to use the `USING` clause with
only some column names in the column name list.

    WITH
      A AS (
        SELECT 1 as x, 15 as y UNION ALL
        SELECT 2, 10 UNION ALL
        SELECT 9, 16 UNION ALL
        SELECT NULL, 12),
      B AS (
        SELECT 2 as x, 10 as y UNION ALL
        SELECT 9, 17 UNION ALL
        SELECT 9, 16 UNION ALL
        SELECT 5, 15)
    SELECT * FROM A INNER JOIN B USING (x);

    /*
    Table A         Table B         Result
    +---+   +---+     +---+
    | x    | y  | * | x  | y  |  =  | x   | A.y | B.y |
    +---+   +---+     +---+
    | 1    | 15 |   | 2  | 10 |     | 2   | 10  | 10  |
    | 2    | 10 |   | 9  | 17 |     | 9   | 16  | 17  |
    | 9    | 16 |   | 9  | 16 |     | 9   | 16  | 16  |
    | NULL | 12 |   | 5  | 15 |     +---+
    +---+   +---+
    */

The following query performs an `INNER JOIN` on the
[`Roster`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#roster_table) and [`TeamMascot`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#teammascot_table) table.
The query returns the rows from `Roster` and `TeamMascot` where
`Roster.SchoolID` is the same as `TeamMascot.SchoolID`. The results include a
single `SchoolID` column.

    SELECT * FROM Roster INNER JOIN TeamMascot USING (SchoolID);

    /*---+
     | SchoolID   | LastName   | Mascot       |
     +---+
     | 50         | Adams      | Jaguars      |
     | 52         | Buchanan   | Lakers       |
     | 52         | Coolidge   | Lakers       |
     | 51         | Davis      | Knights      |
     +---*/

#### `ON` and `USING` equivalency

The [`ON`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#on_clause) and [`USING`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#using_clause) join conditions aren't
equivalent, but they share some rules and sometimes they can produce similar
results.

In the following examples, observe what is returned when all rows
are produced for inner and outer joins. Also, look at how
each join condition handles `NULL` values.

    WITH
      A AS (SELECT 1 as x UNION ALL SELECT 2 UNION ALL SELECT 3),
      B AS (SELECT 2 as x UNION ALL SELECT 3 UNION ALL SELECT 4)
    SELECT * FROM A INNER JOIN B ON A.x = B.x;

    WITH
      A AS (SELECT 1 as x UNION ALL SELECT 2 UNION ALL SELECT 3),
      B AS (SELECT 2 as x UNION ALL SELECT 3 UNION ALL SELECT 4)
    SELECT * FROM A INNER JOIN B USING (x);

    /*
    Table A   Table B   Result ON     Result USING
    +---+     +---+     +---+     +---+
    | x |  *  | x |  =  | x | x |     | x |
    +---+     +---+     +---+     +---+
    | 1 |     | 2 |     | 2 | 2 |     | 2 |
    | 2 |     | 3 |     | 3 | 3 |     | 3 |
    | 3 |     | 4 |     +---+     +---+
    +---+     +---+
    */

    WITH
      A AS (SELECT 1 as x UNION ALL SELECT 2 UNION ALL SELECT 3 UNION ALL SELECT NULL),
      B AS (SELECT 2 as x UNION ALL SELECT 3 UNION ALL SELECT 4 UNION ALL SELECT 5)
    SELECT * FROM A LEFT OUTER JOIN B ON A.x = B.x;

    WITH
      A AS (SELECT 1 as x UNION ALL SELECT 2 UNION ALL SELECT 3 UNION ALL SELECT NULL),
      B AS (SELECT 2 as x UNION ALL SELECT 3 UNION ALL SELECT 4 UNION ALL SELECT 5)
    SELECT * FROM A LEFT OUTER JOIN B USING (x);

    /*
    Table A    Table B   Result ON           Result USING
    +---+   +---+     +---+     +---+
    | x    | * | x |  =  | x    | x    |     | x    |
    +---+   +---+     +---+     +---+
    | 1    |   | 2 |     | 1    | NULL |     | 1    |
    | 2    |   | 3 |     | 2    | 2    |     | 2    |
    | 3    |   | 4 |     | 3    | 3    |     | 3    |
    | NULL |   | 5 |     | NULL | NULL |     | NULL |
    +---+   +---+     +---+     +---+
    */

    WITH
      A AS (SELECT 1 as x UNION ALL SELECT 2 UNION ALL SELECT 3),
      B AS (SELECT 2 as x UNION ALL SELECT 3 UNION ALL SELECT 4)
    SELECT * FROM A FULL OUTER JOIN B ON A.x = B.x;

    WITH
      A AS (SELECT 1 as x UNION ALL SELECT 2 UNION ALL SELECT 3),
      B AS (SELECT 2 as x UNION ALL SELECT 3 UNION ALL SELECT 4)
    SELECT * FROM A FULL OUTER JOIN B USING (x);

    /*
    Table A   Table B   Result ON           Result USING
    +---+     +---+     +---+     +---+
    | x |  *  | x |  =  | x    | x    |     | x |
    +---+     +---+     +---+     +---+
    | 1 |     | 2 |     | 1    | NULL |     | 1 |
    | 2 |     | 3 |     | 2    | 2    |     | 2 |
    | 3 |     | 4 |     | 3    | 3    |     | 3 |
    +---+     +---+     | NULL | 4    |     | 4 |
                        +---+     +---+
    */

Although `ON` and `USING` aren't equivalent, they can return the same
results in some situations if you specify the columns you want to return.

In the following examples, observe what is returned when a specific row
is produced for inner and outer joins. Also, look at how each
join condition handles `NULL` values.

    WITH
      A AS (SELECT 1 as x UNION ALL SELECT 2 UNION ALL SELECT 3 UNION ALL SELECT NULL),
      B AS (SELECT 2 as x UNION ALL SELECT 3 UNION ALL SELECT 4 UNION ALL SELECT 5)
    SELECT A.x FROM A INNER JOIN B ON A.x = B.x;

    WITH
      A AS (SELECT 1 as x UNION ALL SELECT 2 UNION ALL SELECT 3 UNION ALL SELECT NULL),
      B AS (SELECT 2 as x UNION ALL SELECT 3 UNION ALL SELECT 4 UNION ALL SELECT 5)
    SELECT x FROM A INNER JOIN B USING (x);

    /*
    Table A    Table B   Result ON     Result USING
    +---+   +---+     +---+         +---+
    | x    | * | x |  =  | x |         | x |
    +---+   +---+     +---+         +---+
    | 1    |   | 2 |     | 2 |         | 2 |
    | 2    |   | 3 |     | 3 |         | 3 |
    | 3    |   | 4 |     +---+         +---+
    | NULL |   | 5 |
    +---+   +---+
    */

    WITH
      A AS (SELECT 1 as x UNION ALL SELECT 2 UNION ALL SELECT 3 UNION ALL SELECT NULL),
      B AS (SELECT 2 as x UNION ALL SELECT 3 UNION ALL SELECT 4 UNION ALL SELECT 5)
    SELECT A.x FROM A LEFT OUTER JOIN B ON A.x = B.x;

    WITH
      A AS (SELECT 1 as x UNION ALL SELECT 2 UNION ALL SELECT 3 UNION ALL SELECT NULL),
      B AS (SELECT 2 as x UNION ALL SELECT 3 UNION ALL SELECT 4 UNION ALL SELECT 5)
    SELECT x FROM A LEFT OUTER JOIN B USING (x);

    /*
    Table A    Table B   Result ON    Result USING
    +---+   +---+     +---+     +---+
    | x    | * | x |  =  | x    |     | x    |
    +---+   +---+     +---+     +---+
    | 1    |   | 2 |     | 1    |     | 1    |
    | 2    |   | 3 |     | 2    |     | 2    |
    | 3    |   | 4 |     | 3    |     | 3    |
    | NULL |   | 5 |     | NULL |     | NULL |
    +---+   +---+     +---+     +---+
    */

    WITH
      A AS (SELECT 1 as x UNION ALL SELECT 2 UNION ALL SELECT 3 UNION ALL SELECT NULL),
      B AS (SELECT 2 as x UNION ALL SELECT 3 UNION ALL SELECT 4 UNION ALL SELECT 5)
    SELECT A.x FROM A FULL OUTER JOIN B ON A.x = B.x;

    WITH
      A AS (SELECT 1 as x UNION ALL SELECT 2 UNION ALL SELECT 3 UNION ALL SELECT NULL),
      B AS (SELECT 2 as x UNION ALL SELECT 3 UNION ALL SELECT 4 UNION ALL SELECT 5)
    SELECT x FROM A FULL OUTER JOIN B USING (x);

    /*
    Table A    Table B   Result ON    Result USING
    +---+   +---+     +---+     +---+
    | x    | * | x |  =  | x    |     | x    |
    +---+   +---+     +---+     +---+
    | 1    |   | 2 |     | 1    |     | 1    |
    | 2    |   | 3 |     | 2    |     | 2    |
    | 3    |   | 4 |     | 3    |     | 3    |
    | NULL |   | 5 |     | NULL |     | NULL |
    +---+   +---+     | NULL |     | 4    |
                         | NULL |     | 5    |
                         +---+     +---+
    */

    WITH
      A AS (SELECT 1 as x UNION ALL SELECT 2 UNION ALL SELECT 3 UNION ALL SELECT NULL),
      B AS (SELECT 2 as x UNION ALL SELECT 3 UNION ALL SELECT 4 UNION ALL SELECT 5)
    SELECT B.x FROM A FULL OUTER JOIN B ON A.x = B.x;

    WITH
      A AS (SELECT 1 as x UNION ALL SELECT 2 UNION ALL SELECT 3 UNION ALL SELECT NULL),
      B AS (SELECT 2 as x UNION ALL SELECT 3 UNION ALL SELECT 4 UNION ALL SELECT 5)
    SELECT x FROM A FULL OUTER JOIN B USING (x);

    /*
    Table A    Table B   Result ON    Result USING
    +---+   +---+     +---+     +---+
    | x    | * | x |  =  | x    |     | x    |
    +---+   +---+     +---+     +---+
    | 1    |   | 2 |     | 2    |     | 1    |
    | 2    |   | 3 |     | 3    |     | 2    |
    | 3    |   | 4 |     | NULL |     | 3    |
    | NULL |   | 5 |     | NULL |     | NULL |
    +---+   +---+     | 4    |     | 4    |
                         | 5    |     | 5    |
                         +---+     +---+
    */

In the following example, observe what is returned when `COALESCE` is used
with the `ON` clause. It provides the same results as a query
with the `USING` clause.

    WITH
      A AS (SELECT 1 as x UNION ALL SELECT 2 UNION ALL SELECT 3 UNION ALL SELECT NULL),
      B AS (SELECT 2 as x UNION ALL SELECT 3 UNION ALL SELECT 4 UNION ALL SELECT 5)
    SELECT COALESCE(A.x, B.x) FROM A FULL OUTER JOIN B ON A.x = B.x;

    WITH
      A AS (SELECT 1 as x UNION ALL SELECT 2 UNION ALL SELECT 3 UNION ALL SELECT NULL),
      B AS (SELECT 2 as x UNION ALL SELECT 3 UNION ALL SELECT 4 UNION ALL SELECT 5)
    SELECT x FROM A FULL OUTER JOIN B USING (x);

    /*
    Table A    Table B   Result ON    Result USING
    +---+   +---+     +---+     +---+
    | x    | * | x |  =  | x    |     | x    |
    +---+   +---+     +---+     +---+
    | 1    |   | 2 |     | 1    |     | 1    |
    | 2    |   | 3 |     | 2    |     | 2    |
    | 3    |   | 4 |     | 3    |     | 3    |
    | NULL |   | 5 |     | NULL |     | NULL |
    +---+   +---+     | 4    |     | 4    |
                         | 5    |     | 5    |
                         +---+     +---+
    */

### Join operations in a sequence

The `FROM` clause can contain multiple `JOIN` operations in a sequence.
`JOIN`s are bound from left to right. For example:

    FROM A JOIN B USING (x) JOIN C USING (x)

    -- A JOIN B USING (x)        = result_1
    -- result_1 JOIN C USING (x) = result_2
    -- result_2                  = return value

You can also insert parentheses to group `JOIN`s:

    FROM ( (A JOIN B USING (x)) JOIN C USING (x) )

    -- A JOIN B USING (x)        = result_1
    -- result_1 JOIN C USING (x) = result_2
    -- result_2                  = return value

With parentheses, you can group `JOIN`s so that they are bound in a different
order:

    FROM ( A JOIN (B JOIN C USING (x)) USING (x) )

    -- B JOIN C USING (x)       = result_1
    -- A JOIN result_1          = result_2
    -- result_2                 = return value

A `FROM` clause can have multiple joins. Provided there are no comma cross joins
in the `FROM` clause, joins don't require parenthesis, though parenthesis can
help readability:

    FROM A JOIN B JOIN C JOIN D USING (w) ON B.x = C.y ON A.z = B.x

If your clause contains comma cross joins, you must use parentheses:

    FROM A, B JOIN C JOIN D ON C.x = D.y ON B.z = C.x    // INVALID

    FROM A, B JOIN (C JOIN D ON C.x = D.y) ON B.z = C.x  // VALID

When comma cross joins are present in a query with a sequence of JOINs, they
group from left to right like other `JOIN` types:

    FROM A JOIN B USING (x) JOIN C USING (x), D

    -- A JOIN B USING (x)        = result_1
    -- result_1 JOIN C USING (x) = result_2
    -- result_2 CROSS JOIN D     = return value

There can't be a `RIGHT JOIN` or `FULL JOIN` after a comma cross join unless
it's parenthesized:

    FROM A, B RIGHT JOIN C ON TRUE // INVALID

    FROM A, B FULL JOIN C ON TRUE  // INVALID

    FROM A, B JOIN C ON TRUE       // VALID

    FROM A, (B RIGHT JOIN C ON TRUE) // VALID

    FROM A, (B FULL JOIN C ON TRUE)  // VALID

### Correlated join operation

A join operation is *correlated* when the right `from_item` contains a
reference to at least one range variable or
column name introduced by the left `from_item`.

In a correlated join operation, rows from the right `from_item` are determined
by a row from the left `from_item`. Consequently, `RIGHT OUTER` and `FULL OUTER`
joins can't be correlated because right `from_item` rows can't be determined
in the case when there is no row from the left `from_item`.

All correlated join operations must reference an array in the right `from_item`.

This is a conceptual example of a correlated join operation that includes
a [correlated subquery](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/subqueries#correlated_subquery_concepts):

    FROM A JOIN UNNEST(ARRAY(SELECT AS STRUCT * FROM B WHERE A.ID = B.ID)) AS C

- Left `from_item`: `A`
- Right `from_item`: `UNNEST(...) AS C`
- A correlated subquery: `(SELECT AS STRUCT * FROM B WHERE A.ID = B.ID)`

This is another conceptual example of a correlated join operation.
`array_of_IDs` is part of the left `from_item` but is referenced in the
right `from_item`.

    FROM A JOIN UNNEST(A.array_of_IDs) AS C

The [`UNNEST` operator](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#unnest_operator) can be explicit or implicit.
These are both allowed:

    FROM A JOIN UNNEST(A.array_of_IDs) AS IDs

    FROM A JOIN A.array_of_IDs AS IDs

In a correlated join operation, the right `from_item` is re-evaluated
against each distinct row from the left `from_item`. In the following
conceptual example, the correlated join operation first
evaluates `A` and `B`, then `A` and `C`:

    FROM
      A
      JOIN
      UNNEST(ARRAY(SELECT AS STRUCT * FROM B WHERE A.ID = B.ID)) AS C
      ON A.Name = C.Name

**Caveats**

- In a correlated `LEFT JOIN`, when the input table on the right side is empty for some row from the left side, it's as if no rows from the right side satisfied the join condition in a regular `LEFT JOIN`. When there are no joining rows, a row with `NULL` values for all columns on the right side is generated to join with the row from the left side.
- In a correlated `CROSS JOIN`, when the input table on the right side is empty for some row from the left side, it's as if no rows from the right side satisfied the join condition in a regular correlated `INNER JOIN`. This means that the row is dropped from the results.

**Examples**

This is an example of a correlated join, using the
[Roster](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#roster_table) and [PlayerStats](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#playerstats_table) tables:

    SELECT *
    FROM
      Roster
    JOIN
      UNNEST(
        ARRAY(
          SELECT AS STRUCT *
          FROM PlayerStats
          WHERE PlayerStats.OpponentID = Roster.SchoolID
        )) AS PlayerMatches
      ON PlayerMatches.LastName = 'Buchanan'

    /*---+---+---+---+---+
     | LastName   | SchoolID | LastName | OpponentID | PointsScored |
     +---+---+---+---+---+
     | Adams      | 50       | Buchanan | 50         | 13           |
     | Eisenhower | 77       | Buchanan | 77         | 0            |
     +---+---+---+---+---*/

A common pattern for a correlated `LEFT JOIN` is to have an `UNNEST` operation
on the right side that references an array from some column introduced by
input on the left side. For rows where that array is empty or `NULL`,
the `UNNEST` operation produces no rows on the right input. In that case, a row
with a `NULL` entry in each column of the right input is created to join with
the row from the left input. For example:

    SELECT A.name, item, ARRAY_LENGTH(A.items) item_count_for_name
    FROM
      UNNEST(
        [
          STRUCT(
            'first' AS name,
            [1, 2, 3, 4] AS items),
          STRUCT(
            'second' AS name,
            [] AS items)]) AS A
    LEFT JOIN
      A.items AS item;

    /*---+---+---+
     | name   | item | item_count_for_name |
     +---+---+---+
     | first  | 1    | 4                   |
     | first  | 2    | 4                   |
     | first  | 3    | 4                   |
     | first  | 4    | 4                   |
     | second | NULL | 0                   |
     +---+---+---*/

In the case of a correlated `INNER JOIN` or `CROSS JOIN`, when the input on the
right side is empty for some row from the left side, the final row is dropped
from the results. For example:

    SELECT A.name, item
    FROM
      UNNEST(
        [
          STRUCT(
            'first' AS name,
            [1, 2, 3, 4] AS items),
          STRUCT(
            'second' AS name,
            [] AS items)]) AS A
    INNER JOIN
      A.items AS item;

    /*---+---+
     | name  | item |
     +---+---+
     | first | 1    |
     | first | 2    |
     | first | 3    |
     | first | 4    |
     +---+---*/

## `WHERE` clause

```
WHERE bool_expression
```

The `WHERE` clause filters the results of the `FROM` clause.

Only rows whose `bool_expression` evaluates to `TRUE` are included. Rows
whose `bool_expression` evaluates to `NULL` or `FALSE` are
discarded.

The evaluation of a query with a `WHERE` clause is typically completed in this
order:

- `FROM`
- `WHERE`
- `GROUP BY` and aggregation
- `HAVING`
- `WINDOW`
- `QUALIFY`
- `DISTINCT`
- `ORDER BY`
- `LIMIT`

Evaluation order doesn't always match syntax order.

The `WHERE` clause only references columns available via the `FROM` clause;
it can't reference `SELECT` list aliases.

**Examples**

This query returns returns all rows from the [`Roster`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#roster_table) table
where the `SchoolID` column has the value `52`:

    SELECT * FROM Roster
    WHERE SchoolID = 52;

The `bool_expression` can contain multiple sub-conditions:

    SELECT * FROM Roster
    WHERE STARTS_WITH(LastName, "Mc") OR STARTS_WITH(LastName, "Mac");

Expressions in an `INNER JOIN` have an equivalent expression in the
`WHERE` clause. For example, a query using `INNER` `JOIN` and `ON` has an
equivalent expression using `CROSS JOIN` and `WHERE`. For example,
the following two queries are equivalent:

    SELECT Roster.LastName, TeamMascot.Mascot
    FROM Roster INNER JOIN TeamMascot
    ON Roster.SchoolID = TeamMascot.SchoolID;

    SELECT Roster.LastName, TeamMascot.Mascot
    FROM Roster CROSS JOIN TeamMascot
    WHERE Roster.SchoolID = TeamMascot.SchoolID;

## `GROUP BY` clause

```
GROUP BY group_by_specification

group_by_specification:
  {
    groupable_items
    | ALL
    | grouping_sets_specification
    | rollup_specification
    | cube_specification
    | ()
  }
```

**Description**

The `GROUP BY` clause groups together rows in a table that share common values
for certain columns. For a group of rows in the source table with
non-distinct values, the `GROUP BY` clause aggregates them into a single
combined row. This clause is commonly used when aggregate functions are
present in the `SELECT` list, or to eliminate redundancy in the output.

**Definitions**

- `groupable_items`: Group rows in a table that share common values for certain columns. To learn more, see [Group rows by groupable items](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#group_by_grouping_item).
- `ALL`: Automatically group rows. To learn more, see [Group rows automatically](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#group_by_all).
- `grouping_sets_specification`: Group rows with the `GROUP BY GROUPING SETS` clause. To learn more, see [Group rows by `GROUPING SETS`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#group_by_grouping_sets).
- `rollup_specification`: Group rows with the `GROUP BY ROLLUP` clause. To learn more, see [Group rows by `ROLLUP`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#group_by_rollup).
- `cube_specification`: Group rows with the `GROUP BY CUBE` clause. To learn more, see [Group rows by `CUBE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#group_by_cube).
- `()`: Group all rows and produce a grand total. Equivalent to no `group_by_specification`.

### Group rows by groupable items

```
GROUP BY groupable_item[, ...]

groupable_item:
  {
    value
    | value_alias
    | column_ordinal
  }
```

**Description**

The `GROUP BY` clause can include [groupable](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#data_type_properties) expressions
and their ordinals.

**Definitions**

- `value`: An expression that represents a non-distinct, groupable value. To learn more, see [Group rows by values](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#group_by_values).
- `value_alias`: An alias for `value`. To learn more, see [Group rows by values](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#group_by_values).
- `column_ordinal`: An `INT64` value that represents the ordinal assigned to a groupable expression in the `SELECT` list. To learn more, see [Group rows by column ordinals](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#group_by_col_ordinals).

#### Group rows by values

The `GROUP BY` clause can group rows in a table with non-distinct
values in the `GROUP BY` clause. For example:

    WITH PlayerStats AS (
      SELECT 'Adams' as LastName, 'Noam' as FirstName, 3 as PointsScored UNION ALL
      SELECT 'Buchanan', 'Jie', 0 UNION ALL
      SELECT 'Coolidge', 'Kiran', 1 UNION ALL
      SELECT 'Adams', 'Noam', 4 UNION ALL
      SELECT 'Buchanan', 'Jie', 13)
    SELECT SUM(PointsScored) AS total_points, LastName
    FROM PlayerStats
    GROUP BY LastName;

    /*---+---+
     | total_points | LastName |
     +---+---+
     | 7            | Adams    |
     | 13           | Buchanan |
     | 1            | Coolidge |
     +---+---*/

`GROUP BY` clauses may also refer to aliases. If a query contains aliases in
the `SELECT` clause, those aliases override names in the corresponding `FROM`
clause. For example:

    WITH PlayerStats AS (
      SELECT 'Adams' as LastName, 'Noam' as FirstName, 3 as PointsScored UNION ALL
      SELECT 'Buchanan', 'Jie', 0 UNION ALL
      SELECT 'Coolidge', 'Kiran', 1 UNION ALL
      SELECT 'Adams', 'Noam', 4 UNION ALL
      SELECT 'Buchanan', 'Jie', 13)
    SELECT SUM(PointsScored) AS total_points, LastName AS last_name
    FROM PlayerStats
    GROUP BY last_name;

    /*---+---+
     | total_points | last_name |
     +---+---+
     | 7            | Adams     |
     | 13           | Buchanan  |
     | 1            | Coolidge  |
     +---+---*/

You can use the `GROUP BY` clause with arrays. The following query executes
because the array elements being grouped are the same length and group type:

    WITH PlayerStats AS (
      SELECT ['Coolidge', 'Adams'] as Name, 3 as PointsScored UNION ALL
      SELECT ['Adams', 'Buchanan'], 0 UNION ALL
      SELECT ['Coolidge', 'Adams'], 1 UNION ALL
      SELECT ['Kiran', 'Noam'], 1)
    SELECT SUM(PointsScored) AS total_points, name
    FROM PlayerStats
    GROUP BY Name;

    /*---+---+
     | total_points | name             |
     +---+---+
     | 4            | [Coolidge,Adams] |
     | 0            | [Adams,Buchanan] |
     | 1            | [Kiran,Noam]     |
     +---+---*/

You can use the `GROUP BY` clause with structs. The following query executes
because the struct fields being grouped have the same group types:

    WITH
      TeamStats AS (
        SELECT
          ARRAY<STRUCT<last_name STRING, first_name STRING, age INT64>>[
            ('Adams', 'Noam', 20), ('Buchanan', 'Jie', 19)] AS Team,
          3 AS PointsScored
        UNION ALL
        SELECT [('Coolidge', 'Kiran', 21), ('Yang', 'Jason', 22)], 4
        UNION ALL
        SELECT [('Adams', 'Noam', 20), ('Buchanan', 'Jie', 19)], 10
        UNION ALL
        SELECT [('Coolidge', 'Kiran', 21), ('Yang', 'Jason', 22)], 7
      )
    SELECT
      SUM(PointsScored) AS total_points,
      Team
    FROM TeamStats
    GROUP BY Team;

    /*---+---+
     | total_points | teams                    |
     +---+---+
     | 13           | [{                       |
     |              |    last_name: "Adams",   |
     |              |    first_name: "Noam",   |
     |              |    age: 20               |
     |              |  },{                     |
     |              |    last_name: "Buchanan",|
     |              |    first_name: "Jie",    |
     |              |    age: 19               |
     |              |  }]                      |
     +---+
     | 11           | [{                       |
     |              |    last_name: "Coolidge",|
     |              |    first_name: "Kiran",  |
     |              |    age: 21               |
     |              |  },{                     |
     |              |    last_name: "Yang",    |
     |              |    first_name: "Jason",  |
     |              |    age: 22               |
     |              |  }]                      |
     +---+---*/

To learn more about the data types that are supported for values in the
`GROUP BY` clause, see [Groupable data types](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#data_type_properties).

#### Group rows by column ordinals

The `GROUP BY` clause can refer to expression names in the `SELECT` list. The
`GROUP BY` clause also allows ordinal references to expressions in the `SELECT`
list, using integer values. `1` refers to the first value in the
`SELECT` list, `2` the second, and so forth. The value list can combine
ordinals and value names. The following queries are equivalent:

    WITH PlayerStats AS (
      SELECT 'Adams' as LastName, 'Noam' as FirstName, 3 as PointsScored UNION ALL
      SELECT 'Buchanan', 'Jie', 0 UNION ALL
      SELECT 'Coolidge', 'Kiran', 1 UNION ALL
      SELECT 'Adams', 'Noam', 4 UNION ALL
      SELECT 'Buchanan', 'Jie', 13)
    SELECT SUM(PointsScored) AS total_points, LastName, FirstName
    FROM PlayerStats
    GROUP BY LastName, FirstName;

    /*---+---+---+
     | total_points | LastName | FirstName |
     +---+---+---+
     | 7            | Adams    | Noam      |
     | 13           | Buchanan | Jie       |
     | 1            | Coolidge | Kiran     |
     +---+---+---*/

    WITH PlayerStats AS (
      SELECT 'Adams' as LastName, 'Noam' as FirstName, 3 as PointsScored UNION ALL
      SELECT 'Buchanan', 'Jie', 0 UNION ALL
      SELECT 'Coolidge', 'Kiran', 1 UNION ALL
      SELECT 'Adams', 'Noam', 4 UNION ALL
      SELECT 'Buchanan', 'Jie', 13)
    SELECT SUM(PointsScored) AS total_points, LastName, FirstName
    FROM PlayerStats
    GROUP BY 2, 3;

    /*---+---+---+
     | total_points | LastName | FirstName |
     +---+---+---+
     | 7            | Adams    | Noam      |
     | 13           | Buchanan | Jie       |
     | 1            | Coolidge | Kiran     |
     +---+---+---*/

### Group rows by `ALL`

```
GROUP BY ALL
```

**Description**

The `GROUP BY ALL` clause groups rows by inferring grouping keys from the
`SELECT` items.

The following `SELECT` items are excluded from the `GROUP BY ALL` clause:

- Expressions that include [aggregate functions](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate-function-calls).
- Expressions that include [window functions](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/window-function-calls).
- Expressions that don't reference a name from the `FROM` clause. This includes:
  - Constants
  - Query parameters
  - Correlated column references
  - Expressions that only reference `GROUP BY` keys inferred from other `SELECT` items.

After exclusions are applied, an error is produced if any remaining `SELECT`
item includes a volatile function or has a non-groupable type.

If the set of inferred grouping keys is empty after exclusions are applied, all
input rows are considered a single group for aggregation. This
behavior is equivalent to writing `GROUP BY ()`.

**Examples**

In the following example, the query groups rows by `first_name` and
`last_name`. `total_points` is excluded because it represents an
aggregate function.

    WITH PlayerStats AS (
      SELECT 'Adams' as LastName, 'Noam' as FirstName, 3 as PointsScored UNION ALL
      SELECT 'Buchanan', 'Jie', 0 UNION ALL
      SELECT 'Coolidge', 'Kiran', 1 UNION ALL
      SELECT 'Adams', 'Noam', 4 UNION ALL
      SELECT 'Buchanan', 'Jie', 13)
    SELECT
      SUM(PointsScored) AS total_points,
      FirstName AS first_name,
      LastName AS last_name
    FROM PlayerStats
    GROUP BY ALL;

    /*---+---+---+
     | total_points | first_name | last_name |
     +---+---+---+
     | 7            | Noam       | Adams     |
     | 13           | Jie        | Buchanan  |
     | 1            | Kiran      | Coolidge  |
     +---+---+---*/

If the select list contains an analytic function, the query groups rows by
`first_name` and `last_name`. `total_people` is excluded because it
contains a window function.

    WITH PlayerStats AS (
      SELECT 'Adams' as LastName, 'Noam' as FirstName, 3 as PointsScored UNION ALL
      SELECT 'Buchanan', 'Jie', 0 UNION ALL
      SELECT 'Coolidge', 'Kiran', 1 UNION ALL
      SELECT 'Adams', 'Noam', 4 UNION ALL
      SELECT 'Buchanan', 'Jie', 13)
    SELECT
      COUNT(*) OVER () AS total_people,
      FirstName AS first_name,
      LastName AS last_name
    FROM PlayerStats
    GROUP BY ALL;

    /*---+---+---+
     | total_people | first_name | last_name |
     +---+---+---+
     | 3            | Noam       | Adams     |
     | 3            | Jie        | Buchanan  |
     | 3            | Kiran      | Coolidge  |
     +---+---+---*/

If multiple `SELECT` items reference the same `FROM` item, and any of them is
a path expression prefix of another, only the prefix path is used for grouping.
In the following example, `coordinates` is excluded because `x_coordinate` and
`y_coordinate` have already referenced `Values.x` and `Values.y` in the
`FROM` clause, and they are prefixes of the path expression used in
`x_coordinate`:

    WITH Values AS (
      SELECT 1 AS x, 2 AS y
      UNION ALL SELECT 1 AS x, 4 AS y
      UNION ALL SELECT 2 AS x, 5 AS y
    )
    SELECT
      Values.x AS x_coordinate,
      Values.y AS y_coordinate,
      [Values.x, Values.y] AS coordinates
    FROM Values
    GROUP BY ALL

    /*---+---+---+
     | x_coordinate | y_coordinate | coordinates |
     +---+---+---+
     | 1            | 4            | [1, 4]      |
     | 1            | 2            | [1, 2]      |
     | 2            | 5            | [2, 5]      |
     +---+---+---*/

In the following example, the inferred set of grouping keys is empty. The query
returns one row even when the input contains zero rows.

    SELECT COUNT(*) AS num_rows
    FROM UNNEST([])
    GROUP BY ALL

    /*---+
     | num_rows |
     +---+
     | 0        |
     +---*/

### Group rows by `GROUPING SETS`

```
GROUP BY GROUPING SETS ( grouping_list )

grouping_list:
  {
    rollup_specification
    | cube_specification
    | groupable_item
    | groupable_item_set
  }[, ...]

groupable_item_set:
  ( [ groupable_item[, ...] ] )
```

**Description**

The `GROUP BY GROUPING SETS` clause produces aggregated data for one or more
*grouping sets* . A grouping set is a group of columns by which rows can
be grouped together. This clause is helpful if you want to produce
aggregated data for sets of data without using the `UNION` operation.
For example, `GROUP BY GROUPING SETS(x,y)` is roughly equivalent to
`GROUP BY x UNION ALL GROUP BY y`.

**Definitions**

- `grouping_list`: A list of items that you can add to the `GROUPING SETS` clause. Grouping sets are generated based upon what is in this list.
- `rollup_specification`: Group rows with the `ROLLUP` clause. Don't include `GROUP BY` if you use this inside the `GROUPING SETS` clause. To learn more, see [Group rows by `ROLLUP`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#group_by_rollup).
- `cube_specification`: Group rows with the `CUBE` clause. Don't include `GROUP BY` if you use this inside the `GROUPING SETS` clause. To learn more, see [Group rows by `CUBE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#group_by_cube).
- `groupable_item`: Group rows in a table that share common values for certain columns. To learn more, see [Group rows by groupable items](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#group_by_grouping_item). [Anonymous `STRUCT` values](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#tuple_syntax) aren't allowed.
- `groupable_item_set`: Group rows by a set of [groupable items](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#group_by_grouping_item). If the set contains no groupable items, group all rows and produce a grand total.

**Details**

`GROUP BY GROUPING SETS` works by taking a grouping list, generating
grouping sets from it, and then producing a table as a union of queries
grouped by each grouping set.

For example, `GROUP BY GROUPING SETS (a, b, c)` generates the
following grouping sets from the grouping list, `a, b, c`, and
then produces aggregated rows for each of them:

- `(a)`
- `(b)`
- `(c)`

Here is an example that includes groupable item sets in
`GROUP BY GROUPING SETS (a, (b, c), d)`:

| Conceptual grouping sets | Actual grouping sets |
|---|---|
| `(a)` | `(a)` |
| `((b, c))` | `(b, c)` |
| `(d)` | `(d)` |

`GROUP BY GROUPING SETS` can include `ROLLUP` and `CUBE` operations, which
generate grouping sets. If `ROLLUP` is added, it generates rolled up
grouping sets. If `CUBE` is added, it generates grouping set permutations.

The following grouping sets are generated for
`GROUP BY GROUPING SETS (a, ROLLUP(b, c), d)`.

| Conceptual grouping sets | Actual grouping sets |
|---|---|
| `(a)` | `(a)` |
| `((b, c))` | `(b, c)` |
| `((b))` | `(b)` |
| `(())` | `()` |
| `(d)` | `(d)` |

The following grouping sets are generated for
`GROUP BY GROUPING SETS (a, CUBE(b, c), d)`:

| Conceptual grouping sets | Actual grouping sets |
|---|---|
| `(a)` | `(a)` |
| `((b, c))` | `(b, c)` |
| `((b))` | `(b)` |
| `((c))` | `(c)` |
| `(())` | `()` |
| `(d)` | `(d)` |

When evaluating the results for a particular grouping set,
expressions that aren't in the grouping set are aggregated and produce a
`NULL` placeholder.

You can filter results for specific groupable items. To learn more, see the
[`GROUPING` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#grouping)

`GROUPING SETS` allows up to 4096 groupable items.

**Examples**

The following queries produce the same results, but
the first one uses `GROUP BY GROUPING SETS` and the second one doesn't:

    -- GROUP BY with GROUPING SETS
    WITH
      Products AS (
        SELECT 'shirt' AS product_type, 't-shirt' AS product_name, 3 AS product_count UNION ALL
        SELECT 'shirt', 't-shirt', 8 UNION ALL
        SELECT 'shirt', 'polo', 25 UNION ALL
        SELECT 'pants', 'jeans', 6
      )
    SELECT product_type, product_name, SUM(product_count) AS product_sum
    FROM Products
    GROUP BY GROUPING SETS (product_type, product_name)
    ORDER BY product_name

    /*---+---+---+
     | product_type | product_name | product_sum |
     +---+---+---+
     | shirt        | NULL         | 36          |
     | pants        | NULL         | 6           |
     | NULL         | jeans        | 6           |
     | NULL         | polo         | 25          |
     | NULL         | t-shirt      | 11          |
     +---+---+---*/

    -- GROUP BY without GROUPING SETS
    -- (produces the same results as GROUPING SETS)
    WITH
      Products AS (
        SELECT 'shirt' AS product_type, 't-shirt' AS product_name, 3 AS product_count UNION ALL
        SELECT 'shirt', 't-shirt', 8 UNION ALL
        SELECT 'shirt', 'polo', 25 UNION ALL
        SELECT 'pants', 'jeans', 6
      )
    SELECT product_type, NULL, SUM(product_count) AS product_sum
    FROM Products
    GROUP BY product_type
    UNION ALL
    SELECT NULL, product_name, SUM(product_count) AS product_sum
    FROM Products
    GROUP BY product_name
    ORDER BY product_name

You can include groupable item sets in a `GROUP BY GROUPING SETS` clause.
In the example below, `(product_type, product_name)` is a groupable item set.

    -- GROUP BY with GROUPING SETS and a groupable item set
    WITH
      Products AS (
        SELECT 'shirt' AS product_type, 't-shirt' AS product_name, 3 AS product_count UNION ALL
        SELECT 'shirt', 't-shirt', 8 UNION ALL
        SELECT 'shirt', 'polo', 25 UNION ALL
        SELECT 'pants', 'jeans', 6
      )
    SELECT product_type, product_name, SUM(product_count) AS product_sum
    FROM Products
    GROUP BY GROUPING SETS (
      product_type,
      (product_type, product_name))
    ORDER BY product_type, product_name;

    /*---+---+---+
     | product_type | product_name | product_sum |
     +---+---+---+
     | pants        | NULL         | 6           |
     | pants        | jeans        | 6           |
     | shirt        | NULL         | 36          |
     | shirt        | polo         | 25          |
     | shirt        | t-shirt      | 11          |
     +---+---+---*/

    -- GROUP BY with GROUPING SETS but without a groupable item set
    -- (produces the same results as GROUPING SETS with a groupable item set)
    WITH
      Products AS (
        SELECT 'shirt' AS product_type, 't-shirt' AS product_name, 3 AS product_count UNION ALL
        SELECT 'shirt', 't-shirt', 8 UNION ALL
        SELECT 'shirt', 'polo', 25 UNION ALL
        SELECT 'pants', 'jeans', 6
      )
    SELECT product_type, NULL, SUM(product_count) AS product_sum
    FROM Products
    GROUP BY product_type
    UNION ALL
    SELECT product_type, product_name, SUM(product_count) AS product_sum
    FROM Products
    GROUP BY product_type, product_name
    ORDER BY product_type, product_name;

You can include [`ROLLUP`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#group_by_rollup) in a
`GROUP BY GROUPING SETS` clause. For example:

    -- GROUP BY with GROUPING SETS and ROLLUP
    WITH
      Products AS (
        SELECT 'shirt' AS product_type, 't-shirt' AS product_name, 3 AS product_count UNION ALL
        SELECT 'shirt', 't-shirt', 8 UNION ALL
        SELECT 'shirt', 'polo', 25 UNION ALL
        SELECT 'pants', 'jeans', 6
      )
    SELECT product_type, product_name, SUM(product_count) AS product_sum
    FROM Products
    GROUP BY GROUPING SETS (
      product_type,
      ROLLUP (product_type, product_name))
    ORDER BY product_type, product_name;

    /*---+---+---+
     | product_type | product_name | product_sum |
     +---+---+---+
     | NULL         | NULL         | 42          |
     | pants        | NULL         | 6           |
     | pants        | NULL         | 6           |
     | pants        | jeans        | 6           |
     | shirt        | NULL         | 36          |
     | shirt        | NULL         | 36          |
     | shirt        | polo         | 25          |
     | shirt        | t-shirt      | 11          |
     +---+---+---*/

    -- GROUP BY with GROUPING SETS, but without ROLLUP
    -- (produces the same results as GROUPING SETS with ROLLUP)
    WITH
      Products AS (
        SELECT 'shirt' AS product_type, 't-shirt' AS product_name, 3 AS product_count UNION ALL
        SELECT 'shirt', 't-shirt', 8 UNION ALL
        SELECT 'shirt', 'polo', 25 UNION ALL
        SELECT 'pants', 'jeans', 6
      )
    SELECT product_type, product_name, SUM(product_count) AS product_sum
    FROM Products
    GROUP BY GROUPING SETS(
      product_type,
      (product_type, product_name),
      product_type,
      ())
    ORDER BY product_type, product_name;

You can include [`CUBE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#group_by_cube) in a `GROUP BY GROUPING SETS` clause.
For example:

    -- GROUP BY with GROUPING SETS and CUBE
    WITH
      Products AS (
        SELECT 'shirt' AS product_type, 't-shirt' AS product_name, 3 AS product_count UNION ALL
        SELECT 'shirt', 't-shirt', 8 UNION ALL
        SELECT 'shirt', 'polo', 25 UNION ALL
        SELECT 'pants', 'jeans', 6
      )
    SELECT product_type, product_name, SUM(product_count) AS product_sum
    FROM Products
    GROUP BY GROUPING SETS (
      product_type,
      CUBE (product_type, product_name))
    ORDER BY product_type, product_name;

    /*---+---+---+
     | product_type | product_name | product_sum |
     +---+---+---+
     | NULL         | NULL         | 42          |
     | NULL         | jeans        | 6           |
     | NULL         | polo         | 25          |
     | NULL         | t-shirt      | 11          |
     | pants        | NULL         | 6           |
     | pants        | NULL         | 6           |
     | pants        | jeans        | 6           |
     | shirt        | NULL         | 36          |
     | shirt        | NULL         | 36          |
     | shirt        | polo         | 25          |
     | shirt        | t-shirt      | 11          |
     +---+---+---*/

    -- GROUP BY with GROUPING SETS, but without CUBE
    -- (produces the same results as GROUPING SETS with CUBE)
    WITH
      Products AS (
        SELECT 'shirt' AS product_type, 't-shirt' AS product_name, 3 AS product_count UNION ALL
        SELECT 'shirt', 't-shirt', 8 UNION ALL
        SELECT 'shirt', 'polo', 25 UNION ALL
        SELECT 'pants', 'jeans', 6
      )
    SELECT product_type, product_name, SUM(product_count) AS product_sum
    FROM Products
    GROUP BY GROUPING SETS(
      product_type,
      (product_type, product_name),
      product_type,
      product_name,
      ())
    ORDER BY product_type, product_name;

### Group rows by `ROLLUP`

```
GROUP BY ROLLUP ( grouping_list )

grouping_list:
  { groupable_item | groupable_item_set }[, ...]

groupable_item_set:
  ( groupable_item[, ...] )
```

**Description**

The `GROUP BY ROLLUP` clause produces aggregated data for rolled up
*grouping sets*. A grouping set is a group of columns by which rows can
be grouped together. This clause is helpful if you need to roll up totals
in a set of data.

**Definitions**

- `grouping_list`: A list of items that you can add to the `GROUPING SETS` clause. This is used to create a generated list of grouping sets when the query is run.
- `groupable_item`: Group rows in a table that share common values for certain columns. To learn more, see [Group rows by groupable items](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#group_by_grouping_item).[anonymous `STRUCT` values](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#tuple_syntax) aren't allowed.
- `groupable_item_set`: Group rows by a subset of [groupable items](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#group_by_grouping_item).

**Details**

`GROUP BY ROLLUP` works by taking a grouping list, generating
grouping sets from the prefixes inside this list, and then producing a
table as a union of queries grouped by each grouping set. The resulting
grouping sets include an empty grouping set. In the empty grouping set, all
rows are aggregated down to a single group.

For example, `GROUP BY ROLLUP (a, b, c)` generates the
following grouping sets from the grouping list, `a, b, c`, and then produces
aggregated rows for each of them:

- `(a, b, c)`
- `(a, b)`
- `(a)`
- `()`

Here is an example that includes groupable item sets in
`GROUP BY ROLLUP (a, (b, c), d)`:

| Conceptual grouping sets | Actual grouping sets |
|---|---|
| `(a, (b, c), d)` | `(a, b, c, d)` |
| `(a, (b, c))` | `(a, b, c)` |
| `(a)` | `(a)` |
| `()` | `()` |

When evaluating the results for a particular grouping set,
expressions that aren't in the grouping set are aggregated and produce a
`NULL` placeholder.

You can filter results by specific groupable items. To learn more, see the
[`GROUPING` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#grouping)

`ROLLUP` allows up to 4095 groupable items (equivalent to 4096 grouping sets).

**Examples**

The following queries produce the same subtotals and a grand total, but
the first one uses `GROUP BY` with `ROLLUP` and the second one doesn't:

    -- GROUP BY with ROLLUP
    WITH
      Products AS (
        SELECT 'shirt' AS product_type, 't-shirt' AS product_name, 3 AS product_count UNION ALL
        SELECT 'shirt', 't-shirt', 8 UNION ALL
        SELECT 'shirt', 'polo', 25 UNION ALL
        SELECT 'pants', 'jeans', 6
      )
    SELECT product_type, product_name, SUM(product_count) AS product_sum
    FROM Products
    GROUP BY ROLLUP (product_type, product_name)
    ORDER BY product_type, product_name;

    /*---+---+---+
     | product_type | product_name | product_sum |
     +---+---+---+
     | NULL         | NULL         | 42          |
     | pants        | NULL         | 6           |
     | pants        | jeans        | 6           |
     | shirt        | NULL         | 36          |
     | shirt        | t-shirt      | 11          |
     | shirt        | polo         | 25          |
     +---+---+---*/

    -- GROUP BY without ROLLUP (produces the same results as ROLLUP)
    WITH
      Products AS (
        SELECT 'shirt' AS product_type, 't-shirt' AS product_name, 3 AS product_count UNION ALL
        SELECT 'shirt', 't-shirt', 8 UNION ALL
        SELECT 'shirt', 'polo', 25 UNION ALL
        SELECT 'pants', 'jeans', 6
      )
    SELECT product_type, product_name, SUM(product_count) AS product_sum
    FROM Products
    GROUP BY product_type, product_name
    UNION ALL
    SELECT product_type, NULL, SUM(product_count)
    FROM Products
    GROUP BY product_type
    UNION ALL
    SELECT NULL, NULL, SUM(product_count) FROM Products
    ORDER BY product_type, product_name;

You can include groupable item sets in a `GROUP BY ROLLUP` clause.
In the following example, `(product_type, product_name)` is a
groupable item set.

    WITH
      Products AS (
        SELECT 'shirt' AS product_type, 't-shirt' AS product_name, 3 AS product_count UNION ALL
        SELECT 'shirt', 't-shirt', 8 UNION ALL
        SELECT 'shirt', 'polo', 25 UNION ALL
        SELECT 'pants', 'jeans', 6
      )
    SELECT product_type, product_name, SUM(product_count) AS product_sum
    FROM Products
    GROUP BY ROLLUP (product_type, (product_type, product_name))
    ORDER BY product_type, product_name;

    /*---+---+---+
     | product_type | product_name | product_sum |
     +---+---+---+
     | NULL         | NULL         | 42          |
     | pants        | NULL         | 6           |
     | pants        | jeans        | 6           |
     | shirt        | NULL         | 36          |
     | shirt        | polo         | 25          |
     | shirt        | t-shirt      | 11          |
     +---+---+---*/

### Group rows by `CUBE`

```
GROUP BY CUBE ( grouping_list )

grouping_list:
  { groupable_item | groupable_item_set }[, ...]

groupable_item_set:
  ( groupable_item[, ...] )
```

**Description**

The `GROUP BY CUBE` clause produces aggregated data for all *grouping set*
permutations. A grouping set is a group of columns by which rows
can be grouped together. This clause is helpful if you need to create a
[contingency table](https://en.wikipedia.org/wiki/Contingency_table) to find interrelationships
between items in a set of data.

**Definitions**

- `grouping_list`: A list of items that you can add to the `GROUPING SETS` clause. This is used to create a generated list of grouping sets when the query is run.
- `groupable_item`: Group rows in a table that share common values for certain columns. To learn more, see [Group rows by groupable items](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#group_by_grouping_item). [Anonymous `STRUCT` values](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#tuple_syntax) aren't allowed.
- `groupable_item_set`: Group rows by a set of [groupable items](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#group_by_grouping_item).

**Details**

`GROUP BY CUBE` is similar to `GROUP BY ROLLUP`, except that it takes a
grouping list and generates grouping sets from all permutations in this
list, including an empty grouping set. In the empty grouping set, all rows
are aggregated down to a single group.

For example, `GROUP BY CUBE (a, b, c)` generates the following
grouping sets from the grouping list, `a, b, c`, and then produces
aggregated rows for each of them:

- `(a, b, c)`
- `(a, b)`
- `(a, c)`
- `(a)`
- `(b, c)`
- `(b)`
- `(c)`
- `()`

Here is an example that includes groupable item sets in
`GROUP BY CUBE (a, (b, c), d)`:

| Conceptual grouping sets | Actual grouping sets |
|---|---|
| `(a, (b, c), d)` | `(a, b, c, d)` |
| `(a, (b, c))` | `(a, b, c)` |
| `(a, d)` | `(a, d)` |
| `(a)` | `(a)` |
| `((b, c), d)` | `(b, c, d)` |
| `((b, c))` | `(b, c)` |
| `(d)` | `(d)` |
| `()` | `()` |

When evaluating the results for a particular grouping set,
expressions that aren't in the grouping set are aggregated and produce a
`NULL` placeholder.

You can filter results by specific groupable items. To learn more, see the
[`GROUPING` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#grouping)

`CUBE` allows up to 12 groupable items (equivalent to 4096 grouping sets).

**Examples**

The following query groups rows by all combinations of `product_type` and
`product_name` to produce a contingency table:

    -- GROUP BY with CUBE
    WITH
      Products AS (
        SELECT 'shirt' AS product_type, 't-shirt' AS product_name, 3 AS product_count UNION ALL
        SELECT 'shirt', 't-shirt', 8 UNION ALL
        SELECT 'shirt', 'polo', 25 UNION ALL
        SELECT 'pants', 'jeans', 6
      )
    SELECT product_type, product_name, SUM(product_count) AS product_sum
    FROM Products
    GROUP BY CUBE (product_type, product_name)
    ORDER BY product_type, product_name;

    /*---+---+---+
     | product_type | product_name | product_sum |
     +---+---+---+
     | NULL         | NULL         | 42          |
     | NULL         | jeans        | 6           |
     | NULL         | polo         | 25          |
     | NULL         | t-shirt      | 11          |
     | pants        | NULL         | 6           |
     | pants        | jeans        | 6           |
     | shirt        | NULL         | 36          |
     | shirt        | polo         | 25          |
     | shirt        | t-shirt      | 11          |
     +---+---+---*/

You can include groupable item sets in a `GROUP BY CUBE` clause.
In the following example, `(product_type, product_name)` is a
groupable item set.

    WITH
      Products AS (
        SELECT 'shirt' AS product_type, 't-shirt' AS product_name, 3 AS product_count UNION ALL
        SELECT 'shirt', 't-shirt', 8 UNION ALL
        SELECT 'shirt', 'polo', 25 UNION ALL
        SELECT 'pants', 'jeans', 6
      )
    SELECT product_type, product_name, SUM(product_count) AS product_sum
    FROM Products
    GROUP BY CUBE (product_type, (product_type, product_name))
    ORDER BY product_type, product_name;

    /*---+---+---+
     | product_type | product_name | product_sum |
     +---+---+---+
     | NULL         | NULL         | 42          |
     | pants        | NULL         | 6           |
     | pants        | jeans        | 6           |
     | pants        | jeans        | 6           |
     | shirt        | NULL         | 36          |
     | shirt        | polo         | 25          |
     | shirt        | polo         | 25          |
     | shirt        | t-shirt      | 11          |
     | shirt        | t-shirt      | 11          |
     +---+---+---*/

## `HAVING` clause

```
HAVING bool_expression
```

The `HAVING` clause filters the results produced by `GROUP BY` or
aggregation. `GROUP BY` or aggregation must be present in the query. If
aggregation is present, the `HAVING` clause is evaluated once for every
aggregated row in the result set.

Only rows whose `bool_expression` evaluates to `TRUE` are included. Rows
whose `bool_expression` evaluates to `NULL` or `FALSE` are
discarded.

The evaluation of a query with a `HAVING` clause is typically completed in this
order:

- `FROM`
- `WHERE`
- `GROUP BY` and aggregation
- `HAVING`
- `WINDOW`
- `QUALIFY`
- `DISTINCT`
- `ORDER BY`
- `LIMIT`

Evaluation order doesn't always match syntax order.

The `HAVING` clause references columns available via the `FROM` clause, as
well as `SELECT` list aliases. Expressions referenced in the `HAVING` clause
must either appear in the `GROUP BY` clause or they must be the result of an
aggregate function:

    SELECT LastName
    FROM Roster
    GROUP BY LastName
    HAVING SUM(PointsScored) > 15;

If a query contains aliases in the `SELECT` clause, those aliases override names
in a `FROM` clause.

    SELECT LastName, SUM(PointsScored) AS ps
    FROM Roster
    GROUP BY LastName
    HAVING ps > 0;

### Mandatory aggregation

Aggregation doesn't have to be present in the `HAVING` clause itself, but
aggregation must be present in at least one of the following forms:

#### Aggregation function in the `SELECT` list.

    SELECT LastName, SUM(PointsScored) AS total
    FROM PlayerStats
    GROUP BY LastName
    HAVING total > 15;

#### Aggregation function in the `HAVING` clause.

    SELECT LastName
    FROM PlayerStats
    GROUP BY LastName
    HAVING SUM(PointsScored) > 15;

#### Aggregation in both the `SELECT` list and `HAVING` clause.

When aggregation functions are present in both the `SELECT` list and `HAVING`
clause, the aggregation functions and the columns they reference don't need
to be the same. In the example below, the two aggregation functions,
`COUNT()` and `SUM()`, are different and also use different columns.

    SELECT LastName, COUNT(*)
    FROM PlayerStats
    GROUP BY LastName
    HAVING SUM(PointsScored) > 15;

## `ORDER BY` clause

```
ORDER BY expression
  [{ ASC | DESC }]
  [{ NULLS FIRST | NULLS LAST }]
  [, ...]
```

The `ORDER BY` clause specifies a column or expression as the sort criterion for
the result set. If an `ORDER BY` clause isn't present, the order of the results
of a query isn't defined. Column aliases from a `FROM` clause or `SELECT` list
are allowed. If a query contains aliases in the `SELECT` clause, those aliases
override names in the corresponding `FROM` clause. The data type of
`expression` must be [orderable](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#orderable_data_types).

**Optional Clauses**

- `NULLS FIRST | NULLS LAST`:
  - `NULLS FIRST`: Sort null values before non-null values.
  - `NULLS LAST`. Sort null values after non-null values.
- `ASC | DESC`: Sort the results in ascending or descending order of `expression` values. `ASC` is the default value. If null ordering isn't specified with `NULLS FIRST` or `NULLS LAST`:
  - `NULLS FIRST` is applied by default if the sort order is ascending.
  - `NULLS LAST` is applied by default if the sort order is descending.

**Examples**

Use the default sort order (ascending).

    SELECT x, y
    FROM (SELECT 1 AS x, true AS y UNION ALL
          SELECT 9, true UNION ALL
          SELECT NULL, false)
    ORDER BY x;

    /*---+---+
     | x    | y     |
     +---+---+
     | NULL | false |
     | 1    | true  |
     | 9    | true  |
     +---+---*/

Use the default sort order (ascending), but return null values last.

    SELECT x, y
    FROM (SELECT 1 AS x, true AS y UNION ALL
          SELECT 9, true UNION ALL
          SELECT NULL, false)
    ORDER BY x NULLS LAST;

    /*---+---+
     | x    | y     |
     +---+---+
     | 1    | true  |
     | 9    | true  |
     | NULL | false |
     +---+---*/

Use descending sort order.

    SELECT x, y
    FROM (SELECT 1 AS x, true AS y UNION ALL
          SELECT 9, true UNION ALL
          SELECT NULL, false)
    ORDER BY x DESC;

    /*---+---+
     | x    | y     |
     +---+---+
     | 9    | true  |
     | 1    | true  |
     | NULL | false |
     +---+---*/

Use descending sort order, but return null values first.

    SELECT x, y
    FROM (SELECT 1 AS x, true AS y UNION ALL
          SELECT 9, true UNION ALL
          SELECT NULL, false)
    ORDER BY x DESC NULLS FIRST;

    /*---+---+
     | x    | y     |
     +---+---+
     | NULL | false |
     | 9    | true  |
     | 1    | true  |
     +---+---*/

It's possible to order by multiple columns. In the example below, the result
set is ordered first by `SchoolID` and then by `LastName`:

    SELECT LastName, PointsScored, OpponentID
    FROM PlayerStats
    ORDER BY SchoolID, LastName;

When used in conjunction with
[set operators](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#set_operators),
the `ORDER BY` clause applies to the result set of the entire query; it doesn't
apply only to the closest `SELECT` statement. For this reason, it can be helpful
(though it isn't required) to use parentheses to show the scope of the `ORDER
BY`.

This query without parentheses:

    SELECT * FROM Roster
    UNION ALL
    SELECT * FROM TeamMascot
    ORDER BY SchoolID;

is equivalent to this query with parentheses:

    ( SELECT * FROM Roster
      UNION ALL
      SELECT * FROM TeamMascot )
    ORDER BY SchoolID;

but isn't equivalent to this query, where the `ORDER BY` clause applies only to
the second `SELECT` statement:

    SELECT * FROM Roster
    UNION ALL
    ( SELECT * FROM TeamMascot
      ORDER BY SchoolID );

You can also use integer literals as column references in `ORDER BY` clauses. An
integer literal becomes an ordinal (for example, counting starts at 1) into
the `SELECT` list.

Example - the following two queries are equivalent:

    SELECT SUM(PointsScored), LastName
    FROM PlayerStats
    GROUP BY LastName
    ORDER BY LastName;

    SELECT SUM(PointsScored), LastName
    FROM PlayerStats
    GROUP BY 2
    ORDER BY 2;

## `QUALIFY` clause

```
QUALIFY bool_expression
```

The `QUALIFY` clause filters the results of window functions.
A window function is required to be present in the `QUALIFY` clause or the
`SELECT` list.

Only rows whose `bool_expression` evaluates to `TRUE` are included. Rows
whose `bool_expression` evaluates to `NULL` or `FALSE` are
discarded.

The evaluation of a query with a `QUALIFY` clause is typically completed in this
order:

- `FROM`
- `WHERE`
- `GROUP BY` and aggregation
- `HAVING`
- `WINDOW`
- `QUALIFY`
- `DISTINCT`
- `ORDER BY`
- `LIMIT`

Evaluation order doesn't always match syntax order.

**Examples**

The following query returns the most popular vegetables in the
[`Produce`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/window-function-calls#produce_table) table and their rank.

    SELECT
      item,
      RANK() OVER (PARTITION BY category ORDER BY purchases DESC) as rank
    FROM Produce
    WHERE Produce.category = 'vegetable'
    QUALIFY rank <= 3

    /*---+---+
     | item    | rank |
     +---+---+
     | kale    | 1    |
     | lettuce | 2    |
     | cabbage | 3    |
     +---+---*/

You don't have to include a window function in the `SELECT` list to use
`QUALIFY`. The following query returns the most popular vegetables in the
[`Produce`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/window-function-calls#produce_table) table.

    SELECT item
    FROM Produce
    WHERE Produce.category = 'vegetable'
    QUALIFY RANK() OVER (PARTITION BY category ORDER BY purchases DESC) <= 3

    /*---+
     | item    |
     +---+
     | kale    |
     | lettuce |
     | cabbage |
     +---*/

## `WINDOW` clause

```
WINDOW named_window_expression [, ...]

named_window_expression:
  named_window AS { named_window | ( [ window_specification ] ) }
```

A `WINDOW` clause defines a list of named windows.
A named window represents a group of rows in a table upon which to use a
[window function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/window-function-calls). A named window can be defined with
a [window specification](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/window-function-calls#def_window_spec) or reference another
named window. If another named window is referenced, the definition of the
referenced window must precede the referencing window.

**Examples**

These examples reference a table called [`Produce`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/window-function-calls#produce_table).
They all return the same [result](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/window-function-calls#def_use_named_window). Note the different
ways you can combine named windows and use them in a window function's
`OVER` clause.

    SELECT item, purchases, category, LAST_VALUE(item)
      OVER (item_window) AS most_popular
    FROM Produce
    WINDOW item_window AS (
      PARTITION BY category
      ORDER BY purchases
      ROWS BETWEEN 2 PRECEDING AND 2 FOLLOWING)

    SELECT item, purchases, category, LAST_VALUE(item)
      OVER (d) AS most_popular
    FROM Produce
    WINDOW
      a AS (PARTITION BY category),
      b AS (a ORDER BY purchases),
      c AS (b ROWS BETWEEN 2 PRECEDING AND 2 FOLLOWING),
      d AS (c)

    SELECT item, purchases, category, LAST_VALUE(item)
      OVER (c ROWS BETWEEN 2 PRECEDING AND 2 FOLLOWING) AS most_popular
    FROM Produce
    WINDOW
      a AS (PARTITION BY category),
      b AS (a ORDER BY purchases),
      c AS b

## Set operators

```
  query_expr
  [ { INNER | [ { FULL | LEFT } [ OUTER ] ] } ]
  {
    UNION { ALL | DISTINCT } |
    INTERSECT DISTINCT |
    EXCEPT DISTINCT
  }
  [ { BY NAME [ ON (column_list) ] | [ STRICT ] CORRESPONDING [ BY (column_list) ] } ]
  query_expr
```

Set operators combine or filter
results from two or more input queries into a single result set.

**Definitions**

- `query_expr`: One of two input queries whose results are combined or filtered into a single result set.
- `UNION`: Returns the combined results of the left and right input queries. Values in columns that are matched by position are concatenated vertically.
- `INTERSECT`: Returns rows that are found in the results of both the left and right input queries.
- `EXCEPT`: Returns rows from the left input query that aren't present in the right input query.
- `ALL`: Executes the set operation on all rows.
- `DISTINCT`: Excludes duplicate rows in the set operation.
- `BY NAME`, `CORRESPONDING`: Matches columns by name instead of by position. The `BY NAME` modifier is equivalent to `STRICT CORRESPONDING`. For details, see [`BY NAME` or `CORRESPONDING`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#by_name_or_corresponding).
- `INNER`, `FULL | LEFT [OUTER]`, `STRICT`, `ON`, `BY`: Adjust how the `BY NAME` or `CORRESPONDING` modifier behaves when the column names don't match exactly. For details, see [`BY NAME` or `CORRESPONDING`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#by_name_or_corresponding).

**Positional column matching**

By default, columns are matched positionally and follow these rules. If the
`BY NAME` or `CORRESPONDING` modifier is
used, columns are matched by name, as described in the next section.

- Columns from input queries are matched by their position in the queries. That is, the first column in the first input query is paired with the first column in the second input query and so on.
- The input queries on each side of the operator must return the same number of columns.

**Name-based column matching**

To make set operations match columns by name instead of by column position,
use the [`BY NAME` or `CORRESPONDING`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#by_name_or_corresponding) modifier.

With `BY NAME` or `STRICT CORRESPONDING`, the same column names
must exist in each input, but they can be in different orders. Additional
modifiers can be used to handle cases where the columns don't exactly match.

The `BY NAME` modifier is equivalent to `STRICT CORRESPONDING`, but
the `BY NAME` modifier is recommended because it's shorter and clearer.

Example:

    SELECT 1 AS one_digit, 10 AS two_digit
    UNION ALL BY NAME
    SELECT 20 AS two_digit, 2 AS one_digit;

    -- Column values match by name and not position in query.
    /*---+---+
     | one_digit | two_digit |
     +---+---+
     | 1         | 10        |
     | 2         | 20        |
     +---+---*/

**Other column-related rules**

- For set operations other than `UNION
  ALL`, all column types must support equality comparison.
- The results of the set operation always use the column names from the first input query.
- The results of the set operation always use the supertypes of input types in corresponding columns, so paired columns must also have either the same data type or a common supertype.

**Parenthesized set operators**

- Parentheses must be used to separate different set operations. Set operations like `UNION ALL` and `UNION DISTINCT` are considered different.
- Parentheses are also used to group set operations and control order of operations. In `EXCEPT` set operations, for example, query results can vary depending on the operation grouping.

The following examples illustrate the use of parentheses with set
operations:

    -- Same set operations, no parentheses.
    query1
    UNION ALL
    query2
    UNION ALL
    query3;

    -- Different set operations, parentheses needed.
    query1
    UNION ALL
    (
      query2
      UNION DISTINCT
      query3
    );

    -- Invalid
    query1
    UNION ALL
    query2
    UNION DISTINCT
    query3;

    -- Same set operations, no parentheses.
    query1
    EXCEPT DISTINCT
    query2
    EXCEPT DISTINCT
    query3;

    -- Equivalent query with optional parentheses, returns same results.
    (
      query1
      EXCEPT DISTINCT
      query2
    )
    EXCEPT DISTINCT
    query3;

    -- Different execution order with a subquery, parentheses needed.
    query1
    EXCEPT DISTINCT
    (
      query2
      EXCEPT DISTINCT
      query3
    );

**Set operator behavior with duplicate rows**

Consider a given row `R` that appears exactly `m` times in the first input query
and `n` times in the second input query, where `m >= 0` and `n >= 0`:

- For `UNION ALL`, row `R` appears exactly `m + n` times in the result.
- For `UNION DISTINCT`, the `DISTINCT` is computed after the `UNION` is computed, so row `R` appears exactly one time.
- For `INTERSECT DISTINCT`, row `R` appears once in the output if `m > 0` and `n > 0`.
- For `EXCEPT DISTINCT`, row `R` appears once in the output if `m > 0` and `n = 0`.
- If more than two input queries are used, the above operations generalize and the output is the same as if the input queries were combined incrementally from left to right.

**BY NAME or CORRESPONDING**

Use the `BY NAME` or `CORRESPONDING` modifier
with set operations to match columns by name instead of by position.
The `BY NAME` modifier is equivalent to `STRICT CORRESPONDING`, but the
`BY NAME` modifier is recommended because it's shorter and clearer.
You can use mode prefixes to adjust how
the `BY NAME` or `CORRESPONDING` modifier
behaves when the column names don't match exactly.

- `BY NAME`: Matches columns by name instead of by position.
  - Both input queries must have the same set of column names, but column order can be different. If a column in one input query doesn't appear in the other query, an error is raised.
  - Input queries can't contain duplicate columns.
  - Input queries that produce [value tables](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#value_tables) aren't supported.
- `INNER`: Adjusts the `BY NAME` modifier behavior so that columns that appear in both input queries are included in the query results and any other columns are excluded.
  - No error is raised for the excluded columns that appear in one input query but not in the other input query.
  - At least one column must be common in both left and right input queries.
- `FULL [OUTER]`: Adjusts the `BY NAME` modifier behavior so that all columns from both input queries are included in the query results, even if some columns aren't present in both queries.
  - Columns from the left input query are returned first, followed by unique columns from the right input query.
  - For columns in one input query that aren't present in the other query, a `NULL` value is added as its column value for the other query in the results.
- `LEFT [OUTER]`: Adjusts the `BY NAME` modifier so that all columns from the left input query are included in the results, even if some columns in the left query aren't present in the right query.
  - For columns in the left query that aren't in the right query, a `NULL` value is added as its column value for the right query in the results.
  - At least one column name must be common in both left and right input queries.
- `OUTER`: If used alone, equivalent to `FULL OUTER`.
- `ON (column_list)`: Used after the `BY NAME` modifier to specify a comma-separated list of column names and the column order to return from the input queries.
  - If `BY NAME ON (column_list)` is used alone without mode prefixes like `INNER` or `FULL | LEFT [OUTER]`, then both the left and right input queries must contain all the columns in the `column_list`.
  - If any mode prefixes are used, then any column names not in the `column_list` are excluded from the results according to the mode used.
- `CORRESPONDING`: Equivalent to `INNER...BY NAME`.
  - Supports `FULL | LEFT [OUTER]` modes the same way they're supported by the `BY NAME` modifier.
  - Supports `INNER` mode, but this mode has no effect. The `INNER` mode is used with the `BY NAME` modifier to exclude unmatched columns between input queries, which is the default behavior of the `CORRESPONDING` modifier. Therefore, using `INNER...CORRESPONDING` produces the same results as `CORRESPONDING`.
- `STRICT`: Adjusts the `CORRESPONDING` modifier to be equivalent to the default `BY NAME` modifier, where input queries must have the same set of column names.
- `BY (column_list)`: Equivalent to `ON (column_list)` with `BY NAME`.

The following table shows the equivalent syntaxes between the `BY NAME` and
`CORRESPONDING` modifiers, using the `UNION ALL` set operator as an example:

| BY NAME syntax | Equivalent CORRESPONDING syntax |
|---|---|
| `UNION ALL BY NAME` | `UNION ALL STRICT CORRESPONDING` |
| `INNER UNION ALL BY NAME` | `UNION ALL CORRESPONDING` |
| `{LEFT | FULL} [OUTER] UNION ALL BY NAME` | `{LEFT | FULL} [OUTER] UNION ALL CORRESPONDING` |
| `[FULL] OUTER UNION ALL BY NAME` | `[FULL] OUTER UNION ALL CORRESPONDING` |
| `UNION ALL BY NAME ON (col1, col2, ...)` | `UNION ALL STRICT CORRESPONDING BY (col1, col2, ...)` |

The following table shows the behavior of the mode prefixes for the
`BY NAME` and `CORRESPONDING` modifiers
when left and right input columns don't match:

| Mode prefix and modifier | Behavior when left and right input columns don't match |
|---|---|
| `BY NAME` (no prefix) or `STRICT CORRESPONDING` | Error, all columns must match in both inputs. |
| `INNER BY NAME` or `CORRESPONDING` (no prefix) | Drop all unmatched columns in both inputs. |
| `FULL [OUTER] BY NAME` or `FULL [OUTER] CORRESPONDING` | Include all columns from both inputs. For column values that exist in one input but not in another, add `NULL` values. |
| `LEFT [OUTER] BY NAME` or `LEFT [OUTER] CORRESPONDING` | Include all columns from the left input. For column values that exist in the left input but not in the right input, add `NULL` values. Drop any columns from the right input that don't exist in the left input. |

For example set operations with modifiers, see the sections for each set
operator, such as [`UNION`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#union).

### `UNION`

The `UNION` operator returns the combined results of the left and right input
queries. Columns are matched according to the rules described previously and
rows are concatenated vertically.

**Examples**

    SELECT * FROM UNNEST(ARRAY<INT64>[1, 2, 3]) AS number
    UNION ALL
    SELECT 1;

    /*---+
     | number |
     +---+
     | 1      |
     | 2      |
     | 3      |
     | 1      |
     +---*/

    SELECT * FROM UNNEST(ARRAY<INT64>[1, 2, 3]) AS number
    UNION DISTINCT
    SELECT 1;

    /*---+
     | number |
     +---+
     | 1      |
     | 2      |
     | 3      |
     +---*/

The following example shows multiple chained operators:

    SELECT * FROM UNNEST(ARRAY<INT64>[1, 2, 3]) AS number
    UNION DISTINCT
    SELECT 1
    UNION DISTINCT
    SELECT 2;

    /*---+
     | number |
     +---+
     | 1      |
     | 2      |
     | 3      |
     +---*/

The following example shows input queries with multiple columns. Both
queries specify the same column names but in different orders. As a result, the
column values are matched by column position in the input query and the column
names are ignored.

    SELECT 1 AS one_digit, 10 AS two_digit
    UNION ALL
    SELECT 20 AS two_digit, 2 AS one_digit;

    -- Column values are matched by position and not column name.
    /*---+---+
     | one_digit | two_digit |
     +---+---+
     | 1         | 10        |
     | 20        | 2         |
     +---+---*/

To resolve this ordering issue, the following example uses the
`BY NAME` modifier to match
the columns by name instead of by position in the query results.

    SELECT 1 AS one_digit, 10 AS two_digit
    UNION ALL BY NAME
    SELECT 20 AS two_digit, 2 AS one_digit;

    -- Column values now match.
    /*---+---+
     | one_digit | two_digit |
     +---+---+
     | 1         | 10        |
     | 2         | 20        |
     +---+---*/

The previous set operation with `BY NAME` is equivalent to using the `STRICT
CORRESPONDING` modifier. The `BY NAME` modifier is recommended because it's
shorter and clearer than the `STRICT CORRESPONDING` modifier.

    SELECT 1 AS one_digit, 10 AS two_digit
    UNION ALL STRICT CORRESPONDING
    SELECT 20 AS two_digit, 2 AS one_digit;

    /*---+---+
     | one_digit | two_digit |
     +---+---+
     | 1         | 10        |
     | 2         | 20        |
     +---+---*/

The following example adds a `three_digit` column to the left input query and a
`four_digit` column to the right input query. Because these columns aren't
present in both queries, the `BY NAME`
modifier would trigger an error. Therefore, the example
adds the `INNER`
mode prefix so that the new columns are excluded from the results, executing the
query successfully.

    SELECT 1 AS one_digit, 10 AS two_digit, 100 AS three_digit
    INNER UNION ALL BY NAME
    SELECT 20 AS two_digit, 2 AS one_digit, 1000 AS four_digit;

    /*---+---+
     | one_digit | two_digit |
     +---+---+
     | 1         | 10        |
     | 2         | 20        |
     +---+---*/

To include the differing columns in the results, the following example uses
the `FULL OUTER` mode prefix to populate `NULL` values for the missing column in
each query.

    SELECT 1 AS one_digit, 10 AS two_digit, 100 AS three_digit
    FULL OUTER UNION ALL BY NAME
    SELECT 20 AS two_digit, 2 AS one_digit, 1000 AS four_digit;

    /*---+---+---+---+
     | one_digit | two_digit | three_digit | four_digit |
     +---+---+---+---+
     | 1         | 10        | 100         | NULL       |
     | 2         | 20        | NULL        | 1000       |
     +---+---+---+---*/

Similarly, the following example uses the `LEFT OUTER` mode prefix to include
the new column from only the left input query and populate a `NULL` value for
the missing column in the right input query.

    SELECT 1 AS one_digit, 10 AS two_digit, 100 AS three_digit
    LEFT OUTER UNION ALL BY NAME
    SELECT 20 AS two_digit, 2 AS one_digit, 1000 AS four_digit;

    /*---+---+---+
     | one_digit | two_digit | three_digit |
     +---+---+---+
     | 1         | 10        | 100         |
     | 2         | 20        | NULL        |
     +---+---+---*/

The following example adds the modifier `ON (column_list)`
to return only the specified columns in the specified order.

    SELECT 1 AS one_digit, 10 AS two_digit, 100 AS three_digit
    FULL OUTER UNION ALL BY NAME ON (three_digit, two_digit)
    SELECT 20 AS two_digit, 2 AS one_digit, 1000 AS four_digit;

    /*---+---+
     | three_digit | two_digit |
     +---+---+
     | 100         | 10        |
     | NULL        | 20        |
     +---+---*/

### `INTERSECT`

The `INTERSECT` operator returns rows that are found in the results of both the
left and right input queries.

**Examples**

    SELECT * FROM UNNEST(ARRAY<INT64>[1, 2, 3, 3, 4]) AS number
    INTERSECT DISTINCT
    SELECT * FROM UNNEST(ARRAY<INT64>[2, 3, 3, 5]) AS number;

    /*---+
     | number |
     +---+
     | 2      |
     | 3      |
     +---*/

The following example shows multiple chained operations:

    SELECT * FROM UNNEST(ARRAY<INT64>[1, 2, 3, 3, 4]) AS number
    INTERSECT DISTINCT
    SELECT * FROM UNNEST(ARRAY<INT64>[2, 3, 3, 5]) AS number
    INTERSECT DISTINCT
    SELECT * FROM UNNEST(ARRAY<INT64>[3, 3, 4, 5]) AS number;

    /*---+
     | number |
     +---+
     | 3      |
     +---*/

The following example shows input queries that specify multiple columns. Both
queries specify the same column names but in different orders. As a result, the
same columns in differing order are considered different columns, so the query
doesn't detect any intersecting row values. Therefore, no results are returned.

    WITH
      NumbersTable AS (
        SELECT 1 AS one_digit, 10 AS two_digit
        UNION ALL
        SELECT 2, 20
        UNION ALL
        SELECT 3, 30
      )
    SELECT one_digit, two_digit FROM NumbersTable
    INTERSECT DISTINCT
    SELECT 10 AS two_digit, 1 AS one_digit;

    -- No intersecting values detected because columns aren't recognized as the same.
    /*---+---+

     +---+---*/

To resolve this ordering issue, the following example uses the
`BY NAME` modifier to match
the columns by name instead of by position in the query results.

    WITH
      NumbersTable AS (
        SELECT 1 AS one_digit, 10 AS two_digit
        UNION ALL
        SELECT 2, 20
        UNION ALL
        SELECT 3, 30
      )
    SELECT one_digit, two_digit FROM NumbersTable
    INTERSECT DISTINCT BY NAME
    SELECT 10 AS two_digit, 1 AS one_digit;

    /*---+---+
     | one_digit | two_digit |
     +---+---+
     | 1         | 10        |
     +---+---*/

The previous set operation with `BY NAME` is equivalent to using the `STRICT
CORRESPONDING` modifier. The `BY NAME` modifier is recommended because it's
shorter and clearer than the `STRICT CORRESPONDING` modifier.

    WITH
      NumbersTable AS (
        SELECT 1 AS one_digit, 10 AS two_digit
        UNION ALL
        SELECT 2, 20
        UNION ALL
        SELECT 3, 30
      )
    SELECT one_digit, two_digit FROM NumbersTable
    INTERSECT DISTINCT STRICT CORRESPONDING
    SELECT 10 AS two_digit, 1 AS one_digit;

    /*---+---+
     | one_digit | two_digit |
     +---+---+
     | 1         | 10        |
     +---+---*/

For more syntax examples with the `BY NAME`
modifier, see the [`UNION`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#union) set operator.

### `EXCEPT`

The `EXCEPT` operator returns rows from the left input query that aren't present
in the right input query.

**Examples**

    SELECT * FROM UNNEST(ARRAY<INT64>[1, 2, 3, 3, 4]) AS number
    EXCEPT DISTINCT
    SELECT * FROM UNNEST(ARRAY<INT64>[1, 2]) AS number;

    /*---+
     | number |
     +---+
     | 3      |
     | 4      |
     +---*/

The following example shows multiple chained operations:

    SELECT * FROM UNNEST(ARRAY<INT64>[1, 2, 3, 3, 4]) AS number
    EXCEPT DISTINCT
    SELECT * FROM UNNEST(ARRAY<INT64>[1, 2]) AS number
    EXCEPT DISTINCT
    SELECT * FROM UNNEST(ARRAY<INT64>[1, 4]) AS number;

    /*---+
     | number |
     +---+
     | 3      |
     +---*/

The following example modifies the execution behavior of the set operations. The
first input query is used against the result of the last two input queries
instead of the values of the last two queries individually. In this example,
the `EXCEPT` result of the last two input queries is `2`. Therefore, the
`EXCEPT` results of the entire query are any values other than `2` in the first
input query.

    SELECT * FROM UNNEST(ARRAY<INT64>[1, 2, 3, 3, 4]) AS number
    EXCEPT DISTINCT
    (
      SELECT * FROM UNNEST(ARRAY<INT64>[1, 2]) AS number
      EXCEPT DISTINCT
      SELECT * FROM UNNEST(ARRAY<INT64>[1, 4]) AS number
    );

    /*---+
     | number |
     +---+
     | 1      |
     | 3      |
     | 4      |
     +---*/

The following example shows input queries that specify multiple columns. Both
queries specify the same column names but in different orders. As a result, the
same columns in differing order are considered different columns, so the query
doesn't detect any common rows that should be excluded. Therefore, all column
values from the left input query are returned with no exclusions.

    WITH
      NumbersTable AS (
        SELECT 1 AS one_digit, 10 AS two_digit
        UNION ALL
        SELECT 2, 20
        UNION ALL
        SELECT 3, 30
      )
    SELECT one_digit, two_digit FROM NumbersTable
    EXCEPT DISTINCT
    SELECT 10 AS two_digit, 1 AS one_digit;

    -- No values excluded because columns aren't recognized as the same.
    /*---+---+
     | one_digit | two_digit |
     +---+---+
     | 1         | 10        |
     | 2         | 20        |
     | 3         | 30        |
     +---+---*/

To resolve this ordering issue, the following example uses the
`BY NAME` modifier to match
the columns by name instead of by position in the query results.

    WITH
      NumbersTable AS (
        SELECT 1 AS one_digit, 10 AS two_digit
        UNION ALL
        SELECT 2, 20
        UNION ALL
        SELECT 3, 30
      )
    SELECT one_digit, two_digit FROM NumbersTable
    EXCEPT DISTINCT BY NAME
    SELECT 10 AS two_digit, 1 AS one_digit;

    /*---+---+
     | one_digit | two_digit |
     +---+---+
     | 2         | 20        |
     | 3         | 30        |
     +---+---*/

The previous set operation with `BY NAME` is equivalent to using the `STRICT
CORRESPONDING` modifier. The `BY NAME` modifier is recommended because it's
shorter and clearer than the `STRICT CORRESPONDING` modifier.

    WITH
      NumbersTable AS (
        SELECT 1 AS one_digit, 10 AS two_digit
        UNION ALL
        SELECT 2, 20
        UNION ALL
        SELECT 3, 30
      )
    SELECT one_digit, two_digit FROM NumbersTable
    EXCEPT DISTINCT STRICT CORRESPONDING
    SELECT 10 AS two_digit, 1 AS one_digit;

    /*---+---+
     | one_digit | two_digit |
     +---+---+
     | 2         | 20        |
     | 3         | 30        |
     +---+---*/

For more syntax examples with the `BY NAME`
modifier, see the [`UNION`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#union) set operator.

## `LIMIT` and `OFFSET` clause

    LIMIT count [ OFFSET skip_rows ]

Limits the number of rows to return in a query. Optionally includes
the ability to skip over rows.

**Definitions**

- `LIMIT`: Limits the number of rows to produce.

  `count` is an `INT64` constant expression that represents the
  non-negative, non-`NULL` limit. No more than `count` rows are produced.
  `LIMIT 0` returns 0 rows.

  If there is a set operation, `LIMIT` is applied after the set operation is
  evaluated.
- `OFFSET`: Skips a specific number of rows before applying `LIMIT`.

  `skip_rows` is an `INT64` constant expression that represents
  the non-negative, non-`NULL` number of rows to skip.

**Details**

The rows that are returned by `LIMIT` and `OFFSET` have undefined order unless
these clauses are used after `ORDER BY`.

A constant expression can be represented by a general expression, literal, or
parameter value.

> [!NOTE]
> **Note:** Although the `LIMIT` clause limits the rows that a query produces, it doesn't limit the amount of data processed by that query.

**Examples**

    SELECT *
    FROM UNNEST(ARRAY<STRING>['a', 'b', 'c', 'd', 'e']) AS letter
    ORDER BY letter ASC LIMIT 2;

    /*---+
     | letter  |
     +---+
     | a       |
     | b       |
     +---*/

    SELECT *
    FROM UNNEST(ARRAY<STRING>['a', 'b', 'c', 'd', 'e']) AS letter
    ORDER BY letter ASC LIMIT 3 OFFSET 1;

    /*---+
     | letter  |
     +---+
     | b       |
     | c       |
     | d       |
     +---*/

## `WITH` clause

```
WITH [ RECURSIVE ] { non_recursive_cte | recursive_cte }[, ...]
```

A `WITH` clause contains one or more common table expressions (CTEs).
A CTE acts like a temporary table that you can reference within a single
query expression. Each CTE binds the results of a [subquery](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/subqueries)
to a table name, which can be used elsewhere in the same query expression,
but [rules apply](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#cte_rules).

CTEs can be [non-recursive](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#simple_cte) or
[recursive](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#recursive_cte) and you can include both of these in your
`WITH` clause. A recursive CTE references itself, where a
non-recursive CTE doesn't. If a recursive CTE is included in the `WITH` clause,
the `RECURSIVE` keyword must also be included.

You can include the `RECURSIVE` keyword in a `WITH` clause even if no
recursive CTEs are present. You can learn more about the `RECURSIVE` keyword
[here](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#recursive_keyword).

GoogleSQL only materializes
the results of recursive CTEs, but doesn't materialize the results
of non-recursive CTEs inside the `WITH` clause. If a non-recursive CTE is
referenced in multiple places in a query, then the CTE is executed once for each
reference. The `WITH` clause with non-recursive CTEs is useful primarily for
readability.

### `RECURSIVE` keyword

A `WITH` clause can optionally include the `RECURSIVE` keyword, which does
two things:

- Enables recursion in the `WITH` clause. If this keyword isn't present, you can only include non-recursive common table expressions (CTEs). If this keyword is present, you can use both [recursive](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#recursive_cte) and [non-recursive](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#simple_cte) CTEs.
- [Changes the visibility](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#cte_visibility) of CTEs in the `WITH` clause. If this keyword isn't present, a CTE is only visible to CTEs defined after it in the `WITH` clause. If this keyword is present, a CTE is visible to all CTEs in the `WITH` clause where it was defined.

### Non-recursive CTEs

```
non_recursive_cte:
  cte_name AS ( query_expr )
```

A non-recursive common table expression (CTE) contains
a non-recursive [subquery](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/subqueries)
and a name associated with the CTE.

- A non-recursive CTE can't reference itself.
- A non-recursive CTE can be referenced by the query expression that contains the `WITH` clause, but [rules apply](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#cte_rules).

##### Examples

In this example, a `WITH` clause defines two non-recursive CTEs that
are referenced in the related set operation, where one CTE is referenced by
each of the set operation's input query expressions:

    WITH subQ1 AS (SELECT SchoolID FROM Roster),
         subQ2 AS (SELECT OpponentID FROM PlayerStats)
    SELECT * FROM subQ1
    UNION ALL
    SELECT * FROM subQ2

You can break up more complex queries into a `WITH` clause and
`WITH` `SELECT` statement instead of writing nested table subqueries.
For example:

    WITH q1 AS (my_query)
    SELECT *
    FROM
      (WITH q2 AS (SELECT * FROM q1) SELECT * FROM q2)

    WITH q1 AS (my_query)
    SELECT *
    FROM
      (WITH q2 AS (SELECT * FROM q1),  # q1 resolves to my_query
            q3 AS (SELECT * FROM q1),  # q1 resolves to my_query
            q1 AS (SELECT * FROM q1),  # q1 (in the query) resolves to my_query
            q4 AS (SELECT * FROM q1)   # q1 resolves to the WITH subquery on the previous line.
        SELECT * FROM q1)              # q1 resolves to the third inner WITH subquery.

### Recursive CTEs

```
recursive_cte:
  cte_name AS ( recursive_union_operation )

recursive_union_operation:
  base_term union_operator recursive_term

base_term:
  query_expr

recursive_term:
  query_expr

union_operator:
  UNION ALL
```

A recursive common table expression (CTE) contains a
recursive [subquery](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/subqueries) and a name associated with the CTE.

- A recursive CTE references itself.
- A recursive CTE can be referenced in the query expression that contains the `WITH` clause, but [rules apply](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#cte_rules).
- When a recursive CTE is defined in a `WITH` clause, the [`RECURSIVE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#recursive_keyword) keyword must be present.

A recursive CTE is defined by a *recursive union operation*. The
recursive union operation defines how input is recursively processed
to produce the final CTE result. The recursive union operation has the
following parts:

- `base_term`: Runs the first iteration of the recursive union operation. This term must follow the [base term rules](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#base_term_rules).
- `union_operator`: The `UNION` operator returns the rows that are from the union of the base term and recursive term. With `UNION ALL`, each row produced in iteration `N` becomes part of the final CTE result and input for iteration `N+1`. Iteration stops when an iteration produces no rows to move into the next iteration.
- `recursive_term`: Runs the remaining iterations. It must include one self-reference (recursive reference) to the recursive CTE. Only this term can include a self-reference. This term must follow the [recursive term rules](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#recursive_cte_rules).

A recursive CTE looks like this:

    WITH RECURSIVE
      T1 AS ( (SELECT 1 AS n) UNION ALL (SELECT n + 1 AS n FROM T1 WHERE n < 3) )
    SELECT n FROM T1

    /*---+
     | n |
     +---+
     | 2 |
     | 1 |
     | 3 |
     +---*/

The first iteration of a recursive union operation runs the base term.
Then, each subsequent iteration runs the recursive term and produces
*new rows* which are unioned with the previous iteration. The recursive
union operation terminates when a recursive term iteration produces no new
rows.

If recursion doesn't terminate, the query fails after reaching 500 iterations.

To learn more about recursive CTEs and troubleshooting iteration limit errors,
see [Work with recursive CTEs](https://docs.cloud.google.com/bigquery/docs/recursive-ctes).

##### Examples of allowed recursive CTEs

This is a simple recursive CTE:

    WITH RECURSIVE
      T1 AS (
        (SELECT 1 AS n) UNION ALL
        (SELECT n + 2 FROM T1 WHERE n < 4))
    SELECT * FROM T1 ORDER BY n

    /*---+
     | n |
     +---+
     | 1 |
     | 3 |
     | 5 |
     +---*/

Multiple subqueries in the same recursive CTE are okay, as
long as each recursion has a cycle length of 1. It's also okay for recursive
entries to depend on non-recursive entries and vice-versa:

    WITH RECURSIVE
      T0 AS (SELECT 1 AS n),
      T1 AS ((SELECT * FROM T0) UNION ALL (SELECT n + 1 FROM T1 WHERE n < 4)),
      T2 AS ((SELECT 1 AS n) UNION ALL (SELECT n + 1 FROM T2 WHERE n < 4)),
      T3 AS (SELECT * FROM T1 INNER JOIN T2 USING (n))
    SELECT * FROM T3 ORDER BY n

    /*---+
     | n |
     +---+
     | 1 |
     | 2 |
     | 3 |
     | 4 |
     +---*/

Aggregate functions can be invoked in subqueries, as long as they aren't
aggregating on the table being defined:

    WITH RECURSIVE
      T0 AS (SELECT * FROM UNNEST ([60, 20, 30])),
      T1 AS ((SELECT 1 AS n) UNION ALL (SELECT n + (SELECT COUNT(*) FROM T0) FROM T1 WHERE n < 4))
    SELECT * FROM T1 ORDER BY n

    /*---+
     | n |
     +---+
     | 1 |
     | 4 |
     +---*/

`INNER JOIN` can be used inside subqueries:

    WITH RECURSIVE
      T0 AS (SELECT 1 AS n),
      T1 AS ((SELECT 1 AS n) UNION ALL (SELECT n + 1 FROM T1 INNER JOIN T0 USING (n)))
    SELECT * FROM T1 ORDER BY n

    /*---+
     | n |
     +---+
     | 1 |
     | 2 |
     +---*/

`CROSS JOIN` can be used inside subqueries:

    WITH RECURSIVE
      T0 AS (SELECT 2 AS p),
      T1 AS ((SELECT 1 AS n) UNION ALL (SELECT T1.n + T0.p FROM T1 CROSS JOIN T0 WHERE T1.n < 4))
    SELECT * FROM T1 CROSS JOIN T0 ORDER BY n

    /*---+---+
     | n | p |
     +---+---+
     | 1 | 2 |
     | 3 | 2 |
     | 5 | 2 |
     +---+---*/

Recursive CTEs can be used inside `CREATE TABLE AS SELECT` statements. The
following example creates a table named `new_table` in `mydataset`:

    CREATE OR REPLACE TABLE `myproject.mydataset.new_table` AS
      WITH RECURSIVE
        T1 AS (SELECT 1 AS n UNION ALL SELECT n + 1 FROM T1 WHERE n < 3)
      SELECT * FROM T1

Recursive CTEs can be used inside `CREATE VIEW AS SELECT` statements. The
following example creates a view named `new_view` in `mydataset`:

    CREATE OR REPLACE VIEW `myproject.mydataset.new_view` AS
      WITH RECURSIVE
        T1 AS (SELECT 1 AS n UNION ALL SELECT n + 1 FROM T1 WHERE n < 3)
      SELECT * FROM T1

Recursive CTEs can be used inside `INSERT` statements. The following example
demonstrates how to insert data into a table by using recursive CTEs:

    -- create a temp table.
    CREATE TEMP TABLE tmp_table (n INT64);

    -- insert some values into the temp table by using recursive CTEs.
    INSERT INTO tmp_table(n)
      WITH RECURSIVE
        T1 AS (SELECT 1 AS n UNION ALL SELECT n + 1 FROM T1 WHERE n < 3)
      SELECT * FROM T1

##### Examples of disallowed recursive CTEs

The following recursive CTE is disallowed because the
self-reference doesn't include a set operator, base term, and
recursive term.

    WITH RECURSIVE
      T1 AS (SELECT * FROM T1)
    SELECT * FROM T1

    -- Error

The following recursive CTE is disallowed because the self-reference to `T1`
is in the base term. The self reference is only allowed in the recursive term.

    WITH RECURSIVE
      T1 AS ((SELECT * FROM T1) UNION ALL (SELECT 1))
    SELECT * FROM T1

    -- Error

The following recursive CTE is disallowed because there are multiple
self-references in the recursive term when there must only be one.

    WITH RECURSIVE
      T1 AS ((SELECT 1 AS n) UNION ALL ((SELECT * FROM T1) UNION ALL (SELECT * FROM T1)))
    SELECT * FROM T1

    -- Error

The following recursive CTE is disallowed because the self-reference is
inside an [expression subquery](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/subqueries#expression_subquery_concepts)

    WITH RECURSIVE
      T1 AS ((SELECT 1 AS n) UNION ALL (SELECT (SELECT n FROM T1)))
    SELECT * FROM T1

    -- Error

The following recursive CTE is disallowed because there is a
self-reference as an argument to a table-valued function (TVF).

    WITH RECURSIVE
      T1 AS (
        (SELECT 1 AS n) UNION ALL
        (SELECT * FROM MY_TVF(T1)))
    SELECT * FROM T1;

    -- Error

The following recursive CTE is disallowed because there is a
self-reference as input to an outer join.

    WITH RECURSIVE
      T0 AS (SELECT 1 AS n),
      T1 AS ((SELECT 1 AS n) UNION ALL (SELECT * FROM T1 FULL OUTER JOIN T0 USING (n)))
    SELECT * FROM T1;

    -- Error

The following recursive CTE is disallowed because you can't use aggregation
with a self-reference.

    WITH RECURSIVE
      T1 AS (
        (SELECT 1 AS n) UNION ALL
        (SELECT COUNT(*) FROM T1))
    SELECT * FROM T1;

    -- Error

The following recursive CTE is disallowed because you can't use the
window function `OVER` clause with a self-reference.

    WITH RECURSIVE
      T1 AS (
        (SELECT 1.0 AS n) UNION ALL
        SELECT 1 + AVG(n) OVER(ROWS between 2 PRECEDING and 0 FOLLOWING) FROM T1 WHERE n < 10)
    SELECT n FROM T1;

    -- Error

The following recursive CTE is disallowed because you can't use a
`LIMIT` clause with a self-reference.

    WITH RECURSIVE
      T1 AS ((SELECT 1 AS n) UNION ALL (SELECT n FROM T1 LIMIT 3))
    SELECT * FROM T1;

    -- Error

The following recursive CTEs are disallowed because you can't use an
`ORDER BY` clause with a self-reference.

    WITH RECURSIVE
      T1 AS ((SELECT 1 AS n) UNION ALL (SELECT n + 1 FROM T1 ORDER BY n))
    SELECT * FROM T1;

    -- Error

The following recursive CTE is disallowed because table `T1` can't be
recursively referenced from inside an inner `WITH` clause

    WITH RECURSIVE
      T1 AS ((SELECT 1 AS n) UNION ALL (WITH t AS (SELECT n FROM T1) SELECT * FROM t))
    SELECT * FROM T1

    -- Error

### CTE rules and constraints

Common table expressions (CTEs) can be referenced inside the query expression
that contains the `WITH` clause.

##### General rules

Here are some general rules and constraints to consider when working with CTEs:

- Each CTE in the same `WITH` clause must have a unique name.
- You must include the [`RECURSIVE` keyword](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#recursive_keyword) keyword if the `WITH` clause contains a recursive CTE.
- The [`RECURSIVE` keyword](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#recursive_keyword) in the `WITH` clause changes the visibility of CTEs to other CTEs in the same `WITH` clause. You can learn more [here](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#recursive_keyword).
- `WITH` isn't allowed inside `WITH RECURSIVE`.
- `WITH RECURSIVE` is allowed in the `SELECT` statement.
- `WITH RECURSIVE` is only allowed at the top level of the query.
- `WITH RECURSIVE` isn't allowed in functions.
- `WITH RECURSIVE` isn't allowed in materialized views.
- The `WITH RECURSIVE` clause can't contain generative AI functions.
- `CREATE RECURSIVE VIEW` isn't supported. To work around this, use the `WITH RECURSIVE` clause as the `query_expression` in the `CREATE VIEW` statement. For more information, see [CREATE VIEW](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_view_statement).
- A local CTE overrides an outer CTE or table with the same name.
- A CTE on a subquery may not reference correlated columns from the outer query.
- `UNION DISTINCT` isn't allowed inside a `WITH RECURSIVE` clause.

##### Base term rules

The following rules apply to the base term in a recursive CTE:

- The base term is required to be non-recursive.
- The base term determines the names and types of all of the table columns.

##### Recursive term rules

The following rules apply to the recursive term in a recursive CTE:

- The recursive term must include exactly one reference to the recursively-defined table in the base term.
- The recursive term must contain the same number of columns as the base term, and the type of each column must be implicitly coercible to the type of the corresponding column in the base term.
- A recursive table reference can't be used as an operand to a `FULL JOIN`, a right operand to a `LEFT JOIN`, or a left operand to a `RIGHT JOIN`.
- A recursive table reference can't be used with the `TABLESAMPLE` operator.
- A recursive table reference can't be used as an operand to a table-valued function (TVF).
- Use of the `IN` and `EXISTS` expression subqueries is limited within the recursive term. For example:
  - `[NOT] IN` and `[NOT] EXISTS` aren't allowed in the `SELECT` clause.
  - `NOT IN` isn't allowed in the `WHERE` clause.

The following rules apply to a subquery inside a recursive term:

- A subquery with a recursive table reference must be a `SELECT` expression, not a set operation, such as `UNION ALL`.
- A subquery can't contain, directly or indirectly, a recursive table reference anywhere outside of its `FROM` clause.
- A subquery with a recursive table reference can't contain an `ORDER BY` or `LIMIT` clause.
- A subquery with a recursive table reference can't invoke aggregate functions.
- A subquery with a recursive table reference can't invoke window functions.
- A subquery with a recursive table reference can't contain the `DISTINCT` keyword or `GROUP BY` clause.

### CTE visibility

The visibility of a common table expression (CTE) within a query expression
is determined by whether or not you add the `RECURSIVE` keyword to the
`WITH` clause where the CTE was defined. You can learn more about these
differences in the following sections.

#### Visibility of CTEs in a `WITH` clause with the `RECURSIVE` keyword

When you include the `RECURSIVE` keyword, references between CTEs in the `WITH`
clause can go backwards and forwards. Cycles aren't allowed.

This is what happens when you have two CTEs that reference
themselves or each other in a `WITH` clause with the `RECURSIVE`
keyword. Assume that `A` is the first CTE and `B` is the second
CTE in the clause:

- A references A = Valid
- A references B = Valid
- B references A = Valid
- A references B references A = Invalid (cycles aren't allowed)

`A` can reference itself because self-references are supported:

    WITH RECURSIVE
      A AS (SELECT 1 AS n UNION ALL (SELECT n + 1 FROM A WHERE n < 3))
    SELECT * FROM A

    /*---+
     | n |
     +---+
     | 1 |
     | 2 |
     | 3 |
     +---*/

`A` can reference `B` because references between CTEs can go forwards:

    WITH RECURSIVE
      A AS (SELECT * FROM B),
      B AS (SELECT 1 AS n)
    SELECT * FROM B

    /*---+
     | n |
     +---+
     | 1 |
     +---*/

`B` can reference `A` because references between CTEs can go backwards:

    WITH RECURSIVE
      A AS (SELECT 1 AS n),
      B AS (SELECT * FROM A)
    SELECT * FROM B

    /*---+
     | n |
     +---+
     | 1 |
     +---*/

This produces an error. `A` and `B` reference each other, which creates a cycle:

    WITH RECURSIVE
      A AS (SELECT * FROM B),
      B AS (SELECT * FROM A)
    SELECT * FROM B

    -- Error

#### Visibility of CTEs in a `WITH` clause without the `RECURSIVE` keyword

When you don't include the `RECURSIVE` keyword in the `WITH` clause,
references between CTEs in the clause can go backward but not forward.

This is what happens when you have two CTEs that reference
themselves or each other in a `WITH` clause without
the `RECURSIVE` keyword. Assume that `A` is the first CTE and `B`
is the second CTE in the clause:

- A references A = Invalid
- A references B = Invalid
- B references A = Valid
- A references B references A = Invalid (cycles aren't allowed)

This produces an error. `A` can't reference itself because self-references
aren't supported:

    WITH
      A AS (SELECT 1 AS n UNION ALL (SELECT n + 1 FROM A WHERE n < 3))
    SELECT * FROM A

    -- Error

This produces an error. `A` can't reference `B` because references between
CTEs can go backwards but not forwards:

    WITH
      A AS (SELECT * FROM B),
      B AS (SELECT 1 AS n)
    SELECT * FROM B

    -- Error

`B` can reference `A` because references between CTEs can go backwards:

    WITH
      A AS (SELECT 1 AS n),
      B AS (SELECT * FROM A)
    SELECT * FROM B

    /*---+
     | n |
     +---+
     | 1 |
     +---*/

This produces an error. `A` and `B` reference each other, which creates a
cycle:

    WITH
      A AS (SELECT * FROM B),
      B AS (SELECT * FROM A)
    SELECT * FROM B

    -- Error

## `AGGREGATION_THRESHOLD` clause

Syntax for an aggregation threshold analysis rule--enforced query:

```
WITH AGGREGATION_THRESHOLD OPTIONS (
  threshold = threshold_amount,
  privacy_unit_column = column_name
)
```

Syntax for an aggregation threshold analysis rule--enforced view:

```
WITH AGGREGATION_THRESHOLD [ OPTIONS (
  [ threshold = threshold_amount ],
  [ privacy_unit_column = column_name ]
) ]
```

**Description**

Use the `AGGREGATION_THRESHOLD` clause to enforce an
aggregation threshold. This clause counts the number of distinct privacy units
(represented by the privacy unit column) for each group, and only outputs the
groups where the distinct privacy unit count satisfies the
aggregation threshold. If you want to use an
aggregation threshold analysis rule that you defined
for a view, use the syntax for an [analysis rule--enforced view](https://docs.cloud.google.com/bigquery/docs/analysis-rules#privacy_view).
When querying a privacy-enforced view, the `AGGREGATION_THRESHOLD` clause does
not need to include the `OPTIONS` clause.

**Definitions:**

- `threshold`: The minimum number of distinct
  privacy units (privacy unit column values) that need to contribute to each row
  in the query results. If a potential row doesn't satisfy this threshold,
  that row is omitted from the query results. `threshold_amount` must be
  a positive `INT64` value.

  If you're using this query with an analysis rule--enforced view, you can
  optionally add this query parameter to override the `threshold` parameter for
  the view. The threshold for the query must be equal to or
  greater than the threshold for the view. If the threshold for the query
  is less than the threshold for the view, an error is produced.
- `privacy_unit_column`: The column that represents the
  privacy unit column. Replace `column_name` with the
  path expression for the column. The first identifier in the path can start
  with either a table name or a column name that's visible in the
  `FROM` clause.

  If you're using this query with an analysis rule--enforced view, you can
  optionally add this query parameter. However, it must match the
  value for `privacy_unit_column` on the view. If it doesn't, an error is
  produced.

**Details**

The following functions can be used on any column in a query with the
`AGGREGATION_THRESHOLD` clause, including the commonly used
`COUNT`, `SUM`, and `AVG` functions:

- `APPROX_COUNT_DISTINCT`
- `AVG`
- `COUNT`
- `COUNTIF`
- `LOGICAL_AND`
- `LOGICAL_OR`
- `SUM`
- `COVAR_POP`
- `COVAR_SAMP`
- `STDDEV_POP`
- `STDDEV_SAMP`
- `VAR_POP`
- `VAR_SAMP`

**Example**

In the following example, an aggregation threshold is enforced
on a query. Notice that some privacy units are dropped because
there aren't enough distinct instances.

    WITH ExamTable AS (
      SELECT "Hansen" AS last_name, "P91" AS test_id, 510 AS test_score UNION ALL
      SELECT "Wang", "U25", 500 UNION ALL
      SELECT "Wang", "C83", 520 UNION ALL
      SELECT "Wang", "U25", 460 UNION ALL
      SELECT "Hansen", "C83", 420 UNION ALL
      SELECT "Hansen", "C83", 560 UNION ALL
      SELECT "Devi", "U25", 580 UNION ALL
      SELECT "Devi", "P91", 480 UNION ALL
      SELECT "Ivanov", "U25", 490 UNION ALL
      SELECT "Ivanov", "P91", 540 UNION ALL
      SELECT "Silva", "U25", 550)
    SELECT WITH AGGREGATION_THRESHOLD
      OPTIONS(threshold=3, privacy_unit_column=last_name)
      test_id,
      COUNT(DISTINCT last_name) AS student_count,
      AVG(test_score) AS avg_test_score
    FROM ExamTable
    GROUP BY test_id;

    /*---+---+---+
     | test_id | student_count | avg_test_score |
     +---+---+---+
     | P91     | 3             | 510.0          |
     | U25     | 4             | 516.0          |
     +---+---+---*/

In the following example, an aggregation threshold analysis rule is enforced on
a view with the same results:

    -- Create a table.
    CREATE OR REPLACE TABLE mydataset.ExamTable AS (
      SELECT "Hansen" AS last_name, "P91" AS test_id, 510 AS test_score UNION ALL
      SELECT "Wang", "U25", 500 UNION ALL
      SELECT "Wang", "C83", 520 UNION ALL
      SELECT "Wang", "U25", 460 UNION ALL
      SELECT "Hansen", "C83", 420 UNION ALL
      SELECT "Hansen", "C83", 560 UNION ALL
      SELECT "Devi", "U25", 580 UNION ALL
      SELECT "Devi", "P91", 480 UNION ALL
      SELECT "Ivanov", "U25", 490 UNION ALL
      SELECT "Ivanov", "P91", 540 UNION ALL
      SELECT "Silva", "U25", 550);

    -- Create a view for the table.
    CREATE OR REPLACE VIEW mydataset.ExamView
    OPTIONS(
      privacy_policy= '{"aggregation_threshold_policy": {"threshold": 3, "privacy_unit_column": "last_name"}}'
    )
    AS ( SELECT * FROM mydataset.ExamTable );

    -- Query the aggregation threshold privacy-policy enforced view.
    SELECT WITH AGGREGATION_THRESHOLD
      test_id,
      COUNT(DISTINCT last_name) AS student_count,
      AVG(test_score) AS avg_test_score
    FROM mydataset.ExamView
    GROUP BY test_id;

    /*---+---+---+
     | test_id | student_count | avg_test_score |
     +---+---+---+
     | P91     | 3             | 510.0          |
     | U25     | 4             | 516.0          |
     +---+---+---*/

In the following example, an aggregation threshold analysis rule is enforced on
the previous view, but the threshold is adjusted from `3` in the view to
`4` in the query:

    SELECT WITH AGGREGATION_THRESHOLD
      OPTIONS(threshold=4)
      test_id,
      COUNT(DISTINCT last_name) AS student_count,
      AVG(test_score) AS avg_test_score
    FROM mydataset.ExamView
    GROUP BY test_id;

    /*---+---+---+
     | test_id | student_count | avg_test_score |
     +---+---+---+
     | U25     | 4             | 516.0          |
     +---+---+---*/

In the following example, an aggregation threshold analysis rule is enforced on
the previous view, but the threshold is adjusted from `3` in the view to
`5` in the query. While the analysis rule is satisfied, the query produces
no data.

    -- No data is produced.
    SELECT WITH AGGREGATION_THRESHOLD
      OPTIONS(threshold=5)
      test_id,
      COUNT(DISTINCT last_name) AS student_count,
      AVG(test_score) AS avg_test_score
    FROM mydataset.ExamView
    GROUP BY test_id;

In the following example, an aggregation threshold analysis rule is enforced on
the previous view, but the threshold is adjusted from `3` in the view to
`2` in the query:

    -- Error: Aggregation threshold is too low.
    SELECT WITH AGGREGATION_THRESHOLD
      OPTIONS(threshold=2)
      test_id,
      COUNT(DISTINCT last_name) AS student_count,
      AVG(test_score) AS avg_test_score
    FROM mydataset.ExamView
    GROUP BY test_id;

In the following example, an aggregation threshold analysis rule is enforced on
the previous view, but the threshold is adjusted from `last_name` in the view
to `test_id` in the query:

    -- Error: Can't override the privacy unit column set in view.
    SELECT WITH AGGREGATION_THRESHOLD
      OPTIONS(privacy_unit_column=test_id)
      test_id,
      COUNT(DISTINCT last_name) AS student_count,
      AVG(test_score) AS avg_test_score
    FROM mydataset.ExamView
    GROUP BY test_id;

## Differential privacy clause

```
WITH DIFFERENTIAL_PRIVACY OPTIONS( privacy_parameters )

privacy_parameters:
  epsilon = expression,
  delta = expression,
  [ max_groups_contributed = expression ],
  privacy_unit_column = column_name
```

**Description**

This clause lets you transform the results of a query with
[differentially private aggregations](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate-dp-functions). To learn more about
differential privacy, see [Differential privacy](https://docs.cloud.google.com/bigquery/docs/differential-privacy).

You can use the following syntax to build a differential privacy clause:

- [`epsilon`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#dp_epsilon): Controls the amount of noise added to the results. A higher epsilon means less noise. `expression` must be a literal and return a `FLOAT64`.
- [`delta`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#dp_delta): The probability that any row in the result fails to be epsilon-differentially private. `expression` must be a literal and return a `FLOAT64`.
- [`max_groups_contributed`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#dp_max_groups): A positive integer identifying the limit on the number of groups that an entity is allowed to contribute to. This number is also used to scale the noise for each group. `expression` must be a literal and return an `INT64`.
- [`privacy_unit_column`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#dp_privacy_unit_id): The column that represents the privacy unit column. Replace `column_name` with the path expression for the column. The first identifier in the path can start with either a table name or a column name that's visible in the `FROM` clause.

If you want to use this syntax, add it after the `SELECT` keyword with one or
more differentially private aggregate functions in the `SELECT` list.
To learn more about the privacy parameters in this syntax,
see [Privacy parameters](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#dp_privacy_parameters).

### Privacy parameters

Privacy parameters control how the results of a query are transformed.
Appropriate values for these settings can depend on many things such
as the characteristics of your data, the exposure level, and the
privacy level.

In this section, you can learn more about how you can use
privacy parameters to control how the results are transformed.

#### `epsilon`

Noise is added primarily based on the specified `epsilon`
differential privacy parameter. The higher the epsilon the less noise is added.
More noise corresponding to smaller epsilons equals more privacy protection.

Noise can be eliminated by setting `epsilon` to `1e20`, which can be
useful during initial data exploration and experimentation with
differential privacy. Extremely large `epsilon` values, such as `1e308`,
cause query failure.

GoogleSQL splits `epsilon` between the differentially private
aggregates in the query. In addition to the explicit
differentially private aggregate functions, the differential privacy process
also injects an implicit differentially private aggregate into the plan for
removing small groups that computes a noisy entity count per group. If you have
`n` explicit differentially private aggregate functions in your query, then each
aggregate individually gets `epsilon/(n+1)` for its computation. If used with
`max_groups_contributed`, the effective `epsilon` per function per groups is
further split by `max_groups_contributed`. Additionally, if implicit clamping is
used for an aggregate differentially private function, then half of the
function's epsilon is applied towards computing implicit bounds, and half of the
function's epsilon is applied towards the differentially private aggregation
itself.

#### `delta`

The `delta` differential privacy parameter represents the probability that any
row fails to be `epsilon`-differentially private in the result of a
differentially private query.

#### `max_groups_contributed`

The `max_groups_contributed` differential privacy parameter is a
positive integer that, if specified, scales the noise and limits the number of
groups that each entity can contribute to.

`max_groups_contributed` is set by default, even if you don't specify it.
The default value is `1`. If `max_groups_contributed` is set to
`NULL`, then `max_groups_contributed` is unspecified and there is no limit to the number
of groups that each entity can contribute to.

If `max_groups_contributed` is unspecified, the language can't guarantee that
the results will be differentially private. We recommend that you specify
`max_groups_contributed`. If you don't specify `max_groups_contributed`, the
results might still be differentially private if certain preconditions are met.
For example, if you know that the privacy unit column in a table or view is
unique in the `FROM` clause, the entity can't contribute to more than one group
and therefore the results will be the same regardless of whether
`max_groups_contributed` is set.

#### `privacy_unit_column`

To learn about the privacy unit and how to define a privacy unit column, see
[Define a privacy unit column](https://docs.cloud.google.com/bigquery/docs/differential-privacy#dp_define_privacy_unit_id).

### Differential privacy examples

This section contains examples that illustrate how to work with
differential privacy in GoogleSQL.

#### Tables for examples

The examples in this section reference the following tables:

    CREATE OR REPLACE TABLE professors AS (
      SELECT 101 AS id, "pencil" AS item, 24 AS quantity UNION ALL
      SELECT 123, "pen", 16 UNION ALL
      SELECT 123, "pencil", 10 UNION ALL
      SELECT 123, "pencil", 38 UNION ALL
      SELECT 101, "pen", 19 UNION ALL
      SELECT 101, "pen", 23 UNION ALL
      SELECT 130, "scissors", 8 UNION ALL
      SELECT 150, "pencil", 72);

    CREATE OR REPLACE TABLE students AS (
      SELECT 1 AS id, "pencil" AS item, 5 AS quantity UNION ALL
      SELECT 1, "pen", 2 UNION ALL
      SELECT 2, "pen", 1 UNION ALL
      SELECT 3, "pen", 4);

#### Add noise

You can add noise to a differentially private query. Smaller groups might not be
included. Smaller epsilons and more noise will provide greater
privacy protection.

    -- This gets the average number of items requested per professor and adds
    -- noise to the results
    SELECT
      WITH DIFFERENTIAL_PRIVACY
        OPTIONS(epsilon=10, delta=.01, max_groups_contributed=2, privacy_unit_column=id)
        item,
        AVG(quantity, contribution_bounds_per_group => (0,100)) AS average_quantity
    FROM professors
    GROUP BY item;

    -- These results will change each time you run the query.
    -- The scissors group was removed this time, but might not be
    -- removed the next time.
    /*---+---+
     | item     | average_quantity |
     +---+---+
     | pencil   | 38.5038356810269 |
     | pen      | 13.4725028762032 |
     +---+---*/

#### Remove noise

Removing noise removes privacy protection. Only remove noise for
testing queries on non-private data. When `epsilon` is high, noise is removed
from the results.

    -- This gets the average number of items requested per professor and removes
    -- noise from the results
    SELECT
      WITH DIFFERENTIAL_PRIVACY
        OPTIONS(epsilon=1e20, delta=.01, max_groups_contributed=2, privacy_unit_column=id)
        item,
        AVG(quantity, contribution_bounds_per_group => (0,100)) AS average_quantity
    FROM professors
    GROUP BY item;

    /*---+---+
     | item     | average_quantity |
     +---+---+
     | pencil   | 40               |
     | pen      | 18.5             |
     | scissors | 8                |
     +---+---*/

In this example, `max_groups_contributed` is set to `2` because each professor
contributes to at most two groups (that is, each has at most two distinct `item`
values). A value of `2` or greater is needed to avoid dropping contributions
from any professor. Alternatively, `max_groups_contributed` could be set to
`NULL` to not limit contributions.

#### Limit the groups in which a privacy unit ID can exist

A privacy unit column can exist within multiple groups. For example, in the
`professors` table, the privacy unit column `123` exists in the `pencil` and
`pen` group. You can set `max_groups_contributed` to different values to limit how many
groups each privacy unit column will be included in.

    SELECT
      WITH DIFFERENTIAL_PRIVACY
        OPTIONS(epsilon=1e20, delta=.01, privacy_unit_column=id)
        item,
        AVG(quantity, contribution_bounds_per_group => (0,100)) AS average_quantity
    FROM professors
    GROUP BY item;

    -- The privacy unit column 123 was only included in the pen group in this example.
    -- Noise was removed from this query for demonstration purposes only.
    /*---+---+
     | item     | average_quantity |
     +---+---+
     | pencil   | 40               |
     | pen      | 18.5             |
     | scissors | 8                |
     +---+---*/

## Using aliases

An alias is a temporary name given to a table, column, or expression present in
a query. You can introduce explicit aliases in the `SELECT` list or `FROM`
clause, or GoogleSQL infers an implicit alias for some expressions.
Expressions with neither an explicit nor implicit alias are anonymous and the
query can't reference them by name.

### Explicit aliases

You can introduce explicit aliases in either the `FROM` clause or the `SELECT`
list.

In a `FROM` clause, you can introduce explicit aliases for any item, including
tables, arrays, subqueries, and `UNNEST` clauses, using `[AS] alias`. The `AS`
keyword is optional.

Example:

    SELECT s.FirstName, s2.SongName
    FROM Singers AS s, (SELECT * FROM Songs) AS s2;

You can introduce explicit aliases for any expression in the `SELECT` list using
`[AS] alias`. The `AS` keyword is optional.

Example:

    SELECT s.FirstName AS name, LOWER(s.FirstName) AS lname
    FROM Singers s;

### Implicit aliases

In the `SELECT` list, if there is an expression that doesn't have an explicit
alias, GoogleSQL assigns an implicit alias according to the following
rules. There can be multiple columns with the same alias in the `SELECT` list.

- For identifiers, the alias is the identifier. For example, `SELECT abc` implies `AS abc`.
- For path expressions, the alias is the last identifier in the path. For example, `SELECT abc.def.ghi` implies `AS ghi`.
- For field access using the "dot" member field access operator, the alias is the field name. For example, `SELECT (struct_function()).fname` implies `AS
  fname`.

In all other cases, there is no implicit alias, so the column is anonymous and
can't be referenced by name. The data from that column will still be returned
and the displayed query results may have a generated label for that column, but
the label can't be used like an alias.

In a `FROM` clause, `from_item`s aren't required to have an alias. The
following rules apply:

- If there is an expression that doesn't have an explicit alias, GoogleSQL assigns an implicit alias in these cases:
  - For identifiers, the alias is the identifier. For example, `FROM abc` implies `AS abc`.
  - For path expressions, the alias is the last identifier in the path. For example, `FROM abc.def.ghi` implies `AS ghi`
  - The column produced using `WITH OFFSET` has the implicit alias `offset`.
- Table subqueries don't have implicit aliases.
- `FROM UNNEST(x)` doesn't have an implicit alias.

### Alias visibility

After you introduce an explicit alias in a query, there are restrictions on
where else in the query you can reference that alias. These restrictions on
alias visibility are the result of GoogleSQL name scoping rules.

#### Visibility in the `FROM` clause

GoogleSQL processes aliases in a `FROM` clause from left to right,
and aliases are visible only to subsequent path expressions in a `FROM`
clause.

Example:

Assume the `Singers` table had a `Concerts` column of `ARRAY` type.

    SELECT FirstName
    FROM Singers AS s, s.Concerts;

Invalid:

    SELECT FirstName
    FROM s.Concerts, Singers AS s;  // INVALID.

`FROM` clause aliases are **not** visible to subqueries in the same `FROM`
clause. Subqueries in a `FROM` clause can't contain correlated references to
other tables in the same `FROM` clause.

Invalid:

    SELECT FirstName
    FROM Singers AS s, (SELECT (2020 - ReleaseDate) FROM s)  // INVALID.

You can use any column name from a table in the `FROM` as an alias anywhere in
the query, with or without qualification with the table name.

Example:

    SELECT FirstName, s.ReleaseDate
    FROM Singers s WHERE ReleaseDate = 1975;

If the `FROM` clause contains an explicit alias, you must use the explicit alias
instead of the implicit alias for the remainder of the query (see
[Implicit Aliases](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#implicit_aliases)). A table alias is useful for brevity or
to eliminate ambiguity in cases such as self-joins, where the same table is
scanned multiple times during query processing.

Example:

    SELECT * FROM Singers as s, Songs as s2
    ORDER BY s.LastName

Invalid --- `ORDER BY` doesn't use the table alias:

    SELECT * FROM Singers as s, Songs as s2
    ORDER BY Singers.LastName;  // INVALID.

#### Visibility in the `SELECT` list

Aliases in the `SELECT` list are visible only to the following clauses:

- `GROUP BY` clause
- `ORDER BY` clause
- `HAVING` clause

Example:

    SELECT LastName AS last, SingerID
    FROM Singers
    ORDER BY last;

#### Visibility in the `GROUP BY`, `ORDER BY`, and `HAVING` clauses

These three clauses, `GROUP BY`, `ORDER BY`, and `HAVING`, can refer to only the
following values:

- Tables in the `FROM` clause and any of their columns.
- Aliases from the `SELECT` list.

`GROUP BY` and `ORDER BY` can also refer to a third group:

- Integer literals, which refer to items in the `SELECT` list. The integer `1` refers to the first item in the `SELECT` list, `2` refers to the second item, etc.

Example:

    SELECT SingerID AS sid, COUNT(Songid) AS s2id
    FROM Songs
    GROUP BY 1
    ORDER BY 2 DESC;

The previous query is equivalent to:

    SELECT SingerID AS sid, COUNT(Songid) AS s2id
    FROM Songs
    GROUP BY sid
    ORDER BY s2id DESC;

### Duplicate aliases

A `SELECT` list or subquery containing multiple explicit or implicit aliases
of the same name is allowed, as long as the alias name isn't referenced
elsewhere in the query, since the reference would be
[ambiguous](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#ambiguous_aliases).

When a top-level `SELECT` list contains duplicate column names and no
destination table is specified, all duplicate columns, except for the first one,
are automatically renamed to make them unique. The renamed columns appear in the
query result.

Example:

    SELECT 1 AS a, 2 AS a;

    /*---+---+
     | a | a_1 |
     +---+---+
     | 1 | 2   |
     +---+---*/

Duplicate column names in a table or view definition aren't supported. These
statements with queries that contain duplicate column names will fail:

    -- This query fails.
    CREATE TABLE my_dataset.my_table AS (SELECT 1 AS a, 2 AS a);

    -- This query fails.
    CREATE VIEW my_dataset.my_view AS (SELECT 1 AS a, 2 AS a);

### Ambiguous aliases

GoogleSQL provides an error if accessing a name is ambiguous, meaning
it can resolve to more than one unique object in the query or in a table schema,
including the schema of a destination table.

The following query contains column names that conflict between tables, since
both `Singers` and `Songs` have a column named `SingerID`:

    SELECT SingerID
    FROM Singers, Songs;

The following query contains aliases that are ambiguous in the `GROUP BY` clause
because they are duplicated in the `SELECT` list:

    SELECT FirstName AS name, LastName AS name,
    FROM Singers
    GROUP BY name;

The following query contains aliases that are ambiguous in the `SELECT` list and
`FROM` clause because they share a column and field with same name.

- Assume the `Person` table has three columns: `FirstName`, `LastName`, and `PrimaryContact`.
- Assume the `PrimaryContact` column represents a struct with these fields: `FirstName` and `LastName`.

The alias `P` is ambiguous and will produce an error because `P.FirstName` in
the `GROUP BY` clause could refer to either `Person.FirstName` or
`Person.PrimaryContact.FirstName`.

    SELECT FirstName, LastName, PrimaryContact AS P
    FROM Person AS P
    GROUP BY P.FirstName;

A name is *not* ambiguous in `GROUP BY`, `ORDER BY` or `HAVING` if it's both
a column name and a `SELECT` list alias, as long as the name resolves to the
same underlying object. In the following example, the alias `BirthYear` isn't
ambiguous because it resolves to the same underlying column,
`Singers.BirthYear`.

    SELECT LastName, BirthYear AS BirthYear
    FROM Singers
    GROUP BY BirthYear;

### Range variables

In GoogleSQL, a range variable is a table expression alias in the
`FROM` clause. Sometimes a range variable is known as a `table alias`. A
range variable lets you reference rows being scanned from a table expression.
A table expression represents an item in the `FROM` clause that returns a table.
Common items that this expression can represent include
tables,
[value tables](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#value_tables),
[subqueries](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/subqueries),
[table-valued functions (TVFs)](https://cloud.google.com/bigquery/docs/table-functions),
[joins](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#join_types), and [parenthesized joins](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#join_types).

In general, a range variable provides a reference to the rows of a table
expression. A range variable can be used to qualify a column reference and
unambiguously identify the related table, for example `range_variable.column_1`.

When referencing a range variable on its own without a specified column suffix,
the result of a table expression is the row type of the related table.
Value tables have explicit row types, so for range variables related
to value tables, the result type is the value table's row type. Other tables
don't have explicit row types, and for those tables, the range variable
type is a dynamically defined struct that includes all of the
columns in the table.

**Examples**

In these examples, the `WITH` clause is used to emulate a temporary table
called `Grid`. This table has columns `x` and `y`. A range variable called
`Coordinate` refers to the current row as the table is scanned. `Coordinate`
can be used to access the entire row or columns in the row.

The following example selects column `x` from range variable `Coordinate`,
which in effect selects column `x` from table `Grid`.

    WITH Grid AS (SELECT 1 x, 2 y)
    SELECT Coordinate.x FROM Grid AS Coordinate;

    /*---+
     | x |
     +---+
     | 1 |
     +---*/

The following example selects all columns from range variable `Coordinate`,
which in effect selects all columns from table `Grid`.

    WITH Grid AS (SELECT 1 x, 2 y)
    SELECT Coordinate.* FROM Grid AS Coordinate;

    /*---+---+
     | x | y |
     +---+---+
     | 1 | 2 |
     +---+---*/

The following example selects the range variable `Coordinate`, which is a
reference to rows in table `Grid`. Since `Grid` isn't a value table,
the result type of `Coordinate` is a struct that contains all the columns
from `Grid`.

    WITH Grid AS (SELECT 1 x, 2 y)
    SELECT Coordinate FROM Grid AS Coordinate;

    /*---+
     | Coordinate   |
     +---+
     | {x: 1, y: 2} |
     +---*/

## Value tables

In addition to standard SQL *tables* , GoogleSQL supports *value tables* .
In a value table, rather than having rows made up of a list of columns, each row
is a single value of type `STRUCT`, and there are no column names.

In the following example, a value table for a `STRUCT` is produced with the
`SELECT AS VALUE` statement:

    SELECT * FROM (SELECT AS VALUE STRUCT(123 AS a, FALSE AS b))

    /*---+---+
     | a   | b     |
     +---+---+
     | 123 | FALSE |
     +---+---*/

### Return query results as a value table

You can use GoogleSQL to return query results as a value table. This
is useful when you want to store a query result with a
`STRUCT` type as a
table. To return a query result as a value table, use one of the following
statements:

- [`SELECT AS STRUCT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#select_list)
- [`SELECT AS VALUE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#select_list)

Value tables can also occur as the output of the [`UNNEST`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#unnest_operator)
operator or a [subquery](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/subqueries). The [`WITH` clause](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#with_clause)
introduces a value table if the subquery used produces a value table.

In contexts where a query with exactly one column is expected, a value table
query can be used instead. For example, scalar and
array [subqueries](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/subqueries) normally require a single-column query,
but in GoogleSQL, they also allow using a value table query.

### Create a table with a value table

Value tables aren't supported as top-level queries in the
`CREATE TABLE` statement, but they can be included in subqueries and
`UNNEST` operations. For example, you can create a table from a
value table with this query:

    CREATE TABLE Reviews AS
    SELECT * FROM (SELECT AS VALUE STRUCT(5 AS star_rating, FALSE AS up_down_rating))

| Column Name | Data Type |
|---|---|
| star_rating | `INT64` |
| up_down_rating | `BOOL` |

### Use a set operation on a value table

You can't combine tables and value tables in a `SET` operation.

## Table function calls

To call a TVF, use the function call in place of the table name in a `FROM`
clause.

## Appendix A: examples with sample data

These examples include statements which perform queries on the
[`Roster`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#roster_table) and [`TeamMascot`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#teammascot_table),
and [`PlayerStats`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#playerstats_table) tables.

### Sample tables

The following tables are used to illustrate the behavior of different
query clauses in this reference.

#### Roster table

The `Roster` table includes a list of player names (`LastName`) and the
unique ID assigned to their school (`SchoolID`). It looks like this:

    /*---+
     | LastName   | SchoolID |
     +---+
     | Adams      | 50       |
     | Buchanan   | 52       |
     | Coolidge   | 52       |
     | Davis      | 51       |
     | Eisenhower | 77       |
     +---*/

You can use this `WITH` clause to emulate a temporary table name for the
examples in this reference:

    WITH Roster AS
     (SELECT 'Adams' as LastName, 50 as SchoolID UNION ALL
      SELECT 'Buchanan', 52 UNION ALL
      SELECT 'Coolidge', 52 UNION ALL
      SELECT 'Davis', 51 UNION ALL
      SELECT 'Eisenhower', 77)
    SELECT * FROM Roster

#### PlayerStats table

The `PlayerStats` table includes a list of player names (`LastName`) and the
unique ID assigned to the opponent they played in a given game (`OpponentID`)
and the number of points scored by the athlete in that game (`PointsScored`).

    /*---+
     | LastName   | OpponentID | PointsScored |
     +---+
     | Adams      | 51         | 3            |
     | Buchanan   | 77         | 0            |
     | Coolidge   | 77         | 1            |
     | Adams      | 52         | 4            |
     | Buchanan   | 50         | 13           |
     +---*/

You can use this `WITH` clause to emulate a temporary table name for the
examples in this reference:

    WITH PlayerStats AS
     (SELECT 'Adams' as LastName, 51 as OpponentID, 3 as PointsScored UNION ALL
      SELECT 'Buchanan', 77, 0 UNION ALL
      SELECT 'Coolidge', 77, 1 UNION ALL
      SELECT 'Adams', 52, 4 UNION ALL
      SELECT 'Buchanan', 50, 13)
    SELECT * FROM PlayerStats

#### TeamMascot table

The `TeamMascot` table includes a list of unique school IDs (`SchoolID`) and the
mascot for that school (`Mascot`).

    /*---+
     | SchoolID | Mascot   |
     +---+
     | 50       | Jaguars  |
     | 51       | Knights  |
     | 52       | Lakers   |
     | 53       | Mustangs |
     +---*/

You can use this `WITH` clause to emulate a temporary table name for the
examples in this reference:

    WITH TeamMascot AS
     (SELECT 50 as SchoolID, 'Jaguars' as Mascot UNION ALL
      SELECT 51, 'Knights' UNION ALL
      SELECT 52, 'Lakers' UNION ALL
      SELECT 53, 'Mustangs')
    SELECT * FROM TeamMascot

### `GROUP BY` clause

Example:

    SELECT LastName, SUM(PointsScored)
    FROM PlayerStats
    GROUP BY LastName;

| LastName | SUM |
|---|---|
| Adams | 7 |
| Buchanan | 13 |
| Coolidge | 1 |

### `UNION`

The `UNION` operator combines the result sets of two or more `SELECT` statements
by pairing columns from the result set of each `SELECT` statement and vertically
concatenating them.

Example:

    SELECT Mascot AS X, SchoolID AS Y
    FROM TeamMascot
    UNION ALL
    SELECT LastName, PointsScored
    FROM PlayerStats;

Results:

| X | Y |
|---|---|
| Jaguars | 50 |
| Knights | 51 |
| Lakers | 52 |
| Mustangs | 53 |
| Adams | 3 |
| Buchanan | 0 |
| Coolidge | 1 |
| Adams | 4 |
| Buchanan | 13 |

### `INTERSECT`

This query returns the last names that are present in both Roster and
PlayerStats.

    SELECT LastName
    FROM Roster
    INTERSECT DISTINCT
    SELECT LastName
    FROM PlayerStats;

Results:

| LastName |
|---|
| Adams |
| Coolidge |
| Buchanan |

### `EXCEPT`

The query below returns last names in Roster that are **not** present in
PlayerStats.

    SELECT LastName
    FROM Roster
    EXCEPT DISTINCT
    SELECT LastName
    FROM PlayerStats;

Results:

| LastName |
|---|
| Eisenhower |
| Davis |

Reversing the order of the `SELECT` statements will return last names in
PlayerStats that are **not** present in Roster:

    SELECT LastName
    FROM PlayerStats
    EXCEPT DISTINCT
    SELECT LastName
    FROM Roster;

Results:

    (empty)