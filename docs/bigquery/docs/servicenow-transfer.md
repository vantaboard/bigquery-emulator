# Load ServiceNow data into BigQuery

You can load data from ServiceNow to BigQuery using the
[BigQuery Data Transfer Service](https://docs.cloud.google.com/bigquery/docs/dts-introduction) for ServiceNow connector. With the
BigQuery Data Transfer Service, you can schedule recurring transfer jobs that
add your latest data from ServiceNow to
BigQuery.

## Limitations

ServiceNow data transfers are subject to the following limitations:

- The ServiceNow connector only supports the [ServiceNow table API](https://www.servicenow.com/docs/bundle/zurich-api-reference/page/integrate/inbound-rest/concept/c_TableAPI.html).
- We don't recommend running concurrent data transfers on the same ServiceNow instance. This can lead to delays or failures due to load on the ServiceNow instance.
  - We recommend timing your transfer start times apart to prevent overlapping transfer runs.
- To improve data transfer performance, we recommend limiting the number of assets to 20 items per data transfer.
- The minimum interval time between recurring data transfers is 15 minutes. The default interval for a recurring transfer is 24 hours.
- A single transfer configuration can only support one data transfer run at a given time. In the case where a second data transfer is scheduled to run before the first transfer is completed, then only the first data transfer completes while any other data transfers that overlap with the first transfer is skipped.
  - To avoid skipped transfers within a single transfer configuration, we recommend that you increase the duration of time between large data transfers by configuring the **Repeat frequency**.
- To use a network attachment with this data transfer, you must first [create a
  network attachment by defining a static IP
  address](https://docs.cloud.google.com/bigquery/docs/connections-with-network-attachment).

### Incremental transfer limitations

Incremental ServiceNow transfers are subject to the following limitations:

<br />

- You can only choose `DATETIME` columns as watermark columns.
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

These sections describe the data ingestion options for setting up a
ServiceNow data transfer.

### Full or incremental transfers

You specify how data loads into BigQuery by selecting the
**Full** or **Incremental** write preference in the transfer configuration when
you [set up a ServiceNow transfer](https://docs.cloud.google.com/bigquery/docs/servicenow-transfer#servicenow_transfer_setup).
Incremental transfers are supported in
[preview](https://cloud.google.com/products#product-launch-stages).

> [!NOTE]
> **Note:** To request feedback or support for incremental transfers, send email to [dts-preview-support@google.com](mailto:dts-preview-support@google.com).

You can configure a *full* data transfer to transfer all data from your ServiceNow datasets with each data transfer.

<br />

Alternatively, you can configure an *incremental* data transfer
([Preview](https://cloud.google.com/products#product-launch-stages)) to only
transfer data that was changed since the last data transfer, instead of loading
the entire dataset with each data transfer. If you select **Incremental** for
your data transfer, you must specify either the **Append** or **Upsert** write
modes to define how data is written to BigQuery during an
incremental data transfer. The following sections describe the available write
modes.

#### Upsert write mode

The upsert write mode either updates a row or inserts a new row in your
destination table by checking for a primary key. You can specify a primary key
to let the ServiceNow connector determine what changes are needed to keep
your destination table up to date with your source table. If the specified
primary key is present in the destination BigQuery table during a data transfer, then the ServiceNow
connector updates that row with new data from the source table. If a primary key
is not present during a data transfer, then the ServiceNow connector
inserts a new row.

When you select the upsert mode, you must select a watermark column and a
primary key:

- A watermark column is required for the ServiceNow connector to track changes in the source table.

  Select a watermark column that updates every time a row is modified. We
  recommend columns similar to the `UPDATED_AT` or `LAST_MODIFIED` column.

<!-- -->

- The primary key can be one or more columns on your table that are required
  for the ServiceNow connector to determine if it
  needs to insert or update a row.

  Select columns that contain non-null values that are unique across all
  rows of the table. We recommend columns that include system-generated
  identifiers, unique reference codes (for example, auto-incrementing IDs), or
  immutable time-based sequence IDs.

  To prevent potential data loss or data corruption, the primary key columns
  that you select must have unique values. If you have doubts about the
  uniqueness of your chosen primary key column, then we recommend that you
  use the full ingestion instead.

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

Before you create a ServiceNow data transfer, do the following for
ServiceNow and BigQuery.

### ServiceNow prerequisites

- To access ServiceNow APIs, create [OAuth credentials](https://www.servicenow.com/docs/csh?topicname=t_CreateEndpointforExternalClients.html&version=latest).
- The following ServiceNow applications must all be enabled in the ServiceNow
  instance:

  - [Procurement](https://docs.servicenow.com/csh?topicname=t_ActivateProcurement.html&version=latest)
  - [Product Catalog](https://docs.servicenow.com/csh?topicname=c_ProductCatalog.html&version=latest)
  - [Contract Management](https://docs.servicenow.com/csh?topicname=c_ContractManagement.html&version=latest)
- To start a ServiceNow transfer, you must have the correct
  credentials to connect to the ServiceNow instance.

  - To obtain your credentials to a ServiceNow developer instance, login to the [ServiceNow developer
    portal](https://developer.servicenow.com/dev.do). You can use the username and password listed in the **Manage instance password** page. For information on resetting your ServiceNow password, see [Password Reset](https://www.servicenow.com/docs/csh?topicname=password-reset-landing-page.html&version=latest)
  - To obtain your credentials to a ServiceNow production or sub-production instance, contact your ServiceNow customer administrator to request the username and password.

### BigQuery prerequisites

- Complete all actions required to [enable the BigQuery Data Transfer Service](https://docs.cloud.google.com/bigquery/docs/enable-transfer-service).
- [Create a BigQuery dataset](https://docs.cloud.google.com/bigquery/docs/datasets) for storing the data.
- If you intend to set up transfer run notifications for Pub/Sub, ensure that you have the `pubsub.topics.setIamPolicy` Identity and Access Management (IAM) permission. If you only set up email notifications, Pub/Sub permissions aren't required. For more information, see [BigQuery Data Transfer Service run notifications](https://docs.cloud.google.com/bigquery/docs/transfer-run-notifications).

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

## Set up a ServiceNow data transfer

Add ServiceNow data into BigQuery by setting up a
transfer configuration using one of the following options:

### Console

1. Go to the Data transfers page in the Google Cloud console.

   [Go to Data transfers](https://console.cloud.google.com/bigquery/transfers)
2. Click **Create transfer**.

3. In the **Source type** section, for **Source** , select **ServiceNow**.

4. In the **Data source details** section, do the following:

   - (Optional) For **Network attachment** , select a network attachment from the drop-down menu, or click **Create Network Attachment** .
     - Select a network attachment to configure this data transfer to use a single, consistent IP address. You can use this option if your ServiceNow instance is configured to only accept traffic from specific IP addresses.
     - For more information about creating a network attachment, see [Configure connections with network attachments](https://docs.cloud.google.com/bigquery/docs/connections-with-network-attachment)
     - For more information about defining IP addresses in ServiceNow, see [Define allowed ServiceNow internal IP addresses](https://www.servicenow.com/docs/csh?topicname=sc-ip-addresses-access-allowlist.html&version=latest)
   - For **Instance ID** , enter the ServiceNow instance ID. You can get this from your ServiceNow URL---for example, `https://INSTANCE_ID.service-now.com`.
   - (Optional) For **ServiceNow Cloud Type** , select the cloud type for your ServiceNow account:
     - Select **Commercial** if your ServiceNow instance URL follows the pattern `https://INSTANCE_ID.service-now.com`. This is the default value.
     - Select **Government Community Cloud (GCC)** if your ServiceNow instance URL follows the pattern `https://INSTANCE_ID.servicenowservices.com`.
   - For **Username**, enter the ServiceNow username to use for the connection.
   - For **Password**, enter the ServiceNow password.
   - For **Client ID** , enter the client ID from your OAuth credentials. To generate credentials, see [Create OAuth Credentials](https://docs.oracle.com/cd/B13789_01/server.101/b10759/statements_9013.htm).
   - For **Client secret**, enter the client secret from your OAuth credentials.
   - For **Enable legacy mapping** , select **true** (default) to use the [legacy data type
     mapping](https://docs.cloud.google.com/bigquery/docs/servicenow-transfer#data_type_mapping). Select **false** to use the updated data type mapping. For more information about the data type mapping updates, see [March 16,
     2027](https://docs.cloud.google.com/bigquery/docs/transfer-changes#Mar16-servicenow).
   - For **Ingestion type** , select **Full** or **Incremental** .
     - If you select **Incremental** ([Preview](https://cloud.google.com/products#product-launch-stages)), for **Write mode** , select **Upsert** . For more information about write modes, see [Full or
       incremental
       transfers](https://docs.cloud.google.com/bigquery/docs/servicenow-transfer#full_or_incremental_transfers).
   - For **ServiceNow tables to transfer** , click **Browse** :
     - Select any objects to be transferred to the BigQuery destination dataset. You can also manually enter any objects to include in the data transfer in this field.
     - If you have selected **Upsert** as your incremental write mode, you must select a column as the watermark column, and then select one or more columns as the primary key.
   - For **Value type** , choose one of the following:
     - To transfer the values stored in the database, choose **Actual**.
     - To transfer the display values of the columns, choose **Display**.
5. In the **Destination settings** section, for **Dataset**, select the
   dataset that you created to store your data.

6. In the **Transfer config name** section, for **Display name**, enter a
   name for the data transfer.

7. In the **Schedule options** section, do the following:

   - In the **Repeat frequency** list, select an option to specify how often this data transfer runs. To specify a custom repeat frequency, select **Custom** . If you select **On-demand** , then this data transfer runs when you [manually trigger the transfer](https://docs.cloud.google.com/bigquery/docs/working-with-transfers#manually_trigger_a_transfer).
   - If applicable, select either **Start now** or **Start at set time**, and provide a start date and run time.
8. Optional: In the **Notification options** section, do the following:

   - To enable email notifications, click the **Email notification** toggle. When you enable this option, the transfer administrator receives an email notification when a transfer run fails.
   - To enable [Pub/Sub transfer run notifications](https://docs.cloud.google.com/bigquery/docs/transfer-run-notifications) for this data transfer, click the **Pub/Sub notifications** toggle. You can select your [topic](https://docs.cloud.google.com/pubsub/docs/overview#types) name, or you can click **Create a topic** to create one.
9. Click **Save**.

### bq

Enter the [`bq mk`](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#bq_mk) command
and supply the transfer creation flag, `--transfer_config`:

    bq mk
        --transfer_config
        --project_id=PROJECT_ID
        --data_source=DATA_SOURCE
        --display_name=DISPLAY_NAME
        --target_dataset=DATASET
        --params='PARAMETERS'

Replace the following:

- `PROJECT_ID` (optional): Your Google Cloud project ID. If a project ID isn't specified, the default project is used.
- `DATA_SOURCE`: the data source (for example, `servicenow`).
- `DISPLAY_NAME`: the display name for the transfer configuration. The data transfer name can be any value that lets you identify the transfer if you need to modify it later.
- `DATASET`: the target dataset for the transfer configuration.
- `PARAMETERS`: the parameters for the created transfer
  configuration in JSON format. For example:
  `--params='{"param":"param_value"}'`. The following are the parameters for
  a ServiceNow data transfer:

  | ServiceNow parameter | Required or optional | Description |
  |---|---|---|
  | `connector.networkAttachment` | Optional | Name of the Network attachment to use for ensuring connectivity to the ServiceNow Instance. |
  | `connector.instanceId` | Required | Instance ID of the ServiceNow instance |
  | `connector.authentication.username` | Required | Username for the ServiceNow instance of user. |
  | `connector.authentication.password` | Required | Password for the ServiceNow instance of user. |
  | `connector.authentication.oauth.clientId` | Required | Client ID for OAuth authentication with the ServiceNow instance. |
  | `connector.authentication.oauth.clientSecret` | Required | Client Secret for OAuth authentication with the ServiceNow instance. |
  | `connector.instanceCloudType` | Optional | Specify the cloud type of your ServiceNow account. Supported values are: - `COMMERCIAL_CLOUD`, if your ServiceNow instance URL follows the pattern `https://INSTANCE_ID.service-now.com` - `GOVERNMENT_COMMUNITY_CLOUD` if your ServiceNow instance URL follows the pattern `https://INSTANCE_ID.servicenowservices.com` |
  | `ingestionType` | Optional | Defines the method for transferring data from the source ServiceNow to the destination, determining whether a full dataset reload or an efficient incremental update is performed. |
  | `writeMode` | Optional | If using incremental ingestion, determines the synchronization strategy for incremental ingestion. This field is required for incremental transfers. Supported value is `WRITE_MODE_UPSERT`. |
  | `assets` | Required | List of the names of ServiceNow tables be transferred from ServiceNow as part of the transfer. |
  | `watermarkColumns` | Optional | If using incremental ingestion, source table field (typically datetime) used to track the last successful synchronization point, enabling the connector to efficiently query and transfer only the records that have been created or modified since that specific time. This field is required for incremental transfers. |
  | `primaryKeys` | Optional | If using incremental ingestion, the unique column or combination of columns used to uniquely identify each row in the source table. This field is required for incremental transfers. |
  | `valueType` | Optional | Controls how specific data types from ServiceNow are mapped to BigQuery data types. |
  | `connector.legacyMapping` | Required | Set to `true` (default) to use the [legacy data type mapping](https://docs.cloud.google.com/bigquery/docs/servicenow-transfer#data_type_mapping). Set to `false` to use the updated data type mapping. If you are making an incremental transfer, this value must be `false`. For more information about the data type mapping updates, see [March 16, 2027](https://docs.cloud.google.com/bigquery/docs/transfer-changes#Mar16-servicenow). |

  When specifying multiple assets during an incremental transfer, the values
  of the `watermarkColumns` and `primaryKeys` fields correspond to the
  position of values in the `assets` field. Ensure the order of tables and
  their respective columns is maintained consistently across all related
  configuration lists.

  For example, the following command creates a ServiceNow data transfer in the
  default project with all the required parameters:

        bq mk
          --transfer_config
          --target_dataset=mydataset
          --data_source=servicenow
          --display_name='My Transfer'
          --params='{"connector.authentication.oauth.clientId": "1234567890",
              "connector.authentication.oauth.clientSecret":"ABC12345",
              "connector.authentication.username":"user1",
              "connector.authentication.password":"abcdef1234",
              "connector.instanceId":"dev-instance",
              "connector.networkAttachment": "projects/dev-project1/regions/us-central1/networkattachments/na1"}'

  The following command creates an incremental ServiceNow data transfer in the
  default project, and uses the `UPSERT` write mode.

        bq mk
            --transfer_config
            --target_dataset=mydataset
            --data_source=servicenow
            --display_name='My Transfer'
            --params='{"assets": ["incident", "change_request"],
                "connector.authentication.oauth.clientId": "1234567890",
                "connector.authentication.oauth.clientSecret":"ABC12345",
                "connector.authentication.username":"user1",
                "connector.authentication.password":"abcdef1234",
                "connector.instanceId":"dev-instance",
                "ingestionType":"incremental",
                "writeMode":"WRITE_MODE_UPSERT",
                "watermarkColumns":["sys_updated_on","sys_updated_on"],
                "primaryKeys":[["sys_id"], ["sys_id"]]}'

### API

Use the [`projects.locations.transferConfigs.create`](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs/create)
method and supply an instance of the [`TransferConfig`](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs#TransferConfig)
resource.
When you save the transfer configuration, the ServiceNow connector automatically triggers a transfer run according to your schedule option. With every transfer run, the ServiceNow connector transfers all available data from ServiceNow into BigQuery.

<br />

To manually run a data transfer outside of your regular schedule, you can start
a [backfill run](https://docs.cloud.google.com/bigquery/docs/working-with-transfers#manually_trigger_a_transfer).

## Data type mapping

> [!NOTE]
> **Note:** On March 16, 2027, the ServiceNow connector will update some of its data type mapping. For more information, see [March 16, 2027](https://docs.cloud.google.com/bigquery/docs/transfer-changes#Mar16-servicenow).

The following table shows how data types are mapped in a
ServiceNow data transfer:

| ServiceNow data type | BigQuery data type | [Updated BigQuery data type](https://docs.cloud.google.com/bigquery/docs/transfer-changes#Mar16-servicenow) |
|---|---|---|
| `decimal` | `FLOAT64` |   |
| `integer` | `INTEGER` |   |
| `boolean` | `BOOLEAN` |   |
| `glide_date` | `DATE` |   |
| `glide_date_time` | `DATETIME` |   |
| `glide_list` | `STRING` | `ARRAY` |
| `glide_time` | `INT64` |   |
| `reference` | `STRING` |   |
| `currency` | `STRING` |   |
| `sys_class_name` | `STRING` |   |
| `domain_id` | `STRING` |   |
| `domain_path` | `STRING` |   |
| `guid` | `STRING` |   |
| `translated_html` | `STRING` |   |
| `journal` | `STRING` |   |
| `string` | `STRING` |   |
| `list` | `STRING` | `ARRAY` |

## Troubleshoot transfer issues

The following sections detail common problems when setting up a ServiceNow
data transfer.

For more information, see [Troubleshoot transfer configurations](https://docs.cloud.google.com/bigquery/docs/transfer-troubleshooting).

### Transfer fails due to ServiceNow enablement

An issue occurs causing data transfers to fail when the Procurement, Product
Catalog, or Contract Management applications aren't enabled in
ServiceNow. To fix it, enable all three applications:

- [Procurement](https://www.servicenow.com/docs/csh?topicname=t_ActivateProcurement.html&version=latest)
- [Product Catalog](https://www.servicenow.com/docs/csh?topicname=t_ActivateAProductCatalogItem.html&version=latest)
- [Contract Management](https://www.servicenow.com/docs/csh?topicname=c_ContractManagement.html&version=latest) (enabled by default)

### Issue occurs during transfer run

An issue occurs causing the transfer run to not be created as intended. To
resolve the issue, do the following:

- Check that the ServiceNow account credentials, such as **Username** , **Password** , **Client ID** , and **Client secret** values, are valid.
- Check that the Instance ID is the valid ID of your ServiceNow instance.

### Other errors

For information about other errors that occurred during a ServiceNow
data transfer, see [ServiceNow transfer issues](https://docs.cloud.google.com/bigquery/docs/transfer-troubleshooting#servicenow-issues)

## Pricing

For pricing information about ServiceNow transfers, see
[Data Transfer Service pricing](https://docs.cloud.google.com/bigquery/pricing#bqdts).

## What's next

- For an overview of BigQuery Data Transfer Service, see [Introduction to BigQuery Data Transfer Service](https://docs.cloud.google.com/bigquery/docs/dts-introduction).
- For information on using transfers including getting information about a transfer configuration, listing transfer configurations, and viewing a transfer's run history, see [Working with transfers](https://docs.cloud.google.com/bigquery/docs/working-with-transfers).
- Learn how to [load data with cross-cloud operations](https://docs.cloud.google.com/bigquery/docs/load-data-using-cross-cloud-transfer).