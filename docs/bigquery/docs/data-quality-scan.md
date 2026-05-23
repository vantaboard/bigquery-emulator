# Scan for data quality issues

This document explains how to use BigQuery and
Knowledge Catalog together to ensure that data meets your quality
expectations. Knowledge Catalog automatic data quality lets you define and
measure the quality of the data in your BigQuery tables. You can
automate the scanning of data, validate data against defined rules, and log
alerts if your data doesn't meet quality requirements.

For more information about automatic data quality, see the
[Auto data quality overview](https://docs.cloud.google.com/dataplex/docs/auto-data-quality-overview).

> [!TIP]
> **Tip:** The steps in this document show how to manage data quality scans across your project. You can also create and manage data quality scans when working with a specific table. For more information, see the [Manage data quality scans for a specific table](https://docs.cloud.google.com/bigquery/docs/data-quality-scan#start-from-table) section of this document.

## Before you begin

1.


   Enable the Dataplex API.


   **Roles required to enable APIs**


   To enable APIs, you need the Service Usage Admin IAM
   role (`roles/serviceusage.serviceUsageAdmin`), which
   contains the `serviceusage.services.enable` permission. [Learn how to grant
   roles](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).

   [Enable the API](https://console.cloud.google.com/flows/enableapi?apiid=dataplex.googleapis.com)
2. Optional: If you want Knowledge Catalog to generate recommendations for data quality rules based on the results of a data profile scan, [create and run the data profile scan](https://docs.cloud.google.com/bigquery/docs/data-profile-scan).

## Required roles


This section describes the IAM roles and permissions needed to
use Knowledge Catalog data quality scans.

### User roles and permissions


To get the permissions that
you need to run and manage data quality scans,

ask your administrator to grant you the
following IAM roles:

- Run a data quality scan on a BigQuery table:
  - [BigQuery Job User](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquery#bigquery.jobUser) (`roles/bigquery.jobUser`) on the project to run scan jobs
  - [BigQuery Data Viewer](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquery#bigquery.dataViewer) (`roles/bigquery.dataViewer`) on the BigQuery table to be scanned
- Publish data quality scan results to Knowledge Catalog:
  - [BigQuery Data Editor](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquery#bigquery.dataEditor) (`roles/bigquery.dataEditor`) on the scanned table
  - [Dataplex Catalog Editor](https://docs.cloud.google.com/iam/docs/roles-permissions/dataplex#dataplex.catalogEditor) (`roles/dataplex.catalogEditor`) on the `@bigquery` entry group in the same location as the table
- Perform specific tasks on `DataScan` resources:
  - [Dataplex DataScan Administrator](https://docs.cloud.google.com/iam/docs/roles-permissions/dataplex#dataplex.dataScanAdmin) (`roles/dataplex.dataScanAdmin`) on the project for full access
  - [Dataplex DataScan Creator](https://docs.cloud.google.com/iam/docs/roles-permissions/dataplex#dataplex.dataScanCreator) (`roles/dataplex.dataScanCreator`) on the project to create scans
  - [Dataplex DataScan Editor](https://docs.cloud.google.com/iam/docs/roles-permissions/dataplex#dataplex.dataScanEditor) (`roles/dataplex.dataScanEditor`) on the project for write access
  - [Dataplex DataScan Viewer](https://docs.cloud.google.com/iam/docs/roles-permissions/dataplex#dataplex.dataScanViewer) (`roles/dataplex.dataScanViewer`) on the project to read scan metadata
  - [Dataplex DataScan DataViewer](https://docs.cloud.google.com/iam/docs/roles-permissions/dataplex#dataplex.dataScanDataViewer) (`roles/dataplex.dataScanDataViewer`) on the project to read scan data including rules and results


For more information about granting roles, see [Manage access to projects, folders, and organizations](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).


These predefined roles contain

the permissions required to run and manage data quality scans. To see the exact permissions that are
required, expand the **Required permissions** section:


#### Required permissions

The following permissions are required to run and manage data quality scans:

- Run a data quality scan on a BigQuery table:
  - `bigquery.jobs.create` on the project to run scan jobs
  - `bigquery.tables.get` on the BigQuery table to be scanned
  - `bigquery.tables.getData` on the BigQuery table to be scanned
- Publish data quality scan results to Knowledge Catalog:
  - `bigquery.tables.update` on the scanned table
  - `dataplex.entryGroups.useDataQualityScorecardAspect` on the `@bigquery` entry group in the same location as the table
- Create a `DataScan`: `dataplex.datascans.create` on the project
- Delete a `DataScan`: `dataplex.datascans.delete` on the project
- View `DataScan` metadata: `dataplex.datascans.get` on the project
- View `DataScan` details including rules and results: `dataplex.datascans.getData` on the project
- List `DataScan`s: `dataplex.datascans.list` on the project
- Run a `DataScan`: `dataplex.datascans.run` on the project
- Update a `DataScan`: `dataplex.datascans.update` on the project
- Get or set IAM policy on a `DataScan`:
  - `dataplex.datascans.getIamPolicy` on the project
  - `dataplex.datascans.setIamPolicy` on the project


You might also be able to get
these permissions
with [custom roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles) or
other [predefined roles](https://docs.cloud.google.com/iam/docs/roles-overview#predefined).

If you need to access columns protected by BigQuery column-level
access policies, then you also need permissions for those columns.

> [!NOTE]
> **Note:** Knowledge Catalog doesn't create a BigQuery job in your project for data quality scans. However, you need the `bigquery.jobs.create` permission to create a `DryRun` job to check for permissions for the table.

### Knowledge Catalog service account roles and permissions

If you haven't created any data quality or data profile scans or you
don't have a Knowledge Catalog lake in this project, create a
service identifier by running:
`gcloud beta services identity create --service=dataplex.googleapis.com`.
This command returns a Knowledge Catalog service identifier if it exists.


To ensure that the Knowledge Catalog service account of the project containing the data quality scan has the necessary
permissions to read data from various sources and export results,

ask your administrator to grant the
following IAM roles to the Knowledge Catalog service account of the project containing the data quality scan:

**Important:** You must grant these roles to the Knowledge Catalog service account of the project containing the data quality scan, *not* to your user account. Failure to grant the roles to the correct principal might result in permission errors.

- Read BigQuery table data: [BigQuery Data Viewer](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquery#bigquery.dataViewer) (`roles/bigquery.dataViewer`) on BigQuery tables to be scanned and any other tables referenced in rules
- Read Iceberg REST Catalog table data: [BigLake Viewer](https://docs.cloud.google.com/iam/docs/roles-permissions/biglake#biglake.viewer) (`roles/biglake.viewer`) on Iceberg REST Catalog tables to be scanned and any other tables referenced in rules
- Export scan results to a BigQuery table: [BigQuery Data Editor](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquery#bigquery.dataEditor) (`roles/bigquery.dataEditor`) on the results dataset and table
- Scan BigQuery data organized in a Knowledge Catalog lake:
  - [Dataplex Metadata Reader](https://docs.cloud.google.com/iam/docs/roles-permissions/dataplex#dataplex.metadataReader) (`roles/dataplex.metadataReader`) on Dataplex resources
  - [Dataplex Viewer](https://docs.cloud.google.com/iam/docs/roles-permissions/dataplex#dataplex.viewer) (`roles/dataplex.viewer`) on Dataplex resources
- Scan a BigQuery external table from Cloud Storage: [Storage Object Viewer](https://docs.cloud.google.com/iam/docs/roles-permissions/storage#storage.objectViewer) (`roles/storage.objectViewer`) on the Cloud Storage bucket


For more information about granting roles, see [Manage access to projects, folders, and organizations](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).


These predefined roles contain

the permissions required to read data from various sources and export results. To see the exact permissions that are
required, expand the **Required permissions** section:


#### Required permissions

The following permissions are required to read data from various sources and export results:

- Read BigQuery table data:
  - `bigquery.tables.get` on BigQuery tables
  - `bigquery.tables.getData` on BigQuery tables
- Export scan results to a BigQuery table:
  - `bigquery.datasets.get` on results dataset and table
  - `bigquery.tables.create` on results dataset and table
  - `bigquery.tables.get` on results dataset and table
  - `bigquery.tables.getData` on results dataset and table
  - `bigquery.tables.update` on results dataset and table
  - `bigquery.tables.updateData` on results dataset and table
- Scan BigQuery data organized in a Knowledge Catalog lake:
  - `dataplex.lakes.list` on Dataplex resources
  - `dataplex.lakes.get` on Dataplex resources
  - `dataplex.zones.list` on Dataplex resources
  - `dataplex.zones.get` on Dataplex resources
  - `dataplex.entities.list` on Dataplex resources
  - `dataplex.entities.get` on Dataplex resources
  - `dataplex.operations.get` on Dataplex resources
- Scan a BigQuery external table from Cloud Storage:
  - `storage.buckets.get` on the Cloud Storage bucket
  - `storage.objects.get` on the Cloud Storage bucket


Your administrator might also be able to give the Knowledge Catalog service account of the project containing the data quality scan
these permissions
with [custom roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles) or
other [predefined roles](https://docs.cloud.google.com/iam/docs/roles-overview#predefined).

If you need to access columns protected by BigQuery column-level
access policies, then assign the Knowledge Catalog service account
permissions for those columns.

If a table has BigQuery row-level access policies enabled, then you
can only scan rows visible to the Knowledge Catalog service account. Note
that the individual user's access privileges are not evaluated for row-level
policies.

<br />

## Create a data quality scan


### Console

1.


   In the Google Cloud console, on the BigQuery
   **Metadata curation** page, go to the **Data profiling \& quality** tab.


   [Go to Data profiling \& quality](https://console.cloud.google.com/bigquery/governance/metadata-curation/data-profiling-and-quality)

   <br />

2. Click **Create data quality scan**.

3. In the **Define scan** window, fill in the following fields:

   1. Optional: Enter a **Display name**.

   2. Enter an **ID** . See the
      [resource naming conventions](https://docs.cloud.google.com/compute/docs/naming-resources#resource-name-format).

   3. Optional: Enter a **Description**.

   4. In the **Table** field, click **Browse** . Choose the table to scan, and
      then click **Select**. Only standard BigQuery and
      Iceberg REST Catalog tables are supported.

      For tables in multi-region datasets, choose a region where to create
      the data scan.

      To browse the tables organized within Knowledge Catalog lakes,
      click **Browse within Knowledge Catalog Lakes**.
   5. In the **Scope** field, choose **Incremental** or **Entire data**.

      - If you choose **Incremental** : In the **Timestamp column** field, select a column of type `DATE` or `TIMESTAMP` from your BigQuery table that increases as new records are added, and that can be used to identify new records. It can be a column that partitions the table.
   6. To filter your data, select the **Filter rows** checkbox. Provide a
      row filter consisting of a valid SQL expression that can be used as a part of a
      [`WHERE` clause in GoogleSQL syntax](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#where_clause).
      For example, `col1 >= 0`.
      The filter can be a combination of multiple column conditions. For
      example, `col1 >= 0 AND col2 < 10`.

   7. To sample your data, in the **Sampling size** list, select a
      sampling percentage. Choose a percentage value that ranges between
      0.0% and 100.0% with up to 3 decimal digits. For larger
      datasets, choose a lower sampling percentage. For example, for a
      1 PB table, if you enter a value between 0.1% and 1.0%,
      the data quality scan samples between 1-10 TB of data. For
      incremental data scans, the data quality scan applies sampling to the
      latest increment.

   8. To publish the data quality scan results as Knowledge Catalog
      metadata, select the
      **Publish results to Knowledge Catalog** checkbox.

      You can view the latest scan results on the **Data quality** tab in the
      BigQuery and Knowledge Catalog pages for the source
      table. To enable users to access the published scan results, see the

      [Grant access to data quality scan results](https://docs.cloud.google.com/bigquery/docs/data-quality-scan#share-results) section
      of this document.
   9. In the **Schedule** section, choose one of the following options:

      - **Repeat** : Run the data quality scan on a schedule: hourly, daily,
        weekly, monthly, or custom. Specify how often the scan runs and
        at what time. If you choose custom, use [cron](https://en.wikipedia.org/wiki/Cron)
        format to specify the schedule.

      - **On-demand**: Run the data quality scan on demand.

      - **One-time run** : Run the data quality scan once now, and remove the
        scan after the auto-deletion time. This feature is in [preview](https://cloud.google.com/products/#product-launch-stages).

        - **Set post-scan results auto-deletion**: The auto-deletion time is the time span between when the scan is executed and when the scan is deleted. A data quality scan without a specified auto-deletion time is automatically deleted 24 hours after its execution. The auto-deletion time can range from 0 seconds (immediate deletion) to 365 days.
   10. Click **Continue**.

4. In the **Data quality rules** window, define the rules to
   configure for this data quality scan.

   1. Click **Add rules**, and then choose from the following options.

      - **Profile based recommendations**: Build rules from the
        recommendations based on an existing data profiling scan.

        1. **Choose columns**: Select the columns to get recommended rules for.

        2. **Choose scan project**: If the data profiling scan is in a
           different project than the project where you are creating
           the data quality scan, then select the project to pull profile
           scans from.

        3. **Choose profile results** : Select one or more profile results and
           then click **OK**. This populates a list of suggested rules that
           you can use as a starting point.

        4. Select the checkbox for the rules that you want to add, and then
           click **Select**. Once selected, the rules are added to your
           current rule list. Then, you can edit the rules.

      - **Built-in rule types** : Build rules from predefined rules.
        See the list of [predefined rules](https://docs.cloud.google.com/dataplex/docs/auto-data-quality-overview#predefined-rules).

        1. **Choose columns**: Select the columns to select rules for.

        2. **Choose rule types** : Select the rule types that you want to
           choose from, and then click **OK**. The rule types that appear
           depend on the columns that you selected.

        3. Select the checkbox for the rules that you want to add, and then
           click **Select**. Once selected, the rules are added to your
           current rules list. Then, you can edit the rules.

      - **SQL row check rule**: Create a custom SQL rule to apply to each row.

        1. In **Dimension**, choose one dimension.

        2. In **Passing threshold**, choose a percentage of records that must
           pass the check.

        3. In **Column name**, choose a column.

        4. In the **Provide a SQL expression** field, enter a SQL expression
           that evaluates to a boolean `true` (pass) or `false` (fail). For
           more information, see
           [Supported custom SQL rule types](https://docs.cloud.google.com/dataplex/docs/auto-data-quality-overview#supported-custom-sql-rule-types)
           and the examples in
           [Define data quality rules](https://docs.cloud.google.com/dataplex/docs/use-auto-data-quality#sample-rules).

        5. Click **Add**.

      - **SQL aggregate check rule**: Create a custom SQL
        table condition rule.

        1. In **Dimension**, choose one dimension.

        2. In **Column name**, choose a column.

        3. In the **Provide a SQL expression** field, enter a SQL expression
           that evaluates to a boolean `true` (pass) or `false` (fail). For
           more information, see
           [Supported custom SQL rule types](https://docs.cloud.google.com/dataplex/docs/auto-data-quality-overview#supported-custom-sql-rule-types)
           and the examples in
           [Define data quality rules](https://docs.cloud.google.com/dataplex/docs/use-auto-data-quality#sample-rules).

        4. Click **Add**.

      - **SQL assertion rule**: Create a custom SQL assertion rule to check
        for an invalid state of the data.

        1. In **Dimension**, choose one dimension.

        2. Optional: In **Column name**, choose a column.

        3. In the **Provide a SQL statement** field, enter a SQL statement
           that returns rows that match the invalid state. If any rows are
           returned, this rule fails. Omit the trailing semicolon from the SQL
           statement. For more information, see
           [Supported custom SQL rule types](https://docs.cloud.google.com/dataplex/docs/auto-data-quality-overview#supported-custom-sql-rule-types)
           and the examples in
           [Define data quality rules](https://docs.cloud.google.com/dataplex/docs/use-auto-data-quality#sample-rules).

        4. Click **Add**.

   2. Optional: For any data quality rule, you can assign a custom rule name
      to use for monitoring and alerting, and a description. To do this,
      edit a rule and specify the following details:

      - **Rule name**: Enter a custom rule name with up to 63 characters. The rule name can include letters (a-z, A-Z), digits (0-9), and hyphens (-) and must start with a letter and end with a number or a letter.
      - **Description**: Enter a rule description with a maximum length of 1,024 characters.
   3. Repeat the previous steps to add additional rules to the data quality
      scan. When finished, click **Continue**.

5. Optional: Export the scan results to a BigQuery standard
   table. In the **Export scan results to BigQuery table** section, do the
   following:

   1. In the **Select BigQuery dataset** field, click **Browse**. Select a
      BigQuery dataset to store the data quality scan results.

   2. In the **BigQuery table** field, specify the table to store the data
      quality scan results. If you're using an existing table, make sure
      that it is compatible with the
      [export table schema](https://docs.cloud.google.com/dataplex/docs/use-auto-data-quality#table-schema).
      If the specified table doesn't exist, Knowledge Catalog creates
      it for you.

      > [!NOTE]
      > **Note:** You can use the same results table for multiple data quality scans.

6. Optional: Add labels. Labels are key-value pairs that let you group
   related objects together or with other Google Cloud resources.

7. Optional: Set up email notification reports to alert people about the
   status and results of a data quality scan job. In the **Notification report**
   section, click **Add email ID** and
   enter up to five email addresses. Then, select the scenarios that you want
   to send reports for:

   - **Quality score (\<=)**: sends a report when a job succeeds with a data quality score that is lower than the specified target score. Enter a target quality score between 0 and 100.
   - **Job failures**: sends a report when the job itself fails, regardless of the data quality results.
   - **Job completion (success or failure)**: sends a report when the job ends, regardless of the data quality results.
8. Click **Create**.

   After the scan is created, you can run it at any time by clicking
   **Run now**.

### gcloud

To create a data quality scan, use the
[`gcloud dataplex datascans create data-quality` command](https://docs.cloud.google.com/sdk/gcloud/reference/dataplex/datascans/create/data-quality).

If the source data is organized in a Knowledge Catalog lake, include the
`--data-source-entity` flag:

    gcloud dataplex datascans create data-quality DATASCAN \
        --location=LOCATION \
        --data-quality-spec-file=DATA_QUALITY_SPEC_FILE \
        --data-source-entity=DATA_SOURCE_ENTITY

If the source data isn't organized in a Knowledge Catalog lake, include
the `--data-source-resource` flag:

    gcloud dataplex datascans create data-quality DATASCAN \
        --location=LOCATION \
        --data-quality-spec-file=DATA_QUALITY_SPEC_FILE \
        --data-source-resource=DATA_SOURCE_RESOURCE

Replace the following variables:

- `DATASCAN`: The name of the data quality scan.
- `LOCATION`: The Google Cloud region in which to create the data quality scan.
- `DATA_QUALITY_SPEC_FILE`: The path to the JSON or YAML file containing the specifications for the data quality scan. The file can be a local file or a Cloud Storage path with the prefix `gs://`. Use this file to specify the data quality rules for the scan. You can also specify additional details in this file, such as filters, sampling percent, and post-scan actions like exporting to BigQuery or sending email notification reports. See the [documentation for JSON representation](https://docs.cloud.google.com/dataplex/docs/reference/rest/v1/DataQualitySpec) and the [example YAML representation](https://docs.cloud.google.com/dataplex/docs/use-auto-data-quality#create-scan-using-gcloud).
- `DATA_SOURCE_ENTITY`: The Knowledge Catalog entity that contains the data for the data quality scan. For example, `projects/test-project/locations/test-location/lakes/test-lake/zones/test-zone/entities/test-entity`.
- `DATA_SOURCE_RESOURCE`: The name of the resource that contains the data for the data quality scan. For example, `//bigquery.googleapis.com/projects/test-project/datasets/test-dataset/tables/test-table`.

### C#


### C#


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

### Node.js

### Node.js


To authenticate to BigQuery, set up Application Default Credentials.
For more information, see

[Set up authentication for a local development environment](https://docs.cloud.google.com/docs/authentication/set-up-adc-local-dev-environment).

    // Copyright 2026 Google LLC
    //
    // Licensed under the Apache License, Version 2.0 (the "License");
    // you may not use this file except in compliance with the License.
    // You may obtain a copy of the License at
    //
    //     https://www.apache.org/licenses/LICENSE-2.0
    //
    // Unless required by applicable law or agreed to in writing, software
    // distributed under the License is distributed on an "AS IS" BASIS,
    // WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    // See the License for the specific language governing permissions and
    // limitations under the License.
    //
    // ** This file is automatically generated by gapic-generator-typescript. **
    // ** https://github.com/googleapis/gapic-generator-typescript **
    // ** All changes to this file may be overwritten. **



    'use strict';

    function main(parent, dataScan, dataScanId) {
      /**
       * This snippet has been automatically generated and should be regarded as a code template only.
       * It will require modifications to work.
       * It may require correct/in-range values for request initialization.
       * TODO(developer): Uncomment these variables before running the sample.
       */
      /**
       *  Required. The resource name of the parent location:
       *  `projects/{project}/locations/{location_id}`
       *  where `project` refers to a *project_id* or *project_number* and
       *  `location_id` refers to a Google Cloud region.
       */
      // const parent = 'abc123'
      /**
       *  Required. DataScan resource.
       */
      // const dataScan = {}
      /**
       *  Required. DataScan identifier.
       *  * Must contain only lowercase letters, numbers and hyphens.
       *  * Must start with a letter.
       *  * Must end with a number or a letter.
       *  * Must be between 1-63 characters.
       *  * Must be unique within the customer project / location.
       */
      // const dataScanId = 'abc123'
      /**
       *  Optional. Only validate the request, but do not perform mutations.
       *  The default is `false`.
       */
      // const validateOnly = true

      // Imports the Dataplex library
      const {DataScanServiceClient} = require('https://docs.cloud.google.com/nodejs/docs/reference/dataplex/latest/overview.html').v1;

      // Instantiates a client
      const dataplexClient = new https://docs.cloud.google.com/nodejs/docs/reference/dataplex/latest/overview.html();

      async function callCreateDataScan() {
        // Construct request
        const request = {
          parent,
          dataScan,
          dataScanId,
        };

        // Run request
        const [operation] = await dataplexClient.createDataScan(request);
        const [response] = await operation.promise();
        console.log(response);
      }

      callCreateDataScan();
    }

    process.on('unhandledRejection', err => {
      console.error(err.message);
      process.exitCode = 1;
    });
    main(...process.argv.slice(2));

### Python

### Python


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

To create a data quality scan, use the
[`dataScans.create` method](https://docs.cloud.google.com/dataplex/docs/reference/rest/v1/projects.locations.dataScans/create).

The following request creates a one-time data quality scan:

```json
POST https://dataplex.googleapis.com/v1/projects/PROJECT_ID/locations/LOCATION/dataScans?data_scan_id=DATASCAN_ID

{
"data": {
  "resource": "//bigquery.googleapis.com/projects/PROJECT_ID/datasets/DATASET_ID/tables/TABLE_ID"
},
"type": "DATA_QUALITY",
"executionSpec": {
  "trigger": {
    "oneTime": {
      "ttl_after_scan_completion": "120s"
    }
  }
},
"dataQualitySpec": {
  "rules": [
    {
      "nonNullExpectation": {},
      "column": "COLUMN_NAME",
      "dimension": "DIMENSION",
      "threshold": 1
    }
  ],
  "filter": "FILTER_CONDITION"
}
}
```

Replace the following:

- `PROJECT_ID`: Your project ID.
- `LOCATION`: The region where to create the data quality scan.
- `DATASCAN_ID`: The ID of the data quality scan.
- `DATASET_ID`: The ID of BigQuery dataset.
- `TABLE_ID`: The ID of BigQuery table.
- `COLUMN_NAME`: The column name for the rule.
- `DIMENSION`: The dimension for the rule, for example `VALIDITY`.
- `FILTER_CONDITION`: An optional [AIP-160 filter string](https://docs.cloud.google.com/dataplex/docs/auto-data-quality-overview#rule-filtering) to selectively run rules (for example, `name = \"RULE_NAME\"`).

If you want to build rules for the data quality scan by using rule
recommendations that are based on the results of a data profiling scan, get
the recommendations by calling the
[`dataScans.jobs.generateDataQualityRules` method](https://docs.cloud.google.com/dataplex/docs/reference/rest/v1/projects.locations.dataScans.jobs/generateDataQualityRules)
on the data profiling scan.

<br />


> [!NOTE]
> **Note:** If your BigQuery table is configured with the **Require partition filter** set to `true`, use the BigQuery partition column as the data quality scan row filter or timestamp column.

<br />

## Run a data quality scan


### Console

1.


   In the Google Cloud console, on the BigQuery
   **Metadata curation** page, go to the **Data profiling \& quality** tab.


   [Go to Data profiling \& quality](https://console.cloud.google.com/bigquery/governance/metadata-curation/data-profiling-and-quality)

   <br />

2. Click the data quality scan to run.

3. Click **Run now**.

### gcloud

To run a data quality scan, use the
[`gcloud dataplex datascans run` command](https://docs.cloud.google.com/sdk/gcloud/reference/dataplex/datascans/run):

```
gcloud dataplex datascans run DATASCAN \
--location=LOCATION \
```

Replace the following variables:

- `LOCATION`: The Google Cloud region in which the data quality scan was created.
- `DATASCAN`: The name of the data quality scan.

### C#

### C#


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

To run a data quality scan, use the
[`dataScans.run` method](https://docs.cloud.google.com/dataplex/docs/reference/rest/v1/projects.locations.dataScans/run).

> [!NOTE]
> **Note:** Run isn't supported for data quality scans that are on a one-time schedule.

<br />

## View data quality scan results


### Console

1.


   In the Google Cloud console, on the BigQuery
   **Metadata curation** page, go to the **Data profiling \& quality** tab.


   [Go to Data profiling \& quality](https://console.cloud.google.com/bigquery/governance/metadata-curation/data-profiling-and-quality)

   <br />

2. Click the name of a data quality scan.

   - The **Overview** section displays information about the most recent
     jobs, including when the scan was run, the number of records
     scanned in each job, whether all the data quality checks passed, and
     if there were failures, the number of data quality checks that failed.

   - The **Data quality scan configuration** section displays details about the
     scan.

3. To see detailed information about a job, such as data quality scores that
   indicate the percentage of rules that passed, which rules failed, and the
   job logs, click the **Jobs history** tab. Then, click a job ID.

> [!NOTE]
> **Note:** If you exported the scan results to a BigQuery table, then you can also access the scan results from the table. The data quality scores are available if you published the scan results as Knowledge Catalog metadata.

### gcloud

To view the results of a data quality scan job, use the
[`gcloud dataplex datascans jobs describe` command](https://docs.cloud.google.com/sdk/gcloud/reference/dataplex/datascans/jobs/describe):

```
gcloud dataplex datascans jobs describe JOB \
--location=LOCATION \
--datascan=DATASCAN \
--view=FULL
```

Replace the following variables:

- `JOB`: The job ID of the data quality scan job.
- `LOCATION`: The Google Cloud region in which the data quality scan was created.
- `DATASCAN`: The name of the data quality scan the job belongs to.
- `--view=FULL`: To see the scan job result, specify `FULL`.

### C#

### C#


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

To view the results of a data quality scan, use the
[`dataScans.get` method](https://docs.cloud.google.com/dataplex/docs/reference/rest/v1/projects.locations.dataScans/get).

<br />

### View published results


If the data quality scan results are published as Knowledge Catalog
metadata, then you can see the latest scan results
on the BigQuery and Knowledge Catalog pages in the
Google Cloud console, on the source table's **Data quality** tab.

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)

   If you don't see the left pane, click **Expand left pane** to open the pane.
3. In the **Explorer** pane, click **Datasets**, and then click your dataset.

4. Click **Overview \> Tables**, and then select the table whose data quality scan
   results you want to see.

5. Click the **Data quality** tab.

   The latest published results are displayed.

   > [!NOTE]
   > **Note:** Published results might not be available if a scan is running for the first time.

### View historical scan results


Knowledge Catalog saves the data quality scan history of the last 300
jobs or for the past year, whichever occurs first.

### Console

1.


   In the Google Cloud console, on the BigQuery
   **Metadata curation** page, go to the **Data profiling \& quality** tab.


   [Go to Data profiling \& quality](https://console.cloud.google.com/bigquery/governance/metadata-curation/data-profiling-and-quality)

   <br />

2. Click the name of a data quality scan.

3. Click the **Jobs history** tab.

   The **Jobs history** tab provides information about past jobs, such as
   the number of records scanned in each job, the job status, the time
   the job was run, and whether each rule passed or failed.
4. To view detailed information about a job, click any of the jobs in the
   **Job ID** column.

### gcloud

To view historical data quality scan jobs, use the
[`gcloud dataplex datascans jobs list` command](https://docs.cloud.google.com/sdk/gcloud/reference/dataplex/datascans/jobs/list):

```
gcloud dataplex datascans jobs list \
--location=LOCATION \
--datascan=DATASCAN \
```

Replace the following variables:

- `LOCATION`: The Google Cloud region in which the data quality scan was created.
- `DATASCAN`: The name of the data quality scan to view historical jobs for.

### C#

### C#


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

To view historical data quality scan jobs, use the
[`dataScans.jobs.list` method](https://docs.cloud.google.com/dataplex/docs/reference/rest/v1/projects.locations.dataScans.jobs/list).

<br />

## Grant access to data quality scan results


To enable the users in your organization to view the scan results, do the following:

1.


   In the Google Cloud console, on the BigQuery
   **Metadata curation** page, go to the **Data profiling \& quality** tab.


   [Go to Data profiling \& quality](https://console.cloud.google.com/bigquery/governance/metadata-curation/data-profiling-and-quality)

   <br />

2. Click the data quality scan you want to share the results of.

3. Click the **Permissions** tab.

4. Do the following:

   - To grant access to a principal, click **Grant access** . Grant the **Dataplex DataScan DataViewer** role to the associated principal.
   - To remove access from a principal, select the principal that you want to remove the **Dataplex DataScan DataViewer** role from. Click **Remove access**, and then confirm when prompted.

<br />

## Troubleshoot a data quality failure

You can set alerts for data quality failures using the logs in Cloud Logging.
For more information, including sample queries, see
[Set alerts in Cloud Logging](https://docs.cloud.google.com/dataplex/docs/use-auto-data-quality#set-alerts).


For each job with row-level rules that fail, Knowledge Catalog provides
a query to get the failed records. Run this query to see the records that did
not match your rule.

> [!NOTE]
> **Note:** The query returns all of the columns of the table, not just the failed column.

### Console

1.


   In the Google Cloud console, on the BigQuery
   **Metadata curation** page, go to the **Data profiling \& quality** tab.


   [Go to Data profiling \& quality](https://console.cloud.google.com/bigquery/governance/metadata-curation/data-profiling-and-quality)

   <br />

2. Click the name of the data quality scan whose records you want to troubleshoot.

3. Click the **Jobs history** tab.

4. Click the job ID of the job that identified data quality failures.

5. In the job results window that opens, in the **Rules** section, find the column
   **Query to get failed records** . Click **Copy query to clipboard** for the
   failed rule.

6. [Run the query in BigQuery](https://docs.cloud.google.com/bigquery/docs/running-queries)
   to see the records that caused the job to fail.

### gcloud

Not supported.

### REST

1. To get the job that identified the data quality failures, use the
   [`dataScans.get` method](https://docs.cloud.google.com/dataplex/docs/reference/rest/v1/DataQualityResult).

   In the response object, the `failingRowsQuery` field shows the query.
2. [Run the query in BigQuery](https://docs.cloud.google.com/bigquery/docs/running-queries)
   to see the records that caused the job to fail.

Knowledge Catalog also runs the debug query, provided it was included
during the rule creation. The debug query results are included in each rule's
output. This feature is in [Preview](https://docs.cloud.google.com/products#product-launch-stages).

### Console

Not supported.

### gcloud

Not supported.

### REST

To get the job that identified the data quality failures, use the
[`dataScans.get` method](https://docs.cloud.google.com/dataplex/docs/reference/rest/v1/DataQualityResult).
In the response object, the `debugQueriesResultSets` field shows the
results of the debug queries.

<br />

## Manage data quality scans for a specific table

The steps in this document show how to manage data quality scans across your
project by using the BigQuery
**Metadata curation \> Data profiling \& quality** page in the
Google Cloud console.

You can also create and manage data quality scans when working with a
specific table. In the Google Cloud console, on the BigQuery
page for the table, use the **Data quality** tab. Do the following:


1.

   In the Google Cloud console, go to the **BigQuery** page.


   [Go to BigQuery](https://console.cloud.google.com/bigquery)

   <br />

   In the **Explorer** pane (in the left pane), click **Datasets** , and then click your dataset.
   Click **Overview \> Tables**, and then select the table whose data quality scan
   results you want to see.

2. Click the **Data quality** tab.

3. Depending on whether the table has a data quality scan whose results are
   published as Knowledge Catalog metadata, you can work with the table's
   data quality scans in the following ways:

   - **Data quality scan results are published**: the latest scan results are
     displayed on the page.

     To manage the data quality scans for this table, click **Data quality
     scan**, and then select from the following options:
     - **Create new scan** : create a new data quality scan. For more
       information, see the [Create a data quality scan](https://docs.cloud.google.com/bigquery/docs/data-quality-scan#create-scan) section
       of this document. When you create a scan from a table's details page, the
       table is preselected.

     - **Run now**: run the scan.

     - **Edit scan configuration**: edit settings including the display name,
       filters, and schedule.

       To edit the data quality rules, on the **Data quality** tab, click the
       **Rules** tab. Click **Modify rules** . Update the rules and then click
       **Save**.
     - **Manage scan permissions** : control who can access the scan results.
       For more information, see the
       [Grant access to data quality scan results](https://docs.cloud.google.com/bigquery/docs/data-quality-scan#share-results)
       section of this document.

     - **View historical results** : view detailed information about previous
       data quality scan jobs. For more information, see the
       [View data quality scan results](https://docs.cloud.google.com/bigquery/docs/data-quality-scan#results) and
       [View historical scan results](https://docs.cloud.google.com/bigquery/docs/data-quality-scan#older-scans) sections of
       this document.

     - **View all scans**: view a list of data quality scans that apply to this
       table.

   - **Data quality scan results aren't published**: select from the
     following options:

     - **Create data quality scan** : create a new data quality scan. For more
       information, see the [Create a data quality scan](https://docs.cloud.google.com/bigquery/docs/data-quality-scan#create-scan) section
       of this document. When you create a scan from a table's details page, the
       table is preselected.

     - **View existing scans**: view a list of data quality scans that apply to
       this table.

<br />

## View the data quality scans for a table

To view the data quality scans that apply to a specific table, do the following:

1. In the Google Cloud console, on the BigQuery
   **Metadata curation** page, go to the **Data profiling \& quality** tab.


   [Go to Data profiling \& quality](https://console.cloud.google.com/bigquery/governance/metadata-curation/data-profiling-and-quality)

   <br />

2. Filter the list by table name and scan type.

## Update a data quality scan


You can edit various settings for an existing data quality scan, such as the
display name, filters, schedule, and data quality rules.

> [!NOTE]
> **Note:** If an existing data quality scan publishes the results to the BigQuery and Knowledge Catalog pages in the Google Cloud console, and you instead want to publish future scan results as Knowledge Catalog metadata, you must edit the scan and re-enable publishing. You might need additional permissions to enable catalog publishing.

### Console

1.


   In the Google Cloud console, on the BigQuery
   **Metadata curation** page, go to the **Data profiling \& quality** tab.


   [Go to Data profiling \& quality](https://console.cloud.google.com/bigquery/governance/metadata-curation/data-profiling-and-quality)

   <br />

2. Click the name of a data quality scan.

3. To edit settings including the display name, filters, and schedule, click
   **Edit** . Edit the values and then click **Save**.

4. To edit the data quality rules, on the scan details page, click the
   **Current rules** tab. Click **Modify rules** . Update the rules and
   then click **Save**.

### gcloud

To update the description of a data quality scan, use the
[`gcloud dataplex datascans update data-quality` command](https://docs.cloud.google.com/sdk/gcloud/reference/dataplex/datascans/update/data-quality):

```
gcloud dataplex datascans update data-quality DATASCAN \
--location=LOCATION \
--description=DESCRIPTION
```

Replace the following:

- `DATASCAN`: The name of the data quality scan to update.
- `LOCATION`: The Google Cloud region in which the data quality scan was created.
- `DESCRIPTION`: The new description for the data quality scan.

> [!NOTE]
> **Note:** You can update specification fields, such as `rules`, `rowFilter`, or `samplingPercent`, in the data quality specification file. Refer to [JSON](https://docs.cloud.google.com/dataplex/docs/reference/rest/v1/DataQualitySpec) and [YAML](https://docs.cloud.google.com/dataplex/docs/use-auto-data-quality#create-scan-using-gcloud) representations.

### C#

### C#


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

To edit a data quality scan, use the
[`dataScans.patch` method](https://docs.cloud.google.com/dataplex/docs/reference/rest/v1/projects.locations.dataScans/patch).

> [!NOTE]
> **Note:** Update isn't supported for data quality scans that are on a one-time schedule.

<br />

## Delete a data quality scan


### Console

### Console

1.


   In the Google Cloud console, on the BigQuery
   **Metadata curation** page, go to the **Data profiling \& quality** tab.


   [Go to Data profiling \& quality](https://console.cloud.google.com/bigquery/governance/metadata-curation/data-profiling-and-quality)

   <br />

2. Click the scan you want to delete.

3. Click **Delete**, and then confirm when prompted.

### gcloud

### gcloud

To delete a data quality scan, use the
[`gcloud dataplex datascans delete` command](https://docs.cloud.google.com/sdk/gcloud/reference/dataplex/datascans/delete):

```
gcloud dataplex datascans delete DATASCAN \
--location=LOCATION \
--async
```

Replace the following variables:

- `DATASCAN`: The name of the data quality scan to delete.
- `LOCATION`: The Google Cloud region in which the data quality scan was created.

### C#

### C#


To authenticate to BigQuery, set up Application Default Credentials.
For more information, see

[Set up authentication for a local development environment](https://docs.cloud.google.com/docs/authentication/set-up-adc-local-dev-environment).

    using https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.Dataplex.V1/latest/Google.Cloud.Dataplex.V1.html;
    using https://docs.cloud.google.com/dotnet/docs/reference/Google.LongRunning/latest/Google.LongRunning.html;
    using https://docs.cloud.google.com/dotnet/docs/reference/Google.Protobuf/latest/Google.Protobuf.WellKnownTypes.html;

    public sealed partial class GeneratedDataScanServiceClientSnippets
    {
        /// <summary>Snippet for DeleteDataScan</summary>
        /// <remarks>
        /// This snippet has been automatically generated and should be regarded as a code template only.
        /// It will require modifications to work:
        /// - It may require correct/in-range values for request initialization.
        /// - It may require specifying regional endpoints when creating the service client as shown in
        ///   https://cloud.google.com/dotnet/docs/reference/help/client-configuration#endpoint.
        /// </remarks>
        public void DeleteDataScanRequestObject()
        {
            // Create client
            https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.Dataplex.V1/latest/Google.Cloud.Dataplex.V1.DataScanServiceClient.html dataScanServiceClient = https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.Dataplex.V1/latest/Google.Cloud.Dataplex.V1.DataScanServiceClient.html.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.Dataplex.V1/latest/Google.Cloud.Dataplex.V1.DataScanServiceClient.html#Google_Cloud_Dataplex_V1_DataScanServiceClient_Create();
            // Initialize request argument(s)
            https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.Dataplex.V1/latest/Google.Cloud.Dataplex.V1.DeleteDataScanRequest.html request = new https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.Dataplex.V1/latest/Google.Cloud.Dataplex.V1.DeleteDataScanRequest.html
            {
                DataScanName = https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.Dataplex.V1/latest/Google.Cloud.Dataplex.V1.DataScanName.html.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.Dataplex.V1/latest/Google.Cloud.Dataplex.V1.DataScanName.html#Google_Cloud_Dataplex_V1_DataScanName_FromProjectLocationDataScan_System_String_System_String_System_String_("[PROJECT]", "[LOCATION]", "[DATASCAN]"),
                Force = false,
            };
            // Make the request
            Operation<Empty, OperationMetadata> response = dataScanServiceClient.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.Dataplex.V1/latest/Google.Cloud.Dataplex.V1.DataScanServiceClient.html#Google_Cloud_Dataplex_V1_DataScanServiceClient_DeleteDataScan_Google_Cloud_Dataplex_V1_DataScanName_Google_Api_Gax_Grpc_CallSettings_(request);

            // Poll until the returned long-running operation is complete
            Operation<Empty, OperationMetadata> completedResponse = response.PollUntilCompleted();
            // Retrieve the operation result
            https://docs.cloud.google.com/dotnet/docs/reference/Google.Protobuf/latest/Google.Protobuf.WellKnownTypes.Empty.html result = completedResponse.Result;

            // Or get the name of the operation
            string operationName = response.Name;
            // This name can be stored, then the long-running operation retrieved later by name
            Operation<Empty, OperationMetadata> retrievedResponse = dataScanServiceClient.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.Dataplex.V1/latest/Google.Cloud.Dataplex.V1.DataScanServiceClient.html#Google_Cloud_Dataplex_V1_DataScanServiceClient_PollOnceDeleteDataScan_System_String_Google_Api_Gax_Grpc_CallSettings_(operationName);
            // Check if the retrieved long-running operation has completed
            if (retrievedResponse.IsCompleted)
            {
                // If it has completed, then access the result
                https://docs.cloud.google.com/dotnet/docs/reference/Google.Protobuf/latest/Google.Protobuf.WellKnownTypes.Empty.html retrievedResult = retrievedResponse.Result;
            }
        }
    }

### Go

### Go


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

    	req := &dataplexpb.DeleteDataScanRequest{
    		// TODO: Fill request struct fields.
    		// See https://pkg.go.dev/cloud.google.com/go/dataplex/apiv1/dataplexpb#DeleteDataScanRequest.
    	}
    	op, err := c.DeleteDataScan(ctx, req)
    	if err != nil {
    		// TODO: Handle error.
    	}

    	err = op.Wait(ctx)
    	if err != nil {
    		// TODO: Handle error.
    	}
    }

### Java

### Java


To authenticate to BigQuery, set up Application Default Credentials.
For more information, see

[Set up authentication for a local development environment](https://docs.cloud.google.com/docs/authentication/set-up-adc-local-dev-environment).

    import com.google.cloud.dataplex.v1.https://docs.cloud.google.com/java/docs/reference/google-cloud-dataplex/latest/com.google.cloud.dataplex.v1.DataScanName.html;
    import com.google.cloud.dataplex.v1.https://docs.cloud.google.com/java/docs/reference/google-cloud-dataplex/latest/com.google.cloud.dataplex.v1.DataScanServiceClient.html;
    import com.google.cloud.dataplex.v1.https://docs.cloud.google.com/java/docs/reference/google-cloud-dataplex/latest/com.google.cloud.dataplex.v1.DeleteDataScanRequest.html;
    import com.google.protobuf.https://docs.cloud.google.com/java/docs/reference/protobuf/latest/com.google.protobuf.Empty.html;

    public class SyncDeleteDataScan {

      public static void main(String[] args) throws Exception {
        syncDeleteDataScan();
      }

      public static void syncDeleteDataScan() throws Exception {
        // This snippet has been automatically generated and should be regarded as a code template only.
        // It will require modifications to work:
        // - It may require correct/in-range values for request initialization.
        // - It may require specifying regional endpoints when creating the service client as shown in
        // https://cloud.google.com/java/docs/setup#configure_endpoints_for_the_client_library
        try (https://docs.cloud.google.com/java/docs/reference/google-cloud-dataplex/latest/com.google.cloud.dataplex.v1.DataScanServiceClient.html dataScanServiceClient = https://docs.cloud.google.com/java/docs/reference/google-cloud-dataplex/latest/com.google.cloud.dataplex.v1.DataScanServiceClient.html.create()) {
          https://docs.cloud.google.com/java/docs/reference/google-cloud-dataplex/latest/com.google.cloud.dataplex.v1.DeleteDataScanRequest.html request =
              https://docs.cloud.google.com/java/docs/reference/google-cloud-dataplex/latest/com.google.cloud.dataplex.v1.DeleteDataScanRequest.html.newBuilder()
                  .setName(https://docs.cloud.google.com/java/docs/reference/google-cloud-dataplex/latest/com.google.cloud.dataplex.v1.DataScanName.html.of("[PROJECT]", "[LOCATION]", "[DATASCAN]").toString())
                  .https://docs.cloud.google.com/java/docs/reference/google-cloud-dataplex/latest/com.google.cloud.dataplex.v1.DeleteDataScanRequest.Builder.html#com_google_cloud_dataplex_v1_DeleteDataScanRequest_Builder_setForce_boolean_(true)
                  .build();
          dataScanServiceClient.https://docs.cloud.google.com/java/docs/reference/google-cloud-dataplex/latest/com.google.cloud.dataplex.v1.DataScanServiceClient.html#com_google_cloud_dataplex_v1_DataScanServiceClient_deleteDataScanAsync_com_google_cloud_dataplex_v1_DataScanName_(request).get();
        }
      }
    }

### Python

### Python


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


    def sample_delete_data_scan():
        # Create a client
        client = https://docs.cloud.google.com/python/docs/reference/dataplex/latest.https://docs.cloud.google.com/python/docs/reference/dataplex/latest/google.cloud.dataplex_v1.services.data_scan_service.DataScanServiceClient.html()

        # Initialize request argument(s)
        request = https://docs.cloud.google.com/python/docs/reference/dataplex/latest.https://docs.cloud.google.com/python/docs/reference/dataplex/latest/google.cloud.dataplex_v1.types.DeleteDataScanRequest.html(
            name="name_value",
        )

        # Make the request
        operation = client.https://docs.cloud.google.com/python/docs/reference/dataplex/latest/google.cloud.dataplex_v1.services.data_scan_service.DataScanServiceClient.html#google_cloud_dataplex_v1_services_data_scan_service_DataScanServiceClient_delete_data_scan(request=request)

        print("Waiting for operation to complete...")

        response = operation.result()

        # Handle the response
        print(response)

### Ruby

### Ruby


To authenticate to BigQuery, set up Application Default Credentials.
For more information, see

[Set up authentication for a local development environment](https://docs.cloud.google.com/docs/authentication/set-up-adc-local-dev-environment).

    require "google/cloud/dataplex/v1"

    ##
    # Snippet for the delete_data_scan call in the DataScanService service
    #
    # This snippet has been automatically generated and should be regarded as a code
    # template only. It will require modifications to work:
    # - It may require correct/in-range values for request initialization.
    # - It may require specifying regional endpoints when creating the service
    # client as shown in https://cloud.google.com/ruby/docs/reference.
    #
    # This is an auto-generated example demonstrating basic usage of
    # Google::Cloud::Dataplex::V1::DataScanService::Client#delete_data_scan.
    #
    def delete_data_scan
      # Create a client object. The client can be reused for multiple calls.
      client = Google::Cloud::Dataplex::V1::DataScanService::Client.new

      # Create a request. To set request fields, pass in keyword arguments.
      request = Google::Cloud::Dataplex::V1::DeleteDataScanRequest.new

      # Call the delete_data_scan method.
      result = client.delete_data_scan request

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

### REST

To delete a data quality scan, use the
[`dataScans.delete` method](https://docs.cloud.google.com/dataplex/docs/reference/rest/v1/projects.locations.dataScans/delete).

> [!NOTE]
> **Note:** Delete isn't supported for data quality scans that are on a one-time schedule.

<br />

## What's next

- Learn more about [data governance in BigQuery](https://docs.cloud.google.com/bigquery/docs/data-governance).