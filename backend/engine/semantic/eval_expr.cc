#include "backend/engine/semantic/eval_expr.h"

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/ascii.h"
#include "absl/strings/match.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "backend/engine/semantic/error.h"
#include "backend/engine/semantic/functions/dispatch.h"
#include "backend/engine/semantic/value.h"
#include "googlesql/public/function.h"
#include "googlesql/public/numeric_value.h"
#include "googlesql/public/type.h"
#include "googlesql/public/type.pb.h"
#include "googlesql/public/value.h"
#include "googlesql/resolved_ast/resolved_ast.h"
#include "googlesql/resolved_ast/resolved_node_kind.pb.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {

namespace {

// Build a NULL `Value` of `type`'s kind. Used when an operator
// detects a NULL operand and BigQuery semantics propagate NULL, or
// when SAFE-mode swallows an error.
Value NullOfType(const ::googlesql::Type* type) {
  if (type == nullptr) return Value::NullInt64();
  switch (type->kind()) {
    case ::googlesql::TYPE_BOOL:
      return Value::NullBool();
    case ::googlesql::TYPE_INT64:
      return Value::NullInt64();
    case ::googlesql::TYPE_DOUBLE:
      return Value::NullDouble();
    case ::googlesql::TYPE_STRING:
      return Value::NullString();
    case ::googlesql::TYPE_BYTES:
      return Value::NullBytes();
    case ::googlesql::TYPE_DATE:
      return Value::NullDate();
    case ::googlesql::TYPE_TIME:
      return Value::NullTime();
    case ::googlesql::TYPE_DATETIME:
      return Value::NullDatetime();
    case ::googlesql::TYPE_TIMESTAMP:
      return Value::NullTimestamp();
    case ::googlesql::TYPE_NUMERIC:
      return Value::NullNumeric();
    case ::googlesql::TYPE_BIGNUMERIC:
      return Value::NullBigNumeric();
    case ::googlesql::TYPE_JSON:
      return Value::NullJson();
    case ::googlesql::TYPE_GEOGRAPHY:
      return Value::NullGeography();
    case ::googlesql::TYPE_INTERVAL:
      return Value::NullInterval();
    case ::googlesql::TYPE_UUID:
      return Value::NullUuid();
    default:
      // Fall back: the analyzer always types every output column,
      // and the generic null factory is invalid for compound types.
      // Callers that hit this branch should propagate the failure;
      // we return an invalid `Value` so the caller's status check
      // fires through `ToStorageValue`.
      return Value();
  }
}

// Translate an `absl::StatusOr<NumericValue>` (the shape every
// arithmetic helper on `NumericValue` / `BigNumericValue` returns)
// into a `semantic::Value`. Maps a `kOutOfRange` to a structured
// overflow error so the gateway surfaces the right `reason` token.
template <typename NV>
absl::StatusOr<Value> WrapNumeric(absl::StatusOr<NV> result, bool is_bignum) {
  if (!result.ok()) {
    if (result.status().code() == absl::StatusCode::kOutOfRange) {
      return MakeSemanticError(SemanticErrorReason::kOverflow,
                               result.status().message());
    }
    return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                             result.status().message());
  }
  if (is_bignum) {
    if constexpr (std::is_same_v<NV, ::googlesql::BigNumericValue>) {
      return Value::BigNumeric(*result);
    }
  }
  if constexpr (std::is_same_v<NV, ::googlesql::NumericValue>) {
    return Value::Numeric(*result);
  }
  return absl::InternalError("semantic: WrapNumeric: type tag mismatch");
}

// Coerce a Value to FLOAT64 for arithmetic. The analyzer guarantees
// the operands have the right shared type after implicit coercion,
// but defense-in-depth: unexpected kinds bail with INVALID_ARGUMENT.
absl::StatusOr<double> ToDouble(const Value& v) {
  switch (v.type_kind()) {
    case ::googlesql::TYPE_DOUBLE:
      return v.double_value();
    case ::googlesql::TYPE_INT64:
      return static_cast<double>(v.int64_value());
    case ::googlesql::TYPE_NUMERIC: {
      // NumericValue::ToDouble may lose precision but is the
      // canonical coercion path for mixed-type arithmetic.
      return v.numeric_value().ToDouble();
    }
    default:
      return absl::InvalidArgumentError(absl::StrCat(
          "semantic: cannot coerce ", v.type()->DebugString(), " to FLOAT64"));
  }
}

// INT64 arithmetic with checked overflow via the GCC/Clang builtins.
absl::StatusOr<Value> AddInt64(int64_t a, int64_t b) {
  int64_t out = 0;
  if (__builtin_add_overflow(a, b, &out)) {
    return MakeSemanticError(SemanticErrorReason::kOverflow,
                             absl::StrCat("Int64 overflow: ", a, " + ", b));
  }
  return Value::Int64(out);
}
absl::StatusOr<Value> SubInt64(int64_t a, int64_t b) {
  int64_t out = 0;
  if (__builtin_sub_overflow(a, b, &out)) {
    return MakeSemanticError(SemanticErrorReason::kOverflow,
                             absl::StrCat("Int64 overflow: ", a, " - ", b));
  }
  return Value::Int64(out);
}
absl::StatusOr<Value> MulInt64(int64_t a, int64_t b) {
  int64_t out = 0;
  if (__builtin_mul_overflow(a, b, &out)) {
    return MakeSemanticError(SemanticErrorReason::kOverflow,
                             absl::StrCat("Int64 overflow: ", a, " * ", b));
  }
  return Value::Int64(out);
}

absl::StatusOr<Value> ArithmeticAdd(const Value& a, const Value& b) {
  if (a.type_kind() != b.type_kind()) {
    return absl::InvalidArgumentError(
        absl::StrCat("semantic: '+' operands have mismatched kinds: ",
                     a.type()->DebugString(),
                     " vs ",
                     b.type()->DebugString()));
  }
  switch (a.type_kind()) {
    case ::googlesql::TYPE_INT64:
      return AddInt64(a.int64_value(), b.int64_value());
    case ::googlesql::TYPE_DOUBLE: {
      // BigQuery propagates NaN/Inf rather than erroring; no
      // explicit overflow check needed for FLOAT64.
      return Value::Double(a.double_value() + b.double_value());
    }
    case ::googlesql::TYPE_NUMERIC:
      return WrapNumeric(a.numeric_value().Add(b.numeric_value()),
                         /*is_bignum=*/false);
    case ::googlesql::TYPE_BIGNUMERIC:
      return WrapNumeric(a.bignumeric_value().Add(b.bignumeric_value()),
                         /*is_bignum=*/true);
    default:
      return absl::InvalidArgumentError(absl::StrCat(
          "semantic: '+' not implemented for ", a.type()->DebugString()));
  }
}

absl::StatusOr<Value> ArithmeticSub(const Value& a, const Value& b) {
  if (a.type_kind() != b.type_kind()) {
    return absl::InvalidArgumentError(
        absl::StrCat("semantic: '-' operands have mismatched kinds: ",
                     a.type()->DebugString(),
                     " vs ",
                     b.type()->DebugString()));
  }
  switch (a.type_kind()) {
    case ::googlesql::TYPE_INT64:
      return SubInt64(a.int64_value(), b.int64_value());
    case ::googlesql::TYPE_DOUBLE:
      return Value::Double(a.double_value() - b.double_value());
    case ::googlesql::TYPE_NUMERIC:
      return WrapNumeric(a.numeric_value().Subtract(b.numeric_value()),
                         /*is_bignum=*/false);
    case ::googlesql::TYPE_BIGNUMERIC:
      return WrapNumeric(a.bignumeric_value().Subtract(b.bignumeric_value()),
                         /*is_bignum=*/true);
    default:
      return absl::InvalidArgumentError(absl::StrCat(
          "semantic: '-' not implemented for ", a.type()->DebugString()));
  }
}

absl::StatusOr<Value> ArithmeticMul(const Value& a, const Value& b) {
  if (a.type_kind() != b.type_kind()) {
    return absl::InvalidArgumentError(
        absl::StrCat("semantic: '*' operands have mismatched kinds: ",
                     a.type()->DebugString(),
                     " vs ",
                     b.type()->DebugString()));
  }
  switch (a.type_kind()) {
    case ::googlesql::TYPE_INT64:
      return MulInt64(a.int64_value(), b.int64_value());
    case ::googlesql::TYPE_DOUBLE:
      return Value::Double(a.double_value() * b.double_value());
    case ::googlesql::TYPE_NUMERIC:
      return WrapNumeric(a.numeric_value().Multiply(b.numeric_value()),
                         /*is_bignum=*/false);
    case ::googlesql::TYPE_BIGNUMERIC:
      return WrapNumeric(a.bignumeric_value().Multiply(b.bignumeric_value()),
                         /*is_bignum=*/true);
    default:
      return absl::InvalidArgumentError(absl::StrCat(
          "semantic: '*' not implemented for ", a.type()->DebugString()));
  }
}

absl::StatusOr<Value> ArithmeticDiv(const Value& a, const Value& b) {
  // BigQuery's `<a> / <b>` lowers to FLOAT64 division regardless of
  // operand type (INT64 / INT64 is FLOAT64). The analyzer inserts
  // an explicit CAST inside the call for non-FLOAT64 operands; here
  // we only see FLOAT64-on-FLOAT64 for the `$divide` operator unless
  // the call is wrapped in SAFE_DIVIDE (a separate function).
  if (a.type_kind() == ::googlesql::TYPE_DOUBLE &&
      b.type_kind() == ::googlesql::TYPE_DOUBLE) {
    double divisor = b.double_value();
    if (divisor == 0.0) {
      return MakeSemanticError(
          SemanticErrorReason::kDivisionByZero,
          absl::StrCat("division by zero: ", a.double_value(), " / 0"));
    }
    return Value::Double(a.double_value() / divisor);
  }
  if (a.type_kind() == ::googlesql::TYPE_NUMERIC &&
      b.type_kind() == ::googlesql::TYPE_NUMERIC) {
    auto result = a.numeric_value().Divide(b.numeric_value());
    if (!result.ok()) {
      const std::string msg(result.status().message());
      if (absl::StrContains(msg, "by zero") ||
          absl::StrContains(msg, "Division by zero")) {
        return MakeSemanticError(SemanticErrorReason::kDivisionByZero, msg);
      }
      return MakeSemanticError(SemanticErrorReason::kOverflow, msg);
    }
    return Value::Numeric(*result);
  }
  if (a.type_kind() == ::googlesql::TYPE_BIGNUMERIC &&
      b.type_kind() == ::googlesql::TYPE_BIGNUMERIC) {
    auto result = a.bignumeric_value().Divide(b.bignumeric_value());
    if (!result.ok()) {
      const std::string msg(result.status().message());
      if (absl::StrContains(msg, "by zero") ||
          absl::StrContains(msg, "Division by zero")) {
        return MakeSemanticError(SemanticErrorReason::kDivisionByZero, msg);
      }
      return MakeSemanticError(SemanticErrorReason::kOverflow, msg);
    }
    return Value::BigNumeric(*result);
  }
  return absl::InvalidArgumentError(
      absl::StrCat("semantic: '/' not implemented for (",
                   a.type()->DebugString(),
                   ", ",
                   b.type()->DebugString(),
                   ")"));
}

absl::StatusOr<Value> UnaryMinus(const Value& a) {
  switch (a.type_kind()) {
    case ::googlesql::TYPE_INT64: {
      if (a.int64_value() == std::numeric_limits<int64_t>::min()) {
        return MakeSemanticError(SemanticErrorReason::kOverflow,
                                 "Int64 overflow: -INT64_MIN");
      }
      return Value::Int64(-a.int64_value());
    }
    case ::googlesql::TYPE_DOUBLE:
      return Value::Double(-a.double_value());
    case ::googlesql::TYPE_NUMERIC:
      // NumericValue::Negate is total (returns a NumericValue by
      // value): NUMERIC's max-magnitude representation is symmetric,
      // so unary minus cannot overflow on a valid value.
      return Value::Numeric(a.numeric_value().Negate());
    case ::googlesql::TYPE_BIGNUMERIC: {
      auto neg = a.bignumeric_value().Negate();
      if (!neg.ok()) {
        return MakeSemanticError(SemanticErrorReason::kOverflow,
                                 neg.status().message());
      }
      return Value::BigNumeric(*neg);
    }
    default:
      return absl::InvalidArgumentError(absl::StrCat(
          "semantic: unary '-' not implemented for ", a.type()->DebugString()));
  }
}

// ---- Comparisons -----------------------------------------------------------

// Map a 3-way comparison (a < b, a == b, a > b) onto a BOOL Value
// matching the SQL operator. The analyzer already resolved the
// operand types so they compare directly; we leverage Value::Equals
// / Value::LessThan which both honor BigQuery's collation /
// floating-point conventions (NaN handling, ARRAY ordering, ...).
absl::StatusOr<Value> CompareLess(const Value& a, const Value& b) {
  return Value::Bool(a.LessThan(b));
}
absl::StatusOr<Value> CompareEqual(const Value& a, const Value& b) {
  return Value::Bool(a.Equals(b));
}

// ---- Builtin dispatch ------------------------------------------------------

// Convert a NULL argument to the matching NULL result for operators
// that propagate NULL when ANY argument is NULL. Returns the
// matching `NullOfType(return_type)` Value if any arg is NULL, or
// `std::nullopt` if all args are non-null.
std::optional<Value> NullIfAnyNull(const std::vector<Value>& args,
                                   const ::googlesql::Type* return_type) {
  for (const auto& v : args) {
    if (v.is_null()) return NullOfType(return_type);
  }
  return std::nullopt;
}

absl::StatusOr<Value> DispatchAdd(const std::vector<Value>& args,
                                  const ::googlesql::Type* return_type) {
  if (args.size() != 2) {
    return absl::InvalidArgumentError(
        "semantic: '+' expects exactly two arguments");
  }
  if (auto n = NullIfAnyNull(args, return_type)) return *n;
  return ArithmeticAdd(args[0], args[1]);
}

absl::StatusOr<Value> DispatchSub(const std::vector<Value>& args,
                                  const ::googlesql::Type* return_type) {
  if (args.size() != 2) {
    return absl::InvalidArgumentError(
        "semantic: '-' expects exactly two arguments");
  }
  if (auto n = NullIfAnyNull(args, return_type)) return *n;
  return ArithmeticSub(args[0], args[1]);
}

absl::StatusOr<Value> DispatchMul(const std::vector<Value>& args,
                                  const ::googlesql::Type* return_type) {
  if (args.size() != 2) {
    return absl::InvalidArgumentError(
        "semantic: '*' expects exactly two arguments");
  }
  if (auto n = NullIfAnyNull(args, return_type)) return *n;
  return ArithmeticMul(args[0], args[1]);
}

absl::StatusOr<Value> DispatchDiv(const std::vector<Value>& args,
                                  const ::googlesql::Type* return_type) {
  if (args.size() != 2) {
    return absl::InvalidArgumentError(
        "semantic: '/' expects exactly two arguments");
  }
  if (auto n = NullIfAnyNull(args, return_type)) return *n;
  return ArithmeticDiv(args[0], args[1]);
}

absl::StatusOr<Value> DispatchUnaryMinus(const std::vector<Value>& args,
                                         const ::googlesql::Type* return_type) {
  if (args.size() != 1) {
    return absl::InvalidArgumentError(
        "semantic: unary '-' expects exactly one argument");
  }
  if (args[0].is_null()) return NullOfType(return_type);
  return UnaryMinus(args[0]);
}

absl::StatusOr<Value> DispatchEqual(const std::vector<Value>& args) {
  if (args.size() != 2) {
    return absl::InvalidArgumentError(
        "semantic: '=' expects exactly two arguments");
  }
  if (args[0].is_null() || args[1].is_null()) return Value::NullBool();
  return CompareEqual(args[0], args[1]);
}

absl::StatusOr<Value> DispatchNotEqual(const std::vector<Value>& args) {
  if (args.size() != 2) {
    return absl::InvalidArgumentError(
        "semantic: '!=' expects exactly two arguments");
  }
  if (args[0].is_null() || args[1].is_null()) return Value::NullBool();
  return Value::Bool(!args[0].Equals(args[1]));
}

absl::StatusOr<Value> DispatchLess(const std::vector<Value>& args) {
  if (args.size() != 2) {
    return absl::InvalidArgumentError(
        "semantic: '<' expects exactly two arguments");
  }
  if (args[0].is_null() || args[1].is_null()) return Value::NullBool();
  return CompareLess(args[0], args[1]);
}

absl::StatusOr<Value> DispatchLessOrEqual(const std::vector<Value>& args) {
  if (args.size() != 2) {
    return absl::InvalidArgumentError(
        "semantic: '<=' expects exactly two arguments");
  }
  if (args[0].is_null() || args[1].is_null()) return Value::NullBool();
  return Value::Bool(args[0].LessThan(args[1]) || args[0].Equals(args[1]));
}

absl::StatusOr<Value> DispatchGreater(const std::vector<Value>& args) {
  if (args.size() != 2) {
    return absl::InvalidArgumentError(
        "semantic: '>' expects exactly two arguments");
  }
  if (args[0].is_null() || args[1].is_null()) return Value::NullBool();
  return Value::Bool(args[1].LessThan(args[0]));
}

absl::StatusOr<Value> DispatchGreaterOrEqual(const std::vector<Value>& args) {
  if (args.size() != 2) {
    return absl::InvalidArgumentError(
        "semantic: '>=' expects exactly two arguments");
  }
  if (args[0].is_null() || args[1].is_null()) return Value::NullBool();
  return Value::Bool(args[1].LessThan(args[0]) || args[0].Equals(args[1]));
}

// Three-valued logic for AND: TRUE AND TRUE -> TRUE, ANY FALSE ->
// FALSE, NULL with non-FALSE -> NULL. The order matches BigQuery's
// short-circuit semantics on a TRUE operand discovered last.
absl::StatusOr<Value> DispatchAnd(const std::vector<Value>& args) {
  bool seen_null = false;
  for (const auto& v : args) {
    if (v.is_null()) {
      seen_null = true;
      continue;
    }
    if (v.type_kind() != ::googlesql::TYPE_BOOL) {
      return absl::InvalidArgumentError("semantic: AND argument is not BOOL");
    }
    if (!v.bool_value()) return Value::Bool(false);
  }
  if (seen_null) return Value::NullBool();
  return Value::Bool(true);
}

absl::StatusOr<Value> DispatchOr(const std::vector<Value>& args) {
  bool seen_null = false;
  for (const auto& v : args) {
    if (v.is_null()) {
      seen_null = true;
      continue;
    }
    if (v.type_kind() != ::googlesql::TYPE_BOOL) {
      return absl::InvalidArgumentError("semantic: OR argument is not BOOL");
    }
    if (v.bool_value()) return Value::Bool(true);
  }
  if (seen_null) return Value::NullBool();
  return Value::Bool(false);
}

absl::StatusOr<Value> DispatchNot(const std::vector<Value>& args) {
  if (args.size() != 1) {
    return absl::InvalidArgumentError(
        "semantic: NOT expects exactly one argument");
  }
  if (args[0].is_null()) return Value::NullBool();
  if (args[0].type_kind() != ::googlesql::TYPE_BOOL) {
    return absl::InvalidArgumentError("semantic: NOT argument is not BOOL");
  }
  return Value::Bool(!args[0].bool_value());
}

absl::StatusOr<Value> DispatchIsNull(const std::vector<Value>& args,
                                     bool negate) {
  if (args.size() != 1) {
    return absl::InvalidArgumentError(
        "semantic: IS [NOT] NULL expects exactly one argument");
  }
  bool is_null = args[0].is_null();
  return Value::Bool(negate ? !is_null : is_null);
}

absl::StatusOr<Value> DispatchCoalesce(const std::vector<Value>& args,
                                       const ::googlesql::Type* return_type) {
  for (const auto& v : args) {
    if (!v.is_null()) return v;
  }
  return NullOfType(return_type);
}

absl::StatusOr<Value> DispatchIfNull(const std::vector<Value>& args) {
  if (args.size() != 2) {
    return absl::InvalidArgumentError(
        "semantic: IFNULL expects exactly two arguments");
  }
  if (!args[0].is_null()) return args[0];
  return args[1];
}

absl::StatusOr<Value> DispatchNullIf(const std::vector<Value>& args,
                                     const ::googlesql::Type* return_type) {
  if (args.size() != 2) {
    return absl::InvalidArgumentError(
        "semantic: NULLIF expects exactly two arguments");
  }
  if (args[0].is_null() || args[1].is_null()) {
    return args[0];
  }
  if (args[0].Equals(args[1])) return NullOfType(return_type);
  return args[0];
}

// `IF(cond, then, else)`. NULL `cond` evaluates the else branch
// (BigQuery treats NULL as not-TRUE).
absl::StatusOr<Value> DispatchIf(const std::vector<Value>& args) {
  if (args.size() != 3) {
    return absl::InvalidArgumentError(
        "semantic: IF expects exactly three arguments");
  }
  const Value& cond = args[0];
  if (!cond.is_null() && cond.type_kind() == ::googlesql::TYPE_BOOL &&
      cond.bool_value()) {
    return args[1];
  }
  return args[2];
}

// `$case_with_value(input, when1, then1, ..., else)`. The analyzer
// always appends an explicit ELSE arm (NULL of return_type when the
// SQL omits one), so the argument list size is always even (input
// + N when/then pairs + 1 else).
absl::StatusOr<Value> DispatchCaseWithValue(
    const std::vector<Value>& args, const ::googlesql::Type* return_type) {
  if (args.size() < 2 || (args.size() % 2) != 0) {
    return absl::InvalidArgumentError(
        "semantic: $case_with_value expects an even argument list "
        "(input + N when/then pairs + else)");
  }
  const Value& input = args[0];
  for (size_t i = 1; i + 1 < args.size(); i += 2) {
    const Value& when = args[i];
    const Value& then = args[i + 1];
    // BigQuery's CASE x WHEN y semantics: when both `input` and
    // `when` are NULL the comparison is NULL (skipped); otherwise
    // they compare with the usual Equals contract.
    if (input.is_null() || when.is_null()) continue;
    if (input.Equals(when)) return then;
  }
  // ELSE arm is the last entry.
  return args.back().is_null() ? NullOfType(return_type) : args.back();
}

absl::StatusOr<Value> DispatchCaseNoValue(
    const std::vector<Value>& args, const ::googlesql::Type* return_type) {
  // `$case_no_value(cond1, then1, cond2, then2, ..., else)`. The
  // analyzer always appends an else arm.
  if (args.size() < 1 || (args.size() % 2) == 0) {
    return absl::InvalidArgumentError(
        "semantic: $case_no_value expects an odd argument list "
        "(N cond/then pairs + else)");
  }
  for (size_t i = 0; i + 1 < args.size(); i += 2) {
    const Value& cond = args[i];
    const Value& then = args[i + 1];
    if (!cond.is_null() && cond.type_kind() == ::googlesql::TYPE_BOOL &&
        cond.bool_value()) {
      return then;
    }
  }
  return args.back().is_null() ? NullOfType(return_type) : args.back();
}

// SAFE arithmetic helpers (SAFE_ADD, SAFE_SUBTRACT, SAFE_MULTIPLY,
// SAFE_NEGATE, SAFE_DIVIDE) wrap the strict operators and convert
// overflow / division-by-zero into NULL of the return type.
absl::StatusOr<Value> WrapSafe(absl::StatusOr<Value> result,
                               const ::googlesql::Type* return_type) {
  if (result.ok()) return result;
  SemanticErrorReason reason = GetSemanticErrorReason(result.status());
  if (reason == SemanticErrorReason::kOverflow ||
      reason == SemanticErrorReason::kDivisionByZero) {
    return NullOfType(return_type);
  }
  return result;
}

absl::StatusOr<Value> DispatchFunctionByName(
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
  // Fall through to the per-family dispatch table for functions
  // whose `functions.yaml` row picks the `semantic_executor`
  // disposition with `plan=semantic-functions-compliance.plan.md`.
  // `functions::Dispatch` returns nullopt when the name is not
  // wired here; we surface NOT_IMPLEMENTED in that case so the
  // gateway envelope stays the same as for an unknown function.
  if (auto dispatched = functions::Dispatch(name, args, return_type)) {
    return *std::move(dispatched);
  }
  return MakeSemanticError(
      SemanticErrorReason::kNotImplemented,
      absl::StrCat("semantic: function '",
                   name,
                   "' is not yet implemented in the semantic executor"));
}

}  // namespace

absl::StatusOr<Value> EvalFunctionCall(
    const ::googlesql::ResolvedFunctionCall& call, const EvalContext& ctx) {
  if (call.function() == nullptr) {
    return absl::InvalidArgumentError(
        "semantic: ResolvedFunctionCall has null function");
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
  const std::string name = absl::AsciiStrToLower(call.function()->Name());
  auto result = DispatchFunctionByName(name, args, call.type());
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

absl::StatusOr<Value> EvalExpr(const ::googlesql::ResolvedExpr& expr,
                               const EvalContext& ctx) {
  switch (expr.node_kind()) {
    case ::googlesql::RESOLVED_LITERAL: {
      const auto& lit = *expr.GetAs<::googlesql::ResolvedLiteral>();
      // ResolvedLiteral carries the analyzer-validated `Value`
      // directly; copying it is cheap (refcounted backing for
      // STRING / ARRAY / STRUCT).
      return lit.value();
    }
    case ::googlesql::RESOLVED_PARAMETER: {
      const auto& param = *expr.GetAs<::googlesql::ResolvedParameter>();
      if (ctx.parameters == nullptr) {
        return absl::InvalidArgumentError(
            "semantic: ResolvedParameter referenced but no parameter "
            "bindings supplied");
      }
      if (param.is_untyped()) {
        return MakeSemanticError(
            SemanticErrorReason::kInvalidArgument,
            "semantic: untyped parameter has no value to bind to");
      }
      if (!param.name().empty()) {
        const std::string key = absl::AsciiStrToLower(param.name());
        auto it = ctx.parameters->by_name.find(key);
        if (it == ctx.parameters->by_name.end()) {
          return MakeSemanticError(
              SemanticErrorReason::kInvalidArgument,
              absl::StrCat("semantic: no value bound for parameter @",
                           param.name()));
        }
        return it->second;
      }
      if (param.position() > 0) {
        size_t idx = static_cast<size_t>(param.position()) - 1;
        if (idx >= ctx.parameters->by_position.size()) {
          return MakeSemanticError(
              SemanticErrorReason::kInvalidArgument,
              absl::StrCat(
                  "semantic: no value bound for positional parameter #",
                  param.position()));
        }
        return ctx.parameters->by_position[idx];
      }
      return absl::InvalidArgumentError(
          "semantic: ResolvedParameter has neither name nor position");
    }
    case ::googlesql::RESOLVED_FUNCTION_CALL:
      return EvalFunctionCall(*expr.GetAs<::googlesql::ResolvedFunctionCall>(),
                              ctx);
    case ::googlesql::RESOLVED_CAST: {
      const auto& cast = *expr.GetAs<::googlesql::ResolvedCast>();
      if (cast.expr() == nullptr) {
        return absl::InvalidArgumentError(
            "semantic: ResolvedCast has null expr");
      }
      auto inner = EvalExpr(*cast.expr(), ctx);
      if (!inner.ok()) return inner;
      const ::googlesql::Type* target = cast.type();
      if (target == nullptr) {
        return absl::InvalidArgumentError(
            "semantic: ResolvedCast has null type");
      }
      const ::googlesql::Type* source = cast.expr()->type();
      if (source != nullptr && target != nullptr && source->Equals(target)) {
        return inner;
      }
      // The semantic executor's CAST surface is intentionally
      // narrow today; the full table lives with
      // `semantic-functions-compliance.plan.md`. We cover the
      // implicit-coercion casts the analyzer inserts inside scalar
      // arithmetic (INT64 -> FLOAT64 for `/`, ...).
      if (inner->is_null()) return NullOfType(target);
      if (target->kind() == ::googlesql::TYPE_DOUBLE) {
        auto d = ToDouble(*inner);
        if (!d.ok()) return d.status();
        return Value::Double(*d);
      }
      if (target->kind() == ::googlesql::TYPE_INT64 &&
          inner->type_kind() == ::googlesql::TYPE_INT64) {
        return inner;
      }
      return MakeSemanticError(
          SemanticErrorReason::kNotImplemented,
          absl::StrCat("semantic: CAST from ",
                       source != nullptr ? source->DebugString() : "<null>",
                       " to ",
                       target->DebugString(),
                       " is not yet implemented"));
    }
    case ::googlesql::RESOLVED_COLUMN_REF:
      // Column references appear inside scalar-only SELECTs only
      // when the analyzer inserts a `ProjectScan` over a
      // `SingleRowScan` and the projection rewrites a constant
      // column. We currently never see one because every column
      // resolves through `expr_list` (the executor copies the
      // computed column expression directly). If a ref does
      // appear it points at a column we cannot bind locally;
      // surface NOT_IMPLEMENTED with a pointer at the downstream
      // plan that wires column-binding support.
      return MakeSemanticError(
          SemanticErrorReason::kNotImplemented,
          "semantic: column references in scalar-only SELECT are owned by "
          "array-struct-semantic-path.plan.md");
    default:
      return MakeSemanticError(SemanticErrorReason::kNotImplemented,
                               absl::StrCat("semantic: ResolvedExpr kind ",
                                            expr.node_kind_string(),
                                            " is not yet implemented"));
  }
}

}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
