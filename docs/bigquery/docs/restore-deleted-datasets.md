# Restore deleted datasets

This document describes how to restore (or *undelete*) a deleted dataset in
BigQuery.

You can restore a dataset to recover it to the state that it was in when it was
deleted. You can only restore datasets that are within your
[time travel window](https://docs.cloud.google.com/bigquery/docs/time-travel#time_travel).
This recovery includes all of the objects that were contained in the dataset,
the dataset properties, and the security settings. For resources that are not
recovered, see
[Limitations](https://docs.cloud.google.com/bigquery/docs/restore-deleted-datasets#limitations).

For information about restoring a deleted table or snapshot, see the following
resources:

- [Restore deleted tables](https://docs.cloud.google.com/bigquery/docs/restore-deleted-tables)
- [Restore table snapshots](https://docs.cloud.google.com/bigquery/docs/table-snapshots-restore)

## Limitations

The following is a list of limitations related to restoring a dataset:

- Restored datasets might reference security principals that no longer exist.
- References to a deleted dataset in linked datasets aren't restored when you perform this action. Subscribers must subscribe again to manually restore the links.
- Business tags aren't restored when you perform this action.
- You must [manually refresh materialized views](https://docs.cloud.google.com/bigquery/docs/materialized-views-manage#manual-refresh) and reauthorize [authorized views](https://docs.cloud.google.com/bigquery/docs/authorized-views#manage_users_or_groups_for_authorized_views), [authorized datasets](https://docs.cloud.google.com/bigquery/docs/authorized-datasets#authorize_a_dataset), and [authorized routines](https://docs.cloud.google.com/bigquery/docs/authorized-routines#authorize_routines).
- You can't restore a logical view directly. However, you can undelete the dataset or recreate the view to restore your logical view. For more information on these workarounds, see [Restore a view](https://docs.cloud.google.com/bigquery/docs/managing-views#restore_a_view).
- A [BigQuery CDC-enabled table](https://docs.cloud.google.com/bigquery/docs/change-data-capture) doesn't resume background apply jobs when restored as part of an undeleted dataset.
- It can take up to 24 hours for a restored dataset to appear in
  BigQuery search results.

  When authorized resources (views, datasets, and routines) are
  deleted, it takes up to 24 hours for the authorization to delete. So, if
  you restore a dataset with an authorized resource less than 24 hours after
  deletion, it's possible that reauthorization isn't necessary. As a best
  practice, always verify authorization after restoring resources.
- Once a dataset is undeleted it cannot be deleted within the next seven days.
  The entities of the datasets, such as tables and routines, can be deleted.
  If you require a shorter period, contact
  [Google Cloud Support](https://cloud.google.com/support-hub).

## Before you begin

Ensure that you have the necessary Identity and Access Management (IAM) permissions to
restore a deleted dataset.

### Required roles


To get the permissions that
you need to restore a deleted dataset,

ask your administrator to grant you the
[BigQuery User](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquery#bigquery.user) (`roles/bigquery.user`) IAM role on the project.


For more information about granting roles, see [Manage access to projects, folders, and organizations](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).


This predefined role contains

the permissions required to restore a deleted dataset. To see the exact permissions that are
required, expand the **Required permissions** section:


#### Required permissions

The following permissions are required to restore a deleted dataset:

- `bigquery.datasets.create` on the project
- `bigquery.datasets.get` on the dataset


You might also be able to get
these permissions
with [custom roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles) or
other [predefined roles](https://docs.cloud.google.com/iam/docs/roles-overview#predefined).

## Restore a dataset

> [!CAUTION]
> **Caution:** Only the most recent dataset for a given dataset ID can be restored. If you delete a dataset and then create a new dataset with the same ID, you lose the ability to undelete the original dataset. However, you still might be able to [recover specific tables from the deleted dataset](https://docs.cloud.google.com/bigquery/docs/managing-datasets#restore-delete-tables).

To restore a dataset, select one of the following options:

### SQL

Use the
[`UNDROP SCHEMA` data definition language (DDL) statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#undrop_schema_statement):

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, enter the following statement:

       UNDROP SCHEMA DATASET_ID;

   Replace `DATASET_ID` with the dataset that you
   want to undelete.
3. [Specify the location](https://docs.cloud.google.com/bigquery/docs/locations#specify_locations)
   of the dataset that you want to undelete. To specify the location part
   of the SQL statement use `location` options

       UNDROP SCHEMA DATASET_ID OPTIONS (location=location);

   <br />

4. Click **Run**.

For more information about how to run queries, see [Run an interactive query](https://docs.cloud.google.com/bigquery/docs/running-queries#queries).

### API

Call the
[`datasets.undelete` method](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets/undelete).

> [!NOTE]
> **Note:** If you have two deleted datasets in your project with the same name in two different regions, undeleting a dataset with the BigQuery API undeletes only one, selected at random, unless a location is specified.

When you restore a dataset, the following errors might occur:

- `ALREADY_EXISTS`: a dataset with the same name already exists in the region in which you tried to restore. You can't use undelete to overwrite or merge datasets.
- `NOT_FOUND`: the dataset you're trying to recover is past its time travel window, it never existed, or you didn't [specify the correct location](https://docs.cloud.google.com/bigquery/docs/locations#specify_locations) of the dataset.
- `ACCESS_DENIED`: you don't have the required
  [permissions](https://docs.cloud.google.com/bigquery/docs/restore-deleted-datasets#before-you-begin)
  to undelete this dataset.

  ## What's next

- For information about querying data at a point in time, see [Access historical data](https://docs.cloud.google.com/bigquery/docs/access-historical-data).

- For information about data retention, see [Data retention with time travel and fail-safe](https://docs.cloud.google.com/bigquery/docs/time-travel).

- For information about how to delete a dataset, see [Manage datasets](https://docs.cloud.google.com/bigquery/docs/managing-datasets).