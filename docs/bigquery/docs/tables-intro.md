# Introduction to tables

A BigQuery table contains individual records organized in rows. Each record is
composed of columns (also called *fields*).

Every table is defined by a *schema* that describes the column names, data types, and
other information. You can specify the schema of a table when it is created, or you can create a
table without a schema and declare the schema in the query job or load job that first populates it
with data.

Use the format `projectname.datasetname.tablename` to fully qualify a table
name when using GoogleSQL, or the format `projectname:datasetname.tablename`
to fully qualify a table name when using the bq command-line tool.

## Table types

The following sections describe the table types that BigQuery
supports.

- [Standard BigQuery tables](https://docs.cloud.google.com/bigquery/docs/tables-intro#standard-tables): structured data stored in BigQuery storage.
- [External tables](https://docs.cloud.google.com/bigquery/docs/tables-intro#external_tables): tables that reference data stored outside BigQuery.
- [Views](https://docs.cloud.google.com/bigquery/docs/tables-intro#views): logical tables that are created by using a SQL query.

### Standard BigQuery tables

Standard BigQuery tables contain structured data and are stored
in BigQuery storage in a columnar format. You can also store
references to unstructured data in standard tables by using struct columns
that adhere to the
[`ObjectRef`](https://docs.cloud.google.com/bigquery/docs/work-with-objectref)
format. For more information about working with `ObjectRef` values, see
[Specify ObjectRef columns in table schemas](https://docs.cloud.google.com/bigquery/docs/objectref-columns).

BigQuery has the following table types:

- Tables, which have a schema and every column in the schema has a data type.

  For information about how to create tables, see [Create tables](https://docs.cloud.google.com/bigquery/docs/tables#create-table).
- [Table clones](https://docs.cloud.google.com/bigquery/docs/table-clones-intro), which are
  lightweight, writeable copies of BigQuery tables.
  BigQuery only
  stores the delta between a table clone and its base table.

  For information about how to create table clone, see [Create table clones](https://docs.cloud.google.com/bigquery/docs/table-clones-create).
- [Table snapshots](https://docs.cloud.google.com/bigquery/docs/table-snapshots-intro), which are
  point-in-time copies of tables. They are read-only, but you can restore a table
  from a table snapshot. BigQuery stores bytes that are different
  between a snapshot and its base table, so a table snapshot typically uses less
  storage than a full copy of the table.

  For information about how to create table snapshots, see [Create table snapshots](https://docs.cloud.google.com/bigquery/docs/table-snapshots-create).

### External tables

External tables are stored outside of BigQuery storage and
refer to data that's stored outside of BigQuery. For more information, see
[Introduction to external data sources](https://docs.cloud.google.com/bigquery/docs/external-data-sources).
External tables include the following types:

- [BigLake tables](https://docs.cloud.google.com/bigquery/docs/biglake-intro), which reference
  structured data stored in data stores such as
  Cloud Storage, Amazon Simple Storage Service (Amazon S3), and Azure Blob Storage. These tables let you
  enforce fine-grained security at the table level.

  For information about how to create BigLake tables, see the
  following topics:
  - [Cloud Storage](https://docs.cloud.google.com/bigquery/docs/query-cloud-storage-using-biglake)
  - [Amazon S3](https://docs.cloud.google.com/bigquery/docs/omni-aws-create-external-table)
  - [Blob Storage](https://docs.cloud.google.com/bigquery/docs/omni-azure-create-external-table)
- [Object tables](https://docs.cloud.google.com/bigquery/docs/object-table-introduction), which
  reference unstructured data stored in data stores such as Cloud Storage.

  For information about how to create object tables, see [Create object tables](https://docs.cloud.google.com/bigquery/docs/object-tables).
- [Non-BigLake external tables](https://docs.cloud.google.com/bigquery/docs/external-tables),
  which reference structured data stored in
  data stores such as Cloud Storage, Google Drive, and Bigtable.
  Unlike BigLake tables, these tables don't let you enforce
  fine-grained security at the table level.

  For information about how to create non-BigLake external
  tables, see the following topics:
  - [Cloud Storage](https://docs.cloud.google.com/bigquery/docs/external-data-cloud-storage)
  - [Google Drive](https://docs.cloud.google.com/bigquery/docs/external-data-drive)
  - [Bigtable](https://docs.cloud.google.com/bigquery/docs/external-data-bigtable)

### Views

Views are logical tables that are defined by using a SQL query. These include the
following types:

- [Views](https://docs.cloud.google.com/bigquery/docs/views-intro), which are logical tables that are defined by
  using SQL queries. These queries define the view that is run
  each time the view is queried.

  For information about how to create views, see [Create views](https://docs.cloud.google.com/bigquery/docs/views).
- [Materialized views](https://docs.cloud.google.com/bigquery/docs/materialized-views-intro), which are precomputed
  views that periodically cache the results of the view query. The cached
  results are stored in BigQuery storage.

  For information about how to create materialized views, see [Create materialized views](https://docs.cloud.google.com/bigquery/docs/materialized-views-create).

## Table limitations

BigQuery tables are subject to the following limitations:

- Table names must be unique per dataset.
- When you export BigQuery table data, the only supported destination is Cloud Storage.
- When you use an API call, enumeration performance slows as you approach 50,000 tables in a dataset.
- The Google Cloud console can display up to 50,000 tables for each dataset.

For information about BigQuery external table limitations, see
the following topics:

- [BigLake tables](https://docs.cloud.google.com/bigquery/docs/biglake-intro#limitations)
- [Object tables](https://docs.cloud.google.com/bigquery/docs/object-table-introduction#limitations)
- [External tables](https://docs.cloud.google.com/bigquery/docs/external-tables#limitations)

## Table quotas

Quotas and limits apply to the different types of jobs you can run against
tables, including the following quotas:

- [Load data into tables](https://docs.cloud.google.com/bigquery/quotas#load_jobs) (load jobs)
- [Export data from tables](https://docs.cloud.google.com/bigquery/quotas#export_jobs) (extract jobs)
- [Query table data](https://docs.cloud.google.com/bigquery/quotas#query_jobs) (query jobs)
- [Copy tables](https://docs.cloud.google.com/bigquery/quotas#copy_jobs) (copy jobs)

For more information about all quotas and limits, see [Quotas and limits](https://docs.cloud.google.com/bigquery/quotas).

To troubleshoot quota errors for tables, see the [BigQuery Troubleshooting page](https://docs.cloud.google.com/bigquery/docs/troubleshoot-quotas).

The following quota errors apply specifically to tables:

- [Table imports or query appends quota errors](https://docs.cloud.google.com/bigquery/docs/troubleshoot-quotas#ts-table-import-quota)
- [Too many DML statements outstanding against table](https://docs.cloud.google.com/bigquery/docs/troubleshoot-quotas#ts-too-many-dml-statements-against-table-quota)

## Table pricing

When you create and use tables in BigQuery, your charges are
based on how much data is stored in the tables and partitions and on the queries
you run against the table data:

- For information about storage pricing, see [Storage pricing](https://cloud.google.com/bigquery/pricing#storage).
- For information about query pricing, see [Query pricing](https://cloud.google.com/bigquery/pricing#analysis_pricing_models).

Many table operations are free, including loading, copying, and exporting data.
Though free, these operations are subject to BigQuery
[quotas and limits](https://docs.cloud.google.com/bigquery/quotas). For information about all free operations,
see [Free operations](https://cloud.google.com/bigquery/pricing#free) on the pricing page.

## Table security

To control access to tables in BigQuery, see
[Control access to resources with IAM](https://docs.cloud.google.com/bigquery/docs/control-access-to-resources-iam).

## What's next

- Learn how to [create and use tables](https://docs.cloud.google.com/bigquery/docs/tables).
- Learn how to [manage tables](https://docs.cloud.google.com/bigquery/docs/managing-tables).
- Learn how to [modify table schemas](https://docs.cloud.google.com/bigquery/docs/managing-table-schemas).
- Learn about [working with table data](https://docs.cloud.google.com/bigquery/docs/managing-table-data).