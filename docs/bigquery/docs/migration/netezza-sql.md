# IBM Netezza SQL translation guide

IBM Netezza data warehousing is designed to work with Netezza-specific SQL
syntax. Netezza SQL is based on Postgres 7.2. SQL scripts written for Netezza
can't be used in a BigQuery data warehouse without alterations,
because the SQL dialects vary.

This document details the similarities and differences in SQL syntax between
Netezza and BigQuery in the following areas:

- Data types
- SQL language elements
- Query syntax
- Data manipulation language (DML)
- Data definition language (DDL)
- Stored procedures
- Functions

You can also use
[batch SQL translation](https://docs.cloud.google.com/bigquery/docs/batch-sql-translator)
to migrate your SQL scripts in bulk, or
[interactive SQL translation](https://docs.cloud.google.com/bigquery/docs/interactive-sql-translator)
to translate ad-hoc queries. IBM Netezza SQL/NZPLSQL is supported by both
tools in [preview](https://cloud.google.com/products#product-launch-stages).

## Data types

| **Netezza** | **BigQuery** | **Notes** |
|---|---|---|
| `INTEGER/INT/INT4` | [`INT64`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#integer_types) |   |
| `SMALLINT/INT2` | [`INT64`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#integer_types) |   |
| `BYTEINT/INT1` | [`INT64`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#integer_types) |   |
| `BIGINT/INT8` | [`INT64`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#integer_types) |   |
| `DECIMAL` | [`NUMERIC`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#numeric-type) | The `DECIMAL` data type in Netezza is an alias for the `NUMERIC` data type. |
| `NUMERIC`\] | [`NUMERIC`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#numeric-type) [`INT64`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#integer_types) |   |
| `NUMERIC(p,s)` | [`NUMERIC`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#numeric-type) | The `NUMERIC` type in BigQuery does not enforce custom digit or scale bounds (constraints) like Netezza does. BigQuery has fixed 9 digits after the decimal, while Netezza allows a custom setup. In Netezza, precision `p` can range from 1 to 38, and scale `s` from 0 to the precision. |
| `FLOAT(p)` | [`FLOAT64`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#floating_point_types) |   |
| `REAL/FLOAT(6)` | [`FLOAT64`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#floating_point_types) |   |
| `DOUBLE PRECISION/FLOAT(14)` | [`FLOAT64`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#floating_point_types) |   |
| `CHAR/CHARACTER` | [`STRING`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#string) | The `STRING` type in BigQuery is variable-length and does not require manually setting a max character length as the Netezza `CHARACTER` and `VARCHAR` types require. The default value of `n` in `CHAR(n)` is 1. The maximum character string size is 64,000. |
| `VARCHAR` | [`STRING`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#string) | The `STRING` type in BigQuery is variable-length and does not require manually setting a max character length as the Netezza `CHARACTER` and `VARCHAR` types require. The maximum character string size is 64,000. |
| `NCHAR` | [`STRING`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#string) | The `STRING` type in BigQuery is stored as variable length UTF-8 encoded Unicode. The maximum length is 16,000 characters. |
| `NVARCHAR` | [`STRING`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#string) | The `STRING` type in BigQuery is stored as variable-length UTF-8-encoded Unicode. The maximum length is 16,000 characters. |
| `VARBINARY` | [`BYTES`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#bytes_type) |   |
| `ST_GEOMETRY` | [`GEOGRAPHY`](https://docs.cloud.google.com/bigquery/docs/gis-data) |   |
| `BOOLEAN/BOOL` | [`BOOL`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#boolean_type) | The `BOOL` type in BigQuery can only accept `TRUE/FALSE`, unlike the `BOOL` type in Netezza, which can accept a variety of values like `0/1`, `yes/no`, `true/false,` `on/off`. |
| `DATE` | [`DATE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#date_type) |   |
| `TIME` | [`TIME`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#time_type) |   |
| `TIMETZ/TIME WITH TIME ZONE` | [`TIME`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#time_type) | Netezza stores the `TIME` data type in UTC and lets you pass an offset from UTC using the `WITH TIME ZONE` syntax. The `TIME` data type in BigQuery represents a time that's independent of any date or time zone. |
| `TIMESTAMP` | [`DATETIME`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#datetime_type) | The Netezza `TIMESTAMP` type does not include a time zone, the same as the BigQuery `DATETIME` type. |
|   | [`ARRAY`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#array_type) | There is no array data type in Netezza. The array type is instead stored in a varchar field. |

## Timestamp and date type formatting

When you convert date type formatting elements from Netezza to
GoogleSQL, you must pay particular attention to time zone
differences between `TIMESTAMP` and `DATETIME`, as summarized in the
following table:

| **Netezza** | **BigQuery** |
|---|---|
| `CURRENT_TIMESTAMP` `CURRENT_TIME` <br /> `TIME` information in Netezza can have different time zone information, which is defined using the `WITH TIME ZONE` syntax. | If possible, use the `CURRENT_TIMESTAMP` function, which is formatted correctly. However, the output format does not always show the UTC time zone (internally, BigQuery does not have a time zone). The `DATETIME` object in the bq command-line tool and Google Cloud console is formatted using a `T` separator according to RFC 3339. However, in Python and Java JDBC, a space is used as a separator. Use the explicit [`FORMAT_DATETIME`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/datetime_functions#format_datetime) function to define the date format correctly. Otherwise, an explicit cast is made to a string, for example: `CAST(CURRENT_DATETIME() AS STRING)` This also returns a space separator. |
| `CURRENT_DATE` | [`CURRENT_DATE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#current_date) |
| `CURRENT_DATE-3` | BigQuery does not support arithmetic data operations. Instead, use the [`DATE_ADD`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#date_add) function. |

## `SELECT` statement

Generally, the Netezza `SELECT` statement is compatible with
BigQuery. The following table contains a list of exceptions:

| **Netezza** | **BigQuery** |
|---|---|
| A `SELECT` statement without `FROM` clause | Supports special case such as the following: `SELECT 1 UNION ALL SELECT 2;` |
| <br /> ``` SELECT (subquery) AS flag, CASE WHEN flag = 1 THEN ... ``` <br /> | In BigQuery, columns cannot reference the output of other columns defined within the same query. You must duplicate the logic or move the logic into a nested query. Option 1 <br /> ``` SELECT (subquery) AS flag, CASE WHEN (subquery) = 1 THEN ... ``` <br /> Option 2 <br /> ``` SELECT q.*, CASE WHEN flag = 1 THEN ... FROM ( SELECT (subquery) AS flag, ... ) AS q ``` <br /> |

## Comparison operators

| **Netezza** | **BigQuery** | **Description** |
|---|---|---|
| `exp = exp2` | [`exp = exp2`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/operators#comparison_operators) | Equal |
| `exp <= exp2` | [`exp <= exp2`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/operators#comparison_operators) | Less than or equal to |
| `exp < exp2` | [`exp < exp2`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/operators#comparison_operators) | Less than |
| `exp <> exp2` `exp != exp2` | [`exp <> exp2`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/operators#comparison_operators) [`exp != exp2`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/operators#comparison_operators) | Not equal |
| `exp >= exp2` | [`exp >= exp2`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/operators#comparison_operators) | Greater than or equal to |
| `exp > exp2` | [`exp > exp2`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/operators#comparison_operators) | Greater than |

## Built-in SQL functions

| **Netezza** | **BigQuery** | **Description** |
|---|---|---|
| `CURRENT_DATE` | [`CURRENT_DATE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#current_date) | Get the current date (year, month, and day). |
| `CURRENT_TIME` | [`CURRENT_TIME`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/time_functions#current_time) | Get the current time with fraction. |
| `CURRENT_TIMESTAMP` | [`CURRENT_TIMESTAMP`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#current_timestamp) | Get the current system date and time, to the nearest full second. |
| `NOW` | [`CURRENT_TIMESTAMP`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#current_timestamp) | Get the current system date and time, to the nearest full second. |
| `COALESCE(exp, 0)` | [`COALESCE(exp, 0)`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/conditional_expressions#coalesce) | Replace `NULL` with zero. |
| `NVL(exp, 0)` | [`IFNULL(exp, 0)`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/conditional_expressions#ifnull) | Replace `NULL` with zero. |
| `EXTRACT(DOY FROM timestamp_expression)` | [`EXTRACT(DAYOFYEAR FROM timestamp_expression)`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#extract) | Return the number of days from the beginning of the year. |
| `ADD_MONTHS(date_expr, num_expr)` | [`DATE_ADD(date, INTERVAL k MONTH)`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#date_add) | Add months to a date. |
| `DURATION_ADD(date, k)` | [`DATE_ADD(date, INTERVAL k DAY)`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#date_add) | Perform addition on dates. |
| `DURATION_SUBTRACT(date, k)` | [`DATE_SUB(date, INTERVAL k DAY)`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#date_sub) | Perform subtraction on dates. |
| `str1 || str2` | [`CONCAT(str1, str2)`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#concat) | Concatenate strings. |

## Functions

This section compares Netezza and BigQuery functions.

### Aggregate functions

| **Netezza** | **BigQuery** |
|---|---|
|   | [`ANY_VALUE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#any_value) |
|   | [`APPROX_COUNT_DISTINCT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/approximate_aggregate_functions#approx_count_distinct) |
|   | [`APPROX_QUANTILES`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/approximate_aggregate_functions#approx_quantiles) |
|   | [`APPROX_TOP_COUNT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/approximate_aggregate_functions#approx_top_count) |
|   | [`APPROX_TOP_SUM`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/approximate_aggregate_functions#approx_top_sum) |
| `AVG` | [`AVG`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#avg) |
| `intNand` | [`BIT_AND`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#bit_and) |
| `intNnot` | Bitwise not operator: [`~`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/operators#bitwise_operators) |
| `intNor` | [`BIT_OR`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#bit_or) |
| `intNxor` | [`BIT_XOR`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#bit_xor) |
| `intNshl` |   |
| `intNshr` |   |
| `CORR` | [`CORR`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#corr) |
| `COUNT` | [`COUNT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#count) |
|   | [`COUNTIF`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#countif) |
| `COVAR_POP` | [`COVAR_POP`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#covar_pop) |
| `COVAR_SAMP` | [`COVAR_SAMP`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#covar_samp) |
| `GROUPING` |   |
|   | [`LOGICAL_AND`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#logical_and) |
|   | [`LOGICAL_OR`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#logical_or) |
| `MAX` | [`MAX`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#max) |
| `MIN` | [`MIN`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#min) |
| `MEDIAN` | [`PERCENTILE_CONT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/navigation_functions#percentile_cont)`(x, 0.5)` |
| `STDDEV_POP` | [`STDDEV_POP`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#stddev_pop) |
| `STDDEV_SAMP` | [`STDDEV_SAMP`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#stddev_samp) [`STDDEV`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#stddev) |
|   | [`STRING_AGG`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#string_agg) |
| `SUM` | [`SUM`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#sum) |
| `VAR_POP` | [`VAR_POP`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#var_pop) |
| `VAR_SAMP` | [`VAR_SAMP`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#var_samp) [`VARIANCE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#variance) |

### Analytical functions

| **Netezza** | **BigQuery** |
|---|---|
|   | [`ANY_VALUE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#any_value) |
|   | [`ARRAY_AGG`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#array_agg) |
| `ARRAY_CONCAT` | [`ARRAY_CONCAT_AGG`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#array_concat_agg) |
| `ARRAY_COMBINE` |   |
| `ARRAY_COUNT` |   |
| `ARRAY_SPLIT` |   |
| `ARRAY_TYPE` |   |
| `AVG` | [`AVG`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#avg) |
| `intNand` | [`BIT_AND`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#bit_and) |
| `intNnot` | Bitwise not operator: [`~`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/operators#bitwise_operators) |
| `intNor` | [`BIT_OR`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#bit_or) |
| `intNxor` | [`BIT_XOR`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#bit_xor) |
| `intNshl` |   |
| `intNshr` |   |
| `CORR` | [`CORR`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#corr) |
| `COUNT` | [`COUNT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#count) |
|   | [`COUNTIF`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#countif) |
| `COVAR_POP` | [`COVAR_POP`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#covar_pop) |
| `COVAR_SAMP` | [`COVAR_SAMP`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#covar_samp) |
| `CUME_DIST` | [`CUME_DIST`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/numbering_functions#cume_dist) |
| `DENSE_RANK` | [`DENSE_RANK`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/numbering_functions#dense_rank) |
| `FIRST_VALUE` | [`FIRST_VALUE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/navigation_functions#first_value) |
| `LAG` | [`LAG`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/navigation_functions#lag) |
| `LAST_VALUE` | [`LAST_VALUE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/navigation_functions#last_value) |
| `LEAD` | [`LEAD`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/navigation_functions#lead) |
| `AND` | [`LOGICAL_AND`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#logical_and) |
| `OR` | [`LOGICAL_OR`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#logical_or) |
| `MAX` | [`MAX`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#max) |
| `MIN` | [`MIN`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#min) |
|   | [`NTH_VALUE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/navigation_functions#nth_value) |
| `NTILE` | [`NTILE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/numbering_functions#ntile) |
| `PERCENT_RANK` | [`PERCENT_RANK`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/numbering_functions#percent_rank) |
| `PERCENTILE_CONT` | [`PERCENTILE_CONT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/navigation_functions#percentile_cont) |
| `PERCENTILE_DISC` | [`PERCENTILE_DISC`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/navigation_functions#percentile_disc) |
| `RANK` | [`RANK`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/numbering_functions#rank) |
| `ROW_NUMBER` | [`ROW_NUMBER`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/numbering_functions#row_number) |
| `STDDEV` | [`STDDEV`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#stddev) |
| `STDDEV_POP` | [`STDDEV_POP`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#stddev_pop) |
| `STDDEV_SAMP` | [`STDDEV_SAMP`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#stddev_samp) |
|   | [`STRING_AGG`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#string_agg) |
| `SUM` | [`SUM`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#sum) |
| `VARIANCE` | [`VARIANCE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#variance) |
| `VAR_POP` | [`VAR_POP`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#var_pop) |
| `VAR_SAMP` | [`VAR_SAMP`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#var_samp) [`VARIANCE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#variance) |
| `WIDTH_BUCKET` |   |

### Date and time functions

| **Netezza** | **BigQuery** |
|---|---|
| `ADD_MONTHS` | [`DATE_ADD`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#date_add) [`TIMESTAMP_ADD`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#timestamp_add) |
| `AGE` |   |
| `CURRENT_DATE` | [`CURRENT_DATE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#current_date) |
|   | [`CURRENT_DATETIME`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/datetime_functions#current_datetime) |
| `CURRENT_TIME` | [`CURRENT_TIME`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/time_functions#current_time) |
| `CURRENT_TIME(p)` |   |
| `CURRENT_TIMESTAMP` | [`CURRENT_TIMESTAMP`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#current_timestamp) |
| `CURRENT_TIMESTAMP(p)` |   |
|   | [`DATE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#date) |
|   | [`DATE_ADD`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#date_add) |
|   | [`DATE_DIFF`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#date_diff) |
|   | [`DATE_FROM_UNIX_DATE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#date_from_unix_date) |
|   | [`DATE_SUB`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#date_sub) |
| `DATE_TRUNC` | [`DATE_TRUNC`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#date_trunc) |
| `DATE_PART` |   |
|   | [`DATETIME`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/datetime_functions#datetime) |
|   | [`DATETIME_ADD`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/datetime_functions#datetime_add) |
|   | [`DATETIME_DIFF`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/datetime_functions#datetime_diff) |
|   | [`DATETIME_SUB`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/datetime_functions#datetime_sub) |
|   | [`DATETIME_TRUNC`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/datetime_functions#datetime_trunc) |
| `DURATION_ADD` |   |
| `DURATION_SUBTRACT` |   |
| `EXTRACT` | [`EXTRACT (DATE)`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#extract) [`EXTRACT (TIMESTAMP)`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#extract) |
|   | [`FORMAT_DATE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#format_date) |
|   | [`FORMAT_DATETIME`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/datetime_functions#format_datetime) |
|   | [`FORMAT_TIME`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/time_functions#format_time) |
|   | [`FORMAT_TIMESTAMP`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#format_timestamp) |
| `LAST_DAY` | [`DATE_SUB`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#date_sub)`(` [`DATE_TRUNC`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#date_trunc)`(` [`DATE_ADD`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#date_add)`(` `date_expression, INTERVAL 1 MONTH ), MONTH ),` `INTERVAL 1 DAY )` |
| `MONTHS_BETWEEN` | [`DATE_DIFF`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#date_diff)`(date_expression,` `date_expression, MONTH)` |
| `NEXT_DAY` |   |
| `NOW` |   |
| `OVERLAPS` |   |
|   | [`PARSE_DATE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#parse_date) |
|   | [`PARSE_DATETIME`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/datetime_functions#parse_datetime) |
|   | [`PARSE_TIME`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/time_functions#parse_time) |
|   | [`PARSE_TIMESTAMP`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#parse_timestamp) |
|   | [`STRING`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#string) |
|   | [`TIME`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/time_functions#time) |
|   | [`TIME_ADD`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/time_functions#time_add) |
|   | [`TIME_DIFF`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/time_functions#time_diff) |
|   | [`TIME_SUB`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/time_functions#time_sub) |
|   | [`TIME_TRUNC`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/time_functions#time_trunc) |
| `TIMEOFDAY` |   |
| `TIMESTAMP` | [`DATETIME`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/datetime_functions#datetime) |
|   | [`TIMESTAMP_ADD`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#timestamp_add) |
|   | [`TIMESTAMP_DIFF`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#timestamp_diff) |
|   | [`TIMESTAMP_MICROS`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#timestamp_micros) |
|   | [`TIMESTAMP_MILLIS`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#timestamp_millis) |
|   | [`TIMESTAMP_SECONDS`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#timestamp_seconds) |
|   | [`TIMESTAMP_SUB`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#timestamp_sub) |
|   | [`TIMESTAMP_TRUNC`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#timestamp_trunc) |
| `TIMEZONE` |   |
| `TO_DATE` | [`PARSE_DATE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#parse_date) |
| `TO_TIMESTAMP` | [`PARSE_TIMESTAMP`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#parse_timestamp) |
|   | [`UNIX_DATE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#unix_date) |
|   | [`UNIX_MICROS`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#unix_micros) |
|   | [`UNIX_MILLIS`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#unix_millis) |
|   | [`UNIX_SECONDS`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#unix_seconds) |

### String functions

| **Netezza** | **BigQuery** |
|---|---|
| `ASCII` | [`TO_CODE_POINTS`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#to_code_points)`(string_expr)[OFFSET(0)]` |
|   | [`BYTE_LENGTH`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#byte_length) |
|   | [`TO_HEX`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#to_hex) |
|   | [`CHAR_LENGTH`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#char_length) |
|   | [`CHARACTER_LENGTH`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#character_length) |
|   | [`CODE_POINTS_TO_BYTES`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#code_points_to_bytes) |
| `BTRIM` |   |
| `CHR` | [`CODE_POINTS_TO_STRING`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#code_points_to_string)`([numeric_expr])` |
|   | [`CONCAT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#concat) |
| `DBL_MP` |   |
| `DLE_DST` |   |
|   | [`ENDS_WITH`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#ends_with) |
|   | [`FORMAT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#format_string) |
|   | [`FROM_BASE32`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#from_base32) |
|   | [`FROM_BASE64`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#from_base64) |
|   | [`FROM_HEX`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#from_hex) |
| `HEX_TO_BINARY` |   |
| `HEX_TO_GEOMETRY` |   |
| `INITCAP` |   |
| `INSTR` |   |
| `INT_TO_STRING` |   |
| `LE_DST` |   |
| `LENGTH` | [`LENGTH`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#length) |
| `LOWER` | [`LOWER`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#lower) |
| `LPAD` | [`LPAD`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#lpad) |
| `LTRIM` | [`LTRIM`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#ltrim) |
|   | [`NORMALIZE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#normalize) |
|   | [`NORMALIZE_AND_CASEFOLD`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#normalize_and_casefold) |
| `PRI_MP` |   |
|   | [`REGEXP_CONTAINS`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#regexp_contains) |
| `REGEXP_EXTRACT` | [`REGEXP_EXTRACT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#regexp_extract) |
| `REGEXP_EXTRACT_ALL` | [`REGEXP_EXTRACT_ALL`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#regexp_extract_all) |
| `REGEXP_EXTRACT_ALL_SP` |   |
| `REGEXP_EXTRACT_SP` |   |
| `REGEXP_INSTR` | [`STRPOS`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#strpos)`(col,` [`REGEXP_EXTRACT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#regexp_extract)`()`) |
| `REGEXP_LIKE` |   |
| `REGEXP_MATCH_COUNT` |   |
| `REGEXP_REPLACE` | [`REGEXP_REPLACE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#regexp_replace) |
| `REGEXP_REPLACE_SP` | `IF(`[`REGEXP_CONTAINS`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#regexp_contains)`,1,0)` |
|   | [`REGEXP_EXTRACT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#regexp_extract) |
| `REPEAT` | [`REPEAT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#repeat) |
|   | [`REPLACE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#replace) |
|   | [`REVERSE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#reverse) |
| `RPAD` | [`RPAD`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#rpad) |
| `RTRIM` | [`RTRIM`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#rtrim) |
|   | [`SAFE_CONVERT_BYTES_TO_STRING`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#safe_convert_bytes_to_string) |
| `SCORE_MP` |   |
| `SEC_MP` |   |
| `SOUNDEX` |   |
|   | [`SPLIT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#split) |
|   | [`STARTS_WITH`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#starts_with) |
| `STRING_TO_INT` |   |
| `STRPOS` | [`STRPOS`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#strpos) |
| `SUBSTR` | [`SUBSTR`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#substr) |
|   | [`TO_BASE32`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#to_base32) |
|   | [`TO_BASE64`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#to_base64) |
| `TO_CHAR` |   |
| `TO_DATE` |   |
| `TO_NUMBER` |   |
| `TO_TIMESTAMP` |   |
|   | [`TO_CODE_POINTS`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#to_code_points) |
|   | [`TO_HEX`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#to_hex) |
| `TRANSLATE` |   |
|   | [`TRIM`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#trim) |
| `UPPER` | [`UPPER`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#upper) |
| `UNICODE` |   |
| `UNICODES` |   |

### Math functions

| **Netezza** | **BigQuery** |
|---|---|
| `ABS` | [`ABS`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#abs) |
| `ACOS` | [`ACOS`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#acos) |
|   | [`ACOSH`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#acosh) |
| `ASIN` | [`ASIN`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#asin) |
|   | [`ASINH`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#asinh) |
| `ATAN` | [`ATAN`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#atan) |
| `ATAN2` | [`ATAN2`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#atan2) |
|   | [`ATANH`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#atanh) |
| `CEIL` `DCEIL` | [`CEIL`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#ceil) |
|   | [`CEILING`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#ceiling) |
| `COS` | [`COS`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#cos) |
|   | [`COSH`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#cosh) |
| `COT` | [`COT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#cot) |
| `DEGREES` |   |
|   | [`DIV`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#div) |
| `EXP` | [`EXP`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#exp) |
| `FLOOR` `DFLOOR` | [`FLOOR`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#floor) |
| `GREATEST` | [`GREATEST`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#greatest) |
|   | [`IEEE_DIVIDE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#ieee_divide) |
|   | [`IS_INF`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#is_inf) |
|   | [`IS_NAN`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#is_nan) |
| `LEAST` | [`LEAST`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#least) |
| `LN` | [`LN`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#ln) |
| `LOG` | [`LOG`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#log) |
|   | [`LOG10`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#log10) |
| `MOD` | [`MOD`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#mod) |
|   | [`NULLIF`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/conditional_expressions#nullif)`(expr, 0)` |
| `PI` | [`ACOS`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#acos)`(-1)` |
| `POW` `FPOW` | [`POWER`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#power) [`POW`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#pow) |
| `RADIANS` |   |
| `RANDOM` | [`RAND`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#rand) |
| `ROUND` | [`ROUND`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#round) |
|   | [`SAFE_DIVIDE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#safe_divide) |
| `SETSEED` |   |
| `SIGN` | [`SIGN`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#sign) |
| `SIN` | [`SIN`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#sin) |
|   | [`SINH`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#sinh) |
| `SQRT` `NUMERIC_SQRT` | [`SQRT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#sqrt) |
| `TAN` | [`TAN`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#tan) |
|   | [`TANH`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#tanh) |
| `TRUNC` | [`TRUNC`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#trunc) |
|   | [`IFNULL`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/conditional_expressions#ifnull)`(expr, 0)` |

## DML syntax

This section compares Netezza and BigQuery DML syntax.

### `INSERT` statement

| **Netezza** | **BigQuery** |
|---|---|
| <br /> ``` INSERT INTO table VALUES (...); ``` <br /> | <br /> ``` INSERT INTO table (...) VALUES (...); ``` <br /> <br /> Netezza offers a `DEFAULT` keyword and other constraints for columns. In BigQuery, omitting column names in the `INSERT` statement is valid only if all columns are given. |
| <br /> ``` INSERT INTO table (...) VALUES (...); INSERT INTO table (...) VALUES (...); ``` <br /> | <br /> ``` INSERT INTO table VALUES (), (); ``` <br /> BigQuery imposes [DML quotas](https://docs.cloud.google.com/bigquery/quotas#data-manipulation-language-statements), which restrict the number of DML statements you can execute daily. To make the best use of your quota, consider the following approaches: - Combine multiple rows in a single `INSERT` statement, instead of one row per `INSERT` statement. <!-- --> - Combine multiple DML statements (including an `INSERT` statement) using a `MERGE` statement. <!-- --> - Use a `CREATE TABLE ... AS SELECT` statement to create and populate new tables. |

DML scripts in BigQuery have slightly different consistency
semantics than the equivalent statements in Netezza. Also note that
BigQuery does not offer constraints apart from `NOT
NULL`.

For an overview of snapshot isolation and session and transaction handling, see
[Consistency guarantees and transaction isolation](https://docs.cloud.google.com/bigquery/docs/migration/netezza-sql#consistency_guarantees_and_transaction_isolation).

### `UPDATE` statement

In Netezza, the `WHERE` clause is optional, but in BigQuery it is
necessary.

| **Netezza** | **BigQuery** |
|---|---|
| <br /> ``` UPDATE tbl SET tbl.col1=val1; ``` <br /> | Not supported without the `WHERE` clause. Use a `WHERE true` clause to update all rows. |
| <br /> ``` UPDATE A SET y = B.y, z = B.z + 1 FROM B WHERE A.x = B.x AND A.y IS NULL; ``` <br /> | <br /> ``` UPDATE A SET y = B.y, z = B.z + 1 FROM B WHERE A.x = B.x AND A.y IS NULL; ``` <br /> |
| <br /> ``` UPDATE A alias SET x = x + 1 WHERE f(x) IN (0, 1) ``` <br /> | <br /> ``` UPDATE A SET x = x + 1 WHERE f(x) IN (0, 1); ``` <br /> |
| <br /> ``` UPDATE A SET z = B.z FROM B WHERE A.x = B.x AND A.y = B.y ``` <br /> | <br /> ``` UPDATE A SET z = B.z FROM B WHERE A.x = B.x AND A.y = B.y; ``` <br /> |

For examples, see
[`UPDATE` examples](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/dml-syntax#update_examples).

Because of [DML quotas](https://docs.cloud.google.com/bigquery/quotas#data-manipulation-language-statements),
we recommend that you use larger `MERGE` statements instead of multiple single
`UPDATE` and `INSERT` statements. DML scripts in BigQuery have
slightly different consistency semantics than equivalent statements in Netezza.
For an overview of snapshot isolation and session and transaction handling, see
[Consistency guarantees and transaction isolation](https://docs.cloud.google.com/bigquery/docs/migration/netezza-sql#consistency_guarantees_and_transaction_isolation).

### `DELETE` and `TRUNCATE` statements

The `DELETE` and `TRUNCATE` statements are both ways to remove rows from a table
without affecting the table schema or indexes. The `TRUNCATE` statement has the
same effect as the `DELETE` statement, but is much faster than the `DELETE`
statement for large tables. The `TRUNCATE` statement is supported in Netezza but
not supported in BigQuery. However, you can use `DELETE`
statements in both Netezza and BigQuery.

In BigQuery, the `DELETE` statement must have a `WHERE` clause.
In Netezza, the `WHERE` clause is optional. If the `WHERE` clause is not
specified, all the rows in the Netezza table are deleted.

| **Netezza** | **BigQuery** | **Description** |
|---|---|---|
| <br /> ``` BEGIN; LOCK TABLE A IN EXCLUSIVE MODE; DELETE FROM A; INSERT INTO A SELECT * FROM B; COMMIT; ``` <br /> | Replacing the contents of a table with query output is the equivalent of a transaction. You can do this with either a [`query`](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#bq_query) or a [copy (`cp`)](https://docs.cloud.google.com/bigquery/docs/managing-tables#copy-table) operation. <br /> <br /> ``` bq query \ --replace \ --destination_table \ tableA \ 'SELECT * \ FROM tableB \ WHERE ...' ``` <br /> <br /> ``` bq cp \ -f tableA tableB ``` <br /> | Replace the contents of a table with the results of a query. |
| <br /> ``` DELETE FROM database.table ``` <br /> | <br /> ``` DELETE FROM table WHERE TRUE; ``` <br /> | In Netezza, when a delete statement is run, the rows are not deleted physically but only marked for deletion. Running the `GROOM TABLE` or `nzreclaim` commands later removes the rows marked for deletion and reclaims the corresponding disk space. |
| `GROOM TABLE` |   | Netezza uses the `GROOM TABLE` command to reclaim disk space by removing rows marked for deletion. |

### `MERGE` statement

A `MERGE` statement must match at most one source row for each target row. DML
scripts in BigQuery have slightly different consistency semantics
than the equivalent statements in Netezza. For an overview of snapshot isolation
and session and transaction handling, see
[Consistency guarantees and transaction isolation](https://docs.cloud.google.com/bigquery/docs/migration/netezza-sql#consistency_guarantees_and_transaction_isolation).
For examples, see
[BigQuery `MERGE` examples](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/dml-syntax#merge_examples)
and
Netezza `MERGE` examples.

## DDL syntax

This section compares Netezza and BigQuery DDL syntax.

### `CREATE TABLE` statement

| **Netezza** | **BigQuery** | **Description** |
|---|---|---|
| `TEMP` `TEMPORARY` | With BigQuery's DDL support, you can create a table from the results of a query and specify its expiration at creation time. For example, for three days: <br /> `CREATE TABLE` `'my-project.public_dump.vtemp'` `OPTIONS`( `expiration_timestamp=TIMESTAMP_ADD(CURRENT_TIMESTAMP(),` `INTERVAL 3 DAY))` | Create tables temporary to a session. |
| `ZONE MAPS` | Not supported. | Quick search for `WHERE` condition. |
| `DISTRIBUTE ON` | `PARTITION BY` | Partitioning. This is not a direct translation. `DISTRIBUTE ON` shares data between nodes, usually with a unique key for even distribution, while `PARTITION BY` prunes data into segments. |
| `ORGANIZE ON` | `CLUSTER BY` | Both Netezza and BigQuery support up to four keys for clustering. Netezza clustered base tables (CBT) provide equal precedence to each of the clustering columns. BigQuery gives precedence to the first column on which the table is clustered, followed by the second column, and so on. |
| `ROW SECURITY` | `Authorized View` | Row-level security. |
| `CONSTRAINT` | Not supported | Check constraints. |

### `DROP` statement

| **Netezza** | **BigQuery** | **Description** |
|---|---|---|
| `DROP TABLE` | `DROP TABLE` |   |
| `DROP DATABASE` | `DROP DATABASE` |   |
| `DROP VIEW` | `DROP VIEW` |   |

### Column options and attributes

| **Netezza** | **BigQuery** | **Description** |
|---|---|---|
| `NULL` `NOT NULL` | [`NULLABLE`](https://docs.cloud.google.com/bigquery/docs/schemas#modes) [`REQUIRED`](https://docs.cloud.google.com/bigquery/docs/schemas#modes) | Specify if the column is allowed to contain `NULL` values. |
| `REFERENCES` | Not supported | Specify column constraint. |
| `UNIQUE` | Not supported | Each value in the column must be unique. |
| `DEFAULT` | Not supported | Default value for all values in the column. |

### Temporary tables

Netezza supports
`TEMPORARY` tables
that exist during the duration of a session.

To build a temporary table in BigQuery, do the following:

1. Create a dataset that has a short time to live (for example, 12 hours).
2. Create the temporary table in the dataset, with a table name prefix of
   `temp`. For example, to create a table that expires in one hour, do this:

   ```googlesql
   CREATE TABLE temp.name (col1, col2, ...)
   OPTIONS(expiration_timestamp = TIMESTAMP_ADD(CURRENT_TIMESTAMP(),
   INTERVAL 1 HOUR));
   ```
3. Start reading and writing from the temporary table.

You can also
[remove duplicates independently](https://docs.cloud.google.com/bigquery/streaming-data-into-bigquery#manually_removing_duplicates)
in order to find errors in downstream systems.

Note that BigQuery does not support `DEFAULT` and `IDENTITY`
(sequences) columns.

## Procedural SQL statements

Netezza uses the
NZPLSQL
scripting language to work with stored procedures. NZPLSQL is based on Postgres'
PL/pgSQL language. This section describes how to convert procedural SQL
statements used in stored procedures, functions, and triggers from Netezza to
BigQuery.

### `CREATE PROCEDURE` statement

Netezza and BigQuery both support creating stored procedures
by using the
[`CREATE PROCEDURE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_procedure)
statement. For more information, see
[Work with SQL stored procedures](https://docs.cloud.google.com/bigquery/docs/procedures).

### Variable declaration and assignment

| **Netezza** | **BigQuery** | **Description** |
|---|---|---|
| `DECLARE var datatype(len) [DEFAULT value];` | [`DECLARE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/procedural-language#declare) | Declare variable. |
| `SET var = value;` | [`SET`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/procedural-language#set) | Assign value to variable. |

### Exception handlers

Netezza supports exception handlers that can be triggered for certain error
conditions. BigQuery does not support condition handlers.

| **Netezza** | **BigQuery** | **Description** |
|---|---|---|
| `EXCEPTION` | Not supported | Declare SQL exception handler for general errors. |

### Dynamic SQL statements

Netezza supports dynamic SQL queries inside stored procedures.
BigQuery does not support dynamic SQL statements.

| **Netezza** | **BigQuery** | **Description** |
|---|---|---|
| `EXECUTE IMMEDIATE` `sql_str;` | [`EXECUTE IMMEDIATE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/procedural-language#execute_immediate) `sql_str;` | Execute dynamic SQL. |

### Flow-of-control statements

| **Netezza** | **BigQuery** | **Description** |
|---|---|---|
| `IF THEN ELSE STATEMENT` `IF` *condition* `THEN ...` `ELSE ...` `END IF;` | [`IF`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/procedural-language#if) *condition* `THEN ...` `ELSE ...` `END IF;` | Execute conditionally. |
| Iterative Control `FOR var AS SELECT ...` `DO` *stmts* `END FOR;` `FOR var AS cur CURSOR` `FOR SELECT ...` `DO stmts END FOR;` | Not supported | Iterate over a collection of rows. |
| Iterative Control `LOOP stmts END LOOP;` | [`LOOP`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/procedural-language#loops) `sql_statement_list END LOOP;` | Loop block of statements. |
| `EXIT WHEN` | [`BREAK`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/procedural-language#break) | Exit a procedure. |
| `WHILE *condition* LOOP` | [`WHILE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/procedural-language#while) *condition* `DO ...` `END WHILE` | Execute a loop of statements until a while condition fails. |

### Other statements and procedural language elements

| **Netezza** | **BigQuery** | **Description** |
|---|---|---|
| `CALL` `proc(param,...)` | Not supported | Execute a procedure. |
| `EXEC` `proc(param,...)` | Not supported | Execute a procedure. |
| `EXECUTE` `proc(param,...)` | Not supported | Execute a procedure. |

## Multi-statement and multi-line SQL statements

Both Netezza and BigQuery support transactions (sessions) and
therefore support statements separated by semicolons that are consistently
executed together. For more information, see
[Multi-statement transactions](https://docs.cloud.google.com/bigquery/docs/transactions).

## Other SQL statements

| **Netezza** | **BigQuery** | **Description** |
|---|---|---|
| `GENERATE STATISTICS` |   | Generate statistics for all the tables in the current database. |
| `GENERATE STATISTICS ON table_name` |   | Generate statistics for a specific table. |
| `GENERATE STATISTICS ON table_name(col1,col4)` | Either use [statistical functions](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions) like `MIN, MAX, AVG,` etc., use the UI, or use the Cloud Data Loss Prevention API. | Generate statistics for specific columns in a table. |
| `GENERATE STATISTICS ON table_name` | `APPROX_COUNT_DISTINCT(col)` | Show the number of unique values for columns. |
| `INSERT INTO table_name` | `INSERT INTO table_name` | Insert a row. |
| `LOCK TABLE` `table_name FOR` `EXCLUSIVE;` | Not supported | Lock row. |
| `SET SESSION` `CHARACTERISTICS AS` `TRANSACTION ISOLATION LEVEL` ... | BigQuery always uses Snapshot Isolation. For details, see [Consistency guarantees and transaction isolation](https://docs.cloud.google.com/bigquery/docs/migration/netezza-sql#consistency_guarantees_and_transaction_isolation). | Define the transaction isolation level. |
| `BEGIN TRANSACTION` `END TRANSACTION` `COMMIT` | BigQuery always uses Snapshot Isolation. For details, see [Consistency guarantees and transaction isolation](https://docs.cloud.google.com/bigquery/docs/migration/netezza-sql#consistency_guarantees_and_transaction_isolation). | Define the transaction boundary for multi-statement requests. |
| `EXPLAIN` ... | Not supported. Similar features in the [query plan and timeline](https://docs.cloud.google.com/bigquery/query-plan-explanation) | Show query plan for a `SELECT` statement. |
| User Views metadata System Views metadata | `SELECT` `* EXCEPT(is_typed)` `FROM` `mydataset.INFORMATION_SCHEMA.TABLES;` <br /> BigQuery [Information Schema](https://docs.cloud.google.com/bigquery/docs/information-schema-intro) | Query objects in the database |

## Consistency guarantees and transaction isolation

Both Netezza and BigQuery are atomic, that is,
[ACID](https://en.wikipedia.org/wiki/ACID) compliant on
a per-mutation level across many rows. For example, a `MERGE` operation is
completely atomic, even with multiple inserted values.

### Transactions

Netezza syntactically accepts all four modes of ANSI SQL
transaction isolation.
However, regardless of what mode is specified, only the `SERIALIZABLE` mode is
used, which provides the highest possible level of consistency. This mode also
avoids dirty, non repeatable, and phantom reads between concurrent transactions.
Netezza does not use conventional
locking
to enforce consistency. Instead, it uses
serialization dependency checking,
a form of optimistic concurrency control to automatically roll back the latest
transaction when two transactions attempt to modify the same data.

BigQuery also
[supports transactions](https://docs.cloud.google.com/bigquery/docs/transactions).
BigQuery helps ensure optimistic concurrency control (first to
commit has priority) with snapshot isolation, in which a query reads the last
committed data before the query starts. This approach ensures the same level
of consistency on a per-row, per-mutation basis and across rows within the same
DML statement, yet avoids deadlocks. In the case of multiple DML updates against
the same table, BigQuery switches to
[pessimistic concurrency control](https://docs.cloud.google.com/bigquery/docs/data-manipulation-language#dml-limitations).
Load jobs can run completely independently and append to tables.

### Rollback

Netezza supports the
`ROLLBACK TRANSACTION` statement
to abort the current transaction and rollback all the changes made in the
transaction.

In BigQuery, you can use the
[`ROLLBACK TRANSACTION` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/procedural-language#rollback_transaction).

## Database limits

| **Limit** | **Netezza** | **BigQuery** |
|---|---|---|
| Tables per database | 32,000 | Unrestricted |
| Columns per table | 1600 | 10000 |
| Maximum row size | 64 KB | 100 MB |
| Column and table name length | 128 bytes | 16,384 Unicode characters |
| Rows per table | Unlimited | Unlimited |
| Maximum SQL request length |   | 1 MB (maximum unresolved standard SQL query length). <br /> 12 MB (maximum resolved legacy and standard SQL query length). <br /> Streaming: 10 MB (HTTP request size limit) 10,000 (maximum rows per request) |
| Maximum request and response size |   | 10 MB (request) and 10 GB (response) or virtually unlimited if using pagination or the Cloud Storage API. |
| Maximum number of concurrent sessions | 63 concurrent read-write transactions. 2000 concurrent connections to the server. | 100 concurrent queries (can be raised with [slot reservation](https://docs.cloud.google.com/bigquery/docs/slots)), 300 concurrent API requests per user. |

## What's next

- Get step-by-step instructions to [Migrate from IBM Netezza to BigQuery](https://docs.cloud.google.com/bigquery/docs/migration/netezza).