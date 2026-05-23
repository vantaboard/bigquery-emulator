# Control access with IAM Conditions

This document describes how to use
[IAM Conditions](https://docs.cloud.google.com/iam/docs/conditions-overview) to control access to
BigQuery resources.

IAM Conditions let you grant access to BigQuery
resources only if specified conditions are met. For example, you can grant
access to a resource for a limited duration or periodically for certain hours of
the day. You can add IAM Conditions at the organization, folder,
project, and dataset levels of resources. Allow policies with conditions are
inherited by child resources. For more information about resource levels, see
[resource hierarchies](https://docs.cloud.google.com/iam/docs/overview#resource-hierarchy).

IAM Conditions are useful for granting Identity and Access Management (IAM)
permissions to many related resources simultaneously, including resources
that don't exist yet. To grant permissions to unrelated groups of
BigQuery resources, consider using
[IAM tags](https://docs.cloud.google.com/bigquery/docs/tags).

## Before you begin

1. [Grant users IAM roles](https://docs.cloud.google.com/bigquery/docs/conditions#required_permissions) that contain the necessary permissions to perform each task in this document.
2. [Enable the IAM API](https://docs.cloud.google.com/bigquery/docs/conditions#enable-api).

### Required roles


To get the permissions that
you need to
apply IAM Conditions to BigQuery resources,

ask your administrator to grant you the
following IAM roles:

- For projects: [Project IAM Admin](https://docs.cloud.google.com/iam/docs/roles-permissions/resourcemanager#resourcemanager.projectIamAdmin) (`roles/resourcemanager.projectIamAdmin`)
- For datasets: [BigQuery Data Owner](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquery#bigquery.dataOwner) (`roles/bigquery.dataOwner`)


For more information about granting roles, see [Manage access to projects, folders, and organizations](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).


These predefined roles contain

the permissions required to
apply IAM Conditions to BigQuery resources. To see the exact permissions that are
required, expand the **Required permissions** section:


#### Required permissions

The following permissions are required to
apply IAM Conditions to BigQuery resources:

- Set conditional IAM access at the project level: `resourcemanager.projects.setIamPolicy`
- Set conditional IAM access on datasets:
  - `bigquery.datasets.setIamPolicy`
  - `bigquery.datasets.update`


You might also be able to get
these permissions
with [custom roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles) or
other [predefined roles](https://docs.cloud.google.com/iam/docs/roles-overview#predefined).
If you plan to use IAM Conditions across your organization, you also need [permissions to manage organization policies](https://docs.cloud.google.com/resource-manager/docs/organization-policy/using-constraints#required-roles).

<br />

For more information about IAM roles and permissions in
BigQuery, see
[Introduction to IAM](https://docs.cloud.google.com/bigquery/docs/access-control).

### Enable the IAM API

To enable the IAM API, select one of the following options:

### Console

Go to the **Identity and Access Management (IAM) API** page and enable the
API.

[Enable the API](https://console.cloud.google.com/apis/library/iam.googleapis.com)

### gcloud

Run the [`gcloud services enable` command](https://docs.cloud.google.com/sdk/gcloud/reference/services/enable):

```bash
gcloud services enable iam.googleapis.com
```

<br />

## View conditional access policy on a dataset

Select one of the following options:

### Console

1. Go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)

   If you don't see the left pane, click **Expand left pane** to open the pane.
3. In the **Explorer** pane, expand your project, click **Datasets**, and then
   select a dataset.

4. Click **Sharing**
   \> **Permissions**.

5. Next to the associated role, click **condition:`TITLE`** to see
   the condition for that role.

### bq

To view or update conditional access policies in Cloud Shell, you must
be using Cloud Shell version 503.0.0 or later.

To get an existing access policy and output it to a local file in JSON, use
the
[`bq show` command](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#bq_get-iam-policy)
in Cloud Shell:

```bash
bq show --format=prettyjson PROJECT_ID:DATASET > PATH_TO_FILE
```

Replace the following:

- <var translate="no">PROJECT_ID</var>: your project ID
- <var translate="no">DATASET</var>: the name of your dataset
- <var translate="no">PATH_TO_FILE</var>: the path to the JSON file on your local machine

The `access` property in the dataset resource JSON file contains the access
policy.

### API

To view the access policy of a dataset with conditions, call
[`datasets.get`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets/get) with
`accessPolicyVersion=3` as a request parameter.
The `access` property in the dataset resource contains the access policy.

## Modify conditional access on resources

The following sections describe how to add or remove conditional access to
different resources.

### Add conditions to an organization, folder, or project

To add conditional access to an organization, folder, or project in
BigQuery, see
[Allow policies with conditions](https://docs.cloud.google.com/iam/docs/conditions-overview#syntax_overview).
When building your conditions, refer to the
[attribute format tables](https://docs.cloud.google.com/bigquery/docs/conditions#attribute_formats).

### Add conditional access to dataset

To add conditions to a dataset, select one of the following methods. When you
build your conditions, reference the
[attribute format tables](https://docs.cloud.google.com/bigquery/docs/conditions#attribute_formats).

### Console

1. Go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)
3. In the **Explorer** pane, expand your project, click **Datasets**, and then
   select a dataset.

4. In the details pane, click
   **Share \> Permissions**.

5. Click **Add Principal**.

6. In the **New principals** field, enter a principal.

7. In the **Select a role** list, select a predefined role or a custom role.

8. Click **Add IAM condition**.

9. Add conditions to the `condition` field using the
   [condition attributes](https://docs.cloud.google.com/bigquery/docs/conditions#condition_attributes).

10. In the **Add IAM condition** panel, Click **Save**.

11. In the **Grant access to `DATASET`** panel, click **Save**.

### bq

To view or update conditional access policies in Cloud Shell, you must
be using Cloud Shell version 503.0.0 or later.

To grant conditional access to a dataset using Cloud Shell, follow the
directions for
[granting access to a dataset](https://docs.cloud.google.com/bigquery/docs/control-access-to-resources-iam#grant_access_to_a_dataset).
You can add your conditional access conditions to the `access` section of the
dataset's JSON file.

For example, the following addition to the `access` section of a dataset's
JSON file would grant the `roles/bigquery.dataViewer` role to
`cloudysanfrancisco@gmail.com` until December 31, 2032:

```json
"access": [
  {
    "role": "roles/bigquery.dataViewer",
    "userByEmail": "cloudysanfrancisco@gmail.com",
    "condition": {
      "title": "Grant roles/bigquery.dataViewer until 2033",
      "description": "Role expires on December 31, 2032.",
      "expression": "request.time < timestamp('2032-12-31T12:00:00Z')"
    }
  }
]
```

### API

To grant conditional access to a dataset using the BigQuery API,
follow the directions for
[granting access to a dataset](https://docs.cloud.google.com/bigquery/docs/control-access-to-resources-iam#grant_access_to_a_dataset)
with `accessPolicyVersion=3` in the request parameter.

You can add entries with access conditions to the
[`access.condition`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets)
property of the dataset resource.

For datasets that have conditional access policies, users can update
unconditional access settings by using the standard read, modify, and update
flows without specifying the `accessPolicyVersion` request parameter.

### Remove conditional access from datasets

To remove conditions from a dataset, select one of the following methods. When
you build your conditions, reference the
[attribute format tables](https://docs.cloud.google.com/bigquery/docs/conditions#attribute_formats).

### Console

1. Go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)
3. In the **Explorer** pane, expand your project, click **Datasets**, and then
   select a dataset.

4. In the details pane, click
   **Share \> Permissions**.

5. Select the principal whose access you want to revoke.

6. Click **Delete**.

7. In the **Delete principal?** dialog, click **Delete**.

### bq

To view or update conditional access policies in Cloud Shell, you must
be using Cloud Shell version 503.0.0 or later.

To remove conditional access to a dataset using Cloud Shell, follow the
directions in
[Revoke access to a dataset](https://docs.cloud.google.com/bigquery/docs/control-access-to-resources-iam#revoke_access_to_a_dataset).
You can remove the entry with condition from the `access` section of the
dataset's JSON file.

### API

To revoke conditional access to a dataset using the BigQuery API,
follow the directions for
[revoking access to a dataset](https://docs.cloud.google.com/bigquery/docs/control-access-to-resources-iam#revoke_access_to_a_dataset)
with `accessPolicyVersion=3` in the request parameter.

You can remove entries with conditions from the `access` property of the
dataset resource.

For datasets that have conditional access policies, users can update
unconditional access settings by using the standard read, modify, and update
flows without specifying the `accessPolicyVersion` request parameter.

## Condition attributes

You can set IAM Conditions on your BigQuery resources,
based on the following attributes:

- `request.time`: the time at which the user attempts to access a BigQuery resource. For more details and examples, see [Date/time attribute](https://docs.cloud.google.com/iam/docs/conditions-attribute-reference#date-time).
- `resource.name`: the path of the BigQuery resource. For the format, see the tables in [Attribute formats](https://docs.cloud.google.com/bigquery/docs/conditions#attribute_formats).
- `resource.type`: the type of the BigQuery resource. For the format, see the tables in [Attribute formats](https://docs.cloud.google.com/bigquery/docs/conditions#attribute_formats).
- `resource.service`: the Google Cloud service that the BigQuery resource uses. For the format, see the tables in [Attribute formats](https://docs.cloud.google.com/bigquery/docs/conditions#attribute_formats).
- `resource.tags`: the tags attached to the BigQuery resource. Tags are supported only on BigQuery datasets, tables, and views resources. For the format, see the tables in [Attribute formats](https://docs.cloud.google.com/bigquery/docs/conditions#attribute_formats) and in [IAM docs](https://docs.cloud.google.com/iam/docs/conditions-attribute-reference#resource-tags).

### Attribute formats

When you create conditions for BigQuery datasets, use the
following formats:

| Attribute | Value |
|---|---|
| `resource.type` | `bigquery.googleapis.com/Dataset` |
| `resource.name` | `projects/PROJECT_ID/datasets/DATASET_ID` |
| `resource.service` | `bigquery.googleapis.com` |
| `resource.tags` | Supports `hasTagKey`, `hasTagKeyId`, `matchTag` and `matchTagId`. For more information, see [Resource tags](https://docs.cloud.google.com/iam/docs/conditions-attribute-reference#resource-tags). |

When you create conditions for BigQuery tables and views, use the
following formats:

| Attribute | Value |
|---|---|
| `resource.type` | `bigquery.googleapis.com/Table` |
| `resource.name` | `projects/PROJECT_ID/datasets/DATASET_ID/tables/TABLE_ID` |
| `resource.service` | `bigquery.googleapis.com` |
| `resource.tags` | Supports `hasTagKey`, `hasTagKeyId`, `matchTag` and `matchTagId`. For more information, see [Resource tags](https://docs.cloud.google.com/iam/docs/conditions-attribute-reference#resource-tags). |

When you create conditions for BigQuery routines, use the
following formats:

| Attribute | Value |
|---|---|
| `resource.type` | `bigquery.googleapis.com/Routine` |
| `resource.name` | `projects/PROJECT_ID/datasets/DATASET_ID/routines/ROUTINE_ID` |
| `resource.service` | `bigquery.googleapis.com` |

When you create conditions for BigQuery models, use the following
formats:

| Attribute | Value |
|---|---|
| `resource.type` | `bigquery.googleapis.com/Model` |
| `resource.name` | `projects/PROJECT_ID/datasets/DATASET_ID/models/MODEL_ID` |
| `resource.service` | `bigquery.googleapis.com` |

Replace the following:

- `PROJECT_ID`: the ID of the project that contains the resources that you are granting access to
- `DATASET_ID`: the ID of the dataset that you are granting access to
- `TABLE_ID`: the ID of the table or view that you are granting access to
- `ROUTINE_ID`: the ID of the routine that you are granting access to
- `MODEL_ID`: the ID of the model that you are granting access to

## Conditions best practices

When you build conditions in BigQuery, use the following best
practices:

- We recommend using positive conditions for `resource.type`, `resource.name`, and `resource.service` for greater accuracy. Since unsupported types are represented by an empty string, negative conditions might match a wide range of resources. For more information, see [negative conditions](https://docs.cloud.google.com/bigquery/docs/conditions#negative_conditions).
- Dataset-level IAM conditions should only be used for roles that apply to resources within the dataset, such as tables, views, models, and routines. They shouldn't be used to grant roles that operate at the dataset or project level, like `bigquery.user` or `bigquery.jobUser`.
- Don't use the condition `resource.type == 'bigquery.googleapis.com/Dataset'` for dataset-level policies, because it has no effect on authorization. This attribute is intended to control access to sub-dataset resources like tables, views, routines, and models.
- Include `resource.type`, `resource.name`, and `resource.service` in your condition, even when that level of specificity isn't necessary. This practice helps sustain your conditions as resources in your workflow change so that other resources are not unintentionally included in the future.
- When granting permissions, include the narrowest possible set of permissions to ensure that you don't unintentionally give overly permissive access.
- Use `resource.name.startsWith` with caution. BigQuery table and view paths are prefixed by their parent project ID and dataset ID. Insufficiently specific conditions might grant too much access. However, you can use the `resource.name.startsWith` attribute to let users run wildcard queries on tables. For example, access granted using the `resource.name.startsWith("projects/my_project/datasets/my_dataset/tables/table_prefix")` condition lets users run the `SELECT * FROM my_dataset.table_prefix*` query.
- Don't add conditions for BigQuery resources other than datasets, tables, views, routines, and models.
- Verify that you are granting the correct permissions on the correct resource. For example, the permission to list resources (`bigquery.RESOURCE.list`) must be granted at the parent level, but the permission to delete resources (`bigquery.RESOURCE.delete`) must be granted at the resource level. Dataset deletion, where all contained resources are also deleted, requires table, model, and routine deletion permissions on the dataset.
- Be aware that [table snapshots](https://docs.cloud.google.com/bigquery/docs/table-snapshots-intro) and [time travel](https://docs.cloud.google.com/bigquery/docs/time-travel) have no effect on permissions.

### Negative conditions

Negative conditions like `resource.name != resource` can inadvertently grant
overly permissive access. Unsupported BigQuery resources have
empty resource attributes, meaning they match all negative conditions. Resources
in services outside of BigQuery might match negative conditions
as well.

Additionally, negative conditions create problems when users run queries with
wildcards. For example, consider the negative condition
`resource.name != /projects/my_project/datasets/my_dataset/tables/secret`. This
condition appears to grant access to all resources, except a table named
`secret`. However, the user is still able to query that table using a wildcard
query, such as `SELECT * from my_project.my_dataset.secre*;`.

Also, negative conditions on tables, routines, and models might give overly
permissive access to their parent datasets. Users might then be able to delete
those resources because deletion permissions are managed at the dataset level.

## Limitations

- You can't add [authorized view](https://docs.cloud.google.com/bigquery/docs/authorized-views), [authorized routine](https://docs.cloud.google.com/bigquery/docs/authorized-routines), or [authorized dataset](https://docs.cloud.google.com/bigquery/docs/authorized-datasets) grants with IAM Conditions.
- If an incompatible `accessPolicyVersion` is used while viewing a resource with conditions, bindings might contain `withcond` followed by a hash value. For more information, see [Troubleshooting `withcond` in policies and role bindings](https://docs.cloud.google.com/iam/docs/troubleshooting-withcond).
- Users with conditional access to a dataset or table cannot modify permissions for that resource through the Google Cloud console. Permission modifications are only supported through the bq tool and the BigQuery API.
- Row-level and column-level access control are not supported directly through IAM Conditions. However, a user with conditional access can grant themselves the BigQuery Admin role (`roles/bigquery.admin`) on the table, and then modify row and column access policies.
- Changes to IAM policies can take up to five minutes to take effect.
- Users with conditional access might not be able to query [`INFORMATION_SCHEMA` views](https://docs.cloud.google.com/bigquery/docs/information-schema-intro).
- Users with only conditional table access cannot run [table wildcard functions](https://docs.cloud.google.com/bigquery/docs/reference/legacy-sql#tablewildcardfunctions).

## Examples

The following are examples of use cases for IAM Conditions in
BigQuery.

### Grant read access to a specific table

This example grants `cloudysanfrancisco@gmail.com` the BigQuery Data Viewer role
for the `table_1` table in the `dataset_1` dataset. With this role, the user can
query the table and access it through the bq tool. The user can't view
the table in the Google Cloud console because they don't have the
`bigquery.tables.list` permission on the dataset.

```
{
  "members": [cloudysanfrancisco@gmail.com],
  "role": roles/bigquery.dataViewer,
  "condition": {
    "title": "Table dataset_1.table_1",
    "description": "Allowed to read table with name table_1 in dataset_1 dataset",
    "expression":
resource.name == projects/project_1/datasets/dataset_1/tables/table_1
&& resource.type == bigquery.googleapis.com/Table
  }
}
```

### Grant list access to a specific dataset

This example grants `cloudysanfrancisco@gmail.com` the BigQuery Metadata Viewer
role on the `dataset_2` dataset. With this role, the user can list all the
resources in the dataset, but they can't perform any queries on those resources.

```
{
  "members": [cloudysanfrancisco@gmail.com],
  "role": roles/bigquery.metadataViewer,
  "condition": {
    "title": "Dataset dataset_2",
    "description": "Allowed to list resources in dataset_2 dataset",
    "expression":
resource.name == projects/project_2/datasets/dataset_2
&& resource.type == bigquery.googleapis.com/Dataset
  }
}
```

### Grant owner access to all tables in all datasets with a specific prefix

This example grants `cloudysanfrancisco@gmail.com` the BigQuery Data Owner role
on all tables in all datasets that start with the `public_` prefix:

```
{
  "members": [cloudysanfrancisco@gmail.com],
  "role": roles/bigquery.dataOwner,
  "condition": {
    "title": "Tables public_",
    "description": "Allowed owner access to tables in datasets with public_ prefix",
    "expression":
resource.name.startsWith("projects/project_3/datasets/public_")
&& resource.type == bigquery.googleapis.com/Table
  }
}
```

### Grant owner access to all tables, models, and routines in all datasets that have a specific prefix

This example grants `cloudysanfrancisco@gmail.com` the BigQuery Data Owner role
on all tables, models, and routines in all datasets that start with the
`general_` prefix:

```
{
  "members": [cloudysanfrancisco@gmail.com],
  "role": roles/bigquery.dataOwner,
  "condition": {
    "title": "Tables general_",
    "description": "Allowed owner access to tables in datasets with general_ prefix",
    "expression":
resource.name.startsWith("projects/project_4/datasets/general_")
&& resource.type == bigquery.googleapis.com/Table
  }
},
{
  "members": [cloudysanfrancisco@gmail.com],
  "role": roles/bigquery.dataOwner,
  "condition": {
    "title": "Models general_",
    "description": "Allowed owner access to models in datasets with general_ prefix",
    "expression":
resource.name.startsWith("projects/project_4/datasets/general_")
&& resource.type == bigquery.googleapis.com/Model
  }
},
{
  "members": [cloudysanfrancisco@gmail.com],
  "role": roles/bigquery.dataOwner,
  "condition": {
    "title": "Routines general_",
    "description": "Allowed owner access to routines in datasets with general_ prefix",
    "expression":
resource.name.startsWith("projects/project_4/datasets/general_")
&& resource.type == bigquery.googleapis.com/Routine
  }
}
```

## What's next

- Learn more about [configuring temporary access](https://docs.cloud.google.com/iam/docs/configuring-temporary-access) by using IAM Conditions.
- Learn more about [configuring resource-based access](https://docs.cloud.google.com/iam/docs/configuring-resource-based-access) by using IAM Conditions.