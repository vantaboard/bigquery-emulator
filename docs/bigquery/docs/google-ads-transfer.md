# Load Google Ads data into BigQuery

You can load data from Google Ads (formerly known as
Google AdWords) to BigQuery using the
[BigQuery Data Transfer Service](https://docs.cloud.google.com/bigquery/docs/dts-introduction) for Google Ads connector. With the
BigQuery Data Transfer Service, you can schedule recurring transfer jobs that
add your latest data from your Google Ads to
BigQuery.

To learn about recent data
source changes, see [BigQuery Data Transfer Service data source change
log](https://docs.cloud.google.com/bigquery/docs/transfer-changes).

## Connector overview

The BigQuery Data Transfer Service for the Google Ads connector supports the following options for your data transfer.

| Data transfer options | Support |
|---|---|
| Supported reports | The Google Ads connector supports the transfer of data from the reports in [Google Ads API v22](https://developers.google.com/google-ads/api/fields/v22/overview). For information about how Google Ads reports are transformed into BigQuery tables and views, see [Google Ads report transformation](https://docs.cloud.google.com/bigquery/docs/google-ads-transformation). |
| Repeat frequency | The Google Ads connector supports daily data transfers. <br /> By default, data transfers are scheduled at the time when the data transfer is created. You can configure the time of data transfer when you [set up your data transfer](https://docs.cloud.google.com/bigquery/docs/google-ads-transfer#setup-data-transfer). |
| Refresh window | You can schedule your data transfers to retrieve Google Ads data from up to 30 days at the time the data transfer is run. You can configure the duration of the refresh window when you [set up your data transfer](https://docs.cloud.google.com/bigquery/docs/google-ads-transfer#setup-data-transfer). <br /> By default, the Google Ads connector has a refresh window of 7 days. <br /> For more information, see [Refresh windows](https://docs.cloud.google.com/bigquery/docs/google-ads-transfer#refresh). Snapshots of [Match Tables](https://docs.cloud.google.com/bigquery/docs/google-ads-transformation#google_ads_match_tables) are taken once a day and stored in the partition for the last run date. Match Table snapshots are not updated for backfills or for days loaded using the refresh window. |
| Backfill data availability | [Run a data backfill](https://docs.cloud.google.com/bigquery/docs/working-with-transfers#manually_trigger_a_transfer) to retrieve data outside of your scheduled data transfer. You can retrieve data as far back as the data retention policy on your data source allows. <br /> For information about the data retention policy for Google Ads, see [Google Ads Data Retention Policy](https://support.google.com/google-ads/answer/15188209). |
| Number of Customer IDs per manager account | 8,000 The BigQuery Data Transfer Service supports a maximum of **8000 Customer IDs** for each Google Ads [manager account](https://support.google.com/adwords/answer/6139186) (MCC). |

To map Google Ads reports to what you see in the Google Ads UI,
see [Mapping reports to the Google Ads UI](https://developers.google.com/google-ads/api/docs/conversions/ui-mapping).

## Data ingestion from Google Ads transfers

When you transfer data from Google Ads into BigQuery, the data is loaded into BigQuery tables that are partitioned by date. The table partition that the data is loaded into corresponds to the date from the data source. If you schedule multiple transfers for the same date, BigQuery Data Transfer Service overwrites the partition for that specific date with the latest data. Multiple transfers in the same day or running backfills don't result in duplicate data, and partitions for other dates are not affected.

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

- The maximum frequency that you can configure a Google Ads data transfer for is once every 24 hours. By default, a transfer starts at the time that you create the transfer. However, you can configure the transfer start time when you [create your transfer](https://docs.cloud.google.com/bigquery/docs/google-ads-transfer#setup-data-transfer).
- The BigQuery Data Transfer Service does not support incremental data transfers during a Google Ads transfer. When you specify a date for a data transfer, all of the data that is available for that date is transferred.

## Before you begin

Before you create a Google Ads data transfer, do the following:

- Verify that you have completed all actions required to [enable the BigQuery Data Transfer Service](https://docs.cloud.google.com/bigquery/docs/enable-transfer-service).
- [Create a BigQuery Data Transfer Service dataset](https://docs.cloud.google.com/bigquery/docs/datasets) to store the Google Ads data.
- If you intend to set up transfer run notifications for Pub/Sub, ensure that you have the `pubsub.topics.setIamPolicy` permission. Pub/Sub permissions are not required if you set up email notifications. For more information, see [BigQuery Data Transfer Service run notifications](https://docs.cloud.google.com/bigquery/docs/transfer-run-notifications).

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

### Required Google Ads roles

You must grant read access to the Google Ads Customer ID or
[manager account](https://support.google.com/google-ads/answer/6139186) (MCC)
that is used in the transfer configuration. When you create a new transfer configuration that is authorized with individual user credentials, you must [enable 2-Step Verification](https://support.google.com/accounts/answer/185839).

To authorize a service account for your Google Ads data transfer,
we recommend that you grant the service account direct-account access to Google Ads.
For more information, see [Authorization with direct account access](https://developers.google.com/google-ads/api/docs/oauth/service-accounts#direct). Multi-factor authentication (MFA) requirements, such as 2-Step Verification, are not needed for transfer configurations that are authorized with service accounts.

## Create Google Ads data transfer

To create a data transfer for Google Ads reporting, you need
either your Google Ads customer ID or your manager account (MCC).
For information about retrieving your Google Ads customer ID, see
[Find your Customer ID](https://support.google.com/google-ads/answer/1704344).

To create a data transfer for Google Ads reporting, select one of the following options:

### Console

1. Go to the Data transfers page in the Google Cloud console.

   [Go to Data transfers](https://console.cloud.google.com/bigquery/transfers)
2. Click **Create transfer**.

3. In the **Source type** section, for **Source** , choose **Google Ads**.

4. In the **Data source details** section:

   1. For **Customer ID**, enter your Google Ads customer ID.
   2. For **Report type** , select either **Standard** or **Custom** .
      - If you've selected **Standard** , the transfer includes the standard set of reports and fields as detailed in [Google Ads report transformation](https://docs.cloud.google.com/bigquery/docs/google-ads-transformation).
        - Optional: Select options to exclude removed or deactivated items and include tables new to Google Ads.
        - Optional: Enter a comma-separated list of tables to include, for example, `Campaign, AdGroup`. Prefix this list with the `-` character to exclude certain tables, for example `-Campaign, AdGroup`. All tables are included by default.
        - Optional: Select the option to include tables specific to PMax reports. For more information about PMax support, see [PMax support](https://docs.cloud.google.com/bigquery/docs/google-ads-transfer#pmax-support).
        - Optional: For **Refresh window**, enter a value between 1 and 30.
      - If you've selected **Custom** , enter an output table and a Google Ads query for each [custom report](https://docs.cloud.google.com/bigquery/docs/google-ads-transfer#custom_reports) you want to include in this transfer.
        - Optional: Click **Add query** to add a new custom report.
        - Optional: For **Refresh window**, enter a value between 1 and 30.
5. In the **Destination settings** section, for **Dataset**, select the
   dataset that you created to store your data.

6. In the **Transfer config name** section, for **Display name** , enter a
   name for the data transfer such as `My Transfer`. The transfer name can be
   any value that lets you identify the transfer if you need to modify it
   later.

7. In the **Schedule options** section:

   - For **Repeat frequency** , choose an option for how often to run the data transfer. If you select **Days** , provide a valid time in UTC.
     - Hours
     - Days
     - On-demand
   - If applicable, select either **Start now** or **Start at set time**, and provide a start date and run time.
8. In the **Service Account** menu, select a
   [service account](https://docs.cloud.google.com/iam/docs/service-account-overview) from the service
   accounts associated with your Google Cloud project. You can
   associate a service account with your data transfer instead of using your
   user credentials. For more information about using service accounts with
   data transfers, see
   [Use service accounts](https://docs.cloud.google.com/bigquery/docs/use-service-accounts).

   - If you signed in with a [federated identity](https://docs.cloud.google.com/iam/docs/workforce-identity-federation), then a service account is required to create a transfer. If you signed in with a [Google Account](https://docs.cloud.google.com/iam/docs/principals-overview#google-account), then a service account for the transfer is optional.
   - The service account must have the [required permissions](https://docs.cloud.google.com/bigquery/docs/google-ads-transfer#required_permissions).
9. Optional: In the **Notification options** section:

   - Click the toggle to enable email notifications. When you enable this option, the transfer administrator receives an email notification if a transfer run fails.
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
- `--table_filter`: Specifies which tables to include in the data transfer. If the flag is not specified, all tables are included. To include only specific tables, use a comma-separated list of values (for example, `Ad`, `Campaign`, `AdGroup`). To exclude specific tables, prefix the values with a hyphen (`-`) (for example, `-Ad`, `Campaign`, `AdGroup`).
- `--schedule`: Specifies how often the query runs. If you don't specify `--schedule`, the default is set to `every 24 hours`. For information about the schedule syntax, see [Formatting the schedule](https://docs.cloud.google.com/appengine/docs/flexible/scheduling-jobs-with-cron-yaml#formatting_the_schedule).
- `--refresh_window_days`: Specifies the refresh window for a transfer configuration in days. The default value is `7`.
- `--service_account_name`: Specifies a service account to use for the Google Ads transfer authentication instead of your user account.
- `--include_pmax`: Specify `true` to include tables specific to PMax reports. The default value is `false`. For more information about PMax support, see [PMax
  support](https://docs.cloud.google.com/bigquery/docs/google-ads-transfer#pmax-support)

```bash
bq mk \
--transfer_config \
--project_id=PROJECT_ID \
--target_dataset=DATASET \
--display_name=NAME \
--params='PARAMETERS' \
--data_source=DATA_SOURCE \
--table_filter=TABLES \
--schedule=SCHEDULE \
--refresh_window_days=REFRESH_DAYS \
--service_account_name=SERVICE_ACCOUNT_NAME \
--include_pmax=PMAX_ENABLE
```

Where:

- <var translate="no">PROJECT_ID</var> is your project ID.
- <var translate="no">DATASET</var> is the target dataset for the data transfer configuration.
- <var translate="no">NAME</var> is the display name for the data transfer configuration. The transfer name can be any value that lets you identify the transfer if you need to modify it later.
- <var translate="no">PARAMETERS</var> contains the following JSON parameters for the transfer configuration: `--params='{"param":"param_value"}'`.
  - `customer_id`: enter your Google Ads customer ID. This field is required.
  - `custom_report_table_names`: a list of table names for the [custom
    reports](https://docs.cloud.google.com/bigquery/docs/google-ads-transfer#custom_reports) included in this transfer. This list corresponds to the queries in `custom_report_queries`. The length of this list must match the length of the list in `custom_report_queries`.
  - `custom_report_queries`: a list of [Google Ads Query
    Language (GAQL) queries](https://docs.cloud.google.com/bigquery/docs/google-ads-transfer#custom_reports) for the custom reports included in this transfer. This list corresponds to the names in `custom_report_table_names`. The length of this list must match the length of the list in `custom_report_table_names`.
  - Optional: set the `exclude_removed_items` parameter to `true` to prevent removed or disabled entities and metrics from being transferred.
- <var translate="no">DATA_SOURCE</var> is the data source --- `google_ads`.
- <var translate="no">TABLES</var> is the comma-separated list of tables to include or exclude from the data transfer.
- <var translate="no">SCHEDULE</var> is how often you want the query to run. If `--schedule` isn't specified, the default is every 24 hours, starting from the time the transfer is created.
- <var translate="no">REFRESH_DAYS</var> is an integer that specifies the refresh window for a transfer configuration in days. The default value is `7`.
- <var translate="no">SERVICE_ACCOUNT_NAME</var> is the service account name used to authenticate your transfer. The service account must be owned by the same `project_id` used to create the transfer and it must have all of the [required permissions](https://docs.cloud.google.com/bigquery/docs/google-ads-transfer#required_permissions).
- <var translate="no">PMAX_ENABLE</var>: Specify `true` to include tables specific to PMax reports. The default value is `false`. For more information about PMax support, see [PMax
  support](https://docs.cloud.google.com/bigquery/docs/google-ads-transfer#pmax-support)

> [!CAUTION]
> **Caution:** You can't configure notifications using the command-line tool.

For example, the following command creates a Google Ads data transfer named
`My Transfer` using Customer ID `123-123-1234` and target dataset
`mydataset`. The data transfer is created in the default project:

    bq mk \
    --transfer_config \
    --target_dataset=mydataset \
    --display_name='My Transfer' \
    --params='{"customer_id":"123-123-1234","exclude_removed_items":"true"}' \
    --data_source=google_ads

The first time you run the command, you receive a message like the
following:

`[URL omitted] Please copy and paste the above URL into your web browser and
follow the instructions to retrieve an authentication code.`

Follow the instructions in the message and paste the authentication code on
the command line.

### API

Use the [`projects.locations.transferConfigs.create`](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs/create)
method and supply an instance of the [`TransferConfig`](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs#TransferConfig)
resource.

### Java


Before trying this sample, follow the Java setup instructions in the
[BigQuery quickstart using
client libraries](https://docs.cloud.google.com/bigquery/docs/quickstarts/quickstart-client-libraries).


For more information, see the
[BigQuery Java API
reference documentation](https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/overview).


To authenticate to BigQuery, set up Application Default Credentials.
For more information, see

[Set up authentication for client libraries](https://docs.cloud.google.com/bigquery/docs/authentication#client-libs).

    import com.google.api.gax.rpc.https://docs.cloud.google.com/java/docs/reference/gax/latest/com.google.api.gax.rpc.ApiException.html;
    import com.google.cloud.bigquery.datatransfer.v1.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.CreateTransferConfigRequest.html;
    import com.google.cloud.bigquery.datatransfer.v1.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.DataTransferServiceClient.html;
    import com.google.cloud.bigquery.datatransfer.v1.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.ProjectName.html;
    import com.google.cloud.bigquery.datatransfer.v1.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.TransferConfig.html;
    import com.google.protobuf.https://docs.cloud.google.com/java/docs/reference/protobuf/latest/com.google.protobuf.Struct.html;
    import com.google.protobuf.https://docs.cloud.google.com/java/docs/reference/protobuf/latest/com.google.protobuf.Value.html;
    import java.io.IOException;
    import java.util.HashMap;
    import java.util.Map;

    // Sample to create ads(formerly AdWords) transfer config
    public class CreateAdsTransfer {

      public static void main(String[] args) throws IOException {
        // TODO(developer): Replace these variables before running the sample.
        final String projectId = "MY_PROJECT_ID";
        String datasetId = "MY_DATASET_ID";
        // the customer_id only allows digits and hyphen ('-').
        String customerId = "012-345-6789";
        String refreshWindow = "100";
        Map<String, Value> params = new HashMap<>();
        params.put("customer_id", https://docs.cloud.google.com/java/docs/reference/protobuf/latest/com.google.protobuf.Value.html.newBuilder().setStringValue(customerId).build());
        params.put("refreshWindow", https://docs.cloud.google.com/java/docs/reference/protobuf/latest/com.google.protobuf.Value.html.newBuilder().setStringValue(refreshWindow).build());
        https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.TransferConfig.html transferConfig =
            https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.TransferConfig.html.newBuilder()
                .setDestinationDatasetId(datasetId)
                .setDisplayName("Your Ads Transfer Config Name")
                .setDataSourceId("adwords")
                .setParams(https://docs.cloud.google.com/java/docs/reference/protobuf/latest/com.google.protobuf.Struct.html.newBuilder().https://docs.cloud.google.com/java/docs/reference/protobuf/latest/com.google.protobuf.Struct.Builder.html#com_google_protobuf_Struct_Builder_putAllFields_java_util_Map_java_lang_String_com_google_protobuf_Value__(params).build())
                .build();
        createAdsTransfer(projectId, transferConfig);
      }

      public static void createAdsTransfer(String projectId, https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.TransferConfig.html transferConfig)
          throws IOException {
        try (https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.DataTransferServiceClient.html client = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.DataTransferServiceClient.html.create()) {
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.ProjectName.html parent = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.ProjectName.html.of(projectId);
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.CreateTransferConfigRequest.html request =
              https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.CreateTransferConfigRequest.html.newBuilder()
                  .setParent(parent.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.ProjectName.html#com_google_cloud_bigquery_datatransfer_v1_ProjectName_toString__())
                  .setTransferConfig(transferConfig)
                  .build();
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.TransferConfig.html config = client.createTransferConfig(request);
          System.out.println("Ads transfer created successfully :" + config.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.TransferConfig.html#com_google_cloud_bigquery_datatransfer_v1_TransferConfig_getName__());
        } catch (https://docs.cloud.google.com/java/docs/reference/gax/latest/com.google.api.gax.rpc.ApiException.html ex) {
          System.out.print("Ads transfer was not created." + ex.toString());
        }
      }
    }

## Manually trigger a Google Ads transfer

When you [manually trigger a transfer](https://docs.cloud.google.com/bigquery/docs/working-with-transfers#manually_trigger_a_transfer)
for Google Ads, snapshots of Match Tables are taken once a day
and stored in the partition for the latest run date. When you trigger a
manual transfer, Match Table snapshots for the following tables are not updated:

- Ad
- AdGroup
- AdGroupAudience
- AdGroupBidModifier
- AdGroupAdLabel
- AdGroupCriterion
- AdGroupCriterionLabel
- AdGroupLabel
- AgeRange
- Asset
- AssetGroup
- AssetGroupAsset
- AssetGroupListingGroupFilter
- AssetGroupSignal
- Audience
- BidGoal
- Budget
- Campaign
- CampaignAudience
- CampaignCriterion
- CampaignLabel
- Customer
- Gender
- Keyword
- LocationBasedCampaignCriterion
- ParentalStatus
- Placement
- Video

## Custom reports

The BigQuery Data Transfer Service for Google Ads connector also
supports the use of custom reports using Google Ads Query
Language (GAQL) queries in the Google Ads transfer configuration.
These custom reports ingest data from all resources available in the
[Google Ads API version supported by the BigQuery Data Transfer Service](https://docs.cloud.google.com/bigquery/docs/google-ads-transfer#connector_overview).
For more information about using and validating a GAQL query, see [Google Ads Query Builder](https://developers.google.com/google-ads/api/fields/v22/overview_query_builder).

You can specify custom reports when you [Create a Google Ads transfer](https://docs.cloud.google.com/bigquery/docs/google-ads-transfer#setup-data-transfer).

### Custom reports limitations

Custom reports with the Google Ads connector are subject to the
following limitations:

- The Google Ads connector doesn't support `WHERE`, `ORDER BY`, `LIMIT`, and `PARAMETERS` clauses. Your GAQL query should be in the format similar to the following: `SELECT FIELD_NAME, FIELD_NAME,... FROM RESOURCE_NAME`.
- The Google Ads connector automatically appends `WHERE
  segments.date = run_date` when there is a core date segment (for example, `segments.date`, `segments.week`, `segments.month`, `segments.quarter`, `segments.year`) in the query. This can cause the [Google Ads Query Validator](https://developers.google.com/google-ads/api/fields/v22/query_validator) to return an error, for example, `The filtering conditions in the WHERE clause must
  combine to form a valid, finite date range composed of the core date segments
  ...`. You can safely ignore these errors.
- GAQL queries without a [`segments.date` field](https://developers.google.com/google-ads/api/fields/v22/segments#segments.date) acts as [match tables](https://docs.cloud.google.com/bigquery/docs/google-ads-transformation#google_ads_match_tables), which are only updated once per day and are not supported in backfill runs. If you want to backfill data, you must include a `segments.date` field in the GAQL query.
- The Google Ads connector supports up to 100 custom reports in a single transfer.

## Performance Max (PMax) campaigns

The Google Ads connector lets you export [PMax
campaigns](https://support.google.com/google-ads/answer/10724817) data. You must
select the **Include PMax Campaign Tables** checkbox when
[creating a data transfer](https://docs.cloud.google.com/bigquery/docs/google-ads-transfer#setup-data-transfer), as PMax data is not exported
by default.

Including PMax data removes
`ad_group` fields from certain tables and includes new tables. You cannot include
`ad_group` fields because the Google Ads API filters the PMax data.

The following tables exclude `ad_group` related columns when the **Include PMax
Campaign Tables** checkbox is selected:

- GeoStats
- GeoConversionStats
- ShoppingProductConversionStats
- ShoppingProductStats
- LocationsUserLocationsStats

The following tables are added when the **Include PMax Campaign Tables**
checkbox is selected:

- Asset
- AssetGroup
- AssetGroupAsset
- AssetGroupListingGroupFilter
- AssetGroupSignal
- Audience
- AssetGroupProductGroupStats
- CampaignAssetStats

## Support for Google Ads manager accounts

Existing customers who have multiple Customer ID-specific Google Ads Transfers
are encouraged to set up a single Google Ads Transfer at the Manager Account
(MCC) level, schedule a backfill, and disable individual Customer ID-specific
Google Ads Transfers.

Using Google Ads manager accounts provides several benefits over using
individual Customer IDs:

- You no longer need to manage multiple data transfers to report on multiple Customer IDs.
- Cross-customer queries are much simpler to write because all the Customer IDs are stored in the same table.
- Using MCCs alleviates BigQuery Data Transfer Service load quota issues because multiple Customer IDs are loaded in the same job.

For more information about Google Ads manager accounts (MCCs), see
[Working with managed accounts](https://support.google.com/google-ads/topic/7554359)
and [About linking accounts to your manager account](https://support.google.com/google-ads/answer/7456530).

> [!NOTE]
> **Note:** The BigQuery Data Transfer Service pulls reports for all listed Customer IDs, but you may not see Customer IDs in your reports if they don't report activity for the requested day or the account is inactive when the transfer run happens.

### Example

The following list shows the Customer IDs linked to particular Google Ads
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

Each Customer ID linked to a manager account appears in each report. For more
information about the Google Ads reporting structure in BigQuery Data Transfer Service,
see
[Google Ads report transformation](https://docs.cloud.google.com/bigquery/docs/google-ads-transformation).

#### Transfer configuration for Customer ID 1234567890

A transfer configuration for the root manager account (Customer ID 1234567890)
would generate data transfer runs that include the following Customer IDs:

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

A transfer configuration for sub-manager account 123 (Customer ID 1234) would
generate data transfer runs that include the following Customer IDs:

- 1111
- 2222
- 3333
- 4444
- 5555 (via sub-manager account 567)
- 6666 (via sub-manager account 567)
- 7777 (via sub-manager account 567)

#### Transfer configuration for Customer ID 567

A transfer configuration for sub-manager account 567 (Customer ID 567) would
generate data transfer runs that include the following Customer IDs:

- 5555
- 6666
- 7777

#### Transfer configuration for Customer ID 89

A transfer configuration for sub-manager account 89 (Customer ID 89) would
generate data transfer runs that include the following Customer IDs:

- 8888
- 9999

#### Transfer configuration for Customer ID 0000

A transfer configuration for Customer ID 0000 would generate data transfer runs that
include only the individual Customer ID:

- 0000

### Migrate Google Ads data to MCCs

To migrate your existing Google Ads data in BigQuery Data Transfer Service to the
MCC structure, you can [set up a backfill](https://docs.cloud.google.com/bigquery/docs/working-with-transfers#manually_trigger_a_transfer_or_backfill)
to add your existing data to the tables created by the transfer configuration
linked to the manager account. Note that when you schedule a backfill, match
tables are not updated.

## Troubleshoot Google Ads transfer setup

If you are having issues setting up your data transfer, see
[Google Ads transfer issues](https://docs.cloud.google.com/bigquery/docs/transfer-troubleshooting#ads-transfer)
in [Troubleshooting transfer configurations](https://docs.cloud.google.com/bigquery/docs/transfer-troubleshooting).

## Query your data

When your data is transferred to BigQuery Data Transfer Service, the data is
written to ingestion-time partitioned tables. For more information, see
[Introduction to partitioned tables](https://docs.cloud.google.com/bigquery/docs/partitioned-tables).

If you query your tables directly instead of using the auto-generated views, you
must use the `_PARTITIONTIME` pseudocolumn in your query. For more information,
see [Querying partitioned tables](https://docs.cloud.google.com/bigquery/docs/querying-partitioned-tables).

## Google Ads sample queries

You can use the following Google Ads sample queries to analyze your transferred
data. You can also use the queries in a visualization tool such as
[Data Studio](https://www.google.com/analytics/data-studio/).
These queries are provided to help you get started on querying your Google Ads
data with BigQuery Data Transfer Service. For additional questions about what you can
do with these reports, contact your Google Ads technical representative.

If you query your tables directly instead of using the auto-generated
views, you must use the `_PARTITIONTIME` pseudocolumn in your query. For more
information, see [Querying partitioned tables](https://docs.cloud.google.com/bigquery/docs/querying-partitioned-tables).

### Campaign performance

The following sample query analyzes Google Ads campaign performance for the past
30 days.

### Console

```googlesql
SELECT
  c.customer_id,
  c.campaign_name,
  c.campaign_status,
  SUM(cs.metrics_impressions) AS Impressions,
  SUM(cs.metrics_interactions) AS Interactions,
  (SUM(cs.metrics_cost_micros) / 1000000) AS Cost
FROM
  `DATASET.ads_Campaign_CUSTOMER_ID` c
LEFT JOIN
  `DATASET.ads_CampaignBasicStats_CUSTOMER_ID` cs
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

### bq

```bash
  bq query --use_legacy_sql=false '
  SELECT
    c.customer_id,
    c.campaign_name,
    c.campaign_status,
    SUM(cs.metrics_impressions) AS Impressions,
    SUM(cs.metrics_interactions) AS Interactions,
    (SUM(cs.metrics_cost_micros) / 1000000) AS Cost
  FROM
    `DATASET.ads_Campaign_CUSTOMER_ID` c
  LEFT JOIN
    `DATASET.ads_CampaignBasicStats_CUSTOMER_ID` cs
  ON
    (c.campaign_id = cs.campaign_id
    AND cs._DATA_DATE BETWEEN
    DATE_ADD(CURRENT_DATE(), INTERVAL -31 DAY) AND DATE_ADD(CURRENT_DATE(), INTERVAL -1 DAY))
  WHERE
    c._DATA_DATE = c._LATEST_DATE
  GROUP BY
    1, 2, 3
  ORDER BY
    Impressions DESC'
```

Replace the following:

- <var translate="no">`DATASET`</var>: the name of the dataset that you created to store the transferred table
- <var translate="no">`CUSTOMER_ID`</var>: your Google Ads Customer ID.

### Count of keywords

The following sample query analyzes keywords by campaign, ad group, and keyword
status. This query uses the `KeywordMatchType` function. Keyword match types
help control which searches can trigger your ad. For more information about keyword
matching options, see
[About keyword matching options](https://support.google.com/google-ads/answer/2497836).

### Console

```googlesql
  SELECT
    c.campaign_status AS CampaignStatus,
    a.ad_group_status AS AdGroupStatus,
    k.ad_group_criterion_status AS KeywordStatus,
    k.ad_group_criterion_keyword_match_type AS KeywordMatchType,
    COUNT(*) AS count
  FROM
    `DATASET.ads_Keyword_CUSTOMER_ID` k
    JOIN
    `DATASET.ads_Campaign_CUSTOMER_ID` c
  ON
    (k.campaign_id = c.campaign_id AND k._DATA_DATE = c._DATA_DATE)
  JOIN
    `DATASET.ads_AdGroup_CUSTOMER_ID` a
  ON
    (k.ad_group_id = a.ad_group_id AND k._DATA_DATE = a._DATA_DATE)
  WHERE
    k._DATA_DATE = k._LATEST_DATE
  GROUP BY
    1, 2, 3, 4
```

### bq

```bash
  bq query --use_legacy_sql=false '
  SELECT
    c.campaign_status AS CampaignStatus,
    a.ad_group_status AS AdGroupStatus,
    k.ad_group_criterion_status AS KeywordStatus,
    k.ad_group_criterion_keyword_match_type AS KeywordMatchType,
    COUNT(*) AS count
  FROM
    `DATASET.ads_Keyword_CUSTOMER_ID` k
  JOIN
    `DATASET.ads_Campaign_CUSTOMER_ID` c
  ON
    (k.campaign_id = c.campaign_id AND k._DATA_DATE = c._DATA_DATE)
  JOIN
    `DATASET.ads_AdGroup_CUSTOMER_ID` a
  ON
    (k.ad_group_id = a.ad_group_id AND k._DATA_DATE = a._DATA_DATE)
  WHERE
    k._DATA_DATE = k._LATEST_DATE
  GROUP BY
    1, 2, 3, 4'
```

Replace the following:

- <var translate="no">`DATASET`</var>: the name of the dataset that you created to store the transferred table
- <var translate="no">`CUSTOMER_ID`</var>: your Google Ads Customer ID.