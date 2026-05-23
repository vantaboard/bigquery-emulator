# Package google.cloud.bigquery.migration.tasks.translation.v2alpha

## Index

- `https://docs.cloud.google.com/bigquery/docs/reference/migration/rpc/google.cloud.bigquery.migration.tasks.translation.v2alpha#google.cloud.bigquery.migration.tasks.translation.v2alpha.BteqOptions` (message)
- `https://docs.cloud.google.com/bigquery/docs/reference/migration/rpc/google.cloud.bigquery.migration.tasks.translation.v2alpha#google.cloud.bigquery.migration.tasks.translation.v2alpha.DatasetReference` (message)
- `https://docs.cloud.google.com/bigquery/docs/reference/migration/rpc/google.cloud.bigquery.migration.tasks.translation.v2alpha#google.cloud.bigquery.migration.tasks.translation.v2alpha.Filter` (message)
- `https://docs.cloud.google.com/bigquery/docs/reference/migration/rpc/google.cloud.bigquery.migration.tasks.translation.v2alpha#google.cloud.bigquery.migration.tasks.translation.v2alpha.IdentifierSettings` (message)
- `https://docs.cloud.google.com/bigquery/docs/reference/migration/rpc/google.cloud.bigquery.migration.tasks.translation.v2alpha#google.cloud.bigquery.migration.tasks.translation.v2alpha.IdentifierSettings.IdentifierCase` (enum)
- `https://docs.cloud.google.com/bigquery/docs/reference/migration/rpc/google.cloud.bigquery.migration.tasks.translation.v2alpha#google.cloud.bigquery.migration.tasks.translation.v2alpha.IdentifierSettings.IdentifierRewriteMode` (enum)
- `https://docs.cloud.google.com/bigquery/docs/reference/migration/rpc/google.cloud.bigquery.migration.tasks.translation.v2alpha#google.cloud.bigquery.migration.tasks.translation.v2alpha.TeradataOptions` (message)
- `https://docs.cloud.google.com/bigquery/docs/reference/migration/rpc/google.cloud.bigquery.migration.tasks.translation.v2alpha#google.cloud.bigquery.migration.tasks.translation.v2alpha.TranslationFileMapping` (message)
- `https://docs.cloud.google.com/bigquery/docs/reference/migration/rpc/google.cloud.bigquery.migration.tasks.translation.v2alpha#google.cloud.bigquery.migration.tasks.translation.v2alpha.TranslationTaskDetails` (message)
- `https://docs.cloud.google.com/bigquery/docs/reference/migration/rpc/google.cloud.bigquery.migration.tasks.translation.v2alpha#google.cloud.bigquery.migration.tasks.translation.v2alpha.TranslationTaskDetails.FileEncoding` (enum)
- `https://docs.cloud.google.com/bigquery/docs/reference/migration/rpc/google.cloud.bigquery.migration.tasks.translation.v2alpha#google.cloud.bigquery.migration.tasks.translation.v2alpha.TranslationTaskDetails.TokenType` (enum)

## BteqOptions

BTEQ translation task related settings.

| Fields ||
|---|---|
| `project_dataset` | `https://docs.cloud.google.com/bigquery/docs/reference/migration/rpc/google.cloud.bigquery.migration.tasks.translation.v2alpha#google.cloud.bigquery.migration.tasks.translation.v2alpha.DatasetReference` Specifies the project and dataset in BigQuery that will be used for external table creation during the translation. |
| `default_path_uri` | `string` The Cloud Storage location to be used as the default path for files that are not otherwise specified in the file replacement map. |
| `file_replacement_map` | `map<string, string>` Maps the local paths that are used in BTEQ scripts (the keys) to the paths in Cloud Storage that should be used in their stead in the translation (the value). |

## DatasetReference

| Fields ||
|---|---|
| `dataset_id` | `string` Required. A unique ID for this dataset, without the project name. The ID must contain only letters (a-z, A-Z), numbers (0-9), or underscores (_). The maximum length is 1,024 characters. |
| `project_id` | `string` Optional. The ID of the project containing this dataset. |

## Filter

The filter applied to fields of translation details.

| Fields ||
|---|---|
| `input_file_exclusion_prefixes[]` | `string` The list of prefixes used to exclude processing for input files. |

## IdentifierSettings

Settings related to SQL identifiers.

| Fields ||
|---|---|
| `output_identifier_case` | `https://docs.cloud.google.com/bigquery/docs/reference/migration/rpc/google.cloud.bigquery.migration.tasks.translation.v2alpha#google.cloud.bigquery.migration.tasks.translation.v2alpha.IdentifierSettings.IdentifierCase` The setting to control output queries' identifier case. |
| `identifier_rewrite_mode` | `https://docs.cloud.google.com/bigquery/docs/reference/migration/rpc/google.cloud.bigquery.migration.tasks.translation.v2alpha#google.cloud.bigquery.migration.tasks.translation.v2alpha.IdentifierSettings.IdentifierRewriteMode` Specifies the rewrite mode for SQL identifiers. |

## IdentifierCase

The identifier case type.

| Enums ||
|---|---|
| `IDENTIFIER_CASE_UNSPECIFIED` | The identifier case is not specified. |
| `ORIGINAL` | Identifiers' cases will be kept as the original cases. |
| `UPPER` | Identifiers will be in upper cases. |
| `LOWER` | Identifiers will be in lower cases. |

## IdentifierRewriteMode

The SQL identifier rewrite mode.

| Enums ||
|---|---|
| `IDENTIFIER_REWRITE_MODE_UNSPECIFIED` | SQL Identifier rewrite mode is unspecified. |
| `NONE` | SQL identifiers won't be rewrite. |
| `REWRITE_ALL` | All SQL identifiers will be rewrite. |

## TeradataOptions

This type has no fields.
Teradata SQL specific translation task related settings.

## TranslationFileMapping

Mapping between an input and output file to be translated in a subtask.

| Fields ||
|---|---|
| `input_path` | `string` The Cloud Storage path for a file to translation in a subtask. |
| `output_path` | `string` The Cloud Storage path to write back the corresponding input file to. |

## TranslationTaskDetails

DEPRECATED! Use TranslationTaskDetails defined in com.google.cloud.bigquery.migration.v2alpha.TranslationTaskDetails instead. The translation task details to capture necessary settings for a translation task and subtask.

| Fields ||
|---|---|
| `input_path` | `string` The Cloud Storage path for translation input files. |
| `output_path` | `string` The Cloud Storage path for translation output files. |
| `file_paths[]` | `https://docs.cloud.google.com/bigquery/docs/reference/migration/rpc/google.cloud.bigquery.migration.tasks.translation.v2alpha#google.cloud.bigquery.migration.tasks.translation.v2alpha.TranslationFileMapping` Cloud Storage files to be processed for translation. |
| `schema_path` | `string` The Cloud Storage path to DDL files as table schema to assist semantic translation. |
| `file_encoding` | `https://docs.cloud.google.com/bigquery/docs/reference/migration/rpc/google.cloud.bigquery.migration.tasks.translation.v2alpha#google.cloud.bigquery.migration.tasks.translation.v2alpha.TranslationTaskDetails.FileEncoding` The file encoding type. |
| `identifier_settings` | `https://docs.cloud.google.com/bigquery/docs/reference/migration/rpc/google.cloud.bigquery.migration.tasks.translation.v2alpha#google.cloud.bigquery.migration.tasks.translation.v2alpha.IdentifierSettings` The settings for SQL identifiers. |
| `special_token_map` | ``map<string, `https://docs.cloud.google.com/bigquery/docs/reference/migration/rpc/google.cloud.bigquery.migration.tasks.translation.v2alpha#google.cloud.bigquery.migration.tasks.translation.v2alpha.TranslationTaskDetails.TokenType`>`` The map capturing special tokens to be replaced during translation. The key is special token in string. The value is the token data type. This is used to translate SQL query template which contains special token as place holder. The special token makes a query invalid to parse. This map will be applied to annotate those special token with types to let parser understand how to parse them into proper structure with type information. |
| `filter` | `https://docs.cloud.google.com/bigquery/docs/reference/migration/rpc/google.cloud.bigquery.migration.tasks.translation.v2alpha#google.cloud.bigquery.migration.tasks.translation.v2alpha.Filter` The filter applied to translation details. |
| `translation_exception_table` | `string` Specifies the exact name of the bigquery table ("dataset.table") to be used for surfacing raw translation errors. If the table does not exist, we will create it. If it already exists and the schema is the same, we will re-use. If the table exists and the schema is different, we will throw an error. |
| Union field `language_options`. The language specific settings for the translation task. `language_options` can be only one of the following: ||
| `teradata_options` | `https://docs.cloud.google.com/bigquery/docs/reference/migration/rpc/google.cloud.bigquery.migration.tasks.translation.v2alpha#google.cloud.bigquery.migration.tasks.translation.v2alpha.TeradataOptions` The Teradata SQL specific settings for the translation task. |
| `bteq_options` | `https://docs.cloud.google.com/bigquery/docs/reference/migration/rpc/google.cloud.bigquery.migration.tasks.translation.v2alpha#google.cloud.bigquery.migration.tasks.translation.v2alpha.BteqOptions` The BTEQ specific settings for the translation task. |

## FileEncoding

The file encoding types.

| Enums ||
|---|---|
| `FILE_ENCODING_UNSPECIFIED` | File encoding setting is not specified. |
| `UTF_8` | File encoding is UTF_8. |
| `ISO_8859_1` | File encoding is ISO_8859_1. |
| `US_ASCII` | File encoding is US_ASCII. |
| `UTF_16` | File encoding is UTF_16. |
| `UTF_16LE` | File encoding is UTF_16LE. |
| `UTF_16BE` | File encoding is UTF_16BE. |

## TokenType

The special token data type.

| Enums ||
|---|---|
| `TOKEN_TYPE_UNSPECIFIED` | Token type is not specified. |
| `STRING` | Token type as string. |
| `INT64` | Token type as integer. |
| `NUMERIC` | Token type as numeric. |
| `BOOL` | Token type as boolean. |
| `FLOAT64` | Token type as float. |
| `DATE` | Token type as date. |
| `TIMESTAMP` | Token type as timestamp. |