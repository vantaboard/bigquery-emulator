# Introduction to security and access controls in BigQuery

This document provides an overview of access controls in BigQuery
using Identity and Access Management (IAM). IAM lets you grant granular
access to specific BigQuery resources and helps prevent access to other
resources. IAM helps you apply the security principle of least
privilege, which states that no [IAM principal](https://docs.cloud.google.com/iam/docs/principals-overview)
should have more permissions than they actually need.

When an IAM principal such as a
user, group, or service account calls a Google Cloud API, that principal must
have the minimum IAM permissions necessary to use the resource.
To give a principal the required permissions, you grant an IAM
role to the principal.

This document describes how predefined and custom IAM roles can
be used to allow principals to access BigQuery resources.

To familiarize yourself with how access is managed in Google Cloud, see
[IAM overview](https://docs.cloud.google.com/iam/docs/overview).

## IAM role types

A role is a collection of permissions that can be granted to an IAM
principal. You can use the following types of roles
in IAM to grant access to BigQuery resources:

- [**Predefined roles**](https://docs.cloud.google.com/iam/docs/roles-overview#predefined) are managed by Google Cloud and support common use cases and access control patterns.
- [**Custom roles**](https://docs.cloud.google.com/iam/docs/understanding-custom-roles) provide access according to a user-specified list of permissions. For information on creating custom roles, see [Create and manage custom roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles) in the IAM documentation.

> [!NOTE]
> **Note:** When new capabilities are added to BigQuery, new permissions might be added to predefined IAM roles. Also, new predefined IAM roles can be added to BigQuery at any time. If your organization requires role definitions to remain unchanged, you should create [custom IAM roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles).

To determine if one or more permissions are included in a predefined
IAM role, you can use one of the following methods:

- The [BigQuery IAM roles and permissions](https://docs.cloud.google.com/bigquery/docs/access-control) reference
- The [IAM roles and permissions index](https://docs.cloud.google.com/iam/docs/roles-permissions)
- The [`gcloud iam roles describe`](https://docs.cloud.google.com/sdk/gcloud/reference/iam/roles/describe) command
- The [`roles.get()`](https://docs.cloud.google.com/iam/reference/rest/v1/roles/get) method in the IAM API

## IAM roles in BigQuery

Permissions are not assigned directly to users, groups, or service accounts.
Instead, users, groups, or service accounts are granted one or more predefined
or custom roles that grant them permissions to perform actions on resources. You
grant these roles on a particular resource, but they also apply to all of that
resource's descendants in the [resource hierarchy](https://docs.cloud.google.com/resource-manager/docs/cloud-platform-resource-hierarchy).

When you assign multiple role types to a user, the permissions granted are a
union of each role's permissions.

You can grant access to the following BigQuery resources:

- Datasets and these resources within datasets:
  - Tables and views
  - Routines
- Connections
- Saved queries
- Data canvases
- Data preparations
- Pipelines
- Repositories

### Grant access to Resource Manager resources

You can configure access to BigQuery resources through
Resource Manager by granting a BigQuery role to a principal and
then by granting that role on an organization, a folder, or a project.

When you grant roles to Resource Manager resources such as organizations and
projects, you're granting permissions on **all** of the BigQuery
resources in the organization or project.

For additional information on using IAM to manage access to
Resource Manager resources, see
[Manage access to projects, folders, and organizations](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access)
in the IAM documentation.

### Grant access to datasets

You can assign roles at the dataset level to provide access to a specific
dataset, without providing complete access to the project's other resources. In
the [IAM resource hierarchy](https://docs.cloud.google.com/iam/docs/overview#policy_hierarchy),
BigQuery datasets are child resources of projects. For more information
on assigning roles at the dataset level, see
[Control access to resources with IAM](https://docs.cloud.google.com/bigquery/docs/control-access-to-resources-iam).

> [!CAUTION]
> **Caution:** Don't grant BigQuery basic roles to datasets. BigQuery's dataset-level basic roles existed prior to the introduction of IAM. BigQuery basic roles provide excessive and uneven access, and you are discouraged from using them. For example, the `Owner` basic role does *not* provide table access permissions. For more information, see [Basic roles \& permissions](https://docs.cloud.google.com/bigquery/docs/access-control-basic-roles).

### Grant access to individual resources within datasets

You can grant roles access to certain types of resources within datasets,
without providing complete access to the dataset's resources.

Roles can be applied to the following resources within datasets:

- Tables and views
- Routines

> [!NOTE]
> **Note:** Roles cannot be applied to models.

For more information on assigning roles at the table, view, or routine level,
see [Control access to resources with IAM](https://docs.cloud.google.com/bigquery/docs/control-access-to-resources-iam).

## What's next

- For more information about assigning roles to BigQuery resources, see [Control access to resources with
  IAM](https://docs.cloud.google.com/bigquery/docs/control-access-to-resources-iam).
- For a list of BigQuery predefined IAM roles and permissions, see [BigQuery IAM roles and
  permissions](https://docs.cloud.google.com/bigquery/docs/access-control).