# VPC Service Controls for BigQuery

This page explains how to enhance security around BigQuery resources by
creating perimeters with VPC Service Controls. These perimeters restrict access to
and from BigQuery and are independent from
Identity and Access Management (IAM) controls. They're useful in the following use cases:

- Preventing data leakage by restricting access to resources, except those specifically allowed in the ingress and egress rules.
- Securely loading data into BigQuery from third-party sources or Google Cloud services, such as Cloud Storage.
- Controlling data export from BigQuery to Cloud Storage or other targets.

For more information, see the [overview of VPC Service Controls](https://docs.cloud.google.com/vpc-service-controls/docs/overview).

## Before you begin

- To get the permissions that you need to configure service perimeters, see [Access control with
  IAM](https://docs.cloud.google.com/vpc-service-controls/docs/access-control) for VPC Service Controls.
- You must have an access policy for your organization. For more information, see [Create an access
  policy](https://docs.cloud.google.com/access-context-manager/docs/create-access-policy).

## Create the VPC Service Controls perimeter

The following example shows how to create a VPC Service Controls perimeter
that limits the range of external IP addresses that can access a
BigQuery project.

1. Create an *access level* that only allows access to a specified range of IP
   addresses---for example, those within a corporate network. To create it, use
   the [`gcloud access-context-manager levels create`](https://docs.cloud.google.com/sdk/gcloud/reference/access-context-manager/levels/create) command:

       echo """
       - ipSubnetworks:
         - 162.222.181.0/24
         - 2001:db8::/48
       """ > level.yaml

       gcloud access-context-manager levels create ACCESS_LEVEL_NAME \
           --title="TITLE" --basic-level-spec=level.yaml

   Replace the following:
   - `ACCESS_LEVEL_NAME`: the ID of the access level
   - `TITLE`: the human-readable title for the service perimeter

   For more information about creating access levels, see the [example
   implementations](https://docs.cloud.google.com/access-context-manager/docs/create-basic-access-level#example_implementations).
2. Protect the BigQuery resource by creating or updating a
   perimeter. The following examples protect a project. For other use
   cases, such as protecting data transfer from a Cloud Storage
   bucket in another project, see the [use cases](https://docs.cloud.google.com/bigquery/docs/vpc-sc#use-cases).

   ### Create perimeter


   To create a new perimeter to protect the BigQuery project,
   use the [`gcloud access-context-manager perimeters create`](https://docs.cloud.google.com/sdk/gcloud/reference/access-context-manager/perimeters/create) command:

       echo """
       - ingressFrom:
           identityType: ANY_IDENTITY
           sources:
           - accessLevel: accessPolicies/POLICY_NAME/accessLevels/ACCESS_LEVEL_NAME
         ingressTo:
           operations:
           - methodSelectors:
             - method: '*'
             serviceName: bigquery.googleapis.com
           resources:
           - '*'

       """ > ingress.yaml

       gcloud access-context-manager perimeters create BIGQUERY_PERIMETER --title="TITLE" \
           --resources=BIGQUERY_PROJECT_NUMBER \
           --restricted-services=bigquery.googleapis.com \
           --ingress-policies=ingress.yaml
           --policy=POLICY_NAME

   Replace the following:
   - `POLICY_NAME`: the ID of the access policy
   - `ACCESS_LEVEL_NAME`: the ID of the access level
   - `PERIMETER`: the ID of the perimeter
   - `TITLE`: the short, human-readable title for the service perimeter
   - `BIGQUERY_PROJECT_NUMBER`: the ID of BigQuery project
   - `POLICY_NAME`: the ID of the access policy

   ### Update perimeter


   To update an existing perimeter, use the [`gcloud access-context-manager perimeters update`](https://docs.cloud.google.com/sdk/gcloud/reference/access-context-manager/perimeters/update) command:

       gcloud access-context-manager perimeters update BIGQUERY_PERIMETER --set-ingress-policies=ingress.yaml

   Replace `BIGQUERY_PERIMETER` with the ID of the perimeter
   protecting the BigQuery resource.

## Test the perimeter

Test your VPC Service Controls perimeter before enforcing it. For more
information, see [Dry run mode for service
perimeters](https://docs.cloud.google.com/vpc-service-controls/docs/dry-run-mode) and [Using dry-run
mode to test ingress or egress
policies](https://docs.cloud.google.com/vpc-service-controls/docs/ingress-egress-rules#using-dryrun-ingress-egress-rules).

## Use cases

The following use case examples show how to protect data going in and out of
BigQuery with VPC Service Controls.

### Query external table data from a Cloud Storage bucket in another project

The following examples show how to selectively allow communication between the
BigQuery and Cloud Storage projects when they are
separated by perimeters.

1. Allow the BigQuery project to access the
   Cloud Storage project by updating the egress rules for the perimeter
   around the Cloud Storage project:

       echo """
       - egressFrom:
           identityType: ANY_IDENTITY
         egressTo:
           operations:
           - methodSelectors:
             - method: '*'
             serviceName: storage.googleapis.com
           resources:
           - projects/BIGQUERY_PROJECT_NUMBER
       """ > egress.yaml

       gcloud access-context-manager perimeters update CLOUD_STORAGE_PERIMETER --policy=POLICY_NAME --set-egress-policies=egress.yaml

   Replace the following:
   - `BIGQUERY_PROJECT_NUMBER`: the ID of BigQuery project
   - `CLOUD_STORAGE_PERIMETER`: the ID of the perimeter protecting the Cloud Storage resources
   - `POLICY_NAME`: the ID of the access policy
2. Allow the Cloud Storage project to access the
   BigQuery project by updating the egress rules for the
   perimeter around the BigQuery project:

       echo """
       - egressFrom:
           identityType: ANY_IDENTITY
         egressTo:
           operations:
           - methodSelectors:
             - method: '*'
             serviceName: storage.googleapis.com
           resources:
           - projects/CLOUD_STORAGE_PROJECT_NUMBER
       """ > egress1.yaml

       gcloud access-context-manager perimeters update BIGQUERY_PERIMETER --policy=POLICY_NAME --set-egress-policies=egress1.yaml

   Replace the following:
   - `CLOUD_STORAGE_PROJECT_NUMBER`: the ID of Cloud Storage project
   - `PERIMETER`: the ID of the perimeter
   - `POLICY_NAME`: the ID of the access policy
3. Optional: if the perimeter protecting the BigQuery project
   includes `storage.googleapis.com` as a restricted service, you must update
   the ingress rule:

       echo """
       - ingressFrom:
           identityType: ANY_IDENTITY
           sources:
           - accessLevel: accessPolicies/POLICY_NAME/accessLevels/ACCESS_LEVEL_NAME
         ingressTo:
           operations:
           - methodSelectors:
             - method: '*'
             serviceName: bigquery.googleapis.com
           - methodSelectors:
             - method: '*'
             serviceName: storage.googleapis.com
           resources:
           - '*'

       """ > ingress.yaml

       gcloud access-context-manager perimeters create BIGQUERY_PERIMETER --title="TITLE" \
           --resources=BIGQUERY_PROJECT_NUMBER \
           --restricted-services=bigquery.googleapis.com \
           --ingress-policies=ingress.yaml
           --policy=POLICY_NAME

### Import and export data from BigQuery Omni

As an extra layer of defense, you can use VPC Service Controls perimeters to
restrict access between BigQuery Omni and an external cloud
service. For more information and examples, see the
[VPC Service Controls](https://docs.cloud.google.com/bigquery/docs/omni-azure-create-external-table#vpc-service)
configuration for when you create an Azure Blob Storage BigLake table.

## What's next

- Learn more about [VPC Service Controls in BigQuery sharing](https://docs.cloud.google.com/bigquery/docs/analytics-hub-vpc-sc-rules).
- Learn how to [restrict BigQuery Omni access with an
  external cloud
  service](https://docs.cloud.google.com/bigquery/docs/omni-azure-create-external-table#vpc-service).
- Understand [risks and mitigation through
  VPC Service Controls](https://cloud.google.com/security/vpc-service-controls).
- Learn more about [VPC Service Controls support and
  limitations in BigQuery](https://docs.cloud.google.com/vpc-service-controls/docs/supported-products#table_bigquery).
- [Troubleshoot](https://docs.cloud.google.com/vpc-service-controls/docs/troubleshooting#debugging) common issues for BigQuery and VPC Service Controls.