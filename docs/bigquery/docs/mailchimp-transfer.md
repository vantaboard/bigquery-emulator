# Load Mailchimp data into BigQuery

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

You can load data from Mailchimp to BigQuery using the
[BigQuery Data Transfer Service](https://docs.cloud.google.com/bigquery/docs/dts-introduction) for Mailchimp connector. With the BigQuery Data Transfer Service, you can
schedule recurring transfer jobs that add your latest data from
Mailchimp to BigQuery. The Mailchimp
connector has multi-account support, including both Standard and Express
Mailchimp accounts.

## Limitations

- The Mailchimp marketing API only supports a maximum of 10 simultaneous connections per user. Exceeding this limit results in the error `429: TooManyRequests: You have exceeded the limit of 10 simultaneous connections`
  - To avoid reaching this rate limit, we recommend only running one data transfer per Mailchimp account.
  - For more information, see [Error glossary](https://mailchimp.com/developer/marketing/docs/errors/#error-glossary).
- The `Integer` data type in Mailchimp has a maximum supported value of 2,147,483,647 across all objects.
  - However, some Mailchimp fields support higher values, such as the `Quantity` field in `EcommerceOrderLines` and `EcommerceCartLines`.

### Array field limitations

Mailchimp connector doesn't support `ARRAY` fields in
the following Mailchimp objects:

| Mailchimp Object | Unsupported `ARRAY` fields |
|---|---|
| `Campaigns` | `VariateSettings_SubjectLines` `VariateSettings_SendTimes` `VariateSettings_FromNames` `VariateSettings_ReplyToAddresses` `VariateSettings_Contents` `VariateSettings_Combinations` |
| `EcommerceCarts` | `Lines` |
| `EcommerceProducts` | `Variants` |
| `ListMembers` | `TagsAggregate` |
| `ListMergeFields` | `Options_Choices` |
| `Lists` | `Modules` |
| `AuthorizedApps` | `Users` |
| `AutomationEmails` | `Settings_AutoFbPost` |
| `CampaignOpenEmailDetails` | `Opens` |
| `EcommerceProductImages` | `VariantIds` |
| `ListSignupForms` | `Contents`, `Styles` |
| `ReportEmailActivity` | `Activity` |
| `Reports` | `Timewarp` |

## Before you begin

The following sections describe the prerequisites that you need to do before you
create a Mailchimp data transfer.

### Mailchimp prerequisites

To enable data transfers from Mailchimp to
BigQuery, you must have a Mailchimp API key for
authorization and access. For information on obtaining an API key, see
[Generate an API key](https://mailchimp.com/help/about-api-keys/#Generate_an_API_key).

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

## Set up a Mailchimp data transfer

Add Mailchimp data into BigQuery by setting up a
transfer configuration using one of the following options:

### Console

1. Go to the Data transfers page in the Google Cloud console.

   [Go to Data transfers](https://console.cloud.google.com/bigquery/transfers)
2. Click **Create transfer**.

3. In the **Source type** section, for **Source** , choose **Mailchimp - Preview**.

4. In the **Data source details** section, do the following:

   - For **API Key** , enter your Mailchimp API key. For more information, see [Mailchimp prerequisites](https://docs.cloud.google.com/bigquery/docs/mailchimp-transfer#mailchimp-prerequisites).
   - Optional: For **Start Date** , specify a start date for new records to be included in the data transfer. Only records created on or after this date are included in the data transfer.
     - Enter a date in the format `YYYY-MM-DD`. The minimum value is `2001-01-01`.
   - For **Mailchimp objects to transfer** , click **Browse** to select any objects to be transferred to the BigQuery destination dataset. You can also manually enter any objects to include in the data transfer in this field.
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
- <var translate="no">`DATA_SOURCE`</var>: the data source --- `mailchimp`.
- <var translate="no">`NAME`</var>: the display name for the data transfer configuration. The transfer name can be any value that lets you identify the transfer if you need to modify it later.
- <var translate="no">`DATASET`</var>: the target dataset for the transfer configuration.
- <var translate="no">`PARAMETERS`</var>: the parameters for the created transfer
  configuration in JSON format. For example:
  `--params='{"param":"param_value"}'`. The following are the parameters for
  a Mailchimp data transfer:

  - `assets`: the path to the Mailchimp objects to be transferred to BigQuery.
  - `connector.authentication.apiKey`: the Mailchimp API key.
  - `connector.startDate`: (Optional) a start date for new records to be included in the data transfer, in the format `YYYY-MM-DD`. Only records created on or after this date are included in the data transfer.

The following command creates a Mailchimp data
transfer in the default project.

```bash
    bq mk
        --transfer_config
        --target_dataset=mydataset
        --data_source=mailchimp
        --display_name='My Transfer'
        --params='{"assets": "Lists",
            "connector.authentication.apiKey":"1234567",
            "connector.startDate":"2025-01-01"}'
```
When you save the transfer configuration, the Mailchimp connector automatically triggers a transfer run according to your schedule option. With every transfer run, the Mailchimp connector transfers all available data from Mailchimp into BigQuery.

<br />

To manually run a data transfer outside of your regular schedule, you can start
a [backfill run](https://docs.cloud.google.com/bigquery/docs/working-with-transfers#manually_trigger_a_transfer).

## Data type mapping

The following table maps Mailchimp data types to the
corresponding BigQuery data types:

| Mailchimp data type | BigQuery data type | Description |
|---|---|---|
| `String` | `STRING` |   |
| `Integer` | `INT64` |   |
| `Number` | `BIGNUMERIC` | Mailchimp `Number` data objects are mapped to either the `BIGNUMERIC` data type for financial-related fields such as `Price` and `OrderTotal`, or the `FLOAT64` data type, for other fields such as `Stats_OpenRate` and `Location_Latitude`. |
| `Number` | `FLOAT64` |   |
| `Boolean` | `BOOLEAN` |   |
| `String` in date-time format | `TIMESTAMP` | `STRING` data types in date-time format are represented in ISO 8601 format. For example, `2019-08-24T14:15:22Z`. |

## Pricing

There is no cost to transfer Mailchimp data into
BigQuery while this feature is in
[Preview](https://cloud.google.com/products#product-launch-stages).

## Troubleshoot transfer setup

If you are having issues setting up your data transfer, see [Mailchimp transfer issues](https://docs.cloud.google.com/bigquery/docs/transfer-troubleshooting#mailchimp-issues).

## What's next

- For an overview of the BigQuery Data Transfer Service, see [What is BigQuery Data Transfer Service](https://docs.cloud.google.com/bigquery/docs/dts-introduction).
- For information on using transfers including getting information about a transfer configuration, listing transfer configurations, and viewing a transfer's run history, see [Manage transfers](https://docs.cloud.google.com/bigquery/docs/working-with-transfers).