# Load Google Ad Manager data into BigQuery

You can load data from Google Ad Manager to BigQuery using the
[BigQuery Data Transfer Service](https://docs.cloud.google.com/bigquery/docs/dts-introduction) for Google Ad Manager connector. With the
BigQuery Data Transfer Service, you can schedule recurring transfer jobs that
add your latest data from Google Ad Manager to
BigQuery.

## Connector overview

The BigQuery Data Transfer Service for the Google Ad Manager connector supports the following options for your data transfer.

| Data transfer options | Support |
|---|---|
| Supported reports | The Google Ad Manager connector supports the transfer of data from the following reports: - [Data Transfer (Google Ad Manager DT) files](https://support.google.com/admanager/answer/1733124) - [Data Transfer fields](https://support.google.com/admanager/table/7401123) - [Match tables provided by the BigQuery Data Transfer Service](https://docs.cloud.google.com/bigquery/docs/doubleclick-publisher-transformation). These are automatically created and updated. - [Match tables fetched with PQL](https://developers.google.com/doubleclick-publishers/docs/pqlreference#matchtables) - [Match tables from CompanyService (v201908)](https://developers.google.com/doubleclick-publishers/docs/reference/v201908/CompanyService) - [Match tables from OrderService (v201908)](https://developers.google.com/doubleclick-publishers/docs/reference/v201908/OrderService) - [Match tables from PlacementService (v201908)](https://developers.google.com/doubleclick-publishers/docs/reference/v201908/PlacementService) For information about how Google Ad Manager reports are transformed into BigQuery tables and views, see [Google Ad Manager report transformation](https://docs.cloud.google.com/bigquery/docs/doubleclick-publisher-transformation). |
| Repeat frequency | The Google Ad Manager connector supports data transfers every 4 hours. By default, Google Ad Manager data transfers repeat every 8 hours. <br /> You can configure the time of data transfer when you [set up your data transfer](https://docs.cloud.google.com/bigquery/docs/doubleclick-publisher-transfer#set_up_a_google_ad_manager_transfer). |
| Refresh window | The Google Ad Manager connector retrieves Google Ad Manager data from up to 2 days at the time the data transfer is run. You cannot configure the refresh window for this connector. <br /> For more information, see [Refresh windows](https://docs.cloud.google.com/bigquery/docs/doubleclick-publisher-transfer#refresh). |
| Backfill data availability | [Run a data backfill](https://docs.cloud.google.com/bigquery/docs/working-with-transfers#manually_trigger_a_transfer) to retrieve data outside of your scheduled data transfer. You can retrieve data as far back as the data retention policy on your data source allows. <br /> For information about the data retention policy for Google Ad Manager, see [Google Ad Manager Data Transfer reports](https://support.google.com/admanager/answer/1733124). |

> [!NOTE]
> **Note:** The BigQuery Data Transfer Service supports the following delimiters for Google Ad Manager DT files: Tab ( \\t ), Pipe ( \| ), Caret ( \^ ), and Comma ( , ).

## Data ingestion from Google Ad Manager transfers

When you transfer data from Google Ad Manager into BigQuery, the data is loaded into BigQuery tables that are partitioned by date. The table partition that the data is loaded into corresponds to the date from the data source. If you schedule multiple transfers for the same date, BigQuery Data Transfer Service overwrites the partition for that specific date with the latest data. Multiple transfers in the same day or running backfills don't result in duplicate data, and partitions for other dates are not affected.

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

### Updates to data transfer (DT) files

Tables that are created from Google Ad Manager data transfer (Google Ad Manager
DT) files can be updated incrementally. Google Ad Manager adds the Google Ad Manager
DT files into the Cloud Storage bucket. A transfer run then
incrementally loads the new Google Ad Manager DT files from the Cloud Storage
bucket into the BigQuery table without reloading files that have
already been transferred to the BigQuery table.

For example, Google Ad Manager adds `file1` into the bucket at 1:00 AM and
`file2` at 2:00 AM. A transfer run begins at 3:30 AM and loads `file1` and
`file2` to BigQuery. Google Ad Manager then adds `file3` at 5:00
AM and `file4` at 6:00 AM. A second transfer run begins at 7:30AM and appends
`file3` and `file4` into BigQuery, instead of overwriting the
table by loading all four files.

### Updates to match tables

Match tables provide a lookup mechanism for the raw values contained within data
transfer files. For a list of match tables, see [Google Ad Manager report transformation](https://docs.cloud.google.com/bigquery/docs/doubleclick-publisher-transformation).
Different match tables are updated with different ingestion methods. The match
tables and their ingestion methods are listed in the following table:

| Ingestion method | Description | Match table |
|---|---|---|
| Incremental update | Incremental updates are appended in every run. For example, a first transfer run of the day loads all data modified before the transfer run, the second transfer run in the same day loads data modified after the previous run and before the current run. | `Company`, `Order`, `Placement`, `LineItem`, `AdUnit` |
| Whole table update | Whole table updates loads the whole table once a day. For example, a first transfer run of the day loads all available data for a table. A second transfer run on the same day skips loading these tables. | `AdCategory`, `AudienceSegmentCategory`, `BandwidthGroup`, `Browser`, `BrowserLanguage`, `DeviceCapability`, `DeviceCategory`, `DeviceManufacturer`, `GeoTarget`, `MobileCarrier`, `MobileDevice`, `MobileDeviceSubmodel`, `OperatingSystem`, `OperatingSystemVersion`, `ThirdPartyCompany`, `TimeZone`, `User`, `ProgrammaticBuyer` |
| Whole table overwrite | The whole table is overwritten with every transfer run. | `AudienceSegment` |

## Before you begin

Before you create a Google Ad Manager data transfer:

- Verify that you have completed all actions required to [enable the BigQuery Data Transfer Service](https://docs.cloud.google.com/bigquery/docs/enable-transfer-service).
- [Create a BigQuery dataset](https://docs.cloud.google.com/bigquery/docs/datasets) to store the Google Ad Manager data.
- **Ensure that your organization has access to Google Ad Manager Data
  Transfer (Google Ad Manager DT) files.** These files are delivered by the
  Google Ad Manager team to a Cloud Storage bucket. To gain access to
  Google Ad Manager DT files, review
  [Ad Manager Data Transfer reports](https://support.google.com/admanager/answer/1733124).
  Additional charges from the Google Ad Manager team might apply.

  After completing this step, you will receive a Cloud Storage
  bucket similar to the following:

  ```
      gdfp-12345678
    
  ```

  The Google Cloud team does **NOT** have the
  ability to generate or grant access to Google Ad Manager DT files on your
  behalf. Contact Google Ad Manager
  [support](https://support.google.com/admanager/answer/3059042?&ref_topic=7519191),
  for access to Google Ad Manager DT files.
- [Enable API access](https://support.google.com/admanager/answer/3088588) to your Google Ad Manager network.
- If you intend to set up data transfer notifications, you must have `pubsub.topics.setIamPolicy` permissions for Pub/Sub. Pub/Sub permissions are not required if you just set up email notifications. For more information, see [BigQuery Data Transfer Service run notifications](https://docs.cloud.google.com/bigquery/docs/transfer-run-notifications).

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

### Required Google Ad Manager roles

Grant read access to the Google Ad Manager DT files stored in
Cloud Storage. Permissions for Google Ad Manager DT files are managed by the
Google Ad Manager team. In addition to the Google Ad Manager DT files, the
person creating the data transfer must be added to the Google Ad Manager network,
with read access to all the entities needed to create the various
[match tables](https://docs.cloud.google.com/bigquery/docs/doubleclick-publisher-transformation)
(line item, order, ad unit, etc.). This can be accomplished by adding
the Ad Manager user who authenticated the data transfer to the [All Entities team](https://support.google.com/admanager/answer/2445815#:%7E:text=All%20entities%C2%A0team,with%20their%20account)
in Ad Manager.

## Set up a Google Ad Manager transfer

Setting up a BigQuery data transfer for Google Ad Manager requires
a:

- **Cloud Storage bucket** : The Cloud Storage bucket URI for your Google Ad
  Manager DT files as described in [Before you begin](https://docs.cloud.google.com/bigquery/docs/doubleclick-publisher-transfer#before_you_begin).
  The bucket name should look like the following:

  ```
  gdfp-12345678
  ```
- **Network Code** : You'll find the Google Ad Manager network code in
  the URL when you are logged into your network. For example, in the URL
  `https://admanager.google.com/2032576#delivery`, `2032576` is your network
  code. For more information, see
  [Get started with Google Ad Manager](https://developers.google.com/doubleclick-publishers/docs/start).

To create a BigQuery Data Transfer Service data transfer for Google Ad Manager:

### Console

1. Go to the Data transfers page in the Google Cloud console.

   [Go to Data transfers](https://console.cloud.google.com/bigquery/transfers)
2. Click **Create a transfer**.

3. On the **Create Transfer** page:

   - In the **Source type** section, for **Source** , choose **Google Ad Manager (formerly DFP)**.


   ![Transfer source](https://docs.cloud.google.com/static/bigquery/images/dfp-transfer-source.png)
   - In the **Transfer config name** section, for **Display name** , enter a name for the data transfer such as `My Transfer`. The transfer name can be any value that lets you identify the transfer if you need to modify it later.


   ![Transfer name](https://docs.cloud.google.com/static/bigquery/images/transfer-name.png)
   - In the **Destination settings** section, for **Dataset**, choose the dataset that you created to store your data.


   ![Transfer dataset](https://docs.cloud.google.com/static/bigquery/images/transfer-dataset.png)
   - In the **Data source details** section:
     - For **Cloud Storage bucket** , enter the name of the Cloud Storage bucket that stores your data transfer files. When you enter the bucket name, don't include `gs://`.
     - For **Network code**, enter your network code.


   ![Google Ad Manager source details](https://docs.cloud.google.com/static/bigquery/images/dfp-source-details-console.png)
   - In the **Service account** menu, select a
     [service account](https://docs.cloud.google.com/iam/docs/service-account-overview) from the service
     accounts associated with your Google Cloud project. You can
     associate a service account with your transfer instead of using your
     user credentials. For more information about using service accounts with
     data transfers, see
     [Use service accounts](https://docs.cloud.google.com/bigquery/docs/use-service-accounts).

     <br />


     If you signed in with a
     [federated identity](https://docs.cloud.google.com/iam/docs/workforce-identity-federation), then a
     service account is required to create a transfer. If you signed in with
     a Google Account, then a service account for the transfer is optional.
     The service account must have the
     [required permissions](https://docs.cloud.google.com/bigquery/docs/doubleclick-publisher-transfer#required_permissions).

   - (Optional) In the **Notification options** section:

     - Click the toggle to enable email notifications. When you enable this option, the transfer administrator receives an email notification when a transfer run fails.
     - Click the toggle to enable Pub/Sub run notifications. For **Select a Cloud Pub/Sub topic** , choose your topic name or click **Create a topic** . This option configures Pub/Sub run [notifications](https://docs.cloud.google.com/bigquery/docs/transfer-run-notifications) for your transfer.
4. Click **Save**.

### bq

Enter the `bq mk` command and supply the transfer creation flag ---
`--transfer_config`. The following flags are also required:

- `--data_source`
- `--target_dataset`
- `--display_name`
- `--params`

Optional flags:

- `--service_account_name` - Specifies a service account to use for Google Ad Manager transfer authentication instead of your user account.

```bash
bq mk --transfer_config \
--project_id=project_id \
--target_dataset=dataset \
--display_name=name \
--params='parameters' \
--data_source=data_source \
--service_account_name=service_account_name
```

Where:

- <var translate="no">project_id</var> is your project ID.
- <var translate="no">dataset</var> is the target dataset for the transfer configuration.
- <var translate="no">name</var> is the display name for the data transfer configuration. The transfer name can be any value that lets you identify the data transfer if you need to modify it later.
- <var translate="no">parameters</var> contains the parameters for the created transfer configuration in JSON format. For example: `--params='{"param":"param_value"}'`. For Google Ad Manager, you must supply the `bucket` and `network_code` parameters.
  - `bucket`: The Cloud Storage bucket that contains your Google Ad Manager DT files.
  - `network_code`: Network code
  - `load_match_tables`: Whether to load match tables. By default set to `True`
- <var translate="no">data_source</var> is the data source --- `dfp_dt` (Google Ad Manager).
- <var translate="no">service_account_name</var> is the service account name used to authenticate your data transfer. The service account should be owned by the same `project_id` used to create the transfer and it should have all of the [required permissions](https://docs.cloud.google.com/bigquery/docs/doubleclick-publisher-transfer#required_permissions).

> [!CAUTION]
> **Caution:** You cannot configure notifications using the command-line tool.

You can also supply the `--project_id` flag to specify a particular
project. If `--project_id` isn't specified, the default project is used.

For example, the following command creates a Google Ad Manager
data transfer named `My Transfer` using network code `12345678`, Cloud Storage
bucket `gdfp-12345678`, and target dataset `mydataset`. The data transfer
is created in the default project:

    bq mk --transfer_config \
    --target_dataset=mydataset \
    --display_name='My Transfer' \
    --params='{"bucket": "gdfp-12345678","network_code": "12345678"}' \
    --data_source=dfp_dt

After running the command, you receive a message like the following:

`[URL omitted] Please copy and paste the above URL into your web browser and
follow the instructions to retrieve an authentication code.`

Follow the instructions and paste the authentication code on the command
line.

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

    // Sample to create a ad manager(formerly DFP) transfer config
    public class CreateAdManagerTransfer {

      public static void main(String[] args) throws IOException {
        // TODO(developer): Replace these variables before running the sample.
        final String projectId = "MY_PROJECT_ID";
        String datasetId = "MY_DATASET_ID";
        String bucket = "gs://cloud-sample-data";
        // the network_code can only be digits with length 1 to 15
        String networkCode = "12345678";
        Map<String, Value> params = new HashMap<>();
        params.put("bucket", https://docs.cloud.google.com/java/docs/reference/protobuf/latest/com.google.protobuf.Value.html.newBuilder().setStringValue(bucket).build());
        params.put("network_code", https://docs.cloud.google.com/java/docs/reference/protobuf/latest/com.google.protobuf.Value.html.newBuilder().setStringValue(networkCode).build());
        https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.TransferConfig.html transferConfig =
            https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.TransferConfig.html.newBuilder()
                .setDestinationDatasetId(datasetId)
                .setDisplayName("Your Ad Manager Config Name")
                .setDataSourceId("dfp_dt")
                .setParams(https://docs.cloud.google.com/java/docs/reference/protobuf/latest/com.google.protobuf.Struct.html.newBuilder().https://docs.cloud.google.com/java/docs/reference/protobuf/latest/com.google.protobuf.Struct.Builder.html#com_google_protobuf_Struct_Builder_putAllFields_java_util_Map_java_lang_String_com_google_protobuf_Value__(params).build())
                .build();
        createAdManagerTransfer(projectId, transferConfig);
      }

      public static void createAdManagerTransfer(String projectId, https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.TransferConfig.html transferConfig)
          throws IOException {
        try (https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.DataTransferServiceClient.html client = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.DataTransferServiceClient.html.create()) {
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.ProjectName.html parent = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.ProjectName.html.of(projectId);
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.CreateTransferConfigRequest.html request =
              https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.CreateTransferConfigRequest.html.newBuilder()
                  .setParent(parent.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.ProjectName.html#com_google_cloud_bigquery_datatransfer_v1_ProjectName_toString__())
                  .setTransferConfig(transferConfig)
                  .build();
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.TransferConfig.html config = client.createTransferConfig(request);
          System.out.println("Ad manager transfer created successfully :" + config.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.TransferConfig.html#com_google_cloud_bigquery_datatransfer_v1_TransferConfig_getName__());
        } catch (https://docs.cloud.google.com/java/docs/reference/gax/latest/com.google.api.gax.rpc.ApiException.html ex) {
          System.out.print("Ad manager transfer was not created." + ex.toString());
        }
      }
    }

> [!WARNING]
> **Warning:** If you change the schema of a report, all files on that day must have the same schema, or the data transfer for the entire day will fail.

## Troubleshoot Google Ad Manager transfer setup

If you are having issues setting up your data transfer, see
[Google Ad Manager transfer issues](https://docs.cloud.google.com/bigquery/docs/transfer-troubleshooting#google_ad_manager_transfer_issues)
in [Troubleshooting transfer configurations](https://docs.cloud.google.com/bigquery/docs/transfer-troubleshooting).

## Query your data

When your data is transferred to BigQuery, the data is
written to ingestion-time partitioned tables. For more information, see
[Introduction to partitioned tables](https://docs.cloud.google.com/bigquery/docs/partitioned-tables).

If you query your tables directly instead of using the auto-generated views, you
must use the `_PARTITIONTIME` pseudocolumn in your query. For more information,
see [Querying partitioned tables](https://docs.cloud.google.com/bigquery/docs/querying-partitioned-tables).

## Sample queries

You can use the following Google Ad Manager sample queries to analyze
your transferred data. You can also use the queries in a visualization tool
such as [Data Studio](https://www.google.com/analytics/data-studio/).
These queries are provided to help you get started on querying your Google Ad
Manager data with BigQuery. For additional questions on
what you can do with these reports, contact your Google Ad Manager
technical representative.

> [!NOTE]
> **Note:** If you query your tables directly instead of using the auto-generated views, you must use the `_PARTITIONTIME` pseudocolumn in your query. For more information, see [Querying partitioned tables](https://docs.cloud.google.com/bigquery/docs/querying-partitioned-tables).

In each of the following queries, replace variables like <var translate="no">dataset</var> with
your values. For example, replace <var translate="no">network_code</var> with your Google Ad
Manager network code.

### Impressions and unique users by city

The following SQL sample query analyzes the number of impressions and unique
users by city over the past 30 days.

```googlesql
# START_DATE = DATE_ADD(CURRENT_DATE(), INTERVAL -31 DAY)
# END_DATE = DATE_ADD(CURRENT_DATE(), INTERVAL -1 DAY)
SELECT
  City,
  _DATA_DATE AS Date,
  count(*) AS imps,
  count(distinct UserId) AS uniq_users
FROM `dataset.NetworkImpressions_network_code`
WHERE
  _DATA_DATE BETWEEN start_date AND end_date
GROUP BY City, Date
```

### Impressions and unique users by line item type

The following SQL sample query analyzes the number of impressions and unique
users by line item type over the past 30 days.

```googlesql
# START_DATE = DATE_ADD(CURRENT_DATE(), INTERVAL -31 DAY)
# END_DATE = DATE_ADD(CURRENT_DATE(), INTERVAL -1 DAY)
SELECT
  MT.LineItemType AS LineItemType,
  DT._DATA_DATE AS Date,
  count(*) AS imps,
  count(distinct UserId) AS uniq_users
FROM `dataset.NetworkImpressions_network_code` AS DT
LEFT JOIN `dataset.MatchTableLineItem_network_code` AS MT
ON
  DT.LineItemId = MT.Id
WHERE
  DT._DATA_DATE BETWEEN start_date AND end_date
GROUP BY LineItemType, Date
ORDER BY Date desc, imps desc
```

### Impressions by ad unit

The following SQL sample query analyzes the number of impressions by ad unit
over the past 30 days.

```googlesql
# START_DATE = DATE_ADD(CURRENT_DATE(), INTERVAL -31 DAY)
# END_DATE = DATE_ADD(CURRENT_DATE(), INTERVAL -1 DAY)
SELECT
  MT.AdUnitCode AS AdUnitCode,
  DT.DATA_DATE AS Date,
  count(*) AS imps
FROM `dataset.NetworkImpressions_network_code` AS DT
LEFT JOIN `dataset.MatchTableAdUnit_network_code` AS MT
ON
  DT.AdUnitId = MT.Id
WHERE
  DT._DATA_DATE BETWEEN start_date AND end_date
GROUP BY AdUnitCode, Date
ORDER BY Date desc, imps desc
```

### Impressions by line item

The following SQL sample query analyzes the number of impressions by line item
over the past 30 days.

```googlesql
# START_DATE = DATE_ADD(CURRENT_DATE(), INTERVAL -31 DAY)
# END_DATE = DATE_ADD(CURRENT_DATE(), INTERVAL -1 DAY)
SELECT
  MT.Name AS LineItemName,
  DT._DATA_DATE AS Date,
  count(*) AS imps
FROM `dataset.NetworkImpressions_network_code` AS DT
LEFT JOIN `dataset.MatchTableLineItem_network_code` AS MT
ON
  DT.LineItemId = MT.Id
WHERE
  DT._DATA_DATE BETWEEN start_date AND end_date
GROUP BY LineItemName, Date
ORDER BY Date desc, imps desc
```