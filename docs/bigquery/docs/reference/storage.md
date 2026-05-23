# Use the BigQuery Storage Read API to read table data

<br />

The BigQuery Storage Read API provides fast access to
BigQuery-managed storage by using an
[rpc-based](https://docs.cloud.google.com/bigquery/docs/reference/storage/rpc)
protocol.

## Background

Historically, users of BigQuery have had two mechanisms for
accessing BigQuery-managed table data:

- Record-based paginated access by using the `tabledata.list` or
  `jobs.getQueryResults` REST API methods. The BigQuery API
  provides structured row responses in a paginated fashion appropriate for small
  result sets.

- Bulk data export using BigQuery `extract` jobs that export table
  data to Cloud Storage in a variety of file formats such as CSV, JSON,
  and Avro. Table exports are limited by daily quotas and by the batch
  nature of the export process.

The BigQuery Storage Read API provides a third option that represents an
improvement over prior options. When you use the Storage Read API,
structured data is sent over the wire in a binary serialization format. This
allows for additional parallelism among multiple consumers for a set of results.

The Storage Read API does not provide functionality related to
managing BigQuery resources such as datasets, jobs, or tables.

## Key features

- **Multiple Streams**: The Storage Read API allows consumers to
  read disjoint sets of rows from a table using multiple streams within a
  session. This facilitates consumption from distributed processing frameworks
  or from independent consumer threads within a single client.

- **Column Projection**: At session creation, users can select an optional
  subset of columns to read. This allows efficient reads when tables contain
  many columns.

- **Column Filtering**: Users may provide simple filter predicates to enable
  filtration of data on the server side before transmission to a client.

- **Snapshot Consistency**: Storage sessions read based on a snapshot
  isolation model. All consumers read based on a specific point in time.
  The default snapshot time is based on the session creation time, but consumers
  may read data from an earlier snapshot.

## Enabling the API

The Storage Read API is distinct from the BigQuery API, and
shows up separately in the Google Cloud console as the **BigQuery Storage API**.
However, the Storage Read API is enabled in all projects in which
the BigQuery API is enabled; no additional activation steps are required.

## Permissions


To get the permissions that
you need to create and update read sessions,

ask your administrator to grant you the
Read Session User (`bigquery.readSessionUser`)
IAM role on the project.


For more information about granting roles, see [Manage access to projects, folders, and organizations](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).


This predefined role contains

the permissions required to create and update read sessions. To see the exact permissions that are
required, expand the **Required permissions** section:


#### Required permissions

The following permissions are required to create and update read sessions:

- `bigquery.readsessions.create` on the project
- `bigquery.readsessions.getData` on the table or higher
- `bigquery.readsessions.update` on the table or higher


You might also be able to get
these permissions
with [custom roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles) or
other [predefined roles](https://docs.cloud.google.com/iam/docs/roles-overview#predefined).

For more information about BigQuery roles and permissions, see
[BigQuery IAM roles and permissions](https://docs.cloud.google.com/bigquery/docs/access-control).

## Basic API flow

This section describes the basic flow of using the Storage Read API. For
examples, see the [libraries and samples page](https://docs.cloud.google.com/bigquery/docs/reference/storage/samples).

### Create a session

Storage Read API usage begins with the creation of a read session. The
maximum number of streams, the snapshot time, the set of columns to return, and
the predicate filter are all specified as part of the `ReadSession` message
supplied to the `CreateReadSession` RPC.

The `ReadSession` response contains a set of `Stream` identifiers. When a read
session is created, the server determines the amount of data that can be read in
the context of the session and creates one or more streams, each of which
represents approximately the same amount of table data to be scanned. This means
that, to read all the data from a table, callers must read from all `Stream`
identifiers returned in the `ReadSession` response. This is a change from
earlier versions of the API, in which no limit existed on the amount of data
that could be read in a single stream context.

The `ReadSession` response contains a reference schema for the session and a
list of available `Stream` identifiers. Sessions expire automatically and do not
require any cleanup or finalization. The expiration time is returned as part of
the `ReadSession` response and is guaranteed to be at least 6 hours from session
creation time.

### Read from a session stream

Data from a given stream is retrieved by invoking the `ReadRows` streaming RPC.
Once the read request for a `Stream` is initiated, the backend will begin
transmitting blocks of serialized row data. RPC flow control ensures that
the server does not transmit more data when the client is not ready to receive.
If the client does not request data for more than 1 hour, then the server
suspects that the stream is stalled and closes it to free up resources for other
streams. If there is an error, you can restart reading a stream at a particular
point by supplying the row offset when you call `ReadRows`.

To support dynamic work rebalancing, the Storage Read API provides an
additional method to split a `Stream` into two child `Stream` instances whose
contents are, together, equal to the contents of the parent `Stream`. For more
information, see the [API reference](https://docs.cloud.google.com/bigquery/docs/reference/storage/rpc).

### Decode row blocks

Row blocks must be deserialized once they are received. Users of
the Storage Read API may specify all data in a session to be serialized
using either Apache Avro format, or Apache Arrow.

The reference schema is sent as part of the initial `ReadSession` response,
appropriate for the data format selected. In most cases, decoders can be
long-lived because the schema and serialization are consistent among all streams
and row blocks in a session.

## Schema conversion

### Avro schema details

Due to type system differences between BigQuery and the Avro
specification, Avro schemas may include additional annotations that identify how
to map the Avro types to BigQuery representations. When compatible,
Avro base types and logical types are used. The Avro schema may also include
additional annotations for types present in BigQuery that do not
have a well defined Avro representation.

To represent nullable columns, unions with the Avro `NULL` type are used.

| GoogleSQL type | Avro type | Avro schema annotations | Notes |
|---|---|---|---|
| `BOOLEAN` | boolean |
| `INT64` | long |
| `FLOAT64` | double |
| `BYTES` | bytes |
| `STRING` | string |
| `DATE` | int | logicalType: date |
| `DATETIME` | string | logicalType: datetime |
| `TIMESTAMP` | long | logicalType: timestamp-micros |
| `TIME` | long | logicalType: time-micros |
| `NUMERIC` | bytes | logicalType: decimal (precision = 38, scale = 9) |
| `NUMERIC(P[, S])` | bytes | logicalType: decimal (precision = P, scale = S) |
| `BIGNUMERIC` | bytes | logicalType: decimal (precision = 77, scale = 38) |
| `BIGNUMERIC(P[, S])` | bytes | logicalType: decimal (precision = P, scale = S) |
| `GEOGRAPHY` | string | sqlType: GEOGRAPHY |
| `ARRAY` | array |
| `STRUCT` | record |
| `JSON` | string | sqlType: JSON |
| `RANGE<T>` | record | sqlType: RANGE | Contains the following fields: - `start`, with union type `["null", AVRO_TYPE(T)]` - `end`, with union type `["null", AVRO_TYPE(T)]` `AVRO_TYPE(T)` is the Avro type representation for the range element type `T`. A null field denotes an unbounded range boundary. The first `RANGE` field of type `T` (for example, `range_date_1`) specifies the full Avro record structure under the namespace `google.sqlType` and with the name `RANGE_T` (for example, `RANGE_DATE`). Subsequent `RANGE` fields of the same type `T` (for example, `range_date_2`) references the corresponding Avro record structure by using the full resolution name, `google.sqlType.RANGE_T` (for example, `google.sqlType.RANGE_DATE`). { "name": "range_date_1", "type": { "type": "record", "namespace": "google.sqlType", "name": "RANGE_DATE", "sqlType": "RANGE", "fields": [ { "name": "start", "type": ["null", {"type": "int", "logicalType": "date"}] }, { "name": "end", "type": ["null", {"type": "int", "logicalType": "date"}] }, ] } }, { "name": "range_date_2", "type": "google.sqlType.RANGE_DATE" } |

### Arrow schema details

The Apache Arrow format works well with Python data science workloads.

For cases where multiple BigQuery types converge on a single
Arrow data type, the metadata property of the Arrow schema field indicates
the original data type.

If you're working in an older version of the Storage Read API, then
use the appropriate version of Arrow as follows:

- v1beta1: Arrow 0.14 and earlier
- v1: Arrow 0.15 and later

Regardless of API version, to access API functions, we recommend that you use
the [BigQuery Storage API client libraries](https://docs.cloud.google.com/bigquery/docs/reference/libraries). The libraries can
be used with any version of Arrow and don't obstruct its updates.

| GoogleSQL type | Arrow logical type | Notes |
|---|---|---|
| `BOOLEAN` | Boolean |
| `INT64` | Int64 |
| `FLOAT64` | Double |
| `BYTES` | Binary |
| `STRING` | Utf8 |
| `DATE` | Date | 32-bit days since epoch |
| `DATETIME` | Timestamp | Microsecond precision, no timezone |
| `TIMESTAMP` | Timestamp | Microsecond precision, UTC timezone |
| `TIME` | Time | Microsecond precision |
| `NUMERIC` | Decimal | Precision = 38, scale = 9 |
| `NUMERIC(P[, S])` | Decimal | Precision = P, scale = S |
| `BIGNUMERIC` | Decimal256 | Precision = 76, scale = 38 |
| `BIGNUMERIC(P[, S])` | Decimal256 | Precision = P, scale = S |
| `GEOGRAPHY` | Utf8 |
| `ARRAY` | List |
| `STRUCT` | Struct |
| `JSON` | Utf8 |
| `RANGE<T>` | Struct | Contains the following fields: - `start`, with `ARROW_TYPE(T)` - `end`, with `ARROW_TYPE(T)` `ARROW_TYPE(T)` is the Arrow type representation of the range element type `T`. A null field denotes an unbounded range boundary. For example, `RANGE<DATE>` is represented as a struct with two Arrow `Date` fields. <br /> |

## Limitations

- Because the Storage Read API operates on storage,
  you cannot use the Storage Read API to directly read from logical or
  materialized views. As a workaround, you can execute a BigQuery
  query over the view and use the Storage Read API to read from the
  resulting table. Some connectors, including the
  [Spark-BigQuery connector](https://docs.cloud.google.com/dataproc/docs/tutorials/bigquery-connector-spark-example),
  support this workflow natively.

- Reading [external tables](https://docs.cloud.google.com/bigquery/docs/external-tables) is not supported. To
  use the Storage Read API with external data sources, use
  [BigLake tables](https://docs.cloud.google.com/bigquery/docs/biglake-intro).

## Supported regions

The Storage Read API is supported in the same regions as
BigQuery. See the
[Dataset locations](https://docs.cloud.google.com/bigquery/docs/locations) page for a
complete list of supported regions and multi-regions.

### Data locality

Data locality is the process of moving the computation closer to the location
where the data resides. Data locality impacts both the peak throughput and
consistency of performance.

BigQuery
determines the location to run your load, query, or extract jobs based on the
datasets referenced in the request. For information about location considerations,
see [BigQuery locations](https://docs.cloud.google.com/bigquery/docs/locations).

## Troubleshoot errors

The following are common errors encountered when using the
Storage Read API:

Error: `Stream removed`
:   **Resolution:** Retry the Storage Read API request. This is likely
    a transient error that can be resolved by retrying the request. If the problem
    persists, [contact support](https://docs.cloud.google.com/bigquery/docs/getting-support).

Error: `Stream expired`

:   **Cause:** This error occurs when the Storage Read API session
    reaches the [6 hour timeout](https://docs.cloud.google.com/bigquery/docs/reference/storage#create_a_session).

:   **Resolution:**

1. Increase the parallelism of the job.
2. If the CPU utilization of the worker nodes is relatively consistent and doesn't spike above 85%, consider running the job on a larger machine type.
3. Split the job into multiple jobs or smaller queries.

## Quotas and limits

For Storage Read API quotas and limits, see
[Storage Read API limits](https://docs.cloud.google.com/bigquery/quotas#storage-limits).

## Monitor Storage Read API use

To monitor the data egress and processing associated with the
Storage Read API, specific fields are available in the
[BigQuery AuditLogs](https://docs.cloud.google.com/bigquery/docs/reference/auditlogs).
These logs provide a detailed view of the bytes scanned and the bytes returned
to the client.

The relevant API method for these logs is
`google.cloud.bigquery.storage.v1.BigQueryRead.ReadRows`.

| Field name | Data type | Notes |
|---|---|---|
| `serialized_response_bytes` | INT64 | The total number of bytes sent to the client over the network, after serialization. This field helps you track data egress. |
| `scanned_bytes` | INT64 | The total number of bytes scanned from BigQuery storage to fulfill the request. This value is used to calculate the analysis cost of the read operation. |

## Pricing

For information on Storage Read API pricing, see the
[Pricing](https://cloud.google.com/bigquery/pricing#data-extraction-pricing-details) page.