# Method: routines.getIamPolicy

- [HTTP request](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/routines/getIamPolicy#body.HTTP_TEMPLATE)
- [Path parameters](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/routines/getIamPolicy#body.PATH_PARAMETERS)
- [Request body](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/routines/getIamPolicy#body.request_body)
  - [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/routines/getIamPolicy#body.request_body.SCHEMA_REPRESENTATION)
- [Response body](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/routines/getIamPolicy#body.response_body)
- [Authorization scopes](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/routines/getIamPolicy#body.aspect)
- [Try it!](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/routines/getIamPolicy#try-it)

Gets the access control policy for a resource. Returns an empty policy if the resource exists and does not have a policy set.

### HTTP request

`POST https://bigquery.googleapis.com/bigquery/v2/{resource=projects/*/datasets/*/routines/*}:getIamPolicy`

The URL uses [gRPC Transcoding](https://google.aip.dev/127) syntax.

### Path parameters

| Parameters ||
|---|---|
| `resource` | `string` REQUIRED: The resource for which the policy is being requested. See [Resource names](https://cloud.google.com/apis/design/resource_names) for the appropriate value for this field. |

### Request body

The request body contains data with the following structure:

| JSON representation |
|---|
| ``` { "options": { object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/GetPolicyOptions`) } } ``` |

| Fields ||
|---|---|
| `options` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/GetPolicyOptions`)`` OPTIONAL: A `GetPolicyOptions` object for specifying options to `routines.getIamPolicy`. |

### Response body

If successful, the response body contains an instance of `https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/Policy`.

### Authorization scopes

Requires one of the following OAuth scopes:

- `https://www.googleapis.com/auth/bigquery`
- `https://www.googleapis.com/auth/cloud-platform`
- `https://www.googleapis.com/auth/bigquery.readonly`
- `https://www.googleapis.com/auth/cloud-platform.read-only`

For more information, see the [Authentication Overview](https://docs.cloud.google.com/docs/authentication#authorization-gcp).