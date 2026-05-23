# Introduction to table clones

This document gives an overview of table clones in BigQuery.
It is intended for
users who are familiar with [BigQuery](https://docs.cloud.google.com/bigquery/docs)
and BigQuery [tables](https://docs.cloud.google.com/bigquery/docs/tables-intro).

A *table clone* is a lightweight, writable copy of another table
(called the *base table*). You are only charged for storage of data in the
table clone that differs from the base table, so initially there is no storage
cost for a table clone. Other than the billing model for storage, and some
additional metadata for the base table, a table clone is similar to a
standard table---you can query it, make a copy of it, delete it, and so on.

Common use cases for table clones include the following:

- Creating copies of production tables that you can use for development and testing.
- Creating sandboxes for users to generate their own analytics and data manipulations, without physically copying all of the production data. Only the changed data is billed.

After you create a table clone, it is independent of the base table. Any changes
made to the base table or table clone aren't reflected in the other.

If you need read-only, lightweight copies of your tables, consider using
[table snapshots](https://docs.cloud.google.com/bigquery/docs/table-snapshots-intro).

## Table clone metadata

A table clone has the same metadata as a standard table, plus the following:

- The project, dataset, and name of the table clone's base table.
- The time of the table clone operation. If [time travel](https://docs.cloud.google.com/bigquery/docs/time-travel) was used to create the table clone, then this is the time travel timestamp.

For more information, see
[INFORMATION_SCHEMA.TABLES](https://docs.cloud.google.com/bigquery/docs/information-schema-tables).

## Table clone operations

In general, you use table clones in the same way as you use
[standard tables](https://docs.cloud.google.com/bigquery/docs/managing-tables), including the following
operations:

- Querying
- Access control
- Getting metadata
- Partitioning and clustering
- Working with schemas
- Deleting

However, the creation of a table clone is different from the creation of a
standard table. For more information, see
[Create table clones](https://docs.cloud.google.com/bigquery/docs/table-clones-create).

## Storage costs

[Storage costs](https://cloud.google.com/bigquery/pricing#storage) apply for table clones,
but BigQuery only charges for the data in a table
clone that is not already charged to another table:

- When a table clone is created, there is initially no storage cost for the
  table clone.

- If data is added or changed in a table clone, then you are charged for the
  storage of the added or updated data.

- When you clone a table, the storage type of the clone is the same as the
  storage type of the source data. For example, if you clone a table that is
  classified as active storage, the storage type is active for the clone.
  Similarly, if the base table is classified as long-term storage, the storage
  type is long-term for the clone.

- If data is deleted in a table clone, then you are not charged for the storage
  of the deleted data.

- If data is changed or deleted in the base table that also exists in a table
  clone, then you are charged for the table clone storage of the changed or
  deleted data. If there are multiple clones that contain the changed or
  deleted data, you are only charged for the storage used by the oldest clone.

- If data is added to the base table after the table clone was
  created, then you aren't charged for storage of that data in the table clone,
  but you are charged for it in the base table.

The difference between base table and table clone storage charges is shown in
the following image:
![Table clone billing example](https://docs.cloud.google.com/static/bigquery/images/table-clone-billing.png) Table clone billing example

> [!NOTE]
> **Note:**
> - Because BigQuery storage is column-based, small changes to the data in a base table can result in large increases in storage cost for a clone of the table.
> - Some changes to a base table can result in you being charged the full storage amount for a table clone of the table. For example, if you modify a base table with [clustering](https://docs.cloud.google.com/bigquery/docs/manage-clustered-tables#modifying-cluster-spec), that can lead to automatic re-clustering. Because re-clustering can rewrite the base table's storage blocks, the base table's storage is no longer the same as the storage of its clones. This causes the oldest of the base table's clones to be charged the full storage amount of the modified partition.
> - Partitions can help reduce storage costs for table clones. In general, BigQuery only makes a copy of modified data within a partition, instead of the entire table clone.

For more information, see
[BigQuery storage pricing](https://cloud.google.com/bigquery/pricing#storage).

## Limitations

- You can clone a table between datasets in the same project, and between datasets in different projects. However, the destination dataset for the table clone must be in the same [region](https://docs.cloud.google.com/bigquery/docs/locations), and under the same [organization](https://docs.cloud.google.com/resource-manager/docs/creating-managing-organization), as the table being cloned. For example, you cannot clone a table from an EU-based dataset into a US-based dataset.
- You can't create a clone of a table's data as it was further back than the duration of the [time travel](https://docs.cloud.google.com/bigquery/docs/time-travel) window for the table's dataset.
- You can't create a clone of a [view](https://docs.cloud.google.com/bigquery/docs/views-intro) or a [materialized view](https://docs.cloud.google.com/bigquery/docs/materialized-views-intro).
- You can't create a clone of an [external table](https://docs.cloud.google.com/bigquery/docs/external-tables).
- If you clone a table that has data in write-optimized storage (the [streaming buffer for recently streamed rows](https://docs.cloud.google.com/bigquery/docs/streaming-data-into-bigquery#dataavailability)), the data in the write-optimized storage is not included in the table clone.
- If you clone a table that has data in [time travel](https://docs.cloud.google.com/bigquery/docs/time-travel), the data in time travel is not included in the table clone.
- Table clones can't be distinguished from standard tables in the **Explorer** pane. However, you can tell a table clone from a standard table by [looking at the table details](https://docs.cloud.google.com/bigquery/docs/tables#get_table_information_using_information_schema). Table clone details have a **Base Table Info** section that standard tables don't.
- You can't use a clone operation to append data to an existing table. For example, you can't use the flag settings `--append_table=true` and `--clone=true` in the same [`bq cp`](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#bq_cp) command. To append data when duplicating a table, use a copy operation instead.
- When you create a table clone, its name must adhere to the same [naming rules](https://docs.cloud.google.com/bigquery/docs/tables#table_naming) as when you create a table.
- Table clone creation is subject to BigQuery [limits](https://docs.cloud.google.com/bigquery/quotas#copy_jobs) on copy jobs.
- The time that BigQuery takes to create table clones might vary significantly across different runs because the underlying storage is managed dynamically.

## Quotas and limits

Table clones are subject to the same quotas and limits as standard tables. For
more information, see
[table quotas and limits](https://docs.cloud.google.com/bigquery/quotas#table_limits). They also have
[table clone limits](https://docs.cloud.google.com/bigquery/quotas#table_clones) that apply.

## What's next

- [Create a table clone](https://docs.cloud.google.com/bigquery/docs/table-clones-create).