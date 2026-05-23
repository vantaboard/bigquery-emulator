# Introduction to audit logs in BigQuery

Logs are text records that are generated in response to particular events or
actions. For instance, BigQuery creates log entries for actions such as creating
or deleting a table, purchasing slots, or running a load job.

Google Cloud also writes logs, including audit logs that provide insight into
operational concerns related to your use of Google Cloud services. For more information about how Google Cloud handles logging, see the [Cloud Logging](https://docs.cloud.google.com/logging/docs) documentation and [Cloud Audit Logs overview](https://docs.cloud.google.com/logging/docs/audit).

## Audit logs versus `INFORMATION_SCHEMA` views

Your Google Cloud projects contain audit logs only for the resources that are directly within the Google Cloud project. Other Google Cloud resources, such as folders, organizations, and billing accounts, contain their own audit logs.

Audit logs help you answer the question "Who did what, where, and when?" within your Google Cloud resources. Audit logs are the definitive source of information for system activity by user and access patterns and should be your primary source for audit or security questions.

[`INFORMATION_SCHEMA`](https://docs.cloud.google.com/bigquery/docs/information-schema-intro) views in BigQuery are another source of insights that you can use along with metrics and logs. These views contain metadata about jobs, datasets, tables, and other BigQuery entities. For example, you can get real-time metadata about which BigQuery jobs ran during a specified time. Then, you can group or filter the results by project, user, tables referenced, and other dimensions.

`INFORMATION_SCHEMA` views provide you information to perform a more detailed analysis about your BigQuery workloads, such as the following:

- What is the average slot utilization for all queries over the past seven days for a given project?
- What streaming errors occurred in the past 30 minutes, grouped by error code?

BigQuery audit logs contain log entries for API calls, but they
don't describe the impact of the API calls. A subset of API calls creates jobs
(such as query and load) whose information is captured by `INFORMATION_SCHEMA`
views. For example, you can find information about the time and slots that are
utilized by a specific query in `INFORMATION_SCHEMA` views but not in the audit logs.

To get insights into the performance of your BigQuery workloads in particular, see [jobs metadata](https://docs.cloud.google.com/bigquery/docs/information-schema-jobs), [streaming metadata](https://docs.cloud.google.com/bigquery/docs/information-schema-streaming), and [reservations metadata](https://docs.cloud.google.com/bigquery/docs/information-schema-reservations).

For more information about the types of audit logs that Google Cloud services write, see
[Types of audit logs](https://docs.cloud.google.com/logging/docs/audit#types).

## Audit log format

Google Cloud services write audit logs in a structured JSON format. The base data type for Google Cloud log entries is the [`LogEntry`](https://docs.cloud.google.com/logging/docs/reference/v2/rest/v2/LogEntry) structure. This structure contains the name of the log, the resource that generated the log entry, the timestamp (UTC), and other basic information.

Logs include details of the logged event in a subfield that's called the *payload field* . For audit logs, the payload field is named `protoPayload`. This field's type (`protoPayload.@type`) is set to `type.googleapis.com/google.cloud.audit.AuditLog`, which indicates that the field uses the [`AuditLog`](https://docs.cloud.google.com/logging/docs/reference/audit/auditlog/rest/Shared.Types/AuditLog) log structure.

For operations on datasets, tables, and jobs, BigQuery writes audit logs in two different formats, although both formats share the `AuditLog` base type.

The older format includes the following fields and values:

- The value for the `resource.type` field is `bigquery_resource`.
- BigQuery writes the details about an operation in the `protoPayload.serviceData` field. The value of this field uses the [`AuditData`](https://docs.cloud.google.com/bigquery/docs/reference/auditlogs/rest/Shared.Types/AuditData) log structure.

The newer format includes the following fields and values:

- The value for the `resource.type` field is either `bigquery_project` or `bigquery_dataset`. The `bigquery_project` resource has log entries about jobs, while the `bigquery_dataset` resource has log entries about storage.
- BigQuery writes the details about an operation in the `protoPayload.metadata` field. The value of this field uses the [`BigQueryAuditMetadata`](https://docs.cloud.google.com/bigquery/docs/reference/auditlogs/rest/Shared.Types/BigQueryAuditMetadata) structure.

We recommend consuming logs in the newer format. For more information, see [Audit logs migration guide](https://docs.cloud.google.com/bigquery/docs/reference/auditlogs/migration).

The following is an abbreviated example of a log entry that shows a failed operation:

    {
      "protoPayload": {
        "@type": "type.googleapis.com/google.cloud.audit.AuditLog",
        "status": {
          "code": 5,
          "message": "Not found: Dataset myproject:mydataset was not found in location US"
        },
        "authenticationInfo": { ... },
        "requestMetadata":  { ... },
        "serviceName": "bigquery.googleapis.com",
        "methodName": "google.cloud.bigquery.v2.JobService.InsertJob",
        "metadata": {
      },
      "resource": {
        "type": "bigquery_project",
        "labels": { .. },
      },
      "severity": "ERROR",
      "logName": "projects/myproject/logs/cloudaudit.googleapis.com%2Fdata_access",
      ...
    }

For operations on BigQuery reservations, the `protoPayload` field uses the `AuditLog` structure, and the `protoPayload.request` and `protoPayload.response` fields contain more information. You can find the field definitions in [BigQuery Reservation API](https://docs.cloud.google.com/bigquery/docs/reference/reservations/rpc). For more information, see [Monitoring BigQuery reservations](https://docs.cloud.google.com/bigquery/docs/reservations-monitoring).

For a deeper understanding of the audit log format, see [Understand audit logs](https://docs.cloud.google.com/logging/docs/audit/understanding-audit-logs).

## Limitations

Log messages have a size limit of 100,000 bytes. For more information, see [Truncated log entry](https://docs.cloud.google.com/bigquery/docs/reference/auditlogs#truncated_log_entry).

## Visibility and access control

BigQuery audit logs can include information that users might consider sensitive, such as SQL text, schema definitions, and identifiers for resources such as tables and datasets. For information about managing access to this information, see the Cloud Logging [access control documentation](https://docs.cloud.google.com/logging/docs/access-control).

## What's next

- To learn how to use Cloud Logging to audit activities that are related to policy tags, see [Audit policy tags](https://docs.cloud.google.com/bigquery/docs/auditing-policy-tags).
- To learn how to use BigQuery to analyze logged activity, see [BigQuery audit logs overview](https://docs.cloud.google.com/bigquery/docs/reference/auditlogs).