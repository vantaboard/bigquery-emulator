#include "backend/engine/control/stubs/create_model.h"

#include "absl/status/status.h"
#include "absl/strings/str_cat.h"
#include "googlesql/resolved_ast/resolved_ast.h"
#include "googlesql/resolved_ast/resolved_node_kind.pb.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace control {
namespace stubs {

absl::Status RunCreateModel(const ::googlesql::ResolvedStatement& stmt) {
  if (stmt.node_kind() != ::googlesql::RESOLVED_CREATE_MODEL_STMT) {
    return absl::InternalError(absl::StrCat(
        "control stubs: RunCreateModel coordinator dispatched the wrong "
        "statement kind (expected ResolvedCreateModelStmt; got ",
        stmt.node_kind_string(),
        ")"));
  }
  const auto* model = stmt.GetAs<::googlesql::ResolvedCreateModelStmt>();
  // Mark every field accessed so the analyzer's per-node field-
  // access tracker is satisfied. The stub deliberately does NOT
  // persist any of these to storage -- this is the metadata-only
  // contract from `googlesqlite-15-specialized-stubs.plan.md` -- but
  // the analyzer demands every field on a resolved node be
  // accessed at least once or the AST owner reports an unchecked-
  // fields error when it is destroyed. Marking is cheaper than
  // walking every accessor by hand and stays correct as the
  // GoogleSQL upstream adds new fields to ResolvedCreateModelStmt.
  model->MarkFieldsAccessed();
  // OK. A real BigQuery `CREATE MODEL` would register the model
  // under `projects/<proj>/datasets/<ds>/models/<id>`; the stub
  // skips that. A downstream `ML.PREDICT(MODEL <id>, ...)` reaches
  // the unsupported stub executor (because `ml.predict` is a
  // `functions.yaml` `unsupported` row) and surfaces UNIMPLEMENTED
  // with the ML.* family-named, link-bearing error message --
  // which is the half of the local-stub posture that prevents
  // silent approximation downstream.
  return absl::OkStatus();
}

}  // namespace stubs
}  // namespace control
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
