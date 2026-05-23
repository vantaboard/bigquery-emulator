# Table sampling

> [!WARNING]
>
> **Preview**
>
>
> This product or feature is
>
> subject to the "Pre-GA Offerings Terms" in the General Service Terms section
> of the [Service Specific
> Terms](https://docs.cloud.google.com/terms/service-terms#1).
>
> Pre-GA products and features are available "as is" and might have limited support.
>
> For more information, see the
> [launch stage descriptions](https://cloud.google.com/products/#product-launch-stages).

Table sampling lets you query random subsets of data from large
BigQuery tables. Sampling returns a variety of records while avoiding
the costs associated with scanning and processing an entire table.

## Using table sampling

To use table sampling in a query, include the
[`TABLESAMPLE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#tablesample_operator)
clause. For example, the following query selects approximately 10% of a table's
data:

    SELECT * FROM dataset.my_table TABLESAMPLE SYSTEM (10 PERCENT)

Unlike the `LIMIT` clause, `TABLESAMPLE` returns a random subset of data from a
table. Also, BigQuery does not cache the results of queries that
include a `TABLESAMPLE` clause, so the query might return different results each
time.

You can combine the `TABLESAMPLE` clause with other selection conditions. The
following example samples about 50% of the table and then applies a `WHERE`
clause:

    SELECT *
    FROM dataset.my_table TABLESAMPLE SYSTEM (50 PERCENT)
    WHERE customer_id = 1

The next example combines a `TABLESAMPLE` clause with a `JOIN` clause:

    SELECT *
    FROM dataset.table1 T1 TABLESAMPLE SYSTEM (10 PERCENT)
    JOIN dataset.table2 T2 TABLESAMPLE SYSTEM (20 PERCENT) USING (customer_id)

For smaller tables, if you join two samples and none of the sampled rows meet
the join condition, then you might receive an empty result.

You can specify the percentage as a
[query parameter](https://docs.cloud.google.com/bigquery/docs/parameterized-queries). The next example shows
how to pass the percentage to a query by using the bq command-line tool:

    bq query --use_legacy_sql=false --parameter=percent:INT64:29 \
        'SELECT * FROM `dataset.my_table` TABLESAMPLE SYSTEM (@percent PERCENT)`

BigQuery tables are organized into data blocks. The `TABLESAMPLE`
clause works by randomly selecting a percentage of data blocks from the table
and reading all of the rows in the selected blocks. The sampling granularity
is limited by the number of data blocks.

Typically, BigQuery splits tables or table partitions into blocks
if they are larger than about 1 GB. Smaller tables might consist of a single
data block. In that case, the `TABLESAMPLE` clause reads the entire table. If
the sampling percentage is greater than zero and the table is not empty, then
table sampling always returns some results.

Blocks can be different sizes, so the exact fraction of rows that are sampled
might vary. If you want to sample individual rows, rather than data blocks, then
you can use a `WHERE rand() < K` clause instead. However, this approach requires
BigQuery to scan the entire table. To save costs but still
benefit from row-level sampling, you can combine both techniques.

The following example reads approximately 20% of the data blocks from storage
and then randomly selects 10% of the rows in those blocks:

    SELECT * FROM dataset.my_table TABLESAMPLE SYSTEM (20 PERCENT)
    WHERE rand() < 0.1

## External tables

You can use the `TABLESAMPLE` clause with external tables that store data in a
collection of files. BigQuery samples a subset of the external
files that the table references. For some file formats, BigQuery
can split individual files into blocks for sampling. Some external data, such as
data in Google Sheets, consists of a single file that is sampled as one block
of data.

## Sampling from the write-optimized storage

If you use table sampling with
[streaming inserts](https://docs.cloud.google.com/bigquery/docs/streaming-data-into-bigquery), then
BigQuery samples data from the write-optimized storage. In some cases,
all the data in the write-optimized storage is represented as a single block. When that happens,
either all the data in the write-optimized storage appears in the results, or none of it does.

## Partitioned and clustered tables

Partitioning and clustering produce blocks where all rows within a specific
block have either the same partitioning key or have clustering attributes with
close values. Therefore, sample sets from these tables tend to be more biased
than sample sets from non-partitioned, non-clustered tables.

## Limitations

- A sampled table can only appear once in a query statement. This restriction includes tables that are referenced inside view definitions.
- Sampling data from views is not supported.
- Sampling the results of subqueries or table-valued function calls is not supported.
- Sampling from an array scan, such as the result of calling the `UNNEST` operator, is not supported.
- Sampling inside an `IN` subquery is not supported.
- Sampling from tables with row-level security applied is not supported.

## Table sampling pricing

If you use [on-demand billing](https://cloud.google.com/bigquery/pricing#on_demand_pricing), then you
are charged for reading the data that is sampled. BigQuery does
not cache the results of a query that includes a `TABLESAMPLE` clause, so each
execution incurs the cost of reading the data from storage.