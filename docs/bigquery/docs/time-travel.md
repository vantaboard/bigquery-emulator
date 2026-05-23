# Data retention with time travel and fail-safe

This document describes *time travel* and *fail-safe* data retention windows for
datasets. During the time travel and fail-safe periods, data that you have
changed or deleted in any table in the dataset continues to be stored in case
you need to recover it.

## Time travel and data retention

You can access changed or deleted data from any point within the time travel
window, which covers the past seven days by default. Time travel lets you [query
data that was updated or deleted](https://docs.cloud.google.com/bigquery/docs/access-historical-data),
restore a [table](https://docs.cloud.google.com/bigquery/docs/restore-deleted-tables) or
[dataset](https://docs.cloud.google.com/bigquery/docs/restore-deleted-datasets) that was deleted, restore
a [table that expired](https://docs.cloud.google.com/bigquery/docs/managing-tables#updating_a_tables_expiration_time),
or [restore a table to a point in time](https://docs.cloud.google.com/bigquery/docs/access-historical-data#restore-a-table).

You can set the duration of the time travel window, from a minimum of two days
to a maximum of seven days. A longer time travel window is useful in cases where
it is important to be able to recover updated or deleted data. A shorter time
travel window lets you save on storage costs when using the [physical storage
billing model](https://docs.cloud.google.com/bigquery/docs/datasets-intro#dataset_storage_billing_models).
These savings don't apply when using the logical storage billing model. For more
information on how the storage billing model affects cost, see
[Billing](https://docs.cloud.google.com/bigquery/docs/time-travel#billing). You can't set the time travel
duration for less than 2 days.

## Configure the time travel window

You set the time travel window at the dataset or project level. These settings
then apply to all tables associated with the dataset or project.

### Set the project-level time travel window

To specify the project-level default time travel window, you can use data
definition language (DDL) statements. To learn how to set the project-level time
travel window, see [Manage configuration settings](https://docs.cloud.google.com/bigquery/docs/default-configuration).

### Set the dataset-level time travel window

To specify or modify the time travel window for a dataset, you can use the
Google Cloud console, the bq command-line tool, or the BigQuery API.

- To specify the default time travel window for new datasets, see [Create
  datasets](https://docs.cloud.google.com/bigquery/docs/datasets#create-dataset).
- To modify or update the time travel window for an existing dataset, see [Update time travel windows](https://docs.cloud.google.com/bigquery/docs/updating-datasets#update_time_travel_windows).

When modifying a time travel window, if the timestamp specifies a time outside
the time travel window, or from before the table was created, then the query
fails and returns an error like the following:

    Table ID was created at time which is
    before its allowed time travel interval timestamp. Creation
    time: timestamp

## How time travel works

BigQuery uses a columnar storage format. This means that data is
organized and stored by column rather than by row. When you have a table with
multiple columns, the values for each column across all rows are stored together
in storage blocks.

When you modify a cell in a BigQuery table, you are changing a
specific value within a particular row and a specific column. Because
BigQuery stores columns together, modifying even a single cell
within a column typically requires reading the entire storage block containing
that column's data for the affected rows, applying the change, and then writing
a new version of that storage block.

The time travel feature works by tracking the versions of storage blocks that
make up your table. When you update data, BigQuery doesn't just
modify the existing storage block in place. Instead, it creates a new version of
the affected storage blocks with the updated data. The previous version is
then retained for time travel purposes.

BigQuery uses adaptive file sizes and storage blocks. The size of
storage blocks is not fixed but can vary depending on factors like the size of
the table and its data distribution. Changing even one cell in a storage block
changes the data for that column, potentially affecting many rows. Therefore,
the unit of data that is versioned and sent to time travel is often the entire
storage block that contains the modified data of that column, not just a single
cell.

For this reason, changing one cell can result in more data being sent to time
travel than just the size of the change.

### How the time travel window affects table and dataset recovery

A deleted table or dataset uses the time travel window duration that was in
effect at the time of deletion.

For example, if you have a time travel window duration of two days and then
increase the duration to seven days, tables deleted before that change are still
only recoverable for two days. Similarly, if you have a time travel window
duration of five days and you reduce that duration to three days, any tables
that were deleted before the change are still recoverable for five days.

Because time travel windows are set at the dataset level, you can't change the
time travel window of a deleted dataset until it is undeleted.

If you reduce the time travel window duration, delete a table, and then
realize that you need a longer period of recoverability for that data, you can
create a snapshot of the table from a point in time prior to the table deletion.
You must do this while the deleted table is still recoverable.
For more information, see
[Create a table snapshot using time travel](https://docs.cloud.google.com/bigquery/docs/table-snapshots-create#create_a_table_snapshot_using_time_travel).

### Time travel and row-level access

If a table has, or has had, [row-level access
policies](https://docs.cloud.google.com/bigquery/docs/row-level-security-intro), only a principal that is
granted the following [Identity and Access Management (IAM)](https://docs.cloud.google.com/bigquery/docs/access-control)
permission can access historical data for the table:

| **Permission** | **Resource** |
|---|---|
| [`bigquery.rowAccessPolicies.overrideTimeTravelRestrictions`](https://docs.cloud.google.com/bigquery/docs/access-control#bigquery.rowAccessPolicies.overrideTimeTravelRestrictions) | The table whose historical data is being accessed |

The following predefined IAM roles provide
`bigquery.rowAccessPolicies.overrideTimeTravelRestrictions`
permission:

| **Role** | **Resource** |
|---|---|
| [`roles/bigquery.admin`](https://docs.cloud.google.com/bigquery/docs/access-control#bigquery.admin) | The table whose historical data is being accessed |
| [`roles/bigquery.studioAdmin`](https://docs.cloud.google.com/bigquery/docs/access-control#bigquery.studioAdmin) | The table whose historical data is being accessed |
| [`roles/iam.databasesAdmin`](https://docs.cloud.google.com/iam/docs/roles-permissions/iam#iam.databasesAdmin) | The table whose historical data is being accessed |

You can also grant
`bigquery.rowAccessPolicies.overrideTimeTravelRestrictions`
permission using a [custom role](https://docs.cloud.google.com/iam/docs/creating-custom-roles).

> [!NOTE]
> **Note:** The **`roles/owner`** role does not contain all the permissions present in the table administrator roles, so you must grant one of these table administrator roles to any user who restores tables that have or had row-level access policies applied to them.

- Run the following command to get the equivalent Unix epoch time by passing the
  UTC timestamp:

  ```googlesql
  date -d '2023-08-04 16:00:34.456789Z' +%s000
  ```
- Replace the UNIX epoch time `1691164834000` received from the previous command
  in the bq command-line tool. Run the following command to restore a copy of the deleted
  table `deletedTableID` in another table `restoredTable`, within the same
  dataset `myDatasetID`:

  ```googlesql
  bq cp myProjectID:myDatasetID.deletedTableID@1691164834000 myProjectID:myDatasetID.restoredTable
  ```

## Fail-safe

BigQuery provides a fail-safe period. During the fail-safe period,
deleted data is automatically retained for an additional seven days after the
time travel window, so that the data is available for emergency recovery. Data
is recoverable at the table level. Data is recovered for a table from the point
in time represented by the timestamp of when that table was deleted.
The fail-safe period is not configurable and can't be extended.

When you perform the following operations, the data that is replaced or removed
can be recovered through the time travel window. After the time
travel window ends, this data then enters the fail-safe period for extended
recovery time:

- **Table deletion or replacement:** When a table is deleted, or when its data is fully replaced (for example, by using the [`WRITE_TRUNCATE`](https://docs.cloud.google.com/bigquery/docs/reference/auditlogs/rest/Shared.Types/BigQueryAuditMetadata.WriteDisposition) write disposition in a load job or by using the [`CREATE OR REPLACE TABLE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_table_statement) statement), the previous contents of the table are retained.
- **Partition deletion:** If a specific partition is deleted from a [partitioned table](https://docs.cloud.google.com/bigquery/docs/partitioned-tables), the data belonging to that specific partition is retained. Other partitions in the table aren't affected.

You can't query or directly recover data in fail-safe storage. To recover data
from fail-safe storage, contact [Cloud Customer Care](https://cloud.google.com/support-hub).

> [!WARNING]
> **Warning:** Once the fail-safe period has passed, Cloud Customer Care can't recover any of your deleted data.

## Billing

If you set your
[storage billing model](https://docs.cloud.google.com/bigquery/docs/datasets-intro#dataset_storage_billing_models)
to use physical bytes, you are billed separately for the
bytes used for time travel and fail-safe storage. Time travel and fail-safe
storage are charged at the active physical storage rate. You can
[configure the time travel window](https://docs.cloud.google.com/bigquery/docs/time-travel#configure_the_time_travel_window) to balance
storage costs with your data retention needs.

If you set your storage billing model to use logical bytes, the total storage
costs for time travel and fail-safe storage are included in the base rate that
you are charged.

The following table show a comparison of physical and logical storage costs:

| **Billing model** | **What do you pay for?** |
|---|---|
| Physical (compressed) storage | - You pay for active bytes - You pay for long-term storage - You pay for time travel storage - You pay for fail-safe storage |
| Logical (uncompressed) storage (default setting) | - You pay for active storage - You pay for long-term storage - You don't pay for time travel storage - You don't pay for fail-safe storage |

If you use physical storage, you can see the bytes used by time travel and
fail-safe by looking at the `TIME_TRAVEL_PHYSICAL_BYTES` and
`FAIL_SAFE_PHYSICAL_BYTES` columns in the
[`TABLE_STORAGE`](https://docs.cloud.google.com/bigquery/docs/information-schema-table-storage) and
[`TABLE_STORAGE_BY_ORGANIZATION`](https://docs.cloud.google.com/bigquery/docs/information-schema-table-storage-by-organization)
views. For an example of how to use one of these views to estimate your costs,
see
[Forecast storage billing](https://docs.cloud.google.com/bigquery/docs/information-schema-table-storage#forecast_storage_billing).

[Storage costs](https://cloud.google.com/bigquery/pricing#storage) apply for time travel and fail-safe
data, but you are only billed if data storage fees don't apply elsewhere in
BigQuery. The following details apply:

- When a table is created, there is no time travel or fail-safe storage cost.
- If data is changed or deleted, then you are charged for the storage of the changed or deleted data saved by time travel during the time travel window and the fail-safe period. This is similar to the storage pricing for table snapshots and clones.
- Temporary tables aren't billed for fail-safe storage.

## Data retention example

The following table shows how deleted or changed data moves between
storage retention windows. This example shows a situation where the total active
storage is 200 GiB and 50 GiB is deleted with a time travel
window of seven days:

|   | Day 0 | Day 1 | Day 2 | Day 3 | Day 4 | Day 5 | Day 6 | Day 7 | Day 8 | Day 9 | Day 10 | Day 11 | Day 12 | Day 13 | Day 14 | Day 15 |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| Active storage | 200 | 150 | 150 | 150 | 150 | 150 | 150 | 150 | 150 | 150 | 150 | 150 | 150 | 150 | 150 | 150 |
| Time travel storage |   | 50 | 50 | 50 | 50 | 50 | 50 | 50 |   |   |   |   |   |   |   |   |
| Fail-safe storage |   |   |   |   |   |   |   |   | 50 | 50 | 50 | 50 | 50 | 50 | 50 |   |

Deleting data from long-term physical storage works in the same way.

## Limitations

Data retrieval with time travel is subject to the following limitations:

- Time travel only provides access to historical data for the duration of the time travel window. To preserve table data for non-emergency purposes for longer than the time travel window, use [table snapshots](https://docs.cloud.google.com/bigquery/docs/table-snapshots-intro).
- If a table has, or has previously had, row-level access policies, then time travel can only be used by table administrators. For more information, see [Time travel and row-level access](https://docs.cloud.google.com/bigquery/docs/time-travel#time_travel_and_row-level_access).
- Time travel does not restore table metadata.
- Time travel is not supported in the following table types:
  - [External tables](https://docs.cloud.google.com/bigquery/docs/external-tables). However, for Apache Iceberg external tables, you can use the [`FOR SYSTEM_TIME AS OF` clause](https://docs.cloud.google.com/bigquery/docs/access-historical-data#query_data_at_a_point_in_time) to access snapshots that are retained in your Iceberg metadata.
  - [Temporary cached query result tables](https://docs.cloud.google.com/bigquery/docs/cached-results).
  - [Temporary session tables](https://docs.cloud.google.com/bigquery/docs/sessions-intro).
  - [Temporary multi-statement tables](https://docs.cloud.google.com/bigquery/docs/multi-statement-queries).
  - Tables listed under external datasets.

## What's next

- Learn how to [query and recover time travel data](https://docs.cloud.google.com/bigquery/docs/access-historical-data).
- Learn more about [table snapshots](https://docs.cloud.google.com/bigquery/docs/table-snapshots-intro).