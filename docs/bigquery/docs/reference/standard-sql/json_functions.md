GoogleSQL for BigQuery supports the following functions, which can retrieve and
transform JSON data.

## Categories

The JSON functions are grouped into the following categories based on their
behavior:

| Category | Functions | Description |
|---|---|---|
| Standard extractors | [`JSON_QUERY`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#json_query) [`JSON_VALUE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#json_value) [`JSON_QUERY_ARRAY`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#json_query_array) [`JSON_VALUE_ARRAY`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#json_value_array) | Functions that extract JSON data. |
| Legacy extractors | [`JSON_EXTRACT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#json_extract) [`JSON_EXTRACT_SCALAR`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#json_extract_scalar) [`JSON_EXTRACT_ARRAY`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#json_extract_array) [`JSON_EXTRACT_STRING_ARRAY`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#json_extract_string_array) | Functions that extract JSON data. While these functions are supported by GoogleSQL, we recommend using the [standard extractor functions](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#extractors). |
| Lax converters | [`LAX_BOOL`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#lax_bool) [`LAX_FLOAT64`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#lax_double) [`LAX_INT64`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#lax_int64) [`LAX_STRING`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#lax_string) | Functions that flexibly convert a JSON value to a SQL value without returning errors. |
| Converters | [`BOOL`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#bool_for_json) [`FLOAT64`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#double_for_json) [`INT64`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#int64_for_json) [`STRING`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#string_for_json) | Functions that convert a JSON value to a SQL value. |
| Other converters | [`PARSE_JSON`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#parse_json) [`TO_JSON`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#to_json) [`TO_JSON_STRING`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#to_json_string) | Other conversion functions from or to JSON. |
| Constructors | [`JSON_ARRAY`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#json_array) [`JSON_OBJECT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#json_object) | Functions that create JSON. |
| Mutators | [`JSON_ARRAY_APPEND`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#json_array_append) [`JSON_ARRAY_INSERT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#json_array_insert) [`JSON_REMOVE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#json_remove) [`JSON_SET`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#json_set) [`JSON_STRIP_NULLS`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#json_strip_nulls) | Functions that mutate existing JSON. |
| Accessors | [`JSON_KEYS`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#json_keys) [`JSON_TYPE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#json_type) | Functions that provide access to JSON properties. |
| Transformers | [`JSON_FLATTEN`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#json_flatten) | Functions that apply a transformation to a JSON value. |

## Function list

| Name | Summary |
|---|---|
| [`BOOL`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#bool_for_json) | Converts a JSON boolean to a SQL `BOOL` value. |
| [`FLOAT64`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#double_for_json) | Converts a JSON number to a SQL `FLOAT64` value. |
| [`INT64`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#int64_for_json) | Converts a JSON number to a SQL `INT64` value. |
| [`JSON_ARRAY`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#json_array) | Creates a JSON array. |
| [`JSON_ARRAY_APPEND`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#json_array_append) | Appends JSON data to the end of a JSON array. |
| [`JSON_ARRAY_INSERT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#json_array_insert) | Inserts JSON data into a JSON array. |
| [`JSON_EXTRACT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#json_extract) | (Deprecated) Extracts a JSON value and converts it to a SQL JSON-formatted `STRING` or `JSON` value. |
| [`JSON_EXTRACT_ARRAY`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#json_extract_array) | (Deprecated) Extracts a JSON array and converts it to a SQL `ARRAY<JSON-formatted STRING>` or `ARRAY<JSON>` value. |
| [`JSON_EXTRACT_SCALAR`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#json_extract_scalar) | (Deprecated) Extracts a JSON scalar value and converts it to a SQL `STRING` value. |
| [`JSON_EXTRACT_STRING_ARRAY`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#json_extract_string_array) | (Deprecated) Extracts a JSON array of scalar values and converts it to a SQL `ARRAY<STRING>` value. |
| [`JSON_FLATTEN`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#json_flatten) | Produces a new SQL `ARRAY<JSON>` value containing all non-array values that are either directly in the input JSON value or children of one or more consecutively nested arrays in the input JSON value. |
| [`JSON_KEYS`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#json_keys) | Extracts unique JSON keys from a JSON expression. |
| [`JSON_OBJECT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#json_object) | Creates a JSON object. |
| [`JSON_QUERY`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#json_query) | Extracts a JSON value and converts it to a SQL JSON-formatted `STRING` or `JSON` value. |
| [`JSON_QUERY_ARRAY`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#json_query_array) | Extracts a JSON array and converts it to a SQL `ARRAY<JSON-formatted STRING>` or `ARRAY<JSON>` value. |
| [`JSON_REMOVE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#json_remove) | Produces JSON with the specified JSON data removed. |
| [`JSON_SET`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#json_set) | Inserts or replaces JSON data. |
| [`JSON_STRIP_NULLS`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#json_strip_nulls) | Removes JSON nulls from JSON objects and JSON arrays. |
| [`JSON_TYPE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#json_type) | Gets the JSON type of the outermost JSON value and converts the name of this type to a SQL `STRING` value. |
| [`JSON_VALUE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#json_value) | Extracts a JSON scalar value and converts it to a SQL `STRING` value. |
| [`JSON_VALUE_ARRAY`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#json_value_array) | Extracts a JSON array of scalar values and converts it to a SQL `ARRAY<STRING>` value. |
| [`LAX_BOOL`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#lax_bool) | Attempts to convert a JSON value to a SQL `BOOL` value. |
| [`LAX_FLOAT64`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#lax_double) | Attempts to convert a JSON value to a SQL `FLOAT64` value. |
| [`LAX_INT64`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#lax_int64) | Attempts to convert a JSON value to a SQL `INT64` value. |
| [`LAX_STRING`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#lax_string) | Attempts to convert a JSON value to a SQL `STRING` value. |
| [`PARSE_JSON`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#parse_json) | Converts a JSON-formatted `STRING` value to a `JSON` value. |
| [`STRING` (JSON)](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#string_for_json) | Converts a JSON string to a SQL `STRING` value. |
| [`TO_JSON`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#to_json) | Converts a SQL value to a JSON value. |
| [`TO_JSON_STRING`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#to_json_string) | Converts a SQL value to a JSON-formatted `STRING` value. |

## `BOOL`

    BOOL(json_expr)

**Description**

Converts a JSON boolean to a SQL `BOOL` value.

Arguments:

- `json_expr`: JSON. For example:

      JSON 'true'

  If the JSON value isn't a boolean, an error is produced. If the expression
  is SQL `NULL`, the function returns SQL `NULL`.

**Return type**

`BOOL`

**Examples**

    SELECT BOOL(JSON 'true') AS vacancy;

    /*---+
     | vacancy |
     +---+
     | true    |
     +---*/

    SELECT BOOL(JSON_QUERY(JSON '{"hotel class": "5-star", "vacancy": true}', "$.vacancy")) AS vacancy;

    /*---+
     | vacancy |
     +---+
     | true    |
     +---*/

The following examples show how invalid requests are handled:

    -- An error is thrown if JSON isn't of type bool.
    SELECT BOOL(JSON '123') AS result; -- Throws an error
    SELECT BOOL(JSON 'null') AS result; -- Throws an error
    SELECT SAFE.BOOL(JSON '123') AS result; -- Returns a SQL NULL

## `FLOAT64`

    FLOAT64(
      json_expr
      [, wide_number_mode => { 'exact' | 'round' } ]
    )

**Description**

Converts a JSON number to a SQL `FLOAT64` value.

Arguments:

- `json_expr`: JSON. For example:

      JSON '9.8'

  If the JSON value isn't a number, an error is produced. If the expression
  is a SQL `NULL`, the function returns SQL `NULL`.
- `wide_number_mode`: A named argument with a `STRING` value.
  Defines what happens with a number that can't be
  represented as a `FLOAT64` without loss of
  precision. This argument accepts one of the two case-sensitive values:

  - `exact`: The function fails if the result can't be represented as a `FLOAT64` without loss of precision.
  - `round` (default): The numeric value stored in JSON will be rounded to `FLOAT64`. If such rounding isn't possible, the function fails.

**Return type**

`FLOAT64`

**Examples**

    SELECT FLOAT64(JSON '9.8') AS velocity;

    /*---+
     | velocity |
     +---+
     | 9.8      |
     +---*/

    SELECT FLOAT64(JSON_QUERY(JSON '{"vo2_max": 39.1, "age": 18}', "$.vo2_max")) AS vo2_max;

    /*---+
     | vo2_max |
     +---+
     | 39.1    |
     +---*/

    SELECT FLOAT64(JSON '18446744073709551615', wide_number_mode=>'round') as result;

    /*---+
     | result                 |
     +---+
     | 1.8446744073709552e+19 |
     +---*/

    SELECT FLOAT64(JSON '18446744073709551615') as result;

    /*---+
     | result                 |
     +---+
     | 1.8446744073709552e+19 |
     +---*/

The following examples show how invalid requests are handled:

    -- An error is thrown if JSON isn't of type FLOAT64.
    SELECT FLOAT64(JSON '"strawberry"') AS result;
    SELECT FLOAT64(JSON 'null') AS result;

    -- An error is thrown because `wide_number_mode` is case-sensitive and not "exact" or "round".
    SELECT FLOAT64(JSON '123.4', wide_number_mode=>'EXACT') as result;
    SELECT FLOAT64(JSON '123.4', wide_number_mode=>'exac') as result;

    -- An error is thrown because the number can't be converted to DOUBLE without loss of precision
    SELECT FLOAT64(JSON '18446744073709551615', wide_number_mode=>'exact') as result;

    -- Returns a SQL NULL
    SELECT SAFE.FLOAT64(JSON '"strawberry"') AS result;

## `INT64`

    INT64(json_expr)

**Description**

Converts a JSON number to a SQL `INT64` value.

Arguments:

- `json_expr`: JSON. For example:

      JSON '999'

  If the JSON value isn't a number, or the JSON number isn't in the SQL
  `INT64` domain, an error is produced. If the expression is SQL `NULL`, the
  function returns SQL `NULL`.

**Return type**

`INT64`

**Examples**

    SELECT INT64(JSON '2005') AS flight_number;

    /*---+
     | flight_number |
     +---+
     | 2005          |
     +---*/

    SELECT INT64(JSON_QUERY(JSON '{"gate": "A4", "flight_number": 2005}', "$.flight_number")) AS flight_number;

    /*---+
     | flight_number |
     +---+
     | 2005          |
     +---*/

    SELECT INT64(JSON '10.0') AS score;

    /*---+
     | score |
     +---+
     | 10    |
     +---*/

The following examples show how invalid requests are handled:

    -- An error is thrown if JSON isn't a number or can't be converted to a 64-bit integer.
    SELECT INT64(JSON '10.1') AS result;  -- Throws an error
    SELECT INT64(JSON '"strawberry"') AS result; -- Throws an error
    SELECT INT64(JSON 'null') AS result; -- Throws an error
    SELECT SAFE.INT64(JSON '"strawberry"') AS result;  -- Returns a SQL NULL

## `JSON_ARRAY`

    JSON_ARRAY([value][, ...])

**Description**

Creates a JSON array from zero or more SQL values.

Arguments:

- `value`: A [JSON encoding-supported](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#json_encodings) value to add to a JSON array.

**Return type**

`JSON`

**Examples**

The following query creates a JSON array with one value in it:

    SELECT JSON_ARRAY(10) AS json_data

    /*---+
     | json_data |
     +---+
     | [10]      |
     +---*/

You can create a JSON array with an empty JSON array in it. For example:

    SELECT JSON_ARRAY([]) AS json_data

    /*---+
     | json_data |
     +---+
     | [[]]      |
     +---*/

    SELECT JSON_ARRAY(10, 'foo', NULL) AS json_data

    /*---+
     | json_data       |
     +---+
     | [10,"foo",null] |
     +---*/

    SELECT JSON_ARRAY(STRUCT(10 AS a, 'foo' AS b)) AS json_data

    /*---+
     | json_data            |
     +---+
     | [{"a":10,"b":"foo"}] |
     +---*/

    SELECT JSON_ARRAY(10, ['foo', 'bar'], [20, 30]) AS json_data

    /*---+
     | json_data                  |
     +---+
     | [10,["foo","bar"],[20,30]] |
     +---*/

    SELECT JSON_ARRAY(10, [JSON '20', JSON '"foo"']) AS json_data

    /*---+
     | json_data       |
     +---+
     | [10,[20,"foo"]] |
     +---*/

You can create an empty JSON array. For example:

    SELECT JSON_ARRAY() AS json_data

    /*---+
     | json_data |
     +---+
     | []        |
     +---*/

## `JSON_ARRAY_APPEND`

    JSON_ARRAY_APPEND(
      json_expr,
      json_path_value_pair[, ...]
      [, append_each_element => { TRUE | FALSE } ]
    )

    json_path_value_pair:
      json_path, value

Appends JSON data to the end of a JSON array.

Arguments:

- `json_expr`: JSON. For example:

      JSON '["a", "b", "c"]'

- `json_path_value_pair`: A value and the [JSONPath](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#JSONPath_format) for
  that value. This includes:

  - `json_path`: Append `value` at this [JSONPath](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#JSONPath_format)
    in `json_expr`.

  - `value`: A [JSON encoding-supported](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#json_encodings) value to
    append.

- `append_each_element`: A named argument with a `BOOL` value.

  - If `TRUE` (default), and `value` is a SQL array,
    appends each element individually.

  - If `FALSE,` and `value` is a SQL array, appends
    the array as one element.

Details:

- Path value pairs are evaluated left to right. The JSON produced by evaluating one pair becomes the JSON against which the next pair is evaluated.
- The operation is ignored if the path points to a JSON non-array value that isn't a JSON null.
- If `json_path` points to a JSON null, the JSON null is replaced by a JSON array that contains `value`.
- If the path exists but has an incompatible type at any given path token, the path value pair operation is ignored.
- The function applies all path value pair append operations even if an individual path value pair operation is invalid. For invalid operations, the operation is ignored and the function continues to process the rest of the path value pairs.
- If any `json_path` is an invalid [JSONPath](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#JSONPath_format), an error is produced.
- If `json_expr` is SQL `NULL`, the function returns SQL `NULL`.
- If `append_each_element` is SQL `NULL`, the function returns `json_expr`.
- If `json_path` is SQL `NULL`, the `json_path_value_pair` operation is ignored.

**Return type**

`JSON`

**Examples**

In the following example, path `$` is matched and appends `1`.

    SELECT JSON_ARRAY_APPEND(JSON '["a", "b", "c"]', '$', 1) AS json_data

    /*---+
     | json_data       |
     +---+
     | ["a","b","c",1] |
     +---*/

In the following example, `append_each_element` defaults to `TRUE`, so
`[1, 2]` is appended as individual elements.

    SELECT JSON_ARRAY_APPEND(JSON '["a", "b", "c"]', '$', [1, 2]) AS json_data

    /*---+
     | json_data         |
     +---+
     | ["a","b","c",1,2] |
     +---*/

In the following example, `append_each_element` is `FALSE`, so
`[1, 2]` is appended as one element.

    SELECT JSON_ARRAY_APPEND(
      JSON '["a", "b", "c"]',
      '$', [1, 2],
      append_each_element=>FALSE) AS json_data

    /*---+
     | json_data           |
     +---+
     | ["a","b","c",[1,2]] |
     +---*/

In the following example, `append_each_element` is `FALSE`, so
`[1, 2]` and `[3, 4]` are each appended as one element.

    SELECT JSON_ARRAY_APPEND(
      JSON '["a", ["b"], "c"]',
      '$[1]', [1, 2],
      '$[1][1]', [3, 4],
      append_each_element=>FALSE) AS json_data

    /*---+
     | json_data                   |
     +---+
     | ["a",["b",[1,2,[3,4]]],"c"] |
     +---*/

In the following example, the first path `$[1]` appends `[1, 2]` as single
elements, and then the second path `$[1][1]` isn't a valid path to an array,
so the second operation is ignored.

    SELECT JSON_ARRAY_APPEND(
      JSON '["a", ["b"], "c"]',
      '$[1]', [1, 2],
      '$[1][1]', [3, 4]) AS json_data

    /*---+
     | json_data           |
     +---+
     | ["a",["b",1,2],"c"] |
     +---*/

In the following example, path `$.a` is matched and appends `2`.

    SELECT JSON_ARRAY_APPEND(JSON '{"a": [1]}', '$.a', 2) AS json_data

    /*---+
     | json_data   |
     +---+
     | {"a":[1,2]} |
     +---*/

In the following example, a value is appended into a JSON null.

    SELECT JSON_ARRAY_APPEND(JSON '{"a": null}', '$.a', 10)

    /*---+
     | json_data  |
     +---+
     | {"a":[10]} |
     +---*/

In the following example, path `$.a` isn't an array, so the operation is
ignored.

    SELECT JSON_ARRAY_APPEND(JSON '{"a": 1}', '$.a', 2) AS json_data

    /*---+
     | json_data |
     +---+
     | {"a":1}   |
     +---*/

In the following example, path `$.b` doesn't exist, so the operation is
ignored.

    SELECT JSON_ARRAY_APPEND(JSON '{"a": 1}', '$.b', 2) AS json_data

    /*---+
     | json_data |
     +---+
     | {"a":1}   |
     +---*/

## `JSON_ARRAY_INSERT`

    JSON_ARRAY_INSERT(
      json_expr,
      json_path_value_pair[, ...]
      [, insert_each_element => { TRUE | FALSE } ]
    )

    json_path_value_pair:
      json_path, value

Produces a new JSON value that's created by inserting JSON data into
a JSON array.

Arguments:

- `json_expr`: JSON. For example:

      JSON '["a", "b", "c"]'

- `json_path_value_pair`: A value and the [JSONPath](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#JSONPath_format) for
  that value. This includes:

  - `json_path`: Insert `value` at this [JSONPath](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#JSONPath_format)
    in `json_expr`.

  - `value`: A [JSON encoding-supported](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#json_encodings) value to
    insert.

- `insert_each_element`: A named argument with a `BOOL` value.

  - If `TRUE` (default), and `value` is a SQL array,
    inserts each element individually.

  - If `FALSE,` and `value` is a SQL array, inserts
    the array as one element.

Details:

- Path value pairs are evaluated left to right. The JSON produced by evaluating one pair becomes the JSON against which the next pair is evaluated.
- The operation is ignored if the path points to a JSON non-array value that isn't a JSON null.
- If `json_path` points to a JSON null, the JSON null is replaced by a JSON array of the appropriate size and padded on the left with JSON nulls.
- If the path exists but has an incompatible type at any given path token, the path value pair operator is ignored.
- The function applies all path value pair append operations even if an individual path value pair operation is invalid. For invalid operations, the operation is ignored and the function continues to process the rest of the path value pairs.
- If the array index in `json_path` is larger than the size of the array, the function extends the length of the array to the index, fills in the array with JSON nulls, then adds `value` at the index.
- If any `json_path` is an invalid [JSONPath](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#JSONPath_format), an error is produced.
- If `json_expr` is SQL `NULL`, the function returns SQL `NULL`.
- If `insert_each_element` is SQL `NULL`, the function returns `json_expr`.
- If `json_path` is SQL `NULL`, the `json_path_value_pair` operation is ignored.

**Return type**

`JSON`

**Examples**

In the following example, path `$[1]` is matched and inserts `1`.

    SELECT JSON_ARRAY_INSERT(JSON '["a", ["b", "c"], "d"]', '$[1]', 1) AS json_data

    /*---+
     | json_data             |
     +---+
     | ["a",1,["b","c"],"d"] |
     +---*/

In the following example, path `$[1][0]` is matched and inserts `1`.

    SELECT JSON_ARRAY_INSERT(JSON '["a", ["b", "c"], "d"]', '$[1][0]', 1) AS json_data

    /*---+
     | json_data             |
     +---+
     | ["a",[1,"b","c"],"d"] |
     +---*/

In the following example, `insert_each_element` defaults to `TRUE`, so
`[1, 2]` is inserted as individual elements.

    SELECT JSON_ARRAY_INSERT(JSON '["a", "b", "c"]', '$[1]', [1, 2]) AS json_data

    /*---+
     | json_data         |
     +---+
     | ["a",1,2,"b","c"] |
     +---*/

In the following example, `insert_each_element` is `FALSE`, so `[1, 2]` is
inserted as one element.

    SELECT JSON_ARRAY_INSERT(
      JSON '["a", "b", "c"]',
      '$[1]', [1, 2],
      insert_each_element=>FALSE) AS json_data

    /*---+
     | json_data           |
     +---+
     | ["a",[1,2],"b","c"] |
     +---*/

In the following example, path `$[7]` is larger than the length of the
matched array, so the array is extended with JSON nulls and `"e"` is inserted at
the end of the array.

    SELECT JSON_ARRAY_INSERT(JSON '["a", "b", "c", "d"]', '$[7]', "e") AS json_data

    /*---+
     | json_data                            |
     +---+
     | ["a","b","c","d",null,null,null,"e"] |
     +---*/

In the following example, path `$.a` is an object, so the operation is ignored.

    SELECT JSON_ARRAY_INSERT(JSON '{"a": {}}', '$.a[0]', 2) AS json_data

    /*---+
     | json_data |
     +---+
     | {"a":{}}  |
     +---*/

In the following example, path `$` doesn't specify a valid array position,
so the operation is ignored.

    SELECT JSON_ARRAY_INSERT(JSON '[1, 2]', '$', 3) AS json_data

    /*---+
     | json_data |
     +---+
     | [1,2]     |
     +---*/

In the following example, a value is inserted into a JSON null.

    SELECT JSON_ARRAY_INSERT(JSON '{"a": null}', '$.a[2]', 10) AS json_data

    /*---+
     | json_data            |
     +---+
     | {"a":[null,null,10]} |
     +---*/

In the following example, the operation is ignored because you can't insert
data into a JSON number.

    SELECT JSON_ARRAY_INSERT(JSON '1', '$[0]', 'r1') AS json_data

    /*---+
     | json_data |
     +---+
     | 1         |
     +---*/

## `JSON_EXTRACT`

> [!NOTE]
> **Note:** This function is deprecated. Consider using [JSON_QUERY](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#json_query).

    JSON_EXTRACT(json_string_expr, json_path)

    JSON_EXTRACT(json_expr, json_path)

**Description**

Extracts a JSON value and converts it to a
SQL JSON-formatted `STRING` or `JSON` value.
This function uses single quotes and brackets to escape invalid
[JSONPath](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#JSONPath_format) characters in JSON keys. For example: `['a.b']`.

Arguments:

- `json_string_expr`: A JSON-formatted string. For example:

      '{"class": {"students": [{"name": "Jane"}]}}'

  Extracts a SQL `NULL` when a JSON-formatted string `null` is encountered.
  For example:

      SELECT JSON_EXTRACT("null", "$") -- Returns a SQL NULL

- `json_expr`: JSON. For example:

      JSON '{"class": {"students": [{"name": "Jane"}]}}'

  Extracts a JSON `null` when a JSON `null` is encountered.

      SELECT JSON_EXTRACT(JSON 'null', "$") -- Returns a JSON 'null'

- `json_path`: The [JSONPath](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#JSONPath_format). This identifies the data that
  you want to obtain from the input.

There are differences between the JSON-formatted string and JSON input types.
For details, see [Differences between the JSON and JSON-formatted STRING types](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#differences_json_and_string).

**Return type**

- `json_string_expr`: A JSON-formatted `STRING`
- `json_expr`: `JSON`

**Examples**

In the following example, JSON data is extracted and returned as JSON.

    SELECT
      JSON_EXTRACT(JSON '{"class": {"students": [{"id": 5}, {"id": 12}]}}', '$.class')
      AS json_data;

    /*---+
     | json_data                         |
     +---+
     | {"students":[{"id":5},{"id":12}]} |
     +---*/

In the following examples, JSON data is extracted and returned as
JSON-formatted strings.

    SELECT JSON_EXTRACT(
      '{"class": {"students": [{"name": "Jane"}]}}',
      '$') AS json_text_string;

    /*---+
     | json_text_string                                          |
     +---+
     | {"class":{"students":[{"name":"Jane"}]}}                  |
     +---*/

    SELECT JSON_EXTRACT(
      '{"class": {"students": []}}',
      '$') AS json_text_string;

    /*---+
     | json_text_string                                          |
     +---+
     | {"class":{"students":[]}}                                 |
     +---*/

    SELECT JSON_EXTRACT(
      '{"class": {"students": [{"name": "John"}, {"name": "Jamie"}]}}',
      '$') AS json_text_string;

    /*---+
     | json_text_string                                          |
     +---+
     | {"class":{"students":[{"name":"John"},{"name":"Jamie"}]}} |
     +---*/

    SELECT JSON_EXTRACT(
      '{"class": {"students": [{"name": "Jane"}]}}',
      '$.class.students[0]') AS first_student;

    /*---+
     | first_student   |
     +---+
     | {"name":"Jane"} |
     +---*/

    SELECT JSON_EXTRACT(
      '{"class": {"students": []}}',
      '$.class.students[0]') AS first_student;

    /*---+
     | first_student   |
     +---+
     | NULL            |
     +---*/

    SELECT JSON_EXTRACT(
      '{"class": {"students": [{"name": "John"}, {"name": "Jamie"}]}}',
      '$.class.students[0]') AS first_student;

    /*---+
     | first_student   |
     +---+
     | {"name":"John"} |
     +---*/

    SELECT JSON_EXTRACT(
      '{"class": {"students": [{"name": "Jane"}]}}',
      '$.class.students[1].name') AS second_student;

    /*---+
     | second_student |
     +---+
     | NULL           |
     +---*/

    SELECT JSON_EXTRACT(
      '{"class": {"students": []}}',
      '$.class.students[1].name') AS second_student;

    /*---+
     | second_student |
     +---+
     | NULL           |
     +---*/

    SELECT JSON_EXTRACT(
      '{"class": {"students": [{"name": "John"}, {"name": null}]}}',
      '$.class.students[1].name') AS second_student;

    /*---+
     | second_student |
     +---+
     | NULL           |
     +---*/

    SELECT JSON_EXTRACT(
      '{"class": {"students": [{"name": "John"}, {"name": "Jamie"}]}}',
      '$.class.students[1].name') AS second_student;

    /*---+
     | second_student |
     +---+
     | "Jamie"        |
     +---*/

    SELECT JSON_EXTRACT(
      '{"class": {"students": [{"name": "Jane"}]}}',
      "$.class['students']") AS student_names;

    /*---+
     | student_names                      |
     +---+
     | [{"name":"Jane"}]                  |
     +---*/

    SELECT JSON_EXTRACT(
      '{"class": {"students": []}}',
      "$.class['students']") AS student_names;

    /*---+
     | student_names                      |
     +---+
     | []                                 |
     +---*/

    SELECT JSON_EXTRACT(
      '{"class": {"students": [{"name": "John"}, {"name": "Jamie"}]}}',
      "$.class['students']") AS student_names;

    /*---+
     | student_names                      |
     +---+
     | [{"name":"John"},{"name":"Jamie"}] |
     +---*/

    SELECT JSON_EXTRACT('{"a": null}', "$.a"); -- Returns a SQL NULL
    SELECT JSON_EXTRACT('{"a": null}', "$.b"); -- Returns a SQL NULL

    SELECT JSON_EXTRACT(JSON '{"a": null}', "$.a"); -- Returns a JSON 'null'
    SELECT JSON_EXTRACT(JSON '{"a": null}', "$.b"); -- Returns a SQL NULL

## `JSON_EXTRACT_ARRAY`

> [!NOTE]
> **Note:** This function is deprecated. Consider using [JSON_QUERY_ARRAY](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#json_query_array).

    JSON_EXTRACT_ARRAY(json_string_expr[, json_path])

    JSON_EXTRACT_ARRAY(json_expr[, json_path])

**Description**

Extracts a JSON array and converts it to
a SQL `ARRAY<JSON-formatted STRING>` or
`ARRAY<JSON>` value.
This function uses single quotes and brackets to escape invalid
[JSONPath](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#JSONPath_format) characters in JSON keys. For example: `['a.b']`.

Arguments:

- `json_string_expr`: A JSON-formatted string. For example:

      '["a", "b", {"key": "c"}]'

- `json_expr`: JSON. For example:

      JSON '["a", "b", {"key": "c"}]'

- `json_path`: The [JSONPath](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#JSONPath_format). This identifies the data that
  you want to obtain from the input. If this optional parameter isn't
  provided, then the JSONPath `$` symbol is applied, which means that all of
  the data is analyzed.

There are differences between the JSON-formatted string and JSON input types.
For details, see [Differences between the JSON and JSON-formatted STRING types](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#differences_json_and_string).

**Return type**

- `json_string_expr`: `ARRAY<JSON-formatted STRING>`
- `json_expr`: `ARRAY<JSON>`

**Examples**

This extracts items in JSON to an array of `JSON` values:

    SELECT JSON_EXTRACT_ARRAY(
      JSON '{"fruits":["apples","oranges","grapes"]}','$.fruits'
      ) AS json_array;

    /*---+
     | json_array                      |
     +---+
     | ["apples", "oranges", "grapes"] |
     +---*/

This extracts the items in a JSON-formatted string to a string array:

    SELECT JSON_EXTRACT_ARRAY('[1,2,3]') AS string_array;

    /*---+
     | string_array |
     +---+
     | [1, 2, 3]    |
     +---*/

This extracts a string array and converts it to an integer array:

    SELECT ARRAY(
      SELECT CAST(integer_element AS INT64)
      FROM UNNEST(
        JSON_EXTRACT_ARRAY('[1,2,3]','$')
      ) AS integer_element
    ) AS integer_array;

    /*---+
     | integer_array |
     +---+
     | [1, 2, 3]     |
     +---*/

This extracts string values in a JSON-formatted string to an array:

    -- Doesn't strip the double quotes
    SELECT JSON_EXTRACT_ARRAY('["apples", "oranges", "grapes"]', '$') AS string_array;

    /*---+
     | string_array                    |
     +---+
     | ["apples", "oranges", "grapes"] |
     +---*/

    -- Strips the double quotes
    SELECT ARRAY(
      SELECT JSON_EXTRACT_SCALAR(string_element, '$')
      FROM UNNEST(JSON_EXTRACT_ARRAY('["apples","oranges","grapes"]','$')) AS string_element
    ) AS string_array;

    /*---+
     | string_array              |
     +---+
     | [apples, oranges, grapes] |
     +---*/

This extracts only the items in the `fruit` property to an array:

    SELECT JSON_EXTRACT_ARRAY(
      '{"fruit": [{"apples": 5, "oranges": 10}, {"apples": 2, "oranges": 4}], "vegetables": [{"lettuce": 7, "kale": 8}]}',
      '$.fruit'
    ) AS string_array;

    /*---+
     | string_array                                          |
     +---+
     | [{"apples":5,"oranges":10}, {"apples":2,"oranges":4}] |
     +---*/

These are equivalent:

    SELECT JSON_EXTRACT_ARRAY('{"fruits": ["apples", "oranges", "grapes"]}', '$[fruits]') AS string_array;

    SELECT JSON_EXTRACT_ARRAY('{"fruits": ["apples", "oranges", "grapes"]}', '$.fruits') AS string_array;

    -- The queries above produce the following result:
    /*---+
     | string_array                    |
     +---+
     | ["apples", "oranges", "grapes"] |
     +---*/

In cases where a JSON key uses invalid JSONPath characters, you can escape those
characters using single quotes and brackets, `[' ']`. For example:

    SELECT JSON_EXTRACT_ARRAY('{"a.b": {"c": ["world"]}}', "$['a.b'].c") AS hello;

    /*---+
     | hello     |
     +---+
     | ["world"] |
     +---*/

The following examples explore how invalid requests and empty arrays are
handled:

- If a JSONPath is invalid, an error is thrown.
- If a JSON-formatted string is invalid, the output is NULL.
- It's okay to have empty arrays in the JSON-formatted string.

    -- An error is thrown if you provide an invalid JSONPath.
    SELECT JSON_EXTRACT_ARRAY('["foo", "bar", "baz"]', 'INVALID_JSONPath') AS result;

    -- If the JSONPath doesn't refer to an array, then NULL is returned.
    SELECT JSON_EXTRACT_ARRAY('{"a": "foo"}', '$.a') AS result;

    /*---+
     | result |
     +---+
     | NULL   |
     +---*/

    -- If a key that doesn't exist is specified, then the result is NULL.
    SELECT JSON_EXTRACT_ARRAY('{"a": "foo"}', '$.b') AS result;

    /*---+
     | result |
     +---+
     | NULL   |
     +---*/

    -- Empty arrays in JSON-formatted strings are supported.
    SELECT JSON_EXTRACT_ARRAY('{"a": "foo", "b": []}', '$.b') AS result;

    /*---+
     | result |
     +---+
     | []     |
     +---*/

## `JSON_EXTRACT_SCALAR`

> [!NOTE]
> **Note:** This function is deprecated. Consider using [JSON_VALUE](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#json_value).

    JSON_EXTRACT_SCALAR(json_string_expr[, json_path])

    JSON_EXTRACT_SCALAR(json_expr[, json_path])

**Description**

Extracts a JSON scalar value and converts it to a SQL `STRING` value.
In addition, this function:

- Removes the outermost quotes and unescapes the return values.
- Returns a SQL `NULL` if a non-scalar value is selected.
- Uses single quotes and brackets to escape invalid [JSONPath](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#JSONPath_format) characters in JSON keys. For example: `['a.b']`.

Arguments:

- `json_string_expr`: A JSON-formatted string. For example:

      '{"name": "Jane", "age": "6"}'

- `json_expr`: JSON. For example:

      JSON '{"name": "Jane", "age": "6"}'

- `json_path`: The [JSONPath](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#JSONPath_format). This identifies the data that
  you want to obtain from the input. If this optional parameter isn't
  provided, then the JSONPath `$` symbol is applied, which means that all of
  the data is analyzed.

  If `json_path` returns a JSON `null` or a non-scalar value (in other words,
  if `json_path` refers to an object or an array), then a SQL `NULL` is
  returned.

There are differences between the JSON-formatted string and JSON input types.
For details, see [Differences between the JSON and JSON-formatted STRING types](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#differences_json_and_string).

**Return type**

`STRING`

**Examples**

In the following example, `age` is extracted.

    SELECT JSON_EXTRACT_SCALAR(JSON '{"name": "Jakob", "age": "6" }', '$.age') AS scalar_age;

    /*---+
     | scalar_age |
     +---+
     | 6          |
     +---*/

The following example compares how results are returned for the `JSON_EXTRACT`
and `JSON_EXTRACT_SCALAR` functions.

    SELECT JSON_EXTRACT('{"name": "Jakob", "age": "6" }', '$.name') AS json_name,
      JSON_EXTRACT_SCALAR('{"name": "Jakob", "age": "6" }', '$.name') AS scalar_name,
      JSON_EXTRACT('{"name": "Jakob", "age": "6" }', '$.age') AS json_age,
      JSON_EXTRACT_SCALAR('{"name": "Jakob", "age": "6" }', '$.age') AS scalar_age;

    /*---+---+---+---+
     | json_name | scalar_name | json_age | scalar_age |
     +---+---+---+---+
     | "Jakob"   | Jakob       | "6"      | 6          |
     +---+---+---+---*/

    SELECT JSON_EXTRACT('{"fruits": ["apple", "banana"]}', '$.fruits') AS json_extract,
      JSON_EXTRACT_SCALAR('{"fruits": ["apple", "banana"]}', '$.fruits') AS json_extract_scalar;

    /*---+---+
     | json_extract       | json_extract_scalar |
     +---+---+
     | ["apple","banana"] | NULL                |
     +---+---*/

In cases where a JSON key uses invalid JSONPath characters, you can escape those
characters using single quotes and brackets, `[' ']`. For example:

    SELECT JSON_EXTRACT_SCALAR('{"a.b": {"c": "world"}}', "$['a.b'].c") AS hello;

    /*---+
     | hello |
     +---+
     | world |
     +---*/

## `JSON_EXTRACT_STRING_ARRAY`

> [!NOTE]
> **Note:** This function is deprecated. Consider using [JSON_VALUE_ARRAY](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#json_value_array).

    JSON_EXTRACT_STRING_ARRAY(json_string_expr[, json_path])

    JSON_EXTRACT_STRING_ARRAY(json_expr[, json_path])

**Description**

Extracts a JSON array of scalar values and converts it to a SQL `ARRAY<STRING>`
value. In addition, this function:

- Removes the outermost quotes and unescapes the values.
- Returns a SQL `NULL` if the selected value isn't an array or not an array containing only scalar values.
- Uses single quotes and brackets to escape invalid [JSONPath](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#JSONPath_format) characters in JSON keys. For example: `['a.b']`.

Arguments:

- `json_string_expr`: A JSON-formatted string. For example:

      '["apples", "oranges", "grapes"]'

- `json_expr`: JSON. For example:

      JSON '["apples", "oranges", "grapes"]'

- `json_path`: The [JSONPath](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#JSONPath_format). This identifies the data that
  you want to obtain from the input. If this optional parameter isn't
  provided, then the JSONPath `$` symbol is applied, which means that all of
  the data is analyzed.

There are differences between the JSON-formatted string and JSON input types.
For details, see [Differences between the JSON and JSON-formatted STRING types](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#differences_json_and_string).

Caveats:

- A JSON `null` in the input array produces a SQL `NULL` as the output for that JSON `null`. If the output contains a `NULL` array element, an error is produced because the final output can't be an array with `NULL` values.
- If a JSONPath matches an array that contains scalar objects and a JSON `null`, then the output of the function must be transformed because the final output can't be an array with `NULL` values.

**Return type**

`ARRAY<STRING>`

**Examples**

This extracts items in JSON to a string array:

    SELECT JSON_EXTRACT_STRING_ARRAY(
      JSON '{"fruits": ["apples", "oranges", "grapes"]}', '$.fruits'
      ) AS string_array;

    /*---+
     | string_array              |
     +---+
     | [apples, oranges, grapes] |
     +---*/

The following example compares how results are returned for the
`JSON_EXTRACT_ARRAY` and `JSON_EXTRACT_STRING_ARRAY` functions.

    SELECT JSON_EXTRACT_ARRAY('["apples", "oranges"]') AS json_array,
    JSON_EXTRACT_STRING_ARRAY('["apples", "oranges"]') AS string_array;

    /*---+---+
     | json_array            | string_array      |
     +---+---+
     | ["apples", "oranges"] | [apples, oranges] |
     +---+---*/

This extracts the items in a JSON-formatted string to a string array:

    -- Strips the double quotes
    SELECT JSON_EXTRACT_STRING_ARRAY('["foo", "bar", "baz"]', '$') AS string_array;

    /*---+
     | string_array    |
     +---+
     | [foo, bar, baz] |
     +---*/

This extracts a string array and converts it to an integer array:

    SELECT ARRAY(
      SELECT CAST(integer_element AS INT64)
      FROM UNNEST(
        JSON_EXTRACT_STRING_ARRAY('[1, 2, 3]', '$')
      ) AS integer_element
    ) AS integer_array;

    /*---+
     | integer_array |
     +---+
     | [1, 2, 3]     |
     +---*/

These are equivalent:

    SELECT JSON_EXTRACT_STRING_ARRAY('{"fruits": ["apples", "oranges", "grapes"]}', '$[fruits]') AS string_array;

    SELECT JSON_EXTRACT_STRING_ARRAY('{"fruits": ["apples", "oranges", "grapes"]}', '$.fruits') AS string_array;

    -- The queries above produce the following result:
    /*---+
     | string_array              |
     +---+
     | [apples, oranges, grapes] |
     +---*/

In cases where a JSON key uses invalid JSONPath characters, you can escape those
characters using single quotes and brackets: `[' ']`. For example:

    SELECT JSON_EXTRACT_STRING_ARRAY('{"a.b": {"c": ["world"]}}', "$['a.b'].c") AS hello;

    /*---+
     | hello   |
     +---+
     | [world] |
     +---*/

The following examples explore how invalid requests and empty arrays are
handled:

    -- An error is thrown if you provide an invalid JSONPath.
    SELECT JSON_EXTRACT_STRING_ARRAY('["foo", "bar", "baz"]', 'INVALID_JSONPath') AS result;

    -- If the JSON formatted string is invalid, then NULL is returned.
    SELECT JSON_EXTRACT_STRING_ARRAY('}}', '$') AS result;

    /*---+
     | result |
     +---+
     | NULL   |
     +---*/

    -- If the JSON document is NULL, then NULL is returned.
    SELECT JSON_EXTRACT_STRING_ARRAY(NULL, '$') AS result;

    /*---+
     | result |
     +---+
     | NULL   |
     +---*/

    -- If a JSONPath doesn't match anything, then the output is NULL.
    SELECT JSON_EXTRACT_STRING_ARRAY('{"a": ["foo", "bar", "baz"]}', '$.b') AS result;

    /*---+
     | result |
     +---+
     | NULL   |
     +---*/

    -- If a JSONPath matches an object that isn't an array, then the output is NULL.
    SELECT JSON_EXTRACT_STRING_ARRAY('{"a": "foo"}', '$') AS result;

    /*---+
     | result |
     +---+
     | NULL   |
     +---*/

    -- If a JSONPath matches an array of non-scalar objects, then the output is NULL.
    SELECT JSON_EXTRACT_STRING_ARRAY('{"a": [{"b": "foo", "c": 1}, {"b": "bar", "c":2}], "d": "baz"}', '$.a') AS result;

    /*---+
     | result |
     +---+
     | NULL   |
     +---*/

    -- If a JSONPath matches an array of mixed scalar and non-scalar objects, then the output is NULL.
    SELECT JSON_EXTRACT_STRING_ARRAY('{"a": [10, {"b": 20}]', '$.a') AS result;

    /*---+
     | result |
     +---+
     | NULL   |
     +---*/

    -- If a JSONPath matches an empty JSON array, then the output is an empty array instead of NULL.
    SELECT JSON_EXTRACT_STRING_ARRAY('{"a": "foo", "b": []}', '$.b') AS result;

    /*---+
     | result |
     +---+
     | []     |
     +---*/

    -- The following query produces and error because the final output can't be an
    -- array with NULLs.
    SELECT JSON_EXTRACT_STRING_ARRAY('["world", 1, null]') AS result;

## `JSON_FLATTEN`

> [!WARNING]
> **Preview**
>
>
> This product or feature is subject to the "Pre-GA Offerings Terms"
> in the General Service Terms section of the
> [Service Specific Terms](https://cloud.google.com/terms/service-terms).
> Pre-GA products and features are available "as is" and might have
> limited support. For more information, see the
> [launch stage descriptions](https://cloud.google.com/products#product-launch-stages).

> [!NOTE]
> **Note:** To provide feedback or request support for this feature, send an email to [bigquery-sql-preview-support@googlegroups.com](mailto:bigquery-sql-preview-support@googlegroups.com).

    JSON_FLATTEN(json_expr)

**Description**

Produces a new SQL `ARRAY<JSON>` value containing all non-array values that are
either directly in the input JSON value or children of one or more consecutively
nested arrays in the input JSON value.

Arguments:

- `json_expr`: `JSON`. For example:

      JSON '["Jane", ["John", "Jamie"]]'

Details:

- If `json_expr` is SQL `NULL`, the function returns SQL `NULL`.

**Return type**

`ARRAY<JSON>`

**Examples**

In the following example, there is a single non-array value that is returned.

    SELECT JSON_FLATTEN(JSON '1') AS json_flatten

    /*---+
     | json_flatten |
     +---+
     | [1]          |
     +---*/

In the following example, an input array of values is flattened.

    SELECT JSON_FLATTEN(JSON '[1, 2, null]') AS json_flatten

    /*---+
     | json_flatten |
     +---+
     | [1, 2, null] |
     +---*/

In the following example, an input array which includes nested array elements is
flattened.

    SELECT JSON_FLATTEN(JSON '[[[1]], 2, [3]]') AS json_flatten

    /*---+
     | json_flatten |
     +---+
     | [1, 2, 3]    |
     +---*/

In the following example, the nested-array value in a key-value pair is not
flattened because it is enclosed within a JSON object.

    SELECT JSON_FLATTEN(JSON '{"a": [[1]]}') AS json_flatten

    /*---+
     | json_flatten  |
     +---+
     | [{"a":[[1]]}] |
     +---*/

In the following example, the output contains both the flattened array elements
from the input and the non-array elements from the input.

    SELECT JSON_FLATTEN(JSON '[[[1, 2], 3], {"a": 4}, true]') AS json_flatten

    /*---+
     | json_flatten              |
     +---+
     | [1, 2, 3, {"a": 4}, true] |
     +---*/

## `JSON_KEYS`

    JSON_KEYS(
      json_expr
      [, max_depth ]
      [, mode => { 'strict' | 'lax' | 'lax recursive' } ]
    )

**Description**

Extracts unique JSON keys from a JSON expression.

Arguments:

- `json_expr`: `JSON`. For example:

      JSON '{"class": {"students": [{"name": "Jane"}]}}'

- `max_depth`: An `INT64` value that represents the maximum depth of nested
  fields to search in `json_expr`. If not
  set, the function searches the entire JSON document.

- `mode`: A named argument with a `STRING` value that can be one of the
  following:

  - `strict` (default): Ignore any key that appears in an array.
  - `lax`: Also include keys contained in non-consecutively nested arrays.
  - `lax recursive`: Return all keys.

Details:

- Keys are de-duplicated and returned in alphabetical order.
- Keys don't include array indices.
- Keys containing special characters are escaped using double quotes.
- Keys are case sensitive and not normalized.
- If `json_expr` or `mode` is SQL `NULL`, the function returns SQL `NULL`.
- If `max_depth` is SQL `NULL`, the function ignores the argument.
- If `max_depth` is less than or equal to 0, then an error is returned.

**Return type**

`ARRAY<STRING>`

**Examples**

In the following example, there are no arrays, so all keys are returned.

    SELECT JSON_KEYS(JSON '{"a": {"b":1}}') AS json_keys

    /*---+
     | json_keys |
     +---+
     | [a, a.b]  |
     +---*/

In the following example, `max_depth` is set to 1 so "a.b" isn't included.

    SELECT JSON_KEYS(JSON '{"a": {"b":1}}', 1) AS json_keys

    /*---+
     | json_keys |
     +---+
     | [a]       |
     +---*/

In the following example, the `json_expr` argument contains an array. Because
the mode is `strict`, keys inside the array are excluded.

    SELECT JSON_KEYS(JSON '{"a":[{"b":1}, {"c":2}], "d":3}') AS json_keys

    /*---+
     | json_keys |
     +---+
     | [a, d]    |
     +---*/

In the following example, the `json_expr` argument contains an array. Because
the mode is `lax`, keys inside the array are included.

    SELECT JSON_KEYS(
      JSON '{"a":[{"b":1}, {"c":2}], "d":3}',
      mode => "lax") as json_keys

    /*---+
     | json_keys        |
     +---+
     | [a, a.b, a.c, d] |
     +---*/

In the following example, the `json_expr` argument contains consecutively nested
arrays. Because the mode is `lax`, keys inside the consecutively nested arrays
aren't included.

    SELECT JSON_KEYS(JSON '{"a":[[{"b":1}]]}', mode => "lax") as json_keys

    /*---+
     | json_keys |
     +---+
     | [a]       |
     +---*/

In the following example, the `json_expr` argument contains consecutively nested
arrays. Because the mode is `lax recursive`, every key is returned.

    SELECT JSON_KEYS(JSON '{"a":[[{"b":1}]]}', mode => "lax recursive") as json_keys

    /*---+
     | json_keys |
     +---+
     | [a, a.b]  |
     +---*/

In the following example, the `json_expr` argument contains multiple arrays.
Because the arrays aren't consecutively nested and the mode is `lax`, keys
inside the arrays are included.

    SELECT JSON_KEYS(JSON '{"a":[{"b":[{"c":1}]}]}', mode => "lax") as json_keys

    /*---+
     | json_keys       |
     +---+
     | [a, a.b, a.b.c] |
     +---*/

In the following example, the `json_expr` argument contains both consecutively
nested and single arrays. Because the mode is `lax`, keys inside the
consecutively nested arrays are excluded.

    SELECT JSON_KEYS(JSON '{"a":[{"b":[[{"c":1}]]}]}', mode => "lax") as json_keys

    /*---+
     | json_keys |
     +---+
     | [a, a.b]  |
     +---*/

In the following example, the `json_expr` argument contains both consecutively
nested and single arrays. Because the mode is `lax recursive`, all keys are
included.

    SELECT JSON_KEYS(
      JSON '{"a":[{"b":[[{"c":1}]]}]}', mode => "lax recursive") as json_keys

    /*---+
     | json_keys       |
     +---+
     | [a, a.b, a.b.c] |
     +---*/

## `JSON_OBJECT`

- [Signature 1](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#json_object_signature1): `JSON_OBJECT([json_key, json_value][, ...])`
- [Signature 2](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#json_object_signature2): `JSON_OBJECT(json_key_array, json_value_array)`

#### Signature 1

    JSON_OBJECT([json_key, json_value][, ...])

**Description**

Creates a JSON object, using key-value pairs.

Arguments:

- `json_key`: A `STRING` value that represents a key.
- `json_value`: A [JSON encoding-supported](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#json_encodings) value.

Details:

- If two keys are passed in with the same name, only the first key-value pair is preserved.
- The order of key-value pairs isn't preserved.
- If `json_key` is `NULL`, an error is produced.

**Return type**

`JSON`

**Examples**

You can create an empty JSON object by passing in no JSON keys and values.
For example:

    SELECT JSON_OBJECT() AS json_data

    /*---+
     | json_data |
     +---+
     | {}        |
     +---*/

You can create a JSON object by passing in key-value pairs. For example:

    SELECT JSON_OBJECT('foo', 10, 'bar', TRUE) AS json_data

    /*---+
     | json_data             |
     +---+
     | {"bar":true,"foo":10} |
     +---*/

    SELECT JSON_OBJECT('foo', 10, 'bar', ['a', 'b']) AS json_data

    /*---+
     | json_data                  |
     +---+
     | {"bar":["a","b"],"foo":10} |
     +---*/

    SELECT JSON_OBJECT('a', NULL, 'b', JSON 'null') AS json_data

    /*---+
     | json_data           |
     +---+
     | {"a":null,"b":null} |
     +---*/

    SELECT JSON_OBJECT('a', 10, 'a', 'foo') AS json_data

    /*---+
     | json_data |
     +---+
     | {"a":10}  |
     +---*/

    WITH Items AS (SELECT 'hello' AS key, 'world' AS value)
    SELECT JSON_OBJECT(key, value) AS json_data FROM Items

    /*---+
     | json_data         |
     +---+
     | {"hello":"world"} |
     +---*/

An error is produced if a SQL `NULL` is passed in for a JSON key.

    -- Error: A key can't be NULL.
    SELECT JSON_OBJECT(NULL, 1) AS json_data

An error is produced if the number of JSON keys and JSON values don't match:

    -- Error: No matching signature for function JSON_OBJECT for argument types:
    -- STRING, INT64, STRING
    SELECT JSON_OBJECT('a', 1, 'b') AS json_data

#### Signature 2

    JSON_OBJECT(json_key_array, json_value_array)

Creates a JSON object, using an array of keys and values.

Arguments:

- `json_key_array`: An array of zero or more `STRING` keys.
- `json_value_array`: An array of zero or more [JSON encoding-supported](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#json_encodings) values.

Details:

- If two keys are passed in with the same name, only the first key-value pair is preserved.
- The order of key-value pairs isn't preserved.
- The number of keys must match the number of values, otherwise an error is produced.
- If any argument is `NULL`, an error is produced.
- If a key in `json_key_array` is `NULL`, an error is produced.

**Return type**

`JSON`

**Examples**

You can create an empty JSON object by passing in an empty array of
keys and values. For example:

    SELECT JSON_OBJECT(CAST([] AS ARRAY<STRING>), []) AS json_data

    /*---+
     | json_data |
     +---+
     | {}        |
     +---*/

You can create a JSON object by passing in an array of keys and an array of
values. For example:

    SELECT JSON_OBJECT(['a', 'b'], [10, NULL]) AS json_data

    /*---+
     | json_data         |
     +---+
     | {"a":10,"b":null} |
     +---*/

    SELECT JSON_OBJECT(['a', 'b'], [JSON '10', JSON '"foo"']) AS json_data

    /*---+
     | json_data          |
     +---+
     | {"a":10,"b":"foo"} |
     +---*/

    SELECT
      JSON_OBJECT(
        ['a', 'b'],
        [STRUCT(10 AS id, 'Red' AS color), STRUCT(20 AS id, 'Blue' AS color)])
        AS json_data

    /*---+
     | json_data                                                  |
     +---+
     | {"a":{"color":"Red","id":10},"b":{"color":"Blue","id":20}} |
     +---*/

    SELECT
      JSON_OBJECT(
        ['a', 'b'],
        [TO_JSON(10), TO_JSON(['foo', 'bar'])])
        AS json_data

    /*---+
     | json_data                  |
     +---+
     | {"a":10,"b":["foo","bar"]} |
     +---*/

The following query groups by `id` and then creates an array of keys and
values from the rows with the same `id`:

    WITH
      Fruits AS (
        SELECT 0 AS id, 'color' AS json_key, 'red' AS json_value UNION ALL
        SELECT 0, 'fruit', 'apple' UNION ALL
        SELECT 1, 'fruit', 'banana' UNION ALL
        SELECT 1, 'ripe', 'true'
      )
    SELECT JSON_OBJECT(ARRAY_AGG(json_key), ARRAY_AGG(json_value)) AS json_data
    FROM Fruits
    GROUP BY id

    /*---+
     | json_data                        |
     +---+
     | {"color":"red","fruit":"apple"}  |
     | {"fruit":"banana","ripe":"true"} |
     +---*/

An error is produced if the size of the JSON keys and values arrays don't
match:

    -- Error: The number of keys and values must match.
    SELECT JSON_OBJECT(['a', 'b'], [10]) AS json_data

An error is produced if the array of JSON keys or JSON values is a SQL `NULL`.

    -- Error: The keys array can't be NULL.
    SELECT JSON_OBJECT(CAST(NULL AS ARRAY<STRING>), [10, 20]) AS json_data

    -- Error: The values array can't be NULL.
    SELECT JSON_OBJECT(['a', 'b'], CAST(NULL AS ARRAY<INT64>)) AS json_data

## `JSON_QUERY`

    JSON_QUERY(json_string_expr, json_path)

    JSON_QUERY(json_expr, json_path)

**Description**

Extracts a JSON value and converts it to a SQL
JSON-formatted `STRING` or
`JSON` value.
This function uses double quotes to escape invalid
[JSONPath](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#JSONPath_format) characters in JSON keys. For example: `"a.b"`.

Arguments:

- `json_string_expr`: A JSON-formatted string. For example:

      '{"class": {"students": [{"name": "Jane"}]}}'

  Extracts a SQL `NULL` when a JSON-formatted string `null` is encountered.
  For example:

      SELECT JSON_QUERY("null", "$") -- Returns a SQL NULL

- `json_expr`: JSON. For example:

      JSON '{"class": {"students": [{"name": "Jane"}]}}'

  Extracts a JSON `null` when a JSON `null` is encountered.

      SELECT JSON_QUERY(JSON 'null', "$") -- Returns a JSON 'null'

- `json_path`: The [JSONPath](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#JSONPath_format). This identifies the data that
  you want to obtain from the input. This function
  lets you [specify a mode](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#JSONPath_mode) for the JSONPath.

There are differences between the JSON-formatted string and JSON input types.
For details, see [Differences between the JSON and JSON-formatted STRING types](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#differences_json_and_string).

**Return type**

- `json_string_expr`: A JSON-formatted `STRING`
- `json_expr`: `JSON`

**Examples**

In the following example, JSON data is extracted and returned as JSON.

    SELECT
      JSON_QUERY(
        JSON '{"class": {"students": [{"id": 5}, {"id": 12}]}}',
        '$.class') AS json_data;

    /*---+
     | json_data                         |
     +---+
     | {"students":[{"id":5},{"id":12}]} |
     +---*/

In the following examples, JSON data is extracted and returned as
JSON-formatted strings.

    SELECT
      JSON_QUERY('{"class": {"students": [{"name": "Jane"}]}}', '$') AS json_text_string;

    /*---+
     | json_text_string                                          |
     +---+
     | {"class":{"students":[{"name":"Jane"}]}}                  |
     +---*/

    SELECT JSON_QUERY('{"class": {"students": []}}', '$') AS json_text_string;

    /*---+
     | json_text_string                                          |
     +---+
     | {"class":{"students":[]}}                                 |
     +---*/

    SELECT
      JSON_QUERY(
        '{"class": {"students": [{"name": "John"},{"name": "Jamie"}]}}',
        '$') AS json_text_string;

    /*---+
     | json_text_string                                          |
     +---+
     | {"class":{"students":[{"name":"John"},{"name":"Jamie"}]}} |
     +---*/

    SELECT
      JSON_QUERY(
        '{"class": {"students": [{"name": "Jane"}]}}',
        '$.class.students[0]') AS first_student;

    /*---+
     | first_student   |
     +---+
     | {"name":"Jane"} |
     +---*/

    SELECT
      JSON_QUERY('{"class": {"students": []}}', '$.class.students[0]') AS first_student;

    /*---+
     | first_student   |
     +---+
     | NULL            |
     +---*/

    SELECT
      JSON_QUERY(
        '{"class": {"students": [{"name": "John"}, {"name": "Jamie"}]}}',
        '$.class.students[0]') AS first_student;

    /*---+
     | first_student   |
     +---+
     | {"name":"John"} |
     +---*/

    SELECT
      JSON_QUERY(
        '{"class": {"students": [{"name": "Jane"}]}}',
        '$.class.students[1].name') AS second_student;

    /*---+
     | second_student |
     +---+
     | NULL           |
     +---*/

    SELECT
      JSON_QUERY(
        '{"class": {"students": []}}',
        '$.class.students[1].name') AS second_student;

    /*---+
     | second_student |
     +---+
     | NULL           |
     +---*/

    SELECT
      JSON_QUERY(
        '{"class": {"students": [{"name": "John"}, {"name": null}]}}',
        '$.class.students[1].name') AS second_student;

    /*---+
     | second_student |
     +---+
     | NULL           |
     +---*/

    SELECT
      JSON_QUERY(
        '{"class": {"students": [{"name": "John"}, {"name": "Jamie"}]}}',
        '$.class.students[1].name') AS second_student;

    /*---+
     | second_student |
     +---+
     | "Jamie"        |
     +---*/

    SELECT
      JSON_QUERY(
        '{"class": {"students": [{"name": "Jane"}]}}',
        '$.class."students"') AS student_names;

    /*---+
     | student_names                      |
     +---+
     | [{"name":"Jane"}]                  |
     +---*/

    SELECT
      JSON_QUERY(
        '{"class": {"students": []}}',
        '$.class."students"') AS student_names;

    /*---+
     | student_names                      |
     +---+
     | []                                 |
     +---*/

    SELECT
      JSON_QUERY(
        '{"class": {"students": [{"name": "John"}, {"name": "Jamie"}]}}',
        '$.class."students"') AS student_names;

    /*---+
     | student_names                      |
     +---+
     | [{"name":"John"},{"name":"Jamie"}] |
     +---*/

In the following examples, the JSON data is extracted in [lax mode](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#JSONPath_mode).
Because the keyword `lax` is included in the `JSONPath`, JSON arrays are
automatically unwrapped.

    SELECT
      JSON_QUERY(
        JSON '{"class": {"students": [{"name": "Jane"}]}}',
        'lax $.class.students.name') AS student_names_lax;

    /*---+
     | student_names_lax |
     +---+
     | ["Jane"]          |
     +---*/

    SELECT
      JSON_QUERY(
        JSON '[{"class": {"students": [{"name": "Joe"}, {"name": "Jamie"}]}}]',
        'lax $.class.students.name') AS student_names_lax;

    /*---+
     | student_names_lax |
     +---+
     | ["Joe","Jamie"]   |
     +---*/

    SELECT
      JSON_QUERY(
        JSON '{"class": {"students": [[{"name": "John"}], {"name": "Jamie"}]}}',
        'lax $.class.students.name') AS student_names_lax;

    /*---+
     | student_names_lax |
     +---+
     | ["Jamie"]         |
     +---*/

In the following examples, the JSON data is extracted in [lax recursive mode](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#JSONPath_mode).
Because the keyword `lax recursive` is included in the `JSONPath`, JSON arrays
are unwrapped until a non-array type is found.

    SELECT
      JSON_QUERY(
        JSON '{"class": {"students": [{"name": "Jane"}]}}',
        'lax recursive $.class.students.name') AS student_names_lax_recursive;

    /*---+
     | student_names_lax_recursive |
     +---+
     | ["Jane"]                    |
     +---*/

    SELECT
      JSON_QUERY(
        JSON '[[{"class": {"students": [{"name": "Joe"}, {"name": "Jamie"}]}}]]',
        'lax recursive $.class.students.name') AS student_names_lax_recursive;

    /*---+
     | student_names_lax_recursive |
     +---+
     | ["Joe","Jamie"]             |
     +---*/

    SELECT
      JSON_QUERY(
        JSON '{"class": {"students": [[{"name": "John"}], {"name": "Jamie"}]}}',
        'lax recursive $.class.students.name') AS student_names_lax_recursive;

    /*---+
     | student_names_lax_recursive |
     +---+
     | ["John","Jamie"]            |
     +---*/

In the following examples, the keywords `lax` and `lax recursive` indicate that
non-array types should be wrapped into arrays of size 1 before matching. The
modes `lax` and `lax recursive` behave identically for wrapping arrays.

    SELECT
      JSON_QUERY(
        JSON '{"class": {"students": {"name": "Jane"}}}',
        'lax $.class[0].students[0].name') AS student_names_lax,
      JSON_QUERY(
        JSON '{"class": {"students": {"name": "Jane"}}}',
        'lax recursive $.class[0].students[0].name') AS student_names_lax_recursive;

    /*---*---+
     | student_names_lax | student_names_lax_recursive |
     +---*---+
     | ["Jane"]          | ["Jane"]                    |
     +---*---*/

    SELECT
      JSON_QUERY(
        JSON '[{"class": {"students": [{"name": "Joe"}, {"name": "Jamie"}]}}]',
        'lax $.class[0].students[0].name') AS student_names_lax,
      JSON_QUERY(
        JSON '[{"class": {"students": [{"name": "Joe"}, {"name": "Jamie"}]}}]',
        'lax recursive $.class[0].students[0].name') AS student_names_lax_recursive;

    /*---*---+
     | student_names_lax | student_names_lax_recursive |
     +---*---+
     | ["Joe"]           | ["Joe"]                     |
     +---*---*/

    SELECT
      JSON_QUERY(
        JSON '{"class": {"students": [[{"name": "John"}], {"name": "Jamie"}]}}',
        'lax $.class[0].students[0].name') AS student_names_lax,
      JSON_QUERY(
        JSON '{"class": {"students": [[{"name": "John"}], {"name": "Jamie"}]}}',
        'lax recursive $.class[0].students[0].name') AS student_names_lax_recursive;

    /*---*---+
     | student_names_lax | student_names_lax_recursive |
     +---*---+
     | ["John"]          | ["John"]                    |
     +---*---*/

    SELECT JSON_QUERY('{"a": null}', "$.a"); -- Returns a SQL NULL
    SELECT JSON_QUERY('{"a": null}', "$.b"); -- Returns a SQL NULL

    SELECT JSON_QUERY(JSON '{"a": null}', "$.a"); -- Returns a JSON 'null'
    SELECT JSON_QUERY(JSON '{"a": null}', "$.b"); -- Returns a SQL NULL

## `JSON_QUERY_ARRAY`

    JSON_QUERY_ARRAY(json_string_expr[, json_path])

    JSON_QUERY_ARRAY(json_expr[, json_path])

**Description**

Extracts a JSON array and converts it to
a SQL `ARRAY<JSON-formatted STRING>` or
`ARRAY<JSON>` value.
In addition, this function uses double quotes to escape invalid
[JSONPath](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#JSONPath_format) characters in JSON keys. For example: `"a.b"`.

Arguments:

- `json_string_expr`: A JSON-formatted string. For example:

      '["a", "b", {"key": "c"}]'

- `json_expr`: JSON. For example:

      JSON '["a", "b", {"key": "c"}]'

- `json_path`: The [JSONPath](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#JSONPath_format). This identifies the data that
  you want to obtain from the input. If this optional parameter isn't
  provided, then the JSONPath `$` symbol is applied, which means that all of
  the data is analyzed.

There are differences between the JSON-formatted string and JSON input types.
For details, see [Differences between the JSON and JSON-formatted STRING types](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#differences_json_and_string).

**Return type**

- `json_string_expr`: `ARRAY<JSON-formatted STRING>`
- `json_expr`: `ARRAY<JSON>`

**Examples**

This extracts items in JSON to an array of `JSON` values:

    SELECT JSON_QUERY_ARRAY(
      JSON '{"fruits": ["apples", "oranges", "grapes"]}', '$.fruits'
      ) AS json_array;

    /*---+
     | json_array                      |
     +---+
     | ["apples", "oranges", "grapes"] |
     +---*/

This extracts the items in a JSON-formatted string to a string array:

    SELECT JSON_QUERY_ARRAY('[1, 2, 3]') AS string_array;

    /*---+
     | string_array |
     +---+
     | [1, 2, 3]    |
     +---*/

This extracts a string array and converts it to an integer array:

    SELECT ARRAY(
      SELECT CAST(integer_element AS INT64)
      FROM UNNEST(
        JSON_QUERY_ARRAY('[1, 2, 3]','$')
      ) AS integer_element
    ) AS integer_array;

    /*---+
     | integer_array |
     +---+
     | [1, 2, 3]     |
     +---*/

This extracts string values in a JSON-formatted string to an array:

    -- Doesn't strip the double quotes
    SELECT JSON_QUERY_ARRAY('["apples", "oranges", "grapes"]', '$') AS string_array;

    /*---+
     | string_array                    |
     +---+
     | ["apples", "oranges", "grapes"] |
     +---*/

    -- Strips the double quotes
    SELECT ARRAY(
      SELECT JSON_VALUE(string_element, '$')
      FROM UNNEST(JSON_QUERY_ARRAY('["apples", "oranges", "grapes"]', '$')) AS string_element
    ) AS string_array;

    /*---+
     | string_array              |
     +---+
     | [apples, oranges, grapes] |
     +---*/

This extracts only the items in the `fruit` property to an array:

    SELECT JSON_QUERY_ARRAY(
      '{"fruit": [{"apples": 5, "oranges": 10}, {"apples": 2, "oranges": 4}], "vegetables": [{"lettuce": 7, "kale": 8}]}',
      '$.fruit'
    ) AS string_array;

    /*---+
     | string_array                                          |
     +---+
     | [{"apples":5,"oranges":10}, {"apples":2,"oranges":4}] |
     +---*/

These are equivalent:

    SELECT JSON_QUERY_ARRAY('{"fruits": ["apples", "oranges", "grapes"]}', '$.fruits') AS string_array;

    SELECT JSON_QUERY_ARRAY('{"fruits": ["apples", "oranges", "grapes"]}', '$."fruits"') AS string_array;

    -- The queries above produce the following result:
    /*---+
     | string_array                    |
     +---+
     | ["apples", "oranges", "grapes"] |
     +---*/

In cases where a JSON key uses invalid JSONPath characters, you can escape those
characters using double quotes: `" "`. For example:

    SELECT JSON_QUERY_ARRAY('{"a.b": {"c": ["world"]}}', '$."a.b".c') AS hello;

    /*---+
     | hello     |
     +---+
     | ["world"] |
     +---*/

The following examples show how invalid requests and empty arrays are handled:

    -- An error is returned if you provide an invalid JSONPath.
    SELECT JSON_QUERY_ARRAY('["foo", "bar", "baz"]', 'INVALID_JSONPath') AS result;

    -- If the JSONPath doesn't refer to an array, then NULL is returned.
    SELECT JSON_QUERY_ARRAY('{"a": "foo"}', '$.a') AS result;

    /*---+
     | result |
     +---+
     | NULL   |
     +---*/

    -- If a key that doesn't exist is specified, then the result is NULL.
    SELECT JSON_QUERY_ARRAY('{"a": "foo"}', '$.b') AS result;

    /*---+
     | result |
     +---+
     | NULL   |
     +---*/

    -- Empty arrays in JSON-formatted strings are supported.
    SELECT JSON_QUERY_ARRAY('{"a": "foo", "b": []}', '$.b') AS result;

    /*---+
     | result |
     +---+
     | []     |
     +---*/

## `JSON_REMOVE`

    JSON_REMOVE(json_expr, json_path[, ...])

Produces a new SQL `JSON` value with the specified JSON data removed.

Arguments:

- `json_expr`: JSON. For example:

      JSON '{"class": {"students": [{"name": "Jane"}]}}'

- `json_path`: Remove data at this [JSONPath](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#JSONPath_format) in `json_expr`.

Details:

- Paths are evaluated left to right. The JSON produced by evaluating the first path is the JSON for the next path.
- The operation ignores non-existent paths and continue processing the rest of the paths.
- For each path, the entire matched JSON subtree is deleted.
- If the path matches a JSON object key, this function deletes the key-value pair.
- If the path matches an array element, this function deletes the specific element from the matched array.
- If removing the path results in an empty JSON object or empty JSON array, the empty structure is preserved.
- If `json_path` is `$` or an invalid [JSONPath](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#JSONPath_format), an error is produced.
- If `json_path` is SQL `NULL`, the path operation is ignored.

**Return type**

`JSON`

**Examples**

In the following example, the path `$[1]` is matched and removes
`["b", "c"]`.

    SELECT JSON_REMOVE(JSON '["a", ["b", "c"], "d"]', '$[1]') AS json_data

    /*---+
     | json_data |
     +---+
     | ["a","d"] |
     +---*/

You can use the field access operator to pass JSON data into this function.
For example:

    WITH T AS (SELECT JSON '{"a": {"b": 10, "c": 20}}' AS data)
    SELECT JSON_REMOVE(data.a, '$.b') AS json_data FROM T

    /*---+
     | json_data |
     +---+
     | {"c":20}  |
     +---*/

In the following example, the first path `$[1]` is matched and removes
`["b", "c"]`. Then, the second path `$[1]` is matched and removes `"d"`.

    SELECT JSON_REMOVE(JSON '["a", ["b", "c"], "d"]', '$[1]', '$[1]') AS json_data

    /*---+
     | json_data |
     +---+
     | ["a"]     |
     +---*/

The structure of an empty array is preserved when all elements are deleted
from it. For example:

    SELECT JSON_REMOVE(JSON '["a", ["b", "c"], "d"]', '$[1]', '$[1]', '$[0]') AS json_data

    /*---+
     | json_data |
     +---+
     | []        |
     +---*/

In the following example, the path `$.a.b.c` is matched and removes the
`"c":"d"` key-value pair from the JSON object.

    SELECT JSON_REMOVE(JSON '{"a": {"b": {"c": "d"}}}', '$.a.b.c') AS json_data

    /*---+
     | json_data      |
     +---+
     | {"a":{"b":{}}} |
     +---*/

In the following example, the path `$.a.b` is matched and removes the
`"b": {"c":"d"}` key-value pair from the JSON object.

    SELECT JSON_REMOVE(JSON '{"a": {"b": {"c": "d"}}}', '$.a.b') AS json_data

    /*---+
     | json_data |
     +---+
     | {"a":{}}  |
     +---*/

In the following example, the path `$.b` isn't valid, so the operation makes
no changes.

    SELECT JSON_REMOVE(JSON '{"a": 1}', '$.b') AS json_data

    /*---+
     | json_data |
     +---+
     | {"a":1}   |
     +---*/

In the following example, path `$.a.b` and `$.b` don't exist, so those
operations are ignored, but the others are processed.

    SELECT JSON_REMOVE(JSON '{"a": [1, 2, 3]}', '$.a[0]', '$.a.b', '$.b', '$.a[0]') AS json_data

    /*---+
     | json_data |
     +---+
     | {"a":[3]} |
     +---*/

If you pass in `$` as the path, an error is produced. For example:

    -- Error: The JSONPath can't be '$'
    SELECT JSON_REMOVE(JSON '{}', '$') AS json_data

In the following example, the operation is ignored because you can't remove
data from a JSON null.

    SELECT JSON_REMOVE(JSON 'null', '$.a.b') AS json_data

    /*---+
     | json_data |
     +---+
     | null      |
     +---*/

## `JSON_SET`

    JSON_SET(
      json_expr,
      json_path_value_pair[, ...]
      [, create_if_missing => { TRUE | FALSE } ]
    )

    json_path_value_pair:
      json_path, value

Produces a new SQL `JSON` value with the specified JSON data inserted
or replaced.

Arguments:

- `json_expr`: JSON. For example:

      JSON '{"class": {"students": [{"name": "Jane"}]}}'

- `json_path_value_pair`: A value and the [JSONPath](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#JSONPath_format) for
  that value. This includes:

  - `json_path`: Insert or replace `value` at this [JSONPath](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#JSONPath_format)
    in `json_expr`.

  - `value`: A [JSON encoding-supported](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#json_encodings) value to
    insert.

- `create_if_missing`: A named argument that takes a `BOOL` value.

  - If `TRUE` (default), replaces or inserts data if the path doesn't exist.

  - If `FALSE`, only existing JSONPath values are replaced. If the path
    doesn't exist, the set operation is ignored.

Details:

- Path value pairs are evaluated left to right. The JSON produced by evaluating one pair becomes the JSON against which the next pair is evaluated.
- If a matched path has an existing value, it overwrites the existing data with `value`.
- If `create_if_missing` is `TRUE`:

  - If a path doesn't exist, the remainder of the path is recursively created.
  - If the matched path prefix points to a JSON null, the remainder of the path is recursively created, and `value` is inserted.
  - If a path token points to a JSON array and the specified index is *larger* than the size of the array, pads the JSON array with JSON nulls, recursively creates the remainder of the path at the specified index, and inserts the path value pair.
- This function applies all path value pair set operations even if an
  individual path value pair operation is invalid. For invalid operations,
  the operation is ignored and the function continues to process the rest
  of the path value pairs.

- If the path exists but has an incompatible type at any given path
  token, no update happens for that specific path value pair.

- If any `json_path` is an invalid [JSONPath](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#JSONPath_format), an error is
  produced.

- If `json_expr` is SQL `NULL`, the function returns SQL `NULL`.

- If `json_path` is SQL `NULL`, the `json_path_value_pair` operation is
  ignored.

- If `create_if_missing` is SQL `NULL`, the set operation is ignored.

**Return type**

`JSON`

**Examples**

In the following example, the path `$` matches the entire `JSON` value
and replaces it with `{"b": 2, "c": 3}`.

    SELECT JSON_SET(JSON '{"a": 1}', '$', JSON '{"b": 2, "c": 3}') AS json_data

    /*---+
     | json_data     |
     +---+
     | {"b":2,"c":3} |
     +---*/

In the following example, `create_if_missing` is `FALSE` and the path `$.b`
doesn't exist, so the set operation is ignored.

    SELECT JSON_SET(
      JSON '{"a": 1}',
      "$.b", 999,
      create_if_missing => false) AS json_data

    /*---+
     | json_data  |
     +---+
     | '{"a": 1}' |
     +---*/

In the following example, `create_if_missing` is `TRUE` and the path `$.a`
exists, so the value is replaced.

    SELECT JSON_SET(
      JSON '{"a": 1}',
      "$.a", 999,
      create_if_missing => false) AS json_data

    /*---+
     | json_data    |
     +---+
     | '{"a": 999}' |
     +---*/

In the following example, the path `$.a` is matched, but `$.a.b` doesn't
exist, so the new path and the value are inserted.

    SELECT JSON_SET(JSON '{"a": {}}', '$.a.b', 100) AS json_data

    /*---+
     | json_data       |
     +---+
     | {"a":{"b":100}} |
     +---*/

In the following example, the path prefix `$` points to a JSON null, so the
remainder of the path is created for the value `100`.

    SELECT JSON_SET(JSON 'null', '$.a.b', 100) AS json_data

    /*---+
     | json_data       |
     +---+
     | {"a":{"b":100}} |
     +---*/

In the following example, the path `$.a.c` implies that the value at `$.a` is
a JSON object but it's not. This part of the operation is ignored, but the other
parts of the operation are completed successfully.

    SELECT JSON_SET(
      JSON '{"a": 1}',
      '$.b', 2,
      '$.a.c', 100,
      '$.d', 3) AS json_data

    /*---+
     | json_data           |
     +---+
     | {"a":1,"b":2,"d":3} |
     +---*/

In the following example, the path `$.a[2]` implies that the value for `$.a` is
an array, but it's not, so the operation is ignored for that value.

    SELECT JSON_SET(
      JSON '{"a": 1}',
      '$.a[2]', 100,
      '$.b', 2) AS json_data

    /*---+
     | json_data     |
     +---+
     | {"a":1,"b":2} |
     +---*/

In the following example, the path `$[1]` is matched and replaces the
array element value with `foo`.

    SELECT JSON_SET(JSON '["a", ["b", "c"], "d"]', '$[1]', "foo") AS json_data

    /*---+
     | json_data       |
     +---+
     | ["a","foo","d"] |
     +---*/

In the following example, the path `$[1][0]` is matched and replaces the
array element value with `foo`.

    SELECT JSON_SET(JSON '["a", ["b", "c"], "d"]', '$[1][0]', "foo") AS json_data

    /*---+
     | json_data             |
     +---+
     | ["a",["foo","c"],"d"] |
     +---*/

In the following example, the path prefix `$` points to a JSON null, so the
remainder of the path is created. The resulting array is padded with
JSON nulls and appended with `foo`.

    SELECT JSON_SET(JSON 'null', '$[0][3]', "foo")

    /*---+
     | json_data                |
     +---+
     | [[null,null,null,"foo"]] |
     +---*/

In the following example, the path `$[1]` is matched, the matched array is
extended since `$[1][4]` is larger than the existing array, and then `foo` is
inserted in the array.

    SELECT JSON_SET(JSON '["a", ["b", "c"], "d"]', '$[1][4]', "foo") AS json_data

    /*---+
     | json_data                           |
     +---+
     | ["a",["b","c",null,null,"foo"],"d"] |
     +---*/

In the following example, the path `$[1][0][0]` implies that the value of
`$[1][0]` is an array, but it isn't, so the operation is ignored.

    SELECT JSON_SET(JSON '["a", ["b", "c"], "d"]', '$[1][0][0]', "foo") AS json_data

    /*---+
     | json_data           |
     +---+
     | ["a",["b","c"],"d"] |
     +---*/

In the following example, the path `$[1][2]` is larger than the length of
the matched array. The array length is extended and the remainder of the path
is recursively created. The operation continues to the path `$[1][2][1]`
and inserts `foo`.

    SELECT JSON_SET(JSON '["a", ["b", "c"], "d"]', '$[1][2][1]', "foo") AS json_data

    /*---+
     | json_data                        |
     +---+
     | ["a",["b","c",[null,"foo"]],"d"] |
     +---*/

In the following example, because the `JSON` object is empty, key `b` is
inserted, and the remainder of the path is recursively created.

    SELECT JSON_SET(JSON '{}', '$.b[2].d', 100) AS json_data

    /*---+
     | json_data                   |
     +---+
     | {"b":[null,null,{"d":100}]} |
     +---*/

In the following example, multiple values are set.

    SELECT JSON_SET(
      JSON '{"a": 1, "b": {"c":3}, "d": [4]}',
      '$.a', 'v1',
      '$.b.e', 'v2',
      '$.d[2]', 'v3') AS json_data

    /*---+
     | json_data                                         |
     +---+
     | {"a":"v1","b":{"c":3,"e":"v2"},"d":[4,null,"v3"]} |
     +---*/

## `JSON_STRIP_NULLS`

    JSON_STRIP_NULLS(
      json_expr
      [, json_path ]
      [, include_arrays => { TRUE | FALSE } ]
      [, remove_empty => { TRUE | FALSE } ]
    )

Recursively removes JSON nulls from JSON objects and JSON arrays.

Arguments:

- `json_expr`: JSON. For example:

      JSON '{"a": null, "b": "c"}'

- `json_path`: Remove JSON nulls at this [JSONPath](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#JSONPath_format) for
  `json_expr`.

- `include_arrays`: A named argument that's either
  `TRUE` (default) or `FALSE`. If `TRUE` or omitted, the function removes
  JSON nulls from JSON arrays. If `FALSE`, doesn't.

- `remove_empty`: A named argument that's either
  `TRUE` or `FALSE` (default). If `TRUE`, the function removes empty
  JSON objects after JSON nulls are removed. If `FALSE` or omitted, doesn't.

  If `remove_empty` is `TRUE` and `include_arrays` is `TRUE` or omitted,
  the function additionally removes empty JSON arrays.

Details:

- If a value is a JSON null, the associated key-value pair is removed.
- If `remove_empty` is set to `TRUE`, the function recursively removes empty containers after JSON nulls are removed.
- If the function generates JSON with nothing in it, the function returns a JSON null.
- If `json_path` is an invalid [JSONPath](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#JSONPath_format), an error is produced.
- If `json_expr` is SQL `NULL`, the function returns SQL `NULL`.
- If `json_path`, `include_arrays`, or `remove_empty` is SQL `NULL`, the function returns `json_expr`.

**Return type**

`JSON`

**Examples**

In the following example, all JSON nulls are removed.

    SELECT JSON_STRIP_NULLS(JSON '{"a": null, "b": "c"}') AS json_data

    /*---+
     | json_data |
     +---+
     | {"b":"c"} |
     +---*/

In the following example, all JSON nulls are removed from a JSON array.

    SELECT JSON_STRIP_NULLS(JSON '[1, null, 2, null]') AS json_data

    /*---+
     | json_data |
     +---+
     | [1,2]     |
     +---*/

In the following example, `include_arrays` is set as `FALSE` so that JSON nulls
aren't removed from JSON arrays.

    SELECT JSON_STRIP_NULLS(JSON '[1, null, 2, null]', include_arrays=>FALSE) AS json_data

    /*---+
     | json_data       |
     +---+
     | [1,null,2,null] |
     +---*/

In the following example, `remove_empty` is omitted and defaults to
`FALSE`, and the empty structures are retained.

    SELECT JSON_STRIP_NULLS(JSON '[1, null, 2, null, [null]]') AS json_data

    /*---+
     | json_data |
     +---+
     | [1,2,[]]  |
     +---*/

In the following example, `remove_empty` is set as `TRUE`, and the
empty structures are removed.

    SELECT JSON_STRIP_NULLS(
      JSON '[1, null, 2, null, [null]]',
      remove_empty=>TRUE) AS json_data

    /*---+
     | json_data |
     +---+
     | [1,2]     |
     +---*/

In the following examples, `remove_empty` is set as `TRUE`, and the
empty structures are removed. Because no JSON data is left the function
returns JSON null.

    SELECT JSON_STRIP_NULLS(JSON '{"a": null}', remove_empty=>TRUE) AS json_data

    /*---+
     | json_data |
     +---+
     | null      |
     +---*/

    SELECT JSON_STRIP_NULLS(JSON '{"a": [null]}', remove_empty=>TRUE) AS json_data

    /*---+
     | json_data |
     +---+
     | null      |
     +---*/

In the following example, empty structures are removed for JSON objects,
but not JSON arrays.

    SELECT JSON_STRIP_NULLS(
      JSON '{"a": {"b": {"c": null}}, "d": [null], "e": [], "f": 1}',
      include_arrays=>FALSE,
      remove_empty=>TRUE) AS json_data

    /*---+
     | json_data                 |
     +---+
     | {"d":[null],"e":[],"f":1} |
     +---*/

In the following example, empty structures are removed for both JSON objects,
and JSON arrays.

    SELECT JSON_STRIP_NULLS(
      JSON '{"a": {"b": {"c": null}}, "d": [null], "e": [], "f": 1}',
      remove_empty=>TRUE) AS json_data

    /*---+
     | json_data |
     +---+
     | {"f":1}   |
     +---*/

In the following example, because no JSON data is left, the function returns a
JSON null.

    SELECT JSON_STRIP_NULLS(JSON 'null') AS json_data

    /*---+
     | json_data |
     +---+
     | null      |
     +---*/

## `JSON_TYPE`

    JSON_TYPE(json_expr)

**Description**

Gets the JSON type of the outermost JSON value and converts the name of
this type to a SQL `STRING` value. The names of these JSON types can be
returned: `object`, `array`, `string`, `number`, `boolean`, `null`

Arguments:

- `json_expr`: JSON. For example:

      JSON '{"name": "sky", "color": "blue"}'

  If this expression is SQL `NULL`, the function returns SQL `NULL`. If the
  extracted JSON value isn't a valid JSON type, an error is produced.

**Return type**

`STRING`

**Examples**

    SELECT json_val, JSON_TYPE(json_val) AS type
    FROM
      UNNEST(
        [
          JSON '"apple"',
          JSON '10',
          JSON '3.14',
          JSON 'null',
          JSON '{"city": "New York", "State": "NY"}',
          JSON '["apple", "banana"]',
          JSON 'false'
        ]
      ) AS json_val;

    /*---+---+
     | json_val                         | type    |
     +---+---+
     | "apple"                          | string  |
     | 10                               | number  |
     | 3.14                             | number  |
     | null                             | null    |
     | {"State":"NY","city":"New York"} | object  |
     | ["apple","banana"]               | array   |
     | false                            | boolean |
     +---+---*/

## `JSON_VALUE`

    JSON_VALUE(json_string_expr[, json_path])

    JSON_VALUE(json_expr[, json_path])

**Description**

Extracts a JSON scalar value and converts it to a SQL `STRING` value.
In addition, this function:

- Removes the outermost quotes and unescapes the values.
- Returns a SQL `NULL` if a non-scalar value is selected.
- Uses double quotes to escape invalid [JSONPath](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#JSONPath_format) characters in JSON keys. For example: `"a.b"`.

Arguments:

- `json_string_expr`: A JSON-formatted string. For example:

      '{"name": "Jakob", "age": "6"}'

- `json_expr`: JSON. For example:

      JSON '{"name": "Jane", "age": "6"}'

- `json_path`: The [JSONPath](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#JSONPath_format). This identifies the data that
  you want to obtain from the input. If this optional parameter isn't
  provided, then the JSONPath `$` symbol is applied, which means that all of
  the data is analyzed.

  If `json_path` returns a JSON `null` or a non-scalar value (in other words,
  if `json_path` refers to an object or an array), then a SQL `NULL` is
  returned.

There are differences between the JSON-formatted string and JSON input types.
For details, see [Differences between the JSON and JSON-formatted STRING types](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#differences_json_and_string).

**Return type**

`STRING`

**Examples**

In the following example, JSON data is extracted and returned as a scalar value.

    SELECT JSON_VALUE(JSON '{"name": "Jakob", "age": "6" }', '$.age') AS scalar_age;

    /*---+
     | scalar_age |
     +---+
     | 6          |
     +---*/

The following example compares how results are returned for the `JSON_QUERY`
and `JSON_VALUE` functions.

    SELECT JSON_QUERY('{"name": "Jakob", "age": "6"}', '$.name') AS json_name,
      JSON_VALUE('{"name": "Jakob", "age": "6"}', '$.name') AS scalar_name,
      JSON_QUERY('{"name": "Jakob", "age": "6"}', '$.age') AS json_age,
      JSON_VALUE('{"name": "Jakob", "age": "6"}', '$.age') AS scalar_age;

    /*---+---+---+---+
     | json_name | scalar_name | json_age | scalar_age |
     +---+---+---+---+
     | "Jakob"   | Jakob       | "6"      | 6          |
     +---+---+---+---*/

    SELECT JSON_QUERY('{"fruits": ["apple", "banana"]}', '$.fruits') AS json_query,
      JSON_VALUE('{"fruits": ["apple", "banana"]}', '$.fruits') AS json_value;

    /*---+---+
     | json_query         | json_value |
     +---+---+
     | ["apple","banana"] | NULL       |
     +---+---*/

In cases where a JSON key uses invalid JSONPath characters, you can escape those
characters using double quotes. For example:

    SELECT JSON_VALUE('{"a.b": {"c": "world"}}', '$."a.b".c') AS hello;

    /*---+
     | hello |
     +---+
     | world |
     +---*/

## `JSON_VALUE_ARRAY`

    JSON_VALUE_ARRAY(json_string_expr[, json_path])

    JSON_VALUE_ARRAY(json_expr[, json_path])

**Description**

Extracts a JSON array of scalar values and converts it to a SQL
`ARRAY<STRING>` value.
In addition, this function:

- Removes the outermost quotes and unescapes the values.
- Returns a SQL `NULL` if the selected value isn't an array or not an array containing only scalar values.
- Uses double quotes to escape invalid [JSONPath](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#JSONPath_format) characters in JSON keys. For example: `"a.b"`.

Arguments:

- `json_string_expr`: A JSON-formatted string. For example:

      '["apples", "oranges", "grapes"]'

- `json_expr`: JSON. For example:

      JSON '["apples", "oranges", "grapes"]'

- `json_path`: The [JSONPath](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#JSONPath_format). This identifies the data that
  you want to obtain from the input. If this optional parameter isn't
  provided, then the JSONPath `$` symbol is applied, which means that all of
  the data is analyzed.

There are differences between the JSON-formatted string and JSON input types.
For details, see [Differences between the JSON and JSON-formatted STRING types](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#differences_json_and_string).

Caveats:

- A JSON `null` in the input array produces a SQL `NULL` as the output for JSON `null`. If the output contains a `NULL` array element, an error is produced because the final output can't be an array with `NULL` values.
- If a JSONPath matches an array that contains scalar objects and a JSON `null`, then the output of the function must be transformed because the final output can't be an array with `NULL` values.

**Return type**

`ARRAY<STRING>`

**Examples**

This extracts items in JSON to a string array:

    SELECT JSON_VALUE_ARRAY(
      JSON '{"fruits": ["apples", "oranges", "grapes"]}', '$.fruits'
      ) AS string_array;

    /*---+
     | string_array              |
     +---+
     | [apples, oranges, grapes] |
     +---*/

The following example compares how results are returned for the
`JSON_QUERY_ARRAY` and `JSON_VALUE_ARRAY` functions.

    SELECT JSON_QUERY_ARRAY('["apples", "oranges"]') AS json_array,
           JSON_VALUE_ARRAY('["apples", "oranges"]') AS string_array;

    /*---+---+
     | json_array            | string_array      |
     +---+---+
     | ["apples", "oranges"] | [apples, oranges] |
     +---+---*/

This extracts the items in a JSON-formatted string to a string array:

    -- Strips the double quotes
    SELECT JSON_VALUE_ARRAY('["foo", "bar", "baz"]', '$') AS string_array;

    /*---+
     | string_array    |
     +---+
     | [foo, bar, baz] |
     +---*/

This extracts a string array and converts it to an integer array:

    SELECT ARRAY(
      SELECT CAST(integer_element AS INT64)
      FROM UNNEST(
        JSON_VALUE_ARRAY('[1, 2, 3]', '$')
      ) AS integer_element
    ) AS integer_array;

    /*---+
     | integer_array |
     +---+
     | [1, 2, 3]     |
     +---*/

These are equivalent:

    SELECT JSON_VALUE_ARRAY('{"fruits": ["apples", "oranges", "grapes"]}', '$.fruits') AS string_array;
    SELECT JSON_VALUE_ARRAY('{"fruits": ["apples", "oranges", "grapes"]}', '$."fruits"') AS string_array;

    -- The queries above produce the following result:
    /*---+
     | string_array              |
     +---+
     | [apples, oranges, grapes] |
     +---*/

In cases where a JSON key uses invalid JSONPath characters, you can escape those
characters using double quotes: `" "`. For example:

    SELECT JSON_VALUE_ARRAY('{"a.b": {"c": ["world"]}}', '$."a.b".c') AS hello;

    /*---+
     | hello   |
     +---+
     | [world] |
     +---*/

The following examples explore how invalid requests and empty arrays are
handled:

    -- An error is thrown if you provide an invalid JSONPath.
    SELECT JSON_VALUE_ARRAY('["foo", "bar", "baz"]', 'INVALID_JSONPath') AS result;

    -- If the JSON-formatted string is invalid, then NULL is returned.
    SELECT JSON_VALUE_ARRAY('}}', '$') AS result;

    /*---+
     | result |
     +---+
     | NULL   |
     +---*/

    -- If the JSON document is NULL, then NULL is returned.
    SELECT JSON_VALUE_ARRAY(NULL, '$') AS result;

    /*---+
     | result |
     +---+
     | NULL   |
     +---*/

    -- If a JSONPath doesn't match anything, then the output is NULL.
    SELECT JSON_VALUE_ARRAY('{"a": ["foo", "bar", "baz"]}', '$.b') AS result;

    /*---+
     | result |
     +---+
     | NULL   |
     +---*/

    -- If a JSONPath matches an object that isn't an array, then the output is NULL.
    SELECT JSON_VALUE_ARRAY('{"a": "foo"}', '$') AS result;

    /*---+
     | result |
     +---+
     | NULL   |
     +---*/

    -- If a JSONPath matches an array of non-scalar objects, then the output is NULL.
    SELECT JSON_VALUE_ARRAY('{"a": [{"b": "foo", "c": 1}, {"b": "bar", "c": 2}], "d": "baz"}', '$.a') AS result;

    /*---+
     | result |
     +---+
     | NULL   |
     +---*/

    -- If a JSONPath matches an array of mixed scalar and non-scalar objects,
    -- then the output is NULL.
    SELECT JSON_VALUE_ARRAY('{"a": [10, {"b": 20}]', '$.a') AS result;

    /*---+
     | result |
     +---+
     | NULL   |
     +---*/

    -- If a JSONPath matches an empty JSON array, then the output is an empty array instead of NULL.
    SELECT JSON_VALUE_ARRAY('{"a": "foo", "b": []}', '$.b') AS result;

    /*---+
     | result |
     +---+
     | []     |
     +---*/

    -- The following query produces and error because the final output can't be an
    -- array with NULLs.
    SELECT JSON_VALUE_ARRAY('["world", 1, null]') AS result;

## `LAX_BOOL`

    LAX_BOOL(json_expr)

**Description**

Attempts to convert a JSON value to a SQL `BOOL` value.

Arguments:

- `json_expr`: JSON. For example:

      JSON 'true'

Details:

- If `json_expr` is SQL `NULL`, the function returns SQL `NULL`.
- See the conversion rules in the next section for additional `NULL` handling.

**Conversion rules**

| From JSON type | To SQL `BOOL` |
|---|---|
| boolean | If the JSON boolean is `true`, returns `TRUE`. Otherwise, returns `FALSE`. |
| string | If the JSON string is `'true'`, returns `TRUE`. If the JSON string is `'false'`, returns `FALSE`. If the JSON string is any other value or has whitespace in it, returns `NULL`. This conversion is case-insensitive. |
| number | If the JSON number is a representation of `0`, returns `FALSE`. Otherwise, returns `TRUE`. |
| other type or null | `NULL` |

**Return type**

`BOOL`

**Examples**

Example with input that's a JSON boolean:

    SELECT LAX_BOOL(JSON 'true') AS result;

    /*---+
     | result |
     +---+
     | true   |
     +---*/

Examples with inputs that are JSON strings:

    SELECT LAX_BOOL(JSON '"true"') AS result;

    /*---+
     | result |
     +---+
     | TRUE   |
     +---*/

    SELECT LAX_BOOL(JSON '"true "') AS result;

    /*---+
     | result |
     +---+
     | NULL   |
     +---*/

    SELECT LAX_BOOL(JSON '"foo"') AS result;

    /*---+
     | result |
     +---+
     | NULL   |
     +---*/

Examples with inputs that are JSON numbers:

    SELECT LAX_BOOL(JSON '10') AS result;

    /*---+
     | result |
     +---+
     | TRUE   |
     +---*/

    SELECT LAX_BOOL(JSON '0') AS result;

    /*---+
     | result |
     +---+
     | FALSE  |
     +---*/

    SELECT LAX_BOOL(JSON '0.0') AS result;

    /*---+
     | result |
     +---+
     | FALSE  |
     +---*/

    SELECT LAX_BOOL(JSON '-1.1') AS result;

    /*---+
     | result |
     +---+
     | TRUE   |
     +---*/

## `LAX_FLOAT64`

    LAX_FLOAT64(json_expr)

**Description**

Attempts to convert a JSON value to a
SQL `FLOAT64` value.

Arguments:

- `json_expr`: JSON. For example:

      JSON '9.8'

Details:

- If `json_expr` is SQL `NULL`, the function returns SQL `NULL`.
- See the conversion rules in the next section for additional `NULL` handling.

**Conversion rules**

| From JSON type | To SQL `FLOAT64` |
|---|---|
| boolean | `NULL` |
| string | If the JSON string represents a JSON number, parses it as a `BIGNUMERIC` value, and then safe casts the result as a `FLOAT64` value. If the JSON string can't be converted, returns `NULL`. |
| number | Casts the JSON number as a `FLOAT64` value. Large JSON numbers are rounded. |
| other type or null | `NULL` |

**Return type**

`FLOAT64`

**Examples**

Examples with inputs that are JSON numbers:

    SELECT LAX_FLOAT64(JSON '9.8') AS result;

    /*---+
     | result |
     +---+
     | 9.8    |
     +---*/

    SELECT LAX_FLOAT64(JSON '9') AS result;

    /*---+
     | result |
     +---+
     | 9.0    |
     +---*/

    SELECT LAX_FLOAT64(JSON '9007199254740993') AS result;

    /*---+
     | result             |
     +---+
     | 9007199254740992.0 |
     +---*/

    SELECT LAX_FLOAT64(JSON '1e100') AS result;

    /*---+
     | result |
     +---+
     | 1e+100 |
     +---*/

Examples with inputs that are JSON booleans:

    SELECT LAX_FLOAT64(JSON 'true') AS result;

    /*---+
     | result |
     +---+
     | NULL   |
     +---*/

    SELECT LAX_FLOAT64(JSON 'false') AS result;

    /*---+
     | result |
     +---+
     | NULL   |
     +---*/

Examples with inputs that are JSON strings:

    SELECT LAX_FLOAT64(JSON '"10"') AS result;

    /*---+
     | result |
     +---+
     | 10.0   |
     +---*/

    SELECT LAX_FLOAT64(JSON '"1.1"') AS result;

    /*---+
     | result |
     +---+
     | 1.1    |
     +---*/

    SELECT LAX_FLOAT64(JSON '"1.1e2"') AS result;

    /*---+
     | result |
     +---+
     | 110.0  |
     +---*/

    SELECT LAX_FLOAT64(JSON '"9007199254740993"') AS result;

    /*---+
     | result             |
     +---+
     | 9007199254740992.0 |
     +---*/

    SELECT LAX_FLOAT64(JSON '"+1.5"') AS result;

    /*---+
     | result |
     +---+
     | 1.5    |
     +---*/

    SELECT LAX_FLOAT64(JSON '"NaN"') AS result;

    /*---+
     | result |
     +---+
     | NaN    |
     +---*/

    SELECT LAX_FLOAT64(JSON '"Inf"') AS result;

    /*---+
     | result   |
     +---+
     | Infinity |
     +---*/

    SELECT LAX_FLOAT64(JSON '"-InfiNiTY"') AS result;

    /*---+
     | result    |
     +---+
     | -Infinity |
     +---*/

    SELECT LAX_FLOAT64(JSON '"foo"') AS result;

    /*---+
     | result |
     +---+
     | NULL   |
     +---*/

## `LAX_INT64`

    LAX_INT64(json_expr)

**Description**

Attempts to convert a JSON value to a SQL `INT64` value.

Arguments:

- `json_expr`: JSON. For example:

      JSON '999'

Details:

- If `json_expr` is SQL `NULL`, the function returns SQL `NULL`.
- See the conversion rules in the next section for additional `NULL` handling.

**Conversion rules**

| From JSON type | To SQL `INT64` |
|---|---|
| boolean | If the JSON boolean is `true`, returns `1`. If `false`, returns `0`. |
| string | If the JSON string represents a JSON number, parses it as a `BIGNUMERIC` value, and then safe casts the results as an `INT64` value. If the JSON string can't be converted, returns `NULL`. |
| number | Casts the JSON number as an `INT64` value. If the JSON number can't be converted, returns `NULL`. |
| other type or null | `NULL` |

**Return type**

`INT64`

**Examples**

Examples with inputs that are JSON numbers:

    SELECT LAX_INT64(JSON '10') AS result;

    /*---+
     | result |
     +---+
     | 10     |
     +---*/

    SELECT LAX_INT64(JSON '10.0') AS result;

    /*---+
     | result |
     +---+
     | 10     |
     +---*/

    SELECT LAX_INT64(JSON '1.1') AS result;

    /*---+
     | result |
     +---+
     | 1      |
     +---*/

    SELECT LAX_INT64(JSON '3.5') AS result;

    /*---+
     | result |
     +---+
     | 4      |
     +---*/

    SELECT LAX_INT64(JSON '1.1e2') AS result;

    /*---+
     | result |
     +---+
     | 110    |
     +---*/

    SELECT LAX_INT64(JSON '1e100') AS result;

    /*---+
     | result |
     +---+
     | NULL   |
     +---*/

Examples with inputs that are JSON booleans:

    SELECT LAX_INT64(JSON 'true') AS result;

    /*---+
     | result |
     +---+
     | 1      |
     +---*/

    SELECT LAX_INT64(JSON 'false') AS result;

    /*---+
     | result |
     +---+
     | 0      |
     +---*/

Examples with inputs that are JSON strings:

    SELECT LAX_INT64(JSON '"10"') AS result;

    /*---+
     | result |
     +---+
     | 10     |
     +---*/

    SELECT LAX_INT64(JSON '"1.1"') AS result;

    /*---+
     | result |
     +---+
     | 1      |
     +---*/

    SELECT LAX_INT64(JSON '"1.1e2"') AS result;

    /*---+
     | result |
     +---+
     | 110    |
     +---*/

    SELECT LAX_INT64(JSON '"+1.5"') AS result;

    /*---+
     | result |
     +---+
     | 2      |
     +---*/

    SELECT LAX_INT64(JSON '"1e100"') AS result;

    /*---+
     | result |
     +---+
     | NULL   |
     +---*/

    SELECT LAX_INT64(JSON '"foo"') AS result;

    /*---+
     | result |
     +---+
     | NULL   |
     +---*/

## `LAX_STRING`

    LAX_STRING(json_expr)

**Description**

Attempts to convert a JSON value to a SQL `STRING` value.

Arguments:

- `json_expr`: JSON. For example:

      JSON '"name"'

Details:

- If `json_expr` is SQL `NULL`, the function returns SQL `NULL`.
- See the conversion rules in the next section for additional `NULL` handling.

**Conversion rules**

| From JSON type | To SQL `STRING` |
|---|---|
| boolean | If the JSON boolean is `true`, returns `'true'`. If `false`, returns `'false'`. |
| string | Returns the JSON string as a `STRING` value. |
| number | Returns the JSON number as a `STRING` value. |
| other type or null | `NULL` |

**Return type**

`STRING`

**Examples**

Examples with inputs that are JSON strings:

    SELECT LAX_STRING(JSON '"purple"') AS result;

    /*---+
     | result |
     +---+
     | purple |
     +---*/

    SELECT LAX_STRING(JSON '"10"') AS result;

    /*---+
     | result |
     +---+
     | 10     |
     +---*/

Examples with inputs that are JSON booleans:

    SELECT LAX_STRING(JSON 'true') AS result;

    /*---+
     | result |
     +---+
     | true   |
     +---*/

    SELECT LAX_STRING(JSON 'false') AS result;

    /*---+
     | result |
     +---+
     | false  |
     +---*/

Examples with inputs that are JSON numbers:

    SELECT LAX_STRING(JSON '10.0') AS result;

    /*---+
     | result |
     +---+
     | 10     |
     +---*/

    SELECT LAX_STRING(JSON '10') AS result;

    /*---+
     | result |
     +---+
     | 10     |
     +---*/

    SELECT LAX_STRING(JSON '1e100') AS result;

    /*---+
     | result |
     +---+
     | 1e+100 |
     +---*/

## `PARSE_JSON`

    PARSE_JSON(
      json_string_expr
      [, wide_number_mode => { 'exact' | 'round' } ]
    )

**Description**

Converts a JSON-formatted `STRING` value to a [`JSON` value](https://www.json.org/json-en.html).

Arguments:

- `json_string_expr`: A JSON-formatted string. For example:

      '{"class": {"students": [{"name": "Jane"}]}}'

- `wide_number_mode`: A named argument with a `STRING` value. Determines
  how to handle numbers that can't be stored in a `JSON` value without the
  loss of precision. If used, `wide_number_mode` must include one of the
  following values:

  - `exact` (default): Only accept numbers that can be stored without loss of precision. If a number that can't be stored without loss of precision is encountered, the function throws an error.
  - `round`: If a number that can't be stored without loss of precision is encountered, attempt to round it to a number that can be stored without loss of precision. If the number can't be rounded, the function throws an error.

  If a number appears in a JSON object or array, the `wide_number_mode`
  argument is applied to the number in the object or array.

Numbers from the following domains can be stored in JSON without loss of
precision:

- 64-bit signed/unsigned integers, such as `INT64`
- `FLOAT64`

**Return type**

`JSON`

**Examples**

In the following example, a JSON-formatted string is converted to `JSON`.

    SELECT PARSE_JSON('{"coordinates": [10, 20], "id": 1}') AS json_data;

    /*---+
     | json_data                      |
     +---+
     | {"coordinates":[10,20],"id":1} |
     +---*/

The following queries fail because:

- The number that was passed in can't be stored without loss of precision.
- `wide_number_mode=>'exact'` is used implicitly in the first query and explicitly in the second query.

    SELECT PARSE_JSON('{"id": 922337203685477580701}') AS json_data; -- fails
    SELECT PARSE_JSON('{"id": 922337203685477580701}', wide_number_mode=>'exact') AS json_data; -- fails

The following query rounds the number to a number that can be stored in JSON.

    SELECT PARSE_JSON('{"id": 922337203685477580701}', wide_number_mode=>'round') AS json_data;

    /*---+
     | json_data                    |
     +---+
     | {"id":9.223372036854776e+20} |
     +---*/

You can also use valid JSON-formatted strings that don't represent name/value pairs. For example:

    SELECT PARSE_JSON('6') AS json_data;

    /*---+
     | json_data                    |
     +---+
     | 6                            |
     +---*/

    SELECT PARSE_JSON('"red"') AS json_data;

    /*---+
     | json_data                    |
     +---+
     | "red"                        |
     +---*/

## `STRING`

    STRING(json_expr)

**Description**

Converts a JSON string to a SQL `STRING` value.

Arguments:

- `json_expr`: JSON. For example:

      JSON '"purple"'

  If the JSON value isn't a string, an error is produced. If the expression
  is SQL `NULL`, the function returns SQL `NULL`.

**Return type**

`STRING`

**Examples**

    SELECT STRING(JSON '"purple"') AS color;

    /*---+
     | color  |
     +---+
     | purple |
     +---*/

    SELECT STRING(JSON_QUERY(JSON '{"name": "sky", "color": "blue"}', "$.color")) AS color;

    /*---+
     | color |
     +---+
     | blue  |
     +---*/

The following examples show how invalid requests are handled:

    -- An error is thrown if the JSON isn't of type string.
    SELECT STRING(JSON '123') AS result; -- Throws an error
    SELECT STRING(JSON 'null') AS result; -- Throws an error
    SELECT SAFE.STRING(JSON '123') AS result; -- Returns a SQL NULL

## `TO_JSON`

    TO_JSON(
      sql_value
      [, stringify_wide_numbers => { TRUE | FALSE } ]
    )

**Description**

Converts a SQL value to a JSON value.

Arguments:

- `sql_value`: The SQL value to convert to a JSON value. You can review the GoogleSQL data types that this function supports and their JSON encodings [here](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#json_encodings).
- `stringify_wide_numbers`: A named argument that's either
  `TRUE` or `FALSE` (default).

  - If `TRUE`, numeric values outside of the `FLOAT64` type domain are encoded as strings.
  - If `FALSE` (default), numeric values outside of the `FLOAT64` type domain aren't encoded as strings, but are stored as JSON numbers. If a numerical value can't be stored in JSON without loss of precision, an error is thrown.

  The following numerical data types are affected by the
  `stringify_wide_numbers` argument:
- `INT64`

- `NUMERIC`

- `BIGNUMERIC`

  If one of these numerical data types appears in a container data type
  such as an `ARRAY` or `STRUCT`, the `stringify_wide_numbers` argument is
  applied to the numerical data types in the container data type.

**Return type**

`JSON`

**Examples**

In the following example, the query converts rows in a table to JSON values.

    With CoordinatesTable AS (
        (SELECT 1 AS id, [10, 20] AS coordinates) UNION ALL
        (SELECT 2 AS id, [30, 40] AS coordinates) UNION ALL
        (SELECT 3 AS id, [50, 60] AS coordinates))
    SELECT TO_JSON(t) AS json_objects
    FROM CoordinatesTable AS t;

    /*---+
     | json_objects                   |
     +---+
     | {"coordinates":[10,20],"id":1} |
     | {"coordinates":[30,40],"id":2} |
     | {"coordinates":[50,60],"id":3} |
     +---*/

In the following example, the query returns a large numerical value as a
JSON string.

    SELECT TO_JSON(9007199254740993, stringify_wide_numbers=>TRUE) as stringify_on;

    /*---+
     | stringify_on       |
     +---+
     | "9007199254740993" |
     +---*/

In the following example, both queries return a large numerical value as a
JSON number.

    SELECT TO_JSON(9007199254740993, stringify_wide_numbers=>FALSE) as stringify_off;
    SELECT TO_JSON(9007199254740993) as stringify_off;

    /*---+
     | stringify_off    |
     +---+
     | 9007199254740993 |
     +---*/

In the following example, only large numeric values are converted to
JSON strings.

    With T1 AS (
      (SELECT 9007199254740993 AS id) UNION ALL
      (SELECT 2 AS id))
    SELECT TO_JSON(t, stringify_wide_numbers=>TRUE) AS json_objects
    FROM T1 AS t;

    /*---+
     | json_objects              |
     +---+
     | {"id":"9007199254740993"} |
     | {"id":2}                  |
     +---*/

In this example, the values `9007199254740993` (`INT64`)
and `2.1` (`FLOAT64`) are converted
to the common supertype `FLOAT64`, which isn't
affected by the `stringify_wide_numbers` argument.

    With T1 AS (
      (SELECT 9007199254740993 AS id) UNION ALL
      (SELECT 2.1 AS id))
    SELECT TO_JSON(t, stringify_wide_numbers=>TRUE) AS json_objects
    FROM T1 AS t;

    /*---+
     | json_objects                 |
     +---+
     | {"id":9.007199254740992e+15} |
     | {"id":2.1}                   |
     +---*/

In the following example, a graph path is converted into a JSON array.

    GRAPH graph_db.FinGraph
    MATCH p=(src:Account)-[t1:Transfers]->(dst:Account)
    RETURN TO_JSON(p) AS json_array

    /*---+
     | json_array                                                         |
     +---+
     | [{                                                                 |
     |    "identifier":"mUZpbkdyYXBoLkFjY291bnQAeJEg",                    |
     |    "kind":"node",                                                  |
     |    "labels":["Account"],                                           |
     |    "properties":{                                                  |
     |      "create_time":"2020-01-28T01:55:09.206Z",                     |
     |      "id":16,                                                      |
     |      "is_blocked":true,                                            |
     |      "nick_name":"Vacation Fund"                                   |
     |    }                                                               |
     |  },                                                                |
     |  {                                                                 |
     |    "destination_node_identifier":"mUZpbkdyYXBoLkFjY291bnQAeJEo",   |
     |    "identifier":"mUZpbkdyYXBoLkFjY291...",                         |
     |    "kind":"edge",                                                  |
     |    "labels":["Transfers"],                                         |
     |    "properties":{                                                  |
     |      "amount":300.0,                                               |
     |      "create_time":"2020-09-25T09:36:14.926Z",                     |
     |      "id":16,                                                      |
     |      "order_number":"103650009791820",                             |
     |      "to_id":20                                                    |
     |    },                                                              |
     |    "source_node_identifier":"mUZpbkdyYXBoLkFjY291bnQAeJEg"         |
     |  },                                                                |
     |  {                                                                 |
     |    "identifier":"mUZpbkdyYXBoLkFjY291bnQAeJEo",                    |
     |    "kind":"node",                                                  |
     |    "labels":["Account"],                                           |
     |    "properties":{                                                  |
     |      "create_time":"2020-02-18T13:44:20.655Z",                     |
     |      "id":20,                                                      |
     |      "is_blocked":false,                                           |
     |      "nick_name":"Vacation Fund"                                   |
     |    }                                                               |
     |  }                                                                 |
     |  ...                                                               |
     | ]                                                                  |
     +---/*

In the following example, each graph node called `src` is converted into a
JSON object:

    GRAPH graph_db.FinGraph
    MATCH (src:Account {id: 7})-[t1:Transfers]->(dst:Account)
    RETURN TO_JSON(src) AS json_array

    /*---+
     | json_array                                                         |
     +---+
     | {                                                                  |
     |   "identifier":"rhYAAAANAAAApgAAAAAAAAAApgcAAAAAAAAA",             |
     |   "kind":"node",                                                   |
     |   "labels":["Account"],                                            |
     |   "properties":{                                                   |
     |     "create_time":"2020-01-10T06:22:20.222Z",                      |
     |     "id":7,                                                        |
     |     "is_blocked":false,                                            |
     |     "nick_name":"Vacation Fund"                                    |
     |   }                                                                |
     | }                                                                  |
     | {                                                                  |
     |   "identifier":"rhYAAAANAAAApgAAAAAAAAAApgcAAAAAAAAA",             |
     |   "kind":"node",                                                   |
     |   "labels":["Account"],                                            |
     |   "properties":{                                                   |
     |     "create_time":"2020-01-10T06:22:20.222Z",                      |
     |     "id":7,                                                        |
     |     "is_blocked":false,                                            |
     |     "nick_name":"Vacation Fund"                                    |
     |   }                                                                |
     | }                                                                  |
     +---*/

## `TO_JSON_STRING`

    TO_JSON_STRING(value[, pretty_print])

**Description**

Converts a SQL value to a JSON-formatted `STRING` value.

Arguments:

- `value`: A SQL value. You can review the GoogleSQL data types that this function supports and their JSON encodings in the [JSON encodings](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#json_encodings) section. If the value is `NULL`, a `null` string is returned.
- `pretty_print`: Optional boolean parameter. If `pretty_print` is `true`, the returned value is formatted for easy readability. If `pretty_print` is `NULL`, the function returns `NULL`, regardless of the `value` argument.

**Return type**

A JSON-formatted `STRING`

**Examples**

The following query converts a `STRUCT` value to a JSON-formatted string:

    SELECT TO_JSON_STRING(STRUCT(1 AS id, [10,20] AS coordinates)) AS json_data

    /*---+
     | json_data                      |
     +---+
     | {"id":1,"coordinates":[10,20]} |
     +---*/

The following query converts a `STRUCT` value to a JSON-formatted string that is
easy to read:

    SELECT TO_JSON_STRING(STRUCT(1 AS id, [10,20] AS coordinates), true) AS json_data

    /*---+
     | json_data          |
     +---+
     | {                  |
     |   "id": 1,         |
     |   "coordinates": [ |
     |     10,            |
     |     20             |
     |   ]                |
     | }                  |
     +---*/

## Supplemental materials

### Differences between the JSON and JSON-formatted STRING types

Many JSON functions accept two input types:

- [`JSON`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#json_type) type
- `STRING` type

The `STRING` version of the extraction functions behaves differently than the
`JSON` version, mainly because `JSON` type values are always validated whereas
JSON-formatted `STRING` type values aren't.

#### Non-validation of `STRING` inputs

The following `STRING` is invalid JSON because it's missing a trailing `}`:

    {"hello": "world"

The JSON function reads the input from the beginning and stops as soon as the
field to extract is found, without reading the remainder of the input. A parsing
error isn't produced.

With the `JSON` type, however, `JSON '{"hello": "world"'` returns a parsing
error.

For example:

    SELECT JSON_VALUE('{"hello": "world"', "$.hello") AS hello;

    /*---+
     | hello |
     +---+
     | world |
     +---*/

    SELECT JSON_VALUE(JSON '{"hello": "world"', "$.hello") AS hello;
    -- An error is returned: Invalid JSON literal: syntax error while parsing
    -- object - unexpected end of input; expected '}'

#### No strict validation of extracted values

In the following examples, duplicated keys aren't removed when using a
JSON-formatted string. Similarly, keys order is preserved. For the `JSON`
type, `JSON '{"key": 1, "key": 2}'` will result in `JSON '{"key":1}'` during
parsing.

    SELECT JSON_QUERY('{"key": 1, "key": 2}', "$") AS string;

    /*---+
     | string            |
     +---+
     | {"key":1,"key":2} |
     +---*/

    SELECT JSON_QUERY(JSON '{"key": 1, "key": 2}', "$") AS json;

    /*---+
     | json      |
     +---+
     | {"key":1} |
     +---*/

#### JSON `null`

When using a JSON-formatted `STRING` type in a JSON function, a JSON `null`
value is extracted as a SQL `NULL` value.

When using a JSON type in a JSON function, a JSON `null` value returns a JSON
`null` value.

    WITH t AS (
      SELECT '{"name": null}' AS json_string, JSON '{"name": null}' AS json)
    SELECT JSON_QUERY(json_string, "$.name") AS name_string,
      JSON_QUERY(json_string, "$.name") IS NULL AS name_string_is_null,
      JSON_QUERY(json, "$.name") AS name_json,
      JSON_QUERY(json, "$.name") IS NULL AS name_json_is_null
    FROM t;

    /*---+---+---+---+
     | name_string | name_string_is_null | name_json | name_json_is_null |
     +---+---+---+---+
     | NULL        | true                | null      | false             |
     +---+---+---+---*/

### JSON encodings

You can encode a SQL value as a JSON value with the following functions:

- `TO_JSON_STRING`
- `TO_JSON`
- `JSON_SET` (uses `TO_JSON` encoding)
- `JSON_ARRAY` (uses `TO_JSON` encoding)
- `JSON_ARRAY_APPEND` (uses `TO_JSON` encoding)
- `JSON_ARRAY_INSERT` (uses `TO_JSON` encoding)
- `JSON_OBJECT` (uses `TO_JSON` encoding)

The following SQL to JSON encodings are supported:

| From SQL | To JSON | Examples |
|---|---|---|
| NULL | null | SQL input: `NULL` JSON output: `null` |
| BOOL | boolean | SQL input: `TRUE` JSON output: `true` *** ** * ** *** SQL input: `FALSE` JSON output: `false` |
| INT64 | (TO_JSON_STRING only) number or string Encoded as a number when the value is in the range of \[-2^53^, 2^53^\], which is the range of integers that can be represented losslessly as IEEE 754 double-precision floating point numbers. A value outside of this range is encoded as a string. | SQL input: `9007199254740992` JSON output: `9007199254740992` *** ** * ** *** SQL input: `9007199254740993` JSON output: `"9007199254740993"` |
| INT64 | (TO_JSON only) number or string If the `stringify_wide_numbers` argument is `TRUE` and the value is outside of the FLOAT64 type domain, the value is encoded as a string. If the value can't be stored in JSON without loss of precision, the function fails. Otherwise, the value is encoded as a number. If the `stringify_wide_numbers` isn't used or is `FALSE`, numeric values outside of the \`FLOAT64\` type domain aren't encoded as strings, but are stored as JSON numbers. If a numerical value can't be stored in JSON without loss of precision, an error is thrown. | SQL input: `9007199254740992` JSON output: `9007199254740992` *** ** * ** *** SQL input: `9007199254740993` JSON output: `9007199254740993` *** ** * ** *** SQL input with stringify_wide_numbers=\>TRUE: `9007199254740992` JSON output: `9007199254740992` *** ** * ** *** SQL input with stringify_wide_numbers=\>TRUE: `9007199254740993` JSON output: `"9007199254740993"` |
| INTERVAL | string | SQL input: `INTERVAL '10:20:30.52' HOUR TO SECOND` JSON output: `"PT10H20M30.52S"` *** ** * ** *** SQL input: `INTERVAL 1 SECOND` JSON output: `"PT1S"` *** ** * ** *** `INTERVAL -25 MONTH` JSON output: `"P-2Y-1M"` *** ** * ** *** `INTERVAL '1 5:30' DAY TO MINUTE` JSON output: `"P1DT5H30M"` |
| NUMERIC BIGNUMERIC | (TO_JSON_STRING only) number or string Encoded as a number when the value is in the range of \[-2^53^, 2^53^\] and has no fractional part. A value outside of this range is encoded as a string. | SQL input: `-1` JSON output: `-1` *** ** * ** *** SQL input: `0` JSON output: `0` *** ** * ** *** SQL input: `9007199254740993` JSON output: `"9007199254740993"` *** ** * ** *** SQL input: `123.56` JSON output: `"123.56"` |
| NUMERIC BIGNUMERIC | (TO_JSON only) number or string If the `stringify_wide_numbers` argument is `TRUE` and the value is outside of the FLOAT64 type domain, it's encoded as a string. Otherwise, it's encoded as a number. | SQL input: `-1` JSON output: `-1` *** ** * ** *** SQL input: `0` JSON output: `0` *** ** * ** *** SQL input: `9007199254740993` JSON output: `9007199254740993` *** ** * ** *** SQL input: `123.56` JSON output: `123.56` *** ** * ** *** SQL input with stringify_wide_numbers=\>TRUE: `9007199254740993` JSON output: `"9007199254740993"` *** ** * ** *** SQL input with stringify_wide_numbers=\>TRUE: `123.56` JSON output: `123.56` |
| FLOAT64 | number or string `+/-inf` and `NaN` are encoded as `Infinity`, `-Infinity`, and `NaN`. Otherwise, this value is encoded as a number. | SQL input: `1.0` JSON output: `1` *** ** * ** *** SQL input: `9007199254740993` JSON output: `9007199254740993` *** ** * ** *** SQL input: `"+inf"` JSON output: `"Infinity"` *** ** * ** *** SQL input: `"-inf"` JSON output: `"-Infinity"` *** ** * ** *** SQL input: `"NaN"` JSON output: `"NaN"` |
| STRING | string Encoded as a string, escaped according to the JSON standard. Specifically, `"`, `\,` and the control characters from `U+0000` to `U+001F` are escaped. | SQL input: `"abc"` JSON output: `"abc"` *** ** * ** *** SQL input: `"\"abc\""` JSON output: `"\"abc\""` |
| BYTES | string Uses RFC 4648 Base64 data encoding. | SQL input: `b"Google"` JSON output: `"R29vZ2xl"` |
| DATE | string | SQL input: `DATE '2017-03-06'` JSON output: `"2017-03-06"` |
| TIMESTAMP | string Encoded as ISO 8601 date and time, where T separates the date and time and Z (Zulu/UTC) represents the time zone. | SQL input: `TIMESTAMP '2017-03-06 12:34:56.789012'` JSON output: `"2017-03-06T12:34:56.789012Z"` |
| DATETIME | string Encoded as ISO 8601 date and time, where T separates the date and time. | SQL input: `DATETIME '2017-03-06 12:34:56.789012'` JSON output: `"2017-03-06T12:34:56.789012"` |
| TIME | string Encoded as ISO 8601 time. | SQL input: `TIME '12:34:56.789012'` JSON output: `"12:34:56.789012"` |
| JSON | data of the input JSON | SQL input: `JSON '{"item": "pen", "price": 10}'` JSON output: `{"item":"pen", "price":10}` *** ** * ** *** SQL input:`[1, 2, 3]` JSON output:`[1, 2, 3]` |
| ARRAY | array Can contain zero or more elements. | SQL input: `["red", "blue", "green"]` JSON output: `["red","blue","green"]` *** ** * ** *** SQL input:`[1, 2, 3]` JSON output:`[1,2,3]` |
| STRUCT | object The object can contain zero or more key-value pairs. Each value is formatted according to its type. For `TO_JSON`, a field is included in the output string and any duplicates of this field are omitted. For `TO_JSON_STRING`, a field and any duplicates of this field are included in the output string. Anonymous fields are represented with `""`. Invalid UTF-8 field names might result in unparseable JSON. String values are escaped according to the JSON standard. Specifically, `"`, `\,` and the control characters from `U+0000` to `U+001F` are escaped. | SQL input: `STRUCT(12 AS purchases, TRUE AS inStock)` JSON output: `{"inStock": true,"purchases":12}` |
| GRAPH_ELEMENT | (`TO_JSON` only) object The object can contain zero or more key-value pairs. Each value is formatted according to its type. For `TO_JSON`, graph element (node or edge) objects are supported. - The graph element identifier is only valid within the scope of the same query response and can't be used to correlate entities across different queries. - Field names that aren't valid UTF-8 might result in unparseable JSON. - The result may include internal key-value pairs that aren't defined by the users. - The conversion can fail if the object contains values of unsupported types. | SQL: ``` GRAPH graph_db.FinGraph MATCH (p:Person WHERE p.name = 'Dana') RETURN TO_JSON(p) AS dana_json; ``` <br /> JSON output (truncated): ``` {"identifier":"ZGFuYQ==","kind":"node","labels":["Person"],"properties":{"id":2,"name":"Dana"}} ``` |
| GRAPH_PATH | (`TO_JSON` only) array The array can contain one or more objects that represent graph elements in a graph path. | SQL: ``` GRAPH graph_db.FinGraph MATCH account_ownership = (p:Person)-[o:Owns]->(a:Account) RETURN TO_JSON(account_ownership) AS results ``` <br /> JSON output for `account_ownership` (truncated): ``` [ {"identifier":"ZGFuYQ==","kind":"node","labels":["Person"], ...}, {"identifier":"TPZuYM==","kind":"edge","labels":["Owns"], ...}, {"identifier":"PRTuMI==","kind":"node","labels":["Account"], ...} ] ``` |
| RANGE | range Encoded as an object with a `start` and `end` value. Any unbounded part of the range is represented as `null`. | SQL input: `RANGE<DATE> '[2024-07-24, 2024-07-25)'` JSON output: `{"start":"2024-07-24","end":"2024-07-25"}` *** ** * ** *** SQL input: `RANGE<DATETIME> '[2024-07-24 10:00:00, UNBOUNDED)'` JSON output: `{"start":"2024-07-24T10:00:00","end":null}` |

### JSONPath format

With the JSONPath format, you can identify the values you want to
obtain from a JSON-formatted string.

If a key in a JSON functions contains a JSON format operator, refer to each
JSON function for how to escape them.

A JSON function returns `NULL` if the JSONPath format doesn't match a value in
a JSON-formatted string. If the selected value for a scalar function isn't
scalar, such as an object or an array, the function returns `NULL`. If the
JSONPath format is invalid, an error is produced.

#### Operators for JSONPath

The JSONPath format supports these operators:

| Operator | Description | Examples |
|---|---|---|
| `$` | Root object or element. The JSONPath format must start with this operator, which refers to the outermost level of the JSON-formatted string. | JSON-formatted string: `'{"class" : {"students" : [{"name" : "Jane"}]}}'` JSON path: `"$"` JSON result: `{"class":{"students":[{"name":"Jane"}]}}` |
| `.` | Child operator. You can identify child values using dot-notation. | JSON-formatted string: `'{"class" : {"students" : [{"name" : "Jane"}]}}'` JSON path: `"$.class.students"` JSON result: `[{"name":"Jane"}]` |
| `[]` | Subscript operator. If the object is a JSON array, you can use brackets to specify the array index. | JSON-formatted string: `'{"class" : {"students" : [{"name" : "Jane"}]}}'` JSON path: `"$.class.students[0]"` JSON result: `{"name":"Jane"}` |
| `[][]` `[][][]...` | Child subscript operator. If the object is a JSON array within an array, you can use as many additional brackets as you need to specify the child array index. | JSON-formatted string: `'{"a": [["b", "c"], "d"], "e":"f"}'` JSON path: `"$.a[0][1]"` JSON result: `"c"` |

#### Modes for JSONPath

Some JSON functions that take a JSONPath let you specify a mode that indicates
how the JSONPath matches the JSON data structure. For example, the JSONPath
could be `lax $.class.students`. The following modes are supported:

| Mode | Description | Example JSONPath |
|---|---|---|
| `strict` (default) | The JSONPath must structurally match the JSON data | `"$.class.students"` |
| `lax` | Implicitly adapts the path to the structure of the JSON data. If the JSONPath doesn't exactly match the JSON data, then the following rules apply: - If the JSONPath operator is an object, then it unwraps a single level of array. - If the JSONPath operator is an array and the JSON value isn't, then it wraps JSON values in an array of size 1 . A single value is equivalent to an array of size 1 containing the value. | `"lax $.class.students"` |
| `lax recursive` | In addition to `lax` behavior, JSONPath unwraps consecutive arrays until a non-array type is encountered. | `"lax recursive $.class.students"` |