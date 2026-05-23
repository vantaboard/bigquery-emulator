# Introduction to partitioned tables

A partitioned table is divided into segments, called partitions, that make it
easier to manage and query your data. By dividing a large table into smaller
partitions, you can improve query performance and control costs by reducing the
number of bytes read by a query. You partition tables by specifying a partition
column which is used to segment the table.

If a query uses a qualifying filter on the value of the partitioning column,
BigQuery can scan the partitions that match the filter and skip
the remaining partitions. This process is called [pruning](https://docs.cloud.google.com/bigquery/docs/querying-partitioned-tables).

In a partitioned table, data is stored in physical blocks, each of which holds
one partition of data. Each partitioned table maintains various metadata about
the sort properties across all operations that modify it. The metadata lets BigQuery
more accurately estimate a query cost before the query is run.

> [!NOTE]
> **Note:** The information in [Managing table data](https://docs.cloud.google.com/bigquery/docs/managing-table-data) also applies to partitioned tables.

## When to use partitioning

Consider partitioning a table in the following scenarios:

- You want to improve the query performance by only scanning a portion of a table.
- Your table operation exceeds a [standard table quota](https://docs.cloud.google.com/bigquery/quotas#standard_tables) and you can scope the table operations to specific partition column values allowing higher [partitioned table quotas](https://docs.cloud.google.com/bigquery/quotas#partitioned_tables).
- You want to determine query costs before a query runs. BigQuery provides query cost estimates before the query is run on a partitioned table. Calculate a query cost estimate by [pruning](https://docs.cloud.google.com/bigquery/docs/querying-partitioned-tables) a partitioned table, then issuing a query dry run to estimate query costs.
- You want any of the following partition-level management features:
  - [Set a partition expiration time](https://docs.cloud.google.com/bigquery/docs/managing-partitioned-tables#partition-expiration) to automatically delete entire partitions after a specified period of time.
  - [Write data to a specific partition](https://docs.cloud.google.com/bigquery/docs/load-data-partitioned-tables#write-to-partition) using load jobs without affecting other partitions in the table.
  - [Delete specific partitions](https://docs.cloud.google.com/bigquery/docs/managing-partitioned-tables#delete_a_partition) without scanning the entire table.

Consider [clustering](https://docs.cloud.google.com/bigquery/docs/clustered-tables) a table instead of
partitioning a table in the following circumstances:

- You need more granularity than partitioning allows.
- Your queries commonly use filters or aggregation against multiple columns.
- The cardinality of the number of values in a column or group of columns is large.
- You don't need strict cost estimates before query execution.
- Partitioning results in a small amount of data per partition (approximately less than 10 GB). Creating many small partitions increases the table's metadata, and can affect metadata access times when querying the table.
- Partitioning results in a large number of partitions, exceeding the [limits on partitioned tables](https://docs.cloud.google.com/bigquery/quotas#partitioned_tables).
- Your DML operations frequently modify (for example, every few minutes) most partitions in the table.

In such cases, table clustering lets you accelerate queries by clustering
data in specific columns based on user-defined sort properties.

You can also combine clustering and table partitioning to achieve finer-grained
sorting. For more information about this approach, see [Combining clustered and partitioning tables](https://docs.cloud.google.com/bigquery/docs/partitioned-tables#combining_clustered_and_partitioned_tables).

## Types of partitioning

This section describes the different ways to partition a table.

### Integer range partitioning

You can partition a table based on ranges of values in a specific `INTEGER`
column. To create an integer-range partitioned table, you provide:

- The partitioning column.
- The starting value for range partitioning (inclusive).
- The ending value for range partitioning (exclusive).
- The interval of each range within the partition.

For example, suppose you create an integer range partition with the following
specification:

| Argument | Value |
|---|---|
| column name | `customer_id` |
| start | 0 |
| end | 100 |
| interval | 10 |

The table is partitioned on the `customer_id` column into ranges of interval 10.
The values 0 to 9 go into one partition, values 10 to 19 go into the next
partition, etc., up to 99. Values outside this range go into a partition
named `__UNPARTITIONED__`. Any rows where `customer_id` is `NULL` go into a
partition named `__NULL__`.

For information about integer-range partitioned tables, see
[Create an integer-range partitioned table](https://docs.cloud.google.com/bigquery/docs/creating-partitioned-tables#create_an_integer-range_partitioned_table).

### Time-unit column partitioning

You can partition a table on a `DATE`,`TIMESTAMP`, or `DATETIME` column in the
table. When you write data to the table, BigQuery automatically
puts the data into the correct partition, based on the values in the column.

For `TIMESTAMP` and `DATETIME` columns, the partitions can have either hourly,
daily, monthly, or yearly granularity. For `DATE` columns, the partitions can
have daily, monthly, or yearly granularity. Partition boundaries are based on
UTC time.

For example, suppose that you partition a table on a `DATETIME` column with
monthly partitioning. If you insert the following values into the table, the
rows are written to the following partitions:

| Column value | Partition (monthly) |
|---|---|
| `DATETIME("2019-01-01")` | `201901` |
| `DATETIME("2019-01-15")` | `201901` |
| `DATETIME("2019-04-30")` | `201904` |

In addition, two special partitions are created:

- `__NULL__`: Contains rows with `NULL` values in the partitioning column.
- `__UNPARTITIONED__`: Contains rows where the value of the partitioning column is earlier than 1960-01-01 or later than 2159-12-31.

For information about time-unit column-partitioned tables, see
[Create a time-unit column-partitioned table](https://docs.cloud.google.com/bigquery/docs/creating-partitioned-tables#create_a_time-unit_column-partitioned_table).

### Ingestion time partitioning

When you create a table partitioned by ingestion time, BigQuery
automatically assigns rows to partitions based on the time when
BigQuery ingests the data. You can choose hourly, daily, monthly,
or yearly granularity for the partitions. Partition boundaries are based on UTC
time.

If your data might reach the maximum number of partitions per table when using a
finer time granularity, use a coarser granularity instead. For example, you
can partition by month instead of day to reduce the number of partitions.
You can also [cluster](https://docs.cloud.google.com/bigquery/docs/clustered-tables)
the partition column to further improve performance.

An ingestion-time partitioned table has a pseudocolumn named `_PARTITIONTIME`.
The value of this column is the ingestion time for each row, truncated to the
partition boundary (such as hourly or daily). For example, suppose that you
create an ingestion-time partitioned table with hourly partitioning and send
data at the following times:

| Ingestion time | `_PARTITIONTIME` | Partition (hourly) |
|---|---|---|
| 2021-05-07 17:22:00 | 2021-05-07 17:00:00 | `2021050717` |
| 2021-05-07 17:40:00 | 2021-05-07 17:00:00 | `2021050717` |
| 2021-05-07 18:31:00 | 2021-05-07 18:00:00 | `2021050718` |

Because the table in this example uses hourly partitioning, the value of
`_PARTITIONTIME` is truncated to an hour boundary. BigQuery
uses this value to determine the correct partition for the data.

You can also write data to a specific partition. For example, you might want to
load historical data or adjust for time zones. You can use any valid date
between 0001-01-01 and 9999-12-31. However,
[DML statements](https://docs.cloud.google.com/bigquery/docs/data-manipulation-language)
cannot reference dates prior to 1970-01-01 or after 2159-12-31. For more
information, see
[Write data to a specific partition](https://docs.cloud.google.com/bigquery/docs/load-data-partitioned-tables#write-to-partition).

Instead of using `_PARTITIONTIME`, you can also use
[`_PARTITIONDATE`](https://docs.cloud.google.com/bigquery/docs/querying-partitioned-tables#query_an_ingestion-time_partitioned_table).
The `_PARTITIONDATE` pseudocolumn contains the UTC date corresponding to the value
in the `_PARTITIONTIME` pseudocolumn.

### Select daily, hourly, monthly, or yearly partitioning

When you partition a table by time-unit column or ingestion time, you choose
whether the partitions have daily, hourly, monthly, or yearly granularity.

- **Daily partitioning** is the default partitioning type. Daily partitioning is
  a good choice when your data is spread out over a wide range of dates, or if
  data is continuously added over time.

- Choose **hourly partitioning** if your tables have a high volume of data
  that spans a short date range --- typically less than six months of
  timestamp values. If you choose hourly partitioning, make sure the partition
  count stays within the
  [partition limits](https://docs.cloud.google.com/bigquery/quotas#partitioned_tables).

- Choose **monthly or yearly partitioning** if your tables have a relatively
  small amount of data for each day, but span a wide date range. This
  option is also recommended if your workflow requires frequently updating or
  adding rows that span a wide date range (for example, more than 500 dates).
  In these scenarios, use monthly or yearly partitioning along with clustering
  on the partitioning column to achieve the best performance. For more
  information, see
  [Combining clustered and partitioning tables](https://docs.cloud.google.com/bigquery/docs/partitioned-tables#combining_clustered_and_partitioned_tables)
  in this document.

## Combining clustered and partitioned tables

You can combine table partitioning with [table clustering](https://docs.cloud.google.com/bigquery/docs/clustered-tables)
to achieve fine-grained sorting for further query optimization.

A clustered table contains clustered columns that sort data based on
user-defined sort properties. Data within these clustered columns are sorted
into storage blocks which are adaptively sized based on the size of the table.
When you run a query that filters by the clustered column, BigQuery
only scans the relevant blocks based on the clustered columns instead of the
entire table or table partition. In a combined approach using both table
partitioning and clustering, you first segment table data into partitions,
then you cluster the data within each partition by the clustering columns.

When you create a table that is clustered and partitioned, you can achieve more
fine-grained sorting, as the following diagram shows:

![Comparing tables that are not clustered or partitioned to tables that are clustered and partitioned.](https://docs.cloud.google.com/static/bigquery/images/clustering-and-partitioning-tables.png)

## Partitioning versus sharding

Table sharding is the practice of storing data in multiple tables, using a
naming prefix such as `[PREFIX]_YYYYMMDD`.

Partitioning is recommended over table sharding, because partitioned tables
perform better. With sharded tables, BigQuery must maintain a
copy of the schema and metadata for each table. BigQuery might
also need to verify permissions for each queried table. This practice also adds
to query overhead and affects query performance.

If you previously created date-sharded tables, you can convert them into an
ingestion-time partitioned table. For more information, see
[Convert date-sharded tables into ingestion-time partitioned tables](https://docs.cloud.google.com/bigquery/docs/creating-partitioned-tables#convert-date-sharded-tables).

## Partition decorators

Partition decorators enable you to reference a partition in a table. For
example, you can use them to
[write data](https://docs.cloud.google.com/bigquery/docs/load-data-partitioned-tables#write-to-partition)
to a specific partition.

A partition decorator has the form `table_name$partition_id` where the format
of the `partition_id` segment depends on the type of partitioning:

| Partitioning type | Format | Example |
|---|---|---|
| Hourly | `yyyymmddhh` | `my_table$2021071205` |
| Daily | `yyyymmdd` | `my_table$20210712` |
| Monthly | `yyyymm` | `my_table$202107` |
| Yearly | `yyyy` | `my_table$2021` |
| Integer range | `range_start` | `my_table$40` |

## Browse the data in a partition

To browse the data in a specified partition, use the
[`bq head`](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#bq_head) command with a
partition decorator.

For example, the following command lists all fields in the first 10 rows of
`my_dataset.my_table` in the `2018-02-24` partition:

        bq head --max_rows=10 'my_dataset.my_table$20180224'

## Export table data

Exporting all data from a partitioned table is the same process as exporting
data from a non-partitioned table. For more information, see
[Exporting table data](https://docs.cloud.google.com/bigquery/docs/exporting-data).

To export data from an individual partition, use the `bq extract` command and
append the partition decorator to
the table name. For example, `my_table$20160201`. You can also export data from
the [`__NULL__` and `__UNPARTITIONED__`](https://docs.cloud.google.com/bigquery/docs/partitioned-tables#date_timestamp_partitioned_tables)
partitions by appending the partition names to the table name. For example,
`my_table$__NULL__` or `my_table$__UNPARTITIONED__`.

## Limitations

Partitioned tables have the following limitations:

- You cannot use legacy SQL to query partitioned tables or to write query
  results to partitioned tables.

- BigQuery does not support partitioning by multiple columns.
  Only one column can be used to partition a table.

- You cannot directly convert an existing non-partitioned table to a
  partitioned table. The partitioning strategy is defined when the table is
  created. Instead, use the [`CREATE TABLE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_table_statement)
  statement to create a new partitioned table by querying the data in the
  existing table.

- Time-unit column-partitioned tables are subject to the following
  limitations:

  - The partitioning column must be either a scalar `DATE`, `TIMESTAMP`, or `DATETIME` column. While the mode of the column can be `REQUIRED` or `NULLABLE`, it cannot be `REPEATED` (array-based).
  - The partitioning column must be a top-level field. You cannot use a leaf field from a `RECORD` (`STRUCT`) as the partitioning column.

  For information about time-unit column-partitioned tables, see
  [Create a time-unit column-partitioned table](https://docs.cloud.google.com/bigquery/docs/creating-partitioned-tables#create_a_time-unit_column-partitioned_table).
- Integer-range partitioned tables are subject to the following limitations:

  - The partitioning column must be an `INTEGER` column. While the mode of the column may be `REQUIRED` or `NULLABLE`, it cannot be `REPEATED` (array-based).
  - The partitioning column must be a top-level field. You cannot use a leaf field from a `RECORD` (`STRUCT`) as the partitioning column.

  For information about integer-range partitioned tables, see
  [Create an integer-range partitioned table](https://docs.cloud.google.com/bigquery/docs/creating-partitioned-tables#create_an_integer-range_partitioned_table).

## Quotas and limits

Partitioned tables have defined [limits](https://docs.cloud.google.com/bigquery/quotas#partitioned_tables) in BigQuery.
For more information on all quotas and limits, see [Quotas and limits](https://docs.cloud.google.com/bigquery/quotas).

### Quotas and limits for jobs against partitioned tables

Quotas and limits also apply to the different types of jobs you can run against
partitioned tables, including:

- [Loading data](https://docs.cloud.google.com/bigquery/quotas#load_jobs) (load jobs)
- [Exporting data](https://docs.cloud.google.com/bigquery/quotas#export_jobs) (extract jobs)
- [Querying data](https://docs.cloud.google.com/bigquery/quotas#query_jobs) (query jobs)
- [Copying tables](https://docs.cloud.google.com/bigquery/quotas#copy_jobs) (copy jobs)

### Number of partition modifications for column-partitioned tables quota errors

BigQuery returns this error when your column-partitioned table
reaches the
quota of the number of partition modifications permitted per day.
Partition modifications include the total of all [load jobs](https://docs.cloud.google.com/bigquery/quotas#load_jobs),
[copy jobs](https://docs.cloud.google.com/bigquery/quotas#copy_jobs), and [query jobs](https://docs.cloud.google.com/bigquery/quotas#query_jobs)
that append or overwrite a destination partition.

To see the value of the **Number of partition
modifications per column-partitioned table per day** limit, see [Partitioned
tables](https://docs.cloud.google.com/bigquery/quotas#partitioned_tables).

**Error message**

```
Quota exceeded: Your table exceeded quota for
Number of partition modifications to a column partitioned table
```

#### Resolution

This quota cannot be increased. To resolve this quota error, do the following:

- Change the partitioning on the table to have more data in each partition, in order to decrease the total number of partitions. For example, change from [partitioning by day to partitioning by month](https://docs.cloud.google.com/bigquery/docs/partitioned-tables#select_daily_hourly_monthly_or_yearly_partitioning) or change [how you partition the table](https://docs.cloud.google.com/bigquery/docs/partitioned-tables).
- Use [clustering](https://docs.cloud.google.com/bigquery/docs/clustered-tables#when_to_use_clustering) instead of partitioning.
- If you frequently load data from multiple small files stored in Cloud Storage that uses a job per file, then combine multiple load jobs into a single job. You can load from multiple Cloud Storage URIs with a comma-separated list (for example, `gs://my_path/file_1,gs://my_path/file_2`), or by using wildcards (for example, `gs://my_path/*`).


  For more information, see
  [Batch loading data](https://docs.cloud.google.com/bigquery/docs/batch-loading-data#permissions-load-data-from-cloud-storage).
- If you use load, select or copy jobs to append single rows of data to a table, for example, then you should consider batching multiple jobs into one job. BigQuery doesn't perform well when used as a relational database. As a best practice, avoid running frequent, single-row append actions.
- To append data at a high rate, consider using [BigQuery Storage Write API](https://docs.cloud.google.com/bigquery/docs/write-api). It is a recommended solution for high-performance data ingestion. The BigQuery Storage Write API has robust features, including exactly-once delivery semantics. To learn about limits and quotas, see [Storage Write API](https://cloud.google.com/bigquery/quotas#write-api-limits) and to see costs of using this API, see [BigQuery data ingestion pricing](https://cloud.google.com/bigquery/pricing#data_ingestion_pricing).
- To monitor the number of modified partitions on a table, use the [`INFORMATION_SCHEMA` view](https://cloud.google.com/bigquery/docs/information-schema-jobs#partitions-modified-by).
- For information about optimizing table load jobs to avoid reaching quota limits, see [Optimize load jobs](https://docs.cloud.google.com/bigquery/docs/optimize-load-jobs).

## Table pricing

When you create and use partitioned tables in BigQuery, your
charges are based on how much data is stored in the partitions and on the
queries you run against the data:

- For information on storage pricing, see [Storage pricing](https://cloud.google.com/bigquery/pricing#storage).
- For information on query pricing, see [Query pricing](https://cloud.google.com/bigquery/pricing#analysis_pricing_models).

Many partitioned table operations are free, including loading data into
partitions, copying partitions, and exporting data from partitions. Though free,
these operations are subject to BigQuery's
[Quotas and limits](https://docs.cloud.google.com/bigquery/quotas). For information on all free operations,
see [Free operations](https://cloud.google.com/bigquery/pricing#free) on the pricing page.

For best practices for controlling costs in BigQuery, see [Controlling costs in BigQuery](https://docs.cloud.google.com/bigquery/docs/best-practices-costs)

## Table security

Access control for partitioned tables is the same as access control for
standard tables. For more information, see
[Introduction to table access controls](https://docs.cloud.google.com/bigquery/docs/table-access-controls-intro).

## What's next

- To learn how to create partitioned tables, see [Creating partitioned tables](https://docs.cloud.google.com/bigquery/docs/creating-partitioned-tables).
- To learn how to manage and update partitioned tables, see [Managing partitioned tables](https://docs.cloud.google.com/bigquery/docs/managing-partitioned-tables).
- For information on querying partitioned tables, see [Querying partitioned tables](https://docs.cloud.google.com/bigquery/docs/querying-partitioned-tables).