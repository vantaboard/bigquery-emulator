# Load Google Merchant Center data into BigQuery

> [!WARNING]
>
> **Preview**
>
>
> This product is
>
> subject to the "Pre-GA Offerings Terms" in the General Service Terms section of the
> [Service Specific
> Terms](https://docs.cloud.google.com/terms/service-terms#1).
>
> Pre-GA products are available "as is" and might have limited support.
>
> For more information, see the
> [launch stage descriptions](https://cloud.google.com/products/#product-launch-stages).

> [!NOTE]
> **Note:** To get support or provide feedback for Google Merchant Center transfers with BigQuery Data Transfer Service, contact [gmc-transfer-preview@google.com](mailto:gmc-transfer-preview@google.com).

You can load data from Google Merchant Center to BigQuery using the
[BigQuery Data Transfer Service](https://docs.cloud.google.com/bigquery/docs/dts-introduction) for Google Merchant Center connector. With the
BigQuery Data Transfer Service, you can schedule recurring transfer jobs that
add your latest data from Google Merchant Center to
BigQuery.

## Supported reports

The BigQuery Data Transfer Service for Google Merchant Center supports the following data:

### Products and product issues

The products and product issues report includes product data provided to the Google Merchant Center through feeds or using
the Content API for Shopping. This report also includes item level issues detected by
Google for your products. You can view product and product issues data in the
[Google Merchant Center](https://merchants.google.com/) or by querying
the [Content API for Shopping](https://developers.google.com/shopping-content/v2/reference/v2.1/). For information on how this data is loaded into
BigQuery, see the Google Merchant Center [product table schema](https://docs.cloud.google.com/bigquery/docs/merchant-center-products-schema).

### Regional Inventories

The regional inventories report includes additional product data about
regional availability and pricing overrides of your products. For information
on how this data is loaded into BigQuery, see the Google Merchant Center [regional inventories table
schema](https://docs.cloud.google.com/bigquery/docs/merchant-center-regional-inventories-schema).

### Local Inventories

The local inventories report includes additional product data about local
inventory of your products. This report contains data on local pricing,
availability, quantity, pick-up and in-store product location. For information
on how this data is loaded into BigQuery, see the Google Merchant Center [local inventories table
schema](https://docs.cloud.google.com/bigquery/docs/merchant-center-local-inventories-schema).

### Performance

The performance report provides granular segmentation of your performance data
across both Ads and Free Listings. For information on how this data is loaded
into BigQuery, see the Google Merchant Center [performance table
schema](https://docs.cloud.google.com/bigquery/docs/merchant-center-performance-schema).

### Best Sellers

The Best sellers report provides the same data found in the Google Merchant Center UI and lets you backfill the data across countries or categories for
up to 2 years. This includes data about the most popular products and brands in
Shopping ads and unpaid listings, as well as whether or not you have them in
your inventory. This report is based on the [best sellers
report](https://support.google.com/merchants/answer/9488679)
available through Google Merchant Center. For information on how this data is
loaded into BigQuery, see the Google Merchant Center [best
sellers tables schemas](https://docs.cloud.google.com/bigquery/docs/merchant-center-best-sellers-schema).

### Price Competitiveness

Formerly known as the price benchmarks report, the price competitiveness report includes product level attributes and price benchmark data and is based
on the same definitions as the [price competitiveness
report](https://support.google.com/merchants/answer/9626903)
available through Google Merchant Center. For information on how this data is loaded into
BigQuery, see the Google Merchant Center [price competitiveness table schema](https://docs.cloud.google.com/bigquery/docs/merchant-center-price-competitiveness-schema).

### Price Insights

Use the price insights report to see suggested sale prices for your
products, and predictions for the performance that you can expect if you update
your products' prices. Using the price insights report can help you price your
products more effectively. For more information on how to use the data in this
report, see [Improve product pricing with the price insights
report](https://support.google.com/merchants/answer/11916926) for more
information on how to use the data in this report. For information on how this
data is loaded into BigQuery, see the Google Merchant Center
[Price Insights table
schema](https://docs.cloud.google.com/bigquery/docs/merchant-center-price-insights-schema).

### Product Targeting

Enable the Product Targeting report when you set up a transfer to expose Ads
targeting information when you load data from Google Shopping into
BigQuery. For information on how the data is loaded into
BigQuery, see the Google Merchant Center [Product Targeting table schema](https://docs.cloud.google.com/bigquery/docs/merchant-center-product-targeting-schema).

## Reporting options

The BigQuery Data Transfer Service for Google Merchant Center supports the following
reporting options:

| Reporting option | Support |
|---|---|
| Schedule | Configurable to daily, weekly, monthly, or custom. By default, this is set to daily when the transfer is created. The minimum interval between transfers is 6 hours. |

## Data ingestion

When you transfer data from Google Merchant Center into BigQuery, the data is loaded into BigQuery tables that are partitioned by date. The table partition that the data is loaded into corresponds to the date from the data source. If you schedule multiple transfers for the same date, BigQuery Data Transfer Service overwrites the partition for that specific date with the latest data. Multiple transfers in the same day or running backfills don't result in duplicate data, and partitions for other dates are not affected.

<br />

## Multi-client account (MCA) support

Existing customers with multiple Merchant IDs are encouraged to configure a
parent [Multi-Client Account (MCA)](https://support.google.com/merchants/answer/188487).
Configuring an MCA lets you create a single transfer for all your Merchant
IDs.

Using Google Merchant Center MCAs provides several benefits over using
individual Merchant IDs:

- You no longer need to manage multiple transfers to transfer reporting data for multiple Merchant IDs.
- Queries involving multiple Merchant IDs are much simpler to write because all Merchant ID data is stored in the same table.
- Using MCAs alleviates potential BigQuery load job quota issues because all your Merchant ID data is loaded in the same job.

One possible disadvantage of using MCAs is that your subsequent query costs are
likely to be higher. Because all of your data is stored in the same table,
queries that retrieve data for an individual Merchant ID must still scan the
entire table.

> [!NOTE]
> **Note:** The BigQuery Data Transfer Service pulls reports for all listed Merchant IDs. If there are no products in Google Shopping for a specific day, you may not see Merchant IDs in the BigQuery table.

If you are using an MCA, the MCA ID is listed under `aggregator_id` and the
individual sub-accounts are listed under `merchant_id`. For accounts that
don't use an MCA, `aggregator_id` is set to `null`.

## Limitations

Some reports might have their own constraints, such as different windows of
support for historical backfills. The following sections describe the
limitations for each report.

**Historical Backfills Support**

Not all reports support historical backfills in the same way. The following are
a list of reports and the level of support for historical backfills.

- Products and Product Issues - 14 days
- Local Inventories - 14 days
- Regional Inventories - 14 days
- Performance - 2 years
- Best Sellers - 2 years
- Price Competitiveness - No backfill support
- Price Insights - No backfill support

**Automatic Backfill Transfer Runs**

The Performance report can have latencies in "today's" data. Therefore, when a
data export is requested, data might update up to 3 days in the past to account
for corrections.

To support this functionality, whenever a transfer is triggered on any report,
two more transfer runs are created for `today - 1` and `today - 2`. These
transfer runs only affect the Performance table; other tables are not impacted.

The automatic backfills can't be disabled.