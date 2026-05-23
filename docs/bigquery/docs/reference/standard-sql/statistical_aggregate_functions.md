GoogleSQL for BigQuery supports statistical aggregate functions.
To learn about the syntax for aggregate function calls, see
[Aggregate function calls](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate-function-calls).

## Function list

| Name | Summary |
|---|---|
| [`CORR`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#corr) | Computes the Pearson coefficient of correlation of a set of number pairs. |
| [`COVAR_POP`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#covar_pop) | Computes the population covariance of a set of number pairs. |
| [`COVAR_SAMP`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#covar_samp) | Computes the sample covariance of a set of number pairs. |
| [`STDDEV`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#stddev) | An alias of the `STDDEV_SAMP` function. |
| [`STDDEV_POP`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#stddev_pop) | Computes the population (biased) standard deviation of the values. |
| [`STDDEV_SAMP`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#stddev_samp) | Computes the sample (unbiased) standard deviation of the values. |
| [`VAR_POP`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#var_pop) | Computes the population (biased) variance of the values. |
| [`VAR_SAMP`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#var_samp) | Computes the sample (unbiased) variance of the values. |
| [`VARIANCE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#variance) | An alias of `VAR_SAMP`. |

## `CORR`

    CORR(
      X1, X2
    )
    [ OVER over_clause ]

    over_clause:
      { named_window | ( [ window_specification ] ) }

    window_specification:
      [ named_window ]
      [ PARTITION BY partition_expression [, ...] ]
      [ ORDER BY expression [ { ASC | DESC }  ] [, ...] ]
      [ window_frame_clause ]

**Description**

Returns the [Pearson coefficient](https://en.wikipedia.org/wiki/Pearson_product-moment_correlation_coefficient)
of correlation of a set of number pairs. For each number pair, the first number
is the dependent variable and the second number is the independent variable.
The return result is between `-1` and `1`. A result of `0` indicates no
correlation.

All numeric types are supported. If the
input is `NUMERIC` or `BIGNUMERIC` then the internal aggregation is
stable with the final output converted to a `FLOAT64`.
Otherwise the input is converted to a `FLOAT64`
before aggregation, resulting in a potentially unstable result.

This function ignores any input pairs that contain one or more `NULL` values. If
there are fewer than two input pairs without `NULL` values, this function
returns `NULL`.

`NaN` is produced if:

- Any input value is `NaN`
- Any input value is positive infinity or negative infinity.
- The variance of `X1` or `X2` is `0`.
- The covariance of `X1` and `X2` is `0`.

To learn more about the optional aggregate clauses that you can pass
into this function, see
[Aggregate function calls](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate-function-calls).

To learn more about the `OVER` clause and how to use it, see
[Window function calls](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/window-function-calls).

**Return Data Type**

`FLOAT64`

**Examples**

    SELECT CORR(y, x) AS results
    FROM
      UNNEST(
        [
          STRUCT(1.0 AS y, 5.0 AS x),
          (3.0, 9.0),
          (4.0, 7.0)]);

    /*---+
     | results            |
     +---+
     | 0.6546536707079772 |
     +---*/

    SELECT CORR(y, x) AS results
    FROM
      UNNEST(
        [
          STRUCT(1.0 AS y, 5.0 AS x),
          (3.0, 9.0),
          (4.0, NULL)]);

    /*---+
     | results |
     +---+
     | 1       |
     +---*/

    SELECT CORR(y, x) AS results
    FROM UNNEST([STRUCT(1.0 AS y, NULL AS x),(9.0, 3.0)])

    /*---+
     | results |
     +---+
     | NULL    |
     +---*/

    SELECT CORR(y, x) AS results
    FROM UNNEST([STRUCT(1.0 AS y, NULL AS x),(9.0, NULL)])

    /*---+
     | results |
     +---+
     | NULL    |
     +---*/

    SELECT CORR(y, x) AS results
    FROM
      UNNEST(
        [
          STRUCT(1.0 AS y, 5.0 AS x),
          (3.0, 9.0),
          (4.0, 7.0),
          (5.0, 1.0),
          (7.0, CAST('Infinity' as FLOAT64))])

    /*---+
     | results |
     +---+
     | NaN     |
     +---*/

    SELECT CORR(x, y) AS results
    FROM
      (
        SELECT 0 AS x, 0 AS y
        UNION ALL
        SELECT 0 AS x, 0 AS y
      )

    /*---+
     | results |
     +---+
     | NaN     |
     +---*/

## `COVAR_POP`

    COVAR_POP(
      X1, X2
    )
    [ OVER over_clause ]

    over_clause:
      { named_window | ( [ window_specification ] ) }

    window_specification:
      [ named_window ]
      [ PARTITION BY partition_expression [, ...] ]
      [ ORDER BY expression [ { ASC | DESC }  ] [, ...] ]
      [ window_frame_clause ]

**Description**

Returns the population [covariance](https://en.wikipedia.org/wiki/Covariance) of
a set of number pairs. The first number is the dependent variable; the second
number is the independent variable. The return result is between `-Inf` and
`+Inf`.

All numeric types are supported. If the
input is `NUMERIC` or `BIGNUMERIC` then the internal aggregation is
stable with the final output converted to a `FLOAT64`.
Otherwise the input is converted to a `FLOAT64`
before aggregation, resulting in a potentially unstable result.

This function ignores any input pairs that contain one or more `NULL` values. If
there is no input pair without `NULL` values, this function returns `NULL`.
If there is exactly one input pair without `NULL` values, this function returns
`0`.

`NaN` is produced if:

- Any input value is `NaN`
- Any input value is positive infinity or negative infinity.

To learn more about the optional aggregate clauses that you can pass
into this function, see
[Aggregate function calls](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate-function-calls).

This function can be used with the
[`AGGREGATION_THRESHOLD` clause](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#agg_threshold_clause).

To learn more about the `OVER` clause and how to use it, see
[Window function calls](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/window-function-calls).

**Return Data Type**

`FLOAT64`

**Examples**

    SELECT COVAR_POP(y, x) AS results
    FROM
      UNNEST(
        [
          STRUCT(1.0 AS y, 1.0 AS x),
          (2.0, 6.0),
          (9.0, 3.0),
          (2.0, 6.0),
          (9.0, 3.0)])

    /*---+
     | results             |
     +---+
     | -1.6800000000000002 |
     +---*/

    SELECT COVAR_POP(y, x) AS results
    FROM UNNEST([STRUCT(1.0 AS y, NULL AS x),(9.0, 3.0)])

    /*---+
     | results |
     +---+
     | 0       |
     +---*/

    SELECT COVAR_POP(y, x) AS results
    FROM UNNEST([STRUCT(1.0 AS y, NULL AS x),(9.0, NULL)])

    /*---+
     | results |
     +---+
     | NULL    |
     +---*/

    SELECT COVAR_POP(y, x) AS results
    FROM
      UNNEST(
        [
          STRUCT(1.0 AS y, 1.0 AS x),
          (2.0, 6.0),
          (9.0, 3.0),
          (2.0, 6.0),
          (NULL, 3.0)])

    /*---+
     | results |
     +---+
     | -1      |
     +---*/

    SELECT COVAR_POP(y, x) AS results
    FROM
      UNNEST(
        [
          STRUCT(1.0 AS y, 1.0 AS x),
          (2.0, 6.0),
          (9.0, 3.0),
          (2.0, 6.0),
          (CAST('Infinity' as FLOAT64), 3.0)])

    /*---+
     | results |
     +---+
     | NaN     |
     +---*/

## `COVAR_SAMP`

    COVAR_SAMP(
      X1, X2
    )
    [ OVER over_clause ]

    over_clause:
      { named_window | ( [ window_specification ] ) }

    window_specification:
      [ named_window ]
      [ PARTITION BY partition_expression [, ...] ]
      [ ORDER BY expression [ { ASC | DESC }  ] [, ...] ]
      [ window_frame_clause ]

**Description**

Returns the sample [covariance](https://en.wikipedia.org/wiki/Covariance) of a
set of number pairs. The first number is the dependent variable; the second
number is the independent variable. The return result is between `-Inf` and
`+Inf`.

All numeric types are supported. If the
input is `NUMERIC` or `BIGNUMERIC` then the internal aggregation is
stable with the final output converted to a `FLOAT64`.
Otherwise the input is converted to a `FLOAT64`
before aggregation, resulting in a potentially unstable result.

This function ignores any input pairs that contain one or more `NULL` values. If
there are fewer than two input pairs without `NULL` values, this function
returns `NULL`.

`NaN` is produced if:

- Any input value is `NaN`
- Any input value is positive infinity or negative infinity.

To learn more about the optional aggregate clauses that you can pass
into this function, see
[Aggregate function calls](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate-function-calls).

This function can be used with the
[`AGGREGATION_THRESHOLD` clause](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#agg_threshold_clause).

To learn more about the `OVER` clause and how to use it, see
[Window function calls](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/window-function-calls).

**Return Data Type**

`FLOAT64`

**Examples**

    SELECT COVAR_SAMP(y, x) AS results
    FROM
      UNNEST(
        [
          STRUCT(1.0 AS y, 1.0 AS x),
          (2.0, 6.0),
          (9.0, 3.0),
          (2.0, 6.0),
          (9.0, 3.0)])

    /*---+
     | results |
     +---+
     | -2.1    |
     +---*/

    SELECT COVAR_SAMP(y, x) AS results
    FROM
      UNNEST(
        [
          STRUCT(1.0 AS y, 1.0 AS x),
          (2.0, 6.0),
          (9.0, 3.0),
          (2.0, 6.0),
          (NULL, 3.0)])

    /*---+
     | results              |
     +---+
     | --1.3333333333333333 |
     +---*/

    SELECT COVAR_SAMP(y, x) AS results
    FROM UNNEST([STRUCT(1.0 AS y, NULL AS x),(9.0, 3.0)])

    /*---+
     | results |
     +---+
     | NULL    |
     +---*/

    SELECT COVAR_SAMP(y, x) AS results
    FROM UNNEST([STRUCT(1.0 AS y, NULL AS x),(9.0, NULL)])

    /*---+
     | results |
     +---+
     | NULL    |
     +---*/

    SELECT COVAR_SAMP(y, x) AS results
    FROM
      UNNEST(
        [
          STRUCT(1.0 AS y, 1.0 AS x),
          (2.0, 6.0),
          (9.0, 3.0),
          (2.0, 6.0),
          (CAST('Infinity' as FLOAT64), 3.0)])

    /*---+
     | results |
     +---+
     | NaN     |
     +---*/

## `STDDEV`

    STDDEV(
      [ DISTINCT ]
      expression
    )
    [ OVER over_clause ]

    over_clause:
      { named_window | ( [ window_specification ] ) }

    window_specification:
      [ named_window ]
      [ PARTITION BY partition_expression [, ...] ]
      [ ORDER BY expression [ { ASC | DESC }  ] [, ...] ]
      [ window_frame_clause ]

**Description**

An alias of [STDDEV_SAMP](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#stddev_samp).

## `STDDEV_POP`

    STDDEV_POP(
      [ DISTINCT ]
      expression
    )
    [ OVER over_clause ]

    over_clause:
      { named_window | ( [ window_specification ] ) }

    window_specification:
      [ named_window ]
      [ PARTITION BY partition_expression [, ...] ]
      [ ORDER BY expression [ { ASC | DESC }  ] [, ...] ]
      [ window_frame_clause ]

**Description**

Returns the population (biased) standard deviation of the values. The return
result is between `0` and `+Inf`.

All numeric types are supported. If the
input is `NUMERIC` or `BIGNUMERIC` then the internal aggregation is
stable with the final output converted to a `FLOAT64`.
Otherwise the input is converted to a `FLOAT64`
before aggregation, resulting in a potentially unstable result.

This function ignores any `NULL` inputs. If all inputs are ignored, this
function returns `NULL`. If this function receives a single non-`NULL` input,
it returns `0`.

`NaN` is produced if:

- Any input value is `NaN`
- Any input value is positive infinity or negative infinity.

To learn more about the optional aggregate clauses that you can pass
into this function, see
[Aggregate function calls](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate-function-calls).

This function can be used with the
[`AGGREGATION_THRESHOLD` clause](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#agg_threshold_clause).

If this function is used with the `OVER` clause, it's part of a
window function call. In a window function call,
aggregate function clauses can't be used.
To learn more about the `OVER` clause and how to use it, see
[Window function calls](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/window-function-calls).

**Return Data Type**

`FLOAT64`

**Examples**

    SELECT STDDEV_POP(x) AS results FROM UNNEST([10, 14, 18]) AS x

    /*---+
     | results           |
     +---+
     | 3.265986323710904 |
     +---*/

    SELECT STDDEV_POP(x) AS results FROM UNNEST([10, 14, NULL]) AS x

    /*---+
     | results |
     +---+
     | 2       |
     +---*/

    SELECT STDDEV_POP(x) AS results FROM UNNEST([10, NULL]) AS x

    /*---+
     | results |
     +---+
     | 0       |
     +---*/

    SELECT STDDEV_POP(x) AS results FROM UNNEST([NULL]) AS x

    /*---+
     | results |
     +---+
     | NULL    |
     +---*/

    SELECT STDDEV_POP(x) AS results FROM UNNEST([10, 14, CAST('Infinity' as FLOAT64)]) AS x

    /*---+
     | results |
     +---+
     | NaN     |
     +---*/

## `STDDEV_SAMP`

    STDDEV_SAMP(
      [ DISTINCT ]
      expression
    )
    [ OVER over_clause ]

    over_clause:
      { named_window | ( [ window_specification ] ) }

    window_specification:
      [ named_window ]
      [ PARTITION BY partition_expression [, ...] ]
      [ ORDER BY expression [ { ASC | DESC }  ] [, ...] ]
      [ window_frame_clause ]

**Description**

Returns the sample (unbiased) standard deviation of the values. The return
result is between `0` and `+Inf`.

All numeric types are supported. If the
input is `NUMERIC` or `BIGNUMERIC` then the internal aggregation is
stable with the final output converted to a `FLOAT64`.
Otherwise the input is converted to a `FLOAT64`
before aggregation, resulting in a potentially unstable result.

This function ignores any `NULL` inputs. If there are fewer than two non-`NULL`
inputs, this function returns `NULL`.

`NaN` is produced if:

- Any input value is `NaN`
- Any input value is positive infinity or negative infinity.

To learn more about the optional aggregate clauses that you can pass
into this function, see
[Aggregate function calls](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate-function-calls).

This function can be used with the
[`AGGREGATION_THRESHOLD` clause](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#agg_threshold_clause).

If this function is used with the `OVER` clause, it's part of a
window function call. In a window function call,
aggregate function clauses can't be used.
To learn more about the `OVER` clause and how to use it, see
[Window function calls](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/window-function-calls).

**Return Data Type**

`FLOAT64`

**Examples**

    SELECT STDDEV_SAMP(x) AS results FROM UNNEST([10, 14, 18]) AS x

    /*---+
     | results |
     +---+
     | 4       |
     +---*/

    SELECT STDDEV_SAMP(x) AS results FROM UNNEST([10, 14, NULL]) AS x

    /*---+
     | results            |
     +---+
     | 2.8284271247461903 |
     +---*/

    SELECT STDDEV_SAMP(x) AS results FROM UNNEST([10, NULL]) AS x

    /*---+
     | results |
     +---+
     | NULL    |
     +---*/

    SELECT STDDEV_SAMP(x) AS results FROM UNNEST([NULL]) AS x

    /*---+
     | results |
     +---+
     | NULL    |
     +---*/

    SELECT STDDEV_SAMP(x) AS results FROM UNNEST([10, 14, CAST('Infinity' as FLOAT64)]) AS x

    /*---+
     | results |
     +---+
     | NaN     |
     +---*/

## `VAR_POP`

    VAR_POP(
      [ DISTINCT ]
      expression
    )
    [ OVER over_clause ]

    over_clause:
      { named_window | ( [ window_specification ] ) }

    window_specification:
      [ named_window ]
      [ PARTITION BY partition_expression [, ...] ]
      [ ORDER BY expression [ { ASC | DESC }  ] [, ...] ]
      [ window_frame_clause ]

**Description**

Returns the population (biased) variance of the values. The return result is
between `0` and `+Inf`.

All numeric types are supported. If the
input is `NUMERIC` or `BIGNUMERIC` then the internal aggregation is
stable with the final output converted to a `FLOAT64`.
Otherwise the input is converted to a `FLOAT64`
before aggregation, resulting in a potentially unstable result.

This function ignores any `NULL` inputs. If all inputs are ignored, this
function returns `NULL`. If this function receives a single non-`NULL` input,
it returns `0`.

`NaN` is produced if:

- Any input value is `NaN`
- Any input value is positive infinity or negative infinity.

If this function is used with the `OVER` clause, it's part of a
window function call. In a window function call,
aggregate function clauses can't be used.
To learn more about the `OVER` clause and how to use it, see
[Window function calls](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/window-function-calls).

**Return Data Type**

`FLOAT64`

**Examples**

    SELECT VAR_POP(x) AS results FROM UNNEST([10, 14, 18]) AS x

    /*---+
     | results            |
     +---+
     | 10.666666666666666 |
     +---*/

    SELECT VAR_POP(x) AS results FROM UNNEST([10, 14, NULL]) AS x

    /*---+
     | results |
     +---+
     | 4       |
     +---*/

    SELECT VAR_POP(x) AS results FROM UNNEST([10, NULL]) AS x

    /*---+
     | results |
     +---+
     | 0       |
     +---*/

    SELECT VAR_POP(x) AS results FROM UNNEST([NULL]) AS x

    /*---+
     | results |
     +---+
     | NULL    |
     +---*/

    SELECT VAR_POP(x) AS results FROM UNNEST([10, 14, CAST('Infinity' as FLOAT64)]) AS x

    /*---+
     | results |
     +---+
     | NaN     |
     +---*/

## `VAR_SAMP`

    VAR_SAMP(
      [ DISTINCT ]
      expression
    )
    [ OVER over_clause ]

    over_clause:
      { named_window | ( [ window_specification ] ) }

    window_specification:
      [ named_window ]
      [ PARTITION BY partition_expression [, ...] ]
      [ ORDER BY expression [ { ASC | DESC }  ] [, ...] ]
      [ window_frame_clause ]

**Description**

Returns the sample (unbiased) variance of the values. The return result is
between `0` and `+Inf`.

All numeric types are supported. If the
input is `NUMERIC` or `BIGNUMERIC` then the internal aggregation is
stable with the final output converted to a `FLOAT64`.
Otherwise the input is converted to a `FLOAT64`
before aggregation, resulting in a potentially unstable result.

This function ignores any `NULL` inputs. If there are fewer than two non-`NULL`
inputs, this function returns `NULL`.

`NaN` is produced if:

- Any input value is `NaN`
- Any input value is positive infinity or negative infinity.

To learn more about the optional aggregate clauses that you can pass
into this function, see
[Aggregate function calls](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate-function-calls).

This function can be used with the
[`AGGREGATION_THRESHOLD` clause](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#agg_threshold_clause).

If this function is used with the `OVER` clause, it's part of a
window function call. In a window function call,
aggregate function clauses can't be used.
To learn more about the `OVER` clause and how to use it, see
[Window function calls](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/window-function-calls).

**Return Data Type**

`FLOAT64`

**Examples**

    SELECT VAR_SAMP(x) AS results FROM UNNEST([10, 14, 18]) AS x

    /*---+
     | results |
     +---+
     | 16      |
     +---*/

    SELECT VAR_SAMP(x) AS results FROM UNNEST([10, 14, NULL]) AS x

    /*---+
     | results |
     +---+
     | 8       |
     +---*/

    SELECT VAR_SAMP(x) AS results FROM UNNEST([10, NULL]) AS x

    /*---+
     | results |
     +---+
     | NULL    |
     +---*/

    SELECT VAR_SAMP(x) AS results FROM UNNEST([NULL]) AS x

    /*---+
     | results |
     +---+
     | NULL    |
     +---*/

    SELECT VAR_SAMP(x) AS results FROM UNNEST([10, 14, CAST('Infinity' as FLOAT64)]) AS x

    /*---+
     | results |
     +---+
     | NaN     |
     +---*/

## `VARIANCE`

    VARIANCE(
      [ DISTINCT ]
      expression
    )
    [ OVER over_clause ]

    over_clause:
      { named_window | ( [ window_specification ] ) }

    window_specification:
      [ named_window ]
      [ PARTITION BY partition_expression [, ...] ]
      [ ORDER BY expression [ { ASC | DESC }  ] [, ...] ]
      [ window_frame_clause ]

**Description**

An alias of [VAR_SAMP](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#var_samp).