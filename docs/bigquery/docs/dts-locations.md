# Data location and transfers

This page explains the concepts of transfer configuration
location and source data location, and also describes how locations
and transfers interact.

For more information about BigQuery locations more generally, see
[Dataset locations](https://docs.cloud.google.com/bigquery/docs/locations).


## Transfer location

Transfer configurations have locations. When you set up a transfer
configuration, set the transfer configuration up in the same project as the
destination dataset. The location of the transfer configuration is automatically
set to the same location that you specified for the destination dataset. The
BigQuery Data Transfer Service processes and stages data in the same location as the
destination BigQuery dataset. If you don't have a destination dataset when
you create the transfer, you must create one in BigQuery before configuring
the transfer.

## Source data location

The source data you want to transfer to BigQuery might have a location.
However, the location where your source data is stored and the location of the
destination dataset in BigQuery are irrelevant.

## Location considerations for data warehouse migrations

Data warehouse migrations from
[Teradata](https://docs.cloud.google.com/bigquery/docs/migration/teradata-overview) require a
Cloud Storage bucket as part of the transfer process. The Cloud Storage bucket
must be colocated with the BigQuery destination dataset.

Redshift data warehouse migrations do not require a colocated Cloud Storage
bucket.

> [!IMPORTANT]
> **Key Point:** You can copy a dataset or manually move it to another location. See [Managing datasets](https://docs.cloud.google.com/bigquery/docs/managing-datasets) for details. For more information about using Cloud Storage to store and move large datasets, see [Using Cloud Storage with big data](https://docs.cloud.google.com/storage/docs/working-with-big-data).

## What's next

- View [all the Google Cloud services available in locations worldwide](https://cloud.google.com/about/locations/#region).
- [Explore additional location-based concepts](https://docs.cloud.google.com/docs/geography-and-regions), such as zones, that apply to other Google Cloud services.