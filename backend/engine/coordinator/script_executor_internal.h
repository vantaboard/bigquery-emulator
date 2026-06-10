#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_COORDINATOR_SCRIPT_EXECUTOR_INTERNAL_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_COORDINATOR_SCRIPT_EXECUTOR_INTERNAL_H_

#include "absl/container/flat_hash_set.h"
#include "absl/status/status.h"
#include "absl/strings/string_view.h"
#include "backend/catalog/googlesql_catalog.h"
#include "backend/engine/engine.h"
#include "backend/engine/semantic/row_source.h"
#include "backend/engine/semantic/script/script_driver.h"
#include "googlesql/public/analyzer.h"
#include "googlesql/public/catalog.h"
#include "googlesql/resolved_ast/resolved_ast.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace coordinator {

class LocalCoordinatorEngine;

bool ScriptNeedsGoogleSqlExecutor(absl::string_view sql);

// True when the gateway lowered DECLARE → CREATE CONSTANT before the engine
// round-trip (hybrid AnalyzeNext prefix + ScriptExecutor tail).
bool ScriptUsesCreateConstantLowering(absl::string_view sql);

// First semicolon-delimited statement in sql (respecting single-quoted
// literals). Used so hybrid routing only delegates when the *next* statement
// needs googlesql::ScriptExecutor, not when a later IF/WHILE appears in tail.
std::string LeadingScriptStatement(absl::string_view sql);

std::string StripBeginEnd(absl::string_view sql);

absl::Status ExecuteOneScriptStatement(
    LocalCoordinatorEngine& engine,
    const QueryRequest& request,
    const ::googlesql::ResolvedStatement& stmt,
    ::googlesql::Catalog* catalog,
    semantic::script::ScriptDriver& driver,
    std::unique_ptr<RowSource>* final_rows,
    const ::googlesql::SystemVariableValuesMap* script_system_variables =
        nullptr);

absl::Status RegisterScriptVariablesOnCatalogFromDriver(
    catalog::GoogleSqlCatalog* catalog,
    const semantic::script::ScriptDriver& driver,
    absl::flat_hash_set<std::string>* registered);

}  // namespace coordinator
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_COORDINATOR_SCRIPT_EXECUTOR_INTERNAL_H_
