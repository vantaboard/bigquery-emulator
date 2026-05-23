# Google Merchant Center Performance Table

## Overview

The Performance Table contains columns that you can use to understand the distribution of the following metrics:

- **Clicks:** The total number of clicks on your products on Google. Only
  visits to your product detail pages are counted.

- **Impressions:** The total number of times your products were shown on Google.
  Only impressions where customers had the option to visit your product
  detail pages are counted. If your products show zero impressions, you should verify that the
  products are approved and wait a few days for impressions to appear.
  Impressions may also not show if they are below a minimum threshold for a category, which can lead to different totals than what is reported in other Google surfaces.

Note that Performance data might get updated up to 3 days in the past
to account for corrections. The variations are usually small.

When you select `Performance` as one of the reports that are relevant to your transfer, the following table is created:

- `ProductPerformance_MERCHANT_ID`

## Schema

The `ProductPerformance_` table has the following schema:

| **Column** | **BigQuery data type** | **Description** | **Example data** |
|---|---|---|---|
| `merchant_id` | `INTEGER` | Google Merchant Center account ID. This field is a primary key | 1234 |
| `aggregator_id` | `INTEGER` | ID of the [Multi Client Account (MCA)](https://support.google.com/merchants/answer/188487) if the merchant center account ID is managed by an MCA. Null otherwise. | 12345 |
| `offer_id` | `STRING` | Merchant provided [id of the product](https://support.google.com/merchants/answer/6324405) at the time the interaction happened. This field is a primary key. | tddy123uk |
| `title` | `STRING` | Title of the product at the time the interaction happened. | TN2351 black USB |
| `brand` | `STRING` | Brand of the product at the time the interaction happened. | Brand Name |
| `category_l1` | `STRING` | [Google product category](https://support.google.com/merchants/answer/6324436) of the product at the time the interaction happened. Set to an empty string when no category exists. | Animals \& Pet Supplies |
| `category_l2` | `STRING` | [Google product category](https://support.google.com/merchants/answer/6324436) of the product at the time the interaction happened. Set to an empty string when no category exists. | Pet Supplies |
| `category_l3` | `STRING` | [Google product category](https://support.google.com/merchants/answer/6324436) of the product at the time the interaction happened. Set to an empty string when no category exists. | Dog Supplies |
| `category_l4` | `STRING` | [Google product category](https://support.google.com/merchants/answer/6324436) of the product at the time the interaction happened. Set to an empty string when no category exists. | Dog Beds |
| `category_l5` | `STRING` | [Google product category](https://support.google.com/merchants/answer/6324436) of the product at the time the interaction happened. Set to an empty string when no category exists. |   |
| `product_type_l1` | `STRING` | [Product type attribute](https://support.google.com/merchants/answer/6324406) of the product at the time the interaction happened. |   |
| `product_type_l2` | `STRING` | [Product type attribute](https://support.google.com/merchants/answer/6324406) of the product at the time the interaction happened. |   |
| `product_type_l3` | `STRING` | [Product type attribute](https://support.google.com/merchants/answer/6324406) of the product at the time the interaction happened. |   |
| `product_type_l4` | `STRING` | [Product type attribute](https://support.google.com/merchants/answer/6324406) of the product at the time the interaction happened. |   |
| `product_type_l5` | `STRING` | [Product type attribute](https://support.google.com/merchants/answer/6324406) of the product at the time the interaction happened. |   |
| `custom_label0` | `STRING` | [Custom label attribute](https://support.google.com/merchants/answer/6324473) of the product at the time the interaction happened. |   |
| `custom_label1` | `STRING` | [Custom label attribute](https://support.google.com/merchants/answer/6324473) of the product at the time the interaction happened. |   |
| `custom_label2` | `STRING` | [Custom label attribute](https://support.google.com/merchants/answer/6324473) of the product at the time the interaction happened. |   |
| `custom_label3` | `STRING` | [Custom label attribute](https://support.google.com/merchants/answer/6324473) of the product at the time the interaction happened. |   |
| `custom_label4` | `STRING` | [Custom label attribute](https://support.google.com/merchants/answer/6324473) of the product at the time the interaction happened. |   |
| `customer_country_code` | `STRING` | Country of the customer clicking or seeing the product. | CH |
| `clicks` | `INTEGER` | Total number of clicks on your products on Google that led to visits to your product detail pages. | 17 |
| `impressions` | `INTEGER` | Total number of times your products were shown on Google. Only impressions where customers had the option to visit your product detail pages are counted. If your products show zero impressions, you should verify that the products are approved and wait a few days for impressions to appear. Impressions may also not show if they are below a minimum threshold for a category, which can lead to different totals than what is reported in other Google surfaces. | 601 |
| `destination` | `STRING` | Whether the interaction happened on Shopping Ads or Free Listings. | FREE, ADS |