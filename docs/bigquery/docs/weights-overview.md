# BigQuery ML model weights overview

This document describes how BigQuery ML supports model weights
discoverability for machine learning (ML) models.

An ML model is an artifact that is saved after running an ML algorithm on
training data. The model represents the rules, numbers,
and any other algorithm-specific data structures that are required to make
predictions. Some examples include the following:

- A linear regression model is comprised of a vector of coefficients that have specific values.
- A decision tree model is comprised of one or more trees of if-then statements that have specific values.
- A deep neural network model is comprised of a graph structure with vectors or matrices of weights that have specific values.

In BigQuery ML, the term *model weights* is used to describe the
components that a model is comprised of.

## Model weights offerings in BigQuery ML

BigQuery ML offers multiple functions that you can use to
retrieve the model weights for different models.

| Model category | Model types | Model weights functions | What the function does |
|---|---|---|---|
| Supervised models | [Linear \& Logistic Regression](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-glm) | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-weights` | Retrieves the feature coefficients and the intercept. |
| Unsupervised models | [Kmeans](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-kmeans) | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-centroids` | Retrieves the feature coefficients for all of the centroids. |
| Unsupervised models | [Matrix Factorization](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-matrix-factorization) | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-weights` | Retrieves the weights of all of the latent factors. They represent the two decomposed matrixes, the user matrix and the item matrix. |
| Unsupervised models | [PCA](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-pca) | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-principal-components` | Retrieves the feature coefficients for all principal components, also known as eigenvectors. |
| Unsupervised models | [PCA](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-pca) | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-principal-component-info` | Retrieves the statistics of each principal component, such as eigenvalue. |
| Time series models | [ARIMA_PLUS](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-time-series) | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-arima-coefficients` | Retrieves the coefficients of the ARIMA model, which is used to model the trend component of the input time series. For information about other components, such as seasonal patterns that are present in the time series, use `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-arima-evaluate`. |

BigQuery ML doesn't support model weight functions for the
following types of models:

- [Boosted tree](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-boosted-tree)
- [Random forest](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-random-forest)
- [Deep neural network (DNN)](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-dnn-models)
- [Wide-and-deep](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-wnd-models)
- [AutoML Tables](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-automl)

To see the weights of all of these model types except for AutoML Tables
models, export the model from BigQuery ML to Cloud Storage.
You can then use the XGBoost library to visualize the tree structure for
boosted tree and random forest models, or the TensorFlow library
to visualize the graph structure for DNN and wide-and-deep models. There is no
method for getting model weight information for AutoML Tables models.

For more information about exporting a model, see
[`EXPORT MODEL` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-export-model)
and
[Export a BigQuery ML model for online prediction](https://docs.cloud.google.com/bigquery/docs/export-model-tutorial).

## What's next

For more information about supported SQL statements and functions for ML models,
see [End-to-end user journeys for ML models](https://docs.cloud.google.com/bigquery/docs/e2e-journey).