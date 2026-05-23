# Feature preprocessing overview

*Feature preprocessing* is one of the most important steps in the machine
learning lifecycle. It consists of creating features and cleaning the training
data. Creating features is also referred as *feature engineering*.

BigQuery ML provides the following feature preprocessing techniques:

- **Automatic preprocessing** . BigQuery ML performs automatic
  preprocessing during training. For more information, see [Automatic feature
  preprocessing](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-auto-preprocessing).

- **Manual preprocessing** . You can use the [`TRANSFORM` clause](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create#transform)
  in the `CREATE MODEL` statement to define custom preprocessing using [manual
  preprocessing
  functions](https://docs.cloud.google.com/bigquery/docs/manual-preprocessing#types_of_preprocessing_functions).
  You can also use these functions outside of the `TRANSFORM` clause to
  process training data before creating the model.

## Get feature information

You can use the [`ML.FEATURE_INFO`
function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-feature) to
retrieve the statistics of all input feature columns.

## Recommended knowledge

By using the default settings in the `CREATE MODEL` statements and the
inference functions, you can create and use BigQuery ML models
even without much ML knowledge. However, having basic knowledge about the
ML development lifecycle, such as feature engineering and model training,
helps you optimize both your data and your model to
deliver better results. We recommend using the following resources to develop
familiarity with ML techniques and processes:

- [Machine Learning Crash Course](https://developers.google.com/machine-learning/crash-course)
- [Intro to Machine Learning](https://www.kaggle.com/learn/intro-to-machine-learning)
- [Data Cleaning](https://www.kaggle.com/learn/data-cleaning)
- [Feature Engineering](https://www.kaggle.com/learn/feature-engineering)
- [Intermediate Machine Learning](https://www.kaggle.com/learn/intermediate-machine-learning)

## What's next

- Learn about [feature serving](https://docs.cloud.google.com/bigquery/docs/feature-serving) in BigQuery ML.
- For more information about supported SQL statements and functions for models
  that support feature preprocessing, see the following documents:

  - [End-to-end user journeys for ML models](https://docs.cloud.google.com/bigquery/docs/e2e-journey)
  - [End-to-end user journeys for time series forecasting models](https://docs.cloud.google.com/bigquery/docs/e2e-journey-forecast)
  - [Contribution analysis user journey](https://docs.cloud.google.com/bigquery/docs/contribution-analysis#contribution_analysis_user_journey)