# MetricKind

The kind of measurement. It describes how the data is reported. For information on setting the start time and end time based on the MetricKind, see \[TimeInterval\]\[google.monitoring.v3.TimeInterval\].

| Enums ||
|---|---|
| `METRIC_KIND_UNSPECIFIED` | Do not use this default value. |
| `GAUGE` | An instantaneous measurement of a value. |
| `DELTA` | The change in a value during a time interval. |
| `CUMULATIVE` | A value accumulated over a time interval. Cumulative measurements in a time series should have the same start time and increasing end times, until an event resets the cumulative value to zero and sets a new start time for the following points. |