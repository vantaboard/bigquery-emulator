# BigQuery API

A data platform for customers to create, manage, share and query data.

- [REST Resource: v2.datasets](https://docs.cloud.google.com/bigquery/docs/reference/rest#v2.datasets)
- [REST Resource: v2.jobs](https://docs.cloud.google.com/bigquery/docs/reference/rest#v2.jobs)
- [REST Resource: v2.models](https://docs.cloud.google.com/bigquery/docs/reference/rest#v2.models)
- [REST Resource: v2.projects](https://docs.cloud.google.com/bigquery/docs/reference/rest#v2.projects)
- [REST Resource: v2.routines](https://docs.cloud.google.com/bigquery/docs/reference/rest#v2.routines)
- [REST Resource: v2.rowAccessPolicies](https://docs.cloud.google.com/bigquery/docs/reference/rest#v2.rowAccessPolicies)
- [REST Resource: v2.tabledata](https://docs.cloud.google.com/bigquery/docs/reference/rest#v2.tabledata)
- [REST Resource: v2.tables](https://docs.cloud.google.com/bigquery/docs/reference/rest#v2.tables)

## Service: bigquery.googleapis.com

To call this service, we recommend that you use the Google-provided [client libraries](https://cloud.google.com/apis/docs/client-libraries-explained). If your application needs to use your own libraries to call this service, use the following information when you make the API requests.

### Discovery document

A [Discovery Document](https://developers.google.com/discovery/v1/reference/apis) is a machine-readable specification for describing and consuming REST APIs. It is used to build client libraries, IDE plugins, and other tools that interact with Google APIs. One service may provide multiple discovery documents. This service provides the following discovery document:

- <https://bigquery.googleapis.com/$discovery/rest?version=v2>

### Service endpoint

A [service endpoint](https://cloud.google.com/apis/design/glossary#api_service_endpoint) is a base URL that specifies the network address of an API service. One service might have multiple service endpoints. This service has the following service endpoint and all URIs below are relative to this service endpoint:

- `https://bigquery.googleapis.com`

## REST Resource: [v2.datasets](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets)

| Methods ||
|---|---|
| `https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets/delete` | `DELETE /bigquery/v2/projects/{projectId}/datasets/{datasetId}` Deletes the dataset specified by the datasetId value. |
| `https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets/get` | `GET /bigquery/v2/projects/{projectId}/datasets/{datasetId}` Returns the dataset specified by datasetID. |
| `https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets/insert` | `POST /bigquery/v2/projects/{projectId}/datasets` Creates a new empty dataset. |
| `https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets/list` | `GET /bigquery/v2/projects/{projectId}/datasets` Lists all datasets in the specified project to which the user has been granted the READER dataset role. |
| `https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets/patch` | `PATCH /bigquery/v2/projects/{projectId}/datasets/{datasetId}` Updates information in an existing dataset. |
| `https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets/undelete` | `POST /bigquery/v2/projects/{projectId}/datasets/{datasetId}:undelete` Undeletes a dataset which is within time travel window based on datasetId. |
| `https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets/update` | `PUT /bigquery/v2/projects/{projectId}/datasets/{datasetId}` Updates information in an existing dataset. |

## REST Resource: [v2.jobs](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs)

| Methods ||
|---|---|
| `https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs/cancel` | `POST /bigquery/v2/projects/{projectId}/jobs/{jobId}/cancel` Requests that a job be cancelled. |
| `https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs/delete` | `DELETE /bigquery/v2/projects/{projectId}/jobs/{jobId}/delete` Requests the deletion of the metadata of a job. |
| `https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs/get` | `GET /bigquery/v2/projects/{projectId}/jobs/{jobId}` Returns information about a specific job. |
| `https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs/getQueryResults` | `GET /bigquery/v2/projects/{projectId}/queries/{jobId}` RPC to get the results of a query job. |
| `https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs/insert` | `POST /bigquery/v2/projects/{projectId}/jobs` `POST /upload/bigquery/v2/projects/{projectId}/jobs` Starts a new asynchronous job. |
| `https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs/list` | `GET /bigquery/v2/projects/{projectId}/jobs` Lists all jobs that you started in the specified project. |
| `https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs/query` | `POST /bigquery/v2/projects/{projectId}/queries` Runs a BigQuery SQL query synchronously and returns query results if the query completes within a specified timeout. |

## REST Resource: [v2.models](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/models)

| Methods ||
|---|---|
| `https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/models/delete` | `DELETE /bigquery/v2/projects/{projectId}/datasets/{datasetId}/models/{modelId}` Deletes the model specified by modelId from the dataset. |
| `https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/models/get` | `GET /bigquery/v2/projects/{projectId}/datasets/{datasetId}/models/{modelId}` Gets the specified model resource by model ID. |
| `https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/models/list` | `GET /bigquery/v2/projects/{projectId}/datasets/{datasetId}/models` Lists all models in the specified dataset. |
| `https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/models/patch` | `PATCH /bigquery/v2/projects/{projectId}/datasets/{datasetId}/models/{modelId}` Patch specific fields in the specified model. |

## REST Resource: [v2.projects](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/projects)

| Methods ||
|---|---|
| `https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/projects/getServiceAccount` | `GET /bigquery/v2/projects/{projectId}/serviceAccount` RPC to get the service account for a project used for interactions with Google Cloud KMS |
| `https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/projects/list` | `GET /bigquery/v2/projects` RPC to list projects to which the user has been granted any project role. |

## REST Resource: [v2.routines](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/routines)

| Methods ||
|---|---|
| `https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/routines/delete` | `DELETE /bigquery/v2/projects/{projectId}/datasets/{datasetId}/routines/{routineId}` Deletes the routine specified by routineId from the dataset. |
| `https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/routines/get` | `GET /bigquery/v2/projects/{projectId}/datasets/{datasetId}/routines/{routineId}` Gets the specified routine resource by routine ID. |
| `https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/routines/getIamPolicy` | `POST /bigquery/v2/{resource=projects/*/datasets/*/routines/*}:getIamPolicy` Gets the access control policy for a resource. |
| `https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/routines/insert` | `POST /bigquery/v2/projects/{projectId}/datasets/{datasetId}/routines` Creates a new routine in the dataset. |
| `https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/routines/list` | `GET /bigquery/v2/projects/{projectId}/datasets/{datasetId}/routines` Lists all routines in the specified dataset. |
| `https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/routines/setIamPolicy` | `POST /bigquery/v2/{resource=projects/*/datasets/*/routines/*}:setIamPolicy` Sets the access control policy on the specified resource. |
| `https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/routines/testIamPermissions` | `POST /bigquery/v2/{resource=projects/*/datasets/*/routines/*}:testIamPermissions` Returns permissions that a caller has on the specified resource. |
| `https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/routines/update` | `PUT /bigquery/v2/projects/{projectId}/datasets/{datasetId}/routines/{routineId}` Updates information in an existing routine. |

## REST Resource: [v2.rowAccessPolicies](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/rowAccessPolicies)

| Methods ||
|---|---|
| `https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/rowAccessPolicies/batchDelete` | `POST /bigquery/v2/projects/{projectId}/datasets/{datasetId}/tables/{tableId}/rowAccessPolicies:batchDelete` Deletes provided row access policies. |
| `https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/rowAccessPolicies/delete` | `DELETE /bigquery/v2/projects/{projectId}/datasets/{datasetId}/tables/{tableId}/rowAccessPolicies/{policyId}` Deletes a row access policy. |
| `https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/rowAccessPolicies/get` | `GET /bigquery/v2/projects/{projectId}/datasets/{datasetId}/tables/{tableId}/rowAccessPolicies/{policyId}` Gets the specified row access policy by policy ID. |
| `https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/rowAccessPolicies/getIamPolicy` | `POST /bigquery/v2/{resource=projects/*/datasets/*/tables/*/rowAccessPolicies/*}:getIamPolicy` Gets the access control policy for a resource. |
| `https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/rowAccessPolicies/insert` | `POST /bigquery/v2/projects/{projectId}/datasets/{datasetId}/tables/{tableId}/rowAccessPolicies` Creates a row access policy. |
| `https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/rowAccessPolicies/list` | `GET /bigquery/v2/projects/{projectId}/datasets/{datasetId}/tables/{tableId}/rowAccessPolicies` Lists all row access policies on the specified table. |
| `https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/rowAccessPolicies/testIamPermissions` | `POST /bigquery/v2/{resource=projects/*/datasets/*/tables/*/rowAccessPolicies/*}:testIamPermissions` Returns permissions that a caller has on the specified resource. |
| `https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/rowAccessPolicies/update` | `PUT /bigquery/v2/projects/{projectId}/datasets/{datasetId}/tables/{tableId}/rowAccessPolicies/{policyId}` Updates a row access policy. |

## REST Resource: [v2.tabledata](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tabledata)

| Methods ||
|---|---|
| `https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tabledata/insertAll` | `POST /bigquery/v2/projects/{projectId}/datasets/{datasetId}/tables/{tableId}/insertAll` Streams data into BigQuery one record at a time without needing to run a load job. |
| `https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tabledata/list` | `GET /bigquery/v2/projects/{projectId}/datasets/{datasetId}/tables/{tableId}/data` List the content of a table in rows. |

## REST Resource: [v2.tables](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables)

| Methods ||
|---|---|
| `https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables/delete` | `DELETE /bigquery/v2/projects/{projectId}/datasets/{datasetId}/tables/{tableId}` Deletes the table specified by tableId from the dataset. |
| `https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables/get` | `GET /bigquery/v2/projects/{projectId}/datasets/{datasetId}/tables/{tableId}` Gets the specified table resource by table ID. |
| `https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables/getIamPolicy` | `POST /bigquery/v2/{resource=projects/*/datasets/*/tables/*}:getIamPolicy` Gets the access control policy for a resource. |
| `https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables/insert` | `POST /bigquery/v2/projects/{projectId}/datasets/{datasetId}/tables` Creates a new, empty table in the dataset. |
| `https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables/list` | `GET /bigquery/v2/projects/{projectId}/datasets/{datasetId}/tables` Lists all tables in the specified dataset. |
| `https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables/patch` | `PATCH /bigquery/v2/projects/{projectId}/datasets/{datasetId}/tables/{tableId}` Updates information in an existing table. |
| `https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables/setIamPolicy` | `POST /bigquery/v2/{resource=projects/*/datasets/*/tables/*}:setIamPolicy` Sets the access control policy on the specified resource. |
| `https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables/testIamPermissions` | `POST /bigquery/v2/{resource=projects/*/datasets/*/tables/*}:testIamPermissions` Returns permissions that a caller has on the specified resource. |
| `https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables/update` | `PUT /bigquery/v2/projects/{projectId}/datasets/{datasetId}/tables/{tableId}` Updates information in an existing table. |