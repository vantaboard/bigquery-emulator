#include "frontend/handlers/storage_read.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/match.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_split.h"
#include "absl/strings/string_view.h"
#include "absl/synchronization/mutex.h"
#include "backend/schema/schema.h"
#include "backend/storage/row_restriction.h"
#include "backend/storage/storage.h"
#include "proto/emulator.pb.h"
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

// ValueToCell mirrors the converter in `catalog.cc`. Kept duplicated
// rather than shared because the StorageRead handler ships
// independently and the helper is small; bringing in the catalog
// header just for this one function would invert the dependency
// order (storage_read is a sibling of catalog under
// `//frontend/handlers/`, not its consumer).
//
// The wire shape matches `Catalog.ListRows`: every primitive lands on
// `Cell.string_value` and NULLs on `Cell.null_value`. The gateway
// (plan 39) decodes both REST surfaces with the same converter, so
// keeping the wire shape identical between `tabledata.list` and
// `StorageRead.ReadRows` avoids a second decoder.
void ValueToCell(const backend::storage::Value& value, v1::Cell* out) {
  using Kind = backend::storage::Value::Kind;
  out->Clear();
  switch (value.kind()) {
    case Kind::kNull:
      out->set_null_value(true);
      return;
    case Kind::kBool:
      out->set_string_value(value.bool_value() ? "true" : "false");
      return;
    case Kind::kInt64:
      out->set_string_value(absl::StrCat(value.int64_value()));
      return;
    case Kind::kFloat64:
      out->set_string_value(absl::StrCat(value.float64_value()));
      return;
    case Kind::kString:
    case Kind::kBytes:
      out->set_string_value(value.string_value());
      return;
    case Kind::kArray: {
      auto* arr = out->mutable_array();
      for (const auto& el : value.array_value()) {
        ValueToCell(el, arr->add_elements());
      }
      return;
    }
    case Kind::kStruct: {
      auto* st = out->mutable_struct_value();
      for (const auto& f : value.struct_value()) {
        ValueToCell(f, st->add_fields());
      }
      return;
    }
  }
}

// SchemasEqualByShape compares two `TableSchema` snapshots column-by-
// column, looking only at the bits the handler cares about for drift
// detection: column count, column names, BigQuery types, and modes
// (NULLABLE / REQUIRED / REPEATED). Descriptions and nested struct
// field details are intentionally ignored — they can change without
// invalidating an already-served stream, and pinning them would
// surface noisy false-positive FAILED_PRECONDITION replies when the
// catalog handler updates them out-of-band.
bool SchemasEqualByShape(const backend::schema::TableSchema& a,
                         const backend::schema::TableSchema& b) {
  if (a.columns.size() != b.columns.size()) return false;
  for (size_t i = 0; i < a.columns.size(); ++i) {
    if (a.columns[i].name != b.columns[i].name) return false;
    if (a.columns[i].type != b.columns[i].type) return false;
    if (a.columns[i].mode != b.columns[i].mode) return false;
  }
  return true;
}

// FindColumnByName returns the index of the column named `name` in
// `schema`, or `npos` if no top-level column has that name. Used to
// validate `selected_fields` at CreateReadSession time so a typo
// surfaces as INVALID_ARGUMENT before the streaming RPC starts.
constexpr std::size_t kColumnNotFound = static_cast<std::size_t>(-1);
std::size_t FindColumnByName(const backend::schema::TableSchema& schema,
                             absl::string_view name) {
  for (std::size_t i = 0; i < schema.columns.size(); ++i) {
    if (schema.columns[i].name == name) return i;
  }
  return kColumnNotFound;
}

// Builds a projected TableSchema from `schema` containing only the
// columns named by `field_names`, in the order they appear in
// `field_names`. The caller must have already validated each name
// against `schema` (CreateReadSession does this); a still-unknown
// name here is a programming error.
backend::schema::TableSchema ProjectSchemaForResponse(
    const backend::schema::TableSchema& schema,
    const std::vector<std::string>& field_names) {
  backend::schema::TableSchema out;
  out.columns.reserve(field_names.size());
  for (const std::string& name : field_names) {
    const std::size_t idx = FindColumnByName(schema, name);
    if (idx != kColumnNotFound) {
      out.columns.push_back(schema.columns[idx]);
    }
  }
  return out;
}

}  // namespace

StorageReadService::StorageReadService(backend::storage::Storage* storage)
    : storage_(storage) {}

::grpc::Status StorageReadService::ParseParent(const std::string& parent,
                                               std::string* project_id) const {
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
  // Caller must hold `mu_` (see header annotation
  // ABSL_EXCLUSIVE_LOCKS_REQUIRED(mu_)). Bumping the counter and
  // formatting the string here keeps the critical section short.
  const std::int64_t id = next_session_id_++;
  return absl::StrCat("projects/", project_id, "/locations/-/sessions/s", id);
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
                     project_id,
                     ") does not match read_session.table ",
                     "project (",
                     table_id.project_id,
                     ")"));
  }

  // Validate the table exists by fetching its schema. NOT_FOUND here
  // surfaces as gRPC NOT_FOUND -> BigQuery REST 404 notFound.
  absl::StatusOr<backend::schema::TableSchema> schema_or =
      storage_->GetSchema(table_id);
  if (!schema_or.ok()) {
    return AbslToGrpcStatus(schema_or.status());
  }

  // Plan 39: parse the optional row_restriction NOW so a malformed
  // predicate fails the CreateReadSession request rather than the
  // downstream ReadRows call (which the caller will not retry on
  // INVALID_ARGUMENT — by then they have already committed the
  // session to the harness side).
  std::optional<backend::storage::EqualityPredicate> predicate;
  if (request->read_session().has_read_options() &&
      !request->read_session().read_options().row_restriction().empty()) {
    backend::storage::EqualityPredicate parsed;
    const absl::Status parse_status = backend::storage::ParseRowRestriction(
        request->read_session().read_options().row_restriction(),
        *schema_or,
        &parsed);
    if (!parse_status.ok()) {
      return AbslToGrpcStatus(parse_status);
    }
    predicate = std::move(parsed);
  }

  // Plan 15 (storage-read-write): validate `selected_fields` at
  // session-mint time. Each entry must name a top-level column of
  // the source table; an unknown name surfaces as INVALID_ARGUMENT
  // before any streaming starts. We deliberately reject an empty
  // string entry too — the caller's protobuf `repeated string`
  // would normally only contain non-empty values, but a wire-level
  // bug that smuggled `""` would silently match nothing on the
  // backend, which is the exact "silent approximation" plan 15
  // forbids.
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

  std::string session_name;
  std::string stream_name;
  {
    absl::MutexLock lock(&mu_);
    session_name = NewSessionId(project_id);
    stream_name = StreamIdForSession(session_name);
    SessionState state;
    state.table = table_id;
    state.schema = *schema_or;
    state.equality_predicate = predicate;
    state.selected_fields = selected_fields;
    sessions_.emplace(session_name, std::move(state));
  }

  // Build the response. Plan 37: one session, one stream, schema
  // attached, read_options echoed back. Plan 15 (storage-read-write):
  // when the caller pinned `selected_fields`, the response schema
  // reflects the projection — both the ordering and the column list
  // — so a downstream Avro / Arrow decoder reads cells against the
  // same shape ReadRows will emit. The full schema is still stashed
  // on the SessionState so the drift check in ReadRows runs against
  // the source table.
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
  auto* stream = response->add_streams();
  stream->set_name(stream_name);
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

  // 1. Recover the session name. Plan 37 mints stream ids of the form
  // `{session_name}/streams/0`; anything else is a client bug and
  // surfaces as INVALID_ARGUMENT so the gateway can return BigQuery's
  // 400 envelope. We deliberately do NOT accept arbitrary trailing
  // `/streams/{N}` here because plan 37 only mints stream 0; any
  // other stream id would be one we never created, which is
  // ambiguous between "wrong session" and "wrong stream index".
  //
  // Validate this BEFORE the writer null-check so unit tests can
  // exercise the parser without standing up a server (the writer is
  // only needed once we start streaming rows).
  const std::string& read_stream = request->read_stream();
  constexpr absl::string_view kStreamsSuffix = "/streams/0";
  if (read_stream.empty() || read_stream.size() <= kStreamsSuffix.size() ||
      !absl::EndsWith(read_stream, kStreamsSuffix)) {
    return ::grpc::Status(
        ::grpc::StatusCode::INVALID_ARGUMENT,
        absl::StrCat("StorageRead.ReadRows: read_stream must be of the form "
                     "{session_name}/streams/0 (got: ",
                     read_stream,
                     ")"));
  }
  const std::string session_name =
      read_stream.substr(0, read_stream.size() - kStreamsSuffix.size());

  // 2. Look the session up and copy out the fields we need under the
  // lock so the streaming body below does not hold the mutex for the
  // entire I/O duration.
  backend::storage::TableId table;
  backend::schema::TableSchema session_schema;
  std::optional<backend::storage::EqualityPredicate> predicate;
  std::vector<std::string> selected_fields;
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
    table = it->second.table;
    session_schema = it->second.schema;
    predicate = it->second.equality_predicate;
    selected_fields = it->second.selected_fields;
  }

  // 3. Schema drift check. If the table schema changed between
  // CreateReadSession and ReadRows, the caller is decoding rows with
  // a stale shape and we cannot safely serve the request. BigQuery's
  // public Storage Read API uses ABORTED here; plan 38 follows the
  // simpler FAILED_PRECONDITION convention the rest of the engine
  // already uses for "preconditions of the call have changed."
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

  // From here on we are about to stream rows; the writer must be
  // present. In real gRPC dispatch the server framework guarantees a
  // non-null writer, but the unit tests that exercise the validation
  // path above pass nullptr -- so we delay the null check until we
  // actually need to write.
  if (writer == nullptr) {
    return ::grpc::Status(::grpc::StatusCode::INTERNAL,
                          "StorageRead.ReadRows: writer must be non-null");
  }

  // 4. Open a read stream against the storage backend. We honor the
  // request-side offset (per the proto: gateway uses this to resume
  // after a transient failure). row_limit is not on the request
  // surface; we pass 0 so the iterator yields every remaining row.
  // Plan 39: the parsed row_restriction (if any) lives on the
  // session — both backends apply the predicate before offset/limit
  // so the wire shape matches BigQuery's documented semantics
  // (offset is over the post-filter row stream). Plan 15
  // (storage-read-write): the projected `selected_fields` list (if
  // any) is also session-resident so the same projection applies
  // verbatim across resumptions.
  backend::storage::ReadFilter filter;
  filter.offset = request->offset();
  filter.row_limit = 0;
  filter.equality_predicate = predicate;
  filter.selected_fields = selected_fields;
  absl::StatusOr<std::unique_ptr<backend::storage::RowIterator>> iter_or =
      storage_->CreateReadStream(table, filter);
  if (!iter_or.ok()) {
    return AbslToGrpcStatus(iter_or.status());
  }
  std::unique_ptr<backend::storage::RowIterator> iter = std::move(*iter_or);

  // 5. Stream rows in pages of kReadRowsBatchSize. Each Write that
  // returns false means the client cancelled or the channel broke;
  // we surface ABORTED so the gateway can decide whether to retry
  // the stream.
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

std::size_t StorageReadService::SessionsForTesting() const {
  absl::MutexLock lock(&mu_);
  return sessions_.size();
}

}  // namespace frontend
}  // namespace bigquery_emulator
