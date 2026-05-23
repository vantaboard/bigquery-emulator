Schedule queries or transfer external data from SaaS applications to Google BigQuery on a regular basis.

- [REST Resource: v1.projects](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest#v1.projects)
- [REST Resource: v1.projects.dataSources](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest#v1.projects.dataSources)
- [REST Resource: v1.projects.locations](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest#v1.projects.locations)
- [REST Resource: v1.projects.locations.dataSources](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest#v1.projects.locations.dataSources)
- [REST Resource: v1.projects.locations.transferConfigs](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest#v1.projects.locations.transferConfigs)
- [REST Resource: v1.projects.locations.transferConfigs.runs](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest#v1.projects.locations.transferConfigs.runs)
- [REST Resource: v1.projects.locations.transferConfigs.runs.transferLogs](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest#v1.projects.locations.transferConfigs.runs.transferLogs)
- [REST Resource: v1.projects.locations.transferConfigs.transferResources](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest#v1.projects.locations.transferConfigs.transferResources)
- [REST Resource: v1.projects.transferConfigs](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest#v1.projects.transferConfigs)
- [REST Resource: v1.projects.transferConfigs.runs](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest#v1.projects.transferConfigs.runs)
- [REST Resource: v1.projects.transferConfigs.runs.transferLogs](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest#v1.projects.transferConfigs.runs.transferLogs)
- [REST Resource: v1.projects.transferConfigs.transferResources](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest#v1.projects.transferConfigs.transferResources)

## Service: bigquerydatatransfer.googleapis.com

To call this service, we recommend that you use the Google-provided [client libraries](https://cloud.google.com/apis/docs/client-libraries-explained). If your application needs to use your own libraries to call this service, use the following information when you make the API requests.

### Discovery document

A [Discovery Document](https://developers.google.com/discovery/v1/reference/apis) is a machine-readable specification for describing and consuming REST APIs. It is used to build client libraries, IDE plugins, and other tools that interact with Google APIs. One service may provide multiple discovery documents. This service provides the following discovery document:

- <https://bigquerydatatransfer.googleapis.com/$discovery/rest?version=v1>

### Service endpoint

A [service endpoint](https://cloud.google.com/apis/design/glossary#api_service_endpoint) is a base URL that specifies the network address of an API service. One service might have multiple service endpoints. This service has the following service endpoint and all URIs below are relative to this service endpoint:

- `https://bigquerydatatransfer.googleapis.com`

### Regional service endpoint

A regional service endpoint is a base URL that specifies the network address of an API service in a single region. A service that is available in multiple regions might have multiple regional endpoints. Select a location to see its regional service endpoint for this service.
<button value="global" default="">global</button> <button value="asia-south1">asia-south1</button> <button value="asia-south2">asia-south2</button> <button value="europe-west1">europe-west1</button> <button value="europe-west2">europe-west2</button> <button value="europe-west3">europe-west3</button> <button value="europe-west4">europe-west4</button> <button value="europe-west6">europe-west6</button> <button value="europe-west8">europe-west8</button> <button value="europe-west9">europe-west9</button> <button value="me-central2">me-central2</button> <button value="northamerica-northeast1">northamerica-northeast1</button> <button value="northamerica-northeast2">northamerica-northeast2</button> <button value="us-central1">us-central1</button> <button value="us-central2">us-central2</button> <button value="us-east1">us-east1</button> <button value="us-east4">us-east4</button> <button value="us-east5">us-east5</button> <button value="us-east7">us-east7</button> <button value="us-south1">us-south1</button> <button value="us-west1">us-west1</button> <button value="us-west2">us-west2</button> <button value="us-west3">us-west3</button> <button value="us-west4">us-west4</button> <button value="us-west8">us-west8</button>   
- `https://bigquerydatatransfer.googleapis.com`

<br />

## REST Resource: [v1.projects](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects)

| Methods ||
|---|---|
| `https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects/enrollDataSources` | `POST /v1/{name=projects/*}:enrollDataSources` Enroll data sources in a user project. |

## REST Resource: [v1.projects.dataSources](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.dataSources)

| Methods ||
|---|---|
| `https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.dataSources/checkValidCreds` | `POST /v1/{name=projects/*/dataSources/*}:checkValidCreds` Returns true if valid credentials exist for the given data source and requesting user. |
| `https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.dataSources/get` | `GET /v1/{name=projects/*/dataSources/*}` Retrieves a supported data source and returns its settings. |
| `https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.dataSources/list` | `GET /v1/{parent=projects/*}/dataSources` Lists supported data sources and returns their settings. |

## REST Resource: [v1.projects.locations](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations)

| Methods ||
|---|---|
| `https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations/enrollDataSources` | `POST /v1/{name=projects/*/locations/*}:enrollDataSources` Enroll data sources in a user project. |
| `https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations/get` | `GET /v1/{name=projects/*/locations/*}` Gets information about a location. |
| `https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations/list` | `GET /v1/{name=projects/*}/locations` Lists information about the supported locations for this service. |
| `https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations/unenrollDataSources` | `POST /v1/{name=projects/*/locations/*}:unenrollDataSources` Unenroll data sources in a user project. |

## REST Resource: [v1.projects.locations.dataSources](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.dataSources)

| Methods ||
|---|---|
| `https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.dataSources/checkValidCreds` | `POST /v1/{name=projects/*/locations/*/dataSources/*}:checkValidCreds` Returns true if valid credentials exist for the given data source and requesting user. |
| `https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.dataSources/get` | `GET /v1/{name=projects/*/locations/*/dataSources/*}` Retrieves a supported data source and returns its settings. |
| `https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.dataSources/list` | `GET /v1/{parent=projects/*/locations/*}/dataSources` Lists supported data sources and returns their settings. |

## REST Resource: [v1.projects.locations.transferConfigs](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs)

| Methods ||
|---|---|
| `https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs/create` | `POST /v1/{parent=projects/*/locations/*}/transferConfigs` Creates a new data transfer configuration. |
| `https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs/delete` | `DELETE /v1/{name=projects/*/locations/*/transferConfigs/*}` Deletes a data transfer configuration, including any associated transfer runs and logs. |
| `https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs/get` | `GET /v1/{name=projects/*/locations/*/transferConfigs/*}` Returns information about a data transfer config. |
| `https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs/list` | `GET /v1/{parent=projects/*/locations/*}/transferConfigs` Returns information about all transfer configs owned by a project in the specified location. |
| `https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs/patch` | `PATCH /v1/{transferConfig.name=projects/*/locations/*/transferConfigs/*}` Updates a data transfer configuration. |
| `https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs/scheduleRuns (deprecated)` | `POST /v1/{parent=projects/*/locations/*/transferConfigs/*}:scheduleRuns` Creates transfer runs for a time range \[start_time, end_time\]. |
| `https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs/startManualRuns` | `POST /v1/{parent=projects/*/locations/*/transferConfigs/*}:startManualRuns` Manually initiates transfer runs. |

## REST Resource: [v1.projects.locations.transferConfigs.runs](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs.runs)

| Methods ||
|---|---|
| `https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs.runs/delete` | `DELETE /v1/{name=projects/*/locations/*/transferConfigs/*/runs/*}` Deletes the specified transfer run. |
| `https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs.runs/get` | `GET /v1/{name=projects/*/locations/*/transferConfigs/*/runs/*}` Returns information about the particular transfer run. |
| `https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs.runs/list` | `GET /v1/{parent=projects/*/locations/*/transferConfigs/*}/runs` Returns information about running and completed transfer runs. |

## REST Resource: [v1.projects.locations.transferConfigs.runs.transferLogs](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs.runs.transferLogs)

| Methods ||
|---|---|
| `https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs.runs.transferLogs/list` | `GET /v1/{parent=projects/*/locations/*/transferConfigs/*/runs/*}/transferLogs` Returns log messages for the transfer run. |

## REST Resource: [v1.projects.locations.transferConfigs.transferResources](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs.transferResources)

| Methods ||
|---|---|
| `https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs.transferResources/get` | `GET /v1/{name=projects/*/locations/*/transferConfigs/*/transferResources/*}` Returns a transfer resource. |
| `https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.locations.transferConfigs.transferResources/list` | `GET /v1/{parent=projects/*/locations/*/transferConfigs/*}/transferResources` Returns information about transfer resources. |

## REST Resource: [v1.projects.transferConfigs](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.transferConfigs)

| Methods ||
|---|---|
| `https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.transferConfigs/create` | `POST /v1/{parent=projects/*}/transferConfigs` Creates a new data transfer configuration. |
| `https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.transferConfigs/delete` | `DELETE /v1/{name=projects/*/transferConfigs/*}` Deletes a data transfer configuration, including any associated transfer runs and logs. |
| `https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.transferConfigs/get` | `GET /v1/{name=projects/*/transferConfigs/*}` Returns information about a data transfer config. |
| `https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.transferConfigs/list` | `GET /v1/{parent=projects/*}/transferConfigs` Returns information about all transfer configs owned by a project in the specified location. |
| `https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.transferConfigs/patch` | `PATCH /v1/{transferConfig.name=projects/*/transferConfigs/*}` Updates a data transfer configuration. |
| `https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.transferConfigs/scheduleRuns (deprecated)` | `POST /v1/{parent=projects/*/transferConfigs/*}:scheduleRuns` Creates transfer runs for a time range \[start_time, end_time\]. |
| `https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.transferConfigs/startManualRuns` | `POST /v1/{parent=projects/*/transferConfigs/*}:startManualRuns` Manually initiates transfer runs. |

## REST Resource: [v1.projects.transferConfigs.runs](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.transferConfigs.runs)

| Methods ||
|---|---|
| `https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.transferConfigs.runs/delete` | `DELETE /v1/{name=projects/*/transferConfigs/*/runs/*}` Deletes the specified transfer run. |
| `https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.transferConfigs.runs/get` | `GET /v1/{name=projects/*/transferConfigs/*/runs/*}` Returns information about the particular transfer run. |
| `https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.transferConfigs.runs/list` | `GET /v1/{parent=projects/*/transferConfigs/*}/runs` Returns information about running and completed transfer runs. |

## REST Resource: [v1.projects.transferConfigs.runs.transferLogs](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.transferConfigs.runs.transferLogs)

| Methods ||
|---|---|
| `https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.transferConfigs.runs.transferLogs/list` | `GET /v1/{parent=projects/*/transferConfigs/*/runs/*}/transferLogs` Returns log messages for the transfer run. |

## REST Resource: [v1.projects.transferConfigs.transferResources](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.transferConfigs.transferResources)

| Methods ||
|---|---|
| `https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.transferConfigs.transferResources/get` | `GET /v1/{name=projects/*/transferConfigs/*/transferResources/*}` Returns a transfer resource. |
| `https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest/v1/projects.transferConfigs.transferResources/list` | `GET /v1/{parent=projects/*/transferConfigs/*}/transferResources` Returns information about transfer resources. |