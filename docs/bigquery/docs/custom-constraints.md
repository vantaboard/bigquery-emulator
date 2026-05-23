# Use organization policies to apply custom constraints to BigQuery resources

> [!WARNING]
>
> **Preview
> --- Constraints on tables, data access policies, row access polcies, and routines**
>
>
> This feature is
>
> subject to the "Pre-GA Offerings Terms" in the General Service Terms section of the
> [Service Specific
> Terms](https://docs.cloud.google.com/terms/service-terms#1).
>
> Pre-GA features are available "as is" and might have limited support.
>
> For more information, see the
> [launch stage descriptions](https://cloud.google.com/products/#product-launch-stages).

This page shows you how to use Organization Policy Service custom constraints to restrict
specific operations on the following Google Cloud resources:

- `bigquery.googleapis.com/Dataset`
- `bigquery.googleapis.com/Routine`
- `bigquery.googleapis.com/Table`
- `bigquery.googleapis.com/RowAccessPolicy`
- `bigquerydatapolicy.googleapis.com/DataPolicy`

To learn more about Organization Policy, see
[Custom organization policies](https://docs.cloud.google.com/organization-policy/overview#custom-organization-policies).

## About organization policies and constraints

The Google Cloud Organization Policy Service gives you centralized, programmatic
control over your organization's resources. As the
[organization policy administrator](https://docs.cloud.google.com/iam/docs/roles-permissions/orgpolicy#orgpolicy.policyAdmin), you can define an organization
policy, which is a set of restrictions called *constraints* that apply to
Google Cloud resources and descendants of those resources in the
[Google Cloud resource hierarchy](https://docs.cloud.google.com/resource-manager/docs/cloud-platform-resource-hierarchy). You can enforce organization
policies at the organization, folder, or project level.

Organization Policy provides built-in [managed constraints](https://docs.cloud.google.com/organization-policy/reference/org-policy-constraints)
for various Google Cloud services. However, if you want more granular,
customizable control over the specific fields that are restricted in your
organization policies, you can also create *custom constraints* and use those
custom constraints in an organization policy.

### Policy inheritance

By default, organization policies are inherited by the descendants of the
resources on which you enforce the policy. For example, if you enforce a policy
on a folder, Google Cloud enforces the policy on all projects in the
folder. To learn more about this behavior and how to change it, refer to
[Hierarchy evaluation rules](https://docs.cloud.google.com/organization-policy/hierarchy-evaluation#disallow_inheritance).

### Benefits


You can use a custom organization policy to allow or deny specific operations on
BigQuery resources such as datasets, tables, data access
policies, row access policies, and routines. This lets you control costs and
manage access to your Google Cloud resources to help you meet your
organization's compliance and security requirements. Restrictions on resources
are fine-grained. They can be applied at the project, folder, or organization
level.

## Limitations


- [`PolicyViolationInfo`](https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.Audit/latest/Google.Cloud.Audit.PolicyViolationInfo) isn't published in the [BigQuery audit
  logs](https://docs.cloud.google.com/bigquery/docs/reference/auditlogs) when access is denied due to custom constraints on resources other than datasets. The error message provides the `constraintId` that denied the operation.

## Before you begin

1. Ensure that you know your [organization ID](https://docs.cloud.google.com/resource-manager/docs/creating-managing-organization#retrieving_your_organization_id).

### Required roles


To get the permission that
you need to manage organization policies,

ask your administrator to grant you the
[Organization Policy Administrator](https://docs.cloud.google.com/iam/docs/roles-permissions/orgpolicy#orgpolicy.policyAdmin) (`roles/orgpolicy.policyAdmin`) IAM role on the organization resource.


For more information about granting roles, see [Manage access to projects, folders, and organizations](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).


This predefined role contains the
`orgpolicy.*`
permission,
which is required to
manage organization policies.


You might also be able to get
this permission
with [custom roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles) or
other [predefined roles](https://docs.cloud.google.com/iam/docs/roles-overview#predefined).
Additional permissions are required for creating and managing BigQuery resources. For more information, see [Predefined roles
and permissions](https://docs.cloud.google.com/bigquery/docs/access-control).

## BigQuery supported resources

The following table lists the BigQuery resources that you can reference in custom constraints.

<br />

| Resource | Field |
|---|---|
| bigquery.googleapis.com/Dataset | `resource.datasetReference.datasetId` |
| bigquery.googleapis.com/Dataset | `resource.defaultCollation` |
| bigquery.googleapis.com/Dataset | `resource.defaultEncryptionConfiguration.kmsKeyName` |
| bigquery.googleapis.com/Dataset | `resource.defaultPartitionExpirationMs` |
| bigquery.googleapis.com/Dataset | `resource.defaultRoundingMode` |
| bigquery.googleapis.com/Dataset | `resource.defaultTableExpirationMs` |
| bigquery.googleapis.com/Dataset | `resource.description` |
| bigquery.googleapis.com/Dataset | `resource.externalCatalogDatasetOptions.defaultStorageLocationUri` |
| bigquery.googleapis.com/Dataset | `resource.externalCatalogDatasetOptions.parameters` |
| bigquery.googleapis.com/Dataset | `resource.externalDatasetReference.connection` |
| bigquery.googleapis.com/Dataset | `resource.externalDatasetReference.externalSource` |
| bigquery.googleapis.com/Dataset | `resource.friendlyName` |
| bigquery.googleapis.com/Dataset | `resource.isCaseInsensitive` |
| bigquery.googleapis.com/Dataset | `resource.linkedDatasetSource.sourceDataset.datasetId` |
| bigquery.googleapis.com/Dataset | `resource.location` |
| bigquery.googleapis.com/Dataset | `resource.maxTimeTravelHours` |
| bigquery.googleapis.com/Dataset | `resource.storageBillingModel` |
| bigquery.googleapis.com/Routine | `resource.routineReference.datasetId` |
| bigquery.googleapis.com/Routine | `resource.routineReference.projectId` |
| bigquery.googleapis.com/Routine | `resource.routineReference.routineId` |
| bigquery.googleapis.com/RowAccessPolicy | `resource.rowAccessPolicyReference.datasetId` |
| bigquery.googleapis.com/RowAccessPolicy | `resource.rowAccessPolicyReference.policyId` |
| bigquery.googleapis.com/RowAccessPolicy | `resource.rowAccessPolicyReference.projectId` |
| bigquery.googleapis.com/RowAccessPolicy | `resource.rowAccessPolicyReference.tableId` |
| bigquery.googleapis.com/Table | `resource.tableReference.datasetId` |
| bigquery.googleapis.com/Table | `resource.tableReference.projectId` |
| bigquery.googleapis.com/Table | `resource.tableReference.tableId` |
| bigquerydatapolicy.googleapis.com/DataPolicy | `resource.dataMaskingPolicy.predefinedExpression` |
| bigquerydatapolicy.googleapis.com/DataPolicy | `resource.dataPolicyType` |

## Set up a custom constraint

A custom constraint is defined in a YAML file by the resources, methods,
conditions, and actions that are supported by the service on which you are
enforcing the organization policy. Conditions for your custom constraints are
defined using
[Common Expression Language (CEL)](https://github.com/google/cel-spec/blob/master/doc/intro.md). For more information about how to build
conditions in custom constraints using CEL, see the CEL section of
[Creating and managing custom constraints](https://docs.cloud.google.com/organization-policy/create-custom-constraints#common_expression_language).

### Console


To create a custom constraint, do the following:

1. In the Google Cloud console, go to the **Organization policies** page.

   [Go to Organization policies](https://console.cloud.google.com/iam-admin/orgpolicies)
2. From the project picker, select the project that you want to set the organization policy for.
3. Click **Custom constraint**.
4. In the **Display name** box, enter a human-readable name for the constraint. This name is used in error messages and can be used for identification and debugging. Don't use personally identifiable information (PII) or sensitive data in display names because this name could be exposed in error messages. This field can contain up to 200 characters.
5. In the **Constraint ID** box, enter the ID that you want for your new custom constraint. A custom constraint can only contain letters (including upper and lowercase) or numbers, for example `custom.enforceDatasetId`. This field can contain up to 70 characters, not counting the prefix (`custom.`), for example, `organizations/123456789/customConstraints/custom`. Don't include PII or sensitive data in your constraint ID, because it could be exposed in error messages.
6. In the **Description** box, enter a human-readable description of the constraint. This description is used as an error message when the policy is violated. Include details about why the policy violation occurred and how to resolve the policy violation. Don't include PII or sensitive data in your description, because it could be exposed in error messages. This field can contain up to 2000 characters.
7. In the **Resource type** box, select the name of the Google Cloud REST resource containing the object and field that you want to restrict---for example, `container.googleapis.com/NodePool`. Most resource types support up to 20 custom constraints. If you attempt to create more custom constraints, the operation fails.
8. Under **Enforcement method** , select whether to enforce the constraint on a REST `CREATE` method or both `CREATE` and `UPDATE` methods. If you enforce the constraint with the `UPDATE` method on a resource that violates the constraint, changes to that resource are blocked by the organization policy unless the change resolves the violation.
9. To see supported methods for each service, find the service in [Services that support custom constraints](https://docs.cloud.google.com/organization-policy/reference/custom-constraint-supported-services).
10. To define a condition, click **Edit condition**.
    1. In the **Add condition** panel, create a CEL condition that refers to a supported service resource, for example, `resource.management.autoUpgrade == false`. This field can contain up to 1000 characters. For details about CEL usage, see [Common Expression Language](https://docs.cloud.google.com/resource-manager/docs/organization-policy/creating-managing-custom-constraints#common_expression_language). For more information about the service resources you can use in your custom constraints, see [Custom constraint supported services](https://docs.cloud.google.com/resource-manager/docs/organization-policy/custom-constraint-supported-services).
    2. Click **Save**.
11. Under **Action**, select whether to allow or deny the evaluated method if the condition is met.
12. The deny action means that the operation to create or update the resource is blocked if the condition evaluates to true.
13. The allow action means that the operation to create or update the resource is permitted only if the condition evaluates to true. Every other case except those explicitly listed in the condition is blocked.
14. Click **Create constraint**.
15. When you have entered a value into each field, the equivalent YAML configuration for this custom constraint appears on the right.

### gcloud

1. To create a custom constraint, create a YAML file using the following format:

```yaml
name: organizations/ORGANIZATION_ID/customConstraints/CONSTRAINT_NAME
resourceTypes: RESOURCE_NAME
methodTypes:
  - CREATE
  - UPDATE 
condition: "CONDITION"
actionType: ACTION
displayName: DISPLAY_NAME
description: DESCRIPTION
```
2. Replace the following:
   - `ORGANIZATION_ID`: your organization ID, such as `123456789`.
   - `CONSTRAINT_NAME`: the name that you want for your new custom constraint. A custom constraint can only contain letters (including upper and lowercase) or numbers, for example, `custom.enforceDatasetId`. This field can contain up to 70 characters, not counting the prefix (`custom.`)--- for example, `organizations/123456789/customConstraints/custom`. Don't include PII or sensitive data in your constraint ID, because it could be exposed in error messages.
   - `RESOURCE_NAME`: the fully qualified name of the Google Cloud resource containing the object and field that you want to restrict. For example, `bigquery.googleapis.com/Dataset`. Most resource types support up to 20 custom constraints. If you attempt to create more custom constraints, the operation fails.
   - `methodTypes`: the REST methods that the constraint is enforced on. Can be `CREATE` or both `CREATE` and `UPDATE`. If you enforce the constraint with the `UPDATE` method on a resource that violates the constraint, changes to that resource are blocked by the organization policy unless the change resolves the violation.
   - To see the supported methods for each service, find the service in [Services that support custom constraints](https://docs.cloud.google.com/organization-policy/reference/custom-constraint-supported-services).
   - `CONDITION`: a [CEL condition](https://docs.cloud.google.com/resource-manager/docs/organization-policy/creating-managing-custom-constraints#common_expression_language) that is written against a representation of a supported service resource. This field can contain up to 1000 characters. For example, `"datasetReference.datasetId.startsWith('test')"`.
   - For more information about the resources available to write conditions against, see [Supported resources](https://docs.cloud.google.com/bigquery/docs/custom-constraints#supported_resources).
   - `ACTION`: the action to take if the `condition` is met. Possible values are `ALLOW` and `DENY`.
   - The allow action means that if the condition evaluates to true, the operation to create or update the resource is permitted. This also means that every other case except the one explicitly listed in the condition is blocked.
   - The deny action means that if the condition evaluates to true, the operation to create or update the resource is blocked.
   - `DISPLAY_NAME`: a human-readable name for the constraint. This name is used in error messages and can be used for identification and debugging. Don't use PII or sensitive data in display names because this name could be exposed in error messages. This field can contain up to 200 characters.
   - `DESCRIPTION`: a human-friendly description of the constraint to display as an error message when the policy is violated. This field can contain up to 2000 characters.
3. After you have created the YAML file for a new custom constraint, you must set it up to make it available for organization policies in your organization. To set up a custom constraint, use the [`gcloud org-policies set-custom-constraint`](https://docs.cloud.google.com/sdk/gcloud/reference/org-policies/set-custom-constraint) command:

```bash
gcloud org-policies set-custom-constraint CONSTRAINT_PATH
```
4. Replace `CONSTRAINT_PATH` with the full path to your custom constraint file. For example, `/home/user/customconstraint.yaml`.
5. After this operation is complete, your custom constraints are available as organization policies in your list of Google Cloud organization policies.
6. To verify that the custom constraint exists, use the [`gcloud org-policies list-custom-constraints`](https://docs.cloud.google.com/sdk/gcloud/reference/org-policies/list-custom-constraints) command:

```bash
gcloud org-policies list-custom-constraints --organization=ORGANIZATION_ID
```
7. Replace `ORGANIZATION_ID` with the ID of your organization resource.
8. For more information, see [Viewing organization policies](https://docs.cloud.google.com/resource-manager/docs/organization-policy/creating-managing-policies#viewing_organization_policies).

## Enforce a custom organization policy

You can enforce a constraint by creating an organization policy that references it, and then applying that organization policy to a Google Cloud resource.

### Console

1. In the Google Cloud console, go to the **Organization policies** page.

   [Go to Organization policies](https://console.cloud.google.com/iam-admin/orgpolicies)
2. From the project picker, select the project that you want to set the organization policy for.
3. From the list on the **Organization policies** page, select your constraint to view the **Policy details** page for that constraint.
4. To configure the organization policy for this resource, click **Manage policy**.
5. On the **Edit policy** page, select **Override parent's policy**.
6. Click **Add a rule**.
7. In the **Enforcement** section, select whether this organization policy is enforced or not.
8. Optional: To make the organization policy conditional on a tag, click **Add condition** . Note that if you add a conditional rule to an organization policy, you must add at least one unconditional rule or the policy cannot be saved. For more information, see [Scope organization policies with tags](https://docs.cloud.google.com/organization-policy/scope-policies).
9. Click **Test changes** to simulate the effect of the organization policy. For more information, see [Test organization policy changes with Policy Simulator](https://docs.cloud.google.com/policy-intelligence/docs/test-organization-policies).
10. To enforce the organization policy in dry-run mode, click **Set dry run policy** . For more information, see [Test organization policies](https://docs.cloud.google.com/organization-policy/test-policies).
11. After you verify that the organization policy in dry-run mode works as intended, set the live policy by clicking **Set policy**.

### gcloud

1. To create an organization policy with boolean rules, create a policy YAML file that references the constraint:

```yaml
name: projects/PROJECT_ID/policies/CONSTRAINT_NAME
spec:
  rules:
  - enforce: true

dryRunSpec:
  rules:
  - enforce: true
```
2. Replace the following:
   - `PROJECT_ID`: the project that you want to enforce your constraint on.
   - `CONSTRAINT_NAME`: the name you defined for your custom constraint. For example, `custom.enforceDatasetId`.
3. To enforce the organization policy in [dry-run mode](https://docs.cloud.google.com/organization-policy/test-policies), run the following command with the `dryRunSpec` flag:

```bash
gcloud org-policies set-policy POLICY_PATH --update-mask=dryRunSpec
```
4. Replace `POLICY_PATH` with the full path to your organization policy YAML file. The policy requires up to 15 minutes to take effect.
5. After you verify that the organization policy in dry-run mode works as intended, set the live policy with the `org-policies set-policy` command and the `spec` flag:

```bash
gcloud org-policies set-policy POLICY_PATH --update-mask=spec
```
6. Replace `POLICY_PATH` with the full path to your organization policy YAML file. The policy requires up to 15 minutes to take effect.

## Test the custom organization policy


The following example creates a custom constraint and policy that forbids all
new dataset IDs in a specific project from beginning with `test`.

Before you begin, you must have the following:

- Your organization ID
- A project ID

### Create the constraint

To create a custom constraint, follow these steps:

1. Create the following YAML file and save it as
   `constraint-enforce-datasetId.yaml`:

       name: organizations/ORGANIZATION_ID/customConstraints/custom.enforceDatasetId
       resourceTypes:
       - bigquery.googleapis.com/Dataset
       methodTypes:
       - CREATE
       condition: "datasetReference.datasetId.startsWith('test')"
       actionType: DENY
       displayName: Reject test datasets.
       description: Deny new dataset names that begin with 'test'.

   Replace `ORGANIZATION_ID` with your organization ID.

   This defines a constraint that denies the creation of new datasets if the
   dataset name begins with 'test'.
2. Apply the constraint:

       gcloud org-policies set-custom-constraint ~/constraint-enforce-datasetId

3. Verify that the constraint exists:

       gcloud org-policies list-custom-constraints --organization=ORGANIZATION_ID

   The output is similar to the following:

       CUSTOM_CONSTRAINT                       ACTION_TYPE  METHOD_TYPES   RESOURCE_TYPES                     DISPLAY_NAME
       custom.enforceDatasetId                 DENY         CREATE         bigquery.googleapis.com/Dataset    Reject test datasets
       ...

### Create the policy

Now create a policy and apply it to the custom constraint that you created.

1. Save the following file as `policy-enforce-datasetId.yaml`:

       name: projects/PROJECT_ID/policies/custom.enforceDatasetId
       spec:
         rules:
         - enforce: true

   Replace `PROJECT_ID` with your project ID.
2. Apply the policy:

       gcloud org-policies set-policy ~/policy-enforce-datasetId.yaml

3. Verify that the policy exists:

       gcloud org-policies list --project=PROJECT_ID

   The output is similar to the following:

       CONSTRAINT                  LIST_POLICY    BOOLEAN_POLICY    ETAG
       custom.enforceDatasetId     -              SET               COCsm5QGENiXi2E=

After you apply the policy, it can take up to two minutes for Google Cloud
to start enforcing it.

### Test the policy

Try to create a BigQuery dataset in the project:

    bq --location=US mk -d \
        --default_table_expiration 3600 \
        --description "This is my dataset." \
        testdataset

The output is the following:

    Operation denied by custom org policies: ["customConstraints/custom.enforceDatasetId": "All new datasets can't begin with 'test'."]

## Example custom organization policies for common use cases

This table provides syntax examples for some common custom constraints.

| Description | Constraint syntax |
|---|---|
| Deny new tables with names that begin with 'test'. | ```yaml name: organizations/ORGANIZATION_ID/customConstraints/custom.enforceTableId resourceTypes: - bigquery.googleapis.com/Table methodTypes: - CREATE condition: "resource.tableReference.tableId.startsWith('test')" actionType: DENY displayName: Reject test tables. description: Deny new table names that begin with 'test'. ``` |
| Deny new row access policies with names that begin with 'test'. | ```yaml name: organizations/ORGANIZATION_ID/customConstraints/custom.enforceRowAccessPolicyId resourceTypes: - bigquery.googleapis.com/RowAccessPolicies methodTypes: - CREATE condition: "resource.rowAccessPolicyReference.policyId.startsWith('test')" actionType: DENY displayName: Reject 'test' row access policies. description: Deny new row access policies with names that begin with 'test'. ``` |
| Deny new data access policies with predefined expressions that begin with 'SHA256' (Secure Hash Algorithm 256-bit). | ```yaml name: organizations/ORGANIZATION_ID/customConstraints/custom.enforcePredefinedExpression resourceTypes: - bigquerydatapolicy.googleapis.com/DataPolicy methodTypes: - CREATE condition: "resource.dataMaskingPolicy.predefinedExpression.startsWith('SHA256')" actionType: DENY displayName: Reject SHA256 data policies. description: Deny new data access policies with predefined expressions that begin with 'SHA256'. expression. ``` |
| Deny new routines with names that begin with 'test'. | ```yaml name: organizations/ORGANIZATION_ID/customConstraints/custom.enforceRoutineId resourceTypes: - bigquery.googleapis.com/Routine methodTypes: - CREATE condition: "resource.routineReference.routineId.startsWith('test')" actionType: DENY displayName: Reject test routines. description: Deny new routines with names that begin with 'test'. ``` |

## What's next

- Learn more about [Organization Policy Service](https://docs.cloud.google.com/organization-policy/overview).
- Learn more about how to [create and manage organization policies](https://docs.cloud.google.com/organization-policy/create-organization-policies).
- See the full list of managed [organization policy constraints](https://docs.cloud.google.com/organization-policy/reference/org-policy-constraints).