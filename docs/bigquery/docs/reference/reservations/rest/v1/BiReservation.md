# BiReservation

- [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/reservations/rest/v1/BiReservation#SCHEMA_REPRESENTATION)
- [TableReference](https://docs.cloud.google.com/bigquery/docs/reference/reservations/rest/v1/BiReservation#TableReference)
  - [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/reservations/rest/v1/BiReservation#TableReference.SCHEMA_REPRESENTATION)

Represents a BI Reservation.

| JSON representation |
|---|
| ``` { "name": string, "updateTime": string, "size": string, "preferredTables": [ { object (`https://docs.cloud.google.com/bigquery/docs/reference/reservations/rest/v1/BiReservation#TableReference`) } ] } ``` |

| Fields ||
|---|---|
| `name` | `string` Identifier. The resource name of the singleton BI reservation. Reservation names have the form `projects/{projectId}/locations/{locationId}/biReservation`. |
| `updateTime` | ``string (`https://protobuf.dev/reference/protobuf/google.protobuf#timestamp` format)`` Output only. The last update timestamp of a reservation. Uses RFC 3339, where generated output will always be Z-normalized and use 0, 3, 6 or 9 fractional digits. Offsets other than "Z" are also accepted. Examples: `"2014-10-02T15:01:23Z"`, `"2014-10-02T15:01:23.045123456Z"` or `"2014-10-02T15:01:23+05:30"`. |
| `size` | `string (https://developers.google.com/discovery/v1/type-format format)` Optional. Size of a reservation, in bytes. |
| `preferredTables[]` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/reservations/rest/v1/BiReservation#TableReference`)`` Optional. Preferred tables to use BI capacity for. |

## TableReference

Fully qualified reference to BigQuery table. Internally stored as google.cloud.bi.v1.BqTableReference.

| JSON representation |
|---|
| ``` { "projectId": string, "datasetId": string, "tableId": string } ``` |

| Fields ||
|---|---|
| `projectId` | `string` Optional. The assigned project ID of the project. |
| `datasetId` | `string` Optional. The ID of the dataset in the above project. |
| `tableId` | `string` Optional. The ID of the table in the above dataset. |