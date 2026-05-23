# Snowflake SQL translation guide

This document details the similarities and differences in SQL syntax between
Snowflake and BigQuery to help accelerate the planning and execution of
moving your EDW (Enterprise Data Warehouse) to BigQuery. Snowflake data
warehousing is designed to work with Snowflake-specific SQL syntax. Scripts
written for Snowflake might need to be altered before you can use them in
BigQuery, because the SQL dialects vary between the services. Use
[batch SQL translation](https://docs.cloud.google.com/bigquery/docs/batch-sql-translator) to migrate your SQL
scripts in bulk, or
[interactive SQL translation](https://docs.cloud.google.com/bigquery/docs/interactive-sql-translator) to
translate ad hoc queries. Snowflake SQL is supported by both tools in
[preview](https://cloud.google.com/products#product-launch-stages).

> [!NOTE]
> **Note:** In some cases, there is no direct mapping between a SQL element in Snowflake and BigQuery. However, in most cases, you can achieve the same functionality in BigQuery that you can in Snowflake using an alternative means, as shown in the examples in this document.

## Data types

This section shows equivalents between data types in Snowflake and in
BigQuery.

<br />

| Snowflake | BigQuery | Notes |
|---|---|---|
| `https://docs.snowflake.com/en/sql-reference/data-types-numeric.html#decimal-numeric` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#decimal_types` | Can be mapped to `NUMERIC` or `BIGNUMERIC`, depending on precision and scale. The `NUMBER` data type in Snowflake supports 38 digits of precision and 37 digits of scale. Precision and scale can be specified according to the user. <br /> BigQuery supports `NUMERIC` and `BIGNUMERIC` with [optionally specified precision and scale within certain bounds](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#decimal_types). |
| [`INT/INTEGER `](https://docs.snowflake.com/en/sql-reference/data-types-numeric.html#int-integer-bigint-smallint-tinyint-byteint) | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#decimal_types` | `INT/INTEGER` and all other `INT`-like datatypes, such as `BIGINT, TINYINT, SMALLINT, BYTEINT` represent an alias for the `NUMBER` datatype where the precision and scale cannot be specified and is always `NUMBER(38, 0)` <br /> BigQuery converts `INTEGER` to `INT64` by default. To configure the SQL translation to convert it to other data types, you can use the [`REWRITE_ZERO_SCALE_NUMERIC_AS_INTEGER` configuration option](https://docs.cloud.google.com/bigquery/docs/config-yaml-translation#optimize_and_improve_the_performance_of_translated_sql) |
| `https://docs.snowflake.com/en/sql-reference/data-types-numeric.html#int-integer-bigint-smallint-tinyint-byteint` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#decimal_types` |   |
| `https://docs.snowflake.com/en/sql-reference/data-types-numeric.html#int-integer-bigint-smallint-tinyint-byteint` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#decimal_types` |   |
| `https://docs.snowflake.com/en/sql-reference/data-types-numeric.html#int-integer-bigint-smallint-tinyint-byteint` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#decimal_types` |   |
| `https://docs.snowflake.com/en/sql-reference/data-types-numeric.html#int-integer-bigint-smallint-tinyint-byteint` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#decimal_types` |   |
| `https://docs.snowflake.com/en/sql-reference/data-types-numeric.html#float-float4-float8` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#floating_point_types` | The `FLOAT` data type in Snowflake establishes 'NaN' as \> X, where X is any FLOAT value (other than 'NaN' itself). <br /> The `FLOAT` data type in BigQuery establishes 'NaN' as \< X, where X is any FLOAT value (other than 'NaN' itself). |
| `https://docs.snowflake.com/en/sql-reference/data-types-numeric.html#double-double-precision-real` `https://docs.snowflake.com/en/sql-reference/data-types-numeric.html#double-double-precision-real` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#floating_point_types` | The `DOUBLE` data type in Snowflake is synonymous with the `FLOAT` data type in Snowflake, but is commonly incorrectly displayed as `FLOAT`. It is properly stored as `DOUBLE`. |
| `https://docs.snowflake.com/en/sql-reference/data-types-text.html#varchar` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#string_type` | The `VARCHAR` data type in Snowflake has a maximum length of 128 MB (uncompressed). If length is not specified, the default is the maximum length. <br /> The `STRING` data type in BigQuery is stored as variable length UTF-8 encoded Unicode. For more information about column and row limits, see [Query jobs](https://docs.cloud.google.com/bigquery/quotas#query_jobs). |
| `https://docs.snowflake.com/en/sql-reference/data-types-text.html#char-character` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#string_type` |
| `https://docs.snowflake.com/en/sql-reference/data-types-text.html#string-text` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#string_type` | The `STRING` data type in Snowflake is synonymous with Snowflake's VARCHAR. |
| `https://docs.snowflake.com/en/sql-reference/data-types-text.html#binary` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#bytes_type` |   |
| `https://docs.snowflake.com/en/sql-reference/data-types-text.html#varbinary` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#bytes_type` |   |
| `https://docs.snowflake.com/en/sql-reference/data-types-logical.html#boolean` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#boolean_type` | The `BOOL` data type in BigQuery can only accept `TRUE/FALSE`, unlike the `BOOL` data type in Snowflake, which can accept TRUE/FALSE/NULL. |
| `https://docs.snowflake.com/en/sql-reference/data-types-datetime.html#date` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#date_type` | The `DATE` type in Snowflake accepts most common date formats, unlike the `DATE` type in BigQuery, which only accepts dates in the format, 'YYYY-\[M\]M-\[D\]D'. |
| `https://docs.snowflake.com/en/sql-reference/data-types-datetime.html#time` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#time_type` | The TIME type in Snowflake supports 0 to 9 nanoseconds of precision, whereas the TIME type in BigQuery supports 0 to 6 nanoseconds of precision. |
| `https://docs.snowflake.com/en/sql-reference/data-types-datetime.html#timestamp` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#datetime_type` | `TIMESTAMP` is a user-configurable alias which defaults to `https://docs.snowflake.com/en/sql-reference/data-types-datetime#timestamp-ltz-timestamp-ntz-timestamp-tz` which maps to `DATETIME` in BigQuery. |
| `https://docs.snowflake.com/en/sql-reference/data-types-datetime.html#timestamp-ltz-timestamp-ntz-timestamp-tz` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#timestamp_type` |   |
| `` https://docs.snowflake.com/en/sql-reference/data-types-datetime.html#timestamp-ltz-timestamp-ntz-timestamp-tz` ` `` |
| `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#datetime_type` |   |
| `https://docs.snowflake.com/en/sql-reference/data-types-datetime.html#timestamp-ltz-timestamp-ntz-timestamp-tz` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#timestamp_type` |   |
| `https://docs.snowflake.com/en/sql-reference/data-types-semistructured.html#object` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#json_type` |   |
| `https://docs.snowflake.com/en/sql-reference/data-types-semistructured#variant` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#json_type` |   |
| `https://docs.snowflake.com/en/sql-reference/data-types-semistructured.html#array` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#array_type` | The SQL translation service preserves the data type for typed arrays. For untyped arrays, such as `ARRAY<VARIANT>`, BigQuery converts these to `ARRAY<JSON>` |

BigQuery also has the following data types which do not have a direct
Snowflake analogue:

- [`DATETIME`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#datetime_type)
- [`GEOGRAPHY`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#geography_type)

## `CREATE FUNCTION` syntax

The following table addresses differences in SQL UDF creation syntax between
Snowflake and BigQuery.

| **Snowflake** | BigQuery |
|---|---|
| ` https://docs.snowflake.net/manuals/sql-reference/sql/create-function.html ` ` function_name ` ` ([sql_arg_name sql_arg_data_type[,..]]) ` ` RETURNS data_type ` ` AS sql_function_definition ` ` s ` | ` https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_function_statement function_name ` ` ([sql_arg_name sql_arg_data_type[,..]]) ` ` AS sql_function_definition ` <br /> Note: In BigQuery [SQL UDF](https://docs.cloud.google.com/bigquery/docs/user-defined-functions#sql-udf-structure), return data type is optional. BigQuery infers the result type of the function from the SQL function body when a query calls the function. |
| ` CREATE [OR REPLACE] FUNCTION ` ` function_name ` ` ([sql_arg_name sql_arg_data_type[,..]]) ` ` RETURNS TABLE (col_name, col_data_type[,..]) ` ` AS sql_function_definition ` <br /> | ` CREATE [OR REPLACE] FUNCTION function_name ` ` ([sql_arg_name sql_arg_data_type[,..]]) ` ` RETURNS data_type ` ` AS sql_function_definition ` <br /> Note:In BigQuery [SQL UDF](https://docs.cloud.google.com/bigquery/docs/user-defined-functions#sql-udf-structure), returning table type is not supported but is on the product roadmap and will be available soon. However, BigQuery supports returning ARRAY of type STRUCT. |
| ` https://docs.snowflake.net/manuals/sql-reference/udf-secure.html ` ` function_name ` ` ([sql_arg_name sql_arg_data_type[,..]]) ` ` RETURNS data_type ` ` AS sql_function_definition ` <br /> Note: Snowflake provides secure option to restrict UDF definition and details only to authorized users (that is, users who are granted the role that owns the view). | ` https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_function_statement ` ` function_name ` ` ([sql_arg_name sql_arg_data_type[,..]]) ` ` RETURNS data_type ` ` AS sql_function_definition ` <br /> Note: Function security is not a configurable parameter in BigQuery. BigQuery supports creating IAM roles and permissions to restrict access to underlying data and function definition. |
| ` CREATE [OR REPLACE] FUNCTION ` ` function_name ` ` ([sql_arg_name sql_arg_data_type[,..]]) ` ` RETURNS data_type ` ` [ { CALLED ON NULL INPUT | { RETURNS NULL ON NULL INPUT | STRICT } } ] ` ` AS sql_function_definition ` | ` CREATE [OR REPLACE] FUNCTION function_name ` ` ([sql_arg_name sql_arg_data_type[,..]]) ` ` RETURNS data_type ` ` AS sql_function_definition ` <br /> Note: Function behavior for null inputs is implicitly handled in BigQuery and need not be specified as a separate option. |
| ` CREATE [OR REPLACE] FUNCTION ` ` function_name ` ` ([sql_arg_name sql_arg_data_type[,..]]) ` ` RETURNS data_type ` ` [VOLATILE | IMMUTABLE] ` ` AS sql_function_definition ` | ` CREATE [OR REPLACE] FUNCTION ` ` function_name ` ` ([sql_arg_name sql_arg_data_type[,..]]) ` ` RETURNS data_type ` ` AS sql_function_definition ` <br /> Note:Function volatility is not a configurable parameter in BigQuery. All BigQuery UDF volatility is equivalent to Snowflake's `IMMUTABLE` volatility (that is, it does not do database lookups or otherwise use information not directly present in its argument list). |
| ` CREATE [OR REPLACE] FUNCTION ` ` function_name ` ` ([sql_arg_name sql_arg_data_type[,..]]) ` ` RETURNS data_type ` ` AS [' | $$] ` ` sql_function_definition ` ` [' | $$] ` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_function_statement` ` function_name ` ` ([sql_arg_name sql_arg_data_type[,..]]) ` ` RETURNS data_type ` ` AS sql_function_definition ` <br /> Note: Using single quotes or a character sequence like dollar quoting ($$) is not required or supported in BigQuery. BigQuery implicitly interprets the SQL expression. |
| ` CREATE [OR REPLACE] FUNCTION ` ` function_name ` ` ([sql_arg_name sql_arg_data_type[,..]]) ` ` RETURNS data_type ` ` [COMMENT = '<string_literal>'] ` ` AS sql_function_definition ` | ` CREATE [OR REPLACE] FUNCTION ` ` function_name ` ` ([sql_arg_name sql_arg_data_type[,..]]) ` ` RETURNS data_type ` ` AS sql_function_definition ` <br /> Note: Adding comments or descriptions in UDFs is not supported in BigQuery. |
| ` CREATE [OR REPLACE] FUNCTION function_name ` ` (x integer, y integer) ` ` RETURNS integer ` ` AS $$ ` ` SELECT x + y ` ` $$ ` <br /> Note: Snowflake does not support ANY TYPE for SQL UDFs. However, it supports using [VARIANT](https://docs.snowflake.net/manuals/sql-reference/data-types-semistructured.html#variant) data types. | ` CREATE [OR REPLACE] FUNCTION function_name ` ` (x ANY TYPE, y ANY TYPE) ` ` AS ` ` SELECT x + y ` <br /> <br /> Note: BigQuery supports using ANY TYPE as argument type. The function will accept an input of any type for this argument. For more information, see [templated parameter](https://docs.cloud.google.com/bigquery/docs/user-defined-functions#templated-sql-udf-parameters) in BigQuery. |

BigQuery also supports the `CREATE FUNCTION IF NOT EXISTS` statement
which treats the query as successful and takes no action if a function with the
same name already exists.

BigQuery's `CREATE FUNCTION` statement also supports creating
[`TEMPORARY or TEMP functions`](https://docs.cloud.google.com/bigquery/docs/user-defined-functions), which do
not have a Snowflake equivalent. See
[calling UDFs](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/syntax#calling_persistent_user-defined_functions_udfs)
for details on executing a BigQuery persistent UDF.

## `DROP FUNCTION` syntax

The following table addresses differences in DROP FUNCTION syntax between
Snowflake and BigQuery.

| **Snowflake** | BigQuery |
|---|---|
| ` https://docs.snowflake.net/manuals/sql-reference/sql/drop-function.html [IF EXISTS] ` ` function_name ` ` ([arg_data_type, ... ]) ` | ` https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#drop_function_statement [IF EXISTS] dataset_name.function_name ` <br /> Note: BigQuery does not require using the function's signature (argument data type) for deleting the function. |

BigQuery requires that you specify the `project_name` if the function
is not located in the current project.

## Additional function commands

This section covers additional UDF commands supported by Snowflake that are not
directly available in BigQuery.

### `ALTER FUNCTION` syntax

Snowflake supports the following operations using
[`ALTER FUNCTION`](https://docs.snowflake.net/manuals/sql-reference/sql/alter-function.html)
syntax.

- Renaming a UDF
- Converting to (or reverting from) a secure UDF
- Adding, overwriting, removing a comment for a UDF

As configuring function security and adding function comments is not available
in BigQuery, `ALTER FUNCTION` syntax is not supported. However,
the [CREATE FUNCTION](https://docs.cloud.google.com/bigquery/docs/user-defined-functions#sql-udf-structure)
statement can be used to create a UDF with the same function definition but a
different name.

### `DESCRIBE FUNCTION` syntax

Snowflake supports describing a UDF using
[DESC\[RIBE\] FUNCTION](https://docs.snowflake.net/manuals/sql-reference/sql/desc-function.html)
syntax. This is not supported in BigQuery. However, querying
UDF metadata via INFORMATION SCHEMA will be available soon as part of the
product roadmap.

### `SHOW USER FUNCTIONS` syntax

In Snowflake,
[SHOW USER FUNCTIONS](https://docs.snowflake.net/manuals/sql-reference/sql/show-user-functions.html)
syntax can be used to list all UDFs for which users have access privileges. This
is not supported in BigQuery. However, querying UDF metadata
via INFORMATION SCHEMA will be available soon as part of the product roadmap.

## Stored procedures

Snowflake
[stored procedures](https://docs.snowflake.net/manuals/sql-reference/stored-procedures-usage.html)
are written in JavaScript, which can execute SQL statements by calling a
JavaScript API. In BigQuery, stored procedures are defined using a
[block](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/scripting#begin) of SQL
statements.

### `CREATE PROCEDURE` syntax

In Snowflake, a stored procedure is executed with a
[CALL](https://docs.snowflake.net/manuals/sql-reference/sql/call.html) command
while in BigQuery, stored procedures are
[executed](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_procedure)
like any other BigQuery function.

The following table addresses differences in stored procedure creation syntax
between Snowflake and BigQuery.

| **Snowflake** | BigQuery |
|---|---|
| ` https://docs.snowflake.net/manuals/sql-reference/sql/create-procedure.html ` ` procedure_name ` ` ([arg_name arg_data_type[,..]]) ` ` RETURNS data_type ` ` AS procedure_definition; ` <br /> Note: Snowflake requires that stored procedures return a single value. Hence, return data type is a required option. | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_procedure` ` procedure_name ` ` ([arg_mode arg_name arg_data_type[,..]]) ` ` BEGIN ` ` procedure_definition ` ` END; ` <br /> ` arg_mode: IN | OUT | INOUT ` <br /> Note: BigQuery doesn't support a return type for stored procedures. Also, it requires specifying argument mode for each argument passed. |
| ` CREATE [OR REPLACE] PROCEDURE ` ` procedure_name ` ` ([arg_name arg_data_type[,..]]) ` ` RETURNS data_type ` ` AS ` ` $$ ` ` javascript_code ` ` $$; ` | ` CREATE [OR REPLACE] PROCEDURE ` ` procedure_name ` ` ([arg_name arg_data_type[,..]]) ` ` BEGIN ` ` statement_list ` ` END; ` |
| ` CREATE [OR REPLACE] PROCEDURE ` ` procedure_name ` ` ([arg_name arg_data_type[,..]]) ` ` RETURNS data_type ` ` [{CALLED ON NULL INPUT | {RETURNS NULL ON NULL INPUT | STRICT}}] ` ` AS procedure_definition; ` | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_procedure` ` procedure_name ` ` ([arg_name arg_data_type[,..]]) ` ` BEGIN ` ` procedure_definition ` ` END; ` <br /> Note: Procedure behavior for null inputs is implicitly handled in BigQuery and need not be specified as a separate option. |
| `https://docs.snowflake.net/manuals/sql-reference/sql/create-procedure.html` ` procedure_name ` ` ([arg_name arg_data_type[,..]]) ` ` RETURNS data_type ` ` [VOLATILE | IMMUTABLE] ` ` AS procedure_definition; ` | ` CREATE [OR REPLACE] PROCEDURE ` ` procedure_name ` ` ([arg_name arg_data_type[,..]]) ` ` BEGIN ` ` procedure_definition ` ` END; ` <br /> Note:Procedure volatility is not a configurable parameter in BigQuery. It's equivalent to Snowflake's `IMMUTABLE` volatility. |
| `https://docs.snowflake.net/manuals/sql-reference/sql/create-procedure.html` ` procedure_name ` ` ([arg_name arg_data_type[,..]]) ` ` RETURNS data_type ` ` [COMMENT = '<string_literal>'] ` ` AS procedure_definition; ` | ` CREATE [OR REPLACE] PROCEDURE ` ` procedure_name ` ` ([arg_name arg_data_type[,..]]) ` ` BEGIN ` ` procedure_definition ` ` END; ` <br /> Note: Adding comments or descriptions in procedure definitions is not supported in BigQuery. |
| `https://docs.snowflake.net/manuals/sql-reference/sql/create-procedure.html` ` procedure_name ` ` ([arg_name arg_data_type[,..]]) ` ` RETURNS data_type ` ` [EXECUTE AS { CALLER | OWNER }] ` ` AS procedure_definition; ` <br /> Note: Snowflake supports specifying the caller or owner of the procedure for execution | ` CREATE [OR REPLACE] PROCEDURE ` ` procedure_name ` ` ([arg_name arg_data_type[,..]]) ` ` BEGIN ` ` procedure_definition ` ` END; ` <br /> Note: BigQuery stored procedures are always executed as the caller |

BigQuery also supports the `CREATE PROCEDURE IF NOT EXISTS` statement
which treats the query as successful and takes no action if a function with the
same name already exists.

### `DROP PROCEDURE` syntax

The following table addresses differences in DROP FUNCTION syntax between
Snowflake and BigQuery.

| **Snowflake** | BigQuery |
|---|---|
| ` https://docs.snowflake.net/manuals/sql-reference/sql/drop-procedure.html [IF EXISTS] ` ` procedure_name ` ` ([arg_data_type, ... ]) ` | ` https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#drop_procedure_statement [IF EXISTS] dataset_name.procedure_name ` <br /> Note: BigQuery does not require using procedure's signature (argument data type) for deleting the procedure. |

BigQuery requires that you specify the `project_name` if the procedure
is not located in the current project.

### Additional procedure commands

Snowflake provides additional commands like
[`ALTER PROCEDURE`](https://docs.snowflake.net/manuals/sql-reference/sql/alter-procedure.html),
[`DESC[RIBE] PROCEDURE`](https://docs.snowflake.net/manuals/sql-reference/sql/desc-procedure.html),
and
[`SHOW PROCEDURES`](https://docs.snowflake.net/manuals/sql-reference/sql/show-procedures.html)
to manage the stored procedures. These are not supported in
BigQuery.

## Metadata and transaction SQL statements

| **Snowflake** | BigQuery |
|---|---|
| ` https://docs.snowflake.net/manuals/sql-reference/sql/begin.html [ { WORK | TRANSACTION } ] [ NAME <name> ]; START_TRANSACTION [ name <name> ]; ` | BigQuery always uses Snapshot Isolation. For details, see [Consistency guarantees](https://docs.cloud.google.com/bigquery/docs/migration/snowflake-sql#consistency-guarantees-and-transaction-isolation) elsewhere in this document. |
| ` COMMIT; ` | Not used in BigQuery. |
| ` ROLLBACK; ` | Not used in BigQuery |
| ` SHOW LOCKS [ IN ACCOUNT ]; SHOW TRANSACTIONS [ IN ACCOUNT ]; Note: If the user has the ACCOUNTADMIN role, the user can see locks/transactions for all users in the account. ` | Not used in BigQuery. |

## Multi-statement and multi-line SQL statements

Both Snowflake and BigQuery support transactions (sessions) and
therefore support statements separated by semicolons that are consistently
executed together. For more information, see
[Multi-statement transactions](https://docs.cloud.google.com/bigquery/docs/transactions).

## Metadata columns for staged files

Snowflake automatically generates metadata for files in internal and external
stages. This metadata can be
[queried](https://docs.snowflake.net/manuals/user-guide/querying-stage.html) and
[loaded](https://docs.snowflake.net/manuals/sql-reference/sql/copy-into-table.html)
into a table alongside regular data columns. The following metadata columns can
be utilized:

- [METADATA$FILENAME](https://docs.snowflake.net/manuals/user-guide/querying-metadata.html#metadata-columns)
- [METADATA$FILE_ROW_NUMBER](https://docs.snowflake.net/manuals/user-guide/querying-metadata.html#metadata-columns)

## Consistency guarantees and transaction isolation

Both Snowflake and BigQuery are atomic---that is, ACID-compliant on a
per-mutation level across many rows.

### Transactions

Each Snowflake transaction is assigned a unique start time (includes
milliseconds) that is set as the transaction ID. Snowflake only supports the
[`READ COMMITTED`](https://docs.snowflake.net/manuals/sql-reference/transactions.html#read-committed-isolation)
isolation level. However, a statement can see changes made by another statement
if they are both in the same transaction - even though those changes are not
committed yet. Snowflake transactions acquire locks on resources (tables) when
that resource is being modified. Users can adjust the maximum time a blocked
statement will wait until the statement times out. DML statements are
autocommitted if the
[`AUTOCOMMIT`](https://docs.snowflake.net/manuals/sql-reference/parameters.html#autocommit)
parameter is turned on.

BigQuery also
[supports transactions](https://docs.cloud.google.com/bigquery/docs/transactions). BigQuery helps
ensure
[optimistic concurrency control](https://en.wikipedia.org/wiki/Optimistic_concurrency_control)
(first to commit wins) with
[snapshot isolation](https://en.wikipedia.org/wiki/Snapshot_isolation), in which
a query reads the last committed data before the query starts. This approach
guarantees the same level of consistency on a per-row, per-mutation basis and
across rows within the same DML statement, yet avoids deadlocks. In the case of
multiple DML updates against the same table, BigQuery switches to
[pessimistic concurrency control](https://docs.cloud.google.com/bigquery/docs/data-manipulation-language#dml-limitations).
Load jobs can run completely independently and append to tables. However,
BigQuery does not provide an explicit transaction boundary or
session.

## Rollback

If a Snowflake transaction's session is unexpectedly terminated before the
transaction is committed or rolled back, the transaction is left in a detached
state. The user should run SYSTEM$ABORT_TRANSACTION to abort the detached
transaction or Snowflake will roll back the detached transaction after four idle
hours. If a deadlock occurs, Snowflake detects the deadlock and selects the more
recent statement to roll back. If the DML statement in an explicitly opened
transaction fails, the changes are rolled back, but the transaction is kept open
until it is committed or rolled back. DDL statements in Snowflake cannot be
rolled back as they are autocommitted.

BigQuery supports the
[`ROLLBACK TRANSACTION` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/procedural-language#rollback_transaction).
There is no
[`ABORT` statement](https://docs.teradata.com/reader/huc7AEHyHSROUkrYABqNIg/c6KYQ4ySu4QTCkKS4f5A2w)
in BigQuery.

## Database limits

Always check [the BigQuery public documentation](https://docs.cloud.google.com/bigquery/quotas) for
the latest quotas and limits. Many quotas for large-volume users can be raised
by contacting the Cloud Support team.

All Snowflake accounts have soft-limits set by default. Soft-limits are set
during account creation and can vary. Many Snowflake soft-limits can be raised
through the Snowflake account team or a support ticket.

The following table shows a comparison of the Snowflake and BigQuery
database limits.

| **Limit** | **Snowflake** | BigQuery |
|---|---|---|
| Size of query text | 1 MB | 1 MB |
| Maximum number of concurrent queries | XS Warehouse - 8 S Warehouse - 16 M Warehouse - 32 *L Warehouse - 64 XL Warehouse - 128* | 100 |