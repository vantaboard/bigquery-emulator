This tutorial teaches you how to use the
[`TRANSFORM` clause](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create#transform)
of the `CREATE MODEL` statement to perform feature engineering at the same time
that you create and train a model. Using the `TRANSFORM` clause, you
can specify one or more [preprocessing](https://docs.cloud.google.com/bigquery/docs/manual-preprocessing)
functions to transform the input data you use to train the model. The
preprocessing that you apply to the model is automatically applied when you use
the model with the
[`ML.EVALUATE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-evaluate)
and
[`ML.PREDICT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-predict)
functions.

This tutorial uses the public
[`bigquery-public-data.ml_datasets.penguin` dataset](https://console.cloud.google.com/bigquery?p=bigquery-public-data&d=ml_datasets&t=penguins&page=table).

## Objectives

This tutorial guides you through completing the following tasks:

- Creating a linear regression model to predict service call type by using the [`CREATE MODEL` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-glm). Within the `CREATE MODEL` statement, use the [`ML.QUANTILE_BUCKETIZE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-quantile-bucketize) and [`ML.FEATURE_CROSS`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-feature-cross) functions to preprocess data.
- Evaluating the model by using the [`ML.EVALUATE` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-evaluate).
- Getting predictions from the model by using the [`ML.PREDICT` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-predict).

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

## Create the model

Create a linear regression model to predict penguin weight and train it on
the `penguins` sample table.

The `OPTIONS(model_type='linear_reg', input_label_cols=['body_mass_g'])`
clause indicates that you are creating a
[linear regression](https://en.wikipedia.org/wiki/Linear_regression)
model. A linear regression model generates a
continuous value from a linear combination of input features. The
`body_mass_g` column is the input label column. For linear regression models,
the label column must be real valued (that is, the column values must be
real numbers).

This query's `TRANSFORM` clause uses the following columns from the `SELECT`
statement:

- `body_mass_g`: Used in training without any change.
- `culmen_depth_mm`: Used in training without any change.
- `flipper_length_mm`: Used in training without any change.
- `bucketized_culmen_length`: Generated from `culmen_length_mm` by bucketizing `culmen_length_mm` based on quantiles using the `ML.QUANTILE_BUCKETIZE()` analytic function.
- `culmen_length_mm`: The original `culmen_length_mm` value, cast to a `STRING` value and used in training.
- `species_sex`: Generated from crossing `species` and `sex` using the `ML.FEATURE_CROSS` function.

You don't need to use all of the columns from the training table
in the`TRANSFORM` clause.

The `WHERE` clause---`WHERE body_mass_g IS NOT NULL AND RAND() < 0.2`---
excludes rows where the penguins weight is `NULL`, and uses the `RAND` function
to draw a random sample of the data.

Follow these steps to create the model:

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, paste in the following query and click **Run**:

   ```googlesql
   CREATE OR REPLACE MODEL `bqml_tutorial.penguin_transform`
     TRANSFORM(
       body_mass_g,
       culmen_depth_mm,
       flipper_length_mm,
       ML.QUANTILE_BUCKETIZE(culmen_length_mm, 10) OVER () AS bucketized_culmen_length,
       CAST(culmen_length_mm AS string) AS culmen_length_mm,
       ML.FEATURE_CROSS(STRUCT(species, sex)) AS species_sex)
     OPTIONS (
       model_type = 'linear_reg',
       input_label_cols = ['body_mass_g'])
   AS
   SELECT
     *
   FROM
     `bigquery-public-data.ml_datasets.penguins`
   WHERE
     body_mass_g IS NOT NULL
     AND RAND() < 0.2;
   ```

   The query takes about 15 minutes to complete, after which the
   `penguin_transform` model appears in the **Explorer** pane. Because
   the query uses a `CREATE MODEL` statement to create a model, you don't see
   query results.

## Evaluate the model

Evaluate the performance of the model by using the `ML.EVALUATE` function.
The `ML.EVALUATE` function evaluates the predicted penguin weights returned by
the model against the actual penguin weights from the training data.

This query's nested `SELECT` statement and `FROM` clause are the same as those
in the `CREATE MODEL` query. Because you used the `TRANSFORM` clause when
creating the model, you don't need to specify the columns and transformations
again in the `ML.EVALUATE` function. The function automatically retrieves
them from the model.

Follow these steps to evaluate the model:

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, paste in the following query and click **Run**:

   ```googlesql
   SELECT
     *
   FROM
     ML.EVALUATE(
       MODEL `bqml_tutorial.penguin_transform`,
       (
         SELECT
           *
         FROM
           `bigquery-public-data.ml_datasets.penguins`
         WHERE
           body_mass_g IS NOT NULL
       ));
   ```

   The results should look similar to the following:

   ```
   +---+---+---+---+---+---+
   | mean_absolute_error | mean_squared_error | mean_squared_log_error | median_absolute_error |      r2_score      | explained_variance |
   +---+---+---+---+---+---+
   |   64.21134350607677 | 13016.433317859564 |   7.140935762696211E-4 |     15.31788461553515 | 0.9813042531507734 | 0.9813186268757634 |
   +---+---+---+---+---+---+
   ```

   An important metric in the evaluation results is the
   [R^2^
   score](https://en.wikipedia.org/wiki/Coefficient_of_determination).
   The R^2^ score is a statistical measure that determines if the
   linear regression predictions approximate the actual data. A value of `0`
   indicates that the model explains none of the variability of the
   response data around the mean. A value of `1` indicates that the model
   explains all the variability of the response data around the mean.

   For more information about the `ML.EVALUATE` function output, see
   [Output](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-evaluate#output).

   You can also call `ML.EVALUATE` without providing the input data. It will
   use the evaluation metrics calculated during training.

## Use the model to predict penguin weight

Use the model with the `ML.PREDICT` function to predict the weight of male
penguins.

The `ML.PREDICT` function outputs the predicted value in the
`predicted_label_column_name` column, in this case
`predicted_body_mass_g`.

When you use the `ML.PREDICT` function, you don't have to pass in all of the
columns used in model training. Only the columns that you used in the
`TRANSFORM` clause are required. Similar to `ML.EVALUATE`, the `ML.PREDICT`
function automatically retrieves the `TRANSFORM` columns and transformations
from the model.

Follow these steps to get predictions from the model:

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, paste in the following query and click **Run**:

   ```googlesql
   SELECT
     predicted_body_mass_g
   FROM
     ML.PREDICT(
       MODEL `bqml_tutorial.penguin_transform`,
       (
         SELECT
           *
         FROM
           `bigquery-public-data.ml_datasets.penguins`
         WHERE
           sex = 'MALE'
       ));
   ```

   The results should look similar to the following:

   ```
   +---+
   | predicted_body_mass_g |
   +---+
   |    2810.2868541725757 |
   +---+
   |    3813.6574220842676 |
   +---+
   |     4098.844698262214 |
   +---+
   |     4256.587135004173 |
   +---+
   |     3008.393497302691 |
   +---+
   |     ...               |
   +---+
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

- To learn more about machine learning, see the [Machine learning crash course](https://developers.google.com/machine-learning/crash-course/).
- For an overview of BigQuery ML, see [Introduction to BigQuery ML](https://docs.cloud.google.com/bigquery/docs/bqml-introduction).
- To learn more about the Google Cloud console, see [Using the Google Cloud console](https://docs.cloud.google.com/bigquery/bigquery-web-ui).