# Load Search Ads 360 data into BigQuery

You can load data from Search Ads 360 to BigQuery using the
[BigQuery Data Transfer Service](https://docs.cloud.google.com/bigquery/docs/dts-introduction) for Search Ads 360 connector. With the
BigQuery Data Transfer Service, you can schedule recurring transfer jobs that
add your latest data from Search Ads 360 to
BigQuery.

## Connector overview

The BigQuery Data Transfer Service for the Search Ads 360 connector supports the following options for your data transfer.

| Data transfer options | Support |
|---|---|
| Supported reports | The Search Ads 360 connector supports the transfer of data from the reports in [Search Ads 360 v0 reports](https://developers.google.com/search-ads/reporting/api/reference/fields/v0/overview). For information about how Search Ads 360 reports are transformed into BigQuery tables and views, see [Search Ads 360 report transformation](https://docs.cloud.google.com/bigquery/docs/search-ads-transformation). |
| Repeat frequency | The Search Ads 360 connector supports daily data transfers. <br /> By default, data transfers are scheduled at the time when the data transfer is created. You can configure the time of data transfer when you [set up your data transfer](https://docs.cloud.google.com/bigquery/docs/search-ads-transfer#setup-data-transfer). |
| Refresh window | You can schedule your data transfers to retrieve Search Ads 360 data from up to 30 days at the time the data transfer is run. You can configure the duration of the refresh window when you [set up your data transfer](https://docs.cloud.google.com/bigquery/docs/google-ads-transfer#setup-data-transfer). <br /> By default, the Search Ads 360 connector has a refresh window of 7 days. <br /> For more information, see [Refresh windows](https://docs.cloud.google.com/bigquery/docs/search-ads-transfer#refresh). Snapshots of [Match Tables](https://docs.cloud.google.com/bigquery/docs/search-ads-transformation#search_ads_match_tables) are taken once a day and stored in the partition for the last run date. Match Table snapshots are not updated for backfills or for days loaded using the refresh window. |
| Backfill data availability | [Run a data backfill](https://docs.cloud.google.com/bigquery/docs/working-with-transfers#manually_trigger_a_transfer) to retrieve data outside of your scheduled data transfer. You can retrieve data as far back as the data retention policy on your data source allows. <br /> For information about the data retention policy for Search Ads 360, see [Reporting data retention policy](https://support.google.com/sa360/answer/13292701). |
| Number of Customer IDs per manager account | The BigQuery Data Transfer Service supports a maximum of **8000 Customer IDs** for each Search Ads 360 [manager account](https://support.google.com/sa360/answer/9158072). |

To see the Search Ads 360 transfer guide that uses the old
Search Ads 360 reporting API, see [Search Ads 360 transfers (Deprecated)](https://docs.cloud.google.com/bigquery/docs/sa360-transfer).

## Data ingestion from Search Ads 360 transfers

When you transfer data from Search Ads 360 into BigQuery, the data is loaded into BigQuery tables that are partitioned by date. The table partition that the data is loaded into corresponds to the date from the data source. If you schedule multiple transfers for the same date, BigQuery Data Transfer Service overwrites the partition for that specific date with the latest data. Multiple transfers in the same day or running backfills don't result in duplicate data, and partitions for other dates are not affected.

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

## Limitations

- The maximum frequency that you can configure a Search Ads 360 data transfer for is once every 24 hours. By default, a transfer starts at the time that you create the transfer. However, you can configure the data transfer start time when you [create your transfer](https://docs.cloud.google.com/bigquery/docs/search-ads-transfer#setup-data-transfer).
- The BigQuery Data Transfer Service does not support incremental data transfers during a Search Ads 360 transfer. When you specify a date for a data transfer, all of the data that is available for that date is transferred.

## Before you begin

Before you create a Search Ads 360 data transfer:

- Verify that you have completed all actions required to [enable the
  BigQuery Data Transfer Service](https://docs.cloud.google.com/bigquery/docs/enable-transfer-service).
- [Create a BigQuery Data Transfer Service dataset](https://docs.cloud.google.com/bigquery/docs/datasets) to store the Search Ads 360 reporting data.
- If you intend to setup transfer run notifications for Pub/Sub, you must have `pubsub.topics.setIamPolicy` permissions. Pub/Sub permissions are not required if you just set up email notifications. For more information, see [BigQuery Data Transfer Service run
  notifications](https://docs.cloud.google.com/bigquery/docs/transfer-run-notifications).
- [Enable access](https://console.developers.google.com/apis/api/searchads360.googleapis.com/) to the Search Ads 360 reporting API in your project.

## Required permissions

Ensure that you have granted the following permissions.

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

### Required Google Cloud roles

To download data from Search Ads 360, you must have the
`serviceusage.services.use` permission. The
[Service Usage Consumer](https://docs.cloud.google.com/iam/docs/roles-permissions/serviceusage#serviceusage.serviceUsageConsumer) (`roles/serviceusage.serviceUsageConsumer`)
predefined IAM role includes this permission.

### Required Search Ads 360 roles

Grant read access to the Search Ads 360 Customer ID or [manager
account](https://support.google.com/sa360/answer/9158072) that is used
in the transfer configuration. To configure read access for service
accounts, you can [contact Search Ads 360
support](https://support.google.com/sa360/gethelp) for assistance.

## Create a Search Ads 360 data transfer

To create a data transfer for Search Ads 360 reporting, you need either your
Search Ads 360 Customer ID or [manager account](https://support.google.com/sa360/answer/9158072).
Select one of the following options:

### Console

1. Go to the Data transfers page in the Google Cloud console.

   [Go to Data transfers](https://console.cloud.google.com/bigquery/transfers)
2. Click **Create transfer**.

3. In the **Source type** section, for **Source** , choose
   **Search Ads 360**.

4. In the **Transfer config name** section, for **Display name** , enter a
   name for the data transfer such as `My Transfer`. The transfer name can be
   any value that lets you identify the transfer if you need to modify it
   later.

5. In the **Schedule options** section:

   - For **Repeat frequency** , choose an option for how often to run the data transfer. If you select **Days**, provide a valid time in UTC.
   - If applicable, select either **Start now** or **Start at set time**, and provide a start date and run time.
6. In the **Destination settings** section, for **Dataset**, select the
   dataset that you created to store your data.

7. In the **Data source details** section:

   1. For **Customer ID**, enter your Search Ads 360 customer ID.
   2. Optional: Enter both an **Agency ID** and **Advertiser ID** to retrieve [ID mapping tables](https://docs.cloud.google.com/bigquery/docs/search-ads-transfer#id-mapping).
   3. Optional: For **Custom Floodlight Variables** , enter any
      [custom Floodlight variables](https://support.google.com/sa360/answer/13567857)
      to include in the data transfer. The custom Floodlight variables must
      be owned by the Search Ads 360 account that is specified by
      the Customer ID in the transfer config. This parameter takes string
      inputs in JSON array format and can support multiple custom Floodlight
      variables. In each item of the JSON array, the following parameters
      are required:

      - `id`: the numeric ID of the custom Floodlight variable. This ID is assigned when [a custom Floodlight variable is created in Search Ads 360](https://support.google.com/sa360/answer/14316155). If you have specified an `id`, then a `name` isn't required.
      - `name`: the user-defined name of the custom Floodlight variables in Search Ads 360. If you have specified a `name`, then an `id` isn't required.
      - `cfv_field_name`: the exact custom Floodlight variable field name based on your use case. The supported values are `conversion_custom_metrics`, `conversion_custom_dimensions`, `raw_event_conversion_metrics`, and `raw_event_conversion_dimensions`.
      - `destination_table_name`: a list of BigQuery tables to include the custom Floodlight variables in. When the BigQuery Data Transfer Service retrieves data for these tables, the transfer includes the custom Floodlight variables in the query.
      - `bigquery_column_name_suffix`: the user-defined friendly column name. The BigQuery Data Transfer Service appends the suffix after the standard field name to differentiate different custom Floodlight variables. Depending on the use case, the BigQuery Data Transfer Service generates a BigQuery column name as follows:

      |   | Custom Floodlight variables as metrics and segments | Custom Floodlight variables as Raw Event Attributes in the Conversion Resource |
      |---|---|---|
      | `metrics` | `metrics_conversion_custom_metrics_bigquery_column_name_suffix` | `metrics_raw_event_conversion_metrics_bigquery_column_name_suffix` |
      | `dimension` | `segments_conversion_custom_dimensions_bigquery_column_name_suffix` | `segments_raw_event_conversion_dimensions_bigquery_column_name_suffix` |

      The following is an example **Custom Floodlight Variable** entry
      that specifies two custom Floodlight variables:

      ```bash
      [{
      "id": "1234",
      "cfv_field_name": "raw_event_conversion_metrics",
      "destination_table_name": ["Conversion"],
      "bigquery_column_name_suffix": "suffix1"
      },{
      "name": "example name",
      "cfv_field_name": "conversion_custom_metrics",
      "destination_table_name": ["AdGroupConversionActionAndDeviceStats","CampaignConversionActionAndDeviceStats"],
      "bigquery_column_name_suffix": "suffix2"
      }]
      ```
   4. Optional: In the **Custom Columns** field, enter any
      [custom columns](https://developers.google.com/search-ads/reporting/concepts/custom-columns)
      to include in the data transfer. The custom columns must be owned by
      the Search Ads 360 account that is specified by the
      Customer ID in the transfer config. This field takes string inputs in
      JSON array format and can support multiple columns. In each item of
      the JSON array, the following parameters are required:

      - `id`: the numeric ID of the custom column. This ID is assigned when a [custom column is created](https://support.google.com/sa360/answer/9633916?&ref_topic=14138984&sjid=5858325799664893372-NC). If you have specified an `id`, then a `name` isn't required.
      - `name`: the user-defined name of the custom column in Search Ads 360. If you have specified a `name`, then an `id` isn't required.
      - `destination_table_name`: a list of BigQuery tables to include the custom column in. When the BigQuery Data Transfer Service retrieves data for these tables, the transfer includes the custom column field in the query.
      - `bigquery_column_name`: the user-defined friendly column name. This is the field name of the custom column in the destination tables specified in `destination_table_name`. The column name must [follow the format requirements for BigQuery column names](https://docs.cloud.google.com/bigquery/docs/schemas#column_names) and must be unique to other fields in the [table's standard schema](https://docs.cloud.google.com/bigquery/docs/search-ads-transformation) or other custom columns.

      The following is an example **Custom Columns** entry that specifies
      two custom columns:

      ```bash
      [{
        "id": "1234",
        "destination_table_name": ["Conversion"],
        "bigquery_column_name": "column1"
      },{
        "name": "example name",
        "destination_table_name": ["AdGroupStats","CampaignStats"],
        "bigquery_column_name": "column2"
      }]
      ```
   5. Optional: In the **Table Filter** field, enter a comma-separated list
      of tables to include, for example `Campaign, AdGroup`. Prefix this
      list with the `-` character to exclude certain tables, for example
      `-Campaign, AdGroup`. All tables are included by default.

   6. Optional: Select **Include PMax Campaign Data** to include PMax
      campaign data and excludes `ad_group` fields from certain tables. For
      more information, see [Performance Max (PMax)
      campaigns](https://docs.cloud.google.com/bigquery/docs/search-ads-transfer#pmax-support)

   7. Optional: Select **Use Client Account Currency** to use the currency of
      the client's account to load cost data, instead of the currency of the
      account used in this data transfer.

   8. Optional: For **Refresh window** , enter a value between 1 and 30. If
      not set, the refresh window defaults to 7 days. For more information,
      see [Refresh windows](https://docs.cloud.google.com/bigquery/docs/search-ads-transfer#refresh)

8. In the **Service Account** menu, select a
   [service account](https://docs.cloud.google.com/iam/docs/service-account-overview) from the service
   accounts that are associated with your Google Cloud project. You can
   associate a service account with your transfer instead of using your user
   credentials. For more information about using service accounts with data
   transfers, see
   [Use service accounts](https://docs.cloud.google.com/bigquery/docs/use-service-accounts).

   If you signed in with a
   [federated identity](https://docs.cloud.google.com/iam/docs/workforce-identity-federation), then a
   service account is required to create a transfer. If you signed in with a
   [Google Account](https://docs.cloud.google.com/iam/docs/principals-overview#google-account), then a service
   account for the transfer is optional. The service account must have the
   [required permissions](https://docs.cloud.google.com/bigquery/docs/search-ads-transfer#required_permissions).
9. Optional: In the **Notification options** section:

   - Click the toggle to enable email notifications. When you enable this option, the transfer administrator receives an email notification when a transfer run fails.
   - Click the toggle to enable Pub/Sub notifications. For **Select a Cloud Pub/Sub topic** , choose your [topic](https://docs.cloud.google.com/pubsub/docs/overview#types) name or click **Create a topic** . This option configures Pub/Sub run [notifications](https://docs.cloud.google.com/bigquery/docs/transfer-run-notifications) for your transfer.
10. Click **Save**.

### bq

Enter the `bq mk` command and supply the transfer creation flag ---
`--transfer_config`. The following flags are also required:

- `--data_source`
- `--target_dataset`
- `--display_name`
- `--params`

The following flags are optional:

- `--project_id`: Specifies which project to use. If the flag is not specified, the default project is used.
- `--service_account_name`: Specifies a service account to use for Search Ads 360 transfer authentication instead of your user account.

```bash
bq mk \
--transfer_config \
--project_id=PROJECT_ID \
--target_dataset=DATASET \
--display_name=NAME \
--data_source=DATA_SOURCE \
--service_account_name=SERVICE_ACCOUNT_NAME \
--params='{PARAMETERS,"custom_columns":"[{\"id\": \"CC_ID\",\"destination_table_name\": [\"CC_DESTINATION_TABLE\"],\"bigquery_column_name\": \"CC_COLUMN\"}]","custom_floodlight_variables":"[{\"id\": \"CFV_ID\",\"cfv_field_name\": [\"CFV_FIELD_NAME\"],\"destination_table_name\": [\"CFV_DESTINATION_TABLE\"],\"bigquery_column_name_suffix\": \"CFV_COLUMN_SUFFIX\"}]"}'
```

Where:

- <var translate="no">PROJECT_ID</var> (Optional): specifies which project to use. If the flag is not specified, the default project is used.
- <var translate="no">DATASET</var>: the target dataset for the transfer configuration.
- <var translate="no">NAME</var>: the display name for the transfer configuration. The
  data transfer name can be any value that lets you identify the transfer if you
  need to modify it later.

- <var translate="no">DATA_SOURCE</var>: the data source --- `search_ads`.

- <var translate="no">SERVICE_ACCOUNT_NAME</var> (Optional): the service account name used to
  authenticate your data transfer. The service
  account should be owned by the same `project_id` used to create the
  transfer and it should have all of the [required permissions](https://docs.cloud.google.com/bigquery/docs/search-ads-transfer#required_permissions).

- <var translate="no">PARAMETERS</var>: the parameters for the created transfer
  configuration in JSON format. For example:
  `--params='{"param":"param_value"}'`. You
  must supply the `customer_id` parameter.

  - `table_filter`: Specifies which tables to include in the data transfer. If the flag is not specified, all tables are included. To include only specific tables, use a comma-separated list of values (for example, `Ad, Campaign, AdGroup`). To exclude specific tables, prefix the excluded values with a hyphen (`-`) (for example, using `-Ad, Campaign, AdGroup` excludes all three values.)
  - `custom_columns`: specifies custom columns to your reports. This parameter takes string inputs in JSON array format and can support multiple columns. In each item of the JSON array, the following parameters are required:
    - <var translate="no">CC_ID</var>: the numeric ID of the custom column. This ID is assigned when a [custom column is created](https://support.google.com/sa360/answer/9633916?&ref_topic=14138984&sjid=5858325799664893372-NC).
    - <var translate="no">CC_DESTINATION_TABLE</var>: a list of BigQuery tables to include the custom column in. When the BigQuery Data Transfer Service retrieves data for these tables, the data transfer includes the custom column field in the query.
    - <var translate="no">CC_COLUMN</var>: the user-defined friendly column name. This is the field name of the custom column in the destination tables specified in `destination_table_name`. The column name has to [follow the format requirements for BigQuery column names](https://docs.cloud.google.com/bigquery/docs/schemas#column_names) and must be unique to other fields in the [table's standard schema](https://docs.cloud.google.com/bigquery/docs/search-ads-transformation) or other custom columns.
  - `custom_floodlight_variables`: specifies [custom Floodlight variables](https://support.google.com/campaignmanager/answer/2823222?sjid=11547437748727448706-NA) in your transfer. This parameter takes string inputs in JSON array format and can support multiple custom Floodlight variables. In each item of the JSON array, the following parameters are required:
    - <var translate="no">CFV_ID</var>: the numeric ID of the custom Floodlight variable. This ID is assigned when [a custom Floodlight variable is created in Search Ads 360](https://support.google.com/searchads/answer/6024747#set-up).
    - <var translate="no">CFV_FIELD_NAME</var>: the exact custom Floodlight variable field name based on your use case. The supported values are `conversion_custom_metrics`, `conversion_custom_dimensions`, `raw_event_conversion_metrics` and `raw_event_conversion_dimensions`. For more information, see [Custom Floodlight metrics](https://developers.google.com/search-ads/reporting/concepts/custom-floodlight-variables).
    - <var translate="no">CFV_DESTINATION_TABLE</var>: a list of BigQuery tables to include the custom floodlight variables in. When the BigQuery Data Transfer Service retrieves data for these tables, the data transfer includes the custom Floodlight variables in the query.
    - <var translate="no">CFV_COLUMN_SUFFIX</var>: the user-defined friendly column name. The BigQuery Data Transfer Service appends the suffix after the standard field name to differentiate different custom Floodlight variables. Depending on the use case, the BigQuery Data Transfer Service generates a BigQuery column name as follows:
  - `use_client_account_currency`: specify `TRUE` to use the currency of the client's account to load cost data, instead of the currency of the account used in this data transfer.

  |   | Custom Floodlight variables as metrics and segments | Custom Floodlight variables as Raw Event Attributes in the Conversion Resource |
  |---|---|---|
  | `metrics` | `metrics_conversion_custom_metrics_bigquery_column_name_suffix` | `metrics_raw_event_conversion_metrics_bigquery_column_name_suffix` |
  | `dimension` | `segments_conversion_custom_dimensions_bigquery_column_name_suffix` | `segments_raw_event_conversion_dimensions_bigquery_column_name_suffix` |

> [!CAUTION]
> **Caution:** You cannot configure notifications using the command-line tool.

For example, the following command creates a Search Ads 360
data transfer named `My Transfer` using Customer ID `6828088731` and target
dataset `mydataset`. The transfer also specifies a custom floodlight variable. The
data transfer is created in the default project:

```bash
bq mk \
--transfer_config \
--target_dataset=mydataset \
--display_name='My Transfer' \
--data_source=search_ads \
--params='{"customer_id":"6828088731", "custom_floodlight_variables":"[{\"id\": \"9876\", \"cfv_field_name\": \"raw_event_conversion_metrics\", \"destination_table_name\": [\"Conversion\"],\"bigquery_column_name_suffix\": \"suffix1\" }]"}'
```

The first time you run the command, you receive a message like the
following:

`[URL omitted] Please copy and paste the above URL into your web browser and
follow the instructions to retrieve an authentication code.`

Follow the instructions in the message and paste the authentication code on
the command line.

> [!CAUTION]
> **Caution:** When you create a Search Ads 360 data transfer using the command-line tool, the transfer configuration is set up using the default values for **Schedule** (every 24 hours at creation time) and **Refresh window** (0 --- configures the default refresh window of 7 days).

### API

Use the
[`projects.locations.transferConfigs.create`](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs/create)
method and supply an instance of the
[`TransferConfig`](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs#TransferConfig)
resource.

## Manually trigger a Search Ads 360 transfer

When you [manually trigger a transfer](https://docs.cloud.google.com/bigquery/docs/working-with-transfers#manually_trigger_a_transfer)
for Search Ads 360, snapshots of match tables are taken once a day
and stored in the
partition for the last run date. When you trigger a manual transfer, Match
Table snapshots for the following tables are not updated:

- Account
- Ad
- AdGroup
- AdGroupCriterion
- Any [ID mapping table](https://docs.cloud.google.com/bigquery/docs/search-ads-transfer#id-mapping)
- Asset
- BidStrategy
- Campaign
- CampaignCriterion
- ConversionAction
- Keyword
- NegativeAdGroupKeyword
- NegativeAdGroupCriterion
- NegativeCampaignKeyword
- NegativeCampaignCriterion
- ProductGroup

## Performance Max (PMax) campaigns

The Search Ads 360 connector lets you export [PMax
campaigns](https://support.google.com/google-ads/answer/10724817) data. You must
select the **Include PMax Campaign Data** checkbox when
[creating a data transfer](https://docs.cloud.google.com/bigquery/docs/search-ads-transfer#setup-data-transfer), as PMax data is not exported
by default.

Including PMax data removes
`ad_group` fields from certain tables and includes new tables. You cannot include
`ad_group` fields because the Search Ads 360 API filters the PMax data.

The following tables exclude `ad_group` related columns when the **Include PMax
Campaign Tables** checkbox is selected:

- CartDataSalesStats
- ProductAdvertised
- ProductAdvertisedDeviceStats
- ProductAdvertisedConversionActionAndDeviceStats

## Support for Search Ads 360 manager accounts

Using Search Ads 360 manager accounts provides several benefits over
using individual Customer IDs:

- You don't need to manage multiple data transfers to report on multiple Customer IDs.
- Cross-customer queries are simpler to write because all Customer IDs are stored in the same table.
- Using manager accounts alleviates BigQuery Data Transfer Service load quota issues because multiple Customer IDs are loaded in the same job.

For existing customers who have multiple Customer ID-specific Search Ads 360
data transfers, we recommend that you switch to a Search Ads 360 manager account instead.
You can do this with the following steps:

1. Set up a single Search Ads 360 data transfer at the manager or sub-manager account level.
2. [Schedule a backfill.](https://docs.cloud.google.com/bigquery/docs/working-with-transfers#manually_trigger_a_transfer)
3. [Disable](https://docs.cloud.google.com/bigquery/docs/working-with-transfers#disable_a_transfer) individual Customer ID-specific Search Ads 360 transfers.

For more information about Search Ads 360 manager accounts, see [About manager accounts in the new Search Ads 360](https://support.google.com/sa360/answer/9158072)
and [See how accounts are linked to your manager account](https://support.google.com/sa360/answer/9227233).

> [!NOTE]
> **Note:** The BigQuery Data Transfer Service pulls reports for all listed Customer IDs, but you may not see Customer IDs in your reports if it doesn't report activity for the requested day.

### Example

The following list shows the Customer IDs linked to particular Search Ads 360
manager accounts:

- 1234567890 --- root manager account
  - 1234 --- sub-manager account
    - 1111 --- Customer ID
    - 2222 --- Customer ID
    - 3333 --- Customer ID
    - 4444 --- Customer ID
    - 567 --- sub-manager account
      - 5555 --- Customer ID
      - 6666 --- Customer ID
      - 7777 --- Customer ID
  - 89 --- sub-manager account
    - 8888 --- Customer ID
    - 9999 --- Customer ID
  - 0000 --- Customer ID

Each Customer ID is linked to a manager account appears in each report. For more
information about the Search Ads 360 reporting structure in
BigQuery Data Transfer Service, see [Search Ads 360 report transformation](https://docs.cloud.google.com/bigquery/docs/search-ads-transformation).

#### Transfer configuration for Customer ID 1234567890

A transfer configuration for the root manager account (Customer ID 1234567890)
generates data transfer runs that include the following Customer IDs:

- 1111 (via sub-manager account 1234)
- 2222 (via sub-manager account 1234)
- 3333 (via sub-manager account 1234)
- 4444 (via sub-manager account 1234)
- 5555 (via sub-manager account 567 and sub-manager account 1234)
- 6666 (via sub-manager account 567 and sub-manager account 1234)
- 7777 (via sub-manager account 567 and sub-manager account 1234)
- 8888 (via sub-manager account 89)
- 9999 (via sub-manager account 89)
- 0000 (individual Customer ID)

#### Transfer configuration for Customer ID 1234

A transfer configuration for sub-manager account 123 (Customer ID 1234) generates
data transfer runs that include the following Customer IDs:

- 1111
- 2222
- 3333
- 4444
- 5555 (via sub-manager account 567)
- 6666 (via sub-manager account 567)
- 7777 (via sub-manager account 567)

#### Transfer configuration for Customer ID 567

A transfer configuration for sub-manager account 567 (Customer ID 567) generates
data transfer runs that include the following Customer IDs:

- 5555
- 6666
- 7777

#### Transfer configuration for Customer ID 89

A transfer configuration for sub-manager account 89 (Customer ID 89) generates
data transfer runs that include the following Customer IDs:

- 8888
- 9999

#### Transfer configuration for Customer ID 0000

A transfer configuration for Customer ID 0000 generates data transfer runs that
include only the individual Customer ID:

- 0000

## Query your data

When your data is transferred to BigQuery Data Transfer Service, the data is written
to ingestion-time partitioned tables. For more information, see [Introduction to partitioned tables](https://docs.cloud.google.com/bigquery/docs/partitioned-tables).

If you query your tables directly instead of using the auto-generated views, you
must use the `_PARTITIONTIME` pseudocolumn in your query. For more information,
see [Querying partitioned tables](https://docs.cloud.google.com/bigquery/docs/querying-partitioned-tables).

## Search Ads 360 sample queries

You can use the following Search Ads 360 sample queries to analyze your transferred
data. You can also view the queries in a visualization tool such as [Data Studio](https://cloud.google.com/looker-studio).

The following queries are examples to get started querying your Search Ads 360 data
with BigQuery Data Transfer Service. For additional questions about what you can do
with these reports, contact your Search Ads 360 technical representative.

If you query your tables directly instead of using the auto-generated views, you
must use the `_PARTITIONTIME` pseudocolumn in your query. For more information,
see [Querying partitioned tables](https://docs.cloud.google.com/bigquery/docs/querying-partitioned-tables).

### Campaign performance

The following sample query analyzes Search Ads 360 campaign performance for the past
30 days.

```googlesql
SELECT
  c.customer_id,
  c.campaign_name,
  c.campaign_status,
  SUM(cs.metrics_clicks) AS Clicks,
  (SUM(cs.metrics_cost_micros) / 1000000) AS Cost,
  SUM(cs.metrics_impressions) AS Impressions
FROM
  `DATASET.sa_Campaign_CUSTOMER_ID` c
LEFT JOIN
  `DATASET.sa_CampaignStats_CUSTOMER_ID` cs
ON
  (c.campaign_id = cs.campaign_id
  AND cs._DATA_DATE BETWEEN
  DATE_ADD(CURRENT_DATE(), INTERVAL -31 DAY) AND DATE_ADD(CURRENT_DATE(), INTERVAL -1 DAY))
WHERE
  c._DATA_DATE = c._LATEST_DATE
GROUP BY
  1, 2, 3
ORDER BY
  Impressions DESC
```

Replace the following:

- `DATASET`: the name of the dataset
- `CUSTOMER_ID`: the Search Ads 360 customer ID

### Count of keywords

The following sample query analyzes keywords by campaign, ad group, and keyword
status.

```googlesql
  SELECT
    c.campaign_status AS CampaignStatus,
    a.ad_group_status AS AdGroupStatus,
    k.ad_group_criterion_status AS KeywordStatus,
    k.ad_group_criterion_keyword_match_type AS KeywordMatchType,
    COUNT(*) AS count
  FROM
    `DATASET.sa_Keyword_CUSTOMER_ID` k
    JOIN
    `DATASET.sa_Campaign_CUSTOMER_ID` c
  ON
    (k.campaign_id = c.campaign_id AND k._DATA_DATE = c._DATA_DATE)
  JOIN
    `DATASET.sa_AdGroup_CUSTOMER_ID` a
  ON
    (k.ad_group_id = a.ad_group_id AND k._DATA_DATE = a._DATA_DATE)
  WHERE
    k._DATA_DATE = k._LATEST_DATE
  GROUP BY
    1, 2, 3, 4
```

Replace the following:

- `DATASET`: the name of the dataset
- `CUSTOMER_ID`: the Search Ads 360 customer ID

## ID mapping tables

Entities in the new Search Ads 360, such as customers, campaigns, and ad groups,
have a different [ID space](https://developers.google.com/search-ads/v2/how-tos/reporting/id-mapping)
than the old Search Ads 360. For existing Search Ads 360 transfer
users who want to combine data from the old Search Ads 360 with the
new Search Ads 360 API, you can use the BigQuery Data Transfer Service to transfer
ID mapping tables if you provide a valid agency ID and advertiser ID in the
transfer configuration.

[Supported entities](https://developers.google.com/search-ads/v2/how-tos/reporting/id-mapping)
contain two columns, `legacy_id` and `new_id`, which specifies the ID mapping
for entities in old and new versions of Search Ads 360 respectively.
For the AD, CAMPAIGN_CRITERION, and CRITERION entities, a `new_secondary_id` is
also provided as these entities [don't have globally unique ids in the new
Search Ads 360](https://developers.google.com/search-ads/v2/how-tos/reporting/id-mapping#object-id-uniqueness).
The following is a list of ID mapping tables.

- IdMapping_AD
- IdMapping_AD_GROUP
- IdMapping_CAMPAIGN
- IdMapping_CAMPAIGN_CRITERION
- IdMapping_CAMPAIGN_GROUP
- IdMapping_CAMPAIGN_GROUP_PERFORMANCE_TARGET
- IdMapping_CRITERION
- IdMapping_CUSTOMER
- IdMapping_FEED_ITEM
- IdMapping_FEED_TABLE

> [!NOTE]
> **Note:** Similar to match tables, snapshots of ID mapping tables are taken once a day and stored in the partition for the latest run date. ID mapping table snapshots are not updated for backfills or for days loaded using the refresh window.

### Example queries

The following query makes use of ID mapping tables to aggregate per-campaign
metrics across tables from previous and new Search Ads 360 data transfers in the
new ID space.

```googlesql
SELECT CustomerID, CampaignID, Sum(Clicks), Sum(Cost) FROM
(SELECT
  cs.customer_id AS CustomerID,
  cs.campaign_id AS CampaignID,
  cs.metrics_clicks AS Clicks,
  cs.metrics_cost_micros / 1000000 AS Cost
FROM
  `DATASET.sa_CampaignStats_CUSTOMER_ID` cs
WHERE cs._DATA_DATE = 'NEW_DATA_DATE'
UNION ALL
SELECT
  customer_id_mapping.new_id AS CustomerID,
  campaign_id_mapping.new_id AS CampaignID,
  cs.clicks AS Clicks,
  cs.cost AS Cost
FROM
  `DATASET.CampaignStats_ADVERTISER_ID` cs
LEFT JOIN
  `DATASET.IdMapping_CUSTOMER_ADVERTISER_ID` customer_id_mapping
ON cs.accountId = customer_id_mapping.legacy_id
LEFT JOIN
  `DATASET.IdMapping_CAMPAIGN_ADVERTISER_ID` campaign_id_mapping
ON cs.campaignId = campaign_id_mapping.legacy_id
WHERE cs._DATA_DATE = 'OLD_DATA_DATE')
GROUP BY
1, 2
ORDER BY
1, 2
```

Replace the following:

- `DATASET`: the name of the dataset
- `CUSTOMER_ID`: the Search Ads 360 customer ID
- `ADVERTISER_ID`: the Search Ads 360 advertiser ID
- `NEW_DATA_DATE`: the data date for the new Search Ads 360 table
- `OLD_DATA_DATE`: the data date for the previous Search Ads 360 table

The following query makes use of ID mapping tables to aggregate per-campaign
metrics across tables from previous and new Search Ads 360 data transfers in the
old ID space.

```googlesql
SELECT CustomerID, CampaignID, Sum(Clicks), Sum(Cost) FROM
(SELECT
  customer_id_mapping.legacy_id AS CustomerID,
  campaign_id_mapping.legacy_id AS CampaignID,
  cs.metrics_clicks AS Clicks,
  cs.metrics_cost_micros / 1000000 AS Cost
FROM
  `DATASET.sa_CampaignStats_CUSTOMER_ID` cs
LEFT JOIN
  `DATASET.IdMapping_CUSTOMER_ADVERTISER_ID` customer_id_mapping
ON cs.customer_id = customer_id_mapping.new_id
LEFT JOIN
  `DATASET.IdMapping_CAMPAIGN_ADVERTISER_ID` campaign_id_mapping
ON cs.campaign_id = campaign_id_mapping.new_id
WHERE cs._DATA_DATE = 'NEW_DATA_DATE'
UNION ALL
SELECT
  CAST(accountId AS INT) AS CustomerID,
  CAST(campaignId AS INT) AS CampaignID,
  cs.clicks AS Clicks,
  cs.cost AS Cost
FROM
  `DATASET.CampaignStats_ADVERTISER_ID` cs
WHERE cs._DATA_DATE = 'OLD_DATA_DATE')
GROUP BY
1, 2
ORDER BY
1, 2
```

Replace the following:

- `DATASET`: the name of the dataset
- `CUSTOMER_ID`: the Search Ads 360 customer ID
- `ADVERTISER_ID`: the Search Ads 360 advertiser ID
- `NEW_DATA_DATE`: the data date for the new Search Ads 360 table
- `OLD_DATA_DATE`: the data date for the previous Search Ads 360 table

## Potential quota issues

The Search Ads 360 reporting API assigns a
[quota](https://developers.google.com/search-ads/reporting/concepts/quotas#query_limits)
for the number of requests that the Google project can send. If you are using one
project for the BigQuery Data Transfer Service and other services, all services share the
same quota and can potentially reach the quota limit in any service.

To prevent this potential issue without affecting existing workflows, consider
these options:

- Use the `table_filter` parameter to load only the tables that are needed.
- Set up a separate project for the BigQuery Data Transfer Service. A cross project
  table join might look like the following:

        #standardSQL
        select count(a.item1)
        from (select item1, item2 from project-A.data_set_a.table_name_a) a
        inner join
        (select item3, item4 from project-B.data_set_b.table_name_b) b
        on a.item1 = b.item3

  <br />

- Contact [Search Ads 360 support](https://support.google.com/searchads/answer/9026876)
  and request additional quota.