# Understand reliability

This document provides an understanding of BigQuery reliability
features, such as availability, durability, data consistency, consistency of
performance, data recovery, and a review of error handling considerations.

This introduction helps you address three primary considerations:

- Determine whether BigQuery is the right tool for your job.
- Understand the dimensions of BigQuery reliability.
- Identify specific reliability requirements for specific use cases.

## Select BigQuery

[BigQuery](https://docs.cloud.google.com/bigquery/docs/introduction)
is a fully managed enterprise Data Warehouse built to store and analyze massive
datasets. It provides a way to ingest,
store, read, and query megabytes to petabytes of data with consistent
performance without having to manage any of the underlying infrastructure.
Because of its power and performance, BigQuery is well suited to
be used in a range of solutions. Some of these are documented in detail as
[reference patterns](https://docs.cloud.google.com/bigquery/docs/reference-patterns).

Generally, BigQuery is very well suited for workloads where large
amounts of data are being ingested and analyzed. Specifically, it can be
effectively deployed for use cases such as real-time and predictive data
analytics (with
[streaming ingestion](https://docs.cloud.google.com/bigquery/docs/streaming-data-into-bigquery)
and
[BigQuery ML](https://docs.cloud.google.com/bigquery/docs/bqml-introduction)),
anomaly detection, and other use cases where analyzing large volumes of data
with predictable performance is key. However, if you are looking for a database
to support Online Transaction Processing (OLTP) style applications, you should
consider other Google Cloud services such as
[Spanner](https://docs.cloud.google.com/spanner),
[Cloud SQL](https://docs.cloud.google.com/sql),
or
[Bigtable](https://docs.cloud.google.com/bigtable)
that may be better suited for these use cases.

## Dimensions of reliability in BigQuery

### Availability

Availability defines the user's ability to read data from
BigQuery or write data to it. BigQuery is built to
make both of these highly available with a 99.99%
[SLA](https://cloud.google.com/bigquery/sla).
There are two components involved in both operations:

- The BigQuery service
- Compute resources required to execute the specific query

Reliability of the service is a function of the specific BigQuery API being
used to retrieve the data. The availability of compute resources depends on the
capacity available to the user at the time when the query is run. See
[Understand slots](https://docs.cloud.google.com/bigquery/docs/slots)
for more information about the fundamental unit of compute for
BigQuery and the resulting [slot resource economy](https://docs.cloud.google.com/bigquery/docs/slots#slot_resource_economy).

### Durability

Durability is discussed in the
[Implementing SLOs chapter](https://sre.google/workbook/implementing-slos/)
of the SRE Workbook and is described as the "proportion of data that can be
successfully read."

### Data consistency

Consistency defines the expectations that users have for how the data is able
to be queried once it's written or modified. One aspect of data consistency is
ensuring "exactly-once" semantics for data ingestion. For more information,
see [Retry failed job insertions](https://docs.cloud.google.com/bigquery/docs/reliability-intro#retry_failed_job_insertions).

### Consistency of performance

In general, performance can be expressed in two dimensions. *Latency* is a
measure of the execution time of long data retrieval operations such as queries.
*Throughput* is a measure of how much data BigQuery can process
during a specific period of time. Due to BigQuery's multi-tenant,
horizontally scalable design, its throughput can scale up to arbitrary data
sizes. The relative importance of latency and throughput is determined by the
specific use case.

### Data recovery

Two ways to measure the ability to recover data after an outage are:

- *Recovery Time Objective* (RTO). How long data can be unavailable after an incident.
- *Recovery Point Objective* (RPO). How much of the data collected prior to the incident can acceptably be lost.

These considerations are specifically relevant in the unlikely case that a zone
or region experiences a multi-day or destructive outage.

## Disaster planning

While the
term "disaster" may invoke visions of natural disasters, the scope of this section
addresses specific failures from the loss of an individual machine all the way
through catastrophic loss of a region. The former are everyday occurrences that
BigQuery handles automatically, while the latter is something
that customers may need to design their architecture to handle if required.
Understanding at what scope disaster planning crosses over to customer
responsibility is important.

BigQuery offers an industry leading
[99.99% uptime SLA](https://cloud.google.com/bigquery/sla).
This is made possible by BigQuery's regional architecture that
writes data in two different zones and provisions redundant compute capacity. It
is important to keep in mind that the BigQuery SLA is the same
for regions, for example us-central1, and multi-regions, for example US.

### Automatic scenario handling

Because BigQuery is a regional service, it is
the responsibility of BigQuery to automatically handle the loss
of a machine or even an entire zone. The fact that BigQuery is
built on top of zones is abstracted from users.

#### Loss of a machine

Machine failures are an everyday occurrence at the scale at which Google
operates. BigQuery is designed to handle machine failures
automatically without any impact to the containing zone.  

Under the hood, execution of a query is broken up into small tasks that can be
dispatched in parallel to many machines. The sudden loss or degradation of
machine performance is handled automatically by redispatching work to a
different machine. Such an approach is crucial to reducing tail latency.

BigQuery utilizes
[Reed--Solomon](https://en.wikipedia.org/wiki/Binary_Reed%E2%80%93Solomon_encoding)
encoding to efficiently and durably store a zonal copy of your data. In the
highly unlikely event that multiple machine failures cause the loss of a zonal
copy, data is also stored in the same fashion in at least one other zone. In
such a case, BigQuery will detect the problem and make a new
zonal copy of the data to restore redundancy.

#### Loss of a Zone

The underlying availability of compute resources in any given zone is not
sufficient to meet BigQuery's 99.99% uptime SLA. Hence
BigQuery provides automatic zonal redundancy for both data and
compute slots. While short lived zonal disruptions are not common, they do
happen. BigQuery automation, however, will route new queries
to another zone within a minute of any severe disruption. Already inflight
queries may not immediately recover, but newly issued queries will. This would
manifest as inflight queries taking a long time to finish while newly issued
queries complete quickly.

Even if a zone were to be unavailable for a longer period of time, no data loss
would occur due to the fact that BigQuery synchronously writes
data to two zones. So even in the face of zonal loss, customers will not
experience a service disruption.

### Types of failures

There are two types of failures, soft failures and hard failures.

*Soft failure* is an operational deficiency where hardware is not destroyed.
Examples include power failure, network partition, or a machine crash. In
general, BigQuery should never lose data in the event of a soft
failure.

*Hard failure* is an operational deficiency where hardware is destroyed. Hard
failures are more severe than soft failures. Hard failure examples include
damage from floods, terrorist attacks, earthquakes, and hurricanes.

### Availability and durability

When you create a BigQuery dataset, you select a location in which
to store your data. This location is one of the following:

- A region: a specific geographical location, such as Iowa (`us-central1`) or Montréal (`northamerica-northeast1`).
- A multi-region: a large geographic area that contains two or more geographic places, such as the United States (`US`) or Europe (`EU`).

In either case, BigQuery automatically stores copies of your data
in two different Google Cloud [zones](https://docs.cloud.google.com/docs/geography-and-regions#regions_and_zones)
within a single region in the selected location.

In addition to storage redundancy, BigQuery also maintains
redundant compute capacity across multiple zones. By combining redundant storage
and compute across multiple availability zones, BigQuery provides
both high availability and durability.

> [!NOTE]
> **Note:** Selecting a multi-region location does not provide cross-region replication nor regional redundancy. Data will be stored in a single region within the geographic location.

In the event of a machine-level failure, BigQuery continues to
run with no more than a few milliseconds of delay. All currently running
queries continue processing. In the event of either a soft or hard zonal
failure, no data loss is expected. However, currently running queries
might fail and need to be resubmitted. A soft zonal failure, such as
resulting from a power outage, destroyed transformer, or network partition,
is a well-tested path and is automatically mitigated within a few minutes.

A soft regional failure, such as a region-wide loss of network connectivity,
results in loss of availability until the region is brought back online,
but it doesn't result in lost data. A hard regional failure, for example,
if a disaster destroys the entire region,
could result in loss of data stored in that region. BigQuery does
not automatically provide a backup or replica of your data in another
geographic region. You can use
[cross-region dataset replication](https://docs.cloud.google.com/bigquery/docs/data-replication) or
[managed disaster recovery](https://docs.cloud.google.com/bigquery/docs/managed-disaster-recovery) to enhance
your resiliency to hard regional failures.

To learn more about BigQuery dataset locations, see
[Location considerations](https://docs.cloud.google.com/bigquery/docs/locations#data-locations).

### Scenario: Loss of region

BigQuery does not offer durability or availability in the
extraordinarily unlikely and unprecedented event of physical region loss.
This is true for both regions and multi-regions.
Hence maintaining durability and availability under such a
scenario requires customer planning. In the case of temporary loss, such as a
network outage, redundant availability should be considered if
BigQuery's 99.99% SLA is not considered sufficient.

To avoid data loss in the face of destructive regional loss, you need to back up
data to another geographic location. For example, you could use
[cross-region dataset replication](https://docs.cloud.google.com/bigquery/docs/data-replication) to
continuously replicate your data to a geographically distinct region.

In the case of BigQuery multi-regions, you should avoid backing
up to regions within the scope of the multi-region. See
[BigQuery locations](https://docs.cloud.google.com/bigquery/docs/locations#multi-regions) for information
about the scope of the multi-regions. For example, if you are backing up data
from the US multi-region then you should avoid choosing one of the overlapping
regions such as us-central1, given the chance of correlated failure during a
disaster.

To avoid an extended unavailability, you need to have both data replicated and
slots provisioned in two geographically separate BigQuery
locations. You can use
[managed disaster recovery](https://docs.cloud.google.com/bigquery/docs/managed-disaster-recovery) to
automatically provision slots in a secondary region, and control failover of
your workloads from one region to another.

### Scenario: Accidental deletion or data corruption

By virtue of BigQuery's
[multiversion concurrency control](https://en.wikipedia.org/wiki/Multiversion_concurrency_control)
architecture, BigQuery supports
[time travel](https://docs.cloud.google.com/bigquery/docs/time-travel).
With this feature you can query data from any point in time over the last seven
days. This allows for self service restoration of any data that has been
mistakenly deleted, modified, or corrupted within a 7 day window. Time travel
even works on tables that have been deleted.

BigQuery also supports the ability to [snapshot
tables](https://docs.cloud.google.com/bigquery/docs/table-snapshots-intro). With this
feature you can explicitly backup data within the same region for longer than
the 7 day time travel window. A snapshot is purely a metadata operation and
results in no additional storage bytes. While this can add protection against
accidental deletion, it does not increase the durability of the data.

### Use case: Real-time analytics

In this use case, streaming data is being ingested from endpoint logs into
BigQuery continuously. Protecting against extended
BigQuery unavailability for the entire region requires
continuously replicating data and provisioning slots in a different region.
Given that the architecture is resilient to BigQuery
unavailability due to the
[use of Pub/Sub and Dataflow in the ingestion path](https://docs.cloud.google.com/bigquery/docs/loading-data#methods),
this high level of redundancy is likely not worth the cost.

Assuming the user has configured BigQuery data in
us-east4 to be exported nightly by using extract jobs to Cloud Storage
under the Archive Storage class in us-central1. This provides a durable backup
in case of catastrophic data loss in us-east4. In this case, the Recovery Point
Objective (RPO) is 24 hours, as the last exported backup can be up to 24 hours
old in the worst case. The Recovery Time Objective (RTO) is potentially days, as
data needs to be restored from the Cloud Storage backup to
BigQuery in us-central1. If BigQuery is to be
provisioned in a different region from where backups are placed, data needs to
be transferred to this region first. Also note that unless you have purchased
redundant slots in the recovery region in advance, there may be an additional
delay in getting the required BigQuery capacity provisioned
depending on the quantity requested.

### Use case: Batch data processing

For this use case it is business critical that a daily report is completed by a
fixed deadline to be sent to a regulator. Implementing redundancy by running two
separate instances of the entire processing pipeline is likely worth the cost.
Using two separate regions, for example, us-west1 and us-east4, provides geographic
separation and two independent failure domains in case of extended
unavailability of a region or even the unlikely event of a permanent region
loss.

Assuming we need the report to be delivered exactly once, we need to reconcile
the expected case of both pipelines finishing successfully. A reasonable
strategy is simply picking the result from the pipeline finishing first, for example, by
notifying a Pub/Sub topic on successful completion. Alternatively,
overwrite the result and re-version the Cloud Storage object. If
the pipeline finishing later writes corrupt data, you can recover by restoring
the version written by the pipeline finishing first from Cloud Storage.

## Error handling

The following are best practices for addressing errors that affect reliability.

### Retry failed API requests

Clients of BigQuery, including client libraries and partner
tools, should use
[truncated exponential backoff](https://en.wikipedia.org/wiki/Exponential_backoff)
when issuing API requests. This means that if a client receives a system error
or a quota error, it should retry the request up to a certain number of times,
but with a random and increasing backoff interval.

Employing this method of retries makes your application much more robust in
the face of errors. Even under normal operating conditions, you can expect on
the order of one in ten thousand requests to fail as described in
BigQuery's [99.99% availability SLA](https://cloud.google.com/bigquery/sla). Under
abnormal conditions, this error rate may increase, but if errors are randomly
distributed the strategy of exponential backoff can mitigate all but the most
severe cases.

If you encounter a scenario where a request fails persistently with a 5XX error,
then you should escalate to Google Cloud Support. Be sure to
[clearly communicate the impact](https://docs.cloud.google.com/support/docs/procedures#support_case_priority)
the failure is having on your business so that the issue can be triaged
correctly. If, on the other hand, a request persistently fails with a 4XX error,
the problem should be addressable by changes to your application. Read the
error message for details.

### Exponential backoff logic example

Exponential backoff logic retries a query or request by increasing the
wait time between retries up to a maximum backoff time. For example:

1. Make a request to BigQuery.

2. If the request fails, wait 1 + random_number_milliseconds seconds and retry
   the request.

3. If the request fails, wait 2 + random_number_milliseconds seconds and retry
   the request.

4. If the request fails, wait 4 + random_number_milliseconds seconds and retry
   the request.

5. And so on, up to a (`maximum_backoff`) time.

6. Continue to wait and retry up to a maximum number of retries, but
   don't increase the wait period between retries.

Note the following:

- The wait time is `min(((2^n)+random_number_milliseconds), maximum_backoff)`,
  with `n` incremented by 1 for each iteration (request).

- `random_number_milliseconds` is a random number of milliseconds less than or
  equal to 1000. This randomization helps to avoid situations where many clients
  are synchronized and all retry simultaneously, sending requests in synchronized
  waves. The value of `random_number_milliseconds` is recalculated after each
  retry request.

- The maximum backoff interval (`maximum_backoff`) is typically 32 or 64
  seconds. The appropriate value for `maximum_backoff` depends on the use case.

The client can continue retrying after it reaches the maximum backoff time.
Retries after this point don't need to continue increasing backoff time. For
example, if the client uses a maximum backoff time of 64 seconds, then after
reaching this value the client can continue to retry every 64 seconds. At some
point, clients should be prevented from retrying indefinitely.

The wait time between retries and the number of retries depend on your use case
and network conditions.

### Retry failed job insertions

If exactly-once insertion semantics are important for your application, there
are additional considerations when it comes to inserting jobs. How to achieve
at-most-once semantics depends on which
[WriteDisposition](https://docs.cloud.google.com/bigquery/docs/reference/auditlogs/rest/Shared.Types/BigQueryAuditMetadata.WriteDisposition)
you specify. The write disposition tells BigQuery what it should
do when encountering existing data in a table: fail, overwrite or append.

With a `WRITE_EMPTY` or `WRITE_TRUNCATE` disposition, this is achieved by
simply retrying any failed job insertion or execution. This is because all rows
ingested by a job are atomically written to the table.

With a `WRITE_APPEND` disposition, the client needs to specify the job ID to
guard against a retry appending the same data a second time. This works because
BigQuery rejects job creation requests that attempt to use an
ID from a previous job. This achieves at-most-once semantics for any given job
ID. You can then achieve exactly-once by retrying under a new predictable job ID
once you've confirmed with BigQuery that all previous attempts
have failed.

In some cases, the API client or HTTP client might not receive the confirmation that the job is inserted due to transient issues or network interruptions. When the insertion is retried, that request fails with `status=ALREADY_EXISTS` (`code=409` and `reason="duplicate"`). The existing job status can be retrieved with a call to `jobs.get`. After the status of the existing job is `retrieved`, the caller can determine whether a new job with a new job ID should be created.

## Use cases and reliability requirements

BigQuery might be a critical component of a variety of
architectures. Depending on the use case and architecture deployed, a variety of
availability, performance, or other reliability requirements might need to be met.
For the purposes of this guide, let's select two primary use cases and
architectures to discuss in detail.

### Real-time analytics

The first example is an event data processing pipeline. In this example, log
events from endpoints are ingested using Pub/Sub. From there, a
streaming Dataflow pipeline performs some operations on the data prior
to writing it into BigQuery using the
[Storage Write API](https://docs.cloud.google.com/bigquery/docs/write-api).
The data is then used both for ad hoc querying to, for example, recreate
sequences of events that may have resulted in specific endpoint outcomes, and
for feeding near-real time dashboards to allow the detection of trends and
patterns in the data through visualization.

This example requires you to consider multiple aspects of reliability. Because
the end-to-end data freshness requirements are quite high, **latency** of the
ingestion process is critical. Once data is written to BigQuery,
**reliability** is perceived as the ability of users to issue ad hoc queries
with **consistent** and predictable latency and ensuring that dashboards
utilizing the data reflect the absolute latest available information.

### Batch data processing

The second example is a batch processing architecture based around regulatory
compliance in the financial services industry. A key requirement is to deliver
daily reports to regulators by a fixed nightly deadline. As long as the nightly
batch process that generates the reports completes by this deadline, it is
considered sufficiently fast.

Data needs to be made available in BigQuery and joined with other
data sources for dashboarding, analysis, and ultimately generation of a PDF
report. Having these reports be delivered on time and without error is a
critical business requirement. As such, ensuring the **reliability** of both
data ingestion and producing the report correctly and in a **consistent**
timeframe to meet required deadlines are key.