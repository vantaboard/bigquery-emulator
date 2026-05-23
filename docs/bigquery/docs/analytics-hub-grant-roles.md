# Configure BigQuery sharing roles

This document describes the Identity and Access Management (IAM) roles for
BigQuery sharing (formerly Analytics Hub) and how to grant them.
For more information, see
[BigQuery sharing roles and permissions](https://docs.cloud.google.com/iam/docs/roles-permissions/analyticshub).

> [!NOTE]
> **Note:** When managing access for users in [external identity providers](https://docs.cloud.google.com/iam/docs/workforce-identity-federation), replace instances of Google Account principal identifiers---like `user:kiran@example.com`, `group:support@example.com`, and `domain:example.com`---with appropriate [Workforce Identity Federation principal identifiers](https://docs.cloud.google.com/iam/docs/principal-identifiers).

## BigQuery sharing IAM roles

The following sections describe the predefined BigQuery sharing roles.
You can assign these roles to users to perform various tasks on your data
exchanges and listings.

### Analytics Hub Admin role

To [manage data exchanges](https://docs.cloud.google.com/bigquery/docs/analytics-hub-manage-exchanges),
BigQuery sharing provides the
[Analytics Hub Admin role](https://docs.cloud.google.com/bigquery/docs/access-control#analyticshub.admin)
(`roles/analyticshub.admin`) that you can grant for a Google Cloud project or data
exchange. This role lets users do the following:

- Create, update, and delete data exchanges.
- Create, update, delete, and share listings.
- Manage BigQuery sharing administrators, listing administrators, publishers, subscribers, and viewers.

With this role, you become a *BigQuery sharing administrator*.

### Analytics Hub Publisher and Listing Admin roles

To [manage listings](https://docs.cloud.google.com/bigquery/docs/analytics-hub-manage-listings),
Sharing provides the following predefined roles that you
can grant for a project, a data exchange, or a listing:

- [Analytics Hub Publisher role](https://docs.cloud.google.com/bigquery/docs/access-control#analyticshub.publisher)
  (`roles/analyticshub.publisher`), which lets users do the following:

  - Create, update, and delete listings.
  - [Set IAM policies on listings](https://docs.cloud.google.com/bigquery/docs/analytics-hub-grant-roles#grant-role-listing).

  With this role, you become a *BigQuery sharing publisher*.
- [Analytics Hub Listing Admin role](https://docs.cloud.google.com/bigquery/docs/access-control#analyticshub.listingAdmin)
  (`roles/analyticshub.listingAdmin`), which lets users do the following:

  - Update and delete listings.
  - [Set IAM policies on listings](https://docs.cloud.google.com/bigquery/docs/analytics-hub-grant-roles#grant-role-listing).

  With this role, you become a *BigQuery sharing listing administrator*.

### Analytics Hub Subscriber and Viewer roles

To [view and subscribe to listings and data exchanges](https://docs.cloud.google.com/bigquery/docs/analytics-hub-view-subscribe-listings),
Sharing provides the following predefined roles that you
can grant for a project, a data exchange, or a listing:

- [Analytics Hub Subscriber role](https://docs.cloud.google.com/bigquery/docs/access-control#analyticshub.subscriber)
  (`roles/analyticshub.subscriber`), which lets users view and subscribe to listings.

  With this role, you become a *BigQuery sharing subscriber*.
- [Analytics Hub Viewer role](https://docs.cloud.google.com/bigquery/docs/access-control#analyticshub.viewer)
  (`roles/analyticshub.viewer`), which lets users view listings and data exchange permissions.

  With this role, you become a *BigQuery sharing viewer*.

### Analytics Hub Subscription Owner role

To [manage subscriptions](https://docs.cloud.google.com/bigquery/docs/analytics-hub-manage-subscriptions),
Sharing provides the following predefined role that you can
grant at the project level:

- [Analytics Hub Subscription Owner role](https://docs.cloud.google.com/bigquery/docs/access-control#analyticshub.subscriptionOwner) (`roles/analyticshub.subscriptionOwner`), which lets users manage their subscriptions.

With this role, you become a *BigQuery sharing subscription owner*.

## Grant BigQuery sharing IAM roles

Depending on your need, you can grant the IAM roles at the
following levels of the resource hierarchy:

- **Project.** If you grant a role for a project, it applies to all data exchanges and listings in that project.
- **Data exchange.** If you grant a role for a data exchange, it applies to all listings in that data exchange.
- **Listing.** If you grant a role for a listing, it applies only to that specific listing.

### Grant the role for a project

If you want to set IAM policies on a project, you must have the
[Project IAM Admin role](https://docs.cloud.google.com/iam/docs/roles-permissions/resourcemanager#resourcemanager.projectIamAdmin)
(`roles/resourcemanager.projectIamAdmin`) on that project. To grant the
predefined BigQuery sharing Identity and Access Management roles for a project, select one
of the following options.

### Console

1. Go to **IAM** for the project.

   [Go to IAM](https://console.cloud.google.com/console/iam-admin/iam)
2. Click **Grant access**.

3. In the **New principals** field, enter the email address of the identity
   you want to grant access to. For example:

   - Google Account email: `test-user@gmail.com`
   - Google group: `admins@googlegroups.com`
   - Service account: `server@example.gserviceaccount.com`
   - Google Workspace domain: `example.com`
4. In the **Select a role** list, hold the pointer over **Analytics Hub** and
   select one of the following roles:

   - **Analytics Hub Admin**
   - **Analytics Hub Listing Admin**
   - **Analytics Hub Publisher**
   - **Analytics Hub Subscriber**
   - **Analytics Hub Subscription Owner**
   - **Analytics Hub Viewer**
5. Optional: To further control access to Google Cloud resources,
   [add a conditional role binding](https://docs.cloud.google.com/iam/docs/managing-conditional-role-bindings#add).

6. Save your changes.

   You can delete and update project administrators using the same
   IAM panel.

### gcloud

To grant roles at a project level, use the
[`gcloud projects add-iam-policy-binding` command](https://docs.cloud.google.com/sdk/gcloud/reference/projects/add-iam-policy-binding):

```
gcloud projects add-iam-policy-binding PROJECT_ID \
    --member='PRINCIPAL' \
    --role='roles/analyticshub.admin'
```

Replace the following:

- `PROJECT_ID`: the project---for example, `my-project-1`.
- `PRINCIPAL`: a valid identity to which you want to
  grant the role. For example:

  - Google Account email address: `user:user@gmail.com`
  - Google group: `group:admins@googlegroups.com`
  - Service account: `serviceAccount:server@example.gserviceaccount.com`
  - Google Workspace domain: `domain:example.com`

### API

1. Read the existing policy with the resource's `getIamPolicy` method. For
   projects, use the [`projects.getIamPolicy` method](https://docs.cloud.google.com/resource-manager/reference/rest/v1/projects/getIamPolicy).

   ```
   POST https://cloudresourcemanager.googleapis.com/v1/projects/PROJECT_ID:getIamPolicy
   ```

   Replace `PROJECT_ID` with the project---for
   example, `my-project-1`.
2. To add principals and their associated roles, edit the policy with
   a text editor. Use the following format to add members:

   - `user:test-user@gmail.com`
   - `group:admins@example.com`
   - `serviceAccount:test123@example.domain.com`
   - `domain:example.domain.com`

   For example, to grant the `roles/analyticshub.admin` role to
   `group:admins@example.com`, add the following binding to the policy:

   ```
   {
    "members": [
      "group:admins@example.com"
    ],
    "role":"roles/analyticshub.admin"
   }
   ```
3. Write the updated policy by using the `setIamPolicy` method.

   For example, to set
   a policy at the project level, use the
   [`project.setIamPolicy` method](https://docs.cloud.google.com/resource-manager/reference/rest/v1/projects/setIamPolicy).
   In the body of the request, provide the updated IAM policy
   from the previous step.

   ```
   POST https://cloudresourcemanager.googleapis.com/v1/projects/PROJECT_ID:setIamPolicy
   ```

   Replace the `PROJECT_ID` with the project ID.

### Grant the role for a data exchange

To grant the role for a data exchange, follow these steps:

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

When you grant permissions at the resource level, such as on a
data exchange, you must use lowercase letters for the location part of the
resource name. Using uppercase or mixed-case values
can cause `Permission Denied` errors.

- Use: `projects/myproject/locations/us/dataExchanges/123`
- Avoid: `projects/myproject/locations/US/dataExchanges/123`
- Avoid: `projects/myproject/locations/Eu/dataExchanges/123`

You can delete and update data exchange roles using the same IAM
panel.

### Grant the role for a listing

To grant the role for a listing, follow these steps:

### Console

1. In the Google Cloud console, go to the **Sharing (Analytics Hub)** page.

   [Go to Sharing (Analytics Hub)](https://console.cloud.google.com/bigquery/analytics-hub)
2. Click the data exchange name that contains the listing.

3. Click the listing for which you want to add users.

4. Click **Set permissions**.

5. To add principals, click **Add
   principal**.

6. In the **New principals** field, add the email IDs of the identity to which
   you want to grant access.

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
   the [`projects.locations.dataExchanges.listings.getIamPolicy`
   method](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.listings/getIamPolicy):

   ```
   POST https://analyticshub.googleapis.com/v1/projects/PROJECT_ID/locations/LOCATION/dataExchanges/DATAEXCHANGE_ID/listings/LISTING_ID:getIamPolicy
   ```

   Replace the following:
   - `PROJECT_ID`: the project ID---for example, `my-project-1`.
   - `LOCATION`: the location of the data exchange that contains the listing. Use lowercase letters.
   - `DATAEXCHANGE_ID`: the data exchange ID.
   - `LISTING_ID`: the listing ID.

   Sharing returns the current policy.
2. To add or remove members and their associated Identity and Access Management (IAM)
   roles, edit the policy with a text editor. Use the following format to add
   members:

   - `user:test-user@gmail.com`
   - `group:admins@example.com`
   - `serviceAccount:test123@example.domain.com`
   - `domain:example.domain.com`

   For example, to grant the `roles/analyticshub.publisher` role to
   `group:publishers@example.com`, add the following binding to the policy:

   ```
   {
    "members": [
      "group:publishers@example.com"
    ],
    "role":"roles/analyticshub.publisher"
   }
   ```
3. Write the updated policy by using the
   [`projects.locations.dataExchanges.listings.setIamPolicy`
   method](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.listings/setIamPolicy).
   In the body of the request, provide the updated IAM policy
   from the previous step.

   ```
   POST https://analyticshub.googleapis.com/v1/projects/PROJECT_ID/locations/LOCATION/dataExchanges/DATAEXCHANGE_ID/listings/LISTING-ID:setIamPolicy
   ```

   In the body of the request, provide the listing details. If the request
   is successful, then the response body contains details of the listing.

When you grant permissions at the resource level, such as on a
listing, you must use lowercase letters for the location part of the
resource name. Using uppercase or mixed-case values
can cause `Permission Denied` errors.

- Use: `projects/myproject/locations/us/dataExchanges/123/listings/456`
- Avoid: `projects/myproject/locations/US/dataExchanges/123/listings/456`
- Avoid: `projects/myproject/locations/Eu/dataExchanges/123/listings/456`

You can delete and update listing roles using the same IAM panel.

## What's next

- Learn about [BigQuery IAM roles](https://docs.cloud.google.com/bigquery/docs/access-control).
- Learn about [BigQuery sharing](https://docs.cloud.google.com/bigquery/docs/analytics-hub-introduction).
- Learn how to [manage data exchanges](https://docs.cloud.google.com/bigquery/docs/analytics-hub-manage-exchanges).
- Learn how to [manage listings](https://docs.cloud.google.com/bigquery/docs/analytics-hub-manage-listings).
- Learn how to [view and subscribe to listings and data exchanges](https://docs.cloud.google.com/bigquery/docs/analytics-hub-view-subscribe-listings).