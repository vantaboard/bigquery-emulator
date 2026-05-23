# Export data to Spanner (reverse ETL)

This document describes how you can set up a reverse extract, transform, and
load (reverse ETL) workflow from BigQuery to Spanner. You can
do this by using the
[`EXPORT DATA` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/export-statements)
to export data from BigQuery data sources, including
[Iceberg tables](https://docs.cloud.google.com/bigquery/docs/iceberg-tables), to a
[Spanner](https://docs.cloud.google.com/spanner/docs/overview) table.

This reverse ETL workflow combines analytic capabilities in
BigQuery with low latency and high throughput in
Spanner. This workflow lets you serve data to application users
without exhausting quotas and limits on BigQuery.

## Before you begin

- Create a
  [Spanner database](https://docs.cloud.google.com/spanner/docs/create-manage-databases)
  including a table to receive the exported data.

- Grant [Identity and Access Management (IAM) roles](https://docs.cloud.google.com/bigquery/docs/export-to-spanner#required_roles) that give users the
  necessary permissions to perform each task in this document.

- Create an [Enterprise or a higher tier reservation](https://docs.cloud.google.com/bigquery/docs/reservations-tasks#create_reservations).
  You might reduce BigQuery compute costs when you run one-time
  exports to Spanner by setting a baseline slot capacity of zero
  and enabling [autoscaling](https://docs.cloud.google.com/bigquery/docs/slots-autoscaling-intro).

### Required roles


To get the permissions that
you need to export BigQuery data to Spanner,

ask your administrator to grant you the
following IAM roles on your project:

- Export data from a BigQuery table: [BigQuery Data Viewer](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquery#bigquery.dataViewer) (`roles/bigquery.dataViewer`)
- Run an extract job: [BigQuery User](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquery#bigquery.user) (`roles/bigquery.user`)
- Check parameters of the Spanner instance: [Cloud Spanner Viewer](https://docs.cloud.google.com/iam/docs/roles-permissions/spanner#spanner.viewer) (`roles/spanner.viewer`)
- Write data to a Spanner table: [Cloud Spanner Database User](https://docs.cloud.google.com/iam/docs/roles-permissions/spanner#spanner.databaseUser) (`roles/spanner.databaseUser`)


For more information about granting roles, see [Manage access to projects, folders, and organizations](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).


You might also be able to get
the required permissions through [custom
roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles) or other [predefined
roles](https://docs.cloud.google.com/iam/docs/roles-overview#predefined).

## Limitations

- This feature is not supported in Assured Workloads.

- The following BigQuery data types don't have equivalents in
  Spanner and are not supported:

| Spanner database dialect | Unsupported BigQuery types |
|---|---|
| All dialects | - `STRUCT` - `GEOGRAPHY` - `DATETIME` - `RANGE` - `TIME` |
| GoogleSQL | - `BIGNUMERIC`: The supported `NUMERIC` type is not wide enough. Consider adding explicit casts to the `NUMERIC` type in the query. |

- The maximum size of an exported row cannot exceed 1 MiB.

- Spanner enforces referential integrity during the export.
  If the target table is a child of another table (INTERLEAVE IN PARENT), or if
  the target table has foreign key constraints, the foreign keys and parent key
  will be validated during the export. If an exported row is written to a
  table with INTERLEAVE IN PARENT and the parent row doesn't exist, the export
  will fail with "Parent row is missing. Row cannot be written" error. If the
  exported row is written to a table with foreign key constraints and is
  referencing a key that doesn't exist, the export will fail with
  "Foreign key constraint is violated" error. When exporting to multiple tables,
  we recommend sequencing the export to ensure that referential integrity will
  be maintained through the export. This usually means exporting parent tables
  and tables that are referenced by foreign keys before tables that reference
  them.

  If the table that is the target of the export has foreign key constraints,
  or is a child of another table (INTERLEAVE IN PARENT), the parent table must
  be populated before a child table export, and should contain all the
  corresponding keys. An attempt to export a child table while a parent table
  does not have the complete set of relevant keys will fail.
- A BigQuery job, such as an extract job to
  Spanner, has a maximum duration of 6 hours. For information
  about optimizing large extract jobs, see
  [Export optimization](https://docs.cloud.google.com/bigquery/docs/export-to-spanner#export_optimization). Alternatively, consider splitting
  the input into individual blocks of data, which may be exported as individual
  extract jobs.

- Exports to Spanner are only supported for the
  BigQuery Enterprise or Enterprise Plus
  editions. The BigQuery Standard edition and on-demand
  compute are not supported.

- You cannot use continuous queries to export to Spanner tables
  with [auto-generated primary keys](https://docs.cloud.google.com/spanner/docs/primary-key-default-value).

- You cannot use continuous queries to export to Spanner tables
  in a PostgreSQL-dialect database.

- When using continuous queries to export to a Spanner table,
  ensure that you choose a primary key that doesn't correspond to a monotonically
  increasing integer in your BigQuery table. Doing so might cause performance
  issues in your export. For information about primary keys in Spanner,
  and ways to mitigate these performance issues, see [Choose a primary key](https://docs.cloud.google.com/spanner/docs/schema-and-data-model#choose_a_primary_key).

## Configure exports with `spanner_options` option

You can use the `spanner_options` option to specify a destination
Spanner database and table. The configuration is expressed in
the form of a JSON string, as the following example shows:

    EXPORT DATA OPTIONS(
       uri="https://spanner.googleapis.com/projects/`PROJECT_ID`/instances/`INSTANCE_ID`/databases/`DATABASE_ID`",
      format='CLOUD_SPANNER',
       spanner_options = """{
          "table": "TABLE_NAME",
          "change_timestamp_column": "CHANGE_TIMESTAMP",
          "priority": "PRIORITY",
          "tag": "TAG",
       }"""
    )

Replace the following:

- `PROJECT_ID`: the name of your Google Cloud project.
- `INSTANCE_ID`: the name of your database instance.
- `DATABASE_ID`: the name of your database.
- `TABLE_NAME`: the name of an existing destination table.
- `CHANGE_TIMESTAMP`: the name of the `TIMESTAMP` type column in the destination Spanner table. This option is used during export to track the timestamp of the most recent row update. When this option is specified, the export first performs a read of the row in the Spanner table, to ensure that only the latest row update is written. We recommend specifying a `TIMESTAMP` type column when you run a [continuous export](https://docs.cloud.google.com/bigquery/docs/export-to-spanner#export_continuously), where the ordering of changes to rows with the same primary key is important.
- `PRIORITY` (optional): [priority](https://docs.cloud.google.com/spanner/docs/reference/rest/v1/RequestOptions#priority) of the write requests. Allowed values: `LOW`, `MEDIUM`, `HIGH`. Default value: `MEDIUM`.
- `TAG` (optional): [request tag](https://docs.cloud.google.com/spanner/docs/introspection/troubleshooting-with-tags) to help identify exporter traffic in Spanner monitoring. Default value: `bq_export`.

## Export query requirements

To export query results to Spanner, the results must meet the
following requirements:

- All columns in the result set must exist in the destination table, and their types must match or be [convertible](https://docs.cloud.google.com/bigquery/docs/export-to-spanner#type_conversions).
- The result set must contain all `NOT NULL` columns for the destination table.
- Column values must not exceed Spanner [data size limits within tables](https://docs.cloud.google.com/spanner/quotas#tables).
- Any unsupported column types must be converted to one of the supported types before exporting to Spanner.

### Type conversions

For ease of use, Spanner exporter automatically applies the
following type conversions:

| BigQuery type | Spanner type |
|---|---|
| BIGNUMERIC | NUMERIC (PostgreSQL dialect only) |
| FLOAT64 | FLOAT32 |
| BYTES | PROTO |
| INT64 | ENUM |

## Export data

You can use the
[`EXPORT DATA` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/export-statements)
to export data from a BigQuery table into a
Spanner table.

The following example exports selected fields from a table that's named
`mydataset.table1`:

```googlesql
EXPORT DATA OPTIONS (
  uri="https://spanner.googleapis.com/projects/`PROJECT_ID`/instances/`INSTANCE_ID`/databases/`DATABASE_ID`",
  format='CLOUD_SPANNER',
  spanner_options="""{ "table": "TABLE_NAME" }"""
)
AS SELECT * FROM mydataset.table1;
```

Replace the following:

- `PROJECT_ID`: the name of your Google Cloud project
- `INSTANCE_ID`: the name of your database instance
- `DATABASE_ID`: the name of your database
- `TABLE_NAME`: the name of an existing destination table

## Export multiple results with the same `rowkey` value

When you export a result containing multiple rows with the same `rowkey` value,
values written to Spanner end up in the same
Spanner row. Only single matching BigQuery row
(there is no guarantee which one) will be present in the Spanner
row set produced by export.

## Export using a `CLOUD_RESOURCE` Connection

You can delegate write permissions to a BigQuery [`CLOUD_RESOURCE`](https://docs.cloud.google.com/bigquery/docs/create-cloud-resource-connection) connection to
run exports without giving a user access direct access to the Spanner database.

Before you export to Spanner with a `CLOUD_RESOURCE` connection, do the following:

### Create a connection

You can create or use an existing
[`CLOUD_RESOURCE` connection](https://docs.cloud.google.com/bigquery/docs/create-cloud-resource-connection)
to connect to Spanner.
Select one of the following options:

<br />

### Console

1. Go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click
   **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)

   If you don't see the left pane, click
   **Expand left pane** to open the pane.
3. In the **Explorer** pane, expand your project name, and then click
   **Connections**.

4. On the **Connections** page, click **Create connection**.

5. For **Connection type** , choose **Vertex AI remote models, remote
   functions, BigLake and Spanner (Cloud Resource)**.

6. In the **Connection ID** field, enter a name for your connection.

7. For **Location type**, select a location for your connection. The
   connection should be colocated with your other resources such as
   datasets.

8. Click **Create connection**.

9. Click **Go to connection**.

10. In the **Connection info** pane, copy the service account ID for use in
    a later step.

### SQL

Use the [`CREATE CONNECTION` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_connection_statement):


1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, enter the following statement:

   ```googlesql
   CREATE CONNECTION [IF NOT EXISTS] `CONNECTION_NAME`
   OPTIONS (
     connection_type = "CLOUD_RESOURCE",
     friendly_name = "FRIENDLY_NAME",
     description = "DESCRIPTION"
     );
   ```


   Replace the following:
   - `CONNECTION_NAME`: the name of the connection in either the `PROJECT_ID.LOCATION.CONNECTION_ID`, `LOCATION.CONNECTION_ID`, or `CONNECTION_ID` format. If the project or location are omitted, then they are inferred from the project and location where the statement is run.
   - `FRIENDLY_NAME` (optional): a descriptive name for the connection.
   - `DESCRIPTION` (optional): a description of the connection.

   <br />

3. Click **Run**.

   <br />

For more information about how to run queries, see [Run an interactive query](https://docs.cloud.google.com/bigquery/docs/running-queries#queries).

### bq

1. In a command-line environment, create a connection:

   ```bash
   bq mk --connection --location=REGION --project_id=PROJECT_ID \
       --connection_type=CLOUD_RESOURCE CONNECTION_ID
   ```

   The `--project_id` parameter overrides the default project.

   Replace the following:
   - `REGION`: your [connection region](https://docs.cloud.google.com/bigquery/docs/locations#supported_locations)
   - `PROJECT_ID`: your Google Cloud project ID
   - `CONNECTION_ID`: an ID for your connection

   When you create a connection resource, BigQuery creates a
   unique system service account and associates it with the connection.

   **Troubleshooting** : If you get the following connection error,
   [update the Google Cloud SDK](https://docs.cloud.google.com/sdk/docs/quickstart):

   ```
   Flags parsing error: flag --connection_type=CLOUD_RESOURCE: value should be one of...
   ```
2. Retrieve and copy the service account ID for use in a later
   step:

   ```bash
   bq show --connection PROJECT_ID.REGION.CONNECTION_ID
   ```

   The output is similar to the following:

   ```
   name                          properties
   1234.REGION.CONNECTION_ID     {"serviceAccountId": "connection-1234-9u56h9@gcp-sa-bigquery-condel.iam.gserviceaccount.com"}
   ```

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

    import google.api_core.exceptions
    from google.cloud import https://docs.cloud.google.com/python/docs/reference/bigqueryconnection/latest

    client = https://docs.cloud.google.com/python/docs/reference/bigqueryconnection/latest.https://docs.cloud.google.com/python/docs/reference/bigqueryconnection/latest/google.cloud.bigquery_connection_v1.services.connection_service.ConnectionServiceClient.html()


    def create_connection(
        project_id: str,
        location: str,
        connection_id: str,
    ):
        """Creates a BigQuery connection to a Cloud Resource.

        Cloud Resource connection creates a service account which can then be
        granted access to other Google Cloud resources for federated queries.

        Args:
            project_id: The Google Cloud project ID.
            location: The location of the connection (for example, "us-central1").
            connection_id: The ID of the connection to create.
        """

        parent = client.https://docs.cloud.google.com/python/docs/reference/bigqueryconnection/latest/google.cloud.bigquery_connection_v1.services.connection_service.ConnectionServiceClient.html#google_cloud_bigquery_connection_v1_services_connection_service_ConnectionServiceClient_common_location_path(project_id, location)

        connection = https://docs.cloud.google.com/python/docs/reference/bigqueryconnection/latest.https://docs.cloud.google.com/python/docs/reference/bigqueryconnection/latest/google.cloud.bigquery_connection_v1.types.Connection.html(
            friendly_name="Example Connection",
            description="A sample connection for a Cloud Resource.",
            cloud_resource=https://docs.cloud.google.com/python/docs/reference/bigqueryconnection/latest.https://docs.cloud.google.com/python/docs/reference/bigqueryconnection/latest/google.cloud.bigquery_connection_v1.types.CloudResourceProperties.html(),
        )

        try:
            created_connection = client.https://docs.cloud.google.com/python/docs/reference/bigqueryconnection/latest/google.cloud.bigquery_connection_v1.services.connection_service.ConnectionServiceClient.html#google_cloud_bigquery_connection_v1_services_connection_service_ConnectionServiceClient_create_connection(
                parent=parent, connection_id=connection_id, connection=connection
            )
            print(f"Successfully created connection: {created_connection.name}")
            print(f"Friendly name: {created_connection.friendly_name}")
            print(
                f"Service Account: {created_connection.cloud_resource.service_account_id}"
            )

        except google.api_core.exceptions.AlreadyExists:
            print(f"Connection with ID '{connection_id}' already exists.")
            print("Please use a different connection ID.")
        except Exception as e:
            print(f"An unexpected error occurred while creating the connection: {e}")

### Node.js


Before trying this sample, follow the Node.js setup instructions in the
[BigQuery quickstart using
client libraries](https://docs.cloud.google.com/bigquery/docs/quickstarts/quickstart-client-libraries).


For more information, see the
[BigQuery Node.js API
reference documentation](https://googleapis.dev/nodejs/bigquery/latest/index.html).


To authenticate to BigQuery, set up Application Default Credentials.
For more information, see

[Set up authentication for client libraries](https://docs.cloud.google.com/bigquery/docs/authentication#client-libs).

    const {ConnectionServiceClient} =
      require('https://docs.cloud.google.com/nodejs/docs/reference/bigquery-connection/latest/overview.html').v1;
    const {status} = require('@grpc/grpc-js');

    const client = new https://docs.cloud.google.com/nodejs/docs/reference/bigquery-connection/latest/overview.html();

    /**
     * Creates a new BigQuery connection to a Cloud Resource.
     *
     * A Cloud Resource connection creates a service account that can be granted access
     * to other Google Cloud resources.
     *
     * @param {string} projectId The Google Cloud project ID. for example, 'example-project-id'
     * @param {string} location The location of the project to create the connection in. for example, 'us-central1'
     * @param {string} connectionId The ID of the connection to create. for example, 'example-connection-id'
     */
    async function createConnection(projectId, location, connectionId) {
      const parent = client.locationPath(projectId, location);

      const connection = {
        friendlyName: 'Example Connection',
        description: 'A sample connection for a Cloud Resource',
        // The service account for this cloudResource will be created by the API.
        // Its ID will be available in the response.
        cloudResource: {},
      };

      const request = {
        parent,
        connectionId,
        connection,
      };

      try {
        const [response] = await client.createConnection(request);

        console.log(`Successfully created connection: ${response.name}`);
        console.log(`Friendly name: ${response.friendlyName}`);

        console.log(`Service Account: ${response.cloudResource.serviceAccountId}`);
      } catch (err) {
        if (err.code === status.ALREADY_EXISTS) {
          console.log(`Connection '${connectionId}' already exists.`);
        } else {
          console.error(`Error creating connection: ${err.message}`);
        }
      }
    }

### Terraform

Use the
[`google_bigquery_connection`](https://registry.terraform.io/providers/hashicorp/google/latest/docs/resources/bigquery_connection)
resource.

> [!NOTE]
> **Note:** To create BigQuery objects using Terraform, you must enable the [Cloud Resource Manager API](https://docs.cloud.google.com/resource-manager/reference/rest).

To authenticate to BigQuery, set up Application Default
Credentials. For more information, see
[Set up authentication for client libraries](https://docs.cloud.google.com/bigquery/docs/authentication#client-libs).

The following example creates a Cloud resource connection named
`my_cloud_resource_connection` in the `US` region:


    # This queries the provider for project information.
    data "google_project" "default" {}

    # This creates a cloud resource connection in the US region named my_cloud_resource_connection.
    # Note: The cloud resource nested object has only one output field - serviceAccountId.
    resource "google_bigquery_connection" "default" {
      connection_id = "my_cloud_resource_connection"
      project       = data.google_project.default.project_id
      location      = "US"
      cloud_resource {}
    }

To apply your Terraform configuration in a Google Cloud project, complete the steps in the
following sections.

## Prepare Cloud Shell

1. Launch [Cloud Shell](https://shell.cloud.google.com/).
2. Set the default Google Cloud project
   where you want to apply your Terraform configurations.

   You only need to run this command once per project, and you can run it in any directory.

   ```
   export GOOGLE_CLOUD_PROJECT=PROJECT_ID
   ```

   Environment variables are overridden if you set explicit values in the Terraform
   configuration file.

## Prepare the directory

Each Terraform configuration file must have its own directory (also
called a *root module*).

1. In [Cloud Shell](https://shell.cloud.google.com/), create a directory and a new file within that directory. The filename must have the `.tf` extension---for example `main.tf`. In this tutorial, the file is referred to as `main.tf`.

   ```
   mkdir DIRECTORY && cd DIRECTORY && touch main.tf
   ```
2. If you are following a tutorial, you can copy the sample code in each section or step.

   Copy the sample code into the newly created `main.tf`.

   Optionally, copy the code from GitHub. This is recommended
   when the Terraform snippet is part of an end-to-end solution.
3. Review and modify the sample parameters to apply to your environment.
4. Save your changes.
5. Initialize Terraform. You only need to do this once per directory.

   ```
   terraform init
   ```

   Optionally, to use the latest Google provider version, include the `-upgrade`
   option:

   ```
   terraform init -upgrade
   ```

## Apply the changes

1. Review the configuration and verify that the resources that Terraform is going to create or update match your expectations:

   ```
   terraform plan
   ```

   Make corrections to the configuration as necessary.
2. Apply the Terraform configuration by running the following command and entering `yes` at the prompt:

   ```
   terraform apply
   ```

   Wait until Terraform displays the "Apply complete!" message.
3. [Open your Google Cloud project](https://console.cloud.google.com/) to view the results. In the Google Cloud console, navigate to your resources in the UI to make sure that Terraform has created or updated them.

> [!NOTE]
> **Note:** Terraform samples typically assume that the required APIs are enabled in your Google Cloud project.

After you create the connection, open it. In the **Connection info** pane,
copy the service account ID. You will need this ID when you configure permissions for
the connection. When you create a connection resource, BigQuery
creates a unique system service account and associates it with the connection.

### Set up access

You must give the service account that is associated with the new connection write access to your
Spanner instance or database. We recommend that you use the
**Cloud Spanner Database User** (`roles/spanner.databaseUser`) predefined IAM role.
These steps require the service account ID that you copied when you created your connection.

To grant access to database-level roles for the service account, do the following:

1. Go to the Spanner instances page.

   [Go to the instances page](https://console.cloud.google.com/spanner/instances)
2. Click the name of the instance that contains your database.

3. In the **Overview** tab, select the checkbox for your database.

4. The **Info panel** dialog appears. Click **Add principal**.

5. For **New principals**, enter the service account ID that you copied earlier.

6. In the **Select a role** field, select a role with `spanner.databases.write` permissions. We recommend that you use the **Cloud Spanner Database User** role.

7. Click **Save**.

### Run the export using the `CLOUD_RESOURCE` connection

With the connection created and the appropriate access granted to it, you can run the export using the `CLOUD_RESOURCE` connection. The following example shows an `EXPORT` command that exports with a `CLOUD_RESOURCE` connection.

```googlesql
EXPORT DATA WITH CONNECTION ``PROJECT_ID`.`LOCATION`.`CONNECTION_NAME`` OPTIONS (
  uri="https://spanner.googleapis.com/projects/`PROJECT_ID`/instances/`INSTANCE_ID`/databases/`DATABASE_ID`",
  format='CLOUD_SPANNER',
  spanner_options="""{ "table": "SPANNER_TABLE_NAME" }"""
)
AS SELECT * FROM my_bq_dataset.table1;
```

Replace the following:

- `PROJECT_ID`: the name of your Google Cloud project.
- `LOCATION`: the location where you created the connection---for example, `us`.
- `CONNECTION_NAME`: the name of the connection being used to run the export---for example, `myconnection`.
- `INSTANCE_ID`: the name of your Spanner database instance.
- `DATABASE_ID`: the name of your Spanner database.
- `SPANNER_TABLE_NAME`: The name of the existing destination Spanner table.

<br />

## Export continuously

To continuously process an export query, see
[Create continuous queries](https://docs.cloud.google.com/bigquery/docs/continuous-queries) for instructions
and [example code](https://docs.cloud.google.com/bigquery/docs/continuous-queries#spanner-example).

## Export optimization

To optimize the export of records from BigQuery to
Spanner, you can try the following:

- [Increase the number of nodes in the Spanner destination
  instance](https://docs.cloud.google.com/spanner/docs/compute-capacity). During the early stages of the
  export, increasing the number of nodes in the instance might not immediately
  increase export throughput. A slight delay can occur while
  Spanner performs
  [load-based splitting](https://docs.cloud.google.com/spanner/docs/schema-and-data-model#load-based_splitting).
  With load-based splitting, the export throughput grows and stabilizes. Using
  the `EXPORT DATA` statement batches data to optimize writes to
  Spanner. For more information, see
  [Performance overview](https://docs.cloud.google.com/spanner/docs/performance).

- Specify `HIGH` priority within [`spanner_options`](https://docs.cloud.google.com/bigquery/docs/export-to-spanner#spanner_options). If
  your Spanner instance has
  [autoscaling](https://docs.cloud.google.com/spanner/docs/autoscaling-overview) enabled, setting
  `HIGH` priority helps ensure that CPU utilization reaches the necessary
  threshold to trigger scaling. This allows the autoscaler to add compute
  resources in response to the export load, which can improve overall export
  throughput.

  > [!CAUTION]
  > **Caution:** using `HIGH` priority can cause significant performance degradation for other workloads served by the same Spanner instance. Consider using `HIGH` priority only if the Spanner instance is dedicated to this export, or if other workloads are not sensitive to performance impacts.

  The following example shows a Spanner export command set to
  `HIGH` priority:

      EXPORT DATA OPTIONS (
        uri="https://spanner.googleapis.com/projects/`PROJECT_ID`/instances/`INSTANCE_ID`/databases/`DATABASE_ID`",
        format='CLOUD_SPANNER',
        spanner_options="""{ "table": "TABLE_NAME", "priority": "LOW" }"""
      )

- Avoid ordering the query results. If the result set contains all primary key
  columns, the exporter automatically sorts the primary keys of the
  destination table to streamline writes and minimize contention.

  If the destination table's primary key includes generated columns, add the
  generated columns' expressions to the query to ensure that the exported data
  is sorted and batched properly.

  For example, in the following Spanner schema, `SaleYear`
  and `SaleMonth` are generated columns that make up the beginning of
  the Spanner primary key:

  ```sql
  CREATE TABLE Sales (
    SaleId STRING(36) NOT NULL,
    ProductId INT64 NOT NULL,
    SaleTimestamp TIMESTAMP NOT NULL,
    Amount FLOAT64,
    -- Generated columns
    SaleYear INT64 AS (EXTRACT(YEAR FROM SaleTimestamp)) STORED,
    SaleMonth INT64 AS (EXTRACT(MONTH FROM SaleTimestamp)) STORED,
  ) PRIMARY KEY (SaleYear, SaleMonth, SaleId);
  ```

  When you export data from BigQuery to a
  Spanner table with generated columns used in the primary
  key, it is recommended, but not required, to include the expressions
  for these generated columns in your `EXPORT DATA` query. This lets
  BigQuery pre-sort the data correctly, which is critical
  for efficient batching and writing to Spanner. The values
  for the generated columns in the `EXPORT DATA` statement aren't
  committed in Spanner, because they are
  auto-generated by Spanner, but they are used to
  optimize the export.

  The following example exports data to a Spanner `Sales` table
  whose primary key uses generated columns. To optimize write performance, the
  query includes `EXTRACT` expressions that match the generated `SaleYear` and
  `SaleMonth` columns, letting BigQuery pre-sort the data
  before export:

  ```googlesql
  EXPORT DATA OPTIONS (
    uri="https://spanner.googleapis.com/projects/`PROJECT_ID`/instances/`INSTANCE_ID`/databases/`DATABASE_ID`",
    format='CLOUD_SPANNER',
    spanner_options="""{ "table": "Sales" }"""
  )
  AS SELECT
    s.SaleId,
    s.ProductId,
    s.SaleTimestamp,
    s.Amount,
    -- Add expressions that match the generated columns in the Spanner PK
    EXTRACT(YEAR FROM s.SaleTimestamp) AS SaleYear,
    EXTRACT(MONTH FROM s.SaleTimestamp) AS SaleMonth
  FROM my_dataset.sales_export AS s;
  ```
- To prevent long running jobs, export data by partition. Shard your
  BigQuery data using a partition key, such as a timestamp in
  your query:

      EXPORT DATA OPTIONS (
        uri="https://spanner.googleapis.com/projects/`PROJECT_ID`/instances/`INSTANCE_ID`/databases/`DATABASE_ID`",
        format='CLOUD_SPANNER',
        spanner_options="""{ "table": "TABLE_NAME", "priority": "MEDIUM" }"""
      )
      AS SELECT *
      FROM 'mydataset.table1' d
      WHERE
      d.timestamp >= TIMESTAMP '2025-08-28T00:00:00Z' AND
      d.timestamp < TIMESTAMP '2025-08-29T00:00:00Z';

  This lets the query complete within the 6-hour job runtime. For more
  information about these limits, see the [query job limits](https://docs.cloud.google.com/bigquery/quotas#query_jobs).
- To improve data loading performance, drop the index in the
  Spanner table where data is imported. Then, recreate it after
  the import completes.

- We recommend starting with one Spanner node (1000 processor
  units) and a minimal BigQuery slot reservation. For example,
  100 slots, or 0 baseline slots with autoscaling. For exports under
  100 GB, this configuration typically completes within the 6-hour job
  limit. For exports larger than 100 GB, increase throughput by scaling
  up Spanner nodes and BigQuery slot
  reservations, as needed. Throughput scales at approximately 5 MiB/s per
  node.

## Pricing

When you export data to Spanner using the `EXPORT DATA` statement,
you are billed using
[BigQuery capacity compute pricing](https://cloud.google.com/bigquery/pricing#capacity_compute_analysis_pricing).

To export continuously to Spanner using a continuous query, you
must have a
[BigQuery Enterprise or Enterprise Plus edition](https://docs.cloud.google.com/bigquery/docs/editions-intro)
slot reservation and a [reservation assignment](https://docs.cloud.google.com/bigquery/docs/reservations-workload-management#assignments)
that uses the `CONTINUOUS` job type.

BigQuery exports to Spanner that cross regional
boundaries are charged using data extraction rates. For more information, see
[BigQuery pricing](https://cloud.google.com/bigquery/pricing#data_extraction_pricing).
To avoid data transfer charges, make sure that your BigQuery
export runs in the same region as the Spanner [default leader](https://docs.cloud.google.com/spanner/docs/instance-configurations#configure-leader-region).

After the data is exported, you're charged for storing the data in
Spanner. For more information,
see [Spanner pricing](https://cloud.google.com/spanner/pricing#storage).