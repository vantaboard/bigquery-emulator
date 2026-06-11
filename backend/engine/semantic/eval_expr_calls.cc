#include <string>
#include <utility>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "backend/catalog/udf_registry.h"
#include "backend/engine/semantic/error.h"
#include "backend/engine/semantic/eval_expr.h"
#include "backend/engine/semantic/eval_expr_internal.h"
#include "backend/engine/semantic/eval_udaf.h"
#include "backend/engine/semantic/frame_stack.h"
#include "backend/engine/semantic/functions/dispatch.h"
#include "backend/engine/semantic/functions/geog_funcs.h"
#include "backend/engine/semantic/value.h"
#include "googlesql/public/function.h"
#include "googlesql/public/sql_function.h"
#include "googlesql/public/templated_sql_function.h"
#include "googlesql/public/type.h"
#include "googlesql/resolved_ast/resolved_ast.h"
#include "googlesql/resolved_ast/resolved_node_kind.pb.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace eval_expr_internal {

absl::StatusOr<Value> EvalArrayTransform(
    const ::googlesql::ResolvedFunctionCall& call, const EvalContext& ctx) {
  const ::googlesql::ResolvedExpr* array_expr = nullptr;
  const ::googlesql::ResolvedInlineLambda* lambda = nullptr;
  if (call.generic_argument_list_size() >= 2) {
    const ::googlesql::ResolvedFunctionArgument* a0 =
        call.generic_argument_list(0);
    const ::googlesql::ResolvedFunctionArgument* a1 =
        call.generic_argument_list(1);
    if (a0 != nullptr) array_expr = a0->expr();
    if (a1 != nullptr) lambda = a1->inline_lambda();
  }
  if (array_expr == nullptr && call.argument_list_size() >= 1) {
    array_expr = call.argument_list(0);
  }
  if (array_expr == nullptr || lambda == nullptr) {
    return MakeSemanticError(
        SemanticErrorReason::kNotImplemented,
        "semantic: ARRAY_TRANSFORM requires array and inline lambda arguments");
  }
  if (lambda->body() == nullptr || lambda->parameter_list_size() < 1) {
    return absl::InvalidArgumentError(
        "semantic: ARRAY_TRANSFORM lambda is malformed");
  }
  auto array_val = EvalExpr(*array_expr, ctx);
  if (!array_val.ok()) return array_val.status();
  if (array_val->is_null()) return Value::Null(call.type());
  if (!array_val->type()->IsArray()) {
    return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                             "semantic: ARRAY_TRANSFORM first argument must "
                             "be ARRAY");
  }
  const ::googlesql::ArrayType* out_arr =
      call.type() != nullptr && call.type()->IsArray()
          ? call.type()->AsArray()
          : array_val->type()->AsArray();
  const int param_col_id = lambda->parameter_list(0)->column().column_id();
  std::vector<Value> out_elems;
  out_elems.reserve(array_val->num_elements());
  for (int i = 0; i < array_val->num_elements(); ++i) {
    ColumnBindings bind;
    bind.emplace(param_col_id, array_val->element(i));
    EvalContext row_ctx = ctx;
    row_ctx.columns = &bind;
    auto mapped = EvalExpr(*lambda->body(), row_ctx);
    if (!mapped.ok()) return mapped.status();
    out_elems.push_back(*std::move(mapped));
  }
  return Value::Array(out_arr, std::move(out_elems));
}
}  // namespace eval_expr_internal

using eval_expr_internal::DispatchFunctionByName;
using eval_expr_internal::EvalArrayTransform;
using eval_expr_internal::LowerFunctionDispatchName;
using eval_expr_internal::NullOfType;

absl::StatusOr<Value> EvalIfLazy(const ::googlesql::ResolvedFunctionCall& call,
                                 const EvalContext& ctx) {
  if (call.argument_list_size() != 3) {
    return absl::InvalidArgumentError("semantic: IF expects three arguments");
  }
  auto cond = EvalExpr(*call.argument_list(0), ctx);
  if (!cond.ok()) return cond.status();
  const bool take_then = !cond->is_null() &&
                         cond->type_kind() == ::googlesql::TYPE_BOOL &&
                         cond->bool_value();
  return EvalExpr(*call.argument_list(take_then ? 1 : 2), ctx);
}

absl::StatusOr<Value> EvalIfNullLazy(
    const ::googlesql::ResolvedFunctionCall& call, const EvalContext& ctx) {
  if (call.argument_list_size() != 2) {
    return absl::InvalidArgumentError("semantic: IFNULL expects two arguments");
  }
  auto first = EvalExpr(*call.argument_list(0), ctx);
  if (!first.ok()) return first.status();
  if (!first->is_null()) return *std::move(first);
  return EvalExpr(*call.argument_list(1), ctx);
}

absl::StatusOr<Value> EvalCaseNoValueLazy(
    const ::googlesql::ResolvedFunctionCall& call, const EvalContext& ctx) {
  const int n = call.argument_list_size();
  if (n < 1 || (n % 2) == 0) {
    return absl::InvalidArgumentError(
        "semantic: CASE expects odd argument count");
  }
  for (int i = 0; i + 1 < n; i += 2) {
    auto cond = EvalExpr(*call.argument_list(i), ctx);
    if (!cond.ok()) return cond.status();
    if (!cond->is_null() && cond->type_kind() == ::googlesql::TYPE_BOOL &&
        cond->bool_value()) {
      return EvalExpr(*call.argument_list(i + 1), ctx);
    }
  }
  return EvalExpr(*call.argument_list(n - 1), ctx);
}

absl::StatusOr<Value> EvalCaseWithValueLazy(
    const ::googlesql::ResolvedFunctionCall& call, const EvalContext& ctx) {
  const int n = call.argument_list_size();
  if (n < 2 || (n % 2) != 0) {
    return absl::InvalidArgumentError(
        "semantic: $case_with_value expects even argument count");
  }
  auto input = EvalExpr(*call.argument_list(0), ctx);
  if (!input.ok()) return input.status();
  for (int i = 1; i + 1 < n; i += 2) {
    auto when = EvalExpr(*call.argument_list(i), ctx);
    if (!when.ok()) return when.status();
    if (input->is_null() || when->is_null()) continue;
    if (input->Equals(*when)) {
      return EvalExpr(*call.argument_list(i + 1), ctx);
    }
  }
  return EvalExpr(*call.argument_list(n - 1), ctx);
}

absl::StatusOr<Value> EvalSqlUdfBody(
    const ::googlesql::ResolvedFunctionCall& call,
    const ::googlesql::ResolvedExpr& body,
    const EvalContext& ctx) {
  const ::googlesql::Function* fn = call.function();
  if (fn == nullptr) {
    return absl::InvalidArgumentError(
        "semantic: SQL UDF call has null function");
  }
  std::vector<std::string> arg_names;
  if (fn->GetGroup() ==
      ::googlesql::TemplatedSQLFunction::kTemplatedSQLFunctionGroup) {
    arg_names = static_cast<const ::googlesql::TemplatedSQLFunction*>(fn)
                    ->GetArgumentNames();
  } else if (fn->GetGroup() == ::googlesql::SQLFunction::kSQLFunctionGroup) {
    arg_names =
        static_cast<const ::googlesql::SQLFunction*>(fn)->GetArgumentNames();
  } else {
    return absl::InvalidArgumentError(
        "semantic: SQL UDF call is not a templated or SQL function");
  }
  if (arg_names.size() != static_cast<size_t>(call.argument_list_size())) {
    return absl::InvalidArgumentError(
        absl::StrCat("semantic: SQL UDF argument count mismatch (expected ",
                     arg_names.size(),
                     ", got ",
                     call.argument_list_size(),
                     ")"));
  }
  std::vector<Value> arg_values;
  arg_values.reserve(static_cast<size_t>(call.argument_list_size()));
  for (int i = 0; i < call.argument_list_size(); ++i) {
    auto v = EvalExpr(*call.argument_list(i), ctx);
    if (!v.ok()) return v.status();
    if (!v->is_null() && !v->is_valid()) {
      return absl::InternalError(
          "semantic: SQL UDF argument evaluated to invalid Value");
    }
    if (v->is_null()) {
      const ::googlesql::Type* arg_type = call.argument_list(i)->type();
      arg_values.push_back(NullOfType(arg_type));
    } else {
      arg_values.push_back(*std::move(v));
    }
  }
  FrameStack arg_frames;
  arg_frames.PushFrame();
  for (size_t i = 0; i < arg_values.size(); ++i) {
    absl::Status declared = arg_frames.Declare(arg_names[i], arg_values[i]);
    if (!declared.ok()) return declared;
  }
  EvalContext inner = ctx;
  inner.arguments = &arg_frames;
  auto result = EvalExpr(body, inner);
  if (!result.ok()) return result;
  // BigQuery translates NULL ARRAY values to empty ARRAY in query
  // results (including SQL UDF return values). Keep NULL/empty distinct
  // inside the UDF body; coerce only at the UDF boundary.
  const ::googlesql::Type* return_type = call.type();
  if (result->is_null() && return_type != nullptr && return_type->IsArray()) {
    return Value::Array(return_type->AsArray(), {});
  }
  return *result;
}

absl::StatusOr<Value> EvalFunctionCall(
    const ::googlesql::ResolvedFunctionCall& call, const EvalContext& ctx) {
  if (call.function() == nullptr) {
    return absl::InvalidArgumentError(
        "semantic: ResolvedFunctionCall has null function");
  }
  const ::googlesql::Function* fn = call.function();
  if (ctx.udaf != nullptr && fn != nullptr && fn->IsAggregate()) {
    return EvalUdafInnerFunctionCall(call, *ctx.udaf, ctx);
  }
  if (const std::shared_ptr<::googlesql::ResolvedFunctionCallInfo>& info =
          call.function_call_info();
      info != nullptr && fn != nullptr &&
      fn->GetGroup() ==
          ::googlesql::TemplatedSQLFunction::kTemplatedSQLFunctionGroup) {
    const auto* templated =
        static_cast<const ::googlesql::TemplatedSQLFunctionCall*>(info.get());
    if (templated->expr() != nullptr &&
        catalog::IsProjectRegisteredFunction(ctx.project_id, fn->Name())) {
      return EvalSqlUdfBody(call, *templated->expr(), ctx);
    }
  }
  if (fn != nullptr &&
      fn->GetGroup() == ::googlesql::SQLFunction::kSQLFunctionGroup) {
    const auto* sql_fn = static_cast<const ::googlesql::SQLFunction*>(fn);
    if (sql_fn->FunctionExpression() != nullptr &&
        catalog::IsProjectRegisteredFunction(ctx.project_id, fn->Name())) {
      return EvalSqlUdfBody(call, *sql_fn->FunctionExpression(), ctx);
    }
  }
  if (fn != nullptr && fn->GetGroup() == "External_function" &&
      catalog::IsProjectRegisteredFunction(ctx.project_id, fn->Name())) {
    return absl::UnimplementedError(
        "JavaScript UDF call-time evaluation is not implemented; "
        "CREATE FUNCTION ... LANGUAGE js registers metadata only "
        "(see docs/ENGINE_POLICY.md).");
  }
  const std::string name = LowerFunctionDispatchName(call.function());
  if (name == "emu_format_t") {
    std::vector<Value> args;
    args.reserve(call.argument_list_size());
    for (int i = 0; i < call.argument_list_size(); ++i) {
      auto v = EvalExpr(*call.argument_list(i), ctx);
      if (!v.ok()) return v.status();
      args.push_back(*std::move(v));
    }
    return functions::EmuFormatTypeLiteral(args);
  }
  if (name == "array_transform") {
    return EvalArrayTransform(call, ctx);
  }
  if (name == "if") {
    return EvalIfLazy(call, ctx);
  }
  if (name == "ifnull") {
    return EvalIfNullLazy(call, ctx);
  }
  if (name == "$case_no_value") {
    return EvalCaseNoValueLazy(call, ctx);
  }
  if (name == "$case_with_value") {
    return EvalCaseWithValueLazy(call, ctx);
  }
  if (name == "error" && call.argument_list_size() == 1) {
    auto msg = EvalExpr(*call.argument_list(0), ctx);
    if (!msg.ok()) return msg.status();
    std::vector<Value> args = {*std::move(msg)};
    return DispatchFunctionByName(name, args, call.type(), &ctx);
  }
  std::vector<Value> args;
  args.reserve(call.argument_list_size());
  for (int i = 0; i < call.argument_list_size(); ++i) {
    const ::googlesql::ResolvedExpr* arg = call.argument_list(i);
    if (arg == nullptr) {
      return absl::InvalidArgumentError(
          "semantic: ResolvedFunctionCall argument is null");
    }
    auto v = EvalExpr(*arg, ctx);
    if (!v.ok()) {
      // SAFE_ERROR_MODE swallows evaluation failures from any
      // operand and converts them into NULL of the return type.
      if (call.error_mode() ==
              ::googlesql::ResolvedFunctionCallBase::SAFE_ERROR_MODE &&
          (v.status().code() == absl::StatusCode::kInvalidArgument ||
           v.status().code() == absl::StatusCode::kOutOfRange)) {
        return NullOfType(call.type());
      }
      return v.status();
    }
    args.push_back(*std::move(v));
  }
  // Use `FullName(/*include_group=*/false)` so namespaced families
  // like `KEYS.NEW_KEYSET` / `NET.HOST` / `HLL_COUNT.MERGE`
  // resolve to their dotted, lowercased dispatch key
  // (`keys.new_keyset`, `net.host`, `hll_count.merge`). The route
  // classifier (`route_classifier.cc::CheckFunction`) uses the
  // same name shape when promoting `local_stub` / `semantic_executor`
  // dispositions, so the names line up across the two sides. For
  // non-namespaced functions (`concat`, `abs`, `safe_divide`)
  // `FullName(false) == Name()`, so this is a no-op.
  auto result = DispatchFunctionByName(name, args, call.type(), &ctx);
  if (!result.ok() &&
      call.error_mode() ==
          ::googlesql::ResolvedFunctionCallBase::SAFE_ERROR_MODE) {
    SemanticErrorReason reason = GetSemanticErrorReason(result.status());
    if (reason == SemanticErrorReason::kOverflow ||
        reason == SemanticErrorReason::kDivisionByZero ||
        reason == SemanticErrorReason::kInvalidArgument) {
      return NullOfType(call.type());
    }
  }
  return result;
}

}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
