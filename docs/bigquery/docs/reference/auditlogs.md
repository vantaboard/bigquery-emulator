# BigQuery audit logs overview

Cloud Audit Logs are a collection of logs provided by Google Cloud
that provide insight into operational concerns related to your use of
Google Cloud services. This page provides details about
BigQuery specific log information, and it demonstrates how
to use BigQuery to analyze logged activity. For more information, see
[Introduction to audit logs in BigQuery](https://docs.cloud.google.com/bigquery/docs/introduction-audit-workloads).

This document lists the audited methods for BigQuery. Google Cloud services
generate audit logs that record administrative and access activities within your Google Cloud resources.
For more information about Cloud Audit Logs, see the following:

- [Types of audit logs](https://docs.cloud.google.com/logging/docs/audit#types)
- [Audit log entry structure](https://docs.cloud.google.com/logging/docs/audit#audit_log_entry_structure)
- [Storing and routing audit logs](https://docs.cloud.google.com/logging/docs/audit#storing_and_routing_audit_logs)
- [Cloud Logging pricing summary](https://docs.cloud.google.com/stackdriver/pricing#logs-pricing-summary)
- [Enable Data Access audit logs](https://docs.cloud.google.com/logging/docs/audit/configure-data-access)

<br />

## Service name

To view the BigQuery audit logs, do the following:

1. In the Google Cloud console, go to the Logs Explorer page:

   [Go to Logs Explorer](https://console.cloud.google.com/logs/query)
2. Copy and paste the following query into the **Query** field of the
   Logs Explorer, and then click **Run query**.

   ```
       protoPayload.serviceName="bigquery.googleapis.com"
     
   ```

## Methods by permission type

Each IAM permission has a `type` property, whose value is an enum
that can be one of four values: `ADMIN_READ`, `ADMIN_WRITE`,
`DATA_READ`, or `DATA_WRITE`. When you call a method,
BigQuery generates an audit log whose category is dependent on the
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
| `ADMIN_WRITE` | `datasetservice.delete` `datasetservice.insert` `datasetservice.update` `google.cloud.bigquery.v2.DatasetService.DeleteDataset` `google.cloud.bigquery.v2.DatasetService.InsertDataset` `google.cloud.bigquery.v2.DatasetService.PatchDataset` `google.cloud.bigquery.v2.DatasetService.UndeleteDataset` `google.cloud.bigquery.v2.DatasetService.UpdateDataset` `google.cloud.bigquery.v2.JobService.InsertJob` (LRO) `google.cloud.bigquery.v2.JobService.Query` (LRO) `google.cloud.bigquery.v2.RoutineService.DeleteRoutine` `google.cloud.bigquery.v2.RoutineService.InsertRoutine` `google.cloud.bigquery.v2.RowAccessPolicyService.CreateRowAccessPolicy` `google.cloud.bigquery.v2.RowAccessPolicyService.DeleteRowAccessPolicy` `google.cloud.bigquery.v2.TableService.DeleteTable` `google.cloud.bigquery.v2.TableService.InsertTable` `google.cloud.bigquery.v2.TableService.PatchTable` `google.cloud.bigquery.v2.TableService.UpdateTable` `google.iam.v1.IAMPolicy.SetIamPolicy` `jobservice.getqueryresults` `jobservice.insert` `jobservice.query` `tabledataservice.list` `tableservice.delete` `tableservice.insert` `tableservice.update` |
| `DATA_READ` | `google.cloud.bigquery.v2.JobService.GetQueryResults` `google.cloud.bigquery.v2.TableDataService.List` |

## API interface audit logs

For information about how and which permissions are evaluated for each method,
see the Identity and Access Management documentation for BigQuery.

### `datasetservice`

The following audit logs are associated with methods belonging to
`datasetservice`.

#### `delete`

- **Method** : `datasetservice.delete`  
- **Audit log type** : [Admin activity](https://docs.cloud.google.com/logging/docs/audit#admin-activity)  
- **Permissions** :
  - `bigquery.datasets.delete - ADMIN_WRITE`
  - `bigquery.models.delete - ADMIN_WRITE`
  - `bigquery.routines.delete - ADMIN_WRITE`
  - `bigquery.tables.delete - ADMIN_WRITE`
- **Method is a long-running or streaming operation** : No.  
- **Filter for this method** : `
  protoPayload.methodName="datasetservice.delete"
  `  

#### `insert`

- **Method** : `datasetservice.insert`  
- **Audit log type** : [Admin activity](https://docs.cloud.google.com/logging/docs/audit#admin-activity)  
- **Permissions** :
  - `bigquery.datasets.create - ADMIN_WRITE`
- **Method is a long-running or streaming operation** : No.  
- **Filter for this method** : `
  protoPayload.methodName="datasetservice.insert"
  `  

#### `update`

- **Method** : `datasetservice.update`  
- **Audit log type** : [Admin activity](https://docs.cloud.google.com/logging/docs/audit#admin-activity)  
- **Permissions** :
  - `bigquery.datasets.update - ADMIN_WRITE`
- **Method is a long-running or streaming operation** : No.  
- **Filter for this method** : `
  protoPayload.methodName="datasetservice.update"
  `  

### `google.cloud.bigquery.v2.DatasetService`

The following audit logs are associated with methods belonging to
`google.cloud.bigquery.v2.DatasetService`.

#### `DeleteDataset`

- **Method** : `google.cloud.bigquery.v2.DatasetService.DeleteDataset`  
- **Audit log type** : [Admin activity](https://docs.cloud.google.com/logging/docs/audit#admin-activity)  
- **Permissions** :
  - `bigquery.datasets.delete - ADMIN_WRITE`
- **Method is a long-running or streaming operation** : No.  
- **Filter for this method** : `
  protoPayload.methodName="google.cloud.bigquery.v2.DatasetService.DeleteDataset"
  `  

#### `InsertDataset`

- **Method** : `google.cloud.bigquery.v2.DatasetService.InsertDataset`  
- **Audit log type** : [Admin activity](https://docs.cloud.google.com/logging/docs/audit#admin-activity)  
- **Permissions** :
  - `bigquery.datasets.create - ADMIN_WRITE`
- **Method is a long-running or streaming operation** : No.  
- **Filter for this method** : `
  protoPayload.methodName="google.cloud.bigquery.v2.DatasetService.InsertDataset"
  `  

#### `PatchDataset`

- **Method** : `google.cloud.bigquery.v2.DatasetService.PatchDataset`  
- **Audit log type** : [Admin activity](https://docs.cloud.google.com/logging/docs/audit#admin-activity)  
- **Permissions** :
  - `bigquery.datasets.update - ADMIN_WRITE`
- **Method is a long-running or streaming operation** : No.  
- **Filter for this method** : `
  protoPayload.methodName="google.cloud.bigquery.v2.DatasetService.PatchDataset"
  `  

#### `UndeleteDataset`

- **Method** : `google.cloud.bigquery.v2.DatasetService.UndeleteDataset`  
- **Audit log type** : [Admin activity](https://docs.cloud.google.com/logging/docs/audit#admin-activity)  
- **Permissions** :
  - `bigquery.datasets.create - ADMIN_WRITE`
  - `bigquery.datasets.get - ADMIN_READ`
- **Method is a long-running or streaming operation** : No.  
- **Filter for this method** : `
  protoPayload.methodName="google.cloud.bigquery.v2.DatasetService.UndeleteDataset"
  `  

#### `UpdateDataset`

- **Method** : `google.cloud.bigquery.v2.DatasetService.UpdateDataset`  
- **Audit log type** : [Admin activity](https://docs.cloud.google.com/logging/docs/audit#admin-activity)  
- **Permissions** :
  - `bigquery.datasets.update - ADMIN_WRITE`
- **Method is a long-running or streaming operation** : No.  
- **Filter for this method** : `
  protoPayload.methodName="google.cloud.bigquery.v2.DatasetService.UpdateDataset"
  `  

### `google.cloud.bigquery.v2.JobService`

The following audit logs are associated with methods belonging to
`google.cloud.bigquery.v2.JobService`.

#### `DeleteJob`

- **Method** : `google.cloud.bigquery.v2.JobService.DeleteJob`  
- **Audit log type** : [Data access](https://docs.cloud.google.com/logging/docs/audit#data-access)  
- **Permissions** :
- **Method is a long-running or streaming operation** : No.  
- **Filter for this method** : `
  protoPayload.methodName="google.cloud.bigquery.v2.JobService.DeleteJob"
  `  

#### `GetQueryResults`

- **Method** : `google.cloud.bigquery.v2.JobService.GetQueryResults`  
- **Audit log type** : [Data access](https://docs.cloud.google.com/logging/docs/audit#data-access)  
- **Permissions** :
  - `bigquery.tables.getData - DATA_READ`
- **Method is a long-running or streaming operation** : No.  
- **Filter for this method** : `
  protoPayload.methodName="google.cloud.bigquery.v2.JobService.GetQueryResults"
  `  

#### `InsertJob`

- **Method** : `google.cloud.bigquery.v2.JobService.InsertJob`  
- **Audit log type** : [Data access](https://docs.cloud.google.com/logging/docs/audit#data-access)  
- **Permissions** :
  - `bigquery.jobs.create - ADMIN_WRITE`
  - `bigquery.tables.getData - DATA_READ`
  - `bigquery.tables.updateData - DATA_WRITE`
- **Method is a long-running or streaming operation** : [**Long-running operation**](https://docs.cloud.google.com/logging/docs/audit/understanding-audit-logs#lro)   
- **Filter for this method** : `
  protoPayload.methodName="google.cloud.bigquery.v2.JobService.InsertJob"
  `  

#### `Query`

- **Method** : `google.cloud.bigquery.v2.JobService.Query`  
- **Audit log type** : [Data access](https://docs.cloud.google.com/logging/docs/audit#data-access)  
- **Permissions** :
  - `bigquery.jobs.create - ADMIN_WRITE`
  - `bigquery.tables.getData - DATA_READ`
- **Method is a long-running or streaming operation** : [**Long-running operation**](https://docs.cloud.google.com/logging/docs/audit/understanding-audit-logs#lro)   
- **Filter for this method** : `
  protoPayload.methodName="google.cloud.bigquery.v2.JobService.Query"
  `  

### `google.cloud.bigquery.v2.ModelService`

The following audit logs are associated with methods belonging to
`google.cloud.bigquery.v2.ModelService`.

#### `DeleteModel`

- **Method** : `google.cloud.bigquery.v2.ModelService.DeleteModel`  
- **Audit log type** : [Admin activity](https://docs.cloud.google.com/logging/docs/audit#admin-activity)  
- **Permissions** :
- **Method is a long-running or streaming operation** : No.  
- **Filter for this method** : `
  protoPayload.methodName="google.cloud.bigquery.v2.ModelService.DeleteModel"
  `  

#### `PatchModel`

- **Method** : `google.cloud.bigquery.v2.ModelService.PatchModel`  
- **Audit log type** : [Admin activity](https://docs.cloud.google.com/logging/docs/audit#admin-activity)  
- **Permissions** :
- **Method is a long-running or streaming operation** : No.  
- **Filter for this method** : `
  protoPayload.methodName="google.cloud.bigquery.v2.ModelService.PatchModel"
  `  

### `google.cloud.bigquery.v2.RoutineService`

The following audit logs are associated with methods belonging to
`google.cloud.bigquery.v2.RoutineService`.

#### `DeleteRoutine`

- **Method** : `google.cloud.bigquery.v2.RoutineService.DeleteRoutine`  
- **Audit log type** : [Admin activity](https://docs.cloud.google.com/logging/docs/audit#admin-activity)  
- **Permissions** :
  - `bigquery.routines.delete - ADMIN_WRITE`
- **Method is a long-running or streaming operation** : No.  
- **Filter for this method** : `
  protoPayload.methodName="google.cloud.bigquery.v2.RoutineService.DeleteRoutine"
  `  

#### `InsertRoutine`

- **Method** : `google.cloud.bigquery.v2.RoutineService.InsertRoutine`  
- **Audit log type** : [Admin activity](https://docs.cloud.google.com/logging/docs/audit#admin-activity)  
- **Permissions** :
  - `bigquery.routines.create - ADMIN_WRITE`
- **Method is a long-running or streaming operation** : No.  
- **Filter for this method** : `
  protoPayload.methodName="google.cloud.bigquery.v2.RoutineService.InsertRoutine"
  `  

#### `UpdateRoutine`

- **Method** : `google.cloud.bigquery.v2.RoutineService.UpdateRoutine`  
- **Audit log type** : [Admin activity](https://docs.cloud.google.com/logging/docs/audit#admin-activity)  
- **Permissions** :
- **Method is a long-running or streaming operation** : No.  
- **Filter for this method** : `
  protoPayload.methodName="google.cloud.bigquery.v2.RoutineService.UpdateRoutine"
  `  

### `google.cloud.bigquery.v2.RowAccessPolicyService`

The following audit logs are associated with methods belonging to
`google.cloud.bigquery.v2.RowAccessPolicyService`.

#### `BatchDeleteRowAccessPolicies`

- **Method** : `google.cloud.bigquery.v2.RowAccessPolicyService.BatchDeleteRowAccessPolicies`  
- **Audit log type** : [Admin activity](https://docs.cloud.google.com/logging/docs/audit#admin-activity)  
- **Permissions** :
- **Method is a long-running or streaming operation** : No.  
- **Filter for this method** : `
  protoPayload.methodName="google.cloud.bigquery.v2.RowAccessPolicyService.BatchDeleteRowAccessPolicies"
  `  

#### `CreateRowAccessPolicy`

- **Method** : `google.cloud.bigquery.v2.RowAccessPolicyService.CreateRowAccessPolicy`  
- **Audit log type** : [Admin activity](https://docs.cloud.google.com/logging/docs/audit#admin-activity)  
- **Permissions** :
  - `bigquery.rowAccessPolicies.create - ADMIN_WRITE`
  - `bigquery.rowAccessPolicies.setIamPolicy - ADMIN_WRITE`
- **Method is a long-running or streaming operation** : No.  
- **Filter for this method** : `
  protoPayload.methodName="google.cloud.bigquery.v2.RowAccessPolicyService.CreateRowAccessPolicy"
  `  

#### `DeleteRowAccessPolicy`

- **Method** : `google.cloud.bigquery.v2.RowAccessPolicyService.DeleteRowAccessPolicy`  
- **Audit log type** : [Admin activity](https://docs.cloud.google.com/logging/docs/audit#admin-activity)  
- **Permissions** :
  - `bigquery.rowAccessPolicies.delete - ADMIN_WRITE`
  - `bigquery.rowAccessPolicies.setIamPolicy - ADMIN_WRITE`
- **Method is a long-running or streaming operation** : No.  
- **Filter for this method** : `
  protoPayload.methodName="google.cloud.bigquery.v2.RowAccessPolicyService.DeleteRowAccessPolicy"
  `  

#### `UpdateRowAccessPolicy`

- **Method** : `google.cloud.bigquery.v2.RowAccessPolicyService.UpdateRowAccessPolicy`  
- **Audit log type** : [Admin activity](https://docs.cloud.google.com/logging/docs/audit#admin-activity)  
- **Permissions** :
  - `bigquery.rowAccessPolicies.setIamPolicy - PERMISSION_TYPE_UNSPECIFIED`
  - `bigquery.rowAccessPolicies.update - PERMISSION_TYPE_UNSPECIFIED`
- **Method is a long-running or streaming operation** : No.  
- **Filter for this method** : `
  protoPayload.methodName="google.cloud.bigquery.v2.RowAccessPolicyService.UpdateRowAccessPolicy"
  `  

### `google.cloud.bigquery.v2.TableDataService`

The following audit logs are associated with methods belonging to
`google.cloud.bigquery.v2.TableDataService`.

#### `List`

- **Method** : `google.cloud.bigquery.v2.TableDataService.List`  
- **Audit log type** : [Data access](https://docs.cloud.google.com/logging/docs/audit#data-access)  
- **Permissions** :
  - `bigquery.tables.getData - DATA_READ`
- **Method is a long-running or streaming operation** : No.  
- **Filter for this method** : `
  protoPayload.methodName="google.cloud.bigquery.v2.TableDataService.List"
  `  

### `google.cloud.bigquery.v2.TableService`

The following audit logs are associated with methods belonging to
`google.cloud.bigquery.v2.TableService`.

#### `DeleteTable`

- **Method** : `google.cloud.bigquery.v2.TableService.DeleteTable`  
- **Audit log type** : [Admin activity](https://docs.cloud.google.com/logging/docs/audit#admin-activity)  
- **Permissions** :
  - `bigquery.tables.delete - ADMIN_WRITE`
- **Method is a long-running or streaming operation** : No.  
- **Filter for this method** : `
  protoPayload.methodName="google.cloud.bigquery.v2.TableService.DeleteTable"
  `  

#### `InsertTable`

- **Method** : `google.cloud.bigquery.v2.TableService.InsertTable`  
- **Audit log type** : [Admin activity](https://docs.cloud.google.com/logging/docs/audit#admin-activity)  
- **Permissions** :
  - `bigquery.tables.create - ADMIN_WRITE`
- **Method is a long-running or streaming operation** : No.  
- **Filter for this method** : `
  protoPayload.methodName="google.cloud.bigquery.v2.TableService.InsertTable"
  `  

#### `PatchTable`

- **Method** : `google.cloud.bigquery.v2.TableService.PatchTable`  
- **Audit log type** : [Admin activity](https://docs.cloud.google.com/logging/docs/audit#admin-activity)  
- **Permissions** :
  - `bigquery.tables.update - ADMIN_WRITE`
- **Method is a long-running or streaming operation** : No.  
- **Filter for this method** : `
  protoPayload.methodName="google.cloud.bigquery.v2.TableService.PatchTable"
  `  

#### `UpdateTable`

- **Method** : `google.cloud.bigquery.v2.TableService.UpdateTable`  
- **Audit log type** : [Admin activity](https://docs.cloud.google.com/logging/docs/audit#admin-activity)  
- **Permissions** :
  - `bigquery.tables.update - ADMIN_WRITE`
- **Method is a long-running or streaming operation** : No.  
- **Filter for this method** : `
  protoPayload.methodName="google.cloud.bigquery.v2.TableService.UpdateTable"
  `  

### `google.iam.v1.IAMPolicy`

The following audit logs are associated with methods belonging to
`google.iam.v1.IAMPolicy`.

#### `SetIamPolicy`

- **Method** : `google.iam.v1.IAMPolicy.SetIamPolicy`  
- **Audit log type** : [Admin activity](https://docs.cloud.google.com/logging/docs/audit#admin-activity)  
- **Permissions** :
  - `bigquery.tables.setIamPolicy - ADMIN_WRITE`
- **Method is a long-running or streaming operation** : No.  
- **Filter for this method** : `
  protoPayload.methodName="google.iam.v1.IAMPolicy.SetIamPolicy"
  `  

### `jobservice`

The following audit logs are associated with methods belonging to
`jobservice`.

#### `cancel`

- **Method** : `jobservice.cancel`  
- **Audit log type** : [Data access](https://docs.cloud.google.com/logging/docs/audit#data-access)  
- **Permissions** :
  - `bigquery.jobs.create - ADMIN_WRITE`
- **Method is a long-running or streaming operation** : No.  
- **Filter for this method** : `
  protoPayload.methodName="jobservice.cancel"
  `  

#### `getqueryresults`

- **Method** : `jobservice.getqueryresults`  
- **Audit log type** : [Data access](https://docs.cloud.google.com/logging/docs/audit#data-access)  
- **Permissions** :
  - `bigquery.jobs.create - ADMIN_WRITE`
  - `bigquery.tables.getData - DATA_READ`
- **Method is a long-running or streaming operation** : No.  
- **Filter for this method** : `
  protoPayload.methodName="jobservice.getqueryresults"
  `  

#### `insert`

- **Method** : `jobservice.insert`  
- **Audit log type** : [Data access](https://docs.cloud.google.com/logging/docs/audit#data-access)  
- **Permissions** :
  - `bigquery.jobs.create - ADMIN_WRITE`
  - `bigquery.tables.create - ADMIN_WRITE`
  - `bigquery.tables.getData - DATA_READ`
  - `bigquery.tables.update - ADMIN_WRITE`
  - `bigquery.tables.updateData - DATA_WRITE`
- **Method is a long-running or streaming operation** : No.  
- **Filter for this method** : `
  protoPayload.methodName="jobservice.insert"
  `  

#### `query`

- **Method** : `jobservice.query`  
- **Audit log type** : [Data access](https://docs.cloud.google.com/logging/docs/audit#data-access)  
- **Permissions** :
  - `bigquery.jobs.create - ADMIN_WRITE`
  - `bigquery.tables.getData - DATA_READ`
- **Method is a long-running or streaming operation** : No.  
- **Filter for this method** : `
  protoPayload.methodName="jobservice.query"
  `  

### `tabledataservice`

The following audit logs are associated with methods belonging to
`tabledataservice`.

#### `list`

- **Method** : `tabledataservice.list`  
- **Audit log type** : [Data access](https://docs.cloud.google.com/logging/docs/audit#data-access)  
- **Permissions** :
  - `bigquery.jobs.create - ADMIN_WRITE`
  - `bigquery.tables.getData - DATA_READ`
- **Method is a long-running or streaming operation** : No.  
- **Filter for this method** : `
  protoPayload.methodName="tabledataservice.list"
  `  

### `tableservice`

The following audit logs are associated with methods belonging to
`tableservice`.

#### `delete`

- **Method** : `tableservice.delete`  
- **Audit log type** : [Admin activity](https://docs.cloud.google.com/logging/docs/audit#admin-activity)  
- **Permissions** :
  - `bigquery.tables.delete - ADMIN_WRITE`
- **Method is a long-running or streaming operation** : No.  
- **Filter for this method** : `
  protoPayload.methodName="tableservice.delete"
  `  

#### `insert`

- **Method** : `tableservice.insert`  
- **Audit log type** : [Admin activity](https://docs.cloud.google.com/logging/docs/audit#admin-activity)  
- **Permissions** :
  - `bigquery.tables.create - ADMIN_WRITE`
- **Method is a long-running or streaming operation** : No.  
- **Filter for this method** : `
  protoPayload.methodName="tableservice.insert"
  `  

#### `update`

- **Method** : `tableservice.update`  
- **Audit log type** : [Admin activity](https://docs.cloud.google.com/logging/docs/audit#admin-activity)  
- **Permissions** :
  - `bigquery.tables.delete - ADMIN_WRITE`
  - `bigquery.tables.update - ADMIN_WRITE`
- **Method is a long-running or streaming operation** : No.  
- **Filter for this method** : `
  protoPayload.methodName="tableservice.update"
  `  

## System events

System Event audit logs are generated by GCP systems, not
direct user action. For more information, see
[System Event audit logs](https://docs.cloud.google.com/logging/docs/audit#system-event).

| Method Name | Filter For This Event | Notes |
|---|---|---|
| InternalTableExpired | ` protoPayload.methodName="InternalTableExpired" ` |   |

## Methods that don't produce audit logs

A method might not produce audit logs for one or more of the following
reasons:

- It is a high volume method involving significant log generation and storage costs.
- It has low auditing value.
- Another audit or platform log already provides method coverage.

<br />

The following methods don't produce audit logs:

- `google.cloud.bigquery.v2.DatasetService.GetDataset`
- `google.cloud.bigquery.v2.DatasetService.ListDatasets`
- `google.cloud.bigquery.v2.JobService.GetJob`
- `google.cloud.bigquery.v2.JobService.ListJobs`
- `google.cloud.bigquery.v2.ModelService.GetModel`
- `google.cloud.bigquery.v2.ModelService.ListModels`
- `google.cloud.bigquery.v2.ProjectService.GetServiceAccount`
- `google.cloud.bigquery.v2.ProjectService.ListProjects`
- `google.cloud.bigquery.v2.RoutineService.GetRoutine`
- `google.cloud.bigquery.v2.RoutineService.ListRoutines`
- `google.cloud.bigquery.v2.RowAccessPolicyService.GetRowAccessPolicy`
- `google.cloud.bigquery.v2.RowAccessPolicyService.ListRowAccessPolicies`
- `google.cloud.bigquery.v2.TableDataService.InsertAll`
- `google.cloud.bigquery.v2.TableService.GetTable`
- `google.cloud.bigquery.v2.TableService.ListTables`
- `google.iam.v1.IAMPolicy.GetIamPolicy`

## Versions

The audit log message system relies on structured logs, and the
BigQuery service provides several types of messages:

- `AuditData`: The old version of logs, which
  reports API invocations.

- [`BigQueryAuditMetadata`](https://docs.cloud.google.com/bigquery/docs/reference/auditlogs#bigqueryauditmetadata_examples):
  The new version of logs, which reports resource interactions such as
  which tables were read from and written to by a given query job
  and which tables expired due to having an expiration time configured.

- `AuditLog`: The log format used when reporting requests.

## Limitation

Log messages have a size limit of 100K bytes.
For more information, see [Truncated log entry](https://docs.cloud.google.com/bigquery/docs/reference/auditlogs#truncated_log_entry).

## Message Formats

### AuditData format

The [AuditData](https://docs.cloud.google.com/bigquery/docs/reference/auditlogs/rest/Shared.Types/AuditData)
messages are communicated within the `protoPayload.serviceData` submessage
within the Cloud Logging
[LogEntry](https://docs.cloud.google.com/logging/docs/reference/v2/rest/v2/LogEntry) message. AuditData
payload returns `resource.type` set to `bigquery_resource`, not
`bigquery_dataset`.

### BigQueryAuditMetadata format

You can find
[BigQueryAuditMetadata](https://docs.cloud.google.com/bigquery/docs/reference/auditlogs/rest/Shared.Types/BigQueryAuditMetadata)
details in the `protoPayload.metadata` submessage that is in the
Cloud Logging [LogEntry](https://docs.cloud.google.com/logging/docs/reference/v2/rest/v2/LogEntry) message.

In the Cloud Logging logs, the `protoPayload.serviceData` information is
not set or used. In `BigQueryAuditMetadata` messages, there is more information:

- `resource.type` is set to one of the following values:

  - `bigquery_dataset` for operations to datasets such as `google.cloud.bigquery.v2.DatasetService.*`
    - `resource.labels.dataset_id` contains the encapsulating dataset.
  - `bigquery_project` for all other called methods, such as jobs
    - `resource.labels.location` contains the location of the job.
- `protoPayload.methodName` is set to one of the following values:

  - `google.cloud.bigquery.v2.TableService.InsertTable`
  - `google.cloud.bigquery.v2.TableService.UpdateTable`
  - `google.cloud.bigquery.v2.TableService.PatchTable`
  - `google.cloud.bigquery.v2.TableService.DeleteTable`
  - `google.cloud.bigquery.v2.DatasetService.InsertDataset`
  - `google.cloud.bigquery.v2.DatasetService.UpdateDataset`
  - `google.cloud.bigquery.v2.DatasetService.PatchDataset`
  - `google.cloud.bigquery.v2.DatasetService.DeleteDataset`
  - `google.cloud.bigquery.v2.TableDataService.List`
  - `google.cloud.bigquery.v2.JobService.InsertJob`
  - `google.cloud.bigquery.v2.JobService.Query`
  - `google.cloud.bigquery.v2.JobService.GetQueryResults`
  - `InternalTableExpired`
- `protoPayload.resourceName` now contains the URI for the referenced
  resource. For example, a table created by using an insert job reports
  the resource URI of the table. The earlier format reported the API resource
  (the job identifier).

- `protoPayload.authorizationInfo` only includes information relevant to the
  specific event. With earlier AuditData messages, you could merge multiple
  records when source and destination tables were in the same dataset in
  a query job.

### AuditLog format

[BigQuery Storage API](https://docs.cloud.google.com/bigquery/docs/reference/storage) uses the
[`AuditLog`](https://docs.cloud.google.com/logging/docs/reference/audit/auditlog/rest/Shared.Types/AuditLog)
format when reporting requests. Logs contain information such as:

- `resource.type` is set to:

  - `bigquery_dataset` for `CreateReadSession`.
  - `bigquery_table` for `ReadRows`, `SplitReadStream` and `AppendRows`.
- `protoPayload.methodName` is set to one of the following values:

  - `google.cloud.bigquery.storage.v1.BigQueryRead.CreateReadSession`
  - `google.cloud.bigquery.storage.v1beta1.BigQueryStorage.CreateReadSession`
  - `google.cloud.bigquery.storage.v1beta2.BigQueryRead.CreateReadSession`
  - `google.cloud.bigquery.storage.v1.BigQueryRead.ReadRows`
  - `google.cloud.bigquery.storage.v1.BigQueryRead.SplitReadStream`
  - `google.cloud.bigquery.storage.v1.BigQueryWrite.AppendRows`

## Mapping audit entries to log streams

Audit logs are organized into the following three streams. For more
information about the streams, see the
[Cloud Audit Logs](https://docs.cloud.google.com/logging/docs/audit) documentation.

- Data access
- System event
- Admin activity

### Data access (data_access)

The `data_access` stream contains entries about jobs by using the
`JobInsertion` and `JobChange` events and about table data modifications
by using the `TableDataChange` and `TableDataRead` events. `TableDataChange`
and `TableDataRead` events have a `resource.type` value of `bigquery_dataset`.

For example, when a load job appends data to a table, the `data_access` stream
adds a `TableDataChange` event. A `TableDataRead` event indicates when
a consumer reads a table.

**Note:** BigQuery does not emit data access log entries
in the following scenarios:

- If a job fails before or during execution, `TableDataChange` and
  `TableDataRead` events are not logged.

- Data appended to a table using the legacy streaming API or the Storage Write
  API does not generate `TableDataChange` log entries.

- Recursive dataset deletions, such as removing a dataset and its contents in a
  single API call, don't yield deletion entries for each resource contained in
  the dataset. The dataset removal is present in the activity log.

- Partitioned tables don't generate `TableDataChange` entries for partition
  expirations.

- Wildcard tables access generates a single `TableDataRead` entry and doesn't
  write a separate entry for each queried table.

### System event (system_event)

You can set an expiration time on tables to remove them at a specified time.
The `system_event` stream reports a `TableDeletion` event when
the table expires and is removed.

### Admin activity (activity)

The main `activity` stream reports all remaining activities and events
such as table and dataset creation.

## Visibility and access control

BigQuery audit logs can include information that users might
consider sensitive, such as SQL text, schema definitions, and identifiers
for resources such as table and datasets. For information about
managing access to this information, see the Cloud Logging
[access control documentation](https://docs.cloud.google.com/logging/docs/access-control).

## Caller identities and resource names

Audit logging doesn't redact the caller's identity and IP addresses for any
access that succeeds or for any write operation.

For read-only operations that fail with a "permission denied" error,
Audit logging performs the following tests:

- Is the caller in the same organization as the resource being logged?
- Is the caller a service account?

If the response to any test is true, then Audit logging doesn't redact
the caller's identity and IP addresses. If the response to all tests is false,
then Audit logging redacts the identity and IP addresses.

For cross-project access, there are additional rules that apply:

- The billing project must be the project that sends the request, and
  the data project must be the project whose resources are also accessed
  during the job. For example, a query job in a billing project reads
  some table data from the data project.

- The billing project resource ID is redacted from the data project log
  unless the projects have the same domain associated with them or are in the
  same organization.

- Identities and caller IP addresses are not redacted from the data project
  log if either one of the preceding conditions apply or the billing project
  and the data project are in the same organization and the billing project
  already includes the identity and caller IP address.

## Cloud Logging exports

BigQuery automatically sends audit logs to Cloud Logging.
Cloud Logging lets users filter and
[route messages to other services](https://docs.cloud.google.com/logging/docs/routing/overview),
including Pub/Sub, Cloud Storage, and BigQuery.

With long term log retention and log exports to BigQuery, you can
do aggregated analysis on logs data. Cloud Logging documents
[how messages are transformed](https://docs.cloud.google.com/logging/docs/export/bigquery)
when exported to BigQuery.

### Filtering exports

To filter relevant BigQuery Audit messages, you can express
filters as part of the export.

For example, the following advanced filter represents an export that
only includes the newer `BigQueryAuditMetadata` format:

```
protoPayload.metadata."@type"="type.googleapis.com/google.cloud.audit.BigQueryAuditMetadata"
```

You can express additional filters based on the fields within the log
messages. For more information about crafting advanced filters, see the
[advanced log filter documentation](https://docs.cloud.google.com/logging/docs/view/advanced-filters).

### Defining a BigQuery log sink using gcloud

The following example command line shows how you can use the Google Cloud CLI to
[create a logging sink](https://docs.cloud.google.com/sdk/gcloud/reference/logging/sinks/create)
in a dataset named `auditlog_dataset` that only
includes `BigQueryAuditMetadata` messages. The dataset must already exist before you create the logging sink.

```bash
gcloud logging sinks create my-example-sink bigquery.googleapis.com/projects/my-project-id/datasets/auditlog_dataset \
    --log-filter='protoPayload.metadata."@type"="type.googleapis.com/google.cloud.audit.BigQueryAuditMetadata"'
```

After the sink is created, give the service account created by the previous command
[access to the dataset](https://docs.cloud.google.com/bigquery/docs/dataset-access-controls#granting_access_to_a_dataset).

## Querying exported logs

### BigQueryAuditMetadata examples

The following examples show how you can use `BigQueryAuditMetadata` messages
to analyze BigQuery usage. Because of the schema conversion done during
the export from Cloud Logging into BigQuery, the message bodies are
presented in semi-structured form. The `protopayload_auditlog.metadataJson` is
a `STRING` field, and it contains the JSON representation of the message. You
can leverage
[JSON functions](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions)
in GoogleSQL to analyze this content.

**Note:** Change the `FROM` clause in each of these examples to the
corresponding exported tables in your project.

#### Example: Report expired tables

`BigQueryAuditMetadata` messages log when a table is deleted because its
expiration time was reached. The following sample query shows when these
messages occur and includes a URI that references the table resource
that was removed.

```sql
  #standardSQL
  SELECT
    protopayload_auditlog.resourceName AS resourceName,
    receiveTimestamp as logTime
  FROM `my-project-id.auditlog_dataset.cloudaudit_googleapis_com_system_event_*`
  WHERE
    protopayload_auditlog.methodName = 'InternalTableExpired'
  ORDER BY resourceName
```

#### Example: Most popular datasets

This query shows coarse, per-dataset statistics about table reads and table
modifications. Before you run this example, [define a log sink](https://docs.cloud.google.com/bigquery/docs/reference/auditlogs#defining_a_bigquery_log_sink_using_gcloud)
with an existing dataset.

```sql
  #standardSQL
  SELECT
    REGEXP_EXTRACT(protopayload_auditlog.resourceName, '^projects/[^/]+/datasets/([^/]+)/tables') AS datasetRef,
    COUNT(DISTINCT REGEXP_EXTRACT(protopayload_auditlog.resourceName, '^projects/[^/]+/datasets/[^/]+/tables/(.*)$')) AS active_tables,
    COUNTIF(JSON_QUERY(protopayload_auditlog.metadataJson, "$.tableDataRead") IS NOT NULL) AS dataReadEvents,
    COUNTIF(JSON_QUERY(protopayload_auditlog.metadataJson, "$.tableDataChange") IS NOT NULL) AS dataChangeEvents
  FROM `my-project-id.auditlog_dataset.cloudaudit_googleapis_com_data_access_*`
  WHERE
    JSON_QUERY(protopayload_auditlog.metadataJson, "$.tableDataRead") IS NOT NULL
    OR JSON_QUERY(protopayload_auditlog.metadataJson, "$.tableDataChange") IS NOT NULL
  GROUP BY datasetRef
  ORDER BY datasetRef
```

## Troubleshooting

This section shows you how to resolve issues with BigQuery audit
logs.

### Truncated log entry

The following issue occurs when a log message is larger than
the [log message size limit](https://docs.cloud.google.com/bigquery/docs/reference/auditlogs#limitation):

The `protoPayload.metadata` submessage in the
Cloud Logging `LogEntry` message is truncated.

To resolve this issue, consider the following strategies:

- Retrieve the full log message by using the
  BigQuery API [jobs.get](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs/get) method.

- Reduce the size of the metadata in the log message; for example, by using
  wildcards on common path prefixes to reduce the size of the `sourceUri` list.