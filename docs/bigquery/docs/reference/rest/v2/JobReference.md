# JobReference

- [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/JobReference#SCHEMA_REPRESENTATION)

A job reference is a fully qualified identifier for referring to a job.

| JSON representation |
|---|
| ``` { "projectId": string, "jobId": string, "location": string } ``` |

| Fields ||
|---|---|
| `projectId` | `string` Required. The ID of the project containing this job. |
| `jobId` | `string` Required. The ID of the job. The ID must contain only letters (a-z, A-Z), numbers (0-9), underscores (_), or dashes (-). The maximum length is 1,024 characters. |
| `location` | `string` Optional. The geographic location of the job. The default value is US. For more information about BigQuery locations, see: <https://cloud.google.com/bigquery/docs/locations> |