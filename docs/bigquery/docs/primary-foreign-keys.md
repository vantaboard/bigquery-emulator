# Use primary and foreign keys

Primary keys and foreign keys are table constraints that can help with
query optimization. This document explains how to create, view, and manage
constraints, and use them to optimize your queries.

BigQuery supports the following key constraints:

- **Primary key** : A primary key for a table is a combination of one or more columns that is unique for each row and not `NULL`.
- **Foreign key** : A foreign key for a table is a combination of one or more columns that is present in the primary key column of a referenced table, or is `NULL`.

Primary and foreign keys are typically used to ensure data integrity and enable
query optimization. BigQuery doesn't enforce primary and foreign
key constraints. When you declare constraints on your tables, you must ensure
that your data conforms to them. BigQuery can use table
constraints to optimize your queries.

## Manage constraints

Primary and foreign key relationships can be created and managed through the
following DDL statements:

- Create primary and foreign key constraints when you create a table by using the [`CREATE TABLE` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_table_statement).
- Add a primary key constraint to an existing table by using the [`ALTER TABLE ADD PRIMARY KEY` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#alter_table_add_primary_key_statement).
- Add a foreign key constraint to an existing table by using the [`ALTER TABLE ADD FOREIGN KEY` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#alter_table_add_foreign_key_statement).
- Drop a primary key constraint from a table by using the [`ALTER TABLE DROP PRIMARY KEY` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#alter_table_drop_primary_key_statement).
- Drop a foreign key constraint from a table by using the [`ALTER TABLE DROP CONSTRAINT` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#alter_table_drop_constraint_statement).

You can also manage table constraints through the BigQuery API
by updating the
[`TableConstraints` object](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables#TableConstraints).

## View constraints

The following views give you information about your table constraints:

- The [`INFORMATION_SCHEMA.TABLE_CONSTRAINTS` view](https://docs.cloud.google.com/bigquery/docs/information-schema-table-constraints) contains information about all of the primary and foreign key constraints on tables within a dataset.
- The [`INFORMATION_SCHEMA.CONSTRAINT_COLUMN_USAGE` view](https://docs.cloud.google.com/bigquery/docs/information-schema-constraint-column-usage) contains information about each table's primary key columns and columns referenced by foreign keys from other tables within a dataset.
- The [`INFORMATION_SCHEMA.KEY_COLUMN_USAGE` view](https://docs.cloud.google.com/bigquery/docs/information-schema-key-column-usage) contains information about each table's columns that are constrained as primary or foreign keys.

## Optimize queries

When you create and enforce primary and foreign keys on your tables,
BigQuery can use that information to eliminate or optimize
certain query joins. While it's possible to mimic these optimizations by
rewriting your queries, such rewrites aren't always practical.

In a production environment, you might create views that join many fact and
dimension tables. Developers can query the views instead of querying the
underlying tables and manually rewriting the joins each time. If you define
the proper constraints, join optimizations happen automatically for any
queries they apply to.

> [!CAUTION]
> **Caution:** Key constraints aren't enforced in BigQuery. You are responsible for maintaining the constraints at all times. Queries over tables with violated constraints might return incorrect results.

The examples in the following sections reference the `store_sales`
and `customer` tables with constraints:

    CREATE TABLE mydataset.customer (customer_name STRING PRIMARY KEY NOT ENFORCED);

    CREATE TABLE mydataset.store_sales (
        item STRING PRIMARY KEY NOT ENFORCED,
        sales_customer STRING REFERENCES mydataset.customer(customer_name) NOT ENFORCED,
        category STRING);

### Eliminate inner joins

Consider the following query that contains an `INNER JOIN`:

    SELECT ss.*
    FROM mydataset.store_sales AS ss
        INNER JOIN mydataset.customer AS c
        ON ss.sales_customer = c.customer_name;

The `customer_name` column is a primary key on the `customer` table, so
each row from the `store_sales` table has either a single match, or no match
if `sales_customer` is `NULL`. Since the query only selects columns from the
`store_sales` table, the query optimizer can eliminate the join and rewrite the
query as the following:

    SELECT *
    FROM mydataset.store_sales
    WHERE sales_customer IS NOT NULL;

### Eliminate outer joins

To remove a `LEFT OUTER JOIN`, the join keys on the right side must be unique
and only columns from the left side are selected. Consider the following query:

    SELECT ss.*
    FROM mydataset.store_sales ss
        LEFT OUTER JOIN mydataset.customer c
        ON ss.category = c.customer_name;

In this example, there is no relationship between `category` and
`customer_name`. The selected columns only come from
the `store_sales` table and the join key
`customer_name` is a primary key on the `customer` table, so each value is
unique. This means that there is exactly one (possibly `NULL`) match in the
`customer` table for each row in the `store_sales` table and the
`LEFT OUTER JOIN` can be eliminated:

    SELECT ss.*
    FROM mydataset.store_sales;

### Reorder joins

When BigQuery can't eliminate a join, it can use table
constraints to get information about join cardinalities and optimize the order
in which to perform joins.

## Limitations

Primary keys and foreign keys are subject to the following limitations:

- Key constraints are unenforced in BigQuery. You are responsible for maintaining the constraints at all times. Queries over tables with violated constraints might return incorrect results.
- Primary keys can't exceed 16 columns.
- Foreign keys must have values that are present in the referenced table column. These values can be `NULL`.
- Primary keys and foreign keys must be of one of the following types: `BIGNUMERIC`, `BOOLEAN`, `BYTES`, `DATE`, `DATETIME`, `INT64`, `NUMERIC`, `STRING`, or `TIMESTAMP`.
- Primary keys and foreign keys can only be set on top-level columns.
- Primary keys can't be named.
- Tables with primary key constraints can't be renamed.
- A table can have up to 64 foreign keys.
- A foreign key can't refer to a column in the same table.
- Fields that are part of primary key constraints or foreign key constraints can't be renamed, or have their type changed.
- If you [copy](https://docs.cloud.google.com/bigquery/docs/managing-tables#copy-table), [clone](https://docs.cloud.google.com/bigquery/docs/table-clones-create), [restore](https://docs.cloud.google.com/bigquery/docs/table-snapshots-restore), or [snapshot](https://docs.cloud.google.com/bigquery/docs/table-snapshots-create) a table without the `-a` or `--append_table` option, the source table constraints are copied and overwritten to the destination table. If you use the `-a` or `--append_table` option, only the source table records are added to the destination table without the table constraints.

## What's next

- Learn more about how to [Optimize query computation](https://docs.cloud.google.com/bigquery/docs/best-practices-performance-compute).