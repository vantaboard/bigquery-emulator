This tutorial teaches you how to use a
[boosted trees classifier model](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-boosted-tree)
to predict the income range of individuals based on their demographic data.
The model predicts whether a value falls into one of two categories, in this
case whether an individual's annual income falls above or below $50,000.

This tutorial uses the
[`bigquery-public-data.ml_datasets.census_adult_income`](https://console.cloud.google.com/bigquery?p=bigquery-public-data&d=ml_datasets&t=census_adult_income&page=table)
dataset. This dataset contains the demographic and income information of US
residents from 2000 and 2010.

## Objectives

This tutorial guides you through completing the following tasks:

- Creating a boosted trees model to predict census respondents' income bracket by using the [`CREATE MODEL` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-boosted-tree).
- Evaluating the model by using the [`ML.EVALUATE` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-evaluate).
- Getting predictions from the model by using the [`ML.PREDICT` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-predict).

## Costs

This tutorial uses billable components of Google Cloud, including the following:

- BigQuery
- BigQuery ML

For more information about BigQuery costs, see the
[BigQuery pricing](https://cloud.google.com/bigquery/pricing) page.

For more information about BigQuery ML costs, see
[BigQuery ML pricing](https://cloud.google.com/bigquery/pricing#bqml).

## Before you begin

1. BigQuery is automatically enabled in new projects. To activate BigQuery in a pre-existing project, go to


   Enable the BigQuery API.


   **Roles required to enable APIs**


   To enable APIs, you need the Service Usage Admin IAM
   role (`roles/serviceusage.serviceUsageAdmin`), which
   contains the `serviceusage.services.enable` permission. [Learn how to grant
   roles](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).

   [Enable the API](https://console.cloud.google.com/flows/enableapi?apiid=bigquery)

## Required Permissions

- To create the dataset, you need the `bigquery.datasets.create`
  IAM permission.

- To create the model, you need the following permissions:

  - `bigquery.jobs.create`
  - `bigquery.models.create`
  - `bigquery.models.getData`
  - `bigquery.models.updateData`
- To run inference, you need the following permissions:

  - `bigquery.models.getData`
  - `bigquery.jobs.create`

For more information about IAM roles and permissions in
BigQuery, see
[Introduction to IAM](https://docs.cloud.google.com/bigquery/docs/access-control).

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

## Prepare the sample data

The model you create in this tutorial predicts the income bracket for census
respondents, based on the following features:

- Age
- Type of work performed
- Marital status
- Level of education
- Occupation
- Hours worked per week

The `education` column isn't included in the training data, because
the `education` and `education_num` columns both express the respondent's level
of education in different formats.

You separate the data into training, evaluation, and prediction sets by creating
a new `dataframe` column that is derived from the `functional_weight` column.
Eighty percent of the data is used for training the model, and the remaining
twenty percent of the data is used for evaluation and prediction.

### SQL

To prepare your sample data, create a [view](https://docs.cloud.google.com/bigquery/docs/views-intro) to
contain the training data. This view is used by the `CREATE MODEL` statement
later in this tutorial.

Run the query that prepares the sample data:

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, run the following query:

   ```googlesql
   CREATE OR REPLACE VIEW
     `bqml_tutorial.input_data` AS
   SELECT
     age,
     workclass,
     marital_status,
     education_num,
     occupation,
     hours_per_week,
     income_bracket,
     CASE
       WHEN MOD(functional_weight, 10) < 8 THEN 'training'
       WHEN MOD(functional_weight, 10) = 8 THEN 'evaluation'
       WHEN MOD(functional_weight, 10) = 9 THEN 'prediction'
     END AS dataframe
   FROM
     `bigquery-public-data.ml_datasets.census_adult_income`;
   ```
3. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)

   If you don't see the left pane, click **Expand left pane** to open the pane.
4. In the **Explorer** pane, search for the `bqml_tutorial` dataset.

5. Click the dataset, and then click **Overview \> Tables**.

6. Click the `input_data` view to open the information pane. The view
   schema appears in the **Schema** tab.

### BigQuery DataFrames

Create a DataFrame called `input_data`. You use `input_data` later in this tutorial to use to train the model, evaluate it, and make predictions.

Before trying this sample, follow the BigQuery DataFrames
setup instructions in the [BigQuery quickstart
using BigQuery DataFrames](https://docs.cloud.google.com/bigquery/docs/dataframes-quickstart).
For more information, see the
[BigQuery DataFrames reference documentation](https://docs.cloud.google.com/python/docs/reference/bigframes/latest).

To authenticate to BigQuery, set up Application Default Credentials.
For more information, see [Set
up ADC for a local development environment](https://docs.cloud.google.com/docs/authentication/set-up-adc-local-dev-environment).

    import bigframes.pandas as bpd

    input_data = bpd.read_gbq(
        "bigquery-public-data.ml_datasets.census_adult_income",
        columns=(
            "age",
            "workclass",
            "marital_status",
            "education_num",
            "occupation",
            "hours_per_week",
            "income_bracket",
            "functional_weight",
        ),
    )
    input_data["dataframe"] = bpd.Series("training", index=input_data.index,).case_when(
        [
            (((input_data["functional_weight"] % 10) == 8), "evaluation"),
            (((input_data["functional_weight"] % 10) == 9), "prediction"),
        ]
    )
    del input_data["functional_weight"]

## Create the boosted trees model

Create a boosted trees model to predict census respondents' income bracket, and
train it on the census data. The query takes about 30 minutes to complete.

### SQL

Follow these steps to create the model:

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, paste in the following query and click **Run**:

   ```googlesql
   CREATE MODEL `bqml_tutorial.tree_model`
   OPTIONS(MODEL_TYPE='BOOSTED_TREE_CLASSIFIER',
           BOOSTER_TYPE = 'GBTREE',
           NUM_PARALLEL_TREE = 1,
           MAX_ITERATIONS = 50,
           TREE_METHOD = 'HIST',
           EARLY_STOP = FALSE,
           SUBSAMPLE = 0.85,
           INPUT_LABEL_COLS = ['income_bracket'])
   AS SELECT * EXCEPT(dataframe)
   FROM `bqml_tutorial.input_data`
   WHERE dataframe = 'training';
   ```

   After the query completes, the
   `tree_model` model can be accessed through the **Explorer** pane. Because
   the query uses a `CREATE MODEL` statement to create a model, you don't see
   query results.

### BigQuery DataFrames

Before trying this sample, follow the BigQuery DataFrames
setup instructions in the [BigQuery quickstart
using BigQuery DataFrames](https://docs.cloud.google.com/bigquery/docs/dataframes-quickstart).
For more information, see the
[BigQuery DataFrames reference documentation](https://docs.cloud.google.com/python/docs/reference/bigframes/latest).

To authenticate to BigQuery, set up Application Default Credentials.
For more information, see [Set
up ADC for a local development environment](https://docs.cloud.google.com/docs/authentication/set-up-adc-local-dev-environment).

    from bigframes.ml import ensemble

    # input_data is defined in an earlier step.
    training_data = input_data[input_data["dataframe"] == "training"]
    X = training_data.drop(columns=["income_bracket", "dataframe"])
    y = training_data["income_bracket"]

    # create and train the model
    tree_model = ensemble.XGBClassifier(
        n_estimators=1,
        booster="gbtree",
        tree_method="hist",
        max_iterations=1,  # For a more accurate model, try 50 iterations.
        subsample=0.85,
    )
    tree_model.fit(X, y)

    tree_model.to_gbq(
        your_model_id,  # For example: "your-project.bqml_tutorial.tree_model"
        replace=True,
    )

## Evaluate the model

### SQL

Follow these steps to evaluate the model:

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, paste in the following query and click **Run**:

   ```googlesql
     SELECT
       *
     FROM
       ML.EVALUATE (MODEL `bqml_tutorial.tree_model`,
         (
         SELECT
           *
         FROM
           `bqml_tutorial.input_data`
         WHERE
           dataframe = 'evaluation'
         )
       );
   ```

   The results should look similar to the following:

   ```
   +---+---+---+---+---+---+
   | precision           | recall              | accuracy            | f1_score          | log_loss            | roc_auc             |
   +---+---+---+---+---+
   | 0.67192429022082023 | 0.57880434782608692 | 0.83942963422194672 | 0.621897810218978 | 0.34405456040833338 | 0.88733566433566435 |
   +---+---+ ---+---+---+---+
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

    # Select model you'll use for predictions. `read_gbq_model` loads model
    # data from BigQuery, but you could also use the `tree_model` object
    # from the previous step.
    tree_model = bpd.read_gbq_model(
        your_model_id,  # For example: "your-project.bqml_tutorial.tree_model"
    )

    # input_data is defined in an earlier step.
    evaluation_data = input_data[input_data["dataframe"] == "evaluation"]
    X = evaluation_data.drop(columns=["income_bracket", "dataframe"])
    y = evaluation_data["income_bracket"]

    # The score() method evaluates how the model performs compared to the
    # actual data. Output DataFrame matches that of ML.EVALUATE().
    score = tree_model.score(X, y)
    score.peek()
    # Output:
    #    precision    recall  accuracy  f1_score  log_loss   roc_auc
    # 0   0.671924  0.578804  0.839429  0.621897  0.344054  0.887335

The evaluation metrics indicate good model performance, in particular,
the fact that the
[`roc_auc` score](https://developers.google.com/machine-learning/crash-course/classification/roc-and-auc) is greater than `0.8`.

For more information about the evaluation metrics, see
[Output](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-evaluate#output).

## Use the model to predict classifications

### SQL

Follow these steps to forecast data with the model:

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, paste in the following query and click **Run**:

   ```googlesql
     SELECT
       *
     FROM
       ML.PREDICT (MODEL `bqml_tutorial.tree_model`,
         (
         SELECT
           *
         FROM
           `bqml_tutorial.input_data`
         WHERE
           dataframe = 'prediction'
         )
       );
   ```

The first few columns of the results should look similar to the following:

<br />

```
  +---+---+---+
  | predicted_income_bracket  | predicted_income_bracket_probs.label | predicted_income_bracket_probs.prob |
  +---+---+---+
  |  <=50K                    |  >50K                                | 0.05183430016040802                 |
  +---+---+---+
  |                           |  <50K                                | 0.94816571474075317                 |
  +---+---+---+
  |  <=50K                    |  >50K                                | 0.00365859130397439                 |
  +---+---+---+
  |                           |  <50K                                | 0.99634140729904175                 |
  +---+---+---+
  |  <=50K                    |  >50K                                | 0.037775970995426178                |
  +---+---+---+
  |                           |  <50K                                | 0.96222406625747681                 |
  +---+---+---+
  
```

<br />

### BigQuery DataFrames

Before trying this sample, follow the BigQuery DataFrames
setup instructions in the [BigQuery quickstart
using BigQuery DataFrames](https://docs.cloud.google.com/bigquery/docs/dataframes-quickstart).
For more information, see the
[BigQuery DataFrames reference documentation](https://docs.cloud.google.com/python/docs/reference/bigframes/latest).

To authenticate to BigQuery, set up Application Default Credentials.
For more information, see [Set
up ADC for a local development environment](https://docs.cloud.google.com/docs/authentication/set-up-adc-local-dev-environment).

    # Select model you'll use for predictions. `read_gbq_model` loads model
    # data from BigQuery, but you could also use the `tree_model` object
    # from previous steps.
    tree_model = bpd.read_gbq_model(
        your_model_id,  # For example: "your-project.bqml_tutorial.tree_model"
    )

    # input_data is defined in an earlier step.
    prediction_data = input_data[input_data["dataframe"] == "prediction"]

    predictions = tree_model.predict(prediction_data)
    predictions.peek()
    # Output:
    # predicted_income_bracket   predicted_income_bracket_probs.label  predicted_income_bracket_probs.prob
    #                   <=50K                                   >50K                   0.05183430016040802
    #                                                           <50K                   0.94816571474075317
    #                   <=50K                                   >50K                   0.00365859130397439
    #                                                           <50K                   0.99634140729904175
    #                   <=50K                                   >50K                   0.037775970995426178
    #                                                           <50K                   0.96222406625747681

The `predicted_income_bracket` contains the predicted value from the model.
The `predicted_income_bracket_probs.label` shows the two labels that the
model had to choose between, and the `predicted_income_bracket_probs.prob`
column shows the probability of the given label being the
correct one.

For more information about the output columns, see
[Classification models](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-predict#classification_models).

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

4. In the **Delete dataset** dialog, confirm the delete command by typing
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

- Learn how to [create a logistic regression classification model](https://docs.cloud.google.com/bigquery/docs/logistic-regression-prediction).
- For an overview of BigQuery ML, see [Introduction to AI and ML in BigQuery](https://docs.cloud.google.com/bigquery/docs/bqml-introduction).