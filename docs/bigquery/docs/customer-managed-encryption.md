# Customer-managed Cloud KMS keys

> [!NOTE]
> **Note:** This feature may not be available when using reservations that are created with certain BigQuery editions. For more information about which features are enabled in each edition, see [Introduction to
> BigQuery editions](https://docs.cloud.google.com/bigquery/docs/editions-intro).

By default, BigQuery
[encrypts your content stored at rest](https://docs.cloud.google.com/docs/security/encryption/default-encryption).
BigQuery handles and manages this default encryption for you
without any additional actions on your part. First, data in a
BigQuery table is encrypted using a *data encryption key* . Then,
those data encryption keys are encrypted with *key encryption keys* , which is
known as [envelope encryption](https://docs.cloud.google.com/kms/docs/envelope-encryption). Key encryption
keys don't directly encrypt your data but are used to encrypt the data
encryption keys that Google uses to encrypt your data.

If you want to control encryption yourself, you can use customer-managed
encryption keys (CMEK) for BigQuery. Instead of Google owning and
managing the key encryption keys that protect your data, you control and manage
key encryption keys in [Cloud KMS](https://docs.cloud.google.com/kms/docs). This document provides
details about manually creating Cloud KMS keys for
BigQuery.

Learn more about
[encryption options on Google Cloud](https://docs.cloud.google.com/docs/security/encryption/default-encryption).
For specific information about CMEK, including its advantages and limitations,
see [Customer-managed encryption keys](https://docs.cloud.google.com/kms/docs/cmek).

## Before you begin

- All data assets residing in BigQuery managed storage support
  CMEK. However, if you are also querying data stored in an [external data source](https://docs.cloud.google.com/bigquery/docs/external-data-sources)
  such as Cloud Storage that has CMEK-encrypted
  data, then the data encryption is managed by [Cloud Storage](https://docs.cloud.google.com/storage/docs/encryption/customer-managed-keys).
  For example, BigLake tables support data encrypted with CMEK in
  Cloud Storage.

  BigQuery and [BigLake tables](https://docs.cloud.google.com/bigquery/docs/biglake-intro)
  don't support Customer-Supplied Encryption Keys (CSEK).
- Decide whether you are going to run BigQuery and
  Cloud KMS in the same Google Cloud project, or in different
  projects. For documentation example purposes, the following convention is used:

  - `PROJECT_ID`: the project ID of the project running BigQuery
  - `PROJECT_NUMBER`: the project number of the project running BigQuery
  - `KMS_PROJECT_ID`: the project ID of the project running Cloud KMS (even if this is the same project running BigQuery)

  For information about Google Cloud project IDs and project numbers, see [Find the project name, number, and ID](https://docs.cloud.google.com/resource-manager/docs/view-update-projects#identifying_projects).

  <br />

- BigQuery is automatically enabled in new projects. If you are
  using a pre-existing project to run BigQuery,
  [enable the BigQuery API](https://console.cloud.google.com/flows/enableapi?apiid=bigquery).

- For the Google Cloud project that runs Cloud KMS,
  [enable the Cloud Key Management Service API](https://console.cloud.google.com/flows/enableapi?apiid=cloudkms.googleapis.com).

A decryption call is performed using Cloud KMS once per query to a
CMEK-encrypted table. For more information, see [Cloud KMS pricing](https://cloud.google.com/kms/pricing).

## Encryption specification

Cloud KMS keys used to protect your data in BigQuery are
AES-256 keys. These keys are used as key encryption keys in
BigQuery, in that they encrypt the data encryption keys that
encrypt your data.

## Manual or automated key creation

You can either create your CMEK keys manually or use Cloud KMS Autokey.
Autokey simplifies creating and
managing your CMEK keys by automating provisioning and assignment. With
Autokey, you don't need to provision key rings, keys, and service
accounts ahead of time. Instead, they are generated on demand as part of
BigQuery resource creation. For more information, see the
[Autokey overview](https://docs.cloud.google.com/kms/docs/autokey-overview).

### Manually create key ring and key

For the Google Cloud project that runs Cloud KMS, create a key ring
and a key as described in
[Creating key rings and keys](https://docs.cloud.google.com/kms/docs/creating-keys). Create the key ring in a
location that matches the location of your BigQuery dataset:

- Any multi-regional dataset should use a multi-regional key ring from a
  matching location. For example, a dataset in region `US` should be
  protected with a key ring from region `us`, and a dataset in region `EU`
  should be protected with a key ring from region `europe`.

- Regional datasets should use matching regional keys. For example,
  a dataset in region `asia-northeast1` should be protected with a key
  ring from region `asia-northeast1`.

- You can't use the `global` region when configuring CMEK for
  BigQuery in the Google Cloud console. However, you can
  use the `global` region when configuring CMEK for
  BigQuery by using the bq command-line tool or GoogleSQL.

For more information about the supported locations for BigQuery
and Cloud KMS, see [Cloud locations](https://cloud.google.com/about/locations/).

## Grant encryption and decryption permission

To protect your BigQuery data with a CMEK key, grant the
BigQuery service account permission to encrypt and decrypt using
that key. The
[Cloud KMS CryptoKey Encrypter/Decrypter](https://docs.cloud.google.com/kms/docs/reference/permissions-and-roles#predefined_roles)
role grants this permission.

Make sure your service account has been created, and then use the
Google Cloud console
to determine the BigQuery service
account ID. Next, provide the service account with the appropriate role to
encrypt
and decrypt using Cloud KMS.

### Trigger creation of your service account

Your BigQuery service account is not initially created when you
create a project. To trigger the creation of your service
account, enter a command that uses it, such as the
[`bq show --encryption_service_account`](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#encryption_service_account_flag)
command, or call the
[projects.getServiceAccount](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/projects/getServiceAccount) API method. For example:

```
bq show --encryption_service_account --project_id=PROJECT_ID
```

<br />

### Determine the service account ID

The BigQuery service account ID is of the form:

```
bq-PROJECT_NUMBER@bigquery-encryption.iam.gserviceaccount.com
```

The following techniques show how you can determine the BigQuery
service account ID for your project.

### Console

1. Go to the [**Dashboard** page](https://console.cloud.google.com/home)
   in the Google Cloud console.

   [Go to the Dashboard page](https://console.cloud.google.com/home)
2. Click the **Select from** drop-down list at the top of the page. In the
   **Select From** window that appears, select your project.

3. Both the project ID and project number are displayed on the project Dashboard
   **Project info** card:

   ![project info card](https://docs.cloud.google.com/static/resource-manager/img/project_id.png)
4. In the following string, replace <var translate="no">PROJECT_NUMBER</var> with your
   project number. The new string identifies your BigQuery
   service account ID.

   ```
   bq-PROJECT_NUMBER@bigquery-encryption.iam.gserviceaccount.com
   ```

### bq

Use the `bq show` command with the `--encryption_service_account` flag to
determine the service account ID:

```
bq show --encryption_service_account
```

The command displays the service account ID:

```
                  ServiceAccountID
---
bq-PROJECT_NUMBER@bigquery-encryption.iam.gserviceaccount.com
```

### Assign the Encrypter/Decrypter role

Assign the Cloud KMS CryptoKey Encrypter/Decrypter
[role](https://docs.cloud.google.com/kms/docs/reference/permissions-and-roles#predefined_roles) to the
BigQuery system service account that you copied to your
clipboard. This account is of the form:

```
bq-PROJECT_NUMBER@bigquery-encryption.iam.gserviceaccount.com
```

### Console

1. Open the **Cryptographic Keys** page in the Google Cloud console.

   [Open
   the Cryptographic Keys page](https://console.cloud.google.com/security/kms)

   <br />

2. Click the name of the key ring that contains the key.

3. Click the checkbox for the encryption key to which you want to add the
   role. The **Permissions** tab opens.

4. Click **Add member**.

5. Enter the email address of the service account,
   `bq-PROJECT_NUMBER@bigquery-encryption.iam.gserviceaccount.com`.

   - If the service account is already on the members list, it has existing roles. Click the current role drop-down list for the `bq-PROJECT_NUMBER@bigquery-encryption.iam.gserviceaccount.com` service account.
6. Click the drop-down list for **Select a role** , click **Cloud KMS** , and
   then click the **Cloud KMS CryptoKey Encrypter/Decrypter** role.

7. Click **Save** to apply the role to the `bq-PROJECT_NUMBER@bigquery-encryption.iam.gserviceaccount.com` service
   account.

### gcloud

You can use the Google Cloud CLI to assign the role:

```
gcloud kms keys add-iam-policy-binding \
--project=KMS_PROJECT_ID \
--member serviceAccount:bq-PROJECT_NUMBER@bigquery-encryption.iam.gserviceaccount.com \
--role roles/cloudkms.cryptoKeyEncrypterDecrypter \
--location=KMS_KEY_LOCATION \
--keyring=KMS_KEY_RING \
KMS_KEY
```

Replace the following:

- `KMS_PROJECT_ID`: the ID of your Google Cloud project that is running Cloud KMS
- `PROJECT_NUMBER`: the project number (not project ID) of your Google Cloud project that is running BigQuery
- `KMS_KEY_LOCATION`: the location name of your Cloud KMS key
- `KMS_KEY_RING`: the key ring name of your Cloud KMS key
- `KMS_KEY`: the key name of your Cloud KMS key

## Key resource ID

The resource ID for the Cloud KMS key is required for CMEK use, as
shown in the examples. This key is case-sensitive and in the form:

```
projects/KMS_PROJECT_ID/locations/LOCATION/keyRings/KEY_RING/cryptoKeys/KEY
```

> [!NOTE]
> **Note:** You cannot specify a key resource ID that includes the `/cryptoKeyVersions/` token. BigQuery always uses the key version marked as `primary` to protect a table when it is created.

### Retrieve the key resource ID

1. Open the **Cryptographic Keys** page in the Google Cloud console.

   [Open
   the Cryptographic Keys page](https://console.cloud.google.com/security/kms)

   <br />

2. Click the name of the key ring that contains the key.

3. For the key whose resource ID you are retrieving, click
   **More** .

4. Click **Copy Resource Name**. The resource ID for the key is copied to your
   clipboard. The resource ID is also known as the resource name.

## Create a table protected by Cloud KMS

To create a table that is protected by Cloud KMS:

### Console

1. Open the BigQuery page in the Google Cloud console.

   [Go to the BigQuery page](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)

   If you don't see the left pane, click **Expand left pane** to open the pane.
3. In the **Explorer** pane, expand your project, click **Datasets**, and then click a dataset. The dataset opens in a tab.

4. In the details pane, click **Create table**.

5. On the **Create table** page, fill in the information needed to
   [create an empty table with a schema definition](https://docs.cloud.google.com/bigquery/docs/tables#create_an_empty_table_with_a_schema_definition).
   Before you click **Create Table**, set the encryption type and specify the
   Cloud KMS key to use with the table:

   1. Click **Advanced options**.
   2. Click **Customer-managed key**.
   3. Select the key. If the key you want to use is not listed, enter the [resource ID](https://docs.cloud.google.com/bigquery/docs/customer-managed-encryption#key_resource_id) for the key.
6. Click **Create table**.

### SQL

Use the
[`CREATE TABLE` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_table_statement)
with the `kms_key_name` option:

<br />

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, enter the following statement:

   ```googlesql
   CREATE TABLE DATASET_ID.TABLE_ID (
     name STRING, value INT64
   ) OPTIONS (
       kms_key_name
         = 'projects/KMS_PROJECT_ID/locations/LOCATION/keyRings/KEY_RING/cryptoKeys/KEY');
   ```

   <br />

3. Click **Run**.

   <br />

For more information about how to run queries, see [Run an interactive query](https://docs.cloud.google.com/bigquery/docs/running-queries#queries).

### bq

You can use the bq command-line tool with the `--destination_kms_key` flag
to create the table. The `--destination_kms_key` flag specifies the
[resource ID](https://docs.cloud.google.com/bigquery/docs/customer-managed-encryption#key_resource_id) of the key to use with the table.

To create an empty table with a schema:

```
bq mk --schema name:string,value:integer -t \
--destination_kms_key projects/KMS_PROJECT_ID/locations/LOCATION/keyRings/KEY_RING/cryptoKeys/KEY \
DATASET_ID.TABLE_ID
```

To create a table from a query:

```
bq query --destination_table=DATASET_ID.TABLE_ID \
--destination_kms_key projects/KMS_PROJECT_ID/locations/LOCATION/keyRings/KEY_RING/cryptoKeys/KEY \
"SELECT name,count FROM DATASET_ID.TABLE_ID WHERE gender = 'M' ORDER BY count DESC LIMIT 6"
```

For more information about the bq command-line tool, see
[Using the bq command-line tool](https://docs.cloud.google.com/bigquery/bq-command-line-tool).

### Terraform

Use the
[`google_bigquery_table`](https://registry.terraform.io/providers/hashicorp/google/latest/docs/resources/bigquery_table)
resource.

To authenticate to BigQuery, set up Application Default
Credentials. For more information, see
[Set up authentication for client libraries](https://docs.cloud.google.com/bigquery/docs/authentication#client-libs).

The following example creates a table named `mytable`, and also uses the
[`google_kms_crypto_key`](https://registry.terraform.io/providers/hashicorp/google/latest/docs/resources/kms_crypto_key)
and
[`google_kms_key_ring`](https://registry.terraform.io/providers/hashicorp/google/latest/docs/resources/kms_key_ring)
resources to specify a
[Cloud Key Management Service key](https://docs.cloud.google.com/bigquery/docs/customer-managed-encryption) for the
table.

To run this example, you must enable the
[Cloud Resource Manager API](https://console.cloud.google.com/flows/enableapi?apiid=cloudresourcemanager.googleapis.com)
and the
[Cloud Key Management Service API](https://console.cloud.google.com/flows/enableapi?apiid=cloudkms.googleapis.com).

    resource "google_bigquery_dataset" "default" {
      dataset_id                      = "mydataset"
      default_partition_expiration_ms = 2592000000  # 30 days
      default_table_expiration_ms     = 31536000000 # 365 days
      description                     = "dataset description"
      location                        = "US"
      max_time_travel_hours           = 96 # 4 days

      labels = {
        billing_group = "accounting",
        pii           = "sensitive"
      }
    }

    resource "google_bigquery_table" "default" {
      dataset_id = google_bigquery_dataset.default.dataset_id
      table_id   = "mytable"

      schema = <<EOF
    [
      {
        "name": "ID",
        "type": "INT64",
        "mode": "NULLABLE",
        "description": "Item ID"
      },
      {
        "name": "Item",
        "type": "STRING",
        "mode": "NULLABLE"
      }
    ]
    EOF

      encryption_configuration {
        kms_key_name = google_kms_crypto_key.crypto_key.id
      }

      depends_on = [google_project_iam_member.service_account_access]
    }

    resource "google_kms_crypto_key" "crypto_key" {
      name     = "example-key"
      key_ring = google_kms_key_ring.key_ring.id
    }

    resource "random_id" "default" {
      byte_length = 8
    }

    resource "google_kms_key_ring" "key_ring" {
      name     = "${random_id.default.hex}-example-keyring"
      location = "us"
    }

    # Enable the BigQuery service account to encrypt/decrypt Cloud KMS keys
    data "google_project" "project" {
    }

    resource "google_project_iam_member" "service_account_access" {
      project = data.google_project.project.project_id
      role    = "roles/cloudkms.cryptoKeyEncrypterDecrypter"
      member  = "serviceAccount:bq-${data.google_project.project.number}@bigquery-encryption.iam.gserviceaccount.com"
    }

To apply your Terraform configuration in a Google Cloud project, complete the steps in the
following sections.

## Prepare Cloud Shell

1. Launch [Cloud Shell](https://shell.cloud.google.com/).
2. Set the default Google Cloud project
   where you want to apply your Terraform configurations.

   You only need to run this command once per project, and you can run it in any directory.

   ```
   export GOOGLE_CLOUD_PROJECT=PROJECT_ID
   ```

   Environment variables are overridden if you set explicit values in the Terraform
   configuration file.

## Prepare the directory

Each Terraform configuration file must have its own directory (also
called a *root module*).

1. In [Cloud Shell](https://shell.cloud.google.com/), create a directory and a new file within that directory. The filename must have the `.tf` extension---for example `main.tf`. In this tutorial, the file is referred to as `main.tf`.

   ```
   mkdir DIRECTORY && cd DIRECTORY && touch main.tf
   ```
2. If you are following a tutorial, you can copy the sample code in each section or step.

   Copy the sample code into the newly created `main.tf`.

   Optionally, copy the code from GitHub. This is recommended
   when the Terraform snippet is part of an end-to-end solution.
3. Review and modify the sample parameters to apply to your environment.
4. Save your changes.
5. Initialize Terraform. You only need to do this once per directory.

   ```
   terraform init
   ```

   Optionally, to use the latest Google provider version, include the `-upgrade`
   option:

   ```
   terraform init -upgrade
   ```

## Apply the changes

1. Review the configuration and verify that the resources that Terraform is going to create or update match your expectations:

   ```
   terraform plan
   ```

   Make corrections to the configuration as necessary.
2. Apply the Terraform configuration by running the following command and entering `yes` at the prompt:

   ```
   terraform apply
   ```

   Wait until Terraform displays the "Apply complete!" message.
3. [Open your Google Cloud project](https://console.cloud.google.com/) to view the results. In the Google Cloud console, navigate to your resources in the UI to make sure that Terraform has created or updated them.

> [!NOTE]
> **Note:** Terraform samples typically assume that the required APIs are enabled in your Google Cloud project.

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

    import (
    	"context"
    	"fmt"

    	"cloud.google.com/go/bigquery"
    )

    // createTableWithCMEK demonstrates creating a table protected with a customer managed encryption key.
    func createTableWithCMEK(projectID, datasetID, tableID string) error {
    	// projectID := "my-project-id"
    	// datasetID := "mydatasetid"
    	// tableID := "mytableid"
    	ctx := context.Background()

    	client, err := bigquery.NewClient(ctx, projectID)
    	if err != nil {
    		return fmt.Errorf("bigquery.NewClient: %v", err)
    	}
    	defer client.Close()

    	tableRef := client.Dataset(datasetID).Table(tableID)
    	meta := &bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_TableMetadata{
    		EncryptionConfig: &bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_EncryptionConfig{
    			// TODO: Replace this key with a key you have created in Cloud KMS.
    			KMSKeyName: "projects/cloud-samples-tests/locations/us/keyRings/test/cryptoKeys/test",
    		},
    	}
    	if err := tableRef.Create(ctx, meta); err != nil {
    		return err
    	}
    	return nil
    }

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

    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryException.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryOptions.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.EncryptionConfiguration.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Field.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Schema.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.StandardSQLTypeName.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.StandardTableDefinition.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableDefinition.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableId.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableInfo.html;

    // Sample to create a cmek table
    public class CreateTableCMEK {

      public static void runCreateTableCMEK() {
        // TODO(developer): Replace these variables before running the sample.
        String datasetName = "MY_DATASET_NAME";
        String tableName = "MY_TABLE_NAME";
        String kmsKeyName = "MY_KEY_NAME";
        https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Schema.html schema =
            https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Schema.html.of(
                https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Field.html.of("stringField", https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.StandardSQLTypeName.html.STRING),
                https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Field.html.of("booleanField", https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.StandardSQLTypeName.html.BOOL));
        // i.e. projects/{project}/locations/{location}/keyRings/{key_ring}/cryptoKeys/{cryptoKey}
        https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.EncryptionConfiguration.html encryption =
            https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.EncryptionConfiguration.html.newBuilder().setKmsKeyName(kmsKeyName).build();
        createTableCMEK(datasetName, tableName, schema, encryption);
      }

      public static void createTableCMEK(
          String datasetName, String tableName, https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Schema.html schema, https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.EncryptionConfiguration.html configuration) {
        try {
          // Initialize client that will be used to send requests. This client only needs to be created
          // once, and can be reused for multiple requests.
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html bigquery = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryOptions.html.getDefaultInstance().getService();

          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableId.html tableId = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableId.html.of(datasetName, tableName);
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableDefinition.html tableDefinition = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.StandardTableDefinition.html.of(schema);
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableInfo.html tableInfo =
              https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableInfo.html.newBuilder(tableId, tableDefinition)
                  .setEncryptionConfiguration(configuration)
                  .build();

          bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html#com_google_cloud_bigquery_BigQuery_create_com_google_cloud_bigquery_DatasetInfo_com_google_cloud_bigquery_BigQuery_DatasetOption____(tableInfo);
          System.out.println("Table cmek created successfully");
        } catch (https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryException.html e) {
          System.out.println("Table cmek was not created. \n" + e.toString());
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

Protect a new table with a customer-managed encryption key by setting the [Table.encryption_configuration](https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.table.Table#google_cloud_bigquery_table_Table_encryption_configuration) property to an [EncryptionConfiguration](https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.encryption_configuration.EncryptionConfiguration) object before creating the table.

    from google.cloud import https://docs.cloud.google.com/python/docs/reference/bigquery/latest

    client = https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html()

    # TODO(dev): Change table_id to the full name of the table you want to create.
    table_id = "your-project.your_dataset.your_table_name"

    # Set the encryption key to use for the table.
    # TODO: Replace this key with a key you have created in Cloud KMS.
    kms_key_name = "projects/your-project/locations/us/keyRings/test/cryptoKeys/test"

    table = https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.table.Table.html(table_id)
    table.encryption_configuration = https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.encryption_configuration.EncryptionConfiguration.html(
        kms_key_name=kms_key_name
    )
    table = client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_create_table(table)  # API request

    print(f"Created {table_id}.")
    print(f"Key: {table.encryption_configuration.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.encryption_configuration.EncryptionConfiguration.html#google_cloud_bigquery_encryption_configuration_EncryptionConfiguration_kms_key_name}.")

<br />

### Query a table protected by a Cloud KMS key

No special arrangements are required to query a table protected by
Cloud KMS. BigQuery stores the name of the key used to
encrypt the table content and uses that key when a table protected by
Cloud KMS is queried.

All existing tools, the BigQuery console, and the bq command-line tool run
the same way as with default-encrypted tables, as long as
BigQuery has access to the Cloud KMS key used to encrypt
the table content.

### Protect query results with a Cloud KMS key

By default, query results are stored in a temporary table encrypted with a
Google-owned and Google-managed encryption key. If the project already has a
[default key](https://docs.cloud.google.com/bigquery/docs/customer-managed-encryption#project_default_key),
the key is applied to the temporary (default) query results table. To use a
Cloud KMS key to encrypt your query results instead, select one of the
following options:

### Console

1. Open the BigQuery page in the Google Cloud console.

   [Go to the BigQuery page](https://console.cloud.google.com/bigquery)
2. Click **Compose new query**.

3. Enter a valid GoogleSQL query in the query text area.

4. Click **More** , click **Query settings** , then click
   **Advanced options**.

5. Select **Customer-managed encryption**.

6. Select the key. If the key you want to use is not listed, enter the
   [resource ID](https://docs.cloud.google.com/bigquery/docs/customer-managed-encryption#key_resource_id) for the key.

7. Click **Save**.

8. Click **Run**.

### bq

Specify the flag `--destination_kms_key` to protect the destination table or
query results (if using a temporary table) with your Cloud KMS key.
The `--destination_kms_key` flag specifies the
[resource ID](https://docs.cloud.google.com/bigquery/docs/customer-managed-encryption#key_resource_id) of the key to use with the destination or
resulting table.

Optionally use the `--destination_table` flag to specify the destination for
query results. If `--destination_table` is not used, the query results are
written to a temporary table.

To query a table:

```
bq query \
--destination_table=DATASET_ID.TABLE_ID \
--destination_kms_key projects/KMS_PROJECT_ID/locations/LOCATION/keyRings/KEY_RING/cryptoKeys/KEY \
"SELECT name,count FROM DATASET_ID.TABLE_ID WHERE gender = 'M' ORDER BY count DESC LIMIT 6"
```

For more information about the bq command-line tool, see
[Using the bq command-line tool](https://docs.cloud.google.com/bigquery/bq-command-line-tool).

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

Protect a new table with a customer-managed encryption key by setting the [Table.encryption_configuration](https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.table.Table#google_cloud_bigquery_table_Table_encryption_configuration) property to an [EncryptionConfiguration](https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.encryption_configuration.EncryptionConfiguration) object before creating the table.

    import (
    	"context"
    	"fmt"
    	"io"

    	"cloud.google.com/go/bigquery"
    	"google.golang.org/api/iterator"
    )

    // queryWithDestinationCMEK demonstrates saving query results to a destination table and protecting those results
    // by specifying a customer managed encryption key.
    func queryWithDestinationCMEK(w io.Writer, projectID, dstDatasetID, dstTableID string) error {
    	// projectID := "my-project-id"
    	// datasetID := "mydataset"
    	// tableID := "mytable"
    	ctx := context.Background()
    	client, err := bigquery.NewClient(ctx, projectID)
    	if err != nil {
    		return fmt.Errorf("bigquery.NewClient: %v", err)
    	}
    	defer client.Close()

    	q := client.Query("SELECT 17 as my_col")
    	q.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_Job_Location = "US" // Location must match the dataset(s) referenced in query.
    	q.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_QueryConfig.Dst = client.Dataset(dstDatasetID).Table(dstTableID)
    	q.DestinationEncryptionConfig = &bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_EncryptionConfig{
    		// TODO: Replace this key with a key you have created in Cloud KMS.
    		KMSKeyName: "projects/cloud-samples-tests/locations/us-central1/keyRings/test/cryptoKeys/test",
    	}
    	// Run the query and print results when the query job is completed.
    	job, err := q.Run(ctx)
    	if err != nil {
    		return err
    	}
    	status, err := job.Wait(ctx)
    	if err != nil {
    		return err
    	}
    	if err := status.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_JobStatus_Err(); err != nil {
    		return err
    	}
    	it, err := job.Read(ctx)
    	for {
    		var row []bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_Value
    		err := it.Next(&row)
    		if err == iterator.Done {
    			break
    		}
    		if err != nil {
    			return err
    		}
    		fmt.Fprintln(w, row)
    	}
    	return nil
    }

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

Protect a new table with a customer-managed encryption key by setting the [Table.encryption_configuration](https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.table.Table#google_cloud_bigquery_table_Table_encryption_configuration) property to an [EncryptionConfiguration](https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.encryption_configuration.EncryptionConfiguration) object before creating the table.

    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryException.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryOptions.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.EncryptionConfiguration.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.QueryJobConfiguration.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableResult.html;

    // Sample to query on destination table with encryption key
    public class QueryDestinationTableCMEK {

      public static void runQueryDestinationTableCMEK() {
        // TODO(developer): Replace these variables before running the sample.
        String datasetName = "MY_DATASET_NAME";
        String tableName = "MY_TABLE_NAME";
        String kmsKeyName = "MY_KMS_KEY_NAME";
        String query =
            String.format("SELECT stringField, booleanField FROM %s.%s", datasetName, tableName);
        https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.EncryptionConfiguration.html encryption =
            https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.EncryptionConfiguration.html.newBuilder().setKmsKeyName(kmsKeyName).build();
        queryDestinationTableCMEK(query, encryption);
      }

      public static void queryDestinationTableCMEK(String query, https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.EncryptionConfiguration.html encryption) {
        try {
          // Initialize client that will be used to send requests. This client only needs to be created
          // once, and can be reused for multiple requests.
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html bigquery = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryOptions.html.getDefaultInstance().getService();

          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.QueryJobConfiguration.html config =
              https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.QueryJobConfiguration.html.newBuilder(query)
                  // Set the encryption key to use for the destination.
                  .setDestinationEncryptionConfiguration(encryption)
                  .build();

          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableResult.html results = bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html#com_google_cloud_bigquery_BigQuery_query_com_google_cloud_bigquery_QueryJobConfiguration_com_google_cloud_bigquery_BigQuery_JobOption____(config);

          results
              .https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableResult.html#com_google_cloud_bigquery_TableResult_iterateAll__()
              .forEach(row -> row.forEach(val -> System.out.printf("%s,", val.toString())));
          System.out.println("Query performed successfully with encryption key.");
        } catch (https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryException.html | InterruptedException e) {
          System.out.println("Query not performed \n" + e.toString());
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

Protect a query destination table with a customer-managed encryption key
by setting the [QueryJobConfig.destination_encryption_configuration](https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.job.QueryJobConfig#google_cloud_bigquery_job_QueryJobConfig_destination_encryption_configuration)
property to an
[EncryptionConfiguration](https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.encryption_configuration.EncryptionConfiguration)
and run the query.


    from google.cloud import https://docs.cloud.google.com/python/docs/reference/bigquery/latest

    # Construct a BigQuery client object.
    client = https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html()

    # TODO(developer): Set table_id to the ID of the destination table.
    # table_id = "your-project.your_dataset.your_table_name"

    # Set the encryption key to use for the destination.
    # TODO(developer): Replace this key with a key you have created in KMS.
    # kms_key_name = "projects/{}/locations/{}/keyRings/{}/cryptoKeys/{}".format(
    #     your-project, location, your-ring, your-key
    # )

    job_config = https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.job.QueryJobConfig.html(
        destination=table_id,
        destination_encryption_configuration=https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.encryption_configuration.EncryptionConfiguration.html(
            kms_key_name=kms_key_name
        ),
    )

    # Start the query, passing in the extra configuration.
    query_job = client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html(
        "SELECT 17 AS my_col;", job_config=job_config
    )  # Make an API request.
    https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.dbapi.Cursor.html#google_cloud_bigquery_dbapi_Cursor_query_job.result()  # Wait for the job to complete.

    table = client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_get_table(table_id)  # Make an API request.
    if table.encryption_configuration.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.encryption_configuration.EncryptionConfiguration.html#google_cloud_bigquery_encryption_configuration_EncryptionConfiguration_kms_key_name == kms_key_name:
        print("The destination table is written using the encryption configuration")

<br />

### Load a table protected by Cloud KMS

To load a data file into a table that is protected by Cloud KMS:

### Console

Protect a load job destination table with a customer-managed encryption key
by specifying the key when you load the table.

1. Open the BigQuery page in the Google Cloud console.

   [Go to the BigQuery page](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)
3. In the **Explorer** pane, expand your project, click **Datasets**, and then click a dataset. The dataset opens in a tab.

4. In the details pane, click **Create table**.

5. Enter the options you want to use for loading the table, but before you
   click **Create table** , click **Advanced options**.

6. Under **Encryption** , select **Customer-managed key**.

7. Click the **Select a customer-managed key** drop-down list and select
   the key to use. If you don't see any keys available, enter a
   [key resource ID](https://docs.cloud.google.com/bigquery/docs/customer-managed-encryption#key_resource_id).

   ![Advanced options.](https://docs.cloud.google.com/static/bigquery/images/bq_cmek_advanced_options.png)
8. Click **Create table**.

### bq

Protect a load job destination table with a customer-managed encryption
key by setting the `--destination_kms_key` flag.

```
bq --location=LOCATION load \
--autodetect \
--source_format=FORMAT \
--destination_kms_key projects/KMS_PROJECT_ID/locations/LOCATION/keyRings/KEY_RING/cryptoKeys/KEY \
DATASET.TABLE \
path_to_source
```
For example:

```
bq load \
--autodetect \
--source_format=NEWLINE_DELIMITED_JSON \
--destination_kms_key projects/KMS_PROJECT_ID/locations/LOCATION/keyRings/KEY_RING/cryptoKeys/KEY \
test2.table4 \
gs://cloud-samples-data/bigquery/us-states/us-states.json
```

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

    import (
    	"context"
    	"fmt"

    	"cloud.google.com/go/bigquery"
    )

    // importJSONWithCMEK demonstrates loading newline-delimited JSON from Cloud Storage,
    // and protecting the data with a customer-managed encryption key.
    func importJSONWithCMEK(projectID, datasetID, tableID string) error {
    	// projectID := "my-project-id"
    	// datasetID := "mydataset"
    	// tableID := "mytable"
    	ctx := context.Background()
    	client, err := bigquery.NewClient(ctx, projectID)
    	if err != nil {
    		return fmt.Errorf("bigquery.NewClient: %v", err)
    	}
    	defer client.Close()

    	gcsRef := bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_GCSReference_NewGCSReference("gs://cloud-samples-data/bigquery/us-states/us-states.json")
    	gcsRef.SourceFormat = bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_CSV_Avro_JSON_DatastoreBackup_GoogleSheets_Bigtable_Parquet_ORC_TFSavedModel_XGBoostBooster_Iceberg
    	gcsRef.AutoDetect = true
    	loader := client.Dataset(datasetID).Table(tableID).https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_Table_LoaderFrom(gcsRef)
    	loader.WriteDisposition = bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_WriteAppend_WriteTruncate_WriteTruncateData_WriteEmpty
    	loader.DestinationEncryptionConfig = &bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_EncryptionConfig{
    		// TODO: Replace this key with a key you have created in KMS.
    		KMSKeyName: "projects/cloud-samples-tests/locations/us-central1/keyRings/test/cryptoKeys/test",
    	}

    	job, err := loader.Run(ctx)
    	if err != nil {
    		return err
    	}
    	status, err := job.Wait(ctx)
    	if err != nil {
    		return err
    	}

    	if status.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_JobStatus_Err() != nil {
    		return fmt.Errorf("job completed with error: %v", status.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_JobStatus_Err())
    	}

    	return nil
    }

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

    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryException.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryOptions.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.EncryptionConfiguration.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.FormatOptions.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Job.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.JobInfo.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.LoadJobConfiguration.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableId.html;

    // Sample to load JSON data with configuration key from Cloud Storage into a new BigQuery table
    public class LoadJsonFromGCSCMEK {

      public static void runLoadJsonFromGCSCMEK() {
        // TODO(developer): Replace these variables before running the sample.
        String datasetName = "MY_DATASET_NAME";
        String tableName = "MY_TABLE_NAME";
        String kmsKeyName = "MY_KMS_KEY_NAME";
        String sourceUri = "gs://cloud-samples-data/bigquery/us-states/us-states.json";
        // i.e. projects/{project}/locations/{location}/keyRings/{key_ring}/cryptoKeys/{cryptoKey}
        https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.EncryptionConfiguration.html encryption =
            https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.EncryptionConfiguration.html.newBuilder().setKmsKeyName(kmsKeyName).build();
        loadJsonFromGCSCMEK(datasetName, tableName, sourceUri, encryption);
      }

      public static void loadJsonFromGCSCMEK(
          String datasetName, String tableName, String sourceUri, https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.EncryptionConfiguration.html encryption) {
        try {
          // Initialize client that will be used to send requests. This client only needs to be created
          // once, and can be reused for multiple requests.
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html bigquery = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryOptions.html.getDefaultInstance().getService();

          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableId.html tableId = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableId.html.of(datasetName, tableName);
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.LoadJobConfiguration.html loadConfig =
              https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.LoadJobConfiguration.html.newBuilder(tableId, sourceUri)
                  // Set the encryption key to use for the destination.
                  .setDestinationEncryptionConfiguration(encryption)
                  .setFormatOptions(https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.FormatOptions.html.json())
                  .setAutodetect(true)
                  .build();

          // Load data from a GCS JSON file into the table
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Job.html job = bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html#com_google_cloud_bigquery_BigQuery_create_com_google_cloud_bigquery_DatasetInfo_com_google_cloud_bigquery_BigQuery_DatasetOption____(JobInfo.of(loadConfig));
          // Blocks until this load table job completes its execution, either failing or succeeding.
          job = job.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Job.html#com_google_cloud_bigquery_Job_waitFor_com_google_cloud_bigquery_BigQueryRetryConfig_com_google_cloud_RetryOption____();
          if (job.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Job.html#com_google_cloud_bigquery_Job_isDone__()) {
            System.out.println("Table loaded succesfully from GCS with configuration key");
          } else {
            System.out.println(
                "BigQuery was unable to load into the table due to an error:"
                    + job.getStatus().getError());
          }
        } catch (https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryException.html | InterruptedException e) {
          System.out.println("Column not added during load append \n" + e.toString());
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

Protect a load job destination table with a customer-managed encryption
key by setting the
[LoadJobConfig.destination_encryption_configuration](https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.job.LoadJobConfig#google_cloud_bigquery_job_LoadJobConfig_destination_encryption_configuration)
property to an [EncryptionConfiguration](https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.encryption_configuration.EncryptionConfiguration)
and load the table.


    from google.cloud import https://docs.cloud.google.com/python/docs/reference/bigquery/latest

    # Construct a BigQuery client object.
    client = https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html()

    # TODO(developer): Set table_id to the ID of the table to create.
    # table_id = "your-project.your_dataset.your_table_name

    # Set the encryption key to use for the destination.
    # TODO: Replace this key with a key you have created in KMS.
    # kms_key_name = "projects/{}/locations/{}/keyRings/{}/cryptoKeys/{}".format(
    #     "cloud-samples-tests", "us", "test", "test"
    # )

    job_config = https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.job.LoadJobConfig.html(
        autodetect=True,
        source_format=https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.enums.SourceFormat.html.NEWLINE_DELIMITED_JSON,
        destination_encryption_configuration=https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.encryption_configuration.EncryptionConfiguration.html(
            kms_key_name=kms_key_name
        ),
    )

    uri = "gs://cloud-samples-data/bigquery/us-states/us-states.json"

    load_job = client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_load_table_from_uri(
        uri,
        table_id,
        location="US",  # Must match the destination dataset location.
        job_config=job_config,
    )  # Make an API request.

    assert load_job.job_type == "load"

    load_job.result()  # Waits for the job to complete.

    assert load_job.state == "DONE"
    table = client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_get_table(table_id)

    if table.encryption_configuration.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.encryption_configuration.EncryptionConfiguration.html#google_cloud_bigquery_encryption_configuration_EncryptionConfiguration_kms_key_name == kms_key_name:
        print("A table loaded with encryption configuration key")

<br />

## Stream into a table protected by Cloud KMS

You can stream data into your CMEK-protected BigQuery table
without specifying any additional parameters. Note that this data is encrypted
using your Cloud KMS key in the buffer as well as in the final
location. Before using streaming with a CMEK table, review the requirements on
[key availability and accessibility](https://docs.cloud.google.com/bigquery/docs/customer-managed-encryption#key_access).

Learn more about streaming at
[Streaming data using the BigQuery Storage Write API](https://docs.cloud.google.com/bigquery/docs/write-api).

## Change a table from default encryption to Cloud KMS protection

### bq

You can use the `bq cp` command with the `--destination_kms_key` flag
to copy a table protected by default encryption into a new table, or into
the original table, protected by Cloud KMS. The
`--destination_kms_key` flag specifies the [resource ID](https://docs.cloud.google.com/bigquery/docs/customer-managed-encryption#key_resource_id)
of the key to use with the destination table.

To copy a table that has default encryption to a new table that has
Cloud KMS protection:

```
bq cp \
--destination_kms_key projects/KMS_PROJECT_ID/locations/LOCATION/keyRings/KEY_RING/cryptoKeys/KEY \
SOURCE_DATASET_ID.SOURCE_TABLE_ID DESTINATION_DATASET_ID.DESTINATION_TABLE_ID
```

If you want to copy a table that has default encryption to the same table
with Cloud KMS protection:

```
bq cp -f \
--destination_kms_key projects/KMS_PROJECT_ID/locations/LOCATION/keyRings/KEY_RING/cryptoKeys/KEY \
DATASET_ID.TABLE_ID DATASET_ID.TABLE_ID
```

If you want to change a table from Cloud KMS protection to default
encryption, copy the file to itself by running `bq cp` without using the
`--destination_kms_key` flag.

For more information about the bq command-line tool, see
[Using the bq command-line tool](https://docs.cloud.google.com/bigquery/bq-command-line-tool).

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

    import (
    	"context"
    	"fmt"

    	"cloud.google.com/go/bigquery"
    )

    // copyTableWithCMEK demonstrates creating a copy of a table and ensuring the copied data is
    // protected with a customer managed encryption key.
    func copyTableWithCMEK(projectID, datasetID, tableID string) error {
    	// projectID := "my-project-id"
    	// datasetID := "mydataset"
    	// tableID := "mytable"
    	ctx := context.Background()
    	client, err := bigquery.NewClient(ctx, projectID)
    	if err != nil {
    		return fmt.Errorf("bigquery.NewClient: %v", err)
    	}
    	defer client.Close()

    	srcTable := client.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_Client_DatasetInProject("bigquery-public-data", "samples").Table("shakespeare")
    	copier := client.Dataset(datasetID).Table(tableID).https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_Table_CopierFrom(srcTable)
    	copier.DestinationEncryptionConfig = &bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_EncryptionConfig{
    		// TODO: Replace this key with a key you have created in Cloud KMS.
    		KMSKeyName: "projects/cloud-samples-tests/locations/us-central1/keyRings/test/cryptoKeys/test",
    	}
    	job, err := copier.Run(ctx)
    	if err != nil {
    		return err
    	}
    	status, err := job.Wait(ctx)
    	if err != nil {
    		return err
    	}
    	if err := status.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_JobStatus_Err(); err != nil {
    		return err
    	}
    	return nil
    }

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

    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryException.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryOptions.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.CopyJobConfiguration.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.EncryptionConfiguration.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Job.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.JobInfo.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableId.html;

    // Sample to copy a cmek table
    public class CopyTableCMEK {

      public static void runCopyTableCMEK() {
        // TODO(developer): Replace these variables before running the sample.
        String destinationDatasetName = "MY_DESTINATION_DATASET_NAME";
        String destinationTableId = "MY_DESTINATION_TABLE_NAME";
        String sourceDatasetName = "MY_SOURCE_DATASET_NAME";
        String sourceTableId = "MY_SOURCE_TABLE_NAME";
        String kmsKeyName = "MY_KMS_KEY_NAME";
        https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.EncryptionConfiguration.html encryption =
            https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.EncryptionConfiguration.html.newBuilder().setKmsKeyName(kmsKeyName).build();
        copyTableCMEK(
            sourceDatasetName, sourceTableId, destinationDatasetName, destinationTableId, encryption);
      }

      public static void copyTableCMEK(
          String sourceDatasetName,
          String sourceTableId,
          String destinationDatasetName,
          String destinationTableId,
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.EncryptionConfiguration.html encryption) {
        try {
          // Initialize client that will be used to send requests. This client only needs to be created
          // once, and can be reused for multiple requests.
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html bigquery = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryOptions.html.getDefaultInstance().getService();

          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableId.html sourceTable = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableId.html.of(sourceDatasetName, sourceTableId);
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableId.html destinationTable = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableId.html.of(destinationDatasetName, destinationTableId);

          // For more information on CopyJobConfiguration see:
          // https://googleapis.dev/java/google-cloud-clients/latest/com/google/cloud/bigquery/JobConfiguration.html
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.CopyJobConfiguration.html configuration =
              https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.CopyJobConfiguration.html.newBuilder(destinationTable, sourceTable)
                  .setDestinationEncryptionConfiguration(encryption)
                  .build();

          // For more information on Job see:
          // https://googleapis.dev/java/google-cloud-clients/latest/index.html?com/google/cloud/bigquery/package-summary.html
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Job.html job = bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html#com_google_cloud_bigquery_BigQuery_create_com_google_cloud_bigquery_DatasetInfo_com_google_cloud_bigquery_BigQuery_DatasetOption____(JobInfo.of(configuration));

          // Blocks until this job completes its execution, either failing or succeeding.
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Job.html completedJob = job.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Job.html#com_google_cloud_bigquery_Job_waitFor_com_google_cloud_bigquery_BigQueryRetryConfig_com_google_cloud_RetryOption____();
          if (completedJob == null) {
            System.out.println("Job not executed since it no longer exists.");
            return;
          } else if (completedJob.getStatus().getError() != null) {
            System.out.println(
                "BigQuery was unable to copy table due to an error: \n" + job.getStatus().getError());
            return;
          }
          System.out.println("Table cmek copied successfully.");
        } catch (https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryException.html | InterruptedException e) {
          System.out.println("Table cmek copying job was interrupted. \n" + e.toString());
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

Protect the destination of a table copy with a customer-managed
encryption key by setting the [QueryJobConfig.destination_encryption_configuration](https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.job.CopyJobConfig#google_cloud_bigquery_job_CopyJobConfig_destination_encryption_configuration)
property to an [EncryptionConfiguration](https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.encryption_configuration.EncryptionConfiguration)
and copy the table.


    from google.cloud import https://docs.cloud.google.com/python/docs/reference/bigquery/latest

    # Construct a BigQuery client object.
    client = https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html()

    # TODO(developer): Set dest_table_id to the ID of the destination table.
    # dest_table_id = "your-project.your_dataset.your_table_name"

    # TODO(developer): Set orig_table_id to the ID of the original table.
    # orig_table_id = "your-project.your_dataset.your_table_name"

    # Set the encryption key to use for the destination.
    # TODO(developer): Replace this key with a key you have created in KMS.
    # kms_key_name = "projects/{}/locations/{}/keyRings/{}/cryptoKeys/{}".format(
    #     your-project, location, your-ring, your-key
    # )

    job_config = https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.job.CopyJobConfig.html(
        destination_encryption_configuration=https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.encryption_configuration.EncryptionConfiguration.html(
            kms_key_name=kms_key_name
        )
    )
    job = client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_copy_table(orig_table_id, dest_table_id, job_config=job_config)
    https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.job.html.result()  # Wait for the job to complete.

    dest_table = client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_get_table(dest_table_id)  # Make an API request.
    if dest_table.encryption_configuration.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.encryption_configuration.EncryptionConfiguration.html#google_cloud_bigquery_encryption_configuration_EncryptionConfiguration_kms_key_name == kms_key_name:
        print("A copy of the table created")

<br />

## Determine if a table is protected by Cloud KMS

1. In the Google Cloud console, click the blue arrow to
   the left of your dataset to expand it, or double-click the dataset name. This
   displays the tables and views in the dataset.

2. Click the table name.

3. Click **Details** . The **Table Details** page displays the table's
   description and table information.

4. If the table is protected by Cloud KMS, the **Customer-Managed
   Encryption Key** field displays the key resource ID.

   ![Protected table.](https://docs.cloud.google.com/static/bigquery/images/bq_cmek_table.png)

For each of the keys you've created or that protect your tables, you can see
what resources that key protects with key usage tracking. For more information,
see [View key usage](https://docs.cloud.google.com/kms/docs/view-key-usage).

## Change the Cloud KMS key for a BigQuery table

To change the Cloud KMS key of an existing CMEK-protected table, you
can run an `ALTER TABLE` query, use the API, or use the bq command-line tool.
There are two ways to modify the Cloud KMS key using the API and the
bq command-line tool: `update` or `cp`.

If you use `update`, you can change the Cloud KMS key used for a
CMEK-protected table.

If you use `cp`, you can change the Cloud KMS key used for a
CMEK-protected table, change a table from default encryption to CMEK-protection,
or change a table from CMEK-protection to default encryption.

An advantage of `update` is it is faster than `cp` and it lets you use
[table decorators](https://docs.cloud.google.com/bigquery/docs/table-decorators).

### SQL

Use the
[`ALTER TABLE SET OPTIONS` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#alter_table_set_options_statement)
to update the `kms_key_name` field for a table:

<br />

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, enter the following statement:

   ```googlesql
   ALTER TABLE DATASET_ID.mytable
   SET OPTIONS (
     kms_key_name
       = 'projects/KMS_PROJECT_ID/locations/LOCATION/keyRings/KEY_RING/cryptoKeys/KEY');
   ```

   <br />

3. Click **Run**.

   <br />

For more information about how to run queries, see [Run an interactive query](https://docs.cloud.google.com/bigquery/docs/running-queries#queries).

### bq

You can use the `bq cp` command with the `--destination_kms_key` flag
to change the key for a table protected by Cloud KMS. The
`--destination_kms_key` flag specifies the [resource ID](https://docs.cloud.google.com/bigquery/docs/customer-managed-encryption#key_resource_id)
of the key to use with the table.

```
bq update \
--destination_kms_key projects/KMS_PROJECT_ID/locations/LOCATION/keyRings/KEY_RING/cryptoKeys/KEY \
-t DATASET_ID.TABLE_ID
```

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

    import (
    	"context"
    	"fmt"

    	"cloud.google.com/go/bigquery"
    )

    // updateTableChangeCMEK demonstrates how to change the customer managed encryption key that protects a table.
    func updateTableChangeCMEK(projectID, datasetID, tableID string) error {
    	// projectID := "my-project-id"
    	// datasetID := "mydatasetid"
    	// tableID := "mytableid"
    	ctx := context.Background()

    	client, err := bigquery.NewClient(ctx, projectID)
    	if err != nil {
    		return fmt.Errorf("bigquery.NewClient: %v", err)
    	}
    	defer client.Close()

    	tableRef := client.Dataset(datasetID).Table(tableID)
    	meta, err := tableRef.Metadata(ctx)
    	if err != nil {
    		return err
    	}
    	update := bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_TableMetadataToUpdate{
    		EncryptionConfig: &bigquery.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/index.html#cloud_google_com_go_bigquery_EncryptionConfig{
    			// TODO: Replace this key with a key you have created in Cloud KMS.
    			KMSKeyName: "projects/cloud-samples-tests/locations/us-central1/keyRings/test/cryptoKeys/otherkey",
    		},
    	}
    	if _, err := tableRef.Update(ctx, update, meta.ETag); err != nil {
    		return err
    	}
    	return nil
    }

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

    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryException.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryOptions.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.EncryptionConfiguration.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-biglake/latest/com.google.cloud.bigquery.biglake.v1.Table.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableId.html;

    // Sample to update a cmek table
    public class UpdateTableCMEK {

      public static void runUpdateTableCMEK() {
        // TODO(developer): Replace these variables before running the sample.
        String datasetName = "MY_DATASET_NAME";
        String tableName = "MY_TABLE_NAME";
        String kmsKeyName = "MY_KEY_NAME";
        // Set a new encryption key to use for the destination.
        // i.e. projects/{project}/locations/{location}/keyRings/{key_ring}/cryptoKeys/{cryptoKey}
        https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.EncryptionConfiguration.html encryption =
            https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.EncryptionConfiguration.html.newBuilder().setKmsKeyName(kmsKeyName).build();
        updateTableCMEK(datasetName, tableName, encryption);
      }

      public static void updateTableCMEK(
          String datasetName, String tableName, https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerydatatransfer/latest/com.google.cloud.bigquery.datatransfer.v1.EncryptionConfiguration.html encryption) {
        try {
          // Initialize client that will be used to send requests. This client only needs to be created
          // once, and can be reused for multiple requests.
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html bigquery = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryOptions.html.getDefaultInstance().getService();

          https://docs.cloud.google.com/java/docs/reference/google-cloud-biglake/latest/com.google.cloud.bigquery.biglake.v1.Table.html table = bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html#com_google_cloud_bigquery_BigQuery_getTable_com_google_cloud_bigquery_TableId_com_google_cloud_bigquery_BigQuery_TableOption____(TableId.of(datasetName, tableName));
          bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html#com_google_cloud_bigquery_BigQuery_update_com_google_cloud_bigquery_DatasetInfo_com_google_cloud_bigquery_BigQuery_DatasetOption____(table.toBuilder().setEncryptionConfiguration(encryption).build());
          System.out.println("Table cmek updated successfully");
        } catch (https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryException.html e) {
          System.out.println("Table cmek was not updated. \n" + e.toString());
        }
      }
    }

### Python


Change the customer-managed encryption key for a table by changing the
[Table.encryption_configuration](https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.table.Table#google_cloud_bigquery_table_Table_encryption_configuration)
property to a new [EncryptionConfiguration](https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.encryption_configuration.EncryptionConfiguration)
object and update the table.


Before trying this sample, follow the Python setup instructions in the
[BigQuery quickstart using
client libraries](https://docs.cloud.google.com/bigquery/docs/quickstarts/quickstart-client-libraries).


For more information, see the
[BigQuery Python API
reference documentation](https://docs.cloud.google.com/python/docs/reference/bigquery/latest).


To authenticate to BigQuery, set up Application Default Credentials.
For more information, see

[Set up authentication for client libraries](https://docs.cloud.google.com/bigquery/docs/authentication#client-libs).

    # from google.cloud import bigquery
    # client = bigquery.Client()

    assert table.encryption_configuration.kms_key_name == original_kms_key_name

    # Set a new encryption key to use for the destination.
    # TODO: Replace this key with a key you have created in KMS.
    updated_kms_key_name = (
        "projects/cloud-samples-tests/locations/us/keyRings/test/cryptoKeys/otherkey"
    )
    table.encryption_configuration = bigquery.EncryptionConfiguration(
        kms_key_name=updated_kms_key_name
    )

    table = client.update_table(table, ["encryption_configuration"])  # API request

    assert table.encryption_configuration.kms_key_name == updated_kms_key_name
    assert original_kms_key_name != updated_kms_key_name

<br />

## Set a dataset default key

You can set a dataset-wide default Cloud KMS key that applies to all
newly created tables within the dataset, unless a different Cloud KMS
key is specified when you create the table. The default key does not apply to
existing tables. Changing the default key does not modify any existing tables
and applies only to new tables created after the change.

To apply, change, or remove a dataset default key, select one of the following
options:

### SQL

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, enter one of the following statements:

   - To set the default key when creating a dataset, use the [`CREATE SCHEMA` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_schema_statement) with the `default_kms_key_name` option:

     ```googlesql
     CREATE SCHEMA PROJECT_ID.DATASET_ID
     OPTIONS (
       default_kms_key_name = 'projects/KMS_PROJECT_ID/locations/LOCATION/keyRings/KEY_RING/cryptoKeys/KEY',
       location = 'LOCATION');
     ```
   - To change the default key for a dataset, use the [`ALTER SCHEMA SET OPTIONS` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#alter_schema_set_options_statement):

     ```googlesql
     ALTER SCHEMA PROJECT_ID.DATASET_ID
     SET OPTIONS (
       default_kms_key_name = 'projects/KMS_PROJECT_ID/locations/LOCATION/keyRings/KEY_RING/cryptoKeys/KEY');
     ```
   - To remove the default key from a dataset, set `default_kms_key_name` to `NULL`:

     ```googlesql
     ALTER SCHEMA PROJECT_ID.DATASET_ID
     SET OPTIONS (
       default_kms_key_name = NULL);
     ```
3. Click **Run**.

### bq


To set the default key when creating a dataset, use the `bq mk` command with the `--dataset` and `--default_kms_key` flags:

```
bq mk --dataset 

--default_kms_key projects/KMS_PROJECT_ID/locations/LOCATION/keyRings/KEY_RING/cryptoKeys/KEY 

PROJECT_ID:DATASET_ID
```

<br />

To set or change the default key for an existing dataset, use the `bq update` command with the `--dataset` and `--default_kms_key` flags:

```
bq update --dataset 

--default_kms_key projects/KMS_PROJECT_ID/locations/LOCATION/keyRings/KEY_RING/cryptoKeys/KEY 

PROJECT_ID:DATASET_ID
```

<br />

To remove the default key from a dataset, use the `bq update` command with the `--dataset` flag and set `--default_kms_key` to `""`:

```
bq update --dataset --default_kms_key="" PROJECT_ID:DATASET_ID
```

<br />

### API


To set or change the default key, specify the default key in the [`EncryptionConfiguration.kmsKeyName`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/EncryptionConfiguration#FIELDS.kms_key_name) field when you call the [`datasets.insert`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets/insert) or [`datasets.patch`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets/patch) methods.

To remove the default key, set [`EncryptionConfiguration.kmsKeyName`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/EncryptionConfiguration#FIELDS.kms_key_name) to null when calling the [`datasets.patch`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets/patch) method.

> [!NOTE]
> **Note:** Within a dataset that has a default Cloud KMS key applied, it is not possible to encrypt new tables with a key other than a customer-managed encryption key. That is, if the dataset has a default Cloud KMS key, you cannot use a non-customer-managed encryption key to encrypt new tables in the dataset.

## Set a project default key

You can set project-default Cloud KMS keys that apply to all
query results and newly created tables in the project for that location, unless
you specify a different Cloud KMS key. The default key also applies to
newly created cached results tables that are stored in
[anonymous datasets](https://docs.cloud.google.com/bigquery/docs/cached-results).

The default key does not apply to existing tables. Changing the default key does
not modify any existing tables and applies only to new tables created after the
change.

### SQL

Use the
[`ALTER PROJECT SET OPTIONS` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#alter_project_set_options_statement)
to update the `default_kms_key_name` field for a project. You can find the
resource name for the key on the Cloud KMS page.

<br />

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, enter the following statement:

   ```googlesql
   ALTER PROJECT PROJECT_ID
   SET OPTIONS (
     `region-LOCATION.default_kms_key_name`
       = 'projects/KMS_PROJECT_ID/locations/LOCATION/keyRings/KEY_RING/cryptoKeys/KEY');
   ```

   <br />

3. Click **Run**.

   <br />

For more information about how to run queries, see [Run an interactive query](https://docs.cloud.google.com/bigquery/docs/running-queries#queries).

### bq

You can use the `bq` command to run an
[`ALTER PROJECT SET OPTIONS` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#alter_project_set_options_statement)
to update the `default_kms_key_name` field for a project:

```
bq query --nouse_legacy_sql \
  'ALTER PROJECT PROJECT_ID
  SET OPTIONS (
  `region-LOCATION.default_kms_key_name`
    ="projects/KMS_PROJECT_ID/locations/LOCATION/keyRings/KEY_RING/cryptoKeys/KEY");'
```

## Use CMEK to protect BigQuery ML models

BigQuery ML supports CMEK. Along with the default encryption provided
by BigQuery, you can use your own Cloud Key Management Service keys for encrypting
machine learning models, including imported TensorFlow models.

### Create an encrypted model with a Cloud KMS key

To create an encrypted model, use the
[`CREATE MODEL` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create)
and specify `KMS_KEY_NAME` in the training options:

```googlesql
    CREATE MODEL my_dataset.my_model
    OPTIONS(
      model_type='linear_reg',
      input_label_cols=['your_label'],
      kms_key_name='projects/my_project/locations/my_location/keyRings/my_ring/cryptoKeys/my_key')
    AS SELECT * FROM my_dataset.my_data
```

The same syntax also applies to imported TensorFlow models:

```googlesql
    CREATE MODEL my_dataset.my_model
    OPTIONS(
      model_type='tensorflow',
      path='gs://bucket/path/to/saved_model/*',
      kms_key_name='projects/my_project/locations/my_location/keyRings/my_ring/cryptoKeys/my_key')
    AS SELECT * FROM my_dataset.my_data
```

### Limitations

Customer-managed encryption keys have the following restrictions when
encrypting machine learning models:

- `Global` region CMEK keys are not supported for the following
  types of models:

  - [DNN](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-dnn-models)
  - [Wide-and-Deep](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-wnd-models)
  - [Autoencoder](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-autoencoder)
  - [Boosted tree](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-boosted-tree)
- `Global` region CMEK keys and multi-region CMEK keys, for example `EU` or
  `US`, are not supported when creating the following types of models:

  - [AutoML Tables models](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-automl)
- CMEK keys aren't supported for remote models:

  - [Remote models over Vertex AI built-in models](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model)
  - [Remote models over Vertex AI hosted models](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model-https)
  - [Remote models over Cloud AI services](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model-service)

### Change a model from default encryption to Cloud KMS protection

You can use the [`bq cp` command](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#bq_cp)
with the `--destination_kms_key` flag to copy a model protected by default
encryption into a new model that is protected by Cloud KMS.
Alternatively, you can use the `bq cp` command with the `-f` flag to overwrite a
model protected by default encryption and update it to use Cloud KMS
protection instead. The `--destination_kms_key` flag specifies the
[resource ID](https://docs.cloud.google.com/bigquery/docs/customer-managed-encryption#key_resource_id) of the key to use with the destination model.

To copy a model that has default encryption to a new model that has
Cloud KMS protection:

```
bq cp \
--destination_kms_key projects/KMS_PROJECT_ID/locations/LOCATION/keyRings/KEY_RING/cryptoKeys/KEY \
SOURCE_DATASET_ID.SOURCE_MODEL_ID DESTINATION_DATASET_ID.DESTINATION_MODEL_ID
```

To overwrite a model that has default encryption to the same model
with Cloud KMS protection:

```
bq cp -f \
--destination_kms_key projects/KMS_PROJECT_ID/locations/LOCATION/keyRings/KEY_RING/cryptoKeys/KEY \
DATASET_ID.MODEL_ID DATASET_ID.MODEL_ID
```

To change a model from Cloud KMS protection to default encryption:

```
bq cp -f \
DATASET_ID.MODEL_ID DATASET_ID.MODEL_ID
```

For more information about the bq command-line tool, see
[Using the bq command-line tool](https://docs.cloud.google.com/bigquery/bq-command-line-tool).

### Determine if a model is protected by Cloud KMS

Use the
[`bq show` command](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#bq_show)
to see if a model is protected by Cloud KMS key. The
encryption key is in the `kmsKeyName` field.

```
bq show -m my_dataset.my_model
```

You can also use the Google Cloud console to find the Cloud KMS
key for an encrypted model. CMEK information is in the **Customer-managed key**
field in the **Model Details** section of the model's **Details** pane.

### Change the Cloud KMS key for an encrypted model

Use the
[`bq update` command](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#bq_update) with
the `--destination_kms_key` flag to change the key for a model protected by
Cloud KMS:

```
bq update --destination_kms_key \
projects/my_project/locations/my_location/keyRings/my_ring/cryptoKeys/my_key \
-t my_dataset.my_model
```

### Use default project or dataset keys

If you have a default Cloud KMS key set at the project or dataset level,
BigQuery ML automatically uses this key when creating models.
Use the `CREATE MODEL` statement to specify a different key to encrypt the model
if you don't want to use the default key.

### Use BigQuery ML functions with encrypted models

You can use all BigQuery ML functions with an encrypted model
without specifying an encryption key.

## Use CMEK to protect BigQuery Connection API

For Cloud SQL connections, you can protect your BigQuery Connection API credentials using CMEK.

For more information about how to create a CMEK-protected connection, see [Create Cloud SQL connections](https://docs.cloud.google.com/bigquery/docs/connect-to-sql#create-sql-connection).

## Use CMEK to protect BigQuery Studio code assets

To use CMEK to protect your BigQuery Studio code assets,
you must set a default Dataform CMEK key for the Google Cloud project
that contains your code assets. Code assets include the following:

- [Saved queries](https://docs.cloud.google.com/bigquery/docs/work-with-saved-queries)
- [Notebooks](https://docs.cloud.google.com/bigquery/docs/create-notebooks)
- [Data canvases](https://docs.cloud.google.com/bigquery/docs/data-canvas)
- [Data preparations](https://docs.cloud.google.com/bigquery/docs/data-prep-get-suggestions)

After you set a default
Dataform CMEK key, Dataform applies the key to
all new resources created in the Google Cloud project by default,
including any hidden resources created for storing your code assets.

The default Dataform CMEK key isn't applied to existing
resources. If you already have code assets in that project, they won't be
encrypted by the default Dataform CMEK key. To use CMEK with a
code asset that was created before you set your project's default
Dataform CMEK key, you can save the asset as a new
BigQuery Studio code asset.

Setting Dataform default CMEK configuration for
BigQuery code assets through Terraform isn't supported.
Instead, use the Dataform API. This configuration must be applied
on a per-project basis, not at the organization level. For instructions, see
[Set a default Dataform CMEK key](https://docs.cloud.google.com/dataform/docs/cmek#set-default-key).

## Remove BigQuery's access to the Cloud KMS key

You can remove BigQuery's access to the Cloud KMS key at
any time, by revoking the Identity and Access Management (IAM) permission for that key.

> [!NOTE]
> **Note:** If you remove BigQuery's access to a key, this change doesn't take place instantly, but happens within an hour while the IAM permission change propagates. If you are running a query at the time the query still completes, but the results might not be viewable.

If BigQuery loses access to the Cloud KMS key, the user
experience can suffer significantly and data loss can occur:

- Data in these CMEK-protected tables can no longer be accessed: `query`,
  `cp`, `extract`, and `tabledata.list` will all fail.

- No new data can be added to these CMEK-protected tables.

- After access is granted back, the performance of queries to these tables
  can be degraded for multiple days.

## Control CMEK use with organization policy

BigQuery integrates with CMEK
[organization policy constraints](https://docs.cloud.google.com/resource-manager/docs/organization-policy/org-policy-constraints)
to let you specify encryption compliance requirements for
BigQuery resources in your organization.

This integration lets you do the following:

- Require CMEKs for all BigQuery resources in a project.

- Restrict which Cloud KMS keys can be used to protect resources in a
  project.

### Require CMEKs for all resources

A common policy is to require CMEKs to be used to protect all resources in a
specific set of projects. You can use the
`constraints/gcp.restrictNonCmekServices` constraint to enforce this policy in
BigQuery.

If set, this organization policy causes all resource creation requests without a
specified Cloud KMS key to fail.

After you set this policy, it applies only to new resources in the project. Any
existing resources without Cloud KMS keys set continue to exist and are
accessible without issue.

> [!NOTE]
> **Note:** This constraint only applies to resources that contain data. Resources that only use their specified Cloud KMS key as a default for new resources (for instance, datasets and project config) can still be created or updated without a Cloud KMS key.

### Console

1. Open the **Organization policies** page.

   [Go to Organization policies](https://console.cloud.google.com/iam-admin/orgpolicies/list)
2. In the **Filter** field, enter
   `constraints/gcp.restrictNonCmekServices`, and then click
   **Restrict which services may create resources without CMEK**.

3. Click **Edit**.

4. Select **Customize** , select **Replace** , and then click **Add Rule**.

5. Select **Custom** , then click **Deny**.

6. In the **Custom Value** field, enter `is:bigquery.googleapis.com`.

7. Click **Done** , and then click **Save**.

### gcloud

```sh
  gcloud resource-manager org-policies --project=PROJECT_ID \
    deny gcp.restrictNonCmekServices is:bigquery.googleapis.com
```

To verify that the policy is successfully applied, you can try to create a table
in the project. The process fails unless you specify a Cloud KMS
key.

This policy also applies to query results tables in the project. You can specify
a [project default key](https://docs.cloud.google.com/bigquery/docs/customer-managed-encryption#project_default_key) so users don't have to manually
specify a key each time they execute a query in the project.

### Restrict Cloud KMS keys for a BigQuery project

You can use the `constraints/gcp.restrictCmekCryptoKeyProjects` constraint to
restrict the Cloud KMS keys that you can use to protect a resource in a
BigQuery project.

You might specify a rule - for example, "For all BigQuery
resources in projects/my-company-data-project, Cloud KMS keys used in
this project must come from projects/my-company-central-keys OR
projects/team-specific-keys."

### Console

1. Open the **Organization policies** page.

   [Go to Organization policies](https://console.cloud.google.com/iam-admin/orgpolicies/list)
2. In the **Filter** field, enter
   `constraints/gcp.restrictCmekCryptoKeyProjects`, and then click
   **Restrict which projects may supply KMS CryptoKeys for CMEK**.

3. Click **Edit**.

4. Select **Customize** , select **Replace** , and then click **Add Rule**.

5. Select **Custom** , then click **Allow**.

6. In the **Custom Value** field, enter `under:projects/<var>KMS_PROJECT_ID</var>`.

7. Click **Done** , and then click **Save**.

### gcloud

```sh
  gcloud resource-manager org-policies --project=PROJECT_ID \
    allow gcp.restrictCmekCryptoKeyProjects under:projects/KMS_PROJECT_ID
```

To verify that the policy is successfully applied, you can try to create a table
using a Cloud KMS key from a different project. The process will fail.

### Limitations of organization policies

There are limitations associated with setting an organization policy.

#### Propagation delay

After you set or update an organization policy, it can take up to 15 minutes
for the new policy to take effect. BigQuery caches policies in
order to not negatively affect query and table creation latency.

#### Required permissions to set an organization policy

The permission to set or update the organization policy might be difficult to
acquire for testing purposes. You must be granted the
[Organization Policy Administrator role](https://docs.cloud.google.com/resource-manager/docs/organization-policy/using-constraints#required-roles),
which can only be granted at the organization level (rather than the project or
folder level).

Although the role must be granted at the organization level, it is still
possible to specify a policy that only applies to a specific project
or folder.

## Impact of Cloud KMS key rotation

BigQuery doesn't automatically rotate a table encryption key when
the Cloud KMS key associated with the table is rotated. All data in the
existing tables continue to be protected by the key version with which they were
created.

To update a table to use the most recent key version, update the table with the
same Cloud KMS key. This update won't check any organization policy.
Only updating the key will check the organization policy.

If there is a default key on the dataset, and you rotate the key, any new tables
created in the dataset after key rotation use the latest key version.

## Impact on Cloud KMS billing

When you create or truncate a CMEK-protected table, BigQuery
generates an intermediate key-encryption key which is then encrypted with the
specified Cloud KMS key.

For billing purposes, this means that neither your calls to Cloud KMS
nor their associated costs scale with the table size. For CMEK-protected tables,
you can expect one call to Cloud KMS
[`cryptoKeys.encrypt`](https://docs.cloud.google.com/kms/docs/reference/rest/v1/projects.locations.keyRings.cryptoKeys/encrypt)
for each table creation or truncation and one call to Cloud KMS
[`cryptoKeys.decrypt`](https://docs.cloud.google.com/kms/docs/reference/rest/v1/projects.locations.keyRings.cryptoKeys/decrypt)
for each table involved in a query. These methods both belong to the category of
**Key operations: Cryptographic** listed in
[Cloud KMS Pricing](https://cloud.google.com/kms/pricing).

Either reading from or writing to an existing CMEK-protected table invokes
Cloud KMS `cryptoKeys.decrypt` because the intermediate key must be
decrypted.

## Limitations

### BigQuery access to the Cloud KMS key

A Cloud KMS key is considered available and accessible by
BigQuery under the following conditions:

- The key is [enabled](https://docs.cloud.google.com/kms/docs/key-states#enabled)
- The BigQuery service account has encrypt and decrypt permissions on the key

The following sections describe impact to streaming inserts and long-term
inaccessible data when a key is inaccessible.

#### Impact to streaming inserts

The Cloud KMS key must be available and accessible for at least 24
consecutive hours in the 48-hour period following a streaming insertion request.
If the key is not available and accessible, the streamed data might not be fully
persisted and can be lost. For more information about streaming inserts, see
[Streaming data into BigQuery](https://docs.cloud.google.com/bigquery/docs/streaming-data-into-bigquery).

#### Impact to long-term inaccessible data

As BigQuery provides managed storage, long-term inaccessible data
is not compatible with BigQuery's architecture. If the
Cloud KMS key of a given BigQuery table is not available
and not accessible for 60 consecutive days, BigQuery might choose
to delete the table and its associated data. At least 7 days before the
data is deleted, BigQuery sends an email to the email address
associated with the billing account.

### Using external data sources

If you are querying data stored in an [external data source](https://docs.cloud.google.com/bigquery/docs/external-data-sources)
such as Cloud Storage that has CMEK-encrypted
data, then the data encryption is managed by [Cloud Storage](https://docs.cloud.google.com/storage/docs/encryption/customer-managed-keys).
For example, BigLake tables support data encrypted with CMEK in
Cloud Storage.

BigQuery and [BigLake tables](https://docs.cloud.google.com/bigquery/docs/biglake-intro)
don't support Customer-Supplied Encryption Keys (CSEK).

### Switching between CMEK-protected and default encryption

You cannot switch a table in place between default encryptions and CMEK
encryption. To switch encryption, [copy the
table](https://docs.cloud.google.com/bigquery/docs/managing-tables#copy-table) with destination encryption
information set or use a `SELECT *` query to select the table into itself with
`WRITE_TRUNCATE` disposition.

### Using table decorators

If you protect a table with Cloud KMS and then replace the data in the
table by using the value `WRITE_TRUNCATE` for a `load`, `cp`, or `query`
operation, then [range decorators](https://docs.cloud.google.com/bigquery/docs/table-decorators#range_decorators)
don't work across the encryption change boundary. You can still use table
decorators, including range decorators, to query the data before or after the
boundary, or query the snapshot at a point in time.

### Wildcard table queries

CMEK-protected tables cannot be queried with a [wildcard
suffix](https://docs.cloud.google.com/bigquery/docs/querying-wildcard-tables).

### Script support

[Scripts](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/procedural-language)
cannot define destination tables for CMEK operations.

### Editions support

CMEK support for BigQuery is only available for BigQuery Enterprise, BigQuery Enterprise Plus and BigQuery On-Demand.

> [!NOTE]
> **Note:** Using CMEK to query data in a BigQuery project from a project that does not support CMEK is allowed, however the project storing the data must use a compatible [BigQuery edition](https://docs.cloud.google.com/bigquery/docs/customer-managed-encryption#editions-support) and its service account must have the appropriate [Cloud KMS key permissions](https://docs.cloud.google.com/bigquery/docs/customer-managed-encryption#grant_permission).

### BigQuery Studio support

BigQuery Studio code assets support CMEK. Code assets include the
following:

- [Saved queries](https://docs.cloud.google.com/bigquery/docs/work-with-saved-queries)
- [Notebooks](https://docs.cloud.google.com/bigquery/docs/create-notebooks)
- [Data canvases](https://docs.cloud.google.com/bigquery/docs/data-canvas)
- [Data preparations](https://docs.cloud.google.com/bigquery/docs/data-prep-get-suggestions)

For more information, see
[Use CMEK to protect BigQuery Studio code assets](https://docs.cloud.google.com/bigquery/docs/customer-managed-encryption#cmek-studio).

## Frequently asked questions

### Who needs permission to the Cloud KMS key?

With customer-managed encryption keys, specifying permissions repeatedly is not
required. As long as the BigQuery service account has permission
to use the Cloud KMS key to encrypt and decrypt, anyone with permission
to the BigQuery table can access the data, even if they
don't have direct access to the Cloud KMS key.

### Which service account is used?

The BigQuery service account associated with the
Google Cloud project of the table is used to decrypt that table's data.
The BigQuery service accounts are unique for each project. For a
job that writes data into a Cloud KMS-protected anonymous table, the
job's project's service account is used.

As an example, consider three CMEK-protected tables: `table1`, `table2`, and
`table3`. To query data from `{project1.table1, project2.table2}` with
destination table `{project3.table3}`:

- Use the `project1` service account for `project1.table1`
- Use the `project2` service account for `project2.table2`
- Use the `project3` service account for `project3.table3`

### In what ways can BigQuery use my Cloud KMS key?

BigQuery uses the Cloud KMS key to decrypt data in
response to a user query, for example,
[`tabledata.list`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tabledata/list) or
[`jobs.insert`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs/insert).

BigQuery can also use the key for data maintenance and storage
optimization tasks, like data conversion into a read-optimized format.

### What cryptography libraries are used?

BigQuery relies on Cloud KMS for CMEK functionality.
Cloud KMS uses
[Tink](https://github.com/google/tink) for
encryption.

### How to get more help?

If you have questions that are not answered here, see
[BigQuery support](https://docs.cloud.google.com/bigquery/docs/getting-support).

## Troubleshooting errors

The following describes common errors and recommended mitigations.

| Error | Recommendation |
|---|---|
| Please grant Cloud KMS CryptoKey Encrypter/Decrypter role | The BigQuery service account associated with your project doesn't have sufficient IAM permission to operate on the specified Cloud KMS key. Follow the instructions in the error or [in this documentation](https://docs.cloud.google.com/bigquery/docs/customer-managed-encryption#grant_permission) to grant the proper IAM permission. |
| Existing table encryption settings don't match encryption settings specified in the request | This can occur in scenarios where the destination table has encryption settings that don't match the encryption settings in your request. As mitigation, use write disposition `TRUNCATE` to replace the table, or specify a different destination table. |
| This region is not supported | The region of the Cloud KMS key does not match the region of the BigQuery dataset of the destination table. As a mitigation, select a key in a region that matches your dataset, or load into a dataset that matches the key region. |
| Your administrator requires that you specify an encryption key for queries in project <var translate="no">PROJECT_ID.</var> | An organization policy prevented creating a resource or running a query. To learn more about this policy, see [Requiring CMEKs for all resources in a BigQuery project](https://docs.cloud.google.com/bigquery/docs/customer-managed-encryption#services_constraint). |
| Your administrator prevents using KMS keys from project <var translate="no">KMS_PROJECT_ID</var> to protect resources in project <var translate="no">PROJECT_ID</var>. | An organization policy prevented creating a resource or running a query. To learn more about this policy, see [Restrict Cloud KMS keys for a BigQuery project](https://docs.cloud.google.com/bigquery/docs/customer-managed-encryption#projects_constraint). |