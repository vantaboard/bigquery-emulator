# The ML.HOLIDAY_INFO function

This document describes the `ML.HOLIDAY_INFO` function, which you can use to
return the list of holidays being modeled by an
[`ARIMA_PLUS`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-time-series)
or
[`ARIMA_PLUS_XREG`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-time-series)
time series forecasting model.

## Syntax

```sql
ML.HOLIDAY_INFO(
  MODEL `PROJECT_ID.DATASET.MODEL_NAME`
)
```

### Arguments

`ML.HOLIDAY_INFO` takes the following arguments:

- `PROJECT_ID`: your project ID.
- `DATASET`: the BigQuery dataset that contains the model.
- `MODEL_NAME`: the name of the model.

## Output

`ML.HOLIDAY_INFO` returns the following columns:

- `region`: a `STRING` value that identifies the holiday region.
- `holiday_name`: a `STRING` value that identifies the holiday.
- `primary_date`: a `DATE` value that identifies the date the holiday falls on.
- `preholiday_days`: an `INT64` value that identifies the start of the holiday window around the holiday that was taken into account when modeling.
- `postholiday_days`: an `INT64` value that identifies the end of the holiday window around the holiday that was taken into account when modeling.

## Example

The following example returns the results for a model that uses a
[custom holiday](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-time-series#custom_holidays):

```sql
SELECT * FROM
ML.HOLIDAY_INFO(MODEL `mydataset.arima_model`);
```

The output looks similar to the following:

```
+---+---+---+---+
| region | holiday_name | primary_date | preholiday_days | postholiday_days |
+---+---+
| US     | Members day  | 2001-10-21   | 3               | 1                |
+---+---+---+---+
| US     | Members day  | 2002-10-22   | 3               | 1                |
+---+---+---+---+
| US     | Members day  | 2003-10-21   | 3               | 1                |
+---+---+---+---+
| US     | Members day  | 2004-10-23   | 3               | 1                |
+---+---+---+---+
```

## Limitation

- Results returned by `ML.HOLIDAY_INFO` only indicate the holiday information used during model fitting. They don't necessarily indicate the detection of a holiday effect. Use [`ML.EXPLAIN_FORECAST`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-explain-forecast) instead for actual holiday effect results.

## What's next

- For more information about model evaluation, see [BigQuery ML model evaluation overview](https://docs.cloud.google.com/bigquery/docs/evaluate-overview).
- For more information about supported SQL statements and functions for time series forecasting models, see [End-to-end user journeys for time series forecasting models](https://docs.cloud.google.com/bigquery/docs/e2e-journey-forecast).