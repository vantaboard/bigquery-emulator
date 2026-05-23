# Manage BigQuery API dependencies

This document describes the Google Cloud services and APIs that
BigQuery depends on. It also explains the effects on
BigQuery behavior when you disable those services. Review this
document before you enable or disable services in your project.

Some services are enabled by default in every Google Cloud project that you create.
Other APIs are automatically enabled for all Google Cloud projects that use
BigQuery. The remaining services must be explicitly enabled
before you can use their functionality. For more information, see the
following resources:

- [Services enabled by default](https://docs.cloud.google.com/service-usage/docs/enabled-service#default)
- [Enabling and disabling services](https://docs.cloud.google.com/service-usage/docs/enable-disable)

This document is intended for administrators.

## Services enabled by default

The following services are enabled by default for every new
Google Cloud project:

| **Service** | **Which features rely on it** | **Effects of disabling this service** |
|---|---|---|
| `analyticshub.googleapis.com` | - [Publish data exchanges](https://docs.cloud.google.com/bigquery/docs/analytics-hub-introduction) - [Publish listings](https://docs.cloud.google.com/bigquery/docs/analytics-hub-manage-listings) and [manage subscriptions](https://docs.cloud.google.com/bigquery/docs/analytics-hub-manage-subscriptions) - [Data clean rooms](https://docs.cloud.google.com/bigquery/docs/data-clean-rooms) | - You can't create or manage data exchanges, listings, data clean rooms, or subscriptions. - You can't search and explore exchanges or listings that other providers create. - Created subscriptions persist but aren't accessible. - Linked datasets are accessible as long as the BigQuery API is enabled. - You can't create new subscriptions |
| `bigqueryconnection.googleapis.com` | - [Federated queries](https://docs.cloud.google.com/bigquery/docs/federated-queries-intro) to data stored outside of BigQuery - External tables and datasets - [BigQuery metastore](https://docs.cloud.google.com/bigquery/docs/about-bqms) | - You can't manage external connections. - You can't create remote models. - You can't create remote functions. - You can't query BigLake tables and object tables. |
| `bigquerymigration.googleapis.com` | - [Data migration assessment](https://docs.cloud.google.com/bigquery/docs/migration-assessment) - [SQL query translation](https://docs.cloud.google.com/bigquery/docs/interactive-sql-translator) | - You can't create migration tasks or assessments. - Existing tasks or assessments aren't available. **Note:** Usually you can disable this service after completing data migration. |
| `bigquerydatapolicy.googleapis.com` | - [Data masking](https://docs.cloud.google.com/bigquery/docs/column-data-masking-intro) | - You can't manage your data masking policies. - Data masking policies aren't deleted, but queries to tables with data masking applied fail. |
| `bigquerydatatransfer.googleapis.com` | - [Scheduled data transfers](https://docs.cloud.google.com/bigquery/docs/dts-introduction) | - You can't manage your scheduled data transfers. - Existing data transfers stop. |
| `bigqueryreservation.googleapis.com` | - [Capacity-based workload management](https://docs.cloud.google.com/bigquery/docs/reservations-get-started) | - You can't create or manage capacity commitments, reservations, or assignments. - You can't monitor slot usage. - Disaster recovery failover isn't available. - Slot autoscaling stops. |
| `bigquerystorage.googleapis.com` | - [Streaming data ingestion](https://docs.cloud.google.com/bigquery/docs/write-api-streaming) - [Batch data loading](https://docs.cloud.google.com/bigquery/docs/write-api-batch) - [Change data capture](https://docs.cloud.google.com/bigquery/docs/change-data-capture) | - You can't use the [Storage Read API](https://docs.cloud.google.com/bigquery/docs/reference/storage) or the [Storage Write API](https://docs.cloud.google.com/bigquery/docs/write-api) to access your BigQuery data. |
| `dataform.googleapis.com` | - Dataform provides code repositories that are leveraged by the following features: - [BigQuery pipelines](https://docs.cloud.google.com/dataform/docs/quickstart-create-workflow) - [Saved queries](https://docs.cloud.google.com/bigquery/docs/work-with-saved-queries) - [Colab notebooks](https://docs.cloud.google.com/bigquery/docs/notebooks-introduction) - [Dataform](https://docs.cloud.google.com/dataform/docs) - [Data preparation](https://docs.cloud.google.com/bigquery/docs/data-prep-introduction) - [Data canvas](https://docs.cloud.google.com/bigquery/docs/data-canvas) | - You can't create pipelines, saved queries, Colab notebooks, data canvases, data preparations, or Dataform projects. - Existing scheduled pipelines, notebooks, or Dataform projects stop. - Any existing pipelines, saved queries, Colab notebooks, data canvases, data preparations, or Dataform projects become inaccessible. |
| `dataplex.googleapis.com` | - Knowledge Catalog provides data cataloging and governance capabilities that are used by the following: - Resource Explorer in BigQuery Studio - Autocomplete in BigQuery Studio SQL editor - [BigQuery sharing (formerly Analytics Hub) search for listings](https://docs.cloud.google.com/bigquery/docs/analytics-hub-view-subscribe-listings) - [Profile insights](https://docs.cloud.google.com/bigquery/docs/data-profile-scan) - [Data quality scans](https://docs.cloud.google.com/bigquery/docs/data-quality-scan) - [Data lineage](https://docs.cloud.google.com/dataplex/docs/use-lineage#view-bq-lineage) viewing - [Table and dataset insights](https://docs.cloud.google.com/bigquery/docs/data-insights) - [Data canvas](https://docs.cloud.google.com/bigquery/docs/data-canvas) | - BigQuery data asset search is unavailable. - Sharing listing search is unavailable. - You can't create new or access previously created profile insights, data quality scans, or query suggestions. - You can't see data asset details on a lineage graph. - You can't search for data assets in data canvas. |

### Effect of disabling the BigQuery API

Disabling the BigQuery API also disables the following services which are
dependent upon BigQuery API:

- binaryauthorization.googleapis.com
- container.googleapis.com
- cloudapis.googleapis.com
- dataprep.googleapis.com
- servicebroker.googleapis.com
- telecomdatafabric.googleapis.com

## Services enabled by BigQuery Unified API

The BigQuery Unified API (`bigqueryunified.googleapis.com`)
includes a curated collection of services that are required for various
BigQuery features to function. If you enable the
BigQuery Unified API, then all of these services are activated
simultaneously. Google can update the services in this collection, and those
services are automatically enabled in projects with this API enabled.
You can disable individual services and APIs.

For instructions on enabling `bigqueryunified.googleapis.com`, see
[Enabling and disabling services](https://docs.cloud.google.com/service-usage/docs/enable-disable).

| **Service** | **Which features rely on it** | **Effects of disabling this service** |
|---|---|---|
| `aiplatform.googleapis.com` | - [Colab notebooks](https://docs.cloud.google.com/bigquery/docs/notebooks-introduction) - [BigQuery ML remote models](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model) | - You won't be able to run your notebooks. - Any existing BigQuery ML remote models stop working. - Your existing notebooks remain accessible for editing. |
| `bigqueryunified.googleapis.com` | - Provides a single-click activation of the BigQuery dependent services listed in this document, excluding the **cloudaicompanion** , **composer** and **datalineage** APIs. - Ensures new BigQuery dependencies are enabled in your project. | - Future dependencies aren't automatically enabled in your project. |
| `compute.googleapis.com` | - Google Compute Engine provides a runtime environment for all features provided by Managed Service for Apache Spark and Vertex AI. | - Colab notebooks, remote ML models, Apache Spark, SparkSQL, and PySpark jobs stop. - Source code remains available. - Dataproc API gets disabled. |
| `dataproc.googleapis.com` | - [Query data with open source engines such as Apache Spark.](https://docs.cloud.google.com/bigquery/docs/bqms-use-dataproc) - [Use Spark SQL or PySpark with Managed Service for Apache Spark.](https://docs.cloud.google.com/bigquery/docs/bqms-use-dataproc-serverless) - [Use stored procedures for Spark.](https://docs.cloud.google.com/bigquery/docs/spark-procedures) | - You can't create Managed Service for Apache Spark clusters to run open source data analytics. - You can't run Managed Service for Apache Spark workloads. - You can't run Spark in BigQuery workloads. |
| `datastream.googleapis.com` | - [Provides change data capture and replication to BigQuery.](https://docs.cloud.google.com/datastream/docs/overview) | - All data streams are paused and aren't accessible. |

## Services disabled by default

You must manually enable the following services for the corresponding
capabilities to become available:

| **Service** | **Which features rely on it** | **Effects of disabling this service** |
|---|---|---|
| `cloudaicompanion.googleapis.com` | - Gemini in BigQuery features | - Code completion, generation, and explanation features stop working. Learn more about [turning off Gemini in BigQuery](https://docs.cloud.google.com/bigquery/docs/gemini-set-up#turn-off). |
| `composer.googleapis.com` | - [Schedule workloads](https://docs.cloud.google.com/bigquery/docs/orchestrate-workloads) | - Existing Managed Service for Apache Airflow DAGs aren't listed on the Scheduling page and stop. - Existing Managed Airflow environments become inoperative, stop working, and return an error state. |
| `datalineage.googleapis.com` | - [Data lineage](https://docs.cloud.google.com/dataplex/docs/use-lineage#view-bq-lineage) capture and viewing | - Data lineage isn't captured for your project. - You can't view the lineage graph. |

## Manually enable BigQuery code assets

To manage code assets in BigQuery, such as notebooks and saved queries,
you must enable the following APIs:

- The Compute Engine API
- The Dataform API
- The Vertex AI API

Before March 2024, these APIs were not automatically enabled by default. If you
have automation scripts from before March 2024 that depended on the status of
these APIs, then you might need to update them. If you already have these APIs
enabled, then you will see new **Notebooks** and **Queries** folders in
the **Explorer** pane in BigQuery.

### Before you begin

To manually enable code asset management,
you must have the Identity and Access Management (IAM)
[Owner role](https://docs.cloud.google.com/iam/docs/roles-overview#legacy-basic)
(`roles/owner`).

### Manually enable BigQuery code assets

To enable required API dependencies for code assets, follow these steps:

1. Go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. On the **Studio** , in the tab bar of the editor pane, click the

   arrow drop-down next to the **+** sign, hold the pointer over **Notebook** ,
   and then select **Empty notebook**.

3. Click **Enable APIs**.

   If you don't see this option, check if you have the required
   IAM [Owner role](https://docs.cloud.google.com/iam/docs/roles-overview#legacy-basic)
   (`roles/owner`). If an empty notebook opens, then you already have the
   necessary APIs enabled.
4. In the **Enable core features** pane, in the **Core feature APIs** section,
   do the following:

   1. To enable all BigQuery dependencies for data streaming, scheduling, and notebooks, next to **BigQuery Unified API** click **Enable**.
   2. Optional: To choose which APIs to enable, click **View and enable individual APIs** and then click **Enable** next to each API that you want to enable.
   3. When the APIs are enabled, click **Next**.
5. Optional: Set user permissions in the **Permissions** section:

   - To grant principals the ability to create code assets, and to read, edit, and set permissions for the code assets they created, type their user or group names in the **BigQuery Studio User** field.
   - To grant principals the ability to read, edit, and set permissions for all code assets shared with them, type their user or group names in the **BigQuery Studio Admin** field.
6. Click **Next**.

7. Optional: In the **Additional APIs** section, click **Enable all** to enable
   the APIs that you need to create BigQuery remote procedures by using
   [BigQuery DataFrames](https://docs.cloud.google.com/python/docs/reference/bigframes/latest).

8. If you chose not to enable the additional APIs, click **Close** to close
   the **Enable core features** pane.

### Restrict access to code assets

You can help prevent enablement of additional APIs by setting the
[Restrict Resource Service Usage organization policy constraint](https://docs.cloud.google.com/resource-manager/docs/organization-policy/restricting-resources).
You can [turn off selected APIs](https://docs.cloud.google.com/service-usage/docs/enable-disable#disabling)
at any time.

## What's next?

- To learn how to manage Google Cloud services, see [Enabling and disabling services](https://docs.cloud.google.com/service-usage/docs/enable-disable).
- To learn how to manage API access at a granular level with organization policy constraints, see [Restricting resource usage](https://docs.cloud.google.com/resource-manager/docs/organization-policy/restricting-resources).
- To learn how to control access to services with Identity and Access Management (IAM) roles and permissions for BigQuery, see [BigQuery IAM roles and permissions](https://docs.cloud.google.com/bigquery/docs/access-control).