# BigQueryAuditMetadata.DatasetChange.Reason

Describes how the dataset was changed.

| Enums ||
|---|---|
| `REASON_UNSPECIFIED` | Unknown. |
| `UPDATE` | Dataset was changed using the datasets.update or datasets.patch API. |
| `SET_IAM_POLICY` | Dataset was changed using the SetIamPolicy API. |
| `QUERY` | Dataset was changed using a query job, e.g., ALTER SCHEMA statement. |