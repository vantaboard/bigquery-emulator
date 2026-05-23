# Manage data exchanges

This document describes how to manage data exchanges in BigQuery sharing
(formerly Analytics Hub). As a BigQuery sharing
administrator, you can do the following:

- Create, update, view, share, and delete [data exchanges](https://docs.cloud.google.com/bigquery/docs/analytics-hub-introduction#data_exchanges).
- Create, update, delete, and share listings.
- Manage BigQuery sharing administrators, listing administrators, publishers, subscribers, and viewers.

By default, a data exchange is private. Only users or groups with access to
an exchange can view or subscribe to its data. You can request to
[make your data exchange public](https://docs.cloud.google.com/bigquery/docs/analytics-hub-manage-exchanges#make-data-exchange-public).
Making your data exchange public allows
[Google Cloud users (`allAuthenticatedUsers`)](https://docs.cloud.google.com/iam/docs/principals-overview#all-authenticated-users)
to
[discover](https://docs.cloud.google.com/bigquery/docs/analytics-hub-view-subscribe-listings#discover-listings)
and
[subscribe to](https://docs.cloud.google.com/bigquery/docs/analytics-hub-view-subscribe-listings#subscribe-listings)
listings.

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
you need to manage data exchanges,

ask your administrator to grant you the
[Analytics Hub Admin role](https://docs.cloud.google.com/iam/docs/roles-permissions/analyticshub#analyticshub.admin) (`roles/analyticshub.admin`) IAM role on your project.


For more information about granting roles, see [Manage access to projects, folders, and organizations](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).


You might also be able to get
the required permissions through [custom
roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles) or other [predefined
roles](https://docs.cloud.google.com/iam/docs/roles-overview#predefined).

## Create a data exchange

> [!CAUTION]
> **Caution:** Avoid creating the data exchange in a Google Cloud project with a VPC Service Controls perimeter. If you do so, you must add the appropriate [ingress and egress rules](https://docs.cloud.google.com/bigquery/docs/analytics-hub-vpc-sc-rules#create_a_data_exchange).

To create a data exchange, follow these steps:

### Console

1. Go to the **Sharing (Analytics Hub)** page.

   [Go to Sharing (Analytics Hub)](https://console.cloud.google.com/bigquery/analytics-hub)
2. Click
   **Create exchange**.

3. In the **Create exchange** dialog, select a **Project** and a **Region**
   for your data exchange. You can't update the project and region after you
   create the data exchange.

4. In the **Display name** field, enter a name for your data exchange.

5. Optional: Enter values in the following fields:

   - **Primary contact**: enter the URL or the email address of the primary contact for the data exchange.
   - **Description**: enter a description for the data exchange.
6. To log the [principal identifiers](https://docs.cloud.google.com/iam/docs/principal-identifiers)
   of all users running jobs and queries on linked datasets, click
   the **Subscriber Email Logging** toggle. When you enable this option,
   all future listings under the data exchange have subscriber email logging
   turned on. The logged data is available in the `job_principal_subject`
   field of the
   [`INFORMATION_SCHEMA.SHARED_DATASET_USAGE` view](https://docs.cloud.google.com/bigquery/docs/information-schema-shared-dataset-usage).

   > [!NOTE]
   > **Note:** Once you enable and save email logging, this setting can't be edited. To disable email logging, delete the data exchange and recreate it without clicking the **Subscriber Email Logging** toggle.

7. To enable public discoverability, click the **Public Discoverability**
   toggle. When an exchange is publicly discoverable, all listings in the
   exchange appear and are searchable in the catalog. If you enable public
   discoverability, configure the exchange permissions. All listings inherit
   the data exchange's public discoverability setting by default. This
   setting inheritance means public exchanges can't have private listings, but
   private exchanges can have public listings. You can set the public
   discoverability type at the individual listing level. The project where you
   create the data exchange must have an associated organization and billing
   account.

8. Click **Create Exchange**.

9. Optional: In the **Exchange Permissions** section, complete the following
   steps:

   1. Enter email addresses in the following fields to grant the
      Identity and Access Management (IAM) roles:

      - **Administrators** : assign the [Analytics Hub Admin role](https://docs.cloud.google.com/bigquery/docs/analytics-hub-grant-roles#ah-admin-role) (`roles/analyticshub.admin`) to these users.
      - **Publishers** : assign the [Analytics Hub Publisher role](https://docs.cloud.google.com/bigquery/docs/analytics-hub-grant-roles#ah-publisher-role) (`roles/analyticshub.publisher`) to these users. For information about the tasks that BigQuery sharing publishers can perform, see [Manage listings](https://docs.cloud.google.com/bigquery/docs/analytics-hub-manage-listings).
      - **Subscribers** : assign the [Analytics Hub Subscriber role](https://docs.cloud.google.com/bigquery/docs/analytics-hub-grant-roles#ah-subscriber-role) (`roles/analyticshub.subscriber`) to these users. For information about the tasks that BigQuery sharing subscribers can perform, see [View and subscribe to listings and data exchanges](https://docs.cloud.google.com/bigquery/docs/analytics-hub-view-subscribe-listings).
      - **Viewers** : assign the
        [Analytics Hub Viewer role](https://docs.cloud.google.com/bigquery/docs/analytics-hub-grant-roles#ah-subscriber-role)
        (`roles/analyticshub.viewer`) to these users. BigQuery sharing
        viewers can
        [view listings](https://docs.cloud.google.com/bigquery/docs/analytics-hub-view-subscribe-listings#discover-listings).

        If public discoverability is enabled, grant the Analytics Hub
        Viewer role to `allUsers` or `allAuthenticatedUsers`.
   2. To save permissions, click **Set permissions**.

10. If you didn't set permissions for your data exchange, click **Skip**.

### API

Use the
[`projects.locations.dataExchanges.create` method](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges/create).

```
POST https://analyticshub.googleapis.com/v1/projects/PROJECT_ID/location/LOCATION/dataExchanges?dataExchangeId=DATAEXCHANGE_ID
```

Replace the following:

- `PROJECT_ID`: the ID of the project where you want to create the data exchange.
- `LOCATION`: the location for your data exchange. For more information about locations that support BigQuery sharing, see [Supported regions](https://docs.cloud.google.com/bigquery/docs/analytics-hub-introduction#supported-regions).
- `DATAEXCHANGE_ID`: the ID for your data exchange.

In the body of the request, provide the
[data exchange details](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges#resource:-dataexchange).

If the request is successful, the response body contains the details of the
data exchange.

If you enable subscriber email logging with the
`logLinkedDatasetQueryUserEmail` field, the data exchange response contains
`log_linked_dataset_query_user_email: true`. The logged
data is available in the `job_principal_subject` field of the
[`INFORMATION_SCHEMA.SHARED_DATASET_USAGE` view](https://docs.cloud.google.com/bigquery/docs/information-schema-shared-dataset-usage).


For more information about the tasks that you can perform on data exchanges using
APIs, see [`projects.locations.dataExchanges` methods](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges#methods).

## Update a data exchange

To update a data exchange, follow these steps:

### Console

1. Go to the **Sharing (Analytics Hub)** page.

   [Go to Sharing (Analytics Hub)](https://console.cloud.google.com/bigquery/analytics-hub)
2. From the list of data exchanges, select the data exchange to
   update.

3. Go to the **Details** tab.

4. Click **Edit exchange**.

5. In the **Edit exchange** dialog, update the following fields:

   - **Display name**
   - **Primary contact**
   - **Description**
   - **Public discoverability**
     - If you enable public discoverability, grant the Analytics Hub Viewer role (`roles/analyticshub.viewer`) to `allUsers` or `allAuthenticatedUsers`.
     - If you disable public discoverability, remove the Analytics Hub Viewer role (`roles/analyticshub.viewer`) from `allUsers` or `allAuthenticatedUsers`. Public exchanges can't have private listings, but private exchanges can have public listings.
   - **Subscriber Email Logging**

     > [!NOTE]
     > **Note:** Once you enable and save email logging, this setting can't be edited. To disable email logging, delete the data exchange and recreate it without clicking the **Subscriber Email Logging** toggle.

6. Click **Save**.

### API

Use the
[`projects.locations.dataExchanges.patch` method](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges/patch).

```
PATCH https://analyticshub.googleapis.com/v1/projects/PROJECT_ID/location/LOCATION/dataExchanges/DATAEXCHANGE_ID?updateMask=UPDATEMASK
```

Replace `UPDATEMASK` with the list of fields
that you want to update. To update multiple values, use a comma-separated
list. For example, to update the display name and primary contact for a
data exchange, enter `displayName,primaryContact`.

In the body of the request, specify updated values for the following fields:

- `displayName`
- `description`
- `primaryContact`
- `documentation`
- `icon`
- `discoveryType`
- `logLinkedDatasetQueryUserEmail`

For details on these fields, see
[Resource: DataExchange](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges#resource:-dataexchange).


For more information about the tasks that you can perform on data exchanges using
APIs, see [`projects.locations.dataExchanges` methods](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges#methods).

## View data exchanges

To view the data exchanges in your project or organization that you have access to, follow these steps:

### Console

1. Go to the **Sharing (Analytics Hub)** page.

   [Go to Sharing (Analytics Hub)](https://console.cloud.google.com/bigquery/analytics-hub)
2. The page displays the data exchanges in your Google Cloud project. If you have the `resourcemanager.organizations.get` permission, you can also see the data exchanges in your Google Cloud organization.

### API

To view data exchanges in your project, use the [`projects.locations.dataExchanges.list` method](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges/list):

```
GET https://analyticshub.googleapis.com/v1/projects/PROJECT_ID/location/LOCATION/dataExchanges
```

Replace the following:

- <var translate="no">PROJECT_ID</var>: the project ID.
- <var translate="no">LOCATION</var>: the location for which you want to list the existing data exchanges.

To view data exchanges in your organization, use the
[`organizations.locations.dataExchanges.list` method](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/organizations.locations.dataExchanges/list):

```
GET https://analyticshub.googleapis.com/v1/organizations/ORGANIZATION_ID/location/LOCATION/dataExchanges
```

Replace the following:

- `ORGANIZATION_ID`: the organization ID. For more information, see [Getting your organization ID](https://docs.cloud.google.com/resource-manager/docs/creating-managing-organization#retrieving_your_organization_id).
- `LOCATION`: the location where you want to list the existing data exchanges.

## Share a data exchange

If the BigQuery sharing publisher belongs to a different organization
than the organization that contains the data exchange, the publisher can't
[view your data exchange](https://docs.cloud.google.com/bigquery/docs/analytics-hub-manage-exchanges#view_data_exchanges) in BigQuery sharing.
Share a link to the data exchange with the publisher.

1. Go to the **Sharing (Analytics Hub)** page.

   [Go to Sharing (Analytics Hub)](https://console.cloud.google.com/bigquery/analytics-hub)
2. From the list of data exchanges, click
   **More options**.

3. Click **Copy share link**.

## Give users access to a data exchange

To give users access to a data exchange, you must set the IAM
policy for that data exchange. For information about predefined
IAM user roles, see
[BigQuery sharing IAM roles](https://docs.cloud.google.com/bigquery/docs/analytics-hub-grant-roles#user_roles).

> [!NOTE]
> **Note:** When managing access for users in [external identity providers](https://docs.cloud.google.com/iam/docs/workforce-identity-federation), replace instances of Google Account principal identifiers---like `user:kiran@example.com`, `group:support@example.com`, and `domain:example.com`---with appropriate [Workforce Identity Federation principal identifiers](https://docs.cloud.google.com/iam/docs/principal-identifiers).

### Console

1. In the Google Cloud console, go to the **Sharing (Analytics Hub)** page.

   [Go to Sharing (Analytics Hub)](https://console.cloud.google.com/bigquery/analytics-hub)
2. Click the data exchange name for which you want to set permissions.

3. Go to the **Details** tab.

4. Click **Set permissions**.

5. To add principals, click
   **Add principal**.

6. In the **New principals** field, add the email IDs to which you want to grant
   access. You can also use `allUsers` to make a resource public and
   accessible to everyone on the internet, or `allAuthenticatedUsers` to
   make it accessible only to signed-in Google users.

7. In the **Select a role** menu, select **Analytics Hub**, and then
   select one of the following Identity and Access Management (IAM) roles:

   - **Analytics Hub Admin**
   - **Analytics Hub Listing Admin**
   - **Analytics Hub Publisher**
   - **Analytics Hub Subscriber**
   - **Analytics Hub Subscription Owner**
   - **Analytics Hub Viewer**
8. Click **Save**.

### API

1. Read the existing policy with the listing `getIamPolicy` method by using
   the [`projects.locations.dataExchanges.getIamPolicy`
   method](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges/getIamPolicy):

   ```
   POST https://analyticshub.googleapis.com/v1/projects/PROJECT_ID/locations/LOCATION/dataExchanges/DATAEXCHANGE_ID:getIamPolicy
   ```

   Replace the following:
   - `PROJECT_ID`: the project ID---for example, `my-project-1`.
   - `LOCATION`: the location for your data exchange. Use lowercase letters.
   - `DATAEXCHANGE_ID`: the data exchange ID.

   BigQuery sharing (formerly Analytics Hub) returns the current policy.
2. To add or remove members and their associated Identity and Access Management (IAM)
   roles, edit the policy with a text editor. Use the following format to add
   members:

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
   [`projects.locations.dataExchanges.setIamPolicy`
   method](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges/setIamPolicy).
   In the body of the request, provide the updated IAM policy
   from the previous step.

   ```
   POST https://analyticshub.googleapis.com/v1/projects/PROJECT_ID/locations/LOCATION/dataExchanges/DATAEXCHANGE_ID:setIamPolicy
   ```

   In the body of the request, provide the listing details. If the request
   is successful, then the response body contains details of the listing.

### Create BigQuery sharing administrators

To manage data exchanges, create data exchange administrators by
granting users the
[Analytics Hub Admin role](https://docs.cloud.google.com/bigquery/docs/analytics-hub-grant-roles#ah-admin-role)
(`roles/analyticshub.admin`)
at the project or data exchange level.

To allow administrators to manage all data exchanges in a project,
[grant them the Analytics Hub Admin role for that project](https://docs.cloud.google.com/bigquery/docs/analytics-hub-grant-roles#grant-role-project).

To allow administrators to manage a specific data exchange,
[grant them the Analytics Hub Admin role for that data exchange](https://docs.cloud.google.com/bigquery/docs/analytics-hub-grant-roles#grant-role-data-exchange).

## Make a data exchange public

By default, a data exchange is private. Only users or groups with access to an exchange can view or subscribe to its listings. You can make a data exchange public,
which lets
[Google Cloud users (`allAuthenticatedUsers`)](https://docs.cloud.google.com/iam/docs/principals-overview#all-authenticated-users)
discover and subscribe to its listings.

To make a data exchange public, follow these steps:

1. To allow `allAuthenticatedUsers` to
   [view listings](https://docs.cloud.google.com/bigquery/docs/analytics-hub-view-subscribe-listings#discover-listings),
   grant them the Analytics Hub Viewer role
   (`roles/analyticshub.viewer`) at the data exchange level.

2. To allow `allAuthenticatedUsers` to
   [subscribe to listings](https://docs.cloud.google.com/bigquery/docs/analytics-hub-view-subscribe-listings#subscribe-listings),
   grant them the Analytics Hub Subscriber role
   (`roles/analyticshub.subscriber`) at the data exchange level.

3. Enable public discoverability when you [create](https://docs.cloud.google.com/bigquery/docs/analytics-hub-manage-exchanges#create-exchange) or
   [update](https://docs.cloud.google.com/bigquery/docs/analytics-hub-manage-exchanges#update-exchange) a data exchange. Specify the appropriate
   permissions when you make a data exchange public.

> [!NOTE]
> **Note:** You can also convert a public data exchange to private. To do so, remove `allAuthenticatedUsers` from the permissions list for your data exchange.

## Delete a data exchange

Deleting a data exchange also deletes all its listings. However, shared and
linked datasets aren't deleted. When you delete a project, its data exchanges
aren't deleted. Delete these data exchanges before
[deleting the project](https://docs.cloud.google.com/resource-manager/docs/creating-managing-projects#shutting_down_projects).
You can't undo a data exchange deletion.

Before deleting a data exchange, complete the following steps based on the data exchange's configuration:

- For data exchanges with Google Cloud Marketplace-integrated commercial listings, [offboard the Cloud Marketplace-integrated listings](https://docs.cloud.google.com/bigquery/docs/analytics-hub-cloud-marketplace#offboard-listing).
- For data exchanges with listings for multiple regions, [remove all active subscriptions using the `projects.locations.subscriptions.revoke` method](https://docs.cloud.google.com/bigquery/docs/analytics-hub-manage-subscriptions#revoke-subscription).

To delete a data exchange, follow these steps:

### Console

1. Go to the **Sharing (Analytics Hub)** page.

   [Go to Sharing (Analytics Hub)](https://console.cloud.google.com/bigquery/analytics-hub)
2. From the list of data exchanges, select the data exchange to
   delete.

3. Go to the **Details** tab.

4. Click **Delete exchange**.

5. In the **Delete exchange?** dialog, confirm deletion by typing **delete**.

6. Click **Delete**.

### API

Use the
[`projects.locations.dataExchanges.delete` method](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges/delete).

```
DELETE https://analyticshub.googleapis.com/v1/projects/PROJECT_ID/locations/LOCATION/dataExchanges/DATAEXCHANGE_ID
```

Replace the following:

- `PROJECT_ID`: the ID of the project in which you want to create the data exchange.
- `LOCATION`: the location for your data exchange. For more information about locations that support BigQuery sharing, see [Supported regions](https://docs.cloud.google.com/bigquery/docs/analytics-hub-introduction#supported-regions).
- `DATAEXCHANGE_ID`: the ID for your data exchange.


For more information about the tasks that you can perform on data exchanges using
APIs, see [`projects.locations.dataExchanges` methods](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges#methods).

## What's next

- Learn about [managing listings](https://docs.cloud.google.com/bigquery/docs/analytics-hub-manage-listings).
- Learn how to [grant Analytics Hub user roles](https://docs.cloud.google.com/bigquery/docs/analytics-hub-grant-roles).
- Learn how to [view and subscribe to listings and data exchanges](https://docs.cloud.google.com/bigquery/docs/analytics-hub-view-subscribe-listings).
- Learn about [Sharing audit logging](https://docs.cloud.google.com/bigquery/docs/analytics-hub-audit-logging).