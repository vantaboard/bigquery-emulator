#include <string>
#include <utility>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/ascii.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "backend/engine/semantic/error.h"
#include "backend/engine/semantic/eval_expr_internal.h"
#include "backend/engine/semantic/functions/datetime_funcs.h"
#include "backend/engine/semantic/functions/dispatch.h"
#include "backend/engine/semantic/functions/geog_funcs.h"
#include "backend/engine/semantic/functions/operator_funcs.h"
#include "backend/engine/semantic/stubs/dispatch.h"
#include "backend/engine/semantic/value.h"
#include "googlesql/public/functions/date_time_util.h"
#include "googlesql/public/type.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace eval_expr_internal {

namespace {

absl::StatusOr<Value> DispatchBuiltinArithmetic(
    absl::string_view name,
    const std::vector<Value>& args,
    const ::googlesql::Type* return_type) {
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
  return MakeSemanticError(SemanticErrorReason::kNotImplemented,
                           "semantic: unknown arithmetic builtin");
}

absl::StatusOr<Value> DispatchBuiltinComparison(
    absl::string_view name, const std::vector<Value>& args) {
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
  if (name == "$like" || name == "$not_like") {
    return functions::DispatchLike(name, args);
  }
  if (name == "$between" || name == "$not_between") {
    return functions::DispatchBetween(name, args);
  }
  return MakeSemanticError(SemanticErrorReason::kNotImplemented,
                           "semantic: unknown comparison builtin");
}

absl::StatusOr<Value> DispatchBuiltinIn(absl::string_view name,
                                        const std::vector<Value>& args) {
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
  if (name == "$in_array" || name == "$not_in_array") {
    if (args.size() != 2) {
      return absl::InvalidArgumentError(
          "semantic: $in_array expects exactly two arguments");
    }
    const absl::string_view in_name =
        name == "$not_in_array" ? "$not_in" : "$in";
    if (args[1].is_null()) {
      return Value::NullBool();
    }
    if (!args[1].type()->IsArray()) {
      return absl::InvalidArgumentError(
          "semantic: $in_array expects an ARRAY second argument");
    }
    std::vector<Value> expanded = {args[0]};
    expanded.insert(
        expanded.end(), args[1].elements().begin(), args[1].elements().end());
    return functions::DispatchIn(in_name, expanded);
  }
  return MakeSemanticError(SemanticErrorReason::kNotImplemented,
                           "semantic: unknown IN builtin");
}

absl::StatusOr<Value> DispatchBuiltinLogical(absl::string_view name,
                                             const std::vector<Value>& args) {
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
  if (name == "$is_true" || name == "$is_not_true") {
    return functions::DispatchIsTrue(name, args);
  }
  if (name == "$is_false" || name == "$is_not_false") {
    return functions::DispatchIsFalse(name, args);
  }
  if (name == "$is_distinct_from" || name == "$is_not_distinct_from") {
    return functions::DispatchIsDistinctFrom(name, args);
  }
  return MakeSemanticError(SemanticErrorReason::kNotImplemented,
                           "semantic: unknown logical builtin");
}

absl::StatusOr<Value> DispatchBuiltinConditional(
    absl::string_view name,
    const std::vector<Value>& args,
    const ::googlesql::Type* return_type) {
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
  return MakeSemanticError(SemanticErrorReason::kNotImplemented,
                           "semantic: unknown conditional builtin");
}

absl::StatusOr<Value> DispatchBuiltinSafe(
    absl::string_view name,
    const std::vector<Value>& args,
    const ::googlesql::Type* return_type) {
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
  return MakeSemanticError(SemanticErrorReason::kNotImplemented,
                           "semantic: unknown SAFE builtin");
}

absl::StatusOr<Value> DispatchBuiltinExtractPrefix(
    absl::string_view name,
    const std::vector<Value>& args,
    const ::googlesql::Type* return_type) {
  if (!absl::StartsWith(name, "$extract_")) {
    return MakeSemanticError(SemanticErrorReason::kNotImplemented,
                             "semantic: not an extract builtin");
  }
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

bool IsArithmeticBuiltin(absl::string_view name) {
  return name == "$add" || name == "add" || name == "$subtract" ||
         name == "subtract" || name == "$multiply" || name == "multiply" ||
         name == "$divide" || name == "divide" || name == "$unary_minus" ||
         name == "unary_minus" || name == "abs" || name == "$make_array";
}

bool IsComparisonBuiltin(absl::string_view name) {
  return name == "$equal" || name == "equal" || name == "$not_equal" ||
         name == "not_equal" || name == "$less" || name == "less" ||
         name == "$less_or_equal" || name == "less_or_equal" ||
         name == "$greater" || name == "greater" ||
         name == "$greater_or_equal" || name == "greater_or_equal" ||
         name == "$like" || name == "$not_like" || name == "$between" ||
         name == "$not_between";
}

bool IsInBuiltin(absl::string_view name) {
  return name == "$in" || name == "$not_in" || name == "$in_array" ||
         name == "$not_in_array";
}

bool IsLogicalBuiltin(absl::string_view name) {
  return name == "$and" || name == "and" || name == "$or" || name == "or" ||
         name == "$not" || name == "not" || name == "$is_null" ||
         name == "is_null" || name == "$is_not_null" || name == "is_not_null" ||
         name == "$is_true" || name == "$is_not_true" || name == "$is_false" ||
         name == "$is_not_false" || name == "$is_distinct_from" ||
         name == "$is_not_distinct_from";
}

bool IsConditionalBuiltin(absl::string_view name) {
  return name == "if" || name == "coalesce" || name == "ifnull" ||
         name == "nullif" || name == "$case_with_value" ||
         name == "$case_no_value";
}

bool IsSafeBuiltin(absl::string_view name) {
  return name == "safe_add" || name == "safe_subtract" ||
         name == "safe_multiply" || name == "safe_negate" ||
         name == "safe_divide";
}

}  // namespace

absl::StatusOr<Value> DispatchFunctionByName(
    absl::string_view name,
    const std::vector<Value>& args,
    const ::googlesql::Type* return_type,
    const EvalContext* ctx) {
  if (IsArithmeticBuiltin(name)) {
    return DispatchBuiltinArithmetic(name, args, return_type);
  }
  if (IsComparisonBuiltin(name)) {
    return DispatchBuiltinComparison(name, args);
  }
  if (IsInBuiltin(name)) {
    return DispatchBuiltinIn(name, args);
  }
  if (IsLogicalBuiltin(name)) {
    return DispatchBuiltinLogical(name, args);
  }
  if (name == "$bitwise_and" || name == "$bitwise_or" ||
      name == "$bitwise_xor" || name == "$bitwise_not" ||
      name == "$bitwise_left_shift" || name == "$bitwise_right_shift") {
    return functions::DispatchBitwise(name, args);
  }
  if (name == "$interval") {
    return functions::DispatchInterval(args, return_type);
  }
  if (IsConditionalBuiltin(name)) {
    return DispatchBuiltinConditional(name, args, return_type);
  }
  if (IsSafeBuiltin(name)) {
    return DispatchBuiltinSafe(name, args, return_type);
  }
  if (absl::StartsWith(name, "$extract_")) {
    return DispatchBuiltinExtractPrefix(name, args, return_type);
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

}  // namespace eval_expr_internal
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
