# Method: projects.locations.dataPolicies.setIamPolicy

- [HTTP request](https://docs.cloud.google.com/bigquery/docs/reference/bigquerydatapolicy/rest/v2beta1/projects.locations.dataPolicies/setIamPolicy#body.HTTP_TEMPLATE)
- [Path parameters](https://docs.cloud.google.com/bigquery/docs/reference/bigquerydatapolicy/rest/v2beta1/projects.locations.dataPolicies/setIamPolicy#body.PATH_PARAMETERS)
- [Request body](https://docs.cloud.google.com/bigquery/docs/reference/bigquerydatapolicy/rest/v2beta1/projects.locations.dataPolicies/setIamPolicy#body.request_body)
  - [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/bigquerydatapolicy/rest/v2beta1/projects.locations.dataPolicies/setIamPolicy#body.request_body.SCHEMA_REPRESENTATION)
- [Response body](https://docs.cloud.google.com/bigquery/docs/reference/bigquerydatapolicy/rest/v2beta1/projects.locations.dataPolicies/setIamPolicy#body.response_body)
- [Authorization scopes](https://docs.cloud.google.com/bigquery/docs/reference/bigquerydatapolicy/rest/v2beta1/projects.locations.dataPolicies/setIamPolicy#body.aspect)
- [IAM Permissions](https://docs.cloud.google.com/bigquery/docs/reference/bigquerydatapolicy/rest/v2beta1/projects.locations.dataPolicies/setIamPolicy#body.aspect_1)
- [Try it!](https://docs.cloud.google.com/bigquery/docs/reference/bigquerydatapolicy/rest/v2beta1/projects.locations.dataPolicies/setIamPolicy#try-it)

Sets the IAM policy for the specified data policy.

### HTTP request

`POST https://bigquerydatapolicy.googleapis.com/v2beta1/{resource=projects/*/locations/*/dataPolicies/*}:setIamPolicy`

The URL uses [gRPC Transcoding](https://google.aip.dev/127) syntax.

### Path parameters

| Parameters ||
|---|---|
| `resource` | `string` REQUIRED: The resource for which the policy is being specified. See [Resource names](https://cloud.google.com/apis/design/resource_names) for the appropriate value for this field. |

### Request body

The request body contains data with the following structure:

| JSON representation |
|---|
| ``` { "policy": { object (`https://docs.cloud.google.com/iam/docs/reference/rest/v1/Policy`) }, "updateMask": string } ``` |

| Fields ||
|---|---|
| `policy` | ``object (`https://docs.cloud.google.com/iam/docs/reference/rest/v1/Policy`)`` REQUIRED: The complete policy to be applied to the `resource`. The size of the policy is limited to a few 10s of KB. An empty policy is a valid policy but certain Google Cloud services (such as Projects) might reject them. |
| `updateMask` | ``string (`https://protobuf.dev/reference/protobuf/google.protobuf#field-mask` format)`` OPTIONAL: A FieldMask specifying which fields of the policy to modify. Only the fields in the mask will be modified. If no mask is provided, the following default mask is used: `paths: "bindings, etag"` This is a comma-separated list of fully qualified names of fields. Example: `"user.displayName,photo"`. |

### Response body

If successful, the response body contains an instance of `https://docs.cloud.google.com/iam/docs/reference/rest/v1/Policy`.

### Authorization scopes

Requires one of the following OAuth scopes:

- `https://www.googleapis.com/auth/bigquery`
- `https://www.googleapis.com/auth/cloud-platform`

For more information, see the [Authentication Overview](https://docs.cloud.google.com/docs/authentication#authorization-gcp).

### IAM Permissions

Requires the following [IAM](https://cloud.google.com/iam/docs) permission on the `resource` resource:

- `bigquery.dataPolicies.setIamPolicy`

For more information, see the [IAM documentation](https://cloud.google.com/iam/docs).