#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_EVAL_UDAF_INTERNAL_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_EVAL_UDAF_INTERNAL_H_

#include <cstddef>
#include <string>
#include <utility>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "backend/engine/semantic/eval_udaf.h"
#include "backend/engine/semantic/frame_stack.h"
#include "backend/engine/semantic/value.h"
#include "googlesql/public/function.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace udaf_internal {

absl::Status DeclareOuterNonAggregateArgs(
    FrameStack& outer_args,
    const std::vector<std::string>& arg_names,
    const std::vector<bool>& is_agg,
    const std::vector<std::vector<Value>>& arg_columns,
    const std::vector<size_t>& row_indices);

absl::StatusOr<std::pair<ColumnBindings, absl::flat_hash_map<std::string, Value>>>
EvalListedUdafAggregates(
    const ::googlesql::SQLFunction& sql_fn,
    const UdafEvalScope& udaf,
    FrameStack& outer_args,
    const EvalContext& ctx);

}  // namespace udaf_internal
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_EVAL_UDAF_INTERNAL_H_
