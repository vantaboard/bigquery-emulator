#include <cstddef>
#include <string>
#include <utility>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "backend/engine/semantic/error.h"
#include "backend/engine/semantic/eval_udaf.h"
#include "backend/engine/semantic/eval_udaf_internal.h"
#include "backend/engine/semantic/frame_stack.h"
#include "backend/engine/semantic/value.h"
#include "googlesql/public/function.h"
#include "googlesql/public/function_signature.h"
#include "googlesql/resolved_ast/resolved_ast.h"

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
    const std::vector<size_t>& row_indices) {
  for (size_t i = 0; i < arg_names.size(); ++i) {
    if (is_agg[i]) continue;
    if (i >= arg_columns.size() || arg_columns[i].empty()) {
      return MakeSemanticError(
          SemanticErrorReason::kInvalidArgument,
          absl::StrCat("semantic: SQL UDAF non-aggregate parameter '",
                       arg_names[i],
                       "' has no evaluated argument value"));
    }
    size_t sample_row = row_indices.empty() ? 0 : row_indices.front();
    if (sample_row >= arg_columns[i].size()) {
      return absl::InternalError(
          "semantic: SQL UDAF non-aggregate argument row out of range");
    }
    absl::Status declared =
        outer_args.Declare(arg_names[i], arg_columns[i][sample_row]);
    if (!declared.ok()) return declared;
  }
  return absl::OkStatus();
}

absl::StatusOr<
    std::pair<ColumnBindings, absl::flat_hash_map<std::string, Value>>>
EvalListedUdafAggregates(const ::googlesql::SQLFunction& sql_fn,
                         const UdafEvalScope& udaf,
                         FrameStack& outer_args,
                         const EvalContext& ctx) {
  ColumnBindings agg_results;
  absl::flat_hash_map<std::string, Value> agg_results_by_name;
  if (const auto* agg_list = sql_fn.aggregate_expression_list();
      agg_list != nullptr) {
    for (const auto& cc_ptr : *agg_list) {
      const ::googlesql::ResolvedComputedColumn* cc = cc_ptr.get();
      if (cc == nullptr || cc->expr() == nullptr) {
        return absl::InternalError(
            "semantic: SQL UDAF aggregate_expression_list entry is null");
      }
      const auto* agg_expr =
          cc->expr()->GetAs<::googlesql::ResolvedAggregateFunctionCall>();
      if (agg_expr == nullptr) {
        return absl::InternalError(
            "semantic: SQL UDAF aggregate_expression_list expr is not an "
            "aggregate call");
      }
      EvalContext agg_ctx = ctx;
      agg_ctx.arguments = &outer_args;
      agg_ctx.udaf = &udaf;
      absl::StatusOr<Value> agg_value =
          EvalUdafInnerAggregate(*agg_expr, udaf, agg_ctx);
      if (!agg_value.ok()) return agg_value.status();
      agg_results.emplace(cc->column().column_id(), *agg_value);
      agg_results_by_name.emplace(std::string(cc->column().name()), *agg_value);
    }
  }
  return std::make_pair(std::move(agg_results), std::move(agg_results_by_name));
}

}  // namespace udaf_internal
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
