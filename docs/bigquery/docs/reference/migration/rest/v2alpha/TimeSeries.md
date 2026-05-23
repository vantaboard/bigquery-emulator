# TimeSeries

- [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/v2alpha/TimeSeries#SCHEMA_REPRESENTATION)
- [Point](https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/v2alpha/TimeSeries#Point)
  - [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/v2alpha/TimeSeries#Point.SCHEMA_REPRESENTATION)
- [TimeInterval](https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/v2alpha/TimeSeries#TimeInterval)
  - [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/v2alpha/TimeSeries#TimeInterval.SCHEMA_REPRESENTATION)
- [TypedValue](https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/v2alpha/TimeSeries#TypedValue)
  - [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/v2alpha/TimeSeries#TypedValue.SCHEMA_REPRESENTATION)

The metrics object for a SubTask.

| JSON representation |
|---|
| ``` { "metric": string, "valueType": enum (`https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/Shared.Types/ValueType`), "metricKind": enum (`https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/Shared.Types/MetricKind`), "points": [ { object (`https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/v2alpha/TimeSeries#Point`) } ] } ``` |

| Fields ||
|---|---|
| `metric` | `string` Required. The name of the metric. If the metric is not known by the service yet, it will be auto-created. |
| `valueType` | ``enum (`https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/Shared.Types/ValueType`)`` Required. The value type of the time series. |
| `metricKind` | ``enum (`https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/Shared.Types/MetricKind`)`` Optional. The metric kind of the time series. If present, it must be the same as the metric kind of the associated metric. If the associated metric's descriptor must be auto-created, then this field specifies the metric kind of the new descriptor and must be either `GAUGE` (the default) or `CUMULATIVE`. |
| `points[]` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/v2alpha/TimeSeries#Point`)`` Required. The data points of this time series. When listing time series, points are returned in reverse time order. When creating a time series, this field must contain exactly one point and the point's type must be the same as the value type of the associated metric. If the associated metric's descriptor must be auto-created, then the value type of the descriptor is determined by the point's type, which must be `BOOL`, `INT64`, `DOUBLE`, or `DISTRIBUTION`. |

## Point

A single data point in a time series.

| JSON representation |
|---|
| ``` { "interval": { object (`https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/v2alpha/TimeSeries#TimeInterval`) }, "value": { object (`https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/v2alpha/TimeSeries#TypedValue`) } } ``` |

| Fields ||
|---|---|
| `interval` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/v2alpha/TimeSeries#TimeInterval`)`` The time interval to which the data point applies. For `GAUGE` metrics, the start time does not need to be supplied, but if it is supplied, it must equal the end time. For `DELTA` metrics, the start and end time should specify a non-zero interval, with subsequent points specifying contiguous and non-overlapping intervals. For `CUMULATIVE` metrics, the start and end time should specify a non-zero interval, with subsequent points specifying the same start time and increasing end times, until an event resets the cumulative value to zero and sets a new start time for the following points. |
| `value` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/v2alpha/TimeSeries#TypedValue`)`` The value of the data point. |

## TimeInterval

A time interval extending just after a start time through an end time. If the start time is the same as the end time, then the interval represents a single point in time.

| JSON representation |
|---|
| ``` { "startTime": string, "endTime": string } ``` |

| Fields ||
|---|---|
| `startTime` | ``string (`https://protobuf.dev/reference/protobuf/google.protobuf#timestamp` format)`` Optional. The beginning of the time interval. The default value for the start time is the end time. The start time must not be later than the end time. |
| `endTime` | ``string (`https://protobuf.dev/reference/protobuf/google.protobuf#timestamp` format)`` Required. The end of the time interval. |

## TypedValue

A single strongly-typed value.

| JSON representation |
|---|
| ``` { // Union field `value` can be only one of the following: "boolValue": boolean, "int64Value": string, "doubleValue": number, "stringValue": string, "distributionValue": { object (`https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/Shared.Types/Distribution`) } // End of list of possible types for union field `value`. } ``` |

| Fields ||
|---|---|
| Union field `value`. The typed value field. `value` can be only one of the following: ||
| `boolValue` | `boolean` A Boolean value: `true` or `false`. |
| `int64Value` | `string (https://developers.google.com/discovery/v1/type-format format)` A 64-bit integer. Its range is approximately `+/-9.2x10^18`. |
| `doubleValue` | `number` A 64-bit double-precision floating-point number. Its magnitude is approximately `+/-10^(+/-300)` and it has 16 significant digits of precision. |
| `stringValue` | `string` A variable-length string value. |
| `distributionValue` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/Shared.Types/Distribution`)`` A distribution value. |