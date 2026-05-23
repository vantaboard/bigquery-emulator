# Load YouTube Channel data into BigQuery

You can load data from YouTube Channel to BigQuery using the
[BigQuery Data Transfer Service](https://docs.cloud.google.com/bigquery/docs/dts-introduction) for YouTube Channel connector. With the
BigQuery Data Transfer Service, you can schedule recurring transfer jobs that
add your latest data from YouTube Channel to
BigQuery.

## Connector overview

The BigQuery Data Transfer Service for the YouTube Channel connector supports the following options for your data transfer.

| Data transfer options | Support |
|---|---|
| Supported reports | The YouTube Channel connector supports the transfer of data from [Channel reports](https://developers.google.com/youtube/reporting/v1/reports/channel_reports). The YouTube Channel connector supports the [June 18, 2018](https://developers.google.com/youtube/reporting/revision_history#june-18,-2018) API version. For information about how YouTube Channel reports are transformed into BigQuery tables and views, see [YouTube Channel report transformation](https://docs.cloud.google.com/bigquery/docs/youtube-channel-transformation). |
| Repeat frequency | The YouTube Channel connector supports daily data transfers. <br /> By default, data transfers are scheduled at the time when the data transfer is created. You can configure the time of data transfer when you [set up your data transfer](https://docs.cloud.google.com/bigquery/docs/youtube-channel-transfer#set_up_a_youtube_channel_transfer). |
| Refresh window | The YouTube Channel connector retrieves YouTube Channel data from up to 1 day at the time the data transfer is run. <br /> For more information, see [Refresh windows](https://docs.cloud.google.com/bigquery/docs/youtube-channel-transfer#refresh). |
| Backfill data availability | [Run a data backfill](https://docs.cloud.google.com/bigquery/docs/working-with-transfers#manually_trigger_a_transfer) to retrieve data outside of your scheduled data transfer. You can retrieve data as far back as the data retention policy on your data source allows. <br /> YouTube reports containing historical data are available for 30 days from the time that they are generated. (Reports that contain non-historical data are available for 60 days.) For more information, see [Historical data](https://developers.google.com/youtube/reporting/v1/reports/#historical-data). |

## Data ingestion from YouTube Channel transfers

When you transfer data from a YouTube Channel into BigQuery, the data is loaded into BigQuery tables that are partitioned by date. The table partition that the data is loaded into corresponds to the date from the data source. If you schedule multiple transfers for the same date, BigQuery Data Transfer Service overwrites the partition for that specific date with the latest data. Multiple transfers in the same day or running backfills don't result in duplicate data, and partitions for other dates are not affected.

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

- The minimum frequency that you can schedule a data transfer for is once every 24 hours. By default, a data transfer starts at the time that you create the transfer. However, you can configure the data transfer start time when you [set up your transfer](https://docs.cloud.google.com/bigquery/docs/youtube-channel-transfer#set_up_a_youtube_channel_transfer).
- The BigQuery Data Transfer Service does not support incremental data transfers during a YouTube Content Owner transfer. When you specify a date for a data transfer, all of the data that is available for that date is transferred.
- You cannot create a YouTube Channel data transfer if you are signed in as a federated identity. You can only create a YouTube Channel transfer while signed in using a Google Account.

## Before you begin

Before you create a YouTube Channel data transfer:

- Verify that you have completed all actions required to [enable the BigQuery Data Transfer Service](https://docs.cloud.google.com/bigquery/docs/enable-transfer-service).
- [Create a BigQuery dataset](https://docs.cloud.google.com/bigquery/docs/datasets) to store the YouTube data.

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

### Required YouTube Channel roles

You must have ownership of the YouTube Channel.

## Set up a YouTube Channel transfer

Setting up a YouTube Channel data transfer requires a:

- **Table Suffix** : A user-friendly name for the channel provided by you when you set up the data transfer. The suffix is appended to the job ID to create the table name, for example <var translate="no">reportTypeId_suffix</var>. The suffix is used to prevent separate transfers from writing to the same tables. The table suffix must be unique across all transfers that load data into the same dataset, and the suffix should be short to minimize the length of the resulting table name.

If you use the
[YouTube Reporting API](https://developers.google.com/youtube/reporting/v1/reference/rest/)
and have existing reporting jobs, the BigQuery Data Transfer Service loads your report
data. If you don't have existing reporting jobs, setting up the transfer
automatically enables YouTube reporting jobs.

To create a YouTube Channel data transfer:

### Console

1. Go to the Data transfers page in the Google Cloud console.

   [Go to Data transfers](https://console.cloud.google.com/bigquery/transfers)
2. Click **Create transfer**.

3. On the **Create Transfer** page:

   - In the **Source type** section, for **Source** , choose
     **YouTube Channel**.


     ![Transfer source](https://docs.cloud.google.com/static/bigquery/images/youtube-channel-transfer-source.png)
   - In the **Transfer config name** section, for **Display name** , enter a
     name for the data transfer such as `My Transfer`. The transfer name can
     be any value that lets you identify the transfer if you need to modify
     it later.


     ![Transfer name](https://docs.cloud.google.com/static/bigquery/images/transfer-name.png)
   - In the **Schedule options** section:

     - For **Repeat frequency** , choose an option for how often to run the data transfer. If you select **Days**, provide a valid time in UTC.
     - If applicable, select either **Start now** or **Start at set time**, and provide a start date and run time.
   - In the **Destination settings** section, for **Destination dataset**,
     choose the dataset you created to store your data.


     ![Transfer dataset](https://docs.cloud.google.com/static/bigquery/images/transfer-dataset.png)
   - In the **Data source details** section:

     - For **Table suffix** , enter a suffix such as `MT`.
     - Check the box for **Configure jobs** to allow BigQuery to manage YouTube reporting jobs for you. If there are YouTube reports that don't yet exist for your account, new reporting jobs are created to enable them.


     ![YouTube Channel source details](https://docs.cloud.google.com/static/bigquery/images/youtube-channel-source-details-console.png)
   - (Optional) In the **Notification options** section:

     - Click the toggle to enable email notifications. When you enable this option, the transfer administrator receives an email notification when a transfer run fails.
     - For **Select a Pub/Sub topic** , choose your [topic](https://docs.cloud.google.com/pubsub/docs/overview#types) name or click **Create a topic** . This option configures Pub/Sub run [notifications](https://docs.cloud.google.com/bigquery/docs/transfer-run-notifications) for your data transfer.
4. Click **Save**.

### bq

Enter the `bq mk` command and supply the transfer creation flag ---
`--transfer_config`. The following flags are also required:

- `--data_source`
- `--target_dataset`
- `--display_name`
- `--params`

```bash
bq mk \
--transfer_config \
--project_id=project_id \
--target_dataset=dataset \
--display_name=name \
--params='parameters' \
--data_source=data_source
```

Where:

- <var translate="no">project_id</var> is your project ID.
- <var translate="no">dataset</var> is the target dataset for the transfer configuration.
- <var translate="no">name</var> is the display name for the transfer configuration. The data transfer name can be any value that lets you identify the transfer if you need to modify it later.
- <var translate="no">parameters</var> contains the parameters for the created transfer configuration in JSON format. For example: `--params='{"param":"param_value"}'`. For YouTube Channel data transfers, you must supply the `table_suffix` parameter. You may optionally set the `configure_jobs` parameter to `true` to allow the BigQuery Data Transfer Service to manage YouTube reporting jobs for you. If there are YouTube reports that don't exist for your channel, new reporting jobs are created to enable them.
- <var translate="no">data_source</var> is the data source --- `youtube_channel`.

> [!CAUTION]
> **Caution:** You cannot configure notifications using the command-line tool.

You can also supply the `--project_id` flag to specify a particular
project. If `--project_id` isn't specified, the default project is used.

For example, the following command creates a YouTube Channel data transfer named
`My Transfer` using table suffix `MT`, and target dataset `mydataset`. The
data transfer is created in the default project:

    bq mk \
    --transfer_config \
    --target_dataset=mydataset \
    --display_name='My Transfer' \
    --params='{"table_suffix":"MT","configure_jobs":"true"}' \
    --data_source=youtube_channel

> [!CAUTION]
> **Caution:** When you create a YouTube Channel transfer using the command-line tool, the transfer configuration is set up using the [default value](https://docs.cloud.google.com/bigquery/docs/youtube-channel-transfer#connector_overview) for **Schedule**.

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

    // Sample to create youtube channel transfer config.
    public class CreateYoutubeChannelTransfer {

      public static void main(String[] args) throws IOException {
        // TODO(developer): Replace these variables before running the sample.
        final String projectId = "MY_PROJECT_ID";
        String datasetId = "MY_DATASET_ID";
        String tableSuffix = "_test";
        Map<String, Value> params = new HashMap<>();
        params.put("table_suffix", https://docs.cloud.google.com/java/docs/reference/protobuf/latest/com.google.protobuf.Value.html.newBuilder().setStringValue(tableSuffix).build());
        https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.TransferConfig.html transferConfig =
            https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.TransferConfig.html.newBuilder()
                .setDestinationDatasetId(datasetId)
                .setDisplayName("Your Youtube Channel Config Name")
                .setDataSourceId("youtube_channel")
                .setParams(https://docs.cloud.google.com/java/docs/reference/protobuf/latest/com.google.protobuf.Struct.html.newBuilder().https://docs.cloud.google.com/java/docs/reference/protobuf/latest/com.google.protobuf.Struct.Builder.html#com_google_protobuf_Struct_Builder_putAllFields_java_util_Map_java_lang_String_com_google_protobuf_Value__(params).build())
                .build();
        createYoutubeChannelTransfer(projectId, transferConfig);
      }

      public static void createYoutubeChannelTransfer(String projectId, https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.TransferConfig.html transferConfig)
          throws IOException {
        try (https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.DataTransferServiceClient.html client = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.DataTransferServiceClient.html.create()) {
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.ProjectName.html parent = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.ProjectName.html.of(projectId);
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.CreateTransferConfigRequest.html request =
              https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.CreateTransferConfigRequest.html.newBuilder()
                  .setParent(parent.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.ProjectName.html#com_google_cloud_bigquery_datatransfer_v1_ProjectName_toString__())
                  .setTransferConfig(transferConfig)
                  .build();
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.TransferConfig.html config = client.createTransferConfig(request);
          System.out.println("Youtube channel transfer created successfully :" + config.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.TransferConfig.html#com_google_cloud_bigquery_datatransfer_v1_TransferConfig_getName__());
        } catch (https://docs.cloud.google.com/java/docs/reference/gax/latest/com.google.api.gax.rpc.ApiException.html ex) {
          System.out.print("Youtube channel transfer was not created." + ex.toString());
        }
      }
    }

> [!NOTE]
> **Note:** If you are setting up YouTube reporting jobs for the first time, you will experience a delay of up to 48 hours before your first reports are ready. For more information, see [Create a reporting job](https://developers.google.com/youtube/reporting/v1/reports/#step-3-create-a-reporting-job) in the YouTube Reporting API documentation.

## Query your data

When your data is transferred to BigQuery, the data is
written to ingestion-time partitioned tables. For more information, see
[Introduction to partitioned tables](https://docs.cloud.google.com/bigquery/docs/partitioned-tables).

If you query your tables directly instead of using the auto-generated views, you
must use the `_PARTITIONTIME` pseudocolumn in your query. For more information,
see [Querying partitioned tables](https://docs.cloud.google.com/bigquery/docs/querying-partitioned-tables).

## Troubleshoot YouTube Channel transfer setup

If you are having issues setting up your data transfer, see
[YouTube transfer issues](https://docs.cloud.google.com/bigquery/docs/transfer-troubleshooting#youtube_transfer_issues)
in [Troubleshooting transfer configurations](https://docs.cloud.google.com/bigquery/docs/transfer-troubleshooting).