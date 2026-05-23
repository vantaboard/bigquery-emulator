# Quotas and limits

This document lists the quotas and system limits that apply to
BigQuery.

- *Quotas* have default values, but you can typically request adjustments.
- *System limits* are fixed values that can't be changed.

Google Cloud uses quotas to help ensure fairness and reduce
spikes in resource use and availability. A quota restricts how much of a
Google Cloud resource your Google Cloud project can use. Quotas
apply to a range of resource types, including hardware, software, and network
components. For example, quotas can restrict the number of API calls to a
service, the number of load balancers used concurrently by your project, or the
number of projects that you can create. Quotas protect the community of
Google Cloud users by preventing the overloading of services. Quotas also
help you to manage your own Google Cloud resources.

The Cloud Quotas system does the following:

- Monitors your consumption of Google Cloud products and services
- Restricts your consumption of those resources
- Provides a way to [request changes to the quota value](https://docs.cloud.google.com/docs/quotas/help/request_increase) and [automate quota adjustments](https://docs.cloud.google.com/docs/quotas/quota-adjuster)

In most cases, when you attempt to consume more of a resource than its quota
allows, the system blocks access to the resource, and the task that
you're trying to perform fails.

Quotas generally apply at the Google Cloud project
level. Your use of a resource in one project doesn't affect
your available quota in another project. Within a Google Cloud project, quotas
are shared across all applications and IP addresses.


For more information, see the
[Cloud Quotas overview](https://docs.cloud.google.com/docs/quotas/overview).


There are also *system limits* on BigQuery resources.
System limits can't be changed.

Some error messages specify quotas or limits that you can increase, while other
error messages specify quotas or limits that you can't increase. Reaching a hard
limit means that you need to implement temporary or permanent workarounds or
best practices for your workload. Doing so is a best practice, even for quotas
or limits that can be increased. For details about both types of errors, see
[Troubleshoot quota and limit errors](https://docs.cloud.google.com/bigquery/docs/troubleshoot-quotas).

By default, BigQuery
quotas and limits apply on a [per-project](https://docs.cloud.google.com/bigquery/docs/projects) basis.
Quotas and limits that apply on a different basis are indicated as
such; for example, the maximum number of columns *per table* , or the maximum
number of concurrent API requests *per user*.
Specific policies vary depending on resource availability, user profile,
Service Usage history, and other factors, and are subject to change without
notice.

### Quota replenishment

Daily quotas are replenished at regular intervals throughout the day,
reflecting their intent to guide rate limiting behaviors. Intermittent refresh is also done
to avoid long disruptions when quota is exhausted. More quota is typically
made available within minutes rather than globally replenished once daily.

### Request a quota increase

To adjust most quotas, use the Google Cloud console.
For more information, see
[Request a quota adjustment](https://docs.cloud.google.com/docs/quotas/help/request_increase).

For step-by-step guidance through the process of requesting a quota increase
in Google Cloud console, click **Guide me**:


<br />

[Guide me](https://console.cloud.google.com/bigquery?tutorial=bigquery_quota_request)

<br />

### Cap quota usage

To learn how you can limit usage of a particular resource by creating a quota
override, see
[Create quota override](https://docs.cloud.google.com/docs/quotas/view-manage#capping_usage).

### Required permissions

To view and update your BigQuery quotas in the
Google Cloud console, you need the same permissions as for any Google Cloud
quota. For more information, see
[Google Cloud quota permissions](https://docs.cloud.google.com/docs/quotas/permissions).

### Troubleshoot

For information about troubleshooting errors related to quotas and limits, see
[Troubleshooting BigQuery quota errors](https://docs.cloud.google.com/bigquery/docs/troubleshoot-quotas).

## Jobs

Quotas and limits apply to jobs that BigQuery runs on your behalf
whether they are run by using Google Cloud console, the bq command-line tool, or
programmatically using the REST API or client libraries.

### Query jobs

The following quotas apply to query jobs created automatically by
running interactive queries, scheduled queries, and jobs submitted by using the
[`jobs.query`](https://docs.cloud.google.com/bigquery/docs/reference/v2/jobs/query)
and query-type [`jobs.insert`](https://docs.cloud.google.com/bigquery/docs/reference/v2/jobs/insert)
API methods.

For troubleshooting information, see the BigQuery
[Troubleshooting page](https://docs.cloud.google.com/bigquery/docs/troubleshoot-quotas).

| Quota | Default | Notes |
|---|---|---|
| Query usage per day | 200 Tebibytes (TiB) | This quota applies only to [the on-demand query pricing model.](https://cloud.google.com/bigquery/pricing#on_demand_pricing) Your project can run up to 200 TiB in queries per day. You can change this limit anytime. See [Create custom query quotas](https://docs.cloud.google.com/bigquery/docs/custom-quotas) to learn more about cost controls. [View quota in Google Cloud console](https://console.cloud.google.com/iam-admin/quotas?service=bigquery.googleapis.com&metric=bigquery.googleapis.com/quota/query/usage) |
| Query usage per day per user | Unlimited | This quota applies only to [the on-demand query pricing model.](https://cloud.google.com/bigquery/pricing#on_demand_pricing) There is no default limit on how many TiB in queries a user can run per day. You can set the limit anytime. Regardless of the per user limit, the total usage for all users in the project combined can never exceed the query usage per day limit. See [Create custom query quotas](https://docs.cloud.google.com/bigquery/docs/custom-quotas) to learn more about cost controls. [View quota in Google Cloud console](https://console.cloud.google.com/iam-admin/quotas?service=bigquery.googleapis.com&metric=bigquery.googleapis.com/quota/query/usage) |
| GoogleSQL federated query cross-region bytes per day | 1 TB | If the [BigQuery query processing location](https://docs.cloud.google.com/bigquery/docs/locations) and the Cloud SQL instance location are different, then your query is a cross-region query. Your project can run up to 1 TB in cross-region queries per day. See [Cloud SQL federated queries](https://docs.cloud.google.com/bigquery/docs/cloud-sql-federated-queries). [View quota in Google Cloud console](https://console.cloud.google.com/iam-admin/quotas?service=bigquery.googleapis.com&metric=bigquery.googleapis.com/quota/query/cloud_sql_federated_query_cross_region_bytes) |
| Cross-cloud transferred bytes per day | 1 TB | You can transfer up to 1 TB of data per day from [an Amazon S3 bucket or from Azure Blob Storage](https://docs.cloud.google.com/bigquery/docs/omni-aws-cross-cloud-transfer). [View quota in Google Cloud console](https://console.cloud.google.com/iam-admin/quotas?service=bigquery.googleapis.com&metric=bigquery.googleapis.com/quota/query/cross_cloud_transfer_bytes) |
| Bytes transferred by global queries per day per region pair | 180 TB | You can transfer up to 180 TB of data per day between each pair of regions with [global queries](https://docs.cloud.google.com/bigquery/docs/global-queries). |
| Global queries copy jobs per project | 10,000 | You can execute up to 10,000 copy jobs per project when you run [global queries](https://docs.cloud.google.com/bigquery/docs/global-queries) that copy data between regions. One global query might trigger multiple copy jobs. [View quota in Google Cloud console](https://console.cloud.google.com/iam-admin/quotas?service=bigquery.googleapis.com&metric=bigquery.googleapis.com/quota/query/run_global_queries_copy_job) |
| Bytes transferred by a single copy job in a global query | 100 GB | A single copy job that is a part of a [global query](https://docs.cloud.google.com/bigquery/docs/global-queries) can't transfer more than 100 GB. |
| Global project-level options updates | 5 | When you run a DDL statement that changes [global configuration options](https://docs.cloud.google.com/bigquery/docs/default-configuration#global-settings), you can run up to five statements every 10 seconds. This limit applies to the following DDL statements: - [`ALTER PROJECT SET OPTIONS`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#alter_project_set_options_statement) - [`ALTER ORGANIZATION SET OPTIONS`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#alter_organization_set_options_statement) |

The following limits apply to query jobs created automatically by
running interactive queries, scheduled queries, and jobs submitted by using the
[`jobs.query`](https://docs.cloud.google.com/bigquery/docs/reference/v2/jobs/query)
and query-type [`jobs.insert`](https://docs.cloud.google.com/bigquery/docs/reference/v2/jobs/insert)
API methods:

| Limit | Default | Notes |
|---|---|---|
| Maximum number of queued interactive queries | 1,000 queries | Your project can queue up to 1,000 interactive queries. Additional interactive queries that exceed this limit return a quota error. To troubleshoot these errors, see [Avoid limits for high-volume interactive queries](https://docs.cloud.google.com/bigquery/docs/troubleshoot-quotas#interactive-query-queue-resolution). |
| Maximum number of queued batch queries | 20,000 queries | Your project can queue up to 20,000 batch queries. Additional batch queries that exceed this limit return a quota error. |
| Maximum number of concurrent interactive queries against Bigtable external data sources | 16 queries | Your project can run up to sixteen concurrent queries against a [Bigtable external data source](https://docs.cloud.google.com/bigquery/external-data-bigtable). |
| Maximum number of concurrent queries that contain remote functions | 10 queries | You can run up to ten concurrent queries with [remote functions](https://docs.cloud.google.com/bigquery/docs/remote-functions) per project. |
| Maximum number of concurrent multi-statement queries | 1,000 multi-statement queries | Your project can run up to 1,000 concurrent [multi-statement queries](https://docs.cloud.google.com/bigquery/docs/multi-statement-queries). For other quotas and limits related to multi-statement queries, see [Multi-statement queries](https://docs.cloud.google.com/bigquery/quotas#multi_statement_query_limits). |
| Maximum number of concurrent legacy SQL queries that contain UDFs | 6 queries | Your project can run up to six concurrent legacy SQL queries with user-defined functions (UDFs). This limit includes both [interactive](https://docs.cloud.google.com/bigquery/docs/running-queries#queries) and [batch](https://docs.cloud.google.com/bigquery/docs/running-queries#batch) queries. Interactive queries that contain UDFs also count toward the concurrent limit for interactive queries. This limit does not apply to GoogleSQL queries. |
| Daily query size limit | Unlimited | By default, there is no daily query size limit. However, you can set limits on the amount of data users can query by creating [custom quotas](https://docs.cloud.google.com/bigquery/cost-controls#controlling_query_costs_using_bigquery_ custom_quotas) to control [query usage per day](https://docs.cloud.google.com/bigquery/quotas#query_usage_per_day) or [query usage per day per user](https://docs.cloud.google.com/bigquery/quotas#query_usage_per_day_per_user). |
| Daily destination table update limit | See [Maximum number of table operations per day](https://docs.cloud.google.com/bigquery/quotas#load_job_per_table.long). | Updates to destination tables in a query job count toward the limit on the maximum number of table operations per day for the destination tables. Destination table updates include append and overwrite operations that are performed by queries that you run by using the Google Cloud console, using the bq command-line tool, or calling the [`jobs.query`](https://docs.cloud.google.com/bigquery/docs/reference/v2/jobs/query) and query-type [`jobs.insert`](https://docs.cloud.google.com/bigquery/docs/reference/v2/jobs/insert) API methods. |
| Query/multi-statement query execution-time limit | 6 hours | A query or multi-statement query can execute for up to 6 hours, and then it fails. However, sometimes queries are retried. A query can be tried up to three times, and each attempt can run for up to 6 hours. As a result, it's possible for a query to have a total runtime of more than 6 hours. `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create` job timeout defaults to 24 hours, with the exception of time series, AutoML, and hyperparameter tuning jobs which timeout at 48 hours. |
| Maximum number of resources referenced per query | 1,000 resources | A query can reference up to 1,000 total of unique [tables](https://docs.cloud.google.com/bigquery/docs/tables-intro), unique [views](https://docs.cloud.google.com/bigquery/docs/views-intro), unique [user-defined functions](https://docs.cloud.google.com/bigquery/docs/user-defined-functions) (UDFs), and unique [table functions](https://docs.cloud.google.com/bigquery/docs/table-functions) after full expansion. This limit includes the following: - Tables, views, UDFs, and table functions directly referenced by the query. - Tables, views, UDFs, and table functions referenced by other views/UDFs/table functions referenced in the query. |
| Maximum SQL query character length | 1,024k characters | A SQL query can be up to 1,024k characters long. This limit includes comments and whitespace characters. If your query is longer, you receive the following error: `The query is too large.` To stay within this limit, consider replacing large arrays or lists with query parameters and breaking a long query into multiple queries in the session. |
| Maximum unresolved legacy SQL query length | 256 KB | An unresolved legacy SQL query can be up to 256 KB long. If your query is longer, you receive the following error: `The query is too large.` To stay within this limit, consider replacing large arrays or lists with query parameters. |
| Maximum unresolved GoogleSQL query length | 1 MB | An unresolved GoogleSQL query can be up to 1 MB long. If your query is longer, you receive the following error: `The query is too large.` To stay within this limit, consider replacing large arrays or lists with query parameters. |
| Maximum resolved legacy and GoogleSQL query length | 12 MB | The limit on resolved query length includes the length of all views and wildcard tables referenced by the query. |
| Maximum number of GoogleSQL query parameters | 10,000 parameters | A GoogleSQL query can have up to 10,000 parameters. |
| Maximum request size | 10 MB | The request size can be up to 10 MB, including additional properties like query parameters. |
| Maximum response size | 10 GB compressed | Sizes vary depending on compression ratios for the data. The actual response size might be significantly larger than 10 GB. The maximum response size is unlimited when [writing large query results to a destination table](https://docs.cloud.google.com/bigquery/docs/writing-results#large-results). |
| Maximum row size | 100 MB | The maximum row size is approximate, because the limit is based on the internal representation of row data. The maximum row size limit is enforced during certain stages of query job execution. |
| Maximum columns in a table, query result, or view definition | 10,000 columns | A table, query result, or view definition can have up to 10,000 columns. This includes nested and repeated columns. Deleted columns can continue to count towards the total number of columns. If you've deleted columns, then you might receive quota errors until the total resets. |
| Maximum concurrent slots for on-demand pricing | 2,000 slots per project 20,000 slots per organization | With on-demand pricing, your project can have up to 2,000 concurrent slots. There is also a 20,000 concurrent slots cap at the organization level. BigQuery tries to allocate slots fairly between projects within an organization if their total demand is higher than 20,000 slots. BigQuery slots are shared among all queries in a single project. BigQuery might exceed this limit to accelerate your queries. The capacity is subject to availability. To check how many slots you're using, see [Monitoring BigQuery using Cloud Monitoring](https://docs.cloud.google.com/bigquery/docs/monitoring). |
| Maximum CPU usage per scanned data for on-demand pricing | 256 CPU seconds per MiB scanned | With on-demand pricing, your query can use up to approximately 256 CPU seconds per MiB of scanned data. If your query is too CPU-intensive for the amount of data being processed, the query fails with a `billingTierLimitExceeded` error. For more information, see [Error messages](https://docs.cloud.google.com/bigquery/docs/error-messages). |
| Multi-statement transaction table mutations | 100 tables | A [transaction](https://docs.cloud.google.com/bigquery/docs/transactions) can mutate data in at most 100 tables. |
| Multi-statement transaction partition modifications | 100,000 partition modifications | A [transaction](https://docs.cloud.google.com/bigquery/docs/transactions) can perform at most 100,000 partition modifications. |
| BigQuery Omni maximum query result size | 20 GiB uncompressed | The maximum result size is 20 GiB logical bytes when querying [Microsoft Azure](https://docs.cloud.google.com/bigquery/docs/query-azure-data) or [AWS](https://docs.cloud.google.com/bigquery/docs/query-aws-data) data. If your query result is larger than 20 GiB, consider exporting the results to [Amazon S3](https://docs.cloud.google.com/bigquery/docs/omni-aws-export-results-to-s3) or [Blob Storage](https://docs.cloud.google.com/bigquery/docs/omni-azure-export-results-to-azure-storage). For more information, see [BigQuery Omni Limitations](https://docs.cloud.google.com/bigquery/docs/omni-introduction#limitations). |
| BigQuery Omni total query result size per day | 1 TB | The total query result sizes for a project is 1 TB per day. For more information, see [BigQuery Omni limitations](https://docs.cloud.google.com/bigquery/docs/omni-introduction#limitations). |
| BigQuery Omni maximum row size | 10 MiB | The maximum row size is 10 MiB when querying [Microsoft Azure](https://docs.cloud.google.com/bigquery/docs/query-azure-data) or [AWS](https://docs.cloud.google.com/bigquery/docs/query-aws-data) data. For more information, see [BigQuery Omni Limitations](https://docs.cloud.google.com/bigquery/docs/omni-introduction#limitations). |
| Global queries transfer | 2 GiB/s | The bandwidth of transfer used by [global queries](https://docs.cloud.google.com/bigquery/docs/global-queries), per project, per region pair |

Although scheduled queries use features of the
[BigQuery Data Transfer Service](https://docs.cloud.google.com/bigquery/docs/dts-introduction),
scheduled queries are not transfers, and are not subject to
[load job limits](https://docs.cloud.google.com/bigquery/quotas#load_jobs).

### Extract jobs

The following limits apply to jobs that
[extract data](https://docs.cloud.google.com/bigquery/docs/exporting-data)
from BigQuery by using the bq command-line tool, Google Cloud console,
or the extract-type [`jobs.insert`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs/insert)
API method.

| Limit | Default | Notes |
|---|---|---|
| Maximum number of extracted bytes per day | 50 TiB | You can extract up to 50 TiB(Tebibytes) of data per day from a project at no cost using the shared slot pool. You can [set up a Cloud Monitoring](https://docs.cloud.google.com/bigquery/docs/exporting-data#view_current_quota_usage) alert policy that provides notification of the number of bytes extracted. To extract more than 50 TiB(Tebibytes) of data per day, do one of the following: - Create a [slot reservation](https://docs.cloud.google.com/bigquery/docs/reservations-intro#reservations) or use an existing reservation and [assign](https://docs.cloud.google.com/bigquery/docs/reservations-workload-management#assignments) your project into the reservation with job type `PIPELINE`. You are billed using [capacity-based pricing](https://cloud.google.com/bigquery/pricing#capacity_compute_analysis_pricing). `EXPORT DATA` statements aren't supported for `PIPELINE` reservations. - Use the [`EXPORT DATA`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/other-statements#export_data_statement) SQL statement. We will bill you using either [on-demand](https://cloud.google.com/bigquery/pricing#on_demand_pricing) or [capacity-based pricing](https://cloud.google.com/bigquery/pricing#capacity_compute_analysis_pricing), depending on how your project is configured. - Use the [Storage Read API](https://docs.cloud.google.com/bigquery/docs/reference/storage). We will bill you using the price for [streaming reads](https://cloud.google.com/bigquery/pricing#data_extraction_pricing). The expiration time is guaranteed to be at least [6 hours](https://docs.cloud.google.com/bigquery/docs/reference/storage#create_a_session) from session creation time. |
| Maximum number of extract jobs per day | 100,000 extract jobs | You can run up to 100,000 extract jobs per day in a project. To run more than 100,000 extract jobs per day, do one of the following: - Create a [slot reservation](https://docs.cloud.google.com/bigquery/docs/reservations-intro#reservations) or use an existing reservation and [assign](https://docs.cloud.google.com/bigquery/docs/reservations-workload-management#assignments) your project into the reservation with job type `PIPELINE`. We will bill you using [capacity-based pricing](https://cloud.google.com/bigquery/pricing#capacity_compute_analysis_pricing). - Use the [`EXPORT DATA`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/other-statements#export_data_statement) SQL statement. We will bill you using either [on-demand](https://cloud.google.com/bigquery/pricing#on_demand_pricing) or [capacity-based pricing](https://cloud.google.com/bigquery/pricing#capacity_compute_analysis_pricing), depending on how your project is configured. - Use the [Storage Read API](https://docs.cloud.google.com/bigquery/docs/reference/storage). We will bill you using the price for [streaming reads](https://cloud.google.com/bigquery/pricing#data_extraction_pricing). The expiration time is guaranteed to be at least [6 hours](https://docs.cloud.google.com/bigquery/docs/reference/storage#create_a_session) from session creation time. |
| Maximum table size extracted to a single file | 1 GB | You can extract up to 1 GB of table data to a single file. To extract more than 1 GB of data, use a [wildcard](https://docs.cloud.google.com/bigquery/docs/exporting-data#exporting_data_into_one_or_more_files) to extract the data into multiple files. When you extract data to multiple files, the size of the files varies. In some cases, the size of the output files is more than 1 GB. |
| [Wildcard URIs](https://docs.cloud.google.com/bigquery/docs/exporting-data#exporting_data_into_one_or_more_files) per extract job | 500 URIs | An extract job can have up to 500 wildcard URIs. |

<br />

For more information about viewing your current extract job usage, see [View current quota usage](https://docs.cloud.google.com/bigquery/docs/exporting-data#view_current_quota_usage). For
troubleshooting information, see [Export troubleshooting](https://docs.cloud.google.com/bigquery/docs/exporting-data#troubleshooting).

### Load jobs

The following limits apply when you
[load data](https://docs.cloud.google.com/bigquery/loading-data-into-bigquery)
into BigQuery, using the
Google Cloud console, the bq command-line tool, or the load-type
[`jobs.insert`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs/insert)
API method.

| Limit | Default | Notes |
|---|---|---|
| Load jobs per table per day | 1,500 jobs | Load jobs, including failed load jobs, count toward the limit on the number of table operations per day for the destination table. For information about limits on the number of table operations per day for standard tables and partitioned tables, see [Tables](https://docs.cloud.google.com/bigquery/quotas#table_limits). |
| Load jobs per day | 100,000 jobs | Your project is replenished with a maximum of 100,000 load jobs quota every 24 hours. Failed load jobs count toward this limit. In some cases, it is possible to run more than 100,000 load jobs in 24 hours if a prior day's quota is not fully used. |
| Maximum columns per table | 10,000 columns | A table can have up to 10,000 columns. This includes nested and repeated columns. |
| Maximum size per load job | 15 TB | The total size for all of your CSV, JSON, Avro, Parquet, and ORC input files can be up to 15 TB. This limit does not apply for jobs with a reservation. |
| Maximum number of source URIs in job configuration | 10,000 URIs | A job configuration can have up to 10,000 source URIs. |
| Maximum number of files per load job | 10,000,000 files | A load job can have up to 10 million total files, including all files matching all wildcard URIs. |
| Maximum number of files in the source Cloud Storage bucket | Approximately 60,000,000 files | A load job can read from a Cloud Storage bucket containing up to approximately 60,000,000 files. |
| Load job execution-time limit | 6 hours | A load job fails if it executes for longer than six hours. |
| Avro: Maximum size for file data blocks | 16 MB | The size limit for Avro file data blocks is 16 MB. |
| CSV: Maximum cell size | 100 MB | CSV cells can be up to 100 MB in size. |
| CSV: Maximum row size | 100 MB | CSV rows can be up to 100 MB in size. |
| CSV: Maximum file size - compressed | 4 GB | The size limit for a compressed CSV file is 4 GB. |
| CSV: Maximum file size - uncompressed | 5 TB | The size limit for an uncompressed CSV file is 5 TB. |
| Newline-delimited JSON (ndJSON): Maximum row size | 100 MB | ndJSON rows can be up to 100 MB in size. |
| ndJSON: Maximum file size - compressed | 4 GB | The size limit for a compressed ndJSON file is 4 GB. |
| ndJSON: Maximum file size - uncompressed | 5 TB | The size limit for an uncompressed ndJSON file is 5 TB. |

If you regularly exceed the load job limits due to frequent updates, consider
[streaming data into BigQuery](https://docs.cloud.google.com/bigquery/docs/streaming-data-into-bigquery) instead.

For information on viewing your current load job usage, see [View current quota usage](https://docs.cloud.google.com/bigquery/docs/batch-loading-data#view_current_quota_usage).

#### BigQuery Data Transfer Service load job quota considerations

Load jobs created by BigQuery Data Transfer Service transfers are included in
BigQuery's quotas on load jobs. It's important to consider
how many transfers you enable in each project to prevent transfers and other
load jobs from producing `quotaExceeded` errors.

You can use the following equation to estimate how many load jobs are required
by your transfers:

`Number of daily jobs = Number of transfers x Number of tables x
Schedule frequency x Refresh window`

Where:

- `Number of transfers` is the number of transfer configurations you enable in your project.
- `Number of tables` is the number of tables created by each specific transfer
  type. The number of tables varies by transfer type:

  - Campaign Manager transfers create approximately 25 tables.
  - Google Ads transfers create approximately 60 tables.
  - Google Ad Manager transfers create approximately 40 tables.
  - Google Play transfers create approximately 25 tables.
  - Search Ads 360 transfers create approximately 50 tables.
  - YouTube transfers create approximately 50 tables.
- `Schedule frequency` describes how often the transfer runs. Transfer run schedules
  are provided for each transfer type:

  - [Campaign Manager](https://docs.cloud.google.com/bigquery/docs/doubleclick-campaign-transfer#connector_overview)
  - [Google Ads](https://docs.cloud.google.com/bigquery/docs/adwords-transfer#connector_overview)
  - [Google Ad Manager](https://docs.cloud.google.com/bigquery/docs/doubleclick-publisher-transfer)
  - [Google Merchant Center](https://docs.cloud.google.com/bigquery/docs/merchant-center-transfer#supported_reports) (beta)
  - [Google Play](https://docs.cloud.google.com/bigquery/docs/play-transfer)
  - [Search Ads 360](https://docs.cloud.google.com/bigquery/docs/search-ads-transfer#connector_overview) (beta)
  - [YouTube Channel](https://docs.cloud.google.com/bigquery/docs/youtube-channel-transfer)
  - [YouTube Content Owner](https://docs.cloud.google.com/bigquery/docs/youtube-content-owner-transfer)
- `Refresh window` is the number of days to include in the data transfer. If you
  enter 1, there is no daily backfill.

### Copy jobs

The following limits apply to BigQuery jobs for
[copying tables](https://docs.cloud.google.com/bigquery/docs/managing-tables#copy-table), including jobs
that create a copy, clone, or snapshot of a standard table, table clone, or
table snapshot.
The limits apply to jobs created by using the Google Cloud console, the
bq command-line tool, or the
[`jobs.insert` method](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs/insert) that
specifies the [`copy` field](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/Job#JobConfiguration.FIELDS.copy)
in the job configuration.
Copy jobs count toward these limits whether they succeed or fail.

| Limit | Default | Notes |
|---|---|---|
| Copy jobs per destination table per day |   | See [Table operations per day](https://docs.cloud.google.com/bigquery/quotas#load_job_per_table.long). |
| Copy jobs per day | 100,000 jobs | Your project can run up to 100,000 copy jobs per day. |
| Cross-region copy jobs per destination table per day | 100 jobs | Your project can run up to 100 cross-region copy jobs for a destination table per day. |
| Cross-region copy jobs per day | 2,000 jobs | Your project can run up to 2,000 cross-region copy jobs per day. |
| Number of source tables to copy | 1,200 source tables | You can copy from up to 1,200 source tables per copy job. |

For information on viewing your current copy job usage, see [Copy jobs - View current quota usage](https://docs.cloud.google.com/bigquery/docs/managing-tables#view_current_quota_usage). For information
on troubleshooting copy jobs, see [Maximum number of copy jobs per day per project quota errors](https://docs.cloud.google.com/bigquery/docs/troubleshoot-quotas#ts-maximum-number-of-copy-jobs-per-day-per-project-quota).


The following limits apply to
[copying
datasets](https://docs.cloud.google.com/bigquery/docs/copying-datasets):

| Limit | Default | Notes |
|---|---|---|
| Maximum number of tables in the source dataset | 25,000 tables | A source dataset can have up to 25,000 tables. |
| Maximum number of tables that can be copied per run to a destination dataset in the same region | 20,000 tables | Your project can copy a maximum of 20,000 tables per run to a destination dataset within the same region. If a source dataset contains more than 20,000 tables, the BigQuery Data Transfer Service schedules sequential runs, each copying up to 20,000 tables, until all tables are copied. These runs are separated by a default interval of 24 hours, which users can customize down to a minimum of 12 hours. |
| Maximum number of tables that can be copied per run to a destination dataset in a different region | 1,000 tables | Your project can copy a maximum of 1,000 tables per run to a destination dataset in a different region. If a source dataset contains more than 1,000 tables, the BigQuery Data Transfer Service schedules sequential runs, each copying up to 1,000 tables, until all tables are copied. These runs are separated by a default interval of 24 hours, which users can customize down to a minimum of 12 hours. |

## Reservations

The following quotas apply to
[reservations](https://docs.cloud.google.com/bigquery/docs/reservations-intro):

| Quota | Default | Notes |
|---|---|---|
| Total number of slots for the EU region | 5,000 slots | The maximum number of BigQuery slots you can purchase in the EU multi-region by using the Google Cloud console. [View quotas in Google Cloud console](https://console.cloud.google.com/iam-admin/quotas?service=bigqueryreservation.googleapis.com&metric=bigqueryreservation.googleapis.com/total_slots_eu) |
| Total number of slots for the US region | 10,000 slots | The maximum number of BigQuery slots you can purchase in the US multi-region by using the Google Cloud console. [View quotas in Google Cloud console](https://console.cloud.google.com/iam-admin/quotas?service=bigqueryreservation.googleapis.com&metric=bigqueryreservation.googleapis.com/total_slots_us) |
| Total number of slots for the `us-east1` region | 4,000 slots | The maximum number of BigQuery slots that you can purchase in the listed region by using the Google Cloud console. [View quotas in Google Cloud console](https://console.cloud.google.com/iam-admin/quotas?service=bigqueryreservation.googleapis.com&metric=bigqueryreservation.googleapis.com/total_slots) |
| Total number of slots for the following regions: - `asia-south1` - `asia-southeast1` - `europe-west2` - `us-central1` - `us-west1` | 2,000 slots | The maximum number of BigQuery slots that you can purchase in each of the listed regions by using the Google Cloud console. [View quotas in Google Cloud console](https://console.cloud.google.com/iam-admin/quotas?service=bigqueryreservation.googleapis.com&metric=bigqueryreservation.googleapis.com/total_slots) |
| Total number of slots for the following regions: - `asia-east1` - `asia-northeast1` - `asia-northeast3` - `asia-southeast2` - `australia-southeast1` - `europe-north1` - `europe-west1` - `europe-west3` - `europe-west4` - `northamerica-northeast1` - `us-east4` - `southamerica-east1` | 1,000 slots | The maximum number of BigQuery slots you can purchase in each of the listed regions by using the Google Cloud console. [View quotas in Google Cloud console](https://console.cloud.google.com/iam-admin/quotas?service=bigqueryreservation.googleapis.com&metric=bigqueryreservation.googleapis.com/total_slots) |
| Total number of slots for BigQuery Omni regions | 100 slots | The maximum number of BigQuery slots you can purchase in the [BigQuery Omni](https://docs.cloud.google.com/bigquery/docs/locations#omni-loc) regions by using the Google Cloud console. [View quotas in Google Cloud console](https://console.cloud.google.com/iam-admin/quotas?service=bigqueryreservation.googleapis.com&metric=bigqueryreservation.googleapis.com/total_slots_us) |
| Total number of slots for all other regions | 500 slots | The maximum number of BigQuery slots you can purchase in each other region by using the Google Cloud console. [View quotas in Google Cloud console](https://console.cloud.google.com/iam-admin/quotas?service=bigqueryreservation.googleapis.com&metric=bigqueryreservation.googleapis.com/total_slots) |

The following limits apply to [reservations](https://docs.cloud.google.com/bigquery/docs/reservations-intro):

| Limit | Value | Notes |
|---|---|---|
| Number of [administration projects](https://docs.cloud.google.com/bigquery/docs/reservations-workload-management#admin-project) for slot reservations | 10 projects per organization | The maximum number of projects within an organization that can contain a reservation or an active commitment for slots for a given location / region. |
| Maximum number of [standard](https://docs.cloud.google.com/bigquery/docs/editions-intro) edition reservations | 10 reservations per project | The maximum number of standard edition reservations per administration project within an organization for a given location / region. |
| Maximum number of [Enterprise or Enterprise Plus](https://docs.cloud.google.com/bigquery/docs/editions-intro) edition reservations | 200 reservations per project | The maximum number of Enterprise or Enterprise Plus edition reservations per administration project within an organization for a given location / region. |
| Maximum number of slots in a reservation that is associated with a reservation assignment with a `CONTINUOUS` job type. | 500 slots | When you want to create a reservation assignment that has a `CONTINUOUS` job type, the associated reservation can't have more than 500 slots. |

## Datasets

The following limits apply to BigQuery
[datasets](https://docs.cloud.google.com/bigquery/docs/datasets):

| Limit | Default | Notes |
|---|---|---|
| Maximum number of datasets | Unlimited | There is no limit on the number of datasets that a project can have. |
| Number of tables per dataset | Unlimited | When you use an API call, enumeration performance slows as you approach 50,000 tables in a dataset. The Google Cloud console can display up to 50,000 tables for each dataset. |
| Number of authorized resources in a dataset's access control list | 2,500 resources | A dataset's access control list can have up to 2,500 total authorized resources, including [authorized views](https://docs.cloud.google.com/bigquery/docs/authorized-views), [authorized datasets](https://docs.cloud.google.com/bigquery/docs/authorized-datasets), and [authorized functions](https://docs.cloud.google.com/bigquery/docs/authorized-functions). If you exceed this limit due to a large number of authorized views, consider grouping the views into authorized datasets. As a best practice, group related views into authorized datasets when you design new BigQuery architectures, especially multi-tenant architectures. |
| Number of dataset update operations per dataset per 10 seconds | 5 operations | Your project can make up to five dataset update operations every 10 seconds. The dataset update limit includes all metadata update operations performed by the following: - Google Cloud console - The bq command-line tool - BigQuery client libraries - The following API methods: - [`datasets.insert`](https://docs.cloud.google.com/bigquery/docs/reference/v2/datasets/insert) - [`datasets.patch`](https://docs.cloud.google.com/bigquery/docs/reference/v2/datasets/patch) - [`datasets.update`](https://docs.cloud.google.com/bigquery/docs/reference/v2/datasets/update) - [`datasets.delete`](https://docs.cloud.google.com/bigquery/docs/reference/v2/datasets/delete) - [`datasets.undelete`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets/undelete) - The following DDL statements: - [`CREATE SCHEMA`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_schema_statement) - [`ALTER SCHEMA`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#alter_schema_set_options_statement) - [`DROP SCHEMA`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#drop_schema_statement) - [`UNDROP SCHEMA`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#undrop_schema_statement) |
| Maximum length of a dataset description | 16,384 characters | When you add a description to a dataset, the text can be at most 16,384 characters. |

<br />

## Tables

### All tables

The following limits apply to all BigQuery tables.

> [!NOTE]
> **Note:** Quotas and limits are associated with table names. Therefore, when you truncate the table, or drop the table and then recreate it, the quota/limit doesn't reset, because the table name hasn't changed.

| Limit | Default | Notes |
|---|---|---|
| Maximum length of a column name | 300 characters | Your column name can be at most 300 characters. |
| Maximum length of a column description | 1,024 characters | When you add a description to a column, the text can be at most 1,024 characters. |
| Maximum depth of nested records | 15 levels | Columns of type `RECORD` can contain nested `RECORD` types, also called *child* records. The maximum nested depth limit is 15 levels. This limit is independent of whether the records are scalar or array-based (repeated). |
| Maximum length of a table description | 16,384 characters | When you add a description to a table, the text can be at most 16,384 characters. |

For troubleshooting information related to table quotas or limits, see the
[BigQuery Troubleshooting page](https://docs.cloud.google.com/bigquery/docs/troubleshoot-quotas).

### Standard tables

The following limits apply to BigQuery standard (built-in)
[tables](https://docs.cloud.google.com/bigquery/docs/tables):

| Limit | Default | Notes |
|---|---|---|
| Table modifications per day | 1,500 modifications | Your project can make up to 1,500 table modifications per table per day. A [load job](https://docs.cloud.google.com/bigquery/quotas#load_jobs), [copy job](https://docs.cloud.google.com/bigquery/quotas#copy_jobs), or [query job](https://docs.cloud.google.com/bigquery/quotas#query_jobs) that appends or overwrites table data counts as one modification to the table. This limit cannot be changed. DML statements are excluded and *don't* count toward the number of table modifications per day. Streaming data is excluded and *doesn't* count toward the number of table modifications per day. |
| Maximum rate of table metadata update operations per table | 5 operations per 10 seconds | Your project can make up to five table metadata update operations per 10 seconds per table. This limit applies to all table metadata update operations, performed by the following: - Google Cloud console - The bq command-line tool - BigQuery client libraries - The following API methods: - [`tables.insert`](https://docs.cloud.google.com/bigquery/docs/reference/v2/tables/insert) - [`tables.patch`](https://docs.cloud.google.com/bigquery/docs/reference/v2/tables/patch) - [`tables.update`](https://docs.cloud.google.com/bigquery/docs/reference/v2/tables/update) - [DDL](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language) statements on tables This limit also includes the combined total of all load jobs, copy jobs, and query jobs that append to or overwrite a destination table or that use a [DML](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/dml-syntax) `DELETE`, `INSERT`, `MERGE`, `TRUNCATE TABLE`, or `UPDATE` statements to write data to a table. Note that while DML statements count toward this limit, they are not subject to it if it is reached. DML operations have [dedicated rate limits](https://docs.cloud.google.com/bigquery/quotas#data-manipulation-language-statements). If you exceed this limit, you get an error message like `Exceeded rate limits: too many table update operations for this table`. This error is transient; you can retry with an exponential backoff. To identify the operations that count toward this limit, you can [Inspect your logs](https://docs.cloud.google.com/bigquery/docs/reference/auditlogs#bigqueryauditmetadata_format). Refer to [Troubleshoot quota errors](https://docs.cloud.google.com/bigquery/docs/troubleshoot-quotas#ts-maximum-update-table-metadata-limit) for guidance on diagnosing and resolving this error. |
| Maximum number of columns per table | 10,000 columns | Each table, query result, or view definition can have up to 10,000 columns. This includes nested and repeated columns. |

### External tables

The following limits apply to BigQuery tables with data stored on
Cloud Storage in Parquet, ORC, Avro, CSV, or JSON format:

| Limit | Default | Notes |
|---|---|---|
| Maximum number of source URIs per external table | 10,000 URIs | Each external table can have up to 10,000 source URIs. |
| Maximum number of files per external table | 10,000,000 files | An external table can have up to 10 million files, including all files matching all wildcard URIs. |
| Maximum size of stored data on Cloud Storage per external table | 600 TB | An external table can have up to 600 terabytes across all input files. This limit applies to the file sizes as stored on Cloud Storage; this size is not the same as the size used in the query [pricing](https://cloud.google.com/bigquery/pricing#on_demand_pricing) formula. For [externally partitioned](https://docs.cloud.google.com/bigquery/docs/hive-partitioned-queries-gcs) tables, the limit is applied after [partition pruning](https://docs.cloud.google.com/bigquery/docs/hive-partitioned-queries-gcs#partition_pruning). |
| Maximum number of files in the source Cloud Storage bucket | Approximately 300,000,000 files | An external table can reference a Cloud Storage bucket containing up to approximately 300,000,000 files. For [externally partitioned](https://docs.cloud.google.com/bigquery/docs/hive-partitioned-queries-gcs) tables, this limit is applied before [partition pruning](https://docs.cloud.google.com/bigquery/docs/hive-partitioned-queries-gcs#partition_pruning). |

### Partitioned tables

The following limits apply to BigQuery
[partitioned tables](https://docs.cloud.google.com/bigquery/docs/partitioned-tables).

> [!NOTE]
> **Note:** These limits don't apply to [Hive-partitioned external tables](https://docs.cloud.google.com/bigquery/docs/hive-partitioned-queries).

Partition limits apply to the combined total of all
[load jobs](https://docs.cloud.google.com/bigquery/quotas#load_jobs),
[copy jobs](https://docs.cloud.google.com/bigquery/quotas#copy_jobs), and
[query jobs](https://docs.cloud.google.com/bigquery/quotas#query_jobs)
that append to or overwrite a destination partition.

A single job can affect multiple partitions. For example, query jobs and load
jobs can write to multiple partitions.

BigQuery uses the number of partitions affected by a
job when determining how much of the limit the job consumes. Streaming
inserts do not affect this limit.

For information about strategies to stay within the limits for partitioned
tables, see
[Troubleshooting quota errors](https://docs.cloud.google.com/bigquery/docs/troubleshoot-quotas#ts-number-column-partition-quota).

| Limit | Default | Notes |
|---|---|---|
| Number of partitions per partitioned table | 10,000 partitions | Each partitioned table can have up to 10,000 partitions. If you exceed this limit, consider using [clustering](https://docs.cloud.google.com/bigquery/docs/clustered-tables) in addition to, or instead of, partitioning. |
| Number of partitions modified by a single job | 4,000 partitions | Each job operation (query or load) can affect up to 4,000 partitions. BigQuery rejects any query or load job that attempts to modify more than 4,000 partitions. |
| Number of partition modifications during ingestion-time per partitioned table per day | 11,000 modifications | Your project can make up to 11,000 partition modifications per day. A partition modification is when you append, update, delete, or truncate data in a partitioned table. A partition modification is counted for each type of data modification that you make. For example, deleting one row would count as one partition modification, just as deleting an entire partition would also count as one modification. If you delete a row from one partition and then insert it into another partition, this would count as two partition modifications. Modifications using DML statements or the streaming API don't count toward the number of partition modifications per day. |
| Number of partition modifications per column-partitioned table per day | 30,000 modifications | Your project can make up to 30,000 partition modifications per day for a column-partitioned table. DML statements *do not* count toward the number of partition modifications per day. Streaming data *does not* count toward the number of partition modifications per day. |
| Maximum rate of table metadata update operations per partitioned table | 50 modifications per 10 seconds | Your project can make up to 50 modifications per partitioned table every 10 seconds. This limit applies to all partitioned table metadata update operations, performed by the following: - Google Cloud console - The bq command-line tool - BigQuery client libraries - The following API methods: - [`tables.insert`](https://docs.cloud.google.com/bigquery/docs/reference/v2/tables/insert) - [`tables.patch`](https://docs.cloud.google.com/bigquery/docs/reference/v2/tables/patch) - [`tables.update`](https://docs.cloud.google.com/bigquery/docs/reference/v2/tables/update) - [DDL](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language) statements on tables This limit also includes the combined total of all load jobs, copy jobs, and query jobs that append to or overwrite a destination table or that use a [DML](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/dml-syntax) `DELETE`, `INSERT`, `MERGE`, `TRUNCATE TABLE`, or `UPDATE` statements to write data to a table. If you exceed this limit, you get an error message like `Exceeded rate limits: too many partitioned table update operations for this table`. This error is transient; you can retry with an exponential backoff. To identify the operations that count toward this limit, you can [Inspect your logs](https://docs.cloud.google.com/bigquery/docs/reference/auditlogs#bigqueryauditmetadata_format). |
| Number of possible ranges for range partitioning | 10,000 ranges | A range-partitioned table can have up to 10,000 possible ranges. This limit applies to the partition specification when you create the table. After you create the table, the limit also applies to the actual number of partitions. |

### Table clones

The following limits apply to BigQuery
[table clones](https://docs.cloud.google.com/bigquery/docs/table-clones-intro):

| Limit | Default | Notes |
|---|---|---|
| Maximum number of clones and snapshots in a chain | 3 table clones or snapshots | Clones and snapshots in combination are limited to a depth of 3. When you clone or snapshot a base table, you can clone or snapshot the result only two more times; attempting to clone or snapshot the result a third time results in an error. For example, you can create clone A of the base table, create snapshot B of clone A, and create clone C of snapshot B. To make additional duplicates of the third-level clone or snapshot, use a [copy operation](https://docs.cloud.google.com/bigquery/docs/managing-tables#copy-table) instead. |
| Maximum number of clones and snapshots for a base table | 1,000 table clones or snapshots | You can have no more than 1,000 existing clones and snapshots combined of a given base table. For example, if you have 600 snapshots and 400 clones, you reach the limit. |

### Table snapshots

The following limits apply to BigQuery
[table snapshots](https://docs.cloud.google.com/bigquery/docs/table-snapshots-intro):

| Limit | Default | Notes |
|---|---|---|
| Maximum number of concurrent table snapshot jobs | 100 jobs | Your project can run up to 100 concurrent table snapshot jobs. |
| Maximum number of table snapshot jobs per day | 50,000 jobs | Your project can run up to 50,000 table snapshot jobs per day. |
| Maximum number of table snapshot jobs per table per day | 50 jobs | Your project can run up to 50 table snapshot jobs per table per day. |
| Maximum number of metadata updates per table snapshot per 10 seconds. | 5 updates | Your project can update a table snapshot's metadata up to five times every 10 seconds. |
| Maximum number of clones and snapshots in a chain | 3 table clones or snapshots | Clones and snapshots in combination are limited to a depth of 3. When you clone or snapshot a base table, you can clone or snapshot the result only two more times; attempting to clone or snapshot the result a third time results in an error. For example, you can create clone A of the base table, create snapshot B of clone A, and create clone C of snapshot B. To make additional duplicates of the third-level clone or snapshot, use a [copy operation](https://docs.cloud.google.com/bigquery/docs/managing-tables#copy-table) instead. |
| Maximum number of clones and snapshots for a base table | 1,000 table clones or snapshots | You can have no more than 1,000 existing clones and snapshots combined of a given base table. For example, if you have 600 snapshots and 400 clones, you reach the limit. |

## Views

The following quotas and limits apply to [views](https://docs.cloud.google.com/bigquery/docs/views-intro) and
[materialized views](https://docs.cloud.google.com/bigquery/docs/materialized-views-intro).

### Logical views

The following limits apply to BigQuery standard
[views](https://docs.cloud.google.com/bigquery/docs/views-intro):

| Limit | Default | Notes |
|---|---|---|
| Maximum number of nested view levels | 16 levels | BigQuery supports up to 16 levels of nested views. Creating views up to this limit is possible, but querying is limited to 15 levels. If the limit is exceeded, BigQuery returns an `INVALID_INPUT` error. |
| Maximum length of a GoogleSQL query used to define a view | 256 K characters | A single GoogleSQL query that defines a view can be up to 256 K characters long. This limit applies to a single query and does not include the length of the views referenced in the query. |
| Maximum number of authorized views per dataset |   | See [Datasets](https://docs.cloud.google.com/bigquery/quotas#auth_views_in_dataset_acl). |
| Maximum length of a view description | 16,384 characters | When you add a description to a view, the text can be at most 16,384 characters. |

### Materialized views

The following limits apply to BigQuery
[materialized views](https://docs.cloud.google.com/bigquery/docs/materialized-views-intro):

| Limit | Default | Notes |
|---|---|---|
| Base table references (same project) | 100 materialized views | Each base table can be referenced by up to 100 materialized views from the same project. |
| Base table references (entire organization) | 500 materialized views | Each base table can be referenced by up to 500 materialized views from the entire organization. |
| Maximum number of authorized views per dataset |   | See [Datasets](https://docs.cloud.google.com/bigquery/quotas#auth_views_in_dataset_acl). |
| Maximum length of a materialized view description | 16,384 characters | When you add a description to a materialized view, the text can be at most 16,384 characters. |
| Materialized view refresh job execution-time limit | 12 hours | A [materialized view refresh job](https://docs.cloud.google.com/bigquery/docs/materialized-views-monitor) can run for up to 12 hours before it fails. |

## Search indexes

The following limits apply to BigQuery
[search indexes](https://docs.cloud.google.com/bigquery/docs/search-intro):

| Limit | Default | Notes |
|---|---|---|
| Number of `CREATE INDEX` DDL statements per project per region per day | 500 operations | Your project can issue up to 500 `CREATE INDEX` DDL operations every day within a region. |
| Number of search index DDL statements per table per day | 20 operations | Your project can issue up to 20 `CREATE INDEX` or `DROP INDEX` DDL operations per table per day. |
| Maximum total size of table data per organization allowed for search index creation that does not run in a reservation | 100 TB in multi-regions; 20 TB in all other regions | You can create a search index for a table if the overall size of tables with indexes in your organization is below your region's limit: 100 TB for the `US` and `EU` multi-regions, and 20 TB for all other regions. If your index-management jobs run in [your own reservation](https://docs.cloud.google.com/bigquery/docs/search-index#use_your_own_reservation), then this limit doesn't apply. |
| Number of columns indexed with column granularity per table | 63 columns per table | A table can have up to 63 columns with `index_granularity` set to `COLUMN`. Columns indexed with `COLUMN` granularity from setting the `default_index_column_granularity` option count towards this limit. There is no limit on the number of columns that are indexed with `GLOBAL` granularity. For more information, see [index with column granularity](https://docs.cloud.google.com/bigquery/docs/search-index#column-granularity). |

<br />

## Vector indexes

The following limits apply to BigQuery
[vector indexes](https://docs.cloud.google.com/bigquery/docs/vector-search-intro):

| Limit | Default | Notes |
|---|---|---|
| Base table minimum number of rows | 5,000 rows | A table must have at least 5,000 rows to create a vector index. |
| Base table maximum number of rows for index type `IVF` | 10,000,000,000 rows | A table can have at most 10,000,000,000 rows to create an `IVF` vector index |
| Base table maximum number of rows for index type `TREE_AH` | 200,000,000 rows | A table can have at most 200,000,000 rows to create an `TREE_AH` vector index |
| Base table maximum number of rows for partitioned index type `TREE_AH` | 10,000,000,000 rows in total 200,000,000 rows for each partition | A table can have at most 10,000,000,000 rows, and each partition can have at most 200,000,000 rows to create a `TREE_AH` partitioned vector index. |
| Maximum size of the array in the indexed column | 4,096 elements | The column to index can have at most 4,096 elements in the array. |
| Minimum table size for vector index population | 10 MB | If you create a vector index on a table that is under 10 MB, then the index is not populated. Similarly, if you delete data from a vector-indexed table such that the table size is under 10 MB, then the vector index is temporarily disabled. This happens regardless of whether you use your own reservation for your index-management jobs. Once a vector-indexed table's size again exceeds 10 MB, its index is populated automatically. |
| Number of `CREATE VECTOR INDEX` DDL statements per project per region per day | 500 operations | For each project, you can issue up to 500 `CREATE VECTOR INDEX` operations per day for each region. |
| Number of vector index DDL statements per table per day | 10 operations | You can issue up to 10 `CREATE VECTOR INDEX` or `DROP VECTOR INDEX` operations per table per day. |
| Maximum total size of table data per organization allowed for vector index creation that does not run in a reservation | 6 TB | You can create a vector index for a table if the total size of tables with indexes in your organization is under 6 TB. If your index-management jobs run in [your own reservation](https://docs.cloud.google.com/bigquery/docs/vector-index#use_your_own_reservation), then this limit doesn't apply. |

<br />

## Routines

The following quotas and limits apply to [routines](https://docs.cloud.google.com/bigquery/docs/routines).

### User-defined functions

The following limits apply to both temporary and persistent
[user-defined functions (UDFs)](https://docs.cloud.google.com/bigquery/docs/user-defined-functions) in GoogleSQL queries.

> [!NOTE]
> **Note:** UDFs and the tables they reference count toward the limit on the [number of resources referenced
> in a query](https://docs.cloud.google.com/bigquery/quotas#tables_referenced_per_query).

| Limit | Default | Notes |
|---|---|---|
| Maximum output per row | 5 MB | The maximum amount of data that your JavaScript UDF can output when processing a single row is approximately 5 MB. |
| Maximum concurrent legacy SQL queries with Javascript UDFs | 6 queries | Your project can have up to six concurrent legacy SQL queries that contain UDFs in JavaScript. This limit includes both interactive and [batch](https://docs.cloud.google.com/bigquery/docs/running-queries#batch) queries. This limit does not apply to GoogleSQL queries. |
| Maximum JavaScript UDF resources per query | 50 resources | A query job can have up to 50 JavaScript UDF resources, such as inline code blobs or external files. |
| Maximum size of inline code blob | 32 KB | An inline code blob in a UDF can be up to 32 KB in size. |
| Maximum size of each external code resource | 1 MB | The maximum size of each JavaScript code resource is one MB. |

The following limits apply to persistent UDFs:

| Limit | Default | Notes |
|---|---|---|
| Maximum length of a UDF name | 256 characters | A UDF name can be up to 256 characters long. |
| Maximum number of arguments | 256 arguments | A UDF can have up to 256 arguments. |
| Maximum length of an argument name | 128 characters | A UDF argument name can be up to 128 characters long. |
| Maximum depth of a UDF reference chain | 16 references | A UDF reference chain can be up to 16 references deep. |
| Maximum depth of a `STRUCT` type argument or output | 15 levels | A `STRUCT` type UDF argument or output can be up to 15 levels deep. |
| Maximum number of fields in `STRUCT` type arguments or output per UDF | 1,024 fields | A UDF can have up to 1024 fields in `STRUCT` type arguments and output. |
| Maximum number of JavaScript libraries in a `CREATE FUNCTION` statement | 50 libraries | A `CREATE FUNCTION` statement can have up to 50 JavaScript libraries. |
| Maximum length of included JavaScript library paths | 5,000 characters | The path for a JavaScript library included in a UDF can be up to 5,000 characters long. |
| Maximum update rate per UDF per 10 seconds | 5 updates | Your project can update a UDF up to five times every 10 seconds. |
| Maximum number of authorized UDFs per dataset |   | See [Datasets](https://docs.cloud.google.com/bigquery/quotas#auth_views_in_dataset_acl). |
| Python UDF image storage bytes per project per region | 10 GiB | The total size in bytes of all container images used by Python UDFs in a specific project and region. |
| Python UDF mutation limit | 30 per minute | You can create or update Python UDFs up to 30 times per minute per region per project. |

### Remote functions

The following limits apply to
[remote functions](https://docs.cloud.google.com/bigquery/docs/remote-functions) in
BigQuery.

For troubleshooting information, see [Maximum number of concurrent queries that
contain remote functions](https://docs.cloud.google.com/bigquery/docs/troubleshoot-quotas#ts-maximum-number-of-concurrent-remote-functions).

| Limit | Default | Notes |
|---|---|---|
| Maximum number of concurrent queries that contain remote functions | 10 queries | You can run up to ten concurrent queries with [remote functions](https://docs.cloud.google.com/bigquery/docs/remote-functions) per project. |
| Maximum input size | 5 MB | The maximum total size of all input arguments from a single row is 5 MB. |
| HTTP response size limit (Cloud Run functions 1st gen) | 10 MB | HTTP response body from your Cloud Run function 1st gen is up to 10 MB. Exceeding this value causes query failures. |
| HTTP response size limit (Cloud Run functions 2nd gen or Cloud Run) | 15 MB | HTTP response body from your Cloud Run function 2nd gen or Cloud Run is up to 15 MB. Exceeding this value causes query failures. |
| Max HTTP invocation time limit (Cloud Run functions 1st gen) | 9 minutes | You can set your own time limit for your Cloud Run function 1st gen for an individual HTTP invocation, but the max time limit is [9 minutes](https://docs.cloud.google.com/functions/quotas#time_limits). Exceeding the time limit set for your Cloud Run function 1st gen can cause HTTP invocation failures and query failure. |
| HTTP invocation time limit (Cloud Run functions 2nd gen or Cloud Run) | 20 minutes | The time limit for an individual HTTP invocation to your Cloud Run function 2nd gen or Cloud Run. Exceeding this value can cause HTTP invocation failures and query failure. |
| Maximum number of HTTP invocation retry attempts | 20 | The maximum number of retry attempts for an individual HTTP invocation to your Cloud Run function 1st gen, 2nd gen, or Cloud Run. Exceeding this value can cause HTTP invocation failures and query failure. |

### Table functions

The following limits apply to BigQuery
[table functions](https://docs.cloud.google.com/bigquery/docs/table-functions):

| Limit | Default | Notes |
|---|---|---|
| Maximum length of a table function name | 256 characters | The name of a table function can be up to 256 characters in length. |
| Maximum length of an argument name | 128 characters | The name of a table function argument can be up to 128 characters in length. |
| Maximum number of arguments | 256 arguments | A table function can have up to 256 arguments. |
| Maximum depth of a table function reference chain | 16 references | A table function reference chain can be up to 16 references deep. |
| Maximum depth of argument or output of type `STRUCT` | 15 levels | A `STRUCT` argument for a table function can be up to 15 levels deep. Similarly, a `STRUCT` record in a table function's output can be up to 15 levels deep. |
| Maximum number of fields in argument or return table of type `STRUCT` per table function | 1,024 fields | A `STRUCT` argument for a table function can have up to 1,024 fields. Similarly, a `STRUCT` record in a table function's output can have up to 1,024 fields. |
| Maximum number of columns in return table | 1,024 columns | A table returned by a table function can have up to 1,024 columns. |
| Maximum length of return table column names | 128 characters | Column names in returned tables can be up to 128 characters long. |
| Maximum number of updates per table function per 10 seconds | 5 updates | Your project can update a table function up to five times every 10 seconds. |

### Stored procedures for Apache Spark

The following limits apply for [BigQuery stored procedures for
Apache Spark](https://docs.cloud.google.com/bigquery/docs/spark-procedures):

| **Limit** | **Default** | **Notes** |
|---|---|---|
| Maximum number of concurrent stored procedure queries | 50 | You can run up to 50 concurrent stored procedure queries for each project. |
| Maximum number of in-use CPUs | 12,000 | You can use up to 12,000 CPUs for each project. Queries that have already been processed don't consume this limit. You can use up to 2,400 CPUs for each location for each project, except in the following locations: - `asia-south2` - `australia-southeast2` - `europe-central2` - `europe-west8` - `northamerica-northeast2` - `southamerica-west1` In these locations, you can use up to 500 CPUs for each location for each project. If you run concurrent queries in a multi-region location and a single region location that is in the same geographic area, then your queries might consume the same concurrent CPU quota. |
| Maximum total size of in-use standard persistent disks | 204.8 TB | You can use up to 204.8 TB standard persistent disks for each location for each project. Queries that have already been processed don't consume this limit. If you run concurrent queries in a multi-region location and a single region location that is in the same geographic area, then your queries might consume the same standard persistent disk quota. |

## Notebooks

All [Dataform quotas and limits](https://docs.cloud.google.com/dataform/docs/quotas) and
[Colab Enterprise quotas and limits](https://docs.cloud.google.com/colab/docs/quotas) apply to
[notebooks in BigQuery](https://docs.cloud.google.com/bigquery/docs/notebooks-introduction).
The following limits also apply:

| **Limit** | **Default** | **Notes** |
|---|---|---|
| Maximum notebook size | 20 MB | A notebook's size is the total of its content, metadata, and encoding overhead. You can view the size of notebook content by expanding the notebook header, clicking **View** , and then clicking **Notebook info**. |
| Maximum number of requests per second to Dataform | 100 | Notebooks are created and managed through Dataform. Any action that creates or modifies a notebook counts against this quota. This quota is shared with saved queries. For example, if you make 50 changes to notebooks and 50 changes to saved queries within 1 second, you reach the quota. |

## Saved queries

All [Dataform quotas and limits](https://docs.cloud.google.com/dataform/docs/quotas) apply to
[saved queries](https://docs.cloud.google.com/bigquery/docs/saved-queries-introduction).
The following
limits also apply:

| **Limit** | **Default** | **Notes** |
|---|---|---|
| Maximum saved query size | 10 MB |   |
| Maximum number of requests per second to Dataform | 100 | Saved queries are created and managed through Dataform. Any action that creates or modifies a saved query counts against this quota. This quota is shared with notebooks. For example, if you make 50 changes to notebooks and 50 changes to saved queries within 1 second, you reach the quota. |

## Data manipulation language

The following limits apply for BigQuery
[data manipulation language (DML)](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-manipulation-language)
statements:

| Limit | Default | Notes |
|---|---|---|
| DML statements per day | Unlimited | The number of DML statements your project can run per day is unlimited. <br /> DML statements *do not* count toward the number of [table modifications per day](https://docs.cloud.google.com/bigquery/quotas#load_job_per_table.long) or the number of [partitioned table modifications per day](https://docs.cloud.google.com/bigquery/quotas#load_job_per_partitioned_table.long) for partitioned tables. DML statements have the following [limitations](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-manipulation-language#dml-limitations) to be aware of. |
| Concurrent `INSERT` DML statements per table per day | 1,500 statements | The first 1,500 `INSERT` statements run immediately after they are submitted. After this limit is reached, the concurrency of `INSERT` statements that write to a table is limited to 10. Additional `INSERT` statements are added to a `PENDING` queue. Up to 100 `INSERT` statements can be queued against a table at any given time. When an `INSERT` statement completes, the next `INSERT` statement is removed from the queue and run. <br /> If you must run DML `INSERT` statements more frequently, consider streaming data to your table using the [Storage Write API](https://docs.cloud.google.com/bigquery/docs/write-api). |
| Concurrent mutating DML statements per table | 2 statements | BigQuery runs up to two concurrent mutating DML statements (`UPDATE`, `DELETE`, and `MERGE`) for each table. Additional mutating DML statements for a table are queued. |
| Queued mutating DML statements per table | 20 statements | A table can have up to 20 mutating DML statements in the queue waiting to run. If you submit additional mutating DML statements for the table, then those statements fail. |
| Maximum time in queue for DML statement | 7 hours | An interactive priority DML statement can wait in the queue for up to seven hours. If the statement has not run after seven hours, it fails. |
| Maximum rate of DML statements for each table | 25 statements every 10 seconds | Your project can run up to 25 DML statements every 10 seconds for each table. Both `INSERT` and mutating DML statements contribute to this limit. |

<br />

For more information about mutating DML statements, see
[`INSERT` DML concurrency](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-manipulation-language#insert_dml_concurrency) and
[`UPDATE, DELETE, MERGE` DML concurrency](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-manipulation-language#update_delete_merge_dml_concurrency).

## Multi-statement queries

The following limits apply to
[multi-statement queries](https://docs.cloud.google.com/bigquery/docs/multi-statement-queries) in
BigQuery.

| Limit | Default | Notes |
|---|---|---|
| Maximum number of concurrent multi-statement queries | 1,000 multi-statement queries | Your project can run up to 1,000 concurrent [multi-statement queries](https://docs.cloud.google.com/bigquery/docs/multi-statement-queries). |
| Cumulative time limit | 24 hours | The cumulative time limit for a multi-statement query is 24 hours. |
| Statement time limit | 6 hours | The time limit for an individual statement within a multi-statement query is 6 hours. |

## Recursive CTEs in queries

The following limits apply to
[recursive common table expressions (CTEs)](https://docs.cloud.google.com/bigquery/docs/recursive-ctes) in
BigQuery.

| Limit | Default | Notes |
|---|---|---|
| Iteration limit | 500 iterations | The recursive CTE can execute this number of iterations. If this limit is exceeded, an error is produced. To work around iteration limits, see [Troubleshoot iteration limit errors](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/recursive-ctes#troubleshoot). |

## Row-level security

The following limits apply for BigQuery
[row-level access policies](https://docs.cloud.google.com/bigquery/docs/row-level-security-intro):

| **Limit** | **Default** | **Notes** |
|---|---|---|
| Maximum number of row-access policies per table | 400 policies | A table can have up to 400 row-access policies. |
| Maximum number of row-access policies per query | 6000 policies | A query can access up to a total of 6000 row-access policies. |
| Maximum number of `CREATE` / `DROP` DDL statements per policy per 10 seconds | 5 statements | Your project can make up to five `CREATE` or `DROP` statements per row-access policy resource every 10 seconds. |
| `DROP ALL ROW ACCESS POLICIES` statements per table per 10 seconds | 5 statements | Your project can make up to five `DROP ALL ROW ACCESS POLICIES` statements per table every 10 seconds. |

## Data policies

The following limits apply for
[column-level dynamic data masking](https://docs.cloud.google.com/bigquery/docs/column-data-masking-intro):

| Limit | Default | Notes |
|---|---|---|
| Maximum number of data policies per policy tag. | 8 policies per policy tag | Up to eight data policies per policy tag. One of these policies can be used for [column-level access controls](https://docs.cloud.google.com/bigquery/docs/column-level-security#set_up_column-level_access_control). Duplicate masking expressions are not supported. |

## BigQuery ML

The following limits apply to BigQuery ML.

### Query jobs

All [query job quotas and limits](https://docs.cloud.google.com/bigquery/quotas#query_jobs) apply to GoogleSQL
query jobs that use BigQuery ML statements and functions.

### `CREATE MODEL` statements

The following limits apply to
[`CREATE MODEL`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create)
jobs:

| Limit | Default | Notes |
|---|---|---|
| `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create` statement queries per 48 hours for each project | 20,000 statement queries | Some models are trained by utilizing [Vertex AI services](https://docs.cloud.google.com/vertex-ai/docs/start/introduction-unified-platform), which have their own [resource and quota management](https://docs.cloud.google.com/vertex-ai/docs/quotas). |
| Execution-time limit | 24 hours or 48 hours | `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create` job timeout defaults to 24 hours, with the exception of time series, AutoML, and hyperparameter tuning jobs which timeout at 48 hours. |

### Generative AI functions

The following limits apply to functions that use Vertex AI large
language models (LLMs). For more information, see
[Function quota definitions](https://docs.cloud.google.com/bigquery/quotas#function_quota_definitions).

#### Requests per minute limits

The following limits apply to Vertex AI models that use a
requests per minute limit.

| Function | Model | Region | Requests per minute | Rows per job |
|---|---|---|---|---|
| `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-generate-text` `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-generate-text` `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-generate-table` `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-generate` `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-generate-bool` `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-generate-double` `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-generate-int` | `gemini-2.0-flash-lite-001` | `US` and `EU` multi-regions Single regions as documented for `gemini-2.0-flash-lite-001` in [Google model endpoint locations](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/learn/locations#google_model_endpoint_locations) | No set quota. Quota determined by [dynamic shared quota (DSQ)^1^](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/dynamic-shared-quota) and [Provisioned Throughput^2^](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/provisioned-throughput/overview) | N/A for Provisioned Throughput 10,500,000 for DSQ, for a call with an average of 500 input tokens and 50 output tokens |
| `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-generate-text` `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-generate-text` `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-generate-table` `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-generate` `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-generate-bool` `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-generate-double` `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-generate-int` | `gemini-2.0-flash-001` | `US` and `EU` multi-regions Single regions as documented for `gemini-2.0-flash-001` in [Google model endpoint locations](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/learn/locations#google_model_endpoint_locations) | No set quota. Quota determined by [dynamic shared quota (DSQ)^1^](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/dynamic-shared-quota) and [Provisioned Throughput^2^](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/provisioned-throughput/overview) | N/A for Provisioned Throughput 10,200,000 for DSQ, for a call with an average of 500 input tokens and 50 output tokens |
| `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-generate-text` `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-generate-text` `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-generate-table` `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-generate` `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-generate-bool` `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-generate-double` `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-generate-int` | `gemini-2.5-flash` | `US` and `EU` multi-regions Single regions as documented for `gemini-2.5-flash` in [Google model endpoint locations](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/learn/locations#google_model_endpoint_locations) | No set quota. Quota determined by [dynamic shared quota (DSQ)^1^](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/dynamic-shared-quota) and [Provisioned Throughput^2^](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/provisioned-throughput/overview) | N/A for Provisioned Throughput 9,300,000 for DSQ, for a call with an average of 500 input tokens and 50 output tokens |
| `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-generate-text` `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-generate-text` `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-generate-table` `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-generate` `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-generate-bool` `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-generate-double` `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-generate-int` | `gemini-2.5-pro` | `US` and `EU` multi-regions Single regions as documented for `gemini-2.5-pro` in [Google model endpoint locations](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/learn/locations#google_model_endpoint_locations) | No set quota. Quota determined by [dynamic shared quota (DSQ)^1^](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/dynamic-shared-quota) and [Provisioned Throughput^2^](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/provisioned-throughput/overview) | N/A for Provisioned Throughput 7,600,000 for DSQ, for a call with an average of 500 input tokens and 50 output tokens |
| `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-if` `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-score` `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-classify` | Various `gemini-2.5-*` models | `US` and `EU` multi-regions Any single region supported for one of the `gemini-2.5-* models` in [Google model endpoint locations](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/learn/locations#google_model_endpoint_locations) | No set quota. Quota determined by [dynamic shared quota (DSQ)^1^](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/dynamic-shared-quota) | 10,000,000 for a call with an average of 500 tokens in each input row and 50 output tokens. |
| `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-generate-text` `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-generate-text` | Anthropic Claude | See [Quotas by model and region](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/partner-models/claude/use-claude#quotas) | See [Quotas by model and region](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/partner-models/claude/use-claude#quotas) | The requests per minute value \* 60 \* 6 |
| `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-generate-text` `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-generate-text` | Llama | See [Llama model region availability and quotas](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/partner-models/llama/use-llama#regions-quotas) | See [Llama model region availability and quotas](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/partner-models/llama/use-llama#regions-quotas) | The requests per minute value \* 60 \* 6 |
| `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-generate-text` `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-generate-text` | Mistral AI | See [Mistral AI model region availability and quotas](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/partner-models/mistral#regions-quotas) | See [Mistral AI model region availability and quotas](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/partner-models/mistral#regions-quotas) | The requests per minute value \* 60 \* 6 |
| `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-generate-embedding` `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-embed` `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-similarity` `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-search` `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/search_functions#vector_search` `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-generate-embedding` | `text-embedding` `text-multilingual-embedding` | [All regions that support remote models](https://docs.cloud.google.com/bigquery/docs/locations#locations-for-remote-models) | 1,500^3,4^ | 80,000,000 for a call with an average of 50 tokens in each input row 14,000,000 for a call with an average of 600 tokens in each input row |
| `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-generate-embedding` `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-embed` `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-similarity` `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-search` `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/search_functions#vector_search` `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-generate-embedding` | `multimodalembedding` | [Supported European single regions](https://docs.cloud.google.com/bigquery/docs/locations#regions) | 120^3^ | 43,200 |
| `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-generate-embedding` `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-embed` `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-similarity` `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-search` `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/search_functions#vector_search` `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-generate-embedding` | `multimodalembedding` | Regions other than [supported European single regions](https://docs.cloud.google.com/bigquery/docs/locations#regions) | 600^3^ | 216,000 |
| `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-generate-embedding` `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-embed` `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-similarity` `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-search` `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/search_functions#vector_search` `https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-generate-embedding` | `gemini-embedding-2-preview` | `us-central1` and `US` multi-region | 4,000^3^ | 1,440,000 |

^1^ When you use DSQ, there are no predefined quota limits on your
usage. Instead, DSQ provides access to a large shared pool of resources, which
are dynamically allocated based on real-time availability of resources and the
customer demand for the given model. When more customers are active, each
customer gets less throughput. Similarly, when fewer customers are active, each
customer might get higher throughput.

^2^ Provisioned Throughput is a fixed-cost, fixed-term
subscription available in several term-lengths.
Provisioned Throughput lets you reserve throughput for supported
generative AI models on Vertex AI.

^3^ To increase the quota, request a
[QPM quota adjustment](https://docs.cloud.google.com/docs/quotas/view-manage#requesting_higher_quota) in
Vertex AI. Allow 30 minutes for the increased quota value to
propagate.

^4^ You can increase the quota for Vertex AI `text-embedding` and `text-multilingual-embedding` models to 10,000 RPM without manual approval. This results in increased throughput of 500,000,000 rows per job or more, based on a call with an average of 50 tokens in each input row.

^5^ This function is limited to a maximum of 5 concurrently running
jobs per project.

For more information about quota for Vertex AI LLMs, see
[Generative AI on Vertex AI quota limits](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/quotas).

#### Tokens per minute limits

The following limits apply to Vertex AI models that use a
tokens per minute limit:

| **Function** | **Tokens per minute** | **Rows per job** | **Number of concurrently running jobs** |
|---|---|---|---|
| [`AI.GENERATE_EMBEDDING`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-generate-embedding) or [`ML.GENERATE_EMBEDDING`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-generate-embedding) when using a remote model over a `gemini-embedding-001` model | 10,000,000 | 12,000,000, for a call with an average of 300 tokens per row | 5 |
| [`AI.GENERATE_EMBEDDING`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-generate-embedding) or [`ML.GENERATE_EMBEDDING`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-generate-embedding) when using a remote model over a `gemini-embedding-2-preview` model | 5,000,000 | 1,440,000 | 5 |

### Cloud AI service functions

The following limits apply to functions that use Cloud AI services:

| **Function** | **Requests per minute** | **Rows per job** | **Number of concurrently running jobs** |
|---|---|---|---|
| [`ML.PROCESS_DOCUMENT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-process-document) with documents averaging fifty pages | 600 | 100,000 (based on an average of 50 pages in each input document) | 5 |
| [`ML.TRANSCRIBE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-transcribe) | 200 | 10,000 (based on an average length of 1 minute for each input audio file) | 5 |
| [`ML.ANNOTATE_IMAGE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-annotate-image) | 1,800 | 648,000 | 5 |
| [`ML.TRANSLATE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-translate) | 6,000 | 2,160,000 | 5 |
| [`ML.UNDERSTAND_TEXT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-understand-text) | 600 | 21,600 | 5 |

For more information about quota for Cloud AI service APIs, see the following
documents:

- [Cloud Translation API quota and limits](https://docs.cloud.google.com/translate/quotas)
- [Vision API quota and limits](https://docs.cloud.google.com/vision/quotas)
- [Natural Language API quota and limits](https://docs.cloud.google.com/natural-language/quotas)
- [Document AI quota and limits](https://docs.cloud.google.com/document-ai/quotas)
- [Speech-to-Text quota and limits](https://docs.cloud.google.com/speech-to-text/quotas)

### Function quota definitions

The following list describes the quotas that apply to generative AI and Cloud
AI service functions:

- Functions that call a Vertex AI model use one Vertex AI quota, which is queries per minute (QPM). In this context, the queries are request calls from the function to the Vertex AI model's API. The QPM quota applies to a base model and all versions, identifiers, and tuned versions of that model. For more information on the Vertex AI model quotas, see [Generative AI on Vertex AI quota limits](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/quotas).
- Functions that call a Cloud AI service use the target service's request quotas. Check the given Cloud AI service's quota reference for details.
- BigQuery ML uses the following quotas:

  - **Requests per minute**. This quota is the limit on the number of request
    calls per minute that functions can make to the Vertex AI
    model's or Cloud AI service's API. This limit applies to each project
    and is shared among all jobs using the same model endpoint.

    Calls to Vertex AI Gemini models have no
    predefined quota limits on your usage, because Gemini
    models use
    [dynamic shared quota (DSQ)](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/dynamic-shared-quota).
    DSQ provides access to a large shared pool of resources, which are
    dynamically allocated based on real-time availability of resources
    and the customer demand for the given model.
  - **Tokens per minute**. This quota is the limit on the number of tokens
    per minute that functions can send to the Vertex AI
    model's API. This limit applies to each project.

    For functions that call a Vertex AI foundation model,
    the number of tokens per minute varies depending on the
    Vertex AI model endpoint, version, and region, and also
    your project's reputation. This quota is conceptually the same as
    the QPM quota used by Vertex AI.
  - **Rows per job** . The `Rows per job` value serves as a performance
    benchmark, approximating the processing capacity when a single job has
    exclusive use of the project's model endpoint resources. The actual
    number of processed rows depends on many factors, including the size of
    the input request to the model, the size of output responses from the
    model, and availability of dynamic shared quota. The following examples
    show some common scenarios:

    - For the `gemini-2.0-flash-lite-001` endpoint, the number of rows
      processable by the `AI.GENERATE_TEXT` or
      `ML.GENERATE_TEXT` function depends on input and
      output token counts. The service can process approximately 7.6 million
      rows for calls that have an average input token count of 2,000 and a
      maximum output token count of 50. This number decreases to about 1
      million rows if the average input token count is 10,000 and the maximum
      output token count is 3,000.

      Similarly, the `gemini-2.0-flash-001` endpoint can process 4.4 million
      rows for calls that have an average input token count of 2,000 and a
      maximum output token count of 50, but only about 1 million rows with
      for calls with 10,000 input and 3,000 output tokens.
    - The `ML.PROCESS_DOCUMENT` function can process more rows per job for
      short documents as opposed to long documents.

    - The `ML.TRANSCRIBE` function can process more rows per job for
      short audio clips as opposed to long audio clips.

  - **Number of concurrently running jobs**. This quota is the
    limit per project on the number of SQL queries that can run at the same
    time for the given function.

The following examples show how to interpret quota limitations in typical
situations:

- I have a quota of 1,000 QPM in Vertex AI, so a query with
  100,000 rows should take around 100 minutes. Why is the job running longer?

  Job runtimes can vary even for the same input data. In
  Vertex AI, remote procedure calls (RPCs) have different
  priorities in order to avoid quota drainage. When there isn't enough quota,
  RPCs with lower priorities wait and possibly fail if it takes too long to
  process them.
- How should I interpret the rows per job quota?

  In BigQuery, a query can execute for up to six hours. The
  maximum supported rows is a function of this timeline and your
  Vertex AI QPM quota, in order to make sure that
  BigQuery can complete query processing in six hours. Since
  typically a query can't use the whole quota, this is a lower
  number than your QPM quota multiplied by 360.
- What happens if I run a batch inference job on a table with more
  rows than the rows per job quota, for example 10,000,000 rows?

  BigQuery only processes the number of rows specified by the
  rows per job quota. You are only charged for the successful API calls for
  that number of rows, instead of the full 10,000,000 rows in your table. For
  the rest of the rows, BigQuery responds to the request with a
  `A retryable error occurred: the maximum size quota per query has reached`
  error, which is returned in the `status` column of the result. You can use
  this set of [SQL scripts](https://github.com/GoogleCloudPlatform/bigquery-ml-utils/tree/master/sql_scripts/remote_inference) or this [Dataform package](https://github.com/dataform-co/dataform-bqml)
  to iterate through inference calls until all rows
  are successfully processed.
- I have many more rows to process than the rows per job quota. Will
  splitting my rows across multiple queries and running them simultaneously
  help?

  No, because these queries are consuming the same BigQuery ML
  requests per minute quota and Vertex AI QPM quota. If there
  are multiple queries that all stay within the rows per job quota and number
  of concurrently running jobs quota, the cumulative processing exhausts the
  requests per minute quota.

## BigQuery Graph

The following limits apply to [BigQuery Graph](https://docs.cloud.google.com/bigquery/docs/graph-overview):

| Limit | Default | Notes |
|---|---|---|
| Maximum number of tables referenced by a graph | 1,000 tables | A graph can reference up to 1,000 total node and edge tables in its node and edge definitions. |
| Maximum number of key columns per node or edge table | 16 columns | You can define a key that uses up to 16 columns on a node or edge table of a graph. |
| Maximum number of columns per node reference | 16 columns | A source key or destination key can reference up to 16 columns from a node table in a graph. |
| Maximum number of columns per edge reference | 16 columns | An edge's source key or destination key can reference up to 16 columns from the edge table. |
| Maximum number of labels on a graph | 1,000 labels | You can define up to 1,000 total node and edge labels on a graph. |
| Maximum number of labels defined per node or edge | 20 labels | You can define up to 20 labels on a node or edge in a graph. |
| Maximum number of properties defined on a graph | 5,000 properties | You can define up to 5,000 properties on a graph. |
| Maximum number of properties defined per label | 1,000 properties | You can define up to 1,000 properties per label on a graph. |

## BI Engine

The following limits apply to [BigQuery BI Engine](https://docs.cloud.google.com/bigquery/docs/bi-engine-intro).

| Limit | Default | Notes |
|---|---|---|
| Maximum reservation size per project per location ([BigQuery BI Engine](https://docs.cloud.google.com/bigquery/docs/bi-engine-intro)) | 250 GiB | 250 Gib is the default maximum reservation size per project per location. You can [request an increase](https://docs.google.com/forms/d/1KX2E2ggOy1eUNB0Hjf9l9l0Sm0TbmPuS0XvyZtdnRes/viewform) of the maximum reservation capacity for your projects. Reservation increases are available in most regions, and might take 3 or more business days depending on the size of the increase requested. Please contact your Google Cloud representative or Cloud Customer Care for urgent requests. |
| Maximum number of rows per query | 7 billion | Maximum number of rows per query. |

## BigQuery sharing (formerly Analytics Hub)

The following limits apply to [BigQuery sharing (formerly Analytics Hub)](https://docs.cloud.google.com/bigquery/docs/analytics-hub-introduction):

| Limit | Default | Notes |
|---|---|---|
| Maximum number of data exchanges per project | 500 exchanges | You can create up to 500 data exchanges in a project. |
| Maximum number of listings per data exchange | 1,000 listings | You can create up to 1,000 listings in a data exchange. |
| Maximum number of linked datasets per shared dataset | 1,000 linked datasets | All BigQuery sharing subscribers, combined, can have a maximum of 1,000 linked datasets per shared dataset. |

## Knowledge Catalog automatic discovery

The following limits apply to [Knowledge Catalog automatic discovery](https://docs.cloud.google.com/bigquery/docs/automatic-discovery):

| Limit | Default | Notes |
|---|---|---|
| Maximum BigQuery, BigLake, or external tables per Cloud Storage bucket that a discovery scan supports | 1000 BigQuery tables per bucket | You can create up to 1,000 BigQuery tables per Cloud Storage bucket. |

## API quotas and limits

These quotas and limits apply to [BigQuery API](https://docs.cloud.google.com/bigquery/docs/reference/libraries-overview) requests.

### BigQuery API

The following quotas apply to
[BigQuery API](https://docs.cloud.google.com/bigquery/docs/reference/rest) (core)
requests:

| Quota | Default | Notes |
|---|---|---|
| Requests per day | Unlimited | Your project can make an unlimited number of BigQuery API requests per day. [View quota in Google Cloud console](https://console.cloud.google.com/iam-admin/quotas?metric=bigquery.googleapis.com/unlimited_requests) |
| Maximum [`tabledata.list`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tabledata/list) bytes per minute | 7.5 GB in multi-regions; 3.7 GB in all other regions | Your project can return a maximum of 7.5 GB of table row data per minute via `tabledata.list` in the `us` and `eu` multi-regions, and 3.7 GB of table row data per minute in all other regions. This quota applies to the project that contains the table being read. Other APIs including [`jobs.getQueryResults`](https://docs.cloud.google.com/bigquery/docs/reference/v2/jobs/getQueryResults) and fetching results from [`jobs.query`](https://docs.cloud.google.com/bigquery/docs/reference/v2/jobs/query) and [`jobs.insert`](https://docs.cloud.google.com/bigquery/docs/reference/v2/jobs/insert) can also consume this quota. For troubleshooting information, see the [Troubleshooting page](https://docs.cloud.google.com/bigquery/docs/troubleshoot-quotas#ts-maximum-tabledata-list-bytes-per-second-per-project-quota). [View quota in Google Cloud console](https://console.cloud.google.com/iam-admin/quotas?metric=bigquery.googleapis.com/quota/tabledata/list_bytes) The [BigQuery Storage Read API](https://docs.cloud.google.com/bigquery/docs/reference/storage) can sustain significantly higher throughput than `tabledata.list`. If you need more throughput than allowed under this quota, consider using the BigQuery Storage Read API. |

The following limits apply to [BigQuery API](https://docs.cloud.google.com/bigquery/docs/reference/rest)
(core) requests:

> [!NOTE]
> **Note:** While most BigQuery API core methods have a maximum of 100 API requests per user per method, some core methods can have different rate limits.

| Limit | Default | Notes |
|---|---|---|
| Maximum number of API requests per second per user per method | 100 requests | A user can make up to 100 API requests per second to an API method. If a user makes more than 100 requests per second to a method, then throttling can occur. This limit does not apply to [streaming inserts](https://docs.cloud.google.com/bigquery/streaming-data-into-bigquery). <br /> For troubleshooting information, see the [Troubleshooting page](https://docs.cloud.google.com/bigquery/docs/troubleshoot-quotas#ts-maximum-api-request-limit). |
| Maximum number of concurrent API requests per user | 300 requests | If a user makes more than 300 concurrent requests, throttling can occur. This limit does not apply to streaming inserts. |
| Maximum request header size | 16 KiB | Your BigQuery API request can be up to 16 KiB, including the request URL and all headers. This limit does not apply to the request body, such as in a `POST` request. |
| Maximum [`jobs.get`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs/get) requests per second | 1,000 requests | Your project can make up to 1,000 `jobs.get` requests per second. |
| Maximum [`jobs.query`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs/query) response size | 20 MB | By default, there is no maximum row count for the number of rows of data returned by `jobs.query` per page of results. However, you are limited to the 20-MB maximum response size. You can alter the number of rows to return by using the `maxResults` parameter. |
| Maximum [`jobs.getQueryResults`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs/getQueryResults) row size | 20 MB | The maximum row size is approximate because the limit is based on the internal representation of row data. The limit is enforced during transcoding. |
| Maximum [`projects.list`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/projects/list) requests per second | 10 requests | A user can make up to 10 `projects.list` requests per second. |
| Maximum number of [`tabledata.list`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tabledata/list) requests per second | 1,000 requests | Your project can make up to 1,000 `tabledata.list` requests per second. |
| Maximum rows per [`tabledata.list`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tabledata/list) response | 100,000 rows | A `tabledata.list` call can return up to 100,000 table rows. For more information, see [Paging through results using the API](https://docs.cloud.google.com/bigquery/docs/paging-results#paging). |
| Maximum [`tabledata.list`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tabledata/list) row size | 100 MB | The maximum row size is approximate because the limit is based on the internal representation of row data. The limit is enforced during transcoding. |
| Maximum [`tables.insert`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables/insert) requests per second | 10 requests | A user can make up to 10 `tables.insert` requests per second. The `tables.insert` method creates a new, empty table in a dataset. |

### BigQuery Connection API

The following quotas apply to
[BigQuery Connection API](https://docs.cloud.google.com/bigquery/docs/working-with-connections)
requests:

| Quota | Default | Notes |
|---|---|---|
| Read requests per minute | 1,000 requests per minute | Your project can make up to 1,000 requests per minute to BigQuery Connection API methods that read connection data. [View quota in Google Cloud console](https://console.cloud.google.com/iam-admin/quotas?service=bigqueryconnection.googleapis.com&metric=bigqueryconnection.googleapis.com/read_requests) |
| Write requests per minute | 100 requests per minute | Your project can make up to 100 requests per minute to BigQuery Connection API methods that create or update connections. [View quota in Google Cloud console](https://console.cloud.google.com/iam-admin/quotas?service=bigqueryconnection.googleapis.com&metric=bigqueryconnection.googleapis.com/write_requests) |
| BigQuery Omni connections created per minute | 10 connections created per minute | Your project can create up to 10 BigQuery Omni connections total across both AWS and Azure per minute. |
| BigQuery Omni connection uses | 500 connection uses per minute | Your project can use a BigQuery Omni connection up to 500 times per minute. This applies to operations which use your connection to access your AWS account, such as querying a table. |

### BigQuery Migration API

The following limits apply to the
[BigQuery Migration API](https://docs.cloud.google.com/bigquery/docs/reference/migration/rpc):

| Limit | Default | Notes |
|---|---|---|
| Individual file size for batch SQL translation | 10 MB | Each individual source and metadata file can be up to 10 MB. This limit does not apply to the metadata zip file produced by the `dwh-migration-dumper` command-line extraction tool. |
| Total size of source files for batch SQL translation | 1 GB | The total size of all input files uploaded to Cloud Storage can be up to 1 GB. This includes all source files, and all metadata files if you choose to include them. |
| Input string size for interactive SQL translation | 1 MB | The string that you enter for interactive SQL translation must not exceed 1 MB. When running interactive translations using the Translation API, this limit applies to the total size of all string inputs. |
| Maximum configuration file size for interactive SQL translation | 50 MB | Individual metadata files (compressed) and YAML config files in Cloud Storage must not exceed 50 MB. If the file size exceeds 50 MB, the interactive translator skips that configuration file during translation and produces an error message. One method to reduce the metadata file size is to use the `---database` or `--schema` flags to filter on databases when you [generate the metadata](https://docs.cloud.google.com/bigquery/docs/generate-metadata#run-dumper). |
| Maximum number of Gemini suggestions per hour | 1,000 (can accumulate up to 10,000 if not used) | If necessary, you can request a quota increase by contacting [Cloud Customer Care](https://cloud.google.com/support-hub). |

The following quotas apply to the
[BigQuery Migration API](https://docs.cloud.google.com/bigquery/docs/reference/migration/rpc). The following
default values apply in most cases. The defaults for your project might be
different:

| Quota | Default | Notes |
|---|---|---|
| EDWMigration Service List Requests per minute EDWMigration Service List Requests per minute per user | 12,000 requests 2,500 requests | Your project can make up to 12,000 Migration API List requests per minute. Each user can make up to 2,500 Migration API List requests per minute. [View quotas in Google Cloud console](https://console.cloud.google.com/iam-admin/quotas?service=bigquerymigration.googleapis.com&metric=bigquerymigration.googleapis.com/edwmigration_service_list_requests) |
| EDWMigration Service Get Requests per minute EDWMigration Service Get Requests per minute per user | 25,000 requests 2,500 requests | Your project can make up to 25,000 Migration API Get requests per minute. Each user can make up to 2,500 Migration API Get requests per minute. [View quotas in Google Cloud console](https://console.cloud.google.com/iam-admin/quotas?service=bigquerymigration.googleapis.com&metric=bigquerymigration.googleapis.com/edwmigration_service_get_requests) |
| EDWMigration Service Other Requests per minute EDWMigration Service Other Requests per minute per user | 25 requests 5 requests | Your project can make up to 25 other Migration API requests per minute. Each user can make up to 5 other Migration API requests per minute. [View quotas in Google Cloud console](https://console.cloud.google.com/iam-admin/quotas?service=bigquerymigration.googleapis.com&metric=bigquerymigration.googleapis.com/edwmigration_service_other_requests) |
| Interactive SQL translation requests per minute Interactive SQL translation requests per minute per user | 200 requests 50 requests | Your project can make up to 200 SQL translation service requests per minute. Each user can make up to 50 other SQL translation service requests per minute. [View quotas in Google Cloud console](https://console.cloud.google.com/iam-admin/quotas?service=bigquerymigration.googleapis.com&metric=bigquerymigration.googleapis.com/sql_translation_translate_requests) |

### BigQuery Reservation API

The following quotas apply to
[BigQuery Reservation API](https://docs.cloud.google.com/bigquery/docs/reference/reservations/rpc) requests:

| Quota | Default | Notes |
|---|---|---|
| Requests per minute per region | 100 requests | Your project can make a total of up to 100 calls to BigQuery Reservation API methods per minute per region. [View quotas in Google Cloud console](https://console.cloud.google.com/iam-admin/quotas?service=bigqueryreservation.googleapis.com&metric=bigqueryreservation.googleapis.com/requests) |
| Number of `SearchAllAssignments` calls per minute per region | 100 requests | Your project can make up to 100 calls to the [`SearchAllAssignments`](https://docs.cloud.google.com/bigquery/docs/reference/reservations/rest/v1/projects.locations/searchAllAssignments) method per minute per region. [View quotas in Google Cloud console](https://console.cloud.google.com/iam-admin/quotas?service=bigqueryreservation.googleapis.com&metric=bigqueryreservation.googleapis.com/search_all_assignments_requests) |
| Requests for `SearchAllAssignments` per minute per region per user | 10 requests | Each user can make up to 10 calls to the [`SearchAllAssignments`](https://docs.cloud.google.com/bigquery/docs/reference/reservations/rest/v1/projects.locations/searchAllAssignments) method per minute per region. [View quotas in Google Cloud console](https://console.cloud.google.com/iam-admin/quotas?service=bigqueryreservation.googleapis.com&metric=bigqueryreservation.googleapis.com/search_all_assignments_requests) (In the Google Cloud console search results, search for **per user**.) |

### BigQuery Data Policy API

The following limits apply for
the [Data Policy API](https://docs.cloud.google.com/bigquery/docs/reference/bigquerydatapolicy/rest)
([preview](https://cloud.google.com/products/#product-launch-stages)):

| Limit | Default | Notes |
|---|---|---|
| Maximum number of [`dataPolicies.list`](https://docs.cloud.google.com/bigquery/docs/reference/bigquerydatapolicy/rest/v1beta1/projects.locations.dataPolicies/list) calls. | 400 requests per minute per project 600 requests per minute per organization |   |
| Maximum number of [`dataPolicies.testIamPermissions`](https://docs.cloud.google.com/bigquery/docs/reference/bigquerydatapolicy/rest/v1beta1/projects.locations.dataPolicies/testIamPermissions) calls. | 400 requests per minute per project 600 requests per minute per organization |   |
| Maximum number of read requests. | 1200 requests per minute per project 1800 requests per minute per organization | This includes calls to [`dataPolicies.get`](https://docs.cloud.google.com/bigquery/docs/reference/bigquerydatapolicy/rest/v1beta1/projects.locations.dataPolicies/get) and [`dataPolicies.getIamPolicy`](https://docs.cloud.google.com/bigquery/docs/reference/bigquerydatapolicy/rest/v1beta1/projects.locations.dataPolicies/getIamPolicy). |
| Maximum number of write requests. | 600 requests per minute per project 900 requests per minute per organization | This includes calls to: - [`dataPolicies.create`](https://docs.cloud.google.com/bigquery/docs/reference/bigquerydatapolicy/rest/v1beta1/projects.locations.dataPolicies/create) - [`dataPolicies.delete`](https://docs.cloud.google.com/bigquery/docs/reference/bigquerydatapolicy/rest/v1beta1/projects.locations.dataPolicies/delete) - [`dataPolicies.setIamPolicy`](https://docs.cloud.google.com/bigquery/docs/reference/bigquerydatapolicy/rest/v1beta1/projects.locations.dataPolicies/setIamPolicy) - [`dataPolicies.patch`](https://docs.cloud.google.com/bigquery/docs/reference/bigquerydatapolicy/rest/v1beta1/projects.locations.dataPolicies/patch) |

### IAM API

The following quotas apply when you use
[Identity and Access Management](https://docs.cloud.google.com/iam/docs)
features in BigQuery to retrieve and set IAM
policies, and to test IAM permissions.
[Data control language (DCL) statements](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-control-language)
count towards `SetIAMPolicy` quota.

> [!NOTE]
> **Note:** If you are encountering IAM request constraints, we recommend that you evaluate whether your project can use [IAM permission inheritance](https://docs.cloud.google.com/iam/docs/resource-hierarchy-access-control) to alleviate the constraint.

| Quota | Default | Notes |
|---|---|---|
| `IamPolicy` requests per minute per user | 1,500 requests per minute per user | Each user can make up to 1,500 requests per minute per project. [View quota in Google Cloud console](https://console.cloud.google.com/iam-admin/quotas?service=bigquery.googleapis.com&metric=bigquery.googleapis.com/iam_policy_requests) |
| `IamPolicy` requests per minute per project | 3,000 requests per minute per project | Your project can make up to 3,000 requests per minute. [View quota in Google Cloud console](https://console.cloud.google.com/iam-admin/quotas?service=bigquery.googleapis.com&metric=bigquery.googleapis.com/iam_policy_requests) |
| [Single-region](https://docs.cloud.google.com/bigquery/docs/locations#regions) `SetIAMPolicy `requests per minute per project | 1,000 requests per minute per project | Your single-region project can make up to 1,000 requests per minute. [View quota in Google Cloud console](https://console.cloud.google.com/iam-admin/quotas?service=bigquery.googleapis.com&metric=bigquery.googleapis.com/set_iam_policy_request) |
| [Multi-region](https://docs.cloud.google.com/bigquery/docs/locations#multi-regions) `SetIAMPolicy` requests per minute per project | 2,000 requests per minute per project | Your multi-region project can make up to 2,000 requests per minute. [View quota in Google Cloud console](https://console.cloud.google.com/iam-admin/quotas?service=bigquery.googleapis.com&metric=bigquery.googleapis.com/set_iam_policy_requests_global) |
| [Omni-region](https://docs.cloud.google.com/bigquery/docs/locations#omni-loc) `SetIAMPolicy` requests per minute per project | 200 requests per minute per project | Your Omni-region project can make up to 200 requests per minute. [View quota in Google Cloud console](https://console.cloud.google.com/iam-admin/quotas?service=bigquery.googleapis.com&metric=bigquery.googleapis.com/set_iam_policy_requests) |

### Storage Read API

The following quotas apply to
[BigQuery Storage Read API](https://docs.cloud.google.com/bigquery/docs/reference/storage) requests:

| Quota | Default | Notes |
|---|---|---|
| Read data plane requests per minute per user | 25,000 requests | Each user can make up to 25,000 `ReadRows` calls per minute per project. [View quota in Google Cloud console](https://console.cloud.google.com/iam-admin/quotas?metric=bigquerystorage.googleapis.com/data_plane_requests) |
| Maximum concurrent read connections | 2,000 in multi-regions; 400 in regions | Maximum number of concurrent `ReadRows` connections per project. The default is 2,000 connections in the `us` and `eu` multi-regions, and 400 connections in other regions. Actual connection availability can fluctuate based on overall region-wide service load and demand. This dynamic adjustment ensures fair resource distribution and maintains service stability for all users. When a stream is closed for fairness or when you reach the connection limit, you receive a `RESOURCE_EXHAUSTED` error (HTTP 429). Quota increase requests (QIRs) are reviewed based on the project's past usage patterns and the general availability of resources within the region. [View quota in Google Cloud console](https://console.cloud.google.com/iam-admin/quotas?metric=bigquerystorage.googleapis.com/read/max_concurrent_connections) |
| Read control plane requests per minute per user | 5,000 requests | Each user can make up to 5,000 Storage Read API metadata operation calls per minute per project. The metadata calls include the `CreateReadSession` and `SplitReadStream` methods. [View quota in Google Cloud console](https://console.cloud.google.com/iam-admin/quotas?metric=bigquerystorage.googleapis.com/control_plane_requests) |

The following limits apply to
[BigQuery Storage Read API](https://docs.cloud.google.com/bigquery/docs/reference/storage) requests:

| Limit | Default | Notes |
|---|---|---|
| Maximum row/filter length | 1 MB | When you use the Storage Read API `CreateReadSession` call, you are limited to a maximum length of 1 MB for each row or filter. |
| Maximum serialized data size | 128 MB | When you use the Storage Read API `ReadRows` call, the serialized representation of the data in an individual `ReadRowsResponse` message cannot be larger than 128 MB. |
| Maximum per-stream memory usage | 1.5 GB | The maximum per-stream memory is approximate because the limit is based on the internal representation of the row data. Streams utilizing more than 1.5 GB memory for a single row might fail. For more information, see [Troubleshoot resources exceeded issues](https://docs.cloud.google.com/bigquery/docs/troubleshoot-queries#ts-resources-exceeded). |

### Storage Write API

The following quotas apply to
[Storage Write API](https://docs.cloud.google.com/bigquery/docs/write-api) requests. The following quotas can be applied at the folder level. These quotas are then aggregated and shared across all child projects. To enable this configuration, contact [Cloud Customer Care](https://console.cloud.google.com/support/).

> [!NOTE]
> **Note:** Projects that have opted in folder level quota enforcement can only check folder level quota usage and limit in the folder's Google Cloud console quotas page. Project level quota usage and limit won't be displayed. In this case, the project level [monitoring metrics](https://docs.cloud.google.com/monitoring/api/metrics_gcp_a_b#gcp-bigquerystorage) is still a good source for the project level usage.

> [!NOTE]
> **Note:** Due to performance optimization, BigQuery might report greater concurrent connections quota usage than the actual quota usage. The deviation can be up to 1% of the total quota or 100 connections, whichever is smaller, multiplied by a factor of 1-4. That means the reported usage can deviate by at most 400 connections in multi-regions with a 10,000 default quota, and 40 connections in small regions with a 1,000 default quota. The quota enforcement is always based on the actual usage, not the reported value.

If you plan to [request a quota adjustment](https://docs.cloud.google.com/docs/quotas/help/request_increase),
include the quota error message in your request to expedite processing.
BigQuery might reduce your provisioned quota if your quota is
significantly under-utilized for more than one year.

| Quota | Default | Notes |
|---|---|---|
| Concurrent write connections | 5,000 in a region; 20,000 in a multi-region | The concurrent connections quota is based on the client project that initiates the Storage Write API request, not the project containing the BigQuery dataset resource. The initiating project is the project associated with the [API key](https://docs.cloud.google.com/docs/authentication/api-keys) or the [service account](https://docs.cloud.google.com/iam/docs/understanding-service-accounts). Your project can operate on 5,000 concurrent connections in a region, or 20,000 concurrent connections in the `US` and `EU` multi-regions. A connection should be long lived and used to send as many requests as possible. Use of short-lived connections is discouraged and could cause inflated concurrent connection quota usage. For quota accounting purposes, we suggest a connection lifetime of at least several minutes. When you use the [default stream](https://docs.cloud.google.com/bigquery/docs/write-api#default_stream) in Java or Go, we recommend using [Storage Write API multiplexing](https://docs.cloud.google.com/bigquery/docs/write-api-best-practices#connection_pool_management) to write to multiple destination tables with shared connections in order to reduce the number of overall connections that are needed. If you are using the [Beam connector with at-least-once semantics](https://beam.apache.org/documentation/io/built-in/google-bigquery/#at-least-once-semantics), you can set [UseStorageApiConnectionPool](https://beam.apache.org/releases/javadoc/current/org/apache/beam/sdk/io/gcp/bigquery/BigQueryOptions.html#setUseStorageApiConnectionPool-java.lang.Boolean-) to `TRUE` to enable multiplexing. [View quota in Google Cloud console](https://console.cloud.google.com/iam-admin/quotas?metric=bigquerystorage.googleapis.com/write/max_active_streams) You can view usage quota and limits metrics for your projects in [Cloud Monitoring](https://docs.cloud.google.com/bigquery/docs/monitoring-dashboard#view_quota_usage_and_limits). Select the concurrent connections limit name based on your region. The options are `ConcurrentWriteConnectionsPerProject`, `ConcurrentWriteConnectionsPerProjectEU`, and `ConcurrentWriteConnectionsPerProjectRegion` for `us`, `eu`, and other regions, respectively. <br /> It is strongly recommended that you set up [alerts](https://docs.cloud.google.com/monitoring/alerts/using-quota-metrics) to monitor your quota usage and limits. In addition, if your traffic patterns experience spikes and/or regular organic growth, it might be beneficial to consider over-provisioning your quota by 25 - 50% in order to handle unexpected demand. |
| Throughput | 3 GB per second throughput in multi-regions; 300 MB per second in regions | You can stream up to 3 GBps in the `us` and `eu` multi-regions, and 300 MBps in other regions per project. [View quota in Google Cloud console](https://console.cloud.google.com/iam-admin/quotas?metric=bigquerystorage.googleapis.com/write/append_bytes) You can view usage quota and limits metrics for your projects in [Cloud Monitoring](https://docs.cloud.google.com/bigquery/docs/monitoring-dashboard#view_quota_usage_and_limits). Select the throughput limit name based on your region. The options are `AppendBytesThroughputPerProject`, `AppendBytesThroughputPerProjectEU`, and `AppendBytesThroughputPerProjectRegion` for `us`, `eu`, and other regions, respectively. Write throughput quota is metered based on the project where the target dataset resides, not the client project. <br /> It is strongly recommended that you set up [alerts](https://docs.cloud.google.com/monitoring/alerts/using-quota-metrics) to monitor your quota usage and limits. In addition, if your traffic patterns experience spikes and/or regular organic growth, it might be beneficial to consider over-provisioning your quota by 25 - 50% in order to handle unexpected demand. <br /> |
| `CreateWriteStream` requests | 10,000 streams every hour, per project per region | You can call `CreateWriteStream` up to 10,000 times per hour per project per region. Consider using the [default stream](https://docs.cloud.google.com/bigquery/docs/write-api#default_stream) if you don't need exactly-once semantics. This quota is per hour but the metric shown in the Google Cloud console is per minute. |
| Pending stream bytes | 10 TB in multi-regions; 1 TB in regions | For every commit that you trigger, you can commit up to 10 TB in the `us` and `eu` multi-regions, and 1 TB in other regions. There is no quota reporting on this quota. |

The following limits apply to
[Storage Write API](https://docs.cloud.google.com/bigquery/docs/write-api) requests:

| Limit | Default | Notes |
|---|---|---|
| Batch commits | 10,000 streams per table | You can commit up to 10,000 streams in each `BatchCommitWriteStream` call. |
| `AppendRows` request size | 20 MB | The maximum request size is 20 MB. |

## Streaming inserts

The following quotas and limits apply when you stream data into
BigQuery by using the
[legacy streaming API](https://docs.cloud.google.com/bigquery/docs/streaming-data-into-bigquery).
For information about strategies to stay within these limits, see
[Troubleshooting quota errors](https://docs.cloud.google.com/bigquery/docs/troubleshoot-quotas#ts-streaming-insert-quota).
If you exceed these quotas, BigQuery returns a `quotaExceeded` error.
BigQuery might reduce your provisioned quota if your quota is
significantly under-utilized for more than one year.

| Limit | Default | Notes |
|---|---|---|
| Maximum bytes per second per project in the `us` and `eu` multi-regions | 1 GB per second | Your project can stream up to 1 GB per second. This quota is cumulative within a given multi-region. In other words, the sum of bytes per second streamed to all tables for a given project within a multi-region is limited to 1 GB. Exceeding this limit causes `quotaExceeded` errors. If necessary, you can request a quota increase by contacting [Cloud Customer Care](https://cloud.google.com/support-hub). Request any increase as early as possible, at minimum two weeks before you need it. Quota increase takes time to become available, especially in the case of a significant increase. |
| Maximum bytes per second per project in all other locations | 300 MB per second | Your project can stream up to 300 MB per second in all locations except the `us` and `eu` multi-regions. This quota is cumulative within a given multi-region. In other words, the sum of bytes per second streamed to all tables for a given project within a region is limited to 300 MB. Exceeding this limit causes `quotaExceeded` errors. If necessary, you can request a quota increase by contacting [Cloud Customer Care](https://cloud.google.com/support-hub). Request any increase as early as possible, at minimum two weeks before you need it. Quota increase takes time to become available, especially in the case of a significant increase. |
| Maximum row size | 10 MB | Exceeding this value causes `invalid` errors. |
| HTTP request size limit | 10 MB | Exceeding this value causes `invalid` errors. Internally the request is translated from HTTP JSON into an internal data structure. The translated data structure has its own enforced size limit. It's hard to predict the size of the resulting internal data structure, but if you keep your HTTP requests to 10 MB or less, the chance of hitting the internal limit is low. |
| Maximum rows per request | 50,000 rows | A maximum of 500 rows is recommended. Batching can increase performance and throughput to a point, but at the cost of per-request latency. Too few rows per request and the overhead of each request can make ingestion inefficient. Too many rows per request and the throughput can drop. Experiment with representative data (schema and data sizes) to determine the ideal batch size for your data. |
| `insertId` field length | 128 characters | Exceeding this value causes `invalid` errors. |

For additional streaming quota, see
[Request a quota increase](https://docs.cloud.google.com/bigquery/quotas#requesting_a_quota_increase).

## Bandwidth

The following quotas apply to the replication bandwidth:

| Quota | Default | Notes |
|---|---|---|
| Maximum initial backfill replication bandwidth for each [region](https://docs.cloud.google.com/bigquery/docs/locations#regions) that has cross-region data egress from the primary replica to secondary replicas. | 10 physical GiBps per region per organization |   |
| Maximum ongoing replication bandwidth for each [region](https://docs.cloud.google.com/bigquery/docs/locations#regions) that has cross-region data egress from the primary replica to secondary replicas. | 5 physical GiBps per region per organization |   |
| Maximum turbo replication bandwidth for each [region](https://docs.cloud.google.com/bigquery/docs/locations#regions) that has cross-region data egress from the primary replica to secondary replicas. | 5 physical GiBps per region per organization | Turbo replication bandwidth quota doesn't apply to the initial backfill operation. |

<br />

When a project's replication bandwidth exceeds a certain quota, replication from
affected projects might stop with the `rateLimitExceeded` error that
includes details of the exceeded quota.