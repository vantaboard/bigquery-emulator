# Connect to Apache Spark

As a BigQuery administrator, you can create a
[connection](https://docs.cloud.google.com/bigquery/docs/connections-api-intro) to enable data analysts to
[run stored procedures for Apache Spark](https://docs.cloud.google.com/bigquery/docs/spark-procedures).

## Before you begin

-
  Enable the BigQuery Connection API.


  [Enable the API](https://console.cloud.google.com/apis/library/bigqueryconnection.googleapis.com)
-

  To get the permissions that
  you need to create a Spark connection,

  ask your administrator to grant you the
  [BigQuery Connection Admin](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquery#bigquery.connectionAdmin) (`roles/bigquery.connectionAdmin`) IAM role on the project.


  For more information about granting roles, see [Manage access to projects, folders, and organizations](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).


  You might also be able to get
  the required permissions through [custom
  roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles) or other [predefined
  roles](https://docs.cloud.google.com/iam/docs/roles-overview#predefined).
- Optional: To manage your metadata using [Dataproc Metastore](https://docs.cloud.google.com/dataproc-metastore/docs/overview), ensure that you have [created a Dataproc Metastore service](https://docs.cloud.google.com/dataproc-metastore/docs/create-service).
- Optional: To [view job history using Spark History Server web
  interfaces](https://docs.cloud.google.com/dataproc/docs/concepts/jobs/history-server#spark_history_server_web_interface), ensure that you have [created a Managed Service for Apache Spark Persistent History
  Server (PHS)](https://docs.cloud.google.com/dataproc/docs/concepts/jobs/history-server#create_a_phs_cluster).

### Location considerations

When you choose a location for your data, consider the following:

#### Multi-regions

You must specify Google Cloud resources located in the same large
geographic area:

- A connection in the BigQuery US multi-region can
  reference a [Spark History Server](https://docs.cloud.google.com/dataproc/docs/concepts/jobs/history-server)
  or a [Dataproc Metastore](https://docs.cloud.google.com/dataproc-metastore/docs/overview)
  in any single region in the US geographic area, such as `us-central1`,
  `us-east4`, or `us-west2`.

- A connection in the BigQuery EU multi-region can
  reference a Spark History Server or a Dataproc Metastore in [member
  states](https://europa.eu/european-union/about-eu/countries_en) of the
  European Union, such as `europe-north1` or `europe-west3`.

#### Single regions

A connection in a single region can only reference Google Cloud
resources in the same region. For example, a connection in the single
region `us-east4` can only reference a Spark History Server or a
Dataproc Metastore in `us-east4`.

## Create connections

Select one of the following options:

### Console

1. Go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the **Explorer** pane, click **Add data**.

   The **Add data** dialog opens.
3. In the **Filter By** pane, in the **Data Source Type** section, select **Business Applications**.

   Alternatively, in the **Search for data sources** field, you can enter
   `Spark`.
4. In the **Featured data sources** section, click **Apache Spark**.

5. Click the **Apache Spark: BigQuery Federation** solution card.

6. In the **External data source** pane, enter the following information:

   - In the **Connection type** list, select **Apache Spark**.

   - In the **Connection ID** field, enter a name for your connection---for
     example, `spark_connection`.

   - In the **Data location** list, select a region.

   You can create a connection in [regions and multi-regions that support
   BigQuery](https://docs.cloud.google.com/bigquery/docs/locations).
   For more information, see [Location considerations](https://docs.cloud.google.com/bigquery/docs/connect-to-spark#location-considerations).
   - Optional: From the **Metastore service** list, select a
     [Dataproc Metastore](https://docs.cloud.google.com/dataproc-metastore/docs/overview).

   - Optional: In the **History server cluster** field, enter a
     [Managed Service for Apache Spark Persistent History Server](https://docs.cloud.google.com/dataproc/docs/concepts/jobs/history-server#create_a_phs_cluster).

7. Click **Create connection**.

8. Click **Go to connection**.

9. In the **Connection info** pane, copy the service account ID for use in a
   following step.

### bq

1. In a command-line environment, use the [`bq mk`
   command](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#bq_mk) to create a
   connection:

   ```
   bq mk --connection --connection_type='SPARK' \
    --properties=PROPERTIES \
    --project_id=PROJECT_ID \
    --location=LOCATION
    CONNECTION_ID
   ```

   Replace the following:
   - `PROPERTIES`: a key-value pair to provide
     connection-specific parameters in JSON format

     For example:

     ```
     --properties='{
     "metastoreServiceConfig": {"metastoreService": "METASTORE_SERVICE_NAME"},
     "sparkHistoryServerConfig": {"dataprocCluster": "MANAGED_SERVICE_FOR_APACHE_SPARK_CLUSTER_NAME"}
     }'
     ```

     Replace the following:
     - `METASTORE_SERVICE_NAME`: the
       [Dataproc Metastore with a gRPC
       network configuration](https://docs.cloud.google.com/dataproc-metastore/docs/endpoint-protocol#grpc_network_configuration)---for
       example, `projects/my-project-id/locations/us-central1/services/my-service`

       For more information, see how to access the [stored Hive metastore
       metadata using an endpoint protocol](https://docs.cloud.google.com/dataproc-metastore/docs/endpoint-protocol).
     - `MANAGED_SERVICE_FOR_APACHE_SPARK_CLUSTER_NAME`: the Spark History
       Server configuration---for example,
       `projects/my-project-id/regions/us-central1/clusters/my-cluster`

       For more information, see [Create a Persistent History Server
       cluster](https://docs.cloud.google.com/dataproc/docs/concepts/jobs/history-server#create_a_phs_cluster).
   - `PROJECT_ID`: your Google Cloud project ID

   - `LOCATION`: the location where you want to store
     the connection---for example, `US`

   - `CONNECTION_ID`: the connection
     ID---for example, `myconnection`

     When you [view the connection details](https://docs.cloud.google.com/bigquery/docs/working-with-connections#view-connections)
     in the Google Cloud console, the connection ID is the value in the last
     section of the fully qualified connection ID that is shown in
     **Connection ID** ---for example `projects/.../locations/.../connections/*myconnection*`
2. Retrieve and copy the service account ID because you need it in another step:

   ```
   bq show --location=LOCATION --connection PROJECT_ID.LOCATION.CONNECTION_ID
   ```

   The output is similar to the following:

   ```
   Connection myproject.us.myconnection

          name           type                    properties
   --- --- ---
   myproject.us.myconnection  SPARK   {"serviceAccountId": "bqserver@example.iam.gserviceaccount.com"}
   ```

For information about how to manage connections, see [Manage connections](https://docs.cloud.google.com/bigquery/docs/working-with-connections).

## Grant access to the service account

To let a stored procedure for Apache Spark access your Google Cloud
resources, you need to grant the service account that's
associated with the stored procedure's connection the necessary IAM
permissions. Alternatively, you can use your
[custom service account](https://docs.cloud.google.com/bigquery/docs/spark-procedures#use_a_custom_service_account)
for data access.

- To read and write data from and to BigQuery, you need to give
  the service account the following IAM permissions:

  - `bigquery.tables.*` on your BigQuery tables
  - `bigquery.readsessions.*` on your project

  The `roles/bigquery.admin` IAM role includes the permissions
  that the service account needs in order to read and write data from and to
  BigQuery.

  > [!NOTE]
  > **Note:** If your stored procedure writes data to a temporary Cloud Storage bucket and then [loads the
  > Cloud Storage data to BigQuery](https://docs.cloud.google.com/bigquery/docs/batch-loading-data), then you need to give the service account the `bigquery.jobs.create` permission on your project. For more information about IAM roles and permissions in BigQuery, see [Access control with
  > IAM](https://docs.cloud.google.com/bigquery/access-control).

- To read and write data from and to Cloud Storage, you need to give
  the service account the `storage.objects.*` permission on your
  Cloud Storage objects.

  The `roles/storage.objectAdmin` IAM role includes the permissions
  that the service account needs in order to read and write data from and to Cloud Storage.
- If you specify Dataproc Metastore when you create a
  connection, then for BigQuery to retrieve details about the
  metastore configuration, you need to give the service account the
  `metastore.services.get` permission on your Dataproc Metastore.

  The predefined `roles/metastore.metadataViewer` role includes the permission
  that the service account needs in order to retrieve details about the
  metastore configuration.

  You also need to grant the service account the `roles/storage.objectAdmin`
  role on the Cloud Storage bucket so that your stored
  procedure can access the Hive warehouse directory of your
  Dataproc Metastore (`hive.metastore.warehouse.dir`).
  If your stored procedure performs operations on the metastore, you
  might need to give additional permissions. For more information about
  IAM roles and permissions in Dataproc Metastore,
  see [Dataproc Metastore predefined roles and permissions](https://docs.cloud.google.com/dataproc-metastore/docs/iam-roles).
- If you specify a Managed Service for Apache Spark Persistent History Server when you
  create a connection, then you need to grant the service account the
  following roles:

  - The `roles/dataproc.viewer` role on your Managed Service for Apache Spark Persistent History Server that contains the `dataproc.clusters.get` permission.
  - The `roles/storage.objectAdmin` role on the Cloud Storage bucket that you specify for the property `spark:spark.history.fs.logDirectory` when you create the Managed Service for Apache Spark Persistent History Server.

  For more information, see [Managed Service for Apache Spark Persistent History
  Server](https://docs.cloud.google.com/dataproc/docs/concepts/jobs/history-server#create_a_phs_cluster) and
  [Managed Service for Apache Spark roles and permissions](https://docs.cloud.google.com/dataproc/docs/concepts/iam/iam).

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
- Learn how to [create a stored procedure for Apache Spark](https://docs.cloud.google.com/bigquery/docs/spark-procedures).
- Learn how to [manage stored procedures](https://docs.cloud.google.com/bigquery/docs/routines).