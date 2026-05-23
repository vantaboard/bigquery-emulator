# Load YouTube Content Owner data into BigQuery

You can load data from YouTube Content Owner to BigQuery using the
[BigQuery Data Transfer Service](https://docs.cloud.google.com/bigquery/docs/dts-introduction) for YouTube Content Owner connector. With the
BigQuery Data Transfer Service, you can schedule recurring transfer jobs that
add your latest data from YouTube Content Owner to
BigQuery.

## Connector overview

The BigQuery Data Transfer Service for the YouTube Content Owner connector supports the following options for your data transfer.

| Data transfer options | Support |
|---|---|
| Supported reports | The YouTube Content Owner connector supports the transfer of data from the following reports: - [Content Owner Reports](https://developers.google.com/youtube/reporting/v1/reports/content_owner_reports) - System-Managed Reports: - [Overview](https://developers.google.com/youtube/reporting/v1/reports/system_managed/reports) - [Reports](https://developers.google.com/youtube/reporting/v1/reports/system_managed/reports) - [Fields](https://developers.google.com/youtube/reporting/v1/reports/system_managed/fields) The YouTube Content Owner connector supports the [June 18, 2018](https://developers.google.com/youtube/reporting/revision_history#june-18,-2018) API version. For information about how YouTube Content Owner reports are transformed into BigQuery tables and views, see [YouTube Content Owner report transformation](https://docs.cloud.google.com/bigquery/docs/youtube-content-owner-transformation). |
| Repeat frequency | The YouTube Content Owner connector supports daily data transfers. <br /> By default, data transfers are scheduled at the time when the data transfer is created. You can configure the time of data transfer when you [set up your data transfer](https://docs.cloud.google.com/bigquery/docs/youtube-content-owner-transfer#set_up_a_youtube_content_owner_transfer). |
| Refresh window | The YouTube Content Owner connector retrieves YouTube Content Owner data from up to 1 day at the time the data transfer is run. <br /> For more information, see [Refresh windows](https://docs.cloud.google.com/bigquery/docs/youtube-content-owner-transfer#refresh). |
| Backfill data availability | [Run a data backfill](https://docs.cloud.google.com/bigquery/docs/working-with-transfers#manually_trigger_a_transfer) to retrieve data outside of your scheduled data transfer. You can retrieve data as far back as the data retention policy on your data source allows. <br /> YouTube reports containing historical data are available for 30 days from the time that they are generated. (Reports that contain non-historical data are available for 60 days.) For more information, see [Historical data](https://developers.google.com/youtube/reporting/v1/reports/#historical-data). |

## Data ingestion from YouTube Content Owner transfers

When you transfer data from YouTube Content Owner reports into BigQuery, the data is loaded into BigQuery tables that are partitioned by date. The table partition that the data is loaded into corresponds to the date from the data source. If you schedule multiple transfers for the same date, BigQuery Data Transfer Service overwrites the partition for that specific date with the latest data. Multiple transfers in the same day or running backfills don't result in duplicate data, and partitions for other dates are not affected.

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

- The minimum frequency that you can schedule a data transfer for is once every 24 hours. By default, a data transfer starts at the time that you create the data transfer. However, you can configure the transfer start time when you [set up your transfer](https://docs.cloud.google.com/bigquery/docs/youtube-content-owner-transfer#set_up_a_youtube_content_owner_transfer).
- The BigQuery Data Transfer Service does not support incremental data transfers during a YouTube Content Owner transfer. When you specify a date for a data transfer, all of the data that is available for that date is transferred.

## Before you begin

Before you create a YouTube Content Owner data transfer:

- Verify that you have completed all actions required to [enable the BigQuery Data Transfer Service](https://docs.cloud.google.com/bigquery/docs/enable-transfer-service).
- [Create a BigQuery dataset](https://docs.cloud.google.com/bigquery/docs/datasets) to store YouTube data.
- Verify that you have a [YouTube Content Owner](https://support.google.com/youtube/answer/6301188) account. A YouTube Content Owner is not the same as a YouTube channel. Typically, you only have a YouTube Content Owner account if you manage many different channels.
- If you intend to set up transfer run notifications for Pub/Sub, you must have `pubsub.topics.setIamPolicy` permissions. Pub/Sub permissions are not required if you just set up email notifications. For more information, see [BigQuery Data Transfer Service run notifications](https://docs.cloud.google.com/bigquery/docs/transfer-run-notifications).

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

### Required YouTube roles

- YouTube Content Manager or YouTube Content Owner.

  A Content Manager is granted rights to administer YouTube content for a
  Content Owner. A Content Owner is an umbrella account that owns one or more
  YouTube channels and the videos on those channels.
- `Hide revenue data` is unchecked in YouTube Content Owner report settings.

  For revenue-related reports to transfer, the YouTube reports permission
  setting `Hide revenue data` should be unchecked for the user creating the
  transfer.

  ![youtube-content-owner-reports-uncheck-hide-revenue](https://docs.cloud.google.com/static/bigquery/images/youtube-content-owner-reports-uncheck-hide-revenue.png)

## Set up a YouTube Content Owner transfer

Setting up a YouTube Content Owner data transfer requires a:

- **Content Owner ID** : Provided by YouTube. When you sign in to YouTube as a Content Owner or Manager, your ID appears in the URL after `o=`. For example, if the URL is `https://studio.youtube.com/owner/AbCDE_8FghIjK?o=AbCDE_8FghIjK`, the Content Owner ID is `AbCDE_8FghIjK`. To select a different Content Manager account, see [Sign in to a Content Manager account](https://support.google.com/youtube/answer/6301172) or [YouTube Channel Switcher](https://www.youtube.com/channel_switcher). For more information on creating and managing your Content Manager account, see [Configure Content Manager account settings](https://support.google.com/youtube/topic/6032636).
- **Table Suffix** : A user-friendly name for the channel provided by you when you set up the transfer. The suffix is appended to the job ID to create the table name, for example <var translate="no">reportTypeId_suffix</var>. The suffix is used to prevent separate data transfers from writing to the same tables. The table suffix must be unique across all transfers that load data into the same dataset, and the suffix should be short to minimize the length of the resulting table name.

If you use the
[YouTube Reporting API](https://developers.google.com/youtube/reporting/v1/reference/rest/)
and have existing reporting jobs, the BigQuery Data Transfer Service loads your report
data. If you don't have existing reporting jobs, setting up the data transfer
automatically enables YouTube reporting jobs.

To set up a YouTube Content Owner data transfer:

### Console

1. Go to the BigQuery page in the Google Cloud console.
   Ensure that you are signed in to the account as either the Content Owner
   or Content Manager.

   [Go to the BigQuery page](https://console.cloud.google.com/bigquery)
2. Click **Transfers**.

3. Click **Create Transfer**.

4. On the **Create Transfer** page:

   - In the **Source type** section, for **Source** , choose
     **YouTube Content Owner**.


     ![Transfer source](https://docs.cloud.google.com/static/bigquery/images/youtube-content-owner-transfer-source.png)
   - In the **Transfer config name** section, for **Display name** , enter a
     name for the data transfer such as `My Transfer`. The transfer name can
     be any value that lets you identify the transfer if you need to modify
     it later.


     ![Transfer name](https://docs.cloud.google.com/static/bigquery/images/transfer-name.png)
   - In the **Schedule options** section:

     - For **Repeat frequency** , choose an option for how often to run the data transfer. If you select **Days**, provide a valid time in UTC.
     - If applicable, select either **Start now** or **Start at set time**, and provide a start date and run time.
   - In the **Destination settings** section, for **Destination dataset**,
     choose the dataset that you created to store your data.


     ![Transfer dataset](https://docs.cloud.google.com/static/bigquery/images/transfer-dataset.png)
   - In the **Data source details** section:

     - For **Content owner ID**, enter your Content Owner ID.
     - For **Table suffix** , enter a suffix, such as `MT`.


     ![YouTube Content Owner source details](https://docs.cloud.google.com/static/bigquery/images/youtube-content-owner-source-details-console.png)
   - In the **Service Account** menu, select a
     [service account](https://docs.cloud.google.com/iam/docs/service-account-overview) from the service
     accounts that are associated with your Google Cloud project. You
     can associate a service account with your data transfer instead of using
     your user credentials. For more information about using service accounts
     with data transfers, see
     [Use service accounts](https://docs.cloud.google.com/bigquery/docs/use-service-accounts).

     If you signed in with a
     [federated identity](https://docs.cloud.google.com/iam/docs/workforce-identity-federation), then a
     service account is required to create a data transfer. If you signed
     in with a [Google Account](https://docs.cloud.google.com/iam/docs/principals-overview#google-account), then a
     service account for the data transfer is optional. The service account
     must have the
     [required permissions](https://docs.cloud.google.com/bigquery/docs/youtube-content-owner-transfer#required_permissions).
   - (Optional) In the **Notification options** section:

     - Click the toggle to enable email notifications. When you enable this option, the transfer administrator receives an email notification when a data transfer run fails.
     - For **Select a Pub/Sub topic** , choose your [topic](https://docs.cloud.google.com/pubsub/docs/overview#types) name or click **Create a topic** . This option configures Pub/Sub run [notifications](https://docs.cloud.google.com/bigquery/docs/transfer-run-notifications) for your transfer.
5. Click **Save**.

6. If this is your first time signing into the account, select an account,
   and then click **Allow**. Select the same account where you are the
   Content Owner or Content Manager.

### bq

Enter the `bq mk` command and supply the transfer creation flag ---
`--transfer_config`. The following flags are also required:

- `--data_source`
- `--target_dataset`
- `--display_name`
- `--params`

Optional flags:

- `--service_account_name` - Specifies a service account to use for Content Owner transfer authentication instead of your user account.

```bash
bq mk \
--transfer_config \
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
- <var translate="no">name</var> is the display name for the transfer configuration. The data transfer name can be any value that lets you identify the transfer if you need to modify it later.
- <var translate="no">parameters</var> contains the parameters for the created transfer configuration in JSON format. For example: `--params='{"param":"param_value"}'`. For YouTube Content Owner data transfers, you must supply the `content_owner_id` and `table_suffix` parameters. You may optionally set the `configure_jobs` parameter to `true` to allow the BigQuery Data Transfer Service to manage YouTube reporting jobs for you. If there are YouTube reports that don't exist for your account, new reporting jobs are created to enable them.
- <var translate="no">data_source</var> is the data source --- `youtube_content_owner`.
- <var translate="no">service_account_name</var> is the service account name used to authenticate your data transfer. The service account should be owned by the same `project_id` used to create the transfer and it should have all of the [required permissions](https://docs.cloud.google.com/bigquery/docs/youtube-content-owner-transfer#required_permissions).

> [!CAUTION]
> **Caution:** You cannot configure notifications using the command-line tool.

You can also supply the `--project_id` flag to specify a particular
project. If `--project_id` isn't specified, the default project is used.

For example, the following command creates a YouTube Content Owner data transfer
named `My Transfer` using content owner ID `AbCDE_8FghIjK`, table suffix
`MT`, and target dataset `mydataset`. The data transfer is created in the default
project:

    bq mk \
    --transfer_config \
    --target_dataset=mydataset \
    --display_name='My Transfer' \
    --params='{"content_owner_id":"abCDE_8FghIjK","table_suffix":"MT","configure_jobs":"true"}' \
    --data_source=youtube_content_owner

> [!CAUTION]
> **Caution:** When you create a YouTube Content Owner data transfer using the command-line tool, the transfer configuration is set up using the [default value](https://docs.cloud.google.com/bigquery/docs/youtube-content-owner-transfer#connector_overview) for **Schedule**.

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

    // Sample to create youtube content owner channel transfer config
    public class CreateYoutubeContentOwnerTransfer {

      public static void main(String[] args) throws IOException {
        // TODO(developer): Replace these variables before running the sample.
        final String projectId = "MY_PROJECT_ID";
        String datasetId = "MY_DATASET_ID";
        String contentOwnerId = "MY_CONTENT_OWNER_ID";
        String tableSuffix = "_test";
        Map<String, Value> params = new HashMap<>();
        params.put("content_owner_id", https://docs.cloud.google.com/java/docs/reference/protobuf/latest/com.google.protobuf.Value.html.newBuilder().setStringValue(contentOwnerId).build());
        params.put("table_suffix", https://docs.cloud.google.com/java/docs/reference/protobuf/latest/com.google.protobuf.Value.html.newBuilder().setStringValue(tableSuffix).build());
        https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.TransferConfig.html transferConfig =
            https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.TransferConfig.html.newBuilder()
                .setDestinationDatasetId(datasetId)
                .setDisplayName("Your Youtube Owner Channel Config Name")
                .setDataSourceId("youtube_content_owner")
                .setParams(https://docs.cloud.google.com/java/docs/reference/protobuf/latest/com.google.protobuf.Struct.html.newBuilder().https://docs.cloud.google.com/java/docs/reference/protobuf/latest/com.google.protobuf.Struct.Builder.html#com_google_protobuf_Struct_Builder_putAllFields_java_util_Map_java_lang_String_com_google_protobuf_Value__(params).build())
                .build();
        createYoutubeContentOwnerTransfer(projectId, transferConfig);
      }

      public static void createYoutubeContentOwnerTransfer(
          String projectId, https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.TransferConfig.html transferConfig) throws IOException {
        try (https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.DataTransferServiceClient.html client = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.DataTransferServiceClient.html.create()) {
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.ProjectName.html parent = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.ProjectName.html.of(projectId);
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.CreateTransferConfigRequest.html request =
              https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.CreateTransferConfigRequest.html.newBuilder()
                  .setParent(parent.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.ProjectName.html#com_google_cloud_bigquery_datatransfer_v1_ProjectName_toString__())
                  .setTransferConfig(transferConfig)
                  .build();
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.TransferConfig.html config = client.createTransferConfig(request);
          System.out.println(
              "Youtube content owner channel transfer created successfully :" + config.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.TransferConfig.html#com_google_cloud_bigquery_datatransfer_v1_TransferConfig_getName__());
        } catch (https://docs.cloud.google.com/java/docs/reference/gax/latest/com.google.api.gax.rpc.ApiException.html ex) {
          System.out.print("Youtube content owner channel transfer was not created." + ex.toString());
        }
      }
    }

> [!NOTE]
> **Note:** If you are setting up YouTube reporting jobs for the first time, you will experience a delay of up to 48 hours before your first reports are ready. For more information, see [Create a reporting job](https://developers.google.com/youtube/reporting/v1/reports/#step-3-create-a-reporting-job) in the YouTube Reporting API documentation.

## Query your data

When your data is transferred to BigQuery, the data is
written to ingestion-time partitioned tables. For more information, see
[Partitioned tables](https://docs.cloud.google.com/bigquery/docs/partitioned-tables).

If you query your tables directly instead of using the auto-generated views, you
must use the `_PARTITIONTIME` pseudocolumn in your query. For more information,
see [Querying partitioned tables](https://docs.cloud.google.com/bigquery/docs/querying-partitioned-tables).

## Troubleshoot YouTube Content Owner transfer setup

If you are having issues setting up your data transfer, see
[YouTube transfer issues](https://docs.cloud.google.com/bigquery/docs/transfer-troubleshooting#youtube_transfer_issues)
in [Troubleshooting transfer configurations](https://docs.cloud.google.com/bigquery/docs/transfer-troubleshooting).