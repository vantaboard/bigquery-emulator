# Metadata indexing for BigQuery tables

This document describes column metadata indexing in BigQuery and
explains how to allocate dedicated resources to improve index freshness and
query performance.

BigQuery automatically indexes metadata for
BigQuery tables exceeding 1 GiB. This metadata includes file
location, partitioning information, and column-level attributes, which
BigQuery uses to optimize and accelerate your queries.

By default, metadata indexing in BigQuery is a free background
operation and requires no action on your part. However, index freshness
depends on available free resources and doesn't have
performance service level objectives (SLOs). If index freshness is critical for
your use case, we recommend configuring a
[`BACKGROUND` reservation](https://docs.cloud.google.com/bigquery/docs/reservations-workload-management#assignments),
which shares resources across background optimization jobs.

## View the metadata index refresh time

To see the last metadata index refresh time of a table, query the
`LAST_METADATA_INDEX_REFRESH_TIME` column of the
[`INFORMATION_SCHEMA.TABLE_STORAGE` view](https://docs.cloud.google.com/bigquery/docs/information-schema-table-storage).
To do so, do the following:

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, enter the following statement:

       SELECT
         project_id,
         project_number,
         table_name,
         last_metadata_index_refresh_time
       FROM
         [PROJECT_ID.]region-REGION.INFORMATION_SCHEMA.TABLE_STORAGE;

   <br />

   Replace the following:
   - `PROJECT_ID`: the ID of your Google Cloud project. If not specified, the default project is used.
   - `REGION`: the [region](https://docs.cloud.google.com/bigquery/docs/locations) where the project is located---for example, `region-us`.
3. Click **Run**.

## View column metadata index usage

To view whether the column metadata index was used after a job completes, check
the
[`TableMetadataCacheUsage` property](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/Job#tablemetadatacacheusage)
of the [Job](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/Job) resource. If the
`unusedReason` field is empty (not populated), the column
metadata index was used. If it is populated, the accompanying `explanation`
field provides a reason why the column metadata index wasn't used.

You can also view column metadata index usage with the
`metadata_cache_statistics` field in the
[`INFORMATION_SCHEMA.JOBS` view](https://docs.cloud.google.com/bigquery/docs/information-schema-jobs).

For example, the following displays column metadata index usage for the `my-job`
job:

```googlesql
SELECT metadata_cache_statistics
FROM `region-US`.INFORMATION_SCHEMA.JOBS
WHERE job_id = 'my-job';
```

As another example, the following displays the number of jobs that used column
metadata index for the `my-table` table:

```googlesql
SELECT COUNT(*)
FROM
  `region-US`.INFORMATION_SCHEMA.JOBS,
  UNNEST(metadata_cache_statistics.table_metadata_cache_usage) AS stats
WHERE
  stats.table_reference.table_id='my-table' AND
  stats.table_reference.dataset_id='my-dataset' AND
  stats.table_reference.project_id='my-project' AND
  stats.unusedReason IS NULL;
```

## Set up dedicated indexing resources

To set up resources for metadata indexing updates in your project, you first
need to have a reservation assigned to your project. To do so, do the following:

1. [Create a `BACKGROUND` reservation](https://docs.cloud.google.com/bigquery/docs/reservations-tasks).
2. [Assign your project to the reservation](https://docs.cloud.google.com/bigquery/docs/reservations-assignments#assign_my_prod_project_to_prod_reservation).

After setting up your reservation, select one of the following methods to assign
slots to your metadata indexing job. By default, slots that you allocate in this
manner are shared with other jobs if the slots are idle. For more information,
see
[Idle slots](https://docs.cloud.google.com/bigquery/docs/slots#idle_slots).

### Console

1. In the Google Cloud console, go to the **Capacity Management** page.

   [Go to Capacity Management](https://console.cloud.google.com/bigquery/admin/reservations)
2. Click
   **Reservation actions \> Create assignment**.

3. Select your reservation project.

4. Set **Job Type** to **Background**.

5. Click **Create**.

### bq

Use the
[`bq mk` command](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#bq_mk).

```bash
bq mk \
  --project_id=ADMIN_PROJECT_ID \
  --location=LOCATION \
  --reservation_assignment \
  --reservation_id=RESERVATION_NAME \
  --assignee_id=PROJECT_ID \
  --job_type=BACKGROUND \
  --assignee_type=PROJECT
```

Replace the following:

- `ADMIN_PROJECT_ID`: the project ID of the [administration project](https://docs.cloud.google.com/bigquery/docs/reservations-workload-management#admin-project) that owns the reservation resource.
- `LOCATION`: the [location](https://docs.cloud.google.com/bigquery/docs/locations) of the reservation.
- `RESERVATION_NAME`: the name of the reservation.
- `PROJECT_ID`: the project ID to assign to this reservation.

### SQL

To assign a reservation to a project, use the
[`CREATE ASSIGNMENT` DDL statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_assignment_statement).

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery/)
2. In the query editor, enter the following statement:

   ```sql
   CREATE ASSIGNMENT
   ADMIN_PROJECT_ID.region-LOCATION.RESERVATION_NAME.ASSIGNMENT_ID
   OPTIONS (
     assignee = 'projects/PROJECT_ID',
     job_type = 'BACKGROUND');
   ```
   Replace the following:

   <br />

   - `ADMIN_PROJECT_ID`: the project ID of the [administration project](https://docs.cloud.google.com/bigquery/docs/reservations-workload-management#admin-project) that owns the reservation resource.
   - `LOCATION`: the [location](https://docs.cloud.google.com/bigquery/docs/locations) of the reservation.
   - `RESERVATION_NAME`: the name of the reservation.
   - `ASSIGNMENT_ID`: the ID of the assignment. The ID must be unique to the project and location, start and end with a lowercase letter or a number, and contain only lowercase letters, numbers, and dashes.
   - `PROJECT_ID`: the project ID containing the tables. This project is assigned to the reservation.
3. Click **Run**.

## View indexing job information

After you set up your dedicated indexing jobs, you can view information
about the indexing jobs with the
[`JOBS` view](https://docs.cloud.google.com/bigquery/docs/information-schema-jobs).
The following SQL sample shows the five most recent refresh jobs in
<var translate="no">PROJECT_NAME</var>.

```sql
SELECT *
FROM
  region-us.INFORMATION_SCHEMA.JOBS
WHERE
  project_id = 'PROJECT_NAME'
  AND SEARCH(job_id, '`metadata_cache_refresh`')
ORDER BY
  creation_time DESC
LIMIT 5;
```

Replace `PROJECT_NAME` with the name of the project
containing your metadata indexing jobs.

## Configure metadata indexing alerts

The Cloud Monitoring alerting process notifies you when your
BigQuery performance doesn't meet defined criteria. For more
information, see [Alerting overview](https://docs.cloud.google.com/monitoring/alerts). With metadata
indexing, you can configure alerts for slot usage and staleness.

### Slot usage alert

This alert notifies you when your background reservation exceeds a defined
percentage of its allocation. The default value is 95%. You can configure this
alert for a specific reservation or for every background reservation. When this
alert triggers, we recommend that you
[increase your reservation size](https://docs.cloud.google.com/bigquery/docs/reservations-tasks#update_reservations).

To configure this alert for every background reservation, do the following:

1. Set up a [Monitoring notification channel](https://docs.cloud.google.com/monitoring/support/notification-options#creating_channels) if you haven't already.
2. Go to the **Integrations** page.

   [Go to Integrations](https://console.cloud.google.com/monitoring/integrations)
3. Find the **BigQuery** integration and click **View details**.

4. In the **Alerts** tab, select
   **Slot Usage - Background Metadata Cache Slot Usage Too High**.

5. Optional: To customize this alert further, click
   **Show options \> Customize alert policy**.

6. For **Configure notifications**, select your notification channel.

7. Click **Create**.

### Staleness alert

This alert notifies you when the average column metadata index staleness
increases too much compared to the existing average. The default threshold is
if the average over 4 hours is more than double the previous average for more
than 30 minutes. When this alert triggers, we recommend that you
[increase your reservation size](https://docs.cloud.google.com/bigquery/docs/reservations-tasks#update_reservations)
or create a
[background reservation](https://docs.cloud.google.com/bigquery/docs/reservations-workload-management#assignments)
if you don't have one.

To configure this alert, do the following:

1. Set up a [Monitoring notification channel](https://docs.cloud.google.com/monitoring/support/notification-options#creating_channels) if you haven't already.
2. Go to the **Integrations** page.

   [Go to Integrations](https://console.cloud.google.com/monitoring/integrations)
3. Find the **BigQuery** integration and click **View details**.

4. In the **Alerts** tab, select
   **Column Metadata Index Staleness - Too Much Percent Increase**.

5. Optional: To customize this alert further, click
   **Show options \> Customize alert policy**.

6. For **Configure notifications**, select your notification channel.

7. Click **Create**.

## Limitations

Metadata query performance enhancements only apply to `SELECT`, `INSERT`, and
`CREATE TABLE AS SELECT` statements. Data manipulation language (DML) statements
won't see improvements from metadata indexing.

## What's next

- Learn how to see all jobs in your project with the [`JOBS` view](https://docs.cloud.google.com/bigquery/docs/information-schema-jobs).
- Learn how to [view slot capacity and utilization](https://docs.cloud.google.com/bigquery/docs/slot-estimator#view_slot_capacity_and_utilization).