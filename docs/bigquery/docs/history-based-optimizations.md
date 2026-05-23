# Use history-based optimizations

This guide describes how to enable, disable, and analyze history-based
optimizations for queries.

## About history-based optimizations

History-based optimizations use information from already completed executions
of similar queries to apply additional optimizations and further improve
query performance such as slot time consumed and query latency. For example,
when you apply history-based optimization, the first query execution might take
60 seconds, but the second query execution might take only 30 seconds if a
history-based optimization was identified. This process continues until there
are no additional optimizations to add.

The following is an example of how history-based optimizations work with
BigQuery:

| Execution count | Query slot time consumed | Notes |
|---|---|---|
| 1 | 60 | Original execution. |
| 2 | 30 | First history-based optimization applied. |
| 3 | 20 | Second history-based optimization applied. |
| 4 | 21 | No additional history-based optimizations to apply. |
| 5 | 19 | No additional history-based optimizations to apply. |
| 6 | 20 | No additional history-based optimizations to apply. |

History-based optimizations are only applied when there is high confidence that
there will be a beneficial impact to the query performance. In addition, when an
optimization does not significantly improve query performance, that optimization
is revoked and not used in future executions of that query.

## Roles and permissions

- To enable or disable history-based optimizations, you must have the
  required permissions to create BigQuery
  default configurations, and then you must use the `ALTER PROJECT` statement
  to enable history-based optimizations. Once you've enabled
  history-based optimizations, all jobs in that project use
  history-based optimizations, regardless of which user created the job.
  To learn more about required roles and permissions for default
  configurations, see [Required roles](https://docs.cloud.google.com/bigquery/docs/default-configuration#required_permissions)
  for default configurations. To enable history-based optimizations, see
  [Enable history-based optimizations](https://docs.cloud.google.com/bigquery/docs/history-based-optimizations#enable-history-based-optimization).

- To review the history-based optimizations for a job using the
  `INFORMATION_SCHEMA.JOBS` view, you must have the required role. For more
  information, see [Required role](https://docs.cloud.google.com/bigquery/docs/information-schema-jobs#required_role)
  for `INFORMATION_SCHEMA.JOBS` view.

## Enable history-based optimizations

History-based optimizations are enabled by default. If history-based
optimizations have been disabled for a project or organization, you can manually
re-enable history-based optimizations by including the
`default_query_optimizer_options = 'adaptive=on'` parameter in your
[`ALTER PROJECT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#alter_project_set_options_statement)
or
[`ALTER ORGANIZATION`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#alter_organization_set_options_statement)
statement. For example:

    ALTER PROJECT PROJECT_NAME
    SET OPTIONS (
      `region-`LOCATION`.default_query_optimizer_options` = 'adaptive=on'
    );

Replace the following:

- `PROJECT_NAME`: the name of the project
- `LOCATION`: the location in which jobs should attempt to use history-based optimizations

> [!NOTE]
> **Note:** After you enable history-based optimizations, you can safely ignore the following cautionary message, which is shown for any successful `ALTER PROJECT` or `ALTER ORGANIZATION` statement: `ALTER PROJECT succeeded. Please make sure no existing queries depend on the
> old defaults (such as the default time zone) or else these queries will be
> broken.`

## Disable history-based optimizations

To disable history-based optimizations in a project, include the
`default_query_optimizer_options = 'adaptive=off'` parameter in the
[`ALTER PROJECT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#alter_project_set_options_statement)
or
[`ALTER ORGANIZATION`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#alter_organization_set_options_statement)
statement. For example:

    ALTER PROJECT PROJECT_NAME
    SET OPTIONS (
      `region-`LOCATION`.default_query_optimizer_options` = 'adaptive=off'
    );

Replace the following:

- `PROJECT_NAME`: the name of the project
- `LOCATION`: the location in which jobs should not attempt to use history-based optimizations

> [!NOTE]
> **Note:** After you disable history-based optimizations, you can safely ignore the following cautionary message, which is shown for any successful `ALTER PROJECT` or `ALTER ORGANIZATION` statement: `ALTER PROJECT succeeded. Please make sure no existing queries depend on the
> old defaults (such as the default time zone) or else these queries will be
> broken.`

## Review history-based optimizations for a job

To review the history-based optimizations for a job, you can use a SQL query
or a REST API method call.

### SQL

You can use a query to get the history-based optimizations for a job.
The query must include [`INFORMATION_SCHEMA.JOBS_BY_PROJECT`](https://docs.cloud.google.com/bigquery/docs/information-schema-jobs#schema)
and the `query_info.optimization_details` column name.

In the following example, the optimization details are returned for a job
called `sample_job`. If no history-based optimizations were applied, `NULL` is
produced for `optimization_details`:

    SELECT
      job_id,
      query_info.optimization_details
    FROM `PROJECT_NAME.region-LOCATION`.INFORMATION_SCHEMA.JOBS_BY_PROJECT
    WHERE job_id = 'sample_job'
    LIMIT 1;

The results look similar to the following:

    -- The JSON in optimization_details has been formatted for readability.
    /*---+---*
     | job_id     | optimization_details                                            |
     +---+---+
     | sample_job | {                                                               |
     |            |   "optimizations": [                                            |
     |            |     {                                                           |
     |            |       "semi_join_reduction": "web_sales.web_date,RIGHT"         |
     |            |     },                                                          |
     |            |     {                                                           |
     |            |       "semi_join_reduction": "catalog_sales.catalog_date,RIGHT" |
     |            |     },                                                          |
     |            |     {                                                           |
     |            |       "semi_join_reduction": "store_sales.store_date,RIGHT"     |
     |            |     },
     |            |     {                                                           |
     |            |       "join_commutation": "web_returns.web_item"                |
     |            |     },
     |            |     {                                                           |
     |            |       "parallelism_adjustment": "applied"                       |
     |            |     },
     |            |   ]                                                             |
     |            | }                                                               |
     *---+---*/

### API

To get the optimization details for a job, you can call the
[`jobs.get` method](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs/get).

In the following example, the `jobs.get` method returns the optimization details
([`optimizationDetails`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/Job#queryinfo))
in the full response:

    {
      "jobReference": {
        "projectId": "myProject",
        "jobId": "sample_job"
      }
    }

The results look similar to the following:

    -- The unrelated parts in the full response have been removed.
    {
      "jobReference": {
        "projectId": "myProject",
        "jobId": "sample_job",
        "location": "US"
      },
      "statistics": {
        "query": {
          "queryInfo": {
            "optimizationDetails": {
              "optimizations": [
                {
                  "semi_join_reduction": "web_sales.web_date,RIGHT"
                },
                {
                  "semi_join_reduction": "catalog_sales.catalog_date,RIGHT"
                },
                {
                  "semi_join_reduction": "store_sales.store_date,RIGHT"
                },
                {
                  "join_commutation": "web_returns.web_item"
                },
                {
                  "parallelism_adjustment": "applied"
                }
              ]
            }
          }
        }
      }
    }

## Estimate impact of history-based optimizations

To estimate the impact of history-based optimizations, you can use the
following sample SQL query to identify project queries with the greatest
estimated improvement to execution time.

      WITH
        jobs AS (
          SELECT
            *,
            query_info.query_hashes.normalized_literals AS query_hash,
            TIMESTAMP_DIFF(end_time, start_time, MILLISECOND) AS elapsed_ms,
            IFNULL(
              ARRAY_LENGTH(JSON_QUERY_ARRAY(query_info.optimization_details.optimizations)) > 0,
              FALSE)
              AS has_history_based_optimization,
          FROM region-LOCATION.INFORMATION_SCHEMA.JOBS_BY_PROJECT
          WHERE EXTRACT(DATE FROM creation_time) > DATE_SUB(CURRENT_DATE(), INTERVAL 30 DAY)
        ),
        most_recent_jobs_without_history_based_optimizations AS (
          SELECT *
          FROM jobs
          WHERE NOT has_history_based_optimization
          QUALIFY ROW_NUMBER() OVER (PARTITION BY query_hash ORDER BY end_time DESC) = 1
        )
      SELECT
        job.job_id,
        100 * SAFE_DIVIDE(
          original_job.elapsed_ms - job.elapsed_ms,
          original_job.elapsed_ms) AS percent_execution_time_saved,
        job.elapsed_ms AS new_elapsed_ms,
        original_job.elapsed_ms AS original_elapsed_ms,
      FROM jobs AS job
      INNER JOIN most_recent_jobs_without_history_based_optimizations AS original_job
        USING (query_hash)
      WHERE
        job.has_history_based_optimization
        AND original_job.end_time < job.start_time
      ORDER BY percent_execution_time_saved DESC
      LIMIT 10;

The result of the preceding query is similar to the following if history-based
optimizations were applied:

      /*---+---+---+---*
       |    job_id    | percent_execution_time_saved | new_execution_ms | original_execution_ms |
       +---+---+---+---+
       | sample_job1  |           67.806850186245114 |             7087 |                 22014 |
       | sample_job2  |           66.485800412501987 |            10562 |                 31515 |
       | sample_job3  |           63.285605271764254 |            97668 |                266021 |
       | sample_job4  |           61.134141726887904 |           923384 |               2375823 |
       | sample_job5  |           55.381272089713754 |          1060062 |               2375823 |
       | sample_job6  |           45.396943168036479 |          2324071 |               4256302 |
       | sample_job7  |           38.227031526376024 |            17811 |                 28833 |
       | sample_job8  |           33.826608962725111 |            66360 |                100282 |
       | sample_job9  |           32.087813758311604 |            44020 |                 64819 |
       | sample_job10 |           28.356416319483539 |            19088 |                 26643 |
       *---+---+---+---*/

The results of this query is only an estimation of history-based optimization
impact. Many factors can influence query performance, including but not limited
to slot availability, change in data over time, view or UDF definitions, and
differences in query parameter values.

If the result of this sample query is empty, then either no jobs have used
history-based optimizations, or all queries were optimized more than 30 days
ago.

This query can be applied to other query performance metrics such as
`total_slot_ms` and `total_bytes_billed`. For more information, see the schema
for [`INFORMATION_SCHEMA.JOBS`](https://docs.cloud.google.com/bigquery/docs/information-schema-jobs#schema).