#include "frontend/handlers/storage_write_internal.h"

#include <string>
#include <utility>
#include <vector>

#include "absl/strings/str_cat.h"
#include "absl/strings/str_split.h"

namespace bigquery_emulator {
namespace frontend {
namespace internal {

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

}  // namespace internal
}  // namespace frontend
}  // namespace bigquery_emulator
