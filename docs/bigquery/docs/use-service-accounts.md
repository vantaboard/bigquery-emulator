# Use service accounts with BigQuery Data Transfer Service

Some data sources support data transfer authentication by using a [service account](https://docs.cloud.google.com/iam/docs/service-account-overview)
through the Google Cloud console, API, or the `bq` command line. A service
account is a Google Account associated with your Google Cloud project. A service
account can run jobs, such as scheduled queries or batch processing pipelines by
authenticating with the service account credentials rather than a user's
credentials.

You can update an existing data transfer with the credentials of a service
account. For more information, see [Update data transfer credentials](https://docs.cloud.google.com/bigquery/docs/use-service-accounts#update_data_transfer_credentials).

The following situations require updating credentials:

- Your transfer failed to authorize the user's access to the data source:

  `Error code 401 : Request is missing required authentication credential. UNAUTHENTICATED`
- You receive an **INVALID_USER** error when you attempt to run the transfer:

  `Error code 5 : Authentication failure: User Id not found. Error code: INVALID_USERID`

To learn more about authenticating with service accounts, see
[Introduction to authentication](https://docs.cloud.google.com/bigquery/docs/authentication#sa-impersonation).

## Data sources with service account support

BigQuery Data Transfer Service can use service account credentials for transfers with the
following:

- [Cloud Storage](https://docs.cloud.google.com/bigquery/docs/cloud-storage-transfer)
- [Amazon Redshift](https://docs.cloud.google.com/bigquery/docs/migration/redshift-overview)
- [Amazon S3](https://docs.cloud.google.com/bigquery/docs/s3-transfer-intro)
- [Campaign Manager](https://docs.cloud.google.com/bigquery/docs/doubleclick-campaign-transfer)
- [Carbon Footprint](https://docs.cloud.google.com/carbon-footprint/docs/export)
- [Dataset Copy](https://docs.cloud.google.com/bigquery/docs/copying-datasets)
- [Display \& Video 360](https://docs.cloud.google.com/bigquery/docs/display-video-transfer)
- [Google Ad Manager](https://docs.cloud.google.com/bigquery/docs/doubleclick-publisher-transfer)
- [Google Ads](https://docs.cloud.google.com/bigquery/docs/google-ads-transfer)
- [Google Merchant Center](https://docs.cloud.google.com/bigquery/docs/merchant-center-transfer)
- [Google Play](https://docs.cloud.google.com/bigquery/docs/play-transfer)
- [Scheduled Queries](https://docs.cloud.google.com/bigquery/docs/scheduling-queries#using_a_service_account)
- [Search Ads 360](https://docs.cloud.google.com/bigquery/docs/search-ads-transfer)
- [Teradata](https://docs.cloud.google.com/bigquery/docs/migration/teradata-overview)
- [YouTube Content Owner](https://docs.cloud.google.com/bigquery/docs/youtube-content-owner-transfer)

## Before you begin

- Verify that you have completed all actions required in [Enabling BigQuery Data Transfer Service](https://docs.cloud.google.com/bigquery/docs/enable-transfer-service).
- Grant Identity and Access Management (IAM) roles that give users the necessary permissions to perform each task in this document.

### Required permissions

To update a data transfer to use a service account, you must have the following
permissions:

- The `bigquery.transfers.update` permission to modify the transfer.

  The predefined `roles/bigquery.admin` IAM role includes this
  permission.
- Access to the service account. For more information about granting users the
  service account role, see
  [Service Account User role](https://docs.cloud.google.com/iam/docs/service-account-permissions#user-role).

Ensure that the service account you choose to run the transfer has the
following permissions:

- The `bigquery.datasets.get` and `bigquery.datasets.update` permissions on
  the target dataset. If the table uses
  [column-level access control](https://docs.cloud.google.com/bigquery/docs/column-level-security-intro),
  the service account must also have the `bigquery.tables.setCategory`
  permission.

  The `bigquery.admin` predefined IAM role
  includes all of these permissions. For more information about
  IAM roles in BigQuery Data Transfer Service, see
  [Introduction to IAM](https://docs.cloud.google.com/bigquery/docs/access-control).
- Access to the configured transfer data source. For more information about the
  required permissions for different data sources, see
  [Data sources with service account support](https://docs.cloud.google.com/bigquery/docs/use-service-accounts#data_sources_with_service_account_support).

- For Google Ads transfers, the service account must be granted
  domain-wide authority. For more information, see [Google Ads API Service Account guide](https://developers.google.com/google-ads/api/docs/oauth/service-accounts#service_account_access_setup).

## Update data transfer credentials

### Console

The following procedure updates a data transfer configuration to
authenticate as a service account instead of your individual user account.

1. In the Google Cloud console, go to the Data transfers page.

   [Go to Data transfers](https://console.cloud.google.com/bigquery/transfers)
2. Click the transfer in the data transfers list.

3. Click **EDIT** to update the transfer configuration.

   ![Click edit to edit an existing data transfer](https://docs.cloud.google.com/static/bigquery/images/dts-edit-transfer.png)
4. In the **Service Account** field, enter the service account name.

5. Click **Save**.

### bq

To update the credentials of a
data transfer, you can use the bq command-line tool to
update the transfer configuration.

Use the `bq update` command with the `--transfer_config`,
`--update_credentials`, and `--service_account_name` flags.

For example, the following command updates a data transfer configuration to
authenticate as a service account instead of your individual user account:

    bq update \
    --transfer_config \
    --update_credentials \
    --service_account_name=abcdef-test-sa@abcdef-test.iam.gserviceaccount.com projects/862514376110/locations/us/transferConfigs/5dd12f26-0000-262f-bc38-089e0820fe38 \

> [!NOTE]
> **Note:** If you are using the bq command-line tool, use the `--service_account_name` flag instead of authenticating as a service account.

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
    import com.google.cloud.bigquery.datatransfer.v1.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.DataTransferServiceClient.html;
    import com.google.cloud.bigquery.datatransfer.v1.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.TransferConfig.html;
    import com.google.cloud.bigquery.datatransfer.v1.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.UpdateTransferConfigRequest.html;
    import com.google.protobuf.https://docs.cloud.google.com/java/docs/reference/protobuf/latest/com.google.protobuf.FieldMask.html;
    import com.google.protobuf.util.https://docs.cloud.google.com/java/docs/reference/protobuf/latest/com.google.protobuf.util.FieldMaskUtil.html;
    import java.io.IOException;

    // Sample to update credentials in transfer config.
    public class UpdateCredentials {

      public static void main(String[] args) throws IOException {
        // TODO(developer): Replace these variables before running the sample.
        String configId = "MY_CONFIG_ID";
        String serviceAccount = "MY_SERVICE_ACCOUNT";
        https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.TransferConfig.html transferConfig = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.TransferConfig.html.newBuilder().setName(configId).build();
        https://docs.cloud.google.com/java/docs/reference/protobuf/latest/com.google.protobuf.FieldMask.html updateMask = https://docs.cloud.google.com/java/docs/reference/protobuf/latest/com.google.protobuf.util.FieldMaskUtil.html.fromString("service_account_name");
        updateCredentials(transferConfig, serviceAccount, updateMask);
      }

      public static void updateCredentials(
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.TransferConfig.html transferConfig, String serviceAccount, https://docs.cloud.google.com/java/docs/reference/protobuf/latest/com.google.protobuf.FieldMask.html updateMask)
          throws IOException {
        try (https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.DataTransferServiceClient.html dataTransferServiceClient = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.DataTransferServiceClient.html.create()) {
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.UpdateTransferConfigRequest.html request =
              https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.UpdateTransferConfigRequest.html.newBuilder()
                  .setTransferConfig(transferConfig)
                  .https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.UpdateTransferConfigRequest.Builder.html#com_google_cloud_bigquery_datatransfer_v1_UpdateTransferConfigRequest_Builder_setUpdateMask_com_google_protobuf_FieldMask_(updateMask)
                  .setServiceAccountName(serviceAccount)
                  .build();
          dataTransferServiceClient.updateTransferConfig(request);
          System.out.println("Credentials updated successfully");
        } catch (https://docs.cloud.google.com/java/docs/reference/gax/latest/com.google.api.gax.rpc.ApiException.html ex) {
          System.out.print("Credentials was not updated." + ex.toString());
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

    from google.cloud import bigquery_datatransfer
    from google.protobuf import field_mask_pb2

    transfer_client = bigquery_datatransfer.https://docs.cloud.google.com/python/docs/reference/bigquerydatatransfer/latest/google.cloud.bigquery_datatransfer_v1.services.data_transfer_service.DataTransferServiceClient.html()

    service_account_name = "abcdef-test-sa@abcdef-test.iam.gserviceaccount.com"
    transfer_config_name = "projects/1234/locations/us/transferConfigs/abcd"

    transfer_config = bigquery_datatransfer.https://docs.cloud.google.com/python/docs/reference/bigquerydatatransfer/latest/google.cloud.bigquery_datatransfer_v1.types.TransferConfig.html(name=transfer_config_name)

    transfer_config = transfer_client.https://docs.cloud.google.com/python/docs/reference/bigquerydatatransfer/latest/google.cloud.bigquery_datatransfer_v1.services.data_transfer_service.DataTransferServiceClient.html#google_cloud_bigquery_datatransfer_v1_services_data_transfer_service_DataTransferServiceClient_update_transfer_config(
        {
            "transfer_config": transfer_config,
            "update_mask": field_mask_pb2.FieldMask(paths=["service_account_name"]),
            "service_account_name": service_account_name,
        }
    )

    print("Updated config: '{}'".format(transfer_config.name))