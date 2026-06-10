#include "backend/engine/semantic/eval_udaf.h"

#include <cstddef>
#include <string>
#include <utility>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "backend/engine/semantic/error.h"
#include "backend/engine/semantic/eval_expr.h"
#include "backend/engine/semantic/eval_expr_internal.h"
#include "backend/engine/semantic/frame_stack.h"
#include "backend/engine/semantic/functions/specialized_funcs.h"
#include "backend/engine/semantic/value.h"
#include "googlesql/public/function.h"
#include "googlesql/public/function_signature.h"
#include "googlesql/resolved_ast/resolved_ast.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {

namespace {

absl::StatusOr<std::vector<bool>> ArgIsAggregateFlags(
    const ::googlesql::Function& fn) {
  if (fn.NumSignatures() < 1) {
    return absl::InvalidArgumentError(
        "semantic: SQL UDAF has no function signatures");
  }
  const ::googlesql::FunctionSignature* sig = fn.GetSignature(0);
  if (sig == nullptr) {
    return absl::InvalidArgumentError(
        "semantic: SQL UDAF signature lookup failed");
  }
  std::vector<bool> flags;
  flags.reserve(sig->arguments().size());
  for (const ::googlesql::FunctionArgumentType& arg : sig->arguments()) {
    flags.push_back(!arg.options().is_not_aggregate());
  }
  return flags;
}

}  // namespace

absl::Status BindUdafRowFrame(
    const UdafEvalScope& udaf,
    const EvalContext& ctx,
    size_t row_index,
    FrameStack& row_args,
    ColumnBindings& row_columns,
    absl::flat_hash_map<std::string, Value>& row_columns_by_name) {
  row_columns.clear();
  row_columns_by_name.clear();
  row_args.PushFrame();
  if (udaf.arg_names == nullptr || udaf.arg_is_aggregate == nullptr ||
      udaf.arg_columns == nullptr) {
    return absl::InternalError("semantic: SQL UDAF scope is incomplete");
  }
  size_t agg_slot = 0;
  for (size_t i = 0; i < udaf.arg_names->size(); ++i) {
    const std::string& name = (*udaf.arg_names)[i];
    if ((*udaf.arg_is_aggregate)[i]) {
      if (i >= udaf.arg_columns->size() ||
          row_index >= (*udaf.arg_columns)[i].size()) {
        return absl::InternalError(
            "semantic: SQL UDAF aggregate argument column out of range");
      }
      const Value cell = (*udaf.arg_columns)[i][row_index];
      absl::Status declared = row_args.Declare(name, cell);
      if (!declared.ok()) return declared;
      ++agg_slot;
      row_columns_by_name[absl::StrCat("$agg", agg_slot)] = cell;
      continue;
    }
    if (ctx.arguments == nullptr) {
      return MakeSemanticError(
          SemanticErrorReason::kInvalidArgument,
          absl::StrCat("semantic: SQL UDAF non-aggregate parameter '",
                       name,
                       "' has no outer invocation binding"));
    }
    absl::StatusOr<Value> bound = ctx.arguments->Lookup(name);
    if (!bound.ok()) return bound.status();
    absl::Status declared = row_args.Declare(name, *std::move(bound));
    if (!declared.ok()) return declared;
  }
  if (udaf.agg_column_to_arg != nullptr) {
    for (const auto& [col_id, arg_i] : *udaf.agg_column_to_arg) {
      if (arg_i >= udaf.arg_columns->size() ||
          row_index >= (*udaf.arg_columns)[arg_i].size()) {
        return absl::InternalError(
            "semantic: SQL UDAF aggregate column binding out of range");
      }
      row_columns.emplace(col_id, (*udaf.arg_columns)[arg_i][row_index]);
    }
  }
  return absl::OkStatus();
}

absl::StatusOr<Value> EvalUdafInnerAggregate(
    const ::googlesql::ResolvedAggregateFunctionCall& call,
    const UdafEvalScope& udaf,
    const EvalContext& ctx) {
  if (udaf.row_indices == nullptr) {
    return absl::InternalError(
        "semantic: SQL UDAF inner aggregate missing row indices");
  }
  std::vector<std::vector<Value>> inner_columns(
      static_cast<size_t>(call.argument_list_size()));
  for (size_t r : *udaf.row_indices) {
    FrameStack row_args;
    ColumnBindings row_columns;
    absl::flat_hash_map<std::string, Value> row_columns_by_name;
    absl::Status bound = BindUdafRowFrame(
        udaf, ctx, r, row_args, row_columns, row_columns_by_name);
    if (!bound.ok()) return bound;
    EvalContext row_ctx = ctx;
    row_ctx.arguments = &row_args;
    row_ctx.columns = &row_columns;
    row_ctx.columns_by_name = &row_columns_by_name;
    row_ctx.udaf = nullptr;
    for (int a = 0; a < call.argument_list_size(); ++a) {
      auto v = EvalExpr(*call.argument_list(a), row_ctx);
      if (!v.ok()) return v.status();
      inner_columns[static_cast<size_t>(a)].push_back(*std::move(v));
    }
  }
  return functions::EvalAggregateCall(call, inner_columns);
}

absl::StatusOr<Value> EvalUdafInnerFunctionCall(
    const ::googlesql::ResolvedFunctionCall& call,
    const UdafEvalScope& udaf,
    const EvalContext& ctx) {
  if (udaf.row_indices == nullptr || call.function() == nullptr) {
    return absl::InternalError(
        "semantic: SQL UDAF inner function call invalid");
  }
  std::vector<std::vector<Value>> inner_columns(
      static_cast<size_t>(call.argument_list_size()));
  for (size_t r : *udaf.row_indices) {
    FrameStack row_args;
    ColumnBindings row_columns;
    absl::flat_hash_map<std::string, Value> row_columns_by_name;
    absl::Status bound = BindUdafRowFrame(
        udaf, ctx, r, row_args, row_columns, row_columns_by_name);
    if (!bound.ok()) return bound;
    EvalContext row_ctx = ctx;
    row_ctx.arguments = &row_args;
    row_ctx.columns = &row_columns;
    row_ctx.columns_by_name = &row_columns_by_name;
    row_ctx.udaf = nullptr;
    for (int a = 0; a < call.argument_list_size(); ++a) {
      auto v = EvalExpr(*call.argument_list(a), row_ctx);
      if (!v.ok()) return v.status();
      inner_columns[static_cast<size_t>(a)].push_back(*std::move(v));
    }
  }
  std::string name =
      eval_expr_internal::LowerFunctionDispatchName(call.function());
  return functions::EvalAggregateBuiltin(
      name, call.type(), /*distinct=*/false, inner_columns);
}

absl::StatusOr<Value> EvalSqlUdafBody(
    const ::googlesql::ResolvedAggregateFunctionCall& call,
    const ::googlesql::SQLFunction& sql_fn,
    const std::vector<std::vector<Value>>& arg_columns,
    const std::vector<size_t>& row_indices,
    const EvalContext& ctx) {
  const ::googlesql::ResolvedExpr* body = sql_fn.FunctionExpression();
  if (body == nullptr) {
    return absl::InvalidArgumentError(
        "semantic: SQL UDAF has null function expression");
  }
  std::vector<std::string> arg_names = sql_fn.GetArgumentNames();
  if (arg_names.size() != static_cast<size_t>(call.argument_list_size())) {
    return absl::InvalidArgumentError(
        absl::StrCat("semantic: SQL UDAF argument count mismatch (expected ",
                     arg_names.size(),
                     ", got ",
                     call.argument_list_size(),
                     ")"));
  }
  absl::StatusOr<std::vector<bool>> is_agg_or = ArgIsAggregateFlags(sql_fn);
  if (!is_agg_or.ok()) return is_agg_or.status();
  if (is_agg_or->size() != arg_names.size()) {
    return absl::InternalError(
        "semantic: SQL UDAF signature/name arity mismatch");
  }

  std::vector<bool> is_agg = *is_agg_or;

  FrameStack outer_args;
  outer_args.PushFrame();
  for (size_t i = 0; i < arg_names.size(); ++i) {
    if ((*is_agg_or)[i]) continue;
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

  UdafEvalScope udaf{
      .arg_columns = &arg_columns,
      .row_indices = &row_indices,
      .arg_is_aggregate = &is_agg,
      .arg_names = &arg_names,
      .agg_column_to_arg = nullptr,
  };

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

  EvalContext final_ctx = ctx;
  final_ctx.arguments = &outer_args;
  final_ctx.columns = &agg_results;
  final_ctx.columns_by_name = &agg_results_by_name;
  final_ctx.udaf = nullptr;
  return EvalExpr(*body, final_ctx);
}

}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
