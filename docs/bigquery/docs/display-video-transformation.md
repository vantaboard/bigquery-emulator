# Display \& Video 360 data transformation

When your Display \& Video 360 data are transferred to BigQuery, they are
transformed into the following BigQuery tables and views.

When you view the tables and views in BigQuery, the value for
<var translate="no">displayvideo_id</var> is your Display \& Video 360 partner or advertiser ID.

| **Display \& Video 360 resource** | **BigQuery table** | **BigQuery view** |
|---|---|---|
| **Data Transfer files** |||
| [Impression](https://developers.google.com/bid-manager/dtv2/reference/file-format) | p_Impression_<var translate="no">displayvideo_id</var> | Impression_<var translate="no">displayvideo_id</var> |
| [Click](https://developers.google.com/bid-manager/dtv2/reference/file-format) | p_Click_<var translate="no">displayvideo_id</var> | Click_<var translate="no">displayvideo_id</var> |
| [Activity](https://developers.google.com/bid-manager/dtv2/reference/file-format) | p_Activity_<var translate="no">displayvideo_id</var> | Activity_<var translate="no">displayvideo_id</var> |
| **DV360 API Resource (v3)** |||
| [Partner](https://developers.google.com/display-video/api/reference/rest/v3/partners#resource:-partner) | p_Partner_<var translate="no">displayvideo_id</var> | Partner_<var translate="no">displayvideo_id</var> |
| [Advertiser](https://developers.google.com/display-video/api/reference/rest/v3/advertisers#resource:-advertiser) | p_Advertiser_<var translate="no">displayvideo_id</var> | Advertiser_<var translate="no">displayvideo_id</var> |
| [LineItem](https://developers.google.com/display-video/api/reference/rest/v3/advertisers.lineItems#LineItem) | p_LineItem_<var translate="no">displayvideo_id</var> | LineItem_<var translate="no">displayvideo_id</var> |
| [LineItemTargeting](https://developers.google.com/display-video/api/reference/rest/v3/advertisers.lineItems/bulkListAssignedTargetingOptions#LineItemAssignedTargetingOption) | p_LineItemTargeting_<var translate="no">displayvideo_id</var> | LineItemTargeting_<var translate="no">displayvideo_id</var> |
| [Campaign](https://developers.google.com/display-video/api/reference/rest/v3/advertisers.campaigns#Campaign) | p_Campaign_<var translate="no">displayvideo_id</var> | Campaign_<var translate="no">displayvideo_id</var> |
| [CampaignTargeting](https://developers.google.com/display-video/api/reference/rest/v3/advertisers.campaigns.targetingTypes.assignedTargetingOptions#AssignedTargetingOption) | p_CampaignTargeting_<var translate="no">displayvideo_id</var> | CampaignTargeting_<var translate="no">displayvideo_id</var> |
| [InsertionOrder](https://developers.google.com/display-video/api/reference/rest/v3/advertisers.insertionOrders#InsertionOrder) | p_InsertionOrder_<var translate="no">displayvideo_id</var> | InsertionOrder_<var translate="no">displayvideo_id</var> |
| [InsertionOrderTargeting](https://developers.google.com/display-video/api/reference/rest/v3/advertisers.insertionOrders.targetingTypes.assignedTargetingOptions#AssignedTargetingOption) | p_InsertionOrderTargeting_<var translate="no">displayvideo_id</var> | InsertionOrderTargeting_<var translate="no">displayvideo_id</var> |
| [AdGroup](https://developers.google.com/display-video/api/reference/rest/v3/advertisers.adGroups#AdGroup) | p_AdGroup_<var translate="no">displayvideo_id</var> | AdGroup_<var translate="no">displayvideo_id</var> |
| [AdGroupTargeting](https://developers.google.com/display-video/api/reference/rest/v3/advertisers.adGroups/bulkListAdGroupAssignedTargetingOptions#AdGroupAssignedTargetingOption) | p_AdGroupTargeting_<var translate="no">displayvideo_id</var> | AdGroupTargeting_<var translate="no">displayvideo_id</var> |
| [AdGroupAd](https://developers.google.com/display-video/api/reference/rest/v3/advertisers.adGroupAds#AdGroupAd) | p_AdGroupAd_<var translate="no">displayvideo_id</var> | AdGroupAd_<var translate="no">displayvideo_id</var> |
| [Creative](https://developers.google.com/display-video/api/reference/rest/v3/advertisers.creatives#resource:-creative) | p_Creative_<var translate="no">displayvideo_id</var> | Creative_<var translate="no">displayvideo_id</var> |