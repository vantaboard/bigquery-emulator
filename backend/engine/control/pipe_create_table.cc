#include "backend/engine/control/pipe_create_table.h"

#include <memory>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "backend/engine/engine.h"
#include "googlesql/resolved_ast/resolved_ast.h"
#include "googlesql/resolved_ast/resolved_node_kind.pb.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace control {

absl::StatusOr<std::unique_ptr<RowSource>> RunPipeCreateTable(
    const QueryRequest& request, const ::googlesql::ResolvedStatement& stmt) {
  (void)request;
  const ::googlesql::ResolvedScan* query = nullptr;
  if (stmt.node_kind() == ::googlesql::RESOLVED_QUERY_STMT) {
    query = stmt.GetAs<::googlesql::ResolvedQueryStmt>()->query();
  } else if (stmt.node_kind() == ::googlesql::RESOLVED_GENERALIZED_QUERY_STMT) {
    query = stmt.GetAs<::googlesql::ResolvedGeneralizedQueryStmt>()->query();
  } else {
    return absl::InternalError(
        absl::StrCat("RunPipeCreateTable: coordinator dispatched the wrong "
                     "statement kind (expected ResolvedQueryStmt or "
                     "ResolvedGeneralizedQueryStmt; got ",
                     stmt.node_kind_string(),
                     ")"));
  }
  if (query == nullptr ||
      query->node_kind() != ::googlesql::RESOLVED_PIPE_CREATE_TABLE_SCAN) {
    return absl::InternalError(absl::StrCat(
        "RunPipeCreateTable: coordinator dispatched the wrong "
        "statement body (expected ResolvedPipeCreateTableScan; got ",
        query == nullptr ? "null" : query->node_kind_string(),
        ")"));
  }
  // Touch the inner DDL pointer so the field-access tracker on the
  // owning `AnalyzerOutput` is satisfied. The actual semantics are
  // deferred (see header / message below).
  const auto* pipe_scan =
      query->GetAs<::googlesql::ResolvedPipeCreateTableScan>();
  (void)pipe_scan->create_table_as_select_stmt();
  return absl::UnimplementedError(
      "control op executor: pipe-form CREATE TABLE is not implemented yet; "
      "needs the transpiler to lower an arbitrary pipe-input scan into a "
      "syntactically-valid SELECT so the inner ResolvedCreateTableAsSelectStmt "
      "(whose `query` field is intentionally null in the pipe form) can be "
      "re-issued through the existing CREATE TABLE AS SELECT handler. "
      "Tracked per docs/ENGINE_POLICY.md: 'pipe-form "
      "CREATE "
      "TABLE adapter'.");
}

}  // namespace control
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
