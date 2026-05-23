# Use ML and AI with BigQuery DataFrames

BigQuery DataFrames provides ML and AI capabilities for
BigQuery DataFrames using the `bigframes.ml` library.

You can [preprocess data](https://docs.cloud.google.com/bigquery/docs/dataframes-ml-ai#preprocess-data), [create estimators to train
models](https://docs.cloud.google.com/bigquery/docs/dataframes-ml-ai#train-models) in BigQuery DataFrames, [create ML
pipelines](https://docs.cloud.google.com/bigquery/docs/dataframes-ml-ai#create-pipelines), and [split training and testing
datasets](https://docs.cloud.google.com/bigquery/docs/dataframes-ml-ai#select-models).

## Required roles


To get the permissions that
you need to complete the tasks in this document,

ask your administrator to grant you the
following IAM roles on your project:

- Use remote models or AI functionalities: [BigQuery Connection Admin](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquery#bigquery.connectionAdmin) (`roles/bigquery.connectionAdmin`)
- Use BigQuery DataFrames in a BigQuery notebook:
  - [BigQuery User](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquery#bigquery.user) (`roles/bigquery.user`)
  - [Notebook Runtime User](https://docs.cloud.google.com/iam/docs/roles-permissions/aiplatform#aiplatform.notebookRuntimeUser) (`roles/aiplatform.notebookRuntimeUser`)
  - [Code Creator](https://docs.cloud.google.com/iam/docs/roles-permissions/dataform#dataform.codeCreator) (`roles/dataform.codeCreator`)
- Use default BigQuery connection:
  - [BigQuery Data Editor](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquery#bigquery.dataEditor) (`roles/bigquery.dataEditor`)
  - [BigQuery Connection Admin](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquery#bigquery.connectionAdmin) (`roles/bigquery.connectionAdmin`)
  - [Cloud Functions Developer](https://docs.cloud.google.com/iam/docs/roles-permissions/cloudfunctions#cloudfunctions.developer) (`roles/cloudfunctions.developer`)
  - [Service Account User](https://docs.cloud.google.com/iam/docs/roles-permissions/iam#iam.serviceAccountUser) (`roles/iam.serviceAccountUser`)
  - [Storage Object Viewer](https://docs.cloud.google.com/iam/docs/roles-permissions/storage#storage.objectViewer) (`roles/storage.objectViewer`)
- Use BigQuery DataFrames ML remote models: [BigQuery Connection Admin](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquery#bigquery.connectionAdmin) (`roles/bigquery.connectionAdmin`)


For more information about granting roles, see [Manage access to projects, folders, and organizations](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).


You might also be able to get
the required permissions through [custom
roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles) or other [predefined
roles](https://docs.cloud.google.com/iam/docs/roles-overview#predefined).

## ML locations

The `bigframes.ml` library supports the same locations as
BigQuery ML. BigQuery ML model prediction and other
ML functions are supported in all BigQuery regions. Support for
model training varies by region. For more information, see
[BigQuery ML locations](https://docs.cloud.google.com/bigquery/docs/locations#bqml-loc).

## Preprocess data

Create transformers to prepare data for use in estimators (models) by
using the
[`bigframes.ml.preprocessing` module](https://dataframes.bigquery.dev/reference/api/bigframes.ml.preprocessing.html)
and the
[`bigframes.ml.compose` module](https://dataframes.bigquery.dev/reference/api/bigframes.ml.compose.html).
BigQuery DataFrames offers the following transformations:

- To bin continuous data into intervals, use the
  [`KBinsDiscretizer` class](https://dataframes.bigquery.dev/reference/api/bigframes.ml.preprocessing.KBinsDiscretizer.html#bigframes.ml.preprocessing.KBinsDiscretizer)
  in the `bigframes.ml.preprocessing` module.

- To normalize the target labels as integer values, use the
  [`LabelEncoder` class](https://dataframes.bigquery.dev/reference/api/bigframes.ml.preprocessing.LabelEncoder.html)
  in the `bigframes.ml.preprocessing` module.

- To scale each feature to the range `[-1, 1]` by its maximum absolute value,
  use the
  [`MaxAbsScaler` class](https://dataframes.bigquery.dev/reference/api/bigframes.ml.preprocessing.MaxAbsScaler.html)
  in the `bigframes.ml.preprocessing` module.

- To standardize features by scaling each feature to the range `[0, 1]`,
  use the
  [`MinMaxScaler` class](https://dataframes.bigquery.dev/reference/api/bigframes.ml.preprocessing.MinMaxScaler.html)
  in the `bigframes.ml.preprocessing` module.

- To standardize features by removing the mean and scaling to unit variance,
  use the
  [`StandardScaler` class](https://dataframes.bigquery.dev/reference/api/bigframes.ml.preprocessing.StandardScaler.html)
  in the `bigframes.ml.preprocessing` module.

- To transform categorical values into numeric format, use the
  [`OneHotEncoder` class](https://dataframes.bigquery.dev/reference/api/bigframes.ml.preprocessing.OneHotEncoder.html)
  in the `bigframes.ml.preprocessing` module.

- To apply transformers to DataFrames columns, use the
  [`ColumnTransformer` class](https://dataframes.bigquery.dev/reference/api/bigframes.ml.compose.ColumnTransformer.html#bigframes.ml.compose.ColumnTransformer)
  in the `bigframes.ml.compose` module.

## Train models

You can create estimators to train models in BigQuery DataFrames.

### Clustering models

You can create estimators for clustering models by using the
[`bigframes.ml.cluster` module](https://dataframes.bigquery.dev/reference/api/bigframes.ml.cluster.html).
To create K-means clustering models, use the
[`KMeans` class](https://dataframes.bigquery.dev/reference/api/bigframes.ml.cluster.KMeans.html#bigframes.ml.cluster.KMeans). Use these models for data
segmentation. For example, identifying customer segments. K-means is
an unsupervised learning technique, so model training doesn't
require labels or split data for training or evaluation.

You can use the `bigframes.ml.cluster` module to create estimators for
clustering models.

The following code sample shows using the `bigframes.ml.cluster KMeans`
class to create a k-means clustering model for data segmentation:

    from bigframes.ml.cluster import KMeans
    import bigframes.pandas as bpd

    # Load data from BigQuery
    query_or_table = "bigquery-public-data.ml_datasets.penguins"
    bq_df = bpd.read_gbq(query_or_table)

    # Create the KMeans model
    cluster_model = KMeans(n_clusters=10)
    cluster_model.fit(bq_df["culmen_length_mm"], bq_df["sex"])

    # Predict using the model
    result = cluster_model.predict(bq_df)
    # Score the model
    score = cluster_model.score(bq_df)

### Decomposition models

You can create estimators for decomposition models by using the
[`bigframes.ml.decomposition` module](https://dataframes.bigquery.dev/reference/api/bigframes.ml.decomposition.html).
To create principal component analysis (PCA) models, use the [`PCA`
class](https://dataframes.bigquery.dev/reference/api/bigframes.ml.decomposition.PCA.html#bigframes.ml.decomposition.PCA). Use these
models for computing principal components and using them to perform
a change of basis on the data. Using the `PCA` class provides dimensionality
reduction by projecting each data point onto only the first few
principal components to obtain lower-dimensional data while
preserving as much of the data's variation as possible.

### Ensemble models

You can create estimators for ensemble models by using the
[`bigframes.ml.ensemble` module](https://dataframes.bigquery.dev/reference/api/bigframes.ml.ensemble.html#module-bigframes.ml.ensemble).

- To create random forest classifier models, use the
  [`RandomForestClassifier` class](https://dataframes.bigquery.dev/reference/api/bigframes.ml.ensemble.RandomForestClassifier.html#bigframes.ml.ensemble.RandomForestClassifier). Use these models for
  constructing multiple learning method decision trees for
  classification.

- To create random forest regression models, use the
  [`RandomForestRegressor` class](https://dataframes.bigquery.dev/reference/api/bigframes.ml.ensemble.RandomForestRegressor.html). Use these models for
  constructing multiple learning method decision trees for regression.

- To create gradient boosted tree classifier models, use the
  [`XGBClassifier` class](https://dataframes.bigquery.dev/reference/api/bigframes.ml.ensemble.XGBClassifier.html). Use these models
  for additively constructing multiple learning method decision trees
  for classification.

- To create gradient boosted tree regression models, use the
  [`XGBRegressor` class](https://dataframes.bigquery.dev/reference/api/bigframes.ml.ensemble.XGBRegressor.html). Use these models
  for additively constructing multiple learning method decision trees
  for regression.

### Forecasting models

You can create estimators for forecasting models by using the
[`bigframes.ml.forecasting` module](https://dataframes.bigquery.dev/reference/api/bigframes.ml.forecasting.html#module-bigframes.ml.forecasting).
To create time series forecasting models, use the
[`ARIMAPlus` class](https://dataframes.bigquery.dev/reference/api/bigframes.ml.forecasting.ARIMAPlus.html).

### Imported models

You can create estimators for imported models by using the
[`bigframes.ml.imported` module](https://dataframes.bigquery.dev/reference/api/bigframes.ml.imported.html#module-bigframes.ml.imported).

- To import Open Neural Network Exchange (ONNX) models, use the
  [`ONNXModel` class](https://dataframes.bigquery.dev/reference/api/bigframes.ml.imported.ONNXModel.html).

- To import TensorFlow model, use the
  [`TensorFlowModel` class](https://dataframes.bigquery.dev/reference/api/bigframes.ml.imported.TensorFlowModel.html).

- To import XGBoostModel models, use the
  [`XGBoostModel` class](https://dataframes.bigquery.dev/reference/api/bigframes.ml.imported.XGBoostModel.html).

### Linear models

Create estimators for linear models by using the
[`bigframes.ml.linear_model` module](https://dataframes.bigquery.dev/reference/api/bigframes.ml.linear_model.html#module-bigframes.ml.linear_model).

- To create linear regression models, use the
  [`LinearRegression` class](https://dataframes.bigquery.dev/reference/api/bigframes.ml.linear_model.LinearRegression.html). Use these models for
  forecasting, such as forecasting the sales of an item on a
  given day.

- To create logistic regression models, use the
  [`LogisticRegression` class](https://dataframes.bigquery.dev/reference/api/bigframes.ml.linear_model.LogisticRegression.html). Use these models for the
  classification of two or more possible values such as whether an
  input is `low-value`, `medium-value`, or `high-value`.

The following code sample shows using `bigframes.ml` to do the
following:

- Load data from BigQuery.
- Clean and prepare training data.
- Create and apply a [bigframes.ml.LinearRegression](https://dataframes.bigquery.dev/reference/api/bigframes.ml.linear_model.LinearRegression.html#bigframes.ml.linear_model.LinearRegression) regression model.

    from bigframes.ml.linear_model import LinearRegression
    import bigframes.pandas as bpd

    # Load data from BigQuery
    query_or_table = "bigquery-public-data.ml_datasets.penguins"
    bq_df = bpd.read_gbq(query_or_table)

    # Filter down to the data to the Adelie Penguin species
    adelie_data = bq_df[bq_df.species == "Adelie Penguin (Pygoscelis adeliae)"]

    # Drop the species column
    adelie_data = adelie_data.drop(columns=["species"])

    # Drop rows with nulls to get training data
    training_data = adelie_data.dropna()

    # Specify your feature (or input) columns and the label (or output) column:
    feature_columns = training_data[
        ["island", "culmen_length_mm", "culmen_depth_mm", "flipper_length_mm", "sex"]
    ]
    label_columns = training_data[["body_mass_g"]]

    test_data = adelie_data[adelie_data.body_mass_g.isnull()]

    # Create the linear model
    model = LinearRegression()
    model.fit(feature_columns, label_columns)

    # Score the model
    score = model.score(feature_columns, label_columns)

    # Predict using the model
    result = model.predict(test_data)

### Large language models

You can create estimators for LLMs by using the
[`bigframes.ml.llm` module](https://dataframes.bigquery.dev/reference/api/bigframes.ml.llm.html).

- To create Gemini text generator models, use the
  [`GeminiTextGenerator` class](https://dataframes.bigquery.dev/reference/api/bigframes.ml.llm.GeminiTextGenerator.html). Use these models for text
  generation tasks.

- To create estimators for remote large language models (LLMs), use the
  [`bigframes.ml.llm`](https://dataframes.bigquery.dev/reference/api/bigframes.ml.llm.html)
  module.

The following code sample shows using the `bigframes.ml.llm`
[`GeminiTextGenerator`](https://dataframes.bigquery.dev/reference/api/bigframes.ml.llm.GeminiTextGenerator.html#bigframes.ml.llm.GeminiTextGenerator)
class to create a Gemini model for code generation:

    from bigframes.ml.llm import GeminiTextGenerator
    import bigframes.pandas as bpd

    # Create the Gemini LLM model
    session = bpd.get_global_session()
    connection = f"{PROJECT_ID}.{REGION}.{CONN_NAME}"
    model = GeminiTextGenerator(
        session=session, connection_name=connection, model_name="gemini-2.0-flash-001"
    )

    df_api = bpd.read_csv("gs://cloud-samples-data/vertex-ai/bigframe/df.csv")

    # Prepare the prompts and send them to the LLM model for prediction
    df_prompt_prefix = "Generate Pandas sample code for DataFrame."
    df_prompt = df_prompt_prefix + df_api["API"]

    # Predict using the model
    df_pred = model.predict(df_prompt.to_frame(), max_output_tokens=1024)

### Remote models

To use BigQuery DataFrames ML remote models (`bigframes.ml.remote`
or `bigframes.ml.llm`), you must enable the following APIs:

- [BigQuery API (`bigquery.googleapis.com`)](https://docs.cloud.google.com/bigquery/docs/reference/rest)

- [BigQuery Connection API (`bigqueryconnection.googleapis.com`)](https://docs.cloud.google.com/bigquery/docs/reference/bigqueryconnection/rest)

- [Vertex AI API (`aiplatform.googleapis.com`)](https://docs.cloud.google.com/vertex-ai/docs/reference/rest)

- [Cloud Resource Manager API (`cloudresourcemanager.googleapis.com`)](https://docs.cloud.google.com/resource-manager/reference/rest)

When you use BigQuery DataFrames ML remote models, you need the
[Project IAM Admin role](https://docs.cloud.google.com/iam/docs/roles-permissions/resourcemanager#resourcemanager.projectIamAdmin) (`roles/resourcemanager.projectIamAdmin`)
if you use a default BigQuery connection, or the
[Browser role](https://docs.cloud.google.com/iam/docs/roles-permissions/browser#browser) (`roles/browser`)
if you use a pre-configured connection. You can avoid this requirement by
setting the `bigframes.pandas.options.bigquery.skip_bq_connection_check` option
to `True`, in which case the connection (default or pre-configured) is used
as-is without any existence or permission check. If you use the
pre-configured connection and skip the connection check, verify the
following:

- The connection is created in the right location.
- If you use BigQuery DataFrames ML remote models, the service account has the [Vertex AI User role](https://docs.cloud.google.com/iam/docs/roles-permissions/aiplatform#aiplatform.user) (`roles/aiplatform.user`) on the project.

Creating a remote model in BigQuery DataFrames creates a
[BigQuery connection](https://docs.cloud.google.com/bigquery/docs/create-cloud-resource-connection).
By default, a connection of the name `bigframes-default-connection` is used. You
can use a pre-configured BigQuery connection if you prefer,
in which case the connection creation is skipped. The service account
for the default connection is granted the
[Vertex AI User role](https://docs.cloud.google.com/iam/docs/roles-permissions/aiplatform#aiplatform.user) (`roles/aiplatform.user`) on the project.

## Create pipelines

You can create ML pipelines by using
[`bigframes.ml.pipeline` module](https://dataframes.bigquery.dev/reference/api/bigframes.ml.pipeline.html).
Pipelines let you assemble several ML steps to be cross-validated together while
setting different parameters. This simplifies your code, and lets you deploy
data preprocessing steps and an estimator together.

To create a pipeline of transforms with a final estimator, use the
[`Pipeline` class](https://dataframes.bigquery.dev/reference/api/bigframes.ml.pipeline.Pipeline.html#bigframes.ml.pipeline.Pipeline).

## Select models

To split your training and testing datasets and select the best models, use the
[`bigframes.ml.model_selection` module](https://dataframes.bigquery.dev/reference/api/bigframes.ml.model_selection.html#module-bigframes.ml.model_selection)
module:

- To split the data into training and testing (evaluation sets), as shown in the
  following code sample, use the
  [`train_test_split` function](https://dataframes.bigquery.dev/reference/api/bigframes.ml.model_selection.train_test_split.html#bigframes.ml.model_selection.train_test_split):

      X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.2)

- To create multi-fold training and testing sets to train and evaluate models,
  as shown in the following code sample, use the [`KFold` class](https://dataframes.bigquery.dev/reference/api/bigframes.ml.model_selection.KFold.html#bigframes.ml.model_selection.KFold)
  and the
  [`KFold.split` method](http://dataframes.bigquery.dev/reference/api/bigframes.ml.model_selection.KFold.split.html). This feature is valuable for small
  datasets.

      kf = KFold(n_splits=5)
      for i, (X_train, X_test, y_train, y_test) in enumerate(kf.split(X, y)):
      # Train and evaluate models with training and testing sets

- To automatically create multi-fold training and testing sets, train and
  evaluate the model, and get the result of each fold, as shown in the following
  code sample, use the
  [`cross_validate` function](https://dataframes.bigquery.dev/reference/api/bigframes.ml.model_selection.cross_validate.html#bigframes.ml.model_selection.cross_validate):

      scores = cross_validate(model, X, y, cv=5)

## What's next

- Learn about the [BigQuery DataFrames data type system](https://docs.cloud.google.com/bigquery/docs/dataframes-data-types).
- Learn how to [generate BigQuery DataFrames code with Gemini](https://docs.cloud.google.com/bigquery/docs/write-sql-gemini#dataframe).
- Learn how to [analyze package downloads from PyPI with BigQuery DataFrames](https://github.com/googleapis/python-bigquery-dataframes/blob/main/notebooks/dataframes/pypi.ipynb).
- View BigQuery DataFrames [source code](https://github.com/googleapis/python-bigquery-dataframes), [sample notebooks](https://github.com/googleapis/python-bigquery-dataframes/tree/main/notebooks), and [samples](https://github.com/googleapis/python-bigquery-dataframes/tree/main/samples/snippets) on GitHub.
- Explore the [BigQuery DataFrames API reference](https://dataframes.bigquery.dev/reference/index.html).