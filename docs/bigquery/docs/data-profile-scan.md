# Profile your data

This document explains how to use data profile scans to better understand your data.
BigQuery uses Knowledge Catalog to analyze the statistical
characteristics of your data, such as average values, unique values, and maximum
values. Knowledge Catalog also uses this information to
[recommend rules for data quality checks](https://docs.cloud.google.com/dataplex/docs/auto-data-quality-overview).

For more information about data profiling, see
[About data profiling](https://docs.cloud.google.com/dataplex/docs/data-profiling-overview).

> [!TIP]
> **Tip:** The steps in this document show how to manage data profile scans across your project. You can also create and manage data profile scans when working with a specific table. For more information, see the [Manage data profile scans for a specific table](https://docs.cloud.google.com/bigquery/docs/data-profile-scan#start-from-table) section of this document.

## Before you begin


Enable the Dataplex API.


**Roles required to enable APIs**


To enable APIs, you need the Service Usage Admin IAM
role (`roles/serviceusage.serviceUsageAdmin`), which
contains the `serviceusage.services.enable` permission. [Learn how to grant
roles](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).

[Enable the API](https://console.cloud.google.com/flows/enableapi?apiid=dataplex.googleapis.com)

## Required roles


This section describes the IAM roles and permissions needed to
use Knowledge Catalog data profile scans.

### User roles and permissions


To get the permissions that
you need to create and manage data profile scans,

ask your administrator to grant you the
following IAM roles:

- Create, run, update, and delete data profile scans: [Dataplex DataScan Editor](https://docs.cloud.google.com/iam/docs/roles-permissions/dataplex#dataplex.dataScanEditor) (`roles/dataplex.dataScanEditor`) on the project containing the data scan
- View data profile scan results, jobs, and history: [Dataplex DataScan Viewer](https://docs.cloud.google.com/iam/docs/roles-permissions/dataplex#dataplex.dataScanViewer) (`roles/dataplex.dataScanViewer`) on the project containing the data scan
- Publish data profile scan results to Knowledge Catalog: [Dataplex Catalog Editor](https://docs.cloud.google.com/iam/docs/roles-permissions/dataplex#dataplex.catalogEditor) (`roles/dataplex.catalogEditor`) on the `@bigquery` entry group
- View published data profile scan results in BigQuery on the **Data profile** tab: [BigQuery Data Viewer](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquery#bigquery.dataViewer) (`roles/bigquery.dataViewer`) on the table


For more information about granting roles, see [Manage access to projects, folders, and organizations](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).


These predefined roles contain

the permissions required to create and manage data profile scans. To see the exact permissions that are
required, expand the **Required permissions** section:


#### Required permissions

The following permissions are required to create and manage data profile scans:

- Create, run, update, and delete data profile scans:
  - `dataplex.datascans.create` on project
  - `dataplex.datascans.update` on data scan
  - `dataplex.datascans.delete` on data scan
  - `dataplex.datascans.run` on data scan
  - `dataplex.datascans.get` on data scan
  - `dataplex.datascans.list` on project
  - `dataplex.dataScanJobs.get` on data scan job
  - `dataplex.dataScanJobs.list` on data scan
- View data profile scan results, jobs, and history:
  - `dataplex.datascans.getData` on data scan
  - `dataplex.datascans.list` on project
  - `dataplex.dataScanJobs.get` on data scan job
  - `dataplex.dataScanJobs.list` on data scan
- Publish data profile scan results to Knowledge Catalog:
  - `dataplex.entryGroups.useDataProfileAspect` on entry group
  - `bigquery.tables.update` on table
  - `dataplex.entries.update` on entry
- View published data profile results for a table in BigQuery or Knowledge Catalog:
  - `bigquery.tables.get` on table
  - `bigquery.tables.getData` on table


You might also be able to get
these permissions
with [custom roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles) or
other [predefined roles](https://docs.cloud.google.com/iam/docs/roles-overview#predefined).

### Knowledge Catalog service account roles and permissions


To ensure that the Knowledge Catalog service account has the necessary
permissions to run data profile scans and export results,

ask your administrator to grant the
following IAM roles to the Knowledge Catalog service account:

**Important:** You must grant these roles to the Knowledge Catalog service account, *not* to your user account. Failure to grant the roles to the correct principal might result in permission errors.

- Run data profile scans against BigQuery data:
  - [BigQuery Job User](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquery#bigquery.jobUser) (`roles/bigquery.jobUser`) on project running the scan
  - [BigQuery Data Viewer](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquery#bigquery.dataViewer) (`roles/bigquery.dataViewer`) on tables being scanned
- Run data profile scans for BigQuery external tables that use Cloud Storage data:
  - [Storage Object Viewer](https://docs.cloud.google.com/iam/docs/roles-permissions/storage#storage.objectViewer) (`roles/storage.objectViewer`) on Cloud Storage bucket
  - [Storage Legacy Bucket Reader](https://docs.cloud.google.com/iam/docs/roles-permissions/storage#storage.legacyBucketReader) (`roles/storage.legacyBucketReader`) on Cloud Storage bucket
- Run data profile scans for Iceberg REST Catalog tables on Google Cloud Lakehouse: [BigLake Viewer](https://docs.cloud.google.com/iam/docs/roles-permissions/biglake#biglake.viewer) (`roles/biglake.viewer`) on Iceberg Rest Catalog tables being scanned
- Export data profile scan results to a BigQuery table: [BigQuery Data Editor](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquery#bigquery.dataEditor) (`roles/bigquery.dataEditor`) on table


For more information about granting roles, see [Manage access to projects, folders, and organizations](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).


These predefined roles contain

the permissions required to run data profile scans and export results. To see the exact permissions that are
required, expand the **Required permissions** section:


#### Required permissions

The following permissions are required to run data profile scans and export results:

- Run data profile scans against BigQuery data:
  - `bigquery.jobs.create` on project
  - `bigquery.tables.get` on table
  - `bigquery.tables.getData` on table
- Run data profile scans for BigQuery external tables that use Cloud Storage data:
  - `storage.buckets.get` on bucket
  - `storage.objects.get` on object
- Export data profile scan results to a BigQuery table:
  - `bigquery.tables.create` on dataset
  - `bigquery.tables.updateData` on table


Your administrator might also be able to give the Knowledge Catalog service account
these permissions
with [custom roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles) or
other [predefined roles](https://docs.cloud.google.com/iam/docs/roles-overview#predefined).

If a table uses BigQuery [row-level
security](https://docs.cloud.google.com/bigquery/docs/row-level-security-intro), then Knowledge Catalog
can only scan rows visible to the Knowledge Catalog service account. To
allow Knowledge Catalog to scan all rows, add its service account to a row
filter where the predicate is `TRUE`.

If a table uses BigQuery [column-level security](https://docs.cloud.google.com/bigquery/docs/column-level-security), then Knowledge Catalog
requires access to scan protected columns. To grant access, give the
Knowledge Catalog service account the
**Data Catalog Fine-Grained Reader** (`roles/datacatalog.fineGrainedReader`)
role on all policy tags used in the table. The user creating or updating a data
scan also needs permissions on protected columns.

### Grant roles to the Knowledge Catalog service account

To run data profile scans, Knowledge Catalog uses a service account that
requires permissions to run BigQuery jobs and read
BigQuery table data. To grant the required roles, follow
these steps:

1. Get the Knowledge Catalog service account email address. If you haven't
   created a data profile or data quality scan in this project before,
   run the following `gcloud` command to generate the service identity:

       gcloud beta services identity create --service=dataplex.googleapis.com

   The command returns the service account email, which has the following format:
   service-<var translate="no">PROJECT_ID</var>@gcp-sa-dataplex.iam.gserviceaccount.com.

   If the service account already exists, you can find its email by viewing
   principals with the **Dataplex** name on the [**IAM** page](https://console.cloud.google.com/iam-admin/iam) in the Google Cloud console.
2. Grant the service account the **BigQuery Job User**
   (`roles/bigquery.jobUser`) role on your project. This role lets the
   service account run BigQuery jobs for the scan.

       gcloud projects add-iam-policy-binding PROJECT_ID \
           --member="serviceAccount:service-PROJECT_NUMBER@gcp-sa-dataplex.iam.gserviceaccount.com" \
           --role="roles/bigquery.jobUser"

   Replace the following:
   - `PROJECT_ID`: your Google Cloud project ID.
   - `service-PROJECT_NUMBER@gcp-sa-dataplex.iam.gserviceaccount.com`: the email of the Knowledge Catalog service account.
3. Grant the service account the **BigQuery Data Viewer**
   (`roles/bigquery.dataViewer`) role for each table that you want to
   profile. This role grants read-only access to the tables.

       gcloud bigquery tables add-iam-policy-binding DATASET_ID.TABLE_ID \
           --member="serviceAccount:service-PROJECT_NUMBER@gcp-sa-dataplex.iam.gserviceaccount.com" \
           --role="roles/bigquery.dataViewer"

   Replace the following:
   - `DATASET_ID`: the ID of the dataset containing the table.
   - `TABLE_ID`: the ID of the table to profile.
   - `service-PROJECT_NUMBER@gcp-sa-dataplex.iam.gserviceaccount.com`: the email of the Knowledge Catalog service account.

<br />

## Create a data profile scan


### Console

1.


   In the Google Cloud console, on the BigQuery
   **Metadata curation** page, go to the **Data profiling \& quality** tab.


   [Go to Data profiling \& quality](https://console.cloud.google.com/bigquery/governance/metadata-curation/data-profiling-and-quality)

   <br />

2. Click **Create data profile scan**.

3. Optional: Enter a **Display name**.

4. Enter an **ID** . See the
   [Resource naming conventions](https://docs.cloud.google.com/compute/docs/naming-resources#resource-name-format).

5. Optional: Enter a **Description**.

6. In the **Table** field, click **Browse** . Choose the table to scan, and
   then click **Select**. Only standard BigQuery and
   Iceberg REST Catalog tables are supported.

   For tables in multi-region datasets, choose a region where to create
   the data scan.

   To browse the tables organized within Knowledge Catalog lakes,
   click **Browse within Knowledge Catalog Lakes**.
7. In the **Mode** section, select one of the following options:

   - **Standard**: profiles your data with customizable scan settings.
     This is the default mode.

   - **Lightweight**: provides quick insights with a low-latency,
     low-fidelity scan.

8. If you chose the **Standard** mode, configure the following options. These
   options don't appear when you select **Lightweight** mode.

   1. In the **Scope** field, choose **Incremental** or **Entire data**.

      If you choose **Incremental data** , in the **Timestamp column** field,
      select a column of type `DATE` or `TIMESTAMP` from your
      BigQuery table. Knowledge Catalog uses this
      column to identify new records as they are added. For tables partitioned on a
      column of type `DATE` or `TIMESTAMP`, we recommend using this column as the partition
      column.
   2. Optional: To filter your data, do any of the following:

      - To filter by rows, select the **Filter rows** checkbox.
        Enter a valid SQL expression that can be used in a
        [`WHERE` clause in GoogleSQL syntax](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#where_clause).
        For example: `col1 >= 0`.

        The filter can be a combination of SQL conditions over multiple
        columns. For example: `col1 >= 0 AND col2 < 10`.
      - To filter by columns, select the **Filter columns** checkbox.

      - To include columns in the profile scan, in the **Include columns**
        field, click **Browse** . Select the columns to include, and then
        click **Select**.

      - To exclude columns from the profile scan, in the **Exclude columns**
        field, click **Browse** . Select the columns to exclude, and then
        click **Select**.

      > [!NOTE]
      > **Note:** You can use **Include columns** , **Exclude columns** , or both. If you use both the fields, then the data profile scan first selects the columns based on your input in the **Include columns** field and then excludes the columns based on your input in the **Exclude columns** field.

   3. To apply sampling to your data profile scan, in the **Sampling size**
      list, select a sampling percentage. Choose a percentage value that ranges
      between 0.0% and 100.0% with up to 3 decimal digits.

      - For larger datasets, choose a lower sampling percentage. For example,
        for a 1 PB table, if you enter a value between 0.1% and 1.0%,
        the data profile samples between 1-10 TB of data.

      - There must be at least 100 records in the sampled data to return a result.

      - For incremental data scans, the data profile scan applies sampling to
        the latest increment.

9. Optional: Publish the data profile scan results in the
   BigQuery and Knowledge Catalog pages in the
   Google Cloud console for the source table. Select the
   **Publish results to Knowledge Catalog**
   checkbox.

   You can view the latest scan results in the **Data profile** tab in the
   BigQuery and Knowledge Catalog pages for the source
   table. To enable users to access the published scan results, see the

   [Grant access to data profile scan results](https://docs.cloud.google.com/bigquery/docs/data-profile-scan#share-results) section
   of this document.

   The publishing option might not be available in the following cases:
   - You don't have the required permissions on the table.
   - Another data profile scan is set to publish results.
10. In the **Schedule** section, choose one of the following options:

    - **Repeat** : Run the data profile scan on a schedule: hourly, daily,
      weekly, monthly, or custom. Specify how often the scan should run and
      at what time. If you choose custom, use
      [cron](https://en.wikipedia.org/wiki/Cron) format to specify the
      schedule.

    - **On-demand**: Run the data profile scan on demand.

    - **One-time run** : Run the data profile scan once now, and remove the scan
      after the auto-deletion time. This feature is in [Preview](https://cloud.google.com/products/#product-launch-stages).

      - **Set post-scan results auto-deletion**: The auto-deletion time defines the duration a data profile scan remains active after execution. A data profile scan without a specified auto-deletion time is automatically removed after 24 hours. The auto-deletion time can range from 0 seconds (immediate deletion) to 365 days.
11. Click **Continue**.

12. Optional: Export the scan results to a BigQuery standard
    table. In the **Export scan results to BigQuery table** section, do the
    following:

    1. In the **Select BigQuery dataset** field, click **Browse**. Select a
       BigQuery dataset to store the data profile scan results.

    2. In the **BigQuery table** field, specify the table to store the data
       profile scan results. If you're using an existing table, make sure
       that it is compatible with the
       [export table schema](https://docs.cloud.google.com/dataplex/docs/use-data-profiling#table-schema).
       If the specified table doesn't exist, Knowledge Catalog creates
       it for you.

       > [!NOTE]
       > **Note:** You can use the same results table for multiple data profile scans.

13. Optional: Add labels. Labels are key-value pairs that let you group
    related objects together or with other Google Cloud resources.

14. To create the scan, click **Create**.

    If you set the schedule to on-demand, you can also run the scan now
    by clicking **Run scan**.

### gcloud

To create a data profile scan, use the
[`gcloud dataplex datascans create data-profile` command](https://docs.cloud.google.com/sdk/gcloud/reference/dataplex/datascans/create/data-profile).

If the source data is organized in a Knowledge Catalog lake, include
the `--data-source-entity` flag:

```
gcloud dataplex datascans create data-profile DATASCAN \
--location=LOCATION \
--data-source-entity=DATA_SOURCE_ENTITY
```

If the source data isn't organized in a Knowledge Catalog lake, include
the `--data-source-resource` flag:

```
gcloud dataplex datascans create data-profile DATASCAN \
--location=LOCATION \
--data-source-resource=DATA_SOURCE_RESOURCE
```

Replace the following variables:

- `DATASCAN`: The name of the data profile scan.
- `LOCATION`: The Google Cloud region in which to create the data profile scan.
- `DATA_SOURCE_ENTITY`: The Knowledge Catalog entity that contains the data for the data profile scan. For example, `projects/test-project/locations/test-location/lakes/test-lake/zones/test-zone/entities/test-entity`.
- `DATA_SOURCE_RESOURCE`: The name of the resource that contains the data for the data profile scan. For example, `//bigquery.googleapis.com/projects/test-project/datasets/test-dataset/tables/test-table`.

### C#


### C#


Before trying this sample, follow the C# setup instructions in the
[BigQuery quickstart using
client libraries](https://docs.cloud.google.com/dataplex/docs/reference/libraries).


For more information, see the
[BigQuery C# API
reference documentation](https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.Dataplex.V1/latest).


To authenticate to BigQuery, set up Application Default Credentials.
For more information, see

[Set up authentication for a local development environment](https://docs.cloud.google.com/docs/authentication/set-up-adc-local-dev-environment).

    using https://docs.cloud.google.com/dotnet/docs/reference/Google.Api.Gax/latest/Google.Api.Gax.ResourceNames.html;
    using https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.Dataplex.V1/latest/Google.Cloud.Dataplex.V1.html;
    using https://docs.cloud.google.com/dotnet/docs/reference/Google.LongRunning/latest/Google.LongRunning.html;

    public sealed partial class GeneratedDataScanServiceClientSnippets
    {
        /// <summary>Snippet for CreateDataScan</summary>
        /// <remarks>
        /// This snippet has been automatically generated and should be regarded as a code template only.
        /// It will require modifications to work:
        /// - It may require correct/in-range values for request initialization.
        /// - It may require specifying regional endpoints when creating the service client as shown in
        ///   https://cloud.google.com/dotnet/docs/reference/help/client-configuration#endpoint.
        /// </remarks>
        public void CreateDataScanRequestObject()
        {
            // Create client
            https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.Dataplex.V1/latest/Google.Cloud.Dataplex.V1.DataScanServiceClient.html dataScanServiceClient = https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.Dataplex.V1/latest/Google.Cloud.Dataplex.V1.DataScanServiceClient.html.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.Dataplex.V1/latest/Google.Cloud.Dataplex.V1.DataScanServiceClient.html#Google_Cloud_Dataplex_V1_DataScanServiceClient_Create();
            // Initialize request argument(s)
            https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.Dataplex.V1/latest/Google.Cloud.Dataplex.V1.CreateDataScanRequest.html request = new https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.Dataplex.V1/latest/Google.Cloud.Dataplex.V1.CreateDataScanRequest.html
            {
                ParentAsLocationName = https://docs.cloud.google.com/dotnet/docs/reference/Google.Api.Gax/latest/Google.Api.Gax.ResourceNames.LocationName.html.https://docs.cloud.google.com/dotnet/docs/reference/Google.Api.Gax/latest/Google.Api.Gax.ResourceNames.LocationName.html#Google_Api_Gax_ResourceNames_LocationName_FromProjectLocation_System_String_System_String_("[PROJECT]", "[LOCATION]"),
                DataScan = new https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.Dataplex.V1/latest/Google.Cloud.Dataplex.V1.DataScan.html(),
                DataScanId = "",
                ValidateOnly = false,
            };
            // Make the request
            Operation<DataScan, OperationMetadata> response = dataScanServiceClient.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.Dataplex.V1/latest/Google.Cloud.Dataplex.V1.DataScanServiceClient.html#Google_Cloud_Dataplex_V1_DataScanServiceClient_CreateDataScan_Google_Api_Gax_ResourceNames_LocationName_Google_Cloud_Dataplex_V1_DataScan_System_String_Google_Api_Gax_Grpc_CallSettings_(request);

            // Poll until the returned long-running operation is complete
            Operation<DataScan, OperationMetadata> completedResponse = response.PollUntilCompleted();
            // Retrieve the operation result
            https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.Dataplex.V1/latest/Google.Cloud.Dataplex.V1.DataScan.html result = completedResponse.Result;

            // Or get the name of the operation
            string operationName = response.Name;
            // This name can be stored, then the long-running operation retrieved later by name
            Operation<DataScan, OperationMetadata> retrievedResponse = dataScanServiceClient.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.Dataplex.V1/latest/Google.Cloud.Dataplex.V1.DataScanServiceClient.html#Google_Cloud_Dataplex_V1_DataScanServiceClient_PollOnceCreateDataScan_System_String_Google_Api_Gax_Grpc_CallSettings_(operationName);
            // Check if the retrieved long-running operation has completed
            if (retrievedResponse.IsCompleted)
            {
                // If it has completed, then access the result
                https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.Dataplex.V1/latest/Google.Cloud.Dataplex.V1.DataScan.html retrievedResult = retrievedResponse.Result;
            }
        }
    }

<br />

### Go

### Go


Before trying this sample, follow the Go setup instructions in the
[BigQuery quickstart using
client libraries](https://docs.cloud.google.com/dataplex/docs/reference/libraries).


For more information, see the
[BigQuery Go API
reference documentation](https://pkg.go.dev/cloud.google.com/go/dataplex).


To authenticate to BigQuery, set up Application Default Credentials.
For more information, see

[Set up authentication for a local development environment](https://docs.cloud.google.com/docs/authentication/set-up-adc-local-dev-environment).


    package main

    import (
    	"context"

    	dataplex "cloud.google.com/go/dataplex/apiv1"
    	dataplexpb "cloud.google.com/go/dataplex/apiv1/dataplexpb"
    )

    func main() {
    	ctx := context.Background()
    	// This snippet has been automatically generated and should be regarded as a code template only.
    	// It will require modifications to work:
    	// - It may require correct/in-range values for request initialization.
    	// - It may require specifying regional endpoints when creating the service client as shown in:
    	//   https://pkg.go.dev/cloud.google.com/go#hdr-Client_Options
    	c, err := dataplex.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/dataplex/latest/apiv1.html#cloud_google_com_go_dataplex_apiv1_DataScanClient_NewDataScanClient(ctx)
    	if err != nil {
    		// TODO: Handle error.
    	}
    	defer c.Close()

    	req := &dataplexpb.CreateDataScanRequest{
    		// TODO: Fill request struct fields.
    		// See https://pkg.go.dev/cloud.google.com/go/dataplex/apiv1/dataplexpb#CreateDataScanRequest.
    	}
    	op, err := c.CreateDataScan(ctx, req)
    	if err != nil {
    		// TODO: Handle error.
    	}

    	resp, err := op.Wait(ctx)
    	if err != nil {
    		// TODO: Handle error.
    	}
    	// TODO: Use resp.
    	_ = resp
    }

### Java

### Java


Before trying this sample, follow the Java setup instructions in the
[BigQuery quickstart using
client libraries](https://docs.cloud.google.com/dataplex/docs/reference/libraries).


For more information, see the
[BigQuery Java API
reference documentation](https://docs.cloud.google.com/java/docs/reference/google-cloud-dataplex/latest/overview).


To authenticate to BigQuery, set up Application Default Credentials.
For more information, see

[Set up authentication for a local development environment](https://docs.cloud.google.com/docs/authentication/set-up-adc-local-dev-environment).

    import com.google.cloud.dataplex.v1.https://docs.cloud.google.com/java/docs/reference/google-cloud-dataplex/latest/com.google.cloud.dataplex.v1.CreateDataScanRequest.html;
    import com.google.cloud.dataplex.v1.https://docs.cloud.google.com/java/docs/reference/google-cloud-dataplex/latest/com.google.cloud.dataplex.v1.DataScan.html;
    import com.google.cloud.dataplex.v1.https://docs.cloud.google.com/java/docs/reference/google-cloud-dataplex/latest/com.google.cloud.dataplex.v1.DataScanServiceClient.html;
    import com.google.cloud.dataplex.v1.https://docs.cloud.google.com/java/docs/reference/google-cloud-dataplex/latest/com.google.cloud.dataplex.v1.LocationName.html;

    public class SyncCreateDataScan {

      public static void main(String[] args) throws Exception {
        syncCreateDataScan();
      }

      public static void syncCreateDataScan() throws Exception {
        // This snippet has been automatically generated and should be regarded as a code template only.
        // It will require modifications to work:
        // - It may require correct/in-range values for request initialization.
        // - It may require specifying regional endpoints when creating the service client as shown in
        // https://cloud.google.com/java/docs/setup#configure_endpoints_for_the_client_library
        try (https://docs.cloud.google.com/java/docs/reference/google-cloud-dataplex/latest/com.google.cloud.dataplex.v1.DataScanServiceClient.html dataScanServiceClient = https://docs.cloud.google.com/java/docs/reference/google-cloud-dataplex/latest/com.google.cloud.dataplex.v1.DataScanServiceClient.html.create()) {
          https://docs.cloud.google.com/java/docs/reference/google-cloud-dataplex/latest/com.google.cloud.dataplex.v1.CreateDataScanRequest.html request =
              https://docs.cloud.google.com/java/docs/reference/google-cloud-dataplex/latest/com.google.cloud.dataplex.v1.CreateDataScanRequest.html.newBuilder()
                  .setParent(https://docs.cloud.google.com/java/docs/reference/google-cloud-dataplex/latest/com.google.cloud.dataplex.v1.LocationName.html.of("[PROJECT]", "[LOCATION]").toString())
                  .setDataScan(https://docs.cloud.google.com/java/docs/reference/google-cloud-dataplex/latest/com.google.cloud.dataplex.v1.DataScan.html.newBuilder().build())
                  .https://docs.cloud.google.com/java/docs/reference/google-cloud-dataplex/latest/com.google.cloud.dataplex.v1.CreateDataScanRequest.Builder.html#com_google_cloud_dataplex_v1_CreateDataScanRequest_Builder_setDataScanId_java_lang_String_("dataScanId1260787906")
                  .setValidateOnly(true)
                  .build();
          https://docs.cloud.google.com/java/docs/reference/google-cloud-dataplex/latest/com.google.cloud.dataplex.v1.DataScan.html response = dataScanServiceClient.https://docs.cloud.google.com/java/docs/reference/google-cloud-dataplex/latest/com.google.cloud.dataplex.v1.DataScanServiceClient.html#com_google_cloud_dataplex_v1_DataScanServiceClient_createDataScanAsync_com_google_cloud_dataplex_v1_CreateDataScanRequest_(request).get();
        }
      }
    }

### Python

### Python


Before trying this sample, follow the Python setup instructions in the
[BigQuery quickstart using
client libraries](https://docs.cloud.google.com/dataplex/docs/reference/libraries).


For more information, see the
[BigQuery Python API
reference documentation](https://docs.cloud.google.com/python/docs/reference/dataplex/latest).


To authenticate to BigQuery, set up Application Default Credentials.
For more information, see

[Set up authentication for a local development environment](https://docs.cloud.google.com/docs/authentication/set-up-adc-local-dev-environment).

    # This snippet has been automatically generated and should be regarded as a
    # code template only.
    # It will require modifications to work:
    # - It may require correct/in-range values for request initialization.
    # - It may require specifying regional endpoints when creating the service
    #   client as shown in:
    #   https://googleapis.dev/python/google-api-core/latest/client_options.html
    from google.cloud import https://docs.cloud.google.com/python/docs/reference/dataplex/latest


    def sample_create_data_scan():
        # Create a client
        client = https://docs.cloud.google.com/python/docs/reference/dataplex/latest.https://docs.cloud.google.com/python/docs/reference/dataplex/latest/google.cloud.dataplex_v1.services.data_scan_service.DataScanServiceClient.html()

        # Initialize request argument(s)
        data_scan = https://docs.cloud.google.com/python/docs/reference/dataplex/latest.https://docs.cloud.google.com/python/docs/reference/dataplex/latest/google.cloud.dataplex_v1.types.DataScan.html()
        data_scan.data.entity = "entity_value"

        request = https://docs.cloud.google.com/python/docs/reference/dataplex/latest.https://docs.cloud.google.com/python/docs/reference/dataplex/latest/google.cloud.dataplex_v1.types.CreateDataScanRequest.html(
            parent="parent_value",
            data_scan=data_scan,
            data_scan_id="data_scan_id_value",
        )

        # Make the request
        operation = client.https://docs.cloud.google.com/python/docs/reference/dataplex/latest/google.cloud.dataplex_v1.services.data_scan_service.DataScanServiceClient.html#google_cloud_dataplex_v1_services_data_scan_service_DataScanServiceClient_create_data_scan(request=request)

        print("Waiting for operation to complete...")

        response = operation.result()

        # Handle the response
        print(response)

### Ruby

### Ruby


Before trying this sample, follow the Ruby setup instructions in the
[BigQuery quickstart using
client libraries](https://docs.cloud.google.com/dataplex/docs/reference/libraries).


For more information, see the
[BigQuery Ruby API
reference documentation](https://docs.cloud.google.com/ruby/docs/reference/google-cloud-dataplex/latest).


To authenticate to BigQuery, set up Application Default Credentials.
For more information, see

[Set up authentication for a local development environment](https://docs.cloud.google.com/docs/authentication/set-up-adc-local-dev-environment).

    require "google/cloud/dataplex/v1"

    ##
    # Snippet for the create_data_scan call in the DataScanService service
    #
    # This snippet has been automatically generated and should be regarded as a code
    # template only. It will require modifications to work:
    # - It may require correct/in-range values for request initialization.
    # - It may require specifying regional endpoints when creating the service
    # client as shown in https://cloud.google.com/ruby/docs/reference.
    #
    # This is an auto-generated example demonstrating basic usage of
    # Google::Cloud::Dataplex::V1::DataScanService::Client#create_data_scan.
    #
    def create_data_scan
      # Create a client object. The client can be reused for multiple calls.
      client = Google::Cloud::Dataplex::V1::DataScanService::Client.new

      # Create a request. To set request fields, pass in keyword arguments.
      request = Google::Cloud::Dataplex::V1::CreateDataScanRequest.new

      # Call the create_data_scan method.
      result = client.create_data_scan request

      # The returned object is of type Gapic::Operation. You can use it to
      # check the status of an operation, cancel it, or wait for results.
      # Here is how to wait for a response.
      result.wait_until_done! timeout: 60
      if result.response?
        p result.response
      else
        puts "No response received."
      end
    end

### REST

To create a data profile scan, use the
[`dataScans.create` method](https://docs.cloud.google.com/dataplex/docs/reference/rest/v1/projects.locations.dataScans/create).

> [!NOTE]
> **Note:** If your BigQuery table is configured with the `Require
> partition filter` setting set to `true`, use the table's partition column as the data profile scan's row filter or timestamp column.

<br />

## Create multiple data profile scans


You can configure data profile scans for multiple tables in a
BigQuery dataset at the same time by using the Google Cloud console.

1.


   In the Google Cloud console, on the BigQuery
   **Metadata curation** page, go to the **Data profiling \& quality** tab.


   [Go to Data profiling \& quality](https://console.cloud.google.com/bigquery/governance/metadata-curation/data-profiling-and-quality)

   <br />

2. Click **Create data profile scan**.

3. Select the **Multiple data profile scans** option.

4. Enter an **ID prefix**. Knowledge Catalog automatically generates scan
   IDs by using the provided prefix and unique suffixes.

5. Enter a **Description** for all of the data profile scans.

6. In the **Dataset** field, click **Browse** . Select a dataset to pick tables
   from. Click **Select**.

7. If the dataset is multi-regional, select a **Region** in which to create the
   data profile scans.

8. In the **Mode** section, choose one of the following options:

   - **Standard**: profiles your data with customizable scan settings.
     This is the default mode.

   - **Lightweight** : provides quick insights with a low-latency, low-fidelity
     scan. This feature is in [Preview](https://cloud.google.com/products#product-launch-stages).

9. If you chose the **Standard** mode, configure the following settings for the
   scans. These settings don't appear when **Lightweight** mode is selected.

   1. In the **Scope** field, choose **Incremental** or **Entire data**.

      If you choose **Incremental** data, you can select only tables that
      are partitioned on a column of type `DATE` or `TIMESTAMP`.
   2. To apply sampling to the data profile scans, in the **Sampling size**
      list, select a sampling percentage.

      Choose a percentage value between 0.0% and 100.0% with up to 3 decimal
      digits.
10. Optional: Publish the data profile scan results in the
    BigQuery and Knowledge Catalog pages in the
    Google Cloud console for the source table. Select the
    **Publish results to Knowledge Catalog** checkbox.

    You can view the latest scan results in the **Data profile** tab in the
    BigQuery and Knowledge Catalog pages for the source
    table. To enable users to access the published scan results, see the
    [Grant access to data profile scan
    results](https://docs.cloud.google.com/bigquery/docs/data-profile-scan#share-results) section of this document.

    > [!NOTE]
    > **Note:** You must choose tables that don't have any existing scans publishing their results.

11. In the **Schedule** section, choose one of the following options:

    - **Repeat** : Run the data profile scans on a schedule: hourly, daily,
      weekly, monthly, or custom. Specify how often the scans should run and
      at what time. If you choose custom, use
      [cron](https://en.wikipedia.org/wiki/Cron) format to specify the
      schedule.

    - **On-demand**: Run the data profile scans on demand.

      - **One-time run** : Run the data profile scan once now, and remove
        the scan after the auto-deletion time. This feature is in
        [Preview](https://cloud.google.com/products/#product-launch-stages).

        - **Set post-scan results auto-deletion**: The auto-deletion time defines the duration a data profile scan remains active after execution. A data profile scan without a specified auto-deletion time is automatically removed after 24 hours. The auto-deletion time can range from 0 seconds (immediate deletion) to 365 days.
12. Click **Continue**.

13. In the **Choose tables** field, click **Browse** . Choose one or more tables
    to scan, and then click **Select**.

14. Click **Continue**.

15. Optional: Export the scan results to a BigQuery standard
    table. In the **Export scan results to BigQuery table** section, do the
    following:

    1. In the **Select BigQuery dataset** field, click **Browse**. Select a
       BigQuery dataset to store the data profile scan results.

    2. In the **BigQuery table** field, specify the table to store the data
       profile scan results. If you're using an existing table, make sure that
       it is compatible with the
       [export table schema](https://docs.cloud.google.com/dataplex/docs/use-data-profiling#table-schema).
       If the specified table doesn't exist, Knowledge Catalog creates it
       for you.

       Knowledge Catalog uses the same results table for all of the data
       profile scans.
16. Optional: Add labels. Labels are key-value pairs that let you group related
    objects together or with other Google Cloud resources.

17. To create the scans, click **Create**.

    If you set the schedule to on-demand, you can also run the scans now by
    clicking **Run scan**.

<br />

## Run a data profile scan


### Console

1. In the Google Cloud console, on the BigQuery **Metadata curation** page, go to the **Data profiling \& quality** tab.

   <br />


   [Go to Data profiling \& quality](https://console.cloud.google.com/bigquery/governance/metadata-curation/data-profiling-and-quality)
2. Click the data profile scan to run.
3. Click **Run now**.

### gcloud

To run a data profile scan, use the
[`gcloud dataplex datascans run` command](https://docs.cloud.google.com/sdk/gcloud/reference/dataplex/datascans/run):

```
gcloud dataplex datascans run DATASCAN \
--location=LOCATION
```

Replace the following variables:

- `DATASCAN`: The name of the data profile scan.
- `LOCATION`: The Google Cloud region in which the data profile scan was created.

### C#

### C#


Before trying this sample, follow the C# setup instructions in the
[BigQuery quickstart using
client libraries](https://docs.cloud.google.com/dataplex/docs/reference/libraries).


For more information, see the
[BigQuery C# API
reference documentation](https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.Dataplex.V1/latest).


To authenticate to BigQuery, set up Application Default Credentials.
For more information, see

[Set up authentication for a local development environment](https://docs.cloud.google.com/docs/authentication/set-up-adc-local-dev-environment).

    using https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.Dataplex.V1/latest/Google.Cloud.Dataplex.V1.html;

    public sealed partial class GeneratedDataScanServiceClientSnippets
    {
        /// <summary>Snippet for RunDataScan</summary>
        /// <remarks>
        /// This snippet has been automatically generated and should be regarded as a code template only.
        /// It will require modifications to work:
        /// - It may require correct/in-range values for request initialization.
        /// - It may require specifying regional endpoints when creating the service client as shown in
        ///   https://cloud.google.com/dotnet/docs/reference/help/client-configuration#endpoint.
        /// </remarks>
        public void RunDataScanRequestObject()
        {
            // Create client
            https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.Dataplex.V1/latest/Google.Cloud.Dataplex.V1.DataScanServiceClient.html dataScanServiceClient = https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.Dataplex.V1/latest/Google.Cloud.Dataplex.V1.DataScanServiceClient.html.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.Dataplex.V1/latest/Google.Cloud.Dataplex.V1.DataScanServiceClient.html#Google_Cloud_Dataplex_V1_DataScanServiceClient_Create();
            // Initialize request argument(s)
            https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.Dataplex.V1/latest/Google.Cloud.Dataplex.V1.RunDataScanRequest.html request = new https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.Dataplex.V1/latest/Google.Cloud.Dataplex.V1.RunDataScanRequest.html
            {
                DataScanName = https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.Dataplex.V1/latest/Google.Cloud.Dataplex.V1.DataScanName.html.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.Dataplex.V1/latest/Google.Cloud.Dataplex.V1.DataScanName.html#Google_Cloud_Dataplex_V1_DataScanName_FromProjectLocationDataScan_System_String_System_String_System_String_("[PROJECT]", "[LOCATION]", "[DATASCAN]"),
            };
            // Make the request
            https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.Dataplex.V1/latest/Google.Cloud.Dataplex.V1.RunDataScanResponse.html response = dataScanServiceClient.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.Dataplex.V1/latest/Google.Cloud.Dataplex.V1.DataScanServiceClient.html#Google_Cloud_Dataplex_V1_DataScanServiceClient_RunDataScan_Google_Cloud_Dataplex_V1_DataScanName_Google_Api_Gax_Grpc_CallSettings_(request);
        }
    }

### Go

### Go


Before trying this sample, follow the Go setup instructions in the
[BigQuery quickstart using
client libraries](https://docs.cloud.google.com/dataplex/docs/reference/libraries).


For more information, see the
[BigQuery Go API
reference documentation](https://pkg.go.dev/cloud.google.com/go/dataplex).


To authenticate to BigQuery, set up Application Default Credentials.
For more information, see

[Set up authentication for a local development environment](https://docs.cloud.google.com/docs/authentication/set-up-adc-local-dev-environment).


    package main

    import (
    	"context"

    	dataplex "cloud.google.com/go/dataplex/apiv1"
    	dataplexpb "cloud.google.com/go/dataplex/apiv1/dataplexpb"
    )

    func main() {
    	ctx := context.Background()
    	// This snippet has been automatically generated and should be regarded as a code template only.
    	// It will require modifications to work:
    	// - It may require correct/in-range values for request initialization.
    	// - It may require specifying regional endpoints when creating the service client as shown in:
    	//   https://pkg.go.dev/cloud.google.com/go#hdr-Client_Options
    	c, err := dataplex.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/dataplex/latest/apiv1.html#cloud_google_com_go_dataplex_apiv1_DataScanClient_NewDataScanClient(ctx)
    	if err != nil {
    		// TODO: Handle error.
    	}
    	defer c.Close()

    	req := &dataplexpb.RunDataScanRequest{
    		// TODO: Fill request struct fields.
    		// See https://pkg.go.dev/cloud.google.com/go/dataplex/apiv1/dataplexpb#RunDataScanRequest.
    	}
    	resp, err := c.RunDataScan(ctx, req)
    	if err != nil {
    		// TODO: Handle error.
    	}
    	// TODO: Use resp.
    	_ = resp
    }

### Java

### Java


Before trying this sample, follow the Java setup instructions in the
[BigQuery quickstart using
client libraries](https://docs.cloud.google.com/dataplex/docs/reference/libraries).


For more information, see the
[BigQuery Java API
reference documentation](https://docs.cloud.google.com/java/docs/reference/google-cloud-dataplex/latest/overview).


To authenticate to BigQuery, set up Application Default Credentials.
For more information, see

[Set up authentication for a local development environment](https://docs.cloud.google.com/docs/authentication/set-up-adc-local-dev-environment).

    import com.google.cloud.dataplex.v1.https://docs.cloud.google.com/java/docs/reference/google-cloud-dataplex/latest/com.google.cloud.dataplex.v1.DataScanName.html;
    import com.google.cloud.dataplex.v1.https://docs.cloud.google.com/java/docs/reference/google-cloud-dataplex/latest/com.google.cloud.dataplex.v1.DataScanServiceClient.html;
    import com.google.cloud.dataplex.v1.https://docs.cloud.google.com/java/docs/reference/google-cloud-dataplex/latest/com.google.cloud.dataplex.v1.RunDataScanRequest.html;
    import com.google.cloud.dataplex.v1.https://docs.cloud.google.com/java/docs/reference/google-cloud-dataplex/latest/com.google.cloud.dataplex.v1.RunDataScanResponse.html;

    public class SyncRunDataScan {

      public static void main(String[] args) throws Exception {
        syncRunDataScan();
      }

      public static void syncRunDataScan() throws Exception {
        // This snippet has been automatically generated and should be regarded as a code template only.
        // It will require modifications to work:
        // - It may require correct/in-range values for request initialization.
        // - It may require specifying regional endpoints when creating the service client as shown in
        // https://cloud.google.com/java/docs/setup#configure_endpoints_for_the_client_library
        try (https://docs.cloud.google.com/java/docs/reference/google-cloud-dataplex/latest/com.google.cloud.dataplex.v1.DataScanServiceClient.html dataScanServiceClient = https://docs.cloud.google.com/java/docs/reference/google-cloud-dataplex/latest/com.google.cloud.dataplex.v1.DataScanServiceClient.html.create()) {
          https://docs.cloud.google.com/java/docs/reference/google-cloud-dataplex/latest/com.google.cloud.dataplex.v1.RunDataScanRequest.html request =
              https://docs.cloud.google.com/java/docs/reference/google-cloud-dataplex/latest/com.google.cloud.dataplex.v1.RunDataScanRequest.html.newBuilder()
                  .setName(https://docs.cloud.google.com/java/docs/reference/google-cloud-dataplex/latest/com.google.cloud.dataplex.v1.DataScanName.html.of("[PROJECT]", "[LOCATION]", "[DATASCAN]").toString())
                  .build();
          https://docs.cloud.google.com/java/docs/reference/google-cloud-dataplex/latest/com.google.cloud.dataplex.v1.RunDataScanResponse.html response = dataScanServiceClient.runDataScan(request);
        }
      }
    }

### Python

### Python


Before trying this sample, follow the Python setup instructions in the
[BigQuery quickstart using
client libraries](https://docs.cloud.google.com/dataplex/docs/reference/libraries).


For more information, see the
[BigQuery Python API
reference documentation](https://docs.cloud.google.com/python/docs/reference/dataplex/latest).


To authenticate to BigQuery, set up Application Default Credentials.
For more information, see

[Set up authentication for a local development environment](https://docs.cloud.google.com/docs/authentication/set-up-adc-local-dev-environment).

    # This snippet has been automatically generated and should be regarded as a
    # code template only.
    # It will require modifications to work:
    # - It may require correct/in-range values for request initialization.
    # - It may require specifying regional endpoints when creating the service
    #   client as shown in:
    #   https://googleapis.dev/python/google-api-core/latest/client_options.html
    from google.cloud import https://docs.cloud.google.com/python/docs/reference/dataplex/latest


    def sample_run_data_scan():
        # Create a client
        client = https://docs.cloud.google.com/python/docs/reference/dataplex/latest.https://docs.cloud.google.com/python/docs/reference/dataplex/latest/google.cloud.dataplex_v1.services.data_scan_service.DataScanServiceClient.html()

        # Initialize request argument(s)
        request = https://docs.cloud.google.com/python/docs/reference/dataplex/latest.https://docs.cloud.google.com/python/docs/reference/dataplex/latest/google.cloud.dataplex_v1.types.RunDataScanRequest.html(
            name="name_value",
        )

        # Make the request
        response = client.https://docs.cloud.google.com/python/docs/reference/dataplex/latest/google.cloud.dataplex_v1.services.data_scan_service.DataScanServiceClient.html#google_cloud_dataplex_v1_services_data_scan_service_DataScanServiceClient_run_data_scan(request=request)

        # Handle the response
        print(response)

### Ruby

### Ruby


Before trying this sample, follow the Ruby setup instructions in the
[BigQuery quickstart using
client libraries](https://docs.cloud.google.com/dataplex/docs/reference/libraries).


For more information, see the
[BigQuery Ruby API
reference documentation](https://docs.cloud.google.com/ruby/docs/reference/google-cloud-dataplex/latest).


To authenticate to BigQuery, set up Application Default Credentials.
For more information, see

[Set up authentication for a local development environment](https://docs.cloud.google.com/docs/authentication/set-up-adc-local-dev-environment).

    require "google/cloud/dataplex/v1"

    ##
    # Snippet for the run_data_scan call in the DataScanService service
    #
    # This snippet has been automatically generated and should be regarded as a code
    # template only. It will require modifications to work:
    # - It may require correct/in-range values for request initialization.
    # - It may require specifying regional endpoints when creating the service
    # client as shown in https://cloud.google.com/ruby/docs/reference.
    #
    # This is an auto-generated example demonstrating basic usage of
    # Google::Cloud::Dataplex::V1::DataScanService::Client#run_data_scan.
    #
    def run_data_scan
      # Create a client object. The client can be reused for multiple calls.
      client = Google::Cloud::Dataplex::V1::DataScanService::Client.new

      # Create a request. To set request fields, pass in keyword arguments.
      request = Google::Cloud::Dataplex::V1::RunDataScanRequest.new

      # Call the run_data_scan method.
      result = client.run_data_scan request

      # The returned object is of type Google::Cloud::Dataplex::V1::RunDataScanResponse.
      p result
    end

### REST

To run a data profile scan, use the
[`dataScans.run` method](https://docs.cloud.google.com/dataplex/docs/reference/rest/v1/projects.locations.dataScans/run).

> [!NOTE]
> **Note:** Run isn't supported for data profile scans that are on a one-time schedule.

<br />

## View data profile scan results


### Console

1.


   In the Google Cloud console, on the BigQuery
   **Metadata curation** page, go to the **Data profiling \& quality** tab.


   [Go to Data profiling \& quality](https://console.cloud.google.com/bigquery/governance/metadata-curation/data-profiling-and-quality)

   <br />

2. Click the name of a data profile scan.

   - The **Overview** section displays information about the most recent
     jobs, including when the scan was run, the number of table records
     scanned, and the job status.

   - The **Data profile scan configuration** section displays details about
     the scan.

3. To see detailed information about a job, such as the scanned table's
   columns, statistics about the columns that were found in the scan, and the
   job logs, click the **Jobs history** tab. Then, click a job ID.

> [!NOTE]
> **Note:** If you exported the scan results to a BigQuery table, then you can also access the scan results from the table.

### gcloud

To view the results of a data profile scan job, use the
[`gcloud dataplex datascans jobs describe` command](https://docs.cloud.google.com/sdk/gcloud/reference/dataplex/datascans/jobs/describe):

```
gcloud dataplex datascans jobs describe JOB \
--location=LOCATION \
--datascan=DATASCAN \
--view=FULL
```

Replace the following variables:

- `JOB`: The job ID of the data profile scan job.
- `LOCATION`: The Google Cloud region in which the data profile scan was created.
- `DATASCAN`: The name of the data profile scan the job belongs to.
- `--view=FULL`: To see the scan job result, specify `FULL`.

### C#

### C#


Before trying this sample, follow the C# setup instructions in the
[BigQuery quickstart using
client libraries](https://docs.cloud.google.com/dataplex/docs/reference/libraries).


For more information, see the
[BigQuery C# API
reference documentation](https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.Dataplex.V1/latest).


To authenticate to BigQuery, set up Application Default Credentials.
For more information, see

[Set up authentication for a local development environment](https://docs.cloud.google.com/docs/authentication/set-up-adc-local-dev-environment).

    using https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.Dataplex.V1/latest/Google.Cloud.Dataplex.V1.html;

    public sealed partial class GeneratedDataScanServiceClientSnippets
    {
        /// <summary>Snippet for GetDataScan</summary>
        /// <remarks>
        /// This snippet has been automatically generated and should be regarded as a code template only.
        /// It will require modifications to work:
        /// - It may require correct/in-range values for request initialization.
        /// - It may require specifying regional endpoints when creating the service client as shown in
        ///   https://cloud.google.com/dotnet/docs/reference/help/client-configuration#endpoint.
        /// </remarks>
        public void GetDataScanRequestObject()
        {
            // Create client
            https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.Dataplex.V1/latest/Google.Cloud.Dataplex.V1.DataScanServiceClient.html dataScanServiceClient = https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.Dataplex.V1/latest/Google.Cloud.Dataplex.V1.DataScanServiceClient.html.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.Dataplex.V1/latest/Google.Cloud.Dataplex.V1.DataScanServiceClient.html#Google_Cloud_Dataplex_V1_DataScanServiceClient_Create();
            // Initialize request argument(s)
            https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.Dataplex.V1/latest/Google.Cloud.Dataplex.V1.GetDataScanRequest.html request = new https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.Dataplex.V1/latest/Google.Cloud.Dataplex.V1.GetDataScanRequest.html
            {
                DataScanName = https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.Dataplex.V1/latest/Google.Cloud.Dataplex.V1.DataScanName.html.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.Dataplex.V1/latest/Google.Cloud.Dataplex.V1.DataScanName.html#Google_Cloud_Dataplex_V1_DataScanName_FromProjectLocationDataScan_System_String_System_String_System_String_("[PROJECT]", "[LOCATION]", "[DATASCAN]"),
                View = https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.Dataplex.V1/latest/Google.Cloud.Dataplex.V1.GetDataScanRequest.html.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.Dataplex.V1/latest/Google.Cloud.Dataplex.V1.GetDataScanRequest.Types.html.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.Dataplex.V1/latest/Google.Cloud.Dataplex.V1.GetDataScanRequest.Types.DataScanView.html.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.Dataplex.V1/latest/Google.Cloud.Dataplex.V1.GetDataScanRequest.Types.DataScanView.html#Google_Cloud_Dataplex_V1_GetDataScanRequest_Types_DataScanView_Unspecified,
            };
            // Make the request
            https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.Dataplex.V1/latest/Google.Cloud.Dataplex.V1.DataScan.html response = dataScanServiceClient.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.Dataplex.V1/latest/Google.Cloud.Dataplex.V1.DataScanServiceClient.html#Google_Cloud_Dataplex_V1_DataScanServiceClient_GetDataScan_Google_Cloud_Dataplex_V1_DataScanName_Google_Api_Gax_Grpc_CallSettings_(request);
        }
    }

### Go

### Go


Before trying this sample, follow the Go setup instructions in the
[BigQuery quickstart using
client libraries](https://docs.cloud.google.com/dataplex/docs/reference/libraries).


For more information, see the
[BigQuery Go API
reference documentation](https://pkg.go.dev/cloud.google.com/go/dataplex).


To authenticate to BigQuery, set up Application Default Credentials.
For more information, see

[Set up authentication for a local development environment](https://docs.cloud.google.com/docs/authentication/set-up-adc-local-dev-environment).


    package main

    import (
    	"context"

    	dataplex "cloud.google.com/go/dataplex/apiv1"
    	dataplexpb "cloud.google.com/go/dataplex/apiv1/dataplexpb"
    )

    func main() {
    	ctx := context.Background()
    	// This snippet has been automatically generated and should be regarded as a code template only.
    	// It will require modifications to work:
    	// - It may require correct/in-range values for request initialization.
    	// - It may require specifying regional endpoints when creating the service client as shown in:
    	//   https://pkg.go.dev/cloud.google.com/go#hdr-Client_Options
    	c, err := dataplex.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/dataplex/latest/apiv1.html#cloud_google_com_go_dataplex_apiv1_DataScanClient_NewDataScanClient(ctx)
    	if err != nil {
    		// TODO: Handle error.
    	}
    	defer c.Close()

    	req := &dataplexpb.GetDataScanRequest{
    		// TODO: Fill request struct fields.
    		// See https://pkg.go.dev/cloud.google.com/go/dataplex/apiv1/dataplexpb#GetDataScanRequest.
    	}
    	resp, err := c.GetDataScan(ctx, req)
    	if err != nil {
    		// TODO: Handle error.
    	}
    	// TODO: Use resp.
    	_ = resp
    }

### Java

### Java


Before trying this sample, follow the Java setup instructions in the
[BigQuery quickstart using
client libraries](https://docs.cloud.google.com/dataplex/docs/reference/libraries).


For more information, see the
[BigQuery Java API
reference documentation](https://docs.cloud.google.com/java/docs/reference/google-cloud-dataplex/latest/overview).


To authenticate to BigQuery, set up Application Default Credentials.
For more information, see

[Set up authentication for a local development environment](https://docs.cloud.google.com/docs/authentication/set-up-adc-local-dev-environment).

    import com.google.cloud.dataplex.v1.https://docs.cloud.google.com/java/docs/reference/google-cloud-dataplex/latest/com.google.cloud.dataplex.v1.DataScan.html;
    import com.google.cloud.dataplex.v1.https://docs.cloud.google.com/java/docs/reference/google-cloud-dataplex/latest/com.google.cloud.dataplex.v1.DataScanName.html;
    import com.google.cloud.dataplex.v1.https://docs.cloud.google.com/java/docs/reference/google-cloud-dataplex/latest/com.google.cloud.dataplex.v1.DataScanServiceClient.html;
    import com.google.cloud.dataplex.v1.https://docs.cloud.google.com/java/docs/reference/google-cloud-dataplex/latest/com.google.cloud.dataplex.v1.GetDataScanRequest.html;

    public class SyncGetDataScan {

      public static void main(String[] args) throws Exception {
        syncGetDataScan();
      }

      public static void syncGetDataScan() throws Exception {
        // This snippet has been automatically generated and should be regarded as a code template only.
        // It will require modifications to work:
        // - It may require correct/in-range values for request initialization.
        // - It may require specifying regional endpoints when creating the service client as shown in
        // https://cloud.google.com/java/docs/setup#configure_endpoints_for_the_client_library
        try (https://docs.cloud.google.com/java/docs/reference/google-cloud-dataplex/latest/com.google.cloud.dataplex.v1.DataScanServiceClient.html dataScanServiceClient = https://docs.cloud.google.com/java/docs/reference/google-cloud-dataplex/latest/com.google.cloud.dataplex.v1.DataScanServiceClient.html.create()) {
          https://docs.cloud.google.com/java/docs/reference/google-cloud-dataplex/latest/com.google.cloud.dataplex.v1.GetDataScanRequest.html request =
              https://docs.cloud.google.com/java/docs/reference/google-cloud-dataplex/latest/com.google.cloud.dataplex.v1.GetDataScanRequest.html.newBuilder()
                  .setName(https://docs.cloud.google.com/java/docs/reference/google-cloud-dataplex/latest/com.google.cloud.dataplex.v1.DataScanName.html.of("[PROJECT]", "[LOCATION]", "[DATASCAN]").toString())
                  .build();
          https://docs.cloud.google.com/java/docs/reference/google-cloud-dataplex/latest/com.google.cloud.dataplex.v1.DataScan.html response = dataScanServiceClient.getDataScan(request);
        }
      }
    }

### Python

### Python


Before trying this sample, follow the Python setup instructions in the
[BigQuery quickstart using
client libraries](https://docs.cloud.google.com/dataplex/docs/reference/libraries).


For more information, see the
[BigQuery Python API
reference documentation](https://docs.cloud.google.com/python/docs/reference/dataplex/latest).


To authenticate to BigQuery, set up Application Default Credentials.
For more information, see

[Set up authentication for a local development environment](https://docs.cloud.google.com/docs/authentication/set-up-adc-local-dev-environment).

    # This snippet has been automatically generated and should be regarded as a
    # code template only.
    # It will require modifications to work:
    # - It may require correct/in-range values for request initialization.
    # - It may require specifying regional endpoints when creating the service
    #   client as shown in:
    #   https://googleapis.dev/python/google-api-core/latest/client_options.html
    from google.cloud import https://docs.cloud.google.com/python/docs/reference/dataplex/latest


    def sample_get_data_scan():
        # Create a client
        client = https://docs.cloud.google.com/python/docs/reference/dataplex/latest.https://docs.cloud.google.com/python/docs/reference/dataplex/latest/google.cloud.dataplex_v1.services.data_scan_service.DataScanServiceClient.html()

        # Initialize request argument(s)
        request = https://docs.cloud.google.com/python/docs/reference/dataplex/latest.https://docs.cloud.google.com/python/docs/reference/dataplex/latest/google.cloud.dataplex_v1.types.GetDataScanRequest.html(
            name="name_value",
        )

        # Make the request
        response = client.https://docs.cloud.google.com/python/docs/reference/dataplex/latest/google.cloud.dataplex_v1.services.data_scan_service.DataScanServiceClient.html#google_cloud_dataplex_v1_services_data_scan_service_DataScanServiceClient_get_data_scan(request=request)

        # Handle the response
        print(response)

### Ruby

### Ruby


Before trying this sample, follow the Ruby setup instructions in the
[BigQuery quickstart using
client libraries](https://docs.cloud.google.com/dataplex/docs/reference/libraries).


For more information, see the
[BigQuery Ruby API
reference documentation](https://docs.cloud.google.com/ruby/docs/reference/google-cloud-dataplex/latest).


To authenticate to BigQuery, set up Application Default Credentials.
For more information, see

[Set up authentication for a local development environment](https://docs.cloud.google.com/docs/authentication/set-up-adc-local-dev-environment).

    require "google/cloud/dataplex/v1"

    ##
    # Snippet for the get_data_scan call in the DataScanService service
    #
    # This snippet has been automatically generated and should be regarded as a code
    # template only. It will require modifications to work:
    # - It may require correct/in-range values for request initialization.
    # - It may require specifying regional endpoints when creating the service
    # client as shown in https://cloud.google.com/ruby/docs/reference.
    #
    # This is an auto-generated example demonstrating basic usage of
    # Google::Cloud::Dataplex::V1::DataScanService::Client#get_data_scan.
    #
    def get_data_scan
      # Create a client object. The client can be reused for multiple calls.
      client = Google::Cloud::Dataplex::V1::DataScanService::Client.new

      # Create a request. To set request fields, pass in keyword arguments.
      request = Google::Cloud::Dataplex::V1::GetDataScanRequest.new

      # Call the get_data_scan method.
      result = client.get_data_scan request

      # The returned object is of type Google::Cloud::Dataplex::V1::DataScan.
      p result
    end

### REST

To view the results of a data profile scan, use the
[`dataScans.get` method](https://docs.cloud.google.com/dataplex/docs/reference/rest/v1/projects.locations.dataScans/get).

<br />

### View published results


If the data profile scan results are published to the BigQuery
and Knowledge Catalog pages in the Google Cloud console, then you can
see the latest scan results on the source table's **Data profile** tab.

1. In the Google Cloud console, go to the BigQuery page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)

   If you don't see the left pane, click **Expand left pane** to open the pane.
3. In the **Explorer** pane, click **Datasets**, and then click your dataset.

4. Click **Overview \> Tables**, and then select the table whose data profile scan
   results you want to see.

5. Click the **Data profile** tab.

   The latest published results are displayed.

   > [!NOTE]
   > **Note:** Published results might not be available if a scan is running for the first time.

### View the most recent data profile scan job


### Console

1.


   In the Google Cloud console, on the BigQuery
   **Metadata curation** page, go to the **Data profiling \& quality** tab.


   [Go to Data profiling \& quality](https://console.cloud.google.com/bigquery/governance/metadata-curation/data-profiling-and-quality)

   <br />

2. Click the name of a data profile scan.

3. Click the **Latest job results** tab.

   The **Latest job results** tab, when there is at least one successfully
   completed run, provides information about the most recent job. It lists the scanned
   table's columns and statistics about the columns that were found in the scan.

### gcloud

To view the most recent successful data profile scan, use the
[`gcloud dataplex datascans describe` command](https://docs.cloud.google.com/sdk/gcloud/reference/dataplex/datascans/describe):

```
gcloud dataplex datascans describe DATASCAN \
--location=LOCATION \
--view=FULL
```

Replace the following variables:

- `DATASCAN`: The name of the data profile scan to view the most recent job for.
- `LOCATION`: The Google Cloud region in which the data profile scan was created.
- `--view=FULL`: To see the scan job result, specify `FULL`.

### REST

To view the most recent scan job, use the
[`dataScans.get` method](https://docs.cloud.google.com/dataplex/docs/reference/rest/v1/projects.locations.dataScans/get).

<br />

### View historical scan results


Knowledge Catalog saves the data profile scan history of the last 300
jobs or for the past year, whichever occurs first.

### Console

1.


   In the Google Cloud console, on the BigQuery
   **Metadata curation** page, go to the **Data profiling \& quality** tab.


   [Go to Data profiling \& quality](https://console.cloud.google.com/bigquery/governance/metadata-curation/data-profiling-and-quality)

   <br />

2. Click the name of a data profile scan.

3. Click the **Jobs history** tab.

   The **Jobs history** tab provides information about past jobs, such as
   the number of records scanned in each job, the job status, and the time the
   job was run.
4. To view detailed information about a job, click any of the jobs in the
   **Job ID** column.

### gcloud

To view historical data profile scan jobs, use the
[`gcloud dataplex datascans jobs list` command](https://docs.cloud.google.com/sdk/gcloud/reference/dataplex/datascans/jobs/list):

```
gcloud dataplex datascans jobs list \
--location=LOCATION \
--datascan=DATASCAN
```

Replace the following variables:

- `LOCATION`: The Google Cloud region in which the data profile scan was created.
- `DATASCAN`: The name of the data profile scan to view jobs for.

### C#

### C#


Before trying this sample, follow the C# setup instructions in the
[BigQuery quickstart using
client libraries](https://docs.cloud.google.com/dataplex/docs/reference/libraries).


For more information, see the
[BigQuery C# API
reference documentation](https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.Dataplex.V1/latest).


To authenticate to BigQuery, set up Application Default Credentials.
For more information, see

[Set up authentication for a local development environment](https://docs.cloud.google.com/docs/authentication/set-up-adc-local-dev-environment).

    using https://docs.cloud.google.com/dotnet/docs/reference/Google.Api.Gax/latest/Google.Api.Gax.html;
    using https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.Dataplex.V1/latest/Google.Cloud.Dataplex.V1.html;
    using System;

    public sealed partial class GeneratedDataScanServiceClientSnippets
    {
        /// <summary>Snippet for ListDataScanJobs</summary>
        /// <remarks>
        /// This snippet has been automatically generated and should be regarded as a code template only.
        /// It will require modifications to work:
        /// - It may require correct/in-range values for request initialization.
        /// - It may require specifying regional endpoints when creating the service client as shown in
        ///   https://cloud.google.com/dotnet/docs/reference/help/client-configuration#endpoint.
        /// </remarks>
        public void ListDataScanJobsRequestObject()
        {
            // Create client
            https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.Dataplex.V1/latest/Google.Cloud.Dataplex.V1.DataScanServiceClient.html dataScanServiceClient = https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.Dataplex.V1/latest/Google.Cloud.Dataplex.V1.DataScanServiceClient.html.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.Dataplex.V1/latest/Google.Cloud.Dataplex.V1.DataScanServiceClient.html#Google_Cloud_Dataplex_V1_DataScanServiceClient_Create();
            // Initialize request argument(s)
            https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.Dataplex.V1/latest/Google.Cloud.Dataplex.V1.ListDataScanJobsRequest.html request = new https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.Dataplex.V1/latest/Google.Cloud.Dataplex.V1.ListDataScanJobsRequest.html
            {
                ParentAsDataScanName = https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.Dataplex.V1/latest/Google.Cloud.Dataplex.V1.DataScanName.html.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.Dataplex.V1/latest/Google.Cloud.Dataplex.V1.DataScanName.html#Google_Cloud_Dataplex_V1_DataScanName_FromProjectLocationDataScan_System_String_System_String_System_String_("[PROJECT]", "[LOCATION]", "[DATASCAN]"),
                Filter = "",
            };
            // Make the request
            PagedEnumerable<ListDataScanJobsResponse, DataScanJob> response = dataScanServiceClient.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.Dataplex.V1/latest/Google.Cloud.Dataplex.V1.DataScanServiceClient.html#Google_Cloud_Dataplex_V1_DataScanServiceClient_ListDataScanJobs_Google_Cloud_Dataplex_V1_DataScanName_System_String_System_Nullable_System_Int32__Google_Api_Gax_Grpc_CallSettings_(request);

            // Iterate over all response items, lazily performing RPCs as required
            foreach (https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.Dataplex.V1/latest/Google.Cloud.Dataplex.V1.DataScanJob.html item in response)
            {
                // Do something with each item
                Console.WriteLine(item);
            }

            // Or iterate over pages (of server-defined size), performing one RPC per page
            foreach (https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.Dataplex.V1/latest/Google.Cloud.Dataplex.V1.ListDataScanJobsResponse.html page in response.AsRawResponses())
            {
                // Do something with each page of items
                Console.WriteLine("A page of results:");
                foreach (https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.Dataplex.V1/latest/Google.Cloud.Dataplex.V1.DataScanJob.html item in page)
                {
                    // Do something with each item
                    Console.WriteLine(item);
                }
            }

            // Or retrieve a single page of known size (unless it's the final page), performing as many RPCs as required
            int pageSize = 10;
            Page<DataScanJob> singlePage = response.ReadPage(pageSize);
            // Do something with the page of items
            Console.WriteLine($"A page of {pageSize} results (unless it's the final page):");
            foreach (https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.Dataplex.V1/latest/Google.Cloud.Dataplex.V1.DataScanJob.html item in singlePage)
            {
                // Do something with each item
                Console.WriteLine(item);
            }
            // Store the pageToken, for when the next page is required.
            string nextPageToken = singlePage.NextPageToken;
        }
    }

### Go

### Go


Before trying this sample, follow the Go setup instructions in the
[BigQuery quickstart using
client libraries](https://docs.cloud.google.com/dataplex/docs/reference/libraries).


For more information, see the
[BigQuery Go API
reference documentation](https://pkg.go.dev/cloud.google.com/go/dataplex).


To authenticate to BigQuery, set up Application Default Credentials.
For more information, see

[Set up authentication for a local development environment](https://docs.cloud.google.com/docs/authentication/set-up-adc-local-dev-environment).


    package main

    import (
    	"context"

    	dataplex "cloud.google.com/go/dataplex/apiv1"
    	dataplexpb "cloud.google.com/go/dataplex/apiv1/dataplexpb"
    	"google.golang.org/api/iterator"
    )

    func main() {
    	ctx := context.Background()
    	// This snippet has been automatically generated and should be regarded as a code template only.
    	// It will require modifications to work:
    	// - It may require correct/in-range values for request initialization.
    	// - It may require specifying regional endpoints when creating the service client as shown in:
    	//   https://pkg.go.dev/cloud.google.com/go#hdr-Client_Options
    	c, err := dataplex.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/dataplex/latest/apiv1.html#cloud_google_com_go_dataplex_apiv1_DataScanClient_NewDataScanClient(ctx)
    	if err != nil {
    		// TODO: Handle error.
    	}
    	defer c.Close()

    	req := &dataplexpb.ListDataScanJobsRequest{
    		// TODO: Fill request struct fields.
    		// See https://pkg.go.dev/cloud.google.com/go/dataplex/apiv1/dataplexpb#ListDataScanJobsRequest.
    	}
    	it := c.ListDataScanJobs(ctx, req)
    	for {
    		resp, err := it.Next()
    		if err == iterator.Done {
    			break
    		}
    		if err != nil {
    			// TODO: Handle error.
    		}
    		// TODO: Use resp.
    		_ = resp

    		// If you need to access the underlying RPC response,
    		// you can do so by casting the `Response` as below.
    		// Otherwise, remove this line. Only populated after
    		// first call to Next(). Not safe for concurrent access.
    		_ = it.Response.(*dataplexpb.ListDataScanJobsResponse)
    	}
    }

### Java

### Java


Before trying this sample, follow the Java setup instructions in the
[BigQuery quickstart using
client libraries](https://docs.cloud.google.com/dataplex/docs/reference/libraries).


For more information, see the
[BigQuery Java API
reference documentation](https://docs.cloud.google.com/java/docs/reference/google-cloud-dataplex/latest/overview).


To authenticate to BigQuery, set up Application Default Credentials.
For more information, see

[Set up authentication for a local development environment](https://docs.cloud.google.com/docs/authentication/set-up-adc-local-dev-environment).

    import com.google.cloud.dataplex.v1.https://docs.cloud.google.com/java/docs/reference/google-cloud-dataplex/latest/com.google.cloud.dataplex.v1.DataScanJob.html;
    import com.google.cloud.dataplex.v1.https://docs.cloud.google.com/java/docs/reference/google-cloud-dataplex/latest/com.google.cloud.dataplex.v1.DataScanName.html;
    import com.google.cloud.dataplex.v1.https://docs.cloud.google.com/java/docs/reference/google-cloud-dataplex/latest/com.google.cloud.dataplex.v1.DataScanServiceClient.html;
    import com.google.cloud.dataplex.v1.https://docs.cloud.google.com/java/docs/reference/google-cloud-dataplex/latest/com.google.cloud.dataplex.v1.ListDataScanJobsRequest.html;

    public class SyncListDataScanJobs {

      public static void main(String[] args) throws Exception {
        syncListDataScanJobs();
      }

      public static void syncListDataScanJobs() throws Exception {
        // This snippet has been automatically generated and should be regarded as a code template only.
        // It will require modifications to work:
        // - It may require correct/in-range values for request initialization.
        // - It may require specifying regional endpoints when creating the service client as shown in
        // https://cloud.google.com/java/docs/setup#configure_endpoints_for_the_client_library
        try (https://docs.cloud.google.com/java/docs/reference/google-cloud-dataplex/latest/com.google.cloud.dataplex.v1.DataScanServiceClient.html dataScanServiceClient = https://docs.cloud.google.com/java/docs/reference/google-cloud-dataplex/latest/com.google.cloud.dataplex.v1.DataScanServiceClient.html.create()) {
          https://docs.cloud.google.com/java/docs/reference/google-cloud-dataplex/latest/com.google.cloud.dataplex.v1.ListDataScanJobsRequest.html request =
              https://docs.cloud.google.com/java/docs/reference/google-cloud-dataplex/latest/com.google.cloud.dataplex.v1.ListDataScanJobsRequest.html.newBuilder()
                  .setParent(https://docs.cloud.google.com/java/docs/reference/google-cloud-dataplex/latest/com.google.cloud.dataplex.v1.DataScanName.html.of("[PROJECT]", "[LOCATION]", "[DATASCAN]").toString())
                  .setPageSize(883849137)
                  .setPageToken("pageToken873572522")
                  .setFilter("filter-1274492040")
                  .build();
          for (https://docs.cloud.google.com/java/docs/reference/google-cloud-dataplex/latest/com.google.cloud.dataplex.v1.DataScanJob.html element : dataScanServiceClient.listDataScanJobs(request).iterateAll()) {
            // doThingsWith(element);
          }
        }
      }
    }

### Python

### Python


Before trying this sample, follow the Python setup instructions in the
[BigQuery quickstart using
client libraries](https://docs.cloud.google.com/dataplex/docs/reference/libraries).


For more information, see the
[BigQuery Python API
reference documentation](https://docs.cloud.google.com/python/docs/reference/dataplex/latest).


To authenticate to BigQuery, set up Application Default Credentials.
For more information, see

[Set up authentication for a local development environment](https://docs.cloud.google.com/docs/authentication/set-up-adc-local-dev-environment).

    # This snippet has been automatically generated and should be regarded as a
    # code template only.
    # It will require modifications to work:
    # - It may require correct/in-range values for request initialization.
    # - It may require specifying regional endpoints when creating the service
    #   client as shown in:
    #   https://googleapis.dev/python/google-api-core/latest/client_options.html
    from google.cloud import https://docs.cloud.google.com/python/docs/reference/dataplex/latest


    def sample_list_data_scan_jobs():
        # Create a client
        client = https://docs.cloud.google.com/python/docs/reference/dataplex/latest.https://docs.cloud.google.com/python/docs/reference/dataplex/latest/google.cloud.dataplex_v1.services.data_scan_service.DataScanServiceClient.html()

        # Initialize request argument(s)
        request = https://docs.cloud.google.com/python/docs/reference/dataplex/latest.https://docs.cloud.google.com/python/docs/reference/dataplex/latest/google.cloud.dataplex_v1.types.ListDataScanJobsRequest.html(
            parent="parent_value",
        )

        # Make the request
        page_result = client.https://docs.cloud.google.com/python/docs/reference/dataplex/latest/google.cloud.dataplex_v1.services.data_scan_service.DataScanServiceClient.html#google_cloud_dataplex_v1_services_data_scan_service_DataScanServiceClient_list_data_scan_jobs(request=request)

        # Handle the response
        for response in page_result:
            print(response)

### Ruby

### Ruby


Before trying this sample, follow the Ruby setup instructions in the
[BigQuery quickstart using
client libraries](https://docs.cloud.google.com/dataplex/docs/reference/libraries).


For more information, see the
[BigQuery Ruby API
reference documentation](https://docs.cloud.google.com/ruby/docs/reference/google-cloud-dataplex/latest).


To authenticate to BigQuery, set up Application Default Credentials.
For more information, see

[Set up authentication for a local development environment](https://docs.cloud.google.com/docs/authentication/set-up-adc-local-dev-environment).

    require "google/cloud/dataplex/v1"

    ##
    # Snippet for the list_data_scan_jobs call in the DataScanService service
    #
    # This snippet has been automatically generated and should be regarded as a code
    # template only. It will require modifications to work:
    # - It may require correct/in-range values for request initialization.
    # - It may require specifying regional endpoints when creating the service
    # client as shown in https://cloud.google.com/ruby/docs/reference.
    #
    # This is an auto-generated example demonstrating basic usage of
    # Google::Cloud::Dataplex::V1::DataScanService::Client#list_data_scan_jobs.
    #
    def list_data_scan_jobs
      # Create a client object. The client can be reused for multiple calls.
      client = Google::Cloud::Dataplex::V1::DataScanService::Client.new

      # Create a request. To set request fields, pass in keyword arguments.
      request = Google::Cloud::Dataplex::V1::ListDataScanJobsRequest.new

      # Call the list_data_scan_jobs method.
      result = client.list_data_scan_jobs request

      # The returned object is of type Gapic::PagedEnumerable. You can iterate
      # over elements, and API calls will be issued to fetch pages as needed.
      result.each do |item|
        # Each element is of type ::Google::Cloud::Dataplex::V1::DataScanJob.
        p item
      end
    end

### REST

To view historical data profile scan jobs, use the
[`dataScans.jobs.list` method](https://docs.cloud.google.com/dataplex/docs/reference/rest/v1/projects.locations.dataScans.jobs/list).

<br />

### View the data profile scans for a table

To view the data profile scans that apply to a specific table, do the following:

1. In the Google Cloud console, on the BigQuery
   **Metadata curation** page, go to the **Data profiling \& quality** tab.


   [Go to Data profiling \& quality](https://console.cloud.google.com/bigquery/governance/metadata-curation/data-profiling-and-quality)

   <br />

2. Filter the list by table name and scan type.

## Grant access to data profile scan results


To enable the users in your organization to view the scan results, do the following:

1.


   In the Google Cloud console, on the BigQuery
   **Metadata curation** page, go to the **Data profiling \& quality** tab.


   [Go to Data profiling \& quality](https://console.cloud.google.com/bigquery/governance/metadata-curation/data-profiling-and-quality)

   <br />

2. Click the data profile scan you want to share the results of.

3. Click the **Permissions** tab.

4. Do the following:

   - To grant access to a principal, click **Grant access** . Grant the **Dataplex DataScan DataViewer** role to the associated principal.
   - To remove access from a principal, select the principal that you want to remove the **Dataplex DataScan DataViewer** role from. Click **Remove access**, and then confirm when prompted.

<br />

## Manage data profile scans for a specific table

The steps in this document show how to manage data profile scans across your
project by using the BigQuery
**Metadata curation \> Data profiling \& quality** page in the
Google Cloud console.

You can also create and manage data profile scans when working with a
specific table. In the Google Cloud console, on the BigQuery
page for the table, use the **Data profile** tab. Do the following:


1.

   In the Google Cloud console, go to the **BigQuery** page.


   [Go to BigQuery](https://console.cloud.google.com/bigquery)

   <br />

   In the **Explorer** pane (in the left pane), click **Datasets** , and then click your dataset.
   Now click **Overview \> Tables**, and select the table whose data profile scan
   results you want to see.

2. Click the **Data profile** tab.

3. Depending on whether the table has a data profile scan whose results are
   published, you can work with the table's data profile scans in the following ways:

   - **Data profile scan results are published**: the latest published scan
     results are displayed on the page.

     To manage the data profile scans for this table, click **Data profile
     scan**, and then select from the following options:
     - **Create new scan** : create a new data profile scan. For more
       information, see the [Create a data profile scan](https://docs.cloud.google.com/bigquery/docs/data-profile-scan#create-scan) section
       of this document. When you create a scan from a table's details page, the
       table is preselected.

     - **Run now**: run the scan.

     - **Edit scan configuration**: edit settings including the display name,
       filters, sampling size, and schedule.

     - **Manage scan permissions** : control who can access the scan results.
       For more information, see the
       [Grant access to data profile scan results](https://docs.cloud.google.com/bigquery/docs/data-profile-scan#share-results)
       section of this document.

     - **View historical results** : view detailed information about previous
       data profile scan jobs. For more information, see the
       [View data profile scan results](https://docs.cloud.google.com/bigquery/docs/data-profile-scan#results) and
       [View historical scan results](https://docs.cloud.google.com/bigquery/docs/data-profile-scan#older-scans) sections of
       this document.

     - **View all scans**: view a list of data profile scans that apply to this
       table.

   - **Data profile scan results aren't published** : click the menu next to
     **Quick data profile**, and then select from the following options:

     - **Customize data profiling** : create a new data profile scan. For more
       information, see the
       [Create a data profile scan](https://docs.cloud.google.com/bigquery/docs/data-profile-scan#create-scan) section
       of this document. When you create a scan from a table's details page, the
       table is preselected.

     - **View previous profiles**: view a list of data profile scans that
       apply to this table.

<br />

## Update a data profile scan


### Console

1.


   In the Google Cloud console, on the BigQuery
   **Metadata curation** page, go to the **Data profiling \& quality** tab.


   [Go to Data profiling \& quality](https://console.cloud.google.com/bigquery/governance/metadata-curation/data-profiling-and-quality)

   <br />

2. Click the name of a data profile scan.

3. Click **Edit**, and then edit the values.

4. Click **Save**.

### gcloud

To update a data profile scan, use the
[`gcloud dataplex datascans update data-profile` command](https://docs.cloud.google.com/sdk/gcloud/reference/dataplex/datascans/update/data-profile):

```
gcloud dataplex datascans update data-profile DATASCAN \
--location=LOCATION \
--description=DESCRIPTION
```

Replace the following variables:

- `DATASCAN`: The name of the data profile scan to update.
- `LOCATION`: The Google Cloud region in which the data profile scan was created.
- `DESCRIPTION`: The new description for the data profile scan.

> [!NOTE]
> **Note:** You can update specification fields, such as `rowFilter`, `samplingPercent`, or `includeFields`, in the data profile specification file. See the [JSON format](https://docs.cloud.google.com/dataplex/docs/reference/rest/v1/DataProfileSpec).

### C#

### C#


Before trying this sample, follow the C# setup instructions in the
[BigQuery quickstart using
client libraries](https://docs.cloud.google.com/dataplex/docs/reference/libraries).


For more information, see the
[BigQuery C# API
reference documentation](https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.Dataplex.V1/latest).


To authenticate to BigQuery, set up Application Default Credentials.
For more information, see

[Set up authentication for a local development environment](https://docs.cloud.google.com/docs/authentication/set-up-adc-local-dev-environment).

    using https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.Dataplex.V1/latest/Google.Cloud.Dataplex.V1.html;
    using https://docs.cloud.google.com/dotnet/docs/reference/Google.LongRunning/latest/Google.LongRunning.html;
    using https://docs.cloud.google.com/dotnet/docs/reference/Google.Protobuf/latest/Google.Protobuf.WellKnownTypes.html;

    public sealed partial class GeneratedDataScanServiceClientSnippets
    {
        /// <summary>Snippet for UpdateDataScan</summary>
        /// <remarks>
        /// This snippet has been automatically generated and should be regarded as a code template only.
        /// It will require modifications to work:
        /// - It may require correct/in-range values for request initialization.
        /// - It may require specifying regional endpoints when creating the service client as shown in
        ///   https://cloud.google.com/dotnet/docs/reference/help/client-configuration#endpoint.
        /// </remarks>
        public void UpdateDataScanRequestObject()
        {
            // Create client
            https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.Dataplex.V1/latest/Google.Cloud.Dataplex.V1.DataScanServiceClient.html dataScanServiceClient = https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.Dataplex.V1/latest/Google.Cloud.Dataplex.V1.DataScanServiceClient.html.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.Dataplex.V1/latest/Google.Cloud.Dataplex.V1.DataScanServiceClient.html#Google_Cloud_Dataplex_V1_DataScanServiceClient_Create();
            // Initialize request argument(s)
            https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.Dataplex.V1/latest/Google.Cloud.Dataplex.V1.UpdateDataScanRequest.html request = new https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.Dataplex.V1/latest/Google.Cloud.Dataplex.V1.UpdateDataScanRequest.html
            {
                DataScan = new https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.Dataplex.V1/latest/Google.Cloud.Dataplex.V1.DataScan.html(),
                UpdateMask = new https://docs.cloud.google.com/dotnet/docs/reference/Google.Protobuf/latest/Google.Protobuf.WellKnownTypes.FieldMask.html(),
                ValidateOnly = false,
            };
            // Make the request
            Operation<DataScan, OperationMetadata> response = dataScanServiceClient.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.Dataplex.V1/latest/Google.Cloud.Dataplex.V1.DataScanServiceClient.html#Google_Cloud_Dataplex_V1_DataScanServiceClient_UpdateDataScan_Google_Cloud_Dataplex_V1_DataScan_Google_Protobuf_WellKnownTypes_FieldMask_Google_Api_Gax_Grpc_CallSettings_(request);

            // Poll until the returned long-running operation is complete
            Operation<DataScan, OperationMetadata> completedResponse = response.PollUntilCompleted();
            // Retrieve the operation result
            https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.Dataplex.V1/latest/Google.Cloud.Dataplex.V1.DataScan.html result = completedResponse.Result;

            // Or get the name of the operation
            string operationName = response.Name;
            // This name can be stored, then the long-running operation retrieved later by name
            Operation<DataScan, OperationMetadata> retrievedResponse = dataScanServiceClient.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.Dataplex.V1/latest/Google.Cloud.Dataplex.V1.DataScanServiceClient.html#Google_Cloud_Dataplex_V1_DataScanServiceClient_PollOnceUpdateDataScan_System_String_Google_Api_Gax_Grpc_CallSettings_(operationName);
            // Check if the retrieved long-running operation has completed
            if (retrievedResponse.IsCompleted)
            {
                // If it has completed, then access the result
                https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.Dataplex.V1/latest/Google.Cloud.Dataplex.V1.DataScan.html retrievedResult = retrievedResponse.Result;
            }
        }
    }

### Go

### Go


Before trying this sample, follow the Go setup instructions in the
[BigQuery quickstart using
client libraries](https://docs.cloud.google.com/dataplex/docs/reference/libraries).


For more information, see the
[BigQuery Go API
reference documentation](https://pkg.go.dev/cloud.google.com/go/dataplex).


To authenticate to BigQuery, set up Application Default Credentials.
For more information, see

[Set up authentication for a local development environment](https://docs.cloud.google.com/docs/authentication/set-up-adc-local-dev-environment).


    package main

    import (
    	"context"

    	dataplex "cloud.google.com/go/dataplex/apiv1"
    	dataplexpb "cloud.google.com/go/dataplex/apiv1/dataplexpb"
    )

    func main() {
    	ctx := context.Background()
    	// This snippet has been automatically generated and should be regarded as a code template only.
    	// It will require modifications to work:
    	// - It may require correct/in-range values for request initialization.
    	// - It may require specifying regional endpoints when creating the service client as shown in:
    	//   https://pkg.go.dev/cloud.google.com/go#hdr-Client_Options
    	c, err := dataplex.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/dataplex/latest/apiv1.html#cloud_google_com_go_dataplex_apiv1_DataScanClient_NewDataScanClient(ctx)
    	if err != nil {
    		// TODO: Handle error.
    	}
    	defer c.Close()

    	req := &dataplexpb.UpdateDataScanRequest{
    		// TODO: Fill request struct fields.
    		// See https://pkg.go.dev/cloud.google.com/go/dataplex/apiv1/dataplexpb#UpdateDataScanRequest.
    	}
    	op, err := c.UpdateDataScan(ctx, req)
    	if err != nil {
    		// TODO: Handle error.
    	}

    	resp, err := op.Wait(ctx)
    	if err != nil {
    		// TODO: Handle error.
    	}
    	// TODO: Use resp.
    	_ = resp
    }

### Java

### Java


Before trying this sample, follow the Java setup instructions in the
[BigQuery quickstart using
client libraries](https://docs.cloud.google.com/dataplex/docs/reference/libraries).


For more information, see the
[BigQuery Java API
reference documentation](https://docs.cloud.google.com/java/docs/reference/google-cloud-dataplex/latest/overview).


To authenticate to BigQuery, set up Application Default Credentials.
For more information, see

[Set up authentication for a local development environment](https://docs.cloud.google.com/docs/authentication/set-up-adc-local-dev-environment).

    import com.google.cloud.dataplex.v1.https://docs.cloud.google.com/java/docs/reference/google-cloud-dataplex/latest/com.google.cloud.dataplex.v1.DataScan.html;
    import com.google.cloud.dataplex.v1.https://docs.cloud.google.com/java/docs/reference/google-cloud-dataplex/latest/com.google.cloud.dataplex.v1.DataScanServiceClient.html;
    import com.google.cloud.dataplex.v1.https://docs.cloud.google.com/java/docs/reference/google-cloud-dataplex/latest/com.google.cloud.dataplex.v1.UpdateDataScanRequest.html;
    import com.google.protobuf.https://docs.cloud.google.com/java/docs/reference/protobuf/latest/com.google.protobuf.FieldMask.html;

    public class SyncUpdateDataScan {

      public static void main(String[] args) throws Exception {
        syncUpdateDataScan();
      }

      public static void syncUpdateDataScan() throws Exception {
        // This snippet has been automatically generated and should be regarded as a code template only.
        // It will require modifications to work:
        // - It may require correct/in-range values for request initialization.
        // - It may require specifying regional endpoints when creating the service client as shown in
        // https://cloud.google.com/java/docs/setup#configure_endpoints_for_the_client_library
        try (https://docs.cloud.google.com/java/docs/reference/google-cloud-dataplex/latest/com.google.cloud.dataplex.v1.DataScanServiceClient.html dataScanServiceClient = https://docs.cloud.google.com/java/docs/reference/google-cloud-dataplex/latest/com.google.cloud.dataplex.v1.DataScanServiceClient.html.create()) {
          https://docs.cloud.google.com/java/docs/reference/google-cloud-dataplex/latest/com.google.cloud.dataplex.v1.UpdateDataScanRequest.html request =
              https://docs.cloud.google.com/java/docs/reference/google-cloud-dataplex/latest/com.google.cloud.dataplex.v1.UpdateDataScanRequest.html.newBuilder()
                  .setDataScan(https://docs.cloud.google.com/java/docs/reference/google-cloud-dataplex/latest/com.google.cloud.dataplex.v1.DataScan.html.newBuilder().build())
                  .setUpdateMask(https://docs.cloud.google.com/java/docs/reference/protobuf/latest/com.google.protobuf.FieldMask.html.newBuilder().build())
                  .setValidateOnly(true)
                  .build();
          https://docs.cloud.google.com/java/docs/reference/google-cloud-dataplex/latest/com.google.cloud.dataplex.v1.DataScan.html response = dataScanServiceClient.https://docs.cloud.google.com/java/docs/reference/google-cloud-dataplex/latest/com.google.cloud.dataplex.v1.DataScanServiceClient.html#com_google_cloud_dataplex_v1_DataScanServiceClient_updateDataScanAsync_com_google_cloud_dataplex_v1_DataScan_com_google_protobuf_FieldMask_(request).get();
        }
      }
    }

### Python

### Python


Before trying this sample, follow the Python setup instructions in the
[BigQuery quickstart using
client libraries](https://docs.cloud.google.com/dataplex/docs/reference/libraries).


For more information, see the
[BigQuery Python API
reference documentation](https://docs.cloud.google.com/python/docs/reference/dataplex/latest).


To authenticate to BigQuery, set up Application Default Credentials.
For more information, see

[Set up authentication for a local development environment](https://docs.cloud.google.com/docs/authentication/set-up-adc-local-dev-environment).

    # This snippet has been automatically generated and should be regarded as a
    # code template only.
    # It will require modifications to work:
    # - It may require correct/in-range values for request initialization.
    # - It may require specifying regional endpoints when creating the service
    #   client as shown in:
    #   https://googleapis.dev/python/google-api-core/latest/client_options.html
    from google.cloud import https://docs.cloud.google.com/python/docs/reference/dataplex/latest


    def sample_update_data_scan():
        # Create a client
        client = https://docs.cloud.google.com/python/docs/reference/dataplex/latest.https://docs.cloud.google.com/python/docs/reference/dataplex/latest/google.cloud.dataplex_v1.services.data_scan_service.DataScanServiceClient.html()

        # Initialize request argument(s)
        data_scan = https://docs.cloud.google.com/python/docs/reference/dataplex/latest.https://docs.cloud.google.com/python/docs/reference/dataplex/latest/google.cloud.dataplex_v1.types.DataScan.html()
        data_scan.data.entity = "entity_value"

        request = https://docs.cloud.google.com/python/docs/reference/dataplex/latest.https://docs.cloud.google.com/python/docs/reference/dataplex/latest/google.cloud.dataplex_v1.types.UpdateDataScanRequest.html(
            data_scan=data_scan,
        )

        # Make the request
        operation = client.https://docs.cloud.google.com/python/docs/reference/dataplex/latest/google.cloud.dataplex_v1.services.data_scan_service.DataScanServiceClient.html#google_cloud_dataplex_v1_services_data_scan_service_DataScanServiceClient_update_data_scan(request=request)

        print("Waiting for operation to complete...")

        response = operation.result()

        # Handle the response
        print(response)

### Ruby

### Ruby


Before trying this sample, follow the Ruby setup instructions in the
[BigQuery quickstart using
client libraries](https://docs.cloud.google.com/dataplex/docs/reference/libraries).


For more information, see the
[BigQuery Ruby API
reference documentation](https://docs.cloud.google.com/ruby/docs/reference/google-cloud-dataplex/latest).


To authenticate to BigQuery, set up Application Default Credentials.
For more information, see

[Set up authentication for a local development environment](https://docs.cloud.google.com/docs/authentication/set-up-adc-local-dev-environment).

    require "google/cloud/dataplex/v1"

    ##
    # Snippet for the update_data_scan call in the DataScanService service
    #
    # This snippet has been automatically generated and should be regarded as a code
    # template only. It will require modifications to work:
    # - It may require correct/in-range values for request initialization.
    # - It may require specifying regional endpoints when creating the service
    # client as shown in https://cloud.google.com/ruby/docs/reference.
    #
    # This is an auto-generated example demonstrating basic usage of
    # Google::Cloud::Dataplex::V1::DataScanService::Client#update_data_scan.
    #
    def update_data_scan
      # Create a client object. The client can be reused for multiple calls.
      client = Google::Cloud::Dataplex::V1::DataScanService::Client.new

      # Create a request. To set request fields, pass in keyword arguments.
      request = Google::Cloud::Dataplex::V1::UpdateDataScanRequest.new

      # Call the update_data_scan method.
      result = client.update_data_scan request

      # The returned object is of type Gapic::Operation. You can use it to
      # check the status of an operation, cancel it, or wait for results.
      # Here is how to wait for a response.
      result.wait_until_done! timeout: 60
      if result.response?
        p result.response
      else
        puts "No response received."
      end
    end

### REST

To edit a data profile scan, use the
[`dataScans.patch` method](https://docs.cloud.google.com/dataplex/docs/reference/rest/v1/projects.locations.dataScans/patch).

> [!NOTE]
> **Note:** Update isn't supported for data profile scans that are on a one-time schedule.

<br />

## Delete a data profile scan


### Console

1.


   In the Google Cloud console, on the BigQuery
   **Metadata curation** page, go to the **Data profiling \& quality** tab.


   [Go to Data profiling \& quality](https://console.cloud.google.com/bigquery/governance/metadata-curation/data-profiling-and-quality)

   <br />

2. Click the scan you want to delete.

3. Click **Delete**, and then confirm when prompted.

### gcloud

To delete a data profile scan, use the
[`gcloud dataplex datascans delete` command](https://docs.cloud.google.com/sdk/gcloud/reference/dataplex/datascans/delete):

```
gcloud dataplex datascans delete DATASCAN \
--location=LOCATION --async
```

Replace the following variables:

- `DATASCAN`: The name of the data profile scan to delete.
- `LOCATION`: The Google Cloud region in which the data profile scan was created.

### REST

To delete a data profile scan, use the
[`dataScans.delete` method](https://docs.cloud.google.com/dataplex/docs/reference/rest/v1/projects.locations.dataScans/delete).

> [!NOTE]
> **Note:** Delete isn't supported for data profile scans that are on a one-time schedule.

<br />

## What's next

- Learn how to [explore your data by generating data insights](https://docs.cloud.google.com/bigquery/docs/data-insights).
- Learn more about [data governance in BigQuery](https://docs.cloud.google.com/bigquery/docs/data-governance).
- Learn how to [scan your data for data quality issues](https://docs.cloud.google.com/bigquery/docs/data-quality-scan).
- Learn how to examine table data and create queries with [table explorer](https://docs.cloud.google.com/bigquery/docs/table-explorer).