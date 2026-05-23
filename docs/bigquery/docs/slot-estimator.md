# Estimate slot capacity requirements

When you purchase reserved slots in BigQuery, you must estimate the
right number of slots for your particular workload. The BigQuery
slot estimator helps you to manage slot capacity based on historical performance
metrics.

You can use the slot estimator for your edition, reservation and on-demand
workloads to perform the following tasks:

For the selected [edition](https://docs.cloud.google.com/bigquery/docs/editions-intro) workloads:

- View slot capacity and utilization data for the past 30 days and identify periods of peak utilization when the most slots are used.
- View cost-optimal recommendations for commitment and autoscaling slots with similar performance.
- View your current reservation settings for a specific edition.

For specific reservation workloads:

- View slot capacity and utilization data for the past 30 days and identify periods of peak utilization when the most slots are used.
- View job latency percentiles (P90, P95, etc.) to understand query performance.
- Model how increasing or reducing max reservation slots might affect performance.

For on-demand billing workloads:

- View on-demand slot usage data of the entire organization or an individual project for the past 30 days.
- View cost-optimal recommendations for commitment and autoscaling slots with similar performance if you move to the Enterprise edition.

Customers who use Enterprise edition, Enterprise Plus edition,
or on-demand billing can use BigQuery slot recommender to view
slot usage, optimize commitments, and improve performance. For more
information, see [View edition slot
recommendations](https://docs.cloud.google.com/bigquery/docs/slot-recommender).

## Limitations

- Data is limited to the past 30 days.
- The models do not include [`ML_EXTERNAL`](https://docs.cloud.google.com/bigquery/docs/reservations-workload-management#assignments) assignments. If a large percentage of your slots are used for `ML_EXTERNAL` assignments, then the modeled results are less accurate.

## Before you begin

Grant Identity and Access Management (IAM) roles that give users the necessary permissions
to perform each task in this document.

### Required permissions

To use the slot estimator for reservations data, you need the following
IAM permissions on the administration project:

- `bigquery.reservations.list`
- `bigquery.reservationAssignments.list`
- `bigquery.capacityCommitments.list`

Each of the following predefined IAM roles includes the
permissions that you need in order to use the slot estimator:

- `roles/bigquery.admin`
- `roles/bigquery.resourceAdmin`
- `roles/bigquery.resourceEditor`
- `roles/bigquery.resourceViewer`
- `roles/bigquery.user`

To use the slot estimator for on-demand usage data, you need to [enable the
Reservations
API](https://docs.cloud.google.com/bigquery/docs/reservations-commitments#enabling-reservations-api) on a
project you intend to use as the administration project to manage reservations.
Other than the permissions above, you also need one of the following
IAM permissions on your organization to see organization-level
data or the project to see project-level data:

- `bigquery.jobs.listExecutionMetadata` (can only be applied on organization level)
- `bigquery.jobs.listAll` (can be applied on both organization or project level)

Each of the following predefined IAM roles includes the
permissions that you need in order to use the slot estimator:

- `roles/bigquery.admin`
- `roles/bigquery.resourceAdmin`
- `roles/bigquery.resourceEditor`
- `roles/bigquery.resourceViewer`

To view the commitment slots recommendations, you also need the permissions
described in [View edition slot
recommendations](https://docs.cloud.google.com/bigquery/docs/slot-recommender#required_permissions).

For more information about IAM roles in BigQuery,
see [Predefined roles and permissions](https://docs.cloud.google.com/bigquery/docs/access-control).

## View slot capacity and utilization

To view slot capacity and utilization over time, navigate to the slot estimator:

1. In the Google Cloud console, open the BigQuery page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. Select your administration project.

   1. Click the **Select from** drop-down list at the top of the page.
   2. In the **Select from window** that appears, select your project.
3. In the navigation menu, click **Capacity management**.

4. Click the **Slot estimator** tab.

The utilization chart shows slot capacity and utilization over the past 30 days,
calculated using hourly granularity.

The **Usage and utilization by percentage** tab shows slot utilization as a
percentage of slot usage by max slots.

![Utilization
percentage](https://docs.cloud.google.com/static/bigquery/images/slot-estimator-1.png)

The **Usage and utilization by capacity** tab shows max slots and usage as
absolute values.

![Usage and
capacity](https://docs.cloud.google.com/static/bigquery/images/slot-estimator-2.png)

You can choose an edition or on-demand option from the **Source** drop-down to
view statistics for different scopes. Selecting an edition populates the
**Reservation** drop-down with relevant reservations.

For on-demand options, you can choose either an individual project or the entire
organization from the
**Recommendations for** drop-down if you have organization-level permissions.
The Slot Estimator page only shows the project-level information if you only
have project-level permissions.

![Slot estimator on-demand
options](https://docs.cloud.google.com/static/bigquery/images/slot-estimator-on-demand-options.png)

The statistics for **Usage and utilization by capacity** tab may vary slightly
based on different scope:

- For edition source, it shows max slots available for the entire edition, commitment slots, sum of baseline slots, average slot usage, P99 slot usage and P50 slot usage.
- For specific reservation, it shows max reservation slots, baseline slots, average slot usage, P99 slot usage and P50 slot usage.
- For on-demand source, it shows average slot usage, P99 slot usage and P50 slot usage.

## Model slot performance

When a reservation is selected, you can use the slot estimator to view job
performance data and to model the effect of changing the number of max slots.
The slot estimator lets you model how performance might change at different
capacity levels, ranging from 80% of the minimum value of max slot size in the
observation period to 150% of the current max slots. In other words, the
decrease in options cannot exceed 20% of minimum capacity of the 30-day timeframe,
while the increase in options cannot exceed 50% of current capacity.

The models assume a replay of the previous 30 days' usage pattern, where
everything remains the same except for a change in slots.

The estimated performance improvement is based on several factors. The most
important factors are the number of slots in the model, and the proportion of
jobs in each percentile bucket that ran during peak periods versus regular
periods. Peak periods are defined as durations in which almost all slots were
used. Jobs running during these times are most impacted by slot contention, and
therefore see the most performance gain from additional slots. As a result,
different buckets of jobs can see different effects from the same capacity
increase, depending on when they are run.

> [!CAUTION]
> **Caution:** The actual impact on job performance can vary based on future usage. The estimated performance information is only for guidance.

To model slot performance, perform the following steps:

1. In the Google Cloud console, open the BigQuery page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. Select your administration project.

   1. Click the **Select from** drop-down list at the top of the page.
   2. In the **Select from window** that appears, select your project.
3. In the navigation menu, click **Capacity management**.

4. Click the **Slot estimator** tab.

5. In the **Reservation** drop-down, select a particular reservation. The model
   includes the number of idle slots that the reservation was able to borrow at
   any given time.

6. In the **Model with additional slots on max slots** drop-down, select one or
   more values of slots to model and click **OK**.

The table under **Changes in job performance with additional slots** shows job
performance data from the past 30 days, along with the estimated change in
performance from increasing or decreasing max slots. The data is grouped into
percentages by job duration for all jobs that ran in the selected timeframe.
The column denoted by the light bulb icon corresponds to the performance-enhancing
recommendation for the selected reservation.

![Slot modeling](https://docs.cloud.google.com/static/bigquery/images/slot-estimator-4.png)

The performance data is broken down by percentile. The table splits the data
into at most 12 buckets: P10 through P90, plus P95, P99, and P100. The P100
bucket represents the top 1% of jobs that took the longest time to run; P99
includes the top 96% to 99%; P95 includes the top 91% to 95%; P90 includes 81%
to 90%; and so forth. Depending on the data, the table may group the data into
fewer buckets. In that case, the table contains fewer rows.

For each percentile bucket, the table shows the following information:

- Job duration percentile: The percentile bucket for this row.
- Average job duration: The average time that jobs in that percentile bucket took to run.
- Number of jobs: The number of jobs in that percentile bucket.
- For each model, the estimated average duration for jobs in that percentile.

The table also lists an estimated "30-day change" statistic for each model. This
value is the estimated change in total hours spent processing the jobs in the
30-day history at different slot capacities.

## Understand the modeling results with slot usage

For fixed-capacity reservations, if idle slot sharing is enabled, then jobs in
that reservation can borrow idle slots from other reservations. As a result,
utilization can exceed 100% of allocated slots. If a reservation consistently
utilizes idle slots from other reservations, it suggests that the reservation
size might need to be increased. This is important because the workload's
performance could degrade if idle slot availability decreases in the future. On
the other hand, if a reservation seldom uses its full capacity, the reservation
might be too large.

Reservations that use [autoscaling](https://docs.cloud.google.com/bigquery/docs/slots-autoscaling-intro) use
and add slots in the following priority:

1. Baseline slots.
2. Idle slot sharing (if enabled).
3. Autoscale slots.

If an autoscaling reservation is consistently maxing out autoscaling slots, this
might be a signal to increase the max reservation slots. For information about
viewing your slot usage, see [View administrative resource
charts](https://docs.cloud.google.com/bigquery/docs/admin-resource-charts#view-admin-resource-charts).

## Pricing

You can use the slot estimator at no charge.