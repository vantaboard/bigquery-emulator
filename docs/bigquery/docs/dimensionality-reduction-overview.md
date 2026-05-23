# Dimensionality reduction overview

Dimensionality reduction is the common term for a set of mathematical techniques
used to capture the shape and relationships of data in a high-dimensional space
and translate this information into a low-dimensional space.

Reducing dimensionality is important when you are working with large datasets
that can contain thousands of features. In such a large data space, the wider
range of distances between data points can make model output harder to
interpret. For example, it makes it difficult to understand which data points
are more closely situated and therefore represent more similar data.
Dimensionality reduction helps you reduce the number of features while retaining
the most important characteristics of the dataset. Reducing the number of
features also helps reduce the training time of any models that use the data as
input.

BigQuery ML offers the following models for dimensionality reduction:

- [Principal component analysis (PCA)](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-pca)
- [Autoencoder](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-autoencoder)

You can use PCA and autoencoder models with the
[`ML.PREDICT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-predict)
or
[`AI.GENERATE_EMBEDDING`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-generate-embedding)
functions to embed data into a lower-dimensional space, and with the
[`ML.DETECT_ANOMALIES` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-detect-anomalies)
to perform [anomaly detection](https://docs.cloud.google.com/bigquery/docs/anomaly-detection-overview).

You can use the output from dimensionality reduction models for tasks such as
the following:

- **Similarity search**: Find data points that are similar to each other based on their embeddings. This is great for finding related products, recommending similar content, or identifying duplicate or anomalous items.
- **Clustering**: Use embeddings as input features for k-means models in order to group data points together based on their similarities. This can help you discover hidden patterns and insights in your data.
- **Machine learning**: Use embeddings as input features for classification or regression models.

## Recommended knowledge

By using the default settings in the `CREATE MODEL` statements and the
inference functions, you can create and use a dimensionality reduction model
even without much ML knowledge. However, having basic knowledge about
ML development helps you optimize both your data and your model to
deliver better results. We recommend using the following resources to develop
familiarity with ML techniques and processes:

- [Machine Learning Crash Course](https://developers.google.com/machine-learning/crash-course)
- [Intro to Machine Learning](https://www.kaggle.com/learn/intro-to-machine-learning)
- [Intermediate Machine Learning](https://www.kaggle.com/learn/intermediate-machine-learning)