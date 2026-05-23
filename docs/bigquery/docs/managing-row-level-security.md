# Use row-level security

This document explains how to use row-level security in BigQuery to
restrict access to data at the table row level. Before you read this document,
familiarize yourself with an overview about row-level security by reading
[Introduction to BigQuery row-level security](https://docs.cloud.google.com/bigquery/docs/row-level-security-intro).

You can perform the following tasks with row-level access policies:

- [Create or update a row-level access policy](https://docs.cloud.google.com/bigquery/docs/managing-row-level-security#create-policy) on a table
- [Combine row-level access policies](https://docs.cloud.google.com/bigquery/docs/managing-row-level-security#combine_row-level_access_policies) on a table
- [List a table's row-level access policies](https://docs.cloud.google.com/bigquery/docs/managing-row-level-security#list-policy)
- [Delete a row-level access policy](https://docs.cloud.google.com/bigquery/docs/managing-row-level-security#delete-policy) from a table
- [Query a table with a row-level access policy](https://docs.cloud.google.com/bigquery/docs/managing-row-level-security#query-policy)

> [!NOTE]
> **Note:** When managing access for users in [external identity providers](https://docs.cloud.google.com/iam/docs/workforce-identity-federation), replace instances of Google Account principal identifiers---like `user:kiran@example.com`, `group:support@example.com`, and `domain:example.com`---with appropriate [Workforce Identity Federation principal identifiers](https://docs.cloud.google.com/iam/docs/principal-identifiers).

## Limitations

When you select a table in the Google Cloud console, the **Preview** tab
cannot show previews of tables with row-access policies. To view the
contents of the table, run a query.

## Before you begin

Grant Identity and Access Management (IAM) roles that give users the necessary permissions
to perform each task in this document. The permissions required to perform a
task (if any) are listed in the "Required permissions" section of the task.

## Create or update a row-level access policy

You can create or update a row-level access policy on a table in
BigQuery with a data definition language (DDL) statement.

### Required permissions

To create a row-level access policy on a BigQuery table, you need
the following IAM permissions:

- `bigquery.rowAccessPolicies.create`
- `bigquery.rowAccessPolicies.setIamPolicy`
- `bigquery.tables.getData` (on the target table and any referenced tables in granted subquery row-level access policies)
- `bigquery.jobs.create` (to run the DDL query job)

To update a row-level access policy on a BigQuery table, you need
the following IAM permissions:

- `bigquery.rowAccessPolicies.update`
- `bigquery.rowAccessPolicies.setIamPolicy`
- `bigquery.tables.getData` (on the target table and any referenced tables in granted subquery row-level access policies)
- `bigquery.jobs.create` (to run the DDL query job)

Each of the following predefined IAM roles includes the
permissions that you need in order to create and update a row-level access
policy:

- `roles/bigquery.admin`
- `roles/bigquery.dataOwner`

#### The `bigquery.filteredDataViewer` role

When you create a row-level access policy, BigQuery
automatically grants the `bigquery.filteredDataViewer` role to the members of
the grantee list. When you
[list a table's row-level access policies](https://docs.cloud.google.com/bigquery/docs/managing-row-level-security#list-policy)
in the Google Cloud console, this role is displayed in association with the members
of the policy's grantee list.

> [!CAUTION]
> **Caution:** Don't apply the `bigquery.filteredDataViewer` role directly to a resource through IAM. `bigquery.filteredDataViewer` is a system-managed role. Grant the role only by using row-level access policies. For more information, see [best practices for row-level security](https://docs.cloud.google.com/bigquery/docs/best-practices-row-level-security#filtered-data-viewer).

### Create or update row-level access policies

When you set up row-level access on a table, you'll need at least two row
access policies:

- A policy that grants full access to the table. The first row access policy should grant access to users and groups that require full access to the data in the table for data maintenance or support. For example, your BigQuery administrators and service accounts that use DML statements to transform table data.
- A second policy that filters access. This policy uses filters based on business logic to grant access to specific groups.

To create or update a row-level access policy, use one of the following
DDL statements:

- The `CREATE ROW ACCESS POLICY` creates a new row-level access policy.

- The `CREATE ROW ACCESS POLICY IF NOT EXISTS` statement creates a new row-level
  access policy, if a row-level access policy with the same name does not
  already exist on the specified table.

- The `CREATE OR REPLACE ROW ACCESS POLICY` statement updates an existing
  row-level access policy with the same name on the specified table.

  > [!IMPORTANT]
  > **Key points to remember:**
  > - Each row-level access policy on a table must have a unique name.
  > - Like a `WHERE` clause, the `filter_expression` matches the data that you want to be visible to the members of the `grantee_list`.
  > - You can combine a series of users and groups in the `grantee_list` list, if they are comma-separated and quoted separately.
  > - All identities in the `grantee_list` must exist. If any identity does not exist, the policy is not created and the statement fails.
  > - You cannot apply row-level access policies on [JSON columns](https://docs.cloud.google.com/bigquery/docs/json-data). To learn about additional limitations for row-level security, see [Limitations](https://docs.cloud.google.com/bigquery/docs/row-level-security-intro#limitations)

### Examples

The following examples show you how to create and update row access policies for
different types of [principal identifiers](https://docs.cloud.google.com/iam/docs/principal-identifiers#allow)
including Google Accounts and federated identities. For more information
on federated identities, see [Workload identity federation](https://docs.cloud.google.com/iam/docs/workload-identity-federation).

#### Create a new policy and grant access to a Google Account

Create a new row access policy. Access to the table is restricted to the
user `abc@example.com`. Only the rows where `region = 'APAC'` are
visible:

```googlesql
CREATE ROW ACCESS POLICY apac_filter
ON project.dataset.my_table
GRANT TO ('user:abc@example.com')
FILTER USING (region = 'APAC');
```

#### Create a new policy and grant access to a single identity in a workforce identity pool

Create a new row access policy. Access to the table is restricted to a single
identity in a workforce identity pool using this format:
`principal://iam.googleapis.com/locations/global/workforcePools/POOL_ID/subject/IDENTITY`.
Only the rows where `region = 'APAC'` are visible:

```googlesql
CREATE ROW ACCESS POLICY apac_filter
ON project.dataset.my_table
GRANT TO ('principal://iam.googleapis.com/locations/global/workforcePools/example-contractors/subject/abc@example.com')
FILTER USING (region = 'APAC');
```

#### Update a policy to grant access to a service account

Update the `apac_filter` access policy to apply to the service account
`example@exampleproject.iam.gserviceaccount.com`:

```googlesql
CREATE OR REPLACE ROW ACCESS POLICY apac_filter
ON project.dataset.my_table
GRANT TO ('serviceAccount:example@exampleproject.iam.gserviceaccount.com')
FILTER USING (region = 'APAC');
```

#### Create a policy and grant access to users and groups

Create a row access policy that grants access to a user and two groups:

```googlesql
CREATE ROW ACCESS POLICY sales_us_filter
ON project.dataset.my_table
GRANT TO ('user:john@example.com',
          'group:sales-us@example.com',
          'group:sales-managers@example.com')
FILTER USING (region = 'US');
```

#### Create a policy and grant access to workforce identities in groups

Create a row access policy that grants access to all workforce identities in
groups using this format
`principal://iam.googleapis.com/locations/global/workforcePools/POOL_ID/subject/IDENTITY`:

```googlesql
CREATE ROW ACCESS POLICY sales_us_filter
ON project.dataset.my_table
GRANT TO ('principal://iam.googleapis.com/locations/global/workforcePools/example-contractors/subject/sales-us@example.com',
          'principal://iam.googleapis.com/locations/global/workforcePools/example-contractors/subject/sales-managers@example.com')
FILTER USING (region = 'US');
```

#### Create a policy and grant access to all authenticated users

Create a row access policy with `allAuthenticatedUsers` as the grantees:

```googlesql
CREATE ROW ACCESS POLICY us_filter
ON project.dataset.my_table
GRANT TO ('allAuthenticatedUsers')
FILTER USING (region = 'US');
```

#### Create a policy and filter based on the current user

Create a row access policy with a filter based on the current user:

```googlesql
CREATE ROW ACCESS POLICY my_row_filter
ON dataset.my_table
GRANT TO ('domain:example.com')
FILTER USING (email = SESSION_USER());
```

#### Create a policy and filter on a column

Create a row access policy with a filter on a column with an
`ARRAY` type:

```googlesql
CREATE ROW ACCESS POLICY my_reports_filter
ON project.dataset.my_table
GRANT TO ('domain:example.com')
FILTER USING (SESSION_USER() IN UNNEST(reporting_chain));
```

#### Create a policy and use a region comparison

Create a row access policy with a subquery to replace multiple policies with
a region comparison configured per user:

Consider the following table, `lookup_table`:

```
+---+---+
|      email      |    region    |
+---+---+
| xyz@example.com | europe-west1 |
| abc@example.com | us-west1     |
| abc@example.com | us-west2     |
+---+---+
```

```googlesql
CREATE OR REPLACE ROW ACCESS POLICY apac_filter
ON project.dataset.my_table
GRANT TO ('domain:example.com')
FILTER USING (region IN (
    SELECT
      region
    FROM
      lookup_table
    WHERE
      email = SESSION_USER()));
```

Using the subquery on `lookup_table` lets you avoid creating multiple row access
policies. For example, the preceding statement yields the same result as the
following, with fewer queries:

```googlesql
CREATE OR REPLACE ROW ACCESS POLICY us_filter
ON project.dataset.my_table
GRANT TO ('user:abc@example.com')
FILTER USING (region IN ('us-west1', 'us-west2'));

CREATE OR REPLACE ROW ACCESS POLICY eu_filter
ON project.dataset.my_table
GRANT TO ('user:xyz@example.com')
FILTER USING (region = 'europe-west1');
```

For more information on the syntax and available options, see the
[`CREATE ROW ACCESS POLICY` DDL statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_row_access_policy_statement)
reference.

## Combine row-level access policies

If two or more row-level access policies grant a user or group access to the
same table, then the user or group has access to all of the data covered by any
of the policies.
For example, the following policies grant the user
`abc@example.com` access to specified rows in the `my_table` table:

```googlesql
CREATE ROW ACCESS POLICY shoes
ON project.dataset.my_table
GRANT TO ('user:abc@example.com')
FILTER USING (product_category = 'shoes');
```

```googlesql
CREATE OR REPLACE ROW ACCESS POLICY blue_products
ON project.dataset.my_table
GRANT TO ('user:abc@example.com')
FILTER USING (color = 'blue');
```

In the preceding example, the user `abc@example.com` has access to the rows
in the `my_table` table
that have the `product_category` field set to `shoes`, and `abc@example.com`
also has access to the rows that have the `color` field set to `blue`.
For example, `abc@example.com` would be able to access rows with information
about red shoes and blue cars.

This access is equivalent to the access provided by the following single
row-level access policy:

```googlesql
CREATE ROW ACCESS POLICY shoes_and_blue_products
ON project.dataset.my_table
GRANT TO ('user:abc@example.com')
FILTER USING (product_category = 'shoes' OR color = 'blue');
```

On the other hand, to specify access that is dependent on more than one
condition being true, use a filter
with an `AND` operator. For example, the following row-level access policy
grants `abc@example.com` access only to rows that have both the
`product_category` field set to `shoes` and the `color` field set to `blue`:

```googlesql
CREATE ROW ACCESS POLICY blue_shoes
ON project.dataset.my_table
GRANT TO ('user:abc@example.com')
FILTER USING (product_category = 'shoes' AND color = 'blue');
```

With the preceding row-level access policy, `abc@example.com` would be able to
access information about blue shoes, but not about red shoes or blue cars.

## List table row-level access policies

You can list and view all the row-level access policies on a table
by using the Google Cloud console, bq command-line tool, or `RowAccessPolicies.List` API
method.

### Required permissions

To list row-level access policies on a BigQuery table, you need
the `bigquery.rowAccessPolicies.list` IAM permission.

To view the members of a row-level access policy on a BigQuery
table, you need the `bigquery.rowAccessPolicies.getIamPolicy` IAM
permission.

Each of the following predefined IAM roles includes the
permissions that you need in order to list and view row-level access policies:

- `roles/bigquery.admin`
- `roles/bigquery.dataOwner`

For more information about IAM roles and permissions in
BigQuery, see
[Predefined roles and permissions](https://docs.cloud.google.com/bigquery/access-control).

### List table row-level access policies

To list row-level access policies, do the following:

### Console

1. To view row-level access policies, go to the BigQuery page
   in the Google Cloud console.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. Click the table name to see its details, and then click
   **View row access policies**.

   ![View row access policies](https://docs.cloud.google.com/static/bigquery/images/view-row-access-policies-console.png)
3. When the **Row access policies** panel opens, you see a list of all
   the row-level access policies on the table, by name, and the
   `filter_expression` for each policy.

   ![Row access policies detail](https://docs.cloud.google.com/static/bigquery/images/view-row-access-policies-detail.png)
4. To see all the roles and users affected by a row-level access policy,
   click **VIEW** next to the policy. For example, in the following image,
   you can see in the **View permissions** panel that members of the grantee
   list have the
   [`bigquery.filteredDataViewer` role](https://docs.cloud.google.com/bigquery/docs/managing-row-level-security#filtered-data-viewer-role).

   ![Row access policies detail](https://docs.cloud.google.com/static/bigquery/images/view-row-access-policy-permissions.png)

   > [!IMPORTANT]
   > **Important:** adding members to a policy and removing members from a policy are only supported using DDL statements.

### bq

Enter the `bq ls` command and supply the `--row_access_policies` flag.
The dataset and table names are required.

        bq ls --row_access_policies dataset.table

For example, the following command lists information about the row-level
access policies on a table named `my_table` in a dataset with the ID
`my_dataset`:

        bq ls --row_access_policies my_dataset.my_table

### API

Use the
[`RowAccessPolicies.List` method](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/rowAccessPolicies/list)
in the REST API reference section.

## Delete row-level access policies

You can delete one or all row-level access policies on a table by using a DDL
statement, if you have the permissions to do so.

### Required permissions

To drop a row-level access policy, you need the following IAM
permissions:

- `bigquery.rowAccessPolicies.delete`
- `bigquery.rowAccessPolicies.setIamPolicy`
- `bigquery.jobs.create` (to run the DDL query job)

To drop all the row-level access policies on a table at the same time, you need
the following IAM permissions:

- `bigquery.rowAccessPolicies.delete`
- `bigquery.rowAccessPolicies.setIamPolicy`
- `bigquery.rowAccessPolicies.list`
- `bigquery.jobs.create` (to run the DDL query job)

Each of the following predefined IAM roles includes the
permissions that you need in order to delete row-level access policies:

- `roles/bigquery.admin`
- `roles/bigquery.dataOwner`

For more information about IAM roles and permissions in
BigQuery, see
[Predefined roles and permissions](https://docs.cloud.google.com/bigquery/access-control).

### Delete row-level access policies

To delete a row access policy from a table, use the following
DDL statements:

- The `DROP ROW ACCESS POLICY` statement deletes a row-level access policy on
  the specified table.

- The `DROP ROW ACCESS POLICY IF EXISTS` statement deletes a row-level access
  policy if the row access policy exists on the specified table.

- The `DROP ALL ROW ACCESS POLICIES` statement deletes all row-level access
  policies on the specified table.

> [!IMPORTANT]
> **Important:** You cannot delete the last row-level access policy from a table using `DROP ROW ACCESS POLICY`. Attempting to do so results in an error. To delete the last row-level access policy on a table, you must use `DROP ALL ROW
> ACCESS POLICIES` instead. For more information about dropping the last row-level access policy on a table, see [Best practices for row-level security](https://docs.cloud.google.com/bigquery/docs/best-practices-row-level-security#avoid_inadvertent_access_when_re-creating_row-level_access_policies).

### Examples

Delete a row-level access policy from a table:

```googlesql
DROP ROW ACCESS POLICY my_row_filter ON project.dataset.my_table;
```

Delete all the row-level access policies from a table:

```googlesql
DROP ALL ROW ACCESS POLICIES ON project.dataset.my_table;
```

For more information about deleting a row-level access policy, see the
[`DROP ROW ACCESS POLICY` DDL statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#drop_row_access_policy_statement)
reference.

## Query tables with row access policies

A user must first have access to a BigQuery table to be able to
query it, even if they are on the `grantee_list` of a row access policy on
that table. Without that permission, the query fails with an `access
denied` error.

### Required permissions

To query a BigQuery table with row-level access policies, you
must have the `bigquery.tables.getData` permission on the table. You also need
the `bigquery.rowAccessPolicies.getFilteredData` permission.

To gain these permissions with predefined roles, you need to be granted the
[`roles/bigquery.dataViewer`](https://docs.cloud.google.com/bigquery/docs/access-control#bigquery.dataViewer)
role on the table using IAM, and you must be granted the
[`roles/bigquery.filteredDataViewer`](https://docs.cloud.google.com/bigquery/docs/managing-row-level-security#filtered-data-viewer-role)
IAM role on the table through the row-level access policy.

> [!CAUTION]
> **Caution:** Don't apply the `bigquery.filteredDataViewer` role directly to a resource through IAM. `bigquery.filteredDataViewer` is a system-managed role. Grant the role only by using row-level access policies. For more information, see [best practices for row-level security](https://docs.cloud.google.com/bigquery/docs/best-practices-row-level-security#filtered-data-viewer).

You must have the `datacatalog.categories.fineGrainedGet` permission on all
relevant columns with [column-level
security](https://docs.cloud.google.com/bigquery/docs/column-level-security-intro). To gain this permission
with predefined roles, you need the `datacatalog.categoryFineGrainedReader`
role.

### View query results

In the Google Cloud console, when you query a table with a row-level
access policy, BigQuery displays a banner notice indicating
that your results might be filtered by a row-level access policy. This
notice displays even if you are a member of the grantee list for the policy.

![Query result on table with row-level access policy](https://docs.cloud.google.com/static/bigquery/images/query-results-row-level-security.png)

### Job statistics

When you query a table with a row-level access policy using the Job API,
BigQuery indicates whether the query reads any tables with
row access policies in the `Job` response object:

#### Example

This `Job` object response has been truncated for simplicity:

    {
      "configuration": {
        "jobType": "QUERY",
        "query": {
          "priority": "INTERACTIVE",
          "query": "SELECT * FROM dataset.table",
          "useLegacySql": false
        }
      },
      ...
      "statistics": {
        ...
        rowLevelSecurityStatistics: {
          rowLevelSecurityApplied: true
        },
        ...
      },
      "status": {
        "state": "DONE"
      },
      ...
    }

## What's next

- For information about how row-level security works with other
  BigQuery features and services, see
  [Using row level security with other BigQuery features](https://docs.cloud.google.com/bigquery/docs/using-row-level-security-with-features).

- For information about best practices for row-level security, see
  [Best Practices for row-level security in BigQuery](https://docs.cloud.google.com/bigquery/docs/best-practices-row-level-security).