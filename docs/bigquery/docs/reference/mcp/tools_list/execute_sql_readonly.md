## Tool: `execute_sql_readonly`

Run a read-only SQL query in the project and return the result. Prefer this tool over `execute_sql` if possible.

This tool is restricted to only `SELECT` statements. `INSERT`, `UPDATE`, and `DELETE` statements and stored procedures aren't allowed. If the query doesn't include a `SELECT` statement, an error is returned. For information on creating queries, see the [GoogleSQL documentation](https://cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax).

Queries executed using the `execute_sql_readonly` tool will have the job label `goog-mcp-server: true` automatically set. Queries are charged to the project specified in the `project_id` field.

The following sample demonstrate how to use `curl` to invoke the `execute_sql_readonly` MCP tool.

| Curl Request |
|---|
| ```bash curl --location 'https://bigquery.googleapis.com/mcp' \ --header 'content-type: application/json' \ --header 'accept: application/json, text/event-stream' \ --data '{ "method": "tools/call", "params": { "name": "execute_sql_readonly", "arguments": { // provide these details according to the tool's MCP specification } }, "jsonrpc": "2.0", "id": 1 }' ``` |

## Input Schema

Runs a BigQuery SQL query synchronously and returns query results if the query completes within a specified timeout.

### QueryRequest

| JSON representation |
|---|
| ``` { "projectId": string, "query": string, "dryRun": boolean } ``` |

| Fields ||
|---|---|
| `projectId` | `string` Required. Project that will be used for query execution and billing. |
| `query` | `string` Required. The query to execute in the form of a GoogleSQL query. |
| `dryRun` | `boolean` Optional. If set to true, BigQuery doesn't run the job. Instead, if the query is valid, BigQuery returns statistics about the job such as how many bytes would be processed. If the query is invalid, an error returns. The default value is false. |

## Output Schema

Response for a BigQuery SQL query.

### QueryResponse

| JSON representation |
|---|
| ``` { "schema": { object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_table_info#Output.Schema.TableSchema`) }, "rows": [ { object } ], "jobComplete": boolean, "errors": [ { object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_table_info#Output.Schema.ErrorProto`) } ], "queryId": string, "totalBytesBilled": string, "totalSlotMs": string, "numDmlAffectedRows": string, "totalBytesProcessed": string } ``` |

| Fields ||
|---|---|
| `schema` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_table_info#Output.Schema.TableSchema`)`` The schema of the results. Present only when the query completes successfully. |
| `rows[]` | ``object (`https://protobuf.dev/reference/protobuf/google.protobuf#struct` format)`` An object with as many results as can be contained within the maximum permitted reply size. To get any additional rows, you can call GetQueryResults and specify the jobReference returned above. |
| `jobComplete` | `boolean` Whether the query has completed or not. If rows or totalRows are present, this will always be true. If this is false, totalRows will not be available. |
| `errors[]` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_table_info#Output.Schema.ErrorProto`)`` Output only. The first errors or warnings encountered during the running of the job. The final message includes the number of errors that caused the process to stop. Errors here do not necessarily mean that the job has completed or was unsuccessful. For more information about error messages, see [Error messages](https://cloud.google.com/bigquery/docs/error-messages). |
| `queryId` | `string` Output only. The ID of the query. |
| `totalBytesBilled` | `string (https://developers.google.com/discovery/v1/type-format format)` Output only. The total number of bytes billed for the query. Only applies if the project is configured to use on-demand pricing. |
| `totalSlotMs` | `string (https://developers.google.com/discovery/v1/type-format format)` Output only. Number of slot ms the user is actually billed for. |
| `numDmlAffectedRows` | `string (https://developers.google.com/discovery/v1/type-format format)` Output only. The number of rows affected by a DML statement. |
| `totalBytesProcessed` | `string (https://developers.google.com/discovery/v1/type-format format)` Output only. The total number of bytes processed for this query. |

### TableSchema

| JSON representation |
|---|
| ``` { "fields": [ { object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_table_info#Output.Schema.TableFieldSchema`) } ], "foreignTypeInfo": { object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_table_info#Output.Schema.ForeignTypeInfo`) } } ``` |

| Fields ||
|---|---|
| `fields[]` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_table_info#Output.Schema.TableFieldSchema`)`` Describes the fields in a table. |
| `foreignTypeInfo` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_table_info#Output.Schema.ForeignTypeInfo`)`` Optional. Specifies metadata of the foreign data type definition in field schema (`TableFieldSchema.foreign_type_definition`). |

### TableFieldSchema

| JSON representation |
|---|
| ``` { "name": string, "type": string, "mode": string, "fields": [ { object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_table_info#Output.Schema.TableFieldSchema`) } ], "description": string, "policyTags": { object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_table_info#Output.Schema.PolicyTagList`) }, "dataPolicies": [ { object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_table_info#Output.Schema.DataPolicyOption`) } ], "maxLength": string, "precision": string, "scale": string, "timestampPrecision": string, "roundingMode": enum (`RoundingMode`), "collation": string, "defaultValueExpression": string, "rangeElementType": { object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_table_info#Output.Schema.FieldElementType`) }, "foreignTypeDefinition": string, "generatedColumn": { object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_table_info#Output.Schema.GeneratedColumn`) } } ``` |

| Fields ||
|---|---|
| `name` | `string` Required. The field name. The name must contain only letters (a-z, A-Z), numbers (0-9), or underscores (_), and must start with a letter or underscore. The maximum length is 300 characters. |
| `type` | `string` Required. The field data type. Possible values include: - STRING - BYTES - INTEGER (or INT64) - FLOAT (or FLOAT64) - BOOLEAN (or BOOL) - TIMESTAMP - DATE - TIME - DATETIME - GEOGRAPHY - NUMERIC - BIGNUMERIC - JSON - RECORD (or STRUCT) - RANGE Use of RECORD/STRUCT indicates that the field contains a nested schema. |
| `mode` | `string` Optional. The field mode. Possible values include NULLABLE, REQUIRED and REPEATED. The default value is NULLABLE. |
| `fields[]` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_table_info#Output.Schema.TableFieldSchema`)`` Optional. Describes the nested schema fields if the type property is set to RECORD. |
| `description` | `string` Optional. The field description. The maximum length is 1,024 characters. |
| `policyTags` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_table_info#Output.Schema.PolicyTagList`)`` Optional. The policy tags attached to this field, used for field-level access control. If not set, defaults to empty policy_tags. |
| `dataPolicies[]` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_table_info#Output.Schema.DataPolicyOption`)`` Optional. Data policies attached to this field, used for field-level access control. |
| `maxLength` | `string (https://developers.google.com/discovery/v1/type-format format)` Optional. Maximum length of values of this field for STRINGS or BYTES. If max_length is not specified, no maximum length constraint is imposed on this field. If type = "STRING", then max_length represents the maximum UTF-8 length of strings in this field. If type = "BYTES", then max_length represents the maximum number of bytes in this field. It is invalid to set this field if type ≠ "STRING" and ≠ "BYTES". |
| `precision` | `string (https://developers.google.com/discovery/v1/type-format format)` Optional. Precision (maximum number of total digits in base 10) and scale (maximum number of digits in the fractional part in base 10) constraints for values of this field for NUMERIC or BIGNUMERIC. It is invalid to set precision or scale if type ≠ "NUMERIC" and ≠ "BIGNUMERIC". If precision and scale are not specified, no value range constraint is imposed on this field insofar as values are permitted by the type. Values of this NUMERIC or BIGNUMERIC field must be in this range when: - Precision (<var translate="no">P</var>) and scale (<var translate="no">S</var>) are specified: \[-10^<var translate="no">P</var>-<var translate="no">S</var>^ + 10^-<var translate="no">S</var>^, 10^<var translate="no">P</var>-<var translate="no">S</var>^ - 10^-<var translate="no">S</var>^\] - Precision (<var translate="no">P</var>) is specified but not scale (and thus scale is interpreted to be equal to zero): \[-10^<var translate="no">P</var>^ + 1, 10^<var translate="no">P</var>^ - 1\]. Acceptable values for precision and scale if both are specified: - If type = "NUMERIC": 1 ≤ precision - scale ≤ 29 and 0 ≤ scale ≤ 9. - If type = "BIGNUMERIC": 1 ≤ precision - scale ≤ 38 and 0 ≤ scale ≤ 38. Acceptable values for precision if only precision is specified but not scale (and thus scale is interpreted to be equal to zero): - If type = "NUMERIC": 1 ≤ precision ≤ 29. - If type = "BIGNUMERIC": 1 ≤ precision ≤ 38. If scale is specified but not precision, then it is invalid. |
| `scale` | `string (https://developers.google.com/discovery/v1/type-format format)` Optional. See documentation for precision. |
| `timestampPrecision` | `string (https://developers.google.com/discovery/v1/type-format format)` Optional. Precision (maximum number of total digits in base 10) for seconds of TIMESTAMP type. Possible values include: \* 6 (Default, for TIMESTAMP type with microsecond precision) \* 12 (For TIMESTAMP type with picosecond precision) |
| `roundingMode` | ``enum (`RoundingMode`)`` Optional. Specifies the rounding mode to be used when storing values of NUMERIC and BIGNUMERIC type. |
| `collation` | `string` Optional. Field collation can be set only when the type of field is STRING. The following values are supported: - 'und:ci': undetermined locale, case insensitive. - '': empty string. Default to case-sensitive behavior. |
| `defaultValueExpression` | `string` Optional. A SQL expression to specify the [default value](https://cloud.google.com/bigquery/docs/default-values) for this field. |
| `rangeElementType` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_table_info#Output.Schema.FieldElementType`)`` Optional. The subtype of the RANGE, if the type of this field is RANGE. If the type is RANGE, this field is required. Values for the field element type can be the following: - DATE - DATETIME - TIMESTAMP |
| `foreignTypeDefinition` | `string` Optional. Definition of the foreign data type. Only valid for top-level schema fields (not nested fields). If the type is FOREIGN, this field is required. |
| `generatedColumn` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_table_info#Output.Schema.GeneratedColumn`)`` Optional. Definition of how values are generated for the field. Only valid for top-level schema fields (not nested fields). |

### StringValue

| JSON representation |
|---|
| ``` { "value": string } ``` |

| Fields ||
|---|---|
| `value` | `string` The string value. |

### PolicyTagList

| JSON representation |
|---|
| ``` { "names": [ string ] } ``` |

| Fields ||
|---|---|
| `names[]` | `string` A list of policy tag resource names. For example, "projects/1/locations/eu/taxonomies/2/policyTags/3". At most 1 policy tag is currently allowed. |

### DataPolicyOption

| JSON representation |
|---|
| ``` { // Union field `_name` can be only one of the following: "name": string // End of list of possible types for union field `_name`. } ``` |

| Fields ||
|---|---|
| Union field `_name`. `_name` can be only one of the following: ||
| `name` | `string` Data policy resource name in the form of projects/project_id/locations/location_id/dataPolicies/data_policy_id. |

### Int64Value

| JSON representation |
|---|
| ``` { "value": string } ``` |

| Fields ||
|---|---|
| `value` | `string (https://developers.google.com/discovery/v1/type-format format)` The int64 value. |

### FieldElementType

| JSON representation |
|---|
| ``` { "type": string } ``` |

| Fields ||
|---|---|
| `type` | `string` Required. The type of a field element. For more information, see `TableFieldSchema.type`. |

### GeneratedColumn

| JSON representation |
|---|
| ``` { // Union field `_generated_mode` can be only one of the following: "generatedMode": enum (`GeneratedMode`) // End of list of possible types for union field `_generated_mode`. // Union field `definition` can be only one of the following: "generatedExpressionInfo": { object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_table_info#Output.Schema.GeneratedExpressionInfo`) } // End of list of possible types for union field `definition`. } ``` |

| Fields ||
|---|---|
| Union field `_generated_mode`. `_generated_mode` can be only one of the following: ||
| `generatedMode` | ``enum (`GeneratedMode`)`` Optional. Dictates when system generated values are used to populate the field. |
| Union field `definition`. `definition` can be only one of the following: ||
| `generatedExpressionInfo` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_table_info#Output.Schema.GeneratedExpressionInfo`)`` Definition of the expression used to generate the field. |

### GeneratedExpressionInfo

| JSON representation |
|---|
| ``` { // Union field `_generation_expression` can be only one of the following: "generationExpression": string // End of list of possible types for union field `_generation_expression`. // Union field `_asynchronous` can be only one of the following: "asynchronous": boolean // End of list of possible types for union field `_asynchronous`. // Union field `_stored` can be only one of the following: "stored": boolean // End of list of possible types for union field `_stored`. } ``` |

| Fields ||
|---|---|
| Union field `_generation_expression`. `_generation_expression` can be only one of the following: ||
| `generationExpression` | `string` Optional. The generation expression (e.g. AI.EMBED(...)) used to generated the field. |
| Union field `_asynchronous`. `_asynchronous` can be only one of the following: ||
| `asynchronous` | `boolean` Optional. Whether the column generation is done asynchronously. |
| Union field `_stored`. `_stored` can be only one of the following: ||
| `stored` | `boolean` Optional. Whether the generated column is stored in the table. |

### ForeignTypeInfo

| JSON representation |
|---|
| ``` { "typeSystem": enum (`TypeSystem`) } ``` |

| Fields ||
|---|---|
| `typeSystem` | ``enum (`TypeSystem`)`` Required. Specifies the system which defines the foreign data type. |

### Struct

| JSON representation |
|---|
| ``` { "fields": { string: value, ... } } ``` |

| Fields ||
|---|---|
| `fields` | ``map (key: string, value: value (`https://protobuf.dev/reference/protobuf/google.protobuf#value` format))`` Unordered map of dynamically typed values. An object containing a list of `"key": value` pairs. Example: `{ "name": "wrench", "mass": "1.3kg", "count": "3" }`. |

### FieldsEntry

| JSON representation |
|---|
| ``` { "key": string, "value": value } ``` |

| Fields ||
|---|---|
| `key` | `string` |
| `value` | ``value (`https://protobuf.dev/reference/protobuf/google.protobuf#value` format)`` |

### Value

| JSON representation |
|---|
| ``` { // Union field `kind` can be only one of the following: "nullValue": null, "numberValue": number, "stringValue": string, "boolValue": boolean, "structValue": { object }, "listValue": array // End of list of possible types for union field `kind`. } ``` |

| Fields ||
|---|---|
| Union field `kind`. The kind of value. `kind` can be only one of the following: ||
| `nullValue` | `null` Represents a JSON `null`. |
| `numberValue` | `number` Represents a JSON number. Must not be `NaN`, `Infinity` or `-Infinity`, since those are not supported in JSON. This also cannot represent large Int64 values, since JSON format generally does not support them in its number type. |
| `stringValue` | `string` Represents a JSON string. |
| `boolValue` | `boolean` Represents a JSON boolean (`true` or `false` literal in JSON). |
| `structValue` | ``object (`https://protobuf.dev/reference/protobuf/google.protobuf#struct` format)`` Represents a JSON object. |
| `listValue` | ``array (`https://protobuf.dev/reference/protobuf/google.protobuf#list-value` format)`` Represents a JSON array. |

### ListValue

| JSON representation |
|---|
| ``` { "values": [ value ] } ``` |

| Fields ||
|---|---|
| `values[]` | ``value (`https://protobuf.dev/reference/protobuf/google.protobuf#value` format)`` Repeated field of dynamically typed values. |

### BoolValue

| JSON representation |
|---|
| ``` { "value": boolean } ``` |

| Fields ||
|---|---|
| `value` | `boolean` The bool value. |

### ErrorProto

| JSON representation |
|---|
| ``` { "reason": string, "location": string, "debugInfo": string, "message": string } ``` |

| Fields ||
|---|---|
| `reason` | `string` A short error code that summarizes the error. |
| `location` | `string` Specifies where the error occurred, if present. |
| `debugInfo` | `string` Debugging information. This property is internal to Google and should not be used. |
| `message` | `string` A human-readable description of the error. |

### Tool Annotations

Destructive Hint: ❌ \| Idempotent Hint: ✅ \| Read Only Hint: ✅ \| Open World Hint: ❌