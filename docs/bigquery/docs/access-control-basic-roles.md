# Basic roles and permissions

BigQuery supports IAM
[basic roles](https://docs.cloud.google.com/iam/docs/roles-overview#basic) for project-level access.

> [!CAUTION]
> **Caution:** Avoid using basic roles. They predate IAM and grant excessive and uneven access. Use [predefined IAM](https://docs.cloud.google.com/bigquery/docs/access-control) roles instead.

## Basic roles for projects

By default, granting access to a project also grants access to datasets within
it. Default access can be overridden on a per-dataset basis. The following table
describes what access is granted to members of the basic
IAM roles.

| Basic role | Capabilities |
|---|---|
| `Viewer` | - Can start a job in the project. Additional dataset roles are required depending on the job type. - Can list and get all jobs, and update jobs that they started for the project - If you create a dataset in a project that contains any viewers, BigQuery grants those users the [bigquery.dataViewer](https://docs.cloud.google.com/bigquery/docs/access-control#bigquery.dataViewer) predefined role for the new dataset. |
| `Editor` | - Same as `Viewer`, plus: - Can create a new dataset in the project - If you create a dataset in a project that contains any editors, BigQuery grants those users the [bigquery.dataEditor](https://docs.cloud.google.com/bigquery/docs/access-control#bigquery.dataEditor) predefined role for the new dataset. |
| `Owner` | - Same as `Editor`, plus: - Can revoke or change any project role - Can list all datasets in the project - Can delete any dataset in the project - Can list and get all jobs run on the project, including jobs run by other project users - If you create a dataset, BigQuery grants all project owners the [bigquery.dataOwner](https://docs.cloud.google.com/bigquery/docs/access-control#bigquery.dataOwner) predefined role for the new dataset. **Exception:** When a user runs a query, an [anonymous dataset](https://docs.cloud.google.com/bigquery/docs/cached-results#how_cached_results_are_stored) is created to store the cached results table. Only the user that runs the query is given `OWNER` access to the anonymous dataset. > [!NOTE] > Don't confuse the `OWNER` basic role with the [BigQuery Admin](https://docs.cloud.google.com/bigquery/docs/access-control#bigquery.admin) (`roles/bigquery.admin`) IAM role. BigQuery Admin provides a number of permissions that aren't granted by the `OWNER` basic role. If you're granting project-level access to BigQuery, use IAM roles instead of basic roles. |

Basic roles for projects are granted or revoked through the
[Google Cloud console](https://console.cloud.google.com/). When a project is created,
the `Owner` role is granted to the user who created the
project.

For more information about how to grant or revoke access for project roles, see
[Granting, changing, and revoking access to resources](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access)
in the IAM documentation.

## Basic roles for datasets

The following basic roles apply at the dataset level.

| Dataset role | Capabilities |
|---|---|
| `READER` | - Can read, query, copy or export tables in the dataset. Can read routines in the dataset - Can call [get](https://docs.cloud.google.com/bigquery/docs/reference/v2/datasets/get) on the dataset - Can call [get](https://docs.cloud.google.com/bigquery/docs/reference/v2/tables/get) and [list](https://docs.cloud.google.com/bigquery/docs/reference/v2/tables/list) on tables in the dataset - Can call [get](https://docs.cloud.google.com/bigquery/docs/reference/v2/routines/get) and [list](https://docs.cloud.google.com/bigquery/docs/reference/v2/routines/list) on routines in the dataset - Can call [list](https://docs.cloud.google.com/bigquery/docs/reference/v2/tabledata/list) on table data for tables in the dataset > [!NOTE] > The [BigQuery Data Viewer](https://docs.cloud.google.com/bigquery/docs/access-control#bigquery.dataViewer) (`roles/bigquery.dataViewer`) predefined IAM role is mapped to the `READER` BigQuery basic role. When you grant BigQuery Data Viewer to a principal at the dataset level, the principal is granted `READER` access to the dataset. |
| `WRITER` | - Same as `READER`, plus: - Can edit or append data in the dataset - Can call [insert](https://docs.cloud.google.com/bigquery/docs/reference/v2/tables/insert), [insertAll](https://docs.cloud.google.com/bigquery/docs/reference/v2/tabledata/insertAll), [update](https://docs.cloud.google.com/bigquery/docs/reference/v2/tables/update) or [delete](https://docs.cloud.google.com/bigquery/docs/reference/v2/tables/delete) on tables - Can use tables in the dataset as destinations for load, copy or query jobs - Can call [insert](https://docs.cloud.google.com/bigquery/docs/reference/v2/routines/insert), [update](https://docs.cloud.google.com/bigquery/docs/reference/v2/routines/update), or [delete](https://docs.cloud.google.com/bigquery/docs/reference/v2/routines/delete) on routines > [!NOTE] > The [BigQuery Data Editor](https://docs.cloud.google.com/bigquery/docs/access-control#bigquery.dataEditor) (`roles/bigquery.dataEditor`) predefined IAM role is mapped to the `WRITER` BigQuery basic role. When you grant BigQuery Data Editor to a principal at the dataset level, the principal is granted `WRITER` access to the dataset. |
| `OWNER` | - Same as `WRITER`, plus: - Can call [update](https://docs.cloud.google.com/bigquery/docs/reference/v2/datasets/update) on the dataset - Can call [delete](https://docs.cloud.google.com/bigquery/docs/reference/v2/datasets/delete) on the dataset A dataset must have at least one entity with the `OWNER` role. A user with the `OWNER` role can't remove their own `OWNER` role. > [!NOTE] > The [BigQuery Data Owner](https://docs.cloud.google.com/bigquery/docs/access-control#bigquery.dataOwner) (`roles/bigquery.dataOwner`) predefined IAM role is mapped to the `OWNER` BigQuery basic role. When you grant BigQuery Data Owner to a principal at the dataset level, the principal is granted `OWNER` access to the dataset. |

For more information on assigning roles at the dataset level, see
[Controlling access to datasets](https://docs.cloud.google.com/bigquery/docs/dataset-access-controls).

When you create a new dataset, BigQuery adds default dataset access for
the following entities. Roles that you specify on dataset creation overwrite the
default values.

| Entity | Dataset role |
|---|---|
| All users with `Viewer` access to the project | `READER` |
| All users with `Editor` access to the project | `WRITER` |
| All users with `Owner` access to the project, and the dataset creator | `OWNER` **Exception:** When a user runs a query, an [anonymous dataset](https://docs.cloud.google.com/bigquery/docs/cached-results#how_cached_results_are_stored) is created to store the cached results table. Only the user that runs the query is given `OWNER` access to the anonymous dataset. |