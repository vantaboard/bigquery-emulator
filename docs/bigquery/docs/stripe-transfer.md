# Load Stripe data into BigQuery

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

You can load data from Stripe to BigQuery using the
Stripe connector with the [BigQuery Data Transfer Service](https://docs.cloud.google.com/bigquery/docs/dts-introduction). By using the
Stripe connector, you can schedule recurring transfer jobs that
add your latest data from Stripe to
BigQuery.

## Limitations

Stripe data transfers are subject to the following limitations:

- A Stripe data transfer loads currencies according to Stripe's minor units. For more information, see [Minor units in API amounts](https://docs.stripe.com/currencies#minor-units).
- The Stripe connector only transfers pre-generated reports for each Stripe account. The Stripe connector doesn't generate new reports based on new Stripe data.
  - To transfer up-to-date reports, generate the reports manually in the Stripe dashboard before starting the Stripe data transfer.
  - For more information, see [Stripe reporting](https://docs.stripe.com/stripe-reports).
- The Stripe connector doesn't support webhook-based events, real-time updates, or Stripe Sigma.
- Stripe data transfers from [Stripe regions that are in preview](https://stripe.com/global) might encounter issues with data transfers:
  - Filtering options are restricted or unavailable in Stripe preview regions.
  - Conditional data transfers and queries aren't supported in Stripe preview regions.
  - You might encounter long data transfer runtimes when transferring data from Stripe preview regions.
- The Stripe connector supports some objects with the `StartDate` filter.
  - The required format for the `StartDate` filter is `YYYY-MM-DD`. If no start date is provided, the connector defaults to three years before the current date. If a date before January 1, 2011 is provided, then the connector automatically uses January 1, 2011.
  - For a list of supported objects, see [Objects with `StartDate` filter support](https://docs.cloud.google.com/bigquery/docs/stripe-transfer#objects_with_startdate_filter_support).
- A single transfer configuration can only support one data transfer run at a given time. If a second data transfer is scheduled to run before the first transfer is completed, then only the first data transfer completes while any other data transfers that overlap with the first transfer is skipped.
  - To avoid skipped transfers within a single transfer configuration, we recommend that you increase the duration of time between large data transfers by configuring the **Repeat frequency**.

## Before you begin

The following sections describe the steps that you need to take before you
create a Stripe data transfer.

### Stripe prerequisites

- You must have a Stripe developer account to authorize a Stripe data transfer. To register a Stripe account, see [Stripe registration](https://dashboard.stripe.com/register).
- Configure your Stripe platform application with the following steps:
  1. Navigate to the **Developers** section in the Stripe dashboard.
  2. Under **Connect** , configure your platform to support **Standard** and **Express** accounts.
- The following information is needed to create a Stripe data transfer:
  - Note your Stripe account ID. For more information, see [Create an account](https://docs.stripe.com/get-started/account).
  - Note your secret key or restricted key. For more information, see [API keys](https://docs.stripe.com/keys).
- If you plan on transferring data from connected accounts, ensure that your platform is configured for Stripe Connect and has access to the necessary account capabilities. For more information about Stripe Connect, see [Platforms and marketplaces with Stripe Connect](https://docs.stripe.com/connect).
  - For more information about connected accounts, see [Connect account types](https://docs.stripe.com/connect/accounts).

### Required BigQuery roles


To get the permissions that
you need to create a transfer,

ask your administrator to grant you the
[BigQuery Admin](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquery#bigquery.admin) (`roles/bigquery.admin`) IAM role on the project.


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

## Stripe account types

The Stripe connector supports both Stripe platform
accounts and Stripe connected accounts. For more information,
see [Connect account types](https://docs.stripe.com/connect/accounts).

### Connect to platform accounts

To run a Stripe data transfer from only one platform account,
do the following when you [set up the transfer configuration](https://docs.cloud.google.com/bigquery/docs/stripe-transfer#transfer-setup):

- Enter the platform account ID for the platform account in the **Account Id** field.
- Enter the secret or restricted key for the platform account in the **Secret/API Key** field.
- For **SyncAllConnectedAccounts** , select **False**.

To run a Stripe data transfer for multiple accounts, for example, for a platform account linked with connected accounts, do the following when you [set up the transfer configuration](https://docs.cloud.google.com/bigquery/docs/stripe-transfer#transfer-setup):

- Enter the platform account ID for the platform account in the **Account Id** field.
- Enter the secret or restricted key for the platform account in the **Secret/API Key** field.
- For **SyncAllConnectedAccounts** , select **True**.

### Connect to connected accounts

Connected accounts are Stripe accounts linked to Stripe using Stripe Connect.

To run a Stripe data transfer from a connected account, do the following when you [set up the transfer configuration](https://docs.cloud.google.com/bigquery/docs/stripe-transfer#transfer-setup):

- Enter the platform account ID for the connected account in the **Account Id** field.
- Enter the secret or restricted key for the platform account to which the connected account is connected to in the **Secret/API Key** field.
- For **SyncAllConnectedAccounts** , select **False**.

## Set up a Stripe data transfer

Add Stripe data into BigQuery by setting up a
transfer configuration using one of the following options:

### Console

1. Go to the Data transfers page in the Google Cloud console.

   [Go to Data transfers](https://console.cloud.google.com/bigquery/transfers)
2. Click **Create transfer**.

3. In the **Source type** section, for **Source** , select **Stripe**.

4. In the **Data source details** section, do the following:

   - For **Platform/Connected Account ID** , enter the Stripe account ID. For more information, see [Stripe prerequisites](https://docs.cloud.google.com/bigquery/docs/stripe-transfer#stripe-prerequisites).
   - For **Stripe Secret Key** , enter the API key for the Stripe account. For more information, see [Stripe prerequisites](https://docs.cloud.google.com/bigquery/docs/stripe-transfer#stripe-prerequisites).
   - For **Start Date** , enter a date in the format `YYYY-MM-DD`. The data transfer loads Stripe data starting from this date.
   - Select **Sync all connected accounts**, to sync all connected accounts.
   - For **Stripe objects to transfer** , enter the names of the Stripe objects to transfer, or click **Browse** and select the objects that you want to transfer.
5. In the **Destination settings** section, for **Dataset**, select the dataset
   that you created to store your data.

6. In the **Transfer config name** section, for **Display name**, enter a name
   for the data transfer.

7. In the **Schedule options** section, do the following:

   - In the **Repeat frequency** list, select an option to specify how often this data transfer runs. To specify a custom repeat frequency, select **Custom** . If you select **On-demand** , then this transfer runs when you [manually trigger the
     transfer](https://docs.cloud.google.com/bigquery/docs/working-with-transfers#manually_trigger_a_transfer).
   - If applicable, select either **Start now** or **Start at set time**, and provide a start date and run time.
8. Optional: In the **Notification options** section, do the following:

   - To enable email notifications, click the **Email notification** toggle. After you enable this option, the transfer administrator receives an email notification when a transfer run fails.
   - To enable [Pub/Sub transfer run
     notifications](https://docs.cloud.google.com/bigquery/docs/transfer-run-notifications) for this transfer, click the **Pub/Sub notifications** toggle. You can select your [topic](https://docs.cloud.google.com/pubsub/docs/overview#types) name, or you can click **Create a
     topic** to create one.
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
- <var translate="no">DATA_SOURCE</var>: the data source --- `stripe`.
- <var translate="no">DISPLAY_NAME</var>: the display name for the data transfer configuration. The transfer name can be any value that lets you identify the transfer if you need to modify it later.
- <var translate="no">DATASET</var>: the target dataset for the transfer configuration.
- <var translate="no">PARAMETERS</var>: the parameters for the created transfer
  configuration in JSON format. For example:
  `--params='{"param":"param_value"}'`. The following are the parameters for
  a Stripe data transfer:

  - `assets`: a list of Stripe objects to be included in this transfer.
  - `connector.accountId`: the Stripe account ID.
  - `connector.secretKey`: the API key for the Stripe account.
  - `connector.syncAllConnectedAccounts`: specify `true` to sync all connected accounts.
  - `connector.startDate`: enter a date in the format `YYYY-MM-DD`. The data transfer loads Stripe data starting from this date.

For example, the following command creates a Stripe data transfer in the
default project with all the required parameters:

```bash
  bq mk \
      --transfer_config \
      --target_dataset=mydataset \
      --data_source=stripe \
      --display_name='My Transfer' \
      --params= ' {
  "assets" : [ "Customers" , "Accounts", "BalanceSummaryReport"] ,
  "connector.accountId" : "acct_000000000000",
  "connector.secretKey" : "sk_test_000000000",
  "connector.syncAllConnectedAccounts" : "true",
  "connector.startDate": "2025-05-20"
  }'
```

### API

Use the
[`projects.locations.transferConfigs.create` method](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs/create)
and supply an instance of the
[`TransferConfig` resource](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs#TransferConfig).
When you save the transfer configuration, the Stripe connector automatically triggers a transfer run according to your schedule option. With every transfer run, the Stripe connector transfers all available data from Stripe into BigQuery.

<br />

To manually run a data transfer outside of your regular schedule, you can start
a [backfill run](https://docs.cloud.google.com/bigquery/docs/working-with-transfers#manually_trigger_a_transfer).

## Data type mapping

The following table maps Stripe data types to the corresponding
BigQuery data types.

| Stripe data type | BigQuery data type | Notes |
|---|---|---|
| `String` | `STRING` |
| `Dictionary` | `STRING` | When a nested object is loaded into BigQuery, it is converted into a flattened object. This flattened object is then saved as a single literal string within the table. |
| `Integer` | `INT64` |
| `Double` | `DOUBLE` |
| `Float` | `FLOAT` |
| `Decimal` | `BIGNUMERIC` |
| `BigInt (long)` | `BIGNUMERIC` |
| `Boolean` | `BOOL` |
| `Datetime` | `TIMESTAMP` |
| `Unix timestamp` | `TIMESTAMP` |

## Objects with `StartDate` filter support

The following Stripe objects support the `StartDate` filter,
which lets you load time-based data:

- Accounts
- ApplicationFees
- BalanceTransactions
- Cardholders
- Charges
- Coupons
- Customers
- Disputes
- EarlyFraudWarnings
- Events
- FileLinks
- Files
- InvoiceItems
- Invoices
- IssuingCards
- IssuingDisputes
- PaymentIntent
- Payouts
- Plans
- Prices
- Products
- PromotionCodes
- Refunds
- Reviews
- ShippingRates
- Subscriptions
- TaxRates
- TopUps
- Transfers
- ValueListItems
- ValueLists

## Troubleshoot transfer setup

If you are having issues setting up your data transfer, see [Stripe transfer issues](https://docs.cloud.google.com/bigquery/docs/transfer-troubleshooting#stripe-issues).

## Pricing

There is no cost to transfer Stripe data into
BigQuery while this feature is in
[Preview](https://cloud.google.com/products#product-launch-stages).

## What's next

- For an overview of the BigQuery Data Transfer Service, see [Introduction to BigQuery Data Transfer Service](https://docs.cloud.google.com/bigquery/docs/dts-introduction).
- For information on using transfers including getting information about a transfer configuration, listing transfer configurations, and viewing a transfer's run history, see [Working with transfers](https://docs.cloud.google.com/bigquery/docs/working-with-transfers).
- Learn how to [load data with cross-cloud operations](https://docs.cloud.google.com/bigquery/docs/load-data-using-cross-cloud-transfer).