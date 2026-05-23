# BigLake audit logging

This document describes audit logging for BigLake. Google Cloud services
generate audit logs that record administrative and access activities within your Google Cloud resources.
For more information about Cloud Audit Logs, see the following:

- [Types of audit logs](https://docs.cloud.google.com/logging/docs/audit#types)
- [Audit log entry structure](https://docs.cloud.google.com/logging/docs/audit#audit_log_entry_structure)
- [Storing and routing audit logs](https://docs.cloud.google.com/logging/docs/audit#storing_and_routing_audit_logs)
- [Cloud Logging pricing summary](https://docs.cloud.google.com/stackdriver/pricing#logs-pricing-summary)
- [Enable Data Access audit logs](https://docs.cloud.google.com/logging/docs/audit/configure-data-access)

<br />

## Service name

BigLake audit logs use the service name `biglake.googleapis.com`.
Filter for this service:

```
    protoPayload.serviceName="biglake.googleapis.com"
  
```

<br />

## Methods by permission type

Each IAM permission has a `type` property, whose value is an enum
that can be one of four values: `ADMIN_READ`, `ADMIN_WRITE`,
`DATA_READ`, or `DATA_WRITE`. When you call a method,
BigLake generates an audit log whose category is dependent on the
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
| `ADMIN_READ` | `google.cloud.bigquery.biglake.v1.MetastoreService.GetCatalog` `google.cloud.bigquery.biglake.v1.MetastoreService.GetDatabase` `google.cloud.bigquery.biglake.v1.MetastoreService.GetTable` `google.cloud.bigquery.biglake.v1.MetastoreService.ListCatalogs` `google.cloud.bigquery.biglake.v1.MetastoreService.ListDatabases` `google.cloud.bigquery.biglake.v1.MetastoreService.ListTables` `google.cloud.bigquery.biglake.v1alpha1.MetastoreService.GetCatalog` `google.cloud.bigquery.biglake.v1alpha1.MetastoreService.GetDatabase` `google.cloud.bigquery.biglake.v1alpha1.MetastoreService.GetTable` `google.cloud.bigquery.biglake.v1alpha1.MetastoreService.ListCatalogs` `google.cloud.bigquery.biglake.v1alpha1.MetastoreService.ListDatabases` `google.cloud.bigquery.biglake.v1alpha1.MetastoreService.ListLocks` `google.cloud.bigquery.biglake.v1alpha1.MetastoreService.ListTables` |
| `ADMIN_WRITE` | `google.cloud.bigquery.biglake.v1.MetastoreService.CreateCatalog` `google.cloud.bigquery.biglake.v1.MetastoreService.CreateDatabase` `google.cloud.bigquery.biglake.v1.MetastoreService.CreateTable` `google.cloud.bigquery.biglake.v1.MetastoreService.DeleteCatalog` `google.cloud.bigquery.biglake.v1.MetastoreService.DeleteDatabase` `google.cloud.bigquery.biglake.v1.MetastoreService.DeleteTable` `google.cloud.bigquery.biglake.v1.MetastoreService.RenameTable` `google.cloud.bigquery.biglake.v1.MetastoreService.UpdateDatabase` `google.cloud.bigquery.biglake.v1.MetastoreService.UpdateTable` `google.cloud.bigquery.biglake.v1alpha1.MetastoreService.CheckLock` `google.cloud.bigquery.biglake.v1alpha1.MetastoreService.CreateCatalog` `google.cloud.bigquery.biglake.v1alpha1.MetastoreService.CreateDatabase` `google.cloud.bigquery.biglake.v1alpha1.MetastoreService.CreateLock` `google.cloud.bigquery.biglake.v1alpha1.MetastoreService.CreateTable` `google.cloud.bigquery.biglake.v1alpha1.MetastoreService.DeleteCatalog` `google.cloud.bigquery.biglake.v1alpha1.MetastoreService.DeleteDatabase` `google.cloud.bigquery.biglake.v1alpha1.MetastoreService.DeleteLock` `google.cloud.bigquery.biglake.v1alpha1.MetastoreService.DeleteTable` `google.cloud.bigquery.biglake.v1alpha1.MetastoreService.RenameTable` `google.cloud.bigquery.biglake.v1alpha1.MetastoreService.UpdateDatabase` `google.cloud.bigquery.biglake.v1alpha1.MetastoreService.UpdateTable` |

## API interface audit logs

For information about how and which permissions are evaluated for each method,
see the Identity and Access Management documentation for BigLake.

### `google.cloud.bigquery.biglake.v1.MetastoreService`

The following audit logs are associated with methods belonging to
`google.cloud.bigquery.biglake.v1.MetastoreService`.

#### `CreateCatalog`

- **Method** : `google.cloud.bigquery.biglake.v1.MetastoreService.CreateCatalog`  
- **Audit log type** : [Admin activity](https://docs.cloud.google.com/logging/docs/audit#admin-activity)  
- **Permissions** :
  - `biglake.catalogs.create - ADMIN_WRITE`
- **Method is a long-running or streaming operation** : No.  
- **Filter for this method** : `
  protoPayload.methodName="google.cloud.bigquery.biglake.v1.MetastoreService.CreateCatalog"
  `  

#### `CreateDatabase`

- **Method** : `google.cloud.bigquery.biglake.v1.MetastoreService.CreateDatabase`  
- **Audit log type** : [Admin activity](https://docs.cloud.google.com/logging/docs/audit#admin-activity)  
- **Permissions** :
  - `biglake.databases.create - ADMIN_WRITE`
- **Method is a long-running or streaming operation** : No.  
- **Filter for this method** : `
  protoPayload.methodName="google.cloud.bigquery.biglake.v1.MetastoreService.CreateDatabase"
  `  

#### `CreateTable`

- **Method** : `google.cloud.bigquery.biglake.v1.MetastoreService.CreateTable`  
- **Audit log type** : [Admin activity](https://docs.cloud.google.com/logging/docs/audit#admin-activity)  
- **Permissions** :
  - `biglake.tables.create - ADMIN_WRITE`
- **Method is a long-running or streaming operation** : No.  
- **Filter for this method** : `
  protoPayload.methodName="google.cloud.bigquery.biglake.v1.MetastoreService.CreateTable"
  `  

#### `DeleteCatalog`

- **Method** : `google.cloud.bigquery.biglake.v1.MetastoreService.DeleteCatalog`  
- **Audit log type** : [Admin activity](https://docs.cloud.google.com/logging/docs/audit#admin-activity)  
- **Permissions** :
  - `biglake.catalogs.delete - ADMIN_WRITE`
- **Method is a long-running or streaming operation** : No.  
- **Filter for this method** : `
  protoPayload.methodName="google.cloud.bigquery.biglake.v1.MetastoreService.DeleteCatalog"
  `  

#### `DeleteDatabase`

- **Method** : `google.cloud.bigquery.biglake.v1.MetastoreService.DeleteDatabase`  
- **Audit log type** : [Admin activity](https://docs.cloud.google.com/logging/docs/audit#admin-activity)  
- **Permissions** :
  - `biglake.databases.delete - ADMIN_WRITE`
- **Method is a long-running or streaming operation** : No.  
- **Filter for this method** : `
  protoPayload.methodName="google.cloud.bigquery.biglake.v1.MetastoreService.DeleteDatabase"
  `  

#### `DeleteTable`

- **Method** : `google.cloud.bigquery.biglake.v1.MetastoreService.DeleteTable`  
- **Audit log type** : [Admin activity](https://docs.cloud.google.com/logging/docs/audit#admin-activity)  
- **Permissions** :
  - `biglake.tables.delete - ADMIN_WRITE`
- **Method is a long-running or streaming operation** : No.  
- **Filter for this method** : `
  protoPayload.methodName="google.cloud.bigquery.biglake.v1.MetastoreService.DeleteTable"
  `  

#### `GetCatalog`

- **Method** : `google.cloud.bigquery.biglake.v1.MetastoreService.GetCatalog`  
- **Audit log type** : [Data access](https://docs.cloud.google.com/logging/docs/audit#data-access)  
- **Permissions** :
  - `biglake.catalogs.get - ADMIN_READ`
- **Method is a long-running or streaming operation** : No.  
- **Filter for this method** : `
  protoPayload.methodName="google.cloud.bigquery.biglake.v1.MetastoreService.GetCatalog"
  `  

#### `GetDatabase`

- **Method** : `google.cloud.bigquery.biglake.v1.MetastoreService.GetDatabase`  
- **Audit log type** : [Data access](https://docs.cloud.google.com/logging/docs/audit#data-access)  
- **Permissions** :
  - `biglake.databases.get - ADMIN_READ`
- **Method is a long-running or streaming operation** : No.  
- **Filter for this method** : `
  protoPayload.methodName="google.cloud.bigquery.biglake.v1.MetastoreService.GetDatabase"
  `  

#### `GetTable`

- **Method** : `google.cloud.bigquery.biglake.v1.MetastoreService.GetTable`  
- **Audit log type** : [Data access](https://docs.cloud.google.com/logging/docs/audit#data-access)  
- **Permissions** :
  - `biglake.tables.get - ADMIN_READ`
- **Method is a long-running or streaming operation** : No.  
- **Filter for this method** : `
  protoPayload.methodName="google.cloud.bigquery.biglake.v1.MetastoreService.GetTable"
  `  

#### `ListCatalogs`

- **Method** : `google.cloud.bigquery.biglake.v1.MetastoreService.ListCatalogs`  
- **Audit log type** : [Data access](https://docs.cloud.google.com/logging/docs/audit#data-access)  
- **Permissions** :
  - `biglake.catalogs.list - ADMIN_READ`
- **Method is a long-running or streaming operation** : No.  
- **Filter for this method** : `
  protoPayload.methodName="google.cloud.bigquery.biglake.v1.MetastoreService.ListCatalogs"
  `  

#### `ListDatabases`

- **Method** : `google.cloud.bigquery.biglake.v1.MetastoreService.ListDatabases`  
- **Audit log type** : [Data access](https://docs.cloud.google.com/logging/docs/audit#data-access)  
- **Permissions** :
  - `biglake.databases.list - ADMIN_READ`
- **Method is a long-running or streaming operation** : No.  
- **Filter for this method** : `
  protoPayload.methodName="google.cloud.bigquery.biglake.v1.MetastoreService.ListDatabases"
  `  

#### `ListTables`

- **Method** : `google.cloud.bigquery.biglake.v1.MetastoreService.ListTables`  
- **Audit log type** : [Data access](https://docs.cloud.google.com/logging/docs/audit#data-access)  
- **Permissions** :
  - `biglake.tables.list - ADMIN_READ`
- **Method is a long-running or streaming operation** : No.  
- **Filter for this method** : `
  protoPayload.methodName="google.cloud.bigquery.biglake.v1.MetastoreService.ListTables"
  `  

#### `RenameTable`

- **Method** : `google.cloud.bigquery.biglake.v1.MetastoreService.RenameTable`  
- **Audit log type** : [Admin activity](https://docs.cloud.google.com/logging/docs/audit#admin-activity)  
- **Permissions** :
  - `biglake.tables.update - ADMIN_WRITE`
- **Method is a long-running or streaming operation** : No.  
- **Filter for this method** : `
  protoPayload.methodName="google.cloud.bigquery.biglake.v1.MetastoreService.RenameTable"
  `  

#### `UpdateDatabase`

- **Method** : `google.cloud.bigquery.biglake.v1.MetastoreService.UpdateDatabase`  
- **Audit log type** : [Admin activity](https://docs.cloud.google.com/logging/docs/audit#admin-activity)  
- **Permissions** :
  - `biglake.databases.update - ADMIN_WRITE`
- **Method is a long-running or streaming operation** : No.  
- **Filter for this method** : `
  protoPayload.methodName="google.cloud.bigquery.biglake.v1.MetastoreService.UpdateDatabase"
  `  

#### `UpdateTable`

- **Method** : `google.cloud.bigquery.biglake.v1.MetastoreService.UpdateTable`  
- **Audit log type** : [Admin activity](https://docs.cloud.google.com/logging/docs/audit#admin-activity)  
- **Permissions** :
  - `biglake.tables.update - ADMIN_WRITE`
- **Method is a long-running or streaming operation** : No.  
- **Filter for this method** : `
  protoPayload.methodName="google.cloud.bigquery.biglake.v1.MetastoreService.UpdateTable"
  `  

### `google.cloud.bigquery.biglake.v1alpha1.MetastoreService`

The following audit logs are associated with methods belonging to
`google.cloud.bigquery.biglake.v1alpha1.MetastoreService`.

#### `CheckLock`

- **Method** : `google.cloud.bigquery.biglake.v1alpha1.MetastoreService.CheckLock`  
- **Audit log type** : [Admin activity](https://docs.cloud.google.com/logging/docs/audit#admin-activity)  
- **Permissions** :
  - `biglake.locks.check - ADMIN_WRITE`
- **Method is a long-running or streaming operation** : No.  
- **Filter for this method** : `
  protoPayload.methodName="google.cloud.bigquery.biglake.v1alpha1.MetastoreService.CheckLock"
  `  

#### `CreateCatalog`

- **Method** : `google.cloud.bigquery.biglake.v1alpha1.MetastoreService.CreateCatalog`  
- **Audit log type** : [Admin activity](https://docs.cloud.google.com/logging/docs/audit#admin-activity)  
- **Permissions** :
  - `biglake.catalogs.create - ADMIN_WRITE`
- **Method is a long-running or streaming operation** : No.  
- **Filter for this method** : `
  protoPayload.methodName="google.cloud.bigquery.biglake.v1alpha1.MetastoreService.CreateCatalog"
  `  

#### `CreateDatabase`

- **Method** : `google.cloud.bigquery.biglake.v1alpha1.MetastoreService.CreateDatabase`  
- **Audit log type** : [Admin activity](https://docs.cloud.google.com/logging/docs/audit#admin-activity)  
- **Permissions** :
  - `biglake.databases.create - ADMIN_WRITE`
- **Method is a long-running or streaming operation** : No.  
- **Filter for this method** : `
  protoPayload.methodName="google.cloud.bigquery.biglake.v1alpha1.MetastoreService.CreateDatabase"
  `  

#### `CreateLock`

- **Method** : `google.cloud.bigquery.biglake.v1alpha1.MetastoreService.CreateLock`  
- **Audit log type** : [Admin activity](https://docs.cloud.google.com/logging/docs/audit#admin-activity)  
- **Permissions** :
  - `biglake.locks.create - ADMIN_WRITE`
- **Method is a long-running or streaming operation** : No.  
- **Filter for this method** : `
  protoPayload.methodName="google.cloud.bigquery.biglake.v1alpha1.MetastoreService.CreateLock"
  `  

#### `CreateTable`

- **Method** : `google.cloud.bigquery.biglake.v1alpha1.MetastoreService.CreateTable`  
- **Audit log type** : [Admin activity](https://docs.cloud.google.com/logging/docs/audit#admin-activity)  
- **Permissions** :
  - `biglake.tables.create - ADMIN_WRITE`
- **Method is a long-running or streaming operation** : No.  
- **Filter for this method** : `
  protoPayload.methodName="google.cloud.bigquery.biglake.v1alpha1.MetastoreService.CreateTable"
  `  

#### `DeleteCatalog`

- **Method** : `google.cloud.bigquery.biglake.v1alpha1.MetastoreService.DeleteCatalog`  
- **Audit log type** : [Admin activity](https://docs.cloud.google.com/logging/docs/audit#admin-activity)  
- **Permissions** :
  - `biglake.catalogs.delete - ADMIN_WRITE`
- **Method is a long-running or streaming operation** : No.  
- **Filter for this method** : `
  protoPayload.methodName="google.cloud.bigquery.biglake.v1alpha1.MetastoreService.DeleteCatalog"
  `  

#### `DeleteDatabase`

- **Method** : `google.cloud.bigquery.biglake.v1alpha1.MetastoreService.DeleteDatabase`  
- **Audit log type** : [Admin activity](https://docs.cloud.google.com/logging/docs/audit#admin-activity)  
- **Permissions** :
  - `biglake.databases.delete - ADMIN_WRITE`
- **Method is a long-running or streaming operation** : No.  
- **Filter for this method** : `
  protoPayload.methodName="google.cloud.bigquery.biglake.v1alpha1.MetastoreService.DeleteDatabase"
  `  

#### `DeleteLock`

- **Method** : `google.cloud.bigquery.biglake.v1alpha1.MetastoreService.DeleteLock`  
- **Audit log type** : [Admin activity](https://docs.cloud.google.com/logging/docs/audit#admin-activity)  
- **Permissions** :
  - `biglake.locks.delete - ADMIN_WRITE`
- **Method is a long-running or streaming operation** : No.  
- **Filter for this method** : `
  protoPayload.methodName="google.cloud.bigquery.biglake.v1alpha1.MetastoreService.DeleteLock"
  `  

#### `DeleteTable`

- **Method** : `google.cloud.bigquery.biglake.v1alpha1.MetastoreService.DeleteTable`  
- **Audit log type** : [Admin activity](https://docs.cloud.google.com/logging/docs/audit#admin-activity)  
- **Permissions** :
  - `biglake.tables.delete - ADMIN_WRITE`
- **Method is a long-running or streaming operation** : No.  
- **Filter for this method** : `
  protoPayload.methodName="google.cloud.bigquery.biglake.v1alpha1.MetastoreService.DeleteTable"
  `  

#### `GetCatalog`

- **Method** : `google.cloud.bigquery.biglake.v1alpha1.MetastoreService.GetCatalog`  
- **Audit log type** : [Data access](https://docs.cloud.google.com/logging/docs/audit#data-access)  
- **Permissions** :
  - `biglake.catalogs.get - ADMIN_READ`
- **Method is a long-running or streaming operation** : No.  
- **Filter for this method** : `
  protoPayload.methodName="google.cloud.bigquery.biglake.v1alpha1.MetastoreService.GetCatalog"
  `  

#### `GetDatabase`

- **Method** : `google.cloud.bigquery.biglake.v1alpha1.MetastoreService.GetDatabase`  
- **Audit log type** : [Data access](https://docs.cloud.google.com/logging/docs/audit#data-access)  
- **Permissions** :
  - `biglake.databases.get - ADMIN_READ`
- **Method is a long-running or streaming operation** : No.  
- **Filter for this method** : `
  protoPayload.methodName="google.cloud.bigquery.biglake.v1alpha1.MetastoreService.GetDatabase"
  `  

#### `GetTable`

- **Method** : `google.cloud.bigquery.biglake.v1alpha1.MetastoreService.GetTable`  
- **Audit log type** : [Data access](https://docs.cloud.google.com/logging/docs/audit#data-access)  
- **Permissions** :
  - `biglake.tables.get - ADMIN_READ`
- **Method is a long-running or streaming operation** : No.  
- **Filter for this method** : `
  protoPayload.methodName="google.cloud.bigquery.biglake.v1alpha1.MetastoreService.GetTable"
  `  

#### `ListCatalogs`

- **Method** : `google.cloud.bigquery.biglake.v1alpha1.MetastoreService.ListCatalogs`  
- **Audit log type** : [Data access](https://docs.cloud.google.com/logging/docs/audit#data-access)  
- **Permissions** :
  - `biglake.catalogs.list - ADMIN_READ`
- **Method is a long-running or streaming operation** : No.  
- **Filter for this method** : `
  protoPayload.methodName="google.cloud.bigquery.biglake.v1alpha1.MetastoreService.ListCatalogs"
  `  

#### `ListDatabases`

- **Method** : `google.cloud.bigquery.biglake.v1alpha1.MetastoreService.ListDatabases`  
- **Audit log type** : [Data access](https://docs.cloud.google.com/logging/docs/audit#data-access)  
- **Permissions** :
  - `biglake.databases.list - ADMIN_READ`
- **Method is a long-running or streaming operation** : No.  
- **Filter for this method** : `
  protoPayload.methodName="google.cloud.bigquery.biglake.v1alpha1.MetastoreService.ListDatabases"
  `  

#### `ListLocks`

- **Method** : `google.cloud.bigquery.biglake.v1alpha1.MetastoreService.ListLocks`  
- **Audit log type** : [Data access](https://docs.cloud.google.com/logging/docs/audit#data-access)  
- **Permissions** :
  - `biglake.locks.list - ADMIN_READ`
- **Method is a long-running or streaming operation** : No.  
- **Filter for this method** : `
  protoPayload.methodName="google.cloud.bigquery.biglake.v1alpha1.MetastoreService.ListLocks"
  `  

#### `ListTables`

- **Method** : `google.cloud.bigquery.biglake.v1alpha1.MetastoreService.ListTables`  
- **Audit log type** : [Data access](https://docs.cloud.google.com/logging/docs/audit#data-access)  
- **Permissions** :
  - `biglake.tables.list - ADMIN_READ`
- **Method is a long-running or streaming operation** : No.  
- **Filter for this method** : `
  protoPayload.methodName="google.cloud.bigquery.biglake.v1alpha1.MetastoreService.ListTables"
  `  

#### `RenameTable`

- **Method** : `google.cloud.bigquery.biglake.v1alpha1.MetastoreService.RenameTable`  
- **Audit log type** : [Admin activity](https://docs.cloud.google.com/logging/docs/audit#admin-activity)  
- **Permissions** :
  - `biglake.tables.update - ADMIN_WRITE`
- **Method is a long-running or streaming operation** : No.  
- **Filter for this method** : `
  protoPayload.methodName="google.cloud.bigquery.biglake.v1alpha1.MetastoreService.RenameTable"
  `  

#### `UpdateDatabase`

- **Method** : `google.cloud.bigquery.biglake.v1alpha1.MetastoreService.UpdateDatabase`  
- **Audit log type** : [Admin activity](https://docs.cloud.google.com/logging/docs/audit#admin-activity)  
- **Permissions** :
  - `biglake.databases.update - ADMIN_WRITE`
- **Method is a long-running or streaming operation** : No.  
- **Filter for this method** : `
  protoPayload.methodName="google.cloud.bigquery.biglake.v1alpha1.MetastoreService.UpdateDatabase"
  `  

#### `UpdateTable`

- **Method** : `google.cloud.bigquery.biglake.v1alpha1.MetastoreService.UpdateTable`  
- **Audit log type** : [Admin activity](https://docs.cloud.google.com/logging/docs/audit#admin-activity)  
- **Permissions** :
  - `biglake.tables.update - ADMIN_WRITE`
- **Method is a long-running or streaming operation** : No.  
- **Filter for this method** : `
  protoPayload.methodName="google.cloud.bigquery.biglake.v1alpha1.MetastoreService.UpdateTable"
  `