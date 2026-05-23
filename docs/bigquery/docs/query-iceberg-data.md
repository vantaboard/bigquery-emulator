# Query Apache Iceberg data

This document describes how to query data stored in an
[Apache Iceberg managed table](https://docs.cloud.google.com/bigquery/docs/iceberg-tables).

## Required roles

To query Apache Iceberg managed tables, ensure
that the caller of the BigQuery API has the following roles:

- BigQuery Connection User (`roles/bigquery.connectionUser`)
- BigQuery Data Viewer (`roles/bigquery.dataViewer`)
- BigQuery User (`roles/bigquery.user`)

The caller can be your account, a
[Spark connection service account](https://docs.cloud.google.com/bigquery/docs/connect-to-spark#create-spark-connection),
or a
[Cloud resource connection service account](https://docs.cloud.google.com/bigquery/docs/create-cloud-resource-connection#create-cloud-resource-connection).
Depending on your permissions, you can
grant these roles to yourself or ask your administrator
to grant them to you. For more information about granting roles, see
[Viewing the grantable roles on resources](https://docs.cloud.google.com/iam/docs/viewing-grantable-roles).

To see the exact permissions that are required to query
Spark BigLake tables, expand the
**Required permissions** section:

#### Required permissions

- `bigquery.connections.use`
- `bigquery.jobs.create`
- `bigquery.readsessions.create` (Only required if you are [reading data with the
  BigQuery Storage Read API](https://docs.cloud.google.com/bigquery/docs/reference/storage))
- `bigquery.tables.get`
- `bigquery.tables.getData`

You might also be able to get these permissions with [custom roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles)
or other [predefined roles](https://docs.cloud.google.com/iam/docs/roles-overview#predefined).

## Query Iceberg managed tables

After creating an Iceberg managed table, you can [query it using
GoogleSQL syntax](https://docs.cloud.google.com/bigquery/docs/running-queries), the same as if
it were a standard BigQuery table. For example, `SELECT field1, field2
FROM mydataset.my_iceberg_table;`.

## What's next

- Learn about [using SQL in BigQuery](https://docs.cloud.google.com/bigquery/docs/introduction-sql).
- Learn about [BigLake tables](https://docs.cloud.google.com/bigquery/docs/biglake-intro).
- Learn about [BigQuery quotas](https://docs.cloud.google.com/bigquery/quotas).