# Reference patterns

This page provides links to business use cases, sample code, and technical
reference guides for BigQuery ML use cases. Use these resources to
identify best practices and speed up your application development.

## Logistic regression

This pattern shows how to use logistic regression to perform propensity
modeling for gaming applications.

Learn how to use BigQuery ML to train, evaluate, and get
predictions from several different types of propensity models.
Propensity models can help you to determine the likelihood of specific
users returning to your app, so you can use that information in
marketing decisions.

- Blog post: [Churn prediction for game developers using Google Analytics 4 and BigQuery ML](https://cloud.google.com/blog/topics/developers-practitioners/churn-prediction-game-developers-using-google-analytics-4-ga4-and-bigquery-ml)
- Notebook: [Churn prediction solution notebook](https://github.com/GoogleCloudPlatform/analytics-componentized-patterns/tree/master/gaming/propensity-model/bqml)

## Time-series forecasting

These patterns show how to create time-series forecasting solutions.

### Build a demand forecasting model

Learn how to build a time series model that you can use to forecast retail
demand for multiple products.

- Blog post: [How to build demand forecasting models with BigQuery ML](https://cloud.google.com/blog/topics/developers-practitioners/how-build-demand-forecasting-models-bigquery-ml)
- Notebook: [Demand forecasting solution notebook](https://github.com/GoogleCloudPlatform/analytics-componentized-patterns/blob/master/retail/time-series/bqml-demand-forecasting/bqml_retail_demand_forecasting.ipynb)

### Forecast from Google Sheets using BigQuery ML

Learn how to operationalize machine learning with your business
processes by combining
[Connected Sheets](https://docs.cloud.google.com/bigquery/docs/connected-sheets) with a forecasting
model in BigQuery ML. This pattern walks you through
the process for building a forecasting model for website traffic using
Google Analytics data. You can extend this pattern to work
with other data types and other machine learning models.

- Blog post: [How to use a machine learning model from Google Sheets using BigQuery ML](https://cloud.google.com/blog/topics/developers-practitioners/how-use-machine-learning-model-google-sheet-using-bigquery-ml)
- Sample code: [BigQuery ML forecasting with Sheets](https://github.com/googleworkspace/ml-integration-samples/tree/master/apps-script/BQMLForecasting)
- Template: [BigQuery ML forecasting with Sheets](https://docs.google.com/spreadsheets/d/1njedwGjBOkUbTS_HYD0wIPuQIDHgobp1D80qO-OsNH0/copy)

## Anomaly detection

This pattern shows how to use anomaly detection to find real-time credit
card fraud.

Learn how to use transactions and customer data to train machine
learning models in BigQuery ML that can be used in a
real-time data pipeline to identify, analyze, and trigger alerts for
potential credit card fraud.

- Sample code: [Real-time credit card fraud detection](https://github.com/googlecloudplatform/fraudfinder)
- Overview video: [Fraudfinder: A comprehensive solution for real data science problems](https://io.google/2022/program/9a759b60-9a9b-4744-bd22-6e21a4a864cd/)