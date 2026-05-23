# Analytics Hub API

Exchange data and analytics assets securely and efficiently.

- [REST Resource: v1beta1.organizations.locations.dataExchanges](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest#v1beta1.organizations.locations.dataExchanges)
- [REST Resource: v1beta1.projects.locations.dataExchanges](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest#v1beta1.projects.locations.dataExchanges)
- [REST Resource: v1beta1.projects.locations.dataExchanges.listings](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest#v1beta1.projects.locations.dataExchanges.listings)
- [REST Resource: v1.organizations.locations.dataExchanges](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest#v1.organizations.locations.dataExchanges)
- [REST Resource: v1.projects.locations.dataExchanges](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest#v1.projects.locations.dataExchanges)
- [REST Resource: v1.projects.locations.dataExchanges.listings](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest#v1.projects.locations.dataExchanges.listings)
- [REST Resource: v1.projects.locations.dataExchanges.queryTemplates](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest#v1.projects.locations.dataExchanges.queryTemplates)
- [REST Resource: v1.projects.locations.subscriptions](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest#v1.projects.locations.subscriptions)

## Service: analyticshub.googleapis.com

To call this service, we recommend that you use the Google-provided [client libraries](https://cloud.google.com/apis/docs/client-libraries-explained). If your application needs to use your own libraries to call this service, use the following information when you make the API requests.

### Discovery document

A [Discovery Document](https://developers.google.com/discovery/v1/reference/apis) is a machine-readable specification for describing and consuming REST APIs. It is used to build client libraries, IDE plugins, and other tools that interact with Google APIs. One service may provide multiple discovery documents. This service provides the following discovery documents:

- <https://analyticshub.googleapis.com/$discovery/rest?version=v1>
- <https://analyticshub.googleapis.com/$discovery/rest?version=v1beta1>

### Service endpoint

A [service endpoint](https://cloud.google.com/apis/design/glossary#api_service_endpoint) is a base URL that specifies the network address of an API service. One service might have multiple service endpoints. This service has the following service endpoint and all URIs below are relative to this service endpoint:

- `https://analyticshub.googleapis.com`

## REST Resource: [v1beta1.organizations.locations.dataExchanges](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1beta1/organizations.locations.dataExchanges)

| Methods ||
|---|---|
| `https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1beta1/organizations.locations.dataExchanges/list` | `GET /v1beta1/{organization=organizations/*/locations/*}/dataExchanges` Lists all data exchanges from projects in a given organization and location. |

## REST Resource: [v1beta1.projects.locations.dataExchanges](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1beta1/projects.locations.dataExchanges)

| Methods ||
|---|---|
| `https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1beta1/projects.locations.dataExchanges/create` | `POST /v1beta1/{parent=projects/*/locations/*}/dataExchanges` Creates a new data exchange. |
| `https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1beta1/projects.locations.dataExchanges/delete` | `DELETE /v1beta1/{name=projects/*/locations/*/dataExchanges/*}` Deletes an existing data exchange. |
| `https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1beta1/projects.locations.dataExchanges/get` | `GET /v1beta1/{name=projects/*/locations/*/dataExchanges/*}` Gets the details of a data exchange. |
| `https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1beta1/projects.locations.dataExchanges/getIamPolicy` | `POST /v1beta1/{resource=projects/*/locations/*/dataExchanges/*}:getIamPolicy` Gets the IAM policy. |
| `https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1beta1/projects.locations.dataExchanges/list` | `GET /v1beta1/{parent=projects/*/locations/*}/dataExchanges` Lists all data exchanges in a given project and location. |
| `https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1beta1/projects.locations.dataExchanges/patch` | `PATCH /v1beta1/{dataExchange.name=projects/*/locations/*/dataExchanges/*}` Updates an existing data exchange. |
| `https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1beta1/projects.locations.dataExchanges/setIamPolicy` | `POST /v1beta1/{resource=projects/*/locations/*/dataExchanges/*}:setIamPolicy` Sets the IAM policy. |
| `https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1beta1/projects.locations.dataExchanges/testIamPermissions` | `POST /v1beta1/{resource=projects/*/locations/*/dataExchanges/*}:testIamPermissions` Returns the permissions that a caller has. |

## REST Resource: [v1beta1.projects.locations.dataExchanges.listings](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1beta1/projects.locations.dataExchanges.listings)

| Methods ||
|---|---|
| `https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1beta1/projects.locations.dataExchanges.listings/create` | `POST /v1beta1/{parent=projects/*/locations/*/dataExchanges/*}/listings` Creates a new listing. |
| `https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1beta1/projects.locations.dataExchanges.listings/delete` | `DELETE /v1beta1/{name=projects/*/locations/*/dataExchanges/*/listings/*}` Deletes a listing. |
| `https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1beta1/projects.locations.dataExchanges.listings/get` | `GET /v1beta1/{name=projects/*/locations/*/dataExchanges/*/listings/*}` Gets the details of a listing. |
| `https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1beta1/projects.locations.dataExchanges.listings/getIamPolicy` | `POST /v1beta1/{resource=projects/*/locations/*/dataExchanges/*/listings/*}:getIamPolicy` Gets the IAM policy. |
| `https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1beta1/projects.locations.dataExchanges.listings/list` | `GET /v1beta1/{parent=projects/*/locations/*/dataExchanges/*}/listings` Lists all listings in a given project and location. |
| `https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1beta1/projects.locations.dataExchanges.listings/patch` | `PATCH /v1beta1/{listing.name=projects/*/locations/*/dataExchanges/*/listings/*}` Updates an existing listing. |
| `https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1beta1/projects.locations.dataExchanges.listings/setIamPolicy` | `POST /v1beta1/{resource=projects/*/locations/*/dataExchanges/*/listings/*}:setIamPolicy` Sets the IAM policy. |
| `https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1beta1/projects.locations.dataExchanges.listings/subscribe` | `POST /v1beta1/{name=projects/*/locations/*/dataExchanges/*/listings/*}:subscribe` Subscribes to a listing. |
| `https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1beta1/projects.locations.dataExchanges.listings/testIamPermissions` | `POST /v1beta1/{resource=projects/*/locations/*/dataExchanges/*/listings/*}:testIamPermissions` Returns the permissions that a caller has. |

## REST Resource: [v1.organizations.locations.dataExchanges](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/organizations.locations.dataExchanges)

| Methods ||
|---|---|
| `https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/organizations.locations.dataExchanges/list` | `GET /v1/{organization=organizations/*/locations/*}/dataExchanges` Lists all data exchanges from projects in a given organization and location. |

## REST Resource: [v1.projects.locations.dataExchanges](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges)

| Methods ||
|---|---|
| `https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges/create` | `POST /v1/{parent=projects/*/locations/*}/dataExchanges` Creates a new data exchange. |
| `https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges/delete` | `DELETE /v1/{name=projects/*/locations/*/dataExchanges/*}` Deletes an existing data exchange. |
| `https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges/get` | `GET /v1/{name=projects/*/locations/*/dataExchanges/*}` Gets the details of a data exchange. |
| `https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges/getIamPolicy` | `POST /v1/{resource=projects/*/locations/*/dataExchanges/*}:getIamPolicy` Gets the IAM policy. |
| `https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges/list` | `GET /v1/{parent=projects/*/locations/*}/dataExchanges` Lists all data exchanges in a given project and location. |
| `https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges/listSubscriptions` | `GET /v1/{resource=projects/*/locations/*/dataExchanges/*}:listSubscriptions` Lists all subscriptions on a given Data Exchange or Listing. |
| `https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges/patch` | `PATCH /v1/{dataExchange.name=projects/*/locations/*/dataExchanges/*}` Updates an existing data exchange. |
| `https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges/setIamPolicy` | `POST /v1/{resource=projects/*/locations/*/dataExchanges/*}:setIamPolicy` Sets the IAM policy. |
| `https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges/subscribe` | `POST /v1/{name=projects/*/locations/*/dataExchanges/*}:subscribe` Creates a Subscription to a Data Clean Room. |
| `https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges/testIamPermissions` | `POST /v1/{resource=projects/*/locations/*/dataExchanges/*}:testIamPermissions` Returns the permissions that a caller has. |

## REST Resource: [v1.projects.locations.dataExchanges.listings](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.listings)

| Methods ||
|---|---|
| `https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.listings/create` | `POST /v1/{parent=projects/*/locations/*/dataExchanges/*}/listings` Creates a new listing. |
| `https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.listings/delete` | `DELETE /v1/{name=projects/*/locations/*/dataExchanges/*/listings/*}` Deletes a listing. |
| `https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.listings/get` | `GET /v1/{name=projects/*/locations/*/dataExchanges/*/listings/*}` Gets the details of a listing. |
| `https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.listings/getIamPolicy` | `POST /v1/{resource=projects/*/locations/*/dataExchanges/*/listings/*}:getIamPolicy` Gets the IAM policy. |
| `https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.listings/list` | `GET /v1/{parent=projects/*/locations/*/dataExchanges/*}/listings` Lists all listings in a given project and location. |
| `https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.listings/listSubscriptions` | `GET /v1/{resource=projects/*/locations/*/dataExchanges/*/listings/*}:listSubscriptions` Lists all subscriptions on a given Data Exchange or Listing. |
| `https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.listings/patch` | `PATCH /v1/{listing.name=projects/*/locations/*/dataExchanges/*/listings/*}` Updates an existing listing. |
| `https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.listings/setIamPolicy` | `POST /v1/{resource=projects/*/locations/*/dataExchanges/*/listings/*}:setIamPolicy` Sets the IAM policy. |
| `https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.listings/subscribe` | `POST /v1/{name=projects/*/locations/*/dataExchanges/*/listings/*}:subscribe` Subscribes to a listing. |
| `https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.listings/testIamPermissions` | `POST /v1/{resource=projects/*/locations/*/dataExchanges/*/listings/*}:testIamPermissions` Returns the permissions that a caller has. |

## REST Resource: [v1.projects.locations.dataExchanges.queryTemplates](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.queryTemplates)

| Methods ||
|---|---|
| `https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.queryTemplates/approve` | `POST /v1/{name=projects/*/locations/*/dataExchanges/*/queryTemplates/*}:approve` Approves a query template. |
| `https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.queryTemplates/create` | `POST /v1/{parent=projects/*/locations/*/dataExchanges/*}/queryTemplates` Creates a new QueryTemplate |
| `https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.queryTemplates/delete` | `DELETE /v1/{name=projects/*/locations/*/dataExchanges/*/queryTemplates/*}` Deletes a query template. |
| `https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.queryTemplates/get` | `GET /v1/{name=projects/*/locations/*/dataExchanges/*/queryTemplates/*}` Gets a QueryTemplate |
| `https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.queryTemplates/list` | `GET /v1/{parent=projects/*/locations/*/dataExchanges/*}/queryTemplates` Lists all QueryTemplates in a given project and location. |
| `https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.queryTemplates/patch` | `PATCH /v1/{queryTemplate.name=projects/*/locations/*/dataExchanges/*/queryTemplates/*}` Updates an existing QueryTemplate |
| `https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.dataExchanges.queryTemplates/submit` | `POST /v1/{name=projects/*/locations/*/dataExchanges/*/queryTemplates/*}:submit` Submits a query template for approval. |

## REST Resource: [v1.projects.locations.subscriptions](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.subscriptions)

| Methods ||
|---|---|
| `https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.subscriptions/delete` | `DELETE /v1/{name=projects/*/locations/*/subscriptions/*}` Deletes a subscription. |
| `https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.subscriptions/get` | `GET /v1/{name=projects/*/locations/*/subscriptions/*}` Gets the details of a Subscription. |
| `https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.subscriptions/getIamPolicy` | `POST /v1/{resource=projects/*/locations/*/subscriptions/*}:getIamPolicy` Gets the IAM policy. |
| `https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.subscriptions/list` | `GET /v1/{parent=projects/*/locations/*}/subscriptions` Lists all subscriptions in a given project and location. |
| `https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.subscriptions/refresh` | `POST /v1/{name=projects/*/locations/*/subscriptions/*}:refresh` Refreshes a Subscription to a Data Exchange. |
| `https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.subscriptions/revoke` | `POST /v1/{name=projects/*/locations/*/subscriptions/*}:revoke` Revokes a given subscription. |
| `https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1/projects.locations.subscriptions/setIamPolicy` | `POST /v1/{resource=projects/*/locations/*/subscriptions/*}:setIamPolicy` Sets the IAM policy. |