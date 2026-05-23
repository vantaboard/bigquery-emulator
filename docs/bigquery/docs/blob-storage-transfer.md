# Load Blob Storage data into BigQuery

You can load data from Blob Storage to BigQuery using the
[BigQuery Data Transfer Service](https://docs.cloud.google.com/bigquery/docs/dts-introduction) for Blob Storage connector. With the
BigQuery Data Transfer Service, you can schedule recurring transfer jobs that
add your latest data from Blob Storage to
BigQuery.

## Before you begin

Before you create a Blob Storage data transfer, do the following:

- Verify that you have completed all actions that are required to [enable the BigQuery Data Transfer Service](https://docs.cloud.google.com/bigquery/docs/enable-transfer-service).
- Choose an existing BigQuery dataset or [create a new dataset](https://docs.cloud.google.com/bigquery/docs/datasets) to store your data.
- Choose an existing BigQuery table or [create a new destination table](https://docs.cloud.google.com/bigquery/docs/tables#create_an_empty_table_with_a_schema_definition) for your data transfer, and specify the schema definition. The destination table must follow the [table naming rules](https://docs.cloud.google.com/bigquery/docs/tables#table_naming). Destination table names also support [parameters](https://docs.cloud.google.com/bigquery/docs/blob-storage-transfer-parameters). You can create a BigQuery table or [create Iceberg managed table](https://docs.cloud.google.com/bigquery/docs/iceberg-tables#create-iceberg-tables).
- Retrieve your Blob Storage storage account name, container name, data path (optional), and SAS token. For information about granting access to Blob Storage using a shared access signature (SAS), see [Shared access signature (SAS)](https://docs.cloud.google.com/bigquery/docs/blob-storage-transfer-intro#shared-access-signature).
- If you restrict access to your Azure resources using an Azure Storage firewall, [add BigQuery Data Transfer Service workers to your list of allowed IPs](https://docs.cloud.google.com/bigquery/docs/blob-storage-transfer-intro#ip_restrictions).
- If you plan on specifying a customer-managed encryption key (CMEK), ensure that your [service account has permissions to encrypt and decrypt](https://docs.cloud.google.com/bigquery/docs/customer-managed-encryption#grant_permission), and that you have the [Cloud KMS key resource ID](https://docs.cloud.google.com/bigquery/docs/customer-managed-encryption#key_resource_id) required to use CMEK. For information about how CMEK works with the BigQuery Data Transfer Service, see [Specify encryption key with transfers](https://docs.cloud.google.com/bigquery/docs/blob-storage-transfer#CMEK).

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

### Required Blob Storage roles

For information about the required permissions in Blob Storage to
enable the data transfer, see
[Shared access signature (SAS)](https://docs.cloud.google.com/bigquery/docs/blob-storage-transfer-intro#shared-access-signature).

## Limitations

Blob Storage data transfers are subject to the following limitations:

- The minimum interval time between recurring data transfers is 1 hour. The default interval is 24 hours.
- Depending on the format of your Blob Storage source data, there may be additional limitations:
  - [CSV limitations](https://docs.cloud.google.com/bigquery/docs/loading-data-cloud-storage-csv#limitations)
  - [JSON limitations](https://docs.cloud.google.com/bigquery/docs/loading-data-cloud-storage-json#limitations)
  - [Limitations on nested and repeated data](https://docs.cloud.google.com/bigquery/docs/nested-repeated#limitations)
- Data transfers to [BigQuery Omni locations](https://docs.cloud.google.com/bigquery/docs/omni-introduction#locations) are not supported.

## Set up a Blob Storage data transfer

Select one of the following options:

### Console

1. Go to the Data transfers page in the Google Cloud console.

   [Go to Data transfers](https://console.cloud.google.com/bigquery/transfers)
2. Click **Create transfer**.

3. On the **Create transfer** page, do the following:

   - In the **Source type** section, for **Source** , select
     **Azure Blob Storage \& ADLS**:


     ![Transfer source type](https://docs.cloud.google.com/static/bigquery/images/blob-transfer-source.png)
   - In the **Transfer config name** section, for **Display name**, enter a
     name for the data transfer.

   - In the **Schedule options** section:

     - Select a **Repeat frequency** . If you select **Hours** , **Days** , **Weeks** , or **Months** , you must also specify a frequency. You can also select **Custom** to specify a custom repeat frequency. If you select **On-demand** , then this data transfer runs when you [manually trigger the transfer](https://docs.cloud.google.com/bigquery/docs/working-with-transfers#manually_trigger_a_transfer).
     - If applicable, select either **Start now** or **Start at set time** and provide a start date and run time.
   - In the **Destination settings** section:

     - For **Dataset**, select the dataset that you created to store your data.
     - Select **Native table** if you want to transfer to a BigQuery table.
     - Select **Apache Iceberg** if you want to transfer to a Iceberg managed table.
   - In the **Data source details** section, do the following:

     - For **Destination table** , enter the name of the table you created to store the data in BigQuery. Destination table names support [parameters](https://docs.cloud.google.com/bigquery/docs/blob-storage-transfer-parameters).
     - For **Azure storage account name**, enter the Blob Storage account name.
     - For **Container name**, enter the Blob Storage container name.
     - For **Data path** , enter the path to filter files to be transferred. See [examples](https://docs.cloud.google.com/bigquery/docs/blob-storage-transfer-intro#azure_blob_storage_data_path_examples).
     - For **SAS token**, enter the Azure SAS token.
     - For **File format**, choose your source data format.
     - For **Write disposition** , select **`WRITE_APPEND`** to incrementally append new data to the destination table, or **`WRITE_TRUNCATE`** to overwrite data in the destination table during each transfer run. **`WRITE_APPEND`** is the default value for **Write disposition**.

     For more information about how BigQuery Data Transfer Service ingests
     data using either **`WRITE_APPEND`** or **`WRITE_TRUNCATE`** , see
     [Data ingestion for Azure Blob transfers](https://docs.cloud.google.com/bigquery/docs/blob-storage-transfer-intro#data-ingestion). For more information
     about the `writeDisposition` field, see
     [`JobConfigurationLoad`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/Job#jobconfigurationload).


     ![Data source details](https://docs.cloud.google.com/static/bigquery/images/blob-source-details.png)
   - In the **Transfer options** section, do the following:

     - For **Number of errors allowed**, enter an integer value for the maximum number of bad records that can be ignored. The default value is 0.
     - (Optional) For **Decimal target types** , enter a comma-separated list of possible SQL data types that decimal values in the source data are converted to. Which SQL data type is selected for conversion depends on the following conditions:
       - In the order of `NUMERIC`, `BIGNUMERIC`, and `STRING`, a type is picked if it is in your specified list and if it supports the precision and the scale.
       - If none of your listed data types support the precision and the scale, the data type supporting the widest range in your specified list is selected. If a value exceeds the supported range when reading the source data, an error is thrown.
       - The data type `STRING` supports all precision and scale values.
       - If this field is left empty, the data type defaults to `NUMERIC,STRING` for ORC, and `NUMERIC` for other file formats.
       - This field cannot contain duplicate data types.
       - The order of the data types that you list is ignored.
   - If you chose CSV or JSON as your file format, in the **JSON, CSV**
     section, check **Ignore unknown values** to accept rows that contain
     values that don't match the schema.

   - If you chose CSV as your file format, in the **CSV** section enter any
     [additional CSV options](https://docs.cloud.google.com/bigquery/docs/loading-data-cloud-storage-csv#csv-options)
     for loading data.


     ![CSV options](https://docs.cloud.google.com/static/bigquery/images/csv-options.png)
   - In the **Notification options** section, you can choose to enable
     email notifications and Pub/Sub notifications.

     - When you enable email notifications, the transfer administrator receives an email notification when a transfer run fails.
     - When you enable [Pub/Sub notifications](https://docs.cloud.google.com/bigquery/docs/transfer-run-notifications#notifications), choose a [topic](https://docs.cloud.google.com/pubsub/docs/admin) name to publish to or click **Create a topic** to create one.
   - If you use [CMEKs](https://docs.cloud.google.com/bigquery/docs/customer-managed-encryption), in the
     **Advanced options** section, select **Customer-managed key** . A list of
     your available CMEKs appears for you to choose from. For information
     about how CMEKs work with the BigQuery Data Transfer Service, see
     [Specify encryption key with transfers](https://docs.cloud.google.com/bigquery/docs/blob-storage-transfer#CMEK).

4. Click **Save**.

### bq

Use the
[`bq mk --transfer_config` command](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#mk-transfer-config)
to create a Blob Storage transfer:

```bash
bq mk \
  --transfer_config \
  --project_id=PROJECT_ID \
  --data_source=DATA_SOURCE \
  --display_name=DISPLAY_NAME \
  --target_dataset=DATASET \
  --destination_kms_key=DESTINATION_KEY \
  --params=PARAMETERS
```

Replace the following:

- `PROJECT_ID`: (Optional) the project ID containing your target dataset. If not specified, your default project is used.
- `DATA_SOURCE`: `azure_blob_storage`.
- `DISPLAY_NAME`: the display name for the data transfer configuration. The transfer name can be any value that lets you identify the transfer if you need to modify it later.
- `DATASET`: the target dataset for the data transfer configuration.
- `DESTINATION_KEY`: (Optional) the [Cloud KMS key resource ID](https://docs.cloud.google.com/bigquery/docs/customer-managed-encryption#key_resource_id)--- for example, `projects/project_name/locations/us/keyRings/key_ring_name/cryptoKeys/key_name`.
- `PARAMETERS`: the parameters for the data transfer configuration, listed in JSON format. For example, `--params={"param1":"value1", "param2":"value2"}`. The following are the parameters for a Blob Storage data transfer:
  - `destination_table_name_template`: Required. The name of your destination table.
  - `storage_account`: Required. The Blob Storage account name.
  - `container`: Required. The Blob Storage container name.
  - `data_path`: Optional. The path to filter files to be transferred. See [examples](https://docs.cloud.google.com/bigquery/docs/blob-storage-transfer-intro#azure_blob_storage_data_path_examples).
  - `sas_token`: Required. The Azure SAS token.
  - `file_format`: Optional. The type of files you want to transfer: `CSV`, `JSON`, `AVRO`, `PARQUET`, or `ORC`. The default value is `CSV`.
  - `write_disposition`: Optional. Select `WRITE_APPEND` to append data to the destination table, or `WRITE_TRUNCATE`, to overwrite data in the destination table. The default value is `WRITE_APPEND`.
  - `max_bad_records`: Optional. The number of allowed bad records. The default value is 0.
  - `decimal_target_types`: Optional. A comma-separated list of possible SQL data types that decimal values in the source data are converted to. If this field is not provided, the data type defaults to `NUMERIC,STRING` for ORC, and `NUMERIC` for the other file formats.
  - `ignore_unknown_values`: Optional, and ignored if `file_format` is not `JSON` or `CSV`. Set to `true` to accept rows that contain values that don't match the schema.
  - `field_delimiter`: Optional, and applies only when `file_format` is `CSV`. The character that separates fields. The default value is `,`.
  - `skip_leading_rows`: Optional, and applies only when `file_format` is `CSV`. Indicates the number of header rows that you don't want to import. The default value is 0.
  - `allow_quoted_newlines`: Optional, and applies only when `file_format` is `CSV`. Indicates whether to allow newlines within quoted fields.
  - `allow_jagged_rows`: Optional, and applies only when `file_format` is `CSV`. Indicates whether to accept rows that are missing trailing optional columns. The missing values are filled in with `NULL`.

For example, the following creates a Blob Storage data transfer
called `mytransfer`:

```bash
bq mk \
  --transfer_config \
  --data_source=azure_blob_storage \
  --display_name=mytransfer \
  --target_dataset=mydataset \
  --destination_kms_key=projects/myproject/locations/us/keyRings/mykeyring/cryptoKeys/key1
  --params={"destination_table_name_template":"mytable",
      "storage_account":"myaccount",
      "container":"mycontainer",
      "data_path":"myfolder/*.csv",
      "sas_token":"my_sas_token_value",
      "file_format":"CSV",
      "max_bad_records":"1",
      "ignore_unknown_values":"true",
      "field_delimiter":"|",
      "skip_leading_rows":"1",
      "allow_quoted_newlines":"true",
      "allow_jagged_rows":"false"}
```

### API

Use the [`projects.locations.transferConfigs.create`](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs/create)
method and supply an instance of the [`TransferConfig`](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs#TransferConfig)
resource.

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

    // Sample to create azure blob storage transfer config.
    public class CreateAzureBlobStorageTransfer {

      public static void main(String[] args) throws IOException {
        // TODO(developer): Replace these variables before running the sample.
        final String projectId = "MY_PROJECT_ID";
        final String displayName = "MY_TRANSFER_DISPLAY_NAME";
        final String datasetId = "MY_DATASET_ID";
        String tableId = "MY_TABLE_ID";
        String storageAccount = "MY_AZURE_STORAGE_ACCOUNT_NAME";
        String containerName = "MY_AZURE_CONTAINER_NAME";
        String dataPath = "MY_AZURE_FILE_NAME_OR_PREFIX";
        String sasToken = "MY_AZURE_SAS_TOKEN";
        String fileFormat = "CSV";
        String fieldDelimiter = ",";
        String skipLeadingRows = "1";
        Map<String, Value> params = new HashMap<>();
        params.put(
            "destination_table_name_template", https://docs.cloud.google.com/java/docs/reference/protobuf/latest/com.google.protobuf.Value.html.newBuilder().setStringValue(tableId).build());
        params.put("storage_account", https://docs.cloud.google.com/java/docs/reference/protobuf/latest/com.google.protobuf.Value.html.newBuilder().setStringValue(storageAccount).build());
        params.put("container", https://docs.cloud.google.com/java/docs/reference/protobuf/latest/com.google.protobuf.Value.html.newBuilder().setStringValue(containerName).build());
        params.put("data_path", https://docs.cloud.google.com/java/docs/reference/protobuf/latest/com.google.protobuf.Value.html.newBuilder().setStringValue(dataPath).build());
        params.put("sas_token", https://docs.cloud.google.com/java/docs/reference/protobuf/latest/com.google.protobuf.Value.html.newBuilder().setStringValue(sasToken).build());
        params.put("file_format", https://docs.cloud.google.com/java/docs/reference/protobuf/latest/com.google.protobuf.Value.html.newBuilder().setStringValue(fileFormat).build());
        params.put("field_delimiter", https://docs.cloud.google.com/java/docs/reference/protobuf/latest/com.google.protobuf.Value.html.newBuilder().setStringValue(fieldDelimiter).build());
        params.put("skip_leading_rows", https://docs.cloud.google.com/java/docs/reference/protobuf/latest/com.google.protobuf.Value.html.newBuilder().setStringValue(skipLeadingRows).build());
        createAzureBlobStorageTransfer(projectId, displayName, datasetId, params);
      }

      public static void createAzureBlobStorageTransfer(
          String projectId, String displayName, String datasetId, Map<String, https://docs.cloud.google.com/java/docs/reference/protobuf/latest/com.google.protobuf.Value.html> params)
          throws IOException {
        https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.TransferConfig.html transferConfig =
            https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.TransferConfig.html.newBuilder()
                .setDestinationDatasetId(datasetId)
                .setDisplayName(displayName)
                .setDataSourceId("azure_blob_storage")
                .setParams(https://docs.cloud.google.com/java/docs/reference/protobuf/latest/com.google.protobuf.Struct.html.newBuilder().https://docs.cloud.google.com/java/docs/reference/protobuf/latest/com.google.protobuf.Struct.Builder.html#com_google_protobuf_Struct_Builder_putAllFields_java_util_Map_java_lang_String_com_google_protobuf_Value__(params).build())
                .setSchedule("every 24 hours")
                .build();
        // Initialize client that will be used to send requests. This client only needs to be created
        // once, and can be reused for multiple requests.
        try (https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.DataTransferServiceClient.html client = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.DataTransferServiceClient.html.create()) {
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.ProjectName.html parent = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.ProjectName.html.of(projectId);
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.CreateTransferConfigRequest.html request =
              https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.CreateTransferConfigRequest.html.newBuilder()
                  .setParent(parent.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.ProjectName.html#com_google_cloud_bigquery_datatransfer_v1_ProjectName_toString__())
                  .setTransferConfig(transferConfig)
                  .build();
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.TransferConfig.html config = client.createTransferConfig(request);
          System.out.println("Azure Blob Storage transfer created successfully: " + config.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.TransferConfig.html#com_google_cloud_bigquery_datatransfer_v1_TransferConfig_getName__());
        } catch (https://docs.cloud.google.com/java/docs/reference/gax/latest/com.google.api.gax.rpc.ApiException.html ex) {
          System.out.print("Azure Blob Storage transfer was not created." + ex.toString());
        }
      }
    }

## Specify encryption key with transfers

You can specify [customer-managed encryption keys (CMEKs)](https://docs.cloud.google.com/kms/docs/cmek) to encrypt data for a transfer run. You can use a CMEK to support transfers from [Azure Blob Storage](https://docs.cloud.google.com/bigquery/docs/blob-storage-transfer-intro).

When you specify a CMEK with a transfer, the BigQuery Data Transfer Service applies the
CMEK to any intermediate on-disk cache of ingested data so that the entire
data transfer workflow is CMEK compliant.

You cannot update an existing transfer to add a CMEK if the transfer was not
originally created with a CMEK. For example, you cannot change a destination
table that was originally default encrypted to now be encrypted with CMEK.
Conversely, you also cannot change a CMEK-encrypted destination table
to have a different type of encryption.

You can update a CMEK for a transfer if the transfer configuration was
originally created with a CMEK encryption. When you update a CMEK for a transfer
configuration, the BigQuery Data Transfer Service propagates the CMEK to the destination
tables at the next run of the transfer, where the BigQuery Data Transfer Service
replaces any outdated CMEKs with the new CMEK during the transfer run.
For more information, see [Update a transfer](https://docs.cloud.google.com/bigquery/docs/working-with-transfers#update_a_transfer).

You can also use [project default keys](https://docs.cloud.google.com/bigquery/docs/customer-managed-encryption#project_default_key).
When you specify a project default key with a transfer, the BigQuery Data Transfer Service
uses the project default key as the default key for any new transfer
configurations.

## Troubleshoot transfer setup

If you are having issues setting up your data transfer, see [Blob Storage transfer issues](https://docs.cloud.google.com/bigquery/docs/transfer-troubleshooting#blob-storage).

## What's next

- Learn more about [runtime parameters in transfers](https://docs.cloud.google.com/bigquery/docs/blob-storage-transfer-parameters).
- Learn more about the [BigQuery Data Transfer Service](https://docs.cloud.google.com/bigquery/docs/dts-introduction).
- Learn how to [load data with cross-cloud operations](https://docs.cloud.google.com/bigquery/docs/load-data-using-cross-cloud-transfer).