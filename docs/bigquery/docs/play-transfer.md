# Load Google Play data into BigQuery

You can load data from Google Play to BigQuery using the
[BigQuery Data Transfer Service](https://docs.cloud.google.com/bigquery/docs/dts-introduction) for Google Play connector. With the
BigQuery Data Transfer Service, you can schedule recurring transfer jobs that
add your latest data from Google Play to
BigQuery.

## Connector overview

The BigQuery Data Transfer Service for the Google Play connector supports the following options for your data transfer.

| Data transfer options | Support |
|---|---|
| Supported reports | - Detailed reports: - [Reviews](https://support.google.com/googleplay/android-developer/answer/6135870#reviews) - [Financial reports](https://support.google.com/googleplay/android-developer/answer/6135870#financial) - Aggregated reports: - [Statistics](https://support.google.com/googleplay/android-developer/answer/6135870#statistics) - [User acquisition](https://support.google.com/googleplay/android-developer/answer/6135870#acquisition) For information about how Google Play reports are transformed into BigQuery tables and views, see [Google Play report transformation](https://docs.cloud.google.com/bigquery/docs/play-transformation). |
| Repeat frequency | The Google Play connector supports daily data transfers. <br /> By default, data transfers are scheduled at the time when the data transfer is created. You can configure the time of data transfer when you [set up your data transfer](https://docs.cloud.google.com/bigquery/docs/play-transfer#setup-transfer). |
| Refresh window | The Google Play connector retrieves Google Play data from up to 7 days at the time the data transfer is run. <br /> For more information, see [Refresh windows](https://docs.cloud.google.com/bigquery/docs/play-transfer#refresh). |
| Backfill data availability | [Run a data backfill](https://docs.cloud.google.com/bigquery/docs/working-with-transfers#manually_trigger_a_transfer) to retrieve data outside of your scheduled data transfer. You can retrieve data as far back as the data retention policy on your data source allows. |

## Data ingestion from Google Play transfers

When you transfer data from Google Play into BigQuery, the data is loaded into BigQuery tables that are partitioned by date. The table partition that the data is loaded into corresponds to the date from the data source. If you schedule multiple transfers for the same date, BigQuery Data Transfer Service overwrites the partition for that specific date with the latest data. Multiple transfers in the same day or running backfills don't result in duplicate data, and partitions for other dates are not affected.

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

- The minimum frequency that you can schedule a data transfer for is once every 24 hours. By default, a transfer starts at the time that you create the transfer. However, you can configure the transfer start time when you [set up your transfer](https://docs.cloud.google.com/bigquery/docs/play-transfer#setup-transfer).
- The BigQuery Data Transfer Service does not support incremental data transfers during a Google Play transfer. When you specify a date for a data transfer, all of the data that is available for that date is transferred.

## Before you begin

Before you create a Google Play data transfer:

- Verify that you have completed all actions required to [enable the BigQuery Data Transfer Service](https://docs.cloud.google.com/bigquery/docs/enable-transfer-service).
- [Create a BigQuery dataset](https://docs.cloud.google.com/bigquery/docs/datasets) to store the Google Play data.
- Find your Cloud Storage bucket:
  1. In the [Google Play console](https://play.google.com/apps/publish/), click **Download reports** and select **Reviews** , **Statistics** , or **Financial**.
  2. To copy the ID for your Cloud Storage bucket, click **Copy Cloud Storage URI** . Your bucket ID begins with `gs://`. For example, for the reviews report, your ID is similar to the following:

     ```
     gs://pubsite_prod_rev_01234567890987654321/reviews
     ```
  3. For the Google Play data transfer, you need to copy only the unique ID that comes between `gs://` and `/reviews`:

     ```
     pubsite_prod_rev_01234567890987654321
     ```
  4. If you intend to set up transfer run notifications for Pub/Sub, you must have `pubsub.topics.setIamPolicy` permissions. Pub/Sub permissions are not required if you just set up email notifications. For more information, see [BigQuery Data Transfer Service run notifications](https://docs.cloud.google.com/bigquery/docs/transfer-run-notifications).

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

### Required Google Play roles

Ensure that you have the following permissions in Google Play:

- You must have reporting access in the
  [Google Play console](https://play.google.com/apps/publish/).

  The Google Cloud team does **NOT** have the
  ability to generate or grant access to Google Play files on your behalf. See
  [Contact Google Play support](https://support.google.com/googleplay/answer/9789798?&ref_topic=3364260&visit_id=636444821343154346-869320595&rd=1)
  for help accessing Google Play files.

## Set up a Google Play transfer

Setting up a Google Play data transfer requires a:

- **Cloud Storage bucket** . Steps for locating your Cloud Storage bucket are described in [Before you begin](https://docs.cloud.google.com/bigquery/docs/play-transfer#before_you_begin). Your Cloud Storage bucket begins with `pubsite_prod_rev`. For example: `pubsite_prod_rev_01234567890987654321`.
- **Table suffix**: A user-friendly name for all data sources loading into the same dataset. The suffix is used to prevent separate transfers from writing to the same tables. The table suffix must be unique across all transfers that load data into the same dataset, and the suffix should be short to minimize the length of the resulting table name.

To set up a Google Play data transfer:

### Console

1. Go to the Data transfers page in the Google Cloud console.

   [Go to Data transfers](https://console.cloud.google.com/bigquery/transfers)
2. Click **Create transfer**.

3. On the **Create Transfer** page:

   - In the **Source type** section, for **Source** , choose **Google Play**.


     ![Transfer source](https://docs.cloud.google.com/static/bigquery/images/play-transfer-source.png)
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

     - For **Cloud Storage bucket**, enter the ID for your Cloud Storage bucket.
     - For **Table suffix** , enter a suffix such as `MT` (for `My Transfer`).


     ![Google Play source details](https://docs.cloud.google.com/static/bigquery/images/play-source-details.png)
   - In the **Service Account** menu, select a
     [service account](https://docs.cloud.google.com/iam/docs/service-account-overview) from the service
     accounts that are associated with your Google Cloud project. You
     can associate a service account with your data transfer instead of using
     your user credentials. For more information about using service accounts
     with data transfers, see
     [Use service accounts](https://docs.cloud.google.com/bigquery/docs/use-service-accounts).

     - If you signed in with a [federated identity](https://docs.cloud.google.com/iam/docs/workforce-identity-federation), then a service account is required to create a data transfer. If you signed in with a [Google Account](https://docs.cloud.google.com/iam/docs/principals-overview#google-account), then a service account for the transfer is optional.
     - The service account must have the [required permissions](https://docs.cloud.google.com/bigquery/docs/play-transfer#required_permissions).
   - (Optional) In the **Notification options** section:

     - Click the toggle to enable email notifications. When you enable this option, the transfer administrator receives an email notification when a transfer run fails.
     - For **Select a Pub/Sub topic** , choose your [topic](https://docs.cloud.google.com/pubsub/docs/overview#types) name or click **Create a topic** . This option configures Pub/Sub run [notifications](https://docs.cloud.google.com/bigquery/docs/transfer-run-notifications) for your transfer.
4. Click **Save**.

### bq

Enter the `bq mk` command and supply the transfer creation flag ---
`--transfer_config`. The following flags are also required:

- `--target_dataset`
- `--display_name`
- `--params`
- `--data_source`

```bash
bq mk \
--transfer_config \
--project_id=project_id \
--target_dataset=dataset \
--display_name=name \
--params='parameters' \
--data_source=data_source
--service_account_name=service_account_name
```

Where:

- <var translate="no">project_id</var> is your project ID. If `--project_id` isn't specified, the default project is used.
- <var translate="no">dataset</var> is the target dataset for the transfer configuration.
- <var translate="no">name</var> is the display name for the transfer configuration. The data transfer name can be any value that lets you identify the transfer if you need to modify it later.
- <var translate="no">parameters</var> contains the parameters for the created transfer configuration in JSON format. For example: `--params='{"param":"param_value"}'`. For Google Play, you must supply the `bucket` and `table_suffix`, parameters. `bucket` is the Cloud Storage bucket that contains your Play report files.
- <var translate="no">data_source</var> is the data source: `play`.
- <var translate="no">service_account_name</var> is the service account name used to authenticate your data transfer. The service account should be owned by the same `project_id` used to create the transfer and it should have all of the [required permissions](https://docs.cloud.google.com/bigquery/docs/play-transfer#required_permissions).

> [!CAUTION]
> **Caution:** You cannot configure notifications using the command-line tool.

For example, the following command creates a Google Play data transfer named `My
Transfer` using Cloud Storage bucket `pubsite_prod_rev_01234567890987654321`
and target dataset `mydataset`. The data transfer is created in the default
project:

    bq mk \
    --transfer_config \
    --target_dataset=mydataset \
    --display_name='My Transfer' \
    --params='{"bucket":"pubsite_prod_rev_01234567890987654321","table_suffix":"MT"}' \
    --data_source=play

The first time you run the command, you will receive a message like the
following:

`[URL omitted] Please copy and paste the above URL into your web browser and
follow the instructions to retrieve an authentication code.`

Follow the instructions in the message and paste the authentication code on
the command line.

> [!CAUTION]
> **Caution:** When you create a Google Play data transfer using the command-line tool, the transfer configuration is set up using the default value for **Schedule** (every 24 hours).

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

    // Sample to create a play transfer config.
    public class CreatePlayTransfer {

      public static void main(String[] args) throws IOException {
        // TODO(developer): Replace these variables before running the sample.
        final String projectId = "MY_PROJECT_ID";
        String datasetId = "MY_DATASET_ID";
        String bucket = "gs://cloud-sample-data";
        String tableSuffix = "_test";
        Map<String, Value> params = new HashMap<>();
        params.put("bucket", https://docs.cloud.google.com/java/docs/reference/protobuf/latest/com.google.protobuf.Value.html.newBuilder().setStringValue(bucket).build());
        params.put("table_suffix", https://docs.cloud.google.com/java/docs/reference/protobuf/latest/com.google.protobuf.Value.html.newBuilder().setStringValue(tableSuffix).build());
        https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.TransferConfig.html transferConfig =
            https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.TransferConfig.html.newBuilder()
                .setDestinationDatasetId(datasetId)
                .setDisplayName("Your Play Config Name")
                .setDataSourceId("play")
                .setParams(https://docs.cloud.google.com/java/docs/reference/protobuf/latest/com.google.protobuf.Struct.html.newBuilder().https://docs.cloud.google.com/java/docs/reference/protobuf/latest/com.google.protobuf.Struct.Builder.html#com_google_protobuf_Struct_Builder_putAllFields_java_util_Map_java_lang_String_com_google_protobuf_Value__(params).build())
                .build();
        createPlayTransfer(projectId, transferConfig);
      }

      public static void createPlayTransfer(String projectId, https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.TransferConfig.html transferConfig)
          throws IOException {
        try (https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.DataTransferServiceClient.html client = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.DataTransferServiceClient.html.create()) {
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.ProjectName.html parent = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.ProjectName.html.of(projectId);
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.CreateTransferConfigRequest.html request =
              https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.CreateTransferConfigRequest.html.newBuilder()
                  .setParent(parent.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.ProjectName.html#com_google_cloud_bigquery_datatransfer_v1_ProjectName_toString__())
                  .setTransferConfig(transferConfig)
                  .build();
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.TransferConfig.html config = client.createTransferConfig(request);
          System.out.println("play transfer created successfully :" + config.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.TransferConfig.html#com_google_cloud_bigquery_datatransfer_v1_TransferConfig_getName__());
        } catch (https://docs.cloud.google.com/java/docs/reference/gax/latest/com.google.api.gax.rpc.ApiException.html ex) {
          System.out.print("play transfer was not created." + ex.toString());
        }
      }
    }

> [!WARNING]
> **Warning:** If you change the schema of a report, all files on that day must have the same schema, or the data transfer for the entire day will fail.

## Troubleshoot Google Play transfer set up

If you are having issues setting up your data transfer, see
[Troubleshooting BigQuery Data Transfer Service transfer setup](https://docs.cloud.google.com/bigquery/docs/transfer-troubleshooting).

## Query your data

When your data is transferred to BigQuery, the data is
written to ingestion-time partitioned tables. For more information, see
[Introduction to partitioned tables](https://docs.cloud.google.com/bigquery/docs/partitioned-tables).

If you query your tables directly instead of using the auto-generated views, you
must use the `_PARTITIONTIME` pseudocolumn in your query. For more information,
see [Querying partitioned tables](https://docs.cloud.google.com/bigquery/docs/querying-partitioned-tables).

## Pricing

For information on Google Play data transfer pricing, see the
[Pricing](https://cloud.google.com/bigquery/pricing#data-transfer-service-pricing) page.

Once data is transferred to BigQuery, standard
BigQuery [storage](https://cloud.google.com/bigquery/pricing#storage) and
[query](https://cloud.google.com/bigquery/pricing#queries) pricing applies.

## What's next

- To see how your Google Play reports are transferred to BigQuery, see [Google Play report transformations](https://docs.cloud.google.com/bigquery/docs/play-transformation).
- For an overview of BigQuery Data Transfer Service, see [Introduction to BigQuery Data Transfer Service](https://docs.cloud.google.com/bigquery/docs/dts-introduction).
- For information on using transfers including getting information about a transfer configuration, listing transfer configurations, and viewing a transfer's run history, see [Working with transfers](https://docs.cloud.google.com/bigquery/docs/working-with-transfers).