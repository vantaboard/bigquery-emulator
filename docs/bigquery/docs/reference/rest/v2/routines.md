# REST Resource: routines

- [Resource: Routine](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/routines#Routine)
  - [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/routines#Routine.SCHEMA_REPRESENTATION)
- [RoutineReference](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/routines#RoutineReference)
  - [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/routines#RoutineReference.SCHEMA_REPRESENTATION)
- [RoutineType](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/routines#RoutineType)
- [Language](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/routines#Language)
- [Argument](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/routines#Argument)
  - [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/routines#Argument.SCHEMA_REPRESENTATION)
- [ArgumentKind](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/routines#ArgumentKind)
- [Mode](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/routines#Mode)
- [StandardSqlTableType](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/routines#StandardSqlTableType)
  - [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/routines#StandardSqlTableType.SCHEMA_REPRESENTATION)
- [DeterminismLevel](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/routines#DeterminismLevel)
- [RemoteFunctionOptions](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/routines#RemoteFunctionOptions)
  - [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/routines#RemoteFunctionOptions.SCHEMA_REPRESENTATION)
- [SparkOptions](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/routines#SparkOptions)
  - [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/routines#SparkOptions.SCHEMA_REPRESENTATION)
- [DataGovernanceType](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/routines#DataGovernanceType)
- [PythonOptions](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/routines#PythonOptions)
  - [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/routines#PythonOptions.SCHEMA_REPRESENTATION)
- [ExternalRuntimeOptions](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/routines#ExternalRuntimeOptions)
  - [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/routines#ExternalRuntimeOptions.SCHEMA_REPRESENTATION)
- [Methods](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/routines#METHODS_SUMMARY)

## Resource: Routine

A user-defined function or a stored procedure.

| JSON representation |
|---|
| ``` { "etag": string, "routineReference": { object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/routines#RoutineReference`) }, "routineType": enum (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/routines#RoutineType`), "creationTime": string, "lastModifiedTime": string, "language": enum (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/routines#Language`), "arguments": [ { object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/routines#Argument`) } ], "returnType": { object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/StandardSqlDataType`) }, "returnTableType": { object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/routines#StandardSqlTableType`) }, "importedLibraries": [ string ], "definitionBody": string, "description": string, "determinismLevel": enum (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/routines#DeterminismLevel`), "strictMode": boolean, "remoteFunctionOptions": { object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/routines#RemoteFunctionOptions`) }, "sparkOptions": { object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/routines#SparkOptions`) }, "dataGovernanceType": enum (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/routines#DataGovernanceType`), "pythonOptions": { object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/routines#PythonOptions`) }, "externalRuntimeOptions": { object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/routines#ExternalRuntimeOptions`) } } ``` |

| Fields ||
|---|---|
| `etag` | `string` Output only. A hash of this resource. |
| `routineReference` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/routines#RoutineReference`)`` Required. Reference describing the ID of this routine. |
| `routineType` | ``enum (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/routines#RoutineType`)`` Required. The type of routine. |
| `creationTime` | `string (https://developers.google.com/discovery/v1/type-format format)` Output only. The time when this routine was created, in milliseconds since the epoch. |
| `lastModifiedTime` | `string (https://developers.google.com/discovery/v1/type-format format)` Output only. The time when this routine was last modified, in milliseconds since the epoch. |
| `language` | ``enum (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/routines#Language`)`` Optional. Defaults to "SQL" if remoteFunctionOptions field is absent, not set otherwise. |
| `arguments[]` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/routines#Argument`)`` Optional. |
| `returnType` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/StandardSqlDataType`)`` Optional if language = "SQL"; required otherwise. Cannot be set if routineType = "TABLE_VALUED_FUNCTION". If absent, the return type is inferred from definitionBody at query time in each query that references this routine. If present, then the evaluated result will be cast to the specified returned type at query time. For example, for the functions created with the following statements: - `CREATE FUNCTION Add(x FLOAT64, y FLOAT64) RETURNS FLOAT64 AS (x + y);` - `CREATE FUNCTION Increment(x FLOAT64) AS (Add(x, 1));` - `CREATE FUNCTION Decrement(x FLOAT64) RETURNS FLOAT64 AS (Add(x, -1));` The returnType is `{typeKind: "FLOAT64"}` for `Add` and `Decrement`, and is absent for `Increment` (inferred as FLOAT64 at query time). Suppose the function `Add` is replaced by `CREATE OR REPLACE FUNCTION Add(x INT64, y INT64) AS (x + y);` Then the inferred return type of `Increment` is automatically changed to INT64 at query time, while the return type of `Decrement` remains FLOAT64. |
| `returnTableType` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/routines#StandardSqlTableType`)`` Optional. Can be set only if routineType = "TABLE_VALUED_FUNCTION". If absent, the return table type is inferred from definitionBody at query time in each query that references this routine. If present, then the columns in the evaluated table result will be cast to match the column types specified in return table type, at query time. |
| `importedLibraries[]` | `string` Optional. If language = "JAVASCRIPT", this field stores the path of the imported JAVASCRIPT libraries. |
| `definitionBody` | `string` Required. The body of the routine. For functions, this is the expression in the AS clause. If `language = "SQL"`, it is the substring inside (but excluding) the parentheses. For example, for the function created with the following statement: `CREATE FUNCTION JoinLines(x string, y string) as (concat(x, "\n", y))` The definitionBody is `concat(x, "\n", y)` (\\n is not replaced with linebreak). If `language="JAVASCRIPT"`, it is the evaluated string in the AS clause. For example, for the function created with the following statement: `CREATE FUNCTION f() RETURNS STRING LANGUAGE js AS 'return "\n";\n'` The definitionBody is `return "\n";\n` Note that both \\n are replaced with linebreaks. If `definitionBody` references another routine, then that routine must be fully qualified with its project ID. |
| `description` | `string` Optional. The description of the routine, if defined. |
| `determinismLevel` | ``enum (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/routines#DeterminismLevel`)`` Optional. The determinism level of the JavaScript UDF, if defined. |
| `strictMode` | `boolean` Optional. Use this option to catch many common errors. Error checking is not exhaustive, and successfully creating a procedure doesn't guarantee that the procedure will successfully execute at runtime. If `strictMode` is set to `TRUE`, the procedure body is further checked for errors such as non-existent tables or columns. The `CREATE PROCEDURE` statement fails if the body fails any of these checks. If `strictMode` is set to `FALSE`, the procedure body is checked only for syntax. For procedures that invoke themselves recursively, specify `strictMode=FALSE` to avoid non-existent procedure errors during validation. Default value is `TRUE`. |
| `remoteFunctionOptions` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/routines#RemoteFunctionOptions`)`` Optional. Remote function specific options. |
| `sparkOptions` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/routines#SparkOptions`)`` Optional. Spark specific options. |
| `dataGovernanceType` | ``enum (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/routines#DataGovernanceType`)`` Optional. If set to `DATA_MASKING`, the function is validated and made available as a masking function. For more information, see [Create custom masking routines](https://cloud.google.com/bigquery/docs/user-defined-functions#custom-mask). |
| `pythonOptions` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/routines#PythonOptions`)`` Optional. Options for the Python UDF. [Preview](https://cloud.google.com/products/#product-launch-stages) |
| `externalRuntimeOptions` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/routines#ExternalRuntimeOptions`)`` Optional. Options for the runtime of the external system executing the routine. This field is only applicable for Python UDFs. [Preview](https://cloud.google.com/products/#product-launch-stages) |

## RoutineReference

Id path of a routine.

| JSON representation |
|---|
| ``` { "projectId": string, "datasetId": string, "routineId": string } ``` |

| Fields ||
|---|---|
| `projectId` | `string` Required. The ID of the project containing this routine. |
| `datasetId` | `string` Required. The ID of the dataset containing this routine. |
| `routineId` | `string` Required. The ID of the routine. The ID must contain only letters (a-z, A-Z), numbers (0-9), or underscores (_). The maximum length is 256 characters. |

## RoutineType

The fine-grained type of the routine.

| Enums ||
|---|---|
| `ROUTINE_TYPE_UNSPECIFIED` | Default value. |
| `SCALAR_FUNCTION` | Non-built-in persistent scalar function. |
| `PROCEDURE` | Stored procedure. |
| `TABLE_VALUED_FUNCTION` | Non-built-in persistent TVF. |

## Language

The language of the routine.

| Enums ||
|---|---|
| `LANGUAGE_UNSPECIFIED` | Default value. |
| `SQL` | SQL language. |
| `JAVASCRIPT` | JavaScript language. |
| `PYTHON` | Python language. |
| `JAVA` | Java language. |
| `SCALA` | Scala language. |

## Argument

Input/output argument of a function or a stored procedure.

| JSON representation |
|---|
| ``` { "name": string, "argumentKind": enum (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/routines#ArgumentKind`), "mode": enum (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/routines#Mode`), "dataType": { object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/StandardSqlDataType`) } } ``` |

| Fields ||
|---|---|
| `name` | `string` Optional. The name of this argument. Can be absent for function return argument. |
| `argumentKind` | ``enum (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/routines#ArgumentKind`)`` Optional. Defaults to FIXED_TYPE. |
| `mode` | ``enum (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/routines#Mode`)`` Optional. Specifies whether the argument is input or output. Can be set for procedures only. |
| `dataType` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/StandardSqlDataType`)`` Set if argumentKind == FIXED_TYPE. |

## ArgumentKind

Represents the kind of a given argument.

| Enums ||
|---|---|
| `ARGUMENT_KIND_UNSPECIFIED` | Default value. |
| `FIXED_TYPE` | The argument is a variable with fully specified type, which can be a struct or an array, but not a table. |
| `ANY_TYPE` | The argument is any type, including struct or array, but not a table. |

## Mode

The input/output mode of the argument.

| Enums ||
|---|---|
| `MODE_UNSPECIFIED` | Default value. |
| `IN` | The argument is input-only. |
| `OUT` | The argument is output-only. |
| `INOUT` | The argument is both an input and an output. |

## StandardSqlTableType

A table type

| JSON representation |
|---|
| ``` { "columns": [ { object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/StandardSqlField`) } ] } ``` |

| Fields ||
|---|---|
| `columns[]` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/StandardSqlField`)`` The columns in this table type |

## DeterminismLevel

JavaScript UDF determinism levels.

If all JavaScript UDFs are DETERMINISTIC, the query result is potentially cacheable (see below). If any JavaScript UDF is NOT_DETERMINISTIC, the query result is not cacheable.

Even if a JavaScript UDF is deterministic, many other factors can prevent usage of cached query results. Example factors include but not limited to: DDL/DML, non-deterministic SQL function calls, update of referenced tables/views/UDFs or imported JavaScript libraries.

SQL UDFs cannot have determinism specified. Their determinism is automatically determined.

| Enums ||
|---|---|
| `DETERMINISM_LEVEL_UNSPECIFIED` | The determinism of the UDF is unspecified. |
| `DETERMINISTIC` | The UDF is deterministic, meaning that 2 function calls with the same inputs always produce the same result, even across 2 query runs. |
| `NOT_DETERMINISTIC` | The UDF is not deterministic. |

## RemoteFunctionOptions

Options for a remote user-defined function.

| JSON representation |
|---|
| ``` { "endpoint": string, "connection": string, "userDefinedContext": { string: string, ... }, "maxBatchingRows": string } ``` |

| Fields ||
|---|---|
| `endpoint` | `string` Endpoint of the user-provided remote service, e.g. `https://us-east1-my_gcf_project.cloudfunctions.net/remote_add` |
| `connection` | `string` Fully qualified name of the user-provided connection object which holds the authentication information to send requests to the remote service. Format: `"projects/{projectId}/locations/{locationId}/connections/{connectionId}"` |
| `userDefinedContext` | `map (key: string, value: string)` User-defined context as a set of key/value pairs, which will be sent as function invocation context together with batched arguments in the requests to the remote service. The total number of bytes of keys and values must be less than 8KB. |
| `maxBatchingRows` | `string (https://developers.google.com/discovery/v1/type-format format)` Max number of rows in each batch sent to the remote service. If absent or if 0, BigQuery dynamically decides the number of rows in a batch. |

## SparkOptions

Options for a user-defined Spark routine.

| JSON representation |
|---|
| ``` { "connection": string, "runtimeVersion": string, "containerImage": string, "properties": { string: string, ... }, "mainFileUri": string, "pyFileUris": [ string ], "jarUris": [ string ], "fileUris": [ string ], "archiveUris": [ string ], "mainClass": string } ``` |

| Fields ||
|---|---|
| `connection` | `string` Fully qualified name of the user-provided Spark connection object. Format: `"projects/{projectId}/locations/{locationId}/connections/{connectionId}"` |
| `runtimeVersion` | `string` Runtime version. If not specified, the default runtime version is used. |
| `containerImage` | `string` Custom container image for the runtime environment. |
| `properties` | `map (key: string, value: string)` Configuration properties as a set of key/value pairs, which will be passed on to the Spark application. For more information, see [Apache Spark](https://spark.apache.org/docs/latest/index.html) and the [procedure option list](https://cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#procedure_option_list). |
| `mainFileUri` | `string` The main file/jar URI of the Spark application. Exactly one of the definitionBody field and the mainFileUri field must be set for Python. Exactly one of mainClass and mainFileUri field should be set for Java/Scala language type. |
| `pyFileUris[]` | `string` Python files to be placed on the PYTHONPATH for PySpark application. Supported file types: `.py`, `.egg`, and `.zip`. For more information about Apache Spark, see [Apache Spark](https://spark.apache.org/docs/latest/index.html). |
| `jarUris[]` | `string` JARs to include on the driver and executor CLASSPATH. For more information about Apache Spark, see [Apache Spark](https://spark.apache.org/docs/latest/index.html). |
| `fileUris[]` | `string` Files to be placed in the working directory of each executor. For more information about Apache Spark, see [Apache Spark](https://spark.apache.org/docs/latest/index.html). |
| `archiveUris[]` | `string` Archive files to be extracted into the working directory of each executor. For more information about Apache Spark, see [Apache Spark](https://spark.apache.org/docs/latest/index.html). |
| `mainClass` | `string` The fully qualified name of a class in jarUris, for example, com.example.wordcount. Exactly one of mainClass and main_jar_uri field should be set for Java/Scala language type. |

## DataGovernanceType

Data governance type values. Only supports `DATA_MASKING`.

| Enums ||
|---|---|
| `DATA_GOVERNANCE_TYPE_UNSPECIFIED` | The data governance type is unspecified. |
| `DATA_MASKING` | The data governance type is data masking. |

## PythonOptions

Options for a user-defined Python function.

| JSON representation |
|---|
| ``` { "entryPoint": string, "packages": [ string ] } ``` |

| Fields ||
|---|---|
| `entryPoint` | `string` Required. The name of the function defined in Python code as the entry point when the Python UDF is invoked. |
| `packages[]` | `string` Optional. A list of Python package names along with versions to be installed. Example: \["pandas\>=2.1", "google-cloud-translate==3.11"\]. For more information, see [Use third-party packages](https://cloud.google.com/bigquery/docs/user-defined-functions-python#third-party-packages). |

## ExternalRuntimeOptions

Options for the runtime of the external system.

| JSON representation |
|---|
| ``` { "containerMemory": string, "containerCpu": number, "runtimeConnection": string, "maxBatchingRows": string, "runtimeVersion": string } ``` |

| Fields ||
|---|---|
| `containerMemory` | `string` Optional. Amount of memory provisioned for a Python UDF container instance. Format: {number}{unit} where unit is one of "M", "G", "Mi" and "Gi" (e.g. 1G, 512Mi). If not specified, the default value is 512Mi. For more information, see [Configure container limits for Python UDFs](https://cloud.google.com/bigquery/docs/user-defined-functions-python#configure-container-limits) |
| `containerCpu` | `number` Optional. Amount of CPU provisioned for a Python UDF container instance. For more information, see [Configure container limits for Python UDFs](https://cloud.google.com/bigquery/docs/user-defined-functions-python#configure-container-limits) |
| `runtimeConnection` | `string` Optional. Fully qualified name of the connection whose service account will be used to execute the code in the container. Format: `"projects/{projectId}/locations/{locationId}/connections/{connectionId}"` |
| `maxBatchingRows` | `string (https://developers.google.com/discovery/v1/type-format format)` Optional. Maximum number of rows in each batch sent to the external runtime. If absent or if 0, BigQuery dynamically decides the number of rows in a batch. |
| `runtimeVersion` | `string` Optional. Language runtime version. Example: `python-3.11`. |

| ## Methods ||
|---|---|
| ### `https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/routines/delete` | Deletes the routine specified by routineId from the dataset. |
| ### `https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/routines/get` | Gets the specified routine resource by routine ID. |
| ### `https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/routines/getIamPolicy` | Gets the access control policy for a resource. |
| ### `https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/routines/insert` | Creates a new routine in the dataset. |
| ### `https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/routines/list` | Lists all routines in the specified dataset. |
| ### `https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/routines/setIamPolicy` | Sets the access control policy on the specified resource. |
| ### `https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/routines/testIamPermissions` | Returns permissions that a caller has on the specified resource. |
| ### `https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/routines/update` | Updates information in an existing routine. |