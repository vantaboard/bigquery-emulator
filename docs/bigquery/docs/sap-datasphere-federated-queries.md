# SAP Datasphere federated queries

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

As a data analyst, you can query relational data in SAP Datasphere from BigQuery using federated queries.

BigQuery SAP Datasphere federation lets BigQuery query data residing in SAP Datasphere in real time, without copying or moving data.

To run a SQL query in SAP Datasphere, specify that SQL query within BigQuery in a `EXTERNAL_QUERY` function. The results are then transferred from SAP Datasphere to BigQuery.

## Limitations

- You can only query relational views that are [exposed for consumption](https://help.sap.com/docs/SAP_DATASPHERE/43509d67b8b84e66a30851e832f66911/d7d56284bb5148c887ac4054689bfbca.html?locale=en-US). Other objects in SAP Datasphere are not accessible to the query federated through `EXTERNAL_QUERY`.
- The federated query latency might be noticeably higher than the same query if it was executed directly in SAP Datasphere.
- The first query that uses SAP Datasphere connection in a given project might take more than a minute to run.
- No additional [SQL pushdowns](https://docs.cloud.google.com/bigquery/docs/federated-queries-intro#sql_pushdowns) are supported for SAP Datasphere.
- The SAP Datasphere SQL query must specify aliases for columns that contain function results.
- When the usage of Compute Engine API in the query project is restricted by [VPC Service Controls](https://docs.cloud.google.com/vpc-service-controls/docs/overview), the federated query will fail.

## Before you begin

Ensure that your BigQuery administrator has [created a SAP Datasphere connection](https://docs.cloud.google.com/bigquery/docs/connect-to-sap-datasphere)
and [shared](https://docs.cloud.google.com/bigquery/docs/connect-to-sap-datasphere#share_connections)
it with you.

### Required roles


To get the permissions that
you need to query SAP Datasphere,

ask your administrator to grant you the
[BigQuery Connection User](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquery#bigquery.connectionUser) (`roles/bigquery.connectionUser`) IAM role on the project.


For more information about granting roles, see [Manage access to projects, folders, and organizations](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).


You might also be able to get
the required permissions through [custom
roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles) or other [predefined
roles](https://docs.cloud.google.com/iam/docs/roles-overview#predefined).

## Query data

To send a federated query to SAP Datasphere from a GoogleSQL query, use the
[EXTERNAL_QUERY function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/federated_query_functions#external_query).

The following example is a federated query that joins a table in SAP Datasphere named `ORDERS` and a table in BigQuery named `mydataset.customers`.

    SELECT c.customer_id, c.name, rq.first_order_date
    FROM mydataset.customers AS c
    LEFT OUTER JOIN EXTERNAL_QUERY(
      'connection_id',
      '''SELECT CUSTOMER_ID, MIN(ORDER_DATE) AS first_order_date
         FROM ORDERS
         GROUP BY CUSTOMER_ID''') AS rq
      ON rq.customer_id = c.customer_id
    GROUP BY c.customer_id, c.name, rq.first_order_date;

## View a SAP Datasphere table schema

The following examples use the [EXTERNAL_QUERY function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/federated_query_functions#external_query) to retrieve database metadata from the
`SYS` schema in SAP Datasphere.

    -- List all views in a schema.
    SELECT * FROM EXTERNAL_QUERY(
      'connection_id',
      '''SELECT VIEW_NAME FROM SYS.VIEWS
         WHERE SCHEMA_NAME = 'MY_SCHEMA'''');

    -- List all columns in a view.
    SELECT * FROM EXTERNAL_QUERY(
      'connection_id',
      '''SELECT COLUMN_NAME, DATA_TYPE_NAME
         FROM SYS.VIEW_COLUMNS
         WHERE SCHEMA_NAME = 'MY_SCHEMA' AND
               VIEW_NAME = 'my_view'
         ORDER BY POSITION''');

## Pricing

The cost of running a federated query is based on three factors:

- The compute cost of executing the query in SAP Datasphere.
- The bandwidth cost of transferring the query results from SAP Datasphere to BigQuery.
- The compute cost of executing the query in BigQuery.

Any SAP Datasphere related costs depend on the type of SAP service you use. To
limit the bandwidth cost, we recommend that you write the query in the
`EXTERNAL_QUERY` so that it excludes all columns and rows that are not needed to
compute the final result.

There is no additional cost for running federated queries in BigQuery.
For more information about BigQuery pricing, see [Pricing](https://cloud.google.com/bigquery/pricing).

## What's next

- Learn about [federated queries](https://docs.cloud.google.com/bigquery/docs/federated-queries-intro).
- Learn about [unsupported data types](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/federated_query_functions#unsupported_data_types).