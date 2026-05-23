# Export query results to Blob Storage

This document describes how to export the result of a query that runs against a
[BigLake table](https://docs.cloud.google.com/bigquery/docs/biglake-intro) to your
Azure Blob Storage.

For information about how data flows between BigQuery and
Azure Blob Storage,
see [Data flow when exporting data](https://docs.cloud.google.com/bigquery/docs/omni-introduction#export-data).

## Limitations

For a full list of limitations that apply to BigLake tables
based on Amazon S3 and Blob Storage, see [Limitations](https://docs.cloud.google.com/bigquery/docs/omni-introduction#limitations).

## Before you begin

Ensure that you have the following resources:


- A [connection to access your Blob Storage](https://docs.cloud.google.com/bigquery/docs/omni-azure-create-connection). Within the connection, you must create a policy for the Blob Storage container path that you want to export to. Then, within that policy, create a role that has the `Microsoft.Storage/storageAccounts/blobServices/containers/write` permission.
- An [Blob Storage BigLake table](https://docs.cloud.google.com/bigquery/docs/omni-azure-create-external-table).

<!-- -->

- If you are on the [capacity-based pricing model](https://cloud.google.com/bigquery/pricing#capacity_compute_analysis_pricing), then ensure that you have enabled the [BigQuery Reservation API](https://console.cloud.google.com/apis/library/bigqueryreservation.googleapis.com) for your project. For information about pricing, see [BigQuery Omni pricing](https://cloud.google.com/bigquery/pricing#bqomni).

## Export query results

BigQuery Omni writes to the specified Blob Storage location regardless of any existing
content. The export query can overwrite existing data or mix the query result
with existing data. We recommend that you export the query result to an empty
Blob Storage container.

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the **Query editor** field, enter a GoogleSQL export query:

   ```bash
   EXPORT DATA WITH CONNECTION \`CONNECTION_REGION.CONNECTION_NAME\`
   OPTIONS(
     uri="azure://AZURE_STORAGE_ACCOUNT_NAME.blob.core.windows.net/CONTAINER_NAME/FILE_PATH/*",
     format="FORMAT"
   )
   AS QUERY
   ```

   Replace the following:
   - `CONNECTION_REGION`: the region where the connection was created.
   - `CONNECTION_NAME`: the connection name that you created with the necessary permission to write to the container.
   - `AZURE_STORAGE_ACCOUNT_NAME`: the name of the Blob Storage account to which you want to write the query result.
   - `CONTAINER_NAME`: the name of the container to which you want to write the query result.
   - `FILE_PATH`: the path where you want to write the exported file to. It must contain exactly one wildcard `*` anywhere in the leaf directory of the path string, for example, `../aa/*`, `../aa/b*c`, `../aa/*bc`, and `../aa/bc*`. BigQuery replaces `*` with `0000..N` depending on the number of files exported. BigQuery determines the file count and sizes. If BigQuery decides to export two files, then `*` in the first file's filename is replaced by `000000000000`, and `*` in the second file's filename is replaced by `000000000001`.
   - `FORMAT`: supported formats are `JSON`, `AVRO`, `CSV`, and `PARQUET`.
   - `QUERY`: the query to analyze the data that is stored in a BigLake table.

> [!NOTE]
> **Note:** To override the default project, use the `--project_id=PROJECT_ID` parameter. Replace `PROJECT_ID` with the ID of your Google Cloud project.

## Troubleshooting

If you get an error related to `quota failure`, then check if you have reserved
capacity for your queries. For more information about slot reservations, see
[Before you begin](https://docs.cloud.google.com/bigquery/docs/omni-azure-export-results-to-azure-storage#before_you_begin) in this document.

## What's next

- Learn about [BigQuery Omni](https://docs.cloud.google.com/bigquery/docs/omni-introduction).
- Learn how to [export table data](https://docs.cloud.google.com/bigquery/docs/exporting-data).
- Learn how to [query data stored in Blob Storage](https://docs.cloud.google.com/bigquery/docs/query-azure-data).
- Learn how to [set up VPC Service Controls for BigQuery Omni](https://docs.cloud.google.com/bigquery/docs/omni-vpc-sc).