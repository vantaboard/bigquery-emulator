#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_SCRIPT_DECLARE_STMT_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_SCRIPT_DECLARE_STMT_H_

#include "absl/status/status.h"
#include "backend/engine/engine.h"
#include "backend/engine/semantic/script/script_driver.h"

namespace googlesql {
class ResolvedCreateConstantStmt;
}  // namespace googlesql

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace script {

absl::Status ExecuteDeclare(const QueryRequest& request,
                            const ::googlesql::ResolvedCreateConstantStmt& stmt,
                            ScriptDriver& driver);

}  // namespace script
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_SCRIPT_DECLARE_STMT_H_
