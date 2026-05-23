# Work with change history

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
> **Note:** For support during the preview, contact [bq-change-history-feedback@google.com](mailto:bq-change-history-feedback@google.com).

BigQuery change history lets you track the history of
changes to a BigQuery table. You can use GoogleSQL
[functions](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/table-functions-built-in)
to see particular types of changes made during a specified time range, so
that you can process incremental changes made to a table. Understanding what
changes have been made to a table can help you do things like incrementally
maintain a table replica outside of BigQuery while avoiding
costly copies.

## Required permissions

To view the change history on a table, you need the `bigquery.tables.getData`
permission on that table. The following predefined Identity and Access Management (IAM)
roles include this permission:

- `roles/bigquery.dataViewer`
- `roles/bigquery.dataEditor`
- `roles/bigquery.dataOwner`
- `roles/bigquery.admin`

If a table has, or has had,
[row-level access policies](https://docs.cloud.google.com/bigquery/docs/row-level-security-intro), then only
a table administrator can access historical data for the table. The
`bigquery.rowAccessPolicies.overrideTimeTravelRestrictions` permission is
required on the table and is included in the predefined `roles/bigquery.admin`
IAM role.

If a table has column-level security, you can only view the
change history on the columns that you have access to.

## Change history functions

You can use the following functions to understand a table's change history:

- [`APPENDS`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/time-series-functions#appends):
  returns all rows appended to a table for given time range.

  The following operations add rows to the `APPENDS` change history:
  - [`CREATE TABLE` DDL statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_table_statement)
  - [`INSERT` DML statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/dml-syntax#insert_statement)
  - [Data appended as part of a `MERGE` DML statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/dml-syntax#merge_statement)
  - [Loading data](https://docs.cloud.google.com/bigquery/docs/loading-data) into BigQuery
  - [Streaming ingestion](https://docs.cloud.google.com/bigquery/docs/write-api)
- [`CHANGES`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/time-series-functions#changes):
  returns all rows that have changed in a table for a given time range. To use
  the `CHANGES` function on a table, you must set the table's
  [`enable_change_history` option](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#table_option_list)
  to `TRUE`.

  The following operations add rows to the `CHANGES` change history:
  - [`CREATE TABLE` DDL statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_table_statement)
  - [`INSERT` DML statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/dml-syntax#insert_statement)
  - [Data appended or changed as part of a `MERGE` DML statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/dml-syntax#merge_statement)
  - [`UPDATE` DML statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/dml-syntax#update_statement)
  - [`DELETE` DML statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/dml-syntax#delete_statement)
  - [Loading data](https://docs.cloud.google.com/bigquery/docs/loading-data) into BigQuery
  - [Streaming ingestion](https://docs.cloud.google.com/bigquery/docs/write-api)
  - [`TRUNCATE TABLE` DML statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/dml-syntax#truncate_table_statement)
  - [Jobs](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/Job) configured with a `writeDisposition` of `WRITE_TRUNCATE`
  - Individual [table partition deletions](https://docs.cloud.google.com/bigquery/docs/managing-partitioned-tables#delete_a_partition)

## Pricing and costs

Calling change history functions
incurs [BigQuery compute costs](https://cloud.google.com/bigquery/pricing#analysis_pricing_models).
Both the `APPENDS` and `CHANGES` functions require processing all data written
to the table within the specified time range. This processing applies to all
writes, including both append and mutation operations.
Setting a table's [`enable_change_history` option](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#table_option_list) to `FALSE` does not reduce the data processed by `APPENDS`.

When you set the
[`enable_change_history` option](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#table_option_list)
on a table to `TRUE` in order to use the `CHANGES` function,
BigQuery stores table change metadata. This stored metadata
incurs additional [BigQuery storage costs](https://cloud.google.com/bigquery/pricing#storage)
and [BigQuery compute costs](https://cloud.google.com/bigquery/pricing#analysis_pricing_models).
The amount billed depends on the number and type of changes made to the table,
and is typically small. Tables that have many change operations, especially
large deletions, are the most likely to incur noticeable costs.