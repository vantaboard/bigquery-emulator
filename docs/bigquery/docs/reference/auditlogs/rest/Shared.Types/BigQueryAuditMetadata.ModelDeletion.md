# BigQueryAuditMetadata.ModelDeletion.Reason

Describes how the model was deleted.

| Enums ||
|---|---|
| `REASON_UNSPECIFIED` | Unknown. |
| `MODEL_DELETE_REQUEST` | Model was deleted using the models.delete API. |
| `EXPIRED` | Model expired. |
| `QUERY` | Model was deleted using DDL query. |