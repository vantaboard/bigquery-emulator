# Connect to SAP Datasphere

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

> [!NOTE]
> **Note:** To get support or provide feedback for this preview feature, contact [bq-sap-federation-support@google.com](mailto:bq-sap-federation-support@google.com).

As a BigQuery administrator, you can create a
[connection](https://docs.cloud.google.com/bigquery/docs/connections-api-intro) to access
SAP Datasphere data. This connection enables data analysts to [query
data in SAP Datasphere](https://docs.cloud.google.com/bigquery/docs/sap-datasphere-federated-queries).

## Before you begin

1. Enable the BigQuery Connection API.

   [Enable the API](https://console.cloud.google.com/apis/library/bigqueryconnection.googleapis.com)
2. [Create a SAP Datasphere database user](https://help.sap.com/docs/SAP_DATASPHERE/be5967d099974c69b77f4549425ca4c0/798e3fd6707940c3bd2219b2d1ebaac2.html?locale=en-US).
   Note the username, password, hostname, and port for BigQuery
   to connect to.

3. Configure your SAP Datasphere tenant to accept traffic from
   your selected IP addresses by doing one of the following:

   - Add all [Google IP address ranges](https://www.gstatic.com/ipranges/goog.json) to the "Trusted IPs" allowlist in SAP Datasphere.
   - Open the SAP Datasphere tenant to connections from all IP addresses by adding `0.0.0.0/0` to the allowlist.
   - [Configure your connection with network attachments](https://docs.cloud.google.com/bigquery/docs/connections-with-network-attachment)
     so that BigQuery opens the connection from a static IP
     address.

     > [!NOTE]
     > **Note:** If your configured network attachment and VM are located in different regions, there might be cross-region data movement when you use this connection to query SAP Datasphere data.

   For more information about configuring your SAP Datasphere
   tenant, see
   [Add IP address to IP Allowlist](https://help.sap.com/docs/SAP_DATASPHERE/9f804b8efa8043539289f42f372c4862/a3c214514ef94e899459f68f4c1e2a23.html?locale=en-US).

### Required roles


To get the permissions that
you need to connect to SAP Datasphere,

ask your administrator to grant you the
[BigQuery Connection Admin](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquery#bigquery.connectionAdmin) (`roles/bigquery.connectionAdmin`) IAM role on the project.


For more information about granting roles, see [Manage access to projects, folders, and organizations](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).


You might also be able to get
the required permissions through [custom
roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles) or other [predefined
roles](https://docs.cloud.google.com/iam/docs/roles-overview#predefined).

### Connect BigQuery to SAP Datasphere

You can connect BigQuery to SAP Datasphere in the Google Cloud console or the
bq command-line tool.

### Console

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the **Explorer** pane, click **Add data**.

   The **Add data** dialog opens.
3. In the **Filter By** pane, in the **Data Source Type** section, select **Databases**.

   Alternatively, in the **Search for data sources** field, you can enter
   `SAP HANA`.
4. In the **Featured data sources** section, click **SAP HANA**.

5. Click the **SAP HANA: BigQuery Federation** solution card.

6. In the **External data source** dialog, do the following:

   - For **Connection type** , select `SAP HANA`.
   - For **Connection ID**, enter a connection ID to identify this connection.
   - For **Location type**, specify a region of the BigQuery dataset to be combined with the data from SAP Datasphere. Queries that use this connection must be run from this region.
   - Optional: For **Friendly name** , enter a user-friendly name for the connection, such as `My connection resource`. The friendly name can be any value that helps you identify the connection resource if you need to modify it later.
   - Optional: For **Description**, enter a description for this connection resource.
   - For **Encryption** , select either **Google-managed encryption key** or **Customer-managed encryption key (CMEK)**. The use of a CMEK is optional.
   - For **Host:port** : enter the host and port of the SAP database instance, as shown in the **Database User Details** in the SAP Datasphere web console, in the format `HOST:PORT`.
   - Optional: For **Network attachment** , enter a path to the [network attachment](https://docs.cloud.google.com/bigquery/docs/connections-with-network-attachment) that defines the network configuration that is used for establishing a connection to SAP Datasphere.
   - For **Username** : enter the database username from **Database User Details** in the SAP Datasphere web console. For example, `MY_SPACE#BIGQUERY`.
   - For **Password**: enter the database user's password.
7. Click **Create connection**.

### bq

Enter the [`bq mk`](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#bq_mk) command
with the following flags:

      bq mk \
      --connection \
      --location=LOCATION \
      --project_id=PROJECT_ID \
      --connector_configuration '{
        "connector_id": "saphana",
        "endpoint": {
          "host_port": "HOST_PORT"
        },
        "authentication": {
          "username_password": {
            "username": "USERNAME",
            "password": {
              "plaintext": "PASSWORD"
            }
          }
        },
        "network": {
          "private_service_connect": {
            "network_attachment": "NETWORK_ATTACHMENT"
          }
        }
      }' \
      CONNECTION_ID

Replace the following:

- `LOCATION`: specify a region of the BigQuery dataset to be combined with the data from SAP Datasphere. Queries that use this connection must be run from this region.
- `PROJECT_ID`: enter your Google Cloud project ID.
- `HOST_PORT`: enter the host and port of the SAP database instance, as shown in the **Database User Details** in the SAP Datasphere web console, in the format `HOST:PORT`.
- `NETWORK_ATTACHMENT` (optional): enter the [network attachment](https://docs.cloud.google.com/bigquery/docs/connections-with-network-attachment) in the format `projects/{project}/regions/{region}/networkAttachments/{networkattachment}`. With this field, you can configure the SAP Datasphere connection so that BigQuery opens the connection from a static IP address.
- `USERNAME`: enter the database username from **Database
  User Details** in the SAP Datasphere web console. For example, `MY_SPACE#BIGQUERY`.
- `PASSWORD`: enter the database user's password.
- `CONNECTION_ID`: enter a connection ID to identify this connection.

Optional flag:

- `--kms_key_name`: A customer-managed encryption key. If omitted, credentials are protected by the default Google-owned and Google-managed encryption key.

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
- Learn how to [query SAP Datasphere data](https://docs.cloud.google.com/bigquery/docs/sap-datasphere-federated-queries).