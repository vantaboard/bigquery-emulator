# AlloyDB federated queries

As a data analyst, you can query data in AlloyDB for PostgreSQL from BigQuery
using [federated queries](https://docs.cloud.google.com/bigquery/docs/federated-queries-intro).

BigQuery AlloyDB federation lets BigQuery
query data residing in AlloyDB in real time without copying or
moving the data.

## Before you begin

- Ensure that your BigQuery administrator has created an [AlloyDB connection](https://docs.cloud.google.com/bigquery/docs/connect-to-alloydb#create-alloydb-connection) and [shared](https://docs.cloud.google.com/bigquery/docs/connect-to-alloydb#share_connections) it with you.
-

  To get the permissions that
  you need to query an AlloyDB instance,

  ask your administrator to grant you the
  [BigQuery Connection User](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquery#bigquery.connectionUser) (`roles/bigquery.connectionUser`) IAM role on your project.


  For more information about granting roles, see [Manage access to projects, folders, and organizations](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).


  You might also be able to get
  the required permissions through [custom
  roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles) or other [predefined
  roles](https://docs.cloud.google.com/iam/docs/roles-overview#predefined).

## Query data

To send a federated query to AlloyDB from a
GoogleSQL query, use the
[`EXTERNAL_QUERY` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/federated_query_functions#external_query).

Suppose that you store a customer table in BigQuery, while
storing a sales table in AlloyDB, and want to join the two tables in
a single query. The following example makes a federated query to an
AlloyDB table named `orders` and joins the results with a
BigQuery table named `mydataset.customers`.

The example query includes 3 parts:

1. Run the external query `SELECT customer_id, MIN(order_date) AS
   first_order_date FROM orders GROUP BY customer_id` in the AlloyDB database
   to get the first order date for each customer through
   the `EXTERNAL_QUERY` function.

2. Join the external query results table with the customers table in
   BigQuery by `customer_id`.

3. Select customer information and first order date in the final result set.

    SELECT c.customer_id, c.name, rq.first_order_date
    FROM mydataset.customers AS c
    LEFT OUTER JOIN EXTERNAL_QUERY(
      'us.connection_id',
      '''SELECT customer_id, MIN(order_date) AS first_order_date
      FROM orders
      GROUP BY customer_id''') AS rq ON rq.customer_id = c.customer_id
    GROUP BY c.customer_id, c.name, rq.first_order_date;

## View an AlloyDB table schema

You can use the `EXTERNAL_QUERY` function to query `information_schema` tables
to access database metadata. For example, you can list all of the tables in the
database or view the table schema. For more information, see [PostgreSQL information_schema tables](https://www.postgresql.org/docs/9.1/information-schema.html).

    -- List all tables in a database.
    SELECT * FROM EXTERNAL_QUERY("region.connection_id",
    "select * from information_schema.tables;");

    -- List all columns in a table.
    SELECT * FROM EXTERNAL_QUERY("region.connection_id",
    "select * from information_schema.columns where table_name='x';");

## Track BigQuery federated queries

When you run a federated query against AlloyDB,
BigQuery annotates the query with a comment similar to the following:

```
/* Federated query from BigQuery. Project ID: PROJECT_ID, BigQuery Job ID: JOB_ID. */
```

If you are monitoring logs for query usage, the following annotation can help you
identify queries coming from BigQuery.

1. Go to the **Logs Explorer** page.

   [Go to the Logs Explorer](https://console.cloud.google.com/logs/query)
2. In the **Query** tab, enter the following query:

       resource.type="alloydb.googleapis.com/Instance"
       textPayload=~"Federated query from BigQuery"

3. Click **Run query**.

   If there are records available for BigQuery federated queries,
   a list of records similar to the following appears in **Query results**.

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

   For more information about Cloud Logging, see [Cloud Logging](https://docs.cloud.google.com/logging/docs/view/logs-explorer-interface).

## Troubleshooting

This section describes potential errors you might encounter when sending a
federated query to AlloyDB and provides possible troubleshooting
resolutions.

**Issue:** Failed to connect to the database server with this error:
`Invalid table-valued function EXTERNAL_QUERY Connect to PostgreSQL server failed: server closed the connection unexpectedly. This probably means the server terminated abnormally before or while processing the request.`

**Resolution:** Ensure that you used valid credentials and followed all prerequisites
while creating the [connection to AlloyDB](https://docs.cloud.google.com/bigquery/docs/connect-to-alloydb#create-alloydb-connection).
Check if the service account that is automatically created
when a connection to AlloyDB is
created has the AlloyDB Client (`roles/alloydb.client`) role.
For more information, see
[Grant access to the service account](https://docs.cloud.google.com/bigquery/docs/connect-to-alloydb#access-alloydb).

## What's next

- Learn about [federated queries](https://docs.cloud.google.com/bigquery/docs/federated-queries-intro).
- Learn about [PostgreSQL to BigQuery data type mapping](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/federated_query_functions#postgresql_mapping).
- Learn about [unsupported data types](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/federated_query_functions#unsupported_data_types).