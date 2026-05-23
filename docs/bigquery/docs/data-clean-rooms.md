# Share sensitive data with data clean rooms

Data clean rooms provide a security-enhanced environment in which multiple
parties can share, join, and analyze their data assets without moving or
revealing the underlying data.

BigQuery data clean rooms use the BigQuery sharing
(formerly Analytics Hub) platform. While standard
[BigQuery sharing data exchanges](https://docs.cloud.google.com/bigquery/docs/analytics-hub-manage-exchanges)
let you share data across organizational boundaries at scale, data
clean rooms address use cases for sharing sensitive and protected data.
Data clean rooms provide additional security controls to protect the
underlying data and enforce [analysis rules](https://docs.cloud.google.com/bigquery/docs/analysis-rules)
defined by the data owner.

Primary use cases include the following:

- **Campaign planning and audience insights.** Let two parties (for example, sellers and buyers) mix first-party data and improve data enrichment in a privacy-centric way.
- **Measurement and attribution.** Match customer and media performance data to better understand the effectiveness of marketing efforts and make more informed business decisions.
- **Activation.** Combine customer data with data from other parties to enrich customer understanding, which lets you improve segmentation capabilities and media activation.

Data clean rooms also support several use cases beyond the marketing industry:

- **Retail and consumer packaged goods (CPG).** Optimize marketing and promotional activities by combining point-of-sale data from retailers and marketing data from CPG companies.
- **Financial services.** Improve fraud detection by combining sensitive data from other financial and government agencies. Build credit risk scoring by aggregating customer data across multiple banks.
- **Healthcare.** Share data between doctors and pharmaceutical researchers to learn how patients are reacting to treatments.
- **Supply chain, logistics, and transportation.** Combine data from suppliers and marketers to get a complete picture of how products perform throughout their lifecycle.

## Roles

There are three main roles in BigQuery data clean rooms:

- **Data clean room owner** : manages permissions, visibility, and membership of one or more data clean rooms within a Google Cloud project. The data clean room owner can assign the data contributor and data clean room subscriber roles to users. This role is analogous to the [Analytics Hub Admin](https://docs.cloud.google.com/bigquery/docs/analytics-hub-grant-roles#ah-admin-role) IAM role.
- **Data contributor** : publishes data to a data clean room. In many cases, a data clean room owner is also a data contributor. This role is analogous to the [Analytics Hub Publisher](https://docs.cloud.google.com/bigquery/docs/analytics-hub-grant-roles#ah-publisher-role) IAM role.
- **Data clean room subscriber** : subscribes to the data published in a data clean room and runs queries on the data. This role is analogous to a combination of the [Analytics Hub Subscriber](https://docs.cloud.google.com/bigquery/docs/analytics-hub-grant-roles#ah-subscriber-role) and [Analytics Hub Subscription Owner](https://docs.cloud.google.com/iam/docs/roles-permissions/analyticshub#analyticshub.subscriptionOwner) IAM roles.

## Architecture

BigQuery data clean rooms use a publish and subscribe model of
BigQuery data. BigQuery architecture separates
compute and storage, which lets data contributors share data without making
multiple copies of the data. The following diagram shows the
BigQuery data clean room architecture:

![Data contributors publish data to the data clean room, which subscribers can query with privacy filters.](https://docs.cloud.google.com/static/bigquery/images/clean-room-architecture.png)

#### Data clean room

A data clean room is an environment for sharing sensitive data that helps
prevent raw access and enforces query restrictions. Only users or groups added
as data clean room subscribers can subscribe to the shared data. Data clean
room owners can create any number of data clean rooms in BigQuery sharing.

#### Shared resources

A shared resource is the unit of data sharing in a data clean room. The
resource must be a BigQuery table, view, or routine
(table-valued function). As a data contributor, you create or use an existing
BigQuery resource in your project to share with
your data clean room subscribers.

#### Listings

A data contributor creates a listing when they add data to a data clean room.
It contains a reference to the data contributor's shared resource along with
descriptive information that helps subscribers use the data. As
a data contributor, you can create a listing and include information such as a
description, sample queries, and links to documentation for your subscribers.

#### Linked datasets

A linked dataset is a read-only BigQuery dataset that serves as
a symbolic link to all data in a data clean room. When data clean room
subscribers query resources in a linked dataset, data from the shared resources
is returned, satisfying analysis rules set by the data contributor. As a
subscriber, a linked dataset is created inside your project when you subscribe to
a data clean room. No copy of the data is created, and subscribers can't see
certain metadata, such as view definitions.

#### Analysis rules

As a data contributor, you configure analysis rules on the resources
that you share in the data clean room.
[Analysis rules](https://docs.cloud.google.com/bigquery/docs/analysis-rules) prevent raw access to
underlying data and enforce query restrictions. For example, data clean rooms
support the
[aggregation threshold analysis rule](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#agg_threshold_clause),
which lets data clean room subscribers analyze data only through aggregation
queries.

#### Data egress controls

[Data egress](https://docs.cloud.google.com/bigquery/docs/analytics-hub-introduction#data_egress) controls
automatically prevent data clean room subscribers from
copying and exporting raw data from a data clean room. Data contributors can
configure additional controls to prevent copying and exporting query
results that subscribers obtain.

#### Query templates

[Query templates](https://docs.cloud.google.com/bigquery/docs/query-templates)
[(Preview)](https://cloud.google.com/products#product-launch-stages) let data
clean room owners and BigQuery sharing publishers share predefined queries
without sharing the underlying resources of tables and views.

Predefined queries use
[table-valued functions (TVFs)](https://docs.cloud.google.com/bigquery/docs/table-functions) in
BigQuery that allow an entire table or specific fields to pass
as input parameters and return a table as the output.

> [!WARNING]
> **Warning:** Allowing data clean room subscribers to run arbitrary queries in your data clean rooms can create security vulnerabilities. To mitigate these risks and enhance data security, use query templates.

## Limitations

BigQuery data clean rooms have the following limitations:

- You can set [analysis rules](https://docs.cloud.google.com/bigquery/docs/analysis-rules) only on views, not on tables or materialized views. Because of this limitation, if a data contributor directly shares tables or materialized views, or views without analysis rules, into a data clean room, then data clean room subscribers have raw access to the data in those resources.
- Because data clean rooms use the BigQuery sharing platform, all [BigQuery sharing limitations](https://docs.cloud.google.com/bigquery/docs/analytics-hub-introduction#limitations) apply.
- Data clean rooms are only available in [BigQuery sharing regions](https://docs.cloud.google.com/bigquery/docs/analytics-hub-introduction#supported-regions).
- As a data clean room subscriber, you can't search for shared resources in Knowledge Catalog or Data Catalog.
- As a data clean room subscriber, you can't query [`INFORMATION_SCHEMA` views](https://docs.cloud.google.com/bigquery/docs/information-schema-intro) on linked datasets.
- As a data contributor, you can't publish an entire dataset directly to a data clean room.
- As a data contributor, you can't publish models or routines (outside of query templates) to a data clean room.
- You can add a maximum of 100 shared resources to a data clean room. If you need to increase this limit, contact [bq-dcr-feedback@google.com](mailto:bq-dcr-feedback@google.com).
- Listings for multiple regions aren't supported in data clean rooms.

## Before you begin

Grant Identity and Access Management (IAM) roles to give users the necessary permissions
to perform each task in this document, enable the Analytics Hub API, and assign
the Analytics Hub Admin role to your data clean room owner.

### Required permissions


To get the permissions that
you need to use data clean rooms,

ask your administrator to grant you the
[BigQuery Data Editor](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquery#bigquery.dataEditor) (`roles/bigquery.dataEditor`) IAM role.


For more information about granting roles, see [Manage access to projects, folders, and organizations](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).


This predefined role contains

the permissions required to use data clean rooms. To see the exact permissions that are
required, expand the **Required permissions** section:


#### Required permissions

The following permissions are required to use data clean rooms:

- `serviceUsage.services.get`
- `serviceUsage.services.list`
- `serviceUsage.services.enable`


You might also be able to get
these permissions
with [custom roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles) or
other [predefined roles](https://docs.cloud.google.com/iam/docs/roles-overview#predefined).

For more information about IAM roles and permissions in
BigQuery, see
[Introduction to IAM](https://docs.cloud.google.com/bigquery/docs/access-control).

### Enable the Analytics Hub API

To enable the Analytics Hub API, select one of the following
options:

### Console

Go to the **Analytics Hub API** page and enable the API for
your Google Cloud project.

[Enable the Analytics Hub API](https://console.cloud.google.com/apis/library/analyticshub.googleapis.com)

### bq

Run the
[`gcloud services enable` command](https://docs.cloud.google.com/sdk/gcloud/reference/services/enable):

```bash
gcloud services enable analyticshub.googleapis.com
```

After you enable the Analytics Hub API, you can access the
[Sharing (Analytics Hub) page](https://console.cloud.google.com/bigquery/analytics-hub).

### Assign the Analytics Hub Admin role

Your data clean room owner, the user who creates the data clean
room, must have the
[Analytics Hub Admin role](https://docs.cloud.google.com/bigquery/docs/analytics-hub-grant-roles#ah-admin-role)
(`roles/analyticshub.admin`).
To learn how to grant this role to other users, see
[Create BigQuery sharing administrators](https://docs.cloud.google.com/bigquery/docs/analytics-hub-manage-exchanges#create-exchange-administrator).

## Data clean room owner workflows

As a data clean room owner, you can do the following:

- Create a data clean room.
- Update data clean room properties.
- Delete a data clean room.
- Manage data contributors.
- Manage data clean room subscribers.
- Share a data clean room.

### Additional data clean room owner permissions

You must have the Analytics Hub Admin role (`roles/analyticshub.admin`) on your
project to perform data clean room owner tasks. You can also assign this role
at the folder or organization level, if applicable.

### Create a data clean room

### Console

1. In the Google Cloud console, go to the **Sharing (Analytics Hub)** page.

   [Go to Sharing (Analytics Hub)](https://console.cloud.google.com/bigquery/analytics-hub)
2. Click **Create clean room**.

3. For **Project**, select the project for the data clean room. You must
   enable the Analytics Hub API for the project.

4. Specify the location, name, primary contact, icon (optional), and
   description for the data clean room. You can only list resources in the data clean room that are in the same
   region as the data clean room.

5. Optional: To log the
   [principal identifiers](https://docs.cloud.google.com/iam/docs/principal-identifiers)
   of all users running jobs and queries on linked datasets, click
   the **Subscriber Email Logging** toggle. The logged data appears
   in the `job_principal_subject` field of the
   [`INFORMATION_SCHEMA.SHARED_DATASET_USAGE` view](https://docs.cloud.google.com/bigquery/docs/information-schema-shared-dataset-usage).

   > [!NOTE]
   > **Note:** Once you enable and save email logging, you can't edit this setting. To disable email logging, delete the data clean room and recreate it without clicking the **Subscriber Email Logging** toggle.

6. Click **Create clean room**.

7. Optional: In the **Clean Room Permissions** section, add other data clean
   room owners, data contributors, or data clean room subscribers.

   ![Create data clean room pane.](https://docs.cloud.google.com/static/bigquery/images/clean-room-create.png)

### API

Use the
[`projects.locations.dataExchanges.create` method](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges/create)
and set the
[sharing environment](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges#sharingenvironmentconfig)
to `dcrExchangeConfig`.

The following example shows how to call the `projects.locations.dataExchanges.create` method using the `curl` command:

```bash
  curl -H "Authorization: Bearer $(gcloud auth print-access-token)" -H "Content-Type: application/json" -L -X POST https://analyticshub.googleapis.com/v1/projects/PROJECT_ID/locations/LOCATION/dataExchanges?data_exchange_id=CLEAN_ROOM_ID -d
  '{
    display_name: "CLEAN_ROOM_NAME",
    sharing_environment_config: {dcr_exchange_config: {}}
  }'
```

Replace the following:

- `PROJECT_ID`: your project ID
- `LOCATION`: the location of the data clean room
- `CLEAN_ROOM_ID`: your data clean room ID
- `CLEAN_ROOM_NAME`: the display name of your data clean room

<br />

In the body of the request, provide the
[data exchange details](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges#resource:-dataexchange).

If the request is successful, the response body contains the details of the
data clean room.

If you enable subscriber email logging with the
`logLinkedDatasetQueryUserEmail` field, the data exchange response contains
`log_linked_dataset_query_user_email: true`. The logged data appears
in the `job_principal_subject` field of the
[`INFORMATION_SCHEMA.SHARED_DATASET_USAGE` view](https://docs.cloud.google.com/bigquery/docs/information-schema-shared-dataset-usage).

### Update a data clean room

### Console

1. In the Google Cloud console, go to the **Sharing (Analytics Hub)** page.

   [Go to Sharing (Analytics Hub)](https://console.cloud.google.com/bigquery/analytics-hub)
2. Click the display name of the data clean room that you want to update.

3. In the **Details** tab, click **Edit clean room details**.

4. Update the data clean room name, primary contact, icon, description, or
   subscriber email logging setting as needed.

   > [!NOTE]
   > **Note:** Once you enable and save email logging, you can't edit this setting. To disable email logging, delete the data clean room and recreate it without clicking the **Subscriber Email Logging** toggle.

5. Click **Save**.

### API

Use the
[`projects.locations.dataExchanges.patch` method](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges/patch)
and set the
[sharing environment](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges#sharingenvironmentconfig)
to `dcrExchangeConfig`.

The following example shows how to call the `projects.locations.dataExchanges.patch` method using the `curl` command:

```bash
curl -H "Authorization: Bearer $(gcloud auth print-access-token)" -H "Content-Type: application/json" -L -X PATCH https://analyticshub.googleapis.com/v1/projects/PROJECT_ID/locations/LOCATION/dataExchanges/CLEAN_ROOM_ID?updateMask=UPDATEMASK -d
'{
  display_name: "CLEAN_ROOM_NAME",
  sharing_environment_config: {dcr_exchange_config: {}}
}'
```

Replace the following:

- `PROJECT_ID`: your project ID
- `LOCATION`: the location of the data clean room
- `CLEAN_ROOM_ID`: your data clean room ID
- `CLEAN_ROOM_NAME`: the display name of your data clean room

<br />

Replace `UPDATEMASK` with the list of fields that you
want to update. To update multiple values, use a comma-separated list. For
example, to update the display name and primary contact for a data exchange,
enter `displayName,primaryContact`.

In the body of the request, specify updated values for the following
fields:

- `displayName`
- `description`
- `primaryContact`
- `documentation`
- `icon`
- `discoveryType`
- `logLinkedDatasetQueryUserEmail`

For details on these fields, see
[Resource: DataExchange](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges#resource:-dataexchange).

### Delete a data clean room

### Console

1. In the Google Cloud console, go to the **Sharing (Analytics Hub)** page.

   [Go to Sharing (Analytics Hub)](https://console.cloud.google.com/bigquery/analytics-hub)
2. In the row of the data clean room that you want to delete, click

   **More actions \> Delete**.

3. To confirm, enter `delete`, and then click **Delete**. You can't undo this
   action.

### API

Use the
[`projects.locations.dataExchanges.delete` method](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges/delete)
and set the
[sharing environment](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges#sharingenvironmentconfig)
to `dcrExchangeConfig`.

The following example shows how to call the `projects.locations.dataExchanges.delete` method using the `curl` command:

```bash
curl -H "Authorization: Bearer $(gcloud auth print-access-token)" -H "Content-Type: application/json" -L -X DELETE https://analyticshub.googleapis.com/v1/projects/PROJECT_ID/locations/LOCATION/dataExchanges?data_exchange_id=CLEAN_ROOM_ID
```

Replace the following:

- `PROJECT_ID`: your project ID
- `LOCATION`: the location of the data clean room
- `CLEAN_ROOM_ID`: your data clean room ID
- `CLEAN_ROOM_NAME`: the display name of your data clean room

<br />

When you delete a data clean room, all the listings within it are deleted.
However, the shared resources and linked datasets are not deleted. The linked
datasets are unlinked from the source datasets, so querying resources in the
data clean room starts to fail for data clean room subscribers.

### Manage data contributors

As a data clean room owner, you manage which users can add data to your data
clean rooms (your data contributors). To let a user add data to a data
clean room, grant them the
[Analytics Hub Publisher role](https://docs.cloud.google.com/bigquery/docs/analytics-hub-grant-roles#ah-publisher-role)
(`roles/analyticshub.publisher`) on a specific data clean room:

### Console

1. In the Google Cloud console, go to the **Sharing (Analytics Hub)** page.

   [Go to Sharing (Analytics Hub)](https://console.cloud.google.com/bigquery/analytics-hub)
2. Click the display name of the data clean room that you want to grant
   permissions to.

3. In the **Details** tab, click **Set permissions**.

4. Click **Add principal**.

5. For **New principals**, enter the usernames or emails of the data
   contributors that you're adding.

6. For **Select a role** , select **Analytics Hub \> Analytics Hub
   Publisher**.

7. Click **Save**.

You can delete and update data contributors at any time by clicking
**Set Permissions**.

### API

Use the
[`projects.locations.dataExchanges.setIamPolicy` method](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges/setIamPolicy).

The following example shows how to call the `projects.locations.dataExchanges.setIamPolicy` method using the `curl` command:

```bash
curl -H "Authorization: Bearer $(gcloud auth print-access-token)" -H "Content-Type: application/json" -L -X POST https://analyticshub.googleapis.com/v1/projects/PROJECT_ID/locations/LOCATION/dataExchanges/CLEAN_ROOM_ID:setIamPolicy -d
'{
  "policy": {
    "bindings": [
      {
        "members": [
          "my-service-account@my-project.iam.gserviceaccount.com"
        ],
        "role": "roles/analyticshub.publisher"
      }
    ]
  }
}'
```

The policy in the request body should conform to the structure of a
[Policy](https://docs.cloud.google.com/iam/reference/rest/v1/Policy).

You can grant the Analytics Hub Publisher role (`roles/analyticshub.publisher`)
for an entire project from the
[IAM page](https://console.cloud.google.com/iam-admin),
which gives a user permission to add data to any data clean room in a project.
However, we don't recommend this action, as it might result in users having
overly permissive access.

### Manage data clean room subscribers

As a data clean room owner, you manage which users can subscribe to your data
clean rooms (your subscribers). To allow a user to subscribe to a data clean
room, grant them the
[Analytics Hub Subscriber role](https://docs.cloud.google.com/bigquery/docs/analytics-hub-grant-roles#ah-subscriber-role)
(`roles/analyticshub.subscriber`) and
[Analytics Hub Subscription Owner role](https://docs.cloud.google.com/bigquery/docs/analytics-hub-grant-roles#ah-subscription-owner-role)
(`roles/analyticshub.subscriptionOwner`) on a specific data clean room:

### Console

1. In the Google Cloud console, go to the **Sharing (Analytics Hub)** page.

   [Go to Sharing (Analytics Hub)](https://console.cloud.google.com/bigquery/analytics-hub)
2. Click the display name of the data clean room that you want to grant
   permissions to.

3. In the **Details** tab, click **Set permissions**.

4. Click **Add principal**.

5. For **New principals**, enter the usernames or emails of the data clean
   room subscribers that you're adding.

6. For **Select a role** , select **Analytics Hub \> Analytics Hub
   Subscriber**.

7. Click

   **Add another role**.

8. For **Select a role** , select **Analytics Hub \> Analytics Hub
   Subscription Owner**.

9. Click **Save**.

You can delete and update subscribers at any time by clicking
**Set Permissions**.

### API

Use the
[`projects.locations.dataExchanges.setIamPolicy` method](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges/setIamPolicy).

The following example shows how to call the `projects.locations.dataExchanges.setIamPolicy` method using the `curl` command:

```bash
curl -H "Authorization: Bearer $(gcloud auth print-access-token)" -H "Content-Type: application/json" -L -X POST https://analyticshub.googleapis.com/v1/projects/PROJECT_ID/locations/LOCATION/dataExchanges/CLEAN_ROOM_ID:setIamPolicy -d
'{
  "policy": {
    "bindings": [
      {
        "members": [
          "user:mike@example.com"
        ],
        "role": "roles/analyticshub.subscriptionOwner"
      },
      {
        "members": [
          "user:mike@example.com"
        ],
        "role": "roles/analyticshub.subscriber"
      }
    ]
  }
}'
```

The policy in the request body should conform to the structure of a [Policy](https://docs.cloud.google.com/iam/reference/rest/v1/Policy).

You can grant the Analytics Hub Subscriber role (`roles/analyticshub.subscriber`)
and Analytics Hub Subscription Owner role (`roles/analyticshub.subscriptionOwner`)
for an entire project from the
[IAM page](https://console.cloud.google.com/iam-admin), which gives a user
permission to subscribe to any data clean room in a project. However, we don't
recommend this action, as it might result in users having overly permissive
access.

### Share a data clean room

You can directly share a data clean room with subscribers:

1. In the Google Cloud console, go to the **Sharing (Analytics Hub)** page.

   [Go to Sharing (Analytics Hub)](https://console.cloud.google.com/bigquery/analytics-hub)
2. In the row of the data clean room that you want to share, click

   **More actions \> Copy share link**.

3. Share the copied link with data clean room subscribers to let them view and
   subscribe to the data clean room.

## Data contributor workflows

As a data contributor, you can do the following:

- Add data to a data clean room by creating a listing.
- Update a listing.
- Delete a listing.
- Share a data clean room.
- Monitor listings.

### Additional data contributor permissions

To perform data contributor tasks, you must have the
[Analytics Hub Publisher role](https://docs.cloud.google.com/bigquery/docs/analytics-hub-grant-roles#ah-publisher-role)
(`roles/analyticshub.publisher`) on a data clean room.

To perform data contributor tasks, you also need the
`bigquery.datasets.get`, `bigquery.datasets.update`, and
`bigquery.tables.get` permissions on the source dataset and table. These
permissions are available in the
[BigQuery Data Owner role](https://docs.cloud.google.com/bigquery/docs/access-control#bigquery.dataOwner)
(`roles/bigquery.dataOwner`).

To view data clean rooms in your organization that aren't in your
current project, you need the `resourcemanager.organization.get`
permission.

To use the data clean room creation process to create a
[BigQuery view](https://docs.cloud.google.com/bigquery/docs/views#creating_a_view)
with
[analysis rules](https://docs.cloud.google.com/bigquery/docs/analysis-rules),
you need the `bigquery.tables.setPrivacyPolicy` and
`bigquery.tables.create` permissions. If you reference the view directly,
this permission isn't required.

### Create a listing (add data)

> [!NOTE]
> **Note:** If your collaboration environment requires common identifiers to join data across data contributor and data clean room subscriber datasets, configure an [entity resolution](https://docs.cloud.google.com/bigquery/docs/entity-resolution-setup) before following these steps.

To prepare data with [analysis rules](https://docs.cloud.google.com/bigquery/docs/analysis-rules) and
publish to a data clean room as a listing, do the following:

### Console

1. In the Google Cloud console, go to the **Sharing (Analytics Hub)** page.

   [Go to Sharing (Analytics Hub)](https://console.cloud.google.com/bigquery/analytics-hub)
2. Click the display name of the data clean room that you want to create a
   listing in.

   If you're in a different organization than your data clean room owner and
   the data clean room is not visible to you, ask the data clean room owner for
   a direct link.
3. Click **Add data**.

4. For **Select dataset** and **Table/view name**, enter the table or view that
   you want to list in the data clean room and its corresponding dataset. You
   will add analysis rules to prevent raw access to this underlying data in a
   few steps.

5. Select the columns of your resource that you want to publish.

6. Set the view name, primary contact, and description (optional) for the
   listing.

7. Click **Next**.

8. Choose an analysis rule for your listing and configure the details.

9. Set [data egress](https://docs.cloud.google.com/bigquery/docs/analytics-hub-introduction#data_egress)
   controls for the listing.

10. Click **Next**.

11. Review the data and analysis rule that you're adding to the data clean
    room.

12. Click **Add data**. A view is created for your data and is added as a
    listing to the data clean room. The source table or view itself isn't added.

### API

Use the
[`projects.locations.dataExchanges.listings.create` method](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.listings/create).

The following example shows how to call the `projects.locations.dataExchanges.listings.create` method using the `curl` command:

```bash
  curl -H "Authorization: Bearer $(gcloud auth print-access-token)" -H "Content-Type: application/json" -H 'x-goog-user-project:DCR_PROJECT_ID' -X POST https://analyticshub.googleapis.com/v1/projects/DCR_PROJECT_ID/locations/LOCATION/dataExchanges/CLEAN_ROOM_ID/listings?listingId=LISTING_ID -d
  '{"bigqueryDataset":{"dataset":"projects/PROJECT_ID/datasets/DATASET_ID","selectedResources":[{"table":"projects/PROJECT_ID/datasets/DATASET_ID/tables/VIEW_ID"}],},"displayName":LISTING_NAME"}'
```

Replace the following:

- `DCR_PROJECT_ID`: the project ID of the project where the data clean room was created.
- `PROJECT_ID`: the project ID of the project where the source dataset was contained.
- `DATASET_ID`: your source dataset ID.
- `LOCATION`: the location of the data clean room.
- `CLEAN_ROOM_ID`: your data clean room ID.
- `LISTING_ID`: your listing ID.
- `LISTING_NAME`: your listing name.
- `VIEW_ID`: your view ID. The view that you add to a data clean room must be an [authorized view](https://docs.cloud.google.com/bigquery/docs/authorized-views) that is configured with [analysis rules](https://docs.cloud.google.com/bigquery/docs/analysis-rules).

<br />

By listing a resource in a data clean room, you grant all current and future data
clean room subscribers access to the data in your shared resource.

If you try to create a listing with a shared resource that doesn't have an
analysis rule, you're shown a warning that subscribers will be able to access
the raw data for that resource. If you confirm that you're willingly publishing
such resources without analysis rules, you can still create the listing.

If you get the `Failed to save listing` error, ensure that you have the
[necessary permissions to perform data contributor tasks](https://docs.cloud.google.com/bigquery/docs/data-clean-rooms#additional_data_contributor_permissions).

### Update a listing

### Console

1. In the Google Cloud console, go to the **Sharing (Analytics Hub)** page.

   [Go to Sharing (Analytics Hub)](https://console.cloud.google.com/bigquery/analytics-hub)
2. Click the display name of the data clean room that contains the listing.

3. In the row of the listing that you want to update, click

   **More actions \> Edit listing**.

4. Update the primary contact or description as needed.

5. Click **Next**.

6. Update the analysis rule as needed. You can only update the parameters of
   the chosen rule. You can't switch to a different rule.

7. Click **Next**.

8. Review the listing and click **Add data**.

### API

Use the
[`projects.locations.dataExchanges.listings.patch` method](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.listings/patch).

The following example shows how to call the `projects.locations.dataExchanges.listings.patch` method using the `curl` command:

```bash
curl -H "Authorization: Bearer $(gcloud auth print-access-token)" -H "Content-Type: application/json" -H 'x-goog-user-project:DCR_PROJECT_ID' -X PATCH https://analyticshub.googleapis.com/v1/projects/DCR_PROJECT_ID/locations/LOCATION/dataExchanges/CLEAN_ROOM_ID/listings/listingId=LISTING_ID?updateMask=displayName -d
'{"displayName":LISTING_NAME"}'
```

Replace the following:

- `DCR_PROJECT_ID`: the project ID of the project where the clean room was created.
- `LOCATION`: the location of the data clean room.
- `CLEAN_ROOM_ID`: your data clean room ID.
- `LISTING_ID`: your listing ID.
- `LISTING_NAME`: your listing name.

<br />

You can't change the source resource or data egress controls for a listing
after it's created.

### Delete a listing

### Console

1. In the Google Cloud console, go to the **Sharing (Analytics Hub)** page.

   [Go to Sharing (Analytics Hub)](https://console.cloud.google.com/bigquery/analytics-hub)
2. Click the display name of the data clean room that contains the listing.

3. In the row of the listing that you want to delete, click

   **More actions \> Delete listings**.

4. To confirm, enter `delete`, and then click **Delete**. You can't undo this
   action.

### API

Use the
[`projects.locations.dataExchanges.listings.delete` method](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.listings/delete).

The following example shows how to call the `projects.locations.dataExchanges.listings.delete` method using the `curl` command:

```bash
curl -H "Authorization: Bearer $(gcloud auth print-access-token)" -H "Content-Type: application/json" -H 'x-goog-user-project:DCR_PROJECT_ID' -X DELETE https://analyticshub.googleapis.com/v1/projects/DCR_PROJECT_ID/locations/LOCATION/dataExchanges/CLEAN_ROOM_ID/listings?listingId=LISTING_ID
```

Replace the following:

- `DCR_PROJECT_ID`: the project ID of the project where the clean room was created.
- `LOCATION`: the location of the data clean room.
- `CLEAN_ROOM_ID`: your data clean room ID.
- `LISTING_ID`: your listing ID.

<br />

When you delete a listing, the shared resources and linked datasets are not
deleted. The linked datasets are unlinked from the source datasets, so querying
data in that listing starts to fail for data clean room subscribers.

### Share a data clean room

You can directly share a data clean room with data clean room subscribers:

1. In the Google Cloud console, go to the **Sharing (Analytics Hub)** page.

   [Go to Sharing (Analytics Hub)](https://console.cloud.google.com/bigquery/analytics-hub)
2. In the row of the data clean room that you want to share, click

   **More actions \> Copy share link**.

3. Share the copied link with subscribers to let them view and subscribe to the
   data clean room.

### Monitor listings

You can view the usage metrics on the source datasets of the resources that you
share in a data clean room by querying the
[`INFORMATION_SCHEMA.SHARED_DATASET_USAGE` view](https://docs.cloud.google.com/bigquery/docs/information-schema-shared-dataset-usage).

To view your listing data clean room subscribers, do the following:

1. In the Google Cloud console, go to the **Sharing (Analytics Hub)** page.

   [Go to Sharing (Analytics Hub)](https://console.cloud.google.com/bigquery/analytics-hub)
2. Click the display name of the data clean room.

3. In the row of a listing that you want to view, click

   **More actions \> View subscriptions**.

## Data clean room subscriber workflows

A subscriber can view and subscribe to a data clean room. Subscribing to a data
clean room creates one linked dataset in the subscriber's project. Each linked
dataset has the same name as the data clean room.

You can't subscribe to a specific listing within a data clean room. You can only
subscribe to the data clean room itself.

### Additional subscriber permissions

You must have the
[Analytics Hub Subscriber](https://docs.cloud.google.com/bigquery/docs/analytics-hub-grant-roles#ah-subscriber-role)
(`roles/analyticshub.subscriber`) on a data clean room and
[Analytics Hub Subscription Owner](https://docs.cloud.google.com/bigquery/docs/analytics-hub-grant-roles#ah-subscription-owner-role)
(`roles/analyticshub.subscriptionOwner`) roles on the subscription project
to perform subscriber tasks.

In addition, you need the `bigquery.datasets.create` permission in a project to
create a linked dataset when you subscribe to a clean room.

### Subscribe to a data clean room

Subscribing to a data clean room gives you query access to the data in the
listings by creating a linked dataset in your project. To subscribe to a data
clean room, do the following:

### Console

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the **Explorer** pane, click

   **Add data**.

3. Select **Sharing (Analytics Hub)**. A discovery page opens.

4. To display the data clean rooms that you have access to, in the filters
   list, select **Clean rooms**.

5. Click the data clean room that you want to subscribe to. A description page
   of the data clean room opens. On this page, you can also see if the
   provider has enabled subscriber email logging.

6. Click **Subscribe**.

7. Select the destination project for the subscription and click **Subscribe**.

### API

Use the
[`projects.locations.dataExchanges.subscribe` method](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges/subscribe).

The following example shows how to call the `projects.locations.dataExchanges.subscribe` method using the `curl` command:

```bash
  curl -H "Authorization: Bearer $(gcloud auth print-access-token)" -H "Content-Type: application/json" -L -X POST https://analyticshub.googleapis.com/v1/projects/DCR_PROJECT_ID/locations/LOCATION/dataExchanges/CLEAN_ROOM_ID:subscribe  --data '{"destination":"projects/SUBSCRIBER_PROJECT_ID/locations/LOCATION","subscription":"SUBSCRIPTION"}'
```

Replace the following:

- `DCR_PROJECT_ID`: the project ID of the project where the clean room was created.
- `SUBSCRIBER_PROJECT_ID`: the project ID of the subscriber project.
- `LOCATION`: the location of the data clean room.
- `CLEAN_ROOM_ID`: your data clean room ID.
- `SUBSCRIPTION`: the name of your subscription.

<br />

In the body of the request, specify the dataset where you want to create the
[linked dataset](https://docs.cloud.google.com/bigquery/docs/analytics-hub-introduction#linked_datasets).
If the request is successful, the response body contains the
[subscription object](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.listings/subscribe#response-body).

If you enable subscriber email logging for the data clean room with the
`logLinkedDatasetQueryUserEmail` field, the subscription response contains
`log_linked_dataset_query_user_email: true`. The logged data is
available in the `job_principal_subject` field of the
[`INFORMATION_SCHEMA.SHARED_DATASET_USAGE` view](https://docs.cloud.google.com/bigquery/docs/information-schema-shared-dataset-usage).

A linked dataset is now added to the project that you specified and is available
for query.

As a data clean room subscriber, you can edit some metadata of your linked
datasets, such as description and labels. You can also set permissions on your
linked datasets. However, changes to linked datasets don't affect the source
datasets. You also can't see view definitions.

Resources that are contained in linked datasets are read-only. As a subscriber,
you can't edit data or metadata for resources in linked datasets. You also can't
specify permissions for individual resources within the linked dataset.

To unsubscribe to the data clean room, delete your linked dataset.

#### Query data in a linked dataset

To query data in a linked dataset, use the
[`SELECT WITH AGGREGATION_THRESHOLD` syntax](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#agg_threshold_clause),
which lets you run queries on analysis rule-enforced views. For an
example of this syntax, see
[Query an aggregation threshold analysis rule--enforced view](https://docs.cloud.google.com/bigquery/docs/analysis-rules#view_in_privacy_query).

## Example scenario: Advertiser and publisher attribution analysis

An advertiser wants to track the effectiveness of its marketing campaigns. The
advertiser has first-party data on its customers, including their purchase
history, demographics, and interests. The publisher has data from its website,
including which ads were shown to visitors and their conversions.

The advertiser and publisher agree to use a data clean room to combine data and
measure the results of their campaigns. In this case, the publisher creates the
data clean room and makes their data available for the advertiser to perform the
analysis. The result is an attribution report that shows the advertiser which
ads were most effective in driving sales. The advertiser can then use this
information to improve its future marketing campaigns.

The advertiser and publisher orchestrate the BigQuery data clean
room through the process described in the following sections.

### Create the data clean room (publisher)

1. A data clean room owner in the publisher organization enables the Analytics Hub API in their BigQuery project and assigns User A as the data clean room owner (Analytics Hub Admin (`roles/analyticshub.admin`)).
2. User A creates a data clean room called `Campaign Analysis` and assigns the following permissions:
   - Data contributor (Analytics Hub Publisher (`roles/analyticshub.publisher`)): User B, a data engineer in the publisher organization.
   - Data clean room subscriber (Analytics Hub Subscriber (`roles/analyticshub.subscriber`) and Subscription Owner (`roles/analyticshub.subscriptionOwner`)): User C, a marketing analyst in the advertiser organization.

### Add data to the data clean room (publisher)

1. User B creates a new listing in the data clean room called `Publisher Conversion Data`. As part of listing creation, a new view with analysis rules is created.

### Subscribe to the data clean room (advertiser)

1. User C subscribes to the data clean room, which creates a linked dataset for all listings in the data clean room, including the `Publisher Conversion Data` listing.
2. User C can now run aggregation queries to combine the data from this linked dataset with their first-party data to measure the campaign effectiveness.

## Entity resolution

Data clean room use cases often require linking entities across data contributor
and data clean room subscriber datasets that don't include a common identifier.
Subscribers and data contributors might represent the same
records differently in multiple datasets, either because datasets originate from
different data sources or because datasets use identifiers from different
namespaces.

As a part of [data preparation](https://docs.cloud.google.com/bigquery/docs/data-clean-rooms#add-data), entity resolution in
BigQuery does the following:

- For data contributors, it deduplicates and resolves records in their shared resources by using identifiers from a common provider of their choice. This process enables cross-contributor joins.
- For data clean room subscribers, it deduplicates and resolves records in their first-party datasets and links to entities in data contributor datasets. This process enables joins between subscriber and data contributor data.

To set up entity resolution with the identity provider of your choice, see
[Configure and use entity resolution in BigQuery](https://docs.cloud.google.com/bigquery/docs/entity-resolution-setup).

## Discover data clean room assets

To find all the data clean rooms that you have access to, do the following:

- For data clean room owners and data contributors, in the
  Google Cloud console, go to the **Sharing (Analytics Hub)** page.

  [Go to Sharing (Analytics Hub)](https://console.cloud.google.com/bigquery/analytics-hub)

  All the data clean rooms that you can access are listed.
- For data clean room subscribers, do the following:

  1. In the Google Cloud console, go to the **BigQuery** page.

     [Go to BigQuery](https://console.cloud.google.com/bigquery)
  2. In the **Explorer** pane, click

     **Add data**.

  3. Select **Sharing (Analytics Hub)**. A discovery page opens.

  4. To display the data clean rooms that you have access to, in the filters
     list, select **Clean rooms**.

To find all the linked datasets created by data clean rooms in your project, run
the following command in a command-line environment:

```bash
PROJECT=PROJECT_ID \
for dataset in $(bq ls --project_id $PROJECT | tail +3); \
do [ "$(bq show -d --project_id $PROJECT $dataset | egrep LINKED)" ] \
&& echo $dataset; done
```

Replace `PROJECT_ID` with the project that contains your
linked datasets.

> [!NOTE]
> **Note:** Linked datasets also have a different icon than standard datasets in the **Explorer** pane.

## Pricing

Data contributors are only charged for
[data storage](https://cloud.google.com/bigquery/pricing#storage).
Data clean room subscribers are only charged for
[compute (analysis)](https://cloud.google.com/bigquery/pricing#overview_of_pricing)
when they run queries.

## What's next

- Learn how to [use query templates](https://docs.cloud.google.com/bigquery/docs/query-templates).
- Learn how to [restrict data access with analysis rules](https://docs.cloud.google.com/bigquery/docs/analysis-rules).
- Learn how to [use VPC Service Controls](https://docs.cloud.google.com/bigquery/docs/analytics-hub-introduction#vpc-service).