#include "backend/engine/coordinator/stub_executors.h"

#include <memory>
#include <string>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "backend/engine/engine.h"
#include "googlesql/resolved_ast/resolved_ast.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace coordinator {

namespace {

// Build the route-specific UNIMPLEMENTED message. Centralized so
// each executor renders an identical envelope (route name +
// offending statement kind + plan pointer); the conformance routing
// matrix that lands in `conformance-routing-matrix.plan.md` will
// assert on the route name and we keep that surface stable here.
std::string MakeUnimplementedMessage(absl::string_view route,
                                     absl::string_view operation,
                                     absl::string_view stmt_kind,
                                     absl::string_view plan_pointer) {
  return absl::StrCat("local coordinator: ",
                      operation,
                      " on route '",
                      route,
                      "' is not implemented (statement kind: ",
                      stmt_kind,
                      "); see plan ",
                      plan_pointer);
}

}  // namespace

// --- SemanticExecutor -----------------------------------------------------

SemanticExecutor::~SemanticExecutor() = default;

absl::StatusOr<std::unique_ptr<RowSource>> SemanticExecutor::ExecuteQuery(
    const QueryRequest& request,
    const ::googlesql::ResolvedStatement& stmt,
    ::googlesql::Catalog* catalog) {
  (void)request;
  (void)catalog;
  return absl::UnimplementedError(MakeUnimplementedMessage(
      "semantic_executor",
      "ExecuteQuery",
      stmt.node_kind_string(),
      "semantic-executor-core.plan.md"));
}

absl::StatusOr<DmlStats> SemanticExecutor::ExecuteDml(
    const QueryRequest& request,
    const ::googlesql::ResolvedStatement& stmt,
    ::googlesql::Catalog* catalog) {
  (void)request;
  (void)catalog;
  return absl::UnimplementedError(MakeUnimplementedMessage(
      "semantic_executor",
      "ExecuteDml",
      stmt.node_kind_string(),
      "semantic-executor-core.plan.md"));
}

absl::Status SemanticExecutor::ExecuteDdl(
    const QueryRequest& request,
    const ::googlesql::ResolvedStatement& stmt,
    ::googlesql::Catalog* catalog) {
  (void)request;
  (void)catalog;
  return absl::UnimplementedError(MakeUnimplementedMessage(
      "semantic_executor",
      "ExecuteDdl",
      stmt.node_kind_string(),
      "semantic-executor-core.plan.md"));
}

// --- ControlOpExecutor ----------------------------------------------------

ControlOpExecutor::~ControlOpExecutor() = default;

absl::StatusOr<std::unique_ptr<RowSource>> ControlOpExecutor::ExecuteQuery(
    const QueryRequest& request,
    const ::googlesql::ResolvedStatement& stmt,
    ::googlesql::Catalog* catalog) {
  (void)request;
  (void)catalog;
  return absl::UnimplementedError(MakeUnimplementedMessage(
      "control_op",
      "ExecuteQuery",
      stmt.node_kind_string(),
      "control-op-executor.plan.md"));
}

absl::StatusOr<DmlStats> ControlOpExecutor::ExecuteDml(
    const QueryRequest& request,
    const ::googlesql::ResolvedStatement& stmt,
    ::googlesql::Catalog* catalog) {
  (void)request;
  (void)catalog;
  return absl::UnimplementedError(MakeUnimplementedMessage(
      "control_op",
      "ExecuteDml",
      stmt.node_kind_string(),
      "control-op-executor.plan.md"));
}

absl::Status ControlOpExecutor::ExecuteDdl(
    const QueryRequest& request,
    const ::googlesql::ResolvedStatement& stmt,
    ::googlesql::Catalog* catalog) {
  (void)request;
  (void)catalog;
  return absl::UnimplementedError(MakeUnimplementedMessage(
      "control_op",
      "ExecuteDdl",
      stmt.node_kind_string(),
      "control-op-executor.plan.md"));
}

// --- UnsupportedExecutor --------------------------------------------------

UnsupportedExecutor::~UnsupportedExecutor() = default;

absl::StatusOr<std::unique_ptr<RowSource>> UnsupportedExecutor::ExecuteQuery(
    const QueryRequest& request,
    const ::googlesql::ResolvedStatement& stmt,
    ::googlesql::Catalog* catalog) {
  (void)request;
  (void)catalog;
  return absl::UnimplementedError(MakeUnimplementedMessage(
      "unsupported",
      "ExecuteQuery",
      stmt.node_kind_string(),
      "specialized-feature-policy.plan.md"));
}

absl::StatusOr<DmlStats> UnsupportedExecutor::ExecuteDml(
    const QueryRequest& request,
    const ::googlesql::ResolvedStatement& stmt,
    ::googlesql::Catalog* catalog) {
  (void)request;
  (void)catalog;
  return absl::UnimplementedError(MakeUnimplementedMessage(
      "unsupported",
      "ExecuteDml",
      stmt.node_kind_string(),
      "specialized-feature-policy.plan.md"));
}

absl::Status UnsupportedExecutor::ExecuteDdl(
    const QueryRequest& request,
    const ::googlesql::ResolvedStatement& stmt,
    ::googlesql::Catalog* catalog) {
  (void)request;
  (void)catalog;
  return absl::UnimplementedError(MakeUnimplementedMessage(
      "unsupported",
      "ExecuteDdl",
      stmt.node_kind_string(),
      "specialized-feature-policy.plan.md"));
}

}  // namespace coordinator
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
