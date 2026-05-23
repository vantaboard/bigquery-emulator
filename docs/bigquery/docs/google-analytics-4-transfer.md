# Load Google Analytics 4 data into BigQuery

You can load data from Google Analytics 4 to BigQuery using the
[BigQuery Data Transfer Service](https://docs.cloud.google.com/bigquery/docs/dts-introduction) for Google Analytics 4 connector. With the
BigQuery Data Transfer Service, you can schedule recurring transfer jobs that
add your latest data from Google Analytics 4 to
BigQuery.

## Connector overview

The BigQuery Data Transfer Service for the Google Analytics connector supports the following options for your data transfer.

| Data transfer options | Support |
|---|---|
| Supported reports | The Google Analytics connector supports the transfer of reporting data from [Google Analytics Data API v1](https://developers.google.com/analytics/devguides/reporting/data/v1). For information about how Google Analytics reports are transformed into BigQuery tables and views, see [Google Analytics report transformation](https://docs.cloud.google.com/bigquery/docs/google-analytics-4-transformation). |
| Repeat frequency | The Google Analytics connector supports daily data transfers. <br /> By default, data transfers are scheduled at the time when the data transfer is created. You can configure the time of data transfer when you [set up your data transfer](https://docs.cloud.google.com/bigquery/docs/google-analytics-4-transfer#set-up-ga4-transfer). |
| Refresh window | You can schedule your data transfers to retrieve Google Analytics data from up to 30 days at the time the data transfer is run. You can configure the duration of the refresh window when you [set up your data transfer](https://docs.cloud.google.com/bigquery/docs/google-analytics-4-transfer#set-up-ga4-transfer). <br /> By default, the Google Analytics connector has a refresh window of 4 days. <br /> For more information, see [Refresh windows](https://docs.cloud.google.com/bigquery/docs/google-analytics-4-transfer#refresh). |
| Backfill data availability | [Run a data backfill](https://docs.cloud.google.com/bigquery/docs/working-with-transfers#manually_trigger_a_transfer) to retrieve data outside of your scheduled data transfer. You can retrieve data as far back as the data retention policy on your data source allows. <br /> For information about the data retention policy for Google Analytics, see [Google Analytics Data Retention Policy](https://support.google.com/analytics/answer/7667196). |

## Data ingestion from Google Analytics 4 transfers

When you transfer data from Google Analytics 4 into BigQuery, the data is loaded into BigQuery tables that are partitioned by date. The table partition that the data is loaded into corresponds to the date from the data source. If you schedule multiple transfers for the same date, BigQuery Data Transfer Service overwrites the partition for that specific date with the latest data. Multiple transfers in the same day or running backfills don't result in duplicate data, and partitions for other dates are not affected.

<br />

### Refresh windows

A *refresh window* is the number of days that a data transfer retrieves data
when a data transfer occurs. For example, if the refresh window is three days
and a daily transfer occurs, the BigQuery Data Transfer Service retrieves all data from
your source table from the past three days. In this
example, when a daily transfer occurs, the BigQuery Data Transfer Service creates a new
BigQuery destination table partition with a copy of your source table data
from the current day, then automatically triggers backfill runs to update the
BigQuery destination table partitions with your source table data from the
past two days. The automatically triggered backfill runs will either overwrite
or incrementally update your BigQuery destination table,
depending on whether or not incremental updates are supported in the
BigQuery Data Transfer Service connector.

When you run a data transfer for the first time, the data transfer retrieves all
source data available within the refresh window. For example, if the refresh
window is three days and you run the data transfer for the first time, the
BigQuery Data Transfer Service retrieves all source data within three days.

To retrieve data outside the refresh window, such as historical data, or to
recover data from any transfer outages or gaps, you can initiate or schedule a
[backfill run](https://docs.cloud.google.com/bigquery/docs/working-with-transfers#manually_trigger_a_transfer).

## Before you begin

Review the following prerequisites and information before you create a
Google Analytics 4 data transfer.

### Prerequisites

- In Google Analytics 4, the user account or the service account must have viewer access to the [property ID](https://developers.google.com/analytics/devguides/reporting/data/v1/property-id) that is used in the transfer configuration.
- Verify that you have completed all actions required to [enable the BigQuery Data Transfer Service](https://docs.cloud.google.com/bigquery/docs/enable-transfer-service).
- [Create a BigQuery dataset](https://docs.cloud.google.com/bigquery/docs/datasets) to store your Google Analytics 4 data.
- If you intend to set up transfer run notifications for Pub/Sub, ensure that you have the `pubsub.topics.setIamPolicy` Identity and Access Management (IAM) permission. If you only set up email notifications, Pub/Sub permissions aren't required. For more information, see [BigQuery Data Transfer Service run notifications](https://docs.cloud.google.com/bigquery/docs/transfer-run-notifications).

### Required BigQuery roles


To get the permissions that
you need to create a BigQuery Data Transfer Service data transfer,

ask your administrator to grant you the
[BigQuery Admin](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquery#bigquery.admin) (`roles/bigquery.admin`) IAM role on your project.


For more information about granting roles, see [Manage access to projects, folders, and organizations](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).


This predefined role contains

the permissions required to create a BigQuery Data Transfer Service data transfer. To see the exact permissions that are
required, expand the **Required permissions** section:


#### Required permissions

The following permissions are required to create a BigQuery Data Transfer Service data transfer:

- BigQuery Data Transfer Service permissions:
  - `bigquery.transfers.update`
  - `bigquery.transfers.get`
- BigQuery permissions:
  - `bigquery.datasets.get`
  - `bigquery.datasets.getIamPolicy`
  - `bigquery.datasets.update`
  - `bigquery.datasets.setIamPolicy`
  - `bigquery.jobs.create`


You might also be able to get
these permissions
with [custom roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles) or
other [predefined roles](https://docs.cloud.google.com/iam/docs/roles-overview#predefined).

For more information, see [Grant `bigquery.admin` access](https://docs.cloud.google.com/bigquery/docs/enable-transfer-service#grant_bigqueryadmin_access).

## Custom reports

The BigQuery Data Transfer Service for Google Analytics connector
supports the use of custom reports by specifying [dimensions and metrics](https://developers.google.com/analytics/devguides/reporting/data/v1/api-schema)
in the Google Analytics transfer configuration. These custom reports ingest data from the
[Google Analytics Data API version supported by the BigQuery Data Transfer Service](https://docs.cloud.google.com/bigquery/docs/google-analytics-4-transfer#connector_overview).

You can specify a custom report when you [create a Google Analytics transfer](https://docs.cloud.google.com/bigquery/docs/google-analytics-4-transfer#set-up-ga4-transfer).

### Custom reports limitations

- Only one custom report is supported per transfer configuration.
- A maximum of 9 dimensions and 10 metrics are supported per custom report.
- Not all dimensions and metrics are compatible with each other. Use the [GA4 Dimensions \& Metrics Explorer](https://ga-dev-tools.google/ga4/dimensions-metrics-explorer/) tool to validate your custom report dimensions and metrics before creating the transfer.
- [Custom dimensions and metrics](https://support.google.com/analytics/answer/14240153) are not supported.

## Set up a Google Analytics 4 data transfer

Select one of the following options:

### Console

1. Go to the **Data transfers** page in the Google Cloud console.

   [Go to Data transfers](https://console.cloud.google.com/bigquery/transfers)
2. Click **Create transfer**.

3. On the **Create transfer** page, do the following:

   - In the **Source type** section, for **Source** , choose **Google Analytics 4**.
4. In the **Data source details** section:

   - In the **Property ID** field, enter a [property ID](https://developers.google.com/analytics/devguides/reporting/data/v1/property-id).
   - Optional: In the **Table Filter** field, enter a comma-separated list of tables to include, for example, `Audiences, Events`. Prefix this list with the `-` character to exclude certain tables, for example `-Audiences, Events`. All tables are included by default.
   - Optional: To ingest custom reports instead of the standard reports, do the following:
     - In the **Custom Report Table Name** field, enter the output table name for the custom report. For more information about valid table names, see [Table naming](https://docs.cloud.google.com/bigquery/docs/tables#table_naming).
     - In the **Custom Report Dimensions** field, enter the dimensions for the custom report. For more information, see [Custom reports](https://docs.cloud.google.com/bigquery/docs/google-analytics-4-transfer#custom_reports).
     - In the **Custom Report Metrics** field, enter the metrics for the custom report. For more information, see [Custom reports](https://docs.cloud.google.com/bigquery/docs/google-analytics-4-transfer#custom_reports).
   - Optional: In the **Refresh window** field, enter a duration for your [refresh window](https://docs.cloud.google.com/bigquery/docs/google-analytics-4-transfer#refresh) in days. The refresh window has a default value of four days, and can be a value up to 30 days.
5. In the **Destination settings** section, in the **Destination dataset**
   menu, select the dataset that you created to store your data.

6. In the **Transfer config name** section, for **Display name**, enter a
   name for the data transfer. The transfer name can be any value that lets
   you identify the transfer if you need to modify it later.

7. In the **Schedule options** section:

   - Select either **Start now** or **Start at set time**, then provide a start date and run time.
   - For **Repeats** , choose an option for how often to run the data transfer. If you select **Days**, provide a valid time in UTC.
8. Optional: In the **Service Account** menu, select a
   [service account](https://docs.cloud.google.com/iam/docs/service-account-overview) from the service
   accounts that are associated with your Google Cloud project. The
   selected service account must have the
   [required roles](https://docs.cloud.google.com/bigquery/docs/google-analytics-4-transfer#bq-roles) to run
   this data transfer.

   If you signed in with a
   [federated identity](https://docs.cloud.google.com/iam/docs/workforce-identity-federation), then a
   service account is required to create a data transfer. If you signed in
   with a [Google Account](https://docs.cloud.google.com/iam/docs/principals-overview#google-account), then a service
   account for the data transfer is optional. For more information about
   using service accounts with data transfers, see
   [Use service accounts](https://docs.cloud.google.com/bigquery/docs/use-service-accounts).
9. Optional: In the **Notification options** section:

   - Click the toggle to enable email notifications. When you enable this option, the transfer administrator receives an email notification when a transfer run fails.
   - Click the toggle to enable Pub/Sub notifications. For **Select a Cloud Pub/Sub topic** , choose your [topic](https://docs.cloud.google.com/pubsub/docs/overview#types) name or click **Create a topic** . This option configures Pub/Sub [run notifications](https://docs.cloud.google.com/bigquery/docs/transfer-run-notifications) for your transfer.
10. Optional: If you use [CMEKs](https://docs.cloud.google.com/bigquery/docs/customer-managed-encryption), in the
    **Advanced options** section, select **Customer-managed key** . A list of
    your available CMEKs appears for you to choose from. For information
    about how CMEKs work with the BigQuery Data Transfer Service, see
    [Specify encryption key with transfers](https://docs.cloud.google.com/bigquery/docs/google-analytics-4-transfer#CMEK).

11. Click **Save**.

### bq

Enter the `bq mk` command and supply the transfer creation flag ---
`--transfer_config`. The following flags are required:

- `--data_source`
- `--target_dataset`
- `--display_name`
- `--params`

```bash
  bq mk --transfer_config \
  --project_id=PROJECT_ID \
  --target_dataset=DATASET \
  --display_name=NAME \
  --params='PARAMETERS' \
  --data_source=DATA_SOURCE
```

Where:

- <var translate="no">PROJECT_ID</var>: your project ID. If `--project_id` isn't specified, the default project is used.
- <var translate="no">DATASET</var>: the target dataset for the data transfer configuration.
- <var translate="no">NAME</var>: the display name for the data transfer configuration. The transfer name can be any value that lets you identify the transfer if you need to modify it later.
- <var translate="no">PARAMETERS</var>: the parameters for the created data transfer configuration in JSON format, for example, `--params='{"param":"param_value"}'`. For Google Analytics 4 transfers, the `property_id` parameter is required.
- <var translate="no">DATA_SOURCE</var>: the data source --- `ga4`.

For example, the following command creates a Google Analytics 4
data transfer named `My Transfer` using property ID `468039345`, with the
target dataset `mydataset`.

The data transfer is created in the default project:

```bash
  bq mk --transfer_config
  --project_id=your_project
  --target_dataset=mydataset
  --display_name=My Transfer
  --params='{"property_id":"468039345"}'
  --data_source=ga4
```

### API

Use the
[`projects.locations.transferConfigs.create`](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs/create)
method and supply an instance of the
[`TransferConfig`](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs#TransferConfig)
resource.

## Limitations

- Aggregated totals for distinct users and session metrics might not be accurate and might not match the values in Google Analytics.

## Specify encryption key with transfers

You can specify [customer-managed encryption keys (CMEKs)](https://docs.cloud.google.com/kms/docs/cmek) to encrypt data for a transfer run. You can use a CMEK to support transfers from [Google Analytics 4](https://docs.cloud.google.com/bigquery/docs/google-analytics-4-transfer).

When you specify a CMEK with a transfer, the BigQuery Data Transfer Service applies the
CMEK to any intermediate on-disk cache of ingested data so that the entire
data transfer workflow is CMEK compliant.

You cannot update an existing transfer to add a CMEK if the transfer was not
originally created with a CMEK. For example, you cannot change a destination
table that was originally default encrypted to now be encrypted with CMEK.
Conversely, you also cannot change a CMEK-encrypted destination table
to have a different type of encryption.

You can update a CMEK for a transfer if the transfer configuration was
originally created with a CMEK encryption. When you update a CMEK for a transfer
configuration, the BigQuery Data Transfer Service propagates the CMEK to the destination
tables at the next run of the transfer, where the BigQuery Data Transfer Service
replaces any outdated CMEKs with the new CMEK during the transfer run.
For more information, see [Update a transfer](https://docs.cloud.google.com/bigquery/docs/working-with-transfers#update_a_transfer).

You can also use [project default keys](https://docs.cloud.google.com/bigquery/docs/customer-managed-encryption#project_default_key).
When you specify a project default key with a transfer, the BigQuery Data Transfer Service
uses the project default key as the default key for any new transfer
configurations.

## Pricing

There is no cost to run a Google Analytics 4 transfer.

Once data is transferred to BigQuery, standard
BigQuery [storage](https://cloud.google.com/bigquery/pricing#storage) and
[query](https://cloud.google.com/bigquery/pricing#queries) pricing applies.

## Quota

Google Analytics 4 transfers are subject to the [analytics property quotas](https://developers.google.com/analytics/devguides/reporting/data/v1/quotas#analytics_property_quotas)
as enforced by Google Analytics 4. To allow more quota per property,
you can upgrade to [Google Analytics 360](https://marketingplatform.google.com/about/analytics-360/features/).