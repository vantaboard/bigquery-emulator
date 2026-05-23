# Teradata SQL translation guide

This document details the similarities and differences in SQL syntax between
Teradata and BigQuery to help you plan your migration. Use
[batch SQL translation](https://docs.cloud.google.com/bigquery/docs/batch-sql-translator) to
migrate your SQL scripts in bulk, or
[interactive SQL translation](https://docs.cloud.google.com/bigquery/docs/interactive-sql-translator)
to translate ad-hoc queries.

## Data types

This section shows equivalents between data types in Teradata and in BigQuery.

> [!NOTE]
> **Note:** Teradata supports [`DEFAULT`](https://docs.teradata.com/r/Enterprise_IntelliFlex_VMware/SQL-Data-Definition-Language-Syntax-and-Examples/Table-Statements/CREATE-TABLE-and-CREATE-TABLE-AS/Syntax-Elements/column_partition_definition/column_data_type_attribute/DEFAULT?tocId=FGX~nkdqLciLCOJTJMvFnA) and other [constraints](https://docs.teradata.com/r/Enterprise_IntelliFlex_VMware/SQL-Data-Definition-Language-Syntax-and-Examples/Table-Statements/CREATE-TABLE-and-CREATE-TABLE-AS/Syntax-Elements/column_partition_definition/table_constraint); these are not used in BigQuery.

| Teradata | BigQuery | Notes |
|---|---|---|
| `https://docs.teradata.com/r/SQL-External-Routine-Programming/July-2021/SQL-Data-Type-Mapping/C-Data-Types/INTEGER` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#integer_types` |   |
| `https://docs.teradata.com/r/SQL-External-Routine-Programming/July-2021/SQL-Data-Type-Mapping/C-Data-Types/SMALLINT` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#integer_types` |   |
| `https://docs.teradata.com/r/SQL-External-Routine-Programming/July-2021/SQL-Data-Type-Mapping/C-Data-Types/BYTEINT` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#integer_types` |   |
| `https://docs.teradata.com/r/SQL-External-Routine-Programming/July-2021/SQL-Data-Type-Mapping/C-Data-Types/BIGINT` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#integer_types` |   |
| `https://docs.teradata.com/r/SQL-External-Routine-Programming/July-2021/SQL-Data-Type-Mapping/C-Data-Types/DECIMAL/NUMERIC` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#numeric_type` `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#numeric_type` | Use BigQuery's `NUMERIC` (alias `DECIMAL`) when the scale (digits after the decimal point) \<= 9. Use BigQuery's `BIGNUMERIC` (alias `BIGDECIMAL`) when the scale \> 9. Use BigQuery's [parameterized](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#parameterized_decimal_type) decimal data types if you need to enforce custom digit or scale bounds (constraints). Teradata allows you to insert values of higher precision by rounding the stored value; however, it keeps the high precision in calculations. This can lead to [unexpected rounding behavior](https://docs.teradata.com/r/SQL-Data-Types-and-Literals/July-2021/Numeric-Data-Types/Rounding) compared to the ANSI standard. |
| `https://docs.teradata.com/r/SQL-External-Routine-Programming/July-2021/SQL-Data-Type-Mapping/C-Data-Types/DOUBLE-PRECISION/FLOAT/REAL` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#floating_point_types` |   |
| `https://docs.teradata.com/r/SQL-External-Routine-Programming/July-2021/SQL-Data-Type-Mapping/C-Data-Types/DECIMAL/NUMERIC` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#numeric_type` `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#numeric_type` | Use BigQuery's `NUMERIC` (alias `DECIMAL`) when the scale (digits after the decimal point) \<= 9. Use BigQuery's `BIGNUMERIC` (alias `BIGDECIMAL`) when the scale \> 9. Use BigQuery's [parameterized](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#parameterized_decimal_type) decimal data types if you need to enforce custom digit or scale bounds (constraints). Teradata allows you to insert values of higher precision by rounding the stored value; however, it keeps the high precision in calculations. This can lead to [unexpected rounding behavior](https://docs.teradata.com/r/SQL-Data-Types-and-Literals/July-2021/Numeric-Data-Types/Rounding) compared to the ANSI standard. |
| `https://docs.teradata.com/r/SQL-External-Routine-Programming/July-2021/SQL-Data-Type-Mapping/C-Data-Types/NUMBER` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#numeric_type` `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#numeric_type` | Use BigQuery's `NUMERIC` (alias `DECIMAL`) when the scale (digits after the decimal point) \<= 9. Use BigQuery's `BIGNUMERIC` (alias `BIGDECIMAL`) when the scale \> 9. Use BigQuery's [parameterized](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#parameterized_decimal_type) decimal data types if you need to enforce custom digit or scale bounds (constraints). Teradata allows you to insert values of higher precision by rounding the stored value; however, it keeps the high precision in calculations. This can lead to [unexpected rounding behavior](https://docs.teradata.com/r/SQL-Data-Types-and-Literals/July-2021/Numeric-Data-Types/Rounding) compared to the ANSI standard. |
| `https://docs.teradata.com/r/SQL-External-Routine-Programming/July-2021/SQL-Data-Type-Mapping/C-Data-Types/DOUBLE-PRECISION/FLOAT/REAL` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#floating_point_types` |   |
| `https://docs.teradata.com/r/SQL-External-Routine-Programming/July-2021/SQL-Data-Type-Mapping/C-Data-Types/CHARACTER/CHAR` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#string_type` | Use BigQuery's [parameterized](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#parameterized_string_type) `STRING` data type if you need to enforce a maximum character length. |
| `https://docs.teradata.com/r/SQL-External-Routine-Programming/July-2021/SQL-Data-Type-Mapping/C-Data-Types/VARBYTE` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#string_type` | Use BigQuery's [parameterized](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#parameterized_string_type) `STRING` data type if you need to enforce a maximum character length. |
| `https://docs.teradata.com/r/SQL-External-Routine-Programming/July-2021/SQL-Data-Type-Mapping/C-Data-Types/CHARACTER-LARGE-OBJECT/CLOB` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#string_type` |   |
| `https://docs.teradata.com/r/SQL-External-Routine-Programming/July-2021/SQL-Data-Type-Mapping/C-Data-Types/JSON` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#json_type` |   |
| `https://docs.teradata.com/r/SQL-External-Routine-Programming/July-2021/SQL-Data-Type-Mapping/C-Data-Types/BINARY-LARGE-OBJECT/BLOB` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#bytes_type` |   |
| `https://docs.teradata.com/r/SQL-External-Routine-Programming/July-2021/SQL-Data-Type-Mapping/C-Data-Types/BYTE` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#bytes_type` |   |
| `https://docs.teradata.com/r/SQL-External-Routine-Programming/July-2021/SQL-Data-Type-Mapping/C-Data-Types/VARBYTE` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#bytes_type` |   |
| `https://docs.teradata.com/r/SQL-External-Routine-Programming/July-2021/SQL-Data-Type-Mapping/C-Data-Types/DATE` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#date_type` | BigQuery does not support custom formatting similar to what Teradata with DataForm in SDF supports. |
| `https://docs.teradata.com/r/SQL-External-Routine-Programming/July-2021/SQL-Data-Type-Mapping/C-Data-Types/TIME` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#time_type` |   |
| `https://docs.teradata.com/r/SQL-External-Routine-Programming/July-2021/SQL-Data-Type-Mapping/C-Data-Types/TIME-WITH-TIME-ZONE` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#time_type` | Teradata stores the `TIME` data type in UTC and allows you to pass an offset from UTC using the `WITH TIME ZONE` syntax.` `The `TIME` data type in BigQuery represents a time that's independent of any date or time zone. |
| `https://docs.teradata.com/r/SQL-External-Routine-Programming/July-2021/SQL-Data-Type-Mapping/C-Data-Types/TIMESTAMP` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#timestamp_type` | Both Teradata and BigQuery `https://docs.teradata.com/r/SQL-External-Routine-Programming/July-2021/SQL-Data-Type-Mapping/C-Data-Types/TIMESTAMP` data types have microsecond precision (but Teradata supports leap seconds, while BigQuery does not). <br /> Both Teradata and BigQuery data types are usually associated with a UTC time zone ([details](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#time_zones)). |
| `https://docs.teradata.com/r/SQL-External-Routine-Programming/July-2021/SQL-Data-Type-Mapping/C-Data-Types/TIMESTAMP-WITH-TIME-ZONE` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#timestamp_type` | The Teradata `TIMESTAMP` can be set to a different time zone system-wide, per user or per column (using ` https://docs.teradata.com/r/SQL-External-Routine-Programming/July-2021/SQL-Data-Type-Mapping/C-Data-Types/TIMESTAMP-WITH-TIME-ZONE`). <br /> The BigQuery `TIMESTAMP` type assumes UTC if you don't explicitly specify a time zone. Make sure you either export time zone information correctly (do not concatenate a `DATE` and `TIME` value without time zone information) so that BigQuery can convert it on import. Or make sure that you convert time zone information to UTC before exporting. <br /> BigQuery has `DATETIME` for an abstraction between civil time, which does not show a timezone when it is output, and `TIMESTAMP`, which is a precise point in time that always shows the UTC timezone. |
| `https://docs.teradata.com/r/SQL-Data-Types-and-Literals/July-2021/ARRAY/VARRAY-Data-Type` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#array_type` |   |
| `https://docs.teradata.com/r/SQL-Data-Types-and-Literals/July-2021/ARRAY/VARRAY-Data-Type` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#array_type` | In BigQuery, use an array of structs, with each struct containing a field of type `ARRAY` (For details, see the BigQuery [documentation](https://docs.cloud.google.com/bigquery/docs/arrays#building_arrays_of_arrays)). |
| `https://docs.teradata.com/r/SQL-External-Routine-Programming/July-2021/SQL-Data-Type-Mapping/C-Data-Types/INTERVAL-HOUR` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#integer_types` |   |
| `https://docs.teradata.com/r/SQL-External-Routine-Programming/July-2021/SQL-Data-Type-Mapping/C-Data-Types/INTERVAL-MINUTE` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#integer_types` |   |
| `https://docs.teradata.com/r/SQL-External-Routine-Programming/July-2021/SQL-Data-Type-Mapping/C-Data-Types/INTERVAL-SECOND` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#integer_types` |   |
| `https://docs.teradata.com/r/SQL-External-Routine-Programming/July-2021/SQL-Data-Type-Mapping/C-Data-Types/INTERVAL-DAY` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#integer_types` |   |
| `https://docs.teradata.com/r/SQL-External-Routine-Programming/July-2021/SQL-Data-Type-Mapping/C-Data-Types/INTERVAL-MONTH` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#integer_types` |   |
| `https://docs.teradata.com/r/SQL-External-Routine-Programming/July-2021/SQL-Data-Type-Mapping/C-Data-Types/INTERVAL-YEAR` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#integer_types` |   |
| `https://docs.teradata.com/r/SQL-External-Routine-Programming/July-2021/SQL-Data-Type-Mapping/C-Data-Types/PERIOD-DATE/PERIOD-TIME/PERIOD-TIMESTAMP` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#date_type`, `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#date_type` | `PERIOD(DATE)` should be converted to two `DATE` columns containing the start date and end date so that they can be used with window functions. |
| `https://docs.teradata.com/r/SQL-External-Routine-Programming/July-2021/SQL-Data-Type-Mapping/C-Data-Types/PERIOD-DATE/PERIOD-TIME/PERIOD-TIMESTAMP` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#timestamp_type`, `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#timestamp_type` |   |
| `https://docs.teradata.com/r/SQL-External-Routine-Programming/July-2021/SQL-Data-Type-Mapping/C-Data-Types/PERIOD-DATE/PERIOD-TIME/PERIOD-TIMESTAMP` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#timestamp_type`, `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#timestamp_type` |   |
| `https://docs.teradata.com/r/SQL-External-Routine-Programming/July-2021/SQL-Data-Type-Mapping/C-Data-Types/PERIOD-DATE/PERIOD-TIME/PERIOD-TIMESTAMP` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#time_type`, `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#time_type` |   |
| `https://docs.teradata.com/r/SQL-External-Routine-Programming/July-2021/SQL-Data-Type-Mapping/C-Data-Types/PERIOD-DATE/PERIOD-TIME/PERIOD-TIMESTAMP` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#time_type`, `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#time_type` |   |
| `https://docs.teradata.com/r/SQL-Data-Types-and-Literals/July-2021/UDT-Data-Type` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#string_type` |   |
| `https://docs.teradata.com/r/SQL-External-Routine-Programming/July-2021/SQL-Data-Type-Mapping/C-Data-Types/XML` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#string_type` |   |
| `https://docs.teradata.com/r/SQL-External-Routine-Programming/July-2021/SQL-Data-Type-Mapping/C-Data-Types/TD_ANYTYPE` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#string_type` |   |

For more information on type casting, see the next section.

### Teradata type formatting

Teradata SQL uses a set of default formats for displaying expressions and
column data, and for conversions between data types. For example, a
`PERIOD(DATE)` data type in `INTEGERDATE` mode is formatted as `YY/MM/DD`
by default. We suggest that you use ANSIDATE mode whenever possible to ensure
ANSI SQL compliance, and use this chance to clean up legacy formats.

Teradata allows automatic application of custom formats using the
`https://docs.teradata.com/r/SQL-Data-Types-and-Literals/July-2021/Data-Type-Formats-and-Format-Phrases/FORMAT`
clause, without changing the underlying storage, either as a data type attribute
when you create a table using DDL, or in a derived expression. For
example, a `FORMAT` specification `9.99` rounds any `FLOAT` value to two digits.
In BigQuery, this functionality has to be converted by using the
`ROUND()` function.

This functionality requires handling of intricate edge cases. For instance,
when the `FORMAT` clause is applied to a `NUMERIC` column, you must take into
[account special rounding and formatting rules](https://docs.teradata.com/r/SQL-Data-Types-and-Literals/July-2021/Data-Type-Formats-and-Format-Phrases/FORMAT-Phrase-and-NUMERIC-Formats).
A `FORMAT` clause can be used to implicitly cast an `INTEGER` epoch value to a
`DATE` format. Or a `FORMAT` specification `X(6)` on a `VARCHAR` column
truncates the column value, and you have to therefore convert to a `SUBSTR()`
function. This behavior is not ANSI SQL compliant. Therefore, we suggest not
migrating column formats to BigQuery.

If column formats are absolutely required, use [Views](https://docs.cloud.google.com/bigquery/docs/views)
or [user-defined functions (UDFs)](https://docs.cloud.google.com/bigquery/docs/user-defined-functions).

For information about the default formats that Teradata SQL uses for each data
type, see the
[Teradata default formatting](https://docs.teradata.com/r/SQL-Data-Types-and-Literals/July-2021/Data-Type-Formats-and-Format-Phrases/Data-Type-Default-Formats)
documentation.

### Timestamp and date type formatting

The following table summarizes the differences in timestamp and date
formatting elements between Teradata SQL and
GoogleSQL.

> [!NOTE]
> **Note:** There are no parentheses in the Teradata formats, because the formats (`CURRENT_*`) are keywords, not functions.

| Teradata format | Teradata description | BigQuery |
|---|---|---|
| ` https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/Built-In-Functions/CURRENT_TIMESTAMP https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/Built-In-Functions/CURRENT_TIME/CURTIME ` | `TIME` and `TIMESTAMP` information in Teradata can have different time zone information, which is defined using `WITH TIME ZONE`. | If possible, use `CURRENT_TIMESTAMP()`, which is formatted in ISO format. However, the output format does always show the UTC timezone. (Internally, BigQuery does not have a timezone.) <br /> Note the following details on differences in the ISO format. <br /> `DATETIME` is formatted based on output channel conventions. In the BigQuery command-line tool and BigQuery console, it's formatted using a `T` separator according to RFC 3339. However, in Python and Java JDBC, a space is used as a separator. <br /> If you want to use an explicit format, use `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/datetime_functions#format_datetime`, which makes an explicit cast a string. For example, the following expression always returns a space separator: <br /> `CAST(CURRENT_DATETIME() AS STRING)` <br /> Teradata supports a `DEFAULT` keyword in `TIME` columns to set the current time (timestamp); this is not used in BigQuery. |
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/Built-In-Functions/CURRENT_DATE/CURDATE` | Dates are stored in Teradata as `INT64` values using the following formula: <br /> `(YEAR - 1900) * 10000 + (MONTH * 100) + DAY` <br /> Dates can be formatted as integers. | BigQuery has a separate `DATE` format that always returns a date in ISO 8601 format. <br /> `DATE_FROM_UNIX_DATE` can't be used, because it is 1970-based. <br /> Teradata supports a `DEFAULT` keyword in `DATE` columns to set the current date; this is not used in BigQuery. |
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/Built-In-Functions/CURRENT_DATE/CURDATE` | Date values are represented as integers. Teradata supports arithmetic operators for date types. | For date types, use `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#date_add` or `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#date_sub`. <br /> BigQuery uses arithmetic operators for data types: `INT64`, `NUMERIC`, and `FLOAT64`. |
| `https://docs.teradata.com/r/Data-Dictionary/July-2021/Data-Dictionary-Views/Using-System-Views/System-Calendar-View/Sys_Calendar.CALENDAR_TD1310` | Teradata provides a view for calendar operations to go beyond integer operations. | Not used in BigQuery. |
| `https://docs.teradata.com/r/Enterprise_IntelliFlex_VMware/SQL-Data-Definition-Language-Syntax-and-Examples/Session-Statements/SET-SESSION-DATEFORM/SET-SESSION-DATEFORM-Examples` | Set the session or system date format to ANSI (ISO 8601). | BigQuery always uses ISO 8601, so make sure you convert Teradata dates and times. |

## Query syntax

This section addresses differences in query syntax between Teradata and
BigQuery.

### `SELECT` statement

Most Teradata
[`SELECT` statements](https://docs.teradata.com/r/SQL-Data-Manipulation-Language/July-2021/SELECT-Statements)
are compatible with BigQuery. The following table contains a list
of minor differences.

| Teradata |   | BigQuery |
|---|---|---|
| `https://docs.teradata.com/r/SQL-Data-Manipulation-Language/July-2021/SELECT-Statements` |   | Convert to `SELECT`. BigQuery does not use the `SEL` abbreviation. |
| ` https://docs.teradata.com/r/SQL-Data-Manipulation-Language/July-2021/SELECT-Statements (*subquery*) AS flag, CASE WHEN flag = 1 THEN ... ` |   | In BigQuery, columns cannot reference the output of other columns defined within the same select list. Prefer moving a subquery into a `WITH` clause. <br /> ` WITH flags AS ( *subquery* ), SELECT CASE WHEN flags.flag = 1 THEN ... ` |
| ` https://docs.teradata.com/r/SQL-Data-Manipulation-Language/July-2021/SELECT-Statements * FROM *table* WHERE A LIKE ANY ('*string1*', '*string2*') ` |   | BigQuery does not use the `ANY` logical predicate. <br /> The same functionality can be achieved using multiple `OR` operators: <br /> `SELECT * FROM *table* WHERE col LIKE '*string1*' OR col LIKE '*string2*'` <br /> In this case, string comparison also differs. See [Comparison operators](https://docs.cloud.google.com/bigquery/docs/migration/teradata-sql#comparison_operators). |
| ` https://docs.teradata.com/r/SQL-Data-Manipulation-Language/July-2021/SELECT-Statements TOP 10 * FROM *table* ` |   | BigQuery uses `LIMIT` at the end of a query instead of `TOP *n*` following the `SELECT` keyword. |

### Comparison operators

The following table shows Teradata comparison operators that are specific to
Teradata and must be converted to the ANSI SQL:2011 compliant operators used in
BigQuery.

For information about operators in BigQuery, see the
[Operators](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/operators)
section of the BigQuery documentation.

| Teradata | BigQuery | Notes |
|---|---|---|
| `*exp* EQ *exp2*` `*exp* IN *(exp2, exp3)*` | `*exp* = *exp2*` `*exp* IN *(exp2, exp3)*` <br /> To keep non-ANSI semantics for `NOT CASESPECIFIC`, you can use `RTRIM(UPPER(*exp*)) = RTRIM(UPPER(*exp2*))` | When comparing strings for equality, Teradata *might* ignore trailing whitespaces, while BigQuery considers them part of the string. For example, `'xyz'=' xyz'` is `TRUE` in Teradata but `FALSE` in BigQuery. <br /> Teradata also provides a ` https://docs.teradata.com/r/SQL-Data-Manipulation-Language/July-2021/Statement-Syntax/USING-Request-Modifier/Usage-Notes/Character-String-Definitions-in-a-USING-Request-Modifier` column attribute that instructs Teradata to ignore case when comparing two strings. BigQuery is always case specific when comparing strings. For example, `'xYz' = 'xyz'` is `TRUE` in Teradata but `FALSE` in BigQuery. |
| `*exp* LE **exp2**` | `*exp* <= *exp2*` |   |
| `*exp* LT *exp2*` | `*exp* < *exp2*` |   |
| `*exp* NE *exp2*` | `*exp* <> *exp2* *exp* != *exp2*` |   |
| `*exp* GE *exp2*` | `*exp* >= *exp2*` |   |
| `*exp* GT *exp2*` | `*exp* > *exp2*` |   |

### `JOIN` conditions

BigQuery and Teradata support the same `JOIN`,
`ON`, and `USING` conditions. The following table
contains a list of minor differences.

| Teradata | BigQuery | Notes |
|---|---|---|
| `FROM *A* JOIN *B* ON *A.date* > *B.start_date* AND *A.date* < *B.end_date* ` | `FROM *A* LEFT OUTER JOIN *B* ON *A.date* > *B.start_date* AND *A.date* < *B.end_date* ` | BigQuery supports inequality `JOIN` clauses for all inner joins or if at least one equality condition is given (=). But not just one inequality condition (= and \<) in an `OUTER JOIN`. Such constructs are sometimes used to query date or integer ranges. BigQuery prevents users from inadvertently creating large cross joins. |
| `FROM *A*, *B* ON *A.id* = *B.id* ` | `FROM *A* JOIN *B* ON *A.id* = *B.id* ` | Using a comma between tables in Teradata is equal to an `INNER JOIN`, while in BigQuery it equals a `CROSS JOIN` (Cartesian product). Because the comma in BigQuery [legacy SQL is treated as `UNION`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/migrating-from-legacy-sql#comma_operator_with_tables), we recommend making the operation explicit to avoid confusion. |
| `FROM *A* JOIN *B* ON (COALESCE(*A.id* , 0) = COALESCE(*B.id*, 0)) ` | `FROM *A* JOIN *B* ON (COALESCE(*A.id* , 0) = COALESCE(*B.id*, 0)) ` | No difference for scalar (constant) functions. |
| `FROM *A* JOIN *B* ON *A.id* = (SELECT MAX(*B.id*) FROM *B*) ` | `FROM *A* JOIN (SELECT MAX(*B.id*) FROM *B*) *B1* ON *A.id* = *B1.id* ` | BigQuery prevents users from using subqueries, correlated subqueries, or aggregations in join predicates. This lets BigQuery parallelize queries. |

### Type conversion and casting

BigQuery has fewer but wider data types than Teradata,
which requires BigQuery to be stricter in casting.

| Teradata | BigQuery | Notes |
|---|---|---|
| `*exp* EQ *exp2*` `*exp* IN *(exp2, exp3)*` | `*exp* = *exp2*` `*exp* IN *(exp2, exp3)*` <br /> To keep non-ANSI semantics for `NOT CASESPECIFIC`, you can use `RTRIM(UPPER(*exp*)) = RTRIM(UPPER(*exp2*))` | When comparing strings for equality, Teradata *might* ignore trailing whitespaces, while BigQuery considers them part of the string. For example, `'xyz'=' xyz'` is `TRUE` in Teradata but `FALSE` in BigQuery. <br /> Teradata also provides a ` https://docs.teradata.com/r/SQL-Data-Manipulation-Language/July-2021/Statement-Syntax/USING-Request-Modifier/Usage-Notes/Character-String-Definitions-in-a-USING-Request-Modifier` column attribute that instructs Teradata to ignore case when comparing two strings. BigQuery is always case specific when comparing strings. For example, `'xYz' = 'xyz'` is `TRUE` in Teradata but `FALSE` in BigQuery. |
| `CAST(*long_varchar_column* AS CHAR(6))` | `LPAD(*long_varchar_column*, 6)` | Casting a character column in Teradata is sometimes used as a non-standard and non-optimal way to create a padded substring. |
| `CAST(92617 AS TIME) 92617 (FORMAT '99:99:99')` | `PARSE_TIME("%k%M%S", CAST(92617 AS STRING))` | Teradata performs many [more implicit type conversions](https://docs.teradata.com/r/SQL-Data-Types-and-Literals/July-2021/Data-Type-Conversions/Implicit-Type-Conversions) and rounding than BigQuery, which is generally stricter and enforces ANSI standards. (This example returns 09:26:17) |
| `CAST(48.5 (FORMAT 'zz') AS FLOAT)` | `CAST(SUBSTR(CAST(48.5 AS STRING), 0, 2) AS FLOAT64)` | Floating point and numeric data types can require special [rounding rules](https://docs.teradata.com/r/SQL-Data-Types-and-Literals/July-2021/Numeric-Data-Types/Rounding) when applied with formats such as currencies. (This example returns 48) |

#### Cast `FLOAT`/`DECIMAL` to `INT`

Where Teradata uses Gaussian and Banker algorithms to round
numerics, use the [`ROUND_HALF_EVEN` `RoundingMode`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/RoundingMode) in BigQuery:

    round(CAST(2.5 as Numeric),0, 'ROUND_HALF_EVEN')

#### Cast `STRING` to `NUMERIC` or `BIGNUMERIC`

When converting from `STRING` to numeric values, use the correct data type,
either `NUMERIC` or `BIGNUMERIC`, based on the number of decimal places in your
`STRING` value.

For more information about the supported numeric precision and scale in BigQuery,
see [Decimal types](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#decimal_types).

See also [Comparison operators](https://docs.cloud.google.com/bigquery/docs/migration/teradata-sql#comparison_operators)
and [column formats](https://docs.cloud.google.com/bigquery/docs/migration/teradata-sql#type_formatting). Both comparisons and column formatting can behave like type casts.

### `QUALIFY`, `ROWS` clauses

The `QUALIFY` clause in Teradata allows you to
[filter results for window functions](https://docs.teradata.com/reader/2_MC9vCtAJRlKle2Rpb0mA/19NnI91neorAi7LX6SJXBw).
Alternatively, a
[`ROWS` phrase](https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/Ordered-Analytical/Window-Aggregate-Functions/The-Window-Feature/ROWS-Phrase)
can be used for the same task. These work similar to a `HAVING` condition for a
`GROUP` clause, limiting the output of what in BigQuery are
called
[window functions](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/window-function-calls).

| Teradata | BigQuery |
|---|---|
| ` SELECT *col1*, *col2* FROM *table* QUALIFY ROW_NUMBER() OVER (PARTITION BY *col1* ORDER BY *col2*) = 1; ` | The Teradata `QUALIFY` clause with a window function like `ROW_NUMBER()`, `SUM()`, `COUNT()` and with `OVER PARTITION BY` is expressed in BigQuery as a `WHERE` clause on a subquery that contains an analytics value. <br /> Using `ROW_NUMBER()`: <br /> `SELECT col1, col2 FROM ( SELECT col1, col2, ROW_NUMBER() OVER (PARTITION BY col1 ORDER BY col2) RN FROM table ) WHERE RN = 1;` <br /> Using `ARRAY_AGG`, which supports larger partitions: <br /> ` SELECT *result*.* FROM ( SELECT ARRAY_AGG(*table* ORDER BY *table*.*col2* DESC LIMIT 1)[OFFSET(0)] FROM *table* GROUP BY *col1* ) AS *result*; ` |
| ` SELECT *col1*, *col2* FROM *table* AVG(*col1*) OVER (PARTITION BY *col1* ORDER BY *col2* ROWS BETWEEN 2 PRECEDING AND CURRENT ROW); ` | ` SELECT *col1*, *col2* FROM *table* AVG(*col1*) OVER (PARTITION BY *col1* ORDER BY *col2* RANGE BETWEEN 2 PRECEDING AND CURRENT ROW); ` <br /> In BigQuery, both `RANGE` and `ROWS` can be used in the window frame clause. However, window clauses can only be used with window functions like `AVG()`, not with numbering functions like `ROW_NUMBER()`. |
| ` SELECT *col1*, *col2* FROM *table* QUALIFY ROW_NUMBER() OVER (PARTITION BY *col1* ORDER BY *col2*) = 1; ` | ` SELECT *col1*, *col2* FROM Dataset-name.table QUALIFY row_number() OVER (PARTITION BY upper(*a.col1*) ORDER BY upper(*a.col2*)) = 1 ` |

### `NORMALIZE` keyword

Teradata provides the
[`NORMALIZE`](https://docs.teradata.com/r/Enterprise_IntelliFlex_VMware/SQL-Data-Definition-Language-Syntax-and-Examples/Table-Statements/ALTER-TABLE/ALTER-TABLE-Syntax-Elements/ALTER-TABLE-Syntax-Elements-Basic/ALTER-TABLE-Basic-Options/NORMALIZE)
keyword for `SELECT` clauses to coalesce overlapping periods or intervals into
a single period or interval that encompasses all individual period values.

BigQuery does not support the `PERIOD` type, so any `PERIOD`
type column in Teradata has to be inserted into BigQuery as two
separate `DATE` or `DATETIME` fields that correspond to the start and end of the
period.

| Teradata | BigQuery |
|---|---|
| `SELECT NORMALIZE client_id, item_sid, BEGIN(*period*) AS min_date, END(*period*) AS max_date, FROM *table*; ` | `SELECT t.client_id, t.item_sid, t.min_date, MAX(t.dwh_valid_to) AS max_date FROM ( SELECT d1.client_id, d1.item_sid, d1.dwh_valid_to AS dwh_valid_to, MIN(d2.dwh_valid_from) AS min_date FROM *table* d1 LEFT JOIN *table* d2 ON d1.client_id = d2.client_id AND d1.item_sid = d2.item_sid AND d1.dwh_valid_to >= d2.dwh_valid_from AND d1.dwh_valid_from < = d2.dwh_valid_to GROUP BY d1.client_id, d1.item_sid, d1.dwh_valid_to ) t GROUP BY t.client_id, t.item_sid, t.min_date;` |

## Functions

The following sections list mappings between Teradata functions and
BigQuery equivalents.

### Aggregate functions

The following table maps common Teradata aggregate,
statistical aggregate, and approximate aggregate functions to their
BigQuery equivalents. BigQuery offers the
following additional aggregate functions:

- [`ANY_VALUE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#any_value)
- [`APPROX_COUNT_DISTINCT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/approximate_aggregate_functions#approx_count_distinct)
- [`APPROX_QUANTILES`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/approximate_aggregate_functions#approx_quantiles)
- [`APPROX_TOP_COUNT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/approximate_aggregate_functions#approx_top_count)
- [`APPROX_TOP_SUM`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/approximate_aggregate_functions#approx_top_sum)
- [`COUNTIF`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#countif)
- [`LOGICAL_AND`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#logical_and)
- [`LOGICAL_OR`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#logical_or)
- [`STRING_AGG`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#string_agg)

| Teradata | BigQuery |
|---|---|
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/Aggregate-Functions/AVG` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#avg` **Note** : BigQuery provides approximate results when calculating the average of `INT` values. |
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/Bit/Byte-Manipulation-Functions/BITAND` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#bit_and` |
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/Bit/Byte-Manipulation-Functions/BITNOT` | [Bitwise not operator](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/operators#bitwise_operators) (`~`) |
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/Bit/Byte-Manipulation-Functions/BITOR` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#bit_or` |
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/Bit/Byte-Manipulation-Functions/BITXOR` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#bit_xor` |
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/Aggregate-Functions/CORR` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#corr` |
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/Aggregate-Functions/COUNT` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#count` |
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/Aggregate-Functions/COVAR_POP` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#covar_pop` |
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/Aggregate-Functions/COVAR_SAMP` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#covar_samp` |
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/Aggregate-Functions/MAXIMUM` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#max` |
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/Aggregate-Functions/MINIMUM` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#min` |
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/Aggregate-Functions/REGR_AVGX` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#avg( IF(*dep_var_expression* is NULL OR *ind_var_expression* is NULL, NULL, *ind_var_expression*) )` |
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/Aggregate-Functions/REGR_AVGY` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#avg( IF(*dep_var_expression* is NULL OR *ind_var_expression* is NULL, NULL, *dep_var_expression*) )` |
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/Aggregate-Functions/REGR_COUNT` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#sum( IF(*dep_var_expression* is NULL OR *ind_var_expression* is NULL, NULL, 1) )` |
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/Aggregate-Functions/REGR_INTERCEPT` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#avg(*dep_var_expression*) - https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#avg(*ind_var_expression*) * (https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#covar_samp(*ind_var_expression*, *dep_var_expression*) / https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#variance(*ind_var_expression*))` |
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/Aggregate-Functions/REGR_R2` | `(https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#count(*dep_var_expression*)* https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#sum(*ind_var_expression* * *dep_var_expression*) - https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#sum(*dep_var_expression*) * https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#sum(*ind_var_expression*)) / https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#sqrt( (https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#count(*ind_var_expression*)* https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#sum(https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#power(*ind_var_expression*, 2))* https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#power(https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#sum(*ind_var_expression*),2))* (https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#count(*dep_var_expression*)* https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#sum(https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#power(*dep_var_expression*, 2))* https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#power(https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#sum(*dep_var_expression*), 2)))` |
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/Aggregate-Functions/REGR_SLOPE` | `- https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#covar_samp(*ind_var_expression*, *dep_var_expression*) / https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#variance(*ind_var_expression*)` |
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/Aggregate-Functions/REGR_SXX` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#sum(https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#power(*ind_var_expression*, 2)) - https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#count(*ind_var_expression*) * https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#power(https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#avg(*ind_var_expression*),2)` |
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/Aggregate-Functions/REGR_SXY` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#sum(*ind_var_expression* * *dep_var_expression*) - https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#count(*ind_var_expression*) * https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#avg(*ind_var_expression*) * https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#avg(*dep_var_expression*)` |
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/Aggregate-Functions/REGR_SYY` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#sum(https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#power(*dep_var_expression*, 2)) - https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#count(*dep_var_expression*) * https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#power(https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#sum(*dep_var_expression*),2)` |
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/Aggregate-Functions/SKEW` | Custom user-defined function. |
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/Aggregate-Functions/STDDEV_POP` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#stddev_pop` |
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/Aggregate-Functions/STDDEV_SAMP` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#stddev_samp, https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#stddev` |
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/Aggregate-Functions/SUM` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#sum` |
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/Aggregate-Functions/VAR_POP` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#var_pop` |
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/Aggregate-Functions/VAR_SAMP` | ` https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#var_samp, https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#variance` |

### Analytical functions and window functions

The following table maps common Teradata analytic and
aggregate analytic functions to their BigQuery
window function equivalents. BigQuery offers the following
additional functions:

- [`ANY_VALUE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#any_value)
- [`AVG`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#avg)
- [`COUNTIF`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#countif)
- [`LAG`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/navigation_functions#lag)
- [`LEAD`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/navigation_functions#lead)
- [`LOGICAL_AND`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#logical_and)
- [`LOGICAL_OR`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#logical_or)
- [`NTH_VALUE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/navigation_functions#nth_value)
- [`NTILE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/numbering_functions#ntile)
- [`STRING_AGG`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#string_agg)

| Teradata | BigQuery |
|---|---|
| `https://docs.teradata.com/r/SQL-Data-Types-and-Literals/July-2021/ARRAY/VARRAY-functions-all/ARRAY_AGG` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#array_agg` |
| `https://docs.teradata.com/r/SQL-Data-Types-and-Literals/July-2021/ARRAY/VARRAY-functions-all/ARRAY_CONCATENATION_FUNCTION` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#array_concat_agg` |
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/Bit/Byte-Manipulation-Functions/BITAND` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#bit_and` |
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/Bit/Byte-Manipulation-Functions/BITNOT` | [Bitwise not operator](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/operators#bitwise_operators) (`~`) |
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/Bit/Byte-Manipulation-Functions/BITOR` | ` https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#bit_or ` |
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/Bit/Byte-Manipulation-Functions/BITXOR` | ` https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#bit_xor ` |
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/Aggregate-Functions/CORR` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#corr` |
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/Aggregate-Functions/COUNT` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#count` |
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/Aggregate-Functions/COVAR_POP` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#covar_pop` |
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/Aggregate-Functions/COVAR_SAMP` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#covar_samp` |
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/Ordered-Analytical/Window-Aggregate-Functions/CUME_DIST` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/numbering_functions#cume_dist` |
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/Ordered-Analytical/Window-Aggregate-Functions/DENSE_RANK-ANSI` (ANSI) | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/numbering_functions#dense_rank` |
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/Ordered-Analytical/Window-Aggregate-Functions/FIRST_VALUE/LAST_VALUE` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/navigation_functions#first_value` |
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/Ordered-Analytical/Window-Aggregate-Functions/FIRST_VALUE/LAST_VALUE` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/navigation_functions#last_value` |
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/Aggregate-Functions/MAXIMUM` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#max` |
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/Aggregate-Functions/MINIMUM` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#min` |
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/Ordered-Analytical/Window-Aggregate-Functions/PERCENT_RANK` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/numbering_functions#percent_rank` |
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/Ordered-Analytical/Window-Aggregate-Functions/PERCENTILE_CONT/PERCENTILE_DISC`, `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/Ordered-Analytical/Window-Aggregate-Functions/PERCENTILE_CONT/PERCENTILE_DISC` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/navigation_functions#percentile_cont`, `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/navigation_functions#percentile_disc` |
| `https://docs.teradata.com/r/Enterprise_IntelliFlex_VMware/SQL-Functions-Expressions-and-Predicates/Ordered-Analytical/Window-Aggregate-Functions/QUANTILE` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/numbering_functions#ntile` |
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/Ordered-Analytical/Window-Aggregate-Functions/RANK-ANSI` (ANSI) | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/numbering_functions#rank` |
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/Ordered-Analytical/Window-Aggregate-Functions/ROW_NUMBER` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/numbering_functions#row_number` |
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/Aggregate-Functions/STDDEV_POP` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#stddev_pop` |
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/Aggregate-Functions/STDDEV_SAMP` | ` https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#stddev_samp, https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#stddev ` |
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/Aggregate-Functions/SUM` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#sum` |
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/Aggregate-Functions/VAR_POP` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#var_pop` |
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/Aggregate-Functions/VAR_SAMP` | ` https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#var_samp, https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#variance ` |

### Date/time functions

The following table maps common Teradata date/time functions
to their BigQuery equivalents. BigQuery offers
the following additional date/time functions:

- [`CURRENT_DATETIME`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/datetime_functions#current_datetime)
- [`DATE_ADD`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#date_add)
- [`DATE_DIFF`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#date_diff)
- [`DATE_FROM_UNIX_DATE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#date_from_unix_date)
- [`DATE_SUB`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#date_sub)
- [`DATE_TRUNC`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#date_trunc)
- [`DATETIME`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/datetime_functions#datetime)
- [`DATETIME_ADD`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/datetime_functions#datetime_add)
- [`DATETIME_DIFF`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/datetime_functions#datetime_diff)
- [`DATETIME_SUB`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/datetime_functions#datetime_sub)
- [`DATETIME_TRUNC`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/datetime_functions#datetime_trunc)
- [`PARSE_DATE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#parse_date)
- [`PARSE_DATETIME`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/datetime_functions#parse_datetime)
- [`PARSE_TIME`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/time_functions#parse_time)
- [`PARSE_TIMESTAMP`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#parse_timestamp)
- [`STRING`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions)
- [`TIME`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/time_functions#time)
- [`TIME_ADD`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/time_functions#time_add)
- [`TIME_DIFF`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/time_functions#time_diff)
- [`TIME_SUB`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/time_functions#time_sub)
- [`TIME_TRUNC`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/time_functions#time_trunc)
- [`TIMESTAMP`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#timestamp)
- [`TIMESTAMP_ADD`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#timestamp_add)
- [`TIMESTAMP_DIFF`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#timestamp_diff)
- [`TIMESTAMP_MICROS`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#timestamp_micros)
- [`TIMESTAMP_MILLIS`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#timestamp_millis)
- [`TIMESTAMP_SECONDS`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#timestamp_seconds)
- [`TIMESTAMP_SUB`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#timestamp_sub)
- [`TIMESTAMP_TRUNC`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#timestamp_trunc)
- [`UNIX_DATE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#unix_date)
- [`UNIX_MICROS`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#unix_micros)
- [`UNIX_MILLIS`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#unix_millis)
- [`UNIX_SECONDS`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#unix_seconds)

| Teradata | BigQuery |
|---|---|
| `https://docs.teradata.com/r/SQL-Date-and-Time-Functions-and-Expressions/July-2021/DateTime-and-Interval-Functions-and-Expressions/ADD_MONTHS` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#date_add, https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#timestamp_add ` |
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/Built-In-Functions/CURRENT_DATE/CURDATE` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#current_date ` |
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/Built-In-Functions/CURRENT_TIME/CURTIME` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/time_functions#current_time ` |
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/Built-In-Functions/CURRENT_TIMESTAMP` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#current_timestamp ` |
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/Built-In-Functions/DATE + k` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#date_add(*date_expression*, INTERVAL *k* DAY) ` |
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/Built-In-Functions/DATE - k` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#date_sub(*date_expression*, INTERVAL *k* DAY) ` |
| `https://docs.teradata.com/r/SQL-Date-and-Time-Functions-and-Expressions/July-2021/DateTime-and-Interval-Functions-and-Expressions/EXTRACT` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#extract(DATE), https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#extract(TIMESTAMP) ` |
|   | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#format_date ` |
|   | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/datetime_functions#format_datetime ` |
|   | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/time_functions#format_time ` |
|   | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#format_timestamp ` |
| `https://docs.teradata.com/r/SQL-Date-and-Time-Functions-and-Expressions/July-2021/DateTime-and-Interval-Functions-and-Expressions/LAST_DAY` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#last_day` **Note** : This function supports both `DATE` and `DATETIME` input expressions. |
| `https://docs.teradata.com/r/SQL-Date-and-Time-Functions-and-Expressions/July-2021/DateTime-and-Interval-Functions-and-Expressions/MONTHS_BETWEEN` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#date_diff(*date_expression*, *date_expression*, MONTH) ` |
| `https://docs.teradata.com/r/SQL-Date-and-Time-Functions-and-Expressions/July-2021/DateTime-and-Interval-Functions-and-Expressions/NEXT_DAY` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#date_add( https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#date_trunc( *date_expression*, WEEK(day_value) ), INTERVAL 1 WEEK )` |
| `https://docs.teradata.com/r/SQL-Date-and-Time-Functions-and-Expressions/July-2021/DateTime-and-Interval-Functions-and-Expressions/OADD_MONTHS` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#date_sub( https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#date_trunc( https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#date_add( *date_expression*, INTERVAL *num_months* MONTH ), MONTH ), INTERVAL 1 DAY ) ` |
| `https://docs.teradata.com/r/SQL-Date-and-Time-Functions-and-Expressions/July-2021/Calendar-Functions/td_day_of_month` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#extract(DAY FROM *date_expression*) https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#extract(DAY FROM *timestamp_expression*) ` |
| `https://docs.teradata.com/r/SQL-Date-and-Time-Functions-and-Expressions/July-2021/Calendar-Functions/td_day_of_week/DayOfWeek` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#extract(DAYOFWEEK FROM *date_expression*) https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#extract(DAYOFWEEK FROM *timestamp_expression*) ` |
| `https://docs.teradata.com/r/SQL-Date-and-Time-Functions-and-Expressions/July-2021/Calendar-Functions/td_day_of_year` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#extract(DAYOFYEAR FROM *date_expression*) https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#extract(DAYOFYEAR FROM *timestamp_expression*) ` |
| `https://docs.teradata.com/r/SQL-Date-and-Time-Functions-and-Expressions/July-2021/Business-Calendars/td_friday` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#date_trunc( *date_expression*, WEEK(FRIDAY) ) ` |
| `https://docs.teradata.com/r/SQL-Date-and-Time-Functions-and-Expressions/July-2021/Business-Calendars/td_monday` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#date_trunc( *date_expression*, WEEK(MONDAY) ) ` |
| `https://docs.teradata.com/r/SQL-Date-and-Time-Functions-and-Expressions/July-2021/Business-Calendars/td_month_begin` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#date_trunc(*date_expression*, MONTH) ` |
| `https://docs.teradata.com/r/SQL-Date-and-Time-Functions-and-Expressions/July-2021/Business-Calendars/td_month_end` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#date_sub( https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#date_trunc( https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#date_add( *date_expression*, INTERVAL 1 MONTH ), MONTH ), INTERVAL 1 DAY )` |
| `https://docs.teradata.com/r/SQL-Date-and-Time-Functions-and-Expressions/July-2021/Calendar-Functions/td_month_of_calendar` | `(https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#extract(YEAR FROM *date_expression*) - 1900) * 12 + https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#extract(MONTH FROM *date_expression*) ` |
| `https://docs.teradata.com/r/SQL-Date-and-Time-Functions-and-Expressions/July-2021/Calendar-Functions/td_month_of_quarter` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#extract(MONTH FROM *date_expression*) - ((https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#extract(QUARTER FROM *date_expression*) - 1) * 3) ` |
| `https://docs.teradata.com/r/SQL-Date-and-Time-Functions-and-Expressions/July-2021/Calendar-Functions/td_month_of_year` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#extract(MONTH FROM *date_expression*) https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#extract(MONTH FROM *timestamp_expression*) ` |
| `https://docs.teradata.com/r/SQL-Date-and-Time-Functions-and-Expressions/July-2021/Business-Calendars/td_quarter_begin` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#date_trunc(*date_expression*, QUARTER) ` |
| `https://docs.teradata.com/r/SQL-Date-and-Time-Functions-and-Expressions/July-2021/Business-Calendars/td_quarter_end` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#date_sub( https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#date_trunc( https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#date_add( *date_expression*, INTERVAL 1 QUARTER ), QUARTER ), INTERVAL 1 DAY ) ` |
| `https://docs.teradata.com/r/SQL-Date-and-Time-Functions-and-Expressions/July-2021/Calendar-Functions/td_quarter_of_calendar` | `(https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#extract(YEAR FROM *date_expression*) - 1900) * 4 + https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#extract(QUARTER FROM *date_expression*) ` |
| `https://docs.teradata.com/r/SQL-Date-and-Time-Functions-and-Expressions/July-2021/Calendar-Functions/td_quarter_of_year` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#extract(QUARTER FROM *date_expression*) https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#extract(QUARTER FROM *timestamp_expression*) ` |
| `https://docs.teradata.com/r/SQL-Date-and-Time-Functions-and-Expressions/July-2021/Business-Calendars/td_saturday` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#date_trunc( *date_expression*, WEEK(SATURDAY) ) ` |
| `https://docs.teradata.com/r/SQL-Date-and-Time-Functions-and-Expressions/July-2021/Business-Calendars/td_sunday` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#date_trunc( *date_expression*, WEEK(SUNDAY) ) ` |
| `https://docs.teradata.com/r/SQL-Date-and-Time-Functions-and-Expressions/July-2021/Business-Calendars/td_thursday` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#date_trunc( *date_expression*, WEEK(THURSDAY) ) ` |
| `https://docs.teradata.com/r/SQL-Date-and-Time-Functions-and-Expressions/July-2021/Business-Calendars/td_tuesday` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#date_trunc( *date_expression*, WEEK(TUESDAY) ) ` |
| `https://docs.teradata.com/r/SQL-Date-and-Time-Functions-and-Expressions/July-2021/Business-Calendars/td_wednesday` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#date_trunc( *date_expression*, WEEK(WEDNESDAY) ) ` |
| `https://docs.teradata.com/r/SQL-Date-and-Time-Functions-and-Expressions/July-2021/Business-Calendars/td_week_begin` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#date_trunc(*date_expression*, WEEK) ` |
| `https://docs.teradata.com/r/SQL-Date-and-Time-Functions-and-Expressions/July-2021/Business-Calendars/td_week_end` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#date_sub( https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#date_trunc( https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#date_add( *date_expression*, INTERVAL 1 WEEK ), WEEK ), INTERVAL 1 DAY ) ` |
| `https://docs.teradata.com/r/SQL-Date-and-Time-Functions-and-Expressions/July-2021/Calendar-Functions/td_week_of_calendar` | `(https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#extract(YEAR FROM *date_expression*) - 1900) * 52 + https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#extract(WEEK FROM *date_expression*) ` |
| `https://docs.teradata.com/r/SQL-Date-and-Time-Functions-and-Expressions/July-2021/Calendar-Functions/td_week_of_month` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#extract(WEEK FROM *date_expression*) - https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#extract(WEEK FROM https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#date_trunc(*date_expression*, MONTH)) ` |
| `https://docs.teradata.com/r/SQL-Date-and-Time-Functions-and-Expressions/July-2021/Calendar-Functions/td_week_of_year` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#extract(WEEK FROM *date_expression*) https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#extract(WEEK FROM *timestamp_expression*) ` |
| `https://docs.teradata.com/r/SQL-Date-and-Time-Functions-and-Expressions/July-2021/Calendar-Functions/td_weekday_of_month` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/conversion_functions#cast( https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#ceil( https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#extract(DAY FROM *date_expression*) / 7 ) AS INT64 ) ` |
| `https://docs.teradata.com/r/SQL-Date-and-Time-Functions-and-Expressions/July-2021/Business-Calendars/td_year_begin` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#date_trunc(*date_expression*, YEAR) ` |
| `https://docs.teradata.com/r/SQL-Date-and-Time-Functions-and-Expressions/July-2021/Business-Calendars/td_year_end` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#date_sub( https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#date_trunc( https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#date_add( *date_expression*, INTERVAL 1 YEAR ), YEAR ), INTERVAL 1 DAY ) ` |
| `https://docs.teradata.com/r/SQL-Date-and-Time-Functions-and-Expressions/July-2021/Calendar-Functions/td_year_of_calendar` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#extract(YEAR FROM *date_expression*) ` |
| `https://docs.teradata.com/r/SQL-Data-Types-and-Literals/July-2021/Data-Type-Conversion-Functions/TO_DATE` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#parse_date ` |
| `https://docs.teradata.com/r/SQL-Data-Types-and-Literals/July-2021/Data-Type-Conversion-Functions/TO_TIMESTAMP` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#parse_timestamp ` |
| `https://docs.teradata.com/r/SQL-Data-Types-and-Literals/July-2021/Data-Type-Conversion-Functions/TO_TIMESTAMP_TZ` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#parse_timestamp ` |

### String functions

The following table maps Teradata string functions to their
BigQuery equivalents. BigQuery offers the
following additional string functions:

- [`BYTE_LENGTH`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#byte_length)
- [`CODE_POINTS_TO_BYTES`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#code_points_to_bytes)
- [`ENDS_WITH`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#ends_with)
- [`FROM_BASE32`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#from_base32)
- [`FROM_BASE64`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#from_base64)
- [`FROM_HEX`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#from_hex)
- [`NORMALIZE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#normalize)
- [`NORMALIZE_AND_CASEFOLD`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#normalize_and_casefold)
- [`REGEXP_CONTAINS`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#regexp_contains)
- [`REGEXP_EXTRACT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#regexp_extract)
- [`REGEXP_EXTRACT_ALL`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#regexp_extract_all)
- [`REPEAT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#repeat)
- [`REPLACE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#replace)
- [`SAFE_CONVERT_BYTES_TO_STRING`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#safe_convert_bytes_to_string)
- [`SPLIT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#split)
- [`STARTS_WITH`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#starts_with)
- [`STRPOS`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#strpos)
- [`TO_BASE32`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#to_base32)
- [`TO_BASE64`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#to_base64)
- [`TO_CODE_POINTS`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#to_code_points)
- [`TO_HEX`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#to_hex)

| Teradata | BigQuery |
|---|---|
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/String-Operators-and-Functions/ASCII` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#to_code_points(*string_expression*)[https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/array_functions#offset_and_ordinal(0)] ` |
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/String-Operators-and-Functions/CHAR2HEXINT` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#to_hex ` |
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/Attribute-Functions/CHARACTER_LENGTH` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#char_length ` |
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/Attribute-Functions/CHARACTER_LENGTH` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#character_length ` |
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/String-Operators-and-Functions/CHR` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#code_points_to_string( [mod(*numeric_expression*, 256)] ) ` |
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/String-Operators-and-Functions/CONCAT` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#concat ` |
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/String-Operators-and-Functions/CSV` | Custom user-defined function. |
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/String-Operators-and-Functions/CSVLD` | Custom user-defined function. |
| `https://docs.teradata.com/r/SQL-Data-Types-and-Literals/July-2021/Data-Type-Formats-and-Format-Phrases/FORMAT` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#format_string ` |
| `https://docs.teradata.com/r/SQL-Data-Definition-Language-Syntax-and-Examples/July-2021/Table-Statements/CREATE-TABLE-Queue-Table-Form/CREATE-TABLE-Syntax-Elements-Queue-Table-Form/column_attribute/index_specification` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#strpos(*string*, *substring*) ` |
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/String-Operators-and-Functions/INITCAP` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#initcap` |
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/String-Operators-and-Functions/INSTR` | Custom user-defined function. |
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/String-Operators-and-Functions/LEFT` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#substr(*source_string*, 1, *length*) ` |
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/String-Operators-and-Functions/LENGTH` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#length ` |
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/String-Operators-and-Functions/LOWER` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#lower ` |
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/String-Operators-and-Functions/LPAD` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#lpad ` |
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/String-Operators-and-Functions/LTRIM` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#ltrim ` |
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/String-Operators-and-Functions/NGRAM` | Custom user-defined function. |
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/String-Operators-and-Functions/NVP` | Custom user-defined function. |
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/String-Operators-and-Functions/OREPLACE` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#replace ` |
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/String-Operators-and-Functions/OTRANSLATE` | Custom user-defined function. |
| `https://docs.teradata.com/r/SQL-Stored-Procedures-and-Embedded-SQL/July-2021/SQL-Cursor-Control-and-DML-Statements/POSITION` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#strpos(*string*, *substring*) ` |
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/Regular-Expression-Functions/REGEXP_INSTR` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#strpos(*source_string*, https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#regexp_extract(*source_string*, *regexp_string*))` <br /> **Note**: Returns first occurrence. |
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/Regular-Expression-Functions/REGEXP_REPLACE` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#regexp_replace` |
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/Regular-Expression-Functions/REGEXP_SIMILAR` | `IF(https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#regexp_contains,1,0)` |
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/Regular-Expression-Functions/REGEXP_SUBSTR` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#regexp_extract, https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#regexp_extract_all ` |
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/Regular-Expression-Functions/REGEXP_SPLIT_TO_TABLE` | Custom user-defined function. |
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/String-Operators-and-Functions/REVERSE` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#reverse ` |
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/String-Operators-and-Functions/RIGHT` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#substr(*source_string*, -1, *length*) ` |
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/String-Operators-and-Functions/RPAD` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#rpad ` |
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/String-Operators-and-Functions/RTRIM` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#rtrim ` |
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/String-Operators-and-Functions/STRTOK` <br /> **Note**: Each character in the delimiter string argument is considered a separate delimiter character. The default delimiter is a space character. | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#split(*instring*, *delimiter*)[https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/array_functions#offset_and_ordinal(*tokennum*)]` <br /> **Note** : The entire *delimiter* string argument is used as a single delimiter. The default delimiter is a comma. |
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/String-Operators-and-Functions/STRTOK_SPLIT_TO_TABLE` | Custom user-defined function |
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/String-Operators-and-Functions/SUBSTRING`, `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/String-Operators-and-Functions/SUBSTRING` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#substr ` |
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/String-Operators-and-Functions/TRIM` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#trim ` |
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/String-Operators-and-Functions/UPPER/UCASE` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#upper ` |

### Math functions

The following table maps Teradata math functions to their
BigQuery equivalents. BigQuery offers the
following additional math functions:

- [`DIV`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#div)
- [`IEEE_DIVIDE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#ieee_divide)
- [`IS_INF`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#is_inf)
- [`IS_NAN`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#is_nan)
- [`LOG10`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#log10)
- [`SAFE_DIVIDE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#safe_divide)

| Teradata | BigQuery |
|---|---|
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/Arithmetic-Trigonometric-Hyperbolic-Operators/Functions/ABS` | ` https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#abs` |
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/Arithmetic-Trigonometric-Hyperbolic-Operators/Functions/TRIGONOMETRIC/TRIGONOMETRIC-Function-Syntax` | ` https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#acos` |
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/Arithmetic-Trigonometric-Hyperbolic-Operators/Functions/HYPERBOLIC/HYPERBOLIC-Function-Syntax` | ` https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#acosh` |
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/Arithmetic-Trigonometric-Hyperbolic-Operators/Functions/TRIGONOMETRIC/TRIGONOMETRIC-Function-Syntax` | ` https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#asin` |
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/Arithmetic-Trigonometric-Hyperbolic-Operators/Functions/HYPERBOLIC/HYPERBOLIC-Function-Syntax` | ` https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#asinh` |
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/Arithmetic-Trigonometric-Hyperbolic-Operators/Functions/TRIGONOMETRIC/TRIGONOMETRIC-Function-Syntax` | ` https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#atan` |
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/Arithmetic-Trigonometric-Hyperbolic-Operators/Functions/TRIGONOMETRIC/TRIGONOMETRIC-Function-Syntax` | ` https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#atan2` |
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/Arithmetic-Trigonometric-Hyperbolic-Operators/Functions/HYPERBOLIC/HYPERBOLIC-Function-Syntax` | ` https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#atanh` |
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/Arithmetic-Trigonometric-Hyperbolic-Operators/Functions/CEILING` | ` https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#ceil` |
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/Arithmetic-Trigonometric-Hyperbolic-Operators/Functions/CEILING` | ` https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#ceiling` |
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/Arithmetic-Trigonometric-Hyperbolic-Operators/Functions/TRIGONOMETRIC/TRIGONOMETRIC-Function-Syntax` | ` https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#cos` |
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/Arithmetic-Trigonometric-Hyperbolic-Operators/Functions/HYPERBOLIC/HYPERBOLIC-Function-Syntax` | ` https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#cosh` |
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/Arithmetic-Trigonometric-Hyperbolic-Operators/Functions/EXP` | ` https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#exp` |
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/Arithmetic-Trigonometric-Hyperbolic-Operators/Functions/FLOOR` | ` https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#floor` |
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/Comparison-Operators-and-Functions/GREATEST` | ` https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#greatest` |
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/Comparison-Operators-and-Functions/LEAST` | ` https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#least` |
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/Arithmetic-Trigonometric-Hyperbolic-Operators/Functions/LN` | ` https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#ln` |
| `https://docs.teradata.com/r/Database-Utilities/July-2021/Database-Window-xdbw/Commands/LOG` | ` https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#log` |
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/Arithmetic-Trigonometric-Hyperbolic-Operators/Functions/MOD` (`%` operator) | ` https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#mod` |
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/Arithmetic-Trigonometric-Hyperbolic-Operators/Functions/NULLIFZERO` | ` https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/conditional_expressions(*expression*, 0)` |
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/Arithmetic-Trigonometric-Hyperbolic-Operators/Functions/POWER` (`**` operator) | ` https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#power, https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#pow ` |
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/Arithmetic-Trigonometric-Hyperbolic-Operators/Functions/RANDOM` | ` https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#rand` |
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/Arithmetic-Trigonometric-Hyperbolic-Operators/Functions/ROUND` | ` https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#round` |
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/Arithmetic-Trigonometric-Hyperbolic-Operators/Functions/SIGN` | ` https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#sign` |
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/Arithmetic-Trigonometric-Hyperbolic-Operators/Functions/TRIGONOMETRIC/TRIGONOMETRIC-Function-Syntax` | ` https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#sin` |
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/Arithmetic-Trigonometric-Hyperbolic-Operators/Functions/HYPERBOLIC/HYPERBOLIC-Function-Syntax` | ` https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#sinh` |
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/Arithmetic-Trigonometric-Hyperbolic-Operators/Functions/SQRT` | ` https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#sqrt` |
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/Arithmetic-Trigonometric-Hyperbolic-Operators/Functions/TRIGONOMETRIC/TRIGONOMETRIC-Function-Syntax` | ` https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#tan` |
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/Arithmetic-Trigonometric-Hyperbolic-Operators/Functions/HYPERBOLIC/HYPERBOLIC-Function-Syntax` | ` https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#tanh` |
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/Arithmetic-Trigonometric-Hyperbolic-Operators/Functions/TRUNC` | ` https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#trunc` |
| `https://docs.teradata.com/r/SQL-Functions-Expressions-and-Predicates/July-2021/Arithmetic-Trigonometric-Hyperbolic-Operators/Functions/ZEROIFNULL` | ` https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/conditional_expressions#ifnull(*expression*, 0), https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/conditional_expressions#coalesce(*expression*, 0) ` |

#### Rounding functions

Where Teradata uses Gaussian and Banker algorithms to round
numerics, use the [`ROUND_HALF_EVEN` `RoundingMode`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/RoundingMode) in BigQuery:


    -- Teradata syntax
    round(3.45,1)

    -- BigQuery syntax
    round(CAST(3.45 as Numeric),1, 'ROUND_HALF_EVEN')

## DML syntax

This section addresses differences in data management language syntax between
Teradata and BigQuery.

### `INSERT` statement

Most Teradata `INSERT` statements are compatible with BigQuery.
The following table shows exceptions.

DML scripts in BigQuery have slightly different consistency
semantics than the equivalent statements in Teradata. For an overview of
snapshot isolation and session and transaction handling, see the
[`CREATE INDEX`](https://docs.cloud.google.com/bigquery/docs/migration/teradata-sql#create_index)
section elsewhere in this document.

| Teradata | BigQuery |
|---|---|
| ` https://docs.teradata.com/r/SQL-Data-Manipulation-Language/July-2021/Statement-Syntax/INSERT/INSERT-...-SELECT *table* VALUES (...); ` | ` https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/dml-syntax#insert_statement *table* (...) VALUES (...);` <br /> Teradata offers a `DEFAULT` keyword for non-nullable columns. <br /> **Note:** In BigQuery, omitting column names in the `INSERT` statement only works if values for all columns in the target table are included in ascending order based on their ordinal positions. |
| ` https://docs.teradata.com/r/SQL-Data-Manipulation-Language/July-2021/Statement-Syntax/INSERT/INSERT-...-SELECT *table* VALUES (1,2,3); https://docs.teradata.com/r/SQL-Data-Manipulation-Language/July-2021/Statement-Syntax/INSERT/INSERT-...-SELECT *table* VALUES (4,5,6); https://docs.teradata.com/r/SQL-Data-Manipulation-Language/July-2021/Statement-Syntax/INSERT/INSERT-...-SELECT *table* VALUES (7,8,9); ` | ` https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/dml-syntax#insert_statement *table* VALUES (1,2,3), (4,5,6), (7,8,9); ` Teradata has a concept of [multi-statement request (MSR)](https://docs.teradata.com/r/Basic-Teradata-Query-Reference/February-2022/Using-BTEQ/SQL-Requests/Request-Types), which sends multiple `INSERT` statements at a time. In BigQuery, this is not recommended due to the implicit transaction boundary between statements. Use [multi-value](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/dml-syntax#insert_values_with_subquery) `INSERT` instead. <br /> BigQuery allows concurrent `INSERT` statements but might [queue `UPDATE`](https://cloud.google.com/blog/products/data-analytics/dml-without-limits-now-in-bigquery). To improve performance, consider the following approaches: - Combine multiple rows in a single `INSERT` statement, instead of one row per `INSERT` operation. - Combine multiple DML statements (including `INSERT`) using a `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/dml-syntax#merge_statement` statement. - Use `CREATE TABLE ... AS SELECT` to create and populate new tables instead of `UPDATE` or `DELETE`, in particular when querying [partitioned fields](https://docs.cloud.google.com/bigquery/docs/querying-partitioned-tables#query_an_ingestion-time_partitioned_table) or [rollback or restore](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#for_system_time_as_of). |

### `UPDATE` statement

Most [Teradata `UPDATE` statements](https://docs.teradata.com/r/Teradata-MultiLoad-Reference/February-2022/Teradata-MultiLoad-Commands/UPDATE)
are compatible with BigQuery, except for the following items:

- When you use a `FROM` clause, the ordering of the `FROM` and `SET` clauses is reversed in Teradata and BigQuery.
- In GoogleSQL, each `UPDATE` statement must include the `WHERE` keyword, followed by a condition. To update all rows in the table, use `WHERE true`.

As a best practice, you should group multiple DML mutations instead of single
`UPDATE` and `INSERT` statements. DML scripts in BigQuery have
slightly different consistency semantics than equivalent statements in Teradata.
For an overview on snapshot isolation and session and transaction handling, see
the
[`CREATE INDEX`](https://docs.cloud.google.com/bigquery/docs/migration/teradata-sql#create_index)
section elsewhere in this document.

The following table shows Teradata `UPDATE` statements and
BigQuery statements that accomplish the same tasks.

For more information about `UPDATE` in BigQuery, see the
[BigQuery `UPDATE` examples](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/dml-syntax#update_examples)
in the DML documentation.

| Teradata |   | BigQuery |
|---|---|---|
| ` UPDATE **table_A** FROM *table_A*, *table_B* SET y = *table_B*.y, z = *table_B*.z + 1 WHERE *table_A*.x = *table_B*.x AND *table_A*.y IS NULL; ` |   | ` UPDATE *table_A* SET y = *table_B*.y, z = *table_B*.z + 1 FROM *table_B* WHERE *table_A*.x = *table_B*.x AND *table_A*.y IS NULL; ` |
| ` UPDATE *table* alias SET x = x + 1 WHERE f(x) IN (0, 1); ` |   | ` UPDATE *table* SET x = x + 1 WHERE f(x) IN (0, 1); ` |
| ` UPDATE *table_A* FROM *table_A*, *table_B*, *B* SET z = *table_B*.z WHERE *table_A*.x = *table_B*.x AND *table_A*.y = *table_B*.y; ` |   | ` UPDATE *table_A* SET z = *table_B*.z FROM *table_B* WHERE *table_A*.x = *table_B*.x AND *table_A*.y = *table_B*.y; ` |

### `DELETE` and `TRUNCATE` statements

Both the `DELETE` and `TRUNCATE` statements are supported ways to remove rows
from a table without affecting the table schema or indexes. `TRUNCATE` deletes
all the data, while `DELETE` removes the selected rows from the table.

In BigQuery, the `DELETE` statement must have a `WHERE` clause.
To delete all rows in the table (truncate), use `WHERE true`. To speed truncate
operations up for very large tables, we recommend using the
[`CREATE OR REPLACE TABLE ... AS SELECT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#creating_a_new_table_from_an_existing_table)
statement, using a `LIMIT 0` on the same table to replace itself. However,
make sure to manually add partitioning and clustering information when using it.

Teradata vacuums deleted rows later. This means that `DELETE` operations are
initially faster than in BigQuery, but they require resources
later, especially large-scale `DELETE` operations that impact the majority of a
table. To use a similar approach in BigQuery, we suggest reducing
the number of `DELETE` operations, such as by copying the rows not to be deleted
into a new table. Alternatively, you can [remove entire partitions](https://docs.cloud.google.com/bigquery/docs/managing-partitioned-tables#delete_a_partition).
Both of these options are designed to be faster operations than atomic DML mutations.

For more information about `DELETE` in BigQuery, see the
[`DELETE` examples](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/dml-syntax#delete_examples)
in the DML documentation.

| Teradata | BigQuery |
|---|---|
| ` BEGIN TRANSACTION; LOCKING TABLE *table_A* FOR EXCLUSIVE; DELETE FROM *table_A*; INSERT INTO *table_A* SELECT * FROM *table_B*; END TRANSACTION; ` | Replacing the contents of a table with query output is the equivalent of a transaction. You can do this with either a [query](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#bq_query) operation or a [copy](https://docs.cloud.google.com/bigquery/docs/managing-tables#copy-table) operation. <br /> Using a query operation: <br /> ` bq query --replace --destination_table *table_A* 'SELECT * FROM *table_B*'; ` <br /> Using a copy operation: <br /> `bq cp -f *table_A* *table_B*` |
| `DELETE *database*.*table* ALL;` | `DELETE FROM *table* WHERE TRUE;` Or for very large tables a faster way: `CREATE OR REPLACE *table* AS SELECT * FROM *table* LIMIT 0;` |

### `MERGE` statement

The `MERGE` statement can combine `INSERT`, `UPDATE`, and `DELETE` operations
into a single "upsert" statement and perform the operations atomically. The
`MERGE` operation must match at most one source row for each target row.
BigQuery and Teradata both follow ANSI Syntax.

Teradata's `MERGE` operation is limited to matching primary keys within one
[access module processor (AMP)](https://docs.teradata.com/r/Database-Introduction/July-2021/Teradata-Database-Hardware-and-Software-Architecture/Virtual-Processors/Access-Module-Processor).
In contrast, BigQuery has no size or column limitation for
`MERGE` operations, therefore using `MERGE` is a useful optimization. However,
if the `MERGE` is primarily a large delete, see optimizations for `DELETE`
elsewhere in this document.

DML scripts in BigQuery have slightly different
consistency semantics than equivalent statements in Teradata. For example,
Teradata's SET tables in session mode might
[ignore duplicates](https://docs.teradata.com/reader/b8dd8xEYJnxfsq4uFRrHQQ/4OgUH6g%7EQfr0TwCyhBrg9g)
during a `MERGE` operation. For an overview on handling MULTISET and
SET tables, snapshot isolation, and session and transaction handling,
see the
[`CREATE INDEX`](https://docs.cloud.google.com/bigquery/docs/migration/teradata-sql#create_index)
section elsewhere in this document.

### Rows-affected variables

In Teradata, the
[`ACTIVITY_COUNT`](https://docs.teradata.com/r/SQL-Stored-Procedures-and-Embedded-SQL/July-2021/Result-Code-Variables/ACTIVITY_COUNT)
variable is a Teradata ANSI SQL extension populated with the number of rows
affected by a DML statement.

The [`@@row_count` system variable](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/scripting#system_variables)
in the [Scripting feature](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/scripting) has similar functionality.
In BigQuery it would be more common to check the [`numDmlAffectedRows`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs/query#body.QueryResponse.FIELDS.num_dml_affected_rows)
return value in the audit logs or the [`INFORMATION_SCHEMA`](https://docs.cloud.google.com/bigquery/docs/information-schema-jobs) views.

## DDL syntax

This section addresses differences in data definition language syntax between
Teradata and BigQuery.

### `CREATE TABLE` statement

Most Teradata
[`CREATE TABLE`](https://docs.teradata.com/r/SQL-Data-Definition-Language-Syntax-and-Examples/July-2021/Table-Statements/CREATE-TABLE-and-CREATE-TABLE-AS)
statements are compatible with BigQuery, except for the following
syntax elements, which are not used in BigQuery:

- `https://docs.teradata.com/r/SQL-Data-Definition-Language-Syntax-and-Examples/July-2021/Table-Statements/CREATE-FOREIGN-TABLE/CREATE-FOREIGN-TABLE-Syntax-Elements/MULTISET`. See the `https://docs.cloud.google.com/bigquery/docs/migration/teradata-sql#create_index` section.
- `https://docs.teradata.com/r/SQL-Fundamentals/July-2021/Database-Objects/Tables/Volatile-Tables`. See the [Temporary tables](https://docs.cloud.google.com/bigquery/docs/migration/teradata-sql#temporary_tables) section.
- `[NO] https://docs.teradata.com/r/SQL-Data-Definition-Language-Syntax-and-Examples/July-2021/Table-Statements/ALTER-TABLE/ALTER-TABLE-Syntax-Elements/Table-Options/FALLBACK`. See the [Rollback](https://docs.cloud.google.com/bigquery/docs/migration/teradata-sql#rollback) section.
- `[NO] https://docs.teradata.com/r/SQL-Data-Definition-Language-Syntax-and-Examples/July-2021/Table-Statements/CREATE-TABLE-and-CREATE-TABLE-AS/Syntax-Elements/table_option/BEFORE-JOURNAL`, `[NO] https://docs.teradata.com/r/SQL-Data-Definition-Language-Syntax-and-Examples/July-2021/Table-Statements/CREATE-TABLE-and-CREATE-TABLE-AS/Syntax-Elements/table_option/AFTER-JOURNAL`
- `https://docs.teradata.com/r/SQL-Data-Definition-Language-Syntax-and-Examples/July-2021/Index-Statements/CREATE-JOIN-INDEX/CREATE-JOIN-INDEX-Syntax-Elements/table_option/CHECKSUM
  = DEFAULT | *val*`
- `https://docs.teradata.com/r/SQL-Data-Definition-Language-Syntax-and-Examples/July-2021/Table-Statements/ALTER-TABLE/ALTER-TABLE-Syntax-Elements/Table-Options/MERGEBLOCKRATIO`
- `https://docs.teradata.com/r/SQL-Data-Definition-Language-Syntax-and-Examples/July-2021/Index-Statements/CREATE-JOIN-INDEX/CREATE-JOIN-INDEX-Syntax-Elements/index/PRIMARY-INDEX
  (*col*, ...)`. See the `https://docs.cloud.google.com/bigquery/docs/migration/teradata-sql#create_index` section.
- `https://docs.teradata.com/r/SQL-Data-Definition-Language-Syntax-and-Examples/July-2021/Index-Statements/CREATE-JOIN-INDEX/CREATE-JOIN-INDEX-Syntax-Elements/index/PRIMARY-INDEX`. See the `https://docs.cloud.google.com/bigquery/docs/migration/teradata-sql#create_index` section.
- `https://docs.teradata.com/r/SQL-Data-Definition-Language-Syntax-and-Examples/July-2021/User-Profile-and-Role-Statements/MODIFY-PROFILE/MODIFY-PROFILE-Syntax-Elements/CONSTRAINT`
- `https://docs.teradata.com/r/SQL-Data-Definition-Language-Syntax-and-Examples/July-2021/Table-Statements/CREATE-TABLE-and-CREATE-TABLE-AS/Syntax-Elements/column_partition_definition/column_data_type_attribute/DEFAULT?tocId=TzUUl9nkAjYJJd2~gxqYkw`
- `https://docs.teradata.com/r/SQL-Data-Definition-Language-Detailed-Topics/July-2021/CREATE-TABLE-Options/CREATE-TABLE-Column-Definition-Clause/Identity-Columns`

For more information about `CREATE TABLE` in BigQuery, see the
[BigQuery `CREATE` examples](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_table_statement)
in the DML documentation.

#### Column options and attributes

The following column specifications for the `CREATE TABLE` statement are not
used in BigQuery:

- `https://docs.teradata.com/r/SQL-Data-Types-and-Literals/July-2021/Data-Type-Formats-and-Format-Phrases/FORMAT '*format*'`. See the Teradata [type formatting](https://docs.cloud.google.com/bigquery/docs/migration/teradata-sql#type_formatting) section.
- `https://docs.teradata.com/r/SQL-Data-Definition-Language-Syntax-and-Examples/July-2021/Table-Statements/CREATE-TABLE-and-CREATE-TABLE-AS/Syntax-Elements/column_partition_definition/column_data_type_attribute/CHARACTER-SET-server_character_set?tocId=c36aNoeODQWLvirPeTi70A
  *name*`. BigQuery always uses UTF-8 encoding.
- `[NOT] https://docs.teradata.com/r/SQL-Data-Definition-Language-Syntax-and-Examples/July-2021/Table-Statements/CREATE-TABLE-and-CREATE-TABLE-AS/Syntax-Elements/column_partition_definition/column_data_type_attribute/CASESPECIFIC?tocId=1b9AH4BoNABTrvxXZsbVLg`
- `https://docs.teradata.com/r/SQL-Data-Definition-Language-Syntax-and-Examples/July-2021/Table-Statements/ALTER-TABLE/ALTER-TABLE-Syntax-Elements/Column-Attributes/COMPRESS
  *val* | (*val*, ...)`

Teradata extends the ANSI standard with a column
[`TITLE`](https://docs.teradata.com/r/SQL-Data-Types-and-Literals/July-2021/Data-Type-Formats-and-Format-Phrases/TITLE)
option. This feature can be similarly implemented in BigQuery
using the column description as shown in the following table. Note this option
is not available for Views.

| Teradata | BigQuery |
|---|---|
| ` https://docs.teradata.com/r/SQL-Data-Definition-Language-Syntax-and-Examples/July-2021/Table-Statements/CREATE-TABLE-and-CREATE-TABLE-AS table ( *col1* VARCHAR(30) TITLE '*column desc*' ); ` | ` https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_table_statement *dataset*.*table* ( *col1* STRING https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#column_option_list(description="*column desc*") ); ` |

#### Temporary tables

Teradata supports
[volatile](https://docs.teradata.com/r/SQL-Fundamentals/July-2021/Database-Objects/Tables/Volatile-Tables)
tables, which are often used to store intermediate results in scripts. There are
several ways to achieve something similar to volatile tables in BigQuery:

- **[`CREATE TEMPORARY TABLE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_table_statement)**
  can be used in [Scripting](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/scripting),
  and is valid during the lifetime of the script. If the table has to exist
  beyond a script, you can use the other options in this list.

- **Dataset TTL:** Create a dataset that has a short time to live (for
  example, 1 hour) so that any tables created in the dataset are effectively
  temporary since they won't persist longer than the dataset's time to live.
  You can prefix all the table names in this dataset with `temp` to clearly
  denote that the tables are temporary.

- **Table TTL:** Create a table that has a table-specific short time to
  live using DDL statements similar to the following:

  ```
  CREATE TABLE temp.name (col1, col2, ...)
  OPTIONS(expiration_timestamp=TIMESTAMP_ADD(CURRENT_TIMESTAMP(), INTERVAL 1 HOUR));
  ```
- **`WITH` clause** : If a temporary table is needed only within the same
  block, use a temporary result using a
  [`WITH`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#with_clause)
  statement or subquery. This is the most efficient option.

An often-used pattern in Teradata scripts ([BTEQ](https://docs.teradata.com/r/Basic-Teradata-Query-Reference/February-2022/Introduction-to-BTEQ/Overview-of-BTEQ))
is to create a permanent table, insert a value in it, use this like a temporary
table in ongoing statements, and then delete or truncate the table afterwards.
In effect, this uses the table as a constant variable (a semaphore). This
approach is not efficient in BigQuery, and we recommend using
[real variables](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/scripting#set)
in [Scripting](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/scripting)
instead, or using `CREATE OR REPLACE` with [`AS SELECT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_table_statement)
query syntax to create a table that already has values in it.

### `CREATE VIEW` statement

The following table shows equivalents between Teradata and
BigQuery for the `CREATE VIEW` statement. The clauses for table
locking such as
[`LOCKING ROW FOR ACCESS`](https://docs.teradata.com/r/SQL-Data-Manipulation-Language/July-2021/Statement-Syntax/LOCKING-Request-Modifier/Usage-Notes/Using-LOCKING-ROW)
are not needed within BigQuery.

> [!NOTE]
> **Note:** Teradata does not directly support [materialized views](https://docs.cloud.google.com/bigquery/docs/materialized-views-intro) like BigQuery, only join indexes.

| Teradata | BigQuery | Notes |
|---|---|---|
| ` https://docs.teradata.com/r/SQL-Data-Definition-Language-Syntax-and-Examples/July-2021/View-Statements/CREATE-VIEW-and-REPLACE-VIEW *view_name* AS SELECT ... ` | ` https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_view_statement *view_name* AS SELECT ... ` |   |
| ` https://docs.teradata.com/r/SQL-Data-Definition-Language-Syntax-and-Examples/July-2021/View-Statements/CREATE-VIEW-and-REPLACE-VIEW *view_name* AS SELECT ... ` | ` https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_view_statement *view_name* AS SELECT ... ` |   |
| Not supported | ` https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_view_statement OPTIONS(https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#view_option_list) AS SELECT ... ` | Creates a new view only if the view does not currently exist in the specified dataset. |

### `CREATE [UNIQUE] INDEX` statement

Teradata requires indices for all tables and requires special workarounds like
[MULTISET](https://docs.teradata.com/r/SQL-Data-Definition-Language-Syntax-and-Examples/July-2021/Table-Statements/CREATE-FOREIGN-TABLE/CREATE-FOREIGN-TABLE-Syntax-Elements/MULTISET)
tables and [NoPI Tables](https://docs.teradata.com/r/Database-Design/July-2021/Primary-Index-Primary-AMP-Index-and-NoPI-Objects/NoPI-Tables-Column-Partitioned-NoPI-Tables-and-Column-Partitioned-NoPI-Join-Indexes)
to work with non-unique or non-indexed data.

BigQuery does not require indices. This section describes
approaches in BigQuery for how to create functionality similar
to how indexes are used in Teradata where there is an actual business logic need.

#### Indexing for performance

Because it's a column-oriented database with query and storage optimization,
BigQuery doesn't need explicit indexes. BigQuery
provides functionality such as
[partitioning and clustering](https://docs.cloud.google.com/bigquery/docs/clustered-tables)
as well as
[nested fields](https://docs.cloud.google.com/bigquery/docs/nested-repeated),
which can increase query efficiency and performance by optimizing how data is
stored.

Teradata does not support materialized views. However, it offers
[join indexes](https://docs.teradata.com/r/Database-Design/July-2021/Join-and-Hash-Indexes/Join-Indexes)
using the `CREATE JOIN INDEX` statement, which essentially materializes data
that's needed for a join. BigQuery does not need materialized
indexes to speed up performance, just as it doesn't need dedicated spool
space for joins.

For other optimization cases, [materialized views](https://docs.cloud.google.com/bigquery/docs/materialized-views-intro)
can be used.

#### Indexing for consistency (UNIQUE, PRIMARY INDEX)

In Teradata, a unique index can be used to prevent rows with non-unique keys in
a table. If a process tries to insert or update data that has a value that's
already in the index, the operation either fails with an index violation
(MULTISET tables) or silently ignores it (SET tables).

Because BigQuery doesn't provide explicit indexes, a `MERGE`
statement can be used instead to insert only unique records into a target table
from a staging table while discarding duplicate records. However, there is no
way to prevent a user with edit permissions from inserting a duplicate record,
because BigQuery never locks during `INSERT` operations.
To generate an error for duplicate records in BigQuery, you can
use a `MERGE` statement from a staging table, as shown in the following
example.

| Teradata |   | BigQuery |
|---|---|---|
| ` https://docs.teradata.com/r/SQL-Data-Definition-Language-Syntax-and-Examples/July-2021/Index-Statements/CREATE-INDEX [https://docs.teradata.com/r/SQL-Data-Definition-Language-Syntax-and-Examples/July-2021/Index-Statements/CREATE-INDEX/CREATE-INDEX-Syntax-Elements/index_specification/UNIQUE] https://docs.teradata.com/r/SQL-Data-Definition-Language-Syntax-and-Examples/July-2021/Index-Statements/CREATE-INDEX name; ` |   | `` MERGE `prototype.FIN_MERGE` t USING `prototype.FIN_TEMP_IMPORT` m ON t.*col1* = m.*col1* AND t.*col2* = m.*col2* WHEN MATCHED THEN UPDATE SET t.*col1* = ERROR(CONCAT('Encountered error for ', m.*col1*, ' ', m.*col2*)) WHEN NOT MATCHED THEN INSERT (*col1*,*col2*,*col3*,*col4*,*col5*,*col6*,*col7*,*col8*) VALUES(*col1*,*col2*,*col3*,*col4*,*col5*,*col6*,CURRENT_TIMESTAMP(),CURRENT_TIMESTAMP()); `` |

More often, users prefer to
[remove duplicates independently](https://docs.cloud.google.com/bigquery/streaming-data-into-bigquery#manually_removing_duplicates)
in order to find errors in downstream systems.  

BigQuery does not support `DEFAULT` and `IDENTITY` (sequences)
columns.

#### Indexing to achieve locking

Teradata provides resources in the
[access module processor](https://docs.teradata.com/r/Database-Introduction/July-2021/Teradata-Database-Hardware-and-Software-Architecture/Virtual-Processors/Access-Module-Processor)
(AMP); queries can consume all-AMP, single-AMP, or group-AMP resources. DDL
statements are all-AMP and therefore similar to a global DDL lock.
BigQuery doesn't have a lock mechanism like this and can run
concurrent queries and `INSERT` statements up to your quota;
only concurrent `UPDATE` DML statements have certain
[concurrency implications](https://cloud.google.com/blog/products/data-analytics/dml-without-limits-now-in-bigquery):
`UPDATE` operations against the same partition are queued to ensure snapshot
isolation, so you don't have to lock to prevent phantom reads or lost updates.

Because of these differences, the following Teradata elements are not used in
BigQuery:

- `ON COMMIT DELETE ROWS;`
- `ON COMMIT PRESERVE ROWS;`

## Procedural SQL statements

This section describes how to convert procedural SQL statements that are used
in stored procedures, functions, and triggers from Teradata
to BigQuery Scripting, procedures, or user-defined functions (UDFs).
All of these are available for system administrators to check using the
[`INFORMATION_SCHEMA`](https://docs.cloud.google.com/bigquery/docs/information-schema-routines)
views.

### `CREATE PROCEDURE` statement

Stored procedures are supported as part of BigQuery
[Scripting](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/scripting).

In BigQuery, Scripting refers to any use of control statements,
whereas procedures are named scripts (with arguments if needed) that can be
called from other Scripts and stored permanently, if needed.
A user-defined function (UDF) can also be written in JavaScript.

| Teradata | BigQuery |
|---|---|
| `https://docs.teradata.com/r/SQL-External-Routine-Programming/July-2021/C/C-External-Stored-Procedures/Installing-a-C/C-External-Stored-Procedure/CREATE-PROCEDURE-Statement` | ` https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_procedure` if a name is required, otherwise use inline with `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/scripting#begin` or in a single line with `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_function_statement`. |
| `https://docs.teradata.com/r/SQL-External-Routine-Programming/July-2021/C/C-External-Stored-Procedures/Installing-a-C/C-External-Stored-Procedure/CREATE-PROCEDURE-Statement` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_procedure` |
| `https://docs.teradata.com/r/Enterprise_IntelliFlex_VMware/SQL-Data-Manipulation-Language/Statement-Syntax/CALL` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/scripting#call` |

The sections that follow describe ways to convert existing Teradata procedural
statements to BigQuery Scripting statements that have similar
functionality.

### Variable declaration and assignment

BigQuery variables are valid during the lifetime of the script.

| Teradata | BigQuery |
|---|---|
| `https://docs.teradata.com/r/SQL-Stored-Procedures-and-Embedded-SQL/July-2021/SQL-Control-Statements/DECLARE` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/scripting#declare` |
| `https://docs.teradata.com/r/SQL-Stored-Procedures-and-Embedded-SQL/July-2021/SQL-Control-Statements/SET` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/scripting#set` |

### Error condition handlers

Teradata uses handlers on status codes in procedures for error control. In
BigQuery, error handling is a core feature of the main control
flow, similar to what other languages provide with `TRY ... CATCH` blocks.

| Teradata | BigQuery |
|---|---|
| `https://docs.teradata.com/r/SQL-Stored-Procedures-and-Embedded-SQL/July-2021/Condition-Handling/DECLARE-HANDLER-EXIT-Type FOR https://docs.teradata.com/r/SQL-Stored-Procedures-and-Embedded-SQL/July-2021/Condition-Handling/DECLARE-HANDLER-SQLEXCEPTION-Type` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/scripting#beginexception` |
| `https://docs.teradata.com/r/SQL-Stored-Procedures-and-Embedded-SQL/July-2021/Condition-Handling/Diagnostic-Statements/SIGNAL *sqlstate*` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/scripting#raise *message*` |
| `https://docs.teradata.com/r/SQL-Stored-Procedures-and-Embedded-SQL/July-2021/Condition-Handling/DECLARE-HANDLER-EXIT-Type FOR SQLSTATE VALUE 23505;` | Exception handlers that trigger for certain error conditions are not used by BigQuery. We recommend using `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/debugging-statements#assert` statements where exit conditions are used for pre-checks or debugging, because this is ANSI SQL:2011 compliant. |

The [`SQLSTATE`](https://docs.teradata.com/r/SQL-Stored-Procedures-and-Embedded-SQL/July-2021/Result-Code-Variables/SQLSTATE)
variable in Teradata is similar to the [`@@error` system variable](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/scripting#system_variables)
in BigQuery. In BigQuery, it is more common to
investigate errors using [audit logging](https://docs.cloud.google.com/bigquery/docs/reference/auditlogs)
or the [`INFORMATION_SCHEMA`](https://docs.cloud.google.com/bigquery/docs/information-schema-routines)
views.

### Cursor declarations and operations

Because BigQuery doesn't support cursors or sessions, the
following statements aren't used in BigQuery:

- `https://docs.teradata.com/r/SQL-Stored-Procedures-and-Embedded-SQL/July-2021/SQL-Cursor-Control-and-DML-Statements/DECLARE-CURSOR
  [FOR | WITH] ...`
- `https://docs.teradata.com/r/SQL-Stored-Procedures-and-Embedded-SQL/July-2021/SQL-Cursor-Control-and-DML-Statements/PREPARE
  *stmt_id* FROM *sql_str*;`
- `https://docs.teradata.com/r/SQL-Stored-Procedures-and-Embedded-SQL/July-2021/SQL-Cursor-Control-and-DML-Statements/OPEN-Stored-Procedures-Form
  *cursor_name* [USING *var*, ...];`
- `https://docs.teradata.com/r/SQL-Stored-Procedures-and-Embedded-SQL/July-2021/SQL-Cursor-Control-and-DML-Statements/FETCH-Stored-Procedures-Form
  *cursor_name* INTO *var*, ...;`
- `https://docs.teradata.com/r/SQL-Stored-Procedures-and-Embedded-SQL/July-2021/SQL-Cursor-Control-and-DML-Statements/CLOSE
  *cursor_name*;`

### Dynamic SQL statements

The [Scripting feature](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/scripting)
in BigQuery supports dynamic SQL statements like those shown in the following table.

| Teradata | BigQuery |
|---|---|
| `https://docs.teradata.com/r/SQL-Stored-Procedures-and-Embedded-SQL/July-2021/Dynamic-Embedded-SQL-Statements/Dynamic-SQL-Statement-Syntax/EXECUTE-IMMEDIATE *sql_str*;` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/scripting#execute_immediate *sql_str*;` |
| `https://docs.teradata.com/r/SQL-Stored-Procedures-and-Embedded-SQL/July-2021/SQL-Cursor-Control-and-DML-Statements/EXECUTE *stmt_id* [USING *var*,...];` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/scripting#execute_immediate *stmt_id* USING *var*;` |

The following Dynamic SQL statements are not used in BigQuery:

- `https://docs.teradata.com/r/SQL-Stored-Procedures-and-Embedded-SQL/July-2021/SQL-Cursor-Control-and-DML-Statements/PREPARE
  *stmt_id* FROM *sql_str*;`

### Flow-of-control statements

The [Scripting feature](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/scripting)
in BigQuery supports flow-of-control statements like those shown in the following table.

| Teradata | BigQuery |
|---|---|
| ` https://docs.teradata.com/r/SQL-Stored-Procedures-and-Embedded-SQL/July-2021/SQL-Control-Statements/IF/Usage-Notes/Valid-Forms-of-the-IF-Statement/IF-THEN-ELSE-END-IF *condition* https://docs.teradata.com/r/SQL-Stored-Procedures-and-Embedded-SQL/July-2021/SQL-Control-Statements/IF/Usage-Notes/Valid-Forms-of-the-IF-Statement/IF-THEN-ELSE-END-IF *stmts* https://docs.teradata.com/r/SQL-Stored-Procedures-and-Embedded-SQL/July-2021/SQL-Control-Statements/IF/Usage-Notes/Valid-Forms-of-the-IF-Statement/IF-THEN-ELSE-END-IF *stmts* https://docs.teradata.com/r/SQL-Stored-Procedures-and-Embedded-SQL/July-2021/SQL-Control-Statements/IF/Usage-Notes/Valid-Forms-of-the-IF-Statement/IF-THEN-ELSE-END-IF ` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/scripting#if *condition* https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/scripting#if *stmts* https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/scripting#if *stmts* https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/scripting#if` |
| ` *https://docs.teradata.com/r/SQL-Stored-Procedures-and-Embedded-SQL/July-2021/SQL-Control-Statements/LOOP*: https://docs.teradata.com/r/SQL-Stored-Procedures-and-Embedded-SQL/July-2021/SQL-Control-Statements/LOOP *stmts* https://docs.teradata.com/r/SQL-Stored-Procedures-and-Embedded-SQL/July-2021/SQL-Control-Statements/LOOP *https://docs.teradata.com/r/SQL-Stored-Procedures-and-Embedded-SQL/July-2021/SQL-Control-Statements/LOOP*; ` | GOTO-style block constructs are not used in BigQuery. We recommend rewriting them as [user-defined functions (UDFs)](https://docs.cloud.google.com/bigquery/docs/user-defined-functions) or use `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/debugging-statements#assert` statements where they are used for error handling. |
| ` https://docs.teradata.com/r/SQL-Stored-Procedures-and-Embedded-SQL/July-2021/SQL-Control-Statements/REPEAT *stmts* UNTIL *condition* https://docs.teradata.com/r/SQL-Stored-Procedures-and-Embedded-SQL/July-2021/SQL-Control-Statements/REPEAT; ` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/scripting#while *condition* https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/scripting#while *stmts* https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/scripting#while` |
| ` https://docs.teradata.com/r/SQL-Stored-Procedures-and-Embedded-SQL/July-2021/SQL-Control-Statements/LEAVE *outer_proc_label*; ` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/scripting#leave` is not used for GOTO-style blocks; it is used as a synonym for `BREAK` to leave a `WHILE` loop. |
| ` https://docs.teradata.com/r/SQL-Stored-Procedures-and-Embedded-SQL/July-2021/SQL-Control-Statements/LEAVE *label*; ` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/scripting#leave` is not used for GOTO-style blocks; it is used as a synonym for `BREAK` to leave a `WHILE` loop. |
| ` https://docs.teradata.com/r/SQL-Data-Definition-Language-Detailed-Topics/July-2021/CREATE-TRANSFORM-CREATE-VIEW/CREATE-VIEW-and-REPLACE-VIEW/WITH-RECURSIVE-Clause-Usage *temp_table* AS ( ... ); ` | Recursive queries (also known as recursive common table expressions (CTE)) are not used in BigQuery. They can be rewritten using arrays of `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#union`. |

The following flow-of-control statements are not used in BigQuery
because BigQuery doesn't use cursors or sessions:

- `
  https://docs.teradata.com/r/SQL-Stored-Procedures-and-Embedded-SQL/July-2021/SQL-Control-Statements/FOR *var* AS SELECT ... https://docs.teradata.com/r/SQL-Stored-Procedures-and-Embedded-SQL/July-2021/SQL-Control-Statements/FOR *stmts* https://docs.teradata.com/r/SQL-Stored-Procedures-and-Embedded-SQL/July-2021/SQL-Control-Statements/FOR;
  `
- `
  https://docs.teradata.com/r/SQL-Stored-Procedures-and-Embedded-SQL/July-2021/SQL-Control-Statements/FOR *var* AS *cur* CURSOR https://docs.teradata.com/r/SQL-Stored-Procedures-and-Embedded-SQL/July-2021/SQL-Control-Statements/FOR SELECT ... https://docs.teradata.com/r/SQL-Stored-Procedures-and-Embedded-SQL/July-2021/SQL-Control-Statements/FOR *stmts* https://docs.teradata.com/r/SQL-Stored-Procedures-and-Embedded-SQL/July-2021/SQL-Control-Statements/FOR;
  `

<br />

## Metadata and transaction SQL statements

| Teradata | BigQuery |
|---|---|
| `https://docs.teradata.com/r/SQL-Data-Definition-Language-Syntax-and-Examples/July-2021/Table-Statements/HELP-TABLE *table_name*;` `https://docs.teradata.com/r/SQL-Data-Definition-Language-Syntax-and-Examples/July-2021/View-Statements/HELP-VIEW *view_name*;` | ` SELECT * EXCEPT(is_generated, generation_expression, is_stored, is_updatable) FROM mydataset.INFORMATION_SCHEMA.COLUMNS; WHERE table_name=*table_name* ` <br /> The same query is valid to get column information for views. For more information, see the [Column view in the BigQuery `INFORMATION_SCHEMA`](https://docs.cloud.google.com/bigquery/docs/information-schema-tables#columns_view). |
| `SELECT * FROM dbc.tables WHERE tablekind = 'T';` <br /> (Teradata DBC view) | ` SELECT * EXCEPT(is_typed) FROM mydataset.INFORMATION_SCHEMA.TABLES; ` <br /> For more information, see [Introduction to BigQuery `INFORMATION_SCHEMA`](https://docs.cloud.google.com/bigquery/docs/information-schema-intro). |
| `https://docs.teradata.com/r/SQL-Data-Definition-Language-Syntax-and-Examples/July-2021/Statistics-Statements/HELP-STATISTICS-Optimizer-Form *table_name*;` | `APPROX_COUNT_DISTINCT(*col*)` |
| `COLLECT STATS USING SAMPLE ON *table_name* *column* (...);` | Not used in BigQuery. |
| `LOCKING TABLE *table_name* FOR EXCLUSIVE;` | BigQuery always uses snapshot isolation. For details, see [Consistency guarantees](https://docs.cloud.google.com/bigquery/docs/migration/teradata-sql#consistency_guarantees) elsewhere in this document. |
| `SET SESSION CHARACTERISTICS AS TRANSACTION ISOLATION LEVEL ...` | BigQuery always uses Snapshot Isolation. For details, see [Consistency guarantees](https://docs.cloud.google.com/bigquery/docs/migration/teradata-sql#consistency_guarantees) elsewhere in this document. |
| ` BEGIN TRANSACTION; SELECT ... END TRANSACTION; ` | BigQuery always uses Snapshot Isolation. For details, see [Consistency guarantees](https://docs.cloud.google.com/bigquery/docs/migration/teradata-sql#consistency_guarantees) elsewhere in this document. |
| `https://docs.teradata.com/r/SQL-Data-Manipulation-Language/July-2021/Query-and-Workload-Analysis-Statements/EXPLAIN-Request-Modifier ...` | Not used in BigQuery. <br /> Similar features are the [query plan explanation in the BigQuery web UI](https://docs.cloud.google.com/bigquery/query-plan-explanation) and the slot allocation visible in the `https://docs.cloud.google.com/bigquery/docs/information-schema-jobs` views and in [audit logging in Cloud Monitoring](https://docs.cloud.google.com/bigquery/docs/monitoring). |
| `BEGIN TRANSACTION; SELECT ... END TRANSACTION;` | ` BEGIN BEGIN TRANSACTION; COMMIT TRANSACTION; EXCEPTION WHEN ERROR THEN -- Roll back the transaction inside the exception handler. SELECT @@error.message; ROLLBACK TRANSACTION; END; ` |

### Multi-statement and multi-line SQL statements

Both Teradata and BigQuery support transactions (sessions)
and therefore support statements separated by semicolons that are consistently
executed together. For more information, see
[Multi-statement transactions](https://docs.cloud.google.com/bigquery/docs/transactions).

## Error codes and messages

Teradata error codes and BigQuery error codes are different.
Providing a REST API, BigQuery relies primarily on [HTTP status codes](https://docs.cloud.google.com/bigquery/docs/error-messages)
plus detailed error messages.

If your application logic is currently catching the following errors, try to
eliminate the source of the error, because BigQuery will not
return the same error codes.

- `SQLSTATE = '02000'`---"Row not found"
- `SQLSTATE = '21000'`---"Cardinality violation (Unique Index)"
- `SQLSTATE = '22000'`---"Data violation (Data Type)"
- `SQLSTATE = '23000'`---"Constraint Violation"

In BigQuery, it would be more common to use the [`INFORMATION_SCHEMA`](https://docs.cloud.google.com/bigquery/docs/information-schema-jobs) views
or [audit logging](https://docs.cloud.google.com/bigquery/docs/reference/auditlogs) to drill down into errors.

For information about how to handle errors in Scripting, see the sections that follow.

## Consistency guarantees and transaction isolation

Both Teradata and BigQuery are atomic---that is, ACID-compliant on
a per-mutation level across many rows. For example, a `MERGE` operation is
completely atomic, even with multiple inserted and updated values.

### Transactions

Teradata provides either Read Uncommitted (allowing dirty reads) or
Serializable transaction
[isolation level](https://docs.teradata.com/r/SQL-Data-Definition-Language-Detailed-Topics/July-2021/END-LOGGING-SET-TIME-ZONE/SET-SESSION-CHARACTERISTICS-AS-TRANSACTION-ISOLATION-LEVEL/Definition-of-Isolation-Level)
when running in session mode (instead of auto-commit mode). In the best case,
Teradata achieves strictly serializable isolation by using pessimistic locking
against a row hash across all columns of rows across all partitions. Deadlocks
are possible. DDL always forces a transaction boundary. Teradata Fastload jobs
run independently, but only on empty tables.

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
[pessimistic concurrency control](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-manipulation-language#limitations)
and [queues](https://cloud.google.com/blog/products/data-analytics/dml-without-limits-now-in-bigquery)
multiple `UPDATE` statements, automatically retrying in case of conflicts. `INSERT` DML statements and
load jobs can run concurrently and independently to append to tables.

### Rollback

Teradata supports
[two session rollback modes](https://docs.teradata.com/r/SQL-Request-and-Transaction-Processing/July-2021/Transaction-Processing/Rollback-Processing),
ANSI session mode and Teradata session mode (`SET SESSION CHARACTERISTICS` and
`SET SESSION TRANSACTION`), depending on which rollback mode you want. In
failure cases, the transaction might not be rolled back.

BigQuery supports the
[`ROLLBACK TRANSACTION` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/procedural-language#rollback_transaction).
There is no [`ABORT` statement](https://docs.teradata.com/r/Database-Utilities/July-2021/Dump-Unload/Load-Utility-dul/DUL-Commands/ABORT)
in BigQuery.

## Database limits

Always check
[the BigQuery public documentation](https://docs.cloud.google.com/bigquery/quotas)for
the latest quotas and limits. Many quotas for large-volume users can be raised
by contacting the Cloud Support team. The following table shows a comparison of
the Teradata and BigQuery database limits.

| Limit | Teradata | BigQuery |
|---|---|---|
| Tables per database | Unrestricted | Unrestricted |
| Columns per table | 2,048 | 10,000 |
| Maximum row size | 1 MB | 100 MB |
| Column name length | 128 Unicode chars | 300 Unicode characters |
| Table description length | 128 Unicode chars | 16,384 Unicode characters |
| Rows per table | Unlimited | Unlimited |
| Maximum SQL request length | 1 MB | 1 MB (maximum unresolved GoogleSQL query length) 12 MB (maximum resolved legacy and GoogleSQL query length) <br /> Streaming: - 10 MB (HTTP request size limit) - 10,000 (maximum rows per request) |
| Maximum request and response size | 7 MB (request), 16 MB (response) | 10 MB (request) and 10 GB (response), or virtually unlimited if you use pagination or the Cloud Storage API. |
| Maximum number of concurrent sessions | 120 per parsing engine (PE) | 1,000 concurrent multi-statement queries (can be raised with a [slot reservation](https://docs.cloud.google.com/bigquery/docs/slots)), 300 concurrent API requests per user. |
| Maximum number of concurrent (fast) loads | 30 (default 5) | No concurrency limit; jobs are queued. 100,000 load jobs per project per day. |