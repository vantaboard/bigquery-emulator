# Create materialized views

> [!NOTE]
> **Note:** This feature may not be available when using reservations that are created with certain BigQuery editions. For more information about which features are enabled in each edition, see [Introduction to
> BigQuery editions](https://docs.cloud.google.com/bigquery/docs/editions-intro).

This document describes how to create materialized views in
BigQuery. Before you read this document, familiarize yourself
with [Introduction to materialized views](https://docs.cloud.google.com/bigquery/docs/materialized-views-intro).

## Before you begin

Grant Identity and Access Management (IAM) roles that give users the necessary permissions
to perform each task in this document.

### Required permissions

To create materialized views, you need the `bigquery.tables.create`
IAM permission.

Each of the following predefined IAM roles includes the
permissions that you need in order to create a materialized view:

- `bigquery.dataEditor`
- `bigquery.dataOwner`
- `bigquery.admin`

For more information about
BigQuery Identity and Access Management (IAM), see
[Access control with IAM](https://docs.cloud.google.com/bigquery/docs/access-control).

## Create materialized views

To create a materialized view, select one of the following options:

### SQL

Use the
[`CREATE MATERIALIZED VIEW` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_materialized_view_statement).
The following example creates a materialized view for the number of clicks
for each product ID:

<br />

<br />

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, enter the following statement:

   ```googlesql
   CREATE MATERIALIZED VIEW PROJECT_ID.DATASET.MATERIALIZED_VIEW_NAME AS (
     QUERY_EXPRESSION
   );
   ```


   Replace the following:
   - `PROJECT_ID`: the name of your project in which you want to create the materialized view---for example, `myproject`.
   - `DATASET`: the name of the BigQuery dataset that you want to create the materialized view in---for example, `mydataset`. If you are creating a materialized view over an Amazon Simple Storage Service (Amazon S3) BigLake table ([preview](https://cloud.google.com/products#product-launch-stages)), make sure the dataset is in a [supported region](https://docs.cloud.google.com/bigquery/docs/omni-introduction#locations).
   - `MATERIALIZED_VIEW_NAME`: the name of the materialized view that you want to create---for example, `my_mv`.
   - `QUERY_EXPRESSION`: the GoogleSQL query expression that defines the materialized view---for example, `SELECT product_id, SUM(clicks) AS sum_clicks FROM
     mydataset.my_source_table`.

   <br />

3. Click **Run**.

   <br />

For more information about how to run queries, see [Run an interactive query](https://docs.cloud.google.com/bigquery/docs/running-queries#queries).

**Example**

The following example creates a materialized view for the number of clicks
for each product ID:

```googlesql
CREATE MATERIALIZED VIEW myproject.mydataset.my_mv_table AS (
  SELECT
    product_id,
    SUM(clicks) AS sum_clicks
  FROM
    myproject.mydataset.my_base_table
  GROUP BY
    product_id
);
```

### Terraform

Use the
[`google_bigquery_table`](https://registry.terraform.io/providers/hashicorp/google/latest/docs/resources/bigquery_table)
resource.

> [!NOTE]
> **Note:** To create BigQuery objects using Terraform, you must enable the [Cloud Resource Manager API](https://docs.cloud.google.com/resource-manager/reference/rest).

To authenticate to BigQuery, set up Application Default
Credentials. For more information, see
[Set up authentication for client libraries](https://docs.cloud.google.com/bigquery/docs/authentication#client-libs).

The following example creates a view named `my_materialized_view`:

    resource "google_bigquery_dataset" "default" {
      dataset_id                      = "mydataset"
      default_partition_expiration_ms = 2592000000  # 30 days
      default_table_expiration_ms     = 31536000000 # 365 days
      description                     = "dataset description"
      location                        = "US"
      max_time_travel_hours           = 96 # 4 days

      labels = {
        billing_group = "accounting",
        pii           = "sensitive"
      }
    }

    resource "google_bigquery_table" "default" {
      dataset_id = google_bigquery_dataset.default.dataset_id
      table_id   = "my_materialized_view"

      materialized_view {
        query                            = "SELECT ID, description, date_created FROM `myproject.orders.items`"
        enable_refresh                   = "true"
        refresh_interval_ms              = 172800000 # 2 days
        allow_non_incremental_definition = "false"
      }

    }

To apply your Terraform configuration in a Google Cloud project, complete the steps in the
following sections.

## Prepare Cloud Shell

1. Launch [Cloud Shell](https://shell.cloud.google.com/).
2. Set the default Google Cloud project
   where you want to apply your Terraform configurations.

   You only need to run this command once per project, and you can run it in any directory.

   ```
   export GOOGLE_CLOUD_PROJECT=PROJECT_ID
   ```

   Environment variables are overridden if you set explicit values in the Terraform
   configuration file.

## Prepare the directory

Each Terraform configuration file must have its own directory (also
called a *root module*).

1. In [Cloud Shell](https://shell.cloud.google.com/), create a directory and a new file within that directory. The filename must have the `.tf` extension---for example `main.tf`. In this tutorial, the file is referred to as `main.tf`.

   ```
   mkdir DIRECTORY && cd DIRECTORY && touch main.tf
   ```
2. If you are following a tutorial, you can copy the sample code in each section or step.

   Copy the sample code into the newly created `main.tf`.

   Optionally, copy the code from GitHub. This is recommended
   when the Terraform snippet is part of an end-to-end solution.
3. Review and modify the sample parameters to apply to your environment.
4. Save your changes.
5. Initialize Terraform. You only need to do this once per directory.

   ```
   terraform init
   ```

   Optionally, to use the latest Google provider version, include the `-upgrade`
   option:

   ```
   terraform init -upgrade
   ```

## Apply the changes

1. Review the configuration and verify that the resources that Terraform is going to create or update match your expectations:

   ```
   terraform plan
   ```

   Make corrections to the configuration as necessary.
2. Apply the Terraform configuration by running the following command and entering `yes` at the prompt:

   ```
   terraform apply
   ```

   Wait until Terraform displays the "Apply complete!" message.
3. [Open your Google Cloud project](https://console.cloud.google.com/) to view the results. In the Google Cloud console, navigate to your resources in the UI to make sure that Terraform has created or updated them.

> [!NOTE]
> **Note:** Terraform samples typically assume that the required APIs are enabled in your Google Cloud project.

### API

Call the [`tables.insert` method](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables/insert)
and pass in a
[`Table` resource](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables#Table)
with a defined `materializedView` field:

```googlesql
{
  "kind": "bigquery#table",
  "tableReference": {
    "projectId": "PROJECT_ID",
    "datasetId": "DATASET",
    "tableId": "MATERIALIZED_VIEW_NAME"
  },
  "materializedView": {
    "query": "QUERY_EXPRESSION"
  }
}
```

Replace the following:

- `PROJECT_ID`: the name of your project in which you want to create the materialized view---for example, `myproject`.
- `DATASET`: the name of the BigQuery dataset that you want to create the materialized view in---for example, `mydataset`. If you are creating a materialized view over an Amazon Simple Storage Service (Amazon S3) BigLake table ([preview](https://cloud.google.com/products#product-launch-stages)), make sure the dataset is in a [supported region](https://docs.cloud.google.com/bigquery/docs/omni-introduction#locations).
- `MATERIALIZED_VIEW_NAME`: the name of the materialized view that you want to create---for example, `my_mv`.
- `QUERY_EXPRESSION`: the GoogleSQL query expression that defines the materialized view---for example, `SELECT product_id, SUM(clicks) AS sum_clicks FROM
  mydataset.my_source_table`.

<br />

**Example**

The following example creates a materialized view for the number of clicks
for each product ID:

```googlesql
{
  "kind": "bigquery#table",
  "tableReference": {
    "projectId": "myproject",
    "datasetId": "mydataset",
    "tableId": "my_mv"
  },
  "materializedView": {
    "query": "select product_id,sum(clicks) as
                sum_clicks from myproject.mydataset.my_source_table
                group by 1"
  }
}
```

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

    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryException.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryOptions.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.MaterializedViewDefinition.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableId.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableInfo.html;

    // Sample to create materialized view
    public class CreateMaterializedView {

      public static void main(String[] args) {
        // TODO(developer): Replace these variables before running the sample.
        String datasetName = "MY_DATASET_NAME";
        String tableName = "MY_TABLE_NAME";
        String materializedViewName = "MY_MATERIALIZED_VIEW_NAME";
        String query =
            String.format(
                "SELECT MAX(TimestampField) AS TimestampField, StringField, "
                    + "MAX(BooleanField) AS BooleanField "
                    + "FROM %s.%s GROUP BY StringField",
                datasetName, tableName);
        createMaterializedView(datasetName, materializedViewName, query);
      }

      public static void createMaterializedView(
          String datasetName, String materializedViewName, String query) {
        try {
          // Initialize client that will be used to send requests. This client only needs to be created
          // once, and can be reused for multiple requests.
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html bigquery = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryOptions.html.getDefaultInstance().getService();

          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableId.html tableId = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableId.html.of(datasetName, materializedViewName);

          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.MaterializedViewDefinition.html materializedViewDefinition =
              https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.MaterializedViewDefinition.html.newBuilder(query).build();

          bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html#com_google_cloud_bigquery_BigQuery_create_com_google_cloud_bigquery_DatasetInfo_com_google_cloud_bigquery_BigQuery_DatasetOption____(TableInfo.of(tableId, materializedViewDefinition));
          System.out.println("Materialized view created successfully");
        } catch (https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryException.html e) {
          System.out.println("Materialized view was not created. \n" + e.toString());
        }
      }
    }

After the materialized view is successfully created, it appears
in the **Explorer** pane of BigQuery in the Google Cloud console.
The following example shows a materialized view schema:

![Materialized view schema in Google Cloud console](https://docs.cloud.google.com/static/bigquery/images/mv_schema_ui.png)

Unless you disable [automatic refresh](https://docs.cloud.google.com/bigquery/docs/materialized-views-manage#automatic-refresh),
BigQuery starts an asynchronous full refresh for the materialized
view. The query finishes quickly, but the initial refresh might continue to
run.

> [!NOTE]
> **Note:** Each base table is limited to 100 materialized views within the same project, and 500 materialized views within the same organization.

## Access control

You can grant access to a materialized view at the [dataset level](https://docs.cloud.google.com/bigquery/docs/dataset-access-controls), the
[view level](https://docs.cloud.google.com/bigquery/docs/authorized-views), or the [column level](https://docs.cloud.google.com/bigquery/docs/column-level-security-intro). You can also set access at a
higher level in the [IAM resource hierarchy](https://docs.cloud.google.com/iam/docs/resource-hierarchy-access-control).

Querying a materialized view requires access to the view as well as its base
tables. To share a materialized view, you can grant permissions to the
base tables or configure a materialized view as an authorized view. For more
information, see [Authorized views](https://docs.cloud.google.com/bigquery/docs/authorized-views).

### Access control restrictions

- If a user's query of a materialized view includes base table columns that they
  can't access due to column-level security, then the query fails with the
  message `Access Denied`.

- If a user queries a materialized view but doesn't have full access to all rows
  in the materialized view's base tables, then BigQuery runs the
  query against the base tables instead of reading materialized view data. This
  ensures the query respects all access control constraints. This limitation
  also applies when querying tables with data-masked columns.

## Materialized views query support

Materialized views use a restricted SQL syntax.
Queries must use the following pattern:

```googlesql
[ WITH cte [, ...]]
SELECT  [{ ALL | DISTINCT }]
  expression [ [ AS ] alias ] [, ...]
FROM from_item [, ...]
[ WHERE bool_expression ]
[ GROUP BY expression [, ...] ]

from_item:
    {
      table_name [ as_alias ]
      | { join_operation | ( join_operation ) }
      | field_path
      | unnest_operator
      | cte_name [ as_alias ]
    }

as_alias:
    [ AS ] alias
```

## Query limitations

Incremental materialized views have the following limitations.

### Aggregate requirements

Aggregates in the materialized view query must be outputs. Computing, filtering,
or joining based on an aggregated value is not supported. For example, creating
a view from the following query is not supported because it produces a value
computed from an aggregate, `COUNT(*) / 10 as cnt`.

```googlesql
SELECT TIMESTAMP_TRUNC(ts, HOUR) AS ts_hour, COUNT(*) / 10 AS cnt
FROM mydataset.mytable
GROUP BY ts_hour;
```

Only the following aggregation functions are supported:

- `ANY_VALUE` (but not over `STRUCT`)
- `APPROX_COUNT_DISTINCT`
- `ARRAY_AGG` (but not over `ARRAY` or `STRUCT`)
- `AVG`
- `BIT_AND`
- `BIT_OR`
- `BIT_XOR`
- `COUNT`
- `COUNTIF`
- `HLL_COUNT.INIT`
- `LOGICAL_AND`
- `LOGICAL_OR`
- `MAX`
- `MIN`
- `MAX_BY` (but not over `STRUCT`)
- `MIN_BY` (but not over `STRUCT`)
- `SUM`

### Unsupported SQL features

The following SQL features are not supported in materialized views:

- `UNION ALL`. ([Support in Preview](https://docs.cloud.google.com/bigquery/docs/materialized-views-create#left-union))
- `LEFT OUTER JOIN` ([Support in Preview](https://docs.cloud.google.com/bigquery/docs/materialized-views-create#left-union))
- `RIGHT/FULL OUTER JOIN`.
- Self-joins, also known as using a `JOIN` on the same table more than once.
- [Window functions](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/window-function-calls).
- `ARRAY` subqueries.
- Non-deterministic functions such as `RAND()`, `CURRENT_DATE()`, `SESSION_USER()`, or `CURRENT_TIME()`.
- [User-defined functions (UDFs)](https://docs.cloud.google.com/bigquery/docs/user-defined-functions).
- `TABLESAMPLE`.
- `FOR SYSTEM_TIME AS OF`.
- [Generative AI functions](https://docs.cloud.google.com/bigquery/docs/generative-ai-overview).

#### `LEFT OUTER JOIN` and `UNION ALL` support

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

To request feedback or support for this feature, send an email to
[bq-mv-help@google.com](mailto:bq-mv-help@google.com).

Incremental materialized views support `LEFT OUTER JOIN` and `UNION ALL`.
Materialized views with `LEFT OUTER JOIN` and `UNION ALL` statements share the
limitations of other incremental materialized views. In addition, [smart
tuning](https://docs.cloud.google.com/bigquery/docs/materialized-views-use#smart_tuning) is not supported for
materialized views with union all or left outer join.

##### Examples

The following example creates an aggregate incremental materialized view with
a `LEFT JOIN`. This view is incrementally updated when data appends to the left
table.

```googlesql
CREATE MATERIALIZED VIEW dataset.mv
AS (
  SELECT
    s_store_sk,
    s_country,
    s_zip,
    SUM(ss_net_paid) AS sum_sales,
  FROM dataset.store_sales
  LEFT JOIN dataset.store
    ON ss_store_sk = s_store_sk
  GROUP BY 1, 2, 3
);
```

The following example creates an aggregate incremental materialized view with
a `UNION ALL`. This view is incrementally updated when data appends to either or
both tables. For more information about incremental updates, see
[Incremental Updates](https://docs.cloud.google.com/bigquery/docs/materialized-views-use#incremental_updates).

```googlesql
CREATE MATERIALIZED VIEW dataset.mv PARTITION BY DATE(ts_hour)
AS (
  SELECT
    SELECT TIMESTAMP_TRUNC(ts, HOUR) AS ts_hour, SUM(sales) sum_sales
  FROM
    (SELECT ts, sales from dataset.table1 UNION ALL
     SELECT ts, sales from dataset.table2)
  GROUP BY 1
);
```

### `WITH` clause and common table expressions (CTEs)

Materialized views support `WITH` clauses and common table expressions.
Materialized views with `WITH` clauses must still follow the pattern and
limitations of materialized views without `WITH` clauses.

#### Examples

The following example shows a materialized view using a `WITH` clause:

```googlesql
WITH tmp AS (
  SELECT TIMESTAMP_TRUNC(ts, HOUR) AS ts_hour, *
  FROM mydataset.mytable
)
SELECT ts_hour, COUNT(*) AS cnt
FROM tmp
GROUP BY ts_hour;
```

The following example shows a materialized view using a `WITH` clause that is
not supported because it contains two `GROUP BY` clauses:

```googlesql
WITH tmp AS (
  SELECT city, COUNT(*) AS population
  FROM mydataset.mytable
  GROUP BY city
)
SELECT population, COUNT(*) AS cnt
GROUP BY population;
```

### Materialized views over BigLake tables

To create [materialized views over BigLake
tables](https://docs.cloud.google.com/bigquery/docs/materialized-views-intro#biglake), the
BigLake table must have [metadata caching
enabled](https://docs.cloud.google.com/bigquery/docs/biglake-intro#metadata_caching_for_performance) over
Cloud Storage data and the materialized view must have a
[`max_staleness`](https://docs.cloud.google.com/bigquery/docs/materialized-views-create#max_staleness) option value greater than the base table.
Materialized views over BigLake tables support the [same set of
queries](https://docs.cloud.google.com/bigquery/docs/materialized-views-create#query_limitations) as other
materialized views.

#### Example

Creation of an aggregate view using a BigLake base table:

```googlesql
CREATE MATERIALIZED VIEW sample_dataset.sample_mv
    OPTIONS (max_staleness=INTERVAL "0:30:0" HOUR TO SECOND)
AS SELECT COUNT(*) cnt
FROM dataset.biglake_base_table;
```

For details about the limitations of materialized views over
BigLake tables, see [materialized views over
BigLake tables](https://docs.cloud.google.com/bigquery/docs/materialized-views-intro#biglake).

### Materialized views over Apache Iceberg external tables

You can reference large Iceberg tables in materialized
views instead of migrating that data to BigQuery-managed storage.

#### Create a materialized view over an Iceberg table

The following example creates a partition-aligned materialized view over a
partitioned base Iceberg table:

```googlesql
CREATE MATERIALIZED VIEW mydataset.myicebergmv
  PARTITION BY DATE_TRUNC(birth_month, MONTH)
AS
  SELECT * FROM mydataset.myicebergtable;
```

The underlying base Iceberg table `myicebergtable` must
have a [partition spec](https://iceberg.apache.org/spec/#partition-specs)
like the following:

      "partition-specs" : [ {
        "spec-id" : 0,
        "fields" : [ {
        "name" : "birth_month",
        "transform" : "month",
        "source-id" : 3,
        "field-id" : 1000
        } ]
      } ]

#### Limitations

In addition to the [limitations](https://docs.cloud.google.com/bigquery/docs/iceberg-external-tables#limitations)
of standard Iceberg tables, materialized views over
Iceberg tables have the following limitations:

- You can create a materialized view that is partition aligned with the base table. However, the materialized view only supports time-based [partition transformation](https://iceberg.apache.org/spec/#partition-transforms), for example, `YEAR`, `MONTH`, `DAY`, and `HOUR`.
- The granularity of the materialized view's partition cannot be finer than the granularity of the base table's partition. For example, if you partition the base table yearly using the `birth_date` column, creating a materialized view with `PARTITION BY DATE_TRUNC(birth_date, MONTH)` isn't supported.
- If the base Iceberg tables have changes across more than 4000 partitions, the materialized view is fully invalidated upon refresh, even if it's partitioned.
- [Partition evolutions](https://iceberg.apache.org/spec/#partition-evolution) are supported. However, changing the partitioning columns of a base table without recreating the materialized view might result in full invalidation that cannot be fixed by refreshing the materialized view.
- There must be at least one snapshot in the base table.
- The Iceberg table must be a BigLake table, for example, an authorized external table.
- The query over the materialized view might fail if the `metadata.json` file of your Iceberg table is corrupted.
- If [VPC Service Controls](https://docs.cloud.google.com/vpc-service-controls/docs/overview) is enabled, service accounts of the authorized external table must be added to your ingress rules, otherwise, VPC Service Controls blocks automatic background refresh for the materialized view.

The `metadata.json` file of your Iceberg table must have
the following specifications. Without these specifications, your queries scan
the base table, failing to use the materialized result.

- In [table metadata](https://iceberg.apache.org/spec/#table-metadata):

  - `current-snapshot-id`
  - `current-schema-id`
  - `snapshots`
  - `snapshot-log`
- In [snapshots](https://iceberg.apache.org/spec/#snapshots):

  - `parent-snapshot-id` (if available)
  - `schema-id`
  - `operation` (in the `summary` field)
- [Partitioning](https://iceberg.apache.org/spec/#partitioning) (for the
  partitioned materialized view)

## Partitioned materialized views

Materialized views on partitioned tables can be partitioned. Partitioning a
materialized view is similar to partitioning a normal table, in that it provides
benefit when queries often access a subset of the partitions. In addition,
partitioning a materialized view can improve the view's behavior when data in
the base table or tables is modified or deleted. For more information, see
[Partition alignment](https://docs.cloud.google.com/bigquery/docs/materialized-views-use#partition_alignment).

If the base table is partitioned, then you can partition a materialized view on
the same partitioning column. For time-based partitions, the granularity must
match (hourly, daily, monthly, or yearly). For integer-range partitions, the
range specification must exactly match. You cannot partition a materialized view
over a non-partitioned base table.

If the base table is partitioned by ingestion time, then a materialized view can
group by the `_PARTITIONDATE` column of the base table, and also partition by it.
If you don't explicitly specify partitioning when you create the materialized
view, then the materialized view is unpartitioned.

If the base table is partitioned, consider partitioning your materialized view
as well to reduce [refresh job maintenance](https://docs.cloud.google.com/bigquery/docs/materialized-views-manage)
cost and query cost.

### Partition expiration

Partition expiration can't be set on materialized views. A materialized view
implicitly inherits the partition expiration time from the base table.
Materialized view partitions are aligned with the base table partitions, so they
expire synchronously.

> [!CAUTION]
> **Caution:** A non-partitioned materialized view based on a table with partition expiration is invalidated and must be fully refreshed when a partition expires. Therefore, you should partition the materialized view to avoid additional refresh and query cost.

#### Example 1

In this example, the base table is partitioned on the `transaction_time` column
with daily partitions. The materialized view is partitioned on the same column
and clustered on the `employee_id` column.

```googlesql
CREATE TABLE my_project.my_dataset.my_base_table(
  employee_id INT64,
  transaction_time TIMESTAMP)
  PARTITION BY DATE(transaction_time)
  OPTIONS (partition_expiration_days = 2);

CREATE MATERIALIZED VIEW my_project.my_dataset.my_mv_table
  PARTITION BY DATE(transaction_time)
  CLUSTER BY employee_id
AS (
  SELECT
    employee_id,
    transaction_time,
    COUNT(employee_id) AS cnt
  FROM
    my_dataset.my_base_table
  GROUP BY
    employee_id, transaction_time
);
```

#### Example 2

In this example, the base table is partitioned by ingestion time with daily
partitions. The materialized view selects the ingestion time as a column named
`date`. The materialized view is grouped by the `date` column and partitioned by
the same column.

```googlesql
CREATE MATERIALIZED VIEW my_project.my_dataset.my_mv_table
  PARTITION BY date
  CLUSTER BY employee_id
AS (
  SELECT
    employee_id,
    _PARTITIONDATE AS date,
    COUNT(1) AS count
  FROM
    my_dataset.my_base_table
  GROUP BY
    employee_id,
    date
);
```

#### Example 3

In this example, the base table is partitioned on a `TIMESTAMP` column named
`transaction_time`, with daily partitions. The materialized view defines a
column named `transaction_hour`, using the [`TIMESTAMP_TRUNC`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#timestamp_trunc)
function to truncate the value to the nearest hour. The materialized view is
grouped by `transaction_hour` and also partitioned by it.

Note the following:

- The truncation function that is applied to the partitioning column must be
  at least as granular as the partitioning of the base table. For example, if
  the base table uses daily partitions, the truncation function cannot use
  `MONTH` or `YEAR` granularity.

- In the materialized view's partition specification, the granularity has to
  match the base table.

```googlesql
CREATE TABLE my_project.my_dataset.my_base_table (
  employee_id INT64,
  transaction_time TIMESTAMP)
  PARTITION BY DATE(transaction_time);

CREATE MATERIALIZED VIEW my_project.my_dataset.my_mv_table
  PARTITION BY DATE(transaction_hour)
AS (
  SELECT
    employee_id,
    TIMESTAMP_TRUNC(transaction_time, HOUR) AS transaction_hour,
    COUNT(employee_id) AS cnt
  FROM
    my_dataset.my_base_table
  GROUP BY
    employee_id,
    transaction_hour
);
```

## Cluster materialized views

You can cluster materialized views by their output columns, subject to the
BigQuery [clustered table
limitations](https://docs.cloud.google.com/bigquery/docs/clustered-tables#limitations).
Aggregate output columns cannot be used as clustering columns. Adding clustering
columns to materialized views can improve the performance of queries that
include filters on those columns.

### Reference logical views

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

To request feedback or support for this feature, send email to
[bq-mv-help@google.com](mailto:bq-mv-help@google.com).

Materialized view queries can reference logical views but are subject to the
following limitations:

- [Materialized view limitations apply](https://docs.cloud.google.com/bigquery/docs/materialized-views-intro#limitations).
- If the logical view changes, then the materialized view becomes invalid and must be fully refreshed.
- [Smart tuning](https://docs.cloud.google.com/bigquery/docs/materialized-views-use#smart_tuning) is not supported.

## Best practices when creating materialized views

You should consider the following best practices when creating materialized
views.

### Which materialized views to create

When you create a materialized view, ensure your materialized view definition
reflects query patterns against the base tables. Materialized views are more
effective when they serve a broad set of queries rather than just one specific
query pattern.

For example, consider a query on a table where users often filter by the columns
`user_id` or `department`. You can group by these columns and optionally cluster
by them, instead of adding filters like `user_id = 123` into the materialized
view.

As another example, you can use deterministic date filters, either by
specific date, such as `WHERE order_date = '2019-10-01'`, or date range, such as
`WHERE order_date BETWEEN '2019-10-01' AND '2019-10-31'`. Add a date range
filter in the materialized view that covers expected date ranges in the query:

```googlesql
CREATE MATERIALIZED VIEW ...
  ...
  WHERE date > '2019-01-01'
  GROUP BY date
```

### Joins in materialized views

The following recommendations apply to materialized views with `JOIN`
statements.

#### Put the most frequently changing table first

Ensure that the largest or most frequently changing table is the first or
leftmost table referenced in the view query. Materialized views with joins
support incremental queries and refresh when the first or leftmost table in the
query is appended, but changes to other tables fully invalidate the view cache.
In star or snowflake schemas the first or leftmost table should generally be the
fact table.

#### Avoid joining on clustering keys

Materialized views with joins work best in cases where the data is heavily
aggregated or the original join query is expensive. For selective queries,
BigQuery is often already able to perform the join efficiently
and no materialized view is needed. For example consider the following
materialized view definitions.

```googlesql
CREATE MATERIALIZED VIEW dataset.mv
  CLUSTER BY s_market_id
AS (
  SELECT
    s_market_id,
    s_country,
    SUM(ss_net_paid) AS sum_sales,
    COUNT(*) AS cnt_sales
  FROM dataset.store_sales
  INNER JOIN dataset.store
    ON ss_store_sk = s_store_sk
  GROUP BY s_market_id, s_country
);
```

Suppose `store_sales` is clustered on `ss_store_sk` and you often run queries
like the following:

```googlesql
SELECT
  SUM(ss_net_paid)
FROM dataset.store_sales
INNER JOIN dataset.store
ON ss_store_sk = s_store_sk
WHERE s_country = 'Germany';
```

The materialized view might not be as efficient as the original query. For
best results, experiment with a representative set of queries, with and without
the materialized view.

## Use materialized views with `max_staleness` option

The `max_staleness` materialized view option helps you achieve consistently high
query performance with controlled costs when processing large, frequently
changing datasets. With the `max_staleness` parameter, you can reduce cost and
latency on your queries by setting an interval of time where data staleness of
query results is acceptable. This behavior can be useful for dashboards and
reports for which fully up-to-date query results aren't essential.

### Data staleness

When you query a materialized view with the `max_staleness` option set,
BigQuery returns the result based on the `max_staleness` value
and the time at which the last refresh occurred.

If the last refresh occurred within the `max_staleness` interval, then
BigQuery returns data directly from the materialized view
without reading the base tables. For example, this applies if your
`max_staleness` interval is 4 hours, and the last refresh occurred 2 hours ago.

If the last refresh occurred outside the `max_staleness` interval, then
BigQuery reads the data from the materialized view, combines it
with changes to the base table since the last refresh, and returns the combined
result. This combined result might still be stale, up to your `max_staleness`
interval. For example, this applies if your `max_staleness` interval is 4 hours,
and the last refresh occurred 7 hours ago.

### Create with `max_staleness` option

Select one of the following options:

### SQL

To create a materialized view with the `max_staleness` option, add an
`OPTIONS` clause to the DDL statement when you create the materialized view:

<br />


1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, enter the following statement:

       CREATE MATERIALIZED VIEW  project-id.my_dataset.my_mv_table
         OPTIONS (enable_refresh = true, refresh_interval_minutes = 60,
           max_staleness = INTERVAL "4:0:0" HOUR TO SECOND)
       AS SELECT
         employee_id,
         DATE(transaction_time),
         COUNT(1) AS count
       FROM my_dataset.my_base_table
       GROUP BY 1, 2;


   Replace the following:
   - <var translate="no">project-id</var> is your project ID.
   - <var translate="no">my_dataset</var> is the ID of a dataset in your project.
   - <var translate="no">my_mv_table</var> is the ID of the materialized view that you're creating.
   - <var translate="no">my_base_table</var> is the ID of a table in your dataset that serves as the base table for your materialized view.

     <br />

   - Click **Run**.

     <br />

For more information about how to run queries, see [Run an interactive query](https://docs.cloud.google.com/bigquery/docs/running-queries#queries).

### API

Call the [`tables.insert`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables/insert)
method with a defined `materializedView` resource as part of your API
request. The `materializedView` resource contains a `query` field. For
example:

```googlesql
{
  "kind": "bigquery#table",
  "tableReference": {
    "projectId": "project-id",
    "datasetId": "my_dataset",
    "tableId": "my_mv_table"
  },
  "materializedView": {
    "query": "select product_id,sum(clicks) as
                sum_clicks from project-id.my_dataset.my_base_table
                group by 1"
  }
  "maxStaleness": "4:0:0"
}
```

Replace the following:

- <var translate="no">project-id</var> is your project ID.
- <var translate="no">my_dataset</var> is the ID of a dataset in your project.
- <var translate="no">my_mv_table</var> is the ID of the materialized view that you're creating.
- <var translate="no">my_base_table</var> is the ID of a table in your dataset that serves as the base table for your materialized view.
- `product_id` is a column from the base table.
- `clicks` is a column from the base table.
- `sum_clicks` is a column in the materialized view that you are creating.

### Apply `max_staleness` option

You can apply this parameter to existing materialized views by using the `ALTER
MATERIALIZED VIEW` statement. For example:

```googlesql
ALTER MATERIALIZED VIEW project-id.my_dataset.my_mv_table
SET OPTIONS (enable_refresh = true, refresh_interval_minutes = 120,
  max_staleness = INTERVAL "8:0:0" HOUR TO SECOND);
```

### Query with `max_staleness`

You can query materialized views with the `max_staleness` option as you would
query any other materialized view, logical view, or table.

For example:

```googlesql
SELECT * FROM  project-id.my_dataset.my_mv_table
```

This query returns data from the last refresh if the data is not older than the
`max_staleness` parameter. If the materialized view has not been refreshed
within the `max_staleness` interval, BigQuery merges the results of the
latest available refresh with the base table changes to return results within
the `max_staleness` interval.

#### Data streaming and `max_staleness` results

If you stream data into the base tables of a materialized view with the
`max_staleness` option, then the query of the materialized view might exclude
records that were streamed into its tables before the beginning of the staleness
interval. As a result, a materialized view that includes data from multiple
tables and `max_staleness` option might not represent a point-in-time snapshot
of those tables.

#### Streaming data and time travel limitations

While BigQuery datasets have a default [time travel window](https://docs.cloud.google.com/bigquery/docs/time-travel) of 7 days, the **streaming storage buffer**
only retains data for 3 days. This creates the following limitation for
materialized views:

- **Query failure:** If a materialized view uses the `max_staleness` option and
  hasn't been refreshed for more than 3 days, queries against the view fail with the
  the error message `Streaming data from <materialized_view_name> is temporarily
  unavailable`.

- **Cause:** The failure occurs because the query rewrite process attempts to
  read incremental changes (deltas) from the streaming storage buffer. If the
  required data is older than the 3-day retention period, the system can't
  retrieve the deltas needed for the incremental rewrite.

To avoid these errors, ensure your [refresh
policy](https://docs.cloud.google.com/bigquery/docs/materialized-views-manage#automatic-refresh) updates the
materialized view at least once every 3 days.

### Smart tuning and the `max_staleness` option

Smart tuning automatically rewrites queries to use materialized views whenever
possible regardless of the `max_staleness` option, even if the query does not
reference a materialized view. The `max_staleness` option on a materialized view
does not affect the results of the rewritten query. The `max_staleness` option
only affects queries that directly query the materialized view.

### Manage staleness and refresh frequency

You should set `max_staleness` based on your requirements. To avoid reading
data from base tables, configure the refresh interval so that the refresh takes
place within the staleness interval. You can account for the average refresh
runtime plus a margin for growth.

For example, if one hour is required to refresh your materialized view and you
want a one-hour buffer for growth, then you should set the refresh interval to
two hours. This configuration ensures that the refresh occurs within your
report's four-hour maximum for staleness.

```bash
CREATE MATERIALIZED VIEW project-id.my_dataset.my_mv_table
OPTIONS (enable_refresh = true, refresh_interval_minutes = 120, max_staleness =
INTERVAL "4:0:0" HOUR TO SECOND)
AS SELECT
  employee_id,
  DATE(transaction_time),
  COUNT(1) AS cnt
FROM my_dataset.my_base_table
GROUP BY 1, 2;
```

## Non-incremental materialized views

Non-incremental materialized views support most SQL queries, including `OUTER
JOIN`, `UNION`, and `HAVING` clauses, and analytic functions. To determine
whether a materialized view was used in your query, check the cost estimates
by using a [dry run](https://docs.cloud.google.com/bigquery/docs/materialized-views-intro#interaction).
In scenarios where
data staleness is acceptable, for example for batch data processing or
reporting, non-incremental materialized views can improve query performance and
reduce cost. By using the `max_staleness` option, you can build arbitrary,
complex materialized views that are automatically maintained and have built-in
staleness guarantees.

### Use non-incremental materialized views

You can create non-incremental materialized views by using the
`allow_non_incremental_definition` option. This option must be accompanied by
the `max_staleness` option. To ensure a periodic refresh of the materialized
view, you should also configure a [refresh
policy](https://docs.cloud.google.com/bigquery/docs/materialized-views-manage#enable_and_disable_automatic_refresh).
Without a refresh policy, you must manually refresh the materialized view.

The materialized view always represents the state of the base tables within the
`max_staleness` interval. If the last refresh is too stale and doesn't represent
the base tables within the `max_staleness` interval, then the query reads the
base tables. To learn more about possible performance implications, see [Data
staleness](https://docs.cloud.google.com/bigquery/docs/materialized-views-create#data_staleness).

### Create with `allow_non_incremental_definition`

To create a materialized view with the `allow_non_incremental_definition`
option, follow these steps. After you create the materialized view, you cannot
modify the `allow_non_incremental_definition` option. For example, you cannot
change the value `true` to `false`, or remove the
`allow_non_incremental_definition` option from the materialized view.

### SQL

Add an `OPTIONS` clause to the DDL statement when you create the
materialized view:

<br />

<br />

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, enter the following statement:

       CREATE MATERIALIZED VIEW my_project.my_dataset.my_mv_table
       OPTIONS (
         enable_refresh = true, refresh_interval_minutes = 60,
         max_staleness = INTERVAL "4" HOUR,
           allow_non_incremental_definition = true)
       AS SELECT
         s_store_sk,
         SUM(ss_net_paid) AS sum_sales,
         APPROX_QUANTILES(ss_net_paid, 2)[safe_offset(1)] median
       FROM my_project.my_dataset.store
       LEFT OUTER JOIN my_project.my_dataset.store_sales
         ON ss_store_sk = s_store_sk
       GROUP BY s_store_sk
       HAVING median < 40 OR median is NULL ;


   Replace the following:
   - <var translate="no">my_project</var> is your project ID.
   - <var translate="no">my_dataset</var> is the ID of a dataset in your project.
   - <var translate="no">my_mv_table</var> is the ID of the materialized view that you're creating.
   - <var translate="no">my_dataset.store</var> and <var translate="no">my_dataset.store_sales</var> are the IDs of the tables in your dataset that serve as the base tables for your materialized view.

   <br />

3. Click **Run**.

   <br />

For more information about how to run queries, see [Run an interactive query](https://docs.cloud.google.com/bigquery/docs/running-queries#queries).

### API

Call the [`tables.insert`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables/insert)
method with a defined `materializedView` resource as part of your API
request. The `materializedView` resource contains a `query` field. For
example:

```googlesql
{
  "kind": "bigquery#table",
  "tableReference": {
    "projectId": "my_project",
    "datasetId": "my_dataset",
    "tableId": "my_mv_table"
  },
  "materializedView": {
    "query": "`SELECT`
        s_store_sk,
        SUM(ss_net_paid) AS sum_sales,
        APPROX_QUANTILES(ss_net_paid, 2)[safe_offset(1)] median
      FROM my_project.my_dataset.store
      LEFT OUTER JOIN my_project.my_dataset.store_sales
        ON ss_store_sk = s_store_sk
      GROUP BY s_store_sk
      HAVING median < 40 OR median is NULL`",
    "allowNonIncrementalDefinition": true
  }
  "maxStaleness": "4:0:0"
}
```

Replace the following:

- <var translate="no">my_project</var> is your project ID.
- <var translate="no">my_dataset</var> is the ID of a dataset in your project.
- <var translate="no">my_mv_table</var> is the ID of the materialized view that you're creating.
- <var translate="no">my_dataset.store</var> and <var translate="no">my_dataset.store_sales</var> are the IDs of the tables in your dataset that serve as the base tables for your materialized view.

### Create materialized views over Spanner external datasets

Before you proceed, you must create the underlying Spanner external dataset using a
[`CLOUD_RESOURCE` connection](https://docs.cloud.google.com/bigquery/docs/spanner-external-datasets#use_a_cloud_resource_connection).

You can create non-incremental materialized views that reference
[Spanner external dataset tables](https://docs.cloud.google.com/bigquery/docs/spanner-external-datasets)
by using the `allow_non_incremental_definition` option.
The following example uses a base Spanner external
dataset table:

```googlesql
/*
  You must create the spanner_external_dataset with a CLOUD_RESOURCE connection.
*/
CREATE MATERIALIZED VIEW sample_dataset.sample_spanner_mv
  OPTIONS (
      enable_refresh = true, refresh_interval_minutes = 60,
      max_staleness = INTERVAL "24" HOUR,
        allow_non_incremental_definition = true)
AS
  SELECT COUNT(*) cnt FROM spanner_external_dataset.spanner_table;
```

### Query with `allow_non_incremental_definition`

You can query non-incremental materialized views as you would query any other
materialized view, logical view, or table.

For example:

```bash
SELECT * FROM  my_project.my_dataset.my_mv_table
```

If the data is not older than the `max_staleness` parameter, then this query
returns data from the last refresh. For details about the staleness and
freshness of data, see [data staleness](https://docs.cloud.google.com/bigquery/docs/materialized-views-create#data_staleness).

### Limitations specific to non-incremental materialized views

The following limitations only apply to materialized views with the
`allow_non_incremental_definition` option. With the exception of limitations on
supported query syntax, all [materialized view
limitations](https://docs.cloud.google.com/bigquery/docs/materialized-views-intro#limitations) still apply.

- Smart tuning is not applied to the materialized views that include the `allow_non_incremental_definition` option. The only way to benefit from materialized views with the `allow_non_incremental_definition` option is to query them directly.
- Materialized views without the `allow_non_incremental_definition` option can incrementally refresh a subset of their data. Materialized views with the `allow_non_incremental_definition` option must be refreshed in their entirety.
- Materialized views with `max_staleness` option validates presence of the column-level security constraints during query execution. See more details about this in [column-level access control](https://docs.cloud.google.com/bigquery/docs/column-level-security-intro#time-travel)
- For materialized views over Spanner external dataset tables, if the last refresh of a non-incremental materialized view occurred outside the `max_staleness` interval, then queries read the base Spanner external dataset tables, even if the base table hasn't changed. For example, if your `max_staleness` interval is 4 hours and the last refresh occurred 7 hours ago, then the query will read the base Spanner external dataset tables.

## What's next

- [Manage materialized views](https://docs.cloud.google.com/bigquery/docs/materialized-views-manage).
- [Use materialized views](https://docs.cloud.google.com/bigquery/docs/materialized-views-use).