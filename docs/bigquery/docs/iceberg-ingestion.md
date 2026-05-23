# Transfer data into Iceberg managed table

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
> **Note:** To get support or provide feedback for this feature, contact [dts-preview-support@google.com](mailto:dts-preview-support@google.com).

You can use the BigQuery Data Transfer Service to transfer data from external sources, such as Amazon Simple Storage Service (Amazon S3), Azure Blob Storage, or Cloud Storage into Iceberg managed table.

## Data sources supported

You can specify Iceberg managed table as the destination table type
when you set up your transfer configuration for these data sources:

- [Amazon S3](https://docs.cloud.google.com/bigquery/docs/s3-transfer)
- [Azure Blob Storage](https://docs.cloud.google.com/bigquery/docs/blob-storage-transfer)
- [Cloud Storage](https://docs.cloud.google.com/bigquery/docs/cloud-storage-transfer)

## Limitations

Transfers to Iceberg managed table are subject to the following limitations:

- **Append-only:** Data is only appended to the destination table. Overwriting, updating, or deleting existing data is not supported.
- **Partitioning limitations:** Partitioning is supported on the destination Iceberg managed table, but some limitations apply. For more information, see [Partitioning limitations](https://docs.cloud.google.com/bigquery/docs/biglake-iceberg-tables-in-bigquery#partitioning_limitations).
- **No backfills:** Transfers do not support [backfills](https://docs.cloud.google.com/bigquery/docs/samples/bigquerydatatransfer-schedule-backfill).

## Pricing

For information on BigQuery Data Transfer Service pricing, see the
[Pricing](https://cloud.google.com/bigquery/pricing#data-transfer-service-pricing)
page.

After data is transferred to Iceberg managed table, standard storage
and query [pricing](https://docs.cloud.google.com/bigquery/docs/iceberg-tables#pricing) applies.