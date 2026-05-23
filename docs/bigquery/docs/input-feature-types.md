# Supported input feature types

BigQuery ML supports different input feature types for different model types.
Supported input feature types are listed in the following table:

| Model Category | [Model Types](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create#model_option_list) | [Numeric types](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#numeric_types) ([INT64](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#integer_types), [NUMERIC](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#decimal_types), [BIGNUMERIC](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#decimal_types), [FLOAT64](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#floating_point_types)) | Categorical types ([BOOL](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#boolean_type), [STRING](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#string_type), [BYTES](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#bytes_type), [DATE](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#date_type), [DATETIME](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#datetime_type)) | [TIMESTAMP](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#timestamp_type) | [STRUCT](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#struct_type) | [GEOGRAPHY](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#geography_type) | [ARRAY](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#array_type)\<[Numeric types](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#numeric_types)\> | [ARRAY](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#array_type)\<Categorical types\> | [ARRAY](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#array_type)\<[STRUCT](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#struct_type)\<[INT64](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#integer_types), [Numeric types](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#numeric_types)\>\> |
|---|---|---|---|---|---|---|---|---|---|
| Supervised Learning | Linear \& Logistic Regression | ✔ | ✔ | ✔ | ✔ |   | ✔ | ✔ | ✔ |
| Supervised Learning | Deep Neural Networks | ✔ | ✔ |   | ✔ |   | ✔ | ✔ | ✔ |
| Supervised Learning | Wide-and-Deep | ✔ | ✔ |   | ✔ |   | ✔ | ✔ | ✔ |
| Supervised Learning | Boosted trees | ✔ | ✔ |   | ✔ |   | ✔ | ✔ | ✔ |
| Supervised Learning | AutoML Tables | ✔ | ✔ | ✔ | ✔ |   | ✔ | ✔ |   |
| Unsupervised Learning | K-means | ✔ | ✔ | ✔ | ✔ | ✔ | ✔ | ✔ |   |
| Unsupervised Learning | PCA | ✔ | ✔ | ✔ | ✔ |   | ✔ | ✔ |   |
| Unsupervised Learning | Autoencoder | ✔ | ✔ | ✔ | ✔ |   | ✔ | ✔ | ✔ |
| Time Series Models | ARIMA_PLUS_XREG | ✔ | ✔ | ✔ | ✔ |   |   | ✔ | ✔ |

> [!NOTE]
> **Note:** [Matrix Factorization](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-matrix-factorization#inputs) and [ARIMA_PLUS](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-time-series#time_series_data_col) models have special input feature types. The input types listed for [ARIMA_PLUS_XREG](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-multivariate-time-series#time_series_data_col) are only for external regressors.

## Dense vector input

BigQuery ML supports `ARRAY<numeric>` as dense vector input
during model training. The embedding feature is a special type of dense vector.
see the [`AI.GENERATE_EMBEDDING` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-generate-embedding) for more information.

## Sparse input

BigQuery ML supports `ARRAY<STRUCT>` as sparse input during
model training. Each struct contains an `INT64` value that represents its
zero-based index, and a
[numeric type](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#numeric_types)
that represents the corresponding value.

Below is an example of a sparse tensor input for the integer array
`[0,1,0,0,0,0,1]`:

    ARRAY<STRUCT<k INT64, v INT64>>[(1, 1), (6, 1)] AS f1