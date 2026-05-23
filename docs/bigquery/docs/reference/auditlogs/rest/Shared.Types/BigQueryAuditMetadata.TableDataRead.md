# BigQueryAuditMetadata.TableDataRead.Reason

Describes how the table data was read.

| Enums ||
|---|---|
| `REASON_UNSPECIFIED` | Unknown. |
| `JOB` | Table was used as a source table during a BigQuery job. |
| `TABLEDATA_LIST_REQUEST` | Table data was accessed using the tabledata.list API. |
| `GET_QUERY_RESULTS_REQUEST` | Table data was accessed using the jobs.getQueryResults API. |
| `QUERY_REQUEST` | Table data was accessed using the jobs.query RPC. |
| `CREATE_READ_SESSION` | Table data was accessed using storage.CreateReadSession API. |
| `MATERIALIZED_VIEW_REFRESH` | Table data was accessed during a materialized view refresh. |
| `READ_ROWS` | Table data was accessed using storage.ReadRows API.\` |
| `SPLIT_READ_STREAM` | Table data was accessed using storage.SplitReadStream API. |
| `GET_WRITE_STREAM` | Table data was accessed using storage.GetWriteStream API. |