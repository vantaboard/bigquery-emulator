# Method: datasets.insert

- [HTTP request](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets/insert#body.HTTP_TEMPLATE)
- [Path parameters](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets/insert#body.PATH_PARAMETERS)
- [Query parameters](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets/insert#body.QUERY_PARAMETERS)
- [Request body](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets/insert#body.request_body)
- [Response body](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets/insert#body.response_body)
- [Authorization scopes](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets/insert#body.aspect)
- [Try it!](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets/insert#try-it)

Creates a new empty dataset.

### HTTP request

`POST https://bigquery.googleapis.com/bigquery/v2/projects/{projectId}/datasets`

The URL uses [gRPC Transcoding](https://google.aip.dev/127) syntax.

### Path parameters

| Parameters ||
|---|---|
| `projectId` | `string` Required. Project ID of the new dataset |

### Query parameters

| Parameters ||
|---|---|
| `accessPolicyVersion` | `integer` Optional. The version of the provided access policy schema. Valid values are 0, 1, and 3. Requests specifying an invalid value will be rejected. This version refers to the schema version of the access policy and not the version of access policy. This field's value can be equal or more than the access policy schema provided in the request. For example, \* Requests with conditional access policy binding in datasets must specify version 3. \* But dataset with no conditional role bindings in access policy may specify any valid value or leave the field unset. If unset or if 0 or 1 value is used for dataset with conditional bindings, request will be rejected. This field will be mapped to IAM Policy version (<https://cloud.google.com/iam/docs/policies#versions>) and will be used to set policy in IAM. |

### Request body

The request body contains an instance of `https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets#Dataset`.

### Response body

If successful, the response body contains a newly created instance of `https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets#Dataset`.

### Authorization scopes

Requires one of the following OAuth scopes:

- `https://www.googleapis.com/auth/bigquery`
- `https://www.googleapis.com/auth/cloud-platform`

For more information, see the [Authentication Overview](https://docs.cloud.google.com/docs/authentication#authorization-gcp).