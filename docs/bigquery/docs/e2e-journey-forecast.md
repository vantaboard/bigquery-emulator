# End-to-end user journeys for time series forecasting models

This document describes the user journeys for BigQuery ML
time series forecasting models, including the statements and functions that
you can use to work with time series forecasting models.
BigQuery ML offers the following types of time series
forecasting models:

- [`ARIMA_PLUS` univariate models](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-time-series)
- [`ARIMA_PLUS_XREG` multivariate models](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-multivariate-time-series)
- [TimesFM univariate model](https://docs.cloud.google.com/bigquery/docs/timesfm-model)

## Model creation user journeys

The following table describes the statements and functions you can use to create
time series forecasting models:

| Model type | Model creation | [Feature preprocessing](https://docs.cloud.google.com/bigquery/docs/preprocess-overview) | [Hyperparameter tuning](https://docs.cloud.google.com/bigquery/docs/hp-tuning-overview) | [Model weights](https://docs.cloud.google.com/bigquery/docs/weights-overview) | Tutorials |
|---|---|---|---|---|---|
| `ARIMA_PLUS` | [`CREATE MODEL`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-time-series) | [Automatic preprocessing](https://docs.cloud.google.com/bigquery/docs/auto-preprocessing) | [auto.ARIMA^1^](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-time-series#auto_arima) automatic tuning | [`ML.ARIMA_COEFFICIENTS`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-arima-coefficients) | - [Forecast a single time series](https://docs.cloud.google.com/bigquery/docs/arima-single-time-series-forecasting-tutorial) - [Forecast multiple time series](https://docs.cloud.google.com/bigquery/docs/arima-multiple-time-series-forecasting-tutorial) - [Forecast millions of time series](https://docs.cloud.google.com/bigquery/docs/arima-speed-up-tutorial) - [Use custom holidays](https://docs.cloud.google.com/bigquery/docs/time-series-forecasting-holidays-tutorial) - [Limit forecasted values](https://docs.cloud.google.com/bigquery/docs/arima-time-series-forecasting-with-limits-tutorial) - [Perform hierarchical time series forecasting](https://docs.cloud.google.com/bigquery/docs/arima-time-series-forecasting-with-hierarchical-time-series) - [Perform anomaly detection with a multivariate time-series forecasting model](https://docs.cloud.google.com/bigquery/docs/time-series-anomaly-detection-tutorial) |
| `ARIMA_PLUS_XREG` | [`CREATE MODEL`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-multivariate-time-series) | [Automatic preprocessing](https://docs.cloud.google.com/bigquery/docs/auto-preprocessing) | [auto.ARIMA^1^](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-multivariate-time-series#auto_arima) automatic tuning | [`ML.ARIMA_COEFFICIENTS`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-arima-coefficients) | - [Forecast a single time series](https://docs.cloud.google.com/bigquery/docs/arima-plus-xreg-single-time-series-forecasting-tutorial) - [Forecast multiple time series](https://docs.cloud.google.com/bigquery/docs/arima-plus-xreg-multiple-time-series-forecasting-tutorial) |
| TimesFM | N/A | N/A | N/A | N/A | [Forecast multiple time series](https://docs.cloud.google.com/bigquery/docs/timesfm-time-series-forecasting-tutorial) |

^1^The auto.ARIMA algorithm performs hyperparameter tuning for the
trend module. The entire modeling pipeline doesn't support hyperparameter
tuning. See the
[modeling pipeline](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-time-series#modeling-pipeline) for more details.

## Model use user journeys

The following table describes the statements and functions you can use to
evaluate, explain, and get forecasts from time series forecasting models:

| Model type | [Evaluation](https://docs.cloud.google.com/bigquery/docs/evaluate-overview) | [Inference](https://docs.cloud.google.com/bigquery/docs/inference-overview) | [AI explanation](https://docs.cloud.google.com/bigquery/docs/xai-overview) |
|---|---|---|---|
| `ARIMA_PLUS` | [`ML.EVALUATE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-evaluate)^1^ [`ML.ARIMA_EVALUATE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-arima-evaluate) [`ML.HOLIDAY_INFO`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-holiday-info) | [`ML.FORECAST`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-forecast) [`ML.DETECT_ANOMALIES`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-detect-anomalies) | [`ML.EXPLAIN_FORECAST`^2^](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-explain-forecast) |
| `ARIMA_PLUS_XREG` | [`ML.EVALUATE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-evaluate)^1^ [`ML.ARIMA_EVALUATE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-arima-evaluate) [`ML.HOLIDAY_INFO`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-holiday-info) | [`ML.FORECAST`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-forecast) [`ML.DETECT_ANOMALIES`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-detect-anomalies) | [`ML.EXPLAIN_FORECAST`^2^](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-explain-forecast) |
| TimesFM | [`AI.EVALUATE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-evaluate) | [`AI.FORECAST`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-forecast) | N/A |

^1^You can input evaluation data to the `ML.EVALUATE` function
to compute forecasting metrics such as mean absolute percentage error (MAPE).
If you don't have evaluation data, you can use the
`ML.ARIMA_EVALUATE` function to output information about the
model like drift and variance.

^2^The `ML.EXPLAIN_FORECAST` function encompasses the
`ML.FORECAST` function because its output is a superset of the
results of `ML.FORECAST`.