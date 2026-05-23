# ValueType

The value type of a metric.

| Enums ||
|---|---|
| `VALUE_TYPE_UNSPECIFIED` | Do not use this default value. |
| `BOOL` | The value is a boolean. This value type can be used only if the metric kind is `GAUGE`. |
| `INT64` | The value is a signed 64-bit integer. |
| `DOUBLE` | The value is a double precision floating point number. |
| `STRING` | The value is a text string. This value type can be used only if the metric kind is `GAUGE`. |
| `DISTRIBUTION` | The value is a `https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/Shared.Types/Distribution`. |
| `MONEY` | The value is money. |