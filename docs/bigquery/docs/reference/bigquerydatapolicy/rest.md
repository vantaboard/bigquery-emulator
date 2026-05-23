# BigQuery Data Policy API

Allows users to manage BigQuery data policies.

- [REST Resource: v2beta1.projects.locations.dataPolicies](https://docs.cloud.google.com/bigquery/docs/reference/bigquerydatapolicy/rest#v2beta1.projects.locations.dataPolicies)
- [REST Resource: v2.projects.locations.dataPolicies](https://docs.cloud.google.com/bigquery/docs/reference/bigquerydatapolicy/rest#v2.projects.locations.dataPolicies)
- [REST Resource: v1beta1.projects.locations.dataPolicies](https://docs.cloud.google.com/bigquery/docs/reference/bigquerydatapolicy/rest#v1beta1.projects.locations.dataPolicies)
- [REST Resource: v1.projects.locations.dataPolicies](https://docs.cloud.google.com/bigquery/docs/reference/bigquerydatapolicy/rest#v1.projects.locations.dataPolicies)

## Service: bigquerydatapolicy.googleapis.com

To call this service, we recommend that you use the Google-provided [client libraries](https://cloud.google.com/apis/docs/client-libraries-explained). If your application needs to use your own libraries to call this service, use the following information when you make the API requests.

### Discovery document

A [Discovery Document](https://developers.google.com/discovery/v1/reference/apis) is a machine-readable specification for describing and consuming REST APIs. It is used to build client libraries, IDE plugins, and other tools that interact with Google APIs. One service may provide multiple discovery documents. This service provides the following discovery documents:

- <https://bigquerydatapolicy.googleapis.com/$discovery/rest?version=v2>
- <https://bigquerydatapolicy.googleapis.com/$discovery/rest?version=v2beta1>
- <https://bigquerydatapolicy.googleapis.com/$discovery/rest?version=v1>
- <https://bigquerydatapolicy.googleapis.com/$discovery/rest?version=v1beta1>

### Service endpoint

A [service endpoint](https://cloud.google.com/apis/design/glossary#api_service_endpoint) is a base URL that specifies the network address of an API service. One service might have multiple service endpoints. This service has the following service endpoint and all URIs below are relative to this service endpoint:

- `https://bigquerydatapolicy.googleapis.com`

## REST Resource: [v2beta1.projects.locations.dataPolicies](https://docs.cloud.google.com/bigquery/docs/reference/bigquerydatapolicy/rest/v2beta1/projects.locations.dataPolicies)

| Methods ||
|---|---|
| `https://docs.cloud.google.com/bigquery/docs/reference/bigquerydatapolicy/rest/v2beta1/projects.locations.dataPolicies/addGrantees` | `POST /v2beta1/{dataPolicy=projects/*/locations/*/dataPolicies/*}:addGrantees` Adds new grantees to a data policy. |
| `https://docs.cloud.google.com/bigquery/docs/reference/bigquerydatapolicy/rest/v2beta1/projects.locations.dataPolicies/create` | `POST /v2beta1/{parent=projects/*/locations/*}/dataPolicies` Creates a new data policy under a project with the given `data_policy_id` (used as the display name), and data policy type. |
| `https://docs.cloud.google.com/bigquery/docs/reference/bigquerydatapolicy/rest/v2beta1/projects.locations.dataPolicies/delete` | `DELETE /v2beta1/{name=projects/*/locations/*/dataPolicies/*}` Deletes the data policy specified by its resource name. |
| `https://docs.cloud.google.com/bigquery/docs/reference/bigquerydatapolicy/rest/v2beta1/projects.locations.dataPolicies/get` | `GET /v2beta1/{name=projects/*/locations/*/dataPolicies/*}` Gets the data policy specified by its resource name. |
| `https://docs.cloud.google.com/bigquery/docs/reference/bigquerydatapolicy/rest/v2beta1/projects.locations.dataPolicies/getIamPolicy` | `POST /v2beta1/{resource=projects/*/locations/*/dataPolicies/*}:getIamPolicy` Gets the IAM policy for the specified data policy. |
| `https://docs.cloud.google.com/bigquery/docs/reference/bigquerydatapolicy/rest/v2beta1/projects.locations.dataPolicies/list` | `GET /v2beta1/{parent=projects/*/locations/*}/dataPolicies` List all of the data policies in the specified parent project. |
| `https://docs.cloud.google.com/bigquery/docs/reference/bigquerydatapolicy/rest/v2beta1/projects.locations.dataPolicies/patch` | `PATCH /v2beta1/{dataPolicy.name=projects/*/locations/*/dataPolicies/*}` Updates the metadata for an existing data policy. |
| `https://docs.cloud.google.com/bigquery/docs/reference/bigquerydatapolicy/rest/v2beta1/projects.locations.dataPolicies/removeGrantees` | `POST /v2beta1/{dataPolicy=projects/*/locations/*/dataPolicies/*}:removeGrantees` Removes grantees from a data policy. |
| `https://docs.cloud.google.com/bigquery/docs/reference/bigquerydatapolicy/rest/v2beta1/projects.locations.dataPolicies/setIamPolicy` | `POST /v2beta1/{resource=projects/*/locations/*/dataPolicies/*}:setIamPolicy` Sets the IAM policy for the specified data policy. |
| `https://docs.cloud.google.com/bigquery/docs/reference/bigquerydatapolicy/rest/v2beta1/projects.locations.dataPolicies/testIamPermissions` | `POST /v2beta1/{resource=projects/*/locations/*/dataPolicies/*}:testIamPermissions` Returns the caller's permission on the specified data policy resource. |

## REST Resource: [v2.projects.locations.dataPolicies](https://docs.cloud.google.com/bigquery/docs/reference/bigquerydatapolicy/rest/v2/projects.locations.dataPolicies)

| Methods ||
|---|---|
| `https://docs.cloud.google.com/bigquery/docs/reference/bigquerydatapolicy/rest/v2/projects.locations.dataPolicies/addGrantees` | `POST /v2/{dataPolicy=projects/*/locations/*/dataPolicies/*}:addGrantees` Adds new grantees to a data policy. |
| `https://docs.cloud.google.com/bigquery/docs/reference/bigquerydatapolicy/rest/v2/projects.locations.dataPolicies/create` | `POST /v2/{parent=projects/*/locations/*}/dataPolicies` Creates a new data policy under a project with the given `data_policy_id` (used as the display name), and data policy type. |
| `https://docs.cloud.google.com/bigquery/docs/reference/bigquerydatapolicy/rest/v2/projects.locations.dataPolicies/delete` | `DELETE /v2/{name=projects/*/locations/*/dataPolicies/*}` Deletes the data policy specified by its resource name. |
| `https://docs.cloud.google.com/bigquery/docs/reference/bigquerydatapolicy/rest/v2/projects.locations.dataPolicies/get` | `GET /v2/{name=projects/*/locations/*/dataPolicies/*}` Gets the data policy specified by its resource name. |
| `https://docs.cloud.google.com/bigquery/docs/reference/bigquerydatapolicy/rest/v2/projects.locations.dataPolicies/getIamPolicy` | `POST /v2/{resource=projects/*/locations/*/dataPolicies/*}:getIamPolicy` Gets the IAM policy for the specified data policy. |
| `https://docs.cloud.google.com/bigquery/docs/reference/bigquerydatapolicy/rest/v2/projects.locations.dataPolicies/list` | `GET /v2/{parent=projects/*/locations/*}/dataPolicies` List all of the data policies in the specified parent project. |
| `https://docs.cloud.google.com/bigquery/docs/reference/bigquerydatapolicy/rest/v2/projects.locations.dataPolicies/patch` | `PATCH /v2/{dataPolicy.name=projects/*/locations/*/dataPolicies/*}` Updates the metadata for an existing data policy. |
| `https://docs.cloud.google.com/bigquery/docs/reference/bigquerydatapolicy/rest/v2/projects.locations.dataPolicies/removeGrantees` | `POST /v2/{dataPolicy=projects/*/locations/*/dataPolicies/*}:removeGrantees` Removes grantees from a data policy. |
| `https://docs.cloud.google.com/bigquery/docs/reference/bigquerydatapolicy/rest/v2/projects.locations.dataPolicies/setIamPolicy` | `POST /v2/{resource=projects/*/locations/*/dataPolicies/*}:setIamPolicy` Sets the IAM policy for the specified data policy. |
| `https://docs.cloud.google.com/bigquery/docs/reference/bigquerydatapolicy/rest/v2/projects.locations.dataPolicies/testIamPermissions` | `POST /v2/{resource=projects/*/locations/*/dataPolicies/*}:testIamPermissions` Returns the caller's permission on the specified data policy resource. |

## REST Resource: [v1beta1.projects.locations.dataPolicies](https://docs.cloud.google.com/bigquery/docs/reference/bigquerydatapolicy/rest/v1beta1/projects.locations.dataPolicies)

| Methods ||
|---|---|
| `https://docs.cloud.google.com/bigquery/docs/reference/bigquerydatapolicy/rest/v1beta1/projects.locations.dataPolicies/create` | `POST /v1beta1/{parent=projects/*/locations/*}/dataPolicies` Creates a new data policy under a project with the given `dataPolicyId` (used as the display name), policy tag, and data policy type. |
| `https://docs.cloud.google.com/bigquery/docs/reference/bigquerydatapolicy/rest/v1beta1/projects.locations.dataPolicies/delete` | `DELETE /v1beta1/{name=projects/*/locations/*/dataPolicies/*}` Deletes the data policy specified by its resource name. |
| `https://docs.cloud.google.com/bigquery/docs/reference/bigquerydatapolicy/rest/v1beta1/projects.locations.dataPolicies/get` | `GET /v1beta1/{name=projects/*/locations/*/dataPolicies/*}` Gets the data policy specified by its resource name. |
| `https://docs.cloud.google.com/bigquery/docs/reference/bigquerydatapolicy/rest/v1beta1/projects.locations.dataPolicies/getIamPolicy` | `POST /v1beta1/{resource=projects/*/locations/*/dataPolicies/*}:getIamPolicy` Gets the IAM policy for the specified data policy. |
| `https://docs.cloud.google.com/bigquery/docs/reference/bigquerydatapolicy/rest/v1beta1/projects.locations.dataPolicies/list` | `GET /v1beta1/{parent=projects/*/locations/*}/dataPolicies` List all of the data policies in the specified parent project. |
| `https://docs.cloud.google.com/bigquery/docs/reference/bigquerydatapolicy/rest/v1beta1/projects.locations.dataPolicies/patch` | `PATCH /v1beta1/{dataPolicy.name=projects/*/locations/*/dataPolicies/*}` Updates the metadata for an existing data policy. |
| `https://docs.cloud.google.com/bigquery/docs/reference/bigquerydatapolicy/rest/v1beta1/projects.locations.dataPolicies/setIamPolicy` | `POST /v1beta1/{resource=projects/*/locations/*/dataPolicies/*}:setIamPolicy` Sets the IAM policy for the specified data policy. |
| `https://docs.cloud.google.com/bigquery/docs/reference/bigquerydatapolicy/rest/v1beta1/projects.locations.dataPolicies/testIamPermissions` | `POST /v1beta1/{resource=projects/*/locations/*/dataPolicies/*}:testIamPermissions` Returns the caller's permission on the specified data policy resource. |

## REST Resource: [v1.projects.locations.dataPolicies](https://docs.cloud.google.com/bigquery/docs/reference/bigquerydatapolicy/rest/v1/projects.locations.dataPolicies)

| Methods ||
|---|---|
| `https://docs.cloud.google.com/bigquery/docs/reference/bigquerydatapolicy/rest/v1/projects.locations.dataPolicies/create` | `POST /v1/{parent=projects/*/locations/*}/dataPolicies` Creates a new data policy under a project with the given `dataPolicyId` (used as the display name), policy tag, and data policy type. |
| `https://docs.cloud.google.com/bigquery/docs/reference/bigquerydatapolicy/rest/v1/projects.locations.dataPolicies/delete` | `DELETE /v1/{name=projects/*/locations/*/dataPolicies/*}` Deletes the data policy specified by its resource name. |
| `https://docs.cloud.google.com/bigquery/docs/reference/bigquerydatapolicy/rest/v1/projects.locations.dataPolicies/get` | `GET /v1/{name=projects/*/locations/*/dataPolicies/*}` Gets the data policy specified by its resource name. |
| `https://docs.cloud.google.com/bigquery/docs/reference/bigquerydatapolicy/rest/v1/projects.locations.dataPolicies/getIamPolicy` | `POST /v1/{resource=projects/*/locations/*/dataPolicies/*}:getIamPolicy` Gets the IAM policy for the specified data policy. |
| `https://docs.cloud.google.com/bigquery/docs/reference/bigquerydatapolicy/rest/v1/projects.locations.dataPolicies/list` | `GET /v1/{parent=projects/*/locations/*}/dataPolicies` List all of the data policies in the specified parent project. |
| `https://docs.cloud.google.com/bigquery/docs/reference/bigquerydatapolicy/rest/v1/projects.locations.dataPolicies/patch` | `PATCH /v1/{dataPolicy.name=projects/*/locations/*/dataPolicies/*}` Updates the metadata for an existing data policy. |
| `https://docs.cloud.google.com/bigquery/docs/reference/bigquerydatapolicy/rest/v1/projects.locations.dataPolicies/rename` | `POST /v1/{name=projects/*/locations/*/dataPolicies/*}:rename` Renames the id (display name) of the specified data policy. |
| `https://docs.cloud.google.com/bigquery/docs/reference/bigquerydatapolicy/rest/v1/projects.locations.dataPolicies/setIamPolicy` | `POST /v1/{resource=projects/*/locations/*/dataPolicies/*}:setIamPolicy` Sets the IAM policy for the specified data policy. |
| `https://docs.cloud.google.com/bigquery/docs/reference/bigquerydatapolicy/rest/v1/projects.locations.dataPolicies/testIamPermissions` | `POST /v1/{resource=projects/*/locations/*/dataPolicies/*}:testIamPermissions` Returns the caller's permission on the specified data policy resource. |