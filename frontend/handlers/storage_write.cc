#include "frontend/handlers/storage_write.h"

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <ctime>
#include <iomanip>
#include <map>
#include <memory>
#include <sstream>
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
#include "absl/types/span.h"
#include "backend/schema/schema.h"
#include "backend/storage/storage.h"
#include "proto/emulator.pb.h"
#include "proto/storage_write.pb.h"

namespace bigquery_emulator {
namespace frontend {

namespace {

// AbslToGrpcStatus mirrors the catalog handler's mapping table
// (see `frontend/handlers/catalog.cc::AbslToGrpcStatus`). The
// duplication is justified the same way it is for `storage_read.cc`:
// the StorageWrite handler ships independently and the table is small
// enough that pulling in the catalog header just for the conversion
// would invert the dependency.
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

// CellToValue mirrors `catalog.cc::CellToValue`. The Storage Write API
// `DataRow.Cell` shape is the same one `Catalog.InsertRows` uses, so
// we lower cells through the same conversion. Numeric/bool cells ride
// on `Cell.string_value` (the BigQuery REST `f`/`v` shape) and the
// DuckDB storage backend re-parses them via CAST literals when the
// column type is non-string — see
// `duckdb_storage.cc::RenderScalarLiteral` for that handshake.
backend::storage::Value CellToValue(const v1::Cell& cell) {
  switch (cell.value_case()) {
    case v1::Cell::kStringValue:
      return backend::storage::Value::String(cell.string_value());
    case v1::Cell::kNullValue:
      return backend::storage::Value::Null();
    case v1::Cell::kArray: {
      std::vector<backend::storage::Value> elements;
      elements.reserve(cell.array().elements_size());
      for (const auto& el : cell.array().elements()) {
        elements.push_back(CellToValue(el));
      }
      return backend::storage::Value::Array(std::move(elements));
    }
    case v1::Cell::kStructValue: {
      std::vector<backend::storage::Value> fields;
      fields.reserve(cell.struct_value().fields_size());
      for (const auto& f : cell.struct_value().fields()) {
        fields.push_back(CellToValue(f));
      }
      return backend::storage::Value::Struct(std::move(fields));
    }
    case v1::Cell::VALUE_NOT_SET:
      break;
  }
  return backend::storage::Value::Null();
}

// Splits a path like `projects/{p}/datasets/{d}/tables/{t}` into
// pieces. Returns false on any malformed shape; the caller maps that
// onto INVALID_ARGUMENT with a stable message.
bool SplitTablePath(absl::string_view path, backend::storage::TableId* out) {
  const std::vector<absl::string_view> parts = absl::StrSplit(path, '/');
  if (parts.size() != 6 || parts[0] != "projects" || parts[2] != "datasets" ||
      parts[4] != "tables" || parts[1].empty() || parts[3].empty() ||
      parts[5].empty()) {
    return false;
  }
  out->project_id = std::string(parts[1]);
  out->dataset_id = std::string(parts[3]);
  out->table_id = std::string(parts[5]);
  return true;
}

// Stitches a TableId back into its canonical path. Used to mint the
// stream name and the `_default` reserved id so the wire shape is
// stable regardless of which entry point built the value.
std::string TablePathFor(const backend::storage::TableId& id) {
  return absl::StrCat("projects/",
                      id.project_id,
                      "/datasets/",
                      id.dataset_id,
                      "/tables/",
                      id.table_id);
}

}  // namespace

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

  // Plan 15: only `_default` (the reserved name for an implicit
  // append-only stream) and explicit COMMITTED streams light up
  // here. PENDING / BUFFERED return UNIMPLEMENTED with a clear
  // message so a producer pinning those types fails fast rather
  // than silently routing through the COMMITTED path.
  v1::WriteStream::Type requested = v1::WriteStream::COMMITTED;
  if (request->has_write_stream()) {
    requested = request->write_stream().type();
    if (requested == v1::WriteStream::TYPE_UNSPECIFIED) {
      requested = v1::WriteStream::COMMITTED;
    }
  }
  if (requested == v1::WriteStream::PENDING ||
      requested == v1::WriteStream::BUFFERED) {
    return ::grpc::Status(
        ::grpc::StatusCode::UNIMPLEMENTED,
        absl::StrCat("StorageWrite.CreateWriteStream: stream type ",
                     v1::WriteStream::Type_Name(requested),
                     " is not implemented in this emulator profile (plan 15 "
                     "lights up _default + COMMITTED only); see the deferred "
                     "follow-up to storage-read-write-api-plan.plan.md"));
  }
  if (requested != v1::WriteStream::COMMITTED) {
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
    state.type = v1::WriteStream::COMMITTED;
    state.create_time = create_time;
    streams_.emplace(stream_name, std::move(state));
  }

  response->set_name(stream_name);
  response->set_type(v1::WriteStream::COMMITTED);
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
    // schema; release before calling AppendRows so the storage
    // backend does not contend with subsequent CreateWriteStream
    // requests while it writes the parquet file.
    backend::storage::TableId table;
    backend::schema::TableSchema schema;
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
      table = it->second.table;
      schema = it->second.schema;
    }

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

    // Forward to the storage append primitive (plan 9). The
    // DuckDB backend writes a fresh parquet snapshot per call,
    // committing the rows immediately — that is the
    // `_default` / `COMMITTED` semantic plan 15 promises.
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
    std::int64_t prior_offset = 0;
    {
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

::grpc::Status StorageWriteService::FinalizeWriteStream(
    ::grpc::ServerContext* /*context*/,
    const v1::FinalizeWriteStreamRequest* /*request*/,
    v1::FinalizeWriteStreamResponse* /*response*/) {
  return ::grpc::Status(
      ::grpc::StatusCode::UNIMPLEMENTED,
      "StorageWrite.FinalizeWriteStream: not implemented in this emulator "
      "profile (plan 15 lights up _default + COMMITTED only; FinalizeWrite "
      "lands with the deferred BUFFERED / PENDING follow-up subagent of "
      "storage-read-write-api-plan.plan.md)");
}

::grpc::Status StorageWriteService::BatchCommitWriteStreams(
    ::grpc::ServerContext* /*context*/,
    const v1::BatchCommitWriteStreamsRequest* /*request*/,
    v1::BatchCommitWriteStreamsResponse* /*response*/) {
  return ::grpc::Status(
      ::grpc::StatusCode::UNIMPLEMENTED,
      "StorageWrite.BatchCommitWriteStreams: not implemented in this emulator "
      "profile (plan 15 lights up _default + COMMITTED only; BatchCommit "
      "lands with the deferred PENDING follow-up subagent of "
      "storage-read-write-api-plan.plan.md)");
}

::grpc::Status StorageWriteService::FlushRows(
    ::grpc::ServerContext* /*context*/,
    const v1::FlushRowsRequest* /*request*/,
    v1::FlushRowsResponse* /*response*/) {
  return ::grpc::Status(
      ::grpc::StatusCode::UNIMPLEMENTED,
      "StorageWrite.FlushRows: not implemented in this emulator "
      "profile (plan 15 lights up _default + COMMITTED only; FlushRows "
      "lands with the deferred BUFFERED follow-up subagent of "
      "storage-read-write-api-plan.plan.md)");
}

std::size_t StorageWriteService::StreamsForTesting() const {
  absl::MutexLock lock(&mu_);
  return streams_.size();
}

}  // namespace frontend
}  // namespace bigquery_emulator
