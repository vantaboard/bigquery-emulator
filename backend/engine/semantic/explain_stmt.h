#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_EXPLAIN_STMT_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_EXPLAIN_STMT_H_

#include <memory>

#include "absl/status/statusor.h"
#include "backend/engine/engine.h"

namespace googlesql {
class ResolvedExplainStmt;
}  // namespace googlesql

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {

// Returns a single-row plan describing the route disposition chosen for
// the inner statement (`ResolvedExplainStmt::statement()`).
[[nodiscard]] absl::StatusOr<std::unique_ptr<RowSource>> ExecuteExplainStmt(
    const ::googlesql::ResolvedExplainStmt& explain_stmt);

}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_EXPLAIN_STMT_H_
