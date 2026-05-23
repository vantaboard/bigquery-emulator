# Classification overview

A common use case for machine learning is classifying new data by using a model
trained on similar labeled data. For example, you might want to predict whether
an email is spam, or whether a customer product review is positive, negative, or
neutral.

You can use any of the following models in combination with the
[`ML.PREDICT` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-predict)
to perform classification:

- [Logistic regression models](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-glm): use [logistic regression](https://developers.google.com/machine-learning/crash-course/logistic-regression) by setting the `MODEL_TYPE` option to `LOGISTIC_REG`.
- [Boosted tree models](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-boosted-tree): use a [gradient boosted decision tree](https://developers.google.com/machine-learning/decision-forests/intro-to-gbdt) by setting the `MODEL_TYPE` option to `BOOSTED_TREE_CLASSIFIER`.
- [Random forest models](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-random-forest): use a [random forest](https://developers.google.com/machine-learning/decision-forests/intro-to-decision-forests) by setting the `MODEL_TYPE` option to `RANDOM_FOREST_CLASSIFIER`.
- [Deep neural network (DNN) models](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-dnn-models): use a [neural network](https://developers.google.com/machine-learning/crash-course/neural-networks) by setting the `MODEL_TYPE` option to `DNN_CLASSIFIER`.
- [Wide \& Deep models](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-wnd-models): use [wide \& deep learning](https://dl.acm.org/doi/10.1145/2988450.2988454) by setting the `MODEL_TYPE` option to `DNN_LINEAR_COMBINED_CLASSIFIER`.
- [AutoML models](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-automl): use an [AutoML classification model](https://docs.cloud.google.com/vertex-ai/docs/tabular-data/classification-regression/overview) by setting the `MODEL_TYPE` option to `AUTOML_CLASSIFIER`.

## Recommended knowledge

By using the default settings in the `CREATE MODEL` statements and the
`ML.PREDICT` function, you can create and use a classification model even
without much ML knowledge. However, having basic knowledge about
ML development helps you optimize both your data and your model to
deliver better results. We recommend using the following resources to develop
familiarity with ML techniques and processes:

- [Machine Learning Crash Course](https://developers.google.com/machine-learning/crash-course)
- [Intro to Machine Learning](https://www.kaggle.com/learn/intro-to-machine-learning)
- [Intermediate Machine Learning](https://www.kaggle.com/learn/intermediate-machine-learning)