# Migrate metadata from external data catalogs to Lakehouse REST catalog tables for Apache Iceberg

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
> **Note:** To get support or provide feedback for this feature, contact [da-migrations-feedback@google.com](mailto:da-migrations-feedback@google.com).

This document shows you how to migrate metadata from external data catalogs to
[Lakehouse REST catalog tables for Apache Iceberg](https://docs.cloud.google.com/biglake/docs/biglake-iceberg-tables). The
BigQuery Migration Service supports the migration of metadata from the following
external metastores:

- Apache Hive Metastore
- Apache Iceberg REST Catalogs

## Limitations

> [!WARNING]
> **Warning:** Modifying data on both the source and destination tables after a migration can lead to data loss.

- Metadata migrations from external data catalogs to Lakehouse REST catalog tables for Apache Iceberg are a one-time sync. This feature doesn't support continuous or periodic syncs, so any writes made after migration aren't visible until you migrate again.
- Nested namespaces aren't supported.
- The Iceberg REST catalog only supports Parquet data files.
- Lakehouse doesn't support Apache Iceberg V3 tables.
- Metadata migrations from external data catalogs to Lakehouse REST catalog tables for Apache Iceberg only supports migrations of up to 10,000 tables. If your workload requires processing more than 10,000 tables, then we recommend splitting your workload across multiple migrations.
- Metadata migrations from external data catalogs to Lakehouse REST catalog tables for Apache Iceberg don't support the use of organization policies to enforce [domain-restricted
  sharing](https://docs.cloud.google.com/organization-policy/domain-restricted-sharing).

## Before you begin

Before you can migrate metadata from your external data catalogs, you must do
the steps in the following sections.

### Enable APIs


Enable the BigLake, BigQuery Data Transfer, BigQuery Migration, Secret Manager, Storage Transfer APIs.


**Roles required to enable APIs**


To enable APIs, you need the Service Usage Admin IAM
role (`roles/serviceusage.serviceUsageAdmin`), which
contains the `serviceusage.services.enable` permission. [Learn how to grant
roles](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).

[Enable the APIs](https://console.cloud.google.com/flows/enableapi?apiid=biglake.googleapis.com,bigquerydatatransfer.googleapis.com,bigquerymigration.googleapis.com,secretmanager.googleapis.com,storagetransfer.googleapis.com)

A [service agent](https://docs.cloud.google.com/bigquery/docs/enable-transfer-service#service_agent) is
created when you enable the Data Transfer API.

### Configure permissions

1. The user or the service account creating the transfer should be granted the BigQuery Admin role (`roles/bigquery.admin`). If you use a service account, it's only used to create the transfer.
2. A [service agent](https://docs.cloud.google.com/bigquery/docs/enable-transfer-service#service_agent)
   (P4SA) is created upon enabling the Data Transfer API.


   To ensure that the service agent has the necessary
   permissions to run a Hive Metastore transfer,

   ask your administrator to grant the
   following IAM roles to the service agent on the project:

   **Important:** You must grant these roles to the service agent, *not* to your user account. Failure to grant the roles to the correct principal might result in permission errors.
   - [Storage Transfer Admin](https://docs.cloud.google.com/iam/docs/roles-permissions/storagetransfer#storagetransfer.admin) (`roles/storagetransfer.admin`)
   - [Service Usage Consumer](https://docs.cloud.google.com/iam/docs/roles-permissions/serviceusage#serviceusage.serviceUsageConsumer) (`roles/serviceusage.serviceUsageConsumer`)
   - [Storage Admin](https://docs.cloud.google.com/iam/docs/roles-permissions/storage#storage.admin) (`roles/storage.admin`)
   - To migrate metadata to Lakehouse runtime catalog Iceberg REST Catalog : [BigLake Admin](https://docs.cloud.google.com/iam/docs/roles-permissions/biglake#biglake.admin) (`roles/biglake.admin`)
   - To migrate metadata to Dataproc Metastore: [Dataproc Metastore Data Owner](https://docs.cloud.google.com/iam/docs/roles-permissions/metastore#metastore.metadataOwner) (`roles/metastore.metadataOwner`)


   For more information about granting roles, see [Manage access to projects, folders, and organizations](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).


   Your administrator might also be able to give the service agent
   the required permissions through [custom
   roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles) or other [predefined
   roles](https://docs.cloud.google.com/iam/docs/roles-overview#predefined).

   <br />

3. If you are using a service account, grant the service agent the
   `roles/iam.serviceAccountTokenCreator` role with
   the following command:

   ```bash
   gcloud iam service-accounts add-iam-policy-binding
   SERVICE_ACCOUNT --member
   serviceAccount:service-PROJECT_NUMBER@gcp-sa-bigquerydatatransfer.iam.gserviceaccount.com --role
   roles/iam.serviceAccountTokenCreator
   ```
4. Grant the Storage Transfer Service service agent
   (`project-PROJECT_NUMBER@storage-transfer-service.iam.gserviceaccount.com`) the following roles in the
   project:

   - `roles/storage.admin`
   - If you are migrating from on-prem/HDFS, you must also grant the `roles/storagetransfer.serviceAgent` role.

   You can also configure more granular permissions. For more information, see
   the following guide:
   - [HDFS
     permissions](https://docs.cloud.google.com/storage-transfer/docs/file-system-permissions)
   - [Amazon S3 and Microsoft Azure
     permissions](https://docs.cloud.google.com/storage-transfer/docs/iam-cloud)

### Required user roles and permissions


To ensure that the service agent has the necessary
permissions to create, modify, and run a Lakehouse REST catalog tables for Apache Iceberg migration,

ask your administrator to grant the
following IAM roles to the service agent on the user:

**Important:** You must grant these roles to the service agent, *not* to your user account. Failure to grant the roles to the correct principal might result in permission errors.

- [Service Account User](https://docs.cloud.google.com/iam/docs/roles-permissions/iam#iam.serviceAccountUser) (`roles/iam.serviceAccountUser`)
- [Migration Management Editor](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquerymigration#bigquerymigration.migrationEditor) (`roles/bigquerymigration.migrationEditor`)


For more information about granting roles, see [Manage access to projects, folders, and organizations](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).


Your administrator might also be able to give the service agent
the required permissions through [custom
roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles) or other [predefined
roles](https://docs.cloud.google.com/iam/docs/roles-overview#predefined).

### Required service account roles and permissions


To ensure that the customer-provided service account has the necessary
permissions to create, modify, and run a Lakehouse REST catalog tables for Apache Iceberg migration,

ask your administrator to grant the
following IAM roles to the customer-provided service account:

**Important:** You must grant these roles to the customer-provided service account, *not* to your user account. Failure to grant the roles to the correct principal might result in permission errors.

- Access to the network attachment: [Compute Network Admin](https://docs.cloud.google.com/iam/docs/roles-permissions/compute#compute.networkAdmin) (`roles/compute.networkAdmin`) on the service account
- Access to the BigQuery Data Transfer Service: [BigQuery Admin](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquery#bigquery.admin) (`roles/bigquery.admin`) on the service account
- Access to Lakehouse resources: [BigLake Editor](https://docs.cloud.google.com/iam/docs/roles-permissions/biglake#biglake.editor) (`roles/biglake.editor`) on the service account
- Access to secrets: [Secret Manager Secret Accessor](https://docs.cloud.google.com/iam/docs/roles-permissions/secretmanager#secretmanager.secretAccessor) (`roles/secretmanager.secretAccessor`) on the customer-provided secret


For more information about granting roles, see [Manage access to projects, folders, and organizations](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).


Your administrator might also be able to give the customer-provided service account
the required permissions through [custom
roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles) or other [predefined
roles](https://docs.cloud.google.com/iam/docs/roles-overview#predefined).

You must also grant the service agent the `roles/iam.serviceAccountTokenCreator`
role with the following command:

```
gcloud iam service-accounts add-iam-policy-binding SERVICE_ACCOUNT --member user:bigquerymigration-management-borg@prod.google.com --role roles/iam.serviceAccountTokenCreator --project PROJECT_ID
```

```
gcloud iam service-accounts add-iam-policy-binding SERVICE_ACCOUNT --member bigquerymigration-managementworker-borg@prod.google.com --role roles/iam.serviceAccountTokenCreator --project PROJECT_ID
```

Replace the following:

- `SERVICE_ACCOUNT`: the ID of the service account
- `PROJECT_ID`: the project iD

### Configure secret

If you are migrating to a Apache Iceberg REST catalog, you must create
a secret to authorize the migration. The secret must be formatted in the
following format:

```
{
  "client_id": "CLIENT_ID",
  "client_secret": "CLIENT_SECRET<",
  "polaris_realm": "POLARIS_REALM"
  "scope": "SCOPE"
}
```

Replace the following:

- `CLIENT_ID`: the OAuth2.0 client ID
- `CLIENT_SECRET`: the OAuth2.0 client secret
- `POLARIS_REALM`: the realm for the Polaris catalog. This field is required only for Apache Polaris. For example, `FINANCE`.
- `SCOPE`: (Optional) the OAuth2.0 scope. Default value is `PRINCIPAL_ROLE:ALL`.

## Create a Lakehouse catalog

[Create a Lakehouse
catalog](https://docs.cloud.google.com/biglake/docs/use-biglake-metastore-iceberg-rest-catalog#create-biglake-catalog).
The migrated metadata is stored in the Cloud Storage bucket that you
specified when you create the catalog.

## Migrate metadata

To start a metadata migration to Lakehouse Iceberg REST catalog tables, do
the following:

1. In the Google Cloud console, go to the **Migration** \> **Services** page.

   [Go to Migration services](https://console.cloud.google.com/bigquery/migrations/overview)
2. Under **Register or Migrate Open Lakehouse** , click **Create migration**.

3. Under **Migration configuration**, do the following:

   1. For catalog type, select an external catalog.
   2. For **Region**, select a region. The selected region determines where the
      migration orchestration and the data transfer is run. It also determines
      where the migration service uses or creates any resources, such as network
      attachments or secrets.

      Only [regional
      secrets](https://docs.cloud.google.com/secret-manager/regional-secrets/create-regional-secret) are
      supported. The region of the secret must match the region in this field.
   3. For **Migration display name**, enter a name for this migration.

4. Under **Source system configuration**, do the following:

   1. For **URL**, enter the base endpoint URL that acts as the entry point for the Apache Iceberg or Apache Hive metastore.
   2. For **Service account**, select a service account from the list. If not specified, this migration runs using the user credential.
   3. (Optional) For **Network attachment**, select a network attachment.
5. Click **Continue**.

When the metadata migration completes, the metadata from your external catalogs
is stored in the Cloud Storage bucket that you specified when you [created
the Lakehouse catalog](https://docs.cloud.google.com/biglake/docs/use-biglake-metastore-iceberg-rest-catalog#create-biglake-catalog).

## Pricing

There is no cost to transfer metadata to
Lakehouse Iceberg REST catalog tables. Once metadata is transferred,
[Lakehouse pricing](https://cloud.google.com/products/biglake/pricing)
applies.