# Query multiple tables using a wildcard table

> [!NOTE]
> **Note:** Wildcard tables have many [limitations](https://docs.cloud.google.com/bigquery/docs/querying-wildcard-tables#limitations) and are less performant than regular BigQuery tables that take advantage of [partitioning](https://docs.cloud.google.com/bigquery/docs/partitioned-tables) and [clustering](https://docs.cloud.google.com/bigquery/docs/clustered-tables). As a best practice, use partitioned or clustered tables when possible.

Wildcard tables enable you to query multiple tables using concise SQL
statements. Wildcard tables are available only in GoogleSQL. For equivalent
functionality in legacy SQL, see
[Table wildcard functions](https://docs.cloud.google.com/bigquery/query-reference#tablewildcardfunctions).

A wildcard table represents a union of all the tables that match the wildcard
expression. For example, the following `FROM` clause uses the wildcard
expression `gsod*` to match all tables in the `noaa_gsod` dataset that begin
with the string `gsod`.

    FROM
      `bigquery-public-data.noaa_gsod.gsod*`

Each row in the wildcard table contains a special column, `_TABLE_SUFFIX`, which contains
the value matched by the wildcard character.

## Limitations

Wildcard table queries are subject to the following limitations.

- The wildcard table functionality doesn't support views. If the wildcard table matches any view in the dataset, the query returns an error even if your query contains a `WHERE` clause on the `_TABLE_SUFFIX` pseudocolumn to filter out the view.
- Cached results are not supported for queries against multiple tables using a wildcard even if the **Use Cached Results** option is checked. If you run the same wildcard query multiple times, you are billed for each query.
- Wildcard tables support built-in BigQuery storage only. You cannot use wildcards to query an [external table](https://docs.cloud.google.com/bigquery/external-data-sources) or a [view](https://docs.cloud.google.com/bigquery/docs/views).
- You cannot use wildcard queries over tables with incompatible partitioning or a mix of partitioned and non-partitioned tables. The queried tables also need to have identical clustering specifications.
- You can use wildcard tables with partitioned tables, and both partition pruning and cluster pruning are supported. However, tables that are clustered but not partitioned don't get the full cluster pruning benefit from wildcard usage.
- Queries that contain [data manipulation language](https://docs.cloud.google.com/bigquery/docs/data-manipulation-language) (DML) statements cannot use a wildcard table as the target of the query. For example, a wildcard table may be used in the `FROM` clause of an `UPDATE` query, but a wildcard table cannot be used as the target of the `UPDATE` operation.
- Filters on the `_TABLE_SUFFIX` or `_PARTITIONTIME` pseudocolumns that include JavaScript user-defined functions don't limit the number of tables scanned in a wildcard table.
- Wildcard queries are not supported for tables protected by customer-managed encryption keys (CMEK).
- All tables referenced in a wildcard query must have exactly the same set of [tag](https://docs.cloud.google.com/bigquery/docs/tags) keys and values.
- If a single scanned table has a schema mismatch (that is, a column with the
  same name is of a different type), the query fails with the error **Cannot
  read field of type X as Y Field: column_name** . All tables are matched even
  if you are using the equality operator `=`. For example, in the following
  query, the table `my_dataset.my_table_03_backup` is also scanned. Thus, the
  query may fail due to schema mismatch. However, if there is no schema
  mismatch, the results come from the table `my_dataset.my_table_03` only, as
  expected.

      SELECT *
      FROM my_project.my_dataset.my_table_*
      WHERE _TABLE_SUFFIX = '03'

  <br />

- Wildcard tables aren't supported by BI Engine.

- Wildcard tables aren't supported for tables with flexible column names or
  renamed columns. To work around this limitation, exclude the affected columns
  from the query or filter the table using the `_TABLE_SUFFIX` pseudocolumn.

## Before you begin

- Ensure that you are using GoogleSQL. For more information, see [Switching SQL dialects](https://docs.cloud.google.com/bigquery/sql-reference/enabling-standard-sql).
- If you are using legacy SQL, see [Table wildcard functions](https://docs.cloud.google.com/bigquery/query-reference#tablewildcardfunctions).
- Many of the examples on this page use a public dataset from the National Oceanic and Atmospheric Administration (NOAA). For more information about the data, see [NOAA Global Surface Summary of the Day Weather Data](https://console.cloud.google.com/bigquery?p=bigquery-public-data&d=noaa_gsod&page=dataset).

## When to use wildcard tables

Wildcard tables are useful when a dataset contains multiple, similarly named
tables that have compatible schemas. Typically, such datasets contain tables
that each represent data from a single day, month, or year. For example, a
public dataset hosted by BigQuery, the
[NOAA Global Surface Summary of the Day Weather Data](https://console.cloud.google.com/bigquery?p=bigquery-public-data&d=noaa_gsod&page=dataset),
contains a table for each year from 1929 through the present.

A query that scans all the table IDs from 1929 through 1940 would be very long
if you have to name all 12 tables in the `FROM` clause (most of the tables are
omitted in this sample):

```googlesql
#standardSQL
SELECT
  max,
  ROUND((max-32)*5/9,1) celsius,
  mo,
  da,
  year
FROM (
  SELECT
    *
  FROM
    `bigquery-public-data.noaa_gsod.gsod1929` UNION ALL
  SELECT
    *
  FROM
    `bigquery-public-data.noaa_gsod.gsod1930` UNION ALL
  SELECT
    *
  FROM
    `bigquery-public-data.noaa_gsod.gsod1931` UNION ALL

  # ... Tables omitted for brevity

  SELECT
    *
  FROM
    `bigquery-public-data.noaa_gsod.gsod1940` )
WHERE
  max != 9999.9 # code for missing data
ORDER BY
  max DESC
```

The same query using a wildcard table is much more concise:

```googlesql
#standardSQL
SELECT
  max,
  ROUND((max-32)*5/9,1) celsius,
  mo,
  da,
  year
FROM
  `bigquery-public-data.noaa_gsod.gsod19*`
WHERE
  max != 9999.9 # code for missing data
  AND _TABLE_SUFFIX BETWEEN '29'
  AND '40'
ORDER BY
  max DESC
```
Wildcard tables support built-in BigQuery storage only. You cannot use wildcards when querying an [external table](https://docs.cloud.google.com/bigquery/external-data-sources) or a [view](https://docs.cloud.google.com/bigquery/docs/views).

## Wildcard table syntax

Wildcard table syntax:

```
SELECT
FROM
  `<project-id>.<dataset-id>.<table-prefix>*`
WHERE
  bool_expression
```

\<project-id\>
:   Cloud Platform project ID. Optional if you use your default project ID.

\<dataset-id\>
:   BigQuery dataset ID.

\<table-prefix\>
:   A string that is common across all tables that are matched by the wildcard
    character. The table prefix is optional. Omitting the table prefix matches
    all tables in the dataset.

\* (wildcard character)
:   The wildcard character, "\*", represents one or more characters of a table
    name. The wildcard character can appear only as the final character of a
    wildcard table name.

Queries with wildcard tables support the `_TABLE_SUFFIX` pseudocolumn in the
`WHERE` clause. This column contains the values matched by the wildcard
character, so that queries can filter which tables are accessed. For example,
the following `WHERE` clauses use
[comparison operators](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/operators#comparison_operators)
to filter the matched tables:

    WHERE
      _TABLE_SUFFIX BETWEEN '29' AND '40'

    WHERE
      _TABLE_SUFFIX = '1929'

    WHERE
      _TABLE_SUFFIX < '1941'

For more information about the `_TABLE_SUFFIX` pseudocolumn, see
[Filtering selected tables using _TABLE_SUFFIX](https://docs.cloud.google.com/bigquery/docs/querying-wildcard-tables#filter_selected_tables_using_table_suffix).

### Enclose table names with wildcards in backticks

The wildcard table name contains the special character (\*), which means that
you must enclose the wildcard table name in backtick (\`) characters. For
example, the following query is valid because it uses backticks:

```googlesql
#standardSQL
/* Valid SQL query */
SELECT
  max
FROM
  `bigquery-public-data.noaa_gsod.gsod*`
WHERE
  max != 9999.9 # code for missing data
  AND _TABLE_SUFFIX = '1929'
ORDER BY
  max DESC
```

The following query is NOT valid because it isn't properly quoted with backticks:

```googlesql
#standardSQL
/* Syntax error: Expected end of statement but got "-" at [4:11] */
SELECT
  max
FROM
  # missing backticks
  bigquery-public-data.noaa_gsod.gsod*
WHERE
  max != 9999.9 # code for missing data
  AND _TABLE_SUFFIX = '1929'
ORDER BY
  max DESC
```

Quotation marks don't work:

```googlesql
#standardSQL
/* Syntax error: Unexpected string literal: 'bigquery-public-data.noaa_gsod.gsod*' at [4:3] */
SELECT
  max
FROM
  # quotes are not backticks
  'bigquery-public-data.noaa_gsod.gsod*'
WHERE
  max != 9999.9 # code for missing data
  AND _TABLE_SUFFIX = '1929'
ORDER BY
  max DESC
```

## Query tables using wildcard tables

Wildcard tables enable you to query several tables concisely.
For example, a public dataset hosted by BigQuery,
the [NOAA Global Surface Summary of the Day Weather Data](https://console.cloud.google.com/bigquery?p=bigquery-public-data&d=noaa_gsod&page=dataset),
contains a table for each year from 1929 through the present that all share the
common prefix `gsod` followed by the four-digit year. The tables are named
`gsod1929`, `gsod1930`, `gsod1931`, etc.

To query a group of tables that share a common prefix, use the table wildcard
symbol (\*) after the table prefix in your `FROM` statement. For example,
the following query finds the maximum temperature reported during the 1940s:

    #standardSQL
    SELECT
      max,
      ROUND((max-32)*5/9,1) celsius,
      mo,
      da,
      year
    FROM
      `bigquery-public-data.noaa_gsod.gsod194*`
    WHERE
      max != 9999.9 # code for missing data
    ORDER BY
      max DESC

## Filter selected tables using _TABLE_SUFFIX

To restrict a query so that it scans only a specified set of tables, use the
`_TABLE_SUFFIX` pseudocolumn in a `WHERE` clause with a condition that is a constant expression.

The `_TABLE_SUFFIX` pseudocolumn contains the values matched by the table
wildcard. For example, the previous sample query, which scans all tables from
the 1940s, uses a table wildcard to represent the last digit of the year:

    FROM
      `bigquery-public-data.noaa_gsod.gsod194*`

The corresponding `_TABLE_SUFFIX` pseudocolumn contains values in the range
`0` through `9`, representing the tables `gsod1940` through `gsod1949`. These
`_TABLE_SUFFIX` values can be used in a `WHERE` clause to filter for specific
tables.

For example, to filter for the maximum temperature in the years 1940 and 1944,
use the values `0` and `4` for `_TABLE_SUFFIX`:

    #standardSQL
    SELECT
      max,
      ROUND((max-32)*5/9,1) celsius,
      mo,
      da,
      year
    FROM
      `bigquery-public-data.noaa_gsod.gsod194*`
    WHERE
      max != 9999.9 # code for missing data
      AND ( _TABLE_SUFFIX = '0'
        OR _TABLE_SUFFIX = '4' )
    ORDER BY
      max DESC

Using `_TABLE_SUFFIX` can greatly reduce the number of bytes scanned, which helps reduce the cost of running your queries.

However, filters on `_TABLE_SUFFIX` that include conditions without constant
expressions don't limit the number of tables scanned in a wildcard table. For
example, the following query does not limit the tables scanned for the wildcard
table `bigquery-public-data.noaa_gsod.gsod19*` because the filter uses the
dynamic value of the `table_id` column:

    #standardSQL
    # Scans all tables that match the prefix `gsod19`
    SELECT
      ROUND((max-32)*5/9,1) celsius
    FROM
      `bigquery-public-data.noaa_gsod.gsod19*`
    WHERE
      _TABLE_SUFFIX = (SELECT SUBSTR(MAX(table_name), LENGTH('gsod19') + 1)
          FROM `bigquery-public-data.noaa_gsod.INFORMATION_SCHEMA.TABLES`
          WHERE table_name LIKE 'gsod194%')

As another example, the following query limits the scan based on the first
filter condition,
`_TABLE_SUFFIX BETWEEN '40' and '60'`, because it is a constant expression.
However, the following query does not limit the scan based on the second filter
condition, ``_TABLE_SUFFIX = (SELECT SUBSTR(MAX(table_name), LENGTH('gsod19')
+ 1) FROM `bigquery-public-data.noaa_gsod.INFORMATION_SCHEMA.TABLES` WHERE table_name LIKE
'gsod194%')``, because it is a dynamic expression:

    #standardSQL
    # Scans all tables with names that fall between `gsod1940` and `gsod1960`
    SELECT
      ROUND((max-32)*5/9,1) celsius
    FROM
      `bigquery-public-data.noaa_gsod.gsod19*`
    WHERE
      _TABLE_SUFFIX BETWEEN '40' AND '60'
      AND _TABLE_SUFFIX = (SELECT SUBSTR(MAX(table_name), LENGTH('gsod19') + 1)
          FROM `bigquery-public-data.noaa_gsod.INFORMATION_SCHEMA.TABLES`
          WHERE table_name LIKE 'gsod194%')

As a workaround, you can perform two separate queries instead; for example:

First query:

    #standardSQL
    # Get the list of tables that match the required table name prefixes
    SELECT SUBSTR(MAX(table_name), LENGTH('gsod19') + 1)
          FROM `bigquery-public-data.noaa_gsod.INFORMATION_SCHEMA.TABLES`
          WHERE table_name LIKE 'gsod194%'

Second query:

    #standardSQL
    # Construct the second query based on the values from the first query
    SELECT
      ROUND((max-32)*5/9,1) celsius
    FROM
      `bigquery-public-data.noaa_gsod.gsod19*`
    WHERE _TABLE_SUFFIX = '49'

These example queries use the `INFORMATION_SCHEMA.TABLES` view. For more
information about the `INFORMATION_SCHEMA` table, see [Getting table
metadata using INFORMATION_SCHEMA](https://docs.cloud.google.com/bigquery/docs/information-schema-tables).

## Scanning a range of tables using _TABLE_SUFFIX

To scan a range of tables, use the `_TABLE_SUFFIX` pseudocolumn along with
the `BETWEEN` clause. For example, to find the maximum temperature reported in
the years between 1929 and 1935 inclusive, use the table wildcard to represent
the last two digits of the year:

    #standardSQL
    SELECT
      max,
      ROUND((max-32)*5/9,1) celsius,
      mo,
      da,
      year
    FROM
      `bigquery-public-data.noaa_gsod.gsod19*`
    WHERE
      max != 9999.9 # code for missing data
      AND _TABLE_SUFFIX BETWEEN '29' and '35'
    ORDER BY
      max DESC

## Scanning a range of ingestion-time partitioned tables using _PARTITIONTIME

To scan a range of ingestion-time partitioned tables, use the `_PARTITIONTIME`
pseudocolumn with the `_TABLE_SUFFIX` pseudocolumn. For example, the following query scans the January 1, 2017 partition in the table `my_dataset.mytable_id1`.

    #standardSQL
    SELECT
      field1,
      field2,
      field3
    FROM
      `my_dataset.mytable_*`
    WHERE
      _TABLE_SUFFIX = 'id1'
      AND _PARTITIONTIME = TIMESTAMP('2017-01-01')

## Querying all tables in a dataset

To scan all tables in a dataset, you can use an empty prefix and the table
wildcard, which means that the `_TABLE_SUFFIX` pseudocolumn contains
full table names. For example, the following `FROM` clause scans all tables in
the GSOD dataset:

    FROM
      `bigquery-public-data.noaa_gsod.*`

With an empty prefix, the `_TABLE_SUFFIX` pseudocolumn contains full table
names. For example, the following query is equivalent to the previous example
that finds the maximum temperature between the years 1929 and 1935, but uses
full table names in the `WHERE` clause:

    #standardSQL
    SELECT
      max,
      ROUND((max-32)*5/9,1) celsius,
      mo,
      da,
      year
    FROM
      `bigquery-public-data.noaa_gsod.*`
    WHERE
      max != 9999.9 # code for missing data
      AND _TABLE_SUFFIX BETWEEN 'gsod1929' and 'gsod1935'
    ORDER BY
      max DESC

Note, however, that longer prefixes generally perform better. For more
information, see [Best practices](https://docs.cloud.google.com/bigquery/docs/wildcard-tables#best_practices).

## Query execution details

### Schema used for query evaluation

In order to execute a GoogleSQL query that uses a wildcard table,
BigQuery automatically infers the schema for that table.
BigQuery uses the schema for the most recently created table
that matches the wildcard as the schema for the wildcard table. Even if you
restrict the number of tables that you want to use from the wildcard table using
the `_TABLE_SUFFIX` pseudocolumn in a `WHERE` clause, BigQuery
uses the schema for the most recently created table that matches the wildcard.

If a column from the inferred schema doesn't exist in a matched table, then
BigQuery returns `NULL` values for that column in the rows for
the table that is missing the column.

If the schema is inconsistent across the tables matched by the wildcard
query, then BigQuery returns an error. This is the case when
the columns of the matched tables have different data types, or when the columns
which are not present in all of the matched tables cannot be assumed to have a
null value.

## Best practices

- Longer prefixes generally perform better than shorter prefixes. For example,
  the following query uses a long prefix (`gsod200`):

  ```googlesql
  #standardSQL
  SELECT
  max
  FROM
  `bigquery-public-data.noaa_gsod.gsod200*`
  WHERE
  max != 9999.9 # code for missing data
  AND _TABLE_SUFFIX BETWEEN '0' AND '1'
  ORDER BY
  max DESC
  ```

  The following query generally performs worse because it uses an empty
  prefix:

  ```googlesql
  #standardSQL
  SELECT
  max
  FROM
  `bigquery-public-data.noaa_gsod.*`
  WHERE
  max != 9999.9 # code for missing data
  AND _TABLE_SUFFIX BETWEEN 'gsod2000' AND 'gsod2001'
  ORDER BY
  max DESC
  ```
- Partitioning is recommended over sharding, because partitioned tables perform
  better. Sharding reduces performance while creating more tables to manage. For
  more information, see [Partitioning versus sharding](https://docs.cloud.google.com/bigquery/docs/partitioned-tables#dt_partition_shard).

For best practices for controlling costs in BigQuery, see [Controlling costs in BigQuery](https://docs.cloud.google.com/bigquery/docs/best-practices-costs)

## What's next

- For more information about GoogleSQL, see the [GoogleSQL query reference](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql).