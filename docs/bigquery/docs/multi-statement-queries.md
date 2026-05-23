# Work with multi-statement queries

A *multi-statement query* is a collection of SQL statements that you can execute
in a sequence, with shared state.

This document describes how to use multi-statement queries in
BigQuery, such as how to write multi-statement queries, use
temporary tables in multi-statement queries, reference variables in
multi-statement queries, and debug multi-statement queries.

Multi-statement queries are often used in
[stored procedures](https://docs.cloud.google.com/bigquery/docs/procedures) and support
[procedural language statements](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/procedural-language),
which let you do things like define variables and implement control flow.
Multi-statement queries can contain DDL and DML statements that have side
effects, such as creating or modifying tables or table data.

## Write, run, and save multi-statement queries

A multi-statement query consists of one or more SQL statements
separated by semicolons. Any valid SQL statement can be used in a
multi-statement query. Multi-statement queries can also include
[procedural language statements](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/procedural-language),
which let you use variables or implement control flow with your SQL statements.

### Write a multi-statement query

You can write a multi-statement query in BigQuery. The following
multi-statement query declares a variable and uses the
variable inside an [`IF`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/procedural-language#if)
statement:

    DECLARE day INT64;
    SET day = (SELECT EXTRACT(DAYOFWEEK from CURRENT_DATE));
    if day = 1 or day = 7 THEN
      SELECT 'Weekend';
    ELSE
      SELECT 'Weekday';
    END IF

BigQuery interprets any request with multiple statements as a
multi-statement query, unless the statements consist entirely of
`CREATE TEMP FUNCTION` statements followed by a single `SELECT` statement.
For example, the following is not considered a multi-statement query:

    CREATE TEMP FUNCTION Add(x INT64, y INT64) AS (x + y);
    SELECT Add(3, 4);

### Run a multi-statement query

You can run a multi-statement query in the same way as any other query,
for example, in the Google Cloud console or using the bq command-line tool.

### Dry-run a multi-statement query

To estimate the number of bytes read by a multi-statement query, consider a
[dry run](https://docs.cloud.google.com/bigquery/docs/running-queries#dry-run). A dry run of a
multi-statement query is most accurate for queries that only contain `SELECT`
statements.

Dry runs have special handling for the following query and statement types:

- `CALL` statements: the dry run validates that the called procedure exists and has a signature matching the arguments provided. The content of the called procedure and all statements after the `CALL` statement are not validated.
- [DDL statements](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language): the dry run validates the first DDL statement and then stops. All subsequent statements are skipped. Dry runs of `CREATE TEMP TABLE` statements aren't supported.
- [DML statements](https://docs.cloud.google.com/bigquery/docs/data-manipulation-language): the dry run validates the DML statement and then continues to validate subsequent statements. In this case, byte estimates are based on original table sizes, and don't take into account the outcome of the DML statement.
- `EXECUTE IMMEDIATE` statements: the dry run validates the query expression, but does not evaluate the dynamic query itself. All statements following the `EXECUTE IMMEDIATE` statement are skipped.
- Queries that use variables in a partition filter: the dry run validates the initial query and subsequent statements. However, the dry run is unable to calculate the runtime value of variables in a partition filter. This affects the bytes read estimate.
- Queries that use variables in the timestamp expression of a `FOR SYSTEM TIME AS OF` clause: the dry run uses the table's current content and ignores the `FOR SYSTEM TIME AS OF` clause. This affects the bytes read estimate if there are size differences between the current table and the prior iteration of the table.
- `FOR`, `IF` and `WHILE` control statements: the dry run stops immediately. Condition expressions, bodies of the control statement, and all subsequent statements are not validated.

Dry runs operate on a best-effort basis, and the underlying process is subject
to change. Dry runs are subject to the following stipulations:

- A query that successfully completes a dry run might not execute successfully. For example, queries might fail at runtime due to reasons that are not detected by dry runs.
- A query that successfully executes might not complete a dry run successfully. For example, queries might fail dry runs due to reasons caught at execution.
- Dry runs that successfully run today are not guaranteed to always run in the future. For example, changes to the dry run implementation might detect errors in a query that were previously undetected.

### Save a multi-statement query

To save a multi-statement query, see
[Work with saved queries](https://docs.cloud.google.com/bigquery/docs/work-with-saved-queries).

## Use variables in a multi-statement query

A multi-statement query can contain
[user-created variables](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/procedural-language#declare)
and [system variables](https://docs.cloud.google.com/bigquery/docs/reference/system-variables).

- You can declare user-created variables, assign values to
  them, and reference them throughout the query.

- You can reference system variables in a query and assign
  values to some of them, but unlike user-defined variables, you don't declare
  them. System variables are built into BigQuery.

### Declare a user-created variable

You must declare user-created variables either at the start of the
multi-statement query or at the start of
a [`BEGIN`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/procedural-language#begin)
block. Variables declared at the start of the multi-statement query are in scope
for the entire query. Variables declared inside a `BEGIN` block have scope for
the block. They go out of scope after the corresponding `END` statement. The
maximum size of a variable is 1 MB, and the maximum size of all variables used
in a multi-statement query is 10 MB.

You can declare a variable with the
[`DECLARE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/procedural-language#declare)
procedural statement like this:

    DECLARE x INT64;

    BEGIN
    DECLARE y INT64;
    -- Here you can reference x and y
    END;

    -- Here you can reference x, but not y

### Set a user-created variable

After you declare a user-created variable, you can assign a value to it with the
[`SET`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/procedural-language#set)
procedural statement like this:

    DECLARE x INT64 DEFAULT 0;
    SET x = 10;

### Set a system variable

You don't create system variables, but you can override
the default value for some of them like this:

    SET @@dataset_project_id = 'MyProject';

You can also set and implicitly use a system variable in a multi-statement
query. For example, in the following query you must include the project each
time you wish to create a new table:

    BEGIN
      CREATE TABLE MyProject.MyDataset.MyTempTableA (id STRING);
      CREATE TABLE MyProject.MyDataset.MyTempTableB (id STRING);
    END;

If you don't want to add the project to table paths multiple times, you can
assign the dataset project ID `MyProject` to the `@@dataset_project_id` system
variable in the multi-statement query. This assignment makes `MyProject`
the default project for the rest of the query.

    SET @@dataset_project_id = 'MyProject';

    BEGIN
      CREATE TABLE MyDataset.MyTempTableA (id STRING);
      CREATE TABLE MyDataset.MyTempTableB (id STRING);
    END;

Similarly, you can set the `@@dataset_id` system variable to assign a default
dataset for the query. For example:

    SET @@dataset_project_id = 'MyProject';
    SET @@dataset_id = 'MyDataset';

    BEGIN
      CREATE TABLE MyTempTableA (id STRING);
      CREATE TABLE MyTempTableB (id STRING);
    END;

You can also explicitly reference system variables like `@@dataset_id` in
many parts of a multi-statement query. To learn more, see the
[system variable examples](https://docs.cloud.google.com/bigquery/docs/reference/system-variables#examples).

### Reference a user-created variable

After you have declared and set a user-created variable, you can reference it in
a multi-statement query. If a variable and column share the same name, the
column takes precedence.

This returns `column x` + `column x`:

    DECLARE x INT64 DEFAULT 0;
    SET x = 10;

    WITH Numbers AS (SELECT 50 AS x)
    SELECT (x+x) AS result FROM Numbers;

    +---+
    | result |
    +---+
    | 100    |
    +---+

This returns `column y` + `variable x`:

    DECLARE x INT64 DEFAULT 0;
    SET x = 10;

    WITH Numbers AS (SELECT 50 AS y)
    SELECT (y+x) AS result FROM Numbers;

    +---+
    | result |
    +---+
    | 60     |
    +---+

## Use temporary tables in a multi-statement query

Temporary tables let you save intermediate results to a table. Temporary tables are managed by BigQuery, so you don't need to save or maintain them in a dataset. You are charged for storage of temporary tables.

You can create and reference a temporary table in a multi-statement query. When you are finished with the temporary table, you can delete it manually to
minimize storage costs, or wait for BigQuery to delete it after
24 hours.

### Create a temporary table

You can create a temporary table for a multi-statement query with the
[`CREATE TABLE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_table_statement) statement.
The following example creates a temporary table to store the results of a query
and uses the temporary table in a subquery:

    -- Find the top 100 names from the year 2017.
    CREATE TEMP TABLE top_names(name STRING)
    AS
     SELECT name
     FROM `bigquery-public-data`.usa_names.usa_1910_current
     WHERE year = 2017
     ORDER BY number DESC LIMIT 100
    ;
    -- Which names appear as words in Shakespeare's plays?
    SELECT
     name AS shakespeare_name
    FROM top_names
    WHERE name IN (
     SELECT word
     FROM `bigquery-public-data`.samples.shakespeare
    );

Other than the use of `TEMP` or `TEMPORARY`, the syntax is identical to the
`CREATE TABLE` syntax.

When you create a temporary table, don't use a project or dataset qualifier in
the table name. The table is automatically created in a special dataset.

### Reference a temporary table

You can refer to a temporary table by name for the duration of the current
multi-statement query. This includes temporary tables created by a procedure
within the multi-statement query. You cannot share temporary tables. Temporary
tables reside in hidden `_script%` datasets with randomly generated names.
[Listing datasets](https://docs.cloud.google.com/bigquery/docs/listing-datasets#bq) article describes how to list hidden datasets.

### Delete temporary tables

You can delete a temporary table explicitly before the multi-statement query
completes by using the [`DROP TABLE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#drop_table_statement) statement:

```googlesql
CREATE TEMP TABLE table1(x INT64);
SELECT * FROM table1;  -- Succeeds
DROP TABLE table1;
SELECT * FROM table1;  -- Results in an error
```

After a multi-statement query finishes, the temporary table exists for up to
24 hours.

### View temporary table data

After you create a temporary table, you can view the structure of the
table and any data in it. To view the table structure and data, follow
these steps:

1. In the Google Cloud console, open the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)

   If you don't see the left pane, click **Expand left pane** to open the pane.
3. In the **Explorer** pane, click **Job history**.

4. In the **Personal history** or **Project history** tab, click the query that
   created the temporary table.

5. In the **Destination table** row, click **Temporary table**.

   > [!NOTE]
   > **Note:** The name of the temporary table used in the multi-statement query is not preserved. In the Google Cloud console, it has a random name.

### Qualify temporary tables with `_SESSION`

When temporary tables are used together with a default dataset, unqualified
table names refer to a temporary table if one exists, or a table in the default
dataset. The exception is for `CREATE TABLE` statements, where the target table
is considered a temporary table if and only if the `TEMP` or `TEMPORARY` keyword
is present.

For example, consider the following multi-statement query:

```googlesql
-- Create table t1 in the default dataset
CREATE TABLE t1 (x INT64);

-- Create temporary table t1.
CREATE TEMP TABLE t1 (x INT64);

-- This statement selects from the temporary table.
SELECT * FROM t1;

-- Drop the temporary table
DROP TABLE t1;

-- Now that the temporary table is dropped, this statement selects from the
-- table in the default dataset.
SELECT * FROM t1;
```

You can explicitly indicate that you are referring to a temporary table by
qualifying the table name with `_SESSION`:

```googlesql
-- Create a temp table
CREATE TEMP TABLE t1 (x INT64);

-- Create a temp table using the `_SESSION` qualifier
CREATE TEMP TABLE _SESSION.t2 (x INT64);

-- Select from a temporary table using the `_SESSION` qualifier
SELECT * FROM _SESSION.t1;
```

If you use the `_SESSION` qualifier for a query of a temporary table that does
not exist, the multi-statement query throws an error indicating that the table
does not exist. For example, if there is no temporary table named `t3`, the
multi-statement query throws an error even if a table named `t3` exists in the
default dataset.

You cannot use `_SESSION` to create a non-temporary table:

```googlesql
CREATE TABLE _SESSION.t4 (x INT64);  -- Fails
```

## Collect information about a multi-statement query job

A multi-statement query job contains information about a multi-statement query
that has been executed. Some common tasks that you can perform with job data
include returning the last statement executed with the multi-statement query or
returning all statements executed with the multi-statement query.

### Return the last executed statement

The [`jobs.getQueryResults`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs/getQueryResults)
method returns the query results for the last statement to execute in the
multi-statement query. If no statement was executed, no results are
returned.

### Return all executed statements

To get the results of all statements in a
multi-statement query, [enumerate the child jobs](https://docs.cloud.google.com/bigquery/docs/multi-statement-queries#enumerate_child_jobs_of_a_multi-statement_query)
and call [`jobs.getQueryResults`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs/getQueryResults)
on each of them.

### Enumerate child jobs

Multi-statement queries are executed in BigQuery using
[`jobs.insert`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs/insert),
similar to any other query, with the multi-statement queries specified as the
query text. When a multi-statement query runs, additional jobs, known as
child jobs, are created for each statement in the multi-statement query. You
can enumerate the child jobs of a multi-statement query by calling
[`jobs.list`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs/list), passing in the
multi-statement query job ID as the `parentJobId` parameter.

## Debug a multi-statement-query

Here are some tips for debugging multi-statement queries:

- Use the [`ASSERT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/debugging-statements#assert)
  statement to assert that a Boolean condition is true.

- Use [`BEGIN...EXCEPTION...END`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/procedural-language#beginexceptionend)
  to catch errors and display the error message and stack trace.

- Use `SELECT FORMAT("....")` to show intermediate results.

- When you run a multi-statement query in the Google Cloud console, you can view the
  output of each statement in the multi-statement query. The bq command-line tool's
  `bq query` command also shows the results of each step when you run a
  multi-statement query.

- In the Google Cloud console, you can select an individual statement inside the
  query editor and run it.

## Permissions

Permission to access a table, model, or other resource is checked at the time of
execution. If a statement is not executed, or an expression is not evaluated,
BigQuery does not check whether the user executing the
multi-statement query has access to any resources referenced by it.

Within a multi-statement query, the permissions for each expression or statement
are validated separately. For example:

```googlesql
SELECT * FROM dataset_with_access.table1;
SELECT * FROM dataset_without_access.table2;
```

If the user executing the query has access to `table1`
but does not have access to `table2`, the first query succeeds and the
second query fails. The multi-statement query job itself also
fails.

## Security constraints

In multi-statement queries, you can use [dynamic SQL](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/procedural-language#execute_immediate)
to build SQL statements at runtime. This is convenient, but can offer new
opportunities for misuse. For example, executing the following query poses a
potential [SQL injection](https://en.wikipedia.org/wiki/SQL_injection) security
threat since the table parameter could be improperly filtered, allow access to,
and be executed on unintended tables.

    -- Risky query vulnerable to SQL injection attack.
    EXECUTE IMMEDIATE CONCAT('SELECT * FROM SensitiveTable WHERE id = ', @id);

To avoid exposing or leaking sensitive data in a table or running
commands like `DROP TABLE` to delete data in a table, BigQuery's
dynamic procedural statements support multiple security measures to
reduce exposure to SQL injection attacks, including:

- An `EXECUTE IMMEDIATE` statement does not allow its query, expanded with query parameters and variables, to embed multiple SQL statements.
- The following commands are restricted from being executed dynamically: `BEGIN`/`END`, `CALL`, `CASE`, `IF`, `LOOP`, `WHILE`, and `EXECUTE IMMEDIATE`.

## Configuration field limitations

The following [job configuration query fields](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/Job#JobConfigurationQuery)
cannot be set for a multi-statement query:

- `clustering`
- `create_disposition`
- `destination_table`
- `destination_encryption_configuration`
- `range_partitioning`
- `schema_update_options`
- `time_partitioning`
- `user_defined_function_resources`
- `write_disposition`

## Pricing

Pricing for multi-statement queries includes charges for queries (when using
the [on-demand billing model](https://cloud.google.com/bigquery/pricing#on_demand_pricing))
and storage for [temporary tables](https://docs.cloud.google.com/bigquery/docs/multi-statement-queries#temporary_tables). When you are using
[reservations](https://docs.cloud.google.com/bigquery/docs/reservations-intro), query usage is covered by
your reservation charges.

### On-demand query size calculation

If you use on-demand billing, BigQuery charges for
multi-statement queries based on the number of bytes processed during execution
of the multi-statement queries.

To get an estimate of how many bytes a multi-statement query might process,
you can run a [dry run](https://docs.cloud.google.com/bigquery/docs/multi-statement-queries#dryrun_multi_statement_queries).

> [!NOTE]
> **Note:** The number of bytes scanned by a multi-statement query is generally not known before executing it. To avoid unintended query costs, consider using [capacity-based pricing](https://cloud.google.com/bigquery/pricing#capacity_compute_analysis_pricing). Alternatively, you can use the BigQuery [sandbox](https://docs.cloud.google.com/bigquery/docs/sandbox) to take advantage of limited free query execution.

The following pricing applies for these multi-statement queries:

- `DECLARE`: the sum of bytes scanned for any tables referenced in the `DEFAULT`
  expression. `DECLARE` statements with no table references don't incur a cost.

- `SET`: the sum of bytes scanned for any tables referenced in the expression. `SET`
  statements with no table references don't incur a cost.

- `IF`: the sum of bytes scanned for any tables referenced in the condition
  expression. `IF` condition expressions with no table reference don't incur a
  cost. Any statements within the `IF` block that are not executed don't incur a
  cost.

- `WHILE`: the sum of bytes scanned for any tables referenced in the condition
  expression. `WHILE` statements with no table references in the condition
  expression don't incur a cost. Any statements within the `WHILE` block that are
  not executed don't incur a cost.

- `CONTINUE` or `ITERATE`: No associated cost.

- `BREAK` or `LEAVE`: No associated cost.

- `BEGIN` or `END`: No associated cost.

If a multi-statement query fails, the cost of any statements up until
the failure still applies. The statement that failed does not incur any costs.

For example, the following sample code contains comments preceding
every statement that explain what cost, if any, is incurred by each statement:

```googlesql
-- No cost, since no tables are referenced.
DECLARE x DATE DEFAULT CURRENT_DATE();
-- Incurs the cost of scanning string_col from dataset.table.
DECLARE y STRING DEFAULT (SELECT MAX(string_col) FROM dataset.table);
-- Incurs the cost of copying the data from dataset.big_table.  Once the
-- table is created, you are not charged for storage while the rest of the
-- multi-statement query runs.
CREATE TEMP TABLE t AS SELECT * FROM dataset.big_table;
-- Incurs the cost of scanning column1 from temporary table t.
SELECT column1 FROM t;
-- No cost, since y = 'foo' doesn't reference a table.
IF y = 'foo' THEN
  -- Incurs the cost of scanning all columns from dataset.other_table, if
  -- y was equal to 'foo', or otherwise no cost since it is not executed.
  SELECT * FROM dataset.other_table;
ELSE
  -- Incurs the cost of scanning all columns from dataset.different_table, if
  -- y was not equal to 'foo', or otherwise no cost since it is not executed.
  UPDATE dataset.different_table
  SET col = 10
  WHERE true;
END IF;
-- Incurs the cost of scanning date_col from dataset.table for each
-- iteration of the loop.
WHILE x < (SELECT MIN(date_col) FROM dataset.table) DO
  -- No cost, since the expression does not reference any tables.
  SET x = DATE_ADD(x, INTERVAL 1 DAY);
  -- No cost, since the expression does not reference any tables.
  IF true THEN
    -- LEAVE has no associated cost.
    LEAVE;
  END IF;
  -- Never executed, since the IF branch is always taken, so does not incur
  -- a cost.
  SELECT * FROM dataset.big_table;
END WHILE;
```

For more information, see
[Query size calculation](https://docs.cloud.google.com/bigquery/docs/estimate-costs#query_size_calculation).

### Storage pricing

You are charged for [temporary tables](https://docs.cloud.google.com/bigquery/docs/multi-statement-queries#temporary_tables) created by multi-statement queries. You can use the
[`TABLE_STORAGE`](https://docs.cloud.google.com/bigquery/docs/information-schema-table-storage) or
[`TABLE_STORAGE_USAGE_TIMELINE`](https://docs.cloud.google.com/bigquery/docs/information-schema-table-storage-usage)
views to see the storage used by these temporary tables. Temporary
tables reside in hidden `_script%` datasets with randomly generated names.

## Quotas

For information about multi-statement query quotas, see
[Quotas and limits](https://docs.cloud.google.com/bigquery/quotas#multi_statement_query_limits).

### View the number of multi-statement queries

You can view the number of active multi-statement queries using the
[`INFORMATION_SCHEMA.JOBS_BY_PROJECT` view](https://docs.cloud.google.com/bigquery/docs/information-schema-jobs).
The following example uses the `INFORMATION_SCHEMA.JOBS_BY_PROJECT` view to
show the number of multi-statement queries from the previous day:

    SELECT
      COUNT(*)
    FROM
      `region-us`.INFORMATION_SCHEMA.JOBS_BY_PROJECT
    WHERE
      creation_time BETWEEN TIMESTAMP_SUB(CURRENT_TIMESTAMP(), INTERVAL 1 DAY) AND CURRENT_TIMESTAMP()
    AND job_type = "QUERY"
    AND state = 'RUNNING'
    AND statement_type = 'SCRIPT'

For more information about querying `INFORMATION_SCHEMA.JOBS` for
multi-statement queries, see [Multi-statement query job](https://docs.cloud.google.com/bigquery/docs/information-schema-jobs#multi-statement_query_jobs).