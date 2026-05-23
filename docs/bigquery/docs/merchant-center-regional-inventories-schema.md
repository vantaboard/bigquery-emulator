# Google Merchant Center regional inventories table

## Overview

Regional inventories table shows merchants their regional availability and pricing overrides of their products. The format of regional inventories data
corresponds primarily to the format of the relevant fields of the Content API's
[`regionalinventory`](https://developers.google.com/shopping-content/reference/rest/v2.1/regionalinventory) resource.

The data is written to a table named
`RegionalInventories_MERCHANT_ID` if you are using an
individual Merchant ID, or
`RegionalInventories_AGGREGATOR_ID` if you are using an
MCA account.

## Schema

The `RegionalInventories_` table has the following schema:

| **Column** | **BigQuery data type** | **Description** | **Example data** |
|---|---|---|---|
| `product_id` | `STRING` | Content API's REST ID of the product in the form: `channel:content_language:feed_label:offer_id`. This field is a primary key. | online:en:AU:666840730 |
| `merchant_id` | `INTEGER` | Merchant account ID. This field is a primary key. |   |
| `aggregator_id` | `INTEGER` | Aggregator account ID for multi-client accounts. |   |
| `region_id` | `STRING` | Region ID of the inventory. |   |
| `price` | `RECORD` | Regional price of the item. |   |
| `price.value` | `NUMERIC` | Regional price of the item. | 99 |
| `price.currency` | `STRING` | Currency of the regional price of the item. | CHF |
| `sale_price` | `RECORD` | Regional sale price of the item. |   |
| `sale_price.value` | `NUMERIC` | Regional sale price of the item. | 49 |
| `sale_price.currency` | `STRING` | Currency of the regional sale price of the item. | CHF |
| `sale_price_effective_start_date` | `TIMESTAMP` | Start date and time when the item is on sale in the region. | 2021-03-30 00:00:00 UTC |
| `sale_price_effective_end_date` | `TIMESTAMP` | End date and time when the item is on sale in the region. | 2021-04-14 00:00:00 UTC |
| `availability` | `STRING` | Regional availability status of the item. | out of stock |