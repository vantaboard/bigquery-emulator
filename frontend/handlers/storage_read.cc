#include "frontend/handlers/storage_read.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_split.h"
#include "absl/strings/string_view.h"
#include "absl/synchronization/mutex.h"
#include "backend/schema/schema.h"
#include "backend/storage/storage.h"
#include "proto/storage_read.pb.h"

namespace bigquery_emulator {
namespace frontend {

namespace {

// AbslToGrpcStatus mirrors the catalog handler's mapping table
// (see `frontend/handlers/catalog.cc::AbslToGrpcStatus`). Kept
// duplicated rather than shared because the storage_read handler
// ships independently and the table is small enough that pulling in
// the catalog header just for the conversion would invert the
// dependency.
::grpc::Status AbslToGrpcStatus(const absl::Status& status) {
  if (status.ok()) return ::grpc::Status::OK;
  ::grpc::StatusCode code = ::grpc::StatusCode::INTERNAL;
  switch (status.code()) {
    case absl::StatusCode::kNotFound:
      code = ::grpc::StatusCode::NOT_FOUND;
      break;
    case absl::StatusCode::kAlreadyExists:
      code = ::grpc::StatusCode::ALREADY_EXISTS;
      break;
    case absl::StatusCode::kInvalidArgument:
      code = ::grpc::StatusCode::INVALID_ARGUMENT;
      break;
    case absl::StatusCode::kFailedPrecondition:
      code = ::grpc::StatusCode::FAILED_PRECONDITION;
      break;
    case absl::StatusCode::kUnimplemented:
      code = ::grpc::StatusCode::UNIMPLEMENTED;
      break;
    default:
      code = ::grpc::StatusCode::INTERNAL;
      break;
  }
  return ::grpc::Status(code, std::string(status.message()));
}

}  // namespace

StorageReadService::StorageReadService(backend::storage::Storage* storage)
    : storage_(storage) {}

::grpc::Status StorageReadService::ParseParent(
    const std::string& parent, std::string* project_id) const {
  // Accept `projects/{project_id}` and reject anything else. The
  // public BigQuery surface allows a longer path
  // (`projects/{p}/locations/{l}`) but plan 37 keeps the location
  // slot synthetic (`-`) because the gateway does not yet route on
  // location for read sessions.
  if (parent.empty()) {
    return ::grpc::Status(
        ::grpc::StatusCode::INVALID_ARGUMENT,
        "StorageRead.CreateReadSession: parent is required (expected "
        "form projects/{project_id})");
  }
  const std::vector<absl::string_view> parts =
      absl::StrSplit(parent, '/');
  if (parts.size() != 2 || parts[0] != "projects" || parts[1].empty()) {
    return ::grpc::Status(
        ::grpc::StatusCode::INVALID_ARGUMENT,
        absl::StrCat("StorageRead.CreateReadSession: malformed parent ",
                     "(expected form projects/{project_id}): ", parent));
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
  const std::vector<absl::string_view> parts =
      absl::StrSplit(table_path, '/');
  if (parts.size() != 6 || parts[0] != "projects" ||
      parts[2] != "datasets" || parts[4] != "tables" ||
      parts[1].empty() || parts[3].empty() || parts[5].empty()) {
    return ::grpc::Status(
        ::grpc::StatusCode::INVALID_ARGUMENT,
        absl::StrCat("StorageRead.CreateReadSession: malformed ",
                     "read_session.table (expected form ",
                     "projects/{project_id}/datasets/{dataset_id}/tables/",
                     "{table_id}): ", table_path));
  }
  out->project_id = std::string(parts[1]);
  out->dataset_id = std::string(parts[3]);
  out->table_id = std::string(parts[5]);
  return ::grpc::Status::OK;
}

std::string StorageReadService::NewSessionId(const std::string& project_id) {
  // Caller must hold `mu_` (see header annotation
  // ABSL_EXCLUSIVE_LOCKS_REQUIRED(mu_)). Bumping the counter and
  // formatting the string here keeps the critical section short.
  const std::int64_t id = next_session_id_++;
  return absl::StrCat("projects/", project_id,
                      "/locations/-/sessions/s", id);
}

std::string StorageReadService::StreamIdForSession(
    const std::string& session_name) const {
  return absl::StrCat(session_name, "/streams/0");
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
  // Enforce parent / table project consistency so a caller pinning
  // parent=projects/A and read_session.table=projects/B/... does not
  // get a session that bridges projects. BigQuery returns
  // INVALID_ARGUMENT on the same shape.
  if (table_id.project_id != project_id) {
    return ::grpc::Status(
        ::grpc::StatusCode::INVALID_ARGUMENT,
        absl::StrCat("StorageRead.CreateReadSession: parent project (",
                     project_id, ") does not match read_session.table ",
                     "project (", table_id.project_id, ")"));
  }

  // Validate the table exists by fetching its schema. NOT_FOUND here
  // surfaces as gRPC NOT_FOUND -> BigQuery REST 404 notFound.
  absl::StatusOr<backend::schema::TableSchema> schema_or =
      storage_->GetSchema(table_id);
  if (!schema_or.ok()) {
    return AbslToGrpcStatus(schema_or.status());
  }

  std::string session_name;
  std::string stream_name;
  {
    absl::MutexLock lock(&mu_);
    session_name = NewSessionId(project_id);
    stream_name = StreamIdForSession(session_name);
    SessionState state;
    state.table = table_id;
    state.schema = *schema_or;
    sessions_.emplace(session_name, std::move(state));
  }

  // Build the response. Plan 37: one session, one stream, schema
  // attached, read_options echoed back (we accept selected_fields /
  // row_restriction but do not enforce them until plan 38).
  response->set_name(session_name);
  response->set_table(request->read_session().table());
  backend::schema::TableSchemaToProto(*schema_or, response->mutable_schema());
  if (request->read_session().has_read_options()) {
    *response->mutable_read_options() =
        request->read_session().read_options();
  }
  auto* stream = response->add_streams();
  stream->set_name(stream_name);
  return ::grpc::Status::OK;
}

::grpc::Status StorageReadService::ReadRows(
    ::grpc::ServerContext* /*context*/,
    const v1::ReadRowsRequest* /*request*/,
    ::grpc::ServerWriter<v1::ReadRowsResponse>* /*writer*/) {
  return ::grpc::Status(
      ::grpc::StatusCode::UNIMPLEMENTED,
      "StorageRead.ReadRows: streaming rows lands in plan 38 "
      "(storage-read-rows); plan 37 only implements CreateReadSession");
}

std::size_t StorageReadService::SessionsForTesting() const {
  absl::MutexLock lock(&mu_);
  return sessions_.size();
}

}  // namespace frontend
}  // namespace bigquery_emulator
