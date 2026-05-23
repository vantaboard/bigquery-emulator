# Schedule a Snowflake transfer

The Snowflake connector provided by the BigQuery Data Transfer Service lets you
schedule and manage automated transfer jobs to migrate data from Snowflake
into BigQuery using public IP allowlists.

## Overview

The Snowflake connector engages migration agents in the
Google Kubernetes Engine and triggers a load operation from Snowflake to a
staging area within the same cloud provider where Snowflake is
hosted.

- For Amazon Web Services (AWS)-hosted Snowflake accounts, the data is first staged in your Amazon S3 bucket, which is then transferred to BigQuery with the BigQuery Data Transfer Service.
- For Google Cloud-hosted Snowflake accounts, the data is first staged in your Cloud Storage bucket, which is then transferred to BigQuery with the BigQuery Data Transfer Service.
- For Azure-hosted Snowflake accounts, the data is first staged in your Azure Blob Storage container, which is then transferred to BigQuery with the BigQuery Data Transfer Service.

The following diagram compares data transfers from Snowflake
accounts hosted on other cloud providers, and Snowflake accounts
hosted on Google Cloud.

![Data transfers from AWS or Azure-hosted Snowflake accounts and Google Cloud-hosted Snowflake accounts to BigQuery](https://docs.cloud.google.com/static/bigquery/images/snowflake-dts-overview-diagram.png)

## Limitations

Data transfers made using the Snowflake connector are subject to
the following limitations:

- The Snowflake connector only supports transfers from tables within a single Snowflake database and schema. To transfer from tables with multiple Snowflake databases or schemas, you can set up each transfer job separately.
- The speed of loading data from Snowflake to your Amazon S3 bucket or Azure Blob Storage container or Cloud Storage bucket is limited by the Snowflake warehouse you have chosen for this transfer.
- BigQuery writes data from Snowflake to Cloud Storage as Parquet files. Parquet files don't support the [`TIMESTAMP_TZ` and `TIMESTAMP_LTZ`](https://community.snowflake.com/s/article/How-To-Unload-Timestamp-data-in-a-Parquet-file) data types. If your data contains these types, you can export it to Amazon S3 as CSV files and then import the CSV files into BigQuery. For more information, see [Overview of
  Amazon S3 transfers](https://docs.cloud.google.com/bigquery/docs/s3-transfer-intro).

## Before you begin

Before you set up a Snowflake transfer, you must perform all the
steps listed in this section. The following is a list of all required steps.

1. [Prepare your Google Cloud project](https://docs.cloud.google.com/bigquery/docs/migration/snowflake-transfer#preparing-gcp-project)
2. [Required BigQuery roles](https://docs.cloud.google.com/bigquery/docs/migration/snowflake-transfer#required-roles)
3. [Prepare your staging bucket](https://docs.cloud.google.com/bigquery/docs/migration/snowflake-transfer#preparing-staging-bucket)
4. [Create a Snowflake user with the required permissions](https://docs.cloud.google.com/bigquery/docs/migration/snowflake-transfer#create-snowflake-user)
5. [Add network policies](https://docs.cloud.google.com/bigquery/docs/migration/snowflake-transfer#add_network_policies)
6. Optional: [Schema detection and mapping](https://docs.cloud.google.com/bigquery/docs/migration/snowflake-transfer#schema_detection_and_mapping)
7. [Assess your Snowflake for any unsupported data types](https://docs.cloud.google.com/bigquery/docs/migration/snowflake-transfer#limitations)
8. Optional: [Enable incremental transfers](https://docs.cloud.google.com/bigquery/docs/migration/snowflake-transfer#enable_incremental_transfers)
9. Optional: [Enable private connectivity](https://docs.cloud.google.com/bigquery/docs/migration/snowflake-transfer#enable_private_connectivity)
10. [Gather transfer information](https://docs.cloud.google.com/bigquery/docs/migration/snowflake-transfer#gather_transfer_information)
11. If you plan on specifying a customer-managed encryption key (CMEK), ensure that your [service account has permissions to encrypt and decrypt](https://docs.cloud.google.com/bigquery/docs/customer-managed-encryption#grant_permission), and that you have the [Cloud KMS key resource ID](https://docs.cloud.google.com/bigquery/docs/customer-managed-encryption#key_resource_id) required to use CMEK. For information about how CMEK works with the transfers, see [Specify encryption key with transfers](https://docs.cloud.google.com/bigquery/docs/migration/snowflake-transfer#CMEK).

### Prepare your Google Cloud project

Create and configure your Google Cloud project for a Snowflake
transfer with the following steps:

1. [Create a Google Cloud project](https://docs.cloud.google.com/resource-manager/docs/creating-managing-projects) or select an existing project.

   > [!NOTE]
   > **Note:** If you don't plan on keeping the resources created during this Snowflake transfer, create a new Google Cloud project instead of selecting an existing one. You can then delete the project once you are done with your Snowflake transfer.

2. Verify that you have completed all actions required to [enable the BigQuery Data Transfer Service](https://docs.cloud.google.com/bigquery/docs/enable-transfer-service).

3. [Create a BigQuery dataset](https://docs.cloud.google.com/bigquery/docs/datasets) to store
   your data. You don't need to create any tables.

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

> [!NOTE]
> **Note:** For ease of selection of your service account and the Cloud Storage bucket URI during transfer creation, we recommend that you grant the `iam.serviceAccounts.list` and `storage.buckets.list` permissions on the user creating the transfer configuration.

### Prepare staging bucket

To complete a Snowflake data transfer, you must create a staging
bucket and then configure it to allow write access from Snowflake.

Select one of the following options:

### AWS

**Staging bucket for AWS-hosted Snowflake account**

For AWS-hosted Snowflake account, create an
Amazon S3 bucket to stage the Snowflake data before it is
loaded into BigQuery.

1. [Create an Amazon S3 bucket](https://docs.aws.amazon.com/AmazonS3/latest/userguide/create-bucket-overview.html).

2. [Create and configure a Snowflake storage integration object](https://docs.snowflake.com/en/user-guide/data-load-s3-config-storage-integration)
   to allow Snowflake to write data into the Amazon S3
   bucket as an external stage.

To allow read access on your Amazon S3 bucket,
you must also do the following:

1. Create a dedicated [Amazon IAM user](https://docs.aws.amazon.com/IAM/latest/UserGuide/id_users.html)
   and grant it the [AmazonS3ReadOnlyAccess](https://docs.aws.amazon.com/aws-managed-policy/latest/reference/AmazonS3ReadOnlyAccess.html)
   policy.

2. [Create an Amazon access key pair](https://docs.aws.amazon.com/keyspaces/latest/devguide/create.keypair.html)
   for the IAM user.

### Azure

**Staging Azure Blob Storage container for Azure-hosted Snowflake account**

For Azure-hosted Snowflake accounts, create a
Azure Blob Storage container to stage the Snowflake data before it
is loaded into BigQuery.

1. [Create an Azure storage account](https://learn.microsoft.com/en-us/azure/storage/common/storage-account-create) and a [storage container](https://learn.microsoft.com/en-us/azure/storage/blobs/blob-containers-portal#create-a-container) within it.
2. [Create and configure a Snowflake storage integration object](https://docs.snowflake.com/en/user-guide/data-load-azure-config#option-1-configuring-a-snowflake-storage-integration) to allow Snowflake to write data into the Azure storage container as an external stage. Note that 'Step 3: Creating an external stage' can be skipped as we don't use it.

To allow read access on your Azure container,
[generate a SAS Token](https://learn.microsoft.com/en-us/azure/ai-services/translator/document-translation/how-to-guides/create-sas-tokens?tabs=Containers#create-sas-tokens-in-the-azure-portal) for it.

### Google Cloud

**Staging bucket for Google Cloud-hosted Snowflake account**

For Google Cloud-hosted Snowflake accounts, create a
Cloud Storage bucket to stage the Snowflake data before it
is loaded into BigQuery.

1. [Create a Cloud Storage bucket](https://docs.cloud.google.com/storage/docs/creating-buckets).
2. [Create and configure a Snowflake storage integration object](https://docs.snowflake.com/en/user-guide/data-load-gcs-config) to allow Snowflake to write data into the Cloud Storage bucket as an external stage.
3. To allow access to staging bucket, Grant [DTS service agent](https://docs.cloud.google.com/bigquery/docs/enable-transfer-service#service_agent) the `roles/storage.objectViewer` role with
   the following command:

   ```bash
   gcloud storage buckets add-iam-policy-binding gs://STAGING_BUCKET_NAME \
     --member=serviceAccount:service-PROJECT_NUMBER@gcp-sa-bigquerydatatransfer.iam.gserviceaccount.com \
     --role=roles/storage.objectViewer
   ```

### Create a Snowflake user with the required permissions

During a Snowflake transfer, the Snowflake
connector connects to your Snowflake account using a JDBC
connection. You must create a new Snowflake user
with a custom role that only has the necessary privileges to perform the
data transfer:

```
  // Create and configure new role, MIGRATION_ROLE
  GRANT USAGE
    ON WAREHOUSE WAREHOUSE_NAME
    TO ROLE MIGRATION_ROLE;

  GRANT USAGE
    ON DATABASE DATABASE_NAME
    TO ROLE MIGRATION_ROLE;

  GRANT USAGE
    ON SCHEMA DATABASE_NAME.SCHEMA_NAME
    TO ROLE MIGRATION_ROLE;

  // You can modify this to give select permissions for all tables in a schema
  GRANT SELECT
    ON TABLE DATABASE_NAME.SCHEMA_NAME.TABLE_NAME
    TO ROLE MIGRATION_ROLE;

  GRANT USAGE
    ON STORAGE_INTEGRATION_OBJECT_NAME
    TO ROLE MIGRATION_ROLE;
```

Replace the following:

- `MIGRATION_ROLE`: the name of the custom role you are creating
- `WAREHOUSE_NAME`: the name of your data warehouse
- `DATABASE_NAME`: the name of your Snowflake database
- `SCHEMA_NAME`: the name of your Snowflake schema
- `TABLE_NAME`: the name of the Snowflake included in this data transfer
- `STORAGE_INTEGRATION_OBJECT_NAME`: the name of your Snowflake storage integration object.

#### Generate key pair for authentication

Due to the [deprecation of single factor password sign-ins by Snowflake](https://docs.snowflake.com/en/user-guide/security-mfa-rollout), we recommend that you use key pair for authentication.

You can configure a key pair by generating an encrypted or unencrypted RSA key pair, then assigning the public key to a Snowflake user. For more information, see [Configuring key-pair authentication](https://docs.snowflake.com/en/user-guide/key-pair-auth#configuring-key-pair-authentication).

### Add network policies

For public connectivity, the Snowflake account allows public connection
with database credentials by default. However, you might have configured network rules or
policies that could prevent the Snowflake connector from
connecting to your account. In this case, you must add the necessary IP
addresses to your allowlist. For more information, see
[Configure network policies for Snowflake transfers](https://docs.cloud.google.com/bigquery/docs/migration/snowflake-network-policies).

### Schema detection and mapping

To define your schema, you can use the BigQuery Data Transfer Service to automatically
detect schema and data-type mapping when transferring data from
Snowflake to BigQuery. Alternatively, you can use
the translation engine to define your schema and data types manually.

For more information, see [Schema detection and mapping for Snowflake](https://docs.cloud.google.com/bigquery/docs/migration/snowflake-transfer-schema).

### Enable incremental transfers

To set up an incremental Snowflake data transfer, see
[Set up incremental transfers for Snowflake](https://docs.cloud.google.com/bigquery/docs/migration/snowflake-incremental).

### Enable private connectivity

If you want to create a private Snowflake data transfer, you must
[configure your network for private connectivity](https://docs.cloud.google.com/bigquery/docs/migration/snowflake-private-connectivity).

### Gather transfer information

Gather the information that you need to set up the migration with the BigQuery Data Transfer Service:

- Your Snowflake account identifier, which is the prefix in your Snowflake account URL. For example, `ACCOUNT_IDENTIFIER.snowflakecomputing.com`.
- The username and the associated private key with appropriate permissions to your Snowflake database. It can just have the [required permissions
  to execute the data transfer](https://docs.cloud.google.com/bigquery/docs/migration/snowflake-transfer#create-snowflake-user).
- The URI of the staging bucket that you want to use for the transfer:
  - For an AWS-hosted Snowflake account, an [Amazon S3 bucket URI](https://docs.cloud.google.com/bigquery/docs/migration/snowflake-transfer#preparing-s3-bucket) is required along with access credentials.
  - For an Azure-hosted Snowflake, an [Azure Blob Storage account and container](https://docs.cloud.google.com/bigquery/docs/migration/snowflake-transfer#preparing-azure-container) is required.
  - For a Google Cloud-hosted Snowflake account, a [Cloud Storage bucket URI](https://docs.cloud.google.com/bigquery/docs/migration/snowflake-transfer#preparing-gcs-bucket) is required. We recommend that you set up a lifecycle policy for this bucket to avoid unnecessary charges.
- The URI of the Cloud Storage bucket where you have stored the [schema mapping files obtained from the translation engine](https://docs.cloud.google.com/bigquery/docs/migration/snowflake-transfer-schema).

## Set up a Snowflake transfer

Select one of the following options:

### Console

1. Go to the Data transfers page in the Google Cloud console.

   [Go to Data transfers](https://console.cloud.google.com/bigquery/transfers)
2. Click **Create transfer**.

3. In the **Source type** section, select **Snowflake Migration**
   from the **Source** list.

4. In the **Transfer config name** section, enter a name for the transfer,
   such as `My migration`, in the **Display name** field. The display name
   can be any value that lets you identify the transfer if
   you need to modify it later.

5. In the **Destination settings** section, choose
   [the dataset you created](https://docs.cloud.google.com/bigquery/docs/migration/snowflake-transfer#preparing-gcp-project) from the **Dataset** list.

6. In the **Snowflake Credentials** section, do the following:

   1. For **Account Identifier** , enter a unique identifier for your Snowflake account, which is a combination of your organization name and account name. The identifier is the prefix of Snowflake account URL and not the complete URL. For example, `ACCOUNT_IDENTIFIER.snowflakecomputing.com`.
   2. For **Username** , enter the username of the Snowflake user whose credentials and authorization is used to access your database to transfer the Snowflake tables. We recommend using [the user that you created for this transfer](https://docs.cloud.google.com/bigquery/docs/migration/snowflake-transfer#create-snowflake-user).
   3. For **Authentication Mechanism** , select a Snowflake user authentication method:

      ### PASSWORD

      - For **Password**, enter the password of the Snowflake user.

      ### KEY_PAIR

      - For **Private Key** , enter the private key linked with the [public key associated with the Snowflake user](https://docs.cloud.google.com/bigquery/docs/migration/snowflake-transfer#create-snowflake-user).
      - For **Is Private Key Encrypted**, select this field if the private key is encrypted with a passphrase.
      - For **Private Key Passphrase** , enter the passphrase for the encrypted private key. This field is required if you have selected **Is Private Key Encrypted** . For more information, see [Generate key pair for authentication](https://docs.cloud.google.com/bigquery/docs/migration/snowflake-transfer#generate_key_pair_for_authentication)

   - For **Warehouse** , enter a [warehouse](https://docs.snowflake.com/en/user-guide/warehouses-tasks) that is used for the execution of this data transfer.
   - For **Snowflake Database**, enter the name of the Snowflake database that contains the tables included in this data transfer.
   - For **Snowflake Schema**, enter the name of the Snowflake schema that contains the tables included in this data transfer.
7. In the **Storage Configuration** section, do the following:

   1. For **Storage integration object name**, enter the name of the Snowflake storage integration object.
   2. Optional: For **Max file size**, specify the maximum size of each file unloaded from Snowflake to the staging location (in MB).
   3. For **Cloud Provider** , select `AWS` or `AZURE` or `GCP` depending on which
      cloud provider is hosting your Snowflake account.

      ### AWS

      - For **Amazon S3 URI** , enter the [URI of the Amazon S3 bucket](https://docs.cloud.google.com/bigquery/docs/migration/snowflake-transfer#preparing-s3-bucket) to use as a staging area.
      - For **Access key ID** and **Secret access key** , enter the [access key pair](https://docs.cloud.google.com/bigquery/docs/migration/snowflake-transfer#snowflake_key_pair).

      ### Azure

      - For **Azure storage account name** and **The container in the Azure storage account** , enter the [storage account and container name of the Azure Blob Storage](https://docs.cloud.google.com/bigquery/docs/migration/snowflake-transfer#preparing-azure-container) to use as a staging area.
      - For **SAS Token** , enter the [SAS token generated for the container](https://docs.cloud.google.com/bigquery/docs/migration/snowflake-transfer#azure_sas_token).

      ### Google Cloud

      - For **GCS URI** , enter the [URI of the Cloud Storage](https://docs.cloud.google.com/bigquery/docs/migration/snowflake-transfer#preparing-gcs-bucket) to use as a staging area.
8. In the **Service Account** section, do the following:

   1. For **Service Account** , enter a service account to use with this data transfer. The service account should belong to the same Google Cloud project where the transfer configuration and destination dataset is created. The service account must have the `storage.objects.list` and `storage.objects.get` [required permissions](https://docs.cloud.google.com/bigquery/docs/migration/snowflake-transfer-schema#required_service_account_permissions).
9. In the **Schema Configuration** section, do the following:

   1. For **Ingestion type** , select **Full** or **Incremental** . For more information, see [Configure incremental transfers](https://docs.cloud.google.com/bigquery/docs/migration/snowflake-incremental#configure_incremental_transfers).
   2. For **Table name patterns** , specify a table to transfer by entering a name or a pattern that matches the table name in the schema. You can use regular expressions to specify the pattern, for example `table1_regex;table2_regex`. The pattern should follow Java regular expression syntax. For example,
      - `lineitem;ordertb` matches tables that are named `lineitem` and `ordertb`.
      - `.*` matches all tables.
   3. Optional: For **Use BigQuery Translation Engine Output**, select this field if you want to specify a custom translation output path.
   4. Optional: For **Translation output GCS path** , specify a path to the Cloud Storage folder that contains the [schema mapping files from the translation engine](https://docs.cloud.google.com/bigquery/docs/migration/snowflake-transfer-schema). You can leave this empty to have the Snowflake connector automatically detect your schema.
      - The path should follow the format `translation_target_base_uri/metadata/config/db/schema/` and must end with `/`.
   5. Optional: For **Custom schema file path**, specify the Cloud Storage path to a custom schema file.
   6. Optional: For **Map zero scale Snowflake NUMBER to BigQuery INT64** , select this field if you want Snowflake `NUMBER(p, 0)` types to be mapped to BigQuery `INT64`.
10. In the **Network Connectivity** section, do the following:

    1. For **Use Private Network** , if you are creating a [private data
       transfer](https://docs.cloud.google.com/bigquery/docs/migration/snowflake-private-connectivity), select **True**.
    2. For **PSC Service Attachment** , if you are creating a private connection, enter the service attachment URI. For more information, see [Create a private Snowflake transfer
       configuration](https://docs.cloud.google.com/bigquery/docs/migration/snowflake-private-connectivity#create-transfer-config).
    3. For **Private Network Service** , if you are creating a private data transfer, enter the service directory self-link. For more information, see [Create a private Snowflake transfer
       configuration](https://docs.cloud.google.com/bigquery/docs/migration/snowflake-private-connectivity#create-transfer-config).
11. Optional: In the **Notification options** section, do the following:

    1. Click the toggle to enable email notifications. When you enable this option, the transfer administrator receives an email notification when a transfer run fails.
    2. For **Select a Pub/Sub topic** , choose your [topic](https://docs.cloud.google.com/pubsub/docs/overview#types) name or click **Create a topic** . This option configures Pub/Sub run [notifications](https://docs.cloud.google.com/bigquery/docs/transfer-run-notifications) for your transfer.
12. If you use [CMEKs](https://docs.cloud.google.com/bigquery/docs/customer-managed-encryption), in the
    **Advanced options** section, select **Customer-managed key** . A list of
    your available CMEKs appears for you to choose from. For information
    about how CMEKs work with the BigQuery Data Transfer Service, see
    [Specify encryption key with transfers](https://docs.cloud.google.com/bigquery/docs/migration/snowflake-transfer#CMEK).

13. Click **Save**.

14. The Google Cloud console displays all the transfer setup details,
    including a **Resource name** for this transfer.

### bq

Enter the `bq mk` command and supply the transfer creation flag
`--transfer_config`. The following flags are also required:

- `--project_id`
- `--data_source`
- `--target_dataset`
- `--display_name`
- `--params`

```bash
bq mk \
    --transfer_config \
    --project_id=project_id \
    --data_source=data_source \
    --target_dataset=dataset \
    --display_name=name \
    --service_account_name=service_account \
    --params='parameters'
```

Replace the following:

- <var translate="no">project_id</var>: your Google Cloud project ID. If `--project_id` isn't specified, the default project is used.
- <var translate="no">data_source</var>: the data source, `snowflake_migration`.
- <var translate="no">dataset</var>: the BigQuery target dataset for the transfer configuration.
- <var translate="no">name</var>: the display name for the transfer configuration. The transfer name can be any value that lets you identify the transfer if you need to modify it later.
- <var translate="no">service_account</var>: (Optional) the service account name used to authenticate your transfer. The service account should be owned by the same `project_id` used to create the transfer and it should have all of the [required roles](https://docs.cloud.google.com/bigquery/docs/migration/snowflake-transfer#required-roles).
- <var translate="no">parameters</var>: the parameters for the created transfer configuration in JSON format. For example: `--params='{"param":"param_value"}'`.

You can configure the following parameters for your Snowflake
transfer configuration:

- `account_identifier`: specify a unique identifier for your Snowflake account, which is a combination of your organization name and account name. The identifier is the prefix of Snowflake account URL and not the complete URL. For example, `account_identifier.snowflakecomputing.com`.
- `username`: specify the username of the Snowflake user whose credentials and authorization is used to access your database to transfer the Snowflake tables.
- `auth_mechanism`: specify the Snowflake user authentication method. Supported values are `PASSWORD` and `KEY_PAIR`. For more information, see [Generate key pair for authentication](https://docs.cloud.google.com/bigquery/docs/migration/snowflake-transfer#generate_key_pair_for_authentication).
- `password`: specify the password of the Snowflake user. This field is required if you have specified `PASSWORD` in the `auth_mechanism` field.
- `private_key` : specify the private key linked with the [public key associated with the Snowflake user](https://docs.cloud.google.com/bigquery/docs/migration/snowflake-transfer#create-snowflake-user). This field is required if you have specified `KEY_PAIR` in the `auth_mechanism` field.
- `is_private_key_encrypted` : specify `true` if the private key is encrypted with a passphrase.
- `private_key_passphrase` : specify the passphrase for the encrypted private key. This field is required if you have specified `KEY_PAIR` in the `auth_mechanism` field and specified `true` in the `is_private_key_encrypted` field.
- `warehouse`: specify a [warehouse](https://docs.snowflake.com/en/user-guide/warehouses-tasks) that is used for the execution of this data transfer.
- `service_account`: specify a service account to use with this data transfer. The service account should belong to the same Google Cloud project where the transfer configuration and destination dataset is created. The service account must have the `storage.objects.list` and `storage.objects.get` [required permissions](https://docs.cloud.google.com/bigquery/docs/migration/snowflake-transfer-schema#required_service_account_permissions).
- `database`: specify the name of the Snowflake database that contains the tables included in this data transfer.
- `schema`: specify the name of the Snowflake schema that contains the tables included in this data transfer.
- `table_name_patterns`: specify a table to transfer by entering
  a name or a pattern that matches the table name in the schema. You
  can use regular expressions to specify the pattern, for example
  `table1_regex;table2_regex`. The
  pattern should follow Java regular expression syntax. For example,

  - `lineitem;ordertb` matches tables that are named `lineitem` and `ordertb`.
  - `.*` matches all tables.

    You can also leave this field blank to migrate all tables from the
    specified schema.
- `ingestion_mode`: specify the ingestion mode for the transfer. Supported
  values are `FULL` and `INCREMENTAL`. For
  more information, see [Configure incremental transfers](https://docs.cloud.google.com/bigquery/docs/migration/snowflake-incremental#configure_incremental_transfers).

- `translation_output_gcs_path`: (Optional) specify a path to the
  Cloud Storage folder that contains the [schema mapping files from the translation engine](https://docs.cloud.google.com/bigquery/docs/migration/snowflake-transfer-schema). You can leave this empty to have the Snowflake connector automatically detect your schema.

  - The path should follow the format `gs://translation_target_base_uri/metadata/config/db/schema/` and must end with `/`.
- `storage_integration_object_name`: specify the name of the Snowflake
  storage integration object.

- `cloud_provider`: enter `AWS` or `AZURE` or `GCP` depending on which
  cloud provider is hosting your Snowflake account.

- `staging_s3_uri`: enter the [URI of the S3 bucket](https://docs.cloud.google.com/bigquery/docs/migration/snowflake-transfer#preparing-s3-bucket)
  to use as a staging area. Only required when your `cloud_provider` is `AWS`.

- `aws_access_key_id`: enter the [access key pair](https://docs.cloud.google.com/bigquery/docs/migration/snowflake-transfer#snowflake_key_pair). Only required when your `cloud_provider` is `AWS`.

- `aws_secret_access_key`: enter the [access key pair](https://docs.cloud.google.com/bigquery/docs/migration/snowflake-transfer#snowflake_key_pair). Only required when your `cloud_provider` is `AWS`.

- `azure_storage_account`: enter the [storage account name](https://docs.cloud.google.com/bigquery/docs/migration/snowflake-transfer#preparing-azure-container)
  to use as a staging area. Only required when your `cloud_provider` is `AZURE`.

- `staging_azure_container`: enter the [container within Azure Blob Storage](https://docs.cloud.google.com/bigquery/docs/migration/snowflake-transfer#preparing-azure-container)
  to use as a staging area. Only required when your `cloud_provider` is `AZURE`.

- `azure_sas_token`: enter the [SAS token](https://docs.cloud.google.com/bigquery/docs/migration/snowflake-transfer#azure_sas_token). Only required when your `cloud_provider` is `AZURE`.

- `staging_gcs_uri` : enter the [URI of the Cloud Storage](https://docs.cloud.google.com/bigquery/docs/migration/snowflake-transfer#preparing-gcs-bucket)
  to use as a staging area. Only required when your `cloud_provider` is `GCP`.

- `use_private_network`: if you are creating a [private data
  transfer](https://docs.cloud.google.com/bigquery/docs/migration/snowflake-private-connectivity), set to
  `TRUE`.

- `service_attachment`: if you are creating a private data transfer, specify
  the service attachment URI. For more information, see [Create a private
  Snowflake transfer
  configuration](https://docs.cloud.google.com/bigquery/docs/migration/snowflake-private-connectivity#create-transfer-config).

- `private_network_service`: if you are creating a private data transfer,
  specify the self-link of the NLB service. For more information, see
  [Create a private Snowflake transfer
  configuration](https://docs.cloud.google.com/bigquery/docs/migration/snowflake-private-connectivity#create-transfer-config).

For example, for an AWS-hosted Snowflake
account, the following command creates a Snowflake transfer
named `Snowflake transfer config` with a target dataset named `your_bq_dataset`
and a project with the ID of `your_project_id`.

```bash
  PARAMS='{
  "account_identifier": "your_account_identifier",
  "auth_mechanism": "KEY_PAIR",
  "aws_access_key_id": "your_access_key_id",
  "aws_secret_access_key": "your_aws_secret_access_key",
  "cloud_provider": "AWS",
  "database": "your_sf_database",
  "ingestion_mode": "INCREMENTAL",
  "private_key": "---BEGIN PRIVATE KEY--- privatekey\nseparatedwith\nnewlinecharacters=---END PRIVATE KEY---",
  "schema": "your_snowflake_schema",
  "service_account": "your_service_account",
  "storage_integration_object_name": "your_storage_integration_object",
  "staging_s3_uri": "s3://your/s3/bucket/uri",
  "table_name_patterns": ".*",
  "translation_output_gcs_path": "gs://sf_test_translation/output/metadata/config/database_name/schema_name/",
  "username": "your_sf_username",
  "warehouse": "your_warehouse"
}'

bq mk --transfer_config \
    --project_id=your_project_id \
    --target_dataset=your_bq_dataset \
    --display_name='snowflake transfer config' \
    --params="$PARAMS" \
    --data_source=snowflake_migration
```

> [!NOTE]
> **Note:** You can't configure notifications using the command-line tool.

### API

Use the [`projects.locations.transferConfigs.create`](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs/create)
method and supply an instance of the [`TransferConfig`](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs#TransferConfig)
resource.

> [!NOTE]
> If multiple transfers are created for the same Snowflake tables or if the same transfer configuration is run multiple times, the data in the existing BigQuery destination tables is overwritten.

## Specify encryption key with transfers

You can specify [customer-managed encryption keys (CMEKs)](https://docs.cloud.google.com/kms/docs/cmek) to encrypt data for a transfer run. You can use a CMEK to support transfers from [Snowflake](https://docs.cloud.google.com/bigquery/docs/migration/snowflake-migration-intro).

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

> [!NOTE]
> **Note:** For Snowflake transfers, CMEK encryption handles encryption of the data in the BigQuery destination tables as well as encryption of data in the intermediate Cloud Storage tenant bucket used during the transfer process for Snowflake on Amazon S3 or Azure Blob Storage.

## Quotas and limits

BigQuery has a load quota of 15 TB for each load job for each
table by default. Internally, Snowflake compresses the table data, so the
exported table size is larger than the table size reported by
Snowflake.

To improve load times for larger tables, specify the [`PIPELINE` job type](https://docs.cloud.google.com/bigquery/docs/migration/snowflake-transfer#quotas_and_limits)
for your reservation assignment.

Because of
[Amazon S3's consistency model](https://docs.cloud.google.com/bigquery/docs/s3-transfer-intro#consistency_considerations),
it's possible that some files won't be included in the transfer to
BigQuery.

## Optimize data transfer performance

You can monitor the performance of your data transfers by viewing the [logs for the data transfer](https://docs.cloud.google.com/bigquery/docs/dts-monitor). To improve the performance of your data transfers, we recommend that you perform the following optimization steps:

- Keep your Snowflake instance, staging bucket, and BigQuery dataset in the same region
- You can improve table unload speeds by doing the following:
  - Increase the size of your Snowflake virtual warehouse, especially when transferring large Snowflake tables (1 TiB or higher).
  - Adjust the `MAX_FILE_SIZE` option in the transfer configuration.
    - Smaller file sizes can improve transfer speeds, but making it too small can lead to too many files.
- You can improve table load speeds by increasing the number of BigQuery slot reservations for `PIPELINE` and `QUERY` job types.
- When making a full transfer, avoid clustering and partitioning on the destination BigQuery table.
- When making an incremental transfer in Upsert mode, consider clustering and partitioning on primary key columns to improve transfer performance.
  - However, avoid clustering and partitioning on non-primary key columns to prevent slower merge operations.

## Pricing

For information on BigQuery Data Transfer Service pricing, see the
[Pricing](https://cloud.google.com/bigquery/pricing#data-transfer-service-pricing)
page.

- If the Snowflake warehouse and the Amazon S3 bucket are in different regions, then Snowflake applies egress charges when you run a Snowflake data transfer. There are no egress charges for Snowflake data transfers if both the Snowflake warehouse and the Amazon S3 bucket are in the same region.
- When data is transferred from AWS to Google Cloud, inter-cloud egress charges are applied.

## What's next

- Learn more about the [BigQuery Data Transfer Service](https://docs.cloud.google.com/bigquery/docs/transfer-service-overview).
- Migrate SQL code with the [Batch SQL translation](https://docs.cloud.google.com/bigquery/docs/batch-sql-translator).