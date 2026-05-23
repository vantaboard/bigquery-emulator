# Update table snapshot metadata

This document describes how to update the description, expiration date, or
access policy for a table
snapshot by using the Google Cloud console, the [`bq update`](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#bq_update)
command, or the
[`tables.patch`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables/patch) API.
It is intended for users who are familiar with
[tables](https://docs.cloud.google.com/bigquery/docs/tables-intro) and
[table snapshots](https://docs.cloud.google.com/bigquery/docs/table-snapshots-intro) in BigQuery.

## Permissions and roles

This section describes the
[Identity and Access Management (IAM) permissions](https://docs.cloud.google.com/bigquery/docs/access-control#bq-permissions)
that you need to update the metadata for a table snapshot, and the
[predefined IAM roles](https://docs.cloud.google.com/bigquery/docs/access-control#bigquery)
that grant those permissions.

### Permissions

To update a table snapshot's metadata, you need the following permission:

| **Permission** | **Resource** |
|---|---|
| `bigquery.tables.update` | The table snapshot |

### Roles

The predefined BigQuery roles that provide the required
permission are as follows:

| **Role** | **Resource** |
|---|---|
| Any of the following: `bigquery.dataEditor` `bigquery.dataOwner` `biguqery.admin` | The table snapshot |

## Limitations

You can update a table snapshot's metadata, but you can't update its data
because table snapshot data is read only. To update a table snapshot's data,
you must first restore the table snapshot to a standard table, and then update
the standard table's data. For more information, see
[Restoring table snapshots](https://docs.cloud.google.com/bigquery/docs/table-snapshots-restore).

## Update a table snapshot's metadata

You can change a table snapshot's description, expiration, and access policies
in the same way as you change a standard table's metadata. Some examples are
provided in the following sections.

### Update the description

You can change the description for a table snapshot by using one of the
following options:

### Console

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)

   If you don't see the left pane, click **Expand left pane** to open the pane.
3. In the **Explorer** pane, expand your project, click **Datasets**, and
   then click the dataset that has the table snapshot.

4. Click **Overview \> Tables**, and then click the name of the
   table snapshot that you want to update.

5. Go to the **Details** tab, and then click **Edit Details**.

6. In the **Description** field, add or update the description for the
   table snapshot.

7. Click **Save**.

### bq

Enter the following command in the Cloud Shell:

[Go to Cloud Shell](https://console.cloud.google.com/bigquery?cloudshell=true)

```bash
bq update \
--description="DESCRIPTION" \
PROJECT_ID:DATASET_NAME.SNAPSHOT_NAME
```

Replace the following:

- `DESCRIPTION`: text describing the snapshot. For example, `Snapshot after table schema change X.`.
- `PROJECT_ID`: the project ID of the project that contains the snapshot.
- `DATASET_NAME`: the name of the dataset that contains the snapshot.
- `SNAPSHOT_NAME`: the name of the snapshot.

<br />

### API

Call the
[`tables.patch`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables/patch)
method with the following parameters:

| **Parameter** | **Value** |
|---|---|
| `projectId` | The project ID of the project that contains the snapshot. |
| `datasetId` | The name of the dataset that contains the snapshot. |
| `tableId` | The name of the snapshot. |
| Request body `description` field | Text describing the snapshot. For example, `Snapshot after table schema change X`. |

Prefer the `tables.patch` method over the `tables.update` method because the
`tables.update` method replaces the entire `Table` resource.

### Update the expiration

You can change the expiration of a table snapshot by using one of the
following options:

### Console

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)
3. In the **Explorer** pane, expand your project, click **Datasets**, and
   then click the dataset that has the table snapshot.

4. Click **Overview \> Tables**, and then click the name of the
   table snapshot that you want to update.

5. Go to the **Details** tab and then click **Edit Details**.

6. In the **Expiration time** field, enter the new expiration time for the
   table snapshot.

7. Click **Save**.

### bq

Enter the following command in the Cloud Shell:

[Go to Cloud Shell](https://console.cloud.google.com/bigquery?cloudshell=true)

```bash
bq update \
--expiration=EXPIRATION_TIME \
PROJECT_ID:DATASET_NAME.SNAPSHOT_NAME
```

Replace the following:

- `EXPIRATION_TIME`: the number of seconds from the current time to the expiration time.
- `PROJECT_ID`: the project ID of the project that contains the snapshot.
- `DATASET_NAME`: the name of the dataset that contains the snapshot.
- `SNAPSHOT_NAME`: the name of the snapshot.

<br />

### API

Call the
[`tables.patch`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables/patch)
method with the following parameters:

| **Parameter** | **Value** |
|---|---|
| `projectId` | The project ID of the project that contains the snapshot. |
| `datasetId` | The name of the dataset that contains the snapshot. |
| `tableId` | The name of the snapshot. |
| Request body `expirationTime` field | The time when the snapshot expires, in milliseconds since the epoch. |

Prefer the `tables.patch` method over the `tables.update` method because the
`tables.update` method replaces the entire `Table` resource.

### Update access

You can give a user access to view the data in a table snapshot by using
one of the following options:

### Console

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)
3. In the **Explorer** pane, expand your project, click **Datasets**, and
   then click the dataset that has the table snapshot.

4. Click **Overview \> Tables**, and then click the name of the
   table snapshot that you want to share.

5. In the snapshot pane that appears, click **Share** , then click **Add
   principal**.

6. In the **Add principals** pane that appears, enter the identifier of the
   [principal](https://docs.cloud.google.com/iam/docs/principals-overview) you want to
   give access to the table snapshot.

7. In the **Select a role** dropdown, choose **BigQuery** , then
   **BigQuery Data Viewer**.

8. Click **Save**.

### bq

Enter the following command in the Cloud Shell:

[Go to Cloud Shell](https://console.cloud.google.com/bigquery?cloudshell=true)

```bash
bq add-iam-policy-binding \
    --member="user:PRINCIPAL" \
    --role="roles/bigquery.dataViewer" \
    PROJECT_ID:DATASET_NAME.SNAPSHOT_NAME
```

Replace the following:

- `PRINCIPAL`: the [principal](https://docs.cloud.google.com/iam/docs/principals-overview) you want to give access to the table snapshot.
- `PROJECT_ID`: the project ID of the project that contains the snapshot.
- `DATASET_NAME`: the name of the dataset that contains the snapshot.
- `SNAPSHOT_NAME`: the name of the snapshot.

<br />

### API

Call the
[`tables.setIamPolicy`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables/setIamPolicy)
method with the following parameters:

| **Parameter** | **Value** |
|---|---|
| `Resource` | ```json projects/PROJECT_ID/datasets/DATASET_NAME/tables/SNAPSHOT_NAME ``` |
| Request body | ```json { "policy": { "bindings": [ { "members": [ "user:PRINCIPAL" ], "role": "roles/bigquery.dataViewer" } ] } } ``` |

Replace the following:

- `PROJECT_ID`: the project ID of the project that contains the snapshot.
- `DATASET_NAME`: the name of the dataset that contains the snapshot.
- `SNAPSHOT_NAME`: the name of the snapshot.
- `PRINCIPAL`: the [principal](https://docs.cloud.google.com/iam/docs/overview#concepts_related_identity) you want to give access to the table snapshot.

<br />

## What's next

- [List the table snapshots in a dataset](https://docs.cloud.google.com/bigquery/docs/table-snapshots-list).
- [View the metadata for a table snapshot](https://docs.cloud.google.com/bigquery/docs/table-snapshots-metadata).
- [Delete a table snapshot](https://docs.cloud.google.com/bigquery/docs/table-snapshots-delete).