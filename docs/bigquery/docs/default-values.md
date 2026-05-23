# Specify default column values

This page describes how to set a default value for a column in a
BigQuery table. When you add a row to a table that doesn't contain
data for a column with a default value, the default value is written to the
column instead.

## Default value expression

The default value expression for a column must be a
[literal](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/lexical#literals) or one of the
following functions:

- [`CURRENT_DATE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#current_date)
- [`CURRENT_DATETIME`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/datetime_functions#current_datetime)
- [`CURRENT_TIME`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/time_functions#current_time)
- [`CURRENT_TIMESTAMP`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#current_timestamp)
- [`GENERATE_UUID`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/utility-functions#generate_uuid)
- [`RAND`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#rand)
- [`SESSION_USER`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/security_functions#session_user)
- [`ST_GEOGPOINT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_geogpoint)

You can compose a STRUCT or ARRAY default value with these functions, such as
`[CURRENT_DATE(), DATE '2020-01-01']`.

Functions are evaluated right before the data is written to the table during job
processing. The type of the default value must match or
[coerce](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/conversion_rules#comparison_chart)
to the type of the column it applies to. If no default value is set, the default
value is `NULL`.

## Set default values

You can set the default value for columns when you create a new table. You use the
[`CREATE TABLE` DDL statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_table_statement)
and add the `DEFAULT` keyword and default value expression after the column name
and type. The following example creates a table called `simple_table` with two
`STRING` columns, `a` and `b`. Column `b` has the default value `'hello'`.

```googlesql
CREATE TABLE mydataset.simple_table (
  a STRING,
  b STRING DEFAULT 'hello');
```

When you insert data into `simple_table` that omits column `b`, the default
value `'hello'` is used instead---for example:

```googlesql
INSERT mydataset.simple_table (a) VALUES ('val1'), ('val2');
```

The table `simple_table` contains the following values:

```
+---+---+
| a    | b     |
+---+---+
| val1 | hello |
| val2 | hello |
+---+---+
```

If a column has type `STRUCT`, then you must set the default value for the
entire `STRUCT` field. You cannot set the default value for a subset of the
fields. The
default value for an array cannot be `NULL` or contain any `NULL` elements.
The following example creates a table called `complex_table` and sets a
default value for the column `struct_col`, which contains nested fields,
including an `ARRAY` type:

```googlesql
CREATE TABLE mydataset.complex_table (
  struct_col STRUCT<x STRUCT<x1 TIMESTAMP, x2 NUMERIC>, y ARRAY<DATE>>
    DEFAULT ((CURRENT_TIMESTAMP(), NULL),
             [DATE '2022-01-01', CURRENT_DATE()])
);
```

You can't set default values that violate a constraint on the column, such as
a default value that doesn't conform to a
[parameterized type](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#parameterized_data_types)
or a `NULL` default value when the column's [mode](https://docs.cloud.google.com/bigquery/docs/schemas#modes)
is `REQUIRED`.

## Change default values

To change the default value for a column, select one of the following options:

### Console

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)

   If you don't see the left pane, click **Expand left pane** to open the pane.
3. In the **Explorer** pane, expand the project, click **Datasets**, and
   then select the dataset.

4. Click **Overview \> Tables**, and then click a table.

5. Click the **Schema** tab.

6. Click **Edit schema**. You might need to scroll to see this button.

7. In the **Current schema** page, locate the top-level field that you want
   to change.

8. Enter the default value for that field.

9. Click **Save**.

### SQL

Use the
[`ALTER COLUMN SET DEFAULT` DDL statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#alter_column_set_default_statement).

<br />

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, enter the following statement:

   ```googlesql
   ALTER TABLE mydataset.mytable
   ALTER COLUMN column_name SET DEFAULT default_expression;
   ```

   <br />

3. Click **Run**.

   <br />

For more information about how to run queries, see [Run an interactive query](https://docs.cloud.google.com/bigquery/docs/running-queries#queries).

Setting the default value for a column only affects future inserts to the table.
It does not change any existing table data. The following example sets the
default value of column `a` to `SESSION_USER()`;

```googlesql
ALTER TABLE mydataset.simple_table ALTER COLUMN a SET DEFAULT SESSION_USER();
```

If you insert a row into `simple_table` that omits column `a`, the current
session user is used instead.

```googlesql
INSERT mydataset.simple_table (b) VALUES ('goodbye');
```

The table `simple_table` contains the following values:

```
+---+---+
| a                | b       |
+---+---+
| val1             | hello   |
| val2             | hello   |
| user@example.com | goodbye |
+---+---+
```

## Remove default values

To remove the default value for a column, select one of the following options:

### Console

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)

   If you don't see the left pane, click **Expand left pane** to open the pane.
3. In the **Explorer** pane, expand your project, click **Datasets**, and
   then select a dataset.

4. Click **Overview \> Tables**, and then select the table.

5. In the details pane, click the **Schema** tab.

6. Click **Edit schema**. You might need to scroll to see this button.

7. In the **Current schema** page, locate the top-level field that you want
   to change.

8. Enter `NULL` for the default value.

9. Click **Save**.

### SQL

Use the
[`ALTER COLUMN DROP DEFAULT` DDL statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#alter_column_drop_default_statement).


1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, enter the following statement:

   ```googlesql
   ALTER TABLE mydataset.mytable ALTER COLUMN column_name DROP DEFAULT;
   ```


   You can also remove the default value from a column by changing its value to
   `NULL` with the
   [`ALTER COLUMN SET DEFAULT` DDL statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#alter_column_set_default_statement).
3. Click **Run**.

   <br />

For more information about how to run queries, see [Run an interactive query](https://docs.cloud.google.com/bigquery/docs/running-queries#queries).

## Use DML statements with default values

You can add rows with default values to a table by using the
[`INSERT` DML statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/dml-syntax#insert_statement).
The default value is used when the value for a column is not specified, or when
the keyword `DEFAULT` is used in place of the value expression. The following
example creates a table and inserts a row where every value is the default
value:

```googlesql
CREATE TABLE mydataset.mytable (
  x TIME DEFAULT CURRENT_TIME(),
  y INT64 DEFAULT 5,
  z BOOL);

INSERT mydataset.mytable (x, y, z) VALUES (DEFAULT, DEFAULT, DEFAULT);
```

The table `mytable` looks like the following:

```
+---+---+---+
| x               | y | z    |
+---+---+---+
| 22:13:24.799555 | 5 | null |
+---+---+---+
```

Column `z` doesn't have a default value, so `NULL` is used as the default. When
the default value is a function, such as `CURRENT_TIME()`, it is evaluated at
the time the value is written. Calling `INSERT` with the default value for
column `x` again results in a different value for `TIME`. In the following
example, only
column `z` has a value set explicitly, and the omitted columns use their default
values:

```googlesql
INSERT mydataset.mytable (z) VALUES (TRUE);
```

The table `mytable` looks like the following:

```
+---+---+---+
| x               | y | z    |
+---+---+---+
| 22:13:24.799555 | 5 | null |
| 22:18:29.890547 | 5 | true |
+---+---+---+
```

You can update a table with default values by using the
[`MERGE` DML statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/dml-syntax#merge_statement).
The following example creates two tables and updates one of them with a `MERGE`
statement:

```googlesql
CREATE TABLE mydataset.target_table (
  a STRING,
  b STRING DEFAULT 'default_b',
  c STRING DEFAULT SESSION_USER())
AS (
  SELECT
    'val1' AS a, 'hi' AS b, '123@google.com' AS c
  UNION ALL
  SELECT
    'val2' AS a, 'goodbye' AS b, SESSION_USER() AS c
);

CREATE TABLE mydataset.source_table (
  a STRING DEFAULT 'default_val',
  b STRING DEFAULT 'Happy day!')
AS (
  SELECT
    'val1' AS a, 'Good evening！' AS b
  UNION ALL
  SELECT
    'val3' AS a, 'Good morning!' AS b
);

MERGE mydataset.target_table T
USING mydataset.source_table S
ON T.a = S.a
WHEN NOT MATCHED THEN
  INSERT(a, b) VALUES (a, DEFAULT);
```

The result is the following:

```
+---+---+---+
| a    | b         | c                  |
+---+---+---+
| val1 | hi        | 123@google.com     |
| val2 | goodbye   | default@google.com |
| val3 | default_b | default@google.com |
+---+---+---+
```

You can update a table with default values by using the
[`UPDATE` DML statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/dml-syntax#update_statement).
The following example updates the table `source_table` so that each row of
column `b` is equal to its default value:

```googlesql
UPDATE mydataset.source_table
SET b =  DEFAULT
WHERE TRUE;
```

The result is the following:

```
+---+---+
| a    | b          |
+---+---+
| val1 | Happy day! |
| val3 | Happy day! |
+---+---+
```

## Append a table

You can use the `bq query` command with the `--append_table` flag to append the
results of a query to a destination table that has default values. If the query
omits a column with a default value, the default value is assigned. The following
example appends data that specifies values only for column `z`:

```bash
bq query \
    --nouse_legacy_sql \
    --append_table \
    --destination_table=mydataset.mytable \
    'SELECT FALSE AS z UNION ALL SELECT FALSE AS Z'
```

The table `mytable` uses default values for columns `x` and `y`:

```
+---+---+---+
|        x        | y |   z   |
+---+---+---+
| 22:13:24.799555 | 5 |  NULL |
| 22:18:29.890547 | 5 |  true |
| 23:05:18.841683 | 5 | false |
| 23:05:18.841683 | 5 | false |
+---+---+---+
```

## Load data

You can [load data](https://docs.cloud.google.com/bigquery/docs/loading-data)
into a table with default values by using the
[`bq load` command](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#bq_load) or the
[`LOAD DATA` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/other-statements#load_data_statement).
Default values are applied when the loaded data has fewer columns than the
destination table. `NULL` values in the loaded data are not converted to default
values.

Binary formats, such as AVRO, Parquet, or ORC, have encoded file schemas. When
the file schema omits some columns, default values are applied.

Text formats, such as JSON and CSV, don't have encoded file schema. To specify
their schema using the bq command-line tool, you can use the `--autodetect` flag or supply a
[JSON schema](https://docs.cloud.google.com/bigquery/docs/schemas#specifying_a_json_schema_file). To specify
their schema using the `LOAD DATA` statement, you must provide a list of
columns. The following is an example that loads only column `a` from a CSV file:

```googlesql
LOAD DATA INTO mydataset.insert_table (a)
FROM FILES(
  uris = ['gs://test-bucket/sample.csv'],
  format = 'CSV');
```

## Write API

The Storage Write API only populates default values when the
[write stream](https://docs.cloud.google.com/bigquery/docs/reference/storage/rpc/google.cloud.bigquery.storage.v1#google.cloud.bigquery.storage.v1.WriteStream)
schema is missing a field that is contained in the destination table schema.
In this case, the missing field is populated with the default value on the
column for every write. If the field exists in the write stream schema but is
missing from the data itself, then the missing field is populated with `NULL`.
For example, suppose you are writing data
to a BigQuery table with the following schema:

```json
[
  {
    "name": "a",
    "mode": "NULLABLE",
    "type": "STRING",
  },
  {
    "name": "b",
    "mode": "NULLABLE",
    "type": "STRING",
    "defaultValueExpression": "'default_b'"
  },
  {
    "name": "c",
    "mode": "NULLABLE",
    "type": "STRING",
    "defaultValueExpression": "'default_c'"
  }
]
```

The following
[write stream schema](https://docs.cloud.google.com/bigquery/docs/reference/storage/rpc/google.cloud.bigquery.storage.v1#google.cloud.bigquery.storage.v1.TableSchema)
is missing the field `c` that is present in the destination table:

```json
[
  {
    "name": "a",
    "type": "STRING",
  },
  {
    "name": "b",
    "type": "STRING",
  }
]
```

Suppose you stream the following values to the table:

```json
{'a': 'val_a', 'b': 'val_b'}
{'a': 'val_a'}
```

The result is the following:

```
+---+---+---+
| a     | b     | c         |
+---+---+---+
| val_a | val_b | default_c |
| val_a | NULL  | default_c |
+---+---+---+
```

The write stream schema contains the field `b`, so the default value `default_b`
is not used even when no value is specified for the field. Since the write
stream schema doesn't contain the field `c`, every row in column `c` is
populated with the destination table's default value `default_c`.

The following write stream schema matches the schema of the table you're writing
to:

```json
[
  {
    "name": "a",
    "type": "STRING",
  },
  {
    "name": "b",
    "type": "STRING",
  }
  {
    "name": "c",
    "type": "STRING",
  }
]
```

Suppose you stream the following values to the table:

```json
{'a': 'val_a', 'b': 'val_b'}
{'a': 'val_a'}
```

The write stream schema isn't missing any fields contained in the
destination table, so none of the columns' default values are applied,
regardless of whether the fields are populated in the streamed data:

```
+---+---+---+
| a     | b     | c    |
+---+---+---+
| val_a | val_b | NULL |
| val_a | NULL  | NULL |
+---+---+---+
```

You can specify connection-level default values settings in
`default_missing_value_interpretation` within the [`AppendRowsRequest` message](https://docs.cloud.google.com/bigquery/docs/reference/storage/rpc/google.cloud.bigquery.storage.v1#google.cloud.bigquery.storage.v1.AppendRowsRequest). If the value is set to
`DEFAULT_VALUE`, the missing value will pick up the default value even when the column is
presented in the user schema.

You can also specify request-level default values in the
`missing_value_interpretations` map within the
[`AppendRowsRequest` message](https://docs.cloud.google.com/bigquery/docs/reference/storage/rpc/google.cloud.bigquery.storage.v1#google.cloud.bigquery.storage.v1.AppendRowsRequest).
Each key is the name of a column and its
[value](https://docs.cloud.google.com/bigquery/docs/reference/storage/rpc/google.cloud.bigquery.storage.v1#missingvalueinterpretation)
indicates how to interpret missing values.

For example, the map `{'col1': NULL_VALUE, 'col2': DEFAULT_VALUE}`
means that all missing values in `col1` are interpreted as `NULL`, and
all missing values in `col2` are interpreted as the default value set for `col2`
in the table schema.

If a field is not in this map and has missing values, then the missing values
are interpreted as `NULL`.

Keys can only be top-level column names. Keys can't be struct subfields, such as
`col1.subfield1`.

## Use the `insertAll` API method

The [`tabledata.insertAll` API method](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tabledata/insertAll)
populates default values at the row level when data is written to a table.
If a row is missing columns with default values, then the default values are
applied to those columns.

For example, suppose you have the following
table schema:

```json
[
  {
    "name": "a",
    "mode": "NULLABLE",
    "type": "STRING",
  },
  {
    "name": "b",
    "mode": "NULLABLE",
    "type": "STRING",
    "defaultValueExpression": "'default_b'"
  },
  {
    "name": "c",
    "mode": "NULLABLE",
    "type": "STRING",
    "defaultValueExpression": "'default_c'"
  }
]
```

Suppose you stream the following values to the table:

```json
{'a': 'val_a', 'b': 'val_b'}
{'a': 'val_a'}
{}
```

The result is the following:

```
+---+---+---+
| a     | b          | c         |
+---+---+---+
| val_a | val_b      | default_c |
| val_a | default_b  | default_c |
| NULL  | default_b  | default_c |
+---+---+---+
```

The first inserted row doesn't contain a value for the field `c`, so the default
value `default_c` is written to column `c`. The second inserted row doesn't
contain values for the fields `b` or `c`, so their default values are written to
columns `b` and `c`. The third inserted row
contains no values. The value written to column `a` is `NULL` since no other
default value is set. The default values `default_b` and `default_c` are written
to columns `b` and `c`.

## View default values

To see the default value for a column, query the
[`INFORMATION_SCHEMA.COLUMNS` view](). The `column_default` column field
contains the default value for the column. If no default value is set, it is
`NULL`. The following example shows the column names and default values for the
table `mytable`:

```googlesql
SELECT
  column_name,
  column_default
FROM
  mydataset.INFORMATION_SCHEMA.COLUMNS
WHERE
  table_name = 'mytable';
```

The result is similar to the following:

```
+---+---+
| column_name | column_default |
+---+---+
| x           | CURRENT_TIME() |
| y           | 5              |
| z           | NULL           |
+---+---+
```

## Limitations

- You can read from tables with default values by using Legacy SQL, but you cannot write to tables with default values using Legacy SQL.
- You cannot add a new column with a default value to an existing table. However, you can add the column without a default value, then change its default value by using the `ALTER COLUMN SET DEFAULT` DDL statement.
- You cannot copy and append a source table to a destination table that has more columns than the source table, and the additional columns have default values. Instead, you can run `INSERT destination_table SELECT * FROM source_table` to copy over the data.

## What's next

- For more information about loading data into BigQuery, see [Introduction to loading data](https://docs.cloud.google.com/bigquery/docs/loading-data).