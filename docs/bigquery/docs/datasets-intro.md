# Introduction to datasets

This page provides an overview of datasets in BigQuery.

## Datasets

A dataset is contained within a specific [project](https://docs.cloud.google.com/docs/overview#projects). Datasets
are top-level containers that are used to organize and control access to your
[tables](https://docs.cloud.google.com/bigquery/docs/tables) and [views](https://docs.cloud.google.com/bigquery/docs/views). A table
or view must belong to a dataset, so you need to create at least one dataset before
[loading data into BigQuery](https://docs.cloud.google.com/bigquery/loading-data-into-bigquery).
Use the format `projectname.datasetname` to fully qualify a dataset name when
using GoogleSQL, or the format `projectname:datasetname` to fully qualify
a dataset name when using the bq command-line tool.

## Location

You specify a location for storing your BigQuery data when you
create a dataset. For a list of BigQuery dataset locations, see
[BigQuery locations](https://docs.cloud.google.com/bigquery/docs/locations). BigQuery
stores your data in the selected location
in accordance with the [Service Specific Terms](https://cloud.google.com/terms/service-terms).
For example, if you choose `EU` or an EU-based region for the dataset
location, your Core BigQuery Customer Data resides in the EU.

After you create the dataset, the location cannot be changed,
but you can [copy datasets to different locations](https://docs.cloud.google.com/bigquery/docs/copying-datasets),
or manually [move (recreate) the dataset in a different
location](https://docs.cloud.google.com/bigquery/docs/managing-datasets#recreate-dataset).

If you don't [explicitly specify a location](https://docs.cloud.google.com/bigquery/docs/locations#specify_locations),
the location is determined in one of the following ways:

- The location of the datasets referenced in the request. For example, if a query references a table or view in a dataset stored in the `asia-northeast1` region, the query job runs in `asia-northeast1`.
- The region specified for a connection referenced in a request.
- The location of a destination table.

If the location isn't explicitly specified, and it can't be determined from the
resources in the request, the default location is used. If default location
isn't set, the job runs in the `US` multi-region.

## Data retention

Datasets use [time travel](https://docs.cloud.google.com/bigquery/docs/time-travel#time_travel) in
conjunction with the [fail-safe period](https://docs.cloud.google.com/bigquery/docs/time-travel#fail-safe)
to retain deleted and modified data for a short time, in case you need to
recover it. For more information, see
[Data retention with time travel and fail-safe](https://docs.cloud.google.com/bigquery/docs/time-travel).

## Storage billing models

You can be billed for BigQuery data storage in either logical or
physical (compressed) bytes, or a combination of both.
The storage billing model you choose determines your
[storage pricing](https://cloud.google.com/bigquery/pricing#storage). The storage billing model you
choose doesn't impact BigQuery performance. Whichever billing
model you choose, your data is stored as physical bytes.

You set the storage billing model at the dataset level.
If you don't specify a storage billing model when you create a dataset, it
defaults to using logical storage billing. However, you can
[change a dataset's storage billing model](https://docs.cloud.google.com/bigquery/docs/updating-datasets#update_storage_billing_models)
after you create it. If you change a dataset's storage
billing model, you must wait 14 days before you can change the storage billing
model again.

When you change a dataset's billing model, it takes 24 hours for the
change to take effect. Any tables or table partitions in long-term storage
are not reset to active storage when you change a dataset's billing model.
Query performance and query latency are not affected by changing a dataset's
billing model.

Datasets use [time travel](https://docs.cloud.google.com/bigquery/docs/time-travel#time_travel) and
[fail-safe](https://docs.cloud.google.com/bigquery/docs/time-travel#fail-safe) storage for data retention.
Time travel and fail-safe storage are charged separately at active storage rates
when you use physical storage billing, but are included in the base rate you are
charged when you use logical storage billing. You can modify the time travel
window you use for a dataset in order to balance physical storage costs with
data retention. You can't modify the fail-safe window. For more information
about dataset data retention, see
[Data retention with time travel and fail-safe](https://docs.cloud.google.com/bigquery/docs/time-travel).
For more information on forecasting your storage costs, see
[Forecast storage billing](https://docs.cloud.google.com/bigquery/docs/information-schema-table-storage#forecast_storage_billing).
You can't enroll a dataset in physical storage billing if your organization has any existing legacy [flat-rate slot commitments](https://docs.cloud.google.com/bigquery/docs/reservations-commitments-legacy) located in the same region as the dataset. This doesn't apply to commitments purchased with a [BigQuery edition](https://docs.cloud.google.com/bigquery/docs/editions-intro).

## External datasets

In addition to BigQuery datasets, you can create external datasets, which are links to external data sources:

- [Spanner external dataset](https://docs.cloud.google.com/bigquery/docs/spanner-external-datasets)
- [AWS Glue federated dataset](https://docs.cloud.google.com/bigquery/docs/glue-federated-datasets)

*External datasets* are also known as *federated datasets*; both terms are used interchangeably.

Once created, external datasets contain tables from a referenced external data source. Data from these tables aren't copied into BigQuery, but queried every time they are used. For more information, see [Spanner federated queries](https://docs.cloud.google.com/bigquery/docs/spanner-federated-queries).

## Limitations

BigQuery datasets are subject to the following limitations:

- The [dataset location](https://docs.cloud.google.com/bigquery/docs/locations) can only be set at creation time. After a dataset is created, its location cannot be changed.
- All tables that are referenced in a query must be stored in datasets in the same location.
- External datasets don't support table expiration, replicas, time travel, default collation, default rounding mode, or the option to enable or disable case-insensitive table names.

- When [you copy a table](https://docs.cloud.google.com/bigquery/docs/managing-tables#copy-table), the
  datasets that contain the source table and destination table must reside in
  the same location.

- Dataset names must be unique for each project.

- If you change a dataset's
  [storage billing model](https://docs.cloud.google.com/bigquery/docs/datasets-intro#dataset_storage_billing_models), you must wait 14
  days before you can change the storage billing model again.

- You can't enroll a dataset in physical storage billing if you have any
  existing legacy
  [flat-rate slot commitments](https://docs.cloud.google.com/bigquery/docs/reservations-commitments-legacy)
  located in the same region as the dataset.

## Quotas

For more information on dataset quotas and limits, see
[Quotas and limits](https://docs.cloud.google.com/bigquery/quotas#dataset_limits).

## Pricing

You are not charged for creating, updating, or deleting a dataset.

For more information on BigQuery pricing, see [Pricing](https://cloud.google.com/bigquery/pricing).

## Security

To control access to datasets in BigQuery, see
[Controlling access to datasets](https://docs.cloud.google.com/bigquery/docs/control-access-to-resources-iam).
For information about data encryption, see [Encryption at rest](https://docs.cloud.google.com/bigquery/docs/encryption-at-rest).

## What's next

- For more information on creating datasets, see [Creating datasets](https://docs.cloud.google.com/bigquery/docs/datasets).
- For more information on assigning access controls to datasets, see [Controlling access to datasets](https://docs.cloud.google.com/bigquery/docs/dataset-access-controls).