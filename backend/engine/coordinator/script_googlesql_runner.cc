

#include "googlesql/public/types/type_factory.h"
#include "googlesql/scripting/script_executor.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace coordinator {

namespace {

::googlesql::ScriptExecutorOptions MakeScriptExecutorOptions(
    catalog::GoogleSqlCatalog* catalog) {
  ::googlesql::ScriptExecutorOptions options;
  options.set_type_factory(catalog->type_factory());
  ::googlesql::AnalyzerOptions analyzer_options =
      MakeCoordinatorAnalyzerOptions(/*all_statements=*/true);
  options.PopulateFromAnalyzerOptions(analyzer_options);
  return options;
}

std::string AugmentScriptWithDriverDeclares(
    absl::string_view tail, const semantic::FrameStack& variables) {
  std::string augmented;
  for (const auto& [name, value] : variables.VisibleBindings()) {
    const absl::string_view type_name =
        semantic::BigQueryTypeName(value.type());
    if (type_name.empty()) continue;
    absl::StrAppend(&augmented, "DECLARE ", name, " ", type_name);
    if (!value.is_null()) {
      absl::StrAppend(&augmented, " DEFAULT ", value.GetSQLLiteral());
    }
    absl::StrAppend(&augmented, ";\n");
  }
  absl::StrAppend(&augmented, tail);
  return augmented;
}

std::string WrapScriptInBeginEnd(absl::string_view sql) {
  const std::string trimmed = std::string(absl::StripAsciiWhitespace(sql));
  if (absl::StartsWithIgnoreCase(trimmed, "BEGIN")) {
    return trimmed;
  }
  if (absl::StartsWithIgnoreCase(trimmed, "EXECUTE IMMEDIATE")) {
    return trimmed;
  }
  return absl::StrCat("BEGIN\n", trimmed, "\nEND;");
}

}  // namespace

absl::StatusOr<std::unique_ptr<RowSource>> ExecuteScriptViaGoogleSql(
    LocalCoordinatorEngine& engine,
    const QueryRequest& request,
    ::googlesql::Catalog* catalog,
    semantic::script::ScriptDriver* driver,
    absl::string_view script_sql) {
  auto* bq_catalog = dynamic_cast<catalog::GoogleSqlCatalog*>(catalog);
  if (bq_catalog == nullptr) {
    return absl::FailedPreconditionError(
        "script::ExecuteScriptViaGoogleSql: catalog must be GoogleSqlCatalog");
  }

  semantic::script::ScriptDriver local_driver;
  semantic::script::ScriptDriver* active_driver =
      driver != nullptr ? driver : &local_driver;
  const absl::string_view tail =
      script_sql.empty() ? absl::string_view(request.sql) : script_sql;
  std::string sql = std::string(tail);
  if (driver != nullptr &&
      !active_driver->variables().VisibleBindings().empty()) {
    sql = AugmentScriptWithDriverDeclares(tail, active_driver->variables());
  }
  sql = WrapScriptInBeginEnd(sql);
  QueryRequest script_request = request;
  script_request.sql = sql;

  absl::flat_hash_set<std::string> registered;
  absl::Status registered_status = RegisterScriptVariablesOnCatalogFromDriver(
      bq_catalog, *active_driver, &registered);
  if (!registered_status.ok()) return registered_status;

  std::unique_ptr<RowSource> final_rows;
  EmulatorStatementEvaluator evaluator(
      &engine, script_request, catalog, active_driver);
  evaluator.SetFinalRowsOut(&final_rows);
  ::googlesql::ScriptExecutorOptions options =
      MakeScriptExecutorOptions(bq_catalog);

  absl::StatusOr<std::unique_ptr<::googlesql::ScriptExecutor>> script_executor =
      ::googlesql::ScriptExecutor::Create(sql, options, &evaluator);
  if (!script_executor.ok()) return script_executor.status();

  while (!(*script_executor)->IsComplete()) {
    absl::Status step = (*script_executor)->ExecuteNext();
    if (!step.ok()) return step;
  }

  for (const auto& [name, value] : (*script_executor)->GetCurrentVariables()) {
    const std::string key = std::string(name.ToString());
    if (active_driver->variables().Has(key)) {
      (void)active_driver->variables().Set(key, value);
    }
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
