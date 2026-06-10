#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_COORDINATOR_SCRIPT_EXECUTE_IMMEDIATE_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_COORDINATOR_SCRIPT_EXECUTE_IMMEDIATE_H_

#include "absl/status/status.h"
#include "backend/engine/engine.h"
#include "backend/engine/semantic/script/script_driver.h"
#include "googlesql/public/catalog.h"
#include "googlesql/resolved_ast/resolved_ast.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace coordinator {

class LocalCoordinatorEngine;

absl::Status ExecuteExecuteImmediate(
    LocalCoordinatorEngine& engine,
    const QueryRequest& request,
    const ::googlesql::ResolvedExecuteImmediateStmt& stmt,
    ::googlesql::Catalog* catalog,
    semantic::script::ScriptDriver& driver);

}  // namespace coordinator
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_COORDINATOR_SCRIPT_EXECUTE_IMMEDIATE_H_
