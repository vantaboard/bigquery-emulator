# Method: projects.locations.dataExchanges.getIamPolicy

- [HTTP request](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1beta1/projects.locations.dataExchanges/getIamPolicy#body.HTTP_TEMPLATE)
- [Path parameters](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1beta1/projects.locations.dataExchanges/getIamPolicy#body.PATH_PARAMETERS)
- [Request body](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1beta1/projects.locations.dataExchanges/getIamPolicy#body.request_body)
  - [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1beta1/projects.locations.dataExchanges/getIamPolicy#body.request_body.SCHEMA_REPRESENTATION)
- [Response body](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1beta1/projects.locations.dataExchanges/getIamPolicy#body.response_body)
- [Authorization scopes](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1beta1/projects.locations.dataExchanges/getIamPolicy#body.aspect)
- [IAM Permissions](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1beta1/projects.locations.dataExchanges/getIamPolicy#body.aspect_1)
- [Try it!](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1beta1/projects.locations.dataExchanges/getIamPolicy#try-it)

Gets the IAM policy.

### HTTP request

`POST https://analyticshub.googleapis.com/v1beta1/{resource=projects/*/locations/*/dataExchanges/*}:getIamPolicy`

The URL uses [gRPC Transcoding](https://google.aip.dev/127) syntax.

### Path parameters

| Parameters ||
|---|---|
| `resource` | `string` REQUIRED: The resource for which the policy is being requested. See [Resource names](https://cloud.google.com/apis/design/resource_names) for the appropriate value for this field. |

### Request body

The request body contains data with the following structure:

| JSON representation |
|---|
| ``` { "options": { object (`https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/Shared.Types/GetIamPolicyRequest#GetPolicyOptions`) } } ``` |

| Fields ||
|---|---|
| `options` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/Shared.Types/GetIamPolicyRequest#GetPolicyOptions`)`` OPTIONAL: A `GetPolicyOptions` object for specifying options to `dataExchanges.getIamPolicy`. |

### Response body

If successful, the response body contains an instance of `https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/Shared.Types/Policy`.

### Authorization scopes

Requires one of the following OAuth scopes:

- `https://www.googleapis.com/auth/bigquery`
- `https://www.googleapis.com/auth/cloud-platform`

For more information, see the [Authentication Overview](https://docs.cloud.google.com/docs/authentication#authorization-gcp).

### IAM Permissions

Requires **one of** the following [IAM](https://cloud.google.com/iam/docs) permissions on the `resource` resource, depending on the resource type:

- `analyticshub.dataExchanges.getIamPolicy`
- `analyticshub.listings.getIamPolicy`

For more information, see the [IAM documentation](https://cloud.google.com/iam/docs).