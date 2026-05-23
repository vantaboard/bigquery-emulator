# Migrate the price competitiveness report

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

This document helps you migrate from the price benchmarks report, which will be deprecated on September 1, 2025,
to the new [price competitiveness](https://docs.cloud.google.com/bigquery/docs/merchant-center-price-competitiveness-schema) report.

The new price competitiveness report offers the following:

- Parity with the older version of the report and improved consistency with other similar Google products---for example, the [`PriceCompetitivenessProductView` field](https://developers.google.com/shopping-content/guides/reports/fields#pricecompetitivenessproductview) for the Content API for Shopping.
- [Additional insights](https://docs.cloud.google.com/bigquery/docs/merchant-center-price-competitiveness-migration#compare_price_benchmarks_and_price_competitiveness_table_schemas) about a merchant's pricing data.

## Compare price benchmarks and price competitiveness table schemas

The following table helps you identify fields in the
`Products_PriceBenchmarks` table
that have equivalent replacements in the
[`PriceCompetitiveness_` table](https://docs.cloud.google.com/bigquery/docs/merchant-center-price-competitiveness-schema#schema):

| Price benchmarks (old) | Price competitiveness (new) |
|---|---|
| `product_id` | `id` |
| `merchant_id` | `merchant_id` |
| `aggregator_id` | `aggregator_id` |
| `country_of_sale` | `report_country_code` |
| `price_benchmark_value` | `benchmark_price.amount_micros` |
| `price_benchmark_currency` | `benchmark_price.currency_code` |
| `price_benchmark_timestamp` | `_PARTITIONDATE` or `_PARTITIONTIME` |

Moreover, the `PriceCompetitiveness_` table contains additional data about
inventory such as title, brand, product types and category, and the product
price in a merchant's inventory. This data lets you effectively compare and
analyze the benchmark prices with your own.

The following additional fields are available in the new
[`PriceCompetitiveness_` table](https://docs.cloud.google.com/bigquery/docs/merchant-center-price-competitiveness-schema#schema):

| Field | Description |
|---|---|
| `title` | Title of the product. |
| `brand` | Brand of the product. |
| `offer_id` | Merchant-provided [ID of the product](https://support.google.com/merchants/answer/6324405). |
| `price` | Price of the product. |
| `price.amount_micros` | Price of the item, in micros (1 is represented as 1000000). |
| `price.currency_code` | Currency of the price of the item. |
| `product_type_l1` | [Product type attribute](https://support.google.com/merchants/answer/6324406) of the product. |
| `product_type_l2` | [Product type attribute](https://support.google.com/merchants/answer/6324406) of the product. |
| `product_type_l3` | [Product type attribute](https://support.google.com/merchants/answer/6324406) of the product. |
| `product_type_l4` | [Product type attribute](https://support.google.com/merchants/answer/6324406) of the product. |
| `product_type_l5` | [Product type attribute](https://support.google.com/merchants/answer/6324406) of the product. |
| `category_l1` | [Google product category](https://support.google.com/merchants/answer/6324436) of the product. |
| `category_l2` | [Google product category](https://support.google.com/merchants/answer/6324436) of the product. |
| `category_l3` | [Google product category](https://support.google.com/merchants/answer/6324436) of the product. |
| `category_l4` | [Google product category](https://support.google.com/merchants/answer/6324436) of the product. |
| `category_l5` | [Google product category](https://support.google.com/merchants/answer/6324436) of the product. |

Price competitiveness and price benchmarks don't support backfills. They always
return the current data available when you request a transfer.

## Example queries

This section highlights changes in example queries that are used to retrieve
price competitiveness data.

### Example 1: Retrieve product price benchmarks per country

The following queries return a list of product price benchmarks per country.
Note that a product can have different benchmarks in different countries.

#### Use the `Products_PriceBenchmarks` table (old)

    SELECT
      DATE(price_benchmark_timestamp) AS date,
      product_id,
      merchant_id,
      aggregator_id,
      country_of_sale,
      price_benchmark_value,
      price_benchmark_currency
    FROM
      `DATASET.Products_PriceBenchmarks_MERCHANT_ID`
    WHERE
      _PARTITIONDATE >= 'DATE';

#### Use the `PriceCompetitiveness` table (new)

    SELECT
      _PARTITIONDATE AS date,
      id,
      merchant_id,
      aggregator_id,
      report_country_code,
      benchmark_price.amount_micros,
      benchmark_price.currency_code
    FROM
      `DATASET.PriceCompetitiveness_MERCHANT_ID`
    WHERE
      _PARTITIONDATE >= 'DATE';

### Example 2: Retrieve products and associated benchmarks

The following queries retrieve products and their associated benchmarks.

#### Join the `Products` and `PriceBenchmarks` tables (old)

    WITH products AS (
      SELECT
        _PARTITIONDATE AS date,
        *
      FROM
        `DATASET.Products_MERCHANT_ID`
      WHERE
        _PARTITIONDATE >= 'DATE'
    ), benchmarks AS (
      SELECT
        _PARTITIONDATE AS date,
        *
      FROM
        `DATASET.Products_PriceBenchmarks_MERCHANT_ID`
      WHERE
        _PARTITIONDATE >= 'DATE'
    )
    SELECT
      products.date,
      products.product_id,
      products.merchant_id,
      products.aggregator_id,
      products.price,
      benchmarks.price_benchmark_value,
      benchmarks.price_benchmark_currency,
      benchmarks.country_of_sale
    FROM
      products
    INNER JOIN
      benchmarks
    ON products.product_id = benchmarks.product_id
      AND products.merchant_id = benchmarks.merchant_id
      AND products.date = benchmarks.date;

#### Use the `PriceCompetitiveness` table (new)

    SELECT
      _PARTITIONDATE AS date,
      id AS product_id,
      merchant_id,
      aggregator_id,
      price.amount_micros,
      price.currency_code,
      benchmark_price.amount_micros,
      benchmark_price.currency_code,
      report_country_code AS country_of_sale
    FROM
      `DATASET.PriceCompetitiveness_MERCHANT_ID`
    WHERE
      _PARTITIONDATE >= 'DATE';

In these queries, replace the following:

- `DATASET`: the name of your dataset
- `MERCHANT_ID`: the merchant account ID
- `DATE`: the date in the `YYYY-MM-DD` format

## What's next

- For more information about the new price competitiveness report, see [Google Google Merchant Center Price Competitiveness table](https://docs.cloud.google.com/bigquery/docs/merchant-center-price-competitiveness-schema).