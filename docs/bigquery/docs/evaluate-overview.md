# BigQuery ML model evaluation overview

This document describes how BigQuery ML supports machine learning (ML)
model evaluation.

## Overview of model evaluation

You can use ML model evaluation metrics for the following
purposes:

- To assess the quality of the fit between the model and the data.
- To compare different models.
- To predict how accurately you can expect each model to perform on a specific dataset, in the context of model selection.

Supervised and unsupervised learning model evaluations work differently:

- For supervised learning models, model evaluation is well-defined. An evaluation set, which is data that hasn't been analyzed by the model, is typically excluded from the training set and then used to evaluate model performance. We recommend that you don't use the training set for evaluation because this causes the model to perform poorly when generalizing the prediction results for new data. This outcome is known as *overfitting*.
- For unsupervised learning models, model evaluation is less defined and typically varies from model to model. Because unsupervised learning models don't reserve an evaluation set, the evaluation metrics are calculated using the whole input dataset.

## Model evaluation offerings

BigQuery ML provides the following functions to calculate
evaluation metrics for ML models:

| Model category | Model types | Model evaluation functions | What the function does |
|---|---|---|---|
| Supervised learning | [Linear regression](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-glm) [Boosted trees regressor](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-boosted-tree) [Random forest regressor](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-random-forest) [DNN regressor](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-dnn-models) [Wide-and-deep regressor](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-wnd-models) [AutoML Tables regressor](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-automl) | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-evaluate` | Reports the following metrics: - mean absolute error - mean squared error - mean squared log error - median absolute error - r2 score - explained variance |
| Supervised learning | [Logistic regression](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-glm) [Boosted trees classifier](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-boosted-tree) [Random forest classifier](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-random-forest) [DNN classifier](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-dnn-models) [Wide-and-deep classifier](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-wnd-models) [AutoML Tables classifier](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-automl) | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-evaluate` | Reports the following metrics: - precision - recall - accuracy - F1 score - log loss - roc auc |
| Supervised learning | [Logistic regression](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-glm) [Boosted trees classifier](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-boosted-tree) [Random forest classifier](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-random-forest) [DNN classifier](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-dnn-models) [Wide-and-deep classifier](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-wnd-models) [AutoML Tables classifier](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-automl) | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-confusion` | Reports the [confusion matrix](https://en.wikipedia.org/wiki/Confusion_matrix). |
| Supervised learning | [Logistic regression](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-glm) [Boosted trees classifier](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-boosted-tree) [Random forest classifier](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-random-forest) [DNN classifier](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-dnn-models) [Wide-and-deep classifier](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-wnd-models) [AutoML Tables classifier](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-automl) | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-roc` | Reports metrics for different threshold values, including the following: - recall - false positive rate - true positives - false positives - true negatives - false negatives <br /> Only applies to binary-class classification models. |
| Unsupervised learning | [K-means](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-kmeans) | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-evaluate` | Reports the [Davies-Bouldin index](https://en.wikipedia.org/wiki/Davies%E2%80%93Bouldin_index), and the mean squared distance between data points and the centroids of the assigned clusters. |
| Unsupervised learning | [Matrix factorization](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-matrix-factorization) | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-evaluate` | For [explicit feedback](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-matrix-factorization#feedback_type)-based models, reports the following metrics: - mean absolute error - mean squared error - mean squared log error - median absolute error - r2 score - explained variance |
| Unsupervised learning | [Matrix factorization](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-matrix-factorization) | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-evaluate` | For [implicit feedback](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-matrix-factorization#feedback_type)-based models, reports the following metrics: - [mean average precision](https://en.wikipedia.org/wiki/Evaluation_measures_(information_retrieval)#Mean_average_precision) - mean squared error - [normalized discounted cumulative gain](https://en.wikipedia.org/wiki/Discounted_cumulative_gain#Normalized_DCG) - average rank |
| Unsupervised learning | [PCA](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-pca) | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-evaluate` | Reports the total explained variance ratio. |
| Unsupervised learning | [Autoencoder](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-autoencoder) | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-evaluate` | Reports the following metrics: - mean absolute error - mean squared error - mean squared log error |
| Time series | [ARIMA_PLUS](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-time-series) | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-evaluate` | Reports the following metrics: - mean absolute error - mean squared error - mean absolute percentage error - symmetric mean absolute percentage error <br /> This function requires new data as input. |
| Time series | [ARIMA_PLUS](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-time-series) | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-arima-evaluate` | Reports the following metrics for all ARIMA candidate models characterized by different (p, d, q, has_drift) tuples: - [log_likelihood](https://en.wikipedia.org/wiki/Likelihood_function#Log-likelihood) - [AIC](https://en.wikipedia.org/wiki/Akaike_information_criterion) - variance <br /> It also reports other information about seasonality, holiday effects, and spikes-and-dips outliers. This function doesn't require new data as input. |

## Automatic evaluation in `CREATE MODEL` statements

BigQuery ML supports automatic evaluation during model creation.
Depending on the model type, the data split training options, and whether you're
using hyperparameter tuning, the evaluation metrics are calculated upon
the reserved evaluation dataset, the reserved test dataset, or the entire input
dataset.

- For k-means, PCA, autoencoder, and ARIMA_PLUS models, BigQuery ML
  uses all of the input data as training data, and evaluation metrics are
  calculated against the entire input dataset.

- For linear and logistic regression, boosted tree, random forest, DNN,
  Wide-and-deep, and matrix factorization models, evaluation metrics are
  calculated against the dataset that's specified by the following
  `CREATE MODEL` options:

  - [`DATA_SPLIT_METHOD`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-glm#data_split_method)
  - [`DATA_SPLIT_EVAL_FRACTION`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-glm#data_split_eval_fraction)
  - [`DATA_SPLIT_COL`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-glm#data_split_col)

  When you train these types of models using hyperparameter tuning, the
  [`DATA_SPLIT_TEST_FRACTION`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-hyperparameter-tuning#data_split) option also helps
  define the dataset that the evaluation metrics are calculated against. For
  more information, see
  [Data split](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-hyperparameter-tuning#data_split).
- For AutoML Tables models, see
  [About data splits for AutoML models](https://docs.cloud.google.com/vertex-ai/docs/general/ml-use).

To get evaluation metrics calculated during model creation, use evaluation
functions such as `ML.EVALUATE` on the model with no input data specified.
For an example, see
[`ML.EVALUATE` with no input data specified](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-evaluate#mlevaluate_with_no_input_data_specified).

## Evaluation with a new dataset

After model creation, you can specify new datasets for evaluation. To provide
a new dataset, use evaluation functions like `ML.EVALUATE` on the model with
input data specified. For an example, see
[`ML.EVALUATE` with a custom threshold and input data](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-evaluate#mlevaluate_with_a_custom_threshold_and_input_data).

## What's next

For more information about supported SQL statements and functions for models
that support evaluation, see the following documents:

- [End-to-end user journeys for generative AI models](https://docs.cloud.google.com/bigquery/docs/e2e-journey-genai)
- [End-to-end user journeys for ML models](https://docs.cloud.google.com/bigquery/docs/e2e-journey)