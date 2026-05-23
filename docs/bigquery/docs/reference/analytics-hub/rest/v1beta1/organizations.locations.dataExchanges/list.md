# Method: organizations.locations.dataExchanges.list

- [HTTP request](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1beta1/organizations.locations.dataExchanges/list#body.HTTP_TEMPLATE)
- [Path parameters](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1beta1/organizations.locations.dataExchanges/list#body.PATH_PARAMETERS)
- [Query parameters](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1beta1/organizations.locations.dataExchanges/list#body.QUERY_PARAMETERS)
- [Request body](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1beta1/organizations.locations.dataExchanges/list#body.request_body)
- [Response body](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1beta1/organizations.locations.dataExchanges/list#body.response_body)
  - [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1beta1/organizations.locations.dataExchanges/list#body.ListOrgDataExchangesResponse.SCHEMA_REPRESENTATION)
- [Authorization scopes](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1beta1/organizations.locations.dataExchanges/list#body.aspect)
- [Try it!](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1beta1/organizations.locations.dataExchanges/list#try-it)

Lists all data exchanges from projects in a given organization and location.

### HTTP request

`GET https://analyticshub.googleapis.com/v1beta1/{organization=organizations/*/locations/*}/dataExchanges`

The URL uses [gRPC Transcoding](https://google.aip.dev/127) syntax.

### Path parameters

| Parameters ||
|---|---|
| `organization` | `string` Required. The organization resource path of the projects containing DataExchanges. e.g. `organizations/myorg/locations/us`. |

### Query parameters

| Parameters ||
|---|---|
| `pageSize` | `integer` The maximum number of results to return in a single response page. Leverage the page tokens to iterate through the entire collection. |
| `pageToken` | `string` Page token, returned by a previous call, to request the next page of results. |

### Request body

The request body must be empty.

### Response body

Message for response to listing data exchanges in an organization and location.

If successful, the response body contains data with the following structure:

| JSON representation |
|---|
| ``` { "dataExchanges": [ { object (`https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1beta1/projects.locations.dataExchanges#DataExchange`) } ], "nextPageToken": string } ``` |

| Fields ||
|---|---|
| `dataExchanges[]` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1beta1/projects.locations.dataExchanges#DataExchange`)`` The list of data exchanges. |
| `nextPageToken` | `string` A token to request the next page of results. |

### Authorization scopes

Requires one of the following OAuth scopes:

- `https://www.googleapis.com/auth/bigquery`
- `https://www.googleapis.com/auth/cloud-platform`

For more information, see the [Authentication Overview](https://docs.cloud.google.com/docs/authentication#authorization-gcp).