#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_CONTROL_STUBS_CREATE_MODEL_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_CONTROL_STUBS_CREATE_MODEL_H_

// `CREATE MODEL` statement-level `local_stub` handler.
//
// `specialized-feature-policy.plan.md` pins the posture for the
// BigQuery ML surface (`ML.*` functions, `CREATE MODEL`) to a
// split:
//
//   * `CREATE MODEL` ... -> `local_stub` (here). The statement
//     parses, analyzes, and returns OK without registering a
//     storage-side model entry. A real BigQuery `CREATE MODEL`
//     would train a model and persist it under the
//     `projects/<proj>/datasets/<ds>/models/<id>` namespace; the
//     emulator deliberately does NOT model that surface. Client
//     tools that issue `CREATE MODEL` as a setup step succeed
//     (BigQuery-shaped placeholder).
//
//   * `ML.PREDICT(MODEL `<id>`, ...)` -> `unsupported`. The
//     downstream call site that tries to evaluate the model
//     surfaces UNIMPLEMENTED through the unsupported stub
//     executor. The combination preserves the no-silent-
//     approximation rule (a downstream caller never gets a fake
//     prediction value).
//
// The route classifier promotes `ResolvedCreateModelStmt` to
// `kLocalStub` via `node_dispositions.yaml`. The coordinator's
// `ExecuteDdl` pre-dispatches the statement kind here BEFORE
// handing the statement to a generic executor (no executor in
// `backend/engine/coordinator/local_coordinator_engine.cc::Route
// For` would otherwise know what to do with a CREATE MODEL).
//
// Why a separate translation unit
// -------------------------------
//
// `control_op_executor.cc` is a lint-cap carve-out (see
// `.cursor/plans/advanced-relational-routing.plan.md` "don'ts").
// The local-stub family explicitly lives outside that file so
// future stub handlers (JS UDF metadata-only, when plan 13's
// deferred UDF body storage lands) can extend the surface without
// touching the carve-out.

#include "absl/status/status.h"

namespace googlesql {
class ResolvedStatement;
}  // namespace googlesql

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace control {
namespace stubs {

// Handle `CREATE MODEL`. The caller
// (`LocalCoordinatorEngine::ExecuteDdl`) is responsible for
// verifying that the statement kind is `RESOLVED_CREATE_MODEL_STMT`
// before invoking this function. Returns `absl::OkStatus()` for
// any well-formed `ResolvedCreateModelStmt`; never persists a
// storage-side model entry. NULL-statement defense surfaces
// INTERNAL so a coordinator regression that dispatches the wrong
// shape here fails loudly rather than silently OK'ing nothing.
absl::Status RunCreateModel(const ::googlesql::ResolvedStatement& stmt);

}  // namespace stubs
}  // namespace control
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_CONTROL_STUBS_CREATE_MODEL_H_
