# Working with JSON data in GoogleSQL

This document describes how to create a table with a `JSON` column, insert JSON
data into a BigQuery table, and query JSON data.

BigQuery natively supports JSON data using the
[`JSON`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#json_type) data type.

JSON is a widely used format that allows for semi-structured data, because it
does not require a schema. Applications can use a "schema-on-read" approach,
where the application ingests the data and then queries based on assumptions
about the schema of that data. This approach differs from the `STRUCT` type in
BigQuery, which requires a fixed schema that is enforced for all
values stored in a column of `STRUCT` type.

By using the `JSON` data type, you can load semi-structured JSON into
BigQuery without providing a schema for the JSON data upfront.
This lets you store and query data that doesn't always adhere to fixed schemas
and data types. By ingesting JSON data as a `JSON` data type,
BigQuery can encode and process each JSON field individually. You
can then query the values of fields and array elements within the JSON data by
using the field access operator, which makes JSON queries intuitive and cost
efficient.

## Limitations

- If you use a [batch load job](https://docs.cloud.google.com/bigquery/docs/batch-loading-data) to ingest JSON data into a table, the source data must be in CSV, Avro, or JSON format. Other batch load formats are not supported.
- The `JSON` data type has a nesting limit of 500.
- You can't use [legacy SQL](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/migrating-from-legacy-sql) to query a table that contains `JSON` types.
- Row-level access policies cannot be applied on `JSON` columns.

To learn about the properties of the `JSON` data type, see [`JSON` type](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#json_type).

## Create a table with a `JSON` column

You can create an empty table with a `JSON` column by using SQL or by using the
bq command-line tool.

### SQL

Use the
[`CREATE TABLE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_table_statement)
statement and declare a column with the `JSON` type.

<br />

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, enter the following statement:

   ```googlesql
   CREATE TABLE mydataset.table1(
     id INT64,
     cart JSON
   );
   ```

   <br />

3. Click **Run**.

   <br />

For more information about how to run queries, see [Run an interactive query](https://docs.cloud.google.com/bigquery/docs/running-queries#queries).

### bq

Use the [`bq mk`](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#mk-table) command
and provide a table schema with a `JSON` data type.

```bash
bq mk --table mydataset.table1 id:INT64,cart:JSON
```

You can't partition or cluster a table on `JSON` columns, because the equality
and comparison operators are not defined on the `JSON` type.

## Create `JSON` values

You can create `JSON` values in the following ways:

- Use SQL to create a [`JSON` literal](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/lexical#json_literals).
- Use the [`PARSE_JSON`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#parse_json) function to convert a `STRING` value to a [`JSON` value](https://www.json.org/json-en.html).
- Use the [`TO_JSON`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#to_json) function to convert a SQL value to a [`JSON` value](https://www.json.org/json-en.html).
- Use the [`JSON_ARRAY`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#json_array) function to create a JSON array from SQL values.
- Use the [`JSON_OBJECT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#json_object) function to create a JSON object from key-value pairs.

### Create a `JSON` value

The following example inserts `JSON` values into a table:

```googlesql
INSERT INTO mydataset.table1 VALUES
(1, JSON '{"name": "Alice", "age": 30}'),
(2, JSON_ARRAY(10, ['foo', 'bar'], [20, 30])),
(3, JSON_OBJECT('foo', 10, 'bar', ['a', 'b']));
```

### Convert a `STRING` type to `JSON` type

The following example converts a JSON-formatted `STRING` value by using the
[`PARSE_JSON`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#parse_json)
function. The example converts a column from an existing table to a `JSON` type
and saves the results in a new table.

```googlesql
CREATE OR REPLACE TABLE mydataset.table_new
AS (
  SELECT
    id, SAFE.PARSE_JSON(cart) AS cart_json
  FROM
    mydataset.old_table
);
```

The [`SAFE` prefix](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/functions-reference#safe_prefix)
used in this example ensures that any conversion errors are returned as `NULL`
values.

### Convert schematized data to JSON

The following example converts key-value pairs to JSON using the
[`JSON_OBJECT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#json_object)
function.

```googlesql
WITH Fruits AS (
SELECT 0 AS id, 'color' AS k, 'Red' AS v UNION ALL
SELECT 0, 'fruit', 'apple' UNION ALL
SELECT 1, 'fruit','banana' UNION ALL
SELECT 1, 'ripe', 'true'
)

SELECT JSON_OBJECT(ARRAY_AGG(k), ARRAY_AGG(v)) AS json_data
FROM Fruits
GROUP BY id
```

The result is the following:

```
+---+
| json_data                        |
+---+
| {"color":"Red","fruit":"apple"}  |
| {"fruit":"banana","ripe":"true"} |
+---+
```

### Convert a SQL type to `JSON` type

The following example converts a SQL `STRUCT` value to a `JSON` value by using
the [`TO_JSON`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#to_json)
function:

```googlesql
SELECT TO_JSON(STRUCT(1 AS id, [10,20] AS coordinates)) AS pt;
```

The result is the following:

```
+---+
| pt                             |
+---+
| {"coordinates":[10,20],"id":1} |
+---+
```

## Ingest JSON data

You can ingest JSON data into a BigQuery table in the following
ways:

- Use a batch load job to load into `JSON` columns from the following formats.
  - [CSV](https://docs.cloud.google.com/bigquery/docs/loading-data-cloud-storage-csv)
  - [Avro](https://docs.cloud.google.com/bigquery/docs/loading-data-cloud-storage-avro#extract_json_data_from_avro_data)
  - [JSON](https://docs.cloud.google.com/bigquery/docs/loading-data-cloud-storage-json#loading_semi-structured_json_data)
- Use the [BigQuery Storage Write API](https://docs.cloud.google.com/bigquery/docs/write-api).
- Use the legacy [`tabledata.insertAll` streaming API](https://docs.cloud.google.com/bigquery/docs/streaming-data-into-bigquery)

### Load from CSV files

The following example assumes that you have a CSV file named `file1.csv` that
contains the following records:

```
1,20
2,"""This is a string"""
3,"{""id"": 10, ""name"": ""Alice""}"
```

Note that the second column contains JSON data that is encoded as a string. This
involves correctly escaping the quotes for the CSV format. In CSV format, quotes
are escaped by using the two-character sequence `""`.

To load this file using the bq command-line tool, use the
[`bq load`](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#bq_load) command:

    bq load --source_format=CSV mydataset.table1 file1.csv id:INTEGER,json_data:JSON

    bq show mydataset.table1

    Last modified          Schema         Total Rows   Total Bytes
    --- --- --- ---
     22 Dec 22:10:32   |- id: integer       3            63
                       |- json_data: json

### Load from newline delimited JSON files

The following example assumes that you have a file named `file1.jsonl` that
contains the following records:

```
{"id": 1, "json_data": 20}
{"id": 2, "json_data": "This is a string"}
{"id": 3, "json_data": {"id": 10, "name": "Alice"}}
```

To load this file using the bq command-line tool, use the
[`bq load`](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#bq_load) command:

    bq load --source_format=NEWLINE_DELIMITED_JSON mydataset.table1 file1.jsonl id:INTEGER,json_data:JSON

    bq show mydataset.table1

    Last modified          Schema         Total Rows   Total Bytes
    --- --- --- ---
     22 Dec 22:10:32   |- id: integer       3            63
                       |- json_data: json

### Use the Storage Write API

You can use the [Storage Write API](https://docs.cloud.google.com/bigquery/docs/write-api) to
ingest JSON data. The following example uses the Storage Write API
[Python client](https://docs.cloud.google.com/bigquery/docs/write-api#python_client) to write data into a table with a JSON data type column.

Define a protocol buffer to hold the serialized streaming data. The JSON data
is encoded as a string. In the following example, the `json_col` field holds
JSON data.

```
message SampleData {
  optional string string_col = 1;
  optional int64 int64_col = 2;
  optional string json_col = 3;
}
```

Format the JSON data for each row as a `STRING` value:

```
row.json_col = '{"a": 10, "b": "bar"}'
row.json_col = '"This is a string"' # The double-quoted string is the JSON value.
row.json_col = '10'
```

Append the rows to the write stream as shown in the
[code example](https://docs.cloud.google.com/bigquery/docs/write-api-streaming#at-least-once).
The client library handles serialization to protocol buffer format.

If you aren't able to format the incoming JSON data, you need to use the `json.dumps()` method in your code. Here is an example:

```
import json

...

row.json_col = json.dumps({"a": 10, "b": "bar"})
row.json_col = json.dumps("This is a string") # The double-quoted string is the JSON value.
row.json_col = json.dumps(10)

...

```

### Use the legacy streaming API

The following example loads JSON data from a local file and streams it to
a BigQuery table with a JSON data-type column named `json_data` using the
[legacy streaming API](https://docs.cloud.google.com/bigquery/docs/streaming-data-into-bigquery).

    from google.cloud import https://docs.cloud.google.com/python/docs/reference/bigquery/latest
    import json

    # TODO(developer): Replace these variables before running the sample.
    project_id = 'MY_PROJECT_ID'
    table_id = 'MY_TABLE_ID'

    client = https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html(project=project_id)
    table_obj = client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_get_table(table_id)

    # The column json_data is represented as a JSON data-type column.
    rows_to_insert = [
        {"id": 1, "json_data": 20},
        {"id": 2, "json_data": "This is a string"},
        {"id": 3, "json_data": {"id": 10, "name": "Alice"}}
    ]

    # If the column json_data is represented as a String data type, modify the rows_to_insert values:
    #rows_to_insert = [
    #    {"id": 1, "json_data": json.dumps(20)},
    #    {"id": 2, "json_data": json.dumps("This is a string")},
    #    {"id": 3, "json_data": json.dumps({"id": 10, "name": "Alice"})}
    #]

    # Throw errors if encountered.
    # https://cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client#google_cloud_bigquery_client_Client_insert_rows

    errors = client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_insert_rows(table=table_obj, rows=rows_to_insert)
    if errors == []:
        print("New rows have been added.")
    else:
        print("Encountered errors while inserting rows: {}".format(errors))

For more information, see
[Streaming data into BigQuery](https://docs.cloud.google.com/bigquery/docs/streaming-data-into-bigquery#streaminginsertexamples).

## Query JSON data

This section describes how to use GoogleSQL to extract values from
JSON. JSON is case-sensitive and supports UTF-8 in both fields and values.

The examples in this section use the following table:

```googlesql
CREATE OR REPLACE TABLE mydataset.table1(id INT64, cart JSON);

INSERT INTO mydataset.table1 VALUES
(1, JSON """{
        "name": "Alice",
        "items": [
            {"product": "book", "price": 10},
            {"product": "food", "price": 5}
        ]
    }"""),
(2, JSON """{
        "name": "Bob",
        "items": [
            {"product": "pen", "price": 20}
        ]
    }""");
```

### Extract values as JSON

Given a `JSON` type in BigQuery, you can access the fields in a
JSON expression by using the [field access operator](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/operators#field_access_operator).
The following example returns the `name` field of the `cart` column.

```googlesql
SELECT cart.name
FROM mydataset.table1;
```

```
+---+
|  name   |
+---+
| "Alice" |
| "Bob"   |
+---+
```

To access an array element, use the
[JSON subscript operator](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/operators#json_subscript_operator).
The following example returns the first element of the `items` array:

```googlesql
SELECT
  cart.items[0] AS first_item
FROM mydataset.table1
```

```
+---+
|          first_item           |
+---+
| {"price":10,"product":"book"} |
| {"price":20,"product":"pen"}  |
+---+
```

You can also use the JSON subscript operator to reference the members of a JSON
object by name:

```googlesql
SELECT cart['name']
FROM mydataset.table1;
```

```
+---+
|  name   |
+---+
| "Alice" |
| "Bob"   |
+---+
```

For subscript operations, the expression inside the brackets can be any
arbitrary string or integer expression, including non-constant expressions:

```googlesql
DECLARE int_val INT64 DEFAULT 0;

SELECT
  cart[CONCAT('it','ems')][int_val + 1].product AS item
FROM mydataset.table1;
```

```
+---+
|  item  |
+---+
| "food" |
| NULL   |
+---+
```

Field access and subscript operators both return `JSON` types, so you can chain
expressions that use them or pass the result to other functions that take `JSON`
types.

These operators improve readability for the basic functionality of the
[`JSON_QUERY`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#json_query)
function. For example, the expression
`cart.name` is equivalent to `JSON_QUERY(cart, "$.name")`.

If a member with the specified name is not found in the JSON object, or if
the JSON array doesn't have an element with the specified position, then
these operators return SQL `NULL`.

```googlesql
SELECT
  cart.address AS address,
  cart.items[1].price AS item1_price
FROM
  mydataset.table1;
```

```
+---+---+
| address | item1_price |
+---+---+
| NULL    | NULL        |
| NULL    | 5           |
+---+---+
```

The equality and comparison operators are not defined on the `JSON` data type.
Therefore, you can't use `JSON` values directly in clauses like `GROUP BY` or
`ORDER BY`. Instead, use the `JSON_VALUE` function to extract field values as
SQL strings, as described in the next section.

### Extract values as strings

The [`JSON_VALUE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#json_value)
function extracts a scalar value and returns it as a SQL string. It returns SQL
`NULL` if `cart.name` doesn't point to a scalar value in the JSON.

```googlesql
SELECT JSON_VALUE(cart.name) AS name
FROM mydataset.table1;
```

```
+---+
| name  |
+---+
| Alice |
+---+
```

You can use the `JSON_VALUE` function in contexts that require equality or
comparison, such as `WHERE` clauses and `GROUP BY` clauses. The following
example shows a `WHERE` clause that filters against a `JSON` value:

```googlesql
SELECT
  cart.items[0] AS first_item
FROM
  mydataset.table1
WHERE
  JSON_VALUE(cart.name) = 'Alice';
```

```
+---+
| first_item                    |
+---+
| {"price":10,"product":"book"} |
+---+
```

Alternatively, you can use the [`STRING`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#string_for_json)
function which extracts a JSON string and returns that value as a SQL `STRING`.
For example:

```googlesql
SELECT STRING(JSON '"purple"') AS color;
```

```
+---+
| color  |
+---+
| purple |
+---+
```

In addition to [`STRING`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#string_for_json),
you might have to extract `JSON` values and return them as another SQL data
type. The following value extraction functions are available:

- [`STRING`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#string_for_json)
- [`BOOL`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#bool_for_json)
- [`INT64`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#int64_for_json)
- [`FLOAT64`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#double_for_json)

To obtain the type of the `JSON` value, you can use the [`JSON_TYPE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#json_type)
function.

### Flexibly convert JSON

You can convert a `JSON` value to a scalar SQL value flexibly
with [`LAX conversion`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#lax_converters)
functions.

The following example uses the [`LAX_INT64` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#lax_int64)
to extract an `INT64` value from a `JSON` value.

```googlesql
SELECT LAX_INT64(JSON '"10"') AS id;
```

```
+---+
| id |
+---+
| 10 |
+---+
```

<br />

In addition to [`LAX_INT64`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#lax_int64),
you can convert to other SQL types flexibly to JSON with the following
functions:

- [`LAX_STRING`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#lax_string)
- [`LAX_BOOL`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#lax_bool)
- [`LAX_INT64`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#lax_int64)
- [`LAX_FLOAT64`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#lax_double)

### Extract arrays from JSON

JSON can contain JSON arrays, which are not directly equivalent to an
`ARRAY<JSON>` type in BigQuery. You can use the following
functions to extract a BigQuery `ARRAY` from JSON:

- [`JSON_QUERY_ARRAY`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#json_query_array): extracts an array and returns it as an `ARRAY<JSON>` of JSON.
- [`JSON_VALUE_ARRAY`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#json_value_array): extracts an array of scalar values and returns it as an `ARRAY<STRING>` of scalar values.

The following example uses `JSON_QUERY_ARRAY` to extract JSON arrays:

```googlesql
SELECT JSON_QUERY_ARRAY(cart.items) AS items
FROM mydataset.table1;
```

```
+---+
| items                                                          |
+---+
| [{"price":10,"product":"book"}","{"price":5,"product":"food"}] |
| [{"price":20,"product":"pen"}]                                 |
+---+
```

To split an array into its individual elements, use the
[`UNNEST`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#unnest_operator)
operator, which returns a table with one row for each element in the array. The
following example selects the `product` member from each member of the `items`
array:

```googlesql
SELECT
  id,
  JSON_VALUE(item.product) AS product
FROM
  mydataset.table1, UNNEST(JSON_QUERY_ARRAY(cart.items)) AS item
ORDER BY id;
```

```
+---+---+
| id | product |
+---+---+
|  1 | book    |
|  1 | food    |
|  2 | pen     |
+---+---+
```

The next example is similar but uses the
[`ARRAY_AGG`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#array_agg)
function to aggregate the values back into a SQL array.

```googlesql
SELECT
  id,
  ARRAY_AGG(JSON_VALUE(item.product)) AS products
FROM
  mydataset.table1, UNNEST(JSON_QUERY_ARRAY(cart.items)) AS item
GROUP BY id
ORDER BY id;
```

```
+---+---+
| id | products        |
+---+---+
|  1 | ["book","food"] |
|  2 | ["pen"]         |
+---+---+
```

For more information about arrays, see
[Working with arrays in GoogleSQL](https://docs.cloud.google.com/bigquery/docs/arrays).

## JSON nulls

The `JSON` type has a special `null` value that is different from the SQL
`NULL`. A JSON `null` is not treated as a SQL `NULL` value, as the following
example shows.

```googlesql
SELECT JSON 'null' IS NULL;
```

```
+---+
| f0_   |
+---+
| false |
+---+
```

When you extract a JSON field with a `null` value, the behavior depends on the
function:

- The `JSON_QUERY` function returns a JSON `null`, because it is a valid `JSON` value.
- The `JSON_VALUE` function returns the SQL `NULL`, because JSON `null` is not a scalar value.

The following example shows the different behaviors:

```googlesql
SELECT
  json.a AS json_query, -- Equivalent to JSON_QUERY(json, '$.a')
  JSON_VALUE(json, '$.a') AS json_value
FROM (SELECT JSON '{"a": null}' AS json);
```

```
+---+---+
| json_query | json_value |
+---+---+
| null       | NULL       |
+---+---+
```

> [!CAUTION]
> **Caution:** This behavior does not apply to JSON values stored in a `STRING` type. When passed a `STRING` value, the `JSON_QUERY` function returns SQL `NULL` values in place of JSON `null` values.