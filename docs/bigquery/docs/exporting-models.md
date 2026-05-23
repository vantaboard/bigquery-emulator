# Export models

This page shows you how to export BigQuery ML models. You can export BigQuery ML models to Cloud Storage, and use them for online prediction, or edit them in Python. You can export a
BigQuery ML model by:

- Using the [Google Cloud console](https://docs.cloud.google.com/bigquery/docs/exporting-models).
- Using the [`EXPORT MODEL`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-export-model) statement.
- Using the `bq extract` command in the bq command-line tool.
- Submitting an [`extract`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/Job#JobConfiguration) job through the API or client libraries.

You can export the following model types:

- `AUTOENCODER`
- `AUTOML_CLASSIFIER`
- `AUTOML_REGRESSOR`
- `BOOSTED_TREE_CLASSIFIER`
- `BOOSTED_TREE_REGRESSOR`
- `DNN_CLASSIFIER`
- `DNN_REGRESSOR`
- `DNN_LINEAR_COMBINED_CLASSIFIER`
- `DNN_LINEAR_COMBINED_REGRESSOR`
- `KMEANS`
- `LINEAR_REG`
- `LOGISTIC_REG`
- `MATRIX_FACTORIZATION`
- `RANDOM_FOREST_CLASSIFIER`
- `RANDOM_FOREST_REGRESSOR`
- `TENSORFLOW` (imported TensorFlow models)
- `PCA`
- `TRANSFORM_ONLY`

## Export model formats and samples

The following table shows the export destination formats for each
BigQuery ML model type and provides a sample of files that get written
in the Cloud Storage bucket.

| Model type | Export model format | Exported files sample |
|---|---|---|
| AUTOML_CLASSIFIER | [TensorFlow SavedModel](https://www.tensorflow.org/guide/saved_model) (TF 2.1.0) | `gcs_bucket/ assets/ f1.txt f2.txt saved_model.pb variables/ variables.data-00-of-01 variables.index` |
| AUTOML_REGRESSOR | [TensorFlow SavedModel](https://www.tensorflow.org/guide/saved_model) (TF 2.1.0) | `gcs_bucket/ assets/ f1.txt f2.txt saved_model.pb variables/ variables.data-00-of-01 variables.index` |
| AUTOENCODER | [TensorFlow SavedModel](https://www.tensorflow.org/guide/saved_model) (TF 1.15 or higher) | `gcs_bucket/ assets/ f1.txt f2.txt saved_model.pb variables/ variables.data-00-of-01 variables.index` |
| DNN_CLASSIFIER | [TensorFlow SavedModel](https://www.tensorflow.org/guide/saved_model) (TF 1.15 or higher) | `gcs_bucket/ assets/ f1.txt f2.txt saved_model.pb variables/ variables.data-00-of-01 variables.index` |
| DNN_REGRESSOR | [TensorFlow SavedModel](https://www.tensorflow.org/guide/saved_model) (TF 1.15 or higher) | `gcs_bucket/ assets/ f1.txt f2.txt saved_model.pb variables/ variables.data-00-of-01 variables.index` |
| DNN_LINEAR_COMBINED_CLASSIFIER | [TensorFlow SavedModel](https://www.tensorflow.org/guide/saved_model) (TF 1.15 or higher) | `gcs_bucket/ assets/ f1.txt f2.txt saved_model.pb variables/ variables.data-00-of-01 variables.index` |
| DNN_LINEAR_COMBINED_REGRESSOR | [TensorFlow SavedModel](https://www.tensorflow.org/guide/saved_model) (TF 1.15 or higher) | `gcs_bucket/ assets/ f1.txt f2.txt saved_model.pb variables/ variables.data-00-of-01 variables.index` |
| KMEANS | [TensorFlow SavedModel](https://www.tensorflow.org/guide/saved_model) (TF 1.15 or higher) | `gcs_bucket/ assets/ f1.txt f2.txt saved_model.pb variables/ variables.data-00-of-01 variables.index` |
| LINEAR_REGRESSOR | [TensorFlow SavedModel](https://www.tensorflow.org/guide/saved_model) (TF 1.15 or higher) | `gcs_bucket/ assets/ f1.txt f2.txt saved_model.pb variables/ variables.data-00-of-01 variables.index` |
| LOGISTIC_REG | [TensorFlow SavedModel](https://www.tensorflow.org/guide/saved_model) (TF 1.15 or higher) | `gcs_bucket/ assets/ f1.txt f2.txt saved_model.pb variables/ variables.data-00-of-01 variables.index` |
| MATRIX_FACTORIZATION | [TensorFlow SavedModel](https://www.tensorflow.org/guide/saved_model) (TF 1.15 or higher) | `gcs_bucket/ assets/ f1.txt f2.txt saved_model.pb variables/ variables.data-00-of-01 variables.index` |
| PCA | [TensorFlow SavedModel](https://www.tensorflow.org/guide/saved_model) (TF 1.15 or higher) | `gcs_bucket/ assets/ f1.txt f2.txt saved_model.pb variables/ variables.data-00-of-01 variables.index` |
| TRANSFORM_ONLY | [TensorFlow SavedModel](https://www.tensorflow.org/guide/saved_model) (TF 1.15 or higher) | `gcs_bucket/ assets/ f1.txt f2.txt saved_model.pb variables/ variables.data-00-of-01 variables.index` |
| BOOSTED_TREE_CLASSIFIER | Booster (XGBoost 0.82) | `gcs_bucket/ assets/ 0.txt 1.txt model_metadata.json main.py model.bst xgboost_predictor-0.1.tar.gz .... predictor.py ....` <br /> `main.py` is for local run. See [Model deployment](https://docs.cloud.google.com/bigquery/docs/exporting-models#model-deployment) for more details. |
| BOOSTED_TREE_REGRESSOR | Booster (XGBoost 0.82) | `gcs_bucket/ assets/ 0.txt 1.txt model_metadata.json main.py model.bst xgboost_predictor-0.1.tar.gz .... predictor.py ....` <br /> `main.py` is for local run. See [Model deployment](https://docs.cloud.google.com/bigquery/docs/exporting-models#model-deployment) for more details. |
| RANDOM_FOREST_REGRESSOR | Booster (XGBoost 0.82) | `gcs_bucket/ assets/ 0.txt 1.txt model_metadata.json main.py model.bst xgboost_predictor-0.1.tar.gz .... predictor.py ....` <br /> `main.py` is for local run. See [Model deployment](https://docs.cloud.google.com/bigquery/docs/exporting-models#model-deployment) for more details. |
| RANDOM_FOREST_REGRESSOR | Booster (XGBoost 0.82) | `gcs_bucket/ assets/ 0.txt 1.txt model_metadata.json main.py model.bst xgboost_predictor-0.1.tar.gz .... predictor.py ....` <br /> `main.py` is for local run. See [Model deployment](https://docs.cloud.google.com/bigquery/docs/exporting-models#model-deployment) for more details. |
| TENSORFLOW (imported) | [TensorFlow SavedModel](https://www.tensorflow.org/guide/saved_model) | Exactly the same files that were present when importing the model |

> [!NOTE]
> **Note:** The [automatic data preprocessing](https://docs.cloud.google.com/bigquery/docs/auto-preprocessing) performed during model creation, such as standardization and label encoding, is saved in the exported files as part of the graph for TensorFlow SavedModel, and in the external files for Booster. Explicit preprocessing is unneeded before passing data for prediction. Input should generally match that used for BigQuery ML [`ML.PREDICT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-predict). All numerical values in the exported model signatures are cast as data type `FLOAT64`. Also, all `STRUCT` fields must be expanded into separate fields. For example, field `f1` in `STRUCT f2` should be renamed as `f2_f1` and passed as a separate column.

## Export model trained with `TRANSFORM`

If the model is trained with the
[`TRANSFORM` clause](https://docs.cloud.google.com/bigquery/docs/bigqueryml-transform),
then an additional preprocessing model performs the same logic in the
[`TRANSFORM` clause](https://docs.cloud.google.com/bigquery/docs/bigqueryml-transform) and is saved in the
TensorFlow SavedModel format under the subdirectory `transform`.
You can deploy a model trained with the [`TRANSFORM` clause](https://docs.cloud.google.com/bigquery/docs/bigqueryml-transform)
to Vertex AI as well as locally. For more information, see
[model deployment](https://docs.cloud.google.com/bigquery/docs/exporting-models#model-deployment).

| Export model format | Exported files sample |
|---|---|
| Prediction model: [TensorFlow SavedModel](https://www.tensorflow.org/guide/saved_model) or Booster (XGBoost 0.82). Preprocessing model for TRANSFORM clause: [TensorFlow SavedModel](https://www.tensorflow.org/guide/saved_model) (TF 2.5 or higher) | `gcs_bucket/ ....(model files) transform/ assets/ f1.txt/ f2.txt/ saved_model.pb variables/ variables.data-00-of-01 variables.index ` |
| `gcs_bucket/ ....(model files) transform/ assets/ f1.txt/ f2.txt/ saved_model.pb variables/ variables.data-00-of-01 variables.index ` |
| `gcs_bucket/ ....(model files) transform/ assets/ f1.txt/ f2.txt/ saved_model.pb variables/ variables.data-00-of-01 variables.index ` |
| `gcs_bucket/ ....(model files) transform/ assets/ f1.txt/ f2.txt/ saved_model.pb variables/ variables.data-00-of-01 variables.index ` |

The model doesn't contain the information about the feature engineering
performed outside the [`TRANSFORM` clause](https://docs.cloud.google.com/bigquery/docs/bigqueryml-transform)
during training. For example, anything in the `SELECT` statement. So you would
need to manually convert the input data before feeding into the preprocessing
model.

### Supported data types

When exporting models trained with the [`TRANSFORM` clause](https://docs.cloud.google.com/bigquery/docs/bigqueryml-transform),
the following data types are supported for feeding into the
[`TRANSFORM` clause](https://docs.cloud.google.com/bigquery/docs/bigqueryml-transform).

| TRANSFORM input type | TRANSFORM input samples | Exported preprocessing model input samples |
|---|---|---|
| INT64 | `10, 11 ` | `tf.constant( [10, 11], dtype=tf.int64) ` |
| NUMERIC | `NUMERIC 10, NUMERIC 11 ` | `tf.constant( [10, 11], dtype=tf.float64) ` |
| BIGNUMERIC | `BIGNUMERIC 10, BIGNUMERIC 11 ` | `tf.constant( [10, 11], dtype=tf.float64) ` |
| FLOAT64 | `10.0, 11.0 ` | `tf.constant( [10, 11], dtype=tf.float64) ` |
| BOOL | `TRUE, FALSE ` | `tf.constant( [True, False], dtype=tf.bool) ` |
| STRING | `'abc', 'def' ` | `tf.constant( ['abc', 'def'], dtype=tf.string) ` |
| BYTES | `b'abc', b'def' ` | `tf.constant( ['abc', 'def'], dtype=tf.string) ` |
| DATE | `DATE '2020-09-27', DATE '2020-09-28' ` | `tf.constant( [ '2020-09-27', '2020-09-28' ], dtype=tf.string) "%F" format ` |
| DATETIME | `DATETIME '2023-02-02 02:02:01.152903', DATETIME '2023-02-03 02:02:01.152903' ` | `tf.constant( [ '2023-02-02 02:02:01.152903', '2023-02-03 02:02:01.152903' ], dtype=tf.string) "%F %H:%M:%E6S" format ` |
| TIME | `TIME '16:32:36.152903', TIME '17:32:36.152903' ` | `tf.constant( [ '16:32:36.152903', '17:32:36.152903' ], dtype=tf.string) "%H:%M:%E6S" format ` |
| TIMESTAMP | `TIMESTAMP '2017-02-28 12:30:30.45-08', TIMESTAMP '2018-02-28 12:30:30.45-08' ` | `tf.constant( [ '2017-02-28 20:30:30.4 +0000', '2018-02-28 20:30:30.4 +0000' ], dtype=tf.string) "%F %H:%M:%E1S %z" format ` |
| ARRAY | `['a', 'b'], ['c', 'd'] ` | `tf.constant( [['a', 'b'], ['c', 'd']], dtype=tf.string) ` |
| ARRAY\< STRUCT\< INT64, FLOAT64\>\> | `[(1, 1.0), (2, 1.0)], [(2, 1.0), (3, 1.0)] ` | `tf.sparse.from_dense( tf.constant( [ [0, 1.0, 1.0, 0], [0, 0, 1.0, 1.0] ], dtype=tf.float64)) ` |
| NULL | `NULL, NULL ` | `tf.constant( [123456789.0e10, 123456789.0e10], dtype=tf.float64) tf.constant( [1234567890000000000, 1234567890000000000], dtype=tf.int64) tf.constant( [' __MISSING__ ', ' __MISSING__ '], dtype=tf.string) ` |

### Supported SQL functions

When exporting models trained with the [`TRANSFORM` clause](https://docs.cloud.google.com/bigquery/docs/bigqueryml-transform),
you can use the following SQL functions inside the [`TRANSFORM` clause](https://docs.cloud.google.com/bigquery/docs/bigqueryml-transform)
.

- [Operators](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/operators)
  - `+`, `-`, `*`, `/`, `=`, `<`, `>`, `<=`, `>=`, `!=`, `<>`, `[NOT] BETWEEN`, `[NOT] IN`, `IS [NOT] NULL`, `IS [NOT] TRUE`, `IS [NOT] FALSE`, `NOT`, `AND`, `OR`.
- [Conditional expressions](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/conditional_expressions)
  - `CASE expr`, `CASE`, `COALESCE`, `IF`, `IFNULL`, `NULLIF`.
- [Mathematical functions](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions)
  - `ABS`, `ACOS`, `ACOSH`, `ASINH`, `ATAN`, `ATAN2`, `ATANH`, `CBRT`, `CEIL`, `CEILING`, `COS`, `COSH`, `COT`, `COTH`, `CSC`, `CSCH`, `EXP`, `FLOOR`, `IS_INF`, `IS_NAN`, `LN`, `LOG`, `LOG10`, `MOD`, `POW`, `POWER`, `SEC`, `SECH`, `SIGN`, `SIN`, `SINH`, `SQRT`, `TAN`, `TANH`.
- [Conversion functions](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/conversion_functions)
  - `CAST AS INT64`, `CAST AS FLOAT64`, `CAST AS NUMERIC`, `CAST AS BIGNUMERIC`, `CAST AS STRING`, `SAFE_CAST AS INT64`, `SAFE_CAST AS FLOAT64`
- [String functions](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions)
  - `CONCAT`, `LEFT`, `LENGTH`, `LOWER`, `REGEXP_REPLACE`, `RIGHT`, `SPLIT`, `SUBSTR`, `SUBSTRING`, `TRIM`, `UPPER`.
- [Date functions](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions)
  - `Date`, `DATE_ADD`, `DATE_SUB`, `DATE_DIFF`, `DATE_TRUNC`, `EXTRACT`, `FORMAT_DATE`, `PARSE_DATE`, `SAFE.PARSE_DATE`.
- [Datetime functions](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/datetime_functions)
  - `DATETIME`, `DATETIME_ADD`, `DATETIME_SUB`, `DATETIME_DIFF`, `DATETIME_TRUNC`, `EXTRACT`, `PARSE_DATETIME`, `SAFE.PARSE_DATETIME`.
- [Time functions](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/time_functions)
  - `TIME`, `TIME_ADD`, `TIME_SUB`, `TIME_DIFF`, `TIME_TRUNC`, `EXTRACT`, `FORMAT_TIME`, `PARSE_TIME`, `SAFE.PARSE_TIME`.
- [Timestamp functions](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions)
  - `TIMESTAMP`, `TIMESTAMP_ADD`, `TIMESTAMP_SUB`, `TIMESTAMP_DIFF`, `TIMESTAMP_TRUNC`, `FORMAT_TIMESTAMP`, `PARSE_TIMESTAMP`, `SAFE.PARSE_TIMESTAMP`, `TIMESTAMP_MICROS`, `TIMESTAMP_MILLIS`, `TIMESTAMP_SECONDS`, `EXTRACT`, `STRING`, `UNIX_MICROS`, `UNIX_MILLIS`, `UNIX_SECONDS`.
- [Manual preprocessing functions](https://docs.cloud.google.com/bigquery/docs/manual-preprocessing)
  - `ML.IMPUTER`, `ML.HASH_BUCKETIZE`, `ML.LABEL_ENCODER`, `ML.MULTI_HOT_ENCODER`, `ML.NGRAMS`, `ML.ONE_HOT_ENCODER`, `ML.BUCKETIZE`, `ML.MAX_ABS_SCALER`, `ML.MIN_MAX_SCALER`, `ML.NORMALIZER`, `ML.QUANTILE_BUCKETIZE`, `ML.ROBUST_SCALER`, `ML.STANDARD_SCALER`.

## Limitations

The following limitations apply when exporting models:

- Model export is not supported if any of the following features were used
  during training:

  - `ARRAY`, `TIMESTAMP`, or `GEOGRAPHY` feature types were present in the input data.
- Exported models for model types `AUTOML_REGRESSOR` and `AUTOML_CLASSIFIER`
  do not support Vertex AI deployment for online prediction.

- The model size limit is 1 GB for matrix factorization model export.
  The model size is roughly proportional to `num_factors`, so you can reduce
  `num_factors` during training to shrink the model size if you reach the limit.

- For models trained with the
  [BigQuery ML `TRANSFORM` clause](https://docs.cloud.google.com/bigquery/docs/bigqueryml-transform)
  for [manual feature preprocessing](https://docs.cloud.google.com/bigquery/docs/manual-preprocessing),
  see the [data types](https://docs.cloud.google.com/bigquery/docs/exporting-models#export-transform-types)
  and [functions](https://docs.cloud.google.com/bigquery/docs/exporting-models#export-transform-functions)
  supported for exporting.

- Models trained with the [BigQuery ML `TRANSFORM` clause](https://docs.cloud.google.com/bigquery/docs/bigqueryml-transform)
  before 18 September 2023 must be re-trained before they can be
  [deployed through Model Registry](https://docs.cloud.google.com/bigquery/docs/managing-models-vertex)
  for online prediction.

- During model export, `ARRAY<STRUCT<INT64, FLOAT64>>`, `ARRAY` and
  `TIMESTAMP` are supported as pre-transformed data, but are not supported as
  post-transformed data.

## Export BigQuery ML models

To export a model, select one of the following:

### Console

1. Open the BigQuery page in the Google Cloud console.  

   [Go to the BigQuery page](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)

   If you don't see the left pane, click **Expand left pane** to open the pane.
3. In the **Explorer** pane, expand your project, click **Datasets**, and
   then click your dataset.

4. Click **Overview \> Models** and click the model name that you're exporting.

5. Click **More \> Export**:

   ![Export model](https://docs.cloud.google.com/static/bigquery/images/export-model.png)
6. In the **Export model to Google Cloud Storage** dialog:

   - For **Select GCS location** , browse for the bucket or folder location where you want to export the model, and click **Select**.
   - Click **Submit** to export the model.

To check on the progress of the job, in the **Explorer** pane, click
**Job history** , and look for an **EXTRACT** type job.

### SQL

The `EXPORT MODEL` statement lets you export BigQuery ML models
to [Cloud Storage](https://docs.cloud.google.com/storage/docs) using [GoogleSQL](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql)
query syntax.

To export a BigQuery ML model in the Google Cloud console by
using the `EXPORT MODEL` statement, follow these steps:

1. In the Google Cloud console, open the BigQuery page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. Click **Compose new query**.

3. In the **Query editor** field, type your [`EXPORT MODEL`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-export-model)
   statement.

   The following query exports a model named `myproject.mydataset.mymodel`
   to a Cloud Storage bucket with [URI](https://docs.cloud.google.com/bigquery/docs/batch-loading-data#gcs-uri)
   `gs://bucket/path/to/saved_model/`.

   <br />

   ```googlesql
    EXPORT MODEL `myproject.mydataset.mymodel`
    OPTIONS(URI = 'gs://bucket/path/to/saved_model/')
    
   ```

   <br />

4. Click **Run** . When the query is complete, the following appears in the
   **Query results** pane: `Successfully exported model`.

### bq

> [!NOTE]
> **Note:** To export a model using the bq command-line tool, you must have bq tool version 2.0.56 or later, which is included with gcloud CLI [version 287.0.0](https://docs.cloud.google.com/sdk/docs/release-notes#28700_2020-04-01) and later. To see your installed bq tool version, use [`bq version`](https://docs.cloud.google.com/bigquery/docs/bq-command-line-tool#getting_help) and, if needed, update the gcloud CLI using [`gcloud components update`](https://docs.cloud.google.com/sdk/gcloud/reference/components/update).

Use the `bq extract` command with the `--model` flag.

(Optional) Supply the `--destination_format` flag and pick the format of the
model exported.
(Optional) Supply the `--location` flag and set the value to
your [location](https://docs.cloud.google.com/bigquery/docs/locations).

```
bq --location=location extract \
--destination_format format \
--model project_id:dataset.model \
gs://bucket/model_folder
```

Where:

- <var translate="no">location</var> is the name of your location. The `--location` flag is optional. For example, if you are using BigQuery in the Tokyo region, you can set the flag's value to `asia-northeast1`. You can set a default value for the location using the [.bigqueryrc file](https://docs.cloud.google.com/bigquery/docs/bq-command-line-tool#setting_default_values_for_command-line_flags).
- <var translate="no">destination_format</var> is the format for the exported model: `ML_TF_SAVED_MODEL` (default), or `ML_XGBOOST_BOOSTER`.
- <var translate="no">project_id</var> is your project ID.
- <var translate="no">dataset</var> is the name of the source dataset.
- <var translate="no">model</var> is the model you're exporting.
- <var translate="no">bucket</var> is the name of the Cloud Storage bucket to which you're exporting the data. The BigQuery dataset and the Cloud Storage bucket must be in the same [location](https://docs.cloud.google.com/bigquery/docs/locations).
- <var translate="no">model_folder</var> is the name of the folder where the exported model files will be written.

Examples:

For example, the following command exports `mydataset.mymodel` in TensorFlow SavedModel
format to a Cloud Storage bucket named `mymodel_folder`.

```bash
bq extract --model \
'mydataset.mymodel' \
gs://example-bucket/mymodel_folder
```

The default value of <var translate="no">destination_format</var> is `ML_TF_SAVED_MODEL`.

The following command exports `mydataset.mymodel` in XGBoost Booster format
to a Cloud Storage bucket named `mymodel_folder`.

```bash
bq extract --model \
--destination_format ML_XGBOOST_BOOSTER \
'mydataset.mytable' \
gs://example-bucket/mymodel_folder
```

### API

To export model, create an `extract` job and populate the job configuration.

(Optional) Specify your location in the `location` property in the
`jobReference` section of the
[job resource](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs).

1. Create an extract job that points to the BigQuery ML model and
   the Cloud Storage destination.

2. Specify the source model by using the `sourceModel` configuration object
   that contains the project ID, dataset ID, and model ID.

3. The `destination URI(s)` property must be fully-qualified, in the format
   gs://<var translate="no">bucket</var>/<var translate="no">model_folder</var>.

4. Specify the destination format by setting the
   `configuration.extract.destinationFormat` property. For example, to
   export a boosted tree model, set this property to the value
   `ML_XGBOOST_BOOSTER`.

5. To check the job status, call
   [jobs.get(<var translate="no">job_id</var>)](https://docs.cloud.google.com/bigquery/docs/reference/v2/jobs/get) with
   the ID of the job returned by the initial request.

   - If `status.state = DONE`, the job completed successfully.
   - If the `status.errorResult` property is present, the request failed, and that object will include information describing what went wrong.
   - If `status.errorResult` is absent, the job finished successfully, although there might have been some non-fatal errors. Non-fatal errors are listed in the returned job object's `status.errors` property.

**API notes:**

- As a best practice, generate a unique ID and pass it as
  `jobReference.jobId` when calling `jobs.insert` to create a job. This
  approach is more robust to network failure because the client can poll
  or retry on the known job ID.

- Calling `jobs.insert` on a given job ID is idempotent; in other words,
  you can retry as many times as you like on the same job ID, and at most
  one of those operations will succeed.

### Java


Before trying this sample, follow the Java setup instructions in the
[BigQuery quickstart using
client libraries](https://docs.cloud.google.com/bigquery/docs/quickstarts/quickstart-client-libraries).


For more information, see the
[BigQuery Java API
reference documentation](https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/overview).


To authenticate to BigQuery, set up Application Default Credentials.
For more information, see

[Set up authentication for client libraries](https://docs.cloud.google.com/bigquery/docs/authentication#client-libs).

    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryException.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryOptions.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.ExtractJobConfiguration.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Job.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.JobInfo.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.ModelId.html;

    // Sample to extract model to GCS bucket
    public class ExtractModel {

      public static void main(String[] args) throws InterruptedException {
        // TODO(developer): Replace these variables before running the sample.
        String projectName = "bigquery-public-data";
        String datasetName = "samples";
        String modelName = "model";
        String bucketName = "MY-BUCKET-NAME";
        String destinationUri = "gs://" + bucketName + "/path/to/file";
        extractModel(projectName, datasetName, modelName, destinationUri);
      }

      public static void extractModel(
          String projectName, String datasetName, String modelName, String destinationUri)
          throws InterruptedException {
        try {
          // Initialize client that will be used to send requests. This client only needs to be created
          // once, and can be reused for multiple requests.
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html bigquery = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryOptions.html.getDefaultInstance().getService();

          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.ModelId.html modelId = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.ModelId.html.of(projectName, datasetName, modelName);

          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.ExtractJobConfiguration.html extractConfig =
              https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.ExtractJobConfiguration.html.newBuilder(modelId, destinationUri).build();

          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Job.html job = bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html#com_google_cloud_bigquery_BigQuery_create_com_google_cloud_bigquery_DatasetInfo_com_google_cloud_bigquery_BigQuery_DatasetOption____(JobInfo.of(extractConfig));

          // Blocks until this job completes its execution, either failing or succeeding.
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Job.html completedJob = job.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Job.html#com_google_cloud_bigquery_Job_waitFor_com_google_cloud_bigquery_BigQueryRetryConfig_com_google_cloud_RetryOption____();
          if (completedJob == null) {
            System.out.println("Job not executed since it no longer exists.");
            return;
          } else if (completedJob.getStatus().getError() != null) {
            System.out.println(
                "BigQuery was unable to extract due to an error: \n" + job.getStatus().getError());
            return;
          }
          System.out.println("Model extract successful");
        } catch (https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryException.html ex) {
          System.out.println("Model extraction job was interrupted. \n" + ex.toString());
        }
      }
    }

## Model deployment

You can deploy the exported model to Vertex AI as well as locally. If the
model's [`TRANSFORM` clause](https://docs.cloud.google.com/bigquery/docs/bigqueryml-transform) contains Date
functions, Datetime functions, Time functions or Timestamp functions, you must
use [bigquery-ml-utils library](https://pypi.org/project/bigquery-ml-utils/)
in the container. The exception is if you are [deploying through Model Registry](https://docs.cloud.google.com/bigquery/docs/managing-models-vertex),
which does not need exported models or serving containers.

### Vertex AI deployment

| Export model format | Deployment |
|---|---|
| TensorFlow SavedModel (non-AutoML models) | [Deploy a TensorFlow SavedModel](https://docs.cloud.google.com/vertex-ai/docs/general/deployment). You must create the SavedModel file using a [supported version](https://docs.cloud.google.com/vertex-ai/docs/supported-frameworks-list#tensorflow) of TensorFlow. |
| TensorFlow SavedModel (AutoML models) | Not supported. |
| XGBoost Booster | Use a [custom prediction routine](https://docs.cloud.google.com/vertex-ai/docs/predictions/custom-prediction-routines). For XGBoost Booster models, preprocessing and postprocessing information is saved in the exported files, and a custom prediction routine lets you deploy the model with the extra exported files. <br /> You must create the model files using a [supported version](https://docs.cloud.google.com/vertex-ai/docs/supported-frameworks-list#xgboost_2) of XGBoost. |

### Local deployment

| Export model format | Deployment |
|---|---|
| TensorFlow SavedModel (non-AutoML models) | SavedModel is a standard format, and you can deploy them in [TensorFlow Serving docker container](https://www.tensorflow.org/tfx/serving/serving_basic). You can also leverage the [local run](https://docs.cloud.google.com/vertex-ai/docs/training/containerize-run-code-local) of Vertex AI online prediction. |
| TensorFlow SavedModel (AutoML models) | [Containerize and run the model](https://docs.cloud.google.com/vertex-ai/docs/training/containerize-run-code-local). |
| XGBoost Booster | To run XGBoost Booster models locally, you can use the exported `main.py` file: 1. Download all of the files from Cloud Storage to the local directory. 2. Unzip the `predictor.py` file from `xgboost_predictor-0.1.tar.gz` to the local directory. 3. Run `main.py` (see instructions in `main.py`). |

## Prediction output format

This section provides the prediction output format of the exported models for
each model type. All exported models support batch prediction; they can handle
multiple input rows at a time. For example, there are two input rows in each of
the following output format examples.

### AUTOENCODER

| Prediction output format | Output sample |
|---|---|
| ``` +---+---+---+ |      LATENT_COL_1      |      LATENT_COL_2      |           ...          | +---+---+---+ |       [FLOAT]          |         [FLOAT]        |           ...          | +---+---+---+ ``` | ``` +---+---+---+---+ |   LATENT_COL_1   |   LATENT_COL_2   |   LATENT_COL_3   |   LATENT_COL_4   | +---+---+---+---+ |    0.21384512    |    0.93457112    |    0.64978097    |    0.00480489    | +---+---+---+---+ ``` |

### AUTOML_CLASSIFIER

| Prediction output format | Output sample |
|---|---|
| ``` +---+ | predictions                              | +---+ | [{"scores":[FLOAT], "classes":[STRING]}] | +---+ ``` | ``` +---+ | predictions                                 | +---+ | [{"scores":[1, 2], "classes":['a', 'b']},   | |  {"scores":[3, 0.2], "classes":['a', 'b']}] | +---+ ``` |

### AUTOML_REGRESSOR

| Prediction output format | Output sample |
|---|---|
| ``` +---+ | predictions     | +---+ | [FLOAT]         | +---+ ``` | ``` +---+ | predictions     | +---+ | [1.8, 2.46]     | +---+ ``` |

### BOOSTED_TREE_CLASSIFIER and RANDOM_FOREST_CLASSIFIER

| Prediction output format | Output sample |
|---|---|
| ``` +---+---+---+ | LABEL_PROBS | LABEL_VALUES | PREDICTED_LABEL | +---+---+---+ | [FLOAT]     | [STRING]     | STRING          | +---+---+---+ ``` | ``` +---+---+---+ | LABEL_PROBS | LABEL_VALUES | PREDICTED_LABEL | +---+---+---+ | [0.1, 0.9]  | ['a', 'b']   | ['b']           | +---+---+---+ | [0.8, 0.2]  | ['a', 'b']   | ['a']           | +---+---+---+ ``` |

### BOOSTED_TREE_REGRESSOR AND RANDOM_FOREST_REGRESSOR

| Prediction output format | Output sample |
|---|---|
| ``` +---+ | predicted_label | +---+ | FLOAT           | +---+ ``` | ``` +---+ | predicted_label | +---+ | [1.8]           | +---+ | [2.46]          | +---+ ``` |

### DNN_CLASSIFIER

| Prediction output format | Output sample |
|---|---|
| ``` +---+---+---+---+---+---+---+ | ALL_CLASS_IDS | ALL_CLASSES | CLASS_IDS | CLASSES | LOGISTIC (binary only) | LOGITS | PROBABILITIES | +---+---+---+---+---+---+---+ | [INT64]       | [STRING]    | INT64     | STRING  | FLOAT                  | [FLOAT]| [FLOAT]       | +---+---+---+---+---+---+---+ ``` | ``` +---+---+---+---+---+---+---+ | ALL_CLASS_IDS | ALL_CLASSES | CLASS_IDS | CLASSES | LOGISTIC (binary only) | LOGITS | PROBABILITIES | +---+---+---+---+---+---+---+ | [0, 1]        | ['a', 'b']  | [0]       | ['a']   | [0.36]                 | [-0.53]| [0.64, 0.36]  | +---+---+---+---+---+---+---+ | [0, 1]        | ['a', 'b']  | [0]       | ['a']   | [0.2]                  | [-1.38]| [0.8, 0.2]    | +---+---+---+---+---+---+---+ ``` |

### DNN_REGRESSOR

| Prediction output format | Output sample |
|---|---|
| ``` +---+ | PREDICTED_LABEL | +---+ | FLOAT           | +---+ ``` | ``` +---+ | PREDICTED_LABEL | +---+ | [1.8]           | +---+ | [2.46]          | +---+ ``` |

### DNN_LINEAR_COMBINED_CLASSIFIER

| Prediction output format | Output sample |
|---|---|
| ``` +---+---+---+---+---+---+---+ | ALL_CLASS_IDS | ALL_CLASSES | CLASS_IDS | CLASSES | LOGISTIC (binary only) | LOGITS | PROBABILITIES | +---+---+---+---+---+---+---+ | [INT64]       | [STRING]    | INT64     | STRING  | FLOAT                  | [FLOAT]| [FLOAT]       | +---+---+---+---+---+---+---+ ``` | ``` +---+---+---+---+---+---+---+ | ALL_CLASS_IDS | ALL_CLASSES | CLASS_IDS | CLASSES | LOGISTIC (binary only) | LOGITS | PROBABILITIES | +---+---+---+---+---+---+---+ | [0, 1]        | ['a', 'b']  | [0]       | ['a']   | [0.36]                 | [-0.53]| [0.64, 0.36]  | +---+---+---+---+---+---+---+ | [0, 1]        | ['a', 'b']  | [0]       | ['a']   | [0.2]                  | [-1.38]| [0.8, 0.2]    | +---+---+---+---+---+---+---+ ``` |

### DNN_LINEAR_COMBINED_REGRESSOR

| Prediction output format | Output sample |
|---|---|
| ``` +---+ | PREDICTED_LABEL | +---+ | FLOAT           | +---+ ``` | ``` +---+ | PREDICTED_LABEL | +---+ | [1.8]           | +---+ | [2.46]          | +---+ ``` |

### KMEANS

| Prediction output format | Output sample |
|---|---|
| ``` +---+---+---+ | CENTROID_DISTANCES | CENTROID_IDS | NEAREST_CENTROID_ID | +---+---+---+ | [FLOAT]            | [INT64]      | INT64               | +---+---+---+ ``` | ``` +---+---+---+ | CENTROID_DISTANCES | CENTROID_IDS | NEAREST_CENTROID_ID | +---+---+---+ | [1.2, 1.3]         | [1, 2]       | [1]                 | +---+---+---+ | [0.4, 0.1]         | [1, 2]       | [2]                 | +---+---+---+ ``` |

### LINEAR_REG

| Prediction output format | Output sample |
|---|---|
| ``` +---+ | PREDICTED_LABEL | +---+ | FLOAT           | +---+ ``` | ``` +---+ | PREDICTED_LABEL | +---+ | [1.8]           | +---+ | [2.46]          | +---+ ``` |

### LOGISTIC_REG

| Prediction output format | Output sample |
|---|---|
| ``` +---+---+---+ | LABEL_PROBS | LABEL_VALUES | PREDICTED_LABEL | +---+---+---+ | [FLOAT]     | [STRING]     | STRING          | +---+---+---+ ``` | ``` +---+---+---+ | LABEL_PROBS | LABEL_VALUES | PREDICTED_LABEL | +---+---+---+ | [0.1, 0.9]  | ['a', 'b']   | ['b']           | +---+---+---+ | [0.8, 0.2]  | ['a', 'b']   | ['a']           | +---+---+---+ ``` |

### MATRIX_FACTORIZATION

**Note:** We only support taking an input user and output top 50 (predicted_rating, predicted_item) pairs sorted by predicted_rating in descending order.

| Prediction output format | Output sample |
|---|---|
| ``` +---+---+ | PREDICTED_RATING | PREDICTED_ITEM | +---+---+ | [FLOAT]          | [STRING]       | +---+---+ ``` | ``` +---+---+ | PREDICTED_RATING | PREDICTED_ITEM | +---+---+ | [5.5, 1.7]       | ['A', 'B']     | +---+---+ | [7.2, 2.7]       | ['B', 'A']     | +---+---+ ``` |

### TENSORFLOW (imported)

| Prediction output format |
|---|
| Same as the imported model |

### PCA

| Prediction output format | Output sample |
|---|---|
| ``` +---+---+ | PRINCIPAL_COMPONENT_IDS | PRINCIPAL_COMPONENT_PROJECTIONS | +---+---+ |       [INT64]           |             [FLOAT]             | +---+---+ ``` | ``` +---+---+ | PRINCIPAL_COMPONENT_IDS | PRINCIPAL_COMPONENT_PROJECTIONS | +---+---+ |       [1, 2]            |             [1.2, 5.0]          | +---+---+ ``` |

### TRANSFORM_ONLY

| Prediction output format |
|---|
| Same as the columns specified in the model's `TRANSFORM` clause |

## XGBoost model visualization

You can visualize the boosted trees using the
[plot_tree](https://xgboost.readthedocs.io/en/latest/python/python_api.html#xgboost.plot_tree)
Python API after model export. For example, you can leverage
[Colab](https://colab.research.google.com/)
without installing the dependencies:

1. Export the boosted tree model to a Cloud Storage bucket.
2. Download the `model.bst` file from the Cloud Storage bucket.
3. In a [Colab notebook](https://colab.sandbox.google.com/notebooks/welcome.ipynb), upload the `model.bst` file to `Files`.
4. Run the following code in the notebook:

       import xgboost as xgb
       import matplotlib.pyplot as plt

       model = xgb.Booster(model_file="model.bst")
       num_iterations = <iteration_number>
       for tree_num in range(num_iterations):
         xgb.plot_tree(model, num_trees=tree_num)
       plt.show

This example plots multiple trees (one tree per iteration):

![Export model](https://docs.cloud.google.com/static/bigquery/images/boosted-tree-colab.png)

> [!NOTE]
> **Note:** We use the label encoder to encode categorical features, so you can get the corresponding category for a split value from the vocabulary file in the 'assets/' directory inside the model export Cloud Storage bucket. For example, when you see "f0 \< 2.95" in a node, you can find the corresponding category in the vocabulary file by looking for the 3rd item.

We don't save feature names in the model, so you will see names
such as "f0", "f1", and so on. You can find the corresponding feature names in
the `assets/model_metadata.json` exported file using these names (such as "f0")
as indexes.

## Required permissions

To export a BigQuery ML model to Cloud Storage, you need permissions to
access the BigQuery ML model, permissions to run an extract job, and
permissions to write the data to the Cloud Storage bucket.

**BigQuery permissions**

- At a minimum, to export model, you must be granted `bigquery.models.export`
  permissions. The following predefined Identity and Access Management (IAM) roles are granted
  `bigquery.models.export` permissions:

  - `bigquery.dataViewer`
  - `bigquery.dataOwner`
  - `bigquery.dataEditor`
  - `bigquery.admin`
- At a minimum, to run an export [job](https://docs.cloud.google.com/bigquery/docs/managing-jobs), you must
  be granted `bigquery.jobs.create` permissions. The following predefined
  IAM roles are granted `bigquery.jobs.create` permissions:

  - `bigquery.user`
  - `bigquery.jobUser`
  - `bigquery.admin`

**Cloud Storage permissions**

- To write the data to an existing Cloud Storage bucket, you must be
  granted `storage.objects.create` permissions. The following predefined
  IAM roles are granted `storage.objects.create` permissions:

  - `storage.objectCreator`
  - `storage.objectAdmin`
  - `storage.admin`

For more information on IAM roles and permissions in BigQuery ML, see
[Access control](https://docs.cloud.google.com/bigquery/docs/access-control).

## Move BigQuery data between locations

You cannot change the location of a dataset after it is created, but you can
[make a copy of the dataset](https://docs.cloud.google.com/bigquery/docs/copying-datasets).

## Quota policy

For information on extract job quotas, see
[Extract jobs](https://docs.cloud.google.com/bigquery/quotas#export_jobs) on the Quotas and limits page.

## Pricing

There is no charge for exporting BigQuery ML models, but
exports are subject to BigQuery's
[Quotas and limits](https://docs.cloud.google.com/bigquery/quotas). For more information on BigQuery
pricing, see the [Pricing](https://cloud.google.com/bigquery/pricing) page.

After the data is exported, you are charged for storing the data in
Cloud Storage. For more information on Cloud Storage pricing, see the
Cloud Storage [Pricing](https://cloud.google.com/storage/pricing) page.

## What's next

- Walk through the [Export a BigQuery ML model for online prediction](https://docs.cloud.google.com/bigquery/docs/export-model-tutorial) tutorial.