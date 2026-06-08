#include <string>
#include <utility>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/ascii.h"
#include "absl/strings/match.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "backend/catalog/udf_registry.h"
#include "backend/engine/semantic/error.h"
#include "backend/engine/semantic/eval_expr.h"
#include "backend/engine/semantic/eval_expr_internal.h"
#include "backend/engine/semantic/eval_udaf.h"
#include "backend/engine/semantic/frame_stack.h"
#include "backend/engine/semantic/functions/datetime_funcs.h"
#include "backend/engine/semantic/functions/dispatch.h"
#include "backend/engine/semantic/functions/geog_funcs.h"
#include "backend/engine/semantic/functions/operator_funcs.h"
#include "backend/engine/semantic/stubs/dispatch.h"
#include "backend/engine/semantic/value.h"
#include "googlesql/public/function.h"
#include "googlesql/public/functions/date_time_util.h"
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

absl::StatusOr<Value> DispatchFunctionByName(
    absl::string_view name,
    const std::vector<Value>& args,
    const ::googlesql::Type* return_type,
    const EvalContext* ctx) {
  if (name == "$add" || name == "add") {
    return DispatchAdd(args, return_type);
  }
  if (name == "$subtract" || name == "subtract") {
    return DispatchSub(args, return_type);
  }
  if (name == "$multiply" || name == "multiply") {
    return DispatchMul(args, return_type);
  }
  if (name == "$divide" || name == "divide") {
    return DispatchDiv(args, return_type);
  }
  if (name == "$unary_minus" || name == "unary_minus") {
    return DispatchUnaryMinus(args, return_type);
  }
  if (name == "abs") return DispatchAbs(args, return_type);
  if (name == "$make_array") {
    if (return_type == nullptr || !return_type->IsArray()) {
      return absl::InvalidArgumentError(
          "semantic: $make_array requires ARRAY return type");
    }
    return Value::Array(return_type->AsArray(), args);
  }
  if (name == "$equal" || name == "equal") {
    return DispatchEqual(args);
  }
  if (name == "$not_equal" || name == "not_equal") {
    return DispatchNotEqual(args);
  }
  if (name == "$less" || name == "less") {
    return DispatchLess(args);
  }
  if (name == "$less_or_equal" || name == "less_or_equal") {
    return DispatchLessOrEqual(args);
  }
  if (name == "$greater" || name == "greater") {
    return DispatchGreater(args);
  }
  if (name == "$greater_or_equal" || name == "greater_or_equal") {
    return DispatchGreaterOrEqual(args);
  }
  if (name == "$and" || name == "and") {
    return DispatchAnd(args);
  }
  if (name == "$or" || name == "or") {
    return DispatchOr(args);
  }
  if (name == "$not" || name == "not") {
    return DispatchNot(args);
  }
  if (name == "$is_null" || name == "is_null") {
    return DispatchIsNull(args, /*negate=*/false);
  }
  if (name == "$is_not_null" || name == "is_not_null") {
    return DispatchIsNull(args, /*negate=*/true);
  }
  if (name == "$like" || name == "$not_like") {
    return functions::DispatchLike(name, args);
  }
  if (name == "$between" || name == "$not_between") {
    return functions::DispatchBetween(name, args);
  }
  if (name == "$in" || name == "$not_in") {
    if (args.size() == 2 && args[1].type() != nullptr &&
        args[1].type()->IsArray()) {
      std::vector<Value> expanded = {args[0]};
      expanded.insert(
          expanded.end(), args[1].elements().begin(), args[1].elements().end());
      return functions::DispatchIn(name, expanded);
    }
    return functions::DispatchIn(name, args);
  }
  if (name == "$is_true" || name == "$is_not_true") {
    return functions::DispatchIsTrue(name, args);
  }
  if (name == "$is_false" || name == "$is_not_false") {
    return functions::DispatchIsFalse(name, args);
  }
  if (name == "$is_distinct_from" || name == "$is_not_distinct_from") {
    return functions::DispatchIsDistinctFrom(name, args);
  }
  if (name == "$bitwise_and" || name == "$bitwise_or" ||
      name == "$bitwise_xor" || name == "$bitwise_not" ||
      name == "$bitwise_left_shift" || name == "$bitwise_right_shift") {
    return functions::DispatchBitwise(name, args);
  }
  if (name == "$interval") {
    return functions::DispatchInterval(args, return_type);
  }
  if (name == "if") return DispatchIf(args);
  if (name == "coalesce") return DispatchCoalesce(args, return_type);
  if (name == "ifnull") return DispatchIfNull(args);
  if (name == "nullif") return DispatchNullIf(args, return_type);
  if (name == "$case_with_value") {
    return DispatchCaseWithValue(args, return_type);
  }
  if (name == "$case_no_value") {
    return DispatchCaseNoValue(args, return_type);
  }
  // SAFE_* functions are explicit functions in BigQuery (distinct
  // from the `SAFE.<fn>(...)` SAFE_ERROR_MODE flag). We model them
  // by routing through the underlying strict operator and
  // converting overflow / division-by-zero into NULL.
  if (name == "safe_add") {
    return WrapSafe(DispatchAdd(args, return_type), return_type);
  }
  if (name == "safe_subtract") {
    return WrapSafe(DispatchSub(args, return_type), return_type);
  }
  if (name == "safe_multiply") {
    return WrapSafe(DispatchMul(args, return_type), return_type);
  }
  if (name == "safe_negate") {
    return WrapSafe(DispatchUnaryMinus(args, return_type), return_type);
  }
  if (name == "safe_divide") {
    return WrapSafe(DispatchDiv(args, return_type), return_type);
  }
  if (absl::StartsWith(name, "$extract_")) {
    const std::string part = std::string(name).substr(9);
    if (args.empty() || args[0].is_null()) return NullOfType(return_type);
    if (part == "date" && return_type != nullptr && return_type->IsDate() &&
        args[0].type_kind() == ::googlesql::TYPE_TIMESTAMP) {
      int32_t date = 0;
      if (auto s = ::googlesql::functions::ExtractFromTimestamp(
              ::googlesql::functions::DateTimestampPart::DATE,
              args[0].ToUnixMicros(),
              ::googlesql::functions::TimestampScale::kMicroseconds,
              absl::UTCTimeZone(),
              &date);
          !s.ok()) {
        return s;
      }
      return Value::Date(date);
    }
    std::vector<Value> extract_args;
    extract_args.push_back(args[0]);
    extract_args.push_back(Value::String(absl::AsciiStrToUpper(part)));
    if (args.size() > 1) extract_args.push_back(args[1]);
    auto extracted = functions::Extract(extract_args, return_type);
    if (!extracted.ok()) return extracted.status();
    return *std::move(extracted);
  }
  // Fall through to the per-family dispatch table for functions
  // whose `functions.yaml` row picks the `semantic_executor`
  // disposition with `plan=docs/ENGINE_POLICY.md`.
  // `functions::Dispatch` returns nullopt when the name is not
  // wired here; we surface NOT_IMPLEMENTED in that case so the
  // gateway envelope stays the same as for an unknown function.
  if (auto dispatched = functions::Dispatch(name, args, return_type, ctx)) {
    return *std::move(dispatched);
  }
  // Local-stub families (`local_stub` posture, e.g. KEYS.*).
  // `docs/ENGINE_POLICY.md` picks the deterministic
  // BigQuery-shaped-placeholder posture for a handful of families;
  // the route classifier promotes the surrounding query to
  // `kLocalStub`, the coordinator dispatches it onto the semantic
  // executor, and this Dispatch finally invokes the per-family
  // handler. `stubs::Dispatch` returns nullopt for any name not
  // in its table, so the NOT_IMPLEMENTED fall-through below still
  // applies for genuinely-unsupported functions.
  if (auto dispatched = stubs::Dispatch(name, args, return_type)) {
    return *std::move(dispatched);
  }
  return MakeSemanticError(
      SemanticErrorReason::kNotImplemented,
      absl::StrCat("semantic: function '",
                   name,
                   "' is not yet implemented in the semantic executor"));
}

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
    if (!v->is_valid()) {
      return absl::InternalError(
          "semantic: SQL UDF argument evaluated to invalid Value");
    }
    if (v->is_null()) {
      return NullOfType(call.type());
    }
    arg_values.push_back(*std::move(v));
  }
  FrameStack arg_frames;
  arg_frames.PushFrame();
  for (size_t i = 0; i < arg_values.size(); ++i) {
    absl::Status declared = arg_frames.Declare(arg_names[i], arg_values[i]);
    if (!declared.ok()) return declared;
  }
  EvalContext inner = ctx;
  inner.arguments = &arg_frames;
  return EvalExpr(body, inner);
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
