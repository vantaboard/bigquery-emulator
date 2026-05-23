# Use the BigQuery DataFrames data type system

The BigQuery DataFrames data type system is built upon BigQuery data
types. This design ensures seamless integration and alignment with the
Google Cloud data warehouse, reflecting the built-in types used for data
storage in BigQuery.

## Type mappings

The following table shows data type equivalents in BigQuery,
BigQuery DataFrames, and other Python libraries as well as their levels
of support:

| Data type | BigQuery | BigQuery DataFrames | Python built-in | PyArrow |
|---|---|---|---|---|
| Boolean | `BOOL` | `pandas.BooleanDtype()` | `bool` | `bool_()` |
| Integer | `INT64` | `pandas.Int64Dtype()` | `int` | `int64()` |
| Float | `FLOAT64` | `pandas.Float64Dtype()` | `float` | `float64()` |
| String | `STRING` | `pandas.StringDtype(storage="pyarrow")` | `str` | `string()` |
| Bytes | `BYTES` | `pandas.ArrowDtype(pyarrow.binary())` | `bytes` | `binary()` |
| Date | `DATE` | `pandas.ArrowDtype(pyarrow.date32())` | `datetime.date` | `date32()` |
| Time | `TIME` | `pandas.ArrowDtype(pyarrow.time64("us"))` | `datetime.time` | `time64("us")` |
| Datetime | `DATETIME` | `pandas.ArrowDtype(pyarrow.timestamp("us"))` | `datetime.datetime` | `timestamp("us")` |
| Timestamp | `TIMESTAMP` | `pandas.ArrowDtype(pyarrow.timestamp("us", tz="UTC"))` | `Datetime.datetime` with timezone | `timestamp("us", tz="UTC")` |
| Numeric | `NUMERIC` | `pandas.ArrowDtype(pyarrow.decimal128(38, 9))` | `decimal.Decimal` | `decimal128(38, 9)` |
| Big numeric | `BIGNUMERIC` | `pandas.ArrowDtype(pyarrow.decimal256(76, 38))` | `decimal.Decimal` | `decimal256(76, 38)` |
| List\<T\> | `ARRAY``<T>` | `pandas.ArrowDtype(pyarrow.list_(T))` | `list[T]` | `list_(T)` |
| Struct | `STRUCT` | `pandas.ArrowDtype(pyarrow.struct())` | `dict` | `struct()` |
| JSON | `JSON` | `pandas.ArrowDtype(pyarrow.json_(pa.string())` in pandas version 3.0 or later and PyArrow version 19.0 or later; otherwise, JSON columns are exposed as `pandas.ArrowDtype(db_dtypes.JSONArrowType())`. This feature is in [Preview](https://cloud.google.com/products#product-launch-stages). | Not supported | `json_()` ([Preview](https://cloud.google.com/products#product-launch-stages)) |
| Geography | `GEOGRAPHY` | `Geopandas.array.GeometryDtype()` Supported by `to_pandas()` only. | Not supported | Not supported |
| Timedelta | Not supported | `pandas.ArrowDtype(pyarrow.duration("us"))` | `datetime.timedelta` | `duration("us")` |

> [!NOTE]
> **Note:** BigQuery DataFrames doesn't support the following BigQuery data types: `INTERVAL` and `RANGE`. All other BigQuery data types display as the object type.

### Type conversions

When used with local data, BigQuery DataFrames converts data types to
their corresponding BigQuery DataFrames equivalents wherever a
[type mapping is defined](https://docs.cloud.google.com/bigquery/docs/dataframes-data-types#type-mappings), as shown in the following example:

    import pandas as pd

    import bigframes.pandas as bpd

    s = pd.Series([pd.Timestamp("20250101")])
    assert s.dtype == "datetime64[ns]"
    assert bpd.read_pandas(s).dtype == "timestamp[us][pyarrow]"

PyArrow dictates behavior when there are discrepancies between the data type
equivalents. In rare cases when the Python built-in type functions differently
from its PyArrow counterpart, BigQuery DataFrames generally favors the
PyArrow behavior to ensure consistency.

The following code sample uses the `datetime.date + timedelta` operation to
show that, unlike the Python datetime library that still returns a date
instance, BigQuery DataFrames follows the PyArrow behavior by returning
a timestamp instance:

    import datetime

    import pandas as pd

    import bigframes.pandas as bpd

    s = pd.Series([datetime.date(2025, 1, 1)])
    s + pd.Timedelta(hours=12)
    # 0	2025-01-01
    # dtype: object

    bpd.read_pandas(s) + pd.Timedelta(hours=12)
    # 0    2025-01-01 12:00:00
    # dtype: timestamp[us][pyarrow]

## Special types

The following sections describe the special data types that
BigQuery DataFrames uses.

### JSON

Within BigQuery DataFrames, columns using the BigQuery
[JSON format](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#json_type)
(a lightweight standard) are represented by `pandas.ArrowDtype`. The exact
underlying Arrow type depends on your library versions. Older environments
typically use `db_dtypes.JSONArrowType()` for compatibility, which is an Arrow
extension type that acts as a light wrapper around `pa.string()`. In contrast,
newer setups (pandas 3.0 and later and PyArrow 19.0 and later) use the more
recent `pa.json_(pa.string())` representation.

### `timedelta`

The `timedelta` type lacks a direct equivalent within the
BigQuery built-in type system. To manage duration data,
BigQuery DataFrames utilizes the `INT64` type as the underlying storage
format in BigQuery tables. You can expect the results of your
computations to be consistent with the behavior you would expect from
equivalent operations performed with the pandas library.

You can directly load `timedelta` values into BigQuery DataFrames and
`Series` objects, as shown in the following example:

    import pandas as pd

    import bigframes.pandas as bpd

    s = pd.Series([pd.Timedelta("1s"), pd.Timedelta("2m")])
    bpd.read_pandas(s)
    # 0    0 days 00:00:01
    # 1    0 days 00:02:00
    # dtype: duration[us][pyarrow]

Unlike pandas, BigQuery DataFrames only supports `timedelta` values with
microsecond precision. If your data includes nanoseconds, you must round them to
avoid potential exceptions, as shown in the following example:

    import pandas as pd

    s = pd.Series([pd.Timedelta("999ns")])
    bpd.read_pandas(s.dt.round("us"))
    # 0    0 days 00:00:00.000001
    # dtype: duration[us][pyarrow]

You can use the `bigframes.pandas.to_timedelta` function to cast a
BigQuery DataFrames `Series` object to the `timedelta` type, as shown
in the following example:

    import bigframes.pandas as bpd

    bpd.to_timedelta([1, 2, 3], unit="s")
    # 0    0 days 00:00:01
    # 1    0 days 00:00:02
    # 2    0 days 00:00:03
    # dtype: duration[us][pyarrow]

When you load data containing `timedelta` values to a BigQuery table, the
values are converted to microseconds and stored in `INT64` columns. To
preserve the type information, BigQuery DataFrames appends the
`#microseconds` string to the descriptions of these columns. Some operations,
such as SQL query executions and UDF invocations, don't preserve column
descriptions, and the `timedelta` type information is lost after these
operations are completed.

## Tools for composite types

For certain composite types, BigQuery DataFrames provides tools that
let you access and process the elemental values within those types.

### List accessor

The `ListAccessor` object can help you perform operations on each list element
by using the list property of the `Series` object, as shown in the
following example:

    import bigframes.pandas as bpd

    s = bpd.Series([[1, 2, 3], [4, 5], [6]])  # dtype: list<item: int64>[pyarrow]

    # Access the first elements of each list
    s.list[0]
    # 0    1
    # 1    4
    # 2    6
    # dtype: Int64

    # Get the lengths of each list
    s.list.len()
    # 0    3
    # 1    2
    # 2    1
    # dtype: Int64

### Struct accessor

The `StructAccessor` object can access and process fields in a series of
structs. The API accessor object is `series.struct`, as shown in the
following example:

    import bigframes.pandas as bpd

    structs = [
        {"id": 101, "category": "A"},
        {"id": 102, "category": "B"},
        {"id": 103, "category": "C"},
    ]
    s = bpd.Series(structs)
    # Get the 'id' field of each struct
    s.struct.field("id")
    # 0    101
    # 1    102
    # 2    103
    # Name: id, dtype: Int64

If the `struct` field you plan to access is unambiguous from other `Series`
properties, you can skip calling `struct`, as shown in the following example:

    import bigframes.pandas as bpd

    structs = [
        {"id": 101, "category": "A"},
        {"id": 102, "category": "B"},
        {"id": 103, "category": "C"},
    ]
    s = bpd.Series(structs)

    # not explicitly using the "struct" property
    s.id
    # 0    101
    # 1    102
    # 2    103
    # Name: id, dtype: Int64

However, it's a best practice to use `struct` for accessing fields, because
it makes your code easier to understand and less error-prone.

### String accessor

You can access the `StringAccessor` object with the `str` property on a `Series`
object, as shown in the following example:

    import bigframes.pandas as bpd

    s = bpd.Series(["abc", "de", "1"])  # dtype: string[pyarrow]

    # Get the first character of each string
    s.str[0]
    # 0    a
    # 1    d
    # 2    1
    # dtype: string

    # Check whether there are only alphabetic characters in each string
    s.str.isalpha()
    # 0     True
    # 1     True
    # 2     False
    # dtype: boolean

    # Cast the alphabetic characters to their upper cases for each string
    s.str.upper()
    # 0    ABC
    # 1     DE
    # 2      1
    # dtype: string

### Geography accessor

BigQuery DataFrames provides a `GeographyAccessor` object that shares
similar APIs with the GeoSeries structure provided by the GeoPandas library. You
can invoke the `GeographyAccessor` object with the `geo` property on a `Series`
object, as shown in the following example:

    from shapely.geometry import Point

    import bigframes.pandas as bpd

    s = bpd.Series([Point(1, 0), Point(2, 1)])  # dtype: geometry

    s.geo.y
    # 0    0.0
    # 1    1.0
    # dtype: Float64

## What's next

- Learn about [BigQuery DataFrames](https://docs.cloud.google.com/bigquery/docs/bigquery-dataframes-introduction).
- Learn about [BigQuery DataFrames sessions and I/O](https://docs.cloud.google.com/bigquery/docs/dataframes-sessions-io).
- Learn how to [visualize graphs using BigQuery DataFrames](https://docs.cloud.google.com/bigquery/docs/dataframes-visualizations).
- Explore the [BigQuery DataFrames API reference](https://dataframes.bigquery.dev/reference/index.html).