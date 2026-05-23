# Anomaly detection overview

Anomaly detection is a data mining technique that you can use to identify data
deviations in a given dataset. For example, if the return rate for a given
product increases substantially from the baseline for that product, that might
indicate a product defect or potential fraud. You can use anomaly detection to
detect critical incidents, such as technical issues, or opportunities, such as
changes in consumer behavior.

It can be challenging to determine what counts as anomalous data. If you aren't
certain what counts as anomalous data, or you don't have labeled
data to train a model on, you can use unsupervised machine learning to perform
anomaly detection. Use the
[`AI.DETECT_ANOMALIES` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-detect-anomalies)
or
[`ML.DETECT_ANOMALIES` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-detect-anomalies)
with one of the following models to detect anomalies in training data or new
serving data:

| Data type | Model types | Function | What the function does |
|---|---|---|---|
| Time series | [`TimesFM`](https://docs.cloud.google.com/bigquery/docs/timesfm-model) | `AI.DETECT_ANOMALIES` | Detect the anomalies in the time series. |
| Time series | [`ARIMA_PLUS`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-time-series) | `ML.DETECT_ANOMALIES` | Detect the anomalies in the time series. |
| Time series | [`ARIMA_PLUS_XREG`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-multivariate-time-series) | `ML.DETECT_ANOMALIES` | Detect the anomalies in the time series with external regressors. |
| [Independent and identically distributed random variables (IID)](https://en.wikipedia.org/wiki/Independent_and_identically_distributed_random_variables) | [K-means](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-kmeans) | `ML.DETECT_ANOMALIES` | Detect anomalies based on the shortest distance among the normalized distances from the input data to each cluster centroid. For a definition of normalized distances, see [the k-means model output for the `ML.DETECT_ANOMALIES` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-detect-anomalies#k-means_model_output). |
| [Independent and identically distributed random variables (IID)](https://en.wikipedia.org/wiki/Independent_and_identically_distributed_random_variables) | [Autoencoder](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-autoencoder) | `ML.DETECT_ANOMALIES` | Detect anomalies based on the reconstruction loss in terms of mean squared error. For more information, see `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-reconstruction-loss`. The `ML.RECONSTRUCTION_LOSS` function can retrieve all types of reconstruction loss. |
| [Independent and identically distributed random variables (IID)](https://en.wikipedia.org/wiki/Independent_and_identically_distributed_random_variables) | [PCA](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-pca) | `ML.DETECT_ANOMALIES` | Detect anomalies based upon the reconstruction loss in terms of mean squared error. |

If you already have labeled data that identifies anomalies, you can
perform anomaly detection by using the
[`ML.PREDICT` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-predict)
with one of the following supervised machine learning models:

- [Linear and logistic regression models](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-glm)
- [Boosted trees models](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-boosted-tree)
- [Random forest models](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-random-forest)
- [Deep neural network (DNN) models](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-dnn-models)
- [Wide \& Deep models](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-wnd-models)
- [AutoML models](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-automl)

## Recommended knowledge

By using the default settings in the `CREATE MODEL` statements and the
inference functions, you can create and use an anomaly detection
model even without much ML knowledge. However, having basic knowledge about
ML development helps you optimize both your data and your model to
deliver better results. We recommend using the following resources to develop
familiarity with ML techniques and processes:

- [Machine Learning Crash Course](https://developers.google.com/machine-learning/crash-course)
- [Intro to Machine Learning](https://www.kaggle.com/learn/intro-to-machine-learning)
- [Intermediate Machine Learning](https://www.kaggle.com/learn/intermediate-machine-learning)