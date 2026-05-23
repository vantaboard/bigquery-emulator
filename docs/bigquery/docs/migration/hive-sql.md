# Apache Hive SQL translation guide

This document details the similarities and differences in SQL syntax between
Apache Hive and BigQuery to help you plan your
migration. To migrate your SQL scripts in bulk, use
[batch SQL translation](https://docs.cloud.google.com/bigquery/docs/batch-sql-translator). To translate
ad hoc queries, use
[interactive SQL translation](https://docs.cloud.google.com/bigquery/docs/interactive-sql-translator).

In some cases, there's no direct mapping between a SQL element in
Hive and BigQuery. However, in most cases,
BigQuery offers an alternative element to
Hive to help you achieve the same functionality, as
shown in the examples in this document.

The intended audience for this document is enterprise architects, database
administrators, application developers, and IT security specialists. It
assumes that you're familiar with Hive.

## Data types

Hive and BigQuery have different data type
systems. In most cases, you can map data types in Hive to
[BigQuery data types](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types)
with a few exceptions, such as `MAP` and `UNION`. Hive
supports more implicit type casting than BigQuery. As a result,
the batch SQL translator inserts many explicit casts.

| **Hive** | **BigQuery** |
|---|---|
| `TINYINT` | `INT64` |
| `SMALLINT` | `INT64` |
| `INT` | `INT64` |
| `BIGINT` | `INT64` |
| `DECIMAL` | `NUMERIC` |
| `FLOAT` | `FLOAT64` |
| `DOUBLE` | `FLOAT64` |
| `BOOLEAN` | `BOOL` |
| `STRING` | `STRING` |
| `VARCHAR` | `STRING` |
| `CHAR` | `STRING` |
| `BINARY` | `BYTES` |
| `DATE` | `DATE` |
| - | `DATETIME` |
| - | `TIME` |
| `TIMESTAMP` | `DATETIME/TIMESTAMP` |
| `INTERVAL` | - |
| `ARRAY` | `ARRAY` |
| `STRUCT` | `STRUCT` |
| `MAPS` | `STRUCT` with key values (`REPEAT` field) |
| `UNION` | `STRUCT` with different types |
| - | `GEOGRAPHY` |
| - | `JSON` |

## Query syntax

This section addresses differences in query syntax between
Hive and BigQuery.

### `SELECT` statement

Most Hive [`SELECT`](https://cwiki.apache.org/confluence/display/hive/languagemanual+select#LanguageManualSelect-SelectSyntax) statements are
compatible with BigQuery. The following table contains a list of
minor differences:

| **Case** | **Hive** | **BigQuery** |
|---|---|---|
| Subquery | ` SELECT * FROM ( SELECT 10 as col1, "test" as col2, "test" as col3 ) tmp_table; ` | ` SELECT * FROM ( SELECT 10 as col1, "test" as col2, "test" as col3 ); ` |
| Column filtering | `` SET hive.support.quoted.identifiers=none; SELECT `(col2|col3)?+.+` FROM ( SELECT 10 as col1, "test" as col2, "test" as col3 ) tmp_table; `` | ` SELECT * EXCEPT(col2,col3) FROM ( SELECT 10 as col1, "test" as col2, "test" as col3 ); ` |
| Exploding an array | ` SELECT tmp_table.pageid, adid FROM ( SELECT 'test_value' pageid, Array(1,2,3) ad_id) tmp_table LATERAL VIEW explode(tmp_table.ad_id) adTable AS adid; ` | ` SELECT tmp_table.pageid, ad_id FROM ( SELECT 'test_value' pageid, [1,2,3] ad_id) tmp_table, UNNEST(tmp_table.ad_id) ad_id; ` |

### `FROM` clause

The `FROM` clause in a query lists the table references from which data is
selected. In Hive, possible table references include
tables, views, and subqueries. BigQuery also supports all these
table references.

You can reference BigQuery tables in the `FROM` clause by using
the following:

- `[project_id].[dataset_id].[table_name]`
- `[dataset_id].[table_name]`
- `[table_name]`

BigQuery also supports [additional table references](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#from_clause):

- Historical versions of the table definition and rows using [`FOR SYSTEM_TIME AS OF`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#for_system_time_as_of)
- [Field paths](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#field_path), or any path that resolves to a field within a data type (such as a `STRUCT`)
- [Flattened arrays](https://docs.cloud.google.com/bigquery/docs/arrays#querying_nested_arrays)

### Comparison operators

The following table provides details about converting operators from
Hive to BigQuery:

| **Function or operator** | **Hive** | **BigQuery** |
|---|---|---|
| `-` Unary minus `*` Multiplication `/` Division `+` Addition `-` Subtraction | All [number types](https://cwiki.apache.org/confluence/display/Hive/LanguageManual+Types#LanguageManualTypes-NumericTypes) | All [number types](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#numeric_types). To prevent errors during the divide operation, consider using `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#safe_divide` or `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#ieee_divide`. |
| `~` Bitwise not `|` Bitwise OR `&` Bitwise AND `^` Bitwise XOR | Boolean data type | Boolean data type. |
| Left shift | ` shiftleft(TINYINT|SMALLINT|INT a, INT b) shiftleft(BIGINT a, INT b) ` | ` << ` Integer or bytes `A << B`, where `B` must be same type as `A` |
| Right shift | ` shiftright(TINYINT|SMALLINT|INT a, INT b) shiftright(BIGINT a, INT b) ` | `>>` Integer or bytes `A >> B`, where `B` must be same type as `A` |
| Modulus (remainder) | `X % Y` All [number types](https://cwiki.apache.org/confluence/display/Hive/LanguageManual+Types#LanguageManualTypes-NumericTypes) | `MOD(X, Y)` |
| Integer division | `A DIV B` and `A/B` for detailed precision | All [number types](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#numeric_types). Note: To prevent errors during the divide operation, consider using `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#safe_divide` or `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#ieee_divide`. |
| Unary negation | `!`, `NOT` | `NOT` |
| Types supporting equality comparisons | All [primitive types](https://cwiki.apache.org/confluence/display/Hive/LanguageManual+Types#LanguageManualTypes-Overview) | All [comparable types and `STRUCT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types). |
|   | ` a <=> b ` | Not supported. Translate to the following: ` (a = b AND b IS NOT NULL OR a IS NULL) ` |
|   | ` a <> b ` | Not supported. Translate to the following: ` NOT (a = b AND b IS NOT NULL OR a IS NULL) ` |
| Relational operators (` =, ==, !=, <, >, >= `) | All [primitive types](https://cwiki.apache.org/confluence/display/Hive/LanguageManual+UDF#LanguageManualUDF-RelationalOperators) | All [comparable types](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/operators#comparison_operators). |
| String comparison | `RLIKE`, `REGEXP` | `REGEXP_CONTAINS` built-in function. Uses BigQuery [regex syntax for string functions](https://github.com/google/re2/wiki/Syntax) for the regular expression patterns. |
| `[NOT] LIKE, [NOT] BETWEEN, IS [NOT] NULL` | `A [NOT] BETWEEN B AND C, A IS [NOT] (TRUE|FALSE), A [NOT] LIKE B` | Same as Hive. In addition, BigQuery also supports the [`IN` operator](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/operators#in_operators). |

### JOIN conditions

Both Hive and BigQuery support the
following types of joins:

- `[INNER] JOIN`

- `LEFT [OUTER] JOIN`

- `RIGHT [OUTER] JOIN`

- `FULL [OUTER] JOIN`

- `CROSS JOIN` and the equivalent implicit [comma cross join](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#cross_join)

For more information, see
[Join operation](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#join_types)
and [Hive joins](https://cwiki.apache.org/confluence/display/hive/languagemanual+joins).

### Type conversion and casting

The following table provides details about converting functions from
Hive to BigQuery:

| **Function or operator** | **Hive** | **BigQuery** |
|---|---|---|
| Type casting | When a cast fails, \`NULL\` is returned. | Same syntax as Hive. For more information about BigQuery type conversion rules, see [Conversion rules](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/conversion_rules). If cast fails, you see an error. To have the same behavior as Hive, use `SAFE_CAST` instead. |
| `SAFE` function calls |   | If you prefix function calls with [`SAFE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/functions-reference#safe_prefix), the function returns `NULL` instead of reporting failure. For example, `SAFE.SUBSTR('foo', 0, -2) AS safe_output;` returns `NULL`. Note: When casting safely without errors, use `SAFE_CAST`. |

#### Implicit conversion types

When migrating to BigQuery, you need to convert most of your
[Hive implicit conversions](https://cwiki.apache.org/confluence/display/hive/languagemanual+types#LanguageManualTypes-AllowedImplicitConversions) to
BigQuery explicit conversions except for the following
data types, which BigQuery implicitly converts.

| **From BigQuery type** | **To BigQuery type** |
|---|---|
| `INT64` | `FLOAT64`, `NUMERIC`, `BIGNUMERIC` |
| `BIGNUMERIC` | `FLOAT64` |
| `NUMERIC` | `BIGNUMERIC`, `FLOAT64` |

BigQuery also performs implicit conversions for the following
literals:

| **From BigQuery type** | **To BigQuery type** |
|---|---|
| `STRING` literal (for example, `"2008-12-25"`) | `DATE` |
| `STRING` literal (for example, `"2008-12-25 15:30:00"`) | `TIMESTAMP` |
| `STRING` literal (for example, `"2008-12-25T07:30:00"`) | `DATETIME` |
| `STRING` literal (for example, `"15:30:00"`) | `TIME` |

#### Explicit conversion types

If you want to convert Hive data types that BigQuery doesn't
implicitly convert, use the BigQuery [`CAST(expression AS type)` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/conversion_functions#cast).

## Functions

This section covers common functions used in Hive and
BigQuery.

### Aggregate functions

The following table shows mappings between common Hive
aggregate, statistical aggregate, and approximate aggregate functions with their
BigQuery equivalents:

| **Hive** | **BigQuery** |
|---|---|
| `count(DISTINCT expr[, expr...])` | `count(DISTINCT expr[, expr...])` |
| `percentile_approx(DOUBLE col, array(p1 [, p2]...) [, B]) WITHIN GROUP (ORDER BY expression)` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/approximate_aggregate_functions#approx_quantiles(expression, 100)[OFFSET(CAST(TRUNC(percentile * 100) as INT64))]` BigQuery doesn't support the rest of the arguments that Hive defines. |
| `https://cwiki.apache.org/confluence/display/hive/languagemanual+udf#LanguageManualUDF-Built-inAggregateFunctions(UDAF)` | `AVG` |
| `https://cwiki.apache.org/confluence/display/hive/languagemanual+udf#LanguageManualUDF-OperatorsprecedencesOperatorsPrecedencesOperatorsPrecedences` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#bit_or / X | Y` |
| `https://cwiki.apache.org/confluence/display/hive/languagemanual+udf#LanguageManualUDF-OperatorsprecedencesOperatorsPrecedencesOperatorsPrecedences` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#bit_xor / X ^ Y` |
| `https://cwiki.apache.org/confluence/display/hive/languagemanual+udf#LanguageManualUDF-OperatorsprecedencesOperatorsPrecedencesOperatorsPrecedences` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#bit_and / X & Y` |
| `https://cwiki.apache.org/confluence/display/hive/languagemanual+udf#LanguageManualUDF-Built-inAggregateFunctions(UDAF)` | `COUNT` |
| `COLLECT_SET(col), \ COLLECT_LIST(col`) | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#array_agg(col)` |
| `https://cwiki.apache.org/confluence/display/hive/languagemanual+udf#LanguageManualUDF-Built-inAggregateFunctions(UDAF)` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#count` |
| `MAX` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#max` |
| `MIN` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#min` |
| `REGR_AVGX` | `AVG(` `IF(dep_var_expr is NULL` `OR ind_var_expr is NULL,` `NULL, ind_var_expr)` `)` |
| `REGR_AVGY` | `AVG(` `IF(dep_var_expr is NULL` `OR ind_var_expr is NULL,` `NULL, dep_var_expr)` `)` |
| `REGR_COUNT` | `SUM(` `IF(dep_var_expr is NULL` `OR ind_var_expr is NULL,` `NULL, 1)` `)` |
| `REGR_INTERCEPT` | `AVG(dep_var_expr) ` `- AVG(ind_var_expr) ` `* (COVAR_SAMP(ind_var_expr,dep_var_expr)` ` / VARIANCE(ind_var_expr)` ` )` |
| `REGR_R2` | `(COUNT(dep_var_expr) *` `SUM(ind_var_expr * dep_var_expr) -` `SUM(dep_var_expr) * SUM(ind_var_expr))` `/ SQRT(` `(COUNT(ind_var_expr) *` `SUM(POWER(ind_var_expr, 2)) *` `POWER(SUM(ind_var_expr),2)) *` `(COUNT(dep_var_expr) *` `SUM(POWER(dep_var_expr, 2)) *` `POWER(SUM(dep_var_expr), 2)))` |
| `REGR_SLOPE` | `COVAR_SAMP(ind_var_expr,` `dep_var_expr)` `/ VARIANCE(ind_var_expr)` |
| `REGR_SXX` | `SUM(POWER(ind_var_expr, 2)) - COUNT(ind_var_expr) * POWER(AVG(ind_var_expr),2)` |
| `REGR_SXY` | `SUM(ind_var_expr*dep_var_expr) - COUNT(ind_var_expr) * AVG(ind) * AVG(dep_var_expr)` |
| `REGR_SYY` | `SUM(POWER(dep_var_expr, 2)) - COUNT(dep_var_expr) * POWER(AVG(dep_var_expr),2)` |
| `https://cwiki.apache.org/confluence/display/Hive/Enhanced+Aggregation%2C+Cube%2C+Grouping+and+Rollup#EnhancedAggregation,Cube,GroupingandRollup-CubesandRollups` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#group_by_clause` |
| `STDDEV_POP` | `STDDEV_POP` |
| `STDDEV_SAMP` | `STDDEV_SAMP, STDDEV` |
| `SUM` | `SUM` |
| `VAR_POP` | `VAR_POP` |
| `VAR_SAMP` | `VAR_SAMP, VARIANCE` |
| `CONCAT_WS` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#string_agg` |

### Analytical functions

The following table shows mappings between common Hive
analytical functions with their BigQuery equivalents:

| **Hive** | **BigQuery** |
|---|---|
| `AVG` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#avg` |
| `COUNT` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#count` |
| `COVAR_POP` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#covar_pop` |
| `COVAR_SAMP` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#covar_samp` |
| `CUME_DIST` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/numbering_functions#cume_dist` |
| `DENSE_RANK` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/numbering_functions#dense_rank` |
| `https://cwiki.apache.org/confluence/display/Hive/LanguageManual+WindowingAndAnalytics#LanguageManualWindowingAndAnalytics-EnhancementstoHiveQL` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/navigation_functions#first_value` |
| `LAST_VALUE` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/navigation_functions#last_value` |
| `LAG` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/navigation_functions#lag` |
| `LEAD` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/navigation_functions#lead` |
| `COLLECT_LIST, \ COLLECT_SET` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#array_agg` `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#array_concat_agg` |
| `MAX` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#max` |
| `MIN` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#min` |
| `NTILE` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/numbering_functions#ntile(constant_integer_expression)` |
| `https://cwiki.apache.org/confluence/display/Hive/LanguageManual+WindowingAndAnalytics#LanguageManualWindowingAndAnalytics-EnhancementstoHiveQL` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/numbering_functions#percent_rank` |
| `RANK ()` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/numbering_functions#rank` |
| `ROW_NUMBER` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/numbering_functions#row_number` |
| `STDDEV_POP` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#stddev_pop` |
| `STDDEV_SAMP` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#stddev_samp, https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#stddev` |
| `SUM` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#sum` |
| `VAR_POP` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#var_pop` |
| `VAR_SAMP` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#var_samp, https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#variance` |
| `VARIANCE` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#variance` |
| `https://cwiki.apache.org/confluence/display/Hive/LanguageManual+UDF#LanguageManualUDF-Built-inFunctions` | A user-defined function (UDF) can be used. |

### Date and time functions

The following table shows mappings between common Hive
date and time functions and their BigQuery equivalents:

|---|---|
| `DATE_ADD` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#date_add` |
| `DATE_SUB` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#date_sub` |
| `CURRENT_DATE` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#current_date` |
| `CURRENT_TIME` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/time_functions#current_time` |
| `CURRENT_TIMESTAMP` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/datetime_functions#current_datetime` is recommended, as this value is timezone-free and synonymous with `CURRENT_TIMESTAMP` \\ `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#current_timestamp` in Hive. |
| `EXTRACT(field FROM source)` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/datetime_functions#extract` |
| `LAST_DAY` | ` DATE_SUB( DATE_TRUNC( DATE_ADD( date_expression, INTERVAL 1 MONTH ` ` ), MONTH ), INTERVAL 1 DAY) ` |
| `MONTHS_BETWEEN` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#date_diff(date_expression, date_expression, MONTH)` |
| `NEXT_DAY` | ` DATE_ADD( DATE_TRUNC( date_expression, WEEK(day_value) ), INTERVAL 1 WEEK ` ` ) ` |
| `TO_DATE` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#parse_date` |
| `FROM_UNIXTIME` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#unix_seconds` |
| `FROM_UNIXTIMESTAMP` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#format_timestamp` |
| ` YEAR \ QUARTER \ MONTH \ HOUR \ MINUTE \ SECOND \ WEEKOFYEAR ` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#extract` |
| `DATEDIFF` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#date_diff` |

BigQuery offers the following additional date and time functions:

|---|---|---|
| - `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/datetime_functions#current_datetime` - `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#date_from_unix_date` - `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#date_trunc` - `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/datetime_functions#datetime` - `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/datetime_functions#datetime_trunc` - `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#format_date` - `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/datetime_functions#format_datetime` - `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/time_functions#format_time` | - `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#format_timestamp` - `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/datetime_functions#parse_datetime` - `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/time_functions#parse_time` - `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#string` - `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/time_functions#time` - `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/time_functions#time_add` - `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/time_functions#time_diff` - `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/time_functions#time_sub` - `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/time_functions#time_trunc` - `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#timestamp` - `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#timestamp_add` | - `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#timestamp_diff` - `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#timestamp_micros` - `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#timestamp_millis` - `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#timestamp_seconds` - `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#timestamp_sub` - `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#timestamp_trunc` - `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#unix_date` - `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#unix_micros` - `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#unix_millis` - `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#unix_seconds` |

### String functions

The following table shows mappings between Hive string
functions and their BigQuery equivalents:

| **Hive** | **BigQuery** |
|---|---|
| `ASCII` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#to_code_points` |
| `HEX` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#to_hex` |
| `LENGTH` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#char_length` |
| `LENGTH` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#character_length` |
| `CHR` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#code_points_to_string` |
| `CONCAT` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#concat` |
| `LOWER` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#lower` |
| `LPAD` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#lpad` |
| `LTRIM` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#ltrim` |
| `REGEXP_EXTRACT` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#regexp_extract` |
| `REGEXP_REPLACE` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#regexp_replace` |
| `REPLACE` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#replace` |
| `REVERSE` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#reverse` |
| `RPAD` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#rpad` |
| `RTRIM` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#rtrim` |
| `SOUNDEX` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#soundex` |
| `SPLIT` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#split(instring, delimiter)[ORDINAL(tokennum)]` |
| `SUBSTR, \ SUBSTRING` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#substr` |
| `TRANSLATE` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#translate` |
| `LTRIM` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#ltrim` |
| `RTRIM` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#rtrim` |
| `TRIM` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#trim` |
| `UPPER` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#upper` |

BigQuery offers the following additional string functions:

|---|---|
| - `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#byte_length` - `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#code_points_to_bytes` - `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#ends_with` - `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#from_base32` - `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#from_base64` - `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#from_hex` - `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#normalize` - `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#normalize_and_casefold` | - `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#repeat` - `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#safe_convert_bytes_to_string` - `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#split` - `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#starts_with` - `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#strpos` - `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#to_base32` - `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#to_base64` - `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#to_code_points` |

### Math functions

The following table shows mappings between Hive
[math functions](https://cwiki.apache.org/confluence/display/Hive/LanguageManual+UDF#LanguageManualUDF-MathematicalFunctions)
and their BigQuery equivalents:

| **Hive** | **BigQuery** |
|---|---|
| `ABS` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#abs` |
| `ACOS` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#acos` |
| `ASIN` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#asin` |
| `ATAN` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#atan` |
| `CEIL` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#ceil` |
| `CEILING` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#ceiling` |
| `COS` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#cos` |
| `FLOOR` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#floor` |
| `GREATEST` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#greatest` |
| `LEAST` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#least` |
| `LN` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#ln` |
| `LNNVL` | Use with `ISNULL`. |
| `LOG` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#log` |
| `MOD (% operator)` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#mod` |
| `POWER` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#power, https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#pow` |
| `RAND` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#rand` |
| `ROUND` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#round` |
| `SIGN` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#sign` |
| `SIN` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#sin` |
| `SQRT` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#sqrt` |
| `HASH` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/hash_functions` |
| `STDDEV_POP` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#stddev_pop` |
| `STDDEV_SAMP` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#stddev_samp` |
| `TAN` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#tan` |
| `TRUNC` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#trunc` |
| `NVL` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/conditional_expressions#ifnull(expr, 0),https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/conditional_expressions#coalesce(exp, 0)` |

BigQuery offers the following additional math functions:

- `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#div`
- `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#ieee_divide`
- `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#is_inf`
- `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#is_nan`
- `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#log10`
- `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#safe_divide`

### Logical and conditional functions

The following table shows mappings between Hive logical
and conditional functions and their BigQuery equivalents:

| **Hive** | **BigQuery** |
|---|---|
| `https://cwiki.apache.org/confluence/display/hive/languagemanual+udf#LanguageManualUDF-ConditionalFunctions` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/conditional_expressions#case_expr` |
| `https://cwiki.apache.org/confluence/display/hive/languagemanual+udf#LanguageManualUDF-ConditionalFunctions` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/conditional_expressions#coalesce` |
| `https://cwiki.apache.org/confluence/display/hive/languagemanual+udf#LanguageManualUDF-ConditionalFunctions` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/conditional_expressions#ifnull(expr, 0),https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/conditional_expressions#coalesce(exp, 0)` |
| `https://cwiki.apache.org/confluence/display/hive/languagemanual+udf#LanguageManualUDF-ConditionalFunctions` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/conditional_expressions#nullif` |
| `https://cwiki.apache.org/confluence/display/hive/languagemanual+udf#LanguageManualUDF-ConditionalFunctions` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/conditional_expressions#if` |
| `https://cwiki.apache.org/confluence/display/hive/languagemanual+udf#LanguageManualUDF-ConditionalFunctions` | `IS NULL` |
| `https://cwiki.apache.org/confluence/display/hive/languagemanual+udf#LanguageManualUDF-ConditionalFunctions` | `IS NOT NULL` |
| `https://cwiki.apache.org/confluence/display/hive/languagemanual+udf#LanguageManualUDF-ConditionalFunctions` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/conditional_expressions#nullif` |

### UDFs and UDAFs

Apache Hive supports writing user defined functions (UDFs) in Java.
You can load UDFs into Hive to be used in regular queries.
[BigQuery UDFs](https://docs.cloud.google.com/bigquery/docs/user-defined-functions)
must be written in GoogleSQL or JavaScript. Converting the Hive UDFs
to SQL UDFs is recommended because SQL UDFs perform
better. If you need to use JavaScript, read
[Best Practices for JavaScript UDFs](https://docs.cloud.google.com/bigquery/docs/user-defined-functions#best-practices-for-javascript-udfs).
For other languages, BigQuery supports
[remote functions](https://docs.cloud.google.com/bigquery/docs/remote-functions)
that let you invoke your functions in
[Cloud Run functions](https://docs.cloud.google.com/functions/docs/concepts/overview) or
[Cloud Run](https://docs.cloud.google.com/run/docs/overview/what-is-cloud-run) from
GoogleSQL queries.

BigQuery does not support user-defined aggregation functions
(UDAFs).

## DML syntax

This section addresses differences in data manipulation language (DML)
syntax between Hive and BigQuery.

### `INSERT` statement

Most Hive `INSERT` statements are compatible with
BigQuery. The following table shows exceptions:

| **Hive** | **BigQuery** |
|---|---|
| `INSERT INTO TABLE tablename [PARTITION (partcol1[=val1], partcol2[=val2] ...)] VALUES values_row [, values_row ...]` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/dml-syntax#insert_statement table (...) VALUES (...);` Note: In BigQuery, omitting column names in the `INSERT` statement only works if values for all columns in the target table are included in ascending order based on their ordinal positions. |
| `INSERT OVERWRITE [LOCAL] DIRECTORY directory1` ` [ROW FORMAT row_format] [STORED AS file_format] (Note: Only available starting with Hive 0.11.0)` ` SELECT ... FROM ...` | BigQuery doesn't support the insert-overwrite operations. This Hive syntax can be migrated to `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/dml-syntax#truncate_table_statement` and `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/dml-syntax#insert_statement` statements. |

BigQuery imposes
[DML quotas](https://docs.cloud.google.com/bigquery/quotas#data-manipulation-language-statements) that
restrict the number of DML statements that you can execute daily. To make the
best use of your quota, consider the following approaches:

- Combine multiple rows in a single `INSERT` statement, instead of one row for
  each `INSERT` operation.

- Combine multiple DML statements (including `INSERT`) by using a `MERGE`
  statement.

- Use `CREATE TABLE ... AS SELECT` to create and populate new tables.

### `UPDATE` statement

Most Hive `UPDATE` statements are compatible with
BigQuery. The following table shows exceptions:

| **Hive** | **BigQuery** |
|---|---|
| `UPDATE tablename SET column = value [, column = value ...] [WHERE expression]` | `UPDATE table` `SET column = expression [,...]` `[FROM ...]` `WHERE TRUE` Note: All `UPDATE` statements in BigQuery require a `WHERE` keyword, followed by a condition. |

### `DELETE` and `TRUNCATE` statements

You can use `DELETE` or `TRUNCATE` statements to remove rows from a table
without affecting the table schema or indexes.

In BigQuery, the `DELETE` statement must have a `WHERE` clause.
For more information about `DELETE` in BigQuery, see
[`DELETE` examples](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/dml-syntax#delete_examples).

| **Hive** | **BigQuery** |
|---|---|
| `https://cwiki.apache.org/confluence/display/Hive/LanguageManual+DML#LanguageManualDML-Delete FROM tablename [WHERE expression]` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/dml-syntax#delete_statement table_name` `WHERE TRUE` BigQuery `DELETE` statements require a `WHERE `clause. |
| `https://cwiki.apache.org/confluence/display/hive/languagemanual+ddl#LanguageManualDDL-TruncateTable [TABLE] table_name [PARTITION partition_spec];` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/dml-syntax#truncate_table_statement [[project_name.]dataset_name.]table_name` |

### `MERGE` statement

The `MERGE` statement can combine `INSERT`, `UPDATE`, and `DELETE` operations
into a single *upsert* statement and perform the operations. The
`MERGE` operation must match one source row at most for each target row.

| **Hive** | **BigQuery** |
|---|---|
| `https://cwiki.apache.org/confluence/display/Hive/LanguageManual+DML#LanguageManualDML-Merge AS T USING AS S` `ON ` `WHEN MATCHED [AND ] THEN UPDATE SET ` `WHEN MATCHED [AND ] THEN DELETE` `WHEN NOT MATCHED [AND ] THEN INSERT VALUES` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/dml-syntax#merge_statement` `USING source` `ON target.key = source.key` `WHEN MATCHED AND source.filter = 'filter_exp' THEN` ` UPDATE SET ` ` target.col1 = source.col1,` ` target.col2 = source.col2,` ` ...` Note: You must list all columns that need to be updated. |

### `ALTER` statement

The following table provides details about converting `CREATE VIEW` statements
from Hive to BigQuery:

| **Function** | **Hive** | **BigQuery** |
|---|---|---|
| `Rename table` | `https://cwiki.apache.org/confluence/display/hive/languagemanual+ddl#LanguageManualDDL-RenameTable table_name RENAME TO new_table_name;` | Not supported. A workaround is to use a copy job with the name that you want as the destination table, and then delete the old one. `bq copy project.dataset.old_table project.dataset.new_table` `bq rm --table project.dataset.old_table` |
| `Table properties` | `https://cwiki.apache.org/confluence/display/hive/languagemanual+ddl#LanguageManualDDL-AlterTableProperties table_name SET TBLPROPERTIES table_properties;` ` ` `table_properties:` ` : (property_name = property_value, property_name = property_value, ... )` **`Table Comment:`** `ALTER TABLE table_name SET TBLPROPERTIES ('comment' = new_comment);` | `{ALTER TABLE | ALTER TABLE IF EXISTS}` `table_name` `SET OPTIONS(https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#table_set_options_list)` |
| `SerDe properties (Serialize and deserialize)` | `https://cwiki.apache.org/confluence/display/hive/languagemanual+ddl#LanguageManualDDL-AddSerDeProperties table_name [PARTITION partition_spec] SET SERDE serde_class_name [WITH SERDEPROPERTIES serde_properties];` ` ` `ALTER TABLE table_name [PARTITION partition_spec] SET SERDEPROPERTIES serde_properties;` ` ` `serde_properties:` ` : (property_name = property_value, property_name = property_value, ... )` | Serialization and deserialization is managed by the BigQuery service and isn't user configurable. To learn how to let BigQuery read data from CSV, JSON, AVRO, PARQUET, or ORC files, see [Create Cloud Storage external tables](https://docs.cloud.google.com/bigquery/docs/external-data-cloud-storage). Supports CSV, JSON, AVRO, and PARQUET export formats. For more information, see [Export formats and compression types](https://docs.cloud.google.com/bigquery/docs/exporting-data#export_formats_and_compression_types). |
| `Table storage properties` | `https://cwiki.apache.org/confluence/display/hive/languagemanual+ddl#LanguageManualDDL-AlterTableStorageProperties table_name CLUSTERED BY (col_name, col_name, ...) [SORTED BY (col_name, ...)]` ` INTO num_buckets BUCKETS;` | Not supported for the `ALTER` statements. |
| `Skewed table` | **`Skewed:`** `https://cwiki.apache.org/confluence/display/hive/languagemanual+ddl#LanguageManualDDL-AlterTableSkewedorStoredasDirectories table_name SKEWED BY (col_name1, col_name2, ...)` ` ON ([(col_name1_value, col_name2_value, ...) [, (col_name1_value, col_name2_value), ...]` ` [STORED AS DIRECTORIES];` **`Not Skewed:`** `ALTER TABLE table_name NOT SKEWED;` **`Not Stored as Directories:`** `ALTER TABLE table_name NOT STORED AS DIRECTORIES;` **`Skewed Location:`** `ALTER TABLE table_name SET SKEWED LOCATION (col_name1="location1" [, col_name2="location2", ...] );` | Balancing storage for performance queries is managed by the BigQuery service and isn't configurable. |
| `Table constraints` | `https://cwiki.apache.org/confluence/display/hive/languagemanual+ddl#LanguageManualDDL-AlterTableConstraints table_name ADD CONSTRAINT constraint_name PRIMARY KEY (column, ...) DISABLE NOVALIDATE;` `ALTER TABLE table_name ADD CONSTRAINT constraint_name FOREIGN KEY (column, ...) REFERENCES table_name(column, ...) DISABLE NOVALIDATE RELY;` `ALTER TABLE table_name DROP CONSTRAINT constraint_name;` | `ALTER TABLE [[project_name.]dataset_name.]table_name` `ADD [CONSTRAINT [IF NOT EXISTS] [constraint_name]] constraint NOT ENFORCED;` `ALTER TABLE [[project_name.]dataset_name.]table_name` `ADD PRIMARY KEY(column_list) NOT ENFORCED;` For more information, see [`ALTER TABLE ADD PRIMARY KEY` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#alter_table_add_primary_key_statement). |
| `Add partition` | `https://cwiki.apache.org/confluence/display/hive/languagemanual+ddl#LanguageManualDDL-AlterEitherTableorPartition table_name ADD [IF NOT EXISTS] PARTITION partition_spec [LOCATION 'location'][, PARTITION partition_spec [LOCATION 'location'], ...];` ` ` `partition_spec:` ` : (partition_column = partition_col_value, partition_column = partition_col_value, ...)` | Not supported. Additional partitions are added as needed when data with new values in the partition columns are loaded. For more information, see [Managing partitioned tables](https://docs.cloud.google.com/bigquery/docs/managing-partitioned-tables). |
| `Rename partition` | `https://cwiki.apache.org/confluence/display/hive/languagemanual+ddl#LanguageManualDDL-RenamePartition table_name PARTITION partition_spec RENAME TO PARTITION partition_spec;` | Not supported. |
| `Exchange partition` | `-- Move partition from table_name_1 to table_name_2` `ALTER TABLE table_name_2 https://cwiki.apache.org/confluence/display/hive/languagemanual+ddl#LanguageManualDDL-ExchangePartition PARTITION (partition_spec) WITH TABLE table_name_1;` `-- multiple partitions` `ALTER TABLE table_name_2 EXCHANGE PARTITION (partition_spec, partition_spec2, ...) WITH TABLE table_name_1;` | Not supported. |
| `Recover partition` | `https://cwiki.apache.org/confluence/display/hive/languagemanual+ddl#LanguageManualDDL-RecoverPartitions(MSCKREPAIRTABLE) TABLE table_name [ADD/DROP/SYNC PARTITIONS];` | Not supported. |
| `Drop partition` | `https://cwiki.apache.org/confluence/display/hive/languagemanual+ddl#LanguageManualDDL-DropPartitions [IF EXISTS] PARTITION partition_spec[, PARTITION partition_spec, ...]` ` [IGNORE PROTECTION] [PURGE];` | Supported using the following methods: - `bq rm 'mydataset.table_name$partition_id'` - `DELETE from table_name$partition_id WHERE 1=1` - For more information, see [Delete a partition](https://docs.cloud.google.com/bigquery/docs/managing-partitioned-tables#delete_a_partition). |
| `(Un)Archive partition` | `https://cwiki.apache.org/confluence/display/hive/languagemanual+ddl#LanguageManualDDL-(Un)ArchivePartition PARTITION partition_spec;` `ALTER TABLE table_name UNARCHIVE PARTITION partition_spec;` | Not supported. |
| `Table and partition file format` | `https://cwiki.apache.org/confluence/display/hive/languagemanual+ddl#LanguageManualDDL-AlterTable/PartitionFileFormat SET FILEFORMAT file_format;` | Not supported. |
| `Table and partition location` | `https://cwiki.apache.org/confluence/display/hive/languagemanual+ddl#LanguageManualDDL-AlterTable/PartitionLocation SET LOCATION "new location";` | Not supported. |
| `Table and partition touch` | `https://cwiki.apache.org/confluence/display/hive/languagemanual+ddl#LanguageManualDDL-AlterTable/PartitionTouch [PARTITION partition_spec];` | Not supported. |
| `Table and partition protection` | `https://cwiki.apache.org/confluence/display/hive/languagemanual+ddl#LanguageManualDDL-AlterTable/PartitionProtections ENABLE|DISABLE NO_DROP [CASCADE];` ` ` `ALTER TABLE table_name [PARTITION partition_spec] ENABLE|DISABLE OFFLINE;` | Not supported. |
| `Table and partition compact` | `https://cwiki.apache.org/confluence/display/hive/languagemanual+ddl#LanguageManualDDL-AlterTable/PartitionCompact` ` COMPACT 'compaction_type'[AND WAIT]` ` [WITH OVERWRITE TBLPROPERTIES ("property"="value" [, ...])];` | Not supported. |
| `Table and artition concatenate` | `https://cwiki.apache.org/confluence/display/hive/languagemanual+ddl#LanguageManualDDL-AlterTable/PartitionConcatenate CONCATENATE;` | Not supported. |
| `Table and partition columns` | `https://cwiki.apache.org/confluence/display/hive/languagemanual+ddl#LanguageManualDDL-AlterTable/PartitionUpdatecolumns UPDATE COLUMNS;` | Not supported for the `ALTER TABLE` statements. |
| `Column name, type, position, and comment` | `https://cwiki.apache.org/confluence/display/hive/languagemanual+ddl#LanguageManualDDL-AlterColumn CHANGE [COLUMN] col_old_name col_new_name column_type` ` [COMMENT col_comment] [FIRST|AFTER column_name] [CASCADE|RESTRICT];` | Not supported. |

## DDL syntax

This section addresses differences in Data Definition Language (DDL) syntax
between Hive and BigQuery.

### `CREATE TABLE` and `DROP TABLE` statements

The following table provides details about converting `CREATE TABLE` statements
from Hive to BigQuery:

| **Type** | **Hive** | **BigQuery** |
|---|---|---|
| Managed tables | `create table table_name (` ` id int,` ` dtDontQuery string,` ` name string` `)` | ``CREATE TABLE `myproject`.mydataset.table_name ( id INT64, dtDontQuery STRING, name STRING `` ` ) ` |
| Partitioned tables | `create table table_name (` ` id int,` ` dt string,` ` name string` `)` `partitioned by (date string)` | ``CREATE TABLE `myproject`.mydataset.table_name ( id INT64, dt DATE, name STRING ) PARTITION BY dt OPTIONS( partition_expiration_days=3, description="a table partitioned by date_col" `` ` )` |
| `Create table as select (CTAS)` | `CREATE TABLE new_key_value_store` ` ROW FORMAT SERDE "org.apache.hadoop.hive.serde2.columnar.ColumnarSerDe"` ` STORED AS RCFile` ` AS` `SELECT (key % 1024) new_key, concat(key, value) key_value_pair, dt` `FROM key_value_store` `SORT BY new_key, key_value_pair;` | ``CREATE TABLE `myproject`.mydataset.new_key_value_store`` When partitioning by date, uncomment the following: `PARTITION BY dt ` ` OPTIONS( ` ` description="Table Description", ` When partitioning by date, uncomment the following. It's recommended to use `require_partition` when the table is partitioned. `require_partition_filter=TRUE ` ` ) AS ` ` SELECT (key % 1024) new_key, concat(key, value) key_value_pair, dt ` ` FROM key_value_store ` ` SORT BY new_key, key_value_pair' ` |
| `Create Table Like:` The `LIKE` form of `CREATE TABLE` lets you copy an existing table definition exactly. | `CREATE TABLE empty_key_value_store` `LIKE key_value_store [TBLPROPERTIES (property_name=property_value, ...)];` | Not supported. |
| Bucketed sorted tables (clustered in BigQuery terminology) | `CREATE TABLE page_view(` `viewTime INT, ` `userid BIGINT,` `page_url STRING,` `referrer_url STRING,` `ip STRING COMMENT 'IP Address of the User'` `)` `COMMENT 'This is the page view table'` ` PARTITIONED BY(dt STRING, country STRING)` ` CLUSTERED BY(userid) SORTED BY(viewTime) INTO 32 BUCKETS` ` ROW FORMAT DELIMITED` ` FIELDS TERMINATED BY '\001'` ` COLLECTION ITEMS TERMINATED BY '\002'` ` MAP KEYS TERMINATED BY '\003'` ` STORED AS SEQUENCEFILE;` | ``CREATE TABLE `myproject` mydataset.page_view ( viewTime INT, dt DATE, userId BIGINT, page_url STRING, referrer_url STRING, ip STRING OPTIONS (description="IP Address of the User") ) PARTITION BY dt CLUSTER BY userId OPTIONS ( partition_expiration_days=3, description="This is the page view table", require_partition_filter=TRUE `` ` )' ` For more information, see [Create and use clustered tables](https://docs.cloud.google.com/bigquery/docs/creating-clustered-tables). |
| Skewed tables (tables where one or more columns have skewed values) | `CREATE TABLE list_bucket_multiple (col1 STRING, col2 int, col3 STRING)` ` SKEWED BY (col1, col2) ON (('s1',1), ('s3',3), ('s13',13), ('s78',78)) [STORED AS DIRECTORIES];` | Not supported. |
| Temporary tables | `CREATE TEMPORARY TABLE list_bucket_multiple (` `col1 STRING,` `col2 int,` `col3 STRING);` | You can achieve this using expiration time as follows: `CREATE TABLE mydataset.newtable ` ` ( ` ` col1 STRING OPTIONS(description="An optional INTEGER field"), ` ` col2 INT64, ` ` col3 STRING ` ` ) ` ` PARTITION BY DATE(_PARTITIONTIME) ` ` OPTIONS( ` ` expiration_timestamp=TIMESTAMP "2020-01-01 00:00:00 UTC", ` ` partition_expiration_days=1, ` ` description="a table that expires in 2020, with each partition living for 24 hours", ` ` labels=[("org_unit", "development")] ` ` )` |
| Transactional tables | `CREATE TRANSACTIONAL TABLE transactional_table_test(key string, value string) PARTITIONED BY(ds string) STORED AS ORC;` | All table modifications in BigQuery are ACID (atomicity, consistency, isolation, durability) compliant. |
| Drop table | `DROP TABLE [IF EXISTS] table_name [PURGE];` | `{DROP TABLE | DROP TABLE IF EXISTS} ` ` table_name` |
| Truncate table | `TRUNCATE TABLE table_name [PARTITION partition_spec];` `partition_spec:` ` : (partition_column = partition_col_value, partition_column = partition_col_value, ...)` | Not supported. The following workarounds are available: <br /> - Drop and create the table again with the same schema. - Set write disposition for table to `WRITE_TRUNCATE` if the truncate operation is a common use case for the given table. - Use the `CREATE OR REPLACE TABLE` statement. - Use the `DELETE from table_name WHERE 1=1` statement. <br /> Note: Specific partitions can also be truncated. For more information, see [Delete a partition](https://docs.cloud.google.com/bigquery/docs/managing-partitioned-tables#delete_a_partition). |

### `CREATE EXTERNAL TABLE` and `DROP EXTERNAL TABLE` statements

For external table support in BigQuery, see
[Introduction to external data sources](https://docs.cloud.google.com/bigquery/external-data-sources).

### `CREATE VIEW` and `DROP VIEW` statements

The following table provides details about converting `CREATE VIEW` statements
from Hive to BigQuery:

| **Hive** | **BigQuery** |
|---|---|
| `CREATE VIEW [IF NOT EXISTS] [db_name.]view_name [(column_name [COMMENT column_comment], ...) ]` ` [COMMENT view_comment]` ` [TBLPROPERTIES (property_name = property_value, ...)]` ` AS SELECT ...;` | `{CREATE VIEW | CREATE VIEW IF NOT EXISTS | CREATE OR REPLACE VIEW} view_name \[OPTIONS(https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#view_option_list)\] ` ` AS query_expression` |
| `CREATE MATERIALIZED VIEW [IF NOT EXISTS] [db_name.]materialized_view_name` ` [DISABLE REWRITE]` ` [COMMENT materialized_view_comment]` ` [PARTITIONED ON (col_name, ...)]` ` [` ` [ROW FORMAT row_format]` ` [STORED AS file_format]` ` | STORED BY 'storage.handler.class.name' [WITH SERDEPROPERTIES (...)]` ` ]` ` [LOCATION hdfs_path]` ` [TBLPROPERTIES (property_name=property_value, ...)]` `AS` `;` | `CREATE MATERIALIZED VIEW [IF NOT EXISTS] \ [project_id].[dataset_id].materialized_view_name -- cannot disable rewrites in BigQuery \[OPTIONS( \[description="materialized_view_comment",\] \\ \[other https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#materialized_view_option_list\] )\] ` ` [PARTITION BY (col_name)] --same as source table ` |

### `CREATE FUNCTION` and `DROP FUNCTION` statements

The following table provides details about converting stored procedures from
Hive to BigQuery:

| **Hive** | **BigQuery** |
|---|---|
| `CREATE TEMPORARY FUNCTION function_name AS class_name;` | `CREATE { TEMPORARY | TEMP } FUNCTION function_name ([named_parameter[, ...]])` `[RETURNS data_type]` `AS (sql_expression)` `named_parameter:` `param_name param_type` |
| `DROP TEMPORARY FUNCTION [IF EXISTS] function_name;` | Not supported. |
| `CREATE FUNCTION [db_name.]function_name AS class_name` ` [USING JAR|FILE|ARCHIVE 'file_uri' [, JAR|FILE|ARCHIVE 'file_uri'] ];` | Supported for allowlisted projects as an alpha feature. `CREATE { FUNCTION | FUNCTION IF NOT EXISTS | OR REPLACE FUNCTION }` `function_name ([named_parameter[, ...]])` `[RETURNS data_type]` `AS (expression);` `named_parameter:` `param_name param_type` |
| `DROP FUNCTION [IF EXISTS] function_name;` | `DROP FUNCTION [ IF EXISTS ] function_name` |
| `RELOAD FUNCTION;` | Not supported. |

### `CREATE MACRO` and `DROP MACRO` statements

The following table provides details about converting procedural SQL statements
used in creating macro from Hive to
BigQuery with variable declaration and assignment:

| **Hive** | **BigQuery** |
|---|---|
| `CREATE TEMPORARY MACRO macro_name([col_name col_type, ...]) expression;` | Not supported. In some cases, this can be substituted with a UDF. |
| `DROP TEMPORARY MACRO [IF EXISTS] macro_name;` | Not supported. |

## Error codes and messages

[Hive error codes](https://cwiki.apache.org/confluence/display/GEODE/Error+Codes) and [BigQuery error codes](https://docs.cloud.google.com/bigquery/troubleshooting-errors) are different. If your application logic is catching errors, eliminate the source of the error because BigQuery doesn't return the same error codes.

In BigQuery, it's common to use the [INFORMATION_SCHEMA](https://docs.cloud.google.com/bigquery/docs/information-schema-jobs) views or [audit logging](https://docs.cloud.google.com/bigquery/docs/reference/auditlogs) to examine errors.

## Consistency guarantees and transaction isolation

Both Hive and BigQuery support transactions
with ACID semantics. [Transactions](https://cwiki.apache.org/confluence/display/Hive/Hive+Transactions#HiveTransactions-ACIDandTransactionsinHive) are
enabled by default in Hive 3.

### ACID semantics

Hive supports [snapshot isolation](https://cwiki.apache.org/confluence/display/hive/hive+transactions).
When you execute a query, the query is provided with a consistent snapshot of
the database, which it uses until the end of its execution.
Hive provides full ACID semantics at the row level,
letting one application add rows when another application reads from the
same partition without interfering with each other.

BigQuery provides [optimistic concurrency control](https://en.wikipedia.org/wiki/Optimistic_concurrency_control) (first to commit wins) with [snapshot isolation](https://en.wikipedia.org/wiki/Snapshot_isolation),
in which a query reads the last committed data before the query starts. This
approach guarantees the same level of consistency for each row and mutation, and
across rows within the same DML statement, while avoiding deadlocks. For
multiple DML updates to the same table, BigQuery switches to
[pessimistic concurrency control](https://docs.cloud.google.com/bigquery/docs/data-manipulation-language#dml-limitations).
Load jobs can run independently and append tables; however,
BigQuery doesn't provide an explicit transaction boundary or
session.

### Transactions

Hive doesn't support multi-statement transactions. It
doesn't support `BEGIN`, `COMMIT`, and `ROLLBACK` statements. In
Hive, all language operations are auto-committed.

BigQuery supports multi-statement transactions inside a single
query or across multiple queries when you use sessions. A multi-statement
transaction lets you perform mutating operations, such as inserting or deleting
rows from one or more tables and either committing or rolling back the changes.
For more information, see
[Multi-statement transactions](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/transactions).