# Search Ads 360 report transformation

This document describes how you can transform your reports for
Search Ads 360.

To see the Search Ads 360 report transformation that uses the old
Search Ads 360 reporting API, see [Search Ads 360 report transformation (Deprecated)](https://docs.cloud.google.com/bigquery/docs/sa360-transformation).

## Table mapping for Search Ads 360 reports

When your Search Ads 360 reports are transferred to
BigQuery, the reports are transformed into the following
BigQuery tables and views. When you view the tables and views in
BigQuery, the value for <var translate="no">customer_id</var> is your
Search Ads 360 customer ID.

### ad_group

[ad_group](https://developers.google.com/search-ads/reporting/api/reference/fields/v0/ad_group)

Tables:
:
    p_sa_AdGroup_<var translate="no">customer_id</var>  

    p_sa_AdGroupConversionActionAndDeviceStats_<var translate="no">customer_id</var>  

    p_sa_AdGroupDeviceStats_<var translate="no">customer_id</var>  

    p_sa_AdGroupStats_<var translate="no">customer_id</var>

Views:
:
    sa_AdGroup_<var translate="no">customer_id</var>  

    sa_AdGroupConversionActionAndDeviceStats_<var translate="no">customer_id</var>  

    sa_AdGroupDeviceStats_<var translate="no">customer_id</var>  

    sa_AdGroupStats_<var translate="no">customer_id</var>

### ad_group_ad

[ad_group_ad](https://developers.google.com/search-ads/reporting/api/reference/fields/v0/ad_group_ad)

Tables:
:
    p_sa_Ad_<var translate="no">customer_id</var>  

    p_sa_AdConversionActionAndDeviceStats_<var translate="no">customer_id</var>  

    p_sa_AdDeviceStats_<var translate="no">customer_id</var>

Views:
:
    a_Ad_<var translate="no">customer_id</var>  

    sa_AdConversionActionAndDeviceStats_<var translate="no">customer_id</var>  

    sa_AdDeviceStats_<var translate="no">customer_id</var>

### ad_group_asset

[ad_group_asset](https://developers.google.com/search-ads/reporting/api/reference/fields/v0/ad_group_asset)

Tables:
:
    p_sa_AdGroupAssetStats_<var translate="no">customer_id</var>  

    p_sa_AdGroupConversionActionAndAssetStats_<var translate="no">customer_id</var>

Views:
:
    sa_AdGroupAssetStats_<var translate="no">customer_id</var>  

    sa_AdGroupConversionActionAndAssetStats_<var translate="no">customer_id</var>

### ad_group_audience_view

[ad_group_audience_view](https://developers.google.com/search-ads/reporting/api/reference/fields/v0/ad_group_audience_view)

Tables:
:
    p_sa_AdGroupAudienceConversionActionAndDeviceStats_<var translate="no">customer_id</var>  

    p_sa_AdGroupAudienceDeviceStats_<var translate="no">customer_id</var>

Views:
:
    sa_AdGroupAudienceConversionActionAndDeviceStats_<var translate="no">customer_id</var>  

    sa_AdGroupAudienceDeviceStats_<var translate="no">customer_id</var>

### ad_group_criterion

[ad_group_criterion](https://developers.google.com/search-ads/reporting/api/reference/fields/v0/ad_group_criterion)

Tables:
:
    p_sa_AdGroupCriterion_<var translate="no">customer_id</var>  

    p_sa_NegativeAdGroupCriterion_<var translate="no">customer_id</var>  

    p_sa_NegativeAdGroupKeyword_<var translate="no">customer_id</var>

Views:
:
    sa_AdGroupCriterion_<var translate="no">customer_id</var>  

    sa_NegativeAdGroupCriterion_<var translate="no">customer_id</var>  

    sa_NegativeAdGroupKeyword_<var translate="no">customer_id</var>

### ad_group_label

[ad_group_label](https://developers.google.com/search-ads/reporting/api/reference/fields/v0/ad_group_label)

Tables:
:
    p_sa_AdGroupLabel_<var translate="no">customer_id</var>  

Views:
:
    sa_AdGroupLabel_<var translate="no">customer_id</var>  

### age_range_view

[age_range_view](https://developers.google.com/search-ads/reporting/api/reference/fields/v0/age_range_view)

Tables:
:
    p_sa_AgeRangeConversionActionAndDeviceStats_<var translate="no">customer_id</var>  

    p_sa_AgeRangeDeviceStats_<var translate="no">customer_id</var>

Views:
:
    sa_AgeRangeConversionActionAndDeviceStats_<var translate="no">customer_id</var>  

    sa_AgeRangeDeviceStats_<var translate="no">customer_id</var>

### asset

[asset](https://developers.google.com/search-ads/reporting/api/reference/fields/v0/asset)

Tables:
:
    p_sa_Asset_<var translate="no">customer_id</var>

Views:
:
    sa_Asset_<var translate="no">customer_id</var>

### asset_set_asset

[asset_set_asset](https://developers.google.com/search-ads/reporting/api/reference/fields/v0/asset_set_asset)

Tables:
:
    p_sa_AccountConversionActionAndAssetStats_<var translate="no">customer_id</var>  

    p_sa_AssetSetStats_<var translate="no">customer_id</var>

Views:
:
    sa_AccountConversionActionAndAssetStats_<var translate="no">customer_id</var>  

    sa_AssetSetStats_<var translate="no">customer_id</var>

### bidding_strategy

[bidding_strategy](https://developers.google.com/search-ads/reporting/api/reference/fields/v0/bidding_strategy)

Tables:
:
    p_sa_BidStrategy_<var translate="no">customer_id</var>  

    p_sa_BidStrategyStats_<var translate="no">customer_id</var>

Views:
:
    sa_BidStrategy_<var translate="no">customer_id</var>  

    sa_BidStrategyStats_<var translate="no">customer_id</var>

### campaign

[campaign](https://developers.google.com/search-ads/reporting/api/reference/fields/v0/campaign)

Tables:
:
    p_sa_Campaign_<var translate="no">customer_id</var>  

    p_sa_CampaignConversionActionAndDeviceStats_<var translate="no">customer_id</var>  

    p_sa_CampaignDeviceStats_<var translate="no">customer_id</var>  

    p_sa_CampaignStats_<var translate="no">customer_id</var>

Views:
:
    sa_Campaign_<var translate="no">customer_id</var>  

    sa_CampaignConversionActionAndDeviceStats_<var translate="no">customer_id</var>  

    sa_CampaignDeviceStats_<var translate="no">customer_id</var>  

    sa_CampaignStats_<var translate="no">customer_id</var>

### campaign_asset

[campaign_asset](https://developers.google.com/search-ads/reporting/api/reference/fields/v0/campaign_asset)

Tables:
:
    p_sa_CampaignAssetStats_<var translate="no">customer_id</var>  

    p_sa_CampaignConversionActionAndAssetStats_<var translate="no">customer_id</var>

Views:
:
    sa_CampaignAssetStats_<var translate="no">customer_id</var>  

    sa_CampaignConversionActionAndAssetStats_<var translate="no">customer_id</var>

### campaign_audience_view

[campaign_audience_view](https://developers.google.com/search-ads/reporting/api/reference/fields/v0/campaign_audience_view)

Tables:
:
    p_sa_CampaignAudienceConversionActionAndDeviceStats_<var translate="no">customer_id</var>  

    p_sa_CampaignAudienceDeviceStats_<var translate="no">customer_id</var>

Views:
:
    sa_CampaignAudienceConversionActionAndDeviceStats_<var translate="no">customer_id</var>  

    sa_CampaignAudienceDeviceStats_<var translate="no">customer_id</var>

### campaign_criterion

[campaign_criterion](https://developers.google.com/search-ads/reporting/api/reference/fields/v0/campaign_criterion)

Tables:
:
    p_sa_CampaignCriterion_<var translate="no">customer_id</var>  

    p_sa_NegativeCampaignCriterion_<var translate="no">customer_id</var>  

    p_sa_NegativeCampaignKeyword_<var translate="no">customer_id</var>

Views:
:
    sa_CampaignCriterion_<var translate="no">customer_id</var>  

    sa_NegativeCampaignCriterion_<var translate="no">customer_id</var>  

    sa_NegativeCampaignKeyword_<var translate="no">customer_id</var>

### campaign_label

[campaign_label](https://developers.google.com/search-ads/reporting/api/reference/fields/v0/campaign_label)

Tables:
:
    p_sa_CampaignLabel_<var translate="no">customer_id</var>  

Views:
:
    sa_CampaignLabel_<var translate="no">customer_id</var>  

### cart_data_sales_view

[cart_data_sales_view](https://developers.google.com/search-ads/reporting/api/reference/fields/v0/cart_data_sales_view)

Tables:
:
    p_sa_CartDataSalesStats_<var translate="no">customer_id</var>

Views:
:
    sa_CartDataSalesStats_<var translate="no">customer_id</var>

### conversion

[conversion](https://developers.google.com/search-ads/reporting/api/reference/fields/v0/conversion)

Tables:
:
    p_sa_Conversion_<var translate="no">customer_id</var>

Views:
:
    sa_Conversion_<var translate="no">customer_id</var>

### conversion_action

[conversion_action](https://developers.google.com/search-ads/reporting/api/reference/fields/v0/conversion_action)

Tables:
:
    p_sa_ConversionAction_<var translate="no">customer_id</var>

Views:
:
    sa_ConversionAction_<var translate="no">customer_id</var>

### customer

[customer](https://developers.google.com/search-ads/reporting/api/reference/fields/v0/customer)

Tables:
:
    p_sa_Account_<var translate="no">customer_id</var>  

    p_sa_AccountConversionActionAndDeviceStats_<var translate="no">customer_id</var>  

    p_sa_AccountDeviceStats_<var translate="no">customer_id</var>  

    p_sa_AccountStats_<var translate="no">customer_id</var>

Views:
:
    sa_Account_<var translate="no">customer_id</var>  

    sa_AccountConversionActionAndDeviceStats_<var translate="no">customer_id</var>  

    sa_AccountDeviceStats_<var translate="no">customer_id</var>  

    sa_AccountStats_<var translate="no">customer_id</var>

### customer_asset

[customer_asset](https://developers.google.com/search-ads/reporting/api/reference/fields/v0/customer_asset)

Tables:
:
    p_sa_AccountAssetStats_<var translate="no">customer_id</var>

Views:
:
    sa_AccountAssetStats_<var translate="no">customer_id</var>

### gender_view

[gender_view](https://developers.google.com/search-ads/reporting/api/reference/fields/v0/gender_view)

Tables:
:
    p_sa_GenderConversionActionAndDeviceStats_<var translate="no">customer_id</var>  

    p_sa_GenderDeviceStats_<var translate="no">customer_id</var>

Views:
:
    sa_GenderConversionActionAndDeviceStats_<var translate="no">customer_id</var>  

    sa_GenderDeviceStats_<var translate="no">customer_id</var>

### keyword_view

[keyword_view](https://developers.google.com/search-ads/reporting/api/reference/fields/v0/keyword_view)

Tables:
:
    p_sa_Keyword_<var translate="no">customer_id</var>  

    p_sa_KeywordConversionActionAndDeviceStats_<var translate="no">customer_id</var>  

    p_sa_KeywordDeviceStats_<var translate="no">customer_id</var>  

    p_sa_KeywordStats_<var translate="no">customer_id</var>

Views:
:
    sa_Keyword_<var translate="no">customer_id</var>  

    sa_KeywordConversionActionAndDeviceStats_<var translate="no">customer_id</var>  

    sa_KeywordDeviceStats_<var translate="no">customer_id</var>  

    sa_KeywordStats_<var translate="no">customer_id</var>

### location_view

[location_view](https://developers.google.com/search-ads/reporting/api/reference/fields/v0/location_view)

Tables:
:
    p_sa_LocationConversionActionAndDeviceStats_<var translate="no">customer_id</var>  

    p_sa_LocationDeviceStats_<var translate="no">customer_id</var>

Views:
:
    sa_LocationConversionActionAndDeviceStats_<var translate="no">customer_id</var>  

    sa_LocationDeviceStats_<var translate="no">customer_id</var>

### product_group_view

[product_group_view](https://developers.google.com/search-ads/reporting/api/reference/fields/v0/product_group_view)

Tables:
:
    p_sa_ProductGroup_<var translate="no">customer_id</var>  

    p_sa_ProductGroupStats_<var translate="no">customer_id</var>

Views:
:
    sa_ProductGroup_<var translate="no">customer_id</var>  

    sa_ProductGroupStats_<var translate="no">customer_id</var>

### shopping_performance_view

[shopping_performance_view](https://developers.google.com/search-ads/reporting/api/reference/fields/v0/shopping_performance_view)

Tables:
:
    p_sa_ProductAdvertised_<var translate="no">customer_id</var>  

    p_sa_ProductAdvertisedConversionActionAndDeviceStats_<var translate="no">customer_id</var>  

    p_sa_ProductAdvertisedDeviceStats_<var translate="no">customer_id</var>

Views:
:
    sa_ProductAdvertised_<var translate="no">customer_id</var>  

    sa_ProductAdvertisedConversionActionAndDeviceStats_<var translate="no">customer_id</var>  

    sa_ProductAdvertisedDeviceStats_<var translate="no">customer_id</var>

### visit

[visit](https://developers.google.com/search-ads/reporting/api/reference/fields/v0/visit)

Tables:
:
    p_sa_Visit_<var translate="no">customer_id</var>

Views:
:
    sa_Visit_<var translate="no">customer_id</var>

### webpage_view

[webpage_view](https://developers.google.com/search-ads/reporting/api/reference/fields/v0/webpage_view)

Tables:
:
    p_sa_WebpageConversionActionAndDeviceStats_<var translate="no">customer_id</var>  

    p_sa_WebpageDeviceStats_<var translate="no">customer_id</var>

Views:
:
    sa_WebpageConversionActionAndDeviceStats_<var translate="no">customer_id</var>  

    sa_WebpageDeviceStats_<var translate="no">customer_id</var>

## Column details for Search Ads 360 reports

The BigQuery tables created by a Search Ads 360 transfer consist of the following columns (fields):

Search Ads 360 Table Name: Account

Search Ads 360 API Resource: [customer](https://developers.google.com/search-ads/reporting/api/reference/fields/v0/customer)

| Search Ads 360 Field Name | Description |
|---|---|
| customer_account_level | The account level of the customer: Manager, Sub-manager, Associate manager or Service account. |
| customer_account_type | Engine account type. For example: Google Ads, Microsoft Advertising, Yahoo Japan, Baidu, Facebook, Engine Track. |
| customer_creation_time | The timestamp when this customer was created. The timestamp is in the customer's time zone and in "yyyy-MM-dd HH:mm:ss" format. |
| customer_currency_code | The currency in which the account operates. A subset of the currency codes from the ISO 4217 standard is supported. |
| customer_descriptive_name | Optional, non-unique descriptive name of the customer. |
| customer_manager_descriptive_name | The descriptive name of the manager. |
| customer_sub_manager_descriptive_name | The descriptive name of the sub manager. |
| customer_associate_manager_descriptive_name | The descriptive name of the associate manager. |
| customer_engine_id | ID of the account in the external engine account. |
| customer_id | The ID of the customer. |
| customer_manager_id | The customer ID of the manager. |
| customer_sub_manager_id | The customer ID of the sub manager. |
| customer_associate_manager_id | The customer ID of the associate manager. |
| customer_last_modified_time | The datetime when this customer was last modified. The datetime is in the customer's time zone and in "yyyy-MM-dd HH:mm:ss.ssssss" format. |
| customer_status | The status of the customer. |
| customer_time_zone | The local timezone ID of the customer. |

Search Ads 360 Table Name: AccountAssetStats

Search Ads 360 API Resource: [customer_asset](https://developers.google.com/search-ads/reporting/api/reference/fields/v0/customer_asset)

| Search Ads 360 Field Name | Description |
|---|---|
| customer_asset_asset | Required. Immutable. The asset which is linked to the customer. |
| customer_id | The ID of the customer. |
| metrics_average_cpc | The total cost of all clicks divided by the total number of clicks received. |
| metrics_average_cpm | Average cost-per-thousand impressions (CPM). |
| metrics_clicks | The number of clicks. |
| metrics_client_account_conversions | The number of client account conversions. This only includes conversion actions which include_in_client_account_conversions_metric attribute is set to true. If you use conversion-based bidding, your bid strategies will optimize for these conversions. |
| metrics_client_account_conversions_value | The value of client account conversions. This only includes conversion actions which include_in_client_account_conversions_metric attribute is set to true. If you use conversion-based bidding, your bid strategies will optimize for these conversions. |
| metrics_cost_micros | The sum of your cost-per-click (CPC) and cost-per-thousand impressions (CPM) costs during this period. |
| metrics_ctr | The number of clicks your ad receives (Clicks) divided by the number of times your ad is shown (Impressions). |
| metrics_impressions | Count of how often your ad has appeared on a search results page or website on the Google Network. |
| segments_date | Date to which metrics apply, in yyyy-MM-dd format. For example: 2024-04-17 |

Search Ads 360 Table Name: AccountConversionActionAndAssetStats

Search Ads 360 API Resource: [asset_set_asset](https://developers.google.com/search-ads/reporting/api/reference/fields/v0/asset_set_asset)

| Search Ads 360 Field Name | Description |
|---|---|
| asset_set_asset_asset | Immutable. The asset which this asset set asset is linking to. |
| asset_set_asset_asset_set | Immutable. The asset set which this asset set asset is linking to. |
| customer_id | The ID of the customer. |
| metrics_all_conversions | The total number of conversions. This includes all conversions regardless of the value of include_in_conversions_metric. |
| metrics_all_conversions_value | The value of all conversions. |
| metrics_cross_device_conversions | Conversions from when a customer clicks on an ad on one device, then converts on a different device or browser. Cross-device conversions are already included in all_conversions. |
| metrics_cross_device_conversions_value | The sum of the value of cross-device conversions. |
| segments.conversion_action_name | Conversion action name. |
| segments_date | Date to which metrics apply, in yyyy-MM-dd format. For example: 2024-04-17 |

Search Ads 360 Table Name: AccountConversionActionAndDeviceStats

Search Ads 360 API Resource: [customer](https://developers.google.com/search-ads/reporting/api/reference/fields/v0/customer)

| Search Ads 360 Field Name | Description |
|---|---|
| customer_engine_id | ID of the account in the external engine account. |
| customer_id | The ID of the customer. |
| metrics_all_conversions | The total number of conversions. This includes all conversions regardless of the value of include_in_conversions_metric. |
| metrics_all_conversions_value | The value of all conversions. |
| metrics_cross_device_conversions | Conversions from when a customer clicks on an ad on one device, then converts on a different device or browser. Cross-device conversions are already included in all_conversions. |
| metrics_cross_device_conversions_value | The sum of the value of cross-device conversions. |
| segments.conversion_action_name | Conversion action name. |
| segments_date | Date to which metrics apply, in yyyy-MM-dd format. For example: 2024-04-17 |
| segments_device | Device to which metrics apply. |

Search Ads 360 Table Name: AccountDeviceStats

Search Ads 360 API Resource: [customer](https://developers.google.com/search-ads/reporting/api/reference/fields/v0/customer)

| Search Ads 360 Field Name | Description |
|---|---|
| customer_engine_id | ID of the account in the external engine account. |
| customer_id | The ID of the customer. |
| metrics_average_cpc | The total cost of all clicks divided by the total number of clicks received. |
| metrics_average_cpm | Average cost-per-thousand impressions (CPM). |
| metrics_clicks | The number of clicks. |
| metrics_client_account_conversions | The number of client account conversions. This only includes conversion actions which include_in_client_account_conversions_metric attribute is set to true. If you use conversion-based bidding, your bid strategies will optimize for these conversions. |
| metrics_client_account_conversions_value | The value of client account conversions. This only includes conversion actions which include_in_client_account_conversions_metric attribute is set to true. If you use conversion-based bidding, your bid strategies will optimize for these conversions. |
| metrics_client_account_view_through_conversions | The total number of view-through conversions. These happen when a customer sees an image or rich media ad, then later completes a conversion on your site without interacting with (for example, clicking on) another ad. |
| metrics_cost_micros | The sum of your cost-per-click (CPC) and cost-per-thousand impressions (CPM) costs during this period. |
| metrics_ctr | The number of clicks your ad receives (Clicks) divided by the number of times your ad is shown (Impressions). |
| metrics_impressions | Count of how often your ad has appeared on a search results page or website on the Google Network. |
| metrics_visits | Clicks that Search Ads 360 has successfully recorded and forwarded to an advertiser's landing page. |
| segments_date | Date to which metrics apply, in yyyy-MM-dd format. For example: 2024-04-17 |
| segments_device | Device to which metrics apply. |

Search Ads 360 Table Name: AccountStats

Search Ads 360 API Resource: [customer](https://developers.google.com/search-ads/reporting/api/reference/fields/v0/customer)

| Search Ads 360 Field Name | Description |
|---|---|
| customer_engine_id | ID of the account in the external engine account. |
| customer_id | The ID of the customer. |
| metrics_content_budget_lost_impression_share | The estimated percent of times that your ad was eligible to show on the Search Network but didn't because your budget was too low. Note: Search budget lost impression share is reported in the range of 0 to 0.9. Any value above 0.9 is reported as 0.9001. |
| metrics_content_impression_share | The impressions you've received on the Display Network divided by the estimated number of impressions you were eligible to receive. Note: Content impression share is reported in the range of 0.1 to 1. Any value below 0.1 is reported as 0.0999. |
| metrics_content_rank_lost_impression_share | The estimated percentage of impressions on the Display Network that your ads didn't receive due to poor Ad Rank. Note: Content rank lost impression share is reported in the range of 0 to 0.9. Any value above 0.9 is reported as 0.9001. |
| metrics_historical_quality_score | The historical quality score. |
| metrics_search_budget_lost_impression_share | The estimated percent of times that your ad was eligible to show on the Search Network but didn't because your budget was too low. Note: Search budget lost impression share is reported in the range of 0 to 0.9. Any value above 0.9 is reported as 0.9001. |
| metrics_search_impression_share | The impressions you've received on the Search Network divided by the estimated number of impressions you were eligible to receive. Note: Search impression share is reported in the range of 0.1 to 1. Any value below 0.1 is reported as 0.0999. |
| metrics_search_rank_lost_impression_share | The estimated percentage of impressions on the Search Network that your ads didn't receive due to poor Ad Rank. Note: Search rank lost impression share is reported in the range of 0 to 0.9. Any value above 0.9 is reported as 0.9001. |
| segments_date | Date to which metrics apply, in yyyy-MM-dd format. For example: 2024-04-17 |

Search Ads 360 Table Name: Ad

Search Ads 360 API Resource: [ad_group_ad](https://developers.google.com/search-ads/reporting/api/reference/fields/v0/ad_group_ad)

| Search Ads 360 Field Name | Description |
|---|---|
| ad_group_ad_ad_display_url | The URL that appears in the ad description for some ad formats. |
| ad_group_ad_ad_expanded_dynamic_search_ad_description1 | The first line of the ad's description. |
| ad_group_ad_ad_expanded_dynamic_search_ad_description2 | The second line of the ad's description. |
| ad_group_ad_ad_expanded_text_ad_description1 | The first line of the ad's description. |
| ad_group_ad_ad_expanded_text_ad_description2 | The second line of the ad's description. |
| ad_group_ad_ad_expanded_text_ad_headline | The headline of the ad. |
| ad_group_ad_ad_expanded_text_ad_headline2 | The second headline of the ad. |
| ad_group_ad_ad_expanded_text_ad_headline3 | The third headline of the ad. |
| ad_group_ad_ad_final_urls | The list of possible final URLs after all cross-domain redirects for the ad. |
| ad_group_ad_ad_id | The ID of the ad. |
| ad_group_ad_ad_name | The name of the ad. This is only used to be able to identify the ad. It does not need to be unique and does not affect the served ad. The name field is only supported for DisplayUploadAd, ImageAd, ShoppingComparisonListingAd and VideoAd. |
| ad_group_ad_ad_text_ad_description1 | The first line of the ad's description. |
| ad_group_ad_ad_text_ad_description2 | The second line of the ad's description. |
| ad_group_ad_ad_text_ad_headline | The headline of the ad. |
| ad_group_ad_ad_type | The type of ad. |
| ad_group_ad_creation_time | The timestamp when this ad group ad was created. The datetime is in the customer's time zone and in "yyyy-MM-dd HH:mm:ss.ssssss" format. |
| ad_group_ad_engine_id | ID of the ad in the external engine account. This field is for SearchAds 360 account only. For example: Yahoo Japan, Microsoft, Baidu. For non-SearchAds 360 entity, use "ad_group_ad.ad.id" instead. |
| ad_group_ad_engine_status | Additional status of the ad in the external engine account. Possible statuses (depending on the type of external account) include active, eligible, pending review. |
| ad_group_ad_labels | The resource names of labels attached to this ad group ad. |
| ad_group_ad_last_modified_time | The datetime when this ad group ad was last modified. The datetime is in the customer's time zone and in "yyyy-MM-dd HH:mm:ss.ssssss" format. |
| ad_group_ad_status | The status of the ad. |
| ad_group_id | The ID of the ad group. |
| campaign_id | The ID of the campaign. |
| customer_id | The ID of the customer. |

Search Ads 360 Table Name: AdConversionActionAndDeviceStats

Search Ads 360 API Resource: [ad_group_ad](https://developers.google.com/search-ads/reporting/api/reference/fields/v0/ad_group_ad)

| Search Ads 360 Field Name | Description |
|---|---|
| ad_group_ad_ad_id | The ID of the ad. |
| ad_group_ad_engine_id | ID of the ad in the external engine account. This field is for SearchAds 360 account only. For example: Yahoo Japan, Microsoft, Baidu. For non-SearchAds 360 entity, use "ad_group_ad.ad.id" instead. |
| ad_group_id | The ID of the ad group. |
| campaign_id | The ID of the campaign. |
| customer_id | The ID of the customer. |
| metrics_all_conversions | The total number of conversions. This includes all conversions regardless of the value of include_in_conversions_metric. |
| metrics_all_conversions_by_conversion_date | The total number of conversions. This includes all conversions regardless of the value of include_in_conversions_metric. When this column is selected with date, the values in date column means the conversion date. For more information about `by_conversion_date` columns, see [About the "All conversions" column](https://support.google.com/sa360/answer/9250611). |
| metrics_all_conversions_value | The value of all conversions. |
| metrics_all_conversions_value_by_conversion_date | The value of all conversions. When this column is selected with date, the values in date column means the conversion date. For more information about `by_conversion_date` columns, see [About the "All conversions" column](https://support.google.com/sa360/answer/9250611). |
| metrics_cross_device_conversions | Conversions from when a customer clicks on an ad on one device, then converts on a different device or browser. Cross-device conversions are already included in all_conversions. |
| metrics_cross_device_conversions_by_conversion_date | The number of cross-device conversions by conversion date. For more information about `by_conversion_date` columns, see [About the "All conversions" column](https://support.google.com/sa360/answer/9250611). |
| metrics_cross_device_conversions_value | The sum of the value of cross-device conversions. |
| metrics_cross_device_conversions_value_by_conversion_date | The sum of cross-device conversions value by conversion date. For more information about `by_conversion_date` columns, see [About the "All conversions" column](https://support.google.com/sa360/answer/9250611). |
| segments.conversion_action_name | Conversion action name. |
| segments_date | Date to which metrics apply, in yyyy-MM-dd format. For example: 2024-04-17 |
| segments_device | Device to which metrics apply. |

Search Ads 360 Table Name: AdDeviceStats

Search Ads 360 API Resource: [ad_group_ad](https://developers.google.com/search-ads/reporting/api/reference/fields/v0/ad_group_ad)

| Search Ads 360 Field Name | Description |
|---|---|
| ad_group_ad_ad_id | The ID of the ad. |
| ad_group_ad_engine_id | ID of the ad in the external engine account. This field is for SearchAds 360 account only. For example: Yahoo Japan, Microsoft, Baidu. For non-SearchAds 360 entity, use "ad_group_ad.ad.id" instead. |
| ad_group_id | The ID of the ad group. |
| campaign_id | The ID of the campaign. |
| customer_id | The ID of the customer. |
| metrics_average_cpc | The total cost of all clicks divided by the total number of clicks received. |
| metrics_average_cpm | Average cost-per-thousand impressions (CPM). |
| metrics_clicks | The number of clicks. |
| metrics_client_account_conversions | The number of client account conversions. This only includes conversion actions which include_in_client_account_conversions_metric attribute is set to true. If you use conversion-based bidding, your bid strategies will optimize for these conversions. |
| metrics_client_account_conversions_value | The value of client account conversions. This only includes conversion actions which include_in_client_account_conversions_metric attribute is set to true. If you use conversion-based bidding, your bid strategies will optimize for these conversions. |
| metrics_client_account_view_through_conversions | The total number of view-through conversions. These happen when a customer sees an image or rich media ad, then later completes a conversion on your site without interacting with (for example, clicking on) another ad. |
| metrics_cost_micros | The sum of your cost-per-click (CPC) and cost-per-thousand impressions (CPM) costs during this period. |
| metrics_ctr | The number of clicks your ad receives (Clicks) divided by the number of times your ad is shown (Impressions). |
| metrics_impressions | Count of how often your ad has appeared on a search results page or website on the Google Network. |
| metrics_visits | Clicks that Search Ads 360 has successfully recorded and forwarded to an advertiser's landing page. |
| segments_date | Date to which metrics apply, in yyyy-MM-dd format. For example: 2024-04-17 |
| segments_device | Device to which metrics apply. |

Search Ads 360 Table Name: AdGroup

Search Ads 360 API Resource: [ad_group](https://developers.google.com/search-ads/reporting/api/reference/fields/v0/ad_group)

| Search Ads 360 Field Name | Description |
|---|---|
| ad_group_bid_modifier_bid_modifier | The modifier for the bid when the criterion matches. The modifier must be in the range: 0.1 - 10.0. The range is 1.0 - 6.0 for PreferredContent. Use 0 to opt out of a Device type. |
| ad_group_bid_modifier_device_type | Type of the device. |
| ad_group_cpc_bid_micros | The maximum CPC (cost-per-click) bid. |
| ad_group_creation_time | The timestamp when this ad group was created. The timestamp is in the customer's time zone and in "yyyy-MM-dd HH:mm:ss" format. |
| ad_group_end_date | Date when the ad group ends serving ads. By default, the ad group ends on the ad group's end date. If this field is set, then the ad group ends at the end of the specified date in the customer's time zone. This field is only available for Microsoft Advertising and Facebook gateway accounts. Format: YYYY-MM-DD Example: 2019-03-14 |
| ad_group_engine_id | ID of the ad group in the external engine account. This field is for non-Google Ads account only. For example: Yahoo Japan, Microsoft, Baidu. For Google Ads entity, use "ad_group.id" instead. |
| ad_group_engine_status | The Engine Status for ad group. |
| ad_group_id | The ID of the ad group. |
| ad_group_labels | The resource names of labels attached to this ad group. |
| ad_group_language_code | The language of the ads and keywords in an ad group. This field is only available for Microsoft Advertising accounts. |
| ad_group_last_modified_time | The datetime when this ad group was last modified. The datetime is in the customer's time zone and in "yyyy-MM-dd HH:mm:ss.ssssss" format. |
| ad_group_name | The name of the ad group. This field is required and should not be empty when creating new ad groups. It must contain fewer than 255 UTF-8 full-width characters. It must not contain any null (code point 0x0), NL line feed (code point 0xA) or carriage return (code point 0xD) characters. |
| ad_group_start_date | Date when this ad group starts serving ads. By default, the ad group starts now or the ad group's start date, whichever is later. If this field is set, then the ad group starts at the beginning of the specified date in the customer's time zone. This field is only available for Microsoft Advertising and Facebook gateway accounts. Format: YYYY-MM-DD Example: 2019-03-14 |
| ad_group_status | The status of the ad group. |
| ad_group_targeting_setting_target_restrictions | The per-targeting-dimension setting to restrict the reach of your campaign or ad group. |
| bidding_strategy_id | The ID of the bidding strategy. |
| campaign_id | The ID of the campaign. |
| customer_id | The ID of the customer. |

Search Ads 360 Table Name: AdGroupAssetStats

Search Ads 360 API Resource: [ad_group_asset](https://developers.google.com/search-ads/reporting/api/reference/fields/v0/ad_group_asset)

| Search Ads 360 Field Name | Description |
|---|---|
| ad_group_asset_ad_group | Required. Immutable. The ad group to which the asset is linked. |
| ad_group_asset_asset | Required. Immutable. The asset which is linked to the ad group. |
| ad_group_engine_id | ID of the ad group in the external engine account. This field is for non-Google Ads account only. For example: Yahoo Japan, Microsoft, Baidu. For Google Ads entity, use "ad_group.id" instead. |
| ad_group_id | The ID of the ad group. |
| campaign_id | The ID of the campaign. |
| customer_id | The ID of the customer. |
| metrics_average_cpc | The total cost of all clicks divided by the total number of clicks received. |
| metrics_average_cpm | Average cost-per-thousand impressions (CPM). |
| metrics_clicks | The number of clicks. |
| metrics_client_account_conversions | The number of client account conversions. This only includes conversion actions which include_in_client_account_conversions_metric attribute is set to true. If you use conversion-based bidding, your bid strategies will optimize for these conversions. |
| metrics_client_account_conversions_value | The value of client account conversions. This only includes conversion actions which include_in_client_account_conversions_metric attribute is set to true. If you use conversion-based bidding, your bid strategies will optimize for these conversions. |
| metrics_cost_micros | The sum of your cost-per-click (CPC) and cost-per-thousand impressions (CPM) costs during this period. |
| metrics_ctr | The number of clicks your ad receives (Clicks) divided by the number of times your ad is shown (Impressions). |
| metrics_impressions | Count of how often your ad has appeared on a search results page or website on the Google Network. |
| segments_date | Date to which metrics apply, in yyyy-MM-dd format. For example: 2024-04-17 |

Search Ads 360 Table Name: AdGroupAudienceConversionActionAndDeviceStats

Search Ads 360 API Resource: [ad_group_audience_view](https://developers.google.com/search-ads/reporting/api/reference/fields/v0/ad_group_audience_view)

| Search Ads 360 Field Name | Description |
|---|---|
| ad_group_criterion_age_range_type | Type of the age range. |
| ad_group_criterion_bid_modifier | The modifier for the bid when the criterion matches. The modifier must be in the range: 0.1 - 10.0. Most targetable criteria types support modifiers. |
| ad_group_criterion_cpc_bid_micros | The CPC (cost-per-click) bid. |
| ad_group_criterion_criterion_id | The ID of the criterion. |
| ad_group_criterion_gender_type | Type of the gender. |
| ad_group_criterion_location_geo_target_constant | The geo target constant resource name. |
| ad_group_criterion_user_list_user_list | The User List resource name. |
| ad_group_criterion_webpage_conditions | Conditions, or logical expressions, for webpage targeting. The list of webpage targeting conditions are and-ed together when evaluated for targeting. An empty list of conditions indicates all pages of the campaign's website are targeted. This field is required for CREATE operations and is prohibited on UPDATE operations. |
| ad_group_criterion_webpage_coverage_percentage | Website criteria coverage percentage. This is the computed percentage of website coverage based on the website target, negative website target and negative keywords in the ad group and campaign. For instance, when coverage returns as 1, it indicates it has 100% coverage. |
| ad_group_id | The ID of the ad group. |
| campaign_id | The ID of the campaign. |
| customer_id | The ID of the customer. |
| metrics_cross_device_conversions | Conversions from when a customer clicks on an ad on one device, then converts on a different device or browser. Cross-device conversions are already included in all_conversions. |
| metrics_cross_device_conversions_value | The sum of the value of cross-device conversions. |
| segments.conversion_action_name | Conversion action name. |
| segments_date | Date to which metrics apply, in yyyy-MM-dd format. For example: 2024-04-17. |
| segments_device | Device to which metrics apply. |

Search Ads 360 Table Name: AdGroupAudienceDeviceStats

Search Ads 360 API Resource: [ad_group_audience_view](https://developers.google.com/search-ads/reporting/api/reference/fields/v0/ad_group_audience_view)

| Search Ads 360 Field Name | Description |
|---|---|
| ad_group_criterion_age_range_type | Type of the age range. |
| ad_group_criterion_bid_modifier | The modifier for the bid when the criterion matches. The modifier must be in the range: 0.1 - 10.0. Most targetable criteria types support modifiers. |
| ad_group_criterion_cpc_bid_micros | The CPC (cost-per-click) bid. |
| ad_group_criterion_criterion_id | The ID of the criterion. |
| ad_group_criterion_gender_type | Type of the gender. |
| ad_group_criterion_location_geo_target_constant | The geo target constant resource name. |
| ad_group_criterion_user_list_user_list | The User List resource name. |
| ad_group_criterion_webpage_conditions | Conditions, or logical expressions, for webpage targeting. The list of webpage targeting conditions are and-ed together when evaluated for targeting. An empty list of conditions indicates all pages of the campaign's website are targeted. This field is required for CREATE operations and is prohibited on UPDATE operations. |
| ad_group_criterion_webpage_coverage_percentage | Website criteria coverage percentage. This is the computed percentage of website coverage based on the website target, negative website target and negative keywords in the ad group and campaign. For instance, when coverage returns as 1, it indicates it has 100% coverage. |
| ad_group_id | The ID of the ad group. |
| campaign_id | The ID of the campaign. |
| customer_id | The ID of the customer. |
| metrics_average_cpc | The total cost of all clicks divided by the total number of clicks received. |
| metrics_average_cpm | Average cost-per-thousand impressions (CPM). |
| metrics_clicks | The number of clicks. |
| metrics_client_account_conversions | The number of client account conversions. This only includes conversion actions which include_in_client_account_conversions_metric attribute is set to true. If you use conversion-based bidding, your bid strategies will optimize for these conversions. |
| metrics_client_account_conversions_value | The value of client account conversions. This only includes conversion actions which include_in_client_account_conversions_metric attribute is set to true. If you use conversion-based bidding, your bid strategies will optimize for these conversions. |
| metrics_client_account_view_through_conversions | The total number of view-through conversions. These happen when a customer sees an image or rich media ad, then later completes a conversion on your site without interacting with (for example, clicking on) another ad. |
| metrics_cost_micros | The sum of your cost-per-click (CPC) and cost-per-thousand impressions (CPM) costs during this period. |
| metrics_ctr | The number of clicks your ad receives (Clicks) divided by the number of times your ad is shown (Impressions). |
| metrics_impressions | Count of how often your ad has appeared on a search results page or website on the Google Network. |
| segments_date | Date to which metrics apply, in yyyy-MM-dd format. For example: 2024-04-17 |
| segments_device | Device to which metrics apply. |

Search Ads 360 Table Name: AdGroupConversionActionAndAssetStats

Search Ads 360 API Resource: [ad_group_asset](https://developers.google.com/search-ads/reporting/api/reference/fields/v0/ad_group_asset)

| Search Ads 360 Field Name | Description |
|---|---|
| ad_group_asset_ad_group | Required. Immutable. The ad group to which the asset is linked. |
| ad_group_asset_asset | Required. Immutable. The asset which is linked to the ad group. |
| ad_group_engine_id | ID of the ad group in the external engine account. This field is for non-Google Ads account only. For example: Yahoo Japan, Microsoft, Baidu. For Google Ads entity, use "ad_group.id" instead. |
| ad_group_id | The ID of the ad group. |
| campaign_id | The ID of the campaign. |
| customer_id | The ID of the customer. |
| metrics_all_conversions | The total number of conversions. This includes all conversions regardless of the value of include_in_conversions_metric. |
| metrics_all_conversions_value | The value of all conversions. |
| metrics_cross_device_conversions | Conversions from when a customer clicks on an ad on one device, then converts on a different device or browser. Cross-device conversions are already included in all_conversions. |
| metrics_cross_device_conversions_value | The sum of the value of cross-device conversions. |
| segments.conversion_action_name | Conversion action name. |
| segments_date | Date to which metrics apply, in yyyy-MM-dd format. For example: 2018-04-17 |

Search Ads 360 Table Name: AdGroupConversionActionAndDeviceStats

Search Ads 360 API Resource: [ad_group](https://developers.google.com/search-ads/reporting/api/reference/fields/v0/ad_group)

| Search Ads 360 Field Name | Description |
|---|---|
| ad_group_engine_id | Output only. ID of the ad group in the external engine account. This field is for non-Google Ads account only. For example: Yahoo Japan, Microsoft, Baidu. For Google Ads entity, use "ad_group.id" instead. |
| ad_group_id | Output only. The ID of the ad group. |
| bidding_strategy_id | Output only. The ID of the bidding strategy. |
| campaign_id | Output only. The ID of the campaign. |
| customer_id | Output only. The ID of the customer. |
| metrics_all_conversions | The total number of conversions. This includes all conversions regardless of the value of include_in_conversions_metric. |
| metrics_all_conversions_value | The value of all conversions. |
| metrics_cross_device_conversions | Conversions from when a customer clicks on an ad on one device, then converts on a different device or browser. Cross-device conversions are already included in all_conversions. |
| metrics_cross_device_conversions_value | The sum of the value of cross-device conversions. |
| segments.conversion_action_name | Conversion action name. |
| segments_date | Date to which metrics apply, in yyyy-MM-dd format. For example: 2024-04-17 |
| segments_device | Device to which metrics apply. |

Search Ads 360 Table Name: AdGroupCriterion

Search Ads 360 API Resource: [ad_group_criterion](https://developers.google.com/search-ads/reporting/api/reference/fields/v0/ad_group_criterion)

| Search Ads 360 Field Name | Description |
|---|---|
| ad_group_criterion_age_range_type | Type of the age range. |
| ad_group_criterion_bid_modifier | The modifier for the bid when the criterion matches. The modifier must be in the range: 0.1 - 10.0. Most targetable criteria types support modifiers. |
| ad_group_criterion_cpc_bid_micros | The CPC (cost-per-click) bid. |
| ad_group_criterion_creation_time | The timestamp when this ad group criterion was created. The timestamp is in the customer's time zone and in "yyyy-MM-dd HH:mm:ss" format. |
| ad_group_criterion_criterion_id | The ID of the criterion. |
| ad_group_criterion_gender_type | Type of the gender. |
| ad_group_criterion_last_modified_time | The datetime when this ad group criterion was last modified. The datetime is in the customer's time zone and in "yyyy-MM-dd HH:mm:ss.ssssss" format. |
| ad_group_criterion_location_geo_target_constant | The geo target constant resource name. |
| ad_group_criterion_status | The status of the criterion. This is the status of the ad group criterion entity, set by the client. Note: UI reports may incorporate additional information that affects whether a criterion is eligible to run. In some cases a criterion that's REMOVED in the API can still show as enabled in the UI. For example, campaigns by default show to users of all age ranges unless excluded. The UI will show each age range as "enabled", since they're eligible to see the ads; but AdGroupCriterion.status will show "removed", since no positive criterion was added. |
| ad_group_criterion_type | The type of the criterion. |
| ad_group_criterion_user_list_user_list | The User List resource name. |
| ad_group_criterion_webpage_conditions | Conditions, or logical expressions, for webpage targeting. The list of webpage targeting conditions are and-ed together when evaluated for targeting. An empty list of conditions indicates all pages of the campaign's website are targeted. This field is required for CREATE operations and is prohibited on UPDATE operations. |
| ad_group_criterion_webpage_coverage_percentage | Website criteria coverage percentage. This is the computed percentage of website coverage based on the website target, negative website target and negative keywords in the ad group and campaign. For instance, when coverage returns as 1, it indicates it has 100% coverage. |
| ad_group_id | The ID of the ad group. |
| ad_group_name | The name of the ad group. This field is required and should not be empty when creating new ad groups. It must contain fewer than 255 UTF-8 full-width characters. It must not contain any null (code point 0x0), NL line feed (code point 0xA) or carriage return (code point 0xD) characters. |
| ad_group_status | The status of the ad group. |
| campaign_id | The ID of the campaign. |
| campaign_name | The name of the campaign. This field is required and should not be empty when creating new campaigns. It must not contain any null (code point 0x0), NL line feed (code point 0xA) or carriage return (code point 0xD) characters. |
| campaign_status | The status of the campaign. |
| customer_account_type | Engine account type. For example: Google Ads, Microsoft Advertising, Yahoo Japan, Baidu, Facebook, Engine Track. |
| customer_descriptive_name | Optional, non-unique descriptive name of the customer. |
| customer_id | The ID of the customer. |

Search Ads 360 Table Name: AdGroupDeviceStats

Search Ads 360 API Resource: [ad_group](https://developers.google.com/search-ads/reporting/api/reference/fields/v0/ad_group)

| Search Ads 360 Field Name | Description |
|---|---|
| ad_group_engine_id | ID of the ad group in the external engine account. This field is for non-Google Ads account only. For example: Yahoo Japan, Microsoft, Baidu. For Google Ads entity, use "ad_group.id" instead. |
| ad_group_id | The ID of the ad group. |
| bidding_strategy_id | The ID of the bidding strategy. |
| campaign_id | The ID of the campaign. |
| customer_id | The ID of the customer. |
| metrics_average_cpc | The total cost of all clicks divided by the total number of clicks received. |
| metrics_average_cpm | Average cost-per-thousand impressions (CPM). |
| metrics_clicks | The number of clicks. |
| metrics_client_account_conversions | The number of client account conversions. This only includes conversion actions which include_in_client_account_conversions_metric attribute is set to true. If you use conversion-based bidding, your bid strategies will optimize for these conversions. |
| metrics_client_account_conversions_value | The value of client account conversions. This only includes conversion actions which include_in_client_account_conversions_metric attribute is set to true. If you use conversion-based bidding, your bid strategies will optimize for these conversions. |
| metrics_client_account_view_through_conversions | The total number of view-through conversions. These happen when a customer sees an image or rich media ad, then later completes a conversion on your site without interacting with (for example, clicking on) another ad. |
| metrics_cost_micros | The sum of your cost-per-click (CPC) and cost-per-thousand impressions (CPM) costs during this period. |
| metrics_ctr | The number of clicks your ad receives (Clicks) divided by the number of times your ad is shown (Impressions). |
| metrics_impressions | Count of how often your ad has appeared on a search results page or website on the Google Network. |
| metrics_visits | Clicks that Search Ads 360 has successfully recorded and forwarded to an advertiser's landing page. |
| segments_date | Date to which metrics apply, in yyyy-MM-dd format. For example: 2024-04-17 |
| segments_device | Device to which metrics apply. |

Search Ads 360 Table Name: AdGroupLabel

Search Ads 360 API Resource: [ad_group_label](https://developers.google.com/search-ads/reporting/api/reference/fields/v0/ad_group_label)

| Search Ads 360 Field Name | Description |
|---|---|
| ad_group_label_ad_group | The ad group to which the label is attached. |
| ad_group_label_label | The label assigned to the ad group. |
| ad_group_label_owner_customer_id | The ID of the Customer which owns the label. |
| ad_group_label_resource_name | The resource name of the ad group label. Ad group label resource names have the form: customers/{customer_id}/adGroupLabels/{ad_group_id}\~{label_id} |
| campaign_id | The ID of the campaign. |
| customer_id | The ID of the customer. |

Search Ads 360 Table Name: AdGroupStats

Search Ads 360 API Resource: [ad_group](https://developers.google.com/search-ads/reporting/api/reference/fields/v0/ad_group)

| Search Ads 360 Field Name | Description |
|---|---|
| ad_group_engine_id | ID of the ad group in the external engine account. This field is for non-Google Ads account only. For example: Yahoo Japan, Microsoft, Baidu. For Google Ads entity, use "ad_group.id" instead. |
| ad_group_id | The ID of the ad group. |
| bidding_strategy_id | The ID of the bidding strategy. |
| campaign_id | The ID of the campaign. |
| customer_id | The ID of the customer. |
| metrics_content_impression_share | The impressions you've received on the Display Network divided by the estimated number of impressions you were eligible to receive. Note: Content impression share is reported in the range of 0.1 to 1. Any value below 0.1 is reported as 0.0999. |
| metrics_content_rank_lost_impression_share | The estimated percentage of impressions on the Display Network that your ads didn't receive due to poor Ad Rank. Note: Content rank lost impression share is reported in the range of 0 to 0.9. Any value above 0.9 is reported as 0.9001. |
| metrics_historical_quality_score | The historical quality score. |
| metrics_search_impression_share | The impressions you've received on the Search Network divided by the estimated number of impressions you were eligible to receive. Note: Search impression share is reported in the range of 0.1 to 1. Any value below 0.1 is reported as 0.0999. |
| metrics_search_rank_lost_impression_share | The estimated percentage of impressions on the Search Network that your ads didn't receive due to poor Ad Rank. Note: Search rank lost impression share is reported in the range of 0 to 0.9. Any value above 0.9 is reported as 0.9001. |
| segments_date | Date to which metrics apply, in yyyy-MM-dd format. For example: 2024-04-17. |

Search Ads 360 Table Name: AgeRangeConversionActionAndDeviceStats

Search Ads 360 API Resource: [age_range_view](https://developers.google.com/search-ads/reporting/api/reference/fields/v0/age_range_view)

| Search Ads 360 Field Name | Description |
|---|---|
| ad_group_criterion_age_range_type | Type of the age range. |
| ad_group_criterion_bid_modifier | The modifier for the bid when the criterion matches. The modifier must be in the range: 0.1 - 10.0. Most targetable criteria types support modifiers. |
| ad_group_criterion_cpc_bid_micros | The CPC (cost-per-click) bid. |
| ad_group_criterion_criterion_id | The ID of the criterion. |
| ad_group_criterion_gender_type | Type of the gender. |
| ad_group_criterion_location_geo_target_constant | The geo target constant resource name. |
| ad_group_criterion_user_list_user_list | The User List resource name. |
| ad_group_criterion_webpage_conditions | Conditions, or logical expressions, for webpage targeting. The list of webpage targeting conditions are and-ed together when evaluated for targeting. An empty list of conditions indicates all pages of the campaign's website are targeted. This field is required for CREATE operations and is prohibited on UPDATE operations. |
| ad_group_criterion_webpage_coverage_percentage | Website criteria coverage percentage. This is the computed percentage of website coverage based on the website target, negative website target and negative keywords in the ad group and campaign. For instance, when coverage returns as 1, it indicates it has 100% coverage. |
| ad_group_id | The ID of the ad group. |
| campaign_id | The ID of the campaign. |
| customer_id | The ID of the customer. |
| metrics_cross_device_conversions | Conversions from when a customer clicks on an ad on one device, then converts on a different device or browser. Cross-device conversions are already included in all_conversions. |
| metrics_cross_device_conversions_value | The sum of the value of cross-device conversions. |
| segments.conversion_action_name | Conversion action name. |
| segments_date | Date to which metrics apply, in yyyy-MM-dd format. For example: 2024-04-17 |
| segments_device | Device to which metrics apply. |

Search Ads 360 Table Name: AgeRangeDeviceStats

Search Ads 360 API Resource: [age_range_view](https://developers.google.com/search-ads/reporting/api/reference/fields/v0/age_range_view)

| Search Ads 360 Field Name | Description |
|---|---|
| ad_group_criterion_age_range_type | Type of the age range. |
| ad_group_criterion_bid_modifier | The modifier for the bid when the criterion matches. The modifier must be in the range: 0.1 - 10.0. Most targetable criteria types support modifiers. |
| ad_group_criterion_cpc_bid_micros | The CPC (cost-per-click) bid. |
| ad_group_criterion_criterion_id | The ID of the criterion. |
| ad_group_criterion_gender_type | Type of the gender. |
| ad_group_criterion_location_geo_target_constant | The geo target constant resource name. |
| ad_group_criterion_user_list_user_list | The User List resource name. |
| ad_group_criterion_webpage_conditions | Conditions, or logical expressions, for webpage targeting. The list of webpage targeting conditions are and-ed together when evaluated for targeting. An empty list of conditions indicates all pages of the campaign's website are targeted. This field is required for CREATE operations and is prohibited on UPDATE operations. |
| ad_group_criterion_webpage_coverage_percentage | Website criteria coverage percentage. This is the computed percentage of website coverage based on the website target, negative website target and negative keywords in the ad group and campaign. For instance, when coverage returns as 1, it indicates it has 100% coverage. |
| ad_group_id | The ID of the ad group. |
| campaign_id | The ID of the campaign. |
| customer_id | The ID of the customer. |
| metrics_average_cpc | The total cost of all clicks divided by the total number of clicks received. |
| metrics_average_cpm | Average cost-per-thousand impressions (CPM). |
| metrics_clicks | The number of clicks. |
| metrics_client_account_conversions | The number of client account conversions. This only includes conversion actions which include_in_client_account_conversions_metric attribute is set to true. If you use conversion-based bidding, your bid strategies will optimize for these conversions. |
| metrics_client_account_conversions_value | The value of client account conversions. This only includes conversion actions which include_in_client_account_conversions_metric attribute is set to true. If you use conversion-based bidding, your bid strategies will optimize for these conversions. |
| metrics_client_account_view_through_conversions | The total number of view-through conversions. These happen when a customer sees an image or rich media ad, then later completes a conversion on your site without interacting with (for example, clicking on) another ad. |
| metrics_cost_micros | The sum of your cost-per-click (CPC) and cost-per-thousand impressions (CPM) costs during this period. |
| metrics_ctr | The number of clicks your ad receives (Clicks) divided by the number of times your ad is shown (Impressions). |
| metrics_impressions | Count of how often your ad has appeared on a search results page or website on the Google Network. |
| segments_date | Date to which metrics apply, in yyyy-MM-dd format. For example: 2024-04-17 |
| segments_device | Device to which metrics apply. |

Search Ads 360 Table Name: Asset

Search Ads 360 API Resource: [asset](https://developers.google.com/search-ads/reporting/api/reference/fields/v0/asset)

| Search Ads 360 Field Name | Description |
|---|---|
| asset_creation_time | The timestamp when this asset was created. The timestamp is in the customer's time zone and in "yyyy-MM-dd HH:mm:ss" format. |
| asset_engine_status | The Engine Status of the asset. |
| asset_final_urls | A list of possible final URLs after all cross domain redirects. |
| asset_id | The ID of the asset. |
| asset_last_modified_time | The datetime when this asset was last modified. The datetime is in the customer's time zone and in "yyyy-MM-dd HH:mm:ss.ssssss" format. |
| asset_sitelink_asset.description1 | First line of the description for the sitelink. If set, the length should be between 1 and 35, inclusive, and description2 must also be set. |
| asset_sitelink_asset.description2 | Second line of the description for the sitelink. If set, the length should be between 1 and 35, inclusive, and description1 must also be set. |
| asset_sitelink_asset_link_text | URL display text for the sitelink. The length of this string should be between 1 and 25, inclusive. |
| asset_status | The status of the asset. |
| asset_tracking_url_template | URL template for constructing a tracking URL. |
| customer_id | The ID of the customer. |

Search Ads 360 Table Name: AssetSetStats

Search Ads 360 API Resource: [asset_set_asset](https://developers.google.com/search-ads/reporting/api/reference/fields/v0/asset_set_asset)

| Search Ads 360 Field Name | Description |
|---|---|
| asset_set_asset_asset | Immutable. The asset which this asset set asset is linking to. |
| asset_set_asset_asset_set | Immutable. The asset set which this asset set asset is linking to. |
| customer_id | The ID of the customer. |
| metrics_average_cpc | The total cost of all clicks divided by the total number of clicks received. |
| metrics_average_cpm | Average cost-per-thousand impressions (CPM). |
| metrics_clicks | The number of clicks. |
| metrics_client_account_conversions | The number of client account conversions. This only includes conversion actions which include_in_client_account_conversions_metric attribute is set to true. If you use conversion-based bidding, your bid strategies will optimize for these conversions. |
| metrics_client_account_conversions_value | The value of client account conversions. This only includes conversion actions which include_in_client_account_conversions_metric attribute is set to true. If you use conversion-based bidding, your bid strategies will optimize for these conversions. |
| metrics_cost_micros | The sum of your cost-per-click (CPC) and cost-per-thousand impressions (CPM) costs during this period. |
| metrics_ctr | The number of clicks your ad receives (Clicks) divided by the number of times your ad is shown (Impressions). |
| metrics_impressions | Count of how often your ad has appeared on a search results page or website on the Google Network. |
| segments_date | Date to which metrics apply, in yyyy-MM-dd format. For example: 2024-04-17 |

Search Ads 360 Table Name: BidStrategy

Search Ads 360 API Resource: [bidding_strategy](https://developers.google.com/search-ads/reporting/api/reference/fields/v0/bidding_strategy)

| Search Ads 360 Field Name | Description |
|---|---|
| bidding_strategy_id | The ID of the bidding strategy. |
| bidding_strategy_name | The name of the bidding strategy. All bidding strategies within an account must be named distinctly. The length of this string should be between 1 and 255, inclusive, in UTF-8 bytes, (trimmed). |
| bidding_strategy_status | The status of the bidding strategy. |
| bidding_strategy_target_cpa_cpc_bid_ceiling_micros | Maximum bid limit that can be set by the bid strategy. The limit applies to all keywords managed by the strategy. This should only be set for portfolio bid strategies. |
| bidding_strategy_target_cpa_cpc_bid_floor_micros | Minimum bid limit that can be set by the bid strategy. The limit applies to all keywords managed by the strategy. This should only be set for portfolio bid strategies. |
| bidding_strategy_target_cpa_target_cpa_micros | Average CPA target. This target should be greater than or equal to minimum billable unit based on the currency for the account. |
| bidding_strategy_target_impression_share_cpc_bid_ceiling_micros | The highest CPC bid the automated bidding system is permitted to specify. This is a required field entered by the advertiser that sets the ceiling and specified in local micros. |
| bidding_strategy_target_outrank_share_cpc_bid_ceiling_micros | Maximum bid limit that can be set by the bid strategy. The limit applies to all keywords managed by the strategy. |
| bidding_strategy_target_roas_cpc_bid_ceiling_micros | Maximum bid limit that can be set by the bid strategy. The limit applies to all keywords managed by the strategy. This should only be set for portfolio bid strategies. |
| bidding_strategy_target_roas_cpc_bid_floor_micros | Minimum bid limit that can be set by the bid strategy. The limit applies to all keywords managed by the strategy. This should only be set for portfolio bid strategies. |
| bidding_strategy_target_roas_target_roas | The chosen revenue (based on conversion data) per unit of spend. Value must be between 0.01 and 1000.0, inclusive. |
| bidding_strategy_target_spend_cpc_bid_ceiling_micros | Maximum bid limit that can be set by the bid strategy. The limit applies to all keywords managed by the strategy. |
| bidding_strategy_target_spend_target_spend_micros | The spend target under which to maximize clicks. A TargetSpend bidder will attempt to spend the smaller of this value or the natural throttling spend amount. If not specified, the budget is used as the spend target. This field is deprecated and should no longer be used. |
| segments_conversion_action | Resource name of the conversion action. |

Search Ads 360 Table Name: BidStrategyStats

Search Ads 360 API Resource: [bidding_strategy](https://developers.google.com/search-ads/reporting/api/reference/fields/v0/bidding_strategy)

| Search Ads 360 Field Name | Description |
|---|---|
| bidding_strategy_id | The ID of the bidding strategy. |
| metrics_all_conversions | The total number of conversions. This includes all conversions regardless of the value of include_in_conversions_metric. |
| metrics_all_conversions_by_conversion_date | The total number of conversions. This includes all conversions regardless of the value of include_in_conversions_metric. When this column is selected with date, the values in date column means the conversion date. For more information about `by_conversion_date` columns, see [About the "All conversions" column](https://support.google.com/sa360/answer/9250611). |
| metrics_all_conversions_value | The value of all conversions. |
| metrics_all_conversions_value_by_conversion_date | The value of all conversions. When this column is selected with date, the values in date column means the conversion date. For more information about `by_conversion_date` columns, see [About the "All conversions" column](https://support.google.com/sa360/answer/9250611). |
| metrics_average_cpc | The total cost of all clicks divided by the total number of clicks received. |
| metrics_average_cpm | Average cost-per-thousand impressions (CPM). |
| metrics_clicks | The number of clicks. |
| metrics_client_account_conversions | The number of client account conversions. This only includes conversion actions which include_in_client_account_conversions_metric attribute is set to true. If you use conversion-based bidding, your bid strategies will optimize for these conversions. |
| metrics_client_account_conversions_value | The value of client account conversions. This only includes conversion actions which include_in_client_account_conversions_metric attribute is set to true. If you use conversion-based bidding, your bid strategies will optimize for these conversions. |
| metrics_client_account_view_through_conversions | The total number of view-through conversions. These happen when a customer sees an image or rich media ad, then later completes a conversion on your site without interacting with (for example, clicking on) another ad. |
| metrics_cost_micros | The sum of your cost-per-click (CPC) and cost-per-thousand impressions (CPM) costs during this period. |
| metrics_cross_device_conversions | Conversions from when a customer clicks on an ad on one device, then converts on a different device or browser. Cross-device conversions are already included in all_conversions. |
| metrics_cross_device_conversions_by_conversion_date | The number of cross-device conversions by conversion date. For more information about `by_conversion_date` columns, see [About the "All conversions" column](https://support.google.com/sa360/answer/9250611). |
| metrics_cross_device_conversions_value | The sum of the value of cross-device conversions. |
| metrics_cross_device_conversions_value_by_conversion_date | The sum of cross-device conversions value by conversion date. For more information about `by_conversion_date` columns, see [About the "All conversions" column](https://support.google.com/sa360/answer/9250611). |
| metrics_ctr | The number of clicks your ad receives (Clicks) divided by the number of times your ad is shown (Impressions). |
| metrics_impressions | Count of how often your ad has appeared on a search results page or website on the Google Network. |
| segments_date | Date to which metrics apply, in yyyy-MM-dd format. For example: 2024-04-17 |

Search Ads 360 Table Name: Campaign

Search Ads 360 API Resource: [campaign](https://developers.google.com/search-ads/reporting/api/reference/fields/v0/campaign)

| Search Ads 360 Field Name | Description |
|---|---|
| accessible_bidding_strategy_id | The ID of the bidding strategy. |
| accessible_bidding_strategy_name | The name of the bidding strategy. |
| bidding_strategy_id | The ID of the bidding strategy. |
| bidding_strategy_name | The name of the bidding strategy. All bidding strategies within an account must be named distinctly. The length of this string should be between 1 and 255, inclusive, in UTF-8 bytes, (trimmed). |
| campaign_advertising_channel_sub_type | Optional refinement to advertising_channel_type. Must be a valid sub-type of the parent channel type. Can be set only when creating campaigns. After campaign is created, the field can not be changed. |
| campaign_advertising_channel_type | The primary serving target for ads within the campaign. The targeting options can be refined in network_settings. This field is required and should not be empty when creating new campaigns. Can be set only when creating campaigns. After the campaign is created, the field can not be changed. |
| campaign_bidding_strategy_type | The type of bidding strategy. A bidding strategy can be created by setting either the bidding scheme to create a standard bidding strategy or the bidding_strategy field to create a portfolio bidding strategy. This field is read-only. |
| campaign_budget_amount_micros | The amount of the budget, in the local currency for the account. Amount is specified in micros, where one million is equivalent to one currency unit. Monthly spend is capped at 30.4 times this amount. |
| campaign_budget_delivery_method | The delivery method that determines the rate at which the campaign budget is spent. Defaults to STANDARD if unspecified in a create operation. |
| campaign_budget_period | Period over which to spend the budget. Defaults to DAILY if not specified. |
| campaign_creation_time | The timestamp when this campaign was created. The timestamp is in the customer's time zone and in "yyyy-MM-dd HH:mm:ss" format. |
| campaign_end_date | The last day of the campaign in serving customer's timezone in YYYY-MM-DD format. On create, defaults to 2037-12-30, which means the campaign will run indefinitely. To set an existing campaign to run indefinitely, set this field to 2037-12-30. |
| campaign_engine_id | ID of the campaign in the external engine account. This field is for non-Google Ads account only. For example: Yahoo Japan, Microsoft, Baidu. For Google Ads entity, use "campaign.id" instead. |
| campaign_id | The ID of the campaign. |
| campaign_labels | The resource names of labels attached to this campaign. |
| campaign_last_modified_time | The datetime when this campaign was last modified. The datetime is in the customer's time zone and in "yyyy-MM-dd HH:mm:ss.ssssss" format. |
| campaign_name | The name of the campaign. This field is required and should not be empty when creating new campaigns. It must not contain any null (code point 0x0), NL line feed (code point 0xA) or carriage return (code point 0xD) characters. |
| campaign_network_settings_target_content_network | Whether ads will be served on specified placements in the Google Display Network. Placements are specified using the Placement criterion. |
| campaign_network_settings_target_google_search | Whether ads will be served with google.com search results. |
| campaign_network_settings_target_partner_search_network | Whether ads will be served on the Google Partner Network. This is available only to some select Google partner accounts. |
| campaign_network_settings_target_search_network | Whether ads will be served on partner sites in the Google Search Network (requires target_google_search to also be true). |
| campaign_start_date | The date when campaign started in serving customer's timezone in YYYY-MM-DD format. |
| campaign_status | The status of the campaign. |
| customer_id | The ID of the customer. |

Search Ads 360 Table Name: CampaignAssetStats

Search Ads 360 API Resource: [campaign_asset](https://developers.google.com/search-ads/reporting/api/reference/fields/v0/campaign_asset)

| Search Ads 360 Field Name | Description |
|---|---|
| campaign_asset_asset | Immutable. The asset which is linked to the campaign. |
| campaign_asset_campaign | Immutable. The campaign to which the asset is linked. |
| campaign_engine_id | ID of the campaign in the external engine account. This field is for non-Google Ads account only. For example: Yahoo Japan, Microsoft, Baidu. For Google Ads entity, use "campaign.id" instead. |
| campaign_id | The ID of the campaign. |
| customer_id | The ID of the customer. |
| metrics_average_cpc | The total cost of all clicks divided by the total number of clicks received. |
| metrics_average_cpm | Average cost-per-thousand impressions (CPM). |
| metrics_clicks | The number of clicks. |
| metrics_client_account_conversions | The number of client account conversions. This only includes conversion actions which include_in_client_account_conversions_metric attribute is set to true. If you use conversion-based bidding, your bid strategies will optimize for these conversions. |
| metrics_client_account_conversions_value | The value of client account conversions. This only includes conversion actions which include_in_client_account_conversions_metric attribute is set to true. If you use conversion-based bidding, your bid strategies will optimize for these conversions. |
| metrics_cost_micros | The sum of your cost-per-click (CPC) and cost-per-thousand impressions (CPM) costs during this period. |
| metrics_ctr | The number of clicks your ad receives (Clicks) divided by the number of times your ad is shown (Impressions). |
| metrics_impressions | Count of how often your ad has appeared on a search results page or website on the Google Network. |
| segments_date | Date to which metrics apply, in yyyy-MM-dd format. For example: 2024-04-17 |

Search Ads 360 Table Name: CampaignAudienceConversionActionAndDeviceStats

Search Ads 360 API Resource: [campaign_audience_view](https://developers.google.com/search-ads/reporting/api/reference/fields/v0/campaign_audience_view)

| Search Ads 360 Field Name | Description |
|---|---|
| campaign_criterion_criterion_id | The ID of the criterion. |
| campaign_id | The ID of the campaign. |
| customer_id | The ID of the customer. |
| metrics_all_conversions | The total number of conversions. This includes all conversions regardless of the value of include_in_conversions_metric. |
| metrics_all_conversions_value | The value of all conversions. |
| metrics_cross_device_conversions | Conversions from when a customer clicks on an ad on one device, then converts on a different device or browser. Cross-device conversions are already included in all_conversions. |
| metrics_cross_device_conversions_value | The sum of the value of cross-device conversions. |
| segments.conversion_action_name | Conversion action name. |
| segments_date | Date to which metrics apply, in yyyy-MM-dd format. For example: 2024-04-17 |
| segments_device | Device to which metrics apply. |

Search Ads 360 Table Name: CampaignAudienceDeviceStats

Search Ads 360 API Resource: [campaign_audience_view](https://developers.google.com/search-ads/reporting/api/reference/fields/v0/campaign_audience_view)

| Search Ads 360 Field Name | Description |
|---|---|
| campaign_criterion_criterion_id | The ID of the criterion. |
| campaign_id | The ID of the campaign. |
| customer_id | The ID of the customer. |
| metrics_average_cpc | The total cost of all clicks divided by the total number of clicks received. |
| metrics_average_cpm | Average cost-per-thousand impressions (CPM). |
| metrics_clicks | The number of clicks. |
| metrics_client_account_conversions | The number of client account conversions. This only includes conversion actions which include_in_client_account_conversions_metric attribute is set to true. If you use conversion-based bidding, your bid strategies will optimize for these conversions. |
| metrics_client_account_conversions_value | The value of client account conversions. This only includes conversion actions which include_in_client_account_conversions_metric attribute is set to true. If you use conversion-based bidding, your bid strategies will optimize for these conversions. |
| metrics_client_account_view_through_conversions | The total number of view-through conversions. These happen when a customer sees an image or rich media ad, then later completes a conversion on your site without interacting with (for example, clicking on) another ad. |
| metrics_cost_micros | The sum of your cost-per-click (CPC) and cost-per-thousand impressions (CPM) costs during this period. |
| metrics_ctr | The number of clicks your ad receives (Clicks) divided by the number of times your ad is shown (Impressions). |
| metrics_impressions | Count of how often your ad has appeared on a search results page or website on the Google Network. |
| segments_date | Date to which metrics apply, in yyyy-MM-dd format. For example: 2024-04-17 |
| segments_device | Device to which metrics apply. |

Search Ads 360 Table Name: CampaignConversionActionAndAssetStats

Search Ads 360 API Resource: [campaign_asset](https://developers.google.com/search-ads/reporting/api/reference/fields/v0/campaign_asset)

| Search Ads 360 Field Name | Description |
|---|---|
| ad_group_id | The ID of the ad group. |
| campaign_asset_campaign | Immutable. The campaign to which the asset is linked. |
| campaign_engine_id | ID of the campaign in the external engine account. This field is for non-Google Ads account only. For example: Yahoo Japan, Microsoft, Baidu. For Google Ads entity, use "campaign.id" instead. |
| campaign_id | The ID of the campaign. |
| campain_asset_asset | Immutable. The asset which is linked to the campaign. |
| customer_id | The ID of the customer. |
| metrics_all_conversions | The total number of conversions. This includes all conversions regardless of the value of include_in_conversions_metric. |
| metrics_all_conversions_value | The value of all conversions. |
| metrics_cross_device_conversions | Conversions from when a customer clicks on an ad on one device, then converts on a different device or browser. Cross-device conversions are already included in all_conversions. |
| metrics_cross_device_conversions_value | The sum of the value of cross-device conversions. |
| segments.conversion_action_name | Conversion action name. |
| segments_date | Date to which metrics apply, in yyyy-MM-dd format. For example: 2024-04-17 |

Search Ads 360 Table Name: CampaignConversionActionAndDeviceStats

Search Ads 360 API Resource: [campaign](https://developers.google.com/search-ads/reporting/api/reference/fields/v0/campaign)

| Search Ads 360 Field Name | Description |
|---|---|
| bidding_strategy_id | The ID of the bidding strategy. |
| campaign_engine_id | ID of the campaign in the external engine account. This field is for non-Google Ads account only. For example: Yahoo Japan, Microsoft, Baidu. For Google Ads entity, use "campaign.id" instead. |
| campaign_id | The ID of the campaign. |
| customer_id | The ID of the customer. |
| metrics_all_conversions | The total number of conversions. This includes all conversions regardless of the value of include_in_conversions_metric. |
| metrics_all_conversions_by_conversion_date | The total number of conversions. This includes all conversions regardless of the value of include_in_conversions_metric. When this column is selected with date, the values in date column means the conversion date. For more information about `by_conversion_date` columns, see [About the "All conversions" column](https://support.google.com/sa360/answer/9250611). |
| metrics_all_conversions_value | The value of all conversions. |
| metrics_all_conversions_value_by_conversion_date | The value of all conversions. When this column is selected with date, the values in date column means the conversion date. For more information about `by_conversion_date` columns, see [About the "All conversions" column](https://support.google.com/sa360/answer/9250611). |
| metrics_cross_device_conversions | Conversions from when a customer clicks on an ad on one device, then converts on a different device or browser. Cross-device conversions are already included in all_conversions. |
| metrics_cross_device_conversions_by_conversion_date | The number of cross-device conversions by conversion date. For more information about `by_conversion_date` columns, see [About the "All conversions" column](https://support.google.com/sa360/answer/9250611). |
| metrics_cross_device_conversions_value | The sum of the value of cross-device conversions. |
| metrics_cross_device_conversions_value_by_conversion_date | The sum of cross-device conversions value by conversion date. For more information about `by_conversion_date` columns, see [About the "All conversions" column](https://support.google.com/sa360/answer/9250611). |
| segments.conversion_action_name | Conversion action name. |
| segments_date | Date to which metrics apply, in yyyy-MM-dd format. For example: 2024-04-17 |
| segments_device | Device to which metrics apply. |

Search Ads 360 Table Name: CampaignCriterion

Search Ads 360 API Resource: [campaign_criterion](https://developers.google.com/search-ads/reporting/api/reference/fields/v0/campaign_criterion)

| Search Ads 360 Field Name | Description |
|---|---|
| campaign_criterion_bid_modifier | The modifier for the bid when the criterion matches. The modifier must be in the range: 0.1 - 10.0. Most targetable criteria types support modifiers. |
| campaign_criterion_criterion_id | The ID of the criterion. |
| campaign_criterion_last_modified_time | The datetime when this campaign criterion was last modified. The datetime is in the customer's time zone and in "yyyy-MM-dd HH:mm:ss.ssssss" format. |
| campaign_criterion_location_geo_target_constant | The geo target constant resource name. |
| campaign_criterion_status | The status of the criterion. |
| campaign_criterion_type | The type of the criterion. |
| campaign_criterion_user_list_user_list | The User List resource name. |
| campaign_id | The ID of the campaign. |
| customer_id | The ID of the customer. |

Search Ads 360 Table Name: CampaignDeviceStats

Search Ads 360 API Resource: [campaign](https://developers.google.com/search-ads/reporting/api/reference/fields/v0/campaign)

| Search Ads 360 Field Name | Description |
|---|---|
| bidding_strategy_id | The ID of the bidding strategy. |
| campaign_engine_id | ID of the campaign in the external engine account. This field is for non-Google Ads account only. For example: Yahoo Japan, Microsoft, Baidu. For Google Ads entity, use "campaign.id" instead. |
| campaign_id | The ID of the campaign. |
| customer_id | The ID of the customer. |
| metrics_average_cpc | The total cost of all clicks divided by the total number of clicks received. |
| metrics_average_cpm | Average cost-per-thousand impressions (CPM). |
| metrics_clicks | The number of clicks. |
| metrics_client_account_conversions | The number of client account conversions. This only includes conversion actions which include_in_client_account_conversions_metric attribute is set to true. If you use conversion-based bidding, your bid strategies will optimize for these conversions. |
| metrics_client_account_conversions_value | The value of client account conversions. This only includes conversion actions which include_in_client_account_conversions_metric attribute is set to true. If you use conversion-based bidding, your bid strategies will optimize for these conversions. |
| metrics_client_account_view_through_conversions | The total number of view-through conversions. These happen when a customer sees an image or rich media ad, then later completes a conversion on your site without interacting with (for example, clicking on) another ad. |
| metrics_cost_micros | The sum of your cost-per-click (CPC) and cost-per-thousand impressions (CPM) costs during this period. |
| metrics_ctr | The number of clicks your ad receives (Clicks) divided by the number of times your ad is shown (Impressions). |
| metrics_impressions | Count of how often your ad has appeared on a search results page or website on the Google Network. |
| metrics_visits | Clicks that Search Ads 360 has successfully recorded and forwarded to an advertiser's landing page. |
| segments_date | Date to which metrics apply, in yyyy-MM-dd format. For example: 2024-04-17 |
| segments_device | Device to which metrics apply. |

Search Ads 360 Table Name: CampaignLabel

Search Ads 360 API Resource: [campaign_label](https://developers.google.com/search-ads/reporting/api/reference/fields/v0/campaign_label)

| Search Ads 360 Field Name | Description |
|---|---|
| campaign_id | The ID of the campaign. |
| campaign_label_campaign | The campaign to which the label is attached. |
| campaign_label_label | The label assigned to the campaign. |
| campaign_label_owner_customer_id | The ID of the Customer which owns the label. |
| campaign_label_resource_name | Name of the resource. Campaign label resource names have the form: customers/{customer_id}/campaignLabels/{campaign_id}\~{label_id} |
| customer_id | The ID of the customer. |

Search Ads 360 Table Name: CampaignStats

Search Ads 360 API Resource: [campaign](https://developers.google.com/search-ads/reporting/api/reference/fields/v0/campaign)

| Search Ads 360 Field Name | Description |
|---|---|
| bidding_strategy_id | The ID of the bidding strategy. |
| campaign_engine_id | ID of the campaign in the external engine account. This field is for non-Google Ads account only. For example: Yahoo Japan, Microsoft, Baidu. For Google Ads entity, use "campaign.id" instead. |
| campaign_id | The ID of the campaign. |
| customer_id | The ID of the customer. |
| metrics_all_conversions | The total number of conversions. This includes all conversions regardless of the value of include_in_conversions_metric. |
| metrics_all_conversions_value | The value of all conversions. |
| metrics_average_cpc | The total cost of all clicks divided by the total number of clicks received. |
| metrics_average_cpm | Average cost-per-thousand impressions (CPM). |
| metrics_clicks | The number of clicks. |
| metrics_client_account_conversions | The number of client account conversions. This only includes conversion actions which include_in_client_account_conversions_metric attribute is set to true. If you use conversion-based bidding, your bid strategies will optimize for these conversions. |
| metrics_client_account_conversions_value | The value of client account conversions. This only includes conversion actions which include_in_client_account_conversions_metric attribute is set to true. If you use conversion-based bidding, your bid strategies will optimize for these conversions. |
| metrics_client_account_view_through_conversions | The total number of view-through conversions. These happen when a customer sees an image or rich media ad, then later completes a conversion on your site without interacting with (for example, clicking on) another ad. |
| metrics_content_budget_lost_impression_share | The estimated percent of times that your ad was eligible to show on the Search Network but didn't because your budget was too low. Note: Search budget lost impression share is reported in the range of 0 to 0.9. Any value greater than 0.9 is reported as 0.9001. |
| metrics_content_impression_share | The impressions you've received on the Display Network divided by the estimated number of impressions you were eligible to receive. Note: Content impression share is reported in the range of 0.1 to 1. Any value lower than 0.1 is reported as 0.0999. |
| metrics_content_rank_lost_impression_share | The estimated percentage of impressions on the Display Network that your ads didn't receive due to poor Ad Rank. Note: Content rank lost impression share is reported in the range of 0 to 0.9. Any value greater than 0.9 is reported as 0.9001. |
| metrics_conversions | The number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. If you use conversion-based bidding, your bid strategies will optimize for these conversions. |
| metrics_conversions_value | The sum of conversion values for the conversions included in the "conversions" field. This metric is useful only if you entered a value for your conversion actions. |
| metrics_cost_micros | The sum of your cost-per-click (CPC) and cost-per-thousand impressions (CPM) costs during this period. |
| metrics_ctr | The number of clicks your ad receives (Clicks) divided by the number of times your ad is shown (Impressions). |
| metrics_historical_quality_score | The historical quality score. |
| metrics_impressions | Count of how often your ad has appeared on a search results page or website on the Google Network. |
| metrics_search_budget_lost_impression_share | The estimated percent of times that your ad was eligible to show on the Search Network but didn't because your budget was too low. Note: Search budget lost impression share is reported in the range of 0 to 0.9. Any value above 0.9 is reported as 0.9001. |
| metrics_search_impression_share | The impressions you've received on the Search Network divided by the estimated number of impressions you were eligible to receive. Note: Search impression share is reported in the range of 0.1 to 1. Any value below 0.1 is reported as 0.0999. |
| metrics_search_rank_lost_impression_share | The estimated percentage of impressions on the Search Network that your ads didn't receive due to poor Ad Rank. Note: Search rank lost impression share is reported in the range of 0 to 0.9. Any value above 0.9 is reported as 0.9001. |
| metrics_visits | Clicks that Search Ads 360 has successfully recorded and forwarded to an advertiser's landing page. |
| segments_date | Date to which metrics apply, in yyyy-MM-dd format. For example: 2024-04-17 |

Search Ads 360 Table Name: CartDataSalesStats

Search Ads 360 API Resource: [cart_data_sales_view](https://developers.google.com/search-ads/reporting/api/reference/fields/v0/cart_data_sales_view)

| Search Ads 360 Field Name | Description |
|---|---|
| ad_group_id | The ID of the ad group. |
| campaign_id | The ID of the campaign. |
| customer_id | The ID of the customer. |
| metrics_client_account_cross_sell_gross_profit_micros | https://developers.google.com/search-ads/reporting/api/reference/fields/v0/cart_data_sales_view#metrics.client_account_cross_sell_gross_profit_micros |
| metrics_client_account_cross_sell_revenue_micros | https://developers.google.com/search-ads/reporting/api/reference/fields/v0/cart_data_sales_view#metrics.client_account_cross_sell_revenue_micros |
| metrics_client_account_cross_sell_units_sold | https://developers.google.com/search-ads/reporting/api/reference/fields/v0/cart_data_sales_view#metrics.client_account_cross_sell_units_sold |
| metrics_client_account_lead_gross_profit_micros | https://developers.google.com/search-ads/reporting/api/reference/fields/v0/cart_data_sales_view#metrics.client_account_lead_gross_profit_micros |
| metrics_client_account_lead_revenue_micros | https://developers.google.com/search-ads/reporting/api/reference/fields/v0/cart_data_sales_view#metrics.client_account_lead_revenue_micros |
| metrics_client_account_lead_units_sold | https://developers.google.com/search-ads/reporting/api/reference/fields/v0/cart_data_sales_view#metrics.client_account_lead_units_sold |
| metrics_cross_sell_gross_profit_micros | https://developers.google.com/search-ads/reporting/api/reference/fields/v0/cart_data_sales_view#metrics.cross_sell_gross_profit_micros |
| metrics_cross_sell_revenue_micros | https://developers.google.com/search-ads/reporting/api/reference/fields/v0/cart_data_sales_view#metrics.cross_sell_revenue_micros |
| metrics_cross_sell_units_sold | https://developers.google.com/search-ads/reporting/api/reference/fields/v0/cart_data_sales_view#metrics.cross_sell_units_sold |
| metrics_lead_gross_profit_micros | https://developers.google.com/search-ads/reporting/api/reference/fields/v0/cart_data_sales_view#metrics.lead_gross_profit_micros |
| metrics_lead_revenue_micros | https://developers.google.com/search-ads/reporting/api/reference/fields/v0/cart_data_sales_view#metrics.lead_revenue_micros |
| metrics_lead_units_sold | https://developers.google.com/search-ads/reporting/api/reference/fields/v0/cart_data_sales_view#metrics.lead_units_sold |
| segments_date | Date to which metrics apply, in yyyy-MM-dd format. For example: 2024-04-17 |
| segments_product_brand | Brand of the product. |
| segments_product_item_id | Item ID of the product. |
| segments_product_sold_brand | Brand of the product sold. |
| segments_product_sold_item_id | Item ID of the product sold. |
| segments_product_sold_title | Title of the product sold. |
| segments_product_sold_type_l1 | Type (level 1) of the product sold. |
| segments_product_sold_type_l2 | Type (level 2) of the product sold. |
| segments_product_sold_type_l3 | Type (level 3) of the product sold. |
| segments_product_sold_type_l4 | Type (level 4) of the product sold. |
| segments_product_sold_type_l5 | Type (level 5) of the product sold. |
| segments_product_title | Title of the product. |
| segments_product_type_l1 | Type (level 1) of the product. |
| segments_product_type_l2 | Type (level 2) of the product. |
| segments_product_type_l3 | Type (level 3) of the product. |
| segments_product_type_l4 | Type (level 4) of the product. |
| segments_product_type_l5 | Type (level 5) of the product. |

Search Ads 360 Table Name: Conversion

Search Ads 360 API Resource: [conversion](https://developers.google.com/search-ads/reporting/api/reference/fields/v0/conversion)

| Search Ads 360 Field Name | Description |
|---|---|
| ad_group_criterion_criterion_id | The ID of the criterion. |
| ad_group_id | The ID of the ad group. |
| campaign_id | The ID of the campaign. |
| conversion_ad_id | Ad ID. A value of 0 indicates that the ad is unattributed. |
| conversion_advertiser_conversion_id | For offline conversions, this is an ID provided by advertisers. If an advertiser doesn't specify such an ID, Search Ads 360 generates one. For online conversions, this is equal to the id column or the floodlight_order_id column depending on the advertiser's Floodlight instructions. |
| conversion_attribution_type | What the conversion is attributed to: Visit or Keyword+Ad. |
| conversion_click_id | A unique string, for the visit that the conversion is attributed to, that is passed to the landing page as the click id URL parameter. |
| conversion_conversion_date_time | The timestamp of the conversion event. |
| conversion_conversion_last_modified_date_time | The timestamp of the last time the conversion was modified. |
| conversion_conversion_quantity | The quantity of items recorded by the conversion, as determined by the qty url parameter. The advertiser is responsible for dynamically populating the parameter (such as number of items sold in the conversion), otherwise it defaults to 1. |
| conversion_conversion_revenue_micros | The adjusted revenue in micros for the conversion event. This will always be in the currency of the serving account. |
| conversion_conversion_visit_date_time | The timestamp of the visit that the conversion is attributed to. |
| conversion_criterion_id | Search Ads 360 criterion ID. A value of 0 indicates that the criterion is unattributed. |
| conversion_floodlight_order_id | The Floodlight order ID provided by the advertiser for the conversion. |
| conversion_floodlight_original_revenue | The original, unchanged revenue associated with the Floodlight event (in the currency of the current report), before Floodlight currency instruction modifications. |
| conversion_id | The ID of the conversion |
| conversion_merchant_id | The SearchAds360 inventory account ID containing the product that was clicked on. SearchAds360 generates this ID when you link an inventory account in SearchAds360. |
| conversion_product_channel | The sales channel of the product that was clicked on: Online or Local. |
| conversion_product_country_code | The country (ISO-3166-format) registered for the inventory feed that contains the product clicked on. |
| conversion_product_id | The ID of the product clicked on. |
| conversion_product_language_code | The language (ISO-639-1) that has been set for the Merchant Center feed containing data about the product. |
| conversion_product_store_id | The store in the Local Inventory Ad that was clicked on. This should match the store IDs used in your local products feed. |
| conversion_status | The status of the conversion, either ENABLED or REMOVED. |
| conversion_visit_id | The SearchAds360 visit ID that the conversion is attributed to. |
| customer_account_type | Engine account type. For example: Google Ads, Microsoft Advertising, Yahoo Japan, Baidu, Facebook, Engine Track. |
| customer_descriptive_name | Optional, non-unique descriptive name of the customer. |
| customer_id | The ID of the customer. |
| segments.conversion_action_name | Conversion action name. |
| segments_date | Date to which metrics apply, in yyyy-MM-dd format. For example: 2024-04-17 |
| segments_device | Device to which metrics apply. |

Search Ads 360 Table Name: ConversionAction

Search Ads 360 API Resource: [conversion_action](https://developers.google.com/search-ads/reporting/api/reference/fields/v0/conversion_action)

| Search Ads 360 Field Name | Description |
|---|---|
| conversion_action_creation_time | Timestamp of the Floodlight activity's creation, formatted in ISO 8601. |
| conversion_action_floodlight_settings_activity_group_tag | String used to identify a Floodlight activity group when reporting conversions. |
| conversion_action_floodlight_settings_activity_id | ID of the Floodlight activity in DoubleClick Campaign Manager (DCM). |
| conversion_action_floodlight_settings_activity_tag | String used to identify a Floodlight activity when reporting conversions. |
| conversion_action_name | The name of the conversion action. This field is required and should not be empty when creating new conversion actions. |
| conversion_action_status | The status of this conversion action for conversion event accrual. |
| conversion_action_type | The type of this conversion action. |

Search Ads 360 Table Name: GenderConversionActionAndDeviceStats

Search Ads 360 API Resource: [gender_view](https://developers.google.com/search-ads/reporting/api/reference/fields/v0/gender_view)

| Search Ads 360 Field Name | Description |
|---|---|
| ad_group_criterion_age_range_type | Type of the age range. |
| ad_group_criterion_bid_modifier | The modifier for the bid when the criterion matches. The modifier must be in the range: 0.1 - 10.0. Most targetable criteria types support modifiers. |
| ad_group_criterion_cpc_bid_micros | The CPC (cost-per-click) bid. |
| ad_group_criterion_criterion_id | The ID of the criterion. |
| ad_group_criterion_gender_type | Type of the gender. |
| ad_group_criterion_location_geo_target_constant | The geo target constant resource name. |
| ad_group_criterion_user_list_user_list | The User List resource name. |
| ad_group_criterion_webpage_conditions | Conditions, or logical expressions, for webpage targeting. The list of webpage targeting conditions are and-ed together when evaluated for targeting. An empty list of conditions indicates all pages of the campaign's website are targeted. This field is required for CREATE operations and is prohibited on UPDATE operations. |
| ad_group_criterion_webpage_coverage_percentage | Website criteria coverage percentage. This is the computed percentage of website coverage based on the website target, negative website target and negative keywords in the ad group and campaign. For instance, when coverage returns as 1, it indicates it has 100% coverage. |
| ad_group_id | The ID of the ad group. |
| campaign_id | The ID of the campaign. |
| customer_id | The ID of the customer. |
| metrics_cross_device_conversions | Conversions from when a customer clicks on an ad on one device, then converts on a different device or browser. Cross-device conversions are already included in all_conversions. |
| metrics_cross_device_conversions_value | The sum of the value of cross-device conversions. |
| segments.conversion_action_name | Conversion action name. |
| segments_date | Date to which metrics apply, in yyyy-MM-dd format. For example: 2024-04-17 |
| segments_device | Device to which metrics apply. |

Search Ads 360 Table Name: GenderDeviceStats

Search Ads 360 API Resource: [gender_view](https://developers.google.com/search-ads/reporting/api/reference/fields/v0/gender_view)

| Search Ads 360 Field Name | Description |
|---|---|
| ad_group_criterion_age_range_type | Type of the age range. |
| ad_group_criterion_bid_modifier | The modifier for the bid when the criterion matches. The modifier must be in the range: 0.1 - 10.0. Most targetable criteria types support modifiers. |
| ad_group_criterion_cpc_bid_micros | The CPC (cost-per-click) bid. |
| ad_group_criterion_criterion_id | The ID of the criterion. |
| ad_group_criterion_gender_type | Type of the gender. |
| ad_group_criterion_location_geo_target_constant | The geo target constant resource name. |
| ad_group_criterion_user_list_user_list | The User List resource name. |
| ad_group_criterion_webpage_conditions | Conditions, or logical expressions, for webpage targeting. The list of webpage targeting conditions are and-ed together when evaluated for targeting. An empty list of conditions indicates all pages of the campaign's website are targeted. This field is required for CREATE operations and is prohibited on UPDATE operations. |
| ad_group_criterion_webpage_coverage_percentage | Website criteria coverage percentage. This is the computed percentage of website coverage based on the website target, negative website target and negative keywords in the ad group and campaign. For instance, when coverage returns as 1, it indicates it has 100% coverage. |
| ad_group_id | The ID of the ad group. |
| campaign_id | The ID of the campaign. |
| customer_id | The ID of the customer. |
| metrics_average_cpc | The total cost of all clicks divided by the total number of clicks received. |
| metrics_average_cpm | Average cost-per-thousand impressions (CPM). |
| metrics_clicks | The number of clicks. |
| metrics_client_account_conversions | The number of client account conversions. This only includes conversion actions which include_in_client_account_conversions_metric attribute is set to true. If you use conversion-based bidding, your bid strategies will optimize for these conversions. |
| metrics_client_account_conversions_value | The value of client account conversions. This only includes conversion actions which include_in_client_account_conversions_metric attribute is set to true. If you use conversion-based bidding, your bid strategies will optimize for these conversions. |
| metrics_client_account_view_through_conversions | The total number of view-through conversions. These happen when a customer sees an image or rich media ad, then later completes a conversion on your site without interacting with (for example, clicking on) another ad. |
| metrics_cost_micros | The sum of your cost-per-click (CPC) and cost-per-thousand impressions (CPM) costs during this period. |
| metrics_ctr | The number of clicks your ad receives (Clicks) divided by the number of times your ad is shown (Impressions). |
| metrics_impressions | Count of how often your ad has appeared on a search results page or website on the Google Network. |
| segments_date | Date to which metrics apply, in yyyy-MM-dd format. For example: 2024-04-17 |
| segments_device | Device to which metrics apply. |

Search Ads 360 Table Name: Keyword

Search Ads 360 API Resource: [keyword_view](https://developers.google.com/search-ads/reporting/api/reference/fields/v0/keyword_view)

| Search Ads 360 Field Name | Description |
|---|---|
| ad_group_criterion_cpc_bid_micros | The CPC (cost-per-click) bid. |
| ad_group_criterion_creation_time | The timestamp when this ad group criterion was created. The timestamp is in the customer's time zone and in "yyyy-MM-dd HH:mm:ss" format. |
| ad_group_criterion_criterion_id | The ID of the criterion. |
| ad_group_criterion_effective_cpc_bid_micros | The effective CPC (cost-per-click) bid. |
| ad_group_criterion_engine_id | ID of the ad group criterion in the external engine account. This field is for SearchAds 360 account only. For example: Yahoo Japan, Microsoft, Baidu. For non-SearchAds 360 entity, use "ad_group_criterion.criterion_id" instead. |
| ad_group_criterion_engine_status | The Engine Status for ad group criterion. |
| ad_group_criterion_final_url_suffix | URL template for appending params to final URL. |
| ad_group_criterion_final_urls | The list of possible final URLs after all cross-domain redirects for the ad. |
| ad_group_criterion_keyword_match_type | The match type of the keyword. |
| ad_group_criterion_keyword_text | The text of the keyword (at most 80 characters and 10 words). |
| ad_group_criterion_labels | The resource names of labels attached to this ad group criterion. |
| ad_group_criterion_last_modified_time | The datetime when this ad group criterion was last modified. The datetime is in the customer's time zone and in "yyyy-MM-dd HH:mm:ss.ssssss" format. |
| ad_group_criterion_quality_info_quality_score | The quality score. This field may not be populated if Google does not have enough information to determine a value. |
| ad_group_criterion_status | The status of the criterion. This is the status of the ad group criterion entity, set by the client. Note: UI reports may incorporate additional information that affects whether a criterion is eligible to run. In some cases a criterion that's REMOVED in the API can still show as enabled in the UI. For example, campaigns by default show to users of all age ranges unless excluded. The UI will show each age range as "enabled", since they're eligible to see the ads; but AdGroupCriterion.status will show "removed", since no positive criterion was added. |
| ad_group_criterion_tracking_url_template | The URL template for constructing a tracking URL. |
| ad_group_id | The ID of the ad group. |
| bidding_strategy_id | The ID of the bidding strategy. |
| bidding_strategy_target_cpa_cpc_bid_ceiling_micros | Maximum bid limit that can be set by the bid strategy. The limit applies to all keywords managed by the strategy. This should only be set for portfolio bid strategies. |
| bidding_strategy_target_cpa_cpc_bid_floor_micros | Minimum bid limit that can be set by the bid strategy. The limit applies to all keywords managed by the strategy. This should only be set for portfolio bid strategies. |
| bidding_strategy_target_impression_share_cpc_bid_ceiling_micros | The highest CPC bid the automated bidding system is permitted to specify. This is a required field entered by the advertiser that sets the ceiling and specified in local micros. |
| bidding_strategy_target_outrank_share_cpc_bid_ceiling_micros | Maximum bid limit that can be set by the bid strategy. The limit applies to all keywords managed by the strategy. |
| bidding_strategy_target_roas_cpc_bid_ceiling_micros | Maximum bid limit that can be set by the bid strategy. The limit applies to all keywords managed by the strategy. This should only be set for portfolio bid strategies. |
| bidding_strategy_target_roas_cpc_bid_floor_micros | Minimum bid limit that can be set by the bid strategy. The limit applies to all keywords managed by the strategy. This should only be set for portfolio bid strategies. |
| bidding_strategy_target_spend_cpc_bid_ceiling_micros | Maximum bid limit that can be set by the bid strategy. The limit applies to all keywords managed by the strategy. |
| campaign_id | The ID of the campaign. |
| customer_id | The ID of the customer. |

Search Ads 360 Table Name: KeywordConversionActionAndDeviceStats

Search Ads 360 API Resource: [keyword_view](https://developers.google.com/search-ads/reporting/api/reference/fields/v0/keyword_view)

| Search Ads 360 Field Name | Description |
|---|---|
| ad_group_criterion_criterion_id | The ID of the criterion. |
| ad_group_criterion_engine_id | ID of the ad group criterion in the external engine account. This field is for SearchAds 360 account only. For example: Yahoo Japan, Microsoft, Baidu. For non-SearchAds 360 entity, use "ad_group_criterion.criterion_id" instead. |
| ad_group_id | The ID of the ad group. |
| bidding_strategy_id | The ID of the bidding strategy. |
| campaign_id | The ID of the campaign. |
| customer_id | The ID of the customer. |
| metrics_all_conversions | The total number of conversions. This includes all conversions regardless of the value of include_in_conversions_metric. |
| metrics_all_conversions_by_conversion_date | The total number of conversions. This includes all conversions regardless of the value of include_in_conversions_metric. When this column is selected with date, the values in date column means the conversion date. For more information about `by_conversion_date` columns, see [About the "All conversions" column](https://support.google.com/sa360/answer/9250611). |
| metrics_all_conversions_value | The value of all conversions. |
| metrics_all_conversions_value_by_conversion_date | The value of all conversions. When this column is selected with date, the values in date column means the conversion date. For more information about `by_conversion_date` columns, see [About the "All conversions" column](https://support.google.com/sa360/answer/9250611). |
| metrics_cross_device_conversions | Conversions from when a customer clicks on an ad on one device, then converts on a different device or browser. Cross-device conversions are already included in all_conversions. |
| metrics_cross_device_conversions_by_conversion_date | The number of cross-device conversions by conversion date. For more information about `by_conversion_date` columns, see [About the "All conversions" column](https://support.google.com/sa360/answer/9250611). |
| metrics_cross_device_conversions_value | The sum of the value of cross-device conversions. |
| metrics_cross_device_conversions_value_by_conversion_date | The sum of cross-device conversions value by conversion date. For more information about `by_conversion_date` columns, see [About the "All conversions" column](https://support.google.com/sa360/answer/9250611). |
| segments.conversion_action_name | Conversion action name. |
| segments_date | Date to which metrics apply, in yyyy-MM-dd format. For example: 2024-04-17 |
| segments_device | Device to which metrics apply. |

Search Ads 360 Table Name: KeywordDeviceStats

Search Ads 360 API Resource: [keyword_view](https://developers.google.com/search-ads/reporting/api/reference/fields/v0/keyword_view)

| Search Ads 360 Field Name | Description |
|---|---|
| ad_group_criterion_criterion_id | The ID of the criterion. |
| ad_group_criterion_engine_id | ID of the ad group criterion in the external engine account. This field is for SearchAds 360 account only. For example: Yahoo Japan, Microsoft, Baidu. For non-SearchAds 360 entity, use "ad_group_criterion.criterion_id" instead. |
| ad_group_id | The ID of the ad group. |
| bidding_strategy_id | The ID of the bidding strategy. |
| campaign_id | The ID of the campaign. |
| customer_id | The ID of the customer. |
| metrics_average_cpc | The total cost of all clicks divided by the total number of clicks received. |
| metrics_average_cpm | Average cost-per-thousand impressions (CPM). |
| metrics_clicks | The number of clicks. |
| metrics_client_account_conversions | The number of client account conversions. This only includes conversion actions which include_in_client_account_conversions_metric attribute is set to true. If you use conversion-based bidding, your bid strategies will optimize for these conversions. |
| metrics_client_account_conversions_value | The value of client account conversions. This only includes conversion actions which include_in_client_account_conversions_metric attribute is set to true. If you use conversion-based bidding, your bid strategies will optimize for these conversions. |
| metrics_client_account_view_through_conversions | The total number of view-through conversions. These happen when a customer sees an image or rich media ad, then later completes a conversion on your site without interacting with (for example, clicking on) another ad. |
| metrics_cost_micros | The sum of your cost-per-click (CPC) and cost-per-thousand impressions (CPM) costs during this period. |
| metrics_ctr | The number of clicks your ad receives (Clicks) divided by the number of times your ad is shown (Impressions). |
| metrics_impressions | Count of how often your ad has appeared on a search results page or website on the Google Network. |
| metrics_visits | Clicks that Search Ads 360 has successfully recorded and forwarded to an advertiser's landing page. |
| segments_date | Date to which metrics apply, in yyyy-MM-dd format. For example: 2024-04-17 |
| segments_device | Device to which metrics apply. |

Search Ads 360 Table Name: KeywordStats

Search Ads 360 API Resource: [keyword_view](https://developers.google.com/search-ads/reporting/api/reference/fields/v0/keyword_view)

| Search Ads 360 Field Name | Description |
|---|---|
| ad_group_criterion_criterion_id | The ID of the criterion. |
| ad_group_id | The ID of the ad group. |
| bidding_strategy_id | The ID of the bidding strategy. |
| campaign_id | The ID of the campaign. |
| customer_id | The ID of the customer. |
| metrics_historical_quality_score | The historical quality score. |
| segments_date | Date to which metrics apply, in yyyy-MM-dd format. For example: 2024-04-17 |

Search Ads 360 Table Name: LocationConversionActionAndDeviceStats

Search Ads 360 API Resource: [location_view](https://developers.google.com/search-ads/reporting/api/reference/fields/v0/location_view)

| Search Ads 360 Field Name | Description |
|---|---|
| campaign_criterion_criterion_id | The ID of the criterion. |
| campaign_id | The ID of the campaign. |
| customer_id | The ID of the customer. |
| metrics_all_conversions | The total number of conversions. This includes all conversions regardless of the value of include_in_conversions_metric. |
| metrics_all_conversions_value | The value of all conversions. |
| metrics_cross_device_conversions | Conversions from when a customer clicks on an ad on one device, then converts on a different device or browser. Cross-device conversions are already included in all_conversions. |
| metrics_cross_device_conversions_value | The sum of the value of cross-device conversions. |
| segments.conversion_action_name | Conversion action name. |
| segments_date | Date to which metrics apply, in yyyy-MM-dd format. For example: 2024-04-17 |
| segments_device | Device to which metrics apply. |

Search Ads 360 Table Name: LocationDeviceStats

Search Ads 360 API Resource: [location_view](https://developers.google.com/search-ads/reporting/api/reference/fields/v0/location_view)

| Search Ads 360 Field Name | Description |
|---|---|
| campaign_criterion_criterion_id | The ID of the criterion. |
| campaign_id | The ID of the campaign. |
| customer_id | The ID of the customer. |
| metrics_average_cpc | The total cost of all clicks divided by the total number of clicks received. |
| metrics_average_cpm | Average cost-per-thousand impressions (CPM). |
| metrics_clicks | The number of clicks. |
| metrics_client_account_conversions | The number of client account conversions. This only includes conversion actions which include_in_client_account_conversions_metric attribute is set to true. If you use conversion-based bidding, your bid strategies will optimize for these conversions. |
| metrics_client_account_conversions_value | The value of client account conversions. This only includes conversion actions which include_in_client_account_conversions_metric attribute is set to true. If you use conversion-based bidding, your bid strategies will optimize for these conversions. |
| metrics_client_account_view_through_conversions | The total number of view-through conversions. These happen when a customer sees an image or rich media ad, then later completes a conversion on your site without interacting with (for example, clicking on) another ad. |
| metrics_cost_micros | The sum of your cost-per-click (CPC) and cost-per-thousand impressions (CPM) costs during this period. |
| metrics_ctr | The number of clicks your ad receives (Clicks) divided by the number of times your ad is shown (Impressions). |
| metrics_impressions | Count of how often your ad has appeared on a search results page or website on the Google Network. |
| segments_date | Date to which metrics apply, in yyyy-MM-dd format. For example: 2024-04-17 |
| segments_device | Device to which metrics apply. |

Search Ads 360 Table Name: NegativeAdGroupCriterion

Search Ads 360 API Resource: [ad_group_criterion](https://developers.google.com/search-ads/reporting/api/reference/fields/v0/ad_group_criterion)

| Search Ads 360 Field Name | Description |
|---|---|
| ad_group_criterion_age_range_type | Type of the age range. |
| ad_group_criterion_creation_time | The timestamp when this ad group criterion was created. The timestamp is in the customer's time zone and in "yyyy-MM-dd HH:mm:ss" format. |
| ad_group_criterion_criterion_id | The ID of the criterion. |
| ad_group_criterion_gender_type | Type of the gender. |
| ad_group_criterion_last_modified_time | The datetime when this ad group criterion was last modified. The datetime is in the customer's time zone and in "yyyy-MM-dd HH:mm:ss.ssssss" format. |
| ad_group_criterion_location_geo_target_constant | The geo target constant resource name. |
| ad_group_criterion_status | The status of the criterion. This is the status of the ad group criterion entity, set by the client. Note: UI reports may incorporate additional information that affects whether a criterion is eligible to run. In some cases a criterion that's REMOVED in the API can still show as enabled in the UI. For example, campaigns by default show to users of all age ranges unless excluded. The UI will show each age range as "enabled", since they're eligible to see the ads; but AdGroupCriterion.status will show "removed", since no positive criterion was added. |
| ad_group_criterion_type | The type of the criterion. |
| ad_group_criterion_user_list_user_list | The User List resource name. |
| ad_group_criterion_webpage_conditions | Conditions, or logical expressions, for web page targeting. The list of web page targeting conditions are and-ed together when evaluated for targeting. An empty list of conditions indicates all pages of the campaign's website are targeted. This field is required for CREATE operations and is prohibited on UPDATE operations. |
| ad_group_id | The ID of the ad group. |
| campaign_id | The ID of the campaign. |
| customer_id | The ID of the customer. |

Search Ads 360 Table Name: NegativeAdGroupKeyword

Search Ads 360 API Resource: [ad_group_criterion](https://developers.google.com/search-ads/reporting/api/reference/fields/v0/ad_group_criterion)

| Search Ads 360 Field Name | Description |
|---|---|
| ad_group_criterion_creation_time | The timestamp when this ad group criterion was created. The timestamp is in the customer's time zone and in "yyyy-MM-dd HH:mm:ss" format. |
| ad_group_criterion_criterion_id | The ID of the criterion. |
| ad_group_criterion_keyword_match_type | The match type of the keyword. |
| ad_group_criterion_keyword_text | The text of the keyword (at most 80 characters and 10 words). |
| ad_group_criterion_last_modified_time | The datetime when this ad group criterion was last modified. The datetime is in the customer's time zone and in "yyyy-MM-dd HH:mm:ss.ssssss" format. |
| ad_group_criterion_status | The status of the criterion. This is the status of the ad group criterion entity, set by the client. Note: UI reports may incorporate additional information that affects whether a criterion is eligible to run. In some cases a criterion that's REMOVED in the API can still show as enabled in the UI. For example, campaigns by default show to users of all age ranges unless excluded. The UI will show each age range as "enabled", since they're eligible to see the ads; but AdGroupCriterion.status will show "removed", since no positive criterion was added. |
| ad_group_id | The ID of the ad group. |
| campaign_id | The ID of the campaign. |
| customer_id | The ID of the customer. |

Search Ads 360 Table Name: NegativeCampaignCriterion

Search Ads 360 API Resource: [campaign_criterion](https://developers.google.com/search-ads/reporting/api/reference/fields/v0/campaign_criterion)

| Search Ads 360 Field Name | Description |
|---|---|
| campaign_criterion_age_range_type | Type of the age range. |
| campaign_criterion_criterion_id | The ID of the criterion. |
| campaign_criterion_gender_type | Type of the gender. |
| campaign_criterion_last_modified_time | The datetime when this campaign criterion was last modified. The datetime is in the customer's time zone and in "yyyy-MM-dd HH:mm:ss.ssssss" format. |
| campaign_criterion_location_geo_target_constant | The geo target constant resource name. |
| campaign_criterion_status | The status of the criterion. |
| campaign_criterion_type | The type of the criterion. |
| campaign_criterion_user_list_user_list | The User List resource name. |
| campaign_criterion_webpage_conditions | Conditions, or logical expressions, for webpage targeting. The list of webpage targeting conditions are and-ed together when evaluated for targeting. An empty list of conditions indicates all pages of the campaign's website are targeted. This field is required for CREATE operations and is prohibited on UPDATE operations. |
| campaign_id | The ID of the campaign. |
| customer_id | The ID of the customer. |

Search Ads 360 Table Name: NegativeCampaignKeyword

Search Ads 360 API Resource: [campaign_criterion](https://developers.google.com/search-ads/reporting/api/reference/fields/v0/campaign_criterion)

| Search Ads 360 Field Name | Description |
|---|---|
| campaign_criterion_criterion_id | The ID of the criterion. |
| campaign_criterion_keyword_match_type | The match type of the keyword. |
| campaign_criterion_keyword_text | The text of the keyword (at most 80 characters and 10 words). |
| campaign_criterion_last_modified_time | The datetime when this ad group criterion was last modified. The datetime is in the customer's time zone and in "yyyy-MM-dd HH:mm:ss.ssssss" format. |
| campaign_criterion_status | The status of the criterion. |
| campaign_id | The ID of the campaign. |
| customer_id | The ID of the customer. |

Search Ads 360 Table Name: ProductAdvertised

Search Ads 360 API Resource: [shopping_performance_view](https://developers.google.com/search-ads/reporting/api/reference/fields/v0/shopping_performance_view)

| Search Ads 360 Field Name | Description |
|---|---|
| ad_group_id | The ID of the ad group. |
| campaign_id | The ID of the campaign. |
| customer_id | The ID of the customer. |
| metrics_average_cpc | The total cost of all clicks divided by the total number of clicks received. |
| metrics_clicks | The number of clicks. |
| metrics_client_account_conversions | The number of client account conversions. This only includes conversion actions which include_in_client_account_conversions_metric attribute is set to true. If you use conversion-based bidding, your bid strategies will optimize for these conversions. |
| metrics_client_account_conversions_value | The value of client account conversions. This only includes conversion actions which include_in_client_account_conversions_metric attribute is set to true. If you use conversion-based bidding, your bid strategies will optimize for these conversions. |
| metrics_cost_micros | The sum of your cost-per-click (CPC) and cost-per-thousand impressions (CPM) costs during this period. |
| metrics_ctr | The number of clicks your ad receives (Clicks) divided by the number of times your ad is shown (Impressions). |
| metrics_impressions | Count of how often your ad has appeared on a search results page or website on the Google Network. |
| segments_date | Date to which metrics apply, in yyyy-MM-dd format. For example: 2024-04-17 |
| segments_product_bidding_category_level1 | Bidding category (level 1) of the product. |
| segments_product_bidding_category_level2 | Bidding category (level 2) of the product. |
| segments_product_bidding_category_level3 | Bidding category (level 3) of the product. |
| segments_product_bidding_category_level4 | Bidding category (level 4) of the product. |
| segments_product_bidding_category_level5 | Bidding category (level 5) of the product. |
| segments_product_brand | Brand of the product. |
| segments_product_channel | Channel of the product. |
| segments_product_channel_exclusivity | Channel exclusivity of the product. |
| segments_product_condition | Condition of the product. |
| segments_product_country | Resource name of the geo target constant for the country of sale of the product. |
| segments_product_custom_attribute0 | Custom attribute 0 of the product. |
| segments_product_custom_attribute1 | Custom attribute 1 of the product. |
| segments_product_custom_attribute2 | Custom attribute 2 of the product. |
| segments_product_custom_attribute3 | Custom attribute 3 of the product. |
| segments_product_custom_attribute4 | Custom attribute 4 of the product. |
| segments_product_item_id | Item ID of the product. |
| segments_product_language | Resource name of the language constant for the language of the product. |
| segments_product_store_id | Store ID of the product. |
| segments_product_title | Title of the product. |
| segments_product_type_l1 | Type (level 1) of the product. |
| segments_product_type_l2 | Type (level 2) of the product. |
| segments_product_type_l3 | Type (level 3) of the product. |
| segments_product_type_l4 | Type (level 4) of the product. |
| segments_product_type_l5 | Type (level 5) of the product. |

Search Ads 360 Table Name: ProductAdvertisedConversionActionAndDeviceStats

Search Ads 360 API Resource: [shopping_performance_view](https://developers.google.com/search-ads/reporting/api/reference/fields/v0/shopping_performance_view)

| Search Ads 360 Field Name | Description |
|---|---|
| ad_group_id | The ID of the ad group. |
| campaign_id | The ID of the campaign. |
| customer_id | The ID of the customer. |
| metrics_all_conversions | The total number of conversions. This includes all conversions regardless of the value of include_in_conversions_metric. |
| metrics_all_conversions_value | The value of all conversions. |
| metrics_cross_device_conversions | Conversions from when a customer clicks on an ad on one device, then converts on a different device or browser. Cross-device conversions are already included in all_conversions. |
| metrics_cross_device_conversions_value | The sum of the value of cross-device conversions. |
| segments.conversion_action_name | Conversion action name. |
| segments_date | Date to which metrics apply, in yyyy-MM-dd format. For example: 2024-04-17 |
| segments_device | Device to which metrics apply. |
| segments_product_item_id | Item ID of the product. |

Search Ads 360 Table Name: ProductAdvertisedDeviceStats

Search Ads 360 API Resource: [shopping_performance_view](https://developers.google.com/search-ads/reporting/api/reference/fields/v0/shopping_performance_view)

| Search Ads 360 Field Name | Description |
|---|---|
| customer_id | The ID of the customer. |
| metrics_average_cpc | The total cost of all clicks divided by the total number of clicks received. |
| metrics_clicks | The number of clicks. |
| metrics_client_account_conversions | The number of client account conversions. This only includes conversion actions which include_in_client_account_conversions_metric attribute is set to true. If you use conversion-based bidding, your bid strategies will optimize for these conversions. |
| metrics_client_account_conversions_value | The value of client account conversions. This only includes conversion actions which include_in_client_account_conversions_metric attribute is set to true. If you use conversion-based bidding, your bid strategies will optimize for these conversions. |
| metrics_cost_micros | The sum of your cost-per-click (CPC) and cost-per-thousand impressions (CPM) costs during this period. |
| metrics_ctr | The number of clicks your ad receives (Clicks) divided by the number of times your ad is shown (Impressions). |
| metrics_impressions | Count of how often your ad has appeared on a search results page or website on the Google Network. |
| segments_date | Date to which metrics apply, in yyyy-MM-dd format. For example: 2024-04-17 |
| segments_device | Device to which metrics apply. |
| segments_product_item_id | Item ID of the product. |

Search Ads 360 Table Name: ProductGroup

Search Ads 360 API Resource: [product_group_view](https://developers.google.com/search-ads/reporting/api/reference/fields/v0/product_group_view)

| Search Ads 360 Field Name | Description |
|---|---|
| ad_group_criterion_cpc_bid_micros | The CPC (cost-per-click) bid. |
| ad_group_criterion_creation_time | Output only. The timestamp when this ad group criterion was created. The timestamp is in the customer's time zone and in "yyyy-MM-dd HH:mm:ss" format. |
| ad_group_criterion_criterion_id | Output only. The ID of the criterion. |
| ad_group_criterion_effective_cpc_bid_micros | Output only. The effective CPC (cost-per-click) bid. |
| ad_group_criterion_engine_status | Output only. The Engine Status for ad group criterion. |
| ad_group_criterion_final_urls | The list of possible final URLs after all cross-domain redirects for the ad. |
| ad_group_criterion_last_modified_time | Output only. The datetime when this ad group criterion was last modified. The datetime is in the customer's time zone and in "yyyy-MM-dd HH:mm:ss.ssssss" format. |
| ad_group_criterion_listing_group_type | Type of the listing group. |
| ad_group_criterion_status | The status of the criterion. This is the status of the ad group criterion entity, set by the client. Note: UI reports may incorporate additional information that affects whether a criterion is eligible to run. In some cases a criterion that's REMOVED in the API can still show as enabled in the UI. For example, campaigns by default show to users of all age ranges unless excluded. The UI will show each age range as "enabled", since they're eligible to see the ads; but AdGroupCriterion.status will show "removed", since no positive criterion was added. |
| ad_group_criterion_tracking_url_template | The URL template for constructing a tracking URL. |
| ad_group_id | Output only. The ID of the ad group. |
| campaign_id | Output only. The ID of the campaign. |
| customer_id | The ID of the customer. |
| product_group_view_resource_name | Output only. The resource name of the product group view. Product group view resource names have the form: customers/{customer_id}/productGroupViews/{ad_group_id}\~{criterion_id} |

Search Ads 360 Table Name: ProductGroupStats

Search Ads 360 API Resource: [product_group_view](https://developers.google.com/search-ads/reporting/api/reference/fields/v0/product_group_view)

| Search Ads 360 Field Name | Description |
|---|---|
| ad_group_criterion_criterion_id | Output only. The ID of the criterion. |
| ad_group_id | Output only. The ID of the ad group. |
| campaign_id | Output only. The ID of the campaign. |
| customer_id | The ID of the customer. |
| metrics_all_conversions | The total number of conversions. This includes all conversions regardless of the value of include_in_conversions_metric. |
| metrics_all_conversions_value | The value of all conversions. |
| metrics_average_cpc | The total cost of all clicks divided by the total number of clicks received. |
| metrics_average_cpm | Average cost-per-thousand impressions (CPM). |
| metrics_clicks | The number of clicks. |
| metrics_client_account_conversions | The number of client account conversions. This only includes conversion actions which include_in_client_account_conversions_metric attribute is set to true. If you use conversion-based bidding, your bid strategies will optimize for these conversions. |
| metrics_client_account_conversions_value | The value of client account conversions. This only includes conversion actions which include_in_client_account_conversions_metric attribute is set to true. If you use conversion-based bidding, your bid strategies will optimize for these conversions. |
| metrics_client_account_view_through_conversions | The total number of view-through conversions. These happen when a customer sees an image or rich media ad, then later completes a conversion on your site without interacting with (for example, clicking on) another ad. |
| metrics_cost_micros | The sum of your cost-per-click (CPC) and cost-per-thousand impressions (CPM) costs during this period. |
| metrics_cross_device_conversions | Conversions from when a customer clicks on an ad on one device, then converts on a different device or browser. Cross-device conversions are already included in all_conversions. |
| metrics_cross_device_conversions_value | The sum of the value of cross-device conversions. |
| metrics_ctr | The number of clicks your ad receives (Clicks) divided by the number of times your ad is shown (Impressions). |
| metrics_impressions | Count of how often your ad has appeared on a search results page or website on the Google Network. |
| segments_date | Date to which metrics apply, in yyyy-MM-dd format. For example: 2024-04-17 |

Search Ads 360 Table Name: Visit

Search Ads 360 API Resource: [visit](https://developers.google.com/search-ads/reporting/api/reference/fields/v0/visit)

| Search Ads 360 Field Name | Description |
|---|---|
| ad_group_criterion_criterion_id | The ID of the criterion. |
| ad_group_id | The ID of the ad group. |
| campaign_id | The ID of the campaign. |
| customer_account_type | Engine account type. For example: Google Ads, Microsoft Advertising, Yahoo Japan, Baidu, Facebook, Engine Track. |
| customer_descriptive_name | Optional, non-unique descriptive name of the customer. |
| customer_id | The ID of the customer. |
| segments_ad_network_type | Ad network type. |
| segments_date | Date to which metrics apply, in yyyy-MM-dd format. For example: 2024-04-17 |
| segments_device | Device to which metrics apply. |
| visit_ad_id | Ad ID. A value of 0 indicates that the ad is unattributed. |
| visit_click_id | A unique string for each visit that is passed to the landing page as the click id URL parameter. |
| visit_criterion_id | Search Ads 360 keyword ID. A value of 0 indicates that the keyword is unattributed.. |
| visit_id | The ID of the visit. |
| visit_merchant_id | The Search Ads 360 inventory account ID containing the product that was clicked on. Search Ads 360 generates this ID when you link an inventory account in Search Ads 360. |
| visit_product_channel | The sales channel of the product that was clicked on: Online or Local. |
| visit_product_country_code | The country (ISO-3166 format) registered for the inventory feed that contains the product clicked on. |
| visit_product_id | The ID of the product clicked on. |
| visit_product_language_code | The language (ISO-639-1) that has been set for the Merchant Center feed containing data about the product. |
| visit_product_store_id | The store in the Local Inventory Ad that was clicked on. This should match the store IDs used in your local products feed. |
| visit_visit_date_time | The timestamp of the visit event. The timestamp is in the customer's time zone and in "yyyy-MM-dd HH:mm:ss" format. |

Search Ads 360 Table Name: WebpageConversionActionAndDeviceStats

Search Ads 360 API Resource: [webpage_view](https://developers.google.com/search-ads/reporting/api/reference/fields/v0/webpage_view)

| Search Ads 360 Field Name | Description |
|---|---|
| ad_group_criterion_age_range_type | Type of the age range. |
| ad_group_criterion_bid_modifier | The modifier for the bid when the criterion matches. The modifier must be in the range: 0.1 - 10.0. Most targetable criteria types support modifiers. |
| ad_group_criterion_cpc_bid_micros | The CPC (cost-per-click) bid. |
| ad_group_criterion_criterion_id | The ID of the criterion. |
| ad_group_criterion_gender_type | Type of the gender. |
| ad_group_criterion_location_geo_target_constant | The geo target constant resource name. |
| ad_group_criterion_user_list_user_list | The User List resource name. |
| ad_group_criterion_webpage_conditions | Conditions, or logical expressions, for webpage targeting. The list of webpage targeting conditions are and-ed together when evaluated for targeting. An empty list of conditions indicates all pages of the campaign's website are targeted. This field is required for CREATE operations and is prohibited on UPDATE operations. |
| ad_group_criterion_webpage_coverage_percentage | Website criteria coverage percentage. This is the computed percentage of website coverage based on the website target, negative website target and negative keywords in the ad group and campaign. For instance, when coverage returns as 1, it indicates it has 100% coverage. |
| ad_group_id | The ID of the ad group. |
| campaign_id | The ID of the campaign. |
| customer_id | The ID of the customer. |
| metrics_cross_device_conversions | Conversions from when a customer clicks on an ad on one device, then converts on a different device or browser. Cross-device conversions are already included in all_conversions. |
| metrics_cross_device_conversions_value | The sum of the value of cross-device conversions. |
| segments_conversion_action_name | Conversion action name. |
| segments_date | Date to which metrics apply, in yyyy-MM-dd format. For example: 2024-04-17 |
| segments_device | Device to which metrics apply. |

Search Ads 360 Table Name: WebpageDeviceStats

Search Ads 360 API Resource: [webpage_view](https://developers.google.com/search-ads/reporting/api/reference/fields/v0/webpage_view)

| Search Ads 360 Field Name | Description |
|---|---|
| ad_group_criterion_age_range_type | Type of the age range. |
| ad_group_criterion_bid_modifier | The modifier for the bid when the criterion matches. The modifier must be in the range: 0.1 - 10.0. Most targetable criteria types support modifiers. |
| ad_group_criterion_cpc_bid_micros | The CPC (cost-per-click) bid. |
| ad_group_criterion_criterion_id | The ID of the criterion. |
| ad_group_criterion_gender_type | Type of the gender. |
| ad_group_criterion_location_geo_target_constant | The geo target constant resource name. |
| ad_group_criterion_user_list_user_list | The User List resource name. |
| ad_group_criterion_webpage_conditions | Conditions, or logical expressions, for webpage targeting. The list of webpage targeting conditions are and-ed together when evaluated for targeting. An empty list of conditions indicates all pages of the campaign's website are targeted. This field is required for CREATE operations and is prohibited on UPDATE operations. |
| ad_group_criterion_webpage_coverage_percentage | Website criteria coverage percentage. This is the computed percentage of website coverage based on the website target, negative website target and negative keywords in the ad group and campaign. For instance, when coverage returns as 1, it indicates it has 100% coverage. |
| ad_group_id | The ID of the ad group. |
| campaign_id | The ID of the campaign. |
| customer_id | The ID of the customer. |
| metrics_average_cpc | The total cost of all clicks divided by the total number of clicks received. |
| metrics_average_cpm | Average cost-per-thousand impressions (CPM). |
| metrics_clicks | The number of clicks. |
| metrics_client_account_conversions | The number of client account conversions. This only includes conversion actions which include_in_client_account_conversions_metric attribute is set to true. If you use conversion-based bidding, your bid strategies will optimize for these conversions. |
| metrics_client_account_conversions_value | The value of client account conversions. This only includes conversion actions which include_in_client_account_conversions_metric attribute is set to true. If you use conversion-based bidding, your bid strategies will optimize for these conversions. |
| metrics_client_account_view_through_conversions | The total number of view-through conversions. These happen when a customer sees an image or rich media ad, then later completes a conversion on your site without interacting with (for example, clicking on) another ad. |
| metrics_cost_micros | The sum of your cost-per-click (CPC) and cost-per-thousand impressions (CPM) costs during this period. |
| metrics_ctr | The number of clicks your ad receives (Clicks) divided by the number of times your ad is shown (Impressions). |
| metrics_impressions | Count of how often your ad has appeared on a search results page or website on the Google Network. |
| segments_date | Date to which metrics apply, in yyyy-MM-dd format. For example: 2024-04-17 |
| segments_device | Device to which metrics apply. |

<br />

## Search Ads 360 Match Tables

Search Ads 360 Match Tables are tables that contain only **Attribute** fields (fields containing settings or other fixed data), and they are defined for users to query account structure information. If you use the refresh window or schedule a backfill, Match Table snapshots are not updated.

Below is a list of Match Tables in Search Ads 360 transfer:

- Account
- Ad
- AdGroup
- AdGroupCriterion
- Any [ID mapping table](https://docs.cloud.google.com/bigquery/docs/search-ads-transfer#id-mapping)
- Asset
- BidStrategy
- Campaign
- CampaignCriterion
- ConversionAction
- Keyword
- NegativeAdGroupKeyword
- NegativeAdGroupCriterion
- NegativeCampaignKeyword
- NegativeCampaignCriterion
- ProductGroup