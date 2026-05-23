# Connect to AlloyDB for PostgreSQL

As a BigQuery administrator, you can create a
[connection](https://docs.cloud.google.com/bigquery/docs/connections-api-intro) to access
AlloyDB data. This connection lets data analysts [query
data in AlloyDB](https://docs.cloud.google.com/bigquery/docs/alloydb-federated-queries).

To connect to AlloyDB, you must perform the following steps:

1. [Create an AlloyDB connection](https://docs.cloud.google.com/bigquery/docs/connect-to-alloydb#create-alloydb-connection).

2. [Grant access to the service account](https://docs.cloud.google.com/bigquery/docs/connect-to-alloydb#access-alloydb).

## Before you begin

1. Enable the BigQuery Connection API.  


   [Enable the API](https://console.cloud.google.com/apis/library/bigqueryconnection.googleapis.com)
2.

   To get the permissions that
   you need to create an AlloyDB connection,

   ask your administrator to grant you the
   [BigQuery Connection Admin](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquery#bigquery.connectionAdmin) (`roles/bigquery.connectionAdmin`) IAM role on the project.


   For more information about granting roles, see [Manage access to projects, folders, and organizations](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).


   You might also be able to get
   the required permissions through [custom
   roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles) or other [predefined
   roles](https://docs.cloud.google.com/iam/docs/roles-overview#predefined).

## Create an AlloyDB connection

As a best practice, use connections to handle database credentials when you are
connecting to AlloyDB. Connections are encrypted and stored securely in the
BigQuery connection service. If the user credentials are valid
for other data in the source, you can reuse the connection. For example, you
can use one connection to query the same database in an AlloyDB instance multiple times.

Select one of the following options to create an AlloyDB connection:

### Console

1. Go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the **Explorer** pane, click **Add**.

   The **Add data** dialog opens.
3. In the **Filter By** pane, in the **Data Source Type** section, select **Databases**.

   Alternatively, in the **Search for data sources** field, you can enter
   `alloydb`.
4. In the **Featured data sources** section, click **Google Cloud AlloyDB**.

5. Click the **AlloyDB: BigQuery Federation** solution card.

6. In the **External data source** dialog, enter the following information:

   - For **Connection type** , select **AlloyDB**.
   - For **Connection ID** , enter an identifier for the connection resource. Letters, numbers, and underscores are allowed. For example, `bq_alloydb_connection`.
   - For **Data location** , select a BigQuery location (or region) that is [compatible with your external data source region](https://docs.cloud.google.com/bigquery/docs/federated-queries-intro#supported_regions).
   - Optional: For **Friendly name** , enter a user-friendly name for the connection, such as `My connection resource`. The friendly name can be any value that helps you identify the connection resource if you need to modify it later.
   - Optional: For **Description**, enter a description for this connection resource.
   - Optional: **Encryption** If you want to use a [customer-managed encryption key (CMEK)](https://docs.cloud.google.com/bigquery/docs/customer-managed-encryption) to encrypt your credentials, select **Customer-managed encryption key (CMEK)** and then select a customer-managed key. Otherwise, your credentials are protected by the default Google-owned and Google-managed encryption key.
   - For **Database name**, enter the name of the database.
   - For **Database username**, enter the username for the database.
   - For **Database password** , enter the password for the database.
     - Optional: To see the password, click **Show password**.
   - For **AlloyDB Instance** , enter the connection URI of the AlloyDB
     primary or read instance with the **//alloydb.googleapis.com** prefix.

     - Sample URI: `//alloydb.googleapis.com/projects/PROJECT_ID/locations/REGION_ID/clusters/CLUSTER_NAME/instances/INSTANCE_ID`

     > [!NOTE]
     > **Note:** If the same user credentials are valid for other databases in the external data source, that user can query those databases through the same connection resource.

7. Click **Create connection**.

8. Click **Go to connection**.

9. In the **Connection Info** pane, copy the service account ID for use in the
   [next step](https://docs.cloud.google.com/bigquery/docs/connect-to-alloydb#share_connections) to grant the correct IAM permissions.

### bq

Enter the [`bq mk`](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#bq_mk) command
with the following flags:

      bq mk \
      --connection \
      --location=LOCATION \
      --project_id=PROJECT_ID \
      --connector_configuration '{
        "connector_id": "google-alloydb",
        "asset": {
          "database": "DATABASE",
          "google_cloud_resource": "RESOURCE_PATH"
        },
        "authentication": {
          "username_password": {
            "username": "USERNAME",
            "password": {
              "plaintext": "PASSWORD"
            }
          }
        }
      }' \
      CONNECTION_ID

Replace the following:

- `LOCATION`: Specify a region of the BigQuery dataset to be combined with the data from AlloyDB. Queries that use this connection must be run from this region.
- `PROJECT_ID`: Enter your Google Cloud project ID.
- `DATABASE`: Enter the database name.
- `RESOURCE_PATH`: Enter the connection URI of the AlloyDB primary or read instance with the **//alloydb.googleapis.com** prefix.
  - Sample URI: `//alloydb.googleapis.com/projects/PROJECT_ID/locations/REGION_ID/clusters/CLUSTER_NAME/instances/INSTANCE_ID`
- `USERNAME`: Enter the database user's name.
- `PASSWORD`: Enter the database user's password.
- `CONNECTION_ID`: Enter a connection ID to identify this connection.

Optional flag:

- `--kms_key_name`: A customer-managed encryption key. If omitted, credentials are protected by the default Google-owned and Google-managed encryption key.

### API

Within the BigQuery Connection API, you can invoke `CreateConnection` within
the `ConnectionService` to instantiate a connection. See the [client library page](https://docs.cloud.google.com/bigquery/docs/reference/bigqueryconnection) for more details.

## Grant access to the service account

A [service account](https://docs.cloud.google.com/iam/docs/service-agents) is automatically created when you
create the first connection within a project. The service account's name is
**BigQuery Connection Service Agent**. The service account ID is of the
following format:

`service-PROJECT_NUMBER@gcp-sa-bigqueryconnection.iam.gserviceaccount.com`.

To connect to AlloyDB, you must give the new connection access to AlloyDB so that BigQuery can access data on behalf of users. The service account must have the following permission:

- `alloydb.instances.connect`

You can grant the service account associated with the connection the
[AlloyDB Client IAM role](https://docs.cloud.google.com/alloydb/docs/reference/iam-roles-permissions#roles), which already has this permission assigned.
You can omit this step if the service account already has the required permission.

### Console

1. Go to the **IAM \& Admin** page.

   [Go to IAM \& Admin](https://console.cloud.google.com/project/_/iam-admin)
2. Click **Grant Access**.

   The **Add principals** dialog opens.
3. In the **New principals** field, enter the service account name
   **BigQuery Connection Service Agent** or the service account ID taken from
   the [connection information](https://docs.cloud.google.com/bigquery/docs/working-with-connections#view-connections).

4. In the **Select a role** field, select **AlloyDB** , and then select
   **AlloyDB Client**.

5. Click **Save**.

### gcloud

Use the
[`gcloud projects add-iam-policy-binding`](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access#grant-single-role)
command:

```
gcloud projects add-iam-policy-binding PROJECT_ID --member=serviceAccount:SERVICE_ACCOUNT_ID --role=roles/alloydb.client
```

Provide the following values:

- `PROJECT_ID`: Your Google Cloud project ID.
- `SERVICE_ACCOUNT_ID`: Replace project number in `service-PROJECT_NUMBER@gcp-sa-bigqueryconnection.iam.gserviceaccount.com` and use it.

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

   Connections are listed in your project,
   in a group called **Connections**.
2. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)

   If you don't see the left pane, click **Expand left pane** to open the pane.
3. In the **Explorer** pane, expand your project name.

4. Click **Connections** and then click the connection.

5. In the **Details** pane, click **Share** to share a connection.
   Then do the following:

   1. In the **Connection permissions** dialog, share the
      connection with other principals by adding or editing
      principals.

   2. Click **Save**.

### bq


Use the following [`set-iam-policy` command](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#bq_set-iam-policy):

      bq set-iam-policy RESOURCE FILE_NAME

Replace the following:

- `RESOURCE`: Enter the resource name in the `project_id.region.connection_id` or `region.connection_id` format.
- `FILE_NAME`: Enter the filename that contains the IAM policy in a JSON format.

For more information about the set-iam-policy command, see [Control access to resources with IAM](https://docs.cloud.google.com/bigquery/docs/control-access-to-resources-iam#bq).

### API


Use the
[`projects.locations.connections.setIAM` method](https://docs.cloud.google.com/bigquery/docs/reference/bigqueryconnection/rest/v1/projects.locations.connections#methods)
in the BigQuery Connections REST API reference section and
supply an instance of the `policy` resource.

## What's next

- Learn about different [connection types](https://docs.cloud.google.com/bigquery/docs/connections-api-intro).
- Learn about [managing connections](https://docs.cloud.google.com/bigquery/docs/working-with-connections).
- Learn about [federated queries](https://docs.cloud.google.com/bigquery/docs/federated-queries-intro).
- Learn how to [query AlloyDB data](https://docs.cloud.google.com/bigquery/docs/alloydb-federated-queries).