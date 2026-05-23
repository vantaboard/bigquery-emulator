# Use query templates

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

> [!NOTE]
> **Note:** To request support or provide feedback for this feature, contact [bq-data-sharing-feedback@google.com](mailto:bq-data-sharing-feedback@google.com).

BigQuery data clean room query templates accelerate time to insight
and provide additional layers of security and control to minimize data
exfiltration concerns. By predefining and limiting the queries that can be
executed in data clean rooms, you can do the following:

- **Help prevent the leakage of sensitive data**. When data clean room
  subscribers run queries in a clean room, greater flexibility to explore can
  increase the risk of accidental or intentional exposure of sensitive
  information for data owners.

- **Simplify onboarding and adoption for less-technical users**. Many data
  providers expect data clean room subscribers to have less technical
  proficiency, especially with writing privacy-centric SQL queries and
  allocating privacy budgets.

- **Guarantee consistent analytical outcomes for data clean room subscribers**.
  Without controlling the queries executed in a data clean room, it becomes more
  difficult to enforce specific data analysis rules and verify compliance with
  privacy regulations.

Query templates let data owners and contributors create predefined and
approved queries tailored to the data clean room's use cases. They can also
publish these queries for subscribers to consume. Predefined queries use
[table-valued functions (TVFs)](https://docs.cloud.google.com/bigquery/docs/table-functions) in
BigQuery to pass an entire table or specific fields as input
parameters and return a table as the output.

## Limitations

- Query templates only support a maximum of two data references---that is, the data used to define the TVF's query and the data parameter input that the TVF accepts.
  - Multiple tables or views can be referenced within the TVF's query definition, but they must all belong to the same data owner or party.
- Query template TVFs only support `TABLE` and `VIEW` fixed types.
- Query template definitions are subject to [the same limitations as TVFs](https://docs.cloud.google.com/bigquery/docs/table-functions#limitations).

## Before you begin

Enable the Analytics Hub API for your Google Cloud project by following these steps:

### Console

1. Go to the **Analytics Hub API** page.

   [Go to Analytics Hub API](https://console.cloud.google.com/apis/library/analyticshub.googleapis.com)
2. In the Google Cloud console toolbar, select your project.

3. If the API is not already enabled, click **Enable**.

### bq

Run the
[`gcloud services enable` command](https://docs.cloud.google.com/sdk/gcloud/reference/services/enable):

```bash
gcloud services enable analyticshub.googleapis.com
```

### Required roles


To get the permissions that
you need to perform the tasks in this document,

ask your administrator to grant you the
following IAM roles:

- Create or delete a TVF in a data clean room:
  - [Analytics Hub Publisher](https://docs.cloud.google.com/iam/docs/roles-permissions/analyticshub#analyticshub.publisher) (`roles/analyticshub.publisher`) on the project
  - [Analytics Hub Subscriber](https://docs.cloud.google.com/iam/docs/roles-permissions/analyticshub#analyticshub.subscriber) (`roles/analyticshub.subscriber`) on the project
- Authorize a TVF: [BigQuery Data Owner](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquery#bigquery.dataOwner) (`roles/bigquery.dataOwner`) on the project
- Add, update, or delete a TVF listing in a data clean room:
  - [Analytics Hub Publisher](https://docs.cloud.google.com/iam/docs/roles-permissions/analyticshub#analyticshub.publisher) (`roles/analyticshub.publisher`) on the project
  - [Analytics Hub Subscriber](https://docs.cloud.google.com/iam/docs/roles-permissions/analyticshub#analyticshub.subscriber) (`roles/analyticshub.subscriber`) on the project
- Create a query template:
  - [Analytics Hub Publisher](https://docs.cloud.google.com/iam/docs/roles-permissions/analyticshub#analyticshub.publisher) (`roles/analyticshub.publisher`) on the project
  - [Analytics Hub Subscriber](https://docs.cloud.google.com/iam/docs/roles-permissions/analyticshub#analyticshub.subscriber) (`roles/analyticshub.subscriber`) on the project
- Approve a query template:
  - [Analytics Hub Publisher](https://docs.cloud.google.com/iam/docs/roles-permissions/analyticshub#analyticshub.publisher) (`roles/analyticshub.publisher`) on the project
  - [BigQuery Data Owner](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquery#bigquery.dataOwner) (`roles/bigquery.dataOwner`) on the project
- Subscribe to a data clean room with query templates:
  - [Analytics Hub Subscriber](https://docs.cloud.google.com/iam/docs/roles-permissions/analyticshub#analyticshub.subscriber) (`roles/analyticshub.subscriber`) on the project
  - [Analytics Hub Subscription Owner](https://docs.cloud.google.com/iam/docs/roles-permissions/analyticshub#analyticshub.subscriptionOwner) (`roles/analyticshub.subscriptionOwner`) on the project where you want to subscribe to the data clean room
- Execute the queries defined in query templates:
  - [BigQuery Data Viewer](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquery#bigquery.dataViewer) (`roles/bigquery.dataViewer`) on the project
  - [BigQuery User](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquery#bigquery.user) (`roles/bigquery.user`) on the project


For more information about granting roles, see [Manage access to projects, folders, and organizations](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).


These predefined roles contain

the permissions required to perform the tasks in this document. To see the exact permissions that are
required, expand the **Required permissions** section:


#### Required permissions

The following permissions are required to perform the tasks in this document:

- Create or delete a TVF in a data clean room:
  - `bigquery.routines.create` on the project
  - `bigquery.routines.update` on the project
  - `bigquery.routines.delete` on the project
- Authorize a TVF: `bigquery.datasets.update` on the datasets that the routine accesses
- Create a query template:
  - `analyticshub.listings.subscribe` on the project
  - `analyticshub.queryTemplates.create` on the project
- Approve a query template:
  - `bigquery.routines.create` on the project
  - `bigquery.datasets.update` on the datasets that the routine accesses
  - `analyticshub.listings.create` on the project
  - `analyticshub.queryTemplates.approve` on the project


You might also be able to get
these permissions
with [custom roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles) or
other [predefined roles](https://docs.cloud.google.com/iam/docs/roles-overview#predefined).

## Add an existing TVF to a data clean room

You can add an existing TVF to a data clean room using the Analytics Hub API.

Use the
[`projects.locations.dataExchanges.listings.create` method](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.listings/create).

The following example shows how to call the
`projects.locations.dataExchanges.listings.create` method using the `curl` command:

```bash
    curl -H "Authorization: Bearer $(gcloud auth print-access-token)" -H "Content-Type: application/json" -H 'x-goog-user-project:DCR_PROJECT_ID' -X POST https://analyticshub.googleapis.com/v1/projects/DCR_PROJECT_ID/locations/LOCATION/dataExchanges/CLEAN_ROOM_ID/listings?listingId=LISTING_ID -d
    '{"bigqueryDataset":{"dataset":"projects/PROJECT_ID/datasets/DATASET_ID","selectedResources":[{"routine":"projects/PROJECT_ID/datasets/DATASET_ID/tables/ROUTINE_ID"}],},"displayName":LISTING_NAME"}'
```

Replace the following:

- `DCR_PROJECT_ID`: the project ID of the project where the data clean room was created.
- `PROJECT_ID`: the project ID of the project where the source dataset was contained.
- `DATASET_ID`: the source dataset ID.
- `LOCATION`: the location of the data clean room.
- `CLEAN_ROOM_ID`: the data clean room ID.
- `LISTING_ID`: the listing ID.
- `LISTING_NAME`: the listing name.
- `ROUTINE_ID`: the routine ID.

## Query template roles

There are three main roles for using data clean room query templates. Each role
has specific workflows, which are described later in this document.

- **Template creator** : a user who defines the queries to be executed within
  the clean room. This role is analogous to any of the following
  IAM roles:
  [Analytics Hub Admin](https://docs.cloud.google.com/bigquery/docs/analytics-hub-grant-roles#ah-admin-role),
  [Analytics Hub Publisher](https://docs.cloud.google.com/bigquery/docs/analytics-hub-grant-roles#ah-publisher-role),
  or [Analytics Hub Listing Admin](https://docs.cloud.google.com/bigquery/docs/analytics-hub-grant-roles#ah-publisher-role).
  For more information, see
  [Template creator workflows](https://docs.cloud.google.com/bigquery/docs/query-templates#template-creator-workflows).

- **Template approver** : the owner of the data who must approve the query
  template's references before the template is available for use. This role
  is analogous to any of the following IAM roles:
  [Analytics Hub Admin](https://docs.cloud.google.com/bigquery/docs/analytics-hub-grant-roles#ah-admin-role),
  [Analytics Hub Publisher](https://docs.cloud.google.com/bigquery/docs/analytics-hub-grant-roles#ah-publisher-role),
  or [Analytics Hub Listing Admin](https://docs.cloud.google.com/bigquery/docs/analytics-hub-grant-roles#ah-publisher-role).
  For more information, see
  [Template approver workflows](https://docs.cloud.google.com/bigquery/docs/query-templates#template-approver-workflows).

- **Template subscriber** : a user who subscribes to the clean room and can
  only run the queries that are approved in the template. This role is analogous
  to the
  [Analytics Hub Subscriber](https://docs.cloud.google.com/bigquery/docs/analytics-hub-grant-roles#ah-subscriber-role)
  IAM role. For more information, see
  [Template subscriber workflows](https://docs.cloud.google.com/bigquery/docs/query-templates#template-subscriber-workflows).

## Template creator workflows

As a query template creator, you can do the following:

- [Create a query template](https://docs.cloud.google.com/bigquery/docs/query-templates#create-query-template).
- [Update a query template](https://docs.cloud.google.com/bigquery/docs/query-templates#update-query-template).
- [Submit a query template for review](https://docs.cloud.google.com/bigquery/docs/query-templates#submit-query-template).
- [Delete a query template](https://docs.cloud.google.com/bigquery/docs/query-templates#delete-query-template).

### Add a listing to a data clean room

Before creating a query template, you must add data to a data clean room. To
create a listing in the data clean room, follow these steps:

1. Go to the **Sharing (Analytics Hub)** page.

   [Go to Sharing (Analytics Hub)](https://console.cloud.google.com/bigquery/analytics-hub)
2. Click the display name of the data clean room that you want to create the
   query template in.

3. Click **Add data** and follow the steps to create a view with
   analysis rules configured. For detailed instructions, see
   [Create a listing (add data)](https://docs.cloud.google.com/bigquery/docs/data-clean-rooms#add-data).

   1. To add data from other parties, share the clean room with another [trusted contributor](https://docs.cloud.google.com/bigquery/docs/data-clean-rooms#data_contributor_workflows). This data contributor must also add data to the clean room to be eligible for use in a query template.
4. Set [data egress](https://docs.cloud.google.com/bigquery/docs/analytics-hub-introduction#data_egress)
   controls for the listing.

5. Set the metadata controls for the listing. If you only want to share the
   schemas and descriptions of the data added in the previous step (and not the
   shared data itself), select **Exclude access to listing from linked dataset**.

   > [!NOTE]
   > **Note:** You must have the Analytics Hub Subscriber (`roles/analyticshub.subscriber`) role at the data clean room exchange level before you attempt to create a query template. The Analytics Hub Subscriber role lets you view the schemas of the data added to the clean room.

   > [!NOTE]
   > **Note:** You cannot update a listing to enable the metadata controls, so we recommend creating query templates on new listings. This verifies that shared data is not exposed to template subscribers, including data contributors who need the Analytics Hub Subscriber role to view the metadata.

6. Review the listing details.

7. Click **Add data**. The metadata of the view that is created for your data
   is now added to the clean room.

### Create a query template

Select one of the following options:

### Console

1. Go to the **Sharing (Analytics Hub)** page.

   [Go to Sharing (Analytics Hub)](https://console.cloud.google.com/bigquery/analytics-hub)
2. Click the display name of the data clean room where you want to create the
   query template.

3. In the clean room, go to the **Templates** tab.

4. Click **Create Template**.

5. Enter a template name and description.

   > [!NOTE]
   > **Note:** You can't edit the template name after creating the query template.

6. Click **Next**.

7. You can see the schemas of the views added to the clean room, and you can
   propose a query definition.

   1. Be sure to define the query using the supported [`CREATE TABLE FUNCTION` syntax](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_table_function_statement).
   2. Pass your entire table or view with fixed definitions. You must define
      the full table path reference, including the project ID and dataset ID,
      from the data added to the clean room. For example:

          query_template1(t1 TABLE<year INT64>) AS (SELECT * FROM `project_id.dataset_id.table_id` WHERE year = table_id.year)

   3. If you applied privacy analysis rules to the data, be sure this
      TVF includes privacy-specific SQL syntax, for example,
      [`SELECT WITH AGGREGATION_THRESHOLD`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#agg_threshold_clause).

   > [!NOTE]
   > **Note:** Routine definitions are always hidden and never shared with template subscribers. Subscribers only see the table input parameter that the TVF expects.

8. Review the template details.

9. To save the template without submitting for review, click **Save** .
   The query template now has the status of **DRAFT**.

You can [update the query template](https://docs.cloud.google.com/bigquery/docs/query-templates#update-query-template) or
[submit the query template for review](https://docs.cloud.google.com/bigquery/docs/query-templates#submit-query-template).

> [!NOTE]
> **Note:** You must [authorize the TVF or routine](https://docs.cloud.google.com/bigquery/docs/authorized-routines#authorize_routines) for template subscribers to query the routine. If you modify a routine by running a `CREATE OR REPLACE` statement, such as `CREATE OR REPLACE FUNCTION`, `CREATE OR REPLACE PROCEDURE`, or `CREATE OR REPLACE TABLE FUNCTION`, you must re-authorize the routine.

### API

The following example shows how to create a query template with a `curl`
command:

```bash
curl -H "Authorization: Bearer $(gcloud auth print-access-token)" -H "Content-Type: application/json" -H 'x-goog-user-project:DCR_PROJECT_ID' -X POST https://analyticshub.googleapis.com/v1/projects/DCR_PROJECT_ID/locations/LOCATION/dataExchanges/CLEAN_ROOM_ID/queryTemplates?queryTemplateId=QUERY_TEMPLATE_ID -d
  'query_template {
  display_name: "DISPLAY_NAME",
  routine {
    definition_body: "QUERY_TEMPLATE_ID(TVF arguments) AS (TVF_DEFINITION)"
  }
}'
```

Replace the following:

- `DCR_PROJECT_ID`: the project ID of the project where the data clean room was created.
- `LOCATION`: the location of the data clean room.
- `CLEAN_ROOM_ID`: the data clean room ID.
- `DISPLAY_NAME`: the display name of the query template. You can't edit the display name after creating the query template.
- `QUERY_TEMPLATE_ID`: the query template ID.
- `TVF_DEFINITION`: the TVF definition.

The following code sample shows a `definition_body` example for the API call.
You must define the full table path reference, including the project ID and
dataset ID, from the data added to the clean room.

      query_template1(t1 TABLE<year INT64>) AS (SELECT * FROM `project_id.dataset_id.table_id` WHERE year = table_id.year)

The `definition_body` is analogous to the definition of a routine. The
preceding `definition_body` translates to the following routine:

      CREATE OR REPLACE TABLE FUNCTION <approvers_dataset>.query_template1(t1 TABLE, y INT64)
      AS (SELECT * FROM t1 WHERE year > y)

You can [update the query template](https://docs.cloud.google.com/bigquery/docs/query-templates#update-query-template) or
[submit the query template for review](https://docs.cloud.google.com/bigquery/docs/query-templates#submit-query-template).

### Update a query template

You can only update a query template if it is in the **DRAFT** status. If the
query template has already been submitted for review, you can no longer modify
it.

To update a query template, select one of the following options:

### Console

1. Go to the **Sharing (Analytics Hub)** page.

   [Go to Sharing (Analytics Hub)](https://console.cloud.google.com/bigquery/analytics-hub)
2. Click the display name of the data clean room that contains the query
   template.

3. In the clean room, go to the **Templates** tab.

4. In the row for the template you want to update, click
   **Actions \> Edit template**.

5. Update the description and primary contact as needed.

> [!NOTE]
> **Note:** The query template display name can't be updated.

1. Click **Next**.
2. Review the query template and click **Save** to save the changes without submitting the template for review.

> [!NOTE]
> **Note:** You must [authorize the TVF or routine](https://docs.cloud.google.com/bigquery/docs/authorized-routines#authorize_routines) for template subscribers to query the routine. If you modify a routine by running a `CREATE OR REPLACE` statement, such as `CREATE OR REPLACE
> FUNCTION`, `CREATE OR REPLACE PROCEDURE`, or `CREATE OR REPLACE TABLE
> FUNCTION`, you must re-authorize the routine.

### API

The following example shows how to update a query template with a `curl` command:

```bash
 curl -H "Authorization: Bearer $(gcloud auth print-access-token)" \
 -H "Content-Type: application/json" \
 -H 'x-goog-user-project:DCR_PROJECT_ID' \
 -X PATCH "https://analyticshub.googleapis.com/v1/projects/DCR_PROJECT_ID/locations/LOCATION/dataExchanges/CLEAN_ROOM_ID/queryTemplates/QUERY_TEMPLATE_ID?updateMask=description" \
 -d '{
   "query_template": {
     "description": "New query template"
   }
 }'
```

Replace the following:

- `DCR_PROJECT_ID`: the project ID of the project where the data clean room was created.
- `LOCATION`: the location of the data clean room.
- `CLEAN_ROOM_ID`: the data clean room ID.
- `QUERY_TEMPLATE_ID`: the query template ID.

> [!NOTE]
> **Note:** The query template display name can't be updated.

### Submit a query template for review

Select one of the following options:

### Console

1. Go to the **Sharing (Analytics Hub)** page.

   [Go to Sharing (Analytics Hub)](https://console.cloud.google.com/bigquery/analytics-hub)
2. Click the display name of the data clean room that contains the query
   template.

3. In the clean room, go to the **Templates** tab.

4. In the row for the template you want to submit for review, click
   **Actions \> Submit for review** . The template now has the status
   of **Requires review**.

> [!NOTE]
> **Note:** After a query template is submitted for review, you can no longer modify it.

> [!NOTE]
> **Note:** You must [authorize the TVF or routine](https://docs.cloud.google.com/bigquery/docs/authorized-routines#authorize_routines) for template subscribers to query the routine. If you modify a routine by running a `CREATE OR REPLACE` statement, such as `CREATE OR
> REPLACE FUNCTION`, `CREATE OR REPLACE PROCEDURE`, or `CREATE OR REPLACE TABLE
> FUNCTION`, you must re-authorize the routine.

### API

The following example shows how to submit a query template for review with a
`curl` command:

```bash
  curl -H "Authorization: Bearer $(gcloud auth print-access-token)" -H "Content-Type: application/json" -H 'x-goog-user-project:DCR_PROJECT_ID' -X POST https://analyticshub.googleapis.com/v1/projects/DCR_PROJECT_ID/locations/LOCATION/dataExchanges/CLEAN_ROOM_ID/queryTemplates/QUERY_TEMPLATE_ID:submit
```

Replace the following:

- `DCR_PROJECT_ID`: the project ID of the project where the data clean room was created.
- `LOCATION`: the location of the data clean room.
- `CLEAN_ROOM_ID`: the data clean room ID.
- `QUERY_TEMPLATE_ID`: the query template ID.

### Delete a query template

You can only delete a query template if it is in the **DRAFT** status. If the
query template has already been submitted for review, you can no longer delete
it.

Select one of the following options:

### Console

1. Go to the **Sharing (Analytics Hub)** page.

   [Go to Sharing (Analytics Hub)](https://console.cloud.google.com/bigquery/analytics-hub)
2. Click the display name of the data clean room that contains the query
   template.

3. In the clean room, go to the **Templates** tab.

4. In the row for the template you want to delete, click
   **Actions \> Delete template**.

### API

The following example shows how to delete a query template with a `curl`
command:

```bash
  curl -H "Authorization: Bearer $(gcloud auth print-access-token)" -H "Content-Type: application/json" -H 'x-goog-user-project:DCR_PROJECT_ID' -X DELETE https://analyticshub.googleapis.com/v1/projects/DCR_PROJECT_ID/locations/LOCATION/dataExchanges/CLEAN_ROOM_ID/queryTemplates?queryTemplateId=QUERY_TEMPLATE_ID
```

Replace the following:

- `DCR_PROJECT_ID`: the project ID of the project where the data clean room was created.
- `LOCATION`: the location of the data clean room.
- `CLEAN_ROOM_ID`: the data clean room ID.
- `QUERY_TEMPLATE_ID`: the query template ID.

## Template approver workflows

As a query template approver, you can
[approve a query template](https://docs.cloud.google.com/bigquery/docs/query-templates#approve-query-template).

> [!IMPORTANT]
> **Important:** When a query template is submitted for review, all the [data contributors](https://docs.cloud.google.com/bigquery/docs/data-clean-rooms#roles) of the data clean room can view the query template, but only data contributors who have access to the data referenced in the query can approve the template.

When a TVF references data that you don't own (for example, the other contributor's
data), the query template can only be approved by that data's owner. If you are
creating a TVF that only references your data (for sharing in one direction),
you can approve the query template yourself.

### Approve a query template

Select one of the following options:

### Console

1. Go to the **Sharing (Analytics Hub)** page.

   [Go to Sharing (Analytics Hub)](https://console.cloud.google.com/bigquery/analytics-hub)
2. Click the display name of the data clean room that contains the query
   template.

3. In the clean room, go to the **Templates** tab.

4. In the row for the template that requires your review, click
   **Approval Status \> Requires review**.

5. Click **Approve**.

6. Select the template location. This location is where the TVF is created for
   sharing.

7. Review the proposed query template.

8. Click **Approve** if the query template is approved for use within
   the clean room.

### API

1. Create the routine out of the query template using a `jobserver.query`
   call:

   ```bash
   curl -H "Authorization: Bearer $(gcloud auth print-access-token)" -H "Content-Type: application/json" -L -X POST https://bigquery.googleapis.com/bigquery/v2/projects/ROUTINE_PROJECT_ID/queries --data '{"query":"ROUTINE_CREATION_QUERY","useLegacySql":false}'
   ```

   Replace the following:
   - `ROUTINE_PROJECT_ID`: the project ID of the project where the routine was created.
   - `ROUTINE_CREATION_QUERY`: the query to create the routine.
2. Add the routine you created to the data clean room:

   ```bash
   curl -H "Authorization: Bearer $(gcloud auth print-access-token)" -H "Content-Type: application/json" -H 'x-goog-user-project:DCR_PROJECT_ID' -X POST https://analyticshub.googleapis.com/v1/projects/DCR_PROJECT_ID/locations/LOCATION/dataExchanges/CLEAN_ROOM_ID/listings?listingId=LISTING_ID -d
   '{"bigqueryDataset":{"dataset":"projects/PROJECT_ID/datasets/DATASET_ID","selectedResources":[{"routine":"projects/PROJECT_ID/datasets/DATASET_ID/tables/ROUTINE_ID"}],},"displayName":"LISTING_NAME"}'
   ```

   Replace the following:
   - `DCR_PROJECT_ID`: the project ID of the project where the data clean room was created.
   - `LOCATION`: the location of the data clean room.
   - `CLEAN_ROOM_ID`: the data clean room ID.
   - `LISTING_ID`: the listing ID.
   - `PROJECT_ID`: the project ID of the project where the source dataset was contained.
   - `DATASET_ID`: the source dataset ID.
   - `ROUTINE_ID`: the routine ID.
   - `LISTING_NAME`: the listing name.
3. Update the query template status to `APPROVED`:

   ```bash
   curl -H "Authorization: Bearer $(gcloud auth print-access-token)" -H "Content-Type: application/json" -L -X POST https://analyticshub.googleapis.com/v1/projects/DCR_PROJECT_ID/locations/LOCATION/dataExchanges/CLEAN_ROOM_ID/queryTemplates/QUERY_TEMPLATE_ID:approve  --data '{}'
   ```

   Replace the following:
   - `DCR_PROJECT_ID`: the project ID of the project where the data clean room was created.
   - `LOCATION`: the location of the data clean room.
   - `CLEAN_ROOM_ID`: the data clean room ID.
   - `QUERY_TEMPLATE_ID`: the query template ID.

### Reject a query template

In the Google Cloud console, you can reject a query template by not approving
the submitted query template under review.

## Template subscriber workflows

A query template subscriber can view and subscribe to a data clean room. If only
the query template is added to the clean room, subscribing to the clean room
only grants access to the corresponding TVF, not to the underlying shared data.

### Subscribe to a query template

Select one of the following options:

### Console

You subscribe to a query template by
[subscribing to the data clean room](https://docs.cloud.google.com/bigquery/docs/data-clean-rooms#subscribe_to_a_data_clean_room).
Access is granted to all the listings that have the
[**Exclude access to listing from linked dataset** setting disabled](https://docs.cloud.google.com/bigquery/docs/query-templates#add-listing-to-dcr).

To subscribe to a query template, follow these steps:

1. Go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. Navigate to the linked dataset that you created when you subscribed to the
   clean room.

3. Open the routine or TVF that's shared in the linked dataset.

4. Click **Invoke table function**.

5. Replace the parameter with the accepted input, which is the table name or
   a field.

6. Click **Run**.

If you can't view the TVF nested as a child element of the linked dataset
in the **Explorer** panel, you can query the TVF directly on the linked
dataset:

    SELECT * FROM `myproject.dcr_linked_dataset.mytvf`(TABLE myTable);

### API

Use the
[`projects.locations.dataExchanges.subscribe` method](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges/subscribe).

The following example shows how to call the `projects.locations.dataExchanges.subscribe` method using the `curl` command:

```bash
  curl -H "Authorization: Bearer $(gcloud auth print-access-token)" -H "Content-Type: application/json" -L -X POST https://analyticshub.googleapis.com/v1/projects/DCR_PROJECT_ID/locations/LOCATION/dataExchanges/CLEAN_ROOM_ID:subscribe  --data '{"destination":"projects/SUBSCRIBER_PROJECT_ID/locations/LOCATION","subscription":"SUBSCRIPTION"}'
```

Replace the following:

- `DCR_PROJECT_ID`: the project ID of the project where the data clean room was created.
- `LOCATION`: the location of the data clean room.
- `CLEAN_ROOM_ID`: the data clean room ID.
- `SUBSCRIBER_PROJECT_ID`: the project ID of the template subscriber project.
- `SUBSCRIPTION`: the name of your subscription.

After you've subscribed to the query template, you can query the TVF
directly on the linked dataset:

    SELECT * FROM `myproject.dcr_linked_dataset.mytvf`(TABLE myTable);

## Example scenarios

Query templates can be used to facilitate different forms of data collaboration
within a data clean room. The following sections describe example scenarios.

### Single-direction sharing scenario

A data publisher creates a query template to verify that subscribing partners can
only run the queries that are defined by the publisher. Query template creators
ultimately self-approve the query templates, because no other contributor
is added to the clean room.

In this scenario, user A is a data clean room owner who creates a data clean
room called `campaign_analysis` and adds a dataset named `my_campaign` with a
`campaigns` table. User A configures an aggregation threshold policy and
metadata controls to verify that only the metadata schema is visible and
template subscribers cannot access the source data. User A then creates a query
template by defining a table-valued function from the `campaigns` table,
restricting subscribers of the linked dataset to execute only the TVF.

This is the TVF syntax:

    campaigns_template(t1 TABLE campaign_ID <STRING> ) AS (
    SELECT WITH AGGREGATION_THRESHOLD company_id, company, sum(impressions) FROM myproject.my_campaign.campaigns
    group by company_id, company
    );

Since user A has the appropriate permissions for the campaigns table with the
BigQuery Data Owner role, user A can immediately self-approve the query template
after submitting it for review.

### Multiple party collaboration sharing

A clean room owner invites a trusted contributor to propose queries to be run
against each other's data. Both parties can safely propose queries by viewing
metadata schemas only, without accessing the underlying shared data. When a
query definition references data that does not belong to the template proposer,
the template can only be approved by that data's owner.

In this scenario, user A invites user B, who is a data clean room contributor,
to the `campaign_analysis` clean room. User B wants to propose a query template
to join their own data to the `campaigns` table by viewing the table's metadata
schema.

This is the TVF syntax:

    campaigns_template(t1 TABLE campaign_ID <STRING> ) AS (
    SELECT WITH AGGREGATION_THRESHOLD company_id, company, sum(impressions) FROM my_project.my_campaign.campaigns
    group by company_id, company
    );

Because user B didn't add or doesn't own the `campaigns` table, only user A can
approve the query template after it's submitted for approval. To use the query
template, user B must subscribe to the clean room and invoke the TVF. User B
passes their own table with a field named `campaign_ID` as the table parameter,
and they can execute the private SQL defined in the query template. User B
doesn't need to add their data to the clean room.

User B also adds a dataset named `my_transactions` to the clean room that has a
`transactions` table and a `products` table. User B configures aggregation
threshold policies and metadata controls to verify that only the metadata
schema is visible and template subscribers cannot access the source data.

User A can now propose various query templates to join their own data to the
transactions table by viewing the table's metadata schema. The following are
examples of TVF syntax:

    transactions_template(t1 TABLE user_ID  <STRING> ) AS (
    SELECT WITH AGGREGATION_THRESHOLD company_id, company, campaign_id, sku, category, date, sum(amount) FROM my_project.my_transactions.transactions
    group by company_id, company, campaign_id, sku, category, date
    );

    transactions_template_with_join(t1 TABLE user_ID  <STRING> ) AS (
    SELECT WITH AGGREGATION_THRESHOLD t.company_id, t.company, t.campaign_id, t.sku, t.date, p.product_name, p.product_category, sum(t.amount) FROM myproject.my_transactions.transactions t
    left join my_project.my_transactions.products p
    on t.product_id = p.product_id
    group by t.company_id, t.company, t.campaign_id, t.sku, t.date, p.product_name, p.product_category
    );

> [!NOTE]
> **Note:** Only tables owned by the same party can be referenced within the TVF query syntax. For more details, see [Limitations](https://docs.cloud.google.com/bigquery/docs/query-templates#limitations).

Because user A didn't add or doesn't own the `transactions` and `products`
tables, only user B can approve the query template after it's submitted for
approval. To use the query template, user A must subscribe to the clean room
and invoke the TVF. User A passes their own table with a field named `user_ID`
as the table parameter, and they can execute the privacy SQL defined in the
query template. User A doesn't need to add their data to the clean room.

## Pricing

Data contributors using query templates are only charged for
[data storage](https://cloud.google.com/bigquery/pricing#storage).

Template subscribers using query templates are only charged for
[compute (analysis)](https://cloud.google.com/bigquery/pricing#overview_of_pricing)
when they run queries.

## What's next

- To learn more about data clean rooms, see [Share sensitive data with data clean rooms](https://docs.cloud.google.com/bigquery/docs/data-clean-rooms).
- To learn more about subscriptions, see [Subscribe to a data clean room](https://docs.cloud.google.com/bigquery/docs/data-clean-rooms#subscribe_to_a_data_clean_room).
- To learn more about TVFs, see [Table functions](https://docs.cloud.google.com/bigquery/docs/table-functions).
- To learn more about data egress, see [Data egress options (BigQuery shared datasets only)](https://docs.cloud.google.com/bigquery/docs/analytics-hub-introduction#data_egress).