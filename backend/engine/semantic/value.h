#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_VALUE_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_VALUE_H_

// `semantic::Value` is the unit of evaluation inside the local
// semantic executor.
//
// Design constraint #3 from
// `.cursor/plans/semantic-executor-core.plan.md`: "Same Arrow output
// shape as the DuckDB fast path -- so the Storage Read API and REST
// `f`/`v` marshaler don't branch on route". Rather than invent a
// parallel type system the rest of the engine cannot consume, we
// reuse `googlesql::Value` (the same value type the analyzer hands
// us inside every `ResolvedLiteral`). That gives us:
//
//   * A type tag (`Value::type()` / `Value::type_kind()`) covering
//     every BigQuery primitive (BOOL, INT64, FLOAT64, NUMERIC,
//     BIGNUMERIC, STRING, BYTES, DATE, TIME, DATETIME, TIMESTAMP,
//     JSON, INTERVAL, UUID, ARRAY, STRUCT).
//   * Explicit NULL kinds (`Value::NullInt64()`, ...) so the
//     evaluator never has to thread `std::optional<Value>` through
//     every operator.
//   * In-memory layouts for ARRAY (`std::vector<Value>`) and STRUCT
//     (`std::vector<Value>` plus the named-field schema on the
//     `StructType`) that already match what `arrow_to_bq.cc` and the
//     gateway's `f`/`v` marshaler accept once we convert to
//     `storage::Value`.
//
// The helpers in this header are the thin layer that lets the rest
// of the package treat `googlesql::Value` like a first-class
// in-engine value and convert back to the wire-facing `storage::Value`
// without leaking analyzer types into the gateway path.

#include <cstdint>
#include <string>

#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"
#include "backend/schema/schema.h"
#include "backend/storage/storage.h"
#include "googlesql/public/type.h"
#include "googlesql/public/type.pb.h"
#include "googlesql/public/value.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {

// Alias so the package speaks `Value` without every caller having to
// fully qualify `::googlesql::Value`. The underlying type IS
// `googlesql::Value`; this is purely a readability convenience.
using Value = ::googlesql::Value;

// Render the BigQuery type-name spelling for a `googlesql::Type*`
// (e.g. `TYPE_INT64` -> `"INT64"`). Mirrors the spellings used in
// `backend/schema/schema.h::ColumnTypeName`. Returns "" for an
// unrecognized type kind so the caller can decide whether to error.
absl::string_view BigQueryTypeName(const ::googlesql::Type* type);

// Convert a `googlesql::Value` to the engine-agnostic
// `storage::Value` shape `arrow_to_bq.cc` produces. Each BigQuery
// primitive lowers onto its wire-friendly string / numeric form
// (DATE -> "YYYY-MM-DD", TIMESTAMP -> "YYYY-MM-DD HH:MM:SS.ffffff",
// NUMERIC / BIGNUMERIC -> decimal text, JSON -> normalized JSON
// string, INTERVAL / UUID -> textual form). ARRAY recurses into a
// `storage::Value::Array`; STRUCT recurses into a
// `storage::Value::Struct`. NULL values lower onto
// `storage::Value::Null()`.
//
// Returns INVALID_ARGUMENT on an unsupported type kind so callers
// can surface a clean failure instead of silently mis-rendering a
// cell.
absl::StatusOr<storage::Value> ToStorageValue(const Value& value);

// Build the BigQuery-shaped `ColumnSchema` corresponding to one
// output column of a SELECT projection. `name` is copied onto the
// returned schema verbatim; the rest of the fields come from `type`.
//
// ARRAY types lower onto a NULLABLE column whose `mode == kRepeated`
// (the BigQuery REPEATED mode contract). STRUCT types recurse into
// `fields`. Errors propagate from `schema::TypeToFieldSchema`-style
// rejections (PROTO / ENUM / RANGE / nested ARRAY-of-ARRAY) so the
// gateway sees the same INVALID_ARGUMENT it gets from any other
// engine route.
absl::StatusOr<schema::ColumnSchema> ColumnSchemaForType(
    const ::googlesql::Type* type, absl::string_view name);

// Parse the JSON-encoded literal value the gateway forwards on
// `QueryRequest::parameters` into a `googlesql::Value` of the kind
// `type_kind` names (e.g. "INT64" -> `Value::Int64(42)`). The
// parsing rules mirror BigQuery's REST `QueryParameterValue` shape:
// the JSON is either a bare string carrying the literal (the
// dominant case for INT64 / NUMERIC / STRING / DATE / TIMESTAMP)
// or a primitive JSON value (BOOL true / false; FLOAT64 number;
// signed INT64 number). NULL parameter values surface as the
// matching `NullX()` factory.
//
// `type_kind_name` accepts both the BigQuery REST spelling
// ("INT64") and the GoogleSQL spelling ("TYPE_INT64") so the
// gateway / engine plumbing does not have to canonicalize twice.
//
// Returns INVALID_ARGUMENT when the type kind is unknown or the
// JSON body cannot be parsed against the requested kind. ARRAY and
// STRUCT parameter types are deferred to a downstream plan; calling
// with them returns NOT_IMPLEMENTED so the caller can surface a
// clean envelope.
absl::StatusOr<Value> ParseParameterValue(absl::string_view value_json,
                                          absl::string_view type_kind_name);

// Map a BigQuery / GoogleSQL type-kind spelling onto
// `::googlesql::TypeKind`. Accepts both "INT64" and "TYPE_INT64";
// returns `TYPE_UNKNOWN` on an unrecognized name.
::googlesql::TypeKind ParseTypeKindName(absl::string_view type_kind_name);

}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_VALUE_H_
