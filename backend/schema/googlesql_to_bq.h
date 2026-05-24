#ifndef BIGQUERY_EMULATOR_BACKEND_SCHEMA_GOOGLESQL_TO_BQ_H_
#define BIGQUERY_EMULATOR_BACKEND_SCHEMA_GOOGLESQL_TO_BQ_H_

// Reflection from GoogleSQL's static type system into BigQuery's
// wire-level `FieldSchema` / `TableSchema` proto shape.
//
// The analyzer (Phase 4) produces a `ResolvedStatement` whose output
// columns each carry a `googlesql::Type*`. The DryRun gRPC RPC and
// the future ExecuteQuery RPC both need to surface that type
// information back to the gateway as a `bigquery_emulator.v1.TableSchema`
// proto so the gateway can emit the corresponding BigQuery REST
// `TableSchema` / `Job.statistics.query.schema` payload.
//
// The mapping is the inverse of the one in
// `backend/catalog/googlesql_catalog.cc::ToGoogleSqlType` and stays
// consistent with `schema::ColumnTypeName` (the engine-agnostic
// BigQuery type-name vocabulary):
//
//   googlesql::TypeKind        BigQuery FieldSchema.type
//   ----------------------     -------------------------
//   TYPE_BOOL                  BOOL
//   TYPE_INT64                 INT64
//   TYPE_DOUBLE                FLOAT64
//   TYPE_STRING                STRING
//   TYPE_BYTES                 BYTES
//   TYPE_DATE                  DATE
//   TYPE_TIME                  TIME
//   TYPE_DATETIME              DATETIME
//   TYPE_TIMESTAMP             TIMESTAMP
//   TYPE_NUMERIC               NUMERIC
//   TYPE_BIGNUMERIC            BIGNUMERIC
//   TYPE_JSON                  JSON
//   TYPE_GEOGRAPHY             GEOGRAPHY
//   TYPE_STRUCT                STRUCT  (recurses into `fields`)
//   TYPE_ARRAY                 <element type> with `mode = REPEATED`
//
// `ARRAY<ARRAY<...>>` is illegal in both BigQuery and GoogleSQL so we
// reject it as `INVALID_ARGUMENT`; any other kind we do not yet model
// (`TYPE_PROTO`, `TYPE_ENUM`, `TYPE_INTERVAL`, `TYPE_RANGE`,
// `TYPE_GRAPH_ELEMENT`, ...) returns `INVALID_ARGUMENT` with the
// offending `Type::DebugString` so analysis errors are at least
// self-describing on the wire.

#include <string>
#include <vector>

#include "absl/status/status.h"
#include "absl/strings/string_view.h"
#include "googlesql/public/type.h"
#include "googlesql/resolved_ast/resolved_ast.h"
#include "proto/emulator.pb.h"

namespace bigquery_emulator {
namespace backend {
namespace schema {

// Fills `*out` with the BigQuery proto `FieldSchema` representation of
// `type`. `name` is copied verbatim onto `out->name` so callers do not
// have to do a second assignment.
//
// Cardinality: scalar types and STRUCTs leave `out->mode` set to its
// proto default (empty / NULLABLE on the wire). ARRAY types unwrap
// the element type and set `out->mode = "REPEATED"`. Callers may
// override the mode after this returns when they have extra info
// (e.g. a column the analyzer marked REQUIRED).
//
// Returns `INVALID_ARGUMENT` if `type` uses a kind this reflector
// does not yet map (PROTO, ENUM, INTERVAL, RANGE, GRAPH_ELEMENT,
// nested ARRAY-of-ARRAY).
absl::Status TypeToFieldSchema(const ::googlesql::Type* type,
                               absl::string_view name,
                               v1::FieldSchema* out);

// Fills `*out` with the table-level proto schema corresponding to a
// `ResolvedQueryStmt`'s `output_column_list`. The output columns map
// 1:1 to top-level BigQuery fields; their names come from the
// resolved-output-column `name()` (BigQuery's alias resolution
// already happened in the analyzer) and their types from the inner
// `column().type()`.
absl::Status OutputColumnListToTableSchema(
    const std::vector<std::unique_ptr<const ::googlesql::ResolvedOutputColumn>>&
        output_columns,
    v1::TableSchema* out);

}  // namespace schema
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_SCHEMA_GOOGLESQL_TO_BQ_H_
