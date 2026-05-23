# Google Merchant Center products table schema

## Overview

When your Google Merchant Center reporting data is transferred
to BigQuery, the format of product and product issues data
corresponds primarily to the format of the relevant fields of the Content API's
[Products](https://developers.google.com/shopping-content/v2/reference/v2.1/products)
and
[Productstatuses](https://developers.google.com/shopping-content/v2/reference/v2.1/productstatuses)
resources.

The data is written to a table named
`Products_MERCHANT_ID` if you are using an individual
Merchant ID, or
`Products_AGGREGATOR_ID` if you're using an MCA
account.

> [!NOTE]
> **Note:** The `Products and product issues` data is not available immediately when the report is first requested. When you first request a transfer for a merchant or aggregator ID, there might be a delay of up to 1 day before the `Products_` table is available for exporting.

## Schema

The `Products_` table has the following schema:

| Column | BigQuery data type | Description | Example data |
|---|---|---|---|
| `product_data_timestamp` | `TIMESTAMP` | Timestamp of the product data. | 2023-09-14 11:49:50 UTC |
| `product_id` | `STRING` | Content API's REST ID of the product in the form: `channel:content_language:feed_label:offer_id`. This is the primary key. | online:en:AU:666840730 |
| `merchant_id` | `INTEGER` | Merchant account ID. | 1234 |
| `aggregator_id` | `INTEGER` | Aggregator account ID for multi-client accounts. | 12345 |
| `offer_id` | `STRING` | Merchant provided [id of the product](https://support.google.com/merchants/answer/6324405). | tddy123uk |
| `title` | `STRING` | Title of the item. | TN2351 black USB |
| `description` | `STRING` | [Description](https://support.google.com/merchants/answer/6324468) of the item. | The TN2351 black USB has redefined how XJS can impact LLCD experiences. |
| `link` | `STRING` | Merchant provided [URL of the landing page](https://support.google.com/merchants/answer/6324416) of the product. | https://www.example.com/tn2351-black-usb/6538811?skuId=1234 |
| `mobile_link` | `STRING` | Merchant provided [URL of a mobile-optimized version](https://support.google.com/merchants/answer/6324459) of the landing page. | https://www.example.com/tn2351-black-usb/6538811?skuId=1234 |
| `image_link` | `STRING` | Merchant provided [URL of the main product image](https://support.google.com/merchants/answer/6324350). | https://www.example.com/tn2351-black-usb/6538811?skuId=1234 |
| `additional_image_links` | `STRING`, `REPEATED` | Merchant provided [additional URLs](https://support.google.com/merchants/answer/6324370) of images of the item. |   |
| `content_language` | `STRING` | The two-letter ISO 639-1 language code for the item. | en |
| `target_country` | `STRING` | Deprecated (always set to NULL) as part of a change to allow products to [target multiple countries](https://support.google.com/merchants/answer/7448571). Instead, use the following fields to read the status of each targeted country: [destinations.approved_countries](https://docs.cloud.google.com/bigquery/docs/merchant-center-products-schema#destinations.approved_countries), [destinations.pending_countries](https://docs.cloud.google.com/bigquery/docs/merchant-center-products-schema#destinations.pending_countries), [destinations.disapproved_countries](https://docs.cloud.google.com/bigquery/docs/merchant-center-products-schema#destinations.disapproved_countries). Issues can now apply to certain target countries and not others, as indicated in the field [issues.applicable_countries](https://docs.cloud.google.com/bigquery/docs/merchant-center-products-schema#issues.applicable_countries). | null |
| `feed_label` | `STRING` | The merchant provided [feed label](https://support.google.com/merchants/answer/12453549) for the item, or `-` if not provided. | US |
| `channel` | `STRING` | The item's channel, either `online` or `local`. | local, online |
| `expiration_date` | `TIMESTAMP` | Merchant provided date and time on which the [item should expire](https://support.google.com/merchants/answer/6324499), as specified upon insertion. Set to null if not provided. | 2023-10-14 00:00:00 UTC |
| `google_expiration_date` | `TIMESTAMP` | Date and time on which the item expires in Google Shopping. Never set to null. | 2023-10-14 00:00:00 UTC |
| `adult` | `BOOLEAN` | Set to true if the item is [targeted towards adults.](https://support.google.com/merchants/answer/6324508) | true, false |
| `age_group` | `STRING` | Merchant provided [target age group](https://support.google.com/merchants/answer/6324463) of the item. NULL if not provided. | newborn, infant, toddler, kids, adult |
| `availability` | `STRING` | Merchant provided [availability](https://support.google.com/merchants/answer/6324448) status of the item. | in stock, out of stock |
| `availability_date` | `TIMESTAMP` | Merchant provided date and time [when a pre-ordered product becomes available](https://support.google.com/merchants/answer/6324470) for delivery. NULL if not provided. | 2023-10-14 00:00:00 UTC |
| `brand` | `STRING` | Merchant provided [brand](https://support.google.com/merchants/answer/6324351) of the item. NULL if not provided. | Brand Name |
| `google_brand_id` | `STRING` | Google brand ID of the item. | 12759524623914508053 |
| `color` | `STRING` | Merchant provided [color](https://support.google.com/merchants/answer/6324487) of the item. NULL if not provided. | Silver, Gray, Multi |
| `condition` | `STRING` | Merchant provided [Condition](https://support.google.com/merchants/answer/6324469) or state of the item. | new, used, refurbished |
| `custom_labels` | `RECORD` | Merchant provided [custom labels](https://support.google.com/merchants/answer/6324473) for custom grouping of items in Shopping Ads. NULL if not provided. |   |
| `custom_labels.label_0` | `STRING` | Custom label 0. | my custom label |
| `custom_labels.label_1` | `STRING` | Custom label 1. | my custom label |
| `custom_labels.label_2` | `STRING` | Custom label 2. | my custom label |
| `custom_labels.label_3` | `STRING` | Custom label 3. | my custom label |
| `custom_labels.label_4` | `STRING` | Custom label 4. | my custom label |
| `gender` | `STRING` | Merchant provided target [gender](https://support.google.com/merchants/answer/6324479) of the item. NULL if not provided. | unisex, male, female |
| `gtin` | `STRING` | Merchant provided [Global Trade Item Number (GTIN)](https://support.google.com/merchants/answer/6324461) of the item. NULL if not provided. | 3234567890126 |
| `item_group_id` | `STRING` | Merchant provided [Shared identifier](https://support.google.com/merchants/answer/6324507) for all variants of the same product. NULL if not provided. | AB12345 |
| `material` | `STRING` | Merchant provided [material](https://support.google.com/merchants/answer/6324410) of which the item is made. NULL if not provided. | Leather |
| `mpn` | `STRING` | Merchant provided [Manufacturer Part Number](https://support.google.com/merchants/answer/6324482) (MPN) of the item. Set to NULL if not provided. | GO12345OOGLE |
| `pattern` | `STRING` | Merchant provided [pattern](https://support.google.com/merchants/answer/6324483). NULL if not provided. | Striped |
| `price` | `RECORD` | Merchant provided [price](https://support.google.com/merchants/answer/6324371) of the item. |   |
| `price.value` | `NUMERIC` | The price of the item. | 19.99 |
| `price.currency` | `STRING` | The currency of the price. | USD |
| `sale_price` | `RECORD` | Merchant provided [sale price](https://support.google.com/merchants/answer/6324471) of the item. |   |
| `sale_price.value` | `NUMERIC` | The sale price of the item. NULL if not provided. | 19.99 |
| `sale_price.currency` | `STRING` | The currency of the sale price. NULL if not provided. | USD |
| `sale_price_effective_start_date` | `TIMESTAMP` | Start date and time when the item is on sale. | 2023-10-14 00:00:00 UTC |
| `sale_price_effective_end_date` | `TIMESTAMP` | End date and time when the item is on sale. | 2023-10-14 00:00:00 UTC |
| `google_product_category` | `INTEGER` | The item's [Google product category](https://support.google.com/merchants/answer/1705911) ID. NULL if not provided. | 2271 |
| `google_product_category_ids` | `INTEGER, REPEATED` | The full path of [Google product categories](https://support.google.com/merchants/answer/1705911) to the item, stored as a set of IDs. NULL if not provided. |   |
| `google_product_category_path` | `STRING` | A human-readable version of the full path. Empty if not provided. | Apparel \& Accessories \> Clothing \> Dresses |
| `product_type` | `STRING` | Merchant-provided [category](https://support.google.com/merchants/answer/6324406) of the item. | Home \> Women \> Dresses \> Maxi Dresses |
| `additional_product_types` | `STRING`, `REPEATED` | Additional categories of the item. |   |
| `promotion_ids` | `STRING`, `REPEATED` | The list of [promotion IDs](https://support.google.com/merchants/answer/7050148) associated with the product. |   |
| `destinations` | `RECORD`, `REPEATED` | The intended destinations for the product. |   |
| `destinations.name` | `STRING` | The name of the destination; only `Shopping` is supported. This corresponds to the [Marketing Methods](https://support.google.com/merchants/answer/15130232) "Shopping Ads" and "Local Inventory Ads" in Merchant Center. | Shopping |
| `destinations.statushttps://docs.cloud.google.com/bigquery/docs/merchant-center-products-schema#products_and_product_issues_table_schema` | `STRING` | Deprecated (always set to NULL) as part of a change to allow products to [target multiple countries](https://support.google.com/merchants/answer/7448571). Instead, use the following fields to read the status of each targeted country: [destinations.approved_countries](https://docs.cloud.google.com/bigquery/docs/merchant-center-products-schema#destinations.approved_countries), [destinations.pending_countries](https://docs.cloud.google.com/bigquery/docs/merchant-center-products-schema#destinations.pending_countries), [destinations.disapproved_countries](https://docs.cloud.google.com/bigquery/docs/merchant-center-products-schema#destinations.disapproved_countries). Issues can now apply to certain target countries and not others, as indicated in the field [issues.applicable_countries](https://docs.cloud.google.com/bigquery/docs/merchant-center-products-schema#issues.applicable_countries). | NULL |
| `destinations.approved_countries` | `STRING, REPEATED` | List of [CLDR territory codes](http://www.unicode.org/repos/cldr/tags/latest/common/main/en.xml) where the offer is approved. | US, CH |
| `destinations.pending_countries` | `STRING, REPEATED` | List of [CLDR territory codes](http://www.unicode.org/repos/cldr/tags/latest/common/main/en.xml) where the offer is pending. | US, CH |
| `destinations.disapproved_countries` | `STRING, REPEATED` | List of [CLDR territory codes](http://www.unicode.org/repos/cldr/tags/latest/common/main/en.xml) where the offer is disapproved. | US, CH |
| `issues` | `RECORD`, `REPEATED` | The list of item level issues associated with the product. |   |
| `issues.code` | `STRING` | The error code of the issue. | image_too_generic |
| `issues.servability` | `STRING` | How this issue affects serving of the offer. | disapproved, unaffected |
| `issues.resolution` | `STRING` | Whether the issue can be resolved by the merchant. | merchant_action, pending_processing |
| `issues.attribute_name` | `STRING` | The attribute's name, if the issue is caused by a single attribute. NULL otherwise. | image link |
| `issues.destination` | `STRING` | The destination the issue applies to. Always set to `Shopping`. | Shopping |
| `issues.short_description` | `STRING` | Short issue description in English. | Generic image |
| `issues.detailed_description` | `STRING` | Detailed issue description in English. | Use an image that shows the product |
| `issues.documentation` | `STRING` | URL of a web page to help with resolving this issue. | https://support.google.com/merchants/answer/6098288 |
| `issues.applicable_countries` | `STRING, REPEATED` | List of [CLDR territory codes](http://www.unicode.org/repos/cldr/tags/latest/common/main/en.xml) where the issue applies. | CH |

## Query examples

### Products and product issues statistics

The following SQL sample query provides the number of products, products with
issues, and issues by day.

```googlesql
SELECT
  _PARTITIONDATE AS date,
  COUNT(*) AS num_products,
  COUNTIF(ARRAY_LENGTH(issues) > 0) AS num_products_with_issues,
  SUM(ARRAY_LENGTH(issues)) AS num_issues
FROM
  dataset.Products_merchant_id
WHERE
  _PARTITIONDATE >= 'YYYY-MM-DD'
GROUP BY
  date
ORDER BY
  date DESC
```

### Products disapproved for Shopping Ads

The following SQL sample query provides the number of products that are not
approved for display in Shopping Ads, separated by country. Disapproval
can result from the destination being
[excluded](https://support.google.com/merchants/answer/6324486)
or because of an issue with the product.

```googlesql
SELECT
  _PARTITIONDATE AS date,
  disapproved_country,
  COUNT(*) AS num_products
FROM
  dataset.Products_merchant_id,
  UNNEST(destinations) AS destination,
  UNNEST(disapproved_countries) AS disapproved_country
WHERE
  _PARTITIONDATE >= 'YYYY-MM-DD'
GROUP BY
  date, disapproved_country
ORDER BY
  date DESC
```

### Products with disapproved issues

The following SQL sample query retrieves the number of products with disapproved
issues, separated by country.

```googlesql
SELECT
  _PARTITIONDATE AS date,
  applicable_country,
  COUNT(DISTINCT CONCAT(CAST(merchant_id AS STRING), ':', product_id))
      AS num_distinct_products
FROM
  dataset.Products_merchant_id,
  UNNEST(issues) AS issue,
  UNNEST(issue.applicable_countries) as applicable_country
WHERE
  _PARTITIONDATE >= 'YYYY-MM-DD' AND
  issue.servability = 'disapproved'
GROUP BY
  date, applicable_country
ORDER BY
  date DESC
```

> [!NOTE]
> **Note:** This query constructs a unique key by using `merchant_id` and `product_id`. This is only required if you have an MCA account. When you use an MCA account, there is the potential for `product_id` collisions across multiple sub-accounts.