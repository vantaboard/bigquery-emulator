# MCP Tools Reference: bigquerydatatransfer.googleapis.com

## Tool: `list_data_sources`

List all the data sources that the project has access to.

The following example shows a MCP call to list all data sources in the project `myproject` in the location `myregion`.

If the location isn't explicitly specified, and it can't be determined from the resources in the request, then the [default location](https://docs.cloud.google.com/bigquery/docs/locations#default_location) is used. If the default location isn't set, then the job runs in the `US` multi-region.

`list_data_sources(project_id="myproject", location="myregion")`

The following sample demonstrate how to use `curl` to invoke the `list_data_sources` MCP tool.

| Curl Request |
|---|
| ```bash curl --location 'https://bigquerydatatransfer.googleapis.com/mcp' \ --header 'content-type: application/json' \ --header 'accept: application/json, text/event-stream' \ --data '{ "method": "tools/call", "params": { "name": "list_data_sources", "arguments": { // provide these details according to the tool's MCP specification } }, "jsonrpc": "2.0", "id": 1 }' ``` |

## Input Schema

Request for listing data sources.

### ListDataSourcesRequest

| JSON representation |
|---|
| ``` { "projectId": string } ``` |

| Fields ||
|---|---|
| `projectId` | `string` Required. Project ID or project number. |

## Output Schema

Response for listing data sources.

### ListDataSourcesResponse

| JSON representation |
|---|
| ``` { "dataSources": [ { object (`https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/mcp/tools_list/list_data_sources#Output.Schema.DataSource`) } ] } ``` |

| Fields ||
|---|---|
| `dataSources[]` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/mcp/tools_list/list_data_sources#Output.Schema.DataSource`)`` Data sources. |

### DataSource

| JSON representation |
|---|
| ``` { "name": string, "dataSourceId": string, "displayName": string, "description": string, "clientId": string, "scopes": [ string ], "transferType": enum (`TransferType`), "supportsMultipleTransfers": boolean, "updateDeadlineSeconds": integer, "defaultSchedule": string, "supportsCustomSchedule": boolean, "parameters": [ { object (`https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/mcp/tools_list/list_data_sources#Output.Schema.DataSourceParameter`) } ], "helpUrl": string, "authorizationType": enum (`AuthorizationType`), "dataRefreshType": enum (`DataRefreshType`), "defaultDataRefreshWindowDays": integer, "manualRunsDisabled": boolean, "minimumScheduleInterval": string } ``` |

| Fields ||
|---|---|
| `name` | `string` Output only. Data source resource name. |
| `dataSourceId` | `string` Data source id. |
| `displayName` | `string` User friendly data source name. |
| `description` | `string` User friendly data source description string. |
| `clientId` | `string` Data source client id which should be used to receive refresh token. |
| `scopes[]` | `string` Api auth scopes for which refresh token needs to be obtained. These are scopes needed by a data source to prepare data and ingest them into BigQuery, e.g., <https://www.googleapis.com/auth/bigquery> |
| `transferType (deprecated)` | ``enum (`TransferType`)`` > [!WARNING] > This item is deprecated! Deprecated. This field has no effect. |
| `supportsMultipleTransfers (deprecated)` | `boolean` > [!WARNING] > This item is deprecated! Deprecated. This field has no effect. |
| `updateDeadlineSeconds` | `integer` The number of seconds to wait for an update from the data source before the Data Transfer Service marks the transfer as FAILED. |
| `defaultSchedule` | `string` Default data transfer schedule. Examples of valid schedules include: `1st,3rd monday of month 15:30`, `every wed,fri of jan,jun 13:15`, and `first sunday of quarter 00:00`. |
| `supportsCustomSchedule` | `boolean` Specifies whether the data source supports a user defined schedule, or operates on the default schedule. When set to `true`, user can override default schedule. |
| `parameters[]` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/mcp/tools_list/list_data_sources#Output.Schema.DataSourceParameter`)`` Data source parameters. |
| `helpUrl` | `string` Url for the help document for this data source. |
| `authorizationType` | ``enum (`AuthorizationType`)`` Indicates the type of authorization. |
| `dataRefreshType` | ``enum (`DataRefreshType`)`` Specifies whether the data source supports automatic data refresh for the past few days, and how it's supported. For some data sources, data might not be complete until a few days later, so it's useful to refresh data automatically. |
| `defaultDataRefreshWindowDays` | `integer` Default data refresh window on days. Only meaningful when `data_refresh_type` = `SLIDING_WINDOW`. |
| `manualRunsDisabled` | `boolean` Disables backfilling and manual run scheduling for the data source. |
| `minimumScheduleInterval` | ``string (`https://protobuf.dev/reference/protobuf/google.protobuf#duration` format)`` The minimum interval for scheduler to schedule runs. A duration in seconds with up to nine fractional digits, ending with '`s`'. Example: `"3.5s"`. |

### DataSourceParameter

| JSON representation |
|---|
| ``` { "paramId": string, "displayName": string, "description": string, "type": enum (`Type`), "required": boolean, "repeated": boolean, "validationRegex": string, "allowedValues": [ string ], "minValue": number, "maxValue": number, "fields": [ { object (`https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/mcp/tools_list/list_data_sources#Output.Schema.DataSourceParameter`) } ], "validationDescription": string, "validationHelpUrl": string, "immutable": boolean, "recurse": boolean, "deprecated": boolean, // Union field `_max_list_size` can be only one of the following: "maxListSize": string // End of list of possible types for union field `_max_list_size`. } ``` |

| Fields ||
|---|---|
| `paramId` | `string` Parameter identifier. |
| `displayName` | `string` Parameter display name in the user interface. |
| `description` | `string` Parameter description. |
| `type` | ``enum (`Type`)`` Parameter type. |
| `required` | `boolean` Is parameter required. |
| `repeated` | `boolean` Deprecated. This field has no effect. |
| `validationRegex` | `string` Regular expression which can be used for parameter validation. |
| `allowedValues[]` | `string` All possible values for the parameter. |
| `minValue` | `number` For integer and double values specifies minimum allowed value. |
| `maxValue` | `number` For integer and double values specifies maximum allowed value. |
| `fields[]` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/mcp/tools_list/list_data_sources#Output.Schema.DataSourceParameter`)`` Deprecated. This field has no effect. |
| `validationDescription` | `string` Description of the requirements for this field, in case the user input does not fulfill the regex pattern or min/max values. |
| `validationHelpUrl` | `string` URL to a help document to further explain the naming requirements. |
| `immutable` | `boolean` Cannot be changed after initial creation. |
| `recurse` | `boolean` Deprecated. This field has no effect. |
| `deprecated` | `boolean` If true, it should not be used in new transfers, and it should not be visible to users. |
| Union field `_max_list_size`. `_max_list_size` can be only one of the following: ||
| `maxListSize` | `string (https://developers.google.com/discovery/v1/type-format format)` For list parameters, the max size of the list. |

### DoubleValue

| JSON representation |
|---|
| ``` { "value": number } ``` |

| Fields ||
|---|---|
| `value` | `number` The double value. |

### Duration

| JSON representation |
|---|
| ``` { "seconds": string, "nanos": integer } ``` |

| Fields ||
|---|---|
| `seconds` | `string (https://developers.google.com/discovery/v1/type-format format)` Signed seconds of the span of time. Must be from -315,576,000,000 to +315,576,000,000 inclusive. Note: these bounds are computed from: 60 sec/min \* 60 min/hr \* 24 hr/day \* 365.25 days/year \* 10000 years |
| `nanos` | `integer` Signed fractions of a second at nanosecond resolution of the span of time. Durations less than one second are represented with a 0 `seconds` field and a positive or negative `nanos` field. For durations of one second or more, a non-zero value for the `nanos` field must be of the same sign as the `seconds` field. Must be from -999,999,999 to +999,999,999 inclusive. |

### Tool Annotations

Destructive Hint: ❌ \| Idempotent Hint: ✅ \| Read Only Hint: ✅ \| Open World Hint: ❌