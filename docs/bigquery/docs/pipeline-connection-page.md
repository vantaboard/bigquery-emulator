# Create data integration workflows using the Pipelines \& Connections page

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
> **Note:** To provide feedback, to ask questions, or to request to opt out of this Preview feature, contact [bigquery-pc-feedback@google.com](mailto:bigquery-pc-feedback@google.com).

The BigQuery **Pipelines \& Connections** page in the
console streamlines your data
integration tasks by providing guided, BigQuery-specific
configuration workflows for services like BigQuery Data Transfer Service,
Datastream, and Pub/Sub.

You can use the **Pipelines \& Connections** page to perform the following
tasks:

- **Loading data**. Find and load data from supported data sources using a guided experience.
- **Scheduling data tasks**. Schedule your data processes to run automatically, including scheduled queries, notebooks, data preparations, and pipelines.
- **Transforming data**. Create pipelines and data preparations that perform operations such as cleaning, structuring, and readying your data for analysis and reporting.
- **Creating connections to data sources**. Connect to external data sources, compute runtimes, and AI services for data access and remote execution.

After you create your tasks, the **Pipelines \& Connections** page provides a
single location for managing and monitoring your assets. You can see the status
of your assets in the list view, and you can use the detailed monitoring pages
to see operational metrics for each configured asset such as scheduled queries,
pipelines, transfers, connections, streams, and subscriptions.

Operational metrics include BigQuery Data Transfer Service run histories, throughput and
latency for Pub/Sub subscriptions, and data freshness and event counts
for Datastream pipelines.

The monitoring pages also include troubleshooting information such as logs for
BigQuery Data Transfer Service runs and Datastream object status.

## Opt in or out

During Preview, you can opt into or out of using the **Pipelines \&
Connections** page.

### Opt in

To opt into using the **Pipelines \& Connections** page, follow these steps:

1. Go to the Google Cloud console.

   [Go to the console](https://console.cloud.google.com/)
2. In the Google Cloud console toolbar, click
   **Navigation menu**.

3. Click **Solutions \> All products**.

4. In the **Analytics** section, click **BigQuery**.

   The BigQuery **Studio** page opens.
5. To expand the navigation menu, click
   **Toggle BigQuery navigation menu**.

   ![The BigQuery navigation menu.](https://docs.cloud.google.com/bigquery/images/bq-nav-menu.png)
6. In the navigation menu, click one of the following options:

   - **Data transfers**
   - **Scheduled queries**
   - **Scheduling**
7. Go to the unified pipelines and connections banner.

   ![The Unified pipelines and connections banner in the UI](https://docs.cloud.google.com/bigquery/images/pcbanner.png)
8. Click **Opt-in**.

   After you opt in, the existing **Data transfers** , **Scheduling** , and
   **Scheduled queries** pages are replaced by the **Pipelines \& Connections**
   page.

   > [!NOTE]
   > **Note:** There is no change to the **Dataform** page.

### Opt out

To opt out of using the unified **Pipelines \& Connections** page, follow these
steps:

1. On the **Pipelines \& Connections** page, click **Opt-out** in the
   unified pipelines and connections banner.

2. A feedback dialog opens. Provide feedback on why you're opting out such as
   missing features or bugs.

3. Click **Send** , and then click **Close**.

   After you submit the form, your navigation menu reverts back to using
   separate **Data transfers** , **Scheduling** , and **Scheduled queries**
   pages.

## Create a data integration asset

To create assets such as scheduled queries, data transfers, and external
connections, do the following:

1. Go to the **Pipelines \& Connections** page.

   [Go to Pipelines \& Connections](https://console.cloud.google.com/bigquery/pipelines-and-connections)
2. Click **Create**.

3. For the asset you want to create, click the option in the card:

   1. To add data from one of the supported data sources using a guided
      workflow, in the **Load data** card, click **Add data**.

   2. To schedule a query, notebook, data preparation, or pipeline, in the
      **Schedule data tasks** card, click **Schedule**, and then choose the
      appropriate option.

   3. To create a transformation pipeline or data preparation, in the
      **Transform data** card, click **Create**, and then choose the
      appropriate option.

   4. To create a connection to an external data source, in the **External
      connections** card, click **Create connection**.

## Monitor your assets

After you create your assets, you can view and monitor them.

1. Go to the **Pipelines \& Connections** page.

   [Go to Pipelines \& Connections](https://console.cloud.google.com/bigquery/pipelines-and-connections)
2. To view details about your connections, click the **Connections** tab.

3. To monitor your pipelines, click the **Pipelines** tab. You can use this
   page to view details such as the pipeline's status, type, and run history.

4. To view a pipeline's details, in the **Display name** column, click the
   link.

## What's next

- Learn how to [create a Dataform pipeline in BigQuery](https://docs.cloud.google.com/bigquery/docs/create-pipelines).
- Learn how to [schedule a pipeline](https://docs.cloud.google.com/bigquery/docs/schedule-pipelines).
- Learn how to [schedule a query](https://docs.cloud.google.com/bigquery/docs/scheduling-queries).
- Learn about [BigQuery Data Transfer Service transfers](https://docs.cloud.google.com/bigquery/docs/dts-introduction).