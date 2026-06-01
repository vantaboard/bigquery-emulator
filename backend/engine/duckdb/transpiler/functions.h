#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_DUCKDB_TRANSPILER_FUNCTIONS_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_DUCKDB_TRANSPILER_FUNCTIONS_H_

// BigQuery -> DuckDB function disposition table.
//
// The canonical list lives in `functions.yaml` next to this header;
// a Bazel `genrule` (see `BUILD.bazel`, `:functions_table_inc`)
// turns the YAML into `functions_table.inc`, which `functions.cc`
// `#include`s inside its
// `absl::flat_hash_map<std::string, FnEntry>` initializer. The
// runtime API is one lookup function -- callers stay decoupled from
// the YAML shape and from whether the table is loaded lazily or at
// static-init time.
//
// Disposition semantics are the canonical six-route vocabulary
// defined in `backend/engine/disposition.h`; this table is the
// per-BigQuery-function half of the disposition registry whose
// per-`ResolvedAST` analog lives in
// `node_dispositions.yaml` / `node_dispositions.h`.
//
// Runtime behavior, given a lookup result `entry`:
//
//   * `kDuckdbNative` / `kDuckdbRewrite` -- the transpiler emits
//     `entry->duckdb_name(<args>)`. The caller renders args via
//     its usual emit recursion; if any arg lowers to `""` the
//     whole call falls back through the engine's empty-string
//     contract.
//   * `kDuckdbUdf` -- planned via
//     `duckdb-polyfill-udf-library.plan.md`. Until that plan
//     lands the transpiler returns `""` so the engine surfaces
//     UNIMPLEMENTED. `entry->planned` is true.
//   * `kSemanticExecutor` -- planned via
//     `semantic-functions-compliance.plan.md`. Until that plan
//     lands the transpiler returns `""` so the engine surfaces
//     UNIMPLEMENTED. `entry->planned` is true.
//   * `kControlOp` -- not used by the function table today but
//     accepted by the schema for forward compatibility.
//   * `kUnsupported` -- the function is intentionally out of scope
//     locally (BigQuery-specific semantics or no DuckDB analog).
//     The transpiler returns `""` and the engine surfaces
//     UNIMPLEMENTED. `entry->plan` always points at
//     `specialized-feature-policy.plan.md`.
//
// `SAFE.<fn>` form is not represented here. The transpiler checks
// `ResolvedFunctionCallBase::error_mode() == SAFE_ERROR_MODE` and
// short-circuits to `""` before consulting this table; SAFE mode
// has no native DuckDB analog yet, so every safe-form call
// surfaces as UNIMPLEMENTED independent of the underlying
// function's disposition.

#include <string>

#include "absl/strings/string_view.h"
#include "backend/engine/disposition.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace duckdb {
namespace transpiler {

struct FnEntry {
  Disposition disposition;
  // DuckDB function name emitted verbatim (no quoting) for
  // `kDuckdbNative` / `kDuckdbRewrite` rows. Empty for the other
  // dispositions (the YAML generator enforces both halves of that
  // contract at build time).
  std::string duckdb_name;
  // Owning `.cursor/plans/*.plan.md` file name, or empty when the
  // row has no specific owning plan. Mandatory for
  // `kUnsupported` (the YAML generator rejects an unsupported row
  // without a plan pointer); points at
  // `specialized-feature-policy.plan.md` by convention.
  std::string plan;
  // True when the owning plan has not yet landed an implementation
  // (the YAML row carried `status=planned`). The engine surfaces
  // UNIMPLEMENTED for these rows; the flag exists so the log
  // message can distinguish "planned, waiting for plan X" from "no
  // disposition row". Only valid for dispositions whose runtime
  // emit does not yet exist (`kDuckdbUdf`, `kSemanticExecutor`,
  // `kControlOp`).
  bool planned;
};

// Returns the disposition for `bq_name` (case-insensitive). Returns
// nullptr when the function is *not* in the table -- callers treat
// that the same as a planned-but-not-implemented entry, so an
// unknown function surfaces UNIMPLEMENTED rather than emitting
// garbage SQL.
const FnEntry* LookupFunction(absl::string_view bq_name);

}  // namespace transpiler
}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_DUCKDB_TRANSPILER_FUNCTIONS_H_
