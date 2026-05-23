# End-to-end user journeys for imported models

This document describes the user journeys for ML models that are imported to
BigQuery ML from Cloud Storage, including the statements and
functions that you can use to work with imported models.
BigQuery ML offers the following types of imported models:

- [Open Neural Network Exchange (ONNX)](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-onnx)
- [TensorFlow](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-tensorflow)
- [TensorFlow Lite](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-tflite)
- [XGBoost](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-xgboost)

## Imported model user journeys

The following table describes the statements and functions you can use to create
and use imported models:

| Model types | Model creation | [Inference](https://docs.cloud.google.com/bigquery/docs/inference-overview) | Tutorials |
|---|---|---|---|
| TensorFlow | [`CREATE MODEL`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-tensorflow) | [`ML.PREDICT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-predict) | [Make predictions with imported TensorFlow model](https://docs.cloud.google.com/bigquery/docs/making-predictions-with-imported-tensorflow-models) |
| TensorFlow Lite | [`CREATE MODEL`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-tflite) | [`ML.PREDICT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-predict) | N/A |
| Open Neural Network Exchange (ONNX) | [`CREATE MODEL`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-onnx) | [`ML.PREDICT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-predict) | - [Make predictions with scikit-learn models in ONNX format](https://docs.cloud.google.com/bigquery/docs/making-predictions-with-sklearn-models-in-onnx-format) - [Make predictions PyTorch models in ONNX format](https://docs.cloud.google.com/bigquery/docs/making-predictions-with-pytorch-models-in-onnx-format) |
| XGBoost | [`CREATE MODEL`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-xgboost) | [`ML.PREDICT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-predict) | N/A |