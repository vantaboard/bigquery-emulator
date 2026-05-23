# REST Resource: projects.locations.dataPolicies

- [Resource: DataPolicy](https://docs.cloud.google.com/bigquery/docs/reference/bigquerydatapolicy/rest/v1beta1/projects.locations.dataPolicies#DataPolicy)
  - [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/bigquerydatapolicy/rest/v1beta1/projects.locations.dataPolicies#DataPolicy.SCHEMA_REPRESENTATION)
- [DataMaskingPolicy](https://docs.cloud.google.com/bigquery/docs/reference/bigquerydatapolicy/rest/v1beta1/projects.locations.dataPolicies#DataMaskingPolicy)
  - [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/bigquerydatapolicy/rest/v1beta1/projects.locations.dataPolicies#DataMaskingPolicy.SCHEMA_REPRESENTATION)
- [PredefinedExpression](https://docs.cloud.google.com/bigquery/docs/reference/bigquerydatapolicy/rest/v1beta1/projects.locations.dataPolicies#PredefinedExpression)
- [DataPolicyType](https://docs.cloud.google.com/bigquery/docs/reference/bigquerydatapolicy/rest/v1beta1/projects.locations.dataPolicies#DataPolicyType)
- [Methods](https://docs.cloud.google.com/bigquery/docs/reference/bigquerydatapolicy/rest/v1beta1/projects.locations.dataPolicies#METHODS_SUMMARY)

## Resource: DataPolicy

Represents the label-policy binding.

| JSON representation |
|---|
| ``` { "name": string, "dataPolicyType": enum (`https://docs.cloud.google.com/bigquery/docs/reference/bigquerydatapolicy/rest/v1beta1/projects.locations.dataPolicies#DataPolicyType`), "dataPolicyId": string, // Union field `matching_label` can be only one of the following: "policyTag": string // End of list of possible types for union field `matching_label`. // Union field `policy` can be only one of the following: "dataMaskingPolicy": { object (`https://docs.cloud.google.com/bigquery/docs/reference/bigquerydatapolicy/rest/v1beta1/projects.locations.dataPolicies#DataMaskingPolicy`) } // End of list of possible types for union field `policy`. } ``` |

| Fields ||
|---|---|
| `name` | `string` Output only. Resource name of this data policy, in the format of `projects/{projectNumber}/locations/{locationId}/dataPolicies/{dataPolicyId}`. |
| `dataPolicyType` | ``enum (`https://docs.cloud.google.com/bigquery/docs/reference/bigquerydatapolicy/rest/v1beta1/projects.locations.dataPolicies#DataPolicyType`)`` Required. Data policy type. Type of data policy. |
| `dataPolicyId` | `string` User-assigned (human readable) ID of the data policy that needs to be unique within a project. Used as {dataPolicyId} in part of the resource name. |
| Union field `matching_label`. Label that is bound to this data policy. `matching_label` can be only one of the following: ||
| `policyTag` | `string` Policy tag resource name, in the format of `projects/{projectNumber}/locations/{locationId}/taxonomies/{taxonomyId}/policyTags/{policyTag_id}`. |
| Union field `policy`. The policy that is bound to this data policy. `policy` can be only one of the following: ||
| `dataMaskingPolicy` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/bigquerydatapolicy/rest/v1beta1/projects.locations.dataPolicies#DataMaskingPolicy`)`` The data masking policy that specifies the data masking rule to use. |

## DataMaskingPolicy

The data masking policy that is used to specify data masking rule.

| JSON representation |
|---|
| ``` { // Union field `masking_expression` can be only one of the following: "predefinedExpression": enum (`https://docs.cloud.google.com/bigquery/docs/reference/bigquerydatapolicy/rest/v1beta1/projects.locations.dataPolicies#PredefinedExpression`) // End of list of possible types for union field `masking_expression`. } ``` |

| Fields ||
|---|---|
| Union field `masking_expression`. A masking expression to bind to the data masking rule. `masking_expression` can be only one of the following: ||
| `predefinedExpression` | ``enum (`https://docs.cloud.google.com/bigquery/docs/reference/bigquerydatapolicy/rest/v1beta1/projects.locations.dataPolicies#PredefinedExpression`)`` A predefined masking expression. |

## PredefinedExpression

The available masking rules. Learn more here: <https://cloud.google.com/bigquery/docs/column-data-masking-intro#masking_options>.

| Enums ||
|---|---|
| `PREDEFINED_EXPRESSION_UNSPECIFIED` | Default, unspecified predefined expression. No masking will take place since no expression is specified. |
| `SHA256` | Masking expression to replace data with SHA-256 hash. |
| `ALWAYS_NULL` | Masking expression to replace data with NULLs. |
| `DEFAULT_MASKING_VALUE` | Masking expression to replace data with their default masking values. The default masking values for each type listed as below: - STRING: "" - BYTES: b'' - INTEGER: 0 - FLOAT: 0.0 - NUMERIC: 0 - BOOLEAN: FALSE - TIMESTAMP: 1970-01-01 00:00:00 UTC - DATE: 1970-01-01 - TIME: 00:00:00 - DATETIME: 1970-01-01T00:00:00 - GEOGRAPHY: POINT(0 0) - BIGNUMERIC: 0 - ARRAY: \[\] - STRUCT: NOT_APPLICABLE - JSON: NULL |

## DataPolicyType

A list of supported data policy types.

| Enums ||
|---|---|
| `DATA_POLICY_TYPE_UNSPECIFIED` | Default value for the data policy type. This should not be used. |
| `COLUMN_LEVEL_SECURITY_POLICY` | Used to create a data policy for column-level security, without data masking. |
| `DATA_MASKING_POLICY` | Used to create a data policy for data masking. |

| ## Methods ||
|---|---|
| ### `https://docs.cloud.google.com/bigquery/docs/reference/bigquerydatapolicy/rest/v1beta1/projects.locations.dataPolicies/create` | Creates a new data policy under a project with the given `dataPolicyId` (used as the display name), policy tag, and data policy type. |
| ### `https://docs.cloud.google.com/bigquery/docs/reference/bigquerydatapolicy/rest/v1beta1/projects.locations.dataPolicies/delete` | Deletes the data policy specified by its resource name. |
| ### `https://docs.cloud.google.com/bigquery/docs/reference/bigquerydatapolicy/rest/v1beta1/projects.locations.dataPolicies/get` | Gets the data policy specified by its resource name. |
| ### `https://docs.cloud.google.com/bigquery/docs/reference/bigquerydatapolicy/rest/v1beta1/projects.locations.dataPolicies/getIamPolicy` | Gets the IAM policy for the specified data policy. |
| ### `https://docs.cloud.google.com/bigquery/docs/reference/bigquerydatapolicy/rest/v1beta1/projects.locations.dataPolicies/list` | List all of the data policies in the specified parent project. |
| ### `https://docs.cloud.google.com/bigquery/docs/reference/bigquerydatapolicy/rest/v1beta1/projects.locations.dataPolicies/patch` | Updates the metadata for an existing data policy. |
| ### `https://docs.cloud.google.com/bigquery/docs/reference/bigquerydatapolicy/rest/v1beta1/projects.locations.dataPolicies/setIamPolicy` | Sets the IAM policy for the specified data policy. |
| ### `https://docs.cloud.google.com/bigquery/docs/reference/bigquerydatapolicy/rest/v1beta1/projects.locations.dataPolicies/testIamPermissions` | Returns the caller's permission on the specified data policy resource. |