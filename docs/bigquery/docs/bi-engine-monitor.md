# Monitor BI Engine

[BigQuery BI Engine](https://docs.cloud.google.com/bigquery/docs/bi-engine-intro) accelerates
BigQuery for BI scenarios using memory cache and faster execution.
Acceleration details can be monitored using
[INFORMATION_SCHEMA](https://docs.cloud.google.com/bigquery/docs/information-schema-jobs) and
[Cloud Monitoring metrics](https://docs.cloud.google.com/bigquery/docs/monitoring).

## Cloud Monitoring

You can monitor and configure alerts for BigQuery BI Engine with
Cloud Monitoring. To learn how to create dashboard for
BI Engine metrics, see [Creating charts](https://docs.cloud.google.com/monitoring/charts).

The following metrics are provided for BigQuery BI Engine:

| Resource | Metric | Details |
|---|---|---|
| BigQuery Project | Reservation Total Bytes | Total capacity allocated to one Google Cloud project |
| BigQuery Project | Reservation Used Bytes | Total capacity used in one Google Cloud project |
| BigQuery Project | BI Engine Top Tables Cached Bytes | Cache usage per table. This metric displays the top *N* tables per region report usage. |

## Query statistics for BI Engine

This section explains how to find query statistics to help monitor, diagnose,
and troubleshoot BI Engine use.

### BI Engine acceleration modes

With BI Engine acceleration enabled, your query can run in any one
of these four modes:

|---|---|
| ``` BI_ENGINE_DISABLED ``` | BI Engine disabled the acceleration. `biEngineReasons` specifies a more detailed reason. The query was run using the BigQuery execution engine. |
| ``` PARTIAL_INPUT ``` | Part of the query input was accelerated using BI Engine. As described in [Query optimization and acceleration](https://docs.cloud.google.com/bigquery/docs/bi-engine-query#query_optimization_and_acceleration), a query plan is generally broken down into multiple input stages. BI Engine supports the common types of subquery patterns that are typically used in dashboarding. If the query consists of multiple input stages, only a few of which fall under the supported use cases, then BI Engine runs the unsupported stages using the normal BigQuery engine without acceleration. In this situation, BI Engine returns a `PARTIAL` acceleration code, and uses `biEngineReasons` to populate the reason for not accelerating other input stages. |
| ``` FULL_INPUT ``` | All input stages of the query were accelerated using BI Engine. Cached data is reused across queries, and the computation that follows immediately after reading the data is accelerated. |
| ``` FULL_QUERY ``` | The entire query was accelerated using BI Engine. |

### View BigQuery API job statistics

Detailed statistics on BI Engine are available through the
BigQuery API.

To fetch the statistics associated with BI Engine accelerated
queries, run the following bq command-line tool command:

    bq show --format=prettyjson -j job_id

If the project is enabled for BI Engine acceleration, then the
output produces a new field, `biEngineStatistics`. Here is a sample job
report:

     "statistics": {
        "creationTime": "1602175128902",
        "endTime": "1602175130700",
        "query": {
          "biEngineStatistics": {
            "biEngineMode": "DISABLED",
            "biEngineReasons": [
              {
                "code": "UNSUPPORTED_SQL_TEXT",
                "message": "Detected unsupported join type"
              }
            ]
          },

For more information about the `BiEngineStatistics` field, see the
[Job reference](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/Job#bienginestatistics).

### BigQuery information schema statistics

BI Engine acceleration statistics are included in the
[BigQuery `INFORMATION_SCHEMA`](https://docs.cloud.google.com/bigquery/docs/information-schema-intro)
views as part of the `INFORMATION_SCHEMA.JOBS_BY_*` views in the
[`bi_engine_statistics`](https://docs.cloud.google.com/bigquery/docs/information-schema-jobs#schema) column.
For example, this query returns the `bi_engine_statistics` for all of the
current projects' jobs for the last 24 hours:

    SELECT
      creation_time,
      job_id,
      bi_engine_statistics
    FROM
      `region-us`.INFORMATION_SCHEMA.JOBS_BY_PROJECT
    WHERE
      creation_time >
         TIMESTAMP_SUB(CURRENT_TIMESTAMP(), INTERVAL 1 DAY)
      AND job_type = "QUERY"

Use the following format to specify
[regionality](https://docs.cloud.google.com/bigquery/docs/information-schema-views#scope_and_syntax) for the
`project-id`, `region`, and `views` in the `INFORMATION_SCHEMA` view:

```bash
`PROJECT_ID`.`region-REGION_NAME`.INFORMATION_SCHEMA.VIEW
```

<br />

> [!IMPORTANT]
> **Logging slot metrics:** Although slot metrics for BI Engine are reported, accelerated BI Engine input stages are not counted towards slot reservations. For more information, see the [pricing](https://docs.cloud.google.com/bigquery/pricing#bi-engine-pricing) page.

### View Data Studio information schema details

You can track which Data Studio reports and data sources are
used by BigQuery by viewing the [`INFORMATION_SCHEMA.JOBS`
view](https://docs.cloud.google.com/bigquery/docs/information-schema-jobs). Every
Data Studio query in BigQuery creates an entry
with `report_id` and `datasource_id` labels. Those IDs appear at the end of
Data Studio URL when opening a report or data source page.
For example, a report with URL
`https://lookerstudio.google.com/navigation/reporting/my-report-id-123` has a
report ID of `"my-report-id-123"`.

The following examples show how to view reports and data sources:

#### Find report and data source URL for each Data Studio BigQuery job

```googlesql
-- Standard labels used by Data Studio.
DECLARE requestor_key STRING DEFAULT 'requestor';
DECLARE requestor_value STRING DEFAULT 'looker_studio';

CREATE TEMP FUNCTION GetLabel(labels ANY TYPE, label_key STRING)
AS (
  (SELECT l.value FROM UNNEST(labels) l WHERE l.key = label_key)
);

CREATE TEMP FUNCTION GetDatasourceUrl(labels ANY TYPE)
AS (
  CONCAT("https://lookerstudio.google.com/datasources/", GetLabel(labels, 'looker_studio_datasource_id'))
);

CREATE TEMP FUNCTION GetReportUrl(labels ANY TYPE)
AS (
  CONCAT("https://lookerstudio.google.com/reporting/", GetLabel(labels, 'looker_studio_report_id'))
);

SELECT
  job_id,
  GetDatasourceUrl(labels) AS datasource_url,
  GetReportUrl(labels) AS report_url,
FROM
  `region-us`.INFORMATION_SCHEMA.JOBS jobs
WHERE
  creation_time > TIMESTAMP_SUB(CURRENT_TIMESTAMP(), INTERVAL 7 DAY)
  AND GetLabel(labels, requestor_key) = requestor_value
LIMIT
  100;
```

#### View jobs produced by using a report and data source

```googlesql
-- Specify report and data source id, which can be found at the end of Data Studio URLs.
DECLARE user_report_id STRING DEFAULT '*report id here*';
DECLARE user_datasource_id STRING DEFAULT '*datasource id here*';

-- Data Studio labels for BigQuery.
DECLARE requestor_key STRING DEFAULT 'requestor';
DECLARE requestor_value STRING DEFAULT 'looker_studio';
DECLARE datasource_key STRING DEFAULT 'looker_studio_datasource_id';
DECLARE report_key STRING DEFAULT 'looker_studio_report_id';

CREATE TEMP FUNCTION GetLabel(labels ANY TYPE, label_key STRING)
AS (
  (SELECT l.value FROM UNNEST(labels) l WHERE l.key = label_key)
);

SELECT
  creation_time,
  job_id,
FROM
  `region-us`.INFORMATION_SCHEMA.JOBS jobs
WHERE
  creation_time > TIMESTAMP_SUB(CURRENT_TIMESTAMP(), INTERVAL 7 DAY)
  AND GetLabel(labels, requestor_key) = requestor_value
  AND GetLabel(labels, datasource_key) = user_datasource_id
  AND GetLabel(labels, report_key) = user_report_id
ORDER BY 1
LIMIT 100;
```

## Cloud Logging

BI Engine acceleration is part of BigQuery job
processing. To inspect BigQuery jobs for a specific project,
see the [Cloud Logging](https://console.cloud.google.com/logs/query) page with a payload of
`protoPayload.serviceName="bigquery.googleapis.com"`.

## What's next

- Learn more about [Cloud Monitoring](https://docs.cloud.google.com/monitoring/docs).
- Learn more about Monitoring [charts](https://docs.cloud.google.com/monitoring/charts).
- Learn more about Monitoring [alerts](https://docs.cloud.google.com/monitoring/alerts).
- Learn more about [Cloud Logging](https://docs.cloud.google.com/logging/docs).