# Query Bigtable data

This document describes how to use BigQuery to query data stored in a
[Bigtable external table](https://docs.cloud.google.com/bigquery/docs/create-bigtable-external-table).
For information on how to query data directly from Bigtable,
see [GoogleSQL for Bigtable
overview](https://docs.cloud.google.com/bigtable/docs/googlesql-overview).

[Bigtable](https://docs.cloud.google.com/bigtable/docs) is Google's sparsely populated NoSQL
database that can scale to billions of rows, thousands of columns, and
petabytes of data. For information on the Bigtable data model,
see [Storage model](https://docs.cloud.google.com/bigtable/docs/overview#storage-model).

## Query permanent external tables

Before you begin, you or an administrator in your organization must create an
external table for you to use. For details and required permissions, see [Create
a BigQuery external
table](https://docs.cloud.google.com/bigquery/docs/create-bigtable-external-table).

### Required roles

To query Bigtable external tables, ensure
you have the following roles.

- BigQuery Data Viewer (`roles/bigquery.dataViewer`)
- BigQuery User (`roles/bigquery.user`)
- Bigtable Reader (`roles/bigtable.reader`)

Depending on your permissions, you can
grant these roles to yourself or ask your administrator
to grant them to you. For more information about granting roles, see
[Viewing the grantable roles on resources](https://docs.cloud.google.com/iam/docs/viewing-grantable-roles).

To see the exact BigQuery permissions that are required to query
external tables, expand the **Required permissions** section:

#### Required permissions

- `bigquery.jobs.create`
- `bigquery.readsessions.create` (Only required if you are [streaming data with the
  BigQuery Storage Write API](https://docs.cloud.google.com/bigquery/docs/write-api-streaming))
- `bigquery.tables.get`
- `bigquery.tables.getData`

You might also be able to get these permissions with [custom roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles)
or other [predefined roles](https://docs.cloud.google.com/iam/docs/roles-overview#predefined).

### Query the table

You can run a query against a permanent external Bigtable table
exactly as if it were a
[standard BigQuery table](https://docs.cloud.google.com/bigquery/docs/tables-intro#standard-tables),
subject to the [limitations](https://docs.cloud.google.com/bigquery/docs/external-tables#limitations)
on external data sources. For more information, see [Run interactive and batch
queries](https://docs.cloud.google.com/bigquery/docs/running-queries).

## Query temporary external tables

Querying an external data source using a temporary table is useful
for one-time, ad-hoc queries over external data, or for extract, transform, and load (ETL)
processes.

To query an external data source without creating a permanent table, you provide a table
definition for the temporary table, and then use that table definition in a command or call
to query the temporary table. You can provide the table definition in any of the following
ways:

- A [table definition file](https://docs.cloud.google.com/bigquery/external-table-definition)
- An inline schema definition
- A [JSON schema file](https://docs.cloud.google.com/bigquery/docs/schemas#specifying_a_json_schema_file)

The table definition file or supplied schema is used to create the temporary external table,
and the query runs against the temporary external table.

When you use a temporary external table, you do not create a table in one of your
BigQuery datasets. Because the table is not permanently stored in a dataset, it
cannot be shared with others.

Using a temporary external table instead of a permanent external
table has some limitations, including the following:

- You must have the Bigtable Admin (`roles/bigtable.admin`) role.
- This approach does not let you use the Google Cloud console to infer the schema of the Bigtable table and automatically create the table definition. You must create the table definition yourself.

### Required roles

To query Bigtable temporary external tables, ensure
you have the following roles:

- BigQuery Data Viewer (`roles/bigquery.dataViewer`)
- BigQuery User (`roles/bigquery.user`)
- Bigtable Admin (`roles/bigtable.admin`)

Depending on your permissions, you can
grant these roles to yourself or ask your administrator
to grant them to you. For more information about granting roles, see
[Viewing the grantable roles on resources](https://docs.cloud.google.com/iam/docs/viewing-grantable-roles).

To see the exact BigQuery permissions that are required to query
external tables, expand the **Required permissions** section:

#### Required permissions

- `bigquery.jobs.create`
- `bigquery.readsessions.create` (Only required if you are [streaming data with the
  BigQuery Storage Write API](https://docs.cloud.google.com/bigquery/docs/write-api-streaming))
- `bigquery.tables.get`
- `bigquery.tables.getData`

You might also be able to get these permissions with [custom roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles)
or other [predefined roles](https://docs.cloud.google.com/iam/docs/roles-overview#predefined).

### Create and query the table

To query Bigtable data using a temporary external table, you:

- Create a [table definition file](https://docs.cloud.google.com/bigquery/docs/external-table-definition#tabledef-bigtable)
- Submit both a query and a table definition file

Creating and querying a temporary external table is supported by the
bq command-line tool and the API.

### bq

To query a temporary table using a table definition file, enter the
`bq query` command with the `--external_table_definition` flag.

(Optional) Supply the `--location` flag and set the value to your
[location](https://docs.cloud.google.com/bigquery/docs/locations).

```bash
bq --location=LOCATION query \
--use_legacy_sql=false \
--external_table_definition=TABLE::DEFINITION_FILE \
'QUERY'
```

Replace the following:

- `LOCATION`: the name of your [location](https://docs.cloud.google.com/bigquery/docs/locations). The `--location` flag is optional.
- `TABLE`: the name of the temporary table you're creating.
- `DEFINITION_FILE`: the path to the [table definition file](https://docs.cloud.google.com/bigquery/docs/external-table-definition#tabledef-bigtable) on your local machine.
- `QUERY`: the query you're submitting to the temporary table.

For example, the following command creates and queries a temporary table
named `follows` using a table definition file named `follows_def`.

    bq query \
    --use_legacy_sql=false \
    --external_table_definition=follows::/tmp/follows_def \
    'SELECT
      COUNT(rowkey)
     FROM
       follows'

### API

- Create a query. See [Querying data](https://docs.cloud.google.com/bigquery/querying-data) for
  information about creating a query job.

- (Optional) Specify your location in the `location` property in the
  `jobReference` section of the [job resource](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/Job).

- Specify the external data source properties by setting the
  `ExternalDataConfiguration` for the [table resource](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables#externaldataconfiguration).

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
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigtableColumn.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigtableColumnFamily.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigtableOptions.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.ExternalTableDefinition.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.QueryJobConfiguration.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableResult.html;
    import com.google.common.collect.ImmutableList;
    import org.apache.commons.codec.binary.Base64;

    // Sample to queries an external bigtable data source using a temporary table
    public class QueryExternalBigtableTemp {

      public static void main(String[] args) {
        // TODO(developer): Replace these variables before running the sample.
        String projectId = "MY_PROJECT_ID";
        String bigtableInstanceId = "MY_INSTANCE_ID";
        String bigtableTableName = "MY_BIGTABLE_NAME";
        String bigqueryTableName = "MY_TABLE_NAME";
        String sourceUri =
            String.format(
                "https://googleapis.com/bigtable/projects/%s/instances/%s/tables/%s",
                projectId, bigtableInstanceId, bigtableTableName);
        String query = String.format("SELECT * FROM %s ", bigqueryTableName);
        queryExternalBigtableTemp(bigqueryTableName, sourceUri, query);
      }

      public static void queryExternalBigtableTemp(String tableName, String sourceUri, String query) {
        try {
          // Initialize client that will be used to send requests. This client only needs to be created
          // once, and can be reused for multiple requests.
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html bigquery = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryOptions.html.getDefaultInstance().getService();

          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigtableColumnFamily.html.Builder statsSummary = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigtableColumnFamily.html.newBuilder();

          // Configuring Columns
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigtableColumn.html connectedCell =
              https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigtableColumn.html.newBuilder()
                  .https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigtableColumn.Builder.html#com_google_cloud_bigquery_BigtableColumn_Builder_setQualifierEncoded_java_lang_String_(Base64.encodeBase64String("connected_cell".https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.jdbc.BigQueryBaseResultSet.html#com_google_cloud_bigquery_jdbc_BigQueryBaseResultSet_getBytes_int_()))
                  .https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigtableColumn.Builder.html#com_google_cloud_bigquery_BigtableColumn_Builder_setFieldName_java_lang_String_("connected_cell")
                  .setType("STRING")
                  .setEncoding("TEXT")
                  .build();
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigtableColumn.html connectedWifi =
              https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigtableColumn.html.newBuilder()
                  .https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigtableColumn.Builder.html#com_google_cloud_bigquery_BigtableColumn_Builder_setQualifierEncoded_java_lang_String_(Base64.encodeBase64String("connected_wifi".https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.jdbc.BigQueryBaseResultSet.html#com_google_cloud_bigquery_jdbc_BigQueryBaseResultSet_getBytes_int_()))
                  .https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigtableColumn.Builder.html#com_google_cloud_bigquery_BigtableColumn_Builder_setFieldName_java_lang_String_("connected_wifi")
                  .setType("STRING")
                  .setEncoding("TEXT")
                  .build();
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigtableColumn.html osBuild =
              https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigtableColumn.html.newBuilder()
                  .https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigtableColumn.Builder.html#com_google_cloud_bigquery_BigtableColumn_Builder_setQualifierEncoded_java_lang_String_(Base64.encodeBase64String("os_build".https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.jdbc.BigQueryBaseResultSet.html#com_google_cloud_bigquery_jdbc_BigQueryBaseResultSet_getBytes_int_()))
                  .https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigtableColumn.Builder.html#com_google_cloud_bigquery_BigtableColumn_Builder_setFieldName_java_lang_String_("os_build")
                  .setType("STRING")
                  .setEncoding("TEXT")
                  .build();

          // Configuring column family and columns
          statsSummary
              .setColumns(ImmutableList.of(connectedCell, connectedWifi, osBuild))
              .https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigtableColumnFamily.Builder.html#com_google_cloud_bigquery_BigtableColumnFamily_Builder_setFamilyID_java_lang_String_("stats_summary")
              .setOnlyReadLatest(true)
              .setEncoding("TEXT")
              .setType("STRING")
              .build();

          // Configuring BigtableOptions is optional.
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigtableOptions.html options =
              https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigtableOptions.html.newBuilder()
                  .https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigtableOptions.Builder.html#com_google_cloud_bigquery_BigtableOptions_Builder_setIgnoreUnspecifiedColumnFamilies_java_lang_Boolean_(true)
                  .https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigtableOptions.Builder.html#com_google_cloud_bigquery_BigtableOptions_Builder_setReadRowkeyAsString_java_lang_Boolean_(true)
                  .https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigtableOptions.Builder.html#com_google_cloud_bigquery_BigtableOptions_Builder_setColumnFamilies_java_util_List_com_google_cloud_bigquery_BigtableColumnFamily__(ImmutableList.of(statsSummary.build()))
                  .build();

          // Configure the external data source and query job.
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.ExternalTableDefinition.html externalTable =
              https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.ExternalTableDefinition.html.newBuilder(sourceUri, options).build();
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.QueryJobConfiguration.html queryConfig =
              https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.QueryJobConfiguration.html.newBuilder(query)
                  .https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.QueryJobConfiguration.Builder.html#com_google_cloud_bigquery_QueryJobConfiguration_Builder_addTableDefinition_java_lang_String_com_google_cloud_bigquery_ExternalTableDefinition_(tableName, externalTable)
                  .build();

          // Example query
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableResult.html results = bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html#com_google_cloud_bigquery_BigQuery_query_com_google_cloud_bigquery_QueryJobConfiguration_com_google_cloud_bigquery_BigQuery_JobOption____(queryConfig);

          results
              .https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableResult.html#com_google_cloud_bigquery_TableResult_iterateAll__()
              .forEach(row -> row.forEach(val -> System.out.printf("%s,", val.toString())));

          System.out.println("Query on external temporary table performed successfully.");
        } catch (https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryException.html | InterruptedException e) {
          System.out.println("Query not performed \n" + e.toString());
        }
      }
    }

## Performance considerations

The performance of queries against Bigtable external data sources
depends on three factors:

- The number of rows
- The amount of data read
- The extent of parallelization

BigQuery tries to read as little data as possible by only reading
the column families that are referenced in the query. The extent of
parallelization depends on how many nodes you have in your
Bigtable cluster and how many splits you have for your table.

Note that Bigtable auto-merges splits based on load. If your
table is not being read frequently, there will be fewer splits over time and
a gradual degradation in query performance. For more information, see [How
BigQuery optimizes your data over
time](https://docs.cloud.google.com/bigtable/docs/performance#optimization).

### Compute

> [!WARNING]
>
> **Preview**
>
>
> This product or feature is
>
> subject to the "Pre-GA Offerings Terms" in the General Service Terms section
> of the [Service Specific
> Terms](https://docs.cloud.google.com/terms/service-terms#1).
>
> Pre-GA products and features are available "as is" and might have limited support.
>
> For more information, see the
> [launch stage descriptions](https://cloud.google.com/products/#product-launch-stages).

When you query your Bigtable data from BigQuery,
you have the following options for compute:

- Cluster nodes, which is the default.
- [Data Boost](https://docs.cloud.google.com/bigtable/docs/data-boost-overview) (Preview), a serverless compute option that lets you isolate your analytics traffic without impacting the application-serving traffic that your clusters' nodes are handling.

To use Data Boost, you or your administrator must create a definition file that
specifies a Data Boost app profile in the Bigtable URI. For more
information, see [Create a Bigtable external
table](https://docs.cloud.google.com/bigquery/docs/create-bigtable-external-table).

If you don't use Data Boost, be aware that querying Bigtable from
BigQuery consumes Bigtable CPU cycles. CPU
consumption by BigQuery when using provisioned nodes for compute
might affect latency and throughput for other concurrent requests such as live
user traffic serving. For example, high CPU usage on Bigtable
affects long-tail queries and increases latency at the 99th percentile.

As a result, you should monitor Bigtable CPU usage to verify that
you're within the recommended bounds as noted on the Bigtable
monitoring dashboard in the Google Cloud console. Increasing the number of
nodes for your instance lets you handle both BigQuery traffic and
traffic from other concurrent requests.

## Query filters

You can add query filters when querying an external table
to reduce BigQuery resource usage.

### Row key filter

Queries with a row key equality filter only read that specific row. For example, in
GoogleSQL syntax:

```googlesql
SELECT
  COUNT(follows.column.name)
FROM
  `dataset.table`
WHERE
  rowkey = "alice";
```

Range filters such as `rowkey > '1'` and `rowkey < '8'` are also supported, but
only when rowkey is read as a string with the `readRowkeyAsString` option.

> [!NOTE]
> **Note:** If `readRowkeyAsString` is set to `true`, then the rowkey column families are read and converted to strings. Otherwise they are read with BYTES type values.

### Filter by column family and qualifier

You can also select a specific column family or a specific qualifier within a column family.
To filter by column family, select the column family name, and the result includes only the selected column family. In the following example, `user_info` represents a column family:

        SELECT
          rowkey AS user_id,
          user_info
        FROM
          project.dataset.table;

To filter by a specific qualifier, you must first declare them in `"columns"` in the external table definition:

    CREATE OR REPLACE EXTERNAL TABLE project.dataset.table
      OPTIONS (
        format = 'CLOUD_BIGTABLE',
        uris = ['https://googleapis.com/bigtable/projects/.../instances/.../tables/...'],
        bigtable_options = '''{
      "columnFamilies": [
        {
          "familyId": "user_info",
          "columns": [
            {
              "qualifierString": "name"
            },
            {
              "qualifierString": "email"
            },
            {
              "qualifierString": "registered_at"
            }
          ]
        },
        {
          "familyId": "session_data"
        }
      ],
      "readRowkeyAsString": true,
      "timestampSuffix": "_ts"
    }'''
      );

<br />

After the external table is created, use a `SELECT` statement to query a specific qualifier.
This ensures that BigQuery pushes down the filter to Bigtable and only loads the specified qualifiers when running a `SELECT` statement from BigQuery, not the entire column family's data. This reduces BigQuery resource consumption.

        SELECT
          rowkey AS user_id,
          user_info.email.cell[SAFE_OFFSET(0)].value as email
        FROM
          project.dataset.table;