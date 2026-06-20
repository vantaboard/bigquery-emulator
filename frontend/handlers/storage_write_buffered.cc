#include <cstdint>
#include <string>
#include <vector>

#include "absl/status/status.h"
#include "absl/strings/str_cat.h"
#include "absl/synchronization/mutex.h"
#include "absl/types/span.h"
#include "frontend/handlers/storage_write.h"
#include "frontend/handlers/storage_write_internal.h"
#include "proto/storage_write.pb.h"

namespace bigquery_emulator {
namespace frontend {

using internal::AbslToGrpcStatus;

::grpc::Status StorageWriteService::CommitBufferedPrefix(StreamState* state,
                                                         std::int64_t offset) {
  if (state == nullptr) {
    return ::grpc::Status(::grpc::StatusCode::INTERNAL,
                          "StorageWrite.FlushRows: stream state is null");
  }
  if (state->type != v1::WriteStream::TYPE_BUFFERED) {
    return ::grpc::Status(
        ::grpc::StatusCode::FAILED_PRECONDITION,
        "StorageWrite.FlushRows: FlushRows is only valid for BUFFERED streams");
  }
  if (state->finalized) {
    return ::grpc::Status(::grpc::StatusCode::FAILED_PRECONDITION,
                          "StorageWrite.FlushRows: stream is finalized");
  }
  const std::int64_t buffered_count =
      static_cast<std::int64_t>(state->buffered_rows.size());
  if (offset < 0 || offset >= buffered_count) {
    return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT,
                          absl::StrCat("StorageWrite.FlushRows: offset ",
                                       offset,
                                       " is out of range for ",
                                       buffered_count,
                                       " buffered row(s)"));
  }
  if (offset < state->flushed_rows - 1) {
    return ::grpc::Status(
        ::grpc::StatusCode::INVALID_ARGUMENT,
        absl::StrCat("StorageWrite.FlushRows: offset ",
                     offset,
                     " must not precede the prior flush cursor ",
                     state->flushed_rows));
  }
  const std::int64_t start = state->flushed_rows;
  const std::int64_t end = offset + 1;
  if (start >= end) {
    return ::grpc::Status::OK;
  }
  std::vector<backend::storage::Row> batch;
  batch.reserve(static_cast<std::size_t>(end - start));
  for (std::int64_t i = start; i < end; ++i) {
    batch.push_back(state->buffered_rows[static_cast<std::size_t>(i)]);
  }
  const absl::Status append_status =
      storage_->AppendRows(state->table, absl::MakeConstSpan(batch));
  if (!append_status.ok()) {
    return AbslToGrpcStatus(append_status);
  }
  state->flushed_rows = end;
  return ::grpc::Status::OK;
}

::grpc::Status StorageWriteService::FinalizeWriteStream(
    ::grpc::ServerContext* /*context*/,
    const v1::FinalizeWriteStreamRequest* request,
    v1::FinalizeWriteStreamResponse* response) {
  if (request == nullptr || response == nullptr) {
    return ::grpc::Status(::grpc::StatusCode::INTERNAL,
                          "StorageWrite.FinalizeWriteStream: request and "
                          "response must be non-null");
  }
  if (request->name().empty()) {
    return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT,
                          "StorageWrite.FinalizeWriteStream: name is required");
  }
  backend::storage::TableId table;
  std::string stream_id;
  if (auto s = ParseStreamName(request->name(), &table, &stream_id); !s.ok()) {
    return s;
  }
  absl::MutexLock lock(&mu_);
  auto it = streams_.find(request->name());
  if (it == streams_.end()) {
    return ::grpc::Status(
        ::grpc::StatusCode::NOT_FOUND,
        absl::StrCat("StorageWrite.FinalizeWriteStream: no such stream: ",
                     request->name()));
  }
  if (it->second.finalized) {
    return ::grpc::Status(::grpc::StatusCode::FAILED_PRECONDITION,
                          absl::StrCat("StorageWrite.FinalizeWriteStream: "
                                       "stream already finalized: ",
                                       request->name()));
  }
  it->second.finalized = true;
  if (it->second.type == v1::WriteStream::TYPE_BUFFERED) {
    response->set_row_count(it->second.flushed_rows);
  } else if (it->second.type == v1::WriteStream::TYPE_PENDING) {
    response->set_row_count(
        static_cast<std::int64_t>(it->second.buffered_rows.size()));
  } else {
    response->set_row_count(it->second.committed_rows);
  }
  return ::grpc::Status::OK;
}

::grpc::Status StorageWriteService::FlushRows(
    ::grpc::ServerContext* /*context*/,
    const v1::FlushRowsRequest* request,
    v1::FlushRowsResponse* response) {
  if (storage_ == nullptr) {
    return ::grpc::Status(::grpc::StatusCode::INTERNAL,
                          "StorageWrite.FlushRows: storage backend is not "
                          "configured");
  }
  if (request == nullptr || response == nullptr) {
    return ::grpc::Status(::grpc::StatusCode::INTERNAL,
                          "StorageWrite.FlushRows: request and response must "
                          "be non-null");
  }
  if (request->write_stream().empty()) {
    return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT,
                          "StorageWrite.FlushRows: write_stream is required");
  }
  backend::storage::TableId table;
  std::string stream_id;
  if (auto s = ParseStreamName(request->write_stream(), &table, &stream_id);
      !s.ok()) {
    return s;
  }
  const std::int64_t flush_offset = request->offset();
  {
    absl::MutexLock lock(&mu_);
    auto it = streams_.find(request->write_stream());
    if (it == streams_.end()) {
      return ::grpc::Status(
          ::grpc::StatusCode::NOT_FOUND,
          absl::StrCat("StorageWrite.FlushRows: no such stream: ",
                       request->write_stream()));
    }
    if (auto commit = CommitBufferedPrefix(&it->second, flush_offset);
        !commit.ok()) {
      return commit;
    }
    response->set_offset(flush_offset);
  }
  return ::grpc::Status::OK;
}

}  // namespace frontend
}  // namespace bigquery_emulator
