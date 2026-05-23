This tutorial teaches you how to use a
[multivariate time series model](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-multivariate-time-series) to forecast the future value
for a given column, based on the historical value of multiple input features.

This tutorial forecasts for multiple time series. Forecasted values are
calculated for each time point, for each value in one or more specified columns.
For example, if you wanted to forecast weather and specified a column containing
state data, the forecasted data would contain forecasts for all time points for
State A, then forecasted values for all time points for State B, and so forth.
If you wanted to forecast weather and specified columns containing
state and city data, the forecasted data would contain forecasts for all time
points for State A and City A, then forecasted values for all time points for
State A and City B, and so forth.

This tutorial uses data from the public
[`bigquery-public-data.iowa_liquor_sales.sales`](https://console.cloud.google.com/bigquery?p=bigquery-public-data&d=iowa_liquor_sales&page=dataset&t=sales&page=table)
and
[`bigquery-public-data.covid19_weathersource_com.postal_code_day_history`](https://console.cloud.google.com/bigquery?p=bigquery-public-data&d=covid19_weathersource_com&page=dataset&t=postal_code_day_history&page=table)
tables. The `bigquery-public-data.iowa_liquor_sales.sales` table contains
liquor sales data collected from multiple cities in the state of Iowa. The
`bigquery-public-data.covid19_weathersource_com.postal_code_day_history` table
contains historical weather data, such as temperature and humidity, from
around the world.

Before reading this tutorial, we highly recommend that you read
[Forecast a single time series with a multivariate model](https://docs.cloud.google.com/bigquery/docs/arima-plus-xreg-single-time-series-forecasting-tutorial).

## Objectives

This tutorial guides you through completing the following tasks:

- Creating a time series model to forecast liquor store orders by using the [`CREATE MODEL` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-multivariate-time-series).
- Retrieving the forecasted order values from the model by using the [`ML.FORECAST` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-forecast).
- Retrieving components of the time series, such as seasonality, trend, and feature attributions, by using the [`ML.EXPLAIN_FORECAST` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-explain-forecast). You can inspect these time series components in order to explain the forecasted values.
- Evaluate the model's accuracy by using the [`ML.EVALUATE` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-evaluate).
- Detect anomalies by using the model with the [`ML.DETECT_ANOMALIES` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-detect-anomalies).

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

## Create a table of input data

Create a table of data that you can use to train and evaluate the model. This
table combines columns from the
`bigquery-public-data.iowa_liquor_sales.sales` and
`bigquery-public-data.covid19_weathersource_com.postal_code_day_history` tables
to analyze how weather affects the type and number of items ordered by liquor
stores. You also create the following additional columns that you can use as
input variables for the model:

- `date`: the date of the order
- `store_number`: the unique number of the store that placed the order
- `item_number`: the unique number of the item that was ordered
- `bottles_sold`: the number of bottles ordered of the associated item
- `temperature`: the average temperature at the store location on the order date
- `humidity`: the average humidity at the store location on the order date

Follow these steps to create the input data table:

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, paste in the following query and click **Run**:

   ```googlesql
   CREATE OR REPLACE TABLE
     `bqml_tutorial.iowa_liquor_sales_with_weather` AS
   WITH
     sales AS (
       SELECT
         DATE,
         store_number,
         item_number,
         bottles_sold,
         SAFE_CAST(SAFE_CAST(zip_code AS FLOAT64) AS INT64) AS zip_code
       FROM
         `bigquery-public-data.iowa_liquor_sales.sales` AS sales
       WHERE
         SAFE_CAST(zip_code AS FLOAT64) IS NOT NULL
     ),
     aggregated_sales AS (
       SELECT
         DATE,
         store_number,
         item_number,
         ANY_VALUE(zip_code) AS zip_code,
         SUM(bottles_sold) AS bottles_sold,
       FROM
         sales
       GROUP BY
         DATE,
         store_number,
         item_number
     ),
     weather AS (
       SELECT
         DATE,
         SAFE_CAST(postal_code AS INT64) AS zip_code,
         avg_temperature_air_2m_f AS temperature,
         avg_humidity_specific_2m_gpkg AS humidity,
       FROM
         `bigquery-public-data.covid19_weathersource_com.postal_code_day_history`
       WHERE
         country = 'US' AND
         SAFE_CAST(postal_code AS INT64) IS NOT NULL
     )
   SELECT
     aggregated_sales.date,
     aggregated_sales.store_number,
     aggregated_sales.item_number,
     aggregated_sales.bottles_sold,
     weather.temperature AS temperature,
     weather.humidity AS humidity
   FROM
     aggregated_sales
     LEFT JOIN weather ON aggregated_sales.zip_code=weather.zip_code
     AND aggregated_sales.DATE=weather.DATE;
   ```

## Create the time series model

Create a time series model to forecast bottles sold for each combination
of store ID and item ID, for each date in the
`bqml_tutorial.iowa_liquor_sales_with_weather` table prior to
September 1, 2022. Use the store location's average temperature and humidity
on each date as features to evaluate during forecasting. There are about 1
million distinct combinations of item number and store number in the
`bqml_tutorial.iowa_liquor_sales_with_weather` table, which means there are
1 million different time series to forecast.

Follow these steps to create the model:

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, paste in the following query and click **Run**:

   ```googlesql
   CREATE
   OR REPLACE MODEL `bqml_tutorial.multi_time_series_arimax_model`
   OPTIONS(
     model_type = 'ARIMA_PLUS_XREG',
     time_series_id_col = ['store_number', 'item_number'],
     time_series_data_col = 'bottles_sold',
     time_series_timestamp_col = 'date'
   )
   AS SELECT
     *
   FROM
     `bqml_tutorial.iowa_liquor_sales_with_weather`
   WHERE
     DATE < DATE('2022-09-01');
   ```

   The query takes approximately 38 minutes to complete, after which
   you can access the `multi_time_series_arimax_model` model. Because the
   query uses a `CREATE MODEL` statement to create a model, you don't see
   query results.

## Use the model to forecast data

Forecast future time series values by using the `ML.FORECAST`
function.

In the following GoogleSQL query, the
`STRUCT(5 AS horizon, 0.8 AS confidence_level)` clause indicates that the
query forecasts 5 future time points, and generates a prediction interval
with an 80% confidence level.

The data signature of the input data for the `ML.FORECAST` function is
the same as the data signature for the training data that you used to create
the model. The `bottles_sold` column isn't included in the input, because that
is the data the model is trying to forecast.

Follow these steps to forecast data with the model:

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, paste in the following query and click **Run**:

   ```googlesql
   SELECT
     *
   FROM
     ML.FORECAST (
       model `bqml_tutorial.multi_time_series_arimax_model`,
       STRUCT (5 AS horizon, 0.8 AS confidence_level),
       (
         SELECT
           * EXCEPT (bottles_sold)
         FROM
           `bqml_tutorial.iowa_liquor_sales_with_weather`
         WHERE
           DATE>=DATE('2022-09-01')
       )
     );
   ```

   The results should look similar to the following:

   ![Forecasted data for the number of bottles sold.](https://docs.cloud.google.com/static/bigquery/images/multivariate-multiple-forecast-output.png)

   The output rows are in order by the `store_number` value, then by the
   `item_ID` value, then in chronological order by the
   `forecast_timestamp` column value. In time series forecasting, the prediction
   interval, as represented by the `prediction_interval_lower_bound` and
   `prediction_interval_upper_bound` column values, is as important as the
   `forecast_value` column value. The `forecast_value` value is the middle point
   of the prediction interval. The prediction interval depends on the
   `standard_error` and `confidence_level` column values.

   For more information about the output columns, see
   [`ML.FORECAST`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-forecast).

## Explain the forecasting results

You can get explainability metrics in addition to forecast data by using the
`ML.EXPLAIN_FORECAST` function. The `ML.EXPLAIN_FORECAST` function forecasts
future time series values and also returns all the separate components of the
time series.

Similar to the `ML.FORECAST` function, the
`STRUCT(5 AS horizon, 0.8 AS confidence_level)` clause used in the
`ML.EXPLAIN_FORECAST` function indicates that the query forecasts 30 future
time points and generates a prediction interval with 80% confidence.

The `ML.EXPLAIN_FORECAST` function provides both historical data and
forecast data. To see only the forecast data, add the `time_series_type` option
to the query and specify `forecast` as the option value.

Follow these steps to explain the model's results:

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, paste in the following query and click **Run**:

   ```googlesql
   SELECT
     *
   FROM
     ML.EXPLAIN_FORECAST (
       model `bqml_tutorial.multi_time_series_arimax_model`,
       STRUCT (5 AS horizon, 0.8 AS confidence_level),
       (
         SELECT
           * EXCEPT (bottles_sold)
         FROM
           `bqml_tutorial.iowa_liquor_sales_with_weather`
         WHERE
           DATE >= DATE('2022-09-01')
       )
     );
   ```

   The results should look similar to the following:

   ![The first nine output columns of forecasted data and forecast explanations.](https://docs.cloud.google.com/static/bigquery/images/arima-multiple-series-ml-explain-forecast1.png)
   ![The tenth through seventeenth output columns of forecasted data and forecast explanations.](https://docs.cloud.google.com/static/bigquery/images/arima-multiple-series-ml-explain-forecast2.png)
   ![The last six output columns of forecasted data and forecast explanations.](https://docs.cloud.google.com/static/bigquery/images/arima-multiple-series-ml-explain-forecast3.png)

   The output rows are ordered chronologically by the `time_series_timestamp`
   column value.

   For more information about the output columns, see
   [`ML.EXPLAIN_FORECAST`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-explain-forecast).

## Evaluate forecasting accuracy

Evaluate the forecasting accuracy of the model by running it on data that the
model hasn't been trained on. You can do this by using the `ML.EVALUATE`
function. The `ML.EVALUATE` function evaluates each time series independently.

In the following GoogleSQL query, the second `SELECT` statement
provides the data with the future features, which are used
to forecast the future values to compare to the actual data.

Follow these steps to evaluate the model's accuracy:

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, paste in the following query and click **Run**:

   ```googlesql
   SELECT
     *
   FROM
     ML.EVALUATE (
       model `bqml_tutorial.multi_time_series_arimax_model`,
       (
         SELECT
           *
         FROM
          `bqml_tutorial.iowa_liquor_sales_with_weather`
         WHERE
           DATE >= DATE('2022-09-01')
       )
     );
   ```

   The results should look similar to the following:

   ![Evaluation metrics for the model.](https://docs.cloud.google.com/static/bigquery/images/arima-plus-xreg-multiple-time-series-ml-evaluate.png)

   For more information about the output columns, see
   [`ML.EVALUATE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-evaluate).

## Use the model to detect anomalies

Detect anomalies in the training data by using the `ML.DETECT_ANOMALIES`
function.

In the following query, the `STRUCT(0.95 AS anomaly_prob_threshold)` clause
causes the `ML.DETECT_ANOMALIES` function to identify anomalous data points
with a 95% confidence level.

Follow these steps to detect anomalies in the training data:

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, paste in the following query and click **Run**:

   ```googlesql
   SELECT
     *
   FROM
     ML.DETECT_ANOMALIES (
       model `bqml_tutorial.multi_time_series_arimax_model`,
       STRUCT (0.95 AS anomaly_prob_threshold)
     );
   ```

   The results should look similar to the following:

   ![Anomaly detection information for the training data.](https://docs.cloud.google.com/static/bigquery/images/multivariate-multiple-anomaly-detection.png)

   The `anomaly_probability` column in the results identifies the likelihood
   that a given `bottles_sold` column value is anomalous.

   For more information about the output columns, see
   [`ML.DETECT_ANOMALIES`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-detect-anomalies).

### Detect anomalies in new data

Detect anomalies in the new data by providing input data to the
`ML.DETECT_ANOMALIES` function. The new data must have the same data
signature as the training data.

Follow these steps to detect anomalies in new data:

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, paste in the following query and click **Run**:

   ```googlesql
   SELECT
     *
   FROM
     ML.DETECT_ANOMALIES (
       model `bqml_tutorial.multi_time_series_arimax_model`,
       STRUCT (0.95 AS anomaly_prob_threshold),
       (
         SELECT
           *
         FROM
           `bqml_tutorial.iowa_liquor_sales_with_weather`
         WHERE
           DATE >= DATE('2022-09-01')
       )
     );
   ```

   The results should look similar to the following:

   ![Anomaly detection information for new data.](https://docs.cloud.google.com/static/bigquery/images/multivariate-multiple-anomaly-detection2.png)

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

- Learn how to [forecast a single time series with a univariate model](https://docs.cloud.google.com/bigquery/docs/arima-single-time-series-forecasting-tutorial)
- Learn how to [forecast multiple time series with a univariate model](https://docs.cloud.google.com/bigquery/docs/arima-multiple-time-series-forecasting-tutorial)
- Learn how to [scale a univariate model when forecasting multiple time series over many rows](https://docs.cloud.google.com/bigquery/docs/arima-speed-up-tutorial).
- Learn how to [hierarchically forecast multiple time series with a univariate model](https://docs.cloud.google.com/bigquery/docs/arima-time-series-forecasting-with-hierarchical-time-series)
- For an overview of BigQuery ML, see [Introduction to AI and ML in BigQuery](https://docs.cloud.google.com/bigquery/docs/bqml-introduction).