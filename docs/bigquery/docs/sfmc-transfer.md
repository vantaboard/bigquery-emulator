# Load Salesforce Marketing Cloud data into BigQuery

You can load data from Salesforce Marketing Cloud to BigQuery using the
[BigQuery Data Transfer Service](https://docs.cloud.google.com/bigquery/docs/dts-introduction) for Salesforce Marketing Cloud connector. With the
BigQuery Data Transfer Service, you can schedule recurring transfer jobs that
add your latest data from Salesforce Marketing Cloud to
BigQuery.

## Limitations

Salesforce Marketing Cloud data transfers are subject to the following limitations:

- A single transfer configuration can only support one data transfer run at a given time. In the case where a second data transfer is scheduled to run before the first transfer is completed, then only the first data transfer completes while any other data transfers that overlap with the first transfer is skipped.
  - To avoid skipped transfers within a single transfer configuration, we recommend that you increase the duration of time between large data transfers by configuring the **Repeat frequency**.
- To use a network attachment with this data transfer, you must first [create a
  network attachment by defining a static IP
  address](https://docs.cloud.google.com/bigquery/docs/connections-with-network-attachment).
- If your configured network attachment and virtual machine (VM) instance are located in different regions, there might be cross-region data movement when you transfer data from Salesforce Marketing Cloud.

## Before you begin

The following sections describe the steps that you need to take before you
create a Salesforce Marketing Cloud data transfer.

### Salesforce Marketing Cloud prerequisites

You must have the following information when creating a Salesforce Marketing Cloud
data transfer:

| Parameter Name | Description |
|---|---|
| `subdomain` | The API subdomain, found in the base URI. For example, in the authentication base URI `https://SUBDOMAIN.auth.marketingcloudapis.com/`, <var translate="no">SUBDOMAIN</var> is your subdomain value. |
| `instance` | The API server instance, found in the URL after you sign into the Salesforce Marketing Cloud application. The instance value includes \`s\` followed by a numeric value. For example, in the URL \`https://mc.s4.exacttarget.com/\`, the instance value is \`s4\`. For more information, see [Find the stack location for a Marketing Cloud account](https://help.salesforce.com/s/articleView?id=000383566&type=1) |
| `clientId` | The client ID from the API integration. Navigate to **Setup** \> **Apps** \> **Installed Packages** , and then click the package name. The client ID is listed under **Components**. |
| `clientSecret` | The app integration client secret. Navigate to **Setup** \> **Apps** \> **Installed Packages** , and then click the package name. The client secret is listed under **Components**. |
| `Salesforce Marketing Cloud objects to transfer` | Compile a list of Salesforce Marketing Cloud objects to include in this transfer. You can select objects when you [set up a transfer configuration](https://docs.cloud.google.com/bigquery/docs/sfmc-transfer#sfmc-transfer-setup). For a list of supported objects, see [Supported tables](https://docs.cloud.google.com/bigquery/docs/sfmc-transfer#supported-tables). |

#### Set up IP allowlist for Salesforce Marketing Cloud transfers

You must configure your Google Cloud environment and your Salesforce Marketing Cloud
account to add specific IP addresses to the allowlist for data transfers.
This ensures that Salesforce Marketing Cloud only accepts connections from a trusted,
static IP address.

To do so, you must first set up and configure your Google Cloud network to use
a static IP address:

1. [Set up a public network address translation
   (NAT)](https://docs.cloud.google.com/nat/docs/set-up-manage-network-address-translation) with a static IP address within your Virtual Private Cloud network. The CloudNAT must be configured within the same region as the destination dataset for this data transfer.
2. [Set up a network attachment](https://docs.cloud.google.com/vpc/docs/create-manage-network-attachments) within the same VPC network. This resource is used by the BigQuery Data Transfer Service to access private services.

Next, you must [add the static IP address to the allowlist in
Salesforce Marketing Cloud](https://help.salesforce.com/s/articleView?id=mktg.mc_overview_allowlist_ip.htm&language=en_US&type=5).
When adding the range of the IP address, use the static IP address from your
Google Cloud public NAT for both the beginning and ending IP addresses for the
IP range.

Once you have set up the IP ranges, you can now specify the static IP when you
[set up your transfer configuration](https://docs.cloud.google.com/bigquery/docs/sfmc-transfer#sfmc-transfer-setup) by selecting your network attachment in the
**Network attachment** field.

#### Data extension object requirements

To include data extension objects in your data transfer, the object must meet
the following requirements:

- The name of the data extension object must include the `DataExtensionObject` prefix followed by the object name. For example, `DataExtensionObject_DATA_EXTENSION_NAME`.
- You must enable the `Read` scope for the data extension object.
- The file locations of the data extension object must have the `Read` and `Write` scopes.

### Install and configure Salesforce Marketing Cloud API integration package

You must install a server-to-server API integration package in
Salesforce Marketing Cloud. You can do so in Salesforce Marketing Cloud by installing a new
installed package, and specifying the component **API Integration** \>
**Server-to-Server** . For more information, see [Create and Install Packages](https://developer.salesforce.com/docs/marketing/marketing-cloud/guide/install-packages.html).

Once you've installed the API integration package, you must add the following
permissions scopes:

- Access: `Offline Access`
- Email: `Read`
- OTT: `Read`
- Push:`Read`
- SMS:`Read`
- Web: `Read`
- Documents and images: `Read`
- Saved Content: `Read`
- Journeys: `Read`
- Audiences: `Read`
- List and Subscribers: `Read`
- Date Extensions: `Read`
- File Locations `Read`
- Tracking Events: `Read`
- Callbacks: `Read`
- Subscriptions: `Read`
- Campaign: `Read`
- Assets: `Read`
- Accounts: `Read`
- OTT Channels: `Read`
- Users: `Read`

For more information, see [API Integration Permission Scopes](https://developer.salesforce.com/docs/marketing/marketing-cloud/guide/data-access-permissions.html).

### BigQuery prerequisites

- Verify that you have completed all actions required to [enable the BigQuery Data Transfer Service](https://docs.cloud.google.com/bigquery/docs/enable-transfer-service).
- [Create a BigQuery dataset](https://docs.cloud.google.com/bigquery/docs/datasets) to store your data.
- If you intend to set up transfer run notifications for Pub/Sub, ensure that you have the `pubsub.topics.setIamPolicy` Identity and Access Management (IAM) permission. Pub/Sub permissions are not required if you only set up email notifications. For more information, see [BigQuery Data Transfer Service run notifications](https://docs.cloud.google.com/bigquery/docs/transfer-run-notifications).

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

## Set up a Salesforce Marketing Cloud data transfer

Add Salesforce Marketing Cloud data into BigQuery by setting up a
transfer configuration using one of the following options:

### Console

1. Go to the Data transfers page in the Google Cloud console.

   [Go to Data transfers](https://console.cloud.google.com/bigquery/transfers)
2. Click **Create transfer**.

3. In the **Source type** section, for **Source** , select
   **Salesforce Marketing Cloud**.

4. In the **Data source details** section, do the following:

   - For **Network attachment** , select a network attachment from the menu. Before you can use a network attachment with this data transfer, you must [create a network attachment by defining a static IP address](https://docs.cloud.google.com/bigquery/docs/connections-with-network-attachment).
   - For **API Subdomain** , enter the [subdomain of your authentication base URI](https://docs.cloud.google.com/bigquery/docs/sfmc-transfer#sfmc-prereqs).
   - For **API instance** , enter the [API instance from the URL](https://docs.cloud.google.com/bigquery/docs/sfmc-transfer#sfmc-prereqs) after you sign in to the Marketing Cloud application.
   - For **Client ID** , enter the [client ID from your API integration
     package](https://docs.cloud.google.com/bigquery/docs/sfmc-transfer#sfmc-prereqs).
   - For **Client Secret** , enter the [client secret from your API integration
     package](https://docs.cloud.google.com/bigquery/docs/sfmc-transfer#sfmc-prereqs).
5. In the **Destination settings** section, for **Dataset**, select the
   dataset that you created to store your data.

6. In the **Transfer config name** section, for **Display name**, enter a
   name for the data transfer.

7. In the **Schedule options** section, do the following:

   - In the **Repeat frequency** list, select an option to specify how often this data transfer runs. To specify a custom repeat frequency, select **Custom** . If you select **On-demand** , then this transfer runs when you [manually trigger the transfer](https://docs.cloud.google.com/bigquery/docs/working-with-transfers#manually_trigger_a_transfer).
   - If applicable, select either **Start now** or **Start at set time**, and provide a start date and run time.
8. Optional: In the **Notification options** section, do the following:

   - To enable email notifications, click the **Email notification** toggle. When you enable this option, the transfer administrator receives an email notification when a transfer run fails.
   - To enable [Pub/Sub transfer run notifications](https://docs.cloud.google.com/bigquery/docs/transfer-run-notifications) for this transfer, click the **Pub/Sub notifications** toggle. You can select your [topic](https://docs.cloud.google.com/pubsub/docs/overview#types) name, or you can click **Create a topic** to create one.
9. Click **Save**.

### bq

Enter the [`bq mk` command](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#mk-transfer-config)
and supply the transfer creation flag --- `--transfer_config`.

```bash
bq mk
    --transfer_config
    --project_id=PROJECT_ID
    --data_source=DATA_SOURCE
    --display_name=DISPLAY_NAME
    --target_dataset=DATASET
    --params='PARAMETERS'
```

Replace the following:

- <var translate="no">PROJECT_ID</var> (optional): your Google Cloud project ID. If `--project_id` isn't supplied to specify a particular project, the default project is used.
- <var translate="no">DATA_SOURCE</var>: the data source (for example, `saphana`).
- <var translate="no">DISPLAY_NAME</var>: the display name for the transfer configuration. The data transfer name can be any value that lets you identify the transfer if you need to modify it later.
- <var translate="no">DATASET</var>: the target dataset for the transfer configuration.
- <var translate="no">PARAMETERS</var>: the parameters for the created transfer configuration in JSON format. For example: `--params='{"param":"param_value"}'`. The following are the parameters for a Salesforce Marketing Cloud transfer:
  - `connector.subdomain`: The API subdomain.
  - `connector.instance`: The API instance value.
  - `connector.authentication.oauth.clientId`: The app ID name for the OAuth client.
  - `connector.authentication.oauth.clientSecret`: The app secret for the OAuth client.
  - `assets`: a list of the names of the Salesforce Marketing Cloud tables to be transferred from Salesforce Marketing Cloud as part of the transfer.

For example, the following command creates a Salesforce Marketing Cloud data transfer in the
default project with all the required parameters:

```bash
  bq mk
      --transfer_config
      --target_dataset=mydataset
      --data_source=salesforce_marketing
      --display_name='My Transfer'
      --params='{"connector.subdomain": "abcd",
      "connector.instance": "x",
      "connector.authentication.oauth.clientId": "1234567890",
      "connector.authentication.oauth.clientSecret":"ABC12345"}'
```

### API

Use the [`projects.locations.transferConfigs.create` method](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs/create)
and supply an instance of the [`TransferConfig` resource](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs#TransferConfig).

When you save the transfer configuration, the Salesforce Marketing Cloud
connector automatically triggers a transfer run according to your schedule
option.

### Supported tables

With every transfer run, the Salesforce Marketing Cloud connector
transfers all available data from Salesforce Marketing Cloud into
BigQuery into the following tables based on the REST interface:

- `Assets`
- `CampaignAssets`
- `Campaigns`
- `Categories`
- `EventDefinitions`
- `FacebookMessengerProperties`
- `JourneyActivities`
- `Journeys`
- `LineMessengerProperties`
- `SendDefinitions`
- `Subscriptions`
- `DataExtension`
- `DataExtensionObject_DATA_EXTENSION_NAME`
- `Email`
- `LinkSend`
- `List`
- `ListSubscriber`
- `Subscriber`
- `TriggeredSendDefinition`

To manually run a data transfer outside of your regular schedule, you can start
a [backfill run](https://docs.cloud.google.com/bigquery/docs/working-with-transfers#manually_trigger_a_transfer).

## Data type mapping

The following table maps Salesforce Marketing Cloud data types to the corresponding
BigQuery data types.

| Salesforce Marketing Cloud data type | BigQuery data type |
|---|---|
| `Boolean` | `BOOLEAN` |
| `Number` | `INTEGER` |
| `Text` | `STRING` |
| `Decimal` | `FLOAT` |
| `EmailAddress` | `STRING` |
| `Phone` | `STRING` |
| `Date` | `DATE` |
| `DateTime` | `TIMESTAMP` |
| `Locale` | `STRING` |

## Troubleshoot transfer setup

If you are having issues setting up a Salesforce Marketing Cloud data transfer, try the
following troubleshooting steps:

- Ensure that the [configured authentication for the API integration package](https://docs.cloud.google.com/bigquery/docs/sfmc-transfer#sfmc-prereqs) is configured to **Server-to-server**.
- Ensure that the authentication app is configured with the [required permissions](https://developer.salesforce.com/docs/marketing/marketing-cloud/guide/data-access-permissions.html) under **Scope**.

### Error messages

Error: `invalid_grant. The client's IP address is unauthorized for this account. Allowlist the client's IP address in Marketing Cloud Administration.`

:   **Resolution:** Try one of the following steps:

    - Enable [all available IP addresses for Google Cloud resources](https://www.gstatic.com/ipranges/goog.json).
    - Configure your Google Cloud environment and your Salesforce Marketing Cloud account to add static IP addresses to the allowlist. For more information, see [Set up IP allowlist for Salesforce Marketing Cloud
      transfers](https://docs.cloud.google.com/bigquery/docs/sfmc-transfer#sfmc-allowlist).

Error: `INVALID_ARGUMENT. Table tableName does not exist in asset TableName`

:   **Resolution:** Ensure that you have the correct scope permissions configured
    in the Salesforce Marketing Cloud application. For more information, see [Salesforce Marketing Cloud prerequisites](https://docs.cloud.google.com/bigquery/docs/sfmc-transfer#sfmc-prereqs).

Error: `FAILED_PRECONDITION: There was an issue connecting to API.`

:   **Resolution:** This error can occur when you include a network attachment
    with your transfer but have not configured your public NAT and set up your IP
    allowlist. To resolve this error, follow the steps in [Create a network attachment](https://docs.cloud.google.com/bigquery/docs/connections-with-network-attachment#create_a_network_attachment)
    and follow the steps to create your network attachment by defining a static IP
    address.

## Pricing

For pricing information about Salesforce Marketing Cloud transfers, see
[Data Transfer Service pricing](https://docs.cloud.google.com/bigquery/pricing#bqdts).

## What's next

- For an overview of the BigQuery Data Transfer Service, see [Introduction to BigQuery Data Transfer Service](https://docs.cloud.google.com/bigquery/docs/dts-introduction).
- For information on using data transfers, including getting information about a transfer configuration, listing transfer configurations, and viewing a transfer's run history, see [Manage transfers](https://docs.cloud.google.com/bigquery/docs/working-with-transfers).
- Learn how to [load data with cross-cloud operations](https://docs.cloud.google.com/bigquery/docs/load-data-using-cross-cloud-transfer).