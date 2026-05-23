# Method: datasets.patch

- [HTTP request](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets/patch#body.HTTP_TEMPLATE)
- [Path parameters](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets/patch#body.PATH_PARAMETERS)
- [Query parameters](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets/patch#body.QUERY_PARAMETERS)
- [Request body](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets/patch#body.request_body)
- [Response body](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets/patch#body.response_body)
- [Authorization scopes](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets/patch#body.aspect)
- [Try it!](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets/patch#try-it)

Updates information in an existing dataset. The update method replaces the entire dataset resource, whereas the patch method only replaces fields that are provided in the submitted dataset resource. This method supports RFC5789 patch semantics.

### HTTP request

`PATCH https://bigquery.googleapis.com/bigquery/v2/projects/{projectId}/datasets/{datasetId}`

The URL uses [gRPC Transcoding](https://google.aip.dev/127) syntax.

### Path parameters

| Parameters ||
|---|---|
| `projectId` | `string` Required. Project ID of the dataset being updated |
| `datasetId` | `string` Required. Dataset ID of the dataset being updated |

### Query parameters

| Parameters ||
|---|---|
| `updateMode` | ``enum (`https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/UpdateMode`)`` Optional. Specifies the fields of dataset that update/patch operation is targeting By default, both metadata and ACL fields are updated. |
| `accessPolicyVersion` | `integer` Optional. The version of the provided access policy schema. Valid values are 0, 1, and 3. Requests specifying an invalid value will be rejected. This version refers to the schema version of the access policy and not the version of access policy. This field's value can be equal or more than the access policy schema provided in the request. For example, \* Operations updating conditional access policy binding in datasets must specify version 3. Some of the operations are : - Adding a new access policy entry with condition. - Removing an access policy entry with condition. - Updating an access policy entry with condition. \* But dataset with no conditional role bindings in access policy may specify any valid value or leave the field unset. If unset or if 0 or 1 value is used for dataset with conditional bindings, request will be rejected. This field will be mapped to IAM Policy version (<https://cloud.google.com/iam/docs/policies#versions>) and will be used to set policy in IAM. |

### Request body

The request body contains an instance of `https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets#Dataset`.

### Response body

If successful, the response body contains an instance of `https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets#Dataset`.

### Authorization scopes

Requires one of the following OAuth scopes:

- `https://www.googleapis.com/auth/bigquery`
- `https://www.googleapis.com/auth/cloud-platform`

For more information, see the [Authentication Overview](https://docs.cloud.google.com/docs/authentication#authorization-gcp).