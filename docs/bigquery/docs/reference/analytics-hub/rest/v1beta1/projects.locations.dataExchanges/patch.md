# Method: projects.locations.dataExchanges.patch

- [HTTP request](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1beta1/projects.locations.dataExchanges/patch#body.HTTP_TEMPLATE)
- [Path parameters](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1beta1/projects.locations.dataExchanges/patch#body.PATH_PARAMETERS)
- [Query parameters](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1beta1/projects.locations.dataExchanges/patch#body.QUERY_PARAMETERS)
- [Request body](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1beta1/projects.locations.dataExchanges/patch#body.request_body)
- [Response body](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1beta1/projects.locations.dataExchanges/patch#body.response_body)
- [Authorization scopes](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1beta1/projects.locations.dataExchanges/patch#body.aspect)
- [IAM Permissions](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1beta1/projects.locations.dataExchanges/patch#body.aspect_1)
- [Try it!](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1beta1/projects.locations.dataExchanges/patch#try-it)

Updates an existing data exchange.

### HTTP request

`PATCH https://analyticshub.googleapis.com/v1beta1/{dataExchange.name=projects/*/locations/*/dataExchanges/*}`

The URL uses [gRPC Transcoding](https://google.aip.dev/127) syntax.

### Path parameters

| Parameters ||
|---|---|
| `dataExchange.name` | `string` Output only. The resource name of the data exchange. e.g. `projects/myproject/locations/us/dataExchanges/123`. |

### Query parameters

| Parameters ||
|---|---|
| `updateMask` | ``string (`https://protobuf.dev/reference/protobuf/google.protobuf#field-mask` format)`` Required. Field mask specifies the fields to update in the data exchange resource. The fields specified in the `updateMask` are relative to the resource and are not a full request. This is a comma-separated list of fully qualified names of fields. Example: `"user.displayName,photo"`. |

### Request body

The request body contains an instance of `https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1beta1/projects.locations.dataExchanges#DataExchange`.

### Response body

If successful, the response body contains an instance of `https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1beta1/projects.locations.dataExchanges#DataExchange`.

### Authorization scopes

Requires one of the following OAuth scopes:

- `https://www.googleapis.com/auth/bigquery`
- `https://www.googleapis.com/auth/cloud-platform`

For more information, see the [Authentication Overview](https://docs.cloud.google.com/docs/authentication#authorization-gcp).

### IAM Permissions

Requires the following [IAM](https://cloud.google.com/iam/docs) permission on the `name` resource:

- `analyticshub.dataExchanges.update`

For more information, see the [IAM documentation](https://cloud.google.com/iam/docs).