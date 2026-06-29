#include "backend/engine/coordinator/script_execute_immediate.h"

#include <memory>
#include <string>

#include "backend/catalog/googlesql_catalog.h"
#include "backend/engine/coordinator/local_coordinator_analyze.h"
#include "backend/engine/coordinator/script_executor_internal.h"
#include "backend/engine/semantic/error.h"
#include "backend/engine/semantic/eval_expr.h"
#include "backend/engine/semantic/row_source.h"
#include "backend/engine/semantic/value.h"
#include "googlesql/resolved_ast/resolved_ast.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace coordinator {

namespace {

std::string SqlLiteralToString(const ::googlesql::ResolvedExpr& expr) {
  semantic::EvalContext ctx;
  absl::StatusOr<semantic::Value> value = semantic::EvalExpr(expr, ctx);
  if (!value.ok() || value->is_null()) {
    return "";
  }
  if (value->type_kind() == ::googlesql::TYPE_STRING) {
    return value->string_value();
  }
  return value->DebugString();
}

}  // namespace

absl::Status ExecuteExecuteImmediate(
    LocalCoordinatorEngine& engine,
    const QueryRequest& request,
    const ::googlesql::ResolvedExecuteImmediateStmt& stmt,
    ::googlesql::Catalog* catalog,
    semantic::script::ScriptDriver& driver) {
  if (stmt.sql() == nullptr) {
    return absl::InvalidArgumentError(
        "script::ExecuteExecuteImmediate: null sql expression");
  }
  const std::string dynamic_sql = SqlLiteralToString(*stmt.sql());
  if (dynamic_sql.empty()) {
    return semantic::MakeSemanticError(
        semantic::SemanticErrorReason::kInvalidArgument,
        "script::ExecuteExecuteImmediate: dynamic SQL must be a non-null "
        "STRING literal");
  }

  QueryRequest dynamic_request = request;
  dynamic_request.sql = dynamic_sql;

  auto* bq_catalog = dynamic_cast<catalog::GoogleSqlCatalog*>(catalog);
  if (bq_catalog == nullptr) {
    return absl::FailedPreconditionError(
        "script::ExecuteExecuteImmediate: catalog must be GoogleSqlCatalog");
  }
  absl::flat_hash_set<std::string> registered;
  absl::Status registered_status = RegisterScriptVariablesOnCatalogFromDriver(
      bq_catalog, driver, &registered);
  if (!registered_status.ok()) return registered_status;

  absl::StatusOr<std::unique_ptr<const ::googlesql::AnalyzerOutput>> output =
      AnalyzeStatementImpl(dynamic_request, catalog, /*all_statements=*/true);
  if (!output.ok()) return output.status();
  const ::googlesql::ResolvedStatement* resolved = nullptr;
  resolved = (*output)->resolved_statement();
  if (resolved == nullptr) {
    return absl::InternalError(
        "script::ExecuteExecuteImmediate: analyzer returned null statement");
  }

  std::unique_ptr<RowSource> ignored;
  return ExecuteOneScriptStatement(
      engine, dynamic_request, *resolved, catalog, driver, &ignored);
}

}  // namespace coordinator
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
