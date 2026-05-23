# BigQueryAuditMetadata.DatasetDeletion.Reason

Describes how the dataset was deleted.

| Enums ||
|---|---|
| `REASON_UNSPECIFIED` | Unknown. |
| `DELETE` | Dataset was deleted using the datasets.delete API. |
| `QUERY` | Dataset was deleted using a query job, e.g., DROP SCHEMA statement. |