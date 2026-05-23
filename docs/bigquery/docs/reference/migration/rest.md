# BigQuery Migration API

The migration service, exposing apis for migration jobs operations, and agent management.

- [REST Resource: v2alpha.projects.locations.workflows](https://docs.cloud.google.com/bigquery/docs/reference/migration/rest#v2alpha.projects.locations.workflows)
- [REST Resource: v2alpha.projects.locations.workflows.subtasks](https://docs.cloud.google.com/bigquery/docs/reference/migration/rest#v2alpha.projects.locations.workflows.subtasks)
- [REST Resource: v2.projects.locations.workflows](https://docs.cloud.google.com/bigquery/docs/reference/migration/rest#v2.projects.locations.workflows)
- [REST Resource: v2.projects.locations.workflows.subtasks](https://docs.cloud.google.com/bigquery/docs/reference/migration/rest#v2.projects.locations.workflows.subtasks)

## Service: bigquerymigration.googleapis.com

To call this service, we recommend that you use the Google-provided [client libraries](https://cloud.google.com/apis/docs/client-libraries-explained). If your application needs to use your own libraries to call this service, use the following information when you make the API requests.

### Discovery document

A [Discovery Document](https://developers.google.com/discovery/v1/reference/apis) is a machine-readable specification for describing and consuming REST APIs. It is used to build client libraries, IDE plugins, and other tools that interact with Google APIs. One service may provide multiple discovery documents. This service provides the following discovery documents:

- <https://bigquerymigration.googleapis.com/$discovery/rest?version=v2>
- <https://bigquerymigration.googleapis.com/$discovery/rest?version=v2alpha>

### Service endpoint

A [service endpoint](https://cloud.google.com/apis/design/glossary#api_service_endpoint) is a base URL that specifies the network address of an API service. One service might have multiple service endpoints. This service has the following service endpoint and all URIs below are relative to this service endpoint:

- `https://bigquerymigration.googleapis.com`

## REST Resource: [v2alpha.projects.locations.workflows](https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/v2alpha/projects.locations.workflows)

| Methods ||
|---|---|
| `https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/v2alpha/projects.locations.workflows/create` | `POST /v2alpha/{parent=projects/*/locations/*}/workflows` Creates a migration workflow. |
| `https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/v2alpha/projects.locations.workflows/delete` | `DELETE /v2alpha/{name=projects/*/locations/*/workflows/*}` Deletes a migration workflow by name. |
| `https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/v2alpha/projects.locations.workflows/get` | `GET /v2alpha/{name=projects/*/locations/*/workflows/*}` Gets a previously created migration workflow. |
| `https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/v2alpha/projects.locations.workflows/list` | `GET /v2alpha/{parent=projects/*/locations/*}/workflows` Lists previously created migration workflow. |
| `https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/v2alpha/projects.locations.workflows/start` | `POST /v2alpha/{name=projects/*/locations/*/workflows/*}:start` Starts a previously created migration workflow. |

## REST Resource: [v2alpha.projects.locations.workflows.subtasks](https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/v2alpha/projects.locations.workflows.subtasks)

| Methods ||
|---|---|
| `https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/v2alpha/projects.locations.workflows.subtasks/get` | `GET /v2alpha/{name=projects/*/locations/*/workflows/*/subtasks/*}` Gets a previously created migration subtask. |
| `https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/v2alpha/projects.locations.workflows.subtasks/list` | `GET /v2alpha/{parent=projects/*/locations/*/workflows/*}/subtasks` Lists previously created migration subtasks. |

## REST Resource: [v2.projects.locations.workflows](https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/v2/projects.locations.workflows)

| Methods ||
|---|---|
| `https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/v2/projects.locations.workflows/create` | `POST /v2/{parent=projects/*/locations/*}/workflows` Creates a migration workflow. |
| `https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/v2/projects.locations.workflows/delete` | `DELETE /v2/{name=projects/*/locations/*/workflows/*}` Deletes a migration workflow by name. |
| `https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/v2/projects.locations.workflows/get` | `GET /v2/{name=projects/*/locations/*/workflows/*}` Gets a previously created migration workflow. |
| `https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/v2/projects.locations.workflows/list` | `GET /v2/{parent=projects/*/locations/*}/workflows` Lists previously created migration workflow. |
| `https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/v2/projects.locations.workflows/start` | `POST /v2/{name=projects/*/locations/*/workflows/*}:start` Starts a previously created migration workflow. |

## REST Resource: [v2.projects.locations.workflows.subtasks](https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/v2/projects.locations.workflows.subtasks)

| Methods ||
|---|---|
| `https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/v2/projects.locations.workflows.subtasks/get` | `GET /v2/{name=projects/*/locations/*/workflows/*/subtasks/*}` Gets a previously created migration subtask. |
| `https://docs.cloud.google.com/bigquery/docs/reference/migration/rest/v2/projects.locations.workflows.subtasks/list` | `GET /v2/{parent=projects/*/locations/*/workflows/*}/subtasks` Lists previously created migration subtasks. |