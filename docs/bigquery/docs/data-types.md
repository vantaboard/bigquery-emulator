# Legacy SQL data types


This document details the data types supported by BigQuery's
legacy SQL query syntax. The preferred query syntax for BigQuery
is GoogleSQL. For information on data types in GoogleSQL, see
the [GoogleSQL data types](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types).

## Legacy SQL data types

Your data can include the following data types:

| Data type | Possible values |
|---|---|
| STRING | Variable-length character (UTF-8) data. |
| BYTES | Variable-length binary data. - Imported BYTES data must be base64-encoded, except for Avro BYTES data, which BigQuery can read and convert. - BYTES data read from a BigQuery table are base64-encoded, unless you export to Avro format, in which case the Avro bytes data type applies. |
| INTEGER | 64-bit signed integer. If you are using the BigQuery API to load an integer outside the range of \[-2^53^+1, 2^53^-1\] (in most cases, this means larger than 9,007,199,254,740,991), into an integer (INT64) column, you must pass it as a string to avoid data corruption. This issue is caused by a limitation on integer size in JSON/ECMAScript. For more information, see [the Numbers section of RFC 7159](https://www.rfc-editor.org/rfc/rfc7159.html#section-6). |
| FLOAT | Double-precision floating-point format. |
| NUMERIC | Legacy SQL has limited support for NUMERIC. For more information, see [Exact numeric in legacy SQL](https://docs.cloud.google.com/bigquery/docs/data-types#numeric-type-support). |
| BIGNUMERIC | Legacy SQL has limited support for BIGNUMERIC. For more information, see [Exact numeric in legacy SQL](https://docs.cloud.google.com/bigquery/docs/data-types#numeric-type-support). |
| BOOLEAN | - **CSV format:** `1` or `0`, `true` or `false`, `t` or `f`, `yes` or `no`, or `y` or `n` (all case-insensitive). - **JSON format:** `true` or `false` (case-insensitive). |
| RECORD | A collection of one or more other fields. |
| TIMESTAMP | You can describe TIMESTAMP data types as either UNIX timestamps or calendar datetimes. BigQuery stores TIMESTAMP data internally as a UNIX timestamp with microsecond precision. **UNIX timestamps** A positive or negative decimal number. A positive number specifies the number of seconds since the epoch (1970-01-01 00:00:00 UTC), and a negative number specifies the number of seconds before the epoch. Up to 6 decimal places (microsecond precision) are preserved. **Date and time strings** A date and time string in the format `YYYY-MM-DD HH:MM:SS`. The `UTC` and `Z` specifiers are supported. You can supply a timezone offset in your date and time strings, but BigQuery doesn't preserve the offset after converting the value to its internal format. If you need to preserve the original timezone data, store the timezone offset in a separate column. The leading zero is required when you specify a single-digit timezone offset. Date and time strings must be quoted when using JSON format. **Examples** The following examples show identical ways of describing specific dates, in both UNIX timestamp and date and time string formats. | Event | UNIX timestamp format | Date/time string format | |---|---|---| | Minor (M4.2) earthquake near Oklahoma City | ```sh 1408452095.220 1408452095.220000 ``` | ```sh 2014-08-19 07:41:35.220 -05:00 2014-08-19 12:41:35.220 UTC 2014-08-19 12:41:35.220 2014-08-19 12:41:35.220000 2014-08-19T12:41:35.220Z ``` | | Neil Armstrong sets foot on the moon | ```sh -14182916 ``` | ```sh 1969-07-20 20:18:04 1969-07-20 20:18:04 UTC 1969-07-20T20:18:04 ``` | | Deadline for fixing [Y10k bug](https://en.wikipedia.org/wiki/Year_10,000_problem) | ```sh 253402300800 2.53402300800e11 ``` | ```sh 10000-01-01 00:00 ``` | |
| DATE | Legacy SQL has limited support for DATE. For more information, see [Civil time in legacy SQL](https://docs.cloud.google.com/bigquery/docs/data-types#civil-time). |
| TIME | Legacy SQL has limited support for TIME. For more information, see [Civil time in legacy SQL](https://docs.cloud.google.com/bigquery/docs/data-types#civil-time). |
| DATETIME | Legacy SQL has limited support for DATETIME. For more information, see [Civil time in legacy SQL](https://docs.cloud.google.com/bigquery/docs/data-types#civil-time). |

## Exact numeric in legacy SQL

You can read NUMERIC or BIGNUMERIC values in non-modifying clauses such as
`SELECT list (with aliases)`, `GROUP BY keys`, and pass-through fields in
window functions, and so on. However, any computation over NUMERIC or
BIGNUMERIC values, including comparisons, produces undefined results.

The following cast and conversion functions are supported in legacy SQL:

- `CAST(<numeric> AS STRING)`
- `CAST(<bignumeric> AS STRING)`
- `CAST(<string> AS NUMERIC)`
- `CAST(<string> AS BIGNUMERIC)`

## Civil time in legacy SQL

You can read civil time data types---DATE, TIME, and
DATETIME---and process them with non-modifying operators such as
`SELECT list (with aliases)`, `GROUP BY keys`, and pass-through fields in
window functions, etc. However, any other computation over civil time values,
including comparisons, produces undefined results.

The following casts and conversion functions are supported in legacy SQL:

- `CAST(<date> AS STRING)`
- `CAST(<time> AS STRING)`
- `CAST(<datetime> AS STRING)`
- `CAST(<string> AS DATE)`
- `CAST(<string> AS TIME)`
- `CAST(<string> AS DATETIME)`

In practice, legacy SQL interprets civil time values as integers, and operations
on integers that you think are civil time values produce unexpected results.

To compute values using civil time data types, consider
[GoogleSQL](https://docs.cloud.google.com/bigquery/sql-reference), which supports all SQL
operations on the
[DATE](https://docs.cloud.google.com/bigquery/sql-reference/data-types#date-type),
[DATETIME](https://docs.cloud.google.com/bigquery/sql-reference/data-types#datetime-type), and
[TIME](https://docs.cloud.google.com/bigquery/sql-reference/data-types#time-type) data types.

## What's next

- To set a field's data type using the API, see [`schema.fields.type`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables#TableFieldSchema.FIELDS.type).
- For GoogleSQL data types, see [data types](https://docs.cloud.google.com/bigquery/sql-reference/data-types).