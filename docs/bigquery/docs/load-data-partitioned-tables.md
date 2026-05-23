# Load data into partitioned tables

This document describes how to load data into partitioned tables.

## Write data to a specific partition

You can load data to a specific partition by using the
[`bq load`](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#bq_load) command with a
partition decorator. The following example appends data into the `20160501` (May
1, 2016) partition of an existing table, assuming the table is already
partitioned by date:

```bash
bq load --source_format=CSV 'my_dataset.my_table$20160501' data.csv
```

You can also write the results of a query to a specific partition:

```bash
bq query \
  --use_legacy_sql=false  \
  --destination_table='my_table$20160501' \
  --append_table=true \
  'SELECT * FROM my_dataset.another_table'
```

With ingestion-time partitioning, you can use this technique to load older data
into the partition that corresponds to the time when the data was originally
created.

You can also use this technique to adjust for time zones. By default, ingestion-
time partitions are based on UTC time. If you want the partition time to match a
particular time zone, you can use partition decorators to offset the UTC
ingestion time. For example, if you are on Pacific Standard Time (PST), you can
load data that was generated on May 1, 2016 23:30 PST into the partition for
that date by using the corresponding explicit partition decorator,
`$2016050123`. If you didn't use this explicit decorator, it would instead load
into `$2016050207` (May 2nd 07:00 UTC).

For time-unit column and integer-range partitioned tables, the partition ID
specified in the decorator must match the data being written. For example, if
the table is partitioned on a `DATE` column, the decorator must match the value
in that column. Otherwise, an error occurs. However, if you know beforehand that
your data is in a single partition, specifying the partition decorator can
improve write performance.

The preceding example appends data to a partition. To overwrite data in a
partition instead, you must include different flags for each command, namely
`bq load --replace=true ...` and `bq query --append_table=false ...`.
For more information about the flags in these commands, see [`bq load`](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#bq_load)
and [`bq query`](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#bq_query).

For more information on loading data, see
[Introduction to loading data into BigQuery](https://docs.cloud.google.com/bigquery/docs/loading-data).

## Stream data into partitioned tables

For information about streaming data into a partitioned table with the
BigQuery Storage Write API, see
[Stream into partitioned tables](https://docs.cloud.google.com/bigquery/docs/write-api#stream_into_partitioned_tables).

## What's next

To learn more about working with partitioned tables, see:

- [Creating partitioned tables](https://docs.cloud.google.com/bigquery/docs/creating-partitioned-tables)
- [Managing partitioned tables](https://docs.cloud.google.com/bigquery/docs/managing-partitioned-tables)
- [Querying partitioned tables](https://docs.cloud.google.com/bigquery/docs/querying-partitioned-tables)