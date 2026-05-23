# Introduction to legacy reservations

> [!NOTE]
> **Note:** Legacy reservations, including access to flat-rate billing or certain commitment lengths, are only available to allow-listed customers. To determine if you have access to these legacy features, contact your administrator. The flat-rate billing model defines how you are billed for compute, but flat-rate reservations and commitments function as Enterprise edition slots.

BigQuery reservations enable you to switch from
[on-demand pricing](https://cloud.google.com/bigquery/pricing#on_demand_pricing) to
[capacity-based pricing](https://cloud.google.com/bigquery/pricing#capacity_compute_analysis_pricing).
With capacity-based pricing, you pay for dedicated or autoscaled query
processing capacity rather than paying for each query individually.

Reservations enable you to allocate query capacity, measured in
[slots](https://docs.cloud.google.com/bigquery/docs/slots), to different workloads or different parts of
your organization.

Creating a [capacity commitment](https://docs.cloud.google.com/bigquery/docs/reservations-details) is
optional when working with reservations that leverage
[BigQuery editions](https://docs.cloud.google.com/bigquery/docs/editions-intro),
but can save on costs for steady-state workloads.

> [!NOTE]
> **Note:** [Legacy flat-rate commitments](https://docs.cloud.google.com/bigquery/docs/reservations-intro-legacy#commitments) have access to the same features as [Enterprise edition](https://docs.cloud.google.com/bigquery/docs/editions-intro) commitments. All legacy flat-rate commitments are labeled as `flat-rate` for the compute pricing model value and `Enterprise` for the edition value.

## Overview

BigQuery offers two compute (analysis) pricing models:

- **[On-demand pricing](https://cloud.google.com/bigquery/pricing#on_demand_pricing)** :
  You pay for the data scanned by your queries. You have a fixed, per-project
  [query processing capacity](https://docs.cloud.google.com/bigquery/quotas#max_concurrent_slots_on-demand),
  and your cost is based on the number of bytes processed by each query.

- **[Capacity-based pricing](https://cloud.google.com/bigquery/pricing#capacity_compute_analysis_pricing)** :
  You pay for dedicated or autoscaled query processing capacity, measured in
  [slots](https://docs.cloud.google.com/bigquery/docs/slots), over a period of time. Multiple queries share
  the same slot capacity.

By default, you are billed according to the on-demand pricing model. Using
reservations, you can switch to capacity-based pricing and
use [slots autoscaling](https://docs.cloud.google.com/bigquery/docs/slots-autoscaling-intro) and purchase
discounted [capacity commitments](https://docs.cloud.google.com/bigquery/docs/reservations-details). There is
no charge for bytes processed when using the capacity-based model.

You can combine both billing models. For example, you might run some workloads
with on-demand pricing and others with capacity-based pricing. As the billing
model is specified per project, this would require that you use multiple projects
for your query jobs.

## Benefits of reservations

The benefits of using BigQuery reservations include:

- **Predictability**. Capacity-based pricing offers predictable and consistent costs.
  You specify your maximum cost budget up front and can also take advantage of slot
  commitments, which offer dedicated ongoing capacity at a discounted rate.

- **Flexibility**. You choose how much dedicated capacity to allocate to a
  workload or let BigQuery automatically scale capacity based
  on your workload requirements. You are billed using for the slots consumed at
  a minimum of one second increments.

- **Workload management.** Each workload has a specified pool of BigQuery
  computational resources available for use. At the same time, if a workload
  doesn't use all of its dedicated slots, any unused slots are shared automatically
  across your other workloads.

- **Centralized purchasing:** You can purchase and allocate slots for your
  entire organization. You don't need to purchase slots for each project
  that uses BigQuery.

## Reservations

BigQuery capacity is measured in [*slots*](https://docs.cloud.google.com/bigquery/docs/slots),
which represent virtual CPUs used by queries.
Generally, if you provision more slots, you can run more
concurrent queries, and complex queries can run faster.

Slots are allocated in pools called *reservations*. Reservations let you
allocate the slots in ways that make sense for your particular organization.

For example, you might create a reservation named `prod` for production
workloads, and a separate reservation named `test` for testing. That way, your
test jobs don't compete for resources that your production workloads need. Or,
you might create reservations for different departments in your organization.

Reservations can include *baseline* slots, which are always allocated, as well
as *autoscaled* slots, which are [added or removed dynamically](https://docs.cloud.google.com/bigquery/docs/slots-autoscaling-intro)
based on the demands of your workload.

A reservation named `default` is automatically created if you purchase slot
commitments before creating a reservation. There is nothing special about the
`default` reservation --- it's created as a convenience. You can decide
whether you need additional reservations or just use the default reservation.

To use the slots that you allocate, you must *assign* one or more projects to
a reservation, as described in the next section.

A reservation is the lowest level at which you can specify slot allocation.
[Slot allocation within a reservation](https://docs.cloud.google.com/bigquery/docs/reservations-intro-legacy#slot_allocation_within_reservations)
is handled by the BigQuery scheduler.

## Assignments

To use the slots that you allocate, you must assign one or more projects,
folders, or organizations to a reservation. Each level in the resource hierarchy
inherits the assignment from the level above it. In other words, if a project or
folder is not assigned, then that project or folder inherits the assignment of
its parent folder or organization, if any. For more information about the
resource hierarchy, see
[Organizing BigQuery resources](https://docs.cloud.google.com/bigquery/docs/resource-hierarchy).

When a job is started from a project that is assigned to a reservation, the job
uses that reservation's slots. If a project is not assigned to a reservation
(either directly or by inheriting from its parent folder or organization), the
jobs in that project use on-demand pricing.

`None` assignments represent an absence of an assignment. Projects assigned to
`None` use on-demand pricing. The common use case for `None` assignments is to
assign an organization to the reservation and to opt out some projects or
folders from that reservation by assigning them to `None`. For more information,
see [Assign a project to None](https://docs.cloud.google.com/bigquery/docs/reservations-assignments#assign-project-to-none).

When you create an assignment, you specify the job type for that assignment:

- `QUERY`: Use this reservation for query jobs, including SQL, DDL, DML, and
  BigQuery ML queries.

- `PIPELINE`: Use this reservation for load and extract jobs.

  By default, load and extract jobs are [free](https://cloud.google.com/bigquery/pricing#free) and
  use a shared pool of slots. BigQuery does not make guarantees
  about the available capacity of this shared pool or the throughput you
  see. If you are loading large amounts of data, your job might wait as slots
  become available. In that case, you might want to purchase dedicated slots
  and assign pipeline jobs to them. We recommend creating an additional
  dedicated reservation with idle slot sharing disabled.

  When load jobs are assigned to a reservation, they lose access to the free
  pool. Monitor performance to make sure the jobs have enough capacity.
  Otherwise, performance could actually be worse than using the free pool.
- `BACKGROUND`: Use this reservation when you choose to [use your own
  reservation](https://docs.cloud.google.com/bigquery/docs/search-index#use_your_own_reservation) to run
  your [BigQuery search](https://docs.cloud.google.com/bigquery/docs/search-intro) index
  management jobs or [BigQuery change data capture (CDC) ingestion](https://docs.cloud.google.com/bigquery/docs/change-data-capture)
  background jobs. Also use this reservation when you replicate
  source databases to BigQuery with
  Datastream's background apply operations. `BACKGROUND`
  reservations are not available in the Standard edition.

- `ML_EXTERNAL`: Use this reservation for BigQuery ML queries
  that use services that are external to BigQuery. For more information, see
  [Assign slots to BigQuery ML workloads](https://docs.cloud.google.com/bigquery/docs/reservations-assignments#assign-ml-workload). `ML_EXTERNAL`
  reservations are not available in the Standard edition.

You can't allocate slots to specific assignments. The BigQuery
scheduler handles slot allocation for the assignments in a reservation.

## Commitments

![Reservations concepts.](https://docs.cloud.google.com/static/bigquery/images/reservations-concepts.svg)

A *capacity commitment* is a purchase of a fixed amount of BigQuery
compute capacity for some minimum duration of time. Capacity commitments are
optional for reservations created with an
[edition](https://docs.cloud.google.com/bigquery/docs/editions-intro), but can save on costs for steady-state
workloads.

BigQuery offers several commitment plans to choose from. They
differ mainly by cost and the minimum duration of your commitment. For current
pricing information, see
[capacity commitment pricing](https://cloud.google.com/bigquery/pricing#capacity_compute_analysis_pricing).

- **Annual commitment**. You purchase a 365-day commitment. You can choose
  whether to renew or convert to a different type of commitment plan
  after 365 days.

- **Monthly commitment**. You purchase a minimum 30-day commitment. After 30
  days, you can delete the plan at any time.

- **Flex slots**. You purchase a 60-second commitment. You can delete it at any time
  after 60 seconds. Flex slots are a good way to test how your workloads perform
  with flat-rate billing, before purchasing a longer-term commitment. They are
  also useful for handling cyclical or seasonal demand, or for high-load events
  such as tax season.

Whichever plan you select, your slots don't expire at the end of the commitment
period. You keep the slots and are billed for them until you delete them. You
can also change the plan type after the minimum duration.

Slots are subject to capacity availability. When you attempt to purchase slot
commitments, success of this purchase is not guaranteed. However, once your
commitment purchase is successful, your capacity is guaranteed until you delete
the commitment.

For more details about these plans, see
[Commitment plans](https://docs.cloud.google.com/bigquery/docs/reservations-details#commitment-plans).

## Slot allocation within reservations

BigQuery allocates slot capacity within a single reservation
using an algorithm called [fair scheduling](https://docs.cloud.google.com/bigquery/docs/slots#fair_scheduling_in_bigquery).

The BigQuery scheduler enforces the equal sharing of slots among
projects with running queries within a reservation, and then within jobs of a
given project. The scheduler provides eventual fairness. There might be short
periods where some jobs get a disproportionate share of slots, but the scheduler
eventually corrects this. The goal of the scheduler is to find a medium
between being too aggressive with evicting running tasks (which results in
wasting slot time) and being too lenient (which results in jobs with long
running tasks getting a disproportionate share of the slot time).

If an important job consistently needs more slots than it receives from the
scheduler, consider creating an additional reservation with a guaranteed number
of slots and assigning the job to that reservation. For more information, see
[Workload management](https://docs.cloud.google.com/bigquery/docs/reservations-workload-management).

## Idle slots

At any given time, some slots might be idle. This can include:

- Slot commitments that are not allocated to any reservation.
- Slots that are allocated to a reservation baseline but aren't currently in use.

By default, queries running in a reservation automatically use idle slots from
other reservations within the same administration project.
That means a job can always run as long as there's capacity.
Idle capacity is immediately preemptible back to the original assigned
reservation as needed, regardless of the priority of the query that needs the
resources. This happens automatically in real time.

To disable this functionality and force a reservation to use only the slots
provisioned to it, set `ignore_idle_slots` to `true`. Reservations with
`ignore_idle_slots` set to `true` receive no idle slots.

You cannot share idle slots between reservations of different
[editions](https://docs.cloud.google.com/bigquery/docs/editions-intro). You can share only the baseline
slots or committed slots. [Autoscaled slots](https://docs.cloud.google.com/bigquery/docs/slots-autoscaling-intro)
might be temporarily available, but are not shareable as they may scale down.

As long as `ignore_idle_slots` is false, a reservation can have a slot count of
`0` and still have access to unused slots. If you are only using the `default`
reservation, we recommend setting it up this way. You may then
[assign a project or folder](https://docs.cloud.google.com/bigquery/docs/reservations-assignments#assign_my_prod_project_to_prod_reservation)
to that reservation and it will only use idle slots.

Assignments of type `ML_EXTERNAL` are an exception to the behavior described
earlier. Slots used by BigQuery ML external model creation jobs are not preemptible;
that is, the slots in a reservation with both ml_external and query assignment
types are only available for other query jobs when the slots are not occupied by the `ML_EXTERNAL` jobs.
Also, these jobs don't use idle slots from other reservations.

## Limitations

- Reservations that you buy cannot be shared with other organizations.
- You must create a separate reservation and a separate administration project for each organization.
- Each organization can have a maximum of 10 administration projects with active commitments in a single location.
- Idle capacity cannot be shared between organizations or between different administration projects within a single organization.
- Commitments are a regional resource. Commitments purchased in one region or multi-region cannot be used in any other regions or multi-regions. Commitments cannot be moved between regions or between regions and multi-regions.
- Commitments purchased in one administration project cannot be moved to a different administration project.
- Commitments purchased with one [edition](https://docs.cloud.google.com/bigquery/docs/editions-intro) cannot be used with reservations of another edition.
- Idle slots are not shared between reservations of different [editions](https://docs.cloud.google.com/bigquery/docs/editions-intro).
- [Autoscaled slots](https://docs.cloud.google.com/bigquery/docs/slots-autoscaling-intro) are not shareable as they will scale down when no longer required.

## Quotas

Your slot quota is the maximum number of slots you can
purchase in a location. You are not billed for quotas; you are
only billed for reservations and commitments. For more
information, see
[reservations quotas and limits](https://docs.cloud.google.com/bigquery/quotas#reservation-api-limits).
For information about increasing your slot quota, see
[Requesting a quota increase](https://docs.cloud.google.com/bigquery/quotas#requesting_a_quota_increase).

## Pricing

For information about pricing for reservations, see
[Flat-rate pricing](https://cloud.google.com/bigquery/pricing#flat-rate_pricing).

## What's next

- To get started with BigQuery reservations, see
  [Get started with reservations](https://docs.cloud.google.com/bigquery/docs/reservations-get-started)

- For information about on-demand billing for reservations, see
  [Combine reservations with on-demand billing](https://docs.cloud.google.com/bigquery/docs/reservations-workload-management#combine_reservations_with_on-demand_billing).