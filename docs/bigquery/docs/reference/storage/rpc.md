# BigQuery Storage API

## Service: bigquerystorage.googleapis.com

The Service name `bigquerystorage.googleapis.com` is needed to create RPC client stubs.

## `https://docs.cloud.google.com/bigquery/docs/reference/storage/rpc/google.cloud.bigquery.storage.v1#google.cloud.bigquery.storage.v1.BigQueryRead`

| Methods ||
|---|---|
| `` `https://docs.cloud.google.com/bigquery/docs/reference/storage/rpc/google.cloud.bigquery.storage.v1#google.cloud.bigquery.storage.v1.BigQueryRead.CreateReadSession` `` | Creates a new read session. |
| `` `https://docs.cloud.google.com/bigquery/docs/reference/storage/rpc/google.cloud.bigquery.storage.v1#google.cloud.bigquery.storage.v1.BigQueryRead.ReadRows` `` | Reads rows from the stream in the format prescribed by the ReadSession. |
| `` `https://docs.cloud.google.com/bigquery/docs/reference/storage/rpc/google.cloud.bigquery.storage.v1#google.cloud.bigquery.storage.v1.BigQueryRead.SplitReadStream` `` | Splits a given `ReadStream` into two `ReadStream` objects. |

## `https://docs.cloud.google.com/bigquery/docs/reference/storage/rpc/google.cloud.bigquery.storage.v1#google.cloud.bigquery.storage.v1.BigQueryWrite`

| Methods ||
|---|---|
| `` `https://docs.cloud.google.com/bigquery/docs/reference/storage/rpc/google.cloud.bigquery.storage.v1#google.cloud.bigquery.storage.v1.BigQueryWrite.AppendRows` `` | Appends data to the given stream. |
| `` `https://docs.cloud.google.com/bigquery/docs/reference/storage/rpc/google.cloud.bigquery.storage.v1#google.cloud.bigquery.storage.v1.BigQueryWrite.BatchCommitWriteStreams` `` | Atomically commits a group of `PENDING` streams that belong to the same `parent` table. |
| `` `https://docs.cloud.google.com/bigquery/docs/reference/storage/rpc/google.cloud.bigquery.storage.v1#google.cloud.bigquery.storage.v1.BigQueryWrite.CreateWriteStream` `` | Creates a write stream to the given table. |
| `` `https://docs.cloud.google.com/bigquery/docs/reference/storage/rpc/google.cloud.bigquery.storage.v1#google.cloud.bigquery.storage.v1.BigQueryWrite.FinalizeWriteStream` `` | Finalize a write stream so that no new data can be appended to the stream. |
| `` `https://docs.cloud.google.com/bigquery/docs/reference/storage/rpc/google.cloud.bigquery.storage.v1#google.cloud.bigquery.storage.v1.BigQueryWrite.FlushRows` `` | Flushes rows to a BUFFERED stream. |
| `` `https://docs.cloud.google.com/bigquery/docs/reference/storage/rpc/google.cloud.bigquery.storage.v1#google.cloud.bigquery.storage.v1.BigQueryWrite.GetWriteStream` `` | Gets information about a write stream. |

## `https://docs.cloud.google.com/bigquery/docs/reference/storage/rpc/google.cloud.bigquery.storage.v1beta1#google.cloud.bigquery.storage.v1beta1.BigQueryStorage`

| Methods ||
|---|---|
| `` `https://docs.cloud.google.com/bigquery/docs/reference/storage/rpc/google.cloud.bigquery.storage.v1beta1#google.cloud.bigquery.storage.v1beta1.BigQueryStorage.BatchCreateReadSessionStreams` `` | Creates additional streams for a ReadSession. |
| `` `https://docs.cloud.google.com/bigquery/docs/reference/storage/rpc/google.cloud.bigquery.storage.v1beta1#google.cloud.bigquery.storage.v1beta1.BigQueryStorage.CreateReadSession` `` | Creates a new read session. |
| `` `https://docs.cloud.google.com/bigquery/docs/reference/storage/rpc/google.cloud.bigquery.storage.v1beta1#google.cloud.bigquery.storage.v1beta1.BigQueryStorage.FinalizeStream` `` | Causes a single stream in a ReadSession to gracefully stop. |
| `` `https://docs.cloud.google.com/bigquery/docs/reference/storage/rpc/google.cloud.bigquery.storage.v1beta1#google.cloud.bigquery.storage.v1beta1.BigQueryStorage.ReadRows` `` | Reads rows from the table in the format prescribed by the read session. |
| `` `https://docs.cloud.google.com/bigquery/docs/reference/storage/rpc/google.cloud.bigquery.storage.v1beta1#google.cloud.bigquery.storage.v1beta1.BigQueryStorage.SplitReadStream` `` | Splits a given read stream into two Streams. |

## `https://docs.cloud.google.com/bigquery/docs/reference/storage/rpc/google.cloud.bigquery.storage.v1beta2#google.cloud.bigquery.storage.v1beta2.BigQueryRead`

| Methods ||
|---|---|
| `` `https://docs.cloud.google.com/bigquery/docs/reference/storage/rpc/google.cloud.bigquery.storage.v1beta2#google.cloud.bigquery.storage.v1beta2.BigQueryRead.CreateReadSession` `` | Creates a new read session. |
| `` `https://docs.cloud.google.com/bigquery/docs/reference/storage/rpc/google.cloud.bigquery.storage.v1beta2#google.cloud.bigquery.storage.v1beta2.BigQueryRead.ReadRows` `` | Reads rows from the stream in the format prescribed by the ReadSession. |
| `` `https://docs.cloud.google.com/bigquery/docs/reference/storage/rpc/google.cloud.bigquery.storage.v1beta2#google.cloud.bigquery.storage.v1beta2.BigQueryRead.SplitReadStream` `` | Splits a given `ReadStream` into two `ReadStream` objects. |

## `https://docs.cloud.google.com/bigquery/docs/reference/storage/rpc/google.cloud.bigquery.storage.v1beta2#google.cloud.bigquery.storage.v1beta2.BigQueryWrite`

> [!WARNING]
> This item is deprecated!

| Methods ||
|---|---|
| `` `https://docs.cloud.google.com/bigquery/docs/reference/storage/rpc/google.cloud.bigquery.storage.v1beta2#google.cloud.bigquery.storage.v1beta2.BigQueryWrite.AppendRows` (deprecated) `` | Appends data to the given stream. |
| `` `https://docs.cloud.google.com/bigquery/docs/reference/storage/rpc/google.cloud.bigquery.storage.v1beta2#google.cloud.bigquery.storage.v1beta2.BigQueryWrite.BatchCommitWriteStreams` (deprecated) `` | Atomically commits a group of `PENDING` streams that belong to the same `parent` table. |
| `` `https://docs.cloud.google.com/bigquery/docs/reference/storage/rpc/google.cloud.bigquery.storage.v1beta2#google.cloud.bigquery.storage.v1beta2.BigQueryWrite.CreateWriteStream` (deprecated) `` | Creates a write stream to the given table. |
| `` `https://docs.cloud.google.com/bigquery/docs/reference/storage/rpc/google.cloud.bigquery.storage.v1beta2#google.cloud.bigquery.storage.v1beta2.BigQueryWrite.FinalizeWriteStream` (deprecated) `` | Finalize a write stream so that no new data can be appended to the stream. |
| `` `https://docs.cloud.google.com/bigquery/docs/reference/storage/rpc/google.cloud.bigquery.storage.v1beta2#google.cloud.bigquery.storage.v1beta2.BigQueryWrite.FlushRows` (deprecated) `` | Flushes rows to a BUFFERED stream. |
| `` `https://docs.cloud.google.com/bigquery/docs/reference/storage/rpc/google.cloud.bigquery.storage.v1beta2#google.cloud.bigquery.storage.v1beta2.BigQueryWrite.GetWriteStream` (deprecated) `` | Gets a write stream. |