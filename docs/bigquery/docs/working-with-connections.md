# Manage connections

This document describes how to view, list, share, edit, delete, and
troubleshoot a BigQuery connection.

As a BigQuery administrator, you can create and manage
connections that are used to connect to services and external data sources.
BigQuery analysts use these connections to submit queries
against external data sources without moving or copying data into
BigQuery. You can create the following types of connections:

- [Amazon S3 connections](https://docs.cloud.google.com/bigquery/docs/omni-aws-create-connection)
- [Apache Spark connections](https://docs.cloud.google.com/bigquery/docs/connect-to-spark)
- [Blob Storage connections](https://docs.cloud.google.com/bigquery/docs/omni-azure-create-connection)
- [Cloud resource connections](https://docs.cloud.google.com/bigquery/docs/create-cloud-resource-connection) to connect to Cloud Storage data and to implement [remote functions](https://docs.cloud.google.com/bigquery/docs/remote-functions)
- [Spanner connections](https://docs.cloud.google.com/bigquery/docs/connect-to-spanner)
- [Cloud SQL connections](https://docs.cloud.google.com/bigquery/docs/connect-to-sql)
- [AlloyDB connections](https://docs.cloud.google.com/bigquery/docs/connect-to-alloydb)

To create a default connection for a project, see the
[Default connection overview](https://docs.cloud.google.com/bigquery/docs/default-connections).

> [!TIP]
> **Tip:** You can also use the **Pipelines \& Connections** page to create and manage connections using a [streamlined, BigQuery-specific
> workflow](https://docs.cloud.google.com/bigquery/docs/pipeline-connection-page). This feature is in [preview](https://cloud.google.com/products/#product-launch-stages).

## Before you begin

- Ensure that you have a working [connection](https://docs.cloud.google.com/bigquery/docs/connections-api-intro).
  Connections are [type-specific](https://docs.cloud.google.com/bigquery/docs/connections-api-intro#connection_types)
  and depend on the connected external data source.

- Enable the BigQuery Connection API.

  [Enable the API](https://console.cloud.google.com/apis/library/bigqueryconnection.googleapis.com)
- Ensure that you can view a [list of service accounts in your
  project](https://docs.cloud.google.com/iam/docs/service-accounts-list-edit#listing).
  BigQuery creates and uses a [service account](https://docs.cloud.google.com/iam/docs/service-agents) to
  connect to your external data source. When you create a connection,
  a [Google Cloud--managed Identity and Access Management (IAM) service
  account](https://docs.cloud.google.com/iam/docs/service-account-types#google-managed) is created on your behalf.
  To view the service account attached to a particular connection,
  [view the connection details](https://docs.cloud.google.com/bigquery/docs/working-with-connections#view-connections).

### Required roles


To get the permissions that
you need to manage connections,

ask your administrator to grant you the
following IAM roles:

- [View connection details](https://docs.cloud.google.com/bigquery/docs/working-with-connections#view-connections): [BigQuery Connection User](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquery#bigquery.connectionUser) (`roles/bigquery.connectionUser`) on your dataset
- [List all connections](https://docs.cloud.google.com/bigquery/docs/working-with-connections#list-connections): [BigQuery Connection User](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquery#bigquery.connectionUser) (`roles/bigquery.connectionUser`) on your dataset
- [Share a connection](https://docs.cloud.google.com/bigquery/docs/working-with-connections#share-connections): [BigQuery Connection Admin](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquery#bigquery.connectionAdmin) (`roles/bigquery.connectionAdmin`) on your connection
- [Edit a connection](https://docs.cloud.google.com/bigquery/docs/working-with-connections#edit-connections): [BigQuery Connection Admin](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquery#bigquery.connectionAdmin) (`roles/bigquery.connectionAdmin`) on your connection
- [Delete a connection](https://docs.cloud.google.com/bigquery/docs/working-with-connections#delete-connections): [BigQuery Connection Admin](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquery#bigquery.connectionAdmin) (`roles/bigquery.connectionAdmin`) on your connection


For more information about granting roles, see [Manage access to projects, folders, and organizations](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).


You might also be able to get
the required permissions through [custom
roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles) or other [predefined
roles](https://docs.cloud.google.com/iam/docs/roles-overview#predefined).

For information about the roles needed to create and use a default
connection, see
[Required roles and permissions](https://docs.cloud.google.com/bigquery/docs/default-connections#required_roles_and_permissions).

These predefined roles contain the permissions required to perform the tasks in
this document. To see the exact permissions that are required, expand the
**Required permissions** section:

#### Required permissions

- View connection details: `bigquery.connections.get`
- List all connections: `bigquery.connections.list`
- Edit and delete a connection: `bigquery.connections.update`
- Share a connection: `bigquery.connections.setIamPolicy`

## List all connections

Select one of the following options:

### Console

1. Go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)

   Connections are listed in your project, in a group called **Connections**.
2. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)

   If you don't see the left pane, click **Expand left pane** to open the pane.
3. In the **Explorer** pane, click your project name, and then click
   **Connections** to see a list of all connections.

### bq

Enter the `bq ls` command and specify the `--connection` flag. Optionally,
specify the `--project_id` and `--location` flags to identify the project
and location of the connections to be listed.

```
bq ls --connection --project_id=PROJECT_ID --location=REGION
```

Replace the following:

- `PROJECT_ID`: your Google Cloud project ID
- `REGION`: the [connection region](https://docs.cloud.google.com/bigquery/docs/locations#supported_locations)

### API

[Use the `projects.locations.connections.list` method](https://docs.cloud.google.com/bigquery/docs/reference/bigqueryconnection/rest/v1/projects.locations.connections#methods)
in the REST API reference section.

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


    def list_connections(project_id: str, location: str):
        """Prints all connections in a given project and location.

        Args:
            project_id: The Google Cloud project ID.
            location: The geographic location of the connections (for example, "us", "us-central1").
        """
        client = https://docs.cloud.google.com/python/docs/reference/bigqueryconnection/latest.https://docs.cloud.google.com/python/docs/reference/bigqueryconnection/latest/google.cloud.bigquery_connection_v1.services.connection_service.ConnectionServiceClient.html()

        parent = client.https://docs.cloud.google.com/python/docs/reference/bigqueryconnection/latest/google.cloud.bigquery_connection_v1.services.connection_service.ConnectionServiceClient.html#google_cloud_bigquery_connection_v1_services_connection_service_ConnectionServiceClient_common_location_path(project_id, location)

        request = https://docs.cloud.google.com/python/docs/reference/bigqueryconnection/latest.https://docs.cloud.google.com/python/docs/reference/bigqueryconnection/latest/google.cloud.bigquery_connection_v1.types.ListConnectionsRequest.html(
            parent=parent,
            page_size=100,
        )

        print(f"Listing connections in project '{project_id}' and location '{location}':")

        try:
            for connection in client.https://docs.cloud.google.com/python/docs/reference/bigqueryconnection/latest/google.cloud.bigquery_connection_v1.services.connection_service.ConnectionServiceClient.html#google_cloud_bigquery_connection_v1_services_connection_service_ConnectionServiceClient_list_connections(request=request):
                print(f"Connection ID: {connection.name.split('/')[-1]}")
                print(f"Friendly Name: {connection.friendly_name}")
                print(f"Has Credential: {connection.has_credential}")
                print("-" * 20)

            print("Finished listing connections.")

        except google.api_core.exceptions.InvalidArgument as e:
            print(
                f"Could not list connections. Please check that the project ID '{project_id}' "
                f"and location '{location}' are correct. Details: {e}"
            )

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

    const {ConnectionServiceClient} = require('https://docs.cloud.google.com/nodejs/docs/reference/bigquery-connection/latest/overview.html');
    const {status} = require('@grpc/grpc-js');

    const client = new https://docs.cloud.google.com/nodejs/docs/reference/bigquery-connection/latest/overview.html();

    /**
     * Lists BigQuery connections in a given project and location.
     *
     * @param {string} projectId The Google Cloud project ID. for example, 'example-project-id'
     * @param {string} location The location to list connections for. for example, 'us-central1'
     */
    async function listConnections(projectId, location) {
      const parent = client.locationPath(projectId, location);

      const request = {
        parent,
        pageSize: 100,
      };

      try {
        const [connections] = await client.listConnections(request, {
          autoPaginate: false,
        });

        if (connections.length === 0) {
          console.log(
            `No connections found in ${location} for project ${projectId}.`,
          );
          return;
        }

        console.log('Connections:');
        for (const connection of connections) {
          console.log(`  Name: ${connection.name}`);
          console.log(`  Friendly Name: ${connection.friendlyName}`);
        }
      } catch (err) {
        if (err.code === status.NOT_FOUND) {
          console.log(
            `Project '${projectId}' or location '${location}' not found.`,
          );
        } else {
          console.error('Error listing connections:', err);
        }
      }
    }

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

    import com.google.cloud.bigquery.connection.v1.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigqueryconnection/latest/com.google.cloud.bigquery.connection.v1.ListConnectionsRequest.html;
    import com.google.cloud.bigquery.connection.v1.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigqueryconnection/latest/com.google.cloud.bigquery.connection.v1.LocationName.html;
    import com.google.cloud.bigqueryconnection.v1.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigqueryconnection/latest/com.google.cloud.bigqueryconnection.v1.ConnectionServiceClient.html;
    import java.io.IOException;

    // Sample to get list of connections
    public class ListConnections {

      public static void main(String[] args) throws IOException {
        // TODO(developer): Replace these variables before running the sample.
        String projectId = "MY_PROJECT_ID";
        String location = "MY_LOCATION";
        listConnections(projectId, location);
      }

      static void listConnections(String projectId, String location) throws IOException {
        try (https://docs.cloud.google.com/java/docs/reference/google-cloud-bigqueryconnection/latest/com.google.cloud.bigqueryconnection.v1.ConnectionServiceClient.html client = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigqueryconnection/latest/com.google.cloud.bigqueryconnection.v1.ConnectionServiceClient.html.create()) {
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigqueryconnection/latest/com.google.cloud.bigquery.connection.v1.LocationName.html parent = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigqueryconnection/latest/com.google.cloud.bigquery.connection.v1.LocationName.html.of(projectId, location);
          int pageSize = 10;
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigqueryconnection/latest/com.google.cloud.bigquery.connection.v1.ListConnectionsRequest.html request =
              https://docs.cloud.google.com/java/docs/reference/google-cloud-bigqueryconnection/latest/com.google.cloud.bigquery.connection.v1.ListConnectionsRequest.html.newBuilder()
                  .setParent(parent.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigqueryconnection/latest/com.google.cloud.bigquery.connection.v1.LocationName.html#com_google_cloud_bigquery_connection_v1_LocationName_toString__())
                  .https://docs.cloud.google.com/java/docs/reference/google-cloud-bigqueryconnection/latest/com.google.cloud.bigquery.connection.v1.ListConnectionsRequest.Builder.html#com_google_cloud_bigquery_connection_v1_ListConnectionsRequest_Builder_setPageSize_int_(pageSize)
                  .build();
          client
              .listConnections(request)
              .iterateAll()
              .forEach(con -> System.out.println("Connection Id :" + con.getName()));
        }
      }
    }

## View connection details

After you create a connection, you can get information about the
connection's configuration. The configuration includes the values you
supplied when you created the transfer.

Select one of the following options:

### Console

1. Go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. Connections are listed in your project, in a group called **Connections**.

3. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)
4. In the **Explorer** pane, click your project name, and then click
   **Connections** to see a list of all connections.

5. Click the connection to see the details.

### bq

Enter the `bq show` command and specify the `--connection` flag. Optionally,
qualify the connection ID with the project ID and region of the connection.

```
bq show --connection PROJECT_ID.REGION.CONNECTION_ID
```

Replace the following:

- `PROJECT_ID`: your Google Cloud project ID
- `REGION`: the [connection region](https://docs.cloud.google.com/bigquery/docs/locations#supported_locations)
- `CONNECTION_ID`: the connection ID

### API

Use the
[`projects.locations.connections.get` method](https://docs.cloud.google.com/bigquery/docs/reference/bigqueryconnection/rest/v1/projects.locations.connections#methods)
in the REST API reference section.

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


    def get_connection(project_id: str, location: str, connection_id: str):
        """Retrieves connection metadata about a specified BigQuery connection.

        A connection stores metadata about an external data source and credentials to access it.

        Args:
            project_id: The Google Cloud project ID.
            location: The geographic location of the connection (for example, "us-central1").
            connection_id: The ID of the connection to retrieve.
        """

        name = client.https://docs.cloud.google.com/python/docs/reference/bigqueryconnection/latest/google.cloud.bigquery_connection_v1.services.connection_service.ConnectionServiceClient.html#google_cloud_bigquery_connection_v1_services_connection_service_ConnectionServiceClient_connection_path(project_id, location, connection_id)

        try:
            connection = client.https://docs.cloud.google.com/python/docs/reference/bigqueryconnection/latest/google.cloud.bigquery_connection_v1.services.connection_service.ConnectionServiceClient.html#google_cloud_bigquery_connection_v1_services_connection_service_ConnectionServiceClient_get_connection(name=name)

            print(f"Successfully retrieved connection: {connection.name}")
            print(f"Friendly name: {connection.friendly_name}")
            print(f"Description: {connection.description}")
            if connection.cloud_sql:
                print(f"Cloud SQL instance ID: {connection.cloud_sql.instance_id}")
        except google.api_core.exceptions.NotFound:
            print(f"Connection '{name}' not found.")

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
     * Retrieves connection metadata about a specified BigQuery connection.
     *
     * A connection stores metadata about an external data source and credentials to access it.
     *
     * @param {string} projectId - Google Cloud project ID. for example, 'example-project-id'
     * @param {string} location - The location of the connection. for example, 'us-central1'
     * @param {string} connectionId - The ID of the connection to retrieve. for example, 'example_connection'
     */
    async function getConnection(projectId, location, connectionId) {
      const name = client.connectionPath(projectId, location, connectionId);

      const request = {
        name,
      };

      try {
        const [connection] = await client.getConnection(request);

        console.log(`Successfully retrieved connection: ${connection.name}`);
        console.log(`  Friendly name: ${connection.friendlyName}`);
        console.log(`  Description: ${connection.description}`);
        console.log(`  Has credential: ${connection.hasCredential}`);

        if (connection.cloudSql) {
          console.log(`  Cloud SQL instance ID: ${connection.cloudSql.instanceId}`);
          console.log(`  Cloud SQL database: ${connection.cloudSql.database}`);
          console.log(`  Cloud SQL type: ${connection.cloudSql.type}`);
        }
      } catch (err) {
        if (err.code === status.NOT_FOUND) {
          console.log(`Connection ${name} not found.`);
        } else {
          console.error(`Error getting connection ${name}:`, err);
        }
      }
    }

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

    import com.google.cloud.bigquery.connection.v1.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigqueryconnection/latest/com.google.cloud.bigquery.connection.v1.Connection.html;
    import com.google.cloud.bigquery.connection.v1.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigqueryconnection/latest/com.google.cloud.bigquery.connection.v1.ConnectionName.html;
    import com.google.cloud.bigquery.connection.v1.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigqueryconnection/latest/com.google.cloud.bigquery.connection.v1.GetConnectionRequest.html;
    import com.google.cloud.bigqueryconnection.v1.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigqueryconnection/latest/com.google.cloud.bigqueryconnection.v1.ConnectionServiceClient.html;
    import java.io.IOException;

    // Sample to get connection
    public class GetConnection {

      public static void main(String[] args) throws IOException {
        // TODO(developer): Replace these variables before running the sample.
        String projectId = "MY_PROJECT_ID";
        String location = "MY_LOCATION";
        String connectionId = "MY_CONNECTION_ID";
        getConnection(projectId, location, connectionId);
      }

      static void getConnection(String projectId, String location, String connectionId)
          throws IOException {
        try (https://docs.cloud.google.com/java/docs/reference/google-cloud-bigqueryconnection/latest/com.google.cloud.bigqueryconnection.v1.ConnectionServiceClient.html client = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigqueryconnection/latest/com.google.cloud.bigqueryconnection.v1.ConnectionServiceClient.html.create()) {
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigqueryconnection/latest/com.google.cloud.bigquery.connection.v1.ConnectionName.html name = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigqueryconnection/latest/com.google.cloud.bigquery.connection.v1.ConnectionName.html.of(projectId, location, connectionId);
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigqueryconnection/latest/com.google.cloud.bigquery.connection.v1.GetConnectionRequest.html request =
              https://docs.cloud.google.com/java/docs/reference/google-cloud-bigqueryconnection/latest/com.google.cloud.bigquery.connection.v1.GetConnectionRequest.html.newBuilder().setName(name.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigqueryconnection/latest/com.google.cloud.bigquery.connection.v1.ConnectionName.html#com_google_cloud_bigquery_connection_v1_ConnectionName_toString__()).build();
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigqueryconnection/latest/com.google.cloud.bigquery.connection.v1.Connection.html response = client.getConnection(request);
          System.out.println("Connection info retrieved successfully :" + response.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigqueryconnection/latest/com.google.cloud.bigquery.connection.v1.Connection.html#com_google_cloud_bigquery_connection_v1_Connection_getName__());
        }
      }
    }

## Get the Identity and Access Management (IAM) policy for a connection

Follow these steps to get the IAM policy for a connection:

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


    def get_connection_iam_policy(
        project_id: str,
        location: str,
        connection_id: str,
    ):
        """Gets the IAM policy of a connection.

        Args:
            project_id: The Google Cloud project ID.
            location: The geographic location of the connection (for example, "us").
            connection_id: The ID of the connection.
        """

        resource = client.https://docs.cloud.google.com/python/docs/reference/bigqueryconnection/latest/google.cloud.bigquery_connection_v1.services.connection_service.ConnectionServiceClient.html#google_cloud_bigquery_connection_v1_services_connection_service_ConnectionServiceClient_connection_path(project_id, location, connection_id)

        try:
            policy = client.https://docs.cloud.google.com/python/docs/reference/bigqueryconnection/latest/google.cloud.bigquery_connection_v1.services.connection_service.ConnectionServiceClient.html#google_cloud_bigquery_connection_v1_services_connection_service_ConnectionServiceClient_get_iam_policy(resource=resource)

            print(f"Successfully retrieved IAM policy for connection: {resource}")
            if not policy.bindings:
                print("This policy is empty and has no bindings.")

            for binding in policy.bindings:
                print(f"Role: {binding.role}")
                print("Members:")
                for member in binding.members:
                    print(f"    - {member}")

        except google.api_core.exceptions.NotFound:
            print(f"Connection not found: {resource}")

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
     * Gets the IAM policy for a BigQuery connection.
     * @param {string} projectId Google Cloud project ID (for example, 'example-project-id').
     * @param {string} location The location of the connection (for example, 'us').
     * @param {string} connectionId The connection ID (for example, 'example-connection').
     */
    async function getIamPolicy(projectId, location, connectionId) {

      const resource = client.connectionPath(projectId, location, connectionId);

      const request = {
        resource,
      };

      try {

        const [policy] = await client.getIamPolicy(request);

        console.log(
          `Successfully retrieved IAM policy for connection: ${connectionId}`,
        );

        if (policy.bindings && policy.bindings.length > 0) {
          console.log('Bindings:');
          policy.bindings.forEach(binding => {
            console.log(`  Role: ${binding.role}`);
            console.log('  Members:');
            binding.members.forEach(member => {
              console.log(`    - ${member}`);
            });
          });
        } else {
          console.log('No policy bindings found.');
        }
      } catch (err) {

        if (err.code === status.NOT_FOUND) {
          console.log(
            `Connection '${connectionId}' not found in project '${projectId}' at location '${location}'.`,
          );
        } else {
          console.error('An error occurred while getting the IAM policy:', err);
        }
      }
    }

## Share a connection with users

You can grant the following roles to let users query data and manage connections:

- `roles/bigquery.connectionUser`: enables users to use connections to connect
  with external data sources and run queries on them.

- `roles/bigquery.connectionAdmin`: enables users to manage connections.

For more information about IAM roles and permissions in
BigQuery, see [Predefined roles and permissions](https://docs.cloud.google.com/bigquery/access-control).

Select one of the following options:

### Console

1. Go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)

   Connections are listed in your project, in a group called **Connections**.
2. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)

   If you don't see the left pane, click **Expand left pane** to open the pane.
3. Click your project, click **Connections**, and then select a connection.

4. In the **Details** pane, click **Share** to share a connection.
   Then do the following:

   1. In the **Connection permissions** dialog, share the
      connection with other principals by adding or editing
      principals.

   2. Click **Save**.

### bq

You cannot share a connection with the bq command-line tool.
To share a connection, use the Google Cloud console or
the BigQuery Connections API method to share a connection.

### API

Use the
[`projects.locations.connections.setIAM` method](https://docs.cloud.google.com/bigquery/docs/reference/bigqueryconnection/rest/v1/projects.locations.connections#methods)
in the BigQuery Connections REST API reference section, and
supply an instance of the `policy` resource.

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

    import com.google.api.resourcenames.https://docs.cloud.google.com/java/docs/reference/api-common/latest/com.google.api.resourcenames.ResourceName.html;
    import com.google.cloud.bigquery.connection.v1.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigqueryconnection/latest/com.google.cloud.bigquery.connection.v1.ConnectionName.html;
    import com.google.cloud.bigqueryconnection.v1.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigqueryconnection/latest/com.google.cloud.bigqueryconnection.v1.ConnectionServiceClient.html;
    import com.google.iam.v1.https://docs.cloud.google.com/java/docs/reference/proto-google-iam-v1/latest/com.google.iam.v1.Binding.html;
    import com.google.iam.v1.https://docs.cloud.google.com/java/docs/reference/proto-google-iam-v1/latest/com.google.iam.v1.Policy.html;
    import com.google.iam.v1.https://docs.cloud.google.com/java/docs/reference/proto-google-iam-v1/latest/com.google.iam.v1.SetIamPolicyRequest.html;
    import java.io.IOException;

    // Sample to share connections
    public class ShareConnection {

      public static void main(String[] args) throws IOException {
        // TODO(developer): Replace these variables before running the sample.
        String projectId = "MY_PROJECT_ID";
        String location = "MY_LOCATION";
        String connectionId = "MY_CONNECTION_ID";
        shareConnection(projectId, location, connectionId);
      }

      static void shareConnection(String projectId, String location, String connectionId)
          throws IOException {
        try (https://docs.cloud.google.com/java/docs/reference/google-cloud-bigqueryconnection/latest/com.google.cloud.bigqueryconnection.v1.ConnectionServiceClient.html client = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigqueryconnection/latest/com.google.cloud.bigqueryconnection.v1.ConnectionServiceClient.html.create()) {
          https://docs.cloud.google.com/java/docs/reference/api-common/latest/com.google.api.resourcenames.ResourceName.html resource = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigqueryconnection/latest/com.google.cloud.bigquery.connection.v1.ConnectionName.html.of(projectId, location, connectionId);
          https://docs.cloud.google.com/java/docs/reference/proto-google-iam-v1/latest/com.google.iam.v1.Binding.html binding =
              https://docs.cloud.google.com/java/docs/reference/proto-google-iam-v1/latest/com.google.iam.v1.Binding.html.newBuilder()
                  .https://docs.cloud.google.com/java/docs/reference/proto-google-iam-v1/latest/com.google.iam.v1.Binding.Builder.html#com_google_iam_v1_Binding_Builder_addMembers_java_lang_String_("group:example-analyst-group@google.com")
                  .setRole("roles/bigquery.connectionUser")
                  .build();
          https://docs.cloud.google.com/java/docs/reference/proto-google-iam-v1/latest/com.google.iam.v1.Policy.html policy = https://docs.cloud.google.com/java/docs/reference/proto-google-iam-v1/latest/com.google.iam.v1.Policy.html.newBuilder().https://docs.cloud.google.com/java/docs/reference/proto-google-iam-v1/latest/com.google.iam.v1.Policy.Builder.html#com_google_iam_v1_Policy_Builder_addBindings_com_google_iam_v1_Binding_(binding).build();
          https://docs.cloud.google.com/java/docs/reference/proto-google-iam-v1/latest/com.google.iam.v1.SetIamPolicyRequest.html request =
              https://docs.cloud.google.com/java/docs/reference/proto-google-iam-v1/latest/com.google.iam.v1.SetIamPolicyRequest.html.newBuilder()
                  .setResource(resource.toString())
                  .https://docs.cloud.google.com/java/docs/reference/proto-google-iam-v1/latest/com.google.iam.v1.SetIamPolicyRequest.Builder.html#com_google_iam_v1_SetIamPolicyRequest_Builder_setPolicy_com_google_iam_v1_Policy_(policy)
                  .build();
          client.setIamPolicy(request);
          System.out.println("Connection shared successfully");
        }
      }
    }

## Edit a connection

A connection uses the credentials of the user who created it. If you need to
change the user attached to a connection, you can update the
user's credentials. This is useful if the user who created the connection is
no longer with your organization.

You cannot edit the following elements of a connection:

- Connection type
- Connection ID
- Location

Select one of the following options:

### Console

1. Go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)

   Connections are listed in your project,
   in a group called **Connections**.
2. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)
3. In the **Explorer** pane, click your project name, and then click
   **Connections** to see a list of all connections.

4. To see the details, click the connection.

5. In the **Connection info** section, click **Edit details**. Then do the following:

   1. In the **Edit connection** dialog, edit the connection details
      including the user credentials.

   2. Click **Update connection**.

### bq

Enter the `bq update` command and supply the connection flag:
`--connection`. The fully qualified `connection_id` is required.

<br />

```
  bq update --connection --connection_type='CLOUD_SQL'
      --properties='{"instanceId" : "INSTANCE",
      "database" : "DATABASE", "type" : "MYSQL" }'
      --connection_credential='{"username":"USERNAME", "password":"PASSWORD"}'
      PROJECT.REGION.CONNECTION_ID
 
```

<br />

Replace the following:

- `INSTANCE`: the Cloud SQL instance
- `DATABASE`: the database name
- `USERNAME`: the username of your Cloud SQL database
- `PASSWORD`: the password to your Cloud SQL database
- `PROJECT`: the Google Cloud project ID
- `REGION`: the [connection region](https://docs.cloud.google.com/bigquery/docs/locations#supported_locations)
- `CONNECTION_ID`: the connection ID

For example, the following command updates the connection in a
project with the ID `federation-test` and connection ID `test-mysql`.

```
bq update --connection --connection_type='CLOUD_SQL'
    --properties='{"instanceId" : "federation-test:us-central1:new-mysql",
    "database" : "imdb2", "type" : "MYSQL" }'
    --connection_credential='{"username":"my_username",
    "password":"my_password"}' federation-test.us.test-mysql
```

### API

See the
[`projects.locations.connections.patch` method](https://docs.cloud.google.com/bigquery/docs/reference/bigqueryconnection/rest/v1/projects.locations.connections#methods)
in the REST API reference section, and supply an instance of the `connection`.

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
    from google.protobuf import field_mask_pb2

    client = https://docs.cloud.google.com/python/docs/reference/bigqueryconnection/latest.https://docs.cloud.google.com/python/docs/reference/bigqueryconnection/latest/google.cloud.bigquery_connection_v1.services.connection_service.ConnectionServiceClient.html()


    def update_connection(project_id: str, location: str, connection_id: str):
        """Updates a BigQuery connection's friendly name and description.

        For security reasons, updating connection properties also resets the
        credential. The `update_mask` specifies which fields of the connection
        to update. This sample only updates metadata fields to avoid resetting
        credentials.

        Args:
            project_id: The Google Cloud project ID.
            location: The geographic location of the connection (for example, "us-central1").
            connection_id: The ID of the connection to update.
        """

        connection_name = client.https://docs.cloud.google.com/python/docs/reference/bigqueryconnection/latest/google.cloud.bigquery_connection_v1.services.connection_service.ConnectionServiceClient.html#google_cloud_bigquery_connection_v1_services_connection_service_ConnectionServiceClient_connection_path(project_id, location, connection_id)

        connection = https://docs.cloud.google.com/python/docs/reference/bigqueryconnection/latest.https://docs.cloud.google.com/python/docs/reference/bigqueryconnection/latest/google.cloud.bigquery_connection_v1.types.Connection.html(
            friendly_name="Example Updated BigQuery Connection",
            description="This is an updated description for the connection.",
        )

        update_mask = field_mask_pb2.FieldMask(paths=["friendly_name", "description"])

        try:
            response = client.https://docs.cloud.google.com/python/docs/reference/bigqueryconnection/latest/google.cloud.bigquery_connection_v1.services.connection_service.ConnectionServiceClient.html#google_cloud_bigquery_connection_v1_services_connection_service_ConnectionServiceClient_update_connection(
                name=connection_name,
                connection=connection,
                update_mask=update_mask,
            )

            print(f"Connection '{response.name}' updated successfully.")
            print(f"Friendly Name: {response.friendly_name}")
            print(f"Description: {response.description}")

        except google.api_core.exceptions.NotFound:
            print(f"Connection '{connection_name}' not found. Please create it first.")

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

    const connectionClient = new https://docs.cloud.google.com/nodejs/docs/reference/bigquery-connection/latest/overview.html();

    /**
     * Updates a BigQuery connection, demonstrating how to update the friendly name and description.
     *
     * @param {string} projectId The Google Cloud project ID. for example, 'example-project-id'
     * @param {string} location The location of the connection. for example, 'us-central1'
     * @param {string} connectionId The ID of the connection to update. for example, 'example-connection-id'
     */
    async function updateConnection(projectId, location, connectionId) {
      const name = connectionClient.connectionPath(
        projectId,
        location,
        connectionId,
      );

      const connection = {
        friendlyName: 'Example Updated Connection',
        description: 'A new description for the connection',
      };

      const updateMask = {
        paths: ['friendly_name', 'description'],
      };

      const request = {
        name,
        connection,
        updateMask,
      };

      try {
        const [response] = await connectionClient.updateConnection(request);

        console.log(`Connection updated: ${response.name}`);
        console.log(`  Friendly name: ${response.friendlyName}`);
        console.log(`  Description: ${response.description}`);
      } catch (err) {
        if (err.code === status.NOT_FOUND) {
          console.log(`Connection not found: ${name}`);
        } else {
          console.error(`Error updating connection ${name}:`, err);
        }
      }
    }

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

    import com.google.cloud.bigquery.connection.v1.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigqueryconnection/latest/com.google.cloud.bigquery.connection.v1.Connection.html;
    import com.google.cloud.bigquery.connection.v1.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigqueryconnection/latest/com.google.cloud.bigquery.connection.v1.ConnectionName.html;
    import com.google.cloud.bigquery.connection.v1.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigqueryconnection/latest/com.google.cloud.bigquery.connection.v1.UpdateConnectionRequest.html;
    import com.google.cloud.bigqueryconnection.v1.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigqueryconnection/latest/com.google.cloud.bigqueryconnection.v1.ConnectionServiceClient.html;
    import com.google.protobuf.https://docs.cloud.google.com/java/docs/reference/protobuf/latest/com.google.protobuf.FieldMask.html;
    import com.google.protobuf.util.https://docs.cloud.google.com/java/docs/reference/protobuf/latest/com.google.protobuf.util.FieldMaskUtil.html;
    import java.io.IOException;

    // Sample to update connection
    public class UpdateConnection {

      public static void main(String[] args) throws IOException {
        // TODO(developer): Replace these variables before running the sample.
        String projectId = "MY_PROJECT_ID";
        String location = "MY_LOCATION";
        String connectionId = "MY_CONNECTION_ID";
        String description = "MY_DESCRIPTION";
        https://docs.cloud.google.com/java/docs/reference/google-cloud-bigqueryconnection/latest/com.google.cloud.bigquery.connection.v1.Connection.html connection = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigqueryconnection/latest/com.google.cloud.bigquery.connection.v1.Connection.html.newBuilder().https://docs.cloud.google.com/java/docs/reference/google-cloud-bigqueryconnection/latest/com.google.cloud.bigquery.connection.v1.Connection.Builder.html#com_google_cloud_bigquery_connection_v1_Connection_Builder_setDescription_java_lang_String_(description).build();
        updateConnection(projectId, location, connectionId, connection);
      }

      static void updateConnection(
          String projectId, String location, String connectionId, https://docs.cloud.google.com/java/docs/reference/google-cloud-bigqueryconnection/latest/com.google.cloud.bigquery.connection.v1.Connection.html connection)
          throws IOException {
        try (https://docs.cloud.google.com/java/docs/reference/google-cloud-bigqueryconnection/latest/com.google.cloud.bigqueryconnection.v1.ConnectionServiceClient.html client = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigqueryconnection/latest/com.google.cloud.bigqueryconnection.v1.ConnectionServiceClient.html.create()) {
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigqueryconnection/latest/com.google.cloud.bigquery.connection.v1.ConnectionName.html name = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigqueryconnection/latest/com.google.cloud.bigquery.connection.v1.ConnectionName.html.of(projectId, location, connectionId);
          https://docs.cloud.google.com/java/docs/reference/protobuf/latest/com.google.protobuf.FieldMask.html updateMask = https://docs.cloud.google.com/java/docs/reference/protobuf/latest/com.google.protobuf.util.FieldMaskUtil.html.fromString("description");
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigqueryconnection/latest/com.google.cloud.bigquery.connection.v1.UpdateConnectionRequest.html request =
              https://docs.cloud.google.com/java/docs/reference/google-cloud-bigqueryconnection/latest/com.google.cloud.bigquery.connection.v1.UpdateConnectionRequest.html.newBuilder()
                  .setName(name.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigqueryconnection/latest/com.google.cloud.bigquery.connection.v1.ConnectionName.html#com_google_cloud_bigquery_connection_v1_ConnectionName_toString__())
                  .setConnection(connection)
                  .https://docs.cloud.google.com/java/docs/reference/google-cloud-bigqueryconnection/latest/com.google.cloud.bigquery.connection.v1.UpdateConnectionRequest.Builder.html#com_google_cloud_bigquery_connection_v1_UpdateConnectionRequest_Builder_setUpdateMask_com_google_protobuf_FieldMask_(updateMask)
                  .build();
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigqueryconnection/latest/com.google.cloud.bigquery.connection.v1.Connection.html response = client.updateConnection(request);
          System.out.println("Connection updated successfully :" + response.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigqueryconnection/latest/com.google.cloud.bigquery.connection.v1.Connection.html#com_google_cloud_bigquery_connection_v1_Connection_getDescription__());
        }
      }
    }

## Delete a connection

Select one of the following options:

### Console

1. Go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)

   Connections are listed in your project,
   in a group called **Connections**.
2. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)
3. In the **Explorer** pane, click your project name, and then click
   **Connections** to see a list of all connections.

4. Click the connection to see the details.

5. In the details pane, click
   **Delete** to delete the connection.

6. In the **Delete connection?** dialog, enter `delete` to confirm
   deletion.

7. Click **Delete**.

### bq

Enter the `bq rm` command and supply the connection flag:
`--connection`. The fully qualified `connection_id` is required.

```
bq rm --connection PROJECT_ID.REGION.CONNECTION_ID
```

Replace the following:

- `PROJECT_ID`: your Google Cloud project ID
- `REGION`: the [connection region](https://docs.cloud.google.com/bigquery/docs/locations#supported_locations)
- `CONNECTION_ID`: the connection ID

### API

See the
[`projects.locations.connections.delete` method](https://docs.cloud.google.com/bigquery/docs/reference/bigqueryconnection/rest/v1/projects.locations.connections#methods)
in the REST API reference section.

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


    def delete_connection(project_id: str, location: str, connection_id: str):
        """Deletes a BigQuery connection.

        Args:
            project_id: The Google Cloud project ID.
            location: Location of the connection (for example, "us-central1").
            connection_id: ID of the connection to delete.
        """
        name = client.https://docs.cloud.google.com/python/docs/reference/bigqueryconnection/latest/google.cloud.bigquery_connection_v1.services.connection_service.ConnectionServiceClient.html#google_cloud_bigquery_connection_v1_services_connection_service_ConnectionServiceClient_connection_path(project_id, location, connection_id)

        try:
            client.https://docs.cloud.google.com/python/docs/reference/bigqueryconnection/latest/google.cloud.bigquery_connection_v1.services.connection_service.ConnectionServiceClient.html#google_cloud_bigquery_connection_v1_services_connection_service_ConnectionServiceClient_delete_connection(name=name)
            print(f"Connection '{connection_id}' was deleted.")
        except google.api_core.exceptions.NotFound:
            print(f"Connection '{connection_id}' not found.")

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
     * Deletes a connection and its associated credentials.
     *
     * @param {string} projectId Google Cloud project ID (for example, 'example-project-id').
     * @param {string} location The location where the connection resides (for example, 'us-central1').
     * @param {string} connectionId The ID of the connection to delete (for example, 'example-connection').
     */
    async function deleteConnection(projectId, location, connectionId) {
      const request = {
        name: client.connectionPath(projectId, location, connectionId),
      };

      try {
        await client.deleteConnection(request);
        console.log(`Connection ${connectionId} deleted successfully.`);
      } catch (error) {
        if (error.code === status.NOT_FOUND) {
          console.log(
            `Connection ${connectionId} does not exist in location ${location} of project ${projectId}.`,
          );
        } else {
          console.error(`Error deleting connection ${connectionId}:`, error);
        }
      }
    }

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

    import com.google.cloud.bigquery.connection.v1.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigqueryconnection/latest/com.google.cloud.bigquery.connection.v1.ConnectionName.html;
    import com.google.cloud.bigquery.connection.v1.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigqueryconnection/latest/com.google.cloud.bigquery.connection.v1.DeleteConnectionRequest.html;
    import com.google.cloud.bigqueryconnection.v1.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigqueryconnection/latest/com.google.cloud.bigqueryconnection.v1.ConnectionServiceClient.html;
    import java.io.IOException;

    // Sample to delete a connection
    public class DeleteConnection {

      public static void main(String[] args) throws IOException {
        // TODO(developer): Replace these variables before running the sample.
        String projectId = "MY_PROJECT_ID";
        String location = "MY_LOCATION";
        String connectionName = "MY_CONNECTION_NAME";
        deleteConnection(projectId, location, connectionName);
      }

      static void deleteConnection(String projectId, String location, String connectionName)
          throws IOException {
        try (https://docs.cloud.google.com/java/docs/reference/google-cloud-bigqueryconnection/latest/com.google.cloud.bigqueryconnection.v1.ConnectionServiceClient.html client = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigqueryconnection/latest/com.google.cloud.bigqueryconnection.v1.ConnectionServiceClient.html.create()) {
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigqueryconnection/latest/com.google.cloud.bigquery.connection.v1.ConnectionName.html name = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigqueryconnection/latest/com.google.cloud.bigquery.connection.v1.ConnectionName.html.of(projectId, location, connectionName);
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigqueryconnection/latest/com.google.cloud.bigquery.connection.v1.DeleteConnectionRequest.html request =
              https://docs.cloud.google.com/java/docs/reference/google-cloud-bigqueryconnection/latest/com.google.cloud.bigquery.connection.v1.DeleteConnectionRequest.html.newBuilder().setName(name.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigqueryconnection/latest/com.google.cloud.bigquery.connection.v1.ConnectionName.html#com_google_cloud_bigquery_connection_v1_ConnectionName_toString__()).build();
          client.deleteConnection(request);
          System.out.println("Connection deleted successfully");
        }
      }
    }

## What's next

- Learn how to [use remote functions](https://docs.cloud.google.com/bigquery/docs/remote-functions).
- Learn how to [use stored procedures for Spark](https://docs.cloud.google.com/bigquery/docs/spark-procedures).