# Writing queries in sessions

This document describes how to write queries in a BigQuery session.
It is intended for users who already have a general understanding of
BigQuery [sessions](https://docs.cloud.google.com/bigquery/docs/sessions-intro) and know how
to [run queries in a session](https://docs.cloud.google.com/bigquery/docs/sessions#run-queries).

A session stores state. State created in a session is maintained and usable
throughout the entire session. So, if you create a temporary table in one
query entry, you can use that temporary table in other query entries for the
rest of the session.

A session includes support for [session variables](https://docs.cloud.google.com/bigquery/docs/sessions-write-queries#session_variables),
[session system variables](https://docs.cloud.google.com/bigquery/docs/sessions-write-queries#session_system_variables),
[multi-statement queries](https://docs.cloud.google.com/bigquery/docs/sessions-write-queries#session_scripting), and
[multi-statement transactions](https://docs.cloud.google.com/bigquery/docs/sessions-write-queries#multi_transactions).

Before you complete these steps, ensure you have the necessary
[permissions](https://docs.cloud.google.com/bigquery/docs/sessions-intro#roles_and_permissions) to
work in a session.

## Use system variables in a session

You can set or retrieve session-level data with the following
[system variables](https://docs.cloud.google.com/bigquery/docs/reference/system-variables):

- `@@dataset_id`: The ID of the default dataset in the current project. The system variables `@@dataset_project_id` and `@@dataset_id` can be set and used together.
- `@@dataset_project_id`: The ID of the default project for datasets that are used in the query. If this system variable is not set, or if it is set to `NULL`, the query-executing project is used. The system variables `@@dataset_project_id` and `@@dataset_id` can be set and used together.
- `@@query_label`: The [job label](https://docs.cloud.google.com/bigquery/docs/adding-labels#job-label) to assign to the session. The label can be used throughout the entire session, not just for a specific query in the session.
- `@@session_id`: The ID of the current session.
- `@@time_zone`: The default time zone to use in time zone-dependent SQL functions, when a time zone is not specified as an argument.

These system variables can be used at any time during the session and are in
scope for the remaining session. You don't define these variables, but they can
be assigned a new value with the
[`SET`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/procedural-language#set) statement.

The maximum size of a [variable](https://docs.cloud.google.com/bigquery/docs/multi-statement-queries#variables)
in a session is 1 MB, and the maximum size of all variables in a session is
10 MB.

## Assign a label to a session

You can [assign a job label to a session](https://docs.cloud.google.com/bigquery/docs/adding-labels#adding-label-to-session).
When you do this, all future queries in the session are assigned to the label.
Labels can be used at any time during the session and are in scope for the
remaining session. The job label that you assign will appear in
[audit logs](https://docs.cloud.google.com/bigquery/docs/reference/auditlogs).

## Use variables in a session

You can create, set, and retrieve session-level data with
[variables](https://docs.cloud.google.com/bigquery/docs/multi-statement-queries#variables).
Variables can be used at any time during the session and are in scope for the
remaining session.

- To create a session-scoped variable, use the [`DECLARE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/procedural-language#declare) statement outside of a [`BEGIN...END`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/procedural-language#begin) block.
- To set a session-scoped variable after it has been created, use the [`SET`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/procedural-language#set) statement.
- A variable declared inside of a `BEGIN...END` block is not a session-scoped variable.
- A session-scoped variable can be referenced inside of a `BEGIN...END` block.
- A session-scoped variable can be set inside of a `BEGIN...END` block.

The maximum size of a variable
in a session is 1 MB, and the maximum size of all variables in a session is
10 MB.

## Use temporary tables in sessions

A temporary table lets you save intermediate results to a table. A
temporary table is visible at the session level, so you don't need to save or
maintain it in a dataset. It is automatically deleted after a
session terminates. You are charged for storage of temporary tables while the
session is active. For more information, see
[Use temporary tables in a multi-statement query](https://docs.cloud.google.com/bigquery/docs/multi-statement-queries#temporary_tables).

## Use temporary functions in sessions

A [temporary function](https://docs.cloud.google.com/bigquery/docs/user-defined-functions) or
[temporary aggregate function](https://docs.cloud.google.com/bigquery/docs/user-defined-aggregates#create-temp-sql-udaf)
is visible at the
session level, so you don't need to save or
maintain it in a dataset. It is automatically deleted after a
session terminates.

## Work with multi-statement queries in sessions

You can use
[GoogleSQL multi-statement queries](https://docs.cloud.google.com/bigquery/docs/multi-statement-queries#write_a_multi_statement_query)
in a session. A script can include temporary tables and system variables for
each script. Session variables and temporary tables are visible to scripts.
All top-level variables declared in a script are also session variables.

## Run multi-query multi-statement transactions in sessions

You can run multi-statement transactions over multiple queries in a session.
For example:

The following query begins a transaction.

    BEGIN TRANSACTION

Inside of the transaction, the following query creates a temporary table called
`Flights` and then returns the data in this table. Two statements are included
in the query.

    CREATE TEMP TABLE Flights(total INT64)  AS SELECT * FROM UNNEST([10,23,3,14,55]) AS a;

    SELECT * FROM Flights;

The following query commits the transaction.

    COMMIT

You can find an active transaction that affects the `Flights` table:

    WITH running_transactions AS (
      SELECT DISTINCT transaction_id
      FROM
        `region-us`.INFORMATION_SCHEMA.JOBS_BY_PROJECT
        WHERE creation_time > TIMESTAMP_SUB(CURRENT_TIMESTAMP(), INTERVAL 7 DAY)
        EXCEPT DISTINCT
        SELECT transaction_id FROM `region-us`.INFORMATION_SCHEMA.JOBS_BY_PROJECT
        WHERE
          creation_time > TIMESTAMP_SUB(CURRENT_TIMESTAMP(), INTERVAL 7 DAY)
          AND statement_type = "COMMIT_TRANSACTION"
          OR statement_type = "ROLLBACK_TRANSACTION"
    )
    SELECT
      jobs.transaction_id AS transaction_id,
      project_id,
      user_email,
      session_info.session_id,
      query
    FROM `region-us`.INFORMATION_SCHEMA.JOBS_BY_PROJECT AS jobs, running_transactions
      WHERE
      creation_time > TIMESTAMP_SUB(CURRENT_TIMESTAMP(), INTERVAL 7 DAY)
      AND destination_table = ("Flights")
      AND jobs.transaction_id = running_transactions.transaction_id;

If you want to cancel an ongoing transaction and you have the `bigquery.admin`
role, you can [issue a rollback statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/procedural-language#rollback_transaction),
using the session ID associated with the transaction in the Cloud Shell
or with an API call. When you [run the query](https://docs.cloud.google.com/bigquery/docs/sessions#run-queries),
using the session ID associated with the transaction, the session ID is shown in
the results.

## Example session

This is an example of the session workflow in the Google Cloud console:

1. In the Google Cloud console, open a new editor tab and
   [create a session](https://docs.cloud.google.com/bigquery/docs/sessions#create-session).

2. In the editor tab, add the following query:

       CREATE TEMP TABLE Flights(total INT64)  AS SELECT * FROM UNNEST([10,23,3,14,55]) AS a;
       SELECT * FROM Flights;

3. Run the query. A temporary table called `Flights` is created and all of the
   data is returned.

       +---+
       | total |
       +---+
       |    55 |
       |    23 |
       |     3 |
       |    14 |
       |    10 |
       +---+

4. Delete the content inside the editor tab and add the following query:

       SELECT * FROM Flights LIMIT 2;

5. Run the query. The results for two records are returned. Even though you
   deleted the earlier query, the information from the query is stored in
   the current session.

       +---+
       | total |
       +---+
       |    55 |
       |    23 |
       +---+

6. Delete the content inside the editor tab and add the following query:

       DECLARE x INT64 DEFAULT 10;

       SELECT total * x AS total_a FROM Flights LIMIT 2;

       BEGIN
         SET x = 100;
         SELECT total * x AS total_b FROM Flights LIMIT 2;
       END;

       SELECT total * x AS total_c FROM Flights LIMIT 2;

7. Run the query. The session-scoped variable `x` is used to limit the number
   of results returned for the `Flights` table. Look closely at how scoping
   affects this variable when it is declared outside of a `BEGIN...END`
   statement, set inside of a `BEGIN...END` statement, and then referenced
   outside of the `BEGIN...END` statement again.

       +---+
       | total_a |
       +---+
       |     550 |
       |     230 |
       +---+

       +---+
       | total_b |
       +---+
       |    5500 |
       |    2300 |
       +---+

       +---+
       | total_c |
       +---+
       |    5500 |
       |    2300 |
       +---+

8. Delete the content inside the editor tab and add the following query:

       SELECT STRING(TIMESTAMP "2008-12-20 15:30:00+00", @@time_zone) AS default_time_zone;

       SET @@time_zone = "America/Los_Angeles";

       SELECT STRING(TIMESTAMP "2008-12-20 15:30:00+00", @@time_zone) AS new_time_zone;

9. Run the query. The session-scoped system variable `@@time_zone` is used to
   assign a time zone to a timestamp. The first statement returns a
   timestamp with the default time zone (in this example, `UTC`). The next
   statement assigns `@@time_zone` to a new value. The third statement returns
   a timestamp with the new time zone.

       +---+
       | default_time_zone             |
       +---+
       | 2008-12-20 15:30:00+00        |
       +---+

       +---+
       | new_time_zone                 |
       +---+
       | 2008-12-20 07:30:00-08        |
       +---+

## What's next

- Learn more about how to [work with sessions](https://docs.cloud.google.com/bigquery/docs/sessions), including how to create, use, terminate, and list your sessions.
- Learn more about [multi-statement queries](https://docs.cloud.google.com/bigquery/docs/multi-statement-queries) in GoogleSQL.
- Learn more about [multi-statement transactions](https://docs.cloud.google.com/bigquery/docs/transactions) in GoogleSQL.