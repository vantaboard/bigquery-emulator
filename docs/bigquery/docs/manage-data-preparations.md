# Manage data preparations

This document describes how to manage your BigQuery data preparations,
including managing access, versioning, performance, and metadata. It also
describes how to perform basic tasks, such as viewing and downloading your data
preparations.

Data preparations are
[BigQuery](https://docs.cloud.google.com/bigquery/docs/query-overview#bigquery-studio)
resources powered by [Dataform](https://docs.cloud.google.com/dataform/docs/overview).
For more information, see [BigQuery data preparation overview](https://docs.cloud.google.com/bigquery/docs/data-prep-introduction).

## Before you begin

1. Ensure that you have enabled the [Gemini for Google Cloud API](https://docs.cloud.google.com/bigquery/docs/gemini-set-up#enable-api).
2. To manage data preparation metadata in Knowledge Catalog, ensure that the [Dataplex API](https://docs.cloud.google.com/dataplex/docs/enable-api) is enabled in your Google Cloud project.

### Required roles

Users who are preparing the data and the Dataform service
accounts that are running the jobs require the permissions granted by the
following Identity and Access Management (IAM) roles.

#### Get user access for data preparation


To get the permissions that
you need to prepare data in BigQuery,

ask your administrator to grant you the
following IAM roles:

- [BigQuery Studio User](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquery#bigquery.studioUser) (`roles/bigquery.studioUser`) on the project
- [Gemini for Google Cloud User](https://docs.cloud.google.com/iam/docs/roles-permissions/cloudaicompanion#cloudaicompanion.user) (`roles/cloudaicompanion.user`) on the project
- Access the source tables: [BigQuery Data Viewer](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquery#bigquery.dataViewer) (`roles/bigquery.dataViewer`) on the table, dataset, or project
- Share data preparations: [Dataform Code Owner](https://docs.cloud.google.com/iam/docs/roles-permissions/dataform#dataform.codeOwner) (`roles/dataform.codeOwner`) on the table, dataset, or project


For more information about granting roles, see [Manage access to projects, folders, and organizations](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).

For more information about IAM for datasets in BigQuery, see [Grant access to a dataset](https://docs.cloud.google.com/bigquery/docs/control-access-to-resources-iam#grant_access_to_a_dataset).


You might also be able to get
these permissions
with [custom roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles) or
other [predefined roles](https://docs.cloud.google.com/iam/docs/roles-overview#predefined).

#### Get access to manage metadata

To get the permissions you need to manage data preparation metadata in
Knowledge Catalog, ensure that you have the
required [Knowledge Catalog roles](https://docs.cloud.google.com/dataplex/docs/iam-roles)
and the
[`dataform.repositories.get`](https://docs.cloud.google.com/dataform/docs/access-control#predefined-roles)
permission.

#### Give access to the Dataform service account


To ensure that the Dataform service account has the necessary
permissions to execute data preparations in BigQuery,

ask your administrator to grant the
following IAM roles to the Dataform service account:

- Access the source tables: [BigQuery Data Viewer](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquery#bigquery.dataViewer) (`roles/bigquery.dataViewer`) on the table, dataset, or project
- Access the destination tables: [BigQuery Data Editor](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquery#bigquery.dataEditor) (`roles/bigquery.dataEditor`) on the table, dataset, or project


The Dataform service account might require additional permissions, depending on your data preparation pipeline. For more information, see [Grant Dataform required access](https://docs.cloud.google.com/dataform/docs/access-control#grant-dataform-required-access).

## View existing data preparations

To view a list of existing data preparations, follow these steps:

1. Go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)

   If you don't see the left pane, click **Expand left pane** to open the pane.
3. In the **Explorer** pane, expand your project.

4. Click **Data preparations**.

## Optimize data preparation by incrementally processing data

To configure the way your prepared data is written into a destination table,
follow these steps.

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)
3. In the **Explorer** pane, click **Data preparations**, and then select your data preparation.

4. In the toolbar of your data preparation, select **More \> Write
   mode**.

5. Select one of the options. For more information, see [Write mode](https://docs.cloud.google.com/bigquery/docs/data-prep-introduction#write-mode).

6. Click **Save**.

## Help improve suggestions

You can help improve Gemini suggestions by sharing with Google
the prompt data that you submit to features in [Preview](https://cloud.google.com/products#product-launch-stages).
To share your prompt data, follow these steps:

1. [Open the data preparation editor in BigQuery](https://docs.cloud.google.com/bigquery/docs/data-prep-get-suggestions#open-data-prep-editor).
2. In the data preparation toolbar, click settings **More**.
3. Select **Share data to improve Gemini in BigQuery**.

Data sharing settings apply to the entire project and can only be set by a
project administrator with the `serviceusage.services.enable` and
`serviceusage.services.list` IAM permissions. For more
information about data use in the Trusted Tester Program, see
[Gemini for Google Cloud Trusted Tester Program](https://cloud.google.com/gemini-for-cloud/ttp/welcome).

## Data preparation versions

You can choose to create a data preparation either inside of or outside of
a [repository](https://docs.cloud.google.com/bigquery/docs/repository-intro). Data preparation versioning
is handled differently based on where the data preparation is located.

### Data preparation versioning in repositories

Repositories are Git repositories that reside either in BigQuery
or with a third-party provider. You can use
[workspaces](https://docs.cloud.google.com/bigquery/docs/workspaces-intro) in repositories to perform
version control on data preparations. For more information, see
[Use version control with a file](https://docs.cloud.google.com/bigquery/docs/workspaces#use_version_control_with_a_file).

### Data preparation versioning outside of repositories

BigQuery data preparations that aren't in repositories
don't support viewing, comparing, or restoring data preparation versions.

For a list of data preparation versions in chronological order, follow these
steps:

1. Go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)
3. In the **Explorer** pane, click **Data preparations**, and then select your data preparation.

4. Click
   **Version history**.

## Download a data preparation

To download a data preparation in a SQLX file, follow these steps:

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)
3. In the **Explorer** pane, expand your project and click **Data preparations**.

4. Click the name of the data preparation that you want to download.

5. Click **Download** . The data preparation is saved in the
   [SQLX file format](https://docs.cloud.google.com/dataform/docs/overview#dataform-core)---for example,
   `NAME data preparation.dp.sqlx`.

> [!NOTE]
> **Note:** Data preparation files created before July 2025 are automatically migrated to the SQLX format, which changes how they are stored and run. This one-time migration is triggered in the following scenarios:
>
> - An existing data preparation migrates when you open it.
> - A data preparation in a pipeline migrates when you save or update the data preparation.

## Upload a data preparation

To upload a data preparation from a SQLX file, follow these steps:

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)
3. In the **Explorer** pane, expand your project.

4. Click **Data preparations** and click
   more_vert
   **View actions \> Upload to Data preparation**.

5. In the **Upload data preparation** dialog, select a file to upload, or enter
   the URL of the data preparation.

6. Enter a name for the data preparation.

7. Select a data preparation location where resources are managed and stored.

8. Click **Upload**.

## Manage metadata in Knowledge Catalog

Knowledge Catalog lets you store and manage metadata for
data preparations. Data preparations are available in Knowledge Catalog
by default, without additional configuration.

You can use Knowledge Catalog to manage data preparations
in all [BigQuery locations](https://docs.cloud.google.com/bigquery/docs/locations).
Managing data preparations in Knowledge Catalog
is subject to [Knowledge Catalog quotas and limits](https://docs.cloud.google.com/dataplex/docs/quotas)
and [Knowledge Catalog pricing](https://cloud.google.com/dataplex/pricing).

Knowledge Catalog automatically retrieves
the following metadata from data preparations:

- Data asset name
- Data asset parent
- Data asset location
- Data asset type
- Corresponding Google Cloud project

Knowledge Catalog logs data preparations as
[entries](https://docs.cloud.google.com/dataplex/docs/ingest-custom-sources#entries) with the following
entry values:

System entry group
:   The [system entry group](https://docs.cloud.google.com/dataplex/docs/ingest-custom-sources#entry-groups)
    for data preparations is `@dataform`. To view details of data preparation entries
    in Knowledge Catalog, you need to view the `dataform` system entry group.
    For instructions about how to view a list of all entries in an entry group, see
    [View details of an entry group](https://docs.cloud.google.com/dataplex/docs/ingest-custom-sources#entry-group-details)
    in the Knowledge Catalog documentation.

System entry type
:   The [system entry type](https://docs.cloud.google.com/dataplex/docs/ingest-custom-sources#entry-types)
    for data preparations is `dataform-code-asset`. To view details of
    data preparations,you need to view the `dataform-code-asset` system entry type,
    filter the results with an aspect-based filter,
    and [set the `type` field inside `dataform-code-asset` aspect to `DATA_PREPARATION`](https://docs.cloud.google.com/dataplex/docs/search-syntax#aspect-search).
    Then, select an entry of the selected data preparation.
    For instructions about how to view details of a selected entry type, see
    [View details of an entry type](https://docs.cloud.google.com/dataplex/docs/ingest-custom-sources#entry-type-details)
    in the Knowledge Catalog documentation.
    For instructions about how to view details of a selected entry, see
    [View details of an entry](https://docs.cloud.google.com/dataplex/docs/search-assets#view-entry-details)
    in the Knowledge Catalog documentation.

System aspect type
:   The [system aspect type](https://docs.cloud.google.com/dataplex/docs/enrich-entries-metadata#aspect-types)
    for data preparations is `dataform-code-asset`. To
    provide additional context to data preparations in Knowledge Catalog
    by annotating data preparation entries with
    [aspects](https://docs.cloud.google.com/dataplex/docs/enrich-entries-metadata#aspects),
    view the `dataform-code-asset` aspect type,
    filter the results with an aspect-based filter,
    and [set the `type` field inside `dataform-code-asset` aspect to `DATA_PREPARATION`](https://docs.cloud.google.com/dataplex/docs/search-syntax#aspect-search).
    For instructions about how to annotate entries with aspects, see
    [Manage aspects and enrich metadata](https://docs.cloud.google.com/dataplex/docs/enrich-entries-metadata)
    in the Knowledge Catalog documentation.

Type
:   The type for data canvases is `DATA_PREPARATION`.
    This type lets you filter data preparations in the `dataform-code-asset`
    system entry type and the `dataform-code-asset` aspect type by using the
    `aspect:dataplex-types.global.dataform-code-asset.type=DATA_PREPARATION`
    query in an [aspect-based filter](https://docs.cloud.google.com/dataplex/docs/search-syntax#aspect-search).

For instructions about how to search for assets, see
[Search for data assets in Knowledge Catalog](https://docs.cloud.google.com/dataplex/docs/search-assets)
in the Knowledge Catalog documentation.

## What's next

- Learn more about [preparing data in BigQuery](https://docs.cloud.google.com/bigquery/docs/data-prep-introduction).
- Learn how to [run data preparations manually or with a schedule](https://docs.cloud.google.com/bigquery/docs/orchestrate-data-preparations).
- Learn how to [create data preparations](https://docs.cloud.google.com/bigquery/docs/data-prep-get-suggestions).