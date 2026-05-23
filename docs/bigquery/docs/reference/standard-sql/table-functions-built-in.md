GoogleSQL for BigQuery supports built-in table functions.

This topic includes functions that produce columns of a table.
You can only use these functions in the `FROM` clause.

## Function list

| Name | Summary |
|---|---|
| [`APPENDS`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/time-series-functions#appends) | Returns all rows appended to a table for a given time range. For more information, see [Time series functions](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/time-series-functions). |
| [`CHANGES`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/time-series-functions#changes) | Returns all rows that have changed in a table for a given time range. For more information, see [Time series functions](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/time-series-functions). |
| [`EXTERNAL_OBJECT_TRANSFORM`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/table-functions-built-in#external_object_transform) | Produces an object table with the original columns plus one or more additional columns. |
| [`GAP_FILL`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/time-series-functions#gap_fill) | Finds and fills gaps in a time series. For more information, see [Time series functions](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/time-series-functions). |
| [`RANGE_SESSIONIZE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/range-functions#range_sessionize) | Produces a table of sessionized ranges. For more information, see [Range functions](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/range-functions). |

## `EXTERNAL_OBJECT_TRANSFORM`

    EXTERNAL_OBJECT_TRANSFORM(TABLE object_table_name, transform_types_array)

**Description**

This function returns a transformed object table with the original columns plus
one or more additional columns, depending on the `transform_types` values
specified.

This function only supports
[object tables](https://cloud.google.com/bigquery/docs/object-table-introduction)
as inputs. Subqueries or any other types of tables aren't supported.

`object_table_name` is the name of the object table to be transformed, in
the format `dataset_name.object_table_name`.

`transform_types_array` is an array of `STRING` literals. Currently, the only
supported `transform_types_array` value is `SIGNED_URL`. Specifying `SIGNED_URL`
creates read-only signed URLs for the objects in the identified object table,
which are returned in a `signed_url` column. Generated signed URLs are
valid for 6 hours.

**Return Type**

TABLE

**Example**

Run the following query to return URIs and signed URLs for the objects in the
`mydataset.myobjecttable` object table.

    SELECT uri, signed_url
    FROM EXTERNAL_OBJECT_TRANSFORM(TABLE mydataset.myobjecttable, ['SIGNED_URL']);

    --The preceding statement returns results similar to the following:
    /*---+
     |  uri                                 | signed_url                                                                           |
     +---+
     | gs://myobjecttable/1234_Main_St.jpeg | https://storage.googleapis.com/mybucket/1234_Main_St.jpeg?X-Goog-Algorithm=1234abcd... |
     +---+
     | gs://myobjecttable/345_River_Rd.jpeg | https://storage.googleapis.com/mybucket/345_River_Rd.jpeg?X-Goog-Algorithm=2345bcde... |
     +---*/