#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "absl/status/status.h"
#include "absl/strings/str_cat.h"
#include "absl/synchronization/mutex.h"
#include "absl/types/span.h"
#include "backend/storage/storage.h"
#include "frontend/handlers/storage_write.h"
#include "frontend/handlers/storage_write_internal.h"
#include "proto/storage_write.pb.h"

namespace bigquery_emulator {
namespace frontend {

using internal::AbslToGrpcStatus;
using internal::CellToValue;

namespace {

struct DecodedAppendBatch {
  bool shape_error = false;
  std::string shape_error_detail;
  std::vector<backend::storage::Row> rows;
};

DecodedAppendBatch DecodeAppendRowBatch(
    const v1::AppendRowsRequest& req,
    const backend::schema::TableSchema& schema) {
  DecodedAppendBatch out;
  const auto& rows_in = req.proto_rows().rows();
  out.rows.reserve(rows_in.size());
  for (int r = 0; r < rows_in.size(); ++r) {
    const auto& src = rows_in[r];
    if (src.cells_size() != static_cast<int>(schema.columns.size())) {
      out.shape_error = true;
      out.shape_error_detail =
          absl::StrCat("AppendRows: row ",
                       r,
                       " has ",
                       src.cells_size(),
                       " cell(s) but the stream's table has ",
                       schema.columns.size(),
                       " top-level column(s)");
      break;
    }
    backend::storage::Row row;
    row.cells.reserve(src.cells_size());
    for (const auto& cell : src.cells()) {
      row.cells.push_back(CellToValue(cell));
    }
    out.rows.push_back(std::move(row));
  }
  return out;
}

}  // namespace

::grpc::Status StorageWriteService::BindWriteStreamFromRequest(
    const v1::AppendRowsRequest& req,
    std::string* bound_stream_name,
    backend::storage::TableId* bound_table) {
  if (req.write_stream().empty()) {
    if (bound_stream_name->empty()) {
      return ::grpc::Status(
          ::grpc::StatusCode::INVALID_ARGUMENT,
          "StorageWrite.AppendRows: first message must set write_stream");
    }
    return ::grpc::Status::OK;
  }
  backend::storage::TableId table;
  std::string stream_id;
  if (auto s = ParseStreamName(req.write_stream(), &table, &stream_id);
      !s.ok()) {
    return s;
  }
  if (!bound_stream_name->empty() && *bound_stream_name != req.write_stream()) {
    return ::grpc::Status(
        ::grpc::StatusCode::INVALID_ARGUMENT,
        absl::StrCat("StorageWrite.AppendRows: cannot rebind stream "
                     "mid-call (first message: ",
                     *bound_stream_name,
                     ", later message: ",
                     req.write_stream(),
                     ")"));
  }
  if (stream_id == "_default") {
    if (auto s = EnsureDefaultStream(req.write_stream(), table); !s.ok()) {
      return s;
    }
  }
  *bound_stream_name = req.write_stream();
  *bound_table = table;
  return ::grpc::Status::OK;
}

::grpc::Status StorageWriteService::WriteAppendResponse(
    ::grpc::ServerReaderWriter<v1::AppendRowsResponse, v1::AppendRowsRequest>*
        stream,
    v1::AppendRowsResponse resp) {
  if (!stream->Write(resp)) {
    return ::grpc::Status(::grpc::StatusCode::ABORTED,
                          "StorageWrite.AppendRows: client cancelled "
                          "mid-stream");
  }
  return ::grpc::Status::OK;
}

::grpc::Status StorageWriteService::LoadStreamSessionForAppend(
    absl::string_view bound_stream_name, StreamSessionSnapshot* session) {
  absl::MutexLock lock(&mu_);
  auto it = streams_.find(std::string(bound_stream_name));
  if (it == streams_.end()) {
    return ::grpc::Status(
        ::grpc::StatusCode::NOT_FOUND,
        absl::StrCat("StorageWrite.AppendRows: no such stream (call "
                     "CreateWriteStream first or use the reserved "
                     "_default suffix): ",
                     bound_stream_name));
  }
  if (it->second.finalized) {
    return ::grpc::Status(
        ::grpc::StatusCode::FAILED_PRECONDITION,
        absl::StrCat("StorageWrite.AppendRows: stream is finalized: ",
                     bound_stream_name));
  }
  session->table = it->second.table;
  session->schema = it->second.schema;
  session->type = it->second.type;
  return ::grpc::Status::OK;
}

::grpc::Status StorageWriteService::CommitBufferedAppendRows(
    absl::string_view bound_stream_name,
    std::vector<backend::storage::Row> rows,
    std::int64_t* prior_offset) {
  absl::MutexLock lock(&mu_);
  auto it = streams_.find(std::string(bound_stream_name));
  if (it == streams_.end()) {
    return ::grpc::Status(
        ::grpc::StatusCode::NOT_FOUND,
        absl::StrCat("StorageWrite.AppendRows: no such stream: ",
                     bound_stream_name));
  }
  *prior_offset = it->second.committed_rows;
  it->second.committed_rows += static_cast<std::int64_t>(rows.size());
  for (auto& row : rows) {
    it->second.buffered_rows.push_back(std::move(row));
  }
  return ::grpc::Status::OK;
}

::grpc::Status StorageWriteService::CommitImmediateAppendRows(
    absl::string_view bound_stream_name,
    const backend::storage::TableId& table,
    absl::Span<const backend::storage::Row> rows,
    std::int64_t* prior_offset,
    v1::AppendRowsResponse* resp) {
  const absl::Status append_status =
      rows.empty() ? absl::OkStatus() : storage_->AppendRows(table, rows);
  if (!append_status.ok()) {
    if (append_status.code() == absl::StatusCode::kInvalidArgument ||
        append_status.code() == absl::StatusCode::kFailedPrecondition) {
      resp->set_error_message(std::string(append_status.message()));
      return ::grpc::Status::OK;
    }
    return AbslToGrpcStatus(append_status);
  }
  absl::MutexLock lock(&mu_);
  auto it = streams_.find(std::string(bound_stream_name));
  if (it != streams_.end()) {
    *prior_offset = it->second.committed_rows;
    it->second.committed_rows += static_cast<std::int64_t>(rows.size());
  }
  return ::grpc::Status::OK;
}

::grpc::Status StorageWriteService::AppendRows(
    ::grpc::ServerContext* /*context*/,
    ::grpc::ServerReaderWriter<v1::AppendRowsResponse, v1::AppendRowsRequest>*
        stream) {
  if (storage_ == nullptr) {
    return ::grpc::Status(
        ::grpc::StatusCode::INTERNAL,
        "StorageWrite.AppendRows: storage backend is not configured");
  }
  if (stream == nullptr) {
    return ::grpc::Status(::grpc::StatusCode::INTERNAL,
                          "StorageWrite.AppendRows: stream must be non-null");
  }

  std::string bound_stream_name;
  backend::storage::TableId bound_table;

  v1::AppendRowsRequest req;
  while (stream->Read(&req)) {
    v1::AppendRowsResponse resp;
    if (!req.trace_id().empty()) {
      resp.set_trace_id(req.trace_id());
    }

    if (auto bind_status =
            BindWriteStreamFromRequest(req, &bound_stream_name, &bound_table);
        !bind_status.ok()) {
      return bind_status;
    }

    StreamSessionSnapshot session;
    if (auto session_status =
            LoadStreamSessionForAppend(bound_stream_name, &session);
        !session_status.ok()) {
      return session_status;
    }

    DecodedAppendBatch batch = DecodeAppendRowBatch(req, session.schema);
    if (batch.shape_error) {
      resp.set_error_message(batch.shape_error_detail);
      if (auto write_status = WriteAppendResponse(stream, std::move(resp));
          !write_status.ok()) {
        return write_status;
      }
      continue;
    }

    std::int64_t prior_offset = 0;
    if (session.type == v1::WriteStream::BUFFERED ||
        session.type == v1::WriteStream::PENDING) {
      if (auto commit_status = CommitBufferedAppendRows(
              bound_stream_name, std::move(batch.rows), &prior_offset);
          !commit_status.ok()) {
        return commit_status;
      }
    } else {
      if (auto commit_status =
              CommitImmediateAppendRows(bound_stream_name,
                                        session.table,
                                        absl::MakeConstSpan(batch.rows),
                                        &prior_offset,
                                        &resp);
          !commit_status.ok()) {
        return commit_status;
      }
      if (resp.has_error_message()) {
        if (auto write_status = WriteAppendResponse(stream, std::move(resp));
            !write_status.ok()) {
          return write_status;
        }
        continue;
      }
    }

    resp.mutable_append_result()->set_offset(prior_offset);
    resp.set_row_count(static_cast<std::int64_t>(batch.rows.size()));
    if (auto write_status = WriteAppendResponse(stream, resp);
        !write_status.ok()) {
      return write_status;
    }
  }
  return ::grpc::Status::OK;
}

}  // namespace frontend
}  // namespace bigquery_emulator
