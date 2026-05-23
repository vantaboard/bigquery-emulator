# BigQueryAuditMetadata.WriteDisposition

Describes whether a job should overwrite or append the existing destination table if it already exists.

| Enums ||
|---|---|
| `WRITE_DISPOSITION_UNSPECIFIED` | Unknown. |
| `WRITE_EMPTY` | This job should only be writing to empty tables. |
| `WRITE_TRUNCATE` | This job will truncate table data and write from the beginning. This may not preserve table metadata such as table schema, row access policy, column descriptions etc. This is the default value. |
| `WRITE_APPEND` | This job will append to the table. |
| `WRITE_TRUNCATE_DATA` | This job will truncate table data but preserve table metadata such as table schema, row access policy, column descriptions etc. |