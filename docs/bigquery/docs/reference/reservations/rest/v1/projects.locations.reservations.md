# REST Resource: projects.locations.reservations.assignments

- [Resource: Assignment](https://docs.cloud.google.com/bigquery/docs/reference/reservations/rest/v1/projects.locations.reservations.assignments#Assignment)
  - [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/reservations/rest/v1/projects.locations.reservations.assignments#Assignment.SCHEMA_REPRESENTATION)
- [JobType](https://docs.cloud.google.com/bigquery/docs/reference/reservations/rest/v1/projects.locations.reservations.assignments#JobType)
- [State](https://docs.cloud.google.com/bigquery/docs/reference/reservations/rest/v1/projects.locations.reservations.assignments#State)
- [Methods](https://docs.cloud.google.com/bigquery/docs/reference/reservations/rest/v1/projects.locations.reservations.assignments#METHODS_SUMMARY)

## Resource: Assignment

An assignment allows a project to submit jobs of a certain type using slots from the specified reservation.

| JSON representation |
|---|
| ``` { "name": string, "assignee": string, "jobType": enum (`https://docs.cloud.google.com/bigquery/docs/reference/reservations/rest/v1/projects.locations.reservations.assignments#JobType`), "state": enum (`https://docs.cloud.google.com/bigquery/docs/reference/reservations/rest/v1/projects.locations.reservations.assignments#State`) } ``` |

| Fields ||
|---|---|
| `name` | `string` Output only. Name of the resource. E.g.: `projects/myproject/locations/US/reservations/team1-prod/assignments/123`. The assignmentId must only contain lower case alphanumeric characters or dashes and the max length is 64 characters. |
| `assignee` | `string` Optional. The resource which will use the reservation. E.g. `projects/myproject`, `folders/123`, or `organizations/456`. |
| `jobType` | ``enum (`https://docs.cloud.google.com/bigquery/docs/reference/reservations/rest/v1/projects.locations.reservations.assignments#JobType`)`` Optional. Which type of jobs will use the reservation. |
| `state` | ``enum (`https://docs.cloud.google.com/bigquery/docs/reference/reservations/rest/v1/projects.locations.reservations.assignments#State`)`` Output only. State of the assignment. |

## JobType

Types of job, which could be specified when using the reservation.

| Enums ||
|---|---|
| `JOB_TYPE_UNSPECIFIED` | Invalid type. Requests with this value will be rejected with error code `google.rpc.Code.INVALID_ARGUMENT`. |
| `PIPELINE` | Pipeline (load/export) jobs from the project will use the reservation. |
| `QUERY` | Query jobs from the project will use the reservation. |
| `ML_EXTERNAL` | BigQuery ML jobs that use services external to BigQuery for model training. These jobs will not utilize idle slots from other reservations. |
| `BACKGROUND` | Background jobs that BigQuery runs for the customers in the background. |
| `CONTINUOUS` | Continuous SQL jobs will use this reservation. Reservations with continuous assignments cannot be mixed with non-continuous assignments. |
| `BACKGROUND_CHANGE_DATA_CAPTURE` | Finer granularity background jobs for capturing changes in a source database and streaming them into BigQuery. Reservations with this job type take priority over a default BACKGROUND reservation assignment (if it exists). |
| `BACKGROUND_COLUMN_METADATA_INDEX` | Finer granularity background jobs for refreshing cached metadata for BigQuery tables. Reservations with this job type take priority over a default BACKGROUND reservation assignment (if it exists). |
| `BACKGROUND_SEARCH_INDEX_REFRESH` | Finer granularity background jobs for refreshing search indexes upon BigQuery table columns. Reservations with this job type take priority over a default BACKGROUND reservation assignment (if it exists). |

## State

Assignment will remain in PENDING state if no active capacity commitment is present. It will become ACTIVE when some capacity commitment becomes active.

| Enums ||
|---|---|
| `STATE_UNSPECIFIED` | Invalid state value. |
| `PENDING` | Queries from assignee will be executed as on-demand, if related assignment is pending. |
| `ACTIVE` | Assignment is ready. |

| ## Methods ||
|---|---|
| ### `https://docs.cloud.google.com/bigquery/docs/reference/reservations/rest/v1/projects.locations.reservations.assignments/create` | Creates an assignment object which allows the given project to submit jobs of a certain type using slots from the specified reservation. |
| ### `https://docs.cloud.google.com/bigquery/docs/reference/reservations/rest/v1/projects.locations.reservations.assignments/delete` | Deletes a assignment. |
| ### `https://docs.cloud.google.com/bigquery/docs/reference/reservations/rest/v1/projects.locations.reservations.assignments/getIamPolicy` | Gets the access control policy for a resource. |
| ### `https://docs.cloud.google.com/bigquery/docs/reference/reservations/rest/v1/projects.locations.reservations.assignments/list` | Lists assignments. |
| ### `https://docs.cloud.google.com/bigquery/docs/reference/reservations/rest/v1/projects.locations.reservations.assignments/move` | Moves an assignment under a new reservation. |
| ### `https://docs.cloud.google.com/bigquery/docs/reference/reservations/rest/v1/projects.locations.reservations.assignments/patch` | Updates an existing assignment. |
| ### `https://docs.cloud.google.com/bigquery/docs/reference/reservations/rest/v1/projects.locations.reservations.assignments/setIamPolicy` | Sets an access control policy for a resource. |
| ### `https://docs.cloud.google.com/bigquery/docs/reference/reservations/rest/v1/projects.locations.reservations.assignments/testIamPermissions` | Gets your permissions on a resource. |