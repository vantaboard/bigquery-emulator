# Google Analytics 4 report transformation

When your Google Analytics 4 reports are transferred to BigQuery,
the reports are transformed into the following BigQuery tables
and views.

| **GA4 report name** | **BigQuery table** | **BigQuery view** |
|---|---|---|
| Audiences | p_ga4_Audiences | ga4_Audiences |
| Demographic details | p_ga4_DemographicDetails | ga4_DemographicDetails |
| Ecommerce purchases | p_ga4_EcommercePurchases | ga4_EcommercePurchases |
| Events | p_ga4_Events | ga4_Events |
| Landing page | p_ga4_LandingPage | ga4_LandingPage |
| Pages and screens | p_ga4_PagesAndScreens | ga4_PagesAndScreens |
| Promotions | p_ga4_Promotions | ga4_Promotions |
| Tech details | p_ga4_TechDetails | ga4_TechDetails |
| Traffic Acquisition | p_ga4_TrafficAcquisition | ga4_TrafficAcquisition |
| User Acquisition | p_ga4_UserAcquisition | ga4_UserAcquisition |

## Table schemas for Google Analytics reports

Table Name: Audiences

| Field Name | Description |
|---|---|
| audienceName | The given name of an Audience. Users are reported in the audiences to which they belonged during the report's date range. Current user behavior does not affect historical audience membership in reports. |
| averageSessionDuration | The average duration (in seconds) of users' sessions. |
| newUsers | The number of users who interacted with your site or launched your app for the first time (event triggered: first_open or first_visit). |
| screenPageViewsPerSession | The number of app screens or web pages your users viewed per session. Repeated views of a single page or screen are counted. (screen_view + page_view events) / sessions. |
| sessions | The number of sessions that began on your site or app (event triggered: session_start). |
| totalRevenue | The sum of revenue from purchases, subscriptions, and advertising (Purchase revenue plus Subscription revenue plus Ad revenue) minus refunded transaction revenue. |
| totalUsers | The number of distinct users who have logged at least one event, regardless of whether the site or app was in use when that event was logged. |

Table Name: DemographicDetails

| Field Name | Description |
|---|---|
| brandingInterest | Interests demonstrated by users who are higher in the shopping funnel. Users can be counted in multiple interest categories. For example, Shoppers, Lifestyles \& Hobbies/Pet Lovers, or Travel/Travel Buffs/Beachbound Travelers. |
| city | The city from which the user activity originated. |
| country | The country from which the user activity originated. |
| language | The language setting of the user's browser or device. For example, English. |
| region | The geographic region from which the user activity originated, derived from their IP address. |
| userAgeBracket | User age brackets. |
| userGender | User gender. |
| activeUsers | The number of distinct users who visited your website or application. |
| engagedSessions | The number of sessions that had an engaged event. |
| engagementRate | The percentage of sessions that had an engaged event. |
| eventCount | The count of events. |
| keyEvents | The number of key events that occurred. |
| newUsers | The number of users who interacted with your site or launched your app for the first time (event triggered: first_open or first_visit). |
| totalRevenue | The sum of revenue from purchases, subscriptions, and advertising (Purchase revenue plus Subscription revenue plus Ad revenue) minus refunded transaction revenue. |
| totalUsers | The number of distinct users who have logged at least one event, regardless of whether the site or app was in use when that event was logged. |
| userEngagementDuration | The total amount of time (in seconds) your website or app was in the foreground of users' devices. |
| userKeyEventRate | The percentage of users who triggered any key event. |

Table Name: EcommercePurchases

| Field Name | Description |
|---|---|
| itemBrand | Brand name of the item. |
| itemCategory | The hierarchical category in which the item is classified. For example, in Apparel/Mens/Summer/Shirts/T-shirts, Apparel is the item category. |
| itemCategory2 | The hierarchical category in which the item is classified. For example, in Apparel/Mens/Summer/Shirts/T-shirts, Mens is the item category 2. |
| itemCategory3 | The hierarchical category in which the item is classified. For example, in Apparel/Mens/Summer/Shirts/T-shirts, Summer is the item category 3. |
| itemCategory4 | The hierarchical category in which the item is classified. For example, in Apparel/Mens/Summer/Shirts/T-shirts, Shirts is the item category 4. |
| itemCategory5 | The hierarchical category in which the item is classified. For example, in Apparel/Mens/Summer/Shirts/T-shirts, T-shirts is the item category 5. |
| itemId | The ID of the item. |
| itemListPosition | The position of an item in a list. For example, a product you sell in a list. This dimension is populated in tagging by the index parameter in the items array. |
| itemName | The name of the item. |
| itemVariant | The specific variation of a product. For example, XS, S, M, or L for size; or Red, Blue, Green, or Black for color. Populated by the item_variant parameter. |
| itemAddedToCart | The number of units added to cart for a single item. This metric counts the quantity of items in add_to_cart events. |
| itemRevenue | The total revenue from purchases minus refunded transaction revenue from items only. Item revenue is the product of its price and quantity. Item revenue excludes tax and shipping values; tax \& shipping values are specified at the event and not item level. |
| itemsPurchased | The number of units for a single item included in purchase events. This metric counts the quantity of items in purchase events. |
| itemsViewed | The number of units viewed for a single item. This metric counts the quantity of items in view_item events. |

Table Name: Events

| Field Name | Description |
|---|---|
| eventName | The name of the event. |
| eventCount | The count of events. |
| eventCountPerUser | The average number of events per user (Event count divided by Active users). |
| totalRevenue | The sum of revenue from purchases, subscriptions, and advertising (Purchase revenue plus Subscription revenue plus Ad revenue) minus refunded transaction revenue. |
| totalUsers | The number of distinct users who have logged at least one event, regardless of whether the site or app was in use when that event was logged. |

Table Name: LandingPage

| Field Name | Description |
|---|---|
| landingPage | The page path associated with the first pageview in a session. |
| activeUsers | The number of distinct users who visited your website or application. |
| keyEvents | The number of key events that occurred. |
| newUsers | The number of users who interacted with your site or launched your app for the first time (event triggered: first_open or first_visit). |
| sessionKeyEventRate | The percentage of sessions in which any key event was triggered. |
| sessions | The number of sessions that began on your site or app (event triggered: session_start). |
| totalRevenue | The sum of revenue from purchases, subscriptions, and advertising (Purchase revenue plus Subscription revenue plus Ad revenue) minus refunded transaction revenue. |
| userEngagementDurationPerSession | Average engagement time per session |

Table Name: PagesAndScreens

| Field Name | Description |
|---|---|
| contentGroup | A category that applies to items of published content. Populated by the event parameter content_group. |
| unifiedPagePathScreen | The page path (web) or screen class (app) on which the event was logged. |
| unifiedScreenClass | The page title (web) or screen class (app) on which the event was logged. |
| unifiedScreenName | The page title (web) or screen name (app) on which the event was logged. |
| activeUsers | The number of distinct users who visited your website or application. |
| eventCount | The count of events. |
| keyEvents | The number of key events that occurred. |
| screenPageViews | The number of app screens or web pages your users viewed. Repeated views of a single page or screen are counted. (screen_view + page_view events). |
| screenPageViewsPerUser | The number of app screens or web pages your users viewed per active user. Repeated views of a single page or screen are counted. (screen_view + page_view events) / active users. |
| totalRevenue | The sum of revenue from purchases, subscriptions, and advertising (Purchase revenue plus Subscription revenue plus Ad revenue) minus refunded transaction revenue. |
| userEngagementDuration | The total amount of time (in seconds) your website or app was in the foreground of users' devices. |

Table Name: Promotions

| Field Name | Description |
|---|---|
| itemPromotionCreativeName | The name of the item-promotion creative. |
| itemPromotionId | The ID of the promotion. |
| itemPromotionName | The name of the promotion for the item. |
| itemListPosition | The position of an item in a list. For example, a product you sell in a list. This dimension is populated in tagging by the index parameter in the items array. |
| itemAddedToCart | The number of units added to cart for a single item. This metric counts the quantity of items in add_to_cart events. |
| itemCheckedOut | The number of units checked out for a single item. This metric counts the quantity of items in begin_checkout events. |
| itemPromotionClickThroughRate | The number of users who selected a promotion(s) divided by the number of users who viewed the same promotion(s). This metric is returned as a fraction; for example, 0.1382 means 13.82% of users who viewed a promotion also selected the promotion. |
| itemRevenue | The total revenue from purchases minus refunded transaction revenue from items only. Item revenue is the product of its price and quantity. Item revenue excludes tax and shipping values; tax \& shipping values are specified at the event and not item level. |
| itemsClickedInPromotion | The number of units clicked in promotion for a single item. This metric counts the quantity of items in select_promotion events. |
| itemsPurchased | The number of units for a single item included in purchase events. This metric counts the quantity of items in purchase events. |
| itemsViewedInPromotion | The number of units viewed in promotion for a single item. This metric counts the quantity of items in view_promotion events. |

Table Name: TechDetails

| Field Name | Description |
|---|---|
| appVersion | The app's versionName (Android) or short bundle version (iOS). |
| browser | The browsers used to view your website. |
| deviceCategory | The type of device: Desktop, Tablet, or Mobile. |
| operatingSystem | The operating systems used by visitors to your app or website. Includes desktop and mobile operating systems such as Windows and Android. |
| operatingSystemVersion | The operating system versions used by visitors to your website or app. For example, Android 10's version is 10, and iOS 13.5.1's version is 13.5.1. |
| operatingSystemWithVersion | The operating system and version. For example, Android 10 or Windows 7. |
| platform | The platform on which your app or website ran; for example, web, iOS, or Android. To determine a stream's type in a report, use both platform and streamId. |
| platformDeviceCategory | The platform and type of device on which your website or mobile app ran. (example: Android / mobile) |
| screenResolution | The screen resolution of the user's monitor. For example, 1920x1080. |
| activeUsers | The number of distinct users who visited your website or application. |
| engagedSessions | The number of sessions that lasted longer than 10 seconds, or had a key event, or had 2 or more screen views. |
| engagementRate | The percentage of engaged sessions (Engaged sessions divided by Sessions). This metric is returned as a fraction; for example, 0.7239 means 72.39% of sessions were engaged sessions. |
| eventCount | The count of events. |
| keyEvents | The number of key events that occurred. |
| newUsers | The number of users who interacted with your site or launched your app for the first time (event triggered: first_open or first_visit). |
| totalRevenue | The sum of revenue from purchases, subscriptions, and advertising (Purchase revenue plus Subscription revenue plus Ad revenue) minus refunded transaction revenue. |
| userEngagementDuration | The total amount of time (in seconds) your website or app was in the foreground of users' devices. |

Table Name: TrafficAcquisition

| Field Name | Description |
|---|---|
| sessionCampaignName | The marketing campaign name for a session. Includes Google Ads Campaigns, Manual Campaigns, \& other Campaigns. |
| sessionDefaultChannelGroup | The session's default channel group is based primarily on source and medium. An enumeration which includes Direct, Organic Search, Paid Social, Organic Social, Email, Affiliates, Referral, Paid Search, Video, and Display. |
| sessionMedium | The medium that initiated a session on your website or app. |
| sessionPrimaryChannelGroup | The primary channel group that led to the session. Primary channel groups are the channel groups used in standard reports in Google Analytics and serve as an active record of your property's data in alignment with channel grouping over time. |
| sessionSource | The source that initiated a session on your website or app. |
| sessionSourceMedium | The combined values of the dimensions sessionSource and sessionMedium. |
| sessionSourcePlatform | The source platform of the session's campaign. Don't depend on this field returning Manual for traffic that uses UTMs; this field will update from returning Manual to returning (not set) for an upcoming feature launch. |
| eventCount | The count of events. |
| eventsPerSession | The average number of events per session (Event count divided by Sessions). |
| engagementRate | The percentage of engaged sessions (Engaged sessions divided by Sessions). This metric is returned as a fraction; for example, 0.7239 means 72.39% of sessions were engaged sessions. |
| engagedSessions | The number of sessions that lasted longer than 10 seconds, or had a key event, or had 2 or more screen views. |
| keyEvents | The number of key events that occurred. |
| sessions | The number of sessions that began on your site or app (event triggered: session_start). |
| sessionKeyEventRate | The percentage of sessions in which any key event was triggered. |
| sessionsPerUser | The average number of sessions per user (Sessions divided by Active Users). |
| totalRevenue | The sum of revenue from purchases, subscriptions, and advertising (Purchase revenue plus Subscription revenue plus Ad revenue) minus refunded transaction revenue. |
| userEngagementDurationPerSession | Average engagement time per session |

Table Name: UserAcquisition

| Field Name | Description |
|---|---|
| firstUserCampaignName | Name of the marketing campaign that first acquired the user. Includes Google Ads Campaigns, Manual Campaigns, \& other Campaigns. |
| firstUserDefaultChannelGroup | The default channel group that first acquired the user. Default channel group is based primarily on source and medium. An enumeration which includes Direct, Organic Search, Paid Social, Organic Social, Email, Affiliates, Referral, Paid Search, Video, and Display. |
| firstUserMedium | The medium that first acquired the user to your website or app. |
| firstUserPrimaryChannelGroup | The primary channel group that originally acquired a user. Primary channel groups are the channel groups used in standard reports in Google Analytics and serve as an active record of your property's data in alignment with channel grouping over time. |
| firstUserSource | The source that first acquired the user to your website or app. |
| firstUserSourceMedium | The combined values of the dimensions firstUserSource and firstUserMedium. |
| firstUserSourcePlatform | The source platform that first acquired the user. Don't depend on this field returning Manual for traffic that uses UTMs; this field will update from returning Manual to returning (not set) for an upcoming feature launch. |
| activeUsers | The number of distinct users who visited your website or application. |
| engagedSessions | The number of sessions that lasted longer than 10 seconds, or had a key event, or had 2 or more screen views. |
| engagementRate | The percentage of engaged sessions (Engaged sessions divided by Sessions). This metric is returned as a fraction; for example, 0.7239 means 72.39% of sessions were engaged sessions. |
| eventCount | The count of events. |
| keyEvents | The number of key events that occurred. |
| newUsers | The number of users who interacted with your site or launched your app for the first time (event triggered: first_open or first_visit). |
| totalRevenue | The sum of revenue from purchases, subscriptions, and advertising (Purchase revenue plus Subscription revenue plus Ad revenue) minus refunded transaction revenue. |
| totalUsers | The number of distinct users who have logged at least one event, regardless of whether the site or app was in use when that event was logged. |
| userEngagementDuration | The total amount of time (in seconds) your website or app was in the foreground of users' devices. |
| userKeyEventRate | The percentage of users who triggered any key event. |