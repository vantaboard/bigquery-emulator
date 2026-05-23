# Global queries

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
> **Note:** To get support or provide feedback for this preview feature, contact [bq-xregion-support@google.com](mailto:bq-xregion-support@google.com).

Global queries let you run SQL queries that reference data stored in more than one region.
For example, you can run a global query that joins a table located in `us-central1`
with a table located in `europe-central2`. This document explains how to enable and run global queries in your project.

## Before you begin

Verify that global queries are enabled for your project and ensure that you have
the necessary permissions to run global queries.

### Enable global queries

To enable global queries for your project or organization, use the
[`ALTER PROJECT SET OPTIONS` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#alter_project_set_options_statement)
or [`ALTER ORGANIZATION SET OPTIONS` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#alter_organization_set_options_statement)
to change the [default configuration](https://docs.cloud.google.com/bigquery/docs/default-configuration).

- To run global queries in a region, set the `enable_global_queries_execution` argument to `true` in that region for the project **running** the query.
- To allow global queries to copy data from a region, set the `enable_global_queries_data_access` argument to `true` in that region for the project **containing the data**.
- Any time your query accesses remote tables, these options are checked.
- Global queries can run in one project and pull data from other regions from another project.

### Example: Cross-project configuration

The following example shows how to run a query in one project that accesses a table in another project.

Suppose you have a project `query_project` running jobs in the `us-central1` region, and you want to run a query that accesses a table `data_project.dataset.my_table` located in the `europe-west1` region:

    SET @@location='us-central1';
    SELECT
      *
    FROM
      `query_project.dataset.my_table`
      JOIN `data_project.dataset.my_other_table` USING id;

To allow this global query to execute successfully, the following configuration is required:

1. You need to enable execution of global queries in the project (`query_project`) in the region running a global query (`us-central1`):

   ```googlesql
   ALTER PROJECT ``query_project``
   SET OPTIONS (
   `region-`us-central1`.enable_global_queries_execution` = TRUE
   );
   ```
2. You need to enable copying data by global queries from the project containing the data (`data_project`) for its region (`europe-west1`):

   ```googlesql
   ALTER PROJECT ``data_project``
   SET OPTIONS (
   `region-`europe-west1`.enable_global_queries_data_access` = TRUE
   );
   ```

To create and use [views](https://docs.cloud.google.com/bigquery/docs/views-intro) that contain remote tables, the same principles apply: the project running the queries must have `enable_global_queries_execution` enabled.

These `ALTER PROJECT` operations must be run separately as they refer to different projects and regions.
It can take several minutes for the change to take effect.

### Required permission

To run a global query, you must have the `bigquery.jobs.createGlobalQuery` permission.
The BigQuery Admin role is the only predefined role that contains this permission. To grant permission to run global queries without granting the BigQuery Admin role, follow these steps:

1. Create a [custom role](https://docs.cloud.google.com/iam/docs/creating-custom-roles), for example "BigQuery global queries executor".
2. Add `bigquery.jobs.createGlobalQuery` to this role.
3. Assign this role to selected users or service accounts.

## Query data

To run a global query, you write a SQL query as you would if your data was in a single location. If the data referenced by the query is stored in more than one location, BigQuery tries to execute a global query. In some cases, BigQuery [automatically selects the location](https://docs.cloud.google.com/bigquery/docs/global-queries#automatic-location-selection) of the query. Otherwise, you must [specify the location](https://docs.cloud.google.com/bigquery/docs/locations#specify_locations) in which to run the query. Data referenced by the query that doesn't reside in the selected location is copied to that location.

The following example runs as a global query that unions tables from two different datasets stored in two different locations:

    SELECT id, tr_date, product_id, price FROM us_dataset.transactions
    UNION ALL
    SELECT id, tr_date, product_id, price FROM europe_dataset.transactions

### Automatic location selection

In the following cases, the location in which a query must be executed is
determined automatically and can't be changed:

- Data modification language queries (`INSERT`, `UPDATE`, `DELETE` statements) are always executed in a location of the target table.
- Data definition language queries, such as `CREATE TABLE AS SELECT` statement, are always executed in the location in which a resource is created or modified.
- Queries with a [specified destination table](https://docs.cloud.google.com/bigquery/docs/writing-results#permanent-table) are always executed in the location where the destination table is.

## Choose a location

In general, you decide where your global queries are executed. To make that decision, consider the following:

- Global queries temporarily copy data from one location to another. If your organization has any requirements for data residency, and you don't want your data from location A to leave location A, set the query location to A.

- To minimize the amount of data transferred between locations and reduce the cost of the query, run your query in the region where most of the queried data is stored.

Imagine you have an online store and you keep a list of your products in location
`us-central1`, but transactions in `us-south1` region. If there are more transactions
than products in your catalog then you should run the query in the `us-south1` region.

## Understand global queries

In order to run global queries in an efficient and cost-effective way, it's
important to understand the mechanism behind their execution.

To use data that resides in different locations, it must be replicated to one
location. The following is an abstraction of the global query workflow carried out by BigQuery:

1. Determine where the query must be executed, either from [user's declaration](https://docs.cloud.google.com/bigquery/docs/locations#specify_locations) or [automatically](https://docs.cloud.google.com/bigquery/docs/global-queries#automatic-location-selection). This location is called the *primary* location, and all other locations referenced by the query are *remote*.
2. Run a sub-query in each remote region to collect the data that is needed to finish the query in the primary region.
3. Copy this data from remote locations to the primary location.
4. Save the data in temporary tables in the primary location for 8 hours.
5. Run a final query with all data collected in the primary location.
6. Return the query results.

BigQuery tries to minimize the amount of data transferred between regions.
Consider the following example:

    SET @@location = 'EU';
    SELECT
      t1.col1, t2.col2
    FROM
      eu_dataset.table1 t1
      JOIN us_dataset.table2 t2 using col3
    WHERE
      t2.col4 = 'ABC'

BigQuery doesn't need to replicate all of table `t2` from the US to the EU.
It is sufficient to transfer only the requested columns (`col2` and `col3`)
and only the rows that match the `WHERE` condition (`t2.col4 = 'ABC'`).
However, these mechanisms, known as *pushdowns*, depend on the query structure
and sometimes the amount of data transferred might be large.
We recommend that you test global queries on a small subset of data
and confirm that data is only transferred when needed.

## Observability

To see the query text sent to the remote region, check the [job history](https://docs.cloud.google.com/bigquery/docs/managing-jobs#list_jobs_in_a_project). The remote
job has the same job ID as the original query with an additional `_xregion` suffix.

## Turn off global queries

To disable global queries for your project or organization, use the
[`ALTER PROJECT SET OPTIONS statement`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#alter_project_set_options_statement)
or [`ALTER ORGANIZATION SET OPTIONS statement`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#alter_organization_set_options_statement)
to change the [default configuration](https://docs.cloud.google.com/bigquery/docs/default-configuration).

- To turn off global queries in a region, set the `enable_global_queries_execution` argument to `false` or `NULL` in that region.
- To forbid global queries from copying data from a region, set the `enable_global_queries_data_access` argument to `false` or `NULL` in that region.

The following example shows how to disable global queries at the project level:

    ALTER PROJECT PROJECT_ID
    SET OPTIONS (
      `region-`REGION`.enable_global_queries_execution` = false,
      `region-`REGION`.enable_global_queries_data_access` = false
    );

Replace the following:

- `PROJECT_ID`: the name of the project to alter
- `REGION`: the name of the region in which to disable global queries

It can take several minutes for the change to take effect.

## Pricing

The cost of a global query consists of following components:

- The compute cost of every subquery in remote locations, based on your [pricing model](https://docs.cloud.google.com/bigquery/pricing#analysis_pricing_models) in these locations
- The compute cost of the final query in the region in which it's executed, based on your [pricing model](https://docs.cloud.google.com/bigquery/pricing#analysis_pricing_models) in that region
- The cost of copying data between different locations, according to [Data replication pricing](https://docs.cloud.google.com/bigquery/pricing#data_replication)
- The cost of storing data copied from remote regions to the primary region (for 8 hours), according to [Storage pricing](https://docs.cloud.google.com/bigquery/pricing#storage-pricing)

## Quotas

For information about quotas regarding global queries, see [Query jobs](https://docs.cloud.google.com/bigquery/quotas#query_jobs).

## Limitations

- A query's [execution details](https://docs.cloud.google.com/bigquery/docs/query-plan-explanation) and [execution graph](https://docs.cloud.google.com/bigquery/docs/query-insights) don't show the number of bytes processed and transferred from remote locations. This information appears in copy jobs that you can find in your job history. The job ID of a copy job created by a global query has the job ID of the query job as a prefix.
- Global queries are not supported in sandbox mode
- Global queries incur higher latency than single-region queries due to the time required to transfer data between regions.
- Global queries don't use any cache to avoid transferring data between regions.
- You can't query pseudocolumns, such `_PARTITIONTIME`, with global queries.
- You can't query `RANGE` type columns with global queries.
- You can't query columns using [flexible column names](https://docs.cloud.google.com/bigquery/docs/schemas#flexible-column-names) with global queries.
- You can't query [`INFORMATION_SCHEMA` views](https://docs.cloud.google.com/bigquery/docs/information-schema-intro) from a remote region in a global query.
- Global [authorized views](https://docs.cloud.google.com/bigquery/docs/authorized-views) and [authorized routines](https://docs.cloud.google.com/bigquery/docs/authorized-routines) are not supported (when a view or routine in one location is authorized to access dataset in another location).
- [Materialized views](https://docs.cloud.google.com/bigquery/docs/materialized-views-intro) over global queries are not supported.
- If your global query references `STRUCT` columns, no pushdowns are applied to any remote subqueries. To optimize performance, consider creating a view in the remote region that filters `STRUCT` columns and returns only the necessary fields as individual columns.
- Global queries are not executed atomically. In cases where data replication succeeds, but the overall query fails, you are still billed for the data replication.
- Temporary tables created in remote regions as part of global queries execution are only encrypted using [Customer-managed encryption keys (CMEK)](https://docs.cloud.google.com/bigquery/docs/customer-managed-encryption) if a CMEK key that was configured to encrypt the global query results (either on a table, dataset, or project level) is global. To ensure that remote temporary tables are always protected using CMEK, set a default KMS key for the project running global queries in the remote region.
- Global queries are not supported in [Assured Workloads](https://docs.cloud.google.com/assured-workloads/docs/overview).
- A single global query can access up to 10 remote tables per region.