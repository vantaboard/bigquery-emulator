# MCP Tools Reference: bigquerymigration.googleapis.com

## Tool: `explain_translation`

Explains the SQL translation for a given translation ID.

The following sample demonstrate how to use `curl` to invoke the `explain_translation` MCP tool.

| Curl Request |
|---|
| ```bash curl --location 'https://bigquerymigration.googleapis.com/mcp' \ --header 'content-type: application/json' \ --header 'accept: application/json, text/event-stream' \ --data '{ "method": "tools/call", "params": { "name": "explain_translation", "arguments": { // provide these details according to the tool's MCP specification } }, "jsonrpc": "2.0", "id": 1 }' ``` |

## Input Schema

Request message for `ExplainTranslation`.

### ExplainTranslationRequest

| JSON representation |
|---|
| ``` { "name": string } ``` |

| Fields ||
|---|---|
| `name` | `string` Required. The name of the translation to explain. Format: `projects/PROJECT_ID/locations/LOCATION/translations/TRANSLATION_ID`. |

## Output Schema

Response message for `ExplainTranslation`.

### ExplainTranslationResponse

| JSON representation |
|---|
| ``` { "explanation": string } ``` |

| Fields ||
|---|---|
| `explanation` | `string` The string that explains the translation. |

### Tool Annotations

Destructive Hint: ❌ \| Idempotent Hint: ✅ \| Read Only Hint: ✅ \| Open World Hint: ❌