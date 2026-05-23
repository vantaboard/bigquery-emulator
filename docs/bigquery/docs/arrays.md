In GoogleSQL for BigQuery, an array is an ordered list consisting of zero or more
values of the same data type. You can construct arrays of a simple data type,
such as `INT64`, or a complex data type, such as `STRUCT`. However,
arrays of arrays aren't supported. To learn more about the `ARRAY`
data type, including
`NULL` handling, see [Array type](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#array_type).

With GoogleSQL, you can construct array literals,
build arrays from subqueries using the
[`ARRAY`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/array_functions) function,
and aggregate values into an array using the
[`ARRAY_AGG`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate_functions#array_agg)
function.

You can combine arrays using functions like
`ARRAY_CONCAT()`, and convert arrays to strings using `ARRAY_TO_STRING()`.

## Accessing array elements

Consider the following table called `Sequences`. This table contains
the column `some_numbers` of the `ARRAY` data type.

    WITH
      Sequences AS (
        SELECT [0, 1, 1, 2, 3, 5] AS some_numbers UNION ALL
        SELECT [2, 4, 8, 16, 32] UNION ALL
        SELECT [5, 10]
      )
    SELECT * FROM Sequences;

    /*---+
     | some_numbers        |
     +---+
     | [0, 1, 1, 2, 3, 5]  |
     | [2, 4, 8, 16, 32]   |
     | [5, 10]             |
     +---*/

To access array elements in the `some_numbers` column, specify which
type of indexing you want to use:
either [`index`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/operators#array_subscript_operator)
or [`OFFSET(index)`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/operators#array_subscript_operator) for
zero-based indexes, or [`ORDINAL(index)`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/operators#array_subscript_operator) for
one-based indexes:

    SELECT
      some_numbers,
      some_numbers[0] AS index_0,
      some_numbers[OFFSET(1)] AS offset_1,
      some_numbers[ORDINAL(1)] AS ordinal_1
    FROM Sequences;

    /*---+---+---+---+
     | some_numbers       | index_0 | offset_1 | ordinal_1 |
     +---+---+---+---+
     | [0, 1, 1, 2, 3, 5] | 0       | 1        | 0         |
     | [2, 4, 8, 16, 32]  | 2       | 4        | 2         |
     | [5, 10]            | 5       | 10       | 5         |
     +---+---+---+---*/

> [!NOTE]
> **Note:** `OFFSET` and `ORDINAL` will raise errors if the index is out of range. To avoid this, you can use `SAFE_OFFSET` or `SAFE_ORDINAL` to return `NULL` instead of raising an error.

To access the first or last element in an array, use the
[`ARRAY_FIRST`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/array_functions#array_first) or [`ARRAY_LAST`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/array_functions#array_last)
function:

    SELECT
      some_numbers,
      ARRAY_FIRST(some_numbers) AS first_element,
      ARRAY_LAST(some_numbers) AS last_element
    FROM Sequences;

    /*---+---+---+
     | some_numbers       | first_element | last_element |
     +---+---+---+
     | [0, 1, 1, 2, 3, 5] | 0             | 5            |
     | [2, 4, 8, 16, 32]  | 2             | 32           |
     | [5, 10]            | 5             | 10           |
     +---+---+---*/

With the `ARRAY_FIRST` and `ARRAY_LAST` functions, if an array is
empty, the function produces an error:

    WITH
      Sequences AS (
        SELECT [0, 1, 1, 2, 3, 5] AS some_numbers
        UNION ALL
        SELECT [2, 4, 8, 16, 32]
        UNION ALL
        SELECT [] -- Empty array
      )
    SELECT
      some_numbers,
      ARRAY_LAST(some_numbers) AS last_element
    FROM Sequences;

    -- Error: ARRAY_LAST can't get the last element of an empty array.

To handle empty arrays when accessing first and last elements, you can use the
[`ARRAY_LENGTH`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/array_functions#array_length) function within a `SAFE_OFFSET` of `-1`.
The query returns `NULL` values for any empty arrays instead of an error:

    SELECT
      some_numbers,
      some_numbers[SAFE_OFFSET(ARRAY_LENGTH(some_numbers) - 1)] AS last_element
    FROM Sequences;

    /*---+---+
     | some_numbers       | last_element |
     +---+---+
     | [0, 1, 1, 2, 3, 5] | 5            |
     | [2, 4, 8, 16, 32]  | 32           |
     | []                 | NULL         |
     +---+---*/

`ARRAY_LENGTH(array)` returns the number of elements in the array. Because array
offsets are 0-based, `ARRAY_LENGTH(array) - 1` gives the offset of the last
element. If the array is empty, `ARRAY_LENGTH` is 0, and the offset becomes -1.
`SAFE_OFFSET(-1)` returns `NULL`, so this approach safely handles empty arrays.

## Finding lengths

The [`ARRAY_LENGTH`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/array_functions#array_length) function returns the length of an array.

    WITH Sequences AS
      (SELECT [0, 1, 1, 2, 3, 5] AS some_numbers
       UNION ALL SELECT [2, 4, 8, 16, 32] AS some_numbers
       UNION ALL SELECT [5, 10] AS some_numbers)
    SELECT some_numbers,
           ARRAY_LENGTH(some_numbers) AS len
    FROM Sequences;

    /*---+---+
     | some_numbers       | len    |
     +---+---+
     | [0, 1, 1, 2, 3, 5] | 6      |
     | [2, 4, 8, 16, 32]  | 5      |
     | [5, 10]            | 2      |
     +---+---*/

## Converting elements in an array to rows in a table

To convert an `ARRAY` into a set of rows, also known as "flattening," use the
[`UNNEST`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#unnest_operator)
operator. `UNNEST` takes an `ARRAY` and returns a table with a single row for
each element in the `ARRAY`.

Because `UNNEST` destroys the order of the `ARRAY` elements, you may
wish to restore order to the table. To do so, use the optional `WITH OFFSET`
clause to return an additional column with the offset for each array element,
then use the `ORDER BY` clause to order the rows by their offset.

**Example**

    SELECT *
    FROM UNNEST(['foo', 'bar', 'baz', 'qux', 'corge', 'garply', 'waldo', 'fred'])
      AS element
    WITH OFFSET AS offset
    ORDER BY offset;

    /*---+---+
     | element  | offset |
     +---+---+
     | foo      | 0      |
     | bar      | 1      |
     | baz      | 2      |
     | qux      | 3      |
     | corge    | 4      |
     | garply   | 5      |
     | waldo    | 6      |
     | fred     | 7      |
     +---+---*/

To flatten an entire column of type `ARRAY` while preserving the values of the other
columns in each row, use a correlated [`INNER JOIN`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#inner_join) to join
the table containing the `ARRAY` column to the `UNNEST` output of that `ARRAY`
column.

With a [correlated](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#correlated_join) join, the `UNNEST` operator
references the `ARRAY` typed column from each row in the source table, which
appears previously in the `FROM` clause. For each row `N` in the source table,
`UNNEST` flattens the `ARRAY` from row `N` into a set of rows containing the
`ARRAY` elements, and then a correlated `INNER JOIN` or `CROSS JOIN` combines
this new set of rows with the single row `N` from the source table.

**Examples**

The following example uses [`UNNEST`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#unnest_operator) to return a row for each
element in the array column. Because of the `INNER JOIN`, the `id` column
contains the `id` values for the row in `Sequences` that contains each number.

    WITH
      Sequences AS (
        SELECT 1 AS id, [0, 1, 1, 2, 3, 5] AS some_numbers
        UNION ALL SELECT 2 AS id, [2, 4, 8, 16, 32] AS some_numbers
        UNION ALL SELECT 3 AS id, [5, 10] AS some_numbers
      )
    SELECT id, flattened_numbers
    FROM Sequences
    INNER JOIN UNNEST(Sequences.some_numbers) AS flattened_numbers;

    /*---+---+
     | id   | flattened_numbers |
     +---+---+
     |    1 |                 0 |
     |    1 |                 1 |
     |    1 |                 1 |
     |    1 |                 2 |
     |    1 |                 3 |
     |    1 |                 5 |
     |    2 |                 2 |
     |    2 |                 4 |
     |    2 |                 8 |
     |    2 |                16 |
     |    2 |                32 |
     |    3 |                 5 |
     |    3 |                10 |
     +---+---*/

Note that for correlated joins the `UNNEST` operator is optional and the
`INNER JOIN` can be expressed as a `CROSS JOIN` or a comma cross join. Using the
comma cross join shorthand notation, the previous example is consolidated as
follows:

    WITH
      Sequences AS (
        SELECT 1 AS id, [0, 1, 1, 2, 3, 5] AS some_numbers
        UNION ALL SELECT 2 AS id, [2, 4, 8, 16, 32] AS some_numbers
        UNION ALL SELECT 3 AS id, [5, 10] AS some_numbers
      )
    SELECT id, flattened_numbers
    FROM Sequences, Sequences.some_numbers AS flattened_numbers;

    /*---+---+
     | id   | flattened_numbers |
     +---+---+
     |    1 |                 0 |
     |    1 |                 1 |
     |    1 |                 1 |
     |    1 |                 2 |
     |    1 |                 3 |
     |    1 |                 5 |
     |    2 |                 2 |
     |    2 |                 4 |
     |    2 |                 8 |
     |    2 |                16 |
     |    2 |                32 |
     |    3 |                 5 |
     |    3 |                10 |
     +---+---*/

## Querying nested arrays

If a table contains an `ARRAY` of `STRUCT`s, you can
[flatten the `ARRAY`](https://docs.cloud.google.com/bigquery/docs/arrays#flattening_arrays) to query the fields of the `STRUCT`.
You can also flatten `ARRAY` type fields of `STRUCT` values.

### Querying `STRUCT` elements in an array

The following example uses `UNNEST` with `INNER JOIN` to flatten an `ARRAY` of
`STRUCT`s.

    WITH
      Races AS (
        SELECT
          "800M" AS race,
          [
            STRUCT("Rudisha" AS name, [23.4, 26.3, 26.4, 26.1] AS laps),
            STRUCT("Makhloufi" AS name, [24.5, 25.4, 26.6, 26.1] AS laps),
            STRUCT("Murphy" AS name, [23.9, 26.0, 27.0, 26.0] AS laps),
            STRUCT("Bosse" AS name, [23.6, 26.2, 26.5, 27.1] AS laps),
            STRUCT("Rotich" AS name, [24.7, 25.6, 26.9, 26.4] AS laps),
            STRUCT("Lewandowski" AS name, [25.0, 25.7, 26.3, 27.2] AS laps),
            STRUCT("Kipketer" AS name, [23.2, 26.1, 27.3, 29.4] AS laps),
            STRUCT("Berian" AS name, [23.7, 26.1, 27.0, 29.3] AS laps)
          ] AS participants
        )
    SELECT
      race,
      participant
    FROM Races AS r
    INNER JOIN UNNEST(r.participants) AS participant;

    /*---+---+
     | race | participant                           |
     +---+---+
     | 800M | {Rudisha, [23.4, 26.3, 26.4, 26.1]}   |
     | 800M | {Makhloufi, [24.5, 25.4, 26.6, 26.1]} |
     | 800M | {Murphy, [23.9, 26, 27, 26]}          |
     | 800M | {Bosse, [23.6, 26.2, 26.5, 27.1]}     |
     | 800M | {Rotich, [24.7, 25.6, 26.9, 26.4]}    |
     | 800M | {Lewandowski, [25, 25.7, 26.3, 27.2]} |
     | 800M | {Kipketer, [23.2, 26.1, 27.3, 29.4]}  |
     | 800M | {Berian, [23.7, 26.1, 27, 29.3]}      |
     +---+---*/

You can find specific information from repeated fields. For example, the
following query returns the fastest racer in an 800M race.

> [!NOTE]
> **Note:** This example doesn't involve flattening an array, but does represent a common way to get information from a repeated field.

**Example**

    WITH
      Races AS (
        SELECT
          "800M" AS race,
          [
            STRUCT("Rudisha" AS name, [23.4, 26.3, 26.4, 26.1] AS laps),
            STRUCT("Makhloufi" AS name, [24.5, 25.4, 26.6, 26.1] AS laps),
            STRUCT("Murphy" AS name, [23.9, 26.0, 27.0, 26.0] AS laps),
            STRUCT("Bosse" AS name, [23.6, 26.2, 26.5, 27.1] AS laps),
            STRUCT("Rotich" AS name, [24.7, 25.6, 26.9, 26.4] AS laps),
            STRUCT("Lewandowski" AS name, [25.0, 25.7, 26.3, 27.2] AS laps),
            STRUCT("Kipketer" AS name, [23.2, 26.1, 27.3, 29.4] AS laps),
            STRUCT("Berian" AS name, [23.7, 26.1, 27.0, 29.3] AS laps)
          ] AS participants
      )
    SELECT
      race,
      (
        SELECT name
        FROM UNNEST(participants)
        ORDER BY (SELECT SUM(duration) FROM UNNEST(laps) AS duration) ASC
        LIMIT 1
      ) AS fastest_racer
    FROM Races;

    /*---+---+
     | race | fastest_racer |
     +---+---+
     | 800M | Rudisha       |
     +---+---*/

### Querying `ARRAY`-type fields in a struct

You can also get information from nested repeated fields. For example, the
following statement returns the runner who had the fastest lap in an 800M race.

    WITH
      Races AS (
        SELECT
          "800M" AS race,
          [
            STRUCT("Rudisha" AS name, [23.4, 26.3, 26.4, 26.1] AS laps),
            STRUCT("Makhloufi" AS name, [24.5, 25.4, 26.6, 26.1] AS laps),
            STRUCT("Murphy" AS name, [23.9, 26.0, 27.0, 26.0] AS laps),
            STRUCT("Bosse" AS name, [23.6, 26.2, 26.5, 27.1] AS laps),
            STRUCT("Rotich" AS name, [24.7, 25.6, 26.9, 26.4] AS laps),
            STRUCT("Lewandowski" AS name, [25.0, 25.7, 26.3, 27.2] AS laps),
            STRUCT("Kipketer" AS name, [23.2, 26.1, 27.3, 29.4] AS laps),
            STRUCT("Berian" AS name, [23.7, 26.1, 27.0, 29.3] AS laps)
          ]AS participants
      )
    SELECT
      race,
      (
        SELECT name
        FROM UNNEST(participants), UNNEST(laps) AS duration
        ORDER BY duration ASC
        LIMIT 1
      ) AS runner_with_fastest_lap
    FROM Races;

    /*---+---+
     | race | runner_with_fastest_lap |
     +---+---+
     | 800M | Kipketer                |
     +---+---*/

Notice that the preceding query uses the comma operator (`,`) to perform a cross
join and flatten the array. This is equivalent to using an explicit
`CROSS JOIN`, or the following example which uses an explicit `INNER JOIN`:

    WITH
      Races AS (
        SELECT "800M" AS race,
          [
            STRUCT("Rudisha" AS name, [23.4, 26.3, 26.4, 26.1] AS laps),
            STRUCT("Makhloufi" AS name, [24.5, 25.4, 26.6, 26.1] AS laps),
            STRUCT("Murphy" AS name, [23.9, 26.0, 27.0, 26.0] AS laps),
            STRUCT("Bosse" AS name, [23.6, 26.2, 26.5, 27.1] AS laps),
            STRUCT("Rotich" AS name, [24.7, 25.6, 26.9, 26.4] AS laps),
            STRUCT("Lewandowski" AS name, [25.0, 25.7, 26.3, 27.2] AS laps),
            STRUCT("Kipketer" AS name, [23.2, 26.1, 27.3, 29.4] AS laps),
            STRUCT("Berian" AS name, [23.7, 26.1, 27.0, 29.3] AS laps)
          ] AS participants
      )
    SELECT
      race,
      (
        SELECT name
        FROM UNNEST(participants)
        INNER JOIN UNNEST(laps) AS duration
        ORDER BY duration ASC LIMIT 1
      ) AS runner_with_fastest_lap
    FROM Races;

    /*---+---+
     | race | runner_with_fastest_lap |
     +---+---+
     | 800M | Kipketer                |
     +---+---*/

Flattening arrays with `INNER JOIN` excludes rows that have empty or `NULL`
arrays. If you want to include these rows, use `LEFT JOIN`.

    WITH
      Races AS (
        SELECT
          "800M" AS race,
          [
            STRUCT("Rudisha" AS name, [23.4, 26.3, 26.4, 26.1] AS laps),
            STRUCT("Makhloufi" AS name, [24.5, 25.4, 26.6, 26.1] AS laps),
            STRUCT("Murphy" AS name, [23.9, 26.0, 27.0, 26.0] AS laps),
            STRUCT("Bosse" AS name, [23.6, 26.2, 26.5, 27.1] AS laps),
            STRUCT("Rotich" AS name, [24.7, 25.6, 26.9, 26.4] AS laps),
            STRUCT("Lewandowski" AS name, [25.0, 25.7, 26.3, 27.2] AS laps),
            STRUCT("Kipketer" AS name, [23.2, 26.1, 27.3, 29.4] AS laps),
            STRUCT("Berian" AS name, [23.7, 26.1, 27.0, 29.3] AS laps),
            STRUCT("Nathan" AS name, ARRAY<FLOAT64>[] AS laps),
            STRUCT("David" AS name, NULL AS laps)
          ] AS participants
      )
    SELECT
      Participant.name,
      SUM(duration) AS finish_time
    FROM Races
    INNER JOIN Races.participants AS Participant
    LEFT JOIN Participant.laps AS duration
    GROUP BY name;

    /*---+---+
     | name        | finish_time        |
     +---+---+
     | Murphy      | 102.9              |
     | Rudisha     | 102.19999999999999 |
     | David       | NULL               |
     | Rotich      | 103.6              |
     | Makhloufi   | 102.6              |
     | Berian      | 106.1              |
     | Bosse       | 103.4              |
     | Kipketer    | 106                |
     | Nathan      | NULL               |
     | Lewandowski | 104.2              |
     +---+---*/

## Constructing arrays

You can construct an array using array literals or array functions. To learn
more about constructing arrays, see [Array type](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#constructing_an_array).

## Creating arrays from subqueries

A common task when working with arrays is turning a subquery result into an
array. In GoogleSQL, you can accomplish this using the
[`ARRAY()`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/array_functions) function.

For example, consider the following operation on the `Sequences` table:

    WITH Sequences AS
      (SELECT [0, 1, 1, 2, 3, 5] AS some_numbers
      UNION ALL SELECT [2, 4, 8, 16, 32] AS some_numbers
      UNION ALL SELECT [5, 10] AS some_numbers)
    SELECT some_numbers,
      ARRAY(SELECT x * 2
            FROM UNNEST(some_numbers) AS x) AS doubled
    FROM Sequences;

    /*---+---+
     | some_numbers       | doubled             |
     +---+---+
     | [0, 1, 1, 2, 3, 5] | [0, 2, 2, 4, 6, 10] |
     | [2, 4, 8, 16, 32]  | [4, 8, 16, 32, 64]  |
     | [5, 10]            | [10, 20]            |
     +---+---*/

This example starts with a table named Sequences. This table contains a column,
`some_numbers`, of type `ARRAY<INT64>`.

The query itself contains a subquery. This subquery selects each row in the
`some_numbers` column and uses
[`UNNEST`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#unnest_operator) to return the
array as a set of rows. Next, it multiplies each value by two, and then
re-combines the rows back into an array using the `ARRAY()` operator.

## Filtering arrays

The following example uses a `WHERE` clause in the `ARRAY()` operator's subquery
to filter the returned rows.

> [!NOTE]
> **Note:** In the following examples, the resulting rows are not ordered.

    WITH Sequences AS
      (SELECT [0, 1, 1, 2, 3, 5] AS some_numbers
       UNION ALL SELECT [2, 4, 8, 16, 32] AS some_numbers
       UNION ALL SELECT [5, 10] AS some_numbers)
    SELECT
      ARRAY(SELECT x * 2
            FROM UNNEST(some_numbers) AS x
            WHERE x < 5) AS doubled_less_than_five
    FROM Sequences;

    /*---+
     | doubled_less_than_five |
     +---+
     | [0, 2, 2, 4, 6]        |
     | [4, 8]                 |
     | []                     |
     +---*/

Notice that the third row contains an empty array, because the elements in the
corresponding original row (`[5, 10]`) didn't meet the filter requirement of
`x < 5`.

You can also filter arrays by using `SELECT DISTINCT` to return only
unique elements within an array.

    WITH Sequences AS
      (SELECT [0, 1, 1, 2, 3, 5] AS some_numbers)
    SELECT ARRAY(SELECT DISTINCT x
                 FROM UNNEST(some_numbers) AS x) AS unique_numbers
    FROM Sequences;

    /*---+
     | unique_numbers  |
     +---+
     | [0, 1, 2, 3, 5] |
     +---*/

You can also filter rows of arrays by using the
[`IN`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/operators#in_operators) keyword. This
keyword filters rows containing arrays by determining if a specific
value matches an element in the array.

    WITH Sequences AS
      (SELECT [0, 1, 1, 2, 3, 5] AS some_numbers
       UNION ALL SELECT [2, 4, 8, 16, 32] AS some_numbers
       UNION ALL SELECT [5, 10] AS some_numbers)
    SELECT
       ARRAY(SELECT x
             FROM UNNEST(some_numbers) AS x
             WHERE 2 IN UNNEST(some_numbers)) AS contains_two
    FROM Sequences;

    /*---+
     | contains_two       |
     +---+
     | [0, 1, 1, 2, 3, 5] |
     | [2, 4, 8, 16, 32]  |
     | []                 |
     +---*/

Notice again that the third row contains an empty array, because the array in
the corresponding original row (`[5, 10]`) didn't contain `2`.

## Scanning arrays

To check if an array contains a specific value, use the [`IN`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/operators#in_operators)
operator with [`UNNEST`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#unnest_operator). To check if an array contains a value
matching a condition, use the [`EXISTS`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/operators#exists_operator) operator with
`UNNEST`.

### Scanning for specific values

To scan an array for a specific value, use the `IN` operator with `UNNEST`.

**Example**

The following example returns `true` if the array contains the number 2.

    SELECT 2 IN UNNEST([0, 1, 1, 2, 3, 5]) AS contains_value;

    /*---+
     | contains_value |
     +---+
     | true           |
     +---*/

To return the rows of a table where the array column contains a specific value,
filter the results of `IN UNNEST` using the `WHERE` clause.

**Example**

The following example returns the `id` value for the rows where the array
column contains the value 2.

    WITH Sequences AS
      (SELECT 1 AS id, [0, 1, 1, 2, 3, 5] AS some_numbers
       UNION ALL SELECT 2 AS id, [2, 4, 8, 16, 32] AS some_numbers
       UNION ALL SELECT 3 AS id, [5, 10] AS some_numbers)
    SELECT id AS matching_rows
    FROM Sequences
    WHERE 2 IN UNNEST(Sequences.some_numbers)
    ORDER BY matching_rows;

    /*---+
     | matching_rows |
     +---+
     | 1             |
     | 2             |
     +---*/

### Scanning for values that satisfy a condition

To scan an array for values that match a condition, use `UNNEST` to return a
table of the elements in the array, use `WHERE` to filter the resulting table in
a subquery, and use `EXISTS` to check if the filtered table contains any rows.

**Example**

The following example returns the `id` value for the rows where the array
column contains values greater than 5.

    WITH
      Sequences AS (
        SELECT 1 AS id, [0, 1, 1, 2, 3, 5] AS some_numbers
        UNION ALL
        SELECT 2 AS id, [2, 4, 8, 16, 32] AS some_numbers
        UNION ALL
        SELECT 3 AS id, [5, 10] AS some_numbers
      )
    SELECT id AS matching_rows
    FROM Sequences
    WHERE EXISTS(SELECT * FROM UNNEST(some_numbers) AS x WHERE x > 5);

    /*---+
     | matching_rows |
     +---+
     | 2             |
     | 3             |
     +---*/

#### Scanning for `STRUCT` field values that satisfy a condition

To search an array of `STRUCT` values for a field whose value matches a condition, use
`UNNEST` to return a table with a column for each `STRUCT` field, then filter
non-matching rows from the table using `WHERE EXISTS`.

**Example**

The following example returns the rows where the array column contains a
`STRUCT` whose field `b` has a value greater than 3.

    WITH
      Sequences AS (
        SELECT 1 AS id, [STRUCT(0 AS a, 1 AS b)] AS some_numbers
        UNION ALL
        SELECT 2 AS id, [STRUCT(2 AS a, 4 AS b)] AS some_numbers
        UNION ALL
        SELECT 3 AS id, [STRUCT(5 AS a, 3 AS b), STRUCT(7 AS a, 4 AS b)] AS some_numbers
      )
    SELECT id AS matching_rows
    FROM Sequences
    WHERE EXISTS(SELECT 1 FROM UNNEST(some_numbers) WHERE b > 3);

    /*---+
     | matching_rows |
     +---+
     | 2             |
     | 3             |
     +---*/

## Arrays and aggregation

With GoogleSQL, you can aggregate values into an array using
`ARRAY_AGG()`.

    WITH Fruits AS
      (SELECT "apple" AS fruit
       UNION ALL SELECT "pear" AS fruit
       UNION ALL SELECT "banana" AS fruit)
    SELECT ARRAY_AGG(fruit) AS fruit_basket
    FROM Fruits;

    /*---+
     | fruit_basket          |
     +---+
     | [apple, pear, banana] |
     +---*/

The array returned by `ARRAY_AGG()` is in an arbitrary order, since the order in
which the function concatenates values isn't guaranteed. To order the array
elements, use `ORDER BY`:

    WITH Fruits AS
      (SELECT "apple" AS fruit
       UNION ALL SELECT "pear" AS fruit
       UNION ALL SELECT "banana" AS fruit)
    SELECT ARRAY_AGG(fruit ORDER BY fruit) AS fruit_basket
    FROM Fruits;

    /*---+
     | fruit_basket          |
     +---+
     | [apple, banana, pear] |
     +---*/

You can also apply aggregate functions such as `SUM()` to the elements in an
array. For example, the following query returns the sum of array elements for
each row of the `Sequences` table.

    WITH Sequences AS
      (SELECT [0, 1, 1, 2, 3, 5] AS some_numbers
       UNION ALL SELECT [2, 4, 8, 16, 32] AS some_numbers
       UNION ALL SELECT [5, 10] AS some_numbers)
    SELECT some_numbers,
      (SELECT SUM(x)
       FROM UNNEST(s.some_numbers) AS x) AS sums
    FROM Sequences AS s;

    /*---+---+
     | some_numbers       | sums |
     +---+---+
     | [0, 1, 1, 2, 3, 5] | 12   |
     | [2, 4, 8, 16, 32]  | 62   |
     | [5, 10]            | 15   |
     +---+---*/

GoogleSQL also supports an aggregate function, `ARRAY_CONCAT_AGG()`,
which concatenates the elements of an array column across rows.

    WITH Aggregates AS
      (SELECT [1,2] AS numbers
       UNION ALL SELECT [3,4] AS numbers
       UNION ALL SELECT [5, 6] AS numbers)
    SELECT ARRAY_CONCAT_AGG(numbers) AS count_to_six_agg
    FROM Aggregates;

    /*---+
     | count_to_six_agg                                 |
     +---+
     | [1, 2, 3, 4, 5, 6]                               |
     +---*/

> [!NOTE]
> **Note:** The array returned by `ARRAY_CONCAT_AGG()` is non-deterministic, since the order in which the function concatenates values is not guaranteed.

## Converting arrays to strings

The `ARRAY_TO_STRING()` function allows you to convert an `ARRAY<STRING>` to a
single `STRING` value or an `ARRAY<BYTES>` to a single `BYTES` value where the
resulting value is the ordered concatenation of the array elements.

The second argument is the separator that the function will insert between
inputs to produce the output; this second argument must be of the same
type as the elements of the first argument.

Example:

    WITH Words AS
      (SELECT ["Hello", "World"] AS greeting)
    SELECT ARRAY_TO_STRING(greeting, " ") AS greetings
    FROM Words;

    /*---+
     | greetings   |
     +---+
     | Hello World |
     +---*/

The optional third argument takes the place of `NULL` values in the input
array.

- If you omit this argument, then the function ignores `NULL` array
  elements.

- If you provide an empty string, the function inserts a
  separator for `NULL` array elements.

Example:

    SELECT
      ARRAY_TO_STRING(arr, ".", "N") AS non_empty_string,
      ARRAY_TO_STRING(arr, ".", "") AS empty_string,
      ARRAY_TO_STRING(arr, ".") AS omitted
    FROM (SELECT ["a", NULL, "b", NULL, "c", NULL] AS arr);

    /*---+---+---+
     | non_empty_string | empty_string | omitted |
     +---+---+---+
     | a.N.b.N.c.N      | a..b..c.     | a.b.c   |
     +---+---+---*/

## Combining arrays

In some cases, you might want to combine multiple arrays into a single array.
You can accomplish this using the `ARRAY_CONCAT()` function.

    SELECT ARRAY_CONCAT([1, 2], [3, 4], [5, 6]) AS count_to_six;

    /*---+
     | count_to_six                                     |
     +---+
     | [1, 2, 3, 4, 5, 6]                               |
     +---*/

## Updating arrays

Consider the following table called `arrays_table`. The first column in the
table is an array of integers and the second column contains two nested arrays
of integers.

    WITH arrays_table AS (
      SELECT
        [1, 2] AS regular_array,
        STRUCT([10, 20] AS first_array, [100, 200] AS second_array) AS nested_arrays
      UNION ALL SELECT
        [3, 4] AS regular_array,
        STRUCT([30, 40] AS first_array, [300, 400] AS second_array) AS nested_arrays
    )
    SELECT * FROM arrays_table;

    /*---*---*---+
     | regular_array | nested_arrays.first_array | nested_arrays.second_array |
     +---+---+---+
     | [1, 2]        | [10, 20]                  | [100, 200]                 |
     | [3, 4]        | [30, 40]                  | [130, 400]                 |
     +---*---*---*/

You can update arrays in a table by using the `UPDATE` statement. The following
example inserts the number 5 into the `regular_array` column,
and inserts the elements from the `first_array` field of the `nested_arrays`
column into the `second_array` field:

    UPDATE
      arrays_table
    SET
      regular_array = ARRAY_CONCAT(regular_array, [5]),
      nested_arrays.second_array = ARRAY_CONCAT(nested_arrays.second_array,
                                                nested_arrays.first_array)
    WHERE TRUE;
    SELECT * FROM arrays_table;

    /*---*---*---+
     | regular_array | nested_arrays.first_array | nested_arrays.second_array |
     +---+---+---+
     | [1, 2, 5]     | [10, 20]                  | [100, 200, 10, 20]         |
     | [3, 4, 5]     | [30, 40]                  | [130, 400, 30, 40]         |
     +---*---*---*/

## Zipping arrays

Given two arrays of equal size, you can merge them into a single array
consisting of pairs of elements from input arrays, taken from their
corresponding positions. This operation is sometimes called
[zipping](https://en.wikipedia.org/wiki/Convolution_(computer_science)).

You can zip arrays with `UNNEST` and `WITH OFFSET`. In this example, each value
pair is stored as a `STRUCT` in an array.

    WITH
      Combinations AS (
        SELECT
          ['a', 'b'] AS letters,
          [1, 2, 3] AS numbers
      )
    SELECT
      ARRAY(
        SELECT AS STRUCT
          letters[SAFE_OFFSET(index)] AS letter,
          numbers[SAFE_OFFSET(index)] AS number
        FROM Combinations
        INNER JOIN
          UNNEST(
            GENERATE_ARRAY(
              0,
              LEAST(ARRAY_LENGTH(letters), ARRAY_LENGTH(numbers)) - 1)) AS index
        ORDER BY index
      ) AS pairs;

    /*---+
     | pairs                        |
     +---+
     | [{ letter: "a", number: 1 }, |
     |  { letter: "b", number: 2 }] |
     +---*/

You can use input arrays of different lengths as long as the first array
is equal to or less than the length of the second array. The zipped array
will be the length of the shortest input array.

To get a zipped array that includes all the elements even when the input arrays
are different lengths, change `LEAST` to `GREATEST`. Elements of either array
that have no associated element in the other array will be paired with `NULL`.

    WITH
      Combinations AS (
        SELECT
          ['a', 'b'] AS letters,
          [1, 2, 3] AS numbers
      )
    SELECT
      ARRAY(
        SELECT AS STRUCT
          letters[SAFE_OFFSET(index)] AS letter,
          numbers[SAFE_OFFSET(index)] AS number
        FROM Combinations
        INNER JOIN
          UNNEST(
            GENERATE_ARRAY(
              0,
              GREATEST(ARRAY_LENGTH(letters), ARRAY_LENGTH(numbers)) - 1)) AS index
        ORDER BY index
      ) AS pairs;

    /*---+
     | pairs                         |
     +---+
     | [{ letter: "a", number: 1 },  |
     |  { letter: "b", number: 2 },  |
     |  { letter: null, number: 3 }] |
     +---*/

## Building arrays of arrays

GoogleSQL doesn't support building
[arrays of arrays](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#array_type)
directly. Instead, you must create an array of structs, with each struct
containing a field of type `ARRAY`. To illustrate this, consider the following
`Points` table:

    /*---+
     | point    |
     +---+
     | [1, 5]   |
     | [2, 8]   |
     | [3, 7]   |
     | [4, 1]   |
     | [5, 7]   |
     +---*/

Now, let's say you wanted to create an array consisting of each `point` in the
`Points` table. To accomplish this, wrap the array returned from each row in a
`STRUCT`, as shown below.

    WITH Points AS
      (SELECT [1, 5] AS point
       UNION ALL SELECT [2, 8] AS point
       UNION ALL SELECT [3, 7] AS point
       UNION ALL SELECT [4, 1] AS point
       UNION ALL SELECT [5, 7] AS point)
    SELECT ARRAY(
      SELECT STRUCT(point)
      FROM Points)
      AS coordinates;

    /*---+
     | coordinates       |
     +---+
     | [{point: [1,5]},  |
     |  {point: [2,8]},  |
     |  {point: [5,7]},  |
     |  {point: [3,7]},  |
     |  {point: [4,1]}]  |
     +---*/