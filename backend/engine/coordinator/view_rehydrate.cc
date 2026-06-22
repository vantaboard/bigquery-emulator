#include "backend/engine/coordinator/view_rehydrate.h"

#include <memory>
#include <utility>
#include <vector>

#include "absl/status/status.h"
#include "backend/catalog/googlesql_catalog.h"
#include "backend/catalog/udf_registration_catalog.h"
#include "backend/catalog/udf_registry.h"
#include "backend/catalog/view_registry.h"
#include "backend/engine/coordinator/local_coordinator_analyze.h"
#include "backend/engine/engine.h"
#include "googlesql/public/analyzer_output.h"
#include "googlesql/public/language_options.h"
#include "googlesql/resolved_ast/resolved_ast.h"
#include "googlesql/resolved_ast/resolved_node_kind.pb.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace coordinator {

namespace {

absl::Status AnalyzeAndRegisterViewDdl(storage::Storage* storage,
                                       const storage::ViewRecord& rec) {
  engine::QueryRequest request;
  request.project_id = rec.id.project_id;
  request.default_dataset_id = rec.id.dataset_id;
  request.sql = rec.ddl_sql;

  ::googlesql::TypeFactory* reg_tf =
      catalog::EnsureProjectTypeFactory(rec.id.project_id);
  const ::googlesql::LanguageOptions language =
      catalog::MakeCatalogLanguageOptions();
  catalog::GoogleSqlCatalog* reg_catalog =
      catalog::GetOrCreateRegistrationCatalog(
          rec.id.project_id, storage, reg_tf, language, rec.id.dataset_id);
  if (reg_catalog == nullptr) {
    return absl::InternalError(
        "view_rehydrate: registration catalog unavailable");
  }

  absl::StatusOr<std::unique_ptr<const ::googlesql::AnalyzerOutput>>
      reg_output = AnalyzeStatementImpl(request,
                                        reg_catalog,
                                        /*all_statements=*/true);
  if (!reg_output.ok()) return reg_output.status();
  const ::googlesql::ResolvedStatement* reg_stmt =
      (*reg_output)->resolved_statement();
  if (reg_stmt == nullptr) {
    return absl::InternalError(
        "view_rehydrate: analyzer returned null statement");
  }
  if (reg_stmt->node_kind() != ::googlesql::RESOLVED_CREATE_VIEW_STMT) {
    return absl::InvalidArgumentError(
        "view_rehydrate: stored DDL is not CREATE VIEW");
  }
  const auto* create_view =
      reg_stmt->GetAs<::googlesql::ResolvedCreateViewStmt>();
  if (create_view == nullptr) {
    return absl::InternalError(
        "view_rehydrate: CREATE VIEW has null resolved stmt");
  }
  return catalog::RegisterProjectView(
      rec.id.project_id, *create_view, std::move(*reg_output), reg_tf);
}

}  // namespace

absl::Status RehydrateViewRecord(storage::Storage* storage,
                                 const storage::ViewRecord& record) {
  return AnalyzeAndRegisterViewDdl(storage, record);
}

absl::Status RehydrateViewsFromStorage(storage::Storage* storage) {
  if (storage == nullptr) return absl::OkStatus();
  absl::StatusOr<std::vector<storage::ViewRecord>> all_or =
      storage->ListAllViews();
  if (!all_or.ok()) return all_or.status();
  for (const storage::ViewRecord& rec : *all_or) {
    absl::Status reg = AnalyzeAndRegisterViewDdl(storage, rec);
    if (!reg.ok()) return reg;
  }
  return absl::OkStatus();
}

}  // namespace coordinator
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
