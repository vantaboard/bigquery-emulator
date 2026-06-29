

#include "googlesql/public/analyzer_output.h"
#include "googlesql/public/language_options.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace coordinator {

namespace {

absl::Status RegisterResolvedRoutine(
    const engine::QueryRequest& request,
    const ::googlesql::ResolvedStatement& stmt,
    std::unique_ptr<const ::googlesql::AnalyzerOutput> analyzer_output) {
  switch (stmt.node_kind()) {
    case ::googlesql::RESOLVED_CREATE_FUNCTION_STMT: {
      const auto* create_fn =
          stmt.GetAs<::googlesql::ResolvedCreateFunctionStmt>();
      if (create_fn == nullptr) {
        return absl::InternalError(
            "routine_rehydrate: CREATE FUNCTION has null resolved stmt");
      }
      absl::StatusOr<std::unique_ptr<const ::googlesql::Function>> fn_or =
          catalog::MakeFunctionFromCreateFunction(*create_fn,
                                                  /*function_options=*/nullptr);
      if (!fn_or.ok()) return fn_or.status();
      const bool is_temp =
          create_fn->create_scope() ==
          ::googlesql::ResolvedCreateStatementEnums::CREATE_TEMP;
      const storage::RoutineId routine_id =
          catalog::RoutineIdFromNamePath(create_fn->name_path(),
                                         request.project_id,
                                         request.default_dataset_id);
      absl::Status registered =
          catalog::RegisterProjectFunction(request.project_id,
                                           routine_id.dataset_id,
                                           is_temp,
                                           std::move(analyzer_output),
                                           std::move(*fn_or));
      if (!registered.ok()) return registered;
      absl::Status js_registered = catalog::RegisterJsUdfFromCreateFunction(
          request.project_id, *create_fn);
      if (!js_registered.ok()) return js_registered;
      return catalog::RegisterPythonUdfFromCreateFunction(request.project_id,
                                                          *create_fn);
    }
    case ::googlesql::RESOLVED_CREATE_TABLE_FUNCTION_STMT: {
      const auto* create_tvf =
          stmt.GetAs<::googlesql::ResolvedCreateTableFunctionStmt>();
      if (create_tvf == nullptr) {
        return absl::InternalError(
            "routine_rehydrate: CREATE TABLE FUNCTION has null resolved stmt");
      }
      return catalog::RegisterProjectTvf(
          request.project_id, *create_tvf, std::move(analyzer_output));
    }
    case ::googlesql::RESOLVED_CREATE_PROCEDURE_STMT: {
      const auto* create_proc =
          stmt.GetAs<::googlesql::ResolvedCreateProcedureStmt>();
      if (create_proc == nullptr) {
        return absl::InternalError(
            "routine_rehydrate: CREATE PROCEDURE has null resolved stmt");
      }
      return catalog::RegisterProjectProcedure(
          request.project_id, *create_proc, std::move(analyzer_output));
    }
    default:
      return absl::InvalidArgumentError(
          "routine_rehydrate: unsupported routine statement");
  }
}

absl::Status AnalyzeAndRegisterRoutineDdl(storage::Storage* storage,
                                          const storage::RoutineRecord& rec) {
  if (rec.is_temp) return absl::OkStatus();

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
        "routine_rehydrate: registration catalog unavailable");
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
        "routine_rehydrate: analyzer returned null statement");
  }
  return RegisterResolvedRoutine(request, *reg_stmt, std::move(*reg_output));
}

}  // namespace

absl::Status RehydrateRoutineRecord(storage::Storage* storage,
                                    const storage::RoutineRecord& record) {
  return AnalyzeAndRegisterRoutineDdl(storage, record);
}

absl::Status RehydrateRoutinesFromStorage(storage::Storage* storage) {
  if (storage == nullptr) return absl::OkStatus();
  absl::StatusOr<std::vector<storage::RoutineRecord>> all_or =
      storage->ListAllRoutines();
  if (!all_or.ok()) return all_or.status();
  for (const storage::RoutineRecord& rec : *all_or) {
    if (rec.is_temp) continue;
    absl::Status reg = AnalyzeAndRegisterRoutineDdl(storage, rec);
    if (!reg.ok()) return reg;
  }
  return absl::OkStatus();
}

}  // namespace coordinator
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
