# Model inference overview

This document describes the types of batch inference that BigQuery ML
supports, which include:

- [Batch prediction](https://docs.cloud.google.com/bigquery/docs/inference-overview#prediction)
- [Online prediction](https://docs.cloud.google.com/bigquery/docs/inference-overview#online_prediction)

Machine learning inference is the process of running data points into
a machine learning model to calculate an output such as a single numerical
score. This process is also referred to as "operationalizing a machine learning
model" or "putting a machine learning model into production."

## Batch prediction

The following sections describe the available ways of performing prediction in
BigQuery ML.

### Inference using BigQuery ML trained models

*Prediction* in BigQuery ML is used not only for supervised learning models, but
also unsupervised learning models.

BigQuery ML supports prediction functionalities through the
[`ML.PREDICT` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-predict),
with the following models:

| Model Category | Model Types | What `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-predict` does |
|---|---|---|
| Supervised Learning | [Linear \& logistic regression](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-glm) [Boosted trees](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-boosted-tree) [Random forest](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-random-forest) [Deep Neural Networks](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-dnn-models) [Wide-and-Deep](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-wnd-models) [AutoML Tables](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-automl) | Predict the label, either a numerical value for regression tasks or a categorical value for classification tasks. |
| Unsupervised Learning | [K-means](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-kmeans) | Assign the cluster to the entity. |
| Unsupervised Learning | [PCA](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-pca) | Apply dimensionality reduction to the entity by transforming it into the space spanned by the eigenvectors. |
| Unsupervised Learning | [Autoencoder](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-autoencoder) | Transform the entity into the embedded space. |

### Inference using imported models

With this approach, you create and train a model outside of
BigQuery, import it by using the
[`CREATE MODEL` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create),
and then run inference on it by using the
[`ML.PREDICT` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-predict).
All inference processing occurs in BigQuery, using data from
BigQuery. Imported models can perform supervised or
unsupervised learning.

BigQuery ML supports the following
types of imported models:

- [Open Neural Network Exchange (ONNX)](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-onnx) for models trained in PyTorch, scikit-learn, and other popular ML frameworks.
- [TensorFlow](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-tensorflow)
- [TensorFlow Lite](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-tflite)
- [XGBoost](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-xgboost)

Use this approach to make use of custom models developed with a range
of ML frameworks while taking advantage of BigQuery ML's
inference speed and co-location with data.

To learn more, try one of the following tutorials:

- [Make predictions with imported TensorFlow models](https://docs.cloud.google.com/bigquery/docs/making-predictions-with-imported-tensorflow-models)
- [Make predictions with scikit-learn models in ONNX format](https://docs.cloud.google.com/bigquery/docs/making-predictions-with-sklearn-models-in-onnx-format)
- [Make predictions with PyTorch models in ONNX format](https://docs.cloud.google.com/bigquery/docs/making-predictions-with-pytorch-models-in-onnx-format)

### Inference using remote models

With this approach, you can create a reference to a model
hosted in [Vertex AI Inference](https://docs.cloud.google.com/vertex-ai/docs/predictions/get-predictions)
by using the
[`CREATE MODEL` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model),
and then run inference on it by using the
[`ML.PREDICT` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-predict).
All inference processing occurs in Vertex AI, using data from
BigQuery. Remote models can perform supervised or
unsupervised learning.

Use this approach to run inference against large models that require the GPU
hardware support provided by Vertex AI. If most of your
models are hosted by Vertex AI, this also lets you run
inference against these models by using SQL, without having to manually build
data pipelines to take data to Vertex AI and bring prediction
results back to BigQuery.

For step-by-step instructions, see
[Make predictions with remote models on Vertex AI](https://docs.cloud.google.com/bigquery/docs/bigquery-ml-remote-model-tutorial).

### Batch inference with BigQuery models in Vertex AI

BigQuery ML has built-in support for batch prediction, without the
need to use Vertex AI. It is also possible to register a
BigQuery ML model to Model Registry in order to
perform batch prediction in Vertex AI using a
BigQuery table as input. However, this can only
be done by using the Vertex AI API and setting
[`InstanceConfig.instanceType`](https://docs.cloud.google.com/vertex-ai/docs/reference/rest/v1/projects.locations.batchPredictionJobs#instanceconfig)
to `object`.

## Online prediction

The built-in inference capability of BigQuery ML is optimized for
large-scale use cases, such as batch prediction. While BigQuery ML
delivers low latency inference results when handling small input data, you can
achieve faster online prediction through seamless integration with
[Vertex AI](https://docs.cloud.google.com/bigquery/docs/managing-models-vertex).

You can [manage BigQuery ML models within the Vertex AI environment](https://docs.cloud.google.com/bigquery/docs/managing-models-vertex),
which eliminates the need to export models from BigQuery ML before
deploying them as Vertex AI endpoints. By managing models within
Vertex AI, you get access to all of the Vertex AI MLOps
capabilities, and also to features such as
[Vertex AI Feature Store](https://docs.cloud.google.com/vertex-ai/docs/featurestore/latest/overview).

Additionally, you have the flexibility to
[export BigQuery ML models](https://docs.cloud.google.com/bigquery/docs/exporting-models) to
Cloud Storage for availability on other model hosting platforms.

## What's next

- For more information about using Vertex AI models to generate text and embeddings, see [Generative AI overview](https://docs.cloud.google.com/bigquery/docs/generative-ai-overview).
- For more information about using Cloud AI APIs to perform AI tasks, see [AI application overview](https://docs.cloud.google.com/bigquery/docs/ai-application-overview).
- For more information about supported SQL statements and functions for
  different model types, see the following documents:

  - [End-to-end user journeys for generative AI models](https://docs.cloud.google.com/bigquery/docs/e2e-journey-genai)
  - [End-to-end user journeys for time series forecasting models](https://docs.cloud.google.com/bigquery/docs/e2e-journey-forecast)
  - [End-to-end user journeys for ML models](https://docs.cloud.google.com/bigquery/docs/e2e-journey)
  - [End-to-end user journeys for imported models](https://docs.cloud.google.com/bigquery/docs/e2e-journey-import)