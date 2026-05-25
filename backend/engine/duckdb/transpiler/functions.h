#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_DUCKDB_TRANSPILER_FUNCTIONS_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_DUCKDB_TRANSPILER_FUNCTIONS_H_

// BigQuery -> DuckDB function disposition table.
//
// The canonical list lives in `functions.yaml` next to this header; a
// Bazel `genrule` (see BUILD.bazel) turns the YAML into
// `functions_table.inc`, which `functions.cc` `#include`s inside its
// `absl::flat_hash_map<std::string, FnEntry>` initializer. The runtime
// API is one lookup function -- callers stay decoupled from the YAML
// shape and from whether the table is loaded lazily or at static-init
// time.
//
// Disposition semantics:
//   * `kMap`      — the transpiler emits `<duckdb_name>(<args>)`. The
//                   caller renders args via its usual emit recursion;
//                   if any arg lowers to `""` the whole call falls
//                   back through the engine's empty-string contract.
//   * `kSkiplist` — the function is intentionally out of scope today
//                   (BigQuery-specific semantics or no DuckDB analog).
//                   The transpiler returns `""` from the relevant
//                   emit, the engine surfaces `UNIMPLEMENTED`, and the
//                   `FallbackEngine` wrapper retries on the reference-
//                   impl evaluator.
//   * `kFallback` — the function *could* lower but a dedicated rewrite
//                   pass hasn't landed yet (e.g. interval arithmetic
//                   for DATE_ADD). Same runtime behavior as
//                   `kSkiplist`; the distinction is documentation +
//                   the log message the emitter records, so the
//                   conformance harness can tell deliberate skips
//                   from "not implemented yet" cases.
//
// `SAFE.<fn>` form is not represented here. The transpiler checks
// `ResolvedFunctionCallBase::error_mode() == SAFE_ERROR_MODE` and
// short-circuits to `""` before consulting this table; SAFE mode has
// no native DuckDB analog yet, so every safe-form call falls back
// independent of the underlying function's disposition.

#include <string>

#include "absl/strings/string_view.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace duckdb {
namespace transpiler {

enum class FnKind {
  kMap,       // emit `<duckdb_name>(<args>)`
  kSkiplist,  // BigQuery-specific / no DuckDB analog; trigger fallback
  kFallback,  // lowering deferred; trigger fallback
};

struct FnEntry {
  FnKind kind;
  // For `kMap`, the DuckDB function name (emitted verbatim, no quoting).
  // Empty for `kSkiplist` / `kFallback`.
  std::string duckdb_name;
};

// Returns the disposition for `bq_name` (case-insensitive). Returns
// nullptr when the function is *not* in the table -- callers treat
// that the same as `kFallback` so an unknown function falls back to
// the reference-impl engine rather than emitting garbage SQL.
const FnEntry* LookupFunction(absl::string_view bq_name);

}  // namespace transpiler
}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_DUCKDB_TRANSPILER_FUNCTIONS_H_
