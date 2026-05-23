# Organize code assets with folders

The following document describes how BigQuery folders work. You can use
folders to organize code assets in a hierarchical structure, similar to that
used by operating systems. For example, you could create a folder to organize
code assets for sales analysis, with subfolders for each fiscal year. You
can also use folders to manage access to code assets. Folders offer
Identity and Access Management (IAM) policy inheritance, which you can use to control
access to code assets more efficiently. Policy inheritance lets subfolders
and files inherit the permissions of their parent folder.

BigQuery folders are powered by
[Dataform](https://docs.cloud.google.com/dataform/docs/overview).

For more information about working with folders in BigQuery, see
[Create and manage folders](https://docs.cloud.google.com/bigquery/docs/create-manage-folders).

## Organize code assets with folders

You can access and organize your code assets by using folders in the
**Files** pane of BigQuery Studio:

![The location of the **Files** pane within BigQuery Studio.](https://docs.cloud.google.com/static/bigquery/images/file-browser.png)

A folder is the basic container for organizing code assets, similar to a
standard file system folder. You can create and organize subfolders within a
parent folder, and you can move code assets into and out of folders. When you
grant permissions on a folder, the permissions propagate to all of the folders
and files contained by that folder.

You can only use folders to organize single file code assets, such as notebooks,
saved queries, data canvases, and data preparation files.

### Folder types

BigQuery supports the following types of folders:

- **User folders** : Each user has a personal root folder, displayed as
  **User (user email address)** . Your user folder contains all of the code
  assets that you create in the given project and
  [location](https://docs.cloud.google.com/bigquery/docs/locations). You can create subfolders in your user
  folder to organize these code assets. Files and folders in your user folder
  are accessible only to you unless you choose to share them with other users.

- **Team folders**: A team folder is designed for team collaboration, similar to
  a shared drive in Google Drive. You can use team folders to organize code
  assets that belong to a particular team. Only users who have owner
  permissions on the root team folder can grant permissions to enable other
  users to use the team folder.

### Folder code regions

You can have folders and code assets in different code regions. For
example, you could have `folderA` and the code assets that it contains in the
`us-west1` region, and `folderB` and the code assets that it contains in the
`us-central1` region. The region that you are viewing is displayed
in the **Files** pane:

![The current code region is displayed next to the project name in the **Files** pane.](https://docs.cloud.google.com/static/bigquery/images/folder-location.png)

## IAM policy inheritance

IAM access for file and folder resources uses a
hierarchical structure. This hierarchy ensures that access policies are
inherited from parent folders to their contents.

When an IAM policy is set on a folder, the permissions granted
by that policy also apply to all the files and nested subfolders in the
folder's subtree. This has the following consequences:

- Permissions are inherited through the folder hierarchy. When a user is granted a specific role on a high-level folder, they possess the permissions included in that role for all the resources contained in that folder and its subfolders.
- The permissions that a user has on a resource consist of the policies set directly on that resource and all the policies inherited from every folder in its path up to the root.

As a result, you don't need project-level permissions to perform actions on
resources located deep in a folder structure. You only need the proper
permission on any folder in the path to that resource. For example, if you
want to create a file in a subfolder, you need the necessary permissions on
either the specific subfolder or any of its parent folders, which includes the
top-level folder.

The following are best practices for applying IAM policies to
files and folders:

- Apply IAM policies to the highest folder in the hierarchy where the permissions are uniformly needed. For example, if a team needs access to all the data in their team's directory, grant the necessary roles at the level of the team folder instead of at the level of individual project subfolders.
- Always grant the minimum set of permissions required for users or services to perform their tasks. Avoid granting broad roles where you can use more specific folder-level roles and permissions.

> [!NOTE]
> **Note:** Roles granted to individual files or folders are preserved during a move, but inherited roles update to match the new location. Additionally, roles configured on the source and destination folders remain unaffected by a move.

## IAM roles granted on resource creation

When you create a folder, the following roles are granted automatically:

- Users who create folders in their user root node automatically receive the [Dataform Admin role](https://docs.cloud.google.com/dataform/docs/access-control#dataform.admin) (`roles/dataform.admin`) on those folders.
- The creator of a root team folder automatically receives the [Dataform Admin role](https://docs.cloud.google.com/dataform/docs/access-control#dataform.admin) (`roles/dataform.admin`) on that team folder.

You can
[use the Config API to grant a specific role upon resource creation](https://docs.cloud.google.com/dataform/docs/access-control#grant-specific-role).

You don't automatically receive any roles when you create new files or folders
within a team folder's subtree.

## Busy resources

A user folder or team folder is "busy" if it's actively involved in a
move operation, either as the object being moved or the destination of the
move. The system restricts busy resources from the following actions to ensure
data integrity during the move:

- Being the object of another move operation.
- Being the destination of another move operation.
- Being an ancestor of a move object.
- Being the object of a delete operation.

## Limitations

BigQuery folders have the following limitations:

- You can only nest folders up to 5 levels deep.
- Having a very large number of folders (hundreds of thousands) slows performance when working with folders. For example, when loading the file explorer or expanding a folder.
- You can't [move a folder](https://docs.cloud.google.com/bigquery/docs/create-manage-folders#move_a_folder_or_code_asset) that contains more than 100 files or folders.

## Locations

BigQuery folders are supported in all
[Dataform locations](https://docs.cloud.google.com/dataform/docs/locations).

## What's next

- [Create and manage folders](https://docs.cloud.google.com/bigquery/docs/create-manage-folders)
- [Create notebooks](https://docs.cloud.google.com/bigquery/docs/create-notebooks)
- [Create saved queries](https://docs.cloud.google.com/bigquery/docs/work-with-saved-queries)
- [Create data canvases](https://docs.cloud.google.com/bigquery/docs/data-canvas)
- [Create data preparations](https://docs.cloud.google.com/bigquery/docs/data-prep-get-suggestions)