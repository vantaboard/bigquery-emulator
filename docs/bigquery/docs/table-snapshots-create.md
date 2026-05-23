# Create table snapshots

This document describes how to create a snapshot of a table by using the
Google Cloud console, the [`CREATE SNAPSHOT TABLE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_snapshot_table_statement)
SQL statement, the
[`bq cp --snapshot`](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#bq_cp) command,
or the [`jobs.insert`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs/insert) API. This
document is intended for users who are familiar with BigQuery
[table snapshots](https://docs.cloud.google.com/bigquery/docs/table-snapshots-intro).

## Permissions and roles

This section describes the
[Identity and Access Management (IAM) permissions](https://docs.cloud.google.com/bigquery/docs/access-control#bq-permissions)
that you need to create a table snapshot, and the
[predefined IAM roles](https://docs.cloud.google.com/bigquery/docs/access-control#bigquery)
that grant those permissions.

### Permissions

To create a table snapshot, you need the following permissions:

| **Permission** | **Resource** | **Notes** |
|---|---|---|
| All of the following: `bigquery.tables.get` `bigquery.tables.getData` `bigquery.tables.createSnapshot` `bigquery.datasets.get` `bigquery.jobs.create` | The table that you want to snapshot. | Because snapshot expiration deletes the snapshot at a later time, to create a snapshot with an expiration time you must have the `bigquery.tables.deleteSnapshot` permission. |
| `bigquery.tables.create` `bigquery.tables.updateData` | The dataset that contains the table snapshot. |   |

### Roles

The predefined BigQuery roles that provide the required
permissions are as follows:

| **Role** | **Resource** | **Notes** |
|---|---|---|
| At least one of the following: `bigquery.dataViewer` `bigquery.dataEditor` `bigquery.dataOwner` And at least one of the following: `bigquery.jobUser` `bigquery.studioUser` `bigquery.user` `bigquery.studioAdmin` `bigquery.admin` | The table that you want to snapshot. | Only `bigquery.dataOwner`, `bigquery.admin`, and `bigquery.studioAdmin` can be used for creating a snapshot with an expiration time. |
| At least one of the following: `bigquery.dataEditor` `bigquery.dataOwner` `bigquery.studioAdmin` `bigquery.admin` | The dataset that contains the new table snapshot. |   |

## Limitations

For information about table snapshot limitations, see
[table snapshot limitations](https://docs.cloud.google.com/bigquery/docs/table-snapshots-intro#limitations).

In addition, table snapshot creation is subject to the following limitations,
which apply to all
[table copy jobs](https://docs.cloud.google.com/bigquery/docs/managing-tables#limitations_on_copying_tables):

- When you create a table snapshot, its name must adhere to the same [naming rules](https://docs.cloud.google.com/bigquery/docs/tables#table_naming) as when you create a table.
- Table snapshot creation is subject to BigQuery [limits](https://docs.cloud.google.com/bigquery/quotas#copy_jobs) on copy jobs.
- The table snapshot dataset must be in the same [region](https://docs.cloud.google.com/bigquery/docs/locations), and under the same [organization](https://docs.cloud.google.com/resource-manager/docs/creating-managing-organization), as the dataset that contains the table you are taking a snapshot of. For example, you cannot create a table snapshot in a US-based dataset of a table located in an EU-based dataset. You would need to make a copy of the table instead.
- The time that BigQuery takes to create table snapshots might vary significantly across different runs because the underlying storage is managed dynamically.
- When creating a table snapshot using the BigQuery CLI, the snapshot has the default encryption key of the destination dataset. When creating a table snapshot using SQL, the snapshot has the same encryption key as the source table.

## Create a table snapshot

Best practice is to create a table snapshot in a different dataset from the
base table. This practice allows the base table to be
restored from its
table snapshot even if the base table's dataset is accidentally deleted.

When you create a table snapshot, you specify the table you
want to snapshot and a unique name for the table snapshot. You can optionally
specify the [time](https://docs.cloud.google.com/bigquery/docs/time-travel) of the snapshot and the table
snapshot's
[expiration](https://docs.cloud.google.com/bigquery/docs/managing-tables#updating_a_tables_expiration_time).

### Create a table snapshot with an expiration

You can create a snapshot of a table that expires after 24 hours by using
one of the following options:

### Console

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)

   If you don't see the left pane, click **Expand left pane** to open the pane.
3. In the **Explorer** pane, expand your project, click **Datasets**, and
   then click the dataset.

4. Click **Overview \> Tables**, and then click the name of the
   table that you want to snapshot.

5. In the details pane that appears, click **Snapshot**.

   ![A screenshot showing the Snapshot button in the BigQuery console.](https://docs.cloud.google.com/static/bigquery/images/snapshot-create.png)
6. In the **Create table snapshot** pane that appears, enter the **Project** ,
   **Dataset** , and **Table** information for the new table snapshot.

7. In the **Expiration time** field, enter the date and time for 24 hours
   from now.

8. Click **Save**.

### SQL

Use the
[`CREATE SNAPSHOT TABLE` DDL statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_snapshot_table_statement):

<br />

<br />

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, enter the following statement:

   ```googlesql
   CREATE SNAPSHOT TABLE SNAPSHOT_PROJECT_ID.SNAPSHOT_DATASET_NAME.SNAPSHOT_NAME
   CLONE TABLE_PROJECT_ID.TABLE_DATASET_NAME.TABLE_NAME
     OPTIONS (
       expiration_timestamp = TIMESTAMP 'TIMESTAMP_VALUE');
   ```


   Replace the following:
   - `SNAPSHOT_PROJECT_ID`: the project ID of the project in which to create the snapshot.
   - `SNAPSHOT_DATASET_NAME`: the name of the dataset in which to create the snapshot.
   - `SNAPSHOT_NAME`: the name of the snapshot you are creating.
   - `TABLE_PROJECT_ID`: the project ID of the project that contains the table you are creating the snapshot from.
   - `TABLE_DATASET_NAME`: the name of the dataset that contains the table you are creating the snapshot from.
   - `TABLE_NAME`: the name of the table you are creating the snapshot from.
   - `TIMESTAMP_VALUE`: A [timestamp value](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#timestamp_type) representing the date and time 24 hours from now.

   <br />

3. Click **Run**.

   <br />

For more information about how to run queries, see [Run an interactive query](https://docs.cloud.google.com/bigquery/docs/running-queries#queries).

> [!NOTE]
> **Note:** The snapshot inherits the source table's encryption key.

### bq

Enter the following command in the Cloud Shell:

[Go to Cloud Shell](https://console.cloud.google.com/bigquery?cloudshell=true)

```bash
bq cp \
--snapshot \
--no_clobber \
--expiration=86400 \
TABLE_PROJECT_ID:TABLE_DATASET_NAME.TABLE_NAME \
SNAPSHOT_PROJECT_ID:SNAPSHOT_DATASET_NAME.SNAPSHOT_NAME
```

Replace the following:

- `TABLE_PROJECT_ID`: the project ID of the project that contains the table you are creating the snapshot from.
- `TABLE_DATASET_NAME`: the name of the dataset that contains the table you are creating the snapshot from.
- `TABLE_NAME`: the name of the table you are creating the snapshot from.
- `SNAPSHOT_PROJECT_ID`: the project ID of the project in which to create the snapshot.
- `SNAPSHOT_DATASET_NAME`: the name of the dataset in which to create the snapshot.
- `SNAPSHOT_NAME`: the name of the snapshot you are creating.

<br />

The `--no_clobber` flag is required.

> [!NOTE]
> **Note:** The snapshot inherits the destination dataset's default encryption key.

### API

Call the
[`jobs.insert`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs/insert) method with
the following parameters:

| **Parameter** | **Value** |
|---|---|
| `projectId` | The project ID of the project to bill for this operation. |
| Request body | ```json { "configuration": { "copy": { "sourceTables": [ { "projectId": "TABLE_PROJECT_ID", "datasetId": "TABLE_DATASET_NAME", "tableId": "TABLE_NAME" } ], "destinationTable": { "projectId": "SNAPSHOT_PROJECT_ID", "datasetId": "SNAPSHOT_DATASET_NAME", "tableId": "SNAPSHOT_NAME" }, "operationType": "SNAPSHOT", "writeDisposition": "WRITE_EMPTY", "destinationExpirationTime":"TIMESTAMP_VALUE" } } } ``` |

Replace the following:

- `TABLE_PROJECT_ID`: the project ID of the project that contains the table you are creating the snapshot from.
- `TABLE_DATASET_NAME`: the name of the dataset that contains the table you are creating the snapshot from.
- `TABLE_NAME`: the name of the table you are creating the snapshot from.
- `SNAPSHOT_PROJECT_ID`: the project ID of the project in which to create the snapshot.
- `SNAPSHOT_DATASET_NAME`: the name of the dataset in which to create the snapshot.
- `SNAPSHOT_NAME`: the name of the snapshot you are creating.
- `TIMESTAMP_VALUE`: A [timestamp value](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#timestamp_type) representing the date and time 24 hours from now.

<br />

As with tables, if an expiration is not specified, then the table snapshot
expires after the
[default table expiration time](https://docs.cloud.google.com/bigquery/docs/managing-tables#updating_a_tables_expiration_time)
or the dataset that contains the table snapshot.

> [!NOTE]
> **Note:** Because expiring a snapshot is the same as deleting it at a later time, creating a snapshot with an expiration time requires the `bigquery.tables.deleteSnapshot` permission.

### Create a table snapshot using time travel

You can create a table snapshot of a table as it was one hour ago by using
one of the following options:

### Console

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)
3. In the **Explorer** pane, expand your project, click **Datasets**, and
   then click the dataset.

4. Click **Overview \> Tables**, and then click the name of the
   table that you want to snapshot.

5. In the details pane that appears, click **Snapshot**.

   ![A screenshot showing the Snapshot button in the BigQuery console.](https://docs.cloud.google.com/static/bigquery/images/snapshot-create.png)
6. In the **Create table snapshot** pane that appears, enter the **Project** ,
   **Dataset** , and **Table** information for the new table snapshot.

7. In the **Snapshot time** field, enter the date and time for 1 hour ago.

8. Click **Save**.

### SQL

Use the
[`CREATE SNAPSHOT TABLE` DDL statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_snapshot_table_statement)
with a [`FOR SYSTEM_TIME AS OF` clause](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#for_system_time_as_of):

<br />

<br />

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, enter the following statement:

   ```googlesql
   CREATE SNAPSHOT TABLE SNAPSHOT_PROJECT_ID.SNAPSHOT_DATASET_NAME.SNAPSHOT_NAME
   CLONE TABLE_PROJECT_ID.TABLE_DATASET_NAME.TABLE_NAME
   FOR SYSTEM_TIME AS OF
     TIMESTAMP_SUB(CURRENT_TIMESTAMP(), INTERVAL 1 HOUR);
   ```


   Replace the following:
   - `SNAPSHOT_PROJECT_ID`: the project ID of the project in which to create the snapshot.
   - `SNAPSHOT_DATASET_NAME`: the name of the dataset in which to create the snapshot.
   - `SNAPSHOT_NAME`: the name of the snapshot you are creating.
   - `TABLE_PROJECT_ID`: the project ID of the project that contains the table you are creating the snapshot from.
   - `TABLE_DATASET_NAME`: the name of the dataset that contains the table you are creating the snapshot from.
   - `TABLE_NAME`: the name of the table you are creating the snapshot from.

   <br />

3. Click **Run**.

   <br />

For more information about how to run queries, see [Run an interactive query](https://docs.cloud.google.com/bigquery/docs/running-queries#queries).

### bq

Enter the following command in the Cloud Shell:

[Go to Cloud Shell](https://console.cloud.google.com/bigquery?cloudshell=true)

```bash
bq cp \
--no_clobber \
--snapshot \
TABLE_PROJECT_ID:TABLE_DATASET_NAME.TABLE_NAME@-3600000 \
SNAPSHOT_PROJECT_ID:SNAPSHOT_DATASET_NAME.SNAPSHOT_NAME
```

Replace the following:

- `TABLE_PROJECT_ID`: the project ID of the project that contains the table you are creating the snapshot from.
- `TABLE_DATASET_NAME`: the name of the dataset that contains the table you are creating the snapshot from.
- `TABLE_NAME`: the name of the table you are creating the snapshot from.
- `SNAPSHOT_PROJECT_ID`: the project ID of the project in which to create the snapshot.
- `SNAPSHOT_DATASET_NAME`: the name of the dataset in which to create the snapshot.
- `SNAPSHOT_NAME`: the name of the snapshot you are creating.

<br />

The `--no_clobber` flag is required.

### API

Call the
[`jobs.insert`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs/insert)
method with the following parameters:

| **Parameter** | **Value** |
|---|---|
| `projectId` | The project ID of the project to bill for this operation. |
| Request body | ```json { "configuration": { "copy": { "sourceTables": [ { "projectId": "TABLE_PROJECT_ID", "datasetId": "TABLE_DATASET_NAME", "tableId": "TABLE_NAME@-360000" } ], "destinationTable": { "projectId": "SNAPSHOT_PROJECT_ID", "datasetId": "SNAPSHOT_DATASET_NAME", "tableId": "SNAPSHOT_NAME" }, "operationType": "SNAPSHOT", "writeDisposition": "WRITE_EMPTY" } } } ``` |

Replace the following:

- `TABLE_PROJECT_ID`: the project ID of the project that contains the table you are creating the snapshot from.
- `TABLE_DATASET_NAME`: the name of the dataset that contains the table you are creating the snapshot from.
- `TABLE_NAME`: the name of the table you are creating the snapshot from.
- `SNAPSHOT_PROJECT_ID`: the project ID of the project in which to create the snapshot.
- `SNAPSHOT_DATASET_NAME`: the name of the dataset in which to create the snapshot.
- `SNAPSHOT_NAME`: the name of the snapshot you are creating.

<br />

For more information about specifying a past version of a table, see
[Accessing historical data using time travel](https://docs.cloud.google.com/bigquery/docs/time-travel).

## Table access control

To control access to tables in BigQuery, see
[Control access to resources with IAM](https://docs.cloud.google.com/bigquery/docs/control-access-to-resources-iam).

When you create a table snapshot,
[table-level access](https://docs.cloud.google.com/bigquery/docs/table-access-controls-intro) to the table
snapshot is set as follows:

- If the table snapshot overwrites an existing table, then the table-level access for the existing table is maintained. [Tags](https://docs.cloud.google.com/bigquery/docs/tags) aren't copied from the base table.
- If the table snapshot is a new resource, then the table-level access for the table snapshot is determined by the access policies of the dataset in which the table snapshot is created. Additionally, [tags](https://docs.cloud.google.com/bigquery/docs/tags) are copied from the base table to the table snapshot.

## What's next

- [Update a table snapshot's description, expiration date, or
  access policy](https://docs.cloud.google.com/bigquery/docs/table-snapshots-update).
- [Restore a table snapshot](https://docs.cloud.google.com/bigquery/docs/table-snapshots-restore).
- [Create monthly snapshots of a table by using a service account that runs a scheduled query](https://docs.cloud.google.com/bigquery/docs/table-snapshots-scheduled).