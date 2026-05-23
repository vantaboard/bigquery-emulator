# Introduction to data export

This document describes the different ways of exporting data from
BigQuery.

For more information about data integrations, see
[Introduction to loading, transforming, and exporting data](https://docs.cloud.google.com/bigquery/docs/load-transform-export-intro).

## Export query results

You can export query results to a local file (either as a CSV or JSON file),
Google Drive, or Google Sheets. For more information, see
[Export query results to a file](https://docs.cloud.google.com/bigquery/docs/export-file).

## Export tables

You can export your BigQuery tables in the following data formats:

| Data format | Supported compression types | Supported export methods |
|---|---|---|
| CSV | GZIP | [Export to Cloud Storage](https://docs.cloud.google.com/bigquery/docs/exporting-data) |
| JSON | GZIP | [Export to Cloud Storage](https://docs.cloud.google.com/bigquery/docs/exporting-data) [Read from BigQuery using Dataflow](https://docs.cloud.google.com/dataflow/docs/guides/read-from-bigquery) |
| Avro | DEFLATE, SNAPPY | [Export to Cloud Storage](https://docs.cloud.google.com/bigquery/docs/exporting-data) [Read from BigQuery using Dataflow](https://docs.cloud.google.com/dataflow/docs/guides/read-from-bigquery) |
| Parquet | GZIP, SNAPPY, ZSTD | [Export to Cloud Storage](https://docs.cloud.google.com/bigquery/docs/exporting-data) |

You can also [export your BigQuery tables as Protobuf columns](https://docs.cloud.google.com/bigquery/docs/protobuf-export)
when working with nested data structures that require object
type safety, or if you need a wider language support.

## Export BigQuery code assets

You can download [BigQuery Studio](https://docs.cloud.google.com/bigquery/docs/query-overview#bigquery-studio)
code assets, such as [saved queries](https://docs.cloud.google.com/bigquery/docs/saved-queries-introduction) or [notebooks](https://docs.cloud.google.com/bigquery/docs/notebooks-introduction)
to maintain a local copy of your assets. For more information on downloading your BigQuery code assets, see the
following:

- [Download saved queries](https://docs.cloud.google.com/bigquery/docs/manage-saved-queries#download_saved_queries)
- [Download notebooks](https://docs.cloud.google.com/bigquery/docs/manage-notebooks#download_a_notebook)

## Export using reverse ETL

You can set up reverse ETL (RETL) workflows to move data from
BigQuery to the following databases:

- [Export to Bigtable](https://docs.cloud.google.com/bigquery/docs/export-to-bigtable)
- [Export to Spanner](https://docs.cloud.google.com/bigquery/docs/export-to-spanner)
- [Export to Pub/Sub](https://docs.cloud.google.com/bigquery/docs/export-to-pubsub)
- [Export to AlloyDB](https://docs.cloud.google.com/bigquery/docs/export-to-alloydb) ([preview](https://cloud.google.com/products#product-launch-stages))

## What's next

- Learn about [quotas for extract jobs](https://docs.cloud.google.com/bigquery/quotas#export_jobs).
- Learn about [BigQuery storage pricing](https://cloud.google.com/bigquery/pricing#storage).