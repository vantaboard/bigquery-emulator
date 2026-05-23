# Oracle SQL translation guide

This document details the similarities and differences in SQL syntax between
Oracle and BigQuery to help you plan your migration. Use
[batch SQL translation](https://docs.cloud.google.com/bigquery/docs/batch-sql-translator) to
migrate your SQL scripts in bulk, or
[interactive SQL translation](https://docs.cloud.google.com/bigquery/docs/interactive-sql-translator)
to translate ad-hoc queries.

> [!NOTE]
> **Note:** In some cases, there is no direct mapping between a SQL element in Oracle and BigQuery. However, in most cases, you can achieve the same functionality in BigQuery that you can in Oracle using an alternative means, as shown in the examples in this document.

## Data types

This section shows equivalents between data types in Oracle and in
BigQuery.

| Oracle | BigQuery | Notes |
|---|---|---|
| `https://docs.oracle.com/cd/B19306_01/server.102/b14220/datatype.htm` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#string_type` |   |
| `https://docs.oracle.com/cd/B19306_01/server.102/b14220/datatype.htm` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#string_type` |   |
| `https://docs.oracle.com/cd/B19306_01/server.102/b14220/datatype.htm` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#string_type` |   |
| `https://docs.oracle.com/cd/B19306_01/server.102/b14220/datatype.htm` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#string_type` |   |
| `https://docs.oracle.com/cd/B19306_01/server.102/b14220/datatype.htm` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#string_type` |   |
| `https://docs.oracle.com/cd/B19306_01/server.102/b14220/datatype.htm` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#string_type` |   |
| `https://docs.oracle.com/cd/B19306_01/olap.102/b14346/dml_datatypes002.htm` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#integer_types` |   |
| `https://docs.oracle.com/cd/B19306_01/olap.102/b14346/dml_datatypes002.htm` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#integer_types` |   |
| `https://docs.oracle.com/cd/B19306_01/olap.102/b14346/dml_datatypes002.htm` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#integer_types` |   |
| `https://docs.oracle.com/cd/B19306_01/olap.102/b14346/dml_datatypes002.htm` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#numeric-type` | BigQuery does not allow user specification of custom values for precision or scale. As a result, a column in Oracle may be defined so that it has a bigger scale than BigQuery supports. Additionally, before storing a decimal number Oracle rounds up if that number has more digits after the decimal point than is specified for the corresponding column. In BigQuery this feature could be implemented using `ROUND()` function. |
| `https://docs.oracle.com/cd/B19306_01/olap.102/b14346/dml_datatypes002.htm` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#numeric-type` | BigQuery does not allow user specification of custom values for precision or scale. As a result, a column in Oracle may be defined so that it has a bigger scale than BigQuery supports. Additionally, before storing a decimal number Oracle rounds up if that number has more digits after the decimal point than is specified for the corresponding column. In BigQuery this feature could be implemented using `ROUND()` function. |
| `https://docs.oracle.com/cd/B19306_01/olap.102/b14346/dml_datatypes002.htm` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#integer_types` | If a user tries to store a decimal number, Oracle rounds it up to a whole number. For BigQuery an attempt to store a decimal number in a column defined as `INT64` results in an error. In this case, `ROUND()` function should be applied. BigQuery `INT64` data types allow up to 18 digits of precision. If a number field has more than 18 digits, `FLOAT64` data type should be used in BigQuery. |
| `https://docs.oracle.com/cd/B19306_01/olap.102/b14346/dml_datatypes002.htm` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#integer_types` | If a user tries to store a decimal number, Oracle rounds it up to a whole number. For BigQuery an attempt to store a decimal number in a column defined as `INT64` results in an error. In this case, `ROUND()` function should be applied. BigQuery `INT64` data types allow up to 18 digits of precision. If a number field has more than 18 digits, `FLOAT64` data type should be used in BigQuery. |
| `FLOAT` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#floating_point_types`/`https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#numeric-type` | `FLOAT` is an exact data type, and it's a `NUMBER` subtype in Oracle. In BigQuery, `FLOAT64` is an approximate data type. `NUMERIC` may be a better match for `FLOAT` type in BigQuery. |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/lnpls/plsql-data-types.html#GUID-239A89A6-4CBC-46F5-8A6A-10E8B465B7E8` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#floating_point_types`/`https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#numeric-type` | `FLOAT` is an exact data type, and it's a `NUMBER` subtype in Oracle. In BigQuery, `FLOAT64` is an approximate data type. `NUMERIC` may be a better match for `FLOAT` type in BigQuery. |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/lnpls/plsql-data-types.html#GUID-239A89A6-4CBC-46F5-8A6A-10E8B465B7E8` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#floating_point_types`/`https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#numeric-type` | `FLOAT` is an exact data type, and it's a `NUMBER` subtype in Oracle. In BigQuery, `FLOAT64` is an approximate data type. `NUMERIC` may be a better match for `FLOAT` type in BigQuery. |
| `LONG` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#bytes_type` | `LONG` data type is used in earlier versions and is not suggested in new versions of Oracle Database. `BYTES` data type in BigQuery can be used if it is necessary to hold `LONG` data in BigQuery. A better approach would be putting binary objects in Cloud Storage and holding references in BigQuery. |
| `BLOB` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#bytes_type` | `BYTES` data type can be used to store variable-length binary data. If this field is not queried and not used in analytics, a better option is to store binary data in Cloud Storage. |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/adlob/BFILEs.html#GUID-D4642C92-F343-4700-9F1F-486F82249FB8` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#string_type` | Binary files can be stored in Cloud Storage and `STRING` data type can be used for referencing files in a BigQuery table. |
| `DATE` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#datetime_type` |   |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/nlspg/datetime-data-types-and-time-zone-support.html` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#timestamp_type` | BigQuery supports microsecond precision (10^-6^) in comparison to Oracle which supports precision ranging from 0 to 9. BigQuery supports a time zone region name from a TZ database and time zone offset from UTC. In BigQuery a time zone conversion should be manually performed to match Oracle's `TIMESTAMP WITH LOCAL TIME ZONE` feature. |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/nlspg/datetime-data-types-and-time-zone-support.html` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#timestamp_type` | BigQuery supports microsecond precision (10^-6^) in comparison to Oracle which supports precision ranging from 0 to 9. BigQuery supports a time zone region name from a TZ database and time zone offset from UTC. In BigQuery a time zone conversion should be manually performed to match Oracle's `TIMESTAMP WITH LOCAL TIME ZONE` feature. |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/nlspg/datetime-data-types-and-time-zone-support.html` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#timestamp_type` | BigQuery supports microsecond precision (10^-6^) in comparison to Oracle which supports precision ranging from 0 to 9. BigQuery supports a time zone region name from a TZ database and time zone offset from UTC. In BigQuery a time zone conversion should be manually performed to match Oracle's `TIMESTAMP WITH LOCAL TIME ZONE` feature. |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/nlspg/datetime-data-types-and-time-zone-support.html` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#timestamp_type` | BigQuery supports microsecond precision (10^-6^) in comparison to Oracle which supports precision ranging from 0 to 9. BigQuery supports a time zone region name from a TZ database and time zone offset from UTC. In BigQuery a time zone conversion should be manually performed to match Oracle's `TIMESTAMP WITH LOCAL TIME ZONE` feature. |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/Data-Types.html#GUID-ED59E1B3-BA8D-4711-B5C8-B0199C676A95` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#string_type` | Interval values can be stored as `STRING` data type in BigQuery. |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/Data-Types.html#SQLRF-GUID-B03DD036-66F8-4BD3-AF26-6D4433EBEC1C` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#string_type` | Interval values can be stored as `STRING` data type in BigQuery. |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/lnoci/data-types.html` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#bytes_type` | `BYTES` data type can be used to store variable-length binary data. If this field is not queried and used in analytics, a better option is to store binary data on Cloud Storage. |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/lnoci/data-types.html` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#bytes_type` | `BYTES` data type can be used to store variable-length binary data. If this field is not queried and used in analytics, a better option is to store binary data on Cloud Storage. |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/lnoci/data-types.html` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#string_type` | These data types are used Oracle internally to specify unique addresses to rows in a table. Generally, `ROWID` or `UROWID` field should not be used in applications. But if this is the case, `STRING` data type can be used to hold this data. |

### Type formatting

Oracle SQL uses a set of default formats set as parameters for displaying
expressions and column data, and for conversions between data types. For
example, `NLS_DATE_FORMAT` set as `YYYY/MM/DD` formats dates as `YYYY/MM/DD`
by default. You can find more information about [the NLS settings in the Oracle
online documentation](https://docs.oracle.com/cd/B28359_01/server.111/b28298/ch3globenv.htm).
In BigQuery, there are no initialization parameters.

By default, BigQuery expects all source data to be
UTF-8 encoded when loading. Optionally, if you have CSV files with data encoded
in ISO-8859-1 format, you can explicitly specify the encoding when you import
your data so that BigQuery can properly convert your data to
UTF-8 during the import process.

It is only possible to import data that is ISO-8859-1 or UTF-8
encoded. BigQuery stores and returns the data as UTF-8 encoded.
Intended date format or time zone can be set in
[`DATE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions) and
[`TIMESTAMP`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions)
functions.

### Timestamp and date type formatting

When you convert timestamp and date formatting elements from Oracle to
BigQuery, you must pay attention to time zone differences between
`TIMESTAMP` and `DATETIME` as summarized in the following table.

Notice there are no parentheses in the Oracle formats because the formats
(`CURRENT_*`) are keywords, not functions.

| Oracle | BigQuery | Notes |
|---|---|---|---|
| `https://docs.oracle.com/cd/B19306_01/server.102/b14200/functions037.htm ` | `TIMESTAMP` information in Oracle can have different time zone information, which is defined using `WITH TIME ZONE` in column definition or setting` https://docs.oracle.com/cd/E11882_01/server.112/e10729/ch4datetime.htm#NLSPG004` variable. | If possible, use the `CURRENT_TIMESTAMP()` function, which is formatted in ISO format. However, the output format does always show the UTC time zone. (Internally, BigQuery does not have a time zone.) Note the following details on differences in the ISO format: `DATETIME` is formatted based on output channel conventions. In the BigQuery command-line tool and BigQuery console `DATETIME` is formatted using a `T` separator according to RFC 3339. However, in Python and Java JDBC, a space is used as a separator. If you want to use an explicit format, use the `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/datetime_functions#format_datetime`() function, which makes an explicit cast a string. For example, the following expression always returns a space separator: `CAST(CURRENT_DATETIME() AS STRING) ` ||
| `https://docs.oracle.com/cd/B19306_01/server.102/b14200/functions036.htm https://docs.oracle.com/cd/B19306_01/server.102/b14200/functions172.htm` | Oracle uses 2 types for date: - type 12 - type 13 Oracle uses type 12 when storing dates. Internally, these are numbers with fixed-length. Oracle uses type 13 when a is returned by `SYSDATE or CURRENT_DATE` | BigQuery has a separate `DATE` format that always returns a date in [ISO 8601](https://en.wikipedia.org/wiki/ISO_8601) format. `DATE_FROM_UNIX_DATE` can't be used because it is 1970-based. ||
| `https://docs.oracle.com/cd/E29805_01/server.230/es_eql/src/cdfp_analytics_date_time_arithmetic_operations.html-3` | Date values are represented as integers. Oracle supports arithmetic operators for date types. | For date types, use `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#date_add`() or `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#date_sub`(). BigQuery uses arithmetic operators for data types: `INT64`, `NUMERIC`, and `FLOAT64`. ||
| `https://docs.oracle.com/cd/B19306_01/server.102/b14237/initparams122.htm#REFRN10119` | Set the session or system date format. | BigQuery always uses [ISO 8601](https://en.wikipedia.org/wiki/ISO_8601), so make sure you convert Oracle dates and times. ||

## Query syntax

This section addresses differences in query syntax between Oracle and
BigQuery.

### `SELECT` statements

Most Oracle `SELECT` statements are compatible with BigQuery.

## Functions, operators, and expressions

The following sections list mappings between Oracle functions and
BigQuery equivalents.

### Comparison operators

Oracle and BigQuery comparison operators are ANSI SQL:2011
compliant. The comparison operators in the table below are the same in both
BigQuery and Oracle. You can use
[`REGEXP_CONTAINS`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#regexp_contains)
instead of `REGEXP_LIKE` in BigQuery.

| Operator | Description |
|---|---|
| `"="` | [Equal](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/operators#comparison_operators) |
| `<>` | [Not equal](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/operators#comparison_operators) |
| `!=` | [Not equal](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/operators#comparison_operators) |
| `>` | [Greater than](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/operators#comparison_operators) |
| `>=` | [Greater than or equal](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/operators#comparison_operators) |
| `<` | [Less than](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/operators#comparison_operators) |
| `<=` | [Less than or equal](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/operators#comparison_operators) |
| `IN ( )` | [Matches a value in a list](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/operators#comparison_operators) |
| `NOT` | [Negates a condition](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/operators#comparison_operators) |
| `BETWEEN` | [Within a range (inclusive)](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/operators#comparison_operators) |
| `IS NULL` | [`NULL` value](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/operators#comparison_operators) |
| `IS NOT NULL` | [Not `NULL` value](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/operators#comparison_operators) |
| `LIKE` | [Pattern matching with %](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/operators#comparison_operators) |
| `EXISTS` | [Condition is met if subquery returns at least one row](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/operators#comparison_operators) |

The operators on the table are the same both in BigQuery and Oracle.

### Logical expressions and functions

| Oracle | BigQuery |
|---|---|
| `https://docs.oracle.com/cd/B19306_01/server.102/b14200/expressions004.htm` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/conditional_expressions#case` |
| `https://docs.oracle.com/cd/B19306_01/server.102/b14200/functions023.htm` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/conditional_expressions#coalesce` |
| `https://docs.oracle.com/cd/B19306_01/server.102/b14200/functions040.htm` | ` https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/conditional_expressions` |
| `https://docs.oracle.com/cd/B19306_01/server.102/b14200/functions090.htm` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/conditional_expressions#ifnull` |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/lnpcb/embedded-SQL.html#GUID-AC00EAA0-5C2E-4E26-9287-EFDC5B36383B` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#limit_and_offset_clause` |
| `https://docs.oracle.com/cd/B28359_01/server.111/b28286/functions107.htm#SQLRF00681` | [`NULLIF(expression, expression_to_match)`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/conditional_expressions#nullif) |
| `https://docs.oracle.com/database/121/SQLRF/functions131.htm#SQLRF00684` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/conditional_expressions#ifnull` |
| `https://docs.oracle.com/database/121/SQLRF/functions132.htm#SQLRF00685` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/conditional_expressions#if` |

### Aggregate functions

The following table shows mappings between common Oracle aggregate, statistical
aggregate, and approximate aggregate functions with their
BigQuery equivalents:

| Oracle | BigQuery |
|---|---|
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/ANY_VALUE.html#GUID-A3C47D5E-B145-40B2-93D2-CA3BA65C2D81` (from Oracle 19c) | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#any_value` |
| `APPROX_COUNT` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/hll_functions set of functions with specified precision ` |
| ` https://docs.oracle.com/database/121/SQLRF/functions013.htm#SQLRF56900` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/approximate_aggregate_functions#approx_count_distinct` |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/APPROX_COUNT_DISTINCT_AGG.html` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/approximate_aggregate_functions#approx_count_distinct` |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/APPROX_COUNT_DISTINCT_DETAIL.html` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/approximate_aggregate_functions#approx_count_distinct` |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/APPROX_PERCENTILE.html(percentile) WITHIN GROUP (ORDER BY expression)` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/approximate_aggregate_functions#approx_quantiles(expression, 100)[ OFFSET(CAST(TRUNC(percentile * 100) as INT64))]` BigQuery doesn't support the rest of arguments that Oracle defines. |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/APPROX_PERCENTILE_AGG.html` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/approximate_aggregate_functions#approx_quantiles(expression, 100)[ OFFSET(CAST(TRUNC(percentile * 100) as INT64))]` |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/APPROX_PERCENTILE_DETAIL.html` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/approximate_aggregate_functions#approx_quantiles(expression, 100)[OFFSET(CAST(TRUNC(percentile * 100) as INT64))]` |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/APPROX_SUM.html` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/approximate_aggregate_functions#approx_top_sum` |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/AVG.html` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#avg` |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/arpls/UTL_RAW.html#GUID-D4AB7615-B960-4290-BB61-E71D30729778` | [bitwise not operator: \~](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/operators) |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/arpls/UTL_RAW.html#GUID-D4AB7615-B960-4290-BB61-E71D30729778` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bit_functions#function_list, https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/operators#bitwise_operators` |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/arpls/UTL_RAW.html#GUID-D4AB7615-B960-4290-BB61-E71D30729778` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bit_functions#function_list, https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/operators#bitwise_operators` |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/BITAND.html` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#bit_and, https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/operators#bitwise_operators` |
| `http://docs.oracle.com/database/121/SQLRF/functions023.htm` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#count` |
| `http://docs.oracle.com/database/121/SQLRF/functions034.htm` | BigQuery doesn't support `TYPE AS TABLE OF`. Consider using `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#string_agg` or `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#string_agg` in BigQuery |
| `http://docs.oracle.com/database/121/SQLRF/functions042.htm/CORR_K/` `CORR_S` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#corr` |
| `http://docs.oracle.com/database/121/SQLRF/functions046.htm` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#count` |
| `http://docs.oracle.com/database/121/SQLRF/functions047.htm` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#covar_pop` |
| `http://docs.oracle.com/database/121/SQLRF/functions048.htm` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#covar_samp` |
| `http://docs.oracle.com/database/121/SQLRF/functions074.htm` | Does not exist implicitly in BigQuery. Consider using [user-defined functions (UDFs)](https://docs.cloud.google.com/bigquery/docs/user-defined-functions). |
| `http://docs.oracle.com/database/121/SQLRF/functions079.htm` | Not used in BigQuery |
| `https://oracle-base.com/articles/misc/rollup-cube-grouping-functions-and-grouping-sets#grouping_functions` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#grouping` |
| `http://docs.oracle.com/database/121/SQLRF/functions081.htm` | Not used in BigQuery. |
| `LAST` | Does not exist implicitly in BigQuery. Consider using [UDFs](https://docs.cloud.google.com/bigquery/docs/user-defined-functions). |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/LISTAGG.html` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#string_agg, https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#array_concat_agg(expression [ORDER BY key [{ASC|DESC}] [, ... ]] [LIMIT n])` |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/MAX.html` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#max` |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/MIN.html` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#min` |
| `OLAP_CONDITION` | Oracle specific, does not exist in BigQuery. |
| `OLAP_EXPRESSION` | Oracle specific, does not exist in BigQuery. |
| `OLAP_EXPRESSION_BOOL` | Oracle specific, does not exist in BigQuery. |
| `OLAP_EXPRESSION_DATE` | Oracle specific, does not exist in BigQuery. |
| `OLAP_EXPRESSION_TEXT` | Oracle specific, does not exist in BigQuery. |
| `OLAP_TABLE` | Oracle specific, does not exist in BigQuery. |
| `POWERMULTISET` | Oracle specific, does not exist in BigQuery. |
| `POWERMULTISET_BY_CARDINALITY` | Oracle specific, does not exist in BigQuery. |
| `QUALIFY` | Oracle specific, does not exist in BigQuery. |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/REGR_-Linear-Regression-Functions.html` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#avg(` `IF(dep_var_expr is NULL` `OR ind_var_expr is NULL,` `NULL, ind_var_expr)` `)` |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/REGR_-Linear-Regression-Functions.html` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#avg(` `IF(dep_var_expr is NULL` `OR ind_var_expr is NULL,` `NULL, dep_var_expr)` `)` |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/REGR_-Linear-Regression-Functions.html` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#sum(` `IF(dep_var_expr is NULL` `OR ind_var_expr is NULL,` `NULL, 1)` `)` |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/REGR_-Linear-Regression-Functions.html` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#avg(dep_var_expr) - AVG(ind_var_expr) * (COVAR_SAMP(ind_var_expr,dep_var_expr) / https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#variance(ind_var_expr) )` |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/REGR_-Linear-Regression-Functions.html` | `(COUNT(dep_var_expr) * https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#sum(ind_var_expr * dep_var_expr) - SUM(dep_var_expr) * SUM(ind_var_expr)) / SQRT( (COUNT(ind_var_expr) * SUM(POWER(ind_var_expr, 2)) * POWER(SUM(ind_var_expr),2)) * (COUNT(dep_var_expr) * SUM(POWER(dep_var_expr, 2)) * POWER(SUM(dep_var_expr), 2))) ` |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/REGR_-Linear-Regression-Functions.html` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#covar_samp(ind_var_expr,` `dep_var_expr)` `/ https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#variance(ind_var_expr)` |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/REGR_-Linear-Regression-Functions.html` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#sum(POWER(ind_var_expr, 2)) - COUNT(ind_var_expr) * POWER(https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#avg(ind_var_expr),2)` |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/REGR_-Linear-Regression-Functions.html` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#sum(ind_var_expr*dep_var_expr) - COUNT(ind_var_expr) * https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#avg(ind) * AVG(dep_var_expr)` |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/REGR_-Linear-Regression-Functions.html` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#sum(POWER(dep_var_expr, 2)) - COUNT(dep_var_expr) * POWER(https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#avg(dep_var_expr),2)` |
| `https://oracle-base.com/articles/misc/rollup-cube-grouping-functions-and-grouping-sets#rollup` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#group_by_clause` |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/STDDEV_POP.html` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#stddev_pop` |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/STDDEV_SAMP.html` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#stddev_samp, https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#stddev` |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/SUM.html` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#sum` |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/VAR_POP.html` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#var_pop` |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/VAR_SAMP.html` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#var_samp, https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#variance` |
| `https://oracle-base.com/articles/misc/string-aggregation-techniques` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#string_agg` |

BigQuery offers the following additional aggregate functions:

- `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#any_value`
- `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/approximate_aggregate_functions#approx_top_count`
- `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#countif`
- `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#logical_and`
- `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#logical_or`

### Analytical functions

The following table shows mappings between common Oracle analytic and aggregate analytic functions with their BigQuery equivalents.

| Oracle | BigQuery |
|---|---|
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/AVG.html` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#avg` |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/arpls/UTL_RAW.html#GUID-D4AB7615-B960-4290-BB61-E71D30729778` | [bitwise not operator: \~](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/operators) |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/arpls/UTL_RAW.html#GUID-D4AB7615-B960-4290-BB61-E71D30729778` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#function_list, https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/operators#bitwise_operators` |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/arpls/UTL_RAW.html#GUID-D4AB7615-B960-4290-BB61-E71D30729778` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#function_list, https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/operators#bitwise_operators` |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/BITAND.html` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#bit_and, https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/operators#bitwise_operators` |
| `BOOL_TO_INT` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/conversion_functions#cast(X AS INT64)` |
| `COUNT` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#count` |
| `http://docs.oracle.com/database/121/SQLRF/functions047.htm` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#covar_pop` |
| `http://docs.oracle.com/database/121/SQLRF/functions048.htm` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#covar_samp` |
| `CUBE_TABLE` | Isn't supported in BigQuery. Consider using a BI tool or a custom UDF |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/CUME_DIST.html`[](https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/CUME_DIST.html) | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/numbering_functions#cume_dist` |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/DENSE_RANK.html(ANSI)` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/numbering_functions#dense_rank` |
| `FEATURE_COMPARE` | Does not exist implicitly in BigQuery. Consider using UDFs and BigQuery ML |
| `FEATURE_DETAILS` | Does not exist implicitly in BigQuery. Consider using UDFs and BigQuery ML |
| `FEATURE_ID` | Does not exist implicitly in BigQuery. Consider using UDFs and BigQuery ML |
| `FEATURE_SET` | Does not exist implicitly in BigQuery. Consider using UDFs and BigQuery ML |
| `FEATURE_VALUE` | Does not exist implicitly in BigQuery. Consider using UDFs and BigQuery ML |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/FIRST_VALUE.html` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/navigation_functions#first_value` |
| `HIER_CAPTION` | Hierarchical queries are not supported in BigQuery. |
| `HIER_CHILD_COUNT` | Hierarchical queries are not supported in BigQuery. |
| `HIER_COLUMN` | Hierarchical queries are not supported in BigQuery. |
| `HIER_DEPTH` | Hierarchical queries are not supported in BigQuery. |
| `HIER_DESCRIPTION` | Hierarchical queries are not supported in BigQuery. |
| `HIER_HAS_CHILDREN` | Hierarchical queries are not supported in BigQuery. |
| `HIER_LEVEL` | Hierarchical queries are not supported in BigQuery. |
| `HIER_MEMBER_NAME` | Hierarchical queries are not supported in BigQuery. |
| `HIER_ORDER` | Hierarchical queries are not supported in BigQuery. |
| `HIER_UNIQUE_MEMBER_NAME` | Hierarchical queries are not supported in BigQuery. |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/LAST_VALUE.html` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/navigation_functions#last_value` |
| `https://docs.oracle.com/cd/B19306_01/server.102/b14200/functions070.htm` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/navigation_functions#lag` |
| `https://docs.oracle.com/cd/B19306_01/server.102/b14200/functions074.htm` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/navigation_functions#lead` |
| `LISTAGG` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#array_agg https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#string_agg https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#array_concat_agg` |
| `https://blogs.oracle.com/datawarehousing/sql-pattern-matching-deep-dive-part-2,-using-matchnumber-and-classifier` | Pattern recognition and calculation can be done with regular expressions and UDFs in BigQuery |
| `https://docs.oracle.com/cd/E16764_01/doc.1111/e12048/pattern_recog.htm` | Pattern recognition and calculation can be done with regular expressions and UDFs in BigQuery |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/MAX.html` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#max` |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/MEDIAN.html` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/navigation_functions#percentile_cont` |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/MIN.html` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#min` |
| `https://docs.oracle.com/cd/E11882_01/server.112/e41084/functions114.htm#SQLRF30031` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/navigation_functions#nth_value (value_expression, constant_integer_expression [{RESPECT | IGNORE} NULLS])` |
| `https://docs.oracle.com/cd/B19306_01/server.102/b14200/functions101.htm` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/numbering_functions#ntile(constant_integer_expression)` |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/PERCENT_RANK.html PERCENT_RANKM` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/numbering_functions#percent_rank` |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/PERCENTILE_CONT.html https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/PERCENTILE_DISC.html` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/navigation_functions#percentile_cont` |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/PERCENTILE_CONT.html https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/PERCENTILE_DISC.html` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/navigation_functions#percentile_disc` |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/PRESENTNNV.html` | Oracle specific, does not exist in BigQuery. |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/PRESENTV.html` | Oracle specific, does not exist in BigQuery. |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/PREVIOUS.html` | Oracle specific, does not exist in BigQuery. |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/RANK.html`(ANSI) | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/numbering_functions#rank` |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/RATIO_TO_REPORT.html(expr) OVER (partition clause)` | `expr / SUM(expr) OVER (partition clause)` |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/ROW_NUMBER.html` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/numbering_functions#row_number` |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/STDDEV_POP.html` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#stddev_pop` |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/STDDEV_SAMP.html` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#stddev_samp, https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#stddev` |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/SUM.html` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#sum` |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/VAR_POP.html` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#var_pop` |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/VAR_SAMP.html` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#var_samp, https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#variance` |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/VARIANCE.html` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#variance()` |
| `https://docs.oracle.com/cd/B19306_01/server.102/b14200/functions214.htm` | UDF can be used. |

### Date/time functions

The following table shows mappings between common Oracle date/time functions and their BigQuery equivalents.

| Oracle | BigQuery |
|---|---|
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/ADD_MONTHS.html(date, integer)` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#date_add(date, INTERVAL integer MONTH), ` If date is a `TIMESTAMP` you can use `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#extract(DATE FROM TIMESTAMP_ADD(date, INTERVAL integer MONTH))` |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/CURRENT_DATE.html` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#current_date` |
| `CURRENT_TIME` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/time_functions#current_time` |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/CURRENT_TIMESTAMP.html` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#current_timestamp` |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/nlspg/datetime-data-types-and-time-zone-support.html#GUID-3A1B7AC6-2EDB-4DDC-9C9D-223D4C72AC74 - k` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#date_sub(date_expression, INTERVAL k DAY)` |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/nlspg/datetime-data-types-and-time-zone-support.html#GUID-3A1B7AC6-2EDB-4DDC-9C9D-223D4C72AC74 + k` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#date_add(date_expression, INTERVAL k DAY)` |
| `DBTIMEZONE` | BigQuery does not support the database time zone. |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/EXTRACT-datetime.html` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#extract, https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#extract` |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/LAST_DAY.html` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#date_sub( https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#date_trunc( https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#date_add( date_expression, INTERVAL 1 MONTH ), MONTH ), INTERVAL 1 DAY )` |
| `https://docs.oracle.com/cd/B19306_01/server.102/b14200/functions079.htm` | BigQuery doesn't support time zone settings. |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/MONTHS_BETWEEN.html` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#date_diff(date_expression, date_expression, MONTH)` |
| `https://docs.oracle.com/cd/B19306_01/server.102/b14200/functions092.htm` | ` DATE(timestamp_expression, time zone) TIME(timestamp, time zone) DATETIME(timestamp_expression, time zone) ` |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/NEXT_DAY.html` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#date_add( https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#date_trunc( date_expression, WEEK(day_value) ), INTERVAL 1 WEEK )` |
| `SYS_AT_TIME_ZONE` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#current_date([time_zone])` |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/SYSDATE.html` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#current_date` |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/SYSTIMESTAMP.html` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#current_timestamp` |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/TO_DATE.html` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#parse_date` |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/TO_TIMESTAMP.html` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#parse_timestamp` |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/TO_TIMESTAMP_TZ.html` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#parse_timestamp` |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/TZ_OFFSET.html` | Isn't supported in BigQuery. Consider using a custom UDF. |
| `WM_CONTAINS` `WM_EQUALS` `WM_GREATERTHAN` `WM_INTERSECTION` `WM_LDIFF` `WM_LESSTHAN` `WM_MEETS` `WM_OVERLAPS` `WM_RDIFF` | Periods are not used in BigQuery. UDFs can be used to compare two periods. |

BigQuery offers the following additional date/time functions:

- `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/datetime_functions#current_datetime`
- `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#date_from_unix_date`
- `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#date_trunc`
- `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/datetime_functions#datetime`
- `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/datetime_functions#datetime_add`
- `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/datetime_functions#datetime_diff`
- `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/datetime_functions#datetime_sub`
- `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/datetime_functions#datetime_trunc`
- `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#format_date`
- `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/datetime_functions#format_datetime`
- `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/time_functions#format_time`

- `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#format_timestamp`

- `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/datetime_functions#parse_datetime`

- `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/time_functions#parse_time`

- `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#string`

- `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/time_functions#time`

- `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/time_functions#time_add`

- `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/time_functions#time_diff`

- `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/time_functions#time_sub`

- `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/time_functions#time_trunc`

- `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#timestamp`

- `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#timestamp_add`

- `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#timestamp_diff`

- `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#timestamp_micros`

- `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#timestamp_millis`

- `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#timestamp_seconds`

- `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#timestamp_sub`

- `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#timestamp_trunc`

- `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#unix_date`

- `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#unix_micros`

- `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#unix_millis`

- `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#unix_seconds`

### String functions

The following table shows mappings between Oracle string functions and their
BigQuery equivalents:

| Oracle | BigQuery |
|---|---|
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/ASCII.html` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#to_code_points` |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/ASCIISTR.html` | BigQuery doesn't support UTF-16 |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/RAWTOHEX.html` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#to_hex` |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/LENGTH.html` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#char_length` |
| `LENGTH` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#character_length` |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/CHR.html` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#code_points_to_string( [mod(numeric_expr, 256)] )` |
| `https://docs.oracle.com/en/database/oracle/oracle-database/19/sqlrf/COLLATION.html#GUID-70A694BA-C1A0-4F5A-9492-58A5943D9BDD` | Doesn't exist in BigQuery. BigQuery doesn't support COLLATE in DML |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/COMPOSE.html` | Custom user-defined function. |
| `CONCAT, (|| operator)` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#concat` |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/DECOMPOSE.html` | Custom user-defined function. |
| `ESCAPE_REFERENCE (UTL_I18N)` | Is not supported in BigQuery. Consider using a user-defined function. |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/INITCAP.html` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#initcap` |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/INSTR.html` | Custom user-defined function. |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/LENGTH.html` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#length` |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/LOWER.html` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#lowerLOWER` |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/LPAD.html` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#lpad` |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/LTRIM.html` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#ltrim` |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/NLS_INITCAP.html` | Custom user-defined function. |
| `https://docs.oracle.com/cd/B12037_01/server.101/b10759/functions088.htm` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#lower` |
| `https://docs.oracle.com/cd/B12037_01/server.101/b10759/functions090.htm` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#upper` |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/NLSSORT.html` | Oracle specific, does not exist in BigQuery. |
| `POSITION` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#strpos` |
| `PRINTBLOBTOCLOB` | Oracle specific, does not exist in BigQuery. |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/REGEXP_COUNT.html` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/array_functions#array_length` |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/REGEXP_INSTR.html` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#strpos(source_string, https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#regexp_extract(source_string, regexp_string))` Note: Returns first occurrence. |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/REGEXP_REPLACE.html` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#regexp_replace` |
| `REGEXP_LIKE` | `IF(https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#regexp_contains,1,0)` |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/REGEXP_SUBSTR.html` | `REGEXP_EXTRACT, REGEXP_EXTRACT_ALL` |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/REPLACE.html` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#replace` |
| `https://docs.oracle.com/en/database/other-databases/nosql-database/21.1/sqlreferencefornosql/reverse_function.html` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#reverse` |
| `RIGHT` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#substr(source_string, -1, length)` |
| `https://docs.oracle.com/database/121/SQLRF/functions173.htm#SQLRF06103` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#rpad` |
| `https://docs.oracle.com/database/121/SQLRF/functions174.htm#SQLRF06104` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#rtrim` |
| `https://docs.oracle.com/database/121/SQLRF/functions181.htm#SQLRF06109` | Isn't supported in BigQuery. Consider using a custom UDF |
| `https://docs.oracle.com/cd/E88353_01/html/E37843/strtok-r-3c.html` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#split(instring, delimiter)[ORDINAL(tokennum)]` `Note: The entire delimiter string argument is used as a single delimiter. The default delimiter is a comma.` |
| `https://docs.oracle.com/database/121/SQLRF/functions196.htm#SQLRF06114` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#substr` |
| `https://docs.oracle.com/cd/B19306_01/server.102/b14200/functions196.htm` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#replace` |
| `https://docs.oracle.com/cd/B19306_01/server.102/b14200/functions197.htm` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#replace` |
| `https://docs.oracle.com/database/121/SQLRF/functions235.htm#SQLRF06149` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#trim` |
| `https://docs.oracle.com/cd/B19306_01/server.102/b14200/functions204.htm` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions` |
| `https://docs.oracle.com/database/121/SQLRF/functions242.htm#SQLRF06155` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#upper` |
| `||` (VERTICAL BARS) | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#concat` |

BigQuery offers the following additional string functions:

- `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#byte_length`
- `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#code_points_to_bytes`
- `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#ends_with`
- `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#from_base32`
- `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#from_base64`
- `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#from_hex`
- `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#normalize`
- `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#normalize_and_casefold`
- `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#repeat`
- `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#safe_convert_bytes_to_string`
- `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#split`
- `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#starts_with`
- `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#strpos`
- `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#to_base32`
- `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#to_base64`
- `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#to_code_points`

### Math functions

The following table shows mappings between Oracle math functions and their BigQuery equivalents.

| Oracle | BigQuery |
|---|---|
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/ABS.html` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#abs` |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/ACOS.html` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#acos` |
| `ACOSH` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#acos` |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/ASIN.html` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#asin` |
| `ASINH` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#asinh` |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/ATAN.html` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#atan` |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/ATAN2.html` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#atan2` |
| `ATANH` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#atanh` |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/CEIL.html` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#ceil` |
| `CEILING` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#ceiling` |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/COS.html` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#cos` |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/COSH.html` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#cosh` |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/EXP.html` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#exp` |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/FLOOR.html` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#floor` |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/GREATEST.html` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#greatest` |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/LEAST.html` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#least` |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/LN.html` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#ln` |
| `https://docs.oracle.com/cd/B19306_01/server.102/b14200/functions078.htm` | use with `ISNULL` |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/LOG.html` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#log` |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/MOD.html (% operator)` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#mod` |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/POWER.html (** operator)` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#power, https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#pow` |
| `DBMS_RANDOM.VALUE` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#rand` |
| `RANDOMBYTES` | Isn't supported in BigQuery. Consider using a custom UDF and RAND function |
| `RANDOMINTEGER` | `CAST(FLOOR(10*RAND()) AS INT64)` |
| `RANDOMNUMBER` | Isn't supported in BigQuery. Consider using a custom UDF and RAND function |
| `https://docs.oracle.com/cd/B19306_01/server.102/b14200/functions133.htm` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#mod` |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/ROUND-date.html` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#round` |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/ROUND_TIES_TO_EVEN-number.html` | `ROUND()` |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/SIGN.html` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#sign` |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/SIN.html` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#sin` |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/SINH.html` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#sinh` |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/SQRT.html` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#sqrt` |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/STANDARD_HASH.html` | `FARM_FINGERPRINT, MD5, SHA1, SHA256, SHA512` |
| `https://docs.oracle.com/cd/B19306_01/server.102/b14200/functions159.htm` | [STDDEV](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#stddev) |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/TAN.html` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#tan` |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/TANH.html` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#tanh` |
| `https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/TRUNC-date.html`[](https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/TRUNC-date.html) | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#trunc` |
| `NVL` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/conditional_expressions#ifnull(expr, 0),https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/conditional_expressions#coalesce(exp, 0)` |

BigQuery offers the following additional math functions:

- `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#div`
- `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#ieee_divide`
- `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#is_inf`
- `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#is_nan`
- `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#log10`
- `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#safe_divide`

### Type conversion functions

The following table shows mappings between Oracle type conversion functions and their BigQuery equivalents.

| Oracle | BigQuery |
|---|---|---|
| `BIN_TO_NUM` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#safe_convert_bytes_to_string` `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/conversion_functions` |
| `BINARY2VARCHAR` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#safe_convert_bytes_to_string` |
| `https://docs.oracle.com/en/database/oracle/oracle-database/19/sqlrf/CAST.html CAST_FROM_BINARY_DOUBLE CAST_FROM_BINARY_FLOAT CAST_FROM_BINARY_INTEGER CAST_FROM_NUMBER CAST_TO_BINARY_DOUBLE CAST_TO_BINARY_FLOAT CAST_TO_BINARY_INTEGER CAST_TO_NUMBER CAST_TO_NVARCHAR2 CAST_TO_RAW >CAST_TO_VARCHAR` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/conversion_functions` |
| `CHARTOROWID` | Oracle specific not needed. |
| `CONVERT` | BigQuery doesn't support character sets. Consider using custom user-defined function. |
| `EMPTY_BLOB` | `BLOB` is not used in BigQuery. |
| `EMPTY_CLOB` | `CLOB` is not used in BigQuery. |
| `FROM_TZ` | Types with time zones are not supported in BigQuery. Consider using a user-defined function and FORMAT_TIMESTAMP |
| `INT_TO_BOOL` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#safe_convert_bytes_to_string` |
| `IS_BIT_SET` | Does not exist implicitly in BigQuery. Consider using UDFs |
| `https://docs.oracle.com/cd/B19306_01/server.102/b14200/functions091.htm` | UDF can be used to get char equivalent of binary |
| `NUMTODSINTERVAL` | `INTERVAL` data type is not supported in BigQuery |
| `NUMTOHEX` | Isn't supported in BigQuery. Consider using a custom UDF and `TO_HEX` function ||
| `NUMTOHEX2` |   |
| `https://docs.oracle.com/en/database/oracle/oracle-database/19/sqlrf/NUMTOYMINTERVAL.html` | `INTERVAL` data type is not supported in BigQuery. |
| `RAW_TO_CHAR` | Oracle specific, does not exist in BigQuery. |
| `RAW_TO_NCHAR` | Oracle specific, does not exist in BigQuery. |
| `RAW_TO_VARCHAR2` | Oracle specific, does not exist in BigQuery. |
| `RAWTOHEX` | Oracle specific, does not exist in BigQuery. |
| `https://docs.oracle.com/en/database/oracle/oracle-database/19/sqlrf/RAWTONHEX.html` | Oracle specific, does not exist in BigQuery. |
| `RAWTONUM` | Oracle specific, does not exist in BigQuery. |
| `RAWTONUM2` | Oracle specific, does not exist in BigQuery. |
| `RAWTOREF` | Oracle specific, does not exist in BigQuery. |
| `https://docs.oracle.com/en/database/oracle/oracle-database/19/sqlrf/REFTOHEX.html` | Oracle specific, does not exist in BigQuery. |
| `REFTORAW` | Oracle specific, does not exist in BigQuery. |
| `https://docs.oracle.com/en/database/oracle/oracle-database/19/sqlrf/ROWIDTOCHAR.html` | `ROWID` is Oracle specific type and does not exist in BigQuery. This value should be represented as string. |
| `https://docs.oracle.com/en/database/oracle/oracle-database/19/sqlrf/ROWIDTONCHAR.html` | `ROWID` is Oracle specific type and does not exist in BigQuery. This value should be represented as string. |
| `https://docs.oracle.com/en/database/oracle/oracle-database/19/sqlrf/SCN_TO_TIMESTAMP.html` | `SCN` is Oracle specific type and does not exist in BigQuery. This value should be represented as timestamp. |
| `TO_ACLID TO_ANYLOB https://docs.oracle.com/en/database/oracle/oracle-database/12.2/sqlrf/TO_APPROX_COUNT_DISTINCT.html https://docs.oracle.com/en/database/oracle/oracle-database/12.2/sqlrf/TO_APPROX_PERCENTILE.html https://docs.oracle.com/en/database/oracle/oracle-database/12.2/sqlrf/TO_BINARY_DOUBLE.html https://docs.oracle.com/en/database/oracle/oracle-database/12.2/sqlrf/TO_BINARY_FLOAT.html https://docs.oracle.com/en/database/oracle/oracle-database/12.2/sqlrf/TO_BLOB-bfile.html https://docs.oracle.com/en/database/oracle/oracle-database/12.2/sqlrf/TO_CHAR-bfile-blob.html https://docs.oracle.com/en/database/oracle/oracle-database/12.2/sqlrf/TO_CLOB-bfile-blob.html https://docs.oracle.com/en/database/oracle/oracle-database/12.2/sqlrf/TO_DATE.html https://docs.oracle.com/en/database/oracle/oracle-database/12.2/sqlrf/TO_DSINTERVAL.html https://docs.oracle.com/en/database/oracle/oracle-database/12.2/sqlrf/TO_LOB.html https://docs.oracle.com/en/database/oracle/oracle-database/12.2/sqlrf/TO_MULTI_BYTE.html https://docs.oracle.com/en/database/oracle/oracle-database/12.2/sqlrf/TO_NCHAR-character.html https://docs.oracle.com/en/database/oracle/oracle-database/12.2/sqlrf/TO_NCLOB.html https://docs.oracle.com/en/database/oracle/oracle-database/12.2/sqlrf/TO_NUMBER.html TO_RAW https://docs.oracle.com/en/database/oracle/oracle-database/12.2/sqlrf/TO_SINGLE_BYTE.html TO_TIME` [TO_TIMESTAMP](https://docs.oracle.com/en/database/oracle/oracle-database/12.2/sqlrf/TO_TIMESTAMP.html) [TO_TIMESTAMP_TZ](https://docs.oracle.com/en/database/oracle/oracle-database/12.2/sqlrf/TO_TIMESTAMP_TZ.html) TO_TIME_TZ TO_UTC_TIMEZONE_TZ [TO_YMINTERVAL](https://docs.oracle.com/en/database/oracle/oracle-database/12.2/sqlrf/TO_YMINTERVAL.html) | `CAST(expr AS typename)` `PARSE_DATE` `PARSE_TIMESTAMP` Cast syntax is used in a query to indicate that the result type of an expression should be converted to some other type. |
| `TREAT` | Oracle specific, does not exist in BigQuery. |
| `VALIDATE_CONVERSION` | Isn't supported in BigQuery. Consider using a custom UDF |
| `VSIZE` | Isn't supported in BigQuery. Consider using a custom UDF |

### JSON functions

The following table shows mappings between Oracle JSON functions and their
BigQuery equivalents.

| Oracle | BigQuery |
|---|---|
| `AS_JSON` | `TO_JSON_STRING(value[, pretty_print])` |
| `https://docs.oracle.com/en/database/oracle/oracle-database/12.2/sqlrf/JSON_ARRAY.html` | Consider using UDFs and `TO_JSON_STRING` function |
| `https://docs.oracle.com/en/database/oracle/oracle-database/12.2/sqlrf/JSON_ARRAYAGG.html` | Consider using UDFs and `TO_JSON_STRING` function |
| `https://docs.oracle.com/en/database/oracle/oracle-database/12.2/sqlrf/JSON_DATAGUIDE.html` | Custom user-defined function. |
| `https://docs.oracle.com/en/database/oracle/oracle-database/18/sqlrf/SQL-JSON-Conditions.html` | Custom user-defined function. |
| `https://docs.oracle.com/en/database/oracle/oracle-database/18/sqlrf/SQL-JSON-Conditions.html` | Consider using UDFs and `JSON_EXTRACT` or `JSON_EXTRACT_SCALAR` |
| `https://docs.oracle.com/en/database/oracle/oracle-database/19/sqlrf/JSON_MERGEPATCH.html` | Custom user-defined function. |
| `https://docs.oracle.com/en/database/oracle/oracle-database/19/sqlrf/JSON_OBJECT.html` | Is not supported by BigQuery. |
| `https://docs.oracle.com/en/database/oracle/oracle-database/19/sqlrf/JSON_OBJECTAGG.html` | Is not supported by BigQuery. |
| `https://docs.oracle.com/en/database/oracle/oracle-database/19/sqlrf/JSON_QUERY.html` | Consider using UDFs and `JSON_EXTRACT` or `JSON_EXTRACT_SCALAR`. |
| `https://docs.oracle.com/en/database/oracle/oracle-database/19/sqlrf/JSON_TABLE.html` | Custom user-defined function. |
| `JSON_TEXTCONTAINS` | Consider using UDFs and `JSON_EXTRACT` or `JSON_EXTRACT_SCALAR`. |
| `https://docs.oracle.com/en/database/oracle/oracle-database/19/sqlrf/JSON_VALUE.html` | `JSON_EXTRACT_SCALAR` |

### XML functions

BigQuery does not provide implicit XML functions. XML can be
loaded to BigQuery as string and UDFs can be used to parse XML.
Alternatively, XML processing be done by an ETL/ELT tool such as
[Dataflow](https://docs.cloud.google.com/dataflow/docs). The following list shows Oracle XML
functions:

| Oracle | BigQuery |
|---|---|
| `DELETEXML` | BigQuery [UDFs]() or ETL tool like Dataflow can be used to process XML. |
| `ENCODE_SQL_XML` | BigQuery [UDFs]() or ETL tool like Dataflow can be used to process XML. |
| `EXISTSNODE` | BigQuery [UDFs]() or ETL tool like Dataflow can be used to process XML. |
| `EXTRACTCLOBXML` | BigQuery [UDFs]() or ETL tool like Dataflow can be used to process XML. |
| `EXTRACTVALUE` | BigQuery [UDFs]() or ETL tool like Dataflow can be used to process XML. |
| `INSERTCHILDXML` | BigQuery [UDFs]() or ETL tool like Dataflow can be used to process XML. |
| `INSERTCHILDXMLAFTER` | BigQuery [UDFs]() or ETL tool like Dataflow can be used to process XML. |
| `INSERTCHILDXMLBEFORE` | BigQuery [UDFs]() or ETL tool like Dataflow can be used to process XML. |
| `INSERTXMLAFTER` | BigQuery [UDFs]() or ETL tool like Dataflow can be used to process XML. |
| `INSERTXMLBEFORE` | BigQuery [UDFs]() or ETL tool like Dataflow can be used to process XML. |
| `SYS_XMLAGG` | BigQuery [UDFs]() or ETL tool like Dataflow can be used to process XML. |
| `SYS_XMLANALYZE` | BigQuery [UDFs]() or ETL tool like Dataflow can be used to process XML. |
| `SYS_XMLCONTAINS` | BigQuery [UDFs]() or ETL tool like Dataflow can be used to process XML. |
| `SYS_XMLCONV` | BigQuery [UDFs]() or ETL tool like Dataflow can be used to process XML. |
| `SYS_XMLEXNSURI` | BigQuery [UDFs]() or ETL tool like Dataflow can be used to process XML. |
| `SYS_XMLGEN` | BigQuery [UDFs]() or ETL tool like Dataflow can be used to process XML. |
| `SYS_XMLI_LOC_ISNODE` | BigQuery [UDFs]() or ETL tool like Dataflow can be used to process XML. |
| `SYS_XMLI_LOC_ISTEXT` | BigQuery [UDFs]() or ETL tool like Dataflow can be used to process XML. |
| `SYS_XMLINSTR` | BigQuery [UDFs]() or ETL tool like Dataflow can be used to process XML. |
| `SYS_XMLLOCATOR_GETSVAL` | BigQuery [UDFs]() or ETL tool like Dataflow can be used to process XML. |
| `SYS_XMLNODEID` | BigQuery [UDFs]() or ETL tool like Dataflow can be used to process XML. |
| `SYS_XMLNODEID_GETLOCATOR` | BigQuery [UDFs]() or ETL tool like Dataflow can be used to process XML. |
| `SYS_XMLNODEID_GETOKEY` | BigQuery [UDFs]() or ETL tool like Dataflow can be used to process XML. |
| `SYS_XMLNODEID_GETPATHID` | BigQuery [UDFs]() or ETL tool like Dataflow can be used to process XML. |
| `SYS_XMLNODEID_GETPTRID` | BigQuery [UDFs]() or ETL tool like Dataflow can be used to process XML. |
| `SYS_XMLNODEID_GETRID` | BigQuery [UDFs]() or ETL tool like Dataflow can be used to process XML. |
| `SYS_XMLNODEID_GETSVAL` | BigQuery [UDFs]() or ETL tool like Dataflow can be used to process XML. |
| `SYS_XMLT_2_SC` | BigQuery [UDFs]() or ETL tool like Dataflow can be used to process XML. |
| `SYS_XMLTRANSLATE` | BigQuery [UDFs]() or ETL tool like Dataflow can be used to process XML. |
| `SYS_XMLTYPE2SQL` | BigQuery [UDFs]() or ETL tool like Dataflow can be used to process XML. |
| `UPDATEXML` | BigQuery [UDFs]() or ETL tool like Dataflow can be used to process XML. |
| `XML2OBJECT` | BigQuery [UDFs]() or ETL tool like Dataflow can be used to process XML. |
| `XMLCAST` | BigQuery [UDFs]() or ETL tool like Dataflow can be used to process XML. |
| `XMLCDATA` | BigQuery [UDFs]() or ETL tool like Dataflow can be used to process XML. |
| `XMLCOLLATVAL` | BigQuery [UDFs]() or ETL tool like Dataflow can be used to process XML. |
| `XMLCOMMENT` | BigQuery [UDFs]() or ETL tool like Dataflow can be used to process XML. |
| `XMLCONCAT` | BigQuery [UDFs]() or ETL tool like Dataflow can be used to process XML. |
| `XMLDIFF` | BigQuery [UDFs]() or ETL tool like Dataflow can be used to process XML. |
| `XMLELEMENT` | BigQuery [UDFs]() or ETL tool like Dataflow can be used to process XML. |
| `XMLEXISTS` | BigQuery [UDFs]() or ETL tool like Dataflow can be used to process XML. |
| `XMLEXISTS2` | BigQuery [UDFs]() or ETL tool like Dataflow can be used to process XML. |
| `XMLFOREST` | BigQuery [UDFs]() or ETL tool like Dataflow can be used to process XML. |
| `XMLISNODE` | BigQuery [UDFs]() or ETL tool like Dataflow can be used to process XML. |
| `XMLISVALID` | BigQuery [UDFs]() or ETL tool like Dataflow can be used to process XML. |
| `XMLPARSE` | BigQuery [UDFs]() or ETL tool like Dataflow can be used to process XML. |
| `XMLPATCH` | BigQuery [UDFs]() or ETL tool like Dataflow can be used to process XML. |
| `XMLPI` | BigQuery [UDFs]() or ETL tool like Dataflow can be used to process XML. |
| `XMLQUERY` | BigQuery [UDFs]() or ETL tool like Dataflow can be used to process XML. |
| `XMLQUERYVAL` | BigQuery [UDFs]() or ETL tool like Dataflow can be used to process XML. |
| `XMLSERIALIZE` | BigQuery [UDFs]() or ETL tool like Dataflow can be used to process XML. |
| `XMLTABLE` | BigQuery [UDFs]() or ETL tool like Dataflow can be used to process XML. |
| `XMLTOJSON` | BigQuery [UDFs]() or ETL tool like Dataflow can be used to process XML. |
| `XMLTRANSFORM` | BigQuery [UDFs]() or ETL tool like Dataflow can be used to process XML. |
| `XMLTRANSFORMBLOB` | BigQuery [UDFs]() or ETL tool like Dataflow can be used to process XML. |
| `XMLTYPE` | BigQuery [UDFs]() or ETL tool like Dataflow can be used to process XML. |
| BigQuery [UDFs]() or ETL tool like Dataflow can be used to process XML. |

### Machine learning functions

Machine learning (ML) functions in Oracle and BigQuery are
different.
Oracle requires advanced analytics pack and licenses to do ML on the database.
Oracle uses the `DBMS_DATA_MINING` package for ML. Converting Oracle data miner
jobs requires rewriting the code to work with BigQuery features.
You can choose from comprehensive
[Google AI offerings](https://cloud.google.com/products/ai/), including the
following products and features:

- [BigQuery AI](https://docs.cloud.google.com/bigquery/docs/ai-introduction)
- [Vertex AI](https://docs.cloud.google.com/vertex-ai/docs/start/introduction-unified-platform)
- [AI APIs for Google Cloud](https://cloud.google.com/ai/apis)

You can use [BigQuery ML](https://docs.cloud.google.com/bigquery/docs/bqml-introduction) or
[Vertex AI development tools](https://docs.cloud.google.com/vertex-ai/docs/general/developer-tools-overview)
to develop, train, and evaluate ML models.

The following table shows Oracle ML functions:

| Oracle | BigQuery |
|---|---|
| `CLASSIFIER` | See [BigQuery ML](https://docs.cloud.google.com/bigquery/docs/bqml-introduction) for machine learning classifier and regression options |
| `CLUSTER_DETAILS` | See [BigQuery ML](https://docs.cloud.google.com/bigquery/docs/bqml-introduction) for machine learning classifier and regression options |
| `CLUSTER_DISTANCE` | See [BigQuery ML](https://docs.cloud.google.com/bigquery/docs/bqml-introduction) for machine learning classifier and regression options |
| `CLUSTER_ID` | See [BigQuery ML](https://docs.cloud.google.com/bigquery/docs/bqml-introduction) for machine learning classifier and regression options |
| `CLUSTER_PROBABILITY` | See [BigQuery ML](https://docs.cloud.google.com/bigquery/docs/bqml-introduction) for machine learning classifier and regression options |
| `CLUSTER_SET` | See [BigQuery ML](https://docs.cloud.google.com/bigquery/docs/bqml-introduction) for machine learning classifier and regression options |
| `PREDICTION` | See [BigQuery ML](https://docs.cloud.google.com/bigquery/docs/bqml-introduction) for machine learning classifier and regression options |
| `PREDICTION_BOUNDS` | See [BigQuery ML](https://docs.cloud.google.com/bigquery/docs/bqml-introduction) for machine learning classifier and regression options |
| `PREDICTION_COST` | See [BigQuery ML](https://docs.cloud.google.com/bigquery/docs/bqml-introduction) for machine learning classifier and regression options |
| `PREDICTION_DETAILS` | See [BigQuery ML](https://docs.cloud.google.com/bigquery/docs/bqml-introduction) for machine learning classifier and regression options |
| `PREDICTION_PROBABILITY` | See [BigQuery ML](https://docs.cloud.google.com/bigquery/docs/bqml-introduction) for machine learning classifier and regression options |
| `PREDICTION_SET` | See [BigQuery ML](https://docs.cloud.google.com/bigquery/docs/bqml-introduction) for machine learning classifier and regression options |

### Security functions

The following table shows the functions for identifying the user in Oracle and
BigQuery:

| Oracle | BigQuery |
|---|---|
| `https://docs.oracle.com/cd/B28359_01/server.111/b28286/functions211.htm#SQLRF06153` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/security_functions` |
| `USER/SESSION_USER/CURRENT_USER` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/security_functions` |

### Set or array functions

The following table shows set or array functions in Oracle and their equivalents
in BigQuery:

| Oracle | BigQuery |
|---|---|
| `https://docs.oracle.com/en/database/oracle/oracle-database/12.2/sqlrf/Multiset-Operators.html` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#array_agg` |
| `https://docs.oracle.com/en/database/oracle/oracle-database/12.2/sqlrf/Multiset-Operators.html` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#array_agg` |
| `https://docs.oracle.com/en/database/oracle/oracle-database/12.2/sqlrf/Multiset-Operators.html` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#array_agg` |
| `https://docs.oracle.com/en/database/oracle/oracle-database/12.2/sqlrf/Multiset-Operators.html` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#array_agg` |

### Window functions

The following table shows window functions in Oracle and their equivalents in BigQuery.

| Oracle | BigQuery |
|---|---|
| `https://docs.oracle.com/en/database/oracle/oracle-database/12.2/sqlrf/LAG.html` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/navigation_functions#lag (value_expression[, offset [, default_expression]])` |
| `https://docs.oracle.com/en/database/oracle/oracle-database/12.2/sqlrf/LEAD.html` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/navigation_functions#lead (value_expression[, offset [, default_expression]])` |

### Hierarchical or recursive queries

[Hierarchical](https://docs.oracle.com/cd/B19306_01/server.102/b14200/queries003.htm)
or recursive queries are not used in BigQuery. If the depth of
the hierarchy is known similar functionality can be achieved with joins, as
illustrated in the following example. Another solution would be to utilize the
[BigQueryStorage API](https://docs.cloud.google.com/bigquery/docs/reference/storage) and
[Spark](http://sqlandhadoop.com/how-to-implement-recursive-queries-in-spark/).

    select
      array(
        select e.update.element
        union all
        select c1 from e.update.element.child as c1
        union all
        select c2 from e.update.element.child as c1, c1.child as c2
        union all
        select c3 from e.update.element.child as c1, c1.child as c2, c2.child as c3
        union all
        select c4 from e.update.element.child as c1, c1.child as c2, c2.child as c3, c3.child as c4
        union all
        select c5 from e.update.element.child as c1, c1.child as c2, c2.child as c3, c3.child as c4, c4.child as c5
      ) as flattened,
      e as event
    from t, t.events as e

The following table shows hierarchical functions in Oracle.

| Oracle | BigQuery |
|---|---|
| `DEPTH` | Hierarchical queries are not used in BigQuery. |
| `PATH` | Hierarchical queries are not used in BigQuery. |
| `SYS_CONNECT_BY_PATH (hierarchical)` | Hierarchical queries are not used in BigQuery. |

### UTL functions

[`UTL_File`](https://docs.oracle.com/database/121/ARPLS/u_file.htm#ARPLS069)
package is mainly used for reading and writing the operating system files from
PL/SQL. Cloud Storage can be used for any kind of raw file staging.
[External tables](https://docs.cloud.google.com/bigquery/external-table-definition) and BigQuery
[load](https://docs.cloud.google.com/bigquery/docs/loading-data) and [export](https://docs.cloud.google.com/bigquery/docs/exporting-data)
should be used to read and write files from and to Cloud Storage. For
more information, see
[Introduction to external data sources](https://docs.cloud.google.com/bigquery/external-data-sources).

### Spatial functions

You can use BigQuery geospatial analytics to replace spatial
functionality. There
are `SDO_*` functions and types in Oracle such as `SDO_GEOM_KEY`,
`SDO_GEOM_MBR`, `SDO_GEOM_MMB`. These functions are used for spatial
analysis. You can use [geospatial analytics](https://docs.cloud.google.com/bigquery/docs/geospatial-intro)
to do spatial analysis.

## DML syntax

This section addresses differences in data management language syntax between
Oracle and BigQuery.

### `INSERT` statement

Most Oracle `INSERT` statements are compatible with BigQuery. The
following table shows exceptions.

DML scripts in BigQuery have slightly different consistency
semantics than the equivalent statements in Oracle. For an overview of snapshot
isolation and session and transaction handling, see the [`CREATE [UNIQUE] INDEX
section`](https://docs.cloud.google.com/bigquery/docs/migration/oracle-sql#create-index) elsewhere in this document.

| Oracle | BigQuery |
|---|---|
| `https://docs.oracle.com/en/database/oracle/oracle-database/12.2/sqlrf/INSERT.html table VALUES (...);` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/dml-syntax#insert_statement table (...) VALUES (...);` Oracle offers a `DEFAULT` keyword for non-nullable columns. Note: In BigQuery, omitting column names in the `INSERT` statement only works if values for all columns in the target table are included in ascending order based on their ordinal positions. |
| `https://docs.oracle.com/en/database/oracle/oracle-database/12.2/sqlrf/INSERT.html table VALUES (1,2,3); INSERT INTO table VALUES (4,5,6); INSERT INTO table VALUES (7,8,9); INSERT ALL INTO table (col1, col2) VALUES ('val1_1', 'val1_2') INTO table (col1, col2) VALUES ('val2_1', 'val2_2') INTO table (col1, col2) VALUES ('val3_1', 'val3_2') . . . SELECT 1 FROM DUAL;` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/dml-syntax#insert_statement table VALUES (1,2,3), (4,5,6), (7,8,9);` BigQuery imposes [DML quotas](https://docs.cloud.google.com/bigquery/quotas#data-manipulation-language-statements), which restrict the number of DML statements you can execute daily. To make the best use of your quota, consider the following approaches: - Combine multiple rows in a single `INSERT` statement, instead of one row per `INSERT` operation. - Combine multiple DML statements (including `INSERT`) using a `MERGE` statement. - Use `CREATE TABLE ... AS SELECT` to create and populate new tables. |

### `UPDATE` statement

Oracle `UPDATE` statements are mostly compatible with BigQuery,
however, in BigQuery the `UPDATE` statement must have a `WHERE`
clause.

As a best practice, you should prefer batch DML statements over multiple single
`UPDATE` and `INSERT` statements. DML scripts in BigQuery have
slightly different consistency semantics than equivalent statements in Oracle.
For an overview on snapshot isolation and session and transaction handling see
the [`CREATE INDEX`](https://docs.cloud.google.com/bigquery/docs/migration/oracle-sql#create-index) section in this document.

The following table shows Oracle `UPDATE` statements and BigQuery
statements that accomplish the same tasks.

In BigQuery the `UPDATE` statement must have a `WHERE` clause.
For more information about `UPDATE` in BigQuery, see the
[BigQuery UPDATE examples](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/dml-syntax#update_examples)
in the DML documentation.

### `DELETE` and `TRUNCATE` statements

The `DELETE` and `TRUNCATE` statements are both ways to remove rows from a table
without affecting the table schema. `TRUNCATE` is not used in BigQuery.
However, you can use `DELETE` statements to achieve the same effect.

In BigQuery, the `DELETE` statement must have a `WHERE` clause.
For more information about `DELETE` in BigQuery, see the
[BigQuery `DELETE` examples](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/dml-syntax#delete_examples)
in the DML documentation.

| Oracle | BigQuery |
|---|---|
| `https://docs.oracle.com/en/database/oracle/oracle-database/12.2/sqlrf/DELETE.html# database.table;` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/dml-syntax#delete_statement FROM table WHERE TRUE;` |

### `MERGE` statement

The `MERGE` statement can combine `INSERT`, `UPDATE`, and `DELETE` operations
into a single `UPSERT` statement and perform the operations atomically. The
`MERGE` operation must match, at most, one source row for each target row.
BigQuery and Oracle both follow ANSI Syntax.

However, DML scripts in BigQuery have slightly different
consistency semantics than the equivalent statements in Oracle.

## DDL syntax

This section addresses differences in data definition language syntax between
Oracle and BigQuery.

### `CREATE TABLE` statement

Most Oracle [`CREATE TABLE`](https://docs.oracle.com/cd/B28359_01/server.111/b28310/tables003.htm#ADMIN01503)
statements are compatible with BigQuery, except for the following
constraints and syntax elements, which are not used in BigQuery:

- `STORAGE`
- `TABLESPACE`
- `DEFAULT`
- `GENERATED ALWAYS AS`
- `ENCRYPT`
- `PRIMARY KEY (col, ...)`. For more information, see [`CREATE INDEX`](https://docs.cloud.google.com/bigquery/docs/migration/oracle-sql#create-index).
- `UNIQUE INDEX`. For more information, see [`CREATE INDEX`](https://docs.cloud.google.com/bigquery/docs/migration/oracle-sql#create-index).
- `CONSTRAINT..REFERENCES`
- `DEFAULT`
- `PARALLEL`
- `COMPRESS`

For more information about `CREATE TABLE` in BigQuery,
see the [BigQuery `CREATE TABLE`
examples](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create-table-examples).

#### Column options and attributes

Identity columns are introduced with Oracle 12c version which enables
auto-increment on a column. This is not used in BigQuery, this
can be achieved with the following batch way. For more information about surrogate
keys and slowly changing dimensions (SCD), refer to the following guides:

- [BigQuery Surrogate Keys](https://medium.com/google-cloud/bigquery-surrogate-keys-672b2e110f80)
- [BigQuery and surrogate keys: A practical approach](https://cloud.google.com/blog/products/data-analytics/bigquery-and-surrogate-keys-practical-approach)

| Oracle | BigQuery |
|---|---|
| `https://docs.oracle.com/en/database/oracle/oracle-database/12.2/sqlrf/CREATE-TABLE.html table ( id NUMBER GENERATED ALWAYS AS IDENTITY, description VARCHAR2(30) );` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/dml-syntax#insert_statement dataset.table SELECT *, ROW_NUMBER() OVER () AS id FROM dataset.table` |

#### Column comments

Oracle uses `Comment` syntax to add comments on columns. This feature can be
similarly implemented in BigQuery using the column description as
shown in the following table:

| Oracle | BigQuery |
|---|---|
| `Comment on column table is 'column desc';` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_table_statement dataset.table ( col1 STRING https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#column_option_list(description="column desc") );` |

#### Temporary tables

Oracle supports [temporary](https://docs.oracle.com/cd/B28359_01/server.111/b28310/tables003.htm#ADMIN11633)
tables, which are often used to store intermediate results in scripts.
[Temporary](https://docs.cloud.google.com/bigquery/docs/multi-statement-queries#temporary_tables)
tables are supported in BigQuery.

| Oracle | BigQuery |
|---|---|
| `https://docs.oracle.com/en/database/oracle/oracle-database/12.2/sqlrf/CREATE-TABLE.html temp_tab (x INTEGER, y VARCHAR2(50)) ON COMMIT DELETE ROWS; COMMIT;` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_table_statement temp_tab ( x INT64, y STRING ); DELETE FROM temp_tab WHERE TRUE;` |

The following Oracle elements are not used in BigQuery:

- `ON COMMIT DELETE ROWS;`
- `ON COMMIT PRESERVE ROWS;`

There are also some other ways to emulate temporary tables in BigQuery:

- **Dataset TTL:** Create a dataset that has a short time to live (for example, one hour) so that any tables created in the dataset are effectively temporary (since they won't persist longer than the dataset's time to live). You can prefix all the table names in this dataset with `temp` to clearly denote that the tables are temporary.
- **Table TTL:** Create a table that has a table-specific short time to live
  using DDL statements similar to the following:

  ```
  CREATE TABLE temp.name (col1, col2, ...)

  OPTIONS(expiration_timestamp=TIMESTAMP_ADD(CURRENT_TIMESTAMP(), INTERVAL 1 HOUR));
  ```
- **`WITH` clause:** If a temporary table is needed only within the same
  block, use a temporary result using a [`WITH`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#with_clause)
  statement or subquery.

### `CREATE SEQUENCE` statement

Sequences are not used in BigQuery, this can be achieved with the
following batch way. For more information about surrogate keys and slowly
changing dimensions (SCD), refer to the following guides:

- [BigQuery Surrogate Keys](https://medium.com/google-cloud/bigquery-surrogate-keys-672b2e110f80)
- [BigQuery and surrogate keys: A practical approach](https://cloud.google.com/blog/products/data-analytics/bigquery-and-surrogate-keys-practical-approach)

```
INSERT INTO dataset.table
    SELECT *,
      ROW_NUMBER() OVER () AS id
      FROM dataset.table
```

### `CREATE VIEW` statement

The following table shows equivalents between Oracle and BigQuery
for the `CREATE VIEW` statement.

| Oracle | BigQuery | Notes |
|---|---|---|
| `https://docs.oracle.com/en/database/oracle/oracle-database/12.2/sqlrf/CREATE-VIEW.html view_name AS SELECT ...` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_view_statementview_name AS SELECT ...` |   |
| `https://docs.oracle.com/en/database/oracle/oracle-database/12.2/sqlrf/CREATE-VIEW.htmlview_name AS SELECT ...` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_view_statement` `view_name AS ` `SELECT ...` |   |
| Not supported | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_view_statement` `view_name` `OPTIONS(https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#view_option_list)` `AS SELECT ...` | Creates a new view only if the view does not currently exist in the specified dataset. |

### `CREATE MATERIALIZED VIEW` statement

In BigQuery materialized view refresh operations are done
automatically. There is no need to specify refresh options (for example, on
commit or on schedule) in BigQuery. For more information, see
[Introduction to materialized views](https://docs.cloud.google.com/bigquery/docs/materialized-views-intro).

In case the base table keeps changing by appends only, the query that uses
materialized view (whether view is explicitly referenced or selected by the
query optimizer) scans all materialized view plus a delta in the base table
since the last view refresh. This means queries are faster and cheaper.

On the contrary, if there were any updates (DML UPDATE / MERGE) or deletions
(DML DELETE, truncation, partition expiration) in the base table since the last
view refresh, the materialized view are not be scanned and hence query don't
get any savings until the next view refresh. Basically, any update or deletion
in the base table invalidates the materialized view state.

Also, the data from the streaming buffer of the base table is not saved into
materialized view. Streaming buffer is still being scanned fully regardless of
whether materialized view is used.

The following table shows equivalents between Oracle and BigQuery
for the `CREATE MATERIALIZED VIEW` statement.

| Oracle | BigQuery | Notes |
|---|---|---|
| `https://docs.oracle.com/en/database/oracle/oracle-database/12.2/sqlrf/CREATE-MATERIALIZED-VIEW.htmlview_name REFRESH FAST NEXT sysdate + 7 AS SELECT ... FROM TABLE_1` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_view_statement view_name AS SELECT ...` |   |

### `CREATE [UNIQUE] INDEX` statement

This section describes approaches in BigQuery for how to create
functionality similar to indexes in Oracle.

#### Indexing for performance

BigQuery doesn't need explicit indexes, because it's a
column-oriented database with query and storage optimization.
BigQuery provides functionality such as [partitioning and
clustering](https://docs.cloud.google.com/bigquery/docs/clustered-tables#combine-clustered-partitioned-tables) as
well as [nested fields](https://docs.cloud.google.com/bigquery/docs/nested-repeated), which can increase
query efficiency and performance by optimizing how data is stored.

#### Indexing for consistency (UNIQUE, PRIMARY INDEX)

In Oracle, a unique index can be used to prevent rows with non-unique keys in a
table. If a process tries to insert or update data that has a value that's
already in the index the operation fails with an index violation.

Because BigQuery doesn't provide explicit indexes, a `MERGE`
statement can be used instead to insert only unique records into a target table
from a staging table while discarding duplicate records. However, there is no
way to prevent a user with edit permissions from inserting a duplicate record.

To generate an error for duplicate records in BigQuery you can
use a `MERGE` statement from the staging table, as shown in the following
example:

| Oracle || BigQuery |
|---|---|---|
| `https://docs.oracle.com/en/database/oracle/oracle-database/12.2/sqlrf/CREATE-INDEX.htmlname;` || ``https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/dml-syntax#merge_statement `prototype.FIN_MERGE` t \ USING `prototype.FIN_TEMP_IMPORT` m \ ON t.col1 = m.col1 \ AND t.col2 = m.col2 \ WHEN MATCHED THEN \ UPDATE SET t.col1 = ERROR(CONCAT('Encountered Error for ', m.col1, ' ', m.col2)) \ WHEN NOT MATCHED THEN \ INSERT (col1,col2,col3,col4,col5,col6,col7,col8) VALUES(col1,col2,col3,col4,col5,col6, CURRENT_TIMESTAMP(),CURRENT_TIMESTAMP());`` |

More often, users prefer to [remove duplicates independently](https://docs.cloud.google.com/bigquery/streaming-data-into-bigquery#manually_removing_duplicates) in order to find errors in downstream systems.

BigQuery does not support `DEFAULT` and `IDENTITY` (sequences) columns.

### Locking

BigQuery doesn't have a lock mechanism like Oracle and can run
concurrent queries (up to your [quota](https://docs.cloud.google.com/bigquery/quotas)). Only DML statements
have certain [concurrency limits](https://docs.cloud.google.com/bigquery/quotas#data-manipulation-language-statements)
and might require a [table lock during execution](https://docs.cloud.google.com/bigquery/docs/data-manipulation-language#dml-limitations)
in some scenarios.

## Procedural SQL statements

This section describes how to convert procedural SQL statements used in stored
procedures, functions and triggers from Oracle to BigQuery.

### `CREATE PROCEDURE` statement

Stored Procedure is supported as part of BigQuery Scripting Beta.

| Oracle | BigQuery | Notes |
|---|---|---|
| `https://docs.oracle.com/cd/B19306_01/server.102/b14200/statements_6009.htm` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_procedure` | Similar to Oracle, BigQuery supports `IN, OUT, INOUT` argument modes. Other syntax specifications are not supported in BigQuery. |
| `https://docs.oracle.com/cd/B19306_01/server.102/b14200/statements_6009.htm` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_procedure` |   |
| `https://docs.oracle.com/cd/B28359_01/server.111/b28286/statements_4008.htm#SQLRF01108` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/scripting#call` |   |

The sections that follow describe ways to convert existing Oracle procedural
statements to BigQuery scripting statements that have similar
functionality.

### `CREATE TRIGGER` statement

Triggers are not used in BigQuery. Row based application logic
should be handled on the application layer. Trigger functionality can be
achieved utilising the ingestion tool, Pub/Sub and/or Cloud Run functions
during the ingestion time or utilising regular scans.

### Variable declaration and assignment

The following table shows Oracle `DECLARE`statements and their
BigQuery equivalents.

| Oracle | BigQuery |
|---|---|
| `DECLARE L_VAR NUMBER; BEGIN L_VAR := 10 + 20; END;` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/scripting L_VAR int64; BEGIN SET L_VAR = 10 + 20; SELECT L_VAR; END` |
| `SET var = value;` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/scripting#set var = value;` |

### Cursor declarations and operations

BigQuery does not support cursors, so the following statements are not used in BigQuery:

- `https://docs.oracle.com/cd/B19306_01/appdev.102/b14261/cursor_declaration.htm [FOR | WITH] ...`
- `https://docs.oracle.com/cd/B19306_01/appdev.102/b14261/openfor_statement.htm#i35231 sql_str;`
- `https://docs.oracle.com/cd/B19306_01/appdev.102/b14261/open_statement.htm#i35173 cursor_name [USING var, ...];`
- `https://docs.oracle.com/cd/B19306_01/appdev.102/b14261/fetch_statement.htm#i34221 cursor_name INTO var, ...;`
- `https://docs.oracle.com/cd/B19306_01/appdev.102/b14261/close_statement.htm#i32987 cursor_name;`

### Dynamic SQL statements

The following Oracle Dynamic SQL statement and its BigQuery
equivalent:

| Oracle | BigQuery |
|---|---|
| `https://docs.oracle.com/cd/B19306_01/appdev.102/b14261/executeimmediate_statement.htm#i33888 sql_str \[USING IN OUT \[, ...\]\]; ` | `https://docs.oracle.com/cd/B19306_01/appdev.102/b14261/executeimmediate_statement.htm#i33888 sql_expression \[INTO variable\[, ...\]\] \[USING identifier\[, ...\]\]; ;` |

### Flow-of-control statements

The following table shows Oracle flow-of-control statements and their BigQuery equivalents.

| Oracle | BigQuery |
|---|---|
| `https://docs.oracle.com/database/121/LNPLS/controlstatements.htm#LNPLS386 condition THEN [if_statement_list] [https://docs.oracle.com/database/121/LNPLS/controlstatements.htm#LNPLS388 else_statement_list ] END IF;` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/conditional_expressions#if condition THEN [if_statement_list] [ELSE else_statement_list ] END IF;` |
| `SET SERVEROUTPUT ON; DECLARE x INTEGER DEFAULT 0; y INTEGER DEFAULT 0; BEGIN LOOP IF x>= 10 THEN EXIT; https://docs.oracle.com/database/121/LNPLS/controlstatements.htm#LNPLS391 x>= 5 THEN y := 5; END IF; x := x + 1; END LOOP; dbms_output.put_line(x||','||y); END; /` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/procedural-language#declare x INT64 DEFAULT 0; DECLARE y INT64 DEFAULT 0; LOOP IF x>= 10 THEN LEAVE; ELSE IF x>= 5 THEN SET y = 5; END IF; END IF; SET x = x + 1; END LOOP; SELECT x,y;` |
| `https://docs.oracle.com/database/121/LNPLS/controlstatements.htm#LNPLS399 sql_statement_list END LOOP;` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/procedural-language#loop sql_statement_list END LOOP;` |
| `https://docs.oracle.com/database/121/LNPLS/controlstatements.htm#LNPLS410 boolean_expression DO sql_statement_list END WHILE;` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/procedural-language#while boolean_expression DO sql_statement_list END WHILE;` |
| `https://docs.oracle.com/database/121/LNPLS/controlstatements.htm#LNPLS411` | `FOR LOOP` is not used in BigQuery. Use other `LOOP` statements. |
| `BREAK` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/procedural-language#break` |
| `https://docs.oracle.com/database/121/LNPLS/controlstatements.htm#LNPLS-GUID-F8B243D3-F7B4-4278-B262-283C521D600C` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/procedural-language#continue` |
| `https://docs.oracle.com/database/121/LNPLS/controlstatements.htm#LNPLS406` | Use `CONTINUE` with `IF` condition. |
| `https://docs.oracle.com/database/121/LNPLS/controlstatements.htm#LNPLS428` | `GOTO` statement does not exist in BigQuery. Use `IF` condition. |

## Metadata and transaction SQL statements

| Oracle | BigQuery |
|---|---|
| `https://docs.oracle.com/cd/B19306_01/server.102/b14211/stats.htm#i37048` | Not used in BigQuery yet. |
| `https://docs.oracle.com/cd/B19306_01/server.102/b14200/statements_9015.htm table_name IN [SHARE/EXCLUSIVE] MODE NOWAIT; ` | Not used in BigQuery yet. |
| `Alter session set isolation_level=serializable; /` `https://docs.oracle.com/cd/B28359_01/server.111/b28286/statements_10005.htm#SQLRF01705 ...` | BigQuery always uses Snapshot Isolation. For details, see [Consistency guarantees and transaction isolation](https://docs.cloud.google.com/bigquery/docs/migration/oracle-sql#consistency_guarantees_and_transaction_isolation) in this document. |
| `https://docs.oracle.com/cd/B19306_01/server.102/b14211/ex_plan.htm#g42231 ...` | Not used in BigQuery. Similar features are the [query plan explanation in the BigQuery web UI](https://docs.cloud.google.com/bigquery/query-plan-explanation) and the slot allocation, and in [audit logging in Stackdriver](https://docs.cloud.google.com/bigquery/docs/monitoring). |
| `SELECT * FROM DBA_[*];` (Oracle DBA_/ALL_/V$ views) | `SELECT * FROM mydataset.INFORMATION_SCHEMA.TABLES;` For more information, see [Introduction to BigQuery INFORMATION_SCHEMA](https://docs.cloud.google.com/bigquery/docs/information-schema-intro). |
| `SELECT * FROM https://docs.oracle.com/cd/B19306_01/server.102/b14237/dynviews_2088.htm#REFRN30223;` `SELECT * FROM https://docs.oracle.com/cd/B19306_01/server.102/b14237/dynviews_1007.htm#REFRN30299;` | BigQuery does not have the traditional session concept. You can view query jobs in the UI or export stackdriver audit logs to BigQuery and analyze BigQuery logs for analyzing jobs. For more information, see [View job details](https://docs.cloud.google.com/bigquery/docs/managing-jobs#view-job). |
| `START TRANSACTION;` `LOCK TABLE table_A IN EXCLUSIVE MODE NOWAIT;` `DELETE FROM table_A;` `INSERT INTO table_A SELECT * FROM table_B;` `COMMIT;` | Replacing the contents of a table with query output is the equivalent of a transaction. You can do this with either a [query](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#bq_query) or a [copy](https://docs.cloud.google.com/bigquery/docs/managing-tables#copy-table) operation. Using a query: `bq query --replace --destination_table table_A 'SELECT * FROM table_B';` Using a copy: `bq cp -f table_A table_B` |

### Multi-statement and multi-line SQL statements

Both Oracle and BigQuery support transactions (sessions)
and therefore support statements separated by semicolons that are consistently
executed together. For more information, see
[Multi-statement transactions](https://docs.cloud.google.com/bigquery/docs/transactions).

## Error codes and messages

[Oracle error codes](https://docs.oracle.com/database/121/DRDAS/error_code.htm#DRDAS513)
and [BigQuery error codes](https://docs.cloud.google.com/bigquery/troubleshooting-errors) are
different. If your application logic is currently catching the errors, try to
eliminate the source of the error, because BigQuery doesn't
return the same error codes.

## Consistency guarantees and transaction isolation

Both Oracle and BigQuery are atomic---that is, ACID-compliant on a
per-mutation level across many rows. For example, a `MERGE` operation is
atomic, even with multiple inserted and updated values.

### Transactions

Oracle provides read committed or serializable [transaction isolation
levels](https://blogs.oracle.com/oraclemagazine/on-transaction-isolation-levels).
Deadlocks are possible. Oracle insert append jobs run independently.

BigQuery also
[supports transactions](https://docs.cloud.google.com/bigquery/docs/transactions).
BigQuery helps ensure
[optimistic concurrency control](https://en.wikipedia.org/wiki/Optimistic_concurrency_control)
(first to commit wins) with
[snapshot isolation](https://en.wikipedia.org/wiki/Snapshot_isolation),
in which a query reads the last committed data before the query starts. This
approach guarantees the same level of consistency on a per-row, per-mutation
basis and across rows within the same DML statement, yet avoids deadlocks. In
the case of multiple `UPDATE` statements against the same table, BigQuery
switches to
[pessimistic concurrency control](https://docs.cloud.google.com/bigquery/docs/data-manipulation-language#dml-limitations)
and [queues](https://cloud.google.com/blog/products/data-analytics/dml-without-limits-now-in-bigquery)
multiple `UPDATE` statements, automatically retrying in case of conflicts. `INSERT` DML statements and
load jobs can run concurrently and independently to append to tables.

### Rollback

Oracle supports [rollbacks](https://docs.oracle.com/cd/B19306_01/server.102/b14200/statements_9021.htm). As there is no explicit transaction boundary in BigQuery, there
is no concept of an explicit rollback in BigQuery. The
workarounds are [table decorators](https://docs.cloud.google.com/bigquery/docs/table-decorators) or
using [`FOR SYSTEM_TIME AS OF`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#for_system_time_as_of).

## Database limits

Check [BigQuery latest quotas and limits](https://docs.cloud.google.com/bigquery/quotas). Many
quotas for large-volume users can be raised by contacting the Cloud Customer Care.
The following table shows a comparison of the Oracle and BigQuery
database limits.

| Limit | Oracle | BigQuery |
|---|---|---|
| Tables per database | Unrestricted | Unrestricted |
| Columns per table | 1000 | 10,000 |
| Maximum row size | Unlimited (Depends on the column type) | 100 MB |
| Column and table name length | If v12.2\>= 128 Bytes Else 30 Bytes | 16,384 Unicode characters |
| Rows per table | Unlimited | Unlimited |
| Maximum SQL request length | Unlimited | 1 MB (maximum unresolved GoogleSQL query length) 12 MB (maximum resolved legacy and GoogleSQL query length) Streaming: - 10 MB (HTTP request size limit) - 10,000 (maximum rows per request) |
| Maximum request \& response size | Unlimited | 10 MB (request) and 10 GB (response), or virtually unlimited if you use pagination or the Cloud Storage API. |
| Maximum number of concurrent sessions | Limited by the sessions or processes parameters | 100 concurrent queries (can be raised with [slot reservation](https://docs.cloud.google.com/bigquery/docs/slots)), 300 concurrent API requests per user. |
| Maximum number of concurrent (fast) loads | Limited by the sessions or processes parameters | No concurrency limit; jobs are queued. 100,000 load jobs per project per day. |

Other Oracle Database limits includes [data type limits](https://docs.oracle.com/en/database/oracle/oracle-database/19/refrn/datatype-limits.html#GUID-963C79C9-9303-49FE-8F2D-C8AAF04D3095), [physical database limits](https://docs.oracle.com/en/database/oracle/oracle-database/19/refrn/physical-database-limits.html#GUID-939CB455-783E-458A-A2E8-81172B990FE9), [logical database limits](https://docs.oracle.com/en/database/oracle/oracle-database/19/refrn/logical-database-limits.html#GUID-685230CF-63F5-4C5A-B8B0-037C566BDA76) and [process and runtime limits](https://docs.oracle.com/en/database/oracle/oracle-database/19/refrn/process-and-runtime-limits.html#GUID-213CC210-4B96-420C-B5B8-3A217F17FC2C).