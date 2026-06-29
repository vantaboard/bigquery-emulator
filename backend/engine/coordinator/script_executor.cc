#include "backend/engine/coordinator/script_executor.h"

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "absl/container/flat_hash_set.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/ascii.h"
#include "absl/strings/match.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/strip.h"
#include "backend/catalog/googlesql_catalog.h"
#include "backend/catalog/procedure_registry.h"
#include "backend/catalog/stored_procedure.h"
#include "backend/engine/coordinator/local_coordinator_analyze.h"
#include "backend/engine/coordinator/local_coordinator_engine.h"
#include "backend/engine/coordinator/script_execute_immediate.h"
#include "backend/engine/coordinator/script_executor_internal.h"
#if defined(BIGQUERY_EMULATOR_HAS_GOOGLESQL_SCRIPTING)
#include "backend/engine/coordinator/script_googlesql_runner.h"
#endif
#include "backend/engine/coordinator/script_executor_set.h"
#include "backend/engine/semantic/error.h"
#include "backend/engine/semantic/eval_expr.h"
#include "backend/engine/semantic/executor.h"
#include "backend/engine/semantic/row_source.h"
#include "backend/engine/semantic/script/assert_stmt.h"
#include "backend/engine/semantic/script/assignment_stmt.h"
#include "backend/engine/semantic/script/declare_stmt.h"
#include "backend/engine/semantic/script/script_driver.h"
#include "backend/engine/semantic/value.h"
#include "backend/schema/schema.h"
#include "googlesql/public/analyzer.h"
#include "googlesql/public/function_signature.h"
#include "googlesql/public/parse_resume_location.h"
#include "googlesql/resolved_ast/resolved_ast.h"
#include "googlesql/resolved_ast/resolved_node_kind.pb.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace coordinator {

namespace {

std::string ScriptVariableNameFromExpr(const ::googlesql::ResolvedExpr& expr) {
  if (expr.node_kind() == ::googlesql::RESOLVED_CONSTANT) {
    const ::googlesql::Constant* constant =
        expr.GetAs<::googlesql::ResolvedConstant>()->constant();
    if (constant != nullptr && !constant->Name().empty()) {
      return std::string(constant->Name());
    }
  }
  return "";
}

std::vector<std::string> SplitSemicolonStatements(absl::string_view sql) {
  std::vector<std::string> out;
  std::string current;
  bool in_quote = false;
  for (size_t i = 0; i < sql.size(); ++i) {
    char c = sql[i];
    if (c == '\'') {
      in_quote = !in_quote;
      current.push_back(c);
      continue;
    }
    if (c == ';' && !in_quote) {
      std::string stmt = std::string(absl::StripAsciiWhitespace(current));
      if (!stmt.empty()) {
        out.push_back(std::move(stmt));
      }
      current.clear();
      continue;
    }
    current.push_back(c);
  }
  std::string tail = std::string(absl::StripAsciiWhitespace(current));
  if (!tail.empty()) {
    out.push_back(std::move(tail));
  }
  return out;
}

semantic::Value NullForType(const ::googlesql::Type* type) {
  if (type == nullptr) return semantic::Value::NullString();
  switch (type->kind()) {
    case ::googlesql::TYPE_INT64:
      return semantic::Value::NullInt64();
    case ::googlesql::TYPE_FLOAT:
      return semantic::Value::NullFloat();
    case ::googlesql::TYPE_DOUBLE:
      return semantic::Value::NullDouble();
    case ::googlesql::TYPE_STRING:
      return semantic::Value::NullString();
    case ::googlesql::TYPE_BOOL:
      return semantic::Value::NullBool();
    case ::googlesql::TYPE_ARRAY:
      return semantic::Value::Null(type);
    case ::googlesql::TYPE_STRUCT:
      return semantic::Value::Null(type);
    default:
      return semantic::Value::NullString();
  }
}

}  // namespace

absl::Status ExecuteOneScriptStatement(
    LocalCoordinatorEngine& engine,
    const QueryRequest& request,
    const ::googlesql::ResolvedStatement& stmt,
    ::googlesql::Catalog* catalog,
    semantic::script::ScriptDriver& driver,
    std::unique_ptr<RowSource>* final_rows,
    const ::googlesql::SystemVariableValuesMap* script_system_variables) {
  switch (stmt.node_kind()) {
    case ::googlesql::RESOLVED_CREATE_CONSTANT_STMT:
      return semantic::script::ExecuteDeclare(
          request,
          *stmt.GetAs<::googlesql::ResolvedCreateConstantStmt>(),
          driver);
    case ::googlesql::RESOLVED_ASSIGNMENT_STMT:
      return semantic::script::ExecuteScriptAssignment(
          request, *stmt.GetAs<::googlesql::ResolvedAssignmentStmt>(), driver);
    case ::googlesql::RESOLVED_ASSERT_STMT:
      return semantic::script::ExecuteAssert(
          request, *stmt.GetAs<::googlesql::ResolvedAssertStmt>(), driver);
    case ::googlesql::RESOLVED_CALL_STMT:
      return ExecuteCallStmt(engine,
                             request,
                             *stmt.GetAs<::googlesql::ResolvedCallStmt>(),
                             driver,
                             catalog);
    case ::googlesql::RESOLVED_QUERY_STMT: {
      absl::StatusOr<std::unique_ptr<RowSource>> rows =
          engine.ExecuteResolvedStatement(request,
                                          stmt,
                                          catalog,
                                          &driver.variables(),
                                          script_system_variables);
      if (!rows.ok()) return rows.status();
      *final_rows = std::move(*rows);
      return absl::OkStatus();
    }
    case ::googlesql::RESOLVED_CREATE_TABLE_STMT:
    case ::googlesql::RESOLVED_CREATE_TABLE_AS_SELECT_STMT:
    case ::googlesql::RESOLVED_CREATE_VIEW_STMT:
    case ::googlesql::RESOLVED_CREATE_PROCEDURE_STMT:
    case ::googlesql::RESOLVED_CREATE_FUNCTION_STMT:
    case ::googlesql::RESOLVED_CREATE_TABLE_FUNCTION_STMT:
      return engine.ExecuteDdl(request, catalog);
    case ::googlesql::RESOLVED_INSERT_STMT:
    case ::googlesql::RESOLVED_UPDATE_STMT:
    case ::googlesql::RESOLVED_DELETE_STMT:
    case ::googlesql::RESOLVED_MERGE_STMT:
    case ::googlesql::RESOLVED_TRUNCATE_STMT: {
      absl::StatusOr<DmlResult> result = engine.ExecuteDml(request, catalog);
      if (!result.ok()) return result.status();
      return absl::OkStatus();
    }
    case ::googlesql::RESOLVED_EXECUTE_IMMEDIATE_STMT:
      return ExecuteExecuteImmediate(
          engine,
          request,
          *stmt.GetAs<::googlesql::ResolvedExecuteImmediateStmt>(),
          catalog,
          driver);
    default:
      return semantic::MakeSemanticError(
          semantic::SemanticErrorReason::kNotImplemented,
          absl::StrCat("script: statement kind ",
                       stmt.node_kind_string(),
                       " is not yet implemented"));
  }
}

absl::Status ExecuteStatementList(
    LocalCoordinatorEngine& engine,
    const QueryRequest& request,
    const ::googlesql::ResolvedMultiStmt& multi_stmt,
    ::googlesql::Catalog* catalog,
    semantic::script::ScriptDriver& driver,
    semantic::FrameStack* procedure_args,
    std::unique_ptr<RowSource>* final_rows) {
  for (int i = 0; i < multi_stmt.statement_list_size(); ++i) {
    const ::googlesql::ResolvedStatement* child = multi_stmt.statement_list(i);
    if (child == nullptr) {
      return absl::InternalError("script: MultiStmt child is null");
    }
    if (child->node_kind() == ::googlesql::RESOLVED_QUERY_STMT &&
        procedure_args != nullptr) {
      // Procedure bodies evaluate expressions against formal args.
      (void)procedure_args;
    }
    absl::Status status = ExecuteOneScriptStatement(
        engine, request, *child, catalog, driver, final_rows);
    if (!status.ok()) return status;
  }
  return absl::OkStatus();
}

absl::Status ExecuteCallStmt(LocalCoordinatorEngine& engine,
                             const QueryRequest& request,
                             const ::googlesql::ResolvedCallStmt& stmt,
                             semantic::script::ScriptDriver& driver,
                             ::googlesql::Catalog* catalog) {
  const ::googlesql::Procedure* procedure = stmt.procedure();
  if (procedure == nullptr) {
    return absl::InvalidArgumentError(
        "script::ExecuteCall: CALL has null procedure");
  }
  const catalog::StoredSQLProcedure* sql_proc =
      dynamic_cast<const catalog::StoredSQLProcedure*>(procedure);
  if (sql_proc == nullptr) {
    sql_proc =
        catalog::FindProjectProcedure(request.project_id, procedure->Name());
  }
  if (sql_proc == nullptr) {
    return semantic::MakeSemanticError(
        semantic::SemanticErrorReason::kInvalidArgument,
        absl::StrCat("script::ExecuteCall: procedure '",
                     procedure->Name(),
                     "' not found"));
  }

  const ::googlesql::FunctionSignature& signature = stmt.signature();
  const std::vector<std::string>& arg_names = sql_proc->argument_name_list();
  if (arg_names.size() != static_cast<size_t>(stmt.argument_list_size())) {
    return absl::InvalidArgumentError(
        "script::ExecuteCall: argument count mismatch");
  }

  semantic::FrameStack proc_args;
  proc_args.PushFrame();
  std::vector<std::pair<std::string, std::string>> out_bindings;

  for (int i = 0; i < stmt.argument_list_size(); ++i) {
    const ::googlesql::ResolvedExpr* arg_expr = stmt.argument_list(i);
    if (arg_expr == nullptr) {
      return absl::InvalidArgumentError(
          "script::ExecuteCall: null argument expression");
    }
    const std::string& formal_name = arg_names[static_cast<size_t>(i)];
    const ::googlesql::FunctionArgumentType& formal_type =
        signature.arguments()[static_cast<size_t>(i)];
    const std::string caller_name = ScriptVariableNameFromExpr(*arg_expr);
    if (!caller_name.empty()) {
      absl::StatusOr<semantic::Value> existing =
          driver.variables().Lookup(caller_name);
      semantic::Value seed = existing.ok() ? *std::move(existing)
                                           : NullForType(formal_type.type());
      absl::Status declared = proc_args.Declare(formal_name, seed);
      if (!declared.ok()) return declared;
      out_bindings.emplace_back(formal_name, caller_name);
      continue;
    }

    semantic::EvalContext ctx;
    ctx.project_id = request.project_id;
    ctx.script_variables = &driver.variables();
    absl::StatusOr<semantic::Value> value = semantic::EvalExpr(*arg_expr, ctx);
    if (!value.ok()) return value.status();
    absl::Status declared = proc_args.Declare(formal_name, *std::move(value));
    if (!declared.ok()) return declared;
  }

  absl::Status body_status = ExecuteProcedureBody(
      engine, request, sql_proc->procedure_body(), proc_args, catalog);
  if (!body_status.ok()) return body_status;

  for (const auto& [formal, caller] : out_bindings) {
    absl::StatusOr<semantic::Value> out_value = proc_args.Lookup(formal);
    if (!out_value.ok()) return out_value.status();
    absl::Status set = driver.variables().Set(caller, *std::move(out_value));
    if (!set.ok()) return set;
  }
  return absl::OkStatus();
}

absl::Status ExecuteProcedureBody(LocalCoordinatorEngine& engine,
                                  const QueryRequest& request,
                                  absl::string_view procedure_body,
                                  semantic::FrameStack& arg_frame,
                                  ::googlesql::Catalog* catalog) {
  auto* bq_catalog = dynamic_cast<catalog::GoogleSqlCatalog*>(catalog);
  if (bq_catalog == nullptr) {
    return absl::FailedPreconditionError(
        "script::ExecuteProcedureBody: catalog must be GoogleSqlCatalog");
  }
  semantic::script::ScriptDriver proc_driver(&arg_frame);
  absl::flat_hash_set<std::string> registered_args;
  absl::Status registered = RegisterScriptVariablesOnCatalogFromDriver(
      bq_catalog, proc_driver, &registered_args);
  if (!registered.ok()) return registered;

  std::vector<std::string> statements =
      SplitSemicolonStatements(StripBeginEnd(procedure_body));
  for (const std::string& stmt_sql : statements) {
    std::string trimmed = std::string(absl::StripAsciiWhitespace(stmt_sql));
    if (trimmed.empty()) continue;
    if (absl::StartsWithIgnoreCase(trimmed, "SET ")) {
      std::string rest = trimmed.substr(4);
      size_t eq = rest.find('=');
      if (eq == std::string::npos) {
        return absl::InvalidArgumentError(
            "script::ExecuteProcedureBody: malformed SET statement");
      }
      std::string target =
          std::string(absl::StripAsciiWhitespace(rest.substr(0, eq)));
      std::string expr =
          std::string(absl::StripAsciiWhitespace(rest.substr(eq + 1)));
      absl::Status set_status = ExecuteProcedureSet(
          request, target, expr, bq_catalog, catalog, proc_driver);
      if (!set_status.ok()) return set_status;
      continue;
    }

    QueryRequest body_request = request;
    body_request.sql = trimmed;
    absl::StatusOr<std::unique_ptr<const ::googlesql::AnalyzerOutput>> output =
        AnalyzeStatementImpl(body_request, catalog, /*all_statements=*/true);
    if (!output.ok()) return output.status();
    const ::googlesql::ResolvedStatement* resolved =
        (*output)->resolved_statement();
    if (resolved == nullptr) {
      return absl::InternalError(
          "script::ExecuteProcedureBody: analyzer returned null statement");
    }
    std::unique_ptr<RowSource> ignored;
    absl::Status status = ExecuteOneScriptStatement(
        engine, body_request, *resolved, catalog, proc_driver, &ignored);
    if (!status.ok()) return status;
  }
  return absl::OkStatus();
}

absl::StatusOr<std::unique_ptr<RowSource>> ExecuteMultiStmtScript(
    LocalCoordinatorEngine& engine,
    const QueryRequest& request,
    const ::googlesql::ResolvedMultiStmt& multi_stmt,
    ::googlesql::Catalog* catalog) {
  semantic::script::ScriptDriver driver;
  std::unique_ptr<RowSource> final_rows;
  absl::Status status = ExecuteStatementList(
      engine, request, multi_stmt, catalog, driver, nullptr, &final_rows);
  if (!status.ok()) return status;
  if (final_rows == nullptr) {
    return std::unique_ptr<RowSource>(
        new semantic::MaterializedRowSource(schema::TableSchema{}, {}));
  }
  return final_rows;
}

absl::StatusOr<std::unique_ptr<RowSource>> ExecuteControlFlowTailIfNeeded(
    LocalCoordinatorEngine& engine,
    const QueryRequest& request,
    ::googlesql::Catalog* catalog,
    semantic::script::ScriptDriver* driver,
    absl::string_view remaining) {
  if (!ScriptNeedsGoogleSqlExecutor(LeadingScriptStatement(remaining))) {
    return std::unique_ptr<RowSource>();
  }
#if defined(BIGQUERY_EMULATOR_HAS_GOOGLESQL_SCRIPTING)
  absl::string_view tail_for_executor = remaining;
  const std::string trimmed_remaining =
      std::string(absl::StripAsciiWhitespace(remaining));
  if (!absl::StartsWithIgnoreCase(trimmed_remaining, "BEGIN")) {
    tail_for_executor = StripBeginEnd(remaining);
  }
  return ExecuteScriptViaGoogleSql(
      engine, request, catalog, driver, tail_for_executor);
#else
  return semantic::MakeSemanticError(
      semantic::SemanticErrorReason::kNotImplemented,
      "script: structured control flow (IF/WHILE/LOOP/FOR/EXCEPTION) requires "
      "googlesql/scripting::ScriptExecutor, which is not linked in the "
      "prebuilt artifact");
#endif
}
absl::Status ExecuteAnalyzeNextLoopStep(
    LocalCoordinatorEngine& engine,
    const QueryRequest& request,
    ::googlesql::Catalog* catalog,
    catalog::GoogleSqlCatalog* bq_catalog,
    ::googlesql::AnalyzerOptions& options,
    ::googlesql::TypeFactory* type_factory,
    ::googlesql::ParseResumeLocation* resume,
    semantic::script::ScriptDriver* driver,
    absl::flat_hash_set<std::string>* registered_script_vars,
    std::unique_ptr<RowSource>* final_rows,
    bool* at_end) {
  const absl::string_view remaining =
      resume->input().substr(resume->byte_position());
  absl::StatusOr<std::unique_ptr<RowSource>> control_flow_rows =
      ExecuteControlFlowTailIfNeeded(
          engine, request, catalog, driver, remaining);
  if (!control_flow_rows.ok()) return control_flow_rows.status();
  if (*control_flow_rows != nullptr) {
    *final_rows = std::move(*control_flow_rows);
    *at_end = true;
    return absl::OkStatus();
  }

  absl::Status registered = RegisterScriptVariablesOnCatalogFromDriver(
      bq_catalog, *driver, registered_script_vars);
  if (!registered.ok()) return registered;
  bool set_handled = false;
  absl::Status set_status = TryExecuteLeadingSetStatement(
      request, *resume, bq_catalog, catalog, *driver, &set_handled);
  if (!set_status.ok()) return set_status;
  if (set_handled) {
    const absl::string_view tail =
        resume->input().substr(resume->byte_position());
    if (absl::StripAsciiWhitespace(tail).empty()) *at_end = true;
    return absl::OkStatus();
  }

  std::unique_ptr<const ::googlesql::AnalyzerOutput> output;
  absl::Status analyzed = ::googlesql::AnalyzeNextStatement(
      resume, options, catalog, type_factory, &output, at_end);
  if (!analyzed.ok()) return analyzed;
  if (output == nullptr || output->resolved_statement() == nullptr) {
    return absl::InternalError(
        "script::ExecuteScriptViaAnalyzeNext: analyzer returned null");
  }

  return ExecuteOneScriptStatement(engine,
                                   request,
                                   *output->resolved_statement(),
                                   catalog,
                                   *driver,
                                   final_rows);
}
absl::StatusOr<std::unique_ptr<RowSource>> ExecuteScriptViaAnalyzeNext(
    LocalCoordinatorEngine& engine,
    const QueryRequest& request,
    ::googlesql::Catalog* catalog) {
  auto* bq_catalog = dynamic_cast<catalog::GoogleSqlCatalog*>(catalog);
  if (bq_catalog == nullptr) {
    return absl::FailedPreconditionError(
        "script::ExecuteScriptViaAnalyzeNext: catalog must be "
        "GoogleSqlCatalog");
  }
  absl::StatusOr<::googlesql::AnalyzerOptions> options =
      BuildAnalyzerOptionsForRequest(
          request, bq_catalog, /*all_statements=*/true);
  if (!options.ok()) return options.status();
  ::googlesql::TypeFactory* type_factory = bq_catalog->type_factory();

#if defined(BIGQUERY_EMULATOR_HAS_GOOGLESQL_SCRIPTING)
  {
    const std::string trimmed_sql =
        std::string(absl::StripAsciiWhitespace(request.sql));
    const std::string inner_script = StripBeginEnd(request.sql);
    if (!ScriptUsesCreateConstantLowering(request.sql) &&
        ScriptNeedsGoogleSqlExecutor(inner_script)) {
      const absl::string_view script_for_googlesql =
          absl::StartsWithIgnoreCase(trimmed_sql, "BEGIN") ? request.sql
                                                           : inner_script;
      return ExecuteScriptViaGoogleSql(
          engine, request, catalog, nullptr, script_for_googlesql);
    }
  }
#endif

  ::googlesql::ParseResumeLocation resume =
      ::googlesql::ParseResumeLocation::FromStringView(request.sql);
  semantic::script::ScriptDriver driver;
  std::unique_ptr<RowSource> final_rows;
  bool at_end = false;
  absl::flat_hash_set<std::string> registered_script_vars;
  while (!at_end) {
    absl::Status step = ExecuteAnalyzeNextLoopStep(engine,
                                                   request,
                                                   catalog,
                                                   bq_catalog,
                                                   *options,
                                                   type_factory,
                                                   &resume,
                                                   &driver,
                                                   &registered_script_vars,
                                                   &final_rows,
                                                   &at_end);
    if (!step.ok()) return step;
  }
  if (final_rows == nullptr) {
    return std::unique_ptr<RowSource>(
        new semantic::MaterializedRowSource(schema::TableSchema{}, {}));
  }
  return final_rows;
}

}  // namespace coordinator
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
