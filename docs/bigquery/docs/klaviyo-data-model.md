# Klaviyo data model reference

This page lists the data that's transferred to BigQuery when you
[run a Klaviyo data transfer](https://docs.cloud.google.com/bigquery/docs/klaviyo-transfer).
The data is organized into tables that list each field name, its associated
destination data type, and the JSON path from the source data.

## Accounts

Klaviyo account information and metadata.

- Table name: Accounts
- Endpoint: `/accounts`
- Klaviyo API reference: [Get Accounts](https://developers.klaviyo.com/en/reference/get_accounts)

| Field Name | Type | JSON Path | Description |
|---|---|---|---|
| `type` | STRING | `$.type` | Resource type (always `account`). |
| `id` | STRING | `$.id` | Unique identifier for the account. |
| `test_account` | STRING | `$.attributes.test_account` | Indicates if this is a test account. |
| `default_sender_name` | STRING | `$.attributes.contact_information.default_sender_name` | Default name used as the sender for emails. |
| `default_sender_email` | STRING | `$.attributes.contact_information.default_sender_email` | Default email address used as the sender. |
| `website_url` | STRING | `$.attributes.contact_information.website_url` | URL of the organization's website. |
| `organization_name` | STRING | `$.attributes.contact_information.organization_name` | Name of the organization. |
| `address1` | STRING | `$.attributes.contact_information.street_address.address1` | Street address line 1. |
| `address2` | STRING | `$.attributes.contact_information.street_address.address2` | Street address line 2. |
| `city` | STRING | `$.attributes.contact_information.street_address.city` | City of the organization. |
| `region` | STRING | `$.attributes.contact_information.street_address.region` | State, province, or region. |
| `country` | STRING | `$.attributes.contact_information.street_address.country` | Country. |
| `zip` | STRING | `$.attributes.contact_information.street_address.zip` | Postal or Zip code. |
| `industry` | STRING | `$.attributes.industry` | Industry vertical of the account. |
| `timezone` | STRING | `$.attributes.timezone` | Timezone setting for the account. |
| `preferred_currency` | STRING | `$.attributes.preferred_currency` | Primary currency used by the account. |
| `public_api_key` | STRING | `$.attributes.public_api_key` | Public API key (Site ID) for client-side integrations. |
| `locale` | STRING | `$.attributes.locale` | Locale setting (e.g., en-US). |

## Coupons

Coupons for discounts and promotions.

- Table name: Coupons
- Endpoint: `/coupons`
- Klaviyo API reference: [Get Coupons](https://developers.klaviyo.com/en/reference/get_coupons)

| Field Name | Type | JSON Path | Description |
|---|---|---|---|
| `type` | STRING | `$.type` | Resource type (always `coupon`). |
| `id` | STRING | `$.id` | Unique internal identifier for the coupon. |
| `external_id` | STRING | `$.attributes.external_id` | External identifier (often the same as name/id). |
| `description` | STRING | `$.attributes.description` | Description of the coupon offer. |
| `low_balance_threshold` | STRING | `$.attributes.monitor_configuration.low_balance_threshold` | Threshold to trigger low balance alerts. |

## CouponCode

Individual unique codes generated for specific coupons.

- Table name: CouponCode
- Endpoint: `/coupon-codes`
- Klaviyo API reference: [Get Coupon Codes](https://developers.klaviyo.com/en/reference/get_coupon_codes)

| Field Name | Type | JSON Path | Description |
|---|---|---|---|
| `type` | STRING | `$.type` | Resource type (always `coupon-code`). |
| `id` | STRING | `$.id` | Unique identifier for this specific code instance. |
| `unique_code` | STRING | `$.attributes.unique_code` | The actual alphanumeric code string. |
| `expires_at` | TIMESTAMP | `$.attributes.expires_at` | Timestamp when this code expires. |
| `status` | STRING | `$.attributes.status` | Status of the code (e.g., ASSIGNED, UNASSIGNED). |
| `coupon_id` | STRING | `$.relationships.coupon.data.id` | ID of the parent Coupon definition. |

## Events

Activity events tracked for profiles (e.g., Placed Order, Viewed Product).

- Table name: Events
- Endpoint: `/events`
- Klaviyo API reference: [Get Events](https://developers.klaviyo.com/en/reference/get_events)

| Field Name | Type | JSON Path | Description |
|---|---|---|---|
| `type` | STRING | `$.type` | Resource type (always `event`). |
| `id` | STRING | `$.id` | Unique identifier for the event. |
| `timestamp` | FLOAT | `$.attributes.timestamp` | Unix timestamp of when the event occurred. |
| `event_properties` | JSON | `$.attributes.event_properties` | Custom JSON properties specific to the event type (e.g., items in order). |
| `datetime` | TIMESTAMP | `$.attributes.datetime` | ISO 8601 timestamp of the event. |
| `uuid` | STRING | `$.attributes.uuid` | Universally Unique Identifier for the event. |
| `profile_id` | STRING | `$.relationships.profile.data.id` | ID of the profile (customer) associated with the event. |
| `metric_id` | STRING | `$.relationships.metric.data.id` | ID of the metric (event type) definition. |
| `attribution_ids` | REPEATED STRING | `$.relationships.attributions.data[*].id` | IDs of campaigns/flows attributed to this event. |

## Flows

Automated marketing flows triggered by specific events or conditions.

- Table name: Flows
- Endpoint: `/flows`
- Klaviyo API reference: [Get Flows](https://developers.klaviyo.com/en/reference/get_flows)

| Field Name | Type | JSON Path | Description |
|---|---|---|---|
| `type` | STRING | `$.type` | Resource type (always `flow`). |
| `id` | STRING | `$.id` | Unique identifier for the flow. |
| `name` | STRING | `$.attributes.name` | Name of the flow. |
| `status` | STRING | `$.attributes.status` | Operational status (e.g., live, draft). |
| `archived` | BOOLEAN | `$.attributes.archived` | Whether the flow is archived. |
| `created` | TIMESTAMP | `$.attributes.created` | Creation timestamp. |
| `updated` | TIMESTAMP | `$.attributes.updated` | Last modification timestamp. |
| `trigger_type` | STRING | `$.attributes.trigger_type` | Mechanism triggering the flow (e.g., "Added to List", "Metric"). |
| `flow_actions_ids` | REPEATED STRING | `$.relationships.flow-actions.data[*].id` | IDs of the actions (steps) within this flow. |
| `tag_ids` | REPEATED STRING | `$.relationships.tags.data[*].id` | IDs of tags assigned to this flow. |

## Forms

Signup forms for collecting subscriber information.

- Table name: Forms
- Endpoint: `/forms`
- Klaviyo API reference: [Get Forms](https://developers.klaviyo.com/en/reference/get_forms)

| Field Name | Type | JSON Path | Description |
|---|---|---|---|
| `type` | STRING | `$.type` | Resource type (always `form`). |
| `id` | STRING | `$.id` | Unique identifier for the form. |
| `name` | STRING | `$.attributes.name` | Name of the form. |
| `status` | STRING | `$.attributes.status` | Status of the form (e.g., live, draft). |
| `ab_test` | BOOLEAN | `$.attributes.ab_test` | Whether the form is running an A/B test. |
| `created_at` | TIMESTAMP | `$.attributes.created_at` | Creation timestamp. |
| `updated_at` | TIMESTAMP | `$.attributes.updated_at` | Last modification timestamp. |

## Images

Images uploaded to Klaviyo for use in campaigns and templates.

- Table name: Images
- Endpoint: `/images`
- Klaviyo API reference: [Get Images](https://developers.klaviyo.com/en/reference/get_images)

| Field Name | Type | JSON Path | Description |
|---|---|---|---|
| `type` | STRING | `$.type` | Resource type (always `image`). |
| `id` | STRING | `$.id` | Unique identifier. |
| `name` | STRING | `$.attributes.name` | Filename or name of the image. |
| `image_url` | STRING | `$.attributes.image_url` | Public URL to access the image. |
| `format` | STRING | `$.attributes.format` | Image file format (e.g., jpeg, png). |
| `size` | FLOAT | `$.attributes.size` | File size in bytes. |
| `hidden` | BOOLEAN | `$.attributes.hidden` | Whether the image is hidden in the UI. |
| `updated_at` | TIMESTAMP | `$.attributes.updated_at` | Last modification timestamp. |

## Lists

Static lists of contacts/profiles.

- Table name: Lists
- Endpoint: `/lists`
- Klaviyo API reference: [Get Lists](https://developers.klaviyo.com/en/reference/get_lists)

| Field Name | Type | JSON Path | Description |
|---|---|---|---|
| `type` | STRING | `$.type` | Resource type (always `list`). |
| `id` | STRING | `$.id` | Unique identifier for the list. |
| `name` | STRING | `$.attributes.name` | Name of the contact list. |
| `created` | TIMESTAMP | `$.attributes.created` | Creation timestamp. |
| `updated` | TIMESTAMP | `$.attributes.updated` | Last modification timestamp. |
| `opt_in_process` | STRING | `$.attributes.opt_in_process` | Opt-in setting (e.g., 'single_opt_in' or 'double_opt_in'). |
| `tag_ids` | REPEATED STRING | `$.relationships.tags.data[*].id` | IDs of tags assigned to this list. |
| `flow_triggers_ids` | REPEATED STRING | `$.relationships.flow-triggers.data[*].id` | IDs of flows triggered by adding profiles to this list. |

## Metrics

Types of events that can be tracked (e.g., "Received Email").

- Table name: Metrics
- Endpoint: `/metrics`
- Klaviyo API reference: [Get Metrics](https://developers.klaviyo.com/en/reference/get_metrics)

| Field Name | Type | JSON Path | Description |
|---|---|---|---|
| `type` | STRING | `$.type` | Resource type (always `metric`). |
| `id` | STRING | `$.id` | Unique identifier (e.g., 6-char code for generic, long UUID for custom). |
| `name` | STRING | `$.attributes.name` | Human-readable name (e.g., "Placed Order"). |
| `created` | TIMESTAMP | `$.attributes.created` | Creation timestamp. |
| `updated` | TIMESTAMP | `$.attributes.updated` | Last modification timestamp. |
| `integration` | JSON | `$.attributes.integration` | Info about the integration providing this metric (e.g., name, category, image). |
| `flow_triggers_ids` | REPEATED STRING | `$.relationships.flow-triggers.data[*].id` | IDs of flows triggered by this metric. |

## Profiles

Comprehensive customer profiles containing attributes and activity history.

- Table name: Profiles
- Endpoint: `/profiles`
- Klaviyo API reference: [Get Profiles](https://developers.klaviyo.com/en/reference/get_profiles)

| Field Name | Type | JSON Path | Description |
|---|---|---|---|
| `type` | STRING | `$.type` | Resource type (always `profile`). |
| `id` | STRING | `$.id` | Unique Klaviyo ID for the profile. |
| `email` | STRING | `$.attributes.email` | Primary email address. |
| `phone_number` | STRING | `$.attributes.phone_number` | Phone number in E.164 format. |
| `external_id` | STRING | `$.attributes.external_id` | ID from an external system. |
| `first_name` | STRING | `$.attributes.first_name` | First name. |
| `last_name` | STRING | `$.attributes.last_name` | Last name. |
| `organization` | STRING | `$.attributes.organization` | Company or organization name. |
| `locale` | STRING | `$.attributes.locale` | Locale/Language setting. |
| `title` | STRING | `$.attributes.title` | Job title. |
| `image` | STRING | `$.attributes.image` | Profile image URL. |
| `created` | TIMESTAMP | `$.attributes.created` | Profile creation timestamp. |
| `updated` | TIMESTAMP | `$.attributes.updated` | Last update timestamp. |
| `last_event_date` | TIMESTAMP | `$.attributes.last_event_date` | Timestamp of the most recent event. |
| `address1` | STRING | `$.attributes.location.address1` | Address line 1. |
| `address2` | STRING | `$.attributes.location.address2` | Address line 2. |
| `city` | STRING | `$.attributes.location.city` | City. |
| `country` | STRING | `$.attributes.location.country` | Country. |
| `latitude` | STRING | `$.attributes.location.latitude` | Latitude coordinates. |
| `longitude` | STRING | `$.attributes.location.longitude` | Longitude coordinates. |
| `region` | STRING | `$.attributes.location.region` | State or region. |
| `zip` | STRING | `$.attributes.location.zip` | Postal or Zip code. |
| `timezone` | STRING | `$.attributes.location.timezone` | Timezone. |
| `ip` | STRING | `$.attributes.location.ip` | IP address. |
| `properties` | JSON | `$.attributes.properties` | Custom properties key-value pairs. |
| `email_marketing_can_receive_email_marketing` | BOOLEAN | `$.attributes.subscriptions.email.marketing.can_receive_email_marketing` | Whether profile can receive email marketing. |
| `email_marketing_consent` | STRING | `$.attributes.subscriptions.email.marketing.consent` | Consent status (e.g., SUBSCRIBED, UNSUBSCRIBED). |
| `email_marketing_consent_timestamp` | TIMESTAMP | `$.attributes.subscriptions.email.marketing.consent_timestamp` | When consent was given. |
| `email_marketing_last_updated` | TIMESTAMP | `$.attributes.subscriptions.email.marketing.last_updated` | When email consent was last updated. |
| `email_marketing_method` | STRING | `$.attributes.subscriptions.email.marketing.method` | Method of consent (e.g., FORM). |
| `email_marketing_method_detail` | STRING | `$.attributes.subscriptions.email.marketing.method_detail` | Specific source of method. |
| `email_marketing_custom_method_detail` | STRING | `$.attributes.subscriptions.email.marketing.custom_method_detail` | Custom details for consent method. |
| `email_marketing_double_optin` | BOOLEAN | `$.attributes.subscriptions.email.marketing.double_optin` | Whether double opt-in was completed. |
| `sms_marketing_can_receive_sms_marketing` | BOOLEAN | `$.attributes.subscriptions.sms.marketing.can_receive_sms_marketing` | Whether profile can receive SMS marketing. |
| `sms_marketing_consent` | STRING | `$.attributes.subscriptions.sms.marketing.consent` | SMS consent status. |
| `sms_marketing_consent_timestamp` | TIMESTAMP | `$.attributes.subscriptions.sms.marketing.consent_timestamp` | When SMS consent was given. |
| `sms_marketing_last_updated` | TIMESTAMP | `$.attributes.subscriptions.sms.marketing.last_updated` | When SMS consent was last updated. |
| `sms_marketing_method` | STRING | `$.attributes.subscriptions.sms.marketing.method` | SMS consent method. |
| `sms_marketing_method_detail` | STRING | `$.attributes.subscriptions.sms.marketing.method_detail` | Details of SMS consent method. |
| `sms_transactional_can_receive_sms_transactional` | BOOLEAN | `$.attributes.subscriptions.sms.transactional.can_receive_sms_transactional` | Whether profile can receive transactional SMS. |
| `sms_transactional_consent` | STRING | `$.attributes.subscriptions.sms.transactional.consent` | Transactional SMS consent status. |
| `sms_transactional_consent_timestamp` | TIMESTAMP | `$.attributes.subscriptions.sms.transactional.consent_timestamp` | When transactional consent was given. |
| `sms_transactional_last_updated` | TIMESTAMP | `$.attributes.subscriptions.sms.transactional.last_updated` | When transactional status was last updated. |
| `sms_transactional_method` | STRING | `$.attributes.subscriptions.sms.transactional.method` | Transactional SMS method. |
| `sms_transactional_method_detail` | STRING | `$.attributes.subscriptions.sms.transactional.method_detail` | Transactional SMS method detail. |
| `mobile_push_can_receive_push_marketing` | BOOLEAN | `$.attributes.subscriptions.mobile_push.marketing.can_receive_push_marketing` | Whether profile can receive push marketing. |
| `mobile_push_consent` | STRING | `$.attributes.subscriptions.mobile_push.marketing.consent` | Push consent status. |
| `mobile_push_consent_timestamp` | TIMESTAMP | `$.attributes.subscriptions.mobile_push.marketing.consent_timestamp` | When push consent was given. |
| `predictive_analytics_historic_number_of_orders` | FLOAT | `$.attributes.predictive_analytics.historic_number_of_orders` | Total historical orders. |
| `predictive_analytics_predicted_number_of_orders` | FLOAT | `$.attributes.predictive_analytics.predicted_number_of_orders` | Predicted future orders. |
| `predictive_analytics_average_days_between_orders` | FLOAT | `$.attributes.predictive_analytics.average_days_between_orders` | Avg days between orders. |
| `predictive_analytics_average_order_value` | FLOAT | `$.attributes.predictive_analytics.average_order_value` | Historic average order value. |
| `predictive_analytics_historic_clv` | FLOAT | `$.attributes.predictive_analytics.historic_clv` | Historic Customer Lifetime Value. |
| `predictive_analytics_predicted_clv` | FLOAT | `$.attributes.predictive_analytics.predicted_clv` | Predicted Customer Lifetime Value. |
| `predictive_analytics_total_clv` | FLOAT | `$.attributes.predictive_analytics.total_clv` | Historic + Predicted CLV. |
| `predictive_analytics_churn_probability` | FLOAT | `$.attributes.predictive_analytics.churn_probability` | Probability of churn (0-1). |
| `predictive_analytics_expected_date_of_next_order` | TIMESTAMP | `$.attributes.predictive_analytics.expected_date_of_next_order` | Predicted date of next order. |
| `email_marketing_suppression_reason` | REPEATED JSON | `$.attributes.subscriptions.email.marketing.suppression[*`\] | Reasons for email suppression (e.g., bounced). |
| `email_marketing_list_suppressions_reason` | REPEATED JSON | `$.attributes.subscriptions.email.marketing.list_suppressions[*`\] | List-specific suppression reasons. |
| `push_tokens_ids` | REPEATED STRING | `$.relationships.push-tokens.data[*].id` | Associated push token IDs. |

## Reviews

Product reviews submitted by customers.

- Table name: Reviews
- Endpoint: `/reviews`
- Klaviyo API reference: [Get Reviews](https://developers.klaviyo.com/en/reference/get_reviews)

| Field Name | Type | JSON Path | Description |
|---|---|---|---|
| `type` | STRING | `$.type` | Resource type (always `review`). |
| `id` | STRING | `$.id` | Unique identifier for the review. |
| `email` | STRING | `$.attributes.email` | Email of the reviewer. |
| `value` | STRING | `$.attributes.status.value` | Status value (e.g., published, rejected). |
| `reason` | STRING | `$.attributes.status.rejection_reason.reason` | Reason for rejection if applicable. |
| `status_explanation` | STRING | `$.attributes.status.rejection_reason.status_explanation` | Detailed explanation of status. |
| `verified` | BOOLEAN | `$.attributes.verified` | Whether the purchase was verified. |
| `review_type` | STRING | `$.attributes.review_type` | Type of review (e.g., product review). |
| `created` | TIMESTAMP | `$.attributes.created` | Creation timestamp. |
| `updated` | TIMESTAMP | `$.attributes.updated` | Last modification timestamp. |
| `images` | REPEATED STRING | `$.attributes.images[*`\] | URLs of images attached to the review. |
| `product_url` | STRING | `$.attributes.product.url` | URL of the reviewed product. |
| `product_name` | STRING | `$.attributes.product.name` | Name of the product. |
| `product_image_url` | STRING | `$.attributes.product.image_url` | Image URL of the product. |
| `product_external_id` | STRING | `$.attributes.product.external_id` | External ID of the product. |
| `rating` | INTEGER | `$.attributes.rating` | Rating score. |
| `author` | STRING | `$.attributes.author` | Name of the reviewer. |
| `content` | STRING | `$.attributes.content` | Text content of the review. |
| `title` | STRING | `$.attributes.title` | Title of the review. |
| `smart_quote` | STRING | `$.attributes.smart_quote` | Highlighted quote from the review. |
| `public_reply_content` | STRING | `$.attributes.public_reply.content` | Content of the merchant's public reply. |
| `public_reply_author` | STRING | `$.attributes.public_reply.author` | Author of the reply. |
| `public_reply_updated` | STRING | `$.attributes.public_reply.updated` | Timestamp of reply update. |
| `event_ids` | REPEATED STRING | `$.relationships.events.data[*].id` | Associated event IDs. |

## Segments

Dynamic groups of profiles based on specific criteria.

- Table name: Segments
- Endpoint: `/segments`
- Klaviyo API reference: [Get Segments](https://developers.klaviyo.com/en/reference/get_segments)

| Field Name | Type | JSON Path | Description |
|---|---|---|---|
| `type` | STRING | `$.type` | Resource type (always `segment`). |
| `id` | STRING | `$.id` | Unique identifier for the segment. |
| `name` | STRING | `$.attributes.name` | Segment name. |
| `created` | TIMESTAMP | `$.attributes.created` | Creation timestamp. |
| `updated` | TIMESTAMP | `$.attributes.updated` | Last modification timestamp. |
| `is_active` | BOOLEAN | `$.attributes.is_active` | Whether the segment is active. |
| `is_processing` | BOOLEAN | `$.attributes.is_processing` | Whether the segment is being processed. |
| `is_starred` | BOOLEAN | `$.attributes.is_starred` | Whether the segment is starred/favorited. |
| `tag_ids` | REPEATED STRING | `$.relationships.tags.data[*].id` | IDs of associated tags. |
| `flow_triggers_ids` | REPEATED STRING | `$.relationships.flow-triggers.data[*].id` | IDs of flows triggered by this segment. |
| `condition_groups` | REPEATED RECORD | `$.attributes.definition.condition_groups[*`\] | Groups of logic conditions defining the segment. |

### ConditionGroup

| Field Name | Type | JSON Path | Description |
|---|---|---|---|
| `conditions` | REPEATED Condition | `conditions[*`\] | List of individual conditions within the group. |

#### Condition

| Field Name | Type | JSON Path | Description |
|---|---|---|---|
| `type` | STRING | `type` | Type of condition (e.g., profile-property). |
| `value` | JSON | `N/A` | Condition value/configuration. |

## Tags

Tags used to organize campaigns, flows, and lists.

- Table name: Tags
- Endpoint: `/tags`
- Klaviyo API reference: [Get Tags](https://developers.klaviyo.com/en/reference/get_tags)

| Field Name | Type | JSON Path | Description |
|---|---|---|---|
| `type` | STRING | `$.type` | Resource type (always `tag`). |
| `id` | STRING | `$.id` | Unique tag identifier. |
| `name` | STRING | `$.attributes.name` | Name of the tag. |
| `tag_group_id` | STRING | `$.relationships.tag-group.data.id` | ID of the tag group this tag belongs to. |

## Templates

Email and message templates.

- Table name: Templates
- Endpoint: `/templates`
- Klaviyo API reference: [Get Templates](https://developers.klaviyo.com/en/reference/get_templates)

| Field Name | Type | JSON Path | Description |
|---|---|---|---|
| `type` | STRING | `$.type` | Resource type (always `template`). |
| `id` | STRING | `$.id` | Unique identifier. |
| `name` | STRING | `$.attributes.name` | Template name. |
| `editor_type` | STRING | `$.attributes.editor_type` | Editor used (e.g., drag-and-drop). |
| `html` | STRING | `$.attributes.html` | HTML content |
| `text` | STRING | `$.attributes.text` | Text version of the template. |
| amp | STRING | `$.attributes.amp` | AMP version of the template. |
| `created` | TIMESTAMP | `$.attributes.created` | Creation timestamp. |
| `updated` | TIMESTAMP | `$.attributes.updated` | Last update timestamp. |

## WebFeeds

Web feeds used to populate content in messages.

- Table name: WebFeeds
- Endpoint: `/web-feeds`
- Klaviyo API reference: [Get Web Feeds](https://developers.klaviyo.com/en/reference/get_web_feeds)

| Field Name | Type | JSON Path | Description |
|---|---|---|---|
| `type` | STRING | `$.type` | Resource type (always `web-feed`). |
| `id` | STRING | `$.id` | Unique identifier. |
| `name` | STRING | `$.attributes.name` | Feed name. |
| `url` | STRING | `$.attributes.url` | Feed Source URL. |
| `request_method` | STRING | `$.attributes.request_method` | HTTP method (GET/POST). |
| `content_type` | STRING | `$.attributes.content_type` | Content type (e.g., JSON). |
| `status` | STRING | `$.attributes.status` | Status of the feed. |
| `created` | TIMESTAMP | `$.attributes.created` | Creation timestamp. |
| `updated` | TIMESTAMP | `$.attributes.updated` | Last update timestamp. |

## DataSources

Sources of data integrated into Klaviyo.

- Table name: DataSources
- Endpoint: `/data-sources`
- Klaviyo API reference: [Get Data Sources](https://developers.klaviyo.com/en/reference/get_data_sources)

| Field Name | Type | JSON Path | Description |
|---|---|---|---|
| `type` | STRING | `$.type` | Resource type (always `data-source`). |
| `id` | STRING | `$.id` | Unique identifier. |
| `title` | STRING | `$.attributes.title` | Title of the data source. |
| `visibility` | STRING | `$.attributes.visibility` | Visibility level. |
| `description` | STRING | `$.attributes.description` | Description text. |

## Campaigns

Marketing campaigns sent to lists or segments.

- Table name: Campaigns
- Endpoint: `/campaigns`
- Klaviyo API reference: [Get Campaigns](https://developers.klaviyo.com/en/reference/get_campaigns)

| Field Name | Type | JSON Path | Description |
|---|---|---|---|
| `type` | STRING | `$.type` | Resource type (always `campaign`). |
| `id` | STRING | `$.id` | Unique identifier. |
| `name` | STRING | `$.attributes.name` | Campaign name. |
| `status` | STRING | `$.attributes.status` | Campaign status (e.g., Sent, Scheduling). |
| `archived` | BOOLEAN | `$.attributes.archived` | Whether campaign is archived. |
| `included` | REPEATED STRING | `$.attributes.audiences.included` | IDs of included lists/segments. |
| `excluded` | REPEATED STRING | `$.attributes.audiences.excluded` | IDs of excluded lists/segments. |
| `send_options` | JSON | `$.attributes.send_options` | Configuration for sending (e.g., smart sending). |
| `tracking_options` | JSON | `$.attributes.tracking_options` | Configuration for tracking (e.g., utm params). |
| `send_strategy` | JSON | `$.attributes.send_strategy` | Strategy for delivery time. |
| `created_at` | TIMESTAMP | `$.attributes.created_at` | Creation timestamp. |
| `scheduled_at` | TIMESTAMP | `$.attributes.scheduled_at` | When the campaign is scheduled to send. |
| `updated_at` | TIMESTAMP | `$.attributes.updated_at` | Last update timestamp. |
| `send_time` | TIMESTAMP | `$.attributes.send_time` | Actual time sent. |
| `tag_ids` | REPEATED STRING | `$.relationships.tags.data[*].id` | IDs of associated tags. |
| `campaign_message_ids` | REPEATED STRING | `$.relationships.campaign-messages.data[*].id` | IDs of messages contained in this campaign. |

## CampaignMessages

Individual messages (email/SMS) within a campaign.

- Table name: CampaignMessages
- Endpoint: `/campaign-messages`
- Klaviyo API reference: [Get Campaign Message](https://developers.klaviyo.com/en/reference/get_campaign_message)

| Field Name | Type | JSON Path | Description |
|---|---|---|---|
| `type` | STRING | `$.type` | Resource type (always `campaign-message`). |
| `id` | STRING | `$.id` | Unique identifier. |
| `definition` | JSON | `$.attributes.definition` | Message content and configuration. |
| `send_times` | REPEATED JSON | `$.attributes.send_times[*`\] | Scheduled send times. |
| `created_at` | TIMESTAMP | `$.attributes.created_at` | Creation timestamp. |
| `updated_at` | TIMESTAMP | `$.attributes.updated_at` | Last update timestamp. |
| `campaign_id` | STRING | `$.relationships.campaign.data.id` | ID of parent campaign. |
| `template_id` | STRING | `$.relationships.template.data.id` | ID of used template. |
| `image_id` | STRING | `$.relationships.image.data.id` | ID of attached image. |

## Categories

Product categories from your catalog.

- Table name: Categories
- Endpoint: `/catalog-categories`
- Klaviyo API reference: [Get Catalog Categories](https://developers.klaviyo.com/en/reference/get_catalog_categories)

| Field Name | Type | JSON Path | Description |
|---|---|---|---|
| `type` | STRING | `$.type` | Resource type. |
| `id` | STRING | `$.id` | Unique identifier. |
| `name` | STRING | `$.attributes.name` | Category name. |
| `external_id` | STRING | `$.attributes.external_id` | External system ID. |
| `updated` | TIMESTAMP | `$.attributes.updated` | Last update timestamp. |

## Items

Individual products or items in your catalog.

- Table name: Items
- Endpoint: `/catalog-items`
- Klaviyo API reference: [Get Catalog Items](https://developers.klaviyo.com/en/reference/get_catalog_items)

| Field Name | Type | JSON Path | Description |
|---|---|---|---|
| `type` | STRING | `$.type` | Resource type. |
| `id` | STRING | `$.id` | Unique identifier. |
| `external_id` | STRING | `$.attributes.external_id` | External system ID. |
| `title` | STRING | `$.attributes.title` | Item title/name. |
| `description` | STRING | `$.attributes.description` | Description available. |
| `price` | FLOAT | `$.attributes.price` | Price of the item. |
| `url` | STRING | `$.attributes.url` | URL to the item. |
| `image_full_url` | STRING | `$.attributes.image_full_url` | URL of full-size image. |
| `image_thumbnail_url` | STRING | `$.attributes.image_thumbnail_url` | URL of thumbnail image. |
| `images` | REPEATED STRING | `$.attributes.images[*`\] | List of additional image URLs. |
| `custom_metadata` | JSON | `$.attributes.custom_metadata` | Custom metadata key-values. |
| `published` | BOOLEAN | `$.attributes.published` | Publication status. |
| `created` | TIMESTAMP | `$.attributes.created` | Creation timestamp. |
| `updated` | TIMESTAMP | `$.attributes.updated` | Last update timestamp. |
| `variants_ids` | REPEATED STRING | `$.relationships.variants.data[*].id` | IDs of variants for this item. |

## Variants

Specific variants of catalog items (e.g., sizes, colors).

- Table name: Variants
- Endpoint: `/catalog-variants`
- Klaviyo API reference: [Get Catalog Variants](https://developers.klaviyo.com/en/reference/get_catalog_variants)

| Field Name | Type | JSON Path | Description |
|---|---|---|---|
| `type` | STRING | `$.type` | Resource type. |
| `id` | STRING | `$.id` | Unique identifier. |
| `external_id` | STRING | `$.attributes.external_id` | External system ID. |
| `title` | STRING | `$.attributes.title` | Variant title. |
| `description` | STRING | `$.attributes.description` | Description available. |
| `sku` | STRING | `$.attributes.sku` | Stock Keeping Unit. |
| `inventory_policy` | FLOAT | `$.attributes.inventory_policy` | Policy for inventory management. |
| `inventory_quantity` | FLOAT | `$.attributes.inventory_quantity` | Current stock quantity. |
| `price` | FLOAT | `$.attributes.price` | Price. |
| `url` | STRING | `$.attributes.url` | URL to variant. |
| `image_full_url` | BOOLEAN | `$.attributes.image_full_url` | Full image URL available (Boolean). |
| `image_thumbnail_url` | STRING | `$.attributes.image_thumbnail_url` | Thumbnail image URL. |
| `images` | REPEATED STRING | `$.attributes.images[*`\] | List of images. |
| `published` | BOOLEAN | `$.attributes.published` | Publication status. |
| `created` | TIMESTAMP | `$.attributes.created` | Creation timestamp. |
| `updated` | TIMESTAMP | `$.attributes.updated` | Last update timestamp. |