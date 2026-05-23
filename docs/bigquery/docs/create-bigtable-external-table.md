# Create a Bigtable external table

This page describes how to create a BigQuery permanent external table
that can be used to query data stored in Bigtable. Querying data in
Bigtable is available in all [Bigtable
locations](https://docs.cloud.google.com/bigtable/docs/locations).

## Before you begin

Before you create an external table, gather some information and make sure you
have permission to create the table.

### Required roles

To create an external table to use to query your Bigtable data,
you must be a principal in the Bigtable Admin
(`roles/bigtable.admin`) role for the instance that contains the source table.

You also need the `bigquery.tables.create` BigQuery
Identity and Access Management (IAM) permission.

Each of the following predefined Identity and Access Management roles includes this permission:

- BigQuery Data Editor (`roles/bigquery.dataEditor`)
- BigQuery Data Owner (`roles/bigquery.dataOwner`)
- BigQuery Admin (`roles/bigquery.admin`)

If you are not a principal in any of these roles, ask your administrator
to grant you access or to create the external table for you.

For more information on Identity and Access Management roles and permissions in
BigQuery, see [Predefined roles and
permissions](https://docs.cloud.google.com/bigquery/docs/access-control). To view information on
Bigtable permissions, see [Access control with
Identity and Access Management](https://docs.cloud.google.com/bigtable/docs/access-control). To view the roles required to
query the external table, see [Query Bigtable
data](https://docs.cloud.google.com/bigquery/docs/external-data-bigtable).

### Create or identify a dataset

Before you create an external table, you must
[create a dataset](https://docs.cloud.google.com/bigquery/docs/datasets) to contain the external table. You
can also use an existing dataset.

### Plan your compute usage

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

Determine the type of compute you want to use when you query your data. You
specify that you want to use Data Boost or that you want to route to a
dedicated cluster in your [app profile settings](https://docs.cloud.google.com/bigquery/docs/create-bigtable-external-table#app-profile).

#### Data Boost

To avoid impacting your application serving traffic, you can use Data Boost
serverless compute when you use a BigQuery external
table to read your Bigtable data. To use Data Boost, you must
use a Data Boost app profile and include the app profile ID when you compose
your Bigtable URI. For more information, see
[Bigtable Data Boost overview](https://docs.cloud.google.com/bigtable/docs/data-boost-overview).

> [!NOTE]
> **Note:** While this standard usage is available in both the Enterprise and Enterprise Plus editions, using Data Boost specifically for SQL queries requires the Enterprise Plus edition.

#### Provisioned nodes

If you don't use Data Boost, cluster nodes are used for compute.

If you don't use Data Boost and you plan to frequently query the same data that
serves your production application, we recommend that you designate a cluster in
your Bigtable instance to be used solely for
BigQuery analysis. This isolates the traffic from the cluster or
clusters that you use for your application's reads and writes. To learn more
about replication and creating instances that have more than one cluster, see
[About replication](https://docs.cloud.google.com/bigtable/docs/replication-overview).

### Identify or create an app profile

Before you create an external table, decide which Bigtable app
profile that BigQuery should use to read the data. We recommend
that you use an app profile that you designate for use only with
BigQuery. The app profile can be either a standard app profile or
a Data Boost app profile, depending on the type of compute that you
want to use to query your data.

If you have a cluster in your Bigtable instance that is dedicated
to BigQuery access, configure the app profile to use
single-cluster routing to that cluster.

To use Data Boost serverless compute, create a Data Boost app profile.
To use cluster nodes for compute, create a standard app profile. To learn how
Bigtable app profiles work, see [About app
profiles](https://docs.cloud.google.com/bigtable/docs/app-profiles). To see how to create a new app profile,
see [Create and configure app
profiles](https://docs.cloud.google.com/bigtable/docs/configuring-app-profiles).

### Retrieve the Bigtable URI

To create an external table for a Bigtable data source, you must
provide the Bigtable URI. To retrieve the Bigtable
URI, do the following:

1. Open the Bigtable page in the console.

   [Go to Bigtable](https://console.cloud.google.com/bigtable)
2. Retrieve the following details about your Bigtable
   data source:

   - Your project ID.
   - Your Bigtable instance ID.
   - The ID of the Bigtable app profile that you plan to use. This can be either a standard app profile or a Data Boost app profile, depending on the [type of compute](https://docs.cloud.google.com/bigquery/docs/create-bigtable-external-table#compute) that you want to use. If you don't specify an app profile ID, the default app profile is used.
   - The name of your Bigtable table.
3. Compose the Bigtable URI using the following format, where:

   - <var translate="no">PROJECT_ID</var> is the project that contains your Bigtable instance
   - <var translate="no">INSTANCE_ID</var> is the Bigtable instance ID
   - <var translate="no">APP_PROFILE</var> (optional) is the identifier for the app profile that you want to use
   - <var translate="no">TABLE_NAME</var> is the name of the table you're querying

   `https://googleapis.com/bigtable/projects/PROJECT_ID/instances/INSTANCE_ID[/appProfiles/APP_PROFILE]/tables/TABLE_NAME`

> [!NOTE]
> **Note:** Exactly one Bigtable URI can be specified, and it must be a fully specified, valid HTTPS URL for a Bigtable table. Wildcards are not supported for Bigtable external data sources.

## Create permanent external tables

When you create a permanent external table in BigQuery that is
linked to a Bigtable data source, there are two options for
specifying the format of the external table:

- If you are using the API or the bq command-line tool, you create a [table definition file](https://docs.cloud.google.com/bigquery/docs/external-table-definition) that defines the schema and metadata for the external table.
- If you are using SQL, you use the `uri` option of the `CREATE EXTERNAL TABLE` statement to specify the Bigtable table to pull data from, and the `bigtable_options` option to specify the table schema.

The external table data is not stored in the BigQuery table.
Because the table is permanent, you can use dataset-level [access
controls](https://docs.cloud.google.com/bigquery/docs/access-control) to share the table with others who also
have access to the underlying Bigtable data source.

To create a permanent table, choose one of the following methods.

### SQL

You can create a permanent external table by running the
[`CREATE EXTERNAL TABLE` DDL statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_external_table_statement).
You must specify the table schema explicitly as part of the statement
options.

<br />


1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, enter the following statement:

   ```googlesql
   CREATE EXTERNAL TABLE DATASET.NEW_TABLE
   OPTIONS (
     format = 'CLOUD_BIGTABLE',
     uris = ['URI'],
     bigtable_options = BIGTABLE_OPTIONS );
   ```


   Replace the following:
   - `DATASET`: the dataset in which to create the Bigtable external table.
   - `NEW_TABLE`: the name for the Bigtable external table.
   - `URI`: the URI for the Bigtable table you want to use as a data source. This URI must follow the format described in [Retrieving the Bigtable URI](https://docs.cloud.google.com/bigquery/docs/create-bigtable-external-table#bigtable-uri).
   - `BIGTABLE_OPTIONS`: the schema for the Bigtable table in JSON format. For a list of Bigtable table definition options, see `https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables#bigtableoptions` in the REST API reference.

     <br />

   <br />

   <br />

3. Click **Run**.

   <br />

For more information about how to run queries, see [Run an interactive query](https://docs.cloud.google.com/bigquery/docs/running-queries#queries).

A statement to create an external Bigtable table
might look similar to the following:

    CREATE EXTERNAL TABLE mydataset.BigtableTable
    OPTIONS (
      format = 'CLOUD_BIGTABLE',
      uris = ['https://googleapis.com/bigtable/projects/myproject/instances/myBigtableInstance/appProfiles/myAppProfile/tables/table1'],
      bigtable_options =
        """
        {
          columnFamilies: [
            {
              "familyId": "familyId1",
              "type": "INTEGER",
              "encoding": "BINARY"
            }
          ],
          readRowkeyAsString: true
        }
        """
    );

### bq

You create a table in the bq command-line tool using the
[`bq mk` command](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#bq_mk). When
you use the bq command-line tool to create a table linked to an external data source,
you identify the table's schema using a
[table definition file](https://docs.cloud.google.com/bigquery/docs/external-table-definition#tabledef-bigtable).

1. Use the `bq mk` command to create a permanent table.

   ```bash
   bq mk \
   --external_table_definition=DEFINITION_FILE \
   DATASET.TABLE
   ```

   Replace the following:
   - `DEFINITION_FILE`: the path to the [table definition file](https://docs.cloud.google.com/bigquery/docs/external-table-definition#tabledef-bigtable) on your local machine.
   - `DATASET`: the name of the dataset that contains the table.
   - `TABLE`: the name of the table you're creating.

### API

Use the [`tables.insert`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables/insert)
API method, and create an
[`ExternalDataConfiguration`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables#externaldataconfiguration)
in the [`Table` resource](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables#Table)
that you pass in.

For the `sourceUris` property in the `Table` resource,
specify only one [Bigtable URI](https://docs.cloud.google.com/bigquery/docs/create-bigtable-external-table#bigtable-uri). It must be a
valid HTTPS URL.

For the `sourceFormat` property, specify `"BIGTABLE"`.

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
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableId.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableInfo.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableResult.html;
    import com.google.common.collect.ImmutableList;
    import org.apache.commons.codec.binary.Base64;

    // Sample to queries an external bigtable data source using a permanent table
    public class QueryExternalBigtablePerm {

      public static void main(String[] args) {
        // TODO(developer): Replace these variables before running the sample.
        String projectId = "MY_PROJECT_ID";
        String bigtableInstanceId = "MY_INSTANCE_ID";
        String bigtableTableName = "MY_BIGTABLE_NAME";
        String bigqueryDatasetName = "MY_DATASET_NAME";
        String bigqueryTableName = "MY_TABLE_NAME";
        String sourceUri =
            String.format(
                "https://googleapis.com/bigtable/projects/%s/instances/%s/tables/%s",
                projectId, bigtableInstanceId, bigtableTableName);
        String query = String.format("SELECT * FROM %s ", bigqueryTableName);
        queryExternalBigtablePerm(bigqueryDatasetName, bigqueryTableName, sourceUri, query);
      }

      public static void queryExternalBigtablePerm(
          String datasetName, String tableName, String sourceUri, String query) {
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

          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableId.html tableId = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableId.html.of(datasetName, tableName);
          // Create a permanent table linked to the Bigtable table
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.ExternalTableDefinition.html externalTable =
              https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.ExternalTableDefinition.html.newBuilder(sourceUri, options).build();
          bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html#com_google_cloud_bigquery_BigQuery_create_com_google_cloud_bigquery_DatasetInfo_com_google_cloud_bigquery_BigQuery_DatasetOption____(TableInfo.of(tableId, externalTable));

          // Example query
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableResult.html results = bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html#com_google_cloud_bigquery_BigQuery_query_com_google_cloud_bigquery_QueryJobConfiguration_com_google_cloud_bigquery_BigQuery_JobOption____(https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.QueryJobConfiguration.html.of(query));

          results
              .https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableResult.html#com_google_cloud_bigquery_TableResult_iterateAll__()
              .forEach(row -> row.forEach(val -> System.out.printf("%s,", val.toString())));

          System.out.println("Query on external permanent table performed successfully.");
        } catch (https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryException.html | InterruptedException e) {
          System.out.println("Query not performed \n" + e.toString());
        }
      }
    }

## Query external tables

For more information, see
[Query Bigtable data](https://docs.cloud.google.com/bigquery/docs/external-data-bigtable).

## Generated schema

By default, BigQuery exposes the values in a column family as an
array of columns and within that, an array of values written at different
timestamps. This schema preserves the natural layout of data in
Bigtable, but SQL queries can be challenging. It is possible to
promote columns to subfields within the parent column family and to read only
the latest value from each cell. This represents both of the arrays in the
default schema as scalar values.

### Example

You are storing user profiles for a fictional social network. One data model for
this might be a `profile` column family with individual
columns for `gender`, `age` and `email`:

    rowkey | profile:gender| profile:age| profile:email
    ---| ---| ---| ---
    alice  | female        | 30         | alice@gmail.com

Using the default schema, a GoogleSQL query to count the number of male users
over 30 is:

```googlesql
SELECT
  COUNT(1)
FROM
  `dataset.table`
OMIT
  RECORD IF NOT SOME(profile.column.name = "gender"
    AND profile.column.cell.value = "male")
  OR NOT SOME(profile.column.name = "age"
    AND INTEGER(profile.column.cell.value) > 30)
```

Querying the data is less challenging if `gender` and `age` are exposed as sub-
fields. To expose them as sub-fields, list `gender` and `age` as named columns
in the `profile` column family when defining the table. You can also instruct
BigQuery to expose the latest values from this column family
because typically, only the latest value (and possibly the only value) is of
interest.

After exposing the columns as sub-fields, the GoogleSQL query to count the
number of male users over 30 is:

```googlesql
SELECT
  COUNT(1)
FROM
  `dataset.table`
WHERE
  profile.gender.cell.value="male"
  AND profile.age.cell.value > 30
```

Notice how `gender` and `age` are referenced directly as fields. The JSON
configuration for this setup is:

```json
  "bigtableOptions": {
    "readRowkeyAsString": "true",
    "columnFamilies": [
      {
          "familyId": "profile",
          "onlyReadLatest": "true",
          "columns": [
              {
                  "qualifierString": "gender",
                  "type": "STRING"
              },
              {
                  "qualifierString": "age",
                  "type": "INTEGER"
              }
          ]
      }
    ]
  }
```

## Value encoding

Bigtable stores data as raw bytes, independent to data encoding.
However, byte values are of limited use in SQL query analysis.
Bigtable provides two basic types of scalar decoding: text
and HBase-binary.

The text format assumes that all values are stored as alphanumeric text strings.
For example, an integer 768 will be stored as the string "768". The binary
encoding assumes that HBase's
[`Bytes.toBytes`](https://hbase.apache.org/devapidocs/org/apache/hadoop/hbase/util/Bytes.html)
class of methods were used to encode the data and applies an appropriate
decoding method.

## Supported regions and zones

Querying data in Bigtable is available in all supported
Bigtable zones. For more information, see
[BigQuery locations](https://docs.cloud.google.com/bigtable/docs/locations). For multi-cluster
instances, BigQuery routes traffic based on
Bigtable [app profile](https://docs.cloud.google.com/bigtable/docs/app-profiles) settings.

## Limitations

- You can't create external tables over Bigtable SQL-based objects, such as views and continuous materialized views.
- For more information about limitations that apply to external tables, see [External table limitations](https://docs.cloud.google.com/bigquery/docs/external-tables#limitations).

## Scopes for Compute Engine instances

When you create a Compute Engine instance, you can specify a list of scopes
for the instance. The scopes control the instance's access to Google Cloud
products, including Bigtable. Applications running on the VM
use the service account to call Google Cloud APIs.

If you set up a Compute Engine instance to run as a
[service account](https://docs.cloud.google.com/compute/docs/access/create-enable-service-accounts-for-instances),
and that service account accesses an external table linked to a
Bigtable data source, you must add the Bigtable
read-only data access scope
(`https://www.googleapis.com/auth/bigtable.data.readonly`) to the
instance. For more information, see
[Creating a Compute Engine instance for Bigtable](https://docs.cloud.google.com/bigtable/docs/creating-compute-instance).

For information on applying scopes to a Compute Engine instance,
see
[Changing the service account and access scopes for an instance](https://docs.cloud.google.com/compute/docs/access/create-enable-service-accounts-for-instances#changeserviceaccountandscopes).
For more information on Compute Engine service accounts, see
[Service accounts](https://docs.cloud.google.com/compute/docs/access/service-accounts).

## What's next

- [Learn more about Bigtable schema design.](https://docs.cloud.google.com/bigtable/docs/schema-design)
- [Review the introduction to external data sources.](https://docs.cloud.google.com/bigquery/docs/external-data-sources)