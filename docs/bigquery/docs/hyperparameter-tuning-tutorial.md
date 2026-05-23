This tutorial teaches you how to use
[hyperparameter tuning](https://docs.cloud.google.com/bigquery/docs/hp-tuning-overview) in
BigQuery ML to tune a machine learning model and improve its
performance.

You perform hyperparameter tuning by specifying the
[`NUM_TRIALS` option](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create#num_trials)
of the `CREATE MODEL` statement, in combination with other model-specific
options. When you set these options, BigQuery ML trains
multiple versions, or *trials* of the model, each with slightly different
parameters, and returns the trial that performs the best.

This tutorial uses the public
[`tlc_yellow_trips_2018` sample table](https://console.cloud.google.com/bigquery?p=bigquery-public-data&d=new_york_taxi_trips&t=tlc_yellow_trips_2018&page=table), which contains information about taxi trips in New York City
in 2018.

## Objectives

This tutorial guides you through completing the following tasks:

- Using the [`CREATE MODEL` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-glm) to create a baseline linear regression model.
- Evaluating the baseline model by using the [`ML.EVALUATE` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-evaluate).
- Using the `CREATE MODEL` statement with hyperparameter tuning options to train twenty trials of a linear regression model.
- Reviewing the trials by using the [`ML.TRIAL_INFO` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-trial-info).
- Evaluating the trials by using the `ML.EVALUATE` function.
- Get predictions about taxi trips from the optimal model among the trials by using the [`ML.PREDICT` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-predict).

## Costs

This tutorial uses billable components of Google Cloud,
including:

- BigQuery
- BigQuery ML

For more information about BigQuery costs, see the
[BigQuery pricing](https://cloud.google.com/bigquery/pricing) page.

## Before you begin

1. BigQuery is automatically enabled in new projects. To activate BigQuery in a pre-existing project, go to


   Enable the BigQuery API.


   **Roles required to enable APIs**


   To enable APIs, you need the Service Usage Admin IAM
   role (`roles/serviceusage.serviceUsageAdmin`), which
   contains the `serviceusage.services.enable` permission. [Learn how to grant
   roles](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).

   [Enable the API](https://console.cloud.google.com/flows/enableapi?apiid=bigquery)

## Required permissions

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

## Create a table of training data

Create a table of training data, based on a subset of the
`tlc_yellow_trips_2018` table data.

Follow these steps to create the table:

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, paste in the following query and click **Run**:

   ```googlesql
   CREATE OR REPLACE TABLE `bqml_tutorial.taxi_tip_input`
   AS
   SELECT * EXCEPT (tip_amount), tip_amount AS label
   FROM
     `bigquery-public-data.new_york_taxi_trips.tlc_yellow_trips_2018`
   WHERE
     tip_amount IS NOT NULL
   LIMIT 100000;
   ```

## Create a baseline linear regression model

Create a linear regression model without hyperparameter tuning and train it on
the `taxi_tip_input` table data.

Follow these steps to create the model:

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, paste in the following query and click **Run**:

   ```googlesql
   CREATE OR REPLACE MODEL `bqml_tutorial.baseline_taxi_tip_model`
     OPTIONS (
       MODEL_TYPE = 'LINEAR_REG'
     )
   AS
   SELECT
     *
   FROM
     `bqml_tutorial.taxi_tip_input`;
   ```

   The query takes about 2 minutes to complete.

## Evaluate the baseline model

Evaluate the performance of the model by using the `ML.EVALUATE` function.
The `ML.EVALUATE` function evaluates the predicted content ratings returned by
the model against the evaluation metrics calculated during model training.

Follow these steps to evaluate the model:

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, paste in the following query and click **Run**:

   ```googlesql
   SELECT *
   FROM
     ML.EVALUATE(MODEL `bqml_tutorial.baseline_taxi_tip_model`);
   ```

   The results look similar to the following:

   ```
   +---+---+---+---+---+---+
   | mean_absolute_error | mean_squared_error | mean_squared_log_error | median_absolute_error |      r2_score       | explained_variance  |
   +---+---+---+---+---+---+
   |  2.5853895559690323 | 23760.416358496139 |   0.017392406523370374 | 0.0044248227819481123 | -1934.5450533482465 | -1934.3513857946277 |
   +---+---+---+---+---+---+
   ```

The `r2_score` value for the baseline model is negative, which indicates a
poor fit for the data; the closer the
[R^2^ score](https://en.wikipedia.org/wiki/Coefficient_of_determination)
is to 1, the better the model fit is.

## Create a linear regression model with hyperparameter tuning

Create a linear regression model with hyperparameter tuning and train it on
the `taxi_tip_input` table data.

You use the following hyperparameter tuning options in the `CREATE MODEL`
statement:

- The [`NUM_TRIALS` option](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-glm#num_trials) to set the number of trials to twenty.
- The [`MAX_PARALLEL_TRIALS` option](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-glm#max_parallel_trials) to run two trials in each training job, for a total of ten jobs and twenty trials. This reduces the training time needed. However, the two concurrent trials don't benefit from each other's training results.
- The [`L1_REG` option](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-glm#l1_reg) to try different L1 regularization values in the different trials. L1 regularization removes irrelevant features from the model, which helps prevent [overfitting](https://developers.google.com/machine-learning/glossary/#overfitting).

The other hyperparameter tuning options supported by the model use their default
values, as follows:

- `L1_REG`: `0`
- `HPARAM_TUNING_ALGORITHM`: `'VIZIER_DEFAULT'`
- `HPARAM_TUNING_OBJECTIVES`: `['R2_SCORE']`

Follow these steps to create the model:

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, paste in the following query and click **Run**:

   ```googlesql
   CREATE OR REPLACE MODEL `bqml_tutorial.hp_taxi_tip_model`
     OPTIONS (
       MODEL_TYPE = 'LINEAR_REG',
       NUM_TRIALS = 20,
       MAX_PARALLEL_TRIALS = 2,
       L1_REG = HPARAM_RANGE(0, 5))
   AS
   SELECT
     *
   FROM
     `bqml_tutorial.taxi_tip_input`;
   ```

   The query takes approximately 20 minutes to complete.

## Get information about the training trials

Get information about all of the trials, including their hyperparameter values,
objectives, and status, by using the `ML.TRIAL_INFO` function. This function
also returns information about which trial has the best performance, based on
this information.

Follow these steps to get trial information:

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, paste in the following query and click **Run**:

   ```googlesql
   SELECT *
   FROM
     ML.TRIAL_INFO(MODEL `bqml_tutorial.hp_taxi_tip_model`)
   ORDER BY is_optimal DESC;
   ```

   The results look similar to the following:

   ```
   +---+---+---+---+---+---+---+---+
   | trial_id |           hyperparameters           | hparam_tuning_evaluation_metrics  |   training_loss    |     eval_loss      |  status   | error_message | is_optimal |
   +---+---+---+---+---+---+---+---+
   |        7 |      {"l1_reg":"4.999999999999985"} |  {"r2_score":"0.653653627638174"} | 4.4677841296238165 |  4.478469742512195 | SUCCEEDED | NULL          |       true |
   |        2 |  {"l1_reg":"2.402163664510254E-11"} | {"r2_score":"0.6532493667964732"} |  4.457692508421795 |  4.483697081650438 | SUCCEEDED | NULL          |      false |
   |        3 |  {"l1_reg":"1.2929452948742316E-7"} |  {"r2_score":"0.653249366811995"} |   4.45769250849513 |  4.483697081449748 | SUCCEEDED | NULL          |      false |
   |        4 |  {"l1_reg":"2.5787102060628228E-5"} | {"r2_score":"0.6532493698925899"} |  4.457692523040582 |  4.483697041615808 | SUCCEEDED | NULL          |      false |
   |      ... |                             ...     |                           ...     |              ...   |             ...    |       ... |          ...  |        ... |
   +---+---+---+---+---+---+---+---+
   ```

   The `is_optimal` column value indicates that trial 7 is the optimal model
   returned by the tuning.

## Evaluate the tuned model trials

Evaluate the performance of the trials by using the `ML.EVALUATE` function.
The `ML.EVALUATE` function evaluates the predicted content ratings returned by
the model against the evaluation metrics calculated during training for all
trials.

Follow these steps to evaluate the model trials:

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, paste in the following query and click **Run**:

   ```googlesql
   SELECT *
   FROM
     ML.EVALUATE(MODEL `bqml_tutorial.hp_taxi_tip_model`)
   ORDER BY r2_score DESC;
   ```

   The results look similar to the following:

   ```
   +---+---+---+---+---+---+---+
   | trial_id | mean_absolute_error | mean_squared_error | mean_squared_log_error | median_absolute_error |      r2_score      | explained_variance |
   +---+---+---+---+---+---+---+
   |        7 |   1.151814398002232 |  4.109811493266523 |     0.4918733252641176 |    0.5736103414025084 | 0.6652110305659145 | 0.6652144696114834 |
   |       19 |  1.1518143358927102 |  4.109811921460791 |     0.4918672150119582 |    0.5736106106914161 | 0.6652109956848206 | 0.6652144346901685 |
   |        8 |   1.152747850702547 |  4.123625876152422 |     0.4897808307399327 |    0.5731702310239184 | 0.6640856984144734 |  0.664088410199906 |
   |        5 |   1.152895108945439 |  4.125775524878872 |    0.48939088205957937 |    0.5723300569616766 | 0.6639105860807425 | 0.6639132416838652 |
   |      ... |                ...  |                ... |                    ... |                   ... |                ... |                ... |
   +---+---+---+---+---+---+---+
   ```

   The `r2_score` value for the optimal model, which is trial 7, is
   `0.66521103056591446`, which shows significant improvement over the
   baseline model.

You can evaluate a specific trial by specifying the `TRIAL_ID`
argument in the `ML.EVALUATE` function.

For more information about the difference between `ML.TRIAL_INFO`
objectives and `ML.EVALUATE` evaluation metrics, see
[Model serving functions](https://docs.cloud.google.com/bigquery/docs/hp-tuning-overview#model_serving_functions).

## Use the tuned model to predict taxi tips

Use the optimal model returned by tuning to predict tips for different taxi
trips. The optimal model is automatically used by the `ML.PREDICT` function,
unless you select a different trial by specifying the `TRIAL_ID` argument. The
predictions are returned in the `predicted_label` column.

Follow these steps to get predictions:

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, paste in the following query and click **Run**:

   ```googlesql
   SELECT *
   FROM
     ML.PREDICT(
       MODEL `bqml_tutorial.hp_taxi_tip_model`,
       (
         SELECT
           *
         FROM
           `bqml_tutorial.taxi_tip_input`
         LIMIT 5
       ));
   ```

   The results look similar to the following:

   ```
   +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
   | trial_id |  predicted_label   | vendor_id |   pickup_datetime   |  dropoff_datetime   | passenger_count | trip_distance | rate_code | store_and_fwd_flag | payment_type | fare_amount | extra | mta_tax | tolls_amount | imp_surcharge | total_amount | pickup_location_id | dropoff_location_id | data_file_year | data_file_month | label |
   +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
   |        7 |  1.343367839584448 | 2         | 2018-01-15 18:55:15 | 2018-01-15 18:56:18 |               1 |             0 | 1         | N                  | 1            |           0 |     0 |       0 |            0 |             0 |            0 | 193                | 193                 |           2018 |               1 |     0 |
   |        7 | -1.176072791783461 | 1         | 2018-01-08 10:26:24 | 2018-01-08 10:26:37 |               1 |             0 | 5         | N                  | 3            |        0.01 |     0 |       0 |            0 |           0.3 |         0.31 | 158                | 158                 |           2018 |               1 |     0 |
   |        7 |  3.839580104168765 | 1         | 2018-01-22 10:58:02 | 2018-01-22 12:01:11 |               1 |          16.1 | 1         | N                  | 1            |        54.5 |     0 |     0.5 |            0 |           0.3 |         55.3 | 140                | 91                  |           2018 |               1 |     0 |
   |        7 |  4.677393985230036 | 1         | 2018-01-16 10:14:35 | 2018-01-16 11:07:28 |               1 |            18 | 1         | N                  | 2            |        54.5 |     0 |     0.5 |            0 |           0.3 |         55.3 | 138                | 67                  |           2018 |               1 |     0 |
   |        7 |  7.938988937253062 | 2         | 2018-01-16 07:05:15 | 2018-01-16 08:06:31 |               1 |          17.8 | 1         | N                  | 1            |        54.5 |     0 |     0.5 |            0 |           0.3 |        66.36 | 132                | 255                 |           2018 |               1 | 11.06 |
   +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
   ```

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
2. In the navigation panel, click the **bqml_tutorial** dataset you created.

3. On the right side of the window, click **Delete dataset**. This action
   deletes the dataset, the table, and all the data.

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

- To learn more about machine learning, see the [Machine learning crash course](https://developers.google.com/machine-learning/crash-course/).
- For an overview of BigQuery ML, see [Introduction to BigQuery ML](https://docs.cloud.google.com/bigquery/docs/bqml-introduction).
- To learn more about the Google Cloud console, see [Using the Google Cloud console](https://docs.cloud.google.com/bigquery/bigquery-web-ui).