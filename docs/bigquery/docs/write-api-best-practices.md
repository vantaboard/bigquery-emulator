# BigQuery Storage Write API best practices

This document gives best practices for using the BigQuery Storage Write API. Before
reading this document, read
[Overview of the BigQuery Storage Write API](https://docs.cloud.google.com/bigquery/docs/write-api#overview).

## Limit the rate of stream creation

Before creating a stream, consider whether you can use the
[default stream](https://docs.cloud.google.com/bigquery/docs/write-api#default_stream). For streaming
scenarios, the default stream has fewer quota limitations and can scale better
than using application-created streams. If you use an application-created
stream, then make sure to utilize the maximum throughput on each stream before
creating additional streams. For example, use
[asynchronous writes](https://docs.cloud.google.com/bigquery/docs/write-api-best-practices#do_not_block_on_appendrows_calls).

For application-created streams, avoid calling `CreateWriteStream` at a high
frequency. Generally, if you exceed 40-50 calls per second, the latency of the
API calls grows substantially (\>25s). Make sure your application can accept a
cold start and ramp up the number of streams gradually, and limit the rate of
`CreateWriteStream` calls. You might also set a larger deadline to wait for the
call to complete, so that it doesn't fail with a `DeadlineExceeded` error. There
is also a longer-term [quota](https://docs.cloud.google.com/bigquery/quotas#createwritestream) on the maximum
rate of `CreateWriteStream` calls. Creating streams is a resource-intensive
process, so reducing the rate of stream creations and fully utilizing existing
streams is the best way to not run over this limit.

## Connection pool management

The `AppendRows` method creates a bidirectional connection to a stream. You can
open multiple connections on the default stream, but only a single active
connection on application-created streams.

When using the default stream, you can use Storage Write API
multiplexing to write to multiple destination tables with shared connections.
Multiplexing pools connections for better throughput and utilization of
resources. If your workflow has over 20 concurrent connections, we recommend
that you use multiplexing. Multiplexing is available in Java
and Go. For Java implementation details, see
[Use multiplexing](https://docs.cloud.google.com/bigquery/docs/write-api-streaming#use_multiplexing). For Go
implementation details, see
[Connection Sharing (Multiplexing)](https://pkg.go.dev/cloud.google.com/go/bigquery/storage/managedwriter#hdr-Connection_Sharing__Multiplexing_). If you use the [Beam connector with at-least-once semantics](https://beam.apache.org/documentation/io/built-in/google-bigquery/#at-least-once-semantics),
you can enable multiplexing through
[UseStorageApiConnectionPool](https://beam.apache.org/releases/javadoc/current/org/apache/beam/sdk/io/gcp/bigquery/BigQueryOptions.html#setUseStorageApiConnectionPool-java.lang.Boolean-). Managed Service for Apache Spark
connector has Multiplexing enabled by default.

For best performance, use one connection for as many data writes as possible.
Don't use one connection for just a single write, or open and close streams for
many small writes.

There is a quota on the number of
[concurrent connections](https://docs.cloud.google.com/bigquery/quotas#concurrent_connections) that can be
open at the same time per project. Above the limit, calls to `AppendRows` fail.
However, the quota for concurrent connections can be increased and should not
normally be a limiting factor for scaling.

Each call to `AppendRows` creates a new data writer object. So,
when using an application-created stream, the number of connections corresponds
to the number of streams that have been created. Generally, a single connection
supports at least 1MBps of throughput. The upper bound depends on several
factors, such as network bandwidth, the schema of the data, and server load, but
can exceed 10MBps.

There is also a quota on the
[total throughput per project](https://docs.cloud.google.com/bigquery/quotas#writeapi_throughput). This
represents the bytes per second across all connections flowing through the
Storage Write API service. If your project exceeds this quota, you
can [request a quota adjustment](https://docs.cloud.google.com/docs/quotas/help/request_increase).
Typically this involves raising accompanying quotas, like the concurrent
connections quota, in an equal ratio.

## Manage stream offsets to achieve exactly-once semantics

The Storage Write API only allows writes to the current end of the
stream, which moves as data is appended. The current position in the stream is
specified as an offset from the start of the stream.

When you write to an application-created stream, you can specify the stream
offset to achieve exactly-once write semantics.

When you specify an offset, the write operation is idempotent, which makes it
safe to retry due to network errors or unresponsiveness from the server.
Handle the following errors related to offsets:

- `ALREADY_EXISTS` (`StorageErrorCode.OFFSET_ALREADY_EXISTS`): The row was already written. You can safely ignore this error.
- `OUT_OF_RANGE` (`StorageErrorCode.OFFSET_OUT_OF_RANGE`): A previous write operation failed. Retry from the last successful write.

Note that these errors can also happen if you set the wrong offset value, so you
have to manage offsets carefully.

Before using stream offsets, consider whether you need exactly-once semantics.
For example, if your upstream data pipeline only guarantees at-least-once
writes, or if you can easily detect duplicates after data ingestion, then you
might not require exactly-once writes. In that case, we recommend using the
default stream, which does not require keeping track of row offsets.

## Do not block on `AppendRows` calls

The `AppendRows` method is asynchronous. You can send a series of writes without
blocking on a response for each write individually. The response messages on the
bidirectional connection arrive in the same order as the requests were enqueued.
For the highest throughput, call `AppendRows` without blocking to wait on the
response.

## Handle schema updates

For data streaming scenarios, table schemas are usually managed outside of the
streaming pipeline. It's common for the schema to evolve over time, for example
by adding new nullable fields. A robust pipeline must handle out-of-band schema
updates.

The Storage Write API supports table schemas as follows:

- The first write request includes the schema.
- You send each row of data as a binary protocol buffer. BigQuery maps the data to the schema.
- You can omit nullable fields, but you cannot include any fields that are not present in the current schema. If you send rows with extra fields, the Storage Write API returns a [`StorageError`](https://docs.cloud.google.com/bigquery/docs/reference/storage/rpc/google.cloud.bigquery.storage.v1#google.cloud.bigquery.storage.v1.StorageError) with `StorageErrorCode.SCHEMA_MISMATCH_EXTRA_FIELD`.

If you want to send new fields in the payload, you should first update the table
schema in BigQuery. The Storage Write API detects
schema changes after a short time, on the order of minutes. When the
Storage Write API detects the schema change, the
[`AppendRowsResponse`](https://docs.cloud.google.com/bigquery/docs/reference/storage/rpc/google.cloud.bigquery.storage.v1#google.cloud.bigquery.storage.v1.AppendRowsResponse) response message contains a
`TableSchema` object that describes the new schema.

To send data using the updated schema, you must close existing connections and
open new connections with the new schema.

**Java client** . The Java client library provides some additional features for
schema updates, through the [`JsonStreamWriter`](https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerystorage/latest/com.google.cloud.bigquery.storage.v1.JsonStreamWriter) class. After
a schema update, the `JsonStreamWriter` automatically reconnects with the
updated schema. You don't need to explicitly close and reopen the connection.
To check for schema changes programmatically, call
[`AppendRowsResponse.hasUpdatedSchema`](https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerystorage/latest/com.google.cloud.bigquery.storage.v1.AppendRowsResponse#com_google_cloud_bigquery_storage_v1_AppendRowsResponse_getUpdatedSchema__) after the `append`
method completes.

> [!NOTE]
> **Note:** Schema updates aren't immediately visible to the client library, but are detected on the order of minutes.

You can also configure the `JsonStreamWriter` to ignore unknown fields in the
input data. To set this behavior, call
[`setIgnoreUnknownFields`](https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerystorage/latest/com.google.cloud.bigquery.storage.v1.JsonStreamWriter.Builder#com_google_cloud_bigquery_storage_v1_JsonStreamWriter_Builder_setIgnoreUnknownFields_boolean_). This behavior is similar to
the `ignoreUnknownValues` option when using the legacy
[`tabledata.insertAll`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tabledata/insertAll)
API. However, it can lead to unintentional data loss, because unknown fields are
silently dropped.