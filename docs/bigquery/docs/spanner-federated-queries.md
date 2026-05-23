# Spanner federated queries

As a data analyst, you can query data in Spanner from BigQuery
using [federated queries](https://docs.cloud.google.com/bigquery/docs/federated-queries-intro).

BigQuery Spanner federation lets
BigQuery query data residing in Spanner in
real time, without copying or moving data.

You can query Spanner data in two ways:

- Create a Spanner external dataset.
- Use an [`EXTERNAL_QUERY`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/federated_query_functions#external_query) function.

## Understand roles and permissions

When you query Spanner from BigQuery, you encounter two
distinct types of roles that manage access at different levels.

- **IAM roles:** These roles govern access to
  Google Cloud resources, including Spanner
  instances and databases. They determine which principals can access the
  Spanner service and perform actions at the instance or
  database level, such as connecting, reading data, or administering. You
  manage Identity and Access Management (IAM) roles through the IAM console or
  the Google Cloud CLI. Examples include `roles/bigquery.connectionUser` and
  `roles/spanner.databaseReader`. For more information, see
  [Spanner IAM roles](https://docs.cloud.google.com/spanner/docs/iam) and
  [granting permissions](https://docs.cloud.google.com/spanner/docs/grant-permissions).

- **Spanner database roles:** These roles are defined inside a
  Spanner database using DDL statements such as
  [`CREATE ROLE`](https://docs.cloud.google.com/spanner/docs/reference/standard-sql/data-definition-language#create_role)
  and
  [`GRANT`](https://docs.cloud.google.com/spanner/docs/reference/standard-sql/data-definition-language#grant_statement).
  They control granular access to specific schema objects, such as tables,
  columns, and views, inside the database. This is part of
  [fine-grained access control](https://docs.cloud.google.com/spanner/docs/fgac-about) (FGAC). You use a
  database role if your organization implements FGAC to manage permissions
  inside the database.

### Determine if you're an FGAC user

To determine the correct permissions to request, you need to determine if you're
an FGAC user. To do this, ask your Spanner database administrator
whether your access to the Spanner database is managed through
fine-grained access control.

You are likely an FGAC user if your administrator grants your account
permissions by assigning it to a specific Spanner database role
(for example, by granting your account the IAM role
`roles/spanner.databaseRoleUser` on a database role resource). If this is the
case, you need to know the name of the database roles that you can use. You must
configure the BigQuery connection to use one of these database
roles.

You are likely not an FGAC user if your administrator grants your account
broader database-level IAM roles, such as
`roles/spanner.databaseReader`. In this case, you don't need to use a specific
database role when connecting.

### Compare role application

Although IAM controls access to the database resource itself,
Spanner database roles control permissions for the objects within
that database.

To use an FGAC database role, you typically need the following permissions:

- The `spanner.databases.useRoleBasedAccess` IAM permission, which is often granted through the `roles/spanner.fineGrainedAccessUser` role.
- Permission to use the specific database role, which is granted through the `roles/spanner.databaseRoleUser` role with an IAM condition.

For more information about setting up these permissions, see
[Configure FGAC](https://docs.cloud.google.com/spanner/docs/configure-fgac).

## Use external datasets

The simplest way to query Spanner tables is to [create an external dataset](https://docs.cloud.google.com/bigquery/docs/spanner-external-datasets). After you create the external dataset, your tables from the corresponding Spanner database are visible in BigQuery and you can use them in your queries---for example, in joins, unions, or subqueries. However, no data is moved from Spanner to BigQuery storage.

You don't need to create a connection to query Spanner data if you create an external dataset.

## Use `EXTERNAL_QUERY` function

As with other federated databases, you can also query Spanner
data with an [`EXTERNAL_QUERY`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/federated_query_functions#external_query)
function. This might be useful if you want to have more control over the connection parameters.

### Before you begin

- Ensure that your BigQuery administrator has created a [Spanner connection](https://docs.cloud.google.com/bigquery/docs/connect-to-spanner#create-spanner-connection) and [shared](https://docs.cloud.google.com/bigquery/docs/connect-to-spanner#share_connections) it with you. See [Choose the right connection](https://docs.cloud.google.com/bigquery/docs/spanner-federated-queries#right-connection).
- To get the permissions that you need to query a Spanner instance, ask your administrator to grant you the BigQuery Connection User (`roles/bigquery.connectionUser`) IAM role on the connection. You also need appropriate permissions on the Spanner database that depend on [whether you're an FGAC user](https://docs.cloud.google.com/bigquery/docs/spanner-federated-queries#determine-fgac-user).
  - If you are an fine-grained access control user:
    - You need the IAM roles required to use FGAC. These roles are typically `roles/spanner.fineGrainedAccessUser` and `roles/spanner.databaseRoleUser`. The roles are used with a condition that specifies the database role.
    - The Spanner database role that you specify in the connection must have the `SELECT` privilege on all schema objects that your queries reference. Your database administrator grants privileges by using the [`GRANT`](https://docs.cloud.google.com/spanner/docs/reference/standard-sql/data-definition-language#grant_statement) DDL statement (or the [PostgreSQL
      equivalent](https://docs.cloud.google.com/spanner/docs/reference/postgresql/data-definition-language#grant_statement)).
  - If you aren't an fine-grained access control user, you need the Spanner Database Reader (`roles/spanner.databaseReader`) IAM role on the database.

  For information about granting IAM roles, see
  [Manage access to projects, folders, and organizations](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).

### Choose the right connection

If you are a Spanner fine-grained access control user, when you run a
federated query with an [`EXTERNAL_QUERY`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/federated_query_functions#external_query) function, you must use a
Spanner connection that specifies a database role. This database
role is part of the FGAC configuration within the Spanner
database, separate from your IAM roles. Then all
queries that you run with this connection use the permissions granted to that
database role.

If you use a connection that doesn't specify a database role, you must have the
IAM roles indicated in [Before you begin](https://docs.cloud.google.com/bigquery/docs/spanner-federated-queries#begin).

### Query data

To send a federated query to Spanner from a GoogleSQL query, use the
[`EXTERNAL_QUERY`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/federated_query_functions#external_query)
function.

Formulate your Spanner query in either GoogleSQL or
PostgreSQL, depending on the specified dialect of the database.

The following example makes a federated query to a Spanner
database named `orders` and joins the results with a BigQuery
table named `mydataset.customers`:

```googlesql
SELECT c.customer_id, c.name, rq.first_order_date
FROM mydataset.customers AS c
LEFT OUTER JOIN EXTERNAL_QUERY(
  'my-project.us.example-db',
  '''SELECT customer_id, MIN(order_date) AS first_order_date
  FROM orders
  GROUP BY customer_id''') AS rq
  ON rq.customer_id = c.customer_id
GROUP BY c.customer_id, c.name, rq.first_order_date;
```

## Spanner Data Boost

Data Boost is a fully managed, serverless feature that provides
independent compute resources for supported Spanner workloads.
Data Boost lets you run analytics queries and data exports
with minimal effect on existing workloads on the provisioned
Spanner instance. Data Boost lets you run federated queries
with independent compute capacity separate from your provisioned instances to
avoid affecting existing workloads on Spanner. Data Boost is
most useful when you run complex ad hoc queries, or when you want to process
large amounts of data without affecting the existing Spanner
workload. Running federated queries with Data Boost can lead to
significantly lower CPU consumption, and in some cases, lower query latency.

### Before you begin


To get the permission that
you need to enable access to Data Boost,

ask your administrator to grant you the
[Cloud Spanner Database Reader with DataBoost](https://docs.cloud.google.com/iam/docs/roles-permissions/spanner#spanner.databaseReaderWithDataBoost) (`roles/spanner.databaseReaderWithDataBoost`) IAM role on the Spanner database.


For more information about granting roles, see [Manage access to projects, folders, and organizations](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).


This predefined role contains the
`spanner.databases.useDataBoost`
permission,
which is required to
enable access to Data Boost.


You might also be able to get
this permission
with [custom roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles) or
other [predefined roles](https://docs.cloud.google.com/iam/docs/roles-overview#predefined).

The `roles/spanner.databaseReaderWithDataBoost` IAM role grants permission to use Data Boost. This role is required in addition to the base permissions needed to read data, such as `roles/spanner.databaseReader` for non-FGAC users or the appropriate fine-grained access control permissions.

Using external datasets with Spanner always uses Data Boost and thus requires the `spanner.databases.useDataBoost` permission.

### Enable Data Boost

When using external datasets, Data Boost is always used and you don't have to enable it manually.

If you want to use Data Boost for your `EXTERNAL_QUERY` queries, you must enable it when [creating a connection](https://docs.cloud.google.com/bigquery/docs/connect-to-spanner) that is used by your query.

## Read data in parallel

Spanner can divide certain queries into smaller pieces, or
partitions, and fetch the partitions in parallel. For more information, including a list of limitations, see
[Read data in parallel](https://docs.cloud.google.com/spanner/docs/reads#read_data_in_parallel) in the
Spanner documentation.

To view the query execution plan for a
Spanner query, see
[Understand how Spanner executes queries](https://docs.cloud.google.com/spanner/docs/sql-best-practices#how-execute-queries).

When running federated queries with external datasets, the "Read data in parallel" option is always used.

To enable parallel reads when using the
[`EXTERNAL_QUERY`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/federated_query_functions#external_query),
enable it when you
[create the connection](https://docs.cloud.google.com/bigquery/docs/connect-to-spanner).

## Manage query execution priority

When you run federated queries with an `EXTERNAL_QUERY` function, you can assign priority (`high`, `medium`, or `low`) to individual queries by specifying the `query_execution_priority` option:

```googlesql
SELECT *
FROM EXTERNAL_QUERY(
  'my-project.us.example-db',
  '''SELECT customer_id, MIN(order_date) AS first_order_date
  FROM orders
  GROUP BY customer_id''',
  '{"query_execution_priority":"high"}');
```

The default priority is `medium`.

Queries with priority `high` compete with transactional traffic. Queries with
priority `low` are best effort, and might get preempted by background load, for
example, scheduled backups.

> [!CAUTION]
> **Caution:** Queries with `low` priority fall below queries such as backup jobs that might never complete within the timeouts for BigQuery.

When running federated queries with external datasets, all queries always have `medium` priority.

## View a Spanner table schema

If you use external datasets, your Spanner tables are visible
directly in BigQuery Studio, and you can see their schemas.

However, you can also see the schemas without defining external datasets. You
can also use the `EXTERNAL_QUERY` function to query `information_schema` views
to access database metadata. The following example returns information about the
columns in the table `MyTable`:

### Google SQL database

```googlesql
SELECT *
FROM EXTERNAL_QUERY(
  'my-project.us.example-db',
  '''SELECT t.column_name, t.spanner_type, t.is_nullable
    FROM information_schema.columns AS t
    WHERE
      t.table_catalog = ''
      AND t.table_schema = ''
     AND t.table_name = 'MyTable'
    ORDER BY t.ordinal_position
  ''');
```

### PostgreSQL database

```googlesql
SELECT * from EXTERNAL_QUERY(
 'my-project.us.postgresql.example-db',
  '''SELECT t.column_name, t.data_type, t.is_nullable
    FROM information_schema.columns AS t
    WHERE
      t.table_schema = 'public' and t.table_name='MyTable'
    ORDER BY t.ordinal_position
  ''');
```

For more information, see the following information schema references in the
Spanner documentation:

- [GoogleSQL information schema](https://docs.cloud.google.com/spanner/docs/information-schema)
- [PostgreSQL information schema](https://docs.cloud.google.com/spanner/docs/information-schema-pg)

## Pricing

- On the BigQuery side, standard [federated query pricing](https://docs.cloud.google.com/bigquery/docs/federated-queries-intro#pricing) applies.
- On the Spanner side, queries are subject to [Spanner pricing](https://cloud.google.com/spanner/pricing).

## Cross region queries

BigQuery supports federated queries where Spanner
instances and BigQuery datasets are in different regions. These
queries incur an additional Spanner data transfer charge.
For more information, see [Spanner pricing](https://cloud.google.com/spanner/pricing#network).

You are charged for the data transfer, based on the following [SKUs](https://cloud.google.com/skus/sku-groups/cloud-spanner):

- Network Intra-region Cross-Zone Data Transfer Out
- Network Inter-Region Data Transfer Out to the Same Continent
- Network Inter-Region Data Transfer Out to a Different Continent

Data transfer is charged based on the BigQuery region
you run the query in and the nearest Spanner region that
has read-write or read-only replicas.

For BigQuery multi-region configurations
(`US` or `EU`), data transfer costs from Spanner are determined as follows:

- BigQuery `US` multi-region: Spanner region `us-central1`
- BigQuery `EU` multi-region: Spanner region `europe-west1`

For example:

- BigQuery (`US` multi-region) and Spanner (`us-central1`): Costs apply for data transfer within the same region.
- BigQuery (`US` multi-region) and Spanner (`us-west4`): Costs apply for data transfer between regions within the same continent.

## Troubleshooting

This section helps you troubleshoot issues you might encounter when sending
a federated query to Spanner.

Issue: Query is not root partitionable.
:   **Resolution:** If you configure the connection to read data in parallel,
    either the first operator in the query execution plan must be a distributed
    union, or your execution plan must not have any distributed unions. To resolve
    this error, view the query execution plan and rewrite the query. For more
    information, see
    [Understand how Spanner executes queries](https://docs.cloud.google.com/spanner/docs/sql-best-practices#how-execute-queries).

Issue: Deadline exceeded.
:   **Resolution:** Select the option to
    [read data in parallel](https://docs.cloud.google.com/bigquery/docs/spanner-federated-queries#read_data_in_parallel) and rewrite the query to be root
    partitionable. For more information, see
    [Understand how Spanner executes queries](https://docs.cloud.google.com/spanner/docs/sql-best-practices#how-execute-queries).

## What's next

- Learn about [creating Spanner external datasets](https://docs.cloud.google.com/bigquery/docs/spanner-external-datasets)
- Learn about [federated queries](https://docs.cloud.google.com/bigquery/docs/federated-queries-intro).
- Learn about [Spanner to BigQuery data type mapping](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/federated_query_functions#spanner-mapping).