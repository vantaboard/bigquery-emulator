# REST Resource: projects.locations.workflows.subtasks

- [Resource: MigrationSubtask](https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/v2/projects.locations.workflows.subtasks#MigrationSubtask)
  - [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/v2/projects.locations.workflows.subtasks#MigrationSubtask.SCHEMA_REPRESENTATION)
- [State](https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/v2/projects.locations.workflows.subtasks#State)
- [Methods](https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/v2/projects.locations.workflows.subtasks#METHODS_SUMMARY)

## Resource: MigrationSubtask

A subtask for a migration which carries details about the configuration of the subtask. The content of the details should not matter to the end user, but is a contract between the subtask creator and subtask worker.

| JSON representation |
|---|
| ``` { "name": string, "taskId": string, "type": string, "state": enum (`https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/v2/projects.locations.workflows.subtasks#State`), "processingError": { object (`https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/Shared.Types/ErrorInfo`) }, "resourceErrorDetails": [ { object (`https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/v2/ResourceErrorDetail`) } ], "resourceErrorCount": integer, "createTime": string, "lastUpdateTime": string, "metrics": [ { object (`https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/v2/TimeSeries`) } ] } ``` |

| Fields ||
|---|---|
| `name` | `string` Output only. Immutable. The resource name for the migration subtask. The ID is server-generated. Example: `projects/123/locations/us/workflows/345/subtasks/678` |
| `taskId` | `string` The unique ID of the task to which this subtask belongs. |
| `type` | `string` The type of the Subtask. The migration service does not check whether this is a known type. It is up to the task creator (i.e. orchestrator or worker) to ensure it only creates subtasks for which there are compatible workers polling for Subtasks. |
| `state` | ``enum (`https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/v2/projects.locations.workflows.subtasks#State`)`` Output only. The current state of the subtask. |
| `processingError` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/Shared.Types/ErrorInfo`)`` Output only. An explanation that may be populated when the task is in FAILED state. |
| `resourceErrorDetails[]` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/v2/ResourceErrorDetail`)`` Output only. Provides details to errors and issues encountered while processing the subtask. Presence of error details does not mean that the subtask failed. |
| `resourceErrorCount` | `integer` Output only. The number or resources with errors. Note: This is not the total number of errors as each resource can have more than one error. This is used to indicate truncation by having a `resourceErrorCount` that is higher than the size of `resourceErrorDetails`. |
| `createTime` | ``string (`https://protobuf.dev/reference/protobuf/google.protobuf#timestamp` format)`` Output only. Time when the subtask was created. |
| `lastUpdateTime` | ``string (`https://protobuf.dev/reference/protobuf/google.protobuf#timestamp` format)`` Output only. Time when the subtask was last updated. |
| `metrics[]` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/v2/TimeSeries`)`` Output only. The metrics for the subtask. |

## State

Possible states of a migration subtask.

| Enums ||
|---|---|
| `STATE_UNSPECIFIED` | The state is unspecified. |
| `ACTIVE` | The subtask is ready, i.e. it is ready for execution. |
| `RUNNING` | The subtask is running, i.e. it is assigned to a worker for execution. |
| `SUCCEEDED` | The subtask finished successfully. |
| `FAILED` | The subtask finished unsuccessfully. |
| `PAUSED` | The subtask is paused, i.e., it will not be scheduled. If it was already assigned,it might still finish but no new lease renewals will be granted. |
| `PENDING_DEPENDENCY` | The subtask is pending a dependency. It will be scheduled once its dependencies are done. |

| ## Methods ||
|---|---|
| ### `https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/v2/projects.locations.workflows.subtasks/get` | Gets a previously created migration subtask. |
| ### `https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/v2/projects.locations.workflows.subtasks/list` | Lists previously created migration subtasks. |