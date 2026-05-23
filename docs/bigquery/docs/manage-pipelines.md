# Manage pipelines

This document describes how to manage
[BigQuery pipelines](https://docs.cloud.google.com/bigquery/docs/pipelines-introduction),
including how to schedule and delete pipelines.

This document also describes how to view and manage pipeline metadata in
[Knowledge Catalog](https://docs.cloud.google.com/dataplex/docs/introduction).

Pipelines are powered by [Dataform](https://docs.cloud.google.com/dataform/docs/overview).

## Before you begin

1. [Create a BigQuery pipeline](https://docs.cloud.google.com/bigquery/docs/create-pipelines).
2. To manage pipeline metadata in Knowledge Catalog, ensure that the [Dataplex API](https://docs.cloud.google.com/dataplex/docs/enable-api) is enabled in your Google Cloud project.

### Required roles


To get the permissions that
you need to manage pipelines,

ask your administrator to grant you the
following IAM roles:

- To delete pipelines: [Dataform Admin](https://docs.cloud.google.com/iam/docs/roles-permissions/dataform#dataform.Admin) (`roles/dataform.Admin`) on the pipeline
- To view and run pipelines: [Dataform Viewer](https://docs.cloud.google.com/iam/docs/roles-permissions/dataform#dataform.Viewer) (`roles/dataform.Viewer`) on the project


For more information about granting roles, see [Manage access to projects, folders, and organizations](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).


You might also be able to get
the required permissions through [custom
roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles) or other [predefined
roles](https://docs.cloud.google.com/iam/docs/roles-overview#predefined).

To manage pipeline metadata in Knowledge Catalog,
ensure that you have the required
[Knowledge Catalog roles](https://docs.cloud.google.com/dataplex/docs/iam-roles)

For more information about Dataform IAM, see
[Control access with IAM](https://docs.cloud.google.com/dataform/docs/access-control).

> [!NOTE]
> **Note:** When you create a pipeline, BigQuery grants you the [Dataform Admin role](https://docs.cloud.google.com/dataform/docs/access-control#dataform.admin) (`roles/dataform.admin`) on that pipeline. All users with the Dataform Admin role granted on the Google Cloud project have owner access to all pipelines created in the project.

## View all pipelines

To view a list of all pipelines in your project, do the following:

1. In the Google Cloud console, go to the
   **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)

   If you don't see the left pane, click **Expand left pane** to open the pane.
3. In the **Explorer** pane, expand your project and click **Pipelines**.

## View past manual runs

To view past manual runs of a selected pipeline, follow these steps:

1. In the Google Cloud console, go to the
   **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)
3. In the **Explorer** pane, expand your project, click **Pipelines**,
   and then select a pipeline.

4. Click **Executions**.

5. Optional: To refresh the list of past runs, click **Refresh**.

## Configure alerts for failed pipeline runs

Each pipeline has a corresponding Dataform repository ID.
Each BigQuery pipeline run is logged in
[Cloud Logging](https://docs.cloud.google.com/logging/docs) using the corresponding
Dataform repository ID. You can use Cloud Monitoring to observe
trends in Cloud Logging logs for
BigQuery pipeline runs and to notify
you when conditions you describe occur.

To receive alerts when a BigQuery pipeline run fails,
you can create a log-based alerting policy for the corresponding
Dataform repository ID. For instructions, see
[Configure alerts for failed workflow invocations](https://docs.cloud.google.com/dataform/docs/monitor-runs#configure-alerts-failed-workflow-invocations).

To find the Dataform repository ID of your pipeline, do the following:

1. In the Google Cloud console, go to the
   **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)
3. In the **Explorer** pane, expand your project, click **Pipelines**,
   and then select a pipeline.

4. Click **Settings**.

   The Dataform repository ID of your pipeline is displayed at
   the bottom of the **Settings** tab.

## Delete a pipeline

To permanently delete a pipeline, follow these steps:

1. In the Google Cloud console, go to the
   **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)
3. In the **Explorer** pane, expand your project and click **Pipelines**.

4. Find the pipeline that you want to delete.

5. Click
   **View actions** next to the pipeline, and then click **Delete**.

6. Click **Delete**.

## Manage metadata in Knowledge Catalog

Knowledge Catalog lets you store and manage metadata for
pipelines. Pipelines are available in Knowledge Catalog
by default, without additional configuration.

You can use Knowledge Catalog to manage pipelines
in all [pipeline locations](https://docs.cloud.google.com/bigquery/docs/locations).
Managing pipelines in Knowledge Catalog
is subject to [Knowledge Catalog quotas and limits](https://docs.cloud.google.com/dataplex/docs/quotas)
and [Knowledge Catalog pricing](https://cloud.google.com/dataplex/pricing).

Knowledge Catalog automatically retrieves
the following metadata from pipelines:

- Data asset name
- Data asset parent
- Data asset location
- Data asset type
- Corresponding Google Cloud project

Knowledge Catalog logs pipelines as
[entries](https://docs.cloud.google.com/dataplex/docs/ingest-custom-sources#entries) with the following
entry values:

System entry group
:   The [system entry group](https://docs.cloud.google.com/dataplex/docs/ingest-custom-sources#entry-groups)
    for pipelines is `@dataform`. To view details of pipeline entries
    in Knowledge Catalog, you need to view the `dataform` system entry group.
    For instructions about how to view a list of all entries in an entry group, see
    [View details of an entry group](https://docs.cloud.google.com/dataplex/docs/ingest-custom-sources#entry-group-details)
    in the Knowledge Catalog documentation.

System entry type
:   The [system entry type](https://docs.cloud.google.com/dataplex/docs/ingest-custom-sources#entry-types)
    for pipelines is `dataform-code-asset`. To view details of
    pipelines,you need to view the `dataform-code-asset` system entry type,
    filter the results with an aspect-based filter,
    and [set the `type` field inside `dataform-code-asset` aspect to `WORKFLOW`](https://docs.cloud.google.com/dataplex/docs/search-syntax#aspect-search).
    Then, select an entry of the selected pipeline.
    For instructions about how to view details of a selected entry type, see
    [View details of an entry type](https://docs.cloud.google.com/dataplex/docs/ingest-custom-sources#entry-type-details)
    in the Knowledge Catalog documentation.
    For instructions about how to view details of a selected entry, see
    [View details of an entry](https://docs.cloud.google.com/dataplex/docs/search-assets#view-entry-details)
    in the Knowledge Catalog documentation.

System aspect type
:   The [system aspect type](https://docs.cloud.google.com/dataplex/docs/enrich-entries-metadata#aspect-types)
    for pipelines is `dataform-code-asset`. To
    provide additional context to pipelines in Knowledge Catalog
    by annotating data pipeline entries with
    [aspects](https://docs.cloud.google.com/dataplex/docs/enrich-entries-metadata#aspects),
    view the `dataform-code-asset` aspect type,
    filter the results with an aspect-based filter,
    and [set the `type` field inside `dataform-code-asset` aspect to `WORKFLOW`](https://docs.cloud.google.com/dataplex/docs/search-syntax#aspect-search).
    For instructions about how to annotate entries with aspects, see
    [Manage aspects and enrich metadata](https://docs.cloud.google.com/dataplex/docs/enrich-entries-metadata)
    in the Knowledge Catalog documentation.

Type
:   The type for data canvases is `WORKFLOW`.
    This type lets you filter pipelines in the `dataform-code-asset`
    system entry type and the `dataform-code-asset` aspect type by using the
    `aspect:dataplex-types.global.dataform-code-asset.type=WORKFLOW`
    query in an [aspect-based filter](https://docs.cloud.google.com/dataplex/docs/search-syntax#aspect-search).

For instructions about how to search for assets in Knowledge Catalog, see
[Search for data assets in Knowledge Catalog](https://docs.cloud.google.com/dataplex/docs/search-assets)
in the Knowledge Catalog documentation.

## What's next

- Learn more about [BigQuery pipelines](https://docs.cloud.google.com/bigquery/docs/pipelines-introduction).
- Learn how to [create pipelines](https://docs.cloud.google.com/bigquery/docs/create-pipelines).
- Learn how to [schedule pipelines](https://docs.cloud.google.com/bigquery/docs/schedule-pipelines).