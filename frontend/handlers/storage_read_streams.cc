#include <grpcpp/server_context.h>
#include <grpcpp/support/status.h>
#include <grpcpp/support/sync_stream.h>

#include <cmath>
#include <cstdint>
#include <optional>
#include <string>

#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "absl/synchronization/mutex.h"
#include "backend/schema/schema.h"
#include "backend/storage/storage.h"
#include "frontend/handlers/storage_read.h"
#include "frontend/handlers/storage_read_internal.h"
#include "proto/storage_read.pb.h"

namespace bigquery_emulator {
namespace frontend {

using internal::AbslToGrpcStatus;
using internal::SchemasEqualByShape;
using internal::ValueToCell;

::grpc::Status StorageReadService::ReadRows(
    ::grpc::ServerContext* /*context*/,
    const v1::ReadRowsRequest* request,
    ::grpc::ServerWriter<v1::ReadRowsResponse>* writer) {
  if (request == nullptr) {
    return ::grpc::Status(::grpc::StatusCode::INTERNAL,
                          "StorageRead.ReadRows: request must be non-null");
  }

  std::string session_name;
  std::string stream_key;
  if (auto s = ParseReadStreamName(
          request->read_stream(), &session_name, &stream_key);
      !s.ok()) {
    return s;
  }

  backend::storage::TableId table;
  backend::schema::TableSchema session_schema;
  std::optional<std::string> where_sql;
  std::vector<std::string> selected_fields;
  StreamPartition partition;
  {
    absl::MutexLock lock(&mu_);
    auto it = sessions_.find(session_name);
    if (it == sessions_.end()) {
      return ::grpc::Status(
          ::grpc::StatusCode::NOT_FOUND,
          absl::StrCat("StorageRead.ReadRows: no such session (call "
                       "CreateReadSession first): ",
                       session_name));
    }
    auto stream_it = it->second.streams.find(request->read_stream());
    if (stream_it == it->second.streams.end()) {
      return ::grpc::Status(
          ::grpc::StatusCode::NOT_FOUND,
          absl::StrCat("StorageRead.ReadRows: no such stream: ",
                       request->read_stream()));
    }
    table = it->second.table;
    session_schema = it->second.schema;
    where_sql = it->second.where_sql;
    selected_fields = it->second.selected_fields;
    partition = stream_it->second;
  }

  absl::StatusOr<backend::schema::TableSchema> live_schema_or =
      storage_->GetSchema(table);
  if (!live_schema_or.ok()) {
    return AbslToGrpcStatus(live_schema_or.status());
  }
  if (!SchemasEqualByShape(session_schema, *live_schema_or)) {
    return ::grpc::Status(
        ::grpc::StatusCode::FAILED_PRECONDITION,
        absl::StrCat("StorageRead.ReadRows: table schema for ",
                     table.project_id,
                     ".",
                     table.dataset_id,
                     ".",
                     table.table_id,
                     " changed since CreateReadSession; open a new session"));
  }

  if (writer == nullptr) {
    return ::grpc::Status(::grpc::StatusCode::INTERNAL,
                          "StorageRead.ReadRows: writer must be non-null");
  }

  backend::storage::ReadFilter filter;
  filter.offset = request->offset();
  filter.row_limit = 0;
  filter.where_sql = where_sql;
  filter.selected_fields = selected_fields;
  filter.row_start = partition.row_start;
  filter.row_end = partition.row_end;

  absl::StatusOr<std::unique_ptr<backend::storage::RowIterator>> iter_or =
      storage_->CreateReadStream(table, filter);
  if (!iter_or.ok()) {
    return AbslToGrpcStatus(iter_or.status());
  }
  std::unique_ptr<backend::storage::RowIterator> iter = std::move(*iter_or);

  v1::ReadRowsResponse page;
  int64_t in_page = 0;
  backend::storage::Row row;
  while (true) {
    auto has_or = iter->Next(&row);
    if (!has_or.ok()) {
      return AbslToGrpcStatus(has_or.status());
    }
    if (!*has_or) break;
    auto* proto_row = page.add_rows();
    for (const auto& cell : row.cells) {
      ValueToCell(cell, proto_row->add_cells());
    }
    ++in_page;
    if (in_page >= kReadRowsBatchSize) {
      page.set_row_count(in_page);
      if (!writer->Write(page)) {
        return ::grpc::Status(
            ::grpc::StatusCode::ABORTED,
            "StorageRead.ReadRows: client cancelled mid-stream");
      }
      page.Clear();
      in_page = 0;
    }
  }
  if (in_page > 0) {
    page.set_row_count(in_page);
    if (!writer->Write(page)) {
      return ::grpc::Status(
          ::grpc::StatusCode::ABORTED,
          "StorageRead.ReadRows: client cancelled mid-stream");
    }
  }
  return ::grpc::Status::OK;
}

::grpc::Status StorageReadService::SplitReadStream(
    ::grpc::ServerContext* /*context*/,
    const v1::SplitReadStreamRequest* request,
    v1::SplitReadStreamResponse* response) {
  if (request == nullptr || response == nullptr) {
    return ::grpc::Status(::grpc::StatusCode::INTERNAL,
                          "StorageRead.SplitReadStream: request and response "
                          "must be non-null");
  }
  if (request->name().empty()) {
    return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT,
                          "StorageRead.SplitReadStream: name is required");
  }
  const double fraction = request->fraction();
  if (!(fraction > 0.0 && fraction < 1.0)) {
    return ::grpc::Status(
        ::grpc::StatusCode::INVALID_ARGUMENT,
        absl::StrCat("StorageRead.SplitReadStream: fraction must be in (0.0, "
                     "1.0); got ",
                     fraction));
  }

  std::string session_name;
  std::string stream_key;
  if (auto s = ParseReadStreamName(request->name(), &session_name, &stream_key);
      !s.ok()) {
    return s;
  }

  absl::MutexLock lock(&mu_);
  auto session_it = sessions_.find(session_name);
  if (session_it == sessions_.end()) {
    return ::grpc::Status(
        ::grpc::StatusCode::NOT_FOUND,
        absl::StrCat("StorageRead.SplitReadStream: no such session: ",
                     session_name));
  }
  auto stream_it = session_it->second.streams.find(request->name());
  if (stream_it == session_it->second.streams.end()) {
    return ::grpc::Status(
        ::grpc::StatusCode::NOT_FOUND,
        absl::StrCat("StorageRead.SplitReadStream: no such stream: ",
                     request->name()));
  }

  const StreamPartition& original = stream_it->second;
  const std::int64_t range_start = original.row_start;
  const std::int64_t range_end =
      original.row_end < 0 ? session_it->second.total_rows : original.row_end;
  const std::int64_t span = range_end - range_start;
  if (span <= 1) {
    return ::grpc::Status(
        ::grpc::StatusCode::FAILED_PRECONDITION,
        absl::StrCat("StorageRead.SplitReadStream: stream cannot be split "
                     "(remaining range has ",
                     span,
                     " row(s)): ",
                     request->name()));
  }

  const std::int64_t split_at =
      range_start + static_cast<std::int64_t>(
                        std::floor(static_cast<double>(span) * fraction));
  if (split_at <= range_start || split_at >= range_end) {
    return ::grpc::Status(
        ::grpc::StatusCode::FAILED_PRECONDITION,
        "StorageRead.SplitReadStream: split point leaves an empty child "
        "stream");
  }

  const std::int64_t split_id = session_it->second.next_split_id++;
  StreamPartition primary;
  primary.name = absl::StrCat(session_name, "/streams/split", split_id, "p");
  primary.row_start = range_start;
  primary.row_end = split_at;

  StreamPartition remainder;
  remainder.name = absl::StrCat(session_name, "/streams/split", split_id, "r");
  remainder.row_start = split_at;
  remainder.row_end = original.row_end;

  session_it->second.streams.emplace(primary.name, primary);
  session_it->second.streams.emplace(remainder.name, remainder);

  response->mutable_primary_stream()->set_name(primary.name);
  response->mutable_remainder_stream()->set_name(remainder.name);
  return ::grpc::Status::OK;
}

}  // namespace frontend
}  // namespace bigquery_emulator
