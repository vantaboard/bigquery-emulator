#include "backend/engine/semantic/script/call_stmt.h"

#include "backend/engine/semantic/error.h"
#include "googlesql/resolved_ast/resolved_ast.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace script {

absl::Status ExecuteCall(const QueryRequest& request,
                         const ::googlesql::ResolvedCallStmt& stmt,
                         ScriptDriver& driver) {
  (void)request;
  (void)stmt;
  (void)driver;
  return MakeSemanticError(
      SemanticErrorReason::kNotImplemented,
      "semantic: CALL statement execution is not yet implemented");
}

}  // namespace script
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
