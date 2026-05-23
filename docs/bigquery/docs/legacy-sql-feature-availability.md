# Legacy SQL feature availability

This document describes upcoming restrictions to BigQuery legacy SQL
availability, which are based on usage during an evaluation period and take
effect after June 1, 2026. These changes are part of BigQuery's
transition away from legacy SQL to GoogleSQL, the recommended,
ANSI-compliant dialect for BigQuery.

Migrating to GoogleSQL offers these benefits over legacy SQL:

- It can be more cost-effective, using the [BigQuery advanced
  runtime](https://docs.cloud.google.com/bigquery/docs/advanced-runtime) for better performance.
- It lets you use features not supported by legacy SQL, such as [DML](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/dml-syntax) and [DDL](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language) statements, Common Table Expressions (CTEs), complex subqueries and join predicates, [materialized views](https://docs.cloud.google.com/bigquery/docs/materialized-views-intro), [search indexes](https://docs.cloud.google.com/bigquery/docs/search), and [Generative AI functions](https://docs.cloud.google.com/bigquery/docs/generative-ai-overview).

## How feature availability works

BigQuery monitors the use of legacy SQL features during an
evaluation period. For organizations and projects that don't use legacy SQL
between November 1, 2025, and June 1, 2026, legacy SQL becomes unavailable after
the evaluation period ends. For organizations and projects that use legacy SQL
during the evaluation period, you can continue to run queries using the specific
set of legacy SQL features that you use.

Feature usage is aggregated at the organization level. If any project within an
organization uses a feature, that feature remains available to all other
projects in the organization. For projects not associated with an organization,
feature availability is managed at the project level.

## Legacy SQL feature sets

Legacy SQL capabilities are organized into three feature sets: basic language
capabilities, extended language capabilities, and function groupings. The
following sections detail the features within each set.

### Basic language capabilities

These features are the core of legacy SQL. This entire feature set is available
to any organization or stand-alone project that runs at least one legacy SQL
query during the evaluation period.

| Category | Features |
|---|---|
| Query syntax | - `SELECT` - `FROM` - `JOIN` - `WHERE` - `GROUP BY` - `HAVING` - `ORDER BY` - `LIMIT` |
| Expression logic | **Literals:** - `TRUE` - `FALSE` - `NULL` <br /> **Logical operators:** - `AND` - `OR` - `NOT` <br /> **Comparison functions:** - `=` - `!=` - `<>` - `<` - `<=` - `>` - `>=` - `IN` - `IS NULL` - `IS NOT NULL` - `IS_EXPLICITLY_DEFINED` - `IS_INF` - `IS_NAN` - `... BETWEEN ... AND ...` <br /> **Control flow statements:** - `IF` - `IFNULL` - `CASE WHEN ... THEN ...` |
| Basic operations | **Arithmetic operators:** - `+` - `-` - `*` - `/` - `%` <br /> **Basic aggregate functions:** - `AVG` - `COUNT` - `FIRST` - `LAST` - `MAX` - `MIN` - `NTH` - `SUM` |
| Data elements | **Basic Data Types:** - `BYTES` - `BOOLEAN` - `FLOAT` - `INTEGER` - `STRING` - `TIMESTAMP` <br /> **Structured and partially supported data types:** - Exact Numeric: `NUMERIC`, `BIGNUMERIC` - Civil Time:`DATE`, `TIME`, `DATETIME` - Structured Fields: Nested fields, repeated fields <br /> **Casting Functions:** - `CAST(expr AS type)` - `BOOLEAN` - `BYTES` - `FLOAT` - `INTEGER` - `STRING` <br /> **Coercions:** All automatic data type coercions are included. |

### Extended language capabilities

This category includes specific legacy SQL features that go beyond the basic
set. Unlike basic capabilities or function groupings, each feature in this
category is tracked individually. You must explicitly use each feature during
the evaluation period for it to remain available.

| Category | Features |
|---|---|
| Extended features | - [Comma as `UNION ALL`](https://docs.cloud.google.com/bigquery/docs/reference/legacy-sql#comma-as-union-all) - [Explicit `FLATTEN` operator](https://docs.cloud.google.com/bigquery/docs/reference/legacy-sql#flatten-operator) - [`GROUP BY` with `EACH` modifier](https://docs.cloud.google.com/bigquery/docs/reference/legacy-sql#each) - [`IGNORE CASE` modifier](https://docs.cloud.google.com/bigquery/docs/reference/legacy-sql#stringfunctions) - [`JOIN` with `EACH` modifier](https://docs.cloud.google.com/bigquery/docs/reference/legacy-sql#each-modifier) - [Logical views](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/migrating-from-legacy-sql#logical_views) - [`OMIT ... IF` clause](https://docs.cloud.google.com/bigquery/docs/reference/legacy-sql#omit) - [Semi-join or Anti-join](https://docs.cloud.google.com/bigquery/docs/reference/legacy-sql#semi-joins) - [Table decorator - Partition](https://docs.cloud.google.com/bigquery/docs/reference/legacy-sql#from-tables) - [Table decorator - Range](https://docs.cloud.google.com/bigquery/docs/table-decorators#range_decorators) - [Table decorator - Time](https://docs.cloud.google.com/bigquery/docs/table-decorators#time_decorators) - [User-defined functions](https://docs.cloud.google.com/bigquery/docs/user-defined-functions-legacy) - [Wildcard - `TABLE_DATE_RANGE`](https://docs.cloud.google.com/bigquery/docs/reference/legacy-sql#table-date-range) - [Wildcard - `TABLE_DATE_RANGE_STRICT`](https://docs.cloud.google.com/bigquery/docs/reference/legacy-sql#table-date-range-strict) - [Wildcard - `TABLE_QUERY`](https://docs.cloud.google.com/bigquery/docs/reference/legacy-sql#table-query) - [`WITHIN` modifier for aggregate functions](https://docs.cloud.google.com/bigquery/docs/reference/legacy-sql#within) |

### Function groupings

Built-in functions are organized into related categories. Using any single
function within a grouping during the evaluation period makes all functions in
that entire grouping available.

| Function Grouping | Functions |
|---|---|
| Advanced window functions | - `CUME_DIST` - `DENSE_RANK` - `FIRST_VALUE` - `LAG` - `LAST_VALUE` - `LEAD` - `NTH_VALUE` - `NTILE` - `PERCENT_RANK` - `PERCENTILE_CONT` - `PERCENTILE_DISC` - `RANK` - `RATIO_TO_REPORT` - `ROW_NUMBER` |
| Aggregate functions for statistics | - `CORR` - `COVAR_POP` - `COVAR_SAMP` - `STDDEV` - `STDDEV_POP` - `STDDEV_SAMP` - `VARIANCE` - `VAR_POP` - `VAR_SAMP` |
| Aggregate functions returning repeated field | - `NEST` - `QUANTILES` - `UNIQUE` |
| Aggregate functions with bits operations | - `BIT_AND` - `BIT_OR` - `BIT_XOR` |
| Aggregate functions with concatenation | - `GROUP_CONCAT` - `GROUP_CONCAT_UNQUOTED` |
| Aggregate functions with sort | - `COUNT([DISTINCT])` - `EXACT_COUNT_DISTINCT` - `TOP ... COUNT(*)` |
| Basic window functions | - `AVG` - `COUNT(*)` - `COUNT([DISTINCT])` - `MAX` - `MIN` - `STDDEV` - `SUM` |
| Bitwise functions | - `&` - `|` - `^` - `<<` - `>>` - `~` - `BIT_COUNT` |
| Conditional expressions | - `COALESCE` - `EVERY` - `GREATEST` - `LEAST` - `NVL` - `SOME` |
| Conversion functions | - `FROM_BASE64` - `HEX_STRING` - `TO_BASE64` |
| Current time functions | - `NOW` - `CURRENT_DATE` - `CURRENT_TIME` - `CURRENT_TIMESTAMP` |
| Current user functions | - `CURRENT_USER` |
| Date and time functions | - `DATE` - `DATE_ADD` - `DATEDIFF` - `TIME` - `TIMESTAMP` |
| Function RAND | - `RAND` |
| Functions returning repeated field | - `POSITION` - `SPLIT` |
| Hashing functions | - `HASH` - `SHA1` - `FARM_FINGERPRINT` |
| IP functions | - `FORMAT_IP` - `FORMAT_PACKED_IP` - `PARSE_IP` - `PARSE_PACKED_IP` |
| JSON functions | - `JSON_EXTRACT` - `JSON_EXTRACT_SCALAR` |
| Mathematical functions | - `ABS` - `ACOS` - `ASIN` - `ATAN` - `ATAN2` - `CEIL` - `COS` - `DEGREES` - `EXP` - `FLOOR` - `LN` - `LOG` - `LOG10` - `LOG2` - `PI` - `POW` - `RADIANS` - `ROUND` - `SIN` - `SQRT` - `TAN` |
| Mathematical hyperbolic functions | - `ACOSH` - `ASINH` - `ATANH` - `COSH` - `SINH` - `TANH` |
| Part of TIMESTAMP functions | - `DAY` - `DAYOFWEEK` - `DAYOFYEAR` - `HOUR` - `MINUTE` - `MONTH` - `QUARTER` - `SECOND` - `WEEK` - `YEAR` |
| Regular expression functions | - `REGEXP_MATCH` - `REGEXP_EXTRACT` - `REGEXP_REPLACE` |
| String functions | - `CONTAINS` - `CONCAT` - `INSTR` - `LEFT` - `LENGTH` - `LOWER` - `LPAD` - `LTRIM` - `REPLACE` - `RIGHT` - `RPAD` - `RTRIM` - `SUBSTR` - `UPPER` |
| URL functions | - `HOST` - `DOMAIN` - `TLD` |
| UNIX timestamp functions | - `FORMAT_UTC_USEC` - `MSEC_TO_TIMESTAMP` - `PARSE_UTC_USEC` - `SEC_TO_TIMESTAMP` - `STRFTIME_UTC_USEC` - `TIMESTAMP_TO_SEC` - `TIMESTAMP_TO_MSEC` - `TIMESTAMP_TO_USEC` - `USEC_TO_TIMESTAMP` - `UTC_USEC_TO_DAY` - `UTC_USEC_TO_HOUR` - `UTC_USEC_TO_MONTH` - `UTC_USEC_TO_WEEK` - `UTC_USEC_TO_YEAR` |

## Examples of feature availability

The following examples demonstrate how feature availability works.

### Example: Accessing basic language capabilities

A project runs a legacy SQL query during the evaluation period. Assume
table `T` contains a column `X` of type `INTEGER`.

    #legacySQL
    SELECT X FROM T

This usage ensures that all projects within the organization retain the ability
to run queries that use any feature from the basic language capabilities set.
For example, the following query continues to work:

    #legacySQL
    SELECT X FROM T WHERE X > 10

### Example: Using function groupings

A project uses one function from a specific function grouping. Assume table `T`
contains a column `X` of type `FLOAT`.

    #legacySQL
    SELECT SIN(X) FROM T

The use of the `SIN()` function makes the entire mathematical functions grouping
available. Consequently, all projects within the organization can use any other
function from that grouping, such as `COS()`.

    #legacySQL
    SELECT COS(X) FROM T

Conversely, the following query fails after the evaluation period if no project
in the organization uses any function from the aggregate functions for
statistics grouping.

    #legacySQL
    SELECT STDDEV(X) FROM T

### Example: Feature retention across different tables

Assume table `X` has a column `A` (`INTEGER`) and table `Y` has column `B`
(`FLOAT`). A project runs the following query during the evaluation period:

    #legacySQL
    SELECT SIN(A) FROM X

The organization can run the following query after the evaluation period ends.
The query works because the mathematical functions feature was retained by the
first query. The retention is independent of the specific table, column name, or
data type used, as both `INTEGER` and `FLOAT` are part of the basic language
capability.

    #legacySQL
    SELECT COS(B) FROM Y

### Example: Complex query

Assume table `T` contains a column `X` of type `STRING`. A project runs the
following query during the evaluation period:

    #legacySQL
    SELECT value, AVG(FLOAT(value)) OVER (ORDER BY value) AS avg
     FROM (
      SELECT LENGTH(SPLIT(X, ',')) AS value
        FROM T
    )

This query uses features from the basic language capabilities and three function
groupings: basic window functions, string functions, and functions returning
repeated values. All projects within the organization retain these features.
Therefore, a new query that uses a different combination of functions from
those same retained feature sets succeeds.

    #legacySQL
    SELECT value, COUNT(STRING(value)) OVER (ORDER BY value) as count
     FROM (
      SELECT CONCAT(SPLIT(X, ','), '123') AS value
        FROM T
    )

## Frequently asked questions

**Can a new organization use legacy SQL?**

Following the evaluation period, legacy SQL isn't available for new
organizations or projects. In special cases, you can
[request an exemption](https://forms.gle/mSgyvY9peo4LLBj67). If you're unable to access Google Forms,
instead email [bq-legacysql-support@google.com](mailto:bq-legacysql-support@google.com) with your
organizational ID, current usage levels, recent usage date, migration
challenges, and an estimated timeline for transitioning to
GoogleSQL.

**Do existing legacy SQL queries stop working?**

Existing queries will continue to work as long as all the legacy SQL features
they use were used by at least one project in your organization during the
evaluation period. A query might fail if it relies on a feature that was not
used during this period, so we recommend that you ensure all critical queries
are run.

**Can an existing organization that uses legacy SQL create new projects that
also use it?**

Yes. All features that any project in your organization accessed during the
evaluation period remain available to all projects, old and new, in your
organization.

**Is there a tool to check which legacy SQL features my organization uses?**

There isn't a tool to audit specific feature usage. You can track legacy SQL
usage by querying `INFORMATION_SCHEMA.JOBS` views as described in
[Legacy SQL query jobs count per project](https://docs.cloud.google.com/bigquery/docs/information-schema-jobs#legacy_sql_query_jobs_count_per_project).
You can also review your query logs in Cloud Logging to check for specific
syntax usage.

**Do I have to migrate to GoogleSQL?**

Migration isn't required, but it is encouraged. GoogleSQL is the
modern, fully-featured, and recommended dialect.

**What if a rarely used legacy SQL query does not run during the evaluation
period?**

To ensure that a query continues to work, run it once during the evaluation
period. If you're unable to run it then, you can [request an
exemption](https://forms.gle/mSgyvY9peo4LLBj67). If you're unable to access Google Forms, instead
email [bq-legacysql-support@google.com](mailto:bq-legacysql-support@google.com)
with your organizational ID, current usage levels, recent usage date, migration
challenges, and an estimated timeline for transitioning to
GoogleSQL.

## What's next

- To migrate your queries from legacy SQL to GoogleSQL, see the [migration guide](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/migrating-from-legacy-sql).