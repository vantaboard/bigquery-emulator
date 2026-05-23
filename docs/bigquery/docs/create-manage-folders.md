# Create and manage folders

The following document describes how create and manage folders in
BigQuery. You can use folders to organize and control access to
single file code assets, such as [notebooks](https://docs.cloud.google.com/bigquery/docs/create-notebooks),
[saved queries](https://docs.cloud.google.com/bigquery/docs/work-with-saved-queries),
[data canvases](https://docs.cloud.google.com/bigquery/docs/data-canvas), and
[data preparation](https://docs.cloud.google.com/bigquery/docs/data-prep-get-suggestions) files.
BigQuery offers user folders for individuals to manage their own
code assets, and team folders to manage a team's code assets.

BigQuery folders are powered by
[Dataform](https://docs.cloud.google.com/dataform/docs/overview).

Before creating folders, learn how BigQuery folders work
by reading
[Organize code assets with folders](https://docs.cloud.google.com/bigquery/docs/code-asset-folders).

## Before you begin

<br />

### Required roles

To get the permissions that you need to complete the tasks in this document, ask
your administrator to grant you the appropriate IAM roles on the
project, folder, or resource.

To get the permissions that you need to use the BigQuery file
browser, ask your administrator to grant you the [BigQuery
User](https://docs.cloud.google.com/bigquery/docs/access-control#bigquery.user) (`roles/bigquery.user`) or
[BigQuery Studio User](https://docs.cloud.google.com/bigquery/docs/access-control#bigquery.studioUser)
(`roles/bigquery.studioUser`) role on the project.

Permissions granted on a folder propagate to all the files and folders contained
within it.

The following apply to files and the folders that contain them:

| Role | Granted on | Permissions and use cases |
|---|---|---|
| [Code Owner](https://docs.cloud.google.com/iam/docs/roles-permissions/dataform#dataform.codeOwner) (`roles/dataform.codeOwner`) | File or folder | Grants full control over a resource in the files and folders system. A user with this role can perform all actions, including deleting the resource, setting its IAM policy, and moving it. |
| [Code Editor](https://docs.cloud.google.com/iam/docs/roles-permissions/dataform#dataform.codeEditor) (`roles/dataform.codeEditor`) | File or folder | Allows for editing and managing content. A user with this role can add content to folders, edit files, and get the IAM policy for a file or folder. This role is also required on the destination folder when moving a resource. |
| [Code Commenter](https://docs.cloud.google.com/iam/docs/roles-permissions/dataform#dataform.codeCommenter) (`roles/dataform.codeCommenter`) | File or folder | Allows for commenting on code assets or folders. |
| [Code Viewer](https://docs.cloud.google.com/iam/docs/roles-permissions/dataform#dataform.codeViewer) (`roles/dataform.codeViewer`) | File or folder | Provides read-only access. A user with this role can query the contents of files and folders. |
| [Code Creator](https://docs.cloud.google.com/iam/docs/roles-permissions/dataform#dataform.codeCreator) (`roles/dataform.codeCreator`) | Project | Grants permission to create new files and folders within a project. |

The following roles are specific to managing team folders:

| Role | Granted on | Permissions and use cases |
|---|---|---|
| [Team Folder Owner](https://docs.cloud.google.com/iam/docs/roles-permissions/dataform#dataform.teamFolderOwner) (`roles/dataform.teamFolderOwner`) | Team folder | Grants full control over a team folder in the files and folders system. A user with this role can delete the team folder and set its IAM policy. |
| [Team Folder Contributor](https://docs.cloud.google.com/iam/docs/roles-permissions/dataform#dataform.teamFolderContributor) (`roles/dataform.teamFolderContributor`) | Team folder | Allows for content management within a team folder. A user with this role can update a team folder. |
| [Team Folder Commenter](https://docs.cloud.google.com/iam/docs/roles-permissions/dataform#dataform.teamFolderCommenter) (`roles/dataform.teamFolderCommenter`) | Team folder | Allows for commenting on a team folder and the code assets that it contains. |
| [Team Folder Viewer](https://docs.cloud.google.com/iam/docs/roles-permissions/dataform#dataform.teamFolderViewer) (`roles/dataform.teamFolderViewer`) | Team folder | Provides read-only access to a team folder and its contents. A user with this role can view a team folder and get its IAM policy. |
| [Team Folder Creator](https://docs.cloud.google.com/iam/docs/roles-permissions/dataform#dataform.teamFolderCreator) (`roles/dataform.teamFolderCreator`) | Project | Grants permission to create new team folders within a project. |

For more information about granting roles, see
[Manage access to projects, folders, and organizations](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).

These predefined roles contain the permissions required to complete the
tasks in this document. To see the exact permissions that are required, expand
the **Required permissions** section:

#### Required permissions

- Create a folder:
  - `folders.create` on the parent user folder, team folder, or project
  - `folders.addContents` on the parent folder or team folder
- Retrieve the properties of a folder: `folders.get` on the folder
- Query the contents of a folder or team folder: `folders.queryContents` on the folder
- Update a folder: `folders.update` on the folder
- Delete a folder: `folders.delete` on the folder
- Get the IAM policy for a folder: `folders.getIamPolicy` on the folder
- Set the IAM policy for a folder: `folders.setIamPolicy` on the folder
- Move a folder:
  - `folders.move` on the folder being moved
  - `folders.addContents` on the destination folder or team folder (not needed if moving to a root folder)
- Create a team folder: `teamFolders.create` on the project
- Delete a team folder: `teamFolders.delete` on the team folder
- Get the IAM policy for a team folder: `teamFolders.getIamPolicy` on the team folder
- Set the IAM policy for a team folder: `teamFolders.setIamPolicy` on the team folder
- Retrieve the properties of a team folder: `teamFolders.get` on the team folder
- Update a team folder: `teamFolders.update` on the team folder

You might also be able to get these permissions with
[custom roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles)
or other
[predefined roles](https://docs.cloud.google.com/iam/docs/roles-overview#predefined).

To gain full access to all the folders and files in your project, ask your
administrator to grant you the following IAM roles on the
project:

- [Dataform Admin](https://docs.cloud.google.com/dataform/docs/access-control#dataform.admin) (`roles/dataform.admin`)
- [Dataform Editor](https://docs.cloud.google.com/dataform/docs/access-control#dataform.editor) (`roles/dataform.editor`)
- [Dataform Viewer](https://docs.cloud.google.com/dataform/docs/access-control#dataform.viewer) (`roles/dataform.viewer`)

## View resources

Follow these steps to view folders and code assets in
BigQuery:

1. Go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Files**
   to open the file browser:

   ![Click **Files** to open the file browser.](https://docs.cloud.google.com/static/bigquery/images/select-file-browser.png)

   If you don't see the left pane, click **Expand left pane** to open the pane.
3. Do one of the following to view folders and code assets in the selected
   project and code region:

   - Expand the **User (your email address)** node to see folders and files that you have created.
   - Expand the **Team folders** node to view all team folders that you have access to.
   - Expand the **Shared with me** node to view all folders and files that other users have shared with you.

> [!NOTE]
> **Note:** When you open a folder, the breadcrumb trail at the top of the folder view displays the full path to your current location. The path is visible up to the level of the folder for which you have permissions.

### Set the default region for code assets

You can have folders and code assets in
[different code regions](https://docs.cloud.google.com/bigquery/docs/code-asset-folders#folder_code_regions).

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

### Refresh folder contents

To ensure that any recent changes made by you or shared by others are visible
in the file browser, do the following:

1. Go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Files**
   to open the file browser:

   ![Click **Files** to open the file browser.](https://docs.cloud.google.com/static/bigquery/images/select-file-browser.png)
3. Select the folder that you want to refresh, such as your user root node, a
   team folder root, or an individual folder.

4. Click

   **View actions** \> **Refresh contents**.

## Create a folder or code asset

Use this procedure to create any of the following resources:

- A user folder or code asset at any level.
- A subfolder in a team folder.
- A code asset in the subfolder of a team folder.

For information about creating a team folder, see
[Create a team folder](https://docs.cloud.google.com/bigquery/docs/create-manage-folders#create_a_team_folder).

Follow these steps to create a folder or code asset in BigQuery:

1. Go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Files**
   to open the file browser:

   ![Click **Files** to open the file browser.](https://docs.cloud.google.com/static/bigquery/images/select-file-browser.png)
3. Select the user root node or the folder in which you want to create the
   resource.

4. Click
   **View actions** \> **Create**, and then select the type of
   resource that you want to create.

5. In the create resource pane, type a name for the new resource.

6. Click **Save**.

## Create a team folder

Follow these steps to create a team folder in BigQuery:

1. Go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Files**
   to open the file browser:

   ![Click **Files** to open the file browser.](https://docs.cloud.google.com/static/bigquery/images/select-file-browser.png)
3. Select the team folder root node.

4. Click
   **View actions** \> **Create team folder**.

5. In the **Create team folder** dialog, type a name for the team folder.

6. Click **Create**.

## Upload a code asset

Follow these steps to upload a code asset in BigQuery:

1. Go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Files**
   to open the file browser:

   ![Click **Files** to open the file browser.](https://docs.cloud.google.com/static/bigquery/images/select-file-browser.png)
3. Select the folder to which you want to upload the code asset.

4. Click
   **View actions** \> **Upload**, and then select the type of
   code asset that you want to upload.

5. In the upload resource pane, do one of the following:

   - Click the **File upload** radio button, and then browse for and select a local file.
   - Click the **URL** radio button, and then type the URL for a code asset file that resides in a GitHub repository.
6. Type a name for the code asset.

7. Optional: Select a region in which to store the code asset.

   > [!NOTE]
   > **Note:** If you select a different region than the default value, the region that you select becomes the default region where all new code assets are created going forward.

8. Click **Save**.

## Download a code asset

Follow these steps to download a code asset in BigQuery:

1. Go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Files**
   to open the file browser:

   ![Click **Files** to open the file browser.](https://docs.cloud.google.com/static/bigquery/images/select-file-browser.png)
3. Select the code asset that you want to download.

4. Click
   **View actions** \> **Download**.

## Rename a folder or code asset

Follow these steps to rename a folder or code asset in
BigQuery:

1. Go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Files**
   to open the file browser:

   ![Click **Files** to open the file browser.](https://docs.cloud.google.com/static/bigquery/images/select-file-browser.png)
3. Select the folder or code asset that you want to rename.

4. Click
   **View actions** \> **Rename**.

5. In the resource renaming dialog, type a new name for the resource.

6. Click **Rename**.

## Share a folder or code asset

Follow these steps to share a folder or code asset in
BigQuery:

1. Go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Files**
   to open the file browser:

   ![Click **Files** to open the file browser.](https://docs.cloud.google.com/static/bigquery/images/select-file-browser.png)
3. Select the folder or code asset that you want to share.

4. In the **Share permissions** pane, click **Add User/Group**.

5. In the **New principals** field, enter a principal.

6. Do one of the following:

   - In the **Role** list, select one of the following roles to share a code
     asset, including a user folder:

     - [`roles/dataform.codeOwner`](https://docs.cloud.google.com/dataform/docs/access-control#dataform.codeOwner): Can perform any action on the code asset, including deleting or sharing it.
     - [`roles/dataform.codeEditor`](https://docs.cloud.google.com/dataform/docs/access-control#dataform.codeEditor): Can perform any action on the code asset except for deleting or sharing it.
     - [`roles/dataform.codeCommenter`](https://docs.cloud.google.com/dataform/docs/access-control#dataform.codeCommenter): Can view and comment on the code asset.
     - [`roles/dataform.codeViewer`](https://docs.cloud.google.com/dataform/docs/access-control#dataform.codeViewer): Can view the code asset.
   - In the **Role** list, select one of
     the following roles to share a team folder:

     - [`roles/dataform.teamFolderOwner`](https://docs.cloud.google.com/dataform/docs/access-control#dataform.teamFolderOwner): Can perform any action on the team folder, including deleting or sharing it.
     - [`roles/dataform.teamFolderContributor`](https://docs.cloud.google.com/dataform/docs/access-control#dataform.teamFolderContributor): Can perform any action on the team folder except for deleting or sharing it.
     - [`roles/dataform.teamFolderCommenter`](https://docs.cloud.google.com/dataform/docs/access-control#dataform.teamFolderCommenter): Can view and comment on the team folder and the code assets that it contains.
     - [`roles/dataform.teamFolderViewer`](https://docs.cloud.google.com/dataform/docs/access-control#dataform.teamFolderViewer): Can view the team folder and the code assets that it contains.
7. Click **Save**.

8. To return to the notebook information page, click **Close**.

## Move a folder or code asset

Follow these steps to move a folder or code asset in
BigQuery:

1. Go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Files**
   to open the file browser:

   ![Click **Files** to open the file browser.](https://docs.cloud.google.com/static/bigquery/images/select-file-browser.png)
3. Depending on how many resources you want to move, do one of the following:

   - **Move a single resource** : In the file browser, select the folder or code asset you want to move, and then click **View actions** \> **Move**.
   - **Move multiple resources** : In the folder view, select the checkbox next to each resource that you want to move, and then click **Move**.
4. In the move resource dialog, select the user folder or team folder to which
   you want to move the resource.

5. Click **Move**.

> [!NOTE]
> **Note:** Bulk move operations are independent. If one resource fails to move, the operation still attempts to complete the move for all other selected resources.

## Copy a folder or code asset

Follow these steps to copy a folder or code asset in
BigQuery:

1. Go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Files**
   to open the file browser:

   ![Click **Files** to open the file browser.](https://docs.cloud.google.com/static/bigquery/images/select-file-browser.png)
3. Select the folder or code asset that you want to copy.

4. Click
   **View actions** \> **Copy**.

5. In the copy resource dialog, select the user or team folder to which you want
   to copy the resource.

6. Click **Copy**.

## Delete a folder or code asset

Follow these steps to delete a folder or code asset in
BigQuery:

1. Go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Files**
   to open the file browser:

   ![Click **Files** to open the file browser.](https://docs.cloud.google.com/static/bigquery/images/select-file-browser.png)
3. Depending on how many resources you want to delete, do one of the following:

   - **Delete a single resource** : In the file browser, select the folder or code asset you want to delete, and then click **View actions** \> **Delete**.
   - **Delete multiple resources** : In the folder view, select the checkbox next to each resource you want to delete, and then click **Delete**.
4. In the delete resource dialog, click **Delete**.

> [!NOTE]
> **Note:** When deleting multiple resources, the selected folders must be empty for the operation to succeed. Bulk delete operations are independent; if one resource fails to delete, the operation still attempts to complete the deletion for all other selected resources.

## What's next

- [Organize code assets with folders](https://docs.cloud.google.com/bigquery/docs/code-asset-folders)
- [Create notebooks](https://docs.cloud.google.com/bigquery/docs/create-notebooks)
- [Create saved queries](https://docs.cloud.google.com/bigquery/docs/work-with-saved-queries)
- [Create data canvases](https://docs.cloud.google.com/bigquery/docs/data-canvas)
- [Create data preparations](https://docs.cloud.google.com/bigquery/docs/data-prep-get-suggestions)