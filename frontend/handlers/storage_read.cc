#include "frontend/handlers/storage_read.h"

#include <grpcpp/server_context.h>
#include <grpcpp/support/status.h>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <utility>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "absl/synchronization/mutex.h"
#include "backend/schema/schema.h"
#include "backend/storage/storage.h"
#include "frontend/handlers/handler_common.h"
#include "frontend/handlers/storage_read_internal.h"
#include "frontend/handlers/storage_row_restriction_analyze.h"
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
  return static_cast<std::int32_t>(std::min<std::int64_t>(capped, total_rows));
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
  if (pos == std::string::npos ||
      pos + kStreamsMarker.size() >= read_stream.size()) {
    return ::grpc::Status(
        ::grpc::StatusCode::INVALID_ARGUMENT,
        absl::StrCat("StorageRead: malformed read_stream (expected "
                     "{session_name}/streams/{id}): ",
                     read_stream));
  }
  *session_name = read_stream.substr(0, pos);
  *stream_key = read_stream.substr(pos + kStreamsMarker.size());
  if (stream_key->empty()) {
    return ::grpc::Status(
        ::grpc::StatusCode::INVALID_ARGUMENT,
        absl::StrCat("StorageRead: empty stream id in ", read_stream));
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

std::size_t StorageReadService::SessionsForTesting() const {
  absl::MutexLock lock(&mu_);
  return sessions_.size();
}

}  // namespace frontend
}  // namespace bigquery_emulator
