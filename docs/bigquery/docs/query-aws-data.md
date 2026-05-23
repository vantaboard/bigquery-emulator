# Query Amazon S3 data

This document describes how to query data stored in an
[Amazon Simple Storage Service (Amazon S3) BigLake table](https://docs.cloud.google.com/bigquery/docs/omni-aws-create-external-table).

## Before you begin

Ensure that you have a [Amazon S3 BigLake table](https://docs.cloud.google.com/bigquery/docs/omni-aws-create-external-table).

### Required roles

To query Amazon S3 BigLake tables, ensure
that the caller of the BigQuery API has the following roles:

- BigQuery Connection User (`roles/bigquery.connectionUser`)
- BigQuery Data Viewer (`roles/bigquery.dataViewer`)
- BigQuery User (`roles/bigquery.user`)

The caller can be your account or an
[Amazon S3 connection service account](https://docs.cloud.google.com/bigquery/docs/omni-aws-create-connection#creating-aws-connection).
Depending on your permissions, you can
grant these roles to yourself or ask your administrator
to grant them to you. For more information about granting roles, see
[Viewing the grantable roles on resources](https://docs.cloud.google.com/iam/docs/viewing-grantable-roles).

To see the exact permissions that are required to query
Amazon S3 BigLake tables, expand the
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

## Query Amazon S3 BigLake tables

After creating a Amazon S3 BigLake table, you can [query it using
GoogleSQL syntax](https://docs.cloud.google.com/bigquery/docs/running-queries), the same as if
it were a standard BigQuery table.

The [cached query results](https://docs.cloud.google.com/bigquery/docs/cached-results)
are stored in a BigQuery temporary table. To query a temporary
BigLake table, see
[Query a temporary BigLake table](https://docs.cloud.google.com/bigquery/docs/query-aws-data#query-temp-biglake-table).
For more information about BigQuery Omni limitations and quotas, see
[limitations](https://docs.cloud.google.com/bigquery/docs/omni-introduction#limitations)
and [quotas](https://docs.cloud.google.com/bigquery/docs/omni-introduction#quotas_and_limits).

When creating a reservation in a BigQuery Omni region, use the
Enterprise edition. To learn how to create a reservation with an edition, see
[Create reservations](https://docs.cloud.google.com/bigquery/docs/reservations-tasks#create_reservations).

Run a query on a BigLake Amazon S3 table:

### SQL

To query the table:

<br />


1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, enter the following statement:

   ```googlesql
   SELECT * FROM DATASET_NAME.TABLE_NAME;
   ```


   Replace the following:
   - `DATASET_NAME`: the dataset name that you created
   - `TABLE_NAME`: the name of the [table that you created](https://docs.cloud.google.com/bigquery/docs/query-aws-data#create-biglake-table)

     <br />

   - Click **Run**.

     <br />

For more information about how to run queries, see [Run an interactive query](https://docs.cloud.google.com/bigquery/docs/running-queries#queries).

### Java


Before trying this sample, follow the Java setup instructions in the
[BigQuery quickstart using
client libraries](https://docs.cloud.google.com/bigquery/docs/quickstarts/quickstart-client-libraries).


For more information, see the
[BigQuery Java API
reference documentation](https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/overview).


To authenticate to BigQuery, set up Application Default Credentials.
For more information, see

[Set up authentication for client libraries](https://docs.cloud.google.com/bigquery/docs/authentication#client-libs).

    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryException.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryOptions.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.CsvOptions.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.DatasetId.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.ExternalTableDefinition.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Field.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.QueryJobConfiguration.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Schema.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.StandardSQLTypeName.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableId.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableInfo.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableResult.html;

    // Sample to queries an external data source aws s3 using a permanent table
    public class QueryExternalTableAws {

      public static void main(String[] args) throws InterruptedException {
        // TODO(developer): Replace these variables before running the sample.
        String projectId = "MY_PROJECT_ID";
        String datasetName = "MY_DATASET_NAME";
        String externalTableName = "MY_EXTERNAL_TABLE_NAME";
        // Query to find states starting with 'W'
        String query =
            String.format(
                "SELECT * FROM s%.%s.%s WHERE name LIKE 'W%%'",
                projectId, datasetName, externalTableName);
        queryExternalTableAws(query);
      }

      public static void queryExternalTableAws(String query) throws InterruptedException {
        try {
          // Initialize client that will be used to send requests. This client only needs to be created
          // once, and can be reused for multiple requests.
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html bigquery = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryOptions.html.getDefaultInstance().getService();

          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableResult.html results = bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html#com_google_cloud_bigquery_BigQuery_query_com_google_cloud_bigquery_QueryJobConfiguration_com_google_cloud_bigquery_BigQuery_JobOption____(https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.QueryJobConfiguration.html.of(query));

          results
              .https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableResult.html#com_google_cloud_bigquery_TableResult_iterateAll__()
              .forEach(row -> row.forEach(val -> System.out.printf("%s,", val.toString())));

          System.out.println("Query on aws external permanent table performed successfully.");
        } catch (https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryException.html e) {
          System.out.println("Query not performed \n" + e.toString());
        }
      }
    }

## Query a temporary table

BigQuery creates temporary tables to store query results.
To retrieve query results from temporary tables, you can use the Google Cloud console
or the [BigQuery API](https://docs.cloud.google.com/bigquery/docs/reliability-read#read_with_api).

Select one of the following options:

### Console

When you [query a BigLake table](https://docs.cloud.google.com/bigquery/docs/query-aws-data#query-biglake-table) that
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