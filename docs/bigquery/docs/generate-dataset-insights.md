# Generate dataset insights

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

This document describes how to generate dataset insights for
BigQuery datasets. Dataset insights help you understand
relationships between tables in a dataset by generating relationship graphs and
cross-table queries.

Dataset insights help you accelerate the exploration of datasets with multiple
tables by automatically discovering and visualizing relationships between tables
in a graph, identifying primary-key and foreign-key relationships, and
generating sample cross-table queries. This is useful for understanding
data structure without documentation, discovering schema-defined, usage-based,
or AI-inferred relationships between tables, and generating complex
queries that join multiple tables.

For an overview of table and dataset insights, see
[Data insights overview](https://docs.cloud.google.com/bigquery/docs/data-insights).

## Modes for generating dataset insights

When generating dataset insights, BigQuery provides two modes:

| Mode | Description | Usage |
|---|---|---|
| **Generate and publish** | Persists generated dataset insights into Knowledge Catalog as metadata aspects and relationships. You must have the necessary permissions to publish. When you use **Generate and publish**, BigQuery does the following: - Stores the dataset description in Knowledge Catalog. - Captures suggested queries and questions as reusable aspects. - Captures relationships as metadata in Knowledge Catalog. - Makes published insights accessible to all users who have appropriate Knowledge Catalog access, ensuring shared organizational knowledge. - Lets you edit and save descriptions directly in Knowledge Catalog using the API. You can edit the suggested queries using the Google Cloud console. | Use this mode for enterprise-wide data documentation that persists and is reusable, or when building catalog-driven governance workflows. |
| **Generate without publishing** | Creates dataset insights such as descriptions, natural language questions, relationships, and SQL queries on demand. **Generate without publishing** doesn't publish insights to Knowledge Catalog. | Use this mode for quick, ad hoc exploration to avoid cluttering the catalog. |

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

### Complete a data profile scan

To improve the quality of insights, generate
[data profile scan](https://docs.cloud.google.com/bigquery/docs/data-profile-scan)
for tables in your dataset.

### Required roles


To get the permissions that
you need to generate, manage, and retrieve dataset insights,

ask your administrator to grant you the
following IAM roles:

- To generate, manage, and retrieve insights:
  - Dataplex DataScan Editor (`roles/dataplex.dataScanEditor`) or Dataplex DataScan Administrator (`roles/dataplex.dataScanAdmin`) on project
  - [BigQuery Data Editor](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquery#bigquery.dataEditor) (`roles/bigquery.dataEditor`) on tables
  - BigQuery User (`roles/bigquery.user`) or BigQuery Studio User (`roles/bigquery.studioUser`) on project
  - [BigQuery Resource Viewer](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquery#bigquery.resourceViewer) (`roles/bigquery.resourceViewer`) on project
- To view insights:
  - [Dataplex DataScan DataViewer](https://docs.cloud.google.com/iam/docs/roles-permissions/dataplex#dataplex.dataScanDataViewer) (`roles/dataplex.dataScanDataViewer`) on project
  - [BigQuery Data Viewer](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquery#bigquery.dataViewer) (`roles/bigquery.dataViewer`) on dataset
- To publish insights to Knowledge Catalog: [Dataplex Entry and EntryLink Owner](https://docs.cloud.google.com/iam/docs/roles-permissions/dataplex#dataplex.entryOwner) (`roles/dataplex.entryOwner`) on entry group


For more information about granting roles, see [Manage access to projects, folders, and organizations](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).


You might also be able to get
the required permissions through [custom
roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles) or other [predefined
roles](https://docs.cloud.google.com/iam/docs/roles-overview#predefined).

To see the exact permissions
that are required to generate insights, expand the **Required permissions**
section:

#### Required permissions

- `bigquery.datasets.get`: read dataset metadata
- `bigquery.jobs.create`: create jobs
- `bigquery.jobs.listAll`: list all jobs in the project
- `bigquery.tables.get`: get table metadata
- `bigquery.tables.getData`: get table data and metadata
- `dataplex.datascans.create`: create DataScan resource
- `dataplex.datascans.get`: read DataScan resource metadata
- `dataplex.datascans.getData`: read DataScan execution results
- `dataplex.datascans.run`: run on-demand DataScan
- `dataplex.entryGroups.useSchemaJoinEntryLink`: use `schema-join` entry links
- `dataplex.entryGroups.useSchemaJoinAspect`: use schema join aspects
- `dataplex.entryLinks.create`: create entry links
- `dataplex.entryLinks.update`: update entry links
- `dataplex.entryLinks.delete`: delete entry links
- `dataplex.entries.link`: link entries
- `dataplex.entries.update`: update entries
- `dataplex.entryGroups.useDescriptionsAspect`: use description aspects
- `dataplex.entryGroups.useQueriesAspect`: use query aspects

## Generate dataset insights

### Console

1. In the Google Cloud console, go to **BigQuery Studio**.

   [Go to BigQuery Studio](https://console.cloud.google.com/bigquery)
2. In the **Explorer** pane, select the project and then the dataset
   for which you want to generate insights.

3. Click the **Insights** tab.

4. To generate insights and publish them to Knowledge Catalog,
   click **Generate and publish**.

   To generate insights without publishing them to Knowledge Catalog,
   click **Generate without publishing**.

   For more information about the differences between the
   **Generate and publish** and **Generate without publishing** modes, see
   [Modes for generating dataset insights](https://docs.cloud.google.com/bigquery/docs/generate-dataset-insights#modes-dataset-insights).
5. If your dataset is in a multi-region, you might be prompted to select a
   region to generate insights. Select a region corresponding to the
   multi-region where the insights scan is going to be created.

   It takes a few minutes for the insights to be populated. The quality of
   insights improves if the tables in the dataset have
   [data profiling results](https://docs.cloud.google.com/bigquery/docs/data-profile-scan).

After insights are generated, BigQuery displays a dataset
description, a relationship graph, a relationship table, and sample
cross-table queries.

### REST

To generate insights programmatically, use the Knowledge Catalog
[DataScans API](https://docs.cloud.google.com/dataplex/docs/reference/rest/v1/projects.locations.dataScans).
To do this, complete the following steps:

1. [Generate a data documentation datascan for the BigQuery dataset](https://docs.cloud.google.com/bigquery/docs/generate-dataset-insights#create-doc-scan)
2. [Check the data documentation scan status](https://docs.cloud.google.com/bigquery/docs/generate-dataset-insights#check-scan-status)
3. [Verify publishing to Knowledge Catalog](https://docs.cloud.google.com/bigquery/docs/generate-dataset-insights#verify-publishing)

### Generate a data documentation datascan for the BigQuery dataset

1. Create a data documentation data scan using the
   [`dataScans.create` method](https://docs.cloud.google.com/dataplex/docs/reference/rest/v1/projects.locations.dataScans/create).
   Optionally, you can publish these insights to
   Knowledge Catalog by setting the
   `catalog_publishing_enabled` parameter to `true`.

   For example:

       alias gcurl='curl -H "Authorization: Bearer $(gcloud auth print-access-token)" -H "Content-Type: application/json"'
       gcurl -X POST \
       https://dataplex.googleapis.com/v1/projects/PROJECT_ID/locations/LOCATION/\
       dataScans?dataScanId=DATASCAN_ID \
       -d '{
         "data": {
           "resource": "//bigquery.googleapis.com/projects/PROJECT_ID/datasets/DATASET_ID"
         },
         "executionSpec": {
           "trigger": { "onDemand": {} }
         },
         "type": "DATA_DOCUMENTATION",
         "dataDocumentationSpec": {
           "catalog_publishing_enabled": true
         }
       }'

   Replace the following:
   - <var translate="no">PROJECT_ID</var>: the ID of your Google Cloud project where the dataset resides
   - <var translate="no">LOCATION</var>: the region where the data scan runs
   - <var translate="no">DATASCAN_ID</var>: a unique name you provide for this scan
   - <var translate="no">DATASET_ID</var>: the ID of the BigQuery dataset being scanned
2. Start the data documentation scan job using the
   [`dataScans.run` method](https://docs.cloud.google.com/dataplex/docs/reference/rest/v1/projects.locations.dataScans/run).

   For example:

       gcurl -X POST \
       https://dataplex.googleapis.com/v1/projects/PROJECT_ID/locations/LOCATION/\
       dataScans/DATASCAN_ID:run

   This request returns a unique job ID along with the initial state.

### Check the data documentation scan status

Check completion of the scan job run using the
[`dataScans.get` method](https://docs.cloud.google.com/dataplex/docs/reference/rest/v1/projects.locations.dataScans/get).
To retrieve the full results, including the insights and the publishing status,
set the `view` parameter to `FULL`.

Use the job ID to fetch the status of the job. For example:

    gcurl -X GET https://dataplex.googleapis.com/v1/projects/PROJECT_ID/locations/LOCATION/dataScans/DATASCAN_ID/jobs/JOB_ID?view=FULL

The job completes when the status is either `SUCCEEDED` or `FAILURE`.

A successful job response contains the generated insights in the
`dataDocumentationResult` field.

### Verify publishing to Knowledge Catalog

If `catalog_publishing_enabled` is set to `true`, then the insights are
published to Knowledge Catalog asynchronously after
the datascan job completes. To verify that insights were persisted, use
the Dataplex API to inspect the aspects of the dataset.

While insights are generated from the dataset-level datascan, the resulting
entry links are stored between the tables they connect. To verify these
relationships, use the
[`lookupEntryLinks` method](https://docs.cloud.google.com/dataplex/docs/reference/rest/v1/projects.locations/lookupEntryLinks)
to retrieve the entry links associated with a specific table entry.

To retrieve metadata for your BigQuery dataset, use the
[`entries.get` method](https://docs.cloud.google.com/dataplex/docs/reference/rest/v1/projects.locations.entryGroups.entries/get).
To include all aspects, set the `view` parameter to `FULL`. For example:

    gcurl -X GET https://dataplex.googleapis.com/v1/projects/PROJECT_ID/locations/LOCATION/entryGroups/@bigquery/entries/bigquery.googleapis.com/projects/DATASET_PROJECT_ID/datasets/DATASET_ID?view=FULL

Replace the following:

- <var translate="no">PROJECT_ID</var>: the ID of your Google Cloud project where the DataScan was configured
- <var translate="no">LOCATION</var>: the region where the entry group resides
- <var translate="no">DATASET_PROJECT_ID</var>: the ID of the Google Cloud project where the BigQuery dataset resides
- <var translate="no">DATASET</var>: the ID of the BigQuery dataset

> [!NOTE]
> **Note:** The entry group for BigQuery is always `@bigquery`. The portion of the URL following `entries/` is the Fully Qualified Name of the BigQuery resource, which Knowledge Catalog uses as a unique identifier.

If publishing to Knowledge Catalog is successful,
the following aspects are attached to the BigQuery dataset:

- Descriptions: contains AI-generated descriptions of the dataset
- Queries: contains relevant SQL queries related to the dataset
- Relationships: persisted as entry links between the tables present in the dataset

### View and save the dataset description

Gemini generates a natural language description of the dataset,
summarizing the types of tables it contains and the business domain it
represents. To save this description to the metadata of the dataset, click
**Save to details**.

You can edit the description before saving the details.

### Explore the relationship graph

The **Relationships** graph provides a visual representation of how tables in the
dataset relate to each other. It displays the top 10 most connected tables as
nodes, with lines representing relationships between them.

- To see relationship details, such as the columns that join two tables, hover over the edge connecting the table nodes.
- To rearrange the graph for better visibility, drag the table nodes.

### Use the relationship table

The **Relationship table** lists the discovered relationships in a tabular
format. Each row represents a relationship between two tables, showing the
source table and column, and the destination table and column. The **Source**
column indicates how the relationship was determined:

- **LLM inferred.** Relationships inferred by Gemini, based on table and column names and descriptions across the dataset.
- **Usage based.** Relationships extracted from query logs, based on frequent joins.
- **Schema-defined.** Relationships derived from existing primary key and foreign key mappings in the table schema.

You can filter the relationships for a specific table or provide feedback on
the quality of detected relationships. To export the generated dataset
description and relationships to a JSON file, click **Export to JSON**.

### Use query recommendations

Based on the discovered relationships, Gemini generates
sample queries. These are natural language questions with corresponding
SQL queries that join multiple tables in the dataset.

1. To view a SQL query, click a question.

2. To open the query in the BigQuery query editor, click
   **Copy to query**. You can then run the query or modify it.

3. To ask a follow up question, click **Ask a follow-up**, which opens an
   untitled data canvas where you can chat with Gemini to explore
   your data.

## What's next

- Learn about [data insights overview](https://docs.cloud.google.com/bigquery/docs/data-insights).
- Learn how to [generate table insights](https://docs.cloud.google.com/bigquery/docs/generate-table-insights).
- Learn more about [Knowledge Catalog data profiling](https://docs.cloud.google.com/dataplex/docs/data-profiling-overview).