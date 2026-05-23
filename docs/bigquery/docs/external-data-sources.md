# Introduction to external data sources

This page provides an overview of querying data stored outside of
BigQuery.

An external data source is a data source that you can query directly from
BigQuery, even though the data is not stored in
BigQuery storage. For example, you might have data in a
different Google Cloud database, in files in Cloud Storage, or in a
different cloud product altogether that you would like to analyze in
BigQuery, but that you aren't prepared to migrate.

Use cases for external data sources include the following:

- For extract-load-transform (ELT) workloads, loading and cleaning your data in one pass and writing the cleaned result into BigQuery storage, by using a `CREATE TABLE ... AS SELECT` query.
- Joining BigQuery tables with frequently changing data from an external data source. By querying the external data source directly, you don't need to reload the data into BigQuery storage every time it changes.

BigQuery has two different mechanisms for querying external
data: external tables and federated queries.

## External tables

External tables are similar to standard BigQuery tables, in
that these tables store their metadata and schema in BigQuery
storage. However, their data resides in an external source.

External tables are contained inside a dataset, and you manage them in
the same way that you manage a standard
BigQuery table. For example, you can
[view the table's properties](https://docs.cloud.google.com/bigquery/docs/tables#get_information_about_tables),
[set access controls](https://docs.cloud.google.com/bigquery/docs/table-access-controls), and so
forth. You can query these tables and in most cases you can join them with
other tables.

There are four kinds of external tables:

- BigLake tables
- BigQuery Omni tables
- Object tables
- Non-BigLake external tables

### BigLake tables

BigLake tables let you query structured data in
external data stores with access delegation. Access delegation
decouples access to the BigLake table from access to
the underlying data store. An
[external connection](https://docs.cloud.google.com/bigquery/docs/connections-api-intro)
associated with a service account is used to connect to the data store. Because
the service account handles retrieving data from the data store, you only have
to grant users access to the BigLake table. This lets you enforce
fine-grained security at the table level, including
[row-level](https://docs.cloud.google.com/bigquery/docs/row-level-security-intro) and
[column-level](https://docs.cloud.google.com/bigquery/docs/column-level-security-intro) security. For
BigLake tables based on Cloud Storage, you can also use
[dynamic data masking](https://docs.cloud.google.com/bigquery/docs/column-data-masking). To learn more about
multi-cloud analytic solutions using BigLake tables with
Amazon S3 or Blob Storage data, see
[BigQuery Omni](https://docs.cloud.google.com/bigquery/docs/omni-introduction).

For more information, see
[Introduction to BigLake tables](https://docs.cloud.google.com/bigquery/docs/biglake-intro).

### Object tables

Object tables let you analyze unstructured data in
Cloud Storage. You can perform analysis with remote functions or
perform inference by using BigQuery ML, and then join the results of
these operations with the rest of your structured data in BigQuery.

Like BigLake tables, object tables use access delegation,
which decouples access to the object table from access to the
Cloud Storage objects. An
[external connection](https://docs.cloud.google.com/bigquery/docs/working-with-connections)
associated with a service account is used to connect to Cloud Storage,
so you only have to grant users access to the object table. This lets you
enforce [row-level](https://docs.cloud.google.com/bigquery/docs/row-level-security-intro) security and manage
which objects users have access to.

For more information, see
[Introduction to object tables](https://docs.cloud.google.com/bigquery/docs/object-table-introduction).

### Non-BigLake external tables

Non-BigLake external tables let you query structured data
in external data stores. To query a non-BigLake external
table, you must have permissions to both the external table and the
external data source. For example, to query a non-BigLake
external table that uses a data source in Cloud Storage,
you must have the following permissions:

- `bigquery.tables.getData`
- `bigquery.jobs.create`
- `storage.buckets.get`
- `storage.objects.get`

For more information, see
[Introduction to external tables](https://docs.cloud.google.com/bigquery/docs/external-tables).

## Federated queries


Federated queries let you send a query statement to AlloyDB, Spanner, or Cloud SQL databases
and get the result back as a temporary table. Federated queries use the
BigQuery Connection API to establish a connection with AlloyDB, Spanner, or Cloud SQL.
In your query, you use the `EXTERNAL_QUERY` function to send a
query statement to the external database, using that database's SQL dialect.
The results are converted to GoogleSQL data types.

For more information, see
[Introduction to federated queries](https://docs.cloud.google.com/bigquery/docs/federated-queries-intro).

## External data source feature comparison

The following table compares the behavior of external data sources:

|   | **BigLake tables** | **Object tables** | **Non-BigLake external tables** | **Federated queries** |
|---|---|---|---|---|
| **Uses access delegation** | Yes, through a service account | Yes, through a service account | No | Yes, through a database user account (Cloud SQL only) |
| **Can be based on multiple source URIs** | Yes | Yes | Yes (Cloud Storage only) | Not applicable |
| **Row mapping** | Rows represent file content | Rows represent file metadata | Rows represent file content | Not applicable |
| **Accessible by other data processing tools by using connectors** | Yes (Cloud Storage only) | No | Yes | Not applicable |
| **Can be joined to other BigQuery tables** | Yes (Cloud Storage only) | Yes | Yes | Yes |
| **Can be accessed as a temporary table** | Yes (Cloud Storage only) | No | Yes | Yes |
| **Works with Amazon S3** | [Yes](https://docs.cloud.google.com/bigquery/docs/omni-aws-introduction) | No | No | No |
| **Works with Azure Storage** | [Yes](https://docs.cloud.google.com/bigquery/docs/omni-azure-introduction) | No | No | No |
| **Works with Bigtable** | No | No | [Yes](https://docs.cloud.google.com/bigquery/docs/external-data-bigtable) | No |
| **Works with Spanner** | No | No | No | [Yes](https://docs.cloud.google.com/bigquery/docs/spanner-federated-queries) |
| **Works with Cloud SQL** | No | No | No | [Yes](https://docs.cloud.google.com/bigquery/docs/cloud-sql-federated-queries) |
| **Works with Google Drive** | No | No | [Yes](https://docs.cloud.google.com/bigquery/docs/external-data-drive) | No |
| **Works with Cloud Storage** | [Yes](https://docs.cloud.google.com/bigquery/docs/query-cloud-storage-using-biglake) | [Yes](https://docs.cloud.google.com/bigquery/docs/object-tables) | [Yes](https://docs.cloud.google.com/bigquery/docs/external-data-cloud-storage) | No |

## What's next

- Learn more about [BigLake tables](https://docs.cloud.google.com/bigquery/docs/biglake-intro).
- Learn more about [object tables](https://docs.cloud.google.com/bigquery/docs/object-table-introduction)
- Learn more about [external tables](https://docs.cloud.google.com/bigquery/docs/external-tables).
- Learn more about [federated queries](https://docs.cloud.google.com/bigquery/docs/federated-queries-intro).
- Learn about [BigQuery pricing](https://cloud.google.com/bigquery/pricing#on_demand_pricing).