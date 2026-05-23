# StandardSqlDataType

- [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/StandardSqlDataType#SCHEMA_REPRESENTATION)
- [TypeKind](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/StandardSqlDataType#TypeKind)
- [StandardSqlStructType](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/StandardSqlDataType#StandardSqlStructType)
  - [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/StandardSqlDataType#StandardSqlStructType.SCHEMA_REPRESENTATION)

The data type of a variable such as a function argument. Examples include:

- INT64: `{"typeKind": "INT64"}`

- ARRAY:

    {
      "typeKind": "ARRAY",
      "arrayElementType": {"typeKind": "STRING"}
    }

- STRUCT\<x STRING, y ARRAY\>:

    {
      "typeKind": "STRUCT",
      "structType":
      {
        "fields":
        [
          {
            "name": "x",
            "type": {"typeKind": "STRING"}
          },
          {
            "name": "y",
            "type":
            {
              "typeKind": "ARRAY",
              "arrayElementType": {"typeKind": "DATE"}
            }
          }
        ]
      }
    }

- RANGE:

    {
      "typeKind": "RANGE",
      "rangeElementType": {"typeKind": "DATE"}
    }

| JSON representation |
|---|
| ``` { "typeKind": enum (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/StandardSqlDataType#TypeKind`), // Union field `sub_type` can be only one of the following: "arrayElementType": { object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/StandardSqlDataType`) }, "structType": { object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/StandardSqlDataType#StandardSqlStructType`) }, "rangeElementType": { object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/StandardSqlDataType`) } // End of list of possible types for union field `sub_type`. } ``` |

| Fields ||
|---|---|
| `typeKind` | ``enum (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/StandardSqlDataType#TypeKind`)`` Required. The top level type of this field. Can be any GoogleSQL data type (e.g., "INT64", "DATE", "ARRAY"). |
| Union field `sub_type`. For complex types, the sub type information. `sub_type` can be only one of the following: ||
| `arrayElementType` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/StandardSqlDataType`)`` The type of the array's elements, if typeKind = "ARRAY". |
| `structType` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/StandardSqlDataType#StandardSqlStructType`)`` The fields of this struct, in order, if typeKind = "STRUCT". |
| `rangeElementType` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/StandardSqlDataType`)`` The type of the range's elements, if typeKind = "RANGE". |

## TypeKind

The kind of the datatype.

| Enums ||
|---|---|
| `TYPE_KIND_UNSPECIFIED` | Invalid type. |
| `INT64` | Encoded as a string in decimal format. |
| `BOOL` | Encoded as a boolean "false" or "true". |
| `FLOAT64` | Encoded as a number, or string "NaN", "Infinity" or "-Infinity". |
| `STRING` | Encoded as a string value. |
| `BYTES` | Encoded as a base64 string per RFC 4648, section 4. |
| `TIMESTAMP` | Encoded as an RFC 3339 timestamp with mandatory "Z" time zone string: 1985-04-12T23:20:50.52Z |
| `DATE` | Encoded as RFC 3339 full-date format string: 1985-04-12 |
| `TIME` | Encoded as RFC 3339 partial-time format string: 23:20:50.52 |
| `DATETIME` | Encoded as RFC 3339 full-date "T" partial-time: 1985-04-12T23:20:50.52 |
| `GEOGRAPHY` | Encoded as WKT |
| `NUMERIC` | Encoded as a decimal string. |
| `BIGNUMERIC` | Encoded as a decimal string. |
| `JSON` | Encoded as a string. |
| `ARRAY` | Encoded as a list with types matching Type.array_type. |
| `STRUCT` | Encoded as a list with fields of type Type.struct_type\[i\]. tabledata.list is used because a JSON object cannot have duplicate field names. |
| `RANGE` | Encoded as a pair with types matching rangeElementType. Pairs must begin with "\[", end with ")", and be separated by ", ". |

## StandardSqlStructType

The representation of a SQL STRUCT type.

| JSON representation |
|---|
| ``` { "fields": [ { object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/StandardSqlField`) } ] } ``` |

| Fields ||
|---|---|
| `fields[]` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/StandardSqlField`)`` Fields within the struct. |