# Manage datasets

This document describes how to copy datasets, recreate datasets in another
location, secure datasets, delete datasets, and restore tables from deleted
datasets in BigQuery. For information about how to restore
(or *undelete* ) a deleted dataset, see
[Restore deleted datasets](https://docs.cloud.google.com/bigquery/docs/restore-deleted-datasets).

As a BigQuery administrator, you can organize and control access
to [tables](https://docs.cloud.google.com/bigquery/docs/tables) and [views](https://docs.cloud.google.com/bigquery/docs/views) that
analysts use. For more information about datasets,
see [Introduction to datasets](https://docs.cloud.google.com/bigquery/docs/datasets-intro).

You cannot change the name of an existing dataset or relocate a dataset after
it's created.
As a workaround for changing the dataset name, you can [copy a dataset](https://docs.cloud.google.com/bigquery/docs/managing-datasets#copy-datasets)
and change the destination dataset's name. To relocate a dataset, you can follow
one of the following methods:

- [Recreate a dataset in another location](https://docs.cloud.google.com/bigquery/docs/managing-datasets#recreate-dataset).
- [Make a copy of the dataset](https://docs.cloud.google.com/bigquery/docs/managing-datasets#copy-datasets).

## Required roles

This section describes the roles and permissions that you need to manage
datasets. If your source or destination dataset is in the
same project as the one you are using to copy, then you don't need extra
permissions or roles on that dataset.

### Copy a dataset

Grant these roles to copy a dataset. Copying datasets is in
([Beta](https://cloud.google.com/products/#product-launch-stages)).


To get the permissions that
you need to copy datasets,

ask your administrator to grant you the
following IAM roles:

- BigQuery Admin (`roles/bigquery.admin`) - the destination project
- BigQuery Data Viewer (`roles/bigquery.dataViewer`) - the source dataset
- BigQuery Data Editor (`roles/bigquery.dataEditor`) - the destination dataset


For more information about granting roles, see [Manage access to projects, folders, and organizations](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).


These predefined roles contain

the permissions required to copy datasets. To see the exact permissions that are
required, expand the **Required permissions** section:


#### Required permissions

The following permissions are required to copy datasets:

- `bigquery.transfers.update` on the destination project
- `bigquery.jobs.create` on the destination project
- `bigquery.datasets.get` on the source and destination dataset
- `bigquery.tables.list` on the source and destination dataset
- `bigquery.datasets.update` on the destination dataset
- `bigquery.tables.create` on the destination dataset


You might also be able to get
these permissions
with [custom roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles) or
other [predefined roles](https://docs.cloud.google.com/iam/docs/roles-overview#predefined).

### Delete a dataset

Grant these roles to delete a dataset.


To get the permissions that
you need to delete datasets,

ask your administrator to grant you the
[BigQuery Data Owner](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquery#bigquery.dataOwner) (`roles/bigquery.dataOwner`) IAM role on the project.


For more information about granting roles, see [Manage access to projects, folders, and organizations](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).


This predefined role contains

the permissions required to delete datasets. To see the exact permissions that are
required, expand the **Required permissions** section:


#### Required permissions

The following permissions are required to delete datasets:

- `bigquery.datasets.delete` on the project
- `bigquery.tables.delete` on the project


You might also be able to get
these permissions
with [custom roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles) or
other [predefined roles](https://docs.cloud.google.com/iam/docs/roles-overview#predefined).

## Copy datasets

> [!WARNING]
>
> **Beta**
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

You can copy a dataset, including partitioned data within a region or across
regions, without extracting, moving, or reloading data into BigQuery.
BigQuery uses the [BigQuery Data Transfer Service](https://docs.cloud.google.com/bigquery/docs/dts-introduction)
in the backend to copy datasets. For location considerations when you transfer
data, see [Data location and transfers](https://docs.cloud.google.com/bigquery/docs/dts-locations).

For each dataset copy configuration, you can have one transfer run active at a
time. Additional transfer runs are queued. If you are using the Google Cloud console,
you can schedule recurring copies, and configure an email or Pub/Sub
notifications with the BigQuery Data Transfer Service.

### Limitations

The following limitations apply when you copy datasets:

- You can't copy the following resources from a source dataset:

  - Views.
  - Routines, including UDFs.
  - External tables.
  - [Change data capture (CDC) tables](https://docs.cloud.google.com/bigquery/docs/change-data-capture) if the copy job is across regions. Copying CDC tables within the same region is supported.
  - Cross-region table copy job is not supported for tables encrypted with
    [customer-managed encrypted keys (CMEK)](https://docs.cloud.google.com/bigquery/docs/customer-managed-encryption)
    when the destination dataset is not encrypted with CMEK and there is no
    CMEK provided. Copying tables with default encryption across regions is
    supported.

    You can copy all encrypted tables within the same region, including
    tables encrypted with CMEK.
- You can't use the following resources as destination datasets for copy jobs:

  - Write-optimized storage.
  - Dataset encrypted with CMEK if the copy job is across regions and the
    source table is not encrypted with CMEK.

    However, tables encrypted with CMEK are allowed as destination tables
    when copying within the same region.
- The minimum frequency between copy jobs is 12 hours.

- Appending data to a partitioned or non-partitioned table in the destination
  dataset isn't supported. If there are no changes in the source table, the
  table is skipped. If the source table is updated, the destination table is
  completely truncated and replaced.

- If a table exists in the source dataset and the destination dataset, and the
  source table has not changed since the last successful copy, it's skipped.
  The source table is skipped even if the **Overwrite destination tables**
  checkbox is selected.

- When truncating tables in the destination dataset, the dataset copy job
  doesn't detect any changes made to resources in
  the destination dataset before it begins the copy job. The dataset copy job
  overwrites all of the data in the destination dataset, including both the
  tables and schema.

- The destination table might not reflect changes made to the source tables
  after a copy job starts.

- Copying a dataset is not supported in
  [BigQuery Omni regions](https://docs.cloud.google.com/bigquery/docs/omni-introduction#locations).

- To copy a dataset to a project in another
  [VPC Service Controls service perimeter](https://docs.cloud.google.com/vpc-service-controls/docs/service-perimeters),
  you need to set the following egress rules:

  - In the destination project's VPC Service Controls service perimeter configuration,
    the IAM principal must have the following methods:

    - `bigquery.datasets.get`
    - `bigquery.tables.list`
    - `bigquery.tables.get`,
    - `bigquery.tables.getData`
  - In the source project's VPC Service Controls service perimeter
    configuration, the IAM principal being used must have the method set to
    `All Methods`.

- If you try to update a dataset copy transfer configuration you don't own,
  the update might fail with the following error message:

  `Cannot modify restricted parameters without taking ownership of the transfer configuration.`

  The owner of the dataset copy is the user associated with the dataset copy
  or the user who has access to the service account associated with the dataset copy. The associated user can be seen in the [configuration details](https://docs.cloud.google.com/bigquery/docs/working-with-transfers#get_transfer_details) of the
  dataset copy. For information on how to update the dataset copy to take
  ownership, see
  [Update credentials](https://docs.cloud.google.com/bigquery/docs/working-with-transfers#update_credentials).
  To grant users access to a service account, you must have the
  [Service Account user role](https://docs.cloud.google.com/iam/docs/service-account-permissions#user-role).

  The owner restricted parameters for dataset copies are:
  - Source project
  - Source dataset
  - Destination dataset
  - Overwrite destination table setting
- All [cross-region table copy limitations](https://docs.cloud.google.com/bigquery/docs/managing-tables#limitations) apply.

### Copy a dataset

Select one of the following options:

### Console

1. [Enable the BigQuery Data Transfer Service](https://docs.cloud.google.com/bigquery/docs/enable-transfer-service)
   for your destination dataset.

   [Enable the BigQuery Data Transfer Service API](https://console.cloud.google.com/apis/library/bigquerydatatransfer.googleapis.com)
2. Ensure that you have the [required roles](https://docs.cloud.google.com/bigquery/docs/managing-datasets#required-roles).

   If you intend to set up transfer run notifications for Pub/Sub
   (**Option 2** later in these steps), then you must have the `pubsub.topics.setIamPolicy`
   permission.

   If you only set up email notifications, then Pub/Sub
   permissions are not required. For more information, see the BigQuery Data Transfer Service
   [run notifications](https://docs.cloud.google.com/bigquery/docs/transfer-run-notifications).
3. [Create a BigQuery dataset](https://docs.cloud.google.com/bigquery/docs/datasets)
   in the same region or a different region from your source dataset.

**Option 1: Use the BigQuery copy function**

To create a one-time transfer, use the BigQuery copy function:

1. Go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)

   If you don't see the left pane, click **Expand left pane** to open the pane.
3. In the **Explorer** pane, expand your project, click **Datasets**, and then select a dataset.

4. In the **Dataset info** section, click **Copy**,
   and then do the following:

   1. In the **Dataset** field, either create a new dataset or select an
      existing dataset ID from the list.

      Dataset names within a project must be unique. The project and
      dataset can be in different regions, but not all regions
      are supported for cross-region dataset copying.

      In the **Location** field, the location of the source dataset is
      displayed.
   2. Optional: To overwrite both the data and schema of the destination tables
      with the source tables, select the **Overwrite destination tables**
      checkbox. Both the source and destination tables must have the same
      partitioning schema.

   3. To copy the dataset, click **Copy**.

**Option 2: Use the BigQuery Data Transfer Service**

To schedule recurring copies and configure email or Pub/Sub
notifications, use the BigQuery Data Transfer Service in the Google Cloud console of the
destination project:

1. Go to the **Data transfers** page.

   [Go to Data transfers](https://console.cloud.google.com/bigquery/transfers)
2. Click **Create a transfer**.

3. In the **Source** list, select **Dataset Copy**.

4. In the **Display name** field, enter a name for your transfer run.

5. In the **Schedule options** section, do the following:

   1. For **Repeat frequency**, choose an option for how often to run the transfer:

      If you select **Custom** , enter a custom frequency---for example,
      `every day 00:00`. For more information, see [Formatting the schedule](https://docs.cloud.google.com/appengine/docs/flexible/python/scheduling-jobs-with-cron-yaml#formatting_the_schedule).

      > [!NOTE]
      > **Note:** The minimum frequency between copy jobs is 12 hours.

   2. For **Start date and run time** , enter the date and time to start
      the transfer. If you choose **Start now**, this option is disabled.

6. In the **Destination settings** section, select a destination dataset to
   store your transfer data. You can also click **CREATE NEW DATASET** to
   create a new dataset before you select it for this transfer.

7. In the **Data source details** section, enter the following information:

   1. For **Source dataset**, enter the dataset ID that you want to copy.
   2. For **Source project**, enter the project ID of your source dataset.
8. To overwrite both the data and schema of the destination tables with the
   source tables, select the **Overwrite destination tables** checkbox. Both
   the source and destination tables must have the same partitioning schema.

9. In the **Service Account** menu, select a [service account](https://docs.cloud.google.com/iam/docs/service-account-overview)
   from the service accounts associated with your
   Google Cloud project. You can associate a service account with
   your transfer instead of using your user credentials. For more
   information about using service accounts with data transfers, see
   [Use service accounts](https://docs.cloud.google.com/bigquery/docs/use-service-accounts).

   - If you signed in with a [federated identity](https://docs.cloud.google.com/iam/docs/workforce-identity-federation), then a service account is required to create a transfer. If you signed in with a [Google Account](https://docs.cloud.google.com/iam/docs/principals-overview#google-account), then a service account for the transfer is optional.
   - The service account must have the [required roles](https://docs.cloud.google.com/bigquery/docs/managing-datasets#required-roles).
10. Optional: In the **Notification options** section, do the following:

    - To enable email notifications, click the toggle. When you enable this option, the owner of the transfer configuration receives an email notification when a transfer run fails.
    - To enable Pub/Sub notifications, click the toggle, and then either select a [topic](https://docs.cloud.google.com/pubsub/docs/overview#types) name from the list or click **Create a topic** . This option configures Pub/Sub run [notifications](https://docs.cloud.google.com/bigquery/docs/transfer-run-notifications) for your transfer.
11. Click **Save**.

### bq

1. [Enable the BigQuery Data Transfer Service](https://docs.cloud.google.com/bigquery/docs/enable-transfer-service)
   for your destination dataset.

2. Ensure that you have the [required roles](https://docs.cloud.google.com/bigquery/docs/managing-datasets#required-roles).

3. To [create a BigQuery dataset](https://docs.cloud.google.com/bigquery/docs/datasets),
   use the [`bq mk` command](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#bq_mk)
   with the dataset creation flag `--dataset` and the `location` flag:

   ```bash
   bq mk \
     --dataset \
     --location=LOCATION \
     PROJECT:DATASET
   ```

   Replace the following:
   - `LOCATION`: the location where you want to copy the dataset
   - `PROJECT`: the project ID of your target dataset
   - `DATASET`: the name of the target dataset
4. To copy a dataset, use the `bq mk` command with the transfer creation flag
   [`--transfer_config`](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#mk-transfer-config)
   and the `--data_source` flag.
   You must set the `--data_source` flag to `cross_region_copy`. For a
   complete list of valid values for the `--data_source` flag,
   see the [transfer-config flags](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#mk-transfer-config)
   in the bq command-line tool reference.

   ```bash
   bq mk \
     --transfer_config \
     --project_id=PROJECT \
     --data_source=cross_region_copy \
     --target_dataset=DATASET \
     --display_name=NAME \
    --service_account_name=SERCICE_ACCOUNT \
     --params='PARAMETERS'
   ```

   Replace the following:
   - `NAME`: the display name for the copy job or the
     transfer configuration

   - `SERVICE_ACCOUNT`: the service account name used
     to authenticate your transfer. The service account should
     be owned by the same `project_id` used to create the transfer and it
     should have all of the [required permissions](https://docs.cloud.google.com/bigquery/docs/managing-datasets#required-roles).

   - `PARAMETERS`: the parameters for the
     transfer configuration in the JSON format

     Parameters for a dataset copy configuration include the following:
     - `source_dataset_id`: the ID of the source dataset that you want to copy
     - `source_project_id`: the ID of the project that your source dataset is in
     - `overwrite_destination_table`: an optional flag that lets you truncate the tables of a previous copy and refresh all the data

     Both the source and destination tables must have the same
     partitioning schema.

   The following examples show the formatting of the parameters, based
   on your system's environment:
   - **Linux:** use single quotes to enclose the JSON string--for
     example:

     ```
     '{"source_dataset_id":"mydataset","source_project_id":"mysourceproject","overwrite_destination_table":"true"}'
     ```
   - **Windows command line:** use double quotes to enclose the
     JSON string, and escape double quotes in the string
     with a backslash--for example:

     ```
     "{\"source_dataset_id\":\"mydataset\",\"source_project_id\":\"mysourceproject\",\"overwrite_destination_table\":\"true\"}"
     ```
   - **PowerShell:** use single quotes to enclose the JSON string,
     and escape double quotes in the string with a
     backslash--for example:

     ```
     '{\"source_dataset_id\":\"mydataset\",\"source_project_id\":\"mysourceproject\",\"overwrite_destination_table\":\"true\"}'
     ```

   For example, the following command creates a dataset copy configuration
   that's named `My Transfer` with a target dataset that's named `mydataset`
   and a project with the ID of `myproject`.

   ```bash
   bq mk \
     --transfer_config \
     --project_id=myproject \
     --data_source=cross_region_copy \
     --target_dataset=mydataset \
     --display_name='My Transfer' \
     --params='{
         "source_dataset_id":"123_demo_eu",
         "source_project_id":"mysourceproject",
         "overwrite_destination_table":"true"
         }'
   ```

### API

1. [Enable the BigQuery Data Transfer Service](https://docs.cloud.google.com/bigquery/docs/enable-transfer-service)
   for your destination dataset.

2. Ensure that you have the [required roles](https://docs.cloud.google.com/bigquery/docs/managing-datasets#required-roles).

3. To [create a BigQuery dataset](https://docs.cloud.google.com/bigquery/docs/datasets),
   call the [`datasets.insert`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets/insert)
   method with a defined [dataset resource](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets).

4. To copy a dataset, use the [`projects.locations.transferConfigs.create`](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs/create) method and supply an instance of
   the [`TransferConfig`](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs#TransferConfig) resource.

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

    // Sample to copy dataset from another gcp project
    public class CopyDataset {

      public static void main(String[] args) throws IOException {
        // TODO(developer): Replace these variables before running the sample.
        final String destinationProjectId = "MY_DESTINATION_PROJECT_ID";
        final String destinationDatasetId = "MY_DESTINATION_DATASET_ID";
        final String sourceProjectId = "MY_SOURCE_PROJECT_ID";
        final String sourceDatasetId = "MY_SOURCE_DATASET_ID";
        Map<String, Value> params = new HashMap<>();
        params.put("source_project_id", https://docs.cloud.google.com/java/docs/reference/protobuf/latest/com.google.protobuf.Value.html.newBuilder().setStringValue(sourceProjectId).build());
        params.put("source_dataset_id", https://docs.cloud.google.com/java/docs/reference/protobuf/latest/com.google.protobuf.Value.html.newBuilder().setStringValue(sourceDatasetId).build());
        https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.TransferConfig.html transferConfig =
            https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.TransferConfig.html.newBuilder()
                .setDestinationDatasetId(destinationDatasetId)
                .setDisplayName("Your Dataset Copy Name")
                .setDataSourceId("cross_region_copy")
                .setParams(https://docs.cloud.google.com/java/docs/reference/protobuf/latest/com.google.protobuf.Struct.html.newBuilder().https://docs.cloud.google.com/java/docs/reference/protobuf/latest/com.google.protobuf.Struct.Builder.html#com_google_protobuf_Struct_Builder_putAllFields_java_util_Map_java_lang_String_com_google_protobuf_Value__(params).build())
                .setSchedule("every 24 hours")
                .build();
        copyDataset(destinationProjectId, transferConfig);
      }

      public static void copyDataset(String projectId, https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.TransferConfig.html transferConfig)
          throws IOException {
        try (https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.DataTransferServiceClient.html dataTransferServiceClient = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.DataTransferServiceClient.html.create()) {
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.ProjectName.html parent = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.ProjectName.html.of(projectId);
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.CreateTransferConfigRequest.html request =
              https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.CreateTransferConfigRequest.html.newBuilder()
                  .setParent(parent.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.ProjectName.html#com_google_cloud_bigquery_datatransfer_v1_ProjectName_toString__())
                  .setTransferConfig(transferConfig)
                  .build();
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.TransferConfig.html config = dataTransferServiceClient.createTransferConfig(request);
          System.out.println("Copy dataset created successfully :" + config.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.TransferConfig.html#com_google_cloud_bigquery_datatransfer_v1_TransferConfig_getName__());
        } catch (https://docs.cloud.google.com/java/docs/reference/gax/latest/com.google.api.gax.rpc.ApiException.html ex) {
          System.out.print("Copy dataset was not created." + ex.toString());
        }
      }
    }

### Python


Before trying this sample, follow the Python setup instructions in the
[BigQuery quickstart using
client libraries](https://docs.cloud.google.com/bigquery/docs/quickstarts/quickstart-client-libraries).


For more information, see the
[BigQuery Python API
reference documentation](https://docs.cloud.google.com/python/docs/reference/bigquery/latest).


To authenticate to BigQuery, set up Application Default Credentials.
For more information, see

[Set up authentication for client libraries](https://docs.cloud.google.com/bigquery/docs/authentication#client-libs).

Install the [Python client for the BigQuery Data Transfer API](https://docs.cloud.google.com/python/docs/reference/bigquerydatatransfer/latest) with `pip install google-cloud-bigquery-datatransfer`. Then create a transfer configuration to copy the dataset.

    from google.cloud import bigquery_datatransfer

    transfer_client = bigquery_datatransfer.https://docs.cloud.google.com/python/docs/reference/bigquerydatatransfer/latest/google.cloud.bigquery_datatransfer_v1.services.data_transfer_service.DataTransferServiceClient.html()

    destination_project_id = "my-destination-project"
    destination_dataset_id = "my_destination_dataset"
    source_project_id = "my-source-project"
    source_dataset_id = "my_source_dataset"
    transfer_config = bigquery_datatransfer.https://docs.cloud.google.com/python/docs/reference/bigquerydatatransfer/latest/google.cloud.bigquery_datatransfer_v1.types.TransferConfig.html(
        destination_dataset_id=destination_dataset_id,
        display_name="Your Dataset Copy Name",
        data_source_id="cross_region_copy",
        params={
            "source_project_id": source_project_id,
            "source_dataset_id": source_dataset_id,
        },
        schedule="every 24 hours",
    )
    transfer_config = transfer_client.https://docs.cloud.google.com/python/docs/reference/bigquerydatatransfer/latest/google.cloud.bigquery_datatransfer_v1.services.data_transfer_service.DataTransferServiceClient.html#google_cloud_bigquery_datatransfer_v1_services_data_transfer_service_DataTransferServiceClient_create_transfer_config(
        parent=transfer_client.https://docs.cloud.google.com/python/docs/reference/bigquerydatatransfer/latest/google.cloud.bigquery_datatransfer_v1.services.data_transfer_service.DataTransferServiceClient.html#google_cloud_bigquery_datatransfer_v1_services_data_transfer_service_DataTransferServiceClient_common_project_path(destination_project_id),
        transfer_config=transfer_config,
    )
    print(f"Created transfer config: {transfer_config.name}")

<br />

To avoid additional storage costs, consider [deleting the prior dataset](https://docs.cloud.google.com/bigquery/docs/managing-datasets#delete-datasets).

### View dataset copy jobs

To see the status and view details of a dataset copy job in the
Google Cloud console, do the following:

1. In the Google Cloud console, go to the **Data transfers** page.

   [Go to Data transfers](https://console.cloud.google.com/bigquery/transfers)
2. Select a transfer for which you want to view the transfer details, and then do
   the following:

   1. On the **Transfer details** page, select a transfer run.

   2. To refresh, click **Refresh**.

## Recreate datasets in another location

To manually move a dataset from one location to another, follow these steps:

1. [Export the data](https://docs.cloud.google.com/bigquery/docs/exporting-data) from your BigQuery
   tables to a Cloud Storage bucket.

   There are no charges for exporting data from BigQuery, but you do incur charges
   for [storing the exported data](https://docs.cloud.google.com/storage/pricing#storage-pricing) in
   Cloud Storage. BigQuery exports are subject to the limits on
   [extract jobs](https://docs.cloud.google.com/bigquery/quotas#export_jobs).
2. Copy or move the data from your export Cloud Storage bucket to a new bucket you created
   in the destination location. For example, if you are moving your data from the `US`
   multi-region to the `asia-northeast1` Tokyo region, you would transfer the data to a bucket
   that you created in Tokyo. For information about transferring Cloud Storage objects, see
   [Copy, rename, and move objects](https://docs.cloud.google.com/storage/docs/copying-renaming-moving-objects)
   in the Cloud Storage documentation.

   Transferring data between regions incurs
   [network egress charges](https://docs.cloud.google.com/storage/pricing#network-pricing) in Cloud Storage.
3. Create a new BigQuery
   dataset in the new location, and then load your data from the Cloud Storage bucket
   into the new dataset.

   You are not charged for loading the data into BigQuery, but you will incur
   charges for storing the data in Cloud Storage until you delete the data or the bucket. You
   are also charged for storing the data in BigQuery after it is loaded. Loading
   data into BigQuery is subject to the
   [load jobs](https://docs.cloud.google.com/bigquery/quotas#load_jobs) limits.

You can also use [Managed Service for Apache Airflow](https://cloud.google.com/blog/products/data-analytics/how-to-transfer-bigquery-tables-between-locations-with-cloud-composer) to move and copy large datasets programmatically.

For more information about using Cloud Storage to store and move large datasets, see
[Use Cloud Storage with big data](https://docs.cloud.google.com/storage/docs/working-with-big-data).

## Secure datasets

To control access to datasets in BigQuery, see
[Controlling access to datasets](https://docs.cloud.google.com/bigquery/docs/control-access-to-resources-iam).
For information about data encryption, see [Encryption at rest](https://docs.cloud.google.com/bigquery/docs/encryption-at-rest).

## Delete datasets

When you delete a dataset by using the Google Cloud console, tables and views
in the dataset, including their data, are deleted automatically. However, when
using any other method, you must either empty the dataset first or specify
corresponding flags, parameters or keywords that force removal of the dataset
contents.

If you attempt to delete a non-empty dataset without the proper
flags or parameters, the operation fails with the following error:
`Dataset project:dataset is still in use`.

Deleting a dataset creates one [audit log](https://docs.cloud.google.com/bigquery/docs/introduction-audit-workloads)
entry for the dataset deletion. It doesn't create separate log entries for each
deleted table within the dataset.

To delete a dataset, select one of the following options:

### Console

1. Go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)
3. In the **Explorer** pane, expand your project, click **Datasets**, and then click the dataset.

4. In the details pane, click **Delete**.

5. In the **Delete dataset** dialog, type `delete` into the field, and then
   click **Delete**.

> [!NOTE]
> **Note:** When you delete a dataset using the Google Cloud console, the tables are automatically removed.

### SQL

To delete a dataset, use the
[`DROP SCHEMA` DDL statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#drop_schema_statement).

The following example deletes a dataset named `mydataset`:


1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, enter the following statement:

   ```googlesql
   DROP SCHEMA IF EXISTS mydataset;
   ```


   By default, this only works to delete an empty dataset.
   To delete a dataset and all of its contents, use the `CASCADE`
   keyword:

   ```googlesql
   DROP SCHEMA IF EXISTS mydataset CASCADE;
   ```

   <br />

3. Click **Run**.

   <br />

For more information about how to run queries, see [Run an interactive query](https://docs.cloud.google.com/bigquery/docs/running-queries#queries).

### bq

Use the [`bq rm` command](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#bq_rm)
with the `--dataset` or `-d` flag, which is optional.
If your dataset contains tables, you must use the `-r` flag to
remove all tables in the dataset. If you use the `-r` flag, then you can omit
the `--dataset` or `-d` flag.

After you run the command, the system asks for confirmation. You can use the
`-f` flag to skip the confirmation.

If you are deleting a table in a project other than your default project,
add the project ID to the dataset name in the following format:
`PROJECT_ID:DATASET`.

```bash
bq rm -r -f -d PROJECT_ID:DATASET
```

Replace the following:

- `PROJECT_ID`: your project ID
- `DATASET`: the name of the dataset that you're deleting

**Examples:**

Enter the following command to remove a dataset that's named `mydataset` and all
the tables in it from your default project. The command uses the
`-d` flag.

```
bq rm -r -d mydataset
```

When prompted, type `y` and press enter.

Enter the following command to remove `mydataset` and all the tables in it
from `myotherproject`. The command does not use the optional `-d` flag.
The `-f` flag is used to skip confirmation.

```
bq rm -r -f myotherproject:mydataset
```

You can use the `bq ls` command to confirm that the dataset was deleted.

### API

Call the
[`datasets.delete`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets/delete) method
to delete the dataset and set the `deleteContents` parameter to `true` to
delete the tables in it.

### C#

The following code sample deletes an empty dataset.


Before trying this sample, follow the C# setup instructions in the
[BigQuery quickstart using
client libraries](https://docs.cloud.google.com/bigquery/docs/quickstarts/quickstart-client-libraries).


For more information, see the
[BigQuery C# API
reference documentation](https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest).


To authenticate to BigQuery, set up Application Default Credentials.
For more information, see

[Set up authentication for client libraries](https://docs.cloud.google.com/bigquery/docs/authentication#client-libs).

Install the [Python client for the BigQuery Data Transfer API](https://docs.cloud.google.com/python/docs/reference/bigquerydatatransfer/latest) with `pip install google-cloud-bigquery-datatransfer`. Then create a transfer configuration to copy the dataset.


    using https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.html;
    using System;

    public class BigQueryDeleteDataset
    {
        public void DeleteDataset(
            string projectId = "your-project-id",
            string datasetId = "your_empty_dataset"
        )
        {
            https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryClient.html client = https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryClient.html.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryClient.html#Google_Cloud_BigQuery_V2_BigQueryClient_Create_System_String_Google_Apis_Auth_OAuth2_GoogleCredential_(projectId);
            // Delete a dataset that does not contain any tables
            client.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryClient.html#Google_Cloud_BigQuery_V2_BigQueryClient_DeleteDataset_Google_Apis_Bigquery_v2_Data_DatasetReference_Google_Cloud_BigQuery_V2_DeleteDatasetOptions_(datasetId: datasetId);
            Console.WriteLine($"Dataset {datasetId} deleted.");
        }
    }

The following code sample deletes a dataset and all of its contents:

    // Copyright(c) 2018 Google LLC
    //
    // Licensed under the Apache License, Version 2.0 (the "License"); you may not
    // use this file except in compliance with the License. You may obtain a copy of
    // the License at
    //
    // http://www.apache.org/licenses/LICENSE-2.0
    //
    // Unless required by applicable law or agreed to in writing, software
    // distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
    // WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
    // License for the specific language governing permissions and limitations under
    // the License.
    //

    using https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.html;
    using System;

    public class BigQueryDeleteDatasetAndContents
    {
        public void DeleteDatasetAndContents(
            string projectId = "your-project-id",
            string datasetId = "your_dataset_with_tables"
        )
        {
            https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryClient.html client = https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryClient.html.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryClient.html#Google_Cloud_BigQuery_V2_BigQueryClient_Create_System_String_Google_Apis_Auth_OAuth2_GoogleCredential_(projectId);
            // Use the DeleteDatasetOptions to delete a dataset and its contents
            client.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.BigQueryClient.html#Google_Cloud_BigQuery_V2_BigQueryClient_DeleteDataset_Google_Apis_Bigquery_v2_Data_DatasetReference_Google_Cloud_BigQuery_V2_DeleteDatasetOptions_(
                datasetId: datasetId,
                options: new https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.V2/latest/Google.Cloud.BigQuery.V2.DeleteDatasetOptions.html() { DeleteContents = true }
            );
            Console.WriteLine($"Dataset {datasetId} and contents deleted.");
        }
    }

### Go


Before trying this sample, follow the Go setup instructions in the
[BigQuery quickstart using
client libraries](https://docs.cloud.google.com/bigquery/docs/quickstarts/quickstart-client-libraries).


For more information, see the
[BigQuery Go API
reference documentation](https://godoc.org/cloud.google.com/go/bigquery).


To authenticate to BigQuery, set up Application Default Credentials.
For more information, see

[Set up authentication for client libraries](https://docs.cloud.google.com/bigquery/docs/authentication#client-libs).

Install the [Python client for the BigQuery Data Transfer API](https://docs.cloud.google.com/python/docs/reference/bigquerydatatransfer/latest) with `pip install google-cloud-bigquery-datatransfer`. Then create a transfer configuration to copy the dataset.

    import (
    	"context"
    	"fmt"

    	"cloud.google.com/go/bigquery"
    )

    // deleteDataset demonstrates the deletion of an empty dataset.
    func deleteDataset(projectID, datasetID string) error {
    	// projectID := "my-project-id"
    	// datasetID := "mydataset"
    	ctx := context.Background()

    	client, err := bigquery.NewClient(ctx, projectID)
    	if err != nil {
    		return fmt.Errorf("bigquery.NewClient: %v", err)
    	}
    	defer client.Close()

    	// To recursively delete a dataset and contents, use DeleteWithContents.
    	if err := client.Dataset(datasetID).Delete(ctx); err != nil {
    		return fmt.Errorf("Delete: %v", err)
    	}
    	return nil
    }

### Java

The following code sample deletes an empty dataset.


Before trying this sample, follow the Java setup instructions in the
[BigQuery quickstart using
client libraries](https://docs.cloud.google.com/bigquery/docs/quickstarts/quickstart-client-libraries).


For more information, see the
[BigQuery Java API
reference documentation](https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/overview).


To authenticate to BigQuery, set up Application Default Credentials.
For more information, see

[Set up authentication for client libraries](https://docs.cloud.google.com/bigquery/docs/authentication#client-libs).

Install the [Python client for the BigQuery Data Transfer API](https://docs.cloud.google.com/python/docs/reference/bigquerydatatransfer/latest) with `pip install google-cloud-bigquery-datatransfer`. Then create a transfer configuration to copy the dataset.

    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.DatasetDeleteOption.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryException.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryOptions.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.DatasetId.html;

    public class DeleteDataset {

      public static void runDeleteDataset() {
        // TODO(developer): Replace these variables before running the sample.
        String projectId = "MY_PROJECT_ID";
        String datasetName = "MY_DATASET_NAME";
        deleteDataset(projectId, datasetName);
      }

      public static void deleteDataset(String projectId, String datasetName) {
        try {
          // Initialize client that will be used to send requests. This client only needs to be created
          // once, and can be reused for multiple requests.
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html bigquery = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryOptions.html.getDefaultInstance().getService();

          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.DatasetId.html datasetId = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.DatasetId.html.of(projectId, datasetName);
          boolean success = bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html#com_google_cloud_bigquery_BigQuery_delete_com_google_cloud_bigquery_DatasetId_com_google_cloud_bigquery_BigQuery_DatasetDeleteOption____(datasetId, DatasetDeleteOption.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html#com_google_cloud_bigquery_BigQuery_delete_com_google_cloud_bigquery_DatasetId_com_google_cloud_bigquery_BigQuery_DatasetDeleteOption____Contents());
          if (success) {
            System.out.println("Dataset deleted successfully");
          } else {
            System.out.println("Dataset was not found");
          }
        } catch (https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryException.html e) {
          System.out.println("Dataset was not deleted. \n" + e.toString());
        }
      }
    }

The following code sample deletes a dataset and all of its contents:

    /*
     * Copyright 2020 Google LLC
     *
     * Licensed under the Apache License, Version 2.0 (the "License");
     * you may not use this file except in compliance with the License.
     * You may obtain a copy of the License at
     *
     * http://www.apache.org/licenses/LICENSE-2.0
     *
     * Unless required by applicable law or agreed to in writing, software
     * distributed under the License is distributed on an "AS IS" BASIS,
     * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
     * See the License for the specific language governing permissions and
     * limitations under the License.
     */

    package com.example.bigquery;

    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryException.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryOptions.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.DatasetId.html;

    // Sample to delete dataset with contents.
    public class DeleteDatasetAndContents {

      public static void main(String[] args) {
        // TODO(developer): Replace these variables before running the sample.
        String projectId = "MY_PROJECT_ID";
        String datasetName = "MY_DATASET_NAME";
        deleteDatasetAndContents(projectId, datasetName);
      }

      public static void deleteDatasetAndContents(String projectId, String datasetName) {
        try {
          // Initialize client that will be used to send requests. This client only needs to be created
          // once, and can be reused for multiple requests.
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html bigquery = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryOptions.html.getDefaultInstance().getService();

          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.DatasetId.html datasetId = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.DatasetId.html.of(projectId, datasetName);
          // Use the force parameter to delete a dataset and its contents
          boolean success = bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html#com_google_cloud_bigquery_BigQuery_delete_com_google_cloud_bigquery_DatasetId_com_google_cloud_bigquery_BigQuery_DatasetDeleteOption____(datasetId, BigQuery.DatasetDeleteOption.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html#com_google_cloud_bigquery_BigQuery_delete_com_google_cloud_bigquery_DatasetId_com_google_cloud_bigquery_BigQuery_DatasetDeleteOption____Contents());
          if (success) {
            System.out.println("Dataset deleted with contents successfully");
          } else {
            System.out.println("Dataset was not found");
          }
        } catch (https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryException.html e) {
          System.out.println("Dataset was not deleted with contents. \n" + e.toString());
        }
      }
    }

### Node.js


Before trying this sample, follow the Node.js setup instructions in the
[BigQuery quickstart using
client libraries](https://docs.cloud.google.com/bigquery/docs/quickstarts/quickstart-client-libraries).


For more information, see the
[BigQuery Node.js API
reference documentation](https://googleapis.dev/nodejs/bigquery/latest/index.html).


To authenticate to BigQuery, set up Application Default Credentials.
For more information, see

[Set up authentication for client libraries](https://docs.cloud.google.com/bigquery/docs/authentication#client-libs).

Install the [Python client for the BigQuery Data Transfer API](https://docs.cloud.google.com/python/docs/reference/bigquerydatatransfer/latest) with `pip install google-cloud-bigquery-datatransfer`. Then create a transfer configuration to copy the dataset.

    // Import the Google Cloud client library
    const {BigQuery} = require('https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/overview.html');
    const bigquery = new https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/bigquery/bigquery.html();

    async function deleteDataset() {
      // Deletes a dataset named "my_dataset".

      /**
       * TODO(developer): Uncomment the following lines before running the sample.
       */
      // const datasetId = 'my_dataset';

      // Create a reference to the existing dataset
      const dataset = bigquery.dataset(datasetId);

      // Delete the dataset and its contents
      await dataset.delete({force: true});
      console.log(`Dataset ${dataset.id} deleted.`);
    }

### PHP


Before trying this sample, follow the PHP setup instructions in the
[BigQuery quickstart using
client libraries](https://docs.cloud.google.com/bigquery/docs/quickstarts/quickstart-client-libraries).


For more information, see the
[BigQuery PHP API
reference documentation](https://docs.cloud.google.com/php/docs/reference/cloud-bigquery/latest/BigQueryClient).


To authenticate to BigQuery, set up Application Default Credentials.
For more information, see

[Set up authentication for client libraries](https://docs.cloud.google.com/bigquery/docs/authentication#client-libs).

Install the [Python client for the BigQuery Data Transfer API](https://docs.cloud.google.com/python/docs/reference/bigquerydatatransfer/latest) with `pip install google-cloud-bigquery-datatransfer`. Then create a transfer configuration to copy the dataset.

    use Google\Cloud\BigQuery\BigQueryClient;

    /** Uncomment and populate these variables in your code */
    // $projectId = 'The Google project ID';
    // $datasetId = 'The BigQuery dataset ID';

    $bigQuery = new BigQueryClient([
        'projectId' => $projectId,
    ]);
    $dataset = $bigQuery->dataset($datasetId);
    $table = $dataset->delete();
    printf('Deleted dataset %s' . PHP_EOL, $datasetId);

### Python


Before trying this sample, follow the Python setup instructions in the
[BigQuery quickstart using
client libraries](https://docs.cloud.google.com/bigquery/docs/quickstarts/quickstart-client-libraries).


For more information, see the
[BigQuery Python API
reference documentation](https://docs.cloud.google.com/python/docs/reference/bigquery/latest).


To authenticate to BigQuery, set up Application Default Credentials.
For more information, see

[Set up authentication for client libraries](https://docs.cloud.google.com/bigquery/docs/authentication#client-libs).

Install the [Python client for the BigQuery Data Transfer API](https://docs.cloud.google.com/python/docs/reference/bigquerydatatransfer/latest) with `pip install google-cloud-bigquery-datatransfer`. Then create a transfer configuration to copy the dataset.


    from google.cloud import https://docs.cloud.google.com/python/docs/reference/bigquery/latest

    # Construct a BigQuery client object.
    client = https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html()

    # TODO(developer): Set model_id to the ID of the model to fetch.
    # dataset_id = 'your-project.your_dataset'

    # Use the delete_contents parameter to delete a dataset and its contents.
    # Use the not_found_ok parameter to not receive an error if the dataset has already been deleted.
    client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_delete_dataset(
        dataset_id, delete_contents=True, not_found_ok=True
    )  # Make an API request.

    print("Deleted dataset '{}'.".format(dataset_id))

### Ruby

The following code sample deletes an empty dataset.


Before trying this sample, follow the Ruby setup instructions in the
[BigQuery quickstart using
client libraries](https://docs.cloud.google.com/bigquery/docs/quickstarts/quickstart-client-libraries).


For more information, see the
[BigQuery Ruby API
reference documentation](https://googleapis.dev/ruby/google-cloud-bigquery/latest/Google/Cloud/Bigquery.html).


To authenticate to BigQuery, set up Application Default Credentials.
For more information, see

[Set up authentication for client libraries](https://docs.cloud.google.com/bigquery/docs/authentication#client-libs).

Install the [Python client for the BigQuery Data Transfer API](https://docs.cloud.google.com/python/docs/reference/bigquerydatatransfer/latest) with `pip install google-cloud-bigquery-datatransfer`. Then create a transfer configuration to copy the dataset.

    require "google/cloud/bigquery"

    def delete_dataset dataset_id = "my_empty_dataset"
      bigquery = Google::Cloud::https://docs.cloud.google.com/ruby/docs/reference/google-cloud-bigquery-connection/latest/Google-Cloud-Bigquery.html.https://docs.cloud.google.com/ruby/docs/reference/google-cloud-bigquery/latest/Google-Cloud-Bigquery.html

      # Delete a dataset that does not contain any tables
      dataset = bigquery.dataset dataset_id
      dataset.delete
      puts "Dataset #{dataset_id} deleted."
    end

The following code sample deletes a dataset and all of its contents:

    # Copyright 2020 Google LLC
    #
    # Licensed under the Apache License, Version 2.0 (the "License");
    # you may not use this file except in compliance with the License.
    # You may obtain a copy of the License at
    #
    #     http://www.apache.org/licenses/LICENSE-2.0
    #
    # Unless required by applicable law or agreed to in writing, software
    # distributed under the License is distributed on an "AS IS" BASIS,
    # WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    # See the License for the specific language governing permissions and
    # limitations under the License.
    require "google/cloud/bigquery"

    def delete_dataset_and_contents dataset_id = "my_dataset_with_tables"
      bigquery = Google::Cloud::https://docs.cloud.google.com/ruby/docs/reference/google-cloud-bigquery-connection/latest/Google-Cloud-Bigquery.html.https://docs.cloud.google.com/ruby/docs/reference/google-cloud-bigquery/latest/Google-Cloud-Bigquery.html

      # Use the force parameter to delete a dataset and its contents
      dataset = bigquery.dataset dataset_id
      dataset.delete force: true
      puts "Dataset #{dataset_id} and contents deleted."
    end

## Restore tables from deleted datasets

You can restore tables from a deleted dataset that are within the dataset's
[time travel window](https://docs.cloud.google.com/bigquery/docs/time-travel). To restore the entire dataset,
see [Restore deleted datasets](https://docs.cloud.google.com/bigquery/docs/restore-deleted-datasets).

1. Create a dataset with the same name and in the same location as the original.
2. Choose a timestamp from before the original dataset was deleted by using a format of milliseconds since the epoch--for example, `1418864998000`.
3. Copy the `originaldataset.table1` table at the time `1418864998000` into
   the new dataset:

   ```
   bq cp originaldataset.table1@1418864998000 mydataset.mytable
   ```

   To find the names of the nonempty tables that were in the deleted dataset,
   query the dataset's
   [`INFORMATION_SCHEMA.TABLE_STORAGE` view](https://docs.cloud.google.com/bigquery/docs/information-schema-table-storage)
   within the time travel window.

## Restore deleted datasets

To learn how to restore (or *undelete* ) a deleted dataset, see
[Restore deleted datasets](https://docs.cloud.google.com/bigquery/docs/restore-deleted-datasets).

## Quotas

For information about copy quotas, see [Copy jobs](https://docs.cloud.google.com/bigquery/quotas#copy_jobs).
Usage for copy jobs are available in the `INFORMATION_SCHEMA`. To learn how to
query the `INFORMATION_SCHEMA.JOBS` view, see
[`JOBS` view](https://docs.cloud.google.com/bigquery/docs/information-schema-jobs).

## Pricing

For pricing information for copying datasets, see [Data replication pricing](https://cloud.google.com/bigquery/pricing#data_replication).

BigQuery sends compressed data for copying across regions so the
data that is billed might be less than the actual size of your dataset.
For more information, see [BigQuery pricing](https://cloud.google.com/bigquery/pricing).

## What's next

- Learn how to [create datasets](https://docs.cloud.google.com/bigquery/docs/datasets).
- Learn how to [update datasets](https://docs.cloud.google.com/bigquery/docs/updating-datasets).