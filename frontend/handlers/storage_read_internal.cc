#include "frontend/handlers/storage_read_internal.h"

namespace bigquery_emulator {
namespace frontend {
namespace internal {

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
