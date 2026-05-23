# Create an ML model in BigQuery ML by using the Google Cloud console

This document shows you how to use the Google Cloud console to create a
BigQuery ML model.

## Required roles

- To create a model and run inference, you must be granted the following roles:

  - BigQuery Data Editor (`roles/bigquery.dataEditor`)
  - BigQuery User (`roles/bigquery.user`)

## Before you begin

1. In the Google Cloud console, on the project selector page,
   select or create a Google Cloud project.

   **Roles required to select or create a project**
   - **Select a project**: Selecting a project doesn't require a specific IAM role---you can select any project that you've been granted a role on.
   - **Create a project** : To create a project, you need the Project Creator role (`roles/resourcemanager.projectCreator`), which contains the `resourcemanager.projects.create` permission. [Learn how to grant
     roles](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).

   > [!NOTE]
   > **Note**: If you don't plan to keep the resources that you create in this procedure, create a project instead of selecting an existing project. After you finish these steps, you can delete the project, removing all resources associated with the project.

   [Go to project selector](https://console.cloud.google.com/projectselector2/home/dashboard)
2.
   [Verify that billing is enabled for your Google Cloud project](https://docs.cloud.google.com/billing/docs/how-to/verify-billing-enabled#confirm_billing_is_enabled_on_a_project).

3.


   Enable the BigQuery and BigQuery Connection APIs.


   **Roles required to enable APIs**


   To enable APIs, you need the Service Usage Admin IAM
   role (`roles/serviceusage.serviceUsageAdmin`), which
   contains the `serviceusage.services.enable` permission. [Learn how to grant
   roles](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).

   [Enable the APIs](https://console.cloud.google.com/flows/enableapi?apiid=bigquery.googleapis.com,bigqueryconnection.googleapis.com)

## Model-specific prerequisites

Before you create a model, make sure that you have addressed any
prerequisites for the type of model that you are creating:

- If you want to use a query to select training data for a model, you must have
  that query available as a
  [saved query](https://docs.cloud.google.com/bigquery/docs/saved-queries-introduction).

- Matrix factorization models require reservations. For more information,
  see [Pricing](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-matrix-factorization#pricing).

- The following remote models require a
  [Cloud resource connection](https://docs.cloud.google.com/bigquery/docs/create-cloud-resource-connection):

  - [Remote models over Vertex AI and partner models](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model)
  - [Remote models over open models](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model-open)
  - [Remote models over Cloud AI services](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model-service)
  - [Remote models over custom models in Vertex AI](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model-https)

  The connection's service account must also be granted certain roles,
  depending on the type of remote model.
- To import a model, you must have that model uploaded to a Cloud Storage
  bucket.

## Create a dataset

Create a BigQuery dataset to contain your resources:

### Console

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)

   If you don't see the left pane, click **Expand left pane** to open the pane.
3. In the **Explorer** pane, click your project name.

4. Click **View actions \> Create dataset**.

5. On the **Create dataset** page, do the following:

   1. For **Dataset ID**, type a name for the dataset.

   2. For **Location type** , select **Region** or **Multi-region**.

      - If you selected **Region** , then select a location from the **Region** list.
      - If you selected **Multi-region** , then select **US** or **Europe** from the **Multi-region** list.
   3. Click **Create dataset**.

### bq

1. To create a new dataset, use the
   [`bq mk`](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#mk-dataset) command
   with the `--location` flag:

   ```
   bq --location=LOCATION mk -d DATASET_ID
   ```

   Replace the following:
   - `LOCATION`: the dataset's [location](https://docs.cloud.google.com/bigquery/docs/locations).
   - `DATASET_ID` is the ID of the dataset that you're creating.
2. Confirm that the dataset was created:

   ```bash
   bq ls
   ```

## Create an internally or externally trained model

Use this procedure to create the following types of models:

- Time series models:

  - [`ARIMA_PLUS`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-time-series)
  - [`ARIMA_PLUS_XREG`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-multivariate-time-series)
- Contribution analysis: [Contribution analysis](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-contribution-analysis)

- Classification:

  - [Logistic regression](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-glm)
  - [Boosted tree classification](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-boosted-tree)
  - [Random forest classification](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-boosted-tree)
  - [Deep Neural network (DNN) classification](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-boosted-tree)
  - [Wide-and-deep classification](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-boosted-tree)
  - [AutoML classification](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-boosted-tree)
- Regression:

  - [Linear regression](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-glm)
  - [Boosted tree regression](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-boosted-tree)
  - [Random forest regression](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-boosted-tree)
  - [Deep Neural network (DNN) regression](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-boosted-tree)
  - [Wide-and-deep regression](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-boosted-tree)
  - [AutoML regression](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-boosted-tree)
- Clustering: [K-means](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-kmeans)

- Recommendation: [Matrix factorization](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-matrix-factorization)

- Dimensionality reduction:

  - [Principal component analysis (PCA)](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-pca)
  - [Autoencoder](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-autoencoder)

These models have different sets of options according to their type. While
BigQuery ML automatic tuning works well in most cases, you can
choose to manually tune your model as part of the procedure. If you want to
do so, refer to the documentation for the given type of model to learn more
about the model options.

To create a model:

1. Go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)

   If you don't see the left pane, click **Expand left pane** to open the pane.
3. In the **Explorer** pane, click **Datasets**, and then click the dataset that
   you created.

4. Click
   **View actions** next to the dataset, and then click
   **Create BQML Model**.

   The **Create new model** pane opens.
5. For **Model name**, type a name for the model.

6. If you want to create a saved query that contains the
   `CREATE MODEL` statement for the model, select **Save Query** .

   1. For **Query name**, type a name for the saved query.
   2. For **Region**, choose a region for the saved query.
7. Click **Continue**.

8. In the **Creation method** section, select **Train a Model in BigQuery**.

9. In the **Modeling objective** section, select a modeling objective
   for the model.

10. Click **Continue**.

11. On the **Model options** page, select a model type. The type of model you can
    select varies based on the modeling objective you chose.

12. In the **Training data** section, do one of the following:

    - Select **Table/View** to get training data from a table or view, and then select the project, dataset, and view or table name.
    - Select **Query** to get training data from a saved query, and then select the saved query.
13. In **Selected input label columns**, choose the columns from the
    table, view, or query that you want to use as input to the model.

14. If there is a **Required options** section, specify the
    requested column information:

    - For classification and regression models, for **INPUT_LABEL_COLS**, select the column that contains the label data.
    - For matrix factorization models, select the following:

      - For **RATING_COL**, select the column that contains the rating data.
      - For **USER_COL**, select the column that contains the user data.
      - For **ITEM_COL**, select the column that contains the item data.
    - For time series forecasting models, select the following:

      - For **TIME_SERIES_TIMESTAMP_COL**, select the column that contains the time points to use when training the model.
      - For **TIME_SERIES_DATA_COL**, select the column that contains the data to forecast.
15. Optional: In the **Optional** section, specify values
    for additional model tuning arguments. The arguments that are available vary
    based on the type of model that you are creating.

16. Optional: If there's a **Hyperparameter tuning** section, you can specify
    the **NUM_TRIALS** option to enable
    \[hyperparameter tuning\](/bigquery/docs/hyperparameter-tuning-tutorial
    for your model. The arguments that are available for hyperparameter tuning
    vary based on the type of model that you're creating.

17. Click **Create model**.

18. When model creation is complete, click **Go to model** to view
    model details.

## Create a remote model over a pre-trained model

Use this procedure to create the following types of remote models:

- [Models over Vertex AI or partner models](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model)
- [Models over open models](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model-open)

To create a model:

1. Go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)
3. In the **Explorer** pane, click **Datasets**, and then click the dataset that
   you created.

4. Click
   **View actions** next to the dataset, and then click
   **Create BQML Model**.

   The **Create new model** pane opens.
5. For **Model name**, type a name for the model.

6. If you want to create a saved query that contains the
   `CREATE MODEL` statement for the model, select **Save Query** .

   1. For **Query name**, type a name for the saved query.
   2. For **Region**, choose a region for the saved query.
7. Click **Continue**.

8. In the **Creation method** section, select
   **Connect to Vertex AI LLM service and Cloud AI services**.

9. On the **Model options** page, select **Google and Partner Models** or
   **Open Models** for the model type, as appropriate for your use case.

10. In the **Remote connection** section, do one of the following:

    - If you have a [default connection](https://docs.cloud.google.com/bigquery/docs/default-connections) configured, or if you have both the BigQuery Admin and the Project IAM Admin roles, select **Default connection**.
    - If you don't have a default connection configured, or if you lack
      the appropriate roles, select **Cloud resource connection**.

      1. For **Project**, select the project that contains the connection that you want to use.
      2. For **Location**, select the location used by the connection.
      3. For **Connection** , select the connection to use for the
         remote model, or select **Create new connection** to
         create a new connection.

         > [!IMPORTANT]
         > **Important:** If you create a new connection, you must grant appropriate roles to the connection's service account before continuing. For more information about what roles to grant, see the reference documentation for the type of remote model that you are creating.

11. In the **Required options** section, do one of the following:

    - For remote models over Google models and partner models, specify the endpoint to use. This is the name of the model, for example `gemini-2.0-flash`. For more information about supported models, see [`ENDPOINT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model#endpoint).
    - For remote models over open models, copy and paste in the endpoint to use. This is the shared public endpoint of a model deployed to Vertex AI, in the format `https://location-aiplatform.googleapis.com/v1/projects/project/locations/location/endpoints/endpoint_id`. For more information, see [`ENDPOINT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model-open#endpoint).
12. Click **Create model**.

13. When model creation is complete, click **Go to model** to view
    model details.

## Create a remote model over a custom model

Use this procedure to create remote models over [custom models deployed to
Vertex AI](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model-https).

To create a model:

1. Go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)
3. In the **Explorer** pane, click **Datasets**, and then click the dataset that
   you created.

4. Click
   **View actions** next to the dataset, and then click
   **Create BQML Model**.

   The **Create new model** pane opens.
5. For **Model name**, type a name for the model.

6. If you want to create a saved query that contains the
   `CREATE MODEL` statement for the model, select **Save Query** .

   1. For **Query name**, type a name for the saved query.
   2. For **Region**, choose a region for the saved query.
7. Click **Continue**.

8. In the **Creation method** section, select
   **Connect to user managed Vertex AI endpoints**.

9. In the **Remote connection** section of the **Model options** page, do one
   of the following:

   - If you have a [default connection](https://docs.cloud.google.com/bigquery/docs/default-connections) configured, or if you have both the BigQuery Admin and the Project IAM Admin roles, select **Default connection**.
   - If you don't have a default connection configured, or if you lack
     the appropriate roles, select **Cloud resource connection**.

     1. For **Project**, select the project that contains the connection that you want to use.
     2. For **Location**, select the location used by the connection.
     3. For **Connection** , select the connection to use for the
        remote model, or select **Create new connection** to
        create a new connection.

        > [!IMPORTANT]
        > **Important:** If you create a new connection, you must grant appropriate roles to the connection's service account before continuing. For more information about what roles to grant, see the reference documentation for the type of remote model that you are creating.

10. In the **Required options** section, specify the endpoint to use. This is the
    shared public endpoint
    of a model deployed to Vertex AI, in the format
    `https://location-aiplatform.googleapis.com/v1/projects/project/locations/location/endpoints/endpoint_id`.
    For more information, see
    [`ENDPOINT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model-https#endpoint).

11. Click **Create model**.

12. When model creation is complete, click **Go to model** to view
    model details.

## Create a remote model over a Cloud AI service

Use this procedure to create remote models over
[Cloud AI services](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model-service).

To create a model:

1. Go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)
3. In the **Explorer** pane, click **Datasets**, and then click the dataset that
   you created.

4. Click
   **View actions** next to the dataset, and then click
   **Create BQML Model**.

   The **Create new model** pane opens.
5. For **Model name**, type a name for the model.

6. If you want to create a saved query that contains the
   `CREATE MODEL` statement for the model, select **Save Query** .

   1. For **Query name**, type a name for the saved query.
   2. For **Region**, choose a region for the saved query.
7. Click **Continue**.

8. In the **Creation method** section, select
   **Connect to Vertex AI LLM service and Cloud AI services**.

9. On the **Model options** page, select **Cloud AI Services**.

10. In the **Remote connection** section, do one of the following:

    - If you have a [default connection](https://docs.cloud.google.com/bigquery/docs/default-connections) configured, or if you have both the BigQuery Admin and the Project IAM Admin roles, select **Default connection**.
    - If you don't have a default connection configured, or if you lack
      the appropriate roles, select **Cloud resource connection**.

      1. For **Project**, select the project that contains the connection that you want to use.
      2. For **Location**, select the location used by the connection.
      3. For **Connection** , select the connection to use for the
         remote model, or select **Create new connection** to
         create a new connection.

         > [!IMPORTANT]
         > **Important:** If you create a new connection, you must grant appropriate roles to the connection's service account before continuing. For more information about what roles to grant, see the reference documentation for the type of remote model that you are creating.

11. In the **Required options** section, select the Cloud AI service type to use.

12. In the **Optional** section, specify
    [document processor](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model-service#document_processor)
    information if you are using the `CLOUD_AI_DOCUMENT_V1`
    service. Optionally, you can specify
    [speech recognizer](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model-service#speech_recognizer)
    information if you are using the `CLOUD_AI_SPEECH_TO_TEXT_V2`
    service.

13. Click **Create model**.

14. When model creation is complete, click **Go to model** to view
    model details.

## Create an imported model

Use this procedure to create BigQuery ML models by importing the
following types of models:

- [ONNX](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-onnx)
- [TensorFlow](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-tensorflow)
- [TensorFlow Lite](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-tflite)
- [XGBoost](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-xgboost)

To create a model:

1. Go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)
3. In the **Explorer** pane, click **Datasets**, and then click the dataset that
   you created.

4. Click
   **View actions** next to the dataset, and then click
   **Create BQML Model**.

   The **Create new model** pane opens.
5. For **Model name**, type a name for the model.

6. If you want to create a saved query that contains the
   `CREATE MODEL` statement for the model, select **Save Query** .

   1. For **Query name**, type a name for the saved query.
   2. For **Region**, choose a region for the saved query.
7. Click **Continue**.

8. In the **Creation method** section, select **Import model**.

9. On the **Model options** page, select the type of model that you want to
   import.

10. For **GCS path**, browse for or paste in the URI for the
    Cloud Storage bucket that contains the model.

11. Click **Create model**.

12. When model creation is complete, click **Go to model** to view
    model details.