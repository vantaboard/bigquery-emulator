# Work with time series data

This document describes how to use SQL functions to support time series
analysis.

## Introduction

A time series is a sequence of data points, each consisting of a time and a
value associated with that time. Usually, a time series also has an identifier,
which uniquely names the time series.

In relational databases, a time series is modeled as a table with the following
groups of columns:

- Time column
- Might have partitioning columns, for example, zip code
- One or more value columns, or a `STRUCT` type combining multiple values, for example, temperature and AQI

The following is an example of time series data modeled as a table:

![Time series table example.](https://docs.cloud.google.com/static/bigquery/images/time-series-table-example.png)

## Aggregate a time series

In time series analysis, time aggregation is an aggregation performed along the
time axis.

You can perform time aggregation in BigQuery with the help of time
bucketing functions ([`TIMESTAMP_BUCKET`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/time-series-functions#timestamp_bucket),
[`DATE_BUCKET`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/time-series-functions#date_bucket),
and [`DATETIME_BUCKET`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/time-series-functions#datetime_bucket)).
Time bucketing functions map input time values to the bucket they belong to.

Typically, time aggregation is performed to combine multiple data points in a
time window into a single data point, using an aggregation function, such as
`AVG`, `MIN`, `MAX`, `COUNT`, or `SUM`. For example, 15-minute average
request latency, daily minimum and maximum temperatures, and daily number of
taxi trips.

For the queries in this section, create a table called
`mydataset.environmental_data_hourly`:

    CREATE OR REPLACE TABLE mydataset.environmental_data_hourly AS
    SELECT * FROM UNNEST(
      ARRAY<STRUCT<zip_code INT64, time TIMESTAMP, aqi INT64, temperature INT64>>[
        STRUCT(60606, TIMESTAMP '2020-09-08 00:30:51', 22, 66),
        STRUCT(60606, TIMESTAMP '2020-09-08 01:32:10', 23, 63),
        STRUCT(60606, TIMESTAMP '2020-09-08 02:30:35', 22, 60),
        STRUCT(60606, TIMESTAMP '2020-09-08 03:29:39', 21, 58),
        STRUCT(60606, TIMESTAMP '2020-09-08 04:33:05', 21, 59),
        STRUCT(60606, TIMESTAMP '2020-09-08 05:32:01', 21, 57),
        STRUCT(60606, TIMESTAMP '2020-09-08 06:31:14', 22, 56),
        STRUCT(60606, TIMESTAMP '2020-09-08 07:31:06', 28, 55),
        STRUCT(60606, TIMESTAMP '2020-09-08 08:29:59', 30, 55),
        STRUCT(60606, TIMESTAMP '2020-09-08 09:29:34', 31, 55),
        STRUCT(60606, TIMESTAMP '2020-09-08 10:31:24', 38, 56),
        STRUCT(60606, TIMESTAMP '2020-09-08 11:31:24', 38, 56),
        STRUCT(60606, TIMESTAMP '2020-09-08 12:32:38', 38, 57),
        STRUCT(60606, TIMESTAMP '2020-09-08 13:29:59', 38, 56),
        STRUCT(60606, TIMESTAMP '2020-09-08 14:31:22', 43, 59),
        STRUCT(60606, TIMESTAMP '2020-09-08 15:31:38', 42, 63),
        STRUCT(60606, TIMESTAMP '2020-09-08 16:34:22', 43, 65),
        STRUCT(60606, TIMESTAMP '2020-09-08 17:33:23', 42, 68),
        STRUCT(60606, TIMESTAMP '2020-09-08 18:28:47', 36, 69),
        STRUCT(60606, TIMESTAMP '2020-09-08 19:30:28', 34, 67),
        STRUCT(60606, TIMESTAMP '2020-09-08 20:30:53', 29, 67),
        STRUCT(60606, TIMESTAMP '2020-09-08 21:32:28', 27, 67),
        STRUCT(60606, TIMESTAMP '2020-09-08 22:31:45', 25, 65),
        STRUCT(60606, TIMESTAMP '2020-09-08 23:31:02', 22, 63),
        STRUCT(94105, TIMESTAMP '2020-09-08 00:07:11', 60, 74),
        STRUCT(94105, TIMESTAMP '2020-09-08 01:07:24', 61, 73),
        STRUCT(94105, TIMESTAMP '2020-09-08 02:08:07', 60, 71),
        STRUCT(94105, TIMESTAMP '2020-09-08 03:11:05', 69, 69),
        STRUCT(94105, TIMESTAMP '2020-09-08 04:07:26', 72, 67),
        STRUCT(94105, TIMESTAMP '2020-09-08 05:08:11', 70, 66),
        STRUCT(94105, TIMESTAMP '2020-09-08 06:07:30', 68, 65),
        STRUCT(94105, TIMESTAMP '2020-09-08 07:07:10', 77, 64),
        STRUCT(94105, TIMESTAMP '2020-09-08 08:06:35', 81, 64),
        STRUCT(94105, TIMESTAMP '2020-09-08 09:10:18', 82, 63),
        STRUCT(94105, TIMESTAMP '2020-09-08 10:08:10', 107, 62),
        STRUCT(94105, TIMESTAMP '2020-09-08 11:08:01', 115, 62),
        STRUCT(94105, TIMESTAMP '2020-09-08 12:07:39', 120, 62),
        STRUCT(94105, TIMESTAMP '2020-09-08 13:06:03', 125, 61),
        STRUCT(94105, TIMESTAMP '2020-09-08 14:08:37', 129, 62),
        STRUCT(94105, TIMESTAMP '2020-09-08 15:09:19', 150, 62),
        STRUCT(94105, TIMESTAMP '2020-09-08 16:06:39', 151, 62),
        STRUCT(94105, TIMESTAMP '2020-09-08 17:08:01', 155, 63),
        STRUCT(94105, TIMESTAMP '2020-09-08 18:09:23', 154, 64),
        STRUCT(94105, TIMESTAMP '2020-09-08 19:08:43', 151, 67),
        STRUCT(94105, TIMESTAMP '2020-09-08 20:07:19', 150, 69),
        STRUCT(94105, TIMESTAMP '2020-09-08 21:07:37', 148, 72),
        STRUCT(94105, TIMESTAMP '2020-09-08 22:08:01', 143, 76),
        STRUCT(94105, TIMESTAMP '2020-09-08 23:08:41', 137, 75)
    ]);

One interesting observation about the preceding data is that measurements are
taken at arbitrary time periods, known as *unaligned time series*. Aggregation
is one of the ways by which a time series can be aligned.

### Get a 3-hour average

The following query computes a 3-hour average air quality index (AQI) and
temperature for each zip code. The `TIMESTAMP_BUCKET` function performs time
aggregation by assigning each time value to a particular day.

    SELECT
      TIMESTAMP_BUCKET(time, INTERVAL 3 HOUR) AS time,
      zip_code,
      CAST(AVG(aqi) AS INT64) AS aqi,
      CAST(AVG(temperature) AS INT64) AS temperature
    FROM mydataset.environmental_data_hourly
    GROUP BY zip_code, time
    ORDER BY zip_code, time;

    /*---+---+---+---+
     |        time         | zip_code | aqi | temperature |
     +---+---+---+---+
     | 2020-09-08 00:00:00 |    60606 |  22 |          63 |
     | 2020-09-08 03:00:00 |    60606 |  21 |          58 |
     | 2020-09-08 06:00:00 |    60606 |  27 |          55 |
     | 2020-09-08 09:00:00 |    60606 |  36 |          56 |
     | 2020-09-08 12:00:00 |    60606 |  40 |          57 |
     | 2020-09-08 15:00:00 |    60606 |  42 |          65 |
     | 2020-09-08 18:00:00 |    60606 |  33 |          68 |
     | 2020-09-08 21:00:00 |    60606 |  25 |          65 |
     | 2020-09-08 00:00:00 |    94105 |  60 |          73 |
     | 2020-09-08 03:00:00 |    94105 |  70 |          67 |
     | 2020-09-08 06:00:00 |    94105 |  75 |          64 |
     | 2020-09-08 09:00:00 |    94105 | 101 |          62 |
     | 2020-09-08 12:00:00 |    94105 | 125 |          62 |
     | 2020-09-08 15:00:00 |    94105 | 152 |          62 |
     | 2020-09-08 18:00:00 |    94105 | 152 |          67 |
     | 2020-09-08 21:00:00 |    94105 | 143 |          74 |
     +---+---+---+---*/

### Get a 3-hour minimum and maximum value

In the following query, you compute 3-hour minimum and maximum temperatures for
each zip code:

    SELECT
      TIMESTAMP_BUCKET(time, INTERVAL 3 HOUR) AS time,
      zip_code,
      MIN(temperature) AS temperature_min,
      MAX(temperature) AS temperature_max,
    FROM mydataset.environmental_data_hourly
    GROUP BY zip_code, time
    ORDER BY zip_code, time;

    /*---+---+---+---+
     |        time         | zip_code | temperature_min | temperature_max |
     +---+---+---+---+
     | 2020-09-08 00:00:00 |    60606 |              60 |              66 |
     | 2020-09-08 03:00:00 |    60606 |              57 |              59 |
     | 2020-09-08 06:00:00 |    60606 |              55 |              56 |
     | 2020-09-08 09:00:00 |    60606 |              55 |              56 |
     | 2020-09-08 12:00:00 |    60606 |              56 |              59 |
     | 2020-09-08 15:00:00 |    60606 |              63 |              68 |
     | 2020-09-08 18:00:00 |    60606 |              67 |              69 |
     | 2020-09-08 21:00:00 |    60606 |              63 |              67 |
     | 2020-09-08 00:00:00 |    94105 |              71 |              74 |
     | 2020-09-08 03:00:00 |    94105 |              66 |              69 |
     | 2020-09-08 06:00:00 |    94105 |              64 |              65 |
     | 2020-09-08 09:00:00 |    94105 |              62 |              63 |
     | 2020-09-08 12:00:00 |    94105 |              61 |              62 |
     | 2020-09-08 15:00:00 |    94105 |              62 |              63 |
     | 2020-09-08 18:00:00 |    94105 |              64 |              69 |
     | 2020-09-08 21:00:00 |    94105 |              72 |              76 |
     +---+---+---+---*/

### Get a 3-hour average with custom alignment

When you perform time series aggregation, you use a specific alignment for time
series windows, either implicitly or explicitly. The preceding queries used
implicit alignment, which produced buckets that started at times like
`00:00:00`, `03:00:00`, and `06:00:00`. To explicitly set this alignment in
the `TIMESTAMP_BUCKET` function, pass an optional argument that specifies the
origin.

In the following query, the origin is set as `2020-01-01 02:00:00`. This changes
the alignment and produces buckets that start at times like `02:00:00`,
`05:00:00`, and `08:00:00`:

    SELECT
      TIMESTAMP_BUCKET(time, INTERVAL 3 HOUR, TIMESTAMP '2020-01-01 02:00:00') AS time,
      zip_code,
      CAST(AVG(aqi) AS INT64) AS aqi,
      CAST(AVG(temperature) AS INT64) AS temperature
    FROM mydataset.environmental_data_hourly
    GROUP BY zip_code, time
    ORDER BY zip_code, time;

    /*---+---+---+---+
     |        time         | zip_code | aqi | temperature |
     +---+---+---+---+
     | 2020-09-07 23:00:00 |    60606 |  23 |          65 |
     | 2020-09-08 02:00:00 |    60606 |  21 |          59 |
     | 2020-09-08 05:00:00 |    60606 |  24 |          56 |
     | 2020-09-08 08:00:00 |    60606 |  33 |          55 |
     | 2020-09-08 11:00:00 |    60606 |  38 |          56 |
     | 2020-09-08 14:00:00 |    60606 |  43 |          62 |
     | 2020-09-08 17:00:00 |    60606 |  37 |          68 |
     | 2020-09-08 20:00:00 |    60606 |  27 |          66 |
     | 2020-09-08 23:00:00 |    60606 |  22 |          63 |
     | 2020-09-07 23:00:00 |    94105 |  61 |          74 |
     | 2020-09-08 02:00:00 |    94105 |  67 |          69 |
     | 2020-09-08 05:00:00 |    94105 |  72 |          65 |
     | 2020-09-08 08:00:00 |    94105 |  90 |          63 |
     | 2020-09-08 11:00:00 |    94105 | 120 |          62 |
     | 2020-09-08 14:00:00 |    94105 | 143 |          62 |
     | 2020-09-08 17:00:00 |    94105 | 153 |          65 |
     | 2020-09-08 20:00:00 |    94105 | 147 |          72 |
     | 2020-09-08 23:00:00 |    94105 | 137 |          75 |
     +---+---+---+---*/

## Aggregate a time series with gap filling

Sometimes after you aggregate a time series, the data might have gaps that need to
be filled with some values for further analysis or presentation of the data.
The technique used to fill in those gaps is called *gap filling* . In
BigQuery, you can use the [`GAP_FILL`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/time-series-functions#gap_fill)
table function for filling gaps in time series data, using one of the provided
gap-filling methods:

- NULL, also known as constant
- LOCF, last observation carried forward
- Linear, linear interpolation between the two neighboring data points

For the queries in this section, create a table called
`mydataset.environmental_data_hourly_with_gaps`, which is based on the data used
in the preceding section, but with gaps in it. In the real world scenarios, data
could have missing data points due to a short-term weather station malfunction.

    CREATE OR REPLACE TABLE mydataset.environmental_data_hourly_with_gaps AS
    SELECT * FROM UNNEST(
      ARRAY<STRUCT<zip_code INT64, time TIMESTAMP, aqi INT64, temperature INT64>>[
        STRUCT(60606, TIMESTAMP '2020-09-08 00:30:51', 22, 66),
        STRUCT(60606, TIMESTAMP '2020-09-08 01:32:10', 23, 63),
        STRUCT(60606, TIMESTAMP '2020-09-08 02:30:35', 22, 60),
        STRUCT(60606, TIMESTAMP '2020-09-08 03:29:39', 21, 58),
        STRUCT(60606, TIMESTAMP '2020-09-08 04:33:05', 21, 59),
        STRUCT(60606, TIMESTAMP '2020-09-08 05:32:01', 21, 57),
        STRUCT(60606, TIMESTAMP '2020-09-08 06:31:14', 22, 56),
        STRUCT(60606, TIMESTAMP '2020-09-08 07:31:06', 28, 55),
        STRUCT(60606, TIMESTAMP '2020-09-08 08:29:59', 30, 55),
        STRUCT(60606, TIMESTAMP '2020-09-08 09:29:34', 31, 55),
        STRUCT(60606, TIMESTAMP '2020-09-08 10:31:24', 38, 56),
        STRUCT(60606, TIMESTAMP '2020-09-08 11:31:24', 38, 56),
        -- No data points between hours 12 and 15.
        STRUCT(60606, TIMESTAMP '2020-09-08 16:34:22', 43, 65),
        STRUCT(60606, TIMESTAMP '2020-09-08 17:33:23', 42, 68),
        STRUCT(60606, TIMESTAMP '2020-09-08 18:28:47', 36, 69),
        STRUCT(60606, TIMESTAMP '2020-09-08 19:30:28', 34, 67),
        STRUCT(60606, TIMESTAMP '2020-09-08 20:30:53', 29, 67),
        STRUCT(60606, TIMESTAMP '2020-09-08 21:32:28', 27, 67),
        STRUCT(60606, TIMESTAMP '2020-09-08 22:31:45', 25, 65),
        STRUCT(60606, TIMESTAMP '2020-09-08 23:31:02', 22, 63),
        STRUCT(94105, TIMESTAMP '2020-09-08 00:07:11', 60, 74),
        STRUCT(94105, TIMESTAMP '2020-09-08 01:07:24', 61, 73),
        STRUCT(94105, TIMESTAMP '2020-09-08 02:08:07', 60, 71),
        STRUCT(94105, TIMESTAMP '2020-09-08 03:11:05', 69, 69),
        STRUCT(94105, TIMESTAMP '2020-09-08 04:07:26', 72, 67),
        STRUCT(94105, TIMESTAMP '2020-09-08 05:08:11', 70, 66),
        STRUCT(94105, TIMESTAMP '2020-09-08 06:07:30', 68, 65),
        STRUCT(94105, TIMESTAMP '2020-09-08 07:07:10', 77, 64),
        STRUCT(94105, TIMESTAMP '2020-09-08 08:06:35', 81, 64),
        STRUCT(94105, TIMESTAMP '2020-09-08 09:10:18', 82, 63),
        STRUCT(94105, TIMESTAMP '2020-09-08 10:08:10', 107, 62),
        STRUCT(94105, TIMESTAMP '2020-09-08 11:08:01', 115, 62),
        STRUCT(94105, TIMESTAMP '2020-09-08 12:07:39', 120, 62),
        STRUCT(94105, TIMESTAMP '2020-09-08 13:06:03', 125, 61),
        STRUCT(94105, TIMESTAMP '2020-09-08 14:08:37', 129, 62),
        -- No data points between hours 15 and 18.
        STRUCT(94105, TIMESTAMP '2020-09-08 19:08:43', 151, 67),
        STRUCT(94105, TIMESTAMP '2020-09-08 20:07:19', 150, 69),
        STRUCT(94105, TIMESTAMP '2020-09-08 21:07:37', 148, 72),
        STRUCT(94105, TIMESTAMP '2020-09-08 22:08:01', 143, 76),
        STRUCT(94105, TIMESTAMP '2020-09-08 23:08:41', 137, 75)
    ]);

### Get a 3-hour average (include gaps)

The following query computes 3-hour average AQI and temperature for each zip
code:

    SELECT
      TIMESTAMP_BUCKET(time, INTERVAL 3 HOUR) AS time,
      zip_code,
      CAST(AVG(aqi) AS INT64) AS aqi,
      CAST(AVG(temperature) AS INT64) AS temperature
    FROM mydataset.environmental_data_hourly_with_gaps
    GROUP BY zip_code, time
    ORDER BY zip_code, time;

    /*---+---+---+---+
     |        time         | zip_code | aqi | temperature |
     +---+---+---+---+
     | 2020-09-08 00:00:00 |    60606 |  22 |          63 |
     | 2020-09-08 03:00:00 |    60606 |  21 |          58 |
     | 2020-09-08 06:00:00 |    60606 |  27 |          55 |
     | 2020-09-08 09:00:00 |    60606 |  36 |          56 |
     | 2020-09-08 15:00:00 |    60606 |  43 |          67 |
     | 2020-09-08 18:00:00 |    60606 |  33 |          68 |
     | 2020-09-08 21:00:00 |    60606 |  25 |          65 |
     | 2020-09-08 00:00:00 |    94105 |  60 |          73 |
     | 2020-09-08 03:00:00 |    94105 |  70 |          67 |
     | 2020-09-08 06:00:00 |    94105 |  75 |          64 |
     | 2020-09-08 09:00:00 |    94105 | 101 |          62 |
     | 2020-09-08 12:00:00 |    94105 | 125 |          62 |
     | 2020-09-08 18:00:00 |    94105 | 151 |          68 |
     | 2020-09-08 21:00:00 |    94105 | 143 |          74 |
     +---+---+---+---*/

Note how the output has gaps at certain time intervals. For example, the time
series for zip code `60606` doesn't have a data point at `2020-09-08 12:00:00`,
and the time series for zip code `94105` doesn't have a data point at
`2020-09-08 15:00:00`.

### Get a 3-hour average (fill gaps)

Use the query from the previous section and add the `GAP_FILL` function to fill
the gaps:

    WITH aggregated_3_hr AS (
      SELECT
        TIMESTAMP_BUCKET(time, INTERVAL 3 HOUR) AS time,
        zip_code,
        CAST(AVG(aqi) AS INT64) AS aqi,
        CAST(AVG(temperature) AS INT64) AS temperature
       FROM mydataset.environmental_data_hourly_with_gaps
       GROUP BY zip_code, time)

    SELECT *
    FROM GAP_FILL(
      TABLE aggregated_3_hr,
      ts_column => 'time',
      bucket_width => INTERVAL 3 HOUR,
      partitioning_columns => ['zip_code']
    )
    ORDER BY zip_code, time;

    /*---+---+---+---+
     |        time         | zip_code | aqi  | temperature |
     +---+---+---+---+
     | 2020-09-08 00:00:00 |    60606 |   22 |          63 |
     | 2020-09-08 03:00:00 |    60606 |   21 |          58 |
     | 2020-09-08 06:00:00 |    60606 |   27 |          55 |
     | 2020-09-08 09:00:00 |    60606 |   36 |          56 |
     | 2020-09-08 12:00:00 |    60606 | NULL |        NULL |
     | 2020-09-08 15:00:00 |    60606 |   43 |          67 |
     | 2020-09-08 18:00:00 |    60606 |   33 |          68 |
     | 2020-09-08 21:00:00 |    60606 |   25 |          65 |
     | 2020-09-08 00:00:00 |    94105 |   60 |          73 |
     | 2020-09-08 03:00:00 |    94105 |   70 |          67 |
     | 2020-09-08 06:00:00 |    94105 |   75 |          64 |
     | 2020-09-08 09:00:00 |    94105 |  101 |          62 |
     | 2020-09-08 12:00:00 |    94105 |  125 |          62 |
     | 2020-09-08 15:00:00 |    94105 | NULL |        NULL |
     | 2020-09-08 18:00:00 |    94105 |  151 |          68 |
     | 2020-09-08 21:00:00 |    94105 |  143 |          74 |
     +---+---+---+---*/

The output table now contains a missing row at `2020-09-08 12:00:00` for zip
code `60606` and at `2020-09-08 15:00:00` for zip code `94105`, with `NULL`
values in the corresponding metric columns. Since you didn't specify any
gap-filling method, `GAP_FILL` used the default gap-filling method, NULL.

### Fill gaps with linear and LOCF gap filling

In the following query, the `GAP_FILL` function is used with the LOCF
gap-filling method for the `aqi` column and linear interpolation for the
`temperature` column:

    WITH aggregated_3_hr AS (
      SELECT
        TIMESTAMP_BUCKET(time, INTERVAL 3 HOUR) AS time,
        zip_code,
        CAST(AVG(aqi) AS INT64) AS aqi,
        CAST(AVG(temperature) AS INT64) AS temperature
       FROM mydataset.environmental_data_hourly_with_gaps
       GROUP BY zip_code, time)

    SELECT *
    FROM GAP_FILL(
      TABLE aggregated_3_hr,
      ts_column => 'time',
      bucket_width => INTERVAL 3 HOUR,
      partitioning_columns => ['zip_code'],
      value_columns => [
        ('aqi', 'locf'),
        ('temperature', 'linear')
      ]
    )
    ORDER BY zip_code, time;

    /*---+---+---+---+
     |        time         | zip_code | aqi | temperature |
     +---+---+---+---+
     | 2020-09-08 00:00:00 |    60606 |  22 |          63 |
     | 2020-09-08 03:00:00 |    60606 |  21 |          58 |
     | 2020-09-08 06:00:00 |    60606 |  27 |          55 |
     | 2020-09-08 09:00:00 |    60606 |  36 |          56 |
     | 2020-09-08 12:00:00 |    60606 |  36 |          62 |
     | 2020-09-08 15:00:00 |    60606 |  43 |          67 |
     | 2020-09-08 18:00:00 |    60606 |  33 |          68 |
     | 2020-09-08 21:00:00 |    60606 |  25 |          65 |
     | 2020-09-08 00:00:00 |    94105 |  60 |          73 |
     | 2020-09-08 03:00:00 |    94105 |  70 |          67 |
     | 2020-09-08 06:00:00 |    94105 |  75 |          64 |
     | 2020-09-08 09:00:00 |    94105 | 101 |          62 |
     | 2020-09-08 12:00:00 |    94105 | 125 |          62 |
     | 2020-09-08 15:00:00 |    94105 | 125 |          65 |
     | 2020-09-08 18:00:00 |    94105 | 151 |          68 |
     | 2020-09-08 21:00:00 |    94105 | 143 |          74 |
     +---+---+---+---*/

In this query, the first gap-filled row has `aqi` value `36`, which is taken
from the previous data point of this time series (zip code `60606`) at
`2020-09-08 09:00:00`. The `temperature` value `62` is a result of linear
interpolation between data points `2020-09-08 09:00:00` and
`2020-09-08 15:00:00`. The other missing row was created in a similar way - `aqi`
value `125` was carried over from the previous data point of this time series
(zip code `94105`), and the temperature value `65` is a result of linear
interpolation between the previous and the next available data points.

## Align a time series with gap filling

Time series can be aligned or unaligned. A time series is aligned when data
points only occur at regular intervals.

In the real world, at the time of collection, time series are rarely aligned
and usually require some further processing to align them.

For example, consider IoT devices that send their metrics to a centralized
collector every minute. It would be unreasonable to expect the devices to send
their metrics at exactly the same instants of time. Usually, each device sends
its metrics with the same frequency (period) but with different time offset
(alignment). The following diagram illustrates this example. You can see each
device sending its data with a one-minute interval with some instances of
missing data (Device 3 at `9:36:39` ) and delayed data (Device 1 at `9:37:28`).

![Align time series example](https://docs.cloud.google.com/static/bigquery/images/align-time-series.png)

You can perform *time series alignment* on unaligned data, using
[time aggregation](https://docs.cloud.google.com/bigquery/docs/working-with-time-series#aggregate_a_time_series). This is helpful if you
want to change the sampling period of the time series, such as changing from
the original 1-minute sampling period to a 15-minute period. You can align data
for further time series processing, such as joining the time series data, or for
display purposes (such as graphing).

You can use the `GAP_FILL` table function with LOCF or linear gap-filling
methods to perform time series alignment. The idea is to use `GAP_FILL` with the
selected output period and alignment
(controlled by the optional origin argument). The result of the operation is a
table with aligned time series, where values for each data point are derived
from the input time series with the gap-filling method used for that particular
value column (LOCF or linear).

Create a table `mydataset.device_data`, which resembles the previous illustration:

    CREATE OR REPLACE TABLE mydataset.device_data AS
    SELECT * FROM UNNEST(
      ARRAY<STRUCT<device_id INT64, time TIMESTAMP, signal INT64, state STRING>>[
        STRUCT(2, TIMESTAMP '2023-11-01 09:35:07', 87, 'ACTIVE'),
        STRUCT(1, TIMESTAMP '2023-11-01 09:35:26', 82, 'ACTIVE'),
        STRUCT(3, TIMESTAMP '2023-11-01 09:35:39', 74, 'INACTIVE'),
        STRUCT(2, TIMESTAMP '2023-11-01 09:36:07', 88, 'ACTIVE'),
        STRUCT(1, TIMESTAMP '2023-11-01 09:36:26', 82, 'ACTIVE'),
        STRUCT(2, TIMESTAMP '2023-11-01 09:37:07', 88, 'ACTIVE'),
        STRUCT(1, TIMESTAMP '2023-11-01 09:37:28', 80, 'ACTIVE'),
        STRUCT(3, TIMESTAMP '2023-11-01 09:37:39', 77, 'ACTIVE'),
        STRUCT(2, TIMESTAMP '2023-11-01 09:38:07', 86, 'ACTIVE'),
        STRUCT(1, TIMESTAMP '2023-11-01 09:38:26', 81, 'ACTIVE'),
        STRUCT(3, TIMESTAMP '2023-11-01 09:38:39', 77, 'ACTIVE')
    ]);

The following is the actual data ordered by `time` and `device_id` columns:

    SELECT * FROM mydataset.device_data ORDER BY time, device_id;

    /*---+---+---+---+
     | device_id |        time         | signal |  state   |
     +---+---+---+---+
     |         2 | 2023-11-01 09:35:07 |     87 | ACTIVE   |
     |         1 | 2023-11-01 09:35:26 |     82 | ACTIVE   |
     |         3 | 2023-11-01 09:35:39 |     74 | INACTIVE |
     |         2 | 2023-11-01 09:36:07 |     88 | ACTIVE   |
     |         1 | 2023-11-01 09:36:26 |     82 | ACTIVE   |
     |         2 | 2023-11-01 09:37:07 |     88 | ACTIVE   |
     |         1 | 2023-11-01 09:37:28 |     80 | ACTIVE   |
     |         3 | 2023-11-01 09:37:39 |     77 | ACTIVE   |
     |         2 | 2023-11-01 09:38:07 |     86 | ACTIVE   |
     |         1 | 2023-11-01 09:38:26 |     81 | ACTIVE   |
     |         3 | 2023-11-01 09:38:39 |     77 | ACTIVE   |
     +---+---+---+---*/

The table contains the time series for each device with two metric columns:

- `signal` - signal level as observed by the device at the time of sampling, represented as an integer value between `0` and `100`.
- `state` - state of the device at the time of sampling, represented as a free form string.

In the following query, the `GAP_FILL` function is used to align the time
series at 1-minute intervals. Note how linear interpolation is used to compute
values for the `signal` column and LOCF for the `state` column. For this example
data, linear interpolation is a suitable choice to compute the output values.

    SELECT *
    FROM GAP_FILL(
      TABLE mydataset.device_data,
      ts_column => 'time',
      bucket_width => INTERVAL 1 MINUTE,
      partitioning_columns => ['device_id'],
      value_columns => [
        ('signal', 'linear'),
        ('state', 'locf')
      ]
    )
    ORDER BY time, device_id;

     /*---+---+---+---+
     |        time         | device_id | signal |  state   |
     +---+---+---+---+
     | 2023-11-01 09:36:00 |         1 |     82 | ACTIVE   |
     | 2023-11-01 09:36:00 |         2 |     88 | ACTIVE   |
     | 2023-11-01 09:36:00 |         3 |     75 | INACTIVE |
     | 2023-11-01 09:37:00 |         1 |     81 | ACTIVE   |
     | 2023-11-01 09:37:00 |         2 |     88 | ACTIVE   |
     | 2023-11-01 09:37:00 |         3 |     76 | INACTIVE |
     | 2023-11-01 09:38:00 |         1 |     81 | ACTIVE   |
     | 2023-11-01 09:38:00 |         2 |     86 | ACTIVE   |
     | 2023-11-01 09:38:00 |         3 |     77 | ACTIVE   |
     +---+---+---+---*/

The output table contains an aligned time series for each device and value
columns (`signal` and `state`), computed using the gap-filling methods specified
in the function call.

## Join time series data

You can join time series data using a windowed join or `AS OF` join.

### Windowed join

Sometimes you need to join two or more tables with time series data. Consider
the following two tables:

- `mydataset.sensor_temperatures`, contains temperature data reported by each sensor every 15 seconds.
- `mydataset.sensor_fuel_rates`, contains the fuel consumption rate measured by each sensor every 15 seconds.

To create these tables, run the following queries:

    CREATE OR REPLACE TABLE mydataset.sensor_temperatures AS
    SELECT * FROM UNNEST(
      ARRAY<STRUCT<sensor_id INT64, ts TIMESTAMP, temp FLOAT64>>[
      (1, TIMESTAMP '2020-01-01 12:00:00.063', 37.1),
      (1, TIMESTAMP '2020-01-01 12:00:15.024', 37.2),
      (1, TIMESTAMP '2020-01-01 12:00:30.032', 37.3),
      (2, TIMESTAMP '2020-01-01 12:00:01.001', 38.1),
      (2, TIMESTAMP '2020-01-01 12:00:15.082', 38.2),
      (2, TIMESTAMP '2020-01-01 12:00:31.009', 38.3)
    ]);

    CREATE OR REPLACE TABLE mydataset.sensor_fuel_rates AS
    SELECT * FROM UNNEST(
      ARRAY<STRUCT<sensor_id INT64, ts TIMESTAMP, rate FLOAT64>>[
        (1, TIMESTAMP '2020-01-01 12:00:11.016', 10.1),
        (1, TIMESTAMP '2020-01-01 12:00:26.015', 10.2),
        (1, TIMESTAMP '2020-01-01 12:00:41.014', 10.3),
        (2, TIMESTAMP '2020-01-01 12:00:08.099', 11.1),
        (2, TIMESTAMP '2020-01-01 12:00:23.087', 11.2),
        (2, TIMESTAMP '2020-01-01 12:00:38.077', 11.3)
    ]);

The following is the actual data from the tables:

    SELECT * FROM mydataset.sensor_temperatures ORDER BY sensor_id, ts;

     /*---+---+---+
     | sensor_id |         ts          | temp |
     +---+---+---+
     |         1 | 2020-01-01 12:00:00 | 37.1 |
     |         1 | 2020-01-01 12:00:15 | 37.2 |
     |         1 | 2020-01-01 12:00:30 | 37.3 |
     |         2 | 2020-01-01 12:00:01 | 38.1 |
     |         2 | 2020-01-01 12:00:15 | 38.2 |
     |         2 | 2020-01-01 12:00:31 | 38.3 |
     +---+---+---*/

    SELECT * FROM mydataset.sensor_fuel_rates ORDER BY sensor_id, ts;

     /*---+---+---+
     | sensor_id |         ts          | rate |
     +---+---+---+
     |         1 | 2020-01-01 12:00:11 | 10.1 |
     |         1 | 2020-01-01 12:00:26 | 10.2 |
     |         1 | 2020-01-01 12:00:41 | 10.3 |
     |         2 | 2020-01-01 12:00:08 | 11.1 |
     |         2 | 2020-01-01 12:00:23 | 11.2 |
     |         2 | 2020-01-01 12:00:38 | 11.3 |
     +---+---+---*/

To check the fuel consumption rate at the temperature reported by each sensor,
you can join the two time series.

Although the data in the two time series is unaligned, it is sampled at the same
interval (15 seconds), therefore such data is a good candidate for windowed
join. Use the [time bucketing functions](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/time-series-functions#datetime_bucket)
to align timestamps used as join keys.

The following queries illustrate how each timestamp can be assigned to 15-second
windows using the [`TIMESTAMP_BUCKET`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/time-series-functions#timestamp_bucket)
function:

    SELECT *, TIMESTAMP_BUCKET(ts, INTERVAL 15 SECOND) ts_window
    FROM mydataset.sensor_temperatures
    ORDER BY sensor_id, ts;

    /*---+---+---+---+
     | sensor_id |         ts          | temp |      ts_window      |
     +---+---+---+---+
     |         1 | 2020-01-01 12:00:00 | 37.1 | 2020-01-01 12:00:00 |
     |         1 | 2020-01-01 12:00:15 | 37.2 | 2020-01-01 12:00:15 |
     |         1 | 2020-01-01 12:00:30 | 37.3 | 2020-01-01 12:00:30 |
     |         2 | 2020-01-01 12:00:01 | 38.1 | 2020-01-01 12:00:00 |
     |         2 | 2020-01-01 12:00:15 | 38.2 | 2020-01-01 12:00:15 |
     |         2 | 2020-01-01 12:00:31 | 38.3 | 2020-01-01 12:00:30 |
     +---+---+---+---*/

    SELECT *, TIMESTAMP_BUCKET(ts, INTERVAL 15 SECOND) ts_window
    FROM mydataset.sensor_fuel_rates
    ORDER BY sensor_id, ts;

    /*---+---+---+---+
     | sensor_id |         ts          | rate |      ts_window      |
     +---+---+---+---+
     |         1 | 2020-01-01 12:00:11 | 10.1 | 2020-01-01 12:00:00 |
     |         1 | 2020-01-01 12:00:26 | 10.2 | 2020-01-01 12:00:15 |
     |         1 | 2020-01-01 12:00:41 | 10.3 | 2020-01-01 12:00:30 |
     |         2 | 2020-01-01 12:00:08 | 11.1 | 2020-01-01 12:00:00 |
     |         2 | 2020-01-01 12:00:23 | 11.2 | 2020-01-01 12:00:15 |
     |         2 | 2020-01-01 12:00:38 | 11.3 | 2020-01-01 12:00:30 |
     +---+---+---+---*/

You can use this concept to join the fuel consumption rate data with the
temperature reported by each sensor:

    SELECT
      t1.sensor_id AS sensor_id,
      t1.ts AS temp_ts,
      t1.temp AS temp,
      t2.ts AS rate_ts,
      t2.rate AS rate
    FROM mydataset.sensor_temperatures t1
    LEFT JOIN mydataset.sensor_fuel_rates t2
    ON TIMESTAMP_BUCKET(t1.ts, INTERVAL 15 SECOND) =
         TIMESTAMP_BUCKET(t2.ts, INTERVAL 15 SECOND)
       AND t1.sensor_id = t2.sensor_id
    ORDER BY sensor_id, temp_ts;

    /*---+---+---+---+---+
     | sensor_id |       temp_ts       | temp |       rate_ts       | rate |
     +---+---+---+---+---+
     |         1 | 2020-01-01 12:00:00 | 37.1 | 2020-01-01 12:00:11 | 10.1 |
     |         1 | 2020-01-01 12:00:15 | 37.2 | 2020-01-01 12:00:26 | 10.2 |
     |         1 | 2020-01-01 12:00:30 | 37.3 | 2020-01-01 12:00:41 | 10.3 |
     |         2 | 2020-01-01 12:00:01 | 38.1 | 2020-01-01 12:00:08 | 11.1 |
     |         2 | 2020-01-01 12:00:15 | 38.2 | 2020-01-01 12:00:23 | 11.2 |
     |         2 | 2020-01-01 12:00:31 | 38.3 | 2020-01-01 12:00:38 | 11.3 |
     +---+---+---+---+---*/

### `AS OF` join

For this section, use the `mydataset.sensor_temperatures` table and create a new
table, `mydataset.sensor_location`.

The `mydataset.sensor_temperatures` table contains temperature data from
different sensors, reported every 15 seconds:

    SELECT * FROM mydataset.sensor_temperatures ORDER BY sensor_id, ts;

    /*---+---+---+
     | sensor_id |         ts          | temp |
     +---+---+---+
     |         1 | 2020-01-01 12:00:00 | 37.1 |
     |         1 | 2020-01-01 12:00:15 | 37.2 |
     |         1 | 2020-01-01 12:00:30 | 37.3 |
     |         2 | 2020-01-01 12:00:45 | 38.1 |
     |         2 | 2020-01-01 12:01:01 | 38.2 |
     |         2 | 2020-01-01 12:01:15 | 38.3 |
     +---+---+---*/

To create `mydataset.sensor_location`, run the following query:

    CREATE OR REPLACE TABLE mydataset.sensor_locations AS
    SELECT * FROM UNNEST(
      ARRAY<STRUCT<sensor_id INT64, ts TIMESTAMP, location GEOGRAPHY>>[
      (1, TIMESTAMP '2020-01-01 11:59:47.063', ST_GEOGPOINT(-122.022, 37.406)),
      (1, TIMESTAMP '2020-01-01 12:00:08.185', ST_GEOGPOINT(-122.021, 37.407)),
      (1, TIMESTAMP '2020-01-01 12:00:28.032', ST_GEOGPOINT(-122.020, 37.405)),
      (2, TIMESTAMP '2020-01-01 07:28:41.239', ST_GEOGPOINT(-122.390, 37.790))
    ]);

    /*---+---+---+
     | sensor_id |         ts          |        location        |
     +---+---+---+
     |         1 | 2020-01-01 11:59:47 | POINT(-122.022 37.406) |
     |         1 | 2020-01-01 12:00:08 | POINT(-122.021 37.407) |
     |         1 | 2020-01-01 12:00:28 |  POINT(-122.02 37.405) |
     |         2 | 2020-01-01 07:28:41 |   POINT(-122.39 37.79) |
     +---+---+---*/

Now join data from `mydataset.sensor_temperatures` with data from
`mydataset.sensor_location`.

In this scenario, you can't use a windowed join, since the temperature data
and location data are not reported at the same interval.

One way to do this in BigQuery is to transform the timestamp data
into a range, using the [`RANGE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/range-functions#range)
data type. The range represents the temporal validity of a row, providing the
start and end time for which the row is valid.

Use the [`LEAD`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/navigation_functions#lead)
window function to find the next data point in the time series, relative to the
current data point, which is also the end-boundary of the temporal validity of
the current row. The following queries demonstrate this, converting location
data to validity ranges:

    WITH locations_ranges AS (
      SELECT
        sensor_id,
        RANGE(ts, LEAD(ts) OVER (PARTITION BY sensor_id ORDER BY ts ASC)) AS ts_range,
        location
      FROM mydataset.sensor_locations
    )
    SELECT * FROM locations_ranges ORDER BY sensor_id, ts_range;

    /*---+---+---+
     | sensor_id |                  ts_range                  |        location        |
     +---+---+---+
     |         1 | [2020-01-01 11:59:47, 2020-01-01 12:00:08) | POINT(-122.022 37.406) |
     |         1 | [2020-01-01 12:00:08, 2020-01-01 12:00:28) | POINT(-122.021 37.407) |
     |         1 |           [2020-01-01 12:00:28, UNBOUNDED) |  POINT(-122.02 37.405) |
     |         2 |           [2020-01-01 07:28:41, UNBOUNDED) |   POINT(-122.39 37.79) |
     +---+---+---*/

Now you can join temperatures data (left) with the location data (right):

    WITH locations_ranges AS (
      SELECT
        sensor_id,
        RANGE(ts, LEAD(ts) OVER (PARTITION BY sensor_id ORDER BY ts ASC)) AS ts_range,
        location
      FROM mydataset.sensor_locations
    )
    SELECT
      t1.sensor_id AS sensor_id,
      t1.ts AS temp_ts,
      t1.temp AS temp,
      t2.location AS location
    FROM mydataset.sensor_temperatures t1
    LEFT JOIN locations_ranges t2
    ON RANGE_CONTAINS(t2.ts_range, t1.ts)
    AND t1.sensor_id = t2.sensor_id
    ORDER BY sensor_id, temp_ts;

    /*---+---+---+---+
     | sensor_id |       temp_ts       | temp |        location        |
     +---+---+---+---+
     |         1 | 2020-01-01 12:00:00 | 37.1 | POINT(-122.022 37.406) |
     |         1 | 2020-01-01 12:00:15 | 37.2 | POINT(-122.021 37.407) |
     |         1 | 2020-01-01 12:00:30 | 37.3 |  POINT(-122.02 37.405) |
     |         2 | 2020-01-01 12:00:01 | 38.1 |   POINT(-122.39 37.79) |
     |         2 | 2020-01-01 12:00:15 | 38.2 |   POINT(-122.39 37.79) |
     |         2 | 2020-01-01 12:00:31 | 38.3 |   POINT(-122.39 37.79) |
     +---+---+---+---*/

## Combine and split range data

In this section, combine range data that have overlapping ranges and split
range data into smaller ranges.

### Combine range data

Tables with range values might have overlapping ranges. In the following
query, the time ranges capture the state of sensors at approximately 5-minute
intervals:

    CREATE OR REPLACE TABLE mydataset.sensor_metrics AS
    SELECT * FROM UNNEST(
      ARRAY<STRUCT<sensor_id INT64, duration RANGE<DATETIME>, flow INT64, spins INT64>>[
      (1, RANGE<DATETIME> "[2020-01-01 12:00:01, 2020-01-01 12:05:23)", 10, 1),
      (1, RANGE<DATETIME> "[2020-01-01 12:05:12, 2020-01-01 12:10:46)", 10, 20),
      (1, RANGE<DATETIME> "[2020-01-01 12:10:27, 2020-01-01 12:15:56)", 11, 4),
      (1, RANGE<DATETIME> "[2020-01-01 12:16:00, 2020-01-01 12:20:58)", 11, 9),
      (1, RANGE<DATETIME> "[2020-01-01 12:20:33, 2020-01-01 12:25:08)", 11, 8),
      (2, RANGE<DATETIME> "[2020-01-01 12:00:19, 2020-01-01 12:05:08)", 21, 31),
      (2, RANGE<DATETIME> "[2020-01-01 12:05:08, 2020-01-01 12:10:30)", 21, 2),
      (2, RANGE<DATETIME> "[2020-01-01 12:10:22, 2020-01-01 12:15:42)", 21, 10)
    ]);

The following query on the table shows several overlapping ranges:

    SELECT * FROM mydataset.sensor_metrics;

    /*---+---+---+---+
     | sensor_id |                  duration                  | flow | spins |
     +---+---+---+---+
     |         1 | [2020-01-01 12:00:01, 2020-01-01 12:05:23) | 10   |     1 |
     |         1 | [2020-01-01 12:05:12, 2020-01-01 12:10:46) | 10   |    20 |
     |         1 | [2020-01-01 12:10:27, 2020-01-01 12:15:56) | 11   |     4 |
     |         1 | [2020-01-01 12:16:00, 2020-01-01 12:20:58) | 11   |     9 |
     |         1 | [2020-01-01 12:20:33, 2020-01-01 12:25:08) | 11   |     8 |
     |         2 | [2020-01-01 12:00:19, 2020-01-01 12:05:08) | 21   |    31 |
     |         2 | [2020-01-01 12:05:08, 2020-01-01 12:10:30) | 21   |     2 |
     |         2 | [2020-01-01 12:10:22, 2020-01-01 12:15:42) | 21   |    10 |
     +---+---+---+---*/

For some of the overlapping ranges, the value in the `flow` column is the same.
For example, rows 1 and 2 overlap, and also have the same `flow` readings. You
can combine these two rows to reduce the number of rows in the table. You can
use the `RANGE_SESSIONIZE` table function to find ranges that overlap
with each row, and provide an additional `session_range` column that contains a
range that is the union of all ranges that overlap. To display the session
ranges for each row, run the following query:

    SELECT sensor_id, session_range, flow
    FROM RANGE_SESSIONIZE(
      # Input data.
      (SELECT sensor_id, duration, flow FROM mydataset.sensor_metrics),
      # Range column.
      "duration",
      # Partitioning columns. Ranges are sessionized only within these partitions.
      ["sensor_id", "flow"],
      # Sessionize mode.
      "OVERLAPS")
    ORDER BY sensor_id, session_range;

    /*---+---+---+
     | sensor_id |                session_range               | flow |
     +---+---+---+
     |         1 | [2020-01-01 12:00:01, 2020-01-01 12:10:46) | 10   |
     |         1 | [2020-01-01 12:00:01, 2020-01-01 12:10:46) | 10   |
     |         1 | [2020-01-01 12:10:27, 2020-01-01 12:15:56) | 11   |
     |         1 | [2020-01-01 12:16:00, 2020-01-01 12:25:08) | 11   |
     |         1 | [2020-01-01 12:16:00, 2020-01-01 12:25:08) | 11   |
     |         2 | [2020-01-01 12:00:19, 2020-01-01 12:05:08) | 21   |
     |         2 | [2020-01-01 12:05:08, 2020-01-01 12:15:42) | 21   |
     |         2 | [2020-01-01 12:05:08, 2020-01-01 12:15:42) | 21   |
     +---+---+---*/

Note that for `sensor_id` having value `2`, the first row's end boundary has the
same datetime value as the second row's start boundary. However, because end
boundaries are exclusive, they don't overlap (only meet) and hence were not in
the same session ranges. If you want to place these two rows in the same session
ranges, use the [`MEETS`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/range-functions#range_sessionize)
sessionize mode.

To combine the ranges, group the results by `session_range` and the partitioning
columns (`sensor_id` and `flow`):

    SELECT sensor_id, session_range, flow
    FROM RANGE_SESSIONIZE(
      (SELECT sensor_id, duration, flow FROM mydataset.sensor_metrics),
      "duration",
      ["sensor_id", "flow"],
      "OVERLAPS")
    GROUP BY sensor_id, session_range, flow
    ORDER BY sensor_id, session_range;

    /*---+---+---+
     | sensor_id |                session_range               | flow |
     +---+---+---+
     |         1 | [2020-01-01 12:00:01, 2020-01-01 12:10:46) | 10   |
     |         1 | [2020-01-01 12:10:27, 2020-01-01 12:15:56) | 11   |
     |         1 | [2020-01-01 12:16:00, 2020-01-01 12:25:08) | 11   |
     |         2 | [2020-01-01 12:00:19, 2020-01-01 12:05:08) | 21   |
     |         2 | [2020-01-01 12:05:08, 2020-01-01 12:15:42) | 21   |
     +---+---+---*/

Finally, add the `spins` column in the session data by aggregating it using
`SUM`.

    SELECT sensor_id, session_range, flow, SUM(spins) as spins
    FROM RANGE_SESSIONIZE(
      TABLE mydataset.sensor_metrics,
      "duration",
      ["sensor_id", "flow"],
      "OVERLAPS")
    GROUP BY sensor_id, session_range, flow
    ORDER BY sensor_id, session_range;

    /*---+---+---+---+
     | sensor_id |                session_range               | flow | spins |
     +---+---+---+---+
     |         1 | [2020-01-01 12:00:01, 2020-01-01 12:10:46) | 10   |    21 |
     |         1 | [2020-01-01 12:10:27, 2020-01-01 12:15:56) | 11   |     4 |
     |         1 | [2020-01-01 12:16:00, 2020-01-01 12:25:08) | 11   |    17 |
     |         2 | [2020-01-01 12:00:19, 2020-01-01 12:05:08) | 21   |    31 |
     |         2 | [2020-01-01 12:05:08, 2020-01-01 12:15:42) | 21   |    12 |
     +---+---+---+---*/

### Split range data

You can also split a range into smaller ranges. For this example, use the
following table with range data:

    /*---+---+---+---+
     | sensor_id |         duration         | flow | spins |
     +---+---+---+---+
     |         1 | [2020-01-01, 2020-12-31) | 10   |    21 |
     |         1 | [2021-01-01, 2021-12-31) | 11   |     4 |
     |         2 | [2020-04-15, 2021-04-15) | 21   |    31 |
     |         2 | [2021-04-15, 2021-04-15) | 21   |    12 |
     +---+---+---+---*/

Now, split the original ranges into 3-month intervals:

    WITH sensor_data AS (
      SELECT * FROM UNNEST(
        ARRAY<STRUCT<sensor_id INT64, duration RANGE<DATE>, flow INT64, spins INT64>>[
        (1, RANGE<DATE> "[2020-01-01, 2020-12-31)", 10, 21),
        (1, RANGE<DATE> "[2021-01-01, 2021-12-31)", 11, 4),
        (2, RANGE<DATE> "[2020-04-15, 2021-04-15)", 21, 31),
        (2, RANGE<DATE> "[2021-04-15, 2022-04-15)", 21, 12)
      ])
    )
    SELECT sensor_id, expanded_range, flow, spins
    FROM sensor_data, UNNEST(GENERATE_RANGE_ARRAY(duration, INTERVAL 3 MONTH)) AS expanded_range;

    /*---+---+---+---+
     | sensor_id |      expanded_range      | flow | spins |
     +---+---+---+---+
     |         1 | [2020-01-01, 2020-04-01) |   10 |    21 |
     |         1 | [2020-04-01, 2020-07-01) |   10 |    21 |
     |         1 | [2020-07-01, 2020-10-01) |   10 |    21 |
     |         1 | [2020-10-01, 2020-12-31) |   10 |    21 |
     |         1 | [2021-01-01, 2021-04-01) |   11 |     4 |
     |         1 | [2021-04-01, 2021-07-01) |   11 |     4 |
     |         1 | [2021-07-01, 2021-10-01) |   11 |     4 |
     |         1 | [2021-10-01, 2021-12-31) |   11 |     4 |
     |         2 | [2020-04-15, 2020-07-15) |   21 |    31 |
     |         2 | [2020-07-15, 2020-10-15) |   21 |    31 |
     |         2 | [2020-10-15, 2021-01-15) |   21 |    31 |
     |         2 | [2021-01-15, 2021-04-15) |   21 |    31 |
     |         2 | [2021-04-15, 2021-07-15) |   21 |    12 |
     |         2 | [2021-07-15, 2021-10-15) |   21 |    12 |
     |         2 | [2021-10-15, 2022-01-15) |   21 |    12 |
     |         2 | [2022-01-15, 2022-04-15) |   21 |    12 |
     +---+---+---+---*/

In the previous query, each original range was broken down into smaller
ranges, with width set to `INTERVAL 3 MONTH`. However, the 3-month ranges are
not aligned to a common origin. To align these ranges to a common origin
`2020-01-01`, run the following query:

    WITH sensor_data AS (
      SELECT * FROM UNNEST(
        ARRAY<STRUCT<sensor_id INT64, duration RANGE<DATE>, flow INT64, spins INT64>>[
        (1, RANGE<DATE> "[2020-01-01, 2020-12-31)", 10, 21),
        (1, RANGE<DATE> "[2021-01-01, 2021-12-31)", 11, 4),
        (2, RANGE<DATE> "[2020-04-15, 2021-04-15)", 21, 31),
        (2, RANGE<DATE> "[2021-04-15, 2022-04-15)", 21, 12)
      ])
    )
    SELECT sensor_id, expanded_range, flow, spins
    FROM sensor_data
    JOIN UNNEST(GENERATE_RANGE_ARRAY(RANGE<DATE> "[2020-01-01, 2022-12-31)", INTERVAL 3 MONTH)) AS expanded_range
    ON RANGE_OVERLAPS(duration, expanded_range);

    /*---+---+---+---+
     | sensor_id |      expanded_range      | flow | spins |
     +---+---+---+---+
     |         1 | [2020-01-01, 2020-04-01) |   10 |    21 |
     |         1 | [2020-04-01, 2020-07-01) |   10 |    21 |
     |         1 | [2020-07-01, 2020-10-01) |   10 |    21 |
     |         1 | [2020-10-01, 2021-01-01) |   10 |    21 |
     |         1 | [2021-01-01, 2021-04-01) |   11 |     4 |
     |         1 | [2021-04-01, 2021-07-01) |   11 |     4 |
     |         1 | [2021-07-01, 2021-10-01) |   11 |     4 |
     |         1 | [2021-10-01, 2022-01-01) |   11 |     4 |
     |         2 | [2020-04-01, 2020-07-01) |   21 |    31 |
     |         2 | [2020-07-01, 2020-10-01) |   21 |    31 |
     |         2 | [2020-10-01, 2021-01-01) |   21 |    31 |
     |         2 | [2021-01-01, 2021-04-01) |   21 |    31 |
     |         2 | [2021-04-01, 2021-07-01) |   21 |    31 |
     |         2 | [2021-04-01, 2021-07-01) |   21 |    12 |
     |         2 | [2021-07-01, 2021-10-01) |   21 |    12 |
     |         2 | [2021-10-01, 2022-01-01) |   21 |    12 |
     |         2 | [2022-01-01, 2022-04-01) |   21 |    12 |
     |         2 | [2022-04-01, 2022-07-01) |   21 |    12 |
     +---+---+---+---*/

In the previous query, the row with the range `[2020-04-15, 2021-04-15)` is
split into 5 ranges, starting with the range `[2020-04-01, 2020-07-01)`. Note
that the start boundary now extends beyond the original start boundary, in order
to align with the common origin. If you don't want the start boundary to not
extend beyond the original start boundary, you can restrict the `JOIN`
condition:

    WITH sensor_data AS (
      SELECT * FROM UNNEST(
        ARRAY<STRUCT<sensor_id INT64, duration RANGE<DATE>, flow INT64, spins INT64>>[
        (1, RANGE<DATE> "[2020-01-01, 2020-12-31)", 10, 21),
        (1, RANGE<DATE> "[2021-01-01, 2021-12-31)", 11, 4),
        (2, RANGE<DATE> "[2020-04-15, 2021-04-15)", 21, 31),
        (2, RANGE<DATE> "[2021-04-15, 2022-04-15)", 21, 12)
      ])
    )
    SELECT sensor_id, expanded_range, flow, spins
    FROM sensor_data
    JOIN UNNEST(GENERATE_RANGE_ARRAY(RANGE<DATE> "[2020-01-01, 2022-12-31)", INTERVAL 3 MONTH)) AS expanded_range
    ON RANGE_CONTAINS(duration, RANGE_START(expanded_range));

    /*---+---+---+---+
     | sensor_id |      expanded_range      | flow | spins |
     +---+---+---+---+
     |         1 | [2020-01-01, 2020-04-01) |   10 |    21 |
     |         1 | [2020-04-01, 2020-07-01) |   10 |    21 |
     |         1 | [2020-07-01, 2020-10-01) |   10 |    21 |
     |         1 | [2020-10-01, 2021-01-01) |   10 |    21 |
     |         1 | [2021-01-01, 2021-04-01) |   11 |     4 |
     |         1 | [2021-04-01, 2021-07-01) |   11 |     4 |
     |         1 | [2021-07-01, 2021-10-01) |   11 |     4 |
     |         1 | [2021-10-01, 2022-01-01) |   11 |     4 |
     |         2 | [2020-07-01, 2020-10-01) |   21 |    31 |
     |         2 | [2020-10-01, 2021-01-01) |   21 |    31 |
     |         2 | [2021-01-01, 2021-04-01) |   21 |    31 |
     |         2 | [2021-04-01, 2021-07-01) |   21 |    31 |
     |         2 | [2021-07-01, 2021-10-01) |   21 |    12 |
     |         2 | [2021-10-01, 2022-01-01) |   21 |    12 |
     |         2 | [2022-01-01, 2022-04-01) |   21 |    12 |
     |         2 | [2022-04-01, 2022-07-01) |   21 |    12 |
     +---+---+---+---*/

You now see that the range `[2020-04-15, 2021-04-15)` was split into 4 ranges,
starting with the range `[2020-07-01, 2020-10-01)`.

## Best practices for storing data

- When storing time series data, it is important to consider the query patterns
  that are used against the tables where the data is stored. Typically, when
  querying time series data, you can filter the data for a specific time range.

- To optimize these usage patterns, it is recommended to store time series data
  in [partitioned tables](https://docs.cloud.google.com/bigquery/docs/partitioned-tables),
  with data partitioned either by the [time column](https://docs.cloud.google.com/bigquery/docs/creating-partitioned-tables#create_a_time-unit_column-partitioned_table)
  or [ingestion time](https://docs.cloud.google.com/bigquery/docs/creating-partitioned-tables#create_an_ingestion-time_partitioned_table).
  This can significantly improve the query time performance of the time series
  data, as this lets BigQuery to prune partitions that don't
  contain queried data.

- You can enable [clustering](https://docs.cloud.google.com/bigquery/docs/clustered-tables) on the time,
  range, or one of the partitioning columns for further query time performance
  improvements.