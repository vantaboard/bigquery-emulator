# MCP Tools Reference: bigquerydatatransfer.googleapis.com

## Tool: `list_transfer_runs`

List all the transfer runs for a transfer config.

The following example shows a MCP call to list all transfer runs for a transfer configuration named `transfer_config_id` in the project `myproject` in the location `myregion`.

If the location isn't explicitly specified, and it can't be determined from the resources in the request, then the [default location](https://docs.cloud.google.com/bigquery/docs/locations#default_location) is used. If the default location isn't set, then the job runs in the `US` multi-region.

`list_transfer_runs(project_id="myproject", location="myregion", transfer_config_id="mytransferconfig")`

The following sample demonstrate how to use `curl` to invoke the `list_transfer_runs` MCP tool.

| Curl Request |
|---|
| ```bash curl --location 'https://bigquerydatatransfer.googleapis.com/mcp' \ --header 'content-type: application/json' \ --header 'accept: application/json, text/event-stream' \ --data '{ "method": "tools/call", "params": { "name": "list_transfer_runs", "arguments": { // provide these details according to the tool's MCP specification } }, "jsonrpc": "2.0", "id": 1 }' ``` |

## Input Schema

A request to list data transfer runs.

### ListTransferRunsRequest

| JSON representation |
|---|
| ``` { "parent": string, "states": [ enum (`TransferState`) ], "pageToken": string, "pageSize": integer, "runAttempt": enum (`RunAttempt`) } ``` |

| Fields ||
|---|---|
| `parent` | `string` Required. Name of transfer configuration for which transfer runs should be retrieved. If you are using the regionless method, the location must be `US` and the name should be in the following form: - `projects/{project_id}/transferConfigs/{config_id}` If you are using the regionalized method, the name should be in the following form: - `projects/{project_id}/locations/{location_id}/transferConfigs/{config_id}` |
| `states[]` | ``enum (`TransferState`)`` When specified, only transfer runs with requested states are returned. |
| `pageToken` | `string` Pagination token, which can be used to request a specific page of `ListTransferRunsRequest` list results. For multiple-page results, `ListTransferRunsResponse` outputs a `next_page` token, which can be used as the `page_token` value to request the next page of list results. |
| `pageSize` | `integer` Page size. The default page size is the maximum value of 1000 results. |
| `runAttempt` | ``enum (`RunAttempt`)`` Indicates how run attempts are to be pulled. |

## Output Schema

The returned list of pipelines in the project.

### ListTransferRunsResponse

| JSON representation |
|---|
| ``` { "transferRuns": [ { object (`https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/mcp/tools_list/start_manual_transfer_runs#Output.Schema.TransferRun`) } ], "nextPageToken": string } ``` |

| Fields ||
|---|---|
| `transferRuns[]` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/mcp/tools_list/start_manual_transfer_runs#Output.Schema.TransferRun`)`` Output only. The stored pipeline transfer runs. |
| `nextPageToken` | `string` Output only. The next-pagination token. For multiple-page list results, this token can be used as the `ListTransferRunsRequest.page_token` to request the next page of list results. |

### TransferRun

| JSON representation |
|---|
| ``` { "name": string, "scheduleTime": string, "runTime": string, "errorStatus": { object (`https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/mcp/tools_list/create_transfer_config#Output.Schema.Status`) }, "startTime": string, "endTime": string, "updateTime": string, "params": { object }, "dataSourceId": string, "state": enum (`TransferState`), "userId": string, "schedule": string, "notificationPubsubTopic": string, "emailPreferences": { object (`https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/mcp/tools_list/create_transfer_config#Input.Schema.EmailPreferences`) }, // Union field `destination` can be only one of the following: "destinationDatasetId": string // End of list of possible types for union field `destination`. } ``` |

| Fields ||
|---|---|
| `name` | `string` Identifier. The resource name of the transfer run. Transfer run names have the form `projects/{project_id}/locations/{location}/transferConfigs/{config_id}/runs/{run_id}`. The name is ignored when creating a transfer run. |
| `scheduleTime` | ``string (`https://protobuf.dev/reference/protobuf/google.protobuf#timestamp` format)`` Minimum time after which a transfer run can be started. Uses RFC 3339, where generated output will always be Z-normalized and use 0, 3, 6 or 9 fractional digits. Offsets other than "Z" are also accepted. Examples: `"2014-10-02T15:01:23Z"`, `"2014-10-02T15:01:23.045123456Z"` or `"2014-10-02T15:01:23+05:30"`. |
| `runTime` | ``string (`https://protobuf.dev/reference/protobuf/google.protobuf#timestamp` format)`` For batch transfer runs, specifies the date and time of the data should be ingested. Uses RFC 3339, where generated output will always be Z-normalized and use 0, 3, 6 or 9 fractional digits. Offsets other than "Z" are also accepted. Examples: `"2014-10-02T15:01:23Z"`, `"2014-10-02T15:01:23.045123456Z"` or `"2014-10-02T15:01:23+05:30"`. |
| `errorStatus` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/mcp/tools_list/create_transfer_config#Output.Schema.Status`)`` Status of the transfer run. |
| `startTime` | ``string (`https://protobuf.dev/reference/protobuf/google.protobuf#timestamp` format)`` Output only. Time when transfer run was started. Parameter ignored by server for input requests. Uses RFC 3339, where generated output will always be Z-normalized and use 0, 3, 6 or 9 fractional digits. Offsets other than "Z" are also accepted. Examples: `"2014-10-02T15:01:23Z"`, `"2014-10-02T15:01:23.045123456Z"` or `"2014-10-02T15:01:23+05:30"`. |
| `endTime` | ``string (`https://protobuf.dev/reference/protobuf/google.protobuf#timestamp` format)`` Output only. Time when transfer run ended. Parameter ignored by server for input requests. Uses RFC 3339, where generated output will always be Z-normalized and use 0, 3, 6 or 9 fractional digits. Offsets other than "Z" are also accepted. Examples: `"2014-10-02T15:01:23Z"`, `"2014-10-02T15:01:23.045123456Z"` or `"2014-10-02T15:01:23+05:30"`. |
| `updateTime` | ``string (`https://protobuf.dev/reference/protobuf/google.protobuf#timestamp` format)`` Output only. Last time the data transfer run state was updated. Uses RFC 3339, where generated output will always be Z-normalized and use 0, 3, 6 or 9 fractional digits. Offsets other than "Z" are also accepted. Examples: `"2014-10-02T15:01:23Z"`, `"2014-10-02T15:01:23.045123456Z"` or `"2014-10-02T15:01:23+05:30"`. |
| `params` | ``object (`https://protobuf.dev/reference/protobuf/google.protobuf#struct` format)`` Output only. Parameters specific to each data source. For more information see the bq tab in the 'Setting up a data transfer' section for each data source. For example the parameters for Cloud Storage transfers are listed here: <https://cloud.google.com/bigquery-transfer/docs/cloud-storage-transfer#bq> |
| `dataSourceId` | `string` Output only. Data source id. |
| `state` | ``enum (`TransferState`)`` Data transfer run state. Ignored for input requests. |
| `userId` | `string (https://developers.google.com/discovery/v1/type-format format)` Deprecated. Unique ID of the user on whose behalf transfer is done. |
| `schedule` | `string` Output only. Describes the schedule of this transfer run if it was created as part of a regular schedule. For batch transfer runs that are scheduled manually, this is empty. NOTE: the system might choose to delay the schedule depending on the current load, so `schedule_time` doesn't always match this. |
| `notificationPubsubTopic` | `string` Output only. Pub/Sub topic where a notification will be sent after this transfer run finishes. The format for specifying a pubsub topic is: `projects/{project_id}/topics/{topic_id}` |
| `emailPreferences` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/mcp/tools_list/create_transfer_config#Input.Schema.EmailPreferences`)`` Output only. Email notifications will be sent according to these preferences to the email address of the user who owns the transfer config this run was derived from. |
| Union field `destination`. Data transfer destination. `destination` can be only one of the following: ||
| `destinationDatasetId` | `string` Output only. The BigQuery target dataset id. |

### Timestamp

| JSON representation |
|---|
| ``` { "seconds": string, "nanos": integer } ``` |

| Fields ||
|---|---|
| `seconds` | `string (https://developers.google.com/discovery/v1/type-format format)` Represents seconds of UTC time since Unix epoch 1970-01-01T00:00:00Z. Must be between -62135596800 and 253402300799 inclusive (which corresponds to 0001-01-01T00:00:00Z to 9999-12-31T23:59:59Z). |
| `nanos` | `integer` Non-negative fractions of a second at nanosecond resolution. This field is the nanosecond portion of the duration, not an alternative to seconds. Negative second values with fractions must still have non-negative nanos values that count forward in time. Must be between 0 and 999,999,999 inclusive. |

### Status

| JSON representation |
|---|
| ``` { "code": integer, "message": string, "details": [ { "@type": string, field1: ..., ... } ] } ``` |

| Fields ||
|---|---|
| `code` | `integer` The status code, which should be an enum value of `google.rpc.Code`. |
| `message` | `string` A developer-facing error message, which should be in English. Any user-facing error message should be localized and sent in the `google.rpc.Status.details` field, or localized by the client. |
| `details[]` | `object` A list of messages that carry the error details. There is a common set of message types for APIs to use. An object containing fields of an arbitrary type. An additional field `"@type"` contains a URI identifying the type. Example: `{ "id": 1234, "@type": "types.example.com/standard/id" }`. |

### Any

| JSON representation |
|---|
| ``` { "typeUrl": string, "value": string } ``` |

| Fields ||
|---|---|
| `typeUrl` | `string` Identifies the type of the serialized Protobuf message with a URI reference consisting of a prefix ending in a slash and the fully-qualified type name. Example: type.googleapis.com/google.protobuf.StringValue This string must contain at least one `/` character, and the content after the last `/` must be the fully-qualified name of the type in canonical form, without a leading dot. Do not write a scheme on these URI references so that clients do not attempt to contact them. The prefix is arbitrary and Protobuf implementations are expected to simply strip off everything up to and including the last `/` to identify the type. `type.googleapis.com/` is a common default prefix that some legacy implementations require. This prefix does not indicate the origin of the type, and URIs containing it are not expected to respond to any requests. All type URL strings must be legal URI references with the additional restriction (for the text format) that the content of the reference must consist only of alphanumeric characters, percent-encoded escapes, and characters in the following set (not including the outer backticks): `/-.~_!$&()*+,;=`. Despite our allowing percent encodings, implementations should not unescape them to prevent confusion with existing parsers. For example, `type.googleapis.com%2FFoo` should be rejected. In the original design of `Any`, the possibility of launching a type resolution service at these type URLs was considered but Protobuf never implemented one and considers contacting these URLs to be problematic and a potential security issue. Do not attempt to contact type URLs. |
| `value` | `string (https://developers.google.com/discovery/v1/type-format format)` Holds a Protobuf serialization of the type described by type_url. A base64-encoded string. |

### Struct

| JSON representation |
|---|
| ``` { "fields": { string: value, ... } } ``` |

| Fields ||
|---|---|
| `fields` | ``map (key: string, value: value (`https://protobuf.dev/reference/protobuf/google.protobuf#value` format))`` Unordered map of dynamically typed values. An object containing a list of `"key": value` pairs. Example: `{ "name": "wrench", "mass": "1.3kg", "count": "3" }`. |

### FieldsEntry

| JSON representation |
|---|
| ``` { "key": string, "value": value } ``` |

| Fields ||
|---|---|
| `key` | `string` |
| `value` | ``value (`https://protobuf.dev/reference/protobuf/google.protobuf#value` format)`` |

### Value

| JSON representation |
|---|
| ``` { // Union field `kind` can be only one of the following: "nullValue": null, "numberValue": number, "stringValue": string, "boolValue": boolean, "structValue": { object }, "listValue": array // End of list of possible types for union field `kind`. } ``` |

| Fields ||
|---|---|
| Union field `kind`. The kind of value. `kind` can be only one of the following: ||
| `nullValue` | `null` Represents a JSON `null`. |
| `numberValue` | `number` Represents a JSON number. Must not be `NaN`, `Infinity` or `-Infinity`, since those are not supported in JSON. This also cannot represent large Int64 values, since JSON format generally does not support them in its number type. |
| `stringValue` | `string` Represents a JSON string. |
| `boolValue` | `boolean` Represents a JSON boolean (`true` or `false` literal in JSON). |
| `structValue` | ``object (`https://protobuf.dev/reference/protobuf/google.protobuf#struct` format)`` Represents a JSON object. |
| `listValue` | ``array (`https://protobuf.dev/reference/protobuf/google.protobuf#list-value` format)`` Represents a JSON array. |

### ListValue

| JSON representation |
|---|
| ``` { "values": [ value ] } ``` |

| Fields ||
|---|---|
| `values[]` | ``value (`https://protobuf.dev/reference/protobuf/google.protobuf#value` format)`` Repeated field of dynamically typed values. |

### EmailPreferences

| JSON representation |
|---|
| ``` { "enableFailureEmail": boolean } ``` |

| Fields ||
|---|---|
| `enableFailureEmail` | `boolean` If true, email notifications will be sent on transfer run failures. |

### Tool Annotations

Destructive Hint: ❌ \| Idempotent Hint: ✅ \| Read Only Hint: ✅ \| Open World Hint: ❌