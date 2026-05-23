# BigQueryAuditMetadata.TableDataChange.Reason

Describes how the table data was changed.

| Enums ||
|---|---|
| `REASON_UNSPECIFIED` | Unknown. |
| `JOB` | Table was used as a job destination table. |
| `QUERY` | Table data was updated using a DML or DDL query. |
| `MATERIALIZED_VIEW_REFRESH` | Table data was updated during a materialized view refresh. |
| `WRITE_API_APPEND_ROWS` | Table data was chanegd using the Write API append rows. |
| `WRITE_API_CREATE_WRITE_STREAM` | Table data was changed using the Write API create write stream. |
| `WRITE_API_FINALIZE_WRITE_STREAM` | Table data was changed using the Write API finalize write stream. |
| `WRITE_API_FLUSH_ROWS` | Table data was changed using the Write API flush rows. |
| `WRITE_API_BATCH_COMMIT_WRITE_STREAMS` | Table data was changed using the Write API batch commit stream. |