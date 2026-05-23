# Comparison Shopping Services Center table schema

When your Comparison Shopping Service (CSS) Center reporting data is transferred
to BigQuery, the format of product and product issues data
corresponds primarily to the format of the relevant fields of the Content API's
[`ProductView`](https://developers.google.com/shopping-content/reference/rest/v2.1/reports/search#productview)
and
[`Productstatuses`](https://developers.google.com/shopping-content/v2/reference/v2.1/productstatuses)
resources.

## CSS Center Products Schema

The following table lists the schema for the `Products_` table. Some fields are
included as subsets of other fields. For example, the `Price` field contain
both `Value` and `Currency` fields.

| Field name | Type | Description |
|---|---|---|
| CSS ID | `INTEGER` | CSS ID |
| Merchant ID | `INTEGER` | Merchant account ID who owns the offer |
| Product ID | `STRING` | A unique identifier of the item |
| Feed label | `STRING` | The feed label for the item, or "-" if not provided |
| Language code | `STRING` | The two-letter ISO 639-1 language code for the item |
| Channel | `STRING` | The item's channel, either `online` or `local` |
| Title | `STRING` | Title of the item |
| Brand | `STRING` | Brand of the item |
| Category l{1-5} | `INTEGER` | Google Product Category of the item |
| Product type l{1-5} | `STRING` | Product type of the item |
| Price | `RECORD` | Full price of the item, prior to any discounts |
| Value | `INTEGER` | Price value of the item |
| Currency | `STRING` | The currency of the price |
| Sale Price | `RECORD` | Sale price of the item, if applicable |
| Value | `INTEGER` | Sale price value of the item |
| Currency | `STRING` | The currency of the sale price |
| Condition | `STRING` | Condition or state of the item |
| Availability | `STRING` | Availability status of the item |
| Shipping label | `STRING` | The shipping label specified in the feed |
| Gtin | `STRING` | [Global Trade Item Number](https://support.google.com/merchants/answer/188494#gtin) (GTIN) of the item |
| Item group ID | `STRING` | Shared identifier for all variants of the same product |
| Creation time | `INTEGER` | The time this item was created by the provider as timestamp microseconds |
| Expiration date | `DATE` | Date on which the item should expire, as specified upon insertion |
| Aggregated reporting context status | `STRING` | The status of the product aggregated for all reporting contexts. The supported values are `ELIGIBLE`, `ELIGIBLE_LIMITED`, `PENDING`, `NOT_ELIGIBLE_OR_DISAPPROVED`, `AGGREGATED_STATUS_UNSPECIFIED` |
| Reporting context statuses | `RECORD`, `REPEATED` | The status of the product in each reporting context and region |
| Reporting context | `STRING` | Reporting context |
| Region and status | `RECORD`, `REPEATED` | Status per region |
| Region | `STRING` | Region code represented in ISO 3166 format |
| Status | `STRING` | Status of the product in the region, can be `ELIGIBLE`, `PENDING`, or `DISAPPROVED` |
| Item issues | `RECORD`, `REPEATED` | The list of item level issues associated with the product |
| Type | `RECORD` | Issue type |
| Code | `STRING` | The error code of the issue, equivalent to the [`code`](https://developers.google.com/shopping-content/guides/product-issues) of Product issues |
| Canonical attribute | `STRING` | Canonical attribute name for attribute-specific issues |
| Severity | `RECORD` | How this issue affects serving of the offer |
| Severity per reporting context | `RECORD`, `REPEATED` | Issue severity per reporting context |
| Reporting context | `STRING` | Reporting context the issue applies to |
| Disapproved regions | `STRING`, `REPEATED` | List of disapproved regions in the reporting context, represented in ISO 3166 format |
| Demoted regions | `STRING`, `REPEATED` | List of demoted regions in the reporting context, represented in ISO 3166 format |
| Aggregated severity | `STRING` | Aggregated severity of the issue for all reporting contexts it affects. Its values can be `AGGREGATED_ISSUE_SEVERITY_UNSPECIFIED`, `DISAPPROVED`, `DEMOTED`, or `PENDING` |
| Resolution | `STRING` | Whether the issue can be resolved by the merchant |