# Understand reservations

This page describes how to use slot reservations to
manage your BigQuery workloads.

## Slot reservations

In BigQuery, [slots](https://docs.cloud.google.com/bigquery/docs/slots) are allocated in pools
called *reservations* . Reservations let you manage capacity and isolate workloads
in ways that make sense for your organization. For example, you might create a
reservation named `prod` for production workloads, and a separate reservation
named `test` for testing, so that test jobs don't compete for resources with
production jobs. Or, you might create reservations for different departments in
your organization to allocate compute costs.

Reservations also let administrators configure slot capacity based on the
workloads assigned to them. For example, if you have production-level,
time-sensitive workloads, create a reservation with adequate baseline slots.
Baseline slots are always available and ensure the reservation always has
sufficient capacity. However, if you use autoscaling reservations, the capacity
in a reservation isn't necessarily reserved. When you use [autoscaling
reservations](https://docs.cloud.google.com/bigquery/docs/slots-autoscaling-intro), the capacity is
automatically scaled up and down based on the demand. In addition, [idle
slots](https://docs.cloud.google.com/bigquery/docs/slots#idle_slots) can be shared across reservations.

## Reservation assignments

To use allocated slots in a reservation, you must *assign* it to one or more projects,
folders, or organizations. When a job in a project runs, it uses slots
from its assigned reservation. Resources can inherit assignments from their
parent in the [Google Cloud resource hierarchy](https://docs.cloud.google.com/resource-manager/docs/cloud-platform-resource-hierarchy).
If a project is not assigned to a reservation, it inherits the assignment of the
parent folder or organization, if it exists.

Projects use the single most specific reservation in the resource hierarchy to
which they are assigned. A folder assignment overrides an organization
assignment, and a project assignment overrides a folder assignment.

If a project doesn't have an assigned or inherited reservation, then the
job uses on-demand pricing. For more information about the resource hierarchy,
see [organizing BigQuery resources](https://docs.cloud.google.com/bigquery/docs/resource-hierarchy).

Resources can be assigned to `None` to represent an absence of an assignment.
Projects that are assigned to `None` always use on-demand pricing. A common use
case for `None` assignments is to assign an organization to a reservation and
then use `None` to opt certain projects or folders out of that reservation. For
more information, see [Assign a project to
`None`](https://docs.cloud.google.com/bigquery/docs/reservations-assignments#assign-project-to-none).

When you create an assignment, you specify the job type for that assignment:

- `QUERY`: Use this reservation for non-continuous query jobs, including SQL,
  DDL, DML, and BigQuery ML (built-in models) queries.

- `BACKGROUND_CHANGE_DATA_CAPTURE`: Use this reservation when you choose to [use your own
  reservation](https://docs.cloud.google.com/bigquery/docs/search-index#use_your_own_reservation) to run
  your [BigQuery CDC ingestion](https://docs.cloud.google.com/bigquery/docs/change-data-capture)
  background jobs. `BACKGROUND_CHANGE_DATA_CAPTURE` reservations are not available in the
  Standard edition.

- `BACKGROUND_COLUMN_METADATA_INDEX`: Use this reservation when you choose to [use your own
  reservation](https://docs.cloud.google.com/bigquery/docs/search-index#use_your_own_reservation) to run
  your [BigLake metadata caching](https://docs.cloud.google.com/bigquery/docs/metadata-caching)
  background jobs. Also use this reservation when you replicate source databases to BigQuery with Datastream's background apply
  operations. `BACKGROUND_COLUMN_METADATA_INDEX` reservations are not available in the
  Standard edition.

- `BACKGROUND_SEARCH_INDEX_REFRESH`: Use this reservation when you choose to [use your own
  reservation](https://docs.cloud.google.com/bigquery/docs/search-index#use_your_own_reservation) to run
  your [BigQuery search](https://docs.cloud.google.com/bigquery/docs/search-intro) index
  management background jobs. `BACKGROUND_SEARCH_INDEX_REFRESH` reservations are not available in the
  Standard edition.

- `BACKGROUND`: Use this reservation when you choose to [use your own
  reservation](https://docs.cloud.google.com/bigquery/docs/search-index#use_your_own_reservation) to replicate source databases to BigQuery with Datastream's background apply operations. This reservation will also be used for the jobs described by `BACKGROUND_CHANGE_DATA_CAPTURE`, `BACKGROUND_COLUMN_METADATA_INDEX`, and `BACKGROUND_SEARCH_INDEX_REFRESH` as a fallback if a more specific reservation for those job types does not exist. `BACKGROUND` reservations are not available in the Standard edition.

- `CONTINUOUS`: Use this reservation for
  [continuous query](https://docs.cloud.google.com/bigquery/docs/continuous-queries-introduction) jobs.

- `ML_EXTERNAL`: Use this reservation for BigQuery ML
  [`CREATE MODEL`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create)
  queries that use services that are external to BigQuery. For
  more information, see
  [Assign slots to BigQuery ML workloads](https://docs.cloud.google.com/bigquery/docs/reservations-assignments#assign-ml-workload).
  `ML_EXTERNAL` reservations are not available in the Standard
  edition.

- `PIPELINE`: Use this reservation for load and extract jobs.

  By default, load and extract jobs are [free](https://cloud.google.com/bigquery/pricing#free) and
  use a shared pool of slots. BigQuery does not guarantee
  capacity availability for this shared pool or the throughput you
  see. If you are loading large amounts of data, your job might wait for slots
  to become available.

  When load and extract jobs are assigned to a reservation, they lose access
  to the free pool. You should
  [Monitor resource utilization and
  jobs](https://docs.cloud.google.com/bigquery/docs/admin-resource-charts) to ensure your reservations
  have enough capacity to meet your required job duration times.

You can't allocate individual slots to specific assignments. The BigQuery
scheduler handles slot allocation for jobs using a reservation. For more
information about how slots are used, see [fair scheduling in BigQuery](https://docs.cloud.google.com/bigquery/docs/slots#fair_scheduling_in_bigquery).

### Flexibly assign reservations

BigQuery lets you specify which
reservation a query should run on at runtime. This gives you more control over
resource allocation and lets you avoid creating unnecessary projects. You can
specify a reservation at runtime using the [CLI](https://docs.cloud.google.com/bigquery/docs/reservations-assignments#bq_3),
[UI](https://docs.cloud.google.com/bigquery/docs/reservations-assignments#console_2),
[SQL](https://docs.cloud.google.com/bigquery/docs/reservations-assignments#sql_3),
or [API](https://docs.cloud.google.com/bigquery/docs/reservations-assignments#api), overriding the default
reservation assignment for your project, folder, or organization. The assigned
reservation must be in the same region as the query you are running.
These assignments are supported in all [editions](https://docs.cloud.google.com/bigquery/docs/editions-intro).

You must have [access to the reservation](https://docs.cloud.google.com/bigquery/docs/reservations-workload-management#securable) in order to use the
reservation when you run your query.

If the default assigned reservation's edition is Enterprise Plus it can
be overridden only with an Enterprise Plus reservation.

To flexibly assign reservations, [run an interactive
query](https://docs.cloud.google.com/bigquery/docs/running-queries#queries) and specify the reservation.

### Combine reservations with on-demand billing

You can use capacity-based billing in one region and on-demand in another
region. By default, all projects use on-demand billing. Within a region, you can
opt a project, folder, or organization into capacity-based billing by assigning
it to a reservation. For example, if you purchase a slot commitment in the US
multi-region and assign your organization to the default reservation, your
organization will be on capacity-based billing in the US multi-region, but will
remain on on-demand billing in all other regions.

Within a region, you can combine capacity-based and on-demand billing by explicitly
assigning projects to a reservation. Any project not assigned to a reservation
remains on on-demand billing. You can also explicitly assign a project to use
on-demand billing by assigning the reservation ID `none`. This is useful if you
assign a folder or an organization to a reservation, but want some projects
within that folder or organization to use on-demand billing. For more
information, see
[assign a project to none](https://docs.cloud.google.com/bigquery/docs/reservations-assignments#assign-project-to-none).

Projects with on-demand billing use capacity that's separate from your committed
capacity. Those projects don't affect the availability of your committed
capacity. If you query data in the streaming buffer, on-demand jobs aren't
charged for bytes processed from streaming buffer, but jobs running in a
reservation use slots to process these bytes.

## Specifying an administration project

When you create commitments and reservations, they are associated with a Google Cloud project.
This project manages the BigQuery reservations resources, and is
the primary source of billing for these resources. This project does not have to
be the same project that holds your BigQuery jobs or datasets.

As a best practice, create a dedicated project for reservation resources. This
project is called the *administration* project, because it centralizes the
billing and management of your commitments. Give this project a descriptive name like
`bq-COMPANY_NAME-admin`. Then create one or more separate
projects to hold your BigQuery jobs.

Only projects within the same
[Organization resource](https://docs.cloud.google.com/resource-manager/docs/creating-managing-organization)
as the administration project can be assigned to a reservation. If the
administration project is not part of an Organization, then only that project
can use the slots.

The administration project is billed for the committed slots. Projects that use
the slots are billed for storage but not billed for the slots. You can purchase
more than one type of plan (for example, annual and three-year) and put the slots
into the same administration project.

> [!NOTE]
> **Note:** After you create a reservation or a commitment, you can't move it to a different administration project.

As a best practice, limit the number of administration projects. This helps to
simplify billing management and slot allocation. One administration project for
all of your organization's reservations is recommended where practical.
Complex organizations might require additional administration projects to meet
management or billing requirements.

### Using multiple administration projects

In some cases, you might want to create more than one administration project:

- To separate costs from multiple reservations and commitments into different organizational units.
- To map one or more slot commitments to different sets of reservations.

Idle slot capacity is not shared between reservations in different
administration projects.

When you're on the **Capacity management** page of the BigQuery Google Cloud console,
you can view reservations and commitments only for the selected administrative
project.

## Sizing slot reservations

BigQuery is architected to scale linearly with increased
resources. Depending on the workload, incremental capacity is likely to give you
incremental performance. However, adding capacity also increases your costs.
Therefore, choosing the optimal number of slots to
purchase depends on your requirements for performance, throughput, and utility.

You can experiment with baseline and autoscaling slots to determine the best
configuration of slots. For example, you can test your workload with 500
baseline slots, then 1,000, then 1,500, and 2,000, and observe the impact on
performance.

If the region where performance is tested has idle slots available, a
reservation might meet performance with fewer slots than expected. This occurs
because the reservation used the idle slots to fulfill the workload's slot
requirements. If idle slot availability decreases due to workload changes in
other reservations, your workload performance might degrade. Therefore, for
production-level, time-bound workloads, ensure the reservation is self-reliant
in slot capacity. This prevents changes in idle slots from directly affecting
job performance.

After you allocate slots and run your workloads for at least seven days, you can
use the [slot estimator](https://docs.cloud.google.com/bigquery/docs/slot-estimator) to analyze
performance and model the effect of adding or reducing slots.

You can also examine the current slot usage of your projects, along with the
chosen monthly price that you want to pay. On-demand workloads have a
soft slot cap of 2,000 slots, but it is important to check how many slots are
actually being used by your projects by using
[`INFORMATION_SCHEMA.JOBS*` views](https://docs.cloud.google.com/bigquery/docs/information-schema-jobs),
Cloud Logging, the Jobs API, or BigQuery Audit
logs. For more information, see
[Monitor reservations](https://docs.cloud.google.com/bigquery/docs/reservations-monitoring).

![Slot usage timeline.](https://docs.cloud.google.com/static/bigquery/images/reservations-monitoring-timeline.png)

## Manage workloads using reservations

You can use BigQuery reservations to allocate
capacity between workloads, teams, or departments by creating additional
reservations and assigning projects to these reservations. A reservation is an
isolated pool of resources with the added benefit of being able to take
advantage of idle capacity in a given region across your organization.

For example, you might have a total committed capacity of 1,000 slots and
three workload types: data science, ELT, and BI.

- You can create a `ds` reservation with 500 slots, and assign all relevant Google Cloud projects to the `ds` reservation.
- You can create an `elt` reservation with 300 slots, and assign projects you use for ELT workloads to the `elt` reservation.
- You can create a `bi` reservation with 200 slots, and assign projects connected to your BI tools to the `bi` reservation.

![Commitments delete.](https://docs.cloud.google.com/static/bigquery/images/reservations-reservations.svg)

Instead of partitioning your capacity across workload types, you might choose to
create reservations for individual teams or departments.

### Manage reservations in different regions

Reservations are regional resources. Slots purchased and reservations created
in one region cannot be used in any other region. Projects, folders, and
organizations can be assigned to reservations in one region and run on-demand
in another. To manage reservations in another region, you must change the
region in the BigQuery **Capacity Management** page:

1. In the BigQuery console, click **Reservations**.
2. Click the **Location** picker and select a region where you want to manage reservations. ![Select different region.](https://docs.cloud.google.com/static/bigquery/images/reservations-different-region.png)
3. After a region is selected, you can purchase slots, create reservations, and assign a project to a reservation.

### Manage reservations in complex organizations

Reservations are organization-scoped resources. When you create reservations,
you can assign capacity to any project within the same Google Cloud
organization. Most BigQuery users use a single administration project
for their reservations and commitments. This administration project is associated
with a Cloud Billing account, which is billed for the capacity.

However, if you have a complex organization with multiple divisions that
manage their own bills, you might want to use multiple administration
projects. Note that [idle slots](https://docs.cloud.google.com/bigquery/docs/slots#idle_slots) can only be
shared across reservations created in the same administration project.
You should be aware of the [quotas and limits](https://docs.cloud.google.com/bigquery/quotas#reservations)
for reservations and administration projects.

If you use multiple Google Cloud organizations, you must create
at least one administration project for each organization, and then manage
reservations and commitments for each organization in its related
administration project. You can't share capacity across organizations.

> [!NOTE]
> **Note:** [Changing the Cloud Billing account](https://docs.cloud.google.com/billing/docs/how-to/modify-project#change_the_billing_account_for_a_project) that is linked to the project doesn't create downtime in already existing commitments and reservations. Slots are linked to a project or organization and are independent from the Cloud Billing account.

### Manage enhanced control on reservations

Reservations in BigQuery offer enhanced control over
how reservations are used and provide additional security features. You can
define policies that specify which users or groups can access and use specific
reservations. This ensures that sensitive data and workloads are isolated and
protected. As a reservation administrator, you can precisely control which users
or service accounts (principals) are authorized to use specific reservations.
To do this, you use IAM conditions applied to the
administrative project (the project where the reservations are managed).

For example, you can create an IAM condition that grants the
`reservations.use` permission to a specific group of users for all reservations
whose names begin with a certain prefix. This lets you manage access
to sets of related reservations.

Users must have the `reservations.use` permission to override the default
reservation for their job, or force the job to run on-demand when a reservation
assignment already exists for the project. The `roles/bigquery.resourceAdmin`
and `roles/bigquery.resourceEditor` roles provide this permission. You can grant
access to individual users, groups, or service accounts. You can also define
policies based on reservation attributes like name, as IAM
conditions support attribute-based access control.

To grant IAM conditions on reservations, see [Control
access to
reservations](https://docs.cloud.google.com/bigquery/docs/reservations-tasks#control_access_to_reservations).

## Slot commitments

A *slot commitment* is a purchase of slots for a specified period of time. You
can purchase slots in 50 slot increments, up to your [regional slot
quota](https://docs.cloud.google.com/bigquery/quotas#reservations). Capacity commitments are optional, but
can provide cost savings for steady-state workloads. Slot commitments are used
to cover baseline slots for your reservations. Any unused slot capacity is then
shared as idle slots across other reservations. Slot commitments don't apply to
autoscaling slots. To ensure that you
receive the discounted rate for your committed slots, make sure that your
slot commitments are sufficient to cover your baseline slots. There is no
limit on the number of commitments that you can create. You are charged from the
moment that your commitment purchase is successful. For current pricing
information, see [capacity commitment
pricing](https://cloud.google.com/bigquery/pricing#capacity_compute_analysis_pricing).

- **Annual commitment**. You purchase a 365-day commitment. You can choose
  whether to renew or convert to a different type of commitment plan
  after 365 days.

- **Three-year commitment**. You purchase a three-year commitment. You can
  choose whether to renew or convert to a different type of commitment plan
  after 3 years (1,095 days).

At the end of the commitment period, your commitment will renew based on the
[selected renewal plan](https://docs.cloud.google.com/bigquery/docs/reservations-workload-management#renew-commitments).

You are billed monthly for annual or three-year commitment plans. However, your
financial commitment is for the entire commitment period and can't be canceled
on a monthly basis. Your usage is updated in the billing report daily, which you
can view at any time.

Slot commitments are subject to capacity availability. When you attempt to purchase slot
commitments, the success of your purchase is not guaranteed. However, once your
commitment purchase is successful, the capacity is available until the commitment
expires.

If you purchase slot commitments before creating a reservation, then a
reservation named `default` is created automatically as a convenience. The
`default` reservation has no special behavior. You can create additional
reservations if needed or use the default reservation.

We recommend assigning a non-zero baseline to your reservations for the most
predictable performance and initial capacity. While you can configure a
reservation with zero baseline slots and set a maximum capacity intending to use
autoscaling features, the effectiveness of this approach depends entirely on
autoscaling being correctly enabled and actively acquiring slots. If autoscaling
is not effectively functioning for such a zero-baseline reservation, it reverts
to depending solely on available idle slot capacity, which offers no performance
guarantee and can lead to unpredictable or degraded query speeds.

### Renew commitments

You select a renewal plan when you purchase a commitment. You can change
the renewal plan for a commitment at any time until it expires. The
following renewal plans are available:

- **None.** After the end of the commitment period, the commitment is removed. Reservations are not affected.
- **Annual.** After the end of the commitment period, your commitment renews for another year.
- **Three-Year.** After the end of the commitment period, your commitment renews for another three years.

For information about purchasing and renewing commitments, see [Create a capacity
commitment](https://docs.cloud.google.com/bigquery/docs/reservations-commitments#create_a_capacity_commitment).

For example, if you purchased an annual commitment at 6:00 PM on October 5,
2019, then you started being charged at that second. You can delete or renew the
commitment after 6:00 PM on October 4, 2020, noting that 2020 is a leap year.
You can change the renewal plans prior to 6:00 PM on October 4, 2020 as follows:

- If you choose to renew to an annual commitment, then at 6:00 PM on October 4, 2020, your commitment renews for another year.
- If you choose to renew to a three-year commitment, then at 6:00 PM on October 4, 2020, your commitment renews for three years.

**Note:** The renewal process can take up to roughly one hour after the
commitment expires. For example, if a commitment expires at 6:00 PM on October
4, 2020, the renewed commitment record appears in the system between around
6:00 PM and 7:00 PM on October 4, 2020. No on-demand charges are applied within
this data update period as the effective start time for the created commitment
is 6:00 PM.

### Commitment expiration

You can't delete a commitment once you create it.
To delete an annual or three-year commitment, set its renewal plan to `NONE`.
After the commitment expires, it is deleted automatically. To learn more about
commitment expirations, see [Commitment
expiration](https://docs.cloud.google.com/bigquery/docs/reservations-commitments#commitment_expiration).

If you accidentally purchase a commitment or make a mistake when you configure
your commitment, contact
[Cloud Customer Care](https://cloud.google.com/support-hub) for help.

## Reservation limitations

- Reservations in one organization can't be shared with other organizations.
- You must use separate reservations and separate administration projects for each organization.
- Each organization can have a maximum of 10 administration projects with active reservations in a single location.
- Idle capacity can't be shared between organizations or between different administration projects within a single organization.
- Idle capacity can't be shared across different regions.
- Commitments and reservations are [regional resources](https://docs.cloud.google.com/bigquery/docs/locations#specify_locations). Commitments purchased in one region or multi-region can't be used for reservations in any other regions or multi-regions, even when the single region location is co-located with the multi-region location. For example, you can't use a commitment purchased in the `EU` multi-region for a reservation in `europe-west4`.
- Commitments and reservations can't be moved from one region or multi-region to another.
- Commitments purchased in one administration project can't be moved to a different administration project.
- Commitments purchased with one [edition](https://docs.cloud.google.com/bigquery/docs/editions-intro) can't be used with reservations of another edition.
- Idle slots aren't shared between reservations of different [editions](https://docs.cloud.google.com/bigquery/docs/editions-intro).
- [Autoscaled slots](https://docs.cloud.google.com/bigquery/docs/slots-autoscaling-intro) aren't shareable as they scale down when no longer required.

## Reservation predictability

To use reservation predictability, you must first enable [reservation fairness](https://docs.cloud.google.com/bigquery/docs/slots#fairness).

Reservation predictability lets you set the absolute maximum number of
consumed slots on a reservation. BigQuery offers baseline slots,
idle slots, and autoscaling slots as potential capacity resources. When you
create a reservation with a maximum size, confirm the number of baseline slots
and the appropriate configuration of autoscaling and idle slots based on your
past workloads.

To enable reservation predictability, you must set the value of both the maximum
slots and the scaling mode on the reservation. The number of maximum slots must
be a positive number, and greater than the number of baseline slots assigned to
the reservation. To learn more about working with reservation predictability, see
[Create a reservation with dedicated
slots](https://docs.cloud.google.com/bigquery/docs/reservations-tasks#create_a_reservation_with_dedicated_slots).
You can't configure the value for `autoscale_max_slots` when you set the maximum
slots value on the reservation.

The value of `ignore_idle_slots` must align with the scaling mode. If the
scaling mode is `ALL_SLOTS` or `IDLE_SLOTS_ONLY`, `ignore_idle_slots` must be
false. If the scaling mode is `AUTOSCALE_ONLY`, `ignore_idle_slots` must be
true.

You can configure your reservations to consume only the following combinations
of capacity resources up to the defined maximum:

- **Baseline slots + idle slots** : The reservation slot capacity is greater than
  zero, and the scaling mode is `IDLE_SLOTS_ONLY`. The reservation consumes the
  configured number of baseline and available idle slots up to the maximum number of
  slots. The reservation may not reach the maximum if there aren't sufficient idle
  slots available.

- **Baseline slots + idle slots + autoscaling slots** : The reservation slot
  capacity is greater than zero, and the scaling mode is `ALL_SLOTS`. The
  reservation first consumes the configured number of baseline slots, then all
  available idle slots, then autoscaling slots.

- **Baseline slots + autoscaling slots** : The reservation slot capacity is
  greater than zero, and the scaling mode is `AUTOSCALE_ONLY`. The reservation
  first consumes the configured baseline slots, then autoscaling slots.

- **Idle slots + autoscaling slots** : The reservation slot capacity is zero, and
  the scaling mode is `ALL_SLOTS`. The reservation first consumes all available
  idle slots, then autoscaling slots.

- **Idle slots** : The reservation slot capacity is zero, and the scaling mode is
  `IDLE_SLOTS_ONLY`. The reservation consumes all available idle slots up to the
  configured maximum. The reservation may not reach the maximum if there aren't
  sufficient idle slots available.

The following diagram shows the different configuration options available:

![Predictable reservation configuration
options.](https://docs.cloud.google.com/static/bigquery/images/max_slots_configurations.svg)

In the diagram, the five configuration options show how BigQuery
consumes slots up to your configured maximum. The first three options contain
baseline slots, while the others have no baseline slots configured.

### Limitations

Reservation predictability is subject to the following limitations:

- Reservation predictability is only available with the Enterprise and
  Enterprise Plus editions, unless you choose the `AUTOSCALE_ONLY`
  option.

- Reservation predictability is best-effort. The overall usage might still exceed
  the configured maximum.

### What's next

- To learn more about working with reservation predictability, see [Create a
  reservation with dedicated
  slots](https://docs.cloud.google.com/bigquery/docs/reservations-tasks#create_a_reservation_with_dedicated_slots).

## Reservation groups

> [!WARNING]
>
> **Preview**
>
>
> This product or feature is
>
> subject to the "Pre-GA Offerings Terms" in the General Service Terms section
> of the [Service Specific
> Terms](https://docs.cloud.google.com/terms/service-terms#1).
>
> Pre-GA products and features are available "as is" and might have limited support.
>
> For more information, see the
> [launch stage descriptions](https://cloud.google.com/products/#product-launch-stages).

To request support or provide feedback for this feature, contact
[bigquery-wlm-feedback@google.com](mailto:bigquery-wlm-feedback@google.com).

To use reservation groups, you must first enable [reservation fairness](https://docs.cloud.google.com/bigquery/docs/slots#fairness).

Reservation groups let you manage the properties of multiple reservations,
similar to how datasets organize tables.

Reservations with the same reservation group will share idle slots with each other first before being available to the organization:

![Reservation groups.](https://docs.cloud.google.com/static/bigquery/images/reservation-groups.svg)

### Idle slot sharing with reservation groups

Idle slots are distributed evenly among ungrouped reservations and reservation groups and then evenly within reservation groups.

In the following example, there are three reservations and 1,200 idle slots. With no reservation groups, each reservation has a fair share of 400 idle slots. When reservations 1 and 2 are grouped together, the idle slot distribution changes. The idle slots are first distributed evenly among the reservation group and reservation 3 (600 idle slots to both). Then the 600 idle slots for the reservation group are distributed evenly to reservation 1 and 2.

![Reservation groups idle slot sharing.](https://docs.cloud.google.com/static/bigquery/images/reservation-groups-idle-slot-sharing.svg)

### Limitations

Reservation groups are subject to the following limitations:

- Reservations sharing a reservation group must belong to the same project and same region.

- [Reservation-based fairness](https://docs.cloud.google.com/bigquery/docs/slots#fairness) must be enabled on the project.

- The total size of reservations within a group cannot exceed 30k slots. This includes autoscale limits.

- Reservations within a group must have the same [edition](https://docs.cloud.google.com/bigquery/docs/editions-intro).

- You cannot mix reservations with and without [managed disaster recovery](https://docs.cloud.google.com/bigquery/docs/managed-disaster-recovery) configurations within the same group. All reservations in the group must have disaster recovery configured or all must have it disabled.

- If disaster recovery is enabled for the reservations in a group, all reservations must use the same pair of regions for their primary and secondary locations.

To learn more about working with reservation groups, see [Prioritize idle slots with reservation groups](https://docs.cloud.google.com/bigquery/docs/reservations-tasks#prioritize_idle_slots_with_reservation_groups).

## Troubleshoot issues with reservations

This section is intended to help troubleshoot common issues encountered while interacting with reservations, such as determining reasons for a reservation not being used for a BigQuery job, identifying unknown reservations or problems while adding slots.

### Unable to add more slots to the reservation size

If you encounter errors like `Failed to allocate slots for reservation in the current system state` or `Failed to update reservation: Failed to allocate slots for reservation` while trying to add more slots to your reservation, this is usually a transient issue. To mitigate, do the following:

- Retry with a smaller number of slots
- If trying with a smaller number of slots fails, wait 15 minutes and retry the operation

If after retrying multiple times and waiting for 30 minutes you still receive the same error, [contact BigQuery support](https://docs.cloud.google.com/bigquery/docs/getting-support).

### There is insufficient quota to complete this request

If the error message states `There is insufficient quota to complete this request`, this means that the request exceeds the quota limit that is set for the project.

To resolve this error, do one of the following options:

1. Add a smaller number of slots to the reservation so that the quota limit is not exceeded.
2. Request a quota increase in the corresponding region. See [Requesting a quota increase](https://docs.cloud.google.com/bigquery/quotas#requesting_a_quota_increase).

### Reservation not used by BigQuery to run a job

There are multiple scenarios when a job might run using on-demand resources or a free shared slot pool instead of using the reservation that was created.

#### Query and reservation are in different regions

Reservations are regional resources. The query runs in the same location as any tables referenced in the query.

If the location of a table doesn't match the location of the reservation, the query runs using the shared slot pool and won't use the reservation.

#### Querying BigQuery Omni tables

When querying a BigQuery Omni table, make sure that the reservation is created in the same region as the table, not in a colocated region. If you create the reservation in the colocated BigQuery region, the query runs on-demand.

#### The reservation was created but the project was not assigned to it

To use the slots that you purchased you must create an assignment that assigns the project to the specific reservation. Make sure that the project has a corresponding [assignment for the reservation](https://docs.cloud.google.com/bigquery/docs/reservations-assignments).

#### Job type mismatch

Make sure to select the correct [job type](https://docs.cloud.google.com/bigquery/docs/reservations-workload-management#assignments) when creating an assignment, otherwise the jobs will run using the shared slot pool.

For example, if you select `PIPELINE` as the job type, all query jobs will run on-demand. Change the assignment type to `QUERY` to make the query jobs run using the reservation.

#### Multi-statement queries

If you are running multi-statement queries, the parent job object won't have a reservation associated with it, even if the child jobs were run under a reservation.

To confirm whether the job was actually using a reservation, look at the child job metadata for clarification.

#### Retrieving cached results

When the query job retrieves cached results, the reservation field will be empty because no actual computation is being performed and results are fetched from the temporary table directly.

#### Change data capture row modification operations

If you have [change data capture (CDC) tables](https://docs.cloud.google.com/bigquery/docs/change-data-capture), BigQuery will apply pending row modifications within the `max_staleness` interval as background jobs which will use the `BACKGROUND` assignment type. If there are no `BACKGROUND` assignments, they will use on-demand pricing. Consider creating a `BACKGROUND` assignment for the project to avoid high on-demand costs. You can identify these jobs having the `queueworker_cdc_background_merge_coalesce` substring within the job identifier.

#### BigQuery ML model types that use external services

If no reservation assignment with an `ML_EXTERNAL` job type is found in the project, the query job runs using on-demand pricing. The `QUERY` job type assignment can be only be used for BigQuery ML models that aren't external models or matrix factorization models. Read through the [Reservation Assignment](https://docs.cloud.google.com/bigquery/docs/reservations-assignments#assign-ml-workload) documentation to learn more.

### Unrecognized reservations identified in the project

There are reservations owned by BigQuery that represent a free shared slot pool used for certain operations in BigQuery:

#### `default-pipeline`

By default, performing batch loading or batch exporting of data in BigQuery uses a shared free slot pool. If you inspect these load or extract jobs, the reservation used will be listed as `default-pipeline`.

There are no charges for using the shared slot pool. If you want consistent predictable performance, consider purchasing a `PIPELINE` reservation.