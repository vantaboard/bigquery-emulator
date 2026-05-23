GoogleSQL for BigQuery supports conditional expressions.
Conditional expressions impose constraints on the evaluation order of their
inputs. In essence, they are evaluated left to right, with short-circuiting, and
only evaluate the output value that was chosen. In contrast, all inputs to
regular functions are evaluated before calling the function. Short-circuiting in
conditional expressions can be exploited for error handling or performance
tuning.

### Expression list

| Name | Summary |
|---|---|
| [`CASE expr`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/conditional_expressions#case_expr) | Compares the given expression to each successive `WHEN` clause and produces the first result where the values are equal. |
| [`CASE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/conditional_expressions#case) | Evaluates the condition of each successive `WHEN` clause and produces the first result where the condition evaluates to `TRUE`. |
| [`COALESCE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/conditional_expressions#coalesce) | Produces the value of the first non-`NULL` expression, if any, otherwise `NULL`. |
| [`IF`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/conditional_expressions#if) | If an expression evaluates to `TRUE`, produces a specified result, otherwise produces the evaluation for an *else result*. |
| [`IFNULL`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/conditional_expressions#ifnull) | If an expression evaluates to `NULL`, produces a specified result, otherwise produces the expression. |
| [`NULLIF`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/conditional_expressions#nullif) | Produces `NULL` if the first expression that matches another evaluates to `TRUE`, otherwise returns the first expression. |

### `CASE expr`

    CASE expr
      WHEN expr_to_match THEN result
      [ ... ]
      [ ELSE else_result ]
      END

**Description**

Compares `expr` to `expr_to_match` of each successive `WHEN` clause and returns
the first result where this comparison evaluates to `TRUE`. The remaining `WHEN`
clauses and `else_result` aren't evaluated.

If the `expr = expr_to_match` comparison evaluates to `FALSE` or `NULL` for all
`WHEN` clauses, returns the evaluation of `else_result` if present; if
`else_result` isn't present, then returns `NULL`.

Consistent with [equality comparisons](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/operators#logical_operators) elsewhere, if both
`expr` and `expr_to_match` are `NULL`, then `expr = expr_to_match` evaluates to
`NULL`, which returns `else_result`. If a CASE statement needs to distinguish a
`NULL` value, then the alternate [CASE](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/conditional_expressions#case) syntax should be used.

`expr` and `expr_to_match` can be any type. They must be implicitly
coercible to a common [supertype](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/conversion_rules#supertypes); equality comparisons are
done on coerced values. There may be multiple `result` types. `result` and
`else_result` expressions must be coercible to a common supertype.

This expression supports specifying [collation](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/collation-concepts).

**Return Data Type**

[Supertype](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/conversion_rules#supertypes) of `result`\[, ...\] and `else_result`.

**Example**

    WITH Numbers AS (
      SELECT 90 as A, 2 as B UNION ALL
      SELECT 50, 8 UNION ALL
      SELECT 60, 6 UNION ALL
      SELECT 50, 10
    )
    SELECT
      A,
      B,
      CASE A
        WHEN 90 THEN 'red'
        WHEN 50 THEN 'blue'
        ELSE 'green'
        END
        AS result
    FROM Numbers

    /*---+
     | A  | B  | result |
     +---+
     | 90 | 2  | red    |
     | 50 | 8  | blue   |
     | 60 | 6  | green  |
     | 50 | 10 | blue   |
     +---*/

### `CASE`

    CASE
      WHEN condition THEN result
      [ ... ]
      [ ELSE else_result ]
      END

**Description**

Evaluates the condition of each successive `WHEN` clause and returns the
first result where the condition evaluates to `TRUE`; any remaining `WHEN`
clauses and `else_result` aren't evaluated.

If all conditions evaluate to `FALSE` or `NULL`, returns evaluation of
`else_result` if present; if `else_result` isn't present, then returns `NULL`.

For additional rules on how values are evaluated, see the
three-valued logic table in [Logical operators](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/operators#logical_operators).

`condition` must be a boolean expression. There may be multiple `result` types.
`result` and `else_result` expressions must be implicitly coercible to a common
[supertype](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/conversion_rules#supertypes).

This expression supports specifying [collation](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/collation-concepts).

**Return Data Type**

[Supertype](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/conversion_rules#supertypes) of `result`\[, ...\] and `else_result`.

**Example**

    WITH Numbers AS (
      SELECT 90 as A, 2 as B UNION ALL
      SELECT 50, 6 UNION ALL
      SELECT 20, 10
    )
    SELECT
      A,
      B,
      CASE
        WHEN A > 60 THEN 'red'
        WHEN B = 6 THEN 'blue'
        ELSE 'green'
        END
        AS result
    FROM Numbers

    /*---+
     | A  | B  | result |
     +---+
     | 90 | 2  | red    |
     | 50 | 6  | blue   |
     | 20 | 10 | green  |
     +---*/

### `COALESCE`

    COALESCE(expr[, ...])

**Description**

Returns the value of the first non-`NULL` expression, if any, otherwise
`NULL`. The remaining expressions aren't evaluated. An input expression can be
any type. There may be multiple input expression types.
All input expressions must be implicitly coercible to a common
[supertype](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/conversion_rules#supertypes).

**Return Data Type**

[Supertype](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/conversion_rules#supertypes) of `expr`\[, ...\].

**Examples**

    SELECT COALESCE('A', 'B', 'C') as result

    /*---+
     | result |
     +---+
     | A      |
     +---*/

    SELECT COALESCE(NULL, 'B', 'C') as result

    /*---+
     | result |
     +---+
     | B      |
     +---*/

### `IF`

    IF(expr, true_result, else_result)

**Description**

If `expr` evaluates to `TRUE`, returns `true_result`, else returns the
evaluation for `else_result`. `else_result` isn't evaluated if `expr` evaluates
to `TRUE`. `true_result` isn't evaluated if `expr` evaluates to `FALSE` or
`NULL`.

`expr` must be a boolean expression. `true_result` and `else_result`
must be coercible to a common [supertype](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/conversion_rules#supertypes).

**Return Data Type**

[Supertype](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/conversion_rules#supertypes) of `true_result` and `else_result`.

**Examples**

    SELECT
      10 AS A,
      20 AS B,
      IF(10 < 20, 'true', 'false') AS result

    /*---+
     | A  | B  | result |
     +---+
     | 10 | 20 | true   |
     +---*/

    SELECT
      30 AS A,
      20 AS B,
      IF(30 < 20, 'true', 'false') AS result

    /*---+
     | A  | B  | result |
     +---+
     | 30 | 20 | false  |
     +---*/

### `IFNULL`

    IFNULL(expr, null_result)

**Description**

If `expr` evaluates to `NULL`, returns `null_result`. Otherwise, returns
`expr`. If `expr` doesn't evaluate to `NULL`, `null_result` isn't evaluated.

`expr` and `null_result` can be any type and must be implicitly coercible to
a common [supertype](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/conversion_rules#supertypes). Synonym for
`COALESCE(expr, null_result)`.

**Return Data Type**

[Supertype](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/conversion_rules#supertypes) of `expr` or `null_result`.

**Examples**

    SELECT IFNULL(NULL, 0) as result

    /*---+
     | result |
     +---+
     | 0      |
     +---*/

    SELECT IFNULL(10, 0) as result

    /*---+
     | result |
     +---+
     | 10     |
     +---*/

### `NULLIF`

    NULLIF(expr, expr_to_match)

**Description**

Returns `NULL` if `expr = expr_to_match` evaluates to `TRUE`, otherwise
returns `expr`.

`expr` and `expr_to_match` must be implicitly coercible to a
common [supertype](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/conversion_rules#supertypes), and must be comparable.

This expression supports specifying [collation](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/collation-concepts).

**Return Data Type**

[Supertype](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/conversion_rules#supertypes) of `expr` and `expr_to_match`.

**Example**

    SELECT NULLIF(0, 0) as result

    /*---+
     | result |
     +---+
     | NULL   |
     +---*/

    SELECT NULLIF(10, 0) as result

    /*---+
     | result |
     +---+
     | 10     |
     +---*/