# Understand window aggregation in continuous queries

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

To request support or provide feedback for this feature, send an email to
[bq-continuous-queries-feedback@google.com](mailto:bq-continuous-queries-feedback@google.com).

BigQuery continuous queries support aggregations and windowing as
stateful operations.
Stateful operations let continuous queries perform complex analysis that
requires retaining information across multiple rows or time intervals. This
capability lets you calculate metrics over time---such as a 30-minute
average---by storing necessary data in memory while the query runs.

Windowing functions assign data into logical components, or windows, based on
system time, which indicates the commit time of the transaction that made the
change. In BigQuery, these functions are table-valued functions
(TVFs) that return a table that includes all original columns and two additional
columns: `window_start` and `window_end`. These columns identify the time
interval for each window. For more
information about stateful operations, see
[Supported stateful operations](https://docs.cloud.google.com/bigquery/docs/continuous-queries-introduction#supported_stateful_operations).

Windowing TVFs are only supported with [BigQuery continuous
queries](https://docs.cloud.google.com/bigquery/docs/continuous-queries-introduction).

Windowing TVFs are distinct from [window function
calls](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/window-function-calls).

## Supported aggregation functions

The following aggregation functions are supported:

- [`ANY_VALUE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#any_value)
- [`APPROX_COUNT_DISTINCT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/approximate_aggregate_functions#approx_count_distinct)
- [`APPROX_QUANTILES`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/approximate_aggregate_functions#approx_quantiles)
- [`APPROX_TOP_COUNT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/approximate_aggregate_functions#approx_top_count)
- [`APPROX_TOP_SUM`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/approximate_aggregate_functions#approx_top_sum)
- [`ARRAY_CONCAT_AGG`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#array_concat_agg)
- [`AVG`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#avg)
- [`BIT_AND`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#bit_and)
- [`BIT_OR`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#bit_or)
- [`BIT_XOR`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#bit_xor)
- [`CORR`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#corr)
- [`COUNT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#count)
- [`COUNTIF`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#countif)
- [`COVAR_POP`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#covar_pop)
- [`COVAR_SAMP`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#covar_samp)
- [`LOGICAL_AND`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#logical_and)
- [`LOGICAL_OR`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#logical_or)
- [`MAX`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#max)
- [`MAX_BY`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#max_by)
- [`MIN`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#min)
- [`MIN_BY`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#min_by)
- [`STDDEV`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#stddev)
- [`STDDEV_POP`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#stddev_pop)
- [`STDDEV_SAMP`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#stddev_samp)
- [`SUM`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#sum)
- [`VAR_POP`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#var_pop)
- [`VAR_SAMP`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#var_samp)
- [`VARIANCE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/statistical_aggregate_functions#variance)

## Unsupported aggregation functions

The following aggregation functions are unsupported:

- [`ARRAY_AGG`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#array_agg)
- [`AVG` (Differential Privacy)](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate-dp-functions#dp_avg)
- [`COUNT` (Differential Privacy)](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate-dp-functions#dp_count)
- Functions containing `DISTINCT` expressions.
- [`GROUPING`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#grouping)
- [`PERCENTILE_CONT` (Differential Privacy)](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate-dp-functions#dp_percentile_cont)
- [`ST_CENTROID_AGG`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_centroid_agg)
- [`ST_EXTENT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_extent)
- [`ST_UNION_AGG`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_union_agg)
- [`STRING_AGG`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#string_agg)
- [`SUM` (Differential Privacy)](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate-dp-functions#dp_sum)

## The TUMBLE function

The `TUMBLE` function assigns data into non-overlapping time intervals (tumbling
windows) of specified size. For example, a 5-minute window groups events into
discrete intervals such as `[2026-01-01 12:00:00, 2026-01-01 12:05:00)` and
`[2026-01-01 12:05:00, 2026-01-01 12:10:00)`. A row with a timestamp value
`2026-01-01 12:03:18` is assigned to the first window. Because these windows are
disjoint and don't overlap, every element with a timestamp is assigned to
exactly one window.

The following diagram shows how the `TUMBLE` function assigns events into
non-overlapping time intervals:

![The TUMBLE function assigns events into non-overlapping time intervals.](https://docs.cloud.google.com/static/bigquery/images/tumble-window.png)

You can use this function in real-time event processing to group events by time
ranges before you perform any aggregations.

### Syntax

    TUMBLE(TABLE table, "timestamp_column", window_size)

### Definitions

- `table`: The BigQuery table name. This must be a [standard
  BigQuery table](https://docs.cloud.google.com/bigquery/docs/tables-intro#standard-tables)
  wrapped within the `APPENDS` function. The word `TABLE` must precede this
  argument.

- `timestamp_column`: A `STRING` literal that specifies the name of the column
  in the input table that contains the event time. The values in this column
  assign each row to a window. The `_CHANGE_TIMESTAMP` column, which defines
  the BigQuery system time, is the only supported
  `timestamp_column`. User-defined columns aren't
  supported.

- `window_size`: An `INTERVAL` value that defines the duration of each
  tumbling window. Window sizes can be a maximum of 24 hours. For example:
  `INTERVAL 30 SECOND`.

### Output

The `TUMBLE` function returns an output with the following columns:

- All columns of the input table at the time the query runs.

- `window_start`: A `TIMESTAMP` value that indicates the inclusive start time
  of the window to which the record belongs.

- `window_end`: A `TIMESTAMP` value that indicates the exclusive end time of
  the window to which the record belongs.

### Output materialization

In a BigQuery continuous query, a windowed aggregation doesn't
produce output for a specific time interval until BigQuery
finalizes or *closes* that window. This behavior ensures that
BigQuery emits the aggregated results only after it processes all
relevant data for that window.

For example, if you perform a 5-minute `TUMBLE` window aggregation on a
`user_clickstream` table, the results for the interval `[10:15; 10:20)` are only
emitted after the query processes records with a `_CHANGE_TIMESTAMP` of 10:20 or
later. At that moment, BigQuery considers the window closed.
Additionally, a window *opens* and begins accumulating data the moment the first
record belonging to that specific time range appears.

While a window remains open, BigQuery must preserve the
intermediate aggregation results. This requires storing the state, which means
BigQuery must preserve the intermediate aggregation results.
Because this state must remain in active memory until the window closes, using
longer window durations or processing high-volume streams leads to higher slot
utilization to manage the increased amount of stored context. For more
information, see [Pricing considerations](https://docs.cloud.google.com/bigquery/docs/window-aggregations#pricing_considerations).

### Limitations

- The `TUMBLE` function is supported only in BigQuery continuous queries.
- When starting a continuous query with the `TUMBLE` function, you can use only the `APPENDS` function. The `CHANGES` function isn't supported.
- The BigQuery system time column defined by `_CHANGE_TIMESTAMP` is the only supported `timestamp_column`. User-defined columns aren't supported.
- Window sizes can be a maximum of 24 hours.
- When the `TUMBLE` windowing function runs, it produces two additional output columns: `window_start` and `window_end`. You must include at least one of these columns in the `GROUP BY` statement within the `SELECT` statement that performs the window aggregation.
- When you use the `TUMBLE` function with continuous query joins, you must follow all continuous query join [limitations](https://docs.cloud.google.com/bigquery/docs/continuous-queries-introduction#limitations).

### Pricing considerations

BigQuery continuous queries bill you based on the
[compute capacity (slots) consumed](https://docs.cloud.google.com/bigquery/pricing#capacity-compute-pricing)
while the job runs. This compute-based model also applies to stateful operations
like windowing. Because windowing requires
BigQuery to store "state" while the query is active, it consumes
additional slot resources. In general, the more context or data stored within a
window---such as when using longer window durations---the more state
BigQuery must preserve. This leads to higher slot utilization.

### Examples

The following query shows you how to query a taxi rides table to get a streaming
average number of rides, number of passengers, and average fare per taxi every
30 minutes, and export this data into a table in BigQuery:

    INSERT INTO
     `real_time_taxi_streaming.driver_stats`

    WITH ride_completions AS (
     SELECT
       _CHANGE_TIMESTAMP as bq_changed_ts,
       CAST(timestamp AS DATE) AS ride_date,
       taxi_id,
       meter_reading,
       passenger_count
     FROM
       APPENDS(TABLE `real_time_taxi_streaming.taxirides`,
         CURRENT_TIMESTAMP() - INTERVAL 10 MINUTE)
     WHERE
       ride_status = 'dropoff')

     SELECT
       ride_date,
       window_end,
       taxi_id,
       COUNT(taxi_id) AS total_rides_per_half_hour,
       ROUND(AVG(meter_reading),2) AS avg_fare_per_half_hour,
       SUM(passenger_count) AS total_passengers_per_half_hour
    FROM
      tumble(TABLE ride_completions,"bq_changed_ts",INTERVAL 30 MINUTE)
    GROUP BY
      window_end,
      ride_date,
      taxi_id

## What's next

- Learn how to [perform JOINs, aggregations, and windowing](https://docs.cloud.google.com/bigquery/docs/continuous-queries#join-agg-window-example).
- Learn more about [BigQuery continuous queries](https://docs.cloud.google.com/bigquery/docs/continuous-queries-introduction).
- Learn how to [join data from multiple streams](https://docs.cloud.google.com/bigquery/docs/continuous-query-joins).