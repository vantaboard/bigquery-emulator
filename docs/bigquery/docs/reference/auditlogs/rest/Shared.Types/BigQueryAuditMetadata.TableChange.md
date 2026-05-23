# BigQueryAuditMetadata.TableChange.Reason

Describes how the table metadata was changed.

| Enums ||
|---|---|
| `REASON_UNSPECIFIED` | Unknown. |
| `TABLE_UPDATE_REQUEST` | Table metadata was updated using the tables.update or tables.patch API. |
| `JOB` | Table was used as a job destination table. |
| `QUERY` | Table metadata was updated using a DML or DDL query. |
| `SET_IAM_POLICY` | Table metadata was updated using the SetIamPolicy API. |