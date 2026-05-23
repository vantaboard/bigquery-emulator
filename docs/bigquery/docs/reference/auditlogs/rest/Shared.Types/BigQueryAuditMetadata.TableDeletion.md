# BigQueryAuditMetadata.TableDeletion.Reason

Describes how the table was deleted.

| Enums ||
|---|---|
| `REASON_UNSPECIFIED` | Unknown. |
| `TABLE_DELETE_REQUEST` | Table was deleted using the tables.delete API. |
| `EXPIRED` | Table expired. |
| `QUERY` | Table deleted using a DDL query. |