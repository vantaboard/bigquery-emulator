#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_COORDINATOR_SCRIPT_EXECUTOR_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_COORDINATOR_SCRIPT_EXECUTOR_H_

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "backend/engine/engine.h"
#include "backend/engine/semantic/frame_stack.h"
#include "backend/engine/semantic/row_source.h"
#include "backend/engine/semantic/script/script_driver.h"
#include "googlesql/public/catalog.h"
#include "googlesql/resolved_ast/resolved_ast.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace coordinator {

class LocalCoordinatorEngine;

absl::Status ExecuteCallStmt(
    LocalCoordinatorEngine& engine,
    const QueryRequest& request,
    const ::googlesql::ResolvedCallStmt& stmt,
    semantic::script::ScriptDriver& driver,
    ::googlesql::Catalog* catalog);

// Execute a `ResolvedMultiStmt` script block (DECLARE / SET / CALL /
// SELECT / DML / DDL) and return rows from the final SELECT-shaped
// statement, if any.
absl::StatusOr<std::unique_ptr<RowSource>> ExecuteMultiStmtScript(
    LocalCoordinatorEngine& engine,
    const QueryRequest& request,
    const ::googlesql::ResolvedMultiStmt& multi_stmt,
    ::googlesql::Catalog* catalog);

// Execute a stored SQL procedure body (BEGIN..END string) with `arg_frame`
// bound to the procedure's formal parameters.
absl::Status ExecuteProcedureBody(
    LocalCoordinatorEngine& engine,
    const QueryRequest& request,
    absl::string_view procedure_body,
    semantic::FrameStack& arg_frame,
    ::googlesql::Catalog* catalog);

// Execute semicolon-separated script statements via AnalyzeNextStatement
// (DECLARE is lowered to CREATE CONSTANT by the gateway before this runs).
absl::StatusOr<std::unique_ptr<RowSource>> ExecuteScriptViaAnalyzeNext(
    LocalCoordinatorEngine& engine,
    const QueryRequest& request,
    ::googlesql::Catalog* catalog);

}  // namespace coordinator
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_COORDINATOR_SCRIPT_EXECUTOR_H_
