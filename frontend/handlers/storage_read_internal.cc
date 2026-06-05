#include "frontend/handlers/storage_read_internal.h"

#include <string>

#include "absl/strings/str_cat.h"

namespace bigquery_emulator {
namespace frontend {
namespace internal {

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

}  // namespace internal
}  // namespace frontend
}  // namespace bigquery_emulator
