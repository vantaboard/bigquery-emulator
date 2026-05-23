# Policy

- [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/auditlogs/rest/Shared.Types/Policy#SCHEMA_REPRESENTATION)
- [Binding](https://docs.cloud.google.com/bigquery/docs/reference/auditlogs/rest/Shared.Types/Policy#Binding)
  - [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/auditlogs/rest/Shared.Types/Policy#Binding.SCHEMA_REPRESENTATION)
- [AuditConfig](https://docs.cloud.google.com/bigquery/docs/reference/auditlogs/rest/Shared.Types/Policy#AuditConfig)
  - [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/auditlogs/rest/Shared.Types/Policy#AuditConfig.SCHEMA_REPRESENTATION)
- [AuditLogConfig](https://docs.cloud.google.com/bigquery/docs/reference/auditlogs/rest/Shared.Types/Policy#AuditLogConfig)
  - [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/auditlogs/rest/Shared.Types/Policy#AuditLogConfig.SCHEMA_REPRESENTATION)

An Identity and Access Management (IAM) policy, which specifies access controls for Google Cloud resources.

A `Policy` is a collection of `bindings`. A `binding` binds one or more `members`, or principals, to a single `role`. Principals can be user accounts, service accounts, Google groups, and domains (such as G Suite). A `role` is a named list of permissions; each `role` can be an IAM predefined role or a user-created custom role.

For some types of Google Cloud resources, a `binding` can also specify a `condition`, which is a logical expression that allows access to a resource only if the expression evaluates to `true`. A condition can add constraints based on attributes of the request, the resource, or both. To learn which resources support conditions in their IAM policies, see the [IAM documentation](https://cloud.google.com/iam/help/conditions/resource-policies).

**JSON example:**

        {
          "bindings": [
            {
              "role": "roles/resourcemanager.organizationAdmin",
              "members": [
                "user:mike@example.com",
                "group:admins@example.com",
                "domain:google.com",
                "serviceAccount:my-project-id@appspot.gserviceaccount.com"
              ]
            },
            {
              "role": "roles/resourcemanager.organizationViewer",
              "members": [
                "user:eve@example.com"
              ],
              "condition": {
                "title": "expirable access",
                "description": "Does not grant access after Sep 2020",
                "expression": "request.time < timestamp('2020-10-01T00:00:00.000Z')",
              }
            }
          ],
          "etag": "BwWWja0YfJA=",
          "version": 3
        }

**YAML example:**

        bindings:
        - members:
          - user:mike@example.com
          - group:admins@example.com
          - domain:google.com
          - serviceAccount:my-project-id@appspot.gserviceaccount.com
          role: roles/resourcemanager.organizationAdmin
        - members:
          - user:eve@example.com
          role: roles/resourcemanager.organizationViewer
          condition:
            title: expirable access
            description: Does not grant access after Sep 2020
            expression: request.time < timestamp('2020-10-01T00:00:00.000Z')
        etag: BwWWja0YfJA=
        version: 3

For a description of IAM and its features, see the [IAM documentation](https://cloud.google.com/iam/docs/).

| JSON representation |
|---|
| ``` { "version": integer, "bindings": [ { object (`https://docs.cloud.google.com/bigquery/docs/reference/auditlogs/rest/Shared.Types/Policy#Binding`) } ], "auditConfigs": [ { object (`https://docs.cloud.google.com/bigquery/docs/reference/auditlogs/rest/Shared.Types/Policy#AuditConfig`) } ], "etag": string } ``` |

| Fields ||
|---|---|
| `version` | `integer` Specifies the format of the policy. Valid values are `0`, `1`, and `3`. Requests that specify an invalid value are rejected. Any operation that affects conditional role bindings must specify version `3`. This requirement applies to the following operations: - Getting a policy that includes a conditional role binding - Adding a conditional role binding to a policy - Changing a conditional role binding in a policy - Removing any role binding, with or without a condition, from a policy that includes conditions **Important:** If you use IAM Conditions, you must include the `etag` field whenever you call `setIamPolicy`. If you omit this field, then IAM allows you to overwrite a version `3` policy with a version `1` policy, and all of the conditions in the version `3` policy are lost. If a policy does not include any conditions, operations on that policy may specify any valid version or leave the field unset. To learn which resources support conditions in their IAM policies, see the [IAM documentation](https://cloud.google.com/iam/help/conditions/resource-policies). |
| `bindings[]` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/auditlogs/rest/Shared.Types/Policy#Binding`)`` Associates a list of `members`, or principals, with a `role`. Optionally, may specify a `condition` that determines how and when the `bindings` are applied. Each of the `bindings` must contain at least one principal. The `bindings` in a `Policy` can refer to up to 1,500 principals; up to 250 of these principals can be Google groups. Each occurrence of a principal counts towards these limits. For example, if the `bindings` grant 50 different roles to `user:alice@example.com`, and not to any other principal, then you can add another 1,450 principals to the `bindings` in the `Policy`. |
| `auditConfigs[]` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/auditlogs/rest/Shared.Types/Policy#AuditConfig`)`` Specifies cloud audit logging configuration for this policy. |
| `etag` | `string (https://developers.google.com/discovery/v1/type-format format)` `etag` is used for optimistic concurrency control as a way to help prevent simultaneous updates of a policy from overwriting each other. It is strongly suggested that systems make use of the `etag` in the read-modify-write cycle to perform policy updates in order to avoid race conditions: An `etag` is returned in the response to `getIamPolicy`, and systems are expected to put that etag in the request to `setIamPolicy` to ensure that their change will be applied to the same version of the policy. **Important:** If you use IAM Conditions, you must include the `etag` field whenever you call `setIamPolicy`. If you omit this field, then IAM allows you to overwrite a version `3` policy with a version `1` policy, and all of the conditions in the version `3` policy are lost. A base64-encoded string. |

## Binding

Associates `members`, or principals, with a `role`.

| JSON representation |
|---|
| ``` { "role": string, "members": [ string ], "condition": { object (`https://docs.cloud.google.com/bigquery/docs/reference/auditlogs/rest/Shared.Types/Expr`) } } ``` |

| Fields ||
|---|---|
| `role` | `string` Role that is assigned to the list of `members`, or principals. For example, `roles/viewer`, `roles/editor`, or `roles/owner`. For an overview of the IAM roles and permissions, see the [IAM documentation](https://cloud.google.com/iam/docs/roles-overview). For a list of the available pre-defined roles, see [here](https://cloud.google.com/iam/docs/understanding-roles). |
| `members[]` | `string` Specifies the principals requesting access for a Google Cloud resource. `members` can have the following values: - `allUsers`: A special identifier that represents anyone who is on the internet; with or without a Google account. - `allAuthenticatedUsers`: A special identifier that represents anyone who is authenticated with a Google account or a service account. Does not include identities that come from external identity providers (IdPs) through identity federation. - `user:{emailid}`: An email address that represents a specific Google account. For example, `alice@example.com` . <!-- --> - `serviceAccount:{emailid}`: An email address that represents a Google service account. For example, `my-other-app@appspot.gserviceaccount.com`. - `serviceAccount:{projectid}.svc.id.goog[{namespace}/{kubernetes-sa}]`: An identifier for a [Kubernetes service account](https://cloud.google.com/kubernetes-engine/docs/how-to/kubernetes-service-accounts). For example, `my-project.svc.id.goog[my-namespace/my-kubernetes-sa]`. - `group:{emailid}`: An email address that represents a Google group. For example, `admins@example.com`. <!-- --> - `domain:{domain}`: The G Suite domain (primary) that represents all the users of that domain. For example, `google.com` or `example.com`. <!-- --> - `principal://iam.googleapis.com/locations/global/workforcePools/{pool_id}/subject/{subject_attribute_value}`: A single identity in a workforce identity pool. - `principalSet://iam.googleapis.com/locations/global/workforcePools/{pool_id}/group/{groupId}`: All workforce identities in a group. - `principalSet://iam.googleapis.com/locations/global/workforcePools/{pool_id}/attribute.{attribute_name}/{attribute_value}`: All workforce identities with a specific attribute value. - `principalSet://iam.googleapis.com/locations/global/workforcePools/{pool_id}/*`: All identities in a workforce identity pool. - `principal://iam.googleapis.com/projects/{project_number}/locations/global/workloadIdentityPools/{pool_id}/subject/{subject_attribute_value}`: A single identity in a workload identity pool. - `principalSet://iam.googleapis.com/projects/{project_number}/locations/global/workloadIdentityPools/{pool_id}/group/{groupId}`: A workload identity pool group. - `principalSet://iam.googleapis.com/projects/{project_number}/locations/global/workloadIdentityPools/{pool_id}/attribute.{attribute_name}/{attribute_value}`: All identities in a workload identity pool with a certain attribute. - `principalSet://iam.googleapis.com/projects/{project_number}/locations/global/workloadIdentityPools/{pool_id}/*`: All identities in a workload identity pool. - `deleted:user:{emailid}?uid={uniqueid}`: An email address (plus unique identifier) representing a user that has been recently deleted. For example, `alice@example.com?uid=123456789012345678901`. If the user is recovered, this value reverts to `user:{emailid}` and the recovered user retains the role in the binding. - `deleted:serviceAccount:{emailid}?uid={uniqueid}`: An email address (plus unique identifier) representing a service account that has been recently deleted. For example, `my-other-app@appspot.gserviceaccount.com?uid=123456789012345678901`. If the service account is undeleted, this value reverts to `serviceAccount:{emailid}` and the undeleted service account retains the role in the binding. - `deleted:group:{emailid}?uid={uniqueid}`: An email address (plus unique identifier) representing a Google group that has been recently deleted. For example, `admins@example.com?uid=123456789012345678901`. If the group is recovered, this value reverts to `group:{emailid}` and the recovered group retains the role in the binding. - `deleted:principal://iam.googleapis.com/locations/global/workforcePools/{pool_id}/subject/{subject_attribute_value}`: Deleted single identity in a workforce identity pool. For example, `deleted:principal://iam.googleapis.com/locations/global/workforcePools/my-pool-id/subject/my-subject-attribute-value`. |
| `condition` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/auditlogs/rest/Shared.Types/Expr`)`` The condition that is associated with this binding. If the condition evaluates to `true`, then this binding applies to the current request. If the condition evaluates to `false`, then this binding does not apply to the current request. However, a different role binding might grant the same role to one or more of the principals in this binding. To learn which resources support conditions in their IAM policies, see the [IAM documentation](https://cloud.google.com/iam/help/conditions/resource-policies). |

## AuditConfig

Specifies the audit configuration for a service. The configuration determines which permission types are logged, and what identities, if any, are exempted from logging. An AuditConfig must have one or more AuditLogConfigs.

If there are AuditConfigs for both `allServices` and a specific service, the union of the two AuditConfigs is used for that service: the log_types specified in each AuditConfig are enabled, and the exemptedMembers in each AuditLogConfig are exempted.

Example Policy with multiple AuditConfigs:

    {
      "auditConfigs": [
        {
          "service": "allServices",
          "auditLogConfigs": [
            {
              "logType": "DATA_READ",
              "exemptedMembers": [
                "user:jose@example.com"
              ]
            },
            {
              "logType": "DATA_WRITE"
            },
            {
              "logType": "ADMIN_READ"
            }
          ]
        },
        {
          "service": "sampleservice.googleapis.com",
          "auditLogConfigs": [
            {
              "logType": "DATA_READ"
            },
            {
              "logType": "DATA_WRITE",
              "exemptedMembers": [
                "user:aliya@example.com"
              ]
            }
          ]
        }
      ]
    }

For sampleservice, this policy enables DATA_READ, DATA_WRITE and ADMIN_READ logging. It also exempts `jose@example.com` from DATA_READ logging, and `aliya@example.com` from DATA_WRITE logging.

| JSON representation |
|---|
| ``` { "service": string, "auditLogConfigs": [ { object (`https://docs.cloud.google.com/bigquery/docs/reference/auditlogs/rest/Shared.Types/Policy#AuditLogConfig`) } ] } ``` |

| Fields ||
|---|---|
| `service` | `string` Specifies a service that will be enabled for audit logging. For example, `storage.googleapis.com`, `cloudsql.googleapis.com`. `allServices` is a special value that covers all services. |
| `auditLogConfigs[]` | ``object (`https://docs.cloud.google.com/bigquery/docs/reference/auditlogs/rest/Shared.Types/Policy#AuditLogConfig`)`` The configuration for logging of each type of permission. |

## AuditLogConfig

Provides the configuration for logging a type of permissions. Example:

    {
      "auditLogConfigs": [
        {
          "logType": "DATA_READ",
          "exemptedMembers": [
            "user:jose@example.com"
          ]
        },
        {
          "logType": "DATA_WRITE"
        }
      ]
    }

This enables 'DATA_READ' and 'DATA_WRITE' logging, while exempting [jose@example.com](mailto:jose@example.com) from DATA_READ logging.

| JSON representation |
|---|
| ``` { "logType": enum (`https://docs.cloud.google.com/bigquery/docs/reference/auditlogs/rest/Shared.Types/AuditLogConfig.LogType`), "exemptedMembers": [ string ] } ``` |

| Fields ||
|---|---|
| `logType` | ``enum (`https://docs.cloud.google.com/bigquery/docs/reference/auditlogs/rest/Shared.Types/AuditLogConfig.LogType`)`` The log type that this config enables. |
| `exemptedMembers[]` | `string` Specifies the identities that do not cause logging for this type of permission. Follows the same format of `https://docs.cloud.google.com/bigquery/docs/reference/auditlogs/rest/Shared.Types/Policy#Binding.FIELDS.members`. |