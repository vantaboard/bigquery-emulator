# Introduction to table snapshots

This document is an introduction to BigQuery table snapshots.
It is the first of a set of documents that describes how to work with
BigQuery table snapshots, including
how to create, restore, update, get information about, and query table
snapshots. This document set is intended for users who are familiar with
[BigQuery](https://docs.cloud.google.com/bigquery/docs)
and BigQuery [tables](https://docs.cloud.google.com/bigquery/docs/tables-intro).

## Table snapshots

A BigQuery table snapshot preserves the contents of a table
(called the *base table* ) at a particular
time. You can save a snapshot of a current table, or create a snapshot of a
table as it was at any time in the past seven days. A table snapshot can have an
expiration; when the configured amount of time has passed since the table
snapshot was created,
BigQuery
deletes the table snapshot. You can query a table snapshot as you would a
standard
table. Table snapshots are read-only, but you can create (*restore*) a standard
table from a table snapshot, and then you can modify the restored table.

Benefits of using table snapshots include the following:

- **Keep a record for longer than seven days.** With BigQuery
  [time travel](https://docs.cloud.google.com/bigquery/docs/time-travel), you can only access a table's data
  from seven days ago or more recently. With table snapshots, you can
  preserve a table's data from a specified point in time for as long as you want.

- **Minimize storage cost.** BigQuery only stores bytes that are
  different between a
  snapshot and its base table, so a table snapshot typically uses less storage
  than a full copy of the table.

If you need mutable, lightweight copies of your tables, consider using
[table clones](https://docs.cloud.google.com/bigquery/docs/table-clones-intro).

## Access control for table snapshots

Access control for table snapshots is similar to access control for tables.
For more information, see
[Control access to resources with IAM](https://docs.cloud.google.com/bigquery/docs/control-access-to-resources-iam).

## Querying table snapshots

You query a table snapshot's data in the same way as you query other types
of BigQuery tables. For more information, see
[Querying BigQuery data](https://docs.cloud.google.com/bigquery/docs/query-overview).

## Storage costs

[Storage costs](https://cloud.google.com/bigquery/pricing#storage) apply for table snapshots,
but BigQuery only charges for the data in a table
snapshot that is not already charged to another table:

- When a table snapshot is created, there is initially no storage cost for the
  table snapshot.

- If new data is added to the base table after the table snapshot was
  created, then you don't pay for storage of that data in the table snapshot.

- When you take a snapshot, the storage type of the snapshot is the same as the
  storage type of the source data. For example, if you take a snapshot of a
  table that is classified as active storage, the storage type is active for
  the snapshot. Similarly, if the base table is classified as long-term
  storage, the storage type is long-term for the snapshot.

- If data is changed or deleted in the base table that also exists in a table
  snapshot, the following charges occur:

  - You are charged for the table snapshot storage of the changed
    or deleted data.

  - If the base table is billed as physical storage, time travel
    and failsafe charges aren't charged to the base table. When the snapshot
    is deleted, you are charged for time travel and failsafe.

  - If there are multiple snapshots that contain the changed or
    deleted data, you are only charged for the storage used by the oldest
    snapshot.

- When you copy a table snapshot or clone within a same region or from one
  region or multi-region to another, a full copy of the table is created. This
  incurs additional [storage costs](https://cloud.google.com/bigquery/pricing).

The difference between base table and table snapshot storage charges is shown
in the following image:

![Table snapshot billing example](https://docs.cloud.google.com/static/bigquery/images/table-snapshot-billing.png)

> [!NOTE]
> **Note:**
> - Because BigQuery storage is column-based, small changes to the data in a base table can result in large increases in storage cost for its table snapshot.
> - Some changes to a base table can result in you being charged the full storage amount for a table snapshot of the table. For example, if you modify a base table with [clustering](https://docs.cloud.google.com/bigquery/docs/manage-clustered-tables#modifying-cluster-spec), that can lead to automatic re-clustering. Because re-clustering can rewrite the base table's storage blocks, the base table's storage is no longer the same as the storage of its snapshots. This might cause the oldest of the base table's snapshots to be charged up to the full storage amount of the modified partition.
> - Partitions can help reduce storage costs for table snapshots. In general, BigQuery only makes a copy of modified data within a partition, instead of the entire table snapshot.

For more information, see
[BigQuery storage pricing](https://cloud.google.com/bigquery/pricing#storage).

## Limitations

- A table snapshot must be in the same [region](https://docs.cloud.google.com/bigquery/docs/locations), and
  under the same
  [organization](https://docs.cloud.google.com/resource-manager/docs/creating-managing-organization),
  as its base table. If you select a dataset in a different region,
  BigQuery creates a copy of the table in the target dataset in
  that region.

- Table snapshots are read-only; you can't update the data in a table snapshot
  unless you create a standard table from the snapshot and then update the data.
  You can only update a table snapshot's metadata; for example, its description,
  expiration date, and access policy.

- You can only take a snapshot of a table's data as it was seven days ago or
  more recently, due to the seven-day limit for
  [time travel](https://docs.cloud.google.com/bigquery/docs/time-travel).

- You can't take a snapshot of a view or a materialized view.

- You can't take a snapshot of an [external table](https://docs.cloud.google.com/bigquery/docs/external-tables).

- You can't overwrite an existing table or table snapshot when you create a
  table snapshot.

- If you snapshot a table that has data in
  [write-optimized storage (streaming buffer)](https://docs.cloud.google.com/bigquery/docs/streaming-data-into-bigquery#dataavailability),
  the data in the write-optimized storage is not included in the table snapshot.

- If you snapshot a table that has data in
  [time travel](https://docs.cloud.google.com/bigquery/docs/time-travel),
  the data in time travel is not included in the table snapshot.

- If you snapshot a partitioned table that has a
  [partition expiration](https://docs.cloud.google.com/bigquery/docs/managing-partitioned-tables#partition-expiration)
  set, the partition expiration information isn't retained in the snapshot.
  The snapshotted table uses the destination dataset's default partition
  expiration instead.
  To retain the partition expiration information,
  [copy the table](https://docs.cloud.google.com/bigquery/docs/managing-tables#copy-table) instead.

## Quotas and limits

For information about the quotas and limits that apply for table snapshots, see
[Table snapshots quotas and limits](https://docs.cloud.google.com/bigquery/quotas#table_snapshots).

## What's next

- [Create a table snapshot](https://docs.cloud.google.com/bigquery/docs/table-snapshots-create).
- [Restore a table snapshot](https://docs.cloud.google.com/bigquery/docs/table-snapshots-restore).
- [Update a table snapshot's description, expiration date, or
  access policy](https://docs.cloud.google.com/bigquery/docs/table-snapshots-update).
- [Create monthly snapshots of a table by using a service account that runs a scheduled query](https://docs.cloud.google.com/bigquery/docs/table-snapshots-scheduled).
- [Automate snapshots at the dataset level](https://github.com/GoogleCloudPlatform/bigquery-utils/tree/master/tools/cloud_functions/bq_table_snapshots).