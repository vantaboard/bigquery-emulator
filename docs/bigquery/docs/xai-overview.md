# BigQuery Explainable AI overview

This document describes how BigQuery ML supports
Explainable artificial intelligence (AI), sometimes called XAI.

Explainable AI helps you understand the results that
your predictive machine learning model generates for classification and
regression tasks by defining how each feature in a row of data contributed to
the predicted result. This information is often referred to as feature
attribution. You can use this information to verify that the model is behaving
as expected, to recognize biases in your models, and to inform ways to
improve your model and your training data.

BigQuery ML and Vertex AI both have Explainable AI
offerings which offer feature-based explanations. You can perform
explainability in BigQuery ML, or you can
[register your model](https://docs.cloud.google.com/bigquery/docs/managing-models-vertex#register_models)
in Vertex AI and perform explainability there.

## Local versus global explainability

There are two types of explainability: local explainability and global
explainability. These are also known respectively as
*local feature importance* and *global feature importance*.

- Local explainability returns feature attribution values for each explained example. These values describe how much a particular feature affected the prediction relative to the baseline prediction.
- Global explainability returns the feature's overall influence on the model and is often obtained by aggregating the feature attributions over the entire dataset. A higher absolute value indicates the feature had a greater influence on the model's predictions.

## Explainable AI offerings in BigQuery ML

Explainable AI in BigQuery ML supports a variety of machine
learning models, including both time series and non-time series models. Each of
the models takes advantage of a different explainability method.

| Model category | Model types | Explainability method | Basic explanation of the method | Local explain functions | Global explain functions |
|---|---|---|---|---|---|
| Supervised models | [Linear \& Logistic Regression](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-glm) | [Shapley values](https://christophm.github.io/interpretable-ml-book/shapley.html#the-shapley-value-in-detail) | Shapley values for linear models are equal to `model weight * feature value`, where feature values are standardized and model weights are trained with the standardized feature values. | [`ML.EXPLAIN_PREDICT`^1^](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-explain-predict) | [`ML.GLOBAL_EXPLAIN`^2^](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-global-explain) |
| Supervised models | [Linear \& Logistic Regression](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-glm) | [Standard Errors](https://en.wikipedia.org/wiki/Standard_error) and [P-values](https://en.wikipedia.org/wiki/P-value) | Standard errors and p-values are used for significance testing against the model weights. | N/A | [`ML.ADVANCED_WEIGHTS`^4^](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-advanced-weights) |
| Supervised models | [Boosted trees](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-boosted-tree) [Random forest](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-random-forest) | [Tree SHAP](https://docs.seldon.io/projects/alibi/en/stable/methods/TreeSHAP.html) | Tree SHAP is an algorithm to compute exact [SHAP values](https://christophm.github.io/interpretable-ml-book/shap.html) for decision tree-based models. | [`ML.EXPLAIN_PREDICT`^1^](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-explain-predict) | [`ML.GLOBAL_EXPLAIN`^2^](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-global-explain) |
| Supervised models | [Boosted trees](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-boosted-tree) [Random forest](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-random-forest) | [Approximate Feature Contribution](http://blog.datadive.net/interpreting-random-forests/) | Approximates the feature contribution values. It is faster and simpler compared to Tree SHAP. | [`ML.EXPLAIN_PREDICT`^1^](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-explain-predict) | [`ML.GLOBAL_EXPLAIN`^2^](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-global-explain) |
| Supervised models | [Boosted trees](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-boosted-tree) [Random forest](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-random-forest) | [Gini Index-based feature importance](https://xgboost.readthedocs.io/en/latest/python/python_api.html#xgboost.XGBRegressor.feature_importances_) | A global feature importance score that indicates how useful or valuable each feature was in the construction of the boosted tree or random forest model during training. | N/A | [`ML.FEATURE_IMPORTANCE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-importance) |
| Supervised models | [Deep Neural Network (DNN)](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-dnn-models) [Wide-and-Deep](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-wnd-models) | [Integrated gradients](https://docs.cloud.google.com/ai-platform/prediction/docs/ai-explanations/overview#ig) | A gradients-based method that efficiently computes feature attributions with the same axiomatic properties as the Shapley value. It provides a sampling approximation of exact feature attributions. Its accuracy is controlled by the [`integrated_gradients_num_steps`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-explain-predict#arguments) parameter. | [`ML.EXPLAIN_PREDICT`^1^](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-explain-predict) | [`ML.GLOBAL_EXPLAIN`^2^](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-global-explain) |
| Supervised models | [AutoML Tables](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-automl) | [Sampled Shapley](https://docs.cloud.google.com/vertex-ai/docs/explainable-ai/overview#compare-methods) | Sampled Shapley assigns credit for the model's outcome to each feature, and considers different permutations of the features. This method provides a sampling approximation of exact Shapley values. | N/A | [`ML.GLOBAL_EXPLAIN`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-global-explain)^2^ |
| Time series models | [ARIMA_PLUS](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-time-series) | [Time series decomposition](https://otexts.com/fpp2/decomposition.html) | Decomposes the time series into multiple components if those components are present in the time series. The components include trend, seasonal, holiday, step changes, and spike and dips. See ARIMA_PLUS [modeling pipeline](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-time-series#modeling-pipeline) for more details. | [`ML.EXPLAIN_FORECAST`^3^](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-explain-forecast) | N/A |
| Time series models | [ARIMA_PLUS_XREG](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-multivariate-time-series) | [Time series decomposition](https://otexts.com/fpp2/decomposition.html) and [Shapley values](https://christophm.github.io/interpretable-ml-book/shapley.html#the-shapley-value-in-detail) | Decomposes the time series into multiple components, including trend, seasonal, holiday, step changes, and spike and dips (similar to [ARIMA_PLUS](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-time-series)). Attribution of each external regressor is calculated based on Shapley Values, which is equal to `model weight * feature value`. | [`ML.EXPLAIN_FORECAST`^3^](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-explain-forecast) | N/A |

^1^`ML_EXPLAIN_PREDICT` is an extended version of `ML.PREDICT`.

^2^`ML.GLOBAL_EXPLAIN` returns the global explainability
obtained by taking the mean absolute attribution that each feature receives for
all the rows in the evaluation dataset.

^3^`ML.EXPLAIN_FORECAST` is an extended version of `ML.FORECAST`.

^4^`ML.ADVANCED_WEIGHTS` is an extended version of `ML.WEIGHTS`.

## Explainable AI in Vertex AI

Explainable AI is available in Vertex AI for the following
subset of exportable supervised learning models:

| Model type | Explainable AI method |
|---|---|
| dnn_classifier | Integrated gradients |
| dnn_regressor | Integrated gradients |
| dnn_linear_combined_classifier | Integrated gradients |
| dnn_linear_combined_regressor | Integrated gradients |
| boosted_tree_regressor | Sampled shapley |
| boosted_tree_classifier | Sampled shapley |
| random_forest_regressor | Sampled shapley |
| random_forest_classifier | Sampled shapley |

See
[Feature Attribution Methods](https://docs.cloud.google.com/vertex-ai/docs/explainable-ai/overview#feature-attribution-methods)
to learn more about these methods.

### Enable Explainable AI in Model Registry

When your BigQuery ML model is registered in
Model Registry, and if it is a type of model that supports
Explainable AI, you can enable Explainable AI on the model when deploying to an
endpoint. When you register your BigQuery ML model, all of the
associated metadata is populated for you.

> [!NOTE]
> **Note:** Explainable AI incurs a minor additional cost. See [Vertex AI pricing](https://cloud.google.com/vertex-ai/pricing) to learn more.

1. [Register your BigQuery ML model to the Model Registry](https://docs.cloud.google.com/bigquery/docs/managing-models-vertex#register_models).
2. Go to the **Model Registry** page from the BigQuery section in the Google Cloud console.
3. From the Model Registry, select the BigQuery ML model and click the model version to redirect to the model detail page.
4. Select **More actions** from the model version.
5. Click **Deploy to endpoint**.
6. Define your endpoint - create an endpoint name and click continue.
7. Select a machine type, for example, `n1-standard-2`.
8. Under **Model settings**, in the logging section, select the checkbox to enable Explainability options.
9. Click **Done** , and then **Continue** to deploy to the endpoint.

To learn how to use XAI on your models from the
Model Registry, see
[Get an online explanation using your deployed model](https://docs.cloud.google.com/vertex-ai/docs/tabular-data/classification-regression/get-online-predictions#online-explanation).
To learn more about XAI in Vertex AI, see
[Get explanations](https://docs.cloud.google.com/vertex-ai/docs/explainable-ai/getting-explanations).

## What's next

- Learn how to [manage BigQuery ML models in Vertex AI](https://docs.cloud.google.com/bigquery/docs/managing-models-vertex).
- For more information about supported SQL statements and functions for models that support explainability, see [End-to-end user journeys for ML models](https://docs.cloud.google.com/bigquery/docs/e2e-journey).