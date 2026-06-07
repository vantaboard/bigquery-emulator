#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_EVAL_TVF_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_EVAL_TVF_H_

#include <vector>

#include "absl/status/statusor.h"
#include "backend/engine/semantic/eval_context.h"
#include "googlesql/resolved_ast/resolved_ast.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {

absl::StatusOr<std::vector<ColumnBindings>> MaterializeTvfScan(
    const ::googlesql::ResolvedTVFScan& scan, EvalContext& ctx);

}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_EVAL_TVF_H_
