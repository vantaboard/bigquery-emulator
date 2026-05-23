# Introduction to geospatial analytics

In a data warehouse like BigQuery, location information is
common and can influence critical business decisions. You can use geospatial
analytics to analyze and visualize geospatial data in BigQuery
by using the
[`GEOGRAPHY` data type](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#geography_type)
and
[GoogleSQL geography functions](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions).

For example, you might record the latitude and longitude of your delivery
vehicles or packages over time. You might also record customer transactions and
join the data to another table with store location data. You can use this type
of location data to do the following:

- Estimate when a package is likely to arrive.
- Determine which customers should receive a mailer for a particular store location.
- Combine your data with percent tree cover from satellite imagery to decide if delivery by aerial drone is feasible.

## Limitations

Geospatial analytics is subject to the following limitations:

- [Geography functions](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions) are available only in GoogleSQL.
- Only the BigQuery client library for Python supports the `GEOGRAPHY` data type. For other client libraries, convert `GEOGRAPHY` values to strings by using the `ST_ASTEXT` or `ST_ASGEOJSON` function. Converting to text using `ST_ASTEXT` stores only one value, and converting to WKT means that the data is annotated as a `STRING` type instead of a `GEOGRAPHY` type.

## Quotas

Quotas and limits on geospatial analytics apply to the different types of
jobs you can run against tables that contain geospatial data, including the
following job types:

- [Loading data](https://docs.cloud.google.com/bigquery/quotas#load_jobs) (load jobs)
- [Exporting data](https://docs.cloud.google.com/bigquery/quotas#export_jobs) (extract jobs)
- [Querying data](https://docs.cloud.google.com/bigquery/quotas#query_jobs) (query jobs)
- [Copying tables](https://docs.cloud.google.com/bigquery/quotas#copy_jobs) (copy jobs)

For more information on all quotas and limits, see [Quotas and limits](https://docs.cloud.google.com/bigquery/quotas).

## Pricing

When you use geospatial analytics, your charges are based on the
following factors:

- How much data is stored in the tables that contain geospatial data
- The queries you run against the data

For information on storage pricing, see [Storage pricing](https://cloud.google.com/bigquery/pricing#storage).

For information on query pricing, see [Analysis pricing models](https://cloud.google.com/bigquery/pricing#analysis_pricing_models).

Many table operations are free, including loading data, copying tables, and
exporting data. Though free, these operations are subject to
BigQuery's [Quotas and limits](https://docs.cloud.google.com/bigquery/quotas). For information
on all free operations, see [Free operations](https://cloud.google.com/bigquery/pricing#free) on the
pricing page.

## What's next

- To get started with geospatial analytics, see [Get started with geospatial analytics](https://docs.cloud.google.com/bigquery/docs/geospatial-get-started).
- To learn more about visualization options for geospatial analytics, see [Visualize geospatial data](https://docs.cloud.google.com/bigquery/docs/geospatial-visualize).
- To learn more about working with geospatial data, see [Work with geospatial data](https://docs.cloud.google.com/bigquery/docs/geospatial-data).
- To learn more about working with raster data, see [Work with raster data](https://docs.cloud.google.com/bigquery/docs/raster-data).
- To learn more about incorporating Google Earth Engine geospatial data into BigQuery, see [Load Google Earth Engine geospatial data](https://docs.cloud.google.com/bigquery/docs/geospatial-data#load-ee).
- For documentation on GoogleSQL functions in geospatial analytics, see [Geography functions in GoogleSQL](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions).
- To learn about different grid systems, see [Grid systems for spatial analysis](https://docs.cloud.google.com/bigquery/docs/grid-systems-spatial-analysis).
- To learn more about geospatial datasets and geospatial analytics and AI, see [Geospatial Analytics](https://docs.cloud.google.com/solutions/geospatial).