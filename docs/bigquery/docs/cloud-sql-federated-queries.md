# Cloud SQL federated queries

As a data analyst, you can query data in Cloud SQL from BigQuery
using [federated queries](https://docs.cloud.google.com/bigquery/docs/federated-queries-intro).

BigQuery Cloud SQL federation enables BigQuery
to query data residing in Cloud SQL in real time, without copying or
moving data. Query federation supports both MySQL (2nd generation) and PostgreSQL
instances in Cloud SQL.

Alternatively, to replicate data into
BigQuery, you can also use Cloud Data Fusion or
[Datastream](https://docs.cloud.google.com/datastream/docs/overview). For more about using
Cloud Data Fusion, see [Replicating data from MySQL to
BigQuery](https://docs.cloud.google.com/data-fusion/docs/tutorials/replicating-data/mysql-to-bigquery).

## Before you begin

- Ensure that your BigQuery administrator has created a [Cloud SQL connection](https://docs.cloud.google.com/bigquery/docs/connect-to-sql#create-sql-connection) and [shared](https://docs.cloud.google.com/bigquery/docs/connect-to-sql#share_connections) it with you.
-

  To get the permissions that
  you need to query a Cloud SQL instance,

  ask your administrator to grant you the
  [BigQuery Connection User](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquery#bigquery.connectionUser) (`roles/bigquery.connectionUser`) IAM role on your project.


  For more information about granting roles, see [Manage access to projects, folders, and organizations](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).


  You might also be able to get
  the required permissions through [custom
  roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles) or other [predefined
  roles](https://docs.cloud.google.com/iam/docs/roles-overview#predefined).

## Query data

To send a federated query to Cloud SQL from a
GoogleSQL query, use the
[`EXTERNAL_QUERY` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/federated_query_functions#external_query).

Suppose that you store a customer table in BigQuery, while
storing a sales table in Cloud SQL, and want to join the two tables in
a single query. The following example makes a federated query to a
Cloud SQL table named `orders` and joins the results with a
BigQuery table named `mydataset.customers`.

    SELECT c.customer_id, c.name, rq.first_order_date
    FROM mydataset.customers AS c
    LEFT OUTER JOIN EXTERNAL_QUERY(
      'us.connection_id',
      '''SELECT customer_id, MIN(order_date) AS first_order_date
      FROM orders
      GROUP BY customer_id''') AS rq ON rq.customer_id = c.customer_id
    GROUP BY c.customer_id, c.name, rq.first_order_date;

The example query includes 3 parts:

1. Run the external query `SELECT customer_id, MIN(order_date) AS
   first_order_date FROM orders GROUP BY customer_id` in the operational PostgreSQL database to get the first order date for each customer through the `EXTERNAL_QUERY()` function.
2. Join the external query result table with the customers table in BigQuery by `customer_id`.
3. Select customer information and first order date.

## View a Cloud SQL table schema

You can use the `EXTERNAL_QUERY()` function to query information_schema tables
to access database metadata, such as list all tables in the database or show
table schema. The following example information_schema queries work in both
MySQL and PostgreSQL. You can learn more from
[MySQL information_schema tables](https://dev.mysql.com/doc/refman/8.0/en/information-schema-introduction.html)
and
[PostgreSQL information_schema tables](https://www.postgresql.org/docs/9.1/information-schema.html).

    -- List all tables in a database.
    SELECT * FROM EXTERNAL_QUERY("connection_id",
    "select * from information_schema.tables;");

    -- List all columns in a table.
    SELECT * FROM EXTERNAL_QUERY("connection_id",
    "select * from information_schema.columns where table_name='x';");

## Connection details

The following table shows the Cloud SQL connection properties:

| Property name | Value | Description |
|---|---|---|
| `name` | string | Name of the connection resource in the format: project_id.location_id.connection_id. |
| `location` | string | Location of the connection which must either match the Cloud SQL instance location or be a multi-region of the corresponding jurisdiction. For example, a Cloud SQL instance in `us-east4` can use `US`, while a Cloud SQL instance in `europe-north1` can use `EU`. Only BigQuery queries running in this location will be able to use this connection. |
| `friendlyName` | string | A user-friendly display name for the connection. |
| `description` | string | Description of the connection. |
| `cloudSql.type` | string | Can be "POSTGRES" or "MYSQL". |
| `cloudSql.instanceId` | string | Name of the [Cloud SQL instance](https://docs.cloud.google.com/sql/docs/mysql/instance-settings#instance-id-2ndgen), usually in the format of: `Project-id:location-id:instance-id` You can find the instance ID in the [Cloud SQL instance](https://console.cloud.google.com/sql/instances) detail page. |
| `cloudSql.database` | string | The Cloud SQL database that you want to connect to. |
| `cloudSql.serviceAccountId` | string | The service account configured to access the Cloud SQL database. |

The following table shows the properties for the Cloud SQL instance credential:

| Property name | Value | Description |
|---|---|---|
| `username` | string | Database username |
| `password` | string | Database password |

## Track BigQuery federated queries

When you run a federated query against Cloud SQL,
BigQuery annotates the query with a comment similar to the following:

```
/* Federated query from BigQuery. Project ID: PROJECT_ID, BigQuery Job ID: JOB_ID. */
```

If you are monitoring logs for query usage on a MySQL or PostgreSQL database,
the following annotation can help you identify queries coming from BigQuery.

1. Go to the **Logs Explorer** page.

   [Go to the Logs Explorer](https://console.cloud.google.com/logs/query)
2. In the **Query** tab, enter the following query:

       resource.type="cloudsql_database"
       textPayload=~"Federated query from BigQuery"

3. Click **Run query**.

   If there are records available for BigQuery federated queries,
   a list of records similar to the following appears in **Query results**:

   ```
   YYYY-MM-DD hh:mm:ss.millis UTC [3210064]: [4-1]
   db=DATABASE, user=USER_ACCOUNT
   STATEMENT: SELECT 1 FROM (SELECT FROM company_name_table) t;
   /* Federated query from BigQuery.
   Project ID: PROJECT_ID, BigQuery Job ID: JOB_ID
   */

   YYYY-MM-DD hh:mm:ss.millis UTC [3210532]: [2-1]
   db=DATABASE, user=USER_ACCOUNT
   STATEMENT: SELECT "company_id", "company type_id" FROM
   (SELECT FROM company_name_table) t;
   /* Federated query from BigQuery.
   Project ID: PROJECT_ID, BigQuery Job ID: JOB_ID
   */
   ```

## Troubleshooting

This section helps you troubleshoot issues you might encounter when sending
a federated query to Cloud SQL.

**Issue:** Failed to connect to database server. If you are querying a MySQL
database, you might encounter the following error:

`Invalid table-valued function EXTERNAL_QUERY Failed to connect to MySQL database. Error: MysqlErrorCode(2013): Lost connection to MySQL server during query.`

Alternatively, if you are querying a PostgreSQL database, you might encounter
the following error:

`Invalid table-valued function EXTERNAL_QUERY Connect to PostgreSQL server failed: server closed the connection unexpectedly This probably means the server terminated abnormally before or while processing the request.`

:   **Resolution:** Ensure that valid credentials were used and all prerequisites
    were followed to create the [connection for Cloud SQL](https://docs.cloud.google.com/bigquery/docs/connect-to-sql).
    Check if the
    service account that is automatically created
    when a connection to Cloud SQL is
    created has the Cloud SQL Client (`roles/cloudsql.client`) role. The service
    account is of the following format:
    `service-PROJECT_NUMBER@gcp-sa-bigqueryconnection.iam.gserviceaccount.com`.
    For detailed instructions, see
    [Grant access to the service account](https://docs.cloud.google.com/bigquery/docs/connect-to-sql#access-sql).

    If your Cloud SQL instance uses a private IP address, ensure that you
    enabled a private path when you
    [created the Cloud SQL instance](https://docs.cloud.google.com/sql/docs/postgres/create-instance).
    Doing so lets BigQuery access data in Cloud SQL and
    run queries against this data over a private connection.

**Issue:** Failed to connect to MySQL database. When you run a
federated query against Cloud SQL data, you might encounter the following error:

`Invalid table-valued function EXTERNAL_QUERY Failed to connect to MySQL database. Error: MysqlErrorCode(2059): Authentication plugin 'caching_sha2_password' cannot be loaded: /usr/lib/plugin/caching_sha2_password.so: cannot open shared object file: No such file or directory at [1:15]`

:   **Resolution:**

    This error occurs for database users that use
    [caching_sha2_password authentication](https://dev.mysql.com/doc/refman/8.4/en/native-pluggable-authentication.html),
    which isn't supported for federated queries.

    If you use a Cloud SQL instance running MySQL version
    8.0 or earlier, you can change the authentication plugin for users
    connecting from BigQuery. To change the authentication plugin,
    run the following command on the Cloud SQL instance:

        ALTER USER 'USERNAME'@'%' IDENTIFIED WITH mysql_native_password BY 'PASSWORD';

    Replace the following:

    - `USERNAME`: the database user account that BigQuery uses to authenticate and connect to your Cloud SQL for MySQL instance.
    - `PASSWORD`: the password for the database user.

    If you use a Cloud SQL instance running MySQL version
    8.4, there's no workaround available, because the `mysql_native_password`
    plugin is deprecated for version 8.4. We don't recommend downgrading an
    existing database from 8.4 to 8.0 as a solution for production workloads.

    Upgrading a Cloud SQL instance that runs a MySQL
    version earlier than 8.4 to version 8.4 doesn't affect existing database
    connections, provided that you don't change the authentication plugin for
    users before the upgrade.

    Because only users created with the `mysql_native_password` plugin
    (typically those created before upgrading to version 8.4) can be used for
    federated queries, even after the MySQL instance upgrades to the
    8.4 version, you should send federated queries to MySQL with
    connection settings that use a database user that existed on a
    MySQL version earlier than 8.4.

## Limitations

- The MySQL `caching_sha2_password` plugin isn't supported for federated queries. As a result, federated queries for MySQL 8.0 and 8.4 using this plugin will fail. For more information, see [Troubleshooting](https://docs.cloud.google.com/bigquery/docs/cloud-sql-federated-queries#troubleshooting).

## What's next

- Learn about [federated queries](https://docs.cloud.google.com/bigquery/docs/federated-queries-intro).
- Learn about [MySQL to BigQuery data type mapping](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/federated_query_functions#mysql_mapping).
- Learn about [PostgreSQL to BigQuery data type mapping](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/federated_query_functions#postgresql_mapping).
- Learn about [unsupported data types](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/federated_query_functions#unsupported_data_types).