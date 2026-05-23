# Connect to Spanner

As a BigQuery administrator, you can create a
[connection](https://docs.cloud.google.com/bigquery/docs/connections-api-intro) to access
Spanner data. This connection enables data analysts to [query
data in Spanner](https://docs.cloud.google.com/bigquery/docs/spanner-federated-queries).

## Before you begin

- Enable the BigQuery Connection API.  

  [Enable the API](https://console.cloud.google.com/apis/library/bigqueryconnection.googleapis.com)
-

  To get the permissions that
  you need to connect to Spanner,

  ask your administrator to grant you the
  [BigQuery Connection Admin](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquery#bigquery.connectionAdmin) (`roles/bigquery.connectionAdmin`) IAM role on the project.


  For more information about granting roles, see [Manage access to projects, folders, and organizations](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).


  You might also be able to get
  the required permissions through [custom
  roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles) or other [predefined
  roles](https://docs.cloud.google.com/iam/docs/roles-overview#predefined).

## Create Spanner connections

Select one of the following options:

### Console

1. Go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the **Explorer** pane, click **Add data**.

   The **Add data** dialog opens.
3. In the **Filter By** pane, in the **Data Source Type** section, select **Databases**.

   Alternatively, in the **Search for data sources** field, you can enter
   `Spanner`.
4. In the **Featured data sources** section, click **Google Cloud Spanner**.

5. Click the **Google Cloud Spanner: BigQuery Federation** solution card.

6. In the **External data source** pane, enter the following information:

   - For **Connection type** , select **Cloud Spanner**.
   - For **Connection ID**, enter an identifier for the connection resource. Letters, numbers, and underscores are allowed.
   - For **Location type** , select a BigQuery location (or region) that is [compatible with your external data source region](https://docs.cloud.google.com/bigquery/docs/federated-queries-intro#supported_regions).
   - Optional: For **Friendly name** , enter a user-friendly name for the connection, such as `My connection resource`. The friendly name can be any value that helps you identify the connection resource if you need to modify it later.
   - Optional: For **Description**, enter a description for this connection resource.
   - For **Database name** , enter the name of the Spanner database in the following format: `"projects/PROJECT_ID/instances/INSTANCE/databases/DATABASE"`
   - Optional: To perform parallel reads, select **Read data in
     parallel** . Spanner can divide certain queries into smaller pieces, or partitions, and fetch the partitions in parallel. For more information, see [Read data in parallel](https://docs.cloud.google.com/spanner/docs/reads#read_data_in_parallel) in the Spanner documentation. This option is restricted to queries whose first operator in the execution plan is a [distributed union](https://docs.cloud.google.com/spanner/docs/query-execution-operators#distributed-union) operator. Other queries return an error. To view the query execution plan for a Spanner query, see [Understand how
     Spanner executes queries](https://docs.cloud.google.com/spanner/docs/sql-best-practices#how-execute-queries).
   - Optional: For **Database role** , enter the name of a Spanner database role. If not empty, this connection queries Spanner using this database role by default. Spanner fine-grained access control users who submit queries through this connection must have been granted access to this role by their administrator, and the database role must have the `SELECT` privilege on all schema objects specified in external queries. For information about fine-grained access control, see [About fine-grained access control](https://docs.cloud.google.com/spanner/docs/fgac-about).
   - Optional: To enable Data Boost, select **Use Spanner Data Boost** . [Data Boost](https://docs.cloud.google.com/bigquery/docs/spanner-federated-queries#data_boost) lets you execute analytics queries and data exports with near-zero impact to existing workloads on the provisioned BigQuery instance. To enable Data Boost, select **Data Boost** and **Read data in parallel.**
7. Click **Create connection**.

### bq

To create the connection, use the
[`bq mk`](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#mk-connection) command
with the `--connection` flag.

```bash
bq mk --connection \
    --connection_type=CLOUD_SPANNER \
    --properties='PROPERTIES' \
    --location=LOCATION \
    --display_name='FRIENDLY_NAME' \
    --description 'DESCRIPTION' \
    CONNECTION_ID
```

Replace the following:

- `PROPERTIES`: a JSON object with the following
  fields:

  - `"database"`: the Spanner database for the connection

    Specify as a string with the following format:
    `"projects/PROJECT_ID/instances/INSTANCE/databases/DATABASE"`.
  - `"use_parallelism"`: (Optional) if `true`, this connection performs
    parallel reads

    The default value is `false`. Spanner can divide certain
    queries into smaller pieces, or partitions, and fetch the partitions in
    parallel. For more information, see [Read data in parallel](https://docs.cloud.google.com/spanner/docs/reads#read_data_in_parallel)
    in the Spanner documentation. This option is restricted
    to queries whose first operator in the execution plan is a [distributed
    union](https://docs.cloud.google.com/spanner/docs/query-execution-operators#distributed-union)
    operator. Other queries return an error. To view the query execution
    plan for a Spanner query, see [Understand how
    Spanner executes queries](https://docs.cloud.google.com/spanner/docs/sql-best-practices#how-execute-queries).
  - `"database_role"`: (Optional) If not empty, this connection queries
    Spanner using this database role by default.
    Spanner fine-grained access control users who submit
    queries through this connection
    must have been granted access to this role by their administrator,
    and the database role must have the `SELECT` privilege on all schema
    objects specified in external queries.

    If not specified, the connection authenticates with
    IAM predefined roles for Spanner, and
    the principal running queries with this connection must have
    been granted the `roles/spanner.databaseReader` IAM
    role.

    For information about
    fine-grained access control, see [About fine-grained access control](https://docs.cloud.google.com/spanner/docs/fgac-about).
  - `"useDataBoost"`: (Optional) If `true`, this connection lets users use [Data Boost](https://docs.cloud.google.com/bigquery/docs/spanner-federated-queries#data_boost). Data Boost lets users run federated queries in separate, independent, compute capacity distinct from provisioned instances to avoid impacting existing workloads. To enable Data Boost, set `"useDataBoost"` to `true` and `"use_parallelism"` to `true`.

    In order to use Data Boost, the principal running queries with this connection must have been granted the `spanner.databases.useDataBoost` permission. This permission is included by default in the `roles/spanner.admin` and `roles/spanner.databaseAdmin` roles.
- `LOCATION`: a BigQuery location that
  is [compatible with your external data source region](https://docs.cloud.google.com/bigquery/docs/federated-queries-intro#supported_regions).

- `CONNECTION_ID`: an identifier for the connection
  resource

  The connection ID can contain letters, numbers and underscores.
  If you don't provide a connection ID, BigQuery
  automatically generates a unique ID.

  The following example creates a new connection resource named
  `my_connection_id`.

  ```bash
  bq mk --connection \
    --connection_type='CLOUD_SPANNER' \
    --properties='{"database":"projects/my_project/instances/my_instance/databases/database1"}' \
    --project_id=federation-test \
    --location=us \
    my_connection_id
  ```

### API

Call the [`CreateConnection` method](https://docs.cloud.google.com/bigquery/docs/reference/bigqueryconnection/rpc/google.cloud.bigquery.connection.v1#createconnectionrequest) within
the `ConnectionService` service.

## Share connections with users

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

## What's next

- Learn about different [connection types](https://docs.cloud.google.com/bigquery/docs/connections-api-intro).
- Learn about [managing connections](https://docs.cloud.google.com/bigquery/docs/working-with-connections).
- Learn about [federated queries](https://docs.cloud.google.com/bigquery/docs/federated-queries-intro).
- Learn how to [query Spanner data](https://docs.cloud.google.com/bigquery/docs/spanner-federated-queries).