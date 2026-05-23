# Method: jobs.insert

- [HTTP request](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs/insert#body.HTTP_TEMPLATE)
- [Path parameters](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs/insert#body.PATH_PARAMETERS)
- [Request body](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs/insert#body.request_body)
- [Response body](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs/insert#body.response_body)
- [Authorization scopes](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs/insert#body.aspect)
- [Try it!](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs/insert#try-it)

Starts a new asynchronous job.

This API has two different kinds of endpoint URIs, as this method supports a variety of use cases.

- The *Metadata* URI is used for most interactions, as it accepts the job configuration directly.
- The *Upload* URI is ONLY for the case when you're sending both a load job configuration and a data stream together. In this case, the Upload URI accepts the job configuration and the data as two distinct multipart MIME parts.

### HTTP request

- Upload URI, for media upload requests:  
  `POST https://bigquery.googleapis.com/upload/bigquery/v2/projects/{projectId}/jobs`
- Metadata URI, for metadata-only requests:  
  `POST https://bigquery.googleapis.com/bigquery/v2/projects/{projectId}/jobs`

The URL uses [gRPC Transcoding](https://google.aip.dev/127) syntax.

### Path parameters

| Parameters ||
|---|---|
| `projectId` | `string` Project ID of project that will be billed for the job. |

### Request body

The request body contains an instance of `https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/Job`.

### Response body

If successful, the response body contains an instance of `https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/Job`.

### Authorization scopes

Requires one of the following OAuth scopes:

- `https://www.googleapis.com/auth/bigquery`
- `https://www.googleapis.com/auth/cloud-platform`
- `https://www.googleapis.com/auth/devstorage.full_control`
- `https://www.googleapis.com/auth/devstorage.read_only`
- `https://www.googleapis.com/auth/devstorage.read_write`

For more information, see the [Authentication Overview](https://docs.cloud.google.com/docs/authentication#authorization-gcp).