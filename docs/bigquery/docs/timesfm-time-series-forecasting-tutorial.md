This tutorial teaches you how to use the
[`AI.FORECAST` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-forecast)
with BigQuery ML's built-in
[TimesFM univariate model](https://docs.cloud.google.com/bigquery/docs/timesfm-model) to forecast the future value
for a given column, based on the historical value of that column.

This tutorial uses data from the public
[`bigquery-public-data.san_francisco_bikeshare.bikeshare_trips`](https://console.cloud.google.com/bigquery?p=bigquery-public-data&d=san_francisco_bikeshare&t=bikeshare_trips&page=table)
table.

## Objectives

This tutorial guides you through using the AI.FORECAST function with the built-in TimesFM model to forecast bike share trips. The first two sections cover how to forecast and visualize results for a single time series. The third section covers how to forecast for multiple time series.

## Costs

This tutorial uses billable components of Google Cloud, including the following:

- BigQuery
- BigQuery ML

For more information about BigQuery costs, see the
[BigQuery pricing](https://cloud.google.com/bigquery/pricing) page.

For more information about BigQuery ML costs, see
[BigQuery ML pricing](https://cloud.google.com/bigquery/pricing#bqml).

## Before you begin

1. BigQuery is automatically enabled in new projects. To activate BigQuery in a pre-existing project,


   Enable the BigQuery API.


   **Roles required to enable APIs**


   To enable APIs, you need the Service Usage Admin IAM
   role (`roles/serviceusage.serviceUsageAdmin`), which
   contains the `serviceusage.services.enable` permission. [Learn how to grant
   roles](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).

   [Enable the API](https://console.cloud.google.com/flows/enableapi?apiid=bigquery)

## Forecast a single bike share trips time series

Forecast future time series values by using the `AI.FORECAST`
function.

The following query forecasts the number of subscriber bike share trips per
hour for the next month (approximately 720 hours), based on the previous four
months of historical data. The `confidence_level` argument indicates that the
query generates a prediction interval with a 95% confidence level.

Follow these steps to forecast data with the TimesFM model:

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, paste in the following query and click **Run**:

   ```googlesql
   SELECT *
   FROM
     AI.FORECAST(
       (
         SELECT TIMESTAMP_TRUNC(start_date, HOUR) as trip_hour, COUNT(*) as num_trips
   FROM `bigquery-public-data.san_francisco_bikeshare.bikeshare_trips`
   WHERE subscriber_type = 'Subscriber' AND start_date >= TIMESTAMP('2018-01-01')
   GROUP BY TIMESTAMP_TRUNC(start_date, HOUR)
       ),
       horizon => 720,
       confidence_level => 0.95,
       timestamp_col => 'trip_hour',
       data_col => 'num_trips');
   ```

   The results look similar to the following:

   ```
   +---+---+---+---+---+---+
   | forecast_timestamp      | forecast_value    | confidence_level | prediction_interval_lower_bound | prediction_interval_upper_bound | ai_forecast_status |
   +---+---+---+---+---+---+
   | 2018-05-01 00:00:00 UTC | 26.3045959...     |            0.95  | 21.7088378...                   | 30.9003540...                   |                    |
   +---+---+---+---+---+---+
   | 2018-05-01 01:00:00 UTC | 34.0890502...     |            0.95  | 2.47682913...                   | 65.7012714...                   |                    |
   +---+---+---+---+---+---+
   | 2018-05-01 02:00:00 UTC | 24.2154693...     |            0.95  | 2.87621605...                   | 45.5547226...                   |                    |
   +---+---+---+---+---+---+
   | ...                     | ...               |  ...             | ...                             |  ...                            |                    |
   +---+---+---+---+---+---+
   ```

## Compare the forecasted data to the input data

Chart the `AI.FORECAST` function output alongside a subset of the function
input data to see how they compare.

Follow these steps to chart the function output:

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, paste in the following query and click **Run**:

   ```googlesql
   SELECT *
   FROM
     AI.FORECAST(
       (
         SELECT TIMESTAMP_TRUNC(start_date, HOUR) as trip_hour, COUNT(*) as num_trips
         FROM `bigquery-public-data.san_francisco_bikeshare.bikeshare_trips`
         WHERE subscriber_type = 'Subscriber' AND start_date >= TIMESTAMP('2018-01-01')
         GROUP BY TIMESTAMP_TRUNC(start_date, HOUR)
       ),
       horizon => 720,
       confidence_level => 0.95,
       timestamp_col => 'trip_hour',
       data_col => 'num_trips',
       output_historical_time_series => true);
   ```
3. When the query is finished running, click the **Visualization** tab in the
   **Query results** pane. For **Visualization type** , select **Line** . For **Dimension** , select `time_series_timestamp`. For **Measures** , select `time_series_data`, `prediction_interval_lower_bound`, and `prediction_interval_upper_bound`. The resulting chart looks similar to the following:

   ![Graph 100 time points of input data along with the AI.FORECAST function output data to evaluate their similarity.](https://docs.cloud.google.com/static/bigquery/images/ai-forecast-output-comparison.png)

   You can see that the input data and the forecasted data show similar
   bike share usage. You can also see that the prediction interval lower and
   upper bounds increase as the forecasted time points get further into the
   future.

## Forecast multiple bike share trips time series

The following query forecasts the number of bike share trips per subscriber type
and per hour for the next month (approximately 720 hours), based on the previous four
months of historical data. The `confidence_level` argument indicates that the
query generates a prediction interval with a 95% confidence level.

Follow these steps to forecast data with the TimesFM model:

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, paste in the following query and click **Run**:

   ```googlesql
   SELECT *
   FROM
     AI.FORECAST(
       (
         SELECT TIMESTAMP_TRUNC(start_date, HOUR) as trip_hour, subscriber_type, COUNT(*) as num_trips
         FROM `bigquery-public-data.san_francisco_bikeshare.bikeshare_trips`
         WHERE start_date >= TIMESTAMP('2018-01-01')
         GROUP BY TIMESTAMP_TRUNC(start_date, HOUR), subscriber_type
       ),
       horizon => 720,
       confidence_level => 0.95,
       timestamp_col => 'trip_hour',
       data_col => 'num_trips',
       id_cols => ['subscriber_type']);
   ```

   The results look similar to the following:

   ```
   +---+---+---+---+---+---+---+
   | subscriber_type     | forecast_timestamp       | forecast_value   | confidence_level | prediction_interval_lower_bound | prediction_interval_upper_bound | ai_forecast_status |
   +---+---+---+---+---+---+---+
   | Subscriber          | 2018-05-01 00:00:00 UTC  | 26.3045959...    |            0.95  | 21.7088378...                   | 30.9003540...                   |                    |
   +---+---+---+---+---+---+---+
   | Subscriber          |  2018-05-01 01:00:00 UTC | 34.0890502...    |            0.95  | 2.47682913...                   | 65.7012714...                   |                    |
   +---+---+---+---+---+---+---+
   | Subscriber          |  2018-05-01 02:00:00 UTC | 24.2154693...    |            0.95  | 2.87621605...                   | 45.5547226...                   |                    |
   +---+---+---+---+---+---+---+
   | ...                 | ...                      |  ...             | ...              | ...                             |  ...                            |                    |
   +---+---+---+---+---+---+---+
   ```

## Clean up


To avoid incurring charges to your Google Cloud account for the resources used in this
tutorial, either delete the project that contains the resources, or keep the project and
delete the individual resources.

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

- For an overview of BigQuery ML, see [Introduction to AI and ML in BigQuery](https://docs.cloud.google.com/bigquery/docs/bqml-introduction).