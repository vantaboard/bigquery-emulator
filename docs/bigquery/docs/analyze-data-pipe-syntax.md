# Analyze data using pipe syntax

This tutorial shows you how to write queries using pipe syntax to analyze data.

Pipe syntax is an extension to GoogleSQL that supports a linear query
structure designed to make your queries easier to read, write, and maintain.
Pipe syntax consists of the pipe symbol `|>`, a
[pipe operator](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/pipe-syntax#pipe_operators)
name, and any arguments. For more information, see the following resources:

- For an introduction to pipe syntax, see [Work with pipe query syntax](https://docs.cloud.google.com/bigquery/docs/pipe-syntax-guide).
- For full syntax details, see the [Pipe query syntax](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/pipe-syntax) reference documentation.

In this tutorial, you build a complex query in pipe syntax using the publicly
available
[`bigquery-public-data.austin_bikeshare.bikeshare_trips` table](https://console.cloud.google.com/bigquery?p=bigquery-public-data&d=austin_bikeshare&t=bikeshare_trips&page=table),
which contains data about bicycle trips.

## Objectives

- View table data by starting a query with a [`FROM` clause](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/pipe-syntax#from_queries).
- Add columns by using the [`EXTEND` pipe operator](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/pipe-syntax#extend_pipe_operator).
- Aggregate data by day and week by using the [`AGGREGATE` pipe operator](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/pipe-syntax#extend_pipe_operator).
- Aggregate data over a sliding window by using the [`CROSS JOIN` pipe operator](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/pipe-syntax#join_pipe_operator).
- Filter data by using the [`WHERE` pipe operator](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/pipe-syntax#where_pipe_operator).
- Compare the linear query structure of pipe syntax to the nested query structure of standard syntax when performing multi-level aggregations.

## Before you begin

To get started using a BigQuery public dataset, you must create or select a
project. The first terabyte of data processed per month is free, so you can start querying public
datasets without enabling billing. If you intend to go beyond the
[free tier](https://cloud.google.com/bigquery/pricing#free-tier), you must also enable billing.

1. BigQuery is automatically enabled in new projects. To activate BigQuery in a preexisting project,


   Enable the BigQuery API.


   **Roles required to enable APIs**


   To enable APIs, you need the Service Usage Admin IAM
   role (`roles/serviceusage.serviceUsageAdmin`), which
   contains the `serviceusage.services.enable` permission. [Learn how to grant
   roles](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).

   [Enable the API](https://console.cloud.google.com/flows/enableapi?apiid=bigquery)

For more information about the different ways to run queries, see
[Run a query](https://docs.cloud.google.com/bigquery/docs/running-queries).

## View table data

To retrieve all the data from the `bikeshare_trips` table,
run the following query:

### Pipe syntax

    FROM `bigquery-public-data.austin_bikeshare.bikeshare_trips`;

### Standard syntax

    SELECT *
    FROM `bigquery-public-data.austin_bikeshare.bikeshare_trips`;

In pipe syntax, the query can start with a
[`FROM` clause](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/pipe-syntax#from_queries)
without a `SELECT` clause to return table results.

The result is similar to the following:

```
+---+---+---+---+---+---+
| trip_id  | subscriber_type | bike_id | bike_type | start_time              | ... |
+---+---+---+---+---+---+
| 28875008 | Pay-as-you-ride | 18181   | electric  | 2023-02-12 12:46:32 UTC | ... |
| 28735401 | Explorer        | 214     | classic   | 2023-01-13 12:01:45 UTC | ... |
| 29381980 | Local365        | 21803   | electric  | 2023-04-20 08:43:46 UTC | ... |
| ...      | ...             | ...     | ...       | ...                     | ... |
+---+---+---+---+---+---+
```

## Add columns

In the `bikeshare_trips` table, the `start_time` column is a timestamp, but you
might want to add a column that only shows the date of the trip. To add a column
in pipe syntax, use the
[`EXTEND` pipe operator](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/pipe-syntax#extend_pipe_operator):

### Pipe syntax

    FROM `bigquery-public-data.austin_bikeshare.bikeshare_trips`
    |> EXTEND CAST(start_time AS DATE) AS date;

### Standard syntax

    SELECT *, CAST(start_time AS DATE) AS date
    FROM `bigquery-public-data.austin_bikeshare.bikeshare_trips`;

The result is similar to the following:

```
+---+---+---+---+---+---+---+
| trip_id  | subscriber_type | bike_id | bike_type | start_time              | date       | ... |
+---+---+---+---+---+---+---+
| 28875008 | Pay-as-you-ride | 18181   | electric  | 2023-02-12 12:46:32 UTC | 2023-02-12 | ... |
| 28735401 | Explorer        | 214     | classic   | 2023-01-13 12:01:45 UTC | 2023-01-13 | ... |
| 29381980 | Local365        | 21803   | electric  | 2023-04-20 08:43:46 UTC | 2023-04-20 | ... |
| ...      | ...             | ...     | ...       | ...                     | ...        | ... |
+---+---+---+---+---+---+---+
```

## Aggregate daily data

You can group by date to find the total number of trips taken and the bikes used
per day.

- Use the [`AGGREGATE` pipe operator](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/pipe-syntax#aggregate_pipe_operator) with the `COUNT` function to find the total number of trips taken and bikes used.
- Use the `GROUP BY` clause to group the results by date.

### Pipe syntax

    FROM `bigquery-public-data.austin_bikeshare.bikeshare_trips`
    |> EXTEND CAST(start_time AS DATE) AS date
    |> AGGREGATE
         COUNT(*) AS trips,
         COUNT(DISTINCT bike_id) AS distinct_bikes
       GROUP BY date;

### Standard syntax

    SELECT
      CAST(start_time AS DATE) AS date,
      COUNT(*) AS trips,
      COUNT(DISTINCT bike_id) AS distinct_bikes
    FROM `bigquery-public-data.austin_bikeshare.bikeshare_trips`
    GROUP BY date;

The result is similar to the following:

    +---+---+---+
    | date       | trips | distinct_bikes |
    +---+---+---+
    | 2023-04-20 | 841   | 197            |
    | 2023-01-27 | 763   | 148            |
    | 2023-06-12 | 562   | 202            |
    | ...        | ...   | ...            |
    +---+---+---+

## Order results

To sort the results in descending order by the `date` column, add the
[`DESC`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/pipe-syntax#shorthand_order_pipe_syntax)
suffix to the `GROUP BY` clause:

### Pipe syntax

    FROM `bigquery-public-data.austin_bikeshare.bikeshare_trips`
    |> EXTEND CAST(start_time AS DATE) AS date
    |> AGGREGATE
         COUNT(*) AS trips,
         COUNT(DISTINCT bike_id) AS distinct_bikes
       GROUP BY date DESC;

### Standard syntax

    SELECT
      CAST(start_time AS DATE) AS date,
      COUNT(*) AS trips,
      COUNT(DISTINCT bike_id) AS distinct_bikes
    FROM `bigquery-public-data.austin_bikeshare.bikeshare_trips`
    GROUP BY date
    ORDER BY date DESC;

The result is similar to the following:

    +---+---+---+
    | date       | trips | distinct_bikes |
    +---+---+---+
    | 2024-06-30 | 331   | 90             |
    | 2024-06-29 | 395   | 123            |
    | 2024-06-28 | 437   | 137            |
    | ...        | ...   | ...            |
    +---+---+---+

In pipe syntax, you can add the sorting suffix directly to the `GROUP BY` clause
without using the
[`ORDER BY` pipe operator](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/pipe-syntax#order_by_pipe_operator).
Adding the suffix to the `GROUP BY` clause is one of several optional
[shorthand ordering features with `AGGREGATE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/pipe-syntax#shorthand_order_pipe_syntax)
that pipe syntax supports. In standard syntax, this isn't possible and you
must use the `ORDER BY` clause for sorting.

## Aggregate weekly data

Now that you have data on the number of bikes used each day, you can build
on your query to find the number of distinct bikes used over each seven-day
window.

To update the rows in your table to display weeks instead of days, use the
[`DATE_TRUNC` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/date_functions#date_trunc)
in the `GROUP BY` clause and set the granularity to `WEEK`:

### Pipe syntax

    FROM `bigquery-public-data.austin_bikeshare.bikeshare_trips`
    |> EXTEND CAST(start_time AS DATE) AS date
    |> AGGREGATE
        COUNT(*) AS trips,
        COUNT(DISTINCT bike_id) AS distinct_bikes,
    GROUP BY DATE_TRUNC(date, WEEK) AS date DESC;

### Standard syntax

    SELECT
      DATE_TRUNC(CAST(start_time AS DATE), WEEK) AS date,
      COUNT(*) AS trips,
      COUNT(DISTINCT bike_id) AS distinct_bikes,
    FROM `bigquery-public-data.austin_bikeshare.bikeshare_trips`
    GROUP BY date
    ORDER BY date DESC;

The result is similar to the following:

    +---+---+---+
    | date       | trips | distinct_bikes |
    +---+---+---+
    | 2024-06-30 | 331   | 90             |
    | 2024-06-23 | 3206  | 213            |
    | 2024-06-16 | 3441  | 212            |
    | ...        | ...   | ...            |
    +---+---+---+

## Aggregate over a sliding window

The results in the preceding section show trips in a *fixed window* between
start and end dates, such as `2024-06-23` through
`2024-06-29`. Instead, you might want to see
trips in a *sliding window*, over a seven-day period that
moves forward in time with each new day. In other words, for any given date you
might want to know about the number of trips taken and bikes used over the
following week.

To apply a sliding window to your data, first copy each trip forward six
additional *active* days from its start date. Then, compute the dates of the
active days by using the `DATE_ADD` function. Finally, aggregate the trips and
bike IDs for each active day.

1. To copy your data forward, use the `GENERATE_ARRAY` function and a
   cross join:

   ### Pipe syntax

       FROM `bigquery-public-data.austin_bikeshare.bikeshare_trips`
       |> EXTEND CAST(start_time AS DATE) AS date
       |> CROSS JOIN UNNEST(GENERATE_ARRAY(0, 6)) AS diff_days;

   ### Standard syntax

       SELECT *, CAST(start_time AS DATE) AS date
       FROM `bigquery-public-data.austin_bikeshare.bikeshare_trips`
       CROSS JOIN UNNEST(GENERATE_ARRAY(0, 6)) AS diff_days;

   The `GENERATE_ARRAY` function creates an array with seven elements, `0` to
   `6`. The `CROSS JOIN UNNEST` operation creates seven copies of each row, with a
   new `diff_days` column that contains one of the array element values from
   `0` to `6` for each row. You can use the `diff_days` values as the
   adjustment to the original date to slide the window forward by that many
   days, up to seven days past the original date.
2. To see the calculated active dates for trips, use the `EXTEND` pipe operator
   with the `DATE_ADD` function to create a column called `active_date`
   that contains the start date plus the value in the `diff_days` column:

   ### Pipe syntax

       FROM `bigquery-public-data.austin_bikeshare.bikeshare_trips`
       |> EXTEND CAST(start_time AS DATE) AS date
       |> CROSS JOIN UNNEST(GENERATE_ARRAY(0, 6)) AS diff_days
       |> EXTEND DATE_ADD(date, INTERVAL diff_days DAY) AS active_date;

   ### Standard syntax

       SELECT *, DATE_ADD(date, INTERVAL diff_days DAY) AS active_date
       FROM (
         SELECT *, CAST(start_time AS DATE) AS date
         FROM `bigquery-public-data.austin_bikeshare.bikeshare_trips`
         CROSS JOIN UNNEST(GENERATE_ARRAY(0, 6)) AS diff_days)

   For example, a trip that starts on `2024-05-20` is also
   considered active on each day through `2024-05-26`.
3. Finally, aggregate trips IDs and bike IDs and group by `active_date`:

   ### Pipe syntax

       FROM `bigquery-public-data.austin_bikeshare.bikeshare_trips`
       |> EXTEND CAST(start_time AS DATE) AS date
       |> CROSS JOIN UNNEST(GENERATE_ARRAY(0, 6)) AS diff_days
       |> EXTEND DATE_ADD(date, INTERVAL diff_days DAY) AS active_date
       |> AGGREGATE COUNT(DISTINCT bike_id) AS active_7d_bikes,
                   COUNT(trip_id) AS active_7d_trips
       GROUP BY active_date DESC;

   ### Standard syntax

       SELECT
         DATE_ADD(date, INTERVAL diff_days DAY) AS active_date,
         COUNT(DISTINCT bike_id) AS active_7d_bikes,
         COUNT(trip_id) AS active_7d_trips
       FROM (
         SELECT *, CAST(start_time AS DATE) AS date
         FROM `bigquery-public-data.austin_bikeshare.bikeshare_trips`
         CROSS JOIN UNNEST(GENERATE_ARRAY(0, 6)) AS diff_days)
       GROUP BY active_date
       ORDER BY active_date DESC;

   The result is similar to the following:

       +---+---+---+
       | active_date | active_7d_bikes | active_7d_trips |
       +---+---+---+
       | 2024-07-06  | 90              | 331             |
       | 2024-07-05  | 142             | 726             |
       | 2024-07-04  | 186             | 1163            |
       | ...         | ...             | ...             |
       +---+---+---+

## Filter future dates

In the preceding query, the dates extend into the future up to six days beyond
the last date in your data. To
filter out dates that extend beyond the end of your data, set a
maximum date in your query:

1. Add another `EXTEND` pipe operator that uses a window function with an `OVER` clause to compute the maximum date in the table.
2. Use the `WHERE` pipe operator to filter out the generated rows that are past the maximum date.

### Pipe syntax

    FROM `bigquery-public-data.austin_bikeshare.bikeshare_trips`
    |> EXTEND CAST(start_time AS DATE) AS date
    |> EXTEND MAX(date) OVER () AS max_date
    |> CROSS JOIN UNNEST(GENERATE_ARRAY(0, 6)) AS diff_days
    |> EXTEND DATE_ADD(date, INTERVAL diff_days DAY) AS active_date
    |> WHERE active_date <= max_date
    |> AGGREGATE COUNT(DISTINCT bike_id) AS active_7d_bikes,
                 COUNT(trip_id) AS active_7d_trips
       GROUP BY active_date DESC;

### Standard syntax

    SELECT
      DATE_ADD(date, INTERVAL diff_days DAY) AS active_date,
      COUNT(DISTINCT bike_id) AS active_7d_bikes,
      COUNT(trip_id) AS active_7d_trips
    FROM(
      SELECT *
      FROM (
        SELECT *,
          DATE_ADD(date, INTERVAL diff_days DAY) AS active_date,
          MAX(date) OVER () AS max_date
        FROM(
          SELECT *, CAST(start_time AS DATE) AS date,
          FROM `bigquery-public-data.austin_bikeshare.bikeshare_trips`
          CROSS JOIN UNNEST(GENERATE_ARRAY(0, 6)) AS diff_days))
      WHERE active_date <= max_date)
    GROUP BY active_date
    ORDER BY active_date DESC;

The result is similar to the following:

    +---+---+---+
    | active_date | active_7d_bikes | active_7d_trips |
    +---+---+---+
    | 2024-06-30  | 212             | 3031            |
    | 2024-06-29  | 213             | 3206            |
    | 2024-06-28  | 219             | 3476            |
    | ...         | ...             | ...             |
    +---+---+---+

## What's next

- For more information about how pipe syntax works, see [Work with pipe query syntax](https://docs.cloud.google.com/bigquery/docs/pipe-syntax-guide).
- For more technical information, see the [Pipe query syntax](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/pipe-syntax) reference documentation.