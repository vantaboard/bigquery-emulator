# Google Ads report transformation

This document describes how you can transform your reports for
Google Ads (formerly known as Google AdWords).

## Table mapping for Google Ads reports

When your Google Ads reports are transferred
to BigQuery, the reports are transformed into the following
BigQuery tables and views.

When you view the tables and views in BigQuery, the value for
<var translate="no">customer_id</var> is your Google Ads customer ID.

| AdWords reports (deprecated) | BigQuery AdWords tables | Google Ads tables | Google Ads API resources (v22.0.0) | BigQuery views |
|---|---|---|---|---|
| [Account Performance Report](https://developers.google.com/adwords/api/docs/appendix/reports/account-performance-report) | p_Customer_<var translate="no">customer_id</var> p_HourlyAccountConversionStats_<var translate="no">customer_id</var> p_AccountConversionStats_<var translate="no">customer_id</var> p_HourlyAccountStats_<var translate="no">customer_id</var> p_AccountNonClickStats_<var translate="no">customer_id</var> p_AccountBasicStats_<var translate="no">customer_id</var> p_AccountStats_<var translate="no">customer_id</var> | p_ads_Customer_<var translate="no">customer_id</var> p_ads_HourlyAccountConversionStats_<var translate="no">customer_id</var> p_ads_AccountConversionStats_<var translate="no">customer_id</var> p_ads_HourlyAccountStats_<var translate="no">customer_id</var> p_ads_AccountNonClickStats_<var translate="no">customer_id</var> p_ads_AccountBasicStats_<var translate="no">customer_id</var> p_ads_AccountStats_<var translate="no">customer_id</var> | [Customer](https://developers.google.com/google-ads/api/fields/v22/customer) | Customer_<var translate="no">customer_id</var> HourlyAccountConversionStats_<var translate="no">customer_id</var> AccountConversionStats_<var translate="no">customer_id</var> HourlyAccountStats_<var translate="no">customer_id</var> AccountNonClickStats_<var translate="no">customer_id</var> AccountBasicStats_<var translate="no">customer_id</var> AccountStats_<var translate="no">customer_id</var> |
| [Ad Performance Report](https://developers.google.com/adwords/api/docs/appendix/reports/ad-performance-report) | p_AdBasicStats_<var translate="no">customer_id</var> p_AdCrossDeviceStats_<var translate="no">customer_id</var> p_AdConversionStats_<var translate="no">customer_id</var> p_AdStats_<var translate="no">customer_id</var> p_AdCrossDeviceConversionStats_<var translate="no">customer_id</var> p_Ad_<var translate="no">customer_id</var> | p_ads_AdBasicStats_<var translate="no">customer_id</var> p_ads_AdCrossDeviceStats_<var translate="no">customer_id</var> p_ads_AdConversionStats_<var translate="no">customer_id</var> p_ads_AdStats_<var translate="no">customer_id</var> p_ads_AdCrossDeviceConversionStats_<var translate="no">customer_id</var> p_ads_Ad_<var translate="no">customer_id</var> | [Ad Group Ad](https://developers.google.com/google-ads/api/fields/v22/ad_group_ad) | AdBasicStats_<var translate="no">customer_id</var> AdCrossDeviceStats_<var translate="no">customer_id</var> AdConversionStats_<var translate="no">customer_id</var> AdStats_<var translate="no">customer_id</var> AdCrossDeviceConversionStats_<var translate="no">customer_id</var> Ad_<var translate="no">customer_id</var> |
| [Adgroup Performance Report](https://developers.google.com/adwords/api/docs/appendix/reports/adgroup-performance-report) | p_AdGroupStats_<var translate="no">customer_id</var> p_AdGroupBasicStats_<var translate="no">customer_id</var> p_AdGroupCrossDeviceStats_<var translate="no">customer_id</var> p_HourlyAdGroupConversionStats_<var translate="no">customer_id</var> p_HourlyAdGroupStats_<var translate="no">customer_id</var> p_AdGroupConversionStats_<var translate="no">customer_id</var> p_AdGroupCrossDeviceConversionStats_<var translate="no">customer_id</var> p_AdGroup_<var translate="no">customer_id</var> | p_ads_AdGroupStats_<var translate="no">customer_id</var> p_ads_AdGroupBasicStats_<var translate="no">customer_id</var> p_ads_AdGroupCrossDeviceStats_<var translate="no">customer_id</var> p_ads_HourlyAdGroupConversionStats_<var translate="no">customer_id</var> p_ads_HourlyAdGroupStats_<var translate="no">customer_id</var> p_ads_AdGroupConversionStats_<var translate="no">customer_id</var> p_ads_AdGroupCrossDeviceConversionStats_<var translate="no">customer_id</var> p_ads_AdGroup_<var translate="no">customer_id</var> | [Ad Group](https://developers.google.com/google-ads/api/fields/v22/ad_group) | AdGroupStats_<var translate="no">customer_id</var> AdGroupBasicStats_<var translate="no">customer_id</var> AdGroupCrossDeviceStats_<var translate="no">customer_id</var> HourlyAdGroupConversionStats_<var translate="no">customer_id</var> HourlyAdGroupStats_<var translate="no">customer_id</var> AdGroupConversionStats_<var translate="no">customer_id</var> AdGroupCrossDeviceConversionStats_<var translate="no">customer_id</var> AdGroup_<var translate="no">customer_id</var> |
| [Age Range Performance Report](https://developers.google.com/adwords/api/docs/appendix/reports/age-range-performance-report) | p_AgeRange_<var translate="no">customer_id</var> p_AgeRangeBasicStats_<var translate="no">customer_id</var> p_AgeRangeStats_<var translate="no">customer_id</var> p_AgeRangeConversionStats_<var translate="no">customer_id</var> p_AgeRangeNonClickStats_<var translate="no">customer_id</var> | p_ads_AgeRange_<var translate="no">customer_id</var> p_ads_AgeRangeBasicStats_<var translate="no">customer_id</var> p_ads_AgeRangeStats_<var translate="no">customer_id</var> p_ads_AgeRangeConversionStats_<var translate="no">customer_id</var> p_ads_AgeRangeNonClickStats_<var translate="no">customer_id</var> | [Age Range View](https://developers.google.com/google-ads/api/fields/v22/age_range_view) | AgeRange_<var translate="no">customer_id</var> AgeRangeBasicStats_<var translate="no">customer_id</var> AgeRangeStats_<var translate="no">customer_id</var> AgeRangeConversionStats_<var translate="no">customer_id</var> AgeRangeNonClickStats_<var translate="no">customer_id</var> |
| [Audience Performance Report](https://developers.google.com/adwords/api/docs/appendix/reports/audience-performance-report) | p_Audience_<var translate="no">customer_id</var> p_AudienceConversionStats_<var translate="no">customer_id</var> p_AudienceNonClickStats_<var translate="no">customer_id</var> p_AudienceBasicStats_<var translate="no">customer_id</var> p_AudienceStats_<var translate="no">customer_id</var> | `NULL` `NULL` `NULL` `NULL` `NULL` | [Ad Group Audience View](https://developers.google.com/google-ads/api/fields/v22/ad_group_audience_view) [Campaign Audience View](https://developers.google.com/google-ads/api/fields/v22/campaign_audience_view) | Audience_<var translate="no">customer_id</var> AudienceConversionStats_<var translate="no">customer_id</var> AudienceNonClickStats_<var translate="no">customer_id</var> AudienceBasicStats_<var translate="no">customer_id</var> AudienceStats_<var translate="no">customer_id</var> |
| [Bid Goal Performance Report](https://developers.google.com/adwords/api/docs/appendix/reports/bid-goal-performance-report) | p_BidGoal_<var translate="no">customer_id</var> p_BidGoalStats_<var translate="no">customer_id</var> p_HourlyBidGoalStats_<var translate="no">customer_id</var> p_BidGoalConversionStats_<var translate="no">customer_id</var> | p_ads_BidGoal_<var translate="no">customer_id</var> p_ads_BidGoalStats_<var translate="no">customer_id</var> p_ads_HourlyBidGoalStats_<var translate="no">customer_id</var> p_ads_BidGoalConversionStats_<var translate="no">customer_id</var> | [Bidding Strategy](https://developers.google.com/google-ads/api/fields/v22/bidding_strategy) | BidGoal_<var translate="no">customer_id</var> BidGoalStats_<var translate="no">customer_id</var> HourlyBidGoalStats_<var translate="no">customer_id</var> BidGoalConversionStats_<var translate="no">customer_id</var> |
| [Budget Performance Report](https://developers.google.com/adwords/api/docs/appendix/reports/budget-performance-report) | p_Budget_<var translate="no">customer_id</var> p_BudgetStats_<var translate="no">customer_id</var> | p_ads_Budget_<var translate="no">customer_id</var> p_ads_BudgetStats_<var translate="no">customer_id</var> | [Campaign Budget](https://developers.google.com/google-ads/api/fields/v22/campaign_budget) | Budget_<var translate="no">customer_id</var> BudgetStats_<var translate="no">customer_id</var> |
| [Campaign Location Target Report](https://developers.google.com/adwords/api/docs/appendix/reports/campaign-location-target-report) | p_CampaignLocationTargetStats_<var translate="no">customer_id</var> p_LocationBasedCampaignCriterion_<var translate="no">customer_id</var> | p_ads_CampaignLocationTargetStats_<var translate="no">customer_id</var> p_ads_LocationBasedCampaignCriterion_<var translate="no">customer_id</var> | [Location View](https://developers.google.com/google-ads/api/fields/v22/location_view) | CampaignLocationTargetStats_<var translate="no">customer_id</var> LocationBasedCampaignCriterion_<var translate="no">customer_id</var> |
| [Campaign Performance Report](https://developers.google.com/adwords/api/docs/appendix/reports/campaign-performance-report) | p_Campaign_<var translate="no">customer_id</var> p_CampaignBasicStats_<var translate="no">customer_id</var> p_CampaignConversionStats_<var translate="no">customer_id</var> p_CampaignCrossDeviceStats_<var translate="no">customer_id</var> p_HourlyCampaignConversionStats_<var translate="no">customer_id</var> p_CampaignStats_<var translate="no">customer_id</var> p_HourlyCampaignStats_<var translate="no">customer_id</var> p_CampaignCrossDeviceConversionStats_<var translate="no">customer_id</var> p_CampaignCookieStats_<var translate="no">customer_id</var> | p_ads_Campaign_<var translate="no">customer_id</var> p_ads_CampaignBasicStats_<var translate="no">customer_id</var> p_ads_CampaignConversionStats_<var translate="no">customer_id</var> p_ads_CampaignCrossDeviceStats_<var translate="no">customer_id</var> p_ads_HourlyCampaignConversionStats_<var translate="no">customer_id</var> p_ads_CampaignStats_<var translate="no">customer_id</var> p_ads_HourlyCampaignStats_<var translate="no">customer_id</var> p_ads_CampaignCrossDeviceConversionStats_<var translate="no">customer_id</var> p_ads_CampaignCookieStats_<var translate="no">customer_id</var> | [Campaign](https://developers.google.com/google-ads/api/fields/v22/campaign) | Campaign_<var translate="no">customer_id</var> CampaignBasicStats_<var translate="no">customer_id</var> CampaignConversionStats_<var translate="no">customer_id</var> CampaignCrossDeviceStats_<var translate="no">customer_id</var> HourlyCampaignConversionStats_<var translate="no">customer_id</var> CampaignStats_<var translate="no">customer_id</var> HourlyCampaignStats_<var translate="no">customer_id</var> CampaignCrossDeviceConversionStats_<var translate="no">customer_id</var> CampaignCookieStats_<var translate="no">customer_id</var> |
| [Click Performance Report](https://developers.google.com/adwords/api/docs/appendix/reports/click-performance-report) | p_ClickStats_<var translate="no">customer_id</var> | p_ads_ClickStats_<var translate="no">customer_id</var> | [Click View](https://developers.google.com/google-ads/api/fields/v22/click_view) | ClickStats_<var translate="no">customer_id</var> |
| [Criteria Performance Report](https://developers.google.com/adwords/api/docs/appendix/reports/criteria-performance-report) | p_Criteria_<var translate="no">customer_id</var> p_CriteriaBasicStats_<var translate="no">customer_id</var> p_CriteriaStats_<var translate="no">customer_id</var> p_CriteriaConversionStats_<var translate="no">customer_id</var> p_CriteriaNonClickStats_<var translate="no">customer_id</var> | `NULL` `NULL` `NULL` `NULL` `NULL` |   | Criteria_<var translate="no">customer_id</var> CriteriaBasicStats_<var translate="no">customer_id</var> CriteriaStats_<var translate="no">customer_id</var> CriteriaConversionStats_<var translate="no">customer_id</var> CriteriaNonClickStats_<var translate="no">customer_id</var> |
| [Gender Performance Report](https://developers.google.com/adwords/api/docs/appendix/reports/gender-performance-report) | p_Gender_<var translate="no">customer_id</var> p_GenderBasicStats_<var translate="no">customer_id</var> p_GenderStats_<var translate="no">customer_id</var> p_GenderConversionStats_<var translate="no">customer_id</var> p_GenderNonClickStats_<var translate="no">customer_id</var> | p_ads_Gender_<var translate="no">customer_id</var> p_ads_GenderBasicStats_<var translate="no">customer_id</var> p_ads_GenderStats_<var translate="no">customer_id</var> p_ads_GenderConversionStats_<var translate="no">customer_id</var> p_ads_GenderNonClickStats_<var translate="no">customer_id</var> | [Gender View](https://developers.google.com/google-ads/api/fields/v22/gender_view) | Gender_<var translate="no">customer_id</var> GenderBasicStats_<var translate="no">customer_id</var> GenderStats_<var translate="no">customer_id</var> GenderConversionStats_<var translate="no">customer_id</var> GenderNonClickStats_<var translate="no">customer_id</var> |
| [Geo Performance Report](https://developers.google.com/adwords/api/docs/appendix/reports/geo-performance-report) | p_GeoConversionStats_<var translate="no">customer_id</var> p_GeoStats_<var translate="no">customer_id</var> | p_ads_GeoConversionStats_<var translate="no">customer_id</var> p_ads_GeoStats_<var translate="no">customer_id</var> | [Geographic View](https://developers.google.com/google-ads/api/fields/v22/geographic_view) | GeoConversionStats_<var translate="no">customer_id</var> GeoStats_<var translate="no">customer_id</var> |
| [Keywords Performance Report](https://developers.google.com/adwords/api/docs/appendix/reports/keywords-performance-report) | p_Keyword_<var translate="no">customer_id</var> p_KeywordBasicStats_<var translate="no">customer_id</var> p_KeywordCrossDeviceStats_<var translate="no">customer_id</var> p_KeywordStats_<var translate="no">customer_id</var> p_KeywordCrossDeviceConversionStats_<var translate="no">customer_id</var> p_KeywordConversionStats_<var translate="no">customer_id</var> | p_ads_Keyword_<var translate="no">customer_id</var> p_ads_KeywordBasicStats_<var translate="no">customer_id</var> p_ads_KeywordCrossDeviceStats_<var translate="no">customer_id</var> p_ads_KeywordStats_<var translate="no">customer_id</var> p_ads_KeywordCrossDeviceConversionStats_<var translate="no">customer_id</var> p_ads_KeywordConversionStats_<var translate="no">customer_id</var> | [Keyword View](https://developers.google.com/google-ads/api/fields/v22/keyword_view) | Keyword_<var translate="no">customer_id</var> KeywordBasicStats_<var translate="no">customer_id</var> KeywordCrossDeviceStats_<var translate="no">customer_id</var> KeywordStats_<var translate="no">customer_id</var> KeywordCrossDeviceConversionStats_<var translate="no">customer_id</var> KeywordConversionStats_<var translate="no">customer_id</var> |
| [Paid Organic Query Report](https://developers.google.com/adwords/api/docs/appendix/reports/paid-organic-query-report) | p_PaidOrganicStats_<var translate="no">customer_id</var> | p_ads_PaidOrganicStats_<var translate="no">customer_id</var> | [Paid Organic Search Term View](https://developers.google.com/google-ads/api/fields/v22/paid_organic_search_term_view) | PaidOrganicStats_<var translate="no">customer_id</var> |
| [Parental Status Performance Report](https://developers.google.com/adwords/api/docs/appendix/reports/parental-status-performance-report) | p_ParentalStatus_<var translate="no">customer_id</var> p_ParentalStatusBasicStats_<var translate="no">customer_id</var> p_ParentalStatusStats_<var translate="no">customer_id</var> p_ParentalStatusConversionStats_<var translate="no">customer_id</var> p_ParentalStatusNonClickStats_<var translate="no">customer_id</var> | p_ads_ParentalStatus_<var translate="no">customer_id</var> p_ads_ParentalStatusBasicStats_<var translate="no">customer_id</var> p_ads_ParentalStatusStats_<var translate="no">customer_id</var> p_ads_ParentalStatusConversionStats_<var translate="no">customer_id</var> p_ads_ParentalStatusNonClickStats_<var translate="no">customer_id</var> | [Parental Status View](https://developers.google.com/google-ads/api/fields/v22/parental_status_view) | ParentalStatus_<var translate="no">customer_id</var> ParentalStatusBasicStats_<var translate="no">customer_id</var> ParentalStatusStats_<var translate="no">customer_id</var> ParentalStatusConversionStats_<var translate="no">customer_id</var> ParentalStatusNonClickStats_<var translate="no">customer_id</var> |
| [Placement Performance Report](https://developers.google.com/adwords/api/docs/appendix/reports/placement-performance-report) | p_PlacementBasicStats_<var translate="no">customer_id</var> p_PlacementNonClickStats_<var translate="no">customer_id</var> p_PlacementStats_<var translate="no">customer_id</var> p_Placement_<var translate="no">customer_id</var> p_PlacementConversionStats_<var translate="no">customer_id</var> | p_ads_PlacementBasicStats_<var translate="no">customer_id</var> p_ads_PlacementNonClickStats_<var translate="no">customer_id</var> p_ads_PlacementStats_<var translate="no">customer_id</var> p_ads_Placement_<var translate="no">customer_id</var> p_ads_PlacementConversionStats_<var translate="no">customer_id</var> | [Managed Placement View](https://developers.google.com/google-ads/api/fields/v22/managed_placement_view) | PlacementBasicStats_<var translate="no">customer_id</var> PlacementNonClickStats_<var translate="no">customer_id</var> PlacementStats_<var translate="no">customer_id</var> Placement_<var translate="no">customer_id</var> PlacementConversionStats_<var translate="no">customer_id</var> |
| [Search Query Performance Report](https://developers.google.com/adwords/api/docs/appendix/reports/search-query-performance-report) | p_SearchQueryStats_<var translate="no">customer_id</var> p_SearchQueryConversionStats_<var translate="no">customer_id</var> | p_ads_SearchQueryStats_<var translate="no">customer_id</var> p_ads_SearchQueryConversionStats_<var translate="no">customer_id</var> | [Search Term View](https://developers.google.com/google-ads/api/fields/v22/search_term_view) | SearchQueryStats_<var translate="no">customer_id</var> SearchQueryConversionStats_<var translate="no">customer_id</var> |
| [Shopping Performance Report](https://developers.google.com/adwords/api/docs/appendix/reports/shopping-performance-report) | p_ShoppingProductConversionStats_<var translate="no">customer_id</var> p_ShoppingProductStats_<var translate="no">customer_id</var> | p_ads_ShoppingProductConversionStats_<var translate="no">customer_id</var> p_ads_ShoppingProductStats_<var translate="no">customer_id</var> | [Shopping Performance View](https://developers.google.com/google-ads/api/fields/v22/shopping_performance_view) | ShoppingProductConversionStats_<var translate="no">customer_id</var> ShoppingProductStats_<var translate="no">customer_id</var> |
| [Video Performance Report](https://developers.google.com/adwords/api/docs/appendix/reports/video-performance-report) | p_VideoBasicStats_<var translate="no">customer_id</var> p_VideoConversionStats_<var translate="no">customer_id</var> p_VideoStats_<var translate="no">customer_id</var> p_Video_<var translate="no">customer_id</var> p_VideoNonClickStats_<var translate="no">customer_id</var> | p_ads_VideoBasicStats_<var translate="no">customer_id</var> p_ads_VideoConversionStats_<var translate="no">customer_id</var> p_ads_VideoStats_<var translate="no">customer_id</var> p_ads_Video_<var translate="no">customer_id</var> p_ads_VideoNonClickStats_<var translate="no">customer_id</var> | [Video](https://developers.google.com/google-ads/api/fields/v22/video) | VideoBasicStats_<var translate="no">customer_id</var> VideoConversionStats_<var translate="no">customer_id</var> VideoStats_<var translate="no">customer_id</var> Video_<var translate="no">customer_id</var> VideoNonClickStats_<var translate="no">customer_id</var> |
|   |   | AdGroupBidModifier | [Ad Group Bid Modifier](https://developers.google.com/google-ads/api/fields/v22/ad_group_bid_modifier) |   |
|   |   | AdGroupAdLabel | [Ad Group Ad Label](https://developers.google.com/google-ads/api/fields/v22/ad_group_ad_label) |   |
|   |   | CampaignLabel | [Campaign Label](https://developers.google.com/google-ads/api/fields/v22/campaign_label) |   |
|   |   | CampaignCriterion | [Campaign Criterion](https://developers.google.com/google-ads/api/fields/v22/campaign_criterion) |   |
|   |   | AdGroupLabel | [Ad Group Label](https://developers.google.com/google-ads/api/fields/v22/ad_group_label) |   |
|   |   | AdGroupAudience AdGroupAudienceStats AdGroupAudienceConversionStats AdGroupAudienceNonClickStats AdGroupAudienceBasicStats | [Ad Group Audience View](https://developers.google.com/google-ads/api/fields/v22/ad_group_audience_view) |   |
|   |   | Assets (available if [Pmax](https://developers.google.com/google-ads/api/docs/performance-max/overview) data is enabled) | [Assets](https://developers.google.com/google-ads/api/fields/v22/asset) |   |
|   |   | AssetGroup (available if [Pmax](https://developers.google.com/google-ads/api/docs/performance-max/overview) data is enabled) | [Asset Groups](https://developers.google.com/google-ads/api/fields/v22/asset_group) |   |
|   |   | AssetGroupAsset (available if [Pmax](https://developers.google.com/google-ads/api/docs/performance-max/overview) data is enabled) | [Asset Group Assets](https://developers.google.com/google-ads/api/fields/v22/asset_group_asset) |   |
|   |   | AssetGroupSignal (available if [Pmax](https://developers.google.com/google-ads/api/docs/performance-max/overview) data is enabled) | [Asset Group Signal](https://developers.google.com/google-ads/api/fields/v22/asset_group_signal) |   |
|   |   | AssetGroupProductGroupStats (available if [Pmax](https://developers.google.com/google-ads/api/docs/performance-max/overview) data is enabled) | [AssetGroupProductGroupStats](https://developers.google.com/google-ads/api/fields/v22/asset_group_product_group_view) |   |
|   |   | CampaignAssetStats (available if [Pmax](https://developers.google.com/google-ads/api/docs/performance-max/overview) data is enabled) | [CampaignAssetStats](https://developers.google.com/google-ads/api/fields/v22/campaign_asset) |   |

## Column mapping for Google Ads reports

The BigQuery tables created by a Google Ads
transfer consist of the following columns (fields):

Google Ads Table Name: AccountBasicStats


Google Ads API Resource:
[customer](https://developers.google.com/google-ads/api/fields/v22/customer)

| Google Ads Field Name | Description | Adwords Mapped Field Name |
|---|---|---|
| customer_id | The ID of the customer. | ExternalCustomerId |
| metrics_clicks | The number of clicks. | Clicks |
| metrics_conversions | The number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | Conversions |
| metrics_conversions_value | The total value of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | ConversionValue |
| metrics_cost_micros | The sum of your cost-per-click (CPC) and cost-per-thousand impressions (CPM) costs during this period. | Cost |
| metrics_impressions | Count of how often your ad has appeared on a search results page or website on the Google Network. | Impressions |
| metrics_interaction_event_types | The types of payable and free interactions. | InteractionTypes |
| metrics_interactions | The number of interactions. An interaction is the main user action associated with an ad format, such as clicks for text and shopping ads or views for video ads. | Interactions |
| metrics_view_through_conversions | The total number of view-through conversions. These happen when a customer sees an image or rich media ad, then later completes a conversion on your site without interacting with (for example, clicking on) another ad. | ViewThroughConversions |
| segments_ad_network_type | Ad network type. | AdNetworkType2 |
| segments_date | Date to which metrics apply. yyyy-MM-dd format. For example, 2018-04-17. | Date |
| segments_device | Device to which metrics apply. | Device |
| segments_slot | Position of the ad. | Slot |

Google Ads Table Name: AccountConversionStats


Google Ads API Resource:
[customer](https://developers.google.com/google-ads/api/fields/v22/customer)

| Google Ads Field Name | Description | Adwords Mapped Field Name |
|---|---|---|
| customer_id | The ID of the customer. | ExternalCustomerId |
| metrics_conversions | The number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | Conversions |
| metrics_conversions_value | The total value of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | ConversionValue |
| metrics_value_per_conversion | The value of conversions divided by the number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | ValuePerConversion |
| segments_ad_network_type | Ad network type. | AdNetworkType2 |
| segments_click_type | Click type. | ClickType |
| segments_conversion_action | Resource name of the conversion action. | ConversionTrackerId |
| segments_conversion_action_category | Conversion action category. | ConversionCategoryName |
| segments_conversion_action_name | Conversion action name. | ConversionTypeName |
| segments_date | Date to which metrics apply. yyyy-MM-dd format. For example, 2018-04-17. | Date |
| segments_day_of_week | Day of the week. For example, MONDAY. | DayOfWeek |
| segments_device | Device to which metrics apply. | Device |
| segments_month | Month as represented by the date of the first day of a month. Formatted as yyyy-MM-dd. | Month |
| segments_quarter | Quarter as represented by the date of the first day of a quarter. Uses the calendar year for quarters. For example, the second quarter of 2018 starts on 2018-04-01. Formatted as yyyy-MM-dd. | Quarter |
| segments_slot | Position of the ad. | Slot |
| segments_week | Week as defined as Monday through Sunday, and represented by the date of Monday. Formatted as yyyy-MM-dd. | Week |
| segments_year | Year, formatted as yyyy. | Year |

Google Ads Table Name: AccountNonClickStats


Google Ads API Resource:
[customer](https://developers.google.com/google-ads/api/fields/v22/customer)

| Google Ads Field Name | Description | Adwords Mapped Field Name |
|---|---|---|
| customer_id | The ID of the customer. | ExternalCustomerId |
| metrics_all_conversions | The total number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | AllConversions |
| metrics_all_conversions_from_interactions_rate | All conversions from interactions (as oppose to view through conversions) divided by the number of ad interactions. | AllConversionRate |
| metrics_all_conversions_value | The total value of all conversions. | AllConversionValue |
| metrics_average_cpe | The average amount that you've been charged for an ad engagement. This amount is the total cost of all ad engagements divided by the total number of ad engagements. | AverageCpe |
| metrics_trueview_average_cpv | The average amount you pay each time someone views your ad. The average CPV is defined by the total cost of all ad views divided by the number of views. | AverageCpv |
| metrics_content_budget_lost_impression_share | The estimated percent of times that your ad was eligible to show on the Display Network but didn't because your budget was too low. Note: Content budget lost impression share is reported in the range of 0 to 0.9. Any value above 0.9 is reported as 0.9001. | ContentBudgetLostImpressionShare |
| metrics_content_impression_share | The impressions you've received on the Display Network divided by the estimated number of impressions you were eligible to receive. Note: Content impression share is reported in the range of 0.1 to 1. Any value below 0.1 is reported as 0.0999. | ContentImpressionShare |
| metrics_content_rank_lost_impression_share | The estimated percentage of impressions on the Display Network that your ads didn't receive due to poor Ad Rank. Note: Content rank lost impression share is reported in the range of 0 to 0.9. Any value above 0.9 is reported as 0.9001. | ContentRankLostImpressionShare |
| metrics_cost_per_all_conversions | The cost of ad interactions divided by all conversions. | CostPerAllConversion |
| metrics_cross_device_conversions | Conversions from when a customer clicks on a Google Ads ad on one device, then converts on a different device or browser. Cross-device conversions are already included in all_conversions. | CrossDeviceConversions |
| metrics_engagement_rate | How often people engage with your ad after it's shown to them. This is the number of ad expansions divided by the number of times your ad is shown. | EngagementRate |
| metrics_engagements | The number of engagements. An engagement occurs when a viewer expands your Lightbox ad. Also, in the future, other ad types may support engagement metrics. | Engagements |
| metrics_invalid_click_rate | The percentage of clicks filtered out of your total number of clicks (filtered + non-filtered clicks) during the reporting period. | InvalidClickRate |
| metrics_invalid_clicks | Number of clicks Google considers illegitimate and doesn't charge you for. | InvalidClicks |
| metrics_search_budget_lost_impression_share | The estimated percent of times that your ad was eligible to show on the Search Network but didn't because your budget was too low. Note: Search budget lost impression share is reported in the range of 0 to 0.9. Any value above 0.9 is reported as 0.9001. | SearchBudgetLostImpressionShare |
| metrics_search_exact_match_impression_share | The impressions you've received divided by the estimated number of impressions you were eligible to receive on the Search Network for search terms that matched your keywords exactly (or were close variants of your keyword), regardless of your keyword match types. Note: Search exact match impression share is reported in the range of 0.1 to 1. Any value below 0.1 is reported as 0.0999. | SearchExactMatchImpressionShare |
| metrics_search_impression_share | The impressions you've received on the Search Network divided by the estimated number of impressions you were eligible to receive. Note: Search impression share is reported in the range of 0.1 to 1. Any value below 0.1 is reported as 0.0999. | SearchImpressionShare |
| metrics_search_rank_lost_impression_share | The estimated percentage of impressions on the Search Network that your ads didn't receive due to poor Ad Rank. Note: Search rank lost impression share is reported in the range of 0 to 0.9. Any value above 0.9 is reported as 0.9001. | SearchRankLostImpressionShare |
| metrics_value_per_all_conversions | The value of all conversions divided by the number of all conversions. | ValuePerAllConversion |
| metrics_video_trueview_view_rate | The number of views your TrueView video ad receives divided by its number of impressions, including thumbnail impressions for TrueView in-display ads. | VideoViewRate |
| metrics_video_trueview_views | The number of times your video ads were viewed. | VideoViews |
| segments_ad_network_type | Ad network type. | AdNetworkType2 |
| segments_date | Date to which metrics apply. yyyy-MM-dd format. For example, 2018-04-17. | Date |
| segments_day_of_week | Day of the week. For example, MONDAY. | DayOfWeek |
| segments_month | Month as represented by the date of the first day of a month. Formatted as yyyy-MM-dd. | Month |
| segments_quarter | Quarter as represented by the date of the first day of a quarter. Uses the calendar year for quarters. For example, the second quarter of 2018 starts on 2018-04-01. Formatted as yyyy-MM-dd. | Quarter |
| segments_week | Week as defined as Monday through Sunday, and represented by the date of Monday. Formatted as yyyy-MM-dd. | Week |
| segments_year | Year, formatted as yyyy. | Year |

Google Ads Table Name: AccountStats


Google Ads API Resource:
[customer](https://developers.google.com/google-ads/api/fields/v22/customer)

| Google Ads Field Name | Description | Adwords Mapped Field Name |
|---|---|---|
| customer_id | The ID of the customer. | ExternalCustomerId |
| metrics_active_view_cpm | Average cost of viewable impressions (\`active_view_impressions\`). | ActiveViewCpm |
| metrics_active_view_ctr | Active view measurable clicks divided by active view viewable impressions. This metric is reported only for display network. | ActiveViewCtr |
| metrics_active_view_impressions | A measurement of how often your ad has become viewable on a Display Network site. | ActiveViewImpressions |
| metrics_active_view_measurability | The ratio of impressions that could be measured by Active View over the number of served impressions. | ActiveViewMeasurability |
| metrics_active_view_measurable_cost_micros | The cost of the impressions you received that were measurable by Active View. | ActiveViewMeasurableCost |
| metrics_active_view_measurable_impressions | The number of times your ads are appearing on placements in positions where they can be seen. | ActiveViewMeasurableImpressions |
| metrics_active_view_viewability | The percentage of time when your ad appeared on an Active View enabled site (measurable impressions) and was viewable (viewable impressions). | ActiveViewViewability |
| metrics_average_cost | The average amount you pay per interaction. This amount is the total cost of your ads divided by the total number of interactions. | AverageCost |
| metrics_average_cpc | The total cost of all clicks divided by the total number of clicks received. | AverageCpc |
| metrics_average_cpm | Average cost-per-thousand impressions (CPM). | AverageCpm |
| metrics_clicks | The number of clicks. | Clicks |
| metrics_conversions | The number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | Conversions |
| metrics_conversions_from_interactions_rate | Conversions from interactions divided by the number of ad interactions (such as clicks for text ads or views for video ads). This only includes conversion actions which include_in_conversions_metric attribute is set to true. | ConversionRate |
| metrics_conversions_value | The total value of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | ConversionValue |
| metrics_cost_micros | The sum of your cost-per-click (CPC) and cost-per-thousand impressions (CPM) costs during this period. | Cost |
| metrics_cost_per_conversion | The cost of ad interactions divided by conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | CostPerConversion |
| metrics_ctr | The number of clicks your ad receives (Clicks) divided by the number of times your ad is shown (Impressions). | Ctr |
| metrics_impressions | Count of how often your ad has appeared on a search results page or website on the Google Network. | Impressions |
| metrics_interaction_event_types | The types of payable and free interactions. | InteractionTypes |
| metrics_interaction_rate | How often people interact with your ad after it is shown to them. This is the number of interactions divided by the number of times your ad is shown. | InteractionRate |
| metrics_interactions | The number of interactions. An interaction is the main user action associated with an ad format, such as clicks for text and shopping ads or views for video ads. | Interactions |
| metrics_value_per_conversion | The value of conversions divided by the number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | ValuePerConversion |
| segments_ad_network_type | Ad network type. | AdNetworkType2 |
| segments_click_type | Click type. | ClickType |
| segments_date | Date to which metrics apply. yyyy-MM-dd format. For example, 2018-04-17. | Date |
| segments_day_of_week | Day of the week. For example, MONDAY. | DayOfWeek |
| segments_device | Device to which metrics apply. | Device |
| segments_month | Month as represented by the date of the first day of a month. Formatted as yyyy-MM-dd. | Month |
| segments_quarter | Quarter as represented by the date of the first day of a quarter. Uses the calendar year for quarters. For example, the second quarter of 2018 starts on 2018-04-01. Formatted as yyyy-MM-dd. | Quarter |
| segments_week | Week as defined as Monday through Sunday, and represented by the date of Monday. Formatted as yyyy-MM-dd. | Week |
| segments_year | Year, formatted as yyyy. | Year |

Google Ads Table Name: Ad


Google Ads API Resource:
[ad_group_ad](https://developers.google.com/google-ads/api/fields/v22/ad_group_ad)

| Google Ads Field Name | Description | Adwords Mapped Field Name |
|---|---|---|
| ad_group_ad_ad_added_by_google_ads | Indicates if this ad was automatically added by Google Ads and not by a user. For example, this could happen when ads are automatically created as suggestions for new ads based on knowledge of how existing ads are performing. | Automated |
| ad_group_ad_ad_app_ad_descriptions | List of text assets for descriptions. When the ad serves the descriptions, they are selected from this list. | UniversalAppAdDescriptions |
| ad_group_ad_ad_app_ad_headlines | List of text assets for headlines. When the ad serves the headlines is selected from this list. | UniversalAppAdHeadlines |
| ad_group_ad_ad_app_ad_html5_media_bundles | List of media bundle assets that may be used with the ad. | UniversalAppAdHtml5MediaBundles |
| ad_group_ad_ad_app_ad_images | List of image assets that may be displayed with the ad. | UniversalAppAdImages |
| ad_group_ad_ad_app_ad_mandatory_ad_text | An optional text asset that, if specified, must always be displayed when the ad is served. | UniversalAppAdMandatoryAdText |
| ad_group_ad_ad_app_ad_youtube_videos | List of YouTube video assets that may be displayed with the ad. | UniversalAppAdYouTubeVideos |
| ad_group_ad_ad_call_ad_phone_number | The phone number in the ad. | CallOnlyPhoneNumber |
| ad_group_ad_ad_device_preference | The device preference for the ad. You can only specify a preference for mobile devices. When this preference is set, the ad is preferred over other ads when being displayed on a mobile device. The ad can still be displayed on other device types. For example, if no other ads are available. If unspecified (no device preference), all devices are targeted. This is only supported by some ad types. |   |
| ad_group_ad_ad_display_url | The URL that appears in the ad description for some ad formats. | DisplayUrl |
| ad_group_ad_ad_expanded_dynamic_search_ad_description | The description of the ad. |   |
| ad_group_ad_ad_expanded_dynamic_search_ad_description2 | The second description of the ad. | ExpandedDynamicSearchCreativeDescription2 |
| ad_group_ad_ad_expanded_text_ad_description | The description of the ad. |   |
| ad_group_ad_ad_expanded_text_ad_description2 | The second description of the ad. | ExpandedTextAdDescription2 |
| ad_group_ad_ad_expanded_text_ad_headline_part1 | The first part of the ad's headline. | HeadlinePart1 |
| ad_group_ad_ad_expanded_text_ad_headline_part2 | The second part of the ad's headline. | HeadlinePart2 |
| ad_group_ad_ad_expanded_text_ad_headline_part3 | The third part of the ad's headline. | ExpandedTextAdHeadlinePart3 |
| ad_group_ad_ad_expanded_text_ad_path1 | The text that can appear alongside the ad's displayed URL. | Path1 |
| ad_group_ad_ad_expanded_text_ad_path2 | Additional text that can appear alongside the ad's displayed URL. | Path2 |
| ad_group_ad_ad_final_app_urls | A list of final app URLs that are used on mobile if the user has the specific app installed. | CreativeFinalAppUrls |
| ad_group_ad_ad_final_mobile_urls | The list of possible final mobile URLs after all cross-domain redirects for the ad. | CreativeFinalMobileUrls |
| ad_group_ad_ad_final_urls | The list of possible final URLs after all cross-domain redirects for the ad. | CreativeFinalUrls |
| ad_group_ad_ad_group | The ad group to which the ad belongs. |   |
| ad_group_ad_ad_id | The ID of the ad. | CreativeId |
| ad_group_ad_ad_image_ad_image_url | URL of the full size image. | ImageAdUrl |
| ad_group_ad_ad_image_ad_mime_type | The mime type of the image. | ImageCreativeMimeType |
| ad_group_ad_ad_image_ad_name | The name of the image. If the image was created from a MediaFile, this is the MediaFile's name. If the image was created from bytes, this is empty. | ImageCreativeName |
| ad_group_ad_ad_image_ad_pixel_height | Height in pixels of the full size image. | ImageCreativeImageHeight |
| ad_group_ad_ad_image_ad_pixel_width | Width in pixels of the full size image. | ImageCreativeImageWidth |
| ad_group_ad_ad_legacy_responsive_display_ad_accent_color | The accent color of the ad in hexadecimal. For example, #ffffff for white. If one of main_color and accent_color is set, the other is required as well. | AccentColor |
| ad_group_ad_ad_legacy_responsive_display_ad_allow_flexible_color | Advertiser's consent to allow flexible color. When true, the ad may be served with different colors if necessary. When false, the ad is served with the specified colors or a neutral color. The default value is true. Must be true if main_color and accent_color are not set. | AllowFlexibleColor |
| ad_group_ad_ad_legacy_responsive_display_ad_business_name | The business name in the ad. | BusinessName |
| ad_group_ad_ad_legacy_responsive_display_ad_call_to_action_text | The call-to-action text for the ad. | CallToActionText |
| ad_group_ad_ad_legacy_responsive_display_ad_description | The description of the ad. | Description |
| ad_group_ad_ad_legacy_responsive_display_ad_format_setting | Specifies which format the ad is served in. Default is ALL_FORMATS. | FormatSetting |
| ad_group_ad_ad_legacy_responsive_display_ad_logo_image | The MediaFile resource name of the logo image used in the ad. | EnhancedDisplayCreativeLandscapeLogoImageMediaId |
| ad_group_ad_ad_legacy_responsive_display_ad_long_headline | The long version of the ad's headline. | LongHeadline |
| ad_group_ad_ad_legacy_responsive_display_ad_main_color | The main color of the ad in hexadecimal. For example, #ffffff for white. If one of main_color and accent_color is set, the other is required as well. | MainColor |
| ad_group_ad_ad_legacy_responsive_display_ad_marketing_image | The MediaFile resource name of the marketing image used in the ad. | EnhancedDisplayCreativeMarketingImageMediaId |
| ad_group_ad_ad_legacy_responsive_display_ad_price_prefix | Prefix before price. For example, 'as low as'. | PricePrefix |
| ad_group_ad_ad_legacy_responsive_display_ad_promo_text | Promotion text used for dynamic formats of responsive ads. For example 'Free two-day shipping'. | PromoText |
| ad_group_ad_ad_legacy_responsive_display_ad_short_headline | The short version of the ad's headline. | ShortHeadline |
| ad_group_ad_ad_legacy_responsive_display_ad_square_logo_image | The MediaFile resource name of the square logo image used in the ad. | EnhancedDisplayCreativeLogoImageMediaId |
| ad_group_ad_ad_legacy_responsive_display_ad_square_marketing_image | The MediaFile resource name of the square marketing image used in the ad. | EnhancedDisplayCreativeMarketingImageSquareMediaId |
| ad_group_ad_ad_responsive_display_ad_accent_color | The accent color of the ad in hexadecimal. For example, #ffffff for white. If one of main_color and accent_color is set, the other is required as well. | MultiAssetResponsiveDisplayAdAccentColor |
| ad_group_ad_ad_responsive_display_ad_business_name | The advertiser or brand name. Maximum display width is 25. | MultiAssetResponsiveDisplayAdBusinessName |
| ad_group_ad_ad_responsive_display_ad_call_to_action_text | The call-to-action text for the ad. Maximum display width is 30. | MultiAssetResponsiveDisplayAdCallToActionText |
| ad_group_ad_ad_responsive_display_ad_descriptions | Descriptive texts for the ad. The maximum length is 90 characters. At least 1 and max 5 headlines can be specified. | MultiAssetResponsiveDisplayAdDescriptions |
| ad_group_ad_ad_responsive_display_ad_format_setting | Specifies which format the ad is served in. Default is ALL_FORMATS. | MultiAssetResponsiveDisplayAdFormatSetting |
| ad_group_ad_ad_responsive_display_ad_headlines | Short format headlines for the ad. The maximum length is 30 characters. At least 1 and max 5 headlines can be specified. | MultiAssetResponsiveDisplayAdHeadlines |
| ad_group_ad_ad_responsive_display_ad_logo_images | Logo images to be used in the ad. Valid image types are GIF, JPEG, and PNG. The minimum size is 512x128 and the aspect ratio must be 4:1 (+-1%). Combined with square_logo_images the maximum is 5. | MultiAssetResponsiveDisplayAdLandscapeLogoImages |
| ad_group_ad_ad_responsive_display_ad_long_headline | A required long format headline. The maximum length is 90 characters. | MultiAssetResponsiveDisplayAdLongHeadline |
| ad_group_ad_ad_responsive_display_ad_main_color | The main color of the ad in hexadecimal. For example, #ffffff for white. If one of main_color and accent_color is set, the other is required as well. | MultiAssetResponsiveDisplayAdMainColor |
| ad_group_ad_ad_responsive_display_ad_marketing_images | Marketing images to be used in the ad. Valid image types are GIF, JPEG, and PNG. The minimum size is 600x314 and the aspect ratio must be 1.91:1 (+-1%). At least one marketing_image is required. Combined with square_marketing_images the maximum is 15. | MultiAssetResponsiveDisplayAdMarketingImages |
| ad_group_ad_ad_responsive_display_ad_price_prefix | Prefix before price. For example, 'as low as'. | MultiAssetResponsiveDisplayAdDynamicSettingsPricePrefix |
| ad_group_ad_ad_responsive_display_ad_promo_text | Promotion text used for dynamic formats of responsive ads. For example 'Free two-day shipping'. | MultiAssetResponsiveDisplayAdDynamicSettingsPromoText |
| ad_group_ad_ad_responsive_display_ad_square_logo_images | Square logo images to be used in the ad. Valid image types are GIF, JPEG, and PNG. The minimum size is 128x128 and the aspect ratio must be 1:1 (+-1%). Combined with square_logo_images the maximum is 5. | MultiAssetResponsiveDisplayAdLogoImages |
| ad_group_ad_ad_responsive_display_ad_square_marketing_images | Square marketing images to be used in the ad. Valid image types are GIF, JPEG, and PNG. The minimum size is 300x300 and the aspect ratio must be 1:1 (+-1%). At least one square marketing_image is required. Combined with marketing_images the maximum is 15. | MultiAssetResponsiveDisplayAdSquareMarketingImages |
| ad_group_ad_ad_responsive_display_ad_youtube_videos | Optional YouTube videos for the ad. A maximum of 5 videos can be specified. | MultiAssetResponsiveDisplayAdYouTubeVideos |
| ad_group_ad_ad_responsive_search_ad_descriptions | List of text assets for descriptions. When the ad serves the descriptions, they are selected from this list. | ResponsiveSearchAdDescriptions |
| ad_group_ad_ad_responsive_search_ad_headlines | List of text assets for headlines. When the ad serves the headlines, they are selected from this list. | ResponsiveSearchAdHeadlines |
| ad_group_ad_ad_responsive_search_ad_path1 | First part of text that may appear appended to the url displayed in the ad. | ResponsiveSearchAdPath1 |
| ad_group_ad_ad_responsive_search_ad_path2 | Second part of text that may appear appended to the url displayed in the ad. This field can only be set when path1 is also set. | ResponsiveSearchAdPath2 |
| ad_group_ad_ad_strength | Overall ad strength for this ad group ad. | AdStrengthInfo |
| ad_group_ad_ad_system_managed_resource_source | If this ad is system managed, then this field indicates the source. This field is read-only. | SystemManagedEntitySource |
| ad_group_ad_ad_text_ad_description1 | The first line of the ad's description. | Description1 |
| ad_group_ad_ad_text_ad_description2 | The second line of the ad's description. | Description2 |
| ad_group_ad_ad_text_ad_headline | The headline of the ad. | Headline |
| ad_group_ad_ad_tracking_url_template | The URL template for constructing a tracking URL. | CreativeTrackingUrlTemplate |
| ad_group_ad_ad_type | The type of ad. | AdType |
| ad_group_ad_ad_url_custom_parameters | The list of mappings that can be used to substitute custom parameter tags in a \`tracking_url_template\`, \`final_urls\`, or \`mobile_final_urls\`. For mutates, please use url custom parameter operations. | CreativeUrlCustomParameters |
| ad_group_ad_status | The status of the ad. | Status |
| ad_group_id | Output only. The ID of the ad group. | AdGroupId |
| campaign_id | The ID of the campaign. | CampaignId |
| customer_id | The ID of the customer. | ExternalCustomerId |

Google Ads Table Name: AdBasicStats


Google Ads API Resource:
[ad_group_ad](https://developers.google.com/google-ads/api/fields/v22/ad_group_ad)

| Google Ads Field Name | Description | Adwords Mapped Field Name |
|---|---|---|
| ad_group_ad_ad_group | The ad group to which the ad belongs. |   |
| ad_group_ad_ad_id | The ID of the ad. | CreativeId |
| ad_group_base_ad_group | For draft or experiment ad groups, this field is the resource name of the base ad group from which this ad group was created. If a draft or experiment ad group does not have a base ad group, then this field is null. For base ad groups, this field equals the ad group resource name. This field is read-only. | BaseAdGroupId |
| ad_group_id | Output only. The ID of the ad group. | AdGroupId |
| campaign_base_campaign | The resource name of the base campaign of a draft or experiment campaign. For base campaigns, this is equal to \`resource_name\`. This field is read-only. | BaseCampaignId |
| campaign_id | The ID of the campaign. | CampaignId |
| customer_id | The ID of the customer. | ExternalCustomerId |
| metrics_clicks | The number of clicks. | Clicks |
| metrics_conversions | The number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | Conversions |
| metrics_conversions_value | The total value of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | ConversionValue |
| metrics_cost_micros | The sum of your cost-per-click (CPC) and cost-per-thousand impressions (CPM) costs during this period. | Cost |
| metrics_impressions | Count of how often your ad has appeared on a search results page or website on the Google Network. | Impressions |
| metrics_interaction_event_types | The types of payable and free interactions. | InteractionTypes |
| metrics_interactions | The number of interactions. An interaction is the main user action associated with an ad format, such as clicks for text and shopping ads or views for video ads. | Interactions |
| metrics_view_through_conversions | The total number of view-through conversions. These happen when a customer sees an image or rich media ad, then later completes a conversion on your site without interacting with (for example, clicking on) another ad. | ViewThroughConversions |
| segments_ad_network_type | Ad network type. | AdNetworkType2 |
| segments_date | Date to which metrics apply. yyyy-MM-dd format. For example, 2018-04-17. | Date |
| segments_device | Device to which metrics apply. | Device |
| segments_slot | Position of the ad. | Slot |

Google Ads Table Name: AdConversionStats


Google Ads API Resource:
[ad_group_ad](https://developers.google.com/google-ads/api/fields/v22/ad_group_ad)

| Google Ads Field Name | Description | Adwords Mapped Field Name |
|---|---|---|
| ad_group_ad_ad_group | The ad group to which the ad belongs. |   |
| ad_group_ad_ad_id | The ID of the ad. | CreativeId |
| ad_group_base_ad_group | For draft or experiment ad groups, this field is the resource name of the base ad group from which this ad group was created. If a draft or experiment ad group doesn't have a base ad group, then this field is null. For base ad groups, this field equals the ad group resource name. This field is read-only. | BaseAdGroupId |
| ad_group_id | Output only. The ID of the ad group. | AdGroupId |
| campaign_base_campaign | The resource name of the base campaign of a draft or experiment campaign. For base campaigns, this is equal to \`resource_name\`. This field is read-only. | BaseCampaignId |
| campaign_id | The ID of the campaign. | CampaignId |
| customer_id | The ID of the customer. | ExternalCustomerId |
| metrics_conversions | The number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | Conversions |
| metrics_conversions_value | The total value of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | ConversionValue |
| metrics_value_per_conversion | The value of conversions divided by the number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | ValuePerConversion |
| segments_ad_network_type | Ad network type. | AdNetworkType2 |
| segments_click_type | Click type. | ClickType |
| segments_conversion_action | Resource name of the conversion action. | ConversionTrackerId |
| segments_conversion_action_category | Conversion action category. | ConversionCategoryName |
| segments_conversion_action_name | Conversion action name. | ConversionTypeName |
| segments_date | Date to which metrics apply. yyyy-MM-dd format. For example, 2018-04-17. | Date |
| segments_day_of_week | Day of the week. For example, MONDAY. | DayOfWeek |
| segments_device | Device to which metrics apply. | Device |
| segments_month | Month as represented by the date of the first day of a month. Formatted as yyyy-MM-dd. | Month |
| segments_quarter | Quarter as represented by the date of the first day of a quarter. Uses the calendar year for quarters. For example, the second quarter of 2018 starts on 2018-04-01. Formatted as yyyy-MM-dd. | Quarter |
| segments_slot | Position of the ad. | Slot |
| segments_week | Week as defined as Monday through Sunday, and represented by the date of Monday. Formatted as yyyy-MM-dd. | Week |
| segments_year | Year, formatted as yyyy. | Year |

Google Ads Table Name: AdCrossDeviceConversionStats


Google Ads API Resource:
[ad_group_ad](https://developers.google.com/google-ads/api/fields/v22/ad_group_ad)

| Google Ads Field Name | Description | Adwords Mapped Field Name |
|---|---|---|
| ad_group_ad_ad_group | The ad group to which the ad belongs. |   |
| ad_group_ad_ad_id | The ID of the ad. | CreativeId |
| ad_group_base_ad_group | For draft or experiment ad groups, this field is the resource name of the base ad group from which this ad group was created. If a draft or experiment ad group does not have a base ad group, then this field is null. For base ad groups, this field equals the ad group resource name. This field is read-only. | BaseAdGroupId |
| ad_group_id | Output only. The ID of the ad group. | AdGroupId |
| campaign_base_campaign | The resource name of the base campaign of a draft or experiment campaign. For base campaigns, this is equal to \`resource_name\`. This field is read-only. | BaseCampaignId |
| campaign_id | The ID of the campaign. | CampaignId |
| customer_id | The ID of the customer. | ExternalCustomerId |
| metrics_all_conversions | The total number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | AllConversions |
| metrics_all_conversions_value | The total value of all conversions. | AllConversionValue |
| metrics_cross_device_conversions | Conversions from when a customer clicks on a Google Ads ad on one device, then converts on a different device or browser. Cross-device conversions are already included in all_conversions. | CrossDeviceConversions |
| metrics_value_per_all_conversions | The value of all conversions divided by the number of all conversions. | ValuePerAllConversion |
| segments_ad_network_type | Ad network type. | AdNetworkType2 |
| segments_conversion_action | Resource name of the conversion action. | ConversionTrackerId |
| segments_conversion_action_category | Conversion action category. | ConversionCategoryName |
| segments_conversion_action_name | Conversion action name. | ConversionTypeName |
| segments_date | Date to which metrics apply. yyyy-MM-dd format. For example, 2018-04-17. | Date |
| segments_day_of_week | Day of the week. For example, MONDAY. | DayOfWeek |
| segments_month | Month as represented by the date of the first day of a month. Formatted as yyyy-MM-dd. | Month |
| segments_quarter | Quarter as represented by the date of the first day of a quarter. Uses the calendar year for quarters. For example, the second quarter of 2018 starts on 2018-04-01. Formatted as yyyy-MM-dd. | Quarter |
| segments_week | Week as defined as Monday through Sunday, and represented by the date of Monday. Formatted as yyyy-MM-dd. | Week |
| segments_year | Year, formatted as yyyy. | Year |

Google Ads Table Name: AdCrossDeviceStats


Google Ads API Resource:
[ad_group_ad](https://developers.google.com/google-ads/api/fields/v22/ad_group_ad)

| Google Ads Field Name | Description | Adwords Mapped Field Name |
|---|---|---|
| ad_group_ad_ad_group | The ad group to which the ad belongs. |   |
| ad_group_ad_ad_id | The ID of the ad. | CreativeId |
| ad_group_base_ad_group | For draft or experiment ad groups, this field is the resource name of the base ad group from which this ad group was created. If a draft or experiment ad group does not have a base ad group, then this field is null. For base ad groups, this field equals the ad group resource name. This field is read-only. | BaseAdGroupId |
| ad_group_id | Output only. The ID of the ad group. | AdGroupId |
| campaign_base_campaign | The resource name of the base campaign of a draft or experiment campaign. For base campaigns, this is equal to \`resource_name\`. This field is read-only. | BaseCampaignId |
| campaign_id | The ID of the campaign. | CampaignId |
| customer_id | The ID of the customer. | ExternalCustomerId |
| metrics_absolute_top_impression_percentage | The percent of your ad impressions that are shown as the very first ad above the organic search results. | AbsoluteTopImpressionPercentage |
| metrics_all_conversions | The total number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | AllConversions |
| metrics_all_conversions_from_interactions_rate | All conversions from interactions (as oppose to view through conversions) divided by the number of ad interactions. | AllConversionRate |
| metrics_all_conversions_value | The total value of all conversions. | AllConversionValue |
| metrics_average_cpe | The average amount that you've been charged for an ad engagement. This amount is the total cost of all ad engagements divided by the total number of ad engagements. | AverageCpe |
| metrics_trueview_average_cpv | The average amount you pay each time someone views your ad. The average CPV is defined by the total cost of all ad views divided by the number of views. | AverageCpv |
| metrics_average_page_views | Average number of pages viewed per session. | AveragePageviews |
| metrics_average_time_on_site | Total duration of all sessions (in seconds) / number of sessions. Imported from Google Analytics. | AverageTimeOnSite |
| metrics_bounce_rate | Percentage of clicks where the user only visited a single page on your site. Imported from Google Analytics. | BounceRate |
| metrics_cost_per_all_conversions | The cost of ad interactions divided by all conversions. | CostPerAllConversion |
| metrics_cross_device_conversions | Conversions from when a customer clicks on a Google Ads ad on one device, then converts on a different device or browser. Cross-device conversions are already included in all_conversions. | CrossDeviceConversions |
| metrics_engagement_rate | How often people engage with your ad after it's shown to them. This is the number of ad expansions divided by the number of times your ad is shown. | EngagementRate |
| metrics_engagements | The number of engagements. An engagement occurs when a viewer expands your Lightbox ad. Also, in the future, other ad types may support engagement metrics. | Engagements |
| metrics_percent_new_visitors | Percentage of first-time sessions (from people who had never visited your site before). Imported from Google Analytics. | PercentNewVisitors |
| metrics_top_impression_percentage | The percent of your ad impressions that are shown anywhere above the organic search results. | TopImpressionPercentage |
| metrics_value_per_all_conversions | The value of all conversions divided by the number of all conversions. | ValuePerAllConversion |
| metrics_video_quartile_p100_rate | Percentage of impressions where the viewer watched all of your video. | VideoQuartile100Rate |
| metrics_video_quartile_p25_rate | Percentage of impressions where the viewer watched 25% of your video. | VideoQuartile25Rate |
| metrics_video_quartile_p50_rate | Percentage of impressions where the viewer watched 50% of your video. | VideoQuartile50Rate |
| metrics_video_quartile_p75_rate | Percentage of impressions where the viewer watched 75% of your video. | VideoQuartile75Rate |
| metrics_video_trueview_view_rate | The number of views your TrueView video ad receives divided by its number of impressions, including thumbnail impressions for TrueView in-display ads. | VideoViewRate |
| metrics_video_trueview_views | The number of times your video ads were viewed. | VideoViews |
| segments_ad_network_type | Ad network type. | AdNetworkType2 |
| segments_date | Date to which metrics apply. yyyy-MM-dd format. For example, 2018-04-17. | Date |
| segments_day_of_week | Day of the week. For example, MONDAY. | DayOfWeek |
| segments_month | Month as represented by the date of the first day of a month. Formatted as yyyy-MM-dd. | Month |
| segments_quarter | Quarter as represented by the date of the first day of a quarter. Uses the calendar year for quarters. For example, the second quarter of 2018 starts on 2018-04-01. Formatted as yyyy-MM-dd. | Quarter |
| segments_week | Week as defined as Monday through Sunday, and represented by the date of Monday. Formatted as yyyy-MM-dd. | Week |
| segments_year | Year, formatted as yyyy. | Year |

Google Ads Table Name: AdGroup


Google Ads API Resource:
[ad_group](https://developers.google.com/google-ads/api/fields/v22/ad_group)

| Google Ads Field Name | Description | Adwords Mapped Field Name |
|---|---|---|
| ad_group_ad_rotation_mode | The ad rotation mode of the ad group. | AdRotationMode |
| ad_group_cpc_bid_micros | The maximum CPC (cost-per-click) bid. | CpcBid |
| ad_group_cpm_bid_micros | The maximum CPM (cost-per-thousand viewable impressions) bid. | CpmBidStr |
| ad_group_cpv_bid_micros | The CPV (cost-per-view) bid. | CpvBid |
| ad_group_display_custom_bid_dimension | Allows advertisers to specify a targeting dimension on which to place absolute bids. This is only applicable for campaigns that target only the display network and not search. | ContentBidCriterionTypeGroup |
| ad_group_effective_target_cpa_micros | The effective target CPA (cost-per-acquisition). This field is read-only. | TargetCpa |
| ad_group_effective_target_cpa_source | Source of the effective target CPA. This field is read-only. | TargetCpaBidSource |
| ad_group_effective_target_roas | The effective target ROAS (return-on-ad-spend). This field is read-only. | EffectiveTargetRoas |
| ad_group_effective_target_roas_source | Source of the effective target ROAS. This field is read-only. | EffectiveTargetRoasSource |
| ad_group_id | The ID of the ad group. | AdGroupId |
| ad_group_name | The name of the ad group. This field is required and shouldn't be empty when creating new ad groups. It must contain fewer than 255 UTF-8 full-width characters. It must not contain any null (code point 0x0), NL line feed (code point 0xA) or carriage return (code point 0xD) characters. | AdGroupName |
| ad_group_status | The status of the ad group. | AdGroupStatus |
| ad_group_tracking_url_template | The URL template for constructing a tracking URL. | TrackingUrlTemplate |
| ad_group_type | The type of the ad group. | AdGroupType |
| ad_group_url_custom_parameters | The list of mappings used to substitute custom parameter tags in a \`tracking_url_template\`, \`final_urls\`, or \`mobile_final_urls\`. | UrlCustomParameters |
| campaign_bidding_strategy | Portfolio bidding strategy used by campaign. | BiddingStrategyId |
| campaign_bidding_strategy_type | The type of bidding strategy. A bidding strategy can be created by setting either the bidding scheme to create a standard bidding strategy or the \`bidding_strategy\` field to create a portfolio bidding strategy. This field is read-only. | BiddingStrategyType |
| campaign_id | The ID of the campaign. | CampaignId |
| campaign_manual_cpc_enhanced_cpc_enabled | Whether bids are to be enhanced based on conversion optimizer data. | EnhancedCpcEnabled |
| campaign_percent_cpc_enhanced_cpc_enabled | Adjusts the bid for each auction upward or downward, depending on the likelihood of a conversion. Individual bids may exceed cpc_bid_ceiling_micros, but the average bid amount for a campaign shouldn't. |   |
| customer_id | The ID of the customer. | ExternalCustomerId |

Google Ads Table Name: AdGroupAdLabel


Google Ads API Resource:
[ad_group_ad_label](https://developers.google.com/google-ads/api/fields/v22/ad_group_ad_label)

| Google Ads Field Name | Description | Adwords Mapped Field Name |
|---|---|---|
| ad_group_ad_label_ad_group_ad | The ad group ad to which the label is attached. |   |
| ad_group_ad_label_label | The label assigned to the ad group ad. |   |
| ad_group_ad_label_resource_name | The resource name of the ad group ad label. Ad group ad label resource names have the form: \`customers/{customer_id}/adGroupAdLabels/{ad_group_id}\~{ad_id}\~{label_id}\` |   |
| ad_group_id | The ID of the ad group. |   |
| ad_group_name | The name of the ad group. This field is required and shouldn't be empty when creating new ad groups. It must contain fewer than 255 UTF-8 full-width characters. It must not contain any null (code point 0x0), NL line feed (code point 0xA) or carriage return (code point 0xD) characters. |   |
| label_id | Id of the label. Read only. |   |
| label_name | The name of the label. This field is required and shouldn't be empty when creating a new label. The length of this string should be between 1 and 80, inclusive. |   |
| label_resource_name | Name of the resource. Label resource names have the form: \`customers/{customer_id}/labels/{label_id}\` |   |

Google Ads Table Name: AdGroupAudience


Google Ads API Resource:
[ad_group_audience_view](https://developers.google.com/google-ads/api/fields/v22/ad_group_audience_view)

| Google Ads Field Name | Description | Adwords Mapped Field Name |
|---|---|---|
| ad_group_base_ad_group | For draft or experiment ad groups, this field is the resource name of the base ad group from which this ad group was created. If a draft or experiment ad group does not have a base ad group, then this field is null. For base ad groups, this field equals the ad group resource name. This field is read-only. |   |
| ad_group_campaign | The campaign to which the ad group belongs. |   |
| ad_group_criterion_bid_modifier | The modifier for the bid when the criterion matches. The modifier must be in the range: 0.1 - 10.0. Most targetable criteria types support modifiers. |   |
| ad_group_criterion_criterion_id | The ID of the criterion. This field is ignored for mutates. |   |
| ad_group_criterion_effective_cpc_bid_micros | The effective CPC (cost-per-click) bid. |   |
| ad_group_criterion_effective_cpc_bid_source | Source of the effective CPC bid. |   |
| ad_group_criterion_effective_cpm_bid_micros | The effective CPM (cost-per-thousand viewable impressions) bid. |   |
| ad_group_criterion_effective_cpm_bid_source | Source of the effective CPM bid. |   |
| ad_group_criterion_final_mobile_urls | The list of possible final mobile URLs after all cross-domain redirects. |   |
| ad_group_criterion_final_urls | The list of possible final URLs after all cross-domain redirects for the ad. |   |
| ad_group_criterion_keyword_text | The text of the keyword (at most 80 characters and 10 words). |   |
| ad_group_criterion_status | The status of the criterion. |   |
| ad_group_id | The ID of the ad group. |   |
| ad_group_targeting_setting_target_restrictions | The per-targeting-dimension setting to restrict the reach of your campaign or ad group. |   |
| ad_group_tracking_url_template | The URL template for constructing a tracking URL. |   |
| ad_group_url_custom_parameters | The list of mappings used to substitute custom parameter tags in a \`tracking_url_template\`, \`final_urls\`, or \`mobile_final_urls\`. |   |
| campaign_base_campaign | The resource name of the base campaign of a draft or experiment campaign. For base campaigns, this is equal to \`resource_name\`. This field is read-only. |   |
| campaign_bidding_strategy | Portfolio bidding strategy used by campaign. |   |
| customer_id | The ID of the customer. |   |

Google Ads Table Name: AdGroupAudienceBasicStats


Google Ads API Resource:
[ad_group_audience_view](https://developers.google.com/google-ads/api/fields/v22/ad_group_audience_view)

| Google Ads Field Name | Description | Adwords Mapped Field Name |
|---|---|---|
| ad_group_base_ad_group | For draft or experiment ad groups, this field is the resource name of the base ad group from which this ad group was created. If a draft or experiment ad group does not have a base ad group, then this field is null. For base ad groups, this field equals the ad group resource name. This field is read-only. |   |
| ad_group_campaign | The campaign to which the ad group belongs. |   |
| ad_group_criterion_criterion_id | The ID of the criterion. This field is ignored for mutates. |   |
| ad_group_id | The ID of the ad group. |   |
| campaign_base_campaign | The resource name of the base campaign of a draft or experiment campaign. For base campaigns, this is equal to \`resource_name\`. This field is read-only. |   |
| customer_id | The ID of the customer. |   |
| metrics_clicks | The number of clicks. |   |
| metrics_conversions | The number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. |   |
| metrics_conversions_value | The total value of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. |   |
| metrics_cost_micros | The sum of your cost-per-click (CPC) and cost-per-thousand impressions (CPM) costs during this period. |   |
| metrics_impressions | Count of how often your ad has appeared on a search results page or website on the Google Network. |   |
| metrics_interaction_event_types | The types of payable and free interactions. |   |
| metrics_interactions | The number of interactions. An interaction is the main user action associated with an ad format, such as clicks for text and shopping ads or views for video ads. |   |
| metrics_view_through_conversions | The total number of view-through conversions. These happen when a customer sees an image or rich media ad, then later completes a conversion on your site without interacting with (for example, clicking on) another ad. |   |
| segments_ad_network_type | Ad network type. |   |
| segments_date | Date to which metrics apply. yyyy-MM-dd format. For example, 2018-04-17. |   |
| segments_device | Device to which metrics apply. |   |
| segments_slot | Position of the ad. |   |

Google Ads Table Name: AdGroupAudienceConversionStats


Google Ads API Resource:
[ad_group_audience_view](https://developers.google.com/google-ads/api/fields/v22/ad_group_audience_view)

| Google Ads Field Name | Description | Adwords Mapped Field Name |
|---|---|---|
| ad_group_base_ad_group | For draft or experiment ad groups, this field is the resource name of the base ad group from which this ad group was created. If a draft or experiment ad group does not have a base ad group, then this field is null. For base ad groups, this field equals the ad group resource name. This field is read-only. |   |
| ad_group_campaign | The campaign to which the ad group belongs. |   |
| ad_group_criterion_criterion_id | The ID of the criterion. This field is ignored for mutates. |   |
| ad_group_id | The ID of the ad group. |   |
| campaign_base_campaign | The resource name of the base campaign of a draft or experiment campaign. For base campaigns, this is equal to \`resource_name\`. This field is read-only. |   |
| customer_id | The ID of the customer. |   |
| metrics_all_conversions | The total number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. |   |
| metrics_all_conversions_value | The total value of all conversions. |   |
| metrics_conversions | The number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. |   |
| metrics_conversions_value | The total value of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. |   |
| metrics_cross_device_conversions | Conversions from when a customer clicks on a Google Ads ad on one device, then converts on a different device or browser. Cross-device conversions are already included in all_conversions. |   |
| metrics_value_per_all_conversions | The value of all conversions divided by the number of all conversions. |   |
| metrics_value_per_conversion | The value of conversions divided by the number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. |   |
| metrics_view_through_conversions | The total number of view-through conversions. These happen when a customer sees an image or rich media ad, then later completes a conversion on your site without interacting with (for example, clicking on) another ad. |   |
| segments_ad_network_type | Ad network type. |   |
| segments_conversion_action | Resource name of the conversion action. |   |
| segments_conversion_action_category | Conversion action category. |   |
| segments_conversion_action_name | Conversion action name. |   |
| segments_date | Date to which metrics apply. yyyy-MM-dd format. For example, 2018-04-17. |   |
| segments_day_of_week | Day of the week. For example, MONDAY. |   |
| segments_device | Device to which metrics apply. |   |
| segments_month | Month as represented by the date of the first day of a month. Formatted as yyyy-MM-dd. |   |
| segments_quarter | Quarter as represented by the date of the first day of a quarter. Uses the calendar year for quarters. For example, the second quarter of 2018 starts on 2018-04-01. Formatted as yyyy-MM-dd. |   |
| segments_week | Week as defined as Monday through Sunday, and represented by the date of Monday. Formatted as yyyy-MM-dd. |   |
| segments_year | Year, formatted as yyyy. |   |

Google Ads Table Name: AdGroupAudienceNonClickStats


Google Ads API Resource:
[ad_group_audience_view](https://developers.google.com/google-ads/api/fields/v22/ad_group_audience_view)

| Google Ads Field Name | Description | Adwords Mapped Field Name |
|---|---|---|
| ad_group_base_ad_group | For draft or experiment ad groups, this field is the resource name of the base ad group from which this ad group was created. If a draft or experiment ad group does not have a base ad group, then this field is null. For base ad groups, this field equals the ad group resource name. This field is read-only. |   |
| ad_group_campaign | The campaign to which the ad group belongs. |   |
| ad_group_id | The ID of the ad group. |   |
| campaign_base_campaign | The resource name of the base campaign of a draft or experiment campaign. For base campaigns, this is equal to \`resource_name\`. This field is read-only. |   |
| customer_id | The ID of the customer. |   |
| metrics_average_cpe | The average amount that you've been charged for an ad engagement. This amount is the total cost of all ad engagements divided by the total number of ad engagements. |   |
| metrics_trueview_average_cpv | The average amount you pay each time someone views your ad. The average CPV is defined by the total cost of all ad views divided by the number of views. |   |
| metrics_engagement_rate | How often people engage with your ad after it's shown to them. This is the number of ad expansions divided by the number of times your ad is shown. |   |
| metrics_engagements | The number of engagements. An engagement occurs when a viewer expands your Lightbox ad. Also, in the future, other ad types may support engagement metrics. |   |
| metrics_video_trueview_view_rate | The number of views your TrueView video ad receives divided by its number of impressions, including thumbnail impressions for TrueView in-display ads. |   |
| metrics_video_trueview_views | The number of times your video ads were viewed. |   |
| segments_ad_network_type | Ad network type. |   |
| segments_date | Date to which metrics apply. yyyy-MM-dd format. For example, 2018-04-17. |   |
| segments_day_of_week | Day of the week. For example, MONDAY. |   |
| segments_device | Device to which metrics apply. |   |
| segments_month | Month as represented by the date of the first day of a month. Formatted as yyyy-MM-dd. |   |
| segments_quarter | Quarter as represented by the date of the first day of a quarter. Uses the calendar year for quarters. For example, the second quarter of 2018 starts on 2018-04-01. Formatted as yyyy-MM-dd. |   |
| segments_week | Week as defined as Monday through Sunday, and represented by the date of Monday. Formatted as yyyy-MM-dd. |   |
| segments_year | Year, formatted as yyyy. |   |

Google Ads Table Name: AdGroupAudienceStats


Google Ads API Resource:
[ad_group_audience_view](https://developers.google.com/google-ads/api/fields/v22/ad_group_audience_view)

| Google Ads Field Name | Description | Adwords Mapped Field Name |
|---|---|---|
| ad_group_base_ad_group | For draft or experiment ad groups, this field is the resource name of the base ad group from which this ad group was created. If a draft or experiment ad group does not have a base ad group, then this field is null. For base ad groups, this field equals the ad group resource name. This field is read-only. |   |
| ad_group_campaign | The campaign to which the ad group belongs. |   |
| ad_group_criterion_criterion_id | The ID of the criterion. This field is ignored for mutates. |   |
| ad_group_id | The ID of the ad group. |   |
| campaign_base_campaign | The resource name of the base campaign of a draft or experiment campaign. For base campaigns, this is equal to \`resource_name\`. This field is read-only. |   |
| customer_id | The ID of the customer. |   |
| metrics_active_view_cpm | Average cost of viewable impressions (\`active_view_impressions\`). |   |
| metrics_active_view_ctr | Active view measurable clicks divided by active view viewable impressions. This metric is reported only for display network. |   |
| metrics_active_view_impressions | A measurement of how often your ad has become viewable on a Display Network site. |   |
| metrics_active_view_measurability | The ratio of impressions that could be measured by Active View over the number of served impressions. |   |
| metrics_active_view_measurable_cost_micros | The cost of the impressions you received that were measurable by Active View. |   |
| metrics_active_view_measurable_impressions | The number of times your ads are appearing on placements in positions where they can be seen. |   |
| metrics_active_view_viewability | The percentage of time when your ad appeared on an Active View enabled site (measurable impressions) and was viewable (viewable impressions). |   |
| metrics_all_conversions_from_interactions_rate | All conversions from interactions (as oppose to view through conversions) divided by the number of ad interactions. |   |
| metrics_average_cost | The average amount you pay per interaction. This amount is the total cost of your ads divided by the total number of interactions. |   |
| metrics_average_cpc | The total cost of all clicks divided by the total number of clicks received. |   |
| metrics_average_cpm | Average cost-per-thousand impressions (CPM). |   |
| metrics_clicks | The number of clicks. |   |
| metrics_conversions | The number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. |   |
| metrics_conversions_from_interactions_rate | Conversions from interactions divided by the number of ad interactions (such as clicks for text ads or views for video ads). This only includes conversion actions which include_in_conversions_metric attribute is set to true. |   |
| metrics_conversions_value | The total value of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. |   |
| metrics_cost_micros | The sum of your cost-per-click (CPC) and cost-per-thousand impressions (CPM) costs during this period. |   |
| metrics_cost_per_all_conversions | The cost of ad interactions divided by all conversions. |   |
| metrics_cost_per_conversion | The cost of ad interactions divided by conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. |   |
| metrics_ctr | The number of clicks your ad receives (Clicks) divided by the number of times your ad is shown (Impressions). |   |
| metrics_gmail_forwards | The number of times the ad was forwarded to someone else as a message. |   |
| metrics_gmail_saves | The number of times someone has saved your Gmail ad to their inbox as a message. |   |
| metrics_gmail_secondary_clicks | The number of clicks to the landing page on the expanded state of Gmail ads. |   |
| metrics_impressions | Count of how often your ad has appeared on a search results page or website on the Google Network. |   |
| metrics_interaction_event_types | The types of payable and free interactions. |   |
| metrics_interaction_rate | How often people interact with your ad after it is shown to them. This is the number of interactions divided by the number of times your ad is shown. |   |
| metrics_interactions | The number of interactions. An interaction is the main user action associated with an ad format, such as clicks for text and shopping ads or views for video ads. |   |
| metrics_value_per_conversion | The value of conversions divided by the number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. |   |
| segments_ad_network_type | Ad network type. |   |
| segments_click_type | Click type. |   |
| segments_date | Date to which metrics apply. yyyy-MM-dd format. For example, 2018-04-17. |   |
| segments_day_of_week | Day of the week. For example, MONDAY. |   |
| segments_device | Device to which metrics apply. |   |
| segments_month | Month as represented by the date of the first day of a month. Formatted as yyyy-MM-dd. |   |
| segments_quarter | Quarter as represented by the date of the first day of a quarter. Uses the calendar year for quarters. For example, the second quarter of 2018 starts on 2018-04-01. Formatted as yyyy-MM-dd. |   |
| segments_week | Week as defined as Monday through Sunday, and represented by the date of Monday. Formatted as yyyy-MM-dd. |   |
| segments_year | Year, formatted as yyyy. |   |

Google Ads Table Name: AdGroupBasicStats


Google Ads API Resource:
[ad_group](https://developers.google.com/google-ads/api/fields/v22/ad_group)

| Google Ads Field Name | Description | Adwords Mapped Field Name |
|---|---|---|
| ad_group_base_ad_group | For draft or experiment ad groups, this field is the resource name of the base ad group from which this ad group was created. If a draft or experiment ad group does not have a base ad group, then this field is null. For base ad groups, this field equals the ad group resource name. This field is read-only. | BaseAdGroupId |
| ad_group_id | The ID of the ad group. | AdGroupId |
| campaign_base_campaign | The resource name of the base campaign of a draft or experiment campaign. For base campaigns, this is equal to \`resource_name\`. This field is read-only. | BaseCampaignId |
| campaign_id | The ID of the campaign. | CampaignId |
| customer_id | The ID of the customer. | ExternalCustomerId |
| metrics_clicks | The number of clicks. | Clicks |
| metrics_conversions | The number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | Conversions |
| metrics_conversions_value | The total value of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | ConversionValue |
| metrics_cost_micros | The sum of your cost-per-click (CPC) and cost-per-thousand impressions (CPM) costs during this period. | Cost |
| metrics_impressions | Count of how often your ad has appeared on a search results page or website on the Google Network. | Impressions |
| metrics_interaction_event_types | The types of payable and free interactions. | InteractionTypes |
| metrics_interactions | The number of interactions. An interaction is the main user action associated with an ad format, such as clicks for text and shopping ads or views for video ads. | Interactions |
| metrics_view_through_conversions | The total number of view-through conversions. These happen when a customer sees an image or rich media ad, then later completes a conversion on your site without interacting with (for example, clicking on) another ad. | ViewThroughConversions |
| segments_ad_network_type | Ad network type. | AdNetworkType2 |
| segments_date | Date to which metrics apply. yyyy-MM-dd format. For example, 2018-04-17. | Date |
| segments_device | Device to which metrics apply. | Device |
| segments_slot | Position of the ad. | Slot |

Google Ads Table Name: AdGroupBidModifier


Google Ads API Resource:
[ad_group_bid_modifier](https://developers.google.com/google-ads/api/fields/v22/ad_group_bid_modifier)

| Google Ads Field Name | Description | Adwords Mapped Field Name |
|---|---|---|
| ad_group_bid_modifier_ad_group | The ad group to which this criterion belongs. |   |
| ad_group_bid_modifier_base_ad_group | The base ad group from which this draft or trial adgroup bid modifier was created. If ad_group is a base ad group then this field is equal to ad_group. If the ad group was created in the draft or trial and has no corresponding base ad group, then this field is null. This field is read-only. |   |
| ad_group_bid_modifier_bid_modifier | The modifier for the bid when the criterion matches. The modifier must be in the range: 0.1 - 10.0. The range is 1.0 - 6.0 for PreferredContent. Use 0 to opt out of a Device type. |   |
| ad_group_bid_modifier_bid_modifier_source | Bid modifier source. |   |
| ad_group_bid_modifier_criterion_id | The ID of the criterion to bid modify. This field is ignored for mutates. |   |
| ad_group_bid_modifier_device_type | Type of the device. |   |
| ad_group_bid_modifier_resource_name | The resource name of the ad group bid modifier. Ad group bid modifier resource names have the form: \`customers/{customer_id}/adGroupBidModifiers/{ad_group_id}\~{criterion_id}\` |   |
| ad_group_id | The ID of the ad group. |   |
| ad_group_name | The name of the ad group. This field is required and shouldn't be empty when creating new ad groups. It must contain fewer than 255 UTF-8 full-width characters. It must not contain any null (code point 0x0), NL line feed (code point 0xA) or carriage return (code point 0xD) characters. |   |

Google Ads Table Name: AdGroupConversionStats


Google Ads API Resource:
[ad_group](https://developers.google.com/google-ads/api/fields/v22/ad_group)

| Google Ads Field Name | Description | Adwords Mapped Field Name |
|---|---|---|
| ad_group_base_ad_group | For draft or experiment ad groups, this field is the resource name of the base ad group from which this ad group was created. If a draft or experiment ad group does not have a base ad group, then this field is null. For base ad groups, this field equals the ad group resource name. This field is read-only. | BaseAdGroupId |
| ad_group_id | The ID of the ad group. | AdGroupId |
| campaign_base_campaign | The resource name of the base campaign of a draft or experiment campaign. For base campaigns, this is equal to \`resource_name\`. This field is read-only. | BaseCampaignId |
| campaign_id | The ID of the campaign. | CampaignId |
| customer_id | The ID of the customer. | ExternalCustomerId |
| metrics_conversions | The number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | Conversions |
| metrics_conversions_value | The total value of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | ConversionValue |
| metrics_value_per_conversion | The value of conversions divided by the number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | ValuePerConversion |
| segments_ad_network_type | Ad network type. | AdNetworkType2 |
| segments_click_type | Click type. | ClickType |
| segments_conversion_action | Resource name of the conversion action. | ConversionTrackerId |
| segments_conversion_action_category | Conversion action category. | ConversionCategoryName |
| segments_conversion_action_name | Conversion action name. | ConversionTypeName |
| segments_date | Date to which metrics apply. yyyy-MM-dd format. For example, 2018-04-17. | Date |
| segments_day_of_week | Day of the week. For example, MONDAY. | DayOfWeek |
| segments_device | Device to which metrics apply. | Device |
| segments_month | Month as represented by the date of the first day of a month. Formatted as yyyy-MM-dd. | Month |
| segments_quarter | Quarter as represented by the date of the first day of a quarter. Uses the calendar year for quarters. For example, the second quarter of 2018 starts on 2018-04-01. Formatted as yyyy-MM-dd. | Quarter |
| segments_slot | Position of the ad. | Slot |
| segments_week | Week as defined as Monday through Sunday, and represented by the date of Monday. Formatted as yyyy-MM-dd. | Week |
| segments_year | Year, formatted as yyyy. | Year |

Google Ads Table Name: AdGroupCriterion


Google Ads API Resource:
[ad_group_criterion](https://developers.google.com/google-ads/api/fields/v22/ad_group_criterion)

| Google Ads Field Name | Description | Adwords Mapped Field Name |
|---|---|---|
| ad_group_criterion_ad_group | The ad group to which the criterion belongs. |   |
| ad_group_criterion_bid_modifier | The modifier for the bid when the criterion matches. The modifier must be in the range: 0.1 - 10.0. Most targetable criteria types support modifiers. |   |
| ad_group_criterion_cpc_bid_micros | The CPC (cost-per-click) bid. |   |
| ad_group_criterion_cpm_bid_micros | The CPM (cost-per-thousand viewable impressions) bid. |   |
| ad_group_criterion_cpv_bid_micros | The CPC (cost-per-view) bid. |   |
| ad_group_criterion_criterion_id | The ID of the criterion. This field is ignored for mutates. |   |
| ad_group_criterion_display_name | The display name of the criterion. This field is ignored for mutates. |   |
| ad_group_criterion_negative | Whether to target (\`false\`) or exclude (\`true\`) the criterion. |   |
| ad_group_criterion_status | The status of the criterion. |   |
| ad_group_criterion_type | The type of the criterion. |   |
| ad_group_id | The ID of the ad group. |   |
| ad_group_name | The name of the ad group. This field is required and shouldn't be empty when creating new ad groups. It must contain fewer than 255 UTF-8 full-width characters. It must not contain any null (code point 0x0), NL line feed (code point 0xA) or carriage return (code point 0xD) characters. |   |

Google Ads Table Name: AdGroupCriterionLabel


Google Ads API Resource:
[ad_group_criterion_label](https://developers.google.com/google-ads/api/fields/v22/ad_group_criterion_label)

| Google Ads Field Name | Description | Adwords Mapped Field Name |
|---|---|---|
| ad_group_criterion_criterion_id | The ID of the criterion. This field is ignored for mutates. |   |
| ad_group_criterion_label_ad_group_criterion | The ad group criterion to which the label is attached. |   |
| ad_group_criterion_label_label | The label assigned to the ad group criterion. |   |
| ad_group_criterion_label_resource_name | The resource name of the ad group criterion label. Ad group criterion label resource names have the form: \`customers/{customer_id}/adGroupCriterionLabels/{ad_group_id}\~{criterion_id}\~{label_id}\` |   |
| label_id | Id of the label. Read only. |   |
| label_name | The name of the label. This field is required and shouldn't be empty when creating a new label. The length of this string should be between 1 and 80, inclusive. |   |
| label_resource_name | Name of the resource. Label resource names have the form: \`customers/{customer_id}/labels/{label_id}\` |   |

Google Ads Table Name: AdGroupCrossDeviceConversionStats


Google Ads API Resource:
[ad_group](https://developers.google.com/google-ads/api/fields/v22/ad_group)

| Google Ads Field Name | Description | Adwords Mapped Field Name |
|---|---|---|
| ad_group_base_ad_group | For draft or experiment ad groups, this field is the resource name of the base ad group from which this ad group was created. If a draft or experiment ad group does not have a base ad group, then this field is null. For base ad groups, this field equals the ad group resource name. This field is read-only. | BaseAdGroupId |
| ad_group_id | The ID of the ad group. | AdGroupId |
| campaign_base_campaign | The resource name of the base campaign of a draft or experiment campaign. For base campaigns, this is equal to \`resource_name\`. This field is read-only. | BaseCampaignId |
| campaign_id | The ID of the campaign. | CampaignId |
| customer_id | The ID of the customer. | ExternalCustomerId |
| metrics_all_conversions | The total number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | AllConversions |
| metrics_all_conversions_value | The total value of all conversions. | AllConversionValue |
| metrics_cross_device_conversions | Conversions from when a customer clicks on a Google Ads ad on one device, then converts on a different device or browser. Cross-device conversions are already included in all_conversions. | CrossDeviceConversions |
| metrics_value_per_all_conversions | The value of all conversions divided by the number of all conversions. | ValuePerAllConversion |
| segments_ad_network_type | Ad network type. | AdNetworkType2 |
| segments_conversion_action | Resource name of the conversion action. | ConversionTrackerId |
| segments_conversion_action_category | Conversion action category. | ConversionCategoryName |
| segments_conversion_action_name | Conversion action name. | ConversionTypeName |
| segments_date | Date to which metrics apply. yyyy-MM-dd format. For example, 2018-04-17. | Date |
| segments_day_of_week | Day of the week. For example, MONDAY. | DayOfWeek |
| segments_month | Month as represented by the date of the first day of a month. Formatted as yyyy-MM-dd. | Month |
| segments_quarter | Quarter as represented by the date of the first day of a quarter. Uses the calendar year for quarters. For example, the second quarter of 2018 starts on 2018-04-01. Formatted as yyyy-MM-dd. | Quarter |
| segments_week | Week as defined as Monday through Sunday, and represented by the date of Monday. Formatted as yyyy-MM-dd. | Week |
| segments_year | Year, formatted as yyyy. | Year |

Google Ads Table Name: AdGroupCrossDeviceStats


Google Ads API Resource:
[ad_group](https://developers.google.com/google-ads/api/fields/v22/ad_group)

| Google Ads Field Name | Description | Adwords Mapped Field Name |
|---|---|---|
| ad_group_base_ad_group | For draft or experiment ad groups, this field is the resource name of the base ad group from which this ad group was created. If a draft or experiment ad group does not have a base ad group, then this field is null. For base ad groups, this field equals the ad group resource name. This field is read-only. | BaseAdGroupId |
| ad_group_id | The ID of the ad group. | AdGroupId |
| campaign_base_campaign | The resource name of the base campaign of a draft or experiment campaign. For base campaigns, this is equal to \`resource_name\`. This field is read-only. | BaseCampaignId |
| campaign_id | The ID of the campaign. | CampaignId |
| customer_id | The ID of the customer. | ExternalCustomerId |
| metrics_absolute_top_impression_percentage | The percent of your ad impressions that are shown as the very first ad above the organic search results. | AbsoluteTopImpressionPercentage |
| metrics_all_conversions | The total number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | AllConversions |
| metrics_all_conversions_from_interactions_rate | All conversions from interactions (as oppose to view through conversions) divided by the number of ad interactions. | AllConversionRate |
| metrics_all_conversions_value | The total value of all conversions. | AllConversionValue |
| metrics_average_cpe | The average amount that you've been charged for an ad engagement. This amount is the total cost of all ad engagements divided by the total number of ad engagements. | AverageCpe |
| metrics_trueview_average_cpv | The average amount you pay each time someone views your ad. The average CPV is defined by the total cost of all ad views divided by the number of views. | AverageCpv |
| metrics_average_page_views | Average number of pages viewed per session. | AveragePageviews |
| metrics_average_time_on_site | Total duration of all sessions (in seconds) / number of sessions. Imported from Google Analytics. | AverageTimeOnSite |
| metrics_bounce_rate | Percentage of clicks where the user only visited a single page on your site. Imported from Google Analytics. | BounceRate |
| metrics_content_impression_share | The impressions you've received on the Display Network divided by the estimated number of impressions you were eligible to receive. Note: Content impression share is reported in the range of 0.1 to 1. Any value below 0.1 is reported as 0.0999. | ContentImpressionShare |
| metrics_content_rank_lost_impression_share | The estimated percentage of impressions on the Display Network that your ads didn't receive due to poor Ad Rank. Note: Content rank lost impression share is reported in the range of 0 to 0.9. Any value above 0.9 is reported as 0.9001. | ContentRankLostImpressionShare |
| metrics_cost_per_all_conversions | The cost of ad interactions divided by all conversions. | CostPerAllConversion |
| metrics_cross_device_conversions | Conversions from when a customer clicks on a Google Ads ad on one device, then converts on a different device or browser. Cross-device conversions are already included in all_conversions. | CrossDeviceConversions |
| metrics_engagement_rate | How often people engage with your ad after it's shown to them. This is the number of ad expansions divided by the number of times your ad is shown. | EngagementRate |
| metrics_engagements | The number of engagements. An engagement occurs when a viewer expands your Lightbox ad. Also, in the future, other ad types may support engagement metrics. | Engagements |
| metrics_percent_new_visitors | Percentage of first-time sessions (from people who had never visited your site before). Imported from Google Analytics. | PercentNewVisitors |
| metrics_phone_calls | Number of offline phone calls. | NumOfflineInteractions |
| metrics_phone_impressions | Number of offline phone impressions. | NumOfflineImpressions |
| metrics_phone_through_rate | Number of phone calls received (phone_calls) divided by the number of times your phone number is shown (phone_impressions). | OfflineInteractionRate |
| metrics_relative_ctr | Your clickthrough rate (Ctr) divided by the average clickthrough rate of all advertisers on the websites that show your ads. Measures how your ads perform on Display Network sites compared to other ads on the same sites. | RelativeCtr |
| metrics_search_absolute_top_impression_share | The percentage of the customer's Shopping or Search ad impressions that are shown in the most prominent Shopping position. See https://support.google.com/google-ads/answer/7501826 for details. Any value below 0.1 is reported as 0.0999. | SearchAbsoluteTopImpressionShare |
| metrics_search_budget_lost_absolute_top_impression_share | The number estimating how often your ad wasn't the very first ad above the organic search results due to a low budget. Note: Search budget lost absolute top impression share is reported in the range of 0 to 0.9. Any value above 0.9 is reported as 0.9001. | SearchBudgetLostAbsoluteTopImpressionShare |
| metrics_search_budget_lost_top_impression_share | The number estimating how often your ad didn't show anywhere above the organic search results due to a low budget. Note: Search budget lost top impression share is reported in the range of 0 to 0.9. Any value above 0.9 is reported as 0.9001. | SearchBudgetLostTopImpressionShare |
| metrics_search_exact_match_impression_share | The impressions you've received divided by the estimated number of impressions you were eligible to receive on the Search Network for search terms that matched your keywords exactly (or were close variants of your keyword), regardless of your keyword match types. Note: Search exact match impression share is reported in the range of 0.1 to 1. Any value below 0.1 is reported as 0.0999. | SearchExactMatchImpressionShare |
| metrics_search_impression_share | The impressions you've received on the Search Network divided by the estimated number of impressions you were eligible to receive. Note: Search impression share is reported in the range of 0.1 to 1. Any value below 0.1 is reported as 0.0999. | SearchImpressionShare |
| metrics_search_rank_lost_absolute_top_impression_share | The number estimating how often your ad wasn't the very first ad above the organic search results due to poor Ad Rank. Note: Search rank lost absolute top impression share is reported in the range of 0 to 0.9. Any value above 0.9 is reported as 0.9001. | SearchRankLostAbsoluteTopImpressionShare |
| metrics_search_rank_lost_impression_share | The estimated percentage of impressions on the Search Network that your ads didn't receive due to poor Ad Rank. Note: Search rank lost impression share is reported in the range of 0 to 0.9. Any value above 0.9 is reported as 0.9001. | SearchRankLostImpressionShare |
| metrics_search_rank_lost_top_impression_share | The number estimating how often your ad didn't show anywhere above the organic search results due to poor Ad Rank. Note: Search rank lost top impression share is reported in the range of 0 to 0.9. Any value above 0.9 is reported as 0.9001. | SearchRankLostTopImpressionShare |
| metrics_search_top_impression_share | The impressions you've received in the top location (anywhere above the organic search results) compared to the estimated number of impressions you were eligible to receive in the top location. Note: Search top impression share is reported in the range of 0.1 to 1. Any value below 0.1 is reported as 0.0999. | SearchTopImpressionShare |
| metrics_top_impression_percentage | The percent of your ad impressions that are shown anywhere above the organic search results. | TopImpressionPercentage |
| metrics_value_per_all_conversions | The value of all conversions divided by the number of all conversions. | ValuePerAllConversion |
| metrics_video_quartile_p100_rate | Percentage of impressions where the viewer watched all of your video. | VideoQuartile100Rate |
| metrics_video_quartile_p25_rate | Percentage of impressions where the viewer watched 25% of your video. | VideoQuartile25Rate |
| metrics_video_quartile_p50_rate | Percentage of impressions where the viewer watched 50% of your video. | VideoQuartile50Rate |
| metrics_video_quartile_p75_rate | Percentage of impressions where the viewer watched 75% of your video. | VideoQuartile75Rate |
| metrics_video_trueview_view_rate | The number of views your TrueView video ad receives divided by its number of impressions, including thumbnail impressions for TrueView in-display ads. | VideoViewRate |
| metrics_video_trueview_views | The number of times your video ads were viewed. | VideoViews |
| segments_ad_network_type | Ad network type. | AdNetworkType2 |
| segments_date | Date to which metrics apply. yyyy-MM-dd format. For example, 2018-04-17. | Date |
| segments_day_of_week | Day of the week. For example, MONDAY. | DayOfWeek |
| segments_month | Month as represented by the date of the first day of a month. Formatted as yyyy-MM-dd. | Month |
| segments_quarter | Quarter as represented by the date of the first day of a quarter. Uses the calendar year for quarters. For example, the second quarter of 2018 starts on 2018-04-01. Formatted as yyyy-MM-dd. | Quarter |
| segments_week | Week as defined as Monday through Sunday, and represented by the date of Monday. Formatted as yyyy-MM-dd. | Week |
| segments_year | Year, formatted as yyyy. | Year |

Google Ads Table Name: AdGroupLabel


Google Ads API Resource:
[ad_group_label](https://developers.google.com/google-ads/api/fields/v22/ad_group_label)

| Google Ads Field Name | Description | Adwords Mapped Field Name |
|---|---|---|
| ad_group_id | The ID of the ad group. |   |
| ad_group_label_ad_group | The ad group to which the label is attached. |   |
| ad_group_label_label | The label assigned to the ad group. |   |
| ad_group_label_resource_name | The resource name of the ad group label. Ad group label resource names have the form: \`customers/{customer_id}/adGroupLabels/{ad_group_id}\~{label_id}\` |   |
| ad_group_name | The name of the ad group. This field is required and shouldn't be empty when creating new ad groups. It must contain fewer than 255 UTF-8 full-width characters. It must not contain any null (code point 0x0), NL line feed (code point 0xA) or carriage return (code point 0xD) characters. |   |
| label_id | Id of the label. Read only. |   |
| label_name | The name of the label. This field is required and shouldn't be empty when creating a new label. The length of this string should be between 1 and 80, inclusive. |   |
| label_resource_name | Name of the resource. Label resource names have the form: \`customers/{customer_id}/labels/{label_id}\` |   |

Google Ads Table Name: AdGroupStats


Google Ads API Resource:
[ad_group](https://developers.google.com/google-ads/api/fields/v22/ad_group)

| Google Ads Field Name | Description | Adwords Mapped Field Name |
|---|---|---|
| ad_group_base_ad_group | For draft or experiment ad groups, this field is the resource name of the base ad group from which this ad group was created. If a draft or experiment ad group does not have a base ad group, then this field is null. For base ad groups, this field equals the ad group resource name. This field is read-only. | BaseAdGroupId |
| ad_group_id | The ID of the ad group. | AdGroupId |
| campaign_base_campaign | The resource name of the base campaign of a draft or experiment campaign. For base campaigns, this is equal to \`resource_name\`. This field is read-only. | BaseCampaignId |
| campaign_id | The ID of the campaign. | CampaignId |
| customer_id | The ID of the customer. | ExternalCustomerId |
| metrics_active_view_cpm | Average cost of viewable impressions (\`active_view_impressions\`). | ActiveViewCpm |
| metrics_active_view_ctr | Active view measurable clicks divided by active view viewable impressions. This metric is reported only for display network. | ActiveViewCtr |
| metrics_active_view_impressions | A measurement of how often your ad has become viewable on a Display Network site. | ActiveViewImpressions |
| metrics_active_view_measurability | The ratio of impressions that could be measured by Active View over the number of served impressions. | ActiveViewMeasurability |
| metrics_active_view_measurable_cost_micros | The cost of the impressions you received that were measurable by Active View. | ActiveViewMeasurableCost |
| metrics_active_view_measurable_impressions | The number of times your ads are appearing on placements in positions where they can be seen. | ActiveViewMeasurableImpressions |
| metrics_active_view_viewability | The percentage of time when your ad appeared on an Active View enabled site (measurable impressions) and was viewable (viewable impressions). | ActiveViewViewability |
| metrics_average_cost | The average amount you pay per interaction. This amount is the total cost of your ads divided by the total number of interactions. | AverageCost |
| metrics_average_cpc | The total cost of all clicks divided by the total number of clicks received. | AverageCpc |
| metrics_average_cpm | Average cost-per-thousand impressions (CPM). | AverageCpm |
| metrics_clicks | The number of clicks. | Clicks |
| metrics_conversions | The number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | Conversions |
| metrics_conversions_from_interactions_rate | Conversions from interactions divided by the number of ad interactions (such as clicks for text ads or views for video ads). This only includes conversion actions which include_in_conversions_metric attribute is set to true. | ConversionRate |
| metrics_conversions_value | The total value of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | ConversionValue |
| metrics_cost_micros | The sum of your cost-per-click (CPC) and cost-per-thousand impressions (CPM) costs during this period. | Cost |
| metrics_cost_per_conversion | The cost of ad interactions divided by conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | CostPerConversion |
| metrics_cost_per_current_model_attributed_conversion | The cost of ad interactions divided by current model attributed conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | CostPerCurrentModelAttributedConversion |
| metrics_ctr | The number of clicks your ad receives (Clicks) divided by the number of times your ad is shown (Impressions). | Ctr |
| metrics_current_model_attributed_conversions | Shows how your historic conversions data would look under the attribution model you've selected. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | CurrentModelAttributedConversions |
| metrics_current_model_attributed_conversions_value | The total value of current model attributed conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | CurrentModelAttributedConversionValue |
| metrics_gmail_forwards | The number of times the ad was forwarded to someone else as a message. | GmailForwards |
| metrics_gmail_saves | The number of times someone has saved your Gmail ad to their inbox as a message. | GmailSaves |
| metrics_gmail_secondary_clicks | The number of clicks to the landing page on the expanded state of Gmail ads. | GmailSecondaryClicks |
| metrics_impressions | Count of how often your ad has appeared on a search results page or website on the Google Network. | Impressions |
| metrics_interaction_event_types | The types of payable and free interactions. | InteractionTypes |
| metrics_interaction_rate | How often people interact with your ad after it is shown to them. This is the number of interactions divided by the number of times your ad is shown. | InteractionRate |
| metrics_interactions | The number of interactions. An interaction is the main user action associated with an ad format, such as clicks for text and shopping ads or views for video ads. | Interactions |
| metrics_value_per_conversion | The value of conversions divided by the number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | ValuePerConversion |
| metrics_value_per_current_model_attributed_conversion | The value of current model attributed conversions divided by the number of the conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | ValuePerCurrentModelAttributedConversion |
| segments_ad_network_type | Ad network type. | AdNetworkType2 |
| segments_click_type | Click type. | ClickType |
| segments_date | Date to which metrics apply. yyyy-MM-dd format. For example, 2018-04-17. | Date |
| segments_day_of_week | Day of the week. For example, MONDAY. | DayOfWeek |
| segments_device | Device to which metrics apply. | Device |
| segments_month | Month as represented by the date of the first day of a month. Formatted as yyyy-MM-dd. | Month |
| segments_quarter | Quarter as represented by the date of the first day of a quarter. Uses the calendar year for quarters. For example, the second quarter of 2018 starts on 2018-04-01. Formatted as yyyy-MM-dd. | Quarter |
| segments_week | Week as defined as Monday through Sunday, and represented by the date of Monday. Formatted as yyyy-MM-dd. | Week |
| segments_year | Year, formatted as yyyy. | Year |

Google Ads Table Name: AdStats


Google Ads API Resource:
[ad_group_ad](https://developers.google.com/google-ads/api/fields/v22/ad_group_ad)

| Google Ads Field Name | Description | Adwords Mapped Field Name |
|---|---|---|
| ad_group_ad_ad_group | The ad group to which the ad belongs. |   |
| ad_group_ad_ad_id | The ID of the ad. | CreativeId |
| ad_group_base_ad_group | For draft or experiment ad groups, this field is the resource name of the base ad group from which this ad group was created. If a draft or experiment ad group does not have a base ad group, then this field is null. For base ad groups, this field equals the ad group resource name. This field is read-only. | BaseAdGroupId |
| ad_group_id | Output only. The ID of the ad group. | AdGroupId |
| campaign_base_campaign | The resource name of the base campaign of a draft or experiment campaign. For base campaigns, this is equal to \`resource_name\`. This field is read-only. | BaseCampaignId |
| campaign_id | The ID of the campaign. | CampaignId |
| customer_id | The ID of the customer. | ExternalCustomerId |
| metrics_active_view_cpm | Average cost of viewable impressions (\`active_view_impressions\`). | ActiveViewCpm |
| metrics_active_view_ctr | Active view measurable clicks divided by active view viewable impressions. This metric is reported only for display network. | ActiveViewCtr |
| metrics_active_view_impressions | A measurement of how often your ad has become viewable on a Display Network site. | ActiveViewImpressions |
| metrics_active_view_measurability | The ratio of impressions that could be measured by Active View over the number of served impressions. | ActiveViewMeasurability |
| metrics_active_view_measurable_cost_micros | The cost of the impressions you received that were measurable by Active View. | ActiveViewMeasurableCost |
| metrics_active_view_measurable_impressions | The number of times your ads are appearing on placements in positions where they can be seen. | ActiveViewMeasurableImpressions |
| metrics_active_view_viewability | The percentage of time when your ad appeared on an Active View enabled site (measurable impressions) and was viewable (viewable impressions). | ActiveViewViewability |
| metrics_average_cost | The average amount you pay per interaction. This amount is the total cost of your ads divided by the total number of interactions. | AverageCost |
| metrics_average_cpc | The total cost of all clicks divided by the total number of clicks received. | AverageCpc |
| metrics_average_cpm | Average cost-per-thousand impressions (CPM). | AverageCpm |
| metrics_clicks | The number of clicks. | Clicks |
| metrics_conversions | The number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | Conversions |
| metrics_conversions_from_interactions_rate | Conversions from interactions divided by the number of ad interactions (such as clicks for text ads or views for video ads). This only includes conversion actions which include_in_conversions_metric attribute is set to true. | ConversionRate |
| metrics_conversions_value | The total value of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | ConversionValue |
| metrics_cost_micros | The sum of your cost-per-click (CPC) and cost-per-thousand impressions (CPM) costs during this period. | Cost |
| metrics_cost_per_conversion | The cost of ad interactions divided by conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | CostPerConversion |
| metrics_cost_per_current_model_attributed_conversion | The cost of ad interactions divided by current model attributed conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | CostPerCurrentModelAttributedConversion |
| metrics_ctr | The number of clicks your ad receives (Clicks) divided by the number of times your ad is shown (Impressions). | Ctr |
| metrics_current_model_attributed_conversions | Shows how your historic conversions data would look under the attribution model you've selected. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | CurrentModelAttributedConversions |
| metrics_gmail_forwards | The number of times the ad was forwarded to someone else as a message. | GmailForwards |
| metrics_gmail_saves | The number of times someone has saved your Gmail ad to their inbox as a message. | GmailSaves |
| metrics_gmail_secondary_clicks | The number of clicks to the landing page on the expanded state of Gmail ads. | GmailSecondaryClicks |
| metrics_impressions | Count of how often your ad has appeared on a search results page or website on the Google Network. | Impressions |
| metrics_interaction_event_types | The types of payable and free interactions. | InteractionTypes |
| metrics_interaction_rate | How often people interact with your ad after it is shown to them. This is the number of interactions divided by the number of times your ad is shown. | InteractionRate |
| metrics_interactions | The number of interactions. An interaction is the main user action associated with an ad format, such as clicks for text and shopping ads or views for video ads. | Interactions |
| metrics_value_per_conversion | The value of conversions divided by the number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | ValuePerConversion |
| metrics_value_per_current_model_attributed_conversion | The value of current model attributed conversions divided by the number of the conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | ValuePerCurrentModelAttributedConversion |
| segments_ad_network_type | Ad network type. | AdNetworkType2 |
| segments_click_type | Click type. | ClickType |
| segments_date | Date to which metrics apply. yyyy-MM-dd format. For example, 2018-04-17. | Date |
| segments_day_of_week | Day of the week. For example, MONDAY. | DayOfWeek |
| segments_device | Device to which metrics apply. | Device |
| segments_month | Month as represented by the date of the first day of a month. Formatted as yyyy-MM-dd. | Month |
| segments_quarter | Quarter as represented by the date of the first day of a quarter. Uses the calendar year for quarters. For example, the second quarter of 2018 starts on 2018-04-01. Formatted as yyyy-MM-dd. | Quarter |
| segments_week | Week as defined as Monday through Sunday, and represented by the date of Monday. Formatted as yyyy-MM-dd. | Week |
| segments_year | Year, formatted as yyyy. | Year |

Google Ads Table Name: AgeRange


Google Ads API Resource:
[age_range_view](https://developers.google.com/google-ads/api/fields/v22/age_range_view)

| Google Ads Field Name | Description | Adwords Mapped Field Name |
|---|---|---|
| ad_group_base_ad_group | For draft or experiment ad groups, this field is the resource name of the base ad group from which this ad group was created. If a draft or experiment ad group does not have a base ad group, then this field is null. For base ad groups, this field equals the ad group resource name. This field is read-only. | BaseAdGroupId |
| ad_group_criterion_age_range_type | Type of the age range. | Criteria |
| ad_group_criterion_bid_modifier | The modifier for the bid when the criterion matches. The modifier must be in the range: 0.1 - 10.0. Most targetable criteria types support modifiers. | BidModifier |
| ad_group_criterion_criterion_id | The ID of the criterion. This field is ignored for mutates. | CriterionId |
| ad_group_criterion_effective_cpc_bid_micros | The effective CPC (cost-per-click) bid. | CpcBid |
| ad_group_criterion_effective_cpc_bid_source | Source of the effective CPC bid. | CpcBidSource |
| ad_group_criterion_effective_cpm_bid_micros | The effective CPM (cost-per-thousand viewable impressions) bid. | CpmBidStr |
| ad_group_criterion_effective_cpm_bid_source | Source of the effective CPM bid. | CpmBidSource |
| ad_group_criterion_final_mobile_urls | The list of possible final mobile URLs after all cross-domain redirects. | FinalMobileUrls |
| ad_group_criterion_final_urls | The list of possible final URLs after all cross-domain redirects for the ad. | FinalUrls |
| ad_group_criterion_negative | Whether to target (\`false\`) or exclude (\`true\`) the criterion. This field is immutable. To switch a criterion from positive to negative, remove then re-add it. | IsNegative |
| ad_group_criterion_status | The status of the criterion. | Status |
| ad_group_criterion_tracking_url_template | The URL template for constructing a tracking URL. | TrackingUrlTemplate |
| ad_group_criterion_url_custom_parameters | The list of mappings used to substitute custom parameter tags in a \`tracking_url_template\`, \`final_urls\`, or \`mobile_final_urls\`. | UrlCustomParameters |
| ad_group_id | The ID of the ad group. | AdGroupId |
| ad_group_targeting_setting_target_restrictions | The per-targeting-dimension setting to restrict the reach of your campaign or ad group. |   |
| campaign_base_campaign | The resource name of the base campaign of a draft or experiment campaign. For base campaigns, this is equal to \`resource_name\`. This field is read-only. | BaseCampaignId |
| campaign_bidding_strategy | Portfolio bidding strategy used by campaign. | BiddingStrategyId |
| campaign_bidding_strategy_type | The type of bidding strategy. A bidding strategy can be created by setting either the bidding scheme to create a standard bidding strategy or the \`bidding_strategy\` field to create a portfolio bidding strategy. This field is read-only. | BiddingStrategyType |
| campaign_id | The ID of the campaign. | CampaignId |
| customer_id | The ID of the customer. | ExternalCustomerId |

Google Ads Table Name: AgeRangeBasicStats


Google Ads API Resource:
[age_range_view](https://developers.google.com/google-ads/api/fields/v22/age_range_view)

| Google Ads Field Name | Description | Adwords Mapped Field Name |
|---|---|---|
| ad_group_base_ad_group | For draft or experiment ad groups, this field is the resource name of the base ad group from which this ad group was created. If a draft or experiment ad group does not have a base ad group, then this field is null. For base ad groups, this field equals the ad group resource name. This field is read-only. | BaseAdGroupId |
| ad_group_criterion_criterion_id | The ID of the criterion. This field is ignored for mutates. | CriterionId |
| ad_group_id | The ID of the ad group. | AdGroupId |
| campaign_base_campaign | The resource name of the base campaign of a draft or experiment campaign. For base campaigns, this is equal to \`resource_name\`. This field is read-only. | BaseCampaignId |
| campaign_id | The ID of the campaign. | CampaignId |
| customer_id | The ID of the customer. | ExternalCustomerId |
| metrics_active_view_impressions | A measurement of how often your ad has become viewable on a Display Network site. | ActiveViewImpressions |
| metrics_active_view_measurability | The ratio of impressions that could be measured by Active View over the number of served impressions. | ActiveViewMeasurability |
| metrics_active_view_measurable_cost_micros | The cost of the impressions you received that were measurable by Active View. | ActiveViewMeasurableCost |
| metrics_active_view_measurable_impressions | The number of times your ads are appearing on placements in positions where they can be seen. | ActiveViewMeasurableImpressions |
| metrics_active_view_viewability | The percentage of time when your ad appeared on an Active View enabled site (measurable impressions) and was viewable (viewable impressions). | ActiveViewViewability |
| metrics_all_conversions | The total number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | AllConversions |
| metrics_all_conversions_value | The total value of all conversions. | AllConversionValue |
| metrics_clicks | The number of clicks. | Clicks |
| metrics_conversions | The number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | Conversions |
| metrics_conversions_value | The total value of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | ConversionValue |
| metrics_cost_micros | The sum of your cost-per-click (CPC) and cost-per-thousand impressions (CPM) costs during this period. | Cost |
| metrics_cross_device_conversions | Conversions from when a customer clicks on a Google Ads ad on one device, then converts on a different device or browser. Cross-device conversions are already included in all_conversions. | CrossDeviceConversions |
| metrics_impressions | Count of how often your ad has appeared on a search results page or website on the Google Network. | Impressions |
| metrics_interaction_event_types | The types of payable and free interactions. | InteractionTypes |
| metrics_interactions | The number of interactions. An interaction is the main user action associated with an ad format, such as clicks for text and shopping ads or views for video ads. | Interactions |
| metrics_view_through_conversions | The total number of view-through conversions. These happen when a customer sees an image or rich media ad, then later completes a conversion on your site without interacting with (for example, clicking on) another ad. | ViewThroughConversions |
| segments_ad_network_type | Ad network type. | AdNetworkType2 |
| segments_date | Date to which metrics apply. yyyy-MM-dd format. For example, 2018-04-17. | Date |
| segments_device | Device to which metrics apply. | Device |

Google Ads Table Name: AgeRangeConversionStats


Google Ads API Resource:
[age_range_view](https://developers.google.com/google-ads/api/fields/v22/age_range_view)

| Google Ads Field Name | Description | Adwords Mapped Field Name |
|---|---|---|
| ad_group_base_ad_group | For draft or experiment ad groups, this field is the resource name of the base ad group from which this ad group was created. If a draft or experiment ad group does not have a base ad group, then this field is null. For base ad groups, this field equals the ad group resource name. This field is read-only. | BaseAdGroupId |
| ad_group_criterion_criterion_id | The ID of the criterion. This field is ignored for mutates. | CriterionId |
| ad_group_id | The ID of the ad group. | AdGroupId |
| campaign_base_campaign | The resource name of the base campaign of a draft or experiment campaign. For base campaigns, this is equal to \`resource_name\`. This field is read-only. | BaseCampaignId |
| campaign_id | The ID of the campaign. | CampaignId |
| customer_id | The ID of the customer. | ExternalCustomerId |
| metrics_all_conversions | The total number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | AllConversions |
| metrics_all_conversions_value | The total value of all conversions. | AllConversionValue |
| metrics_conversions | The number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | Conversions |
| metrics_conversions_value | The total value of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | ConversionValue |
| metrics_cross_device_conversions | Conversions from when a customer clicks on a Google Ads ad on one device, then converts on a different device or browser. Cross-device conversions are already included in all_conversions. | CrossDeviceConversions |
| metrics_value_per_all_conversions | The value of all conversions divided by the number of all conversions. | ValuePerAllConversion |
| metrics_value_per_conversion | The value of conversions divided by the number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | ValuePerConversion |
| segments_ad_network_type | Ad network type. | AdNetworkType2 |
| segments_click_type | Click type. | ClickType |
| segments_conversion_action | Resource name of the conversion action. | ConversionTrackerId |
| segments_conversion_action_category | Conversion action category. | ConversionCategoryName |
| segments_conversion_action_name | Conversion action name. | ConversionTypeName |
| segments_date | Date to which metrics apply. yyyy-MM-dd format. For example, 2018-04-17. | Date |
| segments_day_of_week | Day of the week. For example, MONDAY. | DayOfWeek |
| segments_device | Device to which metrics apply. | Device |
| segments_month | Month as represented by the date of the first day of a month. Formatted as yyyy-MM-dd. | Month |
| segments_quarter | Quarter as represented by the date of the first day of a quarter. Uses the calendar year for quarters. For example, the second quarter of 2018 starts on 2018-04-01. Formatted as yyyy-MM-dd. | Quarter |
| segments_week | Week as defined as Monday through Sunday, and represented by the date of Monday. Formatted as yyyy-MM-dd. | Week |
| segments_year | Year, formatted as yyyy. | Year |

Google Ads Table Name: AgeRangeNonClickStats


Google Ads API Resource:
[age_range_view](https://developers.google.com/google-ads/api/fields/v22/age_range_view)

| Google Ads Field Name | Description | Adwords Mapped Field Name |
|---|---|---|
| ad_group_base_ad_group | For draft or experiment ad groups, this field is the resource name of the base ad group from which this ad group was created. If a draft or experiment ad group does not have a base ad group, then this field is null. For base ad groups, this field equals the ad group resource name. This field is read-only. | BaseAdGroupId |
| ad_group_criterion_criterion_id | The ID of the criterion. This field is ignored for mutates. | CriterionId |
| ad_group_id | The ID of the ad group. | AdGroupId |
| campaign_base_campaign | The resource name of the base campaign of a draft or experiment campaign. For base campaigns, this is equal to \`resource_name\`. This field is read-only. | BaseCampaignId |
| campaign_id | The ID of the campaign. | CampaignId |
| customer_id | The ID of the customer. | ExternalCustomerId |
| metrics_average_cpe | The average amount that you've been charged for an ad engagement. This amount is the total cost of all ad engagements divided by the total number of ad engagements. | AverageCpe |
| metrics_trueview_average_cpv | The average amount you pay each time someone views your ad. The average CPV is defined by the total cost of all ad views divided by the number of views. | AverageCpv |
| metrics_engagement_rate | How often people engage with your ad after it's shown to them. This is the number of ad expansions divided by the number of times your ad is shown. | EngagementRate |
| metrics_engagements | The number of engagements. An engagement occurs when a viewer expands your Lightbox ad. Also, in the future, other ad types may support engagement metrics. | Engagements |
| metrics_video_quartile_p100_rate | Percentage of impressions where the viewer watched all of your video. | VideoQuartile100Rate |
| metrics_video_quartile_p25_rate | Percentage of impressions where the viewer watched 25% of your video. | VideoQuartile25Rate |
| metrics_video_quartile_p50_rate | Percentage of impressions where the viewer watched 50% of your video. | VideoQuartile50Rate |
| metrics_video_quartile_p75_rate | Percentage of impressions where the viewer watched 75% of your video. | VideoQuartile75Rate |
| metrics_video_trueview_view_rate | The number of views your TrueView video ad receives divided by its number of impressions, including thumbnail impressions for TrueView in-display ads. | VideoViewRate |
| metrics_video_trueview_views | The number of times your video ads were viewed. | VideoViews |
| segments_ad_network_type | Ad network type. | AdNetworkType2 |
| segments_date | Date to which metrics apply. yyyy-MM-dd format. For example, 2018-04-17. | Date |
| segments_day_of_week | Day of the week. For example, MONDAY. | DayOfWeek |
| segments_device | Device to which metrics apply. | Device |
| segments_month | Month as represented by the date of the first day of a month. Formatted as yyyy-MM-dd. | Month |
| segments_quarter | Quarter as represented by the date of the first day of a quarter. Uses the calendar year for quarters. For example, the second quarter of 2018 starts on 2018-04-01. Formatted as yyyy-MM-dd. | Quarter |
| segments_week | Week as defined as Monday through Sunday, and represented by the date of Monday. Formatted as yyyy-MM-dd. | Week |
| segments_year | Year, formatted as yyyy. | Year |

Google Ads Table Name: AgeRangeStats


Google Ads API Resource:
[age_range_view](https://developers.google.com/google-ads/api/fields/v22/age_range_view)

| Google Ads Field Name | Description | Adwords Mapped Field Name |
|---|---|---|
| ad_group_base_ad_group | For draft or experiment ad groups, this field is the resource name of the base ad group from which this ad group was created. If a draft or experiment ad group does not have a base ad group, then this field is null. For base ad groups, this field equals the ad group resource name. This field is read-only. | BaseAdGroupId |
| ad_group_criterion_criterion_id | The ID of the criterion. This field is ignored for mutates. | CriterionId |
| ad_group_id | The ID of the ad group. | AdGroupId |
| campaign_base_campaign | The resource name of the base campaign of a draft or experiment campaign. For base campaigns, this is equal to \`resource_name\`. This field is read-only. | BaseCampaignId |
| campaign_id | The ID of the campaign. | CampaignId |
| customer_id | The ID of the customer. | ExternalCustomerId |
| metrics_active_view_cpm | Average cost of viewable impressions (\`active_view_impressions\`). | ActiveViewCpm |
| metrics_active_view_ctr | Active view measurable clicks divided by active view viewable impressions. This metric is reported only for display network. | ActiveViewCtr |
| metrics_active_view_impressions | A measurement of how often your ad has become viewable on a Display Network site. | ActiveViewImpressions |
| metrics_active_view_measurability | The ratio of impressions that could be measured by Active View over the number of served impressions. | ActiveViewMeasurability |
| metrics_active_view_measurable_cost_micros | The cost of the impressions you received that were measurable by Active View. | ActiveViewMeasurableCost |
| metrics_active_view_measurable_impressions | The number of times your ads are appearing on placements in positions where they can be seen. | ActiveViewMeasurableImpressions |
| metrics_active_view_viewability | The percentage of time when your ad appeared on an Active View enabled site (measurable impressions) and was viewable (viewable impressions). | ActiveViewViewability |
| metrics_all_conversions | The total number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | AllConversions |
| metrics_all_conversions_from_interactions_rate | All conversions from interactions (as oppose to view through conversions) divided by the number of ad interactions. | AllConversionRate |
| metrics_all_conversions_value | The total value of all conversions. | AllConversionValue |
| metrics_average_cost | The average amount you pay per interaction. This amount is the total cost of your ads divided by the total number of interactions. | AverageCost |
| metrics_average_cpc | The total cost of all clicks divided by the total number of clicks received. | AverageCpc |
| metrics_average_cpm | Average cost-per-thousand impressions (CPM). | AverageCpm |
| metrics_clicks | The number of clicks. | Clicks |
| metrics_conversions | The number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | Conversions |
| metrics_conversions_from_interactions_rate | Conversions from interactions divided by the number of ad interactions (such as clicks for text ads or views for video ads). This only includes conversion actions which include_in_conversions_metric attribute is set to true. | ConversionRate |
| metrics_conversions_value | The total value of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | ConversionValue |
| metrics_cost_micros | The sum of your cost-per-click (CPC) and cost-per-thousand impressions (CPM) costs during this period. | Cost |
| metrics_cost_per_all_conversions | The cost of ad interactions divided by all conversions. | CostPerAllConversion |
| metrics_cost_per_conversion | The cost of ad interactions divided by conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | CostPerConversion |
| metrics_cross_device_conversions | Conversions from when a customer clicks on a Google Ads ad on one device, then converts on a different device or browser. Cross-device conversions are already included in all_conversions. | CrossDeviceConversions |
| metrics_ctr | The number of clicks your ad receives (Clicks) divided by the number of times your ad is shown (Impressions). | Ctr |
| metrics_gmail_forwards | The number of times the ad was forwarded to someone else as a message. | GmailForwards |
| metrics_gmail_saves | The number of times someone has saved your Gmail ad to their inbox as a message. | GmailSaves |
| metrics_gmail_secondary_clicks | The number of clicks to the landing page on the expanded state of Gmail ads. | GmailSecondaryClicks |
| metrics_impressions | Count of how often your ad has appeared on a search results page or website on the Google Network. | Impressions |
| metrics_interaction_event_types | The types of payable and free interactions. | InteractionTypes |
| metrics_interaction_rate | How often people interact with your ad after it is shown to them. This is the number of interactions divided by the number of times your ad is shown. | InteractionRate |
| metrics_interactions | The number of interactions. An interaction is the main user action associated with an ad format, such as clicks for text and shopping ads, or views for video ads. | Interactions |
| metrics_value_per_all_conversions | The value of all conversions divided by the number of all conversions. | ValuePerAllConversion |
| metrics_value_per_conversion | The value of conversions divided by the number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | ValuePerConversion |
| segments_ad_network_type | Ad network type. | AdNetworkType2 |
| segments_click_type | Click type. | ClickType |
| segments_date | Date to which metrics apply. yyyy-MM-dd format. For example, 2018-04-17. | Date |
| segments_day_of_week | Day of the week. For example, MONDAY. | DayOfWeek |
| segments_device | Device to which metrics apply. | Device |
| segments_month | Month as represented by the date of the first day of a month. Formatted as yyyy-MM-dd. | Month |
| segments_quarter | Quarter as represented by the date of the first day of a quarter. Uses the calendar year for quarters. For example, the second quarter of 2018 starts on 2018-04-01. Formatted as yyyy-MM-dd. | Quarter |
| segments_week | Week as defined as Monday through Sunday, and represented by the date of Monday. Formatted as yyyy-MM-dd. | Week |
| segments_year | Year, formatted as yyyy. | Year |

Google Ads Table Name: Asset


Google Ads API Resource:
[asset](https://developers.google.com/google-ads/api/fields/v22/asset)

| Google Ads Field Name | Description | Adwords Mapped Field Name |
|---|---|---|
| asset_final_urls | Output only. Type of the asset. |   |
| asset_name | Optional name of the asset. |   |
| asset_source | Output only. Source of the asset. |   |
| asset_type | A list of possible final URLs after all cross domain redirects. |   |
| segments_conversion_action | Resource name of the conversion action. |   |

Google Ads Table Name: AssetGroup


Google Ads API Resource:
[asset_group](https://developers.google.com/google-ads/api/fields/v22/asset_group)

| Google Ads Field Name | Description | Adwords Mapped Field Name |
|---|---|---|
| asset_group_campaign | Immutable. The campaign with which this asset group is associated. The asset which is linked to the asset group. |   |
| asset_group_final_mobile_urls | A list of final mobile URLs after all cross domain redirects. In performance max, by default, the urls are eligible for expansion unless opted out. |   |
| asset_group_final_urls | A list of final URLs after all cross domain redirects. In performance max, by default, the urls are eligible for expansion unless opted out. |   |
| asset_group_id | Output only. The ID of the asset group. |   |
| asset_group_name | Required. Name of the asset group. Required. It must have a minimum length of 1 and maximum length of 128. It must be unique under a campaign. |   |
| asset_group_status | The status of the asset group. |   |

Google Ads Table Name: AssetGroupAsset


Google Ads API Resource:
[asset_group_asset](https://developers.google.com/google-ads/api/fields/v22/asset_group_asset)

| Google Ads Field Name | Description | Adwords Mapped Field Name |
|---|---|---|
| asset_group_asset_asset | The asset which this asset group asset is linking. |   |
| asset_group_asset_asset_group | The asset group which this asset group asset is linking. |   |
| asset_group_asset_field_type | The description of the placement of the asset within the asset group. For example: HEADLINE, YOUTUBE_VIDEO. |   |
| asset_group_asset_performance_label | The performance of this asset group asset. |   |
| asset_group_asset_policy_summary_approval_status | The overall approval status, which is calculated based on the status of its individual policy topic entries. |   |
| asset_group_asset_policy_summary_policy_topic_entries | The list of policy findings. |   |
| asset_group_asset_policy_summary_review_status | Where in the review process the resource is. |   |

Google Ads Table Name: AssetGroupListingGroupFilter


Google Ads API Resource:
[asset_group_listing_group_filter](https://developers.google.com/google-ads/api/fields/v22/asset_group_listing_group_filter)

| Google Ads Field Name | Description | Adwords Mapped Field Name |
|---|---|---|
| asset_group_listing_group_filter_asset_group | The asset group which this asset group listing group filter is part of. |   |
| asset_group_listing_group_filter_case_value_product_bidding_category_id (obsolete - use asset_group_listing_group_filter_case_value_product_category_category_id instead) | ID of the product bidding category. This ID is equivalent to the google_product_category ID as described in this document: https://support.google.com/merchants/answer/6324436 |   |
| asset_group_listing_group_filter_case_value_product_bidding_category_level (obsolete - use asset_group_listing_group_filter_case_value_product_category_level instead) | Indicates the level of the category in the taxonomy. |   |
| asset_group_listing_group_filter_case_value_product_brand_value | String value of the product brand. |   |
| asset_group_listing_group_filter_case_value_product_channel_channel | Value of the locality. |   |
| asset_group_listing_group_filter_case_value_product_condition_condition | Value of the condition. |   |
| asset_group_listing_group_filter_case_value_product_custom_attribute_index | Indicates the index of the custom attribute. |   |
| asset_group_listing_group_filter_case_value_product_custom_attribute_value | String value of the product custom attribute. |   |
| asset_group_listing_group_filter_case_value_product_item_id_value | Value of the id. |   |
| asset_group_listing_group_filter_case_value_product_type_level | Level of the type. |   |
| asset_group_listing_group_filter_case_value_product_type_value | Value of the type. |   |
| asset_group_listing_group_filter_id | The ID of the ListingGroupFilter. |   |
| asset_group_listing_group_filter_parent_listing_group_filter | Resource name of the parent listing group subdivision. Null for the root listing group filter node. |   |
| asset_group_listing_group_filter_type | Type of a listing group filter node. |   |
| asset_group_listing_group_filter_vertical (obsolete - use asset_group_listing_group_filter_listing_source instead) | The vertical the current node tree represents. All nodes in the same tree must belong to the same vertical. |   |

Google Ads Table Name: AssetGroupProductGroupStats


Google Ads API Resource:
[asset_group_product_group_view](https://developers.google.com/google-ads/api/fields/v22/asset_group_product_group_view)

| Google Ads Field Name | Description | Adwords Mapped Field Name |
|---|---|---|
| asset_group_product_group_view_asset_group | The asset group associated with the listing group filter. |   |
| asset_group_product_group_view_asset_group_listing_group_filter | The resource name of the asset group listing group filter. |   |
| asset_group_product_group_view_resource_name | The resource name of the asset group product group view. Asset group product group view resource names have the form: customers/{customer_id}/assetGroupProductGroupViews/{asset_group_id}\~{listing_group_filter_id} |   |
| metrics_average_cpc | The total cost of all clicks divided by the total number of clicks received. |   |
| metrics_average_cpm | Average cost-per-thousand impressions (CPM). |   |
| metrics_clicks | The number of clicks. |   |
| metrics_conversions | The number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. If you use conversion-based bidding, your bid strategies optimize for these conversions. |   |
| metrics_conversions_from_interactions_rate | Conversions from interactions divided by the number of ad interactions (such as clicks for text ads or views for video ads). This only includes conversion actions which include_in_conversions_metric attribute is set to true. If you use conversion-based bidding, your bid strategies optimize for these conversions. |   |
| metrics_conversions_value | The value of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. If you use conversion-based bidding, your bid strategies optimize for these conversions. |   |
| metrics_cost_micros | The sum of your cost-per-click (CPC) and cost-per-thousand impressions (CPM) costs during this period. |   |
| metrics_cost_per_conversion | The cost of ad interactions divided by conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. If you use conversion-based bidding, your bid strategies optimize for these conversions. |   |
| metrics_ctr | The number of clicks your ad receives (Clicks) divided by the number of times your ad is shown (Impressions). |   |
| metrics_impressions | Count of how often your ad has appeared on a search results page or website on the Google Network. |   |
| segments_ad_network_type | Ad network type. |   |
| segments_date | Date to which metrics apply. yyyy-MM-dd format. For example, 2018-04-17. |   |
| segments_day_of_week | Day of the week. For example, MONDAY. |   |
| segments_device | Device to which metrics apply. |   |
| segments_month | Month as represented by the date of the first day of a month. Formatted as yyyy-MM-dd. |   |
| segments_quarter | Quarter as represented by the date of the first day of a quarter. Uses the calendar year for quarters. For example, the second quarter of 2018 starts on 2018-04-01. Formatted as yyyy-MM-dd. |   |
| segments_week | Week as defined as Monday through Sunday, and represented by the date of Monday. Formatted as yyyy-MM-dd. |   |
| segments_year | Year, formatted as yyyy. |   |

Google Ads Table Name: AssetGroupSignal


Google Ads API Resource:
[asset_group_signal](https://developers.google.com/google-ads/api/fields/v22/asset_group_signal)

| Google Ads Field Name | Description | Adwords Mapped Field Name |
|---|---|---|
| asset_group_signal_asset_group | The asset group which this asset group signal belongs to. |   |
| asset_group_signal_audience_audience | The Audience resource name. |   |
| asset_group_signal_resource_name | The resource name of the asset group signal. Asset group signal resource name have the form: customers/{customer_id}/assetGroupSignals/{asset_group_id}\~{signal_id} |   |

Google Ads Table Name: Audience


Google Ads API Resource:
[audience](https://developers.google.com/google-ads/api/fields/v22/audience)

| Google Ads Field Name | Description | Adwords Mapped Field Name |
|---|---|---|
| audience_description | Description of this audience. |   |
| audience_dimensions | Positive dimensions specifying the audience composition. |   |
| audience_exclusion_dimension | Negative dimension specifying the audience composition. |   |
| audience_id | ID of the audience. |   |
| audience_name | Name of the audience. It should be unique across all audiences. It must have a minimum length of 1 and maximum length of 255. |   |
| audience_status | Status of this audience. Indicates whether the audience is enabled or removed. |   |

Google Ads Table Name: BidGoal


Google Ads API Resource:
[bidding_strategy](https://developers.google.com/google-ads/api/fields/v22/bidding_strategy)

| Google Ads Field Name | Description | Adwords Mapped Field Name |
|---|---|---|
| bidding_strategy_id | The ID of the bidding strategy. | BidStrategyID |
| bidding_strategy_name | The name of the bidding strategy. All bidding strategies within an account must be named distinctly. The length of this string should be between 1 and 255, inclusive, in UTF-8 bytes, (trimmed). | Name |
| bidding_strategy_status | The status of the bidding strategy. This field is read-only. | Status |
| bidding_strategy_target_cpa_cpc_bid_ceiling_micros | Maximum bid limit that can be set by the bid strategy. The limit applies to all keywords managed by the strategy. | TargetCpaMaxCpcBidCeiling |
| bidding_strategy_target_cpa_cpc_bid_floor_micros | Minimum bid limit that can be set by the bid strategy. The limit applies to all keywords managed by the strategy. | TargetCpaMaxCpcBidFloor |
| bidding_strategy_target_cpa_target_cpa_micros | Average CPA target. This target should be greater than or equal to minimum billable unit based on the currency for the account. | TargetCpa |
| bidding_strategy_target_roas_cpc_bid_ceiling_micros | Maximum bid limit that can be set by the bid strategy. The limit applies to all keywords managed by the strategy. | TargetRoasBidCeiling |
| bidding_strategy_target_roas_cpc_bid_floor_micros | Minimum bid limit that can be set by the bid strategy. The limit applies to all keywords managed by the strategy. | TargetRoasBidFloor |
| bidding_strategy_target_roas_target_roas | Required. The wanted revenue (based on conversion data) per unit of spend. Value must be between 0.01 and 1000.0, inclusive. | TargetRoas |
| bidding_strategy_target_spend_cpc_bid_ceiling_micros | Maximum bid limit that can be set by the bid strategy. The limit applies to all keywords managed by the strategy. | TargetSpendBidCeiling |
| bidding_strategy_target_spend_target_spend_micros | The spend target under which to maximize clicks. A TargetSpend bidder attempts to spend the smaller of this value or the natural throttling spend amount. If not specified, the budget is used as the spend target. | TargetSpendSpendTarget |
| bidding_strategy_type | The type of the bidding strategy. Create a bidding strategy by setting the bidding scheme. This field is read-only. | Type |
| customer_descriptive_name | Optional, non-unique descriptive name of the customer. | AccountDescriptiveName |
| customer_id | The ID of the customer. | ExternalCustomerId |

Google Ads Table Name: BidGoalConversionStats


Google Ads API Resource:
[bidding_strategy](https://developers.google.com/google-ads/api/fields/v22/bidding_strategy)

| Google Ads Field Name | Description | Adwords Mapped Field Name |
|---|---|---|
| bidding_strategy_campaign_count | The number of campaigns attached to this bidding strategy. This field is read-only. | CampaignCount |
| bidding_strategy_id | The ID of the bidding strategy. | BidStrategyID |
| bidding_strategy_non_removed_campaign_count | The number of non-removed campaigns attached to this bidding strategy. This field is read-only. | NonRemovedCampaignCount |
| customer_id | The ID of the customer. | ExternalCustomerId |
| metrics_all_conversions | The total number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | AllConversions |
| metrics_all_conversions_value | The total value of all conversions. | AllConversionValue |
| metrics_conversions | The number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | Conversions |
| metrics_conversions_value | The total value of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | ConversionValue |
| metrics_cross_device_conversions | Conversions from when a customer clicks on a Google Ads ad on one device, then converts on a different device or browser. Cross-device conversions are already included in all_conversions. | CrossDeviceConversions |
| metrics_value_per_all_conversions | The value of all conversions divided by the number of all conversions. | ValuePerAllConversion |
| metrics_view_through_conversions | The total number of view-through conversions. These happen when a customer sees an image or rich media ad, then later completes a conversion on your site without interacting with (for example, clicking on) another ad. | ViewThroughConversions |
| segments_conversion_action | Resource name of the conversion action. | ConversionTrackerId |
| segments_conversion_action_category | Conversion action category. | ConversionCategoryName |
| segments_conversion_action_name | Conversion action name. | ConversionTypeName |
| segments_date | Date to which metrics apply. yyyy-MM-dd format. For example, 2018-04-17. | Date |
| segments_day_of_week | Day of the week. For example, MONDAY. | DayOfWeek |
| segments_device | Device to which metrics apply. | Device |
| segments_month | Month as represented by the date of the first day of a month. Formatted as yyyy-MM-dd. | Month |
| segments_quarter | Quarter as represented by the date of the first day of a quarter. Uses the calendar year for quarters. For example, the second quarter of 2018 starts on 2018-04-01. Formatted as yyyy-MM-dd. | Quarter |
| segments_week | Week as defined as Monday through Sunday, and represented by the date of Monday. Formatted as yyyy-MM-dd. | Week |
| segments_year | Year, formatted as yyyy. | Year |

Google Ads Table Name: BidGoalStats


Google Ads API Resource:
[bidding_strategy](https://developers.google.com/google-ads/api/fields/v22/bidding_strategy)

| Google Ads Field Name | Description | Adwords Mapped Field Name |
|---|---|---|
| bidding_strategy_campaign_count | The number of campaigns attached to this bidding strategy. This field is read-only. | CampaignCount |
| bidding_strategy_id | The ID of the bidding strategy. | BidStrategyID |
| bidding_strategy_non_removed_campaign_count | The number of non-removed campaigns attached to this bidding strategy. This field is read-only. | NonRemovedCampaignCount |
| customer_id | The ID of the customer. | ExternalCustomerId |
| metrics_all_conversions_from_interactions_rate | All conversions from interactions (as oppose to view through conversions) divided by the number of ad interactions. |   |
| metrics_average_cpc | The total cost of all clicks divided by the total number of clicks received. | AverageCpc |
| metrics_average_cpm | Average cost-per-thousand impressions (CPM). | AverageCpm |
| metrics_clicks | The number of clicks. | Clicks |
| metrics_conversions | The number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | Conversions |
| metrics_conversions_from_interactions_rate | Conversions from interactions divided by the number of ad interactions (such as clicks for text ads or views for video ads). This only includes conversion actions which include_in_conversions_metric attribute is set to true. | ConversionRate |
| metrics_conversions_value | The total value of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | ConversionValue |
| metrics_cost_micros | The sum of your cost-per-click (CPC) and cost-per-thousand impressions (CPM) costs during this period. | Cost |
| metrics_cost_per_all_conversions | The cost of ad interactions divided by all conversions. |   |
| metrics_cost_per_conversion | The cost of ad interactions divided by conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | CostPerConversion |
| metrics_ctr | The number of clicks your ad receives (Clicks) divided by the number of times your ad is shown (Impressions). | Ctr |
| metrics_impressions | Count of how often your ad has appeared on a search results page or website on the Google Network. | Impressions |
| metrics_value_per_conversion | The value of conversions divided by the number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | ValuePerConversion |
| segments_date | Date to which metrics apply. yyyy-MM-dd format. For example, 2018-04-17. | Date |
| segments_day_of_week | Day of the week. For example, MONDAY. | DayOfWeek |
| segments_device | Device to which metrics apply. | Device |
| segments_month | Month as represented by the date of the first day of a month. Formatted as yyyy-MM-dd. | Month |
| segments_quarter | Quarter as represented by the date of the first day of a quarter. Uses the calendar year for quarters. For example, the second quarter of 2018 starts on 2018-04-01. Formatted as yyyy-MM-dd. | Quarter |
| segments_week | Week as defined as Monday through Sunday, and represented by the date of Monday. Formatted as yyyy-MM-dd. | Week |
| segments_year | Year, formatted as yyyy. | Year |

Google Ads Table Name: Budget


Google Ads API Resource:
[campaign_budget](https://developers.google.com/google-ads/api/fields/v22/campaign_budget)

| Google Ads Field Name | Description | Adwords Mapped Field Name |
|---|---|---|
| campaign_budget_amount_micros | The amount of the budget, in the local currency for the account. Amount is specified in micros, where one million is equivalent to one currency unit. Monthly spend is capped at 30.4 times this amount. | Amount |
| campaign_budget_delivery_method | The delivery method that determines the rate at which the campaign budget is spent. Defaults to STANDARD if unspecified in a create operation. | DeliveryMethod |
| campaign_budget_explicitly_shared | Specifies whether the budget is explicitly shared. Defaults to true if unspecified in a create operation. If true, the budget was created with the purpose of sharing across one or more campaigns. If false, the budget was created with the intention of only being used with a single campaign. The budget's name and status stays in sync with the campaign's name and status. Attempting to share the budget with a second campaign results in an error. A non-shared budget can become an explicitly shared. The same operation must also assign the budget a name. A shared campaign budget can never become non-shared. | IsBudgetExplicitlyShared |
| campaign_budget_has_recommended_budget | Indicates whether there is a recommended budget for this campaign budget. This field is read-only. | HasRecommendedBudget |
| campaign_budget_id | The ID of the campaign budget. A campaign budget is created using the CampaignBudgetService create operation and is assigned a budget ID. A budget ID can be shared across different campaigns; the system allocates the campaign budget among different campaigns to get optimum results. | BudgetId |
| campaign_budget_name | The name of the campaign budget. When creating a campaign budget through CampaignBudgetService, every explicitly shared campaign budget must have a non-null, non-empty name. Campaign budgets that are not explicitly shared derive their name from the attached campaign's name. The length of this string must be between 1 and 255, inclusive, in UTF-8 bytes, (trimmed). | BudgetName |
| campaign_budget_period | Period over which to spend the budget. Defaults to DAILY if not specified. | Period |
| campaign_budget_recommended_budget_amount_micros | The recommended budget amount. If no recommendation is available, this is set to the budget amount. Amount is specified in micros, where one million is equivalent to one currency unit. This field is read-only. | RecommendedBudgetAmount |
| campaign_budget_reference_count | The number of campaigns actively using the budget. This field is read-only. | BudgetReferenceCount |
| campaign_budget_status | The status of this campaign budget. This field is read-only. | BudgetStatus |
| campaign_budget_total_amount_micros | The lifetime amount of the budget, in the local currency for the account. Amount is specified in micros, where one million is equivalent to one currency unit. | TotalAmount |
| customer_descriptive_name | Optional, non-unique descriptive name of the customer. | AccountDescriptiveName |
| customer_id | The ID of the customer. | ExternalCustomerId |

Google Ads Table Name: BudgetStats


Google Ads API Resource:
[campaign_budget](https://developers.google.com/google-ads/api/fields/v22/campaign_budget)

| Google Ads Field Name | Description | Adwords Mapped Field Name |
|---|---|---|
| campaign_budget_id | The ID of the campaign budget. A campaign budget is created using the CampaignBudgetService create operation and is assigned a budget ID. A budget ID can be shared across different campaigns; the system then allocates the campaign budget among different campaigns to get optimum results. | BudgetId |
| campaign_budget_recommended_budget_estimated_change_weekly_clicks | The estimated change in weekly clicks if the recommended budget is applied. This field is read-only. | RecommendedBudgetEstimatedChangeInWeeklyClicks |
| campaign_budget_recommended_budget_estimated_change_weekly_cost_micros | The estimated change in weekly cost in micros if the recommended budget is applied. One million is equivalent to one currency unit. This field is read-only. | RecommendedBudgetEstimatedChangeInWeeklyCost |
| campaign_budget_recommended_budget_estimated_change_weekly_interactions | The estimated change in weekly interactions if the recommended budget is applied. This field is read-only. | RecommendedBudgetEstimatedChangeInWeeklyInteractions |
| campaign_budget_recommended_budget_estimated_change_weekly_views | The estimated change in weekly views if the recommended budget is applied. This field is read-only. | RecommendedBudgetEstimatedChangeInWeeklyViews |
| campaign_id | The ID of the campaign. | AssociatedCampaignId |
| campaign_name | The name of the campaign. This field is required and shouldn't be empty when creating new campaigns. It must not contain any null (code point 0x0), NL line feed (code point 0xA) or carriage return (code point 0xD) characters. | AssociatedCampaignName |
| campaign_status | The status of the campaign. When a new campaign is added, the status defaults to ENABLED. | AssociatedCampaignStatus |
| customer_id | The ID of the customer. | ExternalCustomerId |
| metrics_all_conversions | The total number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | AllConversions |
| metrics_all_conversions_from_interactions_rate | All conversions from interactions (as oppose to view through conversions) divided by the number of ad interactions. | AllConversionRate |
| metrics_all_conversions_value | The total value of all conversions. | AllConversionValue |
| metrics_average_cost | The average amount you pay per interaction. This amount is the total cost of your ads divided by the total number of interactions. | AverageCost |
| metrics_average_cpc | The total cost of all clicks divided by the total number of clicks received. | AverageCpc |
| metrics_average_cpe | The average amount that you've been charged for an ad engagement. This amount is the total cost of all ad engagements divided by the total number of ad engagements. | AverageCpe |
| metrics_average_cpm | Average cost-per-thousand impressions (CPM). | AverageCpm |
| metrics_trueview_average_cpv | The average amount you pay each time someone views your ad. The average CPV is defined by the total cost of all ad views divided by the number of views. | AverageCpv |
| metrics_clicks | The number of clicks. | Clicks |
| metrics_conversions | The number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | Conversions |
| metrics_conversions_from_interactions_rate | Conversions from interactions divided by the number of ad interactions (such as clicks for text ads or views for video ads). This only includes conversion actions which include_in_conversions_metric attribute is set to true. | ConversionRate |
| metrics_conversions_value | The total value of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | ConversionValue |
| metrics_cost_micros | The sum of your cost-per-click (CPC) and cost-per-thousand impressions (CPM) costs during this period. | Cost |
| metrics_cost_per_all_conversions | The cost of ad interactions divided by all conversions. | CostPerAllConversion |
| metrics_cost_per_conversion | The cost of ad interactions divided by conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | CostPerConversion |
| metrics_cross_device_conversions | Conversions from when a customer clicks on a Google Ads ad on one device, then converts on a different device or browser. Cross-device conversions are already included in all_conversions. | CrossDeviceConversions |
| metrics_ctr | The number of clicks your ad receives (Clicks) divided by the number of times your ad is shown (Impressions). | Ctr |
| metrics_engagement_rate | How often people engage with your ad after it's shown to them. This is the number of ad expansions divided by the number of times your ad is shown. | EngagementRate |
| metrics_engagements | The number of engagements. An engagement occurs when a viewer expands your Lightbox ad. Also, in the future, other ad types may support engagement metrics. | Engagements |
| metrics_impressions | Count of how often your ad has appeared on a search results page or website on the Google Network. | Impressions |
| metrics_interaction_event_types | The types of payable and free interactions. | InteractionTypes |
| metrics_interaction_rate | How often people interact with your ad after it is shown to them. This is the number of interactions divided by the number of times your ad is shown. | InteractionRate |
| metrics_interactions | The number of interactions. An interaction is the main user action associated with an ad format, such as clicks for text and shopping ads or views for video ads. | Interactions |
| metrics_value_per_all_conversions | The value of all conversions divided by the number of all conversions. | ValuePerAllConversion |
| metrics_value_per_conversion | The value of conversions divided by the number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | ValuePerConversion |
| metrics_video_trueview_view_rate | The number of views your TrueView video ad receives divided by its number of impressions, including thumbnail impressions for TrueView in-display ads. | VideoViewRate |
| metrics_video_trueview_views | The number of times your video ads were viewed. | VideoViews |
| metrics_view_through_conversions | The total number of view-through conversions. These happen when a customer sees an image or rich media ad, then later completes a conversion on your site without interacting with (for example, clicking on) another ad. | ViewThroughConversions |
| segments_date | Date to which metrics apply. yyyy-MM-dd format. For example, 2018-04-17. |   |

Google Ads Table Name: Campaign


Google Ads API Resource:
[campaign](https://developers.google.com/google-ads/api/fields/v22/campaign)

| Google Ads Field Name | Description | Adwords Mapped Field Name |
|---|---|---|
| bidding_strategy_name | The name of the bidding strategy. All bidding strategies within an account must be named distinctly. The length of this string should be between 1 and 255, inclusive, in UTF-8 bytes, (trimmed). | BiddingStrategyName |
| campaign_advertising_channel_sub_type | Optional refinement to \`advertising_channel_type\`. Must be a valid sub-type of the parent channel type. Can be set only when creating campaigns. After campaign is created, the field cannot be changed. | AdvertisingChannelSubType |
| campaign_advertising_channel_type | The primary serving target for ads within the campaign. The targeting options can be refined in \`network_settings\`. This field is required and shouldn't be empty when creating new campaigns. Can be set only when creating campaigns. After the campaign is created, the field cannot be changed. | AdvertisingChannelType |
| campaign_bidding_strategy | Portfolio bidding strategy used by campaign. | BiddingStrategyId |
| campaign_bidding_strategy_type | The type of bidding strategy. A bidding strategy can be created by setting either the bidding scheme to create a standard bidding strategy or the \`bidding_strategy\` field to create a portfolio bidding strategy. This field is read-only. | BiddingStrategyType |
| campaign_budget_amount_micros | The amount of the budget, in the local currency for the account. Amount is specified in micros, where one million is equivalent to one currency unit. Monthly spend is capped at 30.4 times this amount. | Amount |
| campaign_budget_explicitly_shared | Specifies whether the budget is explicitly shared. Defaults to true if unspecified in a create operation. If true, the budget was created with the purpose of sharing across one or more campaigns. If false, the budget was created with the intention of only being used with a single campaign. The budget's name and status stays in sync with the campaign's name and status. Attempting to share the budget with a second campaign results in an error. A non-shared budget can become an explicitly shared. The same operation must also assign the budget a name. A shared campaign budget can never become non-shared. | IsBudgetExplicitlyShared |
| campaign_budget_has_recommended_budget | Indicates whether there is a recommended budget for this campaign budget. This field is read-only. | HasRecommendedBudget |
| campaign_budget_period | Period over which to spend the budget. Defaults to DAILY if not specified. | Period |
| campaign_budget_recommended_budget_amount_micros | The recommended budget amount. If no recommendation is available, this is set to the budget amount. Amount is specified in micros, where one million is equivalent to one currency unit. This field is read-only. | RecommendedBudgetAmount |
| campaign_budget_total_amount_micros | The lifetime amount of the budget, in the local currency for the account. Amount is specified in micros, where one million is equivalent to one currency unit. | TotalAmount |
| campaign_campaign_budget | The budget of the campaign. | BudgetId |
| campaign_end_date | The date when campaign ended. This field must not be used in WHERE clauses. | EndDate |
| campaign_experiment_type | The type of campaign: normal, draft, or experiment. | CampaignTrialType |
| campaign_id | The ID of the campaign. | CampaignId |
| campaign_manual_cpc_enhanced_cpc_enabled | Whether bids are to be enhanced based on conversion optimizer data. | EnhancedCpcEnabled |
| campaign_maximize_conversion_value_target_roas | The target return on ad spend (ROAS) option. If set, the bid strategy maximizes revenue while averaging the target return on ad spend. If the target ROAS is high, the bid strategy may not be able to spend the full budget. If the target ROAS is not set, the bid strategy aims to achieve the highest possible ROAS for the budget. | MaximizeConversionValueTargetRoas |
| campaign_name | The name of the campaign. This field is required and shouldn't be empty when creating new campaigns. It must not contain any null (code point 0x0), NL line feed (code point 0xA) or carriage return (code point 0xD) characters. | CampaignName |
| campaign_percent_cpc_enhanced_cpc_enabled | Adjusts the bid for each auction upward or downward, depending on the likelihood of a conversion. Individual bids may exceed cpc_bid_ceiling_micros, but the average bid amount for a campaign shouldn't. |   |
| campaign_serving_status | The ad serving status of the campaign. | ServingStatus |
| campaign_start_date | The date when campaign started. This field must not be used in WHERE clauses. | StartDate |
| campaign_status | The status of the campaign. When a new campaign is added, the status defaults to ENABLED. | CampaignStatus |
| campaign_tracking_url_template | The URL template for constructing a tracking URL. | TrackingUrlTemplate |
| campaign_url_custom_parameters | The list of mappings used to substitute custom parameter tags in a \`tracking_url_template\`, \`final_urls\`, or \`mobile_final_urls\`. | UrlCustomParameters |
| customer_id | The ID of the customer. | ExternalCustomerId |

Google Ads Table Name: CampaignAssetStats


Google Ads API Resource:
[campaign_asset](https://developers.google.com/google-ads/api/fields/v22/campaign_asset)

| Google Ads Field Name | Description | Adwords Mapped Field Name |
|---|---|---|
| campaign_asset_asset | The asset which is linked to the campaign. |   |
| campaign_asset_campaign | The campaign to which the asset is linked. |   |
| campaign_asset_field_type | Role that the asset takes under the linked campaign. |   |
| campaign_asset_resource_name | The resource name of the campaign asset. CampaignAsset resource names have the form: customers/{customer_id}/campaignAssets/{campaign_id}\~{asset_id}\~{field_type} |   |
| campaign_asset_source | Source of the campaign asset link. |   |
| campaign_asset_status | Status of the campaign asset. |   |
| metrics_average_cpc | The total cost of all clicks divided by the total number of clicks received. |   |
| metrics_average_cpe | The average amount that you've been charged for an ad engagement. This amount is the total cost of all ad engagements divided by the total number of ad engagements. |   |
| metrics_average_cpm | Average cost-per-thousand impressions (CPM). |   |
| metrics_trueview_average_cpv | The average amount you pay each time someone views your ad. The average CPV is defined by the total cost of all ad views divided by the number of views. |   |
| metrics_clicks | The number of clicks. |   |
| metrics_conversions | The number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. If you use conversion-based bidding, your bid strategies optimize for these conversions. |   |
| metrics_conversions_from_interactions_rate | Conversions from interactions divided by the number of ad interactions (such as clicks for text ads or views for video ads). This only includes conversion actions which include_in_conversions_metric attribute is set to true. If you use conversion-based bidding, your bid strategies optimize for these conversions. |   |
| metrics_conversions_value | The value of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. If you use conversion-based bidding, your bid strategies optimize for these conversions. |   |
| metrics_cost_micros | The sum of your cost-per-click (CPC) and cost-per-thousand impressions (CPM) costs during this period. |   |
| metrics_cost_per_conversion | The cost of ad interactions divided by conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. If you use conversion-based bidding, your bid strategies optimize for these conversions. |   |
| metrics_ctr | The number of clicks your ad receives (Clicks) divided by the number of times your ad is shown (Impressions). |   |
| metrics_impressions | Count of how often your ad has appeared on a search results page or website on the Google Network. |   |
| metrics_interactions | The number of interactions. An interaction is the main user action associated with an ad format, such as clicks for text and shopping ads or views for video ads. |   |
| metrics_top_impression_percentage | The percent of your ad impressions that are shown as the very first ad above the organic search results. |   |
| segments_ad_network_type | Ad network type. |   |
| segments_date | Date to which metrics apply. yyyy-MM-dd format. For example, 2018-04-17. |   |
| segments_day_of_week | Day of the week. For example, MONDAY. |   |
| segments_device | Device to which metrics apply. |   |
| segments_month | Month as represented by the date of the first day of a month. Formatted as yyyy-MM-dd. |   |
| segments_quarter | Quarter as represented by the date of the first day of a quarter. Uses the calendar year for quarters. For example, the second quarter of 2018 starts on 2018-04-01. Formatted as yyyy-MM-dd. |   |
| segments_week | Week as defined as Monday through Sunday, and represented by the date of Monday. Formatted as yyyy-MM-dd. |   |
| segments_year | Year, formatted as yyyy. |   |

Google Ads Table Name: CampaignAudience


Google Ads API Resource:
[campaign_audience_view](https://developers.google.com/google-ads/api/fields/v22/campaign_audience_view)

| Google Ads Field Name | Description | Adwords Mapped Field Name |
|---|---|---|
| campaign_base_campaign | The resource name of the base campaign of a draft or experiment campaign. For base campaigns, this is equal to \`resource_name\`. This field is read-only. |   |
| campaign_criterion_bid_modifier | Portfolio bidding strategy used by campaign. |   |
| campaign_criterion_criterion_id | Output only. The ID of the criterion. This field is ignored during mutate. |   |
| campaign_id | The ID of the campaign. | CampaignId |
| customer_id | The ID of the customer. |   |

Google Ads Table Name: CampaignAudienceBasicStats


Google Ads API Resource:
[campaign_audience_view](https://developers.google.com/google-ads/api/fields/v22/campaign_audience_view)

| Google Ads Field Name | Description | Adwords Mapped Field Name |
|---|---|---|
| campaign_base_campaign | The resource name of the base campaign of a draft or experiment campaign. For base campaigns, this is equal to \`resource_name\`. This field is read-only. |   |
| campaign_criterion_criterion_id | Output only. The ID of the criterion. This field is ignored during mutate. |   |
| customer_id | The ID of the customer. |   |
| metrics_clicks | The number of clicks. |   |
| metrics_conversions | The number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. |   |
| metrics_conversions_value | The total value of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. |   |
| metrics_cost_micros | The sum of your cost-per-click (CPC) and cost-per-thousand impressions (CPM) costs during this period. |   |
| metrics_impressions | Count of how often your ad has appeared on a search results page or website on the Google Network. |   |
| metrics_interaction_event_types | The types of payable and free interactions. |   |
| metrics_interactions | The number of interactions. An interaction is the main user action associated with an ad format, such as clicks for text and shopping ads or views for video ads. |   |
| metrics_view_through_conversions | The total number of view-through conversions. These happen when a customer sees an image or rich media ad, then later completes a conversion on your site without interacting with (for example, clicking on) another ad. |   |
| segments_ad_network_type | Ad network type. |   |
| segments_date | Date to which metrics apply. yyyy-MM-dd format. For example, 2018-04-17. |   |
| segments_device | Device to which metrics apply. |   |
| segments_slot | Position of the ad. |   |

Google Ads Table Name: CampaignAudienceConversionStats


Google Ads API Resource:
[campaign_audience_view](https://developers.google.com/google-ads/api/fields/v22/campaign_audience_view)

| Google Ads Field Name | Description | Adwords Mapped Field Name |
|---|---|---|
| campaign_base_campaign | The resource name of the base campaign of a draft or experiment campaign. For base campaigns, this is equal to \`resource_name\`. This field is read-only. |   |
| campaign_criterion_criterion_id | Output only. The ID of the criterion. This field is ignored during mutate. |   |
| customer_id | The ID of the customer. |   |
| metrics_all_conversions | The total number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. |   |
| metrics_all_conversions_value | The total value of all conversions. |   |
| metrics_conversions | The number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. |   |
| metrics_conversions_value | The total value of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. |   |
| metrics_cross_device_conversions | Conversions from when a customer clicks on a Google Ads ad on one device, then converts on a different device or browser. Cross-device conversions are already included in all_conversions. |   |
| metrics_value_per_all_conversions | The value of all conversions divided by the number of all conversions. |   |
| metrics_value_per_conversion | The value of conversions divided by the number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. |   |
| metrics_view_through_conversions | The total number of view-through conversions. These happen when a customer sees an image or rich media ad, then later completes a conversion on your site without interacting with (for example, clicking on) another ad. |   |
| segments_ad_network_type | Ad network type. |   |
| segments_conversion_action | Resource name of the conversion action. |   |
| segments_conversion_action_category | Conversion action category. |   |
| segments_conversion_action_name | Conversion action name. |   |
| segments_date | Date to which metrics apply. yyyy-MM-dd format. For example, 2018-04-17. |   |
| segments_day_of_week | Day of the week. For example, MONDAY. |   |
| segments_device | Device to which metrics apply. |   |
| segments_month | Month as represented by the date of the first day of a month. Formatted as yyyy-MM-dd. |   |
| segments_quarter | Quarter as represented by the date of the first day of a quarter. Uses the calendar year for quarters. For example, the second quarter of 2018 starts on 2018-04-01. Formatted as yyyy-MM-dd. |   |
| segments_week | Week as defined as Monday through Sunday, and represented by the date of Monday. Formatted as yyyy-MM-dd. |   |
| segments_year | Year, formatted as yyyy. |   |

Google Ads Table Name: CampaignAudienceNonClickStats


Google Ads API Resource:
[campaign_audience_view](https://developers.google.com/google-ads/api/fields/v22/campaign_audience_view)

| Google Ads Field Name | Description | Adwords Mapped Field Name |
|---|---|---|
| campaign_base_campaign | The resource name of the base campaign of a draft or experiment campaign. For base campaigns, this is equal to \`resource_name\`. This field is read-only. |   |
| customer_id | The ID of the customer. |   |
| metrics_average_cpe | The average amount that you've been charged for an ad engagement. This amount is the total cost of all ad engagements divided by the total number of ad engagements. |   |
| metrics_trueview_average_cpv | The average amount you pay each time someone views your ad. The average CPV is defined by the total cost of all ad views divided by the number of views. |   |
| metrics_engagement_rate | How often people engage with your ad after it's shown to them. This is the number of ad expansions divided by the number of times your ad is shown. |   |
| metrics_engagements | The number of engagements. An engagement occurs when a viewer expands your Lightbox ad. Also, in the future, other ad types may support engagement metrics. |   |
| metrics_video_trueview_view_rate | The number of views your TrueView video ad receives divided by its number of impressions, including thumbnail impressions for TrueView in-display ads. |   |
| metrics_video_trueview_views | The number of times your video ads were viewed. |   |
| segments_ad_network_type | Ad network type. |   |
| segments_date | Date to which metrics apply. yyyy-MM-dd format. For example, 2018-04-17. |   |
| segments_day_of_week | Day of the week. For example, MONDAY. |   |
| segments_device | Device to which metrics apply. |   |
| segments_month | Month as represented by the date of the first day of a month. Formatted as yyyy-MM-dd. |   |
| segments_quarter | Quarter as represented by the date of the first day of a quarter. Uses the calendar year for quarters. For example, the second quarter of 2018 starts on 2018-04-01. Formatted as yyyy-MM-dd. |   |
| segments_week | Week as defined as Monday through Sunday, and represented by the date of Monday. Formatted as yyyy-MM-dd. |   |
| segments_year | Year, formatted as yyyy. |   |

Google Ads Table Name: CampaignAudienceStats


Google Ads API Resource:
[campaign_audience_view](https://developers.google.com/google-ads/api/fields/v22/campaign_audience_view)

| Google Ads Field Name | Description | Adwords Mapped Field Name |
|---|---|---|
| campaign_base_campaign | The resource name of the base campaign of a draft or experiment campaign. For base campaigns, this is equal to \`resource_name\`. This field is read-only. |   |
| campaign_criterion_criterion_id | Output only. The ID of the criterion. This field is ignored during mutate. |   |
| customer_id | The ID of the customer. |   |
| metrics_active_view_cpm | Average cost of viewable impressions (\`active_view_impressions\`). |   |
| metrics_active_view_ctr | Active view measurable clicks divided by active view viewable impressions. This metric is reported only for display network. |   |
| metrics_active_view_impressions | A measurement of how often your ad has become viewable on a Display Network site. |   |
| metrics_active_view_measurability | The ratio of impressions that could be measured by Active View over the number of served impressions. |   |
| metrics_active_view_measurable_cost_micros | The cost of the impressions you received that were measurable by Active View. |   |
| metrics_active_view_measurable_impressions | The number of times your ads are appearing on placements in positions where they can be seen. |   |
| metrics_active_view_viewability | The percentage of time when your ad appeared on an Active View enabled site (measurable impressions) and was viewable (viewable impressions). |   |
| metrics_all_conversions_from_interactions_rate | All conversions from interactions (as oppose to view through conversions) divided by the number of ad interactions. |   |
| metrics_average_cost | The average amount you pay per interaction. This amount is the total cost of your ads divided by the total number of interactions. |   |
| metrics_average_cpc | The total cost of all clicks divided by the total number of clicks received. |   |
| metrics_average_cpm | Average cost-per-thousand impressions (CPM). |   |
| metrics_clicks | The number of clicks. |   |
| metrics_conversions | The number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. |   |
| metrics_conversions_from_interactions_rate | Conversions from interactions divided by the number of ad interactions (such as clicks for text ads or views for video ads). This only includes conversion actions which include_in_conversions_metric attribute is set to true. |   |
| metrics_conversions_value | The total value of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. |   |
| metrics_cost_micros | The sum of your cost-per-click (CPC) and cost-per-thousand impressions (CPM) costs during this period. |   |
| metrics_cost_per_all_conversions | The cost of ad interactions divided by all conversions. |   |
| metrics_cost_per_conversion | The cost of ad interactions divided by conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. |   |
| metrics_ctr | The number of clicks your ad receives (Clicks) divided by the number of times your ad is shown (Impressions). |   |
| metrics_gmail_forwards | The number of times the ad was forwarded to someone else as a message. |   |
| metrics_gmail_saves | The number of times someone has saved your Gmail ad to their inbox as a message. |   |
| metrics_gmail_secondary_clicks | The number of clicks to the landing page on the expanded state of Gmail ads. |   |
| metrics_impressions | Count of how often your ad has appeared on a search results page or website on the Google Network. |   |
| metrics_interaction_event_types | The types of payable and free interactions. |   |
| metrics_interaction_rate | How often people interact with your ad after it is shown to them. This is the number of interactions divided by the number of times your ad is shown. |   |
| metrics_interactions | The number of interactions. An interaction is the main user action associated with an ad format, such as clicks for text and shopping ads or views for video ads. |   |
| metrics_value_per_conversion | The value of conversions divided by the number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. |   |
| segments_ad_network_type | Ad network type. |   |
| segments_click_type | Click type. |   |
| segments_date | Date to which metrics apply. yyyy-MM-dd format. For example, 2018-04-17. |   |
| segments_day_of_week | Day of the week. For example, MONDAY. |   |
| segments_device | Device to which metrics apply. |   |
| segments_month | Month as represented by the date of the first day of a month. Formatted as yyyy-MM-dd. |   |
| segments_quarter | Quarter as represented by the date of the first day of a quarter. Uses the calendar year for quarters. For example, the second quarter of 2018 starts on 2018-04-01. Formatted as yyyy-MM-dd. |   |
| segments_week | Week as defined as Monday through Sunday, and represented by the date of Monday. Formatted as yyyy-MM-dd. |   |
| segments_year | Year, formatted as yyyy. |   |

Google Ads Table Name: CampaignBasicStats


Google Ads API Resource:
[campaign](https://developers.google.com/google-ads/api/fields/v22/campaign)

| Google Ads Field Name | Description | Adwords Mapped Field Name |
|---|---|---|
| campaign_base_campaign | The resource name of the base campaign of a draft or experiment campaign. For base campaigns, this is equal to \`resource_name\`. This field is read-only. | BaseCampaignId |
| campaign_id | The ID of the campaign. | CampaignId |
| customer_id | The ID of the customer. | ExternalCustomerId |
| metrics_clicks | The number of clicks. | Clicks |
| metrics_conversions | The number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | Conversions |
| metrics_conversions_value | The total value of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | ConversionValue |
| metrics_cost_micros | The sum of your cost-per-click (CPC) and cost-per-thousand impressions (CPM) costs during this period. | Cost |
| metrics_impressions | Count of how often your ad has appeared on a search results page or website on the Google Network. | Impressions |
| metrics_interaction_event_types | The types of payable and free interactions. | InteractionTypes |
| metrics_interactions | The number of interactions. An interaction is the main user action associated with an ad format, such as clicks for text and shopping ads or views for video ads. | Interactions |
| metrics_view_through_conversions | The total number of view-through conversions. These happen when a customer sees an image or rich media ad, then later completes a conversion on your site without interacting with (for example, clicking on) another ad. | ViewThroughConversions |
| segments_ad_network_type | Ad network type. | AdNetworkType2 |
| segments_date | Date to which metrics apply. yyyy-MM-dd format. For example, 2018-04-17. | Date |
| segments_device | Device to which metrics apply. | Device |
| segments_slot | Position of the ad. | Slot |

Google Ads Table Name: CampaignConversionStats


Google Ads API Resource:
[campaign](https://developers.google.com/google-ads/api/fields/v22/campaign)

| Google Ads Field Name | Description | Adwords Mapped Field Name |
|---|---|---|
| campaign_base_campaign | The resource name of the base campaign of a draft or experiment campaign. For base campaigns, this is equal to \`resource_name\`. This field is read-only. | BaseCampaignId |
| campaign_id | The ID of the campaign. | CampaignId |
| customer_id | The ID of the customer. | ExternalCustomerId |
| metrics_conversions | The number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | Conversions |
| metrics_conversions_value | The total value of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | ConversionValue |
| metrics_value_per_conversion | The value of conversions divided by the number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | ValuePerConversion |
| segments_ad_network_type | Ad network type. | AdNetworkType2 |
| segments_conversion_action | Resource name of the conversion action. | ConversionTrackerId |
| segments_conversion_action_category | Conversion action category. | ConversionCategoryName |
| segments_conversion_action_name | Conversion action name. | ConversionTypeName |
| segments_conversion_attribution_event_type | Conversion attribution event type. | ConversionAttributionEventType |
| segments_date | Date to which metrics apply. yyyy-MM-dd format. For example, 2018-04-17. | Date |
| segments_day_of_week | Day of the week. For example, MONDAY. | DayOfWeek |
| segments_month | Month as represented by the date of the first day of a month. Formatted as yyyy-MM-dd. | Month |
| segments_quarter | Quarter as represented by the date of the first day of a quarter. Uses the calendar year for quarters. For example, the second quarter of 2018 starts on 2018-04-01. Formatted as yyyy-MM-dd. | Quarter |
| segments_slot | Position of the ad. | Slot |
| segments_week | Week as defined as Monday through Sunday, and represented by the date of Monday. Formatted as yyyy-MM-dd. | Week |
| segments_year | Year, formatted as yyyy. | Year |

Google Ads Table Name: CampaignCookieStats


Google Ads API Resource:
[campaign](https://developers.google.com/google-ads/api/fields/v22/campaign)

| Google Ads Field Name | Description | Adwords Mapped Field Name |
|---|---|---|
| campaign_base_campaign | The resource name of the base campaign of a draft or experiment campaign. For base campaigns, this is equal to \`resource_name\`. This field is read-only. | BaseCampaignId |
| campaign_id | The ID of the campaign. | CampaignId |
| customer_id | The ID of the customer. | ExternalCustomerId |
| metrics_absolute_top_impression_percentage | The percent of your ad impressions that are shown as the very first ad above the organic search results. | AbsoluteTopImpressionPercentage |
| metrics_search_budget_lost_absolute_top_impression_share | The number estimating how often your ad wasn't the very first ad above the organic search results due to a low budget. Note: Search budget lost absolute top impression share is reported in the range of 0 to 0.9. Any value above 0.9 is reported as 0.9001. | SearchBudgetLostAbsoluteTopImpressionShare |
| metrics_search_budget_lost_top_impression_share | The number estimating how often your ad didn't show anywhere above the organic search results due to a low budget. Note: Search budget lost top impression share is reported in the range of 0 to 0.9. Any value above 0.9 is reported as 0.9001. | SearchBudgetLostTopImpressionShare |
| metrics_search_rank_lost_absolute_top_impression_share | The number estimating how often your ad wasn't the very first ad above the organic search results due to poor Ad Rank. Note: Search rank lost absolute top impression share is reported in the range of 0 to 0.9. Any value above 0.9 is reported as 0.9001. | SearchRankLostAbsoluteTopImpressionShare |
| metrics_search_rank_lost_top_impression_share | The number estimating how often your ad didn't show anywhere above the organic search results due to poor Ad Rank. Note: Search rank lost top impression share is reported in the range of 0 to 0.9. Any value above 0.9 is reported as 0.9001. | SearchRankLostTopImpressionShare |
| metrics_search_top_impression_share | The impressions you've received in the top location (anywhere above the organic search results) compared to the estimated number of impressions you were eligible to receive in the top location. Note: Search top impression share is reported in the range of 0.1 to 1. Any value below 0.1 is reported as 0.0999. | SearchTopImpressionShare |
| metrics_top_impression_percentage | The percent of your ad impressions that are shown anywhere above the organic search results. | TopImpressionPercentage |
| segments_ad_network_type | Ad network type. | AdNetworkType2 |
| segments_date | Date to which metrics apply. yyyy-MM-dd format. For example, 2018-04-17. | Date |
| segments_month | Month as represented by the date of the first day of a month. Formatted as yyyy-MM-dd. | Month |
| segments_week | Week as defined as Monday through Sunday, and represented by the date of Monday. Formatted as yyyy-MM-dd. | Week |

Google Ads Table Name: CampaignCriterion


Google Ads API Resource:
[campaign_criterion](https://developers.google.com/google-ads/api/fields/v22/campaign_criterion)

| Google Ads Field Name | Description | Adwords Mapped Field Name |
|---|---|---|
| campaign_criterion_bid_modifier | The modifier for the bids when the criterion matches. The modifier must be in the range: 0.1 - 10.0. Most targetable criteria types support modifiers. Use 0 to opt out of a Device type. |   |
| campaign_criterion_campaign | The campaign to which the criterion belongs. |   |
| campaign_criterion_criterion_id | The ID of the criterion. This field is ignored during mutate. |   |
| campaign_criterion_device_type | Type of the device. |   |
| campaign_criterion_display_name | The display name of the criterion. This field is ignored for mutates. |   |
| campaign_criterion_negative | Whether to target (\`false\`) or exclude (\`true\`) the criterion. |   |
| campaign_criterion_status | The status of the criterion. |   |
| campaign_criterion_type | The type of the criterion. |   |
| campaign_id | The ID of the campaign. |   |
| campaign_name | The name of the campaign. This field is required and shouldn't be empty when creating new campaigns. It must not contain any null (code point 0x0), NL line feed (code point 0xA) or carriage return (code point 0xD) characters. |   |

Google Ads Table Name: CampaignCrossDeviceConversionStats


Google Ads API Resource:
[campaign](https://developers.google.com/google-ads/api/fields/v22/campaign)

| Google Ads Field Name | Description | Adwords Mapped Field Name |
|---|---|---|
| campaign_base_campaign | The resource name of the base campaign of a draft or experiment campaign. For base campaigns, this is equal to \`resource_name\`. This field is read-only. | BaseCampaignId |
| campaign_id | The ID of the campaign. | CampaignId |
| customer_id | The ID of the customer. | ExternalCustomerId |
| metrics_all_conversions | The total number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | AllConversions |
| metrics_all_conversions_value | The total value of all conversions. | AllConversionValue |
| metrics_cross_device_conversions | Conversions from when a customer clicks on a Google Ads ad on one device, then converts on a different device or browser. Cross-device conversions are already included in all_conversions. | CrossDeviceConversions |
| metrics_value_per_all_conversions | The value of all conversions divided by the number of all conversions. | ValuePerAllConversion |
| segments_ad_network_type | Ad network type. | AdNetworkType2 |
| segments_conversion_action | Resource name of the conversion action. | ConversionTrackerId |
| segments_conversion_action_category | Conversion action category. | ConversionCategoryName |
| segments_conversion_action_name | Conversion action name. | ConversionTypeName |
| segments_conversion_attribution_event_type | Conversion attribution event type. | ConversionAttributionEventType |
| segments_date | Date to which metrics apply. yyyy-MM-dd format. For example, 2018-04-17. | Date |
| segments_day_of_week | Day of the week. For example, MONDAY. | DayOfWeek |
| segments_month | Month as represented by the date of the first day of a month. Formatted as yyyy-MM-dd. | Month |
| segments_quarter | Quarter as represented by the date of the first day of a quarter. Uses the calendar year for quarters. For example, the second quarter of 2018 starts on 2018-04-01. Formatted as yyyy-MM-dd. | Quarter |
| segments_week | Week as defined as Monday through Sunday, and represented by the date of Monday. Formatted as yyyy-MM-dd. | Week |
| segments_year | Year, formatted as yyyy. | Year |

Google Ads Table Name: CampaignCrossDeviceStats


Google Ads API Resource:
[campaign](https://developers.google.com/google-ads/api/fields/v22/campaign)

| Google Ads Field Name | Description | Adwords Mapped Field Name |
|---|---|---|
| campaign_base_campaign | The resource name of the base campaign of a draft or experiment campaign. For base campaigns, this is equal to \`resource_name\`. This field is read-only. | BaseCampaignId |
| campaign_id | The ID of the campaign. | CampaignId |
| customer_id | The ID of the customer. | ExternalCustomerId |
| metrics_absolute_top_impression_percentage | The percent of your ad impressions that are shown as the very first ad above the organic search results. | AbsoluteTopImpressionPercentage |
| metrics_all_conversions | The total number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | AllConversions |
| metrics_all_conversions_from_interactions_rate | All conversions from interactions (as oppose to view through conversions) divided by the number of ad interactions. | AllConversionRate |
| metrics_all_conversions_value | The total value of all conversions. | AllConversionValue |
| metrics_average_cpe | The average amount that you've been charged for an ad engagement. This amount is the total cost of all ad engagements divided by the total number of ad engagements. | AverageCpe |
| metrics_trueview_average_cpv | The average amount you pay each time someone views your ad. The average CPV is defined by the total cost of all ad views divided by the number of views. | AverageCpv |
| metrics_average_page_views | Average number of pages viewed per session. | AveragePageviews |
| metrics_average_time_on_site | Total duration of all sessions (in seconds) / number of sessions. Imported from Google Analytics. | AverageTimeOnSite |
| metrics_bounce_rate | Percentage of clicks where the user only visited a single page on your site. Imported from Google Analytics. | BounceRate |
| metrics_content_budget_lost_impression_share | The estimated percent of times that your ad was eligible to show on the Display Network but didn't because your budget was too low. Note: Content budget lost impression share is reported in the range of 0 to 0.9. Any value above 0.9 is reported as 0.9001. | ContentBudgetLostImpressionShare |
| metrics_content_impression_share | The impressions you've received on the Display Network divided by the estimated number of impressions you were eligible to receive. Note: Content impression share is reported in the range of 0.1 to 1. Any value below 0.1 is reported as 0.0999. | ContentImpressionShare |
| metrics_content_rank_lost_impression_share | The estimated percentage of impressions on the Display Network that your ads didn't receive due to poor Ad Rank. Note: Content rank lost impression share is reported in the range of 0 to 0.9. Any value above 0.9 is reported as 0.9001. | ContentRankLostImpressionShare |
| metrics_cost_per_all_conversions | The cost of ad interactions divided by all conversions. | CostPerAllConversion |
| metrics_cross_device_conversions | Conversions from when a customer clicks on a Google Ads ad on one device, then converts on a different device or browser. Cross-device conversions are already included in all_conversions. | CrossDeviceConversions |
| metrics_engagement_rate | How often people engage with your ad after it's shown to them. This is the number of ad expansions divided by the number of times your ad is shown. | EngagementRate |
| metrics_engagements | The number of engagements. An engagement occurs when a viewer expands your Lightbox ad. Also, in the future, other ad types may support engagement metrics. | Engagements |
| metrics_invalid_click_rate | The percentage of clicks filtered out of your total number of clicks (filtered + non-filtered clicks) during the reporting period. | InvalidClickRate |
| metrics_invalid_clicks | Number of clicks Google considers illegitimate and doesn't charge you for. | InvalidClicks |
| metrics_percent_new_visitors | Percentage of first-time sessions (from people who had never visited your site before). Imported from Google Analytics. | PercentNewVisitors |
| metrics_phone_calls | Number of offline phone calls. | NumOfflineInteractions |
| metrics_phone_impressions | Number of offline phone impressions. | NumOfflineImpressions |
| metrics_phone_through_rate | Number of phone calls received (phone_calls) divided by the number of times your phone number is shown (phone_impressions). | OfflineInteractionRate |
| metrics_relative_ctr | Your clickthrough rate (Ctr) divided by the average clickthrough rate of all advertisers on the websites that show your ads. Measures how your ads perform on Display Network sites compared to other ads on the same sites. | RelativeCtr |
| metrics_search_absolute_top_impression_share | The percentage of the customer's Shopping or Search ad impressions that are shown in the most prominent Shopping position. See https://support.google.com/google-ads/answer/7501826 for details. Any value below 0.1 is reported as 0.0999. | SearchAbsoluteTopImpressionShare |
| metrics_search_budget_lost_absolute_top_impression_share | The number estimating how often your ad wasn't the very first ad above the organic search results due to a low budget. Note: Search budget lost absolute top impression share is reported in the range of 0 to 0.9. Any value above 0.9 is reported as 0.9001. | SearchBudgetLostAbsoluteTopImpressionShare |
| metrics_search_budget_lost_impression_share | The estimated percent of times that your ad was eligible to show on the Search Network but didn't because your budget was too low. Note: Search budget lost impression share is reported in the range of 0 to 0.9. Any value above 0.9 is reported as 0.9001. | SearchBudgetLostImpressionShare |
| metrics_search_budget_lost_top_impression_share | The number estimating how often your ad didn't show anywhere above the organic search results due to a low budget. Note: Search budget lost top impression share is reported in the range of 0 to 0.9. Any value above 0.9 is reported as 0.9001. | SearchBudgetLostTopImpressionShare |
| metrics_search_click_share | The number of clicks you've received on the Search Network divided by the estimated number of clicks you were eligible to receive. Note: Search click share is reported in the range of 0.1 to 1. Any value below 0.1 is reported as 0.0999. | SearchClickShare |
| metrics_search_exact_match_impression_share | The impressions you've received divided by the estimated number of impressions you were eligible to receive on the Search Network for search terms that matched your keywords exactly (or were close variants of your keyword), regardless of your keyword match types. Note: Search exact match impression share is reported in the range of 0.1 to 1. Any value below 0.1 is reported as 0.0999. | SearchExactMatchImpressionShare |
| metrics_search_impression_share | The impressions you've received on the Search Network divided by the estimated number of impressions you were eligible to receive. Note: Search impression share is reported in the range of 0.1 to 1. Any value below 0.1 is reported as 0.0999. | SearchImpressionShare |
| metrics_search_rank_lost_absolute_top_impression_share | The number estimating how often your ad wasn't the very first ad above the organic search results due to poor Ad Rank. Note: Search rank lost absolute top impression share is reported in the range of 0 to 0.9. Any value above 0.9 is reported as 0.9001. | SearchRankLostAbsoluteTopImpressionShare |
| metrics_search_rank_lost_impression_share | The estimated percentage of impressions on the Search Network that your ads didn't receive due to poor Ad Rank. Note: Search rank lost impression share is reported in the range of 0 to 0.9. Any value above 0.9 is reported as 0.9001. | SearchRankLostImpressionShare |
| metrics_search_rank_lost_top_impression_share | The number estimating how often your ad didn't show anywhere above the organic search results due to poor Ad Rank. Note: Search rank lost top impression share is reported in the range of 0 to 0.9. Any value above 0.9 is reported as 0.9001. | SearchRankLostTopImpressionShare |
| metrics_search_top_impression_share | The impressions you've received in the top location (anywhere above the organic search results) compared to the estimated number of impressions you were eligible to receive in the top location. Note: Search top impression share is reported in the range of 0.1 to 1. Any value below 0.1 is reported as 0.0999. | SearchTopImpressionShare |
| metrics_top_impression_percentage | The percent of your ad impressions that are shown anywhere above the organic search results. | TopImpressionPercentage |
| metrics_value_per_all_conversions | The value of all conversions divided by the number of all conversions. | ValuePerAllConversion |
| metrics_video_quartile_p100_rate | Percentage of impressions where the viewer watched all of your video. | VideoQuartile100Rate |
| metrics_video_quartile_p25_rate | Percentage of impressions where the viewer watched 25% of your video. | VideoQuartile25Rate |
| metrics_video_quartile_p50_rate | Percentage of impressions where the viewer watched 50% of your video. | VideoQuartile50Rate |
| metrics_video_quartile_p75_rate | Percentage of impressions where the viewer watched 75% of your video. | VideoQuartile75Rate |
| metrics_video_trueview_view_rate | The number of views your TrueView video ad receives divided by its number of impressions, including thumbnail impressions for TrueView in-display ads. | VideoViewRate |
| metrics_video_trueview_views | The number of times your video ads were viewed. | VideoViews |
| segments_ad_network_type | Ad network type. | AdNetworkType2 |
| segments_date | Date to which metrics apply. yyyy-MM-dd format. For example, 2018-04-17. | Date |
| segments_day_of_week | Day of the week. For example, MONDAY. | DayOfWeek |
| segments_month | Month as represented by the date of the first day of a month. Formatted as yyyy-MM-dd. | Month |
| segments_quarter | Quarter as represented by the date of the first day of a quarter. Uses the calendar year for quarters. For example, the second quarter of 2018 starts on 2018-04-01. Formatted as yyyy-MM-dd. | Quarter |
| segments_week | Week as defined as Monday through Sunday, and represented by the date of Monday. Formatted as yyyy-MM-dd. | Week |
| segments_year | Year, formatted as yyyy. | Year |

Google Ads Table Name: CampaignLabel


Google Ads API Resource:
[campaign_label](https://developers.google.com/google-ads/api/fields/v22/campaign_label)

| Google Ads Field Name | Description | Adwords Mapped Field Name |
|---|---|---|
| campaign_id | The ID of the campaign. |   |
| campaign_label_campaign | The campaign to which the label is attached. |   |
| campaign_label_label | The label assigned to the campaign. |   |
| campaign_label_resource_name | Name of the resource. Campaign label resource names have the form: \`customers/{customer_id}/campaignLabels/{campaign_id}\~{label_id}\` |   |
| campaign_name | The name of the campaign. This field is required and shouldn't be empty when creating new campaigns. It must not contain any null (code point 0x0), NL line feed (code point 0xA) or carriage return (code point 0xD) characters. |   |
| label_id | Id of the label. Read only. |   |
| label_name | The name of the label. This field is required and shouldn't be empty when creating a new label. The length of this string should be between 1 and 80, inclusive. |   |
| label_resource_name | Name of the resource. Label resource names have the form: \`customers/{customer_id}/labels/{label_id}\` |   |

Google Ads Table Name: CampaignLocationTargetStats


Google Ads API Resource:
[location_view](https://developers.google.com/google-ads/api/fields/v22/location_view)

| Google Ads Field Name | Description | Adwords Mapped Field Name |
|---|---|---|
| campaign_criterion_criterion_id | The ID of the criterion. This field is ignored during mutate. | CriterionId |
| campaign_id | The ID of the campaign. | CampaignId |
| customer_id | The ID of the customer. | ExternalCustomerId |
| metrics_all_conversions | The total number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | AllConversions |
| metrics_all_conversions_from_interactions_rate | All conversions from interactions (as oppose to view through conversions) divided by the number of ad interactions. | AllConversionRate |
| metrics_all_conversions_value | The total value of all conversions. | AllConversionValue |
| metrics_average_cost | The average amount you pay per interaction. This amount is the total cost of your ads divided by the total number of interactions. | AverageCost |
| metrics_average_cpc | The total cost of all clicks divided by the total number of clicks received. | AverageCpc |
| metrics_average_cpe | The average amount that you've been charged for an ad engagement. This amount is the total cost of all ad engagements divided by the total number of ad engagements. | AverageCpe |
| metrics_average_cpm | Average cost-per-thousand impressions (CPM). | AverageCpm |
| metrics_trueview_average_cpv | The average amount you pay each time someone views your ad. The average CPV is defined by the total cost of all ad views divided by the number of views. | AverageCpv |
| metrics_clicks | The number of clicks. | Clicks |
| metrics_conversions | The number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | Conversions |
| metrics_conversions_from_interactions_rate | Conversions from interactions divided by the number of ad interactions (such as clicks for text ads or views for video ads). This only includes conversion actions which include_in_conversions_metric attribute is set to true. | ConversionRate |
| metrics_conversions_value | The total value of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | ConversionValue |
| metrics_cost_micros | The sum of your cost-per-click (CPC) and cost-per-thousand impressions (CPM) costs during this period. | Cost |
| metrics_cost_per_all_conversions | The cost of ad interactions divided by all conversions. | CostPerAllConversion |
| metrics_cost_per_conversion | The cost of ad interactions divided by conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | CostPerConversion |
| metrics_cross_device_conversions | Conversions from when a customer clicks on a Google Ads ad on one device, then converts on a different device or browser. Cross-device conversions are already included in all_conversions. | CrossDeviceConversions |
| metrics_ctr | The number of clicks your ad receives (Clicks) divided by the number of times your ad is shown (Impressions). | Ctr |
| metrics_engagement_rate | How often people engage with your ad after it's shown to them. This is the number of ad expansions divided by the number of times your ad is shown. | EngagementRate |
| metrics_engagements | The number of engagements. An engagement occurs when a viewer expands your Lightbox ad. Also, in the future, other ad types may support engagement metrics. | Engagements |
| metrics_impressions | Count of how often your ad has appeared on a search results page or website on the Google Network. | Impressions |
| metrics_interaction_event_types | The types of payable and free interactions. | InteractionTypes |
| metrics_interaction_rate | How often people interact with your ad after it is shown to them. This is the number of interactions divided by the number of times your ad is shown. | InteractionRate |
| metrics_interactions | The number of interactions. An interaction is the main user action associated with an ad format, such as clicks for text and shopping ads or views for video ads. | Interactions |
| metrics_value_per_all_conversions | The value of all conversions divided by the number of all conversions. | ValuePerAllConversion |
| metrics_value_per_conversion | The value of conversions divided by the number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | ValuePerConversion |
| metrics_video_trueview_view_rate | The number of views your TrueView video ad receives divided by its number of impressions, including thumbnail impressions for TrueView in-display ads. | VideoViewRate |
| metrics_video_trueview_views | The number of times your video ads were viewed. | VideoViews |
| metrics_view_through_conversions | The total number of view-through conversions. These happen when a customer sees an image or rich media ad, then later completes a conversion on your site without interacting with (for example, clicking on) another ad. | ViewThroughConversions |
| segments_date | Date to which metrics apply. yyyy-MM-dd format. For example, 2018-04-17. | Date |
| segments_month | Month as represented by the date of the first day of a month. Formatted as yyyy-MM-dd. | Month |
| segments_quarter | Quarter as represented by the date of the first day of a quarter. Uses the calendar year for quarters. For example, the second quarter of 2018 starts on 2018-04-01. Formatted as yyyy-MM-dd. | Quarter |
| segments_week | Week as defined as Monday through Sunday, and represented by the date of Monday. Formatted as yyyy-MM-dd. | Week |
| segments_year | Year, formatted as yyyy. | Year |

Google Ads Table Name: CampaignStats


Google Ads API Resource:
[campaign](https://developers.google.com/google-ads/api/fields/v22/campaign)

| Google Ads Field Name | Description | Adwords Mapped Field Name |
|---|---|---|
| campaign_base_campaign | The resource name of the base campaign of a draft or experiment campaign. For base campaigns, this is equal to \`resource_name\`. This field is read-only. | BaseCampaignId |
| campaign_id | The ID of the campaign. | CampaignId |
| customer_id | The ID of the customer. | ExternalCustomerId |
| metrics_active_view_cpm | Average cost of viewable impressions (\`active_view_impressions\`). | ActiveViewCpm |
| metrics_active_view_ctr | Active view measurable clicks divided by active view viewable impressions. This metric is reported only for display network. | ActiveViewCtr |
| metrics_active_view_impressions | A measurement of how often your ad has become viewable on a Display Network site. | ActiveViewImpressions |
| metrics_active_view_measurability | The ratio of impressions that could be measured by Active View over the number of served impressions. | ActiveViewMeasurability |
| metrics_active_view_measurable_cost_micros | The cost of the impressions you received that were measurable by Active View. | ActiveViewMeasurableCost |
| metrics_active_view_measurable_impressions | The number of times your ads are appearing on placements in positions where they can be seen. | ActiveViewMeasurableImpressions |
| metrics_active_view_viewability | The percentage of time when your ad appeared on an Active View enabled site (measurable impressions) and was viewable (viewable impressions). | ActiveViewViewability |
| metrics_average_cost | The average amount you pay per interaction. This amount is the total cost of your ads divided by the total number of interactions. | AverageCost |
| metrics_average_cpc | The total cost of all clicks divided by the total number of clicks received. | AverageCpc |
| metrics_average_cpm | Average cost-per-thousand impressions (CPM). | AverageCpm |
| metrics_clicks | The number of clicks. | Clicks |
| metrics_conversions | The number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | Conversions |
| metrics_conversions_from_interactions_rate | Conversions from interactions divided by the number of ad interactions (such as clicks for text ads or views for video ads). This only includes conversion actions which include_in_conversions_metric attribute is set to true. | ConversionRate |
| metrics_conversions_value | The total value of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | ConversionValue |
| metrics_cost_micros | The sum of your cost-per-click (CPC) and cost-per-thousand impressions (CPM) costs during this period. | Cost |
| metrics_cost_per_conversion | The cost of ad interactions divided by conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | CostPerConversion |
| metrics_cost_per_current_model_attributed_conversion | The cost of ad interactions divided by current model attributed conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | CostPerCurrentModelAttributedConversion |
| metrics_ctr | The number of clicks your ad receives (Clicks) divided by the number of times your ad is shown (Impressions). | Ctr |
| metrics_current_model_attributed_conversions | Shows how your historic conversions data would look under the attribution model you've selected. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | CurrentModelAttributedConversions |
| metrics_current_model_attributed_conversions_value | The total value of current model attributed conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | CurrentModelAttributedConversionValue |
| metrics_gmail_forwards | The number of times the ad was forwarded to someone else as a message. | GmailForwards |
| metrics_gmail_saves | The number of times someone has saved your Gmail ad to their inbox as a message. | GmailSaves |
| metrics_gmail_secondary_clicks | The number of clicks to the landing page on the expanded state of Gmail ads. | GmailSecondaryClicks |
| metrics_impressions | Count of how often your ad has appeared on a search results page or website on the Google Network. | Impressions |
| metrics_interaction_event_types | The types of payable and free interactions. | InteractionTypes |
| metrics_interaction_rate | How often people interact with your ad after it is shown to them. This is the number of interactions divided by the number of times your ad is shown. | InteractionRate |
| metrics_interactions | The number of interactions. An interaction is the main user action associated with an ad format, such as clicks for text and shopping ads or views for video ads. | Interactions |
| metrics_value_per_conversion | The value of conversions divided by the number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | ValuePerConversion |
| metrics_value_per_current_model_attributed_conversion | The value of current model attributed conversions divided by the number of the conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | ValuePerCurrentModelAttributedConversion |
| segments_ad_network_type | Ad network type. | AdNetworkType2 |
| segments_click_type | Click type. | ClickType |
| segments_date | Date to which metrics apply. yyyy-MM-dd format. For example, 2018-04-17. | Date |
| segments_day_of_week | Day of the week. For example, MONDAY. | DayOfWeek |
| segments_device | Device to which metrics apply. | Device |
| segments_month | Month as represented by the date of the first day of a month. Formatted as yyyy-MM-dd. | Month |
| segments_quarter | Quarter as represented by the date of the first day of a quarter. Uses the calendar year for quarters. For example, the second quarter of 2018 starts on 2018-04-01. Formatted as yyyy-MM-dd. | Quarter |
| segments_week | Week as defined as Monday through Sunday, and represented by the date of Monday. Formatted as yyyy-MM-dd. | Week |
| segments_year | Year, formatted as yyyy. | Year |

Google Ads Table Name: ClickStats


Google Ads API Resource:
[click_view](https://developers.google.com/google-ads/api/fields/v22/click_view)

| Google Ads Field Name | Description | Adwords Mapped Field Name |
|---|---|---|
| ad_group_id | The ID of the ad group. | AdGroupId |
| ad_group_status | The status of the ad group. |   |
| campaign_id | The ID of the campaign. | CampaignId |
| click_view_ad_group_ad | The associated ad. | CreativeId |
| click_view_area_of_interest_city | The city location criterion associated with the impression. | AoiCityCriteriaId |
| click_view_area_of_interest_country | The country location criterion associated with the impression. | AoiCountryCriteriaId |
| click_view_area_of_interest_metro | The metro location criterion associated with the impression. | AoiMetroCriteriaId |
| click_view_area_of_interest_most_specific | The most specific location criterion associated with the impression. | AoiMostSpecificTargetId |
| click_view_area_of_interest_region | The region location criterion associated with the impression. | AoiRegionCriteriaId |
| click_view_gclid | The Google Click ID. | GclId |
| click_view_keyword | The associated keyword, if one exists and the click corresponds to the SEARCH channel. | CriteriaId |
| click_view_keyword_info_match_type | The match type of the keyword. | KeywordMatchType |
| click_view_keyword_info_text | The text of the keyword (at most 80 characters and 10 words). | CriteriaParameters |
| click_view_location_of_presence_city | The city location criterion associated with the impression. | LopCityCriteriaId |
| click_view_location_of_presence_country | The country location criterion associated with the impression. | LopCountryCriteriaId |
| click_view_location_of_presence_metro | The metro location criterion associated with the impression. | LopMetroCriteriaId |
| click_view_location_of_presence_most_specific | The most specific location criterion associated with the impression. | LopMostSpecificTargetId |
| click_view_location_of_presence_region | The region location criterion associated with the impression. | LopRegionCriteriaId |
| click_view_page_number | Page number in search results where the ad was shown. | Page |
| customer_descriptive_name | Optional, non-unique descriptive name of the customer. | AccountDescriptiveName |
| customer_id | The ID of the customer. | ExternalCustomerId |
| metrics_clicks | The number of clicks. | Clicks |
| segments_ad_network_type | Ad network type. | AdNetworkType2 |
| segments_click_type | Click type. | ClickType |
| segments_date | Date to which metrics apply. yyyy-MM-dd format. For example, 2018-04-17. | Date |
| segments_device | Device to which metrics apply. | Device |
| segments_slot | Position of the ad. | Slot |

Google Ads Table Name: Customer


Google Ads API Resource:
[customer](https://developers.google.com/google-ads/api/fields/v22/customer)

| Google Ads Field Name | Description | Adwords Mapped Field Name |
|---|---|---|
| customer_auto_tagging_enabled | Whether autotagging is enabled for the customer. | IsAutoTaggingEnabled |
| customer_currency_code | The currency in which the account operates. A subset of the currency codes from the ISO 4217 standard is supported. | AccountCurrencyCode |
| customer_descriptive_name | Optional, non-unique descriptive name of the customer. | AccountDescriptiveName,CustomerDescriptiveName |
| customer_id | The ID of the customer. | ExternalCustomerId |
| customer_manager | Whether the customer is a manager. | CanManageClients |
| customer_test_account | Whether the customer is a test account. | IsTestAccount |
| customer_time_zone | The local timezone ID of the customer. | AccountTimeZone |

Google Ads Table Name: DisplayVideoAutomaticPlacementsStats


Google Ads API Resource:
[group_placement_view](https://developers.google.com/google-ads/api/fields/v22/group_placement_view)

| Google Ads Field Name | Description | Adwords Mapped Field Name |
|---|---|---|
| ad_group_id | The ID of the ad group. |   |
| ad_group_name | The name of the ad group. This field is required and shouldn't be empty when creating new ad groups. It must contain fewer than 255 UTF-8 full-width characters. It must not contain any null (code point 0x0), NL line feed (code point 0xA) or carriage return (code point 0xD) characters. |   |
| campaign_id | The ID of the campaign. |   |
| campaign_name | The name of the campaign. This field is required and shouldn't be empty when creating new campaigns. It must not contain any null (code point 0x0), NL line feed (code point 0xA) or carriage return (code point 0xD) characters. |   |
| customer_id | The ID of the customer. |   |
| group_placement_view_placement | The automatic placement string at group level, e. g. web domain, mobile app ID, or a YouTube channel ID. |   |
| group_placement_view_placement_type | Type of the placement. For example, Website, YouTube Channel, Mobile Application. |   |
| metrics_average_cpc | The total cost of all clicks divided by the total number of clicks received. |   |
| metrics_average_cpm | Average cost-per-thousand impressions (CPM). |   |
| metrics_trueview_average_cpv | The average amount you pay each time someone views your ad. The average CPV is defined by the total cost of all ad views divided by the number of views. |   |
| metrics_clicks | The number of clicks. |   |
| metrics_cost_micros | The sum of your cost-per-click (CPC) and cost-per-thousand impressions (CPM) costs during this period. |   |
| metrics_ctr | The number of clicks your ad receives (Clicks) divided by the number of times your ad is shown (Impressions). |   |
| metrics_impressions | Count of how often your ad has appeared on a search results page or website on the Google Network. |   |
| metrics_video_trueview_view_rate | The number of views your TrueView video ad receives divided by its number of impressions, including thumbnail impressions for TrueView in-display ads. |   |
| metrics_video_trueview_views | The number of times your video ads were viewed. |   |
| segments_date | Date to which metrics apply. yyyy-MM-dd format. For example, 2018-04-17. |   |
| segments_day_of_week | Day of the week. For example, MONDAY. |   |
| segments_month | Month as represented by the date of the first day of a month. Formatted as yyyy-MM-dd. |   |
| segments_quarter | Quarter as represented by the date of the first day of a quarter. Uses the calendar year for quarters. For example, the second quarter of 2018 starts on 2018-04-01. Formatted as yyyy-MM-dd. |   |
| segments_week | Week as defined as Monday through Sunday, and represented by the date of Monday. Formatted as yyyy-MM-dd. |   |
| segments_year | Year, formatted as yyyy. |   |

Google Ads Table Name: DisplayVideoKeywordStats


Google Ads API Resource:
[display_keyword_view](https://developers.google.com/google-ads/api/fields/v22/display_keyword_view)

| Google Ads Field Name | Description | Adwords Mapped Field Name |
|---|---|---|
| ad_group_criterion_keyword_text | The text of the keyword (at most 80 characters and 10 words). |   |
| ad_group_id | The ID of the ad group. |   |
| ad_group_name | The name of the ad group. This field is required and shouldn't be empty when creating new ad groups. It must contain fewer than 255 UTF-8 full-width characters. It must not contain any null (code point 0x0), NL line feed (code point 0xA) or carriage return (code point 0xD) characters. |   |
| campaign_advertising_channel_sub_type | Optional refinement to \`advertising_channel_type\`. Must be a valid sub-type of the parent channel type. Can be set only when creating campaigns. After campaign is created, the field cannot be changed. |   |
| campaign_bidding_strategy_type | The type of bidding strategy. A bidding strategy can be created by setting either the bidding scheme to create a standard bidding strategy or the \`bidding_strategy\` field to create a portfolio bidding strategy. This field is read-only. |   |
| campaign_id | The ID of the campaign. |   |
| campaign_name | The name of the campaign. This field is required and shouldn't be empty when creating new campaigns. It must not contain any null (code point 0x0), NL line feed (code point 0xA) or carriage return (code point 0xD) characters. |   |
| customer_id | The ID of the customer. |   |
| metrics_average_cpc | The total cost of all clicks divided by the total number of clicks received. |   |
| metrics_average_cpm | Average cost-per-thousand impressions (CPM). |   |
| metrics_trueview_average_cpv | The average amount you pay each time someone views your ad. The average CPV is defined by the total cost of all ad views divided by the number of views. |   |
| metrics_clicks | The number of clicks. |   |
| metrics_conversions | The number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. |   |
| metrics_conversions_from_interactions_rate | Conversions from interactions divided by the number of ad interactions (such as clicks for text ads or views for video ads). This only includes conversion actions which include_in_conversions_metric attribute is set to true. |   |
| metrics_cost_micros | The sum of your cost-per-click (CPC) and cost-per-thousand impressions (CPM) costs during this period. |   |
| metrics_cost_per_conversion | The cost of ad interactions divided by conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. |   |
| metrics_impressions | Count of how often your ad has appeared on a search results page or website on the Google Network. |   |
| metrics_interaction_rate | How often people interact with your ad after it is shown to them. This is the number of interactions divided by the number of times your ad is shown. |   |
| metrics_interactions | The number of interactions. An interaction is the main user action associated with an ad format, such as clicks for text and shopping ads or views for video ads. |   |
| metrics_view_through_conversions | The total number of view-through conversions. These happen when a customer sees an image or rich media ad, then later completes a conversion on your site without interacting with (for example, clicking on) another ad. |   |
| segments_date | Date to which metrics apply. yyyy-MM-dd format. For example, 2018-04-17. |   |
| segments_day_of_week | Day of the week. For example, MONDAY. |   |
| segments_month | Month as represented by the date of the first day of a month. Formatted as yyyy-MM-dd. |   |
| segments_quarter | Quarter as represented by the date of the first day of a quarter. Uses the calendar year for quarters. For example, the second quarter of 2018 starts on 2018-04-01. Formatted as yyyy-MM-dd. |   |
| segments_week | Week as defined as Monday through Sunday, and represented by the date of Monday. Formatted as yyyy-MM-dd. |   |
| segments_year | Year, formatted as yyyy. |   |

Google Ads Table Name: DisplayVideoTopicStats


Google Ads API Resource:
[topic_view](https://developers.google.com/google-ads/api/fields/v22/topic_view)

| Google Ads Field Name | Description | Adwords Mapped Field Name |
|---|---|---|
| ad_group_criterion_status | The status of the criterion. |   |
| ad_group_criterion_topic_path | The category to target or exclude. Each subsequent element in the array describes a more specific sub-category. For example, \\"Pets \& Animals\\", \\"Pets\\", \\"Dogs\\" represents the \\"Pets \& Animals/Pets/Dogs\\" category. |   |
| ad_group_id | The ID of the ad group. |   |
| ad_group_name | The name of the ad group. This field is required and shouldn't be empty when creating new ad groups. It must contain fewer than 255 UTF-8 full-width characters. It must not contain any null (code point 0x0), NL line feed (code point 0xA) or carriage return (code point 0xD) characters. |   |
| campaign_id | The ID of the campaign. |   |
| campaign_name | The name of the campaign. This field is required and shouldn't be empty when creating new campaigns. It must not contain any null (code point 0x0), NL line feed (code point 0xA) or carriage return (code point 0xD) characters. |   |
| customer_id | The ID of the customer. |   |
| metrics_average_cpc | The total cost of all clicks divided by the total number of clicks received. |   |
| metrics_average_cpm | Average cost-per-thousand impressions (CPM). |   |
| metrics_clicks | The number of clicks. |   |
| metrics_cost_micros | The sum of your cost-per-click (CPC) and cost-per-thousand impressions (CPM) costs during this period. |   |
| metrics_ctr | The number of clicks your ad receives (Clicks) divided by the number of times your ad is shown (Impressions). |   |
| metrics_impressions | Count of how often your ad has appeared on a search results page or website on the Google Network. |   |
| segments_date | Date to which metrics apply. yyyy-MM-dd format. For example, 2018-04-17. |   |
| segments_day_of_week | Day of the week. For example, MONDAY. |   |
| segments_month | Month as represented by the date of the first day of a month. Formatted as yyyy-MM-dd. |   |
| segments_quarter | Quarter as represented by the date of the first day of a quarter. Uses the calendar year for quarters. For example, the second quarter of 2018 starts on 2018-04-01. Formatted as yyyy-MM-dd. |   |
| segments_week | Week as defined as Monday through Sunday, and represented by the date of Monday. Formatted as yyyy-MM-dd. |   |
| segments_year | Year, formatted as yyyy. |   |

Google Ads Table Name: Gender


Google Ads API Resource:
[gender_view](https://developers.google.com/google-ads/api/fields/v22/gender_view)

| Google Ads Field Name | Description | Adwords Mapped Field Name |
|---|---|---|
| ad_group_base_ad_group | For draft or experiment ad groups, this field is the resource name of the base ad group from which this ad group was created. If a draft or experiment ad group does not have a base ad group, then this field is null. For base ad groups, this field equals the ad group resource name. This field is read-only. | BaseAdGroupId |
| ad_group_criterion_bid_modifier | The modifier for the bid when the criterion matches. The modifier must be in the range: 0.1 - 10.0. Most targetable criteria types support modifiers. | BidModifier |
| ad_group_criterion_criterion_id | The ID of the criterion. This field is ignored for mutates. | CriterionId |
| ad_group_criterion_effective_cpc_bid_micros | The effective CPC (cost-per-click) bid. | CpcBid |
| ad_group_criterion_effective_cpc_bid_source | Source of the effective CPC bid. | CpcBidSource |
| ad_group_criterion_effective_cpm_bid_micros | The effective CPM (cost-per-thousand viewable impressions) bid. | CpmBidStr |
| ad_group_criterion_effective_cpm_bid_source | Source of the effective CPM bid. | CpmBidSource |
| ad_group_criterion_final_mobile_urls | The list of possible final mobile URLs after all cross-domain redirects. | FinalMobileUrls |
| ad_group_criterion_final_urls | The list of possible final URLs after all cross-domain redirects for the ad. | FinalUrls |
| ad_group_criterion_gender_type | Type of the gender. | Criteria |
| ad_group_criterion_negative | Whether to target (\`false\`) or exclude (\`true\`) the criterion. This field is immutable. To switch a criterion from positive to negative, remove then re-add it. | IsNegative |
| ad_group_criterion_status | The status of the criterion. | Status |
| ad_group_criterion_tracking_url_template | The URL template for constructing a tracking URL. | TrackingUrlTemplate |
| ad_group_criterion_url_custom_parameters | The list of mappings used to substitute custom parameter tags in a \`tracking_url_template\`, \`final_urls\`, or \`mobile_final_urls\`. | UrlCustomParameters |
| ad_group_id | The ID of the ad group. | AdGroupId |
| ad_group_targeting_setting_target_restrictions | The per-targeting-dimension setting to restrict the reach of your campaign or ad group. |   |
| bidding_strategy_name | The name of the bidding strategy. All bidding strategies within an account must be named distinctly. The length of this string should be between 1 and 255, inclusive, in UTF-8 bytes, (trimmed). | BiddingStrategyName |
| bidding_strategy_type | The type of the bidding strategy. Create a bidding strategy by setting the bidding scheme. This field is read-only. | BiddingStrategyType |
| campaign_base_campaign | The resource name of the base campaign of a draft or experiment campaign. For base campaigns, this is equal to \`resource_name\`. This field is read-only. | BaseCampaignId |
| campaign_bidding_strategy | Portfolio bidding strategy used by campaign. | BiddingStrategyId |
| campaign_id | The ID of the campaign. | CampaignId |
| customer_id | The ID of the customer. | ExternalCustomerId |

Google Ads Table Name: GenderBasicStats


Google Ads API Resource:
[gender_view](https://developers.google.com/google-ads/api/fields/v22/gender_view)

| Google Ads Field Name | Description | Adwords Mapped Field Name |
|---|---|---|
| ad_group_base_ad_group | For draft or experiment ad groups, this field is the resource name of the base ad group from which this ad group was created. If a draft or experiment ad group does not have a base ad group, then this field is null. For base ad groups, this field equals the ad group resource name. This field is read-only. | BaseAdGroupId |
| ad_group_criterion_criterion_id | The ID of the criterion. This field is ignored for mutates. | CriterionId |
| ad_group_id | The ID of the ad group. | AdGroupId |
| campaign_base_campaign | The resource name of the base campaign of a draft or experiment campaign. For base campaigns, this is equal to \`resource_name\`. This field is read-only. | BaseCampaignId |
| campaign_id | The ID of the campaign. | CampaignId |
| customer_id | The ID of the customer. | ExternalCustomerId |
| metrics_active_view_impressions | A measurement of how often your ad has become viewable on a Display Network site. | ActiveViewImpressions |
| metrics_active_view_measurability | The ratio of impressions that could be measured by Active View over the number of served impressions. | ActiveViewMeasurability |
| metrics_active_view_measurable_cost_micros | The cost of the impressions you received that were measurable by Active View. | ActiveViewMeasurableCost |
| metrics_active_view_measurable_impressions | The number of times your ads are appearing on placements in positions where they can be seen. | ActiveViewMeasurableImpressions |
| metrics_active_view_viewability | The percentage of time when your ad appeared on an Active View enabled site (measurable impressions) and was viewable (viewable impressions). | ActiveViewViewability |
| metrics_all_conversions | The total number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | AllConversions |
| metrics_all_conversions_value | The total value of all conversions. | AllConversionValue |
| metrics_clicks | The number of clicks. | Clicks |
| metrics_conversions | The number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | Conversions |
| metrics_conversions_value | The total value of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | ConversionValue |
| metrics_cost_micros | The sum of your cost-per-click (CPC) and cost-per-thousand impressions (CPM) costs during this period. | Cost |
| metrics_cross_device_conversions | Conversions from when a customer clicks on a Google Ads ad on one device, then converts on a different device or browser. Cross-device conversions are already included in all_conversions. | CrossDeviceConversions |
| metrics_impressions | Count of how often your ad has appeared on a search results page or website on the Google Network. | Impressions |
| metrics_interaction_event_types | The types of payable and free interactions. | InteractionTypes |
| metrics_interactions | The number of interactions. An interaction is the main user action associated with an ad format, such as clicks for text and shopping ads or views for video ads. | Interactions |
| metrics_view_through_conversions | The total number of view-through conversions. These happen when a customer sees an image or rich media ad, then later completes a conversion on your site without interacting with (for example, clicking on) another ad. | ViewThroughConversions |
| segments_ad_network_type | Ad network type. | AdNetworkType2 |
| segments_date | Date to which metrics apply. yyyy-MM-dd format. For example, 2018-04-17. | Date |
| segments_device | Device to which metrics apply. | Device |

Google Ads Table Name: GenderConversionStats


Google Ads API Resource:
[gender_view](https://developers.google.com/google-ads/api/fields/v22/gender_view)

| Google Ads Field Name | Description | Adwords Mapped Field Name |
|---|---|---|
| ad_group_base_ad_group | For draft or experiment ad groups, this field is the resource name of the base ad group from which this ad group was created. If a draft or experiment ad group does not have a base ad group, then this field is null. For base ad groups, this field equals the ad group resource name. This field is read-only. | BaseAdGroupId |
| ad_group_criterion_criterion_id | The ID of the criterion. This field is ignored for mutates. | CriterionId |
| ad_group_id | The ID of the ad group. | AdGroupId |
| campaign_base_campaign | The resource name of the base campaign of a draft or experiment campaign. For base campaigns, this is equal to \`resource_name\`. This field is read-only. | BaseCampaignId |
| campaign_id | The ID of the campaign. | CampaignId |
| customer_id | The ID of the customer. | ExternalCustomerId |
| metrics_all_conversions | The total number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | AllConversions |
| metrics_all_conversions_value | The total value of all conversions. | AllConversionValue |
| metrics_conversions | The number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | Conversions |
| metrics_conversions_value | The total value of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | ConversionValue |
| metrics_cross_device_conversions | Conversions from when a customer clicks on a Google Ads ad on one device, then converts on a different device or browser. Cross-device conversions are already included in all_conversions. | CrossDeviceConversions |
| metrics_value_per_all_conversions | The value of all conversions divided by the number of all conversions. | ValuePerAllConversion |
| metrics_value_per_conversion | The value of conversions divided by the number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | ValuePerConversion |
| segments_ad_network_type | Ad network type. | AdNetworkType2 |
| segments_click_type | Click type. | ClickType |
| segments_conversion_action | Resource name of the conversion action. | ConversionTrackerId |
| segments_conversion_action_category | Conversion action category. | ConversionCategoryName |
| segments_conversion_action_name | Conversion action name. | ConversionTypeName |
| segments_date | Date to which metrics apply. yyyy-MM-dd format. For example, 2018-04-17. | Date |
| segments_day_of_week | Day of the week. For example, MONDAY. | DayOfWeek |
| segments_device | Device to which metrics apply. | Device |
| segments_month | Month as represented by the date of the first day of a month. Formatted as yyyy-MM-dd. | Month |
| segments_quarter | Quarter as represented by the date of the first day of a quarter. Uses the calendar year for quarters. For example, the second quarter of 2018 starts on 2018-04-01. Formatted as yyyy-MM-dd. | Quarter |
| segments_week | Week as defined as Monday through Sunday, and represented by the date of Monday. Formatted as yyyy-MM-dd. | Week |
| segments_year | Year, formatted as yyyy. | Year |

Google Ads Table Name: GenderNonClickStats


Google Ads API Resource:
[gender_view](https://developers.google.com/google-ads/api/fields/v22/gender_view)

| Google Ads Field Name | Description | Adwords Mapped Field Name |
|---|---|---|
| ad_group_base_ad_group | For draft or experiment ad groups, this field is the resource name of the base ad group from which this ad group was created. If a draft or experiment ad group does not have a base ad group, then this field is null. For base ad groups, this field equals the ad group resource name. This field is read-only. | BaseAdGroupId |
| ad_group_criterion_criterion_id | The ID of the criterion. This field is ignored for mutates. | CriterionId |
| ad_group_id | The ID of the ad group. | AdGroupId |
| campaign_base_campaign | The resource name of the base campaign of a draft or experiment campaign. For base campaigns, this is equal to \`resource_name\`. This field is read-only. | BaseCampaignId |
| campaign_id | The ID of the campaign. | CampaignId |
| customer_id | The ID of the customer. | ExternalCustomerId |
| metrics_average_cpe | The average amount that you've been charged for an ad engagement. This amount is the total cost of all ad engagements divided by the total number of ad engagements. | AverageCpe |
| metrics_trueview_average_cpv | The average amount you pay each time someone views your ad. The average CPV is defined by the total cost of all ad views divided by the number of views. | AverageCpv |
| metrics_engagement_rate | How often people engage with your ad after it's shown to them. This is the number of ad expansions divided by the number of times your ad is shown. | EngagementRate |
| metrics_engagements | The number of engagements. An engagement occurs when a viewer expands your Lightbox ad. Also, in the future, other ad types may support engagement metrics. | Engagements |
| metrics_video_quartile_p100_rate | Percentage of impressions where the viewer watched all of your video. | VideoQuartile100Rate |
| metrics_video_quartile_p25_rate | Percentage of impressions where the viewer watched 25% of your video. | VideoQuartile25Rate |
| metrics_video_quartile_p50_rate | Percentage of impressions where the viewer watched 50% of your video. | VideoQuartile50Rate |
| metrics_video_quartile_p75_rate | Percentage of impressions where the viewer watched 75% of your video. | VideoQuartile75Rate |
| metrics_video_trueview_view_rate | The number of views your TrueView video ad receives divided by its number of impressions, including thumbnail impressions for TrueView in-display ads. | VideoViewRate |
| metrics_video_trueview_views | The number of times your video ads were viewed. | VideoViews |
| segments_ad_network_type | Ad network type. | AdNetworkType2 |
| segments_date | Date to which metrics apply. yyyy-MM-dd format. For example, 2018-04-17. | Date |
| segments_day_of_week | Day of the week. For example, MONDAY. | DayOfWeek |
| segments_device | Device to which metrics apply. | Device |
| segments_month | Month as represented by the date of the first day of a month. Formatted as yyyy-MM-dd. | Month |
| segments_quarter | Quarter as represented by the date of the first day of a quarter. Uses the calendar year for quarters. For example, the second quarter of 2018 starts on 2018-04-01. Formatted as yyyy-MM-dd. | Quarter |
| segments_week | Week as defined as Monday through Sunday, and represented by the date of Monday. Formatted as yyyy-MM-dd. | Week |
| segments_year | Year, formatted as yyyy. | Year |

Google Ads Table Name: GenderStats


Google Ads API Resource:
[gender_view](https://developers.google.com/google-ads/api/fields/v22/gender_view)

| Google Ads Field Name | Description | Adwords Mapped Field Name |
|---|---|---|
| ad_group_base_ad_group | For draft or experiment ad groups, this field is the resource name of the base ad group from which this ad group was created. If a draft or experiment ad group does not have a base ad group, then this field is null. For base ad groups, this field equals the ad group resource name. This field is read-only. | BaseAdGroupId |
| ad_group_criterion_criterion_id | The ID of the criterion. This field is ignored for mutates. | CriterionId |
| ad_group_id | The ID of the ad group. | AdGroupId |
| campaign_base_campaign | The resource name of the base campaign of a draft or experiment campaign. For base campaigns, this is equal to \`resource_name\`. This field is read-only. | BaseCampaignId |
| campaign_id | The ID of the campaign. | CampaignId |
| customer_id | The ID of the customer. | ExternalCustomerId |
| metrics_active_view_cpm | Average cost of viewable impressions (\`active_view_impressions\`). | ActiveViewCpm |
| metrics_active_view_ctr | Active view measurable clicks divided by active view viewable impressions. This metric is reported only for display network. | ActiveViewCtr |
| metrics_active_view_impressions | A measurement of how often your ad has become viewable on a Display Network site. | ActiveViewImpressions |
| metrics_active_view_measurability | The ratio of impressions that could be measured by Active View over the number of served impressions. | ActiveViewMeasurability |
| metrics_active_view_measurable_cost_micros | The cost of the impressions you received that were measurable by Active View. | ActiveViewMeasurableCost |
| metrics_active_view_measurable_impressions | The number of times your ads are appearing on placements in positions where they can be seen. | ActiveViewMeasurableImpressions |
| metrics_active_view_viewability | The percentage of time when your ad appeared on an Active View enabled site (measurable impressions) and was viewable (viewable impressions). | ActiveViewViewability |
| metrics_all_conversions | The total number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | AllConversions |
| metrics_all_conversions_from_interactions_rate | All conversions from interactions (as oppose to view through conversions) divided by the number of ad interactions. | AllConversionRate |
| metrics_all_conversions_value | The total value of all conversions. | AllConversionValue |
| metrics_average_cost | The average amount you pay per interaction. This amount is the total cost of your ads divided by the total number of interactions. | AverageCost |
| metrics_average_cpc | The total cost of all clicks divided by the total number of clicks received. | AverageCpc |
| metrics_average_cpm | Average cost-per-thousand impressions (CPM). | AverageCpm |
| metrics_clicks | The number of clicks. | Clicks |
| metrics_conversions | The number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | Conversions |
| metrics_conversions_from_interactions_rate | Conversions from interactions divided by the number of ad interactions (such as clicks for text ads or views for video ads). This only includes conversion actions which include_in_conversions_metric attribute is set to true. | ConversionRate |
| metrics_conversions_value | The total value of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | ConversionValue |
| metrics_cost_micros | The sum of your cost-per-click (CPC) and cost-per-thousand impressions (CPM) costs during this period. | Cost |
| metrics_cost_per_all_conversions | The cost of ad interactions divided by all conversions. | CostPerAllConversion |
| metrics_cost_per_conversion | The cost of ad interactions divided by conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | CostPerConversion |
| metrics_cross_device_conversions | Conversions from when a customer clicks on a Google Ads ad on one device, then converts on a different device or browser. Cross-device conversions are already included in all_conversions. | CrossDeviceConversions |
| metrics_ctr | The number of clicks your ad receives (Clicks) divided by the number of times your ad is shown (Impressions). | Ctr |
| metrics_gmail_forwards | The number of times the ad was forwarded to someone else as a message. | GmailForwards |
| metrics_gmail_saves | The number of times someone has saved your Gmail ad to their inbox as a message. | GmailSaves |
| metrics_gmail_secondary_clicks | The number of clicks to the landing page on the expanded state of Gmail ads. | GmailSecondaryClicks |
| metrics_impressions | Count of how often your ad has appeared on a search results page or website on the Google Network. | Impressions |
| metrics_interaction_event_types | The types of payable and free interactions. | InteractionTypes |
| metrics_interaction_rate | How often people interact with your ad after it is shown to them. This is the number of interactions divided by the number of times your ad is shown. | InteractionRate |
| metrics_interactions | The number of interactions. An interaction is the main user action associated with an ad format, such as clicks for text and shopping ads or views for video ads. | Interactions |
| metrics_value_per_all_conversions | The value of all conversions divided by the number of all conversions. | ValuePerAllConversion |
| metrics_value_per_conversion | The value of conversions divided by the number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | ValuePerConversion |
| segments_ad_network_type | Ad network type. | AdNetworkType2 |
| segments_click_type | Click type. | ClickType |
| segments_date | Date to which metrics apply. yyyy-MM-dd format. For example, 2018-04-17. | Date |
| segments_day_of_week | Day of the week. For example, MONDAY. | DayOfWeek |
| segments_device | Device to which metrics apply. | Device |
| segments_month | Month as represented by the date of the first day of a month. Formatted as yyyy-MM-dd. | Month |
| segments_quarter | Quarter as represented by the date of the first day of a quarter. Uses the calendar year for quarters. For example, the second quarter of 2018 starts on 2018-04-01. Formatted as yyyy-MM-dd. | Quarter |
| segments_week | Week as defined as Monday through Sunday, and represented by the date of Monday. Formatted as yyyy-MM-dd. | Week |
| segments_year | Year, formatted as yyyy. | Year |

Google Ads Table Name: GeoConversionStats


Google Ads API Resource:
[geographic_view](https://developers.google.com/google-ads/api/fields/v22/geographic_view)

| Google Ads Field Name | Description | Adwords Mapped Field Name |
|---|---|---|
| ad_group_id | The ID of the ad group. | AdGroupId |
| ad_group_name | The name of the ad group. This field is required and shouldn't be empty when creating new ad groups. It must contain fewer than 255 UTF-8 full-width characters. It must not contain any null (code point 0x0), NL line feed (code point 0xA) or carriage return (code point 0xD) characters. | AdGroupName |
| ad_group_status | The status of the ad group. | AdGroupStatus |
| campaign_id | The ID of the campaign. | CampaignId |
| campaign_status | The status of the campaign. When a new campaign is added, the status defaults to ENABLED. |   |
| customer_id | The ID of the customer. | ExternalCustomerId |
| geographic_view_country_criterion_id | Criterion Id for the country. | CountryCriteriaId |
| geographic_view_location_type | Type of the geo targeting of the campaign. | LocationType |
| metrics_all_conversions | The total number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | AllConversions |
| metrics_all_conversions_value | The total value of all conversions. | AllConversionValue |
| metrics_conversions | The number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | Conversions |
| metrics_conversions_value | The total value of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | ConversionValue |
| metrics_cross_device_conversions | Conversions from when a customer clicks on a Google Ads ad on one device, then converts on a different device or browser. Cross-device conversions are already included in all_conversions. | CrossDeviceConversions |
| metrics_value_per_all_conversions | The value of all conversions divided by the number of all conversions. | ValuePerAllConversion |
| metrics_value_per_conversion | The value of conversions divided by the number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | ValuePerConversion |
| metrics_view_through_conversions | The total number of view-through conversions. These happen when a customer sees an image or rich media ad, then later completes a conversion on your site without interacting with (for example, clicking on) another ad. | ViewThroughConversions |
| segments_ad_network_type | Ad network type. | AdNetworkType2 |
| segments_conversion_action | Resource name of the conversion action. | ConversionTrackerId |
| segments_conversion_action_category | Conversion action category. | ConversionCategoryName |
| segments_conversion_action_name | Conversion action name. | ConversionTypeName |
| segments_date | Date to which metrics apply. yyyy-MM-dd format. For example, 2018-04-17. | Date |
| segments_day_of_week | Day of the week. For example, MONDAY. | DayOfWeek |
| segments_device | Device to which metrics apply. | Device |
| segments_geo_target_most_specific_location | Resource name of the geo target constant that represents the most specific location. | MostSpecificCriteriaId |
| segments_month | Month as represented by the date of the first day of a month. Formatted as yyyy-MM-dd. | Month |
| segments_quarter | Quarter as represented by the date of the first day of a quarter. Uses the calendar year for quarters. For example, the second quarter of 2018 starts on 2018-04-01. Formatted as yyyy-MM-dd. | Quarter |
| segments_week | Week as defined as Monday through Sunday, and represented by the date of Monday. Formatted as yyyy-MM-dd. | Week |
| segments_year | Year, formatted as yyyy. | Year |

Google Ads Table Name: GeoStats


Google Ads API Resource:
[geographic_view](https://developers.google.com/google-ads/api/fields/v22/geographic_view)

| Google Ads Field Name | Description | Adwords Mapped Field Name |
|---|---|---|
| ad_group_id | The ID of the ad group. | AdGroupId |
| ad_group_name | The name of the ad group. This field is required and shouldn't be empty when creating new ad groups. It must contain fewer than 255 UTF-8 full-width characters. It must not contain any null (code point 0x0), NL line feed (code point 0xA) or carriage return (code point 0xD) characters. | AdGroupName |
| ad_group_status | The status of the ad group. | AdGroupStatus |
| campaign_id | The ID of the campaign. | CampaignId |
| campaign_status | The status of the campaign. When a new campaign is added, the status defaults to ENABLED. |   |
| customer_id | The ID of the customer. | ExternalCustomerId |
| geographic_view_country_criterion_id | Criterion Id for the country. | CountryCriteriaId |
| geographic_view_location_type | Type of the geo targeting of the campaign. | LocationType |
| metrics_all_conversions | The total number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | AllConversions |
| metrics_all_conversions_from_interactions_rate | All conversions from interactions (as oppose to view through conversions) divided by the number of ad interactions. | AllConversionRate |
| metrics_all_conversions_value | The total value of all conversions. | AllConversionValue |
| metrics_average_cost | The average amount you pay per interaction. This amount is the total cost of your ads divided by the total number of interactions. | AverageCost |
| metrics_average_cpc | The total cost of all clicks divided by the total number of clicks received. | AverageCpc |
| metrics_average_cpm | Average cost-per-thousand impressions (CPM). | AverageCpm |
| metrics_trueview_average_cpv | The average amount you pay each time someone views your ad. The average CPV is defined by the total cost of all ad views divided by the number of views. | AverageCpv |
| metrics_clicks | The number of clicks. | Clicks |
| metrics_conversions | The number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | Conversions |
| metrics_conversions_from_interactions_rate | Conversions from interactions divided by the number of ad interactions (such as clicks for text ads or views for video ads). This only includes conversion actions which include_in_conversions_metric attribute is set to true. | ConversionRate |
| metrics_conversions_value | The total value of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | ConversionValue |
| metrics_cost_micros | The sum of your cost-per-click (CPC) and cost-per-thousand impressions (CPM) costs during this period. | Cost |
| metrics_cost_per_all_conversions | The cost of ad interactions divided by all conversions. | CostPerAllConversion |
| metrics_cost_per_conversion | The cost of ad interactions divided by conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | CostPerConversion |
| metrics_cross_device_conversions | Conversions from when a customer clicks on a Google Ads ad on one device, then converts on a different device or browser. Cross-device conversions are already included in all_conversions. | CrossDeviceConversions |
| metrics_ctr | The number of clicks your ad receives (Clicks) divided by the number of times your ad is shown (Impressions). | Ctr |
| metrics_impressions | Count of how often your ad has appeared on a search results page or website on the Google Network. | Impressions |
| metrics_interaction_event_types | The types of payable and free interactions. | InteractionTypes |
| metrics_interaction_rate | How often people interact with your ad after it is shown to them. This is the number of interactions divided by the number of times your ad is shown. | InteractionRate |
| metrics_interactions | The number of interactions. An interaction is the main user action associated with an ad format, such as clicks for text and shopping ads or views for video ads. | Interactions |
| metrics_value_per_all_conversions | The value of all conversions divided by the number of all conversions. | ValuePerAllConversion |
| metrics_value_per_conversion | The value of conversions divided by the number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | ValuePerConversion |
| metrics_video_trueview_view_rate | The number of views your TrueView video ad receives divided by its number of impressions, including thumbnail impressions for TrueView in-display ads. | VideoViewRate |
| metrics_video_trueview_views | The number of times your video ads were viewed. | VideoViews |
| metrics_view_through_conversions | The total number of view-through conversions. These happen when a customer sees an image or rich media ad, then later completes a conversion on your site without interacting with (for example, clicking on) another ad. | ViewThroughConversions |
| segments_ad_network_type | Ad network type. | AdNetworkType2 |
| segments_date | Date to which metrics apply. yyyy-MM-dd format. For example, 2018-04-17. | Date |
| segments_day_of_week | Day of the week. For example, MONDAY. | DayOfWeek |
| segments_device | Device to which metrics apply. | Device |
| segments_geo_target_most_specific_location | Resource name of the geo target constant that represents the most specific location. | MostSpecificCriteriaId |
| segments_month | Month as represented by the date of the first day of a month. Formatted as yyyy-MM-dd. | Month |
| segments_quarter | Quarter as represented by the date of the first day of a quarter. Uses the calendar year for quarters. For example, the second quarter of 2018 starts on 2018-04-01. Formatted as yyyy-MM-dd. | Quarter |
| segments_week | Week as defined as Monday through Sunday, and represented by the date of Monday. Formatted as yyyy-MM-dd. | Week |
| segments_year | Year, formatted as yyyy. | Year |

Google Ads Table Name: HourlyAccountConversionStats


Google Ads API Resource:
[customer](https://developers.google.com/google-ads/api/fields/v22/customer)

| Google Ads Field Name | Description | Adwords Mapped Field Name |
|---|---|---|
| customer_id | The ID of the customer. | ExternalCustomerId |
| metrics_conversions | The number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | Conversions |
| metrics_conversions_value | The total value of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | ConversionValue |
| metrics_value_per_conversion | The value of conversions divided by the number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | ValuePerConversion |
| segments_ad_network_type | Ad network type. | AdNetworkType2 |
| segments_click_type | Click type. | ClickType |
| segments_conversion_action | Resource name of the conversion action. | ConversionTrackerId |
| segments_conversion_action_category | Conversion action category. | ConversionCategoryName |
| segments_conversion_action_name | Conversion action name. | ConversionTypeName |
| segments_date | Date to which metrics apply. yyyy-MM-dd format. For example, 2018-04-17. | Date |
| segments_day_of_week | Day of the week. For example, MONDAY. | DayOfWeek |
| segments_device | Device to which metrics apply. | Device |
| segments_hour | Hour of day as a number between 0 and 23, inclusive. | HourOfDay |
| segments_month | Month as represented by the date of the first day of a month. Formatted as yyyy-MM-dd. | Month |
| segments_quarter | Quarter as represented by the date of the first day of a quarter. Uses the calendar year for quarters. For example, the second quarter of 2018 starts on 2018-04-01. Formatted as yyyy-MM-dd. | Quarter |
| segments_week | Week as defined as Monday through Sunday, and represented by the date of Monday. Formatted as yyyy-MM-dd. | Week |
| segments_year | Year, formatted as yyyy. | Year |

Google Ads Table Name: HourlyAccountStats


Google Ads API Resource:
[customer](https://developers.google.com/google-ads/api/fields/v22/customer)

| Google Ads Field Name | Description | Adwords Mapped Field Name |
|---|---|---|
| customer_id | The ID of the customer. | ExternalCustomerId |
| metrics_active_view_cpm | Average cost of viewable impressions (\`active_view_impressions\`). | ActiveViewCpm |
| metrics_active_view_ctr | Active view measurable clicks divided by active view viewable impressions. This metric is reported only for display network. | ActiveViewCtr |
| metrics_active_view_impressions | A measurement of how often your ad has become viewable on a Display Network site. | ActiveViewImpressions |
| metrics_active_view_measurability | The ratio of impressions that could be measured by Active View over the number of served impressions. | ActiveViewMeasurability |
| metrics_active_view_measurable_cost_micros | The cost of the impressions you received that were measurable by Active View. | ActiveViewMeasurableCost |
| metrics_active_view_measurable_impressions | The number of times your ads are appearing on placements in positions where they can be seen. | ActiveViewMeasurableImpressions |
| metrics_active_view_viewability | The percentage of time when your ad appeared on an Active View enabled site (measurable impressions) and was viewable (viewable impressions). | ActiveViewViewability |
| metrics_average_cost | The average amount you pay per interaction. This amount is the total cost of your ads divided by the total number of interactions. | AverageCost |
| metrics_average_cpc | The total cost of all clicks divided by the total number of clicks received. | AverageCpc |
| metrics_average_cpm | Average cost-per-thousand impressions (CPM). | AverageCpm |
| metrics_clicks | The number of clicks. | Clicks |
| metrics_conversions | The number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | Conversions |
| metrics_conversions_from_interactions_rate | Conversions from interactions divided by the number of ad interactions (such as clicks for text ads or views for video ads). This only includes conversion actions which include_in_conversions_metric attribute is set to true. | ConversionRate |
| metrics_conversions_value | The total value of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | ConversionValue |
| metrics_cost_micros | The sum of your cost-per-click (CPC) and cost-per-thousand impressions (CPM) costs during this period. | Cost |
| metrics_cost_per_conversion | The cost of ad interactions divided by conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | CostPerConversion |
| metrics_ctr | The number of clicks your ad receives (Clicks) divided by the number of times your ad is shown (Impressions). | Ctr |
| metrics_impressions | Count of how often your ad has appeared on a search results page or website on the Google Network. | Impressions |
| metrics_interaction_event_types | The types of payable and free interactions. | InteractionTypes |
| metrics_interaction_rate | How often people interact with your ad after it is shown to them. This is the number of interactions divided by the number of times your ad is shown. | InteractionRate |
| metrics_interactions | The number of interactions. An interaction is the main user action associated with an ad format, such as clicks for text and shopping ads or views for video ads. | Interactions |
| metrics_value_per_conversion | The value of conversions divided by the number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | ValuePerConversion |
| segments_ad_network_type | Ad network type. | AdNetworkType2 |
| segments_click_type | Click type. | ClickType |
| segments_date | Date to which metrics apply. yyyy-MM-dd format. For example, 2018-04-17. | Date |
| segments_day_of_week | Day of the week. For example, MONDAY. | DayOfWeek |
| segments_device | Device to which metrics apply. | Device |
| segments_hour | Hour of day as a number between 0 and 23, inclusive. | HourOfDay |
| segments_month | Month as represented by the date of the first day of a month. Formatted as yyyy-MM-dd. | Month |
| segments_quarter | Quarter as represented by the date of the first day of a quarter. Uses the calendar year for quarters. For example, the second quarter of 2018 starts on 2018-04-01. Formatted as yyyy-MM-dd. | Quarter |
| segments_week | Week as defined as Monday through Sunday, and represented by the date of Monday. Formatted as yyyy-MM-dd. | Week |
| segments_year | Year, formatted as yyyy. | Year |

Google Ads Table Name: HourlyAdGroupConversionStats


Google Ads API Resource:
[ad_group](https://developers.google.com/google-ads/api/fields/v22/ad_group)

| Google Ads Field Name | Description | Adwords Mapped Field Name |
|---|---|---|
| ad_group_base_ad_group | For draft or experiment ad groups, this field is the resource name of the base ad group from which this ad group was created. If a draft or experiment ad group does not have a base ad group, then this field is null. For base ad groups, this field equals the ad group resource name. This field is read-only. | BaseAdGroupId |
| ad_group_id | The ID of the ad group. | AdGroupId |
| campaign_base_campaign | The resource name of the base campaign of a draft or experiment campaign. For base campaigns, this is equal to \`resource_name\`. This field is read-only. | BaseCampaignId |
| campaign_id | The ID of the campaign. | CampaignId |
| customer_id | The ID of the customer. | ExternalCustomerId |
| metrics_conversions | The number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | Conversions |
| metrics_conversions_value | The total value of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | ConversionValue |
| metrics_value_per_conversion | The value of conversions divided by the number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | ValuePerConversion |
| segments_ad_network_type | Ad network type. | AdNetworkType2 |
| segments_click_type | Click type. | ClickType |
| segments_conversion_action | Resource name of the conversion action. | ConversionTrackerId |
| segments_conversion_action_category | Conversion action category. | ConversionCategoryName |
| segments_conversion_action_name | Conversion action name. | ConversionTypeName |
| segments_date | Date to which metrics apply. yyyy-MM-dd format. For example, 2018-04-17. | Date |
| segments_day_of_week | Day of the week. For example, MONDAY. | DayOfWeek |
| segments_device | Device to which metrics apply. | Device |
| segments_hour | Hour of day as a number between 0 and 23, inclusive. | HourOfDay |
| segments_month | Month as represented by the date of the first day of a month. Formatted as yyyy-MM-dd. | Month |
| segments_quarter | Quarter as represented by the date of the first day of a quarter. Uses the calendar year for quarters. For example, the second quarter of 2018 starts on 2018-04-01. Formatted as yyyy-MM-dd. | Quarter |
| segments_week | Week as defined as Monday through Sunday, and represented by the date of Monday. Formatted as yyyy-MM-dd. | Week |
| segments_year | Year, formatted as yyyy. | Year |

Google Ads Table Name: HourlyAdGroupStats


Google Ads API Resource:
[ad_group](https://developers.google.com/google-ads/api/fields/v22/ad_group)

| Google Ads Field Name | Description | Adwords Mapped Field Name |
|---|---|---|
| ad_group_base_ad_group | For draft or experiment ad groups, this field is the resource name of the base ad group from which this ad group was created. If a draft or experiment ad group does not have a base ad group, then this field is null. For base ad groups, this field equals the ad group resource name. This field is read-only. | BaseAdGroupId |
| ad_group_id | The ID of the ad group. | AdGroupId |
| campaign_base_campaign | The resource name of the base campaign of a draft or experiment campaign. For base campaigns, this is equal to \`resource_name\`. This field is read-only. | BaseCampaignId |
| campaign_id | The ID of the campaign. | CampaignId |
| customer_id | The ID of the customer. | ExternalCustomerId |
| metrics_active_view_cpm | Average cost of viewable impressions (\`active_view_impressions\`). | ActiveViewCpm |
| metrics_active_view_ctr | Active view measurable clicks divided by active view viewable impressions. This metric is reported only for display network. | ActiveViewCtr |
| metrics_active_view_impressions | A measurement of how often your ad has become viewable on a Display Network site. | ActiveViewImpressions |
| metrics_active_view_measurability | The ratio of impressions that could be measured by Active View over the number of served impressions. | ActiveViewMeasurability |
| metrics_active_view_measurable_cost_micros | The cost of the impressions you received that were measurable by Active View. | ActiveViewMeasurableCost |
| metrics_active_view_measurable_impressions | The number of times your ads are appearing on placements in positions where they can be seen. | ActiveViewMeasurableImpressions |
| metrics_active_view_viewability | The percentage of time when your ad appeared on an Active View enabled site (measurable impressions) and was viewable (viewable impressions). | ActiveViewViewability |
| metrics_average_cost | The average amount you pay per interaction. This amount is the total cost of your ads divided by the total number of interactions. | AverageCost |
| metrics_average_cpc | The total cost of all clicks divided by the total number of clicks received. | AverageCpc |
| metrics_average_cpm | Average cost-per-thousand impressions (CPM). | AverageCpm |
| metrics_clicks | The number of clicks. | Clicks |
| metrics_conversions | The number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | Conversions |
| metrics_conversions_from_interactions_rate | Conversions from interactions divided by the number of ad interactions (such as clicks for text ads or views for video ads). This only includes conversion actions which include_in_conversions_metric attribute is set to true. | ConversionRate |
| metrics_conversions_value | The total value of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | ConversionValue |
| metrics_cost_micros | The sum of your cost-per-click (CPC) and cost-per-thousand impressions (CPM) costs during this period. | Cost |
| metrics_cost_per_conversion | The cost of ad interactions divided by conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | CostPerConversion |
| metrics_ctr | The number of clicks your ad receives (Clicks) divided by the number of times your ad is shown (Impressions). | Ctr |
| metrics_impressions | Count of how often your ad has appeared on a search results page or website on the Google Network. | Impressions |
| metrics_interaction_event_types | The types of payable and free interactions. | InteractionTypes |
| metrics_interaction_rate | How often people interact with your ad after it is shown to them. This is the number of interactions divided by the number of times your ad is shown. | InteractionRate |
| metrics_interactions | The number of interactions. An interaction is the main user action associated with an ad format, such as clicks for text and shopping ads or views for video ads. | Interactions |
| metrics_value_per_conversion | The value of conversions divided by the number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | ValuePerConversion |
| segments_ad_network_type | Ad network type. | AdNetworkType2 |
| segments_click_type | Click type. | ClickType |
| segments_date | Date to which metrics apply. yyyy-MM-dd format. For example, 2018-04-17. | Date |
| segments_day_of_week | Day of the week. For example, MONDAY. | DayOfWeek |
| segments_device | Device to which metrics apply. | Device |
| segments_hour | Hour of day as a number between 0 and 23, inclusive. | HourOfDay |
| segments_month | Month as represented by the date of the first day of a month. Formatted as yyyy-MM-dd. | Month |
| segments_quarter | Quarter as represented by the date of the first day of a quarter. Uses the calendar year for quarters. For example, the second quarter of 2018 starts on 2018-04-01. Formatted as yyyy-MM-dd. | Quarter |
| segments_week | Week as defined as Monday through Sunday, and represented by the date of Monday. Formatted as yyyy-MM-dd. | Week |
| segments_year | Year, formatted as yyyy. | Year |

Google Ads Table Name: HourlyBidGoalStats


Google Ads API Resource:
[bidding_strategy](https://developers.google.com/google-ads/api/fields/v22/bidding_strategy)

| Google Ads Field Name | Description | Adwords Mapped Field Name |
|---|---|---|
| bidding_strategy_campaign_count | The number of campaigns attached to this bidding strategy. This field is read-only. | CampaignCount |
| bidding_strategy_id | The ID of the bidding strategy. | BidStrategyID |
| bidding_strategy_non_removed_campaign_count | The number of non-removed campaigns attached to this bidding strategy. This field is read-only. | NonRemovedCampaignCount |
| customer_id | The ID of the customer. | ExternalCustomerId |
| metrics_average_cpc | The total cost of all clicks divided by the total number of clicks received. | AverageCpc |
| metrics_average_cpm | Average cost-per-thousand impressions (CPM). | AverageCpm |
| metrics_clicks | The number of clicks. | Clicks |
| metrics_conversions | The number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | Conversions |
| metrics_conversions_from_interactions_rate | Conversions from interactions divided by the number of ad interactions (such as clicks for text ads or views for video ads). This only includes conversion actions which include_in_conversions_metric attribute is set to true. | ConversionRate |
| metrics_conversions_value | The total value of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | ConversionValue |
| metrics_cost_micros | The sum of your cost-per-click (CPC) and cost-per-thousand impressions (CPM) costs during this period. | Cost |
| metrics_cost_per_conversion | The cost of ad interactions divided by conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | CostPerConversion |
| metrics_ctr | The number of clicks your ad receives (Clicks) divided by the number of times your ad is shown (Impressions). | Ctr |
| metrics_impressions | Count of how often your ad has appeared on a search results page or website on the Google Network. | Impressions |
| metrics_value_per_conversion | The value of conversions divided by the number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | ValuePerConversion |
| segments_date | Date to which metrics apply. yyyy-MM-dd format. For example, 2018-04-17. | Date |
| segments_day_of_week | Day of the week. For example, MONDAY. | DayOfWeek |
| segments_device | Device to which metrics apply. | Device |
| segments_hour | Hour of day as a number between 0 and 23, inclusive. | HourOfDay |
| segments_month | Month as represented by the date of the first day of a month. Formatted as yyyy-MM-dd. | Month |
| segments_quarter | Quarter as represented by the date of the first day of a quarter. Uses the calendar year for quarters. For example, the second quarter of 2018 starts on 2018-04-01. Formatted as yyyy-MM-dd. | Quarter |
| segments_week | Week as defined as Monday through Sunday, and represented by the date of Monday. Formatted as yyyy-MM-dd. | Week |
| segments_year | Year, formatted as yyyy. | Year |

Google Ads Table Name: HourlyCampaignConversionStats


Google Ads API Resource:
[campaign](https://developers.google.com/google-ads/api/fields/v22/campaign)

| Google Ads Field Name | Description | Adwords Mapped Field Name |
|---|---|---|
| campaign_base_campaign | The resource name of the base campaign of a draft or experiment campaign. For base campaigns, this is equal to \`resource_name\`. This field is read-only. | BaseCampaignId |
| campaign_id | The ID of the campaign. | CampaignId |
| customer_id | The ID of the customer. | ExternalCustomerId |
| metrics_conversions | The number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | Conversions |
| metrics_conversions_value | The total value of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | ConversionValue |
| metrics_value_per_conversion | The value of conversions divided by the number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | ValuePerConversion |
| segments_ad_network_type | Ad network type. | AdNetworkType2 |
| segments_click_type | Click type. | ClickType |
| segments_conversion_action | Resource name of the conversion action. | ConversionTrackerId |
| segments_conversion_action_category | Conversion action category. | ConversionCategoryName |
| segments_conversion_action_name | Conversion action name. | ConversionTypeName |
| segments_date | Date to which metrics apply. yyyy-MM-dd format. For example, 2018-04-17. | Date |
| segments_day_of_week | Day of the week. For example, MONDAY. | DayOfWeek |
| segments_device | Device to which metrics apply. | Device |
| segments_hour | Hour of day as a number between 0 and 23, inclusive. | HourOfDay |
| segments_month | Month as represented by the date of the first day of a month. Formatted as yyyy-MM-dd. | Month |
| segments_quarter | Quarter as represented by the date of the first day of a quarter. Uses the calendar year for quarters. For example, the second quarter of 2018 starts on 2018-04-01. Formatted as yyyy-MM-dd. | Quarter |
| segments_week | Week as defined as Monday through Sunday, and represented by the date of Monday. Formatted as yyyy-MM-dd. | Week |
| segments_year | Year, formatted as yyyy. | Year |

Google Ads Table Name: HourlyCampaignStats


Google Ads API Resource:
[campaign](https://developers.google.com/google-ads/api/fields/v22/campaign)

| Google Ads Field Name | Description | Adwords Mapped Field Name |
|---|---|---|
| campaign_base_campaign | The resource name of the base campaign of a draft or experiment campaign. For base campaigns, this is equal to \`resource_name\`. This field is read-only. | BaseCampaignId |
| campaign_id | The ID of the campaign. | CampaignId |
| customer_id | The ID of the customer. | ExternalCustomerId |
| metrics_active_view_cpm | Average cost of viewable impressions (\`active_view_impressions\`). | ActiveViewCpm |
| metrics_active_view_ctr | Active view measurable clicks divided by active view viewable impressions. This metric is reported only for display network. | ActiveViewCtr |
| metrics_active_view_impressions | A measurement of how often your ad has become viewable on a Display Network site. | ActiveViewImpressions |
| metrics_active_view_measurability | The ratio of impressions that could be measured by Active View over the number of served impressions. | ActiveViewMeasurability |
| metrics_active_view_measurable_cost_micros | The cost of the impressions you received that were measurable by Active View. | ActiveViewMeasurableCost |
| metrics_active_view_measurable_impressions | The number of times your ads are appearing on placements in positions where they can be seen. | ActiveViewMeasurableImpressions |
| metrics_active_view_viewability | The percentage of time when your ad appeared on an Active View enabled site (measurable impressions) and was viewable (viewable impressions). | ActiveViewViewability |
| metrics_average_cost | The average amount you pay per interaction. This amount is the total cost of your ads divided by the total number of interactions. | AverageCost |
| metrics_average_cpc | The total cost of all clicks divided by the total number of clicks received. | AverageCpc |
| metrics_average_cpm | Average cost-per-thousand impressions (CPM). | AverageCpm |
| metrics_clicks | The number of clicks. | Clicks |
| metrics_conversions | The number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | Conversions |
| metrics_conversions_from_interactions_rate | Conversions from interactions divided by the number of ad interactions (such as clicks for text ads or views for video ads). This only includes conversion actions which include_in_conversions_metric attribute is set to true. | ConversionRate |
| metrics_conversions_value | The total value of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | ConversionValue |
| metrics_cost_micros | The sum of your cost-per-click (CPC) and cost-per-thousand impressions (CPM) costs during this period. | Cost |
| metrics_cost_per_conversion | The cost of ad interactions divided by conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | CostPerConversion |
| metrics_ctr | The number of clicks your ad receives (Clicks) divided by the number of times your ad is shown (Impressions). | Ctr |
| metrics_impressions | Count of how often your ad has appeared on a search results page or website on the Google Network. | Impressions |
| metrics_interaction_event_types | The types of payable and free interactions. | InteractionTypes |
| metrics_interaction_rate | How often people interact with your ad after it is shown to them. This is the number of interactions divided by the number of times your ad is shown. | InteractionRate |
| metrics_interactions | The number of interactions. An interaction is the main user action associated with an ad format, such as clicks for text and shopping ads or views for video ads. | Interactions |
| metrics_value_per_conversion | The value of conversions divided by the number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | ValuePerConversion |
| segments_ad_network_type | Ad network type. | AdNetworkType2 |
| segments_click_type | Click type. | ClickType |
| segments_date | Date to which metrics apply. yyyy-MM-dd format. For example, 2018-04-17. | Date |
| segments_day_of_week | Day of the week. For example, MONDAY. | DayOfWeek |
| segments_device | Device to which metrics apply. | Device |
| segments_hour | Hour of day as a number between 0 and 23, inclusive. | HourOfDay |
| segments_month | Month as represented by the date of the first day of a month. Formatted as yyyy-MM-dd. | Month |
| segments_quarter | Quarter as represented by the date of the first day of a quarter. Uses the calendar year for quarters. For example, the second quarter of 2018 starts on 2018-04-01. Formatted as yyyy-MM-dd. | Quarter |
| segments_week | Week as defined as Monday through Sunday, and represented by the date of Monday. Formatted as yyyy-MM-dd. | Week |
| segments_year | Year, formatted as yyyy. | Year |

Google Ads Table Name: Keyword


Google Ads API Resource:
[keyword_view](https://developers.google.com/google-ads/api/fields/v22/keyword_view)

| Google Ads Field Name | Description | Adwords Mapped Field Name |
|---|---|---|
| ad_group_criterion_approval_status | Approval status of the criterion. | ApprovalStatus |
| ad_group_criterion_criterion_id | The ID of the criterion. This field is ignored for mutates. | CriterionId |
| ad_group_criterion_effective_cpc_bid_micros | The effective CPC (cost-per-click) bid. | CpcBid |
| ad_group_criterion_effective_cpc_bid_source | Source of the effective CPC bid. | CpcBidSource |
| ad_group_criterion_effective_cpm_bid_micros | The effective CPM (cost-per-thousand viewable impressions) bid. | CpmBidStr |
| ad_group_criterion_final_mobile_urls | The list of possible final mobile URLs after all cross-domain redirects. | FinalMobileUrls |
| ad_group_criterion_final_url_suffix | URL template for appending params to final URL. | FinalUrlSuffix |
| ad_group_criterion_final_urls | The list of possible final URLs after all cross-domain redirects for the ad. | FinalUrls |
| ad_group_criterion_keyword_match_type | The match type of the keyword. | KeywordMatchType |
| ad_group_criterion_keyword_text | The text of the keyword (at most 80 characters and 10 words). | Criteria |
| ad_group_criterion_negative | Whether to target (\`false\`) or exclude (\`true\`) the criterion. This field is immutable. To switch a criterion from positive to negative, remove then re-add it. | IsNegative |
| ad_group_criterion_position_estimates_estimated_add_clicks_at_first_position_cpc | Estimate of how many clicks per week you might get by changing your keyword bid to the value in first_position_cpc_micros. | EstimatedAddClicksAtFirstPositionCpc |
| ad_group_criterion_position_estimates_estimated_add_cost_at_first_position_cpc | Estimate of how your cost per week might change when changing your keyword bid to the value in first_position_cpc_micros. | EstimatedAddCostAtFirstPositionCpc |
| ad_group_criterion_position_estimates_first_page_cpc_micros | The estimate of the CPC bid required for ad to be shown on first page of search results. | FirstPageCpc |
| ad_group_criterion_position_estimates_first_position_cpc_micros | The estimate of the CPC bid required for ad to be displayed in first position, at the top of the first page of search results. | FirstPositionCpc |
| ad_group_criterion_position_estimates_top_of_page_cpc_micros | The estimate of the CPC bid required for ad to be displayed at the top of the first page of search results. | TopOfPageCpc |
| ad_group_criterion_quality_info_creative_quality_score | The performance of the ad compared to other advertisers. | CreativeQualityScore |
| ad_group_criterion_quality_info_post_click_quality_score | The quality score of the landing page. | PostClickQualityScore |
| ad_group_criterion_quality_info_quality_score | The quality score. This field may not be populated if Google does not have enough information to determine a value. | QualityScore |
| ad_group_criterion_quality_info_search_predicted_ctr | The click-through rate compared to that of other advertisers. | SearchPredictedCtr |
| ad_group_criterion_status | The status of the criterion. | Status |
| ad_group_criterion_system_serving_status | Serving status of the criterion. | SystemServingStatus |
| ad_group_criterion_topic_topic_constant | The Topic Constant resource name. | VerticalId |
| ad_group_criterion_tracking_url_template | The URL template for constructing a tracking URL. | TrackingUrlTemplate |
| ad_group_criterion_url_custom_parameters | The list of mappings used to substitute custom parameter tags in a \`tracking_url_template\`, \`final_urls\`, or \`mobile_final_urls\`. | UrlCustomParameters |
| ad_group_id | The ID of the ad group. | AdGroupId |
| campaign_bidding_strategy | Portfolio bidding strategy used by campaign. | BiddingStrategyId |
| campaign_bidding_strategy_type | The type of bidding strategy. A bidding strategy can be created by setting either the bidding scheme to create a standard bidding strategy or the \`bidding_strategy\` field to create a portfolio bidding strategy. This field is read-only. | BiddingStrategyType |
| campaign_id | The ID of the campaign. | CampaignId |
| campaign_manual_cpc_enhanced_cpc_enabled | Whether bids are to be enhanced based on conversion optimizer data. | EnhancedCpcEnabled |
| campaign_percent_cpc_enhanced_cpc_enabled | Adjusts the bid for each auction upward or downward, depending on the likelihood of a conversion. Individual bids may exceed cpc_bid_ceiling_micros, but the average bid amount for a campaign shouldn't. |   |
| customer_id | The ID of the customer. | ExternalCustomerId |

Google Ads Table Name: KeywordBasicStats


Google Ads API Resource:
[keyword_view](https://developers.google.com/google-ads/api/fields/v22/keyword_view)

| Google Ads Field Name | Description | Adwords Mapped Field Name |
|---|---|---|
| ad_group_base_ad_group | For draft or experiment ad groups, this field is the resource name of the base ad group from which this ad group was created. If a draft or experiment ad group does not have a base ad group, then this field is null. For base ad groups, this field equals the ad group resource name. This field is read-only. | BaseAdGroupId |
| ad_group_criterion_criterion_id | The ID of the criterion. This field is ignored for mutates. | CriterionId |
| ad_group_id | The ID of the ad group. | AdGroupId |
| campaign_base_campaign | The resource name of the base campaign of a draft or experiment campaign. For base campaigns, this is equal to \`resource_name\`. This field is read-only. | BaseCampaignId |
| campaign_id | The ID of the campaign. | CampaignId |
| customer_id | The ID of the customer. | ExternalCustomerId |
| metrics_clicks | The number of clicks. | Clicks |
| metrics_conversions | The number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | Conversions |
| metrics_conversions_value | The total value of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | ConversionValue |
| metrics_cost_micros | The sum of your cost-per-click (CPC) and cost-per-thousand impressions (CPM) costs during this period. | Cost |
| metrics_impressions | Count of how often your ad has appeared on a search results page or website on the Google Network. | Impressions |
| metrics_interaction_event_types | The types of payable and free interactions. | InteractionTypes |
| metrics_interactions | The number of interactions. An interaction is the main user action associated with an ad format, such as clicks for text and shopping ads or views for video ads. | Interactions |
| metrics_view_through_conversions | The total number of view-through conversions. These happen when a customer sees an image or rich media ad, then later completes a conversion on your site without interacting with (for example, clicking on) another ad. | ViewThroughConversions |
| segments_ad_network_type | Ad network type. | AdNetworkType2 |
| segments_date | Date to which metrics apply. yyyy-MM-dd format. For example, 2018-04-17. | Date |
| segments_device | Device to which metrics apply. | Device |
| segments_slot | Position of the ad. | Slot |

Google Ads Table Name: KeywordConversionStats


Google Ads API Resource:
[keyword_view](https://developers.google.com/google-ads/api/fields/v22/keyword_view)

| Google Ads Field Name | Description | Adwords Mapped Field Name |
|---|---|---|
| ad_group_base_ad_group | For draft or experiment ad groups, this field is the resource name of the base ad group from which this ad group was created. If a draft or experiment ad group does not have a base ad group, then this field is null. For base ad groups, this field equals the ad group resource name. This field is read-only. | BaseAdGroupId |
| ad_group_criterion_criterion_id | The ID of the criterion. This field is ignored for mutates. | CriterionId |
| ad_group_id | The ID of the ad group. | AdGroupId |
| campaign_base_campaign | The resource name of the base campaign of a draft or experiment campaign. For base campaigns, this is equal to \`resource_name\`. This field is read-only. | BaseCampaignId |
| campaign_id | The ID of the campaign. | CampaignId |
| customer_id | The ID of the customer. | ExternalCustomerId |
| metrics_conversions | The number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | Conversions |
| metrics_conversions_value | The total value of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | ConversionValue |
| metrics_value_per_conversion | The value of conversions divided by the number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | ValuePerConversion |
| segments_ad_network_type | Ad network type. | AdNetworkType2 |
| segments_click_type | Click type. | ClickType |
| segments_conversion_action | Resource name of the conversion action. | ConversionTrackerId |
| segments_conversion_action_category | Conversion action category. | ConversionCategoryName |
| segments_conversion_action_name | Conversion action name. | ConversionTypeName |
| segments_date | Date to which metrics apply. yyyy-MM-dd format. For example, 2018-04-17. | Date |
| segments_day_of_week | Day of the week. For example, MONDAY. | DayOfWeek |
| segments_device | Device to which metrics apply. | Device |
| segments_month | Month as represented by the date of the first day of a month. Formatted as yyyy-MM-dd. | Month |
| segments_quarter | Quarter as represented by the date of the first day of a quarter. Uses the calendar year for quarters. For example, the second quarter of 2018 starts on 2018-04-01. Formatted as yyyy-MM-dd. | Quarter |
| segments_slot | Position of the ad. | Slot |
| segments_week | Week as defined as Monday through Sunday, and represented by the date of Monday. Formatted as yyyy-MM-dd. | Week |
| segments_year | Year, formatted as yyyy. | Year |

Google Ads Table Name: KeywordCrossDeviceConversionStats


Google Ads API Resource:
[keyword_view](https://developers.google.com/google-ads/api/fields/v22/keyword_view)

| Google Ads Field Name | Description | Adwords Mapped Field Name |
|---|---|---|
| ad_group_base_ad_group | For draft or experiment ad groups, this field is the resource name of the base ad group from which this ad group was created. If a draft or experiment ad group does not have a base ad group, then this field is null. For base ad groups, this field equals the ad group resource name. This field is read-only. | BaseAdGroupId |
| ad_group_criterion_criterion_id | The ID of the criterion. This field is ignored for mutates. | CriterionId |
| ad_group_id | The ID of the ad group. | AdGroupId |
| campaign_base_campaign | The resource name of the base campaign of a draft or experiment campaign. For base campaigns, this is equal to \`resource_name\`. This field is read-only. | BaseCampaignId |
| campaign_id | The ID of the campaign. | CampaignId |
| customer_id | The ID of the customer. | ExternalCustomerId |
| metrics_all_conversions | The total number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | AllConversions |
| metrics_all_conversions_value | The total value of all conversions. | AllConversionValue |
| metrics_cross_device_conversions | Conversions from when a customer clicks on a Google Ads ad on one device, then converts on a different device or browser. Cross-device conversions are already included in all_conversions. | CrossDeviceConversions |
| metrics_value_per_all_conversions | The value of all conversions divided by the number of all conversions. | ValuePerAllConversion |
| segments_ad_network_type | Ad network type. | AdNetworkType2 |
| segments_conversion_action | Resource name of the conversion action. | ConversionTrackerId |
| segments_conversion_action_category | Conversion action category. | ConversionCategoryName |
| segments_conversion_action_name | Conversion action name. | ConversionTypeName |
| segments_date | Date to which metrics apply. yyyy-MM-dd format. For example, 2018-04-17. | Date |
| segments_day_of_week | Day of the week. For example, MONDAY. | DayOfWeek |
| segments_month | Month as represented by the date of the first day of a month. Formatted as yyyy-MM-dd. | Month |
| segments_quarter | Quarter as represented by the date of the first day of a quarter. Uses the calendar year for quarters. For example, the second quarter of 2018 starts on 2018-04-01. Formatted as yyyy-MM-dd. | Quarter |
| segments_week | Week as defined as Monday through Sunday, and represented by the date of Monday. Formatted as yyyy-MM-dd. | Week |
| segments_year | Year, formatted as yyyy. | Year |

Google Ads Table Name: KeywordCrossDeviceStats


Google Ads API Resource:
[keyword_view](https://developers.google.com/google-ads/api/fields/v22/keyword_view)

| Google Ads Field Name | Description | Adwords Mapped Field Name |
|---|---|---|
| ad_group_base_ad_group | For draft or experiment ad groups, this field is the resource name of the base ad group from which this ad group was created. If a draft or experiment ad group does not have a base ad group, then this field is null. For base ad groups, this field equals the ad group resource name. This field is read-only. | BaseAdGroupId |
| ad_group_criterion_criterion_id | The ID of the criterion. This field is ignored for mutates. | CriterionId |
| ad_group_id | The ID of the ad group. | AdGroupId |
| campaign_base_campaign | The resource name of the base campaign of a draft or experiment campaign. For base campaigns, this is equal to \`resource_name\`. This field is read-only. | BaseCampaignId |
| campaign_id | The ID of the campaign. | CampaignId |
| customer_id | The ID of the customer. | ExternalCustomerId |
| metrics_absolute_top_impression_percentage | The percent of your ad impressions that are shown as the very first ad above the organic search results. | AbsoluteTopImpressionPercentage |
| metrics_all_conversions | The total number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | AllConversions |
| metrics_all_conversions_from_interactions_rate | All conversions from interactions (as oppose to view through conversions) divided by the number of ad interactions. | AllConversionRate |
| metrics_all_conversions_value | The total value of all conversions. | AllConversionValue |
| metrics_average_cpe | The average amount that you've been charged for an ad engagement. This amount is the total cost of all ad engagements divided by the total number of ad engagements. | AverageCpe |
| metrics_trueview_average_cpv | The average amount you pay each time someone views your ad. The average CPV is defined by the total cost of all ad views divided by the number of views. | AverageCpv |
| metrics_average_page_views | Average number of pages viewed per session. | AveragePageviews |
| metrics_average_time_on_site | Total duration of all sessions (in seconds) / number of sessions. Imported from Google Analytics. | AverageTimeOnSite |
| metrics_bounce_rate | Percentage of clicks where the user only visited a single page on your site. Imported from Google Analytics. | BounceRate |
| metrics_cost_per_all_conversions | The cost of ad interactions divided by all conversions. | CostPerAllConversion |
| metrics_cross_device_conversions | Conversions from when a customer clicks on a Google Ads ad on one device, then converts on a different device or browser. Cross-device conversions are already included in all_conversions. | CrossDeviceConversions |
| metrics_engagement_rate | How often people engage with your ad after it's shown to them. This is the number of ad expansions divided by the number of times your ad is shown. | EngagementRate |
| metrics_engagements | The number of engagements. An engagement occurs when a viewer expands your Lightbox ad. Also, in the future, other ad types may support engagement metrics. | Engagements |
| metrics_percent_new_visitors | Percentage of first-time sessions (from people who had never visited your site before). Imported from Google Analytics. | PercentNewVisitors |
| metrics_search_absolute_top_impression_share | The percentage of the customer's Shopping or Search ad impressions that are shown in the most prominent Shopping position. See https://support.google.com/google-ads/answer/7501826 for details. Any value below 0.1 is reported as 0.0999. | SearchAbsoluteTopImpressionShare |
| metrics_search_budget_lost_absolute_top_impression_share | The number estimating how often your ad wasn't the very first ad above the organic search results due to a low budget. Note: Search budget lost absolute top impression share is reported in the range of 0 to 0.9. Any value above 0.9 is reported as 0.9001. | SearchBudgetLostAbsoluteTopImpressionShare |
| metrics_search_budget_lost_top_impression_share | The number estimating how often your ad didn't show anywhere above the organic search results due to a low budget. Note: Search budget lost top impression share is reported in the range of 0 to 0.9. Any value above 0.9 is reported as 0.9001. | SearchBudgetLostTopImpressionShare |
| metrics_search_exact_match_impression_share | The impressions you've received divided by the estimated number of impressions you were eligible to receive on the Search Network for search terms that matched your keywords exactly (or were close variants of your keyword), regardless of your keyword match types. Note: Search exact match impression share is reported in the range of 0.1 to 1. Any value below 0.1 is reported as 0.0999. | SearchExactMatchImpressionShare |
| metrics_search_impression_share | The impressions you've received on the Search Network divided by the estimated number of impressions you were eligible to receive. Note: Search impression share is reported in the range of 0.1 to 1. Any value below 0.1 is reported as 0.0999. | SearchImpressionShare |
| metrics_search_rank_lost_absolute_top_impression_share | The number estimating how often your ad wasn't the very first ad above the organic search results due to poor Ad Rank. Note: Search rank lost absolute top impression share is reported in the range of 0 to 0.9. Any value above 0.9 is reported as 0.9001. | SearchRankLostAbsoluteTopImpressionShare |
| metrics_search_rank_lost_impression_share | The estimated percentage of impressions on the Search Network that your ads didn't receive due to poor Ad Rank. Note: Search rank lost impression share is reported in the range of 0 to 0.9. Any value above 0.9 is reported as 0.9001. | SearchRankLostImpressionShare |
| metrics_search_rank_lost_top_impression_share | The number estimating how often your ad didn't show anywhere above the organic search results due to poor Ad Rank. Note: Search rank lost top impression share is reported in the range of 0 to 0.9. Any value above 0.9 is reported as 0.9001. | SearchRankLostTopImpressionShare |
| metrics_search_top_impression_share | The impressions you've received in the top location (anywhere above the organic search results) compared to the estimated number of impressions you were eligible to receive in the top location. Note: Search top impression share is reported in the range of 0.1 to 1. Any value below 0.1 is reported as 0.0999. | SearchTopImpressionShare |
| metrics_top_impression_percentage | The percent of your ad impressions that are shown anywhere above the organic search results. | TopImpressionPercentage |
| metrics_value_per_all_conversions | The value of all conversions divided by the number of all conversions. | ValuePerAllConversion |
| metrics_video_quartile_p100_rate | Percentage of impressions where the viewer watched all of your video. | VideoQuartile100Rate |
| metrics_video_quartile_p25_rate | Percentage of impressions where the viewer watched 25% of your video. | VideoQuartile25Rate |
| metrics_video_quartile_p50_rate | Percentage of impressions where the viewer watched 50% of your video. | VideoQuartile50Rate |
| metrics_video_quartile_p75_rate | Percentage of impressions where the viewer watched 75% of your video. | VideoQuartile75Rate |
| metrics_video_trueview_view_rate | The number of views your TrueView video ad receives divided by its number of impressions, including thumbnail impressions for TrueView in-display ads. | VideoViewRate |
| metrics_video_trueview_views | The number of times your video ads were viewed. | VideoViews |
| segments_ad_network_type | Ad network type. | AdNetworkType2 |
| segments_date | Date to which metrics apply. yyyy-MM-dd format. For example, 2018-04-17. | Date |
| segments_day_of_week | Day of the week. For example, MONDAY. | DayOfWeek |
| segments_month | Month as represented by the date of the first day of a month. Formatted as yyyy-MM-dd. | Month |
| segments_quarter | Quarter as represented by the date of the first day of a quarter. Uses the calendar year for quarters. For example, the second quarter of 2018 starts on 2018-04-01. Formatted as yyyy-MM-dd. | Quarter |
| segments_week | Week as defined as Monday through Sunday, and represented by the date of Monday. Formatted as yyyy-MM-dd. | Week |
| segments_year | Year, formatted as yyyy. | Year |

Google Ads Table Name: KeywordStats


Google Ads API Resource:
[keyword_view](https://developers.google.com/google-ads/api/fields/v22/keyword_view)

| Google Ads Field Name | Description | Adwords Mapped Field Name |
|---|---|---|
| ad_group_base_ad_group | For draft or experiment ad groups, this field is the resource name of the base ad group from which this ad group was created. If a draft or experiment ad group does not have a base ad group, then this field is null. For base ad groups, this field equals the ad group resource name. This field is read-only. | BaseAdGroupId |
| ad_group_criterion_criterion_id | The ID of the criterion. This field is ignored for mutates. | CriterionId |
| ad_group_id | The ID of the ad group. | AdGroupId |
| campaign_base_campaign | The resource name of the base campaign of a draft or experiment campaign. For base campaigns, this is equal to \`resource_name\`. This field is read-only. | BaseCampaignId |
| campaign_id | The ID of the campaign. | CampaignId |
| customer_id | The ID of the customer. | ExternalCustomerId |
| metrics_active_view_cpm | Average cost of viewable impressions (\`active_view_impressions\`). | ActiveViewCpm |
| metrics_active_view_ctr | Active view measurable clicks divided by active view viewable impressions. This metric is reported only for display network. | ActiveViewCtr |
| metrics_active_view_impressions | A measurement of how often your ad has become viewable on a Display Network site. | ActiveViewImpressions |
| metrics_active_view_measurability | The ratio of impressions that could be measured by Active View over the number of served impressions. | ActiveViewMeasurability |
| metrics_active_view_measurable_cost_micros | The cost of the impressions you received that were measurable by Active View. | ActiveViewMeasurableCost |
| metrics_active_view_measurable_impressions | The number of times your ads are appearing on placements in positions where they can be seen. | ActiveViewMeasurableImpressions |
| metrics_active_view_viewability | The percentage of time when your ad appeared on an Active View enabled site (measurable impressions) and was viewable (viewable impressions). | ActiveViewViewability |
| metrics_average_cost | The average amount you pay per interaction. This amount is the total cost of your ads divided by the total number of interactions. | AverageCost |
| metrics_average_cpc | The total cost of all clicks divided by the total number of clicks received. | AverageCpc |
| metrics_average_cpm | Average cost-per-thousand impressions (CPM). | AverageCpm |
| metrics_clicks | The number of clicks. | Clicks |
| metrics_conversions | The number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | Conversions |
| metrics_conversions_from_interactions_rate | Conversions from interactions divided by the number of ad interactions (such as clicks for text ads or views for video ads). This only includes conversion actions which include_in_conversions_metric attribute is set to true. | ConversionRate |
| metrics_conversions_value | The total value of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | ConversionValue |
| metrics_cost_micros | The sum of your cost-per-click (CPC) and cost-per-thousand impressions (CPM) costs during this period. | Cost |
| metrics_cost_per_conversion | The cost of ad interactions divided by conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | CostPerConversion |
| metrics_cost_per_current_model_attributed_conversion | The cost of ad interactions divided by current model attributed conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | CostPerCurrentModelAttributedConversion |
| metrics_ctr | The number of clicks your ad receives (Clicks) divided by the number of times your ad is shown (Impressions). | Ctr |
| metrics_current_model_attributed_conversions | Shows how your historic conversions data would look under the attribution model you've selected. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | CurrentModelAttributedConversions |
| metrics_current_model_attributed_conversions_value | The total value of current model attributed conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | CurrentModelAttributedConversionValue |
| metrics_gmail_forwards | The number of times the ad was forwarded to someone else as a message. | GmailForwards |
| metrics_gmail_saves | The number of times someone has saved your Gmail ad to their inbox as a message. | GmailSaves |
| metrics_gmail_secondary_clicks | The number of clicks to the landing page on the expanded state of Gmail ads. | GmailSecondaryClicks |
| metrics_impressions | Count of how often your ad has appeared on a search results page or website on the Google Network. | Impressions |
| metrics_interaction_event_types | The types of payable and free interactions. | InteractionTypes |
| metrics_interaction_rate | How often people interact with your ad after it is shown to them. This is the number of interactions divided by the number of times your ad is shown. | InteractionRate |
| metrics_interactions | The number of interactions. An interaction is the main user action associated with an ad format, such as clicks for text and shopping ads or views for video ads. | Interactions |
| metrics_value_per_conversion | The value of conversions divided by the number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | ValuePerConversion |
| metrics_value_per_current_model_attributed_conversion | The value of current model attributed conversions divided by the number of the conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | ValuePerCurrentModelAttributedConversion |
| segments_ad_network_type | Ad network type. | AdNetworkType2 |
| segments_click_type | Click type. | ClickType |
| segments_date | Date to which metrics apply. yyyy-MM-dd format. For example, 2018-04-17. | Date |
| segments_day_of_week | Day of the week. For example, MONDAY. | DayOfWeek |
| segments_device | Device to which metrics apply. | Device |
| segments_month | Month as represented by the date of the first day of a month. Formatted as yyyy-MM-dd. | Month |
| segments_quarter | Quarter as represented by the date of the first day of a quarter. Uses the calendar year for quarters. For example, the second quarter of 2018 starts on 2018-04-01. Formatted as yyyy-MM-dd. | Quarter |
| segments_week | Week as defined as Monday through Sunday, and represented by the date of Monday. Formatted as yyyy-MM-dd. | Week |
| segments_year | Year, formatted as yyyy. | Year |

Google Ads Table Name: LandingPageStats


Google Ads API Resource:
[landing_page_view](https://developers.google.com/google-ads/api/fields/v22/landing_page_view)

| Google Ads Field Name | Description | Adwords Mapped Field Name |
|---|---|---|
| ad_group_name | The name of the ad group. This field is required and shouldn't be empty when creating new ad groups. It must contain fewer than 255 UTF-8 full-width characters. It must not contain any null (code point 0x0), NL line feed (code point 0xA) or carriage return (code point 0xD) characters. |   |
| ad_group_status | The status of the ad group. |   |
| campaign_name | The name of the campaign. This field is required and shouldn't be empty when creating new campaigns. It must not contain any null (code point 0x0), NL line feed (code point 0xA) or carriage return (code point 0xD) characters. |   |
| customer_id | The ID of the customer. |   |
| landing_page_view_unexpanded_final_url | The advertiser-specified final URL. |   |
| metrics_average_cpc | The total cost of all clicks divided by the total number of clicks received. |   |
| metrics_clicks | The number of clicks. |   |
| metrics_cost_micros | The sum of your cost-per-click (CPC) and cost-per-thousand impressions (CPM) costs during this period. |   |
| metrics_ctr | The number of clicks your ad receives (Clicks) divided by the number of times your ad is shown (Impressions). |   |
| metrics_impressions | Count of how often your ad has appeared on a search results page or website on the Google Network. |   |
| metrics_mobile_friendly_clicks_percentage | The percentage of mobile clicks that go to a mobile-friendly page. |   |
| metrics_speed_score | A measure of how quickly your page loads after clicks on your mobile ads. The score is a range from 1 to 10, 10 being the fastest. |   |
| metrics_valid_accelerated_mobile_pages_clicks_percentage | The percentage of ad clicks to Accelerated Mobile Pages (AMP) landing pages that reach a valid AMP page. |   |
| segments_date | Date to which metrics apply. yyyy-MM-dd format. For example, 2018-04-17. |   |
| segments_day_of_week | Day of the week. For example, MONDAY. |   |
| segments_month | Month as represented by the date of the first day of a month. Formatted as yyyy-MM-dd. |   |
| segments_quarter | Quarter as represented by the date of the first day of a quarter. Uses the calendar year for quarters. For example, the second quarter of 2018 starts on 2018-04-01. Formatted as yyyy-MM-dd. |   |
| segments_week | Week as defined as Monday through Sunday, and represented by the date of Monday. Formatted as yyyy-MM-dd. |   |
| segments_year | Year, formatted as yyyy. |   |

Google Ads Table Name: LocationBasedCampaignCriterion


Google Ads API Resource:
[location_view](https://developers.google.com/google-ads/api/fields/v22/location_view)

| Google Ads Field Name | Description | Adwords Mapped Field Name |
|---|---|---|
| campaign_criterion_bid_modifier | The modifier for the bids when the criterion matches. The modifier must be in the range: 0.1 - 10.0. Most targetable criteria types support modifiers. Use 0 to opt out of a Device type. | BidModifier |
| campaign_criterion_criterion_id | The ID of the criterion. This field is ignored during mutate. | CriterionId |
| campaign_criterion_negative | Whether to target (\`false\`) or exclude (\`true\`) the criterion. | IsNegative |
| campaign_id | The ID of the campaign. | CampaignId |
| customer_id | The ID of the customer. | ExternalCustomerId |

Google Ads Table Name: LocationsDistanceStats


Google Ads API Resource:
[distance_view](https://developers.google.com/google-ads/api/fields/v22/distance_view)

| Google Ads Field Name | Description | Adwords Mapped Field Name |
|---|---|---|
| customer_id | The ID of the customer. |   |
| distance_view_distance_bucket | Grouping of user distance from location extensions. |   |
| metrics_average_cpc | The total cost of all clicks divided by the total number of clicks received. |   |
| metrics_clicks | The number of clicks. |   |
| metrics_conversions | The number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. |   |
| metrics_conversions_from_interactions_rate | Conversions from interactions divided by the number of ad interactions (such as clicks for text ads or views for video ads). This only includes conversion actions which include_in_conversions_metric attribute is set to true. |   |
| metrics_cost_micros | The sum of your cost-per-click (CPC) and cost-per-thousand impressions (CPM) costs during this period. |   |
| metrics_cost_per_conversion | The cost of ad interactions divided by conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. |   |
| metrics_ctr | The number of clicks your ad receives (Clicks) divided by the number of times your ad is shown (Impressions). |   |
| metrics_impressions | Count of how often your ad has appeared on a search results page or website on the Google Network. |   |
| metrics_view_through_conversions | The total number of view-through conversions. These happen when a customer sees an image or rich media ad, then later completes a conversion on your site without interacting with (for example, clicking on) another ad. |   |
| segments_date | Date to which metrics apply. yyyy-MM-dd format. For example, 2018-04-17. |   |
| segments_day_of_week | Day of the week. For example, MONDAY. |   |
| segments_month | Month as represented by the date of the first day of a month. Formatted as yyyy-MM-dd. |   |
| segments_quarter | Quarter as represented by the date of the first day of a quarter. Uses the calendar year for quarters. For example, the second quarter of 2018 starts on 2018-04-01. Formatted as yyyy-MM-dd. |   |
| segments_week | Week as defined as Monday through Sunday, and represented by the date of Monday. Formatted as yyyy-MM-dd. |   |
| segments_year | Year, formatted as yyyy. |   |

Google Ads Table Name: LocationsUserLocationsStats


Google Ads API Resource:
[user_location_view](https://developers.google.com/google-ads/api/fields/v22/user_location_view)

| Google Ads Field Name | Description | Adwords Mapped Field Name |
|---|---|---|
| customer_id | The ID of the customer. |   |
| metrics_average_cpc | The total cost of all clicks divided by the total number of clicks received. |   |
| metrics_clicks | The number of clicks. |   |
| metrics_conversions | The number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. |   |
| metrics_conversions_from_interactions_rate | Conversions from interactions divided by the number of ad interactions (such as clicks for text ads or views for video ads). This only includes conversion actions which include_in_conversions_metric attribute is set to true. |   |
| metrics_cost_micros | The sum of your cost-per-click (CPC) and cost-per-thousand impressions (CPM) costs during this period. |   |
| metrics_cost_per_conversion | The cost of ad interactions divided by conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. |   |
| metrics_ctr | The number of clicks your ad receives (Clicks) divided by the number of times your ad is shown (Impressions). |   |
| metrics_impressions | Count of how often your ad has appeared on a search results page or website on the Google Network. |   |
| metrics_view_through_conversions | The total number of view-through conversions. These happen when a customer sees an image or rich media ad, then later completes a conversion on your site without interacting with (for example, clicking on) another ad. |   |
| segments_date | Date to which metrics apply. yyyy-MM-dd format. For example, 2018-04-17. |   |
| segments_day_of_week | Day of the week. For example, MONDAY. |   |
| segments_geo_target_city | Resource name of the geo target constant that represents a city. |   |
| segments_geo_target_most_specific_location | Resource name of the geo target constant that represents the most specific location. |   |
| segments_geo_target_region | Resource name of the geo target constant that represents a region. |   |
| segments_month | Month as represented by the date of the first day of a month. Formatted as yyyy-MM-dd. |   |
| segments_quarter | Quarter as represented by the date of the first day of a quarter. Uses the calendar year for quarters. For example, the second quarter of 2018 starts on 2018-04-01. Formatted as yyyy-MM-dd. |   |
| segments_week | Week as defined as Monday through Sunday, and represented by the date of Monday. Formatted as yyyy-MM-dd. |   |
| segments_year | Year, formatted as yyyy. |   |
| user_location_view_country_criterion_id | Criterion Id for the country. |   |
| user_location_view_targeting_location | Target location. |   |

Google Ads Table Name: PaidOrganicStats


Google Ads API Resource:
[paid_organic_search_term_view](https://developers.google.com/google-ads/api/fields/v22/paid_organic_search_term_view)

| Google Ads Field Name | Description | Adwords Mapped Field Name |
|---|---|---|
| ad_group_id | The ID of the ad group. | AdGroupId |
| campaign_id | The ID of the campaign. | CampaignId |
| customer_id | The ID of the customer. | ExternalCustomerId |
| metrics_average_cpc | The total cost of all clicks divided by the total number of clicks received. | AverageCpc |
| metrics_clicks | The number of clicks. | Clicks |
| metrics_combined_clicks | The number of times your ad or your site's listing in the unpaid results was clicked. See the help page at https://support.google.com/google-ads/answer/3097241 for details. | CombinedAdsOrganicClicks |
| metrics_combined_clicks_per_query | The number of times your ad or your site's listing in the unpaid results was clicked (combined_clicks) divided by combined_queries. See the help page at https://support.google.com/google-ads/answer/3097241 for details. | CombinedAdsOrganicClicksPerQuery |
| metrics_combined_queries | The number of searches that returned pages from your site in the unpaid results or showed one of your text ads. See the help page at https://support.google.com/google-ads/answer/3097241 for details. | CombinedAdsOrganicQueries |
| metrics_ctr | The number of clicks your ad receives (Clicks) divided by the number of times your ad is shown (Impressions). | Ctr |
| metrics_impressions | Count of how often your ad has appeared on a search results page or website on the Google Network. | Impressions |
| metrics_organic_clicks | The number of times someone clicked your site's listing in the unpaid results for a particular query. See the help page at https://support.google.com/google-ads/answer/3097241 for details. | OrganicClicks |
| metrics_organic_clicks_per_query | The number of times someone clicked your site's listing in the unpaid results (organic_clicks) divided by the total number of searches that returned pages from your site (organic_queries). See the help page at https://support.google.com/google-ads/answer/3097241 for details. | OrganicClicksPerQuery |
| metrics_organic_impressions | The number of listings for your site in the unpaid search results. See the help page at https://support.google.com/google-ads/answer/3097241 for details. | OrganicImpressions |
| metrics_organic_impressions_per_query | The number of times a page from your site was listed in the unpaid search results (organic_impressions) divided by the number of searches returning your site's listing in the unpaid results (organic_queries). See the help page at https://support.google.com/google-ads/answer/3097241 for details. | OrganicImpressionsPerQuery |
| metrics_organic_queries | The total number of searches that returned your site's listing in the unpaid results. See the help page at https://support.google.com/google-ads/answer/3097241 for details. | OrganicQueries |
| paid_organic_search_term_view_search_term | The search term. | SearchQuery |
| segments_date | Date to which metrics apply. yyyy-MM-dd format. For example, 2018-04-17. | Date |
| segments_day_of_week | Day of the week. For example, MONDAY. | DayOfWeek |
| segments_month | Month as represented by the date of the first day of a month. Formatted as yyyy-MM-dd. | Month |
| segments_quarter | Quarter as represented by the date of the first day of a quarter. Uses the calendar year for quarters. For example, the second quarter of 2018 starts on 2018-04-01. Formatted as yyyy-MM-dd. | Quarter |
| segments_search_engine_results_page_type | Type of the search engine results page. | SerpType |
| segments_week | Week as defined as Monday through Sunday, and represented by the date of Monday. Formatted as yyyy-MM-dd. | Week |
| segments_year | Year, formatted as yyyy. | Year |

Google Ads Table Name: ParentalStatus


Google Ads API Resource:
[parental_status_view](https://developers.google.com/google-ads/api/fields/v22/parental_status_view)

| Google Ads Field Name | Description | Adwords Mapped Field Name |
|---|---|---|
| ad_group_base_ad_group | For draft or experiment ad groups, this field is the resource name of the base ad group from which this ad group was created. If a draft or experiment ad group does not have a base ad group, then this field is null. For base ad groups, this field equals the ad group resource name. This field is read-only. | BaseAdGroupId |
| ad_group_criterion_criterion_id | The ID of the criterion. This field is ignored for mutates. | CriterionId |
| ad_group_criterion_effective_cpc_bid_micros | The effective CPC (cost-per-click) bid. | CpcBid |
| ad_group_criterion_effective_cpc_bid_source | Source of the effective CPC bid. | CpcBidSource |
| ad_group_criterion_effective_cpm_bid_micros | The effective CPM (cost-per-thousand viewable impressions) bid. | CpmBidStr |
| ad_group_criterion_effective_cpm_bid_source | Source of the effective CPM bid. | CpmBidSource |
| ad_group_criterion_final_mobile_urls | The list of possible final mobile URLs after all cross-domain redirects. | FinalMobileUrls |
| ad_group_criterion_final_urls | The list of possible final URLs after all cross-domain redirects for the ad. | FinalUrls |
| ad_group_criterion_negative | Whether to target (\`false\`) or exclude (\`true\`) the criterion. This field is immutable. To switch a criterion from positive to negative, remove then re-add it. | IsNegative |
| ad_group_criterion_parental_status_type | Type of the parental status. | Criteria |
| ad_group_criterion_status | The status of the criterion. | Status |
| ad_group_criterion_tracking_url_template | The URL template for constructing a tracking URL. | TrackingUrlTemplate |
| ad_group_criterion_url_custom_parameters | The list of mappings used to substitute custom parameter tags in a \`tracking_url_template\`, \`final_urls\`, or \`mobile_final_urls\`. | UrlCustomParameters |
| ad_group_id | The ID of the ad group. | AdGroupId |
| ad_group_targeting_setting_target_restrictions | The per-targeting-dimension setting to restrict the reach of your campaign or ad group. |   |
| campaign_base_campaign | The resource name of the base campaign of a draft or experiment campaign. For base campaigns, this is equal to \`resource_name\`. This field is read-only. | BaseCampaignId |
| campaign_bidding_strategy | Portfolio bidding strategy used by campaign. | BiddingStrategyId |
| campaign_id | The ID of the campaign. | CampaignId |
| customer_id | The ID of the customer. | ExternalCustomerId |

Google Ads Table Name: ParentalStatusBasicStats


Google Ads API Resource:
[parental_status_view](https://developers.google.com/google-ads/api/fields/v22/parental_status_view)

| Google Ads Field Name | Description | Adwords Mapped Field Name |
|---|---|---|
| ad_group_base_ad_group | For draft or experiment ad groups, this field is the resource name of the base ad group from which this ad group was created. If a draft or experiment ad group does not have a base ad group, then this field is null. For base ad groups, this field equals the ad group resource name. This field is read-only. | BaseAdGroupId |
| ad_group_criterion_criterion_id | The ID of the criterion. This field is ignored for mutates. | CriterionId |
| ad_group_id | The ID of the ad group. | AdGroupId |
| campaign_base_campaign | The resource name of the base campaign of a draft or experiment campaign. For base campaigns, this is equal to \`resource_name\`. This field is read-only. | BaseCampaignId |
| campaign_id | The ID of the campaign. | CampaignId |
| customer_id | The ID of the customer. | ExternalCustomerId |
| metrics_active_view_impressions | A measurement of how often your ad has become viewable on a Display Network site. | ActiveViewImpressions |
| metrics_active_view_measurability | The ratio of impressions that could be measured by Active View over the number of served impressions. | ActiveViewMeasurability |
| metrics_active_view_measurable_cost_micros | The cost of the impressions you received that were measurable by Active View. | ActiveViewMeasurableCost |
| metrics_active_view_measurable_impressions | The number of times your ads are appearing on placements in positions where they can be seen. | ActiveViewMeasurableImpressions |
| metrics_active_view_viewability | The percentage of time when your ad appeared on an Active View enabled site (measurable impressions) and was viewable (viewable impressions). | ActiveViewViewability |
| metrics_all_conversions | The total number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | AllConversions |
| metrics_all_conversions_value | The total value of all conversions. | AllConversionValue |
| metrics_clicks | The number of clicks. | Clicks |
| metrics_conversions | The number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | Conversions |
| metrics_conversions_value | The total value of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | ConversionValue |
| metrics_cost_micros | The sum of your cost-per-click (CPC) and cost-per-thousand impressions (CPM) costs during this period. | Cost |
| metrics_cross_device_conversions | Conversions from when a customer clicks on a Google Ads ad on one device, then converts on a different device or browser. Cross-device conversions are already included in all_conversions. | CrossDeviceConversions |
| metrics_impressions | Count of how often your ad has appeared on a search results page or website on the Google Network. | Impressions |
| metrics_interaction_event_types | The types of payable and free interactions. | InteractionTypes |
| metrics_interactions | The number of interactions. An interaction is the main user action associated with an ad format, such as clicks for text and shopping ads or views for video ads. | Interactions |
| metrics_view_through_conversions | The total number of view-through conversions. These happen when a customer sees an image or rich media ad, then later completes a conversion on your site without interacting with (for example, clicking on) another ad. | ViewThroughConversions |
| segments_ad_network_type | Ad network type. | AdNetworkType2 |
| segments_date | Date to which metrics apply. yyyy-MM-dd format. For example, 2018-04-17. | Date |
| segments_device | Device to which metrics apply. | Device |

Google Ads Table Name: ParentalStatusConversionStats


Google Ads API Resource:
[parental_status_view](https://developers.google.com/google-ads/api/fields/v22/parental_status_view)

| Google Ads Field Name | Description | Adwords Mapped Field Name |
|---|---|---|
| ad_group_base_ad_group | For draft or experiment ad groups, this field is the resource name of the base ad group from which this ad group was created. If a draft or experiment ad group does not have a base ad group, then this field is null. For base ad groups, this field equals the ad group resource name. This field is read-only. | BaseAdGroupId |
| ad_group_criterion_criterion_id | The ID of the criterion. This field is ignored for mutates. | CriterionId |
| ad_group_id | The ID of the ad group. | AdGroupId |
| campaign_base_campaign | The resource name of the base campaign of a draft or experiment campaign. For base campaigns, this is equal to \`resource_name\`. This field is read-only. | BaseCampaignId |
| campaign_id | The ID of the campaign. | CampaignId |
| customer_id | The ID of the customer. | ExternalCustomerId |
| metrics_all_conversions | The total number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | AllConversions |
| metrics_all_conversions_value | The total value of all conversions. | AllConversionValue |
| metrics_conversions | The number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | Conversions |
| metrics_conversions_value | The total value of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | ConversionValue |
| metrics_cross_device_conversions | Conversions from when a customer clicks on a Google Ads ad on one device, then converts on a different device or browser. Cross-device conversions are already included in all_conversions. | CrossDeviceConversions |
| metrics_value_per_all_conversions | The value of all conversions divided by the number of all conversions. | ValuePerAllConversion |
| metrics_value_per_conversion | The value of conversions divided by the number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | ValuePerConversion |
| segments_ad_network_type | Ad network type. | AdNetworkType2 |
| segments_click_type | Click type. | ClickType |
| segments_conversion_action | Resource name of the conversion action. | ConversionTrackerId |
| segments_conversion_action_category | Conversion action category. | ConversionCategoryName |
| segments_conversion_action_name | Conversion action name. | ConversionTypeName |
| segments_date | Date to which metrics apply. yyyy-MM-dd format. For example, 2018-04-17. | Date |
| segments_day_of_week | Day of the week. For example, MONDAY. | DayOfWeek |
| segments_device | Device to which metrics apply. | Device |
| segments_month | Month as represented by the date of the first day of a month. Formatted as yyyy-MM-dd. | Month |
| segments_quarter | Quarter as represented by the date of the first day of a quarter. Uses the calendar year for quarters. For example, the second quarter of 2018 starts on 2018-04-01. Formatted as yyyy-MM-dd. | Quarter |
| segments_week | Week as defined as Monday through Sunday, and represented by the date of Monday. Formatted as yyyy-MM-dd. | Week |
| segments_year | Year, formatted as yyyy. | Year |

Google Ads Table Name: ParentalStatusNonClickStats


Google Ads API Resource:
[parental_status_view](https://developers.google.com/google-ads/api/fields/v22/parental_status_view)

| Google Ads Field Name | Description | Adwords Mapped Field Name |
|---|---|---|
| ad_group_base_ad_group | For draft or experiment ad groups, this field is the resource name of the base ad group from which this ad group was created. If a draft or experiment ad group does not have a base ad group, then this field is null. For base ad groups, this field equals the ad group resource name. This field is read-only. | BaseAdGroupId |
| ad_group_criterion_criterion_id | The ID of the criterion. This field is ignored for mutates. | CriterionId |
| ad_group_id | The ID of the ad group. | AdGroupId |
| campaign_base_campaign | The resource name of the base campaign of a draft or experiment campaign. For base campaigns, this is equal to \`resource_name\`. This field is read-only. | BaseCampaignId |
| campaign_id | The ID of the campaign. | CampaignId |
| customer_id | The ID of the customer. | ExternalCustomerId |
| metrics_average_cpe | The average amount that you've been charged for an ad engagement. This amount is the total cost of all ad engagements divided by the total number of ad engagements. | AverageCpe |
| metrics_trueview_average_cpv | The average amount you pay each time someone views your ad. The average CPV is defined by the total cost of all ad views divided by the number of views. | AverageCpv |
| metrics_engagement_rate | How often people engage with your ad after it's shown to them. This is the number of ad expansions divided by the number of times your ad is shown. | EngagementRate |
| metrics_engagements | The number of engagements. An engagement occurs when a viewer expands your Lightbox ad. Also, in the future, other ad types may support engagement metrics. | Engagements |
| metrics_impressions | Count of how often your ad has appeared on a search results page or website on the Google Network. |   |
| metrics_video_quartile_p100_rate | Percentage of impressions where the viewer watched all of your video. | VideoQuartile100Rate |
| metrics_video_quartile_p25_rate | Percentage of impressions where the viewer watched 25% of your video. | VideoQuartile25Rate |
| metrics_video_quartile_p50_rate | Percentage of impressions where the viewer watched 50% of your video. | VideoQuartile50Rate |
| metrics_video_quartile_p75_rate | Percentage of impressions where the viewer watched 75% of your video. | VideoQuartile75Rate |
| metrics_video_trueview_view_rate | The number of views your TrueView video ad receives divided by its number of impressions, including thumbnail impressions for TrueView in-display ads. | VideoViewRate |
| metrics_video_trueview_views | The number of times your video ads were viewed. | VideoViews |
| segments_ad_network_type | Ad network type. | AdNetworkType2 |
| segments_date | Date to which metrics apply. yyyy-MM-dd format. For example, 2018-04-17. | Date |
| segments_day_of_week | Day of the week. For example, MONDAY. | DayOfWeek |
| segments_device | Device to which metrics apply. | Device |
| segments_month | Month as represented by the date of the first day of a month. Formatted as yyyy-MM-dd. | Month |
| segments_quarter | Quarter as represented by the date of the first day of a quarter. Uses the calendar year for quarters. For example, the second quarter of 2018 starts on 2018-04-01. Formatted as yyyy-MM-dd. | Quarter |
| segments_week | Week as defined as Monday through Sunday, and represented by the date of Monday. Formatted as yyyy-MM-dd. | Week |
| segments_year | Year, formatted as yyyy. | Year |

Google Ads Table Name: ParentalStatusStats


Google Ads API Resource:
[parental_status_view](https://developers.google.com/google-ads/api/fields/v22/parental_status_view)

| Google Ads Field Name | Description | Adwords Mapped Field Name |
|---|---|---|
| ad_group_base_ad_group | For draft or experiment ad groups, this field is the resource name of the base ad group from which this ad group was created. If a draft or experiment ad group does not have a base ad group, then this field is null. For base ad groups, this field equals the ad group resource name. This field is read-only. | BaseAdGroupId |
| ad_group_criterion_criterion_id | The ID of the criterion. This field is ignored for mutates. | CriterionId |
| ad_group_id | The ID of the ad group. | AdGroupId |
| campaign_base_campaign | The resource name of the base campaign of a draft or experiment campaign. For base campaigns, this is equal to \`resource_name\`. This field is read-only. | BaseCampaignId |
| campaign_id | The ID of the campaign. | CampaignId |
| customer_id | The ID of the customer. | ExternalCustomerId |
| metrics_active_view_cpm | Average cost of viewable impressions (\`active_view_impressions\`). | ActiveViewCpm |
| metrics_active_view_ctr | Active view measurable clicks divided by active view viewable impressions. This metric is reported only for display network. | ActiveViewCtr |
| metrics_active_view_impressions | A measurement of how often your ad has become viewable on a Display Network site. | ActiveViewImpressions |
| metrics_active_view_measurability | The ratio of impressions that could be measured by Active View over the number of served impressions. | ActiveViewMeasurability |
| metrics_active_view_measurable_cost_micros | The cost of the impressions you received that were measurable by Active View. | ActiveViewMeasurableCost |
| metrics_active_view_measurable_impressions | The number of times your ads are appearing on placements in positions where they can be seen. | ActiveViewMeasurableImpressions |
| metrics_active_view_viewability | The percentage of time when your ad appeared on an Active View enabled site (measurable impressions) and was viewable (viewable impressions). | ActiveViewViewability |
| metrics_all_conversions | The total number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | AllConversions |
| metrics_all_conversions_from_interactions_rate | All conversions from interactions (as oppose to view through conversions) divided by the number of ad interactions. | AllConversionRate |
| metrics_all_conversions_value | The total value of all conversions. | AllConversionValue |
| metrics_average_cost | The average amount you pay per interaction. This amount is the total cost of your ads divided by the total number of interactions. | AverageCost |
| metrics_average_cpc | The total cost of all clicks divided by the total number of clicks received. | AverageCpc |
| metrics_average_cpm | Average cost-per-thousand impressions (CPM). | AverageCpm |
| metrics_clicks | The number of clicks. | Clicks |
| metrics_conversions | The number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | Conversions |
| metrics_conversions_from_interactions_rate | Conversions from interactions divided by the number of ad interactions (such as clicks for text ads or views for video ads). This only includes conversion actions which include_in_conversions_metric attribute is set to true. | ConversionRate |
| metrics_conversions_value | The total value of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | ConversionValue |
| metrics_cost_micros | The sum of your cost-per-click (CPC) and cost-per-thousand impressions (CPM) costs during this period. | Cost |
| metrics_cost_per_all_conversions | The cost of ad interactions divided by all conversions. | CostPerAllConversion |
| metrics_cost_per_conversion | The cost of ad interactions divided by conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | CostPerConversion |
| metrics_cross_device_conversions | Conversions from when a customer clicks on a Google Ads ad on one device, then converts on a different device or browser. Cross-device conversions are already included in all_conversions. | CrossDeviceConversions |
| metrics_ctr | The number of clicks your ad receives (Clicks) divided by the number of times your ad is shown (Impressions). | Ctr |
| metrics_gmail_forwards | The number of times the ad was forwarded to someone else as a message. | GmailForwards |
| metrics_gmail_saves | The number of times someone has saved your Gmail ad to their inbox as a message. | GmailSaves |
| metrics_gmail_secondary_clicks | The number of clicks to the landing page on the expanded state of Gmail ads. | GmailSecondaryClicks |
| metrics_impressions | Count of how often your ad has appeared on a search results page or website on the Google Network. | Impressions |
| metrics_interaction_event_types | The types of payable and free interactions. | InteractionTypes |
| metrics_interaction_rate | How often people interact with your ad after it is shown to them. This is the number of interactions divided by the number of times your ad is shown. | InteractionRate |
| metrics_interactions | The number of interactions. An interaction is the main user action associated with an ad format, such as clicks for text and shopping ads or views for video ads. | Interactions |
| metrics_value_per_all_conversions | The value of all conversions divided by the number of all conversions. | ValuePerAllConversion |
| metrics_value_per_conversion | The value of conversions divided by the number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | ValuePerConversion |
| segments_ad_network_type | Ad network type. | AdNetworkType2 |
| segments_click_type | Click type. | ClickType |
| segments_date | Date to which metrics apply. yyyy-MM-dd format. For example, 2018-04-17. | Date |
| segments_day_of_week | Day of the week. For example, MONDAY. | DayOfWeek |
| segments_device | Device to which metrics apply. | Device |
| segments_month | Month as represented by the date of the first day of a month. Formatted as yyyy-MM-dd. | Month |
| segments_quarter | Quarter as represented by the date of the first day of a quarter. Uses the calendar year for quarters. For example, the second quarter of 2018 starts on 2018-04-01. Formatted as yyyy-MM-dd. | Quarter |
| segments_week | Week as defined as Monday through Sunday, and represented by the date of Monday. Formatted as yyyy-MM-dd. | Week |
| segments_year | Year, formatted as yyyy. | Year |

Google Ads Table Name: Placement


Google Ads API Resource:
[managed_placement_view](https://developers.google.com/google-ads/api/fields/v22/managed_placement_view)

| Google Ads Field Name | Description | Adwords Mapped Field Name |
|---|---|---|
| ad_group_base_ad_group | For draft or experiment ad groups, this field is the resource name of the base ad group from which this ad group was created. If a draft or experiment ad group does not have a base ad group, then this field is null. For base ad groups, this field equals the ad group resource name. This field is read-only. | BaseAdGroupId |
| ad_group_criterion_bid_modifier | The modifier for the bid when the criterion matches. The modifier must be in the range: 0.1 - 10.0. Most targetable criteria types support modifiers. | BidModifier |
| ad_group_criterion_criterion_id | The ID of the criterion. This field is ignored for mutates. | CriterionId |
| ad_group_criterion_effective_cpc_bid_micros | The effective CPC (cost-per-click) bid. | CpcBid |
| ad_group_criterion_effective_cpc_bid_source | Source of the effective CPC bid. | CpcBidSource |
| ad_group_criterion_effective_cpm_bid_micros | The effective CPM (cost-per-thousand viewable impressions) bid. | CpmBidStr |
| ad_group_criterion_effective_cpm_bid_source | Source of the effective CPM bid. | CpmBidSource |
| ad_group_criterion_final_mobile_urls | The list of possible final mobile URLs after all cross-domain redirects. | FinalMobileUrls |
| ad_group_criterion_final_urls | The list of possible final URLs after all cross-domain redirects for the ad. | FinalUrls |
| ad_group_criterion_negative | Whether to target (\`false\`) or exclude (\`true\`) the criterion. This field is immutable. To switch a criterion from positive to negative, remove then re-add it. | IsNegative |
| ad_group_criterion_placement_url | URL of the placement. For example, \\"http://www.domain.com\\". | Criteria |
| ad_group_criterion_status | The status of the criterion. | Status |
| ad_group_criterion_tracking_url_template | The URL template for constructing a tracking URL. | TrackingUrlTemplate |
| ad_group_criterion_url_custom_parameters | The list of mappings used to substitute custom parameter tags in a \`tracking_url_template\`, \`final_urls\`, or \`mobile_final_urls\`. | UrlCustomParameters |
| ad_group_id | The ID of the ad group. | AdGroupId |
| ad_group_targeting_setting_target_restrictions | The per-targeting-dimension setting to restrict the reach of your campaign or ad group. |   |
| bidding_strategy_name | The name of the bidding strategy. All bidding strategies within an account must be named distinctly. The length of this string should be between 1 and 255, inclusive, in UTF-8 bytes, (trimmed). | BiddingStrategyName |
| bidding_strategy_type | The type of the bidding strategy. Create a bidding strategy by setting the bidding scheme. This field is read-only. | BiddingStrategyType |
| campaign_base_campaign | The resource name of the base campaign of a draft or experiment campaign. For base campaigns, this is equal to \`resource_name\`. This field is read-only. | BaseCampaignId |
| campaign_bidding_strategy | Portfolio bidding strategy used by campaign. | BiddingStrategyId |
| campaign_id | The ID of the campaign. | CampaignId |
| customer_id | The ID of the customer. | ExternalCustomerId |

Google Ads Table Name: PlacementBasicStats


Google Ads API Resource:
[managed_placement_view](https://developers.google.com/google-ads/api/fields/v22/managed_placement_view)

| Google Ads Field Name | Description | Adwords Mapped Field Name |
|---|---|---|
| ad_group_base_ad_group | For draft or experiment ad groups, this field is the resource name of the base ad group from which this ad group was created. If a draft or experiment ad group does not have a base ad group, then this field is null. For base ad groups, this field equals the ad group resource name. This field is read-only. | BaseAdGroupId |
| ad_group_criterion_criterion_id | The ID of the criterion. This field is ignored for mutates. | CriterionId |
| ad_group_id | The ID of the ad group. | AdGroupId |
| campaign_base_campaign | The resource name of the base campaign of a draft or experiment campaign. For base campaigns, this is equal to \`resource_name\`. This field is read-only. | BaseCampaignId |
| campaign_id | The ID of the campaign. | CampaignId |
| customer_id | The ID of the customer. | ExternalCustomerId |
| metrics_active_view_impressions | A measurement of how often your ad has become viewable on a Display Network site. | ActiveViewImpressions |
| metrics_active_view_measurability | The ratio of impressions that could be measured by Active View over the number of served impressions. | ActiveViewMeasurability |
| metrics_active_view_measurable_cost_micros | The cost of the impressions you received that were measurable by Active View. | ActiveViewMeasurableCost |
| metrics_active_view_measurable_impressions | The number of times your ads are appearing on placements in positions where they can be seen. | ActiveViewMeasurableImpressions |
| metrics_active_view_viewability | The percentage of time when your ad appeared on an Active View enabled site (measurable impressions) and was viewable (viewable impressions). | ActiveViewViewability |
| metrics_all_conversions | The total number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | AllConversions |
| metrics_all_conversions_value | The total value of all conversions. | AllConversionValue |
| metrics_clicks | The number of clicks. | Clicks |
| metrics_conversions | The number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | Conversions |
| metrics_conversions_value | The total value of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | ConversionValue |
| metrics_cost_micros | The sum of your cost-per-click (CPC) and cost-per-thousand impressions (CPM) costs during this period. | Cost |
| metrics_cross_device_conversions | Conversions from when a customer clicks on a Google Ads ad on one device, then converts on a different device or browser. Cross-device conversions are already included in all_conversions. | CrossDeviceConversions |
| metrics_impressions | Count of how often your ad has appeared on a search results page or website on the Google Network. | Impressions |
| metrics_interaction_event_types | The types of payable and free interactions. | InteractionTypes |
| metrics_interactions | The number of interactions. An interaction is the main user action associated with an ad format, such as clicks for text and shopping ads or views for video ads. | Interactions |
| metrics_view_through_conversions | The total number of view-through conversions. These happen when a customer sees an image or rich media ad, then later completes a conversion on your site without interacting with (for example, clicking on) another ad. | ViewThroughConversions |
| segments_ad_network_type | Ad network type. | AdNetworkType2 |
| segments_date | Date to which metrics apply. yyyy-MM-dd format. For example, 2018-04-17. | Date |
| segments_device | Device to which metrics apply. | Device |

Google Ads Table Name: PlacementConversionStats


Google Ads API Resource:
[managed_placement_view](https://developers.google.com/google-ads/api/fields/v22/managed_placement_view)

| Google Ads Field Name | Description | Adwords Mapped Field Name |
|---|---|---|
| ad_group_base_ad_group | For draft or experiment ad groups, this field is the resource name of the base ad group from which this ad group was created. If a draft or experiment ad group does not have a base ad group, then this field is null. For base ad groups, this field equals the ad group resource name. This field is read-only. | BaseAdGroupId |
| ad_group_criterion_criterion_id | The ID of the criterion. This field is ignored for mutates. | CriterionId |
| ad_group_id | The ID of the ad group. | AdGroupId |
| campaign_base_campaign | The resource name of the base campaign of a draft or experiment campaign. For base campaigns, this is equal to \`resource_name\`. This field is read-only. | BaseCampaignId |
| campaign_id | The ID of the campaign. | CampaignId |
| customer_id | The ID of the customer. | ExternalCustomerId |
| metrics_all_conversions | The total number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | AllConversions |
| metrics_all_conversions_value | The total value of all conversions. | AllConversionValue |
| metrics_conversions | The number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | Conversions |
| metrics_conversions_value | The total value of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | ConversionValue |
| metrics_cross_device_conversions | Conversions from when a customer clicks on a Google Ads ad on one device, then converts on a different device or browser. Cross-device conversions are already included in all_conversions. | CrossDeviceConversions |
| metrics_value_per_all_conversions | The value of all conversions divided by the number of all conversions. | ValuePerAllConversion |
| metrics_value_per_conversion | The value of conversions divided by the number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | ValuePerConversion |
| segments_ad_network_type | Ad network type. | AdNetworkType2 |
| segments_click_type | Click type. | ClickType |
| segments_conversion_action | Resource name of the conversion action. | ConversionTrackerId |
| segments_conversion_action_category | Conversion action category. | ConversionCategoryName |
| segments_conversion_action_name | Conversion action name. | ConversionTypeName |
| segments_date | Date to which metrics apply. yyyy-MM-dd format. For example, 2018-04-17. | Date |
| segments_day_of_week | Day of the week. For example, MONDAY. | DayOfWeek |
| segments_device | Device to which metrics apply. | Device |
| segments_month | Month as represented by the date of the first day of a month. Formatted as yyyy-MM-dd. | Month |
| segments_quarter | Quarter as represented by the date of the first day of a quarter. Uses the calendar year for quarters. For example, the second quarter of 2018 starts on 2018-04-01. Formatted as yyyy-MM-dd. | Quarter |
| segments_week | Week as defined as Monday through Sunday, and represented by the date of Monday. Formatted as yyyy-MM-dd. | Week |
| segments_year | Year, formatted as yyyy. | Year |

Google Ads Table Name: PlacementNonClickStats


Google Ads API Resource:
[managed_placement_view](https://developers.google.com/google-ads/api/fields/v22/managed_placement_view)

| Google Ads Field Name | Description | Adwords Mapped Field Name |
|---|---|---|
| ad_group_base_ad_group | For draft or experiment ad groups, this field is the resource name of the base ad group from which this ad group was created. If a draft or experiment ad group does not have a base ad group, then this field is null. For base ad groups, this field equals the ad group resource name. This field is read-only. | BaseAdGroupId |
| ad_group_criterion_criterion_id | The ID of the criterion. This field is ignored for mutates. | CriterionId |
| ad_group_id | The ID of the ad group. | AdGroupId |
| campaign_base_campaign | The resource name of the base campaign of a draft or experiment campaign. For base campaigns, this is equal to \`resource_name\`. This field is read-only. | BaseCampaignId |
| campaign_id | The ID of the campaign. | CampaignId |
| customer_id | The ID of the customer. | ExternalCustomerId |
| metrics_average_cpe | The average amount that you've been charged for an ad engagement. This amount is the total cost of all ad engagements divided by the total number of ad engagements. | AverageCpe |
| metrics_trueview_average_cpv | The average amount you pay each time someone views your ad. The average CPV is defined by the total cost of all ad views divided by the number of views. | AverageCpv |
| metrics_engagement_rate | How often people engage with your ad after it's shown to them. This is the number of ad expansions divided by the number of times your ad is shown. | EngagementRate |
| metrics_engagements | The number of engagements. An engagement occurs when a viewer expands your Lightbox ad. Also, in the future, other ad types may support engagement metrics. | Engagements |
| metrics_video_quartile_p100_rate | Percentage of impressions where the viewer watched all of your video. | VideoQuartile100Rate |
| metrics_video_quartile_p25_rate | Percentage of impressions where the viewer watched 25% of your video. | VideoQuartile25Rate |
| metrics_video_quartile_p50_rate | Percentage of impressions where the viewer watched 50% of your video. | VideoQuartile50Rate |
| metrics_video_quartile_p75_rate | Percentage of impressions where the viewer watched 75% of your video. | VideoQuartile75Rate |
| metrics_video_trueview_view_rate | The number of views your TrueView video ad receives divided by its number of impressions, including thumbnail impressions for TrueView in-display ads. | VideoViewRate |
| metrics_video_trueview_views | The number of times your video ads were viewed. | VideoViews |
| segments_ad_network_type | Ad network type. | AdNetworkType2 |
| segments_date | Date to which metrics apply. yyyy-MM-dd format. For example, 2018-04-17. | Date |
| segments_day_of_week | Day of the week. For example, MONDAY. | DayOfWeek |
| segments_device | Device to which metrics apply. | Device |
| segments_month | Month as represented by the date of the first day of a month. Formatted as yyyy-MM-dd. | Month |
| segments_quarter | Quarter as represented by the date of the first day of a quarter. Uses the calendar year for quarters. For example, the second quarter of 2018 starts on 2018-04-01. Formatted as yyyy-MM-dd. | Quarter |
| segments_week | Week as defined as Monday through Sunday, and represented by the date of Monday. Formatted as yyyy-MM-dd. | Week |
| segments_year | Year, formatted as yyyy. | Year |

Google Ads Table Name: PlacementStats


Google Ads API Resource:
[managed_placement_view](https://developers.google.com/google-ads/api/fields/v22/managed_placement_view)

| Google Ads Field Name | Description | Adwords Mapped Field Name |
|---|---|---|
| ad_group_base_ad_group | For draft or experiment ad groups, this field is the resource name of the base ad group from which this ad group was created. If a draft or experiment ad group does not have a base ad group, then this field is null. For base ad groups, this field equals the ad group resource name. This field is read-only. | BaseAdGroupId |
| ad_group_criterion_criterion_id | The ID of the criterion. This field is ignored for mutates. | CriterionId |
| ad_group_id | The ID of the ad group. | AdGroupId |
| campaign_base_campaign | The resource name of the base campaign of a draft or experiment campaign. For base campaigns, this is equal to \`resource_name\`. This field is read-only. | BaseCampaignId |
| campaign_id | The ID of the campaign. | CampaignId |
| customer_id | The ID of the customer. | ExternalCustomerId |
| metrics_active_view_cpm | Average cost of viewable impressions (\`active_view_impressions\`). | ActiveViewCpm |
| metrics_active_view_ctr | Active view measurable clicks divided by active view viewable impressions. This metric is reported only for display network. | ActiveViewCtr |
| metrics_active_view_impressions | A measurement of how often your ad has become viewable on a Display Network site. | ActiveViewImpressions |
| metrics_active_view_measurability | The ratio of impressions that could be measured by Active View over the number of served impressions. | ActiveViewMeasurability |
| metrics_active_view_measurable_cost_micros | The cost of the impressions you received that were measurable by Active View. | ActiveViewMeasurableCost |
| metrics_active_view_measurable_impressions | The number of times your ads are appearing on placements in positions where they can be seen. | ActiveViewMeasurableImpressions |
| metrics_active_view_viewability | The percentage of time when your ad appeared on an Active View enabled site (measurable impressions) and was viewable (viewable impressions). | ActiveViewViewability |
| metrics_all_conversions | The total number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | AllConversions |
| metrics_all_conversions_from_interactions_rate | All conversions from interactions (as oppose to view through conversions) divided by the number of ad interactions. | AllConversionRate, ConversionRate |
| metrics_all_conversions_value | The total value of all conversions. | AllConversionValue |
| metrics_average_cost | The average amount you pay per interaction. This amount is the total cost of your ads divided by the total number of interactions. | AverageCost |
| metrics_average_cpc | The total cost of all clicks divided by the total number of clicks received. | AverageCpc |
| metrics_average_cpm | Average cost-per-thousand impressions (CPM). | AverageCpm |
| metrics_clicks | The number of clicks. | Clicks |
| metrics_conversions | The number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | Conversions |
| metrics_conversions_value | The total value of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | ConversionValue |
| metrics_cost_micros | The sum of your cost-per-click (CPC) and cost-per-thousand impressions (CPM) costs during this period. | Cost |
| metrics_cost_per_all_conversions | The cost of ad interactions divided by all conversions. | CostPerAllConversion |
| metrics_cost_per_conversion | The cost of ad interactions divided by conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | CostPerConversion |
| metrics_cross_device_conversions | Conversions from when a customer clicks on a Google Ads ad on one device, then converts on a different device or browser. Cross-device conversions are already included in all_conversions. | CrossDeviceConversions |
| metrics_ctr | The number of clicks your ad receives (Clicks) divided by the number of times your ad is shown (Impressions). | Ctr |
| metrics_gmail_forwards | The number of times the ad was forwarded to someone else as a message. | GmailForwards |
| metrics_gmail_saves | The number of times someone has saved your Gmail ad to their inbox as a message. | GmailSaves |
| metrics_gmail_secondary_clicks | The number of clicks to the landing page on the expanded state of Gmail ads. | GmailSecondaryClicks |
| metrics_impressions | Count of how often your ad has appeared on a search results page or website on the Google Network. | Impressions |
| metrics_interaction_event_types | The types of payable and free interactions. | InteractionTypes |
| metrics_interaction_rate | How often people interact with your ad after it is shown to them. This is the number of interactions divided by the number of times your ad is shown. | InteractionRate |
| metrics_interactions | The number of interactions. An interaction is the main user action associated with an ad format, such as clicks for text and shopping ads or views for video ads. | Interactions |
| metrics_value_per_all_conversions | The value of all conversions divided by the number of all conversions. | ValuePerAllConversion |
| metrics_value_per_conversion | The value of conversions divided by the number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | ValuePerConversion |
| segments_ad_network_type | Ad network type. | AdNetworkType2 |
| segments_click_type | Click type. | ClickType |
| segments_date | Date to which metrics apply. yyyy-MM-dd format. For example, 2018-04-17. | Date |
| segments_day_of_week | Day of the week. For example, MONDAY. | DayOfWeek |
| segments_device | Device to which metrics apply. | Device |
| segments_month | Month as represented by the date of the first day of a month. Formatted as yyyy-MM-dd. | Month |
| segments_quarter | Quarter as represented by the date of the first day of a quarter. Uses the calendar year for quarters. For example, the second quarter of 2018 starts on 2018-04-01. Formatted as yyyy-MM-dd. | Quarter |
| segments_week | Week as defined as Monday through Sunday, and represented by the date of Monday. Formatted as yyyy-MM-dd. | Week |
| segments_year | Year, formatted as yyyy. | Year |

Google Ads Table Name: ProductGroupStats


Google Ads API Resource:
[product_group_view](https://developers.google.com/google-ads/api/fields/v22/product_group_view)

| Google Ads Field Name | Description | Adwords Mapped Field Name |
|---|---|---|
| ad_group_criterion_cpc_bid_micros | The CPC (cost-per-click) bid. |   |
| ad_group_criterion_criterion_id | The ID of the criterion. This field is ignored for mutates. |   |
| ad_group_criterion_display_name | The display name of the criterion. This field is ignored for mutates. |   |
| ad_group_criterion_listing_group_case_value_product_bidding_category_id (obsolete - use ad_group_criterion_listing_group_case_value_product_category_category_id instead) | ID of the product bidding category. This ID is equivalent to the google_product_category ID as described in this document: https://support.google.com/merchants/answer/6324436 |   |
| ad_group_criterion_listing_group_case_value_product_bidding_category_level (obsolete - use ad_group_criterion_listing_group_case_value_product_category_level instead) | Level of the product bidding category. |   |
| ad_group_criterion_listing_group_case_value_product_brand_value | String value of the product brand. |   |
| ad_group_criterion_listing_group_case_value_product_channel_channel | Value of the locality. |   |
| ad_group_criterion_listing_group_case_value_product_channel_exclusivity_channel_exclusivity | Value of the availability. |   |
| ad_group_criterion_listing_group_case_value_product_condition_condition | Value of the condition. |   |
| ad_group_criterion_listing_group_case_value_product_custom_attribute_index | Indicates the index of the custom attribute. |   |
| ad_group_criterion_listing_group_case_value_product_custom_attribute_value | String value of the product custom attribute. |   |
| ad_group_criterion_listing_group_case_value_product_item_id_value | Value of the id. |   |
| ad_group_criterion_listing_group_case_value_product_type_level | Level of the type. |   |
| ad_group_criterion_listing_group_case_value_product_type_value | Value of the type. |   |
| ad_group_criterion_status | The status of the criterion. |   |
| ad_group_id | The ID of the ad group. |   |
| campaign_id | The ID of the campaign. |   |
| metrics_average_cpc | The total cost of all clicks divided by the total number of clicks received. |   |
| metrics_average_cpm | Average cost-per-thousand impressions (CPM). |   |
| metrics_clicks | The number of clicks. |   |
| metrics_cost_micros | The sum of your cost-per-click (CPC) and cost-per-thousand impressions (CPM) costs during this period. |   |
| metrics_ctr | The number of clicks your ad receives (Clicks) divided by the number of times your ad is shown (Impressions). |   |
| metrics_impressions | Count of how often your ad has appeared on a search results page or website on the Google Network. |   |
| product_group_view_resource_name | The resource name of the product group view. Product group view resource names have the form: customers/{customer_id}/productGroupViews/{ad_group_id}\~{criterion_id} |   |
| segments_date | Date to which metrics apply. yyyy-MM-dd format. For example, 2018-04-17. |   |
| segments_day_of_week | Day of the week. For example, MONDAY. |   |
| segments_month | Month as represented by the date of the first day of a month. Formatted as yyyy-MM-dd. |   |
| segments_quarter | Quarter as represented by the date of the first day of a quarter. Uses the calendar year for quarters. For example, the second quarter of 2018 starts on 2018-04-01. Formatted as yyyy-MM-dd. |   |
| segments_week | Week as defined as Monday through Sunday, and represented by the date of Monday. Formatted as yyyy-MM-dd. |   |
| segments_year | Year, formatted as yyyy. |   |

Google Ads Table Name: SearchQueryConversionStats


Google Ads API Resource:
[search_term_view](https://developers.google.com/google-ads/api/fields/v22/search_term_view)

| Google Ads Field Name | Description | Adwords Mapped Field Name |
|---|---|---|
| ad_group_ad_ad_id | The ID of the ad. | CreativeId |
| ad_group_id | The ID of the ad group. | AdGroupId |
| campaign_id | The ID of the campaign. | CampaignId |
| customer_id | The ID of the customer. | ExternalCustomerId |
| metrics_all_conversions | The total number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | AllConversions |
| metrics_all_conversions_value | The total value of all conversions. | AllConversionValue |
| metrics_conversions | The number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | Conversions |
| metrics_conversions_value | The total value of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | ConversionValue |
| metrics_cross_device_conversions | Conversions from when a customer clicks on a Google Ads ad on one device, then converts on a different device or browser. Cross-device conversions are already included in all_conversions. | CrossDeviceConversions |
| metrics_value_per_all_conversions | The value of all conversions divided by the number of all conversions. | ValuePerAllConversion |
| metrics_value_per_conversion | The value of conversions divided by the number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | ValuePerConversion |
| metrics_view_through_conversions | The total number of view-through conversions. These happen when a customer sees an image or rich media ad, then later completes a conversion on your site without interacting with (for example, clicking on) another ad. | ViewThroughConversions |
| search_term_view_search_term | The search term. | Query |
| search_term_view_status | Indicates whether the search term is one of your targeted or excluded keywords. | QueryTargetingStatus |
| segments_ad_network_type | Ad network type. | AdNetworkType2 |
| segments_conversion_action | Resource name of the conversion action. | ConversionTrackerId |
| segments_conversion_action_category | Conversion action category. | ConversionCategoryName |
| segments_conversion_action_name | Conversion action name. | ConversionTypeName |
| segments_date | Date to which metrics apply. yyyy-MM-dd format. For example, 2018-04-17. | Date |
| segments_day_of_week | Day of the week. For example, MONDAY. | DayOfWeek |
| segments_device | Device to which metrics apply. | Device |
| segments_month | Month as represented by the date of the first day of a month. Formatted as yyyy-MM-dd. | Month |
| segments_quarter | Quarter as represented by the date of the first day of a quarter. Uses the calendar year for quarters. For example, the second quarter of 2018 starts on 2018-04-01. Formatted as yyyy-MM-dd. | Quarter |
| segments_search_term_match_type | Match type of the keyword that triggered the ad, including variants. | QueryMatchTypeWithVariant |
| segments_week | Week as defined as Monday through Sunday, and represented by the date of Monday. Formatted as yyyy-MM-dd. | Week |
| segments_year | Year, formatted as yyyy. | Year |

Google Ads Table Name: SearchQueryStats


Google Ads API Resource:
[search_term_view](https://developers.google.com/google-ads/api/fields/v22/search_term_view)

| Google Ads Field Name | Description | Adwords Mapped Field Name |
|---|---|---|
| ad_group_ad_ad_id | The ID of the ad. | CreativeId |
| ad_group_id | The ID of the ad group. | AdGroupId |
| campaign_id | The ID of the campaign. | CampaignId |
| customer_id | The ID of the customer. | ExternalCustomerId |
| metrics_absolute_top_impression_percentage | The percent of your ad impressions that are shown as the very first ad above the organic search results. | AbsoluteTopImpressionPercentage |
| metrics_all_conversions | The total number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | AllConversions |
| metrics_all_conversions_from_interactions_rate | All conversions from interactions (as oppose to view through conversions) divided by the number of ad interactions. | AllConversionRate |
| metrics_all_conversions_value | The total value of all conversions. | AllConversionValue |
| metrics_average_cost | The average amount you pay per interaction. This amount is the total cost of your ads divided by the total number of interactions. | AverageCost |
| metrics_average_cpc | The total cost of all clicks divided by the total number of clicks received. | AverageCpc |
| metrics_average_cpe | The average amount that you've been charged for an ad engagement. This amount is the total cost of all ad engagements divided by the total number of ad engagements. | AverageCpe |
| metrics_average_cpm | Average cost-per-thousand impressions (CPM). | AverageCpm |
| metrics_trueview_average_cpv | The average amount you pay each time someone views your ad. The average CPV is defined by the total cost of all ad views divided by the number of views. | AverageCpv |
| metrics_clicks | The number of clicks. | Clicks |
| metrics_conversions | The number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | Conversions |
| metrics_conversions_from_interactions_rate | Conversions from interactions divided by the number of ad interactions (such as clicks for text ads or views for video ads). This only includes conversion actions which include_in_conversions_metric attribute is set to true. | ConversionRate |
| metrics_conversions_value | The total value of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | ConversionValue |
| metrics_cost_micros | The sum of your cost-per-click (CPC) and cost-per-thousand impressions (CPM) costs during this period. | Cost |
| metrics_cost_per_all_conversions | The cost of ad interactions divided by all conversions. | CostPerAllConversion |
| metrics_cost_per_conversion | The cost of ad interactions divided by conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | CostPerConversion |
| metrics_cross_device_conversions | Conversions from when a customer clicks on a Google Ads ad on one device, then converts on a different device or browser. Cross-device conversions are already included in all_conversions. | CrossDeviceConversions |
| metrics_ctr | The number of clicks your ad receives (Clicks) divided by the number of times your ad is shown (Impressions). | Ctr |
| metrics_engagement_rate | How often people engage with your ad after it's shown to them. This is the number of ad expansions divided by the number of times your ad is shown. | EngagementRate |
| metrics_engagements | The number of engagements. An engagement occurs when a viewer expands your Lightbox ad. Also, in the future, other ad types may support engagement metrics. | Engagements |
| metrics_impressions | Count of how often your ad has appeared on a search results page or website on the Google Network. | Impressions |
| metrics_interaction_event_types | The types of payable and free interactions. | InteractionTypes |
| metrics_interaction_rate | How often people interact with your ad after it is shown to them. This is the number of interactions divided by the number of times your ad is shown. | InteractionRate |
| metrics_interactions | The number of interactions. An interaction is the main user action associated with an ad format, such as clicks for text and shopping ads or views for video ads. | Interactions |
| metrics_top_impression_percentage | The percent of your ad impressions that are shown anywhere above the organic search results. | TopImpressionPercentage |
| metrics_value_per_all_conversions | The value of all conversions divided by the number of all conversions. | ValuePerAllConversion |
| metrics_value_per_conversion | The value of conversions divided by the number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | ValuePerConversion |
| metrics_video_quartile_p100_rate | Percentage of impressions where the viewer watched all of your video. | VideoQuartile100Rate |
| metrics_video_quartile_p25_rate | Percentage of impressions where the viewer watched 25% of your video. | VideoQuartile25Rate |
| metrics_video_quartile_p50_rate | Percentage of impressions where the viewer watched 50% of your video. | VideoQuartile50Rate |
| metrics_video_quartile_p75_rate | Percentage of impressions where the viewer watched 75% of your video. | VideoQuartile75Rate |
| metrics_video_trueview_view_rate | The number of views your TrueView video ad receives divided by its number of impressions, including thumbnail impressions for TrueView in-display ads. | VideoViewRate |
| metrics_video_trueview_views | The number of times your video ads were viewed. | VideoViews |
| metrics_view_through_conversions | The total number of view-through conversions. These happen when a customer sees an image or rich media ad, then later completes a conversion on your site without interacting with (for example, clicking on) another ad. | ViewThroughConversions |
| search_term_view_search_term | The search term. | Query |
| search_term_view_status | Indicates whether the search term is one of your targeted or excluded keywords. | QueryTargetingStatus |
| segments_ad_network_type | Ad network type. | AdNetworkType2 |
| segments_date | Date to which metrics apply. yyyy-MM-dd format. For example, 2018-04-17. | Date |
| segments_day_of_week | Day of the week. For example, MONDAY. | DayOfWeek |
| segments_device | Device to which metrics apply. | Device |
| segments_month | Month as represented by the date of the first day of a month. Formatted as yyyy-MM-dd. | Month |
| segments_quarter | Quarter as represented by the date of the first day of a quarter. Uses the calendar year for quarters. For example, the second quarter of 2018 starts on 2018-04-01. Formatted as yyyy-MM-dd. | Quarter |
| segments_search_term_match_type | Match type of the keyword that triggered the ad, including variants. | QueryMatchTypeWithVariant |
| segments_week | Week as defined as Monday through Sunday, and represented by the date of Monday. Formatted as yyyy-MM-dd. | Week |
| segments_year | Year, formatted as yyyy. | Year |

Google Ads Table Name: ShoppingProductConversionStats


Google Ads API Resource:
[shopping_performance_view](https://developers.google.com/google-ads/api/fields/v22/shopping_performance_view)

| Google Ads Field Name | Description | Adwords Mapped Field Name |
|---|---|---|
| ad_group_id | The ID of the ad group. | AdGroupId |
| campaign_id | The ID of the campaign. | CampaignId |
| campaign_status | The status of the campaign. When a new campaign is added, the status defaults to ENABLED. |   |
| customer_id | The ID of the customer. | ExternalCustomerId |
| metrics_all_conversions | The total number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | AllConversions |
| metrics_all_conversions_value | The total value of all conversions. | AllConversionValue |
| metrics_conversions_value | The total value of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | ConversionValue |
| metrics_value_per_all_conversions | The value of all conversions divided by the number of all conversions. | ValuePerAllConversion |
| metrics_value_per_conversion | The value of conversions divided by the number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | ValuePerConversion |
| segments_ad_network_type | Ad network type. | AdNetworkType2 |
| segments_click_type | Click type. | ClickType |
| segments_conversion_action | Resource name of the conversion action. | ConversionTrackerId |
| segments_conversion_action_category | Conversion action category. | ConversionCategoryName |
| segments_conversion_action_name | Conversion action name. | ConversionTypeName |
| segments_date | Date to which metrics apply. yyyy-MM-dd format. For example, 2018-04-17. | Date |
| segments_day_of_week | Day of the week. For example, MONDAY. | DayOfWeek |
| segments_device | Device to which metrics apply. | Device |
| segments_month | Month as represented by the date of the first day of a month. Formatted as yyyy-MM-dd. | Month |
| segments_product_aggregator_id | Aggregator ID of the product. | AggregatorId |
| segments_product_bidding_category_level1 **(obsolete - use segments_product_category_level1 instead)** | Bidding category (level 1) of the product. | CategoryL1 |
| segments_product_bidding_category_level2 **(obsolete - use segments_product_category_level2 instead)** | Bidding category (level 2) of the product. | CategoryL2 |
| segments_product_bidding_category_level3 **(obsolete - use segments_product_category_level3 instead)** | Bidding category (level 3) of the product. | CategoryL3 |
| segments_product_bidding_category_level4 **(obsolete - use segments_product_category_level4 instead)** | Bidding category (level 4) of the product. | CategoryL4 |
| segments_product_bidding_category_level5 **(obsolete - use segments_product_category_level5 instead)** | Bidding category (level 5) of the product. | CategoryL5 |
| segments_product_category_level1 | Category (level 1) of the product. | CategoryL1 |
| segments_product_category_level2 | Category (level 2) of the product. | CategoryL2 |
| segments_product_category_level3 | Category (level 3) of the product. | CategoryL3 |
| segments_product_category_level4 | Category (level 4) of the product. | CategoryL4 |
| segments_product_category_level5 | Category (level 5) of the product. | CategoryL5 |
| segments_product_brand | Brand of the product. | Brand |
| segments_product_channel | Channel of the product. | Channel |
| segments_product_channel_exclusivity | Channel exclusivity of the product. | ChannelExclusivity |
| segments_product_condition | Condition of the product. | ProductCondition |
| segments_product_country | Resource name of the geo target constant for the country of sale of the product. | CountryCriteriaId |
| segments_product_custom_attribute0 | Custom attribute 0 of the product. | CustomAttribute0 |
| segments_product_custom_attribute1 | Custom attribute 1 of the product. | CustomAttribute1 |
| segments_product_custom_attribute2 | Custom attribute 2 of the product. | CustomAttribute2 |
| segments_product_custom_attribute3 | Custom attribute 3 of the product. | CustomAttribute3 |
| segments_product_custom_attribute4 | Custom attribute 4 of the product. | CustomAttribute4 |
| segments_product_item_id | Item ID of the product. | OfferId |
| segments_product_language | Resource name of the language constant for the language of the product. | LanguageCriteriaId |
| segments_product_merchant_id | Merchant ID of the product. | MerchantId |
| segments_product_store_id | Store ID of the product. | StoreId |
| segments_product_type_l1 | Type (level 1) of the product. | ProductTypeL1 |
| segments_product_type_l2 | Type (level 2) of the product. | ProductTypeL2 |
| segments_product_type_l3 | Type (level 3) of the product. | ProductTypeL3 |
| segments_product_type_l4 | Type (level 4) of the product. | ProductTypeL4 |
| segments_product_type_l5 | Type (level 5) of the product. | ProductTypeL5 |
| segments_quarter | Quarter as represented by the date of the first day of a quarter. Uses the calendar year for quarters. For example, the second quarter of 2018 starts on 2018-04-01. Formatted as yyyy-MM-dd. | Quarter |
| segments_week | Week as defined as Monday through Sunday, and represented by the date of Monday. Formatted as yyyy-MM-dd. | Week |
| segments_year | Year, formatted as yyyy. | Year |

Google Ads Table Name: ShoppingProductStats


Google Ads API Resource:
[shopping_performance_view](https://developers.google.com/google-ads/api/fields/v22/shopping_performance_view)

| Google Ads Field Name | Description | Adwords Mapped Field Name |
|---|---|---|
| ad_group_id | The ID of the ad group. | AdGroupId |
| campaign_id | The ID of the campaign. | CampaignId |
| campaign_status | The status of the campaign. When a new campaign is added, the status defaults to ENABLED. |   |
| customer_id | The ID of the customer. | ExternalCustomerId |
| metrics_all_conversions | The total number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | AllConversions |
| metrics_all_conversions_from_interactions_rate | All conversions from interactions (as oppose to view through conversions) divided by the number of ad interactions. | AllConversionRate |
| metrics_all_conversions_value | The total value of all conversions. | AllConversionValue |
| metrics_average_cpc | The total cost of all clicks divided by the total number of clicks received. | AverageCpc |
| metrics_clicks | The number of clicks. | Clicks |
| metrics_conversions | The number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | Conversions |
| metrics_conversions_from_interactions_rate | Conversions from interactions divided by the number of ad interactions (such as clicks for text ads or views for video ads). This only includes conversion actions which include_in_conversions_metric attribute is set to true. | ConversionRate |
| metrics_conversions_value | The total value of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | ConversionValue |
| metrics_cost_micros | The sum of your cost-per-click (CPC) and cost-per-thousand impressions (CPM) costs during this period. | Cost |
| metrics_cost_per_all_conversions | The cost of ad interactions divided by all conversions. | CostPerAllConversion |
| metrics_cost_per_conversion | The cost of ad interactions divided by conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | CostPerConversion |
| metrics_cross_device_conversions | Conversions from when a customer clicks on a Google Ads ad on one device, then converts on a different device or browser. Cross-device conversions are already included in all_conversions. | CrossDeviceConversions |
| metrics_ctr | The number of clicks your ad receives (Clicks) divided by the number of times your ad is shown (Impressions). | Ctr |
| metrics_impressions | Count of how often your ad has appeared on a search results page or website on the Google Network. | Impressions |
| metrics_search_absolute_top_impression_share | The percentage of the customer's Shopping or Search ad impressions that are shown in the most prominent Shopping position. See https://support.google.com/google-ads/answer/7501826 for details. Any value below 0.1 is reported as 0.0999. | SearchAbsoluteTopImpressionShare |
| metrics_search_click_share | The number of clicks you've received on the Search Network divided by the estimated number of clicks you were eligible to receive. Note: Search click share is reported in the range of 0.1 to 1. Any value below 0.1 is reported as 0.0999. | SearchClickShare |
| metrics_search_impression_share | The impressions you've received on the Search Network divided by the estimated number of impressions you were eligible to receive. Note: Search impression share is reported in the range of 0.1 to 1. Any value below 0.1 is reported as 0.0999. | SearchImpressionShare |
| metrics_value_per_all_conversions | The value of all conversions divided by the number of all conversions. | ValuePerAllConversion |
| metrics_value_per_conversion | The value of conversions divided by the number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | ValuePerConversion |
| segments_ad_network_type | Ad network type. | AdNetworkType2 |
| segments_date | Date to which metrics apply. yyyy-MM-dd format. For example, 2018-04-17. | Date |
| segments_day_of_week | Day of the week. For example, MONDAY. | DayOfWeek |
| segments_device | Device to which metrics apply. | Device |
| segments_month | Month as represented by the date of the first day of a month. Formatted as yyyy-MM-dd. | Month |
| segments_product_aggregator_id | Aggregator ID of the product. | AggregatorId |
| segments_product_bidding_category_level1 **(obsolete - use segments_product_category_level1 instead)** | Bidding category (level 1) of the product. | CategoryL1 |
| segments_product_bidding_category_level2 **(obsolete - use segments_product_category_level2 instead)** | Bidding category (level 2) of the product. | CategoryL2 |
| segments_product_bidding_category_level3 **(obsolete - use segments_product_category_level3 instead)** | Bidding category (level 3) of the product. | CategoryL3 |
| segments_product_bidding_category_level4 **(obsolete - use segments_product_category_level4 instead)** | Bidding category (level 4) of the product. | CategoryL4 |
| segments_product_bidding_category_level5 **(obsolete - use segments_product_category_level5 instead)** | Bidding category (level 5) of the product. | CategoryL5 |
| segments_product_category_level1 | Category (level 1) of the product. | CategoryL1 |
| segments_product_category_level2 | Category (level 2) of the product. | CategoryL2 |
| segments_product_category_level3 | Category (level 3) of the product. | CategoryL3 |
| segments_product_category_level4 | Category (level 4) of the product. | CategoryL4 |
| segments_product_category_level5 | Category (level 5) of the product. | CategoryL5 |
| segments_product_brand | Brand of the product. | Brand |
| segments_product_channel | Channel of the product. | Channel |
| segments_product_channel_exclusivity | Channel exclusivity of the product. | ChannelExclusivity |
| segments_product_condition | Condition of the product. | ProductCondition |
| segments_product_country | Resource name of the geo target constant for the country of sale of the product. | CountryCriteriaId |
| segments_product_custom_attribute0 | Custom attribute 0 of the product. | CustomAttribute0 |
| segments_product_custom_attribute1 | Custom attribute 1 of the product. | CustomAttribute1 |
| segments_product_custom_attribute2 | Custom attribute 2 of the product. | CustomAttribute2 |
| segments_product_custom_attribute3 | Custom attribute 3 of the product. | CustomAttribute3 |
| segments_product_custom_attribute4 | Custom attribute 4 of the product. | CustomAttribute4 |
| segments_product_item_id | Item ID of the product. | OfferId |
| segments_product_language | Resource name of the language constant for the language of the product. | LanguageCriteriaId |
| segments_product_merchant_id | Merchant ID of the product. | MerchantId |
| segments_product_type_l1 | Type (level 1) of the product. | ProductTypeL1 |
| segments_product_type_l2 | Type (level 2) of the product. | ProductTypeL2 |
| segments_product_type_l3 | Type (level 3) of the product. | ProductTypeL3 |
| segments_product_type_l4 | Type (level 4) of the product. | ProductTypeL4 |
| segments_product_type_l5 | Type (level 5) of the product. | ProductTypeL5 |
| segments_quarter | Quarter as represented by the date of the first day of a quarter. Uses the calendar year for quarters. For example, the second quarter of 2018 starts on 2018-04-01. Formatted as yyyy-MM-dd. | Quarter |
| segments_week | Week as defined as Monday through Sunday, and represented by the date of Monday. Formatted as yyyy-MM-dd. | Week |
| segments_year | Year, formatted as yyyy. | Year |

Google Ads Table Name: Video


Google Ads API Resource:
[video](https://developers.google.com/google-ads/api/fields/v22/video)

| Google Ads Field Name | Description | Adwords Mapped Field Name |
|---|---|---|
| ad_group_ad_status | The status of the ad. |   |
| ad_group_id | The ID of the ad group. | AdGroupId |
| campaign_id | The ID of the campaign. | CampaignId |
| customer_id | The ID of the customer. | ExternalCustomerId |
| video_duration_millis | The duration of the video in milliseconds. | VideoDuration |
| video_id | The ID of the video. | VideoId |
| video_title | The title of the video. | VideoTitle |

Google Ads Table Name: VideoBasicStats


Google Ads API Resource:
[video](https://developers.google.com/google-ads/api/fields/v22/video)

| Google Ads Field Name | Description | Adwords Mapped Field Name |
|---|---|---|
| ad_group_ad_ad_id | The ID of the ad. | CreativeId |
| ad_group_ad_status | The status of the ad. | CreativeStatus |
| ad_group_id | The ID of the ad group. | AdGroupId |
| campaign_id | The ID of the campaign. | CampaignId |
| customer_id | The ID of the customer. | ExternalCustomerId |
| metrics_clicks | The number of clicks. | Clicks |
| metrics_conversions | The number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | Conversions |
| metrics_conversions_value | The total value of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | ConversionValue |
| metrics_cost_micros | The sum of your cost-per-click (CPC) and cost-per-thousand impressions (CPM) costs during this period. | Cost |
| metrics_impressions | Count of how often your ad has appeared on a search results page or website on the Google Network. | Impressions |
| metrics_view_through_conversions | The total number of view-through conversions. These happen when a customer sees an image or rich media ad, then later completes a conversion on your site without interacting with (for example, clicking on) another ad. | ViewThroughConversions |
| segments_ad_network_type | Ad network type. | AdNetworkType2 |
| segments_date | Date to which metrics apply. yyyy-MM-dd format. For example, 2018-04-17. | Date |
| segments_device | Device to which metrics apply. | Device |
| video_channel_id | The owner channel id of the video. | VideoChannelId |
| video_id | The ID of the video. | VideoId |

Google Ads Table Name: VideoConversionStats


Google Ads API Resource:
[video](https://developers.google.com/google-ads/api/fields/v22/video)

| Google Ads Field Name | Description | Adwords Mapped Field Name |
|---|---|---|
| ad_group_ad_ad_id | The ID of the ad. | CreativeId |
| ad_group_ad_status | The status of the ad. | CreativeStatus |
| ad_group_id | The ID of the ad group. | AdGroupId |
| campaign_id | The ID of the campaign. | CampaignId |
| customer_id | The ID of the customer. | ExternalCustomerId |
| metrics_conversions | The number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | Conversions |
| metrics_conversions_value | The total value of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | ConversionValue |
| segments_ad_network_type | Ad network type. | AdNetworkType2 |
| segments_click_type | Click type. | ClickType |
| segments_conversion_action | Resource name of the conversion action. | ConversionTrackerId |
| segments_conversion_action_category | Conversion action category. | ConversionCategoryName |
| segments_conversion_action_name | Conversion action name. | ConversionTypeName |
| segments_date | Date to which metrics apply. yyyy-MM-dd format. For example, 2018-04-17. | Date |
| segments_day_of_week | Day of the week. For example, MONDAY. | DayOfWeek |
| segments_device | Device to which metrics apply. | Device |
| segments_month | Month as represented by the date of the first day of a month. Formatted as yyyy-MM-dd. | Month |
| segments_quarter | Quarter as represented by the date of the first day of a quarter. Uses the calendar year for quarters. For example, the second quarter of 2018 starts on 2018-04-01. Formatted as yyyy-MM-dd. | Quarter |
| segments_week | Week as defined as Monday through Sunday, and represented by the date of Monday. Formatted as yyyy-MM-dd. | Week |
| segments_year | Year, formatted as yyyy. | Year |
| video_channel_id | The owner channel id of the video. | VideoChannelId |
| video_id | The ID of the video. | VideoId |

Google Ads Table Name: VideoNonClickStats


Google Ads API Resource:
[video](https://developers.google.com/google-ads/api/fields/v22/video)

| Google Ads Field Name | Description | Adwords Mapped Field Name |
|---|---|---|
| ad_group_ad_ad_id | The ID of the ad. | CreativeId |
| ad_group_ad_status | The status of the ad. |   |
| ad_group_id | The ID of the ad group. | AdGroupId |
| campaign_id | The ID of the campaign. | CampaignId |
| customer_id | The ID of the customer. | ExternalCustomerId |
| metrics_all_conversions | The total number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | AllConversions |
| metrics_all_conversions_from_interactions_rate | All conversions from interactions (as oppose to view through conversions) divided by the number of ad interactions. | AllConversionRate |
| metrics_all_conversions_value | The total value of all conversions. | AllConversionValue |
| metrics_trueview_average_cpv | The average amount you pay each time someone views your ad. The average CPV is defined by the total cost of all ad views divided by the number of views. | AverageCpv |
| metrics_cost_per_all_conversions | The cost of ad interactions divided by all conversions. | CostPerAllConversion |
| metrics_cross_device_conversions | Conversions from when a customer clicks on a Google Ads ad on one device, then converts on a different device or browser. Cross-device conversions are already included in all_conversions. | CrossDeviceConversions |
| metrics_engagement_rate | How often people engage with your ad after it's shown to them. This is the number of ad expansions divided by the number of times your ad is shown. | EngagementRate |
| metrics_engagements | The number of engagements. An engagement occurs when a viewer expands your Lightbox ad. Also, in the future, other ad types may support engagement metrics. | Engagements |
| metrics_value_per_all_conversions | The value of all conversions divided by the number of all conversions. | ValuePerAllConversion |
| metrics_video_quartile_p100_rate | Percentage of impressions where the viewer watched all of your video. | VideoQuartile100Rate |
| metrics_video_quartile_p25_rate | Percentage of impressions where the viewer watched 25% of your video. | VideoQuartile25Rate |
| metrics_video_quartile_p50_rate | Percentage of impressions where the viewer watched 50% of your video. | VideoQuartile50Rate |
| metrics_video_quartile_p75_rate | Percentage of impressions where the viewer watched 75% of your video. | VideoQuartile75Rate |
| metrics_video_trueview_view_rate | The number of views your TrueView video ad receives divided by its number of impressions, including thumbnail impressions for TrueView in-display ads. | VideoViewRate |
| metrics_video_trueview_views | The number of times your video ads were viewed. | VideoViews |
| segments_ad_network_type | Ad network type. | AdNetworkType2 |
| segments_date | Date to which metrics apply. yyyy-MM-dd format. For example, 2018-04-17. | Date |
| segments_day_of_week | Day of the week. For example, MONDAY. | DayOfWeek |
| segments_device | Device to which metrics apply. | Device |
| segments_month | Month as represented by the date of the first day of a month. Formatted as yyyy-MM-dd. | Month |
| segments_quarter | Quarter as represented by the date of the first day of a quarter. Uses the calendar year for quarters. For example, the second quarter of 2018 starts on 2018-04-01. Formatted as yyyy-MM-dd. | Quarter |
| segments_week | Week as defined as Monday through Sunday, and represented by the date of Monday. Formatted as yyyy-MM-dd. | Week |
| segments_year | Year, formatted as yyyy. | Year |
| video_channel_id | The owner channel id of the video. | VideoChannelId |
| video_id | The ID of the video. | VideoId |

Google Ads Table Name: VideoStats


Google Ads API Resource:
[video](https://developers.google.com/google-ads/api/fields/v22/video)

| Google Ads Field Name | Description | Adwords Mapped Field Name |
|---|---|---|
| ad_group_ad_ad_id | The ID of the ad. | CreativeId |
| ad_group_ad_status | The status of the ad. | CreativeStatus |
| ad_group_id | The ID of the ad group. | AdGroupId |
| campaign_id | The ID of the campaign. | CampaignId |
| customer_id | The ID of the customer. | ExternalCustomerId |
| metrics_all_conversions_from_interactions_rate | All conversions from interactions (as oppose to view through conversions) divided by the number of ad interactions. | ConversionRate |
| metrics_average_cpm | Average cost-per-thousand impressions (CPM). | AverageCpm |
| metrics_clicks | The number of clicks. | Clicks |
| metrics_conversions | The number of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | Conversions |
| metrics_conversions_value | The total value of conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | ConversionValue |
| metrics_cost_micros | The sum of your cost-per-click (CPC) and cost-per-thousand impressions (CPM) costs during this period. | Cost |
| metrics_cost_per_conversion | The cost of ad interactions divided by conversions. This only includes conversion actions which include_in_conversions_metric attribute is set to true. | CostPerConversion |
| metrics_ctr | The number of clicks your ad receives (Clicks) divided by the number of times your ad is shown (Impressions). | Ctr |
| metrics_impressions | Count of how often your ad has appeared on a search results page or website on the Google Network. | Impressions |
| segments_ad_network_type | Ad network type. | AdNetworkType2 |
| segments_click_type | Click type. | ClickType |
| segments_date | Date to which metrics apply. yyyy-MM-dd format. For example, 2018-04-17. | Date |
| segments_day_of_week | Day of the week. For example, MONDAY. | DayOfWeek |
| segments_device | Device to which metrics apply. | Device |
| segments_month | Month as represented by the date of the first day of a month. Formatted as yyyy-MM-dd. | Month |
| segments_quarter | Quarter as represented by the date of the first day of a quarter. Uses the calendar year for quarters. For example, the second quarter of 2018 starts on 2018-04-01. Formatted as yyyy-MM-dd. | Quarter |
| segments_week | Week as defined as Monday through Sunday, and represented by the date of Monday. Formatted as yyyy-MM-dd. | Week |
| segments_year | Year, formatted as yyyy. | Year |
| video_channel_id | The owner channel id of the video. | VideoChannelId |
| video_id | The ID of the video. | VideoId |

## Google Ads Match Tables

Google Ads Match Tables are tables that contain only **Attribute**
fields (fields containing settings or other fixed data), and they are defined
for users to query account structure information. If you use the refresh window
or schedule a backfill, Match Table snapshots are not updated.

Below is a list of Match Tables in Google Ads transfer:

- Ad
- AdGroup
- AdGroupAudience
- AdGroupBidModifier
- AdGroupAdLabel
- AdGroupCriterion
- AdGroupCriterionLabel
- AdGroupLabel
- AgeRange
- Asset
- AssetGroup
- AssetGroupAsset
- AssetGroupListingGroupFilter
- AssetGroupSignal
- Audience
- BidGoal
- Budget
- Campaign
- CampaignAudience
- CampaignCriterion
- CampaignLabel
- Customer
- Gender
- Keyword
- LocationBasedCampaignCriterion
- ParentalStatus
- Placement
- Video