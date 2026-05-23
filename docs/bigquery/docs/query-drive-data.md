# Query Drive data

This document describes how to query data stored in an
[Google Drive external table](https://docs.cloud.google.com/bigquery/docs/external-data-drive).

BigQuery supports queries against both personal Drive
files and shared files. For more information on Drive, see [Google Drive training and help](https://support.google.com/a/users/answer/9282958).

You can query Drive data from a
[permanent external table](https://docs.cloud.google.com/bigquery/docs/query-drive-data#permanent-tables) or from a
[temporary external table](https://docs.cloud.google.com/bigquery/docs/query-drive-data#temporary-tables) that you create when you run
the query.

## Limitations

For limitations related to external tables, see [external table
limitations](https://docs.cloud.google.com/bigquery/docs/external-tables#limitations).

## Required roles

To query Drive external tables, ensure you have the
following roles:

- BigQuery Data Viewer (`roles/bigquery.dataViewer`)
- BigQuery User (`roles/bigquery.user`)

Depending on your permissions, you can
grant these roles to yourself or ask your administrator
to grant them to you. For more information about granting roles, see
[Viewing the grantable roles on resources](https://docs.cloud.google.com/iam/docs/viewing-grantable-roles).

To see the exact BigQuery permissions that are required to query
external tables, expand the **Required permissions** section:

#### Required permissions

- `bigquery.jobs.create`
- `bigquery.readsessions.create` (Only required if you are [reading data with the
  BigQuery Storage Read API](https://docs.cloud.google.com/bigquery/docs/reference/storage))
- `bigquery.tables.get`
- `bigquery.tables.getData`

You might also be able to get these permissions with [custom roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles)
or other [predefined roles](https://docs.cloud.google.com/iam/docs/roles-overview#predefined).

## Drive permissions

At a minimum, to query external data in Drive you must be
granted [`View`](https://support.google.com/drive/answer/2494822?co=GENIE.Platform%3DDesktop)
access to the Drive file linked to the external table.

## Scopes for Compute Engine instances

When you create a Compute Engine instance, you can specify a list of scopes
for the instance. The scopes control the instance's access to Google Cloud
products, including Drive. Applications running on the VM use the service
account to call Google Cloud APIs.

If you set up a Compute Engine instance to run as a
[service account](https://docs.cloud.google.com/compute/docs/access/create-enable-service-accounts-for-instances),
and that service account accesses an external table linked to a Drive
data source, you must add the
[OAuth scope for Drive](https://developers.google.com/identity/protocols/googlescopes#drivev3)
(`https://www.googleapis.com/auth/drive.readonly`) to the instance.

For information on applying scopes to a Compute Engine instance,
see
[Changing the service account and access scopes for an instance](https://docs.cloud.google.com/compute/docs/access/create-enable-service-accounts-for-instances#changeserviceaccountandscopes).
For more information on Compute Engine service accounts, see
[Service accounts](https://docs.cloud.google.com/compute/docs/access/service-accounts).

## Query Drive data using permanent external tables

After creating a Drive external table, you can [query it using
GoogleSQL syntax](https://docs.cloud.google.com/bigquery/docs/running-queries), the same as if
it were a standard BigQuery table. For example, `SELECT field1, field2
FROM mydataset.my_drive_table;`.

## Query Drive data using temporary tables

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

### Create and query temporary tables

You can create and query a temporary table linked to an external data source
by using the bq command-line tool, the API, or the client libraries.

### bq

You query a temporary table linked to an external data source using the
`bq query` command with the `--external_table_definition` flag. When you use
the bq command-line tool to query a temporary table linked to an external data source, you
can identify the table's schema using:

- A [table definition file](https://docs.cloud.google.com/bigquery/external-table-definition) (stored on your local machine)
- An inline schema definition
- A JSON schema file (stored on your local machine)

To query a temporary table linked to your external data source using a table
definition file, enter the following command.

```bash
bq --location=LOCATION query \
--external_table_definition=TABLE::DEFINITION_FILE \
'QUERY'
```

Where:

- `LOCATION` is your [location](https://docs.cloud.google.com/bigquery/docs/locations). The `--location` flag is optional.
- `TABLE` is the name of the temporary table you're creating.
- `DEFINITION_FILE` is the path to the [table definition file](https://docs.cloud.google.com/bigquery/external-table-definition) on your local machine.
- `QUERY` is the query you're submitting to the temporary table.

For example, the following command creates and queries a temporary table
named `sales` using a table definition file named `sales_def`.

    bq query \
    --external_table_definition=sales::sales_def \
    'SELECT
       Region,Total_sales
     FROM
       sales'

To query a temporary table linked to your external data source using an
inline schema definition, enter the following command.

```bash
bq --location=LOCATION query \
--external_table_definition=TABLE::SCHEMA@SOURCE_FORMAT=DRIVE_URI \
'QUERY'
```

Where:

- `LOCATION` is your [location](https://docs.cloud.google.com/bigquery/docs/locations). The `--location` flag is optional.
- `TABLE` is the name of the temporary table you're creating.
- `SCHEMA` is the inline schema definition in the format `FIELD:DATA_TYPE,FIELD:DATA_TYPE`.
- `SOURCE_FORMAT` is `CSV`, `NEWLINE_DELIMITED_JSON`, `AVRO`, or `GOOGLE_SHEETS`.
- `DRIVE_URI` is your [Drive URI](https://docs.cloud.google.com/bigquery/docs/query-drive-data#drive-uri).
- `QUERY` is the query you're submitting to the temporary table.

For example, the following command creates and queries a temporary table
named `sales` linked to a CSV file stored in Drive with the following
schema definition: `Region:STRING,Quarter:STRING,Total_sales:INTEGER`.

    bq --location=US query \
    --external_table_definition=sales::Region:STRING,Quarter:STRING,Total_sales:INTEGER@CSV=https://drive.google.com/open?id=1234_AbCD12abCd \
    'SELECT
       Region,Total_sales
     FROM
       sales'

To query a temporary table linked to your external data source using a JSON
schema file, enter the following command.

```bash
bq --location=LOCATION query \
--external_table_definition=SCHEMA_FILE@SOURCE_FORMT=DRIVE_URI \
'QUERY'
```

Where:

- `LOCATION` is your [location](https://docs.cloud.google.com/bigquery/docs/locations). The `--location` flag is optional.
- `SCHEMA_FILE` is the path to the JSON schema file on your local machine.
- `SOURCE_FILE` is `CSV`, `NEWLINE_DELIMITED_JSON`, `AVRO`, or `GOOGLE_SHEETS`.
- `DRIVE_URI` is your [Drive URI](https://docs.cloud.google.com/bigquery/docs/query-drive-data#drive-uri).
- `QUERY` is the query you're submitting to the temporary table.

For example, the following command creates and queries a temporary table
named `sales` linked to a CSV file stored in Drive using the
`/tmp/sales_schema.json` schema file.

    bq query \
    --external_table_definition=sales::/tmp/sales_schema.json@CSV=https://drive.google.com/open?id=1234_AbCD12abCd \
    'SELECT
       Total_sales
     FROM
       sales'

### API

- Create a [query job configuration](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/Job#jobconfigurationquery).
  See [Querying data](https://docs.cloud.google.com/bigquery/querying-data) for information about calling
  [`jobs.query`](https://docs.cloud.google.com/bigquery/docs/reference/v2/jobs/query)
  and [`jobs.insert`](https://docs.cloud.google.com/bigquery/docs/reference/v2/jobs/insert).

- Specify the external data source by creating an
  [`ExternalDataConfiguration`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables#externaldataconfiguration).

### Python


Before trying this sample, follow the Python setup instructions in the
[BigQuery quickstart using
client libraries](https://docs.cloud.google.com/bigquery/docs/quickstarts/quickstart-client-libraries).


For more information, see the
[BigQuery Python API
reference documentation](https://docs.cloud.google.com/python/docs/reference/bigquery/latest).


To authenticate to BigQuery, set up Application Default Credentials.
For more information, see

[Set up authentication for client libraries](https://docs.cloud.google.com/bigquery/docs/authentication#client-libs).

    from google.cloud import https://docs.cloud.google.com/python/docs/reference/bigquery/latest
    import google.auth

    # Create credentials with Drive & BigQuery API scopes.
    # Both APIs must be enabled for your project before running this code.
    credentials, project = google.auth.default(
        scopes=[
            "https://www.googleapis.com/auth/drive",
            "https://www.googleapis.com/auth/bigquery",
        ]
    )

    # Construct a BigQuery client object.
    client = https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html(credentials=credentials, project=project)

    # Configure the external data source and query job.
    external_config = https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.external_config.ExternalConfig.html("GOOGLE_SHEETS")

    # Use a shareable link or grant viewing access to the email address you
    # used to authenticate with BigQuery (this example Sheet is public).
    sheet_url = (
        "https://docs.google.com/spreadsheets"
        "/d/1i_QCL-7HcSyUZmIbP9E6lO_T5u3HnpLe7dnpHaijg_E/edit?usp=sharing"
    )
    https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.external_config.html.source_uris = [sheet_url]
    https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.external_config.html.schema = [
        https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.schema.SchemaField.html("name", "STRING"),
        https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.schema.SchemaField.html("post_abbr", "STRING"),
    ]
    https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.external_config.html.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.external_config.ExternalConfig.html#google_cloud_bigquery_external_config_ExternalConfig_options.skip_leading_rows = 1  # Optionally skip header row.
    https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.external_config.html.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.external_config.ExternalConfig.html#google_cloud_bigquery_external_config_ExternalConfig_options.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.external_config.GoogleSheetsOptions.html#google_cloud_bigquery_external_config_GoogleSheetsOptions_range = (
        "us-states!A20:B49"  # Optionally set range of the sheet to query from.
    )
    table_id = "us_states"
    job_config = https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.job.QueryJobConfig.html(table_definitions={table_id: external_config})

    # Example query to find states starting with "W".
    sql = 'SELECT * FROM `{}` WHERE name LIKE "W%"'.format(table_id)

    query_job = client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html(sql, job_config=job_config)  # Make an API request.

    # Wait for the query to complete.
    w_states = list(query_job)
    print(
        "There are {} states with names starting with W in the selected range.".format(
            len(w_states)
        )
    )

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

    import com.google.auth.oauth2.https://docs.cloud.google.com/java/docs/reference/google-auth-library/latest/com.google.auth.oauth2.GoogleCredentials.html;
    import com.google.auth.oauth2.https://docs.cloud.google.com/java/docs/reference/google-auth-library/latest/com.google.auth.oauth2.ServiceAccountCredentials.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryException.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryOptions.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.ExternalTableDefinition.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Field.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.GoogleSheetsOptions.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.QueryJobConfiguration.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Schema.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.StandardSQLTypeName.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableResult.html;
    import com.google.common.collect.ImmutableSet;
    import java.io.IOException;

    // Sample to queries an external data source using a temporary table
    public class QueryExternalSheetsTemp {

      public static void main(String[] args) {
        // TODO(developer): Replace these variables before running the sample.
        String tableName = "MY_TABLE_NAME";
        String sourceUri =
            "https://docs.google.com/spreadsheets/d/1i_QCL-7HcSyUZmIbP9E6lO_T5u3HnpLe7dnpHaijg_E/edit?usp=sharing";
        https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Schema.html schema =
            https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Schema.html.of(
                https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Field.html.of("name", https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.StandardSQLTypeName.html.STRING),
                https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Field.html.of("post_abbr", https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.StandardSQLTypeName.html.STRING));
        String query = String.format("SELECT * FROM %s WHERE name LIKE 'W%%'", tableName);
        queryExternalSheetsTemp(tableName, sourceUri, schema, query);
      }

      public static void queryExternalSheetsTemp(
          String tableName, String sourceUri, https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Schema.html schema, String query) {
        try {

          // Create credentials with Drive & BigQuery API scopes.
          // Both APIs must be enabled for your project before running this code.
          https://docs.cloud.google.com/java/docs/reference/google-auth-library/latest/com.google.auth.oauth2.GoogleCredentials.html credentials =
              https://docs.cloud.google.com/java/docs/reference/google-auth-library/latest/com.google.auth.oauth2.ServiceAccountCredentials.html.https://docs.cloud.google.com/java/docs/reference/google-auth-library/latest/com.google.auth.oauth2.GoogleCredentials.html#com_google_auth_oauth2_GoogleCredentials_getApplicationDefault__()
                  .createScoped(
                      ImmutableSet.of(
                          "https://www.googleapis.com/auth/bigquery",
                          "https://www.googleapis.com/auth/drive"));

          // Initialize client that will be used to send requests. This client only needs to be created
          // once, and can be reused for multiple requests.
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html bigquery =
              https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryOptions.html.newBuilder().setCredentials(credentials).build().getService();

          // Skip header row in the file.
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.GoogleSheetsOptions.html sheetsOptions =
              https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.GoogleSheetsOptions.html.newBuilder()
                  .setSkipLeadingRows(1) // Optionally skip header row.
                  .setRange("us-states!A20:B49") // Optionally set range of the sheet to query from.
                  .build();

          // Configure the external data source and query job.
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.ExternalTableDefinition.html externalTable =
              https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.ExternalTableDefinition.html.newBuilder(sourceUri, sheetsOptions).setSchema(schema).build();
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.QueryJobConfiguration.html queryConfig =
              https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.QueryJobConfiguration.html.newBuilder(query)
                  .https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.QueryJobConfiguration.Builder.html#com_google_cloud_bigquery_QueryJobConfiguration_Builder_addTableDefinition_java_lang_String_com_google_cloud_bigquery_ExternalTableDefinition_(tableName, externalTable)
                  .build();

          // Example query to find states starting with 'W'
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableResult.html results = bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html#com_google_cloud_bigquery_BigQuery_query_com_google_cloud_bigquery_QueryJobConfiguration_com_google_cloud_bigquery_BigQuery_JobOption____(queryConfig);

          results
              .https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableResult.html#com_google_cloud_bigquery_TableResult_iterateAll__()
              .forEach(row -> row.forEach(val -> System.out.printf("%s,", val.toString())));

          System.out.println("Query on external temporary table performed successfully.");
        } catch (https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryException.html | InterruptedException | IOException e) {
          System.out.println("Query not performed \n" + e.toString());
        }
      }
    }

## Troubleshooting

Error string: `Resources exceeded during query execution: Google Sheets service
overloaded.`

This can be a transient error that can be fixed by rerunning the query. If the
error persists after a query rerun, consider simplifying your spreadsheet; for
example, by minimizing the use of formulas. For more information, see
[external table limitations](https://docs.cloud.google.com/bigquery/docs/external-tables#limitations).

## What's next

- Learn about [using SQL in BigQuery](https://docs.cloud.google.com/bigquery/docs/introduction-sql).
- Learn about [external tables](https://docs.cloud.google.com/bigquery/docs/external-tables).
- Learn about [BigQuery quotas](https://docs.cloud.google.com/bigquery/quotas).