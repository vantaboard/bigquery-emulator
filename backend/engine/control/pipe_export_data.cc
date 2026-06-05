#include "backend/engine/control/pipe_export_data.h"

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

absl::StatusOr<std::unique_ptr<RowSource>> RunPipeExportData(
    const QueryRequest& request, const ::googlesql::ResolvedStatement& stmt) {
  (void)request;
  const ::googlesql::ResolvedScan* query = nullptr;
  if (stmt.node_kind() == ::googlesql::RESOLVED_QUERY_STMT) {
    query = stmt.GetAs<::googlesql::ResolvedQueryStmt>()->query();
  } else if (stmt.node_kind() == ::googlesql::RESOLVED_GENERALIZED_QUERY_STMT) {
    query = stmt.GetAs<::googlesql::ResolvedGeneralizedQueryStmt>()->query();
  } else {
    return absl::InternalError(
        absl::StrCat("RunPipeExportData: coordinator dispatched the wrong "
                     "statement kind (expected ResolvedQueryStmt or "
                     "ResolvedGeneralizedQueryStmt; got ",
                     stmt.node_kind_string(),
                     ")"));
  }
  if (query == nullptr ||
      query->node_kind() != ::googlesql::RESOLVED_PIPE_EXPORT_DATA_SCAN) {
    return absl::InternalError(absl::StrCat(
        "RunPipeExportData: coordinator dispatched the wrong "
        "statement body (expected ResolvedPipeExportDataScan; got ",
        query == nullptr ? "null" : query->node_kind_string(),
        ")"));
  }
  // Mark the inner statement as accessed so the analyzer's
  // field-access tracker doesn't trip when the analyzer-output
  // owner runs `CheckFieldsAccessed()` against the resolved AST.
  // The actual semantics are still deferred (see header / message
  // below).
  const auto* pipe_scan =
      query->GetAs<::googlesql::ResolvedPipeExportDataScan>();
  (void)pipe_scan->export_data_stmt();
  return absl::UnimplementedError(
      "control op executor: pipe-form EXPORT DATA is not implemented yet; "
      "needs the same Arrow / Parquet / CSV / JSON writers and URI scheme "
      "dispatch surface the statement-form EXPORT DATA needs. Tracked by "
      "local-exec-01-ddl-catalog.plan.md follow-up: 'add EXPORT DATA writer "
      "family'.");
}

}  // namespace control
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
