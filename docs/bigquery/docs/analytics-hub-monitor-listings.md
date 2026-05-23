# Monitor listings

This document describes how to monitor listings in BigQuery sharing
(formerly Analytics Hub).
As a data provider, you can track the usage metrics for your listings. There are
two methods to get the usage metrics for your shared data:

- [Use BigQuery sharing](https://docs.cloud.google.com/bigquery/docs/analytics-hub-monitor-listings#use-analytics-hub).
  You can use Sharing to view the usage metrics dashboard
  for your listings. This dashboard includes daily subscriptions, daily
  executed jobs, the number of subscribers for each organization, and job
  frequency for each table. You can retrieve the usage metrics for your
  shared data by querying the `INFORMATION_SCHEMA.SHARED_DATASET_USAGE` view.

- [Use the `INFORMATION_SCHEMA` view](https://docs.cloud.google.com/bigquery/docs/analytics-hub-monitor-listings#use-information-schema). You can track
  how subscribers use your datasets by querying the
  `INFORMATION_SCHEMA.SHARED_DATASET_USAGE` view.

## Use Sharing

To get usage metrics for your shared data using Sharing,
follow these steps:

1. In the Google Cloud console, go to the **Sharing (Analytics Hub)** page.

   [Go to Sharing (Analytics Hub)](https://console.cloud.google.com/bigquery/analytics-hub)
2. Click the name of the
   [data exchange](https://docs.cloud.google.com/bigquery/docs/analytics-hub-introduction#data_exchanges)
   that contains the listing to view its usage metrics.

3. Click **Usage metrics**, and then do the following:

   1. From the **Listings** menu, select the listing.

   2. Set the time range.

The page displays the following usage metrics:

- **Total Subscriptions**: the number of current subscriptions on the selected listing. You can view total subscriptions for up to 60 days.
- **Total Subscribers**: the number of unique subscribers across all subscriptions on the selected listing. You can view total subscribers for up to 60 days.
- **Total jobs executed**: the number of unique jobs run on each table of the selected listing.
- **Total bytes scanned**: the total number of bytes scanned from all tables of the selected listing.
- **Daily Subscriptions**: the chart that tracks the number of subscriptions for the selected listing over a time period. You can view daily subscriptions for up to 60 days.
- **Subscribers per organization**: lists the organizations and their number of subscribers that consume your selected listing.
- **Daily Executed Jobs**: this chart displays the jobs consumption from the selected listing.
- **Tables' job frequency**: the frequency at which the tables are accessed on the selected listing.

> [!NOTE]
> **Note:** You can also use the [BigQuery sharing subscriber APIs](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.subscriptions/list) to retrieve the **Total Subscriptions** , **Total Subscribers** , and **Daily
> Subscriptions** fields.

## Use `INFORMATION_SCHEMA` view

Data providers can track how subscribers use datasets by querying the
[`INFORMATION_SCHEMA.SHARED_DATASET_USAGE` view](https://docs.cloud.google.com/bigquery/docs/information-schema-shared-dataset-usage).
Ensure that you have the required role to query this view.

To run the query against a Google Cloud project other than your default project,
use the following format:

```
PROJECT_ID.region-REGION_NAME.INFORMATION_SCHEMA.SHARED_DATASET_USAGE
```

Replace the following:

- `PROJECT_ID`: the Google Cloud project ID
- `REGION_NAME`: the BigQuery dataset region name

For example, `myproject.region-us.INFORMATION_SCHEMA.SHARED_DATASET_USAGE`.

The following examples describe how to view the usage metrics by querying the
`INFORMATION_SCHEMA` view.

### Get the total number of jobs executed on all shared tables

The following example calculates total jobs run by [subscribers](https://docs.cloud.google.com/bigquery/docs/analytics-hub-view-subscribe-listings) for a project:

```googlesql
SELECT
  COUNT(DISTINCT job_id) AS num_jobs
FROM
  `region-us`.INFORMATION_SCHEMA.SHARED_DATASET_USAGE
```

The result is similar to the following:

```
+---+
| num_jobs   |
+---+
| 1000       |
+---+
```

To check the total jobs run by subscribers, use the `WHERE` clause:

- For datasets, use `WHERE dataset_id = "..."`.
- For tables, use `WHERE dataset_id = "..." AND table_id = "..."`.

### Get the most used table based on the number of rows processed

The following query calculates the most used table based on the number of rows
processed by subscribers.

```googlesql
SELECT
  dataset_id,
  table_id,
  SUM(num_rows_processed) AS usage_rows
FROM
  `region-us`.INFORMATION_SCHEMA.SHARED_DATASET_USAGE
GROUP BY
  1,
  2
ORDER BY
  3 DESC
LIMIT
  1
```

The output is similar to the following:

```
+---+---+---+
| dataset_id    | table_id      | usage_rows     |
+---+---+---+
| mydataset     | mytable     | 15             |
+---+---+---+
```

### Find the top organizations that consume your tables

The following query calculates the top subscribers based on the number of bytes
processed from your tables. You can also use the `num_rows_processed` column as
a metric.

```googlesql
SELECT
  subscriber_org_number,
  ANY_VALUE(subscriber_org_display_name) AS subscriber_org_display_name,
  SUM(total_bytes_processed) AS usage_bytes
FROM
  `region-us`.INFORMATION_SCHEMA.SHARED_DATASET_USAGE
GROUP BY
  1
```

The output is similar to the following:

```
+---+---+---+
|subscriber_org_number     | subscriber_org_display_name    | usage_bytes    |
+---+---+
| 12345                    | myorganization                 | 15             |
+---+---+---+
```

For subscribers without an organization, you can use `job_project_number`
instead of `subscriber_org_number`.

### Get usage metrics for your data exchange

If your [data exchange](https://docs.cloud.google.com/bigquery/docs/analytics-hub-introduction#data_exchanges)
and source dataset are in different projects, follow
these steps to view the usage metrics for your data exchange:

1. Find all [listings](https://docs.cloud.google.com/bigquery/docs/analytics-hub-introduction#listings) that belong to your data exchange.
2. Retrieve the source dataset attached to the listing.
3. To view the usage metrics for your data exchange, use the following query:

```googlesql
SELECT
  *
FROM
  source_project_1.`region-us`.INFORMATION_SCHEMA.SHARED_DATASET_USAGE
WHERE
  dataset_id='source_dataset_id'
AND data_exchange_id="projects/4/locations/us/dataExchanges/x1"
UNION ALL
SELECT
  *
FROM
  source_project_2.`region-us`.INFORMATION_SCHEMA.SHARED_DATASET_USAGE
WHERE
  dataset_id='source_dataset_id'
AND data_exchange_id="projects/4/locations/us/dataExchanges/x1"
```

### Get usage metrics for shared views

The following query displays the usage metrics for all of the shared views
present in a project:

```googlesql
SELECT
  project_id,
  dataset_id,
  table_id,
  num_rows_processed,
  total_bytes_processed,
  shared_resource_id,
  shared_resource_type,
  referenced_tables
FROM `myproject`.`region-us`.INFORMATION_SCHEMA.SHARED_DATASET_USAGE
WHERE shared_resource_type = 'VIEW'
```

The output is similar to the following:

```
+---+---+---+---+---+---+---+---+
|     project_id      |   dataset_id   | table_id | num_rows_processed | total_bytes_processed | shared_resource_id | shared_resource_type |                                                                                                              referenced_tables                                                                                                              |
+---+---+---+---+---+---+---+---+
|     myproject       | source_dataset | view1    |                  6 |                    38 | view1              | VIEW                 | [{"project_id":"myproject","dataset_id":"source_dataset","table_id":"test_table","processed_bytes":"21"},
{"project_id":"bq-dataexchange-exp","dataset_id":"other_dataset","table_id":"other_table","processed_bytes":"17"}]                 |

+---+---+---+---+---+---+---+---+
```

### Get usage metrics for shared table valued functions

The following query displays the usage metrics for all of the shared table
valued functions present in a project:

```googlesql
SELECT
  project_id,
  dataset_id,
  table_id,
  num_rows_processed,
  total_bytes_processed,
  shared_resource_id,
  shared_resource_type,
  referenced_tables
FROM `myproject`.`region-us`.INFORMATION_SCHEMA.SHARED_DATASET_USAGE
WHERE shared_resource_type = 'TABLE_VALUED_FUNCTION'
```

The output is similar to the following:

```
+---+---+---+---+---+---+---+---+
|     project_id      |   dataset_id   | table_id | num_rows_processed | total_bytes_processed | shared_resource_id | shared_resource_type  |                                                  referenced_tables                                                  |
+---+---+---+---+---+---+---+---+
|     myproject       | source_dataset |          |                  3 |                    45 | provider_exp       | TABLE_VALUED_FUNCTION | [{"project_id":"myproject","dataset_id":"source_dataset","table_id":"test_table","processed_bytes":"45"}]           |
+---+---+---+---+---+---+---+---+
```

## What's next

- Learn how to [manage BigQuery sharing listings](https://docs.cloud.google.com/bigquery/docs/analytics-hub-manage-listings).
- Learn about [BigQuery pricing](https://cloud.google.com/bigquery/pricing).