#ifndef BIGQUERY_EMULATOR_BACKEND_SCHEMA_SCHEMA_H_
#define BIGQUERY_EMULATOR_BACKEND_SCHEMA_SCHEMA_H_

// Engine-agnostic schema types used by the in-memory and DuckDB storage
// backends. The Go gateway and the C++ engine speak proto schemas
// (`bigquery_emulator.v1.{Field,Table}Schema` from
// `proto/emulator.proto`), but every layer below the gRPC service
// boundary works in terms of these C++ structs so the storage backends
// never have to depend on the proto runtime or on any GoogleSQL types.
//
// Conversions to/from proto live alongside the struct definitions; both
// directions are total — unknown BigQuery type names round-trip as
// `ColumnType::kUnknown` with the original string preserved on
// `ColumnSchema::raw_type` so callers can decide whether to error or
// pass them through.

#include <string>
#include <vector>

#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"
#include "proto/emulator.pb.h"

namespace bigquery_emulator {
namespace backend {
namespace schema {

// BigQuery scalar / structural types as documented under
// `docs/bigquery/docs/reference/standard-sql/data-types.md`. The values
// mirror `google.cloud.bigquery.v2.StandardSqlDataType.TypeKind` and the
// per-cell BigQuery REST wire shape, which is what the gateway speaks.
enum class ColumnType {
  kUnknown = 0,
  kInt64,
  kFloat64,
  kBool,
  kString,
  kBytes,
  kDate,
  kTime,
  kDatetime,
  kTimestamp,
  kNumeric,
  kBignumeric,
  kJson,
  kGeography,
  kArray,
  kStruct,
};

// BigQuery field cardinality. Empty / unset modes default to NULLABLE,
// matching the public REST contract.
enum class ColumnMode {
  kNullable = 0,
  kRequired,
  kRepeated,
};

// One column in a BigQuery table or one nested field inside a STRUCT
// column. ARRAY columns store the element schema as a single entry in
// `fields`; STRUCT columns store the ordered field list in `fields`.
struct ColumnSchema {
  std::string name;
  ColumnType type = ColumnType::kUnknown;
  ColumnMode mode = ColumnMode::kNullable;
  std::string description;
  // Original `type` string from the proto when the C++ side does not
  // know the BigQuery type name yet. Populated only when `type ==
  // kUnknown`; otherwise empty.
  std::string raw_type;
  // Nested fields (STRUCT) or element schema (ARRAY).
  std::vector<ColumnSchema> fields;
};

// Whole-table schema: the ordered list of top-level columns.
struct TableSchema {
  std::vector<ColumnSchema> columns;
};

// ---------------------------------------------------------------------------
// Type-name conversions
//
// These are deliberately total: any string parses, with unknown names
// mapping to `kUnknown` so the caller can choose whether to reject or
// pass the original through. `ColumnTypeName` returns the canonical
// uppercase name and is the inverse of `ParseColumnType` for
// well-known kinds.
// ---------------------------------------------------------------------------

absl::string_view ColumnTypeName(ColumnType type);
ColumnType ParseColumnType(absl::string_view name);

absl::string_view ColumnModeName(ColumnMode mode);
ColumnMode ParseColumnMode(absl::string_view name);

// ---------------------------------------------------------------------------
// Proto conversions
//
// Total in both directions; the proto round-trip preserves unknown type
// names through `ColumnSchema::raw_type` (see notes on the struct).
// `FromProto` only fails when a required field on the proto is empty
// (for example a column with no name) — every other malformedness is
// surfaced as `kUnknown` so analysis can pick it up later.
// ---------------------------------------------------------------------------

absl::StatusOr<ColumnSchema> ColumnSchemaFromProto(const v1::FieldSchema& proto);
absl::StatusOr<TableSchema> TableSchemaFromProto(const v1::TableSchema& proto);

void ColumnSchemaToProto(const ColumnSchema& column, v1::FieldSchema* out);
void TableSchemaToProto(const TableSchema& schema, v1::TableSchema* out);

// ---------------------------------------------------------------------------
// DuckDB type-name conversions
//
// Used by the DuckDB-backed storage (`backend/storage/duckdb/`) and,
// later, the DuckDB engine transpiler (Phase 5.B) to lower BigQuery
// column types onto DuckDB's type system. The mapping is intentionally
// total: any unknown BigQuery type maps to `VARCHAR` so a misconfigured
// schema does not fail the CREATE TABLE; conversely, `FromDuckDBType`
// returns `kUnknown` for types DuckDB knows but BigQuery does not
// (e.g. `UTINYINT`).
//
// Scalar mapping:
//
//   BigQuery       DuckDB
//   ---------      ----------
//   INT64          BIGINT
//   FLOAT64        DOUBLE
//   BOOL           BOOLEAN
//   STRING         VARCHAR
//   BYTES          BLOB
//   DATE           DATE
//   TIME           TIME
//   DATETIME       TIMESTAMP
//   TIMESTAMP      TIMESTAMP WITH TIME ZONE
//   NUMERIC        DECIMAL(38, 9)
//   BIGNUMERIC     DECIMAL(38, 38)
//   JSON           JSON
//   GEOGRAPHY      VARCHAR (no native; round-tripped as WKT)
//
// Container types do not have a stable scalar name — use
// `ColumnSchemaToDuckDBType` for the full type expression
// (`BIGINT[]`, `STRUCT(a BIGINT, b VARCHAR)`, ...).
// ---------------------------------------------------------------------------

absl::string_view ToDuckDBType(ColumnType type);
ColumnType FromDuckDBType(absl::string_view duckdb_type);

// Renders the full DuckDB type expression for `column`, honoring both
// the cardinality (`mode == kRepeated` becomes a LIST `T[]`) and any
// nested STRUCT fields. Identifiers inside STRUCTs are double-quote
// escaped so column names with hyphens / unicode round-trip safely
// through a `CREATE TABLE` statement.
std::string ColumnSchemaToDuckDBType(const ColumnSchema& column);

}  // namespace schema
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_SCHEMA_SCHEMA_H_
