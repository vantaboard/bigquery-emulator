# Best practices for multi-tenant workloads on BigQuery

This document provides techniques and best practices for common patterns that
are used in multi-tenant data platforms and enterprise data marts.

Commercial enterprises, software as a service (SaaS) vendors, and government
organizations must often securely host both internal and third-party data across
geographic and administrative boundaries. [BigQuery](https://docs.cloud.google.com/bigquery)
is a powerful tool that can consistently address multi-tenant platform
requirements for exabytes of data and hundreds of thousands of data consumers
across disparate business units. This document is for organizations that deploy
multi-tenant platforms on BigQuery and who want to understand the
available
[access controls](https://docs.cloud.google.com/bigquery/docs/data-governance)
and
[performance management features](https://docs.cloud.google.com/bigquery/docs/best-practices-performance-overview).

Multi-tenant platform builders often need to balance considerations for the
following:

- **Data isolation**: Implement strong controls to prevent data leakage across tenants.
- **Consistent performance** : Configure and apportion [BigQuery reservations](https://docs.cloud.google.com/bigquery/docs/reservations-intro) to maintain consistent performance across tenants.
- **Resource management**: Plan for the impact of quotas and limits.
- **Geographic distribution** : Locate data in designated and required [geographic locations](https://docs.cloud.google.com/bigquery/docs/locations#multi-regions). For compliance-related concerns, see Google Cloud's [compliance offerings](https://cloud.google.com/security/compliance/offerings).
- **Auditing and security**: Safeguard tenant data from inappropriate access and exfiltration.
- **Cost management**: Ensure consistent BigQuery costs to host each tenant.
- **Operational Complexity**: Minimize the amount of system variability that is required to host new tenants.

## SaaS vendor with shared tenant infrastructure

SaaS vendors who host third-party data need to ensure the reliability and
isolation of their entire fleet of customers. Those customers might number in
the tens of thousands, and customer data might be accessed through shared
services infrastructure. Some SaaS vendors also maintain a sanitized, central
datastore to do analytics across their entire fleet of tenants.

A dataset-per-tenant design helps to mitigate the following concerns that an
organization experiences when it scales to thousands of tenants:

- **Administrative complexity**: the total number of new projects and cloud resources on a per customer basis
- **End-to-end latency**: how up to date the datastore is for both the tenants and cross-customer analytics solutions
- **Performance expectations**: ensuring that tenant performance stays within acceptable limits

### Configure datasets for each tenant

Within a project that is dedicated to storing customer data, each customer's
data is separated by BigQuery datasets. Within the host
organization, you use a second project to deploy analytics and machine learning
on combined customer data. You can then configure the data processing engine,
[Dataflow](https://docs.cloud.google.com/dataflow),
to dual-write incoming data to the internal and tenant-specific tables. The
Dataflow configuration uses fully written tables instead of
authorized views. Using fully written tables allows uniform handling of
geographic distribution and helps to avoid reaching
[authorized view limits](https://docs.cloud.google.com/bigquery/quotas#view_limits)
when the number of tenants scales.

BigQuery's separation of storage and
[compute](https://docs.cloud.google.com/bigquery/docs/slots)
lets you configure fewer projects compared to cluster-based warehouses to handle
problems such as service tiers and data isolation. When you don't need to
configure tenants with additional
[dedicated cloud](https://docs.cloud.google.com/bigquery/docs/best-practices-for-multi-tenant-workloads-on-bigquery#saas-dedicated)
resources, we recommend that you consider default configuration of a dedicated
dataset for each tenant. The following diagram shows an example project
configuration based on this recommended design:

![A default configuration with dedicated projects for each tenant.](https://docs.cloud.google.com/static/bigquery/images/best-practices-for-multi-tenant-workloads-on-bigquery-saas.svg)

**Figure 1.** A constant number of projects handles data and processing needs as
the number of tenants grows.

The project configuration in figure 1 includes the following projects:

- **Data pipeline project**: the core infrastructure components that receive, process, and distribute tenant data are all packaged into a single project.
- **Combined tenant data project**: the core data project that maintains a dataset per customer. Tenant data is expected to be accessed through compute tier projects.
- **Internal development projects**: projects that represent the self-managed resources that analytics teams use to evaluate tenant data and build new features.
- **End-user application projects** : projects that contain resources that are designed to interact with end users. We recommend that you use tenant-scoped service accounts to access tenant datasets and use a robust and secure [build pipeline](https://docs.cloud.google.com/kubernetes-engine/docs/tutorials/gitops-cloud-build) to deploy applications.
- **Reservation compute tier projects**: the projects that map tenant query activity to BigQuery reservations.

### Share reservations

Reservations in this approach rely on the
[fair scheduling](https://docs.cloud.google.com/bigquery/docs/slots#fair_scheduling_in_bigquery)
algorithm that is built into BigQuery reservations. Each compute
tier reservation is assigned to a single project. Tenant queries use fair
scheduling slots that are available to each compute tier project, and unused
slots from any tier are automatically reused in another tier. If a tenant has
specific timing requirements, you can use a project-reservation pair that is
dedicated to providing an exact number of slots.

### Configure VPC Service Controls perimeters

In this configuration, we recommend
VPC Service Controls perimeters to prevent accidental exposure of tenant
datasets outside of your Google Cloud organization and to prevent
unauthorized data joining within the organization.

#### Perimeters

In this configuration, we recommend that you create the following
[service perimeters](https://docs.cloud.google.com/vpc-service-controls/docs/create-service-perimeters):

- **Data pipeline**: a perimeter around the data pipeline projects should enforce all services that don't need to receive tenant data.
- **Tenant data**: a perimeter around the tenant dataset project and around the tenant BigQuery compute projects. Enforce all services to prevent access from outside of the organization.
- **Internal applications** : enforce all services and use [access levels](https://docs.cloud.google.com/vpc-service-controls/docs/use-access-levels#using_access_levels) to grant resource access to department teams.
- **External applications**: a perimeter around your SaaS applications. Enforce all services that aren't necessary for the applications to function.

#### Perimeter bridges

In this configuration, we recommend that you create the following
[perimeter bridges](https://docs.cloud.google.com/vpc-service-controls/docs/create-perimeter-bridges):

- **Data pipeline and tenant data**: allow the pipeline to write data into tenant datasets.
- **Data pipeline and internal applications**: allow the pipeline to write data into the cross-customer dataset.
- **External applications and tenant data**: allow the external-facing applications to query tenant data.
- **External applications and internal applications**: allow external-facing applications to process data using models that the internal applications develop and deploy.

### SaaS vendor with dedicated tenant infrastructure

In more complex scenarios, SaaS vendors might deploy dedicated compute
infrastructure for each tenant. In this scenario, dedicated infrastructure is
responsible for serving requests for tenant data inside
BigQuery.

A dedicated tenant infrastructure design addresses the following common
concerns when deploying infrastructure for each tenant alongside
BigQuery:

- **Billing accountability**: tracking infrastructure costs associated with each onboarded tenant.
- **End-to-end latency**: how up to date the datastore is for both the tenants and cross-customer analytics solutions.
- **Performance expectations**: ensuring tenant performance stays within acceptable limits.

#### Colocate datasets with dedicated resources

When you deploy dedicated tenant infrastructure, we recommend that you create a
parent
[folder](https://docs.cloud.google.com/resource-manager/docs/cloud-platform-resource-hierarchy#folders)
for the tenant-specific projects. Then colocate the tenant's
BigQuery datasets in projects with the dedicated resources that
access that data on behalf of the tenant. To minimize end-to-end latency for
tenant data, Dataflow pipelines insert data directly into the
tenant datasets.

This design handles upstream data processing and distribution, similar to the
preceding shared infrastructure design. However, tenant data and applications
that access tenant data are organized under tenant-specific projects (and
optionally organized under tenant-dedicated folders) to simplify the billing and
resource management. The following diagram shows an example project
configuration based on this recommended design:

![A project configuration that has tenant-specific projects.](https://docs.cloud.google.com/static/bigquery/images/best-practices-for-multi-tenant-workloads-on-bigquery-saas-dedicated-infrastructure.svg)

**Figure 2.** A data pipelines project handles data for a single tenant from
several other projects.

The project configuration in figure 2 includes the following projects:

- **Data pipelines project**: the core infrastructure components that receive, process, and distribute tenant data are all packaged into a single project.
- **Dedicated tenant projects** : projects that contain all cloud resources that are dedicated to a single tenant, including BigQuery datasets. We recommend that you use [Identity and Access Management (IAM)](https://docs.cloud.google.com/iam) to greatly limit the scope of which accounts and service accounts can access the customer datasets.
- **Internal analytics projects**: projects that represent the self-managed resources that analytics teams use to evaluate tenant data and build new features.
- **External networking project**: project that handles and routes tenant requests to their dedicated backends.

#### Share reservations

Reservations in this approach rely on the fair scheduling algorithm that is
built into BigQuery reservations. In this configuration, compute
tier reservations are assigned to each tenant project that uses that tier. If a
tenant has specific timing requirements, you can create a dedicated reservation
to provide a precise number of slots to a tenant project.

#### Use IAM controls and disable key creation

VPC Service Controls perimeters might not be appropriate for this scenario.
Project-related
[limits](https://docs.cloud.google.com/vpc-service-controls/quotas#limits)
prevent an organization from using perimeter boundaries around projects that
interact with the tenant projects. Instead, we recommend that you use strict IAM
controls and
[disable key creation](https://docs.cloud.google.com/resource-manager/docs/organization-policy/restricting-service-accounts#disable_service_account_key_creation)
to help protect against undesired external access.

## Data mart with central authority

Data marts are a common design theme in which core analytics data is stored in a
central repository and subsets are shared along lines of business. Data marts
frequently have dozens or hundreds of tenants, represented as lines of business
to consider.

A data mart design in BigQuery addresses the following needs:

- **Secure data collaboration**: sharing data with technical controls to minimize inappropriate access across teams.
- **Central data governance**: ensuring core data assets used for critical business reports are standardized and validated.
- **Business unit cost attribution**: tracking and adjusting computation usage by business units.

### Use a centrally administered repository

In this design, core data is captured in a centrally administered repository.
Authorized views,
[authorized user-defined functions (UDFs)](https://docs.cloud.google.com/bigquery/docs/user-defined-functions#authorize_routines),
and column policies are frequently used together to share data with lines of
business while preventing accidental distribution of sensitive data.

Teams in tenant projects can access centrally governed datasets based on their
account permissions. Teams use slots allocated to their own projects to build
reports and derived datasets. The core data team uses authorized views to
maintain full ownership of the access control to the data mart's assets. In this
configuration, we recommend that you avoid building multiple layers of views on
top of the objects that the core data project presents. The following diagram
shows an example project configuration based on this recommended design:

![A project configuration that uses a centrally administered repository.](https://docs.cloud.google.com/static/bigquery/images/best-practices-for-multi-tenant-workloads-on-bigquery-data-mart-tenants.svg)

**Figure 3.** A core data project maintains a centralized data mart that is
accessible from across the organization.

The project configuration in figure 3 includes the following projects:

- **Core data project**: the governance perimeter for managing access to core data and the data mart views. You maintain authorized views within datasets inside this project and grant authorized views to your analytics teams based on group membership.
- **[Extract, transform, load (ETL)](https://wikipedia.org/wiki/Extract,_transform,_load)
  infrastructure**: infrastructure for processing upstream data sources into the core data. Depending on administrative separation needs, you might choose to deploy the ETL infrastructure as its own project or as a part of the core data project.
- **Analytics team projects**: consumers of the data mart use these projects, and use their own provisioned infrastructure access to process data within the data mart. Analytics team projects are expected to be able to build derived datasets for local use.

### Use a two-tier reservation design

When working with data marts, we recommend that you use a two-tier design. In a
two-tier design, you assign the
[organization resource](https://docs.cloud.google.com/resource-manager/docs/cloud-platform-resource-hierarchy#organizations)
a reservation that has a small number of slots to cover general usage. If teams
have greater needs, assign reservations at the project or folder level.

### Configure VPC Service Controls perimeters

In this configuration, we recommend VPC Service Controls perimeters to
prevent accidental exposure of BigQuery datasets outside of your
Google Cloud organization.

#### Perimeters

In this configuration, we recommend that you create the following service
perimeters:

- **Core data**: a perimeter to protect the data warehouse and data mart datasets.
- **Data pipelines**: a perimeter for the ETL infrastructure project. If the data pipelines need to make requests outside of your Google Cloud organization, we recommend that you separate this perimeter from the core data perimeter.
- **Analytics**: a perimeter to build and deploy analytics assets that are internal to your organization. This perimeter is expected to have a more permissive access policy than the core data perimeter that it is bridged with.

#### Perimeter bridges

In this configuration, we recommend that you create the following perimeter
bridges:

- **Data pipelines and core data**: allow data pipelines to write into the core data project.
- **Core data and analytics**: allow users in the analytics projects to query the authorized views.

### Copy datasets for multiregional configurations

Because BigQuery disallows cross-regional queries, you can't use
the strategy of segmenting data with authorized views when data marts must exist
across multiple regions. Instead, you can use
[BigQuery Data Transfer Service](https://docs.cloud.google.com/bigquery/docs/dts-introduction) to copy relevant datasets into
another region. In this scenario, you create column policies within the data
catalog for each additional region that the data resides in. The following
diagram shows a multiregional configuration:

![A multiregional project configuration uses column policies.](https://docs.cloud.google.com/static/bigquery/images/best-practices-for-multi-tenant-workloads-on-bigquery-mutilregional-data-mart.svg)

**Figure 4.** A multiregional configuration uses the BigQuery Data Transfer Service
to copy datasets across regions.

The project configuration in figure 4 includes the following projects.

- **Core data project**: the governance perimeter for managing access to core data and the data mart views. Data is copied and maintained into regional datasets that can serve teams globally.
- **ETL infrastructure**: infrastructure for processing upstream data sources into the core data. Depending on administrative separation needs, you might choose to deploy the ETL infrastructure as its own project or as a part of the core data project.
- **Analytics team projects**: consumers of the data mart use these projects, and use their own provisioned infrastructure to process data within regional datasets of the data mart. Analytics team projects are expected to be able to build derived datasets for local use.

[BigQuery Data Transfer Service](https://docs.cloud.google.com/bigquery/docs/dts-introduction) is an additional scheduled
component with some limitations. The built-in service scheduler is limited to a
minimum interval time of 15 minutes and it must copy all the tables from the
source dataset. There is no way to embed additional scripts to create
region-specific data subsets into the BigQuery Data Transfer Service scheduler.

If your organization needs more flexibility, the following options are
available:

- **Managed Service for Apache Airflow jobs** : you can schedule [Managed Service for Apache Airflow](https://docs.cloud.google.com/composer) jobs to issue ETL jobs that create regional subsets before triggering the BigQuery Data Transfer Service through its [client API](https://docs.cloud.google.com/bigquery/docs/reference/libraries). If your organization can support additional latency, we recommend this option.
- **ETL infrastructure**: ETL infrastructure, such as Dataflow, can dual-write regional subsets into target regions. If your organization requires minimal data latency between regions, we recommend this option.

## Data marts with decentralized authority

Use decentralized authority when you need administrative separation by system
owner, lines of business, or geographical concerns.

A decentralized data mart has the following different concerns compared to a
standard data mart:

- **Secure data collaboration**: sharing data with technical controls to minimize inappropriate access across teams.
- **Data discoverability**: teams need to be able to discover and request access to datasets.
- **Data provenance**: without a central curator team, teams need to be able to trust the origin of data that goes into their analytics products.

### Delegate core data administration

This design is different from a conventional data mart approach because
decentralized authority delegates core data administration decisions to
component subgroups of the organization. When you use decentralized authority,
you maintain central control of security and BigQuery capacity by
using [Cloud Key Management Service (Cloud KMS)](https://docs.cloud.google.com/security-key-management), column
policies, VPC Service Controls, and reservations. Therefore, you avoid the
data sprawl that's common with self-managed warehouses. The following diagram
shows an architecture that uses decentralized authority:

![An architecture uses decentralized authority to delegate core data administration decisions.](https://docs.cloud.google.com/static/bigquery/images/best-practices-for-multi-tenant-workloads-on-bigquery-decentralized-data-marts.svg)

**Figure 5.** A core governance project helps to ensure consistent security,
while individual groups maintain their data operations.

The project configuration in figure 5 includes the following projects:

- **Core governance project** : the project that is responsible for cross-organization management concerns. In this project, you create security resources like Cloud KMS key rings and data catalog column policies. This project acts as the BigQuery [reservations admininistration project](https://docs.cloud.google.com/bigquery/docs/reservations-workload-management#admin-project), enabling organization-wide sharing of slots.
- **Organizational unit data projects**: the owners of self-managed data marts within the broader organization. The core governance project manages restricted scope for the organizational unit data projects.
- **Analytics team projects**: the projects that are used by consumers of the data marts. These projects use their own provisioned infrastructure and slots to access and process data within the data mart.

### Use a two-tier reservation design

We recommend that your decentralized data marts use
[the same two-tier design](https://docs.cloud.google.com/bigquery/docs/best-practices-for-multi-tenant-workloads-on-bigquery#data-marts-central-reservation)
as standard data marts. In this configuration, you assign the
[organization resource](https://docs.cloud.google.com/resource-manager/docs/cloud-platform-resource-hierarchy#organizations)
a reservation that has a small number of slots to cover general usage. If teams
have greater needs, assign reservations at the project or folder level.

### Use a data catalog

A data catalog provides organization-wide discovery, metadata tagging, and
column policy configuration. Knowledge Catalog discovery
automatically creates [metadata entries](https://docs.cloud.google.com/dataplex/docs/catalog-overview)
for all new
BigQuery tables across your organization. Capabilities in
Knowledge Catalog also
help data governance admins quickly identify new data assets and apply
appropriate controls.

### Configure VPC Service Controls perimeters

In this configuration, we recommend VPC Service Controls perimeters to
prevent accidental exposure of BigQuery datasets outside of your
Google Cloud organization.

#### Perimeters

In this configuration, we recommend that you create the following service
perimeters:

- **Core data**: a perimeter to protect the data warehouse and data mart datasets. This perimeter should include all organizational unit projects and the data governance project.
- **Analytics**: a perimeter to build and deploy analytics assets internal to the organization. This perimeter is expected to have a more permissive access policy than the core data perimeter that it's bridged with.

#### Perimeter bridges

In this configuration, we recommend that you create the following perimeter
bridges:

- **Core data and analytics**: allow users in the analytics projects to query the authorized views.

## Multi-organization data sharing

Multi-organization sharing is a special design consideration for a data mart
design. This data sharing design is for organizations that want to securely
share their datasets with another entity that has its own Google organization.

Multi-organization data sharing addresses the following concerns for the data
sharer:

- **Sharing confidentiality**: only the intended party can access shared data.
- **Protection from inappropriate access**: only resources that are intended to be accessed can be accessed externally.
- **Compute separation**: external parties are billed for queries that they initiate.

> [!NOTE]
> **Note:** In some cases, queries that originate from the outside organization can fail unexpectedly. [Cloud Customer Care](https://cloud.google.com/support-hub/) can help you initiate a configuration change to fix the issue.

### Protect internal projects from shared data projects

The multi-organizational data sharing design focuses on protecting the
organization's internal projects from activity in shared data projects. The
shared dataset project acts as a buffer to disallow access to sensitive internal
data processing, while providing the capability to share data externally.

Queries that initiate from the external project use the invoking project's
compute resources. If all queried datasets share the same Google Cloud
region, these queries can join data across organizations. The following diagram
shows how datasets are shared in a multi-organizational project configuration:

![A multi-organizational project configuration uses a shared dataset project to protect internal projects.](https://docs.cloud.google.com/static/bigquery/images/best-practices-for-multi-tenant-workloads-on-bigquery-multi-organization-sharing.svg)

**Figure 6.** An external organization queries data from multiple datasets in
shared projects.

The project configuration in figure 6 includes the following projects:

- **Organization internal project**: the project that contains sensitive internal data. The internal project can share data externally by copying sanitized data into the datasets of the shared data project. The internal project should own the service account that is responsible for updating the shared data.
- **Shared data project** : the project that contains the sanitized information that is copied from the internal project. Use external users [groups](https://docs.cloud.google.com/iam/docs/groups-in-cloud-console) to manage access by external parties. In this scenario, you manage group membership as an administrative function and you give external accounts the viewer permission so that they can access the dataset through these groups.

### Configure VPC Service Controls perimeters

In this configuration, we recommend VPC Service Controls perimeters to
share data externally and to prevent accidental exposure of
BigQuery datasets outside of your internal projects.

#### Perimeters

In this configuration, we recommend that you create the following service
perimeters:

- **Internal data**: a perimeter to protect core data assets. VPC Service Controls enforces access to BigQuery.
- **Externally shared data**: a perimeter to host datasets that can be shared with outside organizations. This perimeter disables enforcement of access to BigQuery.

#### Perimeter bridges

In this configuration, we recommend that you create the following perimeter
bridge:

- **Internal to external data**: a perimeter bridge allows the more protected internal data projects to egress data into external data share projects.

## Additional considerations in multi-tenant systems

This section provides a deeper look at special cases that you can consider
alongside the preceding best practices.

### Google Cloud resource limits and quotas

- Service accounts are limited to a soft quota of 100 service accounts per project. You can request quota through [the Google Cloud console](https://console.cloud.google.com/iam-admin/quotas) for projects that maintain tenant service accounts.
- BigQuery concurrency has a default concurrency of 100 queries per project that issues queries (projects that hold datasets have no such limits). To increase this soft quota, contact your sales representative.
- VPC Service Controls has a limit of 10,000 projects within service perimeters organization-wide. If your project-per-tenant designs have high scale up, we recommend using a dataset-per-tenant design instead.
- VPC Service Controls has a limit of 100 perimeters, including bridges, per organization.

### Using authorized views or materialized subset tables

To manage tenant access to subsets of large fact tables, you can use
tenant-specific authorized views or create tenant-specific subset tables. The
following table provides a comparison of these approaches:

| Feature | Authorized views | Subset tables |
|---|---|---|
| Number of tenants supported | There is a hard limit of [2500 authorized resources](https://docs.cloud.google.com/bigquery/quotas#dataset_limits) per dataset. | Authorized resources include authorized views, authorized datasets, and authorized functions.There are no limits on the number of datasets in a project or tables in a dataset. |
| Partitioning and clustering | Authorized views must share the common partitioning and cluster scheme of the base table. To improve the performance of tenant segmentation, we recommend that you cluster the parent table on the tenant ID. | You can partition the subset table and cluster it to the needs of the tenant. |
| Regionalization | Authorized views cannot cross regions and must be in the Google Cloud region of the base table. Regionalization affects geographically remote tenants. | Subset tables can exist in the region that's most appropriate for the tenant. Additional [costs](https://docs.cloud.google.com/bigquery/docs/copying-datasets#pricing) might apply. |
| Column policy enforcement | Column policies applied to a base table are applied to all authorized views regardless of the permissions on those views. | Each subset table must apply the column policy for it to take effect. |
| Data access logging | Data access logs are reflected in the logging of the base table. | Access to each subset table is logged separately. |
| Transformation flexibility | Authorized views allow for instant redesign of the object that tenants are accessing. | Complex schema changes are required to change subset tables. |

### Controlling for sensitive data

To prevent unauthorized access to data, BigQuery offers several
additional features beyond standard IAM permissions.

#### Client-supplied encryption

Client data encryption covers cases in which a hosting organization stores and
processes data on a tenant's behalf, but is prevented from accessing some of the
data contents. For example, the hosting organization might be prevented from
accessing personal or device data that is considered sensitive.

We recommend that the data sender uses AEAD encryption keys, from the open
source
[Tink library](https://github.com/google/tink),
to encrypt sensitive data. The Tink library AEAD encryption keys are compatible
with the
[BigQuery AEAD functions](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aead_encryption_functions).
The tenant can then decrypt the data either by accessing the key material in an
[authorized UDF](https://docs.cloud.google.com/bigquery/docs/user-defined-functions#authorize_routines)
or by passing the key material as a
[query parameter](https://docs.cloud.google.com/bigquery/docs/parameterized-queries#bq)
to BigQuery, where the host organization cannot log the key.

#### Column access policies

In multi-tenant data marts, column policies are frequently used to prevent
sensitive content from accidentally leaking between collaborating teams.
Authorized views are the preferred mechanism to share data between teams in a
data mart scenario. Authorized views cannot grant access to a protected
column.

When you set the policy to enforce access control, you prevent access to users
who have not been granted the
[fine-grained reader](https://docs.cloud.google.com/bigquery/docs/column-level-security#fine_grained_reader)
role to the policy. Even when the policy isn't enforced, the policy logs all
user access to the categorized column.

#### Sensitive Data Protection

[Sensitive Data Protection](https://docs.cloud.google.com/sensitive-data-protection) provides APIs and scanning utilities that help you
identify and mitigate sensitive content that is stored inside
BigQuery or [Cloud Storage](https://docs.cloud.google.com/storage) datasets. Organizations
in multi-tenant scenarios frequently use the DLP API (part of Sensitive Data Protection) to identify
and optionally tokenize sensitive data before it's stored.

### Slots reservation management

Reservation management in multi-tenant systems helps to control costs as
tenants scale up and ensures performance guarantees for each tenant.

To manage reservations, we recommend that you create a single reservation
administrative project. Slot commitments that are purchased within the same
administrative project are shareable across all
[reservations](https://docs.cloud.google.com/bigquery/docs/reservations-workload-management#admin-project)
that originate from the administrative project. A project that uses slot
commitments can only be assigned to one reservation at a time. All queries that
[originate from a project](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs/insert#path-parameters)
share slots based on available resources.

To ensure that tenant service level objectives (SLOs) are met, you can
[monitor](https://docs.cloud.google.com/bigquery/docs/reservations-monitoring)
reservations through Cloud Logging and the BigQuery
[information schema](https://docs.cloud.google.com/bigquery/docs/information-schema-jobs).
Organizations that experience busy periods from analyst activity or priority
jobs can allocate extra capacity using
[flex slots](https://cloud.google.com/bigquery/pricing#flex-slots-pricing).

#### Reservations as tenant compute tiers

SaaS vendors who have dozens up to many thousands of tenants commonly configure
a finite number of BigQuery reservations as shared resources.

If you are an SaaS vendor who has shared tenant infrastructure, we recommend
that you dedicate each reservation to a single project and group tenants to
share that project for BigQuery compute. This design reduces the
administrative overhead of having thousands of additional projects and
reservations, while allowing your organization to allocate a minimum
[slot capacity](https://docs.cloud.google.com/bigquery/docs/reference/reservations/rest/v1/projects.locations.reservations)
necessary to meet anticipated performance needs for the reservation.

If
[ELT data processing](https://wikipedia.org/wiki/Extract,_transform,_load#Vs._ELT)
timeliness is a top priority, we recommend that you allocate a reservation
to handle the processing. To prevent using extra slots that could be used for
ad hoc workloads, set the reservation to
[ignore idle slots](https://docs.cloud.google.com/bigquery/docs/slots#idle_slots).

Following is an example of how to configure reservations as tenant compute
tiers:

- **Data processing**: 2000 slots, ignore idle. This reservation is configured to meet data processing SLOs.
- **Internal projects**: 1000 slots, allow idle. This reservation is applied to the projects that are used for internal analytics.Slots are reused if they're left over from data processing or compute tiers.
- **Low compute tier**: 2000 slots, ignore idle. This reservation is applied to tenants given low resources. Unlike the high tier, this reservation ignores idle slots.
- **High compute tier**: 3000 slots, allow idle. This reservation is applied to tenants given high resources. To speed queries, idle slots from other reservations are automatically applied.

If your tenants operate on dedicated infrastructure, we recommend that you
assign the designated folder or project to the appropriate shared reservation.

#### Reservations per team

When you work with teams in a data mart setting, we recommend that you create a
reservation for each team that requires additional compute. Then assign that
reservation to the parent folder that contains the team's projects. All new
projects for that team use
[fair scheduling](https://docs.cloud.google.com/bigquery/docs/slots#fair_scheduling_in_bigquery)
slots from the same allocation of resources.

Following is an example of how to configure reservations per team:

- **Organization level reservation**: 500 slots, allow idle. This reservation is assigned to the top-level organization, and it gives slots to any BigQuery user who isn't using a project that has a dedicated reservation
- **Data processing**: 1000 slots, ignore idle. This reservation is configured to meet minimum data processing SLOs.
- **Core data administration**: 500 slots, allow idle. This reservation is applied to the projects that are used for internal administration. Slots are reused if they're left over from data processing or compute tiers.
- **Analytics processing reservations**: 500 slots, allow idle. This is a dedicated reservation that's given to an analytics team.

### Multi-regional hosting requirements

Multi-regional hosting requirements are typically a result of either a need to
reduce data latency for consumers or to meet regulatory mandates.

Projects in Google Cloud are considered global and can provision
resources in any region. BigQuery considers datasets, column
policies, and slot commitments to be regional resources. Slots can only access
datasets in the local region, and column policies can only be applied to tables
within local datasets. To use capacity-based pricing, you must purchase slots in each
region that contains datasets.

For guidance on adhering to regulatory requirements, consult with your sales
representative.

## What's next

- Learn about [IAM best practices](https://docs.cloud.google.com/iam/docs/recommender-best-practices).