# Supported protocol buffer and Arrow data types

This document describes the supported protocol buffer and Arrow data types
for each respective BigQuery data type. Before reading this document, read
[Overview of the BigQuery Storage Write API](https://docs.cloud.google.com/bigquery/docs/write-api#overview).

## Supported protocol buffer data types

The following table shows the supported data types in protocol buffers and the
corresponding input format in BigQuery:

| BigQuery data type | Supported protocol buffer types |
|---|---|
| `BOOL` | `bool`, `int32`, `int64`, `uint32`, `uint64`, `google.protobuf.BoolValue` |
| `BYTES` | `bytes`, `string`, `google.protobuf.BytesValue` |
| `DATE` | `int32` (preferred), `int64`, `string` The value is the number of days since the Unix epoch (1970-01-01). The valid range is `-719162` (0001-01-01) to `2932896` (9999-12-31). |
| `DATETIME`, `TIME` | `string` The value must be a [`DATETIME`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/lexical#datetime_literals) or [`TIME`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/lexical#time_literals) literal. |
| `DATETIME`, `TIME` | `int64` Use the [`CivilTimeEncoder` class](https://github.com/googleapis/java-bigquerystorage/blob/main/google-cloud-bigquerystorage/src/main/java/com/google/cloud/bigquery/storage/v1/CivilTimeEncoder.java) to perform the conversion. |
| `FLOAT` | `double`, `float`, `google.protobuf.DoubleValue`, `google.protobuf.FloatValue` |
| `GEOGRAPHY` | `string` The value is a geometry in either WKT or GeoJson format. |
| `INTEGER` | `int32`, `int64`, `uint32`, `enum`, `google.protobuf.Int32Value`, `google.protobuf.Int64Value`, `google.protobuf.UInt32Value` |
| `JSON` | `string` |
| `NUMERIC`, `BIGNUMERIC` | `int32`, `int64`, `uint32`, `uint64`, `double`, `float`, `string` |
| `NUMERIC`, `BIGNUMERIC` | `bytes`, `google.protobuf.BytesValue` Use the [`BigDecimalByteStringEncoder` class](https://github.com/googleapis/java-bigquerystorage/blob/main/google-cloud-bigquerystorage/src/main/java/com/google/cloud/bigquery/storage/v1/BigDecimalByteStringEncoder.java) to perform the conversion. |
| `STRING` | `string`, `enum`, `google.protobuf.StringValue` |
| `TIME` | `string` The value must be a [`TIME` literal](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/lexical#time_literals). |
| `TIMESTAMP` | `int64` (preferred), `int32`, `uint32`, `google.protobuf.Timestamp` The value is given in microseconds since the Unix epoch (1970-01-01). |
| `INTERVAL` | `string`, `google.protobuf.Duration` The string value must be an [`INTERVAL` literal](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/lexical#interval_literals). |
| `RANGE<T>` | `message` A nested message type in the proto with two fields, `start` and `end`, where both fields must be of the same supported protocol buffer type that corresponds to a BigQuery data type `T`. `T` must be one of `DATE`, `DATETIME`, or `TIMESTAMP`. If a field (`start` or `end`) is not set in the proto message, it represents an unbounded boundary. In the following example, `f_range_date` represents a `RANGE` column in a table. Since the `end` field is not set in the proto message, the end boundary of this range is unbounded. { f_range_date: { start: 1 } } |
| `REPEATED FIELD` | `array` An array type in the proto corresponds to a repeated field in BigQuery. |
| `RECORD` | `message` A nested message type in the proto corresponds to a record field in BigQuery. |

## Supported Apache Arrow data types

The following table shows the supported data types in Apache Arrow and
the corresponding input format in BigQuery.

| BigQuery data type | Supported Apache Arrow types | Supported type parameters |
|---|---|---|
| `BOOL` | `Boolean` |   |
| `BYTES` | `Binary` |   |
| `DATE` | `Date` | unit = Day |
| `DATE` | `String`, `int32` |   |
| `DATETIME` | `Timestamp` | unit = MICROSECONDS timezone is empty |
| `FLOAT` | `FloatingPoint` | Precision in {SINGLE, DOUBLE} |
| `GEOGRAPHY` | `Utf8` The value is a geometry in either WKT or GeoJson format. |   |
| `INTEGER` | `int` | bitWidth in {8, 16, 32, 64} is_signed = false |
| `JSON` | `Utf8` |   |
| `NUMERIC` | `Decimal128` | You can provide a NUMERIC that has any precision or scale that's smaller than the [BigQuery supported range](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#decimal_types). |
| `BIGNUMERIC` | `Decimal256` | You can provide a BIGNUMERIC that has any precision or scale that's smaller than the [BigQuery supported range](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#decimal_types). |
| `STRING` | `Utf8` |   |
| `TIMESTAMP` | `Timestamp` | unit= MICROSECONDS timezone = UTC |
| `INTERVAL` | `Interval` | unit in {YEAR_MONTH, DAY_TIME, MONTH_DAY_NANO} |
| `INTERVAL` | `Utf8` |   |
| `RANGE<T>` | `Struct` The Arrow Struct must have two subfields named `start` and `end`. For the `RANGE<DATE>` column, the fields must be Arrow type `Date` with `unit=Day`. For the `RANGE<DATETIME>` column, the fields must be the Arrow type `Timestamp` with `unit=MICROSECONDS`, without the timezone. For the `RANGE<TIMESTAMP>`, the fields must be the Arrow type `Timestamp` with `unit=MICROSECONDS`, `timezone=UTC`. A `NULL` value in any of the `start` and `end` fields will be treated as `UNBOUNDED`. |   |
| `REPEATED FIELD` | `List` | A `NULL` value must be represented by an empty list. |
| `RECORD` | `Struct` |   |