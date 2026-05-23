# Load HubSpot data into BigQuery

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

You can load data from HubSpot to BigQuery using the
[BigQuery Data Transfer Service](https://docs.cloud.google.com/bigquery/docs/dts-introduction) for HubSpot connector. With the BigQuery Data Transfer Service, you can
schedule recurring transfer jobs that add your latest data from
HubSpot to BigQuery.

## Limitations

The HubSpot connector requires a private app access token for
authentication.

- You must have a HubSpot private app to have a private app access token before you can set up a HubSpot data transfer. For more information, see [HubSpot prerequisites](https://docs.cloud.google.com/bigquery/docs/hubspot-transfer#hubspot-prerequisites).

## Before you begin

The following sections describe the prerequisites that you need to do before you
create a HubSpot data transfer.

### HubSpot prerequisites

You must create a HubSpot private app and retrieve your private
app access token. For more information, see the following:

- [Create a private app](https://developers.hubspot.com/docs/apps/legacy-apps/private-apps/overview)
- [View private app access token information](https://developers.hubspot.com/docs/apps/legacy-apps/private-apps/overview#view-private-app-access-token-information)

#### Required scopes

[HubSpot scopes](https://developers.hubspot.com/docs/apps/developer-platform/build-apps/authentication/scopes)
allow other services to access specific API endpoints to access data on a
HubSpot account. To allow the HubSpot connector to
transfer data from your HubSpot account, you must enable the
following scopes for your private app:

#### Required scopes

- `automation`
- `business-intelligence`
- `collector_graphql_query_execute`
- `collector_graphql_query_read`
- `content`
- `conversations.read`
- `conversations.visitor_identification.tokens.create`
- `crm.import`
- `crm.lists.read`
- `crm.objects.companies.read`
- `crm.objects.contacts.read`
- `crm.objects.deals.read`
- `crm.objects.owners.read`
- `crm.schemas.companies.read`
- `crm.schemas.contacts.read`
- `crm.schemas.deals.read`
- `e-commerce`
- `files`
- `forms`
- `forms-uploaded-files`
- `hubdb`
- `integration-sync`
- `oauth`
- `sales-email-read`
- `settings.users.read`
- `settings.users.teams.read`
- `social`
- `tickets`
- `timeline`
- `transactional-email`

For information about configuring scopes for your private app, see
[Legacy private apps](https://developers.hubspot.com/docs/apps/legacy-apps/private-apps/overview).

#### Optional scopes

To transfer certain HubSpot data objects, such as sensitive or
highly sensitive data, you must also enable the following scopes:

#### Scopes for sensitive data

To transfer sensitive data, enable the following scopes:

- `crm.objects.contacts.sensitive.read`
- `crm.objects.companies.sensitive.read`
- `crm.objects.deals.sensitive.read`
- `crm.objects.appointments.sensitive.read`
- `crm.objects.custom.sensitive.read`
- `crm.objects.projects.sensitive.read`
- `tickets.sensitive`

<br />

#### Scopes for highly sensitive data

To transfer highly sensitive data, enable the following scopes:

- `crm.objects.contacts.highly_sensitive.read`
- `crm.objects.companies.highly_sensitive.read`
- `crm.objects.deals.highly_sensitive.read`
- `crm.objects.custom.highly_sensitive.read`
- `crm.objects.projects.highly_sensitive.read`
- `tickets.highly_sensitive`

<br />

#### Scopes for other data

Some data objects require specific scopes to be included in a data
transfer. For example, to transfer `objects.courses`, you must
have the scope `crm.objects.courses.read`. The following list
includes a list of required scopes needed to transfer its corresponding data
object:

- `crm.objects.appointments.read`
- `crm.objects.courses.read`
- `crm.objects.custom.read`
- `crm.objects.goals.read`
- `crm.objects.leads.read`
- `crm.objects.line_items.read`
- `crm.objects.listings.read`
- `crm.objects.orders.read`
- `crm.objects.owners.read`
- `crm.objects.products.read`
- `crm.objects.services.read`
- `crm.objects.users.read`
- `crm.pipelines.orders.read`
- `crm.schemas.appointments.read`
- `crm.schemas.courses.read`
- `crm.schemas.custom.read`
- `crm.schemas.deals.read`
- `crm.schemas.line_items.read`
- `crm.schemas.listings.read`
- `crm.schemas.orders.read`
- `crm.schemas.quotes.read`
- `crm.schemas.services.read`
- `marketing.campaigns.read`
- `marketing.campaigns.revenue.read`
- `settings.users.read`
- `settings.users.teams.read`
- `crm.dealsplits.read_write`
- `crm.export`
- `crm.extensions_calling_transcripts.read`
- `crm.schemas.carts.read`
- `crm.objects.carts.read`
- `crm.objects.subscriptions.read`
- `crm.objects.commercepayments.read`
- `crm.objects.projects.read`
- `crm.objects.quotes.read`
- `crm.objects.partner-clients.read`
- `crm.objects.partner-services.read`
- `crm.objects.marketing_events.read`
- `crm.objects.invoices.read`
- `crm.objects.feedback_submissions.read`
- `crm.objects.forecasts.read`
- `crm.schemas.commercepayments.read`
- `crm.schemas.forecasts.read`
- `crm.schemas.invoices.read`
- `crm.schemas.projects.read`
- `crm.schemas.subscriptions.read`

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

## Set up a HubSpot data transfer

Add HubSpot data into BigQuery by setting up a
transfer configuration using one of the following options:

### Console

1. Go to the Data transfers page in the Google Cloud console.

   [Go to Data transfers](https://console.cloud.google.com/bigquery/transfers)
2. Click **Create transfer**.

3. In the **Source type** section, for **Source** , choose **HubSpot - Preview**.

4. In the **Data source details** section, do the following:

   - For **Access token** , enter your private access token key. For more information, see [HubSpot prerequisites]().
   - For **HubSpot objects to transfer** , click **Browse** to select any objects to be transferred to the BigQuery destination dataset. You can also manually enter any objects to include in the data transfer in this field.
     - You can select custom HubSpot objects, which appear in the **Browse** menu in the format `CUSTOM_OBJECT_NAME__c`.
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
- <var translate="no">`DATA_SOURCE`</var>: the data source --- `hubspot`.
- <var translate="no">`NAME`</var>: the display name for the data transfer configuration. The transfer name can be any value that lets you identify the transfer if you need to modify it later.
- <var translate="no">`DATASET`</var>: the target dataset for the transfer configuration.
- <var translate="no">`PARAMETERS`</var>: the parameters for the created transfer
  configuration in JSON format. For example:
  `--params='{"param":"param_value"}'`. The following are the parameters for
  a HubSpot data transfer:

  - `assets`: the path to the HubSpot objects to be transferred to BigQuery.
    - You can specify custom HubSpot objects using the format `CUSTOM_OBJECT_NAME__c`.
  - `connector.authentication.oauth.accessToken`: the HubSpot private access token key.

The following command creates a HubSpot data
transfer in the default project.

```bash
    bq mk \
        --transfer_config \
        --target_dataset=mydataset \
        --data_source=hubspot \
        --display_name='My Transfer' \
        --params= ' {
            "assets": ["Contacts", "Deals"],
            "connector.authentication.oauth.accessToken": "pat_123456789123"}'
```
When you save the transfer configuration, the HubSpot connector automatically triggers a transfer run according to your schedule option. With every transfer run, the HubSpot connector transfers all available data from HubSpot into BigQuery.

<br />

To manually run a data transfer outside of your regular schedule, you can start
a [backfill run](https://docs.cloud.google.com/bigquery/docs/working-with-transfers#manually_trigger_a_transfer).

## Data type mapping

The following table maps HubSpot data types to the
corresponding BigQuery data types:

| HubSpot data type | BigQuery data type |
|---|---|
| `String` | `STRING` |
| `Text` | `STRING` |
| `Integer` | `INTEGER` |
| `Boolean` | `BOOLEAN` |
| `Date` | `TIMESTAMP` |
| `Datetime` | `TIMESTAMP` |
| `Long` | `BIGNUMERIC` |

## Pricing

There is no cost to transfer HubSpot data into
BigQuery while this feature is in
[Preview](https://cloud.google.com/products#product-launch-stages).

## Troubleshoot transfer setup

If you are having issues setting up your data transfer, see [HubSpot transfer issues](https://docs.cloud.google.com/bigquery/docs/transfer-troubleshooting#hubspot-issues).

## What's next

- For an overview of the BigQuery Data Transfer Service, see [What is BigQuery Data Transfer Service](https://docs.cloud.google.com/bigquery/docs/dts-introduction).
- For information on using transfers including getting information about a transfer configuration, listing transfer configurations, and viewing a transfer's run history, see [Manage transfers](https://docs.cloud.google.com/bigquery/docs/working-with-transfers).
- Learn how to [load data with cross-cloud operations](https://docs.cloud.google.com/bigquery/docs/load-data-using-cross-cloud-transfer).