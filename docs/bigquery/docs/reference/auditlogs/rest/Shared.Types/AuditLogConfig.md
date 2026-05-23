# AuditLogConfig.LogType

The list of valid permission types for which logging can be configured. Admin writes are always logged, and are not configurable.

| Enums ||
|---|---|
| `LOG_TYPE_UNSPECIFIED` | Default case. Should never be this. |
| `ADMIN_READ` | Admin reads. Example: CloudIAM getIamPolicy |
| `DATA_WRITE` | Data writes. Example: CloudSQL Users create |
| `DATA_READ` | Data reads. Example: CloudSQL Users list |