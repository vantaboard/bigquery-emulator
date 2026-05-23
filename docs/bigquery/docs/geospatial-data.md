# Working with geospatial data

Geospatial analytics let you analyze geographic data in
BigQuery. Geographic data is also known as *geospatial data*.

Common types of objects when working with geospatial data include the following:

- A *geometry* represents a surface area on the Earth. It is often described using points, lines, polygons, or a collection of points, lines, and polygons. A *geometry collection* is a geometry that represents the spatial union of all shapes in the collection.
- A *spatial feature* represents a logical spatial object. It combines a geometry with additional attributes that are application-specific.
- A *spatial feature collection* is a set of spatial features.

In BigQuery, the
[`GEOGRAPHY`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#geography_type)
data type represents a geometry value or geometry collection. To represent
spatial features, create a table with a `GEOGRAPHY` column for the geometry plus
additional columns for the attributes. Each row of the table is a spatial
feature, and the entire table represents a spatial feature collection.

The `GEOGRAPHY` data type describes a *point set* on the Earth's surface. A
point set is a set of points, lines, and polygons on the
[WGS84](https://earth-info.nga.mil/GandG/update/index.php?action=home#tab_wgs84-data)
reference spheroid, with geodesic edges. You can use the `GEOGRAPHY` data type
by calling one of the GoogleSQL
[geography functions](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions).

## Loading geospatial data

Single points on Earth can be described by just a longitude, latitude pair.
For example, you can load a CSV file that contains longitude and latitude values
and then use the
[`ST_GEOGPOINT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_geogpoint)
function to convert them into `GEOGRAPHY` values.

For more complex geographies, you can load the following geospatial data formats
into a `GEOGRAPHY` column:

- Well-known text (WKT)
- Well-known binary (WKB)
- GeoJSON
- GeoParquet

### Loading WKT or WKB data

[WKT](https://en.wikipedia.org/wiki/Well-known_text) is a
text format for describing individual geometry shapes using points, lines,
polygons with optional holes, or a collection of points, lines, or polygons. WKB
is the binary version of the WKT format. WKB can be hex encoded for formats that
don't support binary data, like JSON.

For example, the following defines a point in WKT:

    POINT(-121 41)

To describe a spatial feature, WKT is usually embedded in a container file
format, such as a CSV file, or in a database table. A file row or a table row
usually corresponds to the spatial feature. The whole file or the whole table
corresponds to the feature collection. To load WKT data into
BigQuery, provide a [schema](https://docs.cloud.google.com/bigquery/docs/schemas) that
specifies a `GEOGRAPHY` column for the geospatial data.

> [!NOTE]
> **Note:** You can't use schema auto-detection to load WKT data as a `GEOGRAPHY` value. If auto-detection is enabled, then BigQuery loads the data as a `STRING` value.

For example, you might have a CSV file that contains the following data:

    "POLYGON((-124.49 47.35,-124.49 40.73,-116.49 40.73,-116.49 47.35,-124.49 47.35))",poly1
    "POLYGON((-85.6 31.66,-85.6 24.29,-78.22 24.29,-78.22 31.66,-85.6 31.66))",poly2
    "POINT(1 2)",point1

You can load this file by running the bq command-line tool `load` command:

    bq load --source_format=CSV \
      --schema="geography:GEOGRAPHY,name:STRING" \
      mydataset.mytable filename1.csv

For more information about loading data in BigQuery, see
[Introduction to loading data](https://docs.cloud.google.com/bigquery/docs/loading-data).

To stream WKT data to an existing BigQuery table with a
`GEOGRAPHY` column, serialize the data as a string in the API request.

### bq

Run the bq command-line tool `insert` command:

    echo '{"geo": "LINESTRING (-118.4085 33.9416, -73.7781 40.6413)"}' \
        | bq insert my_dataset.geo_table

### Python


Before trying this sample, follow the Python setup instructions in the
[BigQuery quickstart using
client libraries](https://docs.cloud.google.com/bigquery/docs/quickstarts/quickstart-client-libraries).


For more information, see the
[BigQuery Python API
reference documentation](https://docs.cloud.google.com/python/docs/reference/bigquery/latest).


To authenticate to BigQuery, set up Application Default Credentials.
For more information, see

[Set up authentication for client libraries](https://docs.cloud.google.com/bigquery/docs/authentication#client-libs).

    from google.cloud import https://docs.cloud.google.com/python/docs/reference/bigquery/latest
    import shapely.geometry
    import shapely.wkt

    bigquery_client = https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html()

    # This example uses a table containing a column named "geo" with the
    # GEOGRAPHY data type.
    table_id = "my-project.my_dataset.my_table"

    # Use the Shapely library to generate WKT of a line from LAX to
    # JFK airports. Alternatively, you may define WKT data directly.
    my_geography = shapely.geometry.LineString(
        [(-118.4085, 33.9416), (-73.7781, 40.6413)]
    )
    rows = [
        # Convert data into a WKT string.
        {"geo": shapely.wkt.dumps(my_geography)},
    ]

    #  table already exists and has a column
    # named "geo" with data type GEOGRAPHY.
    errors = bigquery_client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_insert_rows_json(table_id, rows)
    if errors:
        raise RuntimeError(f"row insert failed: {errors}")
    else:
        print(f"wrote 1 row to {table_id}")

For more information about streaming data in BigQuery, see
[Streaming data into BigQuery](https://docs.cloud.google.com/bigquery/docs/streaming-data-into-bigquery).

You can also convert a WKT text string into a `GEOGRAPHY` value by using the
[`ST_GEOGFROMTEXT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_geogfromtext)
function.

### Loading GeoJSON data

[GeoJSON](https://geojson.org/) is a JSON-based format for geometries and spatial features. For example,
the following defines a point in GeoJSON:

    { "type": "Point", "coordinates": [-121,41] }

GeoJSON data can contain any of the following object types:

- *Geometry objects*. A geometry object is a spatial shape, described as a union of points, lines, and polygons with optional holes.
- *Feature objects*. A feature object contains a geometry plus additional name/value pairs, whose meaning is application-specific.
- *Feature collections*. A feature collection is a set of feature objects.

There are two ways to load GeoJSON data into BigQuery:

- [Load newline-delimited GeoJSON files](https://docs.cloud.google.com/bigquery/docs/geospatial-data#geojson-files).
- [Load individual GeoJSON geometry objects embedded in other file types](https://docs.cloud.google.com/bigquery/docs/geospatial-data#geojson-data).

#### Loading newline-delimited GeoJSON files

A newline-delimited GeoJSON file contains a list of GeoJSON feature objects, one
per line in the file. A GeoJSON feature object is a JSON object with the
following members:

- `type`. For feature objects, the value must be `Feature`.
  BigQuery validates the value but does not include it in the
  table schema.

- `geometry`. The value is a GeoJSON `Geometry` object or `null`.
  BigQuery converts this member into a `GEOGRAPHY` value.

- `properties`. The value is any JSON object or null. If the value isn't `null`,
  then BigQuery loads each member of the JSON object as a
  separate table column. For more information about how BigQuery
  parses JSON data types, see
  [Details of loading JSON data](https://docs.cloud.google.com/bigquery/docs/loading-data-cloud-storage-json#details_of_loading_json_data).

- `id`. Optional. If present, the value is either a string or a number.
  BigQuery loads this value into a column named `id`.

If the feature object contains other members that are not listed here, then
BigQuery converts those members directly into table columns.

You can load a newline-delimited GeoJSON file by using the bq command-line tool's `bq
load` command, as follows:

```
bq load \
 --source_format=NEWLINE_DELIMITED_JSON \
 --json_extension=GEOJSON \
 --autodetect \
 DATASET.TABLE \
 FILE_PATH_OR_URI
```

Replace the following:

- `DATASET` is the name of your dataset.
- `TABLE` is the name of the destination table.
- `FILE_PATH_OR_URI` is a path to a local file or a [Cloud Storage URI](https://docs.cloud.google.com/bigquery/docs/batch-loading-data#gcs-uri).

The previous example enables
[schema auto-detection](https://docs.cloud.google.com/bigquery/docs/schema-detect). For more control over
how BigQuery converts the values inside the `properties` object,
you can provide an explicit schema instead. For more information, see
[Specify schemas](https://docs.cloud.google.com/bigquery/docs/schemas#specify_schemas).
If you provide an explicit schema, then don't include a top-level `type` column
in the schema definition. For each member of the `properties` member, define
separate columns, not a single nested column.

As defined by [RFC 7946](https://tools.ietf.org/html/rfc7946),
a complete GeoJSON data structure is a single JSON object. Many systems export
GeoJSON data as a single `FeatureCollection` object that contains all of the
geometries. To load this format into BigQuery, you must convert
the file by removing the root-level `FeatureCollection` object and splitting the
individual feature objects into separate lines. For example, the following
command uses the `jq` command-line tool to split a GeoJSON file into
newline-delimited format:

    cat ~/file1.json | jq -c '.features[]' > converted.json

#### Creating an external table from a newline-delimited GeoJSON file

You can query a newline-delimited GeoJSON file stored in Cloud Storage by
creating an [external table](https://docs.cloud.google.com/bigquery/docs/external-tables). To create the
external table, use the
[`CREATE EXTERNAL TABLE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_external_table_statement)
DDL statement. In the `OPTIONS` clause, set the `format` option to
`NEWLINE_DELIMITED_JSON` and the `json_extension` option to `GEOJSON`.

Example:

    CREATE EXTERNAL TABLE mydataset.table1 OPTIONS (
      format="NEWLINE_DELIMITED_JSON",
      json_extension = 'GEOJSON',
      uris = ['gs://mybucket/geofile.json']
    );

#### Loading GeoJSON geometry data

Geospatial analytics supports loading individual GeoJSON geometry objects that are
embedded as text strings in other file types. For example, you can load a CSV
file where one of the columns contains a GeoJSON geometry object.

To load this type of GeoJSON data into BigQuery, provide a
[schema](https://docs.cloud.google.com/bigquery/docs/schemas) that specifies a `GEOGRAPHY` column for the
GeoJSON data. You must manually provide the schema. Otherwise, if auto-detection
is enabled, then BigQuery loads the data as a `STRING` value.

Geospatial analytics does not support loading GeoJSON feature objects or feature
collections using this approach. If you need to load feature objects, then
consider using newline-delimited GeoJSON files.

To stream GeoJSON data to an existing BigQuery table with a
`GEOGRAPHY` column, serialize the data as a string in the API request.

### bq

Run the bq command-line tool `insert` command:

    echo '{"geo": "{\"type\": \"LineString\", \"coordinates\": [[-118.4085, 33.9416], [-73.7781, 40.6413]]}"}' \
      | bq insert my_dataset.geo_table

### Python


Before trying this sample, follow the Python setup instructions in the
[BigQuery quickstart using
client libraries](https://docs.cloud.google.com/bigquery/docs/quickstarts/quickstart-client-libraries).


For more information, see the
[BigQuery Python API
reference documentation](https://docs.cloud.google.com/python/docs/reference/bigquery/latest).


To authenticate to BigQuery, set up Application Default Credentials.
For more information, see

[Set up authentication for client libraries](https://docs.cloud.google.com/bigquery/docs/authentication#client-libs).

    import geojson
    from google.cloud import https://docs.cloud.google.com/python/docs/reference/bigquery/latest

    bigquery_client = https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html()

    # This example uses a table containing a column named "geo" with the
    # GEOGRAPHY data type.
    table_id = "my-project.my_dataset.my_table"

    # Use the python-geojson library to generate GeoJSON of a line from LAX to
    # JFK airports. Alternatively, you may define GeoJSON data directly, but it
    # must be converted to a string before loading it into BigQuery.
    my_geography = geojson.LineString([(-118.4085, 33.9416), (-73.7781, 40.6413)])
    rows = [
        # Convert GeoJSON data into a string.
        {"geo": geojson.dumps(my_geography)}
    ]

    #  table already exists and has a column
    # named "geo" with data type GEOGRAPHY.
    errors = bigquery_client.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_insert_rows_json(table_id, rows)
    if errors:
        raise RuntimeError(f"row insert failed: {errors}")
    else:
        print(f"wrote 1 row to {table_id}")

You can also convert a GeoJSON geometry object into a `GEOGRAPHY` value by using
the [`ST_GEOGFROMGEOJSON`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_geogfromgeojson)
function. For example, you can store the geometries as `STRING` values and then
run a query that calls `ST_GEOGFROMGEOJSON`.

### Loading GeoParquet files

[GeoParquet](https://geoparquet.org) is a specification
that adds geospatial types to the [Parquet](https://parquet.apache.org/) file
format. GeoParquet includes metadata that provides definite semantics to the
contained geospatial data, avoiding the [interpretation
issues](https://docs.cloud.google.com/bigquery/docs/geospatial-data#coordinate_systems_and_edges) that occur with other geospatial data
formats.

When loading Parquet files, BigQuery checks for GeoParquet
metadata. If GeoParquet metadata exists, BigQuery loads all of
the columns it describes into a corresponding `GEOGRAPHY` column by default.
For more information about loading Parquet files, see
[Loading Parquet data](https://docs.cloud.google.com/bigquery/docs/loading-data-cloud-storage-parquet).

> [!NOTE]
> **Note:** GeoParquet support is disabled for a few projects to avoid breaking existing workflows. If your GeoParquet files aren't loaded directly to `GEOGRAPHY` columns, [contact support](https://docs.cloud.google.com/bigquery/docs/getting-support).

#### Creating an external table from GeoParquet data

[External tables](https://docs.cloud.google.com/bigquery/docs/external-data-cloud-storage)
that reference GeoParquet files map relevant columns to the `GEOGRAPHY`
type.

Statistics available in the GeoParquet file (`bbox`, `covering`) aren't
used to accelerate queries over an external table.

### Coordinate systems and edges

In geospatial analytics, points are positions on the surface of a WGS84 spheroid,
expressed as longitude and geodetic latitude. An edge is a spherical geodesic
between two endpoints. (That is, edges are the shortest path on the surface of
a sphere.)

The WKT format does not provide a coordinate system. When loading WKT data,
geospatial analytics assumes the data uses WGS84 coordinates with spherical edges.
Make sure your source data matches that coordinate system, unless the
geographies are small enough that the difference between spherical and planar
edges can be ignored.

GeoJSON explicitly uses WGS84 coordinates with planar edges. When loading
GeoJSON data, geospatial analytics converts planar edges to spherical edges.
Geospatial analytics adds additional points to the line as necessary, so that the
converted sequence of edges remains within 10 meters of the original line. This
process is known as *tessellation* or *non-uniform densification*. You cannot
directly control the tessellation process.

To load geographies with spherical edges, use WKT. To load geographies with
planar edges, often called *geometries* , it's simplest to use GeoJSON. However,
if your geometry data is already in WKT format, another option is to load the
data as a `STRING` type and then use the
[`ST_GEOGFROMTEXT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_geogfromtext_signature2)
function to convert to `GEOGRAPHY` values. Set the `planar` parameter to `TRUE`
to interpret the data as planar.

GeoParquet files include metadata about the coordinate system and edges that
were used to create the data. When reading GeoParquet files with planar edges,
geospatial analytics converts planar edges to spherical edges. GeoParquet files
with coordinate systems other than WGS84 are rejected.

When choosing an interchange format, be sure to understand the coordinate system
used by your source data. Most systems either explicitly support parsing
geography (as opposed to geometry) from WKT, or else they assume planar edges.

Your coordinates should be longitude first, latitude second. If the geography
has any long segments or edges then they must be tessellated, because
geospatial analytics interprets them as spherical geodesics, which may not
correspond to the coordinate system where your data originated.

### Polygon orientation

On a sphere, every polygon has a complementary polygon. For example, a polygon
that describes the Earth's continents would have a complementary polygon
that describes the Earth's oceans. Because the two polygons are described by the
same boundary rings, rules are required to resolve the ambiguity around which
of the two polygons is described by a given WKT string.

When you load WKT and WKB strings from files or by using streaming ingestion,
geospatial analytics assumes the polygons in the input are oriented as follows:
If you traverse the boundary of the polygon in the order of the input
vertices, the interior of the polygon is on the left. Geospatial analytics uses
the same rule when exporting geography objects to WKT and WKB strings.

If you use the [`ST_GEOGFROMTEXT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_geogfromtext)
function to convert a WKT string to a `GEOGRAPHY` value, the `oriented`
parameter specifies how the function determines the polygon:

- `FALSE`: Interpret the input as the polygon with the smaller area. This is the
  default behavior.

- `TRUE`: Use the left-hand orientation rule described previously. This option
  allows you to load polygons with an area larger than a hemisphere.

Because GeoJSON strings are defined on a planar map, the orientation can be
determined without ambiguity, even if the input does not follow the orientation
rule defined in the GeoJSON format specification,
[RFC 7946](https://tools.ietf.org/html/rfc7946).

### Handling improperly formatted spatial data

When you load spatial data from other tools into BigQuery, you
might encounter conversion errors due to invalid WKT or GeoJSON data. For
example, an error such as `Edge K has duplicate vertex with edge N` indicates
that the polygon has duplicate vertices (besides the first and last).

To avoid formatting issues, you can use a function that generates
standards-compliant output. For example, when you export data from PostGIS, you
can use the PostGIS `ST_MakeValid` function to standardize the output.
Alternatively, import your data as text and then convert it by calling
[`ST_GEOGFROMTEXT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_geogfromtext_signature2)
or [`ST_GEOGFROMGEOJSON`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_geogfromgeojson)
with the `make_valid` parameter. When `make_valid` is `TRUE`, these functions
attempt to repair invalid polygons.

To find or to ignore the improperly formatted data, use the `SAFE` function
prefix to output the problematic data. For example, the following query
uses the `SAFE` prefix to retrieve improperly formatted spatial data.

```googlesql
SELECT
  geojson AS bad_geojson
FROM
  mytable
WHERE
  geojson IS NOT NULL
  AND SAFE.ST_GEOGFROMGEOJSON(geojson) IS NULL
```

### Constraints

Geospatial analytics does not support the following features in geospatial
formats:

- Three-dimensional geometries. This includes the "Z" suffix in the WKT format, and the altitude coordinate in the GeoJSON format.
- Linear reference systems. This includes the "M" suffix in WKT format.
- WKT geometry objects other than geometry primitives or multipart geometries. In particular, geospatial analytics supports only Point, MultiPoint, LineString, MultiLineString, Polygon, MultiPolygon, and GeometryCollection.

See
[`ST_GEOGFROMGEOJSON`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_geogfromgeojson)
and
[`ST_GEOGFROMTEXT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_geogfromtext)
for constraints specific to GeoJSON and WKT input formats.

## Integrate geospatial raster data with Google Earth Engine

Geospatial insights are often presented as grid-based, or *raster* , data. Raster
data organizes regionally continuous data, such as satellite imagery, weather
forecasts, and land cover, into a grid of pixels. Although
BigQuery primarily specializes in tabular vector data,
representing features with boundaries and points, you can integrate raster data
into your BigQuery analyses by using the
[`ST_REGIONSTATS` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_regionstats).
This function uses Earth Engine, Google's raster analysis
platform, to perform computations and aggregations on raster data
for enhanced geospatial analysis. For more information, see
[Work with raster data](https://docs.cloud.google.com/bigquery/docs/raster-data).

For information about exporting Earth Engine data to
BigQuery, see
[Exporting to BigQuery](https://developers.google.com/earth-engine/guides/exporting_to_bigquery).
For more information about integrations between Earth Engine and
BigQuery, see
[BigQuery integration](https://developers.google.com/earth-engine/guides/bigquery_integrations)
in the Earth Engine documentation.

## Transforming geospatial data

If your table contains separate columns for longitude and latitude, you can
transform the values into geographies by using GoogleSQL
[geography functions](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions)
such as [`ST_GEOGPOINT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_geogpoint).
For example, if you have two `DOUBLE` columns for longitude and latitude, you
can create a geography column with the following query:

```googlesql
SELECT
  *,
  ST_GEOGPOINT(longitude, latitude) AS g
FROM
  mytable
```

BigQuery can convert WKT and GeoJSON strings to geography types.
If your data is in another format such as Shapefiles, use an external tool to
convert the data to a supported input file format, such as a CSV file, with
`GEOGRAPHY` columns encoded as WKT or GeoJSON strings.

## Partitioning and clustering geospatial data

You can [partition](https://docs.cloud.google.com/bigquery/docs/partitioned-tables) and
[cluster](https://docs.cloud.google.com/bigquery/docs/clustered-tables) tables that contain `GEOGRAPHY`
columns. You can use a `GEOGRAPHY` column as a clustering column, but you cannot
use a `GEOGRAPHY` column as a partitioning column.

If you store `GEOGRAPHY` data in a table and your queries filter data by using a
spatial predicate, ensure that the table is clustered by the `GEOGRAPHY` column.
This typically improves query performance and might reduce cost. A spatial
predicate calls a boolean geography function and has a `GEOGRAPHY` column as one
of the arguments. The following sample shows a spatial predicate that uses the
`ST_DWITHIN` function:

```googlesql
WHERE ST_DWITHIN(geo, ST_GeogPoint(longitude, latitude), 100)
```

## Using JOINs with spatial data

Spatial JOINs are joins of two tables with a predicate geographic function in
the `WHERE` clause. For example:

```googlesql
-- how many stations within 1 mile range of each zip code?
SELECT
    zip_code AS zip,
    ANY_VALUE(zip_code_geom) AS polygon,
    COUNT(*) AS bike_stations
FROM
    `bigquery-public-data.new_york.citibike_stations` AS bike_stations,
    `bigquery-public-data.geo_us_boundaries.zip_codes` AS zip_codes
WHERE ST_DWITHIN(
         zip_codes.zip_code_geom,
         ST_GEOGPOINT(bike_stations.longitude, bike_stations.latitude),
         1609.34)
GROUP BY zip
ORDER BY bike_stations DESC
```

Spatial joins perform better when your geography data is persisted. The example
above creates the geography values in the query. It is more performant to store
the geography values in a BigQuery table.

For example, the following query retrieves longitude, latitude pairs and
converts them to geographic points. When you run this query, you specify a new
destination table to store the query results:

```googlesql
SELECT
  *,
  ST_GEOGPOINT(pLongitude, pLatitude) AS p
FROM
  mytable
```

BigQuery implements optimized spatial JOINs for INNER JOIN and
CROSS JOIN operators with the following GoogleSQL predicate functions:

- [`ST_DWITHIN`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_dwithin)
- [`ST_INTERSECTS`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_intersects)
- [`ST_CONTAINS`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_contains)
- [`ST_WITHIN`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_within)
- [`ST_COVERS`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_covers)
- [`ST_COVEREDBY`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_coveredby)
- [`ST_EQUALS`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_equals)
- [`ST_TOUCHES`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_touches)

Spatial joins are not optimized:

- For `LEFT`, `RIGHT` or `FULL OUTER` joins
- In cases involving anti-joins
- When the spatial predicate is negated

A `JOIN` that uses the `ST_DWITHIN` predicate is optimized only when
the distance parameter is a constant expression.

## Exporting spatial data

When you export spatial data from BigQuery, `GEOGRAPHY` column
values are always formatted as WKT strings. To export data in GeoJSON format,
use the [`ST_ASGEOJSON`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_asgeojson)
function.

If the tools you're using to analyze the exported data don't understand the
`GEOGRAPHY` data type, you can convert the column values to strings using a
geographic function such as
[`ST_ASTEXT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_astext)
or [`ST_ASGEOJSON`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_asgeojson).
Geospatial analytics adds additional points to the line where necessary so that
the converted sequence of edges remains within 10 meters of the original
geodesic line.

For example, the following query uses `ST_ASGEOJSON` to convert GeoJSON values
to strings.

```googlesql
SELECT
  ST_ASGEOJSON(ST_MAKELINE(ST_GEOGPOINT(1,1), ST_GEOGPOINT(3,2)))
```

The resulting data would look like the following:

```bash
{ "type": "LineString", "coordinates": [ [1, 1], [1.99977145571783, 1.50022838764041], [2.49981908082299, 1.75018082434274], [3, 2] ] }
```

The GeoJSON line has two additional points. Geospatial analytics adds
these points so that the GeoJSON line closely follows the same path on the
ground as the original line.

## What's next

- To get started with geospatial analytics, see [Getting started with geospatial analytics for data analysts](https://docs.cloud.google.com/bigquery/docs/geospatial-get-started).
- To learn more about visualization options for geospatial analytics, see [Visualizing geospatial data](https://docs.cloud.google.com/bigquery/docs/geospatial-visualize).
- For documentation on GoogleSQL functions in geospatial analytics, see [Geography functions in GoogleSQL](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions).