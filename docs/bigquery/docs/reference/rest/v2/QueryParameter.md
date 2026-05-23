# QueryParameter

- [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/QueryParameter#SCHEMA_REPRESENTATION)
- [QueryParameterType](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/QueryParameter#QueryParameterType)
  - [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/QueryParameter#QueryParameterType.SCHEMA_REPRESENTATION)
- [QueryParameterValue](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/QueryParameter#QueryParameterValue)
  - [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/QueryParameter#QueryParameterValue.SCHEMA_REPRESENTATION)
- [RangeValue](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/QueryParameter#RangeValue)
  - [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/QueryParameter#RangeValue.SCHEMA_REPRESENTATION)

A parameter given to a query.

| JSON representation |
|---|
| ``` { "name": string, "parameterType": { object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/QueryParameter#QueryParameterType`) }, "parameterValue": { object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/QueryParameter#QueryParameterValue`) } } ``` |

| Fields ||
|---|---|
| `name` | `string` Optional. If unset, this is a positional parameter. Otherwise, should be unique within a query. |
| `parameterType` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/QueryParameter#QueryParameterType`)`` Required. The type of this parameter. |
| `parameterValue` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/QueryParameter#QueryParameterValue`)`` Required. The value of this parameter. |

## QueryParameterType

The type of a query parameter.

| JSON representation |
|---|
| ``` { "type": string, "arrayType": { object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/QueryParameter#QueryParameterType`) }, "structTypes": [ { "name": string, "type": { object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/QueryParameter#QueryParameterType`) }, "description": string } ], "rangeElementType": { object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/QueryParameter#QueryParameterType`) } } ``` |

| Fields ||
|---|---|
| `type` | `string` Required. The top level type of this field. |
| `arrayType` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/QueryParameter#QueryParameterType`)`` Optional. The type of the array's elements, if this is an array. |
| `structTypes[]` | `object` Optional. The types of the fields of this struct, in order, if this is a struct. |
| `structTypes[].name` | `string` Optional. The name of this field. |
| `structTypes[].type` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/QueryParameter#QueryParameterType`)`` Required. The type of this field. |
| `structTypes[].description` | `string` Optional. Human-oriented description of the field. |
| `rangeElementType` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/QueryParameter#QueryParameterType`)`` Optional. The element type of the range, if this is a range. |

## QueryParameterValue

The value of a query parameter.

| JSON representation |
|---|
| ``` { "value": string, "arrayValues": [ { object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/QueryParameter#QueryParameterValue`) } ], "structValues": { string: { object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/QueryParameter#QueryParameterValue`) }, ... }, "rangeValue": { object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/QueryParameter#RangeValue`) } } ``` |

| Fields ||
|---|---|
| `value` | `string` Optional. The value of this value, if a simple scalar type. |
| `arrayValues[]` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/QueryParameter#QueryParameterValue`)`` Optional. The array values, if this is an array type. |
| `structValues` | ``map (key: string, value: object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/QueryParameter#QueryParameterValue`))`` The struct field values. |
| `rangeValue` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/QueryParameter#RangeValue`)`` Optional. The range value, if this is a range type. |

## RangeValue

Represents the value of a range.

| JSON representation |
|---|
| ``` { "start": { object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/QueryParameter#QueryParameterValue`) }, "end": { object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/QueryParameter#QueryParameterValue`) } } ``` |

| Fields ||
|---|---|
| `start` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/QueryParameter#QueryParameterValue`)`` Optional. The start value of the range. A missing value represents an unbounded start. |
| `end` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/QueryParameter#QueryParameterValue`)`` Optional. The end value of the range. A missing value represents an unbounded end. |