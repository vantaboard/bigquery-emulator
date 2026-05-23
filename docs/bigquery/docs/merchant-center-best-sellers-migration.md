# Migrate the best sellers report

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

This document helps you migrate from the older version of the best sellers reports
to its [newer version](https://docs.cloud.google.com/bigquery/docs/merchant-center-best-sellers-schema). The older version of the report that exports the `BestSellers_TopBrands_`, `BestSellers_TopProducts_`, and `BestSellers_TopProducts_Inventory_` tables will be deprecated on September 1, 2025.

The new best sellers report offers the following:

- Parity with the older version of the report and improved consistency with
  other similar Google products---for example, the [BestSellersBrandView](https://developers.google.com/shopping-content/guides/reports/fields#bestsellersbrandview) and [BestSellerProductClusterView](https://developers.google.com/shopping-content/guides/reports/fields#bestsellersproductclusterview) fields from Content API for Shopping.

- Additional insights about the popular products in [Google Merchant Center Analytics](https://support.google.com/merchants/answer/13299535).

- Extended backfilling capability (2 years instead of 14 days).

## Tables exported by old and new reports

The following table compares the tables exported by the old and new reports:

| Old report | New report |
|---|---|
| `BestSellers_TopBrands` | `BestSellersBrandWeekly` and `BestSellersBrandMonthly` |
| `BestSellers_TopProducts` | `BestSellersProductClusterWeekly` and `BestSellersProductClusterMonthly` |
| `BestSellers_TopProducts_Inventory` | `BestSellersEntityProductMapping` |

The old report contains a single aggregation of best sellers data over an
unspecified time window. The new report provides both the latest weekly and
monthly aggregations of this data at the time of the request.

### Compare `BestSellers_TopBrands` with `BestSellersBrandWeekly` and `BestSellersBrandMonthly`

The following table helps you identify fields in the `BestSellers_TopBrands` table that have equivalent replacements in the [`BestSellersBrandWeekly`](https://docs.cloud.google.com/bigquery/docs/merchant-center-best-sellers-schema#best-sellers-brand) and [`BestSellersBrandMonthly`](https://docs.cloud.google.com/bigquery/docs/merchant-center-best-sellers-schema#best-sellers-brand) tables. Replacements for some fields from the old table aren't available.

| `BestSellers_TopBrands` (old) | `BestSellersBrandWeekly` and `BestSellersBrandMonthly` (new) |
|---|---|
| `rank_timestamp` | `_PARTITIONDATE` and `_PARTITIONTIME` |
| `brand` | `brand` |
| `google_brand_id` |   |
| `ranking_category` | `category_id` |
| `ranking_category_path.locale` |   |
| `ranking_category_path.name` |   |
| `ranking_country` | `country_code` |
| `rank_id` |   |
| `rank` | `rank` |
| `previous_rank` | `previous_rank` |
| `relative_demand.bucket` | `relative_demand` |
| `relative_demand.min` |   |
| `relative_demand.max` |   |
| `previous_relative_demand.bucket` | `previous_relative_demand` |
| `previous_relative_demand.min` |   |
| `previous_relative_demand.max` |   |
|   | `relative_demand_change` |

### Compare `BestSellers_TopProducts` with `BestSellersProductClusterWeekly` and `BestSellersProductClusterMonthly`

The following table helps you identify fields in the `BestSellers_TopProducts` table that have equivalent replacements in the [`BestSellersProductClusterWeekly`](https://docs.cloud.google.com/bigquery/docs/merchant-center-best-sellers-schema#best-sellers-product-cluster) and [`BestSellersProductClusterMonthly`](https://docs.cloud.google.com/bigquery/docs/merchant-center-best-sellers-schema#best-sellers-product-cluster) tables. Replacements for some fields from the old table aren't available.

| `BestSellers_TopProducts` (old) | `BestSellersProductClusterWeekly` and `BestSellersProductClusterMonthly` (new) |
|---|---|
| `rank_timestamp` | `_PARTITIONDATE` and `_PARTITIONTIME` |
| `rank_id` | `entity_id` |
| `rank` | `rank` |
| `previous_rank` | `previous_rank` |
| `ranking_country` | `country_code` |
| `ranking_category` | `report_category_id` |
| `ranking_category_path.locale` |   |
| `ranking_category_path.name` |   |
| `relative_demand.bucket` | `relative_demand` |
| `relative_demand.min` |   |
| `relative_demand.max` |   |
| `previous_relative_demand.bucket` | `previous_relative_demand` |
| `previous_relative_demand.min` |   |
| `previous_relative_demand.max` |   |
|   | `relative_demand_change` |
| `product_title.locale` |   |
| `product_title.name` | `title` (single title instead of an array for every locale) |
| `gtins` | `variant_gtins` |
| `google_brand_id` |   |
| `brand` | `brand` |
| `google_product_category` |   |
|   | `category_l1`, `category_l2`, `category_l3`, `category_l4`, `category_l5` |
| `google_product_category_path.locale` |   |
| `google_product_category_path.name` |   |
| `price_range.min` | `price_range.min_amount_micros` |
| `price_range.max` | `price_range.max_amount_micros` |
| `price_range.currency` | `price_range.currency_code` |
|   | `product_inventory_status` |
|   | `brand_inventory_status` |

### Inventory mapping of best sellers data

In the old best sellers report, the best sellers data is mapped to the
merchant's inventory data in a new generated table,
using the `rank_id` column from the `TopProducts` table.

In the new best sellers report, the `entity_id` column is exported in the
`BestSellersProductCluster` tables, which is mapped to all the product IDs from
the merchant's inventory in the
[`BestSellersEntityProductMapping`](https://docs.cloud.google.com/bigquery/docs/merchant-center-best-sellers-schema#best-sellers-mapping) table.

| `BestSellers_TopProductsInventory` (old) | `BestSellersEntityProductMapping` (new) |
|---|---|
| `rank_id` (found in `BestSellers_TopProducts`) | `entity_id` (found in the `BestSellersProductClustersWeekly` and `BestSellersProductClustersMonthly` tables) |
| `product_id` | `product_id` |
| `merchant_id` |   |
| `aggregator_id` |   |

## Example queries

This section highlights changes in example queries that are used to retrieve
best sellers data.

### Example 1: Retrieve top products for a given category and country

The following queries return top products for a given category and country.

#### Use the `BestSellers_TopProducts` table (old)

    SELECT
      rank,
      previous_rank,
      relative_demand.bucket,
      (SELECT name FROM top_products.product_title WHERE locale = 'en-US') AS product_title,
      brand,
      price_range,
      google_product_category
    FROM
      `DATASET.BestSellers_TopProducts_MERCHANT_ID` AS top_products
    WHERE
      _PARTITIONDATE = 'DATE' AND
      ranking_category = 267 /*Smartphones*/ AND
      ranking_country = 'US'
    ORDER BY
      rank;

#### Use the `BestSellersProductClusterWeekly` or `BestSellersProductClusterMonthly` table (new)

    SELECT
      rank,
      previous_rank,
      relative_demand,
      title AS product_title,
      brand,
      price_range,
      category_l1,
      category_l2
    FROM
      `DATASET.BestSellersProductClusterWeekly_MERCHANT_ID` AS top_products
    WHERE
      _PARTITIONDATE = 'DATE' AND
      report_category_id = 267 /*Smartphones*/ AND
      country_code = 'US'
    ORDER BY
      rank;

### Example 2: Retrieve top products in your inventory

The following queries return a list of top products in your inventory.

#### Use the `BestSellers_TopProducts` table (old)

    WITH latest_top_products AS
    (
      SELECT
        *
      FROM
        `DATASET.BestSellers_TopProducts_MERCHANT_ID`
      WHERE
        _PARTITIONDATE = 'DATE'
    ),
    latest_top_products_inventory AS
    (
      SELECT
        *
      FROM
        `DATASET.BestSellers_TopProducts_Inventory_MERCHANT_ID`
      WHERE
        _PARTITIONDATE = 'DATE'
    )
    SELECT
      top_products.rank,
      inventory.product_id,
      (SELECT ANY_VALUE(name) FROM top_products.product_title) AS product_title,
      top_products.brand,
      top_products.gtins
    FROM
      latest_top_products AS top_products
    INNER JOIN
      latest_top_products_inventory AS inventory
    USING (rank_id);

#### Use the `BestSellersProductClusterWeekly` or `BestSellersProductClusterMonthly` table (new)

    WITH latest_top_products AS
    (
      SELECT
        *
      FROM
        `DATASET.BestSellersProductClusterWeekly_MERCHANT_ID`
      WHERE
        _PARTITIONDATE = 'DATE'
    ),
    latest_top_products_inventory AS
    (
      SELECT
        *
      FROM
        `DATASET.BestSellersEntityProductMapping_MERCHANT_ID`
      WHERE
        _PARTITIONDATE = 'DATE'
    )
    SELECT
      top_products.rank,
      inventory.product_id,
      top_products.title AS product_title,
      top_products.brand,
      top_products.variant_gtins
    FROM
      latest_top_products AS top_products
    INNER JOIN
      latest_top_products_inventory AS inventory
    USING (entity_id);

Moreover, if you want find the number of best selling products or brands in
your inventory, run a query on the `BestSellerProductClusterWeekly` or `BestSellerProductClusterMonthly` tables
using the `product_inventory_status` or `brand_inventory_status` columns.
See the following example query:

    SELECT
      *
    FROM
      `DATASET.BestSellersProductClusterMonthly_MERCHANT_ID`
    WHERE
      _PARTITIONDATE = 'DATE' AND
      product_inventory_status != 'NOT_IN_INVENTORY'
    ORDER BY
      rank;

### Example 3: Retrieve top brands for a given category and country

The following queries return a list of top brands for a given category and country.

#### Use the `BestSellers_TopBrands` table (old)

    SELECT
      rank,
      previous_rank,
      brand
    FROM
      `DATASET.BestSellers_TopBrands_MERCHANT_ID`
    WHERE
      _PARTITIONDATE = 'DATE' AND
      ranking_category = 267 /*Smartphones*/ AND
      ranking_country = 'US'
    ORDER BY
      rank;

#### Use the `BestSellersTopBrandsWeekly` or `BestSellersTopBrandsMonthly` table (new)

    SELECT
      rank,
      previous_rank,
      brand
    FROM
      `DATASET.BestSellersTopBrandsWeekly_MERCHANT_ID`
    WHERE
      _PARTITIONDATE = 'DATE' AND
      report_category_id = 267 /*Smartphones*/ AND
      country_code = 'US'
    ORDER BY
      rank;

### Example 4: Retrieve products of top brands in your inventory

The following queries return a list of products of top brands in your inventory.

#### Use the `BestSellers_TopBrands` table (old)

    WITH latest_top_brands AS
      (
        SELECT
          *
        FROM
          `DATASET.BestSellers_TopBrands_MERCHANT_ID`
        WHERE
          _PARTITIONDATE = 'DATE'
      ),
      latest_products AS
      (
        SELECT
          product.*,
          product_category_id
        FROM
          `DATASET.Products_MERCHANT_ID` AS product,
          UNNEST(product.google_product_category_ids) AS product_category_id,
          UNNEST(destinations) AS destination,
          UNNEST(destination.approved_countries) AS approved_country
        WHERE
          _PARTITIONDATE = 'DATE'
      )
    SELECT
      top_brands.brand,
      (SELECT name FROM top_brands.ranking_category_path
      WHERE locale = 'en-US') AS ranking_category,
      top_brands.ranking_country,
      top_brands.rank,
      products.product_id,
      products.title
    FROM
      latest_top_brands AS top_brands
    INNER JOIN
      latest_products AS products
    ON top_brands.google_brand_id = products.google_brand_id AND
       top_brands.ranking_category = product_category_id AND
       top_brands.ranking_country = products.approved_country;

#### Use the `BestSellersTopBrandsWeekly` or `BestSellersTopBrandsMonthly` table (new)

    WITH latest_top_brands AS
      (
        SELECT
          *
        FROM
          `DATASET.BestSellersBrandMonthly_MERCHANT_ID`
        WHERE
          _PARTITIONDATE = 'DATE'
      ),
      latest_products AS
      (
        SELECT
          product.*,
          product_category_id
        FROM
          `DATASET.Products_MERCHANT_ID` AS product,
          UNNEST(product.google_product_category_ids) AS product_category_id,
          UNNEST(destinations) AS destination,
          UNNEST(destination.approved_countries) AS approved_country
        WHERE
          _PARTITIONDATE = 'DATE'
      )
    SELECT
      top_brands.brand,
      --- The full category name is not supported in the new BestSellersTopBrands tables.
      --- (SELECT name FROM top_brands.ranking_category_path
      --- WHERE locale = 'en-US') AS ranking_category,
      top_brands.category_id,
      top_brands.rank,
      products.product_id,
      products.title
    FROM
      latest_top_brands AS top_brands
    INNER JOIN
      latest_products AS products
    ON top_brands.brand = products.brand AND
       top_brands.category_id = product_category_id AND
       top_brands.country_code = products.approved_country;

In these queries, replace the following:

- `DATASET`: the name of your dataset
- `MERCHANT_ID`: the merchant account ID
- `DATE`: the date in the `YYYY-MM-DD` format

## What's next

- For more information about the new best sellers report, see [Google Merchant Center best sellers table](https://docs.cloud.google.com/bigquery/docs/merchant-center-best-sellers-schema).