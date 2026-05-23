# Search Ads 360 migration guide

The Search Ads 360 connector (formerly known as *Doubleclick Search* )
relies on the new [Search Ads 360 reporting API](https://developers.google.com/search-ads/reporting/overview).
The [old Search Ads 360 reporting API](https://developers.google.com/search-ads/v2/how-tos/reporting)
is no longer supported, so you should migrate your BigQuery Data Transfer Service
workflows to be compatible with the new Search Ads 360 reporting API.
This document shows you the changes of the new Search Ads 360 from
the old Search Ads 360 and provides mapping
information to migrate your existing resources to the new
Search Ads 360.

## What's new with Search Ads 360

The new Search Ads 360 reporting API offers several changes that
might affect your existing BigQuery Data Transfer Service workflows.

### Account structure

The new Search Ads 360 reporting API organizes accounts into a
hierarchy of manager accounts, sub-manager accounts, and client accounts. For more
information, see [Account hierarchy differences](https://support.google.com/sa360/answer/13633455)
and [About manager accounts](https://support.google.com/sa360/answer/9158072).

### ID space

Entities in the new Search Ads 360 have a different [ID space](https://developers.google.com/search-ads/v2/how-tos/reporting/id-mapping)
mapping than previous versions of Search Ads 360. For information
about mapping between previous IDs and new IDs, see [ID mapping](https://docs.cloud.google.com/bigquery/docs/search-ads-migration-guide#id_mapping).

### Resource-based reporting

The new Search Ads 360 API data model uses a resource-based data
model, as opposed to the old Search Ads 360 API which
uses a report-based data model. The new Search Ads 360 API connector
creates BigQuery tables by querying
[resources](https://developers.google.com/search-ads/reporting/concepts/api-structure#resources)
in Search Ads 360. For more information about the resource structure
in the new Search Ads 360 API, see [Search Ads 360 reporting API structure](https://developers.google.com/search-ads/reporting/concepts/api-structure).

## Migrate transfer configurations

There is no automated method to convert existing Search Ads 360 transfer
configurations to the new Search Ads 360 reporting API. You must [create a new Search Ads 360 data transfer](https://docs.cloud.google.com/bigquery/docs/search-ads-transfer#setup-data-transfer)
with the new Search Ads 360 reporting API as the data source.

## Review mapping information

Review the following mapping information to map your existing Search Ads 360
resources to the new Search Ads 360 reporting API.

### Report mapping

The new Search Ads 360 reports are based on resources and have a
different structure than reports from the old Search Ads 360. For a
complete mapping of old and new reports, see [Report mappings for the Search Ads 360 reporting API](https://developers.google.com/search-ads/reporting/migrate/mappings/report-mappings).

The following table lists the tables supported by the BigQuery Data Transfer Service along
with the resources queried to generate the tables.

| Old Search Ads Report | New Search Ads Resource | New BigQuery Table Name |
|---|---|---|
| [adGroup](https://developers.google.com/search-ads/v2/report-types/adGroup) | [ad_group](https://developers.google.com/search-ads/reporting/api/reference/fields/v0/ad_group) | p_sa_AdGroupStats_customer_id p_sa_AdGroup_customer_id p_sa_AdGroupDeviceStats_customer_id p_sa_AdGroupConversionActionAndDeviceStats_customer_id |
| [ad](https://developers.google.com/search-ads/v2/report-types/ad) | [ad_group_ad](https://developers.google.com/search-ads/reporting/api/reference/fields/v0/ad_group_ad) | p_sa_AdConversionActionAndDeviceStats_customer_id p_sa_AdDeviceStats_customer_id p_sa_Ad_customer_id |
| N/A | [ad_group_asset](https://developers.google.com/search-ads/reporting/api/reference/fields/v0/ad_group_asset) | p_sa_AdGroupAssetStats_customer_id p_sa_AdGroupConversionActionAndAssetStats_customer_id |
| N/A | [ad_group_asset_set](https://developers.google.com/search-ads/reporting/api/reference/fields/v0/ad_group_asset_set) | p_sa_AdGroupAssetSet_customer_id |
| [adGroupTarget](https://developers.google.com/search-ads/v2/report-types/adGroupTarget) | [ad_group_audience_view](https://developers.google.com/search-ads/reporting/api/reference/fields/v0/ad_group_audience_view) | p_sa_AdGroupAudienceDeviceStats_customer_id p_sa_AdGroupAudienceConversionActionAndDeviceStats_customer_id |
| [adGroupTarget](https://developers.google.com/search-ads/v2/report-types/adGroupTarget) | [ad_group_criterion](https://developers.google.com/search-ads/reporting/api/reference/fields/v0/ad_group_criterion) | p_sa_NegativeAdGroupCriterion_customer_id p_sa_NegativeAdGroupKeyword_customer_id p_sa_AdGroupCriterion_customer_id |
| [adGroupTarget](https://developers.google.com/search-ads/v2/report-types/adGroupTarget) | [age_range_view](https://developers.google.com/search-ads/reporting/api/reference/fields/v0/age_range_view) | p_sa_AgeRangeDeviceStats_customer_id p_sa_AgeRangeConversionActionAndDeviceStats_customer_id |
| N/A | [asset](https://developers.google.com/search-ads/reporting/api/reference/fields/v0/asset) | p_sa_Asset_customer_id |
| [bidStrategy](https://developers.google.com/search-ads/v2/report-types/bidStrategy) | [bidding_strategy](https://developers.google.com/search-ads/reporting/api/reference/fields/v0/bidding_strategy) | p_sa_BidStrategy_customer_id p_sa_BidStrategyStats_customer_id |
| [campaign](https://developers.google.com/search-ads/v2/report-types/campaign) | [campaign](https://developers.google.com/search-ads/reporting/api/reference/fields/v0/campaign) | p_sa_CampaignConversionActionAndDeviceStats_customer_id p_sa_Campaign_customer_id p_sa_CampaignDeviceStats_customer_id p_sa_CampaignStats_customer_id |
| N/A | [campaign_asset](https://developers.google.com/search-ads/reporting/api/reference/fields/v0/campaign_asset) | p_sa_CampaignAssetStats_customer_id p_sa_CampaignConversionActionAndAssetStats_customer_id |
| N/A | [campaign_asset_set](https://developers.google.com/search-ads/reporting/api/reference/fields/v0/campaign_asset_set) | p_sa_CampaignAssetSet_customer_id |
| [campaignTarget](https://developers.google.com/search-ads/v2/report-types/campaignTarget) | [campaign_audience_view](https://developers.google.com/search-ads/reporting/api/reference/fields/v0/campaign_audience_view) | p_sa_CampaignAudienceConversionActionAndDeviceStats_customer_id p_sa_CampaignAudienceDeviceStats_customer_id |
| [campaignTarget](https://developers.google.com/search-ads/v2/report-types/campaignTarget) | [campaign_criterion](https://developers.google.com/search-ads/reporting/api/reference/fields/v0/campaign_criterion) | p_sa_CampaignCriterion_customer_id p_sa_NegativeCampaignKeyword_customer_id p_sa_NegativeCampaignCriterion_customer_id |
| [productLeadAndCrossSell](https://developers.google.com/search-ads/v2/report-types/productLeadAndCrossSell) | [cart_data_sales_view](https://developers.google.com/search-ads/reporting/api/reference/fields/v0/cart_data_sales_view) | p_sa_CartDataSalesStats_customer_id |
| [conversion](https://developers.google.com/search-ads/v2/report-types/conversion) | [conversion](https://developers.google.com/search-ads/reporting/api/reference/fields/v0/conversion) | p_sa_Conversion_customer_id |
| [floodlightActivity](https://developers.google.com/search-ads/v2/report-types/floodlightActivity) | [conversion_action](https://developers.google.com/search-ads/reporting/api/reference/fields/v0/conversion_action) | p_sa_ConversionAction_customer_id |
| [account](https://developers.google.com/search-ads/v2/report-types/account) | [customer](https://developers.google.com/search-ads/reporting/api/reference/fields/v0/customer) | p_sa_Account_customer_id p_sa_AccountDeviceStats_customer_id p_sa_AccountConversionActionAndDeviceStats_customer_id p_sa_AccountStats_customer_id |
| N/A | [customer_asset](https://developers.google.com/search-ads/reporting/api/reference/fields/v0/customer_asset) | p_sa_CustomerAssetStats_customer_id p_sa_CustomerConversionActionAndAssetStats_customer_id |
| N/A | [customer_asset_set](https://developers.google.com/search-ads/reporting/api/reference/fields/v0/customer_asset_set) | p_sa_CustomerAssetSet_customer_id |
| [adGroupTarget](https://developers.google.com/search-ads/v2/report-types/adGroupTarget) | [gender_view](https://developers.google.com/search-ads/reporting/api/reference/fields/v0/gender_view) | p_sa_GenderDeviceStats_customer_id p_sa_GenderConversionActionAndDeviceStats_customer_id |
| [keyword](https://developers.google.com/search-ads/v2/report-types/keyword) | [keyword_view](https://developers.google.com/search-ads/reporting/api/reference/fields/v0/keyword_view) | p_sa_Keyword_customer_id p_sa_KeywordDeviceStats_customer_id p_sa_KeywordStats_customer_id p_sa_KeywordConversionActionAndDeviceStats_customer_id |
| [adGroupTarget](https://developers.google.com/search-ads/v2/report-types/adGroupTarget) | [location_view](https://developers.google.com/search-ads/reporting/api/reference/fields/v0/location_view) | p_sa_LocationDeviceStats_customer_id p_sa_LocationConversionActionAndDeviceStats_customer_id |
| [productAdvertised](https://developers.google.com/search-ads/v2/report-types/productAdvertised) | [shopping_performance_view](https://developers.google.com/search-ads/reporting/api/reference/fields/v0/shopping_performance_view) | p_sa_ProductAdvertised_customer_id p_sa_ProductAdvertisedConversionActionAndDeviceStats_customer_id p_sa_ProductAdvertisedDeviceStats_customer_id |
| [productGroup](https://developers.google.com/search-ads/v2/report-types/productGroup) | [product_group_view](https://developers.google.com/search-ads/reporting/api/reference/fields/v0/product_group_view) | p_sa_ProductGroupStats_customer_id p_sa_ProductGroup_customer_id |
| [visit](https://developers.google.com/search-ads/v2/report-types/visit) | [visit](https://developers.google.com/search-ads/reporting/api/reference/fields/v0/visit) | p_sa_Visit_customer_id |
| [adGroupTarget](https://developers.google.com/search-ads/v2/report-types/adGroupTarget) | [webpage_view](https://developers.google.com/search-ads/reporting/api/reference/fields/v0/webpage_view) | p_sa_WebpageDeviceStats_customer_id p_sa_WebpageConversionActionAndDeviceStats_customer_id |

### Field mapping

The BigQuery Data Transfer Service supports a subset of Search Ads 360
report fields as listed in [Search Ads 360 report transformation](https://docs.cloud.google.com/bigquery/docs/search-ads-transformation).
BigQuery does not support `.` in column names, so all transferred
reports replace `.` with `_`. For example, the field
`ad_group_ad.ad.text_ad.description1` in a Search Ads 360 resource is
transferred to BigQuery as `ad_group_ad_ad_text_ad_description1`.

### ID mapping

Entities in the new Search Ads 360, such as customers, campaigns, and
ad groups,
have a different [ID space](https://developers.google.com/search-ads/v2/how-tos/reporting/id-mapping)
than the old Search Ads 360. For more information about ID
mapping tables for the new Search Ads 360, see [ID mapping tables](https://docs.cloud.google.com/bigquery/docs/search-ads-transfer#id-mapping).

## Examples of migrated queries

The following examples demonstrate how a BigQuery query
might look before and after it is mapped to the new Search Ads 360
reporting API.

Consider the following example query that analyzes Search Ads campaign
performance from the past 30 days using the old Search Ads 360
reporting API.

```googlesql
SELECT
  c.accountId,
  c.campaign,
  C.status,
  SUM(cs.impr) AS Impressions,
  SUM(cs.clicks) AS Clicks,
  (SUM(cs.cost) / 1000000) AS Cost
FROM
  `previous_dataset.Campaign_advertiser_id` c
LEFT JOIN
  `previous_dataset.CampaignStats_advertiser_id` cs
ON
  (c.campaignId = cs.campaignId
  AND cs._DATA_DATE BETWEEN
  DATE_ADD(CURRENT_DATE(), INTERVAL -31 DAY) AND DATE_ADD(CURRENT_DATE(), INTERVAL -1 DAY))
WHERE
  c._DATA_DATE = c._LATEST_DATE
GROUP BY
  1, 2, 3
ORDER BY
  Impressions DESC
```

When mapped to be compatible with the new Search Ads 360 reporting
API, the same query is converted to the following:

```googlesql
SELECT
  c.customer_id,
  c.campaign_name,
  C.campaign_status,
  SUM(cs.metrics_impressions) AS Impressions,
  SUM(cs.metrics_clicks) AS Clicks,
  (SUM(cs.metrics_cost_micros) / 1000000) AS Cost
FROM
  `new_dataset.sa_Campaign_customer_id` c
LEFT JOIN
  `new_dataset.sa_CampaignStats_customer_id` cs
ON
  (c.campaign_id = cs.campaign_id
  AND cs._DATA_DATE BETWEEN
  DATE_ADD(CURRENT_DATE(), INTERVAL -31 DAY) AND DATE_ADD(CURRENT_DATE(), INTERVAL -1 DAY))
WHERE
  c._DATA_DATE = c._LATEST_DATE
GROUP BY
  1, 2, 3
ORDER BY
  Impressions DESC
```

For more examples of queries that are compatible with the new Search Ads 360,
see [Example queries](https://docs.cloud.google.com/bigquery/docs/search-ads-transfer#example_queries).

## What's next

- To learn how to schedule and manage recurring load jobs from Search Ads 360, see [Search Ads 360 transfers](https://docs.cloud.google.com/bigquery/docs/search-ads-transfer).
- To see how you can transform your Search Ads 360 reports, see [Search Ads 360 report transformation](https://docs.cloud.google.com/bigquery/docs/search-ads-transformation).