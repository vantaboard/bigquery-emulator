# BigQuery Connection API

Allows users to manage BigQuery connections to external data sources.

- [REST Resource: v1beta1.projects.locations.connections](https://docs.cloud.google.com/bigquery/docs/reference/bigqueryconnection/rest#v1beta1.projects.locations.connections)
- [REST Resource: v1.projects.locations.connections](https://docs.cloud.google.com/bigquery/docs/reference/bigqueryconnection/rest#v1.projects.locations.connections)

## Service: bigqueryconnection.googleapis.com

To call this service, we recommend that you use the Google-provided [client libraries](https://cloud.google.com/apis/docs/client-libraries-explained). If your application needs to use your own libraries to call this service, use the following information when you make the API requests.

### Discovery document

A [Discovery Document](https://developers.google.com/discovery/v1/reference/apis) is a machine-readable specification for describing and consuming REST APIs. It is used to build client libraries, IDE plugins, and other tools that interact with Google APIs. One service may provide multiple discovery documents. This service provides the following discovery documents:

- <https://bigqueryconnection.googleapis.com/$discovery/rest?version=v1>
- <https://bigqueryconnection.googleapis.com/$discovery/rest?version=v1beta1>

### Service endpoint

A [service endpoint](https://cloud.google.com/apis/design/glossary#api_service_endpoint) is a base URL that specifies the network address of an API service. One service might have multiple service endpoints. This service has the following service endpoint and all URIs below are relative to this service endpoint:

- `https://bigqueryconnection.googleapis.com`

## REST Resource: [v1beta1.projects.locations.connections](https://docs.cloud.google.com/bigquery/docs/reference/bigqueryconnection/rest/v1beta1/projects.locations.connections)

| Methods ||
|---|---|
| `https://docs.cloud.google.com/bigquery/docs/reference/bigqueryconnection/rest/v1beta1/projects.locations.connections/create` | `POST /v1beta1/{parent=projects/*/locations/*}/connections` Creates a new connection. |
| `https://docs.cloud.google.com/bigquery/docs/reference/bigqueryconnection/rest/v1beta1/projects.locations.connections/delete` | `DELETE /v1beta1/{name=projects/*/locations/*/connections/*}` Deletes connection and associated credential. |
| `https://docs.cloud.google.com/bigquery/docs/reference/bigqueryconnection/rest/v1beta1/projects.locations.connections/get` | `GET /v1beta1/{name=projects/*/locations/*/connections/*}` Returns specified connection. |
| `https://docs.cloud.google.com/bigquery/docs/reference/bigqueryconnection/rest/v1beta1/projects.locations.connections/getIamPolicy` | `POST /v1beta1/{resource=projects/*/locations/*/connections/*}:getIamPolicy` Gets the access control policy for a resource. |
| `https://docs.cloud.google.com/bigquery/docs/reference/bigqueryconnection/rest/v1beta1/projects.locations.connections/list` | `GET /v1beta1/{parent=projects/*/locations/*}/connections` Returns a list of connections in the given project. |
| `https://docs.cloud.google.com/bigquery/docs/reference/bigqueryconnection/rest/v1beta1/projects.locations.connections/patch` | `PATCH /v1beta1/{name=projects/*/locations/*/connections/*}` Updates the specified connection. |
| `https://docs.cloud.google.com/bigquery/docs/reference/bigqueryconnection/rest/v1beta1/projects.locations.connections/setIamPolicy` | `POST /v1beta1/{resource=projects/*/locations/*/connections/*}:setIamPolicy` Sets the access control policy on the specified resource. |
| `https://docs.cloud.google.com/bigquery/docs/reference/bigqueryconnection/rest/v1beta1/projects.locations.connections/testIamPermissions` | `POST /v1beta1/{resource=projects/*/locations/*/connections/*}:testIamPermissions` Returns permissions that a caller has on the specified resource. |
| `https://docs.cloud.google.com/bigquery/docs/reference/bigqueryconnection/rest/v1beta1/projects.locations.connections/updateCredential` | `PATCH /v1beta1/{name=projects/*/locations/*/connections/*/credential}` Sets the credential for the specified connection. |

## REST Resource: [v1.projects.locations.connections](https://docs.cloud.google.com/bigquery/docs/reference/bigqueryconnection/rest/v1/projects.locations.connections)

| Methods ||
|---|---|
| `https://docs.cloud.google.com/bigquery/docs/reference/bigqueryconnection/rest/v1/projects.locations.connections/create` | `POST /v1/{parent=projects/*/locations/*}/connections` Creates a new connection. |
| `https://docs.cloud.google.com/bigquery/docs/reference/bigqueryconnection/rest/v1/projects.locations.connections/delete` | `DELETE /v1/{name=projects/*/locations/*/connections/*}` Deletes connection and associated credential. |
| `https://docs.cloud.google.com/bigquery/docs/reference/bigqueryconnection/rest/v1/projects.locations.connections/get` | `GET /v1/{name=projects/*/locations/*/connections/*}` Returns specified connection. |
| `https://docs.cloud.google.com/bigquery/docs/reference/bigqueryconnection/rest/v1/projects.locations.connections/getIamPolicy` | `POST /v1/{resource=projects/*/locations/*/connections/*}:getIamPolicy` Gets the access control policy for a resource. |
| `https://docs.cloud.google.com/bigquery/docs/reference/bigqueryconnection/rest/v1/projects.locations.connections/list` | `GET /v1/{parent=projects/*/locations/*}/connections` Returns a list of connections in the given project. |
| `https://docs.cloud.google.com/bigquery/docs/reference/bigqueryconnection/rest/v1/projects.locations.connections/patch` | `PATCH /v1/{name=projects/*/locations/*/connections/*}` Updates the specified connection. |
| `https://docs.cloud.google.com/bigquery/docs/reference/bigqueryconnection/rest/v1/projects.locations.connections/setIamPolicy` | `POST /v1/{resource=projects/*/locations/*/connections/*}:setIamPolicy` Sets the access control policy on the specified resource. |
| `https://docs.cloud.google.com/bigquery/docs/reference/bigqueryconnection/rest/v1/projects.locations.connections/testIamPermissions` | `POST /v1/{resource=projects/*/locations/*/connections/*}:testIamPermissions` Returns permissions that a caller has on the specified resource. |