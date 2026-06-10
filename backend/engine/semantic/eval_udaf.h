#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_EVAL_UDAF_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_EVAL_UDAF_H_

#include <vector>

#include "absl/status/statusor.h"
#include "backend/engine/semantic/eval_context.h"
#include "backend/engine/semantic/value.h"
#include "googlesql/public/sql_function.h"
#include "googlesql/resolved_ast/resolved_ast.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {

absl::StatusOr<Value> EvalUdafInnerAggregate(
    const ::googlesql::ResolvedAggregateFunctionCall& call,
    const UdafEvalScope& udaf,
    const EvalContext& ctx);

absl::StatusOr<Value> EvalUdafInnerFunctionCall(
    const ::googlesql::ResolvedFunctionCall& call,
    const UdafEvalScope& udaf,
    const EvalContext& ctx);

absl::StatusOr<Value> EvalSqlUdafBody(
    const ::googlesql::ResolvedAggregateFunctionCall& call,
    const ::googlesql::SQLFunction& sql_fn,
    const std::vector<std::vector<Value>>& arg_columns,
    const std::vector<size_t>& row_indices,
    const EvalContext& ctx);

}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_EVAL_UDAF_H_
