# YouTube Channel report transformation

When your YouTube Channel reports are transferred to BigQuery,
the reports are transformed into the following BigQuery tables
and views.

When you view the tables and views in BigQuery, the value for
<var translate="no">suffix</var> is the table suffix you configured when you created the
transfer.

| **YouTube Channel report** | **BigQuery table** | **BigQuery view** |
|---|---|---|
| **Video reports** |   |   |
| [User activity](https://developers.google.com/youtube/reporting/v1/reports/channel_reports#video-user-activity) | p_channel_basic_a3_<var translate="no">suffix</var> | channel_basic_a3_<var translate="no">suffix</var> |
| [User activity by province](https://developers.google.com/youtube/reporting/v1/reports/channel_reports#video-province) | p_channel_province_a3_<var translate="no">suffix</var> | channel_province_a3_<var translate="no">suffix</var> |
| [Playback locations](https://developers.google.com/youtube/reporting/v1/reports/channel_reports#video-playback-locations) | p_channel_playback_location_a3_<var translate="no">suffix</var> | channel_playback_location_a3_<var translate="no">suffix</var> |
| [Traffic sources](https://developers.google.com/youtube/reporting/v1/reports/channel_reports#video-traffic-sources) | p_channel_traffic_source_a3_<var translate="no">suffix</var> | channel_traffic_source_a3_<var translate="no">suffix</var> |
| [Device type and operating system](https://developers.google.com/youtube/reporting/v1/reports/channel_reports#video-device-type-and-operating-system) | p_channel_device_os_a3_<var translate="no">suffix</var> | channel_device_os_a3_<var translate="no">suffix</var> |
| [Viewer demographics](https://developers.google.com/youtube/reporting/v1/reports/channel_reports#video-viewer-demographics) | p_channel_demographics_a1_<var translate="no">suffix</var> | channel_demographics_a1_<var translate="no">suffix</var> |
| [Content sharing by platform](https://developers.google.com/youtube/reporting/v1/reports/channel_reports#video-content-sharing) | p_channel_sharing_service_a1_<var translate="no">suffix</var> | channel_sharing_service_a1_<var translate="no">suffix</var> |
| [Annotations](https://developers.google.com/youtube/reporting/v1/reports/channel_reports#video-annotations) | p_channel_annotations_a1_<var translate="no">suffix</var> | channel_annotations_a1_<var translate="no">suffix</var> |
| [Cards](https://developers.google.com/youtube/reporting/v1/reports/channel_reports#video-cards) | p_channel_cards_a1_<var translate="no">suffix</var> | channel_cards_a1_<var translate="no">suffix</var> |
| [End screens](https://developers.google.com/youtube/reporting/v1/reports/channel_reports#video-end-screens) | p_channel_end_screens_a1_<var translate="no">suffix</var> | channel_end_screens_a1_<var translate="no">suffix</var> |
| [Subtitles](https://developers.google.com/youtube/reporting/v1/reports/channel_reports#video-subtitles) | p_channel_subtitles_a3_<var translate="no">suffix</var> | channel_subtitles_a3_<var translate="no">suffix</var> |
| [Combined](https://developers.google.com/youtube/reporting/v1/reports/channel_reports#video-combined) | p_channel_combined_a3_<var translate="no">suffix</var> | channel_combined_a3_<var translate="no">suffix</var> |
| **Playlist reports** |   |   |
| [User activity](https://developers.google.com/youtube/reporting/v1/reports/channel_reports#playlist-user-activity) | p_playlist_basic_a2_<var translate="no">suffix</var> | playlist_basic_a2_<var translate="no">suffix</var> |
| [User activity by province](https://developers.google.com/youtube/reporting/v1/reports/channel_reports#playlist-province) | p_playlist_province_a2_<var translate="no">suffix</var> | playlist_province_a2_<var translate="no">suffix</var> |
| [Playback locations](https://developers.google.com/youtube/reporting/v1/reports/channel_reports#playlist-playback-locations) | p_playlist_playback_location_a2_<var translate="no">suffix</var> | playlist_playback_location_a2_<var translate="no">suffix</var> |
| [Traffic sources](https://developers.google.com/youtube/reporting/v1/reports/channel_reports#playlist-traffic-sources) | p_playlist_traffic_source_a2_<var translate="no">suffix</var> | playlist_traffic_source_a2_<var translate="no">suffix</var> |
| [Device type and operating system](https://developers.google.com/youtube/reporting/v1/reports/channel_reports#playlist-device-type-and-operating-system) | p_playlist_device_os_a2_<var translate="no">suffix</var> | playlist_device_os_a2_<var translate="no">suffix</var> |
| [Combined](https://developers.google.com/youtube/reporting/v1/reports/channel_reports#playlist-combined) | p_playlist_combined_a2_<var translate="no">suffix</var> | playlist_combined_a2_<var translate="no">suffix</var> |
| **Reach reports** |   |   |
| [Reach basic](https://developers.google.com/youtube/reporting/v1/reports/channel_reports#reach-reports) | p_channel_reach_basic_a1_<var translate="no">suffix</var> | channel_reach_basic_a1_<var translate="no">suffix</var> |
| [Reach combined](https://developers.google.com/youtube/reporting/v1/reports/channel_reports#reach-reports) | p_channel_reach_combined_a1_<var translate="no">suffix</var> | channel_reach_combined_a1_<var translate="no">suffix</var> |