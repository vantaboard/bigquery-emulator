#include "backend/engine/control/pipe_export_data.h"

#include <memory>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "backend/engine/control/control_op_internal.h"
#include "backend/engine/engine.h"
#include "backend/engine/semantic/row_source.h"
#include "backend/schema/schema.h"
#include "backend/storage/storage.h"
#include "googlesql/resolved_ast/resolved_ast.h"
#include "googlesql/resolved_ast/resolved_node_kind.pb.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace control {

absl::StatusOr<std::unique_ptr<RowSource>> RunPipeExportData(
    storage::Storage& storage,
    const QueryRequest& request,
    const ::googlesql::ResolvedStatement& stmt) {
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
  const auto* pipe_scan =
      query->GetAs<::googlesql::ResolvedPipeExportDataScan>();
  const ::googlesql::ResolvedExportDataStmt* export_stmt =
      pipe_scan->export_data_stmt();
  if (export_stmt == nullptr) {
    return absl::InternalError(
        "RunPipeExportData: pipe EXPORT DATA scan has null export_data_stmt");
  }
  absl::Status exported =
      internal::RunExportData(storage, request, export_stmt, stmt);
  if (!exported.ok()) return exported;
  return std::make_unique<semantic::MaterializedRowSource>(
      schema::TableSchema{}, std::vector<storage::Row>{});
}

}  // namespace control
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
