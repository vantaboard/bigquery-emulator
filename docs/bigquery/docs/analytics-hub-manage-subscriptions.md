# Manage subscriptions

This document describes how to manage subscriptions in BigQuery sharing
(formerly Analytics Hub), covering tasks for both
subscribers and publishers.

BigQuery sharing subscribers can do the following:

- Subscribe to a listing.
- List your current subscriptions in a given Google Cloud project.
- Delete a subscription.

BigQuery sharing publishers can do the following:

- View all subscriptions to your listing.
- Revoke access to a specific subscription.

A BigQuery sharing subscription is a regionalized resource that resides
in the subscriber's project. Subscriptions store relevant information about the
subscriber and represent the contract between publisher and subscriber.

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


To get the permissions that
you need to manage subscriptions,

ask your administrator to grant you the
[Analytics Hub Subscription Owner](https://docs.cloud.google.com/iam/docs/roles-permissions/analyticshub#analyticshub.subscriptionOwner) (`roles/analyticshub.subscriptionOwner`) IAM role on the project.


For more information about granting roles, see [Manage access to projects, folders, and organizations](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).


You might also be able to get
the required permissions through [custom
roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles) or other [predefined
roles](https://docs.cloud.google.com/iam/docs/roles-overview#predefined).

## Subscriber workflows for managing subscriptions

This section describes how BigQuery sharing subscribers manage subscriptions.

### Subscribe to listings

To subscribe to listings, follow the steps in
[View and subscribe to listings and data exchanges](https://docs.cloud.google.com/bigquery/docs/analytics-hub-view-subscribe-listings#subscribe-listings).

### List subscriptions

To list your current subscriptions in a given project, use the
[`projects.locations.subscriptions.list` method](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.subscriptions/list):

```
GET https://analyticshub.googleapis.com/v1/projects/PROJECT_ID/location/LOCATION/subscriptions
```

Replace the following:

- `PROJECT_ID`: the Google Cloud project ID for the subscriptions that you want to list.
- `LOCATION`: the location for the subscriptions that you want to list.

### Delete a subscription

To delete a subscription, use the
[`projects.locations.subscriptions.delete` method](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.subscriptions/delete):

    DELETE https://analyticshub.googleapis.com/v1/projects/PROJECT_ID/location/LOCATION/subscriptions/SUBSCRIPTION_ID

Replace the following:

- `PROJECT_ID`: the project ID for the subscription to delete.
- `LOCATION`: the location of the subscription to delete. For more information about locations that support sharing, see [Supported regions](https://docs.cloud.google.com/bigquery/docs/analytics-hub-introduction#supported-regions).
- `SUBSCRIPTION_ID`: the ID of the subscription to delete.

The request body must be empty. If successful, the response body contains an
operation instance.

When a BigQuery sharing subscriber deletes a subscription, it also deletes
the linked dataset from the subscriber's project.

When you delete a subscription from a multi-region listing, all the
primary and secondary linked dataset replicas are also deleted from the
subscriber's project.

For more information about managing subscriptions using the API, see the
[`projects.locations.subscriptions` methods](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.subscriptions#methods).

## Publisher workflows for managing subscriptions

This section describes how BigQuery sharing publishers manage
subscriptions. For more information about managing subscriptions to
listings, see
[Manage listings](https://docs.cloud.google.com/bigquery/docs/analytics-hub-manage-listings).

### List subscriptions

To list all subscriptions, select one of the following options.

### Console

1. In the Google Cloud console, go to the **Sharing (Analytics Hub)** page.

   [Go to Sharing (Analytics Hub)](https://console.cloud.google.com/bigquery/analytics-hub)

   The page lists all the
   [data exchanges](https://docs.cloud.google.com/bigquery/docs/analytics-hub-introduction#data_exchanges)
   you can access.
2. Select the data exchange name where you want to list subscriptions.

3. Select the **Subscriptions** tab to view all subscriptions for listings
   within the data exchange.

### API

To list subscriptions for listings in a particular data exchange, use the
[`projects.locations.dataExchanges.listSubscriptions` method](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges/listSubscriptions).

```
GET https://analyticshub.googleapis.com/v1/projects/PROJECT_ID/location/LOCATION/dataExchanges/DATAEXCHANGE_ID:listSubscriptions
```

Replace the following:

- `PROJECT_ID`: the project ID of the data exchange for which to list subscriptions.
- `LOCATION`: the location of the data exchange for which to list subscriptions.
- `DATAEXCHANGE_ID`: the ID of the data exchange for which to list subscriptions.

### Revoke a subscription

When a BigQuery sharing publisher revokes a subscription, the subscriber
can no longer query the linked dataset. Because this action is initiated by the
publisher on a subscriber-owned resource, the linked dataset remains in the
subscriber's project. The subscriber can remove the dataset by deleting it.

If a publisher revokes a subscription from a multi-region listing, subscribers can no longer query any primary or secondary linked dataset replicas.

> [!CAUTION]
> **Caution:** Revoking [Cloud Marketplace-integrated commercial subscriptions](https://docs.cloud.google.com/bigquery/docs/analytics-hub-cloud-marketplace) might affect your customers and violate the [Cloud Marketplace Terms of Service](https://cloud.google.com/terms/marketplace/launcher).

To revoke a subscription, select one of the following options:

### Console

1. In the Google Cloud console, go to the **Sharing (Analytics Hub)** page.

   [Go to Sharing (Analytics Hub)](https://console.cloud.google.com/bigquery/analytics-hub)

   The page lists all the data exchanges you can access.
2. Select the data exchange name where you want to revoke the listing.

3. Select the **Subscriptions** tab to view all subscriptions for the data
   exchange.

4. Select the subscriptions to revoke.

5. Click **Revoke subscriptions**.

### API

To revoke a subscription, use the
[`projects.locations.subscriptions.revoke` method](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.subscriptions/revoke).

```
POST https://analyticshub.googleapis.com/v1/projects/PROJECT_ID/location/LOCATION/subscriptions/SUBSCRIPTION_ID:revoke
```

Replace the following:

- `PROJECT_ID`: the project ID of the subscription to revoke.
- `LOCATION`: the location of the subscription.
- `SUBSCRIPTION_ID`: the ID of the subscription to revoke.

## Limitations

Subscriptions have the following limitations:

- You can only use the API to manage subscriptions created after July 25, 2023. Linked datasets created before this date are unsupported because they lack the required subscription resource.

## What's next

- Read about [BigQuery sharing architecture](https://docs.cloud.google.com/bigquery/docs/analytics-hub-introduction#architecture).
- Learn how to [view and subscribe to listings and data exchanges](https://docs.cloud.google.com/bigquery/docs/analytics-hub-view-subscribe-listings).
- Learn about [BigQuery sharing user roles](https://docs.cloud.google.com/bigquery/docs/analytics-hub-grant-roles#user_roles).
- Learn how to [create datasets](https://docs.cloud.google.com/bigquery/docs/datasets).
- Learn about [BigQuery sharing audit logging](https://docs.cloud.google.com/bigquery/docs/analytics-hub-audit-logging).