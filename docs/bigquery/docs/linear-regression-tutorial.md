In this tutorial, you use a
[linear regression model](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-glm)
in BigQuery ML to predict the weight of a penguin based on the
penguin's demographic information. A linear regression is a type of regression
model that generates a continuous value from a linear combination of input
features.

This tutorial uses the
[`bigquery-public-data.ml_datasets.penguins`](https://console.cloud.google.com/bigquery?p=bigquery-public-data&d=ml_datasets&t=penguins&page=table)
dataset.

## Objectives

In this tutorial, you will perform the following tasks:

- Create a linear regression model.
- Evaluate the model.
- Make predictions by using the model.

> [!NOTE]
> **Note:** This tutorial covers linear regression in pure SQL. For a linear regression tutorial using Python and BigQuery DataFrames on the same dataset, see [Create a regression model with BigQuery DataFrames](https://docs.cloud.google.com/bigquery/docs/samples/bigquery-dataframes-regression-model).

## Costs

This tutorial uses billable components of Google Cloud,
including the following:

- BigQuery
- BigQuery ML

For more information on BigQuery costs, see the
[BigQuery pricing](https://cloud.google.com/bigquery/pricing) page.

For more information on BigQuery ML costs, see
[BigQuery ML pricing](https://cloud.google.com/bigquery/pricing#bqml).

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


   Enable the BigQuery API.


   **Roles required to enable APIs**


   To enable APIs, you need the Service Usage Admin IAM
   role (`roles/serviceusage.serviceUsageAdmin`), which
   contains the `serviceusage.services.enable` permission. [Learn how to grant
   roles](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).

   [Enable the API](https://console.cloud.google.com/flows/enableapi?apiid=bigquery.googleapis.com)

## Required permissions

To create the model using BigQuery ML, you need the following
IAM permissions:

- `bigquery.jobs.create`
- `bigquery.models.create`
- `bigquery.models.getData`
- `bigquery.models.updateData`
- `bigquery.models.updateMetadata`

To run inference, you need the following permissions:

- `bigquery.models.getData` on the model
- `bigquery.jobs.create`

## Create a dataset

Create a BigQuery dataset to store your ML model.

<br />

### Console

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to the BigQuery page](https://console.cloud.google.com/bigquery)
2. In the **Explorer** pane, click your project name.

3. Click **View actions \> Create dataset**

4. On the **Create dataset** page, do the following:

   - For **Dataset ID** , enter `bqml_tutorial`.

   - For **Location type** , select **Multi-region** , and then select
     **US**.

   - Leave the remaining default settings as they are, and click
     **Create dataset**.

### bq

To create a new dataset, use the
[`bq mk --dataset` command](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#mk-dataset).

1. Create a dataset named `bqml_tutorial` with the data location set to `US`.

   ```
   bq mk --dataset \
     --location=US \
     --description "BigQuery ML tutorial dataset." \
     bqml_tutorial
   ```
2. Confirm that the dataset was created:

   ```bash
   bq ls
   ```

### API

Call the [`datasets.insert`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets/insert)
method with a defined [dataset resource](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets).

<br />

```json
{
  "datasetReference": {
     "datasetId": "bqml_tutorial"
  }
}
```

<br />

### BigQuery DataFrames

<br />

Before trying this sample, follow the BigQuery DataFrames
setup instructions in the [BigQuery quickstart
using BigQuery DataFrames](https://docs.cloud.google.com/bigquery/docs/dataframes-quickstart).
For more information, see the
[BigQuery DataFrames reference documentation](https://docs.cloud.google.com/python/docs/reference/bigframes/latest).

To authenticate to BigQuery, set up Application Default Credentials.
For more information, see [Set
up ADC for a local development environment](https://docs.cloud.google.com/docs/authentication/set-up-adc-local-dev-environment).


    import google.cloud.bigquery

    bqclient = google.cloud.https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html()
    bqclient.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_create_dataset("bqml_tutorial", exists_ok=True)

## Create the model

Create a linear regression model using the Analytics sample
dataset for BigQuery.

### SQL

You can create a linear regression model by using the
[`CREATE MODEL` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-glm)
and specifying `LINEAR_REG` for the model type. Creating the model includes
training the model.

The following are useful things to know about the `CREATE MODEL` statement:

- The [`input_label_cols`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-glm#input_label_cols) option specifies which column in the `SELECT` statement to use as the label column. Here, the label column is `body_mass_g`. For linear regression models, the label column must be real-valued, that is, the column values must be real numbers.
- This query's `SELECT` statement uses the following columns in the
  `bigquery-public-data.ml_datasets.penguins` table to predict a penguin's
  weight:

  - `species`: the species of penguin.
  - `island`: the island that the penguin resides on.
  - `culmen_length_mm`: the length of the penguin's culmen in millimeters.
  - `culmen_depth_mm`: the depth of the penguin's culmen in millimeters.
  - `flipper_length_mm`: the length of the penguin's flippers in millimeters.
  - `sex`: the sex of the penguin.
- The `WHERE` clause in this query's `SELECT` statement, `WHERE body_mass_g IS
  NOT NULL`, excludes rows where the `body_mass_g` column is `NULL`.

Run the query that creates your linear regression model:

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, run the following query:

   ```googlesql
   CREATE OR REPLACE MODEL `bqml_tutorial.penguins_model`
   OPTIONS
     (model_type='linear_reg',
     input_label_cols=['body_mass_g']) AS
   SELECT
     *
   FROM
     `bigquery-public-data.ml_datasets.penguins`
   WHERE
     body_mass_g IS NOT NULL;
   ```
3. It takes about 30 seconds to create the `penguins_model` model.

   To see the model, follow these steps:
   1. In the left pane, click **Explorer**:

      ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)

      If you don't see the left pane, click **Expand left pane** to open the pane.
   2. In the **Explorer** pane, expand your project and click **Datasets**.

   3. Click the `bqml_tutorial` dataset.

   4. Click the **Models** tab.

### BigQuery DataFrames

Before trying this sample, follow the BigQuery DataFrames
setup instructions in the [BigQuery quickstart
using BigQuery DataFrames](https://docs.cloud.google.com/bigquery/docs/dataframes-quickstart).
For more information, see the
[BigQuery DataFrames reference documentation](https://docs.cloud.google.com/python/docs/reference/bigframes/latest).

To authenticate to BigQuery, set up Application Default Credentials.
For more information, see [Set
up ADC for a local development environment](https://docs.cloud.google.com/docs/authentication/set-up-adc-local-dev-environment).

    from bigframes.ml.linear_model import LinearRegression
    import bigframes.pandas as bpd

    # Load data from BigQuery
    bq_df = bpd.read_gbq("bigquery-public-data.ml_datasets.penguins")

    # Drop rows with nulls to get training data
    training_data = bq_df.dropna(subset=["body_mass_g"])

    # Specify your feature (or input) columns and the label (or output) column:
    feature_columns = training_data.drop(columns=["body_mass_g"])
    label_columns = training_data[["body_mass_g"]]

    # Create the linear model
    model = LinearRegression()
    model.fit(feature_columns, label_columns)
    model.to_gbq(
        your_model_id,  # For example: "bqml_tutorial.penguins_model"
        replace=True,
    )

It takes about 30 seconds to create the model. To see the model, follow these
steps:

1. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)
2. In the **Explorer** pane, expand your project and click **Datasets**.

3. Click the `bqml_tutorial` dataset.

4. Click the **Models** tab.

## Get training statistics

To see the results of the model training, you can use the
[`ML.TRAINING_INFO` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-train),
or you can view the statistics in the Google Cloud console. In this
tutorial, you use the Google Cloud console.

A machine learning algorithm builds a model by examining many examples and
attempting to find a model that minimizes loss. This process is called empirical
risk minimization.

Loss is the penalty for a bad prediction. It is a number indicating
how bad the model's prediction was on a single example. If the model's
prediction is perfect, the loss is zero; otherwise, the loss is greater. The
goal of training a model is to find a set of weights and biases that have low
loss, on average, across all examples.

See the model training statistics that were generated when you ran the
`CREATE MODEL` query:

1. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)
2. In the **Explorer** pane, expand your project and click **Datasets**.

3. Click the `bqml_tutorial` dataset.

4. Click the **Models** tab.

5. To open the model information pane, click **penguins_model**.

6. Click the **Training** tab, and then click **Table**. The results should look
   similar to the following:

   ![ML.TRAINING_INFO output](https://docs.cloud.google.com/static/bigquery/images/training-stats-penguins-insta-model.png)

   The **Training Data Loss** column represents the loss metric calculated
   after the model is trained on the training dataset. Since you performed a
   linear regression, this column shows the
   [mean squared error](https://developers.google.com/machine-learning/glossary/#MSE)
   value. A
   [normal_equation](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-glm#optimize_strategy)
   optimization strategy is automatically used for this training, so only one
   iteration is required to converge to the final model. For more information
   on setting the model optimization strategy, see
   [`optimize_strategy`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-glm#optimize_strategy).

## Evaluate the model

After creating the model, evaluate the model's performance by using the
[`ML.EVALUATE` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-evaluate)
or the
[`score` BigQuery DataFrames function](https://dataframes.bigquery.dev/reference/api/bigframes.ml.linear_model.LinearRegression.html#bigframes.ml.linear_model.LinearRegression.score)
to evaluate the predicted values generated by the model against the actual data.

### SQL

For input, the `ML.EVALUATE` function takes the trained model and a dataset
that matches the schema of the data that you used to train the model. In
a production environment, you should
evaluate the model on different data than the data you used to train the model.
If you run `ML.EVALUATE` without providing input data, the function retrieves
the evaluation metrics calculated during training. These metrics are calculated
by using the automatically reserved evaluation dataset:

<br />

        SELECT
          *
        FROM
          ML.EVALUATE(MODEL bqml_tutorial.penguins_model);
        
<br />

Run the `ML.EVALUATE` query:

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, run the following query:

   ```googlesql
     SELECT
       *
     FROM
       ML.EVALUATE(MODEL `bqml_tutorial.penguins_model`,
         (
         SELECT
           *
         FROM
           `bigquery-public-data.ml_datasets.penguins`
         WHERE
           body_mass_g IS NOT NULL));
     
   ```

### BigQuery DataFrames

Before trying this sample, follow the BigQuery DataFrames
setup instructions in the [BigQuery quickstart
using BigQuery DataFrames](https://docs.cloud.google.com/bigquery/docs/dataframes-quickstart).
For more information, see the
[BigQuery DataFrames reference documentation](https://docs.cloud.google.com/python/docs/reference/bigframes/latest).

To authenticate to BigQuery, set up Application Default Credentials.
For more information, see [Set
up ADC for a local development environment](https://docs.cloud.google.com/docs/authentication/set-up-adc-local-dev-environment).

    import bigframes.pandas as bpd

    # Select the model you will be evaluating. `read_gbq_model` loads model data from
    # BigQuery, but you could also use the `model` object from the previous steps.
    model = bpd.read_gbq_model(
        your_model_id,  # For example: "bqml_tutorial.penguins_model"
    )

    # Score the model with input data defined in an earlier step to compare
    # model predictions on feature_columns to true labels in label_columns.
    score = model.score(feature_columns, label_columns)
    # Expected output results:
    # index  mean_absolute_error  mean_squared_error  mean_squared_log_error  median_absolute_error  r2_score  explained_variance
    #   0        227.012237         81838.159892            0.00507                173.080816        0.872377    0.872377
    #   1 rows x 6 columns

The results should look similar to the following:

![ML.EVALUATE output](https://docs.cloud.google.com/static/bigquery/images/evaluate-penguins-insta-model.png)

Because you performed a linear regression, the results include the following
columns:

- `mean_absolute_error`
- `mean_squared_error`
- `mean_squared_log_error`
- `median_absolute_error`
- `r2_score`
- `explained_variance`

An important metric in the evaluation results is the
[R^2^ score](https://en.wikipedia.org/wiki/Coefficient_of_determination).
The R^2^ score is a statistical measure that determines if the linear
regression predictions approximate the actual data. A value of `0` indicates
that the model explains none of the variability of the response data around the
mean. A value of `1` indicates that the model explains all the variability of
the response data around the mean.

You can also look at the model's information pane in the Google Cloud console
to view the evaluation metrics:

![ML.EVALUATE output](https://docs.cloud.google.com/static/bigquery/images/eval-metrics-penguins-insta-model.png)

## Use the model to predict outcomes

Now that you have evaluated your model, the next step is to use it to predict
an outcome. You can run the
[`ML.PREDICT` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-predict)
or the
[`predict` BigQuery DataFrames function](https://dataframes.bigquery.dev/reference/api/bigframes.ml.linear_model.LinearRegression.html#bigframes.ml.linear_model.LinearRegression.predict)
on the model to predict the body mass in grams of all penguins that reside on
the Biscoe Islands.

### SQL

For input, the `ML.PREDICT` function takes the trained model and a dataset that
matches the schema of the data that you used to train the model, excluding the
label column.

Run the `ML.PREDICT` query:

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, run the following query:

   ```googlesql
   SELECT
   *
   FROM
   ML.PREDICT(MODEL `bqml_tutorial.penguins_model`,
     (
     SELECT
       *
     FROM
       `bigquery-public-data.ml_datasets.penguins`
     WHERE island = 'Biscoe'));
   ```

### BigQuery DataFrames

Before trying this sample, follow the BigQuery DataFrames
setup instructions in the [BigQuery quickstart
using BigQuery DataFrames](https://docs.cloud.google.com/bigquery/docs/dataframes-quickstart).
For more information, see the
[BigQuery DataFrames reference documentation](https://docs.cloud.google.com/python/docs/reference/bigframes/latest).

To authenticate to BigQuery, set up Application Default Credentials.
For more information, see [Set
up ADC for a local development environment](https://docs.cloud.google.com/docs/authentication/set-up-adc-local-dev-environment).

    # Select the model you'll use for predictions. `read_gbq_model` loads
    # model data from BigQuery, but you could also use the `model` object
    # object from previous steps.
    model = bpd.read_gbq_model(
        your_model_id,
        # For example: "bqml_tutorial.penguins_model",
    )

    # Load data from BigQuery
    bq_df = bpd.read_gbq("bigquery-public-data.ml_datasets.penguins")

    # Use 'contains' function to filter by island containing the string
    # "Biscoe".
    biscoe_data = bq_df.loc[bq_df["island"].str.contains("Biscoe")]

    result = model.predict(biscoe_data)

    # Expected output results:
    #     predicted_body_mass_g  	      species	                island	 culmen_length_mm  culmen_depth_mm   body_mass_g 	flipper_length_mm	sex
    # 23	  4681.782896	   Gentoo penguin (Pygoscelis papua)	Biscoe	      <NA>	            <NA>	        <NA>	          <NA>	        <NA>
    # 332	  4740.7907	       Gentoo penguin (Pygoscelis papua)	Biscoe	      46.2	            14.4	        214.0	          4650.0	    <NA>
    # 160	  4731.310452	   Gentoo penguin (Pygoscelis papua)	Biscoe	      44.5	            14.3	        216.0	          4100.0	    <NA>

The results should look similar to the following:

![ML.PREDICT output](https://docs.cloud.google.com/static/bigquery/images/predict-penguins-insta-model.png)

## Explain the prediction results

### SQL


To understand why the model is generating these prediction results, you can use
the
[`ML.EXPLAIN_PREDICT` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-explain-predict).

`ML.EXPLAIN_PREDICT` is an extended version of the `ML.PREDICT` function.
`ML.EXPLAIN_PREDICT` not only outputs prediction results, but also outputs
additional columns to explain the prediction results. In practice, you can run
`ML.EXPLAIN_PREDICT` instead of `ML.PREDICT`. For more information, see
[BigQuery ML explainable AI overview](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-xai-overview#explainable_ai_offerings_in_bigquery_ml).

Run the `ML.EXPLAIN_PREDICT` query:

1. In the Google Cloud console, go to the **BigQuery** page.

[Go to BigQuery](https://console.cloud.google.com/bigquery)

1. In the query editor, run the following query:

```googlesql
SELECT
  *
FROM
  ML.EXPLAIN_PREDICT(MODEL `bqml_tutorial.penguins_model`,
    (
    SELECT
      *
    FROM
      `bigquery-public-data.ml_datasets.penguins`
    WHERE island = 'Biscoe'),
    STRUCT(3 as top_k_features));
```

1. The results should look similar to the following:

   ![ML.EXPLAIN_PREDICT output](https://docs.cloud.google.com/static/bigquery/images/explain-penguin.png)

> [!NOTE]
> **Note:** The `ML.EXPLAIN_PREDICT` query outputs all the input feature columns, similar to what `ML.PREDICT` does. For readability purposes, only one feature column, `species`, is shown in the preceding figure.

### BigQuery DataFrames


Before trying this sample, follow the BigQuery DataFrames
setup instructions in the [BigQuery quickstart
using BigQuery DataFrames](https://docs.cloud.google.com/bigquery/docs/dataframes-quickstart).
For more information, see the
[BigQuery DataFrames reference documentation](https://docs.cloud.google.com/python/docs/reference/bigframes/latest).

To authenticate to BigQuery, set up Application Default Credentials.
For more information, see [Set
up ADC for a local development environment](https://docs.cloud.google.com/docs/authentication/set-up-adc-local-dev-environment).

<br />

    # Use 'predict_explain' function to understand why the model is generating these prediction results.
    # 'predict_explain'is an extended version of the 'predict' function that not only outputs prediction results, but also outputs additional columns to explain the prediction results.
    # Using the trained model and utilizing data specific to Biscoe Island, explain the predictions of the top 3 features
    explained = model.predict_explain(biscoe_data, top_k_features=3)

    # Expected results:
    #   predicted_body_mass_g               top_feature_attributions	        baseline_prediction_value	prediction_value	approximation_error	              species	            island	culmen_length_mm	culmen_depth_mm	flipper_length_mm	body_mass_g	    sex
    # 0	 5413.510134	        [{'feature': 'island', 'attribution': 7348.877...	-5320.222128	          5413.510134	            0.0	         Gentoo penguin (Pygoscelis papua)	Biscoe	    45.2	              16.4	        223.0	           5950.0	    MALE
    # 1	 4768.351092            [{'feature': 'island', 'attribution': 7348.877...	-5320.222128	          4768.351092	            0.0	         Gentoo penguin (Pygoscelis papua)	Biscoe	    46.5	              14.5	        213.0	           4400.0	   FEMALE
    # 2	 3235.896372	        [{'feature': 'island', 'attribution': 7348.877...	-5320.222128	          3235.896372	            0.0	        Adelie Penguin (Pygoscelis adeliae)	Biscoe	    37.7	              16.0          183.0	           3075.0	   FEMALE
    # 3	 5349.603734	        [{'feature': 'island', 'attribution': 7348.877...	-5320.222128	          5349.603734	            0.0	         Gentoo penguin (Pygoscelis papua)	Biscoe	    46.4	              15.6	        221.0	           5000.0	    MALE
    # 4	 4637.165037	        [{'feature': 'island', 'attribution': 7348.877...	-5320.222128	          4637.165037	            0.0	         Gentoo penguin (Pygoscelis papua)	Biscoe	    46.1	              13.2	        211.0	           4500.0	   FEMALE

For linear regression models, Shapley values are used to generate feature
attribution values for each feature in the model. The output includes
the top three feature attributions per row of the `penguins` table because
`top_k_features` was set to `3`. These attributions are sorted by
the absolute value of the attribution in descending order. In all examples, the
feature `sex` contributed the most to the overall prediction.

## Globally explain the model

### SQL

To know which features are generally the most important to determine penguin
weight, you can use the
[`ML.GLOBAL_EXPLAIN` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-global-explain).
In order to use `ML.GLOBAL_EXPLAIN`, you must retrain the model with the
`ENABLE_GLOBAL_EXPLAIN` option set to `TRUE`.

Retrain and get global explanations for the model:

1. In the Google Cloud console, go to the **BigQuery** page.

[Go to BigQuery](https://console.cloud.google.com/bigquery)

1. In the query editor, run the following query to retrain the model:

   ```googlesql
   #standardSQL
   CREATE OR REPLACE MODEL `bqml_tutorial.penguins_model`
   OPTIONS (
     model_type = 'linear_reg',
     input_label_cols = ['body_mass_g'],
     enable_global_explain = TRUE)
   AS
   SELECT
   *
   FROM
   `bigquery-public-data.ml_datasets.penguins`
   WHERE
   body_mass_g IS NOT NULL;
   ```
2. In the query editor, run the following query to get global explanations:

   ```googlesql
   SELECT
   *
   FROM
   ML.GLOBAL_EXPLAIN(MODEL `bqml_tutorial.penguins_model`)
   ```
3. The results should look similar to the following:

   ![ML.GLOBAL_EXPLAIN output](https://docs.cloud.google.com/static/bigquery/images/global-explain-penguin.png)

### BigQuery DataFrames


Before trying this sample, follow the BigQuery DataFrames
setup instructions in the [BigQuery quickstart
using BigQuery DataFrames](https://docs.cloud.google.com/bigquery/docs/dataframes-quickstart).
For more information, see the
[BigQuery DataFrames reference documentation](https://docs.cloud.google.com/python/docs/reference/bigframes/latest).

To authenticate to BigQuery, set up Application Default Credentials.
For more information, see [Set
up ADC for a local development environment](https://docs.cloud.google.com/docs/authentication/set-up-adc-local-dev-environment).

<br />

    # To use the `global_explain()` function, the model must be recreated with `enable_global_explain` set to `True`.
    model = LinearRegression(enable_global_explain=True)

    # The model must the be fitted before it can be saved to BigQuery and then explained.
    training_data = bq_df.dropna(subset=["body_mass_g"])
    X = training_data.drop(columns=["body_mass_g"])
    y = training_data[["body_mass_g"]]
    model.fit(X, y)
    model.to_gbq("bqml_tutorial.penguins_model", replace=True)

    # Explain the model
    explain_model = model.global_explain()

    # Expected results:
    #                       attribution
    # feature
    # island	            5737.315921
    # species	            4073.280549
    # sex	                622.070896
    # flipper_length_mm	    193.612051
    # culmen_depth_mm	    117.084944
    # culmen_length_mm	    94.366793

## Clean up


To avoid incurring charges to your Google Cloud account for the resources used in this
tutorial, either delete the project that contains the resources, or keep the project and
delete the individual resources.

- You can delete the project you created.
- Or you can keep the project and delete the dataset.

### Delete your dataset

Deleting your project removes all datasets and all tables in the project. If you
prefer to reuse the project, you can delete the dataset you created in this
tutorial:

1. If necessary, open the BigQuery page in the
   Google Cloud console.

   [Go to the BigQuery page](https://console.cloud.google.com/bigquery)
2. In the navigation, click the **bqml_tutorial** dataset you created.

3. Click **Delete dataset** on the right side of the window.
   This action deletes the dataset, the table, and all the data.

4. In the **Delete dataset** dialog box, confirm the delete command by typing
   the name of your dataset (`bqml_tutorial`) and then click **Delete**.

### Delete your project

To delete the project:

> [!CAUTION]
> **Caution** : Deleting a project has the following effects:
>
> - **Everything in the project is deleted.** If you used an existing project for the tasks in this document, when you delete it, you also delete any other work you've done in the project.
> - **Custom project IDs are lost.** When you created this project, you might have created a custom project ID that you want to use in the future. To preserve the URLs that use the project ID, such as an `appspot.com` URL, delete selected resources inside the project instead of deleting the whole project.
>
>
> If you plan to explore multiple architectures, tutorials, or quickstarts, reusing projects
> can help you avoid exceeding project quota limits.

1. In the Google Cloud console, go to the **Manage resources** page.

   [Go to Manage resources](https://console.cloud.google.com/iam-admin/projects)
2. In the project list, select the project that you want to delete, and then click **Delete**.
3. In the dialog, type the project ID, and then click **Shut down** to delete the project.

<br />

## What's next

- For an overview of BigQuery ML, see [Introduction to BigQuery ML](https://docs.cloud.google.com/bigquery/docs/bqml-introduction).
- For information on creating models, see the [`CREATE MODEL`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create) syntax page.