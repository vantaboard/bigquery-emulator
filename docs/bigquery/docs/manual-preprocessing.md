# Manual feature preprocessing

You can use the
[`TRANSFORM` clause](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create#transform)
of the `CREATE MODEL` statement in combination with manual preprocessing
functions to define custom data preprocessing. You can
also use these manual preprocessing functions outside of the `TRANSFORM` clause.

If you want to decouple data preprocessing from model training, you can create a
[transform-only model](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-transform)
that only performs data transformations by using the `TRANSFORM` clause.

You can use the
[`ML.TRANSFORM` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-transform)
to increase the transparency of feature preprocessing. This function lets you
return the preprocessed data from a model's `TRANSFORM` clause, so that you can
see the actual training data that goes into the model training, as well as the
actual prediction data that goes into model serving.

For information about feature preprocessing support in
BigQuery ML, see
[Feature preprocessing overview](https://docs.cloud.google.com/bigquery/docs/preprocess-overview).

## Types of preprocessing functions

There are several types of manual preprocessing functions:

- Scalar functions operate on a single row. For example, [`ML.BUCKETIZE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-bucketize).
- Table-valued functions operate on all rows and output a table. For example, [`ML.FEATURES_AT_TIME`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-feature-time).
- Analytic functions operate on all rows, and output the result for each
  row based on the statistics collected across all rows. For example,
  [`ML.QUANTILE_BUCKETIZE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-quantile-bucketize).

  You must always use an empty `OVER()` clause with ML analytic functions.

  When you use ML analytic functions inside the `TRANSFORM` clause
  during training, the same statistics are automatically applied to
  the input in prediction.

The following sections describe the available preprocessing functions.

### General functions

Use the following function on string or numerical expressions to do data cleanup:

- [`ML.IMPUTER`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-imputer)

### Numerical functions

Use the following functions on numerical expressions to regularize data:

- [`ML.BUCKETIZE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-bucketize)
- [`ML.MAX_ABS_SCALER`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-max-abs-scaler)
- [`ML.MIN_MAX_SCALER`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-min-max-scaler)
- [`ML.NORMALIZER`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-normalizer)
- [`ML.POLYNOMIAL_EXPAND`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-polynomial-expand)
- [`ML.QUANTILE_BUCKETIZE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-quantile-bucketize)
- [`ML.ROBUST_SCALER`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-robust-scaler)
- [`ML.STANDARD_SCALER`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-standard-scaler)

### Categorical functions

Use the following functions on categorical data:

- [`ML.FEATURE_CROSS`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-feature-cross)
- [`ML.HASH_BUCKETIZE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-hash-bucketize)
- [`ML.LABEL_ENCODER`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-label-encoder)
- [`ML.MULTI_HOT_ENCODER`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-multi-hot-encoder)
- [`ML.ONE_HOT_ENCODER`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-one-hot-encoder)

### Text functions

Use the following functions on text string expressions:

- [`ML.NGRAMS`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ngrams)
- [`ML.BAG_OF_WORDS`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-bag-of-words)
- [`ML.TF_IDF`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-tf-idf)

### Image functions

Use the following functions on image data:

- [`ML.CONVERT_COLOR_SPACE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-convert-color-space)
- [`ML.CONVERT_IMAGE_TYPE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-convert-image-type)
- [`ML.DECODE_IMAGE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-decode-image)
- [`ML.RESIZE_IMAGE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-resize-image)

## Known limitations

- BigQuery ML supports both automatic preprocessing and manual preprocessing in the [model export](https://docs.cloud.google.com/bigquery/docs/exporting-models). See the [supported data types](https://docs.cloud.google.com/bigquery/docs/exporting-models#export-transform-types) and [functions](https://docs.cloud.google.com/bigquery/docs/exporting-models#export-transform-functions) for exporting models trained with the [BigQuery ML `TRANSFORM` clause](https://docs.cloud.google.com/bigquery/docs/bigqueryml-transform).

## What's next

For more information about supported SQL statements and functions for models
that support manual feature preprocessing, see the following documents:

- [End-to-end user journeys for ML models](https://docs.cloud.google.com/bigquery/docs/e2e-journey)
- [Contribution analysis user journey](https://docs.cloud.google.com/bigquery/docs/contribution-analysis#contribution_analysis_user_journey)