#ifndef BIGQUERY_EMULATOR_FRONTEND_HANDLERS_STORAGE_READ_H_
#define BIGQUERY_EMULATOR_FRONTEND_HANDLERS_STORAGE_READ_H_

#include <grpcpp/grpcpp.h>

#include <cstddef>
#include <cstdint>
#include <map>
#include <memory>
#include <string>

#include "absl/synchronization/mutex.h"
#include "backend/schema/schema.h"
#include "backend/storage/storage.h"
#include "proto/storage_read.grpc.pb.h"
#include "proto/storage_read.pb.h"

namespace bigquery_emulator {
namespace frontend {

// StorageReadService is the C++ engine's implementation of the
// `bigquery_emulator.v1.StorageRead` gRPC service (plan 37). Plan 37
// lights up `CreateReadSession` (validate the table, mint a session +
// stream id, attach the schema); plan 38 wires the streaming
// `ReadRows` reply.
//
// Session lifecycle is in-process: each session is a `SessionState`
// struct kept in `sessions_` keyed by the server-assigned session id.
// The struct holds the `TableId` the session is reading from and the
// schema we returned at create time (so a follow-up `ReadRows` can
// re-verify the schema has not drifted under the session). Sessions
// never expire today; phase 7c will revisit when long-lived readers
// land.
//
// `Storage` errors map to gRPC status codes via the same convention
// `CatalogService` uses (see catalog.cc :: AbslToGrpcStatus):
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
class StorageReadService final : public v1::StorageRead::Service {
 public:
  explicit StorageReadService(backend::storage::Storage* storage);

  ::grpc::Status CreateReadSession(
      ::grpc::ServerContext* context,
      const v1::CreateReadSessionRequest* request,
      v1::ReadSession* response) override;

  // Streams rows off the stream id minted by `CreateReadSession`.
  // The handler:
  //   1. Strips the trailing `/streams/{id}` segment from the
  //      `read_stream` request field to recover the session name.
  //      Plan 37 mints exactly `streams/0`, so anything else is
  //      INVALID_ARGUMENT.
  //   2. Looks the session up in `sessions_`. Missing sessions
  //      surface as NOT_FOUND; the gateway maps that onto the
  //      BigQuery REST 404 envelope.
  //   3. Re-fetches the table schema and compares it cell-by-cell
  //      against the snapshot we recorded at session-create time.
  //      Drift (column added / dropped / type changed) is reported
  //      as FAILED_PRECONDITION because the caller is decoding rows
  //      with a stale schema.
  //   4. Opens a `Storage::CreateReadStream` against the session's
  //      table with the request's `offset` plumbed through (plan 38
  //      does not yet honor a request-side row_limit — the proto
  //      does not surface one and the gateway is the only caller).
  //   5. Streams batches of up to `kReadRowsBatchSize` rows per
  //      `ReadRowsResponse`, one writer->Write per page. The final
  //      page may carry fewer rows; an empty table emits zero pages.
  //
  // The handler obeys the gRPC server-streaming contract: it stops
  // sending after `writer->Write` returns false (the client cancelled
  // or the channel broke) and surfaces ABORTED in that case so the
  // caller sees the truncation.
  ::grpc::Status ReadRows(
      ::grpc::ServerContext* context,
      const v1::ReadRowsRequest* request,
      ::grpc::ServerWriter<v1::ReadRowsResponse>* writer) override;

  // Page size for ReadRows. Picked to keep each
  // `ReadRowsResponse` well under the 4 MiB default gRPC message
  // cap when rows are wide (e.g. 32-column tables with KB-sized
  // strings) while still amortizing the per-message overhead. The
  // value is exposed publicly so the unit tests can assert that a
  // multi-page response actually paginates instead of bundling the
  // whole table into one message.
  static constexpr int kReadRowsBatchSize = 100;

  // SessionsForTesting exposes the live session count so unit tests
  // can pin the mint-on-create / look-up-on-read contract without
  // grepping logs. Returns 0 when no sessions have been minted.
  std::size_t SessionsForTesting() const;

 private:
  struct SessionState {
    // BigQuery resource id of the table the session is pinned to.
    // Plan 38 reads rows off `storage_->ListRows(table, ...)` and
    // compares the schema we recorded at create time against the
    // live `Storage::GetSchema` reply to catch drift.
    backend::storage::TableId table;
    // Schema we returned in `ReadSession.schema` so plan 38 can
    // confirm the live schema still matches before streaming.
    backend::schema::TableSchema schema;
  };

  // ParseParent enforces the
  // `projects/{project_id}` shape on the `parent` field and returns
  // the bare project id. Empty / malformed parents map to gRPC
  // INVALID_ARGUMENT so the gateway can surface BigQuery's standard
  // 400 error envelope.
  ::grpc::Status ParseParent(const std::string& parent,
                              std::string* project_id) const;

  // ParseTablePath enforces the
  // `projects/{project_id}/datasets/{dataset_id}/tables/{table_id}`
  // shape on `read_session.table` and writes the parsed pieces into
  // `*out`. Plan 37 only cares about project/dataset/table; the
  // `location` slot the public API exposes is not yet honored.
  ::grpc::Status ParseTablePath(const std::string& table_path,
                                 backend::storage::TableId* out) const;

  // NewSessionId mints a unique session id of the form
  // `projects/{project_id}/locations/-/sessions/s{N}` where N comes
  // from a monotonic counter. The synthetic `-` location placeholder
  // matches what BigQuery does when the caller does not pin a
  // location on the read request.
  //
  // The counter bump is mutating shared state, so the caller MUST
  // already hold `mu_` exclusively. The annotation lets clang's
  // -Wthread-safety verify this at the call site.
  std::string NewSessionId(const std::string& project_id)
      ABSL_EXCLUSIVE_LOCKS_REQUIRED(mu_);

  // StreamIdForSession builds the canonical stream id of the form
  // `{session_name}/streams/0`. Plan 37 only mints stream 0 because
  // we always return exactly one stream per session.
  std::string StreamIdForSession(const std::string& session_name) const;

  backend::storage::Storage* storage_;  // not owned
  mutable absl::Mutex mu_;
  std::int64_t next_session_id_ ABSL_GUARDED_BY(mu_) = 1;
  std::map<std::string, SessionState> sessions_ ABSL_GUARDED_BY(mu_);
};

}  // namespace frontend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_FRONTEND_HANDLERS_STORAGE_READ_H_
