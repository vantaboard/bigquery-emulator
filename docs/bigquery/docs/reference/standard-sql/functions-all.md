> [!WARNING]
> GoogleSQL is the new name for Google Standard SQL! New name, same great SQL dialect.

This topic contains all functions supported by GoogleSQL for BigQuery.

## Function list

| Name | Summary |
|---|---|
| [`ABS`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#abs) | Computes the absolute value of `X`. |
| [`ACOS`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#acos) | Computes the inverse cosine of `X`. |
| [`ACOSH`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#acosh) | Computes the inverse hyperbolic cosine of `X`. |
| [`AEAD.DECRYPT_BYTES`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aead_encryption_functions#aeaddecrypt_bytes) | Uses the matching key from a keyset to decrypt a `BYTES` ciphertext. |
| [`AEAD.DECRYPT_STRING`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aead_encryption_functions#aeaddecrypt_string) | Uses the matching key from a keyset to decrypt a `BYTES` ciphertext into a `STRING` plaintext. |
| [`AEAD.ENCRYPT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aead_encryption_functions#aeadencrypt) | Encrypts `STRING` plaintext, using the primary cryptographic key in a keyset. |
| [`AGG`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#agg) | Aggregates a measure type. |
| [`ANY_VALUE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#any_value) | Gets an expression for some row. |
| [`APPENDS`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/time-series-functions#appends) | Returns all rows appended to a table for a given time range. |
| [`APPROX_COUNT_DISTINCT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/approximate_aggregate_functions#approx_count_distinct) | Gets the approximate result for `COUNT(DISTINCT expression)`. |
| [`APPROX_QUANTILES`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/approximate_aggregate_functions#approx_quantiles) | Gets the approximate quantile boundaries. |
| [`APPROX_TOP_COUNT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/approximate_aggregate_functions#approx_top_count) | Gets the approximate top elements and their approximate count. |
| [`APPROX_TOP_SUM`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/approximate_aggregate_functions#approx_top_sum) | Gets the approximate top elements and sum, based on the approximate sum of an assigned weight. |
| [`ARRAY`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/array_functions#array) | Produces an array with one element for each row in a subquery. |
| [`ARRAY_AGG`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#array_agg) | Gets an array of values. |
| [`ARRAY_CONCAT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/array_functions#array_concat) | Concatenates one or more arrays with the same element type into a single array. |
| [`ARRAY_CONCAT_AGG`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#array_concat_agg) | Concatenates arrays and returns a single array as a result. |
| [`ARRAY_FIRST`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/array_functions#array_first) | Gets the first element in an array. |
| [`ARRAY_LAST`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/array_functions#array_last) | Gets the last element in an array. |
| [`ARRAY_LENGTH`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/array_functions#array_length) | Gets the number of elements in an array. |
| [`ARRAY_REVERSE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/array_functions#array_reverse) | Reverses the order of elements in an array. |
| [`ARRAY_SLICE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/array_functions#array_slice) | Produces an array containing zero or more consecutive elements from an input array. |
| [`ARRAY_TO_STRING`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/array_functions#array_to_string) | Produces a concatenation of the elements in an array as a `STRING` value. |
| [`ASCII`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#ascii) | Gets the ASCII code for the first character or byte in a `STRING` or `BYTES` value. |
| [`ASIN`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#asin) | Computes the inverse sine of `X`. |
| [`ASINH`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#asinh) | Computes the inverse hyperbolic sine of `X`. |
| [`ATAN`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#atan) | Computes the inverse tangent of `X`. |
| [`ATAN2`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#atan2) | Computes the inverse tangent of `X/Y`, using the signs of `X` and `Y` to determine the quadrant. |
| [`ATANH`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#atanh) | Computes the inverse hyperbolic tangent of `X`. |
| [`AVG`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#avg) | Gets the average of non-`NULL` values. |
| [`AVG` (Differential Privacy)](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate-dp-functions#dp_avg) | `DIFFERENTIAL_PRIVACY`-supported `AVG`. Gets the differentially-private average of non-`NULL`, non-`NaN` values in a query with a `DIFFERENTIAL_PRIVACY` clause. |
| [`BAG_OF_WORDS`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/text-analysis-functions#bag_of_words) | Gets the frequency of each term (token) in a tokenized document. |
| [`BIT_AND`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#bit_and) | Performs a bitwise AND operation on an expression. |
| [`BIT_COUNT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bit_functions#bit_count) | Gets the number of bits that are set in an input expression. |
| [`BIT_OR`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#bit_or) | Performs a bitwise OR operation on an expression. |
| [`BIT_XOR`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#bit_xor) | Performs a bitwise XOR operation on an expression. |
| [`BOOL`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#bool_for_json) | Converts a JSON boolean to a SQL `BOOL` value. |
| [`BYTE_LENGTH`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#byte_length) | Gets the number of `BYTES` in a `STRING` or `BYTES` value. |
| [`CAST`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/conversion_functions#cast) | Convert the results of an expression to the given type. |
| [`CBRT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#cbrt) | Computes the cube root of `X`. |
| [`CEIL`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#ceil) | Gets the smallest integral value that isn't less than `X`. |
| [`CEILING`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#ceiling) | Synonym of `CEIL`. |
| [`CHANGES`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/time-series-functions#changes) | Returns all rows that have changed in a table for a given time range. |
| [`CHAR_LENGTH`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#char_length) | Gets the number of characters in a `STRING` value. |
| [`CHARACTER_LENGTH`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#character_length) | Synonym for `CHAR_LENGTH`. |
| [`CHR`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#chr) | Converts a Unicode code point to a character. |
| [`CODE_POINTS_TO_BYTES`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#code_points_to_bytes) | Converts an array of extended ASCII code points to a `BYTES` value. |
| [`CODE_POINTS_TO_STRING`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#code_points_to_string) | Converts an array of extended ASCII code points to a `STRING` value. |
| [`COLLATE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#collate) | Combines a `STRING` value and a collation specification into a collation specification-supported `STRING` value. |
| [`CONCAT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#concat) | Concatenates one or more `STRING` or `BYTES` values into a single result. |
| [`CONTAINS_SUBSTR`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#contains_substr) | Performs a normalized, case-insensitive search to see if a value exists as a substring in an expression. |
| [`CORR`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#corr) | Computes the Pearson coefficient of correlation of a set of number pairs. |
| [`COS`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#cos) | Computes the cosine of `X`. |
| [`COSH`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#cosh) | Computes the hyperbolic cosine of `X`. |
| [`COSINE_DISTANCE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#cosine_distance) | Computes the cosine distance between two vectors. |
| [`COT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#cot) | Computes the cotangent of `X`. |
| [`COTH`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#coth) | Computes the hyperbolic cotangent of `X`. |
| [`COUNT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#count) | Gets the number of rows in the input, or the number of rows with an expression evaluated to any value other than `NULL`. |
| [`COUNT` (Differential Privacy)](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate-dp-functions#dp_count) | `DIFFERENTIAL_PRIVACY`-supported `COUNT`. Signature 1: Gets the differentially-private count of rows in a query with a `DIFFERENTIAL_PRIVACY` clause. <br /> Signature 2: Gets the differentially-private count of rows with a non-`NULL` expression in a query with a `DIFFERENTIAL_PRIVACY` clause. |
| [`COUNTIF`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#countif) | Gets the number of `TRUE` values for an expression. |
| [`COVAR_POP`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#covar_pop) | Computes the population covariance of a set of number pairs. |
| [`COVAR_SAMP`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#covar_samp) | Computes the sample covariance of a set of number pairs. |
| [`CSC`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#csc) | Computes the cosecant of `X`. |
| [`CSCH`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#csch) | Computes the hyperbolic cosecant of `X`. |
| [`CUME_DIST`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/numbering_functions#cume_dist) | Gets the cumulative distribution (relative position (0,1\]) of each row within a window. |
| [`CURRENT_DATE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#current_date) | Returns the current date as a `DATE` value. |
| [`CURRENT_DATETIME`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/datetime_functions#current_datetime) | Returns the current date and time as a `DATETIME` value. |
| [`CURRENT_TIME`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/time_functions#current_time) | Returns the current time as a `TIME` value. |
| [`CURRENT_TIMESTAMP`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#current_timestamp) | Returns the current date and time as a `TIMESTAMP` object. |
| [`DATE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#date) | Constructs a `DATE` value. |
| [`DATE_ADD`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#date_add) | Adds a specified time interval to a `DATE` value. |
| [`DATE_BUCKET`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/time-series-functions#date_bucket) | Gets the lower bound of the date bucket that contains a date. |
| [`DATE_DIFF`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#date_diff) | Gets the number of unit boundaries between two `DATE` values at a particular time granularity. |
| [`DATE_FROM_UNIX_DATE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#date_from_unix_date) | Interprets an `INT64` expression as the number of days since 1970-01-01. |
| [`DATE_SUB`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#date_sub) | Subtracts a specified time interval from a `DATE` value. |
| [`DATE_TRUNC`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#date_trunc) | Truncates a `DATE`, `DATETIME`, or `TIMESTAMP` value at a particular granularity. |
| [`DATETIME`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/datetime_functions#datetime) | Constructs a `DATETIME` value. |
| [`DATETIME_ADD`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/datetime_functions#datetime_add) | Adds a specified time interval to a `DATETIME` value. |
| [`DATETIME_BUCKET`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/time-series-functions#datetime_bucket) | Gets the lower bound of the datetime bucket that contains a datetime. |
| [`DATETIME_DIFF`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/datetime_functions#datetime_diff) | Gets the number of unit boundaries between two `DATETIME` values at a particular time granularity. |
| [`DATETIME_SUB`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/datetime_functions#datetime_sub) | Subtracts a specified time interval from a `DATETIME` value. |
| [`DATETIME_TRUNC`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/datetime_functions#datetime_trunc) | Truncates a `DATETIME` or `TIMESTAMP` value at a particular granularity. |
| [`DENSE_RANK`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/numbering_functions#dense_rank) | Gets the dense rank (1-based, no gaps) of each row within a window. |
| [`DESTINATION_NODE_ID`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/graph-sql-functions#destination_node_id) | Gets a unique identifier of a graph edge's destination node. |
| [`DETERMINISTIC_DECRYPT_BYTES`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aead_encryption_functions#deterministic_decrypt_bytes) | Uses the matching key from a keyset to decrypt a `BYTES` ciphertext, using deterministic AEAD. |
| [`DETERMINISTIC_DECRYPT_STRING`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aead_encryption_functions#deterministic_decrypt_string) | Uses the matching key from a keyset to decrypt a `BYTES` ciphertext into a `STRING` plaintext, using deterministic AEAD. |
| [`DETERMINISTIC_ENCRYPT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aead_encryption_functions#deterministic_encrypt) | Encrypts `STRING` plaintext, using the primary cryptographic key in a keyset, using deterministic AEAD encryption. |
| [`DIV`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#div) | Divides integer `X` by integer `Y`. |
| [`DLP_DETERMINISTIC_ENCRYPT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/dlp_functions#dlp_deterministic_encrypt) | Encrypts data with a DLP compatible algorithm. |
| [`DLP_DETERMINISTIC_DECRYPT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/dlp_functions#dlp_deterministic_decrypt) | Decrypts DLP-encrypted data. |
| [`DLP_KEY_CHAIN`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/dlp_functions#dlp_key_chain) | Gets a data encryption key that's wrapped by Cloud Key Management Service. |
| [`FLOAT64`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#double_for_json) | Converts a JSON number to a SQL `FLOAT64` value. |
| [`EDGES`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/graph-sql-functions#edges) | Gets the edges in a graph path. The resulting array retains the original order in the graph path. |
| [`EDIT_DISTANCE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#edit_distance) | Computes the Levenshtein distance between two `STRING` or `BYTES` values. |
| [`ELEMENT_ID`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/graph-sql-functions#element_id) | Gets a graph element's unique identifier. |
| [`ENDS_WITH`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#ends_with) | Checks if a `STRING` or `BYTES` value is the suffix of another value. |
| [`ERROR`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/debugging_functions#error) | Produces an error with a custom error message. |
| [`EXP`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#exp) | Computes `e` to the power of `X`. |
| [`EXTERNAL_OBJECT_TRANSFORM`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/table-functions-built-in#external_object_transform) | Produces an object table with the original columns plus one or more additional columns. |
| [`EXTERNAL_QUERY`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/federated_query_functions#external_query) | Executes a query on an external database and returns the results as a temporary table. |
| [`EXTRACT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#extract) | Extracts part of a date from a `DATE` value. |
| [`EXTRACT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/datetime_functions#extract) | Extracts part of a date and time from a `DATETIME` value. |
| [`EXTRACT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/interval_functions#extract) | Extracts part of an `INTERVAL` value. |
| [`EXTRACT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/time_functions#extract) | Extracts part of a `TIME` value. |
| [`EXTRACT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#extract) | Extracts part of a `TIMESTAMP` value. |
| [`EUCLIDEAN_DISTANCE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#euclidean_distance) | Computes the Euclidean distance between two vectors. |
| [`FARM_FINGERPRINT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/hash_functions#farm_fingerprint) | Computes the fingerprint of a `STRING` or `BYTES` value, using the FarmHash Fingerprint64 algorithm. |
| [`FIRST_VALUE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/navigation_functions#first_value) | Gets a value for the first row in the current window frame. |
| [`FLOOR`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#floor) | Gets the largest integral value that isn't greater than `X`. |
| [`FORMAT_DATE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#format_date) | Formats a `DATE` value according to a specified format string. |
| [`FORMAT_DATETIME`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/datetime_functions#format_datetime) | Formats a `DATETIME` value according to a specified format string. |
| [`FORMAT_TIME`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/time_functions#format_time) | Formats a `TIME` value according to the specified format string. |
| [`FORMAT_TIMESTAMP`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#format_timestamp) | Formats a `TIMESTAMP` value according to the specified format string. |
| [`FORMAT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#format_string) | Formats data and produces the results as a `STRING` value. |
| [`FROM_BASE32`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#from_base32) | Converts a base32-encoded `STRING` value into a `BYTES` value. |
| [`FROM_BASE64`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#from_base64) | Converts a base64-encoded `STRING` value into a `BYTES` value. |
| [`FROM_HEX`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#from_hex) | Converts a hexadecimal-encoded `STRING` value into a `BYTES` value. |
| [`GAP_FILL`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/time-series-functions#gap_fill) | Finds and fills gaps in a time series. |
| [`GENERATE_ARRAY`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/array_functions#generate_array) | Generates an array of values in a range. |
| [`GENERATE_DATE_ARRAY`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/array_functions#generate_date_array) | Generates an array of dates in a range. |
| [`GENERATE_RANGE_ARRAY`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/range-functions#generate_range_array) | Splits a range into an array of subranges. |
| [`GENERATE_TIMESTAMP_ARRAY`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/array_functions#generate_timestamp_array) | Generates an array of timestamps in a range. |
| [`GENERATE_UUID`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/utility-functions#generate_uuid) | Produces a random universally unique identifier (UUID) as a `STRING` value. |
| [`GREATEST`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#greatest) | Gets the greatest value among `X1,...,XN`. |
| [`GROUPING`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#grouping) | Checks if a groupable value in the `GROUP BY` clause is aggregated. |
| [`HLL_COUNT.EXTRACT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/hll_functions#hll_countextract) | Extracts a cardinality estimate of an HLL++ sketch. |
| [`HLL_COUNT.INIT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/hll_functions#hll_countinit) | Aggregates values of the same underlying type into a new HLL++ sketch. |
| [`HLL_COUNT.MERGE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/hll_functions#hll_countmerge) | Merges HLL++ sketches of the same underlying type into a new sketch, and then gets the cardinality of the new sketch. |
| [`HLL_COUNT.MERGE_PARTIAL`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/hll_functions#hll_countmerge_partial) | Merges HLL++ sketches of the same underlying type into a new sketch. |
| [`IEEE_DIVIDE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#ieee_divide) | Divides `X` by `Y`, but doesn't generate errors for division by zero or overflow. |
| [`INITCAP`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#initcap) | Formats a `STRING` as proper case, which means that the first character in each word is uppercase and all other characters are lowercase. |
| [`INSTR`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#instr) | Finds the position of a subvalue inside another value, optionally starting the search at a given offset or occurrence. |
| [`INT64`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#int64_for_json) | Converts a JSON number to a SQL `INT64` value. |
| [`IS_INF`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#is_inf) | Checks if `X` is positive or negative infinity. |
| [`IS_NAN`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#is_nan) | Checks if `X` is a `NaN` value. |
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
| [`JUSTIFY_DAYS`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/interval_functions#justify_days) | Normalizes the day part of an `INTERVAL` value. |
| [`JUSTIFY_HOURS`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/interval_functions#justify_hours) | Normalizes the time part of an `INTERVAL` value. |
| [`JUSTIFY_INTERVAL`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/interval_functions#justify_interval) | Normalizes the day and time parts of an `INTERVAL` value. |
| [`KEYS.ADD_KEY_FROM_RAW_BYTES`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aead_encryption_functions#keysadd_key_from_raw_bytes) | Adds a key to a keyset, and return the new keyset as a serialized `BYTES` value. |
| [`KEYS.KEYSET_CHAIN`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aead_encryption_functions#keyskeyset_chain) | Produces a Tink keyset that's encrypted with a Cloud KMS key. |
| [`KEYS.KEYSET_FROM_JSON`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aead_encryption_functions#keyskeyset_from_json) | Converts a `STRING` JSON keyset to a serialized `BYTES` value. |
| [`KEYS.KEYSET_LENGTH`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aead_encryption_functions#keyskeyset_length) | Gets the number of keys in the provided keyset. |
| [`KEYS.KEYSET_TO_JSON`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aead_encryption_functions#keyskeyset_to_json) | Gets a JSON `STRING` representation of a keyset. |
| [`KEYS.NEW_KEYSET`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aead_encryption_functions#keysnew_keyset) | Gets a serialized keyset containing a new key based on the key type. |
| [`KEYS.NEW_WRAPPED_KEYSET`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aead_encryption_functions#keysnew_wrapped_keyset) | Creates a new keyset and encrypts it with a Cloud KMS key. |
| [`KEYS.REWRAP_KEYSET`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aead_encryption_functions#keysrewrap_keyset) | Re-encrypts a wrapped keyset with a new Cloud KMS key. |
| [`KEYS.ROTATE_KEYSET`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aead_encryption_functions#keysrotate_keyset) | Adds a new primary cryptographic key to a keyset, based on the key type. |
| [`KEYS.ROTATE_WRAPPED_KEYSET`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aead_encryption_functions#keysrotate_wrapped_keyset) | Rewraps a keyset and rotates it. |
| [`KLL_QUANTILES.EXTRACT_INT64`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/kll_functions#kll_quantilesextract_int64) | Gets a selected number of quantiles from an `INT64`-initialized KLL sketch. |
| [`KLL_QUANTILES.EXTRACT_FLOAT64`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/kll_functions#kll_quantilesextract_double) | Gets a selected number of quantiles from a `FLOAT64`-initialized KLL sketch. |
| [`KLL_QUANTILES.EXTRACT_POINT_INT64`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/kll_functions#kll_quantilesextract_point_int64) | Gets a specific quantile from an `INT64`-initialized KLL sketch. |
| [`KLL_QUANTILES.EXTRACT_POINT_FLOAT64`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/kll_functions#kll_quantilesextract_point_double) | Gets a specific quantile from a `FLOAT64`-initialized KLL sketch. |
| [`KLL_QUANTILES.INIT_INT64`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/kll_functions#kll_quantilesinit_int64) | Aggregates values into an `INT64`-initialized KLL sketch. |
| [`KLL_QUANTILES.INIT_FLOAT64`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/kll_functions#kll_quantilesinit_double) | Aggregates values into a `FLOAT64`-initialized KLL sketch. |
| [`KLL_QUANTILES.MERGE_INT64`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/kll_functions#kll_quantilesmerge_int64) | Merges `INT64`-initialized KLL sketches into a new sketch, and then gets the quantiles from the new sketch. |
| [`KLL_QUANTILES.MERGE_FLOAT64`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/kll_functions#kll_quantilesmerge_double) | Merges `FLOAT64`-initialized KLL sketches into a new sketch, and then gets the quantiles from the new sketch. |
| [`KLL_QUANTILES.MERGE_PARTIAL`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/kll_functions#kll_quantilesmerge_partial) | Merges KLL sketches of the same underlying type into a new sketch. |
| [`KLL_QUANTILES.MERGE_POINT_INT64`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/kll_functions#kll_quantilesmerge_point_int64) | Merges `INT64`-initialized KLL sketches into a new sketch, and then gets a specific quantile from the new sketch. |
| [`KLL_QUANTILES.MERGE_POINT_FLOAT64`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/kll_functions#kll_quantilesmerge_point_double) | Merges `FLOAT64`-initialized KLL sketches into a new sketch, and then gets a specific quantile from the new sketch. |
| [`LABELS`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/graph-sql-functions#labels) | Gets the labels associated with a graph element. |
| [`LAG`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/navigation_functions#lag) | Gets a value for a preceding row. |
| [`LAST_DAY`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#last_day) | Gets the last day in a specified time period that contains a `DATE` value. |
| [`LAST_DAY`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/datetime_functions#last_day) | Gets the last day in a specified time period that contains a `DATETIME` value. |
| [`LAST_VALUE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/navigation_functions#last_value) | Gets a value for the last row in the current window frame. |
| [`LAX_BOOL`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#lax_bool) | Attempts to convert a JSON value to a SQL `BOOL` value. |
| [`LAX_FLOAT64`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#lax_double) | Attempts to convert a JSON value to a SQL `FLOAT64` value. |
| [`LAX_INT64`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#lax_int64) | Attempts to convert a JSON value to a SQL `INT64` value. |
| [`LAX_STRING`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#lax_string) | Attempts to convert a JSON value to a SQL `STRING` value. |
| [`LEAD`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/navigation_functions#lead) | Gets a value for a subsequent row. |
| [`LEAST`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#least) | Gets the least value among `X1,...,XN`. |
| [`LEFT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#left) | Gets the specified leftmost portion from a `STRING` or `BYTES` value. |
| [`LENGTH`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#length) | Gets the length of a `STRING` or `BYTES` value. |
| [`LN`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#ln) | Computes the natural logarithm of `X`. |
| [`LOG`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#log) | Computes the natural logarithm of `X` or the logarithm of `X` to base `Y`. |
| [`LOG10`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#log10) | Computes the natural logarithm of `X` to base 10. |
| [`LOGICAL_AND`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#logical_and) | Gets the logical AND of all non-`NULL` expressions. |
| [`LOGICAL_OR`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#logical_or) | Gets the logical OR of all non-`NULL` expressions. |
| [`LOWER`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#lower) | Formats alphabetic characters in a `STRING` value as lowercase. <br /> Formats ASCII characters in a `BYTES` value as lowercase. |
| [`LPAD`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#lpad) | Prepends a `STRING` or `BYTES` value with a pattern. |
| [`LTRIM`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#ltrim) | Identical to the `TRIM` function, but only removes leading characters. |
| [`MAKE_INTERVAL`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/interval_functions#make_interval) | Constructs an `INTERVAL` value. |
| [`MAX`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#max) | Gets the maximum non-`NULL` value. |
| [`MAX_BY`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#max_by) | Synonym for `ANY_VALUE(x HAVING MAX y)`. |
| [`MD5`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/hash_functions#md5) | Computes the hash of a `STRING` or `BYTES` value, using the MD5 algorithm. |
| [`MIN`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#min) | Gets the minimum non-`NULL` value. |
| [`MIN_BY`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#min_by) | Synonym for `ANY_VALUE(x HAVING MIN y)`. |
| [`MOD`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#mod) | Gets the remainder of the division of `X` by `Y`. |
| [`NET.HOST`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/net_functions#nethost) | Gets the hostname from a URL. |
| [`NET.IP_FROM_STRING`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/net_functions#netip_from_string) | Converts an IPv4 or IPv6 address from a `STRING` value to a `BYTES` value in network byte order. |
| [`NET.IP_NET_MASK`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/net_functions#netip_net_mask) | Gets a network mask. |
| [`NET.IP_TO_STRING`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/net_functions#netip_to_string) | Converts an IPv4 or IPv6 address from a `BYTES` value in network byte order to a `STRING` value. |
| [`NET.IP_TRUNC`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/net_functions#netip_trunc) | Converts a `BYTES` IPv4 or IPv6 address in network byte order to a `BYTES` subnet address. |
| [`NET.IPV4_FROM_INT64`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/net_functions#netipv4_from_int64) | Converts an IPv4 address from an `INT64` value to a `BYTES` value in network byte order. |
| [`NET.IPV4_TO_INT64`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/net_functions#netipv4_to_int64) | Converts an IPv4 address from a `BYTES` value in network byte order to an `INT64` value. |
| [`NET.PUBLIC_SUFFIX`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/net_functions#netpublic_suffix) | Gets the public suffix from a URL. |
| [`NET.REG_DOMAIN`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/net_functions#netreg_domain) | Gets the registered or registrable domain from a URL. |
| [`NET.SAFE_IP_FROM_STRING`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/net_functions#netsafe_ip_from_string) | Similar to the `NET.IP_FROM_STRING`, but returns `NULL` instead of producing an error if the input is invalid. |
| [`NODES`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/graph-sql-functions#nodes) | Gets the nodes in a graph path. The resulting array retains the original order in the graph path. |
| [`NORMALIZE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#normalize) | Case-sensitively normalizes the characters in a `STRING` value. |
| [`NORMALIZE_AND_CASEFOLD`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#normalize_and_casefold) | Case-insensitively normalizes the characters in a `STRING` value. |
| [`NTH_VALUE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/navigation_functions#nth_value) | Gets a value for the Nth row of the current window frame. |
| [`NTILE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/numbering_functions#ntile) | Gets the quantile bucket number (1-based) of each row within a window. |
| [`OBJ.FETCH_METADATA`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/objectref_functions#objfetch_metadata) | Fetches Cloud Storage metadata for a partially populated `ObjectRef` value. |
| [`OBJ.GET_ACCESS_URL`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/objectref_functions#objget_access_url) | Returns access URLs for a Cloud Storage object. |
| [`OBJ.GET_READ_URL`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/objectref_functions#objget_read_url) | Returns a read URL and status for a Cloud Storage object. |
| [`OBJ.MAKE_REF`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/objectref_functions#objmake_ref) | Creates an `ObjectRef` value that contains reference information for a Cloud Storage object. |
| [`OCTET_LENGTH`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#octet_length) | Alias for `BYTE_LENGTH`. |
| [`PARSE_BIGNUMERIC`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/conversion_functions#parse_bignumeric) | Converts a `STRING` value to a `BIGNUMERIC` value. |
| [`PARSE_DATE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#parse_date) | Converts a `STRING` value to a `DATE` value. |
| [`PARSE_DATETIME`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/datetime_functions#parse_datetime) | Converts a `STRING` value to a `DATETIME` value. |
| [`PARSE_JSON`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#parse_json) | Converts a JSON-formatted `STRING` value to a `JSON` value. |
| [`PARSE_NUMERIC`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/conversion_functions#parse_numeric) | Converts a `STRING` value to a `NUMERIC` value. |
| [`PARSE_TIME`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/time_functions#parse_time) | Converts a `STRING` value to a `TIME` value. |
| [`PARSE_TIMESTAMP`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#parse_timestamp) | Converts a `STRING` value to a `TIMESTAMP` value. |
| [`PATH_FIRST`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/graph-sql-functions#path_first) | Gets the first node in a graph path. |
| [`PATH_LAST`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/graph-sql-functions#path_last) | Gets the last node in a graph path. |
| [`PATH_LENGTH`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/graph-sql-functions#path_length) | Gets the number of edges in a graph path. |
| [`PERCENT_RANK`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/numbering_functions#percent_rank) | Gets the percentile rank (from 0 to 1) of each row within a window. |
| [`PERCENTILE_CONT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/navigation_functions#percentile_cont) | Computes the specified percentile for a value, using linear interpolation. |
| [`PERCENTILE_CONT` (Differential Privacy)](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate-dp-functions#dp_percentile_cont) | `DIFFERENTIAL_PRIVACY`-supported `PERCENTILE_CONT`. Computes a differentially-private percentile across privacy unit columns in a query with a `DIFFERENTIAL_PRIVACY` clause. |
| [`PERCENTILE_DISC`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/navigation_functions#percentile_disc) | Computes the specified percentile for a discrete value. |
| [`POW`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#pow) | Produces the value of `X` raised to the power of `Y`. |
| [`POWER`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#power) | Synonym of `POW`. |
| [`RAND`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#rand) | Generates a pseudo-random value of type `FLOAT64` in the range of `[0, 1)`. |
| [`RANGE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/range-functions#range) | Constructs a range of `DATE`, `DATETIME`, or `TIMESTAMP` values. |
| [`RANGE_BUCKET`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#range_bucket) | Scans through a sorted array and returns the 0-based position of a point's upper bound. |
| [`RANGE_CONTAINS`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/range-functions#range_contains) | Signature 1: Checks if one range is in another range. <br /> Signature 2: Checks if a value is in a range. |
| [`RANGE_END`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/range-functions#range_end) | Gets the upper bound of a range. |
| [`RANGE_INTERSECT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/range-functions#range_intersect) | Gets a segment of two ranges that intersect. |
| [`RANGE_OVERLAPS`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/range-functions#range_overlaps) | Checks if two ranges overlap. |
| [`RANGE_SESSIONIZE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/range-functions#range_sessionize) | Produces a table of sessionized ranges. |
| [`RANGE_START`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/range-functions#range_start) | Gets the lower bound of a range. |
| [`RANK`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/numbering_functions#rank) | Gets the rank (1-based) of each row within a window. |
| [`REGEXP_CONTAINS`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#regexp_contains) | Checks if a value is a partial match for a regular expression. |
| [`REGEXP_EXTRACT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#regexp_extract) | Produces a substring that matches a regular expression. |
| [`REGEXP_EXTRACT_ALL`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#regexp_extract_all) | Produces an array of all substrings that match a regular expression. |
| [`REGEXP_INSTR`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#regexp_instr) | Finds the position of a regular expression match in a value, optionally starting the search at a given offset or occurrence. |
| [`REGEXP_REPLACE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#regexp_replace) | Produces a `STRING` value where all substrings that match a regular expression are replaced with a specified value. |
| [`REGEXP_SUBSTR`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#regexp_substr) | Synonym for `REGEXP_EXTRACT`. |
| [`REPEAT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#repeat) | Produces a `STRING` or `BYTES` value that consists of an original value, repeated. |
| [`REPLACE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#replace) | Replaces all occurrences of a pattern with another pattern in a `STRING` or `BYTES` value. |
| [`REVERSE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#reverse) | Reverses a `STRING` or `BYTES` value. |
| [`RIGHT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#right) | Gets the specified rightmost portion from a `STRING` or `BYTES` value. |
| [`ROUND`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#round) | Rounds `X` to the nearest integer or rounds `X` to `N` decimal places after the decimal point. |
| [`ROW_NUMBER`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/numbering_functions#row_number) | Gets the sequential row number (1-based) of each row within a window. |
| [`RPAD`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#rpad) | Appends a `STRING` or `BYTES` value with a pattern. |
| [`RTRIM`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#rtrim) | Identical to the `TRIM` function, but only removes trailing characters. |
| [`S2_CELLIDFROMPOINT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#s2_cellidfrompoint) | Gets the S2 cell ID covering a point `GEOGRAPHY` value. |
| [`S2_COVERINGCELLIDS`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#s2_coveringcellids) | Gets an array of S2 cell IDs that cover a `GEOGRAPHY` value. |
| [`SAFE_ADD`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#safe_add) | Equivalent to the addition operator (`X + Y`), but returns `NULL` if overflow occurs. |
| [`SAFE_CAST`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/conversion_functions#safe_casting) | Similar to the `CAST` function, but returns `NULL` when a runtime error is produced. |
| [`SAFE_CONVERT_BYTES_TO_STRING`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#safe_convert_bytes_to_string) | Converts a `BYTES` value to a `STRING` value and replace any invalid UTF-8 characters with the Unicode replacement character, `U+FFFD`. |
| [`SAFE_DIVIDE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#safe_divide) | Equivalent to the division operator (`X / Y`), but returns `NULL` if an error occurs. |
| [`SAFE_MULTIPLY`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#safe_multiply) | Equivalent to the multiplication operator (`X * Y`), but returns `NULL` if overflow occurs. |
| [`SAFE_NEGATE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#safe_negate) | Equivalent to the unary minus operator (`-X`), but returns `NULL` if overflow occurs. |
| [`SAFE_SUBTRACT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#safe_subtract) | Equivalent to the subtraction operator (`X - Y`), but returns `NULL` if overflow occurs. |
| [`SEARCH`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/search_functions#search) | Checks to see whether a table or other search data contains a set of search terms. |
| [`SEC`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#sec) | Computes the secant of `X`. |
| [`SECH`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#sech) | Computes the hyperbolic secant of `X`. |
| [`SESSION_USER`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/security_functions#session_user) | Get the email address or principal identifier of the user that's running the query. |
| [`SHA1`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/hash_functions#sha1) | Computes the hash of a `STRING` or `BYTES` value, using the SHA-1 algorithm. |
| [`SHA256`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/hash_functions#sha256) | Computes the hash of a `STRING` or `BYTES` value, using the SHA-256 algorithm. |
| [`SHA512`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/hash_functions#sha512) | Computes the hash of a `STRING` or `BYTES` value, using the SHA-512 algorithm. |
| [`SIGN`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#sign) | Produces -1 , 0, or +1 for negative, zero, and positive arguments respectively. |
| [`SIN`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#sin) | Computes the sine of `X`. |
| [`SINH`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#sinh) | Computes the hyperbolic sine of `X`. |
| [`SOURCE_NODE_ID`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/graph-sql-functions#source_node_id) | Gets a unique identifier of a graph edge's source node. |
| [`SOUNDEX`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#soundex) | Gets the Soundex codes for words in a `STRING` value. |
| [`SPLIT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#split) | Splits a `STRING` or `BYTES` value, using a delimiter. |
| [`SQRT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#sqrt) | Computes the square root of `X`. |
| [`ST_ANGLE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_angle) | Takes three point `GEOGRAPHY` values, which represent two intersecting lines, and returns the angle between these lines. |
| [`ST_AREA`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_area) | Gets the area covered by the polygons in a `GEOGRAPHY` value. |
| [`ST_ASBINARY`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_asbinary) | Converts a `GEOGRAPHY` value to a `BYTES` WKB geography value. |
| [`ST_ASGEOJSON`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_asgeojson) | Converts a `GEOGRAPHY` value to a `STRING` GeoJSON geography value. |
| [`ST_ASTEXT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_astext) | Converts a `GEOGRAPHY` value to a `STRING` WKT geography value. |
| [`ST_AZIMUTH`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_azimuth) | Gets the azimuth of a line segment formed by two point `GEOGRAPHY` values. |
| [`ST_BOUNDARY`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_boundary) | Gets the union of component boundaries in a `GEOGRAPHY` value. |
| [`ST_BOUNDINGBOX`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_boundingbox) | Gets the bounding box for a `GEOGRAPHY` value. |
| [`ST_BUFFER`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_buffer) | Gets the buffer around a `GEOGRAPHY` value, using a specific number of segments. |
| [`ST_BUFFERWITHTOLERANCE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_bufferwithtolerance) | Gets the buffer around a `GEOGRAPHY` value, using tolerance. |
| [`ST_CENTROID`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_centroid) | Gets the centroid of a `GEOGRAPHY` value. |
| [`ST_CENTROID_AGG`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_centroid_agg) | Gets the centroid of a set of `GEOGRAPHY` values. |
| [`ST_CLOSESTPOINT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_closestpoint) | Gets the point on a `GEOGRAPHY` value which is closest to any point in a second `GEOGRAPHY` value. |
| [`ST_CLUSTERDBSCAN`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_clusterdbscan) | Performs DBSCAN clustering on a group of `GEOGRAPHY` values and produces a 0-based cluster number for this row. |
| [`ST_CONTAINS`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_contains) | Checks if one `GEOGRAPHY` value contains another `GEOGRAPHY` value. |
| [`ST_CONVEXHULL`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_convexhull) | Returns the convex hull for a `GEOGRAPHY` value. |
| [`ST_COVEREDBY`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_coveredby) | Checks if all points of a `GEOGRAPHY` value are on the boundary or interior of another `GEOGRAPHY` value. |
| [`ST_COVERS`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_covers) | Checks if all points of a `GEOGRAPHY` value are on the boundary or interior of another `GEOGRAPHY` value. |
| [`ST_DIFFERENCE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_difference) | Gets the point set difference between two `GEOGRAPHY` values. |
| [`ST_DIMENSION`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_dimension) | Gets the dimension of the highest-dimensional element in a `GEOGRAPHY` value. |
| [`ST_DISJOINT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_disjoint) | Checks if two `GEOGRAPHY` values are disjoint (don't intersect). |
| [`ST_DISTANCE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_distance) | Gets the shortest distance in meters between two `GEOGRAPHY` values. |
| [`ST_DUMP`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_dump) | Returns an array of simple `GEOGRAPHY` components in a `GEOGRAPHY` value. |
| [`ST_DWITHIN`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_dwithin) | Checks if any points in two `GEOGRAPHY` values are within a given distance. |
| [`ST_ENDPOINT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_endpoint) | Gets the last point of a linestring `GEOGRAPHY` value. |
| [`ST_EQUALS`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_equals) | Checks if two `GEOGRAPHY` values represent the same `GEOGRAPHY` value. |
| [`ST_EXTENT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_extent) | Gets the bounding box for a group of `GEOGRAPHY` values. |
| [`ST_EXTERIORRING`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_exteriorring) | Returns a linestring `GEOGRAPHY` value that corresponds to the outermost ring of a polygon `GEOGRAPHY` value. |
| [`ST_GEOGFROM`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_geogfrom) | Converts a `STRING` or `BYTES` value into a `GEOGRAPHY` value. |
| [`ST_GEOGFROMGEOJSON`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_geogfromgeojson) | Converts a `STRING` GeoJSON geometry value into a `GEOGRAPHY` value. |
| [`ST_GEOGFROMTEXT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_geogfromtext) | Converts a `STRING` WKT geometry value into a `GEOGRAPHY` value. |
| [`ST_GEOGFROMWKB`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_geogfromwkb) | Converts a `BYTES` or hexadecimal-text `STRING` WKT geometry value into a `GEOGRAPHY` value. |
| [`ST_GEOGPOINT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_geogpoint) | Creates a point `GEOGRAPHY` value for a given longitude and latitude. |
| [`ST_GEOGPOINTFROMGEOHASH`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_geogpointfromgeohash) | Gets a point `GEOGRAPHY` value that's in the middle of a bounding box defined in a `STRING` GeoHash value. |
| [`ST_GEOHASH`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_geohash) | Converts a point `GEOGRAPHY` value to a `STRING` GeoHash value. |
| [`ST_GEOMETRYTYPE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_geometrytype) | Gets the Open Geospatial Consortium (OGC) geometry type for a `GEOGRAPHY` value. |
| [`ST_HAUSDORFFDISTANCE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_hausdorffdistance) | Gets the discrete Hausdorff distance between two geometries. |
| [`ST_HAUSDORFFDWITHIN`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_hausdorffdwithin) | Checks if the Hausdorff distance between two `GEOGRAPHY` values is within a given distance. |
| [`ST_INTERIORRINGS`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_interiorrings) | Gets the interior rings of a polygon `GEOGRAPHY` value. |
| [`ST_INTERSECTION`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_intersection) | Gets the point set intersection of two `GEOGRAPHY` values. |
| [`ST_INTERSECTS`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_intersects) | Checks if at least one point appears in two `GEOGRAPHY` values. |
| [`ST_INTERSECTSBOX`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_intersectsbox) | Checks if a `GEOGRAPHY` value intersects a rectangle. |
| [`ST_ISCLOSED`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_isclosed) | Checks if all components in a `GEOGRAPHY` value are closed. |
| [`ST_ISCOLLECTION`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_iscollection) | Checks if the total number of points, linestrings, and polygons is greater than one in a `GEOGRAPHY` value. |
| [`ST_ISEMPTY`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_isempty) | Checks if a `GEOGRAPHY` value is empty. |
| [`ST_ISRING`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_isring) | Checks if a `GEOGRAPHY` value is a closed, simple linestring. |
| [`ST_LENGTH`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_length) | Gets the total length of lines in a `GEOGRAPHY` value. |
| [`ST_LINEINTERPOLATEPOINT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_lineinterpolatepoint) | Gets a point at a specific fraction in a linestring `GEOGRAPHY` value. |
| [`ST_LINELOCATEPOINT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_linelocatepoint) | Gets a section of a linestring `GEOGRAPHY` value between the start point and a point `GEOGRAPHY` value. |
| [`ST_LINESUBSTRING`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_linesubstring) | Gets a segment of a single linestring at a specific starting and ending fraction. |
| [`ST_MAKELINE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_makeline) | Creates a linestring `GEOGRAPHY` value by concatenating the point and linestring vertices of `GEOGRAPHY` values. |
| [`ST_MAKEPOLYGON`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_makepolygon) | Constructs a polygon `GEOGRAPHY` value by combining a polygon shell with polygon holes. |
| [`ST_MAKEPOLYGONORIENTED`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_makepolygonoriented) | Constructs a polygon `GEOGRAPHY` value, using an array of linestring `GEOGRAPHY` values. The vertex ordering of each linestring determines the orientation of each polygon ring. |
| [`ST_MAXDISTANCE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_maxdistance) | Gets the longest distance between two non-empty `GEOGRAPHY` values. |
| [`ST_NPOINTS`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_npoints) | An alias of `ST_NUMPOINTS`. |
| [`ST_NUMGEOMETRIES`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_numgeometries) | Gets the number of geometries in a `GEOGRAPHY` value. |
| [`ST_NUMPOINTS`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_numpoints) | Gets the number of vertices in the a `GEOGRAPHY` value. |
| [`ST_PERIMETER`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_perimeter) | Gets the length of the boundary of the polygons in a `GEOGRAPHY` value. |
| [`ST_POINTN`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_pointn) | Gets the point at a specific index of a linestring `GEOGRAPHY` value. |
| [`ST_REGIONSTATS`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_regionstats) | Computes statistics describing the pixels in a geospatial raster image that intersect a `GEOGRAPHY` value. |
| [`ST_SIMPLIFY`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_simplify) | Converts a `GEOGRAPHY` value into a simplified `GEOGRAPHY` value, using tolerance. |
| [`ST_SNAPTOGRID`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_snaptogrid) | Produces a `GEOGRAPHY` value, where each vertex has been snapped to a longitude/latitude grid. |
| [`ST_STARTPOINT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_startpoint) | Gets the first point of a linestring `GEOGRAPHY` value. |
| [`ST_TOUCHES`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_touches) | Checks if two `GEOGRAPHY` values intersect and their interiors have no elements in common. |
| [`ST_UNION`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_union) | Gets the point set union of multiple `GEOGRAPHY` values. |
| [`ST_UNION_AGG`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_union_agg) | Aggregates over `GEOGRAPHY` values and gets their point set union. |
| [`ST_WITHIN`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_within) | Checks if one `GEOGRAPHY` value contains another `GEOGRAPHY` value. |
| [`ST_X`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_x) | Gets the longitude from a point `GEOGRAPHY` value. |
| [`ST_Y`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_y) | Gets the latitude from a point `GEOGRAPHY` value. |
| [`STARTS_WITH`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#starts_with) | Checks if a `STRING` or `BYTES` value is a prefix of another value. |
| [`STDDEV`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#stddev) | An alias of the `STDDEV_SAMP` function. |
| [`STDDEV_POP`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#stddev_pop) | Computes the population (biased) standard deviation of the values. |
| [`STDDEV_SAMP`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#stddev_samp) | Computes the sample (unbiased) standard deviation of the values. |
| [`STRING` (JSON)](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#string_for_json) | Converts a JSON string to a SQL `STRING` value. |
| [`STRING` (Timestamp)](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#string) | Converts a `TIMESTAMP` value to a `STRING` value. |
| [`STRING_AGG`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#string_agg) | Concatenates non-`NULL` `STRING` or `BYTES` values. |
| [`STRPOS`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#strpos) | Finds the position of the first occurrence of a subvalue inside another value. |
| [`SUBSTR`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#substr) | Gets a portion of a `STRING` or `BYTES` value. |
| [`SUBSTRING`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#substring) | Alias for `SUBSTR` |
| [`SUM`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#sum) | Gets the sum of non-`NULL` values. |
| [`SUM` (Differential Privacy)](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate-dp-functions#dp_sum) | `DIFFERENTIAL_PRIVACY`-supported `SUM`. Gets the differentially-private sum of non-`NULL`, non-`NaN` values in a query with a `DIFFERENTIAL_PRIVACY` clause. |
| [`TAN`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#tan) | Computes the tangent of `X`. |
| [`TANH`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#tanh) | Computes the hyperbolic tangent of `X`. |
| [`TEXT_ANALYZE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/text-analysis-functions#text_analyze) | Extracts terms (tokens) from text and converts them into a tokenized document. |
| [`TF_IDF`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/text-analysis-functions#tf_idf) | Evaluates how relevant a term (token) is to a tokenized document in a set of tokenized documents. |
| [`TIME`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/time_functions#time) | Constructs a `TIME` value. |
| [`TIME_ADD`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/time_functions#time_add) | Adds a specified time interval to a `TIME` value. |
| [`TIME_DIFF`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/time_functions#time_diff) | Gets the number of unit boundaries between two `TIME` values at a particular time granularity. |
| [`TIME_SUB`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/time_functions#time_sub) | Subtracts a specified time interval from a `TIME` value. |
| [`TIME_TRUNC`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/time_functions#time_trunc) | Truncates a `TIME` value at a particular granularity. |
| [`TIMESTAMP`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#timestamp) | Constructs a `TIMESTAMP` value. |
| [`TIMESTAMP_ADD`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#timestamp_add) | Adds a specified time interval to a `TIMESTAMP` value. |
| [`TIMESTAMP_BUCKET`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/time-series-functions#timestamp_bucket) | Gets the lower bound of the timestamp bucket that contains a timestamp. |
| [`TIMESTAMP_DIFF`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#timestamp_diff) | Gets the number of unit boundaries between two `TIMESTAMP` values at a particular time granularity. |
| [`TIMESTAMP_MICROS`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#timestamp_micros) | Converts the number of microseconds since 1970-01-01 00:00:00 UTC to a `TIMESTAMP`. |
| [`TIMESTAMP_MILLIS`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#timestamp_millis) | Converts the number of milliseconds since 1970-01-01 00:00:00 UTC to a `TIMESTAMP`. |
| [`TIMESTAMP_SECONDS`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#timestamp_seconds) | Converts the number of seconds since 1970-01-01 00:00:00 UTC to a `TIMESTAMP`. |
| [`TIMESTAMP_SUB`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#timestamp_sub) | Subtracts a specified time interval from a `TIMESTAMP` value. |
| [`TIMESTAMP_TRUNC`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#timestamp_trunc) | Truncates a `TIMESTAMP` or `DATETIME` value at a particular granularity. |
| [`TO_BASE32`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#to_base32) | Converts a `BYTES` value to a base32-encoded `STRING` value. |
| [`TO_BASE64`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#to_base64) | Converts a `BYTES` value to a base64-encoded `STRING` value. |
| [`TO_CODE_POINTS`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#to_code_points) | Converts a `STRING` or `BYTES` value into an array of extended ASCII code points. |
| [`TO_HEX`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#to_hex) | Converts a `BYTES` value to a hexadecimal `STRING` value. |
| [`TO_JSON`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#to_json) | Converts a SQL value to a JSON value. |
| [`TO_JSON_STRING`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#to_json_string) | Converts a SQL value to a JSON-formatted `STRING` value. |
| [`TRANSLATE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#translate) | Within a value, replaces each source character with the corresponding target character. |
| [`TRIM`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#trim) | Removes the specified leading and trailing Unicode code points or bytes from a `STRING` or `BYTES` value. |
| [`TRUNC`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#trunc) | Rounds a number like `ROUND(X)` or `ROUND(X, N)`, but always rounds towards zero and never overflows. |
| [`TYPEOF`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/utility-functions#typeof) | Gets the name of the data type for an expression. |
| [`UNICODE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#unicode) | Gets the Unicode code point for the first character in a value. |
| [`UNIX_DATE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#unix_date) | Converts a `DATE` value to the number of days since 1970-01-01. |
| [`UNIX_MICROS`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#unix_micros) | Converts a `TIMESTAMP` value to the number of microseconds since 1970-01-01 00:00:00 UTC. |
| [`UNIX_MILLIS`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#unix_millis) | Converts a `TIMESTAMP` value to the number of milliseconds since 1970-01-01 00:00:00 UTC. |
| [`UNIX_SECONDS`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#unix_seconds) | Converts a `TIMESTAMP` value to the number of seconds since 1970-01-01 00:00:00 UTC. |
| [`UPPER`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#upper) | Formats alphabetic characters in a `STRING` value as uppercase. <br /> Formats ASCII characters in a `BYTES` value as uppercase. |
| [`VAR_POP`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#var_pop) | Computes the population (biased) variance of the values. |
| [`VAR_SAMP`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#var_samp) | Computes the sample (unbiased) variance of the values. |
| [`VARIANCE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#variance) | An alias of `VAR_SAMP`. |
| [`VECTOR_SEARCH`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/search_functions#vector_search) | Performs a vector search on embeddings to find semantically similar entities. |
| [`VECTOR_INDEX.STATISTICS`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/vectorindex_functions#vector_indexstatistics) | Calculate how much an indexed table's data has drifted between when a vector index was trained and the present. |