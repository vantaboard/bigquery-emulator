#include "backend/engine/coordinator/stub_executors.h"

#include <memory>
#include <string>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "backend/engine/coordinator/route_classifier.h"
#include "backend/engine/disposition.h"
#include "backend/engine/engine.h"
#include "googlesql/resolved_ast/resolved_ast.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace coordinator {

namespace {

// Build the route-specific UNIMPLEMENTED message. Centralized so
// each executor renders an identical envelope; the conformance
// routing matrix that lands in `conformance-routing-matrix.plan.md`
// will assert on the route name and we keep that surface stable
// here. For `unsupported` routes the message also names the
// offending family (e.g. `function:keys.encrypt`,
// `ResolvedCreateModelStmt`) so the operator can map the
// failure back to the exact `functions.yaml` /
// `node_dispositions.yaml` row that owns the posture. The link to
// `docs/ENGINE_POLICY.md` is part of the contract from
// `specialized-feature-policy.plan.md`: a user who hits the
// envelope can read the policy document for the family-by-family
// posture table without first having to find the plan file. The
// plan file remains the source of truth (the policy doc links
// back); both pointers are emitted so a grep on either lands the
// reader in the right place.
std::string MakeUnimplementedMessage(absl::string_view route,
                                     absl::string_view operation,
                                     absl::string_view stmt_kind,
                                     absl::string_view offending_family,
                                     absl::string_view plan_pointer) {
  std::string family_segment;
  if (!offending_family.empty() && offending_family != stmt_kind) {
    family_segment = absl::StrCat(", family: ", offending_family);
  }
  return absl::StrCat("local coordinator: ",
                      operation,
                      " on route '",
                      route,
                      "' is not implemented (statement kind: ",
                      stmt_kind,
                      family_segment,
                      "); see docs/ENGINE_POLICY.md and plan ",
                      plan_pointer);
}

// Re-classify `stmt` to pull the offending-node string out of the
// `RouteDecision`. The classifier is stateless / cheap (single AST
// walk) and the executor only fires on the unhappy path, so the
// extra classify is dwarfed by the error-handling cost on the
// caller. We do NOT thread the original `RouteDecision` down from
// `LocalCoordinatorEngine::RouteFor` because doing so would force
// every executor to take the decision (or a side-channel) -- the
// re-classify keeps the executor signature stable and matches
// what the route classifier already promises the coordinator.
std::string OffendingFamilyFor(const ::googlesql::ResolvedStatement& stmt) {
  RouteClassifier classifier;
  RouteDecision decision = classifier.Classify(stmt);
  // Only the `kUnsupported` route is expected to call this helper;
  // the others would surface a different envelope. Defense in
  // depth: returning the offending node even on a non-unsupported
  // decision is harmless (it just produces a slightly noisier
  // message), but `kDuckdbNative` would emit an empty offending
  // node and the message would then drop the family segment. That
  // matches the existing behavior for "statement is unsupported
  // by class, not by a particular function inside it".
  (void)decision;
  return decision.offending_node;
}

}  // namespace

// `SemanticExecutor` graduated out of this stub file when
// `semantic-executor-core.plan.md` landed. It now lives at
// `backend/engine/semantic/executor.{h,cc}` with the real
// scalar-only SELECT / expression evaluator. The coordinator's
// `semantic_executor_` member references the new package; this
// file is intentionally silent on `kSemanticExecutor`.
//
// `ControlOpExecutor` graduated out of this stub file when
// `control-op-executor.plan.md` landed. It now lives at
// `backend/engine/control/control_op_executor.{h,cc}` with real
// per-statement handlers (CREATE TABLE / CTAS / DROP TABLE /
// ANALYZE) plus a focused-UNIMPLEMENTED dispatch table for the rest
// of the control-op surface. The coordinator's
// `control_op_executor_` member references the new package; this
// file is intentionally silent on `kControlOp`.

// --- UnsupportedExecutor --------------------------------------------------

UnsupportedExecutor::~UnsupportedExecutor() = default;

absl::StatusOr<std::unique_ptr<RowSource>> UnsupportedExecutor::ExecuteQuery(
    const QueryRequest& request,
    const ::googlesql::ResolvedStatement& stmt,
    ::googlesql::Catalog* catalog) {
  (void)request;
  (void)catalog;
  return absl::UnimplementedError(
      MakeUnimplementedMessage("unsupported",
                               "ExecuteQuery",
                               stmt.node_kind_string(),
                               OffendingFamilyFor(stmt),
                               "specialized-feature-policy.plan.md"));
}

absl::StatusOr<DmlStats> UnsupportedExecutor::ExecuteDml(
    const QueryRequest& request,
    const ::googlesql::ResolvedStatement& stmt,
    ::googlesql::Catalog* catalog) {
  (void)request;
  (void)catalog;
  return absl::UnimplementedError(
      MakeUnimplementedMessage("unsupported",
                               "ExecuteDml",
                               stmt.node_kind_string(),
                               OffendingFamilyFor(stmt),
                               "specialized-feature-policy.plan.md"));
}

absl::Status UnsupportedExecutor::ExecuteDdl(
    const QueryRequest& request,
    const ::googlesql::ResolvedStatement& stmt,
    ::googlesql::Catalog* catalog) {
  (void)request;
  (void)catalog;
  return absl::UnimplementedError(
      MakeUnimplementedMessage("unsupported",
                               "ExecuteDdl",
                               stmt.node_kind_string(),
                               OffendingFamilyFor(stmt),
                               "specialized-feature-policy.plan.md"));
}

}  // namespace coordinator
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
