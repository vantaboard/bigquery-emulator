# List table snapshots

This document describes how to get a list of the table snapshots in a
BigQuery dataset in the Google Cloud console, by querying the
[`INFORMATION_SCHEMA.TABLE_SNAPSHOTS`](https://docs.cloud.google.com/bigquery/docs/information-schema-snapshots)
table, by using the [`bq ls`](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#bq_ls)
command, or by calling the [`tables.list`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables/list)
API. It also describes how to list all of the table snapshots of a
specified base table by querying the `INFORMATION_SCHEMA.TABLE_SNAPSHOTS` table.
This document is intended for users who are familiar with
BigQuery
[tables](https://docs.cloud.google.com/bigquery/docs/tables-intro) and
[table snapshots](https://docs.cloud.google.com/bigquery/docs/table-snapshots-intro).

## Permissions and roles

This section describes the
[Identity and Access Management (IAM) permissions](https://docs.cloud.google.com/bigquery/docs/access-control#bq-permissions)
that you need to list the table snapshots in a dataset, and the
[predefined IAM roles](https://docs.cloud.google.com/bigquery/docs/access-control#bigquery)
that grant those permissions. The permissions and roles for listing table
snapshots are the same as the permissions and roles required for
[listing other types of tables](https://docs.cloud.google.com/bigquery/docs/tables#list_tables_in_a_dataset).

### Permissions

To list the table snapshots in a dataset, you need the following permission:

| **Permission** | **Resource** |
|---|---|
| `bigquery.tables.list` | The dataset that contains the table snapshots. |

### Roles

The predefined BigQuery roles that provide the required
permission are as follows:

| **Role** | **Resource** |
|---|---|
| Any of the following: `bigquery.dataUser` `bigquery.dataViewer` `bigquery.dataEditor` `bigquery.dataOwner` `bigquery.admin` | The dataset that contains the table snapshots. |

## List the table snapshots in a dataset

Getting a list of table snapshots in a dataset is similar to listing other
types of tables. The table snapshots have the type `SNAPSHOT`.

You can list table snapshots by using one of the following options:

### Console

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)

   If you don't see the left pane, click **Expand left pane** to open the pane.
3. In the **Explorer** pane, expand the project, click **Datasets**, and then
   select the dataset that contains the table snapshots that you want to list.

4. Click **Overview \> Tables** . To find snapshots from the list,
   check for the **`SNAPSHOT`** value in the **Type** column.

### SQL

Query the
[`INFORMATION_SCHEMA.TABLE_SNAPSHOTS` view](https://docs.cloud.google.com/bigquery/docs/information-schema-snapshots):

<br />

<br />

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, enter the following statement:

   ```googlesql
   SELECT
     *
   FROM
     PROJECT_ID.DATASET_NAME.INFORMATION_SCHEMA.TABLE_SNAPSHOTS;
   ```


   Replace the following:
   - `PROJECT_ID`: the project ID of the project that contains the snapshots you want to list.
   - `DATASET_NAME`: the name of the dataset that contains the snapshots you want to list.

   <br />

3. Click **Run**.

   <br />

For more information about how to run queries, see [Run an interactive query](https://docs.cloud.google.com/bigquery/docs/running-queries#queries).

The result looks similar to the following:

```
+---+---+---+---+---+---+---+
| table_catalog | table_schema   | table_name       | base_table_catalog | base_table_schema | base_table_name | snapshot_time               |
+---+---+---+---+---+---+---+
| myproject     | mydataset      | mysnapshot       | basetableproject   | basetabledataset           | basetable           | 2021-04-16 14:05:27.519 UTC |
+---+---+---+---+---+---+---+
```

<br />

### bq

Enter the following command in the Cloud Shell:

[Go to Cloud Shell](https://console.cloud.google.com/bigquery?cloudshell=true)

```bash
bq ls \
PROJECT_ID:DATASET_NAME
```

Replace the following:

- `PROJECT_ID`: the project ID of the project that contains the snapshots you want to list.
- `DATASET_NAME`: the name of the dataset that contains the snapshots you want to list.

<br />

The output looks similar to the following:

```
+---+---+---+---+
|         tableId         |  Type  |       Labels        | Time Partitioning |
+---+---+---+---+
| mysnapshot              |SNAPSHOT|                     |                   |
+---+---+---+---+
```

<br />

### API

Call the
[`tables.list`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables/list)
method with the following parameters:

| **Parameter** | **Value** |
|---|---|
| `projectId` | The project ID of the project that contains the snapshots you want to list. |
| `datasetId` | The name of the dataset that contains the snapshots you want to list. |

## List the table snapshots of a specified base table

You can list the table snapshots of a specified base table by querying the
`INFORMATION_SCHEMA.TABLE_SNAPSHOTS` view:

```googlesql
SELECT
  *
FROM
  PROJECT_ID.DATASET_NAME.INFORMATION_SCHEMA.TABLE_SNAPSHOTS
WHERE
  base_table_name = 'books';
  
```

Replace the following:

- `PROJECT_ID`: the project ID of the project that contains the snapshots you want to list.
- `DATASET_NAME`: the name of the dataset that contains the snapshots you want to list.

<br />

## What's next

- [Get information about a table snapshot](https://docs.cloud.google.com/bigquery/docs/table-snapshots-metadata).
- [Update a table snapshot's description, expiration date, or
  access policy](https://docs.cloud.google.com/bigquery/docs/table-snapshots-update).
- [Delete a table snapshot](https://docs.cloud.google.com/bigquery/docs/table-snapshots-delete).