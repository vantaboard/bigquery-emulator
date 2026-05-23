# Export data to Pub/Sub (reverse ETL)

Exporting data to Pub/Sub requires using BigQuery
[continuous queries](https://docs.cloud.google.com/bigquery/docs/continuous-queries-introduction).

This document describes how you can set up reverse extract-transform-load
(RETL) from
BigQuery to [Pub/Sub](https://docs.cloud.google.com/pubsub/docs/overview).
You can do this by using the
[`EXPORT DATA` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/export-statements)
in a [continuous query](https://docs.cloud.google.com/bigquery/docs/continuous-queries-introduction) to
export data from BigQuery to a
[Pub/Sub topic](https://docs.cloud.google.com/pubsub/docs/create-topic).

You can use a RETL workflow to Pub/Sub to combine
BigQuery's analytics capabilities with Pub/Sub's
asynchronous and scalable global messaging service. This workflow lets you
serve data to downstream applications and services in an event-driven manner.

## Prerequisites

You must [create a service account](https://docs.cloud.google.com/iam/docs/service-accounts-create). A
[service account](https://docs.cloud.google.com/iam/docs/service-account-overview)
is required to run a continuous query that exports results to a
Pub/Sub topic.

You must create a
[Pub/Sub topic](https://docs.cloud.google.com/pubsub/docs/publish-message-overview#about_topics)
to receive the continuous query results as messages, and a
[Pub/Sub subscription](https://docs.cloud.google.com/pubsub/docs/subscription-overview)
that the target application can use to receive those messages.

## Required roles

This section provides information about the roles and permissions required by
the user account that creates the continuous query, and the service account that
runs the continuous query.

### User account permissions

To create a job in BigQuery, the user account must have the
`bigquery.jobs.create` IAM permission. Each of the following
IAM roles grants the `bigquery.jobs.create` permission:

- [BigQuery User (`roles/bigquery.user`)](https://docs.cloud.google.com/bigquery/docs/access-control#bigquery.user)
- [BigQuery Job User (`roles/bigquery.jobUser`)](https://docs.cloud.google.com/bigquery/docs/access-control#bigquery.jobUser)
- [BigQuery Admin (`roles/bigquery.admin`)](https://docs.cloud.google.com/bigquery/docs/access-control#bigquery.admin)

To submit a job that runs using a service account, the user account must have the
[Service Account User (`roles/iam.serviceAccountUser`)](https://docs.cloud.google.com/iam/docs/service-account-permissions#user-role)
role. If you are using the same user account to create the service account,
then the user account must have the
[Service Account Admin (`roles/iam.serviceAccountAdmin`)](https://docs.cloud.google.com/iam/docs/roles-permissions/iam#iam.serviceAccountUser)
role. For information on how to limit a user's access to single service account,
rather than to all service accounts within a project, see
[Grant a single role](https://docs.cloud.google.com/iam/docs/manage-access-service-accounts#grant-single-role).

If the user account must enable the APIs required for your
continuous query use case, the user account must have the
[Service Usage Admin (`roles/serviceusage.serviceUsageAdmin`)](https://docs.cloud.google.com/iam/docs/roles-permissions/serviceusage#serviceusage.serviceUsageAdmin)
role.

### Service account permissions

To export data from a BigQuery table, the service account must
have the `bigquery.tables.export` IAM permission. Each of the
following IAM roles grants the `bigquery.tables.export`
permission:

- [BigQuery Data Viewer (`roles/bigquery.dataViewer`)](https://docs.cloud.google.com/bigquery/docs/access-control#bigquery.dataViewer)
- [BigQuery Data Editor (`roles/bigquery.dataEditor`)](https://docs.cloud.google.com/bigquery/docs/access-control#bigquery.dataEditor)
- [BigQuery Data Owner (`roles/bigquery.dataOwner`)](https://docs.cloud.google.com/bigquery/docs/access-control#bigquery.dataOwner)
- [BigQuery Admin (`roles/bigquery.admin`)](https://docs.cloud.google.com/bigquery/docs/access-control#bigquery.admin)

For the service account to access Pub/Sub, you must grant the
service account both of the following IAM roles:

- [Pub/Sub Viewer (`roles/pubsub.viewer`)](https://docs.cloud.google.com/iam/docs/roles-permissions/pubsub#pubsub.viewer)
- [Pub/Sub Publisher (`roles/pubsub.publisher`)](https://docs.cloud.google.com/iam/docs/roles-permissions/pubsub#pubsub.publisher)

You might also be able to get the required permissions through
[custom roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles).

## Before you begin


Enable the BigQuery and Pub/Sub APIs.


**Roles required to enable APIs**


To enable APIs, you need the Service Usage Admin IAM
role (`roles/serviceusage.serviceUsageAdmin`), which
contains the `serviceusage.services.enable` permission. [Learn how to grant
roles](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).

[Enable the APIs](https://console.cloud.google.com/flows/enableapi?apiid=bigquery.googleapis.com,pubsub.googleapis.com)

## Export to Pub/Sub

Use the
[`EXPORT DATA` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/export-statements)
to export data to a Pub/Sub topic:

### Console

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, click **More** \> **Query settings**.

3. In the **Continuous Query** section, select the
   **Use continuous query mode** checkbox.

4. In the **Service account** box, select the service account that you
   created.

5. Click **Save**.

6. In the query editor, enter the following statement:

   ```googlesql
   EXPORT DATA
   OPTIONS (
   format = 'CLOUD_PUBSUB',
   uri = 'https://pubsub.googleapis.com/projects/PROJECT_ID/topics/TOPIC_ID'
   ) AS
   (
   QUERY
   );
   ```

   Replace the following:
   - `PROJECT_ID`: your project ID.
   - `TOPIC_ID`: the Pub/Sub topic ID. You can get the topic ID from the [**Topics** page](https://console.cloud.google.com/cloudpubsub/topic/list) of the Google Cloud console.
   - `QUERY`: the SQL statement to select the data to export. The SQL statement must only contain [supported operations](https://docs.cloud.google.com/bigquery/docs/continuous-queries-introduction#supported_functionality). You must use the [`APPENDS` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/time-series-functions#appends) in the `FROM` clause of a continuous query to specify the point in time at which to start processing data.
7. Click **Run**.

### bq


1. In the Google Cloud console, activate Cloud Shell.

   [Activate Cloud Shell](https://console.cloud.google.com/?cloudshell=true)


   At the bottom of the Google Cloud console, a
   [Cloud Shell](https://docs.cloud.google.com/shell/docs/how-cloud-shell-works)
   session starts and displays a command-line prompt. Cloud Shell is a shell environment
   with the Google Cloud CLI
   already installed and with values already set for
   your current project. It can take a few seconds for the session to initialize.
2. On the command line, run the continuous query by using the
   [`bq query` command](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#bq_query)
   with the following flags:

   - Set the `--continuous` flag to `true` to make the query continuous.
   - Use the `--connection_property` flag to specify a service account to use.

   ```bash
   bq query --project_id=PROJECT_ID --use_legacy_sql=false \
   --continuous=true --connection_property=service_account=SERVICE_ACCOUNT_EMAIL \
   'EXPORT DATA OPTIONS (format = "CLOUD_PUBSUB", uri = "https://pubsub.googleapis.com/projects/PROJECT_ID/topics/TOPIC_ID") AS (QUERY);'
   ```

   Replace the following:
   - `PROJECT_ID`: your project ID.
   - `SERVICE_ACCOUNT_EMAIL`: the service account email. You can get the service account email on the [**Service accounts** page](https://console.cloud.google.com/iam-admin/serviceaccounts) of the Google Cloud console.
   - `QUERY`: the SQL statement to select the data to export. The SQL statement must only contain [supported operations](https://docs.cloud.google.com/bigquery/docs/continuous-queries-introduction#supported_functionality). You must use the [`APPENDS` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/time-series-functions#appends) in the `FROM` clause of a continuous query to specify the point in time at which to start processing data.

### API

1. Run the continuous query by calling the
   [`jobs.insert` method](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs/insert).
   Set the following fields in the
   [`JobConfigurationQuery` resource](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/Job#JobConfigurationQuery)
   of the [`Job` resource](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/Job) that you pass in:

   - Set the `continuous` field to `true` to make the query continuous.
   - Use the `connection_property` field to specify a service account to use.

   ```bash
   curl --request POST \
     'https://bigquery.googleapis.com/bigquery/v2/projects/PROJECT_ID/jobs'
     --header 'Authorization: Bearer $(gcloud auth print-access-token) \
     --header 'Accept: application/json' \
     --header 'Content-Type: application/json' \
     --data '("configuration":("query":"EXPORT DATA OPTIONS (format = 'CLOUD_PUBSUB', uri = 'https://pubsub.googleapis.com/projects/PROJECT_ID/topics/TOPIC_ID') AS (QUERY);","useLegacySql":false,"continuous":true,"connectionProperties":["key": "service_account","value":"SERVICE_ACCOUNT_EMAIL"]))' \
     --compressed
   ```

   Replace the following:
   - `PROJECT_ID`: your project ID.
   - `QUERY`: the SQL statement to select the data to export. The SQL statement must only contain [supported operations](https://docs.cloud.google.com/bigquery/docs/continuous-queries-introduction#supported_functionality). You must use the [`APPENDS` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/time-series-functions#appends) in the `FROM` clause of a continuous query to specify the point in time at which to start processing data.
   - `SERVICE_ACCOUNT_EMAIL`: the service account email. You can get the service account email on the [**Service accounts** page](https://console.cloud.google.com/iam-admin/serviceaccounts) of the Google Cloud console.

## Export multiple columns to Pub/Sub

If you want to include multiple columns in your output, you can create a struct
column to contain the column values, and then convert the struct value
to a JSON string by using the
[`TO_JSON_STRING` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/json_functions#to_json_string).
The following example exports data from four columns, formatted as a JSON
string:

```googlesql
EXPORT DATA
  OPTIONS (
    format = 'CLOUD_PUBSUB',
    uri = 'https://pubsub.googleapis.com/projects/myproject/topics/taxi-real-time-rides')
AS (
  SELECT
    TO_JSON_STRING(
      STRUCT(
        ride_id,
        timestamp,
        latitude,
        longitude)) AS message
  FROM
    APPENDS(TABLE `myproject.real_time_taxi_streaming.taxi_rides`,
      -- Configure the APPENDS TVF start_timestamp to specify when you want to
      -- start processing data using your continuous query.
      -- This example starts processing at 10 minutes before the current time.
      CURRENT_TIMESTAMP() - INTERVAL 10 MINUTE)
  WHERE ride_status = 'enroute'
);
```

## Export optimization

If your continuous query job performance appears to be limited by
[available compute resources](https://docs.cloud.google.com/bigquery/docs/continuous-queries-monitor#view_slot_consumption_information),
try increasing the size of your BigQuery
[`CONTINUOUS` slot reservation assignment](https://docs.cloud.google.com/bigquery/docs/reservations-assignments).

## Limitations

- The exported data must consist of a single [`STRING`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#string_type) or [`BYTES`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#bytes_type) column. The column name can be whatever you choose.
- You must use a [continuous query](https://docs.cloud.google.com/bigquery/docs/continuous-queries-introduction) to export to Pub/Sub.
- You can't pass a schema to a Pub/Sub topic in the continuous query.
- You can't export data to a Pub/Sub topic that uses a schema.
- When exporting to Pub/Sub, you can export JSON formatted records where some values are `NULL`, but you can't export records that consist of only `NULL` values. You can exclude `NULL` records from the query results by including a `WHERE message IS NOT NULL` filter in the continuous query.
- When exporting data to a Pub/Sub topic configured with a [locational endpoint](https://docs.cloud.google.com/pubsub/docs/reference/service_apis_overview#pubsub_endpoints), the endpoint must be configured within the same Google Cloud regional boundary as the BigQuery dataset that contains the table you are querying.
- Exported data must not exceed [Pub/Sub quotas](https://docs.cloud.google.com/pubsub/quotas).

## Pricing

When you export data in a continuous query, you are billed using
[BigQuery capacity compute pricing](https://cloud.google.com/bigquery/pricing#capacity_compute_analysis_pricing).
To run continuous queries, you must have a
[reservation](https://docs.cloud.google.com/bigquery/docs/reservations-workload-management) that uses the
[Enterprise or Enterprise Plus edition](https://docs.cloud.google.com/bigquery/docs/editions-intro),
and a [reservation assignment](https://docs.cloud.google.com/bigquery/docs/reservations-workload-management#assignments)
that uses the `CONTINUOUS` job type.

After the data is exported, you're charged for using Pub/Sub.
For more information, see [Pub/Sub pricing](https://cloud.google.com/pubsub/pricing).