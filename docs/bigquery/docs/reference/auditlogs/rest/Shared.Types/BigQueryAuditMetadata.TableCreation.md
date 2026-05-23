# BigQueryAuditMetadata.TableCreation.Reason

Describes how the table was created.

| Enums ||
|---|---|
| `REASON_UNSPECIFIED` | Unknown. |
| `JOB` | Table was created as a destination table during a query, load or copy job. |
| `QUERY` | Table was created using a DDL query. |
| `TABLE_INSERT_REQUEST` | Table was created using the tables.create API. |