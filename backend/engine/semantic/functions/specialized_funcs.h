#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_FUNCTIONS_SPECIALIZED_FUNCS_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_FUNCTIONS_SPECIALIZED_FUNCS_H_

#include <optional>
#include <vector>

#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"
#include "backend/engine/semantic/value.h"
#include "googlesql/public/type.h"
#include "googlesql/resolved_ast/resolved_ast.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace functions {

std::optional<absl::StatusOr<Value>> DispatchSpecializedScalar(
    absl::string_view name,
    const std::vector<Value>& args,
    const ::googlesql::Type* return_type);

absl::StatusOr<Value> EvalAggregateCall(
    const ::googlesql::ResolvedAggregateFunctionCall& call,
    const std::vector<std::vector<Value>>& input_column_values);

}  // namespace functions
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_FUNCTIONS_SPECIALIZED_FUNCS_H_
