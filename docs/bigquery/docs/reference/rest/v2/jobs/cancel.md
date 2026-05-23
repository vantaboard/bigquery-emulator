# Method: jobs.cancel

- [HTTP request](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs/cancel#body.HTTP_TEMPLATE)
- [Path parameters](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs/cancel#body.PATH_PARAMETERS)
- [Query parameters](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs/cancel#body.QUERY_PARAMETERS)
- [Request body](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs/cancel#body.request_body)
- [Response body](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs/cancel#body.response_body)
  - [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs/cancel#body.JobCancelResponse.SCHEMA_REPRESENTATION)
- [Authorization scopes](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs/cancel#body.aspect)
- [Try it!](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs/cancel#try-it)

Requests that a job be cancelled. This call will return immediately, and the client will need to poll for the job status to see if the cancel completed successfully. Cancelled jobs may still incur costs.

### HTTP request

`POST https://bigquery.googleapis.com/bigquery/v2/projects/{projectId}/jobs/{jobId}/cancel`

The URL uses [gRPC Transcoding](https://google.aip.dev/127) syntax.

### Path parameters

| Parameters ||
|---|---|
| `projectId` | `string` Required. Project ID of the job to cancel |
| `jobId` | `string` Required. Job ID of the job to cancel |

### Query parameters

| Parameters ||
|---|---|
| `location` | `string` The geographic location of the job. You must [specify the location](https://cloud.google.com/bigquery/docs/locations#specify_locations) to run the job for the following scenarios: - If the location to run a job is not in the `us` or the `eu` multi-regional location - If the job's location is in a single region (for example, `us-central1`) |

### Request body

The request body must be empty.

### Response body

Describes format of a jobs cancellation response.

If successful, the response body contains data with the following structure:

| JSON representation |
|---|
| ``` { "kind": string, "job": { object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/Job`) } } ``` |

| Fields ||
|---|---|
| `kind` | `string` The resource type of the response. |
| `job` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/Job`)`` The final state of the job. |

### Authorization scopes

Requires one of the following OAuth scopes:

- `https://www.googleapis.com/auth/bigquery`
- `https://www.googleapis.com/auth/cloud-platform`

For more information, see the [Authentication Overview](https://docs.cloud.google.com/docs/authentication#authorization-gcp).