# MCP Tools Reference: bigquerymigration.googleapis.com

## Tool: `get_translation`

Gets the SQL translation for a given translation ID.

The following sample demonstrate how to use `curl` to invoke the `get_translation` MCP tool.

| Curl Request |
|---|
| ```bash curl --location 'https://bigquerymigration.googleapis.com/mcp' \ --header 'content-type: application/json' \ --header 'accept: application/json, text/event-stream' \ --data '{ "method": "tools/call", "params": { "name": "get_translation", "arguments": { // provide these details according to the tool's MCP specification } }, "jsonrpc": "2.0", "id": 1 }' ``` |

## Input Schema

Request message for `FetchTranslation`.

### FetchTranslationRequest

| JSON representation |
|---|
| ``` { "name": string } ``` |

| Fields ||
|---|---|
| `name` | `string` Required. The name of the translation to retrieve. Format: `projects/PROJECT_ID/locations/LOCATION/translations/TRANSLATION_ID`. |

## Output Schema

Response message for `FetchTranslation`.

### FetchTranslationResponse

| JSON representation |
|---|
| ``` { "translation": { object (`https://docs.cloud.google.com/bigquery/docs/reference/migration/mcp/tools_list/get_translation#Output.Schema.Translation`) }, "translationLogs": [ { object (`https://docs.cloud.google.com/bigquery/docs/reference/migration/mcp/tools_list/translate_query#Output.Schema.Log`) } ], "errorInfo": { object (`https://docs.cloud.google.com/bigquery/docs/reference/migration/mcp/tools_list/translate_query#Output.Schema.ErrorInfo`) } } ``` |

| Fields ||
|---|---|
| `translation` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/migration/mcp/tools_list/get_translation#Output.Schema.Translation`)`` The translation resource. |
| `translationLogs[]` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/migration/mcp/tools_list/translate_query#Output.Schema.Log`)`` A list of logs generated during the translation process. |
| `errorInfo` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/migration/mcp/tools_list/translate_query#Output.Schema.ErrorInfo`)`` The error information. |

### Translation

| JSON representation |
|---|
| ``` { "name": string, "translatedQuery": string, "state": string } ``` |

| Fields ||
|---|---|
| `name` | `string` The name of the translation. Format: `projects/PROJECT_ID/locations/LOCATION/translations/TRANSLATION_ID`. |
| `translatedQuery` | `string` The translated query. |
| `state` | `string` The current state of the translation workflow, for example, `SUCCEEDED` or `FAILED`. |

### Log

| JSON representation |
|---|
| ``` { "severity": string, "category": string, "message": string, "action": string, "effect": string, "impactedObject": string } ``` |

| Fields ||
|---|---|
| `severity` | `string` Severity of the translation record, for example, `INFO`, `WARNING`, or `ERROR`. |
| `category` | `string` Category of the error or warning, for example, `SyntaxError`. |
| `message` | `string` Detailed message of the record. |
| `action` | `string` Recommended action to address the log. |
| `effect` | `string` The effect or impact of the issue noted in the log. Effect can be one of the following values: `CORRECTNESS`: Errors with this effect indicate that the translation service couldn't meaningfully process the translation. This is caused by issues in the user's input such as incorrect language or formatting, or using an unsupported file type. `COMPLETENESS`: Errors with this effect indicate that the translation service doesn't have sufficient information to complete the translation. This can be caused by missing information in the user's input such as missing metadata for name resolution. `COMPATIBILITY`: Errors with this effect indicate that the translation service encountered compatibility issues when it processed the translation. This can happen when the target platform doesn't support a feature used in the input script, and the translation service tries to make a semantic approximation for the target platform. `NONE`: Errors with this effect are purely informational messages that have no effect on the output. Effects are ordered by their stage in the translation process. For example, `CORRECTNESS` issues are identified before `COMPLETENESS` issues. |
| `impactedObject` | `string` Name of the object that is impacted by the log message. |

### ErrorInfo

| JSON representation |
|---|
| ``` { "reason": string, "domain": string, "metadata": { string: string, ... } } ``` |

| Fields ||
|---|---|
| `reason` | `string` The reason of the error. This is a constant value that identifies the proximate cause of the error. Error reasons are unique within a particular domain of errors. This should be at most 63 characters and match a regular expression of `[A-Z][A-Z0-9_]+[A-Z0-9]`, which represents UPPER_SNAKE_CASE. |
| `domain` | `string` The logical grouping to which the "reason" belongs. The error domain is typically the registered service name of the tool or product that generates the error. Example: "pubsub.googleapis.com". If the error is generated by some common infrastructure, the error domain must be a globally unique value that identifies the infrastructure. For Google API infrastructure, the error domain is "googleapis.com". |
| `metadata` | `map (key: string, value: string)` Additional structured details about this error. Keys must match a regular expression of `[a-z][a-zA-Z0-9-_]+` but should ideally be lowerCamelCase. Also, they must be limited to 64 characters in length. When identifying the current value of an exceeded limit, the units should be contained in the key, not the value. For example, rather than `{"instanceLimit": "100/request"}`, should be returned as, `{"instanceLimitPerRequest": "100"}`, if the client exceeds the number of instances that can be created in a single (batch) request. An object containing a list of `"key": value` pairs. Example: `{ "name": "wrench", "mass": "1.3kg", "count": "3" }`. |

### MetadataEntry

| JSON representation |
|---|
| ``` { "key": string, "value": string } ``` |

| Fields ||
|---|---|
| `key` | `string` |
| `value` | `string` |

### Tool Annotations

Destructive Hint: ❌ \| Idempotent Hint: ✅ \| Read Only Hint: ✅ \| Open World Hint: ❌