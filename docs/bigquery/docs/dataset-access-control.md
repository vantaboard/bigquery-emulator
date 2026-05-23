# Changes to dataset-level access controls

If you opt into enforcement of the
`enable_fine_grained_dataset_acls_option` configuration for
dataset-level access controls, the `bigquery.datasets.getIamPolicy`
Identity and Access Management (IAM) permission is
required to view a dataset's access controls and to query the
[`INFORMATION_SCHEMA.OBJECT_PRIVILEGES`](https://docs.cloud.google.com/bigquery/docs/information-schema-object-privileges)
view. The `bigquery.datasets.setIamPolicy` permission is required to update a
dataset's access controls or to [create a dataset with access controls using the
API](https://docs.cloud.google.com/bigquery/docs/dataset-access-control#changes_to_api_methods).

If you don't opt in, the dataset-level access controls are unchanged.

## Opt into enforcement

You can opt into enforcement of the permission
changes. When you opt in, the `bigquery.datasets.getIamPolicy` permission is
necessary to get a dataset's access controls, and the
`bigquery.datasets.setIamPolicy` permission is necessary to update a dataset's
access controls or to create a dataset with access controls using the API.

To opt into enforcement, set the `enable_fine_grained_dataset_acls_option`
configuration setting to `TRUE` at the organization or project level. If you want to opt out after opt in, set the `enable_fine_grained_dataset_acls_option`
configuration setting to `FALSE` at the organization or project level. For
instructions on enabling configuration settings, see [Manage configuration
settings](https://docs.cloud.google.com/bigquery/docs/default-configuration).

### Configuration setting examples

The following examples show you how to set and remove the
`enable_fine_grained_dataset_acls_option` configuration setting.

#### Configure organization settings

To configure organization settings, use the
[`ALTER ORGANIZATION SET OPTIONS` DDL statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#alter_organization_set_options_statement).
The following example sets `enable_fine_grained_dataset_acls_option` to `TRUE`
at the organization level:

```googlesql
ALTER ORGANIZATION
SET OPTIONS (
  `region-REGION.enable_fine_grained_dataset_acls_option` = TRUE);
```

Replace <var translate="no">REGION</var> with the
[region](https://docs.cloud.google.com/bigquery/docs/locations#regions)
associated with your organization---for example, `us` or `europe-west6`.

The following example clears the organization-level
`enable_fine_grained_dataset_acls_option` setting:

```googlesql
ALTER ORGANIZATION
SET OPTIONS (
  `region-REGION.enable_fine_grained_dataset_acls_option` = FALSE);
```

#### Configure project settings

To configure project settings, use the
[`ALTER PROJECT SET OPTIONS` DDL statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#alter_project_set_options_statement).
The `ALTER PROJECT SET OPTIONS` DDL statement optionally accepts the
`project_id` variable. If the `project_id` is not specified, it defaults to the
current project where the query runs.

The following example sets `enable_fine_grained_dataset_acls_option` to `TRUE`.

```googlesql
ALTER PROJECT PROJECT_ID
SET OPTIONS (
  `region-REGION.enable_fine_grained_dataset_acls_option` = TRUE);
```

Replace <var translate="no">PROJECT_ID</var> with your project ID.

The following example clears the project-level
`enable_fine_grained_dataset_acls_option` setting:

```googlesql
ALTER PROJECT PROJECT_ID
SET OPTIONS (
  `region-REGION.enable_fine_grained_dataset_acls_option` = FALSE);
```

## Changes to custom roles

If you opt into enforcement of the permission changes, any custom roles that
grant `bigquery.datasets.get`, `bigquery.datasets.create`, or
`bigquery.datasets.update` permission and don't also grant the
`bigquery.datasets.getIamPolicy` or `bigquery.datasets.setIamPolicy` permission
are impacted.

Any custom roles that only include the `bigquery.datasets.get`,
`bigquery.datasets.update`, or `bigquery.datasets.create` permission must be
updated to include the `bigquery.datasets.getIamPolicy` or
`bigquery.datasets.setIamPolicy` permission, if you want
to maintain the existing functionality of the custom roles. If your custom roles
need to view or update only a dataset's metadata, use the new `dataset_view` and
`update_mode` parameters.

BigQuery predefined roles are *not* affected by this change. All
predefined roles that grant the `bigquery.datasets.get` permission also
grant the `bigquery.datasets.getIamPolicy` permission. All predefined roles that
grant the `bigquery.datasets.update` permission also grant the
`bigquery.datasets.setIamPolicy` permission.

## Changes to bq command-line tool commands

When you opt into early enforcement, the following bq tool commands are
affected.

### bq show

You can use the [`bq show`](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#bq_show)
command with the following flag:

**`--dataset_view={METADATA|ACL|FULL}`**
:   Specifies how to apply permissions when you're viewing a dataset's access
    controls or metadata.
    Use one of the following values:

    - `METADATA`: view only the dataset's metadata. This value requires the `bigquery.datasets.get` permission.
    - `ACL`: view only the dataset's access controls. This value requires the `bigquery.datasets.getIamPolicy` permission.
    - `FULL`: view both the dataset's metadata and access controls. This value requires the `bigquery.datasets.get` permission and `bigquery.datasets.getIamPolicy` permissions.

### bq update

You can use the [`bq update`](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#bq_update)
command with the following flag:

**`--update_mode={UPDATE_METADATA|UPDATE_ACL|UPDATE_FULL}`**
:   Specifies how to apply permissions when you're updating a dataset's access
    controls or metadata.
    Use one of the following values:

    - `UPDATE_METADATA`: update only the dataset's metadata. This value requires the `bigquery.datasets.update` permission.
    - `UPDATE_ACL`: update only the dataset's access controls. This value requires the `bigquery.datasets.setIamPolicy` permission.
    - `UPDATE_FULL`: update both the dataset's metadata and access controls. This value requires the `bigquery.datasets.update` permission and `bigquery.datasets.setIamPolicy` permissions.

## Changes to data control language (DCL) statements

When you opt into early enforcement, the following permissions are required to
run `GRANT` and `REVOKE` statements on datasets using the [data control language
(DCL)](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-control-language):

- `bigquery.datasets.setIamPolicy`

## Changes to `INFORMATION_SCHEMA` view queries

When you opt into early enforcement, the `bigquery.datasets.getIamPolicy`
permission is required to query the
[`INFORMATION_SCHEMA.OBJECT_PRIVILEGES`](https://docs.cloud.google.com/bigquery/docs/information-schema-object-privileges)
view.

## Changes to API methods

After you opt into early enforcement, the following REST v2 API dataset methods
are affected.

### datasets.get method

The [`datasets.get` method](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets/get) has
an additional [query parameter](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets/get#query-parameters)
named `dataset_view`.

This parameter gives you more control over the information returned by the
`datasets.get` method. Rather than always returning both access controls and
metadata, the `dataset_view` parameter lets you specify whether to return just
metadata, just access controls, or both.

The `access` field in the [dataset resource](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets)
contains the dataset's access controls. The other fields such as `friendlyName`,
`description`, and `labels` represent the dataset's metadata.

The following table shows the required permission and API response for the
different values supported by the `dataset_view` parameter:

| Parameter value | Permissions required | API response |
|---|---|---|
| `DATASET_VIEW_UNSPECIFIED` (or empty) | - `bigquery.datasets.get` - `bigquery.datasets.getIamPolicy` | The default value. Returns the dataset's metadata and access controls. |
| `METADATA` | - `bigquery.datasets.get` | Returns the dataset's metadata. |
| `ACL` | - `bigquery.datasets.getIamPolicy` | Returns the dataset's access controls, required fields, and fields in the dataset resource that are output only. |
| `FULL` | - `bigquery.datasets.get` - `bigquery.datasets.getIamPolicy` | Returns the dataset's metadata and access controls. |

If you don't opt into early enforcement, or if you opt out after opting in, you
can use the `dataset_view` parameter with the `METADATA` or `ACL` values. The
`FULL` and `DATASET_VIEW_UNSPECIFIED` (or empty) values default to the previous
behavior; the `bigquery.datasets.get` permission lets you get both metadata
and access controls.

#### Example

The following example sends a `GET` request with the `dataset_view`
parameter set to `METADATA`:

```html
GET https://bigquery.googleapis.com/bigquery/v2/projects/YOUR_PROJECT/datasets/YOUR_DATASET?datasetView=METADATA&key=YOUR_API_KEY HTTP/1.1
```

Replace the following:

- <var translate="no">YOUR_PROJECT</var>: the name of your project
- <var translate="no">YOUR_DATASET</var>: the name of the dataset
- <var translate="no">YOUR_API_KEY</var>: your API key

### datasets.update method

The [`datasets.update` method](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets/update)
has an additional [query parameter](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets/update#query-parameters)
named `update_mode`.

This parameter gives you more control over the fields updated by the
`datasets.update` method. Rather than always allowing updates to both access
controls and metadata, the `update_mode` parameter lets you specify whether to
update just metadata, just access controls, or both.

The `access` field in the [dataset resource](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets)
contains the dataset's access controls. The other fields such as `friendlyName`,
`description`, and `labels` represent the dataset's metadata.

The following table shows the required permission and API response for the
different values supported by the `update_mode` parameter:

| Parameter value | Permissions required | API response |
|---|---|---|
| `UPDATE_MODE_UNSPECIFIED` (or empty) | - `bigquery.datasets.update` - `bigquery.datasets.setIamPolicy` | The default value. Returns the dataset's updated metadata and access controls. |
| `UPDATE_METADATA` | - `bigquery.datasets.update` | Returns the dataset's updated metadata. |
| `UPDATE_ACL` | - `bigquery.datasets.update` - `bigquery.datasets.setIamPolicy` | Returns the dataset's updated access controls, required fields, and fields in the dataset resource that are output only. |
| `UPDATE_FULL` | - `bigquery.datasets.update` - `bigquery.datasets.setIamPolicy` | Returns the dataset's updated metadata and access controls. |

If you don't opt into early enforcement, or if you opt out after opting in,
BigQuery defaults to the previous
behavior; the `bigquery.datasets.update` permission lets you update both
metadata and access controls.

#### Example

The following example sends a `PUT` request with the
`update_mode` parameter set to `METADATA`:

```html
PUT https://bigquery.googleapis.com/bigquery/v2/projects/YOUR_PROJECT/datasets/YOUR_DATASET?updateMode=METADATA&key=YOUR_API_KEY HTTP/1.1
```

Replace the following:

- <var translate="no">YOUR_PROJECT</var>: the name of your project
- <var translate="no">YOUR_DATASET</var>: the name of the dataset
- <var translate="no">YOUR_API_KEY</var>: your API key name

### datasets.patch method

The [`datasets.patch` method](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets/patch)
has an additional [query parameter](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets/patch#query-parameters)
named `update_mode`.

This parameter gives you more control over the fields updated by the
`datasets.patch` method. Rather than always allowing updates to both access
controls and metadata, the `update_mode` parameter lets you specify whether to
update just metadata, just access controls, or both.

The `access` field in the [dataset resource](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets)
contains the dataset's access controls. The other fields such as `friendlyName`,
`description`, and `labels` represent the dataset's metadata.

The following table shows the required permission and API response for the
different values supported by the `update_mode` parameter:

| Parameter value | Permissions required | API response |
|---|---|---|
| `UPDATE_MODE_UNSPECIFIED` (or empty) | - `bigquery.datasets.update` - `bigquery.datasets.setIamPolicy` | The default value. Returns the dataset's updated metadata and access controls. |
| `UPDATE_METADATA` | - `bigquery.datasets.update` | Returns the dataset's updated metadata. |
| `UPDATE_ACL` | - `bigquery.datasets.setIamPolicy` | Returns the dataset's updated access controls, required fields, and fields in the dataset resource that are output only. |
| `UPDATE_FULL` | - `bigquery.datasets.update` - `bigquery.datasets.setIamPolicy` | Returns the dataset's updated metadata and access controls. |

If you don't opt into early enforcement, or if you opt out after opting in,
BigQuery defaults to the previous
behavior; the `bigquery.datasets.update` permission lets you update both
metadata and access controls.

#### Example

The following example sends a `PUT` request with the
`update_mode` parameter set to `METADATA`:

```html
PUT https://bigquery.googleapis.com/bigquery/v2/projects/YOUR_PROJECT/datasets/YOUR_DATASET?updateMode=METADATA&key=YOUR_API_KEY HTTP/1.1
```

Replace the following:

- <var translate="no">YOUR_PROJECT</var>: the name of your project
- <var translate="no">YOUR_DATASET</var>: the name of the dataset
- <var translate="no">YOUR_API_KEY</var>: your API key name

### datasets.insert method

If you opt into early enforcement and use the [`datasets.insert` method](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets/patch),
to create a dataset with access controls, BigQuery verifies
that the `bigquery.datasets.create` and `bigquery.datasets.setIamPolicy`
permissions are granted to the user.

If you use the API to create a dataset without access controls, only
the `bigquery.datasets.create` permission is required.