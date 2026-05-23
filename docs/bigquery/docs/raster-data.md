# Work with raster data using Earth Engine in BigQuery

This document
explains how to combine raster and vector data by using the
[`ST_REGIONSTATS` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_regionstats),
which uses Google Earth Engine to get access to image and raster data
in BigQuery.

## Overview

A *raster* is a two-dimensional grid of pixels, each of which is assigned one
or more values called *bands*. For example, each pixel could correspond to one
particular square kilometer on the earth's surface and have bands for
average temperature and average rainfall. Raster data includes satellite
imagery and other continuous, grid-based data such as weather forecasts and
land cover. Many common image
formats, such as PNG or JPEG files, are formatted as raster data.

Raster data
is often contrasted with *vector* data, in which the data is described by
lines or curves rather than a fixed rectangular grid. For
example, you can use the `GEOGRAPHY` data type in BigQuery to
describe the boundaries of countries, cities, or other regions.

Geospatial raster and vector data is often combined using a *zonal statistics*
operation, which computes an aggregate of all raster values within a given
vector region. For example, you might want to compute the following:

- Average air quality in a collection of cities.
- Solar potential for a collection of building polygons.
- Fire risk summarized along power line corridors in forested areas.

BigQuery excels in processing vector data, and Google Earth Engine
excels in processing raster data. You can use the
[`ST_REGIONSTATS` geography function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_regionstats)
to combine raster data using Earth Engine with your vector data stored
in BigQuery.

![A map of the Earth with raster values and computed zonal statistics.](https://docs.cloud.google.com/static/bigquery/images/raster-example.png)

## Before you begin

1. To use the `ST_REGIONSTATS` function in your queries,
   enable the Earth Engine API.

   [Enable the API](https://console.cloud.google.com/apis/library/earthengine.googleapis.com)
2. Optional: To subscribe to and use data published to
   BigQuery sharing (formerly Analytics Hub) by using the
   `ST_REGIONSTATS` function, enable the Analytics Hub API.

   [Enable the API](https://console.cloud.google.com/apis/library/analyticshub.googleapis.com)

### Required permissions


To get the permissions that
you need to call the `ST_REGIONSTATS` function,

ask your administrator to grant you the
following IAM roles on your project:

- [Earth Engine Resource Viewer](https://docs.cloud.google.com/iam/docs/roles-permissions/earthengine#earthengine.viewer) (`roles/earthengine.viewer`)
- [Service Usage Consumer](https://docs.cloud.google.com/iam/docs/roles-permissions/serviceusage#serviceusage.serviceUsageConsumer) (`roles/serviceusage.serviceUsageConsumer`)
- Subscribe to datasets in BigQuery sharing: [BigQuery Data Editor](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquery#bigquery.dataEditor) (`roles/bigquery.dataEditor`)


For more information about granting roles, see [Manage access to projects, folders, and organizations](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).


These predefined roles contain

the permissions required to call the `ST_REGIONSTATS` function. To see the exact permissions that are
required, expand the **Required permissions** section:


#### Required permissions

The following permissions are required to call the `ST_REGIONSTATS` function:

- `earthengine.computations.create`
- `serviceusage.services.use`
- `bigquery.datasets.create`


You might also be able to get
these permissions
with [custom roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles) or
other [predefined roles](https://docs.cloud.google.com/iam/docs/roles-overview#predefined).

## Find raster data

The `raster_id` parameter in the `ST_REGIONSTATS` function is a string that
specifies the source of your raster data. The following sections explain how to
find and format the raster ID.

### BigQuery image tables

You can use BigQuery sharing (formerly Analytics Hub) to discover and
access raster datasets
in BigQuery. To use BigQuery sharing, you need to
[enable the Analytics Hub API](https://docs.cloud.google.com/bigquery/docs/analytics-hub-manage-exchanges#before_you_begin)
and ensure that you have required permissions to
[view and subscribe to listings and data exchanges](https://docs.cloud.google.com/bigquery/docs/analytics-hub-view-subscribe-listings).

Google Earth Engine publishes publicly available datasets
that contain raster data in the `US` and `EU` multi-regions. To
[subscribe](https://docs.cloud.google.com/bigquery/docs/analytics-hub-view-subscribe-listings#subscribe-listings)
to an Earth Engine dataset with raster data, follow these steps:

1. Go to the **Sharing (Analytics Hub)** page.

   [Go to Sharing (Analytics Hub)](https://console.cloud.google.com/bigquery/analytics-hub)
2. Click **Search listings**.

3. In the **Search for listings** field, enter `"Google Earth Engine"`.

4. Click a dataset that you want to subscribe to.

5. Click **Subscribe**.

6. Optional: Update the **Project** or **Linked dataset name** fields.

7. Click **Save**. The linked dataset is added to your project.

The dataset contains an *image table* that
stores metadata for a collection of raster images following
the [STAC](https://stacspec.org/) items specification. An image table is
analogous to an Earth Engine image collection
([`ImageCollection`](https://developers.google.com/earth-engine/guides/ic_creating)).

Each row in the table corresponds to a single raster image, with columns
containing image properties and metadata. The raster ID for each image is stored
in the `assets.image.href` column. Reference images in your queries using this
ID as the `raster_id` parameter value.

Filter the table using property columns to select specific images or image
subsets that meet your criteria. For more information about
available bands, pixel size, and property definitions, open the table and click
the **Image details** tab.

Each image table includes a corresponding `*_metadata` table that
provides supporting information for the image table.

For example, the ERA5-Land dataset provides daily climate variable statistics
and is publicly available. The `climate`
table contains multiple raster IDs. The following query filters the image table
using the `start_datetime` column to get the raster ID for the
image corresponding to January 1, 2025 and computes the average temperature
for each country using the `temperature_2m` band:

### SQL

    WITH SimplifiedCountries AS (
      SELECT
        ST_SIMPLIFY(geometry, 10000) AS simplified_geometry,
        names.primary AS name
      FROM
        `bigquery-public-data.overture_maps.division_area`
      WHERE
        subtype = 'country'
    )
    SELECT
      sc.simplified_geometry AS geometry,
      sc.name,
      ST_REGIONSTATS(
        sc.simplified_geometry,
        (SELECT assets.image.href
        FROM `LINKED_DATASET_NAME.climate`
        WHERE start_datetime = '2025-01-01 00:00:00'),
        'temperature_2m'
      ).mean - 273.15 AS mean_temperature
    FROM
      SimplifiedCountries AS sc
    ORDER BY
      mean_temperature DESC;

### BigQuery DataFrames

Before trying this sample, follow the BigQuery DataFrames
setup instructions in the [BigQuery quickstart
using BigQuery DataFrames](https://docs.cloud.google.com/bigquery/docs/dataframes-quickstart).
For more information, see the
[BigQuery DataFrames reference documentation](https://docs.cloud.google.com/python/docs/reference/bigframes/latest).

To authenticate to BigQuery, set up Application Default Credentials.
For more information, see [Set
up ADC for a local development environment](https://docs.cloud.google.com/docs/authentication/set-up-adc-local-dev-environment).

    import datetime
    from typing import cast

    import bigframes.bigquery as bbq
    import bigframes.pandas as bpd

    # TODO: Set the project_id to your Google Cloud project ID.
    # project_id = "your-project-id"
    bpd.options.bigquery.project = project_id

    # TODO: Set the dataset_id to the ID of the dataset that contains the
    # `climate` table. This is likely a linked dataset to Earth Engine.
    # See: https://cloud.google.com/bigquery/docs/link-earth-engine
    linked_dataset = "era5_land_daily_aggregated"

    # For the best efficiency, use partial ordering mode.
    bpd.options.bigquery.ordering_mode = "partial"

    # Load the table of country boundaries.
    countries = bpd.read_gbq("bigquery-public-data.overture_maps.division_area")

    # Filter to just the countries.
    countries = countries[countries["subtype"] == "country"].copy()
    countries["name"] = countries["names"].struct.field("primary")
    countries["simplified_geometry"] = bbq.st_simplify(
        countries["geometry"],
        tolerance_meters=10_000,
    )

    # Get the reference to the temperature data from a linked dataset.
    # Note: This sample assumes you have a linked dataset to Earth Engine.
    image_href = (
        bpd.read_gbq(f"{project_id}.{linked_dataset}.climate")
        .set_index("start_datetime")
        .loc[[datetime.datetime(2025, 1, 1, tzinfo=datetime.timezone.utc)], :]
    )
    raster_id = image_href["assets"].struct.field("image").struct.field("href")
    raster_id = raster_id.item()
    stats = bbq.st_regionstats(
        countries["simplified_geometry"],
        raster_id=cast(str, raster_id),
        band="temperature_2m",
    )

    # Extract the mean and convert from Kelvin to Celsius.
    countries["mean_temperature"] = stats.struct.field("mean") - 273.15

    # Sort by the mean temperature to find the warmest countries.
    result = countries[["name", "mean_temperature"]].sort_values(
        "mean_temperature", ascending=False
    )
    print(result.head(10))

### Cloud Storage GeoTIFF

GeoTIFF is a common file format for storing geospatial raster data. The
`ST_REGIONSTATS` function supports raster data stored in the
[Cloud Optimized GeoTIFF](https://developers.google.com/earth-engine/Earth_Engine_asset_from_cloud_geotiff)
(COG) format in
Cloud Storage buckets that are located in the following regions:

- `US` multi-region
- `us-central1`
- `EU` multi-region
- `europe-west1`

Provide the Cloud Storage URI as the raster ID, such as
`gs://bucket/folder/raster.tif`.

### Earth Engine image assets

The `ST_REGIONSTATS` function supports passing an Earth Engine
image asset path for the `raster_id` argument. Earth Engine raster
data is available as individual images or collections of images. These data
exist in the `US` region and are only compatible with queries that run in the
`US` region. To find the raster ID for an image, follow
these steps:

1. Search the [Earth Engine data catalog](https://developers.google.com/earth-engine/datasets) for the dataset that you're interested in.
2. To open the description page for that entry, click the dataset name.
   The **Earth Engine Snippet** either describes a single image or a
   collection of images.

   If the Earth Engine snippet is of the form
   `ee.Image('IMAGE_PATH')`,
   then the raster ID is `'ee://IMAGE_PATH'`.

   If the Earth Engine snippet is of the form
   `ee.ImageCollection('IMAGE_COLLECTION_PATH')`, you
   can use the
   [Earth Engine Code Editor](https://developers.google.com/earth-engine/guides/quickstart_javascript)
   to
   [filter the ImageCollection](https://developers.google.com/earth-engine/guides/ic_filtering)
   to a single image. Use the `ee.Image.get('system:id')` method to print the
   `IMAGE_PATH` value for that image to the console. The raster ID is
   `'ee://IMAGE_PATH'`.

## Pixel weights

You can specify a *weight* , sometimes referred to as a *mask value* ,
for the `include` parameter in the `ST_REGIONSTATS`
function that determines how much to
weight each pixel in calculations. Weight values must range from 0 to 1.
Weights outside this range are set to the nearest limit, either 0 or 1.

A pixel is considered *valid* if
it has a weight greater than 0. A weight of 0 indicates an *invalid* pixel.
Invalid pixels usually represent missing or unreliable data, such as areas
obscured by clouds, sensor anomalies, processing errors, or locations outside
of a defined boundary.

If you don't specify a weight, each pixel is automatically weighted by
the proportion of the pixel that falls within the geometry,
allowing for proportional inclusion in zonal statistics. If the geometry is less
than 1/256 of the size of the pixel, the weight of the pixel is 0. In these
cases, `null` is returned for all statistics except `count` and `area`,
which are 0.

If a partially intersecting pixel has a weight from the `include`
argument to `ST_REGIONSTATS`, then BigQuery uses the minimum of
that weight and the fraction of the pixel that intersects the region.

Weight values don't have the same precision as `FLOAT64` values. In practice,
their true value might differ from the value used in computations by up to
1/256 (about 0.4%).

You can provide an expression using Earth Engine
[image expression syntax](https://developers.google.com/earth-engine/guides/image_math#expressions)
in your `include` argument to dynamically weight
pixels based on specific criteria within raster bands. For example, the
following expression restricts calculations to pixels where the `probability`
band exceeds 70%:

    include => 'probability > 0.7'

If the dataset includes a weight-factor band, you can use it with
the following syntax:

    include => 'weight_factor_band_name'

## Pixel size and scale of analysis

A geospatial raster image is a grid of pixels that corresponds to some location
on the surface of the Earth. The pixel size of a raster, sometimes called the
*scale*, is the nominal size of one edge of a pixel in the grid's coordinate
reference system.
For example, a raster with 10-meter resolution has pixels of size 10 meters by
10 meters. Original reported pixel size can vary dramatically between datasets,
from less than 1 meter to greater than 20 kilometers.

When using the `ST_REGIONSTATS` function to compute zonal statistics, the pixel
size of the raster data is a crucial consideration. For example, aggregating
high-resolution raster data over the region of a country can be computationally
intensive and unnecessarily granular. Conversely, aggregating low-resolution
data over the region, such as city parcels, might not provide sufficient detail.

To get meaningful and efficient results from your analysis, we recommend
choosing a pixel size appropriate for the size of your
polygons and the objective of your analysis. You can find the pixel size
for each raster dataset in the description section of image tables in
BigQuery sharing.

Changing the pixel size changes the number of pixels that intersect a given
geography, which affects the results and their interpretation. We don't
recommend changing the pixel size for production analyses. However, if you're
prototyping a query, increasing the pixel size can reduce query runtime and
cost, especially for high-resolution data.

To change the pixel size, set the
`scale` in the `options` argument to the `ST_REGIONSTATS` function. For example,
to compute statistics over 1,000-meter pixels, use
`options => JSON '{"scale":1000}'`, which tells
Earth Engine to resample the image at the requested scale. To learn
more about how Earth Engine handles rescaling, see
[Scale](https://developers.google.com/earth-engine/guides/scale) in the
Google Earth Engine documentation.

Computing statistics for polygons that are significantly smaller than the
pixels of the raster can produce inaccurate or null results. In such a case,
one alternative is to replace the polygon with its centroid point using
[`ST_CENTROID`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_centroid).

## Billing

When you run a query, usage of the `ST_REGIONSTATS` function is billed
separately from the rest of the query because Earth Engine computes
the results of the function call. You are
billed for this usage in slot hours under the BigQuery Services
SKU, regardless of whether you use on-demand billing or reservations. To see the
amount billed for BigQuery calls to Earth Engine,
[view your billing report](https://docs.cloud.google.com/billing/docs/how-to/reports#overview)
and use [labels](https://docs.cloud.google.com/billing/docs/how-to/reports#filter-by-labels) to filter by
the label key `goog-bq-feature-type`, with value `EARTH_ENGINE`. If the
`ST_REGIONSTATS` function fails, then you aren't billed for any
Earth Engine computation that was used.

For each query, you can use the
[`jobs.get` method](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs/get)
in the BigQuery API to see the following information:

- The [`slotMs` field](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/Job#externalservicecost), which shows the number of slot milliseconds consumed by Earth Engine when the `externalService` field is `EARTH_ENGINE` and the `billingMethod` field is `SERVICES_SKU`.
- The [`totalServicesSkuSlotMs` field](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/Job#jobstatistics2), which shows the total number of slot milliseconds used by all BigQuery external services that get billed on the BigQuery Services SKU.

You can also query the `total_services_sku_slot_ms` field in the
[`INFORMATION_SCHEMA.JOBS` view](https://docs.cloud.google.com/bigquery/docs/information-schema-jobs)
to find the total slot milliseconds consumed by external services billed on the
BigQuery Services SKU.

### Cost factors

The following factors impact the compute usage when you run the `ST_REGIONSTATS`
function:

- The number of input rows.
- The raster image that you use. Some rasters are composites created from source image collections in the Earth Engine data catalog, and the computational resources to produce the composite result varies.
- The resolution of the image.
- The size and complexity of the input geography, number of pixels that intersect the geography, and the number of image tiles and bytes read by Earth Engine.
- The location of the input geography on Earth relative to the source images
  and the image's projection and resolution.

  - Image projections can warp pixels, especially pixels at high latitudes or far outside the image's intended coverage area.
  - For composite rasters, the number of source images intersecting the input geography can vary regionally and over time. For example, some satellites produce more images at low or high latitudes, depending on their orbit and data collection parameters, or may omit images depending on changing atmospheric conditions.
- The use of formulas in the `include` or `band_name` arguments, and the
  number of bands they involve.

- The caching of previous results.

### Control costs

To control costs associated with the `ST_REGIONSTATS` function, you can adjust
the quota that controls the amount of slot time that the function is
allowed to consume. The default is 350 slot-hours per day.
When you [view your quotas](https://docs.cloud.google.com/docs/quotas/view-manage),
filter the **Metric** list to
`earthengine.googleapis.com/bigquery_slot_usage_time`
to see the Earth Engine quota associated with calls from
BigQuery. For more information, read about
[BigQuery
raster functions quotas](https://developers.google.com/earth-engine/guides/usage#bigquery_slot-time_per_day)
in the Google Earth Engine documentation.

> [!NOTE]
> **Note:** Like custom query quotas in BigQuery, this quota is approximate. It provides a safeguard against excessive spending, but it's not designed to strictly limit slot time. BigQuery might occasionally run a query that exceeds the quota limit, and you might exhaust your quota without being billed for the entire consumed amount.

## Supported regions

Queries that call the `ST_REGIONSTATS` function must run in one of the following
regions:

- `US` multi-region
- `us-central1`
- `us-central2`
- `EU` multi-region
- `europe-west1`

## What's next

- Try the tutorial that shows you how to [use raster data to analyze temperature](https://docs.cloud.google.com/bigquery/docs/raster-tutorial-weather).
- Learn more about [geography functions in BigQuery](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions).
- Learn more about [working with geospatial data](https://docs.cloud.google.com/bigquery/docs/geospatial-data).