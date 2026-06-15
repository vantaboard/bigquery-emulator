#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_FUNCTIONS_KLL_FUNCS_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_FUNCTIONS_KLL_FUNCS_H_

#include <optional>
#include <vector>

#include "absl/status/statusor.h"
#include "backend/engine/semantic/value.h"
#include "googlesql/public/type.h"
#include "googlesql/resolved_ast/resolved_ast.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace functions {

absl::StatusOr<Value> KllQuantilesInitInt64Values(
    const std::vector<Value>& input_values,
    std::optional<int64_t> precision = std::nullopt,
    const std::vector<Value>* weights = nullptr);

absl::StatusOr<Value> KllQuantilesInitFloat64Values(
    const std::vector<Value>& input_values,
    std::optional<int64_t> precision = std::nullopt,
    const std::vector<Value>* weights = nullptr);

absl::StatusOr<Value> KllQuantilesExtractInt64Scalar(
    const std::vector<Value>& args,
    const ::googlesql::Type* return_type);

absl::StatusOr<Value> KllQuantilesExtractFloat64Scalar(
    const std::vector<Value>& args,
    const ::googlesql::Type* return_type);

absl::StatusOr<Value> KllQuantilesExtractPointInt64Scalar(
    const std::vector<Value>& args);

absl::StatusOr<Value> KllQuantilesExtractPointFloat64Scalar(
    const std::vector<Value>& args);

absl::StatusOr<Value> KllQuantilesInitInt64Aggregate(
    const ::googlesql::ResolvedAggregateFunctionCall& call,
    const std::vector<std::vector<Value>>& input_column_values);

absl::StatusOr<Value> KllQuantilesInitFloat64Aggregate(
    const ::googlesql::ResolvedAggregateFunctionCall& call,
    const std::vector<std::vector<Value>>& input_column_values);

absl::StatusOr<Value> KllQuantilesMergePartialAggregate(
    const std::vector<std::vector<Value>>& input_column_values);

absl::StatusOr<Value> KllQuantilesMergeInt64Aggregate(
    const std::vector<std::vector<Value>>& input_column_values,
    const ::googlesql::Type* return_type);

absl::StatusOr<Value> KllQuantilesMergeFloat64Aggregate(
    const std::vector<std::vector<Value>>& input_column_values,
    const ::googlesql::Type* return_type);

absl::StatusOr<Value> KllQuantilesMergePointInt64Aggregate(
    const std::vector<std::vector<Value>>& input_column_values);

absl::StatusOr<Value> KllQuantilesMergePointFloat64Aggregate(
    const std::vector<std::vector<Value>>& input_column_values);

}  // namespace functions
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_FUNCTIONS_KLL_FUNCS_H_
