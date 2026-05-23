# Model monitoring overview

This document describes how BigQuery ML supports monitoring of
machine learning (ML) models through evaluation and comparison the data a model
uses. This includes comparing a model's serving data to its training data,and
comparing new serving data to previously used serving data.

Understanding the data used by your models is a critical aspect of ML, because
this data affects model performance. Understanding any variance between
your training and serving data is especially important in ensuring that your
models remain accurate over time. A model performs best on serving data
that is similar to the training data. When the serving data deviates from the
data used to train the model, the model's performance can deteriorate, even if
the model itself hasn't changed.

BigQuery ML provides functions to help you analyze your training
and serving data for *data skew* and *data drift*:

- *Data skew* occurs when the distribution of feature values for training data is significantly different from serving data in production. Training statistics for the model are saved during model training, so the original training data isn't required for you to use skew detection.
- *Data drift* occurs when feature data distribution in production changes significantly over time. Drift detection is supported for consecutive spans of data, for example, between different days of serving data. This lets you get notified if the serving data is changing over time, before the data sets diverge too much to retrain the model.

Use the following functions to monitor models in BigQuery ML:

- [`ML.DESCRIBE_DATA`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-describe-data): compute descriptive statistics for a set of training or serving data.
- [`ML.VALIDATE_DATA_SKEW`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-validate-data-skew): compute the statistics for a set of serving data, and then compare them to the training data statistics that were computed when a BigQuery ML model was trained, in order to identify anomalous differences between the two data sets. Statistics are only computed for feature columns in the serving data that match feature columns in the training data, in order to achieve better performance and lower cost.
- [`ML.VALIDATE_DATA_DRIFT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-validate-data-drift): compute and compare the statistics for two sets of serving data in order to identify anomalous differences between the two data sets.
- [`ML.TFDV_DESCRIBE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-tfdv-describe): compute fine-grained descriptive statistics for a set of training or serving data. This function provides the same behavior as the [TensorFlow `tfdv.generate_statistics_from_csv` API](https://www.tensorflow.org/tfx/data_validation/api_docs/python/tfdv/generate_statistics_from_csv).
- [`ML.TFDV_VALIDATE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-tfdv-validate): compare the statistics for training and serving data statistics, or two sets of serving data statistics, in order to identify anomalous differences between the two data sets. This function provides the same behavior as the [TensorFlow `validate_statistics` API](https://www.tensorflow.org/tfx/data_validation/api_docs/python/tfdv/validate_statistics).

## Monitoring use cases

This section describes how to use the BigQuery ML model
monitoring functions in common monitoring use cases.

### Basic data skew monitoring

This use case is appropriate when you want to quickly develop and monitor a
model for data skew and don't need fine-grained skew statistics to
integrate with an existing monitoring solution.

Typical steps for this use case are as follows:

1. Run the `ML.DESCRIBE_DATA` function on your training and serving data, to make sure both data sets compare appropriately to each other and are within expected parameters.
2. [Create a BigQuery ML model](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create) and train it on the training data.
3. Run the `ML.VALIDATE_DATA_SKEW` function to compare the serving data statistics with the training data statistics that were computed during model creation in order to see if there's any data skew.
4. If there is data skew, investigate the root cause, adjust the training data appropriately, and then retrain the model.

### Basic data drift monitoring

This use case is appropriate when you want to quickly develop and monitor a
model for data drift and don't need fine-grained drift statistics to
integrate with an existing monitoring solution.

Typical steps for this use case are as follows:

1. Run the `ML.DESCRIBE_DATA` function on your training and serving data to make sure both data sets compare appropriately to each other and are within expected parameters.
2. [Create a BigQuery ML model](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create) and train it on the training data.
3. Run the `ML.VALIDATE_DATA_DRIFT` function to compare the statistics for two different serving data sets in order to see if there's any data drift. For example, you might want to compare the current serving data to historical serving data from a [table snapshot](https://docs.cloud.google.com/bigquery/docs/table-snapshots-intro), or to the features served at a particular point in time, which you can get by using the [`ML.FEATURES_AT_TIME` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-feature-time).
4. If there is data drift, investigate the root cause, adjust the training data appropriately, and then retrain the model.

### Advanced data skew or drift monitoring

This use case is appropriate when you want fine-grained skew or drift statistics
to integrate with an existing monitoring solution or for other purposes.

Typical steps for this use case are as follows:

1. Run the `ML.TFDV_DESCRIBE` function on your training and serving data at intervals appropriate to your monitoring solution, and [save the query results](https://docs.cloud.google.com/bigquery/docs/writing-results). This step lets you compare future serving data to training and serving data from past points in time.
2. Run the `ML.TFDV_VALIDATE` function on your training and serving data
   statistics, or on two sets of serving data statistics, to evaluate data skew
   or feature drift, respectively. The training and serving data must be
   provided as a TensorFlow
   [`DatasetFeatureStatisticsList` protocol buffer](https://www.tensorflow.org/tfx/tf_metadata/api_docs/python/tfmd/proto/statistics_pb2/DatasetFeatureStatisticsList)
   in JSON format. You can generate a protocol buffer in the correct
   format by running the `ML.TFDV_DESCRIBE` function, or you can load it from
   outside of BigQuery. The following example shows how to evaluate
   feature skew:

   ```googlesql
   DECLARE stats1 JSON;
   DECLARE stats2 JSON;

   SET stats1 = (
     SELECT * FROM ML.TFDV_DESCRIBE(TABLE `myproject.mydataset.training`)
   );
   SET stats2 = (
     SELECT * FROM ML.TFDV_DESCRIBE(TABLE `myproject.mydataset.serving`)
   );

   SELECT ML.TFDV_VALIDATE(stats1, stats2, 'SKEW');

   INSERT `myproject.mydataset.serve_stats`
     (t, dataset_feature_statistics_list)
   SELECT CURRENT_TIMESTAMP() AS t, stats1;
   ```
3. If there is data skew or data drift, investigate the root cause,
   adjust the training data appropriately, and then retrain the model.

## Monitoring visualization

Some monitoring functions offer integration with
[Vertex AI model monitoring](https://docs.cloud.google.com/vertex-ai/docs/model-monitoring/overview),
so that you can use charts and graphs to
[analyze model monitoring function output](https://docs.cloud.google.com/vertex-ai/docs/model-monitoring/run-monitoring-job#analyze_monitoring_job_results).

Using Vertex AI visualizations offers the
following benefits:

- **Interactive visualizations**: explore data distributions, skew metrics, and drift metrics by using charts and graphs in the Vertex AI console.
- **Historical analysis**: track model monitoring results over time by using Vertex AI visualizations. This lets you identify trends and patterns in data changes so that you can proactively update and maintain models.
- **Centralized management**: manage monitoring for all BigQuery ML and Vertex AI models in the unified Vertex AI dashboard.

You can enable visualization of the `ML.VALIDATE_DATA_DRIFT` function output
by using that function's `MODEL` argument. You can enable visualization of the
`ML.VALIDATE_DATA_SKEW` function output by using that function's
`enable_visualization_link` argument.

You can only use monitoring visualization with models that are
[registered](https://docs.cloud.google.com/bigquery/docs/managing-models-vertex#register_models) with
Vertex AI. You can register an existing model by using the
[`ALTER MODEL` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-alter-model).

## Monitoring automation

You can automate monitoring by using a
[scheduled query](https://docs.cloud.google.com/bigquery/docs/scheduling-queries) to run the
monitoring function, evaluate the output, and retrain the model
if anomalies are detected. You must enable email notifications as part of
[setting up the scheduled query](https://docs.cloud.google.com/bigquery/docs/scheduling-queries#set_up_scheduled_queries).

For an example that shows how to automate the `ML.VALIDATE_DATA_SKEW`
function, see
[Automate skew detection](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-validate-data-skew#automate_skew_detection).

## What's next

For more information about supported SQL statements and functions for ML models,
see [End-to-end user journey for ML models](https://docs.cloud.google.com/bigquery/docs/e2e-journey).