# Updating partitioned table data using DML

This page provides an overview of data manipulation language (DML) support for
partitioned tables.

For more information on DML, see:

- [Introduction to DML](https://docs.cloud.google.com/bigquery/docs/data-manipulation-language)
- [DML syntax](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/dml-syntax)
- [Updating table data using data manipulation language](https://docs.cloud.google.com/bigquery/docs/updating-data)

## Tables used in examples

The following JSON schema definitions represent the tables used in the examples
on this page.

`mytable`: an [ingestion-time partitioned table](https://docs.cloud.google.com/bigquery/docs/partitioned-tables#ingestion_time)

```
    [
      {"name": "field1", "type": "INTEGER"},
      {"name": "field2", "type": "STRING"}
    ]
```

<br />

`mytable2`: a standard (non-partitioned) table

```
    [
      {"name": "id", "type": "INTEGER"},
      {"name": "ts", "type": "TIMESTAMP"}
    ]
```

<br />

`mycolumntable`: a [partitioned table](https://docs.cloud.google.com/bigquery/docs/partitioned-tables#date_timestamp_partitioned_tables)
that is partitioned by using the `ts` `TIMESTAMP` column

```
    [
      {"name": "field1", "type": "INTEGER"},
      {"name": "field2", "type": "STRING"}
      {"name": "field3", "type": "BOOLEAN"}
      {"name": "ts", "type": "TIMESTAMP"}
    ]
```

<br />

In examples where <var translate="no">COLUMN_ID</var> appears, replace it with the name of the
column you wish to operate on.

## Inserting data

You use a DML [`INSERT` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/dml-syntax#insert_statement)
to add rows to a partitioned table.

### Inserting data into ingestion-time partitioned tables

When you use a DML statement to add rows to an ingestion-time partitioned table,
you can specify the partition to which the rows should be added. You reference
the partition using the [`_PARTITIONTIME` pseudocolumn](https://docs.cloud.google.com/bigquery/docs/querying-partitioned-tables#query_an_ingestion-time_partitioned_table).

For example, the following `INSERT` statement adds a row to the May 1, 2017
partition of `mytable` --- `"2017-05-01"`.

```googlesql
INSERT INTO
  project_id.dataset.mytable (_PARTITIONTIME,
    field1,
    field2)
SELECT
  TIMESTAMP("2017-05-01"),
  1,
  "one"
```

Only timestamps that correspond to exact date boundaries can be used. For
example, the following DML statement returns an error:

```googlesql
INSERT INTO
  project_id.dataset.mytable (_PARTITIONTIME,
    field1,
    field2)
SELECT
  TIMESTAMP("2017-05-01 21:30:00"),
  1,
  "one"
```

> [!NOTE]
> **Note:** The `_PARTITIONTIME` pseudocolumn can also be modified using an [`UPDATE` statement](https://docs.cloud.google.com/bigquery/docs/using-dml-with-partitioned-tables#updating_data).

### Inserting data into partitioned tables

Inserting data into a partitioned table using DML is the same as inserting data
into a non-partitioned table.

For example, the following `INSERT` statement adds rows to partitioned table
`mycolumntable` by selecting data from `mytable2` (a non-partitioned table).

```googlesql
INSERT INTO
  project_id.dataset.mycolumntable (ts,
    field1)
SELECT
  ts,
  id
FROM
  project_id.dataset.mytable2
```

## Deleting data

You use a DML [`DELETE` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/dml-syntax#delete_statement)
to delete rows from a partitioned table.

### Deleting data in ingestion-time partitioned tables

The following `DELETE` statement deletes all rows from the June 1, 2017
partition (`"2017-06-01"`) of `mytable` where `field1` is equal to `21`. You
reference the partition using the [`_PARTITIONTIME` pseudocolumn](https://docs.cloud.google.com/bigquery/docs/querying-partitioned-tables#query_an_ingestion-time_partitioned_table).

```googlesql
DELETE
  project_id.dataset.mytable
WHERE
  field1 = 21
  AND _PARTITIONTIME = "2017-06-01"
```

### Deleting data in partitioned tables

Deleting data in a partitioned table using DML is the same as deleting data
from a non-partitioned table.

For example, the following `DELETE` statement deletes all rows from the
June 1, 2017 partition (`"2017-06-01"`) of `mycolumntable` where `field1` is
equal to `21`.

```googlesql
DELETE
  project_id.dataset.mycolumntable
WHERE
  field1 = 21
  AND DATE(ts) = "2017-06-01"
```

## Using DML DELETE to delete partitions

If a qualifying `DELETE` statement covers all rows in a partition,
BigQuery removes the entire partition. This removal is done
without scanning bytes or consuming slots. The following example of a `DELETE`
statement covers the entire partition of a filter on the `_PARTITIONDATE`
pseudocolumn:

```googlesql
DELETE mydataset.mytable
WHERE _PARTITIONDATE IN ('2076-10-07', '2076-03-06');
```

### Common disqualifications

Queries with the following characteristics may not benefit from the optimization:

- Partial partition coverage
- References to non-partitioning columns
- [Recently ingested data](https://docs.cloud.google.com/bigquery/docs/write-api#stream_into_partitioned_tables) through the BigQuery [Storage Write API](https://docs.cloud.google.com/bigquery/docs/write-api) or the [legacy streaming API](https://docs.cloud.google.com/bigquery/streaming-data-into-bigquery)
- Filters with subqueries or unsupported predicates

Eligibility for optimization can vary with the type of partitioning, the
underlying storage metadata, and the filter predicates. As a best practice,
perform a dry run to verify that the query results in 0 bytes processed.

### Multi-statement transaction

This optimization works within a [multi-statement transaction](https://docs.cloud.google.com/bigquery/docs/transactions). The following query example replaces a partition with data from another table in a single transaction, without scanning the partition for
the `DELETE` statement.

```googlesql
DECLARE REPLACE_DAY DATE;
BEGIN TRANSACTION;

-- find the partition which we want to replace
SET REPLACE_DAY = (SELECT MAX(d) FROM mydataset.mytable_staging);

-- delete the entire partition from mytable
DELETE FROM mydataset.mytable WHERE part_col = REPLACE_DAY;

-- insert the new data into the same partition in mytable
INSERT INTO mydataset.mytable
SELECT * FROM mydataset.mytable_staging WHERE part_col = REPLACE_DAY;

COMMIT TRANSACTION;
```

## Updating data

You use an [`UPDATE` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/dml-syntax#update_statement)
to update rows in a partitioned table.

### Updating data in ingestion-time partitioned tables

The following `UPDATE` statement moves rows from one partition to another.
Rows in the May 1, 2017 partition (`"2017-05-01"`) of `mytable` where `field1`
is equal to `21` are moved to the June 1, 2017 partition (`"2017-06-01"`).

```googlesql
UPDATE
  project_id.dataset.mytable
SET
  _PARTITIONTIME = "2017-06-01"
WHERE
  _PARTITIONTIME = "2017-05-01"
  AND field1 = 21
```

### Updating data in partitioned tables

Updating data in a partitioned table using DML is the same as updating data
from a non-partitioned table. For example, the following `UPDATE`
statement moves rows from one partition to another. Rows in the May 1, 2017
partition (`"2017-05-01"`) of `mytable` where `field1` is equal to `21` are
moved to the June 1, 2017 partition (`"2017-06-01"`).

```googlesql
UPDATE
  project_id.dataset.mycolumntable
SET
  ts = "2017-06-01"
WHERE
  DATE(ts) = "2017-05-01"
  AND field1 = 21
```

## DML in hourly, monthly, and yearly partitioned tables

You can use DML statements to modify an hourly, monthly, or yearly partitioned table.
Provide the hour, month, or year range of the relevant dates/timestamps/datetimes, as
in the following example for monthly partitioned tables:

```bash
    bq query --nouse_legacy_sql 'DELETE FROM my_dataset.my_table WHERE
    TIMESTAMP_TRUNC(ts_column, MONTH) = "2020-01-01 00:00:00";'
```

Or another example for partitioned tables with `DATETIME` column:

```bash
    bq query --nouse_legacy_sql 'DELETE FROM my_dataset.my_table WHERE
    dt_column BETWEEN DATETIME("2020-01-01")
    AND DATETIME("2020-05-01");'
```

## Using a `MERGE` statement

You use a DML [`MERGE` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/dml-syntax#merge_statement)
to combine `INSERT`, `UPDATE`, and `DELETE` operations for a partitioned table
into one statement and perform them atomically.

### Pruning partitions when using a `MERGE` statement

When you run a `MERGE` statement against a partitioned table, you can limit
which partitions are scanned by including the partitioning column in either
a subquery filter, a `search_condition` filter, or a `merge_condition` filter.
Pruning can occur when scanning the source table or the target table, or both.

Each of the examples below queries an ingestion-time partitioned table using
the `_PARTITIONTIME` pseudocolumn as a filter.

#### Using a subquery to filter source data

In the following `MERGE` statement, the subquery in the `USING` clause filters
on the `_PARTITIONTIME` pseudocolumn in the source table.

```googlesql
MERGE dataset.target T
USING (SELECT * FROM dataset.source WHERE _PARTITIONTIME = '2018-01-01') S
ON T.COLUMN_ID = S.COLUMN_ID
WHEN MATCHED THEN
  DELETE
```

Looking at the query execution plan, the subquery runs first. Only the rows in
the `'2018-01-01'` partition in the source table are scanned. Here is the
relevant stage in the query plan:

    READ $10:name, $11:_PARTITIONTIME
    FROM temp.source
    WHERE equal($11, 1514764800.000000000)

#### Using a filter in the `search_condition` of a `when_clause`

If a `search_condition` contains a filter, then the query optimizer attempts to
prune partitions. For example, in the following `MERGE` statement, each `WHEN
MATCHED` and `WHEN NOT MATCHED` clause contains a filter on the `_PARTITIONTIME`
pseudocolumn.

```googlesql
MERGE dataset.target T
USING dataset.source S
ON T.COLUMN_ID = S.COLUMN_ID
WHEN MATCHED AND T._PARTITIONTIME = '2018-01-01' THEN
  UPDATE SET COLUMN_ID = S.COLUMN_ID
WHEN MATCHED AND T._PARTITIONTIME = '2018-01-02' THEN
  UPDATE SET COLUMN_ID = S.COLUMN_ID + 10
WHEN NOT MATCHED BY SOURCE AND T._PARTITIONTIME = '2018-01-03' THEN
  DELETE
```

During the join stage, only the following partitions are scanned in the target
table: `'2018-01-01'`, `'2018-01-02'`, and `'2018-01-03'` --- that is, the
union of all the `search_condition` filters.

From the query execution plan:

    READ
    $1:COLUMN_ID, $2:_PARTITIONTIME, $3:$file_temp_id, $4:$row_temp_id
    FROM temp.target
    WHERE or(equal($2, 1514764800.000000000), equal($2, 1514851200.000000000), equal($2, 1514937600.000000000))

However, in the following example, the `WHEN NOT MATCHED BY SOURCE` clause does
not have a filter expression:

```googlesql
MERGE dataset.target T
USING dataset.source S
ON T.COLUMN_ID = S.COLUMN_ID
WHEN MATCHED AND T._PARTITIONTIME = '2018-01-01' THEN
  UPDATE SET COLUMN_ID = S.COLUMN_ID
WHEN NOT MATCHED BY SOURCE THEN
  UPDATE SET COLUMN_ID = COLUMN_ID + 1
```

This query must scan the entire target table to compute the `WHEN NOT MATCHED BY
SOURCE` clause. As a result, no partitions are pruned.

#### Using a constant false predicate in a `merge_condition`

If you use the `WHEN NOT MATCHED` and `WHEN NOT MATCHED BY SOURCE` clauses
together, then BigQuery usually performs a full outer join, which
cannot be pruned. However, if the merge condition uses a constant false
predicate, then BigQuery can use the filter condition for
partition pruning. For more information about the use of constant false
predicates, see the description of the `merge_condition` clause in the
[`MERGE` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/dml-syntax#merge_statement)
documentation.

The following example scans only the `'2018-01-01'` partition in both the target
and source tables.

```googlesql
MERGE dataset.target T
USING dataset.source S
ON FALSE
WHEN NOT MATCHED AND _PARTITIONTIME = '2018-01-01' THEN
  INSERT(COLUMN_ID) VALUES(COLUMN_ID)
WHEN NOT MATCHED BY SOURCE AND _PARTITIONTIME = '2018-01-01' THEN
  DELETE
```

#### Using a filter in a `merge_condition`

The query optimizer attempts to use a filter in a `merge_condition` to prune
partitions. The query optimizer might or might not be able to push the predicate
down to the table scanning stage, depending on the type of join.

In the following example, the `merge_condition` is used as a predicate to join
the source and target tables. The query optimizer can push this predicate down
when it scans both tables. As a result, the query only scans the `'2018-01-01'`
partition in both the target and source tables.

```googlesql
MERGE dataset.target T
USING dataset.source S
ON T.COLUMN_ID = S.COLUMN_ID AND
  T._PARTITIONTIME = '2018-01-01' AND
  S._PARTITIONTIME = '2018-01-01'
WHEN MATCHED THEN
  UPDATE SET COLUMN_ID = NEW_VALUE
```

In the next example, the `merge_condition` does not contain a predicate for the
source table, so no partition pruning can be performed on the source table. The
statement does contain a predicate for the target table, but the statement uses
a `WHEN NOT MATCHED BY SOURCE` clause, rather than a `WHEN MATCHED` clause. That
means the query has to scan the entire target table for the rows that don't
match.

```googlesql
MERGE dataset.target T
USING dataset.source S
ON T.COLUMN_ID = S.COLUMN_ID AND T._PARTITIONTIME = '2018-01-01'
WHEN NOT MATCHED BY SOURCE THEN
  UPDATE SET COLUMN_ID = NEW_VALUE
```

## Limitations

For information about DML limitations, see
[Limitations](https://docs.cloud.google.com/bigquery/docs/data-manipulation-language#dml-limitations)
on the
[DML reference](https://docs.cloud.google.com/bigquery/docs/data-manipulation-language)
page.

## Quotas

For information about DML quota information, see
[DML statements](https://docs.cloud.google.com/bigquery/quotas#data-manipulation-language-statements) on the
[Quotas and limits](https://docs.cloud.google.com/bigquery/quota-policy) page.

## Pricing

For information about DML pricing, see how to compute the query size for DML
statements run on
[Partitioned tables](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/dml-syntax#partitioned_tables).

## Table security

To control access to tables in BigQuery, see
[Control access to resources with IAM](https://docs.cloud.google.com/bigquery/docs/control-access-to-resources-iam).

## What's next

- Learn how to [create partitioned tables](https://docs.cloud.google.com/bigquery/docs/creating-partitioned-tables)
- Learn how to [query partitioned tables](https://docs.cloud.google.com/bigquery/docs/querying-partitioned-tables)
- Get an [introduction to DML](https://docs.cloud.google.com/bigquery/docs/data-manipulation-language)
- Learn how to compose DML statements using [DML syntax](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/dml-syntax)