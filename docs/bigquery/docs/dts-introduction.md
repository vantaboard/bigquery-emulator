# What is the BigQuery Data Transfer Service?

The BigQuery Data Transfer Service automates data movement into [BigQuery](https://docs.cloud.google.com/bigquery/docs/introduction)
on a scheduled, managed basis. Your analytics team can lay the foundation for a
BigQuery data warehouse without writing a single line of code.

You can access the BigQuery Data Transfer Service using the:

- [Google Cloud console](https://docs.cloud.google.com/bigquery/docs/bigquery-web-ui)
- [bq command-line tool](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference)
- [BigQuery Data Transfer Service API](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest)

> [!TIP]
> **Tip:** You can also use the **Pipelines \& Connections** page to create a transfer using a [streamlined workflow](https://docs.cloud.google.com/bigquery/docs/pipeline-connection-page). This feature is in [preview](https://cloud.google.com/products/#product-launch-stages).

After you configure a data transfer, the BigQuery Data Transfer Service automatically
loads data into BigQuery on a regular basis. You can also
initiate data backfills to recover from any outages or gaps. You
cannot use the BigQuery Data Transfer Service to transfer data out of
BigQuery.

In addition to loading data into BigQuery,
BigQuery Data Transfer Service is used for two BigQuery operations:
[dataset copies](https://docs.cloud.google.com/bigquery/docs/copying-datasets) and
[scheduled queries](https://docs.cloud.google.com/bigquery/docs/scheduling-queries).

> [!NOTE]
> **Note:** Subscribe to the [BigQuery DTS announcements group](https://groups.google.com/g/bigquery-dts-announcements) to receive announcements related to the BigQuery Data Transfer Service.

## Supported data sources

The BigQuery Data Transfer Service supports loading data from the following data sources:

- SaaS platforms:
  - [Salesforce](https://docs.cloud.google.com/bigquery/docs/salesforce-transfer)
  - [Salesforce Marketing Cloud](https://docs.cloud.google.com/bigquery/docs/sfmc-transfer)

  - [ServiceNow](https://docs.cloud.google.com/bigquery/docs/servicenow-transfer)
- Marketing platforms:
  - [Facebook Ads](https://docs.cloud.google.com/bigquery/docs/facebook-ads-transfer)
  - [HubSpot](https://docs.cloud.google.com/bigquery/docs/hubspot-transfer) ([Preview](https://cloud.google.com/products/#product-launch-stages))
  - [Klaviyo](https://docs.cloud.google.com/bigquery/docs/klaviyo-transfer) ([Preview](https://cloud.google.com/products/#product-launch-stages))
  - [Mailchimp](https://docs.cloud.google.com/bigquery/docs/mailchimp-transfer) ([Preview](https://cloud.google.com/products/#product-launch-stages))
- Payment platforms:
  - [PayPal](https://docs.cloud.google.com/bigquery/docs/paypal-transfer) ([Preview](https://cloud.google.com/products/#product-launch-stages))
  - [Stripe](https://docs.cloud.google.com/bigquery/docs/stripe-transfer) ([Preview](https://cloud.google.com/products/#product-launch-stages))
  - [Shopify](https://docs.cloud.google.com/bigquery/docs/shopify-transfer) ([Preview](https://cloud.google.com/products/#product-launch-stages))
- Databases and data warehouses:
  - [Amazon Redshift](https://docs.cloud.google.com/bigquery/docs/migration/redshift)
  - [Apache Hive Metastore](https://docs.cloud.google.com/bigquery/docs/hdfs-data-lake-transfer)
  - [Microsoft SQL Server](https://docs.cloud.google.com/bigquery/docs/sqlserver-transfer) ([Preview](https://cloud.google.com/products/#product-launch-stages))
  - [MySQL](https://docs.cloud.google.com/bigquery/docs/mysql-transfer)
  - [Oracle](https://docs.cloud.google.com/bigquery/docs/oracle-transfer)
  - [PostgreSQL](https://docs.cloud.google.com/bigquery/docs/postgresql-transfer)
  - [Snowflake](https://docs.cloud.google.com/bigquery/docs/migration/snowflake-transfer) ([Preview](https://cloud.google.com/products/#product-launch-stages))
  - [Teradata](https://docs.cloud.google.com/bigquery/docs/migration/teradata)
- Cloud storage:
  - [Cloud Storage](https://docs.cloud.google.com/bigquery/docs/cloud-storage-transfer)
  - [Amazon Simple Storage Service (Amazon S3)](https://docs.cloud.google.com/bigquery/docs/s3-transfer)
  - [Azure Blob Storage](https://docs.cloud.google.com/bigquery/docs/blob-storage-transfer)
- Google Services:
  - [Campaign Manager](https://docs.cloud.google.com/bigquery/docs/doubleclick-campaign-transfer)
  - [Comparison Shopping Service (CSS) Center](https://docs.cloud.google.com/bigquery/docs/css-center-transfer-schedule-transfers) ([Preview](https://cloud.google.com/products/#product-launch-stages))
  - [Display \& Video 360](https://docs.cloud.google.com/bigquery/docs/display-video-transfer)
  - [Google Ads](https://docs.cloud.google.com/bigquery/docs/google-ads-transfer)
  - [Google Ad Manager](https://docs.cloud.google.com/bigquery/docs/doubleclick-publisher-transfer)
  - [Google Analytics 4](https://docs.cloud.google.com/bigquery/docs/google-analytics-4-transfer)
  - [Google Merchant Center](https://docs.cloud.google.com/bigquery/docs/merchant-center-transfer) ([Preview](https://cloud.google.com/products/#product-launch-stages))
  - [Search Ads 360](https://docs.cloud.google.com/bigquery/docs/search-ads-transfer)
  - [Google Play](https://docs.cloud.google.com/bigquery/docs/play-transfer)
  - [YouTube Channel](https://docs.cloud.google.com/bigquery/docs/youtube-channel-transfer)
  - [YouTube Content Owner](https://docs.cloud.google.com/bigquery/docs/youtube-content-owner-transfer)

### Data delivery SLO considerations

The [Data Delivery SLO](https://cloud.google.com/bigquery/sla?e=48754805)
applies to automatically scheduled data transfers using the
BigQuery Data Transfer Service from sources within Google Cloud.

For data transfers involving third-party or non-Google Cloud sources, service
outages with these sources can impact performance with the BigQuery Data Transfer Service.
As such, the Data Delivery SLO does not apply to BigQuery Data Transfer Service data
transfers from non-Google Cloud sources.

## Supported regions

Like BigQuery, the BigQuery Data Transfer Service is a
[multi-regional resource](https://docs.cloud.google.com/docs/geography-and-regions#regional_resources), with many additional single regions available.

A BigQuery dataset's locality is specified when you
[create a destination dataset](https://docs.cloud.google.com/bigquery/docs/datasets) to store the data
transferred by the BigQuery Data Transfer Service. When you set up a transfer, the
transfer configuration itself is set to the same location as the destination
dataset. The BigQuery Data Transfer Service processes and stages data in the same
location as the destination dataset.

The BigQuery Data Transfer Service supports data transfers from any region where your data
is stored to any location where your destination dataset is located.

For detailed information about transfers and region compatibility for
BigQuery Data Transfer Service, see [Dataset locations and transfers](https://docs.cloud.google.com/bigquery/docs/dts-locations).
For supported regions for BigQuery, see [Dataset locations](https://docs.cloud.google.com/bigquery/docs/locations#supported_locations).

## Using reservation slots with data transfers

Jobs triggered by the BigQuery Data Transfer Service only use reservation slots if the
project, folder, or organization is assigned to a reservation with any of the
following [job types](https://docs.cloud.google.com/bigquery/docs/reservations-workload-management#assignments):

- Query jobs using `QUERY`
- Load jobs using `PIPELINE`

Jobs that [copy datasets](https://docs.cloud.google.com/bigquery/docs/managing-datasets#copy-datasets)
don't use reservation slots.

## Pricing

For information on BigQuery Data Transfer Service pricing, see the
[Pricing](https://cloud.google.com/bigquery/pricing) page.

Once data is transferred to BigQuery, standard
BigQuery [storage](https://cloud.google.com/bigquery/pricing#storage) and
[query](https://cloud.google.com/bigquery/pricing#queries) pricing applies.

## Quotas

For information on BigQuery Data Transfer Service quotas, see the
[Quotas and limits](https://docs.cloud.google.com/bigquery/quotas) page.

## What's next

To learn how to create a transfer, see the documentation for your
[data source](https://docs.cloud.google.com/bigquery/docs/dts-introduction#supported_data_sources).