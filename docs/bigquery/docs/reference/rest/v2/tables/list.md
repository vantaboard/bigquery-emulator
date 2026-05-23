# Method: tables.list

- [HTTP request](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables/list#body.HTTP_TEMPLATE)
- [Path parameters](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables/list#body.PATH_PARAMETERS)
- [Query parameters](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables/list#body.QUERY_PARAMETERS)
- [Request body](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables/list#body.request_body)
- [Response body](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables/list#body.response_body)
  - [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables/list#body.TableList.SCHEMA_REPRESENTATION)
    - [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables/list#body.TableList.SCHEMA_REPRESENTATION.tables.SCHEMA_REPRESENTATION)
- [Authorization scopes](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables/list#body.aspect)
- [Try it!](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables/list#try-it)

Lists all tables in the specified dataset. Requires the READER dataset role.

### HTTP request

`GET https://bigquery.googleapis.com/bigquery/v2/projects/{projectId}/datasets/{datasetId}/tables`

The URL uses [gRPC Transcoding](https://google.aip.dev/127) syntax.

### Path parameters

| Parameters ||
|---|---|
| `projectId` | `string` Required. Project ID of the tables to list |
| `datasetId` | `string` Required. Dataset ID of the tables to list |

### Query parameters

| Parameters ||
|---|---|
| `maxResults` | `integer` The maximum number of results to return in a single response page. Leverage the page tokens to iterate through the entire collection. |
| `pageToken` | `string` Page token, returned by a previous call, to request the next page of results |

### Request body

The request body must be empty.

### Response body

Partial projection of the metadata for a given table in a list response.

If successful, the response body contains data with the following structure:

| JSON representation |
|---|
| ``` { "kind": string, "etag": string, "nextPageToken": string, "tables": [ { "kind": string, "id": string, "tableReference": { object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/TableReference`) }, "friendlyName": string, "type": string, "timePartitioning": { object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables#TimePartitioning`) }, "rangePartitioning": { object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables#RangePartitioning`) }, "clustering": { object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables#Clustering`) }, "hivePartitioningOptions": { object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables#HivePartitioningOptions`) }, "labels": { string: string, ... }, "view": { "useLegacySql": boolean, "privacyPolicy": { object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables#PrivacyPolicy`) } }, "creationTime": string, "expirationTime": string, "requirePartitionFilter": boolean } ], "totalItems": integer } ``` |

| Fields ||
|---|---|
| `kind` | `string` The type of list. |
| `etag` | `string` A hash of this page of results. |
| `nextPageToken` | `string` A token to request the next page of results. |
| `tables[]` | `object` Tables in the requested dataset. |
| `tables[].kind` | `string` The resource type. |
| `tables[].id` | `string` An opaque ID of the table. |
| `tables[].tableReference` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/TableReference`)`` A reference uniquely identifying table. |
| `tables[].friendlyName` | `string` The user-friendly name for this table. |
| `tables[].type` | `string` The type of table. |
| `tables[].timePartitioning` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables#TimePartitioning`)`` The time-based partitioning for this table. |
| `tables[].rangePartitioning` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables#RangePartitioning`)`` The range partitioning for this table. |
| `tables[].clustering` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables#Clustering`)`` Clustering specification for this table, if configured. |
| `tables[].hivePartitioningOptions` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables#HivePartitioningOptions`)`` The hive partitioning configuration for this table, when applicable. |
| `tables[].labels` | `map (key: string, value: string)` The labels associated with this table. You can use these to organize and group your tables. |
| `tables[].view` | `object` Additional details for a view. |
| `tables[].view.useLegacySql` | `boolean` True if view is defined in legacy SQL dialect, false if in GoogleSQL. |
| `tables[].view.privacyPolicy` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables#PrivacyPolicy`)`` Specifies the privacy policy for the view. |
| `tables[].creationTime` | `string (https://developers.google.com/discovery/v1/type-format format)` Output only. The time when this table was created, in milliseconds since the epoch. |
| `tables[].expirationTime` | `string (https://developers.google.com/discovery/v1/type-format format)` The time when this table expires, in milliseconds since the epoch. If not present, the table will persist indefinitely. Expired tables will be deleted and their storage reclaimed. |
| `tables[].requirePartitionFilter` | `boolean` Optional. If set to true, queries including this table must specify a partition filter. This filter is used for partition elimination. |
| `totalItems` | `integer` The total number of tables in the dataset. |

### Authorization scopes

Requires one of the following OAuth scopes:

- `https://www.googleapis.com/auth/bigquery`
- `https://www.googleapis.com/auth/cloud-platform`
- `https://www.googleapis.com/auth/bigquery.readonly`
- `https://www.googleapis.com/auth/cloud-platform.read-only`

For more information, see the [Authentication Overview](https://docs.cloud.google.com/docs/authentication#authorization-gcp).