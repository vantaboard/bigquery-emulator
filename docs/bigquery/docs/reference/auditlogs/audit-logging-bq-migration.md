# BigQuery Migration Service audit logging

This document describes audit logging for BigQuery Migration Service. Google Cloud services
generate audit logs that record administrative and access activities within your Google Cloud resources.
For more information about Cloud Audit Logs, see the following:

- [Types of audit logs](https://docs.cloud.google.com/logging/docs/audit#types)
- [Audit log entry structure](https://docs.cloud.google.com/logging/docs/audit#audit_log_entry_structure)
- [Storing and routing audit logs](https://docs.cloud.google.com/logging/docs/audit#storing_and_routing_audit_logs)
- [Cloud Logging pricing summary](https://docs.cloud.google.com/stackdriver/pricing#logs-pricing-summary)
- [Enable Data Access audit logs](https://docs.cloud.google.com/logging/docs/audit/configure-data-access)

<br />

## Service name

BigQuery Migration Service audit logs use the service name `bigquerymigration.googleapis.com`.
Filter for this service:

```
    protoPayload.serviceName="bigquerymigration.googleapis.com"
  
```

<br />

## Methods by permission type

Each IAM permission has a `type` property, whose value is an enum
that can be one of four values: `ADMIN_READ`, `ADMIN_WRITE`,
`DATA_READ`, or `DATA_WRITE`. When you call a method,
BigQuery Migration Service generates an audit log whose category is dependent on the
`type` property of the permission required to perform the method.

Methods that require an IAM permission with the `type` property value
of `DATA_READ`, `DATA_WRITE`, or `ADMIN_READ` generate
[Data Access](https://docs.cloud.google.com/logging/docs/audit#data-access) audit logs.

Methods that require an IAM permission with the `type` property value
of `ADMIN_WRITE` generate
[Admin Activity](https://docs.cloud.google.com/logging/docs/audit#admin-activity) audit logs.
API methods in the following list that are marked with (LRO) are long-running operations (LROs). These methods usually generate two audit log entries: one when the operation starts and another when it ends. For more information see [Audit logs for long-running operations](https://docs.cloud.google.com/logging/docs/audit/understanding-audit-logs#lro).

| Permission type | Methods |
|---|---|
| `DATA_READ` | `google.cloud.bigquery.migration.v2.MigrationService.GetMigrationSubtask` `google.cloud.bigquery.migration.v2.MigrationService.GetMigrationWorkflow` `google.cloud.bigquery.migration.v2.MigrationService.ListMigrationSubtasks` `google.cloud.bigquery.migration.v2.MigrationService.ListMigrationWorkflows` `google.cloud.bigquery.migration.v2.SqlTranslationService.TranslateQuery` `google.cloud.bigquery.migration.v2alpha.MigrationService.GetMigrationSubtask` `google.cloud.bigquery.migration.v2alpha.MigrationService.GetMigrationWorkflow` `google.cloud.bigquery.migration.v2alpha.MigrationService.ListMigrationSubtasks` `google.cloud.bigquery.migration.v2alpha.MigrationService.ListMigrationWorkflows` `google.cloud.bigquery.migration.v2alpha.SqlTranslationService.TranslateQuery` |
| `DATA_WRITE` | `google.cloud.bigquery.migration.v2.MigrationService.CreateMigrationWorkflow` `google.cloud.bigquery.migration.v2.MigrationService.DeleteMigrationWorkflow` `google.cloud.bigquery.migration.v2.MigrationService.StartMigrationWorkflow` `google.cloud.bigquery.migration.v2alpha.MigrationService.CreateMigrationWorkflow` `google.cloud.bigquery.migration.v2alpha.MigrationService.DeleteMigrationWorkflow` `google.cloud.bigquery.migration.v2alpha.MigrationService.StartMigrationWorkflow` |

## API interface audit logs

For information about how and which permissions are evaluated for each method,
see the Identity and Access Management documentation for BigQuery Migration Service.

### `google.cloud.bigquery.migration.v2.MigrationService`

The following audit logs are associated with methods belonging to
`google.cloud.bigquery.migration.v2.MigrationService`.

#### `CreateMigrationWorkflow`

- **Method** : `google.cloud.bigquery.migration.v2.MigrationService.CreateMigrationWorkflow`  
- **Audit log type** : [Data access](https://docs.cloud.google.com/logging/docs/audit#data-access)  
- **Permissions** :
  - `bigquerymigration.workflows.create - DATA_WRITE`
- **Method is a long-running or streaming operation** : No.  
- **Filter for this method** : `
  protoPayload.methodName="google.cloud.bigquery.migration.v2.MigrationService.CreateMigrationWorkflow"
  `  

#### `DeleteMigrationWorkflow`

- **Method** : `google.cloud.bigquery.migration.v2.MigrationService.DeleteMigrationWorkflow`  
- **Audit log type** : [Data access](https://docs.cloud.google.com/logging/docs/audit#data-access)  
- **Permissions** :
  - `bigquerymigration.workflows.delete - DATA_WRITE`
- **Method is a long-running or streaming operation** : No.  
- **Filter for this method** : `
  protoPayload.methodName="google.cloud.bigquery.migration.v2.MigrationService.DeleteMigrationWorkflow"
  `  

#### `GetMigrationSubtask`

- **Method** : `google.cloud.bigquery.migration.v2.MigrationService.GetMigrationSubtask`  
- **Audit log type** : [Data access](https://docs.cloud.google.com/logging/docs/audit#data-access)  
- **Permissions** :
  - `bigquerymigration.subtasks.get - DATA_READ`
- **Method is a long-running or streaming operation** : No.  
- **Filter for this method** : `
  protoPayload.methodName="google.cloud.bigquery.migration.v2.MigrationService.GetMigrationSubtask"
  `  

#### `GetMigrationWorkflow`

- **Method** : `google.cloud.bigquery.migration.v2.MigrationService.GetMigrationWorkflow`  
- **Audit log type** : [Data access](https://docs.cloud.google.com/logging/docs/audit#data-access)  
- **Permissions** :
  - `bigquerymigration.workflows.get - DATA_READ`
- **Method is a long-running or streaming operation** : No.  
- **Filter for this method** : `
  protoPayload.methodName="google.cloud.bigquery.migration.v2.MigrationService.GetMigrationWorkflow"
  `  

#### `ListMigrationSubtasks`

- **Method** : `google.cloud.bigquery.migration.v2.MigrationService.ListMigrationSubtasks`  
- **Audit log type** : [Data access](https://docs.cloud.google.com/logging/docs/audit#data-access)  
- **Permissions** :
  - `bigquerymigration.subtasks.list - DATA_READ`
- **Method is a long-running or streaming operation** : No.  
- **Filter for this method** : `
  protoPayload.methodName="google.cloud.bigquery.migration.v2.MigrationService.ListMigrationSubtasks"
  `  

#### `ListMigrationWorkflows`

- **Method** : `google.cloud.bigquery.migration.v2.MigrationService.ListMigrationWorkflows`  
- **Audit log type** : [Data access](https://docs.cloud.google.com/logging/docs/audit#data-access)  
- **Permissions** :
  - `bigquerymigration.workflows.list - DATA_READ`
- **Method is a long-running or streaming operation** : No.  
- **Filter for this method** : `
  protoPayload.methodName="google.cloud.bigquery.migration.v2.MigrationService.ListMigrationWorkflows"
  `  

#### `StartMigrationWorkflow`

- **Method** : `google.cloud.bigquery.migration.v2.MigrationService.StartMigrationWorkflow`  
- **Audit log type** : [Data access](https://docs.cloud.google.com/logging/docs/audit#data-access)  
- **Permissions** :
  - `bigquerymigration.workflows.update - DATA_WRITE`
- **Method is a long-running or streaming operation** : No.  
- **Filter for this method** : `
  protoPayload.methodName="google.cloud.bigquery.migration.v2.MigrationService.StartMigrationWorkflow"
  `  

### `google.cloud.bigquery.migration.v2.SqlTranslationService`

The following audit logs are associated with methods belonging to
`google.cloud.bigquery.migration.v2.SqlTranslationService`.

#### `TranslateQuery`

- **Method** : `google.cloud.bigquery.migration.v2.SqlTranslationService.TranslateQuery`  
- **Audit log type** : [Data access](https://docs.cloud.google.com/logging/docs/audit#data-access)  
- **Permissions** :
  - `bigquerymigration.translation.translate - DATA_READ`
- **Method is a long-running or streaming operation** : No.  
- **Filter for this method** : `
  protoPayload.methodName="google.cloud.bigquery.migration.v2.SqlTranslationService.TranslateQuery"
  `  

### `google.cloud.bigquery.migration.v2alpha.MigrationService`

The following audit logs are associated with methods belonging to
`google.cloud.bigquery.migration.v2alpha.MigrationService`.

#### `CreateMigrationWorkflow`

- **Method** : `google.cloud.bigquery.migration.v2alpha.MigrationService.CreateMigrationWorkflow`  
- **Audit log type** : [Data access](https://docs.cloud.google.com/logging/docs/audit#data-access)  
- **Permissions** :
  - `bigquerymigration.workflows.create - DATA_WRITE`
- **Method is a long-running or streaming operation** : No.  
- **Filter for this method** : `
  protoPayload.methodName="google.cloud.bigquery.migration.v2alpha.MigrationService.CreateMigrationWorkflow"
  `  

#### `DeleteMigrationWorkflow`

- **Method** : `google.cloud.bigquery.migration.v2alpha.MigrationService.DeleteMigrationWorkflow`  
- **Audit log type** : [Data access](https://docs.cloud.google.com/logging/docs/audit#data-access)  
- **Permissions** :
  - `bigquerymigration.workflows.delete - DATA_WRITE`
- **Method is a long-running or streaming operation** : No.  
- **Filter for this method** : `
  protoPayload.methodName="google.cloud.bigquery.migration.v2alpha.MigrationService.DeleteMigrationWorkflow"
  `  

#### `GetMigrationSubtask`

- **Method** : `google.cloud.bigquery.migration.v2alpha.MigrationService.GetMigrationSubtask`  
- **Audit log type** : [Data access](https://docs.cloud.google.com/logging/docs/audit#data-access)  
- **Permissions** :
  - `bigquerymigration.subtasks.get - DATA_READ`
- **Method is a long-running or streaming operation** : No.  
- **Filter for this method** : `
  protoPayload.methodName="google.cloud.bigquery.migration.v2alpha.MigrationService.GetMigrationSubtask"
  `  

#### `GetMigrationWorkflow`

- **Method** : `google.cloud.bigquery.migration.v2alpha.MigrationService.GetMigrationWorkflow`  
- **Audit log type** : [Data access](https://docs.cloud.google.com/logging/docs/audit#data-access)  
- **Permissions** :
  - `bigquerymigration.workflows.get - DATA_READ`
- **Method is a long-running or streaming operation** : No.  
- **Filter for this method** : `
  protoPayload.methodName="google.cloud.bigquery.migration.v2alpha.MigrationService.GetMigrationWorkflow"
  `  

#### `ListMigrationSubtasks`

- **Method** : `google.cloud.bigquery.migration.v2alpha.MigrationService.ListMigrationSubtasks`  
- **Audit log type** : [Data access](https://docs.cloud.google.com/logging/docs/audit#data-access)  
- **Permissions** :
  - `bigquerymigration.subtasks.list - DATA_READ`
- **Method is a long-running or streaming operation** : No.  
- **Filter for this method** : `
  protoPayload.methodName="google.cloud.bigquery.migration.v2alpha.MigrationService.ListMigrationSubtasks"
  `  

#### `ListMigrationWorkflows`

- **Method** : `google.cloud.bigquery.migration.v2alpha.MigrationService.ListMigrationWorkflows`  
- **Audit log type** : [Data access](https://docs.cloud.google.com/logging/docs/audit#data-access)  
- **Permissions** :
  - `bigquerymigration.workflows.list - DATA_READ`
- **Method is a long-running or streaming operation** : No.  
- **Filter for this method** : `
  protoPayload.methodName="google.cloud.bigquery.migration.v2alpha.MigrationService.ListMigrationWorkflows"
  `  

#### `StartMigrationWorkflow`

- **Method** : `google.cloud.bigquery.migration.v2alpha.MigrationService.StartMigrationWorkflow`  
- **Audit log type** : [Data access](https://docs.cloud.google.com/logging/docs/audit#data-access)  
- **Permissions** :
  - `bigquerymigration.workflows.update - DATA_WRITE`
- **Method is a long-running or streaming operation** : No.  
- **Filter for this method** : `
  protoPayload.methodName="google.cloud.bigquery.migration.v2alpha.MigrationService.StartMigrationWorkflow"
  `  

### `google.cloud.bigquery.migration.v2alpha.SqlTranslationService`

The following audit logs are associated with methods belonging to
`google.cloud.bigquery.migration.v2alpha.SqlTranslationService`.

#### `TranslateQuery`

- **Method** : `google.cloud.bigquery.migration.v2alpha.SqlTranslationService.TranslateQuery`  
- **Audit log type** : [Data access](https://docs.cloud.google.com/logging/docs/audit#data-access)  
- **Permissions** :
  - `bigquerymigration.translation.translate - DATA_READ`
- **Method is a long-running or streaming operation** : No.  
- **Filter for this method** : `
  protoPayload.methodName="google.cloud.bigquery.migration.v2alpha.SqlTranslationService.TranslateQuery"
  `