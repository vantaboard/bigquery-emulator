# View and subscribe to listings and data exchanges

This document describes how to view and subscribe to listings and data exchanges
in BigQuery sharing (formerly Analytics Hub). As a BigQuery sharing
subscriber, you can view and subscribe to listings and data exchanges for which
you have access. Subscribing to a listing or a data exchange in
BigQuery sharing creates a linked dataset in your Google Cloud project.

## Required roles

To get the permissions that you need to use listings, ask your
BigQuery sharing administrator to grant you the following
Identity and Access Management (IAM) roles on the BigQuery sharing subscriber
project:

- [Discover listings](https://docs.cloud.google.com/bigquery/docs/analytics-hub-view-subscribe-listings#discover-listings): [Analytics Hub Viewer](https://docs.cloud.google.com/iam/docs/roles-permissions/analyticshub#analyticshub.viewer) (`roles/analyticshub.viewer`)
- [Discover data exchanges](https://docs.cloud.google.com/bigquery/docs/analytics-hub-view-subscribe-listings#discover-data-exchanges): [Analytics Hub Viewer](https://docs.cloud.google.com/iam/docs/roles-permissions/analyticshub#analyticshub.viewer) (`roles/analyticshub.viewer`)
- [Subscribe to listings](https://docs.cloud.google.com/bigquery/docs/analytics-hub-view-subscribe-listings#subscribe-listings):
  [BigQuery User](https://docs.cloud.google.com/bigquery/docs/access-control#bigquery.user) (`roles/bigquery.user`)

  - To subscribe to listings, you must also ask the BigQuery sharing listing publisher to grant you the [Analytics Hub Subscriber role](https://docs.cloud.google.com/bigquery/docs/access-control#analyticshub.subscriber) (`roles/analyticshub.subscriber`) on their listing, exchange, or project, depending on the scope that's most appropriate for your use case.
- [Subscribe to data exchanges](https://docs.cloud.google.com/bigquery/docs/analytics-hub-view-subscribe-listings#subscribe-data-exchanges):
  [BigQuery User](https://docs.cloud.google.com/bigquery/docs/access-control#bigquery.user) (`roles/bigquery.user`)

  - To subscribe to data exchanges in the context of data clean room exchanges, you must also ask the BigQuery sharing exchange publisher to grant you the Analytics Hub Subscriber role (`roles/analyticshub.subscriber`) on the specific data clean room. Additionally, you must ask the destination project owners in the BigQuery sharing subscriber organization to grant you the [Analytics Hub Subscription Owner role](https://docs.cloud.google.com/bigquery/docs/access-control#analyticshub.subscriptionOwner) (`roles/analyticshub.subscriptionOwner`) on the destination project.
- [View linked datasets](https://docs.cloud.google.com/bigquery/docs/analytics-hub-view-subscribe-listings#view-linked-datasets):
  [BigQuery Data Viewer](https://docs.cloud.google.com/bigquery/docs/access-control#bigquery.dataViewer)
  (`roles/bigquery.dataViewer`)

- [Query linked datasets](https://docs.cloud.google.com/bigquery/docs/analytics-hub-view-subscribe-listings#query-linked-datasets):
  [BigQuery Data Viewer](https://docs.cloud.google.com/bigquery/docs/access-control#bigquery.dataViewer)
  (`roles/bigquery.dataViewer`)

- [Update linked datasets](https://docs.cloud.google.com/bigquery/docs/analytics-hub-view-subscribe-listings#update-linked-datasets):
  [BigQuery Data Owner](https://docs.cloud.google.com/bigquery/docs/access-control#bigquery.dataOwner)
  (`roles/bigquery.dataOwner`)

- [View table metadata](https://docs.cloud.google.com/bigquery/docs/analytics-hub-view-subscribe-listings#view-table-metadata):
  [BigQuery Data Viewer](https://docs.cloud.google.com/bigquery/docs/access-control#bigquery.dataViewer)
  (`roles/bigquery.dataViewer`)

- [Delete linked datasets](https://docs.cloud.google.com/bigquery/docs/analytics-hub-view-subscribe-listings#delete-linked-datasets):
  [BigQuery Admin](https://docs.cloud.google.com/bigquery/docs/access-control#bigquery.admin)
  (`roles/bigquery.admin`)

For more information about granting roles, see
[Manage access to projects, folders, and organizations](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).

These predefined roles contain the permissions required to perform the tasks in
this document. To see the exact permissions that are required to create and
query datasets, expand the **Required permissions** section:

#### Required permissions

- Create new datasets: `bigquery.datasets.create` or `bigquery.datasets.*` to perform additional actions on datasets.
- Query datasets: `bigquery.jobs.create` or `bigquery.jobs.*` to perform additional actions on jobs.

You might also be able to get these permissions with
[custom roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles)
or other
[predefined roles](https://docs.cloud.google.com/iam/docs/roles-overview#predefined).

## Discover listings

To discover public and private listings, follow these steps:

1. In the Google Cloud console, go to the **Sharing (Analytics Hub)** page.

   [Go to Sharing (Analytics Hub)](https://console.cloud.google.com/bigquery/analytics-hub)
2. Click **Search listings**. A dialog appears containing listings that you can
   access.

3. To filter listings by their name or description, enter the name or
   description of the listing in the **Search for listings** field.

4. In the **Filters** section, you can filter listings based on the following
   fields:

   - **Listings** : select whether you want to view private listings, public
     listings, or
     [listings](https://docs.cloud.google.com/bigquery/docs/analytics-hub-introduction#listings)
     within your organization.

   - **Categories**: select one or more categories.

   - **Location** : select a location. You can only search by data
     exchange location. For more information, see
     [Supported regions](https://docs.cloud.google.com/bigquery/docs/analytics-hub-introduction#supported-regions).

   - **Provider**: select the data provider. Some data providers require you to
     request access to their commercial datasets. After requesting access,
     the data provider contacts you to share their datasets.

5. Browse through the filtered listings.

## Discover data exchanges

To discover data exchanges, follow these steps:

1. In the Google Cloud console, go to the **Sharing (Analytics Hub)** page.

   [Go to Sharing (Analytics Hub)](https://console.cloud.google.com/bigquery/analytics-hub)
2. Click **Search listings**. A dialog appears containing listings and data
   exchanges that you can subscribe to.

3. To filter data exchanges by their name or description, enter the name or
   description of the data clean room exchange in the **Search for listings**
   field.

4. In the **Filters** section, you can filter data clean room exchanges based on
   the following fields:

   - **Listings** : select the **Clean rooms** checkbox to view the data clean
     rooms shared with you.

   - **Categories**: select one or more categories.

   - **Location** : select a location. You can only search by the data
     exchange location. For more information, see
     [Supported regions](https://docs.cloud.google.com/bigquery/docs/analytics-hub-introduction#supported-regions).

5. Browse through the filtered data clean rooms.

## Subscribe to listings

Subscribing to a [listing](https://docs.cloud.google.com/bigquery/docs/analytics-hub-introduction#listings)
gives you *read-only access* to the data in the listing by creating a
[linked dataset](https://docs.cloud.google.com/bigquery/docs/analytics-hub-introduction#linked_datasets)
in your project.

> [!CAUTION]
> **Caution:** We recommend that you avoid placing data in a project that is within a VPC Service Controls perimeter. If you do so, then you must add the appropriate [ingress and egress rules](https://docs.cloud.google.com/bigquery/docs/analytics-hub-vpc-sc-rules#subscribe_to_a_listing).

To subscribe to a listing, follow these steps:

### Console

1. To view a list of listings that you have access to, follow the steps in
   [Discover listings](https://docs.cloud.google.com/bigquery/docs/analytics-hub-view-subscribe-listings#discover-listings).

2. Browse through the listings and click a listing that you want to subscribe
   to. A dialog with the listing details appears. The dialog shows if the
   provider enabled subscriber email logging. In the **Additional details**
   section, you can see the regions where the provider made the listing
   available.

3. If you don't have access to subscribe to a listing, such as a listing
   referencing a *commercial dataset* , then click **Request access** or
   **Purchase via Marketplace** . If you click a dataset that you can subscribe
   to, then click **Subscribe** to open the **Create linked dataset** dialog.

4. If you don't have the Analytics Hub API enabled in your project,
   an error message appears with a link to enable the API. Click
   **Enable Analytics Hub API**.

5. In the **Create linked dataset** dialog, specify the following details:

   - **Project**: specify the name of the project in which you want to add the dataset.
   - **Linked dataset name**: specify the name of the linked dataset.
   - **Primary region**: select the region where you want to create the
     linked dataset.

     > [!NOTE]
     > **Note:** The selected primary region doesn't need to be the same as the provider's primary region. You might choose to colocate your linked dataset in the same region as the provider to minimize data replication latency.

   - Optional: **Replica regions** :
     select the region or regions where you want to create additional linked
     dataset secondary replicas. You might choose to colocate your linked
     dataset in the same region as your other data to minimize egress and
     facilitate cross-dataset joins. To create linked dataset replicas, you
     must have the `bigquery.datasets.update` permission on the linked dataset.

   > [!NOTE]
   > **Note:** Linked dataset replicas are created on a best-effort basis. If permissions are missing, replicas aren't created.

6. To save your changes, click **Save**. The linked dataset is listed in your
   project.

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

In the body of the request, specify the dataset where you want to create the
[linked dataset](https://docs.cloud.google.com/bigquery/docs/analytics-hub-introduction#linked_datasets).

To create a subscription with linked dataset replicas available in multiple
regions, specify the primary region of the linked dataset using the `location` field in the request body.
For the secondary regions where you want to create linked dataset replicas,
you can optionally use the `destinationDataset.replica_locations` field in
the request body and list all the selected secondary replica regions. Ensure
that the specified regions in the `location` property and in the
`destinationDataset.replica_locations` field are regions where the associated
listing is available.

> [!NOTE]
> **Note:** Linked dataset replicas are created on a best-effort basis. If the `bigquery.datasets.update` permission is missing on the linked dataset, replicas aren't created.

If the request is successful, the response body contains the
[subscription object](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.listings/subscribe#response-body).

If you enable subscriber email logging for the data exchange or listing with
the `logLinkedDatasetQueryUserEmail` field, the subscription response contains
`log_linked_dataset_query_user_email: true`. The logged data is available in
the `job_principal_subject` field of the
[`INFORMATION_SCHEMA.SHARED_DATASET_USAGE` view](https://docs.cloud.google.com/bigquery/docs/information-schema-shared-dataset-usage).

If you enable stored procedure sharing
([Preview](https://cloud.google.com/products#product-launch-stages)), the listing response
contains `stored_procedure_config: true`.

> [!NOTE]
> **Note:** BigQuery sharing subscribers must [authorize shared stored procedures](https://docs.cloud.google.com/bigquery/docs/authorized-routines) in a linked dataset to read from and write to certain resources owned by the subscriber.

## Subscribe to data exchanges

Subscribing to a
[data exchange](https://docs.cloud.google.com/bigquery/docs/analytics-hub-introduction#data_exchanges)
gives you read-only access to the data in the data clean room exchange by
creating a
[linked dataset](https://docs.cloud.google.com/bigquery/docs/analytics-hub-introduction#linked_datasets)
in your project.

To subscribe to a data clean room exchange, follow these steps:

### Console

1. To view a list of data clean room exchanges that you have access to, follow
   the steps in
   [Discover data exchanges](https://docs.cloud.google.com/bigquery/docs/analytics-hub-view-subscribe-listings#discover-data-exchanges).

2. Browse through the data clean room exchanges and click a data clean room
   exchange that you want to subscribe to. A dialog with the data clean room
   exchange details appears.

3. If you click a data clean room exchange that you can subscribe
   to, then click **Subscribe** to open the
   **Add data clean room to project** dialog.

4. If you don't have the Analytics Hub API enabled in your project,
   an error message appears with a link to enable the API. Click
   **Enable Analytics Hub API**.

5. In the **Add data clean room to project** dialog, specify the following
   details:

   - **Destination**: specify the name of the project in which you want to add the dataset.
6. To save your changes, click **Save**. The linked dataset is listed in your
   project.

### API

Use the
[`projects.locations.dataExchanges.subscribe` method](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges/subscribe).

```
POST https://analyticshub.googleapis.com/v1/projects/PROJECT_ID/location/LOCATION/dataExchanges/DATAEXCHANGE_ID:subscribe
```

Replace the following:

- `PROJECT_ID`: the project ID of the data exchange that you want to subscribe to.
- `LOCATION`: the location for your data exchange that you want to subscribe to.
- `DATAEXCHANGE_ID`: the data exchange ID that you want to subscribe to.

In the body of the request, specify the dataset where you want to create the
[linked dataset](https://docs.cloud.google.com/bigquery/docs/analytics-hub-introduction#linked_datasets).

If the request is successful, the response body contains the
[subscription object](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges/subscribe#response-body).
If you have enabled subscriber email logging for the data exchange, the subscription response contains
`log_linked_dataset_query_user_email: true`.

## View linked datasets

Linked datasets are displayed together with other datasets in the
Google Cloud console.

To view linked datasets in your project, follow these steps:

### Console

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the **Classic Explorer** pane, click category **Classic Explorer**:

   ![Highlighted button for the Classic Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/classic-explorer-tab.png)

   If the **Classic Explorer** pane is not visible, click **Expand left pane** to open the pane.
3. In the **Classic Explorer** pane, click the project name that contains the
   ![Analytics Hub linked dataset icon](https://docs.cloud.google.com/static/bigquery/images/analytics-hub-linked-dataset.png)
   linked dataset.

Alternatively, you can also use
[Data Catalog (deprecated)](https://docs.cloud.google.com/data-catalog/docs/how-to/search#how_to_search_for_data_assets)
or
[Knowledge Catalog](https://docs.cloud.google.com/dataplex/docs/search-assets)
to search and view linked datasets. To match all BigQuery sharing
linked datasets, use the `type=dataset.linked` predicate. For more
information, see
[Data Catalog search syntax](https://docs.cloud.google.com/data-catalog/docs/how-to/search-reference)
or
[Knowledge Catalog search syntax](https://docs.cloud.google.com/dataplex/docs/search-syntax).

### Cloud Shell

Run the following command:

```
PROJECT=PROJECT_ID \
for dataset in $(bq ls --project_id $PROJECT | tail +3); do [ "$(bq show -d --project_id $PROJECT $dataset | egrep LINKED)" ] && echo $dataset; done
```

Replace `PROJECT_ID` with your Google Cloud project ID.

> [!NOTE]
> **Note:** If a BigQuery sharing publisher [removes the subscription](https://docs.cloud.google.com/bigquery/docs/analytics-hub-manage-listings#remove_a_subscription), then the linked dataset details page shows that the dataset is unlinked. You can delete an [unlinked dataset](https://docs.cloud.google.com/bigquery/docs/analytics-hub-view-subscribe-listings#delete-linked-datasets) because you can't query an unlinked dataset.

## Query linked datasets

You can query tables and views in your linked datasets
in the same way you
[query any other BigQuery table](https://docs.cloud.google.com/bigquery/docs/managing-table-data#querying_table_data).

## Update linked datasets

Resources in a linked dataset are *read-only*. You can't edit the data or
metadata for resources in linked datasets, or specify permissions for individual
resources.

You can only update the description and labels of your linked datasets.
Changes to a linked dataset don't affect the source or shared datasets.

To update the description and labels of a linked dataset, follow these steps:

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the **Explorer** pane, click
   **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)
3. In the **Explorer** pane, expand your project name, click **Datasets**, and
   then click the name of the linked dataset to open it.

4. In the details pane, click **Edit details** and then specify the following details:

   1. To add labels, see [Adding a label to a dataset](https://docs.cloud.google.com/bigquery/docs/adding-labels#adding_a_label_to_a_dataset).
   2. To enable
      [collation](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/collation-concepts),
      expand the **Advanced options** section and follow these steps:

      1. Select **Enable default collation**.
      2. From the **Default collation** list, select an option.
5. Click **Save**.

## View table metadata

To view the underlying table metadata, query the
[`INFORMATION_SCHEMA.TABLES`](https://docs.cloud.google.com/bigquery/docs/information-schema-tables)
view:

```
SELECT * FROM `LINKED-DATASET.INFORMATION_SCHEMA.TABLES`
```

Replace <var translate="no">LINKED-DATASET</var> with the name of your linked dataset.

> [!NOTE]
> **Note:** [Region-based `INFORMATION_SCHEMA` queries](https://docs.cloud.google.com/bigquery/docs/information-schema-intro#region_qualifier) don't return metadata of linked tables. To learn about `INFORMATION_SCHEMA` views that don't support dataset qualifiers for linked datasets, see [Limitations](https://docs.cloud.google.com/bigquery/docs/analytics-hub-introduction#limitations).

## Unsubscribe from or delete linked datasets

To unsubscribe from a dataset, you must delete the linked dataset. Deleting a
linked dataset doesn't delete the source dataset.

You cannot retrieve a linked dataset after you delete it. However, you can
recreate the deleted linked dataset by
[subscribing to the listing](https://docs.cloud.google.com/bigquery/docs/analytics-hub-view-subscribe-listings#subscribe-listings) again and adding the
dataset and the linked datasets created from Google Cloud Marketplace-integrated
listings to your project.

If your
[subscription is removed](https://docs.cloud.google.com/bigquery/docs/analytics-hub-manage-listings#remove_a_subscription)
by a BigQuery sharing publisher, then your
[linked dataset](https://docs.cloud.google.com/bigquery/docs/analytics-hub-introduction#listings)
is unlinked from the
[shared dataset](https://docs.cloud.google.com/bigquery/docs/analytics-hub-introduction#shared_datasets).
Since this is a publisher-initiated action on a subscriber-owned resource, the
linked dataset remains in the BigQuery sharing subscriber's project in an
unlinked state. You can remove the unlinked dataset by deleting it.

To delete a linked dataset, do the following:

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the **Explorer** pane, click
   **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)
3. In the **Explorer** pane, expand your project name, click **Datasets**, and
   then click the name of the linked dataset to open it.

4. Click **Delete**.

5. In the **Delete linked dataset?** dialog, confirm deletion by typing
   **delete**.

6. Click **Delete**.

## What's next

- Learn about [BigQuery sharing](https://docs.cloud.google.com/bigquery/docs/analytics-hub-introduction).
- Learn about [managing listings](https://docs.cloud.google.com/bigquery/docs/analytics-hub-manage-listings).
- Learn about [managing data exchanges](https://docs.cloud.google.com/bigquery/docs/analytics-hub-manage-exchanges).
- Learn about [BigQuery sharing audit logging](https://docs.cloud.google.com/bigquery/docs/analytics-hub-audit-logging).