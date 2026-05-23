# The AI.FORECAST function

This document describes the `AI.FORECAST` function, which lets you
forecast a time series by using BigQuery ML's built-in
[TimesFM model](https://docs.cloud.google.com/bigquery/docs/timesfm-model).

Using the `AI.FORECAST` function with the built-in TimesFM model lets you
perform forecasting without having to create and train your own model, so you
can avoid the need for model management.

## Syntax

```sql
SELECT
  *
FROM
  AI.FORECAST(
    { TABLE TABLE | (QUERY_STATEMENT) },
    data_col => 'DATA_COL',
    timestamp_col => 'TIMESTAMP_COL'
    [, model => 'MODEL']
    [, id_cols => ID_COLS]
    [, horizon => HORIZON]
    [, forecast_end_timestamp => FORECAST_END_TIMESTAMP]
    [, confidence_level => CONFIDENCE_LEVEL]
    [, output_historical_time_series => OUTPUT_HISTORICAL_TIME_SERIES]
    [, context_window => CONTEXT_WINDOW]
  )
```

### Arguments

`AI.FORECAST` takes the following arguments:

- `TABLE`: the name of the table that contains the
  data that you want to forecast. For example, `` `mydataset.mytable` ``.

  If the table is in a different project, then you must prepend the
  project ID to the table name in the following format, including backticks:

  `` `[PROJECT_ID].[DATASET].[TABLE]` ``

  For example, `` `myproject.mydataset.mytable` ``.

  To prevent query errors, we recommend providing the fully qualified table
  name, including backticks. This is especially important if the project name
  contains characters other than letters, numbers, and underscores.
- `QUERY_STATEMENT`: the GoogleSQL query that
  generates the data that you want to forecast. See the
  [GoogleSQL query syntax](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#sql_syntax)
  page for the supported SQL syntax of the `QUERY_STATEMENT` clause.

- `DATA_COL`: a `STRING` value that specifies the name of
  the data column. The data column contains the data to forecast. The data
  column must use one of the following data types:

  - `INT64`
  - `NUMERIC`
  - `BIGNUMERIC`
  - `FLOAT64`
- `TIMESTAMP_COL`: a `STRING` value that specified the
  name of the timestamp column. The timestamp
  column must use one of the following data types:

  - `TIMESTAMP`
  - `DATE`
  - `DATETIME`
- `MODEL`: a `STRING` value that specifies the name of the
  model to use. Supported models include `TimesFM 2.0` and `TimesFM 2.5`. The
  default value is `TimesFM 2.0`.

- `ID_COLS`: an `ARRAY<STRING>` value that specifies the
  names of one or more ID columns. Each unique combination of IDs identifies a
  unique time series to forecast. Specify one or more values for this argument
  in order to forecast multiple time series using a single query. The columns
  that you specify must use one of the following data types:

  - `STRING`
  - `INT64`
  - `ARRAY<STRING>`
  - `ARRAY<INT64>`
- `HORIZON`: an `INT64` value that specifies the number of
  time series data points to forecast. The default value is `10`. The valid
  input range is `[1, 10,000]`. This argument can't be used with the
  `forecast_end_timestamp` argument.

- `FORECAST_END_TIMESTAMP`: a timestamp literal value that
  specifies the end timestamp for the forecasted values. The horizon is
  calculated based on the end timestamp and the frequency provided from the
  input table for each time series. If the calculated horizon is out of the
  valid range `[1, 10,000]`, the query returns an error. You can then adjust the
  `forecast_end_timestamp` value so that the calculated horizon is within the
  valid range. This argument can't be used with the `horizon` argument.

- `CONFIDENCE_LEVEL`: a `FLOAT64` value that specifies the
  percentage of the future values that fall in the prediction interval. The
  default value is `0.95`. The valid input range is `[0, 1)`.

- `OUTPUT_HISTORICAL_TIME_SERIES`: a `BOOL` value that
  determines whether the input data is returned along with the forecasted
  data. Set this argument to `TRUE` to return input data. The default value
  is `FALSE`.

  Returning the input data along with the forecasted data lets you compare the
  historical value of the data column with the forecasted value of the data
  column, or chart the change in the data column values over time.
- `CONTEXT_WINDOW`: an `INT64` value that specifies the
  context window length used by BigQuery ML's built-in TimesFM model.
  The context window length determines how many of the most recent data points
  from the input time series are used by the model. For example, if your time
  series date range is March 1 to April 15, data points are selected starting
  at April 15 and working backwards. Valid values for models are as follows:

  | **Model Name** | **Supported Context Window Length** |
  |---|---|
  | TimesFM 2.0 | 64, 128, 256, 512, 1024, 2048 |
  | TimesFM 2.5 | 64, 128, 256, 512, 1024, 2048, 4096, 8192, 15360 |

  If you don't specify a `CONTEXT_WINDOW` value, the `AI.FORECAST` function
  automatically chooses the smallest possible context window length to use that
  is still large enough to cover the number of time series data points in your
  input data. The following table shows the relationships between the number of
  time series data points in the input data, the selected context window length
  and the corresponding supported TimesFM model name:

  | **Number of time series data points** | **Context window length** | **Supported Model Names** |
  |---|---|---|
  | (1, 64\] | 64 | TimesFM 2.0, TimesFM 2.5 |
  | (65, 128\] | 128 | TimesFM 2.0, TimesFM 2.5 |
  | (129, 256\] | 256 | TimesFM 2.0, TimesFM 2.5 |
  | (257, 512\] | 512 | TimesFM 2.0, TimesFM 2.5 |
  | (513, 1024\] | 1,024 | TimesFM 2.0, TimesFM 2.5 |
  | (1025, 2048\] | 2,048 | TimesFM 2.0, TimesFM 2.5 |
  | \>(2049, 4096\] | 4,096 | TimesFM 2.5 |
  | \>(4097, 8192\] | 8,192 | TimesFM 2.5 |
  | \>(8193, 15360\] | 15,360 | TimesFM 2.5 |
  | \>15360 | 15,360 | TimesFM 2.5 |

  For the `TimesFM 2.0` model, 2,048 is the maximum number of time series data
  points that are passed to the model. For the `TimesFM 2.5` model, 15,360 is
  the maximum number of time series data points that are passed to the model.
  Any additional time series data points in the input data are ignored.

## Output

The output schema of `AI.FORECAST` depends on the value of the
`output_historical_time_series` argument. The following columns are always part
of the output:

- `id_cols`: one or more values that contain the identifiers of a time series. `id_cols` can be an `INT64`, `STRING`, `ARRAY<INT64>` or `ARRAY<STRING>` value. The column names and types are inherited from the `ID_COLS` argument value specified in the function input.
- `confidence_level`: a `FLOAT64` value that contains the `confidence_level` value that you specified in the function input, or `0.95` if you didn't specify a `confidence_level` value. This value is the same across all rows.
- `prediction_interval_lower_bound`: a `FLOAT64` value that contains the lower bound of the prediction interval for each forecasted point. For historical points, the value is `NULL`.
- `prediction_interval_upper_bound`: a `FLOAT64` value that contains the upper bound of the prediction interval for each forecasted point. For historical points, the value is `NULL`.
- `ai_forecast_status`: a `STRING` value that contains the forecast status. This value is empty if the operation was successful. If the operation wasn't successful, the value is the error string. A common error is `The time series data is too short.` This error indicates that there wasn't enough historical data in the time series to generate a forecast. A minimum of 3 data points is required.

If you set `output_historical_time_series` to `FALSE`, then the output rows
only include forecasted data and the output columns are the following:

- `forecast_timestamp`: a `TIMESTAMP` value that contains the timestamps of the time series.
- `forecast_value`: a `FLOAT64` value that contains the 50% quantile value for the forecasting output from the model. The 50% quantile value represents the median value of the forecasted data.

If you set `output_historical_time_series` to `TRUE`, then the output rows
include all data, and the output columns are the following:

- `time_series_type`: a `STRING` value that contains a value of either `history` or `forecast`. Rows that have a value of `history` contain data from the input table or query. Rows that have a value of `forecast` contain forecasted data.
- `time_series_timestamp`: a `TIMESTAMP` value that contains the timestamps of the time series.
- `time_series_data`: a `FLOAT64` value that represents the historical input if the `time_series_type` is `history`. Otherwise, if the `time_series_type` is `forecast` this number represents the 50% quantile of the forecast.

## Examples

The following example forecasts the daily number of bike trips for each
different user type for the next 30 days.

    WITH
      citibike_trips AS (
        SELECT EXTRACT(DATE FROM starttime) AS date, usertype, COUNT(*) AS num_trips
        FROM `bigquery-public-data.new_york.citibike_trips`
        GROUP BY date, usertype
      )
    SELECT *
    FROM
      AI.FORECAST(
        TABLE citibike_trips,
        data_col => 'num_trips',
        timestamp_col => 'date',
        id_cols => ['usertype'],
        horizon => 30);

The result is similar to the following:

    +---+---+---+---+---+---+---+
    | usertype   | forecast_timestamp      | forecast_value | confidence_level | prediction_interval_lower_bound | prediction_interval_upper_bound | ai_forecast_status |
    +---+---+---+---+---+---+---+
    | Subscriber | 2016-10-01 00:00:00 UTC | 21225.859375   | 0.95             | -3706.6855220930775             | 46158.404272093074              |                    |
    | Subscriber | 2016-10-02 00:00:00 UTC | 25503.98046875 | 0.95             | 1723.3565528871222              | 49284.604384612874              |                    |
    | ...        | ...                     | ...            | ...              | ...                             | ...                             | ...                |
    +---+---+---+---+---+---+---+

The following example forecasts the daily number of bike trips for each
different user type, the `output_historical_time_series` argument is set to
`TRUE`, so the output includes historical and forecasted data.

    WITH
      citibike_trips AS (
        SELECT EXTRACT(DATE FROM starttime) AS date, usertype, COUNT(*) AS num_trips
        FROM `bigquery-public-data.new_york.citibike_trips`
        GROUP BY date, usertype
      )
    SELECT *
    FROM
      AI.FORECAST(
        TABLE citibike_trips,
        data_col => 'num_trips',
        timestamp_col => 'date',
        id_cols => ['usertype'],
        horizon => 30,
        output_historical_time_series => true);

The result is similar to the following:

    +---+---+---+---+---+---+---+---+
    | usertype | time_series_type | time_series_timestamp   | time_series_data | confidence_level | prediction_interval_lower_bound | prediction_interval_upper_bound | ai_forecast_status |
    +---+---+---+---+---+---+---+---+
    | Customer | history          | 2013-07-01 00:00:00 UTC | 2734.0           | null             | null                            | null                            | null               |
    | Customer | history          | 2013-07-02 00:00:00 UTC | 4070.0           | null             | null                            | null                            | null               |
    | Customer | history          | 2013-07-03 00:00:00 UTC | 4152.0           | null             | null                            | null                            | null               |
    | Customer | history          | 2013-07-04 00:00:00 UTC | 10773.0          | null             | null                            | null                            | null               |
    | ...      | ...              | ...                     | ...              | ...              | ...                             | ...                             | ...                |
    | Customer | forecast         | 2016-10-21 00:00:00 UTC | 3912.359375      | .95              | 862.76858192148256              | 6961.9501680785179              | null               |
    | Customer | forecast         | 2016-10-22 00:00:00 UTC | 5978.39404296875 | .95              | 2670.9416601195244              | 9285.8464258179756              | null               |
    +---+---+---+---+---+---+---+---+

## Locations

`AI.FORECAST` and the TimesFM model are available in all
[supported BigQuery ML locations](https://docs.cloud.google.com/bigquery/docs/locations#locations-for-non-remote-models).

## Pricing

`AI.FORECAST` usage is billed at the evaluation, inspection, and prediction
rate documented in the **BigQuery ML on-demand pricing** section
of the [BigQuery ML pricing](https://cloud.google.com/bigquery/pricing#bigquery-ml-pricing) page.

## What's next

- Try [using a TimesFM model with the `AI.FORECAST` function](https://docs.cloud.google.com/bigquery/docs/timesfm-time-series-forecasting-tutorial).
- Evaluate forecasting results from the TimesFM model using the [`AI.EVALUATE` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-evaluate).
- For information about forecasting in BigQuery ML, see [Forecasting overview](https://docs.cloud.google.com/bigquery/docs/forecasting-overview).
- For more information about supported SQL statements and functions for time series forecasting models, see [End-to-end user journeys for time series forecasting models](https://docs.cloud.google.com/bigquery/docs/e2e-journey-forecast).