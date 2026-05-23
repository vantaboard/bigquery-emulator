# Method: projects.locations.dataExchanges.listings.list

- [HTTP request](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1beta1/projects.locations.dataExchanges.listings/list#body.HTTP_TEMPLATE)
- [Path parameters](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1beta1/projects.locations.dataExchanges.listings/list#body.PATH_PARAMETERS)
- [Query parameters](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1beta1/projects.locations.dataExchanges.listings/list#body.QUERY_PARAMETERS)
- [Request body](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1beta1/projects.locations.dataExchanges.listings/list#body.request_body)
- [Response body](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1beta1/projects.locations.dataExchanges.listings/list#body.response_body)
  - [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1beta1/projects.locations.dataExchanges.listings/list#body.ListListingsResponse.SCHEMA_REPRESENTATION)
- [Authorization scopes](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1beta1/projects.locations.dataExchanges.listings/list#body.aspect)
- [IAM Permissions](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1beta1/projects.locations.dataExchanges.listings/list#body.aspect_1)
- [Try it!](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1beta1/projects.locations.dataExchanges.listings/list#try-it)

Lists all listings in a given project and location.

### HTTP request

`GET https://analyticshub.googleapis.com/v1beta1/{parent=projects/*/locations/*/dataExchanges/*}/listings`

The URL uses [gRPC Transcoding](https://google.aip.dev/127) syntax.

### Path parameters

| Parameters ||
|---|---|
| `parent` | `string` Required. The parent resource path of the listing. e.g. `projects/myproject/locations/us/dataExchanges/123`. |

### Query parameters

| Parameters ||
|---|---|
| `pageSize` | `integer` The maximum number of results to return in a single response page. Leverage the page tokens to iterate through the entire collection. |
| `pageToken` | `string` Page token, returned by a previous call, to request the next page of results. |

### Request body

The request body must be empty.

### Response body

Message for response to the list of Listings.

If successful, the response body contains data with the following structure:

| JSON representation |
|---|
| ``` { "listings": [ { object (`https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1beta1/projects.locations.dataExchanges.listings#Listing`) } ], "nextPageToken": string } ``` |

| Fields ||
|---|---|
| `listings[]` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1beta1/projects.locations.dataExchanges.listings#Listing`)`` The list of Listing. |
| `nextPageToken` | `string` A token to request the next page of results. |

### Authorization scopes

Requires one of the following OAuth scopes:

- `https://www.googleapis.com/auth/bigquery`
- `https://www.googleapis.com/auth/cloud-platform`

For more information, see the [Authentication Overview](https://docs.cloud.google.com/docs/authentication#authorization-gcp).

### IAM Permissions

Requires the following [IAM](https://cloud.google.com/iam/docs) permission on the `parent` resource:

- `analyticshub.listings.list`

For more information, see the [IAM documentation](https://cloud.google.com/iam/docs).