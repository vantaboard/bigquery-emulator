#include "frontend/handlers/storage_write.h"

#include <grpcpp/server_context.h>
#include <grpcpp/support/status.h>

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <string>

#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "absl/synchronization/mutex.h"
#include "backend/schema/schema.h"
#include "backend/storage/storage.h"
#include "frontend/handlers/handler_common.h"
#include "frontend/handlers/storage_write_internal.h"
#include "proto/storage_write.pb.h"

namespace bigquery_emulator {
namespace frontend {

using internal::AbslToGrpcStatus;
using internal::CellToValue;
using internal::SplitTablePath;
using internal::TablePathFor;

StorageWriteService::StorageWriteService(backend::storage::Storage* storage)
    : storage_(storage) {}

::grpc::Status StorageWriteService::ParseTableParent(
    const std::string& parent, backend::storage::TableId* out) const {
  if (parent.empty()) {
    return ::grpc::Status(
        ::grpc::StatusCode::INVALID_ARGUMENT,
        "StorageWrite.CreateWriteStream: parent is required (expected "
        "form projects/{p}/datasets/{d}/tables/{t})");
  }
  if (!SplitTablePath(parent, out)) {
    return ::grpc::Status(
        ::grpc::StatusCode::INVALID_ARGUMENT,
        absl::StrCat("StorageWrite.CreateWriteStream: malformed parent "
                     "(expected form "
                     "projects/{p}/datasets/{d}/tables/{t}): ",
                     parent));
  }
  return ::grpc::Status::OK;
}

::grpc::Status StorageWriteService::ParseStreamName(
    const std::string& name,
    backend::storage::TableId* table,
    std::string* stream_id) const {
  if (name.empty()) {
    return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT,
                          "StorageWrite: write_stream is required");
  }
  // Streams nest under their owning table: split on the first
  // `/streams/` and parse the prefix as a table path.
  const std::string kSep = "/streams/";
  const auto sep_pos = name.find(kSep);
  if (sep_pos == std::string::npos) {
    return ::grpc::Status(
        ::grpc::StatusCode::INVALID_ARGUMENT,
        absl::StrCat("StorageWrite: malformed write_stream name "
                     "(expected form "
                     "projects/{p}/datasets/{d}/tables/{t}/streams/{id}): ",
                     name));
  }
  const absl::string_view table_prefix =
      absl::string_view(name).substr(0, sep_pos);
  if (!SplitTablePath(table_prefix, table)) {
    return ::grpc::Status(
        ::grpc::StatusCode::INVALID_ARGUMENT,
        absl::StrCat("StorageWrite: malformed table prefix on "
                     "write_stream name (expected "
                     "projects/{p}/datasets/{d}/tables/{t} before "
                     "/streams/): ",
                     name));
  }
  *stream_id = name.substr(sep_pos + kSep.size());
  if (stream_id->empty()) {
    return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT,
                          absl::StrCat("StorageWrite: empty stream id in "
                                       "write_stream name: ",
                                       name));
  }
  return ::grpc::Status::OK;
}

std::string StorageWriteService::NewStreamId(const std::string& table_path) {
  // Caller must hold mu_; the annotation above tells clang's
  // -Wthread-safety to verify this at the call site.
  const std::int64_t id = next_stream_id_++;
  return absl::StrCat(table_path, "/streams/s", id);
}

std::string StorageWriteService::Rfc3339Now() const {
  // RFC3339 in UTC: "YYYY-MM-DDTHH:MM:SSZ". The handler stamps every
  // mint, so we deliberately avoid sub-second precision (callers do
  // not need it and the format matches the public BigQuery surface).
  const auto now = std::chrono::system_clock::now();
  const std::time_t tt = std::chrono::system_clock::to_time_t(now);
  std::tm utc{};
#if defined(_WIN32)
  ::gmtime_s(&utc, &tt);
#else
  ::gmtime_r(&tt, &utc);
#endif
  std::ostringstream os;
  os << std::put_time(&utc, "%Y-%m-%dT%H:%M:%SZ");
  return os.str();
}

::grpc::Status StorageWriteService::CreateWriteStream(
    ::grpc::ServerContext* /*context*/,
    const v1::CreateWriteStreamRequest* request,
    v1::WriteStream* response) {
  if (storage_ == nullptr) {
    return ::grpc::Status(
        ::grpc::StatusCode::INTERNAL,
        "StorageWrite.CreateWriteStream: storage backend is not configured");
  }
  if (request == nullptr || response == nullptr) {
    return ::grpc::Status(::grpc::StatusCode::INTERNAL,
                          "StorageWrite.CreateWriteStream: request and "
                          "response must be non-null");
  }

  backend::storage::TableId table_id;
  if (auto s = ParseTableParent(request->parent(), &table_id); !s.ok()) {
    return s;
  }

  // `_default` (the reserved name for an implicit append-only
  // stream), explicit COMMITTED, and BUFFERED streams light up
  // here. PENDING returns UNIMPLEMENTED with a clear
  // message so a producer pinning that type fails fast rather
  // than silently routing through the COMMITTED path.
  v1::WriteStream::Type requested = v1::WriteStream::COMMITTED;
  if (request->has_write_stream()) {
    requested = request->write_stream().type();
    if (requested == v1::WriteStream::TYPE_UNSPECIFIED) {
      requested = v1::WriteStream::COMMITTED;
    }
  }
  if (requested != v1::WriteStream::COMMITTED &&
      requested != v1::WriteStream::BUFFERED &&
      requested != v1::WriteStream::PENDING) {
    return ::grpc::Status(
        ::grpc::StatusCode::INVALID_ARGUMENT,
        absl::StrCat("StorageWrite.CreateWriteStream: unknown stream type ",
                     static_cast<int>(requested)));
  }

  // Validate the table exists by fetching its schema. NOT_FOUND
  // surfaces as gRPC NOT_FOUND -> BigQuery REST 404 envelope.
  absl::StatusOr<backend::schema::TableSchema> schema_or =
      storage_->GetSchema(table_id);
  if (!schema_or.ok()) {
    return AbslToGrpcStatus(schema_or.status());
  }

  std::string stream_name;
  std::string create_time;
  {
    absl::MutexLock lock(&mu_);
    stream_name = NewStreamId(TablePathFor(table_id));
    create_time = Rfc3339Now();
    StreamState state;
    state.table = table_id;
    state.schema = *schema_or;
    state.type = requested;
    state.create_time = create_time;
    streams_.emplace(stream_name, std::move(state));
  }

  response->set_name(stream_name);
  response->set_type(requested);
  backend::schema::TableSchemaToProto(*schema_or, response->mutable_schema());
  response->set_create_time(create_time);
  return ::grpc::Status::OK;
}

::grpc::Status StorageWriteService::EnsureDefaultStream(
    const std::string& stream_name, const backend::storage::TableId& table) {
  absl::StatusOr<backend::schema::TableSchema> schema_or =
      storage_->GetSchema(table);
  if (!schema_or.ok()) {
    return AbslToGrpcStatus(schema_or.status());
  }
  absl::MutexLock lock(&mu_);
  if (streams_.find(stream_name) == streams_.end()) {
    StreamState state;
    state.table = table;
    state.schema = *schema_or;
    state.type = v1::WriteStream::COMMITTED;
    state.create_time = Rfc3339Now();
    streams_.emplace(stream_name, std::move(state));
  }
  return ::grpc::Status::OK;
}

::grpc::Status StorageWriteService::GetWriteStream(
    ::grpc::ServerContext* /*context*/,
    const v1::GetWriteStreamRequest* request,
    v1::WriteStream* response) {
  if (request == nullptr || response == nullptr) {
    return ::grpc::Status(
        ::grpc::StatusCode::INTERNAL,
        "StorageWrite.GetWriteStream: request and response must be non-null");
  }
  StreamState state;
  {
    absl::MutexLock lock(&mu_);
    auto it = streams_.find(request->name());
    if (it == streams_.end()) {
      return ::grpc::Status(
          ::grpc::StatusCode::NOT_FOUND,
          absl::StrCat("StorageWrite.GetWriteStream: no such stream: ",
                       request->name()));
    }
    state = it->second;
  }
  response->set_name(request->name());
  response->set_type(state.type);
  backend::schema::TableSchemaToProto(state.schema, response->mutable_schema());
  response->set_create_time(state.create_time);
  return ::grpc::Status::OK;
}

::grpc::Status StorageWriteService::BatchCommitWriteStreams(
    ::grpc::ServerContext* /*context*/,
    const v1::BatchCommitWriteStreamsRequest* request,
    v1::BatchCommitWriteStreamsResponse* response) {
  if (storage_ == nullptr) {
    return ::grpc::Status(
        ::grpc::StatusCode::INTERNAL,
        "StorageWrite.BatchCommitWriteStreams: storage backend is not "
        "configured");
  }
  if (request == nullptr || response == nullptr) {
    return ::grpc::Status(
        ::grpc::StatusCode::INTERNAL,
        "StorageWrite.BatchCommitWriteStreams: request and response must "
        "be non-null");
  }
  if (request->write_streams().empty()) {
    return ::grpc::Status(
        ::grpc::StatusCode::INVALID_ARGUMENT,
        "StorageWrite.BatchCommitWriteStreams: write_streams is required");
  }

  backend::storage::TableId parent_table;
  if (!request->parent().empty()) {
    if (auto s = ParseTableParent(request->parent(), &parent_table); !s.ok()) {
      return s;
    }
  }

  struct PendingCommit {
    backend::storage::TableId table;
    std::vector<backend::storage::Row> rows{};
  };
  std::vector<PendingCommit> commits;

  {
    absl::MutexLock lock(&mu_);
    for (const auto& stream_name : request->write_streams()) {
      auto it = streams_.find(stream_name);
      if (it == streams_.end()) {
        return ::grpc::Status(
            ::grpc::StatusCode::NOT_FOUND,
            absl::StrCat("StorageWrite.BatchCommitWriteStreams: no such "
                         "stream: ",
                         stream_name));
      }
      if (it->second.type != v1::WriteStream::PENDING) {
        return ::grpc::Status(
            ::grpc::StatusCode::INVALID_ARGUMENT,
            absl::StrCat("StorageWrite.BatchCommitWriteStreams: stream is not "
                         "PENDING: ",
                         stream_name));
      }
      if (!it->second.finalized) {
        return ::grpc::Status(
            ::grpc::StatusCode::FAILED_PRECONDITION,
            absl::StrCat("StorageWrite.BatchCommitWriteStreams: stream is not "
                         "finalized: ",
                         stream_name));
      }
      if (!request->parent().empty() &&
          (it->second.table.project_id != parent_table.project_id ||
           it->second.table.dataset_id != parent_table.dataset_id ||
           it->second.table.table_id != parent_table.table_id)) {
        return ::grpc::Status(
            ::grpc::StatusCode::INVALID_ARGUMENT,
            absl::StrCat("StorageWrite.BatchCommitWriteStreams: stream does "
                         "not belong to parent table: ",
                         stream_name));
      }
      PendingCommit commit;
      commit.table = it->second.table;
      commit.rows = it->second.buffered_rows;
      commits.push_back(std::move(commit));
    }
  }

  for (const auto& commit : commits) {
    if (commit.rows.empty()) continue;
    const absl::Status append_status =
        storage_->AppendRows(commit.table, absl::MakeConstSpan(commit.rows));
    if (!append_status.ok()) {
      return AbslToGrpcStatus(append_status);
    }
  }

  response->set_commit_time(Rfc3339Now());
  return ::grpc::Status::OK;
}

std::size_t StorageWriteService::StreamsForTesting() const {
  absl::MutexLock lock(&mu_);
  return streams_.size();
}

}  // namespace frontend
}  // namespace bigquery_emulator