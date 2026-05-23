# The TimesFM model

This document describes BigQuery ML's built-in
TimesFM time series forecasting model.

The built-in TimesFM univariate model is an implementation of Google Research's
open source
[TimesFM model](https://github.com/google-research/timesfm). The Google Research
TimesFM model is a foundation model for time-series forecasting that has been
pre-trained on billions of time-points from many real-world datasets, so you
can apply it to new forecasting datasets across many domains.
The TimesFM model is available in all BigQuery supported regions.

Using BigQuery ML's built-in TimesFM model with the
[`AI.FORECAST` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-forecast)
lets you perform
forecasting without having to create and train your own model, so you can
avoid the need for model management.
The forecast results from the TimesFM model are comparable to
conventional statistical methods such as ARIMA. If you want more
model tuning options than the TimesFM model offers, you can create an
[`ARIMA_PLUS`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-time-series)
or
[`ARIMA_PLUS_XREG`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-multivariate-time-series)
model and use it with the
[`ML.FORECAST` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-forecast)
instead.

To try using a TimesFM model with the `AI.FORECAST` function, see
[Forecast multiple time series with a TimesFM univariate model](https://docs.cloud.google.com/bigquery/docs/timesfm-time-series-forecasting-tutorial).

To use the TimesFM model to detect anomalies in time series data, use the
[`AI.DETECT_ANOMALIES` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-detect-anomalies).

To evaluate forecasted values from the TimesFM model against the actual values,
use the
[`AI.EVALUATE` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-evaluate).

To learn more about the Google Research TimesFM model, use the following
resources:

- [Google Research blog](https://research.google/blog/a-decoder-only-foundation-model-for-time-series-forecasting/)
- [GitHub repository](https://github.com/google-research/timesfm)
- [Hugging Face page](https://huggingface.co/collections/google/timesfm-release-66e4be5fdb56e960c1e482a6)