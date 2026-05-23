# Set up alerts with scheduled queries

This document describes how to set up an alert using a BigQuery
scheduled query. This approach supports custom use cases defined by query logic.

## Before you begin

Before you use Cloud Monitoring, ensure that you have the following:

- A Cloud Billing account.
- A BigQuery project with billing enabled.

One way to ensure that you have both is to complete the
[Quickstart using the Google Cloud console](https://docs.cloud.google.com/bigquery/docs/quickstarts/query-public-dataset-console#before-you-begin).

## Create a SQL query

Create and run a SQL query in BigQuery that generates the
output for your alert. The query captures the logic that you want to monitor.
For more information, see [Run a query](https://docs.cloud.google.com/bigquery/docs/running-queries).

## Set up a scheduled query

You can schedule queries to run on a recurring basis, from every 15 minutes
to every several months. You can write any query over your log buckets. For more
information, see [Scheduling queries](https://docs.cloud.google.com/bigquery/docs/scheduling-queries).

### Understand the row count metric

Scheduled queries automatically create a metric in Cloud Monitoring. This
metric records the number of rows that the SQL query returned during its last
evaluation. You can create an alert policy that monitors this row count metric
in Monitoring.

The following [metric](https://docs.cloud.google.com/monitoring/api/metrics_gcp_a_b#gcp-bigquerydatatransfer)
is a gauge that contains the most recent row count of a scheduled query:

`bigquerydatatransfer.googleapis.com/transfer_config/last_execution_job_rows`

All scheduled queries write their row count to this metric, using different labels. You need the `config_id` label when you define an alert policy. To find the `config_id` label, follow these steps:

1. In Google Cloud console, go to the **Scheduled queries** page:

   [Go to Scheduled queries](https://console.cloud.google.com/bigquery/scheduled-queries)
2. Click the scheduled query for which you want to create an alert.

3. Go to the **Details** tab.

4. Check the last string in **Resource name**, as shown in the following
   screenshot:

   ![config_id in resource name.](https://docs.cloud.google.com/static/bigquery/images/scheduled-query-config-id.png)

> [!NOTE]
> **Note:** The last known value for the row count repeats continuously for 5 weeks. If you deactivate a scheduled query or the query fails, the metric remains constant at its last known value for 35 days. After 35 days, the metric disappears.

If a problem occurs with the scheduled query, an error message appears in the
**Run history** tab of your scheduled query.

### Monitor scheduled queries

Monitor scheduled queries to ensure successful executions:

- Look for errors in the **Run history** tab of the scheduled query.
- Check the final status of each scheduled execution that is stored in the `completion_state` field of the [`bigquerydatatransfer.googleapis.com/transfer_config/completed_runs` metric](https://docs.cloud.google.com/monitoring/api/metrics_gcp_a_b#gcp-bigquerydatatransfer).
- Look for errors in the [BigQuery Data Transfer Service logs](https://docs.cloud.google.com/bigquery/docs/dts-monitor#logs).

## Create an alert policy

Use a [metric-threshold alert](https://docs.cloud.google.com/monitoring/alerts/using-alerting-ui) to detect
when the number of rows returned by the scheduled query differs from a threshold.

To set up an alert on the number of rows a scheduled query returns, follow these
steps:

1. In the Google Cloud console, go to the
   **Alerting** page:

   [Go to **Alerting**](https://console.cloud.google.com/monitoring/alerting)

   <br />

   If you use the search bar to find this page, then select the result whose subheading is
   **Monitoring**.
2. Click **Create policy**.

3. Select the row count metric for the scheduled query. In the
   **Select a metric** menu, click **BigQuery DTS Config \> Transfer_config \> Last executed job row count**.

4. In **Add filters** , click **Add a filter**.

5. In the **Filter** menu, select **config_id**.

6. In the **Value** menu, select the `config_id` of the scheduled query for
   which you want to create an alert:

   ![Set the config_id filter.](https://docs.cloud.google.com/static/bigquery/images/config-id-filter.png)

   If you don't set a filter, your alert tests the output of every scheduled
   query. To find the `config_id` of your scheduled query, see
   [Understand the row count metric](https://docs.cloud.google.com/bigquery/docs/create-alert-scheduled-query#understand_the_row_count_metric).
7. Keep the default **Transform data** settings and click **Next**.

8. For **Condition types** , select **Threshold**.

9. Select the condition that you want. For example, to trigger when the query
   returns any rows, set the following condition:

   1. For **Alert trigger** , select **Any time series violates**.
   2. For **Threshold position** , select **Above threshold**.
   3. In **Threshold value** , enter `0`.
10. Keep the default **Advanced Options** and click **Next**.

11. Optional: To configure notifications for your alert, click the
    **Use notification channel** toggle, and then set channels and subject line
    for your notifications. You can also set notifications for incident closure.

    If you don't want notifications, deselect the **Use notification channel**
    toggle.
12. Optional: If you have many alert policies, you can [annotate labels](https://docs.cloud.google.com/monitoring/alerts/labels)
    on them to indicate that they are derived from scheduled queries.

13. Optional: In the **Documentation** field, you can add links that help to
    interpret the alert. For example, you can add a link to the **Logs Explorer**
    page with a similar query, so that you can explore the data that raised the
    alert. You can also link to the specific schedule query's details page.

14. In **Name the alert policy**, enter a name for your alert.

15. Click **Create Policy**.

## Limitations

Alert policies for scheduled queries are subject to the following limitations:

- Scheduled query execution frequency and ingestion delay impact the total time from log emission to alert. For example, if your query runs every 30 minutes and you add a 15-minute lag for ingestion delay, your alert fires approximately 15 minutes after an offending log entry is emitted. In some cases, it might take up to 45 minutes.
- The configuration between a scheduled query and an alert policy isn't linked or synchronized. Editing the configuration in one place might break the relationship that enables the alert function.

## What's next

- Learn how to create and run [scheduled queries](https://docs.cloud.google.com/bigquery/docs/scheduling-queries).