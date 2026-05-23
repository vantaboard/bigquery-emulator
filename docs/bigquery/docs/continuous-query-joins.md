# Use stream-to-stream joins in continuous queries

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

Continuous queries support `JOIN` as a stateful operation. Stateful operations
let continuous queries perform complex analysis that requires retaining
information across multiple rows or time intervals. This capability lets you
correlate events from different streams by storing necessary data in memory
while the query runs. For more information about stateful operations, see
[Supported stateful operations](https://docs.cloud.google.com/bigquery/docs/continuous-queries-introduction#supported_stateful_operations).

Stream-to-stream joins are a join operation between two or more tables that
receive time-oriented data ingestion.

### Supported JOIN types

The following JOIN types are supported by continuous queries:

- Stream-to-stream JOIN-a join operation between two or more tables that receive time-oriented data ingestion.
- [INNER JOIN](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#join_types).

### Unsupported JOIN types

The following JOIN types are unsupported by continuous queries:

- Stream-to-static-table JOINs-a join where at least one joined table is a static table that doesn't receive time-oriented data ingestion. An example of a static table is a dimension table.
- Other forms of JOIN operations besides `INNER`.
- JOINs that don't have time-oriented JOIN clauses.

## Join data from multiple streams

The following query shows you how to join a taxi rides table to a taxi requests
table and identify the closest available taxi to the requestor within a 5-minute
time window, and export this data into another BigQuery table:

    INSERT INTO
     `real_time_taxi_streaming.matched_rides`
    SELECT
     requests.timestamp AS request_time,
     requests.request_id,
     taxis.taxi_id,
     ST_DISTANCE(
       ST_GEOGPOINT(requests.longitude, requests.latitude),
       ST_GEOGPOINT(taxis.longitude, taxis.latitude)
       ) AS distance_in_meters,
     taxis.timestamp AS taxi_available_time
    FROM
     APPENDS (TABLE `real_time_taxi_streaming.ride_requests`,
       CURRENT_TIMESTAMP() - INTERVAL 10 MINUTE) AS requests
    INNER JOIN
     APPENDS (TABLE `real_time_taxi_streaming.taxirides`,
       CURRENT_TIMESTAMP() - INTERVAL 10 MINUTE) AS taxis
    ON
     requests.geohash = taxis.geohash
    WHERE
     taxis.ride_status = 'available'
     AND taxis._CHANGE_TIMESTAMP BETWEEN(requests._CHANGE_TIMESTAMP - INTERVAL 5 MINUTE) AND requests._CHANGE_TIMESTAMP
     AND ST_DWITHIN(
       ST_GEOGPOINT(requests.longitude, requests.latitude),
       ST_GEOGPOINT(taxis.longitude, taxis.latitude),
       2000 -- Distance in meters
       );

## Join considerations

The following sections describe necessary considerations when performing a
stream-to-stream join.

### Limitations

- Only `INNER JOIN` operations are supported; other forms, like `LEFT` or `FULL OUTER`, are not supported.
- Each side of the `INNER JOIN` operation must specify a starting time for the continuous query.
- In addition to a join key (for example, `table1.user_id = table2.user_id`), the `JOIN` clause must include a condition to restrict the timestamp column to a constant interval. This condition limits how long the system waits for a matching event to arrive in the other stream. For example, you can specify that an event from one stream can only be joined with an event from another if their timestamps are within a 5-minute interval. You aren't limited to symmetrical intervals. For example, you could use a 5-minute interval on one side of the join condition and a 1-hour interval on the other.
- When starting a continuous query with a stream-to-stream join, only the `APPENDS` function is supported. The `CHANGES` function is not supported.
- The BigQuery system time column, defined by `_CHANGE_TIMESTAMP`, is the only supported timestamp column for join operations. User-defined columns are not supported.
- When used in conjunction with windowed aggregations, you must follow all documented window aggregation [limitations](https://docs.cloud.google.com/bigquery/docs/window-aggregations#limitations).

### Pricing considerations

BigQuery continuous queries are billed based on the [compute
capacity (slots)](https://docs.cloud.google.com/bigquery/pricing#capacity-compute-pricing) consumed while the
job is running. This compute-based model
also applies to stateful operations like joins. Because joins require the system
to store "state" while the query is active, they consume additional slot
resources. More context or data stored within a join---for example, when you use
longer time intervals in the `JOIN` or `WHERE` clause---requires preserving more
state, which leads to higher slot utilization.

## What's next

- Learn more about [BigQuery continuous queries](https://docs.cloud.google.com/bigquery/docs/continuous-queries-introduction).
- Learn how to [use window aggregation](https://docs.cloud.google.com/bigquery/docs/window-aggregations).
- Learn how to [perform JOINs, aggregations, and windowing](https://docs.cloud.google.com/bigquery/docs/continuous-queries#join-agg-window-example).