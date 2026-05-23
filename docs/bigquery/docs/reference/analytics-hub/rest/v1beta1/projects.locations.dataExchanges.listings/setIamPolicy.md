# Method: projects.locations.dataExchanges.listings.setIamPolicy

- [HTTP request](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1beta1/projects.locations.dataExchanges.listings/setIamPolicy#body.HTTP_TEMPLATE)
- [Path parameters](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1beta1/projects.locations.dataExchanges.listings/setIamPolicy#body.PATH_PARAMETERS)
- [Request body](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1beta1/projects.locations.dataExchanges.listings/setIamPolicy#body.request_body)
  - [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1beta1/projects.locations.dataExchanges.listings/setIamPolicy#body.request_body.SCHEMA_REPRESENTATION)
- [Response body](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1beta1/projects.locations.dataExchanges.listings/setIamPolicy#body.response_body)
- [Authorization scopes](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1beta1/projects.locations.dataExchanges.listings/setIamPolicy#body.aspect)
- [IAM Permissions](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1beta1/projects.locations.dataExchanges.listings/setIamPolicy#body.aspect_1)
- [Try it!](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/v1beta1/projects.locations.dataExchanges.listings/setIamPolicy#try-it)

Sets the IAM policy.

### HTTP request

`POST https://analyticshub.googleapis.com/v1beta1/{resource=projects/*/locations/*/dataExchanges/*/listings/*}:setIamPolicy`

The URL uses [gRPC Transcoding](https://google.aip.dev/127) syntax.

### Path parameters

| Parameters ||
|---|---|
| `resource` | `string` REQUIRED: The resource for which the policy is being specified. See [Resource names](https://cloud.google.com/apis/design/resource_names) for the appropriate value for this field. |

### Request body

The request body contains data with the following structure:

| JSON representation |
|---|
| ``` { "policy": { object (`https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/Shared.Types/Policy`) }, "updateMask": string } ``` |

| Fields ||
|---|---|
| `policy` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/Shared.Types/Policy`)`` REQUIRED: The complete policy to be applied to the `resource`. The size of the policy is limited to a few 10s of KB. An empty policy is a valid policy but certain Google Cloud services (such as Projects) might reject them. |
| `updateMask` | ``string (`https://protobuf.dev/reference/protobuf/google.protobuf#field-mask` format)`` OPTIONAL: A FieldMask specifying which fields of the policy to modify. Only the fields in the mask will be modified. If no mask is provided, the following default mask is used: `paths: "bindings, etag"` This is a comma-separated list of fully qualified names of fields. Example: `"user.displayName,photo"`. |

### Response body

If successful, the response body contains an instance of `https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/Shared.Types/Policy`.

### Authorization scopes

Requires one of the following OAuth scopes:

- `https://www.googleapis.com/auth/bigquery`
- `https://www.googleapis.com/auth/cloud-platform`

For more information, see the [Authentication Overview](https://docs.cloud.google.com/docs/authentication#authorization-gcp).

### IAM Permissions

Requires **one of** the following [IAM](https://cloud.google.com/iam/docs) permissions on the `resource` resource, depending on the resource type:

- `analyticshub.dataExchanges.setIamPolicy`
- `analyticshub.listings.setIamPolicy`

For more information, see the [IAM documentation](https://cloud.google.com/iam/docs).