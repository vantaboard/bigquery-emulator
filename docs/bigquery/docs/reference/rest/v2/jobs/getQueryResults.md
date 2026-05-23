# Method: jobs.getQueryResults

- [HTTP request](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs/getQueryResults#body.HTTP_TEMPLATE)
- [Path parameters](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs/getQueryResults#body.PATH_PARAMETERS)
- [Query parameters](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs/getQueryResults#body.QUERY_PARAMETERS)
- [Request body](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs/getQueryResults#body.request_body)
- [Response body](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs/getQueryResults#body.response_body)
  - [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs/getQueryResults#body.GetQueryResultsResponse.SCHEMA_REPRESENTATION)
- [Authorization scopes](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs/getQueryResults#body.aspect)
- [Try it!](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs/getQueryResults#try-it)

RPC to get the results of a query job.

### HTTP request

`GET https://bigquery.googleapis.com/bigquery/v2/projects/{projectId}/queries/{jobId}`

The URL uses [gRPC Transcoding](https://google.aip.dev/127) syntax.

### Path parameters

| Parameters ||
|---|---|
| `projectId` | `string` Required. Project ID of the query job. |
| `jobId` | `string` Required. Job ID of the query job. |

### Query parameters

| Parameters ||
|---|---|
| `startIndex` | `string (https://developers.google.com/discovery/v1/type-format format)` Zero-based index of the starting row. |
| `pageToken` | `string` Page token, returned by a previous call, to request the next page of results. |
| `maxResults` | `integer` Maximum number of results to read. |
| `timeoutMs` | `integer` Optional: Specifies the maximum amount of time, in milliseconds, that the client is willing to wait for the query to complete. By default, this limit is 10 seconds (10,000 milliseconds). If the query is complete, the jobComplete field in the response is true. If the query has not yet completed, jobComplete is false. You can request a longer timeout period in the timeoutMs field. However, the call is not guaranteed to wait for the specified timeout; it typically returns after around 200 seconds (200,000 milliseconds), even if the query is not complete. If jobComplete is false, you can continue to wait for the query to complete by calling the getQueryResults method until the jobComplete field in the getQueryResults response is true. |
| `location` | `string` The geographic location of the job. You must specify the location to run the job for the following scenarios: - If the location to run a job is not in the `us` or the `eu` multi-regional location - If the job's location is in a single region (for example, `us-central1`) For more information, see how to [specify locations](https://cloud.google.com/bigquery/docs/locations#specify_locations). |
| `formatOptions` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/DataFormatOptions`)`` Optional. Output format adjustments. |

### Request body

The request body must be empty.

### Response body

Response object of jobs.getQueryResults.

If successful, the response body contains data with the following structure:

| JSON representation |
|---|
| ``` { "kind": string, "etag": string, "schema": { object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables#TableSchema`) }, "jobReference": { object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/JobReference`) }, "totalRows": string, "pageToken": string, "rows": [ { object } ], "totalBytesProcessed": string, "jobComplete": boolean, "errors": [ { object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables#ErrorProto`) } ], "cacheHit": boolean, "numDmlAffectedRows": string } ``` |

| Fields ||
|---|---|
| `kind` | `string` The resource type of the response. |
| `etag` | `string` A hash of this response. |
| `schema` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables#TableSchema`)`` The schema of the results. Present only when the query completes successfully. |
| `jobReference` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/JobReference`)`` Reference to the BigQuery Job that was created to run the query. This field will be present even if the original request timed out, in which case jobs.getQueryResults can be used to read the results once the query has completed. Since this API only returns the first page of results, subsequent pages can be fetched via the same mechanism (jobs.getQueryResults). |
| `totalRows` | `string (https://developers.google.com/discovery/v1/type-format format)` The total number of rows in the complete query result set, which can be more than the number of rows in this single page of results. Present only when the query completes successfully. |
| `pageToken` | `string` A token used for paging results. When this token is non-empty, it indicates additional results are available. |
| `rows[]` | ``object (`https://protobuf.dev/reference/protobuf/google.protobuf#struct` format)`` An object with as many results as can be contained within the maximum permitted reply size. To get any additional rows, you can call jobs.getQueryResults and specify the jobReference returned above. Present only when the query completes successfully. The REST-based representation of this data leverages a series of JSON f,v objects for indicating fields and values. |
| `totalBytesProcessed` | `string (https://developers.google.com/discovery/v1/type-format format)` The total number of bytes processed for this query. |
| `jobComplete` | `boolean` Whether the query has completed or not. If rows or totalRows are present, this will always be true. If this is false, totalRows will not be available. |
| `errors[]` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables#ErrorProto`)`` Output only. The first errors or warnings encountered during the running of the job. The final message includes the number of errors that caused the process to stop. Errors here do not necessarily mean that the job has completed or was unsuccessful. For more information about error messages, see [Error messages](https://cloud.google.com/bigquery/docs/error-messages). |
| `cacheHit` | `boolean` Whether the query result was fetched from the query cache. |
| `numDmlAffectedRows` | `string (https://developers.google.com/discovery/v1/type-format format)` Output only. The number of rows affected by a DML statement. Present only for DML statements INSERT, UPDATE or DELETE. |

### Authorization scopes

Requires one of the following OAuth scopes:

- `https://www.googleapis.com/auth/bigquery`
- `https://www.googleapis.com/auth/cloud-platform`
- `https://www.googleapis.com/auth/bigquery.readonly`
- `https://www.googleapis.com/auth/cloud-platform.read-only`

For more information, see the [Authentication Overview](https://docs.cloud.google.com/docs/authentication#authorization-gcp).