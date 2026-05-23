# Load PayPal data into BigQuery

> [!WARNING]
>
> **Preview**
>
>
> This feature is
>
> subject to the "Pre-GA Offerings Terms" in the General Service Terms section of the
> [Service Specific
> Terms](https://docs.cloud.google.com/terms/service-terms#1).
>
> Pre-GA features are available "as is" and might have limited support.
>
> For more information, see the
> [launch stage descriptions](https://cloud.google.com/products/#product-launch-stages).

> [!NOTE]
> **Note:** To get support or provide feedback for this feature, contact [dts-preview-support@google.com](mailto:dts-preview-support@google.com).

You can load data from PayPal to BigQuery using the
PayPal connector with the [BigQuery Data Transfer Service](https://docs.cloud.google.com/bigquery/docs/dts-introduction). With the
PayPal connector, you can schedule recurring transfer jobs that
add your latest data from PayPal to
BigQuery.

The PayPal connector supports production and sandbox PayPal
accounts.

## Supported objects

| PayPal object types | BigQuery-supported objects | Date filter support |
|---|---|---|
| Transactions | TransactionReports | Supported |
| Transactions | TransactionReportsCartInfoItemDetails | Supported |
| Transactions | TransactionReportsIncentiveDetails | Supported |
| Disputes | Disputes | Supported |
| Disputes | DisputeDetails | Supported |
| Disputes | DisputeTransactions | Supported |
| Payments | Payments | Supported |
| Payments | PaymentTransactions | Supported |
| Balance | Balance | Not supported |
| Products | Products | Not supported |
| Products | ProductDetails | Not supported |
| Invoices | Invoices | Supported |

## Limitations

PayPal data transfers are subject to the following limitations:

- There can be a delay of several hours before PayPal transactions become available through the PayPal API.
  - We recommend scheduling subsequent data transfers at longer intervals (no more than one every hour) to prevent missing data.
- The PayPal connector only supports [transactions data](https://docs.cloud.google.com/bigquery/docs/paypal-transfer#supported_objects) from the past 3 years.
- The PayPal connector only supports [disputes data](https://docs.cloud.google.com/bigquery/docs/paypal-transfer#supported_objects) from the past 6 months.
- PayPal APIs use different page size limits for each data object. The PayPal connector uses the maximum page size allowed by PayPal in a data transfer.
  - However, some objects like `Payments` or `Payment Transactions` use smaller page size limits. This can lead to slower data transfers, especially when dealing with large datasets.

## Before you begin

The following sections describe the steps that you need to take before you
create a PayPal data transfer.

### PayPal prerequisites

To enable data transfers from PayPal, you must have the
following:

- You must have a PayPal Developer account. For more information, see [PayPal Developer Program](https://developer.paypal.com/developer-program/).
- Create a PayPal REST API app. For more information, see [Get started with PayPal REST APIs](https://developer.paypal.com/api/rest/).
  - In the **Apps \& Credentials** section, note the client ID and secret key for the app.
  - In the **Features** section, enable the **Transaction search** and **Invoicing** API permissions.

### Required BigQuery roles


To get the permissions that
you need to create a transfer,

ask your administrator to grant you the
[BigQuery Admin](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquery#bigquery.admin) (`roles/bigquery.admin`) IAM role.


For more information about granting roles, see [Manage access to projects, folders, and organizations](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).


This predefined role contains

the permissions required to create a transfer. To see the exact permissions that are
required, expand the **Required permissions** section:


#### Required permissions

The following permissions are required to create a transfer:

- `bigquery.transfers.update` on the user
- `bigquery.datasets.get` on the target dataset
- `bigquery.datasets.update` on the target dataset


You might also be able to get
these permissions
with [custom roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles) or
other [predefined roles](https://docs.cloud.google.com/iam/docs/roles-overview#predefined).

### BigQuery prerequisites

- Verify that you have completed all actions required to [enable the BigQuery Data Transfer Service](https://docs.cloud.google.com/bigquery/docs/enable-transfer-service).
- [Create a BigQuery dataset](https://docs.cloud.google.com/bigquery/docs/datasets) to store your data.
- If you intend to set up transfer run notifications for Pub/Sub, ensure that you have the `pubsub.topics.setIamPolicy` Identity and Access Management (IAM) permission. Pub/Sub permissions are not required if you only set up email notifications. For more information, see [BigQuery Data Transfer Service run notifications](https://docs.cloud.google.com/bigquery/docs/transfer-run-notifications).

## Set up a PayPal data transfer

Add PayPal data into BigQuery by setting up a
transfer configuration using one of the following options:

### Console

1. Go to the Data transfers page in the Google Cloud console.

   [Go to Data transfers](https://console.cloud.google.com/bigquery/transfers)
2. Click **Create transfer**.

3. In the **Source type** section, for **Source** , select **PayPal**.

4. In the **Data source details** section, do the following:

   - For **Client Id** , enter the PayPal client ID. For more information, see [PayPal prerequisites](https://docs.cloud.google.com/bigquery/docs/paypal-transfer#paypal-prerequisites).
   - For **Client Secret** , enter the PayPal client secret key. For more information, see [PayPal prerequisites](https://docs.cloud.google.com/bigquery/docs/paypal-transfer#paypal-prerequisites).
   - Select **Is Sandbox** if you are using a sandbox PayPal account.
   - For **Start Date** , enter a date in the format `YYYY-MM-DD`. The data transfer loads PayPal data starting from this date.
     - If this field is left blank, this transfer defaults to retrieving data from the past 3 years.
     - For information about what objects support the start date filter, see [Supported objects](https://docs.cloud.google.com/bigquery/docs/paypal-transfer#supported_objects).
   - For **PayPal objects to transfer** , enter the names of the PayPal objects to transfer, or click **Browse** and select the objects that you want to transfer.
5. In the **Destination settings** section, for **Dataset**, select the dataset
   that you created to store your data.

6. In the **Transfer config name** section, for **Display name**, enter a name
   for the data transfer.

7. In the **Schedule options** section, do the following:

   - In the **Repeat frequency** list, select an option to specify how often this data transfer runs. To specify a custom repeat frequency, select **Custom** . If you select **On-demand** , then this transfer runs when you [manually trigger the transfer](https://docs.cloud.google.com/bigquery/docs/working-with-transfers#manually_trigger_a_transfer).
   - If applicable, select either **Start now** or **Start at set time**, and provide a start date and run time.
8. Optional: In the **Notification options** section, do the following:

   - To enable email notifications, toggle **Email notification** to the on position. When you enable this option, the transfer administrator receives an email notification when a transfer run fails.
   - To enable [Pub/Sub transfer run
     notifications](https://docs.cloud.google.com/bigquery/docs/transfer-run-notifications) for this transfer, toggle **Pub/Sub notifications** to the on position. You can select your [topic](https://docs.cloud.google.com/pubsub/docs/overview#types) name, or you can click **Create a topic** to create one.
9. Click **Save**.

### bq

Enter the [`bq mk` command](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#bq_mk)
and supply the transfer creation flag
`--transfer_config`:

```bash
bq mk
    --transfer_config
    --project_id=PROJECT_ID
    --data_source=DATA_SOURCE
    --display_name=DISPLAY_NAME
    --target_dataset=DATASET
    --params='PARAMETERS'
```

Where:

- <var translate="no">PROJECT_ID</var> (optional): your Google Cloud project ID. If `--project_id` isn't supplied to specify a particular project, the default project is used.
- <var translate="no">DATA_SOURCE</var>: the data source --- `paypal`.
- <var translate="no">DISPLAY_NAME</var>: the display name for the data transfer configuration. The transfer name can be any value that lets you identify the transfer if you need to modify it later.
- <var translate="no">DATASET</var>: the target dataset for the transfer configuration.
- <var translate="no">PARAMETERS</var>: the parameters for the created transfer
  configuration in JSON format. For example:
  `--params='{"param":"param_value"}'`. The following are the parameters for
  a PayPal data transfer:

  - `assets`: a list of PayPal objects to be included in this transfer.
  - `connector.authentication.clientId`: client ID of the PayPal application.
  - `connector.authentication.clientSecret`: client secret of the PayPal application.
  - `connector.isSandbox`: set value to `true` if you are using a sandbox PayPal account, or `false` if you are using a production PayPal account.
  - `connector.createdStartDate`: (Optional) enter a date in the format `YYYY-MM-DD`. The data transfer loads PayPal data starting from this date.

For example, the following command creates a PayPal data transfer in the
default project with all the required parameters:

```bash
  bq mk \
      --transfer_config \
      --target_dataset=mydataset \
      --data_source=PayPal \
      --display_name='My Transfer' \
      --params='{"assets":  ["Payments", "TransactionReports"],
          "connector.authentication.clientId": "112233445566",
          "connector.authentication.clientSecret":"123456789",
          "connector.isSandbox":"false",
          "connector.createdStartDate":  "2025-01-01"}'
```

When you create a data transfer using the bq command-line tool, the transfer
configuration schedules data transfers once every 8 hours.

> [!NOTE]
> **Note:** You cannot configure notifications if you set up a transfer configuration using the bq command-line tool.

### API

Use the
[`projects.locations.transferConfigs.create` method](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs/create)
and supply an instance of the
[`TransferConfig` resource](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs#TransferConfig).
When you save the transfer configuration, the PayPal connector automatically triggers a transfer run according to your schedule option. With every transfer run, the PayPal connector transfers all available data from PayPal into BigQuery.

<br />

To manually run a data transfer outside of your regular schedule, you can start
a [backfill run](https://docs.cloud.google.com/bigquery/docs/working-with-transfers#manually_trigger_a_transfer).

## Data type mapping

The following table maps PayPal data types to the corresponding
BigQuery data types.

| PayPal data type | BigQuery data type |
|---|---|
| `String` | `STRING` |
| `Decimal` | `BIGNUMERIC` |
| `Boolean` | `BOOL` |
| `Datetime` | `TIMESTAMP` |

## Troubleshoot transfer setup

If you are having issues setting up your data transfer, see [PayPal transfer issues](https://docs.cloud.google.com/bigquery/docs/transfer-troubleshooting#paypal-issues).

## Pricing

There is no cost to transfer PayPal data into
BigQuery while this feature is in
[Preview](https://cloud.google.com/products#product-launch-stages).

## What's next

- For an overview of the BigQuery Data Transfer Service, see [Introduction to BigQuery Data Transfer Service](https://docs.cloud.google.com/bigquery/docs/dts-introduction).
- To learn about managing transfer configurations, including how to obtain information, list configurations, and view run history, see [Manage transfers](https://docs.cloud.google.com/bigquery/docs/working-with-transfers).
- Learn how to [load data with cross-cloud operations](https://docs.cloud.google.com/bigquery/docs/load-data-using-cross-cloud-transfer).