# CONSTRAINT_COLUMN_USAGE view

The `CONSTRAINT_COLUMN_USAGE` view contains all columns used by
[constraints](https://docs.cloud.google.com/bigquery/docs/primary-foreign-keys).
For `PRIMARY KEY` constraints, these are the columns from
the `KEY_COLUMN_USAGE` view. For `FOREIGN KEY` constraints, these are the columns
of the referenced tables.

## Schema

The `INFORMATION_SCHEMA.CONSTRAINT_COLUMN_USAGE` view has the following schema:

| Column Name | Data type | Value |
|---|---|---|
| ` table_catalog ` | ` STRING ` | The name of the project that contains the dataset. |
| ` table_schema ` | ` STRING ` | The name of the dataset that contains the table. Also referred to as the `datasetId`. |
| ` table_name ` | ` STRING ` | The name of the table. Also referred to as the `tableId`. |
| ` column_name ` | ` STRING ` | The column name. |
| ` constraint_catalog ` | ` STRING ` | The constraint project name. |
| ` constraint_schema ` | ` STRING ` | The constraint dataset name. |
| ` constraint_name ` | ` STRING ` | The constraint name. It can be the name of the primary key if the column is used by the primary key or the name of foreign key if the column is used by a foreign key. |

For stability, we recommend that you explicitly list columns in your information schema queries instead of
using a wildcard (`SELECT *`). Explicitly listing columns prevents queries from
breaking if the underlying schema changes.

## Scope and syntax

Queries against this view must include a dataset qualifier. For queries with a
dataset qualifier, you must have permissions for the dataset. For more
information see
[Syntax](https://docs.cloud.google.com/bigquery/docs/information-schema-intro#syntax).
The following table shows the region and resource scopes for this view:

| View name | Resource scope | Region scope |
|---|---|---|
| `[PROJECT_ID.]DATASET.INFORMATION_SCHEMA.CONSTRAINT_COLUMN_USAGE;` | Dataset level | Dataset location |

Replace the following:

- Optional: `PROJECT_ID`: the ID of your Google Cloud project. If not specified, the default project is used.

<br />

## Examples

The following query shows the constraints for a single table in a dataset:

```googlesql
SELECT *
FROM PROJECT_ID.DATASET.INFORMATION_SCHEMA.CONSTRAINT_COLUMN_USAGE
WHERE table_name = TABLE;
```

Replace the following:

- `PROJECT_ID`: Optional. The name of your cloud project. If not specified, this command uses the default project.
- `DATASET`: The name of your dataset.
- `TABLE`: The name of the table.

Conversely, the following query shows the constraints for all tables in a single dataset.

```googlesql
SELECT *
FROM PROJECT_ID.DATASET.INFORMATION_SCHEMA.CONSTRAINT_COLUMN_USAGE;
```

With existing constraints, the query results are similar to the following:

```
+---+---+---+---+---+---+---+---+
| row |    table_catalog    | table_schema | table_name | column_name | constraint_catalog  | constraint_schema |     constraint_name     |
+---+---+---+---+---+---+---+---+
|   1 | myConstraintCatalog | myDataset    | orders     | o_okey      | myConstraintCatalog | myDataset         | orders.pk$              |
|   2 | myConstraintCatalog | myDataset    | orders     | o_okey      | myConstraintCatalog | myDataset         | lineitem.lineitem_order |
+---+---+---+---+---+---+---+---+
```

> [!NOTE]
> **Note:** `lineitem.lineitem_order` is the foreign key defined in the `lineitem` table.

If the table or dataset has no constraints, the query results look like this:

```
+---+
| There is no data to display |
+---+
```