# Delete table snapshots

This document describes how to delete a table snapshot by using the
Google Cloud console, a
[`DROP SNAPSHOT TABLE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#drop_snapshot_table_statement)
GoogleSQL statement, a
[`bq rm`](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#bq_rm) command,
or a BigQuery API
[`tables.delete`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables/delete) call.
It also provides information about how to recover a table snapshot that was
deleted or that expired in the past seven days.
It is intended for users who are familiar with
[table snapshots](https://docs.cloud.google.com/bigquery/docs/table-snapshots-intro).

## Permissions and roles

This section describes the
[Identity and Access Management (IAM) permission](https://docs.cloud.google.com/bigquery/docs/access-control#bq-permissions)
that you need to delete a table snapshot, and the
[predefined IAM roles](https://docs.cloud.google.com/bigquery/docs/access-control#bigquery)
that grant those permissions.

### Permissions

To delete a table snapshot, you need the following permission:

| **Permission** | **Resource** |
|---|---|
| `bigquery.tables.deleteSnapshot` | The table snapshot that you want to delete |

### Roles

The predefined BigQuery roles that provide the required
permissions are as follows:

| **Role** | **Resource** |
|---|---|
| Any of the following: `bigquery.dataOwner` `bigquery.admin` | The table snapshot that you want to delete. |

## Delete a table snapshot

Delete a table snapshot as you would delete a standard table. You don't need to
delete a table snapshot that has expired.

You can delete a table snapshot by using one of the following options:

### Console

1. In the Google Cloud console, go to the **BigQuery** page.

[Go to BigQuery](https://console.cloud.google.com/bigquery)

1. In the left pane, click **Explorer**:

   ![Image of the highlighted Explorer pane button.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)

   If you don't see the left pane, click **Expand left pane** to open the pane.
2. In the **Explorer** pane, expand your project, click **Datasets**, and
   then click the dataset that has the table snapshot.

3. Click **Overview \> Tables**, and then click the name of the
   table snapshot.

4. In the details pane that appears, click **Delete**.

5. In the dialog that appears, type `delete`, and then click **Delete** again.

### SQL

Use the
[`DROP SNAPSHOT TABLE` DDL statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#drop_snapshot_table_statement):

<br />

<br />

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, enter the following statement:

   ```googlesql
   DROP SNAPSHOT TABLE PROJECT_ID.DATASET_NAME.SNAPSHOT_NAME;
   ```


   Replace the following:
   - `PROJECT_ID`: the project ID of the project that contains the snapshot.
   - `DATASET_NAME`: the name of the dataset that contains the snapshot.
   - `SNAPSHOT_NAME`: the name of the snapshot.

   <br />

3. Click **Run**.

   <br />

For more information about how to run queries, see [Run an interactive query](https://docs.cloud.google.com/bigquery/docs/running-queries#queries).

### bq

Enter the following command in the Cloud Shell:

[Go to Cloud Shell](https://console.cloud.google.com/bigquery?cloudshell=true)

```bash
bq rm \
PROJECT_ID:DATASET_NAME.SNAPSHOT_NAME
```

Replace the following:

- `PROJECT_ID`: the project ID of the project that contains the snapshot.
- `DATASET_NAME`: the name of the dataset that contains the snapshot.
- `SNAPSHOT_NAME`: the name of the snapshot.

<br />

### API

Call the
[`tables.delete`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables/delete)
method with the following parameters:

| **Parameter** | **Value** |
|---|---|
| `projectId` | The project ID of the project that contains the snapshot. |
| `datasetId` | The name of the dataset that contains the snapshot. |
| `tableId` | The name of the snapshot. |

## Restore a deleted or expired table snapshot

You can recover a table snapshot that was deleted or that expired in
the past seven days in the same way that you recover a standard table. For more
information, see
[Restore table snapshots](https://docs.cloud.google.com/bigquery/docs/table-snapshots-restore).

## What's next

- [Create monthly snapshots of a table by using a service account that runs a scheduled query](https://docs.cloud.google.com/bigquery/docs/table-snapshots-scheduled).