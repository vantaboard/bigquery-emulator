# DmlStats

- [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/DmlStats#SCHEMA_REPRESENTATION)
- [DmlMode](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/DmlStats#DmlMode)
- [FineGrainedDmlUnusedReason](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/DmlStats#FineGrainedDmlUnusedReason)

Detailed statistics for DML statements

| JSON representation |
|---|
| ``` { "insertedRowCount": string, "deletedRowCount": string, "updatedRowCount": string, "dmlMode": enum (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/DmlStats#DmlMode`), "fineGrainedDmlUnusedReason": enum (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/DmlStats#FineGrainedDmlUnusedReason`) } ``` |

| Fields ||
|---|---|
| `insertedRowCount` | `string (https://developers.google.com/discovery/v1/type-format format)` Output only. Number of inserted Rows. Populated by DML INSERT and MERGE statements |
| `deletedRowCount` | `string (https://developers.google.com/discovery/v1/type-format format)` Output only. Number of deleted Rows. populated by DML DELETE, MERGE and TRUNCATE statements. |
| `updatedRowCount` | `string (https://developers.google.com/discovery/v1/type-format format)` Output only. Number of updated Rows. Populated by DML UPDATE and MERGE statements. |
| `dmlMode` | ``enum (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/DmlStats#DmlMode`)`` Output only. DML mode used. |
| `fineGrainedDmlUnusedReason` | ``enum (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/DmlStats#FineGrainedDmlUnusedReason`)`` Output only. Reason for disabling fine-grained DML if applicable. |

## DmlMode

Enum to specify the DML mode used.

| Enums ||
|---|---|
| `DML_MODE_UNSPECIFIED` | Default value. This value is unused. |
| `COARSE_GRAINED_DML` | Coarse-grained DML was used. |
| `FINE_GRAINED_DML` | Fine-grained DML was used. |

## FineGrainedDmlUnusedReason

Reason for disabling fine-grained DML. Additional values may be added in the future.

| Enums ||
|---|---|
| `FINE_GRAINED_DML_UNUSED_REASON_UNSPECIFIED` | Default value. This value is unused. |
| `MAX_PARTITION_SIZE_EXCEEDED` | Max partition size threshold exceeded. [Fine-grained DML Limitations](https://docs.cloud.google.com/bigquery/docs/data-manipulation-language#fine-grained-dml-limitations) |
| `TABLE_NOT_ENROLLED` | The table is not enrolled for fine-grained DML. |
| `DML_IN_MULTI_STATEMENT_TRANSACTION` | The DML statement is part of a multi-statement transaction. |