# Commercialize listings on Google Cloud Marketplace

As a BigQuery sharing publisher, you can use the integration of
BigQuery sharing (formerly Analytics Hub) with
[Google Cloud Marketplace](https://docs.cloud.google.com/marketplace) to create revenue by listing your own data
products on Cloud Marketplace. By using the publisher-subscriber model,
you can share data offerings with your customers at scale, without having to
manage every transaction and subscription. You can configure aspects of the data
product, such as the kind of data you provide (for example, BigQuery
datasets), the price of the subscription (paid, free, or trial), and the duration.

As a BigQuery sharing subscriber, you can use this integration to
discover and consume a wide range of Google and third-party data products and
commercial datasets.

Before you continue, familiarize yourself with
[BigQuery sharing data exchanges](https://docs.cloud.google.com/bigquery/docs/analytics-hub-manage-exchanges).

## Before you begin

1. [Grant Identity and Access Management (IAM) roles](https://docs.cloud.google.com/bigquery/docs/analytics-hub-cloud-marketplace#required-roles) that give users the necessary permissions to perform each task in this document.
2. [Enable the Analytics Hub API](https://docs.cloud.google.com/bigquery/docs/analytics-hub-cloud-marketplace#enable-api).

### Required roles


To get the permissions that
you need to use Cloud Marketplace-integrated listings,

ask your administrator to grant you the
following IAM roles:

- Create and manage BigQuery sharing listings:
  - [Analytics Hub Admin](https://docs.cloud.google.com/iam/docs/roles-permissions/analyticshub#analyticshub.admin) (`roles/analyticshub.admin`)
  - [BigQuery Data Owner](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquery#bigquery.dataOwner) (`roles/bigquery.dataOwner`)
  - [Service Management Administrator](https://docs.cloud.google.com/iam/docs/roles-permissions/servicemanagement#servicemanagement.admin) (`roles/servicemanagement.admin`)
- Create and manage data product listings on Cloud Marketplace: [Commerce Producer Admin](https://docs.cloud.google.com/iam/docs/roles-permissions/commerceproducer#commerceproducer.admin) (`roles/commerceproducer.admin`)
- Subscribe to paid BigQuery sharing listings on Cloud Marketplace:
  - [Billing Account Administrator](https://docs.cloud.google.com/iam/docs/roles-permissions/billing#billing.admin) (`roles/billing.admin`)
  - [Analytics Hub Viewer](https://docs.cloud.google.com/iam/docs/roles-permissions/analyticshub#analyticshub.viewer) (`roles/analyticshub.viewer`)


For more information about granting roles, see [Manage access to projects, folders, and organizations](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).


You might also be able to get
the required permissions through [custom
roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles) or other [predefined
roles](https://docs.cloud.google.com/iam/docs/roles-overview#predefined).

### Enable the Analytics Hub API

To enable the Analytics Hub API, select one of the following
options:

### Console

Go to the **Analytics Hub API** page and enable the Analytics Hub API for
your Google Cloud project.

[Enable the API](https://console.cloud.google.com/apis/library/analyticshub.googleapis.com)

### gcloud

Run the [`gcloud services enable` command](https://docs.cloud.google.com/sdk/gcloud/reference/services/enable):

```bash
gcloud services enable analyticshub.googleapis.com
```

You can access the
[**Sharing (Analytics Hub)** page](https://console.cloud.google.com/bigquery/analytics-hub) in the
Google Cloud console after you enable the Analytics Hub API.

## Limitations

Cloud Marketplace-integrated listings have the following limitations:

- All [BigQuery sharing limitations](https://docs.cloud.google.com/bigquery/docs/analytics-hub-introduction#limitations) apply.
- BigQuery sharing publishers and subscribers must be located in a supported [Cloud Marketplace Agency Jurisdiction](https://cloud.google.com/terms/marketplace-agency-jurisdictions).
- Cloud Marketplace-integrated listings are indexed in [Data Catalog](https://docs.cloud.google.com/bigquery/docs/data-catalog) (deprecated) and [Knowledge Catalog](https://docs.cloud.google.com/dataplex/docs/catalog-overview), but you can't specifically filter for its resource type.
- Billing usage metrics for Cloud Marketplace-integrated listings aren't captured in provider usage metrics or in [`INFORMATION_SCHEMA` views](https://docs.cloud.google.com/bigquery/docs/information-schema-intro).
- Data clean rooms and Pub/Sub topics aren't supported for Cloud Marketplace integration.

## Architecture and terminology

The following diagram shows the interaction between Cloud Marketplace
and BigQuery sharing for commercial listings:

![A data subscriber searches for a commercial listing on BigQuery sharing and purchases it on Cloud Marketplace, then a linked dataset is created in their project.](https://docs.cloud.google.com/static/bigquery/images/analytics-hub-cloud-marketplace.png)

#### Data product on Cloud Marketplace

A Cloud Marketplace data product listing is created by selecting a
BigQuery sharing listing, choosing a pricing model, and submitting
the product to Cloud Marketplace for review.

#### Cloud Marketplace-integrated listing on BigQuery sharing

A BigQuery sharing listing becomes a
Cloud Marketplace-integrated listing when the
Cloud Marketplace data product listing is approved and published,
creating an integration link between sharing and
Cloud Marketplace and making the listing eligible for purchase.
This type of BigQuery sharing listing supports shared datasets.

#### Linked resource

When subscribing to a Cloud Marketplace-integrated listing, a linked
resource is created in the BigQuery sharing subscriber project. Access to
the linked resource is managed by active Cloud Marketplace orders.
Linked datasets are supported in Cloud Marketplace-integrated listings.

## Create a Cloud Marketplace-integrated listing

To create a BigQuery sharing listing and publish it on
Cloud Marketplace, do the following:

1. In the Google Cloud console, go to the **Sharing (Analytics Hub)** page.

   [Go to Sharing (Analytics Hub)](https://console.cloud.google.com/bigquery/analytics-hub)
2. Create a new
   [sharing data exchange](https://docs.cloud.google.com/bigquery/docs/analytics-hub-manage-exchanges#create-exchange).
   Alternatively, choose an existing data exchange to retain existing
   subscriptions.

3. Create
   [listings in the data exchange](https://docs.cloud.google.com/bigquery/docs/analytics-hub-manage-listings#create_a_listing).
   Alternatively, choose existing listings to retain existing subscriptions.

   > [!NOTE]
   > **Note:** Both requesting access and Cloud Marketplace-integrated flows are supported on a single BigQuery sharing listing. This means that you can create a Cloud Marketplace-integrated listing from an existing (offline) commercial listing, without any disruptions to existing subscriptions.

4. In the row of your data exchange, click

   **More actions \> List on Marketplace**.
   You are redirected to the Cloud Marketplace
   Producer Portal.

5. Follow the instructions on the Cloud Marketplace
   Producer Portal to onboard your BigQuery sharing
   listing as a
   [data product](https://docs.cloud.google.com/marketplace/docs/partners/data).

6. Navigate back to the **Sharing (Analytics Hub)** page. In the row of
   your data exchange, the phrase **Not Published** appears in the
   **Marketplace** column, indicating that your data product was created and
   submitted for approval. Clicking the phrase **Not Published** redirects you
   to the Cloud Marketplace Producer Portal where you
   can check the status.

7. After approval, the word **Published** appears in the **Marketplace** column.
   Clicking the word **Published** redirects you to the listing in
   Cloud Marketplace.

For additional requirements, see
[Offer software on Google Cloud Marketplace](https://docs.cloud.google.com/marketplace/docs/partners/offer-products).

## Update a Cloud Marketplace-integrated listing

Updating a Cloud Marketplace-integrated listing follows the same
process as
[updating a standard listing in a data exchange](https://docs.cloud.google.com/bigquery/docs/analytics-hub-manage-listings#update_a_listing).
You might also need to update
the data product listing in the Cloud Marketplace
Producer Portal, which might require another review and approval.

## Manage subscriptions for a Cloud Marketplace-integrated listing

Commercial subscriptions to Cloud Marketplace-integrated listings are
managed by Cloud Marketplace orders. You can still manually add and
update BigQuery sharing subscribers by following
[the same process that you would with a standard listing](https://docs.cloud.google.com/bigquery/docs/analytics-hub-manage-exchanges#give_users_access_to_a_data_exchange),
but the associated Cloud Marketplace transactions don't take place.

You can also manually revoke subscriptions by following
[the same process that you would use for a standard listing](https://docs.cloud.google.com/bigquery/docs/analytics-hub-manage-subscriptions#revoke-subscription)
and entering your Marketplace Service ID to accept the warning notification.
However, be aware that revoking commercial subscriptions might affect your
customers and violate the
[Cloud Marketplace Terms of Service](https://cloud.google.com/terms/marketplace/launcher).
Additionally, revoking subscriptions doesn't remove the listing from
BigQuery sharing or Cloud Marketplace.

## Offboard a Cloud Marketplace-integrated listing

To offboard a commercial Cloud Marketplace-integrated listing, you must
manage the lifecycle of the associated product in the
[Google Cloud Marketplace Producer Portal](https://docs.cloud.google.com/marketplace/docs/partners).

Offboarding removes the commercial traits from a listing and converts it to a
standard, non-commercial listing. This process helps to ensure that existing
contractual obligations to subscribers are met before commercialization ends.

To offboard a Cloud Marketplace-integrated listing, do the following:

1. In the Producer Portal,
   [request deprecation of your listing](https://docs.cloud.google.com/marketplace/docs/partners/deprecate-product#product-deprecation).

   > [!NOTE]
   > **Note:** Standard product deprecation requires a notice period of at least 180 days for existing customers.

2. Wait for the deprecation date to pass. After the deprecation date passes, the
   Cloud Marketplace listing is permanently deleted.

3. Delete the product from the Producer Portal.

Once a BigQuery sharing listing has been fully offboarded, it functions
as a standard, non-commercial listing. You can choose to keep the listing for internal or free sharing, or
[delete the listing](https://docs.cloud.google.com/bigquery/docs/analytics-hub-manage-listings#delete_a_listing).

### Delete a Cloud Marketplace-integrated listing

> [!WARNING]
> **Warning:** We strongly recommend [offboarding a Cloud Marketplace-integrated listing](https://docs.cloud.google.com/bigquery/docs/analytics-hub-cloud-marketplace#offboard-listing) instead of deleting the Cloud Marketplace-integrated listing. Once you delete a Cloud Marketplace-integrated listing, you can't undo it. Deleting Cloud Marketplace-integrated listings might affect your customers and violate the [Cloud Marketplace Terms of Service](https://cloud.google.com/terms/marketplace/launcher).

To delete a Cloud Marketplace-integrated listing from
BigQuery sharing and Cloud Marketplace, do the following:

1. Revoke all commercial subscriptions for your Cloud Marketplace-integrated listing by following [the same process that you would use for a data exchange](https://docs.cloud.google.com/bigquery/docs/analytics-hub-manage-subscriptions#revoke-subscription). You can't delete Cloud Marketplace-integrated listings with active commercial subscriptions.
2. Follow the standard process to [delete a data exchange](https://docs.cloud.google.com/bigquery/docs/analytics-hub-manage-exchanges#delete_a_data_exchange).
3. When you are prompted with a warning notification, enter your Marketplace Service ID to accept, and then click **Confirm**.

## Subscribe to a Cloud Marketplace-integrated listing

To subscribe to a BigQuery sharing listing on
Cloud Marketplace, do the following:

1. In the Google Cloud console, go to the **Sharing (Analytics Hub)** page.

   [Go to Sharing (Analytics Hub)](https://console.cloud.google.com/bigquery/analytics-hub)
2. Click
   **Search listings**.

3. Search for the listing that you want to subscribe to.

4. Click the listing.

5. If your organization has already purchased the listing, which means the
   **Subscribe** button and purchase date are visible, do the following:

   1. Click **Subscribe**.
   2. Specify the project and linked dataset name.
   3. Click **Save**.

   If you don't have permission to subscribe to listings, click
   **Request access** and submit the request form.
6. If your organization hasn't purchased the listing (the
   **Purchase via Marketplace** button is visible), do the following:

   1. Click **Purchase via Marketplace**.
   2. Click **Subscribe**.
   3. In the **Order summary** page, specify your subscription plan, purchase details, and accept the terms if you agree with them.
   4. Click **Subscribe** , and then click **Go to product page**.
   5. Wait for a few minutes. After your order is activated, click **Manage on BigQuery sharing (Analytics Hub)** . You are redirected back to the **Sharing (Analytics Hub)** page.
   6. Click **Subscribe** on the BigQuery sharing listing page.
   7. Specify the project and linked dataset name.
   8. Click **Save**.

   For some listings, to get a quote, you might have to contact the sales
   team by submitting a form.

Any project with the same billing account can also subscribe to the listing.

## Pricing

Standard
[BigQuery sharing pricing](https://docs.cloud.google.com/bigquery/docs/analytics-hub-introduction#pricing)
applies. Additionally, the
[Cloud Marketplace revenue sharing requirement](https://docs.cloud.google.com/marketplace/docs/partners/get-started)
applies for Cloud Marketplace-integrated listings. For more
information about how BigQuery sharing subscribers are charged by
publishers for use of data products, see
[Managing billing for Cloud Marketplace products](https://docs.cloud.google.com/marketplace/docs/manage-billing).

## What's next

- Learn more about [Cloud Marketplace](https://docs.cloud.google.com/marketplace/docs).
- If you're a VPC Service Controls user, see [VPC Service Controls](https://docs.cloud.google.com/bigquery/docs/analytics-hub-introduction#vpc-service).