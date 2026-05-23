# PROJECT_OPTIONS view

You can query the `INFORMATION_SCHEMA.PROJECT_OPTIONS` view to retrieve
real-time metadata about BigQuery project options. This view contains
configuration options that have been set at the project level. To view the
default values for a configuration option, see [configuration
settings](https://docs.cloud.google.com/bigquery/docs/default-configuration#configuration-settings).

## Required permissions

To get configuration options metadata, you need the following Identity and Access Management (IAM) permissions:

- `bigquery.config.get`

The following predefined IAM role includes the
permissions that you need in order to get project options metadata:

- `roles/bigquery.jobUser`

For more information about granular BigQuery permissions, see
[roles and permissions](https://docs.cloud.google.com/bigquery/docs/access-control).

## Schema

When you query the `INFORMATION_SCHEMA.PROJECT_OPTIONS` view, the query results contain
one row for each configuration option in a project that differs from the default value.

The `INFORMATION_SCHEMA.PROJECT_OPTIONS` view has the following schema:

| Column name | Data type | Value |
|---|---|---|
| `https://docs.cloud.google.com/bigquery/docs/information-schema-project-options#options_table` | `STRING` | Option ID for the specified configuration setting. |
| `project_id` | `STRING` | The ID of the project. |
| `project_number` | `INTEGER` | Number of the project. |
| `option_description` | `STRING` | The option description. |
| `option_type` | `STRING` | The data type of the `OPTION_VALUE`. |
| `option_value` | `STRING` | The current value of the option. |

##### Options table

| `option_name` | `option_type` | `option_value` |
|---|---|---|
| `default_time_zone` | `STRING` | The default time zone for this project. |
| `default_kms_key_name` | `STRING` | The default key name for this project. |
| `default_query_job_timeout_ms` | `STRING` | The default query timeout in milliseconds for this project. This also applies to [continuous queries](https://docs.cloud.google.com/bigquery/docs/continuous-queries-introduction). |
| `default_interactive_query_queue_timeout_ms` | `STRING` | The default timeout in milliseconds for queued interactive queries for this project. |
| `default_batch_query_queue_timeout_ms` | `STRING` | The default timeout in milliseconds for queued batch queries for this project. |

For stability, we recommend that you explicitly list columns in your information schema queries instead of
using a wildcard (`SELECT *`). Explicitly listing columns prevents queries from
breaking if the underlying schema changes.

## Data retention

This view contains currently running sessions and the history of sessions completed in the past 180 days.

## Scope and syntax

Queries against this view must have a [region qualifier](https://docs.cloud.google.com/bigquery/docs/information-schema-intro#syntax).

| View name | Resource scope | Region scope |
|---|---|---|
| `` `region-REGION`.INFORMATION_SCHEMA.PROJECT_OPTIONS `` | Configuration options within the specified project. | `REGION` |

Replace the following:

- `REGION`: any [dataset region name](https://docs.cloud.google.com/bigquery/docs/locations). For example, `region-us`.

<br />

## Examples

The following example retrieves the `OPTION_NAME`, `OPTION_TYPE`, and `OPTION_VALUE` columns from the `INFORMATION_SCHEMA.PROJECT_OPTIONS` view.

```googlesql
SELECT
  option_name, option_type, option_value
FROM
  `region-REGION`.INFORMATION_SCHEMA.PROJECT_OPTIONS;
```

> [!NOTE]
> **Note:** `INFORMATION_SCHEMA` view names are case-sensitive.

The result is similar to the following:

<br />

```
  +---+---+---+
  | option_name                                | option_type | option_value        |
  +---+---+---+
  | default_time_zone                          | STRING      | America/Los_Angeles |
  +---+---+---+
  | default_kms_key_name                       | STRING      | test/testkey1       |
  +---+---+---+
  | default_query_job_timeout_ms               | INT64       | 18000000            |
  +---+---+---+
  | default_interactive_query_queue_timeout_ms | INT64       | 600000              |
  +---+---+---+
  | default_batch_query_queue_timeout_ms       | INT64       | 1200000             |
  +---+---+---+
  
```

<br />