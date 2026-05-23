# Stream sharing with Pub/Sub

You can share Pub/Sub topics through BigQuery sharing (formerly
Analytics Hub) to curate and distribute a library of streaming data across
multiple internal and external organizational boundaries. Real-time streaming data
is shared using BigQuery sharing exchanges and listings, which lets you
logically categorize and group large sets of Pub/Sub topics and
provision access at scale.

You can share streaming data to do the following:

- **Financial services** :
  - Share rapidly changing instrument prices, quotes, and orders in real time.
  - Detect money laundering and payment fraud.
  - Support trading risk computations.
- **Retail and consumer packaged goods (CPG)** :
  - Manage store inventory in real time.
  - Personalize marketing and customer support.
  - Adjust prices dynamically.
  - Monitor social media channels.
  - Optimize physical store layouts.
- **Healthcare** :
  - Power predictive algorithms to monitor patients and analyze risk in real time.
  - Monitor vitals with wearable medical devices.
  - Automatically acquire, structure, store, and process data from patients, healthcare professionals, and facility administration.
- **Telecommunications** :
  - Monitor network health and predict failures.
  - Discover user behavior patterns to better locate devices and antennas.

## Required roles

The roles for sharing Pub/Sub topics are analogous to sharing BigQuery datasets:

- **Administrator** : manages permissions, visibility, and membership of exchanges and listings. This role is analogous to the [Analytics Hub Admin](https://docs.cloud.google.com/bigquery/docs/analytics-hub-grant-roles#ah-admin-role) (`roles/analyticshub.admin`) IAM role.
- **Shared topic publisher** : creates, manages, and grants access to listings of the shared Pub/Sub topics. This role is analogous to the [Analytics Hub Publisher or Analytics Hub Listing Admin](https://docs.cloud.google.com/bigquery/docs/analytics-hub-grant-roles#ah-publisher-role) IAM role. The publisher is a user, not the [publisher application](https://docs.cloud.google.com/pubsub/docs/publisher) that creates and sends messages to the topic.
- **Shared topic subscriber** : subscribes to the shared topic listing. Shared topic subscribers can configure how topic messages are delivered. This role is analogous to the [Analytics Hub Subscriber](https://docs.cloud.google.com/bigquery/docs/analytics-hub-grant-roles#ah-subscriber-role) or [Analytics Hub Subscription Owner](https://docs.cloud.google.com/bigquery/docs/analytics-hub-grant-roles#ah-subscription-owner-role) IAM role. Depending on how the message data is delivered, additional permissions may be required, such as [Pub/Sub service account permissions](https://docs.cloud.google.com/pubsub/docs/bigquery#service_account_permissions) for writing messages to BigQuery.

## Architecture

The following diagram describes how publishers and subscribers of
Pub/Sub resources interact with BigQuery sharing:
![Workflow illustrating interaction between shared topic publishers and BigQuery sharing.](https://docs.cloud.google.com/static/bigquery/images/analytics-hub-pubsub-architecture.svg) **Figure 1.** Shared topic publisher and subscriber workflow with Pub/Sub topics.

<br />

Shared topics

<br />

<br />

A shared topic is the unit for sharing a Pub/Sub topic in BigQuery sharing. As a shared topic publisher, you create or use an existing Pub/Sub topic to distribute message data to subscribers. BigQuery sharing doesn't replicate the source Pub/Sub topic.

<br />

<br />

Listings

<br />

<br />

A listing is created when a publisher adds a shared topic to an exchange. It references the shared topic.

<br />

<br />

Exchanges

<br />

<br />

An exchange is a logical grouping of listings that reference shared topics.

<br />

<br />

Linked Pub/Sub subscriptions

<br />

<br />

When subscribing to a listing with a shared topic, a linked Pub/Sub subscription is created in the shared topic subscriber project. The shared topic publisher's project also represents a Pub/Sub subscription as a Pub/Sub subscription and a Sharing listing subscription.

<br />

## Limitations

Stream sharing with Pub/Sub has the following limitations:

- A shared topic supports a maximum of 10,000 Pub/Sub subscriptions. This limit includes linked Pub/Sub subscriptions and Pub/Sub subscriptions created outside of Sharing, for example, those created directly from Pub/Sub.
- Shared topics are indexed in Data Catalog (deprecated) and Knowledge Catalog, but you cannot filter specifically for their resource type.

- Usage metrics are captured and displayed in the Metrics dashboard for
  shared Pub/Sub topics and subscriptions, but are not
  available in `INFORMATION_SCHEMA.SHARED_DATASET_USAGE`.

  To monitor metrics, see [Monitor Pub/Sub in
  Cloud Monitoring](https://docs.cloud.google.com/pubsub/docs/monitoring).

  These metrics can include the following:
  - Publish message count
  - Publish requests
  - Publish throughput in bytes
  - Top 5 subscriptions
  - Ingested bytes
  - Others

### Enable the Analytics Hub API

To enable the Analytics Hub API, do the following:

### Console

Go to the API library and enable the Analytics Hub API for your
Google Cloud project.

[Enable the Analytics Hub API](https://console.cloud.google.com/apis/library/analyticshub.googleapis.com)

### gcloud

Run the [`gcloud services enable`](https://docs.cloud.google.com/sdk/gcloud/reference/services/enable)
command:

```
gcloud services enable analyticshub.googleapis.com
```

### Enable the Pub/Sub API

Go to the API library and enable the Pub/Sub API for your
Google Cloud project.

[Enable the Pub/Sub API](https://console.cloud.google.com/apis/library/pubsub.googleapis.com)

## Shared topic publisher workflows

As a shared topic publisher, you can do the following:

- Add shared topics to an exchange by creating a listing.
- Update a listing.
- Delete a listing.
- Share a listing.
- Manage subscriptions to your listing.
- Remove subscribers from your listing.

### Additional publisher permissions

To perform shared topic publisher tasks, you must have the
[Analytics Hub Publisher role](https://docs.cloud.google.com/bigquery/docs/analytics-hub-grant-roles#ah-publisher-role)
(`roles/analyticshub.publisher`) on an exchange or listing. To view all the
[data exchanges](https://docs.cloud.google.com/bigquery/docs/analytics-hub-introduction#data_exchanges)
across projects in an organization that you have access to, you must have the
`resourcemanager.organizations.get` permission.

In addition, you need the `pubsub.topics.setIamPolicy` permission on the
Pub/Sub topic you want to list. To share topics with a schema
attached, you must grant BigQuery sharing subscribers the
`pubsub.schemas.get` permission on the schema attached to the topic. This
permission lets BigQuery sharing subscribers properly parse messages from
the Pub/Sub subscription.

### Create a listing (to add a shared topic)

To add a shared topic to a listing, select one of the following options:

### Console

1. Go to the **Sharing (Analytics Hub)** page.

   [Go to Sharing (Analytics Hub)](https://console.cloud.google.com/bigquery/analytics-hub)

   The page lists all data exchanges that you can access.
2. Click the data exchange name where you want to create the listing.

3. Click
   **Create listing**.

4. On the **Create listing** page, select "**Pub/Sub Topic** " from the
   **Resource type** list.

5. Select an existing Pub/Sub topic from the **Shared topic**
   list, or click **Create a topic**.

6. On the **Listing details** page, in the **Display name** field, enter the
   listing name.

7. Enter the following optional details:

   - **Category** : select up to two categories that best represent your
     listing. Shared topic subscribers can
     [filter listings](https://docs.cloud.google.com/bigquery/docs/analytics-hub-view-subscribe-listings#discover-listings)
     based on these categories.

   - **Data affinity** : the region or regions that the shared topic publisher
     uses to publish data. This information helps shared topic subscribers
     minimize or avoid Pub/Sub network egress costs by reading
     data from the same region. For more information about egress costs, see
     [Data transfer costs](https://cloud.google.com/pubsub/pricing#egress_costs).

   - **Icon**: an icon for your listing. PNG and JPEG file formats are
     supported. Icons must be smaller than 512 KiB and have dimensions of
     no more than 512 x 512 pixels.

   - **Description** : a brief description of your listing. Shared topic
     subscribers can
     [search for listings](https://docs.cloud.google.com/bigquery/docs/analytics-hub-view-subscribe-listings#discover-listings)
     based on the description.

   - **Public Discoverability** : enable public discoverability for your
     listing in the BigQuery sharing catalog. You must also set
     permissions for the listing. Click **Actions** for the exchange, and then
     click **Set permissions** . Grant `allUsers` or `allAuthenticatedusers` the
     [Analytics Hub Viewer role](https://docs.cloud.google.com/bigquery/docs/analytics-hub-grant-roles#ah-subscriber-role)
     (`roles/analyticshub.viewer`). Click **Allow Public Access**.

   - **Documentation \> Markdown**: additional information, such as
     links to relevant documentation and other details that
     help shared topic subscribers use your topic.

8. On the **Listing contact information** page, enter the following optional
   details:

   - **Primary contact**: enter an email address or URL for the listing's
     primary contact.

   - **Request access contact**: enter an email address or URL for the intake
     form that shared topic subscribers can use to contact you.

   - **Provider** : expand the **Provider** section and specify details in the
     following fields:

     - **Provider name**: the name of the topic provider.
     - **Provider primary contact**: an email address or URL for the topic provider's primary contact.

     Shared topic subscribers can filter listings based on the data providers.
   - **Publisher** : expand the **Publisher** section and specify details in
     the following fields:

     - **Publisher name**: the name of the shared topic publisher who creates the listing.
     - **Publisher primary contact**: an email address or URL for the shared topic publisher's primary contact.
9. Review the **Listing preview** page.

10. Click **Publish**.

### API

Use the
[`projects.locations.dataExchanges.listings.create` method](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.listings/create).

```
POST https://analyticshub.googleapis.com/v1/projects/PROJECT_ID/location/LOCATION/dataExchanges/DATAEXCHANGE_ID/listings?listingId=LISTING_ID
```

Replace the following:

- `PROJECT_ID`: the project ID that contains the data exchange in which you want to create the listing.
- `LOCATION`: the location for your data exchange. For more information about locations that support Sharing, see [Supported regions](https://docs.cloud.google.com/bigquery/docs/analytics-hub-introduction#supported-regions).
- `DATAEXCHANGE_ID`: the data exchange ID.
- `LISTING_ID`: the listing ID.

In the body of the request, provide the
[listing details](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.listings#resource:-listing).
If the request is successful, the response body contains the listing details.


For more information about the tasks that you can perform on listings using
APIs, see [`projects.locations.dataExchanges.listings` methods](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.listings#methods).

### Update a listing

To update a listing, select one of the following options:

### Console

1. Go to the **Sharing (Analytics Hub)** page.

   [Go to Sharing (Analytics Hub)](https://console.cloud.google.com/bigquery/analytics-hub)
2. Click the data exchange name that contains the listing.

3. Click the listing that you want to update.

4. Click **Edit listing**.

5. Modify the field values. You can modify all values except the listing's
   shared topic.

6. To save changes, click **Publish**.

### API

Use the
[`projects.locations.dataExchanges.listings.patch` method](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.listings/patch).

```
PATCH https://analyticshub.googleapis.com/v1/projects/PROJECT_ID/location/LOCATION/dataExchanges/DATAEXCHANGE_ID/listings/LISTING_ID?updateMask=UPDATEMASK
```

Replace the following:

- `PROJECT_ID`: the project ID that contains the data exchange in which you want to create the listing.
- `LOCATION`: the location for your data exchange. For more information about locations that support Sharing, see [Supported regions](https://docs.cloud.google.com/bigquery/docs/analytics-hub-introduction#supported-regions).
- `DATAEXCHANGE_ID`: the data exchange ID.
- `LISTING_ID`: the listing ID.
- `UPDATEMASK`: the list of fields that you want to update. For updating multiple values, use a comma-separated list.

In the body of the request, specify the updated values.


For more information about the tasks that you can perform on listings using
APIs, see [`projects.locations.dataExchanges.listings` methods](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.listings#methods).

### Delete a listing

You cannot delete listings with shared Pub/Sub topics that have
active subscriptions.
[Revoke all active subscriptions](https://docs.cloud.google.com/bigquery/docs/analytics-hub-stream-sharing#revoke_a_subscription)
before attempting to delete a shared topic listing. You cannot undo a listing
deletion.

To delete a listing, select one of the following options:

### Console

1. Go to the **Sharing (Analytics Hub)** page.

   [Go to Sharing (Analytics Hub)](https://console.cloud.google.com/bigquery/analytics-hub)
2. Click the data exchange name that contains the listing.

3. Click the listing that you want to delete.

4. Click **Delete**.

5. In the **Delete listing?** dialog, confirm deletion by typing `delete`.

6. Click **Delete**.

### API

Use the
[`projects.locations.dataExchanges.listings.delete` method](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.listings/delete).

```
DELETE https://analyticshub.googleapis.com/v1/projects/PROJECT_ID/location/LOCATION/dataExchanges/DATAEXCHANGE_ID/listings/LISTING_ID
```

Replace the following:

- `PROJECT_ID`: the project ID that contains the data exchange in which you want to create the listing.
- `LOCATION`: the location for your data exchange. For more information about locations that support Sharing, see [Supported regions](https://docs.cloud.google.com/bigquery/docs/analytics-hub-introduction#supported-regions).
- `DATAEXCHANGE_ID`: the data exchange ID.
- `LISTING_ID`: the listing ID.


For more information about the tasks that you can perform on listings using
APIs, see [`projects.locations.dataExchanges.listings` methods](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.listings#methods).

### Share a listing

To give users access to a private listing, set an IAM policy for
an individual or a group for that listing. For a commercial listing, your
[data exchange must be public](https://docs.cloud.google.com/bigquery/docs/analytics-hub-manage-exchanges#make-data-exchange-public).
Listings in a public data exchange appear in Sharing for
all
[Google Cloud users (allAuthenticatedUsers)](https://docs.cloud.google.com/iam/docs/principals-overview#all-authenticated-users).
To let users request access for commercial listings, grant them the
[Analytics Hub Viewer role](https://docs.cloud.google.com/bigquery/docs/analytics-hub-grant-roles#ah-subscriber-role)
(`roles/analyticshub.viewer`).

To give users access to view or subscribe to your listing, do the following:

1. Go to the **Sharing (Analytics Hub)** page.

   [Go to Sharing (Analytics Hub)](https://console.cloud.google.com/bigquery/analytics-hub)
2. Click the data exchange name that contains the listing.

3. Click the listing where you want to add shared topic subscribers.

4. Click **Set permissions**.

5. To add principals, click
   **Add principal**.

6. In the **New principals** field, add details based on the listing type:

   - For a private listing, enter the email addresses of the identities to whom you want to grant access.
   - For a public listing, add `allAuthenticatedUsers`.
7. From the **Select a role** menu, select **Analytics Hub**, and then select
   one of the following roles based on the listing type:

   - For a commercial listing, select the **Analytics Hub Viewer** role. This role lets users [view the listing and request access](https://docs.cloud.google.com/bigquery/docs/analytics-hub-view-subscribe-listings#discover-listings).
   - For a private or non-commercial public listing, select the **Analytics Hub Subscriber** role. This role lets users [subscribe to your listing](https://docs.cloud.google.com/bigquery/docs/analytics-hub-view-subscribe-listings#subscribe-listings).
8. Click **Save**.

For more information, see
[Analytics Hub Subscriber and Viewer roles](https://docs.cloud.google.com/bigquery/docs/analytics-hub-grant-roles#ah-subscriber-role).

### Manage subscriptions

To let a user subscribe to a listing with a shared topic, grant them the
[Analytics Hub Subscriber](https://docs.cloud.google.com/bigquery/docs/analytics-hub-grant-roles#ah-subscriber-role)
(`roles/analyticshub.subscriber`) and
[Analytics Hub Subscription Owner](https://docs.cloud.google.com/bigquery/docs/analytics-hub-grant-roles#ah-subscription-owner-role)
(`roles/analyticshub.subscriptionOwner`) roles on a specific listing:

1. Go to the **Sharing (Analytics Hub)** page.

   [Go to Sharing (Analytics Hub)](https://console.cloud.google.com/bigquery/analytics-hub)
2. Click the data exchange name that contains the listing for which you want
   to manage subscriptions.

3. Click the listing for which you want to list shared topic subscribers.

4. Click **Set permissions**.

5. To add principals, click
   **Add principal**.

6. In the **New principals** field, enter the usernames or email addresses of
   the subscribers that you want to add.

7. For **Select a role** , select **Analytics Hub \> Analytics Hub
   Subscriber**.

8. Click
   **Add another role**.

9. For **Select a role** , select **Analytics Hub \> Analytics Hub Subscription Owner**.

10. Click **Save**.

Delete and update subscribers at any time by clicking **Set Permissions**.

### Revoke a subscription

To remove a subscription to a shared topic listing from BigQuery sharing,
select one of the following options:

### Console

1. To list shared topic subscribers for a listing, follow the
   Google Cloud console instructions in
   [View all subscriptions](https://docs.cloud.google.com/bigquery/docs/analytics-hub-manage-listings#view_all_subscriptions).

2. Click the **Subscriptions** tab to view all subscriptions to the data
   exchange.

3. Select the checkbox next to the subscriptions you want to remove, or select
   all subscriptions.

4. In the **Revoke subscription?** dialog, enter `revoke` to confirm.

5. Click **Revoke**.

### API

To remove a subscription, use the
[`projects.locations.subscriptions.revoke`](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.subscriptions/revoke)
method.

```
POST https://analyticshub.googleapis.com/v1/projects/PROJECT_ID/location/LOCATION/subscriptions/SUBSCRIPTION_ID:revoke
```

Replace the following:

- `PROJECT_ID`: the project ID of the subscription that you want to remove.
- `LOCATION`: the location of the subscription.
- `SUBSCRIPTION_ID`: the ID of the subscription that you want to remove.

When you revoke a subscription from BigQuery sharing, a shared topic
subscriber no longer receives message data from the shared topic. The
Pub/Sub subscription is detached from the shared topic. If you
delete a subscription directly from Pub/Sub, the
BigQuery sharing subscription persists and requires cleanup.

## Subscriber workflows

As a BigQuery sharing subscriber, you can view and subscribe to a listing.
Subscribing to a listing of a shared topic creates one linked
Pub/Sub subscription in the subscriber's project. The
Pub/Sub subscription is visible in the shared topic publisher's
project.

### Additional subscriber permissions

You must have the
[Analytics Hub Subscriber](https://docs.cloud.google.com/bigquery/docs/analytics-hub-grant-roles#ah-subscriber-role)
(`roles/analyticshub.subscriber`) role granted at the project, exchange, or
listing-level to perform subscriber tasks.

### Subscribe to a shared topic listing

To subscribe to a listing with a shared topic, select one of the following options:

### Console

1. To view a list of listings you can access, follow the steps in
   [View listings](https://docs.cloud.google.com/bigquery/docs/analytics-hub-view-subscribe-listings#discover-listings).

2. Browse the listings and click a listing to which you want to subscribe. A
   dialog with the listing details appears.

3. Click **Subscribe** to open the **Create subscription** dialog.

4. If you haven't enabled the Analytics Hub API in your project, an error
   message appears with a link to enable the API. Click **Enable Analytics Hub API**.

5. In the **Create subscription** dialog, specify the following details:

   - **Subscription ID**: specify the name of the subscription you are creating.
   - **Delivery type**: select how the message data is delivered.
   - **Message retention duration**: set the message retention duration.
   - **Expiration period**: set when the subscription expires after inactivity, if ever.
   - **Acknowledgement deadline**: set the acknowledgement deadline time.
   - **Subscription filter**: set filter syntax for messages.
   - **Exactly once delivery**: enable exactly once delivery.
   - **Message ordering**: enable message ordering with an order key.
   - **Dead lettering**: enable dead lettering.
   - **Retry policy**: set a retry policy.

   For more information about Pub/Sub subscription properties,
   see
   [subscription properties](https://docs.cloud.google.com/pubsub/docs/subscription-properties).
6. To save your changes, click **Create**. The linked Pub/Sub
   subscription is created in your project.

### API

Use the
[`projects.locations.dataExchanges.listings.subscribe` method](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.listings/subscribe).

```
POST https://analyticshub.googleapis.com/v1/projects/PROJECT_ID/location/LOCATION/dataExchanges/DATAEXCHANGE_ID/listings/LISTING_ID:subscribe
```

Replace the following:

- `PROJECT_ID`: the project ID of the listing that you want to subscribe to.
- `LOCATION`: the location for your listing that you want to subscribe to.
- `DATAEXCHANGE_ID`: the data exchange ID of the listing that you want to subscribe to.
- `LISTING_ID`: the listing ID that you want to subscribe to.

In the request body, specify the Pub/Sub subscription where you
want to create the
[linked Pub/Sub subscription](https://docs.cloud.google.com/bigquery/docs/analytics-hub-introduction#linked_pubsub_subscriptions).
If successful, the response body is empty.

For more information about Pub/Sub subscriptions, see
[subscription overview](https://docs.cloud.google.com/pubsub/docs/subscription-overview).

## Pricing

Pub/Sub topic publishers incur no additional charge to list and
share topics through BigQuery sharing. Shared topic publishers are charged
for the total bytes written (publish throughput) to the shared topic and for
network egress (if applicable). Shared topic subscribers are charged for the
total bytes read (subscribe throughput) from the linked subscription and
for network egress (if applicable). For more details, see
[Pub/Sub pricing](https://cloud.google.com/pubsub/pricing#pubsub).

## VPC Service Controls

For shared topics from projects with VPC Service Controls perimeters, set the
appropriate ingress and egress rules to grant access to shared topic publishers
and subscribers.

## What's next

- Learn about [managing your listings](https://docs.cloud.google.com/bigquery/docs/analytics-hub-introduction).
- Learn about [subscribing to listings](https://docs.cloud.google.com/bigquery/docs/analytics-hub-view-subscribe-listings).
- Learn about [monitoring your Pub/Sub resources](https://docs.cloud.google.com/pubsub/docs/monitoring).
- Learn about [configuring VPC Service Controls for BigQuery sharing](https://docs.cloud.google.com/bigquery/docs/analytics-hub-vpc-sc-rules).