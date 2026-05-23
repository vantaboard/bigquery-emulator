# Google Play report transformation

When your Google Play reports are transferred to BigQuery,
the reports are transformed into the following BigQuery tables
and views.

When you view the tables and views in BigQuery, the value for
<var translate="no">suffix</var> is the table suffix you configured when you created the
transfer.

| Google Play report | BigQuery table | BigQuery view |
|---|---|---|
| **Detailed Reports** |   |   |
| ***Reviews*** |   |   |
| [Reviews](https://support.google.com/googleplay/android-developer/answer/6135870#reviews) | p_Reviews_<var translate="no">suffix</var> | Reviews_<var translate="no">suffix</var> |
| ***Financial Reports*** |   |   |
| [Estimated Sales](https://support.google.com/googleplay/android-developer/answer/6135870#financial) | p_Sales_<var translate="no">suffix</var> | Sales_<var translate="no">suffix</var> |
| [Earnings](https://support.google.com/googleplay/android-developer/answer/6135870#financial) | p_Earnings_<var translate="no">suffix</var> | Earnings_<var translate="no">suffix</var> |
| [Korean Play balance funded](https://support.google.com/googleplay/android-developer/answer/6135870#financial) | p_Korean_Play_balance_funded_<var translate="no">suffix</var> | Korean_Play_balance_funded_<var translate="no">suffix</var> |
| **Aggregated Reports** |   |   |
| ***Statistics*** |   |   |
| [Installs](https://support.google.com/googleplay/android-developer/answer/6135870#statistics) | p_Installs_app_version_<var translate="no">suffix</var> p_Installs_carrier_<var translate="no">suffix</var> p_Installs_country_<var translate="no">suffix</var> p_Installs_device_<var translate="no">suffix</var> p_Installs_language_<var translate="no">suffix</var> p_Installs_os_version_<var translate="no">suffix</var> | Installs_app_version_<var translate="no">suffix</var> Installs_carrier_<var translate="no">suffix</var> Installs_country_<var translate="no">suffix</var> Installs_device_<var translate="no">suffix</var> Installs_language_<var translate="no">suffix</var> Installs_os_version_<var translate="no">suffix</var> |
| [Crashes](https://support.google.com/googleplay/android-developer/answer/6135870#statistics) | p_Crashes_app_version_<var translate="no">suffix</var> p_Crashes_device_<var translate="no">suffix</var> p_Crashes_os_version_<var translate="no">suffix</var> | Crashes_app_version_<var translate="no">suffix</var> Crashes_device_<var translate="no">suffix</var> Crashes_os_version_<var translate="no">suffix</var> |
| [Ratings](https://support.google.com/googleplay/android-developer/answer/6135870#statistics) | p_Ratings_app_version_<var translate="no">suffix</var> p_Ratings_carrier_<var translate="no">suffix</var> p_Ratings_country_<var translate="no">suffix</var> p_Ratings_device_<var translate="no">suffix</var> p_Ratings_language_<var translate="no">suffix</var> p_Ratings_os_version_<var translate="no">suffix</var> | Ratings_app_version_<var translate="no">suffix</var> Ratings_carrier_<var translate="no">suffix</var> Ratings_country_<var translate="no">suffix</var> Ratings_device_<var translate="no">suffix</var> Ratings_language_<var translate="no">suffix</var> Ratings_os_version_<var translate="no">suffix</var> |
| [Subscribers](https://support.google.com/googleplay/android-developer/answer/6135870#statistics) | p_Stats_Subscribers_country_<var translate="no">suffix</var> p_Stats_Subscribers_device_<var translate="no">suffix</var> | Stats_Subscribers_country_<var translate="no">suffix</var> Stats_Subscribers_device_<var translate="no">suffix</var> |
| ***User Acquisition*** |   |   |
| [Store Performance](https://support.google.com/googleplay/android-developer/answer/6135870#acquisition) | p_Store_Performance_country_<var translate="no">suffix</var> p_Store_Performance_traffic_source_<var translate="no">suffix</var> | Store_Performance_country_<var translate="no">suffix</var> Store_Performance_traffic_source_<var translate="no">suffix</var> |