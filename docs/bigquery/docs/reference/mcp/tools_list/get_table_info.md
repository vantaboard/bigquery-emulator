## Tool: `get_table_info`

Get metadata information about a BigQuery table.

The following sample demonstrate how to use `curl` to invoke the `get_table_info` MCP tool.

| Curl Request |
|---|
| ```bash curl --location 'https://bigquery.googleapis.com/mcp' \ --header 'content-type: application/json' \ --header 'accept: application/json, text/event-stream' \ --data '{ "method": "tools/call", "params": { "name": "get_table_info", "arguments": { // provide these details according to the tool's MCP specification } }, "jsonrpc": "2.0", "id": 1 }' ``` |

## Input Schema

Request for a table.

### GetTableRequest

| JSON representation |
|---|
| ``` { "projectId": string, "datasetId": string, "tableId": string } ``` |

| Fields ||
|---|---|
| `projectId` | `string` Required. Project ID of the table request. |
| `datasetId` | `string` Required. Dataset ID of the table request. |
| `tableId` | `string` Required. Table ID of the table request. |

## Output Schema

### Table

| JSON representation |
|---|
| ``` { "kind": string, "etag": string, "id": string, "selfLink": string, "tableReference": { object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_dataset_info#Output.Schema.TableReference`) }, "friendlyName": string, "description": string, "labels": { string: string, ... }, "schema": { object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_table_info#Output.Schema.TableSchema`) }, "timePartitioning": { object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_table_info#Output.Schema.TimePartitioning`) }, "rangePartitioning": { object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_table_info#Output.Schema.RangePartitioning`) }, "clustering": { object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_table_info#Output.Schema.Clustering`) }, "requirePartitionFilter": boolean, "numBytes": string, "numPhysicalBytes": string, "numLongTermBytes": string, "numRows": string, "creationTime": string, "expirationTime": string, "lastModifiedTime": string, "type": string, "view": { object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_table_info#Output.Schema.ViewDefinition`) }, "materializedView": { object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_table_info#Output.Schema.MaterializedViewDefinition`) }, "materializedViewStatus": { object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_table_info#Output.Schema.MaterializedViewStatus`) }, "externalDataConfiguration": { object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_table_info#Output.Schema.ExternalDataConfiguration`) }, "biglakeConfiguration": { object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_table_info#Output.Schema.BigLakeConfiguration`) }, "managedTableType": enum (`ManagedTableType`), "location": string, "streamingBuffer": { object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_table_info#Output.Schema.Streamingbuffer`) }, "encryptionConfiguration": { object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_dataset_info#Output.Schema.EncryptionConfiguration`) }, "snapshotDefinition": { object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_table_info#Output.Schema.SnapshotDefinition`) }, "defaultCollation": string, "defaultRoundingMode": enum (`RoundingMode`), "cloneDefinition": { object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_table_info#Output.Schema.CloneDefinition`) }, "numTimeTravelPhysicalBytes": string, "numTotalLogicalBytes": string, "numActiveLogicalBytes": string, "numLongTermLogicalBytes": string, "numCurrentPhysicalBytes": string, "numTotalPhysicalBytes": string, "numActivePhysicalBytes": string, "numLongTermPhysicalBytes": string, "numPartitions": string, "maxStaleness": string, "restrictions": { object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_dataset_info#Output.Schema.RestrictionConfig`) }, "tableConstraints": { object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_table_info#Output.Schema.TableConstraints`) }, "resourceTags": { string: string, ... }, "tableReplicationInfo": { object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_table_info#Output.Schema.TableReplicationInfo`) }, "replicas": [ { object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_dataset_info#Output.Schema.TableReference`) } ], "externalCatalogTableOptions": { object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_table_info#Output.Schema.ExternalCatalogTableOptions`) }, // Union field `_partition_definition` can be only one of the following: "partitionDefinition": { object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_table_info#Output.Schema.PartitioningDefinition`) } // End of list of possible types for union field `_partition_definition`. } ``` |

| Fields ||
|---|---|
| `kind` | `string` The type of resource ID. |
| `etag` | `string` Output only. A hash of this resource. |
| `id` | `string` Output only. An opaque ID uniquely identifying the table. |
| `selfLink` | `string` Output only. A URL that can be used to access this resource again. |
| `tableReference` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_dataset_info#Output.Schema.TableReference`)`` Required. Reference describing the ID of this table. |
| `friendlyName` | `string` Optional. A descriptive name for this table. |
| `description` | `string` Optional. A user-friendly description of this table. |
| `labels` | `map (key: string, value: string)` The labels associated with this table. You can use these to organize and group your tables. Label keys and values can be no longer than 63 characters, can only contain lowercase letters, numeric characters, underscores and dashes. International characters are allowed. Label values are optional. Label keys must start with a letter and each label in the list must have a different key. An object containing a list of `"key": value` pairs. Example: `{ "name": "wrench", "mass": "1.3kg", "count": "3" }`. |
| `schema` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_table_info#Output.Schema.TableSchema`)`` Optional. Describes the schema of this table. |
| `timePartitioning` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_table_info#Output.Schema.TimePartitioning`)`` If specified, configures time-based partitioning for this table. |
| `rangePartitioning` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_table_info#Output.Schema.RangePartitioning`)`` If specified, configures range partitioning for this table. |
| `clustering` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_table_info#Output.Schema.Clustering`)`` Clustering specification for the table. Must be specified with time-based partitioning, data in the table will be first partitioned and subsequently clustered. |
| `requirePartitionFilter` | `boolean` Optional. If set to true, queries over this table require a partition filter that can be used for partition elimination to be specified. |
| `numBytes` | `string (https://developers.google.com/discovery/v1/type-format format)` Output only. The size of this table in logical bytes, excluding any data in the streaming buffer. |
| `numPhysicalBytes` | `string (https://developers.google.com/discovery/v1/type-format format)` Output only. The physical size of this table in bytes. This includes storage used for time travel. |
| `numLongTermBytes` | `string (https://developers.google.com/discovery/v1/type-format format)` Output only. The number of logical bytes in the table that are considered "long-term storage". |
| `numRows` | `string (https://developers.google.com/discovery/v1/type-format format)` Output only. The number of rows of data in this table, excluding any data in the streaming buffer. |
| `creationTime` | `string (https://developers.google.com/discovery/v1/type-format format)` Output only. The time when this table was created, in milliseconds since the epoch. |
| `expirationTime` | `string (https://developers.google.com/discovery/v1/type-format format)` Optional. The time when this table expires, in milliseconds since the epoch. If not present, the table will persist indefinitely. Expired tables will be deleted and their storage reclaimed. The defaultTableExpirationMs property of the encapsulating dataset can be used to set a default expirationTime on newly created tables. |
| `lastModifiedTime` | `string (https://developers.google.com/discovery/v1/type-format format)` Output only. The time when this table was last modified, in milliseconds since the epoch. |
| `type` | `string` Output only. Describes the table type. The following values are supported: - `TABLE`: A normal BigQuery table. - `VIEW`: A virtual table defined by a SQL query. - `EXTERNAL`: A table that references data stored in an external storage system, such as Google Cloud Storage. - `MATERIALIZED_VIEW`: A precomputed view defined by a SQL query. - `SNAPSHOT`: An immutable BigQuery table that preserves the contents of a base table at a particular time. See additional information on [table snapshots](https://cloud.google.com/bigquery/docs/table-snapshots-intro). The default value is `TABLE`. |
| `view` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_table_info#Output.Schema.ViewDefinition`)`` Optional. The view definition. |
| `materializedView` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_table_info#Output.Schema.MaterializedViewDefinition`)`` Optional. The materialized view definition. |
| `materializedViewStatus` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_table_info#Output.Schema.MaterializedViewStatus`)`` Output only. The materialized view status. |
| `externalDataConfiguration` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_table_info#Output.Schema.ExternalDataConfiguration`)`` Optional. Describes the data format, location, and other properties of a table stored outside of BigQuery. By defining these properties, the data source can then be queried as if it were a standard BigQuery table. |
| `biglakeConfiguration` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_table_info#Output.Schema.BigLakeConfiguration`)`` Optional. Specifies the configuration of a BigQuery table for Apache Iceberg. |
| `managedTableType` | ``enum (`ManagedTableType`)`` Optional. If set, overrides the default managed table type configured in the dataset. |
| `location` | `string` Output only. The geographic location where the table resides. This value is inherited from the dataset. |
| `streamingBuffer` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_table_info#Output.Schema.Streamingbuffer`)`` Output only. Contains information regarding this table's streaming buffer, if one is present. This field will be absent if the table is not being streamed to or if there is no data in the streaming buffer. |
| `encryptionConfiguration` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_dataset_info#Output.Schema.EncryptionConfiguration`)`` Custom encryption configuration (e.g., Cloud KMS keys). |
| `snapshotDefinition` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_table_info#Output.Schema.SnapshotDefinition`)`` Output only. Contains information about the snapshot. This value is set via snapshot creation. |
| `defaultCollation` | `string` Optional. Defines the default collation specification of new STRING fields in the table. During table creation or update, if a STRING field is added to this table without explicit collation specified, then the table inherits the table default collation. A change to this field affects only fields added afterwards, and does not alter the existing fields. The following values are supported: - 'und:ci': undetermined locale, case insensitive. - '': empty string. Default to case-sensitive behavior. |
| `defaultRoundingMode` | ``enum (`RoundingMode`)`` Optional. Defines the default rounding mode specification of new decimal fields (NUMERIC OR BIGNUMERIC) in the table. During table creation or update, if a decimal field is added to this table without an explicit rounding mode specified, then the field inherits the table default rounding mode. Changing this field doesn't affect existing fields. |
| `cloneDefinition` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_table_info#Output.Schema.CloneDefinition`)`` Output only. Contains information about the clone. This value is set via the clone operation. |
| `numTimeTravelPhysicalBytes` | `string (https://developers.google.com/discovery/v1/type-format format)` Output only. Number of physical bytes used by time travel storage (deleted or changed data). This data is not kept in real time, and might be delayed by a few seconds to a few minutes. |
| `numTotalLogicalBytes` | `string (https://developers.google.com/discovery/v1/type-format format)` Output only. Total number of logical bytes in the table or materialized view. |
| `numActiveLogicalBytes` | `string (https://developers.google.com/discovery/v1/type-format format)` Output only. Number of logical bytes that are less than 90 days old. |
| `numLongTermLogicalBytes` | `string (https://developers.google.com/discovery/v1/type-format format)` Output only. Number of logical bytes that are more than 90 days old. |
| `numCurrentPhysicalBytes` | `string (https://developers.google.com/discovery/v1/type-format format)` Output only. Number of physical bytes used by current live data storage. This data is not kept in real time, and might be delayed by a few seconds to a few minutes. |
| `numTotalPhysicalBytes` | `string (https://developers.google.com/discovery/v1/type-format format)` Output only. The physical size of this table in bytes. This also includes storage used for time travel. This data is not kept in real time, and might be delayed by a few seconds to a few minutes. |
| `numActivePhysicalBytes` | `string (https://developers.google.com/discovery/v1/type-format format)` Output only. Number of physical bytes less than 90 days old. This data is not kept in real time, and might be delayed by a few seconds to a few minutes. |
| `numLongTermPhysicalBytes` | `string (https://developers.google.com/discovery/v1/type-format format)` Output only. Number of physical bytes more than 90 days old. This data is not kept in real time, and might be delayed by a few seconds to a few minutes. |
| `numPartitions` | `string (https://developers.google.com/discovery/v1/type-format format)` Output only. The number of partitions present in the table or materialized view. This data is not kept in real time, and might be delayed by a few seconds to a few minutes. |
| `maxStaleness` | `string` Optional. The maximum staleness of data that could be returned when the table (or stale MV) is queried. Staleness encoded as a string encoding of sql IntervalValue type. |
| `restrictions` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_dataset_info#Output.Schema.RestrictionConfig`)`` Optional. Output only. Restriction config for table. If set, restrict certain accesses on the table based on the config. See [Data egress](https://cloud.google.com/bigquery/docs/analytics-hub-introduction#data_egress) for more details. |
| `tableConstraints` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_table_info#Output.Schema.TableConstraints`)`` Optional. Tables Primary Key and Foreign Key information |
| `resourceTags` | `map (key: string, value: string)` Optional. The [tags](https://cloud.google.com/bigquery/docs/tags) attached to this table. Tag keys are globally unique. Tag key is expected to be in the namespaced format, for example "123456789012/environment" where 123456789012 is the ID of the parent organization or project resource for this tag key. Tag value is expected to be the short name, for example "Production". See [Tag definitions](https://cloud.google.com/iam/docs/tags-access-control#definitions) for more details. An object containing a list of `"key": value` pairs. Example: `{ "name": "wrench", "mass": "1.3kg", "count": "3" }`. |
| `tableReplicationInfo` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_table_info#Output.Schema.TableReplicationInfo`)`` Optional. Table replication info for table created `AS REPLICA` DDL like: `CREATE MATERIALIZED VIEW mv1 AS REPLICA OF src_mv` |
| `replicas[]` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_dataset_info#Output.Schema.TableReference`)`` Optional. Output only. Table references of all replicas currently active on the table. |
| `externalCatalogTableOptions` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_table_info#Output.Schema.ExternalCatalogTableOptions`)`` Optional. Options defining open source compatible table. |
| Union field `_partition_definition`. `_partition_definition` can be only one of the following: ||
| `partitionDefinition` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_table_info#Output.Schema.PartitioningDefinition`)`` Optional. The partition information for all table formats, including managed partitioned tables, hive partitioned tables, iceberg partitioned, and metastore partitioned tables. This field is only populated for metastore partitioned tables. For other table formats, this is an output only field. |

### TableReference

| JSON representation |
|---|
| ``` { "projectId": string, "datasetId": string, "tableId": string } ``` |

| Fields ||
|---|---|
| `projectId` | `string` Required. The ID of the project containing this table. |
| `datasetId` | `string` Required. The ID of the dataset containing this table. |
| `tableId` | `string` Required. The ID of the table. The ID can contain Unicode characters in category L (letter), M (mark), N (number), Pc (connector, including underscore), Pd (dash), and Zs (space). For more information, see [General Category](https://wikipedia.org/wiki/Unicode_character_property#General_Category). The maximum length is 1,024 characters. Certain operations allow suffixing of the table ID with a partition decorator, such as `sample_table$20190123`. |

### StringValue

| JSON representation |
|---|
| ``` { "value": string } ``` |

| Fields ||
|---|---|
| `value` | `string` The string value. |

### LabelsEntry

| JSON representation |
|---|
| ``` { "key": string, "value": string } ``` |

| Fields ||
|---|---|
| `key` | `string` |
| `value` | `string` |

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

### TimePartitioning

| JSON representation |
|---|
| ``` { "type": string, "expirationMs": string, "field": string, "requirePartitionFilter": boolean } ``` |

| Fields ||
|---|---|
| `type` | `string` Required. The supported types are DAY, HOUR, MONTH, and YEAR, which will generate one partition per day, hour, month, and year, respectively. |
| `expirationMs` | `string (https://developers.google.com/discovery/v1/type-format format)` Optional. Number of milliseconds for which to keep the storage for a partition. A wrapper is used here because 0 is an invalid value. |
| `field` | `string` Optional. If not set, the table is partitioned by pseudo column '_PARTITIONTIME'; if set, the table is partitioned by this field. The field must be a top-level TIMESTAMP or DATE field. Its mode must be NULLABLE or REQUIRED. A wrapper is used here because an empty string is an invalid value. |
| `requirePartitionFilter (deprecated)` | `boolean` > [!WARNING] > This item is deprecated! If set to true, queries over this table require a partition filter that can be used for partition elimination to be specified. This field is deprecated; please set the field with the same name on the table itself instead. This field needs a wrapper because we want to output the default value, false, if the user explicitly set it. |

### BoolValue

| JSON representation |
|---|
| ``` { "value": boolean } ``` |

| Fields ||
|---|---|
| `value` | `boolean` The bool value. |

### RangePartitioning

| JSON representation |
|---|
| ``` { "field": string, "range": { object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_table_info#Output.Schema.Range`) } } ``` |

| Fields ||
|---|---|
| `field` | `string` Required. The name of the column to partition the table on. It must be a top-level, INT64 column whose mode is NULLABLE or REQUIRED. |
| `range` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_table_info#Output.Schema.Range`)`` Defines the ranges for range partitioning. |

### Range

| JSON representation |
|---|
| ``` { "start": string, "end": string, "interval": string } ``` |

| Fields ||
|---|---|
| `start` | `string` Required. The start of range partitioning, inclusive. This field is an INT64 value represented as a string. |
| `end` | `string` Required. The end of range partitioning, exclusive. This field is an INT64 value represented as a string. |
| `interval` | `string` Required. The width of each interval. This field is an INT64 value represented as a string. |

### Clustering

| JSON representation |
|---|
| ``` { "fields": [ string ] } ``` |

| Fields ||
|---|---|
| `fields[]` | `string` One or more fields on which data should be clustered. Only top-level, non-repeated, simple-type fields are supported. The ordering of the clustering fields should be prioritized from most to least important for filtering purposes. For additional information, see [Introduction to clustered tables](https://cloud.google.com/bigquery/docs/clustered-tables#limitations). |

### PartitioningDefinition

| JSON representation |
|---|
| ``` { "partitionedColumn": [ { object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_table_info#Output.Schema.PartitionedColumn`) } ] } ``` |

| Fields ||
|---|---|
| `partitionedColumn[]` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_table_info#Output.Schema.PartitionedColumn`)`` Optional. Details about each partitioning column. This field is output only for all partitioning types other than metastore partitioned tables. BigQuery native tables only support 1 partitioning column. Other table types may support 0, 1 or more partitioning columns. For metastore partitioned tables, the order must match the definition order in the Hive Metastore, where it must match the physical layout of the table. For example, CREATE TABLE a_table(id BIGINT, name STRING) PARTITIONED BY (city STRING, state STRING). In this case the values must be \['city', 'state'\] in that order. |

### PartitionedColumn

| JSON representation |
|---|
| ``` { // Union field `_field` can be only one of the following: "field": string // End of list of possible types for union field `_field`. } ``` |

| Fields ||
|---|---|
| Union field `_field`. `_field` can be only one of the following: ||
| `field` | `string` Required. The name of the partition column. |

### UInt64Value

| JSON representation |
|---|
| ``` { "value": string } ``` |

| Fields ||
|---|---|
| `value` | `string` The uint64 value. |

### ViewDefinition

| JSON representation |
|---|
| ``` { "query": string, "userDefinedFunctionResources": [ { object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_table_info#Output.Schema.UserDefinedFunctionResource`) } ], "useLegacySql": boolean, "useExplicitColumnNames": boolean, "privacyPolicy": { object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_table_info#Output.Schema.PrivacyPolicy`) }, "foreignDefinitions": [ { object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_table_info#Output.Schema.ForeignViewDefinition`) } ] } ``` |

| Fields ||
|---|---|
| `query` | `string` Required. A query that BigQuery executes when the view is referenced. |
| `userDefinedFunctionResources[]` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_table_info#Output.Schema.UserDefinedFunctionResource`)`` Describes user-defined function resources used in the query. |
| `useLegacySql` | `boolean` Specifies whether to use BigQuery's legacy SQL for this view. The default value is true. If set to false, the view uses BigQuery's [GoogleSQL](https://docs.cloud.google.com/bigquery/docs/introduction-sql). Queries and views that reference this view must use the same flag value. A wrapper is used here because the default value is True. |
| `useExplicitColumnNames` | `boolean` True if the column names are explicitly specified. For example by using the 'CREATE VIEW v(c1, c2) AS ...' syntax. Can only be set for GoogleSQL views. |
| `privacyPolicy` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_table_info#Output.Schema.PrivacyPolicy`)`` Optional. Specifies the privacy policy for the view. |
| `foreignDefinitions[]` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_table_info#Output.Schema.ForeignViewDefinition`)`` Optional. Foreign view representations. |

### UserDefinedFunctionResource

| JSON representation |
|---|
| ``` { "resourceUri": string, "inlineCode": string } ``` |

| Fields ||
|---|---|
| `resourceUri` | `string` \[Pick one\] A code resource to load from a Google Cloud Storage URI (gs://bucket/path). |
| `inlineCode` | `string` \[Pick one\] An inline resource that contains code for a user-defined function (UDF). Providing a inline code resource is equivalent to providing a URI for a file containing the same code. |

### PrivacyPolicy

| JSON representation |
|---|
| ``` { // Union field `privacy_policy` can be only one of the following: "aggregationThresholdPolicy": { object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_table_info#Output.Schema.AggregationThresholdPolicy`) }, "differentialPrivacyPolicy": { object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_table_info#Output.Schema.DifferentialPrivacyPolicy`) } // End of list of possible types for union field `privacy_policy`. // Union field `_join_restriction_policy` can be only one of the following: "joinRestrictionPolicy": { object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_table_info#Output.Schema.JoinRestrictionPolicy`) } // End of list of possible types for union field `_join_restriction_policy`. } ``` |

| Fields ||
|---|---|
| Union field `privacy_policy`. Privacy policy associated with this requirement specification. Only one of the privacy methods is allowed per data source object. `privacy_policy` can be only one of the following: ||
| `aggregationThresholdPolicy` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_table_info#Output.Schema.AggregationThresholdPolicy`)`` Optional. Policy used for aggregation thresholds. |
| `differentialPrivacyPolicy` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_table_info#Output.Schema.DifferentialPrivacyPolicy`)`` Optional. Policy used for differential privacy. |
| Union field `_join_restriction_policy`. `_join_restriction_policy` can be only one of the following: ||
| `joinRestrictionPolicy` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_table_info#Output.Schema.JoinRestrictionPolicy`)`` Optional. Join restriction policy is outside of the one of policies, since this policy can be set along with other policies. This policy gives data providers the ability to enforce joins on the 'join_allowed_columns' when data is queried from a privacy protected view. |

### AggregationThresholdPolicy

| JSON representation |
|---|
| ``` { "privacyUnitColumns": [ string ], // Union field `_threshold` can be only one of the following: "threshold": string // End of list of possible types for union field `_threshold`. } ``` |

| Fields ||
|---|---|
| `privacyUnitColumns[]` | `string` Optional. The privacy unit column(s) associated with this policy. For now, only one column per data source object (table, view) is allowed as a privacy unit column. Representing as a repeated field in metadata for extensibility to multiple columns in future. Duplicates and Repeated struct fields are not allowed. For nested fields, use dot notation ("outer.inner") |
| Union field `_threshold`. `_threshold` can be only one of the following: ||
| `threshold` | `string (https://developers.google.com/discovery/v1/type-format format)` Optional. The threshold for the "aggregation threshold" policy. |

### DifferentialPrivacyPolicy

| JSON representation |
|---|
| ``` { // Union field `_max_epsilon_per_query` can be only one of the following: "maxEpsilonPerQuery": number // End of list of possible types for union field `_max_epsilon_per_query`. // Union field `_delta_per_query` can be only one of the following: "deltaPerQuery": number // End of list of possible types for union field `_delta_per_query`. // Union field `_max_groups_contributed` can be only one of the following: "maxGroupsContributed": string // End of list of possible types for union field `_max_groups_contributed`. // Union field `_privacy_unit_column` can be only one of the following: "privacyUnitColumn": string // End of list of possible types for union field `_privacy_unit_column`. // Union field `_epsilon_budget` can be only one of the following: "epsilonBudget": number // End of list of possible types for union field `_epsilon_budget`. // Union field `_delta_budget` can be only one of the following: "deltaBudget": number // End of list of possible types for union field `_delta_budget`. // Union field `_epsilon_budget_remaining` can be only one of the following: "epsilonBudgetRemaining": number // End of list of possible types for union field `_epsilon_budget_remaining`. // Union field `_delta_budget_remaining` can be only one of the following: "deltaBudgetRemaining": number // End of list of possible types for union field `_delta_budget_remaining`. } ``` |

| Fields ||
|---|---|
| Union field `_max_epsilon_per_query`. `_max_epsilon_per_query` can be only one of the following: ||
| `maxEpsilonPerQuery` | `number` Optional. The maximum epsilon value that a query can consume. If the subscriber specifies epsilon as a parameter in a SELECT query, it must be less than or equal to this value. The epsilon parameter controls the amount of noise that is added to the groups --- a higher epsilon means less noise. |
| Union field `_delta_per_query`. `_delta_per_query` can be only one of the following: ||
| `deltaPerQuery` | `number` Optional. The delta value that is used per query. Delta represents the probability that any row will fail to be epsilon differentially private. Indicates the risk associated with exposing aggregate rows in the result of a query. |
| Union field `_max_groups_contributed`. `_max_groups_contributed` can be only one of the following: ||
| `maxGroupsContributed` | `string (https://developers.google.com/discovery/v1/type-format format)` Optional. The maximum groups contributed value that is used per query. Represents the maximum number of groups to which each protected entity can contribute. Changing this value does not improve or worsen privacy. The best value for accuracy and utility depends on the query and data. |
| Union field `_privacy_unit_column`. `_privacy_unit_column` can be only one of the following: ||
| `privacyUnitColumn` | `string` Optional. The privacy unit column associated with this policy. Differential privacy policies can only have one privacy unit column per data source object (table, view). |
| Union field `_epsilon_budget`. `_epsilon_budget` can be only one of the following: ||
| `epsilonBudget` | `number` Optional. The total epsilon budget for all queries against the privacy-protected view. Each subscriber query against this view charges the amount of epsilon they request in their query. If there is sufficient budget, then the subscriber query attempts to complete. It might still fail due to other reasons, in which case the charge is refunded. If there is insufficient budget the query is rejected. There might be multiple charge attempts if a single query references multiple views. In this case there must be sufficient budget for all charges or the query is rejected and charges are refunded in best effort. The budget does not have a refresh policy and can only be updated via ALTER VIEW or circumvented by creating a new view that can be queried with a fresh budget. |
| Union field `_delta_budget`. `_delta_budget` can be only one of the following: ||
| `deltaBudget` | `number` Optional. The total delta budget for all queries against the privacy-protected view. Each subscriber query against this view charges the amount of delta that is pre-defined by the contributor through the privacy policy delta_per_query field. If there is sufficient budget, then the subscriber query attempts to complete. It might still fail due to other reasons, in which case the charge is refunded. If there is insufficient budget the query is rejected. There might be multiple charge attempts if a single query references multiple views. In this case there must be sufficient budget for all charges or the query is rejected and charges are refunded in best effort. The budget does not have a refresh policy and can only be updated via ALTER VIEW or circumvented by creating a new view that can be queried with a fresh budget. |
| Union field `_epsilon_budget_remaining`. `_epsilon_budget_remaining` can be only one of the following: ||
| `epsilonBudgetRemaining` | `number` Output only. The epsilon budget remaining. If budget is exhausted, no more queries are allowed. Note that the budget for queries that are in progress is deducted before the query executes. If the query fails or is cancelled then the budget is refunded. In this case the amount of budget remaining can increase. |
| Union field `_delta_budget_remaining`. `_delta_budget_remaining` can be only one of the following: ||
| `deltaBudgetRemaining` | `number` Output only. The delta budget remaining. If budget is exhausted, no more queries are allowed. Note that the budget for queries that are in progress is deducted before the query executes. If the query fails or is cancelled then the budget is refunded. In this case the amount of budget remaining can increase. |

### JoinRestrictionPolicy

| JSON representation |
|---|
| ``` { "joinAllowedColumns": [ string ], // Union field `_join_condition` can be only one of the following: "joinCondition": enum (`JoinCondition`) // End of list of possible types for union field `_join_condition`. } ``` |

| Fields ||
|---|---|
| `joinAllowedColumns[]` | `string` Optional. The only columns that joins are allowed on. This field is must be specified for join_conditions JOIN_ANY and JOIN_ALL and it cannot be set for JOIN_BLOCKED. |
| Union field `_join_condition`. `_join_condition` can be only one of the following: ||
| `joinCondition` | ``enum (`JoinCondition`)`` Optional. Specifies if a join is required or not on queries for the view. Default is JOIN_CONDITION_UNSPECIFIED. |

### ForeignViewDefinition

| JSON representation |
|---|
| ``` { "query": string, "dialect": string } ``` |

| Fields ||
|---|---|
| `query` | `string` Required. The query that defines the view. |
| `dialect` | `string` Optional. Represents the dialect of the query. |

### MaterializedViewDefinition

| JSON representation |
|---|
| ``` { "query": string, "lastRefreshTime": string, "enableRefresh": boolean, "refreshIntervalMs": string, "allowNonIncrementalDefinition": boolean } ``` |

| Fields ||
|---|---|
| `query` | `string` Required. A query whose results are persisted. |
| `lastRefreshTime` | `string (https://developers.google.com/discovery/v1/type-format format)` Output only. The time when this materialized view was last refreshed, in milliseconds since the epoch. |
| `enableRefresh` | `boolean` Optional. Enable automatic refresh of the materialized view when the base table is updated. The default value is "true". |
| `refreshIntervalMs` | `string (https://developers.google.com/discovery/v1/type-format format)` Optional. The maximum frequency at which this materialized view will be refreshed. The default value is "1800000" (30 minutes). |
| `allowNonIncrementalDefinition` | `boolean` Optional. This option declares the intention to construct a materialized view that isn't refreshed incrementally. Non-incremental materialized views support an expanded range of SQL queries. The `allow_non_incremental_definition` option can't be changed after the materialized view is created. |

### MaterializedViewStatus

| JSON representation |
|---|
| ``` { "refreshWatermark": string, "lastRefreshStatus": { object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_table_info#Output.Schema.ErrorProto`) } } ``` |

| Fields ||
|---|---|
| `refreshWatermark` | ``string (`https://protobuf.dev/reference/protobuf/google.protobuf#timestamp` format)`` Output only. Refresh watermark of materialized view. The base tables' data were collected into the materialized view cache until this time. Uses RFC 3339, where generated output will always be Z-normalized and use 0, 3, 6 or 9 fractional digits. Offsets other than "Z" are also accepted. Examples: `"2014-10-02T15:01:23Z"`, `"2014-10-02T15:01:23.045123456Z"` or `"2014-10-02T15:01:23+05:30"`. |
| `lastRefreshStatus` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_table_info#Output.Schema.ErrorProto`)`` Output only. Error result of the last automatic refresh. If present, indicates that the last automatic refresh was unsuccessful. |

### Timestamp

| JSON representation |
|---|
| ``` { "seconds": string, "nanos": integer } ``` |

| Fields ||
|---|---|
| `seconds` | `string (https://developers.google.com/discovery/v1/type-format format)` Represents seconds of UTC time since Unix epoch 1970-01-01T00:00:00Z. Must be between -62135596800 and 253402300799 inclusive (which corresponds to 0001-01-01T00:00:00Z to 9999-12-31T23:59:59Z). |
| `nanos` | `integer` Non-negative fractions of a second at nanosecond resolution. This field is the nanosecond portion of the duration, not an alternative to seconds. Negative second values with fractions must still have non-negative nanos values that count forward in time. Must be between 0 and 999,999,999 inclusive. |

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

### ExternalDataConfiguration

| JSON representation |
|---|
| ``` { "sourceUris": [ string ], "fileSetSpecType": enum (`FileSetSpecType`), "schema": { object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_table_info#Output.Schema.TableSchema`) }, "sourceFormat": string, "maxBadRecords": integer, "autodetect": boolean, "ignoreUnknownValues": boolean, "compression": string, "csvOptions": { object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_table_info#Output.Schema.CsvOptions`) }, "jsonOptions": { object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_table_info#Output.Schema.JsonOptions`) }, "bigtableOptions": { object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_table_info#Output.Schema.BigtableOptions`) }, "googleSheetsOptions": { object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_table_info#Output.Schema.GoogleSheetsOptions`) }, "hivePartitioningOptions": { object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_table_info#Output.Schema.HivePartitioningOptions`) }, "connectionId": string, "decimalTargetTypes": [ enum (`DecimalTargetType`) ], "avroOptions": { object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_table_info#Output.Schema.AvroOptions`) }, "jsonExtension": enum (`JsonExtension`), "parquetOptions": { object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_table_info#Output.Schema.ParquetOptions`) }, "referenceFileSchemaUri": string, "metadataCacheMode": enum (`MetadataCacheMode`), "timestampTargetPrecision": [ integer ], // Union field `_object_metadata` can be only one of the following: "objectMetadata": enum (`ObjectMetadata`) // End of list of possible types for union field `_object_metadata`. // Union field `_time_zone` can be only one of the following: "timeZone": string // End of list of possible types for union field `_time_zone`. // Union field `_date_format` can be only one of the following: "dateFormat": string // End of list of possible types for union field `_date_format`. // Union field `_datetime_format` can be only one of the following: "datetimeFormat": string // End of list of possible types for union field `_datetime_format`. // Union field `_time_format` can be only one of the following: "timeFormat": string // End of list of possible types for union field `_time_format`. // Union field `_timestamp_format` can be only one of the following: "timestampFormat": string // End of list of possible types for union field `_timestamp_format`. } ``` |

| Fields ||
|---|---|
| `sourceUris[]` | `string` \[Required\] The fully-qualified URIs that point to your data in Google Cloud. For Google Cloud Storage URIs: Each URI can contain one '\*' wildcard character and it must come after the 'bucket' name. Size limits related to load jobs apply to external data sources. For Google Cloud Bigtable URIs: Exactly one URI can be specified and it has be a fully specified and valid HTTPS URL for a Google Cloud Bigtable table. For Google Cloud Datastore backups, exactly one URI can be specified. Also, the '\*' wildcard character is not allowed. |
| `fileSetSpecType` | ``enum (`FileSetSpecType`)`` Optional. Specifies how source URIs are interpreted for constructing the file set to load. By default source URIs are expanded against the underlying storage. Other options include specifying manifest files. Only applicable to object storage systems. |
| `schema` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_table_info#Output.Schema.TableSchema`)`` Optional. The schema for the data. Schema is required for CSV and JSON formats if autodetect is not on. Schema is disallowed for Google Cloud Bigtable, Cloud Datastore backups, Avro, ORC and Parquet formats. |
| `sourceFormat` | `string` \[Required\] The data format. For CSV files, specify "CSV". For Google sheets, specify "GOOGLE_SHEETS". For newline-delimited JSON, specify "NEWLINE_DELIMITED_JSON". For Avro files, specify "AVRO". For Google Cloud Datastore backups, specify "DATASTORE_BACKUP". For Apache Iceberg tables, specify "ICEBERG". For ORC files, specify "ORC". For Parquet files, specify "PARQUET". \[Beta\] For Google Cloud Bigtable, specify "BIGTABLE". |
| `maxBadRecords` | `integer` Optional. The maximum number of bad records that BigQuery can ignore when reading data. If the number of bad records exceeds this value, an invalid error is returned in the job result. The default value is 0, which requires that all records are valid. This setting is ignored for Google Cloud Bigtable, Google Cloud Datastore backups, Avro, ORC and Parquet formats. |
| `autodetect` | `boolean` Try to detect schema and format options automatically. Any option specified explicitly will be honored. |
| `ignoreUnknownValues` | `boolean` Optional. Indicates if BigQuery should allow extra values that are not represented in the table schema. If true, the extra values are ignored. If false, records with extra columns are treated as bad records, and if there are too many bad records, an invalid error is returned in the job result. The default value is false. The sourceFormat property determines what BigQuery treats as an extra value: CSV: Trailing columns JSON: Named values that don't match any column names Google Cloud Bigtable: This setting is ignored. Google Cloud Datastore backups: This setting is ignored. Avro: This setting is ignored. ORC: This setting is ignored. Parquet: This setting is ignored. |
| `compression` | `string` Optional. The compression type of the data source. Possible values include GZIP and NONE. The default value is NONE. This setting is ignored for Google Cloud Bigtable, Google Cloud Datastore backups, Avro, ORC and Parquet formats. An empty string is an invalid value. |
| `csvOptions` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_table_info#Output.Schema.CsvOptions`)`` Optional. Additional properties to set if sourceFormat is set to CSV. |
| `jsonOptions` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_table_info#Output.Schema.JsonOptions`)`` Optional. Additional properties to set if sourceFormat is set to JSON. |
| `bigtableOptions` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_table_info#Output.Schema.BigtableOptions`)`` Optional. Additional options if sourceFormat is set to BIGTABLE. |
| `googleSheetsOptions` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_table_info#Output.Schema.GoogleSheetsOptions`)`` Optional. Additional options if sourceFormat is set to GOOGLE_SHEETS. |
| `hivePartitioningOptions` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_table_info#Output.Schema.HivePartitioningOptions`)`` Optional. When set, configures hive partitioning support. Not all storage formats support hive partitioning -- requesting hive partitioning on an unsupported format will lead to an error, as will providing an invalid specification. |
| `connectionId` | `string` Optional. The connection specifying the credentials to be used to read external storage, such as Azure Blob, Cloud Storage, or S3. The connection_id can have the form `{project_id}.{location_id};{connection_id}` or `projects/{project_id}/locations/{location_id}/connections/{connection_id}`. |
| `decimalTargetTypes[]` | ``enum (`DecimalTargetType`)`` Defines the list of possible SQL data types to which the source decimal values are converted. This list and the precision and the scale parameters of the decimal field determine the target type. In the order of NUMERIC, BIGNUMERIC, and STRING, a type is picked if it is in the specified list and if it supports the precision and the scale. STRING supports all precision and scale values. If none of the listed types supports the precision and the scale, the type supporting the widest range in the specified list is picked, and if a value exceeds the supported range when reading the data, an error will be thrown. Example: Suppose the value of this field is \["NUMERIC", "BIGNUMERIC"\]. If (precision,scale) is: - (38,9) -\> NUMERIC; - (39,9) -\> BIGNUMERIC (NUMERIC cannot hold 30 integer digits); - (38,10) -\> BIGNUMERIC (NUMERIC cannot hold 10 fractional digits); - (76,38) -\> BIGNUMERIC; - (77,38) -\> BIGNUMERIC (error if value exceeds supported range). This field cannot contain duplicate types. The order of the types in this field is ignored. For example, \["BIGNUMERIC", "NUMERIC"\] is the same as \["NUMERIC", "BIGNUMERIC"\] and NUMERIC always takes precedence over BIGNUMERIC. Defaults to \["NUMERIC", "STRING"\] for ORC and \["NUMERIC"\] for the other file formats. |
| `avroOptions` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_table_info#Output.Schema.AvroOptions`)`` Optional. Additional properties to set if sourceFormat is set to AVRO. |
| `jsonExtension` | ``enum (`JsonExtension`)`` Optional. Load option to be used together with source_format newline-delimited JSON to indicate that a variant of JSON is being loaded. To load newline-delimited GeoJSON, specify GEOJSON (and source_format must be set to NEWLINE_DELIMITED_JSON). |
| `parquetOptions` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_table_info#Output.Schema.ParquetOptions`)`` Optional. Additional properties to set if sourceFormat is set to PARQUET. |
| `referenceFileSchemaUri` | `string` Optional. When creating an external table, the user can provide a reference file with the table schema. This is enabled for the following formats: AVRO, PARQUET, ORC. |
| `metadataCacheMode` | ``enum (`MetadataCacheMode`)`` Optional. Metadata Cache Mode for the table. Set this to enable caching of metadata from external data source. |
| `timestampTargetPrecision[]` | `integer` Precisions (maximum number of total digits in base 10) for seconds of TIMESTAMP types that are allowed to the destination table for autodetection mode. Available for the formats: CSV, PARQUET, and AVRO. Possible values include: Not Specified, \[\], or \[6\]: timestamp(6) for all auto detected TIMESTAMP columns \[6, 12\]: timestamp(6) for all auto detected TIMESTAMP columns that have less than 6 digits of subseconds. timestamp(12) for all auto detected TIMESTAMP columns that have more than 6 digits of subseconds. \[12\]: timestamp(12) for all auto detected TIMESTAMP columns. The order of the elements in this array is ignored. Inputs that have higher precision than the highest target precision in this array will be truncated. |
| Union field `_object_metadata`. `_object_metadata` can be only one of the following: ||
| `objectMetadata` | ``enum (`ObjectMetadata`)`` Optional. ObjectMetadata is used to create Object Tables. Object Tables contain a listing of objects (with their metadata) found at the source_uris. If ObjectMetadata is set, source_format should be omitted. Currently SIMPLE is the only supported Object Metadata type. |
| Union field `_time_zone`. `_time_zone` can be only one of the following: ||
| `timeZone` | `string` Optional. Time zone used when parsing timestamp values that do not have specific time zone information (e.g. 2024-04-20 12:34:56). The expected format is a IANA timezone string (e.g. America/Los_Angeles). |
| Union field `_date_format`. `_date_format` can be only one of the following: ||
| `dateFormat` | `string` Optional. Format used to parse DATE values. Supports C-style and SQL-style values. |
| Union field `_datetime_format`. `_datetime_format` can be only one of the following: ||
| `datetimeFormat` | `string` Optional. Format used to parse DATETIME values. Supports C-style and SQL-style values. |
| Union field `_time_format`. `_time_format` can be only one of the following: ||
| `timeFormat` | `string` Optional. Format used to parse TIME values. Supports C-style and SQL-style values. |
| Union field `_timestamp_format`. `_timestamp_format` can be only one of the following: ||
| `timestampFormat` | `string` Optional. Format used to parse TIMESTAMP values. Supports C-style and SQL-style values. |

### Int32Value

| JSON representation |
|---|
| ``` { "value": integer } ``` |

| Fields ||
|---|---|
| `value` | `integer` The int32 value. |

### CsvOptions

| JSON representation |
|---|
| ``` { "fieldDelimiter": string, "skipLeadingRows": string, "quote": string, "allowQuotedNewlines": boolean, "allowJaggedRows": boolean, "encoding": string, "preserveAsciiControlCharacters": boolean, "nullMarker": string, "nullMarkers": [ string ], "sourceColumnMatch": string } ``` |

| Fields ||
|---|---|
| `fieldDelimiter` | `string` Optional. The separator character for fields in a CSV file. The separator is interpreted as a single byte. For files encoded in ISO-8859-1, any single character can be used as a separator. For files encoded in UTF-8, characters represented in decimal range 1-127 (U+0001-U+007F) can be used without any modification. UTF-8 characters encoded with multiple bytes (i.e. U+0080 and above) will have only the first byte used for separating fields. The remaining bytes will be treated as a part of the field. BigQuery also supports the escape sequence "\\t" (U+0009) to specify a tab separator. The default value is comma (",", U+002C). |
| `skipLeadingRows` | `string (https://developers.google.com/discovery/v1/type-format format)` Optional. The number of rows at the top of a CSV file that BigQuery will skip when reading the data. The default value is 0. This property is useful if you have header rows in the file that should be skipped. When autodetect is on, the behavior is the following: - skipLeadingRows unspecified - Autodetect tries to detect headers in the first row. If they are not detected, the row is read as data. Otherwise data is read starting from the second row. - skipLeadingRows is 0 - Instructs autodetect that there are no headers and data should be read starting from the first row. - skipLeadingRows = N \> 0 - Autodetect skips N-1 rows and tries to detect headers in row N. If headers are not detected, row N is just skipped. Otherwise row N is used to extract column names for the detected schema. |
| `quote` | `string` Optional. The value that is used to quote data sections in a CSV file. BigQuery converts the string to ISO-8859-1 encoding, and then uses the first byte of the encoded string to split the data in its raw, binary state. The default value is a double-quote ("). If your data does not contain quoted sections, set the property value to an empty string. If your data contains quoted newline characters, you must also set the allowQuotedNewlines property to true. To include the specific quote character within a quoted value, precede it with an additional matching quote character. For example, if you want to escape the default character ' " ', use ' "" '. |
| `allowQuotedNewlines` | `boolean` Optional. Indicates if BigQuery should allow quoted data sections that contain newline characters in a CSV file. The default value is false. |
| `allowJaggedRows` | `boolean` Optional. Indicates if BigQuery should accept rows that are missing trailing optional columns. If true, BigQuery treats missing trailing columns as null values. If false, records with missing trailing columns are treated as bad records, and if there are too many bad records, an invalid error is returned in the job result. The default value is false. |
| `encoding` | `string` Optional. The character encoding of the data. The supported values are UTF-8, ISO-8859-1, UTF-16BE, UTF-16LE, UTF-32BE, and UTF-32LE. The default value is UTF-8. BigQuery decodes the data after the raw, binary data has been split using the values of the quote and fieldDelimiter properties. |
| `preserveAsciiControlCharacters` | `boolean` Optional. Indicates if the embedded ASCII control characters (the first 32 characters in the ASCII-table, from '\\x00' to '\\x1F') are preserved. |
| `nullMarker` | `string` Optional. Specifies a string that represents a null value in a CSV file. For example, if you specify "\\N", BigQuery interprets "\\N" as a null value when querying a CSV file. The default value is the empty string. If you set this property to a custom value, BigQuery throws an error if an empty string is present for all data types except for STRING and BYTE. For STRING and BYTE columns, BigQuery interprets the empty string as an empty value. |
| `nullMarkers[]` | `string` Optional. A list of strings represented as SQL NULL value in a CSV file. null_marker and null_markers can't be set at the same time. If null_marker is set, null_markers has to be not set. If null_markers is set, null_marker has to be not set. If both null_marker and null_markers are set at the same time, a user error would be thrown. Any strings listed in null_markers, including empty string would be interpreted as SQL NULL. This applies to all column types. |
| `sourceColumnMatch` | `string` Optional. Controls the strategy used to match loaded columns to the schema. If not set, a sensible default is chosen based on how the schema is provided. If autodetect is used, then columns are matched by name. Otherwise, columns are matched by position. This is done to keep the behavior backward-compatible. Acceptable values are: POSITION - matches by position. This assumes that the columns are ordered the same way as the schema. NAME - matches by name. This reads the header row as column names and reorders columns to match the field names in the schema. |

### JsonOptions

| JSON representation |
|---|
| ``` { "encoding": string } ``` |

| Fields ||
|---|---|
| `encoding` | `string` Optional. The character encoding of the data. The supported values are UTF-8, UTF-16BE, UTF-16LE, UTF-32BE, and UTF-32LE. The default value is UTF-8. |

### BigtableOptions

| JSON representation |
|---|
| ``` { "columnFamilies": [ { object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_table_info#Output.Schema.BigtableColumnFamily`) } ], "ignoreUnspecifiedColumnFamilies": boolean, "readRowkeyAsString": boolean, "outputColumnFamiliesAsJson": boolean } ``` |

| Fields ||
|---|---|
| `columnFamilies[]` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_table_info#Output.Schema.BigtableColumnFamily`)`` Optional. List of column families to expose in the table schema along with their types. This list restricts the column families that can be referenced in queries and specifies their value types. You can use this list to do type conversions - see the 'type' field for more details. If you leave this list empty, all column families are present in the table schema and their values are read as BYTES. During a query only the column families referenced in that query are read from Bigtable. |
| `ignoreUnspecifiedColumnFamilies` | `boolean` Optional. If field is true, then the column families that are not specified in columnFamilies list are not exposed in the table schema. Otherwise, they are read with BYTES type values. The default value is false. |
| `readRowkeyAsString` | `boolean` Optional. If field is true, then the rowkey column families will be read and converted to string. Otherwise they are read with BYTES type values and users need to manually cast them with CAST if necessary. The default value is false. |
| `outputColumnFamiliesAsJson` | `boolean` Optional. If field is true, then each column family will be read as a single JSON column. Otherwise they are read as a repeated cell structure containing timestamp/value tuples. The default value is false. |

### BigtableColumnFamily

| JSON representation |
|---|
| ``` { "familyId": string, "type": string, "encoding": string, "columns": [ { object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_table_info#Output.Schema.BigtableColumn`) } ], "onlyReadLatest": boolean, "protoConfig": { object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_table_info#Output.Schema.BigtableProtoConfig`) } } ``` |

| Fields ||
|---|---|
| `familyId` | `string` Identifier of the column family. |
| `type` | `string` Optional. The type to convert the value in cells of this column family. The values are expected to be encoded using HBase Bytes.toBytes function when using the BINARY encoding value. Following BigQuery types are allowed (case-sensitive): - BYTES - STRING - INTEGER - FLOAT - BOOLEAN - JSON Default type is BYTES. This can be overridden for a specific column by listing that column in 'columns' and specifying a type for it. |
| `encoding` | `string` Optional. The encoding of the values when the type is not STRING. Acceptable encoding values are: TEXT - indicates values are alphanumeric text strings. BINARY - indicates values are encoded using HBase Bytes.toBytes family of functions. PROTO_BINARY - indicates values are encoded using serialized proto messages. This can only be used in combination with JSON type. This can be overridden for a specific column by listing that column in 'columns' and specifying an encoding for it. |
| `columns[]` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_table_info#Output.Schema.BigtableColumn`)`` Optional. Lists of columns that should be exposed as individual fields as opposed to a list of (column name, value) pairs. All columns whose qualifier matches a qualifier in this list can be accessed as `<family field name>.<column field name>`. Other columns can be accessed as a list through the `<family field name>.Column` field. |
| `onlyReadLatest` | `boolean` Optional. If this is set only the latest version of value are exposed for all columns in this column family. This can be overridden for a specific column by listing that column in 'columns' and specifying a different setting for that column. |
| `protoConfig` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_table_info#Output.Schema.BigtableProtoConfig`)`` Optional. Protobuf-specific configurations, only takes effect when the encoding is PROTO_BINARY. |

### BigtableColumn

| JSON representation |
|---|
| ``` { "qualifierEncoded": string, "qualifierString": string, "fieldName": string, "type": string, "encoding": string, "onlyReadLatest": boolean, "protoConfig": { object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_table_info#Output.Schema.BigtableProtoConfig`) } } ``` |

| Fields ||
|---|---|
| `qualifierEncoded` | `string (https://developers.google.com/discovery/v1/type-format format)` \[Required\] Qualifier of the column. Columns in the parent column family that has this exact qualifier are exposed as `<family field name>.<column field name>` field. If the qualifier is valid UTF-8 string, it can be specified in the qualifier_string field. Otherwise, a base-64 encoded value must be set to qualifier_encoded. The column field name is the same as the column qualifier. However, if the qualifier is not a valid BigQuery field identifier i.e. does not match \[a-zA-Z\]\[a-zA-Z0-9_\]\*, a valid identifier must be provided as field_name. |
| `qualifierString` | `string` Qualifier string. |
| `fieldName` | `string` Optional. If the qualifier is not a valid BigQuery field identifier i.e. does not match \[a-zA-Z\]\[a-zA-Z0-9_\]\*, a valid identifier must be provided as the column field name and is used as field name in queries. |
| `type` | `string` Optional. The type to convert the value in cells of this column. The values are expected to be encoded using HBase Bytes.toBytes function when using the BINARY encoding value. Following BigQuery types are allowed (case-sensitive): - BYTES - STRING - INTEGER - FLOAT - BOOLEAN - JSON Default type is BYTES. 'type' can also be set at the column family level. However, the setting at this level takes precedence if 'type' is set at both levels. |
| `encoding` | `string` Optional. The encoding of the values when the type is not STRING. Acceptable encoding values are: TEXT - indicates values are alphanumeric text strings. BINARY - indicates values are encoded using HBase Bytes.toBytes family of functions. PROTO_BINARY - indicates values are encoded using serialized proto messages. This can only be used in combination with JSON type. 'encoding' can also be set at the column family level. However, the setting at this level takes precedence if 'encoding' is set at both levels. |
| `onlyReadLatest` | `boolean` Optional. If this is set, only the latest version of value in this column are exposed. 'onlyReadLatest' can also be set at the column family level. However, the setting at this level takes precedence if 'onlyReadLatest' is set at both levels. |
| `protoConfig` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_table_info#Output.Schema.BigtableProtoConfig`)`` Optional. Protobuf-specific configurations, only takes effect when the encoding is PROTO_BINARY. |

### BytesValue

| JSON representation |
|---|
| ``` { "value": string } ``` |

| Fields ||
|---|---|
| `value` | `string (https://developers.google.com/discovery/v1/type-format format)` The bytes value. A base64-encoded string. |

### BigtableProtoConfig

| JSON representation |
|---|
| ``` { "schemaBundleId": string, "protoMessageName": string } ``` |

| Fields ||
|---|---|
| `schemaBundleId` | `string` Optional. The ID of the Bigtable SchemaBundle resource associated with this protobuf. The ID should be referred to within the parent table, e.g., `foo` rather than `projects/{project}/instances/{instance}/tables/{table}/schemaBundles/foo`. See [more details on Bigtable SchemaBundles](https://docs.cloud.google.com/bigtable/docs/create-manage-protobuf-schemas). |
| `protoMessageName` | `string` Optional. The fully qualified proto message name of the protobuf. In the format of "foo.bar.Message". |

### GoogleSheetsOptions

| JSON representation |
|---|
| ``` { "skipLeadingRows": string, "range": string } ``` |

| Fields ||
|---|---|
| `skipLeadingRows` | `string (https://developers.google.com/discovery/v1/type-format format)` Optional. The number of rows at the top of a sheet that BigQuery will skip when reading the data. The default value is 0. This property is useful if you have header rows that should be skipped. When autodetect is on, the behavior is the following: \* skipLeadingRows unspecified - Autodetect tries to detect headers in the first row. If they are not detected, the row is read as data. Otherwise data is read starting from the second row. \* skipLeadingRows is 0 - Instructs autodetect that there are no headers and data should be read starting from the first row. \* skipLeadingRows = N \> 0 - Autodetect skips N-1 rows and tries to detect headers in row N. If headers are not detected, row N is just skipped. Otherwise row N is used to extract column names for the detected schema. |
| `range` | `string` Optional. Range of a sheet to query from. Only used when non-empty. Typical format: sheet_name!top_left_cell_id:bottom_right_cell_id For example: sheet1!A1:B20 |

### HivePartitioningOptions

| JSON representation |
|---|
| ``` { "mode": string, "sourceUriPrefix": string, "requirePartitionFilter": boolean, "fields": [ string ] } ``` |

| Fields ||
|---|---|
| `mode` | `string` Optional. When set, what mode of hive partitioning to use when reading data. The following modes are supported: - AUTO: automatically infer partition key name(s) and type(s). - STRINGS: automatically infer partition key name(s). All types are strings. - CUSTOM: partition key schema is encoded in the source URI prefix. Not all storage formats support hive partitioning. Requesting hive partitioning on an unsupported format will lead to an error. Currently supported formats are: JSON, CSV, ORC, Avro and Parquet. |
| `sourceUriPrefix` | `string` Optional. When hive partition detection is requested, a common prefix for all source uris must be required. The prefix must end immediately before the partition key encoding begins. For example, consider files following this data layout: gs://bucket/path_to_table/dt=2019-06-01/country=USA/id=7/file.avro gs://bucket/path_to_table/dt=2019-05-31/country=CA/id=3/file.avro When hive partitioning is requested with either AUTO or STRINGS detection, the common prefix can be either of gs://bucket/path_to_table or gs://bucket/path_to_table/. CUSTOM detection requires encoding the partitioning schema immediately after the common prefix. For CUSTOM, any of - gs://bucket/path_to_table/{dt:DATE}/{country:STRING}/{id:INTEGER} - gs://bucket/path_to_table/{dt:STRING}/{country:STRING}/{id:INTEGER} - gs://bucket/path_to_table/{dt:DATE}/{country:STRING}/{id:STRING} would all be valid source URI prefixes. |
| `requirePartitionFilter` | `boolean` Optional. If set to true, queries over this table require a partition filter that can be used for partition elimination to be specified. Note that this field should only be true when creating a permanent external table or querying a temporary external table. Hive-partitioned loads with require_partition_filter explicitly set to true will fail. |
| `fields[]` | `string` Output only. For permanent external tables, this field is populated with the hive partition keys in the order they were inferred. The types of the partition keys can be deduced by checking the table schema (which will include the partition keys). Not every API will populate this field in the output. For example, Tables.Get will populate it, but Tables.List will not contain this field. |

### AvroOptions

| JSON representation |
|---|
| ``` { "useAvroLogicalTypes": boolean } ``` |

| Fields ||
|---|---|
| `useAvroLogicalTypes` | `boolean` Optional. If sourceFormat is set to "AVRO", indicates whether to interpret logical types as the corresponding BigQuery data type (for example, TIMESTAMP), instead of using the raw type (for example, INTEGER). |

### ParquetOptions

| JSON representation |
|---|
| ``` { "enumAsString": boolean, "enableListInference": boolean, "mapTargetType": enum (`MapTargetType`) } ``` |

| Fields ||
|---|---|
| `enumAsString` | `boolean` Optional. Indicates whether to infer Parquet ENUM logical type as STRING instead of BYTES by default. |
| `enableListInference` | `boolean` Optional. Indicates whether to use schema inference specifically for Parquet LIST logical type. |
| `mapTargetType` | ``enum (`MapTargetType`)`` Optional. Indicates how to represent a Parquet map if present. |

### BigLakeConfiguration

| JSON representation |
|---|
| ``` { "connectionId": string, "storageUri": string, "fileFormat": enum (`FileFormat`), "tableFormat": enum (`TableFormat`) } ``` |

| Fields ||
|---|---|
| `connectionId` | `string` Optional. The connection specifying the credentials to be used to read and write to external storage, such as Cloud Storage. The connection_id can have the form `{project}.{location}.{connection_id}` or \`projects/{project}/locations/{location}/connections/{connection_id}". |
| `storageUri` | `string` Optional. The fully qualified location prefix of the external folder where table data is stored. The '\*' wildcard character is not allowed. The URI should be in the format `gs://bucket/path_to_table/` |
| `fileFormat` | ``enum (`FileFormat`)`` Optional. The file format the table data is stored in. |
| `tableFormat` | ``enum (`TableFormat`)`` Optional. The table format the metadata only snapshots are stored in. |

### Streamingbuffer

| JSON representation |
|---|
| ``` { "estimatedBytes": string, "estimatedRows": string, "oldestEntryTime": string } ``` |

| Fields ||
|---|---|
| `estimatedBytes` | `string` Output only. A lower-bound estimate of the number of bytes currently in the streaming buffer. |
| `estimatedRows` | `string` Output only. A lower-bound estimate of the number of rows currently in the streaming buffer. |
| `oldestEntryTime` | `string (https://developers.google.com/discovery/v1/type-format format)` Output only. Contains the timestamp of the oldest entry in the streaming buffer, in milliseconds since the epoch, if the streaming buffer is available. |

### EncryptionConfiguration

| JSON representation |
|---|
| ``` { "kmsKeyName": string } ``` |

| Fields ||
|---|---|
| `kmsKeyName` | `string` Optional. Describes the Cloud KMS encryption key that will be used to protect destination BigQuery table. The BigQuery Service Account associated with your project requires access to this encryption key. |

### SnapshotDefinition

| JSON representation |
|---|
| ``` { "baseTableReference": { object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_dataset_info#Output.Schema.TableReference`) }, "snapshotTime": string } ``` |

| Fields ||
|---|---|
| `baseTableReference` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_dataset_info#Output.Schema.TableReference`)`` Required. Reference describing the ID of the table that was snapshot. |
| `snapshotTime` | ``string (`https://protobuf.dev/reference/protobuf/google.protobuf#timestamp` format)`` Required. The time at which the base table was snapshot. This value is reported in the JSON response using RFC3339 format. Uses RFC 3339, where generated output will always be Z-normalized and use 0, 3, 6 or 9 fractional digits. Offsets other than "Z" are also accepted. Examples: `"2014-10-02T15:01:23Z"`, `"2014-10-02T15:01:23.045123456Z"` or `"2014-10-02T15:01:23+05:30"`. |

### CloneDefinition

| JSON representation |
|---|
| ``` { "baseTableReference": { object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_dataset_info#Output.Schema.TableReference`) }, "cloneTime": string } ``` |

| Fields ||
|---|---|
| `baseTableReference` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_dataset_info#Output.Schema.TableReference`)`` Required. Reference describing the ID of the table that was cloned. |
| `cloneTime` | ``string (`https://protobuf.dev/reference/protobuf/google.protobuf#timestamp` format)`` Required. The time at which the base table was cloned. This value is reported in the JSON response using RFC3339 format. Uses RFC 3339, where generated output will always be Z-normalized and use 0, 3, 6 or 9 fractional digits. Offsets other than "Z" are also accepted. Examples: `"2014-10-02T15:01:23Z"`, `"2014-10-02T15:01:23.045123456Z"` or `"2014-10-02T15:01:23+05:30"`. |

### RestrictionConfig

| JSON representation |
|---|
| ``` { "type": enum (`RestrictionType`) } ``` |

| Fields ||
|---|---|
| `type` | ``enum (`RestrictionType`)`` Output only. Specifies the type of dataset/table restriction. |

### TableConstraints

| JSON representation |
|---|
| ``` { "primaryKey": { object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_table_info#Output.Schema.PrimaryKey`) }, "foreignKeys": [ { object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_table_info#Output.Schema.ForeignKey`) } ] } ``` |

| Fields ||
|---|---|
| `primaryKey` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_table_info#Output.Schema.PrimaryKey`)`` Optional. Represents a primary key constraint on a table's columns. Present only if the table has a primary key. The primary key is not enforced. |
| `foreignKeys[]` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_table_info#Output.Schema.ForeignKey`)`` Optional. Present only if the table has a foreign key. The foreign key is not enforced. |

### PrimaryKey

| JSON representation |
|---|
| ``` { "columns": [ string ] } ``` |

| Fields ||
|---|---|
| `columns[]` | `string` Required. The columns that are composed of the primary key constraint. |

### ForeignKey

| JSON representation |
|---|
| ``` { "name": string, "referencedTable": { object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_dataset_info#Output.Schema.TableReference`) }, "columnReferences": [ { object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_table_info#Output.Schema.ColumnReference`) } ] } ``` |

| Fields ||
|---|---|
| `name` | `string` Optional. Set only if the foreign key constraint is named. |
| `referencedTable` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_dataset_info#Output.Schema.TableReference`)`` Required. The table that holds the primary key and is referenced by this foreign key. |
| `columnReferences[]` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_table_info#Output.Schema.ColumnReference`)`` Required. The columns that compose the foreign key. |

### ColumnReference

| JSON representation |
|---|
| ``` { "referencingColumn": string, "referencedColumn": string } ``` |

| Fields ||
|---|---|
| `referencingColumn` | `string` Required. The column that composes the foreign key. |
| `referencedColumn` | `string` Required. The column in the primary key that are referenced by the referencing_column. |

### ResourceTagsEntry

| JSON representation |
|---|
| ``` { "key": string, "value": string } ``` |

| Fields ||
|---|---|
| `key` | `string` |
| `value` | `string` |

### TableReplicationInfo

| JSON representation |
|---|
| ``` { "sourceTable": { object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_dataset_info#Output.Schema.TableReference`) }, "replicationIntervalMs": string, "replicatedSourceLastRefreshTime": string, "replicationStatus": enum (`ReplicationStatus`), "replicationError": { object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_table_info#Output.Schema.ErrorProto`) } } ``` |

| Fields ||
|---|---|
| `sourceTable` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_dataset_info#Output.Schema.TableReference`)`` Required. Source table reference that is replicated. |
| `replicationIntervalMs` | `string (https://developers.google.com/discovery/v1/type-format format)` Optional. Specifies the interval at which the source table is polled for updates. It's Optional. If not specified, default replication interval would be applied. |
| `replicatedSourceLastRefreshTime` | `string (https://developers.google.com/discovery/v1/type-format format)` Optional. Output only. If source is a materialized view, this field signifies the last refresh time of the source. |
| `replicationStatus` | ``enum (`ReplicationStatus`)`` Optional. Output only. Replication status of configured replication. |
| `replicationError` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_table_info#Output.Schema.ErrorProto`)`` Optional. Output only. Replication error that will permanently stopped table replication. |

### ExternalCatalogTableOptions

| JSON representation |
|---|
| ``` { "parameters": { string: string, ... }, "storageDescriptor": { object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_table_info#Output.Schema.StorageDescriptor`) }, "connectionId": string } ``` |

| Fields ||
|---|---|
| `parameters` | `map (key: string, value: string)` Optional. A map of the key-value pairs defining the parameters and properties of the open source table. Corresponds with Hive metastore table parameters. Maximum size of 4MiB. An object containing a list of `"key": value` pairs. Example: `{ "name": "wrench", "mass": "1.3kg", "count": "3" }`. |
| `storageDescriptor` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_table_info#Output.Schema.StorageDescriptor`)`` Optional. A storage descriptor containing information about the physical storage of this table. |
| `connectionId` | `string` Optional. A connection ID that specifies the credentials to be used to read external storage, such as Azure Blob, Cloud Storage, or Amazon S3. This connection is needed to read the open source table from BigQuery. The connection_id format must be either `<project_id>.<location_id>.<connection_id>` or `projects/<project_id>/locations/<location_id>/connections/<connection_id>`. |

### ParametersEntry

| JSON representation |
|---|
| ``` { "key": string, "value": string } ``` |

| Fields ||
|---|---|
| `key` | `string` |
| `value` | `string` |

### StorageDescriptor

| JSON representation |
|---|
| ``` { "locationUri": string, "inputFormat": string, "outputFormat": string, "serdeInfo": { object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_table_info#Output.Schema.SerDeInfo`) } } ``` |

| Fields ||
|---|---|
| `locationUri` | `string` Optional. The physical location of the table (e.g. `gs://spark-dataproc-data/pangea-data/case_sensitive/` or `gs://spark-dataproc-data/pangea-data/*`). The maximum length is 2056 bytes. |
| `inputFormat` | `string` Optional. Specifies the fully qualified class name of the InputFormat (e.g. "org.apache.hadoop.hive.ql.io.orc.OrcInputFormat"). The maximum length is 128 characters. |
| `outputFormat` | `string` Optional. Specifies the fully qualified class name of the OutputFormat (e.g. "org.apache.hadoop.hive.ql.io.orc.OrcOutputFormat"). The maximum length is 128 characters. |
| `serdeInfo` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/mcp/tools_list/get_table_info#Output.Schema.SerDeInfo`)`` Optional. Serializer and deserializer information. |

### SerDeInfo

| JSON representation |
|---|
| ``` { "name": string, "serializationLibrary": string, "parameters": { string: string, ... } } ``` |

| Fields ||
|---|---|
| `name` | `string` Optional. Name of the SerDe. The maximum length is 256 characters. |
| `serializationLibrary` | `string` Required. Specifies a fully-qualified class name of the serialization library that is responsible for the translation of data between table representation and the underlying low-level input and output format structures. The maximum length is 256 characters. |
| `parameters` | `map (key: string, value: string)` Optional. Key-value pairs that define the initialization parameters for the serialization library. Maximum size 10 Kib. An object containing a list of `"key": value` pairs. Example: `{ "name": "wrench", "mass": "1.3kg", "count": "3" }`. |

### ParametersEntry

| JSON representation |
|---|
| ``` { "key": string, "value": string } ``` |

| Fields ||
|---|---|
| `key` | `string` |
| `value` | `string` |

### Tool Annotations

Destructive Hint: ❌ \| Idempotent Hint: ✅ \| Read Only Hint: ✅ \| Open World Hint: ❌