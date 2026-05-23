# SESSIONS_BY_USER view

The `INFORMATION_SCHEMA.SESSIONS_BY_USER` view contains real-time
metadata about BigQuery sessions created by the current user in
the current project.

## Required permissions

To query the `INFORMATION_SCHEMA.SESSIONS_BY_USER` view, you need
the `bigquery.jobs.list` Identity and Access Management (IAM) permission for the project.
Each of the following predefined IAM roles includes the
required permission:

- Project Viewer
- BigQuery User

For more information about BigQuery permissions, see
[Access control with IAM](https://docs.cloud.google.com/bigquery/docs/access-control).

## Schema

When you query the `INFORMATION_SCHEMA.SESSIONS_BY_*` views, the query results
contain one row for each BigQuery session.

The `INFORMATION_SCHEMA.SESSIONS_BY_*` view has the following schema:

> [!NOTE]
> **Note:** The underlying data is partitioned by the `creation_time` column and clustered by `project_id` and `user_email`.

| Column name | Data type | Value |
|---|---|---|
| `creation_time` | `TIMESTAMP` | (*Partitioning column*) Creation time of this session. Partitioning is based on the UTC time of this timestamp. |
| `expiration_time` | `TIMESTAMP` | (*Partitioning column*) Expiration time of this session. Partitioning is based on the UTC time of this timestamp. |
| `is_active` | `BOOL` | Is the session is still active? `TRUE` if yes, otherwise `FALSE`. |
| `last_modified_time` | `TIMESTAMP` | (*Partitioning column*) Time when the session was last modified. Partitioning is based on the UTC time of this timestamp. |
| `project_id` | `STRING` | (*Clustering column*) ID of the project. |
| `project_number` | `INTEGER` | Number of the project. |
| `session_id` | `STRING` | ID of the session. For example, `bquxsession_1234`. |
| `user_email` | `STRING` | (*Clustering column*) Email address or service account of the user who ran the session. |

For stability, we recommend that you explicitly list columns in your information schema queries instead of
using a wildcard (`SELECT *`). Explicitly listing columns prevents queries from
breaking if the underlying schema changes.

## Data retention

This view contains currently running sessions and the history of sessions
completed in the past 180 days.

## Scope and syntax

Queries against this view must include a [region qualifier](https://docs.cloud.google.com/bigquery/docs/information-schema-intro#syntax).
If you do not specify a regional qualifier, metadata is retrieved from all
regions. The following table explains the region scope for this view:

| View name | Resource scope | Region scope |
|---|---|---|
| ``[PROJECT_ID.]`region-REGION`.INFORMATION_SCHEMA.SESSIONS_BY_USER`` | Sessions created by the current user in the specified project. | `REGION` |

Replace the following:

- Optional: `PROJECT_ID`: the ID of your Google Cloud project. If not specified, the default project is used.
- `REGION`: any [dataset region name](https://docs.cloud.google.com/bigquery/docs/locations). For example, `` `region-us` ``.

  <br />

  <br />

  > [!NOTE]
  > **Note:** You must use [a region qualifier](https://docs.cloud.google.com/bigquery/docs/information-schema-intro#region_qualifier) to query `INFORMATION_SCHEMA` views. The location of the query execution must match the region of the `INFORMATION_SCHEMA` view.

<br />

## Example

To run the query against a project other than your default project, add the
project ID in the following format:

```bash
`PROJECT_ID`.`region-REGION_NAME`.INFORMATION_SCHEMA.SESSIONS_BY_USER
```
For example, `` `myproject`.`region-us`.INFORMATION_SCHEMA.SESSIONS_BY_USER ``.

<br />

The following example lists sessions that were created by the current user:

```googlesql
SELECT
  session_id,
  creation_time
FROM
  `region-us`.INFORMATION_SCHEMA.SESSIONS_BY_USER
WHERE
  creation_time >= TIMESTAMP_SUB(CURRENT_TIMESTAMP(), INTERVAL 1 DAY)
ORDER BY
  creation_time DESC;
```

> [!NOTE]
> **Note:** `INFORMATION_SCHEMA` view names are case-sensitive.

The results should look like the following:

```
+---+
| session_id                                        | creation_time       |
+---+
| CgwKCmZhbGl1LXRlc3QQARokMGQ5YWWYzZmE0YjhkMDBm     | 2021-06-01 08:04:26 |
| CgwKCmZhbGl1LXRlc3QQARokMDAzYjI0OWQtZTczwZjA1NDc2 | 2021-05-31 22:43:02 |
+---+
```