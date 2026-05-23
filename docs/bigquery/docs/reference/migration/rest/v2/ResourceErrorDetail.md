# ResourceErrorDetail

- [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/v2/ResourceErrorDetail#SCHEMA_REPRESENTATION)
- [ErrorDetail](https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/v2/ResourceErrorDetail#ErrorDetail)
  - [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/v2/ResourceErrorDetail#ErrorDetail.SCHEMA_REPRESENTATION)
- [ErrorLocation](https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/v2/ResourceErrorDetail#ErrorLocation)
  - [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/v2/ResourceErrorDetail#ErrorLocation.SCHEMA_REPRESENTATION)

Provides details for errors and the corresponding resources.

| JSON representation |
|---|
| ``` { "resourceInfo": { object (`https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/Shared.Types/ResourceInfo`) }, "errorDetails": [ { object (`https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/v2/ResourceErrorDetail#ErrorDetail`) } ], "errorCount": integer } ``` |

| Fields ||
|---|---|
| `resourceInfo` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/Shared.Types/ResourceInfo`)`` Required. Information about the resource where the error is located. |
| `errorDetails[]` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/v2/ResourceErrorDetail#ErrorDetail`)`` Required. The error details for the resource. |
| `errorCount` | `integer` Required. How many errors there are in total for the resource. Truncation can be indicated by having an `errorCount` that is higher than the size of `errorDetails`. |

## ErrorDetail

Provides details for errors, e.g. issues that where encountered when processing a subtask.

| JSON representation |
|---|
| ``` { "location": { object (`https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/v2/ResourceErrorDetail#ErrorLocation`) }, "errorInfo": { object (`https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/Shared.Types/ErrorInfo`) } } ``` |

| Fields ||
|---|---|
| `location` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/v2/ResourceErrorDetail#ErrorLocation`)`` Optional. The exact location within the resource (if applicable). |
| `errorInfo` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/Shared.Types/ErrorInfo`)`` Required. Describes the cause of the error with structured detail. |

## ErrorLocation

Holds information about where the error is located.

| JSON representation |
|---|
| ``` { "line": integer, "column": integer } ``` |

| Fields ||
|---|---|
| `line` | `integer` Optional. If applicable, denotes the line where the error occurred. A zero value means that there is no line information. |
| `column` | `integer` Optional. If applicable, denotes the column where the error occurred. A zero value means that there is no columns information. |