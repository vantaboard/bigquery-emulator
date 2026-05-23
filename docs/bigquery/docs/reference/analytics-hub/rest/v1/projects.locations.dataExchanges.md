# REST Resource: projects.locations.dataExchanges.queryTemplates

- [Resource: QueryTemplate](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.queryTemplates#QueryTemplate)
  - [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.queryTemplates#QueryTemplate.SCHEMA_REPRESENTATION)
- [State](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.queryTemplates#State)
- [Routine](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.queryTemplates#Routine)
  - [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.queryTemplates#Routine.SCHEMA_REPRESENTATION)
- [RoutineType](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.queryTemplates#RoutineType)
- [Methods](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.queryTemplates#METHODS_SUMMARY)

## Resource: QueryTemplate

A query template is a container for sharing table-valued functions defined by contributors in a data clean room.

| JSON representation |
|---|
| ``` { "name": string, "displayName": string, "description": string, "proposer": string, "primaryContact": string, "documentation": string, "state": enum (`https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.queryTemplates#State`), "routine": { object (`https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.queryTemplates#Routine`) }, "createTime": string, "updateTime": string } ``` |

| Fields ||
|---|---|
| `name` | `string` Output only. The resource name of the QueryTemplate. e.g. `projects/myproject/locations/us/dataExchanges/123/queryTemplates/456` |
| `displayName` | `string` Required. Human-readable display name of the QueryTemplate. The display name must contain only Unicode letters, numbers (0-9), underscores (_), dashes (-), spaces ( ), ampersands (\&) and can't start or end with spaces. Default value is an empty string. Max length: 63 bytes. |
| `description` | `string` Optional. Short description of the QueryTemplate. The description must not contain Unicode non-characters and C0 and C1 control codes except tabs (HT), new lines (LF), carriage returns (CR), and page breaks (FF). Default value is an empty string. Max length: 2000 bytes. |
| `proposer` | `string` Optional. Will be deprecated. Email or URL of the primary point of contact of the QueryTemplate. Max Length: 1000 bytes. |
| `primaryContact` | `string` Optional. Email or URL of the primary point of contact of the QueryTemplate. Max Length: 1000 bytes. |
| `documentation` | `string` Optional. Documentation describing the QueryTemplate. |
| `state` | ``enum (`https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.queryTemplates#State`)`` Output only. The QueryTemplate lifecycle state. |
| `routine` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.queryTemplates#Routine`)`` Optional. The routine associated with the QueryTemplate. |
| `createTime` | ``string (`https://protobuf.dev/reference/protobuf/google.protobuf#timestamp` format)`` Output only. Timestamp when the QueryTemplate was created. Uses RFC 3339, where generated output will always be Z-normalized and use 0, 3, 6 or 9 fractional digits. Offsets other than "Z" are also accepted. Examples: `"2014-10-02T15:01:23Z"`, `"2014-10-02T15:01:23.045123456Z"` or `"2014-10-02T15:01:23+05:30"`. |
| `updateTime` | ``string (`https://protobuf.dev/reference/protobuf/google.protobuf#timestamp` format)`` Output only. Timestamp when the QueryTemplate was last modified. Uses RFC 3339, where generated output will always be Z-normalized and use 0, 3, 6 or 9 fractional digits. Offsets other than "Z" are also accepted. Examples: `"2014-10-02T15:01:23Z"`, `"2014-10-02T15:01:23.045123456Z"` or `"2014-10-02T15:01:23+05:30"`. |

## State

The QueryTemplate lifecycle state.

| Enums ||
|---|---|
| `STATE_UNSPECIFIED` | Default value. This value is unused. |
| `DRAFTED` | The QueryTemplate is in draft state. |
| `PENDING` | The QueryTemplate is in pending state. |
| `DELETED` | The QueryTemplate is in deleted state. |
| `APPROVED` | The QueryTemplate is in approved state. |

## Routine

Represents a bigquery routine.

| JSON representation |
|---|
| ``` { "routineType": enum (`https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.queryTemplates#RoutineType`), "definitionBody": string } ``` |

| Fields ||
|---|---|
| `routineType` | ``enum (`https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.queryTemplates#RoutineType`)`` Required. The type of routine. |
| `definitionBody` | `string` Optional. The definition body of the routine. |

## RoutineType

Represents the type of a given routine.

| Enums ||
|---|---|
| `ROUTINE_TYPE_UNSPECIFIED` | Default value. |
| `TABLE_VALUED_FUNCTION` | Non-built-in persistent TVF. |

| ## Methods ||
|---|---|
| ### `https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.queryTemplates/approve` | Approves a query template. |
| ### `https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.queryTemplates/create` | Creates a new QueryTemplate |
| ### `https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.queryTemplates/delete` | Deletes a query template. |
| ### `https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.queryTemplates/get` | Gets a QueryTemplate |
| ### `https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.queryTemplates/list` | Lists all QueryTemplates in a given project and location. |
| ### `https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.queryTemplates/patch` | Updates an existing QueryTemplate |
| ### `https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.queryTemplates/submit` | Submits a query template for approval. |