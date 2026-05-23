# AuditConfig

- [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/Shared.Types/AuditConfig#SCHEMA_REPRESENTATION)
- [AuditLogConfig](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/Shared.Types/AuditConfig#AuditLogConfig)
  - [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/Shared.Types/AuditConfig#AuditLogConfig.SCHEMA_REPRESENTATION)

Specifies the audit configuration for a service. The configuration determines which permission types are logged, and what identities, if any, are exempted from logging. An AuditConfig must have one or more AuditLogConfigs.

If there are AuditConfigs for both `allServices` and a specific service, the union of the two AuditConfigs is used for that service: the log_types specified in each AuditConfig are enabled, and the exemptedMembers in each AuditLogConfig are exempted.

Example Policy with multiple AuditConfigs:

    {
      "auditConfigs": [
        {
          "service": "allServices",
          "auditLogConfigs": [
            {
              "logType": "DATA_READ",
              "exemptedMembers": [
                "user:jose@example.com"
              ]
            },
            {
              "logType": "DATA_WRITE"
            },
            {
              "logType": "ADMIN_READ"
            }
          ]
        },
        {
          "service": "sampleservice.googleapis.com",
          "auditLogConfigs": [
            {
              "logType": "DATA_READ"
            },
            {
              "logType": "DATA_WRITE",
              "exemptedMembers": [
                "user:aliya@example.com"
              ]
            }
          ]
        }
      ]
    }

For sampleservice, this policy enables DATA_READ, DATA_WRITE and ADMIN_READ logging. It also exempts `jose@example.com` from DATA_READ logging, and `aliya@example.com` from DATA_WRITE logging.

| JSON representation |
|---|
| ``` { "service": string, "auditLogConfigs": [ { object (`https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/Shared.Types/AuditConfig#AuditLogConfig`) } ] } ``` |

| Fields ||
|---|---|
| `service` | `string` Specifies a service that will be enabled for audit logging. For example, `storage.googleapis.com`, `cloudsql.googleapis.com`. `allServices` is a special value that covers all services. |
| `auditLogConfigs[]` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/Shared.Types/AuditConfig#AuditLogConfig`)`` The configuration for logging of each type of permission. |

## AuditLogConfig

Provides the configuration for logging a type of permissions. Example:

    {
      "auditLogConfigs": [
        {
          "logType": "DATA_READ",
          "exemptedMembers": [
            "user:jose@example.com"
          ]
        },
        {
          "logType": "DATA_WRITE"
        }
      ]
    }

This enables 'DATA_READ' and 'DATA_WRITE' logging, while exempting [jose@example.com](mailto:jose@example.com) from DATA_READ logging.

| JSON representation |
|---|
| ``` { "logType": enum (`https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/Shared.Types/LogType`), "exemptedMembers": [ string ] } ``` |

| Fields ||
|---|---|
| `logType` | ``enum (`https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/Shared.Types/LogType`)`` The log type that this config enables. |
| `exemptedMembers[]` | `string` Specifies the identities that do not cause logging for this type of permission. Follows the same format of `https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/Shared.Types/Binding#FIELDS.members`. |