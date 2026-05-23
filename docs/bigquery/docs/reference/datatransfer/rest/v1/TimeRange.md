# TimeRange

- [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/TimeRange#SCHEMA_REPRESENTATION)

A specification for a time range, this will request transfer runs with runTime between startTime (inclusive) and endTime (exclusive).

| JSON representation |
|---|
| ``` { "startTime": string, "endTime": string } ``` |

| Fields ||
|---|---|
| `startTime` | ``string (`https://protobuf.dev/reference/protobuf/google.protobuf#timestamp` format)`` Start time of the range of transfer runs. For example, `"2017-05-25T00:00:00+00:00"`. The startTime must be strictly less than the endTime. Creates transfer runs where runTime is in the range between startTime (inclusive) and endTime (exclusive). |
| `endTime` | ``string (`https://protobuf.dev/reference/protobuf/google.protobuf#timestamp` format)`` End time of the range of transfer runs. For example, `"2017-05-30T00:00:00+00:00"`. The endTime must not be in the future. Creates transfer runs where runTime is in the range between startTime (inclusive) and endTime (exclusive). |