#include "backend/engine/coordinator/local_coordinator_engine.h"

#include <memory>
#include <utility>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/match.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/strip.h"
#include "absl/time/clock.h"
#include "absl/time/time.h"
#include "backend/catalog/create_function_util.h"
#include "backend/catalog/googlesql_catalog.h"
#include "backend/catalog/js_udf_registry.h"
#include "backend/catalog/procedure_registry.h"
#include "backend/catalog/python_udf_registry.h"
#include "backend/catalog/routine_persistence.h"
#include "backend/catalog/tvf_registry.h"
#include "backend/catalog/udf_registration_catalog.h"
#include "backend/catalog/udf_registry.h"
#include "backend/catalog/view_persistence.h"
#include "backend/catalog/view_registry.h"
#include "backend/engine/control/control_op_internal.h"
#include "backend/engine/control/stubs/create_model.h"
#include "backend/engine/coordinator/executor.h"
#include "backend/engine/coordinator/local_coordinator_analyze.h"
#include "backend/engine/coordinator/route_classifier.h"
#include "backend/engine/coordinator/script_executor.h"
#include "backend/engine/disposition.h"
#include "backend/engine/engine.h"
#include "backend/engine/semantic/script/script_driver.h"
#include "googlesql/public/analyzer_output.h"
#include "googlesql/public/catalog.h"
#include "googlesql/public/language_options.h"
#include "googlesql/public/options.pb.h"
#include "googlesql/public/types/type_factory.h"
#include "googlesql/resolved_ast/resolved_ast.h"
#include "googlesql/resolved_ast/resolved_node_kind.pb.h"
#include "proto/emulator.pb.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace coordinator {

LocalCoordinatorEngine::LocalCoordinatorEngine(storage::Storage* storage)
    : storage_(storage),
      duckdb_executor_(storage),
      semantic_executor_(storage),
      control_op_executor_(storage) {}

LocalCoordinatorEngine::~LocalCoordinatorEngine() = default;

namespace {

bool ContainsSetKeyword(absl::string_view sql) {
  return absl::StrContains(sql, " SET ") || absl::StrContains(sql, "\nSET ") ||
         absl::StartsWithIgnoreCase(absl::StripAsciiWhitespace(sql), "SET ");
}

bool NeedsAllStatements(const QueryRequest& request) {
  const absl::string_view sql = absl::StripAsciiWhitespace(request.sql);
  return absl::StrContains(request.sql, "DECLARE") ||
         absl::StrContains(request.sql, "CALL ") ||
         absl::StrContains(request.sql, "BEGIN") ||
         absl::StrContains(request.sql, "CREATE CONSTANT") ||
         absl::StartsWithIgnoreCase(sql, "CREATE FUNCTION") ||
         absl::StartsWithIgnoreCase(sql, "CREATE OR REPLACE FUNCTION") ||
         absl::StartsWithIgnoreCase(sql, "CREATE SNAPSHOT") ||
         absl::StartsWithIgnoreCase(sql, "CREATE TABLE") ||
         ContainsSetKeyword(request.sql);
}

}  // namespace

Executor* LocalCoordinatorEngine::RouteFor(
    const ::googlesql::ResolvedStatement& stmt) {
  RouteDecision decision = classifier_.Classify(stmt);
  switch (decision.disposition) {
    case Disposition::kDuckdbNative:
    case Disposition::kDuckdbRewrite:
    case Disposition::kDuckdbUdf:
      return &duckdb_executor_;
    case Disposition::kSemanticExecutor:
      return &semantic_executor_;
    case Disposition::kLocalStub:
      return &semantic_executor_;
    case Disposition::kControlOp:
      return &control_op_executor_;
    case Disposition::kUnsupported:
      return &unsupported_executor_;
  }
  return nullptr;
}

absl::StatusOr<std::unique_ptr<AnalyzedQuery>> LocalCoordinatorEngine::Analyze(
    const QueryRequest& request, ::googlesql::Catalog* catalog) {
  return AnalyzeSelectQuery(request, catalog);
}

absl::StatusOr<DryRunResult> LocalCoordinatorEngine::DryRun(
    const QueryRequest& request, ::googlesql::Catalog* catalog) {
  absl::StatusOr<std::unique_ptr<AnalyzedQuery>> analyzed =
      Analyze(request, catalog);
  if (!analyzed.ok()) return analyzed.status();
  DryRunResult result;
  result.schema = (*analyzed)->output_schema();
  result.estimated_bytes_processed = 0;
  return result;
}

absl::StatusOr<std::unique_ptr<RowSource>> LocalCoordinatorEngine::ExecuteQuery(
    const QueryRequest& request, ::googlesql::Catalog* catalog) {
  absl::Status valid = ValidateRequest(request, catalog);
  if (!valid.ok()) return valid;
  const bool all_statements = NeedsAllStatements(request);
  if (all_statements && absl::StrContains(request.sql, ";")) {
    return ExecuteScriptViaAnalyzeNext(*this, request, catalog);
  }
  const absl::Time analyze_start = absl::Now();
  absl::StatusOr<std::unique_ptr<const ::googlesql::AnalyzerOutput>> output =
      AnalyzeStatementImpl(request, catalog, all_statements);
  if (request.phase_recorder != nullptr) {
    request.phase_recorder->Record(
        "analyze_coordinator",
        absl::ToInt64Microseconds(absl::Now() - analyze_start));
  }
  if (!output.ok()) return output.status();
  const ::googlesql::ResolvedStatement* stmt = (*output)->resolved_statement();
  if (stmt->node_kind() == ::googlesql::RESOLVED_MULTI_STMT) {
    return ExecuteMultiStmtScript(
        *this,
        request,
        *stmt->GetAs<::googlesql::ResolvedMultiStmt>(),
        catalog);
  }
  const absl::Time route_start = absl::Now();
  Executor* executor = RouteFor(*stmt);
  if (request.phase_recorder != nullptr) {
    request.phase_recorder->Record(
        "route_classify_coordinator",
        absl::ToInt64Microseconds(absl::Now() - route_start));
  }
  if (executor == nullptr) {
    return absl::InternalError(
        "LocalCoordinatorEngine::ExecuteQuery: classifier returned an "
        "unknown disposition");
  }
  return executor->ExecuteQuery(request, *stmt, catalog);
}

absl::StatusOr<std::unique_ptr<RowSource>>
LocalCoordinatorEngine::ExecuteResolvedStatement(
    const QueryRequest& request,
    const ::googlesql::ResolvedStatement& stmt,
    ::googlesql::Catalog* catalog,
    const semantic::FrameStack* script_variables,
    const ::googlesql::SystemVariableValuesMap* script_system_variables) {
  Executor* executor = RouteFor(stmt);
  if (executor == nullptr) {
    return absl::InternalError(
        "LocalCoordinatorEngine::ExecuteResolvedStatement: classifier "
        "returned an unknown disposition");
  }
  if (executor == &semantic_executor_ && script_variables != nullptr &&
      stmt.node_kind() == ::googlesql::RESOLVED_QUERY_STMT) {
    return semantic::ExecuteResolvedQueryStmt(
        request,
        *stmt.GetAs<::googlesql::ResolvedQueryStmt>(),
        script_variables,
        script_system_variables);
  }
  return executor->ExecuteQuery(request, stmt, catalog);
}

absl::StatusOr<DmlResult> LocalCoordinatorEngine::ExecuteDml(
    const QueryRequest& request, ::googlesql::Catalog* catalog) {
  absl::Status valid = ValidateRequest(request, catalog);
  if (!valid.ok()) return valid;
  absl::StatusOr<std::unique_ptr<const ::googlesql::AnalyzerOutput>> output =
      AnalyzeStatementImpl(request, catalog, /*all_statements=*/true);
  if (!output.ok()) return output.status();
  const ::googlesql::ResolvedStatement* stmt = (*output)->resolved_statement();
  if (stmt->node_kind() == ::googlesql::RESOLVED_TRUNCATE_STMT) {
    absl::StatusOr<int64_t> deleted = control::internal::RunTruncateTable(
        *storage_, stmt->GetAs<::googlesql::ResolvedTruncateStmt>());
    if (!deleted.ok()) return deleted.status();
    DmlResult result;
    result.stats.deleted_row_count = *deleted;
    return result;
  }
  Executor* executor = RouteFor(*stmt);
  if (executor == nullptr) {
    return absl::InternalError(
        "LocalCoordinatorEngine::ExecuteDml: classifier returned an "
        "unknown disposition");
  }
  return executor->ExecuteDml(request, *stmt, catalog);
}

absl::Status LocalCoordinatorEngine::ExecuteDdl(const QueryRequest& request,
                                                ::googlesql::Catalog* catalog) {
  absl::Status valid = ValidateRequest(request, catalog);
  if (!valid.ok()) return valid;
  absl::StatusOr<std::unique_ptr<const ::googlesql::AnalyzerOutput>> output =
      AnalyzeStatementImpl(request, catalog, /*all_statements=*/true);
  if (!output.ok()) return output.status();
  const ::googlesql::ResolvedStatement* stmt = (*output)->resolved_statement();
  if (stmt->node_kind() == ::googlesql::RESOLVED_CREATE_MODEL_STMT) {
    return control::stubs::RunCreateModel(*stmt);
  }
  if (stmt->node_kind() == ::googlesql::RESOLVED_CREATE_FUNCTION_STMT) {
    auto* bq_catalog = dynamic_cast<catalog::GoogleSqlCatalog*>(catalog);
    if (bq_catalog == nullptr) {
      return absl::FailedPreconditionError(
          "LocalCoordinatorEngine::ExecuteDdl: CREATE FUNCTION requires "
          "GoogleSqlCatalog");
    }
    ::googlesql::TypeFactory* reg_tf =
        catalog::EnsureProjectTypeFactory(request.project_id);
    const ::googlesql::LanguageOptions language =
        catalog::MakeCatalogLanguageOptions();
    catalog::GoogleSqlCatalog* reg_catalog =
        catalog::GetOrCreateRegistrationCatalog(request.project_id,
                                                bq_catalog->storage(),
                                                reg_tf,
                                                language,
                                                request.default_dataset_id);
    if (reg_catalog == nullptr) {
      return absl::InternalError(
          "LocalCoordinatorEngine::ExecuteDdl: CREATE FUNCTION registration "
          "catalog unavailable");
    }
    absl::StatusOr<std::unique_ptr<const ::googlesql::AnalyzerOutput>>
        reg_output =
            AnalyzeStatementImpl(request, reg_catalog, /*all_statements=*/true);
    if (!reg_output.ok()) return reg_output.status();
    const ::googlesql::ResolvedStatement* reg_stmt =
        (*reg_output)->resolved_statement();
    const auto* create_fn =
        reg_stmt->GetAs<::googlesql::ResolvedCreateFunctionStmt>();
    if (create_fn == nullptr) {
      return absl::InternalError(
          "LocalCoordinatorEngine::ExecuteDdl: CREATE FUNCTION has null "
          "resolved stmt");
    }
    absl::StatusOr<std::unique_ptr<const ::googlesql::Function>> fn_or =
        catalog::MakeFunctionFromCreateFunction(*create_fn,
                                                /*function_options=*/nullptr);
    if (!fn_or.ok()) return fn_or.status();
    const bool is_temp = create_fn->create_scope() ==
                         ::googlesql::ResolvedCreateStatementEnums::CREATE_TEMP;
    const storage::RoutineId routine_id = catalog::RoutineIdFromNamePath(
        create_fn->name_path(), request.project_id, request.default_dataset_id);
    absl::Status registered =
        catalog::RegisterProjectFunction(request.project_id,
                                         routine_id.dataset_id,
                                         is_temp,
                                         std::move(*reg_output),
                                         std::move(*fn_or));
    if (!registered.ok()) return registered;
    absl::Status js_registered = catalog::RegisterJsUdfFromCreateFunction(
        request.project_id, *create_fn);
    if (!js_registered.ok()) return js_registered;
    absl::Status py_registered = catalog::RegisterPythonUdfFromCreateFunction(
        request.project_id, *create_fn);
    if (!py_registered.ok()) return py_registered;
    return catalog::PersistRoutineDdl(
        bq_catalog->storage(), request, *reg_stmt);
  }
  if (stmt->node_kind() == ::googlesql::RESOLVED_CREATE_VIEW_STMT ||
      stmt->node_kind() == ::googlesql::RESOLVED_CREATE_TABLE_FUNCTION_STMT ||
      stmt->node_kind() == ::googlesql::RESOLVED_CREATE_PROCEDURE_STMT) {
    auto* bq_catalog = dynamic_cast<catalog::GoogleSqlCatalog*>(catalog);
    if (bq_catalog == nullptr) {
      return absl::FailedPreconditionError(
          "LocalCoordinatorEngine::ExecuteDdl: view/TVF DDL requires "
          "GoogleSqlCatalog");
    }
    ::googlesql::TypeFactory* reg_tf =
        catalog::EnsureProjectTypeFactory(request.project_id);
    const ::googlesql::LanguageOptions language =
        catalog::MakeCatalogLanguageOptions();
    catalog::GoogleSqlCatalog* reg_catalog =
        catalog::GetOrCreateRegistrationCatalog(request.project_id,
                                                bq_catalog->storage(),
                                                reg_tf,
                                                language,
                                                request.default_dataset_id);
    if (reg_catalog == nullptr) {
      return absl::InternalError(
          "LocalCoordinatorEngine::ExecuteDdl: registration catalog "
          "unavailable");
    }
    absl::StatusOr<std::unique_ptr<const ::googlesql::AnalyzerOutput>>
        reg_output =
            AnalyzeStatementImpl(request, reg_catalog, /*all_statements=*/true);
    if (!reg_output.ok()) return reg_output.status();
    const ::googlesql::ResolvedStatement* reg_stmt =
        (*reg_output)->resolved_statement();
    if (stmt->node_kind() == ::googlesql::RESOLVED_CREATE_VIEW_STMT) {
      const auto* create_view =
          reg_stmt->GetAs<::googlesql::ResolvedCreateViewStmt>();
      if (create_view == nullptr) {
        return absl::InternalError(
            "LocalCoordinatorEngine::ExecuteDdl: CREATE VIEW has null "
            "resolved stmt");
      }
      absl::Status registered =
          catalog::RegisterProjectView(request.project_id,
                                       request.default_dataset_id,
                                       *create_view,
                                       std::move(*reg_output),
                                       reg_tf);
      if (!registered.ok()) return registered;
      return catalog::PersistViewDdl(
          bq_catalog->storage(), request, *create_view);
    }
    if (stmt->node_kind() == ::googlesql::RESOLVED_CREATE_TABLE_FUNCTION_STMT) {
      const auto* create_tvf =
          reg_stmt->GetAs<::googlesql::ResolvedCreateTableFunctionStmt>();
      if (create_tvf == nullptr) {
        return absl::InternalError(
            "LocalCoordinatorEngine::ExecuteDdl: CREATE TABLE FUNCTION has "
            "null "
            "resolved stmt");
      }
      absl::Status registered = catalog::RegisterProjectTvf(
          request.project_id, *create_tvf, std::move(*reg_output));
      if (!registered.ok()) return registered;
      return catalog::PersistRoutineDdl(
          bq_catalog->storage(), request, *reg_stmt);
    }
    const auto* create_proc =
        reg_stmt->GetAs<::googlesql::ResolvedCreateProcedureStmt>();
    if (create_proc == nullptr) {
      return absl::InternalError(
          "LocalCoordinatorEngine::ExecuteDdl: CREATE PROCEDURE has null "
          "resolved stmt");
    }
    absl::Status registered = catalog::RegisterProjectProcedure(
        request.project_id, *create_proc, std::move(*reg_output));
    if (!registered.ok()) return registered;
    return catalog::PersistRoutineDdl(
        bq_catalog->storage(), request, *reg_stmt);
  }
  if (stmt->node_kind() == ::googlesql::RESOLVED_CALL_STMT) {
    semantic::script::ScriptDriver driver;
    return ExecuteCallStmt(*this,
                           request,
                           *stmt->GetAs<::googlesql::ResolvedCallStmt>(),
                           driver,
                           catalog);
  }
  Executor* executor = RouteFor(*stmt);
  if (executor == nullptr) {
    return absl::InternalError(
        "LocalCoordinatorEngine::ExecuteDdl: classifier returned an "
        "unknown disposition");
  }
  return executor->ExecuteDdl(request, *stmt, catalog);
}

}  // namespace coordinator
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
