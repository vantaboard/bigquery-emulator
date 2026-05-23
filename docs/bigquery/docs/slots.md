# Understand slots

A BigQuery slot is a *virtual compute unit* used by BigQuery
to execute SQL queries, Python code, or other [job types](https://docs.cloud.google.com/bigquery/docs/managing-jobs).
During the execution of a query, BigQuery automatically determines
how many slots are used by the query. The number of slots used depends on the
amount of data being processed, the complexity of the query, and the number of
slots available. In general, access to more slots lets you run more concurrent
queries, and your complex queries can run faster. You cannot manually change the
number of slots used by BigQuery to execute queries.

## On-demand and capacity-based pricing

While all queries use slots, you have two options for how you are charged for usage,
the [on-demand pricing model](https://cloud.google.com/bigquery/pricing#on_demand_pricing)
or the [capacity-based pricing model](https://cloud.google.com/bigquery/pricing#flat_rate_pricing).

By default, you are charged using the *on-demand model* . With this model,
you are charged for the amount of data processed (measured in TiB) by
each query. Projects using the on-demand model are subject to
[per-project and per-organization slot limits](https://docs.cloud.google.com/bigquery/quotas#query_jobs) with transient burst
capability. Most users on the on-demand model find the slot capacity limits
more than sufficient. However, depending on your workload, access to more slots
might improve query performance. To check your account's slot usage, see
[Monitor health, resource utilization, and jobs](https://docs.cloud.google.com/bigquery/docs/admin-resource-charts).

With the *capacity-based model* , you pay for the slot capacity allocated for your
queries over time. This model gives you explicit control over total slot capacity.
You explicitly choose the amount of
slots to use through a [*reservation*](https://docs.cloud.google.com/bigquery/docs/reservations-workload-management).
You can specify the number of slots in a reservation as a baseline amount
which is always allocated, or as an autoscaled amount, which is
allocated when needed. Reservations with autoscaling slots scale their capacity
to accommodate your workload demands. BigQuery allocates slots
as workloads change. This lets you
configure the number of slots in a reservation based on the performance or
critical nature of the workload that uses the reservation.

## Query execution using slots

When BigQuery executes a query job, it converts the
SQL statement into an execution plan, comprised of a series of query
*stages* . Stages are in turn comprised of sets of execution *steps* .
BigQuery uses a distributed parallel
architecture to run queries. Stages model the units of work
that can be executed in parallel. Data is passed between
stages by using a
[distributed shuffle architecture](https://cloud.google.com/blog/products/gcp/separation-of-compute-and-state-in-google-bigquery-and-cloud-dataflow-and-why-it-matters),
which is discussed in more detail in this
[Google Cloud blog post](https://cloud.google.com/blog/products/bigquery/in-memory-query-execution-in-google-bigquery).

BigQuery query execution is dynamic. A query plan can be modified
while the query is being processed. Work distribution can be optimized for data
distribution as stages are added. In addition, capacity for a query's execution
can change as other queries start or finish, or as the autoscaler adds slots
to a reservation.

BigQuery can run multiple stages
[concurrently](https://en.wikipedia.org/wiki/Instruction_pipelining), can use [speculative execution](https://en.wikipedia.org/wiki/Speculative_execution)
to accelerate a query, and can [dynamically repartition](https://cloud.google.com/blog/products/gcp/no-shard-left-behind-dynamic-work-rebalancing-in-google-cloud-dataflow)
a stage to achieve optimal parallelization.

## Slot resource economy

If a query requests more slots than are available, BigQuery
queues up individual units of work and waits for slots to become available.
As progress on query execution is made, and as slots free up, these
queued up units of work get dynamically picked up for execution.

BigQuery can request any number of slots for a particular stage
of a query. The number of slots requested is not related to the amount of
capacity you purchase, but rather an indication of the most optimal
parallelization factor chosen by BigQuery for that stage. Units
of work queue up and get executed as slots become available.

When query demands exceed slots you committed to, you are not charged for
additional slots, and you are not charged for additional on-demand rates. Your
individual units of work queue up.

For example,

1. A query stage requests 2,000 slots, but only 1,000 are available.
2. BigQuery consumes all 1,000 slots and queues up the other 1,000 slots.
3. Thereafter, if 100 slots finish their work, they dynamically pick up 100 units of work from the 1,000 queued up units of work. 900 units of queued up work remain.
4. Thereafter, if 500 slots finish their work, they dynamically pick up 500 units of work from the 900 queued up units of work. 400 units of queued up work remain.

![BigQuery slots being queued when demand exceeds availability.](https://docs.cloud.google.com/static/bigquery/images/slots-scheduling.svg) BigQuery slots queued up if demand exceeds availability

If the workload requires more slots than are available to the reservation, the
job runtime can increase as the jobs wait for slots to become available. This is
known as *slot contention*. Slot contention can increase if the workload demand
is much greater than the slots available to the reservation.

### Capacity prioritization

When BigQuery experiences high demand for slot resources in a
specific region, it manages contention by prioritizing capacity. This
prioritization ensures that customers with higher-tier capacity models are
affected less. The system prioritizes capacity in the following order:

1. Enterprise Plus and Enterprise edition baselines and committed capacity.
2. Enterprise Plus autoscaled capacity.
3. Enterprise edition autoscaled capacity.
4. Standard edition and on-demand capacity.

In the event of contention in a region, Standard edition and on-demand
capacity requests are more likely to experience access delays because the system
allocates resources to higher-tier editions first.

## Fair scheduling in BigQuery

BigQuery allocates slot capacity within a single reservation
using an algorithm called *fair scheduling*.

The BigQuery scheduler enforces the equal sharing of slots among
projects with running queries within a reservation, and then within jobs of a
given project. The scheduler provides eventual fairness. During short
periods, some jobs might get a disproportionate share of slots, but the scheduler
eventually corrects this. The goal of the scheduler is to find a balance
between aggressively evicting running tasks (which results in
wasting slot time) and being too lenient (which results in jobs with long
running tasks getting a disproportionate share of the slot time).

Fair scheduling ensures that every query has access to all available slots at any
time, and capacity is dynamically and automatically re-allocated among active
queries as each query's capacity demands change. Queries complete and new
queries get submitted for execution under the following conditions:

- Whenever a new query is submitted, capacity is automatically re-allocated across executing queries. Individual units of work can be gracefully paused, resumed, and queued up as more capacity becomes available to each query.
- Whenever a query completes, capacity consumed by that query automatically becomes immediately available for all other queries to use.
- Whenever a query's capacity demands change due to changes in query's dynamic DAG, BigQuery automatically re-evaluates capacity availability for this and all other queries, re-allocating and pausing slots as necessary.

![Fair scheduling of BigQuery slots between multiple queries.](https://docs.cloud.google.com/static/bigquery/images/slots-scheduling-multiple-queries.svg) Fair scheduling in BigQuery

Depending on complexity and size, a query might not require all the slots it has
the right to, or it might require more. BigQuery dynamically
ensures that, given fair scheduling, all slots can be fully used at any
point in time.

If an important job consistently needs more slots than it receives from the
scheduler, consider creating an additional reservation with the required number
slots and assigning the job to that reservation.

As an example of fair scheduling, suppose you have the following reservation
configuration:

- Reservation `A`, which has 1,000 baseline slots with no autoscaling
- Project `A` and project `B`, which are assigned to your reservation

Scenario 1: In project `A`, you run query `A` (one concurrent query) that requires high slot usage, and in project `B` you
run 20 concurrent queries. Even though there are a total of 21 queries that are
using reservation `A`, the slot distribution is the following:

- Project `A` receives 500 slots, and query `A` runs with 500 slots.
- Project `B` receives 500 slots that are shared among its 20 queries.

Scenario 2: In project `A`, you run query `A` (one concurrent query) that
requires 100 slots to run, and in project `B` you run 20 concurrent queries.
Since query `A` doesn't require 50% of the reservation, then the slot
distribution is the following:

- Project `A` receives 100 slots, and query `A` runs with 100 slots.
- Project `B` receives 900 slots that are shared among its 20 queries.

Inversely, consider the following reservation configuration:

- Reservation `B`, which has 1,000 baseline slots with no autoscaling.
- 10 projects, which are all assigned to reservation `B`.

Assume the 10 projects are running queries that have sufficient slot demand, then each project receives 1/10 of the total
reservation slots (or 100 slots), regardless of how many queries are running on each project.

## Slot quotas and limits

Slot quotas and limits provide a safeguard for BigQuery. Different pricing models use different slot quota types, as follows:

- On-demand pricing model: You are subject to a [per-project and organization slot limit](https://docs.cloud.google.com/bigquery/quotas#query_jobs)
  with transient burst capability. Depending on your workloads, access to more slots can improve query performance.

- Capacity-based pricing model: [Reservations quotas and limits](https://docs.cloud.google.com/bigquery/quotas#reservation-api-limits)
  define the maximum number of slots you can allocate across all reservations in a location.
  If you use autoscaling, the sum of your maximum reservation sizes cannot
  exceed this limit. You are
  only billed for your reservations and commitments, not for the quotas.
  For information about increasing your slot quota, see [Requesting a quota increase](https://docs.cloud.google.com/bigquery/quotas#requesting_a_quota_increase).

To check how many slots you are using, see [BigQuery monitoring](https://docs.cloud.google.com/bigquery/docs/admin-resource-charts).

## Idle slots

At any given time, some slots might be idle. This can include:

- Slot commitments that are not allocated to any reservation baseline.
- Slots that are allocated to a reservation baseline but aren't in use.

Idle slots are not applicable when using the on-demand pricing model.

By default, queries running in a reservation automatically use idle slots from
other reservations within the same region and administration project.
BigQuery immediately allocates idle slots to an assigned
reservation when they are needed. Idle slots that were in use by another
reservation are quickly preempted if required by the original reservation. There
might be a short time when you see total slot consumption exceed the maximum you
specified across all reservations, but you aren't charged for this additional
slot usage.

For example, suppose you have the following reservation setup:

- `project_a` is assigned to `reservation_a`, which has 500 baseline slots with no autoscaling.
- `project_b` is assigned to `reservation_b`, which has 100 baseline slots with no autoscaling.
- Both reservations are in the same region and administrative project and there are no other projects assigned to these reservations.

You run `query_b` in `project_b`. If no query is running in `project_a`, then
`query_b` has access to the 500 idle slots from `reservation_a`. While `query_b`
is still running, it might use up to 600 slots: 100 baseline slots plus 500 idle
slots.

While `query_b` is running, suppose you run `query_a` in `project_a` that can
use 500 slots.

- Since you have 500 baseline slots reserved for `project_a`, `query_a` immediately starts and is allocated 500 slots.
- The number of slots allocated to `query_b` quickly decreases to 100 baseline slots.
- Additional queries run in `project_b` share those 100 slots. If subsequent queries don't have enough slots to start, then they queue up until running queries complete and slots become available.

In this example, if `project_b` was assigned to a reservation with no baseline
slots or autoscaling, then `query_b` would have no slots after `query_a` starts
running. BigQuery would pause `query_b` until idle slots are
available or the query times out. Additional queries in `project_b` would queue
up until idle slots are available.

To ensure a reservation only uses its provisioned
slots, set `ignore_idle_slots` to `true`. Reservations with `ignore_idle_slots`
set to `true` can, however, share their idle slots with other reservations.

You cannot share idle slots between reservations of different
[editions](https://docs.cloud.google.com/bigquery/docs/editions-intro). You can share only the baseline slots
or committed slots. Autoscaled slots
might be temporarily available but are not shareable as idle slots for other reservations because they might scale
down.

As long as `ignore_idle_slots` is false, a reservation can have a slot count of
`0` and still have access to unused slots. If you use only the `default`
reservation, toggle off `ignore_idle_slots` as a best practice. You can
then [assign a project or
folder](https://docs.cloud.google.com/bigquery/docs/reservations-assignments#assign_my_prod_project_to_prod_reservation)
to that reservation and it will only use idle slots.

Assignments of type `ML_EXTERNAL` are an exception in that slots used by
BigQuery ML external model creation jobs are not preemptible. The
slots in a reservation with both `ML_EXTERNAL` and `QUERY` assignment types
are only available for other query jobs when the slots are not occupied by the
`ML_EXTERNAL` jobs. Moreover, these jobs cannot use idle slots from other
reservations.

### Reservation-based fairness

> [!NOTE]
> **Note:** You must [enable reservation-based
> fairness](https://docs.cloud.google.com/bigquery/docs/reservations-tasks#fairness) before you can [create a
> predictable reservation](https://docs.cloud.google.com/bigquery/docs/reservations-tasks#predictable).

With reservation-based fairness, BigQuery prioritizes and allocates idle slots equally across all
reservations within the same [admin
project](https://docs.cloud.google.com/bigquery/docs/reservations-workload-management#admin-project),
regardless of the number of projects running jobs in each reservation. Each
reservation receives a similar share of available capacity in the
idle slot pool, and then its slots are distributed fairly within its
projects. This feature is only supported with the Enterprise or Enterprise Plus editions.

The following chart shows how idle slots are distributed without reservation-based fairness enabled:

![Idle slots are shared across projects.](https://docs.cloud.google.com/static/bigquery/images/project-fairness-idle-slots.svg)

In this chart, idle slots are shared equally across projects.

Without reservation-based fairness enabled, the available idle slots are
distributed evenly across the projects within the reservations.

The following chart shows how idle slots are distributed with reservation-based
fairness enabled:

![Idle slots are shared across reservations.](https://docs.cloud.google.com/static/bigquery/images/reservation-fairness-idle-slots.svg)

In this chart, idle slots are shared equally across reservations, not projects.

With reservation-based fairness enabled, the available idle slots are equally
distributed across the reservations.

When you enable reservation-based fairness, review your resource consumption to
manage your slot availability and query performance.

Avoid relying solely on idle slots for production workloads with strict time
requirements - these jobs must use baseline or autoscaled slots. We recommend
using idle slots for lower priority jobs because the slots can be preempted
at any time.

## Slot autoscaling

The following section discusses autoscaling slots and how they work with
reservations.

### Use autoscaling reservations

You don't need to purchase slot commitments before creating autoscaling
reservations. Slot commitments provide a discounted rate for consistently used
slots but are optional with autoscaling reservations. To create an autoscaling
reservation, you assign a reservation a maximum number of slots (the max
reservation size). You can identify the maximum number of autoscaling slots by
subtracting the max reservation size by any optional baseline slots assigned to
the reservation.

When you create autoscaling reservations, consider the following:

- BigQuery scales reservations almost instantly until it has reached the number of slots needed to execute the jobs, or it reaches the maximum number of slots available to the reservation. Slots always autoscale to a multiple of 50.
- Scaling up is based on actual usage, and is rounded up to the nearest 50 slot increment.
- Your autoscaled slots are charged at [capacity compute pricing](https://cloud.google.com/bigquery/pricing#capacity_compute_analysis_pricing) for your associated edition while scaling up. You are charged for the number of scaled slots, not the number of slots used. This charge applies even if the job that causes BigQuery to scale up fails. For this reason, don't use the [jobs information schema](https://docs.cloud.google.com/bigquery/docs/information-schema-jobs) to match the billing. Instead, see [Monitor
  autoscaling with information schema](https://docs.cloud.google.com/bigquery/docs/reservations-monitoring#monitor_autoscaling_with_information_schema).
- While the number of slots always scales by multiples of 50, it might scale more than 50 slots within one step. For example, if your workload requires an additional 450 slots, BigQuery can attempt to scale by 450 slots at once to meet the capacity requirement.
- BigQuery scales down when the jobs associated with the reservation no longer need the capacity (subject to a 1 minute minimum).

Any autoscaled capacity is retained for at least 60 seconds. This 60-second
period is called the scale-down window. Any new peak in capacity resets the
scale-down window, treating the entire capacity level as a new grant. However,
if 60 seconds or more have passed since the last capacity increase and there is
less demand, the system reduces the capacity without resetting the scale-down
window, enabling consecutive decreases without an imposed delay.

For example, if your initial workload capacity scales to 100 slots, the peak is
retained for at least 60 seconds. If, during that scale-down window, your
workload scales to a new peak of 200 slots, a new scale-down window begins for
60 seconds. If there is no new peak during this scale-down window, your workload
begins to scale down at the end of the 60 seconds.

Consider the following detailed example: At 12:00:00, your initial capacity
scales to 100 slots and the usage lasts for one second. That peak is retained
for at least 60 seconds, beginning at 12:00:00. After the 60 seconds have
elapsed (at 12:01:01), if the new usage is 50 slots, BigQuery scales
down to 50 slots. If, at 12:01:02, the new usage is 0 slots,
BigQuery again scales down immediately to 0 slots. After the
scale-down window has ended, BigQuery can scale down multiple
times consecutively without requiring a new scale-down window.

To learn how to work with autoscaling, see [Work with autoscaling slots](https://docs.cloud.google.com/bigquery/docs/reservations-tasks).

### Using reservations with baseline and autoscaling slots

In addition to specifying the maximum reservation size, you can **optionally**
specify a baseline number of slots per reservation. The baseline is the minimum
number of slots that will always be allocated to the reservation, and you will
always be charged for them. Autoscaling slots are only added after all of the
baseline slots (and idle slots if applicable) are consumed. You can share idle
baseline slots in one reservation with other reservations that need capacity.

You can increase the number of baseline slots in a reservation every few
minutes. If you want to decrease your baseline slots, you are limited to once an
hour if you have recently changed your baseline slot capacity and your baseline
slots exceed your committed slots. Otherwise, you can decrease your baseline
slots every few minutes.

Baseline and autoscaling slots are intended to provide capacity based on your
recent workload. If you anticipate a large workload that is very different from
your workloads in the recent past, we recommend increasing your baseline
capacity ahead of the event rather than rely on autoscaling slots to cover the
workload capacity. If you encounter an issue with increasing your baseline
capacity, retry the request after waiting 15 minutes.

If the reservation doesn't have baseline slots or is not configured to borrow
[idle slots](https://docs.cloud.google.com/bigquery/docs/slots#idle_slots) from other
reservations, then BigQuery attempts to scale.
Otherwise, baseline slots must be fully utilized before scaling.

Reservations use and add slots in the following priority:

1. Baseline slots.
2. Idle slot sharing (if enabled). Reservations can only share idle baseline or committed slots from other reservations that were created with the same edition and the same region.
3. Autoscale slots.

In the following example, slots scale from a specified baseline amount. The
`etl` and `dashboard` reservations have a baseline size of 700 and 300 slots
respectively.

![Autoscaling example with no commitments.](https://docs.cloud.google.com/static/bigquery/images/autoscaling-example-no-commitment.png)

In this example, the `etl` reservation can scale to 1300 slots (700 baseline
slots plus 600 autoscale slots). If the `dashboard` reservation is not in use,
the `etl` reservation can use the 300 slots from the `dashboard` reservation
if no job is running there, leading to a maximum of 1600 possible slots.

The `dashboard` reservation can scale to 1100 slots (300 baseline slots plus
800 autoscale slots). If the `etl` reservation is totally idle, the
`dashboard` reservation can scale to a maximum of 1800 slots (300 baseline
slots plus 800 autoscale slots plus 700 idle slots in the `etl` reservation).

If the `etl` reservation requires more than 700 baseline slots, which are
always available, it attempts to add slots by using the following methods in
order:

1. 700 baseline slots.
2. [Idle slot](https://docs.cloud.google.com/bigquery/docs/slots#idle_slots) sharing with the 300 baseline slots in the `dashboard` reservation. Your reservation only shares idle baseline slots with other reservations that are created with the same edition.
3. Scaling up 600 additional slots to the maximum reservation size.

### Using slot commitments

The following example shows autoscaling slots using capacity commitments.

![Autoscaling reservations with capacity commitments.](https://docs.cloud.google.com/static/bigquery/images/autoscaling-example.png)

Like reservation baselines, slot commitments allow you to allocate a fixed
number of slots that are available to all reservations. Unlike baseline slots, a
commitment cannot be reduced during the term. Slot commitments are *optional*
but can save costs if baseline slots are required for long periods of time. Slot
commitments are used to cover baseline slots for your reservations. Any unused
slot capacity is then shared as idle slots across other reservations. Slot
commitments don't apply to autoscaling slots. To ensure that you
receive the discounted rate for your committed slots, make sure that your
slot commitments are sufficient to cover your baseline slots.

In this example, you are charged a predefined rate for the capacity commitment
slots. You are charged at the autoscaling rate for the number of autoscaling
slots after autoscaling activates and reservations are in an upscaled state.
For autoscaling rate, you are charged for the number of scaled slots,
not the number of slots used.

The following example shows reservations when the number of baseline slots
exceeds the number of committed slots.

![Baseline slots exceed the number of committed slots.](https://docs.cloud.google.com/static/bigquery/images/baseline-exceed-commitments.svg)

In this example, there is a total of 1000 baseline slots between the two
reservations, 500 from the `etl` reservation and 500 from the `dashboard`
reservation. However, the commitment only covers 800 slots. In this scenario,
the excess slots are charged at the pay as you go (PAYG) rate.

### Maximum available slots

You can calculate the maximum number of slots a reservation can use by adding
the baseline slots, the maximum number of autoscale slots, and any slots in
commitments that were created with the same edition and are not covered by the
baseline slots. The following example is set up as follows:

![Autoscaling reservations with capacity commitments.](https://docs.cloud.google.com/static/bigquery/images/autoscaling-example.png)

- A capacity commitment of 1000 annual slots. Those slots are assigned as baseline slots in the `etl` reservation and the `dashboard` reservation.
- 700 baseline slots assigned to the `etl` reservation.
- 300 baseline slots assigned to the `dashboard` reservation.
- Autoscale slots of 600 for the `etl` reservation.
- Autoscale slots of 800 for the `dashboard` reservation.

For the `etl` reservation, the maximum number of slots possible is equal to
the `etl` baseline slots (700) plus the `dashboard` baseline slots (300, if
all slots are idle) plus the maximum number of autoscale slots (600). So the
maximum number of slots the `etl` reservation could use in this example is
1600. This number exceeds the number in the capacity commitment.

In the following example, the annual commitment exceeds the assigned baseline slots.

![How to calculate the maximum available slots in a reservation.](https://docs.cloud.google.com/static/bigquery/images/max-available-slots-calculation.png)

In this example, we have:

- A capacity commitment of 1600 annual slots.
- A maximum reservation size of 1500 (including 500 autoscaling slots).
- 1000 baseline slots assigned to the `etl` reservation.

The maximum number of slots available to the reservation is equal to the
baseline slots (1000) plus any committed idle slots not dedicated to the
baseline slots (1600 annual slots - 1000 baseline slots = 600) plus the number
of autoscaling slots (500). So the maximum potential slots in this reservation
is 2100. The autoscaled slots are additional slots above the capacity
commitment.

### Autoscaling best practices

- When first using autoscaler, set the number of autoscaling slots to a
  meaningful number based on past and expected performance. Once the
  reservation is created, actively monitor the failure rate, performance, and
  bill and adjust the number of autoscaling slots as needed.

- Autoscaler has a 1 minute minimum before scaling down so it is important to
  set the maximum number of autoscaled slots to balance between performance and
  cost. If the maximum number of autoscale slots is too large and your job can
  use all the slots to complete a job in seconds, you still incur costs for the
  maximum slots for the full minute. If you lower your max slots to half the
  current amount, your reservation is scaled to a lower number and the job can
  use more `slot_seconds` during that minute, reducing waste. For help
  determining your slot requirements, see [Monitor job
  performance](https://docs.cloud.google.com/bigquery/docs/admin-resource-charts#monitor_job_performance). As an alternative approach to
  determine your slot requirements, see [View edition slot
  recommendations](https://docs.cloud.google.com/bigquery/docs/slot-recommender).

-
  Slot usage can occasionally exceed the sum of your baseline plus scaled slots.
  You aren't billed for slot usage that's greater than your baseline plus scaled
  slots.

- Autoscaler is most efficient for heavy, long-running workloads, such as
  workloads with multiple concurrent queries. Avoid sending queries one at a
  time, since each query scales the reservation where it will
  remain scaled for a 1-minute minimum. If you continuously send queries,
  causing a constant workload, setting a baseline and buying a commitment
  provides constant capacity at a discounted price.

- BigQuery autoscaling is subject to capacity availability.
  BigQuery attempts to meet customer capacity demand
  based on historical usage.
  To achieve capacity guarantees, you can set an optional slot
  baseline, which is the number of guaranteed slots in a reservation.
  With baselines, slots are immediately available and you pay for them
  whether you use them or not. To ensure capacity is available for large,
  inorganic demands, such as high-traffic holidays, contact
  [the BigQuery team](https://cloud.google.com/support)
  several weeks in advance.

- Baseline slots are always charged. If a [capacity
  commitment](https://docs.cloud.google.com/bigquery/docs/reservations-commitments) expires, you might need
  to manually adjust the amount of baseline slots in your reservations to avoid
  any unwanted charges. For example, consider that you have a 1-year commitment
  with 100 slots and a reservation with 100 baseline slots. The commitment
  expires and doesn't have a renewal plan. Once the commitment expires, you pay
  for 100 baseline slots at the [pay as you go
  rate](https://cloud.google.com/bigquery/pricing#capacity_compute_analysis_pricing).

### Monitor autoscaling

For information about monitoring slot usage and job performance with autoscaling, see [Monitor autoscaling](https://docs.cloud.google.com/bigquery/docs/admin-resource-charts#monitor-autoscaling).

## Excess slot usage

When a job holds onto slots for too long, it can receive an unfair share of slots.
To prevent delays, BigQuery allows other jobs to *borrow* additional
slots, resulting in periods of total slot use above your specified slot capacity.
Any excess slot usage is attributed only to the jobs that receive more than their fair share.

The excess slots are not billed directly to you. Instead, jobs continue to
run and accrue slot usage at their fair share until all of their excess
usage is covered by your allocated capacity. Excess slots are excluded from
reported slot usage with the exception of certain detailed
execution statistics.

Note that some preemptive borrowing of slots can occur to reduce future
delays and to provide other benefits such as reduced slot cost variability
and reduced tail latency. Slot borrowing is limited to a small fraction of
your total slot capacity.