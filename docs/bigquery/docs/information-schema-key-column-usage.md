# KEY_COLUMN_USAGE view

The `KEY_COLUMN_USAGE` view contains columns of the tables from
`TABLE_CONSTRAINTS` that are constrained as keys by
[primary and foreign key](https://docs.cloud.google.com/bigquery/docs/primary-foreign-keys) constraints.

## Schema

The `INFORMATION_SCHEMA.KEY_COLUMN_USAGE` view has the following schema:

| Column Name | Data Type | Value |
|---|---|---|
| ` constraint_catalog ` | ` STRING ` | The constraint project name. |
| ` constraint_schema ` | ` STRING ` | The constraint dataset name. |
| ` constraint_name ` | ` STRING ` | The constraint name. |
| ` table_catalog ` | ` STRING ` | The project name of the constrained table. |
| ` table_schema ` | ` STRING ` | The name of the constrained table dataset. |
| ` table_name ` | ` STRING ` | The name of the constrained table. |
| ` column_name ` | ` STRING ` | The name of the constrained column. |
| ` ordinal_position ` | ` INT64 ` | The ordinal position of the column within the constraint key (starting at 1). |
| ` position_in_unique_constraint ` | ` INT64 ` | For foreign keys, the ordinal position of the column within the primary key constraint (starting at 1). This value is `NULL` for primary key constraints. |

For stability, we recommend that you explicitly list columns in your information schema queries instead of
using a wildcard (`SELECT *`). Explicitly listing columns prevents queries from
breaking if the underlying schema changes.

## Scope and syntax

Queries against this view must include a dataset qualifier. For queries with a
dataset qualifier, you must have permissions for the dataset. For more
information, see
[Syntax](https://docs.cloud.google.com/bigquery/docs/information-schema-intro#syntax).
The following table shows the region and resource scopes for this view:

| View name | Resource scope | Region scope |
|---|---|---|
| `[PROJECT_ID.]DATASET_ID.INFORMATION_SCHEMA.KEY_COLUMN_USAGE;` | Dataset level | Dataset location |

Replace the following:

- Optional: `PROJECT_ID`: the ID of your Google Cloud project. If not specified, the default project is used.

<br />

For queries with a dataset qualifier, you must have permissions for the dataset.
For queries with a region qualifier, you must have permissions for the project.

## Examples

##### Example 1:

The following query shows the constraints for a single table in a dataset:

```googlesql
SELECT *
FROM PROJECT_ID.DATASET.INFORMATION_SCHEMA.KEY_COLUMN_USAGE
WHERE table_name = TABLE;
```

Replace the following:

- `PROJECT_ID`: Optional. The name of your cloud project. If not specified, this command uses the default project.
- `DATASET`: The name of your dataset.
- `TABLE`: The name of the table.

Conversely, the following query shows the key columns usage for all tables in a single dataset.

```googlesql
SELECT *
FROM PROJECT_ID.DATASET.INFORMATION_SCHEMA.KEY_COLUMN_USAGE;
```

If a table or a dataset has no constraints, the query results look like this:

```
+---+
| There is no data to display |
+---+
```

##### Example 2:

The following DDL statements create a primary key table and a foreign key table.

```googlesql
CREATE TABLE composite_pk (x int64, y string, primary key (x, y) NOT ENFORCED);
```

```googlesql
CREATE TABLE table composite_fk (x int64, y string, z string,  primary key (x, y)
NOT ENFORCED, CONSTRAINT composite_fk foreign key (z, x)
REFERENCES composite_pk (y, x) NOT ENFORCED);
```

If queried with the statement in [Example 1](https://docs.cloud.google.com/bigquery/docs/information-schema-key-column-usage#example_01), the query results
are similar to the following. Note that `CONSTRAINT_CATALOG`,
`CONSTRAINT_SCHEMA`, and duplicate columns are not included in the example results.

```
+---+---+---+---+---+
|     CONSTRAINT_NAME       |  TABLE_NAME  | COLUMN_NAME | ORDINAL_POSITION | POSITION_IN_UNIQUE_CONSTRAINT |
+---+---+---+---+---+
| composite_pk.pk$          | composite_pk | x           | 1                | NULL                          |
| composite_pk.pk$          | composite_pk | y           | 2                | NULL                          |
| composite_fk.pk$          | composite_fk | x           | 1                | NULL                          |
| composite_fk.pk$          | composite_fk | y           | 2                | NULL                          |
| composite_fk.composite_fk | composite_fk | z           | 1                | 2                             |
| composite_fk.composite_fk | composite_fk | x           | 2                | 1                             |
+---+---+---+---+---+
```