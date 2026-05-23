GoogleSQL for BigQuery supports the following time functions.

## Function list

| Name | Summary |
|---|---|
| [`CURRENT_TIME`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/time_functions#current_time) | Returns the current time as a `TIME` value. |
| [`EXTRACT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/time_functions#extract) | Extracts part of a `TIME` value. |
| [`FORMAT_TIME`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/time_functions#format_time) | Formats a `TIME` value according to the specified format string. |
| [`PARSE_TIME`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/time_functions#parse_time) | Converts a `STRING` value to a `TIME` value. |
| [`TIME`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/time_functions#time) | Constructs a `TIME` value. |
| [`TIME_ADD`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/time_functions#time_add) | Adds a specified time interval to a `TIME` value. |
| [`TIME_DIFF`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/time_functions#time_diff) | Gets the number of unit boundaries between two `TIME` values at a particular time granularity. |
| [`TIME_SUB`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/time_functions#time_sub) | Subtracts a specified time interval from a `TIME` value. |
| [`TIME_TRUNC`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/time_functions#time_trunc) | Truncates a `TIME` value at a particular granularity. |

## `CURRENT_TIME`

    CURRENT_TIME([time_zone])

    CURRENT_TIME

**Description**

Returns the current time as a `TIME` object. Parentheses are optional when
called with no arguments.

This function supports an optional `time_zone` parameter.
See [Time zone definitions](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#timezone_definitions) for information
on how to specify a time zone.

The current time value is set at the start of the query statement that contains
this function. All invocations of `CURRENT_TIME()` within a query statement
yield the same value.

**Return Data Type**

`TIME`

**Example**

    SELECT CURRENT_TIME() as now;

    /*---+
     | now                        |
     +---+
     | 15:31:38.776361            |
     +---*/

## `EXTRACT`

    EXTRACT(part FROM time_expression)

**Description**

Returns a value that corresponds to the specified `part` from
a supplied `time_expression`.

Allowed `part` values are:

- `MICROSECOND`
- `MILLISECOND`
- `SECOND`
- `MINUTE`
- `HOUR`

Returned values truncate lower order time periods. For example, when extracting
seconds, `EXTRACT` truncates the millisecond and microsecond values.

**Return Data Type**

`INT64`

**Example**

In the following example, `EXTRACT` returns a value corresponding to the `HOUR`
time part.

    SELECT EXTRACT(HOUR FROM TIME "15:30:00") as hour;

    /*---+
     | hour             |
     +---+
     | 15               |
     +---*/

## `FORMAT_TIME`

    FORMAT_TIME(format_string, time_expr)

**Description**

Formats a `TIME` value according to the specified format string.

**Definitions**

- `format_string`: A `STRING` value that contains the [format elements](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/format-elements#format_elements_date_time) to use with `time_expr`.
- `time_expr`: A `TIME` value that represents the time to format.

**Return Data Type**

`STRING`

**Example**

    SELECT FORMAT_TIME("%R", TIME "15:30:00") as formatted_time;

    /*---+
     | formatted_time |
     +---+
     | 15:30          |
     +---*/

## `PARSE_TIME`

    PARSE_TIME(format_string, time_string)

**Description**

Converts a `STRING` value to a `TIME` value.

**Definitions**

- `format_string`: A `STRING` value that contains the [format elements](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/format-elements#format_elements_date_time) to use with `time_string`.
- `time_string`: A `STRING` value that represents the time to parse.

**Details**

Each element in `time_string` must have a corresponding element in
`format_string`. The location of each element in `format_string` must match the
location of each element in `time_string`.

    -- This works because elements on both sides match.
    SELECT PARSE_TIME("%I:%M:%S", "07:30:00");

    -- This produces an error because the seconds element is in different locations.
    SELECT PARSE_TIME("%S:%I:%M", "07:30:00");

    -- This produces an error because one of the seconds elements is missing.
    SELECT PARSE_TIME("%I:%M", "07:30:00");

    -- This works because %T can find all matching elements in time_string.
    SELECT PARSE_TIME("%T", "07:30:00");

The format string fully supports most format elements except for `%P`.

The following additional considerations apply when using the `PARSE_TIME`
function:

- Unspecified fields. Any unspecified field is initialized from `00:00:00.0`. For instance, if `seconds` is unspecified then it defaults to `00`, and so on.
- Whitespace. One or more consecutive white spaces in the format string matches zero or more consecutive white spaces in the `TIME` string. In addition, leading and trailing white spaces in the `TIME` string are always allowed, even if they aren't in the format string.
- Format precedence. When two (or more) format elements have overlapping information, the last one generally overrides any earlier ones.
- Format divergence. `%p` can be used with `am`, `AM`, `pm`, and `PM`.

**Return Data Type**

`TIME`

**Example**

    SELECT PARSE_TIME("%H", "15") as parsed_time;

    /*---+
     | parsed_time |
     +---+
     | 15:00:00    |
     +---*/

    SELECT PARSE_TIME('%I:%M:%S %p', '2:23:38 pm') AS parsed_time;

    /*---+
     | parsed_time |
     +---+
     | 14:23:38    |
     +---*/

## `TIME`

    1. TIME(hour, minute, second)
    2. TIME(timestamp, [time_zone])
    3. TIME(datetime)

**Description**

1. Constructs a `TIME` object using `INT64` values representing the hour, minute, and second.
2. Constructs a `TIME` object using a `TIMESTAMP` object. It supports an optional parameter to [specify a time zone](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/timestamp_functions#timezone_definitions). If no time zone is specified, the default time zone, UTC, is used.
3. Constructs a `TIME` object using a `DATETIME` object.

**Return Data Type**

`TIME`

**Example**

    SELECT
      TIME(15, 30, 00) as time_hms,
      TIME(TIMESTAMP "2008-12-25 15:30:00+08", "America/Los_Angeles") as time_tstz;

    /*---+---+
     | time_hms | time_tstz |
     +---+---+
     | 15:30:00 | 23:30:00  |
     +---+---*/

    SELECT TIME(DATETIME "2008-12-25 15:30:00.000000") AS time_dt;

    /*---+
     | time_dt  |
     +---+
     | 15:30:00 |
     +---*/

## `TIME_ADD`

    TIME_ADD(time_expression, INTERVAL int64_expression part)

**Description**

Adds `int64_expression` units of `part` to the `TIME` object.

`TIME_ADD` supports the following values for `part`:

- `MICROSECOND`
- `MILLISECOND`
- `SECOND`
- `MINUTE`
- `HOUR`

This function automatically adjusts when values fall outside of the 00:00:00 to
24:00:00 boundary. For example, if you add an hour to `23:30:00`, the returned
value is `00:30:00`.

**Return Data Types**

`TIME`

**Example**

    SELECT
      TIME "15:30:00" as original_time,
      TIME_ADD(TIME "15:30:00", INTERVAL 10 MINUTE) as later;

    /*---+---+
     | original_time               | later                  |
     +---+---+
     | 15:30:00                    | 15:40:00               |
     +---+---*/

## `TIME_DIFF`

    TIME_DIFF(end_time, start_time, granularity)

**Description**

Gets the number of unit boundaries between two `TIME` values (`end_time` -
`start_time`) at a particular time granularity.

**Definitions**

- `start_time`: The starting `TIME` value.
- `end_time`: The ending `TIME` value.
- `granularity`: The time part that represents the granularity. If
  you passed in `TIME` values for the first arguments, `granularity` can
  be:

  - `MICROSECOND`
  - `MILLISECOND`
  - `SECOND`
  - `MINUTE`
  - `HOUR`

**Details**

If `end_time` is earlier than `start_time`, the output is negative.
Produces an error if the computation overflows, such as if the difference
in microseconds
between the two `TIME` values overflows.

> [!NOTE]
> **Note:** The behavior of the this function follows the type of arguments passed in. For example, `TIME_DIFF(TIMESTAMP, TIMESTAMP, PART)` behaves like `TIMESTAMP_DIFF(TIMESTAMP, TIMESTAMP, PART)`.

**Return Data Type**

`INT64`

**Example**

    SELECT
      TIME "15:30:00" as first_time,
      TIME "14:35:00" as second_time,
      TIME_DIFF(TIME "15:30:00", TIME "14:35:00", MINUTE) as difference;

    /*---+---+---+
     | first_time                 | second_time            | difference             |
     +---+---+---+
     | 15:30:00                   | 14:35:00               | 55                     |
     +---+---+---*/

## `TIME_SUB`

    TIME_SUB(time_expression, INTERVAL int64_expression part)

**Description**

Subtracts `int64_expression` units of `part` from the `TIME` object.

`TIME_SUB` supports the following values for `part`:

- `MICROSECOND`
- `MILLISECOND`
- `SECOND`
- `MINUTE`
- `HOUR`

This function automatically adjusts when values fall outside of the 00:00:00 to
24:00:00 boundary. For example, if you subtract an hour from `00:30:00`, the
returned value is `23:30:00`.

**Return Data Type**

`TIME`

**Example**

    SELECT
      TIME "15:30:00" as original_date,
      TIME_SUB(TIME "15:30:00", INTERVAL 10 MINUTE) as earlier;

    /*---+---+
     | original_date               | earlier                |
     +---+---+
     | 15:30:00                    | 15:20:00               |
     +---+---*/

## `TIME_TRUNC`

    TIME_TRUNC(time_value, time_granularity)

**Description**

Truncates a `TIME` value at a particular granularity.

**Definitions**

- `time_value`: The `TIME` value to truncate.
- `time_granularity`: The truncation granularity for a `TIME` value. [Time granularities](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/time_functions#time_trunc_granularity_time) can be used.

**Time granularity definitions**

- `MICROSECOND`: If used, nothing is truncated from the value.

- `MILLISECOND`: The nearest lesser than or equal millisecond.

- `SECOND`: The nearest lesser than or equal second.

- `MINUTE`: The nearest lesser than or equal minute.

- `HOUR`: The nearest lesser than or equal hour.

**Details**

The resulting value is always rounded to the beginning of `granularity`.

**Return Data Type**

`TIME`

**Example**

    SELECT
      TIME "15:30:00" as original,
      TIME_TRUNC(TIME "15:30:00", HOUR) as truncated;

    /*---+---+
     | original                   | truncated              |
     +---+---+
     | 15:30:00                   | 15:00:00               |
     +---+---*/