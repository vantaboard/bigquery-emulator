# Load Salesforce data into BigQuery

You can load data from your Salesforce Sales Cloud to BigQuery using the
[BigQuery Data Transfer Service](https://docs.cloud.google.com/bigquery/docs/dts-introduction) for Salesforce connector. With the
BigQuery Data Transfer Service, you can schedule recurring transfer jobs that
add your latest data from your Salesforce Sales Cloud to
BigQuery.

## Limitations

Salesforce data transfers are subject to the following limitations:

- The Salesforce connector only supports transfers from Salesforce Sales Cloud.
- The Salesforce connector only supports fields included in Salesforce Bulk API V1 version 64.0. Some fields that were included in previous versions of the Salesforce Bulk API might not be supported. For more information about these changes to the Salesforce connector, see [Salesforce Bulk API](https://docs.cloud.google.com/bigquery/docs/transfer-changes#salesforce).
- The Salesforce connector uses Salesforce Bulk API V1 to connect to the Salesforce Sales Cloud endpoint to retrieve data.
  - The Salesforce connector only supports the Salesforce Bulk API V1 to connect to the Salesforce instance, and only supports the transfer of entities which are supported by the Salesforce Bulk API. For more information about what entities are supported, see ['Entity is not supported by the Bulk API' error](https://help.salesforce.com/s/articleView?id=000383508&type=1).
- The Salesforce connector does not support the transfer of the following objects that have binary fields.
  - `Attachment`
  - `ContentVersion`
  - `Document`
  - `StaticResource`
  - `Scontrol`
  - `EmailCapture`
  - `MailMergeTemplate`
- The minimum interval time between recurring data transfers is 15 minutes. The default interval for a recurring transfer is 24 hours.
- Due to Salesforce processing limits, scheduling too many data transfers at a time can lead to delays or failures. We recommend that you limit Salesforce data transfers to the following:
  - Have no more than 10 assets per transfer configuration.
  - Across your different transfer configurations, have no more than 10 simultaneous transfer runs at a time.
- A single transfer configuration can only support one data transfer run at a given time. In the case where a second data transfer is scheduled to run before the first transfer is completed, then only the first data transfer completes while any other data transfers that overlap with the first transfer is skipped.
  - To avoid skipped transfers within a single transfer configuration, we recommend that you increase the duration of time between large data transfers by configuring the **Repeat frequency**.
- If you are using network attachments with your data transfer, you must [set up a public network address translation (NAT)](https://docs.cloud.google.com/nat/docs/set-up-manage-network-address-translation) with a static IP address. For more information, see [Set up IP allowlist for Salesforce transfers](https://docs.cloud.google.com/bigquery/docs/salesforce-transfer#salesforce-allowlist).
- If your configured network attachment and virtual machine (VM) instance are located in different regions, there might be cross-region data movement when you transfer data from Salesforce.

### Incremental transfer limitations

Incremental Salesforce transfers are subject to the following limitations:

<br />

- You can only choose `TIMESTAMP` columns as watermark columns.
- Incremental ingestion is only supported for assets with valid watermark columns.
- Values in a watermark column must be monotonically increasing.
- Incremental transfers cannot sync delete operations in the source table.
- A single transfer configuration can only support either incremental or full ingestion.
- You cannot update objects in the `asset` list after the first incremental ingestion run.
- You cannot change the write mode in a transfer configuration after the first incremental ingestion run.
- You cannot change the watermark column or the primary key after the first incremental ingestion run.
- The destination BigQuery table is clustered using the provided primary key and is subject to [clustered table limitations](https://docs.cloud.google.com/bigquery/docs/clustered-tables#limitations).
- When you update an existing transfer configuration to the incremental ingestion mode for the first time, the first data transfer after that update transfers all available data from your data source. Any subsequent incremental data transfers will transfer only the new and updated rows from your data source.

## Data ingestion options

The following sections provide more information on the data ingestion options
when you set up a Salesforce data transfer.

### Full or incremental transfers

You can specify how data is loaded into BigQuery by selecting
either the **Full** or **Incremental** write
preference in the transfer configuration when you [set up a
Salesforce transfer](https://docs.cloud.google.com/bigquery/docs/salesforce-transfer#sf-transfer-setup). Incremental transfers
are supported in [preview](https://cloud.google.com/products#product-launch-stages).

> [!NOTE]
> **Note:** To request feedback or support for incremental transfers, send email to [dts-preview-support@google.com](mailto:dts-preview-support@google.com).

You can configure a *full* data transfer to transfer all data from your Salesforce datasets with each data transfer.

<br />

Alternatively, you can configure an *incremental* data transfer
([Preview](https://cloud.google.com/products#product-launch-stages)) to only
transfer data that was changed since the last data transfer, instead of loading
the entire dataset with each data transfer. If you select **Incremental** for
your data transfer, you must specify either the **Append** or **Upsert** write
modes to define how data is written to BigQuery during an
incremental data transfer. The following sections describe the available write
modes.

#### Append write mode

The append write mode only inserts new rows to your destination table. This option
strictly appends transferred data without checking for existing records, so
this mode can potentially cause data duplication in the destination table.

When you select the append mode, you must select a watermark column. A
watermark column is required for the Salesforce connector to track changes
in the source table.
Select a watermark column that is only updated when the record was created, and won't change with subsequent updates. For example, the `CreatedDate` column.

<br />

#### Upsert write mode

The upsert write mode either updates a row or inserts a new row in your
destination table by checking for a primary key. You can specify a primary key
to let the Salesforce connector determine what changes are needed to keep
your destination table up to date with your source table. If the specified
primary key is present in the destination BigQuery table during a data transfer, then the Salesforce
connector updates that row with new data from the source table. If a primary key
is not present during a data transfer, then the Salesforce connector
inserts a new row.

When you select the upsert mode, you must select a watermark column and a
primary key:
\* A watermark column is required for the Salesforce connector to track changes in the source table. \* Select a watermark column that updates every time a row is modified. We recommend using the `SystemModstamp` or `LastModifiedDate` column.

<br />

- The primary key can be one or more columns on your table that are required
  for the Salesforce connector to determine if it
  needs to insert or update a row.

  Select columns that contain non-null values that are unique across all
  rows of the table. We recommend columns that include system-generated
  identifiers, unique reference codes (for example, auto-incrementing IDs), or
  immutable time-based sequence IDs.

  To prevent potential data loss or data corruption, the primary key columns
  that you select must have unique values. If you have doubts about the
  uniqueness of your chosen primary key column, then we recommend that you
  use the append write mode instead.

### Incremental ingestion behavior

When you make changes to the table schema in your data source, incremental data transfers
from those tables are reflected in BigQuery in the following
ways:

| Changes to data source | Incremental ingestion behavior |
|---|---|
| Adding a new column | A new column is added to the destination BigQuery table. Any previous records for this column will have null values. |
| Deleting a column | The deleted column remains in the destination BigQuery table. New entries to this deleted column are populated with null values. |
| Changing the data type in a column | The connector only supports [data type conversions that are supported by the `ALTER COLUMN` DDL statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#details_21). Any other data type conversions causes the data transfer to fail. If you encounter any issues, we recommend creating a new transfer configuration. |
| Renaming a column | The original column remains in the destination BigQuery table as is, while a new column is added to the destination table with the updated name. |

## Before you begin

The following sections describe the steps that you need to take before you
create a Salesforce data transfer.

### Create a Salesforce Connected App

You must [create a Salesforce Connected App](https://help.salesforce.com/s/articleView?id=sf.connected_app_create.htm&type=5)
with the following required configurations:

- [Configure the Basic Information](https://help.salesforce.com/s/articleView?id=sf.connected_app_create_basics.htm&type=5) in the Connected App. The **Connected App Name** and **Contact Email** fields are required for a Salesforce transfer.
- [Enable OAuth Settings](https://help.salesforce.com/s/articleView?id=sf.connected_app_create_api_integration.htm&type=5) with the following configurations:
  - Select the **Enable OAuth Settings** checkbox.
  - In the **Callback URL** field, enter the following:
    - For a production environment, enter `https://login.salesforce.com/services/oauth2/token`.
    - For a sandbox environment, enter `https://test.salesforce.com/services/oauth2/token`.
  - Verify that the **Issue JSON Web Token(JWT)-based access tokens for named users** checkbox isn't selected.
- In the **Selected OAuth Scopes** section, select **Manage user data via APIs (api)**.
- Clear the **Required Proof Key for Code Exchange (PKCE) Extension for Supported Authorization Flows** checkbox.
- Select the **Enable Client Credentials Flow** , then click **OK** on the notice that appears.

Once you have configured the Connected App with the required configurations,
click **Save**. You are redirected to the detail page of your newly created
Connected App.

Once you have created the Connected App, you must also configure the client
credentials flow by doing the following:

1. Click **Setup**.
2. In the search bar, search for *Connected Apps*.
3. Click **Manage Apps** \> **Connected Apps** . If you are using the Salesforce Lightning Experience, click **Manage Connected Apps**.
4. On the Connected App that you have created, click **Edit**.
5. The **App details** page appears. In the **Client Credentials Flow** section, enter your username in the **Run As** field. You can use the finder tool in this field to ensure that you have selected the correct user.
6. Click **Save**.

### Required Salesforce information

You must have the following Salesforce information when creating a
Salesforce data transfer:

| Parameter Name | Description |
|---|---|
| `myDomain` | Your [My Domain](https://help.salesforce.com/s/articleView?id=sf.domain_name_overview.htm) in Salesforce. |
| `clientId` | Consumer Key of the Salesforce connected application. |
| `clientSecret` | OAuth Client Secret or Consumer Secret of the Salesforce connected application. |

To obtain your `myDomain`, `clientID`, and `clientSecret` values,
select one of the following options:

### Salesforce Classic

### Retrieve `myDomain` details

To find your `myDomain`, do the following:

1. Sign in to the Salesforce platform.
2. Click **Setup**.
3. In the search bar, search for *My Domain*.
4. In the search results, click **Domain Management** \> **My Domain**.

In the **My Domain Details** section, your `myDomain` appears as
the prefix in **Current My Domain URL** . For example, if the My Domain URL is
`example.my.salesforce.com`, the `myDomain` value to use is `example`.

### Retrieve `ClientId` and `ClientSecret` details

To find your `ClientId` and `ClientSecret` values, do the following:

1. Sign in to the Salesforce platform.
2. Click **Setup**.
3. In the search bar, search for *Apps*.
4. In the **Build** section in the search results, click **Create** \> **Apps**.
5. Click a **Connected App Name**.
6. In the **Connected Apps** details page, click **Manage Consumer Details**.
7. Verify your identity using one of the registered methods. You can view the consumer details page for up to five minutes before you're prompted to verify your identity again.
8. In the **Consumer Details** page, the **Consumer Key** is your `ClientId` value. The **Customer Secret** is your `ClientSecret` value.

### Salesforce Lightning Experience

### Retrieve `myDomain` details

To find your `myDomain`, do the following:

1. Sign in to the Salesforce platform.
2. Click **Setup**.

![Open the Setup page in the Salesforce platform.](https://docs.cloud.google.com/static/bigquery/images/salesforce-platform-setup.png)

1. In the search bar, search for *My Domain*.
2. In the search results, click **Company Settings** \> **My Domain**.

In the **My Domain Details** section, your `myDomain` appears as
the prefix in **Current My Domain URL** . For example, if the My Domain URL is
`example.my.salesforce.com`, the `myDomain` value to use is `example`.

### Retrieve `ClientId` and `ClientSecret` details

1. Sign in to the Salesforce platform.
2. Click **Setup**.
3. In the search bar, search for *Apps*.
4. In the search results, click **Apps** \> **App Manager**.
5. Find a connected app, then click **View**.
6. Click **Manage Consumer Details**.
7. Verify your identity using one of the registered methods. You can view the consumer details page for up to five minutes before you're prompted to verify your identity again.
8. In the **Consumer Details** page, the **Consumer Key** is your `ClientId` value. The **Customer Secret** is your `ClientSecret` value.

### Set up IP allowlist for Salesforce transfers

You must configure your Google Cloud environment and your Salesforce
account to add specific IP addresses to the allowlist for data transfers.
This ensures that Salesforce only accepts connections from a trusted,
static IP address. This step is required if you are using network attachments
with your data transfers.

To do so, you must first set up and configure your Google Cloud network to use
a static IP address:

1. [Set up a public network address translation
   (NAT)](https://docs.cloud.google.com/nat/docs/set-up-manage-network-address-translation) with a static IP address within your Virtual Private Cloud network. The CloudNAT must be configured within the same region as the destination dataset for this data transfer.
2. [Set up a network attachment](https://docs.cloud.google.com/vpc/docs/create-manage-network-attachments) within the same VPC network. This resource is used by the BigQuery Data Transfer Service to access private services.

Next, you must [configure the trusted IP ranges in
Salesforce](https://help.salesforce.com/s/articleView?id=xcloud.security_networkaccess.htm&type=5).
When adding the range of the IP address, use the static IP address from your
Google Cloud public NAT for both the beginning and ending IP addresses for the
IP range.

> [!NOTE]
> **Note:** For more granular control, you can apply IP restrictions at the profile level or on every API call. For more information, see [Restrict Login IP Addresses in Profiles](https://help.salesforce.com/s/articleView?id=platform.login_ip_ranges.htm&type=5).

Once you have set up the IP ranges, you can now specify the static IP when you
[set up your transfer configuration](https://docs.cloud.google.com/bigquery/docs/sfmc-transfer#sfmc-transfer-setup) by selecting your network attachment in the
**Network attachment** field.

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

## Set up a Salesforce data transfer

Add Salesforce data into BigQuery by setting up a
transfer configuration using one of the following options:

### Console

1. Go to the Data transfers page in the Google Cloud console.

   [Go to Data transfers](https://console.cloud.google.com/bigquery/transfers)
2. Click **Create transfer**.

3. In the **Source type** section, for **Source** , choose **Salesforce**.

4. In the **Data source details** section, do the following:

   - For **Network attachment** , select a network attachment from the list. You must [configure your public NAT and set up your IP allow list](https://docs.cloud.google.com/bigquery/docs/salesforce-transfer#salesforce-allowlist) before you can use a network attachment with this data transfer.
   - For **My Domain** , enter your Salesforce [My Domain](https://help.salesforce.com/s/articleView?id=sf.domain_name_overview.htm).
   - For **Client ID**, enter the Salesforce connected application Consumer Key.
   - For **Client secret**, enter the Salesforce connected application Consumer Secret.
   - For **Ingestion type** , select **Full** or **Incremental** .
     - If you select **Incremental** ([Preview](https://cloud.google.com/products#product-launch-stages)), for **Write mode** , select either **Append** or **Upsert** . For more information about the different write modes, see [Full or
       incremental
       transfers](https://docs.cloud.google.com/bigquery/docs/salesforce-transfer#full_or_incremental_transfers).
   - For **Salesforce objects to transfer** , click **Browse** :
     - Select any objects to be transferred to the BigQuery destination dataset. You can also manually enter any objects to include in the data transfer in this field.
     - If you have selected **Append** as your incremental write mode, you must select a column as the watermark column.
     - If you have selected **Upsert** as your incremental write mode, you must select a column as the watermark column, and then select one or more columns as the primary key.
5. In the **Destination settings** section, for **Dataset**, choose the
   dataset that you created to store your data.

6. In the **Transfer config name** section, for **Display name**, enter a
   name for the data transfer.

7. In the **Schedule options** section:

   - In the **Repeat frequency** list, select an option to specify how often this data transfer runs. To specify a custom repeat frequency, select **Custom** . If you select **On-demand** , then this transfer runs when you [manually trigger the transfer](https://docs.cloud.google.com/bigquery/docs/working-with-transfers#manually_trigger_a_transfer).
   - If applicable, select either **Start now** or **Start at set time**, and provide a start date and run time.
8. Optional: In the **Notification options** section, do the following:

   - To enable email notifications, click the **Email notification** toggle. When you enable this option, the transfer administrator receives an email notification when a transfer run fails.
   - To enable [Pub/Sub transfer run notifications](https://docs.cloud.google.com/bigquery/docs/transfer-run-notifications) for this transfer, click the **Pub/Sub notifications** toggle. You can select your [topic](https://docs.cloud.google.com/pubsub/docs/overview#types) name, or you can click **Create a topic** to create one.
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

Where:

- <var translate="no">PROJECT_ID</var> (optional): your Google Cloud project ID. If `--project_id` isn't supplied to specify a particular project, the default project is used.
- <var translate="no">DATA_SOURCE</var>: the data source --- `salesforce`.
- <var translate="no">NAME</var>: the display name for the data transfer configuration. The transfer name can be any value that lets you identify the transfer if you need to modify it later.
- <var translate="no">DATASET</var>: the target dataset for the transfer configuration.
- <var translate="no">PARAMETERS</var>: the parameters for the created transfer
  configuration in JSON format. For example:
  `--params='{"param":"param_value"}'`. The following are the parameters for
  a Salesforce data transfer:

  - `connector.authentication.oauth.clientId`: the Consumer Key of the Salesforce connected application.
  - `connector.authentication.oauth.clientSecret`: OAuth Client Secret or Consumer Secret of the Salesforce connected application.
  - `connector.authentication.oauth.myDomain`: the [Salesforce My Domain](https://help.salesforce.com/s/articleView?id=sf.domain_name_overview.htm). For example, if your domain URL is `example.my.salesforce.com`, then the value is `example`.
  - `ingestionType`: specify either `full` or `incremental`. Incremental transfers are supported in [preview](https://cloud.google.com/products#product-launch-stages). For more information, see [Full or incremental
    transfers](https://docs.cloud.google.com/bigquery/docs/salesforce-transfer#full_or_incremental_transfers).
  - `writeMode`: specify either `WRITE_MODE_APPEND` or `WRITE_MODE_UPSERT`.
  - `watermarkColumns`: specify columns in your table as watermark columns. This field is required for incremental transfers.
  - `primaryKeys`: specify columns in your table as primary keys. This field is required for incremental transfers.
  - `assets`: the path to the Salesforce objects to be transferred to BigQuery.

When specifying multiple assets during an incremental transfer, the values
of the `watermarkColumns` and `primaryKeys` fields correspond to the
position of values in the `assets` field. In the following example,
`Id` corresponds to the table `Account`, while `master_label`
and `type` corresponds to the table `CaseHistory`.

<br />

```bash
      "primaryKeys":[['Id'], ['master_label','type']],
      "assets":["Account","CaseHistory"],
  
```

<br />

The following command creates an incremental Salesforce data transfer in the
default project, and uses the `APPEND` write mode.

```bash
bq mk
    --transfer_config
    --target_dataset=mydataset
    --data_source=salesforce
    --display_name='My Transfer'
    --params='{"assets": ["Account", "CaseHistory"]
        "connector.authentication.oauth.clientId": "1234567890",
        "connector.authentication.oauth.clientSecret":"ABC12345",
        "connector.authentication.oauth.myDomain":"MyDomainName",
        "connector.authentication.username":"user1@force.com",
        "connector.authentication.password":"abcdef1234",
        "ingestionType":"incremental",
        "writeMode":"WRITE_MODE_UPSERT",
        "watermarkColumns":["SystemModstamp","CreatedDate"]
        "primaryKeys":[['Id'], ['master_label','type']]}'
```

### API

Use the [`projects.locations.transferConfigs.create`](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs/create)
method and supply an instance of the [`TransferConfig`](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs#TransferConfig)
resource.
When you save the transfer configuration, the Salesforce connector automatically triggers a transfer run according to your schedule option. With every transfer run, the Salesforce connector transfers all available data from Salesforce into BigQuery.

<br />

To manually run a data transfer outside of your regular schedule, you can start
a [backfill run](https://docs.cloud.google.com/bigquery/docs/working-with-transfers#manually_trigger_a_transfer).

## Data type mapping

The following table maps Salesforce data types to the
corresponding BigQuery data types:

| Salesforce data type | BigQuery data type |
|---|---|
| `_bool` | `BOOLEAN` |
| `_int` | `INTEGER` |
| `_long` | `INTEGER` |
| `_double` | `FLOAT` |
| `currency` | `FLOAT` |
| `percent` | `FLOAT` |
| `geolocation (latitude)` | `FLOAT` |
| `geolocation (longitude)` | `FLOAT` |
| `date` | `DATE` |
| `datetime` | `TIMESTAMP` |
| `time` | `TIME` |
| `picklist` | `STRING` |
| `multipicklist` | `STRING` |
| `combobox` | `STRING` |
| `reference` | `STRING` |
| `base64` | `STRING` |
| `textarea` | `STRING` |
| `phone` | `STRING` |
| `id` | `STRING` |
| `url` | `STRING` |
| `email` | `STRING` |
| `encryptedstring` | `STRING` |
| `datacategorygroupreference` | `STRING` |
| `location` | `STRING` |
| `address` | `STRING` |
| `anyType` | `STRING` |

## Pricing

For pricing information about Salesforce transfers, see
[Data Transfer Service pricing](https://docs.cloud.google.com/bigquery/pricing#data-transfer-service-pricing).

## Troubleshoot transfer setup

If you are having issues setting up your data transfer, see [Salesforce transfer issues](https://docs.cloud.google.com/bigquery/docs/transfer-troubleshooting#salesforce-issues).

## What's next

- For an overview of the BigQuery Data Transfer Service, see [Introduction to BigQuery Data Transfer Service](https://docs.cloud.google.com/bigquery/docs/dts-introduction).
- For information on using transfers including getting information about a transfer configuration, listing transfer configurations, and viewing a transfer's run history, see [Working with transfers](https://docs.cloud.google.com/bigquery/docs/working-with-transfers).
- Learn how to [load data with cross-cloud operations](https://docs.cloud.google.com/bigquery/docs/load-data-using-cross-cloud-transfer).