# Transform data with data manipulation language (DML)

The BigQuery data manipulation language (DML) lets you
update, insert, and delete data from your BigQuery tables.

You can execute DML statements just as you would a `SELECT` statement, with the
following conditions:

- You must use GoogleSQL. To enable GoogleSQL, see [Switching SQL dialects](https://docs.cloud.google.com/bigquery/sql-reference/enabling-standard-sql).
- You cannot specify a destination table for the query.

For more information about how to compute the number of bytes processed by a
DML statement, see
[On-demand query size calculation](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/dml-syntax#on-demand-query-size-calculation).

## Limitations

- Each DML statement initiates an implicit transaction, which means that changes
  made by the statement are automatically committed at the end of each
  successful DML statement.

- Rows that were recently written using the
  `tabledata.insertall` streaming method can't be modified with data manipulation
  language (DML), such as `UPDATE`,
  `DELETE`, `MERGE`, or `TRUNCATE` statements. The recent writes are those that occurred
  within the last 30 minutes. All other rows in the table remain modifiable by using
  `UPDATE`, `DELETE`, `MERGE`, or `TRUNCATE` statements. The streamed data can take up to 90
  minutes to become available for copy operations.

  Alternatively, rows that were recently written using the Storage Write API
  can be modified using `UPDATE`, `DELETE`, or `MERGE` statements. For more
  information, see [Use data manipulation language (DML) with recently streamed data](https://docs.cloud.google.com/bigquery/docs/write-api#use_data_manipulation_language_dml_with_recently_streamed_data).
- Correlated subqueries within a `when_clause`, `search_condition`,
  `merge_update_clause` or `merge_insert_clause` are not supported for `MERGE`
  statements.

- Queries that contain [DML statements](https://docs.cloud.google.com/bigquery/docs/data-manipulation-language)
  cannot use a [wildcard table](https://docs.cloud.google.com/bigquery/docs/querying-wildcard-tables)
  as the target of the query. For example, a wildcard table can be used in the
  `FROM` clause of an `UPDATE` query, but a wildcard table cannot be used as the
  target of the `UPDATE` operation.

## DML statements

The following sections describe the different types of DML statements and how
you can use them.

### `INSERT` statement

Use the `INSERT` statement to add new rows to an existing table. The following
example inserts new rows into the table `dataset.Inventory` with explicitly
specified values.

    INSERT dataset.Inventory (product, quantity)
    VALUES('whole milk', 10),
          ('almond milk', 20),
          ('coffee beans', 30),
          ('sugar', 0),
          ('matcha', 20),
          ('oat milk', 30),
          ('chai', 5)

    /+---+---+
     |      product      | quantity |
     +---+---+
     | almond milk       |       20 |
     | chai              |        5 |
     | coffee beans      |       30 |
     | matcha            |       20 |
     | oat milk          |       30 |
     | sugar             |        0 |
     | whole milk        |       10 |
     +---+---+/

For more information about INSERT statements, see [`INSERT` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/dml-syntax#insert_statement).

### `DELETE` statement

Use the `DELETE` statement to delete rows in a table. The following example
deletes all rows in the table `dataset.Inventory` that have the `quantity` value
`0`.

    DELETE dataset.Inventory
    WHERE quantity = 0

    /+---+---+
     |      product      | quantity |
     +---+---+
     | almond milk       |       20 |
     | chai              |        5 |
     | coffee beans      |       30 |
     | matcha            |       20 |
     | oat milk          |       30 |
     | whole milk        |       10 |
     +---+---+/

To delete all rows in a table, use the `TRUNCATE TABLE` statement instead. For
more information about `DELETE` statements, see [`DELETE` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/dml-syntax#delete_statement).

### `TRUNCATE` statement

Use the `TRUNCATE` statement to remove all rows from a table, but leave the table
metadata intact, including table schema, description, and labels. The following
example removes all rows from the table `dataset.Inventory`.

    TRUNCATE dataset.Inventory

To delete specific rows in a table, use the `DELETE` statement instead. For more
information about the `TRUNCATE` statement, see [`TRUNCATE` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/dml-syntax#truncate_table_statement).

### `UPDATE` statement

Use the `UPDATE` statement to update existing rows in a table. The `UPDATE`
statement must also include the `WHERE` keyword to specify a condition. The
following example reduces the `quantity` value of rows by 10 for products that
contain the string `milk`.

    UPDATE dataset.Inventory
    SET quantity = quantity - 10,
    WHERE product LIKE '%milk%'

    /+---+---+
     |      product      | quantity |
     +---+---+
     | almond milk       |       10 |
     | chai              |        5 |
     | coffee beans      |       30 |
     | matcha            |       20 |
     | oat milk          |       20 |
     | whole milk        |        0 |
     +---+---+/

`UPDATE` statements can also include `FROM` clauses to include joined tables.
For more information about `UPDATE` statements, see [`UPDATE` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/dml-syntax#update_statement).

### `MERGE` statement

The `MERGE` statement combines the `INSERT`, `UPDATE`, and `DELETE` operations
into a single statement and performs the operations atomically to merge data
from one table to another. For more information and examples about the `MERGE`
statement, see [`MERGE` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/dml-syntax#merge_statement).

## Concurrent jobs

BigQuery manages the concurrency of DML statements that add,
modify, or delete rows in a table.

> [!NOTE]
> **Note:** DML statements are subject to rate limits such as the [maximum rate of table writes](https://docs.cloud.google.com/bigquery/quotas#standard_tables). You might hit a rate limit if you submit a high number of jobs against a table at one time. These rates do not limit the total number of DML statements that can be run. If you get an error message that says you've [exceeded a rate limit](https://docs.cloud.google.com/bigquery/docs/troubleshoot-quotas#overview), retry the operation using exponential backoff between retries.

### INSERT DML concurrency

During any 24-hour period, the first 1500 `INSERT` statements run immediately
after they are submitted. After this limit is reached, the concurrency
of `INSERT` statements that write to a table is limited to 10. Additional
`INSERT` statements are added to a `PENDING` queue. Up to 100 `INSERT`
statements can be queued against a table at any given time. When an `INSERT`
statement completes, the next `INSERT` statement is removed from the queue and run.

If you must run DML `INSERT` statements more frequently,
consider streaming data to your table using the
[Storage Write API](https://docs.cloud.google.com/bigquery/docs/write-api).

### UPDATE, DELETE, MERGE DML concurrency

The `UPDATE`, `DELETE`, and `MERGE` DML statements are called *mutating DML statements*. If you submit one or more mutating DML statements on a table while
other mutating DML jobs on it are still running (or pending),
BigQuery runs up to 2 of them concurrently, after which up to 20
are queued as `PENDING`. When a previously running job finishes, the next
pending job is dequeued and run. Queued mutating DML statements
share a per-table queue with maximum length 20. Additional statements past
the maximum queue length for each table fail with the error message: `Resources
exceeded during query execution: Too many DML statements outstanding against
table PROJECT_ID:DATASET.TABLE, limit is 20.`

Interactive priority DML jobs that are queued for more than 7 hours fail with
the following error message:

`DML statement has been queued for too long`

### DML statement conflicts

Mutating DML statements that run concurrently on a table cause DML statement conflicts when the statements try to mutate the same partition. The statements succeed as long as they don't modify the same partition. BigQuery tries to rerun failed statements up to three times.

- An `INSERT` DML statement that inserts rows to a table doesn't conflict
  with any other concurrently running DML statement.

- A `MERGE` DML statement does not conflict with other concurrently running DML
  statements as long as the statement only inserts rows and does not delete or
  update any existing rows. This can include `MERGE` statements with `UPDATE`
  or `DELETE` clauses, as long as those clauses aren't invoked when the
  query runs.

## Fine-grained DML

> [!WARNING]
>
> **Preview**
>
>
> This feature is
>
> subject to the "Pre-GA Offerings Terms" in the General Service Terms section of the
> [Service Specific
> Terms](https://docs.cloud.google.com/terms/service-terms#1).
>
> You can process personal data for
>
> this feature
>
> as outlined in the
> [Cloud Data
> Processing Addendum](https://docs.cloud.google.com/terms/data-processing-addendum), subject to the obligations and restrictions described in the
> agreement under which you access Google Cloud.
>
> Pre-GA features are available "as is" and might have limited support.
>
> For more information, see the
> [launch stage descriptions](https://cloud.google.com/products/#product-launch-stages).

> [!NOTE]
> **Note:** To provide feedback or request support for this feature, send an email to [bq-fine-grained-dml-feedback@google.com](mailto:bq-fine-grained-dml-feedback@google.com).

Fine-grained DML is a performance enhancement designed
to optimize the execution of `UPDATE`, `DELETE`, and `MERGE` statements (also
known as *mutating* DML statements).

### Performance considerations

Without fine-grained DML enabled, DML mutations are
performed at the file-group level, which can lead to inefficient data rewrites,
especially for sparse mutations. This can lead to additional slot consumption
and longer execution times.

Fine-grained DML is a performance enhancement designed to optimize these
mutating DML statements by introducing a more granular approach that aims to
reduce the amount of data that needs to be rewritten at the file-group level.
This approach can significantly reduce the processing, I/O, and slot time
consumed for mutating DML jobs.

There are some performance considerations to be aware of when using
fine-grained DML:

- Fine-grained DML operations process deleted data in a hybrid approach that distributes rewrite costs across numerous table mutations. Each DML operation might process a portion of the deleted data, and then offload the remaining deleted data processing to a background garbage collection process. For more information, see [deleted data considerations](https://docs.cloud.google.com/bigquery/docs/data-manipulation-language#deleted_data_considerations).
- Tables with frequent mutating DML operations might experience increased latency for subsequent `SELECT` queries and DML jobs. To evaluate the impact of enabling this feature, benchmark the performance of a realistic sequence of DML operations and subsequent reads.
- Enabling fine-grained DML won't reduce the amount of scanned bytes of the mutating DML statement itself.

### Enable fine-grained DML

To enable fine-grained DML, set the
[`enable_fine_grained_mutations` table option](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#table_option_list)
to `TRUE` when you run a `CREATE TABLE` or `ALTER TABLE` DDL statement.

To create a new table with fine-grained DML, use the
[`CREATE TABLE` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_table_statement):

```googlesql
CREATE TABLE mydataset.mytable (
  product STRING,
  inventory INT64)
OPTIONS(enable_fine_grained_mutations = TRUE);
```

To alter an existing table with fine-grained DML, use the
[`ALTER TABLE` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#alter_table_set_options_statement):

```googlesql
ALTER TABLE mydataset.mytable
SET OPTIONS(enable_fine_grained_mutations = TRUE);
```

To alter all existing tables in a dataset with fine-grained DML, use the
[`ALTER TABLE` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#alter_table_set_options_statement):

    FOR record IN
     (SELECT CONCAT(table_schema, '.', table_name) AS table_path
     FROM mydataset.INFORMATION_SCHEMA.TABLES)
    DO
     EXECUTE IMMEDIATE
       "ALTER TABLE `"` || record.table_path || " SET OPTIONS(enable_fine_grained_mutations = TRUE)";
    END FOR;

<br />

After the `enable_fine_grained_mutations` option is set to `TRUE`, mutating
DML statements are run with fine-grained DML capabilities enabled and
use existing
[DML statement syntax](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/dml-syntax).

To determine if a table has been enabled with fine-grained DML, query the
[`INFORMATION_SCHEMA.TABLES` view](https://docs.cloud.google.com/bigquery/docs/information-schema-tables).
The following example checks which tables within a dataset have been enabled
with this feature:

```googlesql
SELECT
  table_schema AS datasetId,
  table_name AS tableId,
  is_fine_grained_mutations_enabled
FROM
  DATASET_NAME.INFORMATION_SCHEMA.TABLES;
```

Replace `DATASET_NAME` with the name of the dataset in
which to check if any tables have fine-grained DML enabled.

### Disable fine-grained DML

To disable fine-grained DML from an existing table, use the
[`ALTER TABLE` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#alter_table_set_options_statement).

```googlesql
ALTER TABLE mydataset.mytable
SET OPTIONS(enable_fine_grained_mutations = FALSE);
```

When disabling fine-grained DML, it may take some time for all deleted data
to be fully processed, see
[Deleted data considerations](https://docs.cloud.google.com/bigquery/docs/data-manipulation-language#deleted_data_considerations). As a result,
[fine-grained DML limitations](https://docs.cloud.google.com/bigquery/docs/data-manipulation-language#fine-grained-dml-limitations) may persist until
this has occurred.

### Pricing

Enabling fine-grained DML for a table can incur additional costs.
These costs include the following:

- [BigQuery storage costs](https://docs.cloud.google.com/bigquery/pricing#storage) to store the extra mutation metadata that is associated with fine-grained DML operations. The actual storage cost depends on the amount of data that is modified, but for most situations it's expected to be negligible in comparison to the size of the table itself.
- [BigQuery compute costs](https://docs.cloud.google.com/bigquery/pricing#analysis_pricing_models) to process deleted data using offloaded [garbage collection jobs](https://docs.cloud.google.com/bigquery/docs/data-manipulation-language#deleted_data_considerations), and subsequent `SELECT` queries processing additional deletion metadata which has yet to be garbage collected.

You can use [BigQuery reservations](https://docs.cloud.google.com/bigquery/docs/reservations-intro)
to allocate dedicated BigQuery compute resources to process
offloaded deleted data jobs. Reservations let you set a cap on the cost of
performing these operations. This approach is particularly useful, and often
recommended, for very large tables with frequent fine-grained mutating DML
operations, which otherwise would have high on-demand costs due to the large
number of bytes processed when performing each offloaded deleted data processing
job.

Fine-grained DML's offloaded deleted data processing jobs are considered
background jobs and require the use of the
[`BACKGROUND` reservation assignment type](https://docs.cloud.google.com/bigquery/docs/reservations-workload-management#assignments),
rather than the
[`QUERY` reservation assignment type](https://docs.cloud.google.com/bigquery/docs/reservations-workload-management#assignments).
Projects that perform fine-grained DML operations without a
[`BACKGROUND` assignment](https://docs.cloud.google.com/bigquery/docs/reservations-assignments) use
[on-demand pricing](https://docs.cloud.google.com/bigquery/pricing#on-demand-compute-pricing)
to process the offloaded deleted data jobs.

| Operation | On-demand pricing | Capacity-based pricing |
|---|---|---|
| Mutating DML statements | Use standard [DML sizing](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/dml-syntax#on-demand-query-size-calculation) to determine on-demand bytes scanned calculations. Enabling fine-grained DML won't reduce the amount of scanned bytes of the DML statement itself. | Consume slots assigned with a `QUERY` type at statement run time. |
| Offloaded deleted data processing jobs | Use standard [DML sizing](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/dml-syntax#on-demand-query-size-calculation) to determine on-demand bytes scanned calculations when deleted data processing jobs are run. | Consume slots assigned with a `BACKGROUND` type when deleted data processing jobs are run. |

### Deleted data considerations

Fine-grained DML operations use a hybrid approach to manage deleted data,
combining inline processing with offloaded garbage collection to distribute
rewrite costs and optimize performance across multiple mutating DML statements
issued against a table.

During the execution of a mutating DML statement, BigQuery
attempts to perform a portion of relevant garbage collection from prior DML
statements inline. Any deleted data not handled inline is offloaded to a
background process for later cleanup.

Projects that perform fine-grained DML operations with a `BACKGROUND` assignment
process offloaded garbage collection tasks using slots. Processing deleted
data is subject to the configured reservation's resource availability. If there
aren't enough resources available within the configured reservation, processing
offloaded garbage collection operations might take longer than anticipated.

Projects that perform fine-grained DML operations by using
[on-demand pricing](https://docs.cloud.google.com/bigquery/pricing#on-demand-compute-pricing), or without a
`BACKGROUND` assignment, process offloaded garbage collection tasks using
internal BigQuery resources and are charged at
[on-demand pricing](https://docs.cloud.google.com/bigquery/pricing#on-demand-compute-pricing) rates. For more
information, see [Pricing](https://docs.cloud.google.com/bigquery/docs/data-manipulation-language#pricing).

The timing of offloaded garbage collection tasks is determined by the frequency
of DML activity on the table and the availability of resources, if using a
`BACKGROUND` assignment:

- For tables with continuous mutating DML operations, each DML processes a portion of the garbage collection workload to ensure consistent read and write performance, and as a result, the garbage collection is regularly processed as the subsequent DMLs are executed.
- If no subsequent DML activity occurs on a table, offloaded garbage collection is automatically triggered once the deleted data reaches 5 days of age.
- In rare cases, it might take longer to fully process deleted data.

To identify offloaded fine-grained DML deleted data processing jobs, query the
[`INFORMATION_SCHEMA.JOBS` view](https://docs.cloud.google.com/bigquery/docs/information-schema-jobs):

```googlesql
SELECT
  *
FROM
  region-us.INFORMATION_SCHEMA.JOBS
WHERE
  job_id LIKE "%fine_grained_mutation_garbage_collection%"
```

<br />

### Limitations

Tables enabled with fine-grained DML are subject to the following limitations:

- For large tables with frequently mutated partitions exceeding 2 TB, fine-grained DML is not recommended. These tables may experience added memory pressure for subsequent queries, which can lead to additional read latency or query errors.
- Only one mutating DML statement can run at a time on a table that has fine-grained DML enabled. Subsequent jobs are queued as `PENDING`. For more information about mutating DML concurrency behavior, see [UPDATE, DELETE, MERGE DML concurrency](https://docs.cloud.google.com/bigquery/docs/data-manipulation-language#update_delete_merge_dml_concurrency).
- A table enabled with fine-grained DML can't have partitions [individually deleted](https://docs.cloud.google.com/bigquery/docs/managing-partitioned-tables#delete_a_partition) or [overwritten](https://docs.cloud.google.com/bigquery/docs/writing-results#writing_query_results). To delete or replace data within a partition, you must use a mutating DML statement, such as `DELETE`, `UPDATE`, `MERGE`, or `TRUNCATE`.
- You can't use the [`tabledata.list` method](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tabledata/list) to read content from a table with fine-grained DML enabled. Instead, query the table with a `SELECT` statement to read table records.
- A table enabled with fine-grained DML cannot be previewed using the BigQuery console.
- You can't [copy a table](https://docs.cloud.google.com/bigquery/docs/managing-tables#copy-table) with fine-grained DML enabled after executing an `UPDATE`, `DELETE`, or `MERGE` statement.
- You can't create a [table snapshot](https://docs.cloud.google.com/bigquery/docs/table-snapshots-intro) or [table clone](https://docs.cloud.google.com/bigquery/docs/table-clones-intro) of a table with fine-grained DML enabled after executing an `UPDATE`, `DELETE`, or `MERGE` statement.
- You can't enable fine-grained DML on a table in a [replicated dataset](https://docs.cloud.google.com/bigquery/docs/data-replication), and you can't replicate a dataset that contains a table with fine-grained DML enabled.
- DML statements executed in a [multi-statement transaction](https://docs.cloud.google.com/bigquery/docs/transactions) aren't optimized with fine-grained DML.
- You can't enable fine-grained DML on temporary tables created with the [`CREATE TEMP TABLE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_table_statement) statement.
- Metadata reflected within the [`INFORMATION_SCHEMA.TABLE_STORAGE` views](https://docs.cloud.google.com/bigquery/docs/information-schema-table-storage) and [`INFORMATION_SCHEMA.PARTITIONS` views](https://docs.cloud.google.com/bigquery/docs/information-schema-partitions) can temporarily include recently deleted data using fine-grained DML until background garbage collection jobs have completed.

## Best practices

For best performance, Google recommends the following patterns:

- Avoid submitting large numbers of individual row updates or insertions.
  Instead, group DML operations together when possible. For more information,
  see
  [DML statements that update or insert single rows](https://docs.cloud.google.com/bigquery/docs/best-practices-performance-patterns#dml_statements_that_update_or_insert_single_rows).

- If updates or deletions generally happen on older data, or within a particular
  range of dates, consider [partitioning](https://docs.cloud.google.com/bigquery/docs/partitioned-tables)
  your tables. Partitioning ensures that the changes are limited to specific
  partitions within the table.

- Avoid partitioning tables if the amount of data in each partition is small and
  each update modifies a large fraction of the partitions.

- If you often update rows where one or more columns fall within a narrow range
  of values, consider using
  [clustered tables](https://docs.cloud.google.com/bigquery/docs/clustered-tables). Clustering ensures that
  changes are limited to specific sets of blocks, reducing the amount of data
  that needs to be read and written. The following is an example of an `UPDATE`
  statement that filters on a range of column values:

  ```googlesql
  UPDATE mydataset.mytable
  SET string_col = 'some string'
  WHERE id BETWEEN 54 AND 75;
  ```

  Here is a similar example that filters on a small list of column values:

  ```googlesql
  UPDATE mydataset.mytable
  SET string_col = 'some string'
  WHERE id IN (54, 57, 60);
  ```

  Consider clustering on the `id` column in these cases.
- If you need OLTP functionality, consider using
  [Cloud SQL federated queries](https://docs.cloud.google.com/bigquery/docs/cloud-sql-federated-queries),
  which enable BigQuery to query data that resides in
  Cloud SQL.

- To resolve and prevent the quota error
  `Too many DML statements outstanding against table,` follow
  [the guidance for this error](https://docs.cloud.google.com/bigquery/docs/troubleshoot-quotas#ts-too-many-dml-statements-against-table-quota) on the
  BigQuery Troubleshooting page.

For best practices to optimize query performance, see
[Introduction to optimizing query performance](https://docs.cloud.google.com/bigquery/docs/best-practices-performance-overview).

## What's next

- For DML syntax information and samples, see [DML syntax](https://docs.cloud.google.com/bigquery/sql-reference/dml-syntax).
- Learn more about [Updating partitioned table data using DML](https://docs.cloud.google.com/bigquery/docs/using-dml-with-partitioned-tables).
- For information about using DML statements in scheduled queries, see [Scheduling queries](https://docs.cloud.google.com/bigquery/docs/scheduling-queries).