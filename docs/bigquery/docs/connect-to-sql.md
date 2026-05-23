# Connect to Cloud SQL

As a BigQuery administrator, you can create a
[connection](https://docs.cloud.google.com/bigquery/docs/connections-api-intro) to access
Cloud SQL data. This connection enables data analysts to [query
data in Cloud SQL](https://docs.cloud.google.com/bigquery/docs/cloud-sql-federated-queries).
To connect to Cloud SQL, you must follow these steps:

1. [Create a Cloud SQL connection](https://docs.cloud.google.com/bigquery/docs/connect-to-sql#create-sql-connection)
2. [Grant access to the **BigQuery Connection Service Agent**](https://docs.cloud.google.com/bigquery/docs/connect-to-sql#access-sql).

## Before you begin

1. Select the project that contains the Cloud SQL database.  


   [Go to project selector](https://console.cloud.google.com/projectselector2/home/dashboard)
2. Enable the BigQuery Connection API.  


   [Enable the API](https://console.cloud.google.com/apis/library/bigqueryconnection.googleapis.com)
3. Ensure that the Cloud SQL instance has a [public IP connection](https://docs.cloud.google.com/sql/docs/mysql/configure-ip) or a [private connection](https://docs.cloud.google.com/sql/docs/mysql/configure-private-ip):  
   - To secure your Cloud SQL instances, you can add public IP connectivity
     without an authorized address. This makes the instance inaccessible from
     the public internet but accessible to queries from BigQuery.

   - To let BigQuery access Cloud SQL data over a
     private connection, configure private IP connectivity for a
     [new](https://docs.cloud.google.com/sql/docs/mysql/configure-private-ip#new-private-instance)
     or an [existing](https://docs.cloud.google.com/sql/docs/mysql/configure-private-ip#existing-private-instance)
     Cloud SQL instance, and then select the **Enable private path**
     checkbox. This service uses an internal direct path instead of the private IP
     address inside of the Virtual Private Cloud.

4.

   To get the permissions that
   you need to create a Cloud SQL connection,

   ask your administrator to grant you the
   [BigQuery Connection Admin](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquery#bigquery.connectionAdmin) (`roles/bigquery.connectionAdmin`) IAM role on the project.


   For more information about granting roles, see [Manage access to projects, folders, and organizations](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).


   You might also be able to get
   the required permissions through [custom
   roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles) or other [predefined
   roles](https://docs.cloud.google.com/iam/docs/roles-overview#predefined).

## Create Cloud SQL connections

As a best practice, use connections to handle database credentials when you are
connecting to Cloud SQL. Connections are encrypted and stored securely in the
BigQuery connection service. If the user credentials are valid
for other data in the source, you can re-use the connection. For example, you
might be able to use one connection to query multiple databases residing in the
same Cloud SQL instance.

Select one of the following options to create a Cloud SQL connection:

### Console

1. Go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the **Explorer** pane, click **Add**.

   The **Add data** dialog opens.
3. In the **Filter By** pane, in the **Data Source Type** section, select **Databases**.

   Alternatively, in the **Search for data sources** field, you can enter
   `mysql`.
4. In the **Featured data sources** section, click **MySQL**.

5. Click the **CloudSQL (MySQL): BigQuery Federation** solution card.

6. In the **External data source** dialog, enter the following information:

   - For **Connection type** , select the type of source, for example **MySQL** or **PostgreSQL**.
   - For **Connection ID** , enter an identifier for the connection resource. Letters, numbers, and underscores are allowed. For example, `bq_sql_connection`.
   - For **Data location** , select a BigQuery location (or region) that is [compatible with your external data source region](https://docs.cloud.google.com/bigquery/docs/federated-queries-intro#supported_regions).
   - Optional: For **Friendly name** , enter a user-friendly name for the connection, such as `My connection resource`. The friendly name can be any value that helps you identify the connection resource if you need to modify it later.
   - Optional: For **Description**, enter a description for this connection resource.
   - Optional: **Encryption** If you want to use a [customer-managed encryption key (CMEK)](https://docs.cloud.google.com/bigquery/docs/customer-managed-encryption) to encrypt your credentials, select **Customer-managed encryption key (CMEK)** and then select a customer-managed key. Otherwise, your credentials are protected by the default Google-owned and Google-managed encryption key.
   - If you chose Cloud SQL MySQL or Postgres for the connection type, for **Cloud SQL connection name** , enter the full [name of the Cloud SQL instance](https://docs.cloud.google.com/sql/docs/mysql/instance-settings#instance-id-2ndgen), usually in the format `project-id:location-id:instance-id`. You can find the instance ID on the detail page of the [Cloud SQL instance](https://console.cloud.google.com/sql/instances) you want to query.
   - For **Database name**, enter the name of the database.
   - For **Database username**, enter the username for the database.
   - For **Database password**, enter the password for the database.

     - Optional: To see the password, click **Show password**.

     > [!NOTE]
     > **Note:** If the same user credentials are valid for other databases in the external data source, that user can query those databases through the same connection resource.

7. Click **Create connection**.

8. Click **Go to connection**.

9. In the **Connection info** pane, copy the service account ID for use in a
   following step.

### bq

Enter the `bq mk` command and supply the connection flag:
`--connection`. The following flags are also required:

- `--connection_type`
- `--properties`
- `--connection_credential`
- `--project_id`
- `--location`

The following flags are optional:

- `--display_name`: The friendly name for the connection.
- `--description`: A description of the connection.
- `--kms_key_name`: A customer-managed encryption key. If omitted, credentials are protected by the default Google-owned and Google-managed encryption key.

The `connection_id` is an optional parameter that can be added as the last argument of the command which
is used for storage internally. If a connection ID is not provided a unique ID is automatically generated.
The `connection_id` can contain letters, numbers, and underscores.

        bq mk --connection --display_name='friendly name' --connection_type=TYPE \
          --properties=PROPERTIES --connection_credential=CREDENTIALS \
          --project_id=PROJECT_ID --location=LOCATION \
          CONNECTION_ID

Replace the following:

- `TYPE`: the type of the external data source.
- `PROPERTIES`: the parameters for the created connection in JSON format. For example: `--properties='{"param":"param_value"}'`. For creating a connection resource, you must supply the `instanceID`, `database`, and `type` parameters.
- `CREDENTIALS`: the parameters `username` and `password`.
- `PROJECT_ID`: your project ID.
- `LOCATION`: the region your Cloud SQL instance is located in, or the corresponding multi-region.
- `CONNECTION_ID`: the connection identifier.

For example, the following command creates a new connection resource
named my_new_connection (friendly name: "My new connection") in a project
with the ID `federation-test`.

    bq mk --connection --display_name='friendly name' --connection_type='CLOUD_SQL' \
      --properties='{"instanceId":"federation-test:us-central1:mytestsql","database":"mydatabase","type":"MYSQL"}' \
      --connection_credential='{"username":"myusername", "password":"mypassword"}' \
      --project_id=federation-test --location=us my_connection_id

### API

Within the BigQuery Connection API, you can invoke `CreateConnection` within
the `ConnectionService` to instantiate a connection. See the [client library page](https://docs.cloud.google.com/bigquery/docs/reference/bigqueryconnection) for more details.

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

    import com.google.cloud.bigquery.connection.v1.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigqueryconnection/latest/com.google.cloud.bigquery.connection.v1.CloudSqlCredential.html;
    import com.google.cloud.bigquery.connection.v1.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigqueryconnection/latest/com.google.cloud.bigquery.connection.v1.CloudSqlProperties.html;
    import com.google.cloud.bigquery.connection.v1.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigqueryconnection/latest/com.google.cloud.bigquery.connection.v1.Connection.html;
    import com.google.cloud.bigquery.connection.v1.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigqueryconnection/latest/com.google.cloud.bigquery.connection.v1.CreateConnectionRequest.html;
    import com.google.cloud.bigquery.connection.v1.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigqueryconnection/latest/com.google.cloud.bigquery.connection.v1.LocationName.html;
    import com.google.cloud.bigqueryconnection.v1.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigqueryconnection/latest/com.google.cloud.bigqueryconnection.v1.ConnectionServiceClient.html;
    import java.io.IOException;

    // Sample to create a connection with cloud MySql database
    public class CreateConnection {

      public static void main(String[] args) throws IOException {
        // TODO(developer): Replace these variables before running the sample.
        String projectId = "MY_PROJECT_ID";
        String location = "MY_LOCATION";
        String connectionId = "MY_CONNECTION_ID";
        String database = "MY_DATABASE";
        String instance = "MY_INSTANCE";
        String instanceLocation = "MY_INSTANCE_LOCATION";
        String username = "MY_USERNAME";
        String password = "MY_PASSWORD";
        String instanceId = String.format("%s:%s:%s", projectId, instanceLocation, instance);
        https://docs.cloud.google.com/java/docs/reference/google-cloud-bigqueryconnection/latest/com.google.cloud.bigquery.connection.v1.CloudSqlCredential.html cloudSqlCredential =
            https://docs.cloud.google.com/java/docs/reference/google-cloud-bigqueryconnection/latest/com.google.cloud.bigquery.connection.v1.CloudSqlCredential.html.newBuilder().https://docs.cloud.google.com/java/docs/reference/google-cloud-bigqueryconnection/latest/com.google.cloud.bigquery.connection.v1.CloudSqlCredential.Builder.html#com_google_cloud_bigquery_connection_v1_CloudSqlCredential_Builder_setUsername_java_lang_String_(username).https://docs.cloud.google.com/java/docs/reference/google-cloud-bigqueryconnection/latest/com.google.cloud.bigquery.connection.v1.CloudSqlCredential.Builder.html#com_google_cloud_bigquery_connection_v1_CloudSqlCredential_Builder_setPassword_java_lang_String_(password).build();
        https://docs.cloud.google.com/java/docs/reference/google-cloud-bigqueryconnection/latest/com.google.cloud.bigquery.connection.v1.CloudSqlProperties.html cloudSqlProperties =
            https://docs.cloud.google.com/java/docs/reference/google-cloud-bigqueryconnection/latest/com.google.cloud.bigquery.connection.v1.CloudSqlProperties.html.newBuilder()
                .https://docs.cloud.google.com/java/docs/reference/google-cloud-bigqueryconnection/latest/com.google.cloud.bigquery.connection.v1.CloudSqlProperties.Builder.html#com_google_cloud_bigquery_connection_v1_CloudSqlProperties_Builder_setType_com_google_cloud_bigquery_connection_v1_CloudSqlProperties_DatabaseType_(https://docs.cloud.google.com/java/docs/reference/google-cloud-bigqueryconnection/latest/com.google.cloud.bigquery.connection.v1.CloudSqlProperties.html.DatabaseType.MYSQL)
                .setDatabase(database)
                .https://docs.cloud.google.com/java/docs/reference/google-cloud-bigqueryconnection/latest/com.google.cloud.bigquery.connection.v1.CloudSqlProperties.Builder.html#com_google_cloud_bigquery_connection_v1_CloudSqlProperties_Builder_setInstanceId_java_lang_String_(instanceId)
                .https://docs.cloud.google.com/java/docs/reference/google-cloud-bigqueryconnection/latest/com.google.cloud.bigquery.connection.v1.CloudSqlProperties.Builder.html#com_google_cloud_bigquery_connection_v1_CloudSqlProperties_Builder_setCredential_com_google_cloud_bigquery_connection_v1_CloudSqlCredential_(cloudSqlCredential)
                .build();
        https://docs.cloud.google.com/java/docs/reference/google-cloud-bigqueryconnection/latest/com.google.cloud.bigquery.connection.v1.Connection.html connection = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigqueryconnection/latest/com.google.cloud.bigquery.connection.v1.Connection.html.newBuilder().https://docs.cloud.google.com/java/docs/reference/google-cloud-bigqueryconnection/latest/com.google.cloud.bigquery.connection.v1.Connection.Builder.html#com_google_cloud_bigquery_connection_v1_Connection_Builder_setCloudSql_com_google_cloud_bigquery_connection_v1_CloudSqlProperties_(cloudSqlProperties).build();
        createConnection(projectId, location, connectionId, connection);
      }

      static void createConnection(
          String projectId, String location, String connectionId, https://docs.cloud.google.com/java/docs/reference/google-cloud-bigqueryconnection/latest/com.google.cloud.bigquery.connection.v1.Connection.html connection)
          throws IOException {
        try (https://docs.cloud.google.com/java/docs/reference/google-cloud-bigqueryconnection/latest/com.google.cloud.bigqueryconnection.v1.ConnectionServiceClient.html client = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigqueryconnection/latest/com.google.cloud.bigqueryconnection.v1.ConnectionServiceClient.html.create()) {
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigqueryconnection/latest/com.google.cloud.bigquery.connection.v1.LocationName.html parent = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigqueryconnection/latest/com.google.cloud.bigquery.connection.v1.LocationName.html.of(projectId, location);
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigqueryconnection/latest/com.google.cloud.bigquery.connection.v1.CreateConnectionRequest.html request =
              https://docs.cloud.google.com/java/docs/reference/google-cloud-bigqueryconnection/latest/com.google.cloud.bigquery.connection.v1.CreateConnectionRequest.html.newBuilder()
                  .setParent(parent.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigqueryconnection/latest/com.google.cloud.bigquery.connection.v1.LocationName.html#com_google_cloud_bigquery_connection_v1_LocationName_toString__())
                  .setConnection(connection)
                  .https://docs.cloud.google.com/java/docs/reference/google-cloud-bigqueryconnection/latest/com.google.cloud.bigquery.connection.v1.CreateConnectionRequest.Builder.html#com_google_cloud_bigquery_connection_v1_CreateConnectionRequest_Builder_setConnectionId_java_lang_String_(connectionId)
                  .build();
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigqueryconnection/latest/com.google.cloud.bigquery.connection.v1.Connection.html response = client.createConnection(request);
          System.out.println("Connection created successfully :" + response.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigqueryconnection/latest/com.google.cloud.bigquery.connection.v1.Connection.html#com_google_cloud_bigquery_connection_v1_Connection_getName__());
        }
      }
    }

## Grant access to the service agent

A [service agent](https://docs.cloud.google.com/iam/docs/service-agents) is automatically created when you
create the first connection to Cloud SQL within the project.
The service agent's name is **BigQuery Connection Service
Agent** . To get the service agent ID, [view your connection details](https://docs.cloud.google.com/bigquery/docs/working-with-connections#view-connections). The service agent ID is of the
following format:

`service-PROJECT_NUMBER@gcp-sa-bigqueryconnection.iam.gserviceaccount.com`.

To connect to Cloud SQL, you must give the new connection read-only
access to Cloud SQL so that BigQuery can access files on
behalf of users. The service agent must have the following permissions:

- `cloudsql.instances.connect`
- `cloudsql.instances.get`

You can grant the service agent associated with the connection the
[Cloud SQL Client IAM role](https://docs.cloud.google.com/sql/docs/mysql/iam-roles#roles)
(`roles/cloudsql.client`), which has these permissions assigned.
You can skip the following steps if the service agent already has the required
permissions.

### Console

1. Go to the **IAM \& Admin** page.

   [Go to IAM \& Admin](https://console.cloud.google.com/project/_/iam-admin)
2. Click **Grant Access**.

   The **Add principals** dialog opens.
3. In the **New principals** field, enter the service agent name
   **BigQuery Connection Service Agent** or the service agent ID taken from
   the [connection information](https://docs.cloud.google.com/bigquery/docs/working-with-connections#view-connections).

4. In the **Select a role** field, select **Cloud SQL** , and then select
   **Cloud SQL Client**.

5. Click **Save**.

### gcloud

Use the
[`gcloud projects add-iam-policy-binding`](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access#grant-single-role)
command:

```
gcloud projects add-iam-policy-binding PROJECT_ID --member=serviceAccount:SERVICE_AGENT_ID --role=roles/cloudsql.client
```

Provide the following values:

- `PROJECT_ID`: Your Google Cloud project ID.
- `SERVICE_AGENT_ID`: The service agent ID taken from the [connection information](https://docs.cloud.google.com/bigquery/docs/working-with-connections#view-connections).

> [!NOTE]
> **Note:** For more information on how to grant and revoke IAM roles, see [Manage access to projects, folders, and
> organizations](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access#view-access).

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
- Learn how to [query Cloud SQL data](https://docs.cloud.google.com/bigquery/docs/cloud-sql-federated-queries).