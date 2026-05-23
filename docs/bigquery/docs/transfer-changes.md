# BigQuery Data Transfer Service data source change log

This page provides details about changes to BigQuery Data Transfer Service data source
schemas and schema mappings. For information about upcoming changes to the BigQuery Data Transfer Service
connectors, you can search this page for data sources, such as
`Google Ads API` or `Display & Video 360 API`, or for specific table names or
values.

## Campaign Manager 360

The BigQuery Data Transfer Service for Campaign Manager 360 connector periodically updates to support new, deprecated, or migrated columns.
The BigQuery Data Transfer Service for Campaign Manager 360 connector
retrieves data from Campaign Manager 360 [Data Transfer files](https://developers.google.com/doubleclick-advertisers/dtv2/reference/release-notes).

The following sections outline the changes. Changes are organized by release date,
and each entry provides information on the changes you need to make to
continue receiving data from Campaign Manager 360.

### July 07, 2025

Campaign Manager 360 made an [announcement](https://support.google.com/campaignmanager/answer/16320235?hl#111) to update its criterion IDs for browser, operating system, mobile make and model, and ISP data to align with the cross-platform data standards. After the migration, Campaign Manager 360 will stop populating values for deprecated columns and will start populating the new columns. The impacted columns are as follows:

| Deprecated columns | New columns |
|---|---|
| `DBM_Browser_Platform_ID` | `DV360_Browser_Platform_Reportable_ID` |
| `DBM_ISP_ID` | `DV360_ISP_Reportable_ID` |
| `DBM_Operating_System_ID` | `DV360_Operating_System_Reportable_ID` |
| `DBM_Mobile_Make_ID` | `DV360_Mobile_Make_Reportable_ID` |
| `DBM_Mobile_Model_ID` | `DV360_Mobile_Model_Reportable_ID` |

## Display \& Video 360 API

The BigQuery Data Transfer Service for Display \& Video 360 connector periodically updates to support
new columns and adapt to changes introduced by new [Display \& Video 360
API](https://developers.google.com/display-video/api/release-notes) versions.
The BigQuery Data Transfer Service for Display \& Video 360 connector uses the
supported API version to retrieve [configuration data](https://docs.cloud.google.com/bigquery/docs/display-video-transfer#supported_configuration_data).

The following sections outline the changes when updating to a new
Display \& Video 360 API version. Changes are organized by release date,
and each entry provides information on the changes you need to make for you to
continue receiving data from Display \& Video 360.

### August 26, 2025

[The Display \& Video 360 connector](https://docs.cloud.google.com/bigquery/docs/display-video-transfer)
plans to update the [Display \& Video 360 API version](https://developers.google.com/display-video/api/release-notes)
used to retrieve configuration data from [v3](https://developers.google.com/display-video/api/reference/rest/v3) to
[v4](https://developers.google.com/display-video/api/reference/rest/v4).
Changes from the API upgrade are listed in the following section. For more information, see
[Display \& Video 360 API v3 to v4 migration guide](https://developers.google.com/display-video/api/v4-migration-guide).

This update for the Display \& Video 360 connector is planned to start on August 26, 2025.

#### Deprecated Tables

The following tables will stop receiving new data. Existing data will remain,
but no further updates will be populated.

- `CampaignTargeting`
- `InsertionOrderTargeting`

#### Tables with renamed columns

| Tables affected | Deprecated columns | New columns |
|---|---|---|
| - `AdGroupTargeting` - `LineItemTargeting` | `audienceGroupDetails.includedFirstAndThirdPartyAudienceGroups` | `audienceGroupDetails.includedFirstPartyAndPartnerAudienceGroups` |
| - `AdGroupTargeting` - `LineItemTargeting` | `audienceGroupDetails.includedFirstAndThirdPartyAudienceGroups.settings` | `audienceGroupDetails.includedFirstPartyAndPartnerAudienceGroups.settings` |
| - `AdGroupTargeting` - `LineItemTargeting` | `audienceGroupDetails.includedFirstAndThirdPartyAudienceGroups.settings.firstAndThirdPartyAudienceId` | `audienceGroupDetails.includedFirstPartyAndPartnerAudienceGroups.settings.firstPartyAndPartnerAudienceId` |
| - `AdGroupTargeting` - `LineItemTargeting` | `audienceGroupDetails.includedFirstAndThirdPartyAudienceGroups.settings.recency` | `audienceGroupDetails.includedFirstPartyAndPartnerAudienceGroups.settings.recency` |
| - `AdGroupTargeting` - `LineItemTargeting` | `audienceGroupDetails.excludedFirstAndThirdPartyAudienceGroup` | `audienceGroupDetails.excludedFirstPartyAndPartnerAudienceGroup` |
| - `AdGroupTargeting` - `LineItemTargeting` | `audienceGroupDetails.excludedFirstAndThirdPartyAudienceGroup.settings` | `audienceGroupDetails.excludedFirstPartyAndPartnerAudienceGroup.settings` |
| - `AdGroupTargeting` - `LineItemTargeting` | `audienceGroupDetails.excludedFirstAndThirdPartyAudienceGroup.settings.firstAndThirdPartyAudienceId` | `audienceGroupDetails.excludedFirstPartyAndPartnerAudienceGroup.settings.firstPartyAndPartnerAudienceId` |
| - `AdGroupTargeting` - `LineItemTargeting` | `audienceGroupDetails.excludedFirstAndThirdPartyAudienceGroup.settings.recency` | `audienceGroupDetails.excludedFirstPartyAndPartnerAudienceGroup.settings.recency` |

#### Tables with deprecated columns

| Tables affected | Deprecated columns |
|---|---|
| `Creative` | `reviewStatus.publisherReviewStatuses` |

## Facebook Ads

The BigQuery Data Transfer Service for Facebook Ads connector periodically
updates to adapt to new changes introduced by
Facebook Ads.

The following sections outline the changes organized by release date.

### July 25, 2026

On July 25, 2026, the [Facebook Ads
connector](https://docs.cloud.google.com/bigquery/docs/facebook-ads-transfer) plans to update its data type
mapping for the `ActionValue` field in the `AdInsightsActions` report from `INT`
to `FLOAT`. This change is made to more accurately reflect the source data and
to ensure data integrity.

## Google Ads API

The BigQuery Data Transfer Service for Google Ads periodically updates to support
new columns and adapt to changes introduced by the [Google Ads
API](https://developers.google.com/google-ads/api/docs/release-notes).
The BigQuery Data Transfer Service for Google Ads connector uses the
[supported API version](https://docs.cloud.google.com/bigquery/docs/google-ads-transfer#connector_overview)
in the Google Ads connector.

The following sections outline the changes introduced by the
Google Ads API. Changes are organized by release date,
and each entry provides information on the changes you need to make for you to
continue receiving data from Google Ads.

For more information about the Google Ads API release schedule,
see [Timetable](https://developers.google.com/google-ads/api/docs/sunset-dates#timetable).

### June 15, 2026

The [Google Ads connector](https://docs.cloud.google.com/bigquery/docs/google-ads-transfer)
plans to update the [Google Ads API version](https://developers.google.com/google-ads/api/docs/release-notes)
from [v22](https://developers.google.com/google-ads/api/fields/v22/overview) to
[v23](https://developers.google.com/google-ads/api/fields/v23/overview).
After the API upgrade, the column values for newly transferred data in the affected
tables will change. For more information, see
[Google Ads API upgrade](https://developers.google.com/google-ads/api/docs/upgrade#v22-v23).

| Deprecated columns | New columns | Tables affected |
|---|---|---|
| `campaign_start_date` | `campaign_start_date_time` | `Campaign` |
| `campaign_end_date` | `campaign_end_date_time` | `Campaign` |

By April 3, 2026, the Google Ads connector will add the columns `campaign_start_date_time` and `campaign_end_date_time` to the table schema and populate them with `null`. After the update to Google Ads API v23 on June 15, 2026, these new columns will be populated with new values and new data type [datetime](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#datetime_type). `campaign_start_date` and `campaign_end_date` will be deprecated and populated with `null`, but will still remain in the table schema.

For each pair of columns, only one column is populated with values from the Google Ads API while the other is populated with `null`. To prepare for the Google Ads API v23 update, update your queries to specify one of the two columns. If your SQL query selects the deprecated columns, update the query so that it specifies the correct column, for example:

```googlesql
IFNULL(DATE(campaign_start_date_time), campaign_start_date)
```

### June 8, 2026

The following column will be deprecated on June 8, 2026. The column will be populated with `null` for new data transferred.

| Deprecated columns | Tables affected |
|---|---|
| `ad_group_ad_ad_call_ad_phone_number` | `Ad` |

### June 1, 2026

Effective June 1, 2026, the Google Ads connector will limit daily data backfills to the most recent 37 months. Backfill attempts for dates older than 37 months will result in an error. This change is in response to the [New Data Retention Policy for Google Ads starting June 1, 2026](https://ads-developers.googleblog.com/2026/05/new-data-retention-policy-for-google.html).

Any data already transferred and stored in your BigQuery tables will remain unaffected.

### May 7, 2026

Starting May 7, 2026, Google Ads will require [Multi-factor authentication (MFA) for individual user authentication](https://ads-developers.googleblog.com/2026/04/multi-factor-authentication-requirement.html).
Existing transfer configurations and transfer runs aren't impacted. If you want to create new transfer configurations that are authorized with individual user credentials, you must enable [2-step verification](https://support.google.com/accounts/answer/185839). 2-step verification is not required for new transfer configurations that are authorized with service accounts.

### March 2, 2026

The [Google Ads connector](https://docs.cloud.google.com/bigquery/docs/google-ads-transfer)
plans to update the [Google Ads API version](https://developers.google.com/google-ads/api/docs/release-notes)
from [v21](https://developers.google.com/google-ads/api/fields/v21/overview) to
[v22](https://developers.google.com/google-ads/api/fields/v22/overview).
After the API upgrade, the column values for newly transferred data in the affected
tables will change. For more information, see
[Google Ads API upgrade](https://developers.google.com/google-ads/api/docs/upgrade#v21-v22).

| Deprecated columns | New columns | Tables affected |
|---|---|---|---|---|
| `metrics_average_cpv` | `metrics_trueview_average_cpv` | - `AccountNonClickStats` - `AdCrossDeviceStats` - `AdGroupAudienceNonClickStats` - `AdGroupCrossDeviceStats` - `AgeRangeNonClickStats` - `BudgetStats` - `CampaignAssetStats` - `CampaignAudienceNonClickStats` - `CampaignCrossDeviceStats` - `CampaignLocationTargetStats` - `DisplayVideoAutomaticPlacementsStats` - `DisplayVideoKeywordStats` - `GenderNonClickStats` - `GeoStats` - `KeywordCrossDeviceStats` - `ParentalStatusNonClickStats` - `PlacementNonClickStats` - `SearchQueryStats` - `VideoNonClickStats` |
| `metrics_video_view_rate` | `metrics_video_trueview_view_rate` | - `AccountNonClickStats` - `AdCrossDeviceStats` - `AdGroupAudienceNonClickStats` - `AdGroupCrossDeviceStats` - `AgeRangeNonClickStats` - `BudgetStats` - `CampaignAssetStats` - `CampaignAudienceNonClickStats` - `CampaignCrossDeviceStats` - `CampaignLocationTargetStats` - `DisplayVideoAutomaticPlacementsStats` - `DisplayVideoKeywordStats` - `GenderNonClickStats` - `GeoStats` - `KeywordCrossDeviceStats` - `ParentalStatusNonClickStats` - `PlacementNonClickStats` - `SearchQueryStats` - `VideoNonClickStats` | - `AccountNonClickStats` - `AdCrossDeviceStats` - `AdGroupAudienceNonClickStats` - `AdGroupCrossDeviceStats` - `AgeRangeNonClickStats` - `BudgetStats` - `CampaignAudienceNonClickStats` - `CampaignCrossDeviceStats` - `CampaignLocationTargetStats` - `DisplayVideoAutomaticPlacementsStats` - `GenderNonClickStats` - `GeoStats` - `KeywordCrossDeviceStats` - `ParentalStatusNonClickStats` - `PlacementNonClickStats` - `SearchQueryStats` - `VideoNonClickStats` |
| `metrics_video_views` | `metrics_video_trueview_views` | - `AccountNonClickStats` - `AdCrossDeviceStats` - `AdGroupAudienceNonClickStats` - `AdGroupCrossDeviceStats` - `AgeRangeNonClickStats` - `BudgetStats` - `CampaignAssetStats` - `CampaignAudienceNonClickStats` - `CampaignCrossDeviceStats` - `CampaignLocationTargetStats` - `DisplayVideoAutomaticPlacementsStats` - `DisplayVideoKeywordStats` - `GenderNonClickStats` - `GeoStats` - `KeywordCrossDeviceStats` - `ParentalStatusNonClickStats` - `PlacementNonClickStats` - `SearchQueryStats` - `VideoNonClickStats` | - `AccountNonClickStats` - `AdCrossDeviceStats` - `AdGroupAudienceNonClickStats` - `AdGroupCrossDeviceStats` - `AgeRangeNonClickStats` - `BudgetStats` - `CampaignAudienceNonClickStats` - `CampaignCrossDeviceStats` - `CampaignLocationTargetStats` - `DisplayVideoAutomaticPlacementsStats` - `GenderNonClickStats` - `GeoStats` - `KeywordCrossDeviceStats` - `ParentalStatusNonClickStats` - `PlacementNonClickStats` - `SearchQueryStats` - `VideoNonClickStats` | - `AccountNonClickStats` - `AdCrossDeviceStats` - `AdGroupAudienceNonClickStats` - `AdGroupCrossDeviceStats` - `AgeRangeNonClickStats` - `BudgetStats` - `CampaignAudienceNonClickStats` - `CampaignCrossDeviceStats` - `CampaignLocationTargetStats` - `DisplayVideoAutomaticPlacementsStats` - `GenderNonClickStats` - `GeoStats` - `KeywordCrossDeviceStats` - `ParentalStatusNonClickStats` - `PlacementNonClickStats` - `SearchQueryStats` - `VideoNonClickStats` |
| - `AccountNonClickStats` - `AdCrossDeviceStats` - `AdGroupAudienceNonClickStats` - `AdGroupCrossDeviceStats` - `AgeRangeNonClickStats` - `BudgetStats` - `CampaignAssetStats` - `CampaignAudienceNonClickStats` - `CampaignCrossDeviceStats` - `CampaignLocationTargetStats` - `DisplayVideoAutomaticPlacementsStats` - `DisplayVideoKeywordStats` - `GenderNonClickStats` - `GeoStats` - `KeywordCrossDeviceStats` - `ParentalStatusNonClickStats` - `PlacementNonClickStats` - `SearchQueryStats` - `VideoNonClickStats` | - `AccountNonClickStats` - `AdCrossDeviceStats` - `AdGroupAudienceNonClickStats` - `AdGroupCrossDeviceStats` - `AgeRangeNonClickStats` - `BudgetStats` - `CampaignAudienceNonClickStats` - `CampaignCrossDeviceStats` - `CampaignLocationTargetStats` - `DisplayVideoAutomaticPlacementsStats` - `GenderNonClickStats` - `GeoStats` - `KeywordCrossDeviceStats` - `ParentalStatusNonClickStats` - `PlacementNonClickStats` - `SearchQueryStats` - `VideoNonClickStats` | - `AccountNonClickStats` - `AdCrossDeviceStats` - `AdGroupAudienceNonClickStats` - `AdGroupCrossDeviceStats` - `AgeRangeNonClickStats` - `BudgetStats` - `CampaignAudienceNonClickStats` - `CampaignCrossDeviceStats` - `CampaignLocationTargetStats` - `DisplayVideoAutomaticPlacementsStats` - `GenderNonClickStats` - `GeoStats` - `KeywordCrossDeviceStats` - `ParentalStatusNonClickStats` - `PlacementNonClickStats` - `SearchQueryStats` - `VideoNonClickStats` |
| - `AccountNonClickStats` - `AdCrossDeviceStats` - `AdGroupAudienceNonClickStats` - `AdGroupCrossDeviceStats` - `AgeRangeNonClickStats` - `BudgetStats` - `CampaignAssetStats` - `CampaignAudienceNonClickStats` - `CampaignCrossDeviceStats` - `CampaignLocationTargetStats` - `DisplayVideoAutomaticPlacementsStats` - `DisplayVideoKeywordStats` - `GenderNonClickStats` - `GeoStats` - `KeywordCrossDeviceStats` - `ParentalStatusNonClickStats` - `PlacementNonClickStats` - `SearchQueryStats` - `VideoNonClickStats` | - `AccountNonClickStats` - `AdCrossDeviceStats` - `AdGroupAudienceNonClickStats` - `AdGroupCrossDeviceStats` - `AgeRangeNonClickStats` - `BudgetStats` - `CampaignAudienceNonClickStats` - `CampaignCrossDeviceStats` - `CampaignLocationTargetStats` - `DisplayVideoAutomaticPlacementsStats` - `GenderNonClickStats` - `GeoStats` - `KeywordCrossDeviceStats` - `ParentalStatusNonClickStats` - `PlacementNonClickStats` - `SearchQueryStats` - `VideoNonClickStats` | - `AccountNonClickStats` - `AdCrossDeviceStats` - `AdGroupAudienceNonClickStats` - `AdGroupCrossDeviceStats` - `AgeRangeNonClickStats` - `BudgetStats` - `CampaignAudienceNonClickStats` - `CampaignCrossDeviceStats` - `CampaignLocationTargetStats` - `DisplayVideoAutomaticPlacementsStats` - `GenderNonClickStats` - `GeoStats` - `KeywordCrossDeviceStats` - `ParentalStatusNonClickStats` - `PlacementNonClickStats` - `SearchQueryStats` - `VideoNonClickStats` |
| - `AccountNonClickStats` - `AdCrossDeviceStats` - `AdGroupAudienceNonClickStats` - `AdGroupCrossDeviceStats` - `AgeRangeNonClickStats` - `BudgetStats` - `CampaignAudienceNonClickStats` - `CampaignCrossDeviceStats` - `CampaignLocationTargetStats` - `DisplayVideoAutomaticPlacementsStats` - `GenderNonClickStats` - `GeoStats` - `KeywordCrossDeviceStats` - `ParentalStatusNonClickStats` - `PlacementNonClickStats` - `SearchQueryStats` - `VideoNonClickStats` | - `AccountNonClickStats` - `AdCrossDeviceStats` - `AdGroupAudienceNonClickStats` - `AdGroupCrossDeviceStats` - `AgeRangeNonClickStats` - `BudgetStats` - `CampaignAudienceNonClickStats` - `CampaignCrossDeviceStats` - `CampaignLocationTargetStats` - `DisplayVideoAutomaticPlacementsStats` - `GenderNonClickStats` - `GeoStats` - `KeywordCrossDeviceStats` - `ParentalStatusNonClickStats` - `PlacementNonClickStats` - `SearchQueryStats` - `VideoNonClickStats` |
| - `AccountNonClickStats` - `AdCrossDeviceStats` - `AdGroupAudienceNonClickStats` - `AdGroupCrossDeviceStats` - `AgeRangeNonClickStats` - `BudgetStats` - `CampaignAudienceNonClickStats` - `CampaignCrossDeviceStats` - `CampaignLocationTargetStats` - `DisplayVideoAutomaticPlacementsStats` - `GenderNonClickStats` - `GeoStats` - `KeywordCrossDeviceStats` - `ParentalStatusNonClickStats` - `PlacementNonClickStats` - `SearchQueryStats` - `VideoNonClickStats` |

By Jan 16, 2026, the Google Ads connector will add the columns `metrics_trueview_average_cpv`, `metrics_video_trueview_view_rate` and `metrics_video_trueview_views` to the table schema and populate them with `null`. After the update to Google Ads API v22 on March 2, 2026, these new columns will be populated with new values. Some columns are now deprecated, such as `metrics_average_cpv`, `metrics_video_view_rate` and `metrics_video_views`. Deprecated columns are now populated with `null`, but will still remain in the table schema.

For each pair of columns, only one column is populated with values from the Google Ads API while the other is populated with `null`. In response to the Google Ads API v22 update, update your queries to specify one of the two columns. For example, if your SQL query selects the column `metrics_average_cpv`, update the query so that it specifies the correct column:

```googlesql
IFNULL(metrics_average_cpv, metrics_trueview_average_cpv)
```

If you use [custom reports](https://docs.cloud.google.com/bigquery/docs/google-ads-transfer#custom_reports), see the [Google Ads API v22 reference page](https://developers.google.com/google-ads/api/fields/v22/overview) and the [Google Ads API release notes](https://developers.google.com/google-ads/api/docs/release-notes) to update impacted GAQL queries after the [Google Ads connector](https://docs.cloud.google.com/bigquery/docs/google-ads-transfer) is upgraded to Google Ads v22 API.
If you use [custom reports](https://docs.cloud.google.com/bigquery/docs/google-ads-transfer#custom_reports), see the [Google Ads API v22 reference page](https://developers.google.com/google-ads/api/fields/v22/overview) and the [Google Ads API release notes](https://developers.google.com/google-ads/api/docs/release-notes) to update impacted GAQL queries after the [Google Ads connector](https://docs.cloud.google.com/bigquery/docs/google-ads-transfer) is upgraded to Google Ads v22 API.

### August 1, 2025

[Google Ads transfers](https://docs.cloud.google.com/bigquery/docs/google-ads-transfer)
plans to update the [Google Ads API version](https://developers.google.com/google-ads/api/docs/release-notes)
from [v18](https://developers.google.com/google-ads/api/reference/rpc/v18/overview) to
[v20](https://developers.google.com/google-ads/api/reference/rpc/v20/overview).
After the API upgrade, the column values for newly transferred data in the affected
tables will change. For more information, see
[Google Ads API upgrade](https://developers.google.com/google-ads/api/docs/upgrade#v18-v19).

#### Table: `p_ads_Ad_customer_id`

| Columns impacted | Deprecated data type |
|---|---|
| ad_group_type | VIDEO_OUTSTREAM |
| ad_group_ad_ad_type | VIDEO_OUTSTREAM |

#### Table: `p_ads_Campaign_customer_id`

| Columns impacted | Deprecated data type |
|---|---|
| campaign_advertising_channel_sub_type | VIDEO_OUTSTREAM |

#### Table: `p_ads_DisplayVideoKeywordStats_customer_id`

| Columns impacted | Deprecated data type |
|---|---|
| campaign_advertising_channel_sub_type | VIDEO_OUTSTREAM |

### January 20, 2025

[Google Ads transfers](https://docs.cloud.google.com/bigquery/docs/google-ads-transfer)
plans to update the [Google Ads API version](https://developers.google.com/google-ads/api/docs/release-notes)
from [v16](https://developers.google.com/google-ads/api/reference/rpc/v16/overview) to
[v18](https://developers.google.com/google-ads/api/reference/rpc/v18/overview).
After the API upgrade, the column values for newly transferred data in the affected
tables will change. For more information, see
[Google Ads API upgrade](https://developers.google.com/google-ads/api/docs/upgrade#v17-v18).

This update for the Google Ads connector started on January 20, 2025, and was completed on February 4, 2025.

#### Table: `p_ads_Campaign_customer_id`

| Columns impacted | Old value (v16) | New value (v18) |
|---|---|---|
| campaign_advertising_channel_type | DISCOVERY | DEMAND_GEN |

#### Table: `p_ads_Ad_customer_id`

| Columns impacted | Old value (v16) | New value (v18) |
|---|---|---|
| ad_type | DISCOVERY_MULTI_ASSET_AD DISCOVERY_CAROUSEL_AD DISCOVERY_VIDEO_RESPONSIVE_AD | DEMAND_GEN_MULTI_ASSET_AD DEMAND_GEN_CAROUSEL_AD DEMAND_GEN_VIDEO_RESPONSIVE_AD |

#### Table: `Asset`

| Columns impacted | Old value (v16) | New value (v18) |
|---|---|---|
| asset_type | DISCOVERY_CAROUSEL_CARD | DEMAND_GEN_CAROUSEL_CARD |

To ensure your queries work after the update, change your queries to
select both old and new values. For example, if you have the following `WHERE`
condition in your SQL query:

```googlesql
WHERE asset_type='DISCOVERY_CAROUSEL_CARD'
```

Replace with the following statement:

```googlesql
WHERE
  asset_type='DISCOVERY_CAROUSEL_CARD'
  OR asset_type='DEMAND_GEN_CAROUSEL_CARD'
```

### June 24, 2024

[Google Ads transfers](https://docs.cloud.google.com/bigquery/docs/google-ads-transfer)
plans to update the [Google Ads API version](https://developers.google.com/google-ads/api/docs/release-notes)
from v14 to
[v16](https://developers.google.com/google-ads/api/reference/rpc/v16/overview).
In this API upgrade, the column names for newly transferred data in the affected
tables are changed. Also, some columns are deprecated. For more information, see
[Google Ads API upgrade](https://developers.google.com/google-ads/api/docs/upgrade#v17-v18).

This update for the Google Ads connector started on June 17, 2024, and was completed on June 23, 2024.

| Tables affected | Deprecated columns | New columns |
|---|---|---|---|
| - `ShoppingProductStats` - `ShoppingProductConversionStats` | `segments_product_bidding_category_level1` | `segments_product_category_level1` |
| - `ShoppingProductStats` - `ShoppingProductConversionStats` | `segments_product_bidding_category_level2` | `segments_product_category_level2` |
| - `ShoppingProductStats` - `ShoppingProductConversionStats` | `segments_product_bidding_category_level3` | `segments_product_category_level3` |
| - `ShoppingProductStats` - `ShoppingProductConversionStats` | `segments_product_bidding_category_level4` | `segments_product_category_level4` |
| - `ShoppingProductStats` - `ShoppingProductConversionStats` | `segments_product_bidding_category_level5` | `segments_product_category_level5` |
| - `ProductGroupStats` | `ad_group_criterion_listing_group_case_value_product_bidding_category_id` | `ad_group_criterion_listing_group_case_value_product_category_category_id` |
| - `ProductGroupStats` | `ad_group_criterion_listing_group_case_value_product_bidding_category_level` | `ad_group_criterion_listing_group_case_value_product_category_level` |
| - `ProductGroupStats` | - `AssetGroupListingFilter` | `asset_group_listing_group_filter_case_value_product_bidding_category_id` | `asset_group_listing_group_filter_case_value_product_category_category_id` |
| - `ProductGroupStats` | - `AssetGroupListingFilter` | `asset_group_listing_group_filter_case_value_product_bidding_category_level` | `asset_group_listing_group_filter_case_value_product_category_level` |
| - `ProductGroupStats` | - `AssetGroupListingFilter` | `asset_group_listing_group_filter_vertical` | `asset_group_listing_group_filter_listing_source` |
| - `AssetGroupListingFilter` |
| - `AssetGroupListingFilter` |

With Google Ads API v14, new columns, such as `segments_product_category_level1` and `segments_product_category_level2`, were added to the BigQuery table schema but were populated with `null`. With the update to Google Ads API v16, these new columns will be populated with new values. Deprecated columns, such as `segments_product_bidding_category_level1` and `segments_product_bidding_category_level2`, will be populated with `null`, but will still remain in the table schema.

For each pair of columns, only one column is populated with values from the Google Ads API while the other will be populated with `null`. To ensure your existing queries keep working after the update, update your queries to choose one of the two columns. For example, if you have the following statement in your SQL query:

```googlesql
segments_product_bidding_category_level1
```

Replace with the following statement that specifies the correct column:

```googlesql
IFNULL(segments_product_category_level1, segments_product_bidding_category_level1)
```

Transfer configurations that are created after June 24th 2024 will always use the new columns. Deprecated columns will still remain in the table schema but populated with `null`.

## Google Analytics API

The BigQuery Data Transfer Service for Google Analytics connector periodically updates to support
new columns and adapt to changes introduced by the [Google Analytics Data API](https://developers.google.com/analytics/devguides/reporting/data/v1/changelog).
The BigQuery Data Transfer Service for Google Analytics connector uses the latest
supported API version to retrieve reporting data.

The following sections outline the changes introduced by the
Google Analytics Data API. Changes are organized by release date,
and each entry provides information on the changes you need to make to
continue receiving data from Google Analytics.

### June 1, 2026

Effective June 1, 2026, backfills for Google Analytics data using BigQuery Data Transfer Service will be limited to the most recent 37 months. This change is in response to the [New Data Retention Policy for Google Ads starting June 1, 2026](https://ads-developers.googleblog.com/2026/05/new-data-retention-policy-for-google.html).

#### Risk of data loss

> [!WARNING]
> **Warning:** The following actions might result in data loss.

- Attempting to backfill Google Analytics data older than 37 months after June 1, 2026, can lead to data loss.

- The Google Analytics Data API might return partial or empty data for dates outside the 37-month retention period.

- If you trigger a backfill for a date older than 37 months, the transfer run will overwrite the existing data in your BigQuery partition with the incomplete results from the API, potentially replacing complete historical data with empty or partial data.

To avoid data loss, ensure that all manual or automated backfill processes for Google Analytics don't target dates older than 37 months from the schedule date.

Data already transferred and stored in your BigQuery tables before this change remains unaffected. Google won't delete your existing data.

Review your Google Analytics data transfer configurations and backfill scripts to ensure that they comply with the new 37 month retention window before June 1, 2026.

### September 22, 2025

> [!NOTE]
> **Note:** These changes only affect users who used the Google Analytics connector before April 25, 2025.

[The Google Analytics connector](https://docs.cloud.google.com/bigquery/docs/google-analytics-4-transfer)
plans to deprecate tables and update schemas to reflect changes in [Google Analytics Data API v1](https://developers.google.com/analytics/devguides/reporting/data/v1/changelog).
These changes are listed in the following sections.

This update for the Google Analytics connector is planned to start on September 22, 2025.

#### Deprecated tables

The following table shows the tables that will be deprecated and replaced with new tables with updated schemas. Note that the `p_ga4_conversions` and `p_ga4_inAppPurchases` tables will be discontinued after this update. Both deprecated and new tables will be populated until September 22, 2025 to allow time for migration. You can filter out deprecated tables using the [Table Filter](https://docs.cloud.google.com/bigquery/docs/google-analytics-4-transfer#set-up-ga4-transfer) option in the transfer configuration.

| Deprecated Table | New Table |
|---|---|
| `p_ga4_audiences` | `p_ga4_Audiences` |
| `p_ga4_conversions` | `Deprecated` |
| `p_ga4_demographicDetails` | `p_ga4_DemographicDetails` |
| `p_ga4_ecommercePurchase` | `p_ga4_EcommercePurchase` |
| `p_ga4_events` | `p_ga4_Events` |
| `p_ga4_inAppPurchases` | `Deprecated` |
| `p_ga4_landingPage` | `p_ga4_LandingPage` |
| `p_ga4_pagesAndScreens` | `p_ga4_PagesAndScreens` |
| `p_ga4_promotions` | `p_ga4_Promotions` |
| `p_ga4_techDetails` | `p_ga4_TechDetails` |
| `p_ga4_trafficAcquisition` | `p_ga4_TrafficAcquisition` |
| `p_ga4_userAcquisition` | `p_ga4_UserAcquisition` |

#### Updated table schemas

New table schemas can be found on the [Google Analytics report transformation](https://docs.cloud.google.com/bigquery/docs/google-analytics-4-transformation) page.

Summary of schema changes:

- **Corrected schemas:** The schemas for traffic acquisition, user acquisition, and landing page reports are corrected. For example, traffic acquisition and user acquisition report schemas were previously swapped, and the landing page report was missing the `landingPage` dimension.
- **Field renaming and discontinuation:** The conversions field is renamed to `keyEvents` across all reports to align with current Google Analytics terminology. Consequently, the "conversions" report itself is discontinued.
- **Data type changes:** Revenue fields change from `INTEGER` to `FLOAT` in BigQuery to accurately represent floating-point micro values as returned by the API.
- **New table and field naming convention:** Field names in new tables use `camelCase` (for example, `eventCount`) for consistency with the Google Analytics API, replacing the previous `snake_case` (for example, `event_count`).

## Microsoft SQL Server

The BigQuery Data Transfer Service for Microsoft SQL Server connector periodically
updates to adapt to new changes introduced by
Microsoft SQL Server.

The following sections outline the changes organized by release date.

### March 16, 2027

The Microsoft SQL Server connector plans to update its data type mapping to more
accurately reflect the source data and to ensure data integrity. The following
table shows the source data type, and the corresponding deprecated data type
mapping and the updated data type mapping:

| Microsoft SQL Server data type | Deprecated BigQuery data type mapping | Updated BigQuery data type mapping |
|---|---|---|
| `datetime` | `TIMESTAMP` | `DATETIME` |
| `datetime2` | `TIMESTAMP` | `DATETIME` |
| `smalldatetime` | `TIMESTAMP` | `DATETIME` |

You can continue to use the deprecated data type mapping in the transfer
configuration by setting the `connector.legacyMapping` parameter to `true`. You
can use the updated data type mapping by setting the `connector.legacyMapping`
parameter to `false`.

Starting September 16, 2026, all transfer configurations will use the updated
data type mapping by default. Support for the deprecated data type mapping will
end on March 16, 2027.

## MySQL

The BigQuery Data Transfer Service for MySQL connector periodically
updates to adapt to new changes introduced by
MySQL.

The following sections outline the changes organized by release date.

### March 16, 2027

The MySQL connector plans to update its data type mapping to more
accurately reflect the source data and to ensure data integrity. The following
table shows the source data type, and the corresponding deprecated data type
mapping and the updated data type mapping:

| MySQL data type | Deprecated BigQuery data type mapping | Updated BigQuery data type mapping |
|---|---|---|
| `DATETIME` | `TIMESTAMP` | `DATETIME` |
| `JSON` | `STRING` | `JSON` |
| `GEOMETRY` | `BYTES` | `GEOGRAPHY` |

You can continue to use the deprecated data type mapping in the transfer
configuration by setting the `connector.legacyMapping` parameter to `true`. You
can use the updated data type mapping by setting the `connector.legacyMapping`
parameter to `false`.

Starting September 16, 2026, all transfer configurations will use the updated
data type mapping by default. Support for the deprecated data type mapping will
end on March 16, 2027.

## Google Play Console

The BigQuery Data Transfer Service for Google Play connector periodically updates
to support new reports and changes of current reports introduced by Google Play.

The following sections outline the changes organized by release date.

### December 1, 2025

Google Play plans to make the following changes to the [Earnings report](https://support.google.com/googleplay/android-developer/answer/6135870#financial&zippy=%2Cearnings). The changes will be reflected in the BigQuery table `p_Earnings_suffix`. These changes are listed in the following sections.

#### Renamed columns

The following Google Play columns will be renamed.

| Deprecated columns | New columns |
|---|---|
| `Base_Plan_ID` | `Base_Plan_or_Purchase_Option_ID` |
| `Product_id` | `Package_ID` |

#### Column value change

The column `Product_Type` will change from a numeric representation to
a human-readable string.

#### New column

A new column `Sales_Channel` will be added to the Earnings report. This field provides information on where the sale originates from.

## PostgreSQL

The BigQuery Data Transfer Service for PostgreSQL connector periodically
updates to adapt to new changes introduced by
PostgreSQL.

The following sections outline the changes organized by release date.

### March 16, 2027

The PostgreSQL connector plans to update its data type mapping to more
accurately reflect the source data and to ensure data integrity. The following
table shows the source data type, and the corresponding deprecated data type
mapping and the updated data type mapping:

| PostgreSQL data type | Deprecated BigQuery data type mapping | Updated BigQuery data type mapping |
|---|---|---|
| `timestamp[(p)][without time zone]` | `TIMESTAMP` | `DATETIME` |
| `json` | `STRING` | `JSON` |
| `jsonb` | `STRING` | `JSON` |

You can continue to use the deprecated data type mapping in the transfer
configuration by setting the `connector.legacyMapping` parameter to `true`. You
can use the updated data type mapping by setting the `connector.legacyMapping`
parameter to `false`.

Starting September 16, 2026, all transfer configurations will use the updated
data type mapping by default. Support for the deprecated data type mapping will
end on March 16, 2027.

## Salesforce Bulk API

The BigQuery Data Transfer Service for Salesforce connector
periodically updates to support changes introduced by the
Salesforce Bulk API.

The following sections outline the changes when the Salesforce
connector updates to a new Bulk API version. Changes are organized by release
date, and each entry provides information on the changes you need to make for
you to continue receiving data from Salesforce.

### October 14, 2025

As part of the Salesforce connector GA release, the
Salesforce connector now uses Salesforce Bulk API
V1 version 64.0. Several fields that were supported in the Salesforce
Bulk API V1 version 53.0 are no longer supported.

#### Deprecated fields

The following table shows the fields deprecated with the Salesforce
connector GA release, along with the `sObject` name associated with each field.

| Deprecated Field | `sObject` name |
|---|---|
| `EffectiveDate` | `MobSecurityCertPinConfig` |
| `PermissionsAllowObjectDetectionTraining` | `Profile` |
| `PermissionsAllowObjectDetection` | `Profile` |
| `PermissionsAllowObjectDetectionTraining` | `PermissionSet` |
| `PermissionsAllowObjectDetection` | `PermissionSet` |
| `MaximumPermissionsAllowObjectDetectionTraining` | `PermissionSetLicense` |
| `MaximumPermissionsAllowObjectDetection` | `PermissionSetLicense` |
| `PermissionsAllowObjectDetectionTraining` | `UserPermissionAccess` |
| `PermissionsAllowObjectDetection` | `UserPermissionAccess` |
| `PermissionsAllowObjectDetectionTraining` | `MutingPermissionSet` |
| `PermissionsAllowObjectDetection` | `MutingPermissionSet` |
| `OptionsHstsHeaders` | `Domain` |
| `UserPreferencesHideInvoicesRedirectConfirmation` | `User` |
| `UserPreferencesHideStatementsRedirectConfirmation` | `User` |
| `UserPreferencesHideInvoicesRedirectConfirmation` | `UserChangeEvent` |
| `UserPreferencesHideStatementsRedirectConfirmation` | `UserChangeEvent` |

## Search Ads 360

The BigQuery Data Transfer Service for Search Ads 360 connector periodically updates to support
new columns and adapt to changes introduced by the [Search Ads 360 API](https://developers.google.com/search-ads/reporting/api/reference/fields/v0/overview).
The BigQuery Data Transfer Service for Search Ads 360 connector uses the latest
supported API version to retrieve reporting data.

The following sections outline the changes introduced by the
Search Ads 360 Data API. Changes are organized by release date,
and each entry provides information on the changes you need to make to
continue receiving data from Search Ads 360.

### June 1, 2026

Effective June 1, 2026, the Search Ads 360 connector will limit daily data backfills to the most recent 37 months. Backfill attempts for dates older than 37 months will result in an error. This change is in response to the [New Data Retention Policy for Google Ads starting June 1, 2026](https://ads-developers.googleblog.com/2026/05/new-data-retention-policy-for-google.html).

Any data already transferred and stored in your BigQuery tables will remain unaffected.

## ServiceNow

The BigQuery Data Transfer Service for ServiceNow connector periodically
updates to adapt to new changes introduced by
ServiceNow.

The following sections outline the changes organized by release date.

### March 16, 2027

The ServiceNow connector plans to update its data type mapping to more
accurately reflect the source data and to ensure data integrity. The following
table shows the source data type, and the corresponding deprecated data type
mapping and the updated data type mapping:

| ServiceNow data type | Deprecated BigQuery data type mapping | Updated BigQuery data type mapping |
|---|---|---|
| `glide_list` | `STRING` | `ARRAY` |
| `list` | `STRING` | `ARRAY` |

You can continue to use the deprecated data type mapping in the transfer
configuration by setting the `connector.legacyMapping` parameter to `true`. You
can use the updated data type mapping by setting the `connector.legacyMapping`
parameter to `false`.

Starting September 16, 2026, all transfer configurations will use the updated
data type mapping by default. Support for the deprecated data type mapping will
end on March 16, 2027.

## YouTube Reporting API

The BigQuery Data Transfer Service for YouTube Content Owner connector and
YouTube Channel connector periodically updates to support new reports
introduced by [YouTube Reporting API](https://developers.google.com/youtube/reporting)
and deprecate old reports.

The following sections outline the changes when new reports are introduced by
YouTube Reporting API. Changes are organized by release date, and each
entry provides information on the changes you need to make to continue receiving
data from YouTube.

### September 22, 2025

[The YouTube Content Owner connector](https://docs.cloud.google.com/bigquery/docs/youtube-content-owner-transfer)
and [The YouTube Channel connector](https://docs.cloud.google.com/bigquery/docs/youtube-channel-transfer)
plan to introduce new reports and deprecate old reports to reflect the
YouTube [shorts view count change](https://support.google.com/youtube/thread/333869549/a-change-to-how-we-count-views-on-shorts).
These changes are listed in the following sections.

New reports are planned to start on July 7, 2025. No action is required
from you to get the new reports. The deprecation of old reports is planned to
start on September 22, 2025.

#### YouTube Content Owner connector - deprecated tables

For the YouTube Content Owner connector, the following table shows the
BigQuery tables that will be deprecated and replaced with new
tables with updated schemas. Both deprecated and new tables will be populated
until September 22, 2025 to allow time for migration. After September 22, 2025,
only the new tables will be populated. The value for <var translate="no">suffix</var> is the
table suffix you configured when you created the transfer.

| Deprecated Table | New Table |
|---|---|
| `p_content_owner_asset_basic_a2_suffix` | `p_content_owner_asset_basic_a3_suffix` |
| `p_content_owner_asset_combined_a2_suffix` | `p_content_owner_asset_combined_a3_suffix` |
| `p_content_owner_asset_device_os_a2_suffix` | `p_content_owner_asset_device_os_a3_suffix` |
| `p_content_owner_asset_playback_location_a2_suffix` | `p_content_owner_asset_playback_location_a3_suffix` |
| `p_content_owner_asset_province_a2_suffix` | `p_content_owner_asset_province_a3_suffix` |
| `p_content_owner_asset_traffic_source_a2_suffix` | `p_content_owner_asset_traffic_source_a3_suffix` |
| `p_content_owner_basic_a3_suffix` | `p_content_owner_basic_a4_suffix` |
| `p_content_owner_combined_a2_suffix` | `p_content_owner_combined_a3_suffix` |
| `p_content_owner_device_os_a2_suffix` | `p_content_owner_device_os_a3_suffix` |
| `p_content_owner_playback_location_a2_suffix` | `p_content_owner_playback_location_a3_suffix` |
| `p_content_owner_playlist_basic_a1_suffix` | `p_content_owner_playlist_basic_a2_suffix` |
| `p_content_owner_playlist_combined_a1_suffix` | `p_content_owner_playlist_combined_a2_suffix` |
| `p_content_owner_playlist_device_os_a1_suffix` | `p_content_owner_playlist_device_os_a2_suffix` |
| `p_content_owner_playlist_playback_location_a1_suffix` | `p_content_owner_playlist_playback_location_a2_suffix` |
| `p_content_owner_playlist_province_a1_suffix` | `p_content_owner_playlist_province_a2_suffix` |
| `p_content_owner_playlist_traffic_source_a1_suffix` | `p_content_owner_playlist_traffic_source_a2_suffix` |
| `p_content_owner_province_a2_suffix` | `p_content_owner_province_a3_suffix` |
| `p_content_owner_subtitles_a2_suffix` | `p_content_owner_subtitles_a3_suffix` |
| `p_content_owner_traffic_source_a2_suffix` | `p_content_owner_traffic_source_a3_suffix` |
| `p_content_owner_shorts_ad_revenue_summary_a1_suffix` | `p_content_owner_shorts_ad_revenue_summary_a2_suffix` |
| `p_content_owner_shorts_country_ad_revenue_summary_a1_suffix` | `p_content_owner_shorts_country_ad_revenue_summary_a2_suffix` |
| `p_content_owner_shorts_day_ad_revenue_summary_a1_suffix` | `p_content_owner_shorts_day_ad_revenue_summary_a2_suffix` |
| `p_content_owner_shorts_global_ad_revenue_summary_a1_suffix` | `p_content_owner_shorts_global_ad_revenue_summary_a2_suffix` |

#### YouTube Channel connector - deprecated tables

For the YouTube Channel connector, the following table shows the
BigQuery tables that will be deprecated and replaced with new
tables with updated schemas. Both deprecated and new tables will be populated
until September 22, 2025 to allow time for migration. After September 22, 2025,
only the new tables will be populated. The value for <var translate="no">suffix</var> is the
table suffix you configured when you created the transfer.

| Deprecated Table | New Table |
|---|---|
| `p_channel_basic_a2_suffix` | `p_channel_basic_a3_suffix` |
| `p_channel_combined_a2_suffix` | `p_channel_combined_a3_suffix` |
| `p_channel_device_os_a2_suffix` | `p_channel_device_os_a3_suffix` |
| `p_channel_playback_location_a2_suffix` | `p_channel_playback_location_a3_suffix` |
| `p_channel_province_a2_suffix` | `p_channel_province_a3_suffix` |
| `p_channel_subtitles_a2_suffix` | `p_channel_subtitles_a3_suffix` |
| `p_channel_traffic_source_a2_suffix` | `p_channel_traffic_source_a3_suffix` |
| `p_playlist_basic_a1_suffix` | `p_playlist_basic_a2_suffix` |
| `p_playlist_combined_a1_suffix` | `p_playlist_combined_a2_suffix` |
| `p_playlist_device_os_a1_suffix` | `p_playlist_device_os_a2_suffix` |
| `p_playlist_playback_location_a1_suffix` | `p_playlist_playback_location_a2_suffix` |
| `p_playlist_province_a1_suffix` | `p_playlist_province_a2_suffix` |
| `p_playlist_traffic_source_a1_suffix` | `p_playlist_traffic_source_a2_suffix` |

#### Updated table schemas

The new tables will have a new column named `engaged_views`. For more
information about this metric, see [Shorts Viewcounting Changes](https://support.google.com/youtubekb/answer/10950071).