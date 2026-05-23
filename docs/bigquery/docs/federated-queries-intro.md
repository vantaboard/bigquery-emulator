# Introduction to federated queries

This page introduces how to use federated queries and provides guidance on
querying Spanner, AlloyDB, and Cloud SQL data from BigQuery.


Federated queries let you send a query statement to AlloyDB, Spanner, or Cloud SQL databases
and get the result back as a temporary table. Federated queries use the
BigQuery Connection API to establish a connection with AlloyDB, Spanner, or Cloud SQL.
In your query, you use the `EXTERNAL_QUERY` function to send a
query statement to the external database, using that database's SQL dialect.
The results are converted to GoogleSQL data types.

## Supported data stores

You can use federated queries with the following data stores:

- [Spanner](https://docs.cloud.google.com/bigquery/docs/spanner-federated-queries)
- [Cloud SQL](https://docs.cloud.google.com/bigquery/docs/cloud-sql-federated-queries)
- [AlloyDB](https://docs.cloud.google.com/bigquery/docs/alloydb-federated-queries)
- [SAP Datasphere](https://docs.cloud.google.com/bigquery/docs/sap-datasphere-federated-queries) ([Preview](https://cloud.google.com/products/#product-launch-stages))

## Workflow

- Identify the Google Cloud project that includes the data source that you want to query.
- A `bigquery.admin` user creates a connection resource in BigQuery.
- The admin user [grants permission to use the connection resource](https://docs.cloud.google.com/bigquery/docs/working-with-connections#share-connections) to user B.
  - If the admin and user B are the same person, there is no need to grant permission.
- User B writes a query in BigQuery with the new [`EXTERNAL_QUERY`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/federated_query_functions#external_query) SQL function.

> [!CAUTION]
> **Caution:** The performance of federated queries might be lower than queries that read data residing in BigQuery storage.

## Alternatives to federated queries: external tables and datasets

Another option to query operational databases such as Bigtable, Spanner,
Cloud Storage, Google Drive, and Salesforce Data Cloud, is to use
external tables and datasets. External datasets and tables let you view tables
and their schemas and query them without using an
[`EXTERNAL_QUERY`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/federated_query_functions#external_query)
SQL function. You don't have to bring data back into BigQuery and
you can use the BigQuery syntax instead of writing in the specific
SQL database dialect of SQL.

## Supported regions

For a list of supported locations, see the following sections:

### AlloyDB and Cloud SQL

Federated queries are only supported in regions that support both
the external data source and BigQuery.

- [Cloud SQL instance location](https://docs.cloud.google.com/sql/docs/mysql/locations).
- [AlloyDB locations](https://docs.cloud.google.com/alloydb/docs/locations).
- [BigQuery dataset locations](https://docs.cloud.google.com/bigquery/docs/locations).

You can create a connection and run a federated query across regions according
to the following rules:

**Single regions**

A BigQuery single region can only query a resource in the same
region.

For example, if your dataset is in `us-east4`, you can query
Cloud SQL instances or AlloyDB instances that are
located in `us-east4`. The [query processing location](https://docs.cloud.google.com/bigquery/docs/locations#specify_locations)
is the BigQuery single region.

**Multi-regions**

A BigQuery multi-region can query any data source region in the
same large geographic area (US, EU). [Multi-regional locations](https://docs.cloud.google.com/sql/docs/mysql/locations)
aren't available for Cloud SQL instances, because these are only used
for backups.

- A query that runs in the BigQuery US multi-region can query any
  single region in the US geographic area, such as `us-central1`, `us-east4`, or `us-west2`.

  > [!CAUTION]
  > **Caution:** Querying external data sources located in `southamerica-east1` from BigQuery datasets in the US multi-region isn't supported.

- A query that runs in the BigQuery EU multi-region can query any
  single region in [member states](https://europa.eu/european-union/about-eu/countries_en)
  of the European Union, such as `europe-north1` or `europe-west3`.

- The location where the query runs must be the same as the location of the
  connection resource. For example, queries executed from the US multi-region
  must use a connection located in the US multi-region.

  > [!CAUTION]
  > **Caution:** Queries that originate in multi-regions can no longer reference connections in single regions. If you have an affected connection, then recreate the connection in the same multi-region as your query.

The query performance varies based on the proximity between the dataset and the
external data source. For example, a federated query between a dataset in
the US multi-region and a Cloud SQL instance in `us-central1` is
fast. However, if you run the same query between the US multi-region
and a Cloud SQL instance in `us-east4`, the performance might be slower.

The
[query processing location](https://docs.cloud.google.com/bigquery/docs/locations#specify_locations)
is the multi-region location, either `US` or `EU`.

### Spanner

For [Spanner, both regional and multi-regional
configurations](https://docs.cloud.google.com/spanner/docs/instance-configurations) are supported.
A BigQuery single region/multi-region can query a Spanner instance
in any supported Spanner region. For more details refer to
[cross region queries](https://docs.cloud.google.com/bigquery/docs/spanner-federated-queries#cross_region_queries).

## Data type mappings

When you execute a federated query, the data from the external data source
is converted to GoogleSQL
types. For more information, see
[Cloud SQL federated queries](https://docs.cloud.google.com/bigquery/docs/cloud-sql-federated-queries).

## Quotas and limits

- **Cross-region federated querying** . If the [BigQuery query processing location](https://docs.cloud.google.com/bigquery/docs/locations#specify_locations) and the external data source location are different, this is a cross-region query. You can run up to 1 TB in cross-region queries per project per day. The following is an example of a cross-region query.
  - The Cloud SQL instance is in `us-west1` while the BigQuery connection is based in the US multi-region. The BigQuery query processing location is `US`.
- **Quota**. Users should control query quota in the external data source, such as Cloud SQL or AlloyDB. There is no extra quota setting for federated querying. To achieve workload isolation, it's recommended to only query a database read replica.
- **Maximum bytes billed allowed**. This field isn't supported for federated queries. Calculating the bytes billed before actually executing the federated queries isn't possible.
- **Number of connections**. A federated query can have at most 10 unique connections.
- **Cloud SQL [MySQL](https://docs.cloud.google.com/sql/docs/mysql/quotas) and
  [PostgreSQL](https://docs.cloud.google.com/sql/docs/postgres/quotas)**. Quotas and limitations apply.

## Limitations

Federated queries are subject to the following limitations:

- **Performance**. A federated query is likely to not be as fast as querying
  only BigQuery storage. BigQuery needs to wait for
  the source database to execute the external query and temporarily move data
  from the external data source to BigQuery. Also, the source
  database might not be optimized for complex analytical queries.

  The query performance also varies based on the proximity between the dataset
  and the external data source. For more information, see [Supported
  regions](https://docs.cloud.google.com/bigquery/docs/federated-queries-intro#supported_regions).
- **Federated queries are read-only**. The external query that is executed
  in the source database must be read-only. Therefore, DML or DDL statements are
  not supported.

- **Unsupported data types**. If your external query contains a data type that
  is unsupported in BigQuery, the query fails immediately. You can
  cast the unsupported data type to a different supported data type.

- **Customer-managed encryption keys (CMEK)**. CMEK is configured separately for BigQuery and for external data sources.
  If you configure the source database to use CMEK but not BigQuery, then the temporary
  table that contains results of a federated query is encrypted with a Google-owned and Google-managed encryption key.

## Pricing

- If you are using the
  on-demand pricing model, you are charged for the number of bytes returned from
  the external query when executing federated queries from
  BigQuery. For more information, see [On-demand analysis
  pricing](https://cloud.google.com/bigquery/pricing#on_demand_pricing).

- If you are using
  BigQuery editions, you are charged based on the number of slots
  you use. For more information, see [Capacity compute
  pricing](https://cloud.google.com/bigquery/pricing#capacity_compute_analysis_pricing).

## SQL pushdowns

Federated queries are subject to the optimization technique known as SQL pushdowns.
They improve the performance of a query by delegating operations like filtering down
to the external data source instead of performing them in BigQuery.
Reducing the amount of data transferred from the external data source can reduce
query execution time and lower costs. SQL pushdowns include column pruning
(`SELECT` clauses) and filter pushdowns (`WHERE` clauses).

When you use the `EXTERNAL_QUERY` function, SQL pushdowns work by rewriting the original query.
In the following example, the `EXTERNAL_QUERY` function is used to communicate with a Cloud SQL database:

```googlesql
SELECT COUNT(*)
FROM (
  SELECT * FROM EXTERNAL_QUERY("CONNECTION_ID", "select * from operations_table")
  )
WHERE a = 'Y' AND b NOT IN ('COMPLETE','CANCELLED');
```

Replace `CONNECTION_ID` with the ID of the
BigQuery connection.

Without SQL pushdowns, the following query is sent to Cloud SQL:

```googlesql
SELECT *
FROM operations_table
```

When this query is executed, the entire table is sent back to BigQuery,
even though only some rows and columns are needed.

With SQL pushdowns, the following query is sent to Cloud SQL:

```googlesql
SELECT `a`, `b`
FROM (
  SELECT * FROM operations_table) t
WHERE ((`a` = 'Y') AND (NOT `b` IN ('COMPLETE', 'CANCELLED')))
```

When this query is executed, only two columns and the rows that match the filtering
predicate are sent back to BigQuery.

SQL pushdowns are also applied when running federated queries with [Spanner external datasets](https://docs.cloud.google.com/bigquery/docs/spanner-external-datasets).

You can examine applied pushdowns (if any) in the [query plan](https://docs.cloud.google.com/bigquery/docs/query-plan-explanation#explanation_for_federated_queries).

### Limitations

SQL pushdowns have various limitations that vary depending on the external data source and on the way you query data.

#### Limitations for query federation when using `EXTERNAL_QUERY`

- SQL pushdowns are only applied to federated queries of the form `SELECT * FROM T`.
- Only column pruning and filter pushdowns are supported. Specifically, compute, join, limit, order by and aggregation pushdowns aren't supported.
- For filter pushdowns, literals must be of one of the following types: `BOOL`, `INT64`, `FLOAT64`, `STRING`, `DATE`, `DATETIME`, `TIMESTAMP`. Literals that are structs aren't supported.
- SQL function pushdowns are applied only for functions that are supported by both BigQuery and a destination database.
- SQL pushdowns are only supported for AlloyDB, Cloud SQL, and Spanner.
- SQL pushdowns aren't supported for SAP Datasphere.

#### Limitations for query federation when using Spanner external datasets

- Column pruning, filter, compute and partial aggregation pushdowns are supported. Specifically, join, limit and order by pushdowns aren't supported.
- For filter pushdowns, literals must be one of the following types: `BOOL`, `INT64`, `FLOAT64`, `STRING`, `DATE`, `DATETIME`, `TIMESTAMP`, `BYTE` or Arrays. Literals that are structs aren't supported.
- SQL function pushdowns are applied only for functions that are supported by both BigQuery and Spanner.

### Supported functions by data source

The following are supported SQL functions by data source. No functions are
supported for SAP Datasphere.

#### Cloud SQL MySQL

- **Logical operators:** `AND`, `OR`, `NOT`.
- **Comparison operators:** `=`, `>`, `>=`, `<`, `<=`, `<>`, `IN`, `BETWEEN`, `IS NULL`.
- **Arithmetic operators:** `+`, `-`, `*` (only for `INT64` and `FLOAT64`).

#### Cloud SQL PostgreSQL and AlloyDB

- **Logical operators:** `AND`, `OR`, `NOT`.
- **Comparison operators:** `=`, `>`, `>=`, `<`, `<=`, `<>`, `IN`, `BETWEEN`, `IS NULL`.
- **Arithmetic operators:** `+`, `-`, `*`, `/` (only for `INT64`, `FLOAT64`, and `DATE` types, except for `DATE` subtraction).

#### Spanner - PostgreSQL dialect

- **Logical operators:** `AND`, `OR`, `NOT`.
- **Comparison operators:** `=`, `>`, `>=`, `<`, `<=`, `<>`, `IN`, `BETWEEN`, `IS NULL`.
- **Arithmetic operators:** `+`, `-`, `*`, `/` (only for `INT64`, `FLOAT64`, `NUMERIC`).
- When using [external datasets](https://docs.cloud.google.com/bigquery/docs/spanner-external-datasets), additionally:

  - **Compute** pushdown
  - **Partial Aggregate** pushdown
  - **String** functions
  - **Math** functions
  - **Cast** functions
  - **Array** functions
- Expect GoogleSQL semantics, not
  PostgreSQL semantics, when queries are run. For
  example:

  - `NULL` values sort first in ascending order by default, unlike PostgreSQL where they sort last by default.
  - PostgreSQL `NUMERIC` values read from
    Spanner are handled in accordance with the
    [Spanner to BigQuery
    type mapping](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/federated_query_functions#spanner-mapping).
    For example, if a numeric column has the `1.1234567891`
    value, then the following query returns 0 rows:

    ```googlesql
    SELECT * FROM EXTERNAL_QUERY("CONNECTION_ID", "SELECT * from
    operations_table where numeric_col = 1.123456789")
    ```
    However the following statement returns 1 row based on GoogleSQL semantics:

    ```googlesql
    SELECT * from operations_table where numeric_col = 1.123456789
    ```

    <br />

  - JSON object normalization behaves differently. Keys are
    sorted strictly lexicographically in Spanner
    `JSON`, but in PostgreSQL `PG JSONB`, they are
    sorted first by key length, then lexicographically with
    equivalent key length.

#### Spanner - GoogleSQL dialect

- **Logical operators:** `AND`, `OR`, `NOT`.
- **Comparison operators:** `=`, `>`, `>=`, `<`, `<=`, `<>`, `IN`, `BETWEEN`, `IS NULL`.
- **Arithmetic operators:** `+`, `-`, `*`, `/` (only for `INT64`, `FLOAT64`, `NUMERIC`).
- **Safe arithmetic operators:** `SAFE_ADD`, `SAFE_SUBTRACT`, `SAFE_MULTIPLY`, `SAFE_DIVIDE` (only for `INT64`, `FLOAT64`, `NUMERIC`).
- When using [external datasets](https://docs.cloud.google.com/bigquery/docs/spanner-external-datasets), additionally:
  - **Compute** pushdown,
  - **Partial Aggregate** pushdown,
  - **String** functions,
  - **Math** functions,
  - **Cast** functions,
  - **Array** functions.

## Work with collations in external data sources

An external data source might have a collation set on a column
(for example, case-insensitivity). When you execute a federated query, the
remote database takes into account the configured collation.

Consider the following example where you have a `flag` column with a
case-insensitive collation in the external data source:

```googlesql
SELECT * FROM EXTERNAL_QUERY("CONNECTION_ID", "select * from operations_table where flag = 'Y'")
```

Replace `CONNECTION_ID` with the ID of the
BigQuery connection.

The preceding query returns rows where `flag` is `y` or `Y` because the query
is executed on the external data source.

However, for query federation with Cloud SQL, SAP Datasphere,
or AlloyDB data sources, if you add a filter on your main query,
the query is executed on the BigQuery side with the default
collation. See the following query:

```googlesql
SELECT * FROM
  (
    SELECT * FROM EXTERNAL_QUERY("CONNECTION_ID", "select * from operations_table")
  )
WHERE flag = 'Y'
```

Due to the default case-sensitive [collation](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/collation-concepts)
in BigQuery, the preceding query only returns rows where the flag
is `Y` and filters out rows where the flag is `y`. To make your `WHERE` clause
case-insensitive, specify the collation in the query:

```googlesql
SELECT * FROM
  (
    SELECT * FROM EXTERNAL_QUERY("CONNECTION_ID", "select * from operations_table")
  )
WHERE COLLATE(flag, 'und:ci') = 'Y'
```

## What's next

- Learn how to [query Spanner data](https://docs.cloud.google.com/bigquery/docs/spanner-federated-queries).
- Learn how to [create Spanner external datasets](https://docs.cloud.google.com/bigquery/docs/spanner-external-datasets).
- Learn how to [query Cloud SQL data](https://docs.cloud.google.com/bigquery/docs/cloud-sql-federated-queries).
- Learn how to [query AlloyDB data](https://docs.cloud.google.com/bigquery/docs/alloydb-federated-queries).
- Learn how to [query SAP Datasphere data](https://docs.cloud.google.com/bigquery/docs/sap-datasphere-federated-queries)