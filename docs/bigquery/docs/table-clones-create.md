# Create table clones

This document describes how to copy a table to a
[table clone](https://docs.cloud.google.com/bigquery/docs/table-clones-intro) by using a
[`CREATE TABLE CLONE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_table_clone_statement)
SQL statement, a [`bq cp`](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#bq_cp)
command, or a [`jobs.insert`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs/insert)
API call. This document is intended for users who are familiar with
[table clones](https://docs.cloud.google.com/bigquery/docs/table-clones-intro).

## Permissions and roles

This section describes the
[Identity and Access Management (IAM) permissions](https://docs.cloud.google.com/bigquery/docs/access-control#bq-permissions)
that you need to create a table clone, and the
[predefined IAM roles](https://docs.cloud.google.com/bigquery/docs/access-control#bigquery)
that grant those permissions.

### Permissions

To create a table clone, you need the following permissions:

| **Permission** | **Resource** |
|---|---|
| All of the following: `bigquery.tables.get` `bigquery.tables.getData` | The table that you want to make a clone of. |
| `bigquery.tables.create` `bigquery.tables.updateData` | The dataset that contains the table clone. |

### Roles

The predefined BigQuery roles that provide the required
permissions are as follows:

| **Role** | **Resource** |
|---|---|
| Any of the following: `bigquery.dataViewer` `bigquery.dataEditor` `bigquery.dataOwner` `bigquery.admin` | The table that you want to make a clone of. |
| Any of the following: `bigquery.dataEditor` `bigquery.dataOwner` `bigquery.admin` | The dataset that contains the new table clone. |

## Create a table clone

Use GoogleSQL, the bq command-line tool, or the
BigQuery API to create a table clone.

### SQL

To clone a table, use the
[CREATE TABLE CLONE](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_table_clone_statement)
statement.

<br />

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, enter the following statement:

   ```googlesql
   CREATE TABLE
   myproject.myDataset_backup.myTableClone
   CLONE myproject.myDataset.myTable;
   ```

   <br />

3. Click **Run**.

   <br />

For more information about how to run queries, see [Run an interactive query](https://docs.cloud.google.com/bigquery/docs/running-queries#queries).

Replace the following:

- `PROJECT` is the project ID of the target project. This project must be in the same organization as the project containing the table you are cloning.
- `DATASET` is the name of the target dataset. This dataset must be in the same region as the dataset containing the table you are cloning.
- `CLONE_NAME` is name of the table clone that you are creating.

### bq

Use a [`bq cp`](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#bq_cp) command
with the `--clone` flag:

[Go to Cloud Shell](https://console.cloud.google.com/bigquery?cloudshell=true)

```bash
bq cp --clone --no_clobber project1:myDataset.myTable PROJECT:DATASET.CLONE_NAME
```

Replace the following:

- `PROJECT` is the project ID of the target project. This project must be in the same organization as the project containing the table you are cloning.
- `DATASET` is the name of the target dataset. This dataset must be in the same region as the dataset containing the table you are cloning. If the dataset is not in the same region as the dataset containing the table you are cloning then a full table is copied.
- `CLONE_NAME` is name of the table clone that you are creating.

The `--no_clobber` flag is required.

If you are creating a clone in the same project as the base table, you
can skip specifying a project, as shown following:

```bash
bq cp --clone --no_clobber myDataset.myTable DATASET.CLONE_NAME
```

### API

Call the
[`jobs.insert`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs/insert) method with the
`operationType` field set to `CLONE`:

| **Parameter** | **Value** |
|---|---|
| `projectId` | The project ID of the project that runs the job. |
| Request body | ```json { "configuration": { "copy": { "sourceTables": [ { "projectId": "myProject", "datasetId": "myDataset", "tableId": "myTable" } ], "destinationTable": { "projectId": "PROJECT", "datasetId": "DATASET", "tableId": "CLONE_NAME" }, "operationType": "CLONE", "writeDisposition": "WRITE_EMPTY", } } } ``` |

Replace the following:

- `PROJECT` is the project ID of the target project. This project must be in the same organization as the project containing the table you are cloning.
- `DATASET` is the name of the target dataset. This dataset must be in the same region as the dataset containing the table you are cloning. If the dataset is not in the same region as the dataset containing the table you are cloning a full table is copied.
- `CLONE_NAME` is name of the table clone that you are creating.

## Access control

When you create a table clone, access to the table clone is set as follows:

- [Row-level access policies](https://docs.cloud.google.com/bigquery/docs/row-level-security-intro) are copied from the base table to the table clone.
- [Column-level access policies](https://docs.cloud.google.com/bigquery/docs/column-level-security-intro) are copied from the base table to the table clone.
- [Table-level access](https://docs.cloud.google.com/bigquery/docs/table-access-controls-intro) is
  determined as follows:

  - If the table clone overwrites an existing table, then the table-level access for the existing table is maintained. [Tags](https://docs.cloud.google.com/bigquery/docs/tags) aren't copied from the base table.
  - If the table clone is a new resource, then the table-level access for the table clone is determined by the access policies of the dataset in which the table clone is created. Additionally, [tags](https://docs.cloud.google.com/bigquery/docs/tags) are copied from the base table to the table clone.

## What's next

- After you create a table clone, you can use it like you use standard tables. For more information, see [Manage tables](https://docs.cloud.google.com/bigquery/docs/managing-tables).