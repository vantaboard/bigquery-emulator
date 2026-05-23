# Using row-level security with other
BigQuery features

This document describes how to use row-level access security with other
BigQuery features.

Before you read this document, familiarize yourself with
row-level security by reading
[Introduction to BigQuery row-level security](https://docs.cloud.google.com/bigquery/docs/row-level-security-intro)
and
[Working with row-level security](https://docs.cloud.google.com/bigquery/docs/managing-row-level-security).

> [!NOTE]
> **Note:** When managing access for users in [external identity providers](https://docs.cloud.google.com/iam/docs/workforce-identity-federation), replace instances of Google Account principal identifiers---like `user:kiran@example.com`, `group:support@example.com`, and `domain:example.com`---with appropriate [Workforce Identity Federation principal identifiers](https://docs.cloud.google.com/iam/docs/principal-identifiers).

## The `TRUE` filter

Row-level access policies can filter the result data that you see when running
queries. To run non-query operations, such as DML, you need full
access to all rows in the table. Full access is granted
by using a row access policy with the filter expression set to `TRUE`. This
row-level access policy is called the *`TRUE` filter*.

Any user can be granted `TRUE` filter access, including a service account.

Examples of non-query operations are:

- Other BigQuery APIs, such as the [BigQuery Storage Read API](https://docs.cloud.google.com/bigquery/docs/reference/storage).
- Some [`bq` command-line tool](https://docs.cloud.google.com/bigquery/docs/bq-command-line-tool) commands, such as the [`bq head`](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#bq_head) command.
- [Copying a table](https://docs.cloud.google.com/bigquery/docs/using-row-level-security-with-features#features_that_work_with_the_true_filter)

### `TRUE` filter example

    CREATE ROW ACCESS POLICY all_access ON project.dataset.table1
    GRANT TO ("group:all-rows-access@example.com")
    FILTER USING (TRUE);

### Features that work with the `TRUE` filter

When you use a [DML](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/dml-syntax) operation
on a table protected by row access policies, you must use a `TRUE` filter which
implies access to the whole table. Any operations that don't alter the table
schema maintain any row access policies on the table.

For example, the [`ALTER TABLE RENAME
TO`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#alter_table_rename_to_statement)
statement copies row access policies from the original table to the new table.
As another example, the [`TRUNCATE
TABLE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/dml-syntax#truncate_table_statement)
statement removes all of the rows from a table but maintains the table schema as
well as any row access policies.

#### Copy jobs

To [copy a table](https://docs.cloud.google.com/bigquery/docs/managing-tables#copy-table) with one or more
row-level access policies on it, you must first be granted `TRUE` filter access
on
the source table. All row-level access policies on the source table are also
copied to the new destination table. If you copy a source table without
row-level access policies onto a destination table that does have row-level
access policies,
then the row-level access policies are removed from the destination table,
unless the `--append_table` flag is used or `"writeDisposition": "WRITE_APPEND"`
is set.

Cross-region copies are allowed and all policies are copied. Subsequent queries
might be broken after the copy is complete if the queries contain invalid table
references in subquery policies.

Row-level access policies on a table must have unique names. A collision in
row-level access policy names during the copy results in an invalid input
error.

> [!CAUTION]
> **Caution:** After a table is copied, any row-level or column-level access policies that were copied are independent from the original table's security. The destination table's security is not synchronized with the original table's security.

##### Required permissions to copy a table with a row-level access policy

To copy a table with one or more row-level access policies,
you must have the following permissions, in addition to the
[roles to copy tables and partitions](https://docs.cloud.google.com/bigquery/docs/managing-tables#roles_to_copy_tables_and_partitions).

| **Permission** | **Resource** |
|---|---|
| ` bigquery.rowAccessPolicies.list ` | The source table. |
| ` bigquery.rowAccessPolicies.getIamPolicy ` | The source table. |
| The [`TRUE` filter](https://docs.cloud.google.com/bigquery/docs/using-row-level-security-with-features#the_true_filter) | The source table. |
| ` bigquery.rowAccessPolicies.create` | The destination table. |
| ` bigquery.rowAccessPolicies.setIamPolicy` | The destination table. |

#### Tabledata.list in BigQuery API

You need `TRUE` filter access in order to use the `tabledata.list` method in the
BigQuery API on a table with row-level access policies.

#### DML

To execute a DML statement that updates a table that has row-level access
policies, you need `TRUE` filter access for the table.

In particular, `MERGE` statements interact with row-level access policies as
follows:

- If a target table contains row-level access policies, then you need `TRUE` filter access to the target table.
- If a source table contains row-level access policies, then the `MERGE` statement only acts on the rows that are visible to the user.

#### Table snapshots

[Table snapshots](https://docs.cloud.google.com/bigquery/docs/table-snapshots-intro) support
row-level security. The permissions that you need for the base table
(source table) and the table snapshot (destination table) are described in
[Required permissions to copy a table with a row-level access policy](https://docs.cloud.google.com/bigquery/docs/using-row-level-security-with-features#required_permissions_to_copy_a_table_with_a_row-level_access_policy).

## BigQuery table with JSON columns

Row-level access policies cannot be applied on [JSON columns](https://docs.cloud.google.com/bigquery/docs/json-data).
To learn more about the limitations for row-level security, see
[Limitations](https://docs.cloud.google.com/bigquery/docs/row-level-security-intro#limitations).

## BigQuery BI Engine and Data Studio

[BigQuery BI Engine](https://docs.cloud.google.com/bigquery/docs/bi-engine-intro) does not accelerate queries
that are run on tables with one or more row-level access policies; those queries
are run as standard queries in BigQuery.

The data in a Data Studio dashboard is filtered according to the
underlying source table's row-level access policies.

## Column-level security

Row-level security and column-level security, which includes both
[column-level access control](https://docs.cloud.google.com/bigquery/docs/column-level-security-intro) and
[dynamic data masking](https://docs.cloud.google.com/bigquery/docs/column-data-masking-intro), are fully
compatible.

Key points are:

- You can apply a row-level access policy to filter data in any column, even if you don't have access to the data in that column.
  - Attempts to access these columns with the subquery row-level access policy results in an error indicating that access is denied. These columns aren't considered system-referenced columns.
  - Attempts to access these columns with the non-subquery row-level access policy bypass column-level security.
- If the column is restricted due to column-level security, and the column is named in the query's `SELECT` statement or subquery row-level access policies, you receive an error.
- Column-level security also applies with a `SELECT *` query statement. The `SELECT *` is treated the same as a query which explicitly names a restricted column.

### Example of row-level security and column-level security interacting

This example walks you through the steps for securing a table and then
querying it.

#### The data

Suppose that you have the DataOwner role for a dataset named
`my_dataset` which includes a table with three columns, named `my_table`.
The table contains the data shown in the following table.

In this example, one user is **Alice** , whose email address is
`alice@example.com`. A second user is **Bob**, Alice's colleague.

| **rank** | **fruit** | **color** |
|---|---|---|
| 1 | apple | red |
| 2 | orange | orange |
| 3 | lime | green |
| 4 | lemon | yellow |

#### The security

You want Alice to be able to see all the rows that have odd numbers in the
`rank` column, but not even-numbered rows. You don't want Bob to see any rows,
even or odd. You don't want anyone to see any data in the `fruit` column.

- To restrict Alice from seeing the even-numbered rows, you create a
  row-level access policy which has a filter expression based on the data
  that appears in the `rank` column. To prevent Bob from seeing even or odd
  rows, you don't include him in the grantee list.

      CREATE ROW ACCESS POLICY only_odd ON my_dataset.my_table GRANT
      TO ('user:alice@example.com') FILTER USING (MOD(rank, 2) = 1);

- To restrict all users from seeing data in the column named `fruit`,
  you create a column-level security policy tag that prohibits all users from
  accessing any of its data.

Finally, you also restrict access to the column named `color` in two ways:
the column is governed both by a column-level security policy tag prohibiting
all access by anyone, *and* is affected by a row-level access policy, which
filters some of the row data in the `color` column.

- This second row-level access policy only displays rows with the
  value `green` in the `color` column.

      CREATE ROW ACCESS POLICY only_green ON my_dataset.my_table
      GRANT TO ('user:alice@example.com') FILTER USING (color="green");

#### Bob's query

If Alice's coworker Bob tries to query data from `my_dataset.my_table`, he
doesn't see any rows, because Bob isn't in the grantee list for any row-level
access policy on the table.

| **Query** | **`my_dataset.my_table`** ||| **Comments** |
|   | **`rank`** (Some data is affected by the row access policy `only_odd`) | **`fruit`** (All data is secured by a CLS policy tag) | **`color`** (All data is secured by a CLS policy tag, *and* some data is affected by the row access policy `only_green`) |   |
|---|---|---|---|---|
| ` SELECT rank FROM my_dataset.my_table ` | Yes (0) rows returned. |   |   | Bob is not on the row-level access policy's grantee list; therefore this query succeeds, but no row data is returned. A message is displayed to Bob that says his results may be filtered by the row access policy. |

#### Alice's queries

When Alice runs queries to access data from `my_dataset.my_table`, her
results depend on the query she runs and the security, as shown
in the following table.

| **Query** | **`my_dataset.my_table`** ||| **Comments** |
|   | **`rank`** (Some data is affected by the row access policy `only_odd`) | **`fruit`** (All data is secured by a CLS policy tag) | **`color`** (All data is secured by a CLS policy tag, *and* some data is affected by the row access policy `only_green`) |   |
|---|---|---|---|---|
| ` SELECT rank FROM my_dataset.my_table ` | Yes (1) row is returned. |   |   | Alice is on the grantee list for the `only_odd` and the `only_green` row-level access policies. Therefore, Alice sees only ranks that are odd, and colors that are green. Therefore, Alice sees the following row: <br /> `rank: 3, color: green`. Alice does not see the fruit column because it is restricted by a column-level security policy. <br /> A message is displayed to Alice that says her results may be filtered by the row access policy. |
| ` SELECT fruit FROM my_dataset.my_table ` |   | No ` access denied ` |   | The `fruit` column was explicitly named in the query. <br /> The column-level security applies. <br /> Access is denied. |
| ` SELECT color FROM my_dataset.my_table ` |   |   | Yes (1) row is returned. | Alice is on the grantee list for the `only_odd` and the `only_green` row-level access policies. Therefore, Alice sees only ranks that are odd, and colors that are green. Therefore, Alice sees the following row: <br /> `rank: 3, color: green`. Alice does not see the fruit column because it is restricted by a column-level security policy. <br /> A message is displayed to Alice that says her results may be filtered by the row access policy. |
| ` SELECT rank, fruit FROM my_dataset.my_table ` |   | No ` access denied ` |   | The `fruit` column was explicitly named in the query. <br /> The column-level security applies, before the row-level access policy on data in the `rank` column is engaged. <br /> Access is denied. |
| ` SELECT rank, color FROM my_dataset.my_table ` |   |   | Yes (1) row is returned. | Alice is on the grantee list for the `only_odd` and the `only_green` row-level access policies. Therefore, Alice sees only ranks that are odd, and colors that are green. Therefore, Alice sees the following row: <br /> `rank: 3, color: green`. Alice does not see the fruit column because it is restricted by a column-level security policy. <br /> A message is displayed to Alice that says her results may be filtered by the row access policy. |
| ` SELECT fruit, color FROM my_dataset.my_table ` |   | No ` access denied ` |   | The `fruit` column was explicitly named in the query. <br /> The column-level security on the `fruit` column applies, before the row-level access policy on data in the `color` column is engaged. Access is denied. |
| ` SELECT * FROM my_dataset.my_table ` | Yes (1) row is returned. |   |   | Alice is on the grantee list for the `only_odd` and the `only_green` row-level access policies. Therefore, Alice sees only ranks that are odd, and colors that are green. Therefore, Alice sees the following row: <br /> `rank: 3, color: green`. Alice does not see the fruit column because it is restricted by a column-level security policy. <br /> A message is displayed to Alice that says her results may be filtered by the row access policy. |

#### `TRUE` filter access

Finally, as explained in
[the section about `TRUE` filter access](https://docs.cloud.google.com/bigquery/docs/using-row-level-security-with-features#the_true_filter),
if Alice or Bob has `TRUE` filter access, then they can
see all of the rows in the table, and use it in non-query jobs. However, if the
table has column-level security, then it still applies and can affect the
results.

## Execution graph

You can't use the [query execution graph](https://docs.cloud.google.com/bigquery/docs/query-insights) for
jobs with row-level access policies.

## Extract jobs

If a table has row-level access policies, then only the data that you can
view is exported to
Cloud Storage when you run an extract job.

## Legacy SQL

Row-level access policies are not compatible with Legacy SQL. Queries over
tables with row-level access policies must use
[GoogleSQL](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/migrating-from-legacy-sql).
Legacy SQL queries are rejected.

## Partitioned and clustered tables

Row-level security does not participate in query
[pruning](https://docs.cloud.google.com/bigquery/docs/querying-partitioned-tables), which is a feature of
[partitioned tables](https://docs.cloud.google.com/bigquery/docs/partitioned-tables).

While row-level security is compatible with partitioned and clustered
tables, the row-level access policies that filter row data aren't applied
during partition pruning. You can still use partition pruning on a table
that uses row-level security by specifying a `WHERE` clause that operates
on the partition column. Similarly, row-level access policies themselves
don't create any performance benefits for queries against clustered tables,
but they don't interfere with other filtering that you apply.

Query pruning is performed during the execution of row-level access policies
using the filters with the policies.

## Rename a table

You don't need `TRUE` filter access to rename a table with one or more row access
policies on it. You can
[rename a table with a DDL statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#alter_table_rename_to_statement).

As an alternative, you can also copy a table and give the destination table a
different name. If the source table has a row-level access policy on it, see
[table copy jobs](https://docs.cloud.google.com/bigquery/docs/using-row-level-security-with-features#features_that_work_with_the_true_filter)
on this page for more information.

## Streaming updates

To perform streaming table `UPDATE` or `DELETE` operations with [change data capture](https://docs.cloud.google.com/bigquery/docs/change-data-capture), you must have `TRUE` filter access.

## Time travel

Only a table administrator can access historical data for a table that has, or
has previously had, row-level access policies. Other users get an `access
denied` error if they use a time travel decorator on a table that has had
row-level access. For more information, see [Time travel and row-level
access](https://docs.cloud.google.com/bigquery/docs/time-travel#time_travel_and_row-level_access).

## Logical, materialized, and authorized views

This section describes different types of BigQuery views and
how they interact with row-level security.

### Logical or materialized views

Logical or materialized views are built from queries against tables.
The query results are usually a subset of the table data.

The data displayed in either type of view is filtered according to the
underlying source table's row-level access policies. However, you can't
reference views or materialized views in row-level access policies.

### Performance for materialized views

In addition, when a [materialized view](https://docs.cloud.google.com/bigquery/docs/materialized-views-intro)
is derived from an underlying table that has row-level access policies, then the
query performance is the same as when you query the source
table directly. In other words, if the source table has row-level security, you
don't see the typical performance benefits of querying a materialized view
versus querying the source table.

### Authorized views

You can also authorize a logical or materialized view, which means sharing
the view with specific users or groups (principals). Principals can then
query a view, but don't have access to the underlying table. For more
information, see [Authorized views](https://docs.cloud.google.com/bigquery/docs/authorized-views).

## Wildcard queries

[Wildcard queries](https://docs.cloud.google.com/bigquery/docs/querying-wildcard-tables) against
tables with row-level access policies fail with an `INVALID_INPUT` error.

## What's next

- For information about best practices for row-level access policies, see [Best practices for row-level security in BigQuery](https://docs.cloud.google.com/bigquery/docs/best-practices-row-level-security).