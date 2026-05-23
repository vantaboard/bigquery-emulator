# Method: jobs.query

- [HTTP request](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs/query#body.HTTP_TEMPLATE)
- [Path parameters](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs/query#body.PATH_PARAMETERS)
- [Request body](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs/query#body.request_body)
- [Response body](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs/query#body.response_body)
  - [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs/query#body.QueryResponse.SCHEMA_REPRESENTATION)
- [Authorization scopes](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs/query#body.aspect)
- [QueryRequest](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs/query#QueryRequest)
  - [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs/query#QueryRequest.SCHEMA_REPRESENTATION)
- [JobCreationMode](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs/query#JobCreationMode)
- [Try it!](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs/query#try-it)

Runs a BigQuery SQL query synchronously and returns query results if the query completes within a specified timeout.

### HTTP request

`POST https://bigquery.googleapis.com/bigquery/v2/projects/{projectId}/queries`

The URL uses [gRPC Transcoding](https://google.aip.dev/127) syntax.

### Path parameters

| Parameters ||
|---|---|
| `projectId` | `string` Required. Project ID of the query request. |

### Request body

The request body contains an instance of `https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs/query#QueryRequest`.

### Response body

If successful, the response body contains data with the following structure:

| JSON representation |
|---|
| ``` { "kind": string, "schema": { object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables#TableSchema`) }, "jobReference": { object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/JobReference`) }, "jobCreationReason": { object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/JobCreationReason`) }, "queryId": string, "location": string, "totalRows": string, "pageToken": string, "rows": [ { object } ], "totalBytesProcessed": string, "jobComplete": boolean, "errors": [ { object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables#ErrorProto`) } ], "cacheHit": boolean, "numDmlAffectedRows": string, "sessionInfo": { object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/SessionInfo`) }, "dmlStats": { object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/DmlStats`) }, "totalBytesBilled": string, "totalSlotMs": string, "creationTime": string, "startTime": string, "endTime": string } ``` |

| Fields ||
|---|---|
| `kind` | `string` The resource type. |
| `schema` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables#TableSchema`)`` The schema of the results. Present only when the query completes successfully. |
| `jobReference` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/JobReference`)`` Reference to the Job that was created to run the query. This field will be present even if the original request timed out, in which case jobs.getQueryResults can be used to read the results once the query has completed. Since this API only returns the first page of results, subsequent pages can be fetched via the same mechanism (jobs.getQueryResults). If jobCreationMode was set to `JOB_CREATION_OPTIONAL` and the query completes without creating a job, this field will be empty. |
| `jobCreationReason` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/JobCreationReason`)`` Optional. The reason why a Job was created. Only relevant when a jobReference is present in the response. If jobReference is not present it will always be unset. |
| `queryId` | `string` Auto-generated ID for the query. |
| `location` | `string` Output only. The geographic location of the query. For more information about BigQuery locations, see: <https://cloud.google.com/bigquery/docs/locations> |
| `totalRows` | `string (https://developers.google.com/discovery/v1/type-format format)` The total number of rows in the complete query result set, which can be more than the number of rows in this single page of results. |
| `pageToken` | `string` A token used for paging results. A non-empty token indicates that additional results are available. To see additional results, query the [`jobs.getQueryResults`](https://cloud.google.com/bigquery/docs/reference/rest/v2/jobs/getQueryResults) method. For more information, see [Paging through table data](https://cloud.google.com/bigquery/docs/paging-results). |
| `rows[]` | ``object (`https://protobuf.dev/reference/protobuf/google.protobuf#struct` format)`` An object with as many results as can be contained within the maximum permitted reply size. To get any additional rows, you can call jobs.getQueryResults and specify the jobReference returned above. |
| `totalBytesProcessed` | `string (https://developers.google.com/discovery/v1/type-format format)` The total number of bytes processed for this query. If this query was a dry run, this is the number of bytes that would be processed if the query were run. |
| `jobComplete` | `boolean` Whether the query has completed or not. If rows or totalRows are present, this will always be true. If this is false, totalRows will not be available. |
| `errors[]` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables#ErrorProto`)`` Output only. The first errors or warnings encountered during the running of the job. The final message includes the number of errors that caused the process to stop. Errors here do not necessarily mean that the job has completed or was unsuccessful. For more information about error messages, see [Error messages](https://cloud.google.com/bigquery/docs/error-messages). |
| `cacheHit` | `boolean` Whether the query result was fetched from the query cache. |
| `numDmlAffectedRows` | `string (https://developers.google.com/discovery/v1/type-format format)` Output only. The number of rows affected by a DML statement. Present only for DML statements INSERT, UPDATE or DELETE. |
| `sessionInfo` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/SessionInfo`)`` Output only. Information of the session if this job is part of one. |
| `dmlStats` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/DmlStats`)`` Output only. Detailed statistics for DML statements INSERT, UPDATE, DELETE, MERGE or TRUNCATE. |
| `totalBytesBilled` | `string (https://developers.google.com/discovery/v1/type-format format)` Output only. If the project is configured to use on-demand pricing, then this field contains the total bytes billed for the job. If the project is configured to use flat-rate pricing, then you are not billed for bytes and this field is informational only. |
| `totalSlotMs` | `string (https://developers.google.com/discovery/v1/type-format format)` Output only. Number of slot ms the user is actually billed for. |
| `creationTime` | `string (https://developers.google.com/discovery/v1/type-format format)` Output only. Creation time of this query, in milliseconds since the epoch. This field will be present on all queries. |
| `startTime` | `string (https://developers.google.com/discovery/v1/type-format format)` Output only. Start time of this query, in milliseconds since the epoch. This field will be present when the query job transitions from the PENDING state to either RUNNING or DONE. |
| `endTime` | `string (https://developers.google.com/discovery/v1/type-format format)` Output only. End time of this query, in milliseconds since the epoch. This field will be present whenever a query job is in the DONE state. |

### Authorization scopes

Requires one of the following OAuth scopes:

- `https://www.googleapis.com/auth/bigquery`
- `https://www.googleapis.com/auth/cloud-platform`
- `https://www.googleapis.com/auth/bigquery.readonly`
- `https://www.googleapis.com/auth/cloud-platform.read-only`

For more information, see the [Authentication Overview](https://docs.cloud.google.com/docs/authentication#authorization-gcp).

## QueryRequest

Describes the format of the jobs.query request.

| JSON representation |
|---|
| ``` { "kind": string, "query": string, "maxResults": integer, "defaultDataset": { object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets#DatasetReference`) }, "timeoutMs": integer, "destinationEncryptionConfiguration": { object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/EncryptionConfiguration`) }, "dryRun": boolean, "preserveNulls": boolean, "useQueryCache": boolean, "useLegacySql": boolean, "parameterMode": string, "queryParameters": [ { object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/QueryParameter`) } ], "location": string, "formatOptions": { object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/DataFormatOptions`) }, "connectionProperties": [ { object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/ConnectionProperty`) } ], "labels": { string: string, ... }, "maximumBytesBilled": string, "requestId": string, "createSession": boolean, "jobCreationMode": enum (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs/query#JobCreationMode`), "jobTimeoutMs": string, "reservation": string } ``` |

| Fields ||
|---|---|
| `kind` | `string` The resource type of the request. |
| `query` | `string` Required. A query string to execute, using Google Standard SQL or legacy SQL syntax. Example: "SELECT COUNT(f1) FROM myProjectId.myDatasetId.myTableId". |
| `maxResults` | `integer` Optional. The maximum number of rows of data to return per page of results. Setting this flag to a small value such as 1000 and then paging through results might improve reliability when the query result set is large. In addition to this limit, responses are also limited to 10 MB. By default, there is no maximum row count, and only the byte limit applies. |
| `defaultDataset` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets#DatasetReference`)`` Optional. Specifies the default datasetId and projectId to assume for any unqualified table names in the query. If not set, all table names in the query string must be qualified in the format 'datasetId.tableId'. |
| `timeoutMs` | `integer` Optional. Optional: Specifies the maximum amount of time, in milliseconds, that the client is willing to wait for the query to complete. By default, this limit is 10 seconds (10,000 milliseconds). If the query is complete, the jobComplete field in the response is true. If the query has not yet completed, jobComplete is false. You can request a longer timeout period in the timeoutMs field. However, the call is not guaranteed to wait for the specified timeout; it typically returns after around 200 seconds (200,000 milliseconds), even if the query is not complete. If jobComplete is false, you can continue to wait for the query to complete by calling the getQueryResults method until the jobComplete field in the getQueryResults response is true. |
| `destinationEncryptionConfiguration` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/EncryptionConfiguration`)`` Optional. Custom encryption configuration (e.g., Cloud KMS keys) |
| `dryRun` | `boolean` Optional. If set to true, BigQuery doesn't run the job. Instead, if the query is valid, BigQuery returns statistics about the job such as how many bytes would be processed. If the query is invalid, an error returns. The default value is false. |
| `preserveNulls (deprecated)` | `boolean` > [!WARNING] > This item is deprecated! This property is deprecated. |
| `useQueryCache` | `boolean` Optional. Whether to look for the result in the query cache. The query cache is a best-effort cache that will be flushed whenever tables in the query are modified. The default value is true. |
| `useLegacySql` | `boolean` Specifies whether to use BigQuery's legacy SQL dialect for this query. The default value is true. If set to false, the query uses BigQuery's [GoogleSQL](https://docs.cloud.google.com/bigquery/docs/introduction-sql). When useLegacySql is set to false, the value of flattenResults is ignored; query will be run as if flattenResults is false. |
| `parameterMode` | `string` GoogleSQL only. Set to POSITIONAL to use positional (?) query parameters or to NAMED to use named (@myparam) query parameters in this query. |
| `queryParameters[]` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/QueryParameter`)`` jobs.query parameters for GoogleSQL queries. |
| `location` | `string` The geographic location where the job should run. For more information, see how to [specify locations](https://cloud.google.com/bigquery/docs/locations#specify_locations). |
| `formatOptions` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/DataFormatOptions`)`` Optional. Output format adjustments. |
| `connectionProperties[]` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/ConnectionProperty`)`` Optional. Connection properties which can modify the query behavior. |
| `labels` | `map (key: string, value: string)` Optional. The labels associated with this query. Labels can be used to organize and group query jobs. Label keys and values can be no longer than 63 characters, can only contain lowercase letters, numeric characters, underscores and dashes. International characters are allowed. Label keys must start with a letter and each label in the list must have a different key. |
| `maximumBytesBilled` | `string (https://developers.google.com/discovery/v1/type-format format)` Optional. Limits the bytes billed for this query. Queries with bytes billed above this limit will fail (without incurring a charge). If unspecified, the project default is used. |
| `requestId` | `string` Optional. A unique user provided identifier to ensure idempotent behavior for queries. Note that this is different from the jobId. It has the following properties: 1. It is case-sensitive, limited to up to 36 ASCII characters. A UUID is recommended. 2. Read only queries can ignore this token since they are nullipotent by definition. 3. For the purposes of idempotency ensured by the requestId, a request is considered duplicate of another only if they have the same requestId and are actually duplicates. When determining whether a request is a duplicate of another request, all parameters in the request that may affect the result are considered. For example, query, connectionProperties, queryParameters, useLegacySql are parameters that affect the result and are considered when determining whether a request is a duplicate, but properties like timeoutMs don't affect the result and are thus not considered. Dry run query requests are never considered duplicate of another request. 4. When a duplicate mutating query request is detected, it returns: a. the results of the mutation if it completes successfully within the timeout. b. the running operation if it is still in progress at the end of the timeout. 5. Its lifetime is limited to 15 minutes. In other words, if two requests are sent with the same requestId, but more than 15 minutes apart, idempotency is not guaranteed. |
| `createSession` | `boolean` Optional. If true, creates a new session using a randomly generated sessionId. If false, runs query with an existing sessionId passed in ConnectionProperty, otherwise runs query in non-session mode. The session location will be set to QueryRequest.location if it is present, otherwise it's set to the default location based on existing routing logic. |
| `jobCreationMode` | ``enum (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs/query#JobCreationMode`)`` Optional. If not set, jobs are always required. If set, the query request will follow the behavior described JobCreationMode. |
| `jobTimeoutMs` | `string (https://developers.google.com/discovery/v1/type-format format)` Optional. Job timeout in milliseconds. If this time limit is exceeded, BigQuery will attempt to stop a longer job, but may not always succeed in canceling it before the job completes. For example, a job that takes more than 60 seconds to complete has a better chance of being stopped than a job that takes 10 seconds to complete. This timeout applies to the query even if a job does not need to be created. |
| `reservation` | `string` Optional. The reservation that jobs.query request would use. User can specify a reservation to execute the job.query. The expected format is `projects/{project}/locations/{location}/reservations/{reservation}`. |

## JobCreationMode

Job Creation Mode provides different options on job creation.

| Enums ||
|---|---|
| `JOB_CREATION_MODE_UNSPECIFIED` | If unspecified JOB_CREATION_REQUIRED is the default. |
| `JOB_CREATION_REQUIRED` | Default. Job creation is always required. |
| `JOB_CREATION_OPTIONAL` | Job creation is optional. Returning immediate results is prioritized. BigQuery will automatically determine if a Job needs to be created. The conditions under which BigQuery can decide to not create a Job are subject to change. If Job creation is required, JOB_CREATION_REQUIRED mode should be used, which is the default. |