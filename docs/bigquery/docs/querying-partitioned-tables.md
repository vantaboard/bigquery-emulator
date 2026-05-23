# Query partitioned tables

This document describes some specific considerations for querying
[partitioned tables](https://docs.cloud.google.com/bigquery/docs/partitioned-tables) in BigQuery.

For general information on running queries in BigQuery, see
[Running interactive and batch queries](https://docs.cloud.google.com/bigquery/docs/running-queries).

## Overview

If a query uses a qualifying filter on the value of the partitioning column,
BigQuery can scan the partitions that match the filter and skip
the remaining partitions. This process is called *partition pruning*.

Partition pruning is the mechanism BigQuery uses to eliminate
unnecessary partitions from the input scan. The pruned partitions are not
included when calculating the bytes scanned by the query. In general, partition
pruning helps reduce query cost.

Pruning behaviors vary for the different types of partitioning, so you could
see a difference in bytes processed when querying tables that are partitioned
differently but are otherwise identical. To estimate how many bytes a
query will process, perform a [dry run](https://docs.cloud.google.com/bigquery/docs/running-queries#dry-run).

## Query a time-unit column-partitioned table

To prune partitions when you query a
[time-unit column-partitioned table](https://docs.cloud.google.com/bigquery/docs/partitioned-tables#date_timestamp_partitioned_tables),
include a filter on the partitioning column.

In the following example, assume that `dataset.table` is partitioned on the
`transaction_date` column. The example query prunes dates before `2016-01-01`.

```googlesql
SELECT * FROM dataset.table
WHERE transaction_date >= '2016-01-01'
```

## Query an ingestion-time partitioned table

[Ingestion-time partitioned tables](https://docs.cloud.google.com/bigquery/docs/partitioned-tables#ingestion_time)
contain a pseudocolumn named `_PARTITIONTIME`, which is the partitioning
column. The value of the column is the UTC ingestion time for each row,
truncated to the partition boundary (such as hourly or daily), as a `TIMESTAMP`
value.

For example, if you append data on April 15, 2021, 08:15:00 UTC, the
`_PARTITIONTIME` column for those rows contains the following values:

- Hourly partitioned table: `TIMESTAMP("2021-04-15 08:00:00")`
- Daily partitioned table: `TIMESTAMP("2021-04-15")`
- Monthly partitioned table: `TIMESTAMP("2021-04-01")`
- Yearly partitioned table: `TIMESTAMP("2021-01-01")`

If the partition granularity is daily, the table also contains a pseudocolumn
named `_PARTITIONDATE`. The value is equal to `_PARTITIONTIME` truncated to a
`DATE` value.

Both of these pseudocolumn names are reserved. You can't create a column with
either name in any of your tables.

To prune partitions, filter on either of these columns. For example, the
following query scans only the partitions between the dates January 1, 2016 and
January 2, 2016:

```googlesql
SELECT
  column
FROM
  dataset.table
WHERE
  _PARTITIONTIME BETWEEN TIMESTAMP('2016-01-01') AND TIMESTAMP('2016-01-02')
```

To select the `_PARTITIONTIME` pseudocolumn, you must use an alias. For example,
the following query selects `_PARTITIONTIME` by assigning the alias `pt` to
the pseudocolumn:

```googlesql
SELECT
  _PARTITIONTIME AS pt, column
FROM
  dataset.table
```

For daily partitioned tables, you can select the `_PARTITIONDATE` pseudocolumn
in the same way:

```googlesql
SELECT
  _PARTITIONDATE AS pd, column
FROM
  dataset.table
```

The `_PARTITIONTIME` and `_PARTITIONDATE` pseudocolumns are not returned by a
`SELECT *` statement. You must select them explicitly:

```googlesql
SELECT
  _PARTITIONTIME AS pt, *
FROM
  dataset.table
```

### Handle time zones in ingestion-time partitioned tables

The value of `_PARTITIONTIME` is based on the UTC date when the field is
populated. If you want to query data based on a time zone other than UTC, choose
one of the following options:

- Adjust for time zone differences in your SQL queries.
- Use [partition decorators](https://docs.cloud.google.com/bigquery/docs/managing-partitioned-table-data#write-to-partition) to load data into specific ingestion-time partitions, based on a different time zone than UTC.

### Better performance with pseudocolumns

To improve query performance, use the `_PARTITIONTIME` pseudocolumn by itself
on the left side of a comparison.

For example, the following two queries are equivalent. Depending on the table
size, the second query might perform better, because it places `_PARTITIONTIME`
by itself on the left side of the `>` operator. Both queries process the same
amount of data.

```googlesql
-- Might be slower.
SELECT
  field1
FROM
  dataset.table1
WHERE
  TIMESTAMP_ADD(_PARTITIONTIME, INTERVAL 5 DAY) > TIMESTAMP("2016-04-15");

-- Often performs better.
SELECT
  field1
FROM
  dataset.table1
WHERE
  _PARTITIONTIME > TIMESTAMP_SUB(TIMESTAMP('2016-04-15'), INTERVAL 5 DAY);
```

To limit the partitions that are scanned in a query, use a constant expression
in your filter. The following query limits which partitions are pruned based on
the first filter condition in the `WHERE` clause. However, the second filter
condition doesn't limit the scanned partitions, because it uses table values,
which are dynamic.

```googlesql
SELECT
  column
FROM
  dataset.table2
WHERE
  -- This filter condition limits the scanned partitions:
  _PARTITIONTIME BETWEEN TIMESTAMP('2017-01-01') AND TIMESTAMP('2017-03-01')
  -- This one doesn't, because it uses dynamic table values:
  AND _PARTITIONTIME = (SELECT MAX(timestamp) from dataset.table1)
```

To limit the partitions scanned, don't include any other columns in a `_PARTITIONTIME` filter. For example, the
following query does not limit the scanned partitions, because `field1`
is a column in the table.

```googlesql
-- Scans all partitions of table2. No pruning.
SELECT
  field1
FROM
  dataset.table2
WHERE
  _PARTITIONTIME + field1 = TIMESTAMP('2016-03-28');
```

If you often query a particular range of times, consider creating a view that
filters on the `_PARTITIONTIME` pseudocolumn. For example, the following
statement creates a view that includes only the most recent seven days of data
from a table named `dataset.partitioned_table`:

```googlesql
-- This view provides pruning.
CREATE VIEW dataset.past_week AS
  SELECT *
  FROM
    dataset.partitioned_table
  WHERE _PARTITIONTIME BETWEEN
    TIMESTAMP_TRUNC(TIMESTAMP_SUB(CURRENT_TIMESTAMP, INTERVAL 7 * 24 HOUR), DAY)
    AND TIMESTAMP_TRUNC(CURRENT_TIMESTAMP, DAY);
```

For information about creating views, see [Creating views](https://docs.cloud.google.com/bigquery/docs/views).

## Query an integer-range partitioned table

To prune partitions when you query an
[integer-range partitioned table](https://docs.cloud.google.com/bigquery/docs/partitioned-tables#integer_range),
include a filter on the integer partitioning column.

In the following example, assume that `dataset.table` is an integer-range
partitioned table with a partitioning specification of `customer_id:0:100:10`
The example query scans the three partitions that start with 30, 40, and 50.

```googlesql
SELECT * FROM dataset.table
WHERE customer_id BETWEEN 30 AND 50

+---+---+
| customer_id | value |
+---+---+
|          40 |    41 |
|          45 |    46 |
|          30 |    31 |
|          35 |    36 |
|          50 |    51 |
+---+---+
```

Partition pruning is not supported for functions over an integer range
partitioned column. For example, the following query scans the entire table.

```googlesql
SELECT * FROM dataset.table
WHERE customer_id + 1 BETWEEN 30 AND 50
```

## Query data in the write-optimized storage

The `__UNPARTITIONED__` partition temporarily holds data that is streamed to a
partitioned table while it is in the
[write-optimized storage](https://docs.cloud.google.com/bigquery/docs/streaming-data-into-bigquery#dataavailability).
Data that is streamed directly to a specific partition of a partitioned table
does not use the `__UNPARTITIONED__` partition. Instead, the data is streamed
directly to the partition.

Data in the write-optimized storage has `NULL` values in the `_PARTITIONTIME` and
`_PARTITIONDATE` columns.

To query data in the `__UNPARTITIONED__` partition, use the `_PARTITIONTIME`
pseudocolumn with the `NULL` value. For example:

```googlesql
SELECT
  column
FROM dataset.table
WHERE
  _PARTITIONTIME IS NULL
```

For more information, see
[Streaming into partitioned tables](https://docs.cloud.google.com/bigquery/docs/streaming-data-into-bigquery#streaming_into_partitioned_tables).

## Best practices for partition pruning

### Use a constant filter expression

To limit the partitions that are scanned in a query, use a constant expression
in your filter. If you use dynamic expressions in your query filter,
BigQuery must scan all of the partitions.

For example, the following query prunes partitions because the filter contains a
constant expression:

```googlesql
SELECT
  t1.name,
  t2.category
FROM
  table1 AS t1
INNER JOIN
  table2 AS t2
ON t1.id_field = t2.field2
WHERE
  t1.ts = CURRENT_TIMESTAMP()
```

However, the following query doesn't prune partitions, because the filter,
`WHERE t1.ts = (SELECT timestamp from table where key = 2)`, is not
a constant expression; it depends on the dynamic values of the `timestamp` and
`key` fields:

```googlesql
SELECT
  t1.name,
  t2.category
FROM
  table1 AS t1
INNER JOIN
  table2 AS t2
ON
  t1.id_field = t2.field2
WHERE
  t1.ts = (SELECT timestamp from table3 where key = 2)
```

### Isolate the partition column in your filter

Isolate the partition column when expressing a filter. Filters that require data
from multiple fields to compute will not prune partitions. For example, a query
with a date comparison using the partitioning column and a second field, or
queries containing some field concatenations will not prune partitions.

For example, the following filter does not prune partitions because it
requires a computation based on the partitioning `ts` field and a second field
`ts2`:

`WHERE TIMESTAMP_ADD(ts, INTERVAL 6 HOUR) > ts2`

### Require a partition filter in queries

When you create a partitioned table, you can require the use of predicate
filters by enabling the **Require partition filter** option. When this option is
applied, attempts to query the partitioned table without specifying a `WHERE`
clause produce the following error:

`Cannot query over table 'project_id.dataset.table' without a filter that can be
used for partition elimination`.

This requirement also applies to queries on views and materialized views that
reference the partitioned table.

There must be at least one predicate that only references a partition column for
the filter to be considered eligible for partition elimination. For example, for
a table partitioned on column `partition_id` with an additional column `f` in
its schema, both of the following `WHERE` clauses satisfy the requirement:

    WHERE partition_id = "20221231"
    WHERE partition_id = "20221231" AND f = "20221130"

However, `WHERE (partition_id = "20221231" OR f = "20221130")` is not sufficient.

For ingestion-time partitioned tables, use either the `_PARTITIONTIME` or
`_PARTITIONDATE` pseudocolumn.

For more information about adding the **Require partition filter** option when
you create a partitioned table, see
[Creating partitioned tables](https://docs.cloud.google.com/bigquery/docs/creating-partitioned-tables).
You can also [update](https://docs.cloud.google.com/bigquery/docs/managing-partitioned-tables#require-filter)
this setting on an existing table.

## What's next

- For an overview of partitioned tables, see [Introduction to partitioned tables](https://docs.cloud.google.com/bigquery/docs/partitioned-tables).
- To learn more about creating partitioned tables, see [Creating partitioned tables](https://docs.cloud.google.com/bigquery/docs/creating-partitioned-tables).