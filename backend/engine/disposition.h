#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_DISPOSITION_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_DISPOSITION_H_

// Canonical execution-route disposition enum for the local execution
// coordinator.
//
// Every user-visible `ResolvedAST` node kind and every BigQuery
// function the engine sees ends up labeled with exactly one of these
// seven dispositions. The same vocabulary appears in:
//
//   * `backend/engine/duckdb/transpiler/SHAPE_TRACKER.md` (the
//     human-readable per-node disposition table; the markdown is a
//     mirror of the YAML below).
//   * `backend/engine/duckdb/transpiler/node_dispositions.yaml`
//     (machine-readable per-node-kind disposition table; generated
//     into `node_dispositions_table.inc` at Bazel build time).
//   * `backend/engine/duckdb/transpiler/functions.yaml`
//     (machine-readable per-BigQuery-function disposition table;
//     generated into `functions_table.inc` at Bazel build time).
//   * `docs/ENGINE_POLICY.md` and
//     `.cursor/plans/local-execution-roadmap-index.plan.md`.
//
// The values match the route names used in those documents
// one-for-one. Anything outside this enum is by construction a doc /
// generator drift bug — `tools/check_disposition_parity` (see
// `task lint:dispositions`) enforces the SHAPE_TRACKER / YAML
// agreement, and the generated `.inc` files round-trip through this
// enum at build time.
//
// Runtime semantics:
//
//   * `kDuckdbNative`     — lowers to DuckDB SQL whose semantics
//                           already match BigQuery exactly. The
//                           transpiler emits the DuckDB form
//                           directly.
//   * `kDuckdbRewrite`    — lowers to DuckDB SQL via a deliberate
//                           structural rewrite (struct/array shape
//                           rewrites, JSON operator mapping, ...).
//                           Same execution path as `kDuckdbNative`;
//                           the rewrite lives in the transpiler.
//   * `kDuckdbUdf`        — lowers to DuckDB SQL that calls a DuckDB
//                           UDF/macro registered at engine startup.
//                           The UDF body owns the BigQuery-specific
//                           behavior. Not yet emit-able from this
//                           plan; see
//                           `duckdb-polyfill-udf-library.plan.md`.
//   * `kSemanticExecutor` — runs on the local row/value semantic
//                           executor instead of DuckDB SQL
//                           evaluation. DuckDB is still used as the
//                           row source; the executor owns expression
//                           evaluation and error surfaces. Not yet
//                           emit-able from this plan; see
//                           `semantic-executor-core.plan.md`.
//   * `kControlOp`        — DDL / metadata / catalog op routed
//                           through the storage layer. Bypasses
//                           query execution entirely. Not yet
//                           emit-able from this plan; see
//                           `control-op-executor.plan.md`.
//   * `kLocalStub`        — deterministic BigQuery-shaped stub for a
//                           specialized feature family that is
//                           accepted at parse / analyzer time but
//                           whose real semantics are deliberately
//                           NOT implemented locally. Engine evaluates
//                           a small per-family stub that returns a
//                           placeholder value (functions) or an OK
//                           metadata-only acknowledgement
//                           (statements). The route exists so client-
//                           library startup probes (CREATE MODEL
//                           registration, `KEYS.NEW_KEYSET(...)`,
//                           ...) do not fail; downstream calls that
//                           depend on the real BigQuery semantic
//                           surface (e.g. `ML.PREDICT` over a
//                           stub-created model, real AEAD encryption
//                           with a stub keyset) still surface
//                           `UNIMPLEMENTED` from the matching
//                           per-family handler. `specialized-
//                           feature-policy.plan.md` documents the
//                           per-family stub contract.
//   * `kUnsupported`      — deliberately out of scope locally.
//                           Surfaces a BigQuery-shaped
//                           `UNIMPLEMENTED` (or
//                           `INVALID_ARGUMENT` where appropriate).
//                           `specialized-feature-policy.plan.md`
//                           documents the unsupported families.

#include "absl/strings/string_view.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {

enum class Disposition {
  kDuckdbNative,
  kDuckdbRewrite,
  kDuckdbUdf,
  kSemanticExecutor,
  kControlOp,
  kLocalStub,
  kUnsupported,
};

// Number of values in `Disposition`. Generated tables iterate over
// the full set; the parity checker (`tools/check_disposition_parity`)
// and the `node_dispositions_table_test` consult this so adding a
// new disposition fails the build everywhere that has to learn
// about it.
inline constexpr int kDispositionCount = 7;

// Returns the canonical lowercase string spelling used in the YAML
// disposition tables and in `SHAPE_TRACKER.md` (`"duckdb_native"`,
// `"duckdb_rewrite"`, ...). The returned `string_view` references a
// statically-allocated literal; callers do not own it.
absl::string_view DispositionToString(Disposition d);

}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_DISPOSITION_H_
