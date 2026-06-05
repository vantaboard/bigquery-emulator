#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_FUNCTIONS_HLL_FUNCS_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_FUNCTIONS_HLL_FUNCS_H_

#include <optional>
#include <vector>

#include "absl/status/statusor.h"
#include "backend/engine/semantic/value.h"
#include "googlesql/resolved_ast/resolved_ast.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace functions {

absl::StatusOr<Value> HllCountExtractScalar(const std::vector<Value>& args);

absl::StatusOr<Value> HllCountInitValues(const std::vector<Value>& input_values,
                                         std::optional<int64_t> precision);

absl::StatusOr<Value> HllCountInitAggregate(
    const ::googlesql::ResolvedAggregateFunctionCall& call,
    const std::vector<std::vector<Value>>& input_column_values);

absl::StatusOr<Value> HllCountMergeAggregate(
    const std::vector<std::vector<Value>>& input_column_values);

absl::StatusOr<Value> HllCountMergePartialAggregate(
    const std::vector<std::vector<Value>>& input_column_values);

}  // namespace functions
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_FUNCTIONS_HLL_FUNCS_H_
