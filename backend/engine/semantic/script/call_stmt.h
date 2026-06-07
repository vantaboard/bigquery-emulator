#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_SCRIPT_CALL_STMT_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_SCRIPT_CALL_STMT_H_

#include "absl/status/status.h"
#include "backend/engine/engine.h"
#include "backend/engine/semantic/script/script_driver.h"
#include "googlesql/resolved_ast/resolved_ast.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace script {

absl::Status ExecuteCall(const QueryRequest& request,
                         const ::googlesql::ResolvedCallStmt& stmt,
                         ScriptDriver& driver);

}  // namespace script
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_SCRIPT_CALL_STMT_H_
