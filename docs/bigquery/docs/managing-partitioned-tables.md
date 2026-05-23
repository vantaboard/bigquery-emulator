# Managing partitioned tables

This document describes how to manage partitioned tables in BigQuery.

> [!NOTE]
> **Note:** The information in [Managing tables](https://docs.cloud.google.com/bigquery/docs/managing-tables) also applies to partitioned tables.

## Get partition metadata

You can get information about partitioned tables in the following ways:

- Use the [`INFORMATION_SCHEMA.PARTITIONS`](https://docs.cloud.google.com/bigquery/docs/information-schema-partitions) view ([Preview](https://cloud.google.com/products/#product-launch-stages)).
- Use the `__PARTITIONS_SUMMARY__` meta-table (legacy SQL only).

### Getting partition metadata using `INFORMATION_SCHEMA` views

When you query the `INFORMATION_SCHEMA.PARTITIONS` view, the query results
contain one row for each partition. For example, the following query lists all
of the partitions for a specific table within a dataset:

```googlesql
#standardSQL
SELECT
  partition_id
FROM
  `DATASET_ID.INFORMATION_SCHEMA.PARTITIONS`
WHERE
  table_name = 'TABLE_NAME'
  AND partition_id IS NOT NULL --filter out non-partitioned tables
```

For more information, see
[`INFORMATION_SCHEMA.PARTITIONS`](https://docs.cloud.google.com/bigquery/docs/information-schema-partitions).

### Getting partition metadata using meta-tables

In legacy SQL, you can get metadata about table partitions by querying the
`__PARTITIONS_SUMMARY__` meta-table. *Meta-tables* are read-only tables that
contain metadata.

Query the `__PARTITIONS_SUMMARY__` meta-table as follows:

```googlesql
#legacySQL
SELECT
  partition_id
FROM
  [DATASET_ID.TABLE_NAME$__PARTITIONS_SUMMARY__]
```

> [!NOTE]
> **Note:** For migration to GoogleSQL refer to the [legacy SQL migration
> documentation](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/migrating-from-legacy-sql#migrating_partition_meta_table_decorator).

The `__PARTITIONS_SUMMARY__` meta-table has the following columns:

| Value | Description |
|---|---|
| `project_id` | Name of the project. |
| `dataset_id` | Name of the dataset. |
| `table_id` | Name of the time-partitioned table. |
| `partition_id` | Name (date) of the partition. |
| `creation_time` | The time at which the partition was created, in milliseconds since January 1, 1970 UTC. |
| `last_modified_time` | The time at which the partition was last modified, in milliseconds since January 1, 1970 UTC. |

At a minimum, to run a query job that uses the `__PARTITIONS_SUMMARY__` meta-table, you must be granted `bigquery.jobs.create` permissions and
`bigquery.tables.getData` permissions.

For more information on IAM roles in BigQuery, see
[Access control](https://docs.cloud.google.com/bigquery/access-control).

## Set the partition expiration

When you create a table partitioned by ingestion time or time-unit column, you
can specify a partition expiration. This setting specifies how long
BigQuery keeps the data in each partition. The setting applies to
all partitions in the table, but is calculated independently for each partition
based on the partition time.

A partition's expiration time is calculated from the partition boundary in UTC.
For example, with daily partitioning, the partition boundary is at midnight
(00:00:00 UTC). If the table's partition expiration is 6 hours, then each
partition expires at 06:00:00 UTC the following day. When a partition expires,
BigQuery deletes the data in that partition.

You can also specify a
[default partition expiration](https://docs.cloud.google.com/bigquery/docs/updating-datasets#partition-expiration)
at the dataset level. If you set the partition expiration on a table, then
the value overrides the default partition expiration. If you don't specify any
partition expiration (on the table or dataset), then partitions never expire.

> [!NOTE]
> **Note:** Integer-range partitioned tables don't support partition expiration times.

If you set a table expiration, that value takes precedence over the partition
expiration. For example, if the table expiration is set to 5 days, and the
partition expiration is set to 7 days, then the table and all partitions in it
are deleted after 5 days.

At any point after a table is created, you can update the table's partition
expiration. The new setting applies to all partitions in that table, regardless
of when they were created. Existing partitions expire immediately if they are
older than the new expiration time. Similarly, if data is being copied or
inserted to a table partitioned by time-unit column, any partitions older than
partition expiration configured for the table are expired immediately.

When a partition expires, BigQuery deletes that partition.
The partition data is retained in accordance with [time
travel](https://docs.cloud.google.com/bigquery/docs/time-travel) and
[fail-safe](https://docs.cloud.google.com/bigquery/docs/time-travel#fail-safe) policies, and can be
charged
for, depending on your
billing model. Until then, the partition counts for purposes of
[table quotas](https://docs.cloud.google.com/bigquery/quotas#partitioned_tables). To delete a partition
immediately, you can [manually delete the partition](https://docs.cloud.google.com/bigquery/docs/managing-partitioned-tables#delete_a_partition).

> [!NOTE]
> **Note:** The automatic deletion of an expired partition isn't recorded in BigQuery audit logs.

### Update the partition expiration

To update a partitioned table's partition expiration:

### Console

You cannot update the partition expiration in the Google Cloud console.

### SQL

Use the
[`ALTER TABLE SET OPTIONS` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#alter_table_set_options_statement).
The following example
updates the expiration to 5 days. To remove the partition expiration for a
table, set `partition_expiration_days` to `NULL`.

<br />

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, enter the following statement:

   ```googlesql
   ALTER TABLE mydataset.mytable
     SET OPTIONS (
       -- Sets partition expiration to 5 days
       partition_expiration_days = 5);
   ```

   <br />

3. Click **Run**.

   <br />

For more information about how to run queries, see [Run an interactive query](https://docs.cloud.google.com/bigquery/docs/running-queries#queries).

### bq

Issue the `bq update` command with the `--time_partitioning_expiration`
flag. If you are updating a partitioned table in a project other than your
default project, add the project ID to the dataset name in the following
format: `project_id:dataset`.

```bash
bq update \
--time_partitioning_expiration integer_in_seconds \
--time_partitioning_type unit_time \
project_id:dataset.table
```

Where:

- <var translate="no">integer</var> is the default lifetime (in seconds) for the table's partitions. There is no minimum value. The expiration time evaluates to the partition's date plus the integer value. If you specify `0`, the partition expiration is removed, and the partition never expires. Partitions with no expiration must be manually deleted.
- <var translate="no">unit_time</var> is either `DAY`, `HOUR`, `MONTH`, or `YEAR`, based on the table's partitioning granularity. This value must match the granularity that you set when you created the table.
- <var translate="no">project_id</var> is your project ID.
- <var translate="no">dataset</var> is the name of the dataset that contains the table you're updating.
- <var translate="no">table</var> is the name of the table you're updating.

Examples:

Enter the following command to update the expiration time of partitions in
`mydataset.mytable` to 5 days (432000 seconds). `mydataset` is in your
default project.

    bq update --time_partitioning_expiration 432000 mydataset.mytable

Enter the following command to update the expiration time of partitions in
`mydataset.mytable` to 5 days (432000 seconds). `mydataset` is in
`myotherproject`, not your default project.

    bq update \
    --time_partitioning_expiration 432000 \
    myotherproject:mydataset.mytable

### API

Call the [`tables.patch`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables/patch)
method and use the `timePartitioning.expirationMs` property to update the
partition expiration in milliseconds. Because the `tables.update` method
replaces the entire table resource, the `tables.patch` method is preferred.

## Set partition filter requirements

When you create a partitioned table, you can require that all queries on the
table must include a predicate filter (a `WHERE` clause) that filters on the
partitioning column. This setting can improve performance and reduce costs,
because BigQuery can use the filter to prune partitions that
don't match the predicate. This requirement also applies to queries on views and
materialized views that reference the partitioned table.

For information on adding the **Require partition filter** option when you
create a partitioned table, see
[Creating partitioned tables](https://docs.cloud.google.com/bigquery/docs/creating-partitioned-tables).

If a partitioned table has the **Require partition filter** setting, then every
query on that table must include at least one predicate that only references the
partitioning column. Queries without such a predicate return the following
error:

`Cannot query over table 'project_id.dataset.table' without a
filter that can be used for partition elimination`.

For more information, see
[Querying partitioned tables](https://docs.cloud.google.com/bigquery/docs/querying-partitioned-tables).

### Update the partition filter requirement

If you don't enable the **Require partition filter** option when you create the
partitioned table, you can update the table to add the option.

### Console

You cannot use the Google Cloud console to require partition filters after
a partitioned table is created.

### SQL

Use the
[`ALTER TABLE SET OPTIONS` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#alter_table_set_options_statement)
to update the partition filter requirement. The following
example updates the requirement to `true`:

<br />

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, enter the following statement:

   ```googlesql
   ALTER TABLE mydataset.mypartitionedtable
     SET OPTIONS (
       require_partition_filter = true);
   ```

   <br />

3. Click **Run**.

   <br />

For more information about how to run queries, see [Run an interactive query](https://docs.cloud.google.com/bigquery/docs/running-queries#queries).

### bq

To update a partitioned table to require partition filters by using the bq command-line tool,
enter the `bq update` command and supply the `--require_partition_filter`
flag.

To update a partitioned table in a project other than your default project,
add the project ID to the dataset in the following format:
<var translate="no">project_id:dataset</var>.

For example:

To update `mypartitionedtable` in `mydataset` in your default project,
enter:

```bash
bq update --require_partition_filter mydataset.mytable
```

To update `mypartitionedtable` in `mydataset` in `myotherproject`,
enter:

```bash
bq update --require_partition_filter myotherproject:mydataset.mytable
```

### API

Call the [`tables.patch`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables/patch)
method and set the `requirePartitionFilter` property to `true` to require
partition filters. Because the `tables.update` method replaces the entire
table resource, the `tables.patch` method is preferred.

### Java


Before trying this sample, follow the Java setup instructions in the
[BigQuery quickstart using
client libraries](https://docs.cloud.google.com/bigquery/docs/quickstarts/quickstart-client-libraries).


For more information, see the
[BigQuery Java API
reference documentation](https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/overview).


To authenticate to BigQuery, set up Application Default Credentials.
For more information, see

[Set up authentication for client libraries](https://docs.cloud.google.com/bigquery/docs/authentication#client-libs).

    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryException.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryOptions.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-biglake/latest/com.google.cloud.bigquery.biglake.v1.Table.html;

    // Sample to update require partition filter on a table.
    public class UpdateTableRequirePartitionFilter {

      public static void runUpdateTableRequirePartitionFilter() {
        // TODO(developer): Replace these variables before running the sample.
        String datasetName = "MY_DATASET_NAME";
        String tableName = "MY_TABLE_NAME";
        updateTableRequirePartitionFilter(datasetName, tableName);
      }

      public static void updateTableRequirePartitionFilter(String datasetName, String tableName) {
        try {
          // Initialize client that will be used to send requests. This client only needs to be created
          // once, and can be reused for multiple requests.
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html bigquery = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryOptions.html.getDefaultInstance().getService();

          https://docs.cloud.google.com/java/docs/reference/google-cloud-biglake/latest/com.google.cloud.bigquery.biglake.v1.Table.html table = bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html#com_google_cloud_bigquery_BigQuery_getTable_com_google_cloud_bigquery_TableId_com_google_cloud_bigquery_BigQuery_TableOption____(datasetName, tableName);
          table.https://docs.cloud.google.com/java/docs/reference/google-cloud-biglake/latest/com.google.cloud.bigquery.biglake.v1.Table.html#com_google_cloud_bigquery_biglake_v1_Table_toBuilder__().setRequirePartitionFilter(true).build().update();

          System.out.println("Table require partition filter updated successfully");
        } catch (https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryException.html e) {
          System.out.println("Table require partition filter was not updated \n" + e.toString());
        }
      }
    }

## Copy a partitioned table

The process for copying a partitioned table is the same as the process for
copying a standard table. For more information, see
[Copying a table](https://docs.cloud.google.com/bigquery/docs/managing-tables#copy-table).

When you copy a partitioned table, note the following:

-

  Copying a partitioned table to a new destination table
  :   All of the partitioning information is copied with the table. The new
      table and the old table will have identical partitions.
-

  Copying a non-partitioned table into an existing partitioned table
  :   This operation is only supported for ingestion-time partitioning.
      BigQuery copies the source data into the partition that
      represents the current date. This operation is not supported for time-unit
      column-partitioned or integer-range partitioned tables.
-

  Copying a partitioned table into another partitioned table
  :   The partition specifications for the source and destination tables must
      match.
-

  Copying a partitioned table into a non-partitioned table
  :   The destination table remains unpartitioned.
-

  Copying multiple partitioned tables

  :   If you copy multiple source tables into a partitioned table in the same
      job, the source tables can't contain a mixture of partitioned and
      non-partitioned tables.

      If all of the source tables are partitioned tables, the partition
      specifications for all source tables must match the destination table's
      partition specification.
-

  Copying a partitioned table that has a [clustering specification](https://docs.cloud.google.com/bigquery/docs/clustered-tables)

  :   If you copy into a new table, all of the clustering information is copied
      with the table. The new table and the old table will have identical
      clustering.

      If you copy into an existing table, then the cluster specifications for the
      source and destination tables must match.

When you copy to an existing table, you can specify whether to append or
overwrite the destination table.

## Copy individual partitions

You can copy the data from one or more partitions to another table.

> [!NOTE]
> **Note:** The required permissions are the same as for [copying a table](https://docs.cloud.google.com/bigquery/docs/managing-tables#copy-table).

### Console

Copying partitions is not supported by the Google Cloud console.

### bq

To copy a partition, use the bq command-line tool's `bq cp` (copy)
command with a partition decorator (`$date`) such as
`$20160201`.

Optional flags can be used to control the write disposition of the
destination partition:

- `-a` or `--append_table` appends the data from the source partition to an existing table or partition in the destination dataset.
- `-f` or `--force` overwrites an existing table or partition in the destination dataset and doesn't prompt you for confirmation.
- `-n` or `--no_clobber` returns the following error message if the table or partition exists in the destination dataset: `Table '<var>project_id:dataset.table</var> or <var>table$date</var>'
  already exists, skipping.` If `-n` is not specified, the default behavior is to prompt you to choose whether to replace the destination table or partition.
- `--destination_kms_key` is the customer-managed Cloud KMS key used to encrypt the destination table or partition.

The `cp` command does not support the `--time_partitioning_field` or
`--time_partitioning_type` flags. You cannot use a copy job to convert an
ingestion-time partitioned table into a partitioned table.

`--destination_kms_key` is not demonstrated here. See
[Protecting data with Cloud KMS keys](https://docs.cloud.google.com/bigquery/docs/customer-managed-encryption)
for more information.

If the source or destination dataset is in a project other than your default
project, add the project ID to the dataset names in the following format:
`project_id:dataset`.

(Optional) Supply the `--location` flag and set the value to your
[location](https://docs.cloud.google.com/bigquery/docs/dataset-locations).

```bash
bq --location=location cp \
-a -f -n \
project_id:dataset.source_table$source_partition \
project_id:dataset.destination_table$destination_partition
```

Where:

- <var translate="no">location</var> is the name of your location. The `--location` flag is optional. For example, if you are using BigQuery in the Tokyo region, you can set the flag's value to `asia-northeast1`. You can set a default value for the location using the [.bigqueryrc file](https://docs.cloud.google.com/bigquery/docs/bq-command-line-tool#setting_default_values_for_command-line_flags).
- <var translate="no">project_id</var> is your project ID.
- <var translate="no">dataset</var> is the name of the source or destination dataset.
- <var translate="no">source_table</var> is the table you're copying.
- <var translate="no">source_partition</var> is the partition decorator of the source partition.
- <var translate="no">destination_table</var> is the name of the table in the destination dataset.
- <var translate="no">destination_partition</var> is the partition decorator of the destination partition.

Examples:

> [!NOTE]
> **Note:** The partition decorator separator ($) is a special variable in the unix shell. You might have to escape the decorator when you use the command- line tool. The following examples escape the partition decorator: `mydataset.table\$20160519`, `'mydataset.table$20160519'`.

**Copying a partition to a new table**

Enter the following command to copy the January 30, 2018 partition from
`mydataset.mytable` to a new table --- `mydataset.mytable2`. `mydataset`
is in your default project.

    bq cp -a 'mydataset.mytable$20180130' mydataset.mytable2

**Copying a partition to a non-partitioned table**

Enter the following command to copy the January 30, 2018 partition from
`mydataset.mytable` to a non-partitioned table ---
`mydataset2.mytable2`. The `-a` shortcut is used to append the partition's
data to the non-partitioned destination table. Both datasets are in your
default project.

    bq cp -a 'mydataset.mytable$20180130' mydataset2.mytable2

Enter the following command to copy the January 30, 2018 partition from
`mydataset.mytable` to a non-partitioned table ---
`mydataset2.mytable2`. The `-f` shortcut is used to overwrite the
non-partitioned destination table without prompting.

    bq --location=US cp -f 'mydataset.mytable$20180130' mydataset2.mytable2

**Copying a partition to another partitioned table**

Enter the following command to copy the January 30, 2018 partition from
`mydataset.mytable` to another partitioned table ---
`mydataset2.mytable2`. The `-a` shortcut is used to append the partition's
data to the destination table. Since no partition decorator is specified on
the destination table, the source partition key is preserved and the data is
copied to the January 30, 2018 partition in the destination table. You can
also specify a partition decorator on the destination table to copy data to
a specific partition. `mydataset` is in your default project. `mydataset2`
is in `myotherproject`, not your default project.

    bq --location=US cp \
    -a \
    'mydataset.mytable$20180130' \
    myotherproject:mydataset2.mytable2

Enter the following command to copy the January 30, 2018 partition from
`mydataset.mytable` to the January 30, 2018 partition of another
partitioned table --- `mydataset2.mytable2`. The `-f` shortcut is used
to overwrite the January 30, 2018 partition in the destination table
without prompting. If no partition decorator is used, all data in the
destination table is overwritten. `mydataset` is in your default project.
`mydataset2` is in `myotherproject`, not your default project.

    bq cp \
    -f \
    'mydataset.mytable$20180130' \
    'myotherproject:mydataset2.mytable2$20180130'

Enter the following command to copy the January 30, 2018 partition from
`mydataset.mytable` to another partitioned table ---
`mydataset2.mytable2`. `mydataset` is in your default project. `mydataset2`
is in `myotherproject`, not your default project. If there is data in the
destination table, the default behavior is to prompt you to overwrite.

    bq cp \
    'mydataset.mytable$20180130' \
    myotherproject:mydataset2.mytable2

> [!NOTE]
> **Note:** The `bq cp` command with a partition decorator works on column-based partitions in which the source partition and destination partition are identical. The `bq cp` command also works on ingestion-time based partitions where the partition represents either the same time unit or a coarser time unit that contains the source partition. For example, if `$20180130` is the source partition decorator, valid destination partition decorators include `$20180130`, `$201801`, and `$2018`. To copy a column-based partition to a completely different partition decorator or to a time-unit partition with finer granularity, use an [`INSERT SELECT`
> statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/dml-syntax#insert_statement).

To copy multiple partitions, specify them as a comma-separated list:

```bash
bq cp \
'mydataset.mytable$20180130,mydataset.mytable$20180131' \
myotherproject:mydataset.mytable2
```

### API

Call the [`jobs.insert`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs/insert)
method, and configure a `copy` job. (Optional) Specify your region in the
`location` property in the `jobReference` section of the
[job resource](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs).

Specify the following properties in your job configuration:

- Enter the source dataset, table, and partition in the `sourceTables` property.
- Enter the destination dataset and table in the `destinationTable` property.
- Use the `writeDisposition` property to specify whether to append or overwrite the destination table or partition.

To copy multiple partitions, enter the source partitions (including the
dataset and table names) in the `sourceTables` property.

## Delete a partition

You can delete an individual partition from a partitioned table. However, you
can't delete the special `__NULL__` or `__UNPARTITIONED__` partitions.

You can only
delete one partition at a time.

> [!NOTE]
> **Note:** The required permissions are the same as for [deleting a table](https://docs.cloud.google.com/bigquery/docs/managing-tables#deleting_tables).

You can delete a partition by specifying the partition's decorator unless it is
one of the two
[special partitions](https://docs.cloud.google.com/bigquery/docs/partitioned-tables#date_timestamp_partitioned_tables).

To delete a partition in a partitioned table:

### Console

Deleting partitions is not supported by the Google Cloud console.

### SQL

If a
[qualifying `DELETE` statement](https://docs.cloud.google.com/bigquery/docs/using-dml-with-partitioned-tables#using_dml_delete_to_delete_partitions)
covers all rows in a partition,
BigQuery removes the entire partition. This removal is done
without scanning bytes or consuming slots. The following example of a
`DELETE` statement covers the entire partition of a filter on the
`_PARTITIONDATE` pseudocolumn:

<br />

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, enter the following statement:

   ```googlesql
   DELETE mydataset.mytable
   WHERE _PARTITIONDATE IN ('2076-10-07', '2076-03-06');
   ```

   <br />

3. Click **Run**.

   <br />

For more information about how to run queries, see [Run an interactive query](https://docs.cloud.google.com/bigquery/docs/running-queries#queries).

### bq

Use the `bq rm` command with the `--table` flag (or `-t` shortcut) and
specify the partition decorator to delete a specific partition.

```bash
bq rm --table project_id:dataset.table$partition
```

Where:

- <var translate="no">project_id</var> is your project ID. If omitted, your default project is used.
- <var translate="no">dataset</var> is the name of the dataset that contains the table.
- <var translate="no">table</var> is the name of the table.
- <var translate="no">partition</var> is the partition decorator of the partition you're deleting.

Partition decorators have the following format, depending on the type of
partitioning:

- Hourly partition: `yyyymmddhh`. Example: `$2016030100`.
- Daily partition: `yyyymmdd`. Example: `$20160301`.
- Monthly partition: `yyyymm`. Example: `$201603`.
- Yearly partition: `yyyy`. Example: `$2016`.
- Integer range partition: Start of the partition range. Example: `$20`.

The bq command-line tool prompts you to confirm the action. To skip the confirmation,
use the `--force` flag (or `-f` shortcut).

> [!NOTE]
> **Note:** The partition decorator separator ($) is a special variable in the unix shell. You might have to escape the decorator when you use the command- line tool. The following examples escape the partition decorator: `mydataset.table\$20160519`, `'mydataset.table$20160519'`.

Examples:

Delete the partition for March 1, 2016 in a daily partitioned table named
`mydataset.mytable` in your default project:

    bq rm --table 'mydataset.mytable$20160301'

Delete the partition for March, 2016 in a monthly partitioned table:

    bq rm --table 'mydataset.mytable$201603'

Delete the integer range starting at 20 in an integer range partitioned
table named `mydataset.mytable`:

    bq rm --table 'mydataset.mytable$20'

### API

Call the [`tables.delete`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables/delete)
method and specify the table and partition decorator using the `tableId`
parameter.

## Partitioned table security

Access control for partitioned tables is the same as access control for
standard tables. For more information, see
[Introduction to table access controls](https://docs.cloud.google.com/bigquery/docs/table-access-controls-intro).