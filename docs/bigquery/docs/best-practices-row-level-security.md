# Best practices for row-level security in BigQuery

This document explains best practices when using
[row-level security](https://docs.cloud.google.com/bigquery/docs/row-level-security-intro).

Before you read this document, familiarize yourself with
row-level security by reading
[Introduction to BigQuery row-level security](https://docs.cloud.google.com/bigquery/docs/row-level-security-intro)
and
[Working with row-level security](https://docs.cloud.google.com/bigquery/docs/managing-row-level-security).

## Row access policy design considerations

When you set up row access policies on a table, you'll need at least two row
access policies:

- A policy that grants access to the table. The first row access policy should grant access to users and groups that require full access to the data in the table for data maintenance or support. For example, your BigQuery administrators and service accounts that use DML statements to transform table data.
- A second policy that uses filters based on business logic and are granted to specific groups.

For more information on setting up row access policies, see [Create or update a
row-level access policy](https://docs.cloud.google.com/bigquery/docs/managing-row-level-security#create_or_update_a_row-level_access_policy).

## Test service accounts with row access policies

You can use service account impersonation to test how row access policies are
applied. To test your row access policies by using a service account, do the
following:

1. [Create a service account](https://docs.cloud.google.com/iam/docs/service-account-overview#locations).
2. [Update the row access policy](https://docs.cloud.google.com/bigquery/docs/managing-row-level-security#create-policy) to grant the service account access. Alternatively, add the service account to the Google Group that is granted access by the row access policy.
3. Use [service account impersonation](https://docs.cloud.google.com/iam/docs/service-account-impersonation) to validate that the row access policy is working as intended.

## Restrict user permissions to limit side-channel attacks

> [!TIP]
> **Best practice:** Don't grant sensitive permissions to users who should only see filtered data.

A side-channel attack is a security attack based on information gained from the
system itself. An attacker with broader permissions than necessary can
mount side-channel attacks, and learn sensitive data by **observing** or
**searching** billing, logging, or system messages.

To mitigate such opportunities, BigQuery hides
sensitive statistics on all queries against tables with row-level security.
These sensitive statistics include the number of bytes and partitions processed,
the number of bytes billed, and the query plan stages.

*We recommend that admins should refrain from granting the following permissions
to users who should only see filtered data, to avoid giving access to sensitive
data.*

| **Permissions** | **Sensitive data** |
|---|---|
| Project Owner | Project owners can view bytes processed and related data only in audit logs. The billing metadata cannot be viewed from the job details. There's no specific permission or role to grant viewer access to this billing metadata. |
| BigQuery Data Edit, Owner, or Viewer roles | View error messages on queries. |
| Cloud Billing viewer permissions | View BigQuery billing. |

**Examples**

- Through repeated **observation** of query duration when querying tables with row-level access policies, a user could infer the values of rows that otherwise might be protected by row-level access policies. This type of attack requires many repeated attempts over a range of key values in partitioning or clustering columns. Even though there is inherent noise when observing or measuring query duration, with repeated attempts, an attacker could obtain a reliable estimate. *If you are sensitive to this level
  of protection, we recommend using separate tables to isolate rows with
  different access control requirements, instead.*
- An attacker could **search** for the bytes processed by a query by monitoring the errors that occur when the query job limits (such as maximum bytes billed or custom cost controls) are exceeded. However, this attack requires a high volume of queries.
- Through repeated queries and **observing** the BigQuery billing amount in Cloud Billing, a user could infer the values of rows that otherwise might be protected by row-level access policies. This type of attack requires many repeated attempts over a range of key values in partitioning or clustering columns. *If you are sensitive to this level of protection, we recommend that you limit access to billing data for
  queries.*

*We also recommend that admins monitor
Cloud Audit Logs(/bigquery/docs/reference/auditlogs)
for suspicious activity on tables with row-level security, such as unexpected
additions, modifications, and deletions of row-level access policies.*

## Restrict user permissions to limit data tampering

> [!TIP]
> **Best practice:** Don't grant table write permissions to users who should only see filtered data.

Users with write permissions to a table can insert data into the table with the
[`bq load` command](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#bq_load) or with
the BigQuery Storage Write API. This can allow the user with
write permissions to alter the query results of other users.

*We recommend that admins create separate Google groups for table write access
and row-level access policies. Users that should only see filtered table
results shouldn't have write access to the filtered table.*

## Avoid inadvertent access when re-creating row-level access policies

> [!TIP]
> **Best practice:** If there is only one row-level access policy on a table, don't recreate that row-level access policy with the `CREATE OR REPLACE` command. Instead, first remove all access to the table with table access controls, recreate the policies as needed, and then re-enable access.

When you add a row access policy on a table for the first time, you immediately
begin filtering data in query results. When you remove the last row-level access
policy on a table, even if you intend to only re-create the row-level access
policy, you may inadvertently grant unfiltered access to a wider-than-intended
audience.

*We recommend that admins pay special attention when recreating the last
row-level access policy on a table, by following these guidelines:*

1. First remove all access to the table, by using [table access controls](https://docs.cloud.google.com/bigquery/docs/control-access-to-resources-iam).
2. Remove all row-level access policies.
3. Re-create the row-level access policies.
4. Re-enable access to the table.

Alternatively, you can first create new row-level access policies on the table,
then delete the earlier row-level access policies that are no longer needed.

## Use row-level security only within organizations, not across organizations

> [!TIP]
> **Best practice:** Only use row-level security within your organization.

Don't use the row-level security feature across organizations, to help prevent
data leakage through side-channel attacks, and to maintain greater control over
access to sensitive data.

For subquery row-level access policies, create and search tables within
organizations or projects. This leads to better security and simpler ACL
configuration, as grantees must have the `bigquery.tables.getData` permission on
the target and referenced tables in policies, as well as any relevant
[column-level security](https://docs.cloud.google.com/bigquery/docs/column-level-security-intro) permissions.

*We recommend using row-level security feature for within-organization
security constraints only (such as for sharing data within an
organization/enterprise/company), and not for cross-organizational or public
security.*

**Example**

Outside of your organization, you have less control over who has access to data.
Within your organization, you can control who has been granted access to billing
information of queries against tables with row-level access policies. Billing
information is a vector for [side-channel attacks](https://docs.cloud.google.com/bigquery/docs/best-practices-row-level-security#limit-side-channel-attacks).

## Manage the `Filtered Data Viewer` role through row-level access policies

> [!TIP]
> **Best practice:** `bigquery.filteredDataViewer` is a system-managed role granted through row-level access policies. Manage the role only through row-level access policies. Don't apply the role through Identity and Access Management (IAM).

When you
[create a row-level access policy](https://docs.cloud.google.com/bigquery/docs/managing-row-level-security#create-policy),
the principals in the policy are automatically granted the
`bigquery.filteredDataViewer` role. You can only add or remove principals from
the access policy
[with a DDL statement](https://docs.cloud.google.com/bigquery/docs/managing-row-level-security#examples).

The `bigquery.filteredDataViewer` role *must not* be granted through
[IAM](https://docs.cloud.google.com/bigquery/access-control) to a higher-level resource, such
as a table, dataset, or project. Granting the role in this way lets users
view rows defined by *all* row-level access policies within that scope,
regardless of intended restrictions. While the union of row-level access policy
filters might not encompass the entire table, this practice poses a significant
security risk and undermines the purpose of row-level security.

We recommend managing the `bigquery.filteredDataViewer` role exclusively through
row-level access policies. This method ensures that principals are granted the
`bigquery.filteredDataViewer` role implicitly and correctly, respecting the
defined filter predicates for each policy.

## Performance impact of filters on partitioned columns

> [!TIP]
> **Best practice:** Try to avoid making row access policies that filter on clustered and partitioned columns.

Row-level access policy filters don't participate in query
[pruning on partitioned and clustered tables](https://docs.cloud.google.com/bigquery/docs/using-row-level-security-with-features#partitioned_and_clustered_tables).

If your row-level access policy names a partitioned column, your query does not
receive the performance benefits of query pruning.