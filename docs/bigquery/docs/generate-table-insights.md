# Generate table insights

This document describes how to generate table insights for BigQuery
tables, BigLake tables, and external tables. Table insights help you
understand the data within a single table by generating natural language
questions and SQL queries based on table metadata.

Table insights can help you accelerate data exploration by letting you quickly
understand the structure and content of a new or unfamiliar table without
writing complex queries. You can also generate SQL queries based on natural
language questions, which reduces the time and effort required to write queries
manually. Table insights can also help non-SQL users understand and analyze data
through these natural language queries.

For an overview of table and dataset insights, see
[Data insights overview](https://docs.cloud.google.com/bigquery/docs/data-insights).

## Modes for generating table data insights

When generating table insights, BigQuery provides two modes:

| Mode | Description | Usage |
|---|---|---|
| **Generate and publish** ([Preview](https://cloud.google.com/products#product-launch-stages)) | Persists generated table insights into Knowledge Catalog as metadata aspects. You must have the necessary permissions to publish. When you use **Generate and publish**, the following actions occur: - Stores table and column descriptions in Knowledge Catalog. - Captures suggested queries and questions as reusable aspects. - Makes published insights accessible to all users who have appropriate Knowledge Catalog access, ensuring shared organizational knowledge. - Lets you edit and save descriptions directly in Knowledge Catalog. | Use this mode for enterprise-wide data documentation that persists and is reusable, or when building catalog-driven governance workflows. |
| **Generate without publishing** | Creates table insights such as descriptions, natural language questions, and SQL queries on demand. **Generate without publishing** doesn't publish insights to Knowledge Catalog. | Use this mode for quick, ad hoc exploration to avoid cluttering the catalog. |

## Before you begin

Data insights are generated using
[Gemini in BigQuery](https://docs.cloud.google.com/gemini/docs/bigquery/overview).
To start generating insights, you must first
[set up Gemini in BigQuery](https://docs.cloud.google.com/gemini/docs/bigquery/set-up-gemini).

> [!NOTE]
> **Note** : Gemini in BigQuery is part of Gemini for Google Cloud and doesn't support the same compliance and security offerings as BigQuery. You should only set up Gemini in BigQuery for BigQuery projects that don't require [compliance offerings that aren't supported by Gemini for Google Cloud](https://docs.cloud.google.com/gemini/docs/discover/certifications). For information about how to turn off or prevent access to Gemini in BigQuery, see [Turn off Gemini in BigQuery](https://docs.cloud.google.com/bigquery/docs/gemini-set-up#turn-off).

### Enable APIs

> [!IMPORTANT]
> **Important:** To enable any API in your project, ask your administrator to grant you the `serviceusage.services.enable` permission on your project.

To use data insights, enable the following APIs in your project:
Dataplex API, BigQuery API, and Gemini for Google Cloud API.

<br />

**Roles required to enable APIs**


To enable APIs, you need the Service Usage Admin IAM
role (`roles/serviceusage.serviceUsageAdmin`), which
contains the `serviceusage.services.enable` permission. [Learn how to grant
roles](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).

[Enable the APIs](https://console.cloud.google.com/flows/enableapi?apiid=dataplex.googleapis.com,bigquery.googleapis.com,cloudaicompanion.googleapis.com)

For more information about enabling the Gemini for Google Cloud API, see
[Enable the Gemini for Google Cloud API in a Google Cloud project](https://docs.cloud.google.com/gemini/docs/discover/set-up-gemini#enable-api).

### Roles and permissions

To create, manage, and retrieve data insights, ask your administrator to grant
you the following Identity and Access Management (IAM) roles:

- [Dataplex DataScan Editor](https://docs.cloud.google.com/iam/docs/roles-permissions/dataplex#dataplex.dataScanEditor) (`roles/dataplex.dataScanEditor`) or [Dataplex DataScan Administrator](https://docs.cloud.google.com/iam/docs/roles-permissions/dataplex#dataplex.dataScanAdmin) (`roles/dataplex.dataScanAdmin`) on the project where you want to generate insights.
- [BigQuery Data Viewer](https://docs.cloud.google.com/bigquery/docs/access-control#bigquery.dataViewer) (`roles/bigquery.dataViewer`) on the BigQuery tables for which you want to generate insights.
- [BigQuery Data Editor](https://docs.cloud.google.com/bigquery/docs/access-control#bigquery.dataEditor) (`roles/bigquery.dataEditor`) on the BigQuery tables for which you want to generate insights.
- [BigQuery User](https://docs.cloud.google.com/bigquery/docs/access-control#bigquery.user) (`roles/bigquery.user`) or [BigQuery Studio User](https://docs.cloud.google.com/bigquery/docs/access-control#bigquery.studioUser) (`roles/bigquery.studioUser`) on the project where you want to generate insights.

To get read-only access to the generated insights, ask your administrator
to grant you the following IAM role:

- [Dataplex DataScan DataViewer](https://docs.cloud.google.com/iam/docs/roles-permissions/dataplex#dataplex.dataScanDataViewer) (`roles/dataplex.dataScanDataViewer`) on the project containing the BigQuery tables for which you want to view insights.

To publish data insights to Knowledge Catalog, ask your administrator
to grant you the following IAM roles on the resource:

- Publish descriptions as aspects: [Dataplex Catalog Editor](https://docs.cloud.google.com/dataplex/docs/iam-roles#dataplex.catalogEditor) (`roles/dataplex.catalogEditor`)
- Publish queries as aspects: [Dataplex Entry and EntryLink Owner](https://docs.cloud.google.com/dataplex/docs/iam-roles#dataplex.entryOwner) (`roles/dataplex.entryOwner`)

To enable APIs, ask your administrator to grant you the following
IAM role:

- [Service Usage Admin](https://docs.cloud.google.com/iam/docs/roles-permissions/serviceusage#serviceusage.serviceUsageAdmin) (`roles/serviceusage.serviceUsageAdmin`) on the project where you want to generate insights.

For more information about granting roles, see
[Manage access to projects, folders, and organizations](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).

You might also be able to get the required permissions through
[custom roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles) or other
[predefined roles](https://docs.cloud.google.com/iam/docs/roles-overview#predefined). To see the exact permissions
that are required generate insights, expand the **Required permissions**
section:

#### Required permissions

- `bigquery.jobs.create`
- `bigquery.tables.get`
- `bigquery.tables.getData`
- `dataplex.datascans.create`
- `dataplex.datascans.get`
- `dataplex.datascans.getData`
- `dataplex.datascans.run`

## Generate insights for a BigQuery table

To generate insights for BigQuery tables, you can use the
Google Cloud console or the `DATA_DOCUMENTATION` scan type supported by the
Knowledge Catalog
[DataScans API](https://docs.cloud.google.com/dataplex/docs/reference/rest/v1/projects.locations.dataScans).
These scans generate metadata, SQL queries for data exploration, schema
descriptions, and table-level summaries.

### Console

To generate insights for a BigQuery table, you must access the
table entry in BigQuery using BigQuery Studio.

1. In the Google Cloud console, go to BigQuery Studio.

   [Go to BigQuery Studio](https://console.cloud.google.com/bigquery)
2. In the **Explorer** pane, select the project, dataset, and then the table
   you want to generate insights for.

3. Click the **Insights** tab. If the tab is empty, it means that the insights
   for this table are not generated yet.

4. To generate insights and publish them to Knowledge Catalog,
   click **Generate and publish**
   ([Preview](https://cloud.google.com/products#product-launch-stages)).

   To generate insights without publishing them to Knowledge Catalog,
   click **Generate without publishing**.

   For more information about the differences between the
   **Generate and publish** and **Generate without publishing** modes, see
   [Modes for generating insights](https://docs.cloud.google.com/bigquery/docs/generate-table-insights#modes_for_generating_table_data_insights).
5. Select a region to generate insights and click **Generate**.

   It takes a few minutes for the insights to be populated.

   If published data profiling results for the table are available, they're
   used to generate insights. Otherwise, insights are generated based on the
   column names and descriptions. For more information, see [Best practices
   to improve generated
   insights](https://docs.cloud.google.com/bigquery/docs/generate-table-insights#best_practices_for_generating_data_insights).
6. In the **Insights** tab, explore the generated natural language questions.

7. To view the SQL query that answers a question, click the question.

8. To open a query in BigQuery, click **Copy to Query**.

9. To ask follow-up questions, do the following:

   1. Click **Ask a follow-up** . The query opens in a new
      [data canvas](https://docs.cloud.google.com/bigquery/docs/data-canvas).

   2. Click **Run** , then click **Query these results**.

   3. To ask a follow-up question, enter a prompt in the **Natural language**
      prompt field or edit the SQL in the query editor.

10. To generate a new set of queries, click **Generate insights** and trigger
    the pipeline again.

After you have generated insights for a table, anyone with the
`dataplex.datascans.getData` permission and access to the table
can view those insights.

### REST

To generate insights programmatically, use the Knowledge Catalog
[DataScans API](https://docs.cloud.google.com/dataplex/docs/reference/rest/v1/projects.locations.dataScans).
To do this, complete the following steps:

1. [Optional: Create a data profile scan for the table](https://docs.cloud.google.com/bigquery/docs/generate-table-insights#create-profile-scan)
2. [Generate a data documentation datascan for the BigQuery table](https://docs.cloud.google.com/bigquery/docs/generate-table-insights#create-doc-scan)
3. [Check the data documentation scan status](https://docs.cloud.google.com/bigquery/docs/generate-table-insights#check-scan-status)
4. [Publish the data documentation scan results to BigQuery table](https://docs.cloud.google.com/bigquery/docs/generate-table-insights#publish-scan-results)

### Optional: Create a data profile scan for the table

The presence of data profile scans reduces hallucinations and approximations
by Gemini, as they ground the output in real values present in
the data.

To create and run a data profile scan, follow these steps:

1. Create a data profile scan using the
   [`dataScans.create` method](https://docs.cloud.google.com/dataplex/docs/reference/rest/v1/projects.locations.dataScans/create).

2. Run the data profile scan using the
   [`dataScans.run` method](https://docs.cloud.google.com/dataplex/docs/reference/rest/v1/projects.locations.dataScans/run).
   Wait for the run to complete.

3. Publish the scan results to the BigQuery table by
   attaching the following data profiling labels to the table:

   - `dataplex-dp-published-scan:DATASCAN_ID`
   - `dataplex-dp-published-project:PROJECT_ID`
   - `dataplex-dp-published-location:LOCATION`

   For more information, see
   [Add labels to tables and views](https://docs.cloud.google.com/bigquery/docs/adding-labels#adding_table_and_view_labels).

### Generate a data documentation datascan for the BigQuery table

You can choose to run a standard managed scan or a streamlined one-time scan.

#### Option A: Standard managed scan (create + run)

Use this method if you want to manage the scan resource over time.

1. Create a data documentation data scan using the [`dataScans.create` method](https://docs.cloud.google.com/dataplex/docs/reference/rest/v1/projects.locations.dataScans/create).

You can customize the scope of the generation to include schema,
descriptions, queries, or a combination using the `generation_scopes`
parameter. Optionally, you can publish these insights to
Knowledge Catalog by setting the
`catalogPublishingEnabled` parameter to `true`.

- To generate schema, table descriptions, and SQL queries, leave
  `data_documentation_spec` empty or set `generation_scopes` to `ALL`.
  For example:

        gcurl -X POST \
        https://dataplex.googleapis.com/v1/projects/PROJECT_ID/locations/LOCATION/\
        dataScans?dataScanId=DATASCAN_ID \
        -d '{
          "data": {
            "resource": "//bigquery.googleapis.com/projects/PROJECT_ID/\
        datasets/DATASET_ID/tables/TABLE_ID"
          },
          "executionSpec": {
            "trigger": { "onDemand": {} }
          },
          "type": "DATA_DOCUMENTATION",
          "dataDocumentationSpec": {
            "generationScopes": "ALL"
            "catalogPublishingEnabled": true
          }
        }'

  Replace the following:
  - <var translate="no">PROJECT_ID</var>: the ID of your Google Cloud project where the dataset resides
  - <var translate="no">LOCATION</var>: the region where the data scan runs
  - <var translate="no">DATASCAN_ID</var>: a unique name you provide for this scan
  - <var translate="no">DATASET_ID</var>: the ID of the BigQuery dataset being scanned
  - <var translate="no">TABLE_ID</var>: the ID of the BigQuery table being scanned
- To generate the schema, table descriptions, and column descriptions,
  without SQL queries, set `generation_scopes` to
  `TABLE_AND_COLUMN_DESCRIPTIONS`. For example:

        gcurl -X POST \
        https://dataplex.googleapis.com/v1/projects/PROJECT_ID/locations/LOCATION/\
        dataScans?dataScanId=DATASCAN_ID \
        -d '{
          "data": {
            "resource": "//bigquery.googleapis.com/projects/PROJECT_ID/\
        datasets/DATASET_ID/tables/TABLE_ID"
          },
          "executionSpec": {
            "trigger": { "onDemand": {} }
          },
          "type": "DATA_DOCUMENTATION",
          "dataDocumentationSpec": {
            "generationScopes": "TABLE_AND_COLUMN_DESCRIPTIONS"
            "catalogPublishingEnabled": true
          }
        }'

- To generate SQL queries without descriptions, set `generation_scopes` to
  `SQL_QUERIES`. For example:

        gcurl -X POST \
        https://dataplex.googleapis.com/v1/projects/PROJECT_ID/locations/LOCATION/\
        dataScans?dataScanId=DATASCAN_ID \
        -d '{
          "data": {
            "resource": "//bigquery.googleapis.com/projects/PROJECT_ID/\
        datasets/DATASET_ID/tables/TABLE_ID"
          },
          "executionSpec": {
            "trigger": { "onDemand": {} }
          },
          "type": "DATA_DOCUMENTATION",
          "dataDocumentationSpec": {
            "generationScopes": "SQL_QUERIES"
            "catalogPublishingEnabled": true
          }
        }'

1. Start the data documentation scan job using the
   [`dataScans.run` method](https://docs.cloud.google.com/dataplex/docs/reference/rest/v1/projects.locations.dataScans/run).

   For example:

       gcurl -X POST \
       https://dataplex.googleapis.com/v1/projects/PROJECT_ID/locations/LOCATION/\
       dataScans/DATASCAN_ID:run

   This request returns a unique job ID along with the initial state.

#### Option B: One-time scan (streamlined)

Use this method to initiate and complete a scan in a single API call.
This method removes the need to call the run method separately and allows
for automatic deletion of the scan resource using Time to Live (TTL)
functionality.

> [!NOTE]
> **Note:** One-time scan doesn't support subsequent run or update operations.

Create and trigger the scan using the `dataScans.create` method. For
example:

    gcurl -X POST \
    "https://dataplex.googleapis.com/v1/projects/PROJECT_ID/locations/LOCATION/dataScans?\
    dataScanId=DATASCAN_ID" \
    -d '{
      "data": {
        "resource": "//bigquery.googleapis.com/projects/PROJECT_ID/datasets/DATASET_ID/\
        tables/TABLE_ID"
      },
      "type": "DATA_DOCUMENTATION",
      "dataDocumentationSpec": {
        "generationScopes": "ALL",
        "catalogPublishingEnabled": true
      }
      "executionSpec": {
        "trigger": {
          "one_time": {
            "ttl_after_scan_completion": { "seconds": TTL_TIME }
          }
        }
      }
    }'

Replace the following:

- <var translate="no">PROJECT_ID</var>: the ID of your Google Cloud project where the dataset resides
- <var translate="no">LOCATION</var>: the region where the data scan runs
- <var translate="no">DATASCAN_ID</var>: a unique name you provide for this scan
- <var translate="no">DATASET_ID</var>: the ID of the BigQuery dataset being scanned
- <var translate="no">TABLE_ID</var>: the ID of the BigQuery table being scanned
- <var translate="no">TTL_TIME</var>: the duration in seconds after which the scan resource should be automatically deleted (for example, `3600` for one hour)

### Check the data documentation scan status

Check completion of the scan job run using the
[`dataScans.get` method](https://docs.cloud.google.com/dataplex/docs/reference/rest/v1/projects.locations.dataScans/get).

Use the job ID to fetch the status of the job. For example:

    gcurl -X GET https://dataplex.googleapis.com/v1/projects/PROJECT_ID/locations/LOCATION/dataScans/DATASCAN_ID/jobs/JOB_ID

The job completes when the status is either `SUCCEEDED` or `FAILURE`.

### Publish the data documentation scan results to BigQuery table

To publish the scan results to the BigQuery table,
attach the following data documentation labels to the table:

- `dataplex-data-documentation-published-scan:DATASCAN_ID`
- `dataplex-data-documentation-published-project:PROJECT_ID`
- `dataplex-data-documentation-published-location:LOCATION`

## Generate insights for a BigQuery external table

BigQuery data insights supports generating insights for
BigQuery external tables with data in Cloud Storage.
You and the Knowledge Catalog service account for the
current project must have the
[Storage Object Viewer role](https://docs.cloud.google.com/iam/docs/roles-permissions/storage#storage.objectViewer) (`roles/storage.objectViewer`)
on the Cloud Storage bucket that contains the data. For more
information, see
[Add a principal to a bucket-level policy](https://docs.cloud.google.com/storage/docs/access-control/using-iam-permissions#console).

To generate insights for a BigQuery external table, follow the
instructions described in the
[Generate insights for a BigQuery table](https://docs.cloud.google.com/bigquery/docs/generate-table-insights#insights-bigquery-table)
section of this document.

## Generate insights for a BigLake table

To generate insights for a
[BigLake table](https://docs.cloud.google.com/bigquery/docs/biglake-intro), follow these steps:

1. Enable the BigQuery Connection API in your project.

   [Enable the BigQuery Connection API](https://console.cloud.google.com/apis/library/bigqueryconnection.googleapis.com)
2. Create a BigQuery connection. For more information, see
   [Manage connections](https://docs.cloud.google.com/bigquery/docs/working-with-connections).

3. Grant the Storage Object Viewer (`roles/storage.objectViewer`)
   IAM role to the service account corresponding to the
   BigQuery connection that you created.

   You can retrieve the service account ID from the
   [connection details](https://docs.cloud.google.com/bigquery/docs/working-with-connections#view-connections).
4. To generate insights, follow the instructions described in the
   [Generate insights for a BigQuery table](https://docs.cloud.google.com/bigquery/docs/generate-table-insights#insights-bigquery-table)
   section of this document.

## Generate table and column descriptions

Gemini in BigQuery automatically generates table
and column descriptions when you generate data insights. You can edit these
descriptions as necessary, and then save them to the table's metadata. The saved
descriptions are used to generate future insights.

### Control generation language

You can guide Gemini to generate table and column descriptions
in a specific language. To do this, add a short directive (for example,
"Generate table and column descriptions using the French language") to your
table's existing description before you generate the data insights.

> [!NOTE]
> **Note:** To help Gemini understand your request, include the word "language" in your prompt.

When you generate insights, Gemini interprets this directive and
produces the metadata in the requested language. This mechanism works because
Gemini uses existing table descriptions as context when
generating new ones.

For a list of supported languages, see
[Gemini language support](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/models#language_support).

### Generate descriptions

To generate table and column descriptions, follow these steps:

1. Generate insights by following the instructions described in the relevant
   section of this document:

   - [Generate insights for a BigQuery table](https://docs.cloud.google.com/bigquery/docs/generate-table-insights#insights-bigquery-table)
   - [Generate insights for a BigQuery external table](https://docs.cloud.google.com/bigquery/docs/generate-table-insights#insights-bigquery-external-table)
   - [Generate insights for a BigLake table](https://docs.cloud.google.com/bigquery/docs/generate-table-insights#insights-biglake-table)
2. Click the **Schema** tab.

3. Click **View column descriptions**.

   > [!NOTE]
   > **Note:** If you don't see the **View column descriptions** button, click **Describe data**. You might need to scroll to see this button.

   The table description and column descriptions that were generated are
   displayed.
4. To edit and save the generated table description, do the following:

   1. In the **Table description** section, click **Save to details**.

   2. To replace the current description with the generated description, click
      **Copy suggested description**.

   3. Edit the table description as necessary, and then click
      **Save to details**.

      The table description is updated immediately.
5. To edit and save the generated column descriptions, do the following:

   1. In the **Column descriptions** section, click **Save to schema**.

      The column descriptions that were generated are populated in the
      **New description** field for each column.
   2. Edit the column descriptions as necessary, and then click **Save**.

      The column descriptions are updated immediately.
6. To close the preview panel, click
   **Close**.

## Best practices for generating data insights

To enhance the precision of your generated insights, adhere to the following
recommendations:

- Provide comprehensive descriptions. Ensure both tables and columns within
  the dataset have clear, detailed descriptions.

- Ground insights with profiling. If descriptions are unavailable, ensure a
  profile scan is linked to each table in the dataset to aid in grounding the
  generated insights.

- Explicitly define rules. Include any relationships or business logic that
  the insights module uses to influence relationship generation within the
  respective table's description.

### Ground insights to data profiling results

In generative AI, grounding is the ability to connect model output to verifiable
sources of information. You can ground generated table insights to data
profiling results. [Data profiling](https://docs.cloud.google.com/dataplex/docs/data-profiling-overview)
analyzes the columns in your BigQuery tables and identifies common statistical
characteristics, such as typical data values and data distribution.

When you [create a data profiling scan](https://docs.cloud.google.com/bigquery/docs/data-profile-scan#create_a_data_profile_scan)
for a table, you can choose to publish the scan results to the BigQuery and
Knowledge Catalog pages in the Google Cloud console. Insights uses data
profiling results to create more accurate, relevant queries by doing the
following:

1. Analyzes the data profiling results to identify interesting patterns,
   trends, or outliers in the data.

2. Generates queries that focus on these patterns, trends, or outliers to
   uncover insights.

3. Validates the generated queries against the data profiling results to ensure
   that the queries return meaningful results.

Without data profiling scans, the following things happen:

- The generated queries are more likely to include inaccurate clauses or
  produce meaningless results.

- The generated column descriptions are based only on the column name.

Ensure that the data profiling scan for your table is up-to-date and that the
results are published to BigQuery.

You can adjust your data profiling settings to increase the sampling size and
filter out rows and columns. After you run a new data profiling scan, regenerate
insights.

The quality of dataset insights also improves significantly if the tables in the
dataset have data profiling results.

### Add a table description

Detailed table descriptions that describe what you want to analyze in your table
can help Gemini in BigQuery to produce more
relevant insights for both table and dataset insights. After you
[add a table description](https://docs.cloud.google.com/bigquery/docs/managing-tables#updating_a_tables_description),
regenerate insights.

For example, you might add the following description to a `telco_churn` table
such as the one used in
[Example of table data insights](https://docs.cloud.google.com/bigquery/docs/data-insights#example_of_table_data_insights):

    This table tracks customer churn data, including subscription details, tenure,
    and service usage, to predict customer churn behavior.

If you save the [table description that Gemini generates](https://docs.cloud.google.com/bigquery/docs/generate-table-insights#generate-column-table-descriptions),
then that description is used to generate future insights.

### Add a column description

Column descriptions that explain what each column is, or how one column relates
to another, can improve the quality of your insights for both table and dataset
insights. After you [update the column descriptions](https://docs.cloud.google.com/bigquery/docs/managing-table-schemas#change-column-description)
in your table, regenerate insights.

For example, you might add the following descriptions to specific columns of a
`telco_churn` table such as the one used in
[Example of table data insights](https://docs.cloud.google.com/bigquery/docs/data-insights#example_of_table_data_insights):

For the `tenure` column:

    The number of months the customer has been with the service.

For the `churn` column:

    Whether the customer has stopped using the service. TRUE indicates the customer
    no longer uses the service, FALSE indicates the customer is active.

If you save the
[column descriptions that Gemini generates](https://docs.cloud.google.com/bigquery/docs/generate-table-insights#generate-column-table-descriptions),
then those descriptions are used to generate future insights.

## What's next

- Learn about [Data insights overview](https://docs.cloud.google.com/bigquery/docs/data-insights).
- Learn how to [Generate dataset insights](https://docs.cloud.google.com/bigquery/docs/generate-dataset-insights).
- Learn more about [Knowledge Catalog data profiling](https://docs.cloud.google.com/dataplex/docs/data-profiling-overview).