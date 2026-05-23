# Campaign Manager report transformation

When your Campaign Manager (formerly known as DoubleClick Campaign Manager) data
transfer files are transferred to BigQuery, the files are
transformed into the following BigQuery tables and views.

When you view the tables and views in BigQuery, the value for
<var translate="no">campaign_manager_id</var> is your Campaign Manager Network, Advertiser, or
Floodlight ID.

| **Campaign Manager file** | **BigQuery table** | **BigQuery view** |
|---|---|---|
| **Data Transfer files** |   |   |
| [impression](https://developers.google.com/doubleclick-advertisers/dtv2/reference/file-format) | p_impression_<var translate="no">campaign_manager_id</var> | impression_<var translate="no">campaign_manager_id</var> |
| [click](https://developers.google.com/doubleclick-advertisers/dtv2/reference/file-format) | p_click_<var translate="no">campaign_manager_id</var> | click_<var translate="no">campaign_manager_id</var> |
| [activity](https://developers.google.com/doubleclick-advertisers/dtv2/reference/file-format) | p_activity_<var translate="no">campaign_manager_id</var> | activity_<var translate="no">campaign_manager_id</var> |
| [rich_media](https://developers.google.com/doubleclick-advertisers/dtv2/reference/file-format) | p_rich_media_<var translate="no">campaign_manager_id</var> | rich_media_<var translate="no">campaign_manager_id</var> |
| **Match Tables** |   |   |
| [activity_cats](https://developers.google.com/doubleclick-advertisers/dtv2/reference/match-tables#activity_cats) | p_match_table_activity_cats_<var translate="no">campaign_manager_id</var> | match_table_activity_cats_<var translate="no">campaign_manager_id</var> |
| [activity_types](https://developers.google.com/doubleclick-advertisers/dtv2/reference/match-tables#activity_types) | p_match_table_activity_types_<var translate="no">campaign_manager_id</var> | match_table_activity_types_<var translate="no">campaign_manager_id</var> |
| [ads](https://developers.google.com/doubleclick-advertisers/dtv2/reference/match-tables#ads) | p_match_table_ads_<var translate="no">campaign_manager_id</var> | match_table_ads_<var translate="no">campaign_manager_id</var> |
| [ad_placement_assignments](https://developers.google.com/doubleclick-advertisers/dtv2/reference/match-tables#ad_placement_assignments) | p_match_table_ad_placement_assignments_<var translate="no">campaign_manager_id</var> | match_table_ad_placement_assignments_<var translate="no">campaign_manager_id</var> |
| [advertisers](https://developers.google.com/doubleclick-advertisers/dtv2/reference/match-tables#advertisers) | p_match_table_advertisers_<var translate="no">campaign_manager_id</var> | match_table_advertisers_<var translate="no">campaign_manager_id</var> |
| [assets](https://developers.google.com/doubleclick-advertisers/dtv2/reference/match-tables#assets) | p_match_table_assets_<var translate="no">campaign_manager_id</var> | match_table_assets_<var translate="no">campaign_manager_id</var> |
| [browsers](https://developers.google.com/doubleclick-advertisers/dtv2/reference/match-tables#browsers) | p_match_table_browsers_<var translate="no">campaign_manager_id</var> | match_table_browsers_<var translate="no">campaign_manager_id</var> |
| [campaigns](https://developers.google.com/doubleclick-advertisers/dtv2/reference/match-tables#campaigns) | p_match_table_campaigns_<var translate="no">campaign_manager_id</var> | match_table_campaigns_<var translate="no">campaign_manager_id</var> |
| [cities](https://developers.google.com/doubleclick-advertisers/dtv2/reference/match-tables#cities) | p_match_table_cities_<var translate="no">campaign_manager_id</var> | match_table_cities_<var translate="no">campaign_manager_id</var> |
| [creatives](https://developers.google.com/doubleclick-advertisers/dtv2/reference/match-tables#creatives) | p_match_table_creatives_<var translate="no">campaign_manager_id</var> | match_table_creatives_<var translate="no">campaign_manager_id</var> |
| [creative_ad_assignments](https://developers.google.com/doubleclick-advertisers/dtv2/reference/match-tables#creative_ad_assignments) | p_match_table_creative_ad_assignments_<var translate="no">campaign_manager_id</var> | match_table_creative_ad_assignments_<var translate="no">campaign_manager_id</var> |
| [custom_creative_fields](https://developers.google.com/doubleclick-advertisers/dtv2/reference/match-tables#custom_creative_fields) | p_match_table_custom_creative_fields_<var translate="no">campaign_manager_id</var> | match_table_custom_creative_fields_<var translate="no">campaign_manager_id</var> |
| [paid_search](https://developers.google.com/doubleclick-advertisers/dtv2/reference/match-tables#paid_search) | p_match_table_paid_search_<var translate="no">campaign_manager_id</var> | match_table_paid_search_<var translate="no">campaign_manager_id</var> |
| [designated_market_areas](https://developers.google.com/doubleclick-advertisers/dtv2/reference/match-tables#designated_market_areas) | p_match_table_designated_market_areas_<var translate="no">campaign_manager_id</var> | match_table_designated_market_areas_<var translate="no">campaign_manager_id</var> |
| [keyword_value](https://developers.google.com/doubleclick-advertisers/dtv2/reference/match-tables#keyword_value) | p_match_table_keyword_value_<var translate="no">campaign_manager_id</var> | match_table_keyword_value_<var translate="no">campaign_manager_id</var> |
| null user ID reason categories | Unsupported | Unsupported |
| rich media standard event and event type IDs | Unsupported | Unsupported |
| [custom_rich_media](https://developers.google.com/doubleclick-advertisers/dtv2/reference/match-tables#custom_rich_media) | p_match_table_custom_rich_media_<var translate="no">campaign_manager_id</var> | match_table_custom_rich_media_<var translate="no">campaign_manager_id</var> |
| [operating_systems](https://developers.google.com/doubleclick-advertisers/dtv2/reference/match-tables#operating_systems) | p_match_table_operating_systems_<var translate="no">campaign_manager_id</var> | match_table_operating_systems_<var translate="no">campaign_manager_id</var> |
| [placements](https://developers.google.com/doubleclick-advertisers/dtv2/reference/match-tables#placements) | p_match_table_placements_<var translate="no">campaign_manager_id</var> | match_table_placements_<var translate="no">campaign_manager_id</var> |
| [placement_cost](https://developers.google.com/doubleclick-advertisers/dtv2/reference/match-tables#placement_cost) | p_match_table_placement_cost_<var translate="no">campaign_manager_id</var> | match_table_placement_cost_<var translate="no">campaign_manager_id</var> |
| [sites](https://developers.google.com/doubleclick-advertisers/dtv2/reference/match-tables#sites) | p_match_table_sites_<var translate="no">campaign_manager_id</var> | match_table_sites_<var translate="no">campaign_manager_id</var> |
| [states](https://developers.google.com/doubleclick-advertisers/dtv2/reference/match-tables#states) | p_match_table_states_<var translate="no">campaign_manager_id</var> | match_table_states_<var translate="no">campaign_manager_id</var> |
| [custom_floodlight_variables](https://developers.google.com/doubleclick-advertisers/dtv2/reference/match-tables#custom_floodlight_variables) | p_match_table_custom_floodlight_variables_<var translate="no">campaign_manager_id</var> | match_table_custom_floodlight_variables_<var translate="no">campaign_manager_id</var> |
| [landing_page_url](https://developers.google.com/doubleclick-advertisers/dtv2/reference/match-tables#landing_page_url) | p_match_table_landing_page_url_<var translate="no">campaign_manager_id</var> | match_table_landing_page_url_<var translate="no">campaign_manager_id</var> |