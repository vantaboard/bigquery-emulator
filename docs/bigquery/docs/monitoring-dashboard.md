# Create dashboards, charts, and alerts

This document describes how to create charts and alerts to monitor
BigQuery resources using Cloud Monitoring.

## Before you begin

Before you use Cloud Monitoring, ensure that you have the following:

- A Cloud Billing account.
- A BigQuery project with billing enabled.

One way to ensure that you have both is to complete the
[Quickstart using the Google Cloud console](https://docs.cloud.google.com/bigquery/docs/quickstarts/quickstart-web-ui).

## View and create dashboards, charts, and alerts

### View the Cloud Monitoring dashboard

To use Cloud Monitoring to monitor your BigQuery project:

1. In the Google Cloud console, go to the **Monitoring** page.

   [Go to Monitoring](https://console.cloud.google.com/monitoring)

   <br />

2. Select the name of your project if it is not already selected at the top of
   the page.

3. To view BigQuery resources, select **Dashboards \>
   BigQuery**. On this page you see a list of tables, events, and incident
   reporting that are user-configurable as well as charts of project metrics or
   dataset metrics.

   ![BigQuery dashboard.](https://docs.cloud.google.com/static/bigquery/images/stackdriver-dashboards-menu-bigquery.png)

### Visualize slots available and slots allocated

To visualize the slots available and slots allocated to your project, go to the
dashboard for BigQuery described in
[Viewing the Cloud Monitoring dashboard](https://docs.cloud.google.com/bigquery/docs/monitoring-dashboard#view-dashboards):

1. In the Google Cloud console, go to the **Monitoring** page.

   [Go to Monitoring](https://console.cloud.google.com/monitoring)

   <br />

2. Select **Dashboards \> BigQuery**.

3. On the Cloud Monitoring dashboard for BigQuery, scroll to
   the chart named **Slot Utilization**.

The **Slot Utilization** chart appears on both the main Cloud Monitoring
default dashboard and the Cloud Monitoring dashboard for
BigQuery.

### Create a dashboard and chart

Display the metrics collected by Cloud Monitoring in your own charts and
dashboards:

1. In the Google Cloud console, go to the **Monitoring** page.

   [Go to Monitoring](https://console.cloud.google.com/monitoring)

   <br />

2. Select **Dashboards \> Create Dashboard**.

3. Click **Add Chart**. You see the Add Chart page:

   ![Add chart page.](https://docs.cloud.google.com/static/bigquery/images/stackdriver-add-chart.png)
4. In the **Find resource type and metric** panel fields:

   - For the **Resource type** drop-down list, select **Global** . You might need to expand the list of **Resource types** for the **Global** option to be visible.
   - For the **Metric** drop-down list, select **Query execution time**.
5. The **Aggregation** pane fields control how the execution-time data are
   displayed. You can modify the default settings for these fields.

6. Click **Save**.

### View quota usage and limits

In Cloud Monitoring, you can view metrics for quota usage and limits:

1. In the Google Cloud console, go to the **Monitoring** page.

   [Go to Monitoring](https://console.cloud.google.com/monitoring)

   <br />

2. In the navigation pane, select
   **Metrics explorer**.

3. In the toolbar, select **Explorer \> Configuration**.

4. In the **Resource \& Metric** section, click **Select a metric**.

5. Select **Consumer Quota \> Quota \> Quota limit** , and then click **Apply**.

6. Click **Add filter** , and then in the **Label** menu, select **limit_name**.

7. In the **Value** menu, select the quota for which you want to view the metrics.

   ![Metrics explorer.](https://docs.cloud.google.com/static/bigquery/images/stackdriver-metrics-explorer.png)

> [!NOTE]
> **Note:** You can view metrics for quota usage and limits only for the [BigQuery Storage Write API's](https://docs.cloud.google.com/bigquery/quotas#write-api-limits) concurrent connections and throughput quotas.

### Create an alert

To create an alerting policy that triggers when the 99th percentile of the
execution time of a [BigQuery](https://docs.cloud.google.com/bigquery/docs) query
exceeds a user-defined limit, use the following settings.

#### Steps to create an [alerting policy](https://docs.cloud.google.com/monitoring/alerts/using-alerting-ui#create-policy).

To create an alerting policy, do the following:

1. In the Google Cloud console, go to the
   **Alerting** page:

   [Go to **Alerting**](https://console.cloud.google.com/monitoring/alerting)

   <br />

   If you use the search bar to find this page, then select the result whose subheading is
   **Monitoring**.
2. If you haven't created your notification channels and if you want to be notified, then click **Edit Notification Channels** and add your notification channels. Return to the **Alerting** page after you add your channels.
3. From the **Alerting** page, select **Create policy**.
4. To select the resource, metric, and filters, expand the **Select a metric** menu and then use the values in the **New condition** table:
   1. Optional: To limit the menu to relevant entries, enter the resource or metric name in the filter bar.
   2. Select a **Resource type** . For example, select **VM instance**.
   3. Select a **Metric category** . For example, select **instance**.
   4. Select a **Metric** . For example, select **CPU Utilization**.
   5. Select **Apply**.
5. Click **Next** and then configure the alerting policy trigger. To complete these fields, use the values in the **Configure alert trigger** table.
6. Click **Next**.
7. Optional: To add notifications to your alerting policy, click
   **Notification channels** . In the dialog, select one or more notification
   channels from the menu, and then click **OK**.

   To be notified when incidents are openend and closed, check
   **Notify on incident closure**. By default, notifications are sent only when
   incidents are openend.
8. Optional: Update the **Incident autoclose duration**. This field determines when Monitoring closes incidents in the absence of metric data.
9. Optional: Click **Documentation**, and then add any information that you want included in a notification message.
10. Click **Alert name** and enter a name for the alerting policy.
11. Click **Create Policy**.

| **New condition** Field | Value |
|---|---|
| **Resource and Metric** | In the **Resources** menu, select **BigQuery Project** . In the **Metric categories** menu, select **Query** . In the **Metrics** menu, select **Query execution times**. |
| **Filter** |   |
| **Across time series Time series group by** | `priority` |
| **Across time series Time series aggregation** | `99th percentile` |
| **Rolling window** | `5 m` |
| **Rolling window function** | `sum` |

| **Configure alert trigger** Field | Value |
|---|---|
| **Condition type** | `Threshold` |
| **Alert trigger** | `Any time series violates` |
| **Threshold position** | `Above threshold` |
| **Threshold value** | You determine this value; however, a threshold of 60 seconds is recommended. |
| **Retest window** | `most recent value` |

To create an alerting policy that triggers when the total scanned bytes billed for a
BigQuery project exceeds a user-defined limit, use the following
alerting policy configuration.

| **New condition** Field | Value |
|---|---|
| **Resource and Metric** | In the **Resources** menu, select **BigQuery Project** . In the **Metric categories** menu, select **Query** . In the **Metrics** menu, select **Statement scanned bytes billed**. |
| **Filter** | *(No filter needed for a project-wide alert)* |
| **Across time series Time series group by** | *(Leave blank to aggregate all series)* |
| **Across time series Time series aggregation** | `sum` |
| **Rolling window** | `5 m` |
| **Rolling window function** | `sum` |

| **Configure alert trigger** Field | Value |
|---|---|
| **Condition type** | `Threshold` |
| **Alert trigger** | `Any time series violates` |
| **Threshold position** | `Above threshold` |
| **Threshold value** | You determine this value. For example, to trigger an alert when usage exceeds 1 TiB, enter `1000000000000`. |

While the alerting policy monitors the total bytes scanned, you can set a threshold
based on a specific budget. To accomplish this budget based alerting policy, you
must first convert your desired cost threshold into the equivalent number of bytes.
The formula relies on the BigQuery on-demand compute pricing.
For more information, see [On-demand compute pricing](https://cloud.google.com/bigquery/pricing#on_demand_pricing).

You can use the following formula to convert your cost threshold into bytes:

    Threshold in Bytes = (Target Amount / (price per TiB)) * 1,000,000,000,000

#### Example: Trigger an alert when usage exceeds $100

Let's say you want to be alerted when your project's query costs exceed **$100**.

1. **Calculate the equivalent data volume in TiB:**   
   `$100 / (price per TiB) = Equivalent Data Volume in TiB`
2. **Convert data volume to bytes:**   
   `(Equivalent Data Volume in TiB) * 1,000,000,000,000 = Threshold Value in Bytes`
3. **Set the Threshold Value:**   
   In the **Configure alert trigger** section of your policy, enter the "Threshold Value in Bytes" as the **Threshold value**.

Now, your alerting policy triggers when the total scanned bytes billed in the
rolling window corresponds to approximately **$100** in on-demand query
costs.

## Metrics available for visualization

The following metrics are available, time delayed up to several hours.

| **Resource type** | **Name** | **Units** | **Description** |
|---|---|---|---|
| BigQuery | Scanned bytes | Bytes per minute | Number of bytes scanned. |
| BigQuery | Scanned bytes billed | Bytes per minute | Number of bytes sent for billing when using the on-demand analysis model. Scanned bytes and scanned bytes billed can differ as charges are rounded up, with a minimum amount of data processed per query. |
| BigQuery | BI Engine Query Fallback Count ([Preview](https://cloud.google.com/products/#product-launch-stages)) | Queries | The amount of queries that did not use BI Engine as a rate. You can set the **Group By** option to `reason` to separate the count into different fallback reasons, including: - `NO_RESERVATION` - `INSUFFICIENT_RESERVATION` - `UNSUPPORTED_SQL_TEXT` - `INPUT_TOO_LARGE` - `OTHER_REASON` |
| BigQuery | Query count | Queries | Queries in flight. |
| BigQuery | Query execution count ([Preview](https://cloud.google.com/products/#product-launch-stages)) | Queries | The number of queries executed. |
| BigQuery | Query execution times - 5th percentile - 50th percentile - 95th percentile - 99th percentile | Seconds | Non-cached query execution times. |
| BigQuery | Slots used by project | Slots | Number of BigQuery slots allocated for query jobs in the project. Slots are allocated per billing account and multiple projects can share the same reservation of slots. |
| BigQuery | Slots used by project and job type | Slots | Number of slots allocated to the project at any time separated by [job type](https://cloud.google.com/bigquery/docs/reference/rest/v2/jobs). This can also be thought of as the number of slots being utilized by that project. Load and extract jobs are free operations, and they run in a public pool of resources. Slots are allocated per billing account and multiple projects can share the same reservation of slots. |
| BigQuery | Slots used by project, reservation, and job type | Slots | Number of BigQuery slots allocated for project. Slot allocation can be broken down based on reservation and job type. |
| BigQuery | Total slots | Slots | Total number of slots available to the project. If the project shares a reservation of slots with other projects the slots being used by the other projects is not depicted. |
| BigQuery | Slots used across projects in reservations | Slots | Number of BigQuery slots allocated across projects in the reservation. Note that the metric data is only reported while at least one project has been assigned to the reservation and is consuming slots. As an alternative, consider querying reservations information from [`INFORMATION_SCHEMA`](https://docs.cloud.google.com/bigquery/docs/information-schema-reservations). To view slot usage metrics for all projects consuming from a reservation, you must explicitly add those consuming projects to the [metrics scope](https://docs.cloud.google.com/monitoring/settings) of the project where you are viewing the dashboard. |
| BigQuery | Slots used by project in reservation | Slots | Number of BigQuery slots allocated for project in the reservation. To view slot usage metrics for all projects consuming from a reservation, you must explicitly add those consuming projects to the [metrics scope](https://docs.cloud.google.com/monitoring/settings) of the project where you are viewing the dashboard. |
| BigQuery continuous job | Estimated backlog logical bytes | Bytes | The number of bytes in the backlog for each stage of the continuous job. |
| BigQuery continuous job | Estimated backlog records | Records | The estimated number of backlog records for each stage of the continuous job. |
| BigQuery continuous job | Estimated bytes processed | Bytes | The estimated number of bytes processed for each stage of the continuous job. |
| BigQuery continuous job | Output watermark | Timestamp | The most recent timestamp, in microseconds since the epoch, up to which all data has been processed by this stage of the continuous job. |
| BigQuery continuous job | Records read | Records | The number of input records read for each stage of the continuous job. |
| BigQuery continuous job | Records written | Records | The number of output records written for each stage of the continuous job. |
| BigQuery continuous job | Slots used | Slot milliseconds | The total slot milliseconds used by the continuous job. |
| BigQuery dataset | Stored bytes | Bytes | Bytes stored in the dataset - For the 100 largest tables in the dataset, bytes stored is displayed for each individual table (by name). Any additional tables in the dataset (beyond the 100 largest) are reported as single sum, and the table name for the summary is an empty string. |
| BigQuery dataset | Table count | Tables | Number of tables in the dataset. |
| BigQuery dataset | Uploaded bytes | Bytes per minute | Number of bytes uploaded to any table in the dataset. |
| BigQuery dataset | Uploaded rows | Rows per minute | Number of records uploaded to any table in the dataset. |

For a complete list of available Google Cloud metrics, see [Google Cloud metrics](https://docs.cloud.google.com/monitoring/api/metrics_gcp#gcp-bigquerybiengine).

## Known issues

- If no queries are running, then no data is returned for slots allocated, slots available, or any query-related variables. Zoom out to see data.

- If queries are running in both the US and the EU, then slots allocated and
  slots available might be incorrect.

- Slots allocated is reported as the average value in a time window (the width
  of the time window depends on the zoom level of the chart). Zooming in and
  out can change the value of slots allocated. Zooming in to a time window of
  1 hr or less shows the true values of slots allocated. At this range for any
  time visible on the chart, `avg(slots allocated) = slots allocated`.

- The data in Cloud Monitoring charts pertains only to the selected
  project.

- Metrics are instantaneous values, sampled at a point in time, and might miss
  data points between sample intervals. For example, the job count metric is
  sampled every minute. The value is the number of jobs at that particular
  time, not the maximum number of jobs throughout the entire minute.