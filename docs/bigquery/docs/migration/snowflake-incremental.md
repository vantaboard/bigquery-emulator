# Set up incremental transfers for Snowflake

This guide shows you how to configure incremental data transfers from
Snowflake to BigQuery. Incremental transfers let you
transfer only the data that has changed since the last transfer run, which
can reduce transfer time and costs.

## Limitations

Incremental Snowflake transfers are subject to the following
limitations:

- You must provide primary key columns to use the upsert write mode. For more information, see [Defining primary keys for incremental
  transfers](https://docs.cloud.google.com/bigquery/docs/migration/snowflake-incremental#custom_schema_file).
- Primary keys must be unique in the source table. If duplicates exist, the results of the merge operation in BigQuery might be inconsistent and not match the source data.
- The automatic handling of schema changes with incremental transfers is not supported. If the schema of a source table changes, you must manually update the BigQuery table schema.
- Incremental transfers work best when changes in your source data are concentrated within a small number of partitions. Incremental transfer performance can degrade significantly if updates are scattered across the source table, as this requires scanning many partitions. If you have many rows that are changed between data transfers, then we recommend that you use a full transfer instead.
- Some operations in Snowflake, such as `CREATE OR REPLACE TABLE` or `CLONE`, can overwrite the original table object and its associated change tracking history. This makes existing data transfers stale and requires a new full sync to resume incremental transfers.
- Incremental transfers must be run frequently enough to stay within [Snowflake's data retention period](https://docs.snowflake.com/en/user-guide/data-time-travel#data-retention-period) for change tracking. If the last successful transfer is run outside of this window, then the next transfer will be a full transfer.

## Configure incremental transfers

You can configure incremental transfers by selecting
the **Incremental** write
preference in the transfer configuration when you [set up a
Snowflake transfer](https://docs.cloud.google.com/bigquery/docs/migration/snowflake-transfer#set-up-transfer).

If you select **Incremental** for
your data transfer, you can transfer by either **Append** or **Upsert** write
modes which define how data is written to BigQuery during an
incremental data transfer. To enable the **Upsert** write mode, you must define a primary key in a [custom schema file](https://docs.cloud.google.com/bigquery/docs/migration/snowflake-incremental#custom_schema_file). If no primary key is defined, the transfer defaults to the **Append** write mode. The following sections describe the available write modes.

### Append write mode

The append write mode only inserts new rows to your destination table. This option
strictly unloads only new rows inserted in source table and appends transferred data without checking for existing records, so this mode can potentially cause data duplication in the destination table.

### Upsert write mode

The upsert write mode lets you update, insert or delete records in your
destination table using a primary key. If specified primary key is present in the destination
BigQuery table, the Snowflake connector correctly inserts, updates, or
deletes records to reflect changes. If the primary key is not present
in the destination table, the connector inserts new records for source inserts
or updates while deletes are skipped in this case.

To use the upsert write mode with your incremental data transfer, you must
[define primary keys in your custom schema file](https://docs.cloud.google.com/bigquery/docs/migration/snowflake-incremental#custom_schema_file). Note the following while choosing a primary key:

- The primary key can be one or more columns on your table that are required
  for the connector to identify records to update.

- Select columns that contain non-null values that are unique across all
  rows of the table. We recommend columns that include system-generated
  identifiers, unique reference codes (for example, auto-incrementing IDs), or
  immutable time-based sequence IDs.

- To prevent potential data loss or data corruption, the primary key columns
  that you select must have unique values. If you have doubts about the
  uniqueness of your chosen primary key column, then we recommend that you
  use the append write mode or full ingestion instead.

## Schema changes behavior

Incremental transfers don't support schema evolution. Any modifications to the
source table schema, such as adding, removing, renaming columns, or changing
column data types cause subsequent incremental transfer runs to fail. If a
source schema change occurs, you must run a full data transfer to resynchronize the
data.

## Custom schema file for incremental transfers

You can use a custom schema file to define primary keys for incremental
transfers and to customize schema mapping. A custom schema file is a JSON file
that describes the source and target schema.

For incremental transfers in **Upsert** mode, you must identify one or more
columns as primary keys. To do this, annotate the columns with the
`PRIMARY_KEY` usage type in the custom schema file.

The following example shows a custom schema file that defines `O_ORDERKEY` and
`O_ORDERDATE` as primary keys for the `orders` table:


    {
      "databases": [
        {
          "name": "my_db",
          "originalName": "my_db",
          "tables": [
            {
              "name": "orders",
              "originalName": "orders",
              "columns": [
                {
                  "name": "O_ORDERKEY",
                  "originalName": "O_ORDERKEY",
                  "usageType": [
                    "PRIMARY_KEY"
                  ]
                },
                {
                  "name": "O_ORDERDATE",
                  "originalName": "O_ORDERDATE",
                  "usageType": [
                    "PRIMARY_KEY"
                  ]
                }
              ]
            }
          ]
        }
      ]
    }

## Enable change tracking

Before you can set up an incremental Snowflake transfer,
you must enable change tracking on each source table with the following command:

```
ALTER TABLE DATABASE_NAME.SCHEMA_NAME.TABLE_NAME SET CHANGE_TRACKING = TRUE;
```

If change tracking is not enabled for a table, then the Snowflake
connector defaults to a full data transfer for that table.

## What's next

Once you have configured all the steps required for an incremental Snowflake
transfer, you can enable incremental transfers for your
Snowflake transfer configuration. For more information, see
[Set up a Snowflake transfer](https://docs.cloud.google.com/bigquery/docs/migration/snowflake-transfer#set-up-transfer).