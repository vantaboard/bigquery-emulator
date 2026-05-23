# Amazon Redshift SQL translation guide

This document details the similarities and differences in SQL syntax between
Amazon Redshift and BigQuery to help you plan your migration. Use
[batch SQL translation](https://docs.cloud.google.com/bigquery/docs/batch-sql-translator) to
migrate your SQL scripts in bulk, or
[interactive SQL translation](https://docs.cloud.google.com/bigquery/docs/interactive-sql-translator)
to translate ad hoc queries.

The intended audience for this guide is enterprise architects, database
administrators, application developers, and IT security specialists. It
assumes you are familiar with Amazon Redshift.

> [!NOTE]
> **Note:** In some cases, there is no direct mapping between a SQL element in Amazon Redshift and BigQuery. However, in most cases, you can achieve the same functionality in BigQuery that you can in Amazon Redshift using alternative means, as shown in the examples in this document.

## Data types

This section shows equivalents between data types in Amazon Redshift and in BigQuery.

| **Amazon Redshift** || **BigQuery** | **Notes** |
|---|---|---|---|
| **Data type** | **Alias** | **Data type** |   |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_Numeric_types201.html#r_Numeric_types201-integer-types` | `https://docs.aws.amazon.com/redshift/latest/dg/r_Numeric_types201.html#r_Numeric_types201-integer-types` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#integer_types` | Amazon Redshift's `SMALLINT` is 2 bytes, whereas BigQuery's `INT64` is 8 bytes. |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_Numeric_types201.html#r_Numeric_types201-integer-types` | `https://docs.aws.amazon.com/redshift/latest/dg/r_Numeric_types201.html#r_Numeric_types201-integer-types` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#integer_types` | Amazon Redshift's `INTEGER` is 4 bytes, whereas BigQuery's `INT64` is 8 bytes. |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_Numeric_types201.html#r_Numeric_types201-integer-types` | `https://docs.aws.amazon.com/redshift/latest/dg/r_Numeric_types201.html#r_Numeric_types201-integer-types` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#integer_types` | Both Amazon Redshift's `BIGINT` and BigQuery's `INT64` are 8 bytes. |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_Numeric_types201.html#r_Numeric_types201-decimal-or-numeric-type` | `https://docs.aws.amazon.com/redshift/latest/dg/r_Numeric_types201.html#r_Numeric_types201-decimal-or-numeric-type` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#numeric_type` |   |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_Numeric_types201.html#r_Numeric_types201-floating-point-types` | `https://docs.aws.amazon.com/redshift/latest/dg/r_Numeric_types201.html#r_Numeric_types201-floating-point-types` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#floating_point_types` | Amazon Redshift's `REAL` is 4 bytes, whereas BigQuery's `FLOAT64` is 8 bytes. |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_Numeric_types201.html#r_Numeric_types201-floating-point-types` | `https://docs.aws.amazon.com/redshift/latest/dg/r_Numeric_types201.html#r_Numeric_types201-floating-point-types` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#floating_point_types` |   |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_Boolean_type.html` | `https://docs.aws.amazon.com/redshift/latest/dg/r_Boolean_type.html` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#boolean_type` | Amazon Redshift's `BOOLEAN` can use `TRUE`, `t`, `true`, `y`, `yes`, and `1` as valid literal values for true. BigQuery's `BOOL` data type uses case-insensitive `TRUE`. |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_Character_types.html#r_Character_types-char-or-character` | `https://docs.aws.amazon.com/redshift/latest/dg/r_Character_types.html#r_Character_types-char-or-character ` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#string_type` |   |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_Character_types.html#r_Character_types-varchar-or-character-varying` | `https://docs.aws.amazon.com/redshift/latest/dg/r_Character_types.html#r_Character_types-varchar-or-character-varying ` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#string_type` |   |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_Datetime_types.html#r_Datetime_types-date` |   | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#date_type` |   |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_Datetime_types.html#r_Datetime_types-timestamp` | `https://docs.aws.amazon.com/redshift/latest/dg/r_Datetime_types.html#r_Datetime_types-timestamp` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#datetime_type` |   |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_Datetime_types.html#r_Datetime_types-timestamptz` | `https://docs.aws.amazon.com/redshift/latest/dg/r_Datetime_types.html#r_Datetime_types-timestamp ` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#timestamp_type` | Note: In BigQuery, [time zones](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#time_zones) are used when parsing timestamps or formatting timestamps for display. A string-formatted timestamp might include a time zone, but when BigQuery parses the string, it stores the timestamp in the equivalent UTC time. When a time zone is not explicitly specified, the default time zone, UTC, is used. [Time zone names](https://en.wikipedia.org/wiki/List_of_time_zone_abbreviations) or [offset from UTC](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#timezone_definitions) using (-\|+)HH:MM are supported, but time zone abbreviations such as PDT are not supported. |
| `https://aws.amazon.com/about-aws/whats-new/2019/11/amazon-redshift-announces-support-spatial-data/` |   | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#geography_type` | Support for querying geospatial data. |

BigQuery also has the following data types that do not have a direct Amazon Redshift
analog:

- [`ARRAY`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#array_type)
- [`BYTES`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#bytes_type)
- [`TIME`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#time_type)
- [`STRUCT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#struct_type)

### Implicit conversion types

When migrating to BigQuery, you need to convert most of your
[Amazon Redshift implicit conversions](https://docs.aws.amazon.com/redshift/latest/dg/c_Supported_data_types.html#r_Type_conversion)
to BigQuery's explicit conversions except for the following data types, which
BigQuery implicitly converts.

BigQuery performs implicit conversions for the following data types:

| **From BigQuery type** | **To BigQuery type** |
|---|---|
| ` INT64 ` | ` FLOAT64 ` |
| ` INT64 ` | ` NUMERIC ` |
| ` NUMERIC ` | ` FLOAT64 ` |

BigQuery also performs implicit conversions for the following literals:

| **From BigQuery type** | **To BigQuery type** |
|---|---|
| `STRING` literal (e.g. "2008-12-25") | ` DATE ` |
| `STRING` literal (e.g. "2008-12-25 15:30:00") | ` TIMESTAMP ` |
| `STRING` literal (e.g. "2008-12-25T07:30:00") | ` DATETIME ` |
| `STRING` literal (e.g. "15:30:00") | ` TIME ` |

### Explicit conversion types

You can convert Amazon Redshift data types that BigQuery doesn't implicitly convert
using BigQuery's
[`CAST(expression AS type)`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/conversion_functions#cast) function
or any of the
[`DATE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions)
and
[`TIMESTAMP`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions)
conversion functions.

When migrating your queries, change any occurrences of the Amazon Redshift
[`CONVERT(type, expression)`](https://docs.aws.amazon.com/redshift/latest/dg/r_CAST_function.html#convert-function)
function (or the :: syntax) to BigQuery's
[`CAST(expression AS type)`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/conversion_functions#cast) function,
as shown in the table in the [Data type formatting functions](https://docs.cloud.google.com/bigquery/docs/migration/redshift-sql#data_type_formatting_functions) section.

## Query syntax

This section addresses differences in query syntax between Amazon Redshift and
BigQuery.

### `SELECT` statement

Most Amazon Redshift
[`SELECT`](https://docs.aws.amazon.com/redshift/latest/dg/r_SELECT_synopsis.html)
statements are compatible with BigQuery. The following table contains a list of
minor differences.

| **Amazon Redshift** | **BigQuery** |
|---|---|
| ` SELECT TOP number expression FROM table ` | ` SELECT expression FROM table ORDER BY expression DESC LIMIT number` |
| ` SELECT x/total AS probability, ROUND(100 * probability, 1) AS pct FROM raw_data ` <br /> Note: Redshift supports creating and referencing an alias in the same `SELECT` statement. | ` SELECT x/total AS probability, ROUND(100 * (x/total), 1) AS pct FROM raw_data ` |

BigQuery also supports the following expressions in `SELECT` statements, which do
not have a Amazon Redshift equivalent:

- [`EXCEPT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#select_except)
- [`REPLACE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#select_replace)

### `FROM` clause

A
[`FROM`](https://docs.aws.amazon.com/redshift/latest/dg/r_FROM_clause30.html)
clause in a query lists the table references that data is selected from. In
Amazon Redshift, possible table references include tables, views, and subqueries. All
of these table references are supported in BigQuery.

BigQuery tables can be referenced in the `FROM`
clause using the following:

- `[project_id].[dataset_id].[table_name]`
- `[dataset_id].[table_name]`
- `[table_name]`

BigQuery also supports additional table references:

- Historical versions of the table definition and rows using [`FOR SYSTEM_TIME AS OF`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#for_system_time_as_of).
- [Field paths](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#field_path), or any path that resolves to a field within a data type (such as a `STRUCT`).
- [Flattened arrays](https://docs.cloud.google.com/bigquery/docs/arrays#querying_nested_arrays).

### `JOIN` types

Both Amazon Redshift and BigQuery support the following types of join:

- `[INNER] JOIN`
- `LEFT [OUTER] JOIN`
- `RIGHT [OUTER] JOIN`
- `FULL [OUTER] JOIN`
- `CROSS JOIN` and the equivalent [implicit comma cross join](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#cross_join).

The following table contains a list of minor differences.

| **Amazon Redshift** | **BigQuery** |
|---|---|
| ` SELECT col FROM table1 NATURAL INNER JOIN table2 ` | ` SELECT col1 FROM table1 INNER JOIN table2 USING (col1, col2 [, ...]) ` <br /> Note: In BigQuery, `JOIN` clauses require a `JOIN` condition unless the clause is a `CROSS JOIN` or one of the joined tables is a field within a data type or an array. |

### `WITH` clause

A BigQuery
[`WITH`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#with_clause)
clause contains one or more named subqueries that execute when a subsequent
`SELECT` statement references them. Amazon Redshift
[`WITH`](https://docs.aws.amazon.com/redshift/latest/dg/r_WITH_clause.html)
clauses behave the same as BigQuery's with the exception that you can evaluate
the clause once and reuse its results.

### Set operators

There are some minor differences between
[Amazon Redshift set operators](https://docs.aws.amazon.com/redshift/latest/dg/r_UNION.html#r_UNION-order-of-evaluation-for-set-operators)
and
[BigQuery set](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#set_operators)
[operators](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#set_operators).
However, all set operations that are feasible in Amazon Redshift are replicable in
BigQuery.

| **Amazon Redshift** | **BigQuery** |
|---|---|
| ` SELECT * FROM table1 UNION SELECT * FROM table2 ` | ` SELECT * FROM table1 UNION DISTINCT SELECT * FROM table2 ` Note: Both BigQuery and Amazon Redshift support the `UNION ALL` operator. |
| ` SELECT * FROM table1 INTERSECT SELECT * FROM table2 ` | ` SELECT * FROM table1 INTERSECT DISTINCT SELECT * FROM table2 ` |
| ` SELECT * FROM table1 EXCEPT SELECT * FROM table2 ` | ` SELECT * FROM table1 EXCEPT DISTINCT SELECT * FROM table2 ` |
| ` SELECT * FROM table1 MINUS SELECT * FROM table2 ` | ` SELECT * FROM table1 EXCEPT DISTINCT SELECT * FROM table2 ` |
| ` SELECT * FROM table1 UNION SELECT * FROM table2 EXCEPT SELECT * FROM table3 ` | ` SELECT * FROM table1 UNION ALL ( SELECT * FROM table2 EXCEPT SELECT * FROM table3 ) ` <br /> Note: BigQuery requires parentheses to separate different set operations. If the same set operator is repeated, parentheses are not necessary. |

### `ORDER BY` clause

There are some minor differences between Amazon Redshift
[`ORDER BY`](https://docs.amazonaws.cn/en_us/redshift/latest/dg/r_ORDER_BY_clause.html)
clauses and BigQuery
[`ORDER BY`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#order_by_clause)
clauses.

| Amazon Redshift | BigQuery |
|---|---|
| In Amazon Redshift, `NULL`s are ranked last by default (ascending order). | In BigQuery, `NULL`s are ranked first by default (ascending order). |
| ` SELECT * FROM table ORDER BY expression LIMIT ALL ` | ` SELECT * FROM table ORDER BY expression` <br /> Note: BigQuery does not use the `LIMIT ALL` syntax, but `ORDER BY` sorts all rows by default, resulting in the same behavior as Amazon Redshift's `LIMIT ALL` clause. We highly recommend including a `LIMIT` clause with every `ORDER BY` clause. Ordering all result rows unnecessarily degrades query execution performance. |
| ` SELECT * FROM table ORDER BY expression OFFSET 10 ` | ` SELECT * FROM table ORDER BY expression LIMIT *count* OFFSET 10 ` <br /> <br /> Note: In BigQuery, `OFFSET` must be used together with a `LIMIT` *count*. Make sure to set the *count* `INT64` value to the minimum necessary ordered rows. Ordering all result rows unnecessarily degrades query execution performance. |

### Conditions

The following table
shows [Amazon Redshift conditions](https://docs.aws.amazon.com/redshift/latest/dg/r_conditions.html),
or predicates, that are specific to Amazon Redshift and must be converted to their
BigQuery equivalent.

| **Amazon Redshift** | **BigQuery** |
|---|---|
| ` a https://docs.aws.amazon.com/redshift/latest/dg/r_comparison_condition.html (subquery) ` ` a https://docs.aws.amazon.com/redshift/latest/dg/r_comparison_condition.html (subquery) ` | ` a https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/operators#in_operators subquery ` |
| ` a https://docs.aws.amazon.com/redshift/latest/dg/r_comparison_condition.html (subquery) ` ` a != ALL (subquery) ` | ` a https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/operators#in_operators subquery ` |
| ` a https://docs.aws.amazon.com/redshift/latest/dg/r_comparison_condition.html ` ` expression https://docs.aws.amazon.com/redshift/latest/dg/r_patternmatching_condition_like.html pattern ` | ` a https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/operators#is_operators ` ` https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#lower(expression) https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/operators#comparison_operators LOWER(pattern) ` |
| ` expression https://docs.aws.amazon.com/redshift/latest/dg/r_patternmatching_condition_like.html pattern ESCAPE 'escape_char' ` | ` expression https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/operators#comparison_operators pattern ` <br /> Note: BigQuery does not support custom escape characters. You must use two backslashes \\\\ as escape characters for BigQuery. |
| ` expression [NOT] https://docs.aws.amazon.com/redshift/latest/dg/pattern-matching-conditions-similar-to.html pattern ` | ` IF( LENGTH( REGEXP_REPLACE( expression, pattern, '' ) = 0, True, False ) ` <br /> Note: If `NOT` is specified, wrap the above `IF` expression in a `NOT` expression as shown below: <br /> ` NOT( IF( LENGTH(... ) ` |
| ` expression https://docs.aws.amazon.com/redshift/latest/dg/pattern-matching-conditions-posix.html pattern ` | ` https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/operators#logical_operators https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#regexp_contains( expression, regex ) ` |

## Functions

The following sections list Amazon Redshift functions and their BigQuery equivalents.

### Aggregate functions

The following table shows mappings between common Amazon Redshift aggregate, aggregate
analytic, and approximate aggregate functions with their BigQuery equivalents.

| Amazon Redshift | BigQuery |
|---|---|
| `https://docs.aws.amazon.com/redshift/latest/dg/r_COUNT.html(DISTINCT expression)` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/approximate_aggregate_functions#approx_count_distinct(expression)` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_APPROXIMATE_PERCENTILE_DISC.html( percentile ) WITHIN GROUP (ORDER BY expression)` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/approximate_aggregate_functions#approx_quantiles(expression, 100) [OFFSET(CAST(TRUNC(percentile * 100) as INT64))]` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_AVG.html([DISTINCT] expression)` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#avg([DISTINCT] expression)` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_COUNT.html(expression)` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#count(expression)` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_LISTAGG.html( [DISTINCT] aggregate_expression [, delimiter] ) [WITHIN GROUP (ORDER BY order_list)] ` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#string_agg( [DISTINCT] aggregate_expression [, delimiter] [ORDER BY order_list] )` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_MAX.html(expression)` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#max(expression)` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_MEDIAN.html(median_expression)` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/navigation_functions#percentile_cont( median_expression, 0.5 ) OVER() ` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_MIN.html(expression)` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#min(expression)` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_PERCENTILE_CONT.html( percentile ) WITHIN GROUP (ORDER BY expression)` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/navigation_functions#percentile_cont( median_expression, percentile ) OVER()` <br /> Note: Does not cover aggregation use cases. |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_STDDEV_functions.html([DISTINCT] expression)` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#stddev([DISTINCT] expression)` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_STDDEV_functions.html([DISTINCT] expression)` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#stddev_samp([DISTINCT] expression)` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_STDDEV_functions.html([DISTINCT] expression)` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#stddev_pop([DISTINCT] expression)` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_SUM.html([DISTINCT] expression)` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#sum([DISTINCT] expression)` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_VARIANCE_functions.html([DISTINCT] expression)` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#variance([DISTINCT] expression)` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_VARIANCE_functions.html([DISTINCT] expression)` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#var_samp([DISTINCT] expression)` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_VARIANCE_functions.html([DISTINCT] expression)` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#var_pop([DISTINCT] expression)` |

BigQuery also offers the following
[aggregate](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions),
[aggregate analytic](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_analytic_functions),
and
[approximate aggregate](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/approximate_aggregate_functions)
functions, which do not have a direct analogue in Amazon Redshift:

- [`ANY_VALUE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#any_value)
- [`APPROX_TOP_COUNT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/approximate_aggregate_functions#approx_top_count)
- [`APPROX_TOP_SUM`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/approximate_aggregate_functions#approx_top_sum)
- [`ARRAY_AGG`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#array_agg)
- [`ARRAY_CONCAT_AGG`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#array_concat_agg)
- [`COUNTIF`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#countif)
- [`CORR`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#corr)
- [`COVAR_POP`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#covar_pop)
- [`COVAR_SAMP`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#covar_samp)

### Bitwise aggregate functions

The following table shows mappings between common Amazon Redshift bitwise aggregate
functions with their BigQuery equivalents.

| **Amazon Redshift** | **BigQuery** |
|---|---|
| `https://docs.aws.amazon.com/redshift/latest/dg/r_BIT_AND.html(expression)` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#bit_and(expression)` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_BIT_OR.html(expression)` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#bit_or(expression)` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_BOOL_AND.html>(expression)` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#logical_and(expression)` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_BOOL_OR.html(expression)` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#logical_or(expression)` |

BigQuery also offers the following
[bit-wise aggregate](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bit_functions)
function, which does not have a direct analogue in Amazon Redshift:

- [`BIT_XOR`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#bit_xor)

### Window functions

The following table shows mappings between common Amazon Redshift window functions
with their BigQuery equivalents. Windowing functions in BigQuery include
[analytic aggregate functions](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_analytic_functions),
[aggregate functions](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions),
[navigation functions](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/navigation_functions),
and
[numbering functions](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/numbering_functions).

| **Amazon Redshift** | **BigQuery** |
|---|---|
| `https://docs.aws.amazon.com/redshift/latest/dg/r_WF_AVG.html(expression) OVER ( [PARTITION BY expr_list] [ORDER BY order_list frame_clause] )` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#avg(expression) OVER ( [PARTITION BY expr_list] [ORDER BY order_list] [frame_clause] ) ` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_WF_COUNT.html(expression) OVER ( [PARTITION BY expr_list] [ORDER BY order_list frame_clause] )` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#count(expression) OVER ( [PARTITION BY expr_list] [ORDER BY order_list] [frame_clause] ) ` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_WF_CUME_DIST.html() OVER ( [PARTITION BY partition_expression] [ORDER BY order_list] ) ` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/numbering_functions#cume_dist() OVER ( [PARTITION BY partition_expression] ORDER BY order_list ) ` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_WF_DENSE_RANK.html() OVER ( [PARTITION BY expr_list] [ORDER BY order_list] ) ` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/numbering_functions#dense_rank() OVER ( [PARTITION BY expr_list] ORDER BY order_list )` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_WF_first_value.html(expression) OVER ( [PARTITION BY expr_list] [ORDER BY order_list frame_clause] )` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/navigation_functions#first_value(expression) OVER ( [PARTITION BY expr_list] [ORDER BY order_list] [frame_clause] ) ` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_WF_first_value.html(expression) OVER ( [PARTITION BY expr_list] [ORDER BY order_list frame_clause] )` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/navigation_functions#last_value(expression) OVER ( [PARTITION BY expr_list] [ORDER BY order_list frame_clause] ) ` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_WF_LAG.html(value_expr [, offset]) OVER ( [PARTITION BY window_partition] ORDER BY window_ordering ) ` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/navigation_functions#lag(value_expr [, offset]) OVER ( [PARTITION BY window_partition] ORDER BY window_ordering ) ` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_WF_LEAD.html(value_expr [, offset]) OVER ( [PARTITION BY window_partition] ORDER BY window_ordering ) ` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/navigation_functions#lead(value_expr [, offset]) OVER ( [PARTITION BY window_partition] ORDER BY window_ordering ) ` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_LISTAGG.html( [DISTINCT] expression [, delimiter] ) [WITHIN GROUP (ORDER BY order_list)] OVER ( [PARTITION BY partition_expression] )` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#string_agg( [DISTINCT] aggregate_expression [, delimiter] ) OVER ( [PARTITION BY partition_list] [ORDER BY order_list] )` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_WF_MAX.html(expression) OVER ( [PARTITION BY expr_list] [ORDER BY order_list frame_clause] )` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#max(expression) OVER ( [PARTITION BY expr_list] [ORDER BY order_list] [frame_clause] ) ` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_WF_MEDIAN.html(median_expression) OVER ( [PARTITION BY partition_expression] ) ` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/navigation_functions#percentile_cont( median_expression, 0.5 ) OVER ( [PARTITION BY partition_expression] ) ` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_WF_MIN.html(expression) OVER ( [PARTITION BY expr_list] [ORDER BY order_list frame_clause] )` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#min(expression) OVER ( [PARTITION BY expr_list] [ORDER BY order_list] [frame_clause] ) ` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_WF_NTH.html(expression, offset) OVER ( [PARTITION BY window_partition] [ORDER BY window_ordering frame_clause] ) ` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/navigation_functions#nth_value(expression, offset) OVER ( [PARTITION BY window_partition] ORDER BY window_ordering [frame_clause] ) ` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_WF_NTILE.html(expr) OVER ( [PARTITION BY expression_list] [ORDER BY order_list] ) ` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/numbering_functions#ntile(expr) OVER ( [PARTITION BY expression_list] ORDER BY order_list ) ` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_WF_PERCENT_RANK.html() OVER ( [PARTITION BY partition_expression] [ORDER BY order_list] ) ` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/numbering_functions#percent_rank() OVER ( [PARTITION BY partition_expression] ORDER BY order_list ) ` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_WF_PERCENTILE_CONT.html(percentile) WITHIN GROUP (ORDER BY expr) OVER ( [PARTITION BY expr_list] ) ` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/navigation_functions#percentile_cont(expr, percentile) OVER ( [PARTITION BY expr_list] ) ` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_WF_PERCENTILE_DISC.html(percentile) WITHIN GROUP (ORDER BY expr) OVER ( [PARTITION BY expr_list] ) ` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/navigation_functions#percentile_disc(expr, percentile) OVER ( [PARTITION BY expr_list] )` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_WF_RANK.html() OVER ( [PARTITION BY expr_list] [ORDER BY order_list] )` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/numbering_functions#rank() OVER ( [PARTITION BY expr_list] ORDER BY order_list ) ` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_WF_RATIO_TO_REPORT.html(ratio_expression) OVER ( [PARTITION BY partition_expression] ) ` | `ratio_expression https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#sum(ratio_expression) OVER ( [PARTITION BY partition_expression] )` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_WF_ROW_NUMBER.html() OVER ( [PARTITION BY expr_list] [ORDER BY order_list] ) ` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/numbering_functions#row_number() OVER ( [PARTITION BY expr_list] [ORDER BY order_list] ) ` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_WF_STDDEV.html(expression) OVER ( [PARTITION BY expr_list] [ORDER BY order_list frame_clause] )` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#stddev(expression) OVER ( [PARTITION BY expr_list] [ORDER BY order_list] [frame_clause] ) ` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_WF_STDDEV.html(expression) OVER ( [PARTITION BY expr_list] [ORDER BY order_list frame_clause] )` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#stddev_samp(expression) OVER ( [PARTITION BY expr_list] [ORDER BY order_list] [frame_clause] )` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_WF_STDDEV.html(expression) OVER ( [PARTITION BY expr_list] [ORDER BY order_list] [frame_clause] ) ` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#stddev_pop(expression) OVER ( [PARTITION BY expr_list] [ORDER BY order_list] [frame_clause] )` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_WF_SUM.html(expression) OVER ( [PARTITION BY expr_list] [ORDER BY order_list] [frame_clause] ) ` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#sum(expression) OVER ( [PARTITION BY expr_list] [ORDER BY order_list] [frame_clause] )` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_WF_VARIANCE.html(expression) OVER ( [PARTITION BY expr_list] [ORDER BY order_list] [frame_clause] )` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#var_pop(expression) OVER ( [PARTITION BY expr_list] [ORDER BY order_list] [frame_clause] )` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_WF_VARIANCE.html(expression) OVER ( [PARTITION BY expr_list] [ORDER BY order_list] [frame_clause] ) ` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#var_samp(expression) OVER ( [PARTITION BY expr_list] [ORDER BY order_list] [frame_clause] ) ` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_WF_VARIANCE.html(expression) OVER ( [PARTITION BY expr_list] [ORDER BY order_list] [frame_clause] )` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#variance(expression) OVER ( [PARTITION BY expr_list] [ORDER BY order_list] [frame_clause] ) ` |

### Conditional expressions

The following table shows mappings between common Amazon Redshift conditional
expressions with their BigQuery equivalents.

| **Amazon Redshift** | **BigQuery** |
|---|---|
| `https://docs.aws.amazon.com/redshift/latest/dg/r_CASE_function.htmlexpression WHEN value THEN result [WHEN...] [ELSE else_result] END` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/conditional_expressions#case_expr expression WHEN value THEN result [WHEN...] [ELSE else_result] END ` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_COALESCE.html(expression1[, ...])` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/conditional_expressions#coalesce(expression1[, ...]) ` |
| `https://docs.aws.amazon.com/en_pv/redshift/latest/dg/r_DECODE_expression.html( expression, search1, result1 [, search2, result2...] [, default] ) ` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/conditional_expressions#case expression WHEN value1 THEN result1 [WHEN value2 THEN result2] [ELSE default] END ` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_GREATEST_LEAST.html(value [, ...])` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#greatest(value [, ...])` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_GREATEST_LEAST.html(value [, ...]) ` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#least(value [, ...]) ` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_NVL_function.html(expression1[, ...]) ` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/conditional_expressions#coalesce(expression1[, ...]) ` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_NVL2.html( expression, not_null_return_value, null_return_value )` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/conditional_expressions#if( expression IS NULL, null_return_value, not_null_return_value )` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_NULLIF_function.html(expression1, expression2)` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/conditional_expressions#nullif(expression1, expression2)` |

BigQuery also offers the following conditional expressions, which do not have a
direct analogue in Amazon Redshift:

- [`IF`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/conditional_expressions#if)
- [`IFNULL`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/conditional_expressions#ifnull)

### Date and time functions

The following table shows mappings between common Amazon Redshift date and time
functions with their BigQuery equivalents. BigQuery data and time functions
include
[date functions](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions),
[datetime](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/datetime_functions)
[functions](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/datetime_functions),
[time functions](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/time_functions),
and
[timestamp functions](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions).

Keep in mind that functions that seem identical in Amazon Redshift and BigQuery might
return different data types.

| Amazon Redshift | BigQuery |
|---|---|
| `https://docs.aws.amazon.com/redshift/latest/dg/r_ADD_MONTHS.html( date, integer )` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/conversion_functions#cast( https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#date_add( date, INTERVAL integer MONTH ) AS TIMESTAMP ) ` |
| timestamptz_or_timestamp `https://docs.aws.amazon.com/redshift/latest/dg/r_AT_TIME_ZONE.html timezone ` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#parse_timestamp( "%c%z", https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#format_timestamp( "%c%z", timestamptz_or_timestamp, timezone ) ) ` <br /> Note: [Time zones](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#time_zones) are used when parsing timestamps or formatting timestamps for display. A string-formatted timestamp might include a time zone, but when BigQuery parses the string, it stores the timestamp in the equivalent UTC time. When a time zone is not explicitly specified, the default time zone, UTC, is used. [Time zone names](http://www.iana.org/time-zones) or [offset from UTC](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#timezone_definitions) (-HH:MM) are supported, but time zone abbreviations (such as PDT) are not supported. |
| `https://docs.aws.amazon.com/redshift/latest/dg/CONVERT_TIMEZONE.html( [source_timezone], target_timezone, timestamp )` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#parse_timestamp( "%c%z", https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#format_timestamp( "%c%z", timestamp, target_timezone ) ) ` <br /> Note: `source_timezone` is UTC in BigQuery. |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_CURRENT_DATE_function.html ` <br /> Note: Returns start date for the current transaction in the current session time zone (UTC by default). | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#current_date()` <br /> Note: Returns start date for the current statement in the UTC time zone. |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_DATE_CMP.html(date1, date2) ` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/conditional_expressions#case WHEN date1 = date2 THEN 0 WHEN date1 > date2 THEN 1 ELSE -1 END ` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_DATE_CMP_TIMESTAMP.html(date1, date2)` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/conditional_expressions#case WHEN date1 = CAST(date2 AS DATE) THEN 0 WHEN date1 > CAST(date2 AS DATE) THEN 1 ELSE -1 END ` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_DATE_CMP_TIMESTAMPTZ.html(date, timestamptz) ` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/conditional_expressions#case WHEN date > https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#date(timestamptz) THEN 1 WHEN date < https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#date(timestamptz) THEN -1 ELSE 0 END` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_DATE_PART_YEAR.html(date)` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#extract(YEAR FROM date)` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_DATEADD_function.html(date_part, interval, date)` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/conversion_functions#cast( https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#date_add( date, INTERVAL interval datepart ) AS TIMESTAMP )` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_DATEDIFF_function.html( date_part, date_expression1, date_expression2 ) ` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#date_diff( date_expression2, date_expression1, date_part ) ` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_DATE_PART_function.html(date_part, date) ` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#extract(date_part FROM date)` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_DATE_TRUNC.html('date_part', timestamp) ` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#timestamp_trunc(timestamp, date_part) ` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_EXTRACT_function.html(date_part FROM timestamp) ` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#extract(date_part FROM timestamp)` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_GETDATE.html()` | `PARSE_TIMESTAMP( "%c", FORMAT_TIMESTAMP( "%c", https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#current_timestamp() ) )` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_INTERVAL_CMP.html( interval_literal1, interval_literal2 ) ` | For intervals in Redshift, there are 360 days in a year. In BigQuery, you can use the following user-defined function (UDF) to parse a Redshift interval and translate it to seconds. <br /> `https://docs.cloud.google.com/bigquery/docs/user-defined-functions parse_interval(interval_literal STRING) AS ( (select sum(case when unit in ('minutes', 'minute', 'm' ) then num * 60 when unit in ('hours', 'hour', 'h') then num * 60 * 60 when unit in ('days', 'day', 'd' ) then num * 60 * 60 * 24 when unit in ('weeks', 'week', 'w') then num * 60 * 60 * 24 * 7 when unit in ('months', 'month' ) then num * 60 * 60 * 24 * 30 when unit in ('years', 'year') then num * 60 * 60 * 24 * 360 else num end) from ( select cast(regexp_extract(value, r'^[0-9]*\.?[0-9]+') as numeric) num, substr(value, length(regexp_extract(value, r'^[0-9]*\.?[0-9]+')) + 1) unit from https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#unnest_operator( https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#split( replace( interval_literal, ' ', ''), ',')) value ))); ` <br /> To compare interval literals, perform: `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/conditional_expressions( parse_interval(interval_literal1) > parse_interval(interval_literal2), 1, https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/conditional_expressions( parse_interval(interval_literal1) > parse_interval(interval_literal2), -1, 0 ) ) ` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_LAST_DAY.html(date)` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#date_sub( https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#date_add( date, INTERVAL 1 MONTH ), INTERVAL 1 DAY )` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_MONTHS_BETWEEN_function.html( date1, date2 ) ` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#date_diff( date1, date2, MONTH ) ` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_NEXT_DAY.html(date, day)` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#date_add( https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#date_trunc( date, WEEK(day) ), INTERVAL 1 WEEK ) ` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_SYSDATE.html ` <br /> Note: Returns start timestamp for the current transaction in the current session time zone (UTC by default). | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#current_timestamp() ` <br /> Note: Returns start timestamp for the current statement in the UTC time zone. |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_TIMEOFDAY_function.html()` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#format_timestamp( "%a %b %d %H:%M:%E6S %E4Y %Z", https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#current_timestamp())` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_TIMESTAMP_CMP.html( timestamp1, timestamp2 )` | `CASE WHEN timestamp1 = timestamp2 THEN 0 WHEN timestamp1 > timestamp2 THEN 1 ELSE -1 END ` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_TIMESTAMP_CMP_DATE.html( timestamp, date ) ` | `CASE WHEN https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#extract( DATE FROM timestamp ) = date THEN 0 WHEN https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#extract( DATE FROM timestamp) > date THEN 1 ELSE -1 END ` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_TIMESTAMP_CMP_TIMESTAMPTZ.html( timestamp, timestamptz ) ` <br /> Note: Redshift compares timestamps in the user session-defined time zone. Default user session time zone is UTC. | `CASE WHEN timestamp = timestamptz THEN 0 WHEN timestamp > timestamptz THEN 1 ELSE -1 END` <br /> Note: BigQuery compares timestamps in the UTC time zone. |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_TIMESTAMPTZ_CMP.html( timestamptz1, timestamptz2 ) ` <br /> Note: Redshift compares timestamps in the user session-defined time zone. Default user session time zone is UTC. | `CASE WHEN timestamptz1 = timestamptz2 THEN 0 WHEN timestamptz1 > timestamptz2 THEN 1 ELSE -1 END ` <br /> Note: BigQuery compares timestamps in the UTC time zone. |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_TIMESTAMPTZ_CMP_DATE.html( timestamptz, date ) ` <br /> Note: Redshift compares timestamps in the user session-defined time zone. Default user session time zone is UTC. | `CASE WHEN https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#extract( DATE FROM timestamptz) = date THEN 0 WHEN https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#extract( DATE FROM timestamptz) > date THEN 1 ELSE -1 END ` <br /> Note: BigQuery compares timestamps in the UTC time zone. |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_TIMESTAMPTZ_CMP_TIMESTAMP.html( timestamptz, Timestamp ) ` <br /> Note: Redshift compares timestamps in the user session-defined time zone. Default user session time zone is UTC. | `CASE WHEN timestamp = timestamptz THEN 0 WHEN timestamp > timestamptz THEN 1 ELSE -1 END ` <br /> Note: BigQuery compares timestamps in the UTC time zone. |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_TIMEZONE.html( timezone, Timestamptz_or_timestamp ) ` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#parse_timestamp( "%c%z", https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#format_timestamp( "%c%z", timestamptz_or_timestamp, timezone ) ) ` <br /> Note: [Time zones](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#time_zones) are used when parsing timestamps or formatting timestamps for display. A string-formatted timestamp might include a time zone, but when BigQuery parses the string, it stores the timestamp in the equivalent UTC time. When a time zone is not explicitly specified, the default time zone, UTC, is used. [Time zone names](http://www.iana.org/time-zones) or [offset from UTC](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#timezone_definitions) (-HH:MM) are supported but time zone abbreviations (such as PDT) are not supported. |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_TO_TIMESTAMP.html(timestamp, format) ` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#parse_timestamp( https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/format-elements#format_elements_date_time, https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#format_timestamp( https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/format-elements#format_elements_date_time, timestamp ) ) ` <br /> Note: BigQuery follows a different set of [format elements](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/format-elements#format_elements_date_time). [Time zones](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#time_zones) are used when parsing timestamps or formatting timestamps for display. A string-formatted timestamp might include a time zone, but when BigQuery parses the string, it stores the timestamp in the equivalent UTC time. When a time zone is not explicitly specified, the default time zone, UTC, is used. [Time zone names](http://www.iana.org/time-zones) or [offset from UTC](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#timezone_definitions) (-HH:MM) are supported in the format string but time zone abbreviations (such as PDT) are not supported. |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_TRUNC_date.html(timestamp) ` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/conversion_functions#cast(timestamp AS DATE)` |

BigQuery also offers the following date and time functions, which do not have a
direct analogue in Amazon Redshift:

- [`EXTRACT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#extract)
- [`DATE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#date)
- [`DATE_SUB`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#date_sub)
- [`DATE_ADD`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#date_add)(returning `DATE` data type)
- [`DATE_FROM_UNIX_DATE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#date_from_unix_date)
- [`FORMAT_DATE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#format_date)
- [`PARSE_DATE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#parse_date)
- [`UNIX_DATE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#unix_date)
- [`DATETIME`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/datetime_functions#datetime)
- [`DATETIME_ADD`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/datetime_functions#datetime_add)
- [`DATETIME_SUB`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/datetime_functions#datetime_sub)
- [`DATETIME_DIFF`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/datetime_functions#datetime_diff)
- [`DATETIME_TRUNC`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/datetime_functions#datetime_trunc)
- [`FORMAT_DATETIME`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/datetime_functions#format_datetime)
- [`PARSE_DATETIME`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/datetime_functions#parse_datetime)
- [`CURRENT_TIME`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/time_functions#current_time)
- [`TIME`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/time_functions#time)
- [`TIME_ADD`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/time_functions#time_add)
- [`TIME_SUB`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/time_functions#time_sub)
- [`TIME_DIFF`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/time_functions#time_diff)
- [`TIME_TRUNC`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/time_functions#time_trunc)
- [`FORMAT_TIME`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/time_functions#format_time)
- [`PARSE_TIME`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/time_functions#parse_time)
- [`TIMESTAMP_SECONDS`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#timestamp_seconds)
- [`TIMESTAMP_MILLIS`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#timestamp_millis)
- [`TIMESTAMP_MICROS`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#timestamp_micros)
- [`UNIX_SECONDS`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#unix_seconds)
- [`UNIX_MILLIS`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#unix_millis)
- [`UNIX_MICROS`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#unix_micros)

### Mathematical operators

The following table shows mappings between common Amazon Redshift mathematical
operators with their BigQuery equivalents.

| **Amazon Redshift** | **BigQuery** |
|---|---|
| ` X + Y ` | ` X + Y ` |
| ` X - Y ` | ` X - Y ` |
| ` X * Y ` | ` X * Y ` |
| ` X / Y ` <br /> Note: If the operator is performing integer division (in other words, if `X` and `Y` are both integers), an integer is returned. If the operator is performing non-integer division, a non-integer is returned. | If integer division: `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/conversion_functions#cast(https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#floor(X / Y) AS INT64)` <br /> If not integer division: ` https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/conversion_functions#cast(X / Y AS INT64) ` <br /> Note: Division in BigQuery returns a non-integer. To prevent errors from a division operation (division by zero error), use` https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#safe_divide(X, Y) `or `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#ieee_divide(X, Y)`. |
| ` X % Y ` | ` https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#mod(X, Y) ` <br /> Note: To prevent errors from a division operation (division by zero error), use `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/functions-reference#safe_prefix.https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#mod(X, Y)`. `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/functions-reference#safe_prefix.https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#mod(X, 0)` results in 0. |
| ` X ^ Y ` | ` https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#pow(X, Y) ` ` https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#power(X, Y) ` <br /> Note: Unlike Amazon Redshift, the `^` operator in BigQuery performs Bitwise xor. |
| ` | / X ` | ` https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#sqrt(X) ` <br /> Note: To prevent errors from a square root operation (negative input), use `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/functions-reference#safe_prefix.https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#sqrt(X)`. Negative input with `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/functions-reference#safe_prefix.https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#sqrt(X)` results in `NULL`. |
| ` || / X ` | ` https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#sign(X) * https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#power(https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#abs(X), 1/3) ` <br /> Note: BigQuery's `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#power(X, Y)` returns an error if `X` is a finite value less than 0 and `Y` is a noninteger. |
| ` @ X ` | ` https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#abs(X) ` |
| ` X << Y ` | ` X https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/operators Y ` <br /> Note: This operator returns 0 or a byte sequence of b'\\x00' if the second operand `Y` is greater than or equal to the bit length of the first operand `X` (for example, 64 if `X` has the type INT64). This operator throws an error if `Y` is negative. |
| ` X >> Y ` | ` X https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/operators Y ` <br /> Note: Shifts the first operand `X` to the right. This operator does not do sign bit extension with a signed type (it fills vacant bits on the left with 0). This operator returns 0 or a byte sequence of b'\\x00' if the second operand `Y` is greater than or equal to the bit length of the first operand `X` (for example, 64 if `X` has the type INT64). This operator throws an error if `Y` is negative. |
| ` X & Y ` | ` X https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/operators Y ` |
| ` X | Y ` | ` X https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/operators Y ` |
| ` ~X ` | ` https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/operatorsX ` |

BigQuery also offers the following mathematical operator, which does not have a
direct analog in Amazon Redshift:

- [`X ^ Y`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/operators#bitwise_operators) (Bitwise xor)

### Math functions

| **Amazon Redshift** | **BigQuery** |
|---|---|
| `https://docs.aws.amazon.com/redshift/latest/dg/r_ABS.html(number) ` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#abs(number) ` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_ACOS.html(number)` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#acos(number)` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_ASIN.html(number)` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#asin(number)` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_ATAN.html(number)` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#atan(number)` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_ATAN2.html(number1, number2)` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#atan2(number1, number2)` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_CBRT.html(number)` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#power(number, 1/3)` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_CEILING_FLOOR.html(number)` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#ceil(number)` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_CEILING_FLOOR.html(number) ` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#ceiling(number) ` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_CHECKSUM.html(expression) ` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/hash_functions#farm_fingerprint(expression) ` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_COS.html(number)` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#cos(number)` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_COT.html(number)` | `1/https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#tan(number)` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_DEGREES.html(number)` | `number`\*180/`https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#acos(-1)` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_DEXP.html(number)` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#exp(number) ` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_DLOG1.html(number) ` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#ln(number)` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_DLOG10.html(number)` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#log10(number) ` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_EXP.html(number)` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#exp(number)` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_FLOOR.html(number)` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#floor(number)` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_LN.htmlnumber)` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#ln(number)` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_LOG.html(number)` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#log10(number) ` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_MOD.html(number1, number2)` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#mod(number1, number2)` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_PI.html` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#acos(-1)` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_POWER.html(expression1, expression2) ` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#power(expression1, expression2)` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_RADIANS.html(number)` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#acos(-1)*(number/180)` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_RANDOM.html() ` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#rand() ` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_ROUND.html(number [, integer])` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#rand(number [, integer])` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_SIN.html(number)` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#sin(number)` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_SIGN.html(number)` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#sign(number)` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_SQRT.html(number) ` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#sqrt(number) ` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_TAN.html(number) ` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#tan(number) ` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_TO_HEX.html(number)` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#format_string('%x', number)` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_TRUNC.html(number [, integer])+-+++` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/mathematical_functions#trunc(number [, integer]) ` |

### String functions

| **Amazon Redshift** | **BigQuery** |
|---|---|
| string1 `https://docs.aws.amazon.com/redshift/latest/dg/r_concat_op.html string2` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#concat(string1, string2)` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_BPCHARCMP.html(string1, string2)` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/conditional_expressions#case WHEN string1 = string2 THEN 0 WHEN string1 > string2 THEN 1 ELSE -1 END ` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_BTRIM.html(string [, matching_string])` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#trim(string [, matching_string])` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_BTTEXT_PATTERN_CMP.html(string1, string2)` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/conditional_expressions#case WHEN string1 = string2 THEN 0 WHEN string1 > string2 THEN 1 ELSE -1 END ` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_CHAR_LENGTH.html(expression)` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#char_length(expression)` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_CHARACTER_LENGTH.html(expression)` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#character_length(expression)` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_CHARINDEX.html(substring, string)` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#strpos(string, substring) ` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_CHR.html(number)` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#code_points_to_string([number]) ` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_CONCAT.html(string1, string2)` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#concat(string1, string2)` <br /> Note: BigQuery's `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#concat`(...) supports concatenating any number of strings. |
| `https://docs.aws.amazon.com/redshift/latest/dg/crc32-function.html` | Custom user-defined function |
| `https://docs.aws.amazon.com/redshift/latest/dg/FUNC_SHA1.html(string)` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/hash_functions#sha1(string)` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_INITCAP.html` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#initcap` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_LEFT.html(string, integer) ` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#substr(string, 0, integer) ` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_LEFT.html(string, integer) ` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#substr(string, -integer)` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_LEN.html(expression)` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#length(expression)` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_LENGTH.html(expression)` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#length(expression)` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_LOWER.html(string) ` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#lower(string) ` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_LPAD.html(string1, length[, string2])` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#lpad(string1, length[, string2])` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_LPAD.html(string1, length[, string2])` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#rpad(string1, length[, string2])` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_LTRIM.html(string, trim_chars)` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#ltrim(string, trim_chars)` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_MD5.html(string)` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/hash_functions#md5(string)` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_OCTET_LENGTH.html(expression)` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#byte_length(expression)` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_POSITION.html(substring IN string)` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#strpos(string, substring)` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_QUOTE_IDENT.html(string)` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#concat('"',string,'"')` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_QUOTE_LITERAL.html(string)` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#concat("'",string,"'") ` |
| `https://docs.aws.amazon.com/redshift/latest/dg/REGEXP_COUNT.html( source_string, pattern [,position] ) ` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/array_functions#array_length( https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#regexp_extract_all( source_string, pattern ) ) ` <br /> If `position` is specified: `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/array_functions#array_length( https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#regexp_extract_all( https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#substr(source_string, https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/conditional_expressions#if(position <= 0, 1, position)), pattern ) ) ` <br /> Note: BigQuery provides regular expression support using the `https://github.com/google/re2/wiki/Syntax` library; see that documentation for its regular expression syntax. |
| `https://docs.aws.amazon.com/redshift/latest/dg/REGEXP_INSTR.html( source_string, pattern [,position [,occurrence]] ) ` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/conditional_expressions#ifnull( https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#strpos( source_string, https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#regexp_extract( source_string, pattern) ),0) ` If `source_string` is specified: <br /> `REGEXP_REPLACE( source_string, pattern, replace_string ) ` If `position` is specified: <br /> `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/conditional_expressions#ifnull( https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#strpos( https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#substr(source_string, https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/conditional_expressions(position <= 0, 1, position)), https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#regexp_extract( https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#substr(source_string, https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/conditional_expressions#if(position <= 0, 1, position)), pattern) ) + https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/conditional_expressions#if(position <= 0, 1, position) - 1, 0)` <br /> If `occurrence` is specified: <br /> `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/conditional_expressions#ifnull( https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#strpos( https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#substr(source_string, https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/conditional_expressions#if(position <= 0, 1, position)), https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#regexp_extract_all( https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#substr(source_string, https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/conditional_expressions#if(position <= 0, 1, position)), pattern )[https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/operators#array_subscript_operator(occurrence)] ) + https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/conditional_expressions#if(position <= 0, 1, position) - 1, 0) ` <br /> Note: BigQuery provides regular expression support using the `https://github.com/google/re2/wiki/Syntax` library; see that documentation for its regular expression syntax. |
| `https://docs.aws.amazon.com/redshift/latest/dg/REGEXP_REPLACE.html( source_string, pattern [, replace_string [, position]] ) ` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#regexp_replace( source_string, pattern, "" )` <br /> If `source_string` is specified: ` https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#regexp_replace( source_string, pattern, replace_string )` <br /> If `position` is specified: `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/conditional_expressions#case WHEN position > https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#length(source_string) THEN source_string WHEN position <= 0 THEN https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#regexp_replace( source_string, pattern, "" ) ELSE https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#concat( https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#substr( source_string, 1, position - 1), https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#regexp_replace( https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#substr(source_string, position), pattern, replace_string ) ) END` |
| `https://docs.aws.amazon.com/redshift/latest/dg/REGEXP_SUBSTR.html( source_string, pattern [, position [, occurrence]] ) ` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#regexp_extract( source_string, pattern )` <br /> If `position` is specified: <br /> `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#regexp_extract( https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#substr(source_string, https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/conditional_expressions#if(position <= 0, 1, position)), pattern )` <br /> If `occurrence` is specified: `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#regexp_extract_all( https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#substr(source_string, https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/conditional_expressions#if(position <= 0, 1, position)), pattern )[https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/operators#array_subscript_operator(occurrence)]` <br /> Note: BigQuery provides regular expression support using the `https://github.com/google/re2/wiki/Syntax` library; see that documentation for its regular expression syntax. |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_REPEAT.html(string, integer)` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#repeat(string, integer)` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_REPLACE.html(string, old_chars, new_chars)` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#replace(string, old_chars, new_chars)` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_REPLICATE.html(string, integer)` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#repeat(string, integer)` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_REVERSE.html(expression)` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#reverse(expression)` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_RTRIM.html(string, trim_chars)` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#rtrim(string, trim_chars)` |
| `https://docs.aws.amazon.com/redshift/latest/dg/SPLIT_PART.html(string, delimiter, part)` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#split( string delimiter )https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/operators#array_subscript_operator(part)` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_STRPOS.html(string, substring)` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#strpos(string, substring)` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_STRTOL.html(string, base)` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_SUBSTRING.html( string, start_position, number_characters ) ` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#substr( string, start_position, number_characters ) ` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_TEXTLEN.html(expression)` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#length(expression)` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_TRANSLATE.html( expression, characters_to_replace, characters_to_substitute ) ` | Can be implemented using UDFs: `https://docs.cloud.google.com/bigquery/docs/user-defined-functions translate(expression STRING, characters_to_replace STRING, characters_to_substitute STRING) AS ( IF(LENGTH(characters_to_replace) < LENGTH(characters_to_substitute) OR LENGTH(expression) < LENGTH(characters_to_replace), expression, (SELECT STRING_AGG( IFNULL( (SELECT ARRAY_CONCAT([c], SPLIT(characters_to_substitute, ''))[SAFE_OFFSET(( SELECT IFNULL(MIN(o2) + 1, 0) FROM UNNEST(SPLIT(characters_to_replace, '')) AS k WITH OFFSET o2 WHERE k = c))] ), ''), '' ORDER BY o1) FROM UNNEST(SPLIT(expression, '')) AS c WITH OFFSET o1 )) ); ` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_TRIM.html([BOTH] string)` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#trim(string)` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_TRIM.html([BOTH] characters FROM string)` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#trim(string, characters) ` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_UPPER.html(string)` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#upper(string)` |

### Data type formatting functions

| **Amazon Redshift** | **BigQuery** |
|---|---|
| `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/conversion_functions#cast(expression AS type)` | `https://docs.aws.amazon.com/redshift/latest/dg/r_CAST_function.html#r_CAST_function-cast(expression AS type)` |
| `expression https://docs.aws.amazon.com/redshift/latest/dg/r_CAST_function.html#r_CAST_function-cast type` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/conversion_functions#cast(expression AS type) ` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_CAST_function.html#convert-function(type, expression)` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/conversion_functions#cast(expression AS type)` |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_TO_CHAR.html( timestamp_expression, format )` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#format_timestamp( format, timestamp_expression ) ` <br /> Note: BigQuery and Amazon Redshift differ in how to specify a format string for `timestamp_expression`. |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_TO_CHAR.html( numeric_expression, format )` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#format_string( format, numeric_expression )` <br /> Note: BigQuery and Amazon Redshift differ in how to specify a format string for `timestamp_expression`. |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_TO_DATE_function.html(date_string, format)` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#parse_date(date_string, format)` <br /> Note: BigQuery and Amazon Redshift differ in how to specify a format string for `date_string`. |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_TO_NUMBER.html(string, format)` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/conversion_functions#cast( https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#format_string( format, numeric_expression ) TO INT64 ) ` <br /> Note: BigQuery and Amazon Redshift differ in how to specify a numeric format string. |

BigQuery also supports [`SAFE_CAST`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/conversion_functions#safe_casting)`(expression
AS typename)`, which returns `NULL` if BigQuery is unable to perform a cast; for
example,
[`SAFE_CAST`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/conversion_functions#safe_casting)`("apple"
AS INT64)` returns `NULL`.

## DML syntax

This section addresses differences in data management language syntax between
Amazon Redshift and BigQuery.

### `INSERT` statement

Amazon Redshift offers a configurable `DEFAULT` keyword for columns. In
BigQuery, the `DEFAULT` value for nullable columns is `NULL`,
and `DEFAULT` is not supported for
required columns. Most
[Amazon Redshift `INSERT` statements](https://docs.aws.amazon.com/redshift/latest/dg/r_INSERT_30.html)
are compatible with BigQuery. The following table shows exceptions.

| **Amazon Redshift** | **BigQuery** |
|---|---|
| `INSERT INTO table (column1 [, ...]) DEFAULT VALUES` | `INSERT [INTO] table (column1 [, ...]) VALUES (DEFAULT [, ...])` |
| `INSERT INTO table (column1, [,...]) VALUES ( SELECT ... FROM ... )` | `INSERT [INTO] table (column1, [,...]) SELECT ... FROM ...` |

BigQuery also supports inserting values using a subquery (where one of the
values is computed using a subquery), which is not supported in Amazon Redshift. For
example:

    INSERT INTO table (column1, column2)
    VALUES ('value_1', (
    SELECT column2
    FROM table2
    ))

### `COPY` statement

Amazon Redshift's
[`COPY`](https://docs.aws.amazon.com/redshift/latest/dg/r_COPY.html)command
loads data into a table from data files or from an Amazon DynamoDB table.
BigQuery does not use the `SQL COPY` command to load data,
but you can use any of several non-SQL tools and options to
[load data into BigQuery tables](https://docs.cloud.google.com/bigquery/docs/loading-data).
You can also use data pipeline sinks provided in
[Apache Spark](https://docs.cloud.google.com/dataproc/docs/concepts/connectors/cloud-storage#other_sparkhadoop_clusters)
or
[Apache Beam](https://beam.apache.org/documentation/io/built-in/google-bigquery/#writing-to-bigquery)
to write data into BigQuery.

### `UPDATE` statement

Most Amazon Redshift `UPDATE` statements are compatible with BigQuery. The following
table shows exceptions.

| **Amazon Redshift** | **BigQuery** |
|---|---|
| `UPDATE table SET column = expression [,...] [FROM ...] ` | `UPDATE table SET column = expression [,...] [FROM ...] WHERE TRUE` Note: All `UPDATE` statements in BigQuery require a `WHERE` keyword, followed by a condition. |
| `UPDATE table SET column = DEFAULT [,...] [FROM ...] [WHERE ...]` | `UPDATE table SET column = NULL [, ...] [FROM ...] WHERE ... ` Note: BigQuery's `UPDATE` command does not support `DEFAULT` values. <br /> If the Amazon Redshift `UPDATE` statement does not include a `WHERE` clause, the BigQuery `UPDATE` statement should be conditioned `WHERE TRUE`. |

### `DELETE` and `TRUNCATE` statements

The `DELETE` and `TRUNCATE` statements are both ways to remove rows from a table
without affecting the table schema or indexes.

In Amazon Redshift, the `TRUNCATE` statement is recommended over an
unqualified `DELETE` statement because it is
faster and does not require `VACUUM` and `ANALYZE` operations afterward.
However, you can use `DELETE` statements to achieve the same effect.

In BigQuery, the `DELETE` statement must have a `WHERE` clause. For more
information about `DELETE` in BigQuery, see the
BigQuery
[`DELETE` examples](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/dml-syntax#delete_examples)
in the DML documentation.

| **Amazon Redshift** | **BigQuery** |
|---|---|
| `https://docs.aws.amazon.com/redshift/latest/dg/r_TRUNCATE.html table_name` <br /> `https://docs.aws.amazon.com/redshift/latest/dg/r_TRUNCATE.html table_name` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/dml-syntax#delete_statement table_name WHERE TRUE ` BigQuery `DELETE` statements require a `WHERE` clause. |
| `DELETE FROM table_name USING other_table WHERE table_name.id=other_table.id` | `DELETE FROM table_name WHERE table_name.id IN ( SELECT id FROM other_table ) ` `DELETE FROM table_name WHERE EXISTS ( SELECT id FROM other_table WHERE table_name.id = other_table.id ) ` <br /> In Amazon Redshift, `USING` allows additional tables to be referenced in the `WHERE` clause. This can be achieved in BigQuery by using a subquery in the `WHERE` clause. |

### `MERGE` statement

The `MERGE` statement can combine `INSERT`, `UPDATE`,
and `DELETE` operations into a
single upsert statement and perform the operations atomically. The `MERGE`
operation must match at most one source row for each target row.

Amazon Redshift does not support a single `MERGE` command. However, a
merge operation
can be performed in Amazon Redshift by performing
`INSERT`, `UPDATE`, and `DELETE` operations
in a transaction.

#### Merge operation by replacing existing rows

In Amazon Redshift, an overwrite of all of the columns in the target table can be
performed using a `DELETE` statement and then an `INSERT` statement. The `DELETE`
statement removes rows that should be updated, and then the `INSERT` statement
inserts the updated rows. BigQuery tables are limited to
1,000 DML statements
per day, so you should consolidate `INSERT`, `UPDATE`, and `DELETE`
statements into a
single `MERGE` statement as shown in the following table.

| **Amazon Redshift** | **BigQuery** |
|---|---|
| See [Performing a merge operation by](https://docs.aws.amazon.com/redshift/latest/dg/merge-replacing-existing-rows.html) [replacing existing rows](https://docs.aws.amazon.com/redshift/latest/dg/merge-replacing-existing-rows.html). `CREATE TEMP TABLE temp_table; INSERT INTO temp_table SELECT * FROM source WHERE source.filter = 'filter_exp'; BEGIN TRANSACTION; DELETE FROM target USING temp_table WHERE target.key = temp_table.key; INSERT INTO target SELECT * FROM temp_table; END TRANSACTION; DROP TABLE temp_table; ` | `MERGE target USING source ON target.key = source.key WHEN MATCHED AND source.filter = 'filter_exp' THEN UPDATE SET target.col1 = source.col1, target.col2 = source.col2, ... ` Note: All columns must be listed if updating all columns. |
| See [Performing a merge operation by](https://docs.aws.amazon.com/redshift/latest/dg/merge-specify-a-column-list.html) [specifying a column list](https://docs.aws.amazon.com/redshift/latest/dg/merge-specify-a-column-list.html). `CREATE TEMP TABLE temp_table; INSERT INTO temp_table SELECT * FROM source WHERE source.filter = 'filter_exp'; BEGIN TRANSACTION; UPDATE target SET col1 = temp_table.col1, col2 = temp_table.col2 FROM temp_table WHERE target.key=temp_table.key; INSERT INTO target SELECT * FROM ` | `MERGE target USING source ON target.key = source.key WHEN MATCHED AND source.filter = 'filter_exp' THEN UPDATE SET target.col1 = source.col1, target.col2 = source.col2 ` |

## DDL syntax

This section addresses differences in data definition language syntax between
Amazon Redshift and BigQuery.

### `SELECT INTO` statement

In Amazon Redshift, the `SELECT INTO` statement can be used to insert the results of a
query into a new table, combining table creation and insertion.

| **Amazon Redshift** | **BigQuery** |
|---|---|
| `SELECT expression, ... INTO table FROM ... ` | `INSERT table SELECT expression, ... FROM ... ` |
| `WITH subquery_table AS ( SELECT ... ) SELECT expression, ... INTO table FROM subquery_table ... ` | `INSERT table WITH subquery_table AS ( SELECT ... ) SELECT expression, ... FROM subquery_table ... ` |
| `SELECT expression INTO TEMP table FROM ... SELECT expression INTO TEMPORARY table FROM ... ` | BigQuery offers several ways to emulate temporary tables. See the [temporary tables](https://docs.cloud.google.com/bigquery/docs/migration/redshift-sql#temporary_tables) section for more information. |

### `CREATE TABLE` statement

Most Amazon Redshift
[`CREATE TABLE`](https://docs.teradata.com/reader/scPHvjfglIlB8F70YliLAw/t9ZHBbmVpK7GrnocHmVG1Q)
statements are compatible with BigQuery, except for the following syntax elements, which
are not used in BigQuery:

| **Amazon Redshift** | **BigQuery** |
|---|---|
| `CREATE TABLE table_name ( col1 data_type1 NOT NULL, col2 data_type2 NULL, col3 data_type3 UNIQUE, col4 data_type4 PRIMARY KEY, col5 data_type5 ) ` <br /> Note: `UNIQUE` and `PRIMARY KEY` constraints are informational and [are not enforced by the Amazon Redshift](https://docs.aws.amazon.com/redshift/latest/dg/t_Defining_constraints.html) [system](https://docs.aws.amazon.com/redshift/latest/dg/t_Defining_constraints.html). | `CREATE TABLE table_name ( col1 data_type1 NOT NULL, col2 data_type2, col3 data_type3, col4 data_type4, col5 data_type5, )` |
| `CREATE TABLE table_name ( col1 data_type1[,...] table_constraints ) where table_constraints are: [UNIQUE(column_name [, ... ])] [PRIMARY KEY(column_name [, ...])] [FOREIGN KEY(column_name [, ...]) REFERENCES reftable [(refcolumn)] ` Note: `UNIQUE` and `PRIMARY KEY` constraints are informational and [are not enforced by the Amazon Redshift system](https://docs.aws.amazon.com/redshift/latest/dg/t_Defining_constraints.html). | `CREATE TABLE table_name ( col1 data_type1[,...] ) https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#partition_expression column_name https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#clustering_column_list column_name [, ...]` <br /> Note: BigQuery does not use `UNIQUE`, `PRIMARY KEY`, or `FOREIGN KEY` table constraints. To achieve similar optimization that these constraints provide during query execution, partition and cluster your BigQuery tables. `CLUSTER BY` supports up to 4 columns. |
| `CREATE TABLE table_name LIKE original_table_name ` | Reference [this example](https://docs.cloud.google.com/bigquery/docs/information-schema-tables#example_3) to learn how to use the `INFORMATION_SCHEMA` tables to copy column names, data types, and `NOT NULL` constraints to a new table. |
| `CREATE TABLE table_name ( col1 data_type1 ) BACKUP NO ` Note: In Amazon Redshift, the `https://docs.aws.amazon.com/redshift/latest/dg/r_CREATE_TABLE_NEW.html` setting is specified to save processing time and reduce storage space. | The `BACKUP NO` table option is not used or needed because BigQuery automatically keeps up to 7 days of historical versions of all of your tables with no effect on processing time or billed storage. |
| `CREATE TABLE table_name ( col1 data_type1 ) table_attributes where table_attributes are: [DISTSTYLE {AUTO|EVEN|KEY|ALL}] [DISTKEY (column_name)] [[COMPOUND|INTERLEAVED] SORTKEY (column_name [, ...])] ` | BigQuery supports clustering, which allows storing keys in sorted order. |
| `CREATE TABLE table_name AS SELECT ... ` | `CREATE TABLE table_name AS SELECT ... ` |
| `CREATE TABLE IF NOT EXISTS table_name ... ` | `CREATE TABLE IF NOT EXISTS table_name ...` |

BigQuery also supports the DDL statement `CREATE OR REPLACE TABLE`,
which overwrites a table if it already exists.

BigQuery's `CREATE TABLE` statement also supports the following clauses, which do
not have an Amazon Redshift equivalent:

- [`PARTITION BY partition` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#partition_expression)
- [`CLUSTER BY clustering_column_list`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#clustering_column_list)
- [`OPTIONS(table_options_list)`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#table_option_list)

For more information about `CREATE TABLE` in BigQuery, see the
BigQuery
[`CREATE TABLE` examples](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create-table-examples)
in the DML documentation.

#### Temporary tables

Amazon Redshift supports temporary tables, which are only visible within the current
session. There are several ways to emulate temporary tables in BigQuery:

- Dataset TTL: Create a dataset that has a short time to live (for example, one hour) so that any tables created in the dataset are effectively temporary because they won't persist longer than the dataset's time to live. You can prefix all of the table names in this dataset with temp to clearly denote that the tables are temporary.
- Table TTL: Create a table that has a table-specific short time to live
  using DDL statements similar to the following:

      CREATE TABLE
      temp.name (col1, col2, ...)
      OPTIONS (expiration_timestamp=TIMESTAMP_ADD(CURRENT_TIMESTAMP(),
      INTERVAL 1 HOUR));

### `CREATE VIEW` statement

The following table shows equivalents between Amazon Redshift and BigQuery for the
`CREATE VIEW` statement.

| **Amazon Redshift** | **BigQuery** |
|---|---|
| `CREATE VIEW view_name AS SELECT ...`code\> | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_view_statement view_name AS SELECT ...` |
| `CREATE OR REPLACE VIEW view_name AS SELECT ...` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_view_statement view_name AS SELECT ... ` |
| `CREATE VIEW view_name (column_name, ...) AS SELECT ... ` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_view_statement view_name AS SELECT ...` |
| Not supported. | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_view_statement c view_name OPTIONS(https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#view_option_list) AS SELECT ... ` Creates a new view only if the view does not exist in the specified dataset. |
| `CREATE VIEW view_name AS SELECT ... https://docs.aws.amazon.com/redshift/latest/dg/r_CREATE_VIEW.html ` <br /> In Amazon Redshift, a late binding view is required in order to reference an external table. | In BigQuery, to create a view, all referenced objects must already exist. BigQuery allows you to [query external data sources.](https://docs.cloud.google.com/bigquery/external-data-sources) |

## User-defined functions (UDFs)

A UDF lets you create functions for custom operations. These functions accept
columns of input, perform actions, and return the result of those actions as a
value.

Both Amazon Redshift and BigQuery support UDFs using SQL expressions. Additionally, in
Amazon Redshift you can create a
[Python-based UDF](https://docs.aws.amazon.com/redshift/latest/dg/udf-python-language-support.html),
and in BigQuery you can create a
[JavaScript-based UDF](https://docs.cloud.google.com/bigquery/docs/user-defined-functions#javascript-udf-structure).

Refer to the
[Google Cloud BigQuery utilities GitHub repository](https://github.com/GoogleCloudPlatform/bigquery-utils/tree/master/udfs/community)
for a library of common BigQuery UDFs.

### `CREATE FUNCTION` syntax

The following table addresses differences in SQL UDF creation syntax between
Amazon Redshift and BigQuery.

| **Amazon Redshift** | **BigQuery** |
|---|---|
| `https://docs.aws.amazon.com/redshift/latest/dg/r_CREATE_FUNCTION.html function_name ([sql_arg_name sql_arg_data_type[,..]]) RETURNS data_type IMMUTABLE AS $$ sql_function_definition $$ LANGUAGE sql ` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_function_statement function_name ([sql_arg_name sql_arg_data_type[,..]]) AS sql_function_definition ` Note: In a BigQuery [SQL UDF](https://docs.cloud.google.com/bigquery/docs/user-defined-functions#sql-udf-structure), a return data type is optional. BigQuery infers the result type of the function from the SQL function body when a query calls the function. |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_CREATE_FUNCTION.html function_name ([sql_arg_name sql_arg_data_type[,..]]) RETURNS data_type { VOLATILE | STABLE | IMMUTABLE } AS $$ sql_function_definition $$ LANGUAGE sql ` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_function_statement function_name ([sql_arg_name sql_arg_data_type[,..]]) RETURNS data_type AS sql_function_definition ` Note: Function volatility is not a configurable parameter in BigQuery. All BigQuery UDF volatility is equivalent to Amazon Redshift's `IMMUTABLE` volatility (that is, it does not do database lookups or otherwise use information not directly present in its argument list). |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_CREATE_FUNCTION.html function_name ([sql_arg_name sql_arg_data_type[,..]]) RETURNS data_type IMMUTABLE AS $$ SELECT_clause $$ LANGUAGE sql ` Note: Amazon Redshift supports only a `SQL SELECT` clause as function definition. Also, the `SELECT` clause cannot include any of the `FROM, INTO, WHERE, GROUP BY, ORDER BY,` and `LIMIT` clauses. | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_function_statement function_name ([sql_arg_name sql_arg_data_type[,..]]) RETURNS data_type AS sql_expression ` Note: BigQuery supports any SQL expressions as function definition. However, referencing tables, views, or models is not supported. |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_CREATE_FUNCTION.html function_name ([sql_arg_name sql_arg_data_type[,..]]) RETURNS data_type IMMUTABLE AS $$ sql_function_definition $$ LANGUAGE sql ` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_function_statement function_name ([sql_arg_name sql_arg_data_type[,..]]) RETURNS data_type AS sql_function_definition` <br /> Note: Language literal need not be specified in a GoogleSQL UDF. BigQuery interprets the SQL expression by default. Also, the Amazon Redshift dollar quoting (`$$`) is not supported in BigQuery. |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_CREATE_FUNCTION.html function_name (integer, integer) RETURNS integer IMMUTABLE AS $$ SELECT $1 + $2 $$ LANGUAGE sql` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_function_statement function_name (x INT64, y INT64) RETURNS INT64 AS SELECT x + y ` Note: BigQuery UDFs require all input arguments to be named. The Amazon Redshift argument variables (`$1`, `$2`, ...) are not supported in BigQuery. |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_CREATE_FUNCTION.html function_name (integer, integer) RETURNS integer IMMUTABLE AS $$ SELECT $1 + $2 $$ LANGUAGE sql` Note: Amazon Redshift does not support `ANY TYPE` for SQL UDFs. However, it supports using the `https://docs.aws.amazon.com/redshift/latest/dg/udf-creating-a-scalar-udf.html#udf-anyelement-data-type` data type in Python-based UDFs. | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_function_statement function_name (x ANY TYPE, y ANY TYPE) AS SELECT x + y` Note: BigQuery supports using `ANY TYPE` as argument type. The function accepts an input of any type for this argument. For more information, see [templated parameter](https://docs.cloud.google.com/bigquery/docs/user-defined-functions#templated-sql-udf-parameters) in BigQuery. |

BigQuery also supports the `CREATE FUNCTION IF NOT EXISTS` statement, which
treats the query as successful and takes no action if a function with the same
name already exists.

BigQuery's `CREATE FUNCTION` statement also supports creating
[`TEMPORARY`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_function_statement)
or `TEMP` functions, which do not have an Amazon Redshift equivalent.

See
[calling UDFs](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/syntax#calling_persistent_user-defined_functions_udfs)
for details on executing a BigQuery-persistent UDF.

### `DROP FUNCTION` syntax

The following table addresses differences in `DROP FUNCTION` syntax between
Amazon Redshift and BigQuery.

| **Amazon Redshift** | **BigQuery** |
|---|---|
| `https://docs.aws.amazon.com/redshift/latest/dg/r_DROP_FUNCTION.html function_name ( [arg_name] arg_type [, ...] ) [ CASCADE | RESTRICT ]` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#drop_function_statement dataset_name.function_name` <br /> Note: BigQuery does not require using the function's signature for deleting the function. Also, removing function dependencies is not supported in BigQuery. |

BigQuery also supports the
[`DROP FUNCTION IF EXISTS`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#drop_function_statement) statement,
which deletes the function only if the function exists in the specified
dataset.

BigQuery requires that you specify the `project_name`
if the function is not located in the current project.

### UDF components

This section highlights the similarities and differences in UDF components
between Amazon Redshift andBigQuery.

| **Component** | **Amazon Redshift** | **BigQuery** |
|---|---|---|
| **Name** | Amazon Redshift [recommends](https://docs.aws.amazon.com/redshift/latest/dg/udf-naming-udfs.html) using the prefix `_f `for function names to avoid conflicts with existing or future built-in SQL function names. | In BigQuery, you can use any custom function name. |
| **Arguments** | Arguments are optional. You can use name and data types for Python UDF arguments and only data types for SQL UDF arguments. In SQL UDFs, you must refer to arguments using `$1`, `$2`, and so on. Amazon Redshift also [restricts the number of arguments](https://docs.aws.amazon.com/redshift/latest/dg/r_CREATE_FUNCTION.html) to 32. | Arguments are optional, but if you specify arguments, they must use both name and data types for both JavaScript and SQL UDFs. The maximum number of arguments for a persistent UDF is 256. |
| **Data type** | Amazon Redshift supports a different set of data types for [SQL](https://docs.aws.amazon.com/redshift/latest/dg/c_Supported_data_types.html) and [Python](https://docs.aws.amazon.com/redshift/latest/dg/udf-data-types.html) UDFs. For a Python UDF, the data type might also be `https://docs.aws.amazon.com/redshift/latest/dg/udf-creating-a-scalar-udf.html#udf-anyelement-data-type`. <br /> You must specify a `RETURN `data type for both SQL and Python UDFs. <br /> See [Data types](https://docs.cloud.google.com/bigquery/docs/migration/redshift-sql#data_types) in this document for equivalents between data types in Amazon Redshift and in BigQuery. | BigQuery supports a different set of data types for [SQL](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types) and [JavaScript](https://docs.cloud.google.com/bigquery/docs/user-defined-functions#supported-javascript-udf-data-types) UDFs. For a SQL UDF, the data type might also be `ANY TYPE`. For more information, see [templated parameters](https://docs.cloud.google.com/bigquery/docs/user-defined-functions#templated-sql-udf-parameters) in BigQuery. <br /> The `RETURN `data type is optional for SQL UDFs. <br /> See [Supported JavaScript UDF data types](https://docs.cloud.google.com/bigquery/docs/user-defined-functions#supported-javascript-udf-data-types) for information on how BigQuery data types map to JavaScript data types. |
| **Definition** | For both SQL and Python UDFs, you must enclose the function definition using dollar quoting, as in a pair of dollar signs (`$$`), to indicate the start and end of the function statements. <br /> For [SQL UDFs,](https://docs.aws.amazon.com/redshift/latest/dg/udf-creating-a-scalar-sql-udf.html) Amazon Redshift supports only a SQL `SELECT `clause as the function definition. Also, the `SELECT` clause cannot include any of the `FROM`, `INTO`, `WHERE`, `GROUP` `BY`, `ORDER BY`, and `LIMIT `clauses. <br /> For [Python UDFs](https://docs.aws.amazon.com/redshift/latest/dg/udf-python-language-support.html), you can write a Python program using the [Python 2.7 Standard Library](https://docs.python.org/2/library/index.html) or import your custom modules by creating one using the `https://docs.aws.amazon.com/redshift/latest/dg/r_CREATE_LIBRARY.html `command. | In BigQuery, you need to enclose the JavaScript code in quotes. See [Quoting](https://docs.cloud.google.com/bigquery/docs/user-defined-functions#quoting-rules) [rules](https://docs.cloud.google.com/bigquery/docs/user-defined-functions#quoting-rules) for more information. <br /> For [SQL UDFs](https://docs.cloud.google.com/bigquery/docs/user-defined-functions#sql-udf-structure), you can use any SQL expressions as the function definition. However, BigQuery doesn't support referencing tables, views, or models. <br /> For [JavaScript UDFs](https://docs.cloud.google.com/bigquery/docs/user-defined-functions#javascript-udf-structure), you can [include](https://docs.cloud.google.com/bigquery/docs/user-defined-functions#including-javascript-libraries) [external code libraries](https://docs.cloud.google.com/bigquery/docs/user-defined-functions#including-javascript-libraries) directly using the `OPTIONS `section. You can also use the [BigQuery UDF test tool](https://github.com/GoogleCloudPlatform/bigquery-udf-test-tool) to test your functions. |
| **Language** | You must use the `LANGUAGE `literal to specify the language as either `sql `for SQL UDFs or `plpythonu `for Python UDFs. | You need not specify `LANGUAGE `for SQL UDFs but must specify the language as `js `for JavaScript UDFs. |
| **State** | Amazon Redshift does not support creating temporary UDFs. <br /> Amazon Redshift provides an option to define the [volatility of a function](https://docs.aws.amazon.com/redshift/latest/dg/r_CREATE_FUNCTION.html#r_CREATE_FUNCTION-parameters) using `VOLATILE`, `STABLE`, or `IMMUTABLE `literals. This is used for optimization by the query optimizer. | BigQuery supports both persistent and temporary UDFs. You can reuse persistent UDFs across multiple queries, whereas you can only use temporary UDFs in a single query. <br /> Function volatility is not a configurable parameter in BigQuery. All BigQuery UDF volatility is equivalent to Amazon Redshift's `IMMUTABLE `volatility. |
| **Security and privileges** | To create a UDF, you must have [permission for usage on language](https://docs.aws.amazon.com/redshift/latest/dg/udf-security-and-privileges.html) for SQL or plpythonu (Python). By default, `USAGE ON LANGUAGE SQL `is granted to `PUBLIC`, but you must explicitly grant `USAGE ON LANGUAGE PLPYTHONU `to specific users or groups. Also, you must be a superuser to replace a UDF. | Granting explicit permissions for creating or deleting any type of UDF is not necessary in BigQuery. Any user assigned a role of [BigQuery Data Editor](https://docs.cloud.google.com/bigquery/docs/access-control#bigquery) (having `bigquery.routines.\*`as one of the permissions) can create or delete functions for the specified dataset. BigQuery also supports creating custom roles. This can be managed using [Cloud IAM](https://docs.cloud.google.com/iam/docs/roles-overview#custom). |
| **Limits** | See [Python UDF limits](https://docs.aws.amazon.com/redshift/latest/dg/udf-constraints.html). | See [limits on user-defined functions](https://docs.cloud.google.com/bigquery/quotas#udf_limits). |

## Metadata and transaction SQL statements

| **Amazon Redshift** | **BigQuery** |
|---|---|
| `SELECT * FROM https://docs.aws.amazon.com/redshift/latest/dg/c_check_last_analyze.html WHERE name = 'T';` | Not used in BigQuery. You don't need to gather statistics in order to improve query performance. To get information about your data distribution, you can use [approximate aggregate functions](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/approximate_aggregate_functions). |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_ANALYZE.html [[ table_name[(column_name [, ...])]]` | Not used in BigQuery. |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_LOCK.html table_name;` | Not used in BigQuery. |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_BEGIN.html; SELECT ... https://docs.aws.amazon.com/redshift/latest/dg/r_END.html;` | BigQuery uses snapshot isolation. For details, see [Consistency guarantees](https://docs.cloud.google.com/bigquery/docs/migration/redshift-sql#consistency_guarantees_and_transaction_isolation). |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_EXPLAIN.html ...` | Not used in BigQuery. <br /> Similar features are the [query plan explanation](https://docs.cloud.google.com/bigquery/query-plan-explanation) in the BigQuery Google Cloud console, and in [audit logging in Cloud Monitoring](https://docs.cloud.google.com/bigquery/docs/monitoring). |
| `SELECT * FROM https://docs.aws.amazon.com/redshift/latest/dg/r_SVV_TABLE_INFO.html WHERE table = 'T';` | `SELECT * EXCEPT(is_typed) FROM mydataset.INFORMATION_SCHEMA.TABLES;` <br /> For more information see [Introduction to BigQuery `INFORMATION_SCHEMA`](https://docs.cloud.google.com/bigquery/docs/information-schema-intro). |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_VACUUM_command.html [table_name]` | Not used in BigQuery. BigQuery [clustered tables are automatically sorted](https://docs.cloud.google.com/bigquery/docs/clustered-tables#automatic_reclustering). |

### Multi-statement and multi-line SQL statements

Both Amazon Redshift and BigQuery support transactions (sessions)
and therefore support statements separated by semicolons that are consistently
executed together. For more information, see
[Multi-statement transactions](https://docs.cloud.google.com/bigquery/docs/transactions).

## Procedural SQL statements

### `CREATE PROCEDURE` statement

| **Amazon Redshift** | **BigQuery** |
|---|---|
| `https://docs.aws.amazon.com/redshift/latest/dg/r_CREATE_PROCEDURE.html` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_procedure` if a name is required. <br /> Otherwise, use inline with `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/scripting#begin` or in a single line with `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_function_statement`. |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_CALL_procedure.html` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/scripting#call` |

### Variable declaration and assignment

| **Amazon Redshift** | **BigQuery** |
|---|---|
| `https://docs.aws.amazon.com/redshift/latest/dg/c_PLpgSQL-structure.html#r_PLpgSQL-variable-declaration` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/scripting#declare` <br /> Declares a variable of the specified type. |
| `https://docs.aws.amazon.com/redshift/latest/dg/r_SET.html` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/scripting#declare` <br /> Sets a variable to have the value of the provided expression, or sets multiple variables at the same time based on the result of multiple expressions. |

### Error condition handlers

In Amazon Redshift, an error encountered during the execution of a stored procedure
ends the execution flow, ends the transaction, and rolls back the transaction.
These results occur because subtransactions are not supported. In an
Amazon Redshift-stored procedure, the only supported `handler_statement`
is `RAISE`. In BigQuery, error
handling is a core feature of the main control flow, similar to what other
languages provide with `TRY ... CATCH` blocks.

| **Amazon Redshift** | **BigQuery** |
|---|---|
| `https://docs.aws.amazon.com/redshift/latest/dg/stored-procedure-trapping-errors.html https://docs.aws.amazon.com/redshift/latest/dg/stored-procedure-trapping-errors.html` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/scripting#beginexception` |
| `https://docs.amazonaws.cn/en_us/redshift/latest/dg/c_PLpgSQL-statements.html#r_PLpgSQL-messages-errors` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/scripting#raise` |
| `[ <<label>> ] [ DECLARE declarations ] BEGIN statements EXCEPTION BEGIN statements EXCEPTION WHEN OTHERS THEN Handler_statements END;` | `BEGIN BEGIN ... EXCEPTION WHEN ERROR THEN SELECT 1/0; END; EXCEPTION WHEN ERROR THEN -- The exception thrown from the inner exception handler lands here. END;` |

### Cursor declarations and operations

Because BigQuery doesn't support cursors or sessions, the following statements
aren't used in BigQuery:

- [`DECLARE`](https://docs.aws.amazon.com/redshift/latest/dg/declare.html) [`cursor_name`](https://docs.aws.amazon.com/redshift/latest/dg/declare.html) [`CURSOR`](https://docs.aws.amazon.com/redshift/latest/dg/declare.html) `[FOR] ...`
- [`PREPARE`](https://docs.aws.amazon.com/redshift/latest/dg/r_PREPARE.html) `plan_name [ (datatype [, ...] ) ] AS statement`
- [`OPEN`](https://docs.amazonaws.cn/en_us/redshift/latest/dg/c_PLpgSQL-statements.html#r_PLpgSQL-cursors) `cursor_name FOR SELECT ...`
- [`FETCH`](https://docs.aws.amazon.com/redshift/latest/dg/fetch.html) `[ NEXT | ALL | {FORWARD [ count | ALL ] } ] FROM cursor_name`
- [`CLOSE`](https://docs.aws.amazon.com/redshift/latest/dg/close.html) `cursor_name;`

If you're using the cursor to return a result set, you can achieve similar
behavior using
[temporary tables](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_table_statement)
in BigQuery.

### Dynamic SQL statements

The
[scripting feature](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/scripting)
in BigQuery supports dynamic SQL statements like those shown in the following
table.

| **Amazon Redshift** | **BigQuery** |
|---|---|
| `https://docs.aws.amazon.com/en_us/redshift/latest/dg/c_PLpgSQL-statements.html#r_PLpgSQL-dynamic-sql` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/scripting#execute_immediate` |

### Flow-of-control statements

| **Amazon Redshift** | **BigQuery** |
|---|---|
| `https://docs.aws.amazon.com/redshift/latest/dg/c_PLpgSQL-statements.html#r_PLpgSQL-conditionals-if` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/scripting#if condition THEN stmts ELSE stmts END IF ` |
| `https://docs.aws.amazon.com/redshift/latest/dg/c_PLpgSQL-statements.html#r_PLpgSQL-cursors` | Cursors or sessions are not used in BigQuery. |
| `[<>] https://docs.aws.amazon.com/redshift/latest/dg/c_PLpgSQL-statements.html#r_PLpgSQL-loops statements END LOOP [ label ];` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/scripting#loops sql_statement_list END LOOP;` |
| `https://docs.aws.amazon.com/redshift/latest/dg/c_PLpgSQL-statements.html#r_PLpgSQL-loops` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/scripting#while condition DO stmts END WHILE` |
| `https://docs.aws.amazon.com/redshift/latest/dg/c_PLpgSQL-statements.html#r_PLpgSQL-loops` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/scripting#break` |

## Consistency guarantees and transaction isolation

Both Amazon Redshift and BigQuery are atomic---that is, ACID-compliant on a per-mutation
level across many rows.

### Transactions

Amazon Redshift supports
[serializable isolation](https://docs.aws.amazon.com/redshift/latest/dg/r_BEGIN.html#r_BEGIN-synopsis)
by default for transactions. Amazon Redshift lets you
[specify](https://docs.aws.amazon.com/redshift/latest/dg/r_BEGIN.html#r_BEGIN-parameters)
any of the four SQL standard transaction isolation levels but processes all
isolation levels as serializable.

BigQuery also
[supports transactions](https://docs.cloud.google.com/bigquery/docs/transactions).
BigQuery helps ensure
[optimistic concurrency control](https://en.wikipedia.org/wiki/Optimistic_concurrency_control)
(first to commit has priority) with
[snapshot](https://en.wikipedia.org/wiki/Snapshot_isolation)
[isolation](https://en.wikipedia.org/wiki/Snapshot_isolation),
in which a query reads the last committed data before the query starts. This
approach guarantees the same level of consistency on a per-row, per-mutation
basis and across rows within the same DML statement, yet avoids deadlocks. In
the case of multiple DML updates against the same table, BigQuery switches to
[pessimistic concurrency control](https://docs.cloud.google.com/bigquery/docs/data-manipulation-language#dml-limitations).
Load jobs can run completely independently and append to tables.

### Rollback

If Amazon Redshift encounters any error while running a stored procedure, it rolls
back all changes made in a transaction. Additionally, you can use the `ROLLBACK`
transaction control statement in a stored procedure to discard all changes.

In BigQuery, you can use the
[`ROLLBACK TRANSACTION` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/procedural-language#rollback_transaction).

## Database limits

Check the
[BigQuery public documentation](https://docs.cloud.google.com/bigquery/quotas)
for the latest quotas and limits. Many quotas for large-volume users can be
raised by contacting the Cloud support team. The following table shows a
comparison of the Amazon Redshift and BigQuery database limits.

| **Limit** | **Amazon Redshift** | **BigQuery** |
|---|---|---|
| Tables in each database for large and xlarge cluster node types | 9,900 | Unrestricted |
| Tables in each database for 8xlarge cluster node types | 20,000 | Unrestricted |
| User-defined databases you can create for each cluster | 60 | Unrestricted |
| Maximum row size | 4 MB | 100 MB |