# Introduction to column-level access control

> [!NOTE]
> **Note:** This feature may not be available when using reservations that are created with certain BigQuery editions. For more information about which features are enabled in each edition, see [Introduction to
> BigQuery editions](https://docs.cloud.google.com/bigquery/docs/editions-intro).

BigQuery provides fine-grained access to sensitive columns using
*policy tags*, or type-based classification, of data. Using
BigQuery column-level access control, you can create policies that
check, at query time, whether a user has proper access. For example, a policy
can enforce access checks such as:

- You must be in `group:high-access` to see the columns containing `TYPE_SSN`.

To enhance column-level access control, you can optionally use
[dynamic data masking](https://docs.cloud.google.com/bigquery/docs/column-data-masking-intro).
Data masking lets you mask sensitive data by substituting null, default, or
hashed content in place of the column's actual value.

## Column-level access control workflow

![Workflow](https://docs.cloud.google.com/static/bigquery/images/column-level-security-workflow.png)

To restrict data access at the column level:

1. **Define a taxonomy and policy tags** . Create and manage a taxonomy and
   policy tags for your data. For guidelines, see
   [Best practices for policy tags](https://docs.cloud.google.com/bigquery/docs/best-practices-policy-tags).

2. **Assign policy tags to your BigQuery columns**. In
   BigQuery, use schema annotations to assign a policy tag to each
   column where you want to restrict access.

3. **Enforce access control on the taxonomy**. Enforcing access control causes
   the access restrictions defined for all of the policy tags in the taxonomy
   to be applied.

4. **Manage access on the policy tags** . Use [Identity and Access Management](https://docs.cloud.google.com/iam)
   (IAM) policies to restrict access to each policy tag. The policy
   is in effect for each column that belongs to the policy tag.

When a user tries to access column data at query time, BigQuery
checks the column policy tag and its policy to see whether the user is authorized to
access the data.

> [!NOTE]
> **Note:** Column-level access control is enforced *in addition* to existing [dataset
> ACLs](https://docs.cloud.google.com/bigquery/docs/dataset-access-controls). A user needs both dataset permission and policy tag permission in order to access data protected by column-level access control.

## Identify what needs to be tagged

To determine the types of sensitive data that you have and which columns need
policy tags, consider generating profiles about your data across an
organization, folder, or project by using Sensitive Data Protection. *Data
profiles* contain metrics and metadata about your tables and help you determine
where [sensitive and high-risk
data](https://docs.cloud.google.com/sensitive-data-protection/docs/sensitivity-risk-calculation) reside.
Sensitive Data Protection reports these metrics at the project, table, and column
levels. For more information, see [Overview of sensitive data discovery](https://docs.cloud.google.com/sensitive-data-protection/docs/data-profiles).

The following image shows a list of column data profiles (click to enlarge).
Columns with high data-risk values might contain [high-sensitivity
data](https://docs.cloud.google.com/sensitive-data-protection/docs/sensitivity-risk-calculation#high-sensitivity) and have no
column-level access controls. Alternatively, those columns might contain
moderate or high-sensitivity data that is accessible to many people.
[![Column data profiles](https://docs.cloud.google.com/static/sensitive-data-protection/docs/images/column-data-profiles.png)](https://docs.cloud.google.com/static/sensitive-data-protection/docs/images/column-data-profiles.png) Column data profiles in Sensitive Data Protection

## Example use case

Consider an organization that needs to classify sensitive data into two
categories: **High** and **Medium**.

![Policy tags](https://docs.cloud.google.com/static/bigquery/images/policy-tags.png)

To set up column level security, a data steward, who has the
[appropriate permissions](https://docs.cloud.google.com/bigquery/docs/column-level-security-intro#roles), would perform the following steps to set up
a [hierarchy of data classification](https://docs.cloud.google.com/bigquery/docs/best-practices-policy-tags).

1. The data steward creates a taxonomy named "Business criticality". The
   taxonomy includes the nodes, or *policy tags* **High** and **Medium**.

2. The data steward decides that the policy for the **High** node includes access
   for a group named **high-tier-access**.

3. The data steward creates more levels of nodes in the taxonomy, under
   **High** and **Medium** . The lowest level node is a leaf node,
   such as the *employee_ssn* leaf node. The data steward can create a different
   access policy for the *employee_ssn* leaf node, or not.

4. The data steward assigns a policy tag to specific table columns.
   In this example, the data steward assigns the **High** access policy to the
   *employee_ssn* column in a table.

5. In the **Current schema** page of the console, the data steward can see the
   policy tag that governs a particular column. In this example, the
   **employee_ssn** column is under the **High** policy tag, so when viewing the
   schema for **employee_ssn** , the console displays the taxonomy name and
   the policy tag in the `Policy tags` field: `Business criticality:High`.

   ![Policy tag UI](https://docs.cloud.google.com/static/bigquery/images/schema-ui-policy-tags3.png)

   For details on using the console to set a policy tag, see
   [Set a policy tag on a column](https://docs.cloud.google.com/bigquery/docs/column-level-security#set_policy).

   Alternatively, you can set the policy tag using the `bq update` command. The
   `names` field of `policyTags` includes the ID of the **High** policy tag,
   `projects/project-id/locations/location/taxonomies/taxonomy-id/policyTags/policytag-id`:

   ```googlesql
   [
    ...
    {
      "name": "ssn",
      "type": "STRING",
      "mode": "REQUIRED",
      "policyTags": {
        "names": ["projects/project-id/locations/location/taxonomies/taxonomy-id/policyTags/policytag-id"]
      }
    },
    ...
   ]
   ```

   For details on using the `bq update` command to set a policy tag, see
   [Set a policy tag on a column](https://docs.cloud.google.com/bigquery/docs/column-level-security#set_policy).

   > [!NOTE]
   > **Note:** You can assign only one policy tag per column.

6. The admin performs similar steps for the **Medium** policy tag.

With this fine-grained access, you can manage access to many columns by
controlling only a small number of data classification policy tags.

For details about these steps, see
[Restricting access with column-level access control](https://docs.cloud.google.com/bigquery/docs/column-level-security).

## Roles used with column-level access control

The following roles are used for BigQuery column-level access control.

The Data Catalog Policy Tag Admin role is required for users who
need to create and manage taxonomies and policy tags.

| Role/ID | Permissions | Description |
|---|---|---|
| Data Catalog Policy Tag Admin (`datacatalog.categoryAdmin`) | `datacatalog.categories.getIamPolicy` `datacatalog.categories.setIamPolicy` `datacatalog.taxonomies.create` `datacatalog.taxonomies.delete` `datacatalog.taxonomies.get` `datacatalog.taxonomies.getIamPolicy` `datacatalog.taxonomies.list` `datacatalog.taxonomies.setIamPolicy` `datacatalog.taxonomies.update` `resourcemanager.projects.get` `resourcemanager.projects.list` | Applies at the project level. This role grants the ability to do the following: - Create, read, update, and delete taxonomies and policy tags. - Get and set IAM policies on policy tags. |

The BigQuery Data Policy Admin role, the BigQuery Admin role or the BigQuery Data Owner role is required in order to create and manage data policies. When you use the
Google Cloud console to enforce access control on a taxonomy, the service silently
creates a data policy for you.

| Role/ID | Permissions | Description |
|---|---|---|
| BigQuery Data Policy Admin (`bigquerydatapolicy.admin`) BigQuery Admin (`bigquery.admin`) BigQuery Data Owner (`bigquery.dataOwner`) | `bigquery.dataPolicies.create` `bigquery.dataPolicies.delete` `bigquery.dataPolicies.get` `bigquery.dataPolicies.getIamPolicy` `bigquery.dataPolicies.list` `bigquery.dataPolicies.setIamPolicy` `bigquery.dataPolicies.update` | The `bigquery.dataPolicies.create` and `bigquery.dataPolicies.list` permissions apply at the project level. The other permissions apply at the data policy level. This role grants the ability to do the following: - Create, read, update, and delete data policies. - Get and set IAM policies on data policies. |

You also need the `datacatalog.taxonomies.get` permission, which you can get from several of the [Data Catalog predefined
roles](https://docs.cloud.google.com/iam/docs/roles-permissions/datacatalog).

The Data Catalog Fine-Grained Reader role is required for users
who need access to data in secured columns.

| Role/ID | Permissions | Description |
|---|---|---|
| Fine-Grained Reader/`datacatalog.categoryFineGrainedReader` | `datacatalog.categories.fineGrainedGet` | Applies at the policy tag level. This role grants the ability to access the content of columns restricted by a policy tag. |

To learn more about Data Catalog roles, see
[Data Catalog Identity and Access Management (IAM)](https://docs.cloud.google.com/data-catalog/docs/concepts/iam).
To learn more about BigQuery roles, see
[Access control with IAM](https://docs.cloud.google.com/bigquery/docs/access-control).

## Impact of writes

To read data from a column that is protected by column-level access control, the user
is always required to have read permission through the fine-grained read access
on the policy tags for the column.

This applies to:

- Tables, including wildcard tables
- Views
- Copying tables

To write data to a row for a column that is protected by column-level access control,
the user requirement depends on the type of write.

If the write operation is an *insert*, fine-grained read access is not required.
However, the user doesn't have access to read the data that was inserted, unless
the user has fine-grained read access.

If a user runs an [INSERT SELECT statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/dml-syntax#insert_select_statement), then the [fine-grained reader role](https://docs.cloud.google.com/bigquery/docs/column-level-security#fine_grained_reader)
is required on the queried table.

If the write operation is an *update* , *delete* , or *merge*, the user can't
perform the operation unless the user has fine-grained read access on the
read columns.

A user can load data from local files or from Cloud Storage. When loading
data to a table, BigQuery does not check the fine-grained reader
permission on the columns of the destination table. This is because loading data
does not require reading content from the destination table. Likewise, a user
can load data from streaming, because streaming loads do not check policy tags.
The user doesn't have access to read the data that was loaded from a stream,
unless the user has fine-grained read access.

For more information, see
[Impact on writes with column-level access control](https://docs.cloud.google.com/bigquery/docs/column-level-security-writes).

## Query tables

If a user has dataset access and has the Data Catalog
Fine-Grained Reader role, the column data is available to the user. The user
runs a query as normal.

If a user has dataset access but does not have the Data Catalog
Fine-Grained Reader role, the column data is not available to the user. If such
a user runs `SELECT *`, they receive an error which lists the columns
that the user cannot access. To resolve the error, you can either:

- Modify the query to exclude the columns that the user cannot access. For
  example, if the user does not have access to the `ssn` column, but does have
  access to the remaining columns, the user can run the following query:

  ```googlesql
  SELECT * EXCEPT (ssn) FROM ...
  ```

  In the preceding example, the `EXCEPT` clause excludes the `ssn` column.
- Ask a Data Catalog Administrator to add the user as a
  Data Catalog Fine-Grained Reader to the relevant data class. The
  error message provides the full name of the policy tag for which the user would
  need access.

## Query views

The impact of column-level security on views is independent of whether or not
the view is an authorized view. In both cases, column-level
security is enforced transparently.

An *authorized view* is one of the following:

- A view that is explicitly authorized to access the tables in a dataset.
- A view that is implicitly authorized to access the tables in a dataset because it is contained in an authorized dataset.

For more information,
see [Authorized views](https://docs.cloud.google.com/bigquery/docs/authorized-views) and
[Authorized datasets](https://docs.cloud.google.com/bigquery/docs/authorized-datasets).

**If the view is not an authorized view:**

If the user has IAM access to the view's underlying tables and dataset as well as column-level access to the view's underlying tables, then the user can query the columns in the view. Otherwise, the user
cannot query the columns in the view.

**If the view is an authorized view:**

Only the column-level security on the columns in the
view's underlying tables controls the access. Table-level and dataset-level IAM
policies, if any, are not used to check access. If the user has access to the
policy tags used in the authorized view's underlying tables, then the user can query
the columns in the authorized view.

The following diagram shows how access to a view is evaluated.

![Accessing views](https://docs.cloud.google.com/static/bigquery/images/view-access.png)

## Impact of time travel and materialized views with max_staleness

BigQuery lets you query a table in an earlier state. This
capability lets you query the rows from a previous point in time. It also
lets you restore a table from a point in time.

In legacy SQL, you query historical data by using [time decorators](https://docs.cloud.google.com/bigquery/docs/table-decorators#time_decorators) on the
table name. In GoogleSQL, you query historical data by using the
[`FOR SYSTEM_TIME AS OF`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#for_system_time_as_of) clause on the table.

Materialized views with the [`max_staleness`](https://docs.cloud.google.com/bigquery/docs/materialized-views-create#max_staleness) option set return historical data
from within their staleness interval. This behavior is similar to a query using
[`FOR SYSTEM_TIME AS OF`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#for_system_time_as_of) at the time of the view's last refresh, as it
allows BigQuery to query records that have been deleted or updated.
Suppose that you query a table's historical data at time *t*. In that case:

- If the schema at time *t* is identical to, or a subset of, the table's current
  schema, then BigQuery checks against the latest column-level
  security on the current table. If the user is allowed to read the current
  columns, then the user can query the historical data of those columns.
  In order to delete or mask sensitive data of columns that are
  protected by column-level security, the column-level security can be safely
  relaxed only after
  [the configured time travel window](https://docs.cloud.google.com/bigquery/docs/time-travel#configure_the_time_travel_window)
  has passed since the clean-up of the sensitive data.

- If the schema at time *t* differs from the current schema for the columns in
  the query, the query fails.

## Location considerations

When you choose a location for your taxonomy, consider the following
limitations.

### Policy tags

Taxonomies are regional resources, like BigQuery datasets and
tables. When you create a taxonomy, you specify the region, or *location*, for
the taxonomy.

You can create a taxonomy and apply policy tags to tables in
[all regions where BigQuery is available](https://docs.cloud.google.com/bigquery/docs/locations#supported_locations).
However, to apply policy tags from a taxonomy to a table column, the taxonomy and the table must exist in the same regional location.

Although you cannot apply a policy tag to a table column that exists in a
different location, you can copy the taxonomy to another location by explicitly
replicating it there.

If you want to use the same taxonomy and policy tags across multiple regional
locations, learn more about replicating taxonomies in
[Managing policy tags across locations](https://docs.cloud.google.com/bigquery/docs/managing-policy-tags-across-locations#replicating_a_taxonomy_in_a_new_location).

### Organizations

You can't use references across organizations. A table and any policy tags that
you want to apply to its columns must exist within the same organization.

## Limitations

- This feature may not be available when using reservations that are created
  with certain BigQuery editions. For more information about
  which features are enabled in each edition, see
  [Introduction to BigQuery editions](https://docs.cloud.google.com/bigquery/docs/editions-intro).

- BigQuery only supports column-level access control for
  [BigLake tables](https://docs.cloud.google.com/bigquery/docs/biglake-intro),
  [BigQuery tables](https://docs.cloud.google.com/bigquery/docs/tables-intro),
  and [BigQuery Omni tables](https://docs.cloud.google.com/bigquery/docs/omni-introduction).

- If you overwrite to a destination table, any existing policy tags are removed
  from the table, unless you use the `--destination_schema` flag to specify a
  schema with policy tags. The following example shows how to use
  `--destination_schema`.

      bq query --destination_table mydataset.mytable2 \
        --use_legacy_sql=false --destination_schema=schema.json \
        'SELECT * FROM mydataset.mytable1'

  Schema changes happen in a separate operation from query execution.
  If you write query results to a table by specifying the
  `--destination_table` flag, and the query subsequently raises an exception,
  it is possible that any schema changes will be skipped. If this occurs,
  check the destination table schema and
  [manually update it](https://docs.cloud.google.com/bigquery/docs/managing-table-schemas) if necessary.
- A column can have only one policy tag.

- A table can have at most 1,000 unique policy tags.

- You can't use legacy SQL if you enabled column-level access control. Any legacy SQL
  queries are rejected if there are any policy tags on the target tables.

- A policy tag hierarchy can be no more than five levels deep from the root node
  to the lowest-level subtag, as shown in the following screenshot:

  ![Policy tag depth.](https://docs.cloud.google.com/static/bigquery/images/policy-tag-depth.png)
- Taxonomy names must be unique among all projects within an organization.

- You can't copy a table across regions if you enabled column-level or row-level
  access control. Any copies of tables across regions are rejected if there are
  any policy tags on the source tables.

## Pricing

Column-level access control requires the use of both BigQuery and
Data Catalog. For pricing information about these products, see the
following topics:

- [BigQuery pricing](https://cloud.google.com/bigquery/pricing)
- [Data Catalog pricing](https://docs.cloud.google.com/dataplex/pricing#data-catalog-pricing)

## Audit logging

When table data with policy tags is read, we save the referenced policy tags in
Cloud Logging. However, the policy tag check is not associated with the
query that triggered the check.

Through Cloud Logging, auditors can understand who has what kind of access to
which categories of sensitive data. For more information, see
[Auditing policy tags](https://docs.cloud.google.com/bigquery/docs/auditing-policy-tags).

For more information about logging in BigQuery, see
[Introduction to BigQuery monitoring](https://docs.cloud.google.com/bigquery/docs/monitoring).

For more information about logging in Google Cloud, see
[Cloud Logging](https://docs.cloud.google.com/logging/docs).

## What's next

- For details about using column-level access control, see
  [Restricting access with column-level access control](https://docs.cloud.google.com/bigquery/docs/column-level-security).

- For information about best practices for policy tags, see
  [BigQuery best practices: Using policy tags](https://docs.cloud.google.com/bigquery/docs/best-practices-policy-tags).