# Manage listings

This document describes how to manage listings in BigQuery sharing
(formerly Analytics Hub). As a BigQuery sharing publisher, you can do the following:

- Create listings in a data exchange for which you have publishing access.
- Update, delete, share, and view usage metrics for listings.
- Manage different BigQuery sharing roles for your listings, such as listing administrators, subscribers, and viewers.
- View all subscribers who subscribed to your listing.
- [Monitor usage](https://docs.cloud.google.com/bigquery/docs/analytics-hub-monitor-listings) of your listings.
- Remove subscribers from your listing.

A listing is a reference to a shared dataset that a publisher lists in a
[data exchange](https://docs.cloud.google.com/bigquery/docs/analytics-hub-introduction#data_exchanges).
A listing can be of the following two types based on the
Identity and Access Management (IAM) policy that's set for the listing and the type of
data exchange that contains the listing:

- **Public listing.** A public listing can be
  [discovered](https://docs.cloud.google.com/bigquery/docs/analytics-hub-view-subscribe-listings#discover-listings)
  and
  [subscribed to](https://docs.cloud.google.com/bigquery/docs/analytics-hub-view-subscribe-listings#subscribe-listings)
  by
  [Google Cloud users (`allAuthenticatedUsers`)](https://docs.cloud.google.com/iam/docs/principals-overview#all-authenticated-users).
  Listings in a public data exchange are public listings. These listings can be
  references to a *free public dataset* or a *commercial dataset* . If the
  listing is of a commercial dataset, BigQuery sharing subscribers can
  either request access to the listing directly from the data provider, or they
  can browse and purchase
  [Google Cloud Marketplace-integrated commercial listings](https://docs.cloud.google.com/bigquery/docs/analytics-hub-cloud-marketplace).

- **Private listing.** A private listing is shared directly with individuals or
  groups. For example, a private listing can reference marketing metrics dataset that you share with other internal teams within your organization. Even though you can
  [allow Google Cloud users (`allAuthenticatedUsers`)](https://docs.cloud.google.com/bigquery/docs/analytics-hub-manage-listings#give_users_access_to_a_listing)
  to subscribe to your listings, the listing will remain private and won't
  [show as a public listing on the BigQuery sharing page](https://docs.cloud.google.com/bigquery/docs/analytics-hub-view-subscribe-listings#discover-listings).
  To share such listings with users, share the listing's URL with them. To make
  a private listing discoverable,
  [make your exchange public](https://docs.cloud.google.com/bigquery/docs/analytics-hub-manage-exchanges#make-data-exchange-public).

> [!NOTE]
> **Note:** Both requesting access and Cloud Marketplace-integrated flows are supported on a single BigQuery sharing listing. This means that you can create a Cloud Marketplace-integrated listing from an existing (offline) commercial listing, without any disruptions to existing subscriptions.

## Before you begin

To get started with BigQuery sharing (formerly Analytics Hub), you need to
enable the Analytics Hub API inside your Google Cloud project.

To enable the Analytics Hub API, you need the following
Identity and Access Management (IAM) permissions:

- `serviceUsage.services.get`
- `serviceUsage.services.list`
- `serviceUsage.services.enable`

The following predefined IAM role includes the
permissions that you need to enable the Analytics Hub API:

- [Service Usage Admin](https://docs.cloud.google.com/service-usage/docs/access-control#serviceusage.serviceUsageAdmin) (`roles/serviceusage.serviceUsageAdmin`)

To enable the Analytics Hub API, select one of the following options:

### Console

Go to the **Analytics Hub API** page and enable the Analytics Hub API for
your Google Cloud project.

[Enable the Analytics Hub API](https://console.cloud.google.com/apis/library/analyticshub.googleapis.com)

### gcloud

Run the [gcloud services enable](https://docs.cloud.google.com/sdk/gcloud/reference/services/enable)
command:

```
gcloud services enable analyticshub.googleapis.com
```

### Required roles

To manage listings and subscriptions, you must have one of the following
BigQuery sharing Identity and Access Management (IAM) roles:

- [Analytics Hub Publisher role](https://docs.cloud.google.com/bigquery/docs/access-control#analyticshub.publisher)
  (`roles/analyticshub.publisher`), which lets you
  create, update, delete, and set IAM policies on your listings.

- [Analytics Hub Listing Admin role](https://docs.cloud.google.com/bigquery/docs/access-control#analyticshub.listingAdmin)
  (`roles/analyticshub.listingAdmin`), which lets you
  update, delete, and set IAM policies on your listings.

- [Analytics Hub Admin role](https://docs.cloud.google.com/bigquery/docs/access-control#analyticshub.admin)
  (`roles/analyticshub.admin`), which lets you
  create, update, delete, and set IAM policies on all listings in
  your data exchange.

For more information, see
[BigQuery sharing IAM roles](https://docs.cloud.google.com/bigquery/docs/analytics-hub-grant-roles#user_roles).
To learn how to grant these roles to other users, see
[Create a listing administrator](https://docs.cloud.google.com/bigquery/docs/analytics-hub-manage-listings#create-listing-administrator).

To create listings or to update replica regions for a listing,
you must have the `bigquery.datasets.get`
and `bigquery.datasets.update` permissions for the datasets where you want
to create or update listings. The following
[BigQuery predefined roles](https://docs.cloud.google.com/bigquery/docs/access-control#bigquery)
contain the `bigquery.datasets.update` permission:

- [BigQuery Data Owner](https://docs.cloud.google.com/bigquery/docs/access-control#bigquery.dataOwner) (`roles/bigquery.dataOwner`)
- [BigQuery Admin](https://docs.cloud.google.com/bigquery/docs/access-control#bigquery.admin) (`roles/bigquery.admin`)

To view all data exchanges across projects in an organization that you have
access to, you must have the `resourcemanager.organizations.get` permission.
There are no
[BigQuery predefined roles](https://docs.cloud.google.com/bigquery/docs/access-control#bigquery)
that contain this permission, so you would need to use an
[IAM custom role](https://docs.cloud.google.com/iam/docs/creating-custom-roles).

## View data exchanges

To view the list of data exchanges in your organization that you have access to,
see
[View data exchanges](https://docs.cloud.google.com/bigquery/docs/analytics-hub-manage-exchanges#view_data_exchanges).
If the data exchange is in another organization, then the
BigQuery sharing administrator must
[share a link to that data exchange](https://docs.cloud.google.com/bigquery/docs/analytics-hub-manage-exchanges#share_a_data_exchange)
with you.

## Create a listing

A listing is a reference to a
[shared dataset](https://docs.cloud.google.com/bigquery/docs/analytics-hub-introduction#shared_datasets)
that a BigQuery sharing publisher lists in a data exchange.

> [!CAUTION]
> **Caution:** We recommend that you don't add your shared datasets in a Google Cloud project with a VPC Service Controls perimeter. If you do so, then you must add the appropriate [ingress and egress rules](https://docs.cloud.google.com/bigquery/docs/analytics-hub-vpc-sc-rules#create_a_listing).

To create a listing, follow these steps:

### Console

1. Go to the **Sharing (Analytics Hub)** page.

   [Go to Sharing (Analytics Hub)](https://console.cloud.google.com/bigquery/analytics-hub)

   A page appears that lists all data exchanges that you can access.
2. Click the data exchange name in which you want to create the listing.

3. Click
   **Create listing**.

4. In the **Configure data** section, in the **Resource type** menu, select
   **BigQuery dataset** or **Pub/Sub Topic**.

   - If you select **BigQuery dataset**, then do the following:

     1. In the **Shared dataset** menu, select an existing dataset, or
        click **Create a dataset** to create a new dataset. Select the
        dataset that you want to list in the data exchange. The dataset must be in the same region as the data exchange. You cannot update this field after the listing is created. The source dataset name and the
        ID of the project that contains the dataset are returned when BigQuery sharing subscribers
        [view the metadata of their linked dataset](https://docs.cloud.google.com/bigquery/docs/analytics-hub-view-subscribe-listings#view-table-metadata).

     2. Optional: To let subscribers
        [share a SQL stored procedure within a listing](https://docs.cloud.google.com/bigquery/docs/analytics-hub-manage-listings#share-stored-procedure-in-listing),
        select **Allow stored procedure sharing**
        ([Preview](https://docs.cloud.google.com/products#product-launch-stages)).

     3. Expand the **Region data availability** menu to make the shared dataset available in additional regions. The
        menu displays the regions where dataset replicas exist with the
        **Ready to use** label. Before configuring the listing for multiple
        regions, verify you've enabled
        [cross-region dataset replication](https://docs.cloud.google.com/bigquery/docs/data-replication#use_dataset_replication)
        on the shared dataset, as you can select only regions where
        cross-region dataset replication is enabled. All other regions are
        labeled as **Unavailable** . If no additional region is selected, the
        listing uses the shared dataset primary region by default, which is
        labeled as **Provider primary**.

     4. In **Data Egress controls**, select the appropriate data egress
        option.

        - To apply data egress restrictions on your shared dataset, but not on query results of your shared dataset, select **Disable copy and export of shared data**.
        - To apply data egress restrictions on your shared dataset and query results of your shared dataset, select **Disable copy and export of query results** , which will automatically set **Disable copy and export of shared data** as well.
        - To apply data API copy and export egress restrictions on your shared dataset, select **Disable copy and export of tables through APIs** , which will automatically set **Disable copy and export of shared data** as well.

        For more information about data egress controls, including
        restrictions, see
        [Data egress options (BigQuery shared datasets only)](https://docs.cloud.google.com/bigquery/docs/analytics-hub-introduction#data_egress).
   - If you select **Pub/Sub Topic** , then in the **Shared topic** menu, you
     can select an existing Pub/Sub topic, or click **Create a topic**
     to create a new topic.

5. In the **Listing details** section, in **Display name**, enter the name of the listing.

6. Enter the following optional details:

   - **Category** : select up to two categories that best represent your
     listing. BigQuery sharing subscribers can
     [filter listings](https://docs.cloud.google.com/bigquery/docs/analytics-hub-view-subscribe-listings#discover-listings)
     based on these categories.

   - **Data affinity** : regions used by the BigQuery sharing publisher
     for publishing the data if you're using a Pub/Sub topic. This
     information is useful for BigQuery sharing subscribers to minimize
     or avoid Pub/Sub network egress costs by reading the data from
     the same region. For more information about egress costs, see
     [Data transfer costs](https://cloud.google.com/pubsub/pricing#egress_costs).

   - **Icon**: an icon for your listing. PNG and JPEG file formats are
     supported. Icons must have a file size of less than 512 KiB and
     dimensions of no more than 512 x 512 pixels.

   - **Description** : a brief description about your listing. Subscribers can
     [search for listings](https://docs.cloud.google.com/bigquery/docs/analytics-hub-view-subscribe-listings#discover-listings)
     based on the description.

   - **Public Discoverability** : enable public discoverability of your
     listing in the BigQuery sharing catalog. If you enable this option,
     grant `allUsers` or `allAuthenticatedUsers` the
     [Analytics Hub Viewer role](https://docs.cloud.google.com/bigquery/docs/access-control#analyticshub.viewer)
     (`roles/analyticshub.viewer`). For more information, see
     [Grant the role for a listing](https://docs.cloud.google.com/bigquery/docs/analytics-hub-grant-roles#grant-role-listing).
     If the exchange is already
     [public](https://docs.cloud.google.com/bigquery/docs/analytics-hub-manage-exchanges#make-data-exchange-public),
     listing permissions are already inherited and no further action is
     required.

     Publicly discoverable exchanges can't have private listings due to
     permission inheritance, but private exchanges can have public listings.
     For public listings to be created, the project the data listing is in
     must have an associated organization and billing account. If you're
     creating a
     [Cloud Marketplace-integrated commercial listing](https://docs.cloud.google.com/bigquery/docs/analytics-hub-cloud-marketplace),
     we recommend making your listing publicly discoverable.
   - **Subscriber Email Logging** : turn on logging of the
     [principal identifiers](https://docs.cloud.google.com/iam/docs/principal-identifiers)
     of all users running jobs and queries on linked datasets. When you enable
     this option, all future subscriptions for this listing have subscriber
     email logging turned on. The logged data is available in the
     `job_principal_subject` field of the
     [`INFORMATION_SCHEMA.SHARED_DATASET_USAGE` view](https://docs.cloud.google.com/bigquery/docs/information-schema-shared-dataset-usage).

     > [!NOTE]
     > **Note:** Once you enable and save email logging, this setting cannot be edited. To disable email logging, delete the listing and recreate it without clicking the **Subscriber Email Logging** toggle.

   - **Documentation \> Markdown**: additional information such as
     links to any relevant documentation and any additional information that
     can help BigQuery sharing subscribers to use your topic.

7. On the **Listing contact information** section, enter the following
   optional details:

   - **Primary contact**: enter an email ID or a URL of the primary contact
     for the listing.

   - **Request access contact**: enter an email ID or URL of the intake form
     for BigQuery sharing subscribers to contact you.

   - **Provider** : expand the **Provider** section and specify details in the
     following fields:

     - **Provider name**: the name of the topic provider.
     - **Provider primary contact**: an email ID or a URL of the topic provider's primary contact.

     Subscribers can filter listings based on the data providers.
   - **Publisher** : expand the **Publisher** section and specify details in
     the following fields:

     - **Publisher name**: the name of the BigQuery sharing publisher who's creating the listing.
     - **Publisher primary contact**: an email ID or a URL of the topic publisher's primary contact.
8. Review the **Listing preview** section.

9. Click **Publish**.

### API

Use the
[`projects.locations.dataExchanges.listings.create` method](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.listings/create).

```
POST https://analyticshub.googleapis.com/v1/projects/PROJECT_ID/location/LOCATION/dataExchanges/DATAEXCHANGE_ID/listings?listingId=LISTING_ID
```

Replace the following:

- `PROJECT_ID`: the project ID that contains the data exchange in which you want to create the listing.
- `LOCATION`: the location for your data exchange. For more information about locations that support BigQuery sharing, see [Supported regions](https://docs.cloud.google.com/bigquery/docs/analytics-hub-introduction#supported-regions).
- `DATAEXCHANGE_ID`: the data exchange ID.
- `LISTING_ID`: the listing ID.

In the body of the request, provide the
[listing details](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.listings#resource:-listing).

To create a listing for multiple regions, specify the additional regions in the `bigqueryDataset.replicaLocations`
field in the request body. Before configuring the listing for multiple
regions, verify you've enabled
[cross-region dataset replication](https://docs.cloud.google.com/bigquery/docs/data-replication#use_dataset_replication)
on the shared dataset. You can select only regions where cross-region dataset
replication is enabled. If this optional field is not included, the listing
is created using the shared dataset's primary region.

If the request is successful, the response body contains details of the
listing. If you enable subscriber email logging with
the `logLinkedDatasetQueryUserEmail` field, the listing response
contains `log_linked_dataset_query_user_email: true`. The logged data
is available in the `job_principal_subject` field of the
[`INFORMATION_SCHEMA.SHARED_DATASET_USAGE` view](https://docs.cloud.google.com/bigquery/docs/information-schema-shared-dataset-usage).


For more information about the tasks that you can perform on listings using
APIs, see [`projects.locations.dataExchanges.listings` methods](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.listings#methods).

### Create a listing from a dataset

You can also create a listing from a dataset by doing the following:

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. Click a dataset to view its details.

3. Click
   **Sharing** \> **Publish as listing**.

   The **Create listing** dialog opens.
4. Select a data exchange to publish this listing in. The data exchange must be
   in the same region as the dataset. For more information about creating a
   data exchange, see
   [create an exchange and set permissions](https://docs.cloud.google.com/bigquery/docs/analytics-hub-manage-exchanges).

5. In the **Shared dataset** menu, select an existing dataset, or click
   **Create a dataset** to create a new dataset. Select the dataset that
   you want to list in the data exchange. The dataset must be in the same
   region as the data exchange. You cannot update this field after the
   listing is created.

   The source dataset name and the ID of the project that contains the
   dataset are returned when BigQuery sharing subscribers
   [view the metadata of their linked dataset](https://docs.cloud.google.com/bigquery/docs/analytics-hub-view-subscribe-listings#view-table-metadata).
6. Optional: To let subscribers
   [share a SQL stored procedure within a listing](https://docs.cloud.google.com/bigquery/docs/analytics-hub-manage-listings#share-stored-procedure-in-listing),
   select **Allow stored procedure sharing**
   ([Preview](https://docs.cloud.google.com/products#product-launch-stages)).

7. Expand the **Region data availability** menu
   to make the shared dataset available in additional regions. The menu displays
   the regions where dataset replicas exist with the **Ready to use**
   label. Before configuring the listing for multiple regions, verify
   you've enabled
   [cross-region dataset replication](https://docs.cloud.google.com/bigquery/docs/data-replication#use_dataset_replication)
   on the shared dataset, as you can select only regions where
   cross-region dataset replication is enabled. All other regions are
   labeled as **Unavailable** . If no additional region is selected, the
   listing uses the shared dataset region by default, which is labeled
   as **Provider primary**.

8. In **Data Egress controls**, select the appropriate data egress option.

   - To apply data egress restrictions on your shared dataset, but not on your query results of your shared dataset, select **Disable copy and
     export of shared data**.
   - To apply data egress restrictions on your shared dataset and query results of your shared dataset, select **Disable copy and export of query results** , which will automatically set **Disable copy and export of shared data** as well.
   - To apply data API copy and export egress restrictions on your shared dataset, select **Disable copy and export of tables through APIs** , which will automatically set **Disable copy and export of shared data** as well.

   For more information about data egress controls, including restrictions, see
   [Data egress options (BigQuery shared datasets only)](https://docs.cloud.google.com/bigquery/docs/analytics-hub-introduction#data_egress).
9. In the **Listing details** section, in **Display name**, enter the name of the listing.

10. Enter the following optional details:

    - **Category** : select up to two categories that best represent your
      listing. BigQuery sharing subscribers can
      [filter listings](https://docs.cloud.google.com/bigquery/docs/analytics-hub-view-subscribe-listings#discover-listings)
      based on these categories.

    - **Data affinity** : region(s) used by the BigQuery sharing publisher
      for publishing the data. This information is useful for
      BigQuery sharing subscribers to minimize or avoid Pub/Sub
      network egress costs by reading the data from the same region. For more
      information about egress costs, see
      [Data transfer costs](https://cloud.google.com/pubsub/pricing#egress_costs).

    - **Icon**: an icon for your listing. PNG and JPEG file formats are
      supported. Icons must have a file size of less than 512 KiB and
      dimensions of no more than 512 x 512 pixels.

    - **Description** : a brief description about your listing.
      BigQuery sharing subscribers can
      [search for listings](https://docs.cloud.google.com/bigquery/docs/analytics-hub-view-subscribe-listings#discover-listings)
      based on the description.

    - **Public Discoverability** : enable public discoverability of your listing
      in the BigQuery sharing catalog. If you enable this option, grant
      `allUsers` or `allAuthenticatedUsers` the
      [Analytics Hub Viewer role](https://docs.cloud.google.com/bigquery/docs/access-control#analyticshub.viewer)
      (`roles/analyticshub.viewer`). For more information, see
      [Grant the role for a listing](https://docs.cloud.google.com/bigquery/docs/analytics-hub-grant-roles#grant-role-listing).
      If the exchange is already
      [public](https://docs.cloud.google.com/bigquery/docs/analytics-hub-manage-exchanges#make-data-exchange-public),
      listing permissions are already inherited and no further action is required.

      Publicly discoverable exchanges can't have private listings due to
      permission inheritance, but private exchanges can have public listings. For
      public listings to be created, the project the data listing is in must have
      an associated organization and billing account. If you're creating a
      [Cloud Marketplace-integrated commercial listing](https://docs.cloud.google.com/bigquery/docs/analytics-hub-cloud-marketplace),
      we recommend making your listing publicly discoverable.
    - **Subscriber Email Logging** : turn on logging of the
      [principal identifiers](https://docs.cloud.google.com/iam/docs/principal-identifiers) of subscribers
      running jobs and queries on this listing's linked dataset for all future
      subscriptions. When you enable this option, only newly created
      subscriptions log the principal identifiers. The logged data is available
      in the `job_principal_subject` field of the
      [`INFORMATION_SCHEMA.SHARED_DATASET_USAGE` view](https://docs.cloud.google.com/bigquery/docs/information-schema-shared-dataset-usage).

      > [!NOTE]
      > **Note:** Once you enable and save email logging, this setting cannot be edited. To disable email logging, delete the listing and recreate it without clicking the **Subscriber Email Logging** toggle.

    - **Documentation \> Markdown**: additional information such as
      links to any relevant documentation and any additional information that can
      help subscribers to use your topic.

11. On the **Listing contact information** section, enter the following optional
    details:

    - **Primary contact**: enter an email ID or a URL of the primary contact for
      the listing.

    - **Request access contact**: enter an email ID or URL of the intake form
      for subscribers to contact you.

    - **Provider** : expand the **Provider** section and specify details in the
      following fields:

      - **Provider name**: the name of the topic provider.
      - **Provider primary contact**: an email ID or a URL of the topic provider's primary contact.

      Subscribers can filter listings based on the data providers.
    - **Publisher** : expand the **Publisher** section and specify details in
      the following fields:

      - **Publisher name**: the name of the BigQuery sharing publisher who's creating the listing.
      - **Publisher primary contact**: an email ID or a URL of the topic publisher's primary contact.
12. Review the **Listing preview** section.

13. Click **Publish**.

### Share a SQL stored procedure within a listing

> [!WARNING]
>
> **Preview**
>
>
> This product or feature is
>
> subject to the "Pre-GA Offerings Terms" in the General Service Terms section
> of the [Service Specific
> Terms](https://docs.cloud.google.com/terms/service-terms#1).
>
> Pre-GA products and features are available "as is" and might have limited support.
>
> For more information, see the
> [launch stage descriptions](https://cloud.google.com/products/#product-launch-stages).

Note: To request support or provide feedback for this feature, contact [bq-data-sharing-feedback@google.com](mailto:bq-data-sharing-feedback@google.com).

<br />

You can share
[SQL stored procedures](https://docs.cloud.google.com/bigquery/docs/procedures)
when creating listings with BigQuery datasets. Since stored
procedures can create, drop, and manipulate tables, as well as invoke
other stored procedures, additional authorization is needed.

#### Subscriber authorization

After subscribing to a listing, the linked stored procedures might not be
executed directly. To ensure that the linked stored procedures can be accessed,
the subscriber must communicate to the provider with the linked dataset name so that
[the provider authorizes the linked stored procedure on the provider resources](https://docs.cloud.google.com/bigquery/docs/analytics-hub-manage-listings#provider-authorization).
In addition, the subscriber must
[authorize the linked shared stored procedure and attach an IAM role](https://docs.cloud.google.com/bigquery/docs/authorized-routines#bq-attach-role)
to the resources that they own in order to read from and write to those
resources.

#### Provider authorization

When a provider creates a listing with a stored procedure, they need to let the
subscriber read from and write to their tables through the linked stored
procedure. To ensure this, do the following:

- For non-read operations, the provider must
  authorize the linked shared stored procedure and
  [attach an IAM role](https://docs.cloud.google.com/bigquery/docs/authorized-routines#bq-attach-role)
  to any of the provider's resources that are accessed by the linked stored
  procedure.

- For read operations, the provider can authorize either the linked shared stored
  procedure (in the subscriber's linked dataset) or their original shared stored
  procedure (in the provider's dataset) and
  [attach an IAM role](https://docs.cloud.google.com/bigquery/docs/authorized-routines#bq-attach-role)
  to any of the provider's resources that are accessed by the linked stored
  procedure.

## Give users access to a listing

If you want to give users access to a private listing, you must set
IAM policy for an individual or a group for that listing. For a
commercial listing, your
[data exchange must be public](https://docs.cloud.google.com/bigquery/docs/analytics-hub-manage-exchanges#make-data-exchange-public).
Listings in a public data exchange appear in BigQuery sharing for all
[Google Cloud users (`allAuthenticatedUsers`)](https://docs.cloud.google.com/iam/docs/principals-overview#all-authenticated-users).
To enable users to browse and request access to commercial listings, you must
grant users the
[Analytics Hub Viewer role](https://docs.cloud.google.com/bigquery/docs/analytics-hub-grant-roles#ah-subscriber-role)
(`roles/analyticshub.viewer`). To enable users to subscribe to commercial
listings, you must explicitly grant users the
[Analytics Hub Subscriber role](https://docs.cloud.google.com/bigquery/docs/analytics-hub-grant-roles#ah-subscriber-role)
(`roles/analyticshub.subscriber`). For
[Cloud Marketplace-integrated commercial listings](https://docs.cloud.google.com/bigquery/docs/analytics-hub-cloud-marketplace),
the Analytics Hub Subscriber role is automatically provisioned
based on the Cloud Marketplace orders.

If you want to make your listing accessible to everyone, including people who
don't use Google Cloud, you must grant `allUsers` the Analytics Hub Viewer role
(`roles/analyticshub.viewer`).

To give users access to view or subscribe to your listing, follow these steps:

### Console

1. Go to the **Sharing (Analytics Hub)** page.

   [Go to Sharing (Analytics Hub)](https://console.cloud.google.com/bigquery/analytics-hub)
2. Click the data exchange name that contains the listing.

3. Click the listing for which you want to add subscribers.

4. Click **Set permissions**.

5. To add principals, click
   **Add principal**.

6. In the **New principals** field, add the following details based on the
   type of listing:

   - For a private listing, enter email IDs of the identity to whom you want
     to grant access.

   - For a public listing, add `allAuthenticatedUsers`.

   - For a public listing discoverable to everyone, including non-Google Cloud
     users, add `allUsers`.

7. For **Select a role** , hold the pointer over **Analytics Hub**, and then
   based on the type of listing, select one of the following roles:

   - For a commercial listing (including Cloud Marketplace-integrated
     listings), select the **Analytics Hub Viewer** role. This role lets users
     [view the listing and request access](https://docs.cloud.google.com/bigquery/docs/analytics-hub-view-subscribe-listings#discover-listings).

   - For a private or non-commercial public listing, select the
     **Analytics Hub Subscriber** role. This role lets users
     [subscribe to your listing](https://docs.cloud.google.com/bigquery/docs/analytics-hub-view-subscribe-listings#subscribe-listings).

   - For Cloud Marketplace-integrated listings, the
     Analytics Hub Subscriber role (`roles/analyticshub.subscriber`)
     doesn't need to be granted, as subscriptions are automatically governed
     and managed based on the Cloud Marketplace order.

   > [!NOTE]
   > **Note:** After you grant licenses to users to access non-Cloud Marketplace-integrated commercial listings, you can either create a private listing for those users, or grant those users the Analytics Hub Subscriber (`roles/analyticshub.subscriber`) role for your commercial listing.

   For more information, see the
   [Analytics Hub Subscriber and Viewer roles](https://docs.cloud.google.com/bigquery/docs/analytics-hub-grant-roles#ah-subscriber-role).
8. Click **Save**.

### API

1. Read the existing policy with the listing `getIamPolicy` method by using
   the
   [`projects.locations.dataExchanges.listings.getIamPolicy` method](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.listings/getIamPolicy).

   ```
   POST https://analyticshub.googleapis.com/v1/projects/PROJECT_ID/locations/LOCATION/dataExchanges/DATAEXCHANGE_ID/listings/LISTING_ID:getIamPolicy
   ```

   Replace the following:
   - `PROJECT_ID`: the project ID---for example, `my-project-1`.
   - `LOCATION`: the location of the data exchange that contains the listing.
   - `DATAEXCHANGE_ID`: the data exchange ID.
   - `LISTING_ID`: the listing ID.

   Sharing returns the current policy in the response.
2. To add or remove members and their associated roles, edit the policy with
   a text editor. Use the following format to add members:

   - `user:test-user@gmail.com`
   - `group:admins@example.com`
   - `serviceAccount:test123@example.domain.com`
   - `domain:example.domain.com`

   For example, to grant the `roles/analyticshub.subscriber` role to
   `group:subscribers@example.com`, add the following binding to the policy:

   ```
   {
    "members": [
      "group:subscribers@example.com"
    ],
    "role":"roles/analyticshub.subscriber"
   }
   ```
3. Write the updated policy by using the
   [`projects.locations.dataExchanges.listings.setIamPolicy` method](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.listings/setIamPolicy).
   In the body of the request, provide the updated IAM policy
   from the previous step.

   ```
   POST https://analyticshub.googleapis.com/v1/projects/PROJECT_ID/location/LOCATION/dataExchanges/DATAEXCHANGE_ID/listings/LISTING_ID:setIamPolicy
   ```

   In the body of the request, provide the listing details. If the request
   is successful, then the response body contains details of the listing.


For more information about the tasks that you can perform on listings using
APIs, see [`projects.locations.dataExchanges.listings` methods](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.listings#methods).

> [!NOTE]
> **Note:** After you grant licenses to users to access your commercial listing, you can either [create a private listing](https://docs.cloud.google.com/bigquery/docs/analytics-hub-manage-listings#create_a_listing) for those users or grant those users the [Analytics Hub Subscriber role](https://docs.cloud.google.com/bigquery/docs/analytics-hub-grant-roles#ah-subscriber-role) (`roles/analyticshub.subscriber`) for your commercial listing.

### Create a non-authenticated URL for public listing

To create a non-authenticated BigQuery sharing listing URL that is
viewable to even non-Google Cloud users, do the following:

1. Go to the **Sharing (Analytics Hub)** page.

   [Go to Sharing (Analytics Hub)](https://console.cloud.google.com/bigquery/analytics-hub)

   A page appears that lists all data exchanges that you can access.
2. Click the data exchange name that contains the listing.

3. Click the display name to view the listing details. The listing must have
   [public discoverability](https://docs.cloud.google.com/bigquery/docs/analytics-hub-manage-listings#create_a_listing)
   enabled.

4. Click **Copy public link** to generate an unauthenticated listing URL. Ensure
   that this listing grants `allUsers` the
   Analytics Hub Viewer role (`roles/analyticshub.viewer`).

### Create a listing administrator

To let users manage listings, you must create listing administrators. To
create listing administrators, you need to grant users the
[Analytics Hub Publisher or the Analytics Hub Listing Admin IAM role](https://docs.cloud.google.com/bigquery/docs/analytics-hub-grant-roles#ah-publisher-role)
at the listing level. For more information about how to grant these roles for a
listing, see
[Grant the role for a listing](https://docs.cloud.google.com/bigquery/docs/analytics-hub-grant-roles#grant-role-listing).

## View all subscriptions

To view all the current subscriptions to your listing, select one of the
following options:

### Console

1. Go to the **Sharing (Analytics Hub)** page.

   [Go to Sharing (Analytics Hub)](https://console.cloud.google.com/bigquery/analytics-hub)
2. Click the data exchange name that contains the listing for which you want
   to manage the subscriptions.

3. Click the listing for which you want to list all subscribers.

4. To view all subscribers of your listing, click **Manage subscriptions**.

   ![Manage subscribers](https://docs.cloud.google.com/static/bigquery/images/analytics-hub-manage-subscription.png)
5. Optional: You can filter results by subscriber details.

Alternatively, if you have access to the
[shared dataset](https://docs.cloud.google.com/bigquery/docs/analytics-hub-introduction#shared_datasets),
you can follow these steps to list subscribers:

1. Go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click
   **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)

   If you don't see the left pane, click **Expand left pane** to open the pane.
3. In the **Explorer** pane, expand your project name, click **Datasets**, and
   then click the name of the shared dataset.

4. In the **Sharing** list,
   select **Manage subscriptions**.

### SQL

The following example uses the
[`INFORMATION_SCHEMA.SCHEMATA_LINKS` view](https://docs.cloud.google.com/bigquery/docs/information-schema-datasets-schemata-links)
to list all the linked datasets linked to a shared dataset in `myproject`
that are in the `us` region:

```
SELECT * FROM `myproject`.`region-us`.INFORMATION_SCHEMA.SCHEMATA_LINKS;
```

The output is similar to the following. Some columns are omitted to simplify
the output.

```
+---+---+---+---+---+---+
|  catalog_name  | schema_name | linked_schema_catalog_name | linked_schema_catalog_number | linked_schema_name | linked_schema_org_display_name |
+---+---+---+---+---+---+
| myproject      | myschema1   | subscriptionproject1       |                 974999999291 | subscriptionld1    | subscriptionorg                |
| myproject      | myschema2   | subscriptionproject2       |                 974999999292 | subscriptionld2    | subscriptionorg                |
| myproject      | myschema3   | subscriptionproject3       |                 974999999293 | subscriptionld3    | subscriptionorg                |
+---+---+---+---+---+---+

```

For a listing with multiple regions, you can view the subscriptions across different regions by replacing the `us`
region with the intended replica location. For example, to view the linked
datasets linked to a shared dataset in `myproject` that are in the `eu` region,
use the following query:

```
SELECT * FROM `myproject`.`region-eu`.INFORMATION_SCHEMA.SCHEMATA_LINKS;
```

### API

Use the [projects.locations.dataExchanges.listings.listSubscriptions method](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.listings/listSubscriptions).

```
GET https://analyticshub.googleapis.com/v1/projects/PROJECT_ID/location/LOCATION/dataExchanges/DATAEXCHANGE_ID/listings/LISTING_ID:listSubscriptions
```

Replace the following:

- `PROJECT_ID`: the project ID of the listing that you want to subscribe to.
- `LOCATION`: the location for the listing that you want to subscribe to.
- `DATAEXCHANGE_ID`: the data exchange ID that contains the listing that you want to subscribe to.
- `LISTING_ID`: the ID of the listing that you want to subscribe to.

## Remove a subscription

When you remove a subscription created before July 25, 2023 from your listings,
the
[linked dataset](https://docs.cloud.google.com/bigquery/docs/analytics-hub-introduction#listings)
gets unlinked from the
[shared dataset](https://docs.cloud.google.com/bigquery/docs/analytics-hub-introduction#shared_datasets).
Subscribers can still see the datasets in their projects but they are no longer
linked with the shared dataset.

> [!NOTE]
> **Note:** Revoking [Cloud Marketplace-integrated commercial subscriptions](https://docs.cloud.google.com/bigquery/docs/analytics-hub-cloud-marketplace) might impact your customers and violate the [Cloud Marketplace Terms of Service](https://cloud.google.com/terms/marketplace/launcher).

To remove a subscription created before July 25, 2023 from your listings, follow
these steps:

1. To list all subscribers of a listing, follow the Google Cloud console
   instructions in [View all subscriptions](https://docs.cloud.google.com/bigquery/docs/analytics-hub-manage-listings#view_all_subscriptions).

2. To remove a subscriber from a listing, click
   **Delete** . If you want to remove
   all subscriptions, click **Remove all subscriptions**.

3. In the **Remove subscription?** dialog, enter `remove` to confirm.

4. Click **Remove**.

To remove subscriptions created after July 25, 2023, follow these steps:

### Console

1. To list all subscribers of a listing, follow the Google Cloud console
   instructions in [View all subscriptions](https://docs.cloud.google.com/bigquery/docs/analytics-hub-manage-listings#view_all_subscriptions).

2. Click the **Subscriptions** tab.

3. To remove a subscriber from a listing, select the Subscription(s) you would
   like to remove and click
   **Remove Subscriptions**.

4. In the **Remove subscription?** dialog, enter `remove` to confirm.

5. Click **Remove**.

### API

Use the [projects.locations.subscriptions.revoke method](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.subscriptions/revoke).

```
POST https://analyticshub.googleapis.com/v1/projects/PROJECT_ID/location/LOCATION/subscriptions/SUBSCRIPTION_ID:revoke
```

Replace the following:

- `PROJECT_ID`: the project ID of the subscription that you want to remove.
- `LOCATION`: the location of the subscription that you want to remove.
- `SUBSCRIPTION`: the ID of the [subscription](https://docs.cloud.google.com/bigquery/docs/analytics-hub-manage-subscriptions#list_subscriptions) that you want to remove.

## Update a listing

To update a listing, follow these steps:

### Console

1. Go to the **Sharing (Analytics Hub)** page.

   [Go to Sharing (Analytics Hub)](https://console.cloud.google.com/bigquery/analytics-hub)
2. Click the data exchange name that contains the listing.

3. Click the listing that you want to update.

4. Click **Edit listing**.

5. Modify values in the fields. You can modify all values except the shared
   dataset of the listing.

6. Optional:

   - If you enable public discoverability, grant the Analytics Hub Viewer role (`roles/analyticshub.viewer`) to `allUsers` or `allAuthenticatedUsers`. For more information, see [Grant the role for a listing](https://docs.cloud.google.com/bigquery/docs/analytics-hub-grant-roles#grant-role-listing)
   - If you disable public discoverability, remove the Analytics Hub Viewer role (`roles/analyticshub.viewer`) from `allUsers` and `allAuthenticatedUsers`. Public exchanges can't have private listings, but private exchanges can have public listings.
   - If you enable and save subscriber email logging, this setting cannot be edited. To disable email logging, delete the listing and recreate it without clicking the **Subscriber email logging** toggle.
   - Add or remove regions from the listing. Before adding multiple regions, verify you've enabled [cross-region dataset replication](https://docs.cloud.google.com/bigquery/docs/data-replication#use_dataset_replication) on the shared dataset. When removing regions, delete the shared dataset replica in that region first.
7. Preview the listing.

8. To save changes, click **Save**. To avoid discrepancies with
   Cloud Marketplace-integrated listings, a notification appears that
   prompts an update to the Cloud Marketplace data product listing.

   > [!NOTE]
   > **Note:** Updating the Cloud Marketplace data product listing requires review and approval by the Marketplace Operations Team.

### API

Use the
[`projects.locations.dataExchanges.listings.patch` method](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.listings/patch).

```
PATCH https://analyticshub.googleapis.com/v1/projects/PROJECT_ID/location/LOCATION/dataExchanges/DATAEXCHANGE_ID/listings/LISTING_ID?updateMask=UPDATEMASK
```

Replace `UPDATEMASK` with the list of fields that you
want to update. To update multiple values, use a comma-separated list. For
example, to update the display name and primary contact for a data exchange,
enter `displayName,primaryContact`.

In the body of the request, specify updated values for the following fields:

- `displayName`
- `description`
- `primaryContact`
- `documentation`
- `icon`
- `categories[]`
- `discoveryType`
- `logLinkedDatasetQueryUserEmail`
- `bigqueryDataset.replicaLocations`

For details on these fields, see
[Resource: Listing](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.listings#resource:-listing).

When updating the replica regions for your listing, ensure that you specify
all applicable regions. Before updating the listing, verify you've enabled
[cross-region dataset replication](https://docs.cloud.google.com/bigquery/docs/data-replication#use_dataset_replication)
on the shared dataset. You can add only regions where the shared dataset is replicated. To remove a region, delete the shared
dataset replica for the region before removing it from the listing. You can
also convert pre-existing listings to listings for multiple regions.


For more information about the tasks that you can perform on listings using
APIs, see [`projects.locations.dataExchanges.listings` methods](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.listings#methods).

## Delete a listing

When you delete a listing, subscribers can no longer
[view the listing](https://docs.cloud.google.com/bigquery/docs/analytics-hub-view-subscribe-listings#discover-listings).
Deleting a listing also
[deletes all the linked datasets](https://docs.cloud.google.com/bigquery/docs/analytics-hub-manage-listings#remove_a_subscription)
and removes all the subscriptions from your subscribers' projects. If a dataset
remains linked, remove the dataset manually by clicking

**Sharing \> Manage Subscription** .
The **Subscriptions** page opens, where you can remove a specific subscriber
dataset or all subscriber datasets at once.

You can't delete
[Cloud Marketplace-integrated listings](https://docs.cloud.google.com/bigquery/docs/analytics-hub-cloud-marketplace)
with active commercial subscriptions.
[Revoke all commercial subscriptions](https://docs.cloud.google.com/bigquery/docs/analytics-hub-manage-subscriptions#revoke-subscription)
before you delete the listing.

> [!CAUTION]
> **Caution:** Be aware that revoking Cloud Marketplace-integrated commercial subscriptions might impact your customers and violate the [Cloud Marketplace Terms of Service](https://cloud.google.com/terms/marketplace/launcher).

Deleting a listing for multiple regions
doesn't delete the shared dataset replicas. After deleting the listing for
multiple regions, subscribers can no longer view the listing or query the linked
datasets. If the shared dataset replicas aren't referenced in other listings,
you can
[choose to delete them](https://docs.cloud.google.com/bigquery/docs/data-replication#remove_a_dataset_replica).

Before deleting a listing for multiple regions, ensure there are no active subscriptions associated with it. If active
subscriptions exist, you must first revoke them using the
[`projects.locations.subscriptions.revoke` method](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.subscriptions/revoke).
After all the active subscriptions are removed, you can proceed with deleting the
listing for multiple regions.

> [!CAUTION]
> **Caution:** If you delete a listing, you cannot undo it.

To delete a listing, follow these steps:

### Console

1. Go to the **Sharing (Analytics Hub)** page.

   [Go to Sharing (Analytics Hub)](https://console.cloud.google.com/bigquery/analytics-hub)
2. Click the data exchange name that contains the listing.

3. Click the listing that you want to delete.

4. Click **Delete**.

5. In the **Delete listing?** dialog, confirm deletion by typing **delete**.

6. Click **Delete**.

### API

Use the
[`projects.locations.dataExchanges.listings.delete` method](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.listings/delete).

```
DELETE https://analyticshub.googleapis.com/v1/projects/PROJECT_ID/location/LOCATION/dataExchanges/DATAEXCHANGE_ID/listings/LISTING_ID
```


For more information about the tasks that you can perform on listings using
APIs, see [`projects.locations.dataExchanges.listings` methods](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.listings#methods).

## Display a listing in the featured section

To increase visibility and awareness of your listing in the
BigQuery sharing catalog, listings can be displayed in the
**Featured** section. Featured listings are governed by the Google Cloud
Partner Advantage Agreement.

Partners that are interested in their listings being in the **Featured** section
of the BigQuery sharing catalog must meet the following criteria:

- Shared data must reside in BigQuery.

- They must be enrolled in the
  [Partner Advantage Program](https://partners.cloud.google.com/)
  with the Build designation.

- The listing must be created and have
  [public discoverability](https://docs.cloud.google.com/bigquery/docs/analytics-hub-manage-listings#create_a_listing)
  enabled.

To request your listing to be in the **Featured** section, complete and submit the
[intake form](https://docs.google.com/forms/d/e/1FAIpQLSe9nLw7kmvU2AEUgaWn5vvPQMFs1Q7XwqKBy7TD5xR1DLX4bQ/viewform?resourcekey=0-zRsM2reDM3QjxegIUluHJA&pli=1).
To request that your listing be removed from the section, submit the same
[intake form](https://docs.google.com/forms/d/e/1FAIpQLSe9nLw7kmvU2AEUgaWn5vvPQMFs1Q7XwqKBy7TD5xR1DLX4bQ/viewform?resourcekey=0-zRsM2reDM3QjxegIUluHJA&pli=1).

## What's next

- Read about [BigQuery sharing architecture](https://docs.cloud.google.com/bigquery/docs/analytics-hub-introduction#architecture).
- Learn how to [view and subscribe to listings](https://docs.cloud.google.com/bigquery/docs/analytics-hub-view-subscribe-listings).
- Learn about [BigQuery sharing IAM roles](https://docs.cloud.google.com/bigquery/docs/analytics-hub-grant-roles#user_roles).
- Learn about [creating datasets](https://docs.cloud.google.com/bigquery/docs/datasets).
- Learn about [BigQuery sharing audit logging](https://docs.cloud.google.com/bigquery/docs/analytics-hub-audit-logging).
- Learn how to [monitor listings](https://docs.cloud.google.com/bigquery/docs/analytics-hub-monitor-listings).