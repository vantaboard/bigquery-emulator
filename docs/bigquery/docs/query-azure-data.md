# Query Blob Storage data

This document describes how to query data stored in an
[Azure Blob Storage BigLake table](https://docs.cloud.google.com/bigquery/docs/omni-azure-create-external-table).

## Before you begin

Ensure that you have a [Blob Storage BigLake table](https://docs.cloud.google.com/bigquery/docs/omni-azure-create-external-table).

### Required roles

To query Blob Storage BigLake tables, ensure
that the caller of the BigQuery API has the following roles:

- BigQuery Connection User (`roles/bigquery.connectionUser`)
- BigQuery Data Viewer (`roles/bigquery.dataViewer`)
- BigQuery User (`roles/bigquery.user`)

The caller can be your account or an
[Blob Storage connection service account](https://docs.cloud.google.com/bigquery/docs/omni-azure-create-connection#create_an_azure_connection).
Depending on your permissions, you can
grant these roles to yourself or ask your administrator
to grant them to you. For more information about granting roles, see
[Viewing the grantable roles on resources](https://docs.cloud.google.com/iam/docs/viewing-grantable-roles).

To see the exact permissions that are required to query
Blob Storage BigLake tables, expand the
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

## Query Blob Storage BigLake tables

After creating a Blob Storage BigLake table, you can [query it using
GoogleSQL syntax](https://docs.cloud.google.com/bigquery/docs/running-queries), the same as if
it were a standard BigQuery table.

The [cached query results](https://docs.cloud.google.com/bigquery/docs/cached-results)
are stored in a BigQuery temporary table. To query a temporary
BigLake table, see
[Query a temporary BigLake table](https://docs.cloud.google.com/bigquery/docs/query-azure-data#query-temp-biglake-table).
For more information about BigQuery Omni limitations and quotas, see
[limitations](https://docs.cloud.google.com/bigquery/docs/omni-introduction#limitations)
and [quotas](https://docs.cloud.google.com/bigquery/docs/omni-introduction#quotas_and_limits).

When creating a reservation in a BigQuery Omni region, use the
Enterprise edition. To learn how to create a reservation with an edition, see
[Create reservations](https://docs.cloud.google.com/bigquery/docs/reservations-tasks#create_reservations).

Run a query on the Blob Storage BigLake table:

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, enter the following statement:

   ```googlesql
   SELECT * FROM DATASET_NAME.TABLE_NAME;
   ```


   Replace the following:
   - `DATASET_NAME`: the dataset name that you created
   - `TABLE_NAME`: the BigLake table that name you
     created

   - Click **Run**.

     <br />

For more information about how to run queries, see [Run an interactive query](https://docs.cloud.google.com/bigquery/docs/running-queries#queries).

## Query a temporary table

BigQuery creates temporary tables to store query results.
To retrieve query results from temporary tables, you can use the Google Cloud console
or the [BigQuery API](https://docs.cloud.google.com/bigquery/docs/reliability-read#read_with_api).

Select one of the following options:

### Console

When you [query a BigLake table](https://docs.cloud.google.com/bigquery/docs/query-azure-data#query-biglake-table) that
references external cloud data, you can view the query results displayed
in the Google Cloud console.

### API

To query a BigLake table using the API, follow these steps:

1. Create a [Job object](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/Job).
2. Call the [`jobs.insert` method](https://docs.cloud.google.com/bigquery/docs/reference/v2/jobs/insert) to run the query asynchronously or the [`jobs.query` method](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs/query) to run the query synchronously, passing in the `Job` object.
3. Read rows with the [`jobs.getQueryResults`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs/getQueryResults) by passing the given job reference, and the [`tabledata.list`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tabledata/list) methods by passing the given table reference of the query result.

## Query the `_FILE_NAME` pseudocolumn


Tables based on external data sources provide a pseudocolumn named `_FILE_NAME`. This
column contains the fully qualified path to the file to which the row belongs. This column is
available only for tables that reference external data stored in
**Cloud Storage** , **Google Drive** ,
**Amazon S3** , and **Azure Blob Storage**.


The `_FILE_NAME` column name is reserved, which means that you cannot create a column
by that name in any of your tables. To select the value of `_FILE_NAME`, you must use
an alias. The following example query demonstrates selecting `_FILE_NAME` by assigning
the alias `fn` to the pseudocolumn.

```bash
  bq query \
  --project_id=PROJECT_ID \
  --use_legacy_sql=false \
  'SELECT
     name,
     _FILE_NAME AS fn
   FROM
     `` `DATASET.TABLE_NAME` ``
   WHERE
     name contains "Alex"' 
```

Replace the following:

- `PROJECT_ID` is a valid project ID (this flag is not required if you use Cloud Shell or if you set a default project in the Google Cloud CLI)
- `DATASET `is the name of the dataset that stores the permanent external table
- `TABLE_NAME` is the name of the permanent external table


When the query has a filter predicate on the `_FILE_NAME` pseudocolumn,
BigQuery attempts to skip reading files that do not satisfy the filter. Similar
recommendations to
[querying ingestion-time partitioned tables using pseudocolumns](https://docs.cloud.google.com/bigquery/docs/querying-partitioned-tables#query_an_ingestion-time_partitioned_table)
apply when constructing query predicates with the `_FILE_NAME` pseudocolumn.

## What's next

- Learn about [using SQL in BigQuery](https://docs.cloud.google.com/bigquery/docs/introduction-sql).
- Learn about [BigQuery Omni](https://docs.cloud.google.com/bigquery/docs/omni-introduction).
- Learn about [BigQuery quotas](https://docs.cloud.google.com/bigquery/quotas).