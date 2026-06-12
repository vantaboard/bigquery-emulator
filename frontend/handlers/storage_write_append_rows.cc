#include <cstdint>
#include <string>
#include <vector>

#include "absl/status/status.h"
#include "absl/strings/str_cat.h"
#include "absl/synchronization/mutex.h"
#include "absl/types/span.h"
#include "backend/schema/schema.h"
#include "backend/storage/storage.h"
#include "frontend/handlers/storage_write.h"
#include "frontend/handlers/storage_write_internal.h"
#include "proto/storage_write.pb.h"

namespace bigquery_emulator {
namespace frontend {

using internal::AbslToGrpcStatus;
using internal::CellToValue;

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

  // The first message MUST set `write_stream`; subsequent messages
  // may leave it empty (the binding sticks for the rest of the
  // stream) or re-assert the same value. The handler keeps the
  // bound name + table id around so the per-batch lookup does not
  // re-parse the path on every read.
  std::string bound_stream_name;
  backend::storage::TableId bound_table;

  v1::AppendRowsRequest req;
  while (stream->Read(&req)) {
    v1::AppendRowsResponse resp;
    if (!req.trace_id().empty()) {
      resp.set_trace_id(req.trace_id());
    }

    // Resolve / re-resolve the stream binding.
    if (!req.write_stream().empty()) {
      backend::storage::TableId table;
      std::string stream_id;
      if (auto s = ParseStreamName(req.write_stream(), &table, &stream_id);
          !s.ok()) {
        return s;
      }
      if (!bound_stream_name.empty() &&
          bound_stream_name != req.write_stream()) {
        return ::grpc::Status(
            ::grpc::StatusCode::INVALID_ARGUMENT,
            absl::StrCat("StorageWrite.AppendRows: cannot rebind stream "
                         "mid-call (first message: ",
                         bound_stream_name,
                         ", later message: ",
                         req.write_stream(),
                         ")"));
      }
      // For the reserved `_default` name we mint the StreamState
      // lazily so callers do not need a CreateWriteStream round-trip
      // before the very first AppendRows.
      if (stream_id == "_default") {
        if (auto s = EnsureDefaultStream(req.write_stream(), table); !s.ok()) {
          return s;
        }
      }
      bound_stream_name = req.write_stream();
      bound_table = table;
    } else if (bound_stream_name.empty()) {
      return ::grpc::Status(
          ::grpc::StatusCode::INVALID_ARGUMENT,
          "StorageWrite.AppendRows: first message must set write_stream");
    }

    // Lookup the SessionState under the lock and snapshot the
    // schema + stream type; release before calling AppendRows so the
    // storage backend does not contend with subsequent
    // CreateWriteStream requests while it writes the parquet file.
    backend::storage::TableId table;
    backend::schema::TableSchema schema;
    v1::WriteStream::Type stream_type;
    bool stream_finalized = false;
    {
      absl::MutexLock lock(&mu_);
      auto it = streams_.find(bound_stream_name);
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
      table = it->second.table;
      schema = it->second.schema;
      stream_type = it->second.type;
      stream_finalized = it->second.finalized;
    }
    (void)stream_finalized;

    // Decode the rows. The wire shape is identical to
    // `Catalog.InsertRows` so we lower one cell at a time through
    // the shared `CellToValue` helper. A row whose cell count
    // does not match the schema's top-level column count is a
    // recoverable error — surface it on the response envelope so
    // the producer can fix the shape and retry without tearing the
    // stream down.
    const auto& rows_in = req.proto_rows().rows();
    bool shape_error = false;
    std::string shape_error_detail;
    std::vector<backend::storage::Row> rows;
    rows.reserve(rows_in.size());
    for (int r = 0; r < rows_in.size(); ++r) {
      const auto& src = rows_in[r];
      if (src.cells_size() != static_cast<int>(schema.columns.size())) {
        shape_error = true;
        shape_error_detail =
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
      rows.push_back(std::move(row));
    }

    if (shape_error) {
      resp.set_error_message(shape_error_detail);
      if (!stream->Write(resp)) {
        return ::grpc::Status(::grpc::StatusCode::ABORTED,
                              "StorageWrite.AppendRows: client cancelled "
                              "mid-stream");
      }
      continue;
    }

    std::int64_t prior_offset = 0;
    if (stream_type == v1::WriteStream::BUFFERED ||
        stream_type == v1::WriteStream::PENDING) {
      // BUFFERED / PENDING streams hold rows server-side until
      // FlushRows / BatchCommitWriteStreams makes them visible.
      absl::MutexLock lock(&mu_);
      auto it = streams_.find(bound_stream_name);
      if (it == streams_.end()) {
        return ::grpc::Status(::grpc::StatusCode::NOT_FOUND,
                              absl::StrCat("StorageWrite.AppendRows: no such "
                                           "stream: ",
                                           bound_stream_name));
      }
      prior_offset = it->second.committed_rows;
      it->second.committed_rows += static_cast<std::int64_t>(rows.size());
      for (auto& row : rows) {
        it->second.buffered_rows.push_back(std::move(row));
      }
    } else {
      // Forward to the storage append primitive
      // `DuckDBStorage::AppendRows`. The DuckDB backend writes a
      // fresh parquet snapshot per call, committing the rows
      // immediately — that is the documented `_default` /
      // `COMMITTED` semantic.
      const absl::Status append_status =
          rows.empty() ? absl::OkStatus()
                       : storage_->AppendRows(table, absl::MakeConstSpan(rows));
      if (!append_status.ok()) {
        // Recoverable storage errors land on the response envelope so
        // the producer can retry without tearing the stream down.
        // Hard `INTERNAL` failures (the storage layer's "anything
        // else" bucket) close the stream so the producer's caller
        // sees the matching gRPC status.
        if (append_status.code() == absl::StatusCode::kInvalidArgument ||
            append_status.code() == absl::StatusCode::kFailedPrecondition) {
          resp.set_error_message(std::string(append_status.message()));
          if (!stream->Write(resp)) {
            return ::grpc::Status(::grpc::StatusCode::ABORTED,
                                  "StorageWrite.AppendRows: client cancelled "
                                  "mid-stream");
          }
          continue;
        }
        return AbslToGrpcStatus(append_status);
      }

      // Successful append: bump the per-stream offset and reply.
      absl::MutexLock lock(&mu_);
      auto it = streams_.find(bound_stream_name);
      if (it != streams_.end()) {
        prior_offset = it->second.committed_rows;
        it->second.committed_rows += static_cast<std::int64_t>(rows.size());
      }
    }
    auto* result = resp.mutable_append_result();
    result->set_offset(prior_offset);
    resp.set_row_count(static_cast<std::int64_t>(rows.size()));
    if (!stream->Write(resp)) {
      return ::grpc::Status(
          ::grpc::StatusCode::ABORTED,
          "StorageWrite.AppendRows: client cancelled mid-stream");
    }
  }
  return ::grpc::Status::OK;
}

}  // namespace frontend
}  // namespace bigquery_emulator
