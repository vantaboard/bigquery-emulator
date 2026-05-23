# BigQuery Connection API audit logging

This document describes audit logging for BigQuery Connection API. Google Cloud services
generate audit logs that record administrative and access activities within your Google Cloud resources.
For more information about Cloud Audit Logs, see the following:

- [Types of audit logs](https://docs.cloud.google.com/logging/docs/audit#types)
- [Audit log entry structure](https://docs.cloud.google.com/logging/docs/audit#audit_log_entry_structure)
- [Storing and routing audit logs](https://docs.cloud.google.com/logging/docs/audit#storing_and_routing_audit_logs)
- [Cloud Logging pricing summary](https://docs.cloud.google.com/stackdriver/pricing#logs-pricing-summary)
- [Enable Data Access audit logs](https://docs.cloud.google.com/logging/docs/audit/configure-data-access)

<br />

## Service name

BigQuery Connection API audit logs use the service name `bigqueryconnection.googleapis.com`.
Filter for this service:

```
    protoPayload.serviceName="bigqueryconnection.googleapis.com"
  
```

<br />

## Methods by permission type

Each IAM permission has a `type` property, whose value is an enum
that can be one of four values: `ADMIN_READ`, `ADMIN_WRITE`,
`DATA_READ`, or `DATA_WRITE`. When you call a method,
BigQuery Connection API generates an audit log whose category is dependent on the
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
| `ADMIN_READ` | `google.cloud.bigquery.connection.v1.ConnectionService.GetConnection` `google.cloud.bigquery.connection.v1.ConnectionService.GetIamPolicy` `google.cloud.bigquery.connection.v1.ConnectionService.ListConnections` `google.cloud.bigquery.connection.v1.ConnectionService.SetIamPolicy` |
| `ADMIN_WRITE` | `google.cloud.bigquery.connection.v1.ConnectionService.CreateConnection` `google.cloud.bigquery.connection.v1.ConnectionService.DeleteConnection` `google.cloud.bigquery.connection.v1.ConnectionService.UpdateConnection` |

## API interface audit logs

For information about how and which permissions are evaluated for each method,
see the Identity and Access Management documentation for BigQuery Connection API.

### `google.cloud.bigquery.connection.v1.ConnectionService`

The following audit logs are associated with methods belonging to
`google.cloud.bigquery.connection.v1.ConnectionService`.

#### `CreateConnection`

- **Method** : `google.cloud.bigquery.connection.v1.ConnectionService.CreateConnection`  
- **Audit log type** : [Admin activity](https://docs.cloud.google.com/logging/docs/audit#admin-activity)  
- **Permissions** :
  - `bigquery.connections.create - ADMIN_WRITE`
- **Method is a long-running or streaming operation** : No.  
- **Filter for this method** : `
  protoPayload.methodName="google.cloud.bigquery.connection.v1.ConnectionService.CreateConnection"
  `  

#### `DeleteConnection`

- **Method** : `google.cloud.bigquery.connection.v1.ConnectionService.DeleteConnection`  
- **Audit log type** : [Admin activity](https://docs.cloud.google.com/logging/docs/audit#admin-activity)  
- **Permissions** :
  - `bigquery.connections.delete - ADMIN_WRITE`
- **Method is a long-running or streaming operation** : No.  
- **Filter for this method** : `
  protoPayload.methodName="google.cloud.bigquery.connection.v1.ConnectionService.DeleteConnection"
  `  

#### `GetConnection`

- **Method** : `google.cloud.bigquery.connection.v1.ConnectionService.GetConnection`  
- **Audit log type** : [Data access](https://docs.cloud.google.com/logging/docs/audit#data-access)  
- **Permissions** :
  - `bigquery.connections.get - ADMIN_READ`
- **Method is a long-running or streaming operation** : No.  
- **Filter for this method** : `
  protoPayload.methodName="google.cloud.bigquery.connection.v1.ConnectionService.GetConnection"
  `  

#### `GetIamPolicy`

- **Method** : `google.cloud.bigquery.connection.v1.ConnectionService.GetIamPolicy`  
- **Audit log type** : [Data access](https://docs.cloud.google.com/logging/docs/audit#data-access)  
- **Permissions** :
  - `bigquery.connections.getIamPolicy - ADMIN_READ`
- **Method is a long-running or streaming operation** : No.  
- **Filter for this method** : `
  protoPayload.methodName="google.cloud.bigquery.connection.v1.ConnectionService.GetIamPolicy"
  `  

#### `ListConnections`

- **Method** : `google.cloud.bigquery.connection.v1.ConnectionService.ListConnections`  
- **Audit log type** : [Data access](https://docs.cloud.google.com/logging/docs/audit#data-access)  
- **Permissions** :
  - `bigquery.connections.list - ADMIN_READ`
- **Method is a long-running or streaming operation** : No.  
- **Filter for this method** : `
  protoPayload.methodName="google.cloud.bigquery.connection.v1.ConnectionService.ListConnections"
  `  

#### `SetIamPolicy`

- **Method** : `google.cloud.bigquery.connection.v1.ConnectionService.SetIamPolicy`  
- **Audit log type** : [Data access](https://docs.cloud.google.com/logging/docs/audit#data-access)  
- **Permissions** :
  - `bigquery.connections.setIamPolicy - ADMIN_READ`
- **Method is a long-running or streaming operation** : No.  
- **Filter for this method** : `
  protoPayload.methodName="google.cloud.bigquery.connection.v1.ConnectionService.SetIamPolicy"
  `  

#### `UpdateConnection`

- **Method** : `google.cloud.bigquery.connection.v1.ConnectionService.UpdateConnection`  
- **Audit log type** : [Admin activity](https://docs.cloud.google.com/logging/docs/audit#admin-activity)  
- **Permissions** :
  - `bigquery.connections.update - ADMIN_WRITE`
- **Method is a long-running or streaming operation** : No.  
- **Filter for this method** : `
  protoPayload.methodName="google.cloud.bigquery.connection.v1.ConnectionService.UpdateConnection"
  `  

## Methods that don't produce audit logs

A method might not produce audit logs for one or more of the following
reasons:

- It is a high volume method involving significant log generation and storage costs.
- It has low auditing value.
- Another audit or platform log already provides method coverage.

<br />

The following methods don't produce audit logs:

- `google.cloud.bigquery.connection.v1.ConnectionService.TestIamPermissions`
- `google.cloud.bigquery.connection.v1beta1.ConnectionService.CreateConnection`
- `google.cloud.bigquery.connection.v1beta1.ConnectionService.DeleteConnection`
- `google.cloud.bigquery.connection.v1beta1.ConnectionService.GetConnection`
- `google.cloud.bigquery.connection.v1beta1.ConnectionService.GetIamPolicy`
- `google.cloud.bigquery.connection.v1beta1.ConnectionService.ListConnections`
- `google.cloud.bigquery.connection.v1beta1.ConnectionService.SetIamPolicy`
- `google.cloud.bigquery.connection.v1beta1.ConnectionService.TestIamPermissions`
- `google.cloud.bigquery.connection.v1beta1.ConnectionService.UpdateConnection`
- `google.cloud.bigquery.connection.v1beta1.ConnectionService.UpdateConnectionCredential`