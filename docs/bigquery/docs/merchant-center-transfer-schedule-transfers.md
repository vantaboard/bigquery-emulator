# Schedule Merchant Center Transfers

> [!WARNING]
>
> **Preview**
>
>
> This product is
>
> subject to the "Pre-GA Offerings Terms" in the General Service Terms section of the
> [Service Specific
> Terms](https://docs.cloud.google.com/terms/service-terms#1).
>
> Pre-GA products are available "as is" and might have limited support.
>
> For more information, see the
> [launch stage descriptions](https://cloud.google.com/products/#product-launch-stages).

> [!NOTE]
> **Note:** To get support or provide feedback for Merchant Center transfers with BigQuery Data Transfer Service, contact [gmc-transfer-preview@google.com](mailto:gmc-transfer-preview@google.com).

## Before you begin

Before you create a Merchant Center data transfer:

- Verify that you have completed all actions required to [enable the BigQuery Data Transfer Service](https://docs.cloud.google.com/bigquery/docs/enable-transfer-service).
- [Create a BigQuery dataset](https://docs.cloud.google.com/bigquery/docs/datasets) to store the Merchant Center data.
  - For dataset region, we support using the default option, Multi-region, in either US or EU.
  - If you want to create a dataset in a specific region, the Merchant Center data transfer is only supported in the following regions:
  - `us-east4 (Northern Virginia)`,
  - `asia-northeast1 (Tokyo)`,
  - `asia-southeast1 (Singapore)`,
  - `australia-southeast1 (Sydney)`,
  - `europe-north1 (Finland)`,
  - `europe-west2 (London)`,
  - `europe-west6 (Zurich)`.
- If you intend to set up transfer run notifications for Pub/Sub, you must have `pubsub.topics.setIamPolicy` permissions. Pub/Sub permissions are not required if you just set up email notifications. For more information, see [BigQuery Data Transfer Service Run Notifications](https://docs.cloud.google.com/bigquery/docs/transfer-run-notifications).

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

### Required Merchant Center roles

- [Standard access](https://support.google.com/merchants/answer/1637190) to
  the Merchant Center account that is used in the transfer
  configuration. If you set up a transfer using a service account, the
  [service account must have access](https://developers.google.com/merchant/api/guides/authorization/access-your-account#give-service-account-access-account) to
  the Merchant Center account. You can verify access by clicking the
  **Users** section in the [Merchant Center UI](https://merchants.google.com/).

- To access price competitiveness, price insights, and best sellers data, you
  must meet the [eligibility requirements for market insights](https://support.google.com/merchants/answer/9712881).

## Set up a Merchant Center transfer

Setting up a data transfer for Merchant Center reporting requires the
following:

- **Merchant ID** or **Multi-client account ID** : This is the Merchant ID shown in the [Merchant Center UI](https://merchants.google.com/mc).

To create a data transfer for Merchant Center reporting:

### Console

1. Go to the Data transfers page in the Google Cloud console.

   [Go to Data transfers](https://console.cloud.google.com/bigquery/transfers)
2. Click **Create transfer**.

3. On the **Create Transfer** page:

   - In the **Source type** section, for **Source** , choose **Google Merchant Center**.
   - In the **Transfer config name** section, for **Display name** , enter a
     name for the data transfer such as `My Transfer`. The transfer name can
     be any value that lets you identify the transfer if you need to modify
     it later.


     ![Transfer name](https://docs.cloud.google.com/static/bigquery/images/transfer-name.png)
   - In the **Schedule options** section:

     - Select a **Repeat frequency** . If you select **Hours** , **Days** , **Weeks** , or **Months** , you must also specify a frequency. You can also select **Custom** to specify a custom repeat frequency. If you select **On-demand** , then this data transfer runs when you [manually trigger the transfer](https://docs.cloud.google.com/bigquery/docs/working-with-transfers#manually_trigger_a_transfer).
     - If applicable, select either **Start now** or **Start at set time**, and provide a start date and run time.
     - For **Start date and run time** , enter the date and time to start the transfer. This value should be at least 24 hours later than the current UTC time. If you chose **Start now**, this option is disabled.

     If you leave the schedule options set to **Start now** , the first data
     transfer run starts immediately, and it fails with the following error
     message: `No data to transfer found for the Merchant account. If you
     have just created this transfer, you may need to wait for up to a day
     before the data of your Merchant account are prepared and available
     for the transfer.` The next scheduled run should run successfully. If
     the data of your Merchant account are prepared on the same date in UTC
     time, you can
     [set up a backfill](https://docs.cloud.google.com/bigquery/docs/working-with-transfers#manually_trigger_a_transfer)
     for today's run.


     ![Transfer schedule](https://docs.cloud.google.com/static/bigquery/images/merchant-schedule.png)
   - In the **Destination settings** section, for **Destination dataset**,
     choose the dataset that you created to store your data.


     ![Transfer dataset](https://docs.cloud.google.com/static/bigquery/images/transfer-dataset.png)
   - In the **Data source details** section, for **Merchant ID** , enter your
     Merchant ID or MCA ID. Select the report(s) that you would like to
     transfer. See
     [Supported Reports](https://docs.cloud.google.com/bigquery/docs/merchant-center-transfer#supported_reports)
     for more details.


     ![Google Merchant Center transfer data.](https://docs.cloud.google.com/static/bigquery/images/merchant-transfer-data-console.png)
   - In the **Service Account** menu, select a
     [service account](https://docs.cloud.google.com/iam/docs/service-account-overview)
     from the service accounts associated with yourGoogle Cloud project.
     You can associate a service account with your data transfer instead of
     using your user credentials. For more information about using service
     accounts with data transfers, see
     [Use service accounts](https://docs.cloud.google.com/bigquery/docs/use-service-accounts).

     - If you signed in with a [federated identity](https://docs.cloud.google.com/iam/docs/workforce-identity-federation), then a service account is required to create a data transfer. If you signed in with a [Google Account](https://docs.cloud.google.com/iam/docs/principals-overview#google-account), then a service account for the transfer is optional.
     - The service account must have the [required permissions](https://docs.cloud.google.com/bigquery/docs/merchant-center-transfer-schedule-transfers#required_permissions).
   - (Optional) In the **Notification options** section:

     - Click the toggle to enable email notifications. When you enable this option, the transfer administrator receives an email notification when a transfer run fails.
     - For **Select a Pub/Sub topic** , choose your [topic](https://docs.cloud.google.com/pubsub/docs/overview#types) name or click **Create a topic** . This option configures Pub/Sub run [notifications](https://docs.cloud.google.com/bigquery/docs/transfer-run-notifications) for your transfer.
4. Click **Save**.

### bq

Enter the `bq mk` command and supply the transfer creation flag ---
`--transfer_config`. The following flags are also required:

- `--data_source`
- `--target_dataset`
- `--display_name`
- `--params`

You can also supply the `--project_id` flag to specify a particular
project. If `--project_id` isn't specified, the default project is used.

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

- <var translate="no">project_id</var> is your project ID.
- <var translate="no">dataset</var> is the target dataset for the transfer configuration.
- <var translate="no">name</var> is the display name for the transfer configuration. The transfer name can be any value that lets you identify the transfer if you need to modify it later.
- <var translate="no">parameters</var> contains the parameters for the created transfer configuration in JSON format. For example: `--params='{"param":"param_value"}'`.
  - For Merchant Center data transfers, you must supply the `merchant_id` parameter.
  - The `export_products` parameter specifies whether to transfer product and product issues data. This parameter is included by default, even if you don't specify the `export_products` parameter. Google recommends that you include this parameter explicitly and set it to "true".
  - The `export_regional_inventories` parameter specifies whether to transfer regional inventories data.
  - The `export_local_inventories` parameter specifies whether to transfer local inventories data.
  - The `export_price_competitiveness` parameter specifies whether to transfer price competitiveness data.
  - The `export_price_insights` parameter specifies whether to transfer price insights data.
  - The `export_best_sellers_v2` parameter specifies whether to transfer best sellers data.
  - The `export_performance` parameter specifies whether to transfer product performance data.
- <var translate="no">data_source</var> is the data source --- `merchant_center`.
- <var translate="no">service_account_name</var> is the service account name used to authenticate your data transfer. The service account should be owned by the same `project_id` used to create the transfer and it should have all of the [required permissions](https://docs.cloud.google.com/bigquery/docs/merchant-center-transfer-schedule-transfers#required_permissions).

> [!CAUTION]
> **Caution:** You cannot configure notifications by using the command-line tool.

For example, the following command creates a Merchant Center data transfer
named `My Transfer` using Merchant ID `1234` and target dataset
`mydataset`. The data transfer is created in your default project.

    bq mk \
    --transfer_config \
    --target_dataset=mydataset \
    --display_name='My Transfer' \
    --params='{"merchant_id":"1234","export_products":"true","export_regional_inventories":"true","export_local_inventories":"true","export_price_benchmarks":"true","export_best_sellers":"true"}' \
    --data_source=merchant_center

The first time you run the command, you receive a message like the
following:

`[URL omitted] Please copy and paste the above URL into your web browser and
follow the instructions to retrieve an authentication code.`

Follow the instructions in the message and paste the authentication code on
the command line.

> [!CAUTION]
> **Caution:** When you create a Merchant Center data transfer by using the command-line tool, the transfer configuration is set up using the default values for **Schedule** (every 24 hours at creation time). The first transfer run will start immediately, and it will fail with the following error message: \`No data to transfer found for the Merchant account.

If you have just created this data transfer, you may need to wait for up to a day
before the data of your Merchant account are prepared and available for the
transfer.\` The next scheduled run should run successfully. If the data of
your Merchant account are prepared on the same date in UTC time, you can
[set up a backfill](https://docs.cloud.google.com/bigquery/docs/working-with-transfers#manually_trigger_a_transfer) for today's run.

### API

Use the [`projects.locations.transferConfigs.create`](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs/create)
method and supply an instance of the [`TransferConfig`](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs#TransferConfig)
resource.

## Troubleshoot Merchant Center transfer setup

If you are having issues setting up your data transfer, see
[Merchant Center transfer issues](https://docs.cloud.google.com/bigquery/docs/transfer-troubleshooting#merchant)
in [Troubleshooting BigQuery Data Transfer Service transfer setup](https://docs.cloud.google.com/bigquery/docs/transfer-troubleshooting).