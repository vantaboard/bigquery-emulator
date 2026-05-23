# Export data to AlloyDB (reverse ETL)

> [!WARNING]
>
> **Preview**
>
>
> This feature is
>
> subject to the "Pre-GA Offerings Terms" in the General Service Terms section of the
> [Service Specific
> Terms](https://docs.cloud.google.com/terms/service-terms#1).
>
> Pre-GA features are available "as is" and might have limited support.
>
> For more information, see the
> [launch stage descriptions](https://cloud.google.com/products/#product-launch-stages).

> [!NOTE]
> **Note:** To request access to this preview feature, complete the [BigQuery to AlloyDB Batch Exports - Preview
> Sign-Up](https://forms.gle/nTbRPvmMQLDGRsYr8) interest form. To provide feedback or request support for this feature, send email to [bq-alloydb-export-feedback@google.com](mailto:bq-alloydb-export-feedback@google.com).

This document describes how you can set up a reverse extract, transform, and
load (reverse ETL) workflow from BigQuery to AlloyDB for PostgreSQL. You can
do this by using the
[`EXPORT DATA` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/export-statements).

## Before you begin

- Create an [AlloyDB cluster and
  instance](https://docs.cloud.google.com/alloydb/docs/cluster-create), which includes a database, schema,
  and table, to receive the exported data. You must have the target schema and
  table before you run the export job.

- The target AlloyDB instance must be a `PRIMARY` instance in
  `READY` state.

- [Create a BigQuery
  connection](https://docs.cloud.google.com/bigquery/docs/connect-to-alloydb) to connect to your
  AlloyDB instance. The connection must be authenticated by
  username and password. The database user specified in the connection must have
  `INSERT` privileges on the target table and `USAGE` on the target schema.

- Create a [BigQuery Enterprise or Enterprise Plus tier reservation](https://docs.cloud.google.com/bigquery/docs/reservations-tasks#create_reservations).

- Grant [Identity and Access Management (IAM) roles](https://docs.cloud.google.com/bigquery/docs/export-to-alloydb#required_roles) that give users the
  necessary permissions to perform each task in this document.

### Required roles


To get the permissions that
you need to export BigQuery data to AlloyDB,

ask your administrator to grant you the
following IAM roles on your project:

- Export data from a BigQuery table: [BigQuery Data Viewer](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquery#bigquery.dataViewer) (`roles/bigquery.dataViewer`)
- Run an extract job: [BigQuery User](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquery#bigquery.user) (`roles/bigquery.user`)
- Use a BigQuery connection: [BigQuery Connection User](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquery#bigquery.connectionUser) (`roles/bigquery.connectionUser`)
- Connect to an AlloyDB instance: AlloyDB Client (`roles/alloydb.client`) - the connection service account


For more information about granting roles, see [Manage access to projects, folders, and organizations](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).


You might also be able to get
the required permissions through [custom
roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles) or other [predefined
roles](https://docs.cloud.google.com/iam/docs/roles-overview#predefined).

## Limitations

- AlloyDB exports only support batch exports. You can't use
  continuous queries to export to AlloyDB.

- Data exported to AlloyDB is added exclusively as new rows. The
  export process doesn't modify or delete existing records within
  AlloyDB. If the target table doesn't have `PRIMARY KEY` or
  `UNIQUE` constraints defined, exporting the same data multiple times will
  result in duplicate entries.

- To ensure data integrity, we recommend defining `PRIMARY KEY` or `UNIQUE`
  constraints on your AlloyDB tables. Export jobs don't perform
  "upserts", where an existing record is updated if there is a unique matching
  key. If any incoming row violates a `PRIMARY KEY` or `UNIQUE` constraint, the
  entire export job will fail.

- We don't recommend running multiple concurrent `EXPORT DATA` jobs to the same
  AlloyDB table. Doing so can result in unpredictable behavior,
  like data loss or job failures. We recommend verifying that
  only one export job writes to a specific table at a time.

- Only username and password authentication through a
  BigQuery connection is supported.

- `ARRAY`, `BYTES`, `GEOGRAPHY`, `INTERVAL`, and `STRUCT` BigQuery
  data types are not supported.

- If the BigQuery `SELECT` statement omits
  columns that exist in the target AlloyDB table, those columns
  must either allow `NULL` values or have default values defined in
  AlloyDB. If they have a `NOT NULL` constraint and no
  default value, the export will fail.

- Exports to AlloyDB are only supported for the
  BigQuery Enterprise or Enterprise Plus
  editions. The BigQuery Standard edition and on-demand
  compute are not supported. For more information, see [Administration
  features](https://docs.cloud.google.com/bigquery/docs/editions-intro#administration_features).

- A BigQuery job, such as an extract job to
  AlloyDB, has a maximum duration of 6 hours. For very large
  exports, we recommend breaking down the export into multiple smaller jobs.

## Location considerations

Exporting data to AlloyDB has specific requirements regarding the
location of your BigQuery dataset and your AlloyDB
instance:

- **Same-region exports:** The target AlloyDB instance must
  reside in the exact same Google Cloud region as the BigQuery
  dataset. For example, a dataset in `us-east1` can only be exported to an
  AlloyDB instance in `us-east1`.

- **Multi-region exports:**

  - Datasets in the `US` multi-region can only be exported to an AlloyDB instance located in the `us-central1` region.
  - Datasets in the `EU` multi-region can only be exported to an AlloyDB instance located in the `europe-west4` region.

Cross-region exports other than the previously mentioned combinations aren't
supported.

## Configure exports with `alloydb_options`

You can use the `alloydb_options` option to specify the destination
AlloyDB schema, table, and maximum connections. The configuration
is expressed as a JSON string. Only the `table` parameter is required;
all other parameters are optional.

When configuring the export, the columns in the `SELECT` statement must have
aliases that match the names of the columns in the target
AlloyDB table.

    EXPORT DATA
      WITH CONNECTION ``PROJECT_ID`.`LOCATION`.`CONNECTION_ID``
      OPTIONS(
        format='ALLOYDB',
        uri="https://alloydb.googleapis.com/v1/projects/`PROJECT_ID`/locations/`LOCATION`/clusters/`CLUSTER_ID`/instances/`INSTANCE_ID`",
        alloydb_options="""{
          "schema": "SCHEMA_NAME",
          "table": "TABLE_NAME",
          "max_parallel_connections": `MAX_CONNECTIONS`
        }"""
      )
    AS SELECT * FROM `mydataset.table1`;

Replace the following:

- `PROJECT_ID`: the name of your Google Cloud project.
- `LOCATION`: the location of your connection and target instance.
- `CONNECTION_ID`: the name of your BigQuery connection.
- `CLUSTER_ID`: the name of your AlloyDB cluster.
- `INSTANCE_ID`: the name of your target AlloyDB instance.
- `SCHEMA_NAME` (Optional): the name of the destination schema in AlloyDB. If not provided, the default schema configured for the database user is used.
- `TABLE_NAME`: the name of an existing destination table in AlloyDB, without the schema prefix.
- `MAX_CONNECTIONS` (Optional): the maximum number of concurrent parallel connections from BigQuery workers to the AlloyDB instance. Limiting connections can prevent overloading the target instance during large exports.

## Export data

You can use the
[`EXPORT DATA` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/export-statements)
to export data from a BigQuery table into an
AlloyDB table.

The following example exports selected fields from a table that's named
`mydataset.table1` into an AlloyDB table named `my_target_table`:

```googlesql
EXPORT DATA
  WITH CONNECTION `myproject.us-central1.my-alloydb-conn`
  OPTIONS (
    format='ALLOYDB',
    uri="https://alloydb.googleapis.com/v1/projects/myproject/locations/us-central1/clusters/my-cluster/instances/my-instance",
    alloydb_options="""{
      "schema": "public",
      "table": "my_target_table"
    }"""
  )
AS SELECT
  col1 AS id,
  col2 AS name,
  col3 AS value
FROM
  `mydataset.table1`;
```

## Pricing

When you export data to AlloyDB using the `EXPORT DATA` statement,
you are billed using
[BigQuery capacity compute pricing](https://cloud.google.com/bigquery/pricing#capacity_compute_analysis_pricing).

After the data is exported, you're charged for storing the data in
AlloyDB. For more information,
see [AlloyDB for PostgreSQL pricing](https://cloud.google.com/alloydb/pricing).