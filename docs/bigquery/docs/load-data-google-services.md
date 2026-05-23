# Load data from other Google and Google Cloud services

You can use a number of Google Cloud services to load data into BigQuery where you can then
perform further analysis. These services typically require that you initiate
extract jobs from the respective console or API of the service. Once enabled,
data is loaded into BigQuery according to the cadence defined in
the service's extract job. Some extract jobs run in real time and others provide batch data loads.

For Google Cloud databases and services, including Google Drive and Google Sheets,
data queries originate from BigQuery. For more information,
see [external data sources](https://docs.cloud.google.com/bigquery/external-data-sources).

If a service is not listed, you might still be able to export data from the
service, but this might require using additional functionality. For more
information about how to set up custom exports or how to create load jobs and
queries from BigQuery, see
[Introduction to loading data](https://docs.cloud.google.com/bigquery/docs/loading-data).

## Cloud services that support data exports

### Carbon Footprint

The [Carbon Footprint](https://docs.cloud.google.com/carbon-footprint) export captures gross estimated greenhouse gas
emissions associated with the usage of covered Google Cloud services for the
selected billing account.

You can export your Carbon Footprint data to BigQuery in
order to perform data analysis, or to create custom dashboards and reports.

To set up exports of your Carbon Footprint data, see [Export your carbon footprint](https://docs.cloud.google.com/carbon-footprint/docs/export).

### Google Security Operations

You can export [Google Security Operations](https://chronicle.security) security logs to BigQuery for
additional data joins and analytics.

To set up exports of your Google Security Operations security logs, reach out to your
[Google Security Operations support](https://docs.cloud.google.com/chronicle/docs/getting-support) to set this up.

### Cloud Asset Inventory

[Cloud Asset Inventory](https://docs.cloud.google.com/asset-inventory) lets you export the asset metadata for your organization,
folder, or project to a BigQuery table, and then run data
analysis on your inventory.

To set up exports of your Cloud Asset Inventory data, see [Exporting to BigQuery](https://docs.cloud.google.com/asset-inventory/docs/exporting-to-bigquery).

### Cloud Billing

[Cloud Billing](https://docs.cloud.google.com/billing) export to BigQuery lets you export detailed
Google Cloud billing data (such as usage, cost estimates, and pricing data)
automatically throughout the day.

**Timing is important.** To have access to a more comprehensive set of billing
data for your analysis needs, we recommend that you enable Cloud Billing data
export to BigQuery at the same time that you create a
Cloud Billing account.

To set up exports of your Cloud Billing data, see [Export Cloud Billing data to BigQuery](https://docs.cloud.google.com/billing/docs/how-to/export-data-bigquery).

### Cloud Logging

You can route logs from [Cloud Logging](https://docs.cloud.google.com/logging) to BigQuery tables for
additional analytics and joins. For Google Cloud services, log data becomes
available for querying approximately 1 minute after it is generated.

To use BigQuery as a part of Observability Analytics, see [Observability Analytics](https://docs.cloud.google.com/logging/docs/analyze/query-and-view).

To set up exports of your Cloud Logging data, see [Route logs to supported sinks](https://docs.cloud.google.com/logging/docs/export/configure_export_v2).

### Customer Experience Insights

[CX Insights](https://docs.cloud.google.com/solutions/ccai-insights) lets you export your CX Insights conversation and analysis
data to BigQuery so that you can perform your own raw queries.

To configure exports of your CX Insights data, see [Export conversations to BigQuery](https://docs.cloud.google.com/contact-center/insights/docs/export).

### Dialogflow CX

[Dialogflow CX](https://docs.cloud.google.com/dialogflow) generates logs of the conversations between agents and
your customers.

To configure exports of conversations from Dialogflow CX, see [Interaction logging export to BigQuery](https://docs.cloud.google.com/dialogflow/cx/docs/concept/export-bq).

### Firebase

[Firebase](https://firebase.google.com/) contains a number of analytics exports you can send to BigQuery.
These include:

- Analytics
- Cloud messaging
- Crashlytics
- Performance monitoring
- A/B testing
- Remote configuration personalization

To configure exports of Firebase data, see [Export project data to BigQuery](https://firebase.google.com/docs/projects/bigquery-export).

### Google Analytics 4

To learn how to export your session data from a [Google Analytics 4](https://developers.google.com/analytics/devguides/collection/ga4)
reporting view into BigQuery, see
[BigQuery export](https://support.google.com/analytics/answer/9358801)
[Set up BigQuery export](https://support.google.com/analytics/answer/3416092)
in the Analytics Help Center. After the Google Analytics
4 data is in BigQuery, you can query it by using GoogleSQL.

If you are experiencing errors with your Google Analytics 4 export,
see [Reasons for linking failures](https://support.google.com/analytics/answer/9823238#reasons&zippy=%2Cin-this-article)
or [Reasons for export failures](https://support.google.com/analytics/answer/9823238#export&zippy=%2Cin-this-article).

If you are experiencing data delays from Google Analytics
4 exports, missing Google Analytics 4 data after an export, or other
issues, contact [Analytics support](https://support.google.com/analytics/).

For BigQuery issues, like billing, contact [Google Cloud Support](https://cloud.google.com/support/).

### Google Analytics 360

To learn how to export your session data from a [Google Analytics 360](https://marketingplatform.google.com/about/analytics-360/)
reporting view into BigQuery, see
[BigQuery export](https://support.google.com/analytics/topic/3416089)
in the Analytics Help Center. After the Google Analytics
360 data is in BigQuery, you can query it by using GoogleSQL.

For examples of querying Analytics data in
BigQuery, see
[BigQuery cookbook](https://support.google.com/analytics/answer/4419694)
in the Analytics Help.

For issues related to linking BigQuery and
Google Analytics 360, missing Google Analytics 360 data after
an export, or issues related to data delays when exporting
from Google Analytics 360, contact [Google Analytics 360 support](https://support.google.com/analytics/answer/9026876).

For BigQuery issues, like billing, contact [Google Cloud Support](https://cloud.google.com/support/).

### Google Search Console data

You can schedule a daily export of your [Google Search](https://www.google.com/search/about/) Console performance data
to BigQuery, where you can run complex queries over your data.

To set up exports of your data, see
[About bulk data export of Search Console data to BigQuery](https://support.google.com/webmasters/answer/12918484).

### Recommender

You can schedule daily snapshots of the [recommendations](https://docs.cloud.google.com/recommender/docs/whatis-activeassist)
using the BigQuery Data Transfer Service. Recommendations provide advice on optimizing
your usage of Google Cloud products and resources, and also provide insights
into your resource usage patterns.

To set up snapshots of your data using the BigQuery Data Transfer Service, see
[Export recommendations to BigQuery](https://docs.cloud.google.com/recommender/docs/bq-export/export-recommendations-to-bq).

### Vertex AI Batch Prediction

[Vertex AI](https://docs.cloud.google.com/vertex-ai) Batch Prediction creates a set of predictions based on an
input to a model. You can store these results in BigQuery for
additional analytics and joins.

To configure exports of batch prediction results, see [Get batch predictions and explanations](https://docs.cloud.google.com/vertex-ai/docs/tabular-data/classification-regression/get-batch-predictions#make-batch-request).

### Vertex AI Predictions

You can use Vertex AI Predictions to store prediction results from
online endpoints in BigQuery for additional analysis.

To configure model prediction integration with BigQuery, see [Online prediction logging](https://docs.cloud.google.com/vertex-ai/docs/predictions/online-prediction-logging#enabling-and-disabling).

## What's next

- Learn about other integrations you can initiate in BigQuery using [BigQuery Data Transfer Service](https://docs.cloud.google.com/bigquery/docs/dts-introduction).
- Learn about connections to other [external data sources](https://docs.cloud.google.com/bigquery/external-data-sources).