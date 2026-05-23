# BigQuery Data Transfer Service audit logging

This document describes audit logging for BigQuery Data Transfer Service. Google Cloud services
generate audit logs that record administrative and access activities within your Google Cloud resources.
For more information about Cloud Audit Logs, see the following:

- [Types of audit logs](https://docs.cloud.google.com/logging/docs/audit#types)
- [Audit log entry structure](https://docs.cloud.google.com/logging/docs/audit#audit_log_entry_structure)
- [Storing and routing audit logs](https://docs.cloud.google.com/logging/docs/audit#storing_and_routing_audit_logs)
- [Cloud Logging pricing summary](https://docs.cloud.google.com/stackdriver/pricing#logs-pricing-summary)
- [Enable Data Access audit logs](https://docs.cloud.google.com/logging/docs/audit/configure-data-access)

<br />

## Service name

BigQuery Data Transfer Service audit logs use the service name `bigquerydatatransfer.googleapis.com`.
Filter for this service:

```
    protoPayload.serviceName="bigquerydatatransfer.googleapis.com"
  
```

<br />

## Methods by permission type

Each IAM permission has a `type` property, whose value is an enum
that can be one of four values: `ADMIN_READ`, `ADMIN_WRITE`,
`DATA_READ`, or `DATA_WRITE`. When you call a method,
BigQuery Data Transfer Service generates an audit log whose category is dependent on the
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
| `ADMIN_READ` | `google.cloud.bigquery.datatransfer.v1.DataTransferService.CheckValidCreds` `google.cloud.bigquery.datatransfer.v1.DataTransferService.GetDataSource` `google.cloud.bigquery.datatransfer.v1.DataTransferService.GetTransferConfig` `google.cloud.bigquery.datatransfer.v1.DataTransferService.GetTransferRun` `google.cloud.bigquery.datatransfer.v1.DataTransferService.ListDataSources` `google.cloud.bigquery.datatransfer.v1.DataTransferService.ListTransferConfigs` `google.cloud.bigquery.datatransfer.v1.DataTransferService.ListTransferLogs` `google.cloud.bigquery.datatransfer.v1.DataTransferService.ListTransferRuns` `google.cloud.location.Locations.GetLocation` `google.cloud.location.Locations.ListLocations` |
| `ADMIN_WRITE` | `google.cloud.bigquery.datatransfer.v1.DataTransferService.CreateTransferConfig` `google.cloud.bigquery.datatransfer.v1.DataTransferService.DeleteTransferConfig` `google.cloud.bigquery.datatransfer.v1.DataTransferService.DeleteTransferRun` `google.cloud.bigquery.datatransfer.v1.DataTransferService.EnrollDataSources` `google.cloud.bigquery.datatransfer.v1.DataTransferService.ScheduleTransferRuns` `google.cloud.bigquery.datatransfer.v1.DataTransferService.StartManualTransferRuns` `google.cloud.bigquery.datatransfer.v1.DataTransferService.UnenrollDataSources` `google.cloud.bigquery.datatransfer.v1.DataTransferService.UpdateTransferConfig` |

## API interface audit logs

For information about how and which permissions are evaluated for each method,
see the Identity and Access Management documentation for BigQuery Data Transfer Service.

### `google.cloud.bigquery.datatransfer.v1.DataTransferService`

The following audit logs are associated with methods belonging to
`google.cloud.bigquery.datatransfer.v1.DataTransferService`.

#### `CheckValidCreds`

- **Method** : `google.cloud.bigquery.datatransfer.v1.DataTransferService.CheckValidCreds`  
- **Audit log type** : [Data access](https://docs.cloud.google.com/logging/docs/audit#data-access)  
- **Permissions** :
  - `bigquery.transfers.get - ADMIN_READ`
- **Method is a long-running or streaming operation** : No.  
- **Filter for this method** : `
  protoPayload.methodName="google.cloud.bigquery.datatransfer.v1.DataTransferService.CheckValidCreds"
  `  

#### `CreateTransferConfig`

- **Method** : `google.cloud.bigquery.datatransfer.v1.DataTransferService.CreateTransferConfig`  
- **Audit log type** : [Admin activity](https://docs.cloud.google.com/logging/docs/audit#admin-activity)  
- **Permissions** :
  - `bigquery.transfers.update - ADMIN_WRITE`
- **Method is a long-running or streaming operation** : No.  
- **Filter for this method** : `
  protoPayload.methodName="google.cloud.bigquery.datatransfer.v1.DataTransferService.CreateTransferConfig"
  `  

#### `DeleteTransferConfig`

- **Method** : `google.cloud.bigquery.datatransfer.v1.DataTransferService.DeleteTransferConfig`  
- **Audit log type** : [Admin activity](https://docs.cloud.google.com/logging/docs/audit#admin-activity)  
- **Permissions** :
  - `bigquery.transfers.update - ADMIN_WRITE`
- **Method is a long-running or streaming operation** : No.  
- **Filter for this method** : `
  protoPayload.methodName="google.cloud.bigquery.datatransfer.v1.DataTransferService.DeleteTransferConfig"
  `  

#### `DeleteTransferRun`

- **Method** : `google.cloud.bigquery.datatransfer.v1.DataTransferService.DeleteTransferRun`  
- **Audit log type** : [Admin activity](https://docs.cloud.google.com/logging/docs/audit#admin-activity)  
- **Permissions** :
  - `bigquery.transfers.update - ADMIN_WRITE`
- **Method is a long-running or streaming operation** : No.  
- **Filter for this method** : `
  protoPayload.methodName="google.cloud.bigquery.datatransfer.v1.DataTransferService.DeleteTransferRun"
  `  

#### `EnrollDataSources`

- **Method** : `google.cloud.bigquery.datatransfer.v1.DataTransferService.EnrollDataSources`  
- **Audit log type** : [Admin activity](https://docs.cloud.google.com/logging/docs/audit#admin-activity)  
- **Permissions** :
  - `resourcemanager.projects.update - ADMIN_WRITE`
- **Method is a long-running or streaming operation** : No.  
- **Filter for this method** : `
  protoPayload.methodName="google.cloud.bigquery.datatransfer.v1.DataTransferService.EnrollDataSources"
  `  

#### `GetDataSource`

- **Method** : `google.cloud.bigquery.datatransfer.v1.DataTransferService.GetDataSource`  
- **Audit log type** : [Data access](https://docs.cloud.google.com/logging/docs/audit#data-access)  
- **Permissions** :
  - `bigquery.transfers.get - ADMIN_READ`
- **Method is a long-running or streaming operation** : No.  
- **Filter for this method** : `
  protoPayload.methodName="google.cloud.bigquery.datatransfer.v1.DataTransferService.GetDataSource"
  `  

#### `GetTransferConfig`

- **Method** : `google.cloud.bigquery.datatransfer.v1.DataTransferService.GetTransferConfig`  
- **Audit log type** : [Data access](https://docs.cloud.google.com/logging/docs/audit#data-access)  
- **Permissions** :
  - `bigquery.transfers.get - ADMIN_READ`
- **Method is a long-running or streaming operation** : No.  
- **Filter for this method** : `
  protoPayload.methodName="google.cloud.bigquery.datatransfer.v1.DataTransferService.GetTransferConfig"
  `  

#### `GetTransferRun`

- **Method** : `google.cloud.bigquery.datatransfer.v1.DataTransferService.GetTransferRun`  
- **Audit log type** : [Data access](https://docs.cloud.google.com/logging/docs/audit#data-access)  
- **Permissions** :
  - `bigquery.transfers.get - ADMIN_READ`
- **Method is a long-running or streaming operation** : No.  
- **Filter for this method** : `
  protoPayload.methodName="google.cloud.bigquery.datatransfer.v1.DataTransferService.GetTransferRun"
  `  

#### `ListDataSources`

- **Method** : `google.cloud.bigquery.datatransfer.v1.DataTransferService.ListDataSources`  
- **Audit log type** : [Data access](https://docs.cloud.google.com/logging/docs/audit#data-access)  
- **Permissions** :
  - `bigquery.transfers.get - ADMIN_READ`
- **Method is a long-running or streaming operation** : No.  
- **Filter for this method** : `
  protoPayload.methodName="google.cloud.bigquery.datatransfer.v1.DataTransferService.ListDataSources"
  `  

#### `ListTransferConfigs`

- **Method** : `google.cloud.bigquery.datatransfer.v1.DataTransferService.ListTransferConfigs`  
- **Audit log type** : [Data access](https://docs.cloud.google.com/logging/docs/audit#data-access)  
- **Permissions** :
  - `bigquery.transfers.get - ADMIN_READ`
- **Method is a long-running or streaming operation** : No.  
- **Filter for this method** : `
  protoPayload.methodName="google.cloud.bigquery.datatransfer.v1.DataTransferService.ListTransferConfigs"
  `  

#### `ListTransferLogs`

- **Method** : `google.cloud.bigquery.datatransfer.v1.DataTransferService.ListTransferLogs`  
- **Audit log type** : [Data access](https://docs.cloud.google.com/logging/docs/audit#data-access)  
- **Permissions** :
  - `bigquery.transfers.get - ADMIN_READ`
- **Method is a long-running or streaming operation** : No.  
- **Filter for this method** : `
  protoPayload.methodName="google.cloud.bigquery.datatransfer.v1.DataTransferService.ListTransferLogs"
  `  

#### `ListTransferRuns`

- **Method** : `google.cloud.bigquery.datatransfer.v1.DataTransferService.ListTransferRuns`  
- **Audit log type** : [Data access](https://docs.cloud.google.com/logging/docs/audit#data-access)  
- **Permissions** :
  - `bigquery.transfers.get - ADMIN_READ`
- **Method is a long-running or streaming operation** : No.  
- **Filter for this method** : `
  protoPayload.methodName="google.cloud.bigquery.datatransfer.v1.DataTransferService.ListTransferRuns"
  `  

#### `ScheduleTransferRuns`

- **Method** : `google.cloud.bigquery.datatransfer.v1.DataTransferService.ScheduleTransferRuns`  
- **Audit log type** : [Admin activity](https://docs.cloud.google.com/logging/docs/audit#admin-activity)  
- **Permissions** :
  - `bigquery.transfers.update - ADMIN_WRITE`
- **Method is a long-running or streaming operation** : No.  
- **Filter for this method** : `
  protoPayload.methodName="google.cloud.bigquery.datatransfer.v1.DataTransferService.ScheduleTransferRuns"
  `  

#### `StartManualTransferRuns`

- **Method** : `google.cloud.bigquery.datatransfer.v1.DataTransferService.StartManualTransferRuns`  
- **Audit log type** : [Admin activity](https://docs.cloud.google.com/logging/docs/audit#admin-activity)  
- **Permissions** :
  - `bigquery.transfers.update - ADMIN_WRITE`
- **Method is a long-running or streaming operation** : No.  
- **Filter for this method** : `
  protoPayload.methodName="google.cloud.bigquery.datatransfer.v1.DataTransferService.StartManualTransferRuns"
  `  

#### `UnenrollDataSources`

- **Method** : `google.cloud.bigquery.datatransfer.v1.DataTransferService.UnenrollDataSources`  
- **Audit log type** : [Admin activity](https://docs.cloud.google.com/logging/docs/audit#admin-activity)  
- **Permissions** :
  - `resourcemanager.projects.update - ADMIN_WRITE`
- **Method is a long-running or streaming operation** : No.  
- **Filter for this method** : `
  protoPayload.methodName="google.cloud.bigquery.datatransfer.v1.DataTransferService.UnenrollDataSources"
  `  

#### `UpdateTransferConfig`

- **Method** : `google.cloud.bigquery.datatransfer.v1.DataTransferService.UpdateTransferConfig`  
- **Audit log type** : [Admin activity](https://docs.cloud.google.com/logging/docs/audit#admin-activity)  
- **Permissions** :
  - `bigquery.transfers.update - ADMIN_WRITE`
- **Method is a long-running or streaming operation** : No.  
- **Filter for this method** : `
  protoPayload.methodName="google.cloud.bigquery.datatransfer.v1.DataTransferService.UpdateTransferConfig"
  `  

### `google.cloud.location.Locations`

The following audit logs are associated with methods belonging to
`google.cloud.location.Locations`.

#### `GetLocation`

- **Method** : `google.cloud.location.Locations.GetLocation`  
- **Audit log type** : [Data access](https://docs.cloud.google.com/logging/docs/audit#data-access)  
- **Permissions** :
  - `bigquery.transfers.get - ADMIN_READ`
- **Method is a long-running or streaming operation** : No.  
- **Filter for this method** : `
  protoPayload.methodName="google.cloud.location.Locations.GetLocation"
  `  

#### `ListLocations`

- **Method** : `google.cloud.location.Locations.ListLocations`  
- **Audit log type** : [Data access](https://docs.cloud.google.com/logging/docs/audit#data-access)  
- **Permissions** :
  - `bigquery.transfers.get - ADMIN_READ`
- **Method is a long-running or streaming operation** : No.  
- **Filter for this method** : `
  protoPayload.methodName="google.cloud.location.Locations.ListLocations"
  `