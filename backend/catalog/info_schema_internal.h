#ifndef BIGQUERY_EMULATOR_BACKEND_CATALOG_INFO_SCHEMA_INTERNAL_H_
#define BIGQUERY_EMULATOR_BACKEND_CATALOG_INFO_SCHEMA_INTERNAL_H_

// Internal helpers shared between the INFORMATION_SCHEMA materializer
// translation units (info_schema_table.cc, info_schema_rows.cc,
// info_schema_schema.cc). Not part of the public catalog surface.

#include <string>

#include "backend/catalog/info_schema_table.h"
#include "backend/schema/schema.h"

namespace bigquery_emulator {
namespace backend {
namespace catalog {
namespace info_schema_internal {

// Per-view row schema descriptor. Column names, order, and types are
// contract surfaces mirroring docs/bigquery/docs/information-schema-*.md.
schema::TableSchema RowSchemaForView(InfoSchemaViewKind kind);

// BigQuery GoogleSQL type name for `column`, used by the data_type
// columns of COLUMNS / COLUMN_FIELD_PATHS (recurses STRUCT / ARRAY).
std::string InfoSchemaDataType(const schema::ColumnSchema& column);

}  // namespace info_schema_internal
}  // namespace catalog
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_CATALOG_INFO_SCHEMA_INTERNAL_H_
