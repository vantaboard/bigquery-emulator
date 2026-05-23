# Monitor continuous queries

You can monitor BigQuery
[continuous queries](https://docs.cloud.google.com/bigquery/docs/continuous-queries-introduction) by using
the following BigQuery tools:

- [`INFORMATION_SCHEMA` views](https://docs.cloud.google.com/bigquery/docs/information-schema-intro)
- [Query execution graphs](https://docs.cloud.google.com/bigquery/docs/query-insights#view_query_performance_insights)
- [Job history](https://docs.cloud.google.com/bigquery/docs/managing-jobs#view-job)
- [Administrative jobs explorer](https://docs.cloud.google.com/bigquery/docs/admin-jobs-explorer)

Due to the long running nature of a BigQuery continuous query,
metrics that are usually generated upon the completion of a SQL query might be
absent or inaccurate.

## Use `INFORMATION_SCHEMA` views

You can use a number of the `INFORMATION_SCHEMA` views to monitor continuous
queries and continuous query reservations.

> [!NOTE]
> **Note:** Data for a running continuous query is only retained for two days by the `JOBS* INFORMATION_SCHEMA` views.

### View job details

You can use the
[`JOBS`](https://docs.cloud.google.com/bigquery/docs/information-schema-jobs) view to get
continuous query job metadata.

The following query returns the metadata for all active continuous queries. The
metadata includes the output watermark timestamp, which represents the point up
to which the continuous query has successfully processed data.

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, run the following query:

   ```googlesql
   SELECT
     start_time,
     job_id,
     user_email,
     query,
     state,
     reservation_id,
     continuous_query_info.output_watermark
   FROM `PROJECT_ID.region-REGION.INFORMATION_SCHEMA.JOBS`
   WHERE
     creation_time > TIMESTAMP_SUB(CURRENT_TIMESTAMP(), INTERVAL 7 day)
     AND continuous IS TRUE
     AND state = "RUNNING"
   ORDER BY
     start_time DESC
   ```

   Replace the following:
   - `PROJECT_ID`: the ID of the project.
   - `REGION`: any [dataset region name](https://docs.cloud.google.com/bigquery/docs/locations). For example, `region-us`.

### View reservation assignment details

You can use the
[`ASSIGNMENTS`](https://docs.cloud.google.com/bigquery/docs/information-schema-assignments)
and
[`RESERVATIONS`](https://docs.cloud.google.com/bigquery/docs/information-schema-reservations) views to get
continuous query reservation assignment details.

Return reservation assignment details for continuous queries:

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, run the following query:

   ```googlesql
   SELECT
     reservation.reservation_name,
     reservation.slot_capacity
   FROM
     `ADMIN_PROJECT_ID.region-LOCATION.INFORMATION_SCHEMA.ASSIGNMENTS`
       AS assignment
   INNER JOIN
     `ADMIN_PROJECT_ID.region-LOCATION.INFORMATION_SCHEMA.RESERVATIONS`
       AS reservation
     ON (assignment.reservation_name = reservation.reservation_name)
   WHERE
     assignment.assignee_id = 'PROJECT_ID'
     AND job_type = 'CONTINUOUS';
   ```

   Replace the following:
   - `ADMIN_PROJECT_ID`: the ID of the [administration project](https://docs.cloud.google.com/bigquery/docs/reservations-workload-management#admin-project) that owns the reservation.
   - `LOCATION`: the location of the reservation.
   - `PROJECT_ID`: the ID of the project that is assigned to the reservation. Only information about continuous queries running in this project is returned.

### View slot consumption information

You can use the
[`ASSIGNMENTS`](https://docs.cloud.google.com/bigquery/docs/information-schema-assignments),
[`RESERVATIONS`](https://docs.cloud.google.com/bigquery/docs/information-schema-reservations), and
[`JOBS_TIMELINE`](https://docs.cloud.google.com/bigquery/docs/information-schema-jobs-timeline) views to get
continuous query slot consumption information.

Return slot consumption information for continuous queries:

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, run the following query:

   ```googlesql
   SELECT
     jobs.period_start,
     reservation.reservation_name,
     reservation.slot_capacity,
     SUM(jobs.period_slot_ms) / 1000 AS consumed_total_slots
   FROM
     `ADMIN_PROJECT_ID.region-LOCATION.INFORMATION_SCHEMA.ASSIGNMENTS`
       AS assignment
   INNER JOIN
     `ADMIN_PROJECT_ID.region-LOCATION.INFORMATION_SCHEMA.RESERVATIONS`
       AS reservation
     ON (assignment.reservation_name = reservation.reservation_name)
   INNER JOIN
     `PROJECT_ID.region-LOCATION.INFORMATION_SCHEMA.JOBS_TIMELINE` AS jobs
     ON (
       UPPER(CONCAT('ADMIN_PROJECT_ID:LOCATION.', assignment.reservation_name))
       = UPPER(jobs.reservation_id))
   WHERE
     assignment.assignee_id = 'PROJECT_ID'
     AND assignment.job_type = 'CONTINUOUS'
     AND jobs.period_start
       BETWEEN TIMESTAMP_SUB(CURRENT_TIMESTAMP(), INTERVAL 1 DAY)
       AND CURRENT_TIMESTAMP()
   GROUP BY 1, 2, 3
   ORDER BY jobs.period_start DESC;
   ```

   Replace the following:
   - `ADMIN_PROJECT_ID`: the ID of the [administration project](https://docs.cloud.google.com/bigquery/docs/reservations-workload-management#admin-project) that owns the reservation.
   - `LOCATION`: the location of the reservation.
   - `PROJECT_ID`: the ID of the project that is assigned to the reservation. Only information about continuous queries running in this project is returned.

You can also monitor continuous query reservations using other tools such as
[Metrics Explorer](https://docs.cloud.google.com/monitoring/charts/metrics-explorer) and
[administrative resource charts](https://docs.cloud.google.com/bigquery/docs/admin-resource-charts#view-resource-utilization).
For more information, see
[Monitor BigQuery reservations](https://docs.cloud.google.com/bigquery/docs/reservations-monitoring).

## Use the query execution graph

You can use the
[query execution graph](https://docs.cloud.google.com/bigquery/docs/query-insights#view_query_performance_insights)
to get performance insights and general statistics for a continuous query.
For more information, see
[View query performance insights](https://docs.cloud.google.com/bigquery/docs/query-insights#view_query_performance_insights).

## View job history

You can view continuous query job details in your personal job history or the
project's job history. For more information, see
[View job details](https://docs.cloud.google.com/bigquery/docs/managing-jobs#view-job).

Be aware that the historical list of jobs is sorted by the
job start time, so continuous queries that have been running for a while
might not be close to the start of the list.

## Use the administrative jobs explorer

In the administrative jobs explorer,
[filter your jobs](https://docs.cloud.google.com/bigquery/docs/admin-jobs-explorer#filter-jobs) to show
continuous queries by setting the **Job category** filter to
**Continuous query**.

## Use Cloud Monitoring

You can view metrics specific to BigQuery continuous queries by
using Cloud Monitoring. For more information, see
[Create dashboards, charts, and alerts](https://docs.cloud.google.com/bigquery/docs/monitoring-dashboard)
and read about the [metrics available for visualization](https://docs.cloud.google.com/bigquery/docs/monitoring-dashboard#metrics).

## Alert on failed queries

Instead of routinely checking whether your continuous queries have failed, it
can be helpful to create an alert to notify you of
failure. One way to do this is to create a custom
[Cloud Logging log-based metric](https://docs.cloud.google.com/logging/docs/logs-based-metrics/counter-metrics)
with a filter for your jobs, and a
[Cloud Monitoring alerting policy](https://docs.cloud.google.com/monitoring/alerts/using-alerting-ui)
based on that metric:

1. When you create a continuous query, use a [custom job ID prefix](https://docs.cloud.google.com/bigquery/docs/continuous-queries#custom-job-id). Multiple continuous queries can share the same prefix. For example, you might use the prefix `prod-` to indicate a production query.
2. In the Google Cloud console, go to the **Log-based Metrics** page.

   [Go to Log-based Metrics](https://console.cloud.google.com/logs/metrics)
3. Click **Create metric** . The **Create logs metric** panel appears.

4. For **Metric type** , select **Counter**.

5. In the **Details** section, give your metric a name. For example,
   `CUSTOM_JOB_ID_PREFIX-metric`.

6. In the **Filter selection** section, enter the following into the
   **Build filter** editor:

   ```none
   resource.type = "bigquery_project"
   protoPayload.resourceName : "projects/PROJECT_ID/jobs/CUSTOM_JOB_ID_PREFIX"
   severity = ERROR
   ```

   Replace the following:
   - `PROJECT_ID`: the name of your project.
   - `CUSTOM_JOB_ID_PREFIX`: the name of the [custom job ID prefix](https://docs.cloud.google.com/bigquery/docs/continuous-queries#custom-job-id) that you set for your continuous query.
7. Click **Create metric**.

8. In the navigation menu, click **Log-based metrics**. The metric you just
   created appears in the list of user-defined metrics.

9. In your metric's row, click
   **More actions** , and then click **Create alert from metric**.

10. Click **Next** . You don't need to change the default settings on the
    **Policy configuration mode** page.

11. Click **Next** . You don't need to change the default settings on the
    **Configure alert trigger** page.

12. Select your notification channels and enter a name for the alert policy.

13. Click **Create policy**.

You can test your alert by running a continuous query with the custom
job ID prefix that you selected and then cancelling it. It might take a few
minutes for the alert to reach your notification channel.

## Retry failed queries

Retrying a failed continuous query might help avoid situations where a
continuous pipeline is down for an extended period of time or requires human
intervention to restart. Important things to consider when you
retry a failed continuous query include the following:

- Whether reprocessing some amount of data processed by the previous query before it failed is tolerable.
- How to handle limiting retries or using exponential backoff.

One possible approach to automating query retry is the following:

1. Create a [Cloud Logging sink](https://docs.cloud.google.com/logging/docs/export/configure_export_v2#creating_sink)
   based on an inclusion filter matching the following criteria to route logs
   to a Pub/Sub topic:

   ```none
   resource.type = "bigquery_project"
   protoPayload.resourceName : "projects/PROJECT_ID/jobs/CUSTOM_JOB_ID_PREFIX"
   severity = ERROR
   ```

   Replace the following:
   - `PROJECT_ID`: the name of your project.
   - `CUSTOM_JOB_ID_PREFIX`: the name of the [custom job ID prefix](https://docs.cloud.google.com/bigquery/docs/continuous-queries#custom-job-id) that you set for your continuous query.
2. Create a [Cloud Run function](https://docs.cloud.google.com/functions/docs/calling) that is
   triggered in response to the Pub/Sub receiving logs matching
   your filter.

   The Cloud Run function could accept the data payload from the
   Pub/Sub message and attempt to start a new continuous query
   using the same SQL syntax as the failed query, but at beginning just
   after the previous job stopped.

For example, you can use a function similar to the following:

### Python


Before trying this sample, follow the Python setup instructions in the
[BigQuery quickstart using
client libraries](https://docs.cloud.google.com/bigquery/docs/quickstarts/quickstart-client-libraries).


For more information, see the
[BigQuery Python API
reference documentation](https://docs.cloud.google.com/python/docs/reference/bigquery/latest).


To authenticate to BigQuery, set up Application Default Credentials.
For more information, see

[Set up authentication for client libraries](https://docs.cloud.google.com/bigquery/docs/authentication#client-libs).

    import base64
    import json
    import logging
    import re
    import uuid

    import google.auth
    import google.auth.transport.requests
    import requests


    def retry_continuous_query(event, context):
        logging.info("Cloud Function started.")

        if "data" not in event:
            logging.info("No data in Pub/Sub message.")
            return

        try:
            # Decode and parse the Pub/Sub message data
            log_entry = json.loads(base64.b64decode(event["data"]).decode("utf-8"))

            # Extract the SQL query and other necessary data
            proto_payload = log_entry.get("protoPayload", {})
            metadata = proto_payload.get("metadata", {})
            job_change = metadata.get("jobChange", {})
            job = job_change.get("job", {})
            job_config = job.get("jobConfig", {})
            query_config = job_config.get("queryConfig", {})
            sql_query = query_config.get("query")
            job_stats = job.get("jobStats", {})
            end_timestamp = job_stats.get("endTime")
            failed_job_id = job.get("jobName")

            # Check if required fields are missing
            if not all([sql_query, failed_job_id, end_timestamp]):
                logging.error("Required fields missing from log entry.")
                return

            logging.info(f"Retrying failed job: {failed_job_id}")

            # Adjust the timestamp in the SQL query
            timestamp_match = re.search(
                r"\s*TIMESTAMP\(('.*?')\)(\s*\+ INTERVAL 1 MICROSECOND)?", sql_query
            )

            if timestamp_match:
                original_timestamp = timestamp_match.group(1)
                new_timestamp = f"'{end_timestamp}'"
                sql_query = sql_query.replace(original_timestamp, new_timestamp)
            elif "CURRENT_TIMESTAMP() - INTERVAL 10 MINUTE" in sql_query:
                new_timestamp = f"TIMESTAMP('{end_timestamp}') + INTERVAL 1 MICROSECOND"
                sql_query = sql_query.replace(
                    "CURRENT_TIMESTAMP() - INTERVAL 10 MINUTE", new_timestamp
                )

            # Get access token
            credentials, project = google.auth.default(
                scopes=["https://www.googleapis.com/auth/cloud-platform"]
            )
            request = google.auth.transport.requests.Request()
            credentials.refresh(request)
            access_token = credentials.token

            # API endpoint
            url = f"https://bigquery.googleapis.com/bigquery/v2/projects/{project}/jobs"

            # Request headers
            headers = {
                "Authorization": f"Bearer {access_token}",
                "Content-Type": "application/json",
            }

            # Generate a random UUID
            random_suffix = str(uuid.uuid4())[:8]  # Take the first 8 characters of the UUID

            # Combine the prefix and random suffix
            job_id = f"CUSTOM_JOB_ID_PREFIX{random_suffix}"

            # Request payload
            data = {
                "configuration": {
                    "query": {
                        "query": sql_query,
                        "useLegacySql": False,
                        "continuous": True,
                        "connectionProperties": [
                            {"key": "service_account", "value": "SERVICE_ACCOUNT"}
                        ],
                        # ... other query parameters ...
                    },
                    "labels": {"bqux_job_id_prefix": "CUSTOM_JOB_ID_PREFIX"},
                },
                "jobReference": {
                    "projectId": project,
                    "jobId": job_id,  # Use the generated job ID here
                },
            }

            # Make the API request
            response = requests.post(url, headers=headers, json=data)

            # Handle the response
            if response.status_code == 200:
                logging.info("Query job successfully created.")
            else:
                logging.error(f"Error creating query job: {response.text}")

        except Exception as e:
            logging.error(
                f"Error processing log entry or retrying query: {e}", exc_info=True
            )

        logging.info("Cloud Function finished.")

<br />

## What's next

- Learn how to create and run [continuous queries](https://docs.cloud.google.com/bigquery/docs/continuous-queries).