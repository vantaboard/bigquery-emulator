# Load Klaviyo data into BigQuery

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

You can load data from Klaviyo to BigQuery using the
[BigQuery Data Transfer Service](https://docs.cloud.google.com/bigquery/docs/dts-introduction) for Klaviyo connector. With the BigQuery Data Transfer Service, you can
schedule recurring transfer jobs that add your latest data from
Klaviyo to BigQuery.

## Before you begin

The following sections describe the prerequisites that you need to do before you
create a Klaviyo data transfer.

### Klaviyo prerequisites

You must have a read-only private API key to allow the Klaviyo
connector to transfer data to BigQuery. For more information, see
[Create a private key](https://developers.klaviyo.com/en/docs/authenticate_#create-a-private-key).

### BigQuery prerequisites

- Verify that you have completed all actions required to [enable the BigQuery Data Transfer Service](https://docs.cloud.google.com/bigquery/docs/enable-transfer-service).
- [Create a BigQuery dataset](https://docs.cloud.google.com/bigquery/docs/datasets) to store your data.

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

If you intend to set up transfer run notifications for Pub/Sub,
ensure that you have the `pubsub.topics.setIamPolicy` IAM
permission. Pub/Sub permissions aren't required if you only set up
email notifications. For more information, see
[BigQuery Data Transfer Service run notifications](https://docs.cloud.google.com/bigquery/docs/transfer-run-notifications).

## Set up a Klaviyo data transfer

Add Klaviyo data into BigQuery by setting up a
transfer configuration using one of the following options:

### Console

1. Go to the Data transfers page in the Google Cloud console.

   [Go to Data transfers](https://console.cloud.google.com/bigquery/transfers)
2. Click **Create transfer**.

3. In the **Source type** section, for **Source** , choose **Klaviyo - Preview**.

4. In the **Data source details** section, do the following:

   - For **Private API Key** , enter your private API key. For more information, see [Klaviyo prerequisites](https://docs.cloud.google.com/bigquery/docs/klaviyo-transfer#klaviyo-prerequisites).
   - Optional: For **Start Date**, specify a start date for new records to be included in the data transfer. Only records created on or after this date are included in the data transfer. The default value is 3 months before the transfer run date.
   - For **Klaviyo objects to transfer** , click **Browse** to select any objects to be transferred to the BigQuery destination dataset. You can also manually enter any objects to include in the data transfer in this field.
5. In the **Destination settings** section, for **Dataset**, choose the
   dataset that you created to store your data.

6. In the **Transfer config name** section, for **Display name**, enter a
   name for the data transfer.

7. In the **Schedule options** section:

   - In the **Repeat frequency** list, select an option to specify how often this data transfer runs. To specify a custom repeat frequency, select **Custom** . If you select **On-demand** , then this transfer runs when you [manually trigger the transfer](https://docs.cloud.google.com/bigquery/docs/working-with-transfers#manually_trigger_a_transfer).
   - If applicable, select either **Start now** or **Start at set time**, and provide a start date and run time.
8. Optional: In the **Notification options** section, do the following:

   - To enable email notifications, click the **Email notification** toggle. When you enable this option, the transfer administrator receives an email notification when a transfer run fails.
   - To enable [Pub/Sub transfer run notifications](https://docs.cloud.google.com/bigquery/docs/transfer-run-notifications) for this transfer, click the **Pub/Sub notifications** toggle. You can select your [topic](https://docs.cloud.google.com/pubsub/docs/publish-message-overview) name, or you can click **Create a topic** to create one.
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
    --display_name=NAME
    --target_dataset=DATASET
    --params='PARAMETERS'
```

Replace the following:

- <var translate="no">`PROJECT_ID`</var> (optional): your Google Cloud project ID. If `--project_id` isn't supplied to specify a particular project, the default project is used.
- <var translate="no">`DATA_SOURCE`</var>: the data source --- `klaviyo`.
- <var translate="no">`NAME`</var>: the display name for the data transfer configuration. The transfer name can be any value that lets you identify the transfer if you need to modify it later.
- <var translate="no">`DATASET`</var>: the target dataset for the transfer configuration.
- <var translate="no">`PARAMETERS`</var>: the parameters for the created transfer
  configuration in JSON format. For example:
  `--params='{"param":"param_value"}'`. The following are the parameters for
  a Klaviyo data transfer:

  - `assets`: the path to the Klaviyo objects to be transferred to BigQuery.
  - `connector.authentication.privateApiKey`: the private API key for the Klaviyo account.
  - `connector.startDate`: (Optional) a start date for new records to be included in the data transfer, in the format `YYYY-MM-DD`. Only records created on or after this date are included in the data transfer. The default value is 3 months before the transfer run date.

The following command creates a Klaviyo data
transfer in the default project.

```bash
    bq mk
        --transfer_config
        --target_dataset=mydataset
        --data_source=klaviyo
        --display_name='My Transfer'
        --params= ' {
            "assets": [ "Events" , "Flows"] ,
            "connector.authentication.privateApiKey" : "pk_123456789123",
            "connector.startDate": "2025-10-20"
            }'
```
When you save the transfer configuration, the Klaviyo connector automatically triggers a transfer run according to your schedule option. With every transfer run, the Klaviyo connector transfers all available data from Klaviyo into BigQuery.

<br />

To manually run a data transfer outside of your regular schedule, you can start
a [backfill run](https://docs.cloud.google.com/bigquery/docs/working-with-transfers#manually_trigger_a_transfer).

For a list of data that's included in a Klaviyo data transfer,
see [Klaviyo data model reference](https://docs.cloud.google.com/bigquery/docs/klaviyo-data-model).

## Data type mapping

The following table maps Klaviyo data types to the
corresponding BigQuery data types:

| Klaviyo data type | BigQuery data type |
|---|---|
| `String` | `STRING` |
| `Text` | `STRING` |
| `Integer` | `INTEGER` |
| `Boolean` | `BOOLEAN` |
| `Date (YYYY-MM-DD HH:MM:SS)` | `TIMESTAMP` |
| `List` | `ARRAY` |

## Pricing

There is no cost to transfer Klaviyo data into
BigQuery while this feature is in
[Preview](https://cloud.google.com/products#product-launch-stages).

## Troubleshoot transfer setup

If you are having issues setting up your data transfer, see [Klaviyo transfer issues](https://docs.cloud.google.com/bigquery/docs/transfer-troubleshooting#klaviyo-issues).

## What's next

- For an overview of the BigQuery Data Transfer Service, see [What is BigQuery Data Transfer Service](https://docs.cloud.google.com/bigquery/docs/dts-introduction).
- For information on using transfers including getting information about a transfer configuration, listing transfer configurations, and viewing a transfer's run history, see [Manage transfers](https://docs.cloud.google.com/bigquery/docs/working-with-transfers).