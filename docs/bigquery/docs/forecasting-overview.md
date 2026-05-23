# Forecasting overview

Forecasting is a technique where you analyze historical data in order to make an
informed prediction about future trends. For example, you might analyze
historical sales data from several store locations in order to predict future
sales at those locations. In BigQuery ML, you perform forecasting on
[time series](https://en.wikipedia.org/wiki/Time_series) data.

You can perform forecasting in the following ways:

- By using the [`AI.FORECAST` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-forecast) with the built-in [TimesFM model](https://docs.cloud.google.com/bigquery/docs/timesfm-model). Use this approach when you need to forecast future values for a single variable. This approach doesn't require you to create and manage a model.
- By using the [`ML.FORECAST` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-forecast) with the [`ARIMA_PLUS` model](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-time-series). Use this approach when you need to run an ARIMA-based modeling pipeline and decompose the time series into multiple components in order to explain the results. This approach requires you to create and manage a model.
- By using the [`ML.FORECAST` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-forecast) with the [`ARIMA_PLUS_XREG` model](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-multivariate-time-series). Use this approach when you need to forecast future values for multiple variables. This approach requires you to create and manage a model.

In addition to forecasting, you can use `ARIMA_PLUS` and `ARIMA_PLUS_XREG`
models for anomaly detection. For more information, see the following
documents:

- [Anomaly detection overview](https://docs.cloud.google.com/bigquery/docs/anomaly-detection-overview)
- [Perform anomaly detection with a multivariate time-series forecasting model](https://docs.cloud.google.com/bigquery/docs/time-series-anomaly-detection-tutorial)

## Compare `ARIMA_PLUS` models and the TimesFM model

Use the following table to determine whether to use TimesFM, `ARIMA_PLUS`, or `ARIMA_PLUS_XREG` model for your use case:

| Model type | `ARIMA_PLUS` and `ARIMA_PLUS_XREG` | `TimesFM` |
|---|---|---|
| Model details | Statistical model that uses the `ARIMA` algorithm for the trend component, and a variety of other algorithms for non-trend components. For more information, see [Time series modeling pipeline](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-time-series#modeling-pipeline) and publication below. | Transformer-based foundation model. For more information, see the publications in the next row. |
| Publication | [ARIMA_PLUS: Large-scale, Accurate, Automatic and Interpretable In-Database Time Series Forecasting and Anomaly Detection in Google BigQuery](https://arxiv.org/abs/2510.24452) | [A Decoder-only Foundation Model for Time-series Forecasting](https://arxiv.org/pdf/2310.10688) |
| Training required | Yes, one `ARIMA_PLUS` or `ARIMA_PLUS_XREG` model is trained for each time series. | No, the TimesFM model is pre-trained. |
| SQL ease of use | High. Requires a `CREATE MODEL` statement and a function call. | Very high. Requires a single function call. |
| Data history used | Uses all time points in the training data, but can be customized to use fewer time points. | Can be customized using the `context_window` parameter of the [`AI.FORECAST` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-forecast). |
| Accuracy | Very high. For more information, see publications listed in a previous row. | Very high. For more information, see publications listed in a previous row. |
| Customization | High. The [`CREATE MODEL` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-time-series) offers arguments that let you tune many model settings, such as the following: - Seasonality - Holiday effects - Step changes - Trend - Spikes and dips removal - Forecasting upper and lower bounds | Low. |
| Supports covariates | Yes, when using the [`ARIMA_PLUS_XREG` model](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-multivariate-time-series). | No. |
| Explainability | High. You can use the [`ML.EXPLAIN_FORECAST` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-explain-forecast) to inspect model components. | Low. |
| Model evaluation | Use the [`ML.ARIMA_EVALUATE` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-arima-evaluate). | Use the [`AI.EVALUATE` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-evaluate). |
| Best use cases | - You want full control of the model including customization. - You need explainability for model output. | - You want minimal setup -- doing forecast without creating a model first. |

## Recommended knowledge

By using the default settings of BigQuery ML's statements and
functions, you can create and use a forecasting model even
without much ML knowledge. However, having basic knowledge about
ML development, and forecasting models in particular,
helps you optimize both your data and your model to
deliver better results. We recommend using the following resources to develop
familiarity with ML techniques and processes:

- [Machine Learning Crash Course](https://developers.google.com/machine-learning/crash-course)
- [Intro to Machine Learning](https://www.kaggle.com/learn/intro-to-machine-learning)
- [Intermediate Machine Learning](https://www.kaggle.com/learn/intermediate-machine-learning)
- [Time Series](https://www.kaggle.com/learn/time-series)