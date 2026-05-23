# The ML.ROBUST_SCALER function

This document describes the `ML.ROBUST_SCALER` function, which lets you scale a
numerical expression by using statistics that are robust to outliers. The
function performs the scaling by removing the
[median](https://en.wikipedia.org/wiki/Median) and scaling
the data according to the [quantile](https://en.wikipedia.org/wiki/Quantile)
range.

When used in the
[`TRANSFORM` clause](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create#transform),
the median and quantile range calculated during training are automatically
used in prediction.

You can use this function with models that support
[manual feature preprocessing](https://docs.cloud.google.com/bigquery/docs/manual-preprocessing). For more
information, see the following documents:

- [End-to-end user journeys for ML models](https://docs.cloud.google.com/bigquery/docs/e2e-journey)
- [Contribution analysis user journey](https://docs.cloud.google.com/bigquery/docs/contribution-analysis#contribution_analysis_user_journey)

## Syntax

```sql
ML.ROBUST_SCALER(numerical_expression [, quantile_range] [, with_median] [, with_quantile_range]) OVER()
```

### Arguments

`ML.ROBUST_SCALER` takes the following arguments:

- `numerical_expression`: the [numerical](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#numeric_types) expression to scale.
- `quantile_range`: an array of two `INT64` elements that specifies the quantile range. The first element provides the lower boundary of the range. It must be greater than `0`. The second element provides the upper boundary of the range. It must be greater than the first element but less than `100`. The default value is `[25, 75]`.
- `with_median`: a `BOOL` value that specifies whether the data is centered. If `TRUE`, the function centers the data by removing the median before scaling. The default value is `TRUE`.
- `with_quantile_range`: a `BOOL` value that specifies whether the data is scaled to the quantile range. If `TRUE`, the data is scaled. The default value is `TRUE`.

## Output

`ML.ROBUST_SCALER` returns a `FLOAT64` value that represents the scaled
numerical expression.

## Example

The following example centers a set of numerical expressions and then
scales it to the range `[25, 75]`:

```sql
SELECT f, ML.ROBUST_SCALER(f) OVER () AS output
FROM
  UNNEST([NULL, -3, 1, 2, 3, 4, 5]) AS f
ORDER BY f;
```

The output looks similar to the following:

```
+---+---+
|  f   |       output        |
+---+---+
| NULL |                NULL |
|   -3 | -1.6666666666666667 |
|    1 | -0.3333333333333333 |
|    2 |                 0.0 |
|    3 |  0.3333333333333333 |
|    4 |  0.6666666666666666 |
|    5 |                 1.0 |
+---+---+
```

## What's next

- For information about feature preprocessing, see [Feature preprocessing overview](https://docs.cloud.google.com/bigquery/docs/preprocess-overview).