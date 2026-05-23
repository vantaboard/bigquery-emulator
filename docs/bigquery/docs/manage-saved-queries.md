# Manage saved queries

This document describes how to manage
[saved queries and classic saved queries](https://docs.cloud.google.com/bigquery/docs/saved-queries-introduction)
and how to manage saved query metadata in
[Knowledge Catalog](https://docs.cloud.google.com/dataplex/docs/introduction).

Saved queries are
[BigQuery Studio](https://docs.cloud.google.com/bigquery/docs/query-overview#bigquery-studio)
code assets powered by [Dataform](https://docs.cloud.google.com/dataform/docs/overview).

## Before you begin

You can optionally set IAM permissions on migrated public or
project classic saved queries during [migration](https://docs.cloud.google.com/bigquery/docs/manage-saved-queries#migrate_classic_saved_queries),
[create](https://docs.cloud.google.com/bigquery/docs/work-with-saved-queries#create_saved_queries)
or select a BigQuery Studio saved query and
[grant selected Identity and Access Management (IAM) permissions](https://docs.cloud.google.com/bigquery/docs/manage-saved-queries#share-saved-query)
to that saved query.

During migration of public or project classic saved queries to
BigQuery Studio saved queries, select a BigQuery Studio
saved query to copy the permissions granted on it to the migrated
saved queries.

### Required roles


To get the permissions that
you need to manage saved queries,

ask your administrator to grant you the
following IAM roles on the project that you want to manage saved queries for:

- To manage BigQuery Studio saved queries in the Google Cloud console:
  - [BigQuery Job User](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquery#bigquery.jobUser) (`roles/bigquery.jobUser`)
  - [BigQuery Read Session User](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquery#bigquery.readSessionUser) (`roles/bigquery.readSessionUser`)
  - [Code Owner](https://docs.cloud.google.com/iam/docs/roles-permissions/dataform#dataform.codeOwner) (`roles/dataform.codeOwner`)
- To manage BigQuery Studio saved queries by using the BigQuery API: [Code Owner](https://docs.cloud.google.com/iam/docs/roles-permissions/dataform#dataform.codeOwner) (`roles/dataform.codeOwner`)
- To migrate project classic saved queries to BigQuery Studio saved queries: [BigQuery Studio Admin](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquery#bigquery.studioAdmin) (`roles/bigquery.studioAdmin`)
- To let [authenticated users](https://docs.cloud.google.com/iam/docs/principals-overview#all-authenticated-users) view [public access queries](https://docs.cloud.google.com/bigquery/docs/manage-saved-queries#grant-public-access): [Code Viewer](https://docs.cloud.google.com/iam/docs/roles-permissions/dataform#dataform.codeViewer) (`roles/dataform.codeViewer`)


For more information about granting roles, see [Manage access to projects, folders, and organizations](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).


These predefined roles contain

the permissions required to manage saved queries. To see the exact permissions that are
required, expand the **Required permissions** section:


#### Required permissions

The following permissions are required to manage saved queries:

- To manage BigQuery Studio saved queries in the Google Cloud console: `bigquery.config.get, bigquery.jobs.create, dataform.locations., resourcemanager.projects.get, resourcemanager.projects.list, bigquery.readsessions., dataform.repositories., dataform.workspaces.`
- To manage BigQuery Studio saved queries by using the BigQuery API: `dataform.locations., dataform.repositories., dataform.workspaces.*, resourcemanager.projects.get, resourcemanager.projects.list`
- To migrate project classic saved queries to BigQuery Studio saved queries: `bigquery.savedqueries.get, bigquery.savedqueries.list, bigquery.savedqueries.update, bigquery.savedqueries.delete, bigquery.savedqueries.create`
- To let [authenticated users](https://docs.cloud.google.com/iam/docs/principals-overview#all-authenticated-users) view [public access queries](https://docs.cloud.google.com/bigquery/docs/manage-saved-queries#grant-public-access): `dataform.locations.*, dataform.repositories.computeAccessTokenStatus, dataform.repositories.fetchHistory, dataform.repositories.fetchRemoteBranches, dataform.repositories.get, dataform.repositories.getIamPolicy, dataform.repositories.list, dataform.repositories.queryDirectoryContents, dataform.repositories.readFile, dataform.workspaces.fetchFileDiff, dataform.workspaces.fetchFileGitStatuses, dataform.workspaces.fetchGitAheadBehind. dataform.workspaces.get, dataform.workspaces.getIamPolicy, dataform.workspaces.list, dataform.workspaces.queryDirectoryContents, dataform.workspaces.readFile, dataform.workspaces.searchFiles, resourcemanager.projects.get, resourcemanager.projects.list`


You might also be able to get
these permissions
with [custom roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles) or
other [predefined roles](https://docs.cloud.google.com/iam/docs/roles-overview#predefined).

<br />

For more information about BigQuery IAM, see
[Access control with IAM](https://docs.cloud.google.com/bigquery/docs/access-control).

To manage saved query metadata in Knowledge Catalog,
ensure that you have the required
[Knowledge Catalog roles](https://docs.cloud.google.com/dataplex/docs/iam-roles) and the
[`dataform.repositories.get`](https://docs.cloud.google.com/dataform/docs/access-control#predefined-roles) permission.

## Share saved queries

To share a saved query with a user, you first grant that user access to the
saved query and add them to an appropriate IAM role. Then you
generate a link to the saved query and share that link with the user.

Users that you share a query with only see the most recent version of a query.
Autosaved changes that you haven't explicitly saved don't appear in the shared
query.

1. In the Google Cloud console, go to the
   **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)

   If you don't see the left pane, click **Expand left pane** to open the pane.
3. In the **Explorer** pane, expand your project and click **Queries**.

4. Find and click the saved query that you want to grant access to. You can use
   the search feature or filters to find your query.

5. Click
   **Share** ,
   and then click **Manage permissions**.

6. In the **Manage permissions** pane, click **Add user/group**.

7. In the **New principals** field, enter a principal.

8. In the **Role** list, select one of
   the following roles:

   - [**Code Owner**](https://docs.cloud.google.com/dataform/docs/access-control#dataform.codeOwner): can perform any action on the saved query, including deleting or sharing it.
   - [**Code Editor**](https://docs.cloud.google.com/dataform/docs/access-control#dataform.codeEditor): can edit the query.
   - [**Code Viewer**](https://docs.cloud.google.com/dataform/docs/access-control#dataform.codeViewer): can view the query.

   > [!NOTE]
   > **Note:** The principal must also have the [BigQuery User (`roles/bigquery.user`)](https://docs.cloud.google.com/bigquery/docs/access-control#bigquery.user) role to run the saved query.

9. Optional: To view a complete list of roles and advanced sharing settings,
   click **Advanced sharing**.

10. Click **Save**.

11. To return to the saved query info, click **Close**.

12. To generate a link to the saved query, click
    **Share** ,
    and then click **Get link**.

    The link is copied to your clipboard.

## Grant public access to a saved query

You can grant public access to a BigQuery Studio saved query by granting
the Code Viewer ([roles/dataform.codeViewer](https://docs.cloud.google.com/dataform/docs/access-control#dataform.codeViewer))
role on the saved query to the `allAuthenticatedUsers` principal.

When you assign an IAM role to the `allAuthenticatedUsers`
principal,
service accounts and all users on the internet who have authenticated with a
Google Account are granted that role. This includes accounts that aren't
connected to a Google Workspace account or Cloud Identity domain,
such as personal Gmail accounts. Users who aren't authenticated,
such as anonymous visitors, aren't included. For more information, see
[All authenticated users](https://docs.cloud.google.com/iam/docs/principals-overview#all-authenticated-users).

For example, when you grant the Code Viewer role to `allAuthenticatedUsers`
on the `sales` saved query, all service accounts
and users on the internet who have authenticated with a Google Account have
read-only access to the `sales` saved query.

> [!CAUTION]
> **Caution:** Granting administrator, edit, or execution level permissions to `allAuthenticatedUsers` can allow bad actors to access your data. Grant only the minimal required permissions.

To grant public access to a BigQuery Studio saved query, do the following:

1. In the Google Cloud console, go to the
   **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)

   If you don't see the left pane, click **Expand left pane** to open the pane.
3. In the **Explorer** pane, expand your project and click **Queries**.

4. Find and click the saved query that you want to grant public access to. You
   can use the search feature or filters to find your query.

5. Click
   **View actions** next to the saved query, and then click
   **Share \> Manage Permissions**.

6. In the **Manage permissions** pane, click **Add user/group**.

7. In the **New principals** field, enter `allAuthenticatedUsers`.

8. In the **Role** list, select the
   [**Code Viewer**](https://docs.cloud.google.com/dataform/docs/access-control#dataform.codeViewer) role.

9. Click **Save**.

10. To return to the saved query info, click **Close**.

## Prevent public access to saved queries

To ensure no public access is granted to any BigQuery Studio saved query,
restrict the `allAuthenticatedUsers` principal in your project.

To restrict `allAuthenticatedUsers` in your project, you can
[set the `iam.allowedPolicyMemberDomains` policy](https://docs.cloud.google.com/resource-manager/docs/organization-policy/restricting-domains#setting_the_organization_policy),
and remove `allAuthenticatedUsers` from the list of `allowed_values`.

When you restrict `allAuthenticatedUsers` in the `iam.allowedPolicyMemberDomains`
policy, the `allAuthenticatedUsers` principal cannot be used in any
IAM policy in your project, which prevents granting public access
to all resources, including BigQuery Studio saved queries.

For more information about the `iam.allowedPolicyMemberDomains` policy
and instructions to set it, see
[Restricting identities by domain](https://docs.cloud.google.com/resource-manager/docs/organization-policy/restricting-domains).

## Set the default region for code assets

All new code assets in your Google Cloud project use a default region. After the
asset is created, you can't change its region.

> [!IMPORTANT]
> **Important:** If you change the region while creating a code asset, that region becomes the default for all subsequent code assets. Existing code assets are not affected.

To set the default region for new code assets, do the following:

1. Go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Files**
   to open the file browser:

   ![Click **Files** to open the file browser.](https://docs.cloud.google.com/static/bigquery/images/select-file-browser.png)
3. Next to the project name, click

   **View files panel actions** \> **Switch code region**.

4. Select the code region that you want to use as a default.

5. Click **Save**.

For a list of supported regions, see
[BigQuery Studio locations](https://docs.cloud.google.com/bigquery/docs/locations#bqstudio-loc).

## View all saved queries

To view a list of all saved queries in your project, do the following:

1. In the Google Cloud console, go to the
   **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)
3. In the **Explorer** pane,
   click
   **View actions** next to **Queries**, and then do one of the following:

- To open the list in the current tab, click **Show all**.
- To open the list in a new tab, click **Show all in** \> **New tab**.
- To open the list in a split tab, click **Show all in** \> **Split tab**.

## View saved query metadata

To view saved query metadata, do the following:

1. In the Google Cloud console, go to the
   **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)
3. In the **Explorer** pane, expand your project and click **Queries**.

4. Find and click the saved query that you want to view metadata for.

5. Click info **Details**
   to see information about the saved query such as
   the [region](https://docs.cloud.google.com/bigquery/docs/saved-queries-introduction#supported_regions)
   it uses and the date it was last modified.

## Work with saved query versions

You can choose to create a saved query either inside of or outside of
a [repository](https://docs.cloud.google.com/bigquery/docs/repository-intro). Saved query versioning
is handled differently based on where the saved query is located.

### Saved query versioning in repositories

Repositories are Git repositories that reside either in BigQuery
or with a third-party provider. You can use
[workspaces](https://docs.cloud.google.com/bigquery/docs/workspaces-intro) in repositories to perform
version control on saved queries. For more information, see
[Use version control with a file](https://docs.cloud.google.com/bigquery/docs/workspaces#use_version_control_with_a_file).

### Saved query versioning outside of repositories

Use the following sections to learn how to view, compare, and restore versions
of a saved query.

#### View saved query versions

To view saved query versions, do the following:

1. In the Google Cloud console, go to the
   **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)
3. In the **Explorer** pane, expand your project and click **Queries**.

4. Click the name of the saved query you want to view version history for.

5. Click **Version history**
   to see a list of the saved query versions in descending order by date.

#### Compare saved query versions

To compare saved query versions, do the following:

1. In the Google Cloud console, go to the
   **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)
3. In the **Explorer** pane, expand your project and click **Queries**.

4. Click the name of the saved query you want to compare version information for.

5. Click **Version history**.

6. Click
   **View actions** next to a saved query version and then click **Compare**.
   The comparison pane opens, comparing the saved query version that you
   selected with the current query.

7. Optional: The current query also shows autosaved changes. To
   explicitly save these changes, click **Overwrite**.

8. Optional: To compare the versions inline instead of in separate panes,
   click **Compare** and then click **Inline**.

#### Restore a saved query version

Restoring from the comparison pane lets you compare the previous version of
the saved query to the current version before choosing whether to restore it.

1. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)
2. In the **Explorer** pane, expand your project and click **Queries**.

3. Click the name of the saved query you want to restore a previous version of.

4. Click **Version history**.

5. Click
   **View actions** next to the version of the saved query that you want to
   restore and then click
   **Compare**. The comparison pane opens, comparing the saved query version
   you selected with the most recent query version, including any autosaved
   changes.

6. If you want to restore the previous saved query version after
   comparison, click **Restore**.

7. Click **Confirm**.

## Open saved queries in Connected Sheets

To open a saved query in Connected Sheets, follow these steps:

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)
3. In the **Explorer** pane, expand your project and click **Queries**.
   Find the saved query that you want to open in Connected Sheets.

4. Click **Open actions**
   next to the saved query, and then click
   **Open in \> Connected Sheets**.

   Alternatively, click the name of the saved query to open it in the details
   pane, and then click **Open in \> Connected Sheets**.

## Download saved queries

To download a saved query, do the following:

1. In the Google Cloud console, go to the
   **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)
3. In the **Explorer** pane, expand your project and click **Queries**.

4. Click the name of a saved query to open it.

5. Click **Download**.

## Delete saved queries

To delete a saved query, do the following:

1. In the Google Cloud console, go to the
   **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)
3. In the **Explorer** pane, expand your project and click **Queries**.

4. Find the saved query you want to delete.

5. Click
   **Open actions** next to the saved query and then click **Delete**.

6. To confirm deletion, type `delete` in the dialog.

7. Click **Delete**.

## Classic saved queries

> [!WARNING]
> **Deprecated:** Saved queries, available in [BigQuery Studio](https://docs.cloud.google.com/bigquery/docs/enable-assets), will fully replace classic saved queries in the future. The deprecation timeline is being reviewed. For more information, see [Deprecation of classic saved queries](https://docs.cloud.google.com/bigquery/docs/saved-queries-introduction#classic-saved-queries-deprecation). To learn how to migrate to saved queries, see [Migrate classic saved queries](https://docs.cloud.google.com/bigquery/docs/manage-saved-queries#migrate_classic_saved_queries).

Use the following sections to learn how to manage
[classic saved queries](https://docs.cloud.google.com/bigquery/docs/saved-queries-introduction#classic_saved_queries).

> [!NOTE]
> **Note:** If you have not enabled BigQuery Studio, then classic saved queries appear in the **Saved queries (<var translate="no">NUMBER</var>)** folder in the **Classic Explorer** pane instead of the **(Classic) Queries** folder.

### Share classic saved queries

You can share classic saved queries that you have given project or public
visibility. Project visibility allows principals with the
[required permissions](https://docs.cloud.google.com/bigquery/docs/work-with-saved-queries#required_permissions_for_classic_saved_queries)
to view, update, or delete the query. Public visibility allows anyone with
the query link to view but not update or delete the query.

You share a classic saved query with other users by generating and sharing
a link to the classic saved query.

To run a classic shared query, users must have access to the data that the query
accesses. For more information, see
[Grant access to a dataset](https://docs.cloud.google.com/bigquery/docs/control-access-to-resources-iam#grant_access_to_a_dataset).

If you are plan to share a classic saved query, consider including a
comment in the query that describes its purpose.

1. In the left pane, click category **Classic Explorer**:

   ![Highlighted button for the Classic Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/classic-explorer-tab.png)

   If you don't see the left pane, click **Expand left pane** to open the pane.
2. In the **Classic Explorer** pane, expand your project, click **(Classic) Queries**, and then find
   the classic saved query you want to share.

3. Click
   **View actions** next to the query and then click **Get link**.

4. Share the link with the users you want to grant access to the query.

### Save a classic query as a saved query

1. In the Google Cloud console, go to the
   **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click category **Classic Explorer**:

   ![Highlighted button for the Classic Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/classic-explorer-tab.png)
3. In the **Classic Explorer** pane, expand your project and the **(Classic) Queries** folder, and if necessary, the **Project queries** folder.

4. Click the name of a classic saved query to open it.

5. Click ![](https://docs.cloud.google.com/static/bigquery/images/save-bigquery-console.png) **Save Query (Classic) \> Save query as...**.

6. In the **Save query** dialog, type a name and choose the location
   for the query.

7. Click **Save**.

### Migrate classic saved queries

To batch migrate classic saved queries, you must be granted the
[required roles](https://docs.cloud.google.com/bigquery/docs/manage-saved-queries#required_roles).
The permissions granted by these roles determine the type of classic saved
queries you can migrate.

You can batch migrate the following classic saved queries:

Personal classic saved queries
:   Personal classic saved queries are visible only to the user who
    creates them. They are identified by the
    icon. Personal classic saved
    queries can only be migrated by their owners.

Public classic saved queries

:   Public classic saved queries are visible to anyone with a link to
    the query. They are identified by the
    icon. Public classic saved queries
    can only be migrated by their owners.

    IAM permissions on public classic saved queries don't map to
    permissions on BigQuery Studio saved queries. This means that BigQuery Studio
    saved queries migrated from public classic saved queries
    are not publicly available by default. You need to set IAM
    permissions for migrated BigQuery Studio saved queries, either during
    or after migration.

    To set IAM permissions for the migrated BigQuery Studio
    saved queries during migration, you can select an existing BigQuery Studio
    saved query that has permissions which you want to apply to the migrated
    saved queries. BigQuery will copy permissions granted on the
    selected BigQuery Studio saved query, and apply them to the
    migrated saved queries. You can also manually add users or groups with
    whom you want to share the migrated saved queries.

    If you don't set IAM permissions during migration, only you
    will have access to the migrated BigQuery Studio saved queries.

Project classic saved queries

:   Project-level saved queries are visible to
    principals that have the
    [required permissions](https://docs.cloud.google.com/bigquery/docs/work-with-saved-queries#required_permissions_for_classic_saved_queries).
    They are identified by the icon.
    You can batch-migrate all project classic saved queries in your project.

    IAM permissions on project classic saved queries don't directly
    map to permissions on BigQuery Studio saved queries. You need to set
    IAM permissions for migrated BigQuery Studio saved queries,
    either during, or after migration.

    To set IAM permissions for the migrated BigQuery Studio
    saved queries during migration, you can select an existing BigQuery Studio
    saved query that has permissions which you want to apply to the migrated
    saved queries. BigQuery will copy permissions granted on the
    selected BigQuery Studio saved query, and apply them to the
    migrated saved queries. You can also manually add users or groups with
    whom you want to share the migrated saved queries.

    If you don't set IAM permissions during migration, only you will
    have access to the migrated BigQuery Studio saved queries.

During batch migration of classic saved queries,
BigQuery does the following:

- Saves all of the migrating classic saved queries as BigQuery Studio saved queries, stored in the selected region.
- Converts all of the migrating classic saved queries to read-only classic saved queries.

After migration, you can access your personal, public, and project classic saved
queries both as BigQuery Studio saved queries and as read-only classic saved
queries.

#### Migration risks

After batch migration, you won't be able to modify migrated classic saved
queries. Your migrated personal, public, and project
classic saved queries become read-only.

BigQuery will add migrated BigQuery Studio saved queries
to your Google Cloud project using the Dataform API. Reverting these changes
requires manual cleanup.

> [!WARNING]
> **Warning:** Migration cannot be stopped or canceled once started.

#### Batch migrate classic saved queries

To batch migrate classic saved queries in your project to BigQuery Studio
saved queries, do the following:

1. In the Google Cloud console, go to the
   **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click category **Classic Explorer**:

   ![Highlighted button for the Classic Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/classic-explorer-tab.png)
3. In the **Classic Explorer** pane, expand your project and click

   **View actions** next to **(Classic) Queries** ,
   and then click **Migrate classic saved queries**.

4. In the **Classic saved queries migration** pane, in the
   **Check migration readiness** section,
   click **Next** to confirm that you have the
   [required roles](https://docs.cloud.google.com/bigquery/docs/manage-saved-queries#required_roles).

   Your IAM permissions determine which type of classic saved
   queries you can migrate and which sections of the
   **Classic saved queries migration** pane are visible to you.
5. In the **Region** section, in the **Region** drop-down, select a region
   where BigQuery will store the migrated saved queries.

   We recommend selecting your default region for BigQuery Studio code assets.
   For more information, see
   [Set the default region](https://docs.cloud.google.com/bigquery/docs/manage-saved-queries#set-default-region).
6. To migrate all your personal classic saved queries, In the
   **Migrate personal queries** section, select the
   **Migrate all personal queries** checkbox, and then click **Next**.

7. To migrate all public classic saved queries in your project,
   in the **Migrate public queries** section, do the following:

   1. Select the **Migrate all public queries** checkbox.
   2. In the **SQL** drop-down, select a BigQuery Studio saved query that has the IAM policies which you want apply to the migrated saved queries.
   3. Optional: To add a user or group with whom you want to share the
      migrated saved queries, click **Add User/Group**.

      To share the migrated saved queries publicly, set `allAuthenticatedUsers`
      as the principal, and grant it the Code Viewer role.
      For more information, see
      [Grant public access](https://docs.cloud.google.com/bigquery/docs/manage-saved-queries#grant-public-access).
   4. Click **Next**.

8. To migrate project-level classic saved queries,
   in the **Migrate project queries** section, do the following:

   1. Select the **Migrate all project queries** checkbox.
   2. In the **SQL** drop-down, select a BigQuery Studio saved query that has the IAM policies which you want apply to the migrated saved queries.
   3. Optional: To add a user or group with whom you want to share the migrated saved queries, click **Add User/Group**.
   4. Click **Next**.
9. To confirm that you understand the [migration risks](https://docs.cloud.google.com/bigquery/docs/manage-saved-queries#migration-risks)
   and that you want to batch migrate classic saved queries,
   in the **Confirm** section, in the **Confirm** field, enter `confirm`,
   and then click **Next**.

   > [!WARNING]
   > **Warning:** Migration cannot be stopped or canceled once started.

10. Click **Submit**.

Migration can take over 15 minutes, depending on the number of migrating queries.

### Delete classic saved queries

1. In the Google Cloud console, go to the
   **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click category **Classic Explorer**:

   ![Highlighted button for the Classic Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/classic-explorer-tab.png)
3. In the **Classic Explorer** pane, expand your project and the **(Classic) Queries** folder, and
   if necessary, the **Project queries** folder.

4. Find the classic saved query you want to delete.

5. Click **View actions** next to the query and then click
   **Delete**.

6. To confirm deletion, type `delete` in the dialog.

7. Click **Delete**.

## Manage metadata in Knowledge Catalog

Knowledge Catalog lets you store and manage metadata for
saved queries. Saved queries are available in Knowledge Catalog
by default, without additional configuration.

You can use Knowledge Catalog to manage saved queries
in all [saved query locations](https://docs.cloud.google.com/bigquery/docs/locations).
Managing saved queries in Knowledge Catalog
is subject to [Knowledge Catalog quotas and limits](https://docs.cloud.google.com/dataplex/docs/quotas)
and [Knowledge Catalog pricing](https://cloud.google.com/dataplex/pricing).

Knowledge Catalog automatically retrieves
the following metadata from saved queries:

- Data asset name
- Data asset parent
- Data asset location
- Data asset type
- Corresponding Google Cloud project

Knowledge Catalog logs saved queries as
[entries](https://docs.cloud.google.com/dataplex/docs/ingest-custom-sources#entries) with the following
entry values:

System entry group
:   The [system entry group](https://docs.cloud.google.com/dataplex/docs/ingest-custom-sources#entry-groups)
    for saved queries is `@dataform`. To view details of saved query entries
    in Knowledge Catalog, you need to view the `dataform` system entry group.
    For instructions about how to view a list of all entries in an entry group, see
    [View details of an entry group](https://docs.cloud.google.com/dataplex/docs/ingest-custom-sources#entry-group-details)
    in the Knowledge Catalog documentation.

System entry type
:   The [system entry type](https://docs.cloud.google.com/dataplex/docs/ingest-custom-sources#entry-types)
    for saved queries is `dataform-code-asset`. To view details of
    saved queries,you need to view the `dataform-code-asset` system entry type,
    filter the results with an aspect-based filter,
    and [set the `type` field inside `dataform-code-asset` aspect to `SQL_QUERY`](https://docs.cloud.google.com/dataplex/docs/search-syntax#aspect-search).
    Then, select an entry of the selected saved query.
    For instructions about how to view details of a selected entry type, see
    [View details of an entry type](https://docs.cloud.google.com/dataplex/docs/ingest-custom-sources#entry-type-details)
    in the Knowledge Catalog documentation.
    For instructions about how to view details of a selected entry, see
    [View details of an entry](https://docs.cloud.google.com/dataplex/docs/search-assets#view-entry-details)
    in the Knowledge Catalog documentation.

System aspect type
:   The [system aspect type](https://docs.cloud.google.com/dataplex/docs/enrich-entries-metadata#aspect-types)
    for saved queries is `dataform-code-asset`. To
    provide additional context to saved queries in Knowledge Catalog
    by annotating data saved query entries with
    [aspects](https://docs.cloud.google.com/dataplex/docs/enrich-entries-metadata#aspects),
    view the `dataform-code-asset` aspect type,
    filter the results with an aspect-based filter,
    and [set the `type` field inside `dataform-code-asset` aspect to `SQL_QUERY`](https://docs.cloud.google.com/dataplex/docs/search-syntax#aspect-search).
    For instructions about how to annotate entries with aspects, see
    [Manage aspects and enrich metadata](https://docs.cloud.google.com/dataplex/docs/enrich-entries-metadata)
    in the Knowledge Catalog documentation.

Type
:   The type for saved queries is `SQL_QUERY`.
    This type lets you filter saved queries in the `dataform-code-asset`
    system entry type and the `dataform-code-asset` aspect type by using the
    `aspect:dataplex-types.global.dataform-code-asset.type=SQL_QUERY`
    query in an [aspect-based filter](https://docs.cloud.google.com/dataplex/docs/search-syntax#aspect-search).

For instructions about how to search for assets in Knowledge Catalog, see
[Search for data assets in Knowledge Catalog](https://docs.cloud.google.com/dataplex/docs/search-assets)
in the Knowledge Catalog documentation.

## What's next

- To learn more about BigQuery Studio saved queries, see [Introduction to saved queries](https://docs.cloud.google.com/bigquery/docs/saved-queries-introduction).
- To learn how to create saved queries, see [Create saved queries](https://docs.cloud.google.com/bigquery/docs/work-with-saved-queries).