# Regression overview

A common use case for machine learning is predicting the value of a numerical
metric for new data by using a model trained on similar historical data.
For example, you might want to predict a house's expected sale price. By using
the house's location and characteristics as features, you can compare this house
to similar houses that have already sold, and use their sales prices to estimate
the house's sale price.

You can use any of the following models in combination with the
[`ML.PREDICT` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-predict)
to perform regression:

- [Linear regression models](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-glm): use [linear regression](https://developers.google.com/machine-learning/crash-course/linear-regression) by setting the `MODEL_TYPE` option to `LINEAR_REG`.
- [Boosted tree models](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-boosted-tree): use a [gradient boosted decision tree](https://developers.google.com/machine-learning/decision-forests/intro-to-gbdt) by setting the `MODEL_TYPE` option to `BOOSTED_TREE_REGRESSOR`.
- [Random forest models](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-random-forest): use a [random forest](https://developers.google.com/machine-learning/decision-forests/intro-to-decision-forests) by setting the `MODEL_TYPE` option to `RANDOM_FOREST_REGRESSOR`.
- [Deep neural network (DNN) models](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-dnn-models): use a [neural network](https://developers.google.com/machine-learning/crash-course/neural-networks) by setting the `MODEL_TYPE` option to `DNN_REGRESSOR`.
- [Wide \& Deep models](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-wnd-models): use [wide \& deep learning](https://dl.acm.org/doi/10.1145/2988450.2988454) by setting the `MODEL_TYPE` option to `DNN_LINEAR_COMBINED_REGRESSOR`.
- [AutoML models](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-automl): use an [AutoML classification model](https://docs.cloud.google.com/vertex-ai/docs/tabular-data/classification-regression/overview) by setting the `MODEL_TYPE` option to `AUTOML_REGRESSOR`.

## Recommended knowledge

By using the default settings in the `CREATE MODEL` statements and the
`ML.PREDICT` function, you can create and use a regression model even
without much ML knowledge. However, having basic knowledge about
ML development helps you optimize both your data and your model to
deliver better results. We recommend using the following resources to develop
familiarity with ML techniques and processes:

- [Machine Learning Crash Course](https://developers.google.com/machine-learning/crash-course)
- [Intro to Machine Learning](https://www.kaggle.com/learn/intro-to-machine-learning)
- [Intermediate Machine Learning](https://www.kaggle.com/learn/intermediate-machine-learning)