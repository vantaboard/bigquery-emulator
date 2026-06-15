#include "backend/engine/coordinator/script_statement_evaluator.h"

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "absl/container/flat_hash_set.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "backend/catalog/googlesql_catalog.h"
#include "backend/catalog/procedure_registry.h"
#include "backend/catalog/stored_procedure.h"
#include "backend/engine/coordinator/local_coordinator_analyze.h"
#include "backend/engine/coordinator/local_coordinator_engine.h"
#include "backend/engine/coordinator/script_executor_internal.h"
#include "backend/engine/coordinator/script_row_iterator.h"
#include "backend/engine/semantic/error.h"
#include "backend/engine/semantic/eval_expr.h"
#include "backend/engine/semantic/expression_column_bindings.h"
#include "backend/engine/semantic/row_source.h"
#include "backend/engine/semantic/system_variables.h"
#include "backend/engine/semantic/value.h"
#include "googlesql/proto/script_exception.pb.h"
#include "googlesql/public/analyzer.h"
#include "googlesql/public/types/type_factory.h"
#include "googlesql/scripting/error_helpers.h"
#include "googlesql/scripting/script_executor.h"
#include "googlesql/scripting/script_segment.h"
#include "googlesql/scripting/type_aliases.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace coordinator {

namespace {

absl::Status AttachScriptExceptionIfCatchable(absl::Status status) {
  if (status.ok()) return status;
  const std::string script_exception_type_url =
      absl::StrCat("type.googleapis.com/",
                   ::googlesql::ScriptException::descriptor()->full_name());
  if (status.GetPayload(script_exception_type_url).has_value()) {
    return status;
  }
  const semantic::SemanticErrorReason reason =
      semantic::GetSemanticErrorReason(status);
  if (reason == semantic::SemanticErrorReason::kNotImplemented ||
      status.code() == absl::StatusCode::kInternal) {
    return status;
  }
  return static_cast<absl::Status>(::googlesql::MakeScriptException()
                                   << status.message());
}

absl::Status SyncDriverFromExecutor(const ::googlesql::ScriptExecutor& executor,
                                    semantic::script::ScriptDriver& driver) {
  for (const auto& [name, value] : executor.GetCurrentVariables()) {
    const std::string key = std::string(name.ToString());
    if (driver.variables().Has(key)) {
      absl::Status set = driver.variables().Set(key, value);
      if (!set.ok()) return set;
    } else {
      absl::Status declared = driver.variables().Declare(key, value);
      if (!declared.ok()) return declared;
    }
  }
  return absl::OkStatus();
}

absl::StatusOr<std::unique_ptr<MaterializedEvaluatorTableIterator>>
RowSourceToEvaluatorIterator(RowSource& source,
                             ::googlesql::TypeFactory* type_factory) {
  auto drained = semantic::DrainRowSource(source);
  if (!drained.ok()) return drained.status();
  const schema::TableSchema& schema = (*drained)->schema();
  std::vector<std::string> names;
  std::vector<const ::googlesql::Type*> types;
  names.reserve(schema.columns.size());
  types.reserve(schema.columns.size());
  for (const schema::ColumnSchema& col : schema.columns) {
    names.push_back(col.name);
    auto gs_type =
        catalog::GoogleSqlCatalog::ToGoogleSqlType(col, type_factory);
    if (!gs_type.ok()) return gs_type.status();
    types.push_back(*gs_type);
  }
  std::vector<std::vector<::googlesql::Value>> rows;
  storage::Row row;
  while (true) {
    absl::StatusOr<bool> has = (*drained)->Next(&row);
    if (!has.ok()) return has.status();
    if (!*has) break;
    std::vector<::googlesql::Value> converted;
    converted.reserve(row.cells.size());
    for (size_t i = 0; i < row.cells.size(); ++i) {
      auto v = catalog::StorageValueToGoogleSqlValue(row.cells[i], types[i]);
      if (!v.ok()) return v.status();
      converted.push_back(*std::move(v));
    }
    rows.push_back(std::move(converted));
  }
  return std::make_unique<MaterializedEvaluatorTableIterator>(
      std::move(names), std::move(types), std::move(rows));
}

}  // namespace

EmulatorStatementEvaluator::EmulatorStatementEvaluator(
    LocalCoordinatorEngine* engine,
    QueryRequest request,
    ::googlesql::Catalog* catalog,
    semantic::script::ScriptDriver* driver)
    : engine_(engine),
      request_(std::move(request)),
      catalog_(catalog),
      driver_(driver) {}

void EmulatorStatementEvaluator::SetFinalRowsOut(
    std::unique_ptr<RowSource>* final_rows_out) {
  final_rows_out_ = final_rows_out;
}

absl::Status EmulatorStatementEvaluator::ExecuteStatement(
    const ::googlesql::ScriptExecutor& executor,
    const ::googlesql::ScriptSegment& segment) {
  auto* bq_catalog = dynamic_cast<catalog::GoogleSqlCatalog*>(catalog_);
  if (bq_catalog == nullptr) {
    return absl::FailedPreconditionError(
        "EmulatorStatementEvaluator: catalog must be GoogleSqlCatalog");
  }
  absl::Status synced = SyncDriverFromExecutor(executor, *driver_);
  if (!synced.ok()) return synced;

  ::googlesql::AnalyzerOptions options =
      MakeCoordinatorAnalyzerOptions(/*all_statements=*/true);
  absl::Status updated = executor.UpdateAnalyzerOptions(options);
  if (!updated.ok()) return updated;

  absl::flat_hash_set<std::string> registered;
  absl::Status registered_status = RegisterScriptVariablesOnCatalogFromDriver(
      bq_catalog, *driver_, &registered);
  if (!registered_status.ok()) return registered_status;

  QueryRequest stmt_request = request_;
  stmt_request.sql = std::string(segment.GetSegmentText());

  absl::StatusOr<std::unique_ptr<const ::googlesql::AnalyzerOutput>> output =
      AnalyzeStatementImpl(stmt_request, catalog_, /*all_statements=*/true);
  if (!output.ok()) return AttachScriptExceptionIfCatchable(output.status());

  const ::googlesql::ResolvedStatement* resolved =
      (*output)->resolved_statement();
  if (resolved == nullptr) {
    return absl::InternalError(
        "EmulatorStatementEvaluator: analyzer returned null statement");
  }

  std::unique_ptr<RowSource> ignored;
  std::unique_ptr<RowSource>* rows_out =
      final_rows_out_ != nullptr ? final_rows_out_ : &ignored;
  absl::Status executed =
      ExecuteOneScriptStatement(*engine_,
                                stmt_request,
                                *resolved,
                                catalog_,
                                *driver_,
                                rows_out,
                                &executor.GetKnownSystemVariables());
  return AttachScriptExceptionIfCatchable(executed);
}

absl::StatusOr<std::unique_ptr<::googlesql::EvaluatorTableIterator>>
EmulatorStatementEvaluator::ExecuteQueryWithResult(
    const ::googlesql::ScriptExecutor& executor,
    const ::googlesql::ScriptSegment& segment) {
  auto* bq_catalog = dynamic_cast<catalog::GoogleSqlCatalog*>(catalog_);
  if (bq_catalog == nullptr) {
    return absl::FailedPreconditionError(
        "EmulatorStatementEvaluator: catalog must be GoogleSqlCatalog");
  }
  absl::Status synced = SyncDriverFromExecutor(executor, *driver_);
  if (!synced.ok()) return synced;

  ::googlesql::AnalyzerOptions options =
      MakeCoordinatorAnalyzerOptions(/*all_statements=*/true);
  absl::Status updated = executor.UpdateAnalyzerOptions(options);
  if (!updated.ok()) return updated;

  absl::flat_hash_set<std::string> registered;
  absl::Status registered_status = RegisterScriptVariablesOnCatalogFromDriver(
      bq_catalog, *driver_, &registered);
  if (!registered_status.ok()) return registered_status;

  QueryRequest stmt_request = request_;
  stmt_request.sql = std::string(segment.GetSegmentText());

  absl::StatusOr<std::unique_ptr<const ::googlesql::AnalyzerOutput>> output =
      AnalyzeStatementImpl(stmt_request, catalog_, /*all_statements=*/true);
  if (!output.ok()) return AttachScriptExceptionIfCatchable(output.status());

  const ::googlesql::ResolvedStatement* resolved =
      (*output)->resolved_statement();
  if (resolved == nullptr) {
    return absl::InternalError(
        "EmulatorStatementEvaluator: analyzer returned null statement");
  }

  std::unique_ptr<RowSource> rows;
  std::unique_ptr<RowSource>* rows_out =
      final_rows_out_ != nullptr ? final_rows_out_ : &rows;
  absl::Status executed =
      ExecuteOneScriptStatement(*engine_,
                                stmt_request,
                                *resolved,
                                catalog_,
                                *driver_,
                                rows_out,
                                &executor.GetKnownSystemVariables());
  if (!executed.ok()) {
    return AttachScriptExceptionIfCatchable(executed);
  }
  RowSource* source = rows_out->get();
  if (source == nullptr) {
    return std::make_unique<MaterializedEvaluatorTableIterator>(
        std::vector<std::string>{},
        std::vector<const ::googlesql::Type*>{},
        std::vector<std::vector<::googlesql::Value>>{});
  }
  return RowSourceToEvaluatorIterator(*source, bq_catalog->type_factory());
}

absl::Status EmulatorStatementEvaluator::SerializeIterator(
    const ::googlesql::EvaluatorTableIterator& iterator,
    google::protobuf::Any& out) {
  (void)iterator;
  (void)out;
  return absl::UnimplementedError(
      "EmulatorStatementEvaluator::SerializeIterator is not supported");
}

absl::StatusOr<std::unique_ptr<::googlesql::EvaluatorTableIterator>>
EmulatorStatementEvaluator::DeserializeToIterator(
    const google::protobuf::Any& msg,
    const ::googlesql::ScriptExecutor& executor,
    const ::googlesql::ParsedScript& parsed_script) {
  (void)msg;
  (void)executor;
  (void)parsed_script;
  return absl::UnimplementedError(
      "EmulatorStatementEvaluator::DeserializeToIterator is not supported");
}

absl::StatusOr<int> EmulatorStatementEvaluator::EvaluateCaseExpression(
    const ::googlesql::ScriptSegment& case_value,
    const std::vector<::googlesql::ScriptSegment>& when_values,
    const ::googlesql::ScriptExecutor& executor) {
  absl::Status synced = SyncDriverFromExecutor(executor, *driver_);
  if (!synced.ok()) return synced;
  semantic::EvalContext ctx;
  ctx.project_id = request_.project_id;
  ctx.script_variables = &driver_->variables();
  ctx.arguments = &driver_->variables();
  ctx.script_system_variables = &executor.GetKnownSystemVariables();
  absl::StatusOr<semantic::Value> case_val =
      EvalScalarSegment(case_value, ctx, /*target_type=*/nullptr);
  if (!case_val.ok()) return case_val.status();
  for (size_t i = 0; i < when_values.size(); ++i) {
    absl::StatusOr<semantic::Value> when_val =
        EvalScalarSegment(when_values[i], ctx, /*target_type=*/nullptr);
    if (!when_val.ok()) return when_val.status();
    if (*case_val == *when_val) {
      return static_cast<int>(i);
    }
  }
  return -1;
}

absl::StatusOr<int64_t> EmulatorStatementEvaluator::GetIteratorMemoryUsage(
    const ::googlesql::EvaluatorTableIterator& iterator) {
  (void)iterator;
  return 0;
}

absl::StatusOr<::googlesql::Value>
EmulatorStatementEvaluator::EvaluateScalarExpression(
    const ::googlesql::ScriptExecutor& executor,
    const ::googlesql::ScriptSegment& segment,
    const ::googlesql::Type* target_type) {
  absl::Status synced = SyncDriverFromExecutor(executor, *driver_);
  if (!synced.ok()) return synced;
  semantic::EvalContext ctx;
  ctx.project_id = request_.project_id;
  ctx.script_variables = &driver_->variables();
  ctx.arguments = &driver_->variables();
  ctx.script_system_variables = &executor.GetKnownSystemVariables();
  return EvalScalarSegment(segment, ctx, target_type);
}

absl::StatusOr<::googlesql::TypeWithParameters>
EmulatorStatementEvaluator::ResolveTypeName(
    const ::googlesql::ScriptExecutor& executor,
    const ::googlesql::ScriptSegment& segment) {
  (void)executor;
  auto* bq_catalog = dynamic_cast<catalog::GoogleSqlCatalog*>(catalog_);
  if (bq_catalog == nullptr) {
    return absl::FailedPreconditionError(
        "EmulatorStatementEvaluator: catalog must be GoogleSqlCatalog");
  }
  ::googlesql::AnalyzerOptions options =
      MakeCoordinatorAnalyzerOptions(/*all_statements=*/true);
  const std::string type_name = std::string(segment.GetSegmentText());
  const ::googlesql::Type* type = nullptr;
  ::googlesql::TypeModifiers type_modifiers;
  absl::Status analyzed = ::googlesql::AnalyzeType(type_name,
                                                   options,
                                                   catalog_,
                                                   bq_catalog->type_factory(),
                                                   &type,
                                                   &type_modifiers);
  if (!analyzed.ok()) return analyzed;
  if (type == nullptr) {
    return absl::InternalError(
        "EmulatorStatementEvaluator::ResolveTypeName: null resolved type");
  }
  ::googlesql::TypeWithParameters result;
  result.type = type;
  result.type_params = type_modifiers.type_parameters();
  return result;
}

bool EmulatorStatementEvaluator::IsSupportedVariableType(
    const ::googlesql::TypeWithParameters& type_with_params) {
  (void)type_with_params;
  return true;
}

absl::Status EmulatorStatementEvaluator::ApplyTypeParameterConstraints(
    const ::googlesql::TypeParameters& type_params, ::googlesql::Value* value) {
  (void)type_params;
  (void)value;
  return absl::OkStatus();
}

absl::StatusOr<std::unique_ptr<::googlesql::ProcedureDefinition>>
EmulatorStatementEvaluator::LoadProcedure(
    const ::googlesql::ScriptExecutor& executor,
    const absl::Span<const std::string>& path,
    int64_t num_arguments) {
  (void)executor;
  (void)num_arguments;
  if (path.empty()) {
    return absl::NotFoundError("procedure path is empty");
  }
  const std::string name = path.back();
  const catalog::StoredSQLProcedure* proc =
      catalog::FindProjectProcedure(request_.project_id, name);
  if (proc == nullptr) {
    return absl::NotFoundError(
        absl::StrCat("procedure '", name, "' not found"));
  }
  return std::make_unique<::googlesql::ProcedureDefinition>(
      proc->Name(),
      proc->signature(),
      proc->argument_name_list(),
      proc->procedure_body());
}

absl::Status EmulatorStatementEvaluator::AssignSystemVariable(
    ::googlesql::ScriptExecutor* executor,
    const ::googlesql::ASTSystemVariableAssignment* ast_assignment,
    const ::googlesql::Value& value) {
  if (executor == nullptr || ast_assignment == nullptr) {
    return absl::InvalidArgumentError(
        "EmulatorStatementEvaluator::AssignSystemVariable: null argument");
  }
  absl::StatusOr<bool> assigned =
      executor->DefaultAssignSystemVariable(ast_assignment, value);
  if (!assigned.ok()) return assigned.status();
  if (*assigned) return absl::OkStatus();

  if (ast_assignment->system_variable() == nullptr ||
      ast_assignment->system_variable()->path() == nullptr) {
    return absl::InvalidArgumentError(
        "EmulatorStatementEvaluator::AssignSystemVariable: null system "
        "variable");
  }
  const std::vector<std::string> path =
      ast_assignment->system_variable()->path()->ToIdentifierVector();
  return semantic::SetSystemVariable(request_.project_id, path, value);
}

absl::StatusOr<::googlesql::Value>
EmulatorStatementEvaluator::EvalScalarSegment(
    const ::googlesql::ScriptSegment& segment,
    semantic::EvalContext& ctx,
    const ::googlesql::Type* target_type) {
  auto* bq_catalog = dynamic_cast<catalog::GoogleSqlCatalog*>(catalog_);
  if (bq_catalog == nullptr) {
    return absl::FailedPreconditionError(
        "EmulatorStatementEvaluator: catalog must be GoogleSqlCatalog");
  }
  ::googlesql::AnalyzerOptions options =
      MakeCoordinatorAnalyzerOptions(/*all_statements=*/false);
  absl::Status registered =
      semantic::RegisterExpressionColumnsOnAnalyzerOptions(driver_->variables(),
                                                           options);
  if (!registered.ok()) return registered;

  std::unique_ptr<const ::googlesql::AnalyzerOutput> output;
  absl::Status analyzed =
      ::googlesql::AnalyzeExpression(segment.GetSegmentText(),
                                     options,
                                     catalog_,
                                     bq_catalog->type_factory(),
                                     &output);
  if (!analyzed.ok()) return analyzed;
  if (output == nullptr || output->resolved_expr() == nullptr) {
    return absl::InternalError(
        "EmulatorStatementEvaluator::EvalScalarSegment: null resolved expr");
  }
  absl::flat_hash_map<std::string, ::googlesql::Value> columns_by_name;
  semantic::PopulateEvalContextExpressionColumns(
      driver_->variables(), ctx, &columns_by_name);
  absl::StatusOr<semantic::Value> value =
      semantic::EvalExpr(*output->resolved_expr(), ctx);
  if (!value.ok()) return value.status();
  (void)target_type;
  return *value;
}

}  // namespace coordinator
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
