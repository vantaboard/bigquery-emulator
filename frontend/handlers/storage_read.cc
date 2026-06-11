#include "frontend/handlers/storage_read.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "absl/status/statusor.h"
#include "absl/strings/match.h"
#include "absl/strings/numbers.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_split.h"
#include "absl/synchronization/mutex.h"
#include "backend/schema/schema.h"
#include "frontend/handlers/storage_row_restriction_analyze.h"
#include "backend/storage/storage.h"
#include "frontend/handlers/storage_read_internal.h"
#include "proto/storage_read.pb.h"

namespace bigquery_emulator {
namespace frontend {

using internal::AbslToGrpcStatus;
using internal::FindColumnByName;
using internal::kColumnNotFound;
using internal::kPublicDataProject;
using internal::ProjectSchemaForResponse;
using internal::SchemasEqualByShape;
using internal::ValueToCell;

namespace {

constexpr std::int32_t kMaxStreamCount = 1000;

std::int32_t NegotiateStreamCount(std::int32_t requested,
                                  std::int64_t total_rows) {
  if (requested <= 0) return 1;
  if (total_rows <= 0) return 1;
  const std::int64_t capped =
      std::min<std::int64_t>(requested, kMaxStreamCount);
  return static_cast<std::int32_t>(
      std::min<std::int64_t>(capped, total_rows));
}

struct LocalStreamPartition {
  std::string name;
  std::int64_t row_start = 0;
  std::int64_t row_end = -1;
};

std::vector<LocalStreamPartition> BuildPartitions(
    const std::string& session_name,
    std::int32_t stream_count,
    std::int64_t total_rows) {
  std::vector<LocalStreamPartition> out;
  out.reserve(static_cast<std::size_t>(stream_count));
  for (std::int32_t i = 0; i < stream_count; ++i) {
    const std::int64_t start = (total_rows * i) / stream_count;
    const std::int64_t end = (total_rows * (i + 1)) / stream_count;
    LocalStreamPartition part;
    part.name = absl::StrCat(session_name, "/streams/", i);
    part.row_start = start;
    part.row_end = end;
    out.push_back(std::move(part));
  }
  return out;
}

}  // namespace

StorageReadService::StorageReadService(backend::storage::Storage* storage)
    : storage_(storage) {}

::grpc::Status StorageReadService::ParseParent(const std::string& parent,
                                               std::string* project_id) const {
  if (parent.empty()) {
    return ::grpc::Status(
        ::grpc::StatusCode::INVALID_ARGUMENT,
        "StorageRead.CreateReadSession: parent is required (expected "
        "form projects/{project_id})");
  }
  const std::vector<absl::string_view> parts = absl::StrSplit(parent, '/');
  if (parts.size() != 2 || parts[0] != "projects" || parts[1].empty()) {
    return ::grpc::Status(
        ::grpc::StatusCode::INVALID_ARGUMENT,
        absl::StrCat("StorageRead.CreateReadSession: malformed parent ",
                     "(expected form projects/{project_id}): ",
                     parent));
  }
  *project_id = std::string(parts[1]);
  return ::grpc::Status::OK;
}

::grpc::Status StorageReadService::ParseTablePath(
    const std::string& table_path, backend::storage::TableId* out) const {
  if (table_path.empty()) {
    return ::grpc::Status(
        ::grpc::StatusCode::INVALID_ARGUMENT,
        "StorageRead.CreateReadSession: read_session.table is required "
        "(expected form "
        "projects/{project_id}/datasets/{dataset_id}/tables/{table_id})");
  }
  const std::vector<absl::string_view> parts = absl::StrSplit(table_path, '/');
  if (parts.size() != 6 || parts[0] != "projects" || parts[2] != "datasets" ||
      parts[4] != "tables" || parts[1].empty() || parts[3].empty() ||
      parts[5].empty()) {
    return ::grpc::Status(
        ::grpc::StatusCode::INVALID_ARGUMENT,
        absl::StrCat("StorageRead.CreateReadSession: malformed ",
                     "read_session.table (expected form ",
                     "projects/{project_id}/datasets/{dataset_id}/tables/",
                     "{table_id}): ",
                     table_path));
  }
  out->project_id = std::string(parts[1]);
  out->dataset_id = std::string(parts[3]);
  out->table_id = std::string(parts[5]);
  return ::grpc::Status::OK;
}

std::string StorageReadService::NewSessionId(const std::string& project_id) {
  const std::int64_t id = next_session_id_++;
  return absl::StrCat("projects/", project_id, "/locations/-/sessions/s", id);
}

std::string StorageReadService::StreamIdForSession(
    const std::string& session_name, std::int64_t stream_index) const {
  return absl::StrCat(session_name, "/streams/", stream_index);
}

::grpc::Status StorageReadService::ParseReadStreamName(
    const std::string& read_stream,
    std::string* session_name,
    std::string* stream_key) const {
  if (read_stream.empty()) {
    return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT,
                          "StorageRead: read_stream is required");
  }
  constexpr absl::string_view kStreamsMarker = "/streams/";
  const auto pos = read_stream.rfind(kStreamsMarker);
  if (pos == std::string::npos || pos + kStreamsMarker.size() >= read_stream.size()) {
    return ::grpc::Status(
        ::grpc::StatusCode::INVALID_ARGUMENT,
        absl::StrCat("StorageRead: malformed read_stream (expected "
                     "{session_name}/streams/{id}): ",
                     read_stream));
  }
  *session_name = read_stream.substr(0, pos);
  *stream_key = read_stream.substr(pos + kStreamsMarker.size());
  if (stream_key->empty()) {
    return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT,
                          absl::StrCat("StorageRead: empty stream id in ",
                                       read_stream));
  }
  return ::grpc::Status::OK;
}

absl::StatusOr<std::int64_t> StorageReadService::CountTableRows(
    const backend::storage::TableId& table) const {
  return storage_->CountRows(table);
}

::grpc::Status StorageReadService::CreateReadSession(
    ::grpc::ServerContext* /*context*/,
    const v1::CreateReadSessionRequest* request,
    v1::ReadSession* response) {
  if (request == nullptr || response == nullptr) {
    return ::grpc::Status(::grpc::StatusCode::INTERNAL,
                          "StorageRead.CreateReadSession: request and "
                          "response must be non-null");
  }

  std::string project_id;
  if (auto s = ParseParent(request->parent(), &project_id); !s.ok()) {
    return s;
  }

  backend::storage::TableId table_id;
  if (auto s = ParseTablePath(request->read_session().table(), &table_id);
      !s.ok()) {
    return s;
  }
  if (table_id.project_id != project_id &&
      table_id.project_id != kPublicDataProject) {
    return ::grpc::Status(
        ::grpc::StatusCode::INVALID_ARGUMENT,
        absl::StrCat("StorageRead.CreateReadSession: parent project (",
                     project_id,
                     ") does not match read_session.table project (",
                     table_id.project_id,
                     ")"));
  }

  absl::StatusOr<backend::schema::TableSchema> schema_or =
      storage_->GetSchema(table_id);
  if (!schema_or.ok()) {
    return AbslToGrpcStatus(schema_or.status());
  }

  std::optional<std::string> where_sql;
  if (request->read_session().has_read_options() &&
      !request->read_session().read_options().row_restriction().empty()) {
    std::string transpiled;
    const absl::Status parse_status = TranspileRowRestriction(
            request->read_session().read_options().row_restriction(),
            table_id,
            storage_,
            &transpiled);
    if (!parse_status.ok()) {
      return AbslToGrpcStatus(parse_status);
    }
    where_sql = std::move(transpiled);
  }

  std::vector<std::string> selected_fields;
  if (request->read_session().has_read_options() &&
      request->read_session().read_options().selected_fields_size() > 0) {
    selected_fields.reserve(
        request->read_session().read_options().selected_fields_size());
    for (const auto& name :
         request->read_session().read_options().selected_fields()) {
      if (name.empty()) {
        return ::grpc::Status(
            ::grpc::StatusCode::INVALID_ARGUMENT,
            "StorageRead.CreateReadSession: read_options.selected_fields "
            "entries must be non-empty");
      }
      if (FindColumnByName(*schema_or, name) == kColumnNotFound) {
        return ::grpc::Status(
            ::grpc::StatusCode::INVALID_ARGUMENT,
            absl::StrCat(
                "StorageRead.CreateReadSession: read_options.selected_fields "
                "names unknown column `",
                name,
                "` (table has no top-level column with that name)"));
      }
      selected_fields.emplace_back(name);
    }
  }

  absl::StatusOr<std::int64_t> total_rows_or = CountTableRows(table_id);
  if (!total_rows_or.ok()) {
    return AbslToGrpcStatus(total_rows_or.status());
  }
  const std::int64_t total_rows = *total_rows_or;
  const std::int32_t stream_count =
      NegotiateStreamCount(request->max_stream_count(), total_rows);

  std::string session_name;
  std::vector<LocalStreamPartition> partitions;
  {
    absl::MutexLock lock(&mu_);
    session_name = NewSessionId(project_id);
    partitions = BuildPartitions(session_name, stream_count, total_rows);
    SessionState state;
    state.table = table_id;
    state.schema = *schema_or;
    state.where_sql = where_sql;
    state.selected_fields = selected_fields;
    state.total_rows = total_rows;
    for (const auto& part : partitions) {
      StreamPartition stored;
      stored.name = part.name;
      stored.row_start = part.row_start;
      stored.row_end = part.row_end;
      state.streams.emplace(stored.name, std::move(stored));
    }
    sessions_.emplace(session_name, std::move(state));
  }

  response->set_name(session_name);
  response->set_table(request->read_session().table());
  if (selected_fields.empty()) {
    backend::schema::TableSchemaToProto(*schema_or, response->mutable_schema());
  } else {
    const backend::schema::TableSchema projected =
        ProjectSchemaForResponse(*schema_or, selected_fields);
    backend::schema::TableSchemaToProto(projected, response->mutable_schema());
  }
  if (request->read_session().has_read_options()) {
    *response->mutable_read_options() = request->read_session().read_options();
  }
  for (const auto& part : partitions) {
    auto* stream = response->add_streams();
    stream->set_name(part.name);
  }
  return ::grpc::Status::OK;
}

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
  if (auto s = ParseReadStreamName(request->read_stream(), &session_name,
                                   &stream_key);
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
      range_start + static_cast<std::int64_t>(std::floor(span * fraction));
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
  remainder.name =
      absl::StrCat(session_name, "/streams/split", split_id, "r");
  remainder.row_start = split_at;
  remainder.row_end = original.row_end;

  session_it->second.streams.emplace(primary.name, primary);
  session_it->second.streams.emplace(remainder.name, remainder);

  response->mutable_primary_stream()->set_name(primary.name);
  response->mutable_remainder_stream()->set_name(remainder.name);
  return ::grpc::Status::OK;
}

std::size_t StorageReadService::SessionsForTesting() const {
  absl::MutexLock lock(&mu_);
  return sessions_.size();
}

}  // namespace frontend
}  // namespace bigquery_emulator
