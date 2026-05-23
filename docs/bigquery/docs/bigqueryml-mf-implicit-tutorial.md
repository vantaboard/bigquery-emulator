This tutorial teaches you how to create a
[matrix factorization model](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-matrix-factorization)
and train it on the Google Analytics 360 user session data in the public
[`GA360_test.ga_sessions_sample` table](https://console.cloud.google.com/bigquery?p=cloud-training-demos&d=GA360_test&t=ga_sessions_sample&page=table). You then use the matrix factorization model to generate content recommendations
for site users.

Using indirect customer preference information, like user session duration,
to train the model is called training with *implicit feedback* . Matrix
factorization models are trained using the
[Weighted-Alternating Least Squares algorithm](http://yifanhu.net/PUB/cf.pdf)
when you use implicit feedback as training data.

> [!IMPORTANT]
> **Important:** You must have a reservation in order to use a matrix factorization model. For more information, see [Pricing](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-matrix-factorization#pricing).

## Objectives

This tutorial guides you through completing the following tasks:

- Creating a matrix factorization model by using the `CREATE MODEL` statement.
- Evaluating the model by using the [`ML.EVALUATE` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-evaluate).
- Generating content recommendations for users by using the model with the [`ML.RECOMMEND` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-recommend).

## Costs

This tutorial uses billable components of Google Cloud,
including:

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

## Prepare the sample data

Transform the data from the `GA360_test.ga_sessions_sample` table into a better
structure for model training, and then write this data to a
BigQuery table. The following query calculates the session
duration for each user for each piece of content, which you can then use as
implicit feedback to infer the user's preference for that content.

Follow these steps to create the training data table:

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. Create the training data table. In the query editor, paste in the following
   query and click **Run**:

   ```googlesql
   CREATE OR REPLACE TABLE `bqml_tutorial.analytics_session_data`
   AS
   WITH
     visitor_page_content AS (
       SELECT
         fullVisitorID,
         (
           SELECT
             MAX(
               IF(
                 index = 10,
                 value,
                 NULL))
           FROM
             UNNEST(hits.customDimensions)
         ) AS latestContentId,
         (LEAD(hits.time, 1) OVER (PARTITION BY fullVisitorId ORDER BY hits.time ASC) - hits.time)
           AS session_duration
       FROM
         `cloud-training-demos.GA360_test.ga_sessions_sample`,
         UNNEST(hits) AS hits
       WHERE
         # only include hits on pages
         hits.type = 'PAGE'
       GROUP BY
         fullVisitorId,
         latestContentId,
         hits.time
     )
   # aggregate web stats
   SELECT
     fullVisitorID AS visitorId,
     latestContentId AS contentId,
     SUM(session_duration) AS session_duration
   FROM
     visitor_page_content
   WHERE
     latestContentId IS NOT NULL
   GROUP BY
     fullVisitorID,
     latestContentId
   HAVING
     session_duration > 0
   ORDER BY
     latestContentId;
   ```
3. View a subset of the training data. In the query editor, paste in the following
   query and click **Run**:

   ```googlesql
   SELECT * FROM `bqml_tutorial.analytics_session_data` LIMIT 5;
   ```

   The results should look similar to the following:

   ```
   +---+---+---+
   | visitorId           | contentId | session_duration |
   +---+---+---+
   | 7337153711992174438 | 100074831 | 44652            |
   +---+---+---+
   | 5190801220865459604 | 100170790 | 121420           |
   +---+---+---+
   | 2293633612703952721 | 100510126 | 47744            |
   +---+---+---+
   | 5874973374932455844 | 100510126 | 32109            |
   +---+---+---+
   | 1173698801255170595 | 100676857 | 10512            |
   +---+---+---+
   ```

## Create the model

Create a matrix factorization model and train it on the data in the
`analytics_session_data` table. The model is trained to predict a confidence
rating for every `visitorId`-`contentId` pair. The confidence rating is created
with centering and scaling by the median session duration. Records where the
session duration is more than 3.33 times the median are filtered out
as outliers.

The following `CREATE MODEL` statement uses these columns to generate
recommendations:

- `visitorId`---The visitor ID.
- `contentId`---The content ID.
- `rating`---The implicit rating from 0 to 1 calculated for each visitor-content pair, centered and scaled.

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, paste in the following query and click **Run**:

   ```googlesql
   CREATE OR REPLACE MODEL `bqml_tutorial.mf_implicit`
     OPTIONS (
       MODEL_TYPE = 'matrix_factorization',
       FEEDBACK_TYPE = 'implicit',
       USER_COL = 'visitorId',
       ITEM_COL = 'contentId',
       RATING_COL = 'rating',
       L2_REG = 30,
       NUM_FACTORS = 15)
   AS
   SELECT
     visitorId,
     contentId,
     0.3 * (1 + (session_duration - 57937) / 57937) AS rating
   FROM `bqml_tutorial.analytics_session_data`
   WHERE 0.3 * (1 + (session_duration - 57937) / 57937) < 1;
   ```

   The query takes about 10 minutes to complete, after which the
   `mf_implicit` model appears in the **Explorer** pane. Because
   the query uses a `CREATE MODEL` statement to create a model, you don't see
   query results.

## Get training statistics

Optionally, you can view the model's training statistics in the
Google Cloud console.

A machine learning algorithm builds a model by creating many iterations of
the model using different parameters, and then selecting the version of the
model that minimizes [loss](https://en.wikipedia.org/wiki/Loss_function).
This process is called empirical risk minimization. The model's training
statistics let you see the loss associated with each iteration of the model.

Follow these steps to view the model's training statistics:

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)

   If you don't see the left pane, click **Expand left pane** to open the pane.
3. In the **Explorer** pane, expand your project and click **Datasets**.

4. Click the `bqml_tutorial` dataset. You can also use the search feature or
   filters to find the dataset.

5. Click the **Models** tab.

6. Click the `mf_implicit` model and then click the **Training** tab

7. In the **View as** section, click **Table**. The results should look
   similar to the following:

   ```
   +---+---+---+
   | Iteration | Training Data Loss | Duration (seconds) |
   +---+---+---+
   |  5        | 0.0027             | 47.27              |
   +---+---+---+
   |  4        | 0.0028             | 39.60              |
   +---+---+---+
   |  3        | 0.0032             | 55.57              |
   +---+---+---+
   |  ...      | ...                | ...                |
   +---+---+---+
   ```

   The **Training Data Loss** column represents the loss metric calculated
   after the model is trained. Because this is a matrix factorization model,
   this column shows the
   [mean squared error](https://developers.google.com/machine-learning/glossary/#MSE).

## Evaluate the model

Evaluate the performance of the model by using the `ML.EVALUATE` function.
The `ML.EVALUATE` function evaluates the predicted content ratings returned by
the model against the evaluation metrics calculated during training.

Follow these steps to evaluate the model:

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, paste in the following query and click **Run**:

   ```googlesql
   SELECT
     *
   FROM
     ML.EVALUATE(MODEL `bqml_tutorial.mf_implicit`);
   ```

   The results should look similar to the following:

   ```
   +---+---+---+---+
   | mean_average_precision |  mean_squared_error   | normalized_discounted_cumulative_gain |    average_rank     |
   +---+---+---+---+
   |     0.4434341257478137 | 0.0013381759837648962 |                    0.9433280547112802 | 0.24031636088594222 |
   +---+---+---+---+
   ```

   For more information about the `ML.EVALUATE` function output, see
   [Output](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-evaluate#output).

## Get the predicted ratings for a subset of visitor-content pairs

Use the `ML.RECOMMEND` to get the predicted rating for each piece of content
for five site visitors.

Follow these steps to get predicted ratings:

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, paste in the following query and click **Run**:

   ```googlesql
   SELECT
     *
   FROM
     ML.RECOMMEND(
       MODEL `bqml_tutorial.mf_implicit`,
       (
         SELECT
           visitorId
         FROM
           `bqml_tutorial.analytics_session_data`
         LIMIT 5
       ));
   ```

   The results should look similar to the following:

   ```
   +---+---+---+
   | predicted_rating_confidence   | visitorId           | contentId |
   +---+---+---+
   | 0.0033608418060270262         | 7337153711992174438 | 277237933 |
   +---+---+---+
   | 0.003602395397293956          | 7337153711992174438 | 158246147 |
   +---+---+--  ---+
   | 0.0053197670652785356         | 7337153711992174438 | 299389988 |
   +---+---+---+
   | ...                           | ...                 | ...       |
   +---+---+---+
   ```

## Generate recommendations

Use the predicted ratings to generate the top five recommended content IDs
for each visitor ID.

Follow these steps to generate recommendations:

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. Write the predicted ratings to a table. In the query editor, paste in the
   following query and click **Run**:

   ```googlesql
   CREATE OR REPLACE TABLE `bqml_tutorial.recommend_content`
   AS
   SELECT
     *
   FROM
     ML.RECOMMEND(MODEL `bqml_tutorial.mf_implicit`);
   ```
3. Select the top five results per visitor. In the query editor, paste in the
   following query and click **Run**:

   ```googlesql
   SELECT
     visitorId,
     ARRAY_AGG(
       STRUCT(contentId, predicted_rating_confidence)
       ORDER BY predicted_rating_confidence DESC
       LIMIT 5) AS rec
   FROM
     `bqml_tutorial.recommend_content`
   GROUP BY
     visitorId;
   ```

   The results should look similar to the following:

   ```
   +---+---+---+
   | visitorId           | rec:contentId   | rec:predicted_rating_confidence |
   +---+---+---  ---+
   | 867526255058981688  | 299804319       | 0.88170525357178664             |
   |                     | 299935287       | 0.54699439944935124             |
   |                     | 299410466       | 0.53424780863188659             |
   |                     | 299826767       | 0.46949603950374219             |
   |                     | 299809748       | 0.3379991197434149              |
   +---+---+---+
   | 2434264018925667659 | 299824032       | 1.3903516407308065              |
   |                     | 299410466       | 0.9921995618196483              |
   |                     | 299903877       | 0.92333625294129218             |
   |                     | 299816215       | 0.91856701667757279             |
   |                     | 299852437       | 0.86973661454890561             |
   +---+---+---+
   | ...                 | ...             | ...                             |
   +---+---+---+
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

- Try [creating a matrix factorization model based on explicit feedback](https://docs.cloud.google.com/bigquery/docs/bigqueryml-mf-explicit-tutorial).
- For an overview of BigQuery ML, see [Introduction to BigQuery ML](https://docs.cloud.google.com/bigquery/docs/bqml-introduction).
- To learn more about machine learning, see the [Machine learning crash course](https://developers.google.com/machine-learning/crash-course/).