# REST Resource: projects.locations.dataExchanges

- [Resource: DataExchange](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1beta1/projects.locations.dataExchanges#DataExchange)
  - [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1beta1/projects.locations.dataExchanges#DataExchange.SCHEMA_REPRESENTATION)
- [Methods](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1beta1/projects.locations.dataExchanges#METHODS_SUMMARY)

## Resource: DataExchange

A data exchange is a container that lets you share data. Along with the descriptive information about the data exchange, it contains listings that reference shared datasets.

| JSON representation |
|---|
| ``` { "name": string, "displayName": string, "description": string, "primaryContact": string, "documentation": string, "listingCount": integer, "icon": string } ``` |

| Fields ||
|---|---|
| `name` | `string` Output only. The resource name of the data exchange. e.g. `projects/myproject/locations/us/dataExchanges/123`. |
| `displayName` | `string` Required. Human-readable display name of the data exchange. The display name must contain only Unicode letters, numbers (0-9), underscores (_), dashes (-), spaces ( ), ampersands (\&) and must not start or end with spaces. Default value is an empty string. Max length: 63 bytes. |
| `description` | `string` Optional. Description of the data exchange. The description must not contain Unicode non-characters as well as C0 and C1 control codes except tabs (HT), new lines (LF), carriage returns (CR), and page breaks (FF). Default value is an empty string. Max length: 2000 bytes. |
| `primaryContact` | `string` Optional. Email or URL of the primary point of contact of the data exchange. Max Length: 1000 bytes. |
| `documentation` | `string` Optional. Documentation describing the data exchange. |
| `listingCount` | `integer` Output only. Number of listings contained in the data exchange. |
| `icon` | `string (https://developers.google.com/discovery/v1/type-format format)` Optional. Base64 encoded image representing the data exchange. Max Size: 3.0MiB Expected image dimensions are 512x512 pixels, however the API only performs validation on size of the encoded data. Note: For byte fields, the content of the fields are base64-encoded (which increases the size of the data by 33-36%) when using JSON on the wire. A base64-encoded string. |

| ## Methods ||
|---|---|
| ### `https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1beta1/projects.locations.dataExchanges/create` | Creates a new data exchange. |
| ### `https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1beta1/projects.locations.dataExchanges/delete` | Deletes an existing data exchange. |
| ### `https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1beta1/projects.locations.dataExchanges/get` | Gets the details of a data exchange. |
| ### `https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1beta1/projects.locations.dataExchanges/getIamPolicy` | Gets the IAM policy. |
| ### `https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1beta1/projects.locations.dataExchanges/list` | Lists all data exchanges in a given project and location. |
| ### `https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1beta1/projects.locations.dataExchanges/patch` | Updates an existing data exchange. |
| ### `https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1beta1/projects.locations.dataExchanges/setIamPolicy` | Sets the IAM policy. |
| ### `https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1beta1/projects.locations.dataExchanges/testIamPermissions` | Returns the permissions that a caller has. |