#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_DUCKDB_TRANSPILER_TYPES_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_DUCKDB_TRANSPILER_TYPES_H_

// Type lowering for the DuckDB transpiler.
//
// The transpiler walks a GoogleSQL `ResolvedAST` and emits DuckDB
// SQL. Wherever the emitted SQL needs a DuckDB type expression — the
// second operand of `CAST(x AS T)`, the column type list in a CTE
// projection, the declared type on a STRUCT field, etc. — we
// translate the GoogleSQL `Type` it points at into the matching
// DuckDB type name through the helpers below. Keeping the mapping in
// a single file lets the per-shape `Emit*` methods stay focused on
// SQL shape; the type contract is what the storage layer's
// `schema::ColumnSchemaToDuckDBType` already enforces for column
// definitions, so the two surfaces have to agree.
//
// The mapping is *intentionally total*: any GoogleSQL `TypeKind` we
// don't have an exact DuckDB analog for (`TYPE_GEOGRAPHY`,
// `TYPE_EXTENDED`, ...) lowers to `VARCHAR` so the analyzer-side
// type pointer can still flow through the transpiled SQL. Callers
// that need stricter behavior should consult `SHAPE_TRACKER.md` for
// the per-kind disposition before relying on the value.

#include <string>

#include "absl/strings/string_view.h"
#include "googlesql/public/type.h"
#include "googlesql/public/type.pb.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace duckdb {
namespace transpiler {

// Returns the DuckDB type *head* name for a GoogleSQL `TypeKind`
// (e.g. `BIGINT`, `DOUBLE`, `VARCHAR`). The result is safe to
// substitute directly into `CAST(x AS T)` for scalar conversions
// and into a column definition for scalar columns.
//
// For container kinds (`TYPE_ARRAY`, `TYPE_STRUCT`) the head is
// returned bare (`STRUCT`, `LIST`) because the full DuckDB
// expression requires the element / field shape; callers that need
// the full expression should reach for `ToDuckDBSqlType` instead.
//
// The mapping mirrors `schema::ToDuckDBType` in
// `backend/schema/schema.cc` so the storage layer's column
// definitions and the transpiler's `CAST` expressions agree on the
// DuckDB-side names.
absl::string_view DuckDBSqlTypeName(::googlesql::TypeKind kind);

// Renders the *full* DuckDB type expression for `type`, including
// the element type on `ARRAY<T>` (`T[]`) and the ordered field list
// on `STRUCT<...>` (`STRUCT(name1 T1, name2 T2)`). STRUCT field
// names are double-quote escaped so identifiers with hyphens /
// unicode round-trip safely through a DuckDB `CREATE TABLE` /
// `CAST` expression.
//
// Safe to call on any non-null `Type*` the analyzer can produce.
// Unsupported kinds fall through to the head name returned by
// `DuckDBSqlTypeName`; the transpiler treats those as "best
// effort" — the runtime DuckDB error is what surfaces if the
// mapping turns out to be wrong for a given query.
std::string ToDuckDBSqlType(const ::googlesql::Type& type);

}  // namespace transpiler
}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_DUCKDB_TRANSPILER_TYPES_H_
