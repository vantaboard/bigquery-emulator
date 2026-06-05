#ifndef BIGQUERY_EMULATOR_FRONTEND_HANDLERS_STORAGE_WRITE_H_
#define BIGQUERY_EMULATOR_FRONTEND_HANDLERS_STORAGE_WRITE_H_

#include <grpcpp/grpcpp.h>

#include <cstddef>
#include <cstdint>
#include <map>
#include <memory>
#include <string>

#include "absl/synchronization/mutex.h"
#include "backend/schema/schema.h"
#include "backend/storage/storage.h"
#include "proto/storage_write.grpc.pb.h"
#include "proto/storage_write.pb.h"

namespace bigquery_emulator {
namespace frontend {

// StorageWriteService is the C++ engine's implementation of the
// `bigquery_emulator.v1.StorageWrite` gRPC service (plan 15:
// `storage-read-write-api-plan.plan.md`).
//
// Plan-15 scope: `_default` + `COMMITTED` stream types end-to-end.
// Both commit on every flushed AppendRows batch through the same
// `DuckDBStorage::AppendRows` primitive the local DML executor uses
// (plan 9, `local-exec-14-dml-system.plan.md`). The four other RPCs
// (`FinalizeWriteStream`, `BatchCommitWriteStreams`, `FlushRows`,
// plus `BUFFERED` / `PENDING` stream creation) reserve their proto
// slots but return UNIMPLEMENTED until the deferred follow-up subagent
// lights them up — silent approximation here would be especially bad
// because the public Storage Write API's transactional contract
// (PENDING streams committing through BatchCommit) is exactly what
// distinguishes it from `tabledata.insertAll`.
//
// Stream lifecycle is in-process:
//   * `CreateWriteStream` mints a `StreamState` keyed by stream name
//     (`{table}/streams/s{N}`) and stashes the source table id +
//     pinned schema.
//   * The reserved `_default` stream name (`{table}/streams/_default`)
//     is implicitly available without a `CreateWriteStream` call;
//     `AppendRows` mints the `StreamState` lazily on first use.
//   * `AppendRows` (bidi-streaming) reads requests off the wire, looks
//     the stream up, and forwards each batch to
//     `Storage::AppendRows`. Schema-shape mismatches surface on the
//     `AppendRowsResponse.error_message` envelope without tearing the
//     stream down — the producer can recover by sending a correctly-
//     shaped batch on the next request, which mirrors the public
//     surface's recoverable-error semantics.
//   * `GetWriteStream` echoes back the stashed metadata.
//
// Storage errors map to gRPC status codes via the same convention
// `CatalogService` and `StorageReadService` use:
//
//   absl::NotFound          -> grpc::NOT_FOUND
//   absl::AlreadyExists     -> grpc::ALREADY_EXISTS
//   absl::InvalidArgument   -> grpc::INVALID_ARGUMENT
//   absl::FailedPrecondition-> grpc::FAILED_PRECONDITION
//   anything else           -> grpc::INTERNAL
//
// The service does not own the storage pointer; the caller (typically
// `Server::Create`) keeps the `Storage` alive for the gRPC server's
// lifetime. `storage` must be non-null.
class StorageWriteService final : public v1::StorageWrite::Service {
 public:
  explicit StorageWriteService(backend::storage::Storage* storage);

  ::grpc::Status CreateWriteStream(::grpc::ServerContext* context,
                                   const v1::CreateWriteStreamRequest* request,
                                   v1::WriteStream* response) override;

  // AppendRows is the bidi-streaming append entry point. The handler
  // reads each `AppendRowsRequest` off `stream`, decodes the
  // `proto_rows.rows` into engine-side `Row` values, and forwards
  // them to `Storage::AppendRows`. Errors are reported on the
  // streamed `AppendRowsResponse` envelope (per the public Storage
  // Write API contract) so a single bad batch does not tear the
  // whole stream down. Hard, non-recoverable errors (unknown stream,
  // table dropped under the session) close the stream with the
  // matching gRPC status.
  ::grpc::Status AppendRows(
      ::grpc::ServerContext* context,
      ::grpc::ServerReaderWriter<v1::AppendRowsResponse, v1::AppendRowsRequest>*
          stream) override;

  ::grpc::Status GetWriteStream(::grpc::ServerContext* context,
                                const v1::GetWriteStreamRequest* request,
                                v1::WriteStream* response) override;

  // Plan 15 returns UNIMPLEMENTED for these; the proto + handler
  // hooks are in place so the deferred follow-up can light them up
  // without re-routing through the Server::Create plumbing.
  ::grpc::Status FinalizeWriteStream(
      ::grpc::ServerContext* context,
      const v1::FinalizeWriteStreamRequest* request,
      v1::FinalizeWriteStreamResponse* response) override;
  ::grpc::Status BatchCommitWriteStreams(
      ::grpc::ServerContext* context,
      const v1::BatchCommitWriteStreamsRequest* request,
      v1::BatchCommitWriteStreamsResponse* response) override;
  ::grpc::Status FlushRows(::grpc::ServerContext* context,
                           const v1::FlushRowsRequest* request,
                           v1::FlushRowsResponse* response) override;

  // StreamsForTesting exposes the live stream count so unit tests
  // can pin the create-on-explicit / mint-on-default contract
  // without grepping logs.
  std::size_t StreamsForTesting() const;

  // Reserved suffix for the table's implicit default write stream.
  // Mirrors the public BigQuery surface: clients that call
  // AppendRows against `{table}/streams/_default` without a prior
  // CreateWriteStream get the implicit append-only stream.
  static constexpr char kDefaultStreamSuffix[] = "/streams/_default";

 private:
  struct StreamState {
    backend::storage::TableId table;
    backend::schema::TableSchema schema;
    v1::WriteStream::Type type = v1::WriteStream::COMMITTED;
    std::string create_time;
    // Per-stream commit offset. Plan 15 increments this by
    // `rows.size()` on every successful append; the value rides on
    // `AppendRowsResponse.AppendResult.offset` so a producer that
    // pinned an offset on its first message can sanity-check the
    // engine's running count.
    std::int64_t committed_rows = 0;
  };

  // ParseTableParent enforces
  // `projects/{p}/datasets/{d}/tables/{t}` on the parent of a
  // CreateWriteStreamRequest and writes the parsed pieces into
  // `*out`. Empty / malformed parents map to gRPC INVALID_ARGUMENT.
  ::grpc::Status ParseTableParent(const std::string& parent,
                                  backend::storage::TableId* out) const;

  // ParseStreamName splits a write-stream name into the source
  // table id and the trailing `/streams/{id}` suffix. Both pieces
  // are returned verbatim so the caller can compare them against
  // a SessionState lookup.
  ::grpc::Status ParseStreamName(const std::string& name,
                                 backend::storage::TableId* table,
                                 std::string* stream_id) const;

  // NewStreamId mints a unique stream id of the form
  // `{table_path}/streams/s{N}` where N comes from a monotonic
  // counter. Caller MUST hold `mu_`.
  std::string NewStreamId(const std::string& table_path)
      ABSL_EXCLUSIVE_LOCKS_REQUIRED(mu_);

  // Format an RFC3339 timestamp at "now" so a fresh stream's
  // `create_time` field is wire-ready without pulling in a
  // separate time util.
  std::string Rfc3339Now() const;

  // EnsureDefaultStream is called from AppendRows when the caller
  // pins the `_default` stream id. If no SessionState exists yet
  // for that name, the handler mints one against the table's live
  // schema. Returns INVALID_ARGUMENT when the underlying table is
  // missing or the stream name is malformed.
  ::grpc::Status EnsureDefaultStream(const std::string& stream_name,
                                     const backend::storage::TableId& table)
      ABSL_LOCKS_EXCLUDED(mu_);

  backend::storage::Storage* storage_;  // not owned
  mutable absl::Mutex mu_;
  std::int64_t next_stream_id_ ABSL_GUARDED_BY(mu_) = 1;
  std::map<std::string, StreamState> streams_ ABSL_GUARDED_BY(mu_){};
};

}  // namespace frontend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_FRONTEND_HANDLERS_STORAGE_WRITE_H_
