#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_EXPRESSION_COLUMN_BINDINGS_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_EXPRESSION_COLUMN_BINDINGS_H_

#include "absl/container/flat_hash_map.h"
#include "absl/status/status.h"
#include "backend/engine/semantic/eval_context.h"
#include "backend/engine/semantic/frame_stack.h"
#include "googlesql/public/analyzer_options.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {

// Register each visible script / invocation binding as an analyzer
// expression column so `AnalyzeExpression` resolves names to
// `ResolvedExpressionColumn` rather than catalog constants.
absl::Status RegisterExpressionColumnsOnAnalyzerOptions(
    const FrameStack& variables, ::googlesql::AnalyzerOptions& options);

// Populate `ctx.columns_by_name` from the visible frame bindings.
// Returns a map owned by `storage` that remains valid for the eval
// call when pointed to by `ctx.columns_by_name`.
void PopulateEvalContextExpressionColumns(
    const FrameStack& variables,
    EvalContext& ctx,
    absl::flat_hash_map<std::string, ::googlesql::Value>* storage);

}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_EXPRESSION_COLUMN_BINDINGS_H_
