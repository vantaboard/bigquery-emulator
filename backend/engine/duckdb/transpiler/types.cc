#include "backend/engine/duckdb/transpiler/types.h"

#include <string>

#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"
#include "absl/strings/str_replace.h"
#include "absl/strings/string_view.h"
#include "googlesql/public/type.h"
#include "googlesql/public/type.pb.h"
#include "googlesql/public/types/array_type.h"
#include "googlesql/public/types/struct_type.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace duckdb {
namespace transpiler {

namespace {

// Double-quote escape a DuckDB identifier so STRUCT field names can
// safely round-trip through `STRUCT(name1 T1, ...)`. DuckDB doubles
// embedded `"` characters; we do the same.
std::string QuoteIdentifier(absl::string_view name) {
  return absl::StrCat("\"", absl::StrReplaceAll(name, {{"\"", "\"\""}}), "\"");
}

}  // namespace

absl::string_view DuckDBSqlTypeName(::googlesql::TypeKind kind) {
  switch (kind) {
    case ::googlesql::TYPE_BOOL:
      return "BOOLEAN";
    // GoogleSQL's narrow integer kinds (`INT32`, `UINT32`, `UINT64`)
    // ride through the BigQuery surface as `INT64`; the storage
    // layer flattens them onto a single `BIGINT`, and the transpiler
    // follows suit so `CAST(x AS T)` lines up with the catalog
    // schema the engine sees.
    case ::googlesql::TYPE_INT32:
    case ::googlesql::TYPE_INT64:
    case ::googlesql::TYPE_UINT32:
    case ::googlesql::TYPE_UINT64:
      return "BIGINT";
    case ::googlesql::TYPE_FLOAT:
    case ::googlesql::TYPE_DOUBLE:
      return "DOUBLE";
    case ::googlesql::TYPE_STRING:
      return "VARCHAR";
    case ::googlesql::TYPE_BYTES:
      return "BLOB";
    case ::googlesql::TYPE_DATE:
      return "DATE";
    case ::googlesql::TYPE_TIME:
      return "TIME";
    // BigQuery DATETIME is naive (no zone); DuckDB TIMESTAMP is the
    // matching naive type. TIMESTAMP carries a zone in BigQuery; we
    // emit DuckDB's `TIMESTAMP WITH TIME ZONE` so the wire-side RFC
    // 3339 strings round-trip the way the gateway expects.
    case ::googlesql::TYPE_DATETIME:
      return "TIMESTAMP";
    case ::googlesql::TYPE_TIMESTAMP:
      return "TIMESTAMP WITH TIME ZONE";
    // BigQuery NUMERIC is fixed at 38/9; BIGNUMERIC at 76/38 which
    // exceeds DuckDB's max precision (38), so we lossily clamp the
    // scale on output. The DML / cast-policy work gets to revisit
    // this — for now we mirror `schema::ToDuckDBType`.
    case ::googlesql::TYPE_NUMERIC:
      return "DECIMAL(38, 9)";
    case ::googlesql::TYPE_BIGNUMERIC:
      return "DECIMAL(38, 38)";
    case ::googlesql::TYPE_JSON:
      return "JSON";
    case ::googlesql::TYPE_INTERVAL:
      return "INTERVAL";
    case ::googlesql::TYPE_GEOGRAPHY:
      return "VARCHAR";
    case ::googlesql::TYPE_UUID:
      return "UUID";
    // Container kinds — the bare head identifier is the best we can
    // do without the inner shape. Callers that need the full
    // expression should reach for `ToDuckDBSqlType`.
    case ::googlesql::TYPE_ARRAY:
      return "LIST";
    case ::googlesql::TYPE_STRUCT:
      return "STRUCT";
    // Everything else — proto / enum / range / measure / graph /
    // tokenlist / extended / unknown — falls back to VARCHAR so the
    // transpiled CAST keeps compiling even if we don't have a
    // first-class DuckDB analog yet. The shape tracker records the
    // gap.
    default:
      return "VARCHAR";
  }
}

std::string ToDuckDBSqlType(const ::googlesql::Type& type) {
  switch (type.kind()) {
    case ::googlesql::TYPE_ARRAY: {
      const ::googlesql::ArrayType* array = type.AsArray();
      if (array == nullptr || array->element_type() == nullptr) {
        // Defensive: an ARRAY without an element type is malformed;
        // emit `VARCHAR[]` rather than crash so the downstream
        // DuckDB parser surfaces the failure with the offending SQL.
        return "VARCHAR[]";
      }
      return absl::StrCat(ToDuckDBSqlType(*array->element_type()), "[]");
    }
    case ::googlesql::TYPE_STRUCT: {
      const ::googlesql::StructType* struct_type = type.AsStruct();
      if (struct_type == nullptr) return "STRUCT()";
      std::vector<std::string> fields;
      fields.reserve(struct_type->num_fields());
      for (int i = 0; i < struct_type->num_fields(); ++i) {
        const ::googlesql::StructField& f = struct_type->field(i);
        if (f.type == nullptr) continue;
        fields.push_back(absl::StrCat(
            QuoteIdentifier(f.name), " ", ToDuckDBSqlType(*f.type)));
      }
      return absl::StrCat("STRUCT(", absl::StrJoin(fields, ", "), ")");
    }
    default:
      return std::string(DuckDBSqlTypeName(type.kind()));
  }
}

}  // namespace transpiler
}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
