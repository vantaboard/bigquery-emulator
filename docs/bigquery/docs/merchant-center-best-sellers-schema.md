# Google Merchant Center best sellers table

## Overview

You can use the best sellers report to view the best-selling products and brands
on Google Shopping and in Shopping Ads. You can use the information from this
report to understand which products are performing well on Google, and whether
you carry them.

Selecting the best sellers report for your transfer creates five tables.
For a given merchant ID, the report creates the following tables:

- `BestSellersBrandWeekly_MERCHANT_ID`
- `BestSellersBrandMonthly_MERCHANT_ID`
- `BestSellersProductClusterWeekly_MERCHANT_ID`
- `BestSellersProductClusterMonthly_MERCHANT_ID`
- `BestSellersEntityProductMapping_MERCHANT_ID`

For an MCA account, the report generates the following tables:

- `BestSellersBrandWeekly_AGGREGATOR_ID`
- `BestSellersBrandMonthly_AGGREGATOR_ID`
- `BestSellersProductClusterWeekly_AGGREGATOR_ID`
- `BestSellersProductClusterMonthly_AGGREGATOR_ID`
- `BestSellersEntityProductMapping_AGGREGATOR_ID`

The best sellers tables follow the monthly or weekly best seller reports, both
product and brand, from
[Google Merchant Center](https://support.google.com/merchants/answer/9488679).
The latest monthly or weekly snapshot is updated daily. Since data is updated at
the start of each week or month, some data might be repeated several days in a
row. You can expect updated popular products data every week or month for the
previous period. The new data included in your metrics might not be available
for up to two weeks.

The mapping table `BestSellersEntityProductMapping_` contains
ranking entity IDs from the
`BestSellersProductCluster<Weekly/Monthly>_` tables and their
corresponding product IDs from the merchant's inventory. When generated at the
MCA level, the table contains mapping data for all of the subaccounts. This
table is meant for joining the best sellers data with information in other
tables exported by the Merchant Center transfer, which have the
same format of product ID (Products, Local Inventories, Regional Inventories,
Price Insights, Price Competitiveness, Product Targeting).

While brands are ranked across many different categories, all products in the
`Products_` table are in leaf categories. To join brands and products on
non-leaf categories, use the `google_product_category_ids` field.

> [!NOTE]
> **Note:** To access best sellers data, you must meet the [eligibility requirements for market insights](https://support.google.com/merchants/answer/9712881).

## `BestSellersProductCluster<Weekly/Monthly>_` tables

| **Column** | **BigQuery data type** | **Description** | **Example data** |
|---|---|---|---|
| `country_code` | `STRING` | Country in which the products are sold. Not all countries will contain ranking data. For more information, see the [list of included countries](https://support.google.com/merchants/answer/13299535#Availability). | CH |
| `report_category_id` | `INTEGER` | [Google product category ID](https://support.google.com/merchants/answer/6324436) of the sold products. | 1234 |
| `title` | `STRING` | Title of the best-selling product cluster. | TN2351 black USB |
| `brand` | `STRING` | Brand of the best-selling product cluster. Set to null when no brand exists. | Brand Name |
| `category_l1` | `STRING` | Google product category of the best-selling product cluster. Set to an empty string when no category exists. | Animals \& Pet Supplies |
| `category_l2` | `STRING` | Google product category of the best-selling product cluster. Set to an empty string when no category exists. | Pet Supplies |
| `category_l3` | `STRING` | Google product category of the best-selling product cluster. Set to an empty string when no category exists. | Dog Supplies |
| `category_l4` | `STRING` | Google product category of the best-selling product cluster. Set to an empty string when no category exists. | Dog Beds |
| `category_l5` | `STRING` | Google product category of the best-selling product cluster. |   |
| `variant_gtins` | `STRING` | [GTINs](https://support.google.com/merchants/answer/6324461) of products from your inventory corresponding to this product cluster, each separated by a space. | 3234567890126 3234567890131 |
| `product_inventory_status` | `STRING` | Status of this product in your inventory. In MCA-level tables, the value is always `NOT_IN_INVENTORY` or `UNKNOWN`. To get the inventory status of subaccounts, join the `BestSellersEntityProductMapping_` table with the `Products_` table. | IN_STOCK, NOT_IN_INVENTORY, OUT_OF_STOCK |
| `brand_inventory_status` | `STRING` | Status of this brand in your inventory, based on the status of products from this brand. Set to `UNKNOWN` when no brand exists. | IN_STOCK, NOT_IN_INVENTORY, OUT_OF_STOCK |
| `entity_id` | `STRING` | Identifier of this ranked best sellers entry. This column is used for joining with other tables, using the `BestSellersEntityProductMapping` table. | ab12345cdef6789gh |
| `rank` | `INTEGER` | Rank of the product (the lower, the more sold the product). | 5 |
| `previous_rank` | `INTEGER` | Rank of the product in the previous period (week or month). | 5 |
| `relative_demand` | `STRING` | Product's estimated demand in relation to the product with the highest rank in the same category and country. | VERY_HIGH, HIGH, MEDIUM, LOW, VERY_LOW |
| `previous_relative_demand` | `STRING` | Relative demand value for this product compared to the previous period (week or month). Set to `null` when no previous demand exists. | VERY_HIGH, HIGH, MEDIUM, LOW, VERY_LOW |
| `relative_demand_change` | `STRING` | How the relative demand changed for this product compared to the previous period (week or month). Set to `UNKNOWN` when no previous demand exists. | FLAT, SINKER, RISER |
| `price_range` | `RECORD` | Price range: lower and upper (with no decimals) and currency. The price doesn't include shipping costs. | n/a |
| `price_range.min_amount_micros` | `NUMERIC` | Price of the item, in micros (1 is represented as 1000000). | 115000000 |
| `price_range.max_amount_micros` | `NUMERIC` | Price of the item, in micros (1 is represented as 1000000). | 147000000 |
| `price_range.currency_code` | `STRING` | Currency of the price range of the item. | AUD |
| `snapshot_date` | `STRING` | The date for which the table is computed. Has the format `M%Y%m` for monthly tables and `W%Y%m%D` for weekly tables. | M202601, W20260216 |

> [!NOTE]
> **Note:** These tables don't have primary keys.

## `BestSellersBrand<Weekly/Monthly>_` tables

| **Column** | **BigQuery data type** | **Description** | **Example data** |
|---|---|---|---|
| `brand` | `STRING` | Best-selling brand. | Brand Name |
| `category_id` | `INTEGER` | [Google product category ID](https://support.google.com/merchants/answer/6324436) of the best-selling brand. | 1234 |
| `country_code` | `STRING` | Country in which the best-selling brand has been sold. For more information, see the [list of included countries](https://support.google.com/merchants/answer/13299535#Availability). | CH |
| `rank` | `INTEGER` | Rank of the best-selling brand (the lower the more sold). | 5 |
| `previous_rank` | `INTEGER` | Rank of the best-selling brand in the previous period (week or month). Set to `0` when no previous rank exists. | 5 |
| `relative_demand` | `STRING` | Product's estimated demand in relation to the product with the highest rank in the same category and country. | VERY_HIGH, HIGH, MEDIUM, LOW, VERY_LOW |
| `previous_relative_demand` | `STRING` | Relative demand in the previous period (week or month). Set to `null` when no previous demand exists. | VERY_HIGH, HIGH, MEDIUM, LOW, VERY_LOW |
| `relative_demand_change` | `STRING` | Change in demand compared to the previous period (week or month). Set to `UNKNOWN` when no previous rank exists. | FLAT, SINKER, RISER |
| `snapshot_date` | `STRING` | The date for which the table is computed. Has the format `M%Y%m` for monthly tables and `W%Y%m%D` for weekly tables. | M202601, W20260216 |

> [!NOTE]
> **Note:** These tables don't have primary keys.

## `BestSellersEntityProductMapping_` table

| **Column** | **BigQuery data type** | **Description** | **Example data** |
|---|---|---|---|
| `merchant_id` | `INTEGER` | Merchant Center account ID. If you query the table at the MCA level, this field contains the subaccount merchant ID. If you query the table for standalone accounts or subaccounts, this field contains the merchant account ID. | 1234 |
| `product_id` | `STRING` | REST ID of the product in the form: `channel:content_language:feed_label:offer_id`. This ID will always reflect the latest state of the product at the time the data is exported to BigQuery. This is the join key with all other tables containing the product ID in this format. | online:en:AU:666840730 |
| `entity_id` | `STRING` | Identifier of the ranked best sellers entry. This is the join key with the `BestSellersProductCluster&ltWeekly/Monthly>_` tables. | ab12345cdef6789gh |